/*
 * Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the.
 * copyrighted works and confidential proprietary information of Tensilica Inc..
 * They may not be modified, copied, reproduced, distributed, or disclosed to.
 * third parties in any manner, medium, or form, in whole or in part, without.
 * the prior written consent of Tensilica Inc..
 */

/* Do not modify. This is automatically generated.*/

#ifndef CSTUB_Xm_peregrine_HEADER
#define CSTUB_Xm_peregrine_HEADER

#include <string.h>

/* Cstub definitions */

#if defined(__GNUC__)
#define CSTUB_MSC_ALIGN(x) 
#define CSTUB_GCC_ALIGN(x) __attribute__((aligned(x))) 
#elif defined(_MSC_VER)
#define CSTUB_MSC_ALIGN(x) _declspec(align(x)) 
#define CSTUB_GCC_ALIGN(x) 
#else 
#error "Error: Only GCC/G++ and Visual C++ are supported"
#endif

#define CSTUB_(X) cstub_##X

/* Ctype macros */

#if defined(__cplusplus)
/* Ctype declarations */
typedef int immediate;

/* Ctype definitions */
/* Proto declarations */
extern void CSTUB_(_TIE_xt_density_NOP_N)(void);
extern void CSTUB_(_TIE_xt_core_NOP)(void);
extern int CSTUB_(_TIE_xt_density_L32I_N)(const int * p, immediate i);
extern void CSTUB_(_TIE_xt_density_S32I_N)(const int t, const int * p, immediate i);
extern int CSTUB_(_TIE_xt_density_ADD_N)(const int s, const int t);
extern int CSTUB_(_TIE_xt_density_ADDI_N)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_density_MOV_N)(const int s);
extern int CSTUB_(_TIE_xt_density_MOVI_N)(immediate i);
extern unsigned CSTUB_(_TIE_xt_core_uint32_loadi)(const unsigned * p, immediate o);
extern void CSTUB_(_TIE_xt_core_uint32_storei)(const unsigned c, const unsigned * p, immediate o);
extern unsigned CSTUB_(_TIE_xt_core_uint32_move)(const unsigned b);
extern int CSTUB_(_TIE_xt_core_ADDI)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_core_OR)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_L32I)(const int * p, immediate i);
extern void CSTUB_(_TIE_xt_core_S32I)(const int r, const int * p, immediate i);
extern unsigned char CSTUB_(_TIE_xt_core_L8UI)(const unsigned char * p, immediate i);
extern void CSTUB_(_TIE_xt_core_S8I)(const signed char r, const signed char * p, immediate i);
extern unsigned short CSTUB_(_TIE_xt_core_L16UI)(const unsigned short * p, immediate i);
extern short CSTUB_(_TIE_xt_core_L16SI)(const short * p, immediate i);
extern void CSTUB_(_TIE_xt_core_S16I)(const short r, const short * p, immediate i);
extern int CSTUB_(_TIE_xt_core_ADDMI)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_core_ADD)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_ADDX2)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_ADDX4)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_ADDX8)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUB)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUBX2)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUBX4)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUBX8)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_AND)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_XOR)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_core_EXTUI)(const unsigned t, immediate i, immediate o);
extern int CSTUB_(_TIE_xt_core_MOVI)(immediate i);
extern void CSTUB_(_TIE_xt_core_MOVEQZ)(int& r /*inout*/, const int s, const int t);
extern void CSTUB_(_TIE_xt_core_MOVNEZ)(int& r /*inout*/, const int s, const int t);
extern void CSTUB_(_TIE_xt_core_MOVLTZ)(int& r /*inout*/, const int s, const int t);
extern void CSTUB_(_TIE_xt_core_MOVGEZ)(int& r /*inout*/, const int s, const int t);
extern int CSTUB_(_TIE_xt_core_NEG)(const int t);
extern int CSTUB_(_TIE_xt_core_ABS)(const int t);
extern void CSTUB_(_TIE_xt_core_SSR)(const int s);
extern void CSTUB_(_TIE_xt_core_SSL)(const int s);
extern void CSTUB_(_TIE_xt_core_SSA8L)(const int s);
extern void CSTUB_(_TIE_xt_core_SSA8B)(const int s);
extern void CSTUB_(_TIE_xt_core_SSAI)(immediate i);
extern int CSTUB_(_TIE_xt_core_SLL)(const int s);
extern int CSTUB_(_TIE_xt_core_SRC)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_core_SRL)(const unsigned t);
extern int CSTUB_(_TIE_xt_core_SRA)(const int t);
extern int CSTUB_(_TIE_xt_core_SLLI)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_core_SRAI)(const int t, immediate i);
extern unsigned CSTUB_(_TIE_xt_core_SRLI)(const unsigned t, immediate i);
extern int CSTUB_(_TIE_xt_core_SSAI_SRC)(const int src1, const int src2, immediate amount);
extern int CSTUB_(_TIE_xt_core_SSR_SRC)(const int src1, const int src2, const int amount);
extern int CSTUB_(_TIE_xt_core_SSR_SRA)(const int src, const int amount);
extern unsigned CSTUB_(_TIE_xt_core_SSR_SRL)(const unsigned src, const int amount);
extern int CSTUB_(_TIE_xt_core_SSL_SLL)(const int src, const int amount);
extern int CSTUB_(_TIE_xt_mul_MUL16S)(const short s, const short t);
extern unsigned CSTUB_(_TIE_xt_mul_MUL16U)(const unsigned short s, const unsigned short t);
extern int CSTUB_(_TIE_xt_mul_MULL)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_mul_MULUH)(const unsigned s, const unsigned t);
extern int CSTUB_(_TIE_xt_mul_MULSH)(const int s, const int t);
extern int CSTUB_(_TIE_xt_misc_CLAMPS)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_misc_MIN)(const int s, const int t);
extern int CSTUB_(_TIE_xt_misc_MAX)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_misc_MINU)(const unsigned s, const unsigned t);
extern unsigned CSTUB_(_TIE_xt_misc_MAXU)(const unsigned s, const unsigned t);
extern int CSTUB_(_TIE_xt_misc_NSA)(const int s);
extern unsigned CSTUB_(_TIE_xt_misc_NSAU)(const unsigned s);
extern int CSTUB_(_TIE_xt_misc_SEXT)(const int s, immediate i);

