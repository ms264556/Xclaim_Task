# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000,2001 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.

TOPDIR=../../../
include $(TOPDIR)Rules.mak

CSRC=__uClibc_main.c errno.c __errno_location.c tempname.c
ifeq ($(TARGET_OS), "linux")
CSRC += __h_errno_location.c
endif

COBJS=$(patsubst %.c,%.o, $(CSRC))

OBJS=$(COBJS)

all: $(OBJS) interp.o $(LIBC)

$(LIBC): interp.c ar-target

ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBC) $(OBJS)

interp.c: Makefile
	echo "/* Force shared libraries to know about the correct library loader */" > interp.c
	echo "#include <features.h>" >> interp.c
	echo "#ifdef __HAVE_ELF__" >> interp.c
	echo "const char __dl_ldso__[] __attribute__ ((section " \
		"(\".interp\"))) =\""$(DYNAMIC_LINKER)"\";" >> interp.c
	echo "#endif" >> interp.c

interp.o: interp.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] interp.c *~ core

