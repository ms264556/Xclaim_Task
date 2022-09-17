/*******************************************************************************

Copyright (c) 2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xthifi2-hdmi.c        Part of board-support-package for XT-HiFi2 board.

Driver implementation for Analog Devices AD9889B High Definition 
Multimedia Interface (HDMI) transmitter.

Uses GPIO, I2S and TWI drivers, and 4 KB of the local data RAM.

*******************************************************************************/

#include <xtensa/xtruntime.h>
#include <xtensa/xthifi2.h>
#include <xtensa/xthifi2-sx-types.h>
#include <xtensa/xthifi2-sx-errors.h>
#include <xtensa/xthifi2-sx-gpio.h>
#include <xtensa/xthifi2-sx-twi.h>
#include <xtensa/xthifi2-sx-i2s.h>
#include <xtensa/xthifi2-audio-clkgen.h>

#include <xtensa/xthifi2-hdmi.h>

#include <stdio.h>
#include <stdlib.h>                     // for malloc()
#include <string.h>                     // for memset()
#include <assert.h>

/* HDMI device registers */
/* General Control */
#define HDMI_SYSTEM_PD                       0x41
#define HDMI_PD_POL                          0x42
#define HDMI_MASK1                           0x94
#define HDMI_MASK2                           0x95
#define HDMI_HPD_INT                         0x96

/* HDMI DVI Selection */
#define HDMI_DVI_SELECT                      0xaf
#define HDMI_MODE                            0xc6

/* TMDS Power down */
#define HDMI_TMDSPOWERDOWN                   0xa1

/* System Monitoring */
#define HDMI_HDCP_ERR_INT                    0x97
#define HDMI_AUDIO_DETECTION                 0xb8
#define HDMI_HDCP_CONTROLLER_ERROR           0xc8

/* Reserved Registers that must be set properly!! */
#define HDMI_RSVD0                           0x0a //set to 0x0
#define HDMI_RSVD1                           0x98 //set to 0x03
#define HDMI_RSVD2                           0x9c //set to 0x38
#define HDMI_RSVD3                           0x9d //set to 0x61
#define HDMI_DIFFDATA_DRIVE_STRENGTH         0xa2 //set to 0x84
#define HDMI_DIFFCLK_DRIVE_STRENGTH          0xa3 //set to 0x84
#define HDMI_RSVD4                           0xbb //set to 0xff

/* Video Setup */
#define HDMI_VFE_INPUT_ID                    0x15
#define HDMI_VFE_422_WIDTH                   0x16

/* ReadOnly */
#define HDMI_VFE_FMT_VID                     0x3e
#define HDMI_VFE_AUX_VID                     0x3f

/* Pixel Repeat Related Registers */
#define HDMI_EXT_AUDIOSF_SEL                 0x3b
#define HDMI_EXT_VID_TO_RX                   0x3c
#define HDMI_PR_TO_RX                        0x3d

/* DE HSYNC/VSYNC Registers */
#define HDMI_SYNC_POLARITY                   0x17
#define HDMI_VFE_HS_PLA_MSB                  0x30
#define HDMI_VFE_HS_PLA                      0x31
#define HDMI_VFE_HS_DUR                      0x32
#define HDMI_VFE_VS_PLA                      0x33
#define HDMI_VFE_VS_DUR                      0x34
#define HDMI_VFE_HSDELAYIN_MSB               0x35
#define HDMI_VFE_HSDELAYIN                   0x36
#define HDMI_VFE_INTERLACE_OFFSET            0x37
#define HDMI_VFE_WIDTH                       0x38
#define HDMI_VFE_HEIGHT_MSB                  0x39
#define HDMI_VFE_HEIGHT                      0x3a
#define HDMI_COLOR_SPACE_CONV                0x3b

/* Info Frames Setup */
#define HDMI_AVIIF_PKT                       0x44
#define HDMI_Y1Y0                            0x45
#define AV_MUTE                              0x45
#define HDMI_SCAN_INFO                       0x46
#define HDMI_ACTIVE_FORMAT_ASPECT_RATIO      0x47


