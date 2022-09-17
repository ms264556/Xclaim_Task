/* Low-level ELF data structures and other definitions. */

/*
 * Copyright (c) 2003-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#ifndef _ELF_H_
#define _ELF_H_

#define Elf32_Addr   unsigned int
#define Elf32_Half   unsigned short int
#define Elf32_Off    unsigned int
#define Elf32_Sword  signed int
#define Elf32_Word   unsigned int
#define Elf32_Byte   unsigned char

#define EI_NIDENT 16

/* The header is always present in an elf file and will always be first */
typedef struct {
  Elf32_Byte  e_ident[ EI_NIDENT ];  /* Identifier block. see below */
  Elf32_Half  e_type;         /* object file type */
  Elf32_Half  e_machine;      /* machine type */
  Elf32_Word  e_version;      /* elf version */
  Elf32_Addr  e_entry;        /* entry point ( _start ) */
  Elf32_Off   e_phoff;        /* program header offset in bytes */
  Elf32_Off   e_shoff;        /* section header offset in bytes */
  Elf32_Word  e_flags;        /* processor specific flags */
  Elf32_Half  e_ehsize;       /* header size in bytes */
  Elf32_Half  e_phentsize;    /* program header entry size */
  Elf32_Half  e_phnum;        /* program header entry count */
  Elf32_Half  e_shentsize;    /* section header entry size */
  Elf32_Half  e_shnum;        /* section header entry count */
  Elf32_Half  e_shstrndx;     /* index of section name string table */
} Elf32_Ehdr;


/* object file identification */
/* indicies into e_ident */
#define EI_MAG0    0  /* 0-3 are magic '0x7f' */
#define EI_MAG1    1  /* 'E' */
#define EI_MAG2    2  /* 'L' */
#define EI_MAG3    3  /* 'F' */
#define EI_CLASS   4  /* file class */
#define EI_DATA    5  /* data encoding */
#define EI_VERSION 6  /* file version */
#define EI_OSABI      7  /* OS/ABI id */
#define EI_ABIVERSION 8
#define EI_PAD     9  /* start of padding bytes */

/* file class */
#define ELFCLASSNONE  0  /* invalid class */
#define ELFCLASS32    1  /* 32 bit objects */
#define ELFCLASS64    2  /* 64 bit objects */

/* encoding */
#define ELFDATANONE   0  /* invalid encoding */
#define ELFDATA2LSB   1  /* little endian */
#define ELFDATA2MSB   2  /* big endian */

/* object file type */
#define ET_NONE      0    /* no file type */
#define ET_REL       1    /* relocatable file */
#define ET_EXEC      2    /* executable file */
#define ET_DYN       3    /* shared object file */
#define ET_CORE      4    /* core file */
#define ET_LOPROC    0xff00  /* processor specific */
#define ET_HIPROC    0xffff  /* processor specific */

/* machine types */
#define EM_NONE      0    /* no type */
/* other machines are defined but are not included here since this
   reader is only intended for xtensa elf */
#define EM_SPARC        2
#define EM_386          3
#define EM_SPARC32PLUS 18
#define EM_XTENSA      94
#define EM_XTENSA_DEPRECATED  0xabc7

/* version */
#define EV_NONE      0   /* invalid version */
#define EV_CURRENT   1   /* current version */

/*** Sections ***/

/* special section indicies for reserved sections */
#define SHN_UNDEF      0
#define SHN_LORESERVE  0xff00
#define SHN_LOPROC     0xff00
#define SHN_HIPROC     0xff1f
#define SHN_ABS        0xfff1
#define SHN_COMMON     0xfff2
#define SHN_HIRESERVE  0xffff

typedef struct {
  Elf32_Word  sh_name;  /* index into string table for the name of the section */
  Elf32_Word  sh_type;  /* section semantics */
  Elf32_Word  sh_flags; /* misc attributes */
  Elf32_Addr  sh_addr;  /* first byte address of a memory image */
  Elf32_Off   sh_offset; /* offset to section from beginning of file */
  Elf32_Word  sh_size;  /* section size in bytes */
  Elf32_Word  sh_link;  /* table index link */
  Elf32_Word  sh_info;  /* contextual information */
  Elf32_Word  sh_addralign; /* section will be sh_addr % sh_addralign == 0 */
  Elf32_Word  sh_entsize; /* size of entry for fixed size sections */
} Elf32_Shdr;

#define SHT_NULL           0 /* inactive section header */
#define SHT_PROGBITS       1 /* contents defined by program */
#define SHT_SYMTAB         2 /* symbol table */
#define SHT_STRTAB         3 /* string table */
#define SHT_RELA           4 /* relocation section with explicit addends */
#define SHT_HASH           5 /* symbol hash table */
#define SHT_DYNAMIC        6 /* dynamic link info */
#define SHT_NOTE           7 /* comment section */
#define SHT_NOBITS         8 /* section occupies no space */
#define SHT_REL            9 /* relocation section with no explicit addends */
#define SHT_SHLIB         10 /* reserved with unknown semantics */
#define SHT_DYNSYM        11 /* symbol table */
#define SHT_INIT_ARRAY    14 /* initialization functions */
#define SHT_FINI_ARRAY    15 /* termination functions */
#define SHT_PREINIT_ARRAY 16 /* preinitialization functions */
#define SHT_GROUP         17 /* grouping section */
#define SHT_SYMTAB_SHNDX  18 /* special symbol table index */

