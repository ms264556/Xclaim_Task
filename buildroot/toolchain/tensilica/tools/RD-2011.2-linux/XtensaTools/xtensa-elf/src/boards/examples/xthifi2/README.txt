Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
These coded instructions, statements, and computer programs are the
copyrighted works and confidential proprietary information of Tensilica Inc.
They may not be modified, copied, reproduced, distributed, or disclosed to
third parties in any manner, medium, or form, in whole or in part, without
the prior written consent of Tensilica Inc.
--------------------------------------------------------------------------------

This directory contains examples and other support for the XT-HiFi2 board.
This README describes the board, how it boots, and how to build the examples.


Background
----------

The XT-HiFi2 board is based on a Stretch board with a S6105 chip which
contains an Xtensa core with the Tensilica HiFi2 engine. For Tensilica 
customers, the XT-HiFi2 board is supported entirely by Tensilica tools 
and does not require support from Stretch or Stretch tools.

In reality the board contains another core and a bootloader (miniboot)
which performs some configuration and passes control to the reset vector
of the HiFi2 engine (which we may refer to as the XT-HiFi2 core).
The board contains 4 MB of serial flash on a SPI interface. It cannot be 
mapped to the reset vector so the board cannot boot directly from flash.
The board is supplied with the flash preconfigured to boot to XT-HiFi2.
The boot process of this board is discussed in a section below along with
instructions to recover if the boot sectors of the flash are corrupted.

The board also contains many devices, only some of which are supported
by the Tensilica XT-HiFi2 product, its libraries and documentation.
See the XT-HiFi2 development board user manual for details.


Examples
--------

This directory contains examples specific to the Stretch-based XT-HiFi2 
board, using information from the board-specific header xthifi2.h and 
others. These examples can be compiled and linked for XT-HiFi2 board by 
using an appropriate linker-support-package (LSP). Since the core is fixed,
your tools must be cofigured to use this core (as described in the manual).

    xt-xcc -g -I<xtensa_tools_root>/xtensa-elf/include/xtensa/xthifi2 \
        <example>.c -mlsp=<lsp> -o <example>

    <lsp> is the name of the desired LSP (eg. xthifi2-rt)

The LSPs named xthifi2-rt[-rom] are appropriate and automatically cause 
the board-support library libxthifi2.a to be linked.

The supplied Makefile may be used to build any or all of the examples by

    xt-make [<example>]

Then connect to the board using XOCD and xt-gdb as usual and run <example>.


Boot Process
------------

The process by which this board is booted is briefly outlined here so 
the user can better understand what might be going on when something is
not working. The process should normally proceed automatically as far as
the reset vector of the XT-HiFi2 core, where the user's program takes over.

1. First Stage Boot - Hardware

1.1 Hardware on the Stretch S6105 chip reads the boot paramater table (BPT)
    from the top 2KB of the on-board serial flash, and configures various
    aspects of the core (clocks, routing, memory map, ...).

1.2 Hardware loads the miniboot loader from serial flash and runs it on
    the primary core (this is the Stretch Configurable Processor, SCP).
    The miniboot loader lives in the first sector at the base of flash.

2. Second Stage Boot - Miniboot Loader

   The XT-HiFi2 board from Tensilica is supplied with a customized version
   of the miniboot loader whose jobs is to establish the environment of 
   the XT-HiFi2 board, which is assumed by all the Tensilica XT-HiFi2
   libraries and examples. The Tensilica miniboot tries to be transparent
   to XT-HiFi2 users so produces no console output via the serial port.

   The miniboot loader performs more initialization of the board (devices,
   memory map adjustments, assignment of interrupts, etc.). It looks for 
   an XT-HiFi2 application image in the flash and (if found) loads it to 
   RAM (including reset vector code). If an application is not found, a 
   jump-to-self is inserted at the reset vector so the core awaits a 
   connection from OCD.

   Finally miniboot starts the XT-HiFi2 core (which executes from its
   reset vector) and idles the SCP core on which is was running.

Please see the XT-HiFi2 board user manual (PDF) for more information.


Recovery from Corruption of Boot Sectors in Flash
-------------------------------------------------

