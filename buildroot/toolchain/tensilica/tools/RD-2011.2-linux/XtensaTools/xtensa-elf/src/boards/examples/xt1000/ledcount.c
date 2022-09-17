/*
 * Copyright (c) 2001 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include "leds.h"

main()
{
  int i, j;
  for(;;)
    {
      
      led_pause(16000);
      for( i = 0 ; i <= 0xf; i++)
	{
	  led_display_digit(i);
	  led_pause(160000);
	}
    }
}