extern unsigned CSTUB_(_TIE_xt_core_RSR_SAR)(void);
extern void CSTUB_(_TIE_xt_core_WSR_SAR)(unsigned t);
extern void CSTUB_(_TIE_xt_core_XSR_SAR)(unsigned& t /*inout*/);
/* Ctype convertion functions */


/* Proto macros */
#define XT_NOP_N() \
	CSTUB_(_TIE_xt_density_NOP_N)()

#define XT_NOP() \
	CSTUB_(_TIE_xt_core_NOP)()

#define XT_L32I_N(p, i) \
	CSTUB_(_TIE_xt_density_L32I_N)(p, i)

#define XT_S32I_N(t, p, i) \
	CSTUB_(_TIE_xt_density_S32I_N)(t, p, i)

#define XT_ADD_N(s, t) \
	CSTUB_(_TIE_xt_density_ADD_N)(s, t)

#define XT_ADDI_N(s, i) \
	CSTUB_(_TIE_xt_density_ADDI_N)(s, i)

#define XT_MOV_N(s) \
	CSTUB_(_TIE_xt_density_MOV_N)(s)

#define XT_MOVI_N(i) \
	CSTUB_(_TIE_xt_density_MOVI_N)(i)

#define XT_uint32_loadi(p, o) \
	CSTUB_(_TIE_xt_core_uint32_loadi)(p, o)

#define XT_uint32_storei(c, p, o) \
	CSTUB_(_TIE_xt_core_uint32_storei)(c, p, o)

#define XT_uint32_move(b) \
	CSTUB_(_TIE_xt_core_uint32_move)(b)

#define XT_ADDI(s, i) \
	CSTUB_(_TIE_xt_core_ADDI)(s, i)

#define XT_OR(s, t) \
	CSTUB_(_TIE_xt_core_OR)(s, t)

#define XT_L32I(p, i) \
	CSTUB_(_TIE_xt_core_L32I)(p, i)

