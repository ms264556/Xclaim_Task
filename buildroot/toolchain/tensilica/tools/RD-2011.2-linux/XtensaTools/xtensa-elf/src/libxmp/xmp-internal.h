// Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

#ifndef __XMP_INTERNAL_H__
#define __XMP_INTERNAL_H__

#include "xtensa/config/core-isa.h"
#include "xtensa/config/core.h"
#if XCHAL_HAVE_CCOUNT
#include "xtensa/tie/xt_timer.h"
#endif
#include <stdio.h>
#include "xmp-library.h"
#if XCHAL_HAVE_RELEASE_SYNC
#include <xtensa/tie/xt_sync.h>
#endif
#if XCHAL_HAVE_EXTERN_REGS
#include <xtensa/tie/xt_externalregisters.h>
#endif
#if XCHAL_HAVE_MP_INTERRUPTS
#include <xtensa/xtruntime.h>
#include <xtensa/tie/xt_interrupt.h>
#include <xtensa/xtensa-xer.h>

static inline void
xmp_wake_core (unsigned int core_id)
{
  unsigned int core_bit = 0x1 << core_id;
  XT_WER (core_bit, XER_IPI_WAKE_ADDRESS);
}

static inline void
xmp_wake_cores (unsigned int ids)
{
  XT_WER (ids, XER_IPI_WAKE_ADDRESS);
}


static inline unsigned int 
xmp_set_int_level ()
{
  return XTOS_SET_INTLEVEL(XCHAL_NUM_INTLEVELS);
}

static inline void
xmp_restore_int_level (unsigned int intlevel) {
  XTOS_RESTORE_INTLEVEL(intlevel);
}

static inline void
xmp_waiti (void)
{
  XT_WAITI(0);
}

#else /* !XCHAL_HAVE_MP_INTERRUPTS */

static inline void
xmp_wake_cores (unsigned int ids)
{
}


static inline void
xmp_wake_core (unsigned int core_id)
{
}

static inline unsigned int 
xmp_set_int_level ()
{
  return 0;
}


static inline void
xmp_restore_int_level (unsigned int intlevel) 
{
}

static inline void
xmp_waiti (void)
{
}

#endif

void xmp_sleep (unsigned char * address, unsigned char value);
extern FILE * xmp_get_trace_file(void);

static inline int next_queue_index(int idx)
{
  return (idx + 1) & 0x3; //cheaper than modulus
}

static inline unsigned int xmp_time_stamp (void)
{
#if XCHAL_HAVE_CCOUNT
  return XT_RSR_CCOUNT();
#else
  return 1;
#endif
}

extern unsigned int xmp_max_spins;

#ifdef XMP_DEBUG

#define XMP_TRACE(flag, fmt, ...)					\
  {									\
    unsigned int prid = xmp_prid();					\
    unsigned int time = xmp_time_stamp();				\
    FILE * tfile = xmp_get_trace_file();				\
    if (flag && tfile) {						\
      fprintf (tfile, "core%d@%08d: " fmt, prid, time, __VA_ARGS__);	\
    }									\
  }

#else

#define XMP_TRACE(fmt, ...)

#endif

#endif /*__XMP_INTERNAL_H__ */
