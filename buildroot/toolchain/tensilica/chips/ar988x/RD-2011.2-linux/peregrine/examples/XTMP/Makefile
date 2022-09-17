# Makefile for building XTMP examples 

# Customer ID=8327; Build=0x3b95c; Copyright (c) 2003-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
# 
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.


#==================================================================#
# BEGIN:                                                           #
# VARIABLES YOU MAY WANT TO CHANGE BEFORE USING THIS MAKEFILE      #
#==================================================================#

# Xtensa Tools location <xtensa_tools_root>
XTTOOLS = /home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/tools/RD-2011.2-linux/XtensaTools
XTTOOLS_TOOLS = /home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/tools/RD-2011.2-linux/XtensaTools/Tools

# Xtensa core registry
ifndef XTENSA_SYSTEM
XTENSA_SYSTEM = $(XTTOOLS)/config
endif

# gcc/g++ versions 3.4.X and 4.1.X are supported 
# support for RHEL3 default compiler gcc 3.2.3 has been *deprecated*
CC = gcc
CXX = g++

# Several variants of XTMP/ISS libraries are provided inside XTTOOLS:
# "iss"         directory contains libraries compatible with gcc 3.4.X
# "iss-GCC-4.1" directory contains libraries compatible with gcc 4.1.X
# "iss-RHEL3"   directory contains *deprecated* libraries compatible with
#               the RHEL3 default compiler (gcc 3.2.3)
ifneq ($(findstring 4.1.,$(shell $(CC) -dumpversion)),)
LIB_VARIANT = -GCC-4.1
endif
ifneq ($(findstring 3.2.3,$(shell $(CC) -dumpversion)),)
LIB_VARIANT = -RHEL3
endif
ISSLIBDIR = iss$(LIB_VARIANT)

# If you are using gcc from a non-standard location, and you don't set
# LD_LIBRARY_PATH, you may need to specify the location of gcc libraries
#GCC_DIR = /usr
#GCC_LIBS = -L$(GCC_DIR)/lib -Wl,-rpath $(GCC_DIR)/lib

# By default, QuickThreads are used for XTMP core and user threads.
# Uncomment the following line if you want to use SystemC (version 2.2)
#USE_SYSTEMC = true

# SystemC 2.2 location
SYSTEMC = $(XTTOOLS_TOOLS)/systemc
SYSTEMC_INCLUDE = $(SYSTEMC)/include
SYSTEMC_LIB=$(SYSTEMC)/lib-linux$(LIB_VARIANT)/libsystemc.a

# QuickThreads package
QT_DIR = $(XTTOOLS_TOOLS)
QT_INCLUDE = $(QT_DIR)/include
QT_LIB = $(QT_DIR)/lib/libqt.a

#==================================================================#
# END:                                                             #
# VARIABLES YOU MAY WANT TO CHANGE BEFORE USING THIS MAKEFILE      #
#==================================================================#

# Name of the Xtensa processor core
XTENSA_CORE = peregrine

# Files required by XTMP main program

ifeq ($(USE_SYSTEMC),true)
# The precompiled driver is in $(XTTOOLS)/lib/$(ISSLIBDIR)/sysc_driver.so 
MP_DRIVER = sysc_driver.$(OBJ)
else
# The precompiled driver is in $(XTTOOLS)/lib/$(ISSLIBDIR)/qt_driver.so 
MP_DRIVER = qt_driver.$(OBJ)
endif


LD = $(CXX)
OPT = -g
# OPT = -O2 for optimized build
ARCH_FLAG = -m32
WARN = -Wall
# WARN = -w      for no warnings
# WARN = -Wextra for more warnings
COMPILE_ONLY = -c
INC_FLAG = -I
INCLUDES = -I $(XTTOOLS)/include
DEFINES = -DXTENSA_SYSTEM=\"$(XTENSA_SYSTEM)\" -DXTENSA_CORE=\"$(XTENSA_CORE)\"
CFLAGS  = $(OPT) $(ARCH_FLAG) $(WARN) $(INCLUDES)
LDFLAGS = $(OPT) $(ARCH_FLAG)
LDOUT = -o 
EXE = 
OBJ = o
RMDIR = rm -rf
RM = rm -rf

XTMP_OBJS = xtmp_options.$(OBJ) getopt.$(OBJ)

XTMP_LIBS = -L$(XTTOOLS)/lib/$(ISSLIBDIR) -lxtmp \
            -Wl,-rpath $(XTTOOLS)/lib/$(ISSLIBDIR) \
            -Wl,-rpath $(XTTOOLS)/lib \
            $(GCC_LIBS)

ifeq ($(USE_SYSTEMC),true)
LIBS = $(XTMP_OBJS) $(XTMP_LIBS) $(SYSTEMC_LIB) 
else
LIBS = $(XTMP_OBJS) $(XTMP_LIBS) $(QT_LIB) 
endif


QUEUE_EXE = 
TIEPORTS_EXE = 
LOOKUP_EXE = 

