$(error config does not support this example)
# Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2006 Tensilica Inc.  ALL RIGHTS RESERVED.
# 
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.

EXAMPLE_NAME = xtsc_tx_loader$(EXE)
HOST_OBJECTS = sc_main.$(OBJ)
TARGET_OBJECTS = target/producer.out target/consumer.out
EXAMPLE_DIR = $(CURDIR)
CUSTOM_TARGET_CFLAGS = -mlsp=tiny

include $(EXAMPLE_DIR)/../Makefile.include
include $(EXAMPLE_DIR)/../Makefile.common

custom_target: $(TARGET_OBJECTS) job1.scr job2.scr
	xt-load --hex job1.scr -o job1.img
	xt-load --hex job2.scr -o job2.img

custom_clean:
	-$(RM) job1.img
	-$(RM) job2.img
