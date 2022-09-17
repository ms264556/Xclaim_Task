/* Copyright (c) 2006 Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.
*/

#ifndef _JTAG_XTENSA_H_
#define _JTAG_XTENSA_H_

typedef enum {
	ji_EnableOCD = 0x11,
	ji_DebugInt,
	ji_RetDebugInt,  // TBD: remove
	ji_DisRetOCD,    // TBD: remove
	ji_ExecuteDI,
	ji_LoadDI,
	ji_ScanDDR,
	ji_ReadDOSR,
	ji_ScanDCR,
	ji_LoadWDI,
	ji_TRAX   = 0x1c,
	ji_BYPASS = 0x1f,
} xtensaJtagInstruction;


typedef enum {
	OCDNormalMode,
	OCDRunMode,
	OCDHaltMode,
	OCDStepMode
} xtensaMode;

typedef enum {
	regInstruction,
	regBypass,
	regDIRW,
	regDIR,
	regDDR,
	regDOSR,
	regESR,
	regDCR,
	regTraxNDR,
	regTraxNAR,
	regMAX
} xtensaJtagReg;


typedef struct {
	xtensaMode mode;
	XTMP_core core;
	XTMP_tap tap;
	int core_num;
	jtagReg_t regs[regMAX];
	int dir_array_option;
} coreSlaveData_t;

#endif
