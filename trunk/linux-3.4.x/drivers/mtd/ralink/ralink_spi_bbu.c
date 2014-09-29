/*
 * MTD SPI driver for ST M25Pxx flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include "ralink_spi_bbu.h"
#include "ralink-flash.h"
#include "ralink-flash-map.h"

//#define SPI_DEBUG
//#define TEST_CS1_FLASH

#if defined (CONFIG_MTD_SPI_DUAL_READ)
#define RD_MODE_DIOR
#endif

#if defined(RD_MODE_DIOR) || defined(RD_MODE_DOR)
#define RD_MODE_DUAL
#elif defined(RD_MODE_QIOR) || defined(RD_MODE_QOR)
#define RD_MODE_QUAD
#endif

/* Choose the SPI flash mode */
#define BBU_MODE		// MT7621/MT7628 is BBU SPI flash controller
#define MORE_BUF_MODE

// check DUAL/QUAD and MORE_BUF_MODE, they can't be enabled together
#if defined(MORE_BUF_MODE) && (defined(RD_MODE_DIOR) || defined(RD_MODE_DOR) || defined(RD_MODE_QIOR) || defined(RD_MODE_QOR))
#error "DUAL/QUAD mode and MORE_BUF_MODE can't be enabled together"
#endif

/******************************************************************************
 * SPI FLASH elementray definition and function
 ******************************************************************************/

#define FLASH_PAGESIZE		256

/* Flash opcodes. */
#define OPCODE_WREN		6	/* Write enable */
#define OPCODE_WRDI		4	/* Write disable */
#define OPCODE_RDSR		5	/* Read status register */
#define OPCODE_WRSR		1	/* Write status register */
#define OPCODE_READ		3	/* Read data bytes */
#define OPCODE_PP		2	/* Page program */
#define OPCODE_SE		0xD8	/* Sector erase */
#define OPCODE_RES		0xAB	/* Read Electronic Signature */
#define OPCODE_RDID		0x9F	/* Read JEDEC ID */
#define OPCODE_DOR		0x3B	/* Dual Output Read */
#define OPCODE_QOR		0x6B	/* Quad Output Read */
#define OPCODE_DIOR		0xBB	/* Dual IO High Performance Read */
#define OPCODE_QIOR		0xEB	/* Quad IO High Performance Read */

/* Status Register bits. */
#define SR_WIP			1	/* Write in progress */
#define SR_WEL			2	/* Write enable latch */
#define SR_BP0			4	/* Block protect 0 */
#define SR_BP1			8	/* Block protect 1 */
#define SR_BP2			0x10	/* Block protect 2 */
#define SR_EPE			0x20	/* Erase/Program error */
#define SR_SRWD			0x80	/* SR write protect */

#define OPCODE_BRRD		0x16
#define OPCODE_BRWR		0x17
#define OPCODE_RDCR		0x35

#if !defined (SPI_DEBUG)
#define ra_inl(addr)  (*(volatile u32 *)(addr))
#define ra_outl(addr, value)  (*(volatile u32 *)(addr) = (value))
#define ra_dbg(args...) do {} while(0)
/*#define ra_dbg(args...) do { printk(args); } while(0)*/
#else
#define ra_dbg(args...) do { printk(args); } while(0)
#define _ra_inl(addr)  (*(volatile u32 *)(addr))
#define _ra_outl(addr, value)  (*(volatile u32 *)(addr) = (value))
u32 ra_inl(u32 addr)
{
	u32 retval = _ra_inl(addr);
	printk("%s(%x) => %x \n", __func__, addr, retval);
	return retval;
}

u32 ra_outl(u32 addr, u32 val)
{
	_ra_outl(addr, val);
	printk("%s(%x, %x) \n", __func__, addr, val);
	return val;
}
#endif // SPI_DEBUG //

#define ra_aor(addr, a_mask, o_value)  ra_outl(addr, (ra_inl(addr) & (a_mask)) | (o_value))
#define ra_and(addr, a_mask)  ra_aor(addr, a_mask, 0)
#define ra_or(addr, o_value)  ra_aor(addr, -1, o_value)

#define SPIC_READ_BYTES		(1<<0)
#define SPIC_WRITE_BYTES	(1<<1)

void usleep(unsigned int usecs)
{
	unsigned long timeout = usecs_to_jiffies(usecs);

	while (timeout)
		timeout = schedule_timeout_interruptible(timeout);
}

