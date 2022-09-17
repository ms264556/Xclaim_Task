/*********************************************************************
 **
 * Module: V3USC32
 **
 * File Name: v3usci2c.c
 **
 * Authors: Phil Sikora
 **
 * Copyright (c) 1997-1999 V3 Semiconductor. All rights reserved.
 **
 * V3 Semiconductor makes no warranties for the use of its products.  V3 does
 * not assume any liability for errors which may appear in these files or
 * documents, however, we will attempt to notify customers of such errors.
 *
 * V3 Semiconductor retains the right to make changes to components,
 * documentation or specifications without notice.
 *
 * Please verify with V3 Semiconductor to be sure you have the latest
 * specifications before finalizing a design.
 **
 * $Revision: 20 $	$Date: 7/22/99 12:03p $
 * $NoKeywords: $
 **
 * Description:
 **
 * This code has been adapted from James Flynn article in Embedded
 * Systems November 1997.  See www.embedded.com web site.
 **
 * For porting to other Operating Systems see the following three
 * routines at the end of this file, I2C_ReadRegSystem, I2C_WriteRegSystem
 * and the I2C_Delay routines.
 * Re-designed I2C to split the clock into four time slices, shortened
 * I2C_Delay from 5uS to 2.5uS, extended delay for start condition
 * setup and hold times.
 **
 ********************************************************************/

/*********************************************************************
 *
 * Include files that this module depends on.
 *
 ********************************************************************/

#define GLOBAL_VARIABLES
#define _V3USCREG_H_AB_

#include "V3usci2c.h"
#include "V3uscreg.h"

//#include "v3usc32.h"

#define V3_BASE_ADDR 0x3d000000

BOOL fConfigCycle;


/*!
 *********************************************************************
 **
 * Function: void I2C_Init(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 **
 * Description:
 * Note: I2C_Init should be called for each board.
 * This routine will force a stop condition on the I2C bus.
 * The clock and data lines will be left high and this routine will 
 * return after a half clock.
 * This routine is only called now if EEPROM is about to be accessed
 * previously this was always called which modified the system
 * register, and assumed the EEPROM was always present.
 * Now the first EEPROM routine read/write will initialized the
 * I2C bus.
 **
 ********************************************************************/
void I2C_Init(V3USCHANDLE hV3)
{
	static bOneTimeInit = FALSE;

	if(!bOneTimeInit)
	{
		/* Unlock the system register */
		I2C_UnLock(hV3);
		
		/* Ensure that the I2C clock and data lines are in a known state */
		I2C_Stop(hV3);
	
		/* Lock the system register */
		I2C_Lock(hV3);
	
		/* ensure once only */
		bOneTimeInit = TRUE;
	}
}

/*!
 *********************************************************************
 **
 * Function: BOOL I2C_EEPROMWrite(V3USCHANDLE hV3, BYTE bSlaveAddr BYTE bAddr, BYTE bData, BOOL fUseConfigCycle)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bSlaveAddr - 3 bit address of device been selected
 * BYTE bAddr - address of EEPROM location to be read from
 * BYTE bData - value to be written to EEPROM 
 * BOOL fUseConfigCycle - Use PCI configuration cycles to access EEPROM
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 * This routine writes a byte to the slave device's address specified.
 * This routine uses the Write Byte method from the Atmel documentation.
 **
 ********************************************************************/
BOOL I2C_EEPROMWrite(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE bData, BOOL fUseConfigCycle)
{
  BOOL fStatus = TRUE;

  /* Set Global flag to use PCI configuration cycles */
  fConfigCycle = fUseConfigCycle;

  /* Ensure the I2C bus is initialized */
  I2C_Init(hV3);

  /* Set upper device address */
  bSlaveAddr |= I2C_1010;

  /* Unlock the system register */
  I2C_UnLock(hV3);

  /* Send device Address and wait for Ack, bAddr is sent as a write */
  if ((I2C_PollAckStart(hV3, bSlaveAddr, I2C_WRITE)) != I2C_OK)
    fStatus = FALSE;
  else
    /* Send bAddr as the address to write */
    if (I2C_Write8(hV3, bAddr))
      fStatus = FALSE;
    else
      /* Send the value to write */
      if (I2C_Write8(hV3, bData))
	fStatus = FALSE;

  /* Clean up the bus with a stop condition */
  I2C_Stop(hV3);

  /* Lock the system register */
  I2C_Lock(hV3);
  return (fStatus);
}

