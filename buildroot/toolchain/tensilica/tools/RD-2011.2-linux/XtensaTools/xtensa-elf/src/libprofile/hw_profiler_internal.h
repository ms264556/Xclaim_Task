/* Interrupt dispatcher for those not using xtos.  */

/*
 * Copyright (c) 2005-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#define INTERRUPT_IS_HI(level)  \
	( XCHAL_HAVE_INTERRUPTS && \
	 (XCHAL_EXCM_LEVEL < level) && \
	 (XCHAL_NUM_INTLEVELS >= level) && \
	 (XCHAL_HAVE_DEBUG ? XCHAL_DEBUGLEVEL != level : 1))

#define INTERRUPT_IS_MED(level) \
	(XCHAL_HAVE_INTERRUPTS && (XCHAL_EXCM_LEVEL >= level))

#define _JOIN(x,y)	x ## y
#define JOIN(x,y)	_JOIN(x,y)

#define _JOIN3(a,b,c)	a ## b ## c
#define JOIN3(a,b,c)	_JOIN3(a,b,c)

#define TIMER_INTERRUPT XCHAL_TIMER_INTERRUPT(TIMER)
#define LEVEL XCHAL_INT_LEVEL(TIMER_INTERRUPT)
#define LABEL(x,y)		JOIN3(x,LEVEL,y)

#define rsr_excsave  JOIN(rsr.excsave, LEVEL)
#define wsr_excsave  JOIN(wsr.excsave, LEVEL)
#define xsr_excsave  JOIN(xsr.excsave, LEVEL)
#define rsr_epc      JOIN(rsr.epc, LEVEL)
#define wsr_epc      JOIN(wsr.epc, LEVEL)
#define rsr_ccompare JOIN(rsr.ccompare, TIMER)
#define wsr_ccompare JOIN(wsr.ccompare, TIMER)

/* MUST be a power of 2 or the masking and offset calculations won't work */
#define ADDRESSES_PER_RECORD 0x800

/* 2 == sizeof(unsigned short), but is included in assembly file */
#define BUCKETS_PER_RECORD (ADDRESSES_PER_RECORD / 2)