static int bbu_spic_busy_wait(void)
{
	int n = 100000;
	do {
		if ((ra_inl(SPI_REG_CTL) & SPI_CTL_BUSY) == 0)
			return 0;
		udelay(1);
	} while (--n > 0);

	printk("%s: fail \n", __func__);
	return -1;
}

void spic_init(void)
{
#if defined (CONFIG_RALINK_MT7621)
	// set default clock to hclk/5
	ra_and(SPI_REG_MASTER, ~(0xfff << 16));
	ra_or(SPI_REG_MASTER, (0x5 << 16));	//work-around 3-wire SPI issue (3 for RFB, 5 for EVB)
#elif defined (CONFIG_RALINK_MT7628)
	// set default clock to hclk/8
	ra_and(SPI_REG_MASTER, ~(0xfff << 16));
	ra_or(SPI_REG_MASTER, (0x6 << 16));
#endif

#ifdef TEST_CS1_FLASH
	ra_and(RALINK_SYSCTL_BASE + 0x60, ~(1 << 12));
	ra_or(SPI_REG_MASTER, (1 << 29));
#endif

	printk("Ralink SPI flash driver\n");
}

struct chip_info {
	char		*name;
	u8		id;
	u32		jedec_id;
	unsigned long	sector_size;
	unsigned int	n_sectors;
	char		addr4b;
};

static struct chip_info chips_data [] = {
	/* REVISIT: fill in JEDEC ids, for parts that have them */
	{ "FL016AIF",		0x01, 0x02140000, 64 * 1024, 32,  0 },
	{ "FL064AIF",		0x01, 0x02160000, 64 * 1024, 128, 0 },

	{ "S25FL032P",		0x01, 0x02154D00, 64 * 1024, 64,  0 },
	{ "S25FL064P",		0x01, 0x02164D00, 64 * 1024, 128, 0 },
	{ "S25FL128P",		0x01, 0x20180301, 64 * 1024, 256, 0 },
	{ "S25FL129P",		0x01, 0x20184D01, 64 * 1024, 256, 0 },
	{ "S25FL256S",		0x01, 0x02194D01, 64 * 1024, 512, 1 },

	{ "S25FL116K",		0x01, 0x40150140, 64 * 1024, 32,  0 },
	{ "S25FL132K",		0x01, 0x40160140, 64 * 1024, 64,  0 },
	{ "S25FL164K",		0x01, 0x40170140, 64 * 1024, 128, 0 },

	{ "EN25F16",		0x1c, 0x31151c31, 64 * 1024, 32,  0 },
	{ "EN25F32",		0x1c, 0x31161c31, 64 * 1024, 64,  0 },
	{ "EN25Q32B",		0x1c, 0x30161c30, 64 * 1024, 64,  0 },
	{ "EN25F64",		0x1c, 0x20171c20, 64 * 1024, 128, 0 }, // EN25P64
	{ "EN25Q64",		0x1c, 0x30171c30, 64 * 1024, 128, 0 },

	{ "AT26DF161",		0x1f, 0x46000000, 64 * 1024, 32,  0 },
	{ "AT25DF321",		0x1f, 0x47000000, 64 * 1024, 64,  0 },

	{ "N25Q032A",		0x20, 0xba161000, 64 * 1024, 64,  0 },
	{ "N25Q064A",		0x20, 0xba171000, 64 * 1024, 128, 0 },
	{ "N25Q128A",		0x20, 0xba181000, 64 * 1024, 256, 0 },

	{ "F25L32QA",		0x8c, 0x41168c41, 64 * 1024, 64,  0 }, // ESMT
	{ "F25L64QA",		0x8c, 0x41170000, 64 * 1024, 128, 0 }, // ESMT

	{ "MX25L1605D",		0xc2, 0x2015c220, 64 * 1024, 32,  0 },
	{ "MX25L3205D",		0xc2, 0x2016c220, 64 * 1024, 64,  0 },
	{ "MX25L6405D",		0xc2, 0x2017c220, 64 * 1024, 128, 0 },
	{ "MX25L12805D",	0xc2, 0x2018c220, 64 * 1024, 256, 0 },
	{ "MX25L25635E",	0xc2, 0x2019c220, 64 * 1024, 512, 1 },

	{ "GD25Q32B",		0xc8, 0x40160000, 64 * 1024, 64,  0 },
	{ "GD25Q64B",		0xc8, 0x40170000, 64 * 1024, 128, 0 },
	{ "GD25Q128C",		0xc8, 0x40180000, 64 * 1024, 256, 0 },

