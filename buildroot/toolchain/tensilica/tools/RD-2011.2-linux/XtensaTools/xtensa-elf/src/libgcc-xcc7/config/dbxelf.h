
/* 
   Copyright (C) 2001 Tensilica, Inc.  All Rights Reserved.
   Revised to support Tensilica processors and to improve overall performance
 */

/* Definitions needed when using stabs embedded in ELF sections.
   Copyright (C) 1999 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* This file may be included by any ELF target which wishes to
   support -gstabs generating stabs in sections, as produced by gas
   and understood by gdb.  */

#ifndef __DBX_ELF_H
#define __DBX_ELF_H

/* Output DBX (stabs) debugging information if doing -gstabs.  */

#undef  DBX_DEBUGGING_INFO
#define DBX_DEBUGGING_INFO

/* Make LBRAC and RBRAC addresses relative to the start of the
   function.  The native Solaris stabs debugging format works this
   way, gdb expects it, and it reduces the number of relocation
   entries...  */

#undef  DBX_BLOCKS_FUNCTION_RELATIVE
#define DBX_BLOCKS_FUNCTION_RELATIVE 1

/* ... but, to make this work, functions must appear prior to line info.  */

#undef  DBX_FUNCTION_FIRST
#define DBX_FUNCTION_FIRST

/* When generating stabs debugging, use N_BINCL entries.  */

#undef  DBX_USE_BINCL
#define DBX_USE_BINCL

/* There is no limit to the length of stabs strings.  */

#ifndef DBX_CONTIN_LENGTH
#define DBX_CONTIN_LENGTH 0
#endif

/* When using stabs, gcc2_compiled must be a stabs entry, not an
   ordinary symbol, or gdb won't see it.  Furthermore, since gdb reads
   the input piecemeal, starting with each N_SO, it's a lot easier if
   the gcc2 flag symbol is *after* the N_SO rather than before it.  So
   we emit an N_OPT stab there.  */

#define ASM_IDENTIFY_GCC(FILE)						\
do									\
  {									\
    if (write_symbols != DBX_DEBUG)					\
      fputs ("gcc2_compiled.:\n", FILE);				\
  }									\
while (0)

#define ASM_IDENTIFY_GCC_AFTER_SOURCE(FILE)				\
do									\
  {									\
    if (write_symbols == DBX_DEBUG)					\
      fputs ("\t.stabs\t\"gcc2_compiled.\", 0x3c, 0, 0, 0\n", FILE);	\
  }									\
while (0)

/* Like block addresses, stabs line numbers are relative to the
   current function.  */

#undef  ASM_OUTPUT_SOURCE_LINE
#define ASM_OUTPUT_SOURCE_LINE(FILE, LINE)				\
do									\
  {									\
    static int sym_lineno = 1;						\
    char temp[256];							\
    ASM_GENERATE_INTERNAL_LABEL (temp, "LM", sym_lineno);		\
    fprintf (FILE, "\t.stabn 68,0,%d,", LINE);				\
    assemble_name (FILE, temp);						\
    putc ('-', FILE);							\
    assemble_name (FILE,						\
		   XSTR (XEXP (DECL_RTL (current_function_decl), 0), 0));\
    putc ('\n', FILE);							\
    ASM_OUTPUT_INTERNAL_LABEL (FILE, "LM", sym_lineno);			\
    sym_lineno += 1;							\
  }									\
while (0)

/* Generate a blank trailing N_SO to mark the end of the .o file, since
   we can't depend upon the linker to mark .o file boundaries with
   embedded stabs.  */

#undef  DBX_OUTPUT_MAIN_SOURCE_FILE_END
#define DBX_OUTPUT_MAIN_SOURCE_FILE_END(FILE, FILENAME)			\
  asm_fprintf (FILE,							\
	       "\t.text\n\t.stabs \"\",%d,0,0,%LLetext\n%LLetext:\n", N_SO)

#endif /* __DBX_ELF_H */