/* Audio Setup */
#define HDMI_N_19_16                         0x01
#define HDMI_N_15_8                          0x02
#define HDMI_N_7_0                           0x03
#define HDMI_SPDIF_SF                        0x04
#define HDMI_CTS_INT_15_8                    0x05
#define HDMI_CTS_INT_7_0                     0x06
#define HDMI_CTS_EXT_19_16                   0x07
#define HDMI_CTS_EXT_15_8                    0x08
#define HDMI_CTS_EXT_7_0                     0x09
#define HDMI_CTS_SEL                         0x0a
#define HDMI_MCLK_POL                        0x0b
#define HDMI_I2S_ENABLE                      0x0c
#define HDMI_I2S_BIT_WIDTH                   0x0d
#define HDMI_COPYRIGHT                       0x12
#define HDMI_AUDIO_WORD_LENGTH               0x14
#define HDMI_AUDIO_IF_CC                     0x50
#define HDMI_SPEAKER_MAPPING                 0x51

/* Base oscillator control registers */
#define PLL_MUX         (3  | 0x80)
#define PLL_3_DIV_M     (7  | 0x80)
#define PLL_3_DIV_N     (8  | 0x80)
#define P3_SWITCH_A     (11 | 0x80)
#define P3_DIV          (16 | 0x80)
#define Y2_SWITCH_B     (21 | 0x80)
#define Y3_SWITCH_B     (22 | 0x80)

/* HDMI driver internal constants and compile-time options */
#define HDMI_TWI_ADDR           0x72
#define HDMI_BUFS_PER_CHAN      16     // number of frame buffers

/*
By default the HDMI uses 4 buffers of 1024x4 bytes (4 KB). These are placed 
in local data RAM. If more or larger buffers are used, they are placed in DDR.
To change this, it is necessary to edit this driver and header and recompile.
FIXME: Make it possible for client to provide buffer space of any size.
The buffers must be 16 byte aligned for the DMA.
*/
static int hdmi_buffers [HDMI_BUFS_PER_CHAN] [HDMI_FRAME_SAMPLES]
        #if XCHAL_NUM_DATARAM > 0 && \
            4 * HDMI_FRAME_SAMPLES * HDMI_BUFS_PER_CHAN <= XCHAL_DATARAM0_SIZE
        __attribute__ ((section (".dram0.bss"))) 
        #endif
        __attribute__ ((aligned(16)));
static int (*hdmi_next_buffer)[HDMI_FRAME_SAMPLES] = &hdmi_buffers[0];

static sx_i2s_frame_buf*    i2s_buffers[HDMI_BUFS_PER_CHAN];
static sx_i2s_stream*       i2s_handle = NULL;

/* Write <data> to clock generator register <reg_no>. */
static hdmi_err_t write_clk_gen_reg(
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
        return hdmi_err_fail;

    return hdmi_err_ok;
}

/* Write <data> to HDMI register <reg_no>. */
static hdmi_err_t write_ad9889b_hdmi_reg(
                    unsigned char reg_no, unsigned char data)
{
    sx_uint8 write_data[2];
    sx_uint32 status;

    write_data[0] = reg_no;
    write_data[1] = data;

    /* Select register using reg_no */
    status = sx_twi_write_bytes(HDMI_TWI_ADDR, 2, write_data);
    if (status != SXERR_NONE)
        return hdmi_err_fail;

    return hdmi_err_ok;
}

/* Read from register <reg_no> and return in <data>. */
static hdmi_err_t read_ad9889b_hdmi_reg(
                    unsigned char reg_no, unsigned char* data)
{
    sx_uint32 status;

    status = sx_twi_write_read_bytes(HDMI_TWI_ADDR, 1, &reg_no, 1, data);
    if (status != SXERR_NONE)
        return hdmi_err_fail;

    return status == SXERR_NONE ? hdmi_err_ok : hdmi_err_fail;
}

/* Power up HDMI by writing to system_PD register */
static hdmi_err_t hdmi_powerup(void)
{
    if (write_ad9889b_hdmi_reg(HDMI_SYSTEM_PD, 0x00) != hdmi_err_ok)
        return hdmi_err_fail; 

    return hdmi_err_ok;
}

/* Power down HDMI by writing to system_PD register */
static hdmi_err_t hdmi_powerdown(void)
{
    if (write_ad9889b_hdmi_reg(HDMI_SYSTEM_PD, 0x40) != hdmi_err_ok)
        return hdmi_err_fail; 

    return hdmi_err_ok;
}