	{ "W25X32VS",		0xef, 0x30160000, 64 * 1024, 64,  0 },
	{ "W25Q32BV",		0xef, 0x40160000, 64 * 1024, 64,  0 },
	{ "W25Q64BV",		0xef, 0x40170000, 64 * 1024, 128, 0 }, // S25FL064K
	{ "W25Q128BV",		0xef, 0x40180000, 64 * 1024, 256, 0 },
	{ "W25Q256FV",		0xef, 0x40190000, 64 * 1024, 512, 1 },

	{ "STUB",		0x00, 0xffffffff, 64 * 1024, 64,  0 },
};

struct flash_info {
	struct mutex		lock;
	struct mtd_info		mtd;
	struct chip_info	*chip;
	u8			command[5];
};

struct flash_info *flash = NULL;

#ifdef MORE_BUF_MODE
static int bbu_mb_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg;
	int i, q, r;
	int rc = -1;

	if (flag != SPIC_READ_BYTES && flag != SPIC_WRITE_BYTES) {
		printk("we currently support more-byte-mode for reading and writing data only\n");
		return -1;
	}

	/* step 0. enable more byte mode */
	ra_or(SPI_REG_MASTER, (1 << 2));

	bbu_spic_busy_wait();

	/* step 1. set opcode & address, and fix cmd bit count to 32 (or 40) */
	if (flash && flash->chip->addr4b) {
		ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
		ra_or(SPI_REG_CTL, (code << 24) & SPI_CTL_ADDREXT_MASK);
		ra_outl(SPI_REG_OPCODE, addr);
	}
	else
	{
		ra_outl(SPI_REG_OPCODE, (code << 24) & 0xff000000);
		ra_or(SPI_REG_OPCODE, (addr & 0xffffff));
	}
	ra_and(SPI_REG_MOREBUF, ~SPI_MBCTL_CMD_MASK);
	if (flash && flash->chip->addr4b)
		ra_or(SPI_REG_MOREBUF, (40 << 24));
	else
		ra_or(SPI_REG_MOREBUF, (32 << 24));

	/* step 2. write DI/DO data #0 ~ #7 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL) {
			printk("%s: write null buf\n", __func__);
			goto RET_MB_TRANS;
		}
		for (i = 0; i < n_tx; i++) {
			q = i / 4;
			r = i % 4;
			if (r == 0)
				ra_outl(SPI_REG_DATA(q), 0);
			ra_or(SPI_REG_DATA(q), (*(buf + i) << (r * 8)));
		}
	}

	/* step 3. set rx (miso_bit_cnt) and tx (mosi_bit_cnt) bit count */
	ra_and(SPI_REG_MOREBUF, ~SPI_MBCTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_MOREBUF, (n_rx << 3 << 12));
	ra_or(SPI_REG_MOREBUF, n_tx << 3);

	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();
	if (flag & SPIC_WRITE_BYTES) {
		rc = 0;
		goto RET_MB_TRANS;
	}

	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL) {
			printk("%s: read null buf\n", __func__);
			return -1;
		}
		for (i = 0; i < n_rx; i++) {
			q = i / 4;
			r = i % 4;
			reg = ra_inl(SPI_REG_DATA(q));
			*(buf + i) = (u8)(reg >> (r * 8));
		}
	}

	rc = 0;
RET_MB_TRANS:
	/* step #. disable more byte mode */
	ra_and(SPI_REG_MASTER, ~(1 << 2));
	return rc;
}
#endif // MORE_BUF_MODE //

static int bbu_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg;

	bbu_spic_busy_wait();

	/* step 1. set opcode & address */
	if (flash && flash->chip->addr4b) {
		ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
		ra_or(SPI_REG_CTL, addr & SPI_CTL_ADDREXT_MASK);
	}
	ra_outl(SPI_REG_OPCODE, ((addr & 0xffffff) << 8));
	ra_or(SPI_REG_OPCODE, code);

#if defined(RD_MODE_QUAD) || defined(RD_MODE_DUAL)
	if (flag & SPIC_READ_BYTES)
		ra_outl(SPI_REG_DATA0, 0); // clear data bit for dummy bits in Dual/Quad IO Read
