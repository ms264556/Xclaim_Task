/*
 *  leds.c  --  XT1000 16-segment LED utility routines
 *
 *  Copyright 2002 Tensilica Inc.
 */

#include <xtensa/xt1000-led.h>


/*
 *  Current LED reg contents, if written only via XT1000_SETLED() macro.
 */
volatile unsigned _led_shadow;


/*
 *  Include perl-generated pattern arrays:
 *	const unsigned short _LedSegments[];
 *	const unsigned short _LedRotate[];
 */
#include "ledpats.c"




/***********************   Basic LED routines   ****************************/

void led_display_char( char c )
{
    if( c >= 32 && c <= 127 )	/* visible char? */
    	XT1000_SETLED( _LedSegments[c - 32] );
    else
	XT1000_SETLED( XT1000_LED_UNKNOWN );
}

void led_display_digit( int d )
{
    static const unsigned char LedHex[] = "0123456789AbcdEF";
    if( 0 <= d && d <= 0xF )
	led_display_char( LedHex[d] );
    else
        XT1000_SETLED( XT1000_LED_BLANK );
}

void led_blank( void )
{
    XT1000_SETLED( XT1000_LED_BLANK );
}


/***********************   Paused LED routines   ****************************/

void led_pause( int i )
{
    extern unsigned get_ccount(void);

    unsigned expiry = get_ccount() + (i <= 0 ? XT1000_LED_PAUSECLKS : i);
    while( (long)(expiry - get_ccount()) > 0 )
  	;
}

void led_display_string( const char *s )
{
    while( *s != 0 ) {
	led_display_char( ' ' );
	led_pause( XT1000_LED_PAUSECLKS/2 );
	led_display_char( *s++ );
	led_pause( XT1000_LED_PAUSECLKS*5 );
    }
}

void led_display_sequence( const short *seq )
{
    XT1000_SETLED( XT1000_LED_BLANK );
    while( *seq ) {
        XT1000_SETLED( *seq++ );
        led_pause(-1);
    }
}

void led_display_ok( void )
{
    int i;
    for(i=0;i<3;i++) {
	led_display_string( "oK" );
        led_pause(-1);
    }
}

