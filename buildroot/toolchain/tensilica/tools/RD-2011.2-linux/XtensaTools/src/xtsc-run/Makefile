# Copyright (c) 2006-2010 Tensilica Inc.  ALL RIGHTS RESERVED.
# 
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

EXAMPLE_NAME = xtsc-run$(EXE)
HOST_OBJECTS = xtsc-run.$(OBJ)
EXAMPLE_DIR = $(CURDIR)

# NOTE: Ensure the XTTOOLS macro points from this directory to the XtensaTools directory
XTTOOLS = ../..
SYSTEMC_INC = $(XTTOOLS)/Tools/systemc/include
SYSTEMC_LIB = $(XTTOOLS)/Tools/systemc/lib-linux
ISSLIBDIR = iss

XTSCLIBDIR = $(ISSLIBDIR)
SYSTEMC_LIBS = -L $(SYSTEMC_LIB) -l systemc
ADDITIONAL_DEFINES =

# If you have a customized version of the XTSC component library 
# (libxtsc_comp.a on Linux, xtsc_comp.lib/xtsc_compd.lib on MS Windows),
# then uncomment and edit the following line to point to its location
# (for more information about building a customized XTSC component
# library see file ../xtsc/README.txt):
# XTSC_COMPONENT_LIBRARY_PATH = PutPathHere

ifdef XTSC_COMPONENT_LIBRARY_PATH
XTSC_COMP_INC = -I $(XTSC_COMPONENT_LIBRARY_PATH)
XTSC_COMP_LIB = -L $(XTSC_COMPONENT_LIBRARY_PATH)
endif

# The -libdebug flag makes TIE wires visible to xt-gdb and Xplorer
# when running the Xtensa Instruction Set Simulator; however, it
# can slow down simulation speed.  Comment out the following line
# to disable TIE wire visibility.
TCFLAGS = -libdebug

CXX = g++
LINK = g++
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
	-I $(XTTOOLS)/include \
	-I $(SYSTEMC_INC) 

XTSC_LIBS = \
	$(XTSC_COMP_LIB) \
	-L $(XTTOOLS)/lib/$(XTSCLIBDIR) \
	-l xtsc \
	-l xtsc_comp \
	-L $(XTTOOLS)/lib/$(ISSLIBDIR) \
	-l xtmp \
	-l simxtcore \
	-l log4xtensa \
	-L $(XTTOOLS)/lib \
	-l xtparams

OTHER_LIBS = -Wl,-rpath,$(XTTOOLS)/lib/$(ISSLIBDIR):$(XTTOOLS)/lib
D = -D

LIBS = $(XTSC_LIBS) $(SYSTEMC_LIBS) $(OTHER_LIBS)
DEFINES = \
        $(ADDITIONAL_DEFINES)

CXXFLAGS = $(COMMON_CXXFLAGS) $(DEFINES) $(INCLUDES)

HOST_SOURCES = $(HOST_OBJECTS:.o=.cpp)




.PHONY: all clean
BASE_NAME := $(EXAMPLE_NAME)


all:

$(EXAMPLE_NAME): $(HOST_OBJECTS)
	$(LINK) $(LDFLAGS) $(LDOUT)$@ $(HOST_OBJECTS) $(LIBS)

%.$(OBJ): %.cpp
	$(CXX) $(CXXFLAGS) $<

all: $(EXAMPLE_NAME)

clean:
	-$(RM) *.$(OBJ)
	-$(RM) $(EXAMPLE_NAME)
	-$(RM) *.log




