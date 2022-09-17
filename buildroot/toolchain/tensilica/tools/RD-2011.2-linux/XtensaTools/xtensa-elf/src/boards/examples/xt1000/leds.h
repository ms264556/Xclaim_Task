/*
 * Copyright (c) 2001 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <xtensa/xt1000.h>

#define LED_ADDR	((short*)XT1000_LED_VADDR)
#define LED_BLANK 0xffff
#define PAUSE_ITERATIONS 4000

void led_display_digit(int d);
void led_pause();

