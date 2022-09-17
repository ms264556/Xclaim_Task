# Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

COMP_NAME = xtsc_pin2tlm_memory_transactor_sd
TARGET_OBJECTS = target/hello.out

NO_TEST = 1
NO_TIE = 1

EXAMPLE_NAME = $(COMP_NAME)
TARGET_SOURCE_PATH = ../../hello_world

ADDITIONAL_CORE_INTERFACES = -add=inbound=inbound_pif -add=memory=dram0ls0 

include ../Makefile.include
include ../Makefile.common

MAXLIBS += --maxlib $(MAXSIM_HOME)/etc/sc_support/sc_prim_channels.conf 

