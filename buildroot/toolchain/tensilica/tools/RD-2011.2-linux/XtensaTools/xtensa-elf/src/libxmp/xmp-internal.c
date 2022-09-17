// Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

#include "xmp-library.h"
#include "xmp-internal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int sys_num_cores = 0;
static int xmp_spin_wait_cycles = 16;
static xmp_spin_wait_function_t xmp_wait_func = NULL;

extern void
xmp_spin (void)
{
  if (xmp_wait_func)
    xmp_wait_func();
  else {
    int i;
    for (i = 0; i < xmp_spin_wait_cycles; i++)
      asm volatile ("nop");
  }
}

extern void
xmp_spin_wait_set_cycles (unsigned int limit)
{
  xmp_spin_wait_cycles = limit;
}

extern void
xmp_spin_wait_set_function (xmp_spin_wait_function_t func)
{
  xmp_wait_func = func;
}


#if (XCHAL_HAVE_MP_INTERRUPTS)

/* just clear the interrupt and go.  */
extern void 
xmp_handle_wake_interrupt ()
{
  unsigned int address = XER_MIPICAUSE + xmp_prid ();
  XT_WER (XT_RER(address), address);
}

extern void 
xmp_enable_ipi_interrupts (void)
{
  _xtos_set_interrupt_handler (XER_IPI_WAKE_EXT_INTERRUPT, 
			       xmp_handle_wake_interrupt);
  _xtos_ints_on (0x1 << XER_IPI_WAKE_EXT_INTERRUPT);
}


extern void 
xmp_route_interrupts (unsigned int interrupt_routing)
{
  XT_WER (interrupt_routing, XER_PART);
}

extern void 
xmp_disable_ipi_interrupts (void)
{
  _xtos_ints_off (0x1 << XER_IPI_WAKE_EXT_INTERRUPT);
  _xtos_set_interrupt_handler (XER_IPI_WAKE_EXT_INTERRUPT, NULL);
}

#endif /*XCHAL_HAVE_MP_INTERRUPTS */

extern void 
xmp_init (int num_cores, unsigned int interrupt_routing)
{
  sys_num_cores = num_cores;
  if (xmp_prid() == 0) {
#if XCHAL_HAVE_MP_INTERRUPTS
    if (interrupt_routing == 0)
      interrupt_routing = XER_DEFAULT_IPI_ROUTING;
    xmp_route_interrupts (interrupt_routing);
#endif
    xmp_unpack_shared ();
  }
#if XCHAL_HAVE_MP_INTERRUPTS
  xmp_enable_ipi_interrupts ();
#endif
  xmp_initial_sync (num_cores);
}

extern void 
xmp_end (void)
{
#if XCHAL_HAVE_MP_INTERRUPTS
  xmp_disable_ipi_interrupts ();
#endif
}

extern void * _shared_rom_store_table;
extern char *_shared_bss_start;
extern char *_shared_bss_end;

extern void 
xmp_unpack_shared (void)
{
  void ** table_entry = &_shared_rom_store_table;
  if (table_entry != NULL) {
    do {
      void * start_vaddr = table_entry[0];
      void * end_vaddr = table_entry[1];
      void * source_vaddr = table_entry[2];
      if (start_vaddr < end_vaddr) {
	memcpy (start_vaddr, source_vaddr, end_vaddr - start_vaddr);
	/* Make sure all cores' icaches sees these writes. Cores need
	   to invalidate these regions in their own icache if they
	   might possibly have already executed someting from these
	   locations.

	   At boot time, which is when this code normally runs, no
	   core will have. So this works OK in these circumstances.
	*/
	/* This is overkill--most of these regions won't be
	   executable, but we don't track that.  */
	xthal_dcache_region_writeback (start_vaddr, end_vaddr - start_vaddr);
      }
      table_entry += 3;
    } while (table_entry[0] != 0 || table_entry[1] != 0);
  }
  /* Now zero out the shared-bss.  */
  memset(&_shared_bss_start, 0, 
	 (unsigned char *) &_shared_bss_end
	 - (unsigned char *) &_shared_bss_start);
}

#if XCHAL_DCACHE_IS_COHERENT

unsigned int xmp_max_spins = XCHAL_HAVE_CCOUNT ? 256 : 0;

extern void 
xmp_spin_wait_set (unsigned int limit)
{
  xmp_max_spins = limit;
}

#endif

extern int 
xmp_num_cores (void)
{
  return sys_num_cores;
}

/* observe that there is one of these per core  */
static FILE * trace_file = NULL;

void 
xmp_set_trace_file (FILE * file)
{
  trace_file = file;
}

FILE * 
xmp_get_trace_file (void)
{
  return trace_file;
}

#if XCHAL_DCACHE_IS_COHERENT
extern void 
xmp_mutex_trace_on (xmp_mutex_t * mutex) 
{
  XMP_INVALIDATE_ELEMENT(mutex);
  if (!mutex->system)
    mutex->trace = 1;
  XMP_WRITE_BACK_ELEMENT(mutex);
}

extern void 
xmp_mutex_trace_off (xmp_mutex_t * mutex)
{
  XMP_INVALIDATE_ELEMENT(mutex);
  mutex->trace = 0;
  XMP_WRITE_BACK_ELEMENT(mutex);
}
#endif

extern void 
xmp_barrier_trace_on (xmp_barrier_t * barrier) 
{
  XMP_INVALIDATE_ELEMENT(barrier);
  barrier->trace = 1;
  XMP_WRITE_BACK_ELEMENT(barrier);
}

extern void 
xmp_barrier_trace_off (xmp_barrier_t * barrier)
{
  XMP_INVALIDATE_ELEMENT(barrier);
  barrier->trace = 0;
  XMP_WRITE_BACK_ELEMENT(barrier);
}

