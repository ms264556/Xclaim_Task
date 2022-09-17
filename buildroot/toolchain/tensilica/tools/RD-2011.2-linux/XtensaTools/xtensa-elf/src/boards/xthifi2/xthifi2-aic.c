/*******************************************************************************

Copyright (c) 2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xthifi2-aic.c        Part of board-support-package for XT-HiFi2 board.

Driver implementation for Texas Instruments TLV320AIC31 Audio Codec.

Uses GPIO, I2S and TWI drivers, and 4 KB of the local data RAM.

*******************************************************************************/

#include <xtensa/xtbsp.h>
#include <xtensa/xthifi2.h>
#include <xtensa/xthifi2-sx-types.h>
#include <xtensa/xthifi2-sx-errors.h>
#include <xtensa/xthifi2-sx-gpio.h>
#include <xtensa/xthifi2-sx-twi.h>
#include <xtensa/xthifi2-sx-i2s.h>
#include <xtensa/xthifi2-audio-clkgen.h>

#include <xtensa/xthifi2-aic.h>

#include <stdlib.h>                     // for malloc()
#include <string.h>                     // for memset()
#include <assert.h>

/* AIC device registers */
#define AIC_REG_PAGE_SEL        0
#define AIC_REG_SW_RESET        1
#define AIC_REG_SAMP_RATE       2
#define AIC_REG_PLL_PROG_A      3
#define AIC_REG_PLL_PROG_B      4
#define AIC_REG_PLL_PROG_C      5
#define AIC_REG_PLL_PROG_D      6
#define AIC_REG_DATA_PATH       7
#define AIC_REG_SER_CTRL_A      8
#define AIC_REG_SER_CTRL_B      9
#define AIC_REG_SER_CTRL_C      10
#define AIC_REG_OVF_FLAGS       11
#define AIC_REG_PLL_PROG_R      AIC_REG_OVF_FLAGS
#define AIC_REG_DIG_FILTER      12
// reserved                     13
#define AIC_REG_HEADSET         14
#define AIC_REG_ADC_L_PGA_GAIN  15
#define AIC_REG_ADC_R_PGA_GAIN  16
#define AIC_REG_ADC_IN2LR_TO_L  17
#define AIC_REG_ADC_IN2LR_TO_R  18
#define AIC_REG_ADC_IN1L_TO_L   19
// reserved                     20
#define AIC_REG_ADC_IN1R_TO_L   21
#define AIC_REG_ADC_IN1R_TO_R   22
// reserved                     23
#define AIC_REG_ADC_IN1L_TO_R   24
#define AIC_REG_MIC_BIAS        25
#define AIC_REG_AGC_L_CTRL_A    26
#define AIC_REG_AGC_L_CTRL_B    27
#define AIC_REG_AGC_L_CTRL_C    28
#define AIC_REG_AGC_R_CTRL_A    29
#define AIC_REG_AGC_R_CTRL_B    30
#define AIC_REG_AGC_R_CTRL_C    31
#define AIC_REG_AGC_L_GAIN      32
#define AIC_REG_AGC_R_GAIN      33
#define AIC_REG_AGC_L_NGDEB     34
#define AIC_REG_AGC_R_NGDEB     35
#define AIC_REG_ADC_FLAGS       36
#define AIC_REG_DAC_PWR_CTRL    37
#define AIC_REG_HP_OUT_DVR_CTRL 38
// reserved                     39
#define AIC_REG_HPOUT_STG_CTRL  40
#define AIC_REG_DAC_OUT_SW_CTRL 41
#define AIC_REG_DVR_POP_REDUC   42
#define AIC_REG_DAC_L_VOLUME    43
#define AIC_REG_DAC_R_VOLUME    44
// reserved                     45
// From here, only the regs this driver uses are defined.
#define AIC_REG_HP_L_OUT_LEVEL  51
#define AIC_REG_HP_R_OUT_LEVEL  65
#define AIC_REG_MOD_PWR_STATUS  94