/* Write all reserved registers and turn chip on */
static hdmi_err_t hdmi_init(void)
{
    if (write_ad9889b_hdmi_reg(HDMI_SYSTEM_PD, 0x10) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_RSVD0, 0x0) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_RSVD1, 0x03) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_RSVD2, 0x38) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_RSVD3, 0x61) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(
                HDMI_DIFFDATA_DRIVE_STRENGTH, 0x84) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(
                HDMI_DIFFCLK_DRIVE_STRENGTH, 0x84) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_RSVD4, 0xff) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(
                HDMI_ACTIVE_FORMAT_ASPECT_RATIO, 0x80) != hdmi_err_ok)
        return hdmi_err_fail; 

    //Turn on HDMI mode
    if (write_ad9889b_hdmi_reg(HDMI_DVI_SELECT, 0x06) != hdmi_err_ok)
        return hdmi_err_fail; 

    return hdmi_err_ok;
}

/* HDMI video init */
static hdmi_err_t hdmi_video_init()
{
    char rgb_rgb[24] = {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0};  
    char data[24];

    // HDMI INFO frames
    // Set up AVI info frames for RGB 4:3 aspect ratio
    if (write_ad9889b_hdmi_reg(HDMI_Y1Y0, 0x80) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_SCAN_INFO, 0x14) != hdmi_err_ok)
        return hdmi_err_fail; 
    
    // Color space conversion RGB to HDTV
    //  if (write_ad9889b_hdmi_reg(0x18, 0x8) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x19, 0x2d) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x1a, 0x19) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x1b, 0x27) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x1c, 0x1e) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x1d, 0xac) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x1e, 0x08) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x1f, 0x0) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //
    //  if (write_ad9889b_hdmi_reg(0x20, 0x4) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x21, 0xc9) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x22, 0x9) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x23, 0x64) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x24, 0x01) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x25, 0xd3) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x26, 0x0) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x27, 0x0) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //
    //  if (write_ad9889b_hdmi_reg(0x28, 0x1d) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x29, 0x3f) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x2a, 0x1a) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x2b, 0x93) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x2c, 0x8) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x2d, 0x2d) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x2e, 0x8) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
    //  if (write_ad9889b_hdmi_reg(0x2f, 0x0) != hdmi_err_ok)  
    //      return hdmi_err_fail; 
 
    // Set up video
    if (write_ad9889b_hdmi_reg(HDMI_EXT_VID_TO_RX, 0x01) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_VFE_INPUT_ID, 0x0a) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_VFE_422_WIDTH, 0x2) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_SYNC_POLARITY, 0x68) != hdmi_err_ok)
        return hdmi_err_fail; 
    if (write_ad9889b_hdmi_reg(HDMI_COLOR_SPACE_CONV, 0x80) != hdmi_err_ok)
        return hdmi_err_fail; 

    return hdmi_err_ok;
}

