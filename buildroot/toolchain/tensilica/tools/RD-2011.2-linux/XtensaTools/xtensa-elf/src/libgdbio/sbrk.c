/* sbrk.c -- allocate memory dynamically
 *
 * Copyright (c) 2004, 2005, 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <errno.h>

char *heap_ptr;
extern char _end[];	//  _end is set in the linker command file

// This is a variable so that we can override it.
//   Coware crt run-time can do this.
extern char _heap_sentry;	// Defined by the linker
char *_heap_sentry_ptr = &_heap_sentry;

// sbrk -- changes heap size size. Get nbytes more
//         RAM. We just increment a pointer in what's
//         left of memory on the board.

#include <sys/reent.h>

char *
_sbrk_r (ptr, nbytes)
     struct _reent *ptr;
     int nbytes;
{
  char *base;

  if (!heap_ptr)
    heap_ptr = (char *)&_end;
  base = heap_ptr;

  /* The heap is limited by _heap_sentry.
     We're out of memory if we exceed that.  */
  if (heap_ptr + nbytes >= _heap_sentry_ptr)
    {
      ptr->_errno = ENOMEM;
      return (char *) -1;
    }

  heap_ptr += nbytes;

  return base;
}
