/*
 * Copyright (c) 2001 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include "leds.h"

static short segments[] =
{
~((short)((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7))), /*0*/
~((short)((1<<2)|(1<<3))), /*1*/
~((short)((1<<0)|(1<<1)|(1<<2)|(1<<11)|(1<<15)|(1<<6)|(1<<5)|(1<<4))), /*2*/
~((short)((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<11)|(1<<15)|(1<<5)|(1<<4))), /*3*/
~((short)((1<<7)|(1<<15)|(1<<11)|(1<<2)|(1<<3))), /*4*/
~((short)((1<<0)|(1<<1)|(1<<7)|(1<<15)|(1<<11)|(1<<3)|(1<<5)|(1<<4))), /*5*/
~((short)((1<<0)|(1<<1)|(1<<7)|(1<<15)|(1<<11)|(1<<6)|(1<<3)|(1<<5)|(1<<4))), /*6*/
~((short)((1<<0)|(1<<1)|(1<<2)|(1<<3))), /*7*/
~((short)((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<15)|(1<<11))), /*8*/
~((short)((1<<0)|(1<<1)|(1<<2)|(1<<11)|(1<<15)|(1<<7)|(1<<3)|(1<<4)|(1<<5))), /*9*/
~((short)((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<6)|(1<<7)|(1<<11)|(1<<15))), /*a*/
~((short)((1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<11)|(1<<15))), /*b*/
~((short)((1<<15)|(1<<11)|(1<<6)|(1<<5)|(1<<4))), /*c*/
~((short)((1<<2)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<11)|(1<<15))), /*d*/
~((short)((1<<0)|(1<<1)|(1<<15)|(1<<11)|(1<<5)|(1<<4)|(1<<7)|(1<<6))), /*e*/
~((short)((1<<0)|(1<<1)|(1<<15)|(1<<11)|(1<<7)|(1<<6))), /*f*/
};

void led_display_digit(int d)
{
  short *led = LED_ADDR;
  *led = segments[0x0f&d];
}

void led_pause( int i )
{
  i = i <= 0 ? PAUSE_ITERATIONS : i;
  while( i > 0 ) i--;
}

void led_blank()
{
  short *led = LED_ADDR;
  *led = LED_BLANK;
}