EXAMPLES = \
	up_iss$(EXE) \
	regtest_iss$(EXE) \
	peekpoke_iss$(EXE) \
	tieGroups_iss$(EXE) \
	$(TIEPORTS_EXE) \
	$(QUEUE_EXE) \
	$(LOOKUP_EXE) \
	forward_iss$(EXE) \
	localMemDevice_iss$(EXE) \
	pifMemDevice_iss$(EXE) \
	inbound_iss$(EXE) \
	event_iss$(EXE) \
	simModeSwitch_iss$(EXE) \


.PHONY: all
all: $(EXAMPLES)

%.$(OBJ) : %.c
	$(CC) $(CFLAGS) $(DEFINES) $(COMPILE_ONLY) $<

XCC = $(XTTOOLS)/bin/xt-xcc 
XCCFLAGS = -g

%.out: %.c
	$(XCC) --xtensa-core=$(XTENSA_CORE) $(XCCFLAGS) -o $@ $<

%_tdk: %.tie
	$(XTTOOLS)/bin/tc $(TC_FLAGS) --xtensa-core=$(XTENSA_CORE) -d $@ $<

# Compile sysc_driver.cxx, the top-level program for XTMP that uses
# System-C as the thread package.
# Feel free to copy sysc_driver.cxx and modify it yourself
# You can also link in the pre-compiled version of sysc_driver.so
sysc_driver.$(OBJ): $(XTTOOLS)/src/iss/sysc_driver.cxx
	$(CC) $(CFLAGS) $(COMPILE_ONLY) $(INC_FLAG) $(SYSTEMC_INCLUDE) $<

# Compile either qt_driver.c or fiber_driver.c, the top-level program 
# for XTMP that use QT and NT fibers as the thread package, respectively.
# Feel free to copy qt_driver.c or fiber_driver.c and modify it yourself.
# You can also link in the pre-compiled version.
qt_driver.$(OBJ): $(XTTOOLS)/src/iss/qt_driver.c
	$(CC) $(CFLAGS) $(COMPILE_ONLY) -I$(QT_INCLUDE) $<

# Uniprocessor example (Section 2.7.1)
up_iss$(EXE): up_iss.$(OBJ) sieve.out sieve_long.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

sieve_long.out: sieve.c
	$(XCC) --xtensa-core=$(XTENSA_CORE) $(XCCFLAGS) -DITERS=1000 -o $@ $<
	
# Multiprocessor example (Section 2.7.2)
producer.out consumer.out mp_iss$(EXE):
	@(echo "**** Error: Multiprocessor synchronization example requires 'Conditional store synchronize' option."; exit 1); 

# Register test example (Section 3.2.2)
regtest_iss$(EXE): regtest_iss.$(OBJ) empty.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# Peek/Poke example (Sections 3.2.4 and 5.4)
peekpoke_iss$(EXE): peekpoke_iss.$(OBJ) empty.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# TIE group example (Section 4.1)
tieGroups_iss$(EXE): tieGroups_iss.$(OBJ) $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# TIE port example: export state, import wire (Sections 4.2.1 and 4.2.2)
export_import.out tiePorts_iss$(EXE):
	@(echo "**** Error: TIE port example cannot be built on this configuration."; exit 1); 

# TIE queue examples (Sections 4.3 and 4.4)
queue_reader.out queue_writer.out queue_reader_long.out queue_writer_long.out queue_iss$(EXE) builtinQueue_iss$(EXE):
	@(echo "**** Error: TIE queue examples cannot be built on this configuration."; exit 1); 

# Transaction forwarding example (Section 5.5.1)
forward_iss$(EXE): forward_iss.$(OBJ) sieve.out sieve_long.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# Custom local memory device example (Section 5.8)
localMemDevice_iss$(EXE): localMemDevice_iss.$(OBJ) sieve.out sieve_long.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# Inbound PIF example (Section 5.9.1)
inbound_iss$(EXE): inbound_iss.$(OBJ) inbound.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# Custom PIF memory device example (Section 6.4.1)
pifMemDevice_iss$(EXE): pifMemDevice_iss.$(OBJ) sieve.out sieve_long.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# Event synchronization example (Section 6.4.2)
event_iss$(EXE): event_iss.$(OBJ) event.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)

# Byte disable example
byte_disable.out byteDisable_iss$(EXE): 
	@(echo "**** Error: Byte disable example requires 'TIE arbitrary byte enables' option."; exit 1); 

# Example to demonstrate RCW
rcw.out readCondWrite_iss$(EXE): 
	@(echo "**** Error: Read-conditional-write example requires 'Conditional store synchronize' option."; exit 1); 

# Example to illustrate cores simulating in multiple clock domains
multClock_iss$(EXE):
	@(echo "**** Error: Multiple-clock example requires 'Conditional store synchronize' option."; exit 1); 

# Simulation mode switching example (Section 7.7)
simModeSwitch_iss$(EXE): simModeSwitch_iss.$(OBJ) sieve_long.out $(MP_DRIVER) $(XTMP_OBJS)
	$(LD) $(LDFLAGS) $(LDOUT)$@ $< $(MP_DRIVER) $(LIBS)


.PHONY: clean
clean:
	-$(RMDIR) byte_disable_tdk export_import_tdk queue_tdk 
	-$(RM) $(EXAMPLES) *.out *.$(OBJ) *.dis *~ \#* *.log

