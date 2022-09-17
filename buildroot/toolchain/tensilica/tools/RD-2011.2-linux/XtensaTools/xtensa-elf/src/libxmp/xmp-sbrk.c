/* sbrk.c -- allocate memory dynamically.
 * 
 * Copyright (c) 1995,1996 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include <errno.h>
#include <xtensa/config/core-isa.h>
#include <xtensa/config/system.h>
#if (XSHAL_CLIB == XTHAL_CLIB_NEWLIB)
#include "xmp-library.h"
#include <reent.h>

//#include "glue.h"

#define REENTRANT_SYSCALLS_PROVIDED

#define heap_ptr shared_heap_ptr
#define _heap_sentry _shared_heap_sentry
#define _end _shared_end
#define _heap_sentry_ptr _shared_heap_sentry_ptr
#define sbrk shared_sbrk

#define _sbrk_r _xmp_sbrk_r

extern struct _reent *reent_ptr;
extern char *heap_ptr;

/* This is a variable so that we can override it.
   Coware crt run-time can do this.  */
extern char _heap_sentry;	/* Defined by the linker */
extern char _end;	        /* Defined by the linker */
extern char *_heap_sentry_ptr;


/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */

#ifndef REENTRANT_SYSCALLS_PROVIDED

char *
sbrk (nbytes)
     int nbytes;
{
  char        *base;

  if (!heap_ptr)
    heap_ptr = (char *)&_end;
  base = heap_ptr;

  /* The heap is limited by _heap_sentry.
     We're out of memory if we exceed that.  */
  if (heap_ptr + nbytes >= _heap_sentry_ptr)
    {
      errno = ENOMEM;
      return (char *) -1;
    }

  heap_ptr += nbytes;

  return base;
}

#else /* REENTRANT_SYSCALLS_PROVIDED */

#include <sys/reent.h>

#if !XCHAL_DCACHE_IS_COHERENT
extern xmp_atomic_int_t xmp_sbrk_lock;
#endif

char *
_sbrk_r (ptr, nbytes)
     struct _reent *ptr;
     int nbytes;
{
  char *base;
  
#if !XCHAL_DCACHE_IS_COHERENT
  /* _end, _heap_sentry_ptr are constant in a program, so don't need invalidation.  */
  XMP_SIMPLE_SPINLOCK_ACQUIRE(&xmp_sbrk_lock);
  XMP_INVALIDATE_ELEMENT(&heap_ptr);
#endif

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

#if !XCHAL_DCACHE_IS_COHERENT
  XMP_WRITE_BACK_ELEMENT(&heap_ptr);
  XMP_SIMPLE_SPINLOCK_RELEASE(&xmp_sbrk_lock);
#endif
  return base;
}

void *
xmp_sbrk (int nbytes)
{
  return _xmp_sbrk_r(_REENT, nbytes);
}

#endif /* REENTRANT_SYSCALLS_PROVIDED */
#endif /* CLIBRARY */
