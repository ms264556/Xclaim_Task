# Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

# Note: This Xtensa configuration is incompatible with the example systems in the following sub-directories:
#       xtsc_lookup_pin
#       xtsc_lookup.ram
#       xtsc_mmio
#       xtsc_queue_pin
#       xtsc_queue.diamond
#       xtsc_wire
#       xtsc_wire.diamond


DIRS =                                  \
        hello_world                     \
        xtsc_pin2tlm_memory_transactor  \
        xtsc_tlm2pin_memory_transactor  \
        Xtensa.Testbench                \


.PHONY: clean all run

default: all

clean all run xtsc-run:
	for dir in .. $(DIRS); do \
	  if [ x$$dir != x.. ]; then \
	    if [ -d $$dir ]; then \
	      if (cd $$dir && $(MAKE) $@); then \
		true; else exit $$?; fi; \
	    else true; fi; \
	  else true; fi; \
	done

all:

clean:

tie:
	cd .. && $(MAKE) tie

