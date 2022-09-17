/* xtfreq.c - Measure the system clock speed from the UART crystal */
/*
 * Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */                                                                             
#include <xtensa/xt2000-uart.h>

static void ignore(unsigned t) { t; }

/*
 *  Before we initialize the UART, use it to measure system clock speed.
 *  The XT2000's UART crystal should always be 18.432 MHz, but the
 *  system clock can vary widely.  So we use the UART crystal as
 *  a reference point to measure system clock rate.
 *
 *  The 16552 DUART is put temporarily in loopback mode so we can measure
 *  character transmission delay without outputting junk on the console.
 */
unsigned xtboard_measure_sys_clk(void)
{
	register uart_dev_t *u = &DUART_1_BASE;
	unsigned tstart, tend, tperiod;

	/*  Divisor for 100bps loopback speed (turns out to be 11520):  */
	#define DIV100BPS       DUART_DIVISOR(DUART16552_XTAL_FREQ, 100)

	_LCR(u) = DLAB_ENABLE;      /* DLAB=1 on UART1 at least */
	_DLL(u) = (DIV100BPS & 0xFF);       /* set baudrate gen. divider LSB */
	_DLM(u) = (DIV100BPS >> 8);         /* set baudrate gen. divider MSB */
	_AFR(u) = AFR_CONC_WRITE;   /* enable concurrent writes to both ports */
	_MCR(u) = LOOP_BACK;                /* enable loopback mode, external pins idled */
	_LCR(u) = 0;                        /* DLAB=0 on both ports */
	_IER(u) = 0;                        /* disable interrupts */
	_FCR(u) = (RCVR_FIFO_RESET | XMIT_FIFO_RESET);
	_FCR(u) = _FIFO_ENABLE;
	_LCR(u) = DLAB_ENABLE;      /* DLAB=1 on both ports */
	/*_AFR(u) = AFR_CONC_WRITE;*/ /* again for UART2 */
	_AFR(u) = 0;                        /* turn off concurrent writes */
	_LCR(u) = WORD_LENGTH(8);   /* DLAB=0, 8N1 (10 bits total per char) */
	/*
	 *  Send two characters at 10 chars/sec.  The first lets us sync up with
	 *  the UART clock, and we then measure transmission time of the second
	 *  character (time from receipt of the 1st to that of the 2nd character).
	 */
	_TXB(u) = '@';
	_TXB(u) = '@';
	while (!RCVR_READY(u));
	tstart = xthal_get_ccount();
	ignore(_RXB(u));                /* clear Rx ready status */
	while (!RCVR_READY(u));
	tend = xthal_get_ccount();
	ignore(_RXB(u));                /* clear Rx ready status */
	tperiod = (tend - tstart)*10;
	tperiod += 5000;                /* Round to nearest .01 MHZ */
	tperiod = tperiod / 10000;
	tperiod = tperiod * 10000;
	return tperiod;
}
 
