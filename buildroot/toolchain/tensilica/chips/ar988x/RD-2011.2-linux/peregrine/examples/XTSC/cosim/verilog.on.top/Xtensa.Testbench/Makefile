# Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

EXAMPLE_NAME = Xtensa.Testbench
TARGET_OBJECTS = target/memory_test.out
NO_TIE = 1

TARGET_SOURCE_PATH = ../../../xtsc_tlm2pin_memory_transactor

include ../Makefile.include
include ../Makefile.common

ifdef XTSC_RUN_COSIM_USE_DSO
.PHONY: dso dso_clean

xtsc-run: dso

clean: dso_clean

dso:
	$(MAKE) -f ../../dso/Makefile VENDOR=$(XTSC_RUN_COSIM_VENDOR)

dso_clean:
	$(MAKE) -f ../../dso/Makefile clean VENDOR=$(XTSC_RUN_COSIM_VENDOR)

endif