#define XT_S32I(r, p, i) \
	CSTUB_(_TIE_xt_core_S32I)(r, p, i)

#define XT_L8UI(p, i) \
	CSTUB_(_TIE_xt_core_L8UI)(p, i)

#define XT_S8I(r, p, i) \
	CSTUB_(_TIE_xt_core_S8I)(r, p, i)

#define XT_L16UI(p, i) \
	CSTUB_(_TIE_xt_core_L16UI)(p, i)

#define XT_L16SI(p, i) \
	CSTUB_(_TIE_xt_core_L16SI)(p, i)

#define XT_S16I(r, p, i) \
	CSTUB_(_TIE_xt_core_S16I)(r, p, i)

#define XT_ADDMI(s, i) \
	CSTUB_(_TIE_xt_core_ADDMI)(s, i)

#define XT_ADD(s, t) \
	CSTUB_(_TIE_xt_core_ADD)(s, t)

#define XT_ADDX2(s, t) \
	CSTUB_(_TIE_xt_core_ADDX2)(s, t)

#define XT_ADDX4(s, t) \
	CSTUB_(_TIE_xt_core_ADDX4)(s, t)

#define XT_ADDX8(s, t) \
	CSTUB_(_TIE_xt_core_ADDX8)(s, t)

#define XT_SUB(s, t) \
	CSTUB_(_TIE_xt_core_SUB)(s, t)

#define XT_SUBX2(s, t) \
	CSTUB_(_TIE_xt_core_SUBX2)(s, t)

#define XT_SUBX4(s, t) \
	CSTUB_(_TIE_xt_core_SUBX4)(s, t)

#define XT_SUBX8(s, t) \
	CSTUB_(_TIE_xt_core_SUBX8)(s, t)

#define XT_AND(s, t) \
	CSTUB_(_TIE_xt_core_AND)(s, t)

#define XT_XOR(s, t) \
	CSTUB_(_TIE_xt_core_XOR)(s, t)

#define XT_EXTUI(t, i, o) \
	CSTUB_(_TIE_xt_core_EXTUI)(t, i, o)

#define XT_MOVI(i) \
	CSTUB_(_TIE_xt_core_MOVI)(i)

#define XT_MOVEQZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVEQZ)(r, s, t)

#define XT_MOVNEZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVNEZ)(r, s, t)

#define XT_MOVLTZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVLTZ)(r, s, t)

#define XT_MOVGEZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVGEZ)(r, s, t)

#define XT_NEG(t) \
	CSTUB_(_TIE_xt_core_NEG)(t)

#define XT_ABS(t) \
	CSTUB_(_TIE_xt_core_ABS)(t)

#define XT_SSR(s) \
	CSTUB_(_TIE_xt_core_SSR)(s)

#define XT_SSL(s) \
	CSTUB_(_TIE_xt_core_SSL)(s)

#define XT_SSA8L(s) \
	CSTUB_(_TIE_xt_core_SSA8L)(s)

#define XT_SSA8B(s) \
	CSTUB_(_TIE_xt_core_SSA8B)(s)

#define XT_SSAI(i) \
	CSTUB_(_TIE_xt_core_SSAI)(i)

#define XT_SLL(s) \
	CSTUB_(_TIE_xt_core_SLL)(s)

#define XT_SRC(s, t) \
	CSTUB_(_TIE_xt_core_SRC)(s, t)

#define XT_SRL(t) \
	CSTUB_(_TIE_xt_core_SRL)(t)

#define XT_SRA(t) \
	CSTUB_(_TIE_xt_core_SRA)(t)

#define XT_SLLI(s, i) \
	CSTUB_(_TIE_xt_core_SLLI)(s, i)

#define XT_SRAI(t, i) \
	CSTUB_(_TIE_xt_core_SRAI)(t, i)

#define XT_SRLI(t, i) \
	CSTUB_(_TIE_xt_core_SRLI)(t, i)

#define XT_SSAI_SRC(src1, src2, amount) \
	CSTUB_(_TIE_xt_core_SSAI_SRC)(src1, src2, amount)

#define XT_SSR_SRC(src1, src2, amount) \
	CSTUB_(_TIE_xt_core_SSR_SRC)(src1, src2, amount)

