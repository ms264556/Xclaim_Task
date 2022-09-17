# Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2007 Tensilica Inc.  ALL RIGHTS RESERVED.
# 
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.


EXAMPLE_NAME = hello_world.standalone$(EXE)
HOST_OBJECTS = sc_main.$(OBJ)
TARGET_OBJECTS = target/hello.out
export XTENSA_SYSTEM = /home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/chips/ar988x/RD-2011.2-linux/peregrine/config
export XTENSA_CORE = peregrine

# Supply the name of your TIE file here, or leave blank if you don't have
# a TIE file:
TIE_FILE = 

CXX = g++

# Three variants of XTSC/ISS libraries are provided inside XTTOOLS:
# "iss"         directory contains libraries compatible with gcc 3.4.X
# "iss-GCC-4.1" directory contains libraries compatible with gcc 4.1.X
# "iss-RHEL3"   directory contains *deprecated* libraries compatible with
#               the RHEL3 default compiler (gcc 3.2.3)
ifneq ($(findstring (GCC) 4.1.,$(shell $(CXX) --version)),)
LIB_VARIANT = -GCC-4.1
endif
ifneq ($(findstring (GCC) 3.2.3,$(shell $(CXX) --version)),)
LIB_VARIANT = -RHEL3
endif

ISSLIBDIR = iss$(LIB_VARIANT)

# NOTE: If desired, you can use the original OSCI SystemC 2.2 (available
# from www.systemc.org); however, you will lose the benefits of Tensilica's
# modifications (see "Getting Started" in the XTSC User's Guide).
# To use the original OSCI SystemC 2.2, change the following two defines
# to point to its locations:
SYSTEMC_INC = /home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/tools/RD-2011.2-linux/XtensaTools/Tools/systemc/include
SYSTEMC_LIB = /home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/tools/RD-2011.2-linux/XtensaTools/Tools/systemc/lib-linux$(LIB_VARIANT)
SYSTEMC_LIBS = -L$(SYSTEMC_LIB) -lsystemc
XTSCLIBDIR = $(ISSLIBDIR)
ADDITIONAL_DEFINES =

# If you have a customized version of the XTSC component library 
# (libxtsc_comp.a on Linux, xtsc_comp.lib/xtsc_compd.lib on MS Windows),
# then uncomment and edit the following line to point to its location
# (for more information about building a customized XTSC component
# library see file /home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/tools/RD-2011.2-linux/XtensaTools/src/xtsc/README.txt):
# XTSC_COMPONENT_LIBRARY_PATH = PutPathHere

ifdef XTSC_COMPONENT_LIBRARY_PATH
XTSC_COMP_INC = -I$(XTSC_COMPONENT_LIBRARY_PATH)
XTSC_COMP_LIB = -L$(XTSC_COMPONENT_LIBRARY_PATH)
endif


XTTOOLS = /home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/tools/RD-2011.2-linux/XtensaTools
XTTOOLSB = \.\.\home\customer\tree\RD-2011.2\tools\swtools-x86-linux

ifdef RUNARGS
FWD_RUNARGS=RUNARGS="$(RUNARGS)"
endif

TARGET_CXX = $(XTTOOLS)/bin/xt-xcc
TARGET_CFLAGS  = -g $(CUSTOM_TARGET_CFLAGS)
TARGET_LIBS = -lm

LINK = $(CXX)
DEBUG_CXXFLAGS = -UNDEBUG -DDEBUG -Wall -O0 -g
#DEBUG_CXXFLAGS = -DNDEBUG -UDEBUG -Wall -O3
COMMON_CXXFLAGS = -m32 -c $(DEBUG_CXXFLAGS)
LDFLAGS = -m32
LDOUT = -o 
OBJ = o
EXE = 
RM = rm -f
RMDIR = rm -rf

INCLUDES = \
	$(XTSC_COMP_INC) \
	-I$(XTTOOLS)/include \
	-I$(SYSTEMC_INC) 
XTSC_LIBS = \
	$(XTSC_COMP_LIB) \
	-L$(XTTOOLS)/lib/$(XTSCLIBDIR) \
	-lxtsc \
	-lxtsc_comp \
	-lxtmp \
	-lsimxtcore \
	-llog4xtensa \
	-L$(XTTOOLS)/lib \
	-lxtparams
OTHER_LIBS = -Wl,-rpath,$(XTTOOLS)/lib/$(ISSLIBDIR):$(XTTOOLS)/lib
D = -D

LIBS = $(XTSC_LIBS) $(SYSTEMC_LIBS) $(OTHER_LIBS)
DEFINES = \
	$(D) CONFIG_NAME=\"$(XTENSA_CORE)\" \
	$(D) XTENSA_REGISTRY=\"$(XTENSA_SYSTEM)\" \
        $(ADDITIONAL_DEFINES)

CXXFLAGS = $(INCLUDES) $(DEFINES) $(COMMON_CXXFLAGS)



.PHONY: all custom_target target run clean xtsc-run
BASE_NAME := $(EXAMPLE_NAME)


all: target

custom_target:

target: $(TARGET_OBJECTS) custom_target 

$(EXAMPLE_NAME): $(HOST_OBJECTS)
	$(LINK) $(LDFLAGS) $(LDOUT)$@ $(HOST_OBJECTS) $(LIBS)

%.out: %.c 
	$(TARGET_CXX) $(TARGET_CFLAGS) -o $@ $< $(TARGET_LIBS)

%.$(OBJ): %.cpp
	$(CXX) $(CXXFLAGS) $<

all: $(EXAMPLE_NAME)

run: $(EXAMPLE_NAME)
	./$(EXAMPLE_NAME) $(RUNARGS)

clean:
	-$(RM) target/*.out *.$(OBJ)
	-$(RM) $(EXAMPLE_NAME)
	-$(RM) *.log


TDK_DIR = $(TIE_FILE:.tie=.tdk)

$(TARGET_OBJECTS): $(TDK_DIR)

TCFLAGS = -libdebug
%.tdk: %.tie
	$(XTTOOLS)/bin/tc $(TCFLAGS)  -d $@ $<       

TARGET_CFLAGS += --xtensa-params="$(TDK_DIR)"
DEFINES += \
        $(D) TDK_DIR=\"$(TDK_DIR)\"


.PHONY: vclean

vclean: clean
	-$(RMDIR) $(TDK_DIR)

xtsc-run:
	xtsc-run -include=$(BASE_NAME).inc

