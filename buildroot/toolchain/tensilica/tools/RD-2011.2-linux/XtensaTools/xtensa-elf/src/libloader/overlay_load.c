/* Copyright (c) 1999-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.
*/

#include "elf.h"
#include "xt_library_loader.h"
#include "loader_internal.h"
#include <xtensa/hal.h>
#include <string.h>

static void *
load_static_lib (Elf32_Ehdr * eheader, memcpy_func mcpy, memset_func mset)
{
  char * lib_addr = (char *) eheader;
  Elf32_Phdr * pheader = (Elf32_Phdr *) (lib_addr + eheader->e_phoff);
  int seg = 0;
  while (seg < eheader->e_phnum) {
    if (pheader[seg].p_type == PT_LOAD) {
      void * src_addr = lib_addr + pheader[seg].p_offset;
      void * dst_addr = (void *) pheader[seg].p_paddr;
      xtlib_load_seg(&pheader[seg], src_addr, dst_addr, mcpy, mset);
    }
    seg++;
  }

  xthal_dcache_sync();
  xthal_icache_sync();
#if XCHAL_HAVE_LOOPS
  asm __volatile__ ("movi a7, 0\n wsr.lcount a7");
#endif
  return (void *) eheader->e_entry;
}


void * 
xtlib_load_overlay( xtlib_packaged_library * library)
{
  return xtlib_load_overlay_custom_fn (library, xthal_memcpy, memset);
}


void * 
xtlib_load_overlay_custom_fn( xtlib_packaged_library * library, 
			      memcpy_func mcpy, 
			      memset_func mset)
{
  Elf32_Ehdr * header = (Elf32_Ehdr *) library;

  if (xtlib_verify_magic (header) != 0) {
    xtlib_globals.err = XTLIB_NOT_ELF;
    return 0;
  }

  if (header->e_type == ET_DYN) {
    xtlib_globals.err = XTLIB_NOT_STATIC;
    return 0;
  }

  return load_static_lib(header, mcpy, mset);
}
