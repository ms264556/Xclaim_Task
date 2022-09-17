/* Tell simulator that the ferret library has been linked in */

/*
 * Copyright (c) 2003, 2004 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include "simcall.h"

	/* Define dummy symbol used to force this file to be linked.  */
	.global	__ferret_init
__ferret_init:

	.begin literal_prefix .init

	.section .init
	movi	a2, SYS_ferret
	movi	a3, __stack
	movi	a4, _bss_end
	simcall

	.end literal_prefix
