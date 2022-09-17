/* sbrk system call for the Xtensa semihosting simulator.  */

/*
 * Copyright (c) 2004 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <errno.h>
#include <sys/reent.h>


void *
_sbrk_r (struct _reent *reent, int incr)
{
  static char *heap_end;
  void *prev_heap_end;
  extern char _end, _heap_sentry, __stack; /* defined by the linker */
  char *stack_ptr;

  if (heap_end == 0)
    heap_end = &_end;
  prev_heap_end = heap_end;

  /* We have a heap and stack collision if the stack started above the
     heap and is now below the end of the heap.  */
  __asm__ ("mov %0, sp" : "=a" (stack_ptr));
  if (&__stack > &_end && heap_end + incr > stack_ptr)
    {
      reent->_errno = ENOMEM;
      return (void *) -1;
    }

  /* The heap is limited by _heap_sentry -- we're out of memory if we
     exceed that.  */
  if (heap_end + incr >= &_heap_sentry)
    {
      reent->_errno = ENOMEM;
      return (void *) -1;
    }

  heap_end += incr;
  return prev_heap_end;
}