#endif
	/* step 2. write DI/DO data #0 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL) {
			printk("%s: write null buf\n", __func__);
			return -1;
		}
		ra_outl(SPI_REG_DATA0, 0);
		switch (n_tx) {
		case 8:
			ra_or(SPI_REG_DATA0, (*(buf+3) << 24));
		case 7:
			ra_or(SPI_REG_DATA0, (*(buf+2) << 16));
		case 6:
			ra_or(SPI_REG_DATA0, (*(buf+1) << 8));
		case 5:
			ra_or(SPI_REG_DATA0, *buf);
			break;
		case 3:
			reg = ra_inl(SPI_REG_CTL);
			if (((reg & (0x3<<19)) == (0x3 << 19)) && (flash && flash->chip->addr4b)) 
			{
				ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
				ra_or(SPI_REG_CTL, (*buf << 24) & SPI_CTL_ADDREXT_MASK);
				ra_and(SPI_REG_OPCODE, 0xff);
				ra_or(SPI_REG_OPCODE, (*(buf+1) & 0xff) << 24);
			}
			else
			{
				ra_and(SPI_REG_OPCODE, 0xff);
				ra_or(SPI_REG_OPCODE, (*buf & 0xff) << 24);
				ra_or(SPI_REG_OPCODE, (*(buf+1) & 0xff) << 16);
			}
			break;
		case 2:
			reg = ra_inl(SPI_REG_CTL);
			if (((reg & (0x3<<19)) == (0x3 << 19)) && (flash && flash->chip->addr4b)) 
			{
				ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
				ra_or(SPI_REG_CTL, (*buf << 24) & SPI_CTL_ADDREXT_MASK);
			}
			else
			{
				ra_and(SPI_REG_OPCODE, 0xff);
				ra_or(SPI_REG_OPCODE, (*buf & 0xff) << 24);
			}
			break;
		default:
			printk("%s: fixme, write of length %d\n", __func__, n_tx);
			return -1;
		}
	}

	/* step 3. set mosi_byte_cnt */
	ra_and(SPI_REG_CTL, ~SPI_CTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_CTL, (n_rx << 4));
	if (flash && flash->chip->addr4b && n_tx >= 4)
		ra_or(SPI_REG_CTL, (n_tx + 1));
	else
		ra_or(SPI_REG_CTL, n_tx);

	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();
	if (flag & SPIC_WRITE_BYTES)
		return 0;

	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL) {
			printk("%s: read null buf\n", __func__);
			return -1;
		}
		reg = ra_inl(SPI_REG_DATA0);
		switch (n_rx) {
		case 4:
			*(buf+3) = (u8)(reg >> 24);
		case 3:
			*(buf+2) = (u8)(reg >> 16);
		case 2:
			*(buf+1) = (u8)(reg >> 8);
		case 1:
			*buf = (u8)reg;
			break;
		default:
			printk("%s: fixme, read of length %d\n", __func__, n_rx);
			return -1;
		}
	}

	return 0;
}

static int raspi_read_rg(u8 code, u8 *val);
static int raspi_write_rg(u8 code, u8 *val);
static int raspi_wait_ready(int sleep_ms);
/*
 * read SPI flash device ID
 */
static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
	u8 code = OPCODE_RDID;
	int retval;

	retval = bbu_spic_trans(code, 0, rxbuf, 1, 3, SPIC_READ_BYTES);
	if (!retval)
		retval = n_rx;

	if (retval != n_rx) {
		printk("%s: ret: %x\n", __func__, retval);
		return retval;
	}

	return retval;
}

static int raspi_read_sr(u8 *val)
{
	return raspi_read_rg(OPCODE_RDSR, val);
}

static int raspi_write_sr(u8 *val)
{
	return raspi_write_rg(OPCODE_WRSR, val);
}

/*
 * Read the status register, returning its value in the location
 */
static int raspi_read_rg(u8 code, u8 *val)
{
	ssize_t retval;

	retval = bbu_spic_trans(code, 0, val, 1, 1, SPIC_READ_BYTES);
	return retval;
}

/*
 * write status register
 */
static int raspi_write_rg(u8 code, u8 *val)
{
	ssize_t retval;
	u32 address = (*val) << 24;

	// put the value to be written in address register, so it will be transfered
	retval = bbu_spic_trans(code, address, val, 2, 0, SPIC_WRITE_BYTES);
	return retval;
}

#if defined(RD_MODE_DUAL) || defined(RD_MODE_QUAD)
static int raspi_write_rg16(u8 code, u8 *val)
{
	ssize_t retval;
	u32 address = (*val) << 24;

	// put the value to be written in address register, so it will be transfered
	address |= (*(val+1)) << 16;
	retval = bbu_spic_trans(code, address, val, 3, 0, SPIC_WRITE_BYTES);
	return retval;
}
#endif