#define XT_SSR_SRA(src, amount) \
	CSTUB_(_TIE_xt_core_SSR_SRA)(src, amount)

#define XT_SSR_SRL(src, amount) \
	CSTUB_(_TIE_xt_core_SSR_SRL)(src, amount)

#define XT_SSL_SLL(src, amount) \
	CSTUB_(_TIE_xt_core_SSL_SLL)(src, amount)

#define XT_MUL16S(s, t) \
	CSTUB_(_TIE_xt_mul_MUL16S)(s, t)

#define XT_MUL16U(s, t) \
	CSTUB_(_TIE_xt_mul_MUL16U)(s, t)

#define XT_MULL(s, t) \
	CSTUB_(_TIE_xt_mul_MULL)(s, t)

#define XT_MULUH(s, t) \
	CSTUB_(_TIE_xt_mul_MULUH)(s, t)

#define XT_MULSH(s, t) \
	CSTUB_(_TIE_xt_mul_MULSH)(s, t)

#define XT_CLAMPS(s, i) \
	CSTUB_(_TIE_xt_misc_CLAMPS)(s, i)

#define XT_MIN(s, t) \
	CSTUB_(_TIE_xt_misc_MIN)(s, t)

#define XT_MAX(s, t) \
	CSTUB_(_TIE_xt_misc_MAX)(s, t)

#define XT_MINU(s, t) \
	CSTUB_(_TIE_xt_misc_MINU)(s, t)

#define XT_MAXU(s, t) \
	CSTUB_(_TIE_xt_misc_MAXU)(s, t)

#define XT_NSA(s) \
	CSTUB_(_TIE_xt_misc_NSA)(s)

#define XT_NSAU(s) \
	CSTUB_(_TIE_xt_misc_NSAU)(s)

#define XT_SEXT(s, i) \
	CSTUB_(_TIE_xt_misc_SEXT)(s, i)

#define XT_RSR_SAR() \
	CSTUB_(_TIE_xt_core_RSR_SAR)()

#define XT_WSR_SAR(t) \
	CSTUB_(_TIE_xt_core_WSR_SAR)(t)

#define XT_XSR_SAR(t) \
	CSTUB_(_TIE_xt_core_XSR_SAR)(t)

#else /* !__cplusplus */
/* Ctype declarations */
typedef int immediate;

