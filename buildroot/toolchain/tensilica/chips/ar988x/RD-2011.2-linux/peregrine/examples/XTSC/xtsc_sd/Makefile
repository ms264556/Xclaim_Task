# Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2010 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

# Note: This Xtensa configuration is incompatible with the example systems in the following sub-directories:
#       xtsc_lookup_sd
#       xtsc_mmio_sd
#       xtsc_queue_sd
#       xtsc_queue_sd.diamond
#       xtsc_queue_producer_sd
#       xtsc_tx_loader_sd
#       xtsc_wire_sd
#       xtsc_wire_sd.diamond 
#       xtsc_wire_source_sd

include Makefile.include


XTSC_CORE_SD = xtsc_core_sd_peregrine


SUBDIRS =                               \
        $(XTSC_CORE_SD)                 \
        xtsc_pin2tlm_memory_transactor_sd \
        xtsc_arbiter_sd                 \
        xtsc_dma_engine_sd              \
        xtsc_lookup_driver_sd           \
        xtsc_master_sd                  \
        xtsc_memory_sd                  \
        xtsc_tlm2pin_memory_transactor_sd \
        xtsc_queue_consumer_sd          \
        xtsc_router_sd                  \
        xtsc_slave_sd                   \
        xtsc_wire_logic_sd              \


EXAMPLE_SUBDIRS =                       \
        xtsc_pin2tlm_memory_transactor_sd \
        xtsc_arbiter_sd                 \
        xtsc_dma_engine_sd              \
        xtsc_lookup_driver_sd           \
        xtsc_master_sd                  \
        xtsc_memory_sd                  \
        xtsc_tlm2pin_memory_transactor_sd \
        xtsc_queue_consumer_sd          \
        xtsc_router_sd                  \
        xtsc_slave_sd                   \
        xtsc_wire_logic_sd              \



ifdef AMBA2_HOME

SUBDIRS +=                              \
        xtsc_ahb2pif_bridge_sd          \
        xtsc_ahb_master_sd              \
        xtsc_ahb_memory_sd              \
        xtsc_ahb_translator_sd          \
        xtsc_pif2ahb_bridge_sd          \


EXAMPLE_SUBDIRS +=                      \
        xtsc_ahb2pif_bridge_sd          \
        xtsc_ahb_master_sd              \
        xtsc_ahb_memory_sd              \
        xtsc_ahb_translator_sd          \
        xtsc_pif2ahb_bridge_sd          \

endif


.PHONY: all clean run create_xtsc_core_sd clean_xtsc_core_sd 

default: all

all clean: DIRS = $(SUBDIRS)
run:       DIRS = $(EXAMPLE_SUBDIRS)

all clean run:
	for dir in .. $(DIRS); do \
	  if [ x$$dir != x.. ]; then \
	    if [ -d $$dir ]; then \
	      if (cd $$dir && $(MAKE) $@); then \
		true; else exit $$?; fi; \
	    else true; fi; \
	  else true; fi; \
	done

all: create_xtsc_core_sd

clean: clean_xtsc_core_sd

create_xtsc_core_sd: 
	if [ ! -d $(XTSC_CORE_SD) ]; then \
          xt-gensdmodel -name=$(XTSC_CORE_SD) -version=$(SOCDESIGNER_VERSION_NUMBER) \
                --del=sysio=BInterrupt && \
          cd $(XTSC_CORE_SD) && $(MAKE) ; \
	else true; fi;
        
clean_xtsc_core_sd:
	if [ -d $(XTSC_CORE_SD) ]; then \
	  rm -rf $(XTSC_CORE_SD) ; \
	else true; fi;
        
