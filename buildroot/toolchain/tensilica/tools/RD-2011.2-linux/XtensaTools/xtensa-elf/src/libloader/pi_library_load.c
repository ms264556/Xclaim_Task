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

static int 
find_align(Elf32_Ehdr * header)
{
  Elf32_Phdr * pheader = (Elf32_Phdr *) (((char *)header) + header->e_phoff);
  int seg = 0;
  int align = 0;
  while (seg < header->e_phnum) {
    if ((pheader[seg].p_type == PT_LOAD) && pheader[seg].p_align > align) {
      align = pheader[seg].p_align;
    }
    seg++;
  }
  return align;  
}


static Elf32_Dyn * 
find_dynamic_info (Elf32_Ehdr * eheader)
{
  int seg = 0;
  
  void * base_addr = (void *) eheader;

  Elf32_Phdr * pheader = base_addr + eheader->e_phoff;
  while (seg < eheader->e_phnum) {
    if (pheader[seg].p_type == PT_DYNAMIC) {
      return base_addr + pheader[seg].p_offset;
    }
    seg++;
  }
  return NULL;
}


static int
load_pi_lib (xtlib_pil_info * lib_info, Elf32_Ehdr * eheader, 
	     memcpy_func mcpy, memset_func mset)
{
  Elf32_Phdr * pheader = (Elf32_Phdr *) (lib_info->src_addr + eheader->e_phoff);
  int seg = 0;
  while (seg < eheader->e_phnum) {
    if (pheader[seg].p_type == PT_LOAD) {
      void * src_addr = lib_info->src_addr + pheader[seg].p_offset;
      void * dst_addr = lib_info->dst_addr + pheader[seg].p_paddr;
      xtlib_load_seg(&pheader[seg], src_addr, dst_addr, mcpy, mset);
      if (lib_info->text_addr == 0) {
	lib_info->text_addr = dst_addr;
      }
    }
    seg++;
  }
  xthal_dcache_sync();
  xthal_icache_sync();
#if XCHAL_HAVE_LOOPS
  asm __volatile__ ("movi a7, 0\n wsr.lcount a7");
#endif
  return XTLIB_NO_ERR;
}


static int
get_dyn_info(Elf32_Ehdr * eheader, void * dst_addr, xtlib_pil_info * info)
{
  unsigned int jmprel = 0;
  unsigned int pltrelsz = 0;
  Elf32_Dyn * dyn_entry = find_dynamic_info(eheader);
  if (dyn_entry == NULL) {
    return XTLIB_NO_DYNAMIC_SEGMENT;
  }

  info->dst_addr = dst_addr;
  info->src_addr = (void *) eheader;
  info->start_sym = dst_addr + eheader->e_entry;
  info->align = find_align(eheader);

  while (dyn_entry->d_tag != DT_NULL) {
    switch (dyn_entry->d_tag) 
      {
      case DT_RELA:
	info->rel = dst_addr + dyn_entry->d_un.d_ptr;
	break;
      case DT_RELASZ:
	info->rela_count = dyn_entry->d_un.d_val / sizeof(Elf32_Rela);
	break;
      case DT_INIT:
	info->init = dst_addr + dyn_entry->d_un.d_ptr;
	break;
      case DT_FINI:
	info->fini = dst_addr + dyn_entry->d_un.d_ptr;
	break;
      case DT_HASH:
	info->hash = dst_addr + dyn_entry->d_un.d_ptr;
	break;
      case DT_SYMTAB:
	info->symtab = dst_addr + dyn_entry->d_un.d_ptr;
	break;
      case DT_STRTAB:
	info->strtab = dst_addr + dyn_entry->d_un.d_ptr;
	break;
      case DT_JMPREL:
	jmprel = dyn_entry->d_un.d_val;
	break;
      case DT_PLTRELSZ:
	pltrelsz = dyn_entry->d_un.d_val;
	break;
      case DT_LOPROC + 2:
	info->text_addr = dst_addr + dyn_entry->d_un.d_ptr;
	break;
	
      default:
	/* do nothing */
	break;
      }
    dyn_entry++;
  }


  return XTLIB_NO_ERR;
}


