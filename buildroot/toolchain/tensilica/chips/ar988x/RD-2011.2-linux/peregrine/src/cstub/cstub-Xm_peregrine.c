/*
 * Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the.
 * copyrighted works and confidential proprietary information of Tensilica Inc..
 * They may not be modified, copied, reproduced, distributed, or disclosed to.
 * third parties in any manner, medium, or form, in whole or in part, without.
 * the prior written consent of Tensilica Inc..
 */

/* Do not modify. This is automatically generated.*/

/* Included files */
#include "cstub-Xm_peregrine.h"
#include <stdlib.h>
#include <stdio.h>

#if defined(__GNUC__)
#if defined(__LP64__) 
#error "Error: Compiling c-stub on 64 bit machines is not supported"
#endif /* __LP64__ */ 
#define INLINE inline 
#elif defined(_MSC_VER)
#if defined(_WIN64) 
#error "Error: Compiling c-stub on 64 bit machines is not supported"
#endif /* _WIN64 */
#define INLINE __inline 
#else 
#error "Error: Only GCC/G++ and Visual C++ are supported"
#endif

#define CSTUB_PROC CSTUB_(proc) 
/* Processor states */
typedef struct CSTUB_(states) {
unsigned XTSYNC;
unsigned EPC1;
unsigned EPC2;
unsigned EPC3;
unsigned EPC4;
unsigned EPC5;
unsigned EXCSAVE1;
unsigned EXCSAVE2;
unsigned EXCSAVE3;
unsigned EXCSAVE4;
unsigned EXCSAVE5;
unsigned EPS2;
unsigned EPS3;
unsigned EPS4;
unsigned EPS5;
unsigned EXCCAUSE;
unsigned PSINTLEVEL;
unsigned PSUM;
unsigned PSWOE;
unsigned PSEXCM;
unsigned DEPC;
unsigned EXCVADDR;
unsigned WindowBase;
unsigned WindowStart;
unsigned PSCALLINC;
unsigned PSOWB;
unsigned LBEG;
unsigned LEND;
unsigned SAR;
unsigned LITBADDR;
unsigned LITBEN;
unsigned MISC0;
unsigned MISC1;
unsigned MISC2;
unsigned MISC3;
unsigned InOCDMode;
unsigned INTENABLE;
unsigned DBREAKA0;
unsigned DBREAKC0;
unsigned DBREAKA1;
unsigned DBREAKC1;
unsigned IBREAKA0;
unsigned IBREAKA1;
unsigned IBREAKENABLE;
unsigned ICOUNTLEVEL;
unsigned DEBUGCAUSE;
unsigned DBNUM;
unsigned CCOMPARE0;
} CSTUB_(states_t);

/* Processor struct */
typedef struct CSTUB_(processor) {
  CSTUB_(states_t) states;
  void *last;
} CSTUB_(processor_t);

static CSTUB_(processor_t) CSTUB_(proc) = {
/* START states */
{
0x0, 	/* XTSYNC */
0x0, 	/* EPC1 */
0x0, 	/* EPC2 */
0x0, 	/* EPC3 */
0x0, 	/* EPC4 */
0x0, 	/* EPC5 */
0x0, 	/* EXCSAVE1 */
0x0, 	/* EXCSAVE2 */
0x0, 	/* EXCSAVE3 */
0x0, 	/* EXCSAVE4 */
0x0, 	/* EXCSAVE5 */
0x0, 	/* EPS2 */
0x0, 	/* EPS3 */
0x0, 	/* EPS4 */
0x0, 	/* EPS5 */
0x0, 	/* EXCCAUSE */
0xf, 	/* PSINTLEVEL */
0x0, 	/* PSUM */
0x0, 	/* PSWOE */
0x1, 	/* PSEXCM */
0x0, 	/* DEPC */
0x0, 	/* EXCVADDR */
0x0, 	/* WindowBase */
0x1, 	/* WindowStart */
0x0, 	/* PSCALLINC */
0x0, 	/* PSOWB */
0x0, 	/* LBEG */
0x0, 	/* LEND */
0x0, 	/* SAR */
0x0, 	/* LITBADDR */
0x0, 	/* LITBEN */
0x0, 	/* MISC0 */
0x0, 	/* MISC1 */
0x0, 	/* MISC2 */
0x0, 	/* MISC3 */
0x0, 	/* InOCDMode */
0x0, 	/* INTENABLE */
0x0, 	/* DBREAKA0 */
0x0, 	/* DBREAKC0 */
0x0, 	/* DBREAKA1 */
0x0, 	/* DBREAKC1 */
0x0, 	/* IBREAKA0 */
0x0, 	/* IBREAKA1 */
0x0, 	/* IBREAKENABLE */
0x0, 	/* ICOUNTLEVEL */
0x0, 	/* DEBUGCAUSE */
0x0, 	/* DBNUM */
0x0, 	/* CCOMPARE0 */
}, 
/* END states */
NULL	/* last */
};	/* CSTUB_(proc) */

/* Table declarations */
static const unsigned int table__xt_bytelane_lsb[16] = {
0x0U, 0xffU, 0xff00U, 0xffffU, 0xff0000U, 0xff00ffU, 0xffff00U, 0xffffffU, 0xff000000U, 0xff0000ffU, 0xff00ff00U, 0xff00ffffU, 0xffff0000U, 0xffff00ffU, 0xffffff00U, 0xffffffffU};
static const unsigned int table__ai4c[16] = {
0xffffffffU, 0x1U, 0x2U, 0x3U, 0x4U, 0x5U, 0x6U, 0x7U, 0x8U, 0x9U, 0xaU, 0xbU, 0xcU, 0xdU, 0xeU, 0xfU};
static const unsigned int table__b4c[16] = {
0xffffffffU, 0x1U, 0x2U, 0x3U, 0x4U, 0x5U, 0x6U, 0x7U, 0x8U, 0xaU, 0xcU, 0x10U, 0x20U, 0x40U, 0x80U, 0x100U};
static const unsigned int table__b4cu[16] = {
0x8000U, 0x10000U, 0x2U, 0x3U, 0x4U, 0x5U, 0x6U, 0x7U, 0x8U, 0xaU, 0xcU, 0x10U, 0x20U, 0x40U, 0x80U, 0x100U};

/* Mask for StoreByteDisable */
static unsigned mask[16] = {
  0xffffffff, /* 0 */
  0xffffff00, /* 1 */
  0xffff00ff, /* 2 */
  0xffff0000, /* 3 */
  0xff00ffff, /* 4 */
  0xff00ff00, /* 5 */
  0xff0000ff, /* 6 */
  0xff000000, /* 7 */
  0x00ffffff, /* 8 */
  0x00ffff00, /* 9 */
  0x00ff00ff, /* a */
  0x00ff0000, /* b */
  0x0000ffff, /* c */
  0x0000ff00, /* d */
  0x000000ff, /* e */
  0x00000000, /* f */
};

/* Memory unit functions */

typedef union CSTUB_(mem) {
  unsigned char  d8;
  unsigned short d16;
  unsigned int   d32;
  unsigned int   d64[2];
  unsigned int   d128[4];
  unsigned int   d256[8];
  unsigned int   d512[16];
} CSTUB_(mem_t);

static INLINE void MemDataIn512(unsigned addr, unsigned opcode, unsigned *data) {
  CSTUB_(mem_t) *d = (CSTUB_(mem_t) *)addr;
  data[0] = d->d512[0];
  data[1] = d->d512[1];
  data[2] = d->d512[2];
  data[3] = d->d512[3];
  data[4] = d->d512[4];
  data[5] = d->d512[5];
  data[6] = d->d512[6];
  data[7] = d->d512[7];
  data[8] = d->d512[8];
  data[9] = d->d512[9];
  data[10] = d->d512[10];
  data[11] = d->d512[11];
  data[12] = d->d512[12];
  data[13] = d->d512[13];
  data[14] = d->d512[14];
  data[15] = d->d512[15];
}