/*!
 *********************************************************************
 **
 * Function: BOOL I2C_EEPROMRead(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE *bData, BOOL fUseConfigCycle)
 **
 * Input Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bSlaveAddr - 3 bit address of device been selected
 * BYTE bAddr - address of EEPROM location to be read from
 * BOOL fUseConfigCycle - Use PCI configuration cycles to access EEPROM
 **
 * Input Parameters:
 * BYTE *bData - EEPROM data read back (pointer to data passed)
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 * This routine reads back a byte from the slave device's address
 * specificed.  This routine uses the Random Read method in the Atmel
 * documentation.
 **
 ********************************************************************/
BOOL I2C_EEPROMRead(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE *bData, BOOL fUseConfigCycle)
{
  BOOL fStatus = TRUE;

  /* Set Global flag to use PCI configuration cycles */
  fConfigCycle = fUseConfigCycle;

  /* Ensure the I2C bus is initialized */
  I2C_Init(hV3);

  /* Set upper device address */
  bSlaveAddr |= I2C_1010;

  /* Unlock the system register */
  I2C_UnLock(hV3);

  /* Send device Address and wait for Ack, bAddr is sent as a write */
  if ((I2C_PollAckStart(hV3, bSlaveAddr, I2C_WRITE)) != I2C_OK)
    fStatus = FALSE;
  else
    /* Send bAddr as the random address to be read */
    if (I2C_Write8(hV3, bAddr))
      fStatus = FALSE;
    else
      /* Send a START condition and device address this time as read */
      if (I2C_StartSlave(hV3, bSlaveAddr, I2C_READ))
	fStatus = FALSE;
      else
	/* read the date back from EEPROM */
	*bData = I2C_Read8(hV3);

  /* clean up bus with a NaK and Stop condition */
  I2C_NoAck(hV3);
  I2C_Stop(hV3);

  /* Lock the system register */
  I2C_Lock(hV3);
  return (fStatus);
}


/*!
 *********************************************************************
 **
 * Function: BOOL I2C_RTCWriteByte(V3USCHANDLE hV3, Byte bAddress, BYTE bData)
 **
 * Parameters:
 * V3USCHANDLE hV3      - Handle to USC data structure.
 * BYTE        bAddress - Address to write
 * BYTE        bData    - Data to write
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 * This routine writes 4-bytes to the Real time clock
 **
 ********************************************************************/
BOOL I2C_RTCWriteByte(V3USCHANDLE hV3, BYTE bAddress, BYTE bData)
{
  BOOL fStatus    = TRUE;
  BYTE bSlaveAddr = 0;
  int  i          = 0;

  /* We are always using the config cycles. */
  fConfigCycle = TRUE;

  /* Ensure the I2C bus is initialized */
  I2C_Init(hV3);

  /* Set upper device address */
  bSlaveAddr = I2C_1101;

  /* Unlock the system register */
  I2C_UnLock(hV3);

  /* Send device Address and wait for Ack, bAddr is sent as a write */
  if ((I2C_PollAckStart(hV3, bSlaveAddr, I2C_WRITE)) != I2C_OK)\
    {
      fStatus = FALSE;
    }
  else
    {
      /* Send bAddr as the address to write */
      if (I2C_Write8(hV3, bAddress ))
	fStatus = FALSE;
      else
	/* Send the value to write */
	if (I2C_Write8(hV3, bData))
	  fStatus = FALSE;
    }

  /*  Clean up the bus with a stop condition */
  I2C_Stop(hV3);

  /* Lock the system register */
  I2C_Lock(hV3);
  return (fStatus);
}



