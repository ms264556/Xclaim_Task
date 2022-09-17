/*********************************************************************************/
/*                                                                               */
/* Copyright (c) 2009 by Tensilica Inc.  ALL RIGHTS RESERVED.                    */
/* These coded instructions, statements, and computer programs are the           */
/* copyrighted works and confidential proprietary information of Tensilica Inc.  */
/* They may not be modified, copied, reproduced, distributed, or disclosed to    */
/* third parties in any manner, medium, or form, in whole or in part, without    */
/* the prior written consent of Tensilica Inc.                                   */
/*                                                                               */
/*********************************************************************************/

/*
This example records a short sond clip using the microphone jack and then 
outputs the recording to the audio output jack on the XT-HiFi2 board.
IMPORTANT NOTE: THIS EXAMPLE ONLY WORKS WITH A SELF POWERED MICROPHONE.
*/

#include <xtensa/xthifi2.h>
#include <xtensa/xthifi2-aic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(XTBOARD_RAM_VADDR) || (XTBOARD_RAM_SIZE < 0x480000)
int main(int argc, char *arv[])
{
  xtbsp_display_string("SysRAM too small");
  printf("This example requires at least 4.5 MB of system RAM.\n");
  return 0;
}
#else
#define NUMBER_OF_FRAMES    (1000)
static char one_frame[AIC_FRAME_SIZE];
static char data[NUMBER_OF_FRAMES][AIC_FRAME_SIZE];

int main(int argc, char *arv[])
{
    int i;
    char ans;

    /* Open AIC driver, sampling freq = 48KHz */
    if (aic_open(aic_samp_freq_48k) != aic_err_ok)
    {
        return 1;
    }
    aic_set_volume(-2); // start out at -2 dB
    aic_set_mute(0);    // make sure it's not muted

    printf("\n\n");
    printf("Welcome to the Tensilica ADC Recorder Test\n");
    printf("This application will record a short clip\n");
    printf("and then play it out on the headphone output\n");

    printf("\nHit 'Enter' to begin recording\n");
    scanf ("%c", &ans);
#pragma flush

    printf ("\nRecording a short clip");
    fflush(stdout);

    /* Record and copy to data buffer */
    for(i = 0; i < NUMBER_OF_FRAMES; i++)   {
        aic_read((int *)one_frame);
        memcpy(data[i], one_frame, AIC_FRAME_SIZE);
    }

    printf("\n\nHit 'Enter' to begin playback\n");
    scanf ("%c", &ans);

    printf ("\nPlaying back the recorded clip\n");
    fflush(stdout);

    /* Playback */
    for(i = 0; i < NUMBER_OF_FRAMES; i++)   {
        aic_write((int *)data[i]);
    }

    printf("\nPlayback Complete!\n");

    /* Close driver */
    aic_close();
}

#endif /* XTBOARD_RAM_VADDR */

