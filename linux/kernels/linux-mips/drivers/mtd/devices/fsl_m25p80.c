/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 * only for Freescale eSPI controller
 *
 * Author: Chen Gong <g.chen@freescale.com>
 * heavily based on m25p80.c by Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/delay.h>

/* Flash opcodes. */
#define	OPCODE_WREN		0x06	/* Write enable */
#define	OPCODE_RDSR		0x05	/* Read status register */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K 		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_BE		0xc7	/* Erase whole flash block */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */
#define	OPCODE_EN4B		0xb7	/* Enter 4-byte mode */
#define	OPCODE_EX4B		0xe9	/* Exit 4-byte mode */

/* Spansion flashes */
#define OPCODE_BRWR     0x17    /* Bank register write */
#define S25FL256S       0x010219

/* Micron flashes */
#define N25Q256A            0x20ba19
#define OPCODE_WRVCR        0x81    /* Write Volatile Configuration Register */
#define OPCODE_RDVCR        0x85    /* Read Volatile Configuration Register */
#define OPCODE_EN4BYTEADDR  0xb7    /* Enter 4-byte Address Mode */
#define OPCODE_EX4BYTEADDR  0xe9    /* Exit 4-byte Address Mode */
#define OPCODE_RFSR         0x70    /* Read Flag Status Register */
#define OPCODE_CLFSR        0x50    /* Clear Flag Status Register */
#define FSR_ADD             1       /* 3-byte/4-byte address mode */
#define FSR_RDY             0x80    /* Ready/Busy program erase controller */
#define	MAX_READY_WAIT_JIFFIES_N25  (480 * HZ)  /* N25Q256 specs 480s max chip erase */

/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_JIFFIES  (3 * HZ)
#define	MAX_READY_WAIT_COUNT	100000
#define	MAX_CMD_SIZE		5

#ifdef CONFIG_M25PXX_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
#else
#define	mtd_has_partitions()	(0)
#endif

/****************************************************************************/

struct m25p {
	struct spi_device	*spi;
	struct mutex		lock;
	struct mtd_info		mtd;
	unsigned		partitioned:1;
	unsigned int		page_size;	/* of bytes per page */
	u16			addr_width;
	u8			erase_opcode;
	u8			command[MAX_CMD_SIZE + FAST_READ_DUMMY_BYTE];
};

static inline struct m25p *mtd_to_m25p(struct mtd_info *mtd)
{
	return container_of(mtd, struct m25p, mtd);
}

static int m25p_cmdsz(struct m25p *flash)
{
	return 1 + flash->addr_width;
}

/* Micron flashes */
static int enter_4byte_add_mode(struct m25p *flash);
static int read_fsr(struct m25p *flash);
static u32 jedecId;

/****************************************************************************/

/*
 * Internal helper functions
 */

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static int read_sr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;
	struct spi_message	message;
	struct spi_transfer	x[1];
	u8			*local_buf;
	u8 val;

	spi_message_init(&message);
	memset(x, 0, sizeof x);
	x[0].len = 2;
	spi_message_add_tail(&x[0], &message);

	/* ... unless someone else is using the pre-allocated buffer */
	local_buf = kzalloc(32, GFP_KERNEL);
	if (!local_buf)
		return -ENOMEM;

	memcpy(local_buf, &code, 1);
	x[0].tx_buf = local_buf;
	x[0].rx_buf = local_buf + 1;

	/* do the i/o */
	retval = spi_sync(flash->spi, &message);
	if (retval == 0)
		memcpy(&val, x[0].rx_buf + 1, 1);

	kfree(local_buf);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}