#define SHT_LOOS     0x60000000   /* OS specific semantics */    
#define SHT_HIOS     0x6fffffff  
#define SHT_LOPROC   0x70000000   /* processor specific semantics */
#define SHT_HIPROC   0x7fffffff
#define SHT_LOUSER   0x80000000   /* lower bound of reserved indicies for apps */
#define SHT_HIUSER   0xffffffff   /* upper bound of same */

#define SHT_XTENSA_AUTOTIE 0x7f000000  /* AutoTIE analysis information. */

/* flags */
#define SHF_WRITE     0x1    /* writable section */
#define SHF_ALLOC     0x2    /* will reside in the memory image of the process */
#define SHF_EXECINSTR 0x4    /* executable section */
#define SHF_MASKPROC  0xf0000000 /* processor specific semantics */


/* symbol tables */
typedef struct {
  Elf32_Word    st_name;   /* string table index to name of symbol */
  Elf32_Addr    st_value;  /* symbol value */
  Elf32_Word    st_size;   /* size of data object */
  Elf32_Byte    st_info;   /* symbol type and binding attributes */
  Elf32_Byte    st_other;  /* not currently defined by ELF */
  Elf32_Half    st_shndx;  /* section index for the symbol */
} Elf32_Sym;

/* use these macros to extract information from st_info */
#define ELF32_ST_BIND(i)    ( (i) >> 4 )
#define ELF32_ST_TYPE(i)    ( (i) & 0xf )
#define ELF32_ST_INFO(b,t)  ( ( (b) << 4 ) + ( (t) & 0xf ) )

/* symbol bindings */
#define STB_LOCAL   0   /* visible only in current object file */
#define STB_GLOBAL  1   /* visible to all object files */
#define STB_WEAK    2   /* global, but may be redefined by later symbol */
#define STB_LOPROC 13   /* reserved for processor specific semantics */
#define STB_HIPROC 15

/* symbol type */
#define STT_NOTYPE  0  /* no type specified */
#define STT_OBJECT  1  /* data object, e.g. variable, array */
#define STT_FUNC    2  /* executable code */
#define STT_SECTION 3  /* assoc. with a section ( used for relocation ) */
#define STT_FILE    4  /* symbol is name of source file */
#define STT_LOPROC 13  /* reserved for processor specific semantics */
#define STT_HIPROC 15

/* Special symbol table index to indicate the end of a hash table
 * chain.
 */
#define STN_UNDEF 0

/*** relocation ***/

typedef struct {
  Elf32_Addr   r_offset;
  Elf32_Word   r_info;
} Elf32_Rel;

typedef struct {
  Elf32_Addr   r_offset;
  Elf32_Word   r_info;
  Elf32_Sword  r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(i)  ((i)>>8)
#define ELF32_R_TYPE(i)  ((Elf32_Byte)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(Elf32_Byte)(t))

/*** program header ***/

typedef struct {
  Elf32_Word  p_type;     /* type of segment */
  Elf32_Off   p_offset;   /* offset from beginning of file to segment */
  Elf32_Addr  p_vaddr;    /* virtual address of first byte */
  Elf32_Addr  p_paddr;    /* physical address ( if relevant ) */
  Elf32_Word  p_filesz;   /* number of bytes in the file image for segment */
  Elf32_Word  p_memsz;    /* number of bytes in the memory image "  "   */
  Elf32_Word  p_flags;    /* segment flags */
  Elf32_Word  p_align;    /* power of 2 alignment constant */
} Elf32_Phdr;

#define PT_NULL    0  /* unused entry */
#define PT_LOAD    1  /* loadable segment */
#define PT_DYNAMIC 2  /* dynamic linking info */
#define PT_INTERP  3  /* pointer to path to interpreter */
#define PT_NOTE    4  /* auxilary information */
#define PT_SHLIB   5  /* reserved */
#define PT_PHDR    6  /* pointer to program header */
#define PT_LOPROC  0x70000000 /* reserved */
#define PT_HIPROC  0x7fffffff

/* flag values */
#define PF_X   0x1 /* Execute */
#define PF_W   0x2 /* Write */
#define PF_R   0x4 /* Read */
#define PF_MASKOS   0x0ff00000  /* Unspecified */
#define PF_MASKPROC 0xf0000000  /* Unspecified */

/* dynamic section entries */

typedef struct {
  Elf32_Sword	d_tag;
  union {
    Elf32_Word	d_val;
    Elf32_Addr	d_ptr;
  } d_un;
} Elf32_Dyn;

#define DT_NULL       0
#define DT_NEEDED     1
#define DT_PLTRELSZ   2
#define DT_PLTGOT     3
#define DT_HASH       4
#define DT_STRTAB     5
#define DT_SYMTAB     6
#define DT_RELA       7
#define DT_RELASZ     8
#define DT_RELAENT    9
#define DT_STRSZ      10
#define DT_SYMENT     11
#define DT_INIT       12
#define DT_FINI       13
#define DT_SONAME     14
#define DT_RPATH      15
#define DT_SYMBOLIC   16
#define DT_REL        17
#define DT_RELSZ      18
#define DT_RELENT     19
#define DT_PLTREL     20
#define DT_DEBUG      21
#define DT_TEXTREL    22
#define DT_JMPREL     23
#define DT_LOPROC     0x70000000
#define DT_HIPROC     0x7fffffff



#endif /* ! _ELF_H_ */