/* Ctype definitions */
/* Proto declarations */
extern void CSTUB_(_TIE_xt_density_NOP_N)(void);
extern void CSTUB_(_TIE_xt_core_NOP)(void);
extern int CSTUB_(_TIE_xt_density_L32I_N)(const int * p, immediate i);
extern void CSTUB_(_TIE_xt_density_S32I_N)(const int t, const int * p, immediate i);
extern int CSTUB_(_TIE_xt_density_ADD_N)(const int s, const int t);
extern int CSTUB_(_TIE_xt_density_ADDI_N)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_density_MOV_N)(const int s);
extern int CSTUB_(_TIE_xt_density_MOVI_N)(immediate i);
extern unsigned CSTUB_(_TIE_xt_core_uint32_loadi)(const unsigned * p, immediate o);
extern void CSTUB_(_TIE_xt_core_uint32_storei)(const unsigned c, const unsigned * p, immediate o);
extern unsigned CSTUB_(_TIE_xt_core_uint32_move)(const unsigned b);
extern int CSTUB_(_TIE_xt_core_ADDI)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_core_OR)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_L32I)(const int * p, immediate i);
extern void CSTUB_(_TIE_xt_core_S32I)(const int r, const int * p, immediate i);
extern unsigned char CSTUB_(_TIE_xt_core_L8UI)(const unsigned char * p, immediate i);
extern void CSTUB_(_TIE_xt_core_S8I)(const signed char r, const signed char * p, immediate i);
extern unsigned short CSTUB_(_TIE_xt_core_L16UI)(const unsigned short * p, immediate i);
extern short CSTUB_(_TIE_xt_core_L16SI)(const short * p, immediate i);
extern void CSTUB_(_TIE_xt_core_S16I)(const short r, const short * p, immediate i);
extern int CSTUB_(_TIE_xt_core_ADDMI)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_core_ADD)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_ADDX2)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_ADDX4)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_ADDX8)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUB)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUBX2)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUBX4)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_SUBX8)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_AND)(const int s, const int t);
extern int CSTUB_(_TIE_xt_core_XOR)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_core_EXTUI)(const unsigned t, immediate i, immediate o);
extern int CSTUB_(_TIE_xt_core_MOVI)(immediate i);
extern void CSTUB_(_TIE_xt_core_MOVEQZ)(int* r /*inout*/, const int s, const int t);
extern void CSTUB_(_TIE_xt_core_MOVNEZ)(int* r /*inout*/, const int s, const int t);
extern void CSTUB_(_TIE_xt_core_MOVLTZ)(int* r /*inout*/, const int s, const int t);
extern void CSTUB_(_TIE_xt_core_MOVGEZ)(int* r /*inout*/, const int s, const int t);
extern int CSTUB_(_TIE_xt_core_NEG)(const int t);
extern int CSTUB_(_TIE_xt_core_ABS)(const int t);
extern void CSTUB_(_TIE_xt_core_SSR)(const int s);
extern void CSTUB_(_TIE_xt_core_SSL)(const int s);
extern void CSTUB_(_TIE_xt_core_SSA8L)(const int s);
extern void CSTUB_(_TIE_xt_core_SSA8B)(const int s);
extern void CSTUB_(_TIE_xt_core_SSAI)(immediate i);
extern int CSTUB_(_TIE_xt_core_SLL)(const int s);
extern int CSTUB_(_TIE_xt_core_SRC)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_core_SRL)(const unsigned t);
extern int CSTUB_(_TIE_xt_core_SRA)(const int t);
extern int CSTUB_(_TIE_xt_core_SLLI)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_core_SRAI)(const int t, immediate i);
extern unsigned CSTUB_(_TIE_xt_core_SRLI)(const unsigned t, immediate i);
extern int CSTUB_(_TIE_xt_core_SSAI_SRC)(const int src1, const int src2, immediate amount);
extern int CSTUB_(_TIE_xt_core_SSR_SRC)(const int src1, const int src2, const int amount);
extern int CSTUB_(_TIE_xt_core_SSR_SRA)(const int src, const int amount);
extern unsigned CSTUB_(_TIE_xt_core_SSR_SRL)(const unsigned src, const int amount);
extern int CSTUB_(_TIE_xt_core_SSL_SLL)(const int src, const int amount);
extern int CSTUB_(_TIE_xt_mul_MUL16S)(const short s, const short t);
extern unsigned CSTUB_(_TIE_xt_mul_MUL16U)(const unsigned short s, const unsigned short t);
extern int CSTUB_(_TIE_xt_mul_MULL)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_mul_MULUH)(const unsigned s, const unsigned t);
extern int CSTUB_(_TIE_xt_mul_MULSH)(const int s, const int t);
extern int CSTUB_(_TIE_xt_misc_CLAMPS)(const int s, immediate i);
extern int CSTUB_(_TIE_xt_misc_MIN)(const int s, const int t);
extern int CSTUB_(_TIE_xt_misc_MAX)(const int s, const int t);
extern unsigned CSTUB_(_TIE_xt_misc_MINU)(const unsigned s, const unsigned t);
extern unsigned CSTUB_(_TIE_xt_misc_MAXU)(const unsigned s, const unsigned t);
extern int CSTUB_(_TIE_xt_misc_NSA)(const int s);
extern unsigned CSTUB_(_TIE_xt_misc_NSAU)(const unsigned s);
extern int CSTUB_(_TIE_xt_misc_SEXT)(const int s, immediate i);

extern unsigned CSTUB_(_TIE_xt_core_RSR_SAR)(void);
extern void CSTUB_(_TIE_xt_core_WSR_SAR)(unsigned t);
extern void CSTUB_(_TIE_xt_core_XSR_SAR)(unsigned* t /*inout*/);
/* Proto macros */
#define XT_NOP_N() \
	CSTUB_(_TIE_xt_density_NOP_N)()

#define XT_NOP() \
	CSTUB_(_TIE_xt_core_NOP)()

