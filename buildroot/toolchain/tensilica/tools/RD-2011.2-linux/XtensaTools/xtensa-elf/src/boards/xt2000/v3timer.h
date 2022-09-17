
/*********************************************************************
 **
 * Module: V3USC32
 **
 * File Name: v3timer.h
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
 * $Revision: 11 $	$Date: 5/19/99 5:29p $
 * $NoKeywords: $
 **
 * Description:
 * This module provides routines for software delays and to measure
 * time intervals.  The timer interval uses the high performance
 * timers from Pentium or better class computers. This module also
 * uses the large integer types for time calculations.  There is an
 * alternate software delay routine that does not use large intgers
 * or the high peformance timers, but simply access to the PCI
 * configuration registers.
 **
 ********************************************************************/

#ifndef _V3TIMER_H_
#define _V3TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * If the global variables are to be instantiated in a specific module
 * then GLOBAL_VARIABLES is define just prior to including this file.
 * The GLOBAL_VARIABLES variable is promptly undefined to prevent other
 * modules from instantiating their global variables.
 * The _GLOBAL_VARIABLES_V3TIMER_H_ define can then be used to istantiate
 * and global variables.
 * LOCAL_EXTERN is a macro for extern or nothing depending on how this include
 * file is called the variable will be accessible.
 */

#undef LOCAL_EXTERN
#ifdef GLOBAL_VARIABLES
	#undef GLOBAL_VARIABLES
	#define _GLOBAL_VARIABLES_V3TIMER_H_
	#define LOCAL_EXTERN
#else /* GLOBAL VARIABLES */
	#define LOCAL_EXTERN extern
#endif /* GLOBAL_VARIABLES */

/*********************************************************************
 **
 * Include files that this include file depends on.
 **
 ********************************************************************/

#include <windows.h>
#define V3_LARGE_INTEGER
#ifdef V3_LARGE_INTEGER
#include <largeint.h>
#endif
#include <v3usclib.h>
#include "v3usc32.h"

/*********************************************************************
 **
 * Equates
 **
 ********************************************************************/


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

#ifdef _GLOBAL_VARIABLES_V3TIMER_H_

BOOL fIsCounterAvailable = FALSE;

#ifdef V3_LARGE_INTEGER
LARGE_INTEGER CounterPeriod;
LARGE_INTEGER I2CuSecDelay;
LARGE_INTEGER Microseconds;
#endif

#endif /* _GLOBAL_VARIABLES_V3TIMER_H_ */

/*********************************************************************
 **
 * Global Variables to be exported from this module 
 **
 ********************************************************************/


/*********************************************************************
 **
 * Prototypes
 **
 ********************************************************************/
void Timer_Init();
void Timer_I2CDelay(V3USCHANDLE hV3);
void Timer_MicrosecondsOfDelay(V3USCHANDLE hV3, WORD wDelay);
void Timer_MillisecondsOfDelay(V3USCHANDLE hV3, WORD wDelay);
BOOL Timer_StartEvent();
DWORD Timer_StopEvent();

#ifdef __cplusplus
}
#endif

#endif /* _V3TIMER_H_ */
