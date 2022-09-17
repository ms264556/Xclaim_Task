/* ISS simcalls used by ferret.  */

/*
 * Copyright (c) 2004-2005 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include "xt-ferret.h"
#include "simcall.h"

void
__simc_no_heap (char *sentry, int incr)
{
  register int a2 __asm__ ("a2") = SYS_no_heap;
  register char *a3 __asm__ ("a3") = sentry;
  register int a4 __asm__ ("a4") = incr;
  register int ret_val __asm__ ("a2");
  register int ret_err __asm__ ("a3");

  __asm__ volatile ("simcall"
		    : "=a" (ret_val), "=a" (ret_err)
		    : "a" (a2), "a" (a3), "a" (a4));
}


void
__simc_more_heap (char *end, int incr)
{
  register int a2 __asm__ ("a2") = SYS_more_heap;
  register char *a3 __asm__ ("a3") = end;
  register int a4 __asm__ ("a4") = incr;
  register int ret_val __asm__ ("a2");
  register int ret_err __asm__ ("a3");

  __asm__ volatile ("simcall"
		    : "=a" (ret_val), "=a" (ret_err)
		    : "a" (a2), "a" (a3), "a" (a4));
}


void
__simc_malloc (void *mem, int bytes)
{
  register int a2 __asm__ ("a2") = SYS_malloc;
  register void *a3 __asm__ ("a3") = mem;
  register int a4 __asm__ ("a4") = bytes;
  register int ret_val __asm__ ("a2");
  register int ret_err __asm__ ("a3");

  __asm__ volatile ("simcall"
		    : "=a" (ret_val), "=a" (ret_err)
		    : "a" (a2), "a" (a3), "a" (a4));
}


void
__simc_free (void *mem)
{
  register int a2 __asm__ ("a2") = SYS_free;
  register void *a3 __asm__ ("a3") = mem;
  register int ret_val __asm__ ("a2");
  register int ret_err __asm__ ("a3");

  __asm__ volatile ("simcall"
		    : "=a" (ret_val), "=a" (ret_err)
		    : "a" (a2), "a" (a3));
}


void
__simc_enter_ferret ()
{
  register int a2 __asm__ ("a2") = SYS_enter_ferret;
  register int ret_val __asm__ ("a2");
  register int ret_err __asm__ ("a3");

  __asm__ volatile ("simcall"
		    : "=a" (ret_val), "=a" (ret_err)
		    : "a" (a2));
}


void
__simc_leave_ferret ()
{
  register int a2 __asm__ ("a2") = SYS_leave_ferret;
  register int ret_val __asm__ ("a2");
  register int ret_err __asm__ ("a3");

  __asm__ volatile ("simcall"
		    : "=a" (ret_val), "=a" (ret_err)
		    : "a" (a2));
}
