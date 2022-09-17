# Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

ifndef AMBA2_HOME
$(error You must have the AMBA2_HOME environment variable defined in order to run this example)
endif

COMP_NAME = xtsc_ahb2pif_bridge_sd
TARGET_OBJECTS = target/hello.out

NO_TEST = 1
NO_TIE = 1

EXAMPLE_NAME = $(COMP_NAME)
TARGET_SOURCE_PATH = ../../hello_world

ADDITIONAL_CORE_INTERFACES = -add=inbound=inbound_pif -add=memory=dram0ls0 

include ../Makefile.include
include ../Makefile.common

