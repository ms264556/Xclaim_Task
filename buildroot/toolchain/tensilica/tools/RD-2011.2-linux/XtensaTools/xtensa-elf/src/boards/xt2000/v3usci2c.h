/*********************************************************************
 **
 * Module: V3MODULE
 **
 * File Name: v3usci2c.h
 **
 * Authors: Phil Sikora
 **
 * Copyright (c) 1997-1999 V3 Semiconductor. All rights reserved.
 *
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
 * $Revision: 16 $	$Date: 5/10/99 10:58a $
 * $NoKeywords: $
 **
 * Description:
 **
 ********************************************************************/

#ifndef _V3USCI2C_H_
#define _V3USCI2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FALSE 0
#define TRUE  1

  typedef unsigned int 	 BOOL;
  typedef unsigned char  BYTE;
  typedef unsigned int   DWORD;
  typedef unsigned short USHORT;


  


/*
 * If the global variables are to be instantiated in a specific module
 * then GLOBAL_VARIABLES is define just prior to including this file.
 * The GLOBAL_VARIABLES variable is promptly undefined to prevent other
 * modules from instantiating their global variables.
 * The _GLOBAL_VARIABLES_V3USCI2C_H_ define can then be used to istantiate
 * and global variables.
 * LOCAL_EXTERN is a macro for extern or nothing depending on how this include
 * file is called the variable will be accessible.
 */

#undef LOCAL_EXTERN
#ifdef GLOBAL_VARIABLES
	#undef GLOBAL_VARIABLES
	#define _GLOBAL_VARIABLES_V3USCI2C_H_
	#define LOCAL_EXTERN
#else /* GLOBAL VARIABLES */
	#define LOCAL_EXTERN extern
#endif /* GLOBAL_VARIABLES */

/*********************************************************************
 **
 * Include files that this include file depends on.
 **
 ********************************************************************/
  //#include <v3usclib.h>
  //#include "v3timer.h"

/*********************************************************************
 **
 * Equates
 **
 ********************************************************************/

/* I2C Bus states */
#define I2C_HIGH			(1)
#define I2C_LOW				(0)
#define I2C_OK				(0)
#define I2C_READ			(1)
#define I2C_WRITE			(0)
#define I2C_BUSY			(2)
#define I2C_NOACK			(1)
/* number of times a master will try and write to a slave device */
#define I2C_RETRY			(0x30)
#define I2C_1010			0x50
#define I2C_1101			0x68


/*********************************************************************
 **
 * Data Structures
 **
 ********************************************************************/


/*********************************************************************
 **
 * Global Variables with scope of only the module that defined GLOBAL_VARIABLES
 **
 ********************************************************************/

#ifdef _GLOBAL_VARIABLES_V3USCI2C_H_

BOOL fConfigCycle = FALSE;

#endif /* _GLOBAL_VARIABLES_V3USCI2C_H_ */

/*********************************************************************
 **
 * Global Variables to be exported from this module 
 **
 ********************************************************************/



/*********************************************************************
 **
 * Types
 **
 ********************************************************************/

  typedef int V3USCHANDLE;


/*********************************************************************
 **
 * Prototypes
 **
 ********************************************************************/

BOOL I2C_EEPROMWrite(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE bData, BOOL fUseConfigCycle);
BOOL I2C_EEPROMRead(V3USCHANDLE hV3, BYTE bSlaveAddr, BYTE bAddr, BYTE *bData, BOOL fUseConfigCycle);

BOOL I2C_RTCWrite(V3USCHANDLE hV3, DWORD dwTime);
BOOL I2C_RTCRead(V3USCHANDLE hV3,  DWORD *dwTime);

BOOL I2C_RTCWriteByte(V3USCHANDLE hV3, BYTE bAddr, BYTE bData);
BOOL I2C_RTCReadByte(V3USCHANDLE hV3,  BYTE bAddr, BYTE *bData);


#ifdef _GLOBAL_VARIABLES_V3USCI2C_H_

int I2C_StartSlave(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit);
int I2C_PollAckStart(V3USCHANDLE hV3, register BYTE bAddr, BYTE ReadBit);
BYTE I2C_Write8(V3USCHANDLE hV3,register BYTE bData);
BYTE I2C_Read8(V3USCHANDLE hV3);


void I2C_Stop(V3USCHANDLE hV3);
void I2C_NoAck(V3USCHANDLE hV3);
void I2C_Ack(V3USCHANDLE hV3);
BYTE I2C_SDAIn(V3USCHANDLE hV3);
void I2C_SDA(V3USCHANDLE hV3, int state);
void I2C_SCL(V3USCHANDLE hV3, int state);
void I2C_Lock(V3USCHANDLE hV3);
void I2C_UnLock(V3USCHANDLE hV3);

/* For porting these three routines must be changed */
BYTE I2C_ReadRegSystem(V3USCHANDLE hV3);
void I2C_WriteRegSystem(V3USCHANDLE hV3, BYTE bSystem);
void I2C_Delay(V3USCHANDLE hV3);

BYTE V3USC32_ReadConfigByte(V3USCHANDLE hV3, DWORD dwReg);
void V3USC32_WriteConfigByte(V3USCHANDLE hV3, DWORD dwReg, BYTE data);

#endif /* _GLOBAL_VARIABLES_V3USCI2C_H_ */

#ifdef __cplusplus
}
#endif

#endif /* _V3USCI2C_H_ */






