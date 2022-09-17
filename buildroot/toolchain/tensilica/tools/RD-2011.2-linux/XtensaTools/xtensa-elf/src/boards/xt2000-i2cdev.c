/* xt2000-i2cdev.c - I2C driver using the XT2000's V3 chip */
/*
 * Copyright (c) 2002-2005 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <xtensa/xt2000.h>
#include <xtensa/xtboard.h>


#if XCHAL_MEMORY_ORDER == XTHAL_BIGENDIAN
#define DEFAULT_BASE XTBOARD_V3PCI_PADDR+(0x73^3)
#else
#define DEFAULT_BASE XTBOARD_V3PCI_PADDR+(0x73)
#endif

#define V3USC_SYSTEM_B          *(volatile unsigned char *)(DEFAULT_BASE)
#define V3USC_SYS_OUT(__byte__) V3USC_SYSTEM_B = (__byte__)
#define V3USC_SYS_IN()          V3USC_SYSTEM_B

#define SYSTEM_B_SPROM_EN       0x01    /* 1 - Software control         */
                                        /* 0 - Hardware control         */
#define SYSTEM_B_SDA_IN         0x02    /* Serial EEPROM data input     */
#define SYSTEM_B_SDA_IN_SHIFT   1
#define SYSTEM_B_SDA_OUT        0x04    /* Serial EEPROM data output    */
                                        /* SPROM_EN must be enabled     */
#define SYSTEM_B_SCL            0x08    /* Serial EEPROM clock output   */
#define SYSTEM_B_SCL_IN_SHIFT   3
#define SYSTEM_B_LOCK		0x40	/* Lock Register Contents set	*/ 
#define SYSTEM_B_UNLOCK_TOKEN   0xa5

#define XTBOARD_I2C_DELAYNS	50000	/* I2C delay in nanosecs (50us) */

static XtboardDelayFunc *xtboard_nsdelay_func = 0;
/*static void (*bit_delay)( void );*/

#define I2C_BIT_DELAY		(*xtboard_nsdelay_func)( XTBOARD_I2C_DELAYNS )


/************************   Basic signal bit operations:   ************************/

static char i2c_scl, i2c_sda;	/* last value of SCL (clock) and SDA (data) bits sent */

static void i2c_init(void)
{
    i2c_scl = 2;
    i2c_sda = 2;
}

static void i2c_setscl(int state)
{
    /*if( state == i2c_scl )
	return;*/			/* no change */
    i2c_scl = state;
    if (state)
	V3USC_SYS_OUT(V3USC_SYS_IN() | SYSTEM_B_SCL);
    else
	V3USC_SYS_OUT(V3USC_SYS_IN() & ~SYSTEM_B_SCL);
    I2C_BIT_DELAY;
}

static void i2c_setsda(int state)
{
    /*if( state == i2c_sda )
	return;*/			/* no change */
    i2c_sda = state;
    if (state)
	V3USC_SYS_OUT(V3USC_SYS_IN() | SYSTEM_B_SDA_OUT);
    else
	V3USC_SYS_OUT(V3USC_SYS_IN() & ~SYSTEM_B_SDA_OUT);
    I2C_BIT_DELAY;
}

static int i2c_getsda()
{
    return ((V3USC_SYS_IN() >> SYSTEM_B_SDA_IN_SHIFT) & 1);
}


/************************   Basic I2C bit-framing operations:   ************************/

static void i2c_start()
{
    i2c_setsda(1);
    i2c_setscl(1);
    i2c_setsda(0);
    i2c_setscl(0);
}

static void i2c_stop()
{
    i2c_setscl(0);
    i2c_setsda(0);
    i2c_setscl(1);
    i2c_setsda(1);
}

/*
 * Shift a data bit in and out.
 * ASSUMPTION on entry:  SCL line is low (0).
 *
 * NOTES:  The I2C bus is multimaster.  Its signals have pull-up resistors,
 * and are pulled down by master & slave devices using common-emitter circuits.
 * So if 'out' is zero, we necessarily should read and return zero (unless
 * something's really wrong with the circuitry).
 * If 'out' is set, we can read whatever another device is asserting on the
 * data line (or 1 if no device is asserting anything).
 */
static int i2c_databit( int out )
{
    int in;
    i2c_setsda(out);
    i2c_setscl(1);
    in = i2c_getsda();
    i2c_setscl(0);
    return in;
}


/************************   Derived (byte-wide) operations:   ************************/

/*
 *  Send a byte and issue an ACK cycle.
 *  Returns 0 if addressed device acknowledged the cycle, 1 otherwise.
 */
static unsigned i2c_send_byte(unsigned char byte)
{
    int i;
    for (i=0x80; i>0; i>>=1)
	i2c_databit( (byte & i) != 0 );
    return i2c_databit( 1 );
}

/*
 *  Read a byte (doesn't issue the ACK cycle).
 */
static unsigned char i2c_recv_byte()
{
    int i;
    unsigned char byte = 0;
    for (i = 8; i; i--)
	byte = (byte << 1) | i2c_databit( 1 );
    return byte;
}


/************************   I2C transaction operations:   ************************/