#define XT_L32I_N(p, i) \
	CSTUB_(_TIE_xt_density_L32I_N)(p, i)

#define XT_S32I_N(t, p, i) \
	CSTUB_(_TIE_xt_density_S32I_N)(t, p, i)

#define XT_ADD_N(s, t) \
	CSTUB_(_TIE_xt_density_ADD_N)(s, t)

#define XT_ADDI_N(s, i) \
	CSTUB_(_TIE_xt_density_ADDI_N)(s, i)

#define XT_MOV_N(s) \
	CSTUB_(_TIE_xt_density_MOV_N)(s)

#define XT_MOVI_N(i) \
	CSTUB_(_TIE_xt_density_MOVI_N)(i)

#define XT_uint32_loadi(p, o) \
	CSTUB_(_TIE_xt_core_uint32_loadi)(p, o)

#define XT_uint32_storei(c, p, o) \
	CSTUB_(_TIE_xt_core_uint32_storei)(c, p, o)

#define XT_uint32_move(b) \
	CSTUB_(_TIE_xt_core_uint32_move)(b)

#define XT_ADDI(s, i) \
	CSTUB_(_TIE_xt_core_ADDI)(s, i)

#define XT_OR(s, t) \
	CSTUB_(_TIE_xt_core_OR)(s, t)

#define XT_L32I(p, i) \
	CSTUB_(_TIE_xt_core_L32I)(p, i)

#define XT_S32I(r, p, i) \
	CSTUB_(_TIE_xt_core_S32I)(r, p, i)

#define XT_L8UI(p, i) \
	CSTUB_(_TIE_xt_core_L8UI)(p, i)

#define XT_S8I(r, p, i) \
	CSTUB_(_TIE_xt_core_S8I)(r, p, i)

#define XT_L16UI(p, i) \
	CSTUB_(_TIE_xt_core_L16UI)(p, i)

#define XT_L16SI(p, i) \
	CSTUB_(_TIE_xt_core_L16SI)(p, i)

#define XT_S16I(r, p, i) \
	CSTUB_(_TIE_xt_core_S16I)(r, p, i)

#define XT_ADDMI(s, i) \
	CSTUB_(_TIE_xt_core_ADDMI)(s, i)

#define XT_ADD(s, t) \
	CSTUB_(_TIE_xt_core_ADD)(s, t)

#define XT_ADDX2(s, t) \
	CSTUB_(_TIE_xt_core_ADDX2)(s, t)

#define XT_ADDX4(s, t) \
	CSTUB_(_TIE_xt_core_ADDX4)(s, t)

#define XT_ADDX8(s, t) \
	CSTUB_(_TIE_xt_core_ADDX8)(s, t)

#define XT_SUB(s, t) \
	CSTUB_(_TIE_xt_core_SUB)(s, t)

#define XT_SUBX2(s, t) \
	CSTUB_(_TIE_xt_core_SUBX2)(s, t)

#define XT_SUBX4(s, t) \
	CSTUB_(_TIE_xt_core_SUBX4)(s, t)

#define XT_SUBX8(s, t) \
	CSTUB_(_TIE_xt_core_SUBX8)(s, t)

#define XT_AND(s, t) \
	CSTUB_(_TIE_xt_core_AND)(s, t)

#define XT_XOR(s, t) \
	CSTUB_(_TIE_xt_core_XOR)(s, t)

#define XT_EXTUI(t, i, o) \
	CSTUB_(_TIE_xt_core_EXTUI)(t, i, o)

#define XT_MOVI(i) \
	CSTUB_(_TIE_xt_core_MOVI)(i)

#define XT_MOVEQZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVEQZ)(&r, s, t)

#define XT_MOVNEZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVNEZ)(&r, s, t)

#define XT_MOVLTZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVLTZ)(&r, s, t)

#define XT_MOVGEZ(r, s, t) \
	CSTUB_(_TIE_xt_core_MOVGEZ)(&r, s, t)

#define XT_NEG(t) \
	CSTUB_(_TIE_xt_core_NEG)(t)

#define XT_ABS(t) \
	CSTUB_(_TIE_xt_core_ABS)(t)

