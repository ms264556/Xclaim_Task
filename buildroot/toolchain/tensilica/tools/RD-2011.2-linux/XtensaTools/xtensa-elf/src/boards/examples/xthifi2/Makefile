# Copyright (c) 2008 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.
#
# Build examples in this dir (which are single C files) for the XT-HiFi2 board.
#   make ... [<example>]
# XTENSA_CORE is required for Xtensa tools (usually set in your environment)
#   unless your tools installation was configured with a default core.
# Other variables may also be overridden via command line (eg. CFLAGS, LSP).

# Board name (do not edit or override).
BOARD	    = xthifi2

# Specify compilation flags (edit this or override it via command line)
CFLAGS	    = -g -O2

# Build Locations, Flags and Commands
# Board support internal to core package (use standard paths)
COMM_XELF   = $(XTENSA_TOOLS_ROOT)/xtensa-elf
IFLAGS      = -I$(COMM_XELF)/include/xtensa/$(BOARD)
LSP	    = $(BOARD)-rt
ifdef XTENSA_CORE
  CORE_FLGS = --xtensa-core=$(XTENSA_CORE)
else
  CORE_FLGS =
endif
VPATH	    = $(COMM_XELF)/src/boards/examples/$(BOARD)
CC	    = xt-xcc $(CORE_FLGS)
RM	    = rm -f

# Fallback to sources internal to Tensilica (for non-released sources)
ifdef INTERN_SRC
IFLAGS	    += -I$(INTERN_SRC)/include/xtensa/$(BOARD)
VPATH	    += $(INTERN_SRC)/boards/$(BOARD)
endif

# Board-Specific examples
EXAMPLES    =  memtest ethtest flasher restore sinewave hdmi_sinewave soundclip

# Targets and Pattern Rules
.PHONY: all clean

all:	$(EXAMPLES)

clean:
	$(RM) $(EXAMPLES)

% : %.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $@ $< -mlsp=$(LSP)

