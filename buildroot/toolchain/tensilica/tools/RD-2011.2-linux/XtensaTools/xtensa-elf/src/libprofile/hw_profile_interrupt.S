/* Interrupt dispatcher for those not using xtos.  */

/*
 * Copyright (c) 2005-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

	
#include <xtensa/config/core.h>
#include "xt_profiling.h"
#include "hw_profiler_internal.h"

.global LABEL(_Level,Vector)

	.section		LABEL(.Level,InterruptVector.text), "ax"
	.align 4
LABEL(_Level,Vector):

	// This code assumes that the address of body of the handler is
	// stored in the exxsave register for its level. __hw_profiling_enable
	// does this.

	// This handler saves the old value of a2 into the appropriate excsave 
	// register, then jumps to the body of the handler. The body restores 
	// both a2 and it's own address back into the excsave register.
	xsr_excsave	a2
	jx	a2
