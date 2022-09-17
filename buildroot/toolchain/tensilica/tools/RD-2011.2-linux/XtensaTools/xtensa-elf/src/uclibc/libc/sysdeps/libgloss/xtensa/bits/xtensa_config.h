/*
 * This is copied by xtensa's custom configure script to the build area
 * xtensa does this by hand to support building without menuconfig and such
 *
 * Automatically generated C config: don't edit
 */
#if !defined __FEATURES_H && !defined __need_uClibc_config_h
#error Never include <bits/uClibc_config.h> directly; use <features.h> instead
#endif
#define AUTOCONF_INCLUDED

#include <xtensa/config/system.h>
#include <xtensa/hal.h>

/*
 * Version Number
 */
#define __UCLIBC_MAJOR__ 0
#define __UCLIBC_MINOR__ 9
#define __UCLIBC_SUBLEVEL__ 26
#undef __TARGET_alpha__
#undef __TARGET_arm__
#undef __TARGET_cris__
#undef __TARGET_e1__
#undef __TARGET_h8300__
#undef __TARGET_i386__
#undef __TARGET_i960__
#undef __TARGET_m68k__
#undef __TARGET_microblaze__
#undef __TARGET_mips__
#undef __TARGET_powerpc__
#undef __TARGET_sh__
#undef __TARGET_sparc__
#undef __TARGET_v850__
#define __TARGET_xtensa__ 1

/*
 * Target OS
 */
#define __TARGET_OS__ "libgloss"

/*
 * Target Architecture Features and Options
 */
#define __HAVE_ELF__ 1
#define __TARGET_ARCH__ "xtensa"
#define __CONFIG_GENERIC_XTENSA__ 1
#undef __ARCH_HAS_NO_MMU__
#undef __UCLIBC_HAS_MMU__
#if XSHAL_USE_FLOATING_POINT
# define __UCLIBC_HAS_FLOATS__ 1
#else
# undef __UCLIBC_HAS_FLOATS__
#endif

#if XTHAL_LITTLEENDIAN
#define __LITTLE_ENDIAN__ 1
#endif

#if XTHAL_BIGENDIAN
#define __BIG_ENDIAN__ 1
#endif

#if (!__LITTLE_ENDIAN && !__BIG_ENDIAN__)
#error no endianness found!
#endif

#undef __HAS_FPU__
#define __UCLIBC_HAS_SOFT_FLOAT__ 1
#undef __DO_C99_MATH__
#define __WARNINGS__ "-Wall"
#define __KERNEL_SOURCE__ "/usr/src/linux"
#define __UCLIBC_UCLINUX_BROKEN_MUNMAP__ 1
#define __EXCLUDE_BRK__ 1
#define __C_SYMBOL_PREFIX__ ""
#define __HAVE_DOT_CONFIG__ 1

/*
 * General Library Settings
 */
#undef __HAVE_NO_PIC__
#undef __DOPIC__
#undef __HAVE_NO_SHARED__
#undef __ARCH_HAS_NO_LDSO__
#undef __UCLIBC_PIE_SUPPORT__
#define __UCLIBC_CTOR_DTOR__ 1
#undef __UCLIBC_PROPOLICE__
#undef __UCLIBC_PROFILING__
#undef __HAS_NO_THREADS__
#undef __UCLIBC_HAS_THREADS__
#undef __UCLIBC_HAS_LFS__
#define __MALLOC__ 1
#undef __MALLOC_SIMPLE__
#undef __MALLOC_STANDARD__
#undef __MALLOC_GLIBC_COMPAT__
#undef __MALLOC_USE_SBRK__
#define __UCLIBC_DYNAMIC_ATEXIT__ 1
#undef __HAS_SHADOW__
#define __UNIX98PTY_ONLY__ 1
#undef __ASSUME_DEVPTS__
#undef __UCLIBC_HAS_TM_EXTENSIONS__
#undef __UCLIBC_HAS_TZ_CACHING__
#undef __UCLIBC_HAS_TZ_FILE__

/*
 * Networking Support
 */
#undef __UCLIBC_HAS_IPV6__
#undef __UCLIBC_HAS_RPC__

/*
 * String and Stdio Support
 */
#define __UCLIBC_HAS_CTYPE_TABLES__ 1
#define __UCLIBC_HAS_CTYPE_SIGNED__ 1
#define __UCLIBC_HAS_CTYPE_UNSAFE__ 1
#undef __UCLIBC_HAS_CTYPE_CHECKED__
#undef __UCLIBC_HAS_CTYPE_ENFORCED__
#undef __UCLIBC_HAS_WCHAR__
#undef __UCLIBC_HAS_LOCALE__
#undef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
#undef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__
#undef __USE_OLD_VFPRINTF__
#define __UCLIBC_PRINTF_SCANF_POSITIONAL_ARGS__ 9
#undef __UCLIBC_HAS_SCANF_GLIBC_A_FLAG__
#undef __UCLIBC_HAS_STDIO_BUFSIZ_NONE__
#undef __UCLIBC_HAS_STDIO_BUFSIZ_256__
#define __UCLIBC_HAS_STDIO_BUFSIZ_512__ 1
#undef __UCLIBC_HAS_STDIO_BUFSIZ_1024__
#undef __UCLIBC_HAS_STDIO_BUFSIZ_2048__
#undef __UCLIBC_HAS_STDIO_BUFSIZ_4096__ 1
#undef __UCLIBC_HAS_STDIO_BUFSIZ_8192__
#define __UCLIBC_HAS_STDIO_BUILTIN_BUFFER_NONE__ 1
#undef __UCLIBC_HAS_STDIO_BUILTIN_BUFFER_4__
#undef __UCLIBC_HAS_STDIO_BUILTIN_BUFFER_8__
#define __UCLIBC_HAS_STDIO_GETC_MACRO__ 1
#define __UCLIBC_HAS_STDIO_PUTC_MACRO__ 1
#define __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__ 1
#undef __UCLIBC_HAS_FOPEN_EXCLUSIVE_MODE__
#undef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
#undef __UCLIBC_HAS_PRINTF_M_SPEC__
#undef __UCLIBC_HAS_ERRNO_MESSAGES__
#undef __UCLIBC_HAS_SIGNUM_MESSAGES__
#define __UCLIBC_HAS_GNU_GETOPT__ 1

/*
 * Big and Tall
 */
#undef __UCLIBC_HAS_REGEX__
#undef __UCLIBC_HAS_WORDEXP__
#undef __UCLIBC_HAS_FTW__
#undef __UCLIBC_HAS_GLOB__

/*
 * Library Installation Options
 */
#define __RUNTIME_PREFIX__ "/usr/$(TARGET_ARCH)-linux-uclibc/"
#define __DEVEL_PREFIX__ "/usr/$(TARGET_ARCH)-linux-uclibc/usr/"

/*
 * uClibc development/debugging options
 */
#undef __DODEBUG__
#undef __DOASSERTS__
#undef __UCLIBC_MALLOC_DEBUGGING__
#undef __UCLIBC_MJN3_ONLY__
