#  defs.mk  --  default set of definitions,
#		including host/environment specific variables
#
# Copyright (c) 2001-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.
#
#  The Makefile that includes this file must set ...
#
#  The Xtensa Tools are assumed to be on the user's PATH
#  (eg. <release>/XtensaTools/bin which contains xt-xcc, xt-ld, etc.)
#  The including makefile typically makes the same assumption
#  and doesn't add the Xtensa tools to the PATH explicitly.

#SHELL = /bin/sh

#  These things are normally already defined by Makefile.info before
#  this file gets included.  In case this file is used in other contexts,
#  set them if not defined:
ifndef XTENSA_TOOLS_ROOT
 #  XTENSA_TOOLS_ROOT is one directory above this makefile's directory.
 #  Get path to this makefile's directory (<xtensa_tools_root>/misc).
 #  This assignment must be made before including any other makefile:
 MAKEFILE_DIR := $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
 XTENSA_TOOLS_ROOT := $(shell cd $(MAKEFILE_DIR).. ; pwd)
endif
ifdef ANYCORE
 #  For internal Tensilica use -- tools can be anywhere:
 XTENSA_STANDARD_TOOLS := $(shell xt-run --xtensa-core=$(ANYCORE) --show-config=tools)
 XTENSA_TIE_TOOLS      := $(shell xt-run --xtensa-core=$(ANYCORE) --show-config=tctools)
endif
XTENSA_SYSTEM         ?= $(XTENSA_TOOLS_ROOT)/config
XTENSA_STANDARD_TOOLS ?= $(XTENSA_TOOLS_ROOT)/Tools
XTENSA_TIE_TOOLS      ?= $(XTENSA_TOOLS_ROOT)/TIE

#  The source directory is the directory containing the source Makefile
#  (note: we preserve variable references for sub-Makefiles):
ifdef MAKEFILE_SRC
 ifndef SRCDIR
  SRCDIR := $(patsubst %/,%,$(dir $(value MAKEFILE_SRC)))
  $(eval SRCDIR = $(SRCDIR))
 endif
endif

VPATH += $(SRCDIR)

#  Miscellaneous utility macros:
#
#  $(call revpath,PATH)
#  Reverse a path:  if PATH is relative, return an equivalent number of ".."
#  to return to the current directory (convert foo/bar to ../..); if PATH
#  is absolute, just return CWD (CWD is set further below):
revpath = $(if $(filter /%,$(1)),$(CWD),$(patsubst %,..,$(subst /, ,$(1))))
#
#  $(call catpath,DIR,PATH)
#  Return DIR/PATH if PATH is relative, or just PATH if PATH is absolute:
catpath = $(if $(filter /%,$(2)),$(2),$(1)/$(2))

#  If SUBDIRS is defined, these definitions help traversing these subdirectories
#  recursively using $(MAKE):
SUBMAKEFILES = $(SUBDIRS:%=%/Makefile)
#
#  Given element $(sdir) of $(SUBDIRS), we have:
#
#	build-side Makefile (tiny, generated):	./$(sdir)/Makefile
#	(e.g. from $(SUBMAKEFILES))
#
#	source Makefile (from sources):		$(foreach subdir,$(sdir),$(SUBMAKEFILE_SRC))
#	which typically expands to:		$(SRCDIR)/$(sdir)/Makefile.src
#	but can be overridden by a variable named MAKEFILE_SRC_$(sdir),
#	for example:   MAKEFILE_SRC_subdir1 = $(SRCDIR)/some/deeper/dir/Makefile.foo
SUBMAKEFILE_SRC = $(firstword $(value MAKEFILE_SRC_$(subdir)) $(value SRCDIR)/$(subdir)/Makefile.src)
SUBMAKEFLAGS =
#  $(MAKEFLAGS) is normally passed down through the environment, separately.

#  Normally, SET_MAKEFILE_INFO is set by some top-level makefile
#  to something like "include /some_path/Makefile.info" .
#  This is used in generated build sub-directory makefiles.
#  Otherwise, at minimum we set XTENSA_TOOLS_ROOT instead.
ifndef MAKEFILE_INFO
 ifndef SET_MAKEFILE_INFO
  SET_MAKEFILE_INFO := XTENSA_TOOLS_ROOT := $(XTENSA_TOOLS_ROOT)
 endif
endif

#  MAKEFILES is evil, don't use it.  It is the perfect Makefile obfuscator.
#  Causes endless confusion as to how Makefiles actually work.
#  Explicitly including makefile fragments is the way to go...
#  So here we make sure it has an empty value.
#  (Environment variables are also unhealthy as a mechanism for passing
#  things to sub Makefiles [except for MAKEFLAGS which is temporal],
#  but they're not as easily wiped out.  Just be careful not to use them.)
override MAKEFILES :=