/* HDMI Audio init */
static hdmi_err_t hdmi_audio_init(
                    hdmi_samp_freq_t freq, 
                    hdmi_num_chan_t channels,
                    int resolution)
{
    int fifo_int_level_shift = 1;

    /* Sampling frequency */
    switch (freq) {
    case hdmi_samp_freq_32k:
        // Set N Value to 4096 (0x1000)
        if (write_ad9889b_hdmi_reg(HDMI_N_19_16, 0) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_15_8, 0x10) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_7_0, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        /* For HDTV 
        //Set External CTS Value to 27000 (0x6978)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x69) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x78) != hdmi_err_ok)
            return hdmi_err_fail; */
        /* For HDMI Monitor */
        //Set External CTS Value to 25200 (0x6270)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x62) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x70) != hdmi_err_ok)
            return hdmi_err_fail; 
        //Set Sampling frequency to 32KHz
        if (write_ad9889b_hdmi_reg(HDMI_VFE_INPUT_ID, 0x3a) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;

    case hdmi_samp_freq_44_1k:
        // Set N Value to 6272 (0x1880)
        if (write_ad9889b_hdmi_reg(HDMI_N_19_16, 0) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_15_8, 0x18) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_7_0, 0x80) != hdmi_err_ok)
            return hdmi_err_fail; 
        /* For HDTV 
        //Set External CTS Value to 30000 (0x7530)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x75) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x30) != hdmi_err_ok)
            return hdmi_err_fail; */
        /* For HDMI Monitor */
        //Set External CTS Value to 28000 (0x6D60)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x6D) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x60) != hdmi_err_ok)
            return hdmi_err_fail; 
        //Set Sampling frequency to 44.1KHz
        if (write_ad9889b_hdmi_reg(HDMI_VFE_INPUT_ID, 0x0a) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;
        
    case hdmi_samp_freq_48k:
        // Set N Value to 6144 (0x1800)
        if (write_ad9889b_hdmi_reg(HDMI_N_19_16, 0) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_15_8, 0x18) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_7_0, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        /* For HDTV 
        //Set External CTS Value to 27000 (0x6978)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x69) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x78) != hdmi_err_ok)
            return hdmi_err_fail; */
        /* FOr HDMI */
        //Set External CTS Value to 25200 (0x6270)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x62) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x70) != hdmi_err_ok)
            return hdmi_err_fail; 
        //Set Sampling frequency to 48KHz
        if (write_ad9889b_hdmi_reg(HDMI_VFE_INPUT_ID, 0x2a) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;

    case hdmi_samp_freq_88_2k:
        // Set N Value to 12544(0x3100)
        if (write_ad9889b_hdmi_reg(HDMI_N_19_16, 0) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_15_8, 0x31) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_7_0, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        /* For HDTV 
        //Set External CTS Value to 30000 (0x7530)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x75) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x30) != hdmi_err_ok)
            return hdmi_err_fail; */
        /* For HDMI Monitor */
        //Set External CTS Value to 28000 (0x6D60)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x6D) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x60) != hdmi_err_ok)
            return hdmi_err_fail; 
        //Set Sampling frequency to 88.2KHz
        if (write_ad9889b_hdmi_reg(HDMI_VFE_INPUT_ID, 0x8a) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;
        
    case hdmi_samp_freq_96k:
        // Set N Value to 12288(0x3000)
        if (write_ad9889b_hdmi_reg(HDMI_N_19_16, 0) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_15_8, 0x30) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_N_7_0, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        /* For HDTV 
        //Set External CTS Value to 27000 (0x6978)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x69) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x78) != hdmi_err_ok)
            return hdmi_err_fail; */
        /* FOr HDMI */
        //Set External CTS Value to 25200 (0x6270)
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_19_16, 0x00) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_15_8, 0x62) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_CTS_EXT_7_0, 0x70) != hdmi_err_ok)
            return hdmi_err_fail; 
        //Set Sampling frequency to 96KHz
        if (write_ad9889b_hdmi_reg(HDMI_VFE_INPUT_ID, 0xaa) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;

    default:
        return hdmi_err_fail;
        break;
    }

    /* Number of channels */
    /* Speaker channel mapping is hardcoded for now */
    switch (channels) {
    case hdmi_num_chan_2:
        //Enable 2 channels
        if (write_ad9889b_hdmi_reg(HDMI_I2S_ENABLE, 0x04) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_AUDIO_IF_CC, 0x20) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_SPEAKER_MAPPING, 0x0) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;
        
    case hdmi_num_chan_4:
        //Enable 4 channels
        if (write_ad9889b_hdmi_reg(HDMI_I2S_ENABLE, 0x0C) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_AUDIO_IF_CC, 0x60) != hdmi_err_ok)
            return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_SPEAKER_MAPPING, 0x3) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;

    case hdmi_num_chan_6:
        //Enable 6 channels
        if (write_ad9889b_hdmi_reg(HDMI_I2S_ENABLE, 0x1c) != hdmi_err_ok)
                return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_AUDIO_IF_CC, 0xa0) != hdmi_err_ok)
                return hdmi_err_fail; 
        if (write_ad9889b_hdmi_reg(HDMI_SPEAKER_MAPPING, 0xb) != hdmi_err_ok)
                return hdmi_err_fail; 
        break;
        
    case hdmi_num_chan_8:
        //Enable 8 channels
        if (write_ad9889b_hdmi_reg(HDMI_I2S_ENABLE, 0x3c) != hdmi_err_ok)
                return hdmi_err_fail; 

        if (write_ad9889b_hdmi_reg(HDMI_AUDIO_IF_CC, 0xe0) != hdmi_err_ok)
                return hdmi_err_fail; 

        if (write_ad9889b_hdmi_reg(HDMI_SPEAKER_MAPPING, 0xf) != hdmi_err_ok)
                return hdmi_err_fail; 
        break;

    default:
        //Unsupported number
        return hdmi_err_fail;
        break;
    }

    //Set External CTS
    if (write_ad9889b_hdmi_reg(HDMI_CTS_SEL, 0x81) != hdmi_err_ok)
        return hdmi_err_fail; 
    //Set MCLK polarity
    if (write_ad9889b_hdmi_reg(HDMI_MCLK_POL, 0x0e) != hdmi_err_ok)
        return hdmi_err_fail; 
    //Set copyright bit - Not cpoy right protected
    if (write_ad9889b_hdmi_reg(HDMI_COPYRIGHT, 0x20) != hdmi_err_ok)
        return hdmi_err_fail; 

    //Audio resolution
    switch (resolution) {
    case 16:
        if (write_ad9889b_hdmi_reg(HDMI_AUDIO_WORD_LENGTH, 0x2) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;
    case 24:
        if (write_ad9889b_hdmi_reg(HDMI_AUDIO_WORD_LENGTH, 0xb) != hdmi_err_ok)
            return hdmi_err_fail; 
        break;
    default:
        //Unknown resolution
        return hdmi_err_fail;
        break;
    }

    return hdmi_err_ok;
}