static int raspi_4byte_mode(int enable)
{
	ssize_t retval;
	
	raspi_wait_ready(1);
	
	
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 br, br_cfn; // bank register
		if (enable)
		{
			ra_or(SPI_REG_CTL, 0x3 << 19);
			ra_or(SPI_REG_Q_CTL, 0x3 << 8);
			br = 0x81;
		}
		else
		{
			ra_and(SPI_REG_CTL, ~SPI_CTL_SIZE_MASK);
			ra_or(SPI_REG_CTL, 0x2 << 19);
			ra_and(SPI_REG_Q_CTL, ~(0x3 << 8));
			ra_or(SPI_REG_Q_CTL, 0x2 << 8);
			br = 0x0;
		}
		raspi_write_rg(OPCODE_BRWR, &br);
		raspi_wait_ready(1);
		raspi_read_rg(OPCODE_BRRD, &br_cfn);
		if (br_cfn != br)
		{
			printk("4B mode switch failed %d, %x, %x\n", enable, br_cfn, br);
			return -1;
		}
	}
	else
	{
		u8 code;
		
		code = enable? 0xB7 : 0xE9; /* B7: enter 4B, E9: exit 4B */

		if (enable) {
			ra_or(SPI_REG_CTL, 0x3 << 19);
			ra_or(SPI_REG_Q_CTL, 0x3 << 8);

		}
		else {
			ra_and(SPI_REG_CTL, ~SPI_CTL_SIZE_MASK);
			ra_or(SPI_REG_CTL, 0x2 << 19);
			ra_and(SPI_REG_Q_CTL, ~(0x3 << 8));
			ra_or(SPI_REG_Q_CTL, 0x2 << 8);
		}
		retval = bbu_spic_trans(code, 0, NULL, 1, 0, 0);
		if (retval != 0) {
			printk("%s: ret: %x\n", __func__, retval);
			return -1;
		}
	}

	return 0;
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int raspi_write_enable(void)
{
	u8 code = OPCODE_WREN;
	return bbu_spic_trans(code, 0, NULL, 1, 0, 0);
}

/*
 * Set all sectors (global) unprotected if they are protected.
 * Returns negative if error occurred.
 */
static inline int raspi_unprotect(void)
{
	u8 sr = 0;

	if (raspi_read_sr(&sr) < 0) {
		printk("%s: read_sr fail: %x\n", __func__, sr);
		return -1;
	}

	if ((sr & (SR_BP0 | SR_BP1 | SR_BP2)) != 0) {
		sr = 0;
		raspi_write_sr(&sr);
	}
	return 0;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int raspi_wait_ready(int sleep_ms)
{
	int count;
	int sr = 0;

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < ((sleep_ms+1)*1000*10); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;
		udelay(5);
	}

	printk("%s: read_sr fail: %x\n", __func__, sr);
	return -EIO;
}

static int raspi_wait_sleep_ready(int sleep_ms)
{
	int count;
	int sr = 0;

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < ((sleep_ms+1)*1000); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;
		usleep(50);
	}

	printk("%s: read_sr fail: %x\n", __func__, sr);
	return -EIO;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int raspi_erase_sector(u32 offset)
{
	/* Wait until finished previous write command. */
	if (raspi_wait_ready(10))
		return -EIO;

	/* Send write enable, then erase commands. */
	raspi_write_enable();
	raspi_unprotect();

	if (flash->chip->addr4b)
		raspi_4byte_mode(1);

	raspi_write_enable();
	bbu_spic_trans(STM_OP_SECTOR_ERASE, offset, NULL, 4, 0, 0);
	raspi_wait_sleep_ready(950);
	if (flash->chip->addr4b)
		raspi_4byte_mode(0);

	return 0;
}

/*
 * SPI device driver setup and teardown
 */
struct chip_info *chip_prob(void)
{
	struct chip_info *info, *match;
	u8 buf[5];
	u32 jedec, weight;
	int i;

	raspi_read_devid(buf, 5);
	jedec = (u32)((u32)(buf[1] << 24) | ((u32)buf[2] << 16) | ((u32)buf[3] <<8) | (u32)buf[4]);

	printk("flash manufacture id: %x, device id %x %x\n", buf[0], buf[1], buf[2]);

