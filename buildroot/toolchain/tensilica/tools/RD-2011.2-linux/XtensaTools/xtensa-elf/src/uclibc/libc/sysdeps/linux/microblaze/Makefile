# Makefile for uClibc
#
# Copyright (C) 2001,2002  NEC Corporation
# Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
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

TOPDIR=../../../../
include $(TOPDIR)Rules.mak

#FIXME -- this arch should include its own crti.S and crtn.S
UCLIBC_CTOR_DTOR=n

CFLAGS+=-I..
ASFLAGS+=-I.. -D__ASSEMBLER -DASM_GLOBAL_DIRECTIVE=.globl

TARGET_MACHINE_TYPE=$(shell $(CC) -dumpmachine)

CRT0_SRC = crt0.S
CRT0_OBJ = crt0.o crt1.o
CTOR_TARGETS=$(TOPDIR)lib/crti.o $(TOPDIR)lib/crtn.o

SSRC = setjmp.S __longjmp.S vfork.S
SOBJS = $(patsubst %.S,%.o, $(SSRC))

CSRC = mmap.c syscall.c clone.c
COBJS = $(patsubst %.c,%.o, $(CSRC))

OBJS = $(SOBJS) $(COBJS)

all: $(OBJS) $(LIBC)

$(LIBC): ar-target

ar-target: $(OBJS) $(CRT0_OBJ) $(CTOR_TARGETS)
	$(AR) $(ARFLAGS) $(LIBC) $(OBJS)
	cp $(CRT0_OBJ) $(TOPDIR)lib/

$(CRT0_OBJ): $(CRT0_SRC)
	$(CC) $(ASFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(SOBJS): %.o : %.S
	$(CC) $(ASFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

ifeq ($(strip $(UCLIBC_CTOR_DTOR)),y)
crti.o: crti.S
	$(CC) $(ASFLAGS) -c crti.S -o crti.o

$(TOPDIR)lib/crti.o: crti.o
	$(INSTALL) -d $(TOPDIR)lib/
	cp crti.o $(TOPDIR)lib/

crtn.o: crtn.S
	$(CC) $(ASFLAGS) -c crtn.S -o crtn.o

$(TOPDIR)lib/crtn.o: crtn.o
	$(INSTALL) -d $(TOPDIR)lib/
	cp crtn.o $(TOPDIR)lib/
else
$(TOPDIR)lib/crti.o:
	$(INSTALL) -d $(TOPDIR)lib/
	$(AR) $(ARFLAGS) $(TOPDIR)lib/crti.o
$(TOPDIR)lib/crtn.o:
	$(INSTALL) -d $(TOPDIR)lib/
	$(AR) $(ARFLAGS) $(TOPDIR)lib/crtn.o
endif


headers:

clean:
	$(RM) *.[oa] *~ core
	$(RM) bits/sysnum.h