/* TWI initialization */
static sx_uint32 init_twi(void)
{
    sx_twi_config twi_conf;

    memset(&twi_conf, 0, sizeof(twi_conf));
    twi_conf.ten_bit_master = 0;
    twi_conf.speed = SX_TWI_CNTL_SPEED_STANDARD;

    return sx_twi_init(&twi_conf);
}

/* Allocates a frame buffer.
   This is a modified version of sx_i2s_frame_buffer_alloc() which uses
   the buffer space reserved in local DRAM. Use the sx_frame_buffer_allo()
   if allocating buffers in system RAM (DDR). */
static sx_i2s_frame_buf* allocate_frame_buffer(void)
{
    sx_i2s_frame_buf *buffer;

    buffer = malloc(sizeof(sx_i2s_frame_buf));
    if (buffer == NULL)
        return NULL;

    buffer->timestamp = 0;

    if (hdmi_next_buffer >= &hdmi_buffers[HDMI_BUFS_PER_CHAN])
    {
        /* No more buffers.  Free descriptor and return error. */
        free(buffer);
        return NULL;
    }

    buffer->buf = hdmi_next_buffer++;
    assert(((unsigned)buffer->buf & 0xf) == 0); // should be 16-aligned

    /* Success! */
    return buffer;
}

/* Populates an audio frame */
static inline void populate_audio_data(int *buf, int* data)
{
    unsigned i;

    for (i = 0; i < HDMI_FRAME_SAMPLES; i++)
    {
        *buf++ = *data++;
    }
    xthal_dcache_region_writeback(buf, HDMI_FRAME_SIZE);
}

/* Populates an audio frame with zeros */
static inline void populate_zeros(int* buf)
{
    unsigned i;

    for (i = 0; i < HDMI_FRAME_SAMPLES; i++)
    {
        *buf++ = 0;
    }
    xthal_dcache_region_writeback(buf, HDMI_FRAME_SIZE);
}

/* Initialize I2S */
static hdmi_err_t init_i2s(int num_lines)
{
    sx_i2s_stream_conf   config_i2s;

    /* Configure the i2s stream */
    memset(&config_i2s, 0, sizeof(sx_i2s_stream_conf));

    config_i2s.dir = SX_I2S_EGRESS;
    config_i2s.order = SX_I2S_LEFT_FIRST;
    config_i2s.mode  = SX_I2S_WIDE;
    config_i2s.master_mode = SX_I2S_SLAVE;
    config_i2s.num_lines = num_lines;
    config_i2s.frame_size = HDMI_FRAME_SIZE;
    config_i2s.i2s_data_bits = SX_I2S_DATA_32_BITS;
    config_i2s.mem_data_width = SX_I2S_MEMWIDTH_32;
    config_i2s.num_channels = SX_I2S_2_CHANNELS;
    config_i2s.clk_div = 0;

    /* Initialize the I2S stream */
    if (sx_i2s_stream_init(&config_i2s, &i2s_handle))
    {
        return hdmi_err_fail;
    }
    
    return hdmi_err_ok;
}