	// FIXME, assign default as AT25D
	weight = 0xffffffff;
	match = &chips_data[0];
	for (i = 0; i < ARRAY_SIZE(chips_data); i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
			if ((u8)(info->jedec_id >> 24 & 0xff) == buf[1] &&
			    (u8)(info->jedec_id >> 16 & 0xff) == buf[2])
				return info;

			if (weight > (info->jedec_id ^ jedec)) {
				weight = info->jedec_id ^ jedec;
				match = info;
			}
		}
	}

	printk("Warning: un-recognized chip ID, please update SPI driver!\n");

	return match;
}

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int ramtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	u32 addr,len;

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;

	addr = instr->addr;
	len = instr->len;

	mutex_lock(&flash->lock);

	/* now erase those sectors */
	while (len > 0) {
		if (raspi_erase_sector(addr)) {
			instr->state = MTD_ERASE_FAILED;
			mutex_unlock(&flash->lock);
			return -EIO;
		}

		addr += mtd->erasesize;
		len -= mtd->erasesize;
	}

	mutex_unlock(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int ramtd_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	size_t rdlen = 0;

	ra_dbg("%s: from:%x len:%x \n", __func__, from, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

	/* Byte count starts at zero. */
	if (retlen)
		*retlen = 0;

	mutex_lock(&flash->lock);

	/* Wait till previous write/erase is done. */
	if (raspi_wait_ready(1)) {
		/* REVISIT status return?? */
		mutex_unlock(&flash->lock);
		return -EIO;
	}

	if (flash->chip->addr4b)
		raspi_4byte_mode(1);

#ifndef MORE_BUF_MODE
#if defined(RD_MODE_DUAL)
	// serial mode = dual
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 reg[2], cr;
		raspi_read_rg(OPCODE_RDCR, &reg[1]);
		if (reg[1] & (1 << 1))
		{
			reg[1] &= ~(1 << 1);
			raspi_read_sr(&reg[0]);
                        raspi_write_enable();
                        raspi_write_rg16(OPCODE_WRSR, &reg[0]);
			raspi_wait_ready(1);
			raspi_read_rg(OPCODE_RDCR, &cr);
			if (reg[1] != cr)
				printk("warning: set quad failed %x %x\n", reg[1], cr);
		}
	}
	else // MXIC
	{
                u8 sr;
		raspi_read_sr(&sr);
                if ((sr & (1 << 6)))
                {
			u8 get_sr;
                        sr &= ~(1 << 6);
                        raspi_write_enable();
                        raspi_write_sr(&sr);
			raspi_wait_ready(1);
			raspi_read_sr(&get_sr);
			if (get_sr != sr)
				printk("warning: sr write failed %x %x\n", sr, get_sr);
                }
 	}
	ra_and(SPI_REG_MASTER, ~3);
	ra_or(SPI_REG_MASTER, 1);
#elif defined(RD_MODE_QUAD)
	// serial mode = quad
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 reg[2], cr;
		raspi_read_rg(OPCODE_RDCR, &reg[1]);
		if ((reg[1] & (1 << 1)) == 0)
		{
			reg[1] |= (1 << 1);
			raspi_read_sr(&reg[0]);
                        raspi_write_enable();
                        raspi_write_rg16(OPCODE_WRSR, &reg[0]);
			raspi_wait_ready(1); 
			raspi_read_rg(OPCODE_RDCR, &cr);
			if (reg[1] != cr)
				printk("warning: set quad failed %x %x\n", reg[1], cr);
		}
	}
	else // MXIC
	{
                u8 sr, sr2;
		raspi_read_sr(&sr);
		sr2 = sr;
                if ((sr & (1 << 6)) == 0)
                {
			u8 get_sr;
                        sr |= (1 << 6);
                        raspi_write_enable();
                        raspi_write_sr(&sr);
			raspi_wait_ready(1);
			raspi_read_sr(&get_sr);
			if (get_sr != sr)
				printk("warning: quad sr write failed %x %x %x\n", sr, get_sr, sr2);
			
                }
 	}
	ra_and(SPI_REG_MASTER, ~3);
	ra_or(SPI_REG_MASTER, 2);
#endif
#endif

	do {
		int rc, more;
#ifdef MORE_BUF_MODE
		more = 32;
#else
		more = 4;
#endif
		if (len - rdlen <= more) {
#ifdef MORE_BUF_MODE
			rc = bbu_mb_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 0, (len-rdlen), SPIC_READ_BYTES);
#else
#if defined(RD_MODE_DOR)
			rc = bbu_spic_trans(OPCODE_DOR, from, (buf+rdlen), 5, (len-rdlen), SPIC_READ_BYTES);
#elif defined(RD_MODE_DIOR)
			rc = bbu_spic_trans(OPCODE_DIOR, from, (buf+rdlen), 5, (len-rdlen), SPIC_READ_BYTES);
#elif defined(RD_MODE_QOR)
			rc = bbu_spic_trans(OPCODE_QOR, from, (buf+rdlen), 5, (len-rdlen), SPIC_READ_BYTES);
#elif defined(RD_MODE_QIOR)
			rc = bbu_spic_trans(OPCODE_QIOR, from, (buf+rdlen), 7, (len-rdlen), SPIC_READ_BYTES);
#else
			rc = bbu_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 4, (len-rdlen), SPIC_READ_BYTES);
