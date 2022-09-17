# Copyright (c) 2006-2010 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

# For instructions please see file Makefile.include

DIRS =                                  \
        xtsc_arbiter_sd                 \
        xtsc_dma_engine_sd              \
        xtsc_lookup_driver_sd           \
        xtsc_lookup_if_sd_mon           \
        xtsc_lookup_sd                  \
        xtsc_master_sd                  \
        xtsc_memory_sd                  \
        xtsc_memory_pin_sd              \
        xtsc_pin2tlm_memory_transactor_sd \
        xtsc_queue_consumer_sd          \
        xtsc_queue_sd                   \
        xtsc_queue_pop_if_sd_mon        \
        xtsc_queue_producer_sd          \
        xtsc_queue_push_if_sd_mon       \
        xtsc_request_if_sd_mon          \
        xtsc_respond_if_sd_mon          \
        xtsc_router_sd                  \
        xtsc_sc_signal_sc_bv_base_sd    \
        xtsc_sc_signal_sc_uint_base_sd  \
        xtsc_slave_sd                   \
        xtsc_tlm2pin_memory_transactor_sd \
        xtsc_tx_xfer_if_sd_mon          \
        xtsc_tx_loader_sd               \
        xtsc_wire_sd                    \
        xtsc_wire_read_if_sd_mon        \
        xtsc_wire_source_sd             \
        xtsc_wire_write_if_sd_mon       \


ifdef AMBA2_HOME
DIRS +=                                 \
        xtsc_ahb2pif_bridge_sd          \
        xtsc_ahb_master_sd              \
        xtsc_ahb_memory_sd              \
        xtsc_ahb_translator_sd          \
        xtsc_pif2ahb_bridge_sd
endif

DIRS_all = $(DIRS:=_all)
DIRS_clean = $(DIRS:=_clean)

.PHONY: all clean 
.PHONY: $(DIRS_all) $(DIRS_clean)

default: all

all: $(DIRS_all)
$(DIRS_all):
	$(MAKE) -C $(@:_all=) all

clean: $(DIRS_clean)
$(DIRS_clean):
	$(MAKE) -C $(@:_clean=) clean