/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(struct m25p *flash)
{
	u8	code = OPCODE_WREN;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int set_4byte(struct m25p *flash, u32 jedec_id, int enable)
{
	u8	code = enable ? OPCODE_EN4B : OPCODE_EX4B;

#if defined(V54_BSP)
    if (jedec_id == S25FL256S) {
        flash->command[0] = OPCODE_BRWR;
        flash->command[1] = enable << 7;
        return spi_write(flash->spi, flash->command, 2);
    } else if (jedec_id == N25Q256A) {
        return enter_4byte_add_mode(flash);
    } else
#endif
	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}

static void set_cmd_addr(u32 offset, u8 *cmd, int addr_width)
{
	cmd[1] = offset >> (addr_width * 8 - 8);
	cmd[2] = offset >> (addr_width * 8 - 16);
	cmd[3] = offset >> (addr_width * 8 - 24);
	if (addr_width >= 4)
		cmd[4] = offset;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct m25p *flash)
{
  if (jedecId != N25Q256A) {
	unsigned long deadline=jiffies+MAX_READY_WAIT_JIFFIES;
	int sr;

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	do {
		sr = read_sr(flash);
		if (sr < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;
		// sleep?
	} while(!time_after_eq(jiffies, deadline));

	return 1;
  } else {
	unsigned long deadline;
	int fsr;

	deadline = jiffies + MAX_READY_WAIT_JIFFIES_N25;

	do {
		if ((fsr = read_fsr(flash)) < 0)
			break;
		else if ((fsr & FSR_RDY)>>7)
			return 0;

		//cond_resched();

	} while (!time_after_eq(jiffies, deadline));

	return 1;
  }
}

/*
 * Micron N25Q, begin
 */
static int read_fsr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RFSR;
	struct spi_message	message;
	struct spi_transfer	x[1];
	u8			*local_buf;
	u8 val;

	spi_message_init(&message);
	memset(x, 0, sizeof x);
	x[0].len = 2;
	spi_message_add_tail(&x[0], &message);

	/* ... unless someone else is using the pre-allocated buffer */
	local_buf = kzalloc(32, GFP_KERNEL);
	if (!local_buf)
		return -ENOMEM;

	memcpy(local_buf, &code, 1);
	x[0].tx_buf = local_buf;
	x[0].rx_buf = local_buf + 1;

	/* do the i/o */
	retval = spi_sync(flash->spi, &message);
	if (retval == 0)
		memcpy(&val, x[0].rx_buf + 1, 1);

	kfree(local_buf);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}

/*
 * Clear flag status register
 * Returns negative if error occurred.
 */
static int read_clfsr(struct m25p *flash)
{
	u8 code = OPCODE_CLFSR;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}

/*
 * Read the volatile configuration register.
 * Return the volatile configuration register value or negative if an error occurs.
 */
static int read_vcr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDVCR;
	struct spi_message	message;
	struct spi_transfer	x[1];
	u8			*local_buf;
	u8 val;

	spi_message_init(&message);
	memset(x, 0, sizeof x);
	x[0].len = 2;
	spi_message_add_tail(&x[0], &message);

	/* ... unless someone else is using the pre-allocated buffer */
	local_buf = kzalloc(32, GFP_KERNEL);
	if (!local_buf)
		return -ENOMEM;

	memcpy(local_buf, &code, 1);
	x[0].tx_buf = local_buf;
	x[0].rx_buf = local_buf + 1;

	/* do the i/o */
	retval = spi_sync(flash->spi, &message);
	if (retval == 0)
		memcpy(&val, x[0].rx_buf + 1, 1);

	kfree(local_buf);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}

/*
 * Write volatile configuration register (1 byte)
 * Return negative if error occurred.
 */
static int write_vcr(struct m25p *flash, u8 val)
{
	flash->command[0] = OPCODE_WRVCR;
	flash->command[1] = val;

	return spi_write(flash->spi, flash->command, 2);
}

/*
 * Enter 4-byte Address Mode
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int enter_4byte_add_mode(struct m25p *flash)
{

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_EN4BYTEADDR;

	spi_write(flash->spi, flash->command, 1);

	/* Check 4-byte address mode is enabled */
	if((read_fsr(flash) & FSR_ADD) != 1)
		return 1;

	printk("fsr(%x)\n", read_fsr(flash));
	return 0;
}

/*
 * Exit 4-byte Address Mode
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int exit_4byte_add_mode(struct m25p *flash)
{
	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_EX4BYTEADDR;

	spi_write(flash->spi, flash->command, 1);

	return 0;
}
/*
 * Micron N25Q, end
 */