/* Receive data from the HDMI via I2S */
hdmi_err_t hdmi_read(int* data)
{
    return hdmi_err_fail;   // HDMI input is not supported on this board
}

/* Place data on I2S to play it on the HDMI */
hdmi_err_t hdmi_write(int* data)
{
    unsigned            i;
    sx_i2s_frame_buf*   tmp_buf;
    int count = 0;

    /* Check if driver is open */
    if (i2s_handle == NULL)
        return hdmi_err_fail;

    /* Enable the lane */
    if (i2s_handle->state != SX_I2S_STATE_ACTIVE)
    {
        /* Power up oscillators */
        if(write_clk_gen_reg(AUDIO_CLK_GENERATOR_TWI_ADDR,
                AUDIO_CLK_GENERATOR_POWER_UP_REG,1) != hdmi_err_ok)
            return hdmi_err_fail;

        /* Enable i2s stream */
        sx_i2s_stream_enable(i2s_handle);
    }

    /* Wait for a buffer the driver has finished with */
    while ((tmp_buf = sx_i2s_tx_get_empty(i2s_handle)) == NULL);

    /* Fill the buffer with the new data */
    populate_audio_data(tmp_buf->buf, data);

    /* Again, ensure audio frame is resident in memory */
    sx_i2s_tx_put_full(i2s_handle, tmp_buf);

    return hdmi_err_ok;
}

/* Generate audio bit clock and word clock based on sampling frequency */
static hdmi_err_t audio_clk_gen_init(hdmi_samp_freq_t samp_freq_in)
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
    case hdmi_samp_freq_32k: 
        clk_gen_samp_freq = clk_gen_samp_freq_32k;
        sampling_freq_group = 0;
        break;
    case hdmi_samp_freq_44_1k: 
        clk_gen_samp_freq = clk_gen_samp_freq_44_1k;
        sampling_freq_group = 1;
        break;
    case hdmi_samp_freq_48k: 
        clk_gen_samp_freq = clk_gen_samp_freq_48k;
        sampling_freq_group = 0;
        break;
    case hdmi_samp_freq_88_2k: 
        clk_gen_samp_freq = clk_gen_samp_freq_88_2k; 
        sampling_freq_group = 1;
        break;
    case hdmi_samp_freq_96k:
        clk_gen_samp_freq = clk_gen_samp_freq_96k; 
        sampling_freq_group = 0;
        break;
    default:
        return hdmi_err_fail;
    }

    /* Output device as HDMI */
    audio_out_device = audio_out_hdmi;

    /* Set values of M and N based on sampling frequency group */
    switch(sampling_freq_group) {
    case 0:
        ref_divider_M = 0xE1;
        fb_divider_N_L = 0x00;
        //fb_divider_N_H = 0xC8; //for HDTV
        fb_divider_N_H = 0xE8;   //for HDMI Monitor
        break;
    case 1:
        ref_divider_M = 0x77;
        fb_divider_N_L = 0x20;
        //fb_divider_N_H = 0xCD; //for HDTV
        fb_divider_N_H = 0xED;   //for HDMI Monitor
        break;
    default:
        return hdmi_err_fail;
    }

    /* Program the base oscillator */
    // Change video clock to 25.2 MHz for HDMI Montior
    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      PLL_MUX, 0) != hdmi_err_ok)
        return hdmi_err_fail;

    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      PLL_3_DIV_M, 27) != hdmi_err_ok)
        return hdmi_err_fail;

    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      PLL_3_DIV_N, 126) != hdmi_err_ok)
        return hdmi_err_fail;

    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      P3_SWITCH_A, 0x60) != hdmi_err_ok)
        return hdmi_err_fail;

    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                     P3_DIV, 10) != hdmi_err_ok)
        return hdmi_err_fail;

    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      Y2_SWITCH_B, 0x3b) != hdmi_err_ok)
        return hdmi_err_fail;

    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      Y3_SWITCH_B, 0x3b) != hdmi_err_ok)
        return hdmi_err_fail;

    /* Set reference divider M */
    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      BASE_OSCILLATOR_M_REG,
                      ref_divider_M) != hdmi_err_ok)
        return hdmi_err_fail;

    /* Set feedback divider N lower bits */
    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                      BASE_OSCILLATOR_N_L_REG,
                      fb_divider_N_L) != hdmi_err_ok)
        return hdmi_err_fail;

    /* Set feedback divider N higher bits */
    if(write_clk_gen_reg(BASE_OSCILLATOR_TWI_ADDR,
                  BASE_OSCILLATOR_N_H_REG,
                  fb_divider_N_H) != hdmi_err_ok)
        return hdmi_err_fail;

    /* Set sampling frequency and audio out mode */
    if(write_clk_gen_reg(AUDIO_CLK_GENERATOR_TWI_ADDR,
            AUDIO_CLK_GENERATOR_SAMP_FREQ_REG,
            (clk_gen_samp_freq << 1) | audio_out_device) != hdmi_err_ok)
        return hdmi_err_fail;

    return hdmi_err_ok;
}
                             
