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

xtlib_loader_globals xtlib_globals;

int
xtlib_verify_magic( Elf32_Ehdr *header )
{
  Elf32_Byte magic_no;

  magic_no =  header->e_ident[EI_MAG0];
  if (magic_no != 0x7f) {
    return -1;
  }

  magic_no = header->e_ident[EI_MAG1];
  if ( magic_no != 'E' ) {
    return -1;
  }

  magic_no = header->e_ident[EI_MAG2];
  if ( magic_no != 'L' ) {
    return -1;
  }

  magic_no = header->e_ident[EI_MAG3];
  if ( magic_no != 'F' ) {
    return -1;
  }

  return 0;
}


void
xtlib_load_seg(Elf32_Phdr * pheader, void * src_addr, void * dst_addr, 
	       memcpy_func mcpy, memset_func mset)
{
  Elf32_Word bytes_to_copy = pheader->p_filesz;
  Elf32_Word bytes_to_zero = pheader->p_memsz - pheader->p_filesz;

  void * zero_addr = dst_addr + pheader->p_filesz;

  mcpy(dst_addr, src_addr, bytes_to_copy);

  if (pheader->p_flags & PF_X) {
    xthal_dcache_region_writeback (dst_addr, bytes_to_copy);
    xthal_icache_region_invalidate (dst_addr, bytes_to_copy);
  }

  if (bytes_to_zero > 0)
    mset(zero_addr, 0, bytes_to_zero);
}


unsigned int 
xtlib_error(void)
{
  return xtlib_globals.err;
}
