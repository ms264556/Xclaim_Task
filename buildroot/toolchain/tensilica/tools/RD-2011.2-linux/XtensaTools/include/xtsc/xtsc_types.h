#ifndef _XTSC_TYPES_H_
#define _XTSC_TYPES_H_

// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#if 0
#define xtsc_types
#endif

namespace xtsc {


// For Microsoft compiler
#ifdef _MSC_EXTENSIONS_

#define XTSC_PRAGMA_WARNING(arg) __pragma(warning(arg))

typedef unsigned __int64    u64;
typedef unsigned int        u32;
typedef unsigned short      u16;
typedef unsigned char       u8;
typedef   signed __int64    i64;
typedef   signed int        i32;
typedef   signed short      i16;
typedef   signed char       i8;

#else

#define XTSC_PRAGMA_WARNING(arg) 

/// 64-bit unsigned number.
typedef unsigned long long  u64;

/// 32-bit unsigned number.
typedef unsigned int        u32;

// 16-bit unsigned number.
typedef unsigned short      u16;

/// 8-bit unsigned number.
typedef unsigned char       u8;

// 64-bit signed number.
typedef   signed long long  i64;

// 32-bit signed number.
typedef   signed int        i32;

// 16-bit signed number.
typedef   signed short      i16;

// 8-bit signed number.
typedef   signed char       i8;

#endif

/// Host word size - signed
typedef i32                 word;

/// Host word size - unsigned
typedef u32                 uword;

/// Xtensa address
typedef unsigned int        xtsc_address;

/// Byte enables
typedef u64                 xtsc_byte_enables;


} // namespace xtsc


#if defined(XTSC_SD) || defined(SYSTEMC_2_1_BETA)
#include "systemc.h"
#if defined(SYSTEMC_2_1_BETA)
#define sc_core
#endif
#else
#include "systemc"
#endif


// The SYSTEMC_VERSION macro is deprecated in the LRM and Cadence IUS81 opted to remove it from their sc_ver.h.
#if !defined(SYSTEMC_VERSION)
#define SYSTEMC_VERSION 20070314
#endif


#if defined(XTSC_DLL)
#  if defined(EXPORT_XTSC_INTERFACE)
#    define XTSC_API __declspec(dllexport)
#  else
#    define XTSC_API __declspec(dllimport)
#    if defined(EXPORT_XTSC_COMP_INTERFACE)
#      define XTSC_COMP_API __declspec(dllexport)
#    else
#      define XTSC_COMP_API __declspec(dllimport)
#    endif
#  endif
#else
#  define XTSC_API
#  define XTSC_COMP_API
#endif

#endif  // _XTSC_TYPES_H_