#define XT_SSR(s) \
	CSTUB_(_TIE_xt_core_SSR)(s)

#define XT_SSL(s) \
	CSTUB_(_TIE_xt_core_SSL)(s)

#define XT_SSA8L(s) \
	CSTUB_(_TIE_xt_core_SSA8L)(s)

#define XT_SSA8B(s) \
	CSTUB_(_TIE_xt_core_SSA8B)(s)

#define XT_SSAI(i) \
	CSTUB_(_TIE_xt_core_SSAI)(i)

#define XT_SLL(s) \
	CSTUB_(_TIE_xt_core_SLL)(s)

#define XT_SRC(s, t) \
	CSTUB_(_TIE_xt_core_SRC)(s, t)

#define XT_SRL(t) \
	CSTUB_(_TIE_xt_core_SRL)(t)

#define XT_SRA(t) \
	CSTUB_(_TIE_xt_core_SRA)(t)

#define XT_SLLI(s, i) \
	CSTUB_(_TIE_xt_core_SLLI)(s, i)

#define XT_SRAI(t, i) \
	CSTUB_(_TIE_xt_core_SRAI)(t, i)

#define XT_SRLI(t, i) \
	CSTUB_(_TIE_xt_core_SRLI)(t, i)

#define XT_SSAI_SRC(src1, src2, amount) \
	CSTUB_(_TIE_xt_core_SSAI_SRC)(src1, src2, amount)

#define XT_SSR_SRC(src1, src2, amount) \
	CSTUB_(_TIE_xt_core_SSR_SRC)(src1, src2, amount)

#define XT_SSR_SRA(src, amount) \
	CSTUB_(_TIE_xt_core_SSR_SRA)(src, amount)

#define XT_SSR_SRL(src, amount) \
	CSTUB_(_TIE_xt_core_SSR_SRL)(src, amount)

#define XT_SSL_SLL(src, amount) \
	CSTUB_(_TIE_xt_core_SSL_SLL)(src, amount)

#define XT_MUL16S(s, t) \
	CSTUB_(_TIE_xt_mul_MUL16S)(s, t)

#define XT_MUL16U(s, t) \
	CSTUB_(_TIE_xt_mul_MUL16U)(s, t)

#define XT_MULL(s, t) \
	CSTUB_(_TIE_xt_mul_MULL)(s, t)

#define XT_MULUH(s, t) \
	CSTUB_(_TIE_xt_mul_MULUH)(s, t)

#define XT_MULSH(s, t) \
	CSTUB_(_TIE_xt_mul_MULSH)(s, t)

#define XT_CLAMPS(s, i) \
	CSTUB_(_TIE_xt_misc_CLAMPS)(s, i)

#define XT_MIN(s, t) \
	CSTUB_(_TIE_xt_misc_MIN)(s, t)

#define XT_MAX(s, t) \
	CSTUB_(_TIE_xt_misc_MAX)(s, t)

#define XT_MINU(s, t) \
	CSTUB_(_TIE_xt_misc_MINU)(s, t)

#define XT_MAXU(s, t) \
	CSTUB_(_TIE_xt_misc_MAXU)(s, t)

#define XT_NSA(s) \
	CSTUB_(_TIE_xt_misc_NSA)(s)

#define XT_NSAU(s) \
	CSTUB_(_TIE_xt_misc_NSAU)(s)

#define XT_SEXT(s, i) \
	CSTUB_(_TIE_xt_misc_SEXT)(s, i)

#define XT_RSR_SAR() \
	CSTUB_(_TIE_xt_core_RSR_SAR)()

#define XT_WSR_SAR(t) \
	CSTUB_(_TIE_xt_core_WSR_SAR)(t)

#define XT_XSR_SAR(t) \
	CSTUB_(_TIE_xt_core_XSR_SAR)((unsigned *)&t)

#endif /* __cplusplus */

/* User register read/write functions */
#ifndef RUR
#define RUR(NUM) RUR##NUM()
#endif /* RUR */

#ifndef WUR
#define WUR(VAL, NUM) WUR##NUM(VAL)
#endif /* WUR */


#endif /* !CSTUB_Xm_peregrine_HEADER */