/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_block(struct m25p *flash)
{
	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %dKiB\n",
			dev_name(&flash->spi->dev), __func__,
			(u32)flash->mtd.size / 1024);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_BE;

	spi_write(flash->spi, flash->command, 1);

	return 0;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct m25p *flash, u32 offset)
{
	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %dKiB at 0x%08x\n",
			dev_name(&flash->spi->dev), __func__,
			flash->mtd.erasesize / 1024, offset);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	set_cmd_addr(offset, flash->command, flash->addr_width);

	spi_write(flash->spi, flash->command, m25p_cmdsz(flash));

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int m25p80_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 addr, len;
	uint32_t rem;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %d\n",
			dev_name(&flash->spi->dev), __func__, "at",
			(u32)instr->addr, (u32)instr->len);

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;
	div_u64_rem(instr->len, mtd->erasesize, &rem);
	if (rem)
		return -EINVAL;

	addr = instr->addr;
	len = instr->len;

	mutex_lock(&flash->lock);

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K. Based the same
	 * reason, in some cases we could use OPCODE_BE instead of
	 * OPCODE_SE
	 */

	/* now erase those sectors */
	if (len == flash->mtd.size && erase_block(flash)) {
		instr->state = MTD_ERASE_FAILED;
		mutex_unlock(&flash->lock);
		return -EIO;
	} else {
		while (len) {
			if (erase_sector(flash, addr)) {
				instr->state = MTD_ERASE_FAILED;
				mutex_unlock(&flash->lock);
				return -EIO;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;
		}
	}

	mutex_unlock(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int m25p80_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	struct spi_message	m;
	struct spi_transfer	x[1];
	u8			*local_buf;
	u32			i, page_size = 0;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			dev_name(&flash->spi->dev), __func__, "from",
			(u32)from, len);

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

	/* ... unless someone else is using the pre-allocated buffer */
	local_buf = kzalloc(flash->page_size +
			2 * (m25p_cmdsz(flash) + FAST_READ_DUMMY_BYTE), GFP_KERNEL);
	if (!local_buf)
		return -ENOMEM;

	spi_message_init(&m);
	memset(x, 0, sizeof x);
	spi_message_add_tail(&x[0], &m);

	x[0].tx_buf = local_buf;
	x[0].rx_buf = local_buf + m25p_cmdsz(flash) + FAST_READ_DUMMY_BYTE;

	/* Byte count starts at zero. */
	*retlen = 0;

	mutex_lock(&flash->lock);

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */
		mutex_unlock(&flash->lock);
		kfree(local_buf);
		return 1;
	}

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	local_buf[0] = OPCODE_READ;

	for (i = page_size; i < len; i += page_size) {
		page_size = len - i;
		if (page_size > flash->page_size)
			page_size = flash->page_size;

		/* write the next page to flash */
		set_cmd_addr(from + i, local_buf, flash->addr_width);

		x[0].len = m25p_cmdsz(flash) + FAST_READ_DUMMY_BYTE + page_size;

		wait_till_ready(flash);

		spi_sync(flash->spi, &m);
		memcpy(buf + i, x[0].rx_buf + m25p_cmdsz(flash) + FAST_READ_DUMMY_BYTE,
				page_size);

		if (retlen)
			*retlen += m.actual_length -
				m25p_cmdsz(flash) - FAST_READ_DUMMY_BYTE;
	}


	mutex_unlock(&flash->lock);

	kfree(local_buf);
	return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int m25p80_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 page_offset, page_size;
	struct spi_transfer t[1];
	struct spi_message m;
	u8 *local_buf, *tmp;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			dev_name(&flash->spi->dev), __func__, "to",
			(u32)to, len);

	/* sanity checks */
	if (!len)
		return 0;

	if (to + len > flash->mtd.size)
		return -EINVAL;

	/* ... unless someone else is using the pre-allocated buffer */
	local_buf = kzalloc(flash->page_size + m25p_cmdsz(flash), GFP_KERNEL);
	if (!local_buf)
		return -ENOMEM;

	*retlen = 0;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = local_buf;
	spi_message_add_tail(&t[0], &m);

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		mutex_unlock(&flash->lock);
		kfree(local_buf);
		return 1;
	}

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	local_buf[0] = OPCODE_PP;
	set_cmd_addr(to, local_buf, flash->addr_width);

	/* what page do we start with? */
	page_offset = ((unsigned)to % flash->page_size);

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= flash->page_size) {
		t[0].len = m25p_cmdsz(flash) + len;
		memcpy(local_buf + m25p_cmdsz(flash), buf, len);

		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash);
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = flash->page_size - page_offset;

		t[0].len = m25p_cmdsz(flash) + page_size;
		memcpy(local_buf + m25p_cmdsz(flash), buf, page_size);

		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash);

		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > flash->page_size)
				page_size = flash->page_size;

			tmp = local_buf;
			memcpy(tmp + m25p_cmdsz(flash), buf + i, page_size);
			tmp[0] = OPCODE_PP;
			set_cmd_addr(to + i, tmp, flash->addr_width);
			t[0].tx_buf = tmp;
			t[0].len = m25p_cmdsz(flash) + page_size;

			wait_till_ready(flash);

			write_enable(flash);

			spi_sync(flash->spi, &m);

			if (retlen)
				*retlen += m.actual_length - m25p_cmdsz(flash);
		}
	}

	mutex_unlock(&flash->lock);

	kfree(local_buf);

	return 0;
}


