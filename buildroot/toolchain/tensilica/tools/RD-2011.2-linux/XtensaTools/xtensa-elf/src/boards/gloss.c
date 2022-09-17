/*  gloss.c  -  glue functions for libgloss  */

/* $Id: //depot/rel/Cottonwood/Xtensa/OS/boards/gloss.c#1 $ */

/*
 * Copyright (c) 2002,2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <xtensa/xtbsp.h>
#include <xtensa/xt2000-uart.h>


/*  Board init for XT2000: setup serial port P1 at 38400 bps  */
void board_init(void)
{
  xtbsp_board_init();   /* must call before UARTS are initialized */

  uart_init( &DUART_1_BASE, 38400 );
}

void outbyte( int c )
{
  uart_out( &DUART_1_BASE, c );
}

int inbyte( void )
{
  return (unsigned char)uart_in( &DUART_1_BASE );
}

