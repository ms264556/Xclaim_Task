/*******************************************************************************

Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xtav60-uart.c       Part of board-support-package for Avnet 60 board.

Implements the BSP UART functions using the XT2000-specific driver in uart.c.

NOTE:
This implementation "fits in around" the pre-existing XT2000 support in 
libxt2000.a. It merely adds the xtbsp-*() functions to libxt2000.a, leaving
pre-existing functions alone. No attempt has been made to produce a clean 
XT2000 support package like the XT-AV60 package, because XT2000 is deprecated
and only used internally.

The old XT2000 specific driver is required because some XT2000 specific 
legacy code calls it (xmon, VxWorks), so the newer board-independent driver 
in uart-16550.c is not used for XT2000.

*******************************************************************************/

#include <xtensa/xtbsp.h>               /* BSP API */
#include <xtensa/xt2000.h>              /* XT2000 board information */
#include <xtensa/xt2000-uart.h>         /* UART and driver info */

#ifdef DUART16552_1_VADDR

static uart_dev_t *uart = (uart_dev_t *)DUART16552_1_VADDR;

int xtbsp_uart_exists(void)
{
    return 1;
}

int xtbsp_uart_init(unsigned baud, unsigned ndata, 
                    unsigned parity, unsigned nstop)
{
    if (ndata == 8 && parity == 0 && nstop == 1) {
        uart_init(uart, baud);  /* always 8 data, no parity, 1 stop */
        return 0;
    }
    else return 1;
}

char xtbsp_uart_getchar(void)
{
    return uart_in(uart);
}

void xtbsp_uart_putchar(const char c)
{
    uart_out(uart, c);
}

int xtbsp_uart_get_isready(void)
{
    return RCVR_READY(uart);
}

int xtbsp_uart_put_isready(void)
{
    unsigned isr = _ISR(uart);  /* just to clear any Xmit interrupt */
    return XMIT_READY(uart);
}

xtbsp_uart_int xtbsp_uart_int_enable_status(void)
{
    unsigned ier = _IER(uart);

    return (ier & RCVR_DATA_REG_INTENABLE ? xtbsp_uart_int_rx : 0)
         | (ier & XMIT_HOLD_REG_INTENABLE ? xtbsp_uart_int_tx : 0);
}

void xtbsp_uart_int_enable(const xtbsp_uart_int mask)
{
    _IER(uart) = _IER(uart)
               | ((mask & xtbsp_uart_int_rx) ? RCVR_DATA_REG_INTENABLE : 0)
               | ((mask & xtbsp_uart_int_tx) ? XMIT_HOLD_REG_INTENABLE : 0);
}

void xtbsp_uart_int_disable(const xtbsp_uart_int mask)
{
    _IER(uart) = _IER(uart)
               &~ ((mask & xtbsp_uart_int_rx) ? RCVR_DATA_REG_INTENABLE : 0)
               &~ ((mask & xtbsp_uart_int_tx) ? XMIT_HOLD_REG_INTENABLE : 0);
}

int xtbsp_uart_int_number(const xtbsp_uart_int mask)
{
    #ifdef DUART16552_1_INTNUM
    /* all UART interrupts signal the same interrupt line to the CPU */
    return (mask & xtbsp_uart_int_all) ? DUART16552_1_INTNUM : -1;
    #else
    return -1;
    #endif
}

#else

int xtbsp_uart_exists(void)
{
    return 0;
}

int xtbsp_uart_init(unsigned baud, unsigned ndata, 
                    unsigned parity, unsigned nstop)
{
    return 0;
}

char xtbsp_uart_getchar(void)
{
    return (char)-1;
}

void xtbsp_uart_putchar(const char c)
{
}

int xtbsp_uart_get_isready(void)
{
    return 1;
}

int xtbsp_uart_put_isready(void)
{
    return 1;
}

xtbsp_uart_int xtbsp_uart_int_enable_status(void)
{
    return 0;
}

void xtbsp_uart_int_enable(const xtbsp_uart_int mask)
{
}

void xtbsp_uart_int_disable(const xtbsp_uart_int mask)
{
}

int xtbsp_uart_int_number(const xtbsp_uart_int mask)
{
    return -1;
}

#endif

