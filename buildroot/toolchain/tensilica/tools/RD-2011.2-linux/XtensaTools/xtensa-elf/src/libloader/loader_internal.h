/* Copyright (c) 1999-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
   These coded instructions, statements, and computer programs are the
   copyrighted works and confidential proprietary information of Tensilica Inc.
   They may not be modified, copied, reproduced, distributed, or disclosed to
   third parties in any manner, medium, or form, in whole or in part, without
   the prior written consent of Tensilica Inc.
*/

typedef struct xtlib_loader_globals 
{
  int err;
} xtlib_loader_globals;

extern xtlib_loader_globals xtlib_globals;

int
xtlib_verify_magic(Elf32_Ehdr *header);
void
xtlib_load_seg(Elf32_Phdr * pheader, void * src_addr, void * dst_addr, 
	       memcpy_func mcpy, memset_func mset);

