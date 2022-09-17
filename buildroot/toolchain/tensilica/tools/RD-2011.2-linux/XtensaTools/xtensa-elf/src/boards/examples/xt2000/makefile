# Copyright (c) 2002-2004 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

all: ledcount-redboot ledcount-ocd

CFLAGS = -g -save-temps

#  This image is for downloading using GDB with RedBoot
#  as the debug agent:
#
ledcount-redboot: ledcount.c 
	xt-xcc $(CFLAGS) -o $@ -mlsp=xt2000    $^

#  This image is for downloading using GDB over OCD.
#  It requires the ROM area to be writable, which can be done
#  on the XT2000 by configuring its boot-select dipswitch.
#
ledcount-ocd: ledcount.c 
	xt-xcc $(CFLAGS) -o $@ -mlsp=xt2000-rt $^

#  This image is for burning into a ROM, or downloading (eg. using OCD)
#  into the XT2000's ROM area configured (with dipswitches) as static RAM.
#  This ROM image won't build if a system ROM was not configured,
#  so it is not built by default.  Use "make ledcount-rom" to build it:
#
ledcount-rom: ledcount.c 
	xt-xcc $(CFLAGS) -o $@ -mlsp=xt2000-rt-rom $^

