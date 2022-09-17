/*******************************************************************************

Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xt2000-board.c      Part of board-support-package for XT2000 board.

Implementation of BSP initialization, characteristics and convenience functions.

NOTE:
This implementation "fits in around" the pre-existing XT2000 support in 
libxt2000.a. It merely adds the xtbsp-*() functions to libxt2000.a, leaving
pre-existing functions alone. No attempt has been made to produce a clean 
XT2000 support package like the XT-AV60 package, because XT2000 is deprecated
and only used internally.

*******************************************************************************/

#include <xtensa/xtbsp.h>           /* Function descriptions are in here */
#include <xtensa/xt2000.h>          /* XT2000 board information */

/* Pre-computed values of run-time constants to speed run-time computation. */
static unsigned clock_freq_hz;
static unsigned clock_period_ps;
static unsigned delay_scale_factor;


void xtbsp_board_init(void)
{
    /* Pre-compute run-time constants to avoid run-time overhead. */
    clock_freq_hz = xtboard_measure_sys_clk();    /* uses UART */
    clock_period_ps = (4000000000U / clock_freq_hz) * 250;
    delay_scale_factor = clock_freq_hz / (1000000000U/(1<<10));

    /* Initialize display. */
    xtbsp_display_init();

    /*
    The UARTs are initialized by board_init(), located in uart.c (not gloss.c).
    They must be (re)initialized AFTER calling xtboard_measure_sys_clk() above.
    Note, the old XT2000-specific DUART driver is used for legacy reasons.
    */
}

const char * xtbsp_board_name(void)
{
    return "XT2000";
}

void xtbsp_board_reset(void)
{
    #ifdef XT2000_SWRST_REG
    XT2000_SWRST_REG = XT2000_SWRST_RESETVALUE;
    #endif
}

unsigned xtbsp_clock_freq_hz(void)
{
    return clock_freq_hz;
}

unsigned xtbsp_clock_period_ps(void)
{
    return clock_period_ps;
}

void xtbsp_delay_cycles(unsigned cycles)
{
#if XCHAL_HAVE_CCOUNT
    unsigned expiry = xthal_get_ccount() + cycles;
    while( (long)(expiry - xthal_get_ccount()) > 0 );
#else
    /*
    Approximate the cycle count by a loop iteration count. 
    Clearly this is highly dependent on config and optimization.
    */
    volatile unsigned i;
    for (i = cycles>>3; i > 0; --i);
#endif
}

void xtbsp_delay_ns(unsigned ns)
{
    /*
    Scale for clock freq as low as ~1MHz, giving granularity ~1us.
    Compile-time integer divide yields 0 cycles if freq < 977 kHz.
    Shift right (integer divide by pwr of 2) yields 0 cycles if ns < 1024.
    Supports full 32-bit range of ns specifications at freq up to 1 GHz.
    */
    xtbsp_delay_cycles((ns>>10) * delay_scale_factor);
}

