# Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

COMP_NAME = xtsc_wire_source_sd
TARGET_OBJECTS = idle.out


EXAMPLE_NAME = $(subst _sd,,$(COMP_NAME))
TARGET_SOURCE_PATH = ../../xtsc-run/$(EXAMPLE_NAME)

ADDITIONAL_CORE_INTERFACES = -add=sysio=BReset

NO_TIE = 1

include ../Makefile.include
include ../Makefile.common

TIMEOUT = 240
