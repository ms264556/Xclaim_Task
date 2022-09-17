# Makefile for XTSC examples.

# Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

# Note: This Xtensa configuration is incompatible with the following examples:
#       xtsc_mmio
#       xtsc_wire
#       xtsc_wire.diamond 
#       tie_pin
#       sc_fifo
#       template_queue_pop
#       template_queue_push
#       xtsc_queue
#       xtsc_queue.diamond
#       xtsc_queue_pin
#       xtsc_tx_loader
#       xtsc_lookup
#       xtsc_lookup.ram
#       xtsc_lookup_pin


PRELIM_DIRS = \
	hello_world \
	xtsc_pin2tlm_memory_transactor \
	xtsc_dma_engine \
	pif2sb_bridge \
	simple_memory \
	template_memory \
	xtsc_arbiter \
	xtsc_memory \
	xtsc_tlm2pin_memory_transactor \
	xtsc_memory_trace \
	xtsc_router \
	xtsc_wire_logic \
	xtsc-run

DIRS = $(PRELIM_DIRS)
DIRS_all = $(DIRS:=_all)
DIRS_target = $(DIRS:=_target)
DIRS_run = $(DIRS:=_run)
DIRS_clean = $(DIRS:=_clean)

include Makefile.include

.PHONY: all target tie run clean xmp tie_clean vclean
.PHONY: $(DIRS_all) $(DIRS_run) $(DIRS_clean)


all: $(DIRS_all)
$(DIRS_all):
	$(MAKE) -C $(@:_all=) all $(FWD_RUNARGS)

target: $(DIRS_target)
$(DIRS_target):
	$(MAKE) -C $(@:_target=) target $(FWD_RUNARGS)

run: $(DIRS_run)
$(DIRS_run):
	$(MAKE) -C $(@:_run=) run $(FWD_RUNARGS)

tie:
	@echo "The example TIE is not compatible with this Xtensa processor."
	@echo "Unable to continue building this example."
	@exit 1

xmp: 
	$(MAKE) -C XMP

clean: $(DIRS_clean)


tie_clean:
	-$(RMDIR) TIE/example.tdk


vclean: clean tie_clean
	-$(MAKE) -C XMP clean


$(DIRS_clean):
	$(MAKE) -C $(@:_clean=) clean


dir_clean: $(DIRS_clean)