/*
 *  Set the nanosecond delay function needed for I2C operations.
 *  The function provided takes a single unsigned integer parameter
 *  which is an amount of time to delay in nanoseconds.
 *  This function may busywait (or use any other OS-specific
 *  mechanism) to implement the delay; it is typically invoked
 *  with very small delays (eg. 10us) so busywaiting is usually okay.
 *
 *  Returns the previous delay function registered (NULL if none yet registered).
 *
 *  Must be called prior to calling xtboard_i2c_{read,write}
 *  otherwise these functions will return an error.
 */
XtboardDelayFunc* xtboard_set_nsdelay_func( XtboardDelayFunc *delay_fn )
{
    XtboardDelayFunc *prev = xtboard_nsdelay_func;
    xtboard_nsdelay_func = delay_fn;
    return prev;
}

/*
 *  Read a sequence of bytes.
 *  (Also writes an address first.)
 *  Does not access the device at all (returns 0) if 'size' is zero.
 *
 *  Returns:
 *	0	successful (or size == 0)
 *	1	selected device (id) did not respond
 *	2	nanosecond delay function not set
 *
 *  NOTE:  this function is non-reentrant by nature,
 *  because it has no provisions to allow multiple threads
 *  to access the i2c port at the same time.
 */
int xtboard_i2c_read(unsigned id, unsigned char *buf,
	unsigned addr, unsigned size)
{
    int i, result;
    char system;

    if( xtboard_nsdelay_func == 0 )
	return( 2 );

    if( size == 0 )
	return( 0 );

    i2c_init();
    V3USC_SYS_OUT(SYSTEM_B_UNLOCK_TOKEN);
    V3USC_SYS_OUT(V3USC_SYS_IN() | SYSTEM_B_SPROM_EN);

    /*  Set the address:  */
    i2c_start();
    result = i2c_send_byte((id << 1) | (0));	/* select device, initiate write */
    if( result == 0 ) {			/* if device acknowledged... */
	i2c_send_byte(addr);		/* set address */

	/*  Get the bytes:  */
	i2c_start();
	i2c_send_byte((id << 1) | (1));	/* select device, initiate read */
	for(i=0;; ) {
	    buf[i] = i2c_recv_byte();
	    if( ++i >= size )
		break;
	    i2c_databit(0);		/* ACK */
	}
	i2c_databit(1);			/* NACK */
    }
    i2c_stop();

    system = V3USC_SYS_IN() & ~ SYSTEM_B_SPROM_EN;
    V3USC_SYS_OUT(system);
    V3USC_SYS_OUT(system | SYSTEM_B_LOCK);
    return( result );
}

/*
 *  Write a sequence of bytes.
 *  'size' may be zero (eg. for write completion polling;
 *  the device is selected and an address byte also sent,
 *  but no other [ie. data] bytes are written).
 *
 *  Returns:
 *	0	successful
 *	1	selected device (id) did not respond
 *	2	nanosecond delay function not set
 *
 *  NOTE:  this function is non-reentrant by nature,
 *  because it has no provisions to allow multiple threads
 *  to access the i2c port at the same time.
 */
int  xtboard_i2c_write(unsigned id, unsigned char *buf,
	unsigned addr, unsigned size)
{
    int i, result;
    char system;

    if( xtboard_nsdelay_func == 0 )
	return( 2 );

    i2c_init();
    V3USC_SYS_OUT(SYSTEM_B_UNLOCK_TOKEN);
    V3USC_SYS_OUT(V3USC_SYS_IN() | SYSTEM_B_SPROM_EN);

    i2c_start();
    result = i2c_send_byte((id << 1) | (0));	/* select device, initiate write */
    if( result == 0 ) {			/* if device acknowledged... */
	result = i2c_send_byte(addr);
	for( i = 0; result == 0 && i < size; i++ )
	    result = i2c_send_byte(buf[i]);
    }
    i2c_stop();

    system = V3USC_SYS_IN() & ~ SYSTEM_B_SPROM_EN;
    V3USC_SYS_OUT(system);
    V3USC_SYS_OUT(system | SYSTEM_B_LOCK);
    return( result );
}

/*
 *  Waits for ACK from serial EEPROM (also called "nvram")
 *
 *  Returns:
 *	0	timeout
 *   != 0	Ok, get ACK
 *
 *  NOTE:  this function is non-reentrant by nature,
 *  because it has no provisions to allow multiple threads
 *  to access the i2c port at the same time.
 */
int                                           // = 0 - timeout
xtboard_i2c_wait_nvram_ack(unsigned id, unsigned swtimer)
{
   int result = 1;
   char system;

   i2c_init();

   V3USC_SYS_OUT(SYSTEM_B_UNLOCK_TOKEN);
   V3USC_SYS_OUT(V3USC_SYS_IN() | SYSTEM_B_SPROM_EN);

   while (swtimer && result)
   {
      swtimer--;
      i2c_start();
      result = i2c_send_byte((id << 1) | (0));
      i2c_stop();
   }

   system = V3USC_SYS_IN() & ~ SYSTEM_B_SPROM_EN;
   V3USC_SYS_OUT(system);
   V3USC_SYS_OUT(system | SYSTEM_B_LOCK);

   return swtimer;
}

