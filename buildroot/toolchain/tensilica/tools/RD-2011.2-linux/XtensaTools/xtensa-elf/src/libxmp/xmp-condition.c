/* Copyright (c) 1999-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of
   Tensilica Inc.  They may not be modified, copied, reproduced,
   distributed, or disclosed to third parties in any manner, medium,
   or form, in whole or in part, without the prior written consent of
   Tensilica Inc.
*/

#include <limits.h>
#include "xmp-library.h"
#include "xmp-internal.h"

#if XCHAL_DCACHE_IS_COHERENT

typedef xmp_condition_t less_aligned_condition_t __attribute__ ((aligned (4)));

extern int 
xmp_condition_init (xmp_condition_t * condition, const char * name)
{
  less_aligned_condition_t temp = XMP_CONDITION_INITIALIZER(name);
  *condition = temp;
  return 0;
}


extern int 
xmp_condition_destroy (xmp_condition_t * condition)
{
  return 0;
}

extern int 
xmp_condition_wait (xmp_condition_t * condition, 
		     xmp_mutex_t * mutex)
{
  unsigned int prid = xmp_prid ();
  int new_tail;
  unsigned int intlevel;
  unsigned int enter_time;

  if (mutex->owner != prid)
    return XMP_MUTEX_ERROR_NOT_OWNED;

  new_tail = next_queue_index(condition->qtail);
  condition->queue[new_tail] = prid;
  condition->waiting[prid] = 1;
  condition->qtail = new_tail;
  
  xmp_mutex_unlock(mutex);

  enter_time = xmp_time_stamp();
  intlevel = xmp_set_int_level();
  while (xmp_coherent_l32ai(&condition->waiting[prid]) == 1) {
    /* Must leave this in a loop in case we get a spurious interrupt.  */
    if (xmp_time_stamp() - enter_time >= xmp_max_spins) {
      xmp_waiti ();
      enter_time = xmp_time_stamp();
    }
  }
  xmp_restore_int_level(intlevel);

  xmp_mutex_lock(mutex);
  return 0;
}

extern int 
xmp_condition_signal (xmp_condition_t * condition)
{
  unsigned char target_core;
  int new_head;

  target_core = condition->queue[condition->qhead];
  if (target_core != XMP_NO_OWNER) {
    condition->queue[condition->qhead] = XMP_NO_OWNER;
    new_head = next_queue_index (condition->qhead);
    condition->waiting[target_core] = 0;
    condition->qhead = new_head;
    xmp_wake_core (target_core);
  }

  return 0;
}

extern int 
xmp_condition_broadcast (xmp_condition_t * condition)
{
  unsigned char target_core;

  for (target_core = 0; target_core < XMP_MAX_CORES; target_core++) {
    condition->queue[target_core] = XMP_NO_OWNER;
    if (condition->waiting[target_core] != 0) {
      condition->waiting[target_core] = 0;
      xmp_wake_core (target_core);
    }
  }
  condition->qhead = 0;
  condition->qtail = -1;
  return 0;
}

#endif 
