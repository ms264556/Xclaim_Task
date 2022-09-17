# Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2011 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

# Note: This Xtensa configuration is incompatible with the example systems in the following sub-directories:
#       xtsc_lookup_vp
#       xtsc_mmio_vp
#       xtsc_queue_vp
#       xtsc_queue_vp.diamond
#       xtsc_queue_producer_vp
#       xtsc_tx_loader_vp
#       xtsc_wire_vp
#       xtsc_wire_vp.diamond 
#       xtsc_wire_source_vp

include Makefile.include


XTSC_CORE_VP = xtsc_core_vp_peregrine


SUBDIRS =                               \
        $(XTSC_CORE_VP)                 \
        xtsc_arbiter_vp                 \
        xtsc_dma_engine_vp              \
        xtsc_lookup_driver_vp           \
        xtsc_master_vp                  \
        xtsc_memory_vp                  \
        xtsc_queue_consumer_vp          \
        xtsc_router_vp                  \
        xtsc_slave_vp                   \
        xtsc_wire_logic_vp              \


EXAMPLE_SUBDIRS =                       \
        xtsc_arbiter_vp                 \
        xtsc_dma_engine_vp              \
        xtsc_lookup_driver_vp           \
        xtsc_master_vp                  \
        xtsc_memory_vp                  \
        xtsc_queue_consumer_vp          \
        xtsc_router_vp                  \
        xtsc_slave_vp                   \
        xtsc_wire_logic_vp              \



.PHONY: all clean run create_xtsc_core_vp clean_xtsc_core_vp 

default: all

all clean: DIRS = $(SUBDIRS)
run:       DIRS = $(EXAMPLE_SUBDIRS)

all clean run:
	for dir in .. $(DIRS); do \
	  if [ x$$dir != x.. ]; then \
	    if [ -d $$dir ]; then \
	      if [ $@ != test ]; then echo $@ $$dir; fi; \
	      if (cd $$dir && $(MAKE) $@); then \
		true; else exit $$?; fi; \
	    else true; fi; \
	  else true; fi; \
	done

all: create_xtsc_core_vp

clean: clean_xtsc_core_vp

create_xtsc_core_vp: 
	if [ ! -d $(XTSC_CORE_VP) ]; then \
          xt-genvpmodel -name=$(XTSC_CORE_VP) -version=$(SYNOPSYS_VIRTUAL_PROTOTYPING_VERSION) \
                --del=sysio=BInterrupt && \
          cd $(XTSC_CORE_VP) && $(MAKE) ; \
	else true; fi;
        
clean_xtsc_core_vp:
	if [ -d $(XTSC_CORE_VP) ]; then \
	  rm -rf $(XTSC_CORE_VP) ; \
	else true; fi;
        
