#  sld-defs.mk  --  Makefile definitions for building systems
#
# Copyright (c) 2005-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

#  The Xtensa Tools are assumed to be on the user's PATH
#  (i.e. $(XTENSA_TOOLS_ROOT)/bin which contains xt-xcc, xt-ld, etc).
#  However, $(XTENSA_STANDARD_TOOLS)/bin are not assumed on the PATH.

#  Include $(XTENSA_TOOLS_ROOT)/misc/defs.mk if not done already:
ifndef SUBMAKEFILES
include $(XTENSA_TOOLS_ROOT)/misc/defs.mk
endif

SOCFILE = $(XTENSA_SYSTEM_ROOT)/config/system.xsysd

SYS_TEMPLATE_DIR = $(XTENSA_TOOLS_ROOT)/xcl/templates
SYS_TEMPLATES = $(SYS_TEMPLATE_DIR)/PredefinedInterfaces.xsyst \
		$(SYS_TEMPLATE_DIR)/PredefinedComponents.xsyst

GENMPHAL = xt-genmphal
SYSGEN   = $(XTENSA_TOOLS_ROOT)/xcl/sysgen -swtools $(XTENSA_TOOLS_ROOT) \
		-tctools $(XTENSA_TIE_TOOLS) -tools $(XTENSA_STANDARD_TOOLS)
SLD      = $(XTENSA_TOOLS_ROOT)/xcl/sld --xtensa-system=$(XTENSA_SYSTEM) $(SYS_TEMPLATES)
XPLODE   = $(XTENSA_TOOLS_ROOT)/xcl/xplode
XPLAT    = $(XTENSA_TOOLS_ROOT)/xcl/xplat
XCLTPP   = $(XTENSA_TOOLS_ROOT)/xcl/tpp

#XT_XCC   = xt-xcc \
#		-isystem $(XTENSA_SYSTEM_ROOT)/xtensa-elf/include \
#		-L$(XTENSA_SYSTEM_ROOT)/xtensa-elf/config/$(XTENSA_CORE)/lib