/* AIC driver internal constants and compile-time options */
#define AIC_TWI_ADDR            0x30    // AIC audio device address on TWI
#define AIC_BUFS_PER_CHAN_OUT   2       // # frame buffers per chan for output
#define AIC_BUFS_PER_CHAN_IN    2       // # frame buffers per chan for input
#define AIC_BUFS_PER_CHAN       (AIC_BUFS_PER_CHAN_OUT + AIC_BUFS_PER_CHAN_IN)

/*
By default the AIC uses 4 buffers of 1024x4 bytes (4 KB). These are placed 
in local data RAM. If more or larger buffers are used, they are placed in DDR.
To change this, it is necessary to edit this driver and header and recompile.
FIXME: Make it possible for client to provide buffer space of any size.
The buffers must be 16 byte aligned for the DMA.
*/
static int aic_buffers [AIC_BUFS_PER_CHAN] [AIC_FRAME_SAMPLES]
        #if XCHAL_NUM_DATARAM > 0 && \
            4 * AIC_FRAME_SAMPLES * AIC_BUFS_PER_CHAN <= XCHAL_DATARAM0_SIZE
        __attribute__ ((section (".dram0.bss"))) 
        #endif
        __attribute__ ((aligned(16)));
static int (*aic_next_buffer)[AIC_FRAME_SAMPLES] = &aic_buffers[0];

static sx_i2s_frame_buf*    i2s_buffers_out [AIC_BUFS_PER_CHAN_OUT];
static sx_i2s_frame_buf*    i2s_buffers_in  [AIC_BUFS_PER_CHAN_IN ];
static sx_i2s_stream*       i2s_handle_out  = NULL;
static sx_i2s_stream*       i2s_handle_in   = NULL;

/* Resets the AIC using GPIO */
static void reset_aic()
{
    /* Reset tied to GPIO pin 22, hence bank 2 and bit 6 */
    /* Configure GPIO[22] to be output */
    sx_gpio_bank_set_direction(SX_GPIO_BANK_2,0x40);

    /* Write 1 to GPIO pin */
    sx_gpio_bank_set_bits(SX_GPIO_BANK_2, 0x40, 0x40);

    /* Delay */
    xtbsp_delay_ns(1000);

    /* Reset by writing 0 to GPIO pin */
    sx_gpio_bank_set_bits(SX_GPIO_BANK_2, 0x0, 0x40);

    /* Delay */
    xtbsp_delay_ns(1000);

    /* Make AIC come out of reset by writing 1 */
    sx_gpio_bank_set_bits(SX_GPIO_BANK_2, 0x40, 0x40);

    /* Delay */
    xtbsp_delay_ns(1000);
}

/* TWI initialization */
static aic_err_t init_twi()
{
    sx_twi_config twi_conf;

    memset(&twi_conf, 0, sizeof(twi_conf));
    twi_conf.ten_bit_master = 0;
    twi_conf.speed = SX_TWI_CNTL_SPEED_STANDARD;

    return sx_twi_init(&twi_conf) == SXERR_NONE ? aic_err_ok : aic_err_fail;
}

/* Write <data> to clock generator register <reg_no>. */
static aic_err_t write_clk_gen_reg(
                    unsigned char dev_addr, 
                    unsigned char reg_no, 
                    unsigned char data)
{
    sx_uint8 write_data[2];
    sx_uint32 status;

    write_data[0] = reg_no;
    write_data[1] = data;

    /* Select register using reg_no */
    status = sx_twi_write_bytes(dev_addr, 2, write_data);
    if (status != SXERR_NONE)
        return aic_err_fail;

    return aic_err_ok;
}

/* Write <data> to register <reg_no>. */
static aic_err_t write_320_aic_reg(unsigned char reg_no, unsigned char data)
{
    sx_uint8 write_data[2];
    sx_uint32 status;

    write_data[0] = reg_no;
    write_data[1] = data;

    /* Select register using reg_no */
    status = sx_twi_write_bytes(AIC_TWI_ADDR, 2, write_data);
    if (status != SXERR_NONE)
        return aic_err_fail;

    return aic_err_ok;
}

/* Read from register <reg_no> and return in <data>. */
static aic_err_t read_320_aic_reg(unsigned char reg_no, unsigned char* data)
{
    sx_uint32 status;

    status = sx_twi_write_read_bytes(AIC_TWI_ADDR, 1, &reg_no, 1, data);
    if (status != SXERR_NONE)
        return aic_err_fail;

    return status == SXERR_NONE ? aic_err_ok : aic_err_fail;
}

