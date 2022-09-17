/* Copyright (c) 2008-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

 /*
 This is simple example flash programmer for the XT-HiFi2 board.
 The flash is a serial device on a SPI interface (not memory-mapped).
 All flash addresses are offsets within the flash (NOT memory addresses).
 The flasher unprotects the entire flash before erasing or programming.

 The XT-HiFi2 board is based on a Stretch board with a S6105 chip which
 contains an Xtensa core with the Tensilica HiFi2 engine. It also contains
 another core and a built-in bootloader which takes care of loading a boot
 image from the serial flash to the reset vector of the HiFi2 core in RAM,
 then gives control to the HiFi2 core and idles itself (so it is transparent
 to the XT-HiFi2 user) creating the appearance that HiFi2 booted from reset.
 The intended use of the XT-HiFi2 board is supported entirely by Tensilica
 tools and does not require any support from Stretch or Stretch tools.

 The bootloader occupies 64KB at the base of the flash and 2KB at the top.
 This flasher program refuses to erase or overwrite the bootloader sectors
 (doing so would render the XT-HiFi2 board unusable and require use of a
 a special tool to restore the original boot loader images). Its default
 flash base is the first block boundary above the bootloader. Because it 
 has no information about the top parameter blocks, it has to reserve an
 entire 64 KB major block at the top. The effective size is 4 MB - 128 KB.

 While flasher is oriented primarily toward flashing boot images of XT-HiFi2
 applications to the boot flash on the base board (id 0), it also supports
 other serial flash devices. There is one on the audio daughterboard and an
 optional SD card that can be plugged in. See xthifi2-sx-flash.h for IDs.
 If flash_id is not 0, flasher does not preserve the bottom and top blocks.
 */

#include    <stdlib.h>
#include    <xtensa/xthifi2.h>
#include    <xtensa/xtbsp.h>
#include    <xtensa/xthifi2-sx-types.h>
#include    <xtensa/xthifi2-sx-errors.h>
#include    <xtensa/xthifi2-sx-flash.h>

/* Force variables to data section (else compiler might "optimize" to .bss) */
/* so they will be initialized at load-time and can be set by GDB before run. */
#define DSECT __attribute__ ((section (".data")))

/* Uses the SPI flash driver in the board support library, so the actual type */
/* of the flash device is concealed here. The driver interface is CFI-like. */
sx_flash_devinfo flash_info;        /* flash device info (like CFI) */
unsigned flash_id DSECT = 0;        /* flash device (default is boot flash) */

#define BOOTOFFSET  0x10000         /* offset to boot image in flash */
#define TOPBPTSIZE  0x800           /* size of boot parameter table at top */
#ifdef XTBOARD_RAM_VADDR
#define BINARYADDR (XTBOARD_RAM_VADDR + 0x1000000)  /* base of RAM image */
#else
#define BINARYADDR 0                      /* (must set image_base manually) */
#endif

/* User can change these variables from debugger after the call to init(). */
/* Set flash_base and flash_top (block aligned) to limit region affected. */
/* Set image_size to avoid programming entire region. */
/* To erase (a region of) flash w/o programming, set image_size = 0. */
unsigned flash_base DSECT = 0;      /* offset where image starts in flash */
unsigned flash_top  DSECT = 0;      /* top+1 of region erased for image */
unsigned image_base DSECT = BINARYADDR; /* address of image in memory */
unsigned image_size DSECT = 0;      /* size of image, amount programmed */
unsigned verif_base DSECT = 0;      /* address to read back image, if != 0 */

/* Define available flash range that doesn't overwrite bootloader. */
unsigned avail_base DSECT = 0;      /* bootloader lives below here */
unsigned avail_top  DSECT = 0;      /* boot param table lives above here */


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

/* Write len number of hex digits to primary console device (with leading 0s) */
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
  puthex(result, 8);
  putstring("\n");
  exit(err);
}

/* Initialize flash information after querying device. Return any error. */
static sx_uint32 init(void) __attribute__((noinline));
static sx_uint32 init(void)
{
    sx_uint32 result;

    result = sx_flash_query_device_info(flash_id, &flash_info);
    if (result != SXERR_NONE)
        return result;

    /* reserve block-aligned space at bottom and top for bootloader */
    if (flash_id == SX_FLASH_DEV_PRIMARY) {
        avail_base = BOOTOFFSET;
        avail_top  = (flash_info.dev_size - TOPBPTSIZE) 
                        & ~(flash_info.blk_size - 1);
    }
    else {
        avail_base = 0;
        avail_top  = flash_info.dev_size;
    }
    flash_base = avail_base;
    flash_top  = avail_top;
    image_size = flash_top - flash_base;
    image_base = BINARYADDR;
    verif_base = image_base + flash_info.dev_size;

    return SXERR_NONE;
}

/* Erase all flash blocks overlapping a given range of offsets in the flash. */
static void eraseFlash(sx_uint32 from, sx_uint32 to)
{
  sx_uint32 result;
  sx_uint32 flashpos;                   /* offset in flash (not address) */

  /* unlock entire flash */
  sx_flash_unprotect_device(flash_id);

  /* erase blocks overlapping flash range */
  for (                                           /* for each block ... */
    flashpos = from & ~(flash_info.blk_size - 1); /* assume pwr of 2 aligned */
    flashpos < to;
    flashpos += flash_info.blk_size
    ) {
    /* erase block */
    while ((result = sx_flash_erase_sector(flash_id, flashpos)) == SXERR_INUSE);
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
  while ((result = sx_flash_write(flash_id, image, from, size)) == 0);
  if (result != size)
      error(4, "sx_flash_write failed", from + result, result);
}

/* Verify image in flash by reading back and comparing with original. */
/* Caller must ensure image and copy do not overlap (not checked)! */
static void verifyFlash(sx_uint8 *image, sx_uint8 *copy, 
                        sx_uint32 from, sx_uint32 size)
{
  sx_uint32 result;

  if (size == 0)
    return;
  while ((result = sx_flash_read(flash_id, copy, from, size)) == 0);
  if (result != size)
      error(5, "sx_flash_read failed", from + result, result);
  for (result = 0; result < size; ++result) {
    if (copy[result] != image[result])
      error(6, "Verify failed", from + result, result);
  }
}

/* Erase and program flash. Notify user of progress via console. */
int main(int argc, char *argv[])
{
  int ok = 0;

  putstring("\n");
  if (init()) {
    putstring("Error - driver could not recognize flash.\n");
    return 2;
  }
  /* BREAK HERE to change flash_base, flash_top, image_size or set verif_base */
  if (flash_base < avail_base || flash_top > avail_top 
        || flash_base > flash_top) {
    putstring("Error - flash region is not entirely within available flash.\n");
    return 1;
  }
  if (flash_base + image_size > flash_top) {
    putstring("Error - image is too big for flash region.\n");
    return 1;
  }
  putstring("Programming Flash on XT-HiFi2 board.\n");
  putstring("Erase Flash...\n");
  eraseFlash(flash_base, flash_top);
  putstring("Program Flash...\n");
  programFlash((sx_uint8 *)image_base, flash_base, image_size);
  /* Verify only if user has requested it by designating a buffer. */
  if (verif_base != 0) {
    putstring("Verify Flash...\n");
    if (verif_base < image_base + image_size && 
        image_base < verif_base + image_size)
      putstring("Verify buffer overlaps image buffer, cannot verify.\n");
    else
      verifyFlash((sx_uint8 *)image_base, (sx_uint8 *)verif_base, 
                  flash_base, image_size);
  }
  putstring("Flash Done !!!\n");

  return 0;
}

