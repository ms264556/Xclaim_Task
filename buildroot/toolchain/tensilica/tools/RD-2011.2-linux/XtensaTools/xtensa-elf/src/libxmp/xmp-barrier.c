/* Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.  */

#include "xmp-library.h"
#include "xmp-internal.h"

/* State prevents a core from reentering until every core has left.  */
#define  BARRIER_ENTERING 0
#define  BARRIER_LEAVING 1


/* xmp_barrier_init - Initialize a barrier

   Nonsynchronizing, Nonblocking 

   Usage: 
      barrier - pointer to an xmp_barrier_t
   On exit:
      barrier initialized

   Errors: none
*/

int 
xmp_barrier_init (xmp_barrier_t * barrier, int num_cores, const char * name)
{
  barrier->count = 0;
  barrier->name = name;
  barrier->sleeping = 0;
  barrier->num_cores = num_cores;
  barrier->state = 0;
  barrier->trace = 0;
  barrier->system = 0;
  XMP_WRITE_BACK_ELEMENT (barrier);
  return 0;
}


int 
xmp_barrier_wait (xmp_barrier_t * barrier)
{
  int num_left;
  /* Don't enter until all others have left--assume that happens quickly
     so just spin.  */
  while (xmp_atomic_int_value (&barrier->state) == BARRIER_LEAVING)
    xmp_spin();

  XMP_TRACE (barrier->trace, "Arrive %s (%p)\n", 
	      xmp_barrier_name(barrier), barrier);

  xmp_atomic_int_increment(&barrier->count, 1);

  if (xmp_atomic_int_value(&barrier->count) == barrier->num_cores) {

    xmp_atomic_int_increment (&barrier->state, BARRIER_LEAVING);
    num_left = xmp_atomic_int_increment(&barrier->count, -1);

    XMP_TRACE (barrier->trace, "Releasing cores at %s (%p)\n", 
	       xmp_barrier_name(barrier), barrier);
  }
  else {
    while (xmp_atomic_int_value (&barrier->state) != BARRIER_LEAVING)
      xmp_spin ();

    num_left = xmp_atomic_int_increment(&barrier->count, -1);
  }
  
  if (num_left == 0) {
    XMP_TRACE (barrier->trace, "Reset %s (%p)\n", 
	       xmp_barrier_name(barrier), barrier);
    /* Set it to BARRIER_ENTERING  */
    xmp_atomic_int_increment (&barrier->state, -1);
  }

  XMP_TRACE (barrier->trace, "Leave %s (%p)\n", 
	     xmp_barrier_name(barrier), barrier);    

  return 0;
}
