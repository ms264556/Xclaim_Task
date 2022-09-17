/* connector for sbrk */

/*
 * Copyright (c) 2004 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

/* Derived from newlib/libc/syscalls/syssbrk.c.  This is included here so
   that calls to sbrk don't pick up the version in libc.a (which could lead
   to libc's version of _sbrk_r being used).  */

#include <reent.h>
#include <unistd.h>

extern void *_sbrk_r (struct _reent *, ptrdiff_t);

void *
sbrk (ptrdiff_t incr)
{
  return _sbrk_r (_REENT, incr);
}
