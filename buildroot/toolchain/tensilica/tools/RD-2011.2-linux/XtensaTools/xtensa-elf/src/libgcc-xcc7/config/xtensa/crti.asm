# Start .init and .fini sections.
# Copyright (C) 2003 Free Software Foundation, Inc.
# 
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
# 
# In addition to the permissions in the GNU General Public License, the
# Free Software Foundation gives you unlimited permission to link the
# compiled version of this file into combinations with other programs,
# and to distribute those combinations without any restriction coming
# from the use of this file.  (The General Public License restrictions
# do apply in other respects; for example, they cover modification of
# the file, and distribution when not linked into a combine
# executable.)
# 
# GCC is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GCC; see the file COPYING.  If not, write to the Free
# Software Foundation, 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# This file just makes a stack frame for the contents of the .fini and
# .init sections.  Users may put any desired instructions in those
# sections.

#include <xtensa/config/core.h>

	.section .init
	.globl _init
	.type _init,@function
	.align	4
_init:
#if XCHAL_HAVE_WINDOWED && !__XTENSA_CALL0_ABI__
	entry	sp, 64
#else
	addi	sp, sp, -32
	s32i	a0, sp, 0
#endif

	.section .fini
	.globl _fini
	.type _fini,@function
	.align	4
_fini:
#if XCHAL_HAVE_WINDOWED && !__XTENSA_CALL0_ABI__
	entry	sp, 64
#else
	addi	sp, sp, -32
	s32i	a0, sp, 0
#endif
