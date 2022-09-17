# Makefile for building xtsc_comp library
#
# Copyright (c) 2006-2010 Tensilica Inc.  ALL RIGHTS RESERVED.
# 
# These coded instructions, statements, and computer programs are
# the copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but no adapted or modified version
# may be disclosed or distributed to third parties in any manner,
# medium, or form, in whole or in part, without the prior written
# consent of Tensilica Inc.


# Instructions: Before using this make file:
#   -- Ensure the XTTOOLS macro below points to <xtensa_tools_root>, your installation of the Xtensa Tools.
#   -- Select which set of COMMON_CXXFLAGS you want.  The first set is for debugging, the second is for speed.

XTTOOLS = ../..

COMMON_CXXFLAGS = -U NDEBUG -D DEBUG -O0 -g
#COMMON_CXXFLAGS = -D NDEBUG -U DEBUG -O3

COMMON_CXXFLAGS += -Wall -Werror 

# Suppress non-virtual-destructor warning from g++ 4.x.
GCC_VERSION = $(word 3, $(shell g++ --version))
ifeq ($(patsubst 4.%,4,$(GCC_VERSION)),4)
COMMON_CXXFLAGS += -Wno-non-virtual-dtor
endif

INCLUDES = -I . -I $(XTTOOLS)/include -I $(XTTOOLS)/Tools/systemc/include

CXXFLAGS = $(COMMON_CXXFLAGS)

include Makefile.SRCS

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

%.o: %.cpp
	g++ -c -MD $(CXXFLAGS) $(INCLUDES) $<

all: libxtsc_comp.a

libxtsc_comp.a: $(OBJS)
	ar -cr $@ $^

clean:
	-rm -f $(OBJS) $(SRCS:.cpp=.d)
	-rm -f libxtsc_comp.a