/****************************************************************************/

/*
 * SPI device driver setup and teardown
 */

struct flash_info {
	char		*name;

	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	u32		jedec_id;
	u16		ext_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	page_size;
	unsigned	sector_size;
	u16		n_sectors;

	u16		flags;
	u16		addr_width;
#define	SECT_4K		0x01		/* OPCODE_BE_4K works uniformly */
};


/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static struct flash_info __devinitdata m25p_data[] = {

	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ "at25fs010",  0x1f6601, 0, 256, 32 * 1024, 4, SECT_4K, 3},
	{ "at25fs040",  0x1f6604, 0, 256, 64 * 1024, 8, SECT_4K, 3},

	{ "at25df041a", 0x1f4401, 0, 256, 64 * 1024, 8, SECT_4K, 3},
	{ "at25df641",  0x1f4800, 0, 256, 64 * 1024, 128, SECT_4K, 3},

	{ "at26f004",   0x1f0400, 0, 256, 64 * 1024, 8, SECT_4K, 3},
	{ "at26df081a", 0x1f4501, 0, 256, 64 * 1024, 16, SECT_4K, 3},
	{ "at26df161a", 0x1f4601, 0, 256, 64 * 1024, 32, SECT_4K, 3},
	{ "at26df321",  0x1f4701, 0, 256, 64 * 1024, 64, SECT_4K, 3},

	/* Macronix */
	{ "mx25l3205d",  0xc22016, 0, 256, 64 * 1024, 64, 0, 3},
	{ "mx25l6405d",  0xc22017, 0, 256, 64 * 1024, 128, 0, 3},
	{ "mx25l12805d", 0xc22018, 0, 256, 64 * 1024, 256, 0, 3},
	{ "mx25l12855e", 0xc22618, 0, 256, 64 * 1024, 256, 0, 3},
	{ "mx25l25635e", 0xc22019, 0, 256, 64 * 1024, 512, 0, 4},

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ "s25sl004a", 0x010212, 0, 256, 64 * 1024, 8, 0, 3},
	{ "s25sl008a", 0x010213, 0, 256, 64 * 1024, 16, 0, 3},
	{ "s25sl016a", 0x010214, 0, 256, 64 * 1024, 32, 0, 3},
	{ "s25sl032a", 0x010215, 0, 256, 64 * 1024, 64, 0, 3},
	{ "s25sl064a", 0x010216, 0, 256, 64 * 1024, 128, 0, 3},
	{ "s25sl128a", 0x012018, 0x0300, 256, 256 * 1024, 64, 0, 3},
	{ "s25sl128b", 0x012018, 0x0301, 256, 64 * 1024, 256, 0, 3},

	{ "s25fl256s", 0x010219, 0x4d01, 256, 64 * 1024, 512, 0, 4},
	{ "s25fl512s", 0x010220, 0x4d00, 512, 256 * 1024, 256, 0, 4},

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ "sst25vf040b", 0xbf258d, 0, 256, 64 * 1024, 8, SECT_4K, 3},
	{ "sst25vf080b", 0xbf258e, 0, 256, 64 * 1024, 16, SECT_4K, 3},
	{ "sst25vf016b", 0xbf2541, 0, 256, 64 * 1024, 32, SECT_4K, 3},
	{ "sst25vf032b", 0xbf254a, 0, 256, 64 * 1024, 64, SECT_4K, 3},

	/* ST Microelectronics -- newer production may have feature updates */
	{ "m25p05",  0x202010,  0, 256, 32 * 1024, 2, 0, 3},
	{ "m25p10",  0x202011,  0, 256, 32 * 1024, 4, 0, 3},
	{ "m25p20",  0x202012,  0, 256, 64 * 1024, 4, 0, 3},
	{ "m25p40",  0x202013,  0, 256, 64 * 1024, 8, 0, 3},
	{ "m25p80",         0,  0, 256, 64 * 1024, 16, 0, 3},
	{ "m25p16",  0x202015,  0, 256, 64 * 1024, 32, 0, 3},
	{ "m25p32",  0x202016,  0, 256, 64 * 1024, 64, 0, 3},
	{ "m25p64",  0x202017,  0, 256, 64 * 1024, 128, 0, 3},
	{ "m25p128", 0x202018,  0, 256, 256 * 1024, 64, 0, 3},

	{ "m45pe80", 0x204014,  0, 256, 64 * 1024, 16, 0, 3},
	{ "m45pe16", 0x204015,  0, 256, 64 * 1024, 32, 0, 3},

	{ "m25pe80", 0x208014,  0, 256, 64 * 1024, 16, 0, 3},
	{ "m25pe16", 0x208015,  0, 256, 64 * 1024, 32, SECT_4K, 3},

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "w25x10", 0xef3011, 0, 256, 64 * 1024, 2, SECT_4K, 3},
	{ "w25x20", 0xef3012, 0, 256, 64 * 1024, 4, SECT_4K, 3},
	{ "w25x40", 0xef3013, 0, 256, 64 * 1024, 8, SECT_4K, 3},
	{ "w25x80", 0xef3014, 0, 256, 64 * 1024, 16, SECT_4K, 3},
	{ "w25x16", 0xef3015, 0, 256, 64 * 1024, 32, SECT_4K, 3},
	{ "w25x32", 0xef3016, 0, 256, 64 * 1024, 64, SECT_4K, 3},
	{ "w25x64", 0xef3017, 0, 256, 64 * 1024, 128, SECT_4K, 3},

	/* Micron -- new features will be added */
	{ "n25q256a13", 0x20ba19, 0, 256, 64 * 1024, 512, 0, 4},
};