static int
relocate_pi_lib (xtlib_pil_info * lib_info, Elf32_Ehdr * eheader)
{
  int i;
  Elf32_Rela * relocations = (Elf32_Rela *) lib_info->rel;
  for (i = 0; i < lib_info->rela_count; i++) {
    Elf32_Rela * rela = &relocations[i];
    if (ELF32_R_SYM(rela->r_info) != STN_UNDEF) {
      xtlib_globals.err = XTLIB_UNKNOWN_SYMBOL;
      return XTLIB_UNKNOWN_SYMBOL;
    }
    else {
      unsigned int * addr = (unsigned int *)(lib_info->dst_addr + rela->r_offset);
      *addr = *addr + (unsigned int) lib_info->dst_addr + rela->r_addend;
    }
  }
  return XTLIB_NO_ERR;
}


static int 
validate_dynamic(Elf32_Ehdr * header)
{
  if (xtlib_verify_magic (header) != 0) {
    return XTLIB_NOT_ELF;
  }

  if (header->e_type != ET_DYN) {
    return XTLIB_NOT_DYNAMIC;
  }

  return XTLIB_NO_ERR;
}


void * 
xtlib_load_pi_library(xtlib_packaged_library * library, 
		      void * destination_address, 
		      xtlib_pil_info * lib_info)
{
  return xtlib_load_pi_library_custom_fn (library, 
					  destination_address, 
					  lib_info,
					  xthal_memcpy, 
					  memset);
}

void * 
xtlib_load_pi_library_custom_fn(xtlib_packaged_library * library, 
				void * destination_address, 
				xtlib_pil_info * lib_info,
				memcpy_func mcpy,
				memset_func mset)
{
  Elf32_Ehdr * header = (Elf32_Ehdr *) library;
  unsigned int align = find_align(header);

  int err = validate_dynamic (header);
  if (err != XTLIB_NO_ERR) {
    xtlib_globals.err = err;
    return NULL;
  }
  
  destination_address = (void *) (((unsigned int) (destination_address + align - 1)) & ~(align - 1));

  err = get_dyn_info(header, destination_address, lib_info);
  if (err != XTLIB_NO_ERR) {
    xtlib_globals.err = err;
    return NULL;
  }
    
  err = load_pi_lib(lib_info, header, mcpy, mset);
  if (err != XTLIB_NO_ERR) {
    xtlib_globals.err = err;
    return NULL;
  }

  err = relocate_pi_lib(lib_info, header);
  if (err != XTLIB_NO_ERR) {
    xtlib_globals.err = err;
    return NULL;
  }

  lib_info->init();
  
  return lib_info->start_sym;
}


void
xtlib_unload_pi_library(xtlib_pil_info * lib_info)
{
  lib_info->fini();
}


void *
xtlib_pi_library_debug_addr(xtlib_pil_info * lib_info)
{
  return lib_info->text_addr;
}


unsigned int 
xtlib_pi_library_size(xtlib_packaged_library * library)
{
  Elf32_Phdr * pheader;
  int seg = 0;
  unsigned int bytes = 0;
  Elf32_Ehdr * header = (Elf32_Ehdr *) library;

  int err = validate_dynamic (header);
  if (err != XTLIB_NO_ERR) {
    xtlib_globals.err = err;
    return -1;
  }

  pheader = (Elf32_Phdr *) (library + header->e_phoff);
  while (seg < header->e_phnum) {
    if (pheader[seg].p_type == PT_LOAD
	&& pheader[seg].p_paddr + pheader[seg].p_memsz > bytes) {
      bytes = pheader[seg].p_paddr + pheader[seg].p_memsz;
    }
    seg++;
  }

  bytes += find_align(header);
  return bytes;  
}