#endif
#endif
			if (rc != 0) {
				printk("%s: failed\n", __func__);
				break;
			}
			rdlen = len;
		}
		else {
#ifdef MORE_BUF_MODE
			rc = bbu_mb_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 0, more, SPIC_READ_BYTES);
#else
#if defined(RD_MODE_DOR)
			rc = bbu_spic_trans(OPCODE_DOR, from, (buf+rdlen), 5, more, SPIC_READ_BYTES);
#elif defined(RD_MODE_DIOR)
			rc = bbu_spic_trans(OPCODE_DIOR, from, (buf+rdlen), 5, more, SPIC_READ_BYTES);
#elif defined(RD_MODE_QOR)
			rc = bbu_spic_trans(OPCODE_QOR, from, (buf+rdlen), 5, more, SPIC_READ_BYTES);
#elif defined(RD_MODE_QIOR)
			rc = bbu_spic_trans(OPCODE_QIOR, from, (buf+rdlen), 7, more, SPIC_READ_BYTES);
#else
			rc = bbu_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 4, more, SPIC_READ_BYTES);
#endif
#endif
			if (rc != 0) {
				printk("%s: failed\n", __func__);
				break;
			}
			rdlen += more;
			from += more;
		}
	} while (rdlen < len);

#ifndef MORE_BUF_MODE
#if defined(RD_MODE_DUAL) || defined(RD_MODE_QUAD)
	// serial mode = normal
	ra_and(SPI_REG_MASTER, ~3);
#if defined(RD_MODE_QUAD)
	// serial mode = quad
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 reg[2], cr;
		raspi_read_rg(OPCODE_RDCR, &reg[1]);
		if (reg[1] & (1 << 1))
		{
			reg[1] &= ~(1 << 1);
			raspi_read_sr(&reg[0]);
                        raspi_write_enable();
                        raspi_write_rg16(OPCODE_WRSR, &reg[0]);
			raspi_wait_ready(1);
			raspi_read_rg(OPCODE_RDCR, &cr);
			if (reg[1] != cr)
				printk("warning: set quad failed %x %x\n", reg[1], cr);
		}
	}
	else // MXIC
	{
                u8 sr;
		raspi_read_sr(&sr);
                if ((sr & (1 << 6)))
                {
                        sr &= ~(1 << 6);
                        raspi_write_enable();
                        raspi_write_sr(&sr);
			raspi_wait_ready(1);
                }
 	}

#endif
#endif
#endif // MORE_BUF_MODE

	if (flash->chip->addr4b)
		raspi_4byte_mode(0);

	mutex_unlock(&flash->lock);

	if (retlen)
		*retlen = rdlen;

	if (rdlen != len) 
		return -EIO;

	return 0;
}

inline int ramtd_lock (struct mtd_info *mtd, loff_t to, uint64_t len)
{
	return 0; // Not all vendor support lock/unlock cmd
}

inline int ramtd_unlock (struct mtd_info *mtd, loff_t to, uint64_t len)
{
	return 0; // Not all vendor support lock/unlock cmd
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int ramtd_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	u32 page_offset, page_size;
	int rc = 0;
	int wrto, wrlen, more;
	char *wrbuf;
	int count = 0;

	ra_dbg("%s: to:%x len:%x \n", __func__, to, len);
	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (len == 0)
		return 0;
	if (to + len > flash->mtd.size)
		return -EINVAL;

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(2)) {
		mutex_unlock(&flash->lock);
		return -1;
	}

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	if (flash->chip->addr4b)
		raspi_4byte_mode(1);

	/* write everything in PAGESIZE chunks */
	while (len > 0) {
		page_size = min_t(size_t, len, FLASH_PAGESIZE-page_offset);
		page_offset = 0;

		/* write the next page to flash */
		raspi_wait_ready(3);
		raspi_write_enable();
		raspi_unprotect();

		wrto = to;
		wrlen = page_size;
		wrbuf = (char *)buf;
		rc = wrlen;
		do {
#ifdef MORE_BUF_MODE
			more = 32;
#else
			more = 4;
#endif
			if (wrlen <= more) {
#ifdef MORE_BUF_MODE
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, wrlen, 0, SPIC_WRITE_BYTES);
#else
				bbu_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, wrlen+4, 0, SPIC_WRITE_BYTES);
#endif
				wrlen = 0;
			}
			else {
#ifdef MORE_BUF_MODE
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, more, 0, SPIC_WRITE_BYTES);
#else
				bbu_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, more+4, 0, SPIC_WRITE_BYTES);