/*!
 *********************************************************************
 **
 * Function: BOOL I2C_RTCReadByte(V3USCHANDLE hV3, BYTE bAddr, BYTE *bData)
 **
 * Input Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Input Parameters:
 * BYTE bAddr   -- Address to read
 * BYTE bData   -- Pointer to where to read the data
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 *   This routine reads the 4 bytes in the real time clock and returns
 *   that vlues in dwTime.
 **
 ********************************************************************/
BOOL I2C_RTCReadByte(V3USCHANDLE hV3, BYTE bAddr, BYTE *bData)
{
  BOOL 	fStatus 	= TRUE;
  DWORD dwClock 	= 0;
  BYTE  bSlaveAddr 	= 0;


  /* We are very simple and just always use the config cycles */
  fConfigCycle = TRUE;

  /* Ensure the I2C bus is initialized */
  I2C_Init(hV3);

  /* Set upper device address */
  bSlaveAddr = I2C_1101;

  /* Unlock the system register */
  I2C_UnLock(hV3);

  /* Send device Address and wait for Ack, bAddr is sent as a write */
  if ((I2C_PollAckStart(hV3, bSlaveAddr, I2C_WRITE)) != I2C_OK)
    {
      fStatus = FALSE;
    }
  else
    {
      if (I2C_Write8(hV3, bAddr))
	{
	  fStatus = FALSE;
	}
      else
	{
	  /* Send a START condition and device address this time as read */
	  if (I2C_StartSlave(hV3, bSlaveAddr, I2C_READ))
	    {
	      fStatus = FALSE;
	    }
	  else
	    {
	      /* read the date back from EEPROM */
	      *bData = (DWORD)I2C_Read8(hV3);
	    }
	}
    }


  /* clean up bus with a NaK and Stop condition */
  I2C_NoAck(hV3);
  I2C_Stop(hV3);


  /* Lock the system register */
  I2C_Lock(hV3);
  return (fStatus);
}


/*!
 *********************************************************************
 **
 * Function: BOOL I2C_RTCWrite(V3USCHANDLE hV3, DWORD dwTime)
 **
 * Parameters:
 * V3USCHANDLE hV3    - Handle to USC data structure.
 * DWORD       dwTime - Data to write to the Real Time Clock
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 * This routine writes 4-bytes to the Real time clock
 **
 ********************************************************************/
BOOL I2C_RTCWrite(V3USCHANDLE hV3, DWORD dwTime)
{
  BOOL fStatus    = TRUE;
  int  i          = 0;


  for (i = 0; i < 4; ++i)
    {
      BYTE  bData  = 0;
      DWORD dwTemp = 0;

      dwTemp = dwTime;
      dwTemp = dwTemp >> (i * 8);
      dwTemp = dwTemp & 0xff;

      bData = (BYTE)dwTemp;

      fStatus = I2C_RTCWriteByte(hV3, i, bData);
      if ( fStatus = FALSE)
	break;
    }
      

  return (fStatus);
}



/*!
 *********************************************************************
 **
 * Function: BOOL I2C_RTCRead(V3USCHANDLE hV3, BYTE bSlaveAddr, DWORD *dwTime)
 **
 * Input Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Input Parameters:
 * DWORD *dwTime - Time values read back from the Real time clock
 **
 * Return:
 * BOOL fStatus - TRUE if read successful
 **
 * Description:
 *   This routine reads the 4 bytes in the real time clock and returns
 *   that vlues in dwTime.
 **
 ********************************************************************/
BOOL I2C_RTCRead(V3USCHANDLE hV3, DWORD *dwTime)
{
  BOOL 	fStatus 	= TRUE;
  DWORD dwClock 	= 0;
  int   i       	= 0;


  /* iterate over address's 0 - 4 to get the value of the clock */
  for (i = 0; i < 4; ++i)
    {
      BYTE   bData = 0;
      DWORD dwTemp = 0;

      fStatus = I2C_RTCReadByte(hV3, i, &bData);
      if (fStatus == FALSE)
	break;

      dwTemp = bData;
      dwClock = dwClock | (dwTemp << (i*8));
    }


  if ( fStatus == TRUE )
    *dwTime = dwClock;

  return (fStatus);
}




