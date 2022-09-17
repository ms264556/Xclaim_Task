/*
 *  uart.c  --  NatSemi PC16552D DUART utility routines for XT2000 board
 *
 * Copyright (c) 2002, 2005, 2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <xtensa/xt2000-uart.h>


/*  Setup UART for 8N1 at the specified speed in bits-per-second:  */
void uart_init( uart_dev_t *u, int bitrate )
{
#ifdef DUART16552_1_VADDR
    _LCR(u) = (DLAB_ENABLE|STOP_BIT_ENABLE|WORD_LENGTH(8));
    _DLL(u) = DUART_DIVISOR( DUART16552_XTAL_FREQ, bitrate );	/* 0x1e for 38400 bps at 18.432MHz */
    _DLM(u) = 0x0;
    _LCR(u) = (STOP_BIT_ENABLE|WORD_LENGTH(8));
//  _IER(u) = RCVR_STATUS_INTENABLE;	/* enable break interrupts */
    _IER(u) = 0;
    _MCR(u) = (_DTR|_RTS);
    _FCR(u) = _FIFO_ENABLE;
#endif
}

/*  Output char to UART, waiting for transmitter to be ready if necessary:  */
void uart_out( uart_dev_t *u, char c )
{
#ifdef DUART16552_1_VADDR
    while(!XMIT_READY(u))
	;
    _TXB(u) = c;
#endif
}

/*  Return next input char from UART, waiting for one if none available:  */
char uart_in( uart_dev_t *u )
{
#ifdef DUART16552_1_VADDR
    while(!RCVR_READY(u))
	;
    return _RXB(u);
#else
    /* while(1) ; */
    return (char)-1;
#endif
}

/*  Enable Rx interrupts:  */
void uart_enable_rcvr_int( uart_dev_t *u )
{
#ifdef DUART16552_1_VADDR
    _IER(u) |= RCVR_DATA_REG_INTENABLE;
#endif
}

/*  Disable Rx interrupts:  */
void uart_disable_rcvr_int( uart_dev_t *u )
{
#ifdef DUART16552_1_VADDR
    _IER(u) &= ~RCVR_DATA_REG_INTENABLE;
#endif
}


/*  Some convenience utility functions:  */

static inline void uart_out_crlf( uart_dev_t *u, char c )
{
    /*  Do LF to CR-LF conversion here:  */
    if( c == '\n' )		/* LF? */
  	uart_out( u, '\r' );	/* output CR first */
    uart_out( u, c );
}

void uart_puts( uart_dev_t *u, char *s )
{
#ifdef DUART16552_1_VADDR
    while( *s != 0 )
  	uart_out_crlf( u, *s++ );
#endif
}