static struct flash_info *__devinit jedec_probe(struct spi_device *spi)
{
	int			tmp;
	u8			code = OPCODE_RDID;
	u8			id[5];
	u32			jedec = 0;
	u16			ext_jedec = 0;
	struct flash_info	*info;
	struct spi_message	message;
	struct spi_transfer	x[1];
	u8			*local_buf;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */

	spi_message_init(&message);
	memset(x, 0, sizeof x);
	x[0].len = 6;
	spi_message_add_tail(&x[0], &message);

	/* ... unless someone else is using the pre-allocated buffer */
	local_buf = kzalloc(32, GFP_KERNEL);
	if (!local_buf)
		return NULL;

	memcpy(local_buf, &code, 1);
	x[0].tx_buf = local_buf;
	x[0].rx_buf = local_buf + 1;

	/* do the i/o */
	tmp = spi_sync(spi, &message);
	if (tmp == 0)
		memcpy(id, x[0].rx_buf + 1, 5);

	kfree(local_buf);

	if (tmp < 0) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: error %d reading JEDEC ID\n",
			dev_name(&spi->dev), tmp);
		return NULL;
	}
	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	ext_jedec = id[3] << 8 | id[4];

#if defined(V54_BSP)
    printk("jedec=0x%x ext_jedec=0x%x\n", jedec, ext_jedec);