#  Get ARCH_OSTYPE, etc:
include $(XTENSA_TOOLS_ROOT)/misc/version.mk


#######  Host environment specific definitions  #######

ifeq ($(ARCH_OSTYPE),unix)
ARCH_OSLIKE = unix
PERLBINDIR = $(XTENSA_STANDARD_TOOLS)/bin
export PERL5LIB := $(shell $(XTENSA_STANDARD_TOOLS)/bin/perl -e 'map s|^.*(/lib/perl5/.*)|$(XTENSA_STANDARD_TOOLS)$$1|, @INC; print join(":",@INC)."\n";')
endif

ifeq ($(ARCH_OSTYPE),win)
ifeq ($(SHELL:%.exe=),/bin/sh)
#  Looks like a cygwin make, using a bourne shell, use Unix-style commands:
ARCH_OSLIKE = unix
else
#  Looks like a native make, assume a native Windows NT or XP command shell:
RM	= del /f/q
RM_R	= rmdir /s/q
# xt-make uses this:
#RM_R	= cmd /c rd /s/q
CP	= copy
MKPATH	= mkdir
CAT     = type
# This one gives forward slashes (and assumes cygwin):
#CWD	= $(shell pwd)
CWD	= $(shell cd)
DATE    = $(shell date /t) $(shell time /t)
#  $E must be empty, so that $S is a backslash only (neither \ nor \\ by itself works):
E=
S       = \$E
C	= rem ---
#  For use with $(call <func>,args...):
rm      = $(RM)     $(subst /,\,$(1) $(2) $(3) $(4) $(5) $(6) $(7) $(8) $(9))
rm_r    = $(RM_R)   $(subst /,\,$(1) $(2) $(3) $(4) $(5) $(6) $(7) $(8) $(9))
cp      = $(CP)     $(subst /,\,$(1) $(2) $(3) $(4) $(5) $(6) $(7) $(8) $(9))
#cp      = $(CP)     $(subst /,\,$(1) $(2))
mkpath  = $(MKPATH) $(subst /,\,$(1))
cat     = $(CAT)    $(subst /,\,$(1))
addline = echo $(2) >> $(subst \,/,$(1))
endif
PERLBINDIR = $(XTENSA_STANDARD_TOOLS)/perl/bin/MSWin32-x86
endif

ifeq ($(ARCH_OSLIKE),unix)
RM	= rm -f
RM_R	= rm -rf
CP	= cp -fp
MKPATH	= mkdir -p
CAT     = cat
CWD	= $(shell pwd)
DATE    = $(shell date)
#  Path separator, and comment marker:
S       = /
C	= \#
#  For use with $(call <func>,args...):
rm      = $(RM)     $(subst \,/,$(1) $(2) $(3) $(4) $(5) $(6) $(7) $(8) $(9))
rm_r    = $(RM_R)   $(subst \,/,$(1) $(2) $(3) $(4) $(5) $(6) $(7) $(8) $(9))
cp      = $(CP)     $(subst \,/,$(1) $(2) $(3) $(4) $(5) $(6) $(7) $(8) $(9))
#cp      = $(CP)     $(subst \,/,$(1) $(2))
mkpath  = $(MKPATH) $(subst \,/,$(1))
cat     = $(CAT)    $(subst \,/,$(1))
addline = echo '$(2)' >> $(subst \,/,$(1))
endif

#  Paranoia:
ifndef CAT
  $(error Unrecognized or undefined ARCH_OSTYPE ($(ARCH_OSTYPE)), must be unix or win.)
endif


#######  Miscellaneous definitions  #######

#  Newline character (there must be *exactly* two empty lines between define and endef !):
define N


endef

#  The Xtensa Tools are assumed to be on the user's PATH
#  (i.e. $(XTENSA_TOOLS_ROOT)/bin which contains xt-xcc, xt-ld, etc).
#  However, $(XTENSA_STANDARD_TOOLS)/bin is not assumed on the PATH.

PERL	 = $(PERLBINDIR)/perl -w
HOST_CC  = $(XTENSA_STANDARD_TOOLS)/bin/gcc
HOST_CXX = $(XTENSA_STANDARD_TOOLS)/bin/g++
CC  = $(HOST_CC)
CXX = $(HOST_CC)
       
CC_FOR_TARGET	?= xt-xcc
CXX_FOR_TARGET	?= xt-xc++
LD_FOR_TARGET	= xt-ld
AR_FOR_TARGET	= xt-ar

