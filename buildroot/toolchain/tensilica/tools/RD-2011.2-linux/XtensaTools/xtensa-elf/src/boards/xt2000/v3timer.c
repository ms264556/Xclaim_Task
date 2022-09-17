/*********************************************************************
 **
 * Module: V3USC32
 **
 * File Name: v3timer.c
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
 * $Revision: 9 $	$Date: 5/19/99 5:29p $
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

/*********************************************************************
 *
 * Include files that this module depends on.
 *
 ********************************************************************/

#define GLOBAL_VARIABLES
#include "v3timer.h"

void Timer_Init()
{
	LARGE_INTEGER Temporary, Remainder, PicoSeconds;
	
	/* This inialization only needs to be done once */
	if (fIsCounterAvailable == FALSE)
	{
		/* Calculate period of high performance counter if one exists */ 
		if (QueryPerformanceFrequency(&Temporary))
		{
			/* get the number of pico seconds in a second */
			PicoSeconds = EnlargedIntegerMultiply(1000000, 1000000);
	
			/* check that divisor will not be zero */
			if (Temporary.LowPart | Temporary.HighPart)
			{
	
				/* calculate the period in pico seconds */
				CounterPeriod = LargeIntegerDivide(PicoSeconds, Temporary, &Remainder);
	
				/* pre calculate 2.5 uSeconds or 2500000 picoseconds of delay for I2C_Delay routine */
				Temporary = ConvertLongToLargeInteger(2500000);
				I2CuSecDelay = LargeIntegerDivide(Temporary, CounterPeriod, &Remainder);

				/* set global flag that high resolution timer is available */
				fIsCounterAvailable = TRUE;
			}
		}
	}
}

/*!
 *********************************************************************
 **
 * Function: void Timer_I2CDelay(V3USCHANDLE hV3)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 **
 * Return:
 **
 * Description:
 * This routine has been optimized for I2C bus use to give 5uSec delay.
 * This routine inserts delay.  On systems where the high resolution
 * timer is available very accurate delays can be obtained.  Otherwise
 * a simple loop based on PCI read accesses will be used. 
 **
 ********************************************************************/

void Timer_I2CDelay(V3USCHANDLE hV3)
{
	DWORD dwResults;
	LARGE_INTEGER CurrentTime, DelayTime;
	int i;

	if (fIsCounterAvailable)
	{
		QueryPerformanceCounter(&CurrentTime);
		DelayTime = LargeIntegerAdd(CurrentTime, I2CuSecDelay);

		while (LargeIntegerLessThanOrEqualTo(CurrentTime, DelayTime))
			QueryPerformanceCounter(&CurrentTime);

	} else {

		/* read a PCI register 2500 nSec / 4 clocks * 33 nS per clock */
		for (i = 0; i < 2500 /(4 * 33); i++)
			dwResults = V3USC32_ReadRegDWord(hV3, V3USC_PCI_VENDOR_W);
	}
}

/*!
 *********************************************************************
 **
 * Function: void Timer_MillisecondsOfDelay(V3USCHANDLE hV3, WORD wDelay)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * WORD wDelay - delay requested in milliseconds (0-65535)
 **
 * Return:
 **
 * Description:
 * This routine inserts a specified amount of delay.  On systems where
 * the high resolution timer is available very accurate delays can be
 * obtained.  Otherwise a simple loop based on PCI read accesses will
 * be used. 
 * Note: With out the high resolution timer the delays that this
 * routine will generate are extremely longer then specified.
 **
 ********************************************************************/
void Timer_MillisecondsOfDelay(V3USCHANDLE hV3, WORD wDelay)
{
	DWORD dwResults;
	LARGE_INTEGER CurrentTime, DelayTime, DelayInterval, Remainder;
	int i, j;

	/* prevent divide by zeros */
	if (wDelay == 0)
		return;

	if (fIsCounterAvailable)
	{
		QueryPerformanceCounter(&CurrentTime);

		/* get the number of pico seconds in a millisecond */
		DelayInterval = EnlargedIntegerMultiply(1000, 1000000);
		DelayInterval = ExtendedIntegerMultiply(DelayInterval, wDelay );

		/* calculate delay interval in units of high resolution counter */
		DelayInterval = LargeIntegerDivide(DelayInterval, CounterPeriod, &Remainder);
		DelayTime = LargeIntegerAdd(CurrentTime, DelayInterval);

		/* busy wait for time to elapse */
		while (LargeIntegerLessThanOrEqualTo(CurrentTime, DelayTime))
			QueryPerformanceCounter(&CurrentTime);

	} else {

		for (j = 0; j < wDelay; j++)
			/* read a PCI register 1000000 nSec / 4 clocks * 33 nS per clock) */
			for (i = 0; i < 1000000 /(4 * 33); i++)
				dwResults = V3USC32_ReadRegWord(hV3, V3USC_PCI_VENDOR_W);
	}
}