/* Select register page by writing to the page select register
   Only legal inputs are 0 and 1 */
static aic_err_t select_reg_page(unsigned char page_no)
{
    aic_err_t status;
    #ifdef DEBUG
    unsigned char*  page_no_read;
    #endif

    /* Register 0 is page select register */
    status = write_320_aic_reg(AIC_REG_PAGE_SEL, page_no);
    if (status != aic_err_ok)
        return aic_err_fail;
    #ifdef DEBUG
    /* Read back page number from page select register to confirm */
    status = read_320_aic_reg(AIC_REG_PAGE_SEL, &page_no_read);

    if (page_no_read != page_no)
        return aic_err_fail;
    #endif
    return status;
}

/* Generate audio bit clock and word clock based on sampling frequency */
static aic_err_t audio_clk_gen_init(aic_samp_freq_t samp_freq_in)
{
    clk_gen_samp_freq_t clk_gen_samp_freq;
    audio_out_t audio_out_device;

    /* Sampling frequency group: Each group corresponds to a 
     * different frequency of the base oscillator */
    int sampling_freq_group;
    unsigned char ref_divider_M;
    unsigned char fb_divider_N_L;
    unsigned char fb_divider_N_H;

    /* Get clock generator sampling frequency */ 
    switch(samp_freq_in) {
        case aic_samp_freq_8k:
            clk_gen_samp_freq = clk_gen_samp_freq_8k;
            sampling_freq_group = 0;
            break;
        case aic_samp_freq_11_025k:
            clk_gen_samp_freq = clk_gen_samp_freq_11_025k;
            sampling_freq_group = 1;
            break;
        case aic_samp_freq_12k:
            clk_gen_samp_freq = clk_gen_samp_freq_12k;
            sampling_freq_group = 0;
            break;
        case aic_samp_freq_16k:
            clk_gen_samp_freq = clk_gen_samp_freq_16k;
            sampling_freq_group = 0;
            break;
        case aic_samp_freq_22_05k:
            clk_gen_samp_freq = clk_gen_samp_freq_22_05k;
            sampling_freq_group = 1;
            break;
        case aic_samp_freq_24k:
            clk_gen_samp_freq = clk_gen_samp_freq_24k;
            sampling_freq_group = 0;
            break;
        case aic_samp_freq_32k:
            clk_gen_samp_freq = clk_gen_samp_freq_32k;
            sampling_freq_group = 0;
            break;
        case aic_samp_freq_44_1k:
            clk_gen_samp_freq = clk_gen_samp_freq_44_1k;
            sampling_freq_group = 1;
            break;
        case aic_samp_freq_48k:
            clk_gen_samp_freq = clk_gen_samp_freq_48k;
            sampling_freq_group = 0;
            break;
        case aic_samp_freq_88_2k:
            clk_gen_samp_freq = clk_gen_samp_freq_88_2k;
            sampling_freq_group = 1;
            break;
        case aic_samp_freq_96k:     
            clk_gen_samp_freq = clk_gen_samp_freq_96k;
            sampling_freq_group = 0;
            break;
        default:
            return aic_err_fail;
    }

    /* Output device as aic */
    audio_out_device = audio_out_aic;

    /* Set values of M and N based on sampling frequency group */
    switch(sampling_freq_group) {
    case 0:
        ref_divider_M = 0xE1;
        fb_divider_N_L = 0x00;
        fb_divider_N_H = 0xC8;
        break;
    case 1:
        ref_divider_M = 0x77;
        fb_divider_N_L = 0x20;
        fb_divider_N_H = 0xCD;
        break;
    default:
        return aic_err_fail;
    }

    /* Program the base oscillator */
    /* Set reference divider M */
    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      BASE_OSCILLATOR_M_REG,
                      ref_divider_M) != aic_err_ok)
        return aic_err_fail;

    /* Set feedback divider N lower bits */
    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      BASE_OSCILLATOR_N_L_REG,
                      fb_divider_N_L) != aic_err_ok)
        return aic_err_fail;

    /* Set feedback divider N higher bits */
    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                  BASE_OSCILLATOR_N_H_REG,
                  fb_divider_N_H) != aic_err_ok)
        return aic_err_fail;

    /* Set sampling frequency and audio out mode */
    if(write_clk_gen_reg(AUDIO_CLK_GENERATOR_TWI_ADDR,
            AUDIO_CLK_GENERATOR_SAMP_FREQ_REG,
            (clk_gen_samp_freq << 1) | audio_out_device) != aic_err_ok)
        return aic_err_fail;

    return aic_err_ok;
}

