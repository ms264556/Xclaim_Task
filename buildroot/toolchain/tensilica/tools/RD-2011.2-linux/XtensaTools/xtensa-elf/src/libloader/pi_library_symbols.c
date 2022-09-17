/* Copyright (c) 1999-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.
*/

#include <string.h>
#include "elf.h"
#include "xt_library_loader.h"
#include "loader_internal.h"
#include <xtensa/hal.h>


static Elf32_Word
elf_hash(const unsigned char *name)
{
  Elf32_Word h = 0, g;
  while (*name)
    {
      h = (h << 4) + *name++;
      g = h & 0xf0000000;
      if (g)
	h ^= g >> 24;
      h &= ~g;
    }
  return h;
}



void * 
xtlib_lookup_pi_library_symbol(xtlib_pil_info * lib_info, const char * symname)
{
  Elf32_Word * entries = lib_info->hash;
  Elf32_Word nbuckets = *entries;
  
  Elf32_Word * buckets = entries + 2;
  Elf32_Word * chains = buckets + nbuckets;

  Elf32_Word bucket = elf_hash((const unsigned char * )symname) % nbuckets;
  Elf32_Sym * symbols = (Elf32_Sym *) lib_info->symtab;
  
  while (bucket != STN_UNDEF) {
    Elf32_Word idx = buckets[bucket];
    Elf32_Sym * candidate = &symbols[idx];
    
    const char * candname = lib_info->strtab + candidate->st_name;
    if (strcmp (candname, symname) == 0)
      return candidate->st_value + lib_info->dst_addr;
    bucket = chains[idx];

  }
  
  return NULL;
}


