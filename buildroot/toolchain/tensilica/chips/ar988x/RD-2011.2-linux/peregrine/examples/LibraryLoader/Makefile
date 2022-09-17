# Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2008 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

XTENSA_TOOLS_ROOT = $(shell xt-run --show-config=xttools)
include $(XTENSA_TOOLS_ROOT)/misc/defs.mk

# Variables common to both types of file
shell = /bin/sh

CC = xt-xcc
CFLAGS = -g -Wall

LIBRARY_SRCS = library.c

# Overlay test code here
LIBRARY_OBJS = $(LIBRARY_SRCS:.c=.o)

fixed_location.lib : $(LIBRARY_OBJS)
	$(CC) $(CFLAGS) -mlsp=./overlaylsp -o $@ $<

packaged_flo.o : fixed_location.lib
	xt-pkg-loadlib -e fixed_location_overlay -o $@ -s stripped_lib.o $^

OVERLAY_TEST_SRCS = overlay_test.c
OVERLAY_TEST_OBJS = $(OVERLAY_TEST_SRCS:.c=.o)

flo_test.exe : packaged_flo.o $(OVERLAY_TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -mlsp=./mainlsp -lloader

test_overlay: flo_test.exe
	xt-run flo_test.exe

# Position Independent Library test code here
PIC_LIBRARY_OBJS = $(LIBRARY_SRCS:.c=.pic.o)

library.pic.o : library.c
	$(CC) $(CFLAGS) -fpic -o $@ -c $^

position_independent.lib : $(PIC_LIBRARY_OBJS)
	$(CC) $(CFLAGS) -mlsp=piload -Wl,--shared-pagesize=128 \
	-Wl,-pie -o $@ $<

packaged_pil.o : position_independent.lib
	xt-pkg-loadlib -p -e pi_library -o $@ $^

PIL_TEST_SRCS = pil_test.c
PIL_TEST_OBJS = $(PIL_TEST_SRCS:.c=.o)

pil_loader.exe : packaged_pil.o $(PIL_TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lloader

test_pil: pil_loader.exe
	xt-run pil_loader.exe

# Generic Targets here

clean:
	$(RM) *.o *.lib *~ a.out *.exe

test: test_pil test_overlay

all: 
	@echo nothing to be done for all