/* AIC initialization */
static aic_err_t init_aic(aic_samp_freq_t freq)
{
    unsigned char data;
    unsigned char pllProgJ;
    unsigned char pllProgDLower;
    unsigned char pllProgDHigher;
    unsigned int  pllProgD;

    /* Reset 320_AIC_31 */
    reset_aic();

    /* Set active page to page 0 */
    if (select_reg_page(0) != aic_err_ok)
        return aic_err_fail;

    /* Reg3: PLL control - PLL enabled, P = 1, Q = 2*/
    if (write_320_aic_reg(AIC_REG_PLL_PROG_A, 0x91) != aic_err_ok)
        return aic_err_fail;

    switch (freq) {
        case aic_samp_freq_8k:
            pllProgJ = 1;
            pllProgD = 3653;
            break;
        case aic_samp_freq_11_025k:
            pllProgJ = 1;
            pllProgD = 8816;
            break;
        case aic_samp_freq_12k:
            pllProgJ = 2;
            pllProgD = 480;
            break;
        case aic_samp_freq_16k:
            pllProgJ = 2;
            pllProgD = 7300;
            break;
        case aic_samp_freq_22_05k:
            pllProgJ = 3;
            pllProgD = 7632;
            break;
        case aic_samp_freq_24k:
            pllProgJ = 4;
            pllProgD = 960;
            break;
        case aic_samp_freq_32k:
            pllProgJ = 5;
            pllProgD = 4610;
            break;
        case aic_samp_freq_44_1k:
            pllProgJ = 7;
            pllProgD = 5264;
            break;
        case aic_samp_freq_48k:
            pllProgJ = 8;
            pllProgD = 1920;
            break;
        case aic_samp_freq_88_2k:
            pllProgJ = 7;
            pllProgD = 5264;
            /* Have to set PLL R value to 2*/
            if (write_320_aic_reg(AIC_REG_PLL_PROG_R,
                        pllProgDLower << 2) != aic_err_ok) {
                return aic_err_fail;
            }
            break;
        case aic_samp_freq_96k:     
            pllProgJ = 8;
            pllProgD = 1920;
            /* Have to set PLL R value to 2*/
            if (write_320_aic_reg(AIC_REG_PLL_PROG_R,
                        pllProgDLower << 2) != aic_err_ok) {
                return aic_err_fail;
            }
            break;
        default:
            return aic_err_fail;
    }

    /* Reg4: PLL control - J*/
    if (write_320_aic_reg(AIC_REG_PLL_PROG_B,
                            (pllProgJ << 2)) != aic_err_ok) {
        return aic_err_fail;
    }

    pllProgDLower  = pllProgD & 0x3F;
    pllProgDHigher = (pllProgD >> 6) & 0xFF;

    /* Reg5: PLL control - D Higher bits */
    if (write_320_aic_reg(AIC_REG_PLL_PROG_C, 
                        pllProgDHigher) != aic_err_ok)  {
        return aic_err_fail;
    }

    /* Reg6: PLL control - D Lower bits */
    if (write_320_aic_reg(AIC_REG_PLL_PROG_D, 
                        pllProgDLower << 2) != aic_err_ok)  {
        return aic_err_fail;
    }

    /* Reg7: Codec datapath setup
       Left DAC datapath plays left channel
       and right DAC datapath plays right channel */
    if (write_320_aic_reg(AIC_REG_DATA_PATH, 0x0A) != aic_err_ok)
    {
        return aic_err_fail;
    }

    /* Reg8: Setup DAC to be in slave mode  */
    if (write_320_aic_reg(AIC_REG_SER_CTRL_A, 0x00) != aic_err_ok)
        return aic_err_fail;

    /* Reg9: Setup for 32 bit data */
    if (write_320_aic_reg(AIC_REG_SER_CTRL_B, 0x30) != aic_err_ok)
        return aic_err_fail;

    /* Reg15: Unmute left ADC PGA, set gain to 40dB */
    if (write_320_aic_reg(AIC_REG_ADC_L_PGA_GAIN, 0x50) != aic_err_ok)
       return aic_err_fail;

    /* Reg16: Unmute right ADC PGA, set gain to 40dB */
    if (write_320_aic_reg(AIC_REG_ADC_R_PGA_GAIN, 0x50) != aic_err_ok)
       return aic_err_fail;

    /* Reg19: Connect IN1L as single ended input to left ADC PGA.
     * Power up left ADC channel. */
    if (write_320_aic_reg(AIC_REG_ADC_IN1L_TO_L, 0x04) != aic_err_ok)
        return aic_err_fail;

    /* Reg22: Connect IN1R as single ended input to right ADC PGA 
     * Power up right ADC channel. */
    if (write_320_aic_reg(AIC_REG_ADC_IN1R_TO_R, 0x04) != aic_err_ok)
        return aic_err_fail;

    /* Reg37: DAC power - Power up both DACs */
    if (write_320_aic_reg(AIC_REG_DAC_PWR_CTRL, 0xC0) != aic_err_ok)
        return aic_err_fail;

    /* Reg38: High power o/p control - enable Short Circuit protection */
    if (write_320_aic_reg(AIC_REG_HP_OUT_DVR_CTRL, 0x04) != aic_err_ok)
        return aic_err_fail;

    /* Reg41:DAC ouptput control - DAC_L2 and DAC_R2 paths selected as
       high power output drivers are to be used. Right DAC volume control
       set to follow left channel volume control */
    if (write_320_aic_reg(AIC_REG_DAC_OUT_SW_CTRL, 0xA2) != aic_err_ok)
        return aic_err_fail;

    /* Reg43:Left DAC volume control (initially full volume) */
    if (write_320_aic_reg(AIC_REG_DAC_L_VOLUME, 0x80) != aic_err_ok)
        return aic_err_fail;

    /* Reg44:Right DAC volume control (initially full volume) */
    if (write_320_aic_reg(AIC_REG_DAC_R_VOLUME, 0x80) != aic_err_ok)
        return aic_err_fail;

    /* Connect PGA to HPLOUT */
    //if (write_320_aic_reg(46, 0x80) != aic_err_ok)
    //    return aic_err_fail;
    //if (write_320_aic_reg(63, 0x80) != aic_err_ok)
    //    return aic_err_fail;

    /*Turn on AGC */
    //if (write_320_aic_reg(26, 0x80) != aic_err_ok)
    //    return aic_err_fail;
    //if (write_320_aic_reg(29, 0x80) != aic_err_ok)
    //    return aic_err_fail;

    /* Reg51: HPLOUT level control - Power up HPLOUT */
    if (write_320_aic_reg(AIC_REG_HP_L_OUT_LEVEL, 0x0F) != aic_err_ok)
        return aic_err_fail;

    /* Reg65: HPROUT level control - Power up HPROUT */
    if (write_320_aic_reg(AIC_REG_HP_R_OUT_LEVEL, 0x0F) != aic_err_ok)
        return aic_err_fail;

    /* Reg94: Module power status register - Read to make sure
              DACs, HPLOUT and HPROUT are powered up */
    read_320_aic_reg(AIC_REG_MOD_PWR_STATUS, &data);

    if (data != 0xC6)
        return aic_err_fail;

    return aic_err_ok;
}

