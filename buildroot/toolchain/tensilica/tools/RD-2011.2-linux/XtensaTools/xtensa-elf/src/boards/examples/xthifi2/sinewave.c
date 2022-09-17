/* Copyright (c) 2004-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <xtensa/xtbsp.h>
#include <xtensa/xthifi2-aic.h>

#ifndef XTBOARD_RAM_VADDR
int main(int argc, char *arv[]) {
  xtbsp_display_string("No system RAM");
  printf("With no system RAM configured, this example is too big.\n");
  return 0;
}
#else

/* This example for XT-HiFi2 board plays a 1 kHz sinewave using the AIC 
 * device on the daughter card. Optionally accepts single char commands
 * via UART to alter volume and mute. This is a sanity test for the AIC.
 * FIXME: Currently this example will not work with the daughter card as 
 * the AIC is not fully functional, but the example has been tested with 
 * Stretch's daughter card which has the same AIC device. */

/* One sine wave cycle (48 L+R 16-bit samples, byte-reversed). */
static short sinebuf[] = {
    0x0000, 0x0000, 0xb410, 0xb410, 0x2021, 0x2021, 0xfb30, 0xfb30,
    0xff3f, 0xff3f, 0xeb4d, 0xeb4d, 0x815a, 0x815a, 0x8b65, 0x8b65,
    0xd96e, 0xd96e, 0x4076, 0x4076, 0xa27b, 0xa27b, 0xe67e, 0xe67e,
    0xff7f, 0xff7f, 0xe67e, 0xe67e, 0xa27b, 0xa27b, 0x4076, 0x4076,
    0xd96e, 0xd96e, 0x8b65, 0x8b65, 0x815a, 0x815a, 0xeb4d, 0xeb4d,
    0xff3f, 0xff3f, 0xfb30, 0xfb30, 0x2021, 0x2021, 0xb410, 0xb410,
    0x0000, 0x0000, 0x4cef, 0x4cef, 0xe0de, 0xe0de, 0x05cf, 0x05cf,
    0x01c0, 0x01c0, 0x15b2, 0x15b2, 0x7fa5, 0x7fa5, 0x759a, 0x759a,
    0x2791, 0x2791, 0xc089, 0xc089, 0x5e84, 0x5e84, 0x1a81, 0x1a81,
    0x0180, 0x0180, 0x1a81, 0x1a81, 0x5e84, 0x5e84, 0xc089, 0xc089,
    0x2791, 0x2791, 0x759a, 0x759a, 0x7fa5, 0x7fa5, 0x15b2, 0x15b2,
    0x01c0, 0x01c0, 0x05cf, 0x05cf, 0xe0de, 0xe0de, 0x4cef, 0x4cef
};
static unsigned sinelen = sizeof(sinebuf) / sizeof(sinebuf[0]);

/* Play a number of cycles of a sinewave on the AIC DAC (0 = forever). */
static void play_audio(unsigned cycles)
{
    static int sinepos = 0;
    int frame_buffer[AIC_FRAME_SAMPLES];
    int done = 0;
    unsigned i;
    char c;
    aic_err_t err;

    aic_set_volume(-6); // start out at -6 dB
    aic_set_mute(0);    // make sure it's not muted

    while (!done)
    {
        /* Fill an AIC frame buffer with sinewave samples (pad at end) */
        for (i = 0; i < AIC_FRAME_SAMPLES; i++)
        {
            frame_buffer[i] = done ? 0 : (sinebuf[sinepos]) << 16;
            if (++sinepos >= sinelen)
            {
                sinepos = 0;
                if (cycles != 0 && --cycles == 0)
                {
                    done = 1;
                }
            }
        }

        /* Write AIC frame to device */
        if (aic_write(frame_buffer) != aic_err_ok)
        {
            printf("AIC write failed.\n");
            aic_close();
            exit(1);
        }

        /* Listen to UART for single char optional commands */
        if (xtbsp_uart_get_isready())
        {
            c = xtbsp_uart_getchar();
            err = aic_err_ok;
            switch (c)
            {
            case '0':   /* mute */
                err = aic_set_mute(1);
                break;
            case '1':   /* unmute */
                err = aic_set_mute(0);
                break;
            case '+':   /* increase volume by ~ 3dB */
                err = aic_set_volume(aic_volume() + 3);
                break;
            case '-':   /* decrease volume by ~ 3dB */
                err = aic_set_volume(aic_volume() - 3);
                break;
            case '\x1b': /* ESC: stop */
                done = 1;
                break;
            case '?':   /* ?: help */
                printf("+/- (volume), 0/1 (mute/unmute), ESC (stop)\n");
                break;
            }
            if (err != aic_err_ok)
            {
                puts("AIC driver call failed.\n");
                continue;
            }
            printf("Vol = %4d dB%s\n", 
                    aic_volume(), 
                    aic_is_muted() ? ", muted" : ""
                    );
            fflush(stdout);
        }
    }
}

/* Main */
int main(int argc, char *argv[])
{
    int i;
    unsigned char data;

    /* Open AIC driver, sampling freq = 48KHz */
    printf("\nOpening audio device...\n");
    if (aic_open(aic_samp_freq_48k) != aic_err_ok)
    {
        return 1;
    }

    /* Play audio */
    printf("Playing 1KHz sine wave...\n");
    play_audio(0);

    /* Close driver */
    aic_close();

    printf("Done.\n\n");
    return 0;
}

#endif // XTBOARD_RAM_VADDR

