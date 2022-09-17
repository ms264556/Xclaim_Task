/*******************************************************************************

Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xthifi2-board.c     Part of board-support-package for XT-HiFi2 board.

Implementation of BSP initialization, characteristics and convenience functions.

*******************************************************************************/

#include <xtensa/xtbsp.h>
#include <xtensa/xthifi2.h>
#include <xtensa/xthifi2-sx-globalreg.h>
#include <xtensa/uart-16550-board.h>

/* Pre-computed values of run-time constants to speed run-time computation. */
static unsigned _clock_freq_hz;
static unsigned _clock_period_ps;
static unsigned _delay_scale_factor;


/* API Functions */

void xtbsp_board_init(void)
{
    uart_dev_t * const uart = (uart_dev_t *)UART_16550_REGBASE;

    /*
    Compute and cache clock frequency (measured with the UART) and period.
    We only want to do this once, at initialization.
    */
    _clock_freq_hz = uart16550_measure_sys_clk(uart);

    /*
    To fit the ps period computation into 32-bit integer arithmetic, use:
        _clock_period_ps = (1000000000 / _clock_freq_hz) * 1000;
    To retain good precision at freq approx 300 MHz and avoid multiply,
    scale 10^9 numerator by 1000/(2^8) and denominator by 2^8 (shift).
    */
    _clock_period_ps = (3906250000U / (_clock_freq_hz >> 8));

    /*
    Compute, round and cache the scale factor for use in xtbsp_delay_ns().
    We only want to do the run-time divide once, at initialization.
    There is a tradeoff between precision and granularity here.
    This yields a precision of +/- 0.7% at 300MHz, 256ns granularity.
    */
    #define DIV (1000000000U >> 8)
    _delay_scale_factor = (_clock_freq_hz + (DIV >> 2)) / DIV;
    #undef DIV

    /* (Re)Initialize UART */
    xtbsp_uart_init_default();

    /* Init display last because it requires a long busy-wait. */
    xtbsp_display_init();
}

const char * xtbsp_board_name(void)
{
    return "XT-HiFi2";
}

void xtbsp_board_reset(void)
{
    volatile sx_global_registers *greg = SX_GLOBAL_REGISTERS;

    /* NMI timer counts down to 1, then reset timer counts down to 0 */
    greg->reset_timer   = 2;
    greg->nmi_timer     = 2;
}

unsigned xtbsp_clock_freq_hz(void)
{
    return _clock_freq_hz;
}

unsigned xtbsp_clock_period_ps(void)
{
    return _clock_period_ps;
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
    There is a tradeoff between precision and granularity here.
    The cached scale factor has 1.3% precision and yields 256ns granularity.
    The ns parameter can represent over 4s (the lower 7 bits are ignored).
    */
    xtbsp_delay_cycles((ns >> 8) * _delay_scale_factor);
}

