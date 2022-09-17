/*
 * Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

/* 
   timers.c

   This file does some basic timer and interrupt testing.
*/
#include <stdio.h>
#include <xtensa/config/core.h>

#define TIMER_COUNT 20000

/* global to communicate between interrupt handler and main task */
volatile int int_to_task = 0;

/* global to count any stack errors that may appear */
volatile int stack_errors = 0;

/* write a chunk of data */
static void write_data( int *array, int size, int seed )
{
    int *end = array + size;

    while( array < end )
    {
	*array++ = seed++;
    }
}

/* check a chunk of data has been written properly */
static int check_data( int *array, int size, int seed )
{
    int *end = array + size;

    while( array < end )
    {
	if( *array++ != seed++ )
	    return 1;
    }

    return 0;
}

/* to really test the interrupt stack, we need to make sure that
   we test overflows. To that end timer_interrupt_handler calls
   timer_force_overflow which calls recursively. */
void timer_force_overflow( int count )
{
    int data[4];
    int i;

    if( count )
    {
	write_data( data, 4, count );

	timer_force_overflow( count - 1 );

	check_data( data, 4, count );
    }
    else
    {
	/* actually process the interrupt */
	int_to_task++;
	_xtos_clear_ints( 1 << XCHAL_TIMER0_INTERRUPT );
	_xtos_timer_0_delta( TIMER_COUNT );
    }
}


void timer_interrupt_handler( )
{
    timer_force_overflow( 10 );
}

main()
{
    int last_int_to_task = -1;

    _xtos_set_interrupt_handler( XCHAL_TIMER0_INTERRUPT, timer_interrupt_handler );
    _xtos_ints_on( 1 << XCHAL_TIMER0_INTERRUPT );
    _xtos_timer_0_delta( TIMER_COUNT );

    for( ;int_to_task < 2000; )
    {
	if( last_int_to_task != int_to_task ) 
	{
	    printf( "int_to_task: %d\n", int_to_task );
	    last_int_to_task = int_to_task;
	}
    }
}