/* Allocates a frame buffer.
   This is a modified version of sx_i2s_frame_buffer_alloc() which uses
   the buffer space reserved in local DRAM. Use the sx_frame_buffer_allo()
   if allocating buffers in system RAM (DDR). */
static sx_i2s_frame_buf* allocate_frame_buffer(void)
{
    sx_i2s_frame_buf *buffer;

    buffer = malloc(sizeof(sx_i2s_frame_buf));
    if (!buffer)
        return NULL;

    buffer->timestamp = 0;

    if (aic_next_buffer >= &aic_buffers[AIC_BUFS_PER_CHAN])
    {
        /* No more buffers.  Free descriptor and return error. */
        free(buffer);
        return NULL;
    }

    buffer->buf = aic_next_buffer++;
    assert(((unsigned)buffer->buf & 0xf) == 0); // should be 16-aligned

    /* Success! */
    return buffer;
}

/* Populates an audio frame */
static inline void populate_audio_data(int *buf, int* data)
{
    unsigned i;

    for (i = 0; i < AIC_FRAME_SAMPLES; i++)
    {
        *buf++ = *data++;
    }
    xthal_dcache_region_writeback(buf, AIC_FRAME_SIZE);
}

/* Populates an audio frame with zeros */
static inline void populate_zeros(int* buf)
{
    unsigned i;

    for (i = 0; i < AIC_FRAME_SAMPLES; i++)
    {
        *buf++ = 0;
    }
    xthal_dcache_region_writeback(buf, AIC_FRAME_SIZE);
}

