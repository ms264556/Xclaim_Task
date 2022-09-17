/*
 * Copyright (c) 2001 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <xtensa/xt2000.h>
#include <xtensa/xt2000-led.h>

char nibble_to_hex[] = { '0', '1', '2', '3', 
			 '4', '5', '6', '7', 
			 '8', '9', 'a', 'b', 
			 'c', 'd', 'e', 'f' };

void hex_to_ascii(char *hexValue, const int value) 
{
  int i          = 0;
  int shiftvalue = 0;
  int use_blanks = 1;
  int nibble     = 0;

  for (i = 0; i < 8; ++i)
  {
    shiftvalue = 28 - (i << 2);
    nibble = (value >> shiftvalue) & 0xf;

    // This will turn leading zero's into blanks....
    if ( use_blanks )
      {
	if ( nibble == 0 )
	  {
	    hexValue[i] = ' ';
	    continue;
	  }
	use_blanks = 0;
      }

    hexValue[i] = nibble_to_hex[ (value >> shiftvalue) & 0xf ];
  }
  
  return;
}



main()
{
  int i = 0;
  int j = 0;
  int value = 0;
  char hexValue[9];

  xt2000_led_blank();

  for(;;)
  {
    hex_to_ascii(hexValue, value);
    hexValue[8] = 0;

    xt2000_led_display_string(hexValue);
    //    xt2000_led_pause(160000);

    ++value;
  }
}