static INLINE void MemDataIn256(unsigned addr, unsigned opcode, unsigned *data) {
  CSTUB_(mem_t) *d = (CSTUB_(mem_t) *)addr;
  data[0] = d->d256[0];
  data[1] = d->d256[1];
  data[2] = d->d256[2];
  data[3] = d->d256[3];
  data[4] = d->d256[4];
  data[5] = d->d256[5];
  data[6] = d->d256[6];
  data[7] = d->d256[7];
}

static INLINE void MemDataIn128(unsigned addr, unsigned opcode, unsigned *data) {
  CSTUB_(mem_t) *d = (CSTUB_(mem_t) *)addr;
  data[0] = d->d128[0];
  data[1] = d->d128[1];
  data[2] = d->d128[2];
  data[3] = d->d128[3];
}

static INLINE void MemDataIn64(unsigned addr, unsigned opcode, unsigned *data) {
  CSTUB_(mem_t) *d = (CSTUB_(mem_t) *)addr;
  data[0] = d->d64[0];
  data[1] = d->d64[1];
}

static INLINE unsigned MemDataIn32(unsigned addr, unsigned opcode) {
  CSTUB_(mem_t) *d = (CSTUB_(mem_t) *)addr;
  return d->d32;
}

static INLINE unsigned short MemDataIn16(unsigned addr, unsigned opcode) {
  CSTUB_(mem_t) *d = (CSTUB_(mem_t) *) addr;
  return d->d16;
}

static INLINE unsigned char MemDataIn8(unsigned addr, unsigned opcode) {
  CSTUB_(mem_t) *d = (CSTUB_(mem_t) *) addr;
  return d->d8;
}

static INLINE void MemDataOut512(unsigned addr, const unsigned *data, const unsigned *disables, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  if ((disables[0] == 0) && (disables[1] == 0)) {  
    dst->d512[0] = src[0];
    dst->d512[1] = src[1];
    dst->d512[2] = src[2];
    dst->d512[3] = src[3];
    dst->d512[4] = src[4];
    dst->d512[5] = src[5];
    dst->d512[6] = src[6];
    dst->d512[7] = src[7];
    dst->d512[8] = src[8];
    dst->d512[9] = src[9];
    dst->d512[10] = src[10];
    dst->d512[11] = src[11];
    dst->d512[12] = src[12];
    dst->d512[13] = src[13];
    dst->d512[14] = src[14];
    dst->d512[15] = src[15];
  } else  if ((disables[0] == 0xffffffff) && (disables[1] == 0xffffffff)) {
    /* no store */
  } else {
    dst->d512[0] = (dst->d512[0] & (~mask[(disables[0] >> 0) & 0xf])) | (src[0] & mask[(disables[0] >> 0) & 0xf]);
    dst->d512[1] = (dst->d512[1] & (~mask[(disables[0] >> 4) & 0xf])) | (src[1] & mask[(disables[0] >> 4) & 0xf]);
    dst->d512[2] = (dst->d512[2] & (~mask[(disables[0] >> 8) & 0xf])) | (src[2] & mask[(disables[0] >> 8) & 0xf]);
    dst->d512[3] = (dst->d512[3] & (~mask[(disables[0] >> 12) & 0xf])) | (src[3] & mask[(disables[0] >> 12) & 0xf]);
    dst->d512[4] = (dst->d512[4] & (~mask[(disables[0] >> 16) & 0xf])) | (src[4] & mask[(disables[0] >> 16) & 0xf]);
    dst->d512[5] = (dst->d512[5] & (~mask[(disables[0] >> 20) & 0xf])) | (src[5] & mask[(disables[0] >> 20) & 0xf]);
    dst->d512[6] = (dst->d512[6] & (~mask[(disables[0] >> 24) & 0xf])) | (src[6] & mask[(disables[0] >> 24) & 0xf]);
    dst->d512[7] = (dst->d512[7] & (~mask[(disables[0] >> 28) & 0xf])) | (src[7] & mask[(disables[0] >> 28) & 0xf]);
    dst->d512[8] = (dst->d512[8] & (~mask[(disables[1] >> 0) & 0xf])) | (src[8] & mask[(disables[1] >> 0) & 0xf]);
    dst->d512[9] = (dst->d512[9] & (~mask[(disables[1] >> 4) & 0xf])) | (src[9] & mask[(disables[1] >> 4) & 0xf]);
    dst->d512[10] = (dst->d512[10] & (~mask[(disables[1] >> 8) & 0xf])) | (src[10] & mask[(disables[1] >> 8) & 0xf]);
    dst->d512[11] = (dst->d512[11] & (~mask[(disables[1] >> 12) & 0xf])) | (src[11] & mask[(disables[1] >> 12) & 0xf]);
    dst->d512[12] = (dst->d512[12] & (~mask[(disables[1] >> 16) & 0xf])) | (src[12] & mask[(disables[1] >> 16) & 0xf]);
    dst->d512[13] = (dst->d512[13] & (~mask[(disables[1] >> 20) & 0xf])) | (src[13] & mask[(disables[1] >> 20) & 0xf]);
    dst->d512[14] = (dst->d512[14] & (~mask[(disables[1] >> 24) & 0xf])) | (src[14] & mask[(disables[1] >> 24) & 0xf]);
    dst->d512[15] = (dst->d512[15] & (~mask[(disables[1] >> 28) & 0xf])) | (src[15] & mask[(disables[1] >> 28) & 0xf]);
  }
}

static INLINE void MemDataOut256(unsigned addr, const unsigned *data, unsigned disables, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  if (disables == 0) {  
    dst->d256[0] = src[0];
    dst->d256[1] = src[1];
    dst->d256[2] = src[2];
    dst->d256[3] = src[3];
    dst->d256[4] = src[4];
    dst->d256[5] = src[5];
    dst->d256[6] = src[6];
    dst->d256[7] = src[7];
  } else  if (disables == 0xffffffff) {
    /* no store */
  } else {
    dst->d256[0] = (dst->d256[0] & (~mask[(disables >> 0) & 0xf])) | (src[0] & mask[(disables >> 0) & 0xf]);
    dst->d256[1] = (dst->d256[1] & (~mask[(disables >> 4) & 0xf])) | (src[1] & mask[(disables >> 4) & 0xf]);
    dst->d256[2] = (dst->d256[2] & (~mask[(disables >> 8) & 0xf])) | (src[2] & mask[(disables >> 8) & 0xf]);
    dst->d256[3] = (dst->d256[3] & (~mask[(disables >> 12) & 0xf])) | (src[3] & mask[(disables >> 12) & 0xf]);
    dst->d256[4] = (dst->d256[4] & (~mask[(disables >> 16) & 0xf])) | (src[4] & mask[(disables >> 16) & 0xf]);
    dst->d256[5] = (dst->d256[5] & (~mask[(disables >> 20) & 0xf])) | (src[5] & mask[(disables >> 20) & 0xf]);
    dst->d256[6] = (dst->d256[6] & (~mask[(disables >> 24) & 0xf])) | (src[6] & mask[(disables >> 24) & 0xf]);
    dst->d256[7] = (dst->d256[7] & (~mask[(disables >> 28) & 0xf])) | (src[7] & mask[(disables >> 28) & 0xf]);
  }
}

