/* Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.  */

/* This file is a stub that supplies a dummy vale for
   XMP_MAX_DCACHE_LINESIZE when building the xmp library.  This file
   shouldn't be installed or used except when building the xmp-library
   during a config build.  */

#include <xtensa/config/core-isa.h>
#include <xtensa/config/core.h>
#include <xtensa/tie/xt_core.h>

#define XMP_MAX_DCACHE_LINESIZE XCHAL_DCACHE_LINESIZE