This board is highly dependent on the BPT and miniboot loader which 
occupy the first and last sectors of the on-board serial flash. Though
these sectors are pre-programmed by Tensilica, it is possible they can
become corrupted or erased by users in the course of flashing their
own applications. The supplied 'flasher' example attempts to avoid 
overwriting or erasing the first and last sectors, but it is possible
to override it when running flasher in the debugger. Therefore images
of the miniboot loader and the BPT are provided along with a means
to restore them to the flash (the 'restore' example program). 

The steps below assume you have previously set up your OCD topology file
and run the OCD daemon on your OCD probe host, per instructions in the 
XT-HiFi2 user manual. In the steps below:
- The current working directory is the xthifi2 examples source directory 
  of your tools installation:
    <xtensa_tools_root>/xtensa-elf/src/boards/examples/xthifi2
- <xthifi2-bin> is the location of the pre-built example binaries for the
  XT-HiFi2 board, in your core-specific tools tree:
    <xtensa_root>/xtensa-elf/bin/xthifi2
- <OCD-host> is the domain name or IP address of your OCD host.
See XT-HiFi2 board user manual for explanations of the <xtensa_*> paths.

Before doing this invasive procedure, please be sure the problem is not
somewhere else. If you can connect with OCD and run code from the reset
vector, the problem is probably somewhere else. Here's a way to verify:

0.  Attempt to run a pre-built XT-HiFi2 executable via OCD (eg. announce).

    Power-cycle the board.
    Restart the OCD daemon.
    Attempt to run a pre-built XT-HiFi2 executable via OCD.

    > xt-gdb <xthifi2-bin>/announce
    (xt-gdb) tar rem <OCD-host>:20001 0
    (xt-gdb) reset
    (xt-gdb) load
    (xt-gdb) quit
    The program is running.  Exit anyway? (y or n) y

    You should see output on the serial port, like this:

    Hello, World!
    I'm running on a XT-HiFi2 board at 299.638 MHz!
    I'll print chars until you stop me.

            0 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

            (more lines follow, ad. inifinitum).

    If you see this output, the board is working correctly and there is
    no need to restore the boot sectors of the flash.

    If you do not see this output (assuming your serial port connection
    and setup are configured correctly and working), it is reasonable to
    assume the flash on the board is corrupt. Follow the steps below.

You are about to erase and reprogram the boot sectors in the flash.
Please follow these steps exactly and in the order they are given!

1.  Set OCD DIP switch 1 down (ON). This prevents hardware reading the
    BPT from flash, allowing one loaded via OCD to be used.

2.  Power cycle the board (in most cases hard reset is sufficient).

3.  Restart the OCD daemon.

4.  Use xt-gdb to load and run miniboot via OCD.
    You will connect to the hidden SCP core via OCD, port 20000.
    Ignore any warning from xt-gdb about a core configuration mismatch.

    > xt-gdb boot/xthifi2-boot.elf
    (xt-gdb) tar rem <OCD-host>:20000 0
    (xt-gdb) reset
    (xt-gdb) load
    (xt-gdb) quit
    The program is running.  Exit anyway? (y or n) y

5.  Run 'restore' example and use it to burn miniboot and BPT to flash.
    You will connect to the XT-HiFi2 core via OCD, port 20001.

    > xt-gdb <xthifi2-bin>/restore
    (xt-gdb) tar rem <OCD-host>:20001 0
    (xt-gdb) reset
    (xt-gdb) load
    (xt-gdb) restore boot/xthifi2-boot.bin binary ldr_image
    (xt-gdb) restore boot/xthifi2-bpt.bin binary bpt_image
    (xt-gdb) continue

    You should see output on the serial port, like this:

    Erasing miniboot sector 0x000000 - 0x00FFFF ...
    Programming miniboot 0x000000 - 0x00FFFF ...
    Erasing BPT sector 0x3F0000 - 0x3FFFFF ...
    Programming BPT 0x3FF800 - 0x3FFFFF ...
    Miniboot and BPT restored, please reset board !!!

    (xt-gdb) quit
    The program is running.  Exit anyway? (y or n) y

6.  Set OCD DIP switch 1 up (OFF). This causes hardware to read the BPT
    from flash after reset.

7.  Reset the board.

8.  Attempt to run a pre-built XT-HiFi2 executable via OCD (eg. announce)
    as described in step 0 above. If that fails, repeat all the above steps.

