/* xtboard.c - XT2000 functions for NVRAM and RTC (uses I2C driver) */
/*
 * Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <xtensa/xt2000.h>
#include <xtensa/xtboard.h>

#include "crc16.c"


/******************   RTC   *******************/

unsigned xtboard_get_rtc_time(void)
{
    unsigned char buf[8];
    if( xtboard_i2c_read(XT2000_I2C_RTC_ID, buf, 0, 6) )
	return XTBOARD_RTC_ERROR;
    if (buf[4]&0x80)
	return XTBOARD_RTC_STOPPED;
    return (buf[3]<<24)+(buf[2]<<16)+(buf[1]<<8)+buf[0];
}

int xtboard_set_rtc_time(unsigned time)
{
    unsigned char buf[8];
    buf[0] = time;
    buf[1] = (time>>8);
    buf[2] = (time>>16);
    buf[3] = (time>>24);
    buf[4] = 0;		/* enable oscillator */
    buf[5] = 0;		/* disable trickle charger */
    return xtboard_i2c_write(XT2000_I2C_RTC_ID, buf, 0, 6);
}


/******************   NVRAM   *******************/

/*
 *  Read data from NVRAM.
 *
 *  We can read a whole 256-byte block at a time.
 */
int xtboard_nvram_read(unsigned addr, unsigned len, unsigned char *buf)
{
    int error, maxlen;

    if( addr > XT2000_NVRAM_SIZE || len > XT2000_NVRAM_SIZE || addr+len > XT2000_NVRAM_SIZE )
	return 3;
    if( len == 0 )
	return 0;
    maxlen = 256 - addr;
    if( maxlen > 0 ) {
	if( maxlen > len )
	    maxlen = len;
	if( error = xtboard_i2c_read(XT2000_I2C_NVRAM0_ID, buf, addr, maxlen) )
	    return error;
	buf += maxlen;
	addr += maxlen;
	len -= maxlen;
    }
    if( len > 0 )
	return xtboard_i2c_read(XT2000_I2C_NVRAM1_ID, (unsigned char*)buf, addr, len);
    return 0;
}


#define XTBOARD_NVRAM_PAGE_SIZE		8	/* bytes per page for writing; MUST be a power of 2 */
#define XTBOARD_NVRAM_ACK_SWTIMER       100     /* number of attempts to see ACK after page write */
//#define XTBOARD_I2C_DELAYNS		50000	/* I2C transition delay in nanosecs (50us) */
//#define XTBOARD_NVRAM_WRITE_DELAYNS	15000000/* NVRAM write cycle time in nanosecs (15ms) */
//#define XTBOARD_NVRAM_POLL_DELAYS	35	/* number of I2C transitions per NVRAM poll */
//#define XTBOARD_NVRAM_MAX_POLL_LOOPS	(XTBOARD_NVRAM_WRITE_DELAYNS / (XTBOARD_NVRAM_POLL_DELAYS * XTBOARD_I2C_DELAYNS) + 1)

/*
 *  Write data to NVRAM.
 *
 *  Some 24C04 devices have 8-byte pages, some have 16-byte pages.
 *  (Not sure if there are other sizes.)
 *  So to be safe and work on most/all of them, this function
 *  doesn't write more than 8 bytes at a time (XTBOARD_NVRAM_PAGE_SIZE above)
 *
 *  Also, not all devices have the write-protection feature.
 *  This code was only tested with devices not having write-protection.
 *  (Device should simply not ACK certain things when write-protected,
 *  so hopefully that is detected just fine.)
 */
int xtboard_nvram_write(unsigned addr, unsigned len, unsigned char *buf)
{
    int error, maxlen;

    if( addr > XT2000_NVRAM_SIZE || len > XT2000_NVRAM_SIZE || addr+len > XT2000_NVRAM_SIZE )
	return 3;
    while( len > 0 ) {
	maxlen = XTBOARD_NVRAM_PAGE_SIZE - (addr & (XTBOARD_NVRAM_PAGE_SIZE-1));
	if( maxlen > len )
	    maxlen = len;
	if( error = xtboard_i2c_write(XT2000_I2C_NVRAM0_ID + (addr >> 8), buf, addr, maxlen) )
	    return error;
	buf += maxlen;
	addr += maxlen;
	len -= maxlen;

        /*  Poll for completion: */
#if 0
	for( maxlen = XTBOARD_NVRAM_MAX_POLL_LOOPS; maxlen > 0; maxlen-- )
	    if( xtboard_i2c_write(XT2000_I2C_NVRAM0_ID + (addr >> 8), buf, addr, 0) == 0 )
		break;
#endif /*0*/
	if (!xtboard_i2c_wait_nvram_ack(XT2000_I2C_NVRAM0_ID + (addr >> 8), XTBOARD_NVRAM_ACK_SWTIMER)) 
	    return 1;   /* no ACK, timeout */
    }
    return 0;
}

