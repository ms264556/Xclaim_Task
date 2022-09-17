#ifndef _ENDIAN_H
# error "Never use <bits/endian.h> directly; include <endian.h> instead."
#endif

#ifdef __XTENSA_EB__
#define __BYTE_ORDER __BIG_ENDIAN
#define __FLOAT_WORD_ORDER __BIG_ENDIAN
#endif
#ifdef __XTENSA_EL__ 
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __FLOAT_WORD_ORDER __LITTLE_ENDIAN
#endif
#ifndef __BYTE_ORDER
#error What endianness is this config?
#endif