/* Initialize I2S */
static aic_err_t init_i2s(void)
{
    sx_i2s_stream_conf config_i2s;

    memset(&config_i2s, 0, sizeof(sx_i2s_stream_conf));

    /* Configure i2s output stream */
    config_i2s.dir              = SX_I2S_EGRESS;
    config_i2s.order            = SX_I2S_LEFT_FIRST;
    config_i2s.mode             = SX_I2S_DUAL;
    /* I2S port 1 should be used for output data */
    config_i2s.port_num         = 1;
    config_i2s.master_mode      = SX_I2S_SLAVE;
    config_i2s.num_lines        = 1;
    config_i2s.frame_size       = AIC_FRAME_SIZE;
    config_i2s.i2s_data_bits    = SX_I2S_DATA_32_BITS;
    config_i2s.mem_data_width   = SX_I2S_MEMWIDTH_32;
    config_i2s.num_channels     = SX_I2S_2_CHANNELS;
    config_i2s.clk_div          = 0;

    /* Initialize I2S output stream */
    if (sx_i2s_stream_init(&config_i2s, &i2s_handle_out) != SXERR_NONE)
        return aic_err_fail;

    /* Configure I2S input stream */
    memset( &config_i2s, 0, sizeof(sx_i2s_stream_conf) );

    config_i2s.dir              = SX_I2S_INGRESS;
    config_i2s.order            = SX_I2S_LEFT_FIRST;
    config_i2s.mode             = SX_I2S_DUAL;
    /* I2S port 0 should be used for input data */
    config_i2s.port_num         = 0;
    config_i2s.master_mode      = SX_I2S_SLAVE;
    config_i2s.num_lines        = 1;
    config_i2s.frame_size       = AIC_FRAME_SIZE;
    config_i2s.i2s_data_bits    = SX_I2S_DATA_32_BITS;
    config_i2s.mem_data_width   = SX_I2S_MEMWIDTH_32;
    config_i2s.num_channels     = SX_I2S_2_CHANNELS;
    config_i2s.clk_div          = 0;

    /* Initialize I2S input stream */
    if (sx_i2s_stream_init(&config_i2s, &i2s_handle_in) != SXERR_NONE)
        return aic_err_fail;

    return aic_err_ok;
}

/* Driver open function, initializes TWI to program DAC registers,
   then initializes DAC and I2S interface */
