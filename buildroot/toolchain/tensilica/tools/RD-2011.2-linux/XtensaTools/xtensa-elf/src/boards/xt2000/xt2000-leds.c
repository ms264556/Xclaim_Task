/* Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
/  These coded instructions, statements, and computer programs are the
/  copyrighted works and confidential proprietary information of Tensilica Inc.
/  They may not be modified, copied, reproduced, distributed, or disclosed to
/  third parties in any manner, medium, or form, in whole or in part, without
/  the prior written consent of Tensilica Inc.
*/

#include <xtensa/xt2000-led.h>
#include <xtensa/xt2000.h>
#include <xtensa/hal.h>


#ifdef XTBOARD_LED_VADDR
static unsigned int *first_led = (unsigned int*)(XTBOARD_LED_VADDR+0xe0);
#else
static unsigned int *first_led = (unsigned int *)0;
#endif


/*+*----------------------------------------------------------------------------
/ Function: xt2000_led_display_char
/
/ Description:  Sets the first character on the led display.  The remaining
/		elements on the LED will be blanked out.
/
/ Parameters: c -- The character to be
/
/ Returns: nothing.
/-**----------------------------------------------------------------------------*/

void xt2000_led_display_char( const char c )
{
  /* just call xt2000_display_string to do all the hard work */
  char str[2];

  str[0] = c;
  str[1] = 0;

  xt2000_led_display_string( str );

  return;
}
/*-*----------------------------------------------------------------------------*/



/*+*----------------------------------------------------------------------------
/ Function: xt2000_led_blank
/
/ Description: Blanks the led display
/
/ Parameters: none
/
/ Returns: nothing.
/-**----------------------------------------------------------------------------*/

void xt2000_led_blank( void )
{
  /* just call xt2000_display_string to do all the hard work */
  char str[1];

  str[0] = 0;

  xt2000_led_display_string( str );

  return;
}
/*-*----------------------------------------------------------------------------*/



/*+*----------------------------------------------------------------------------
/ Function: xt2000_led_display_ok
/
/ Description: Display an Ok on the led
/
/ Parameters: none
/
/ Returns: nothing.
/-**----------------------------------------------------------------------------*/

void xt2000_led_display_ok( void )
{
  /* just call xt2000_display_string to do all the hard work */
  char str[3];

  str[0] = 'O';
  str[1] = 'k';
  str[2] = 0;

  xt2000_led_display_string( str );

  return;
}
/*-*----------------------------------------------------------------------------*/



/*+*----------------------------------------------------------------------------
/ Function: xt2000_led_display_string
/
/ Description:  Sets the leds on the XT2000.  The first 8 characters in
/		the supplied string will be used, the remaining characters
/		will be ignored.
/
/ Parameters: str -- Pointer to the string to be displayed.
/
/ Returns: nothing.
/-**----------------------------------------------------------------------------*/

void xt2000_led_display_string(const char *str)
{
  int i = 0;

  if ( !str )
    goto exit_gracefully;

  /* Verify that the LED_VADDR is a valid address */
  if ( first_led == 0)
    goto exit_gracefully;

  for (i = 0; i < 8; ++i)
  {
    volatile char *pLed = (char *) &first_led[i];

    if ( *str == '\0' )
      {	
	// Set to a space, setting to Zero displays
	// a funny looking character.

	*pLed = ' ';
      }
    else
      {
	*pLed = *str;
	++str;
      }
  }

exit_gracefully:

  return;
}
/*-*----------------------------------------------------------------------------*/



/*+*----------------------------------------------------------------------------
/ Function: xt2000_led_pause
/
/ Description:  Pauses for a specified number of clock cycles
/
/ Parameters: ticks -- If ticks < 0, pause for XT2000_LED_PAUSECLKS
/
/ Returns: nothing.
/-**----------------------------------------------------------------------------*/

void xt2000_led_pause( int i )
{
    unsigned expiry = xthal_get_ccount() + (i <= 0 ? XT2000_LED_PAUSECLKS : i);
    while( (long)(expiry - xthal_get_ccount()) > 0 )
  	;
}
/*-*----------------------------------------------------------------------------*/