#endif
	for (tmp = 0, info = m25p_data;
			tmp < ARRAY_SIZE(m25p_data);
			tmp++, info++) {
		if (info->jedec_id == jedec) {
			if (info->ext_id != 0 && info->ext_id != ext_jedec)
				continue;
			return info;
		}
	}
	dev_err(&spi->dev, "unrecognized JEDEC id %06x\n", jedec);
	return NULL;
}

/*
 * parse_flash_partition - Parse the flash partition on the SPI bus
 * @spi:	Pointer to spi_device device
 */
void parse_flash_partition(struct spi_device *spi)
{
	struct mtd_partition *parts;
	struct flash_platform_data *spi_pdata;
	int nr_parts = 0;
	static int num_flash;
	struct device_node *np = spi->dev.archdata.of_node;

	nr_parts = of_mtd_parse_partitions(&spi->dev, np, &parts);
	if (!nr_parts)
		goto end;

	spi_pdata = kzalloc(sizeof(*spi_pdata), GFP_KERNEL);
	if (!spi_pdata)
		goto end;
	spi_pdata->name = kzalloc(10, GFP_KERNEL);
	if (!spi_pdata->name)
		goto free_flash;
	snprintf(spi_pdata->name, 10, "SPIFLASH%d", num_flash++);

	spi_pdata->parts = parts;
	spi_pdata->nr_parts = nr_parts;

	spi->dev.platform_data = spi_pdata;

	return;

free_flash:
	kfree(spi_pdata);
end:
	return;
}

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int __devinit m25p_probe(struct spi_device *spi)
{
	struct flash_platform_data	*data;
	struct m25p			*flash;
	struct flash_info		*info;
	unsigned			i;

	/* Platform data helps sort out which chip type we have, as
	 * well as how this board partitions it.  If we don't have
	 * a chip ID, try the JEDEC id commands; they'll work for most
	 * newer chips, even if we don't recognize the particular chip.
	 */
	/* Parse the flash partition */
	parse_flash_partition(spi);
	data = spi->dev.platform_data;
	if (data && data->type) {
		for (i = 0, info = m25p_data;
				i < ARRAY_SIZE(m25p_data);
				i++, info++) {
			if (strcmp(data->type, info->name) == 0)
				break;
		}

		/* unrecognized chip? */
		if (i == ARRAY_SIZE(m25p_data)) {
			DEBUG(MTD_DEBUG_LEVEL0, "%s: unrecognized id %s\n",
					dev_name(&spi->dev), data->type);
			info = NULL;

		/* recognized; is that chip really what's there? */
		} else if (info->jedec_id) {
			struct flash_info	*chip = jedec_probe(spi);

			if (!chip || chip != info) {
				dev_warn(&spi->dev, "found %s, expected %s\n",
						chip ? chip->name : "UNKNOWN",
						info->name);
				info = NULL;
			}
		}
	} else
		info = jedec_probe(spi);

	if (!info)
		return -ENODEV;

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash)
		return -ENOMEM;

	flash->spi = spi;
	mutex_init(&flash->lock);
	dev_set_drvdata(&spi->dev, flash);

    /*
    * Micron N25Q serial flashes have a  configurable dummy 
    * clock cycle for  fast  read  instructions.  The dummy 
    * clock cycle is set to 8 (1Byte) =>  for the QIOFR the 
    * max supported frequency will be 95MHz.  For all other 
    * fast read operations the max supported frequency will 
    * be 108MHz. 
    */

    if (info->jedec_id >> 16 == 0x20 && 
        (((0xff & (info->jedec_id >> 8)) == 0xba) || ((0xff & (info->jedec_id >> 8)) == 0xbb))) {

        /* Set the N25QXXX dummy clock cycle to 8 bits. When the QIOFR is used, the maximum */
        /* allowed frequency is limited to 95MHz */
        write_enable(flash);
        write_vcr(flash, ((read_vcr(flash)) & (0x0F)) | 0x80);
        printk("vcr(%x)\n", read_vcr(flash));
    }

	if (data && data->name)
		flash->mtd.name = data->name;
	else
		flash->mtd.name = dev_name(&spi->dev);

	flash->page_size = info->page_size;
	flash->addr_width = info->addr_width;
	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = 1;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = info->sector_size * info->n_sectors;
	flash->mtd.erase = m25p80_erase;
	flash->mtd.read = m25p80_read;
	flash->mtd.write = m25p80_write;
	jedecId = info->jedec_id;
	if (flash->addr_width == 4)
		set_4byte(flash, info->jedec_id, 1);

	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096;
	} else {
		flash->erase_opcode = OPCODE_SE;
		flash->mtd.erasesize = info->sector_size;
	}

	dev_info(&spi->dev, "%s (%d Kbytes)\n", info->name,
			(u32)flash->mtd.size / 1024);

	DEBUG(MTD_DEBUG_LEVEL2,
		"mtd .name = %s, .size = 0x%.8x (%uMiB) "
			".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		flash->mtd.name,
		(u32)flash->mtd.size, (u32)flash->mtd.size / (1024*1024),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			DEBUG(MTD_DEBUG_LEVEL2,
				"mtd.eraseregions[%d] = { .offset = 0x%.8x, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, (u32)flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);


	/* partitions should match sector boundaries; and it may be good to
	 * use readonly partitions for writeprotected sectors (BP2..BP0).
	 */
	if (mtd_has_partitions()) {
		struct mtd_partition	*parts = NULL;
		int			nr_parts = 0;

#if defined(CONFIG_MTD_CMDLINE_PARTS) || defined(CONFIG_MTD_REDBOOT_PARTS)
		static const char *part_probes[] = {
#ifdef CONFIG_MTD_CMDLINE_PARTS
	"cmdlinepart",
#endif
#ifdef CONFIG_MTD_REDBOOT_PARTS
	"RedBoot",
#endif
	NULL
};
		nr_parts = parse_mtd_partitions(&flash->mtd,
				part_probes, &parts, 0);
#endif

		if (nr_parts <= 0 && data && data->parts) {
			parts = data->parts;
			nr_parts = data->nr_parts;
		}

		if (nr_parts > 0) {
			for (i = 0; i < nr_parts; i++) {
				DEBUG(MTD_DEBUG_LEVEL2, "partitions[%d] = "
					"{.name = %s, .offset = 0x%.8x, "
						".size = 0x%.8x (%uKiB) }\n",
					i, parts[i].name,
					(u32)parts[i].offset,
					(u32)parts[i].size,
					(u32)parts[i].size / 1024);
			}
			flash->partitioned = 1;
#ifdef V54_BSP
			// create a partition for the entire flash as /dev/mtd0
			add_mtd_device(&flash->mtd);
#endif
			return add_mtd_partitions(&flash->mtd, parts, nr_parts);
		}
	} else if (data->nr_parts)
		dev_warn(&spi->dev, "ignoring %d default partitions on %s\n",
				data->nr_parts, data->name);

	return add_mtd_device(&flash->mtd) == 1 ? -ENODEV : 0;
}


static int __devexit m25p_remove(struct spi_device *spi)
{
	struct m25p	*flash = dev_get_drvdata(&spi->dev);
	int		status;

	/* Clean up MTD stuff. */
	if (mtd_has_partitions() && flash->partitioned)
		status = del_mtd_partitions(&flash->mtd);
	else
		status = del_mtd_device(&flash->mtd);
	if (status == 0)
		kfree(flash);
	return 0;
}


static struct spi_driver m25p80_driver = {
	.driver = {
		.name	= "fsl_m25p80",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= m25p_probe,
	.remove	= __devexit_p(m25p_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};


static int m25p80_init(void)
{
	return spi_register_driver(&m25p80_driver);
}


static void m25p80_exit(void)
{
	spi_unregister_driver(&m25p80_driver);
}


module_init(m25p80_init);
module_exit(m25p80_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chen Gong <g.chen@freescale.com>");
MODULE_DESCRIPTION("MTD SPI driver for ST M25Pxx flash chips");
