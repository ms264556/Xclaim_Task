# Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

COMP_NAME = xtsc_mmio_vp
TARGET_OBJECTS = target/core0.out target/core1.out

EXAMPLE_NAME = $(subst _vp,,$(COMP_NAME))
TARGET_SOURCE_PATH = ../../$(EXAMPLE_NAME)

include ../Makefile.include
include ../Makefile.common

ADDITIONAL_CORE_INTERFACES = -add=tie=EXPSTATE -add=sysin=BInterruptXX

.PHONY: create_xtsc_mmio_vp clean_xtsc_mmio_vp 

MMIO_XML = xtsc_mmio_vp/xtsc_mmio_vp.xml
XML_LIBRARIES = $(CORE_XML) $(MMIO_XML)

all: create_xtsc_mmio_vp
clean: clean_xtsc_mmio_vp

create_xtsc_mmio_vp: $(MMIO_XML)

$(MMIO_XML): xtsc_mmio_vp/SRC_DIR/xtsc_mmio_vp.cpp xtsc_mmio_vp/SRC_DIR/xtsc_mmio_vp.h
	cd xtsc_mmio_vp && $(MAKE) xtsc_mmio_vp.xml
        
xtsc_mmio_vp/SRC_DIR/xtsc_mmio_vp.cpp:
	-$(RMDIR) xtsc_mmio_vp
	xt-genvpmodel -module=xtsc_mmio \
          -version=$(SYNOPSYS_VIRTUAL_PROTOTYPING_VERSION) \
          -definition_file=../../xtsc_mmio/mmio.txt
	-mkdir SRC_DIR
	cp -p xtsc_mmio_vp/SRC_DIR/xtsc_mmio_vp.def SRC_DIR/.
        
clean_xtsc_mmio_vp:
	-$(RMDIR) xtsc_mmio_vp
	-$(RMDIR) SRC_DIR
        
