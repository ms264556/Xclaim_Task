# Build the board-support library for the XT-AV110 board.
#
# Makefile - In board-specific source directory of core and board packages.
#
# Copyright (c) 2009-2010 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.
#
# See ../Makefile.inc for instructions on how to use this Makefile.

# This board.
BOARD = xtav110

# The objects to be included in the library.
MODS =	xtav110-board xtav110-display lcd-splc780d-4bitmode \
	xtbsp-gloss-uart xtbsp-uart-16550 uart-16550 

# The knowledge of how to build it is board-generic and included here.
ifdef XTENSA_BOARDS
MAKEINC = $(XTENSA_BOARDS)/common/xtensa-elf/src/boards
else
MAKEINC = $(XTENSA_TOOLS_ROOT)/xtensa-elf/src/boards
endif
include $(MAKEINC)/Makefile.inc