/*!
 *********************************************************************
 **
 * Function: int I2C_StartSlave(V3USCHANDLE hV3, register BYTE bAddr, bit ReadBit)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bAddr - 3 bit address of device been selected
 * bit ReadBit - Set if command is to be a read, reset if command is a write 
 **
 * Return:
 * I2C_OK    - read was successful
 * I2C_NOACK - if negative acknowledge was received
 **
 * Description:
 * All commands are preceded by the start condition, which is a high
 * to low transition of SDA when SCL is high.  All I2C devices
 * continuously monitor the SDA and SCL lines for the start condition 
 * and will not respond to any command until this condition has been met.
 *
 * Once a device detects that it is being addressed it outputs an 
 * acknowledge on the SDA line.  Depending on the state of the read/write 
 * bit, the device will execute a read or write operation.
 *
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | SA6 | SA5 | SA4 | SA3 | SA2 | SA1 | SA0 | R/W |
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 *                                              |--- 1=read, 0=write
 **
 ********************************************************************/
int I2C_StartSlave(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit)
{
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_Delay(hV3);
	I2C_SDA(hV3, I2C_LOW);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_LOW);
	I2C_Delay(hV3);
	return I2C_Write8(hV3, (BYTE) (bAddr<<1 | ReadBit));
}

/*!
 *********************************************************************
 **
 * Function: int I2C_PollAckStart(V3USCHANDLE hV3, register BYTE bAddr, bit ReadBit)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 * BYTE bAddr - 3 bit address of device been selected
 * bit ReadBit - Set if command is to be a read, reset if command is a write 
 **
 * Return:
 * I2C_BUSY  - device is not responding (retried out)
 * I2C_OK    - read was successful
 **
 * Description:
 * This routine checks the status of the device being written to by 
 * issuing a start condition followed by check for ACK.  If the slave
 * device is unable to communicate, the write command will not be
 * acknowledged and we should wait.  This routine will try I2C_RETRY times for 
 * an ACK before returning a I2C_BUSY indication.
 * From testing the number of retries was as high as fifteen. 
 * Added extra I2c_Delay to start condition to better compensate for
 * 2.7 Volt devices tSU.STA timing
 **
 ********************************************************************/
int I2C_PollAckStart(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit)
{
  register BYTE bTries = I2C_RETRY;
  do
    {
      /* setup for start condition if device busy */
      I2C_SCL(hV3, I2C_HIGH);
      I2C_Delay(hV3);
      I2C_Delay(hV3);

		/* send start condition */
      I2C_SDA(hV3, I2C_LOW);
      I2C_Delay(hV3);
      I2C_Delay(hV3);
      I2C_SCL(hV3, I2C_LOW);
      I2C_Delay(hV3);
      if (I2C_Write8(hV3, (BYTE) (bAddr<<1 | ReadBit)) == I2C_OK)
	return (I2C_OK);
    }
  while(bTries--);
  return(I2C_BUSY);
}

/*!
 *********************************************************************
 **
 * Function: BYTE I2C_Read8(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 * BYTE bData - returns a byte of data 
 **
 * Description:
 * Reads eight bits from the I2C bus.
 **
 ********************************************************************/
