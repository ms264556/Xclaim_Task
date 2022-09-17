/* glue.h -- common definitions for "glue" fucntions.
 *
 * Copyright (c) 1995 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#define REENTRANT_SYSCALLS_PROVIDED 1
#define _AND ,
#define _DEFUN(name, arglist, args)     name(args)
#define _DEFUN_VOID(name)               name(void)
#define _EXFUN(name, proto)             name proto
#define __MYPID 1


#ifdef __NO_UNDERSCORE__
#  define _end    end
#  define _exit   exit
#endif

extern char _end[];                /* _end is set in the linker command file */
