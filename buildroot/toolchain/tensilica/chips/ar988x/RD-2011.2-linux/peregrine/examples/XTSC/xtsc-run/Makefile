# Makefile for customer xtsc-run directory build

# Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2008 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

# Note: This Xtensa configuration is incompatible with the following examples:
#       xtsc_queue_producer

DIRS = \
	xtsc_lookup_driver \
	xtsc_queue_consumer \
	xtsc_wire_source \
	xtsc_wire_source.BInterrupt \
	xtsc_master \
	xtsc_slave

DIRS_all = $(DIRS:=_all)
DIRS_target = $(DIRS:=_target)
DIRS_run = $(DIRS:=_run)
DIRS_clean = $(DIRS:=_clean)

include ../Makefile.include

.PHONY: all target clean $(DIRS_all) $(DIRS_clean)

all: $(DIRS_all)
$(DIRS_all):
	$(MAKE) -C $(@:_all=) all


target: $(DIRS_target)
$(DIRS_target):
	$(MAKE) -C $(@:_target=) target 

run: $(DIRS_run)
$(DIRS_run):
	$(MAKE) -C $(@:_run=) run

clean: $(DIRS_clean)
$(DIRS_clean):
	$(MAKE) -C $(@:_clean=) clean

