/* Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.  */

#include <limits.h>
#include "xmp-library.h"
#include "xmp-internal.h"

#if XCHAL_DCACHE_IS_COHERENT

typedef xmp_mutex_t less_aligned_mutex_t __attribute__ ((aligned (4)));

int 
xmp_mutex_init (xmp_mutex_t * mutex, const char * name, 
			  unsigned int recursive)
{
  less_aligned_mutex_t m = XMP_MUTEX_INITIALIZER(name);
  *mutex = m;
  mutex->recursive = recursive;
  XMP_WRITE_BACK_ELEMENT(mutex);
  return 0;
}


int 
xmp_mutex_destroy (xmp_mutex_t * mutex)
{
  return 0;
}

const char * 
xmp_mutex_name (const xmp_mutex_t * mutex)
{
  XMP_INVALIDATE_ELEMENT((xmp_mutex_t *) mutex);
  return mutex->name;
}

/* Increment a mutex's counter already owned by this core. We can 
   be sure that no other core will acquire it, so we can just update
   and go. 

   Be sure that the mutex is held by the current core before
   calling this function.
*/

static int
recursive_mutex_acquire (xmp_mutex_t * mutex)
{
  mutex->held ++;
  XMP_TRACE (mutex->trace, "Acquired %s (%p)--recursive %d times\n", 
	      xmp_mutex_name(mutex), mutex, mutex->held);
  XMP_WRITE_BACK_ELEMENT(mutex);
  return 0;
}


/* We don't strictly need an owner field--the core at the head of the
   queue is the owner. However, having it separate means we don't need
   to synchronize the queue and with reading the owner.  */

int 
xmp_mutex_lock (xmp_mutex_t * mutex)
{
  unsigned char prid = xmp_prid ();
  int new_tail;
  unsigned int enter_time;

  /* It is safe to check if the current core is the owner here because
     we can be sure that it won't interrupt itself and release.  If I'm the owner
     then I have the most recent copy too.  */
  XMP_INVALIDATE_ELEMENT(mutex);
  if (mutex->owner == prid && mutex->recursive)
    return recursive_mutex_acquire (mutex);

  XMP_TRACE (mutex->trace, "Attempt %s (%p)\n", xmp_mutex_name(mutex), mutex);

  XMP_SIMPLE_SPINLOCK_ACQUIRE(&mutex->qlock);
  XMP_INVALIDATE_ELEMENT(mutex);
  new_tail = next_queue_index(mutex->qtail);
  mutex->queue[new_tail] = prid;
  mutex->qtail = new_tail;
  XMP_WRITE_BACK_ELEMENT(mutex);
  XMP_SIMPLE_SPINLOCK_RELEASE(&mutex->qlock);

  enter_time = xmp_time_stamp();
  while (mutex->queue[xmp_coherent_l32ai(&mutex->qhead)] != prid) {
#if XCHAL_HAVE_MP_INTERRUPTS
    /* Must leave this in a loop in case we get a spurious interrupt.  */
    XMP_TRACE (mutex->trace, "Sleep while waiting on %s (%p)\n", 
		xmp_mutex_name(mutex), mutex);

    if (xmp_time_stamp() - enter_time >= xmp_max_spins) {
      unsigned int intlevel = xmp_set_int_level(XCHAL_NUM_INTLEVELS);
      
      if (mutex->queue[xmp_coherent_l32ai(&mutex->qhead)] != prid) {
	xmp_waiti();
      }
      xmp_restore_int_level(intlevel);
      enter_time = xmp_time_stamp();
    }
#endif
  }

  mutex->owner = prid;
  mutex->held++;
  XMP_WRITE_BACK_ELEMENT(mutex);

  XMP_TRACE (mutex->trace, "Acquired %s (%p)\n", xmp_mutex_name(mutex), mutex);

  return 0;
}


int 
xmp_mutex_trylock (xmp_mutex_t * mutex)
{
  unsigned char prid = xmp_prid ();
  int error = 0;
  int new_tail;

  /* It is safe to check if the current core is the owner here because
     we can be sure that it won't interrupt itself and release.  */

  XMP_INVALIDATE_ELEMENT(mutex);
  if (mutex->owner == prid && mutex->recursive)
    return recursive_mutex_acquire (mutex);

  XMP_TRACE (mutex->trace, "Attempt %s (%p)\n", xmp_mutex_name(mutex), mutex);

  if (xmp_atomic_int_conditional_set (&mutex->qlock, 0, 1) == 0) {
    XMP_INVALIDATE_ELEMENT(mutex);
    if (mutex->queue[mutex->qhead] == XMP_NO_OWNER) {
      /* Then the head will be the same as the tail....  */
      new_tail = next_queue_index(mutex->qtail);
      mutex->queue[new_tail] = prid;
      mutex->qtail = new_tail;
      
      mutex->owner = prid;
      mutex->held++;
      
      XMP_TRACE (mutex->trace, "Acquired %s (%p)\n", xmp_mutex_name(mutex), mutex);
    }
    else
      error = XMP_MUTEX_ACQUIRE_FAILED;

    XMP_WRITE_BACK_ELEMENT(mutex);
    XMP_SIMPLE_SPINLOCK_RELEASE (&mutex->qlock);
  }
  else
    error = XMP_MUTEX_ACQUIRE_FAILED;

  /* owner might have changed by now... but this is our best
     guess.  */
  if (error)
    XMP_TRACE (mutex->trace, "Attempt %s (%p) failed--held by core%d\n", 
		xmp_mutex_name(mutex), mutex, mutex->owner);
  return error;
}


int 
xmp_mutex_unlock (xmp_mutex_t * mutex)
{
  /* It is OK to check the owner of a lock without acquiring it.  A
     processor can be sure that it won't acquire the lock during the
     check, and all that matters is whether or not the processor
     itself owns it.

     If we allowed interrupt levels to share locks--which amounts to
     supporting multiple threads per processor--this option wouldn't
     work.
  */
  int prid = xmp_prid ();

  XMP_INVALIDATE_ELEMENT (mutex);
  if (mutex->owner != prid)
    return XMP_MUTEX_ERROR_NOT_OWNED;
  else {
    mutex->held--;
    if (mutex->held == 0) {
      int new_head;
      
      mutex->owner = XMP_NO_OWNER;
      XMP_SIMPLE_SPINLOCK_ACQUIRE(&mutex->qlock);
      
      new_head = next_queue_index(mutex->qhead);
      mutex->queue[mutex->qhead] = XMP_NO_OWNER;
      mutex->qhead = new_head;
      if (mutex->queue[new_head] != XMP_NO_OWNER)
	xmp_wake_core (mutex->queue[new_head]);

      XMP_WRITE_BACK_ELEMENT(mutex);
      XMP_SIMPLE_SPINLOCK_RELEASE(&mutex->qlock);

      XMP_TRACE (mutex->trace, "Released %s (%p)\n", xmp_mutex_name(mutex), mutex);
    }
    return 0;
  }
}

#endif /* XCHAL_DCACHE_IS_COHERENT */

