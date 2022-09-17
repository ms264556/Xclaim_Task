/* Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

 /*
 This is a utility to restore the miniboot loader to the XT-HiFi board
 in case it was accidentally erased or corrupted in the flash. The miniboot
 loader occupies the 64KB first block of the flash and 2KB at the very top.
 The top block contains the Boot Parameter Table (BPT) used by hardware
 immediately after reset to configure some things and load the miniboot.
 This is essential to transform this board into the Tensilica XT-HiFi2.

 Two binary images are provided in the pre-built binary directory alongside
 this program and the other examples. Those images should be loaded to RAM
 at the addresses ldr_image and bpt_image resp. before this program is run.
 See XT-HiFi2 user manual for details.

 It is assumed that 'restore' is only used in rare emergencies. In order to
 guarantee the board will boot to a known state after a fresh miniboot and 
 BPT have been flashed, 'restore' also erases the application boot block
 (the second block). This prevents a possibly errant application from 
 leaving the board in an unusable state after reset.

 If the miniboot loader has indeed been erased or corrupted in the flash, 
 it is not possible to load and run this program in the normal way. First
 the miniboot loader itself has to be loaded via xt-gdb and OCD and run.
 For convenience, an ELF executable of the miniboot loader is provided.

 Background:

 The XT-HiFi2 board is based on a Stretch board with a S6105 chip which
 contains an Xtensa core with the Tensilica HiFi2 engine. It also contains
 another core and a built-in bootloader (miniboot) which loads an XT-HiFi2
 image from the serial flash to the reset vector of the HiFi2 core (in RAM),
 then gives control to the HiFi2 core and idles itself (so it is transparent
 to the XT-HiFi2 user) creating the appearance that HiFi2 booted from reset.
 The intended use of the XT-HiFi2 board is supported entirely by Tensilica
 tools and does not require any support from Stretch or Stretch tools.
 The flash is a serial device on a SPI interface (not memory-mapped).
 All flash addresses are offsets within the flash (NOT memory addresses).
 */

#include    <stdlib.h>
#include    <xtensa/xthifi2.h>
#include    <xtensa/xtbsp.h>
#include    <xtensa/xthifi2-sx-types.h>
#include    <xtensa/xthifi2-sx-errors.h>
#include    <xtensa/xthifi2-sx-flash.h>

/* Uses the SPI flash driver in the board support library, so the actual type */
/* of the flash device is concealed here. The driver interface is CFI-like. */

static sx_flash_devinfo flash_info; /* flash device info (like CFI) */

#define BLKSIZE     0x10000     /* size of (major) flash blocks */
#define LDROFFS     0x000000    /* offset of miniboot block in flash */
#define LDRTOP      (LDROFFS+BLKSIZE)
#define APPOFFS     BLKSIZE     /* offset of application boot block in flash */
#define APPTOP      (APPOFFS+BLKSIZE)
#define BPTSIZE     0x800       /* size of Boot Param Table (top param. blk) */
#define BPTOFFS     0x3ff800    /* offset of BPT in flash (in top block) */
#define BPTSECT     (BPTOFFS &~ (BLKSIZE-1)) /* offset of blk containing BPT */
#define BPTTOP      (BPTOFFS+BPTSIZE)

unsigned ldr_size  = BLKSIZE;   /* optionally modify with GDB after restore */
#ifdef XTBOARD_RAM_VADDR
unsigned ldr_image = (XTBOARD_RAM_VADDR + 0x1000000);   /* RAM image miniboot */
unsigned bpt_image = (XTBOARD_RAM_VADDR + 0x1010000);   /* RAM image BPT */
#else
unsigned ldr_image = 0;         /* (just to get it to compile) */
unsigned bpt_image = 0;         /* (just to get it to compile) */
#endif


/* Write string to primary console device. Much lower overhead than printf(). */
static void putstring(const char *s)
{
  char c;

  while ((c = *s++) != '\0') {
    outbyte(c);
    if (c == '\n')
      outbyte('\r');
  }
}