static INLINE void MemDataOut128(unsigned addr, const unsigned *data, unsigned disables, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  if (disables == 0) {  
    dst->d128[0] = src[0];
    dst->d128[1] = src[1];
    dst->d128[2] = src[2];
    dst->d128[3] = src[3];
  } else  if (disables == 0xffff) {
    /* no store */
  } else {
    dst->d128[0] = (dst->d128[0] & (~mask[disables & 0xf]))| (src[0] & mask[disables & 0xf]);
    dst->d128[1] = (dst->d128[1] & (~mask[(disables >> 4) & 0xf])) | (src[1] & mask[(disables >> 4) & 0xf]);
    dst->d128[2] = (dst->d128[2] & (~mask[(disables >> 8) & 0xf])) | (src[2] & mask[(disables >> 8) & 0xf]);
    dst->d128[3] = (dst->d128[3] & (~mask[(disables >> 12) & 0xf])) | (src[3] & mask[(disables >> 12) & 0xf]);
  }
}

static INLINE void MemDataOut64(unsigned addr, const unsigned *data, unsigned disables, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  if (disables == 0) {
    dst->d64[0] = src[0];
    dst->d64[1] = src[1];
  } else if (disables == 0xff) {
    /* no store */
  } else {
    dst->d64[0] = (dst->d64[0] & (~mask[disables & 0xf])) | (src[0] & mask[disables & 0xf]);
    dst->d64[1] = (dst->d64[1] & (~mask[(disables >> 4) & 0xf])) | (src[1] & mask[(disables >> 4) & 0xf]);
  }
}

static INLINE void MemDataOut32(unsigned addr, unsigned data, unsigned disables, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  if (disables == 0) {
    dst->d32 = data;
  } else if (disables == 0xf) {
    /* no store */
  } else {
    dst->d32 = (dst->d32 & (~mask[disables])) | (data & mask[disables]);
  }
}

static INLINE void MemDataOut16(unsigned addr, unsigned data, unsigned disables, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  unsigned short src = (unsigned short) data;
  if (disables == 0) 
    dst->d16 = src;
  else if (disables == 0x3) {
    /* no store */
  } else {
    dst->d16 = (dst->d16 & (~mask[disables])) | (src & mask[disables]);
  }
}

static INLINE void MemDataOut8(unsigned addr, unsigned data, unsigned disables, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  if (disables == 0)
    dst->d8 = data;
}

static INLINE void MemDataOut512WODisable(unsigned addr, const unsigned *data, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  dst->d512[0] = src[0];
  dst->d512[1] = src[1];
  dst->d512[2] = src[2];
  dst->d512[3] = src[3];
  dst->d512[4] = src[4];
  dst->d512[5] = src[5];
  dst->d512[6] = src[6];
  dst->d512[7] = src[7];
  dst->d512[8] = src[8];
  dst->d512[9] = src[9];
  dst->d512[10] = src[10];
  dst->d512[11] = src[11];
  dst->d512[12] = src[12];
  dst->d512[13] = src[13];
  dst->d512[14] = src[14];
  dst->d512[15] = src[15];
}

static INLINE void MemDataOut256WODisable(unsigned addr, const unsigned *data, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  dst->d256[0] = src[0];
  dst->d256[1] = src[1];
  dst->d256[2] = src[2];
  dst->d256[3] = src[3];
  dst->d256[4] = src[4];
  dst->d256[5] = src[5];
  dst->d256[6] = src[6];
  dst->d256[7] = src[7];
}

static INLINE void MemDataOut128WODisable(unsigned addr, const unsigned *data, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  dst->d128[0] = src[0];
  dst->d128[1] = src[1];
  dst->d128[2] = src[2];
  dst->d128[3] = src[3];
}

static INLINE void MemDataOut64WODisable(unsigned addr, const unsigned *data, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  const unsigned *src = data;
  dst->d64[0] = src[0];
  dst->d64[1] = src[1];
}

static INLINE void MemDataOut32WODisable(unsigned addr, unsigned data, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  dst->d32 = data;
}

static INLINE void MemDataOut16WODisable(unsigned addr, unsigned data, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  dst->d16 = data;
}

static INLINE void MemDataOut8WODisable(unsigned addr, unsigned data, unsigned opcode) {
  CSTUB_(mem_t) *dst = (CSTUB_(mem_t) *) addr;
  dst->d8 = data;
}

static INLINE void cstub_vaddr_not_aligned(unsigned vaddr) {
  fprintf(stderr, "Error: The config takes exception when the address of an load/store is unaligned. The address 0x%x is not properly aligned.\n", vaddr);
  exit(1);
}

/* C implementation of the instructions */
static INLINE void Function_TIE_xt_density_NOP_N() {
return;
}

static INLINE void Function_TIE_xt_core_NOP() {
return;
}

static INLINE void Function_TIE_xt_density_L32I_N(unsigned int *t__out_param, unsigned int p, unsigned int i) {
unsigned int t__out = 0;

unsigned int tn1__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
tn4__o0 = ((p) + (i));
tn5__o0 = (((tn4__o0 & 0x3U)) != 0);
tn1__o0 = (!(tn5__o0));
if (tn1__o0)
{
t__out = MemDataIn32(((tn4__o0 & 0xfffffffcU)), (0x1eU));
}
if (tn5__o0)
{
cstub_vaddr_not_aligned(tn4__o0);
}
*t__out_param = t__out;
return;
}

static INLINE void Function_TIE_xt_density_S32I_N(unsigned int t, unsigned int p, unsigned int i) {
unsigned int tn0__o0;
unsigned int tn3__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
tn4__o0 = ((p) + (i));
tn3__o0 = (((tn4__o0 & 0x3U)) != 0);
tn6__o0 = (!(tn3__o0));
tn0__o0 = ((tn6__o0 & (/* X 0x1U */0U)));
tn5__o0 = ((tn3__o0 | tn0__o0));
if (tn5__o0)
{
cstub_vaddr_not_aligned(tn4__o0);
}
if (tn6__o0)
{
MemDataOut32WODisable(((tn4__o0 & 0xfffffffcU)), (t), (0x23U));
}
return;
}