aic_err_t aic_open(aic_samp_freq_t freq)
{
    unsigned i;

    /* Check if driver is already open (only one instance can exist) */
    if (i2s_handle_out != NULL || 
        i2s_handle_in != NULL || 
        aic_next_buffer != &aic_buffers[0])
        return aic_err_fail;

    /* Initialize TWI */
    if (init_twi() != aic_err_ok)
        return aic_err_fail;

    /*  Initialize AIC */
    if (init_aic(freq) != aic_err_ok)
        return aic_err_fail;

    /* Audio clock generation */
    if (audio_clk_gen_init(freq) != aic_err_ok)
        return aic_err_fail;

    /*  Initialize I2S */
    if (init_i2s() != aic_err_ok)
        return aic_err_fail;

    /* Allocate and populate audio output buffers */
    for (i = 0; i < AIC_BUFS_PER_CHAN_OUT; i++)
    {
        i2s_buffers_out[i] = allocate_frame_buffer();
        populate_zeros(i2s_buffers_out[i]->buf);
    }

    /* Push some output buffers to driver before enabling it,
       to avoid underflow */
    for (i=0; i < AIC_BUFS_PER_CHAN_OUT; i++)
    {
        sx_i2s_tx_put_full(i2s_handle_out, i2s_buffers_out[i]);
    }

    /* Allocate and populate audio input buffers */
    for (i = 0; i < AIC_BUFS_PER_CHAN_IN; i++)
    {
        i2s_buffers_in[i] = allocate_frame_buffer();
        populate_zeros(i2s_buffers_in[i]->buf);
    }

    /* Push some input buffers to driver before enabling it, 
       to avoid underflow */
    for (i=0; i < AIC_BUFS_PER_CHAN_IN; i++)
    {
        sx_i2s_rx_put_empty(i2s_handle_in, i2s_buffers_in[i] );
    }
    return aic_err_ok;
}

/* Driver close function */
void aic_close()
{
    sx_int32          i;
    sx_i2s_frame_buf* tmp_buf = NULL;

    /* Flush out last audio frames and free the buffers */
    if (i2s_handle_out->state == SX_I2S_STATE_ACTIVE)
    {
        for (i=0; i < AIC_BUFS_PER_CHAN_OUT; i++)
        {
            while ((tmp_buf = sx_i2s_tx_get_empty(i2s_handle_out)) == NULL);
            free(tmp_buf);
        }
    }

    /* Flush out last audio frames and free the buffers */
    if (i2s_handle_in->state == SX_I2S_STATE_ACTIVE)
    {
        for (i=0; i < AIC_BUFS_PER_CHAN_IN; i++)
        {
            while ((tmp_buf = sx_i2s_rx_get_full(i2s_handle_in)) == NULL);
            free(tmp_buf);
        }
    }

    /* Release AIC buffers */
    aic_next_buffer = &aic_buffers[0];

    /* Reset AIC by writing 0 to GPIO pin */
    sx_gpio_bank_set_bits(SX_GPIO_BANK_2, 0x0, 0x40);

    /* Release the I2S streams and clear driver init status */
    sx_i2s_stream_free(i2s_handle_out);
    i2s_handle_out = NULL;
    sx_i2s_stream_free(i2s_handle_in);
    i2s_handle_in = NULL;

    /* Power down oscillators */
    write_clk_gen_reg(AUDIO_CLK_GENERATOR_TWI_ADDR, 
                    AUDIO_CLK_GENERATOR_POWER_UP_REG,0);
}

/* Receive input data from the ADC via I2S port 0 */
aic_err_t aic_read(int* data)
{   
    unsigned            i;
    sx_i2s_frame_buf*   tmp_buf;
    aic_err_t           status;

    /* Check if driver is open */
    if (i2s_handle_in == NULL)
        return aic_err_fail;

    /* Enable the lane */
    if (i2s_handle_in->state != SX_I2S_STATE_ACTIVE)
    {
        /* Power up oscillators */
        if(write_clk_gen_reg(AUDIO_CLK_GENERATOR_TWI_ADDR,
                AUDIO_CLK_GENERATOR_POWER_UP_REG,1) != aic_err_ok)
            return aic_err_fail;

        sx_i2s_stream_enable(i2s_handle_in);
    }

    /* Wait for a buffer the driver has finished with */
    while ((tmp_buf = sx_i2s_rx_get_full(i2s_handle_in)) == NULL);

    /* Fill the buffer with received data */
    populate_audio_data(data, tmp_buf->buf);

    /* Again, ensure audio frame is resident in memory */
    sx_i2s_rx_put_empty(i2s_handle_in, tmp_buf);

    return aic_err_ok;
}

