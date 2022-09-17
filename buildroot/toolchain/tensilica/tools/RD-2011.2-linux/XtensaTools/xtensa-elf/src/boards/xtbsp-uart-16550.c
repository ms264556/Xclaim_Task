/*******************************************************************************

Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

xtbsp-uart-16550.c  Part of board-support-package for boards using a 16550 UART.

Implements the XTBSP API UART functions for the National Semiconductor 165550.

*******************************************************************************/

#include <xtensa/xtbsp.h>               /* BSP API */
#include <xtensa/uart-16550-board.h>    /* UART and driver info */

#ifdef UART_16550_REGBASE

static uart_dev_t * const uart = (uart_dev_t *)UART_16550_REGBASE;

int xtbsp_uart_exists(void)
{
    return 1;
}

int xtbsp_uart_init(unsigned baud, unsigned ndata, 
                    unsigned parity, unsigned nstop)
{
    uart16550_init(uart, baud, ndata, parity, nstop);
    return 0;
}

char xtbsp_uart_getchar(void)
{
    return uart16550_in(uart);
}

void xtbsp_uart_putchar(const char c)
{
    uart16550_out(uart, c);
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
    #ifdef UART_16550_INTNUM
    /* all UART interrupts signal the same interrupt line to the CPU */
    return (mask & xtbsp_uart_int_all) ? UART_16550_INTNUM : -1;
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