static INLINE void Function_TIE_xt_density_ADD_N(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((s) + (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_density_ADDI_N(unsigned int *r__out_param, unsigned int s, unsigned int i) {
unsigned int r__out = 0;

r__out = ((s) + (i));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_density_MOV_N(unsigned int *t__out_param, unsigned int s) {
unsigned int t__out = 0;

t__out = ((s));
*t__out_param = t__out;
return;
}

static INLINE void Function_TIE_xt_density_MOVI_N(unsigned int *s__out_param, unsigned int i) {
unsigned int s__out = 0;

s__out = ((i));
*s__out_param = s__out;
return;
}

static INLINE void Function_TIE_xt_core_uint32_loadi(unsigned int *c__out_param, unsigned int p, unsigned int o) {
unsigned int c__out = 0;

unsigned int tn1__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
tn4__o0 = ((p) + (o));
tn5__o0 = (((tn4__o0 & 0x3U)) != 0);
tn1__o0 = (!(tn5__o0));
if (tn1__o0)
{
c__out = MemDataIn32(((tn4__o0 & 0xfffffffcU)), (0x51U));
}
if (tn5__o0)
{
cstub_vaddr_not_aligned(tn4__o0);
}
*c__out_param = c__out;
return;
}

static INLINE void Function_TIE_xt_core_uint32_storei(unsigned int c, unsigned int p, unsigned int o) {
unsigned int tn0__o0;
unsigned int tn3__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
tn4__o0 = ((p) + (o));
tn3__o0 = (((tn4__o0 & 0x3U)) != 0);
tn6__o0 = (!(tn3__o0));
tn0__o0 = ((tn6__o0 & (/* X 0x1U */0U)));
tn5__o0 = ((tn3__o0 | tn0__o0));
if (tn5__o0)
{
cstub_vaddr_not_aligned(tn4__o0);
}
if (tn6__o0)
{
MemDataOut32WODisable(((tn4__o0 & 0xfffffffcU)), (c), (0x62U));
}
return;
}

static INLINE void Function_TIE_xt_core_uint32_move(unsigned int *a__out_param, unsigned int b) {
unsigned int a__out = 0;

a__out = ((b));
*a__out_param = a__out;
return;
}

static INLINE void Function_TIE_xt_core_ADDI(unsigned int *r__out_param, unsigned int s, unsigned int i) {
unsigned int r__out = 0;

r__out = ((s) + (i));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_OR(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = (((s) | (t)));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_L32I(unsigned int *r__out_param, unsigned int p, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn1__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
tn4__o0 = ((p) + (i));
tn5__o0 = (((tn4__o0 & 0x3U)) != 0);
tn1__o0 = (!(tn5__o0));
if (tn1__o0)
{
r__out = MemDataIn32(((tn4__o0 & 0xfffffffcU)), (0x51U));
}
if (tn5__o0)
{
cstub_vaddr_not_aligned(tn4__o0);
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_S32I(unsigned int r, unsigned int p, unsigned int i) {
unsigned int tn0__o0;
unsigned int tn3__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
tn4__o0 = ((p) + (i));
tn3__o0 = (((tn4__o0 & 0x3U)) != 0);
tn6__o0 = (!(tn3__o0));
tn0__o0 = ((tn6__o0 & (/* X 0x1U */0U)));
tn5__o0 = ((tn3__o0 | tn0__o0));
if (tn5__o0)
{
cstub_vaddr_not_aligned(tn4__o0);
}
if (tn6__o0)
{
MemDataOut32WODisable(((tn4__o0 & 0xfffffffcU)), (r), (0x62U));
}
return;
}

static INLINE void Function_TIE_xt_core_L8UI(unsigned int *r__out_param, unsigned int p, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn1__o0;
tn1__o0 = ((p) + (i));
{
tn0__o0 = MemDataIn8(tn1__o0, (0x53U));
}
r__out = ((tn0__o0));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_S8I(unsigned int r, unsigned int p, unsigned int i) {
unsigned int tn3__o0;
tn3__o0 = ((p) + (i));
if ((/* X 0x1U */0U))
{
cstub_vaddr_not_aligned(tn3__o0);
}
{
MemDataOut8WODisable(tn3__o0, (((unsigned int)(unsigned char)((r)))), (0x63U));
}
return;
}

static INLINE void Function_TIE_xt_core_L16UI(unsigned int *r__out_param, unsigned int p, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn2__o0 = 0;
unsigned int tn4__o0;
unsigned int tn5__o0;
tn4__o0 = ((p) + (i));
tn5__o0 = (!(((tn4__o0 & 0x1U))));
if (tn5__o0)
{
tn2__o0 = MemDataIn16(((tn4__o0 & 0xfffffffeU)), (0x4fU));
}
r__out = ((tn2__o0));
if (((tn4__o0 & 0x1U)))
{
cstub_vaddr_not_aligned(tn4__o0);
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_L16SI(unsigned int *r__out_param, unsigned int p, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn1__o0;
unsigned int tn4__o0;
unsigned int tn5__o0 = 0;
tn4__o0 = ((p) + (i));
tn1__o0 = (!(((tn4__o0 & 0x1U))));
if (tn1__o0)
{
tn5__o0 = MemDataIn16(((tn4__o0 & 0xfffffffeU)), (0x50U));
}
r__out = ((((unsigned int)(int)(short)(tn5__o0))));
if (((tn4__o0 & 0x1U)))
{
cstub_vaddr_not_aligned(tn4__o0);
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_S16I(unsigned int r, unsigned int p, unsigned int i) {
unsigned int tn0__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
unsigned int tn7__o0;
tn5__o0 = ((p) + (i));
tn7__o0 = (!(((tn5__o0 & 0x1U))));
if (tn7__o0)
{
MemDataOut16WODisable(((tn5__o0 & 0xfffffffeU)), (((unsigned int)(unsigned short)((r)))), (0x61U));
}
tn0__o0 = ((tn7__o0 & (/* X 0x1U */0U)));
tn6__o0 = ((((tn5__o0 & 0x1U)) | tn0__o0));
if (tn6__o0)
{
cstub_vaddr_not_aligned(tn5__o0);
}
return;
}

static INLINE void Function_TIE_xt_core_ADDMI(unsigned int *r__out_param, unsigned int s, unsigned int i) {
unsigned int r__out = 0;

r__out = ((s) + (i));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_ADD(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((s) + (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_ADDX2(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((((s) << 1)) + (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_ADDX4(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((((s) << 2)) + (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_ADDX8(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((((s) << 3)) + (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SUB(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((s) - (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SUBX2(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((((s) << 1)) - (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SUBX4(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((((s) << 2)) - (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SUBX8(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = ((((s) << 3)) - (t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_AND(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = (((s) & (t)));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_XOR(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

r__out = (((s) ^ (t)));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_EXTUI(unsigned int *r__out_param, unsigned int t, unsigned int i, unsigned int o) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn1__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
{
unsigned shift__ = (((i) & 0x1fU));
tn6__o0 = (((t) >> shift__));
}
tn4__o0 = ((((o) & 0xfU)) - (0x1U)) & 0x0fU;
tn1__o0 = ((tn4__o0) + (0x1U)) & 0x1fU;
{
unsigned shift__ = tn1__o0;
tn5__o0 = (((0x1U) << shift__));
}
tn0__o0 = (tn5__o0 - (0x1U));
r__out = ((tn6__o0 & tn0__o0));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_MOVI(unsigned int *t__out_param, unsigned int i) {
unsigned int t__out = 0;

t__out = ((i));
*t__out_param = t__out;
return;
}

static INLINE void Function_TIE_xt_core_MOVEQZ(unsigned int *r__out_param, unsigned int r, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn1__o0;
tn1__o0 = ((t) != 0);
r__out = (((tn1__o0) != 0)) ? ((r)) : ((s));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_MOVNEZ(unsigned int *r__out_param, unsigned int r, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn1__o0;
tn1__o0 = (((t) == 0));
r__out = (((tn1__o0) != 0)) ? ((r)) : ((s));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_MOVLTZ(unsigned int *r__out_param, unsigned int r, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn1__o0;
tn1__o0 = (!((((t) >> 31))));
r__out = (((tn1__o0) != 0)) ? ((r)) : ((s));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_MOVGEZ(unsigned int *r__out_param, unsigned int r, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn1__o0;
unsigned int tn2__o0;
unsigned int tn5__o0;
tn2__o0 = (!((((t) >> 31))));
tn5__o0 = ((((t)) == 0));
tn1__o0 = ((tn2__o0 | tn5__o0));
tn0__o0 = (!(tn1__o0));
r__out = (((tn0__o0) != 0)) ? ((r)) : ((s));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_NEG(unsigned int *r__out_param, unsigned int t) {
unsigned int r__out = 0;

r__out = (-((t)));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_ABS(unsigned int *r__out_param, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn2__o0;
unsigned int tn3__o0;
tn3__o0 = (!((((t) >> 31))));
if (((tn3__o0) != 0))
{
r__out = ((t));
}
else
{
tn2__o0 = (-(((t))));
r__out = tn2__o0;
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SSR(unsigned int s) {
CSTUB_PROC.states.SAR = (((s) & 0x1fU));
return;
}

static INLINE void Function_TIE_xt_core_SSL(unsigned int s) {
unsigned int tn2__o0;
tn2__o0 = ((0x20U) - (((s) & 0x1fU))) & 0x3fU;
CSTUB_PROC.states.SAR = tn2__o0;
return;
}

static INLINE void Function_TIE_xt_core_SSA8L(unsigned int s) {
CSTUB_PROC.states.SAR = ((((s) << 3) & 0x18U));
return;
}

static INLINE void Function_TIE_xt_core_SSA8B(unsigned int s) {
unsigned int tn2__o0;
tn2__o0 = ((0x20U) - ((((s) << 3) & 0x18U))) & 0x3fU;
CSTUB_PROC.states.SAR = tn2__o0;
return;
}

static INLINE void Function_TIE_xt_core_SSAI(unsigned int i) {
CSTUB_PROC.states.SAR = (((i) & 0x1fU));
return;
}

static INLINE void Function_TIE_xt_core_SLL(unsigned int *r__out_param, unsigned int s) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn3__o0[2];
unsigned int tn3__i0[2];
tn0__o0 = CSTUB_PROC.states.SAR;
tn3__i0[0] = (0x0U);
tn3__i0[1] = ((s));
{
unsigned bit__shift = tn0__o0& 0x1fU;
unsigned word__shift = tn0__o0>> 5;
if (bit__shift == 0)
{
tn3__o0[0] = (tn3__i0[word__shift]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = (word__shift > 0) ? 0 : tn3__i0[1];
tmp__ = tn3__i0[word__shift];
tn3__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
r__out = ((tn3__o0[0]));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SRC(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn4__o0[2];
unsigned int tn4__i0[2];
tn0__o0 = CSTUB_PROC.states.SAR;
tn4__i0[0] = ((t));
tn4__i0[1] = ((s));
{
unsigned bit__shift = tn0__o0& 0x1fU;
unsigned word__shift = tn0__o0>> 5;
if (bit__shift == 0)
{
tn4__o0[0] = (tn4__i0[word__shift]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = (word__shift > 0) ? 0 : tn4__i0[1];
tmp__ = tn4__i0[word__shift];
tn4__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
r__out = ((tn4__o0[0]));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SRL(unsigned int *r__out_param, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn0__o0;
tn0__o0 = CSTUB_PROC.states.SAR;
{
unsigned shift__ = tn0__o0;
r__out = ((shift__ >= 32) ? 0 : ((t) >> shift__));
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SRA(unsigned int *r__out_param, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn3__o0[2];
unsigned int tn3__i0[2];
tn0__o0 = CSTUB_PROC.states.SAR;
tn3__i0[0] = ((t));
tn3__i0[1] = (((unsigned int)(((int)((t))) >> 31)));
{
unsigned bit__shift = tn0__o0& 0x1fU;
unsigned word__shift = tn0__o0>> 5;
if (bit__shift == 0)
{
tn3__o0[0] = (tn3__i0[word__shift]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = (word__shift > 0) ? 0 : tn3__i0[1];
tmp__ = tn3__i0[word__shift];
tn3__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
r__out = ((tn3__o0[0]));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SLLI(unsigned int *r__out_param, unsigned int s, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn1__o0;
tn1__o0 = (-((((i) & 0x1fU)))) & 0x1fU;
tn0__o0 = ((0x20U) - (tn1__o0)) & 0x3fU;
{
unsigned shift__ = tn0__o0;
r__out = ((shift__ >= 32) ? 0 : ((s) << shift__));
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SRAI(unsigned int *r__out_param, unsigned int t, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn0__o0[2];
unsigned int tn0__i0[2];
tn0__i0[0] = ((t));
tn0__i0[1] = ((((unsigned int)(((int)((t))) >> 31)) & 0x7fffffffU));
{
unsigned bit__shift = (((i) & 0x1fU));
if (bit__shift == 0)
{
tn0__o0[0] = (tn0__i0[0]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = tn0__i0[1];
tmp__ = tn0__i0[0];
tn0__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
r__out = ((tn0__o0[0]));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SRLI(unsigned int *r__out_param, unsigned int t, unsigned int i) {
unsigned int r__out = 0;

{
unsigned shift__ = (((i) & 0xfU));
r__out = (((t) >> shift__));
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_core_SSAI_SRC(unsigned int *dst__out_param, unsigned int src1, unsigned int src2, unsigned int amount) {
unsigned int dst__out = 0;

unsigned int tn0__o0;
unsigned int tn6__o0[2];
unsigned int tn6__i0[2];
CSTUB_PROC.states.SAR = (((amount) & 0x1fU));
tn0__o0 = CSTUB_PROC.states.SAR;
tn6__i0[0] = ((src2));
tn6__i0[1] = ((src1));
{
unsigned bit__shift = tn0__o0& 0x1fU;
unsigned word__shift = tn0__o0>> 5;
if (bit__shift == 0)
{
tn6__o0[0] = (tn6__i0[word__shift]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = (word__shift > 0) ? 0 : tn6__i0[1];
tmp__ = tn6__i0[word__shift];
tn6__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
dst__out = ((tn6__o0[0]));
*dst__out_param = dst__out;
return;
}

static INLINE void Function_TIE_xt_core_SSR_SRC(unsigned int *dst__out_param, unsigned int src1, unsigned int src2, unsigned int amount) {
unsigned int dst__out = 0;

unsigned int tn0__o0;
unsigned int tn6__o0[2];
unsigned int tn6__i0[2];
CSTUB_PROC.states.SAR = (((amount) & 0x1fU));
tn0__o0 = CSTUB_PROC.states.SAR;
tn6__i0[0] = ((src2));
tn6__i0[1] = ((src1));
{
unsigned bit__shift = tn0__o0& 0x1fU;
unsigned word__shift = tn0__o0>> 5;
if (bit__shift == 0)
{
tn6__o0[0] = (tn6__i0[word__shift]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = (word__shift > 0) ? 0 : tn6__i0[1];
tmp__ = tn6__i0[word__shift];
tn6__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
dst__out = ((tn6__o0[0]));
*dst__out_param = dst__out;
return;
}

static INLINE void Function_TIE_xt_core_SSR_SRA(unsigned int *dst__out_param, unsigned int src, unsigned int amount) {
unsigned int dst__out = 0;

unsigned int tn0__o0;
unsigned int tn5__o0[2];
unsigned int tn5__i0[2];
CSTUB_PROC.states.SAR = (((amount) & 0x1fU));
tn0__o0 = CSTUB_PROC.states.SAR;
tn5__i0[0] = ((src));
tn5__i0[1] = (((unsigned int)(((int)((src))) >> 31)));
{
unsigned bit__shift = tn0__o0& 0x1fU;
unsigned word__shift = tn0__o0>> 5;
if (bit__shift == 0)
{
tn5__o0[0] = (tn5__i0[word__shift]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = (word__shift > 0) ? 0 : tn5__i0[1];
tmp__ = tn5__i0[word__shift];
tn5__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
dst__out = ((tn5__o0[0]));
*dst__out_param = dst__out;
return;
}

static INLINE void Function_TIE_xt_core_SSR_SRL(unsigned int *dst__out_param, unsigned int src, unsigned int amount) {
unsigned int dst__out = 0;

unsigned int tn0__o0;
CSTUB_PROC.states.SAR = (((amount) & 0x1fU));
tn0__o0 = CSTUB_PROC.states.SAR;
{
unsigned shift__ = tn0__o0;
dst__out = ((shift__ >= 32) ? 0 : ((src) >> shift__));
}
*dst__out_param = dst__out;
return;
}

static INLINE void Function_TIE_xt_core_SSL_SLL(unsigned int *dst__out_param, unsigned int src, unsigned int amount) {
unsigned int dst__out = 0;

unsigned int tn0__o0;
unsigned int tn2__o0;
unsigned int tn5__o0[2];
unsigned int tn5__i0[2];
tn2__o0 = ((0x20U) - (((amount) & 0x1fU))) & 0x3fU;
CSTUB_PROC.states.SAR = tn2__o0;
tn0__o0 = CSTUB_PROC.states.SAR;
tn5__i0[0] = (0x0U);
tn5__i0[1] = ((src));
{
unsigned bit__shift = tn0__o0& 0x1fU;
unsigned word__shift = tn0__o0>> 5;
if (bit__shift == 0)
{
tn5__o0[0] = (tn5__i0[word__shift]);
}
else
{
unsigned int rshift__ = 32 - bit__shift;
unsigned int tmp__;
unsigned int prev__;
prev__ = (word__shift > 0) ? 0 : tn5__i0[1];
tmp__ = tn5__i0[word__shift];
tn5__o0[0] = ((tmp__ >> bit__shift) | (prev__ << rshift__));
}
}
dst__out = ((tn5__o0[0]));
*dst__out_param = dst__out;
return;
}

static INLINE void Function_TIE_xt_mul_MUL16S(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

{
unsigned int mul0__;
unsigned int mul1__;
unsigned int out__;
mul0__ = (((unsigned int)(unsigned short)((s))));
mul1__ = (((unsigned int)(unsigned short)((t))));
mul0__ = ((unsigned int)(int)(signed short)mul0__);
mul1__ = ((unsigned int)(int)(signed short)mul1__);
out__ = ((unsigned int)(((int)mul0__) * ((int)mul1__)));
r__out = out__;
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_mul_MUL16U(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

{
unsigned int mul0__;
unsigned int mul1__;
unsigned int out__;
mul0__ = (((unsigned int)(unsigned short)((s))));
mul1__ = (((unsigned int)(unsigned short)((t))));
out__ = (mul0__ * mul1__);
r__out = out__;
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_mul_MULL(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

{
unsigned int mul0__;
unsigned int mul1__;
unsigned int out__;
mul0__ = (s);
mul1__ = (t);
out__ = (mul0__ * mul1__);
r__out = out__;
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_mul_MULUH(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn0__o0[2];
{
unsigned int mul0__;
unsigned int mul1__;
unsigned long long mult__;
mul0__ = (s);
mul1__ = (t);
mult__ = (((unsigned long long)mul0__ * (unsigned long long)mul1__));
tn0__o0[0] = (unsigned int)mult__;
tn0__o0[1] = (unsigned int)(mult__ >> 32);
}
r__out = ((tn0__o0[1]));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_mul_MULSH(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn0__o0[2];
{
unsigned int mul0__;
unsigned int mul1__;
unsigned long long mult__;
mul0__ = (s);
mul1__ = (t);
mult__ = ((unsigned long long)(((signed long long)(int)mul0__) * ((signed long long)(int)mul1__)));
tn0__o0[0] = (unsigned int)mult__;
tn0__o0[1] = (unsigned int)(mult__ >> 32);
}
r__out = ((tn0__o0[1]));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_misc_CLAMPS(unsigned int *r__out_param, unsigned int s, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn0__o0;
unsigned int tn3__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
unsigned int tn8__o0;
unsigned int tn9__o0;
unsigned int tn10__o0;
unsigned int tn11__o0;
{
unsigned shift__ = (i);
tn9__o0 = ((shift__ >= 32) ? 0 : ((0x1U) << shift__));
}
tn8__o0 = (tn9__o0 - (0x1U));
tn0__o0 = (~(tn8__o0));
tn3__o0 = ((((s)) & tn0__o0));
tn6__o0 = ((tn3__o0 == 0));
tn10__o0 = ((((s)) | tn8__o0));
tn11__o0 = (tn10__o0 == 0xffffffffU);
tn4__o0 = ((tn6__o0 | tn11__o0));
if (((tn4__o0) != 0))
{
r__out = ((s));
}
else
{
tn5__o0 = ((((int)((s))) < 0)) ? (tn0__o0) : (tn8__o0);
r__out = tn5__o0;
}
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_misc_MIN(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn1__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
tn4__o0 = (!((((t) >> 31))));
tn5__o0 = (!((((s) >> 31))));
tn1__o0 = (( ((((t) & 0x7fffffffU) | (tn4__o0 << 31))) < ((((s) & 0x7fffffffU) | (tn5__o0 << 31))) ));
r__out = (((tn1__o0) != 0)) ? (((t))) : (((s)));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_misc_MAX(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn1__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
tn4__o0 = (!((((t) >> 31))));
tn5__o0 = (!((((s) >> 31))));
tn1__o0 = (( ((((t) & 0x7fffffffU) | (tn4__o0 << 31))) < ((((s) & 0x7fffffffU) | (tn5__o0 << 31))) ));
r__out = (((tn1__o0) != 0)) ? (((s))) : (((t)));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_misc_MINU(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn1__o0;
unsigned int tn1__i0;
unsigned int tn1__i1;
unsigned int tn3__i1;
unsigned int tn3__i2;
tn1__i0 = (t);
tn1__i1 = (s);
tn1__o0 = (( ((t)) < ((s)) ));
tn3__i1 = (t);
tn3__i2 = (s);
r__out = (((tn1__o0) != 0)) ? ((t)) : ((s));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_misc_MAXU(unsigned int *r__out_param, unsigned int s, unsigned int t) {
unsigned int r__out = 0;

unsigned int tn1__o0;
unsigned int tn1__i0;
unsigned int tn1__i1;
unsigned int tn3__i1;
unsigned int tn3__i2;
tn1__i0 = (t);
tn1__i1 = (s);
tn1__o0 = (( ((t)) < ((s)) ));
tn3__i1 = (s);
tn3__i2 = (t);
r__out = (((tn1__o0) != 0)) ? ((s)) : ((t));
*r__out_param = r__out;
return;
}

static INLINE void Function_TIE_xt_misc_NSA(unsigned int *t__out_param, unsigned int s) {
unsigned int t__out = 0;

unsigned int tn3__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
unsigned int tn7__o0;
unsigned int tn8__o0;
unsigned int tn9__o0;
unsigned int tn10__o0;
unsigned int tn12__o0;
unsigned int tn14__o0;
unsigned int tn16__o0;
unsigned int tn20__o0;
tn4__o0 = (( ((((unsigned int)(((int)((s))) >> 31)) & 0x7fffffffU)) == (((s) & 0x7fffffffU)) ));
if (((tn4__o0) != 0))
{
tn9__o0 = (0x1fU);
}
else
{
tn6__o0 = (( ((((s) >> 16) & 0x7fffU)) == ((((unsigned int)(((int)((s))) >> 31)) & 0x7fffU)) ));
tn5__o0 = (((tn6__o0) != 0)) ? (((((s) >> 1) & 0x7fffU))) : ((((s) >> 17)));
tn10__o0 = (( ((tn5__o0 >> 7)) == ((((unsigned char)(((int)((s))) >> 31)) & 0xffU)) ));
tn20__o0 = (((tn10__o0) != 0)) ? (((tn5__o0 & 0x7fU))) : (((tn5__o0 >> 8)));
tn16__o0 = (( ((tn20__o0 >> 3)) == ((((unsigned int)(((int)((s))) >> 31)) & 0xfU)) ));
tn14__o0 = (((tn16__o0) != 0)) ? (((tn20__o0 & 0x7U))) : (((tn20__o0 >> 4)));
tn12__o0 = (( ((tn14__o0 >> 1)) == ((((unsigned int)(((int)((s))) >> 31)) & 0x3U)) ));
tn3__o0 = (((tn12__o0) != 0)) ? (((tn14__o0 & 0x1U))) : (((tn14__o0 >> 2)));
tn8__o0 = (( tn3__o0 == (((s) >> 31)) ));
tn7__o0 = ((tn8__o0 | (tn12__o0 << 1) | (tn16__o0 << 2) | (tn10__o0 << 3) | (tn6__o0 << 4)) - (0x1U)) & 0x1fU;
tn9__o0 = tn7__o0;
}
t__out = ((tn9__o0));
*t__out_param = t__out;
return;
}

static INLINE void Function_TIE_xt_misc_NSAU(unsigned int *t__out_param, unsigned int s) {
unsigned int t__out = 0;

unsigned int tn0__o0;
unsigned int tn3__o0;
unsigned int tn4__o0;
unsigned int tn5__o0;
unsigned int tn6__o0;
unsigned int tn7__o0;
unsigned int tn8__o0;
unsigned int tn9__o0;
unsigned int tn11__o0;
unsigned int tn13__o0;
unsigned int tn19__o0;
tn7__o0 = (((((s) >> 16)) == 0));
tn0__o0 = (((tn7__o0) != 0)) ? (((((s) >> 1) & 0x7fffU))) : ((((s) >> 17)));
tn3__o0 = ((((tn0__o0 >> 7)) == 0));
tn19__o0 = (((tn3__o0) != 0)) ? (((tn0__o0 & 0x7fU))) : (((tn0__o0 >> 8)));
tn4__o0 = ((((tn19__o0 >> 3)) == 0));
tn13__o0 = (((tn4__o0) != 0)) ? (((tn19__o0 & 0x7U))) : (((tn19__o0 >> 4)));
tn6__o0 = ((((tn13__o0 >> 1)) == 0));
tn5__o0 = ((((s)) == 0));
if (((tn5__o0) != 0))
{
tn11__o0 = (0x0U);
}
else
{
tn8__o0 = (((tn6__o0) != 0)) ? (((tn13__o0 & 0x1U))) : (((tn13__o0 >> 2)));
tn9__o0 = (!(tn8__o0));
tn11__o0 = (tn9__o0 | (tn6__o0 << 1) | (tn4__o0 << 2) | (tn3__o0 << 3) | (tn7__o0 << 4));
}
t__out = ((tn11__o0 | (tn5__o0 << 5)));
*t__out_param = t__out;
return;
}

static INLINE void Function_TIE_xt_misc_SEXT(unsigned int *r__out_param, unsigned int s, unsigned int i) {
unsigned int r__out = 0;

unsigned int tn1__o0;
unsigned int tn1__i1;
unsigned int tn3__o0;
unsigned int tn5__o0;
unsigned int tn5__i1;
unsigned int tn7__o0;
unsigned int tn7__i0;
unsigned int tn7__i1;
unsigned int tn8__o0;
unsigned int tn9__o0;
unsigned int tn9__i1;
tn9__i1 = (i);
{
unsigned shift__ = (i);
tn9__o0 = ((shift__ >= 32) ? 0 : ((0x1U) << shift__));
}
tn8__o0 = (tn9__o0 - (0x1U));
tn7__i0 = (s);
tn7__i1 = (i);
{
unsigned idx = ((i)) & 0x1fU;
tn7__o0 = ((s) >> idx) & 0x1;
}
if (((tn7__o0) != 0))
{
tn3__o0 = (~(tn8__o0));
tn1__i1 = (s);
tn1__o0 = ((tn3__o0 | (s)));
r__out = tn1__o0;
}
else
{
tn5__i1 = (s);
tn5__o0 = ((tn8__o0 & (s)));
r__out = tn5__o0;
}
*r__out_param = r__out;
return;
}


/* Proto functions that call instructions */
void CSTUB_(_TIE_xt_density_NOP_N)(void) {
Function_TIE_xt_density_NOP_N();
return;
}

void CSTUB_(_TIE_xt_core_NOP)(void) {
Function_TIE_xt_core_NOP();
return;
}

int CSTUB_(_TIE_xt_density_L32I_N)(const int * p, immediate i) {
  unsigned int t;
Function_TIE_xt_density_L32I_N((unsigned int *)&t, (unsigned int)p, i);
return (int) t;
}

void CSTUB_(_TIE_xt_density_S32I_N)(const int t, const int * p, immediate i) {
Function_TIE_xt_density_S32I_N((unsigned int)t, (unsigned int)p, i);
return;
}

int CSTUB_(_TIE_xt_density_ADD_N)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_density_ADD_N((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_density_ADDI_N)(const int s, immediate i) {
  unsigned int r;
Function_TIE_xt_density_ADDI_N((unsigned int *)&r, (unsigned int)s, i);
return (int) r;
}

int CSTUB_(_TIE_xt_density_MOV_N)(const int s) {
  unsigned int t;
Function_TIE_xt_density_MOV_N((unsigned int *)&t, (unsigned int)s);
return (int) t;
}

int CSTUB_(_TIE_xt_density_MOVI_N)(immediate i) {
  unsigned int s;
Function_TIE_xt_density_MOVI_N((unsigned int *)&s, i);
return (int) s;
}

unsigned CSTUB_(_TIE_xt_core_uint32_loadi)(const unsigned * p, immediate o) {
  unsigned int c;
Function_TIE_xt_core_uint32_loadi((unsigned int *)&c, (unsigned int)p, o);
return (unsigned) c;
}

void CSTUB_(_TIE_xt_core_uint32_storei)(const unsigned c, const unsigned * p, immediate o) {
Function_TIE_xt_core_uint32_storei(c, (unsigned int)p, o);
return;
}

unsigned CSTUB_(_TIE_xt_core_uint32_move)(const unsigned b) {
  unsigned int a;
Function_TIE_xt_core_uint32_move((unsigned int *)&a, b);
return (unsigned) a;
}

int CSTUB_(_TIE_xt_core_ADDI)(const int s, immediate i) {
  unsigned int r;
Function_TIE_xt_core_ADDI((unsigned int *)&r, (unsigned int)s, i);
return (int) r;
}

int CSTUB_(_TIE_xt_core_OR)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_OR((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_L32I)(const int * p, immediate i) {
  unsigned int r;
Function_TIE_xt_core_L32I((unsigned int *)&r, (unsigned int)p, i);
return (int) r;
}

void CSTUB_(_TIE_xt_core_S32I)(const int r, const int * p, immediate i) {
Function_TIE_xt_core_S32I((unsigned int)r, (unsigned int)p, i);
return;
}

unsigned char CSTUB_(_TIE_xt_core_L8UI)(const unsigned char * p, immediate i) {
  unsigned int r;
Function_TIE_xt_core_L8UI((unsigned int *)&r, (unsigned int)p, i);
return (unsigned char) r;
}

void CSTUB_(_TIE_xt_core_S8I)(const signed char r, const signed char * p, immediate i) {
Function_TIE_xt_core_S8I((unsigned int)r, (unsigned int)p, i);
return;
}

unsigned short CSTUB_(_TIE_xt_core_L16UI)(const unsigned short * p, immediate i) {
  unsigned int r;
Function_TIE_xt_core_L16UI((unsigned int *)&r, (unsigned int)p, i);
return (unsigned short) r;
}

short CSTUB_(_TIE_xt_core_L16SI)(const short * p, immediate i) {
  unsigned int r;
Function_TIE_xt_core_L16SI((unsigned int *)&r, (unsigned int)p, i);
return (short) r;
}

void CSTUB_(_TIE_xt_core_S16I)(const short r, const short * p, immediate i) {
Function_TIE_xt_core_S16I((unsigned int)r, (unsigned int)p, i);
return;
}

int CSTUB_(_TIE_xt_core_ADDMI)(const int s, immediate i) {
  unsigned int r;
Function_TIE_xt_core_ADDMI((unsigned int *)&r, (unsigned int)s, i);
return (int) r;
}

int CSTUB_(_TIE_xt_core_ADD)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_ADD((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_ADDX2)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_ADDX2((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_ADDX4)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_ADDX4((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_ADDX8)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_ADDX8((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_SUB)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_SUB((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_SUBX2)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_SUBX2((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_SUBX4)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_SUBX4((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_SUBX8)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_SUBX8((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_AND)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_AND((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_XOR)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_XOR((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

unsigned CSTUB_(_TIE_xt_core_EXTUI)(const unsigned t, immediate i, immediate o) {
  unsigned int r;
Function_TIE_xt_core_EXTUI((unsigned int *)&r, t, i, o);
return (unsigned) r;
}

int CSTUB_(_TIE_xt_core_MOVI)(immediate i) {
  unsigned int t;
Function_TIE_xt_core_MOVI((unsigned int *)&t, i);
return (int) t;
}

void CSTUB_(_TIE_xt_core_MOVEQZ)(int* r /*inout*/, const int s, const int t) {
Function_TIE_xt_core_MOVEQZ((unsigned int *)r, (unsigned int)*r, (unsigned int)s, (unsigned int)t);
return;
}

void CSTUB_(_TIE_xt_core_MOVNEZ)(int* r /*inout*/, const int s, const int t) {
Function_TIE_xt_core_MOVNEZ((unsigned int *)r, (unsigned int)*r, (unsigned int)s, (unsigned int)t);
return;
}

void CSTUB_(_TIE_xt_core_MOVLTZ)(int* r /*inout*/, const int s, const int t) {
Function_TIE_xt_core_MOVLTZ((unsigned int *)r, (unsigned int)*r, (unsigned int)s, (unsigned int)t);
return;
}

void CSTUB_(_TIE_xt_core_MOVGEZ)(int* r /*inout*/, const int s, const int t) {
Function_TIE_xt_core_MOVGEZ((unsigned int *)r, (unsigned int)*r, (unsigned int)s, (unsigned int)t);
return;
}

int CSTUB_(_TIE_xt_core_NEG)(const int t) {
  unsigned int r;
Function_TIE_xt_core_NEG((unsigned int *)&r, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_ABS)(const int t) {
  unsigned int r;
Function_TIE_xt_core_ABS((unsigned int *)&r, (unsigned int)t);
return (int) r;
}

void CSTUB_(_TIE_xt_core_SSR)(const int s) {
Function_TIE_xt_core_SSR((unsigned int)s);
return;
}

void CSTUB_(_TIE_xt_core_SSL)(const int s) {
Function_TIE_xt_core_SSL((unsigned int)s);
return;
}

void CSTUB_(_TIE_xt_core_SSA8L)(const int s) {
Function_TIE_xt_core_SSA8L((unsigned int)s);
return;
}

void CSTUB_(_TIE_xt_core_SSA8B)(const int s) {
Function_TIE_xt_core_SSA8B((unsigned int)s);
return;
}

void CSTUB_(_TIE_xt_core_SSAI)(immediate i) {
Function_TIE_xt_core_SSAI(i);
return;
}

int CSTUB_(_TIE_xt_core_SLL)(const int s) {
  unsigned int r;
Function_TIE_xt_core_SLL((unsigned int *)&r, (unsigned int)s);
return (int) r;
}

int CSTUB_(_TIE_xt_core_SRC)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_core_SRC((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

unsigned CSTUB_(_TIE_xt_core_SRL)(const unsigned t) {
  unsigned int r;
Function_TIE_xt_core_SRL((unsigned int *)&r, t);
return (unsigned) r;
}

int CSTUB_(_TIE_xt_core_SRA)(const int t) {
  unsigned int r;
Function_TIE_xt_core_SRA((unsigned int *)&r, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_core_SLLI)(const int s, immediate i) {
  unsigned int r;
Function_TIE_xt_core_SLLI((unsigned int *)&r, (unsigned int)s, i);
return (int) r;
}

int CSTUB_(_TIE_xt_core_SRAI)(const int t, immediate i) {
  unsigned int r;
Function_TIE_xt_core_SRAI((unsigned int *)&r, (unsigned int)t, i);
return (int) r;
}

unsigned CSTUB_(_TIE_xt_core_SRLI)(const unsigned t, immediate i) {
  unsigned int r;
Function_TIE_xt_core_SRLI((unsigned int *)&r, t, i);
return (unsigned) r;
}

int CSTUB_(_TIE_xt_core_SSAI_SRC)(const int src1, const int src2, immediate amount) {
  unsigned int dst;
Function_TIE_xt_core_SSAI_SRC((unsigned int *)&dst, (unsigned int)src1, (unsigned int)src2, amount);
return (int) dst;
}

int CSTUB_(_TIE_xt_core_SSR_SRC)(const int src1, const int src2, const int amount) {
  unsigned int dst;
Function_TIE_xt_core_SSR_SRC((unsigned int *)&dst, (unsigned int)src1, (unsigned int)src2, (unsigned int)amount);
return (int) dst;
}

int CSTUB_(_TIE_xt_core_SSR_SRA)(const int src, const int amount) {
  unsigned int dst;
Function_TIE_xt_core_SSR_SRA((unsigned int *)&dst, (unsigned int)src, (unsigned int)amount);
return (int) dst;
}

unsigned CSTUB_(_TIE_xt_core_SSR_SRL)(const unsigned src, const int amount) {
  unsigned int dst;
Function_TIE_xt_core_SSR_SRL((unsigned int *)&dst, src, (unsigned int)amount);
return (unsigned) dst;
}

int CSTUB_(_TIE_xt_core_SSL_SLL)(const int src, const int amount) {
  unsigned int dst;
Function_TIE_xt_core_SSL_SLL((unsigned int *)&dst, (unsigned int)src, (unsigned int)amount);
return (int) dst;
}

int CSTUB_(_TIE_xt_mul_MUL16S)(const short s, const short t) {
  unsigned int r;
Function_TIE_xt_mul_MUL16S((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

unsigned CSTUB_(_TIE_xt_mul_MUL16U)(const unsigned short s, const unsigned short t) {
  unsigned int r;
Function_TIE_xt_mul_MUL16U((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (unsigned) r;
}

int CSTUB_(_TIE_xt_mul_MULL)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_mul_MULL((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

unsigned CSTUB_(_TIE_xt_mul_MULUH)(const unsigned s, const unsigned t) {
  unsigned int r;
Function_TIE_xt_mul_MULUH((unsigned int *)&r, s, t);
return (unsigned) r;
}

int CSTUB_(_TIE_xt_mul_MULSH)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_mul_MULSH((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_misc_CLAMPS)(const int s, immediate i) {
  unsigned int r;
Function_TIE_xt_misc_CLAMPS((unsigned int *)&r, (unsigned int)s, i);
return (int) r;
}

int CSTUB_(_TIE_xt_misc_MIN)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_misc_MIN((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

int CSTUB_(_TIE_xt_misc_MAX)(const int s, const int t) {
  unsigned int r;
Function_TIE_xt_misc_MAX((unsigned int *)&r, (unsigned int)s, (unsigned int)t);
return (int) r;
}

unsigned CSTUB_(_TIE_xt_misc_MINU)(const unsigned s, const unsigned t) {
  unsigned int r;
Function_TIE_xt_misc_MINU((unsigned int *)&r, s, t);
return (unsigned) r;
}

unsigned CSTUB_(_TIE_xt_misc_MAXU)(const unsigned s, const unsigned t) {
  unsigned int r;
Function_TIE_xt_misc_MAXU((unsigned int *)&r, s, t);
return (unsigned) r;
}

int CSTUB_(_TIE_xt_misc_NSA)(const int s) {
  unsigned int t;
Function_TIE_xt_misc_NSA((unsigned int *)&t, (unsigned int)s);
return (int) t;
}

unsigned CSTUB_(_TIE_xt_misc_NSAU)(const unsigned s) {
  unsigned int t;
Function_TIE_xt_misc_NSAU((unsigned int *)&t, s);
return (unsigned) t;
}

int CSTUB_(_TIE_xt_misc_SEXT)(const int s, immediate i) {
  unsigned int r;
Function_TIE_xt_misc_SEXT((unsigned int *)&r, (unsigned int)s, i);
return (int) r;
}

unsigned CSTUB_(_TIE_xt_core_RSR_SAR)(void) {
  return CSTUB_PROC.states.SAR;
}

void CSTUB_(_TIE_xt_core_WSR_SAR)(unsigned int t) {
  CSTUB_PROC.states.SAR = t & 0x3f;
  return;
}

void CSTUB_(_TIE_xt_core_XSR_SAR)(unsigned* t /*inout*/) {
  unsigned temp = CSTUB_PROC.states.SAR;
  CSTUB_PROC.states.SAR = *t;
  *t = temp;
  return;
}

/* The end */