#endif
				wrto += more;
				wrlen -= more;
				wrbuf += more;
			}
			if (wrlen > 0) {
				raspi_wait_ready(100);
				raspi_write_enable();
			}
		} while (wrlen > 0);

		if (rc > 0) {
			if (retlen)
				*retlen += rc;
			if (rc < page_size) {
				mutex_unlock(&flash->lock);
				printk("%s: rc:%x return:%x page_size:%x \n", 
				       __func__, rc, rc, page_size);
				return -EIO;
			}
		}

		len -= page_size;
		to += page_size;
		buf += page_size;
		count++;
		if ((count & 0xf) == 0)
			raspi_wait_sleep_ready(1);
			
	}

	raspi_wait_ready(100);

	if (flash->chip->addr4b)
		raspi_4byte_mode(0);

	mutex_unlock(&flash->lock);

	return 0;
}

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int raspi_probe(void)
{
	struct chip_info *chip;
#if defined (SPI_DEBUG)
	unsigned i;
#endif
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	loff_t offs;
	size_t len_ret;
	struct __image_header {
		uint8_t unused[60];
		uint32_t ih_ksz;
	} hdr;
#endif

	if (ra_check_flash_type() != BOOT_FROM_SPI)
		return 0;

	spic_init();

	chip = chip_prob();

	flash = kzalloc(sizeof(*flash), GFP_KERNEL);
	if (!flash)
		return -ENOMEM;

	mutex_init(&flash->lock);

	flash->chip = chip;
	flash->mtd.name = "raspi";

	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = 1;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = chip->sector_size * chip->n_sectors;
	flash->mtd.erasesize = chip->sector_size;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
	flash->mtd._erase = ramtd_erase;
	flash->mtd._read = ramtd_read;
	flash->mtd._write = ramtd_write;
	flash->mtd._lock = ramtd_lock;
	flash->mtd._unlock = ramtd_unlock;
#else
	flash->mtd.erase = ramtd_erase;
	flash->mtd.read = ramtd_read;
	flash->mtd.write = ramtd_write;
	flash->mtd.lock = ramtd_lock;
	flash->mtd.unlock = ramtd_unlock;
#endif

	printk("%s (%02x %04x) (%u Kbytes)\n",
	       chip->name, chip->id, chip->jedec_id, (uint32_t)flash->mtd.size / 1024);

#if defined (SPI_DEBUG)
	ra_dbg("mtd .name = %s, .size = 0x%.8x (%uM) "
			".erasesize = 0x%.8x (%uK) .numeraseregions = %d\n",
		flash->mtd.name,
		(uint32_t)flash->mtd.size,
		(uint32_t)flash->mtd.size / (1024*1024),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			ra_dbg("mtd.eraseregions[%d] = { .offset = 0x%.8x, "
				".erasesize = 0x%.8x (%uK), "
				".numblocks = %d }\n",
				i, (uint32_t)flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);
#endif

#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	offs = MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE;
	ramtd_read(NULL, offs, sizeof(hdr), &len_ret, (u_char *)(&hdr));
	if (hdr.ih_ksz != 0) {
		rt2880_partitions[3].size = ntohl(hdr.ih_ksz);
		rt2880_partitions[4].size = IMAGE1_SIZE - (MTD_BOOT_PART_SIZE +
				MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE +
				MTD_STORE_PART_SIZE +
				ntohl(hdr.ih_ksz));
	}
#endif

	return mtd_device_register(&flash->mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
}

static int __init raspi_init(void)
{
	return raspi_probe();
}

static void __exit raspi_exit(void)
{
	if (flash) {
		mtd_device_unregister(&flash->mtd);
		kfree(flash);
		flash = NULL;
	}
}

module_init(raspi_init);
module_exit(raspi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven Liu");
MODULE_DESCRIPTION("Ralink MTD SPI driver for flash chips");