/* Write hex number of len digits to primary console device (with leading 0s). */
static void puthex(unsigned hex, unsigned len)
{
  unsigned lsh, mask, nib;

  putstring("0x");
  if (len == 0) return;
  lsh = (len-1) << 2;
  mask = 0xF << lsh;
  while (len-- > 0) {
    nib = (hex & mask) >> lsh;
    outbyte(nib<10 ? '0'+nib : 'A'+(nib-10));
    hex <<= 4;
  }
}

/* Write message with flash offset and result code and exit with error code. */
static void error(int err, const char *msg, unsigned offs, sx_uint32 result)
{
  putstring("Error ");
  putstring(msg); 
  putstring(" at offset ");
  puthex(offs, 8);
  putstring(", result=");
  puthex(result, 2);
  putstring("\n");
  exit(err);
}

/* Erase all flash blocks overlapping a given range of offsets in the flash. */
static void eraseFlash(sx_uint32 from, sx_uint32 to)
{
  sx_uint32 result;
  sx_uint32 flashpos;                   /* offset in flash (not address) */

  /* unlock entire flash */
  sx_flash_unprotect_device(0);

  /* erase blocks overlapping flash range */
  for (                                           /* for each block ... */
    flashpos = from & ~(flash_info.blk_size - 1); /* assume pwr of 2 aligned */
    flashpos < to;
    flashpos += flash_info.blk_size
    ) {
    /* erase block */
    while ((result = sx_flash_erase_sector(0, flashpos)) == SXERR_INUSE);
    if (result != 0)
        error(3, "sx_flash_erase_sector failed", flashpos, result);
  }
}

/* Program image from RAM to flash (caller ensures blank space is available). */
static void programFlash(sx_uint8 *image, sx_uint32 from, sx_uint32 size)
{
  sx_uint32 result;

  if (size == 0)
    return;
  while ((result = sx_flash_write(0, image, from, size)) == 0);
  if (result != size)
      error(4, "sx_flash_write failed", from + result, result);
}

/* Erase and program miniboot and BPT sectors of flash. Binary images have
   already been loaded to predefined locations in RAM (usually by the GDB
   'restore' command). Notify user of progress via (optional) console. */
int main(int argc, char *argv[])
{
  sx_uint32 result;

  result = sx_flash_query_device_info(0, &flash_info);
  if (result != SXERR_NONE) {
    putstring("Error - driver could not recognize flash.\n");
    return 2;
  }

  putstring("\n");

  putstring("Erasing miniboot sector ");
  puthex(LDROFFS, 6); putstring(" - "); 
  puthex(LDRTOP-1, 6); putstring(" ...\n");
  eraseFlash(LDROFFS, LDRTOP);
  putstring("Programming miniboot ");
  puthex(LDROFFS, 6); putstring(" - "); 
  puthex(ldr_size-1, 6); putstring(" ...\n");
  programFlash((sx_uint8 *)ldr_image, LDROFFS, ldr_size);

  putstring("Erasing application boot sector ");
  puthex(APPOFFS, 6); putstring(" - "); 
  puthex(APPTOP-1, 6); putstring(" ...\n");
  eraseFlash(APPOFFS, APPTOP);

  putstring("Erasing BPT sector ");
  puthex(BPTSECT, 6); putstring(" - "); 
  puthex(BPTTOP-1, 6); putstring(" ...\n");
  eraseFlash(BPTSECT, BPTTOP);
  putstring("Programming BPT ");
  puthex(BPTOFFS, 6); putstring(" - "); 
  puthex(BPTTOP-1, 6); putstring(" ...\n");
  programFlash((sx_uint8 *)bpt_image, BPTOFFS, BPTSIZE);

  sx_flash_protect_device(0);

  putstring("Miniboot and BPT restored, please reset board !!!\n");

  return 0;
}