/* Driver open function, initializes TWI to program HDMI registers,
   then initializes HDMI and I2S interface */
hdmi_err_t hdmi_open(hdmi_samp_freq_t freq, hdmi_num_chan_t num_channels)
{
    unsigned i;
    int num_lines;

    /* Check if driver is already open (only one instance can exist) */
    if (i2s_handle != NULL || hdmi_next_buffer != &hdmi_buffers[0])
        return hdmi_err_fail;

    /* Initialize TWI */
    if (init_twi() != hdmi_err_ok)
        return hdmi_err_fail;

    /* Initialize hdmi */
    /* Power up */
    if (hdmi_powerup() != hdmi_err_ok)
        return hdmi_err_fail;

    /* other initializations */
    if (hdmi_init() != hdmi_err_ok)
        return hdmi_err_fail;

    /* Video setup */
    if (hdmi_video_init() != hdmi_err_ok)
        return hdmi_err_fail;

    /* Audio setup, bit resolution for audio is hardcoded to 24 bits now.
     * In future it can be exposed to API.
     * Additionally, the speaker channel mapping for multichannel audio should
     * be configurable as well. */

    if (hdmi_audio_init(freq, num_channels, 24) != hdmi_err_ok)
        return hdmi_err_fail;

    /* Audio clock generation */
    if (audio_clk_gen_init(freq) != hdmi_err_ok)
        return hdmi_err_fail;

    /* num_lines is number of audio channels / 2 */
    num_lines = num_channels / 2;

    /*  Initialize I2S */
    if (init_i2s(num_lines) != hdmi_err_ok)
        return hdmi_err_fail;

    /* Allocate and populate transmit audio buffers */
    for (i = 0; i < HDMI_BUFS_PER_CHAN; i++)
    {
        i2s_buffers[i] = allocate_frame_buffer();
        populate_zeros(i2s_buffers[i]->buf);
    }

    /* Push some buffers to driver before enabling it
     * to avoid underflow */
    for (i = 0; i < HDMI_BUFS_PER_CHAN; i++)
    {
        sx_i2s_tx_put_full(i2s_handle, i2s_buffers[i]);
    }

    return hdmi_err_ok;
}

/* Driver close function */
void hdmi_close()
{
    sx_int32          i;
    sx_i2s_frame_buf* tmp_buf = NULL;

    /* Flush out last audio frames */
    for (i=0; i<HDMI_BUFS_PER_CHAN; i++)
    {
        while ((tmp_buf = sx_i2s_tx_get_empty(i2s_handle)) == NULL);
        free(tmp_buf);
    }

    /* Release HDMI buffers */
    hdmi_next_buffer = &hdmi_buffers[0];

    /* Release the I2S streams and clear driver init status */
    sx_i2s_stream_free(i2s_handle);
    i2s_handle = NULL;

    /* Power down HDMI device */
    hdmi_powerdown();

    /* Power down oscillators */
    write_clk_gen_reg(AUDIO_CLK_GENERATOR_TWI_ADDR, 
                    AUDIO_CLK_GENERATOR_POWER_UP_REG,0);
}