/* Place output data on I2S port 1 to play it on the DAC */
aic_err_t aic_write(int* data)
{
    unsigned            i;
    sx_i2s_frame_buf*   tmp_buf;
    aic_err_t           status;

    /* Check if driver is open */
    if (i2s_handle_out == NULL)
        return aic_err_fail;

    /* Enable the lane */
    if (i2s_handle_out->state != SX_I2S_STATE_ACTIVE)
    {
        /* Power up oscillators */
        if(write_clk_gen_reg(AUDIO_CLK_GENERATOR_TWI_ADDR,
                AUDIO_CLK_GENERATOR_POWER_UP_REG,1) != aic_err_ok)
            return aic_err_fail;

        sx_i2s_stream_enable(i2s_handle_out);
    }

    /* Wait for a buffer the driver has finished with */
    while ((tmp_buf = sx_i2s_tx_get_empty(i2s_handle_out)) == NULL);

    /* Fill the buffer with the new data */
    populate_audio_data(tmp_buf->buf, data);

    /* Again, ensure audio frame is resident in memory */
    sx_i2s_tx_put_full(i2s_handle_out, tmp_buf);

    return aic_err_ok;
}

/* Probe mute status of left and right DAC channels, return 1 iff both muted */
int aic_is_muted(void)
{
    unsigned char volmute;
    aic_err_t status;
    int is_muted = 0;

    /* Check if driver is open */
    if (i2s_handle_out == NULL)
        return aic_err_fail;

    /* Reg43:Left DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_L_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    is_muted |= volmute & 0x80 ? 1 : 0;

    /* Reg44:Right DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_R_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    is_muted |= volmute & 0x80 ? 1 : 0;

    return is_muted;
}

/* Mutes/unmutes left and right DAC channels */
aic_err_t aic_set_mute(int mute)
{
    unsigned char volmute;
    aic_err_t status;

    /* Check if driver is open */
    if (i2s_handle_out == NULL)
        return aic_err_fail;

    /* Reg43:Left DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_L_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    volmute = (volmute & ~0x80) | (mute ? 0x80 : 0);
    status = write_320_aic_reg(AIC_REG_DAC_L_VOLUME, volmute);
    if (status != aic_err_ok)
        return aic_err_fail;

    /* Reg44:Right DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_R_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    volmute = (volmute & ~0x80) | (mute ? 0x80 : 0);
    status = write_320_aic_reg(AIC_REG_DAC_R_VOLUME, volmute);
    if (status != aic_err_ok)
        return aic_err_fail;

    return aic_err_ok;
}

/* Probe volume of left and right DAC channels, return the average. */
int aic_volume(void)
{
    unsigned char volmute;
    unsigned dacvol;
    aic_err_t status;

    /* Check if driver is open */
    if (i2s_handle_out == NULL)
        return aic_err_fail;


    /* Reg43:Left DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_L_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    dacvol = volmute & 0x7f;

    /* Reg44:Right DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_R_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    dacvol = (dacvol + (volmute & 0x7f)) >> 1;

    /* Return the decibel attenuation represented by the DAC setting. */
    return -(dacvol >> 1);
}

/* Volume Control */
aic_err_t aic_set_volume(int vol)
{
    unsigned char volmute, dacvol;
    aic_err_t status;

    /* Check if driver is open */
    if (i2s_handle_out == NULL)
        return aic_err_fail;

    /* Convert requested attenuation in dB to DAC volume setting */
    dacvol = (vol <= 0) ? ((-vol) << 1) : 0;
    dacvol = (dacvol > 0x7f) ? 0x7f : dacvol;

    /* Reg43:Left DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_L_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    volmute = (volmute & 0x80) | (dacvol & 0x7f);
    status = write_320_aic_reg(AIC_REG_DAC_L_VOLUME, volmute);
    if (status != aic_err_ok)
        return aic_err_fail;

    /* Reg44:Right DAC volume control */
    status = read_320_aic_reg (AIC_REG_DAC_R_VOLUME, &volmute);
    if (status != aic_err_ok)
        return aic_err_fail;
    volmute = (volmute & 0x80) | (dacvol & 0x7f);
    status = write_320_aic_reg(AIC_REG_DAC_R_VOLUME, volmute);
    if (status != aic_err_ok)
        return aic_err_fail;

    return aic_err_ok;
}

