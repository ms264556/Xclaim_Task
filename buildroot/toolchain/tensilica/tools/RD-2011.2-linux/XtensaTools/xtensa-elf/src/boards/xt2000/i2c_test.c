/* i2c_test.c - Simple test for the V3 I2C interface */
/*
 * Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */                                                                             
#include <time.h>
#include "V3usci2c.h"


int main()
{
  int  		i          = 0;
  BYTE 		foo    	   = 0;
  BYTE 		bSlaveAddr = 0;
  int  		bAddr      = 0;
  char      	*pTime     = NULL;
  struct tm 	current_tm = {0};

  //  BYTE *pTest = (BYTE *)0x3d000073;

  BYTE blockOne[256] = {0};
  BYTE blockTwo[256] = {0};

  DWORD dwTime  = 0;
  DWORD dwTemp  = 0;
  DWORD newTime = 0;

  BYTE  rtcControl = 0;

  if ( I2C_RTCReadByte(0, 4, &rtcControl) == TRUE &&
       rtcControl != 0 )
    {
      rtcControl = 0;

	  // Toggle the upper bit
      if ( I2C_RTCWriteByte(0, 4, rtcControl == TRUE))
        {
          // Read it to make sure it took hold
          rtcControl = 0;
   	  I2C_RTCReadByte(0, 4, &rtcControl);
	}
    }
	       
    


  current_tm.tm_sec = 0;
  current_tm.tm_min = 20;
  current_tm.tm_hour = 11;
  current_tm.tm_mday = 7;
  current_tm.tm_mon  = 2;
  current_tm.tm_year = 100;

  // tm_wday and tm_yday are thankfully ignored

  newTime = (DWORD)mktime(&current_tm);

  if ( I2C_RTCWrite(0, newTime) == TRUE)
    {
      if ( I2C_RTCRead(0, &dwTime) == TRUE)
	{
	  pTime = ctime( (time_t*) &dwTime);
	  dwTemp = dwTime;
	}
    }
  

  for (;;)
    {
      if ( I2C_RTCRead(0, &dwTime) == TRUE)
        {
          pTime = ctime( (time_t*) &dwTime);
        }
    }




  //  *pTest = 0xa5;


  for (bAddr = 0; bAddr < 256; ++bAddr)
    {
      if (I2C_EEPROMRead(0, 6, bAddr, &foo, TRUE) == TRUE )
	{
	  blockOne[bAddr] = foo;
	}
      else
	{
	  // BAD!
	  blockOne[bAddr] = 0xff;
	}
    }


  for (bAddr = 0; bAddr < 256; ++bAddr)
    {
      if (I2C_EEPROMRead(0, 7, bAddr, &foo, TRUE) == TRUE )
	{
	  blockTwo[bAddr] = foo;
	}
      else
	{
	  // BAD!
	  blockTwo[bAddr] = 0xff;
	}
    }


  // We'll program block Two with what's in blockOne
  for (bAddr = 0; bAddr < 256; ++bAddr)
    {
      if (I2C_EEPROMWrite(0, 7, bAddr, blockOne[bAddr], TRUE) == FALSE )
	{
	  // BAD!
	  blockTwo[bAddr] = 0xff;
	}
    }

  

  return 0;
}