BYTE I2C_Read8(V3USCHANDLE hV3)
{
	register BYTE i, bData=0;

	I2C_SDA(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	for (i=8; i; --i )
	{
		I2C_SCL(hV3, I2C_HIGH);
		I2C_Delay(hV3);
		bData = (bData<<1) | I2C_SDAIn(hV3);
		I2C_Delay(hV3);
		I2C_SCL(hV3, I2C_LOW);
		I2C_Delay(hV3);
		/* date delay time slice */
		I2C_Delay(hV3);
	}
	return(bData);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Write8(V3USCHANDLE hV3, register BYTE bData)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 * BYTE bAck - returns the acknowledge bit 
 **
 * Description:
 * This routine writes out all 8 bits of data out to the I2C bus.  After 
 * writing out the data, the routine reads the ACK/NACK response back and
 * returns it to the caller.
 **
 ********************************************************************/
BYTE I2C_Write8(V3USCHANDLE hV3,register BYTE bData)
{
	register BYTE i;
	for (i=0x80; i; i >>=1 )
	{
		I2C_SDA(hV3, i & bData);
		I2C_Delay(hV3);
		I2C_SCL(hV3, I2C_HIGH);
		I2C_Delay(hV3);
		/* date delay time slice */
		I2C_Delay(hV3);
		I2C_SCL(hV3, I2C_LOW);
		I2C_Delay(hV3);
	}
	I2C_SDA(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	i = I2C_SDAIn(hV3);
	I2C_Delay(hV3);
 	I2C_SCL(hV3, I2C_LOW);
	I2C_Delay(hV3);
	I2C_Delay(hV3);

	return(i);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Stop(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 **
 * Description:
 * All communications must be terminated by a stop condition which
 * is a low to high transition of SDA while SCL is high.  A stop 
 * condition can only be issued after the transmitting device has 
 * released the bus.
 * This must meet tSU.STO Stop setup time.
 **
 ********************************************************************/
void I2C_Stop(V3USCHANDLE hV3)
{
	I2C_SDA(hV3, I2C_LOW);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_Delay(hV3);
	I2C_SDA(hV3, I2C_HIGH);
	I2C_Delay(hV3);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_NoAck(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 **
 * Description:
 * The No-Acknowledge is a software convention used to indicate
 * unsucessful data transfers.  The transmitting device, either 
 * master or slave, will release the bus after transmitting 
 * eight bits.  During the ninth clock cycle the receiver will 
 * pull the SDA line high to indicate that it did not received the 
 * eight bits of data.
 **
 ********************************************************************/

void I2C_NoAck(V3USCHANDLE hV3)
{
	I2C_SDA(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_LOW);
	I2C_Delay(hV3);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Ack(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure.
 **
 * Return:
 **
 * Description:
 * Acknowledge is a software convention used to indicate
 * sucessful data transfers.  The transmitting device, either 
 * master or slave, will release the bus after transmitting 
 * eight bits.  During the ninth clock cycle the receiver will 
 * pull the SDA line low to acknowledge that it received the 
 * eight bits of data.
 **
 ********************************************************************/

void I2C_Ack(V3USCHANDLE hV3)
{
	I2C_SDA(hV3, I2C_LOW);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_HIGH);
	I2C_Delay(hV3);
	I2C_Delay(hV3);
	I2C_SCL(hV3, I2C_LOW);
}

/*!
 *********************************************************************
 **
 * Function: BYTE I2C_SDAIn(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 * BYTE bData - state of the Serial Data line
 **
 * Description:
 * This routine returns the state of the Serial Data line
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/
BYTE I2C_SDAIn(V3USCHANDLE hV3)
{
	BYTE System;

	System = I2C_ReadRegSystem(hV3);
	return ((System & SYSTEM_B_SDA_IN) >> SYSTEM_B_SDA_IN_SHIFT);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_SDA(V3USCHANDLE hV3, int State)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * int state - what state the SDA is to be set to
 **
 * Return:
 **
 * Description:
 * This routine sets the Serial Data line to the desired state.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/

void I2C_SDA(V3USCHANDLE hV3, int State)
{
	BYTE System;

	System = I2C_ReadRegSystem(hV3);
	if(State)
		System |= SYSTEM_B_SDA_OUT;
	else
		System &= ~SYSTEM_B_SDA_OUT;
	I2C_WriteRegSystem(hV3, System);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_SCL(V3USCHANDLE hV3, int State)
 **
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * int state - what state the SCL is to be set to
 **
 * Return:
 **
 * Description:
 * This routine sets the Serial Clock line to the desired state.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/

void I2C_SCL(V3USCHANDLE hV3, int State)
{
	BYTE System;

	System = I2C_ReadRegSystem(hV3);
	if(State)
		System |= SYSTEM_B_SCL;
	else
		System &= ~SYSTEM_B_SCL;

	I2C_Delay(hV3);
	I2C_WriteRegSystem(hV3, System);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Lock(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * Description:
 * This routine disables the Serial Clock pin, and sets the sytem
 * lock bit.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/

void I2C_Lock(V3USCHANDLE hV3)
{
  BYTE System;

  System = I2C_ReadRegSystem(hV3);
  I2C_WriteRegSystem(hV3, (BYTE) (System & ~SYSTEM_B_SPROM_EN));
  I2C_WriteRegSystem(hV3, (BYTE) (System | SYSTEM_B_LOCK));
}

/*!
 *********************************************************************
 **
 * Function: void I2C_UnLock(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * Description:
 * This routine enable the sytem register for writting, and enables
 * the Serial Clock output pin.
 * Test the global PCI configuration flag to use PCI configuration
 * cycles to access the EEPROM.
 **
 ********************************************************************/
void I2C_UnLock(V3USCHANDLE hV3)
{
  BYTE System;

  I2C_WriteRegSystem(hV3, SYSTEM_B_UNLOCK_TOKEN);
  System = I2C_ReadRegSystem(hV3);
  I2C_WriteRegSystem(hV3, (BYTE) (System |SYSTEM_B_SPROM_EN));
}

/*!
 *********************************************************************
 **
 * Function: BYTE I2C_ReadRegSystem(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * BYTE bSystem - system register contents
 **
 * Description:
 * This routine handles the physical read of the USC system register
 * Both I/O - Memory mapped and PCI configuration cycles are supported
 * Port this routine to your Operating System.
 **
 ********************************************************************/
BYTE I2C_ReadRegSystem(V3USCHANDLE hV3)
{
  if(fConfigCycle)
    return V3USC32_ReadConfigByte(hV3, V3USC_SYSTEM_B);
  //	else
  //		return V3USC32_ReadRegByte(hV3, V3USC_SYSTEM_B);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_WriteRegSystem(V3USCHANDLE hV3, BYTE bSystem )
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * BYTE bSystem - contents to write to USC system register
 **
 * Return:
 **
 * Description:
 * This routine handles the physical write of the USC system register
 * Both I/O - Memory mapped and PCI configuration cycles are supported
 * Port this routine to your Operating System.
 **
 ********************************************************************/
void I2C_WriteRegSystem(V3USCHANDLE hV3, BYTE bSystem)
{
  if(fConfigCycle)
    V3USC32_WriteConfigByte(hV3, V3USC_SYSTEM_B, bSystem);
  //	else
  //		V3USC32_WriteRegByte(hV3, V3USC_SYSTEM_B, bSystem);
}

/*!
 *********************************************************************
 **
 * Function: void I2C_Delay(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * Description:
 * This routine handles the 2.5 microsecond delay
 * Port this routine to your Operating System.
 **
 ********************************************************************/
void I2C_Delay(V3USCHANDLE hV3)
{
  volatile int foo = 0;

  // 0.05 microseconds per clock at 16 Mhz
  // a load on a volatile takes at least
  // 5 clocks.

  // This should be more than long enough
  while (foo < 100)
    ++foo;
}



/*!
 *********************************************************************
 **
 * Function: BYTE V3USC32_ReadConfigByte(V3USCHANDLE hV3, DWORD dwReg)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC32 data structure.
 * DWORD dwReg - offset of the register to read.  The constants of the
 * registers are already defined in V3USCREG.H for example PCI_BASE0
 **
 * Return:
 * BYTE data - the byte read from internal register 
 **
 * Description:
 * Reads a byte from an internal register on the board using a
 * PCI configuration cycle.
 **
 **********************************************************************/

BYTE V3USC32_ReadConfigByte(V3USCHANDLE hV3, DWORD dwReg)
{
  volatile BYTE *pByte = (BYTE*)(V3_BASE_ADDR + dwReg);

  return *pByte;
}


void V3USC32_WriteConfigByte(V3USCHANDLE hV3, DWORD dwReg, BYTE data)
{
  volatile BYTE *pByte = (BYTE*)(V3_BASE_ADDR + dwReg);

  *pByte = data;
}