/******************   NVRAM Info Structure   *******************/

int xtboard_nvram_binfo_read(xt2000_nvram_binfo *buf)
{
    int error = xtboard_nvram_read(XT2000_NVRAM_BINFO_START, XT2000_NVRAM_BINFO_SIZE, (unsigned char*)buf);
    if( error || buf->version != XT2000_NVRAM_BINFO_VERSION )
	memset( (unsigned char*)buf, 0, XT2000_NVRAM_BINFO_SIZE );
    return error;
}

int xtboard_nvram_binfo_write(xt2000_nvram_binfo *buf)
{
    int error;
    unsigned short crc;

    buf->version = XT2000_NVRAM_BINFO_VERSION;
    /*  Set 16-bit CRC (computed such that it must be stored in little-endian order):  */
    crc = calc_crc16(0, (unsigned char*)buf, XT2000_NVRAM_BINFO_SIZE-2);
    buf->crc[0] = crc;
    buf->crc[1] = (crc >> 8);

    return xtboard_nvram_write(XT2000_NVRAM_BINFO_START, XT2000_NVRAM_BINFO_SIZE, (unsigned char*)buf);
}


int xtboard_nvram_binfo_valid(xt2000_nvram_binfo *buf)
{
    return (calc_crc16(0, (unsigned char*)buf, XT2000_NVRAM_BINFO_SIZE) == 0);
}

/*
 *  Returns Ethernet MAC address assigned to the board.
 *  'buf' must point to a 6-byte buffer.
 *  Returns:
 *	0	success (Ethernet address in buf[0..5])
 *	1	NVRAM device did not respond (nothing written to buf[0..5])
 *	2	delay function not set (use xtboard_set_nsdelay_func())
 *	3	invalid board info structure address (should not happen)
 *	20	NVRAM info structure not valid (buf[0..5] is not modified)
 *	21	Ethernet MAC address not set (all zeroes, buf[0..5] is not modified)
 *    [ 22	Ethernet MAC address not a valid unicast address (multicast bit set) ]
 */
int xtboard_ethermac_get(unsigned char *buf)
{
    xt2000_nvram_binfo info;
    int i, error;

    if( error = xtboard_nvram_binfo_read(&info) )
	return error;

    if( !xtboard_nvram_binfo_valid(&info) )
	return 20;

    error = 21;
    for( i = 0; i < 6; i++ )
	if( info.eth_addr[i] ) error = 0;

    /*if( (info.eth_addr[?] & ?) != 0 )
	return 22;*/

    if (error) 
       return error;

    for( i = 0; i < 6; i++ )
       buf[i] = info.eth_addr[i];
	
    return 0;
}

/*
 *  Returns:
 *	0	success (Ethernet address in buf[0..5])
 *	1	NVRAM device did not respond (nothing written to buf[0..5])
 */
int xtboard_ethermac_set(unsigned char *buf)
{
    xt2000_nvram_binfo info;
    int i, error;

    if( error = xtboard_nvram_binfo_read(&info) )
	return error;		/* can return 1 */
    for( i = 0; i < 6; i++ )
	info.eth_addr[i] = buf[i];
    return xtboard_nvram_binfo_write(&info);
}


/******************   Old Stuff   *******************/


#if 0	/*FIXME -- is this okay?*/

static unsigned nvram_data_valid = 0;
static xt2000_nvram_binfo nvram_data;

unsigned xtboard_nvram_valid(void (*delay)( void ))
{
    if (nvram_data_valid)
	return 1;
    xtboard_set_nsdelay_func( (XtboardDelayFunc*)delay );
    xtboard_nvram_binfo_read(&nvram_data);
    return (nvram_data_valid = xtboard_nvram_binfo_valid(&nvram_data));
}

unsigned xtboard_get_nvram_contents(unsigned char *buf, void (*delay)( void ))
{
    if (xtboard_nvram_valid(delay));
	memcpy(buf, (unsigned char*)&nvram_data, XT2000_NVRAM_BINFO_SIZE);
    return nvram_data_valid;
}

void xtboard_get_ether_addr(unsigned char *buf, void (*delay)( void ))
{
    xtboard_set_nsdelay_func( (XtboardDelayFunc*)delay );
    xtboard_ethermac_get(buf);
}

#endif /*0*/

