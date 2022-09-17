#  rules.mk  --  Default makefile rules
#
# Copyright (c) 2001-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.
#
#  Assumes that defs.mk was already included.


#  Default target, if nothing else (including Makefile typically defines its own):
all:

.PHONY: all populate install clean

#  Default to doing nothing for general targets
#  (ie. avoid errors if a Makefile does not specify some of them):
all populate install clean:


#  If the including Makefile defines SUBDIRS, provide rules
#  to recursively invoke make in the subdirectories specified.
#
ifdef SUBDIRS

#  Depth-first traversal of subdirectories:

populate: subdirs-populate
all:      subdirs-all
install:  subdirs-install
clean:    subdirs-clean

#  Eg.  subdirs-all: subdir-foo--all subdir-bar--all (for subdirs foo and bar)
SUBDIRS_populate ?= $(SUBDIRS)
SUBDIRS_all      ?= $(SUBDIRS)
SUBDIRS_install  ?= $(SUBDIRS)
SUBDIRS_clean    ?= $(SUBDIRS)
.PHONY: subdirs-all subdirs-populate subdirs-install subdirs-clean
subdirs-populate: $(SUBDIRS_populate:%=subdir-%--populate)
subdirs-all:      $(SUBDIRS_all:%=subdir-%--all)
subdirs-install:  $(SUBDIRS_install:%=subdir-%--install)
subdirs-clean:    $(SUBDIRS_clean:%=subdir-%--clean)

#  Eg.  subdir-foo--all: foo/Makefile
#SUBTARGETS = all populate install clean
#ALLSUBTARGETS := $(foreach subdir,$(SUBDIRS),$(SUBTARGETS:%=subdir-$(subdir)--%))
ALLSUBTARGETS := $(foreach t,all populate install clean,$(SUBDIRS:%=subdir-%--$t))
.PHONY: $(ALLSUBTARGETS)
#$(SUBDIRS:%=subdir-%--populate): subdir-%--populate: %/Makefile
#$(SUBDIRS:%=subdir-%--all):      subdir-%--all:      %/Makefile
#$(SUBDIRS:%=subdir-%--install):  subdir-%--install:  %/Makefile
#$(SUBDIRS:%=subdir-%--clean):    subdir-%--clean:    %/Makefile

#  How to apply general targets (all, install, etc.) to subdirectories:
$(SUBDIRS:%=subdir-%--populate): subdir-%--populate: %/Makefile
	$(MAKE) $(SUBMAKEFLAGS) -C $* populate
$(SUBDIRS:%=subdir-%--all): subdir-%--all: %/Makefile
	$(MAKE) $(SUBMAKEFLAGS) -C $* all
$(SUBDIRS:%=subdir-%--install): subdir-%--install: %/Makefile
	$(MAKE) $(SUBMAKEFLAGS) -C $* install
#  When cleaning, don't stop because of subdirectory errors (ignore them):
$(SUBDIRS:%=subdir-%--clean): subdir-%--clean: %/Makefile
	-$(MAKE) $(SUBMAKEFLAGS) -C $* clean

#  How to create a subdirectory's Makefile:
$(SUBMAKEFILES): %/Makefile: Makefile
	@echo "Creating $@"
	-@$(call mkpath,$*)
	-@$(call rm,$@)
	@$(call addline,$@,# Automatically generated on $(DATE):)
ifdef SET_MAKEFILE_INFO
	@$(call addline,$@,$(SET_MAKEFILE_INFO))
endif
ifdef MAKEFILE_INFO
	@$(call addline,$@,MAKEFILE_INFO = $(call catpath,$(call revpath,$*),$(MAKEFILE_INFO)))
	@$(call addline,$@,include $$(MAKEFILE_INFO))
endif
	@$(call addline,$@,MAKEFILE_SRC = $(foreach subdir,$*,$(SUBMAKEFILE_SRC)))
	@$(call addline,$@,include $$(MAKEFILE_SRC))

endif

