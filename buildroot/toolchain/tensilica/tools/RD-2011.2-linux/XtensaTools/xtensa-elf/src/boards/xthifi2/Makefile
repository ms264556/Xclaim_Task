# Build the board-support library for the XT-HiFi2 board.
#
# Makefile - In board-specific source directory of core and board packages.
#
# Copyright (c) 2008 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.
#
# See ../Makefile.inc for instructions on how to use this Makefile.

# This board.
BOARD = xthifi2

# The objects to be included in the library.
MODS =	xthifi2-sx-spi xthifi2-sx-flash xthifi2-sx-twi xthifi2-sx-gpio \
	xthifi2-sx-globalreg xthifi2-sx-dmac xthifi2-sx-i2s \
	xthifi2-sx-ppi xthifi2-sx-mii xthifi2-sx-enet-mac xthifi2-sx-enet-phy \
	xthifi2-bus xthifi2-board xthifi2-display xthifi2-aic xthifi2-hdmi \
	xtbsp-gloss-uart xtbsp-uart-16550 uart-16550 uart-16550-sysclk

# The knowledge of how to build it is board-generic and included here.
ifdef XTENSA_BOARDS
MAKEINC = $(XTENSA_BOARDS)/common/xtensa-elf/src/boards
else
MAKEINC = $(XTENSA_TOOLS_ROOT)/xtensa-elf/src/boards
endif
include $(MAKEINC)/Makefile.inc