/*!
 *********************************************************************
 **
 * Function: void Timer_MircosecondsOfDelay(V3USCHANDLE hV3, WORD wDelay)
 **
 * Parameters:
 * V3USCHANDLE hV3 - Handle to USC data structure
 * WORD wDelay - delay requested in microseconds (0-65535)
 **
 * Return:
 **
 * Description:
 * This routine inserts a specified amount of delay.  On systems where
 * the high resolution timer is available very accurate delays can be
 * obtained.  Otherwise a simple loop based on PCI read accesses will
 * be used. 
 * Note: With out the high resolution timer the delays that this
 * routine will generate are extremely longer then specified.
 **
 ********************************************************************/
void Timer_MicrosecondsOfDelay(V3USCHANDLE hV3, WORD wDelay)
{
	DWORD dwResults;
	LARGE_INTEGER CurrentTime, DelayTime, DelayInterval, Remainder;
	int i, j;

	/* prevent divide by zeros */
	if (wDelay == 0)
		return;

	if (fIsCounterAvailable)
	{
		QueryPerformanceCounter(&CurrentTime);

		/* get the number of pico seconds in a mircosecond */
		DelayInterval = EnlargedIntegerMultiply(1000, 1000);
		DelayInterval = ExtendedIntegerMultiply(DelayInterval, wDelay );

		/* calculate delay interval in units of high resolution counter */
		DelayInterval = LargeIntegerDivide(DelayInterval, CounterPeriod, &Remainder);
		DelayTime = LargeIntegerAdd(CurrentTime, DelayInterval);

		/* busy wait for time to elapse */
		while (LargeIntegerLessThanOrEqualTo(CurrentTime, DelayTime))
			QueryPerformanceCounter(&CurrentTime);

	} else {

		for (j = 0; j < wDelay; j++)
			/* read a PCI register 1000 nSec / 4 clocks * 33 nS per clock) */
			for (i = 0; i < 1000 /(4 * 33); i++)
				dwResults = V3USC32_ReadRegWord(hV3, V3USC_PCI_VENDOR_W);
	}
}

/*!
 *********************************************************************
 **
 * Function: BOOL Timer_StartEvent()
 **
 * Global Parameters:
 * LARGE_INTEGER Microseconds - the start time is place here
 **
 * Return:
 * BOOL - Return TRUE if high resoulution timers available.
 **
 * Description:
 * This routine gets the start time of the event to be timed.
 * If this routine returns FALSE there is no way to time the event,
 * which could happen on older 486 base machines and if the
 * LARGE INTEGER support is turned off.
 **
 ********************************************************************/
BOOL Timer_StartEvent()
{

	if (fIsCounterAvailable)
		QueryPerformanceCounter(&Microseconds);

	return fIsCounterAvailable;
}

/*!
 *********************************************************************
 **
 * Function: void Timer_StopEvent()
 **
 * Global Parameters:
 * LARGE_INTEGER Microseconds - the elasped time
 **
 * Return:
 * DWORD dwMicroseconds - the elasped time as a DWORD
 **
 * Description:
 * The value returned is the number of microseconds between calling
 * Timer_StartEvent and TimerStopEvent.
 * The Timer_StopEvent can be called multiple times for a single
 * Timer_StartEvent.
 **
 ********************************************************************/
DWORD Timer_StopEvent()
{
	LARGE_INTEGER CurrentTime, ElapsedTime;
	ULONG Remainder;

	if (fIsCounterAvailable)
	{
		/* get time for end event as soon as possible */
		QueryPerformanceCounter(&CurrentTime);

		ElapsedTime = LargeIntegerSubtract(CurrentTime, Microseconds);

		/* calculate delay in pico seconds */
		ElapsedTime = ExtendedIntegerMultiply(ElapsedTime, CounterPeriod.LowPart);
		/* convert the pico seconds to micro seconds */
		ElapsedTime = ExtendedLargeIntegerDivide(ElapsedTime, 1000000, &Remainder);
		return ElapsedTime.LowPart;		
	}
}
