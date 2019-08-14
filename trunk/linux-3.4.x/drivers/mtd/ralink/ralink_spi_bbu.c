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

#include <ralink/ralink_gpio.h>

#include "ralink-flash.h"
#include "ralink-flash-map.h"
#include "ralink_spi_bbu.h"

//#define SPI_DEBUG
//#define TEST_CS1_FLASH

#if defined (CONFIG_MTD_SPI_READ_FAST)
#define RD_MODE_FAST
#endif

/* DUAL/QUAD and MORE_BUF_MODE can't be enabled together! */

#if defined (CONFIG_MTD_SPI_READ_QOR)
#define RD_MODE_QOR
#elif defined (CONFIG_MTD_SPI_READ_QIOR)
#define RD_MODE_QIOR
#elif defined (CONFIG_MTD_SPI_READ_DOR)
#define RD_MODE_DOR
#elif defined (CONFIG_MTD_SPI_READ_DIOR)
#define RD_MODE_DIOR
#else
#define MORE_BUF_MODE
#endif

#ifdef MORE_BUF_MODE
#define SPI_BBU_MAX_XFER	32
#else
#define SPI_BBU_MAX_XFER	4
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
#define OPCODE_FAST_READ	0x0B	/* Read data bytes */
#define OPCODE_DOR		0x3B	/* Dual Output Read */
#define OPCODE_QOR		0x6B	/* Quad Output Read */
#define OPCODE_DIOR		0xBB	/* Dual IO High Performance Read */
#define OPCODE_QIOR		0xEB	/* Quad IO High Performance Read */

/* Status Register bits. */
#define SR_WIP			0x01	/* Write in progress */
#define SR_WEL			0x02	/* Write enable latch */
#define SR_BP0			0x04	/* Block protect 0 */
#define SR_BP1			0x08	/* Block protect 1 */
#define SR_BP2			0x10	/* Block protect 2 */
#define SR_BP3			0x20	/* Block protect 3 */
#define SR_EPE			0x20	/* Erase/Program error */
#define SR_SRWD			0x80	/* SR write protect */

#define OPCODE_BRRD		0x16
#define OPCODE_BRWR		0x17
#define OPCODE_RDCR		0x35

extern u32 get_surfboard_sysclk(void);

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
	u32 clk_sys, clk_div, reg;

	clk_sys = get_surfboard_sysclk() / 1000000;
#if defined (CONFIG_RALINK_MT7621)
	/* MT7621 sys_clk 220 MHz */
#if defined (CONFIG_MTD_SPI_FAST_CLOCK)
	clk_div = 5;	/* hclk/5 -> 44.0 MHz */
#else
	clk_div = 7;	/* hclk/7 -> 31.4 MHz */
#endif
#elif defined (CONFIG_RALINK_MT7628)
	/* MT7628 sys_clk 193/191 MHz */
#if defined (CONFIG_MTD_SPI_FAST_CLOCK)
	clk_div = 4;	/* hclk/4 -> 48.3 MHz */
#else
	clk_div = 6;	/* hclk/6 -> 32.2 MHz */
#endif
#endif
	reg = ra_inl(SPI_REG_MASTER);
	reg &= ~(0x7);
	reg &= ~(0x0fff << 16);
	reg |= ((clk_div - 2) << 16);
	ra_outl(SPI_REG_MASTER, reg);

#ifdef TEST_CS1_FLASH
#if defined (CONFIG_RALINK_MT7628)
	ra_and(RALINK_REG_GPIOMODE, ~(3 << 4));
#endif
	ra_or(SPI_REG_MASTER, (1 << 29));
#endif

	printk("MediaTek SPI flash driver, SPI clock: %dMHz\n", clk_sys / clk_div);
}

struct chip_info {
	char		*name;
	u8		id;
	u32		jedec_id;
	unsigned int	sector_size;
	unsigned int	n_sectors;
	char		addr4b;
};

#include "flash_ids.h"

struct flash_info {
	struct mutex		lock;
	struct mtd_info		mtd;
	struct chip_info	*chip;
};

static struct flash_info *flash = NULL;

#ifdef MORE_BUF_MODE
static int bbu_mb_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg_mb, reg_ctl, reg_opcode, reg_data;
	int i, q, r;

	if (flag != SPIC_READ_BYTES && flag != SPIC_WRITE_BYTES)
		return -1;

	if (!flash)
		return -1;

	bbu_spic_busy_wait();

	reg_ctl = ra_inl(SPI_REG_CTL);
	reg_ctl &= ~SPI_CTL_TXRXCNT_MASK;
	reg_ctl &= ~SPI_CTL_ADDREXT_MASK;

	/* step 1. set opcode & address */
	if (flash->chip->addr4b) {
		reg_ctl |= ((code << 24) & SPI_CTL_ADDREXT_MASK);
		reg_opcode = addr;
	} else {
		reg_opcode = (code << 24) | (addr & 0xffffff);
	}

	ra_outl(SPI_REG_OPCODE, reg_opcode);

	reg_mb = ra_inl(SPI_REG_MOREBUF);
	reg_mb &= ~SPI_MBCTL_TXRXCNT_MASK;
	reg_mb &= ~SPI_MBCTL_CMD_MASK;

	/* step 2. set cmd bit count to 32 (or 40) */
	if (flash->chip->addr4b)
		reg_mb |= ((5 << 3) << 24);
	else
		reg_mb |= ((4 << 3) << 24);

	/* step 3. set rx (miso_bit_cnt) and tx (mosi_bit_cnt) bit count */
	reg_mb |= ((n_rx << 3) << 12);
	reg_mb |=  (n_tx << 3);

	ra_outl(SPI_REG_MOREBUF, reg_mb);

#if defined(RD_MODE_FAST)
	/* clear data bit for dummy bits in Fast IO Read */
	if (flag & SPIC_READ_BYTES)
		ra_outl(SPI_REG_DATA0, 0);
#endif

	/* step 4. write DI/DO data #0 ~ #7 */
	if (flag & SPIC_WRITE_BYTES) {
		if (!buf)
			return -1;
		reg_data = 0;
		for (i = 0; i < n_tx; i++) {
			r = i % 4;
			if (r == 0)
				reg_data = 0;
			reg_data |= (*(buf + i) << (r * 8));
			if (r == 3 || (i+1) == n_tx) {
				q = i / 4;
				ra_outl(SPI_REG_DATA(q), reg_data);
			}
		}
	}

	/* step 5. kick */
	ra_outl(SPI_REG_CTL, reg_ctl | SPI_CTL_START);

	/* step 6. wait spi_master_busy */
	bbu_spic_busy_wait();

	/* step 7. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (!buf)
			return -1;
		reg_data = 0;
		for (i = 0; i < n_rx; i++) {
			r = i % 4;
			if (r == 0) {
				q = i / 4;
				reg_data = ra_inl(SPI_REG_DATA(q));
			}
			*(buf + i) = (u8)(reg_data >> (r * 8));
		}
	}

	return 0;
}
#endif // MORE_BUF_MODE //

static int bbu_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag)
{
	int addr4b = 0;
	u32 reg_ctl, reg_opcode, reg_data;

	bbu_spic_busy_wait();

	reg_ctl = ra_inl(SPI_REG_CTL);
	reg_ctl &= ~SPI_CTL_TXRXCNT_MASK;
	reg_ctl &= ~SPI_CTL_ADDREXT_MASK;

	if ((reg_ctl & SPI_CTL_SIZE_MASK) == SPI_CTL_SIZE_MASK)
		addr4b = 1;

	/* step 1. set opcode & address */
	if (flash && flash->chip->addr4b && addr4b)
		reg_ctl |= (addr & SPI_CTL_ADDREXT_MASK);

	reg_opcode = ((addr & 0xffffff) << 8) | code;

#if defined(RD_MODE_QIOR) || defined(RD_MODE_QOR) || defined(RD_MODE_DIOR) || defined(RD_MODE_DOR) || defined(RD_MODE_FAST)
	/* clear data bit for dummy bits in Quad/Dual/Fast IO Read */
	if (flag & SPIC_READ_BYTES)
		ra_outl(SPI_REG_DATA0, 0);
#endif

	/* step 2. write DI/DO data #0 */
	if (flag & SPIC_WRITE_BYTES) {
		if (!buf)
			return -1;
		reg_data = 0;
		switch (n_tx) {
		case 8:
			reg_data |= (*(buf+3) << 24);
		case 7:
			reg_data |= (*(buf+2) << 16);
		case 6:
			reg_data |= (*(buf+1) << 8);
		case 5:
			reg_data |= *buf;
			break;
#if defined(RD_MODE_QIOR) || defined(RD_MODE_QOR)
		case 3:
			reg_opcode &= 0xff;
			if (flash->chip->addr4b && addr4b) {
				reg_ctl &= ~SPI_CTL_ADDREXT_MASK;
				reg_ctl |= (*buf << 24);
				
				reg_opcode |= (*(buf+1) << 24);
			} else {
				reg_opcode |= (*buf << 24);
				reg_opcode |= (*(buf+1) << 16);
			}
			break;
#endif
		case 2:
			reg_opcode &= 0xff;
			if (flash->chip->addr4b && addr4b) {
				reg_ctl &= ~SPI_CTL_ADDREXT_MASK;
				reg_ctl |= (*buf << 24);
			} else {
				reg_opcode |= (*buf << 24);
			}
			break;
		default:
			printk("%s: not support write of length %d\n", __func__, n_tx);
			return -1;
		}
		
		ra_outl(SPI_REG_DATA0, reg_data);
	}

	ra_outl(SPI_REG_OPCODE, reg_opcode);

	/* step 3. set mosi_byte_cnt */
	reg_ctl |= (n_rx << 4);
	if (flash && flash->chip->addr4b && addr4b && n_tx >= 4)
		reg_ctl |= (n_tx + 1);
	else
		reg_ctl |= n_tx;

	/* step 4. kick */
	ra_outl(SPI_REG_CTL, reg_ctl | SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();

	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (!buf)
			return -1;
		reg_data = ra_inl(SPI_REG_DATA0);
		switch (n_rx) {
		case 4:
			*(buf+3) = (u8)(reg_data >> 24);
		case 3:
			*(buf+2) = (u8)(reg_data >> 16);
		case 2:
			*(buf+1) = (u8)(reg_data >> 8);
		case 1:
			*buf = (u8)reg_data;
			break;
		default:
			printk("%s:  read of length %d\n", __func__, n_rx);
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
	return bbu_spic_trans(OPCODE_WREN, 0, NULL, 1, 0, 0);
}

static inline int raspi_write_disable(void)
{
	return bbu_spic_trans(OPCODE_WRDI, 0, NULL, 1, 0, 0);
}

/*
 * Read the status register, returning its value in the location
 */
static inline int raspi_read_rg(u8 code, u8 *val)
{
	return bbu_spic_trans(code, 0, val, 1, 1, SPIC_READ_BYTES);
}

/*
 * write status register
 */
static int raspi_write_rg(u8 code, u8 *val)
{
	u32 address = (*val) << 24;

	// put the value to be written in address register, so it will be transfered
	return bbu_spic_trans(code, address, val, 2, 0, SPIC_WRITE_BYTES);
}

/*
 * read SPI flash device ID
 */
static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
	int retval;

	retval = bbu_spic_trans(OPCODE_RDID, 0, rxbuf, 1, 4, SPIC_READ_BYTES);
	if (retval)
		printk("%s: ret: %x\n", __func__, retval);

	return retval;
}

static inline int raspi_read_sr(u8 *val)
{
	return raspi_read_rg(OPCODE_RDSR, val);
}

static inline int raspi_write_sr(u8 *val)
{
	return raspi_write_rg(OPCODE_WRSR, val);
}

static int raspi_wait_ready(int sleep_ms);

#if defined(RD_MODE_QIOR) || defined(RD_MODE_QOR)
static int raspi_write_rg16(u8 code, u8 *val)
{
	ssize_t retval;
	u32 address = (*val) << 24;

	// put the value to be written in address register, so it will be transfered
	address |= (*(val+1)) << 16;
	retval = bbu_spic_trans(code, address, val, 3, 0, SPIC_WRITE_BYTES);
	return retval;
}

static int raspi_set_quad(void)
{
	if ((flash->chip->id == 0x01) || (flash->chip->id == 0xef)) // Spansion or WinBond
	{
		u8 reg[2] = {0}, cr = 0;
		raspi_read_rg(OPCODE_RDCR, &reg[1]);
		if ((reg[1] & (1 << 1)) == 0)
		{
			reg[1] |= (1 << 1);
			raspi_read_sr(&reg[0]);
			raspi_write_enable();
			raspi_write_rg16(OPCODE_WRSR, reg);
			raspi_wait_ready(1);
			raspi_read_rg(OPCODE_RDCR, &cr);
			if (reg[1] != cr)
				printk("warning: set quad failed %x %x\n", reg[1], cr);
		}
	}
	else // MXIC
	{
		u8 sr = 0, sr2;
		raspi_read_sr(&sr);
		sr2 = sr;
		if ((sr & (1 << 6)) == 0)
		{
			u8 get_sr = 0;
			sr |= (1 << 6);
			raspi_write_enable();
			raspi_write_sr(&sr);
			raspi_wait_ready(1);
			raspi_read_sr(&get_sr);
			if (get_sr != sr)
				printk("warning: quad sr write failed %x %x %x\n", sr, get_sr, sr2);
		}
	}
}
#endif

static int raspi_4byte_mode(int enable)
{
	int retval;
	u32 reg_ctl, reg_qctl;

	raspi_wait_ready(1);

	reg_ctl = ra_inl(SPI_REG_CTL);
	reg_qctl = ra_inl(SPI_REG_Q_CTL);

	if (enable) {
		reg_ctl |= SPI_CTL_SIZE_MASK;
		reg_qctl |= SPI_QCTL_FSADSZ_MASK;
	} else {
		reg_ctl &= ~SPI_CTL_SIZE_MASK;
		reg_ctl |= (0x2 << 19);
		reg_qctl &= ~SPI_QCTL_FSADSZ_MASK;
		reg_qctl |= (0x2 << 8);
	}

	ra_outl(SPI_REG_CTL, reg_ctl);
	ra_outl(SPI_REG_Q_CTL, reg_qctl);

	if (flash->chip->id == 0x1) // Spansion
	{
		u8 br, br_cfn; // bank register
		
		br = (enable)? 0x81 : 0x0;
		raspi_write_rg(OPCODE_BRWR, &br);
		raspi_wait_ready(1);
		raspi_read_rg(OPCODE_BRRD, &br_cfn);
		if (br_cfn != br) {
			printk("%s: 4B mode set failed!\n", __func__);
			return -1;
		}
	}
	else
	{
		u8 code;
		
		code = (enable)? 0xB7 : 0xE9; /* B7: enter 4B, E9: exit 4B */
		retval = bbu_spic_trans(code, 0, NULL, 1, 0, 0);
		
		// for Winbond's W25Q256FV, need to clear extend address register
		if ((!enable) && (flash->chip->id == 0xef)) {
			code = 0x0;
			raspi_write_enable();
			raspi_write_rg(0xc5, &code);
		}
		
		if (retval != 0) {
			printk("%s: 4B mode set failed!\n", __func__);
			return -1;
		}
	}

	return 0;
}

#if defined (CONFIG_MTD_SPI_FAST_CLOCK)
static void raspi_drive_strength(void)
{
	u8 code = 0;

	if (flash->chip->id == 0xef) {
		/* set Winbond DVP[1:0] as 10 (driving strength 50%) */
		if (raspi_read_rg(0x15, &code) == 0) {
			/* Winbond DVP[1:0] is 11 by default (driving strength 25%) */
			if ((code & 0x60) == 0x60) {
				code &= ~0x60;
				code |= 0x40;
				raspi_write_enable();
				raspi_write_rg(0x11, &code);
			}
		}
	}
}
#endif

/*
 * Set all sectors (global) unprotected if they are protected.
 * Returns negative if error occurred.
 */
static int raspi_unprotect(void)
{
	u8 sr_bp, sr = 0;

	if (raspi_read_sr(&sr) < 0) {
		printk("%s: read_sr fail: %x\n", __func__, sr);
		return -1;
	}

	sr_bp = SR_BP0 | SR_BP1 | SR_BP2;
	if (flash->chip->addr4b)
		sr_bp |= SR_BP3;

	if ((sr & sr_bp) != 0) {
		sr = 0;
		raspi_write_enable();
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
	bbu_spic_trans(OPCODE_SE, offset, NULL, 4, 0, 0);
	raspi_wait_sleep_ready(950);

	return 0;
}

/*
 * SPI device driver setup and teardown
 */
struct chip_info *chip_prob(void)
{
	struct chip_info *info;
	u8 buf[4] = {0};
	u32 jedec;
	int i, table_size;

	raspi_read_devid(buf, 4);
	jedec = (u32)((u32)(buf[1] << 24) | ((u32)buf[2] << 16) | ((u32)buf[3] << 8));

	ra_dbg("deice id : %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

	table_size = ARRAY_SIZE(chips_data);

	for (i = 0; i < table_size; i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
			if ((info->jedec_id & 0xffff0000) == (jedec & 0xffff0000))
				return info;
		}
	}

	printk(KERN_WARNING "unrecognized SPI chip ID: %x (%x), please update the SPI driver!\n",
		buf[0], jedec);

	/* use last stub item */
	return &chips_data[table_size - 1];
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
	u32 addr, len;
	int exit_code = 0;

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;

	addr = instr->addr;
	len = instr->len;

	mutex_lock(&flash->lock);

	/* wait until finished previous command. */
	if (raspi_wait_ready(10)) {
		instr->state = MTD_ERASE_FAILED;
		mutex_unlock(&flash->lock);
		return -EIO;
	}

	raspi_unprotect();

	if (flash->chip->addr4b)
		raspi_4byte_mode(1);

	/* now erase those sectors */
	while (len > 0) {
		if (raspi_erase_sector(addr)) {
			exit_code = -EIO;
			break;
		}
		addr += mtd->erasesize;
		len -= mtd->erasesize;
	}

	if (flash->chip->addr4b)
		raspi_4byte_mode(0);

	instr->state = (exit_code == 0) ? MTD_ERASE_DONE : MTD_ERASE_FAILED;

	mutex_unlock(&flash->lock);

	if (exit_code == 0)
		mtd_erase_callback(instr);

	return exit_code;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int ramtd_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	int rc;
	size_t rdlen = 0;
	u32 reg_master;
#ifdef MORE_BUF_MODE
#if defined(RD_MODE_FAST)
	u8 code = OPCODE_FAST_READ;
	size_t n_tx = 1;
#else
	u8 code = OPCODE_READ;
	size_t n_tx = 0;
#endif
#else
#if defined(RD_MODE_DOR)
	u8 code = OPCODE_DOR;
	size_t n_tx = 5;
#elif defined(RD_MODE_DIOR)
	u8 code = OPCODE_DIOR;
	size_t n_tx = 5;
#elif defined(RD_MODE_QOR)
	u8 code = OPCODE_QOR;
	size_t n_tx = 5;
#elif defined(RD_MODE_QIOR)
	u8 code = OPCODE_QIOR;
	size_t n_tx = 7;
#elif defined(RD_MODE_FAST)
	u8 code = OPCODE_FAST_READ;
	size_t n_tx = 5;
#else
	u8 code = OPCODE_READ;
	size_t n_tx = 4;
#endif
#endif

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
		mutex_unlock(&flash->lock);
		return -EIO;
	}

	if (flash->chip->addr4b)
		raspi_4byte_mode(1);

	reg_master = ra_inl(SPI_REG_MASTER);
	reg_master &= ~(0x7);

#ifdef MORE_BUF_MODE
	/* SPI mode = more byte mode */
	ra_outl(SPI_REG_MASTER, (reg_master | 0x4));
#else
#if defined(RD_MODE_DIOR) || defined(RD_MODE_DOR)
	/* SPI mode = dual mode */
	ra_outl(SPI_REG_MASTER, (reg_master | 0x1));
#elif defined(RD_MODE_QIOR) || defined(RD_MODE_QOR)
	/* SPI mode = quad mode */
	raspi_set_quad();
	ra_outl(SPI_REG_MASTER, (reg_master | 0x2));
#endif
#endif

	while (rdlen < len) {
		size_t r_part = len - rdlen;
		if (r_part > SPI_BBU_MAX_XFER)
			r_part = SPI_BBU_MAX_XFER;
#ifdef MORE_BUF_MODE
		rc = bbu_mb_spic_trans(code, from, (buf+rdlen), n_tx, r_part, SPIC_READ_BYTES);
#else
		rc = bbu_spic_trans(code, from, (buf+rdlen), n_tx, r_part, SPIC_READ_BYTES);
#endif
		if (rc != 0) {
			printk("%s: failed\n", __func__);
			break;
		}
		from += r_part;
		rdlen += r_part;
	}

	/* SPI mode = normal */
	ra_outl(SPI_REG_MASTER, reg_master);

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
	u32 page_offset, page_size, reg_master;
	int rc = 0, exit_code = 0;
	int wrto, wrlen;
	char *wrbuf;
	int count = 0;

	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (len == 0)
		return 0;

	if (to + len > flash->mtd.size)
		return -EINVAL;

	mutex_lock(&flash->lock);

	/* wait until finished previous write command. */
	if (raspi_wait_ready(2)) {
		mutex_unlock(&flash->lock);
		return -EIO;
	}

	raspi_unprotect();

	if (flash->chip->addr4b)
		raspi_4byte_mode(1);

	reg_master = ra_inl(SPI_REG_MASTER);
	reg_master &= ~(0x7);

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* write everything in PAGESIZE chunks */
	while (len > 0) {
		page_size = min_t(size_t, len, FLASH_PAGESIZE-page_offset);
		page_offset = 0;
		
		/* write the next page to flash */
		wrto = to;
		wrlen = page_size;
		wrbuf = (char *)buf;
		
		while (wrlen > 0) {
			int w_part = (wrlen > SPI_BBU_MAX_XFER) ? SPI_BBU_MAX_XFER : wrlen;
			
			raspi_wait_ready(100);
			raspi_write_enable();
#ifdef MORE_BUF_MODE
			ra_outl(SPI_REG_MASTER, (reg_master | 0x4));
			rc = bbu_mb_spic_trans(OPCODE_PP, wrto, wrbuf, w_part, 0, SPIC_WRITE_BYTES);
			ra_outl(SPI_REG_MASTER, reg_master);
#else
			rc = bbu_spic_trans(OPCODE_PP, wrto, wrbuf, w_part+4, 0, SPIC_WRITE_BYTES);
#endif
			if (rc != 0)
				break;
			
			wrlen -= w_part;
			wrto  += w_part;
			wrbuf += w_part;
		}

		rc = page_size - wrlen;
		if (rc > 0) {
			if (retlen)
				*retlen += rc;
			if (rc < page_size) {
				exit_code = -EIO;
				printk("%s: rc:%x return:%x page_size:%x \n", 
				       __func__, rc, rc, page_size);
				goto exit_mtd_write;
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

exit_mtd_write:

	if (flash->chip->addr4b)
		raspi_4byte_mode(0);

	mutex_unlock(&flash->lock);

	return exit_code;
}

static int __init raspi_init(void)
{
	struct chip_info *chip;
	uint64_t flash_size = IMAGE1_SIZE;
	uint32_t kernel_size = 0x150000;
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	_ihdr_t hdr;
	loff_t offs;
	size_t ret_len = 0;
#endif
#if defined (SPI_DEBUG)
	unsigned i;
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

#if defined (CONFIG_MTD_SPI_FAST_CLOCK)
	/* tune flash chip output driving strength */
	raspi_drive_strength();
#endif

	printk("SPI flash chip: %s (%02x %04x) (%u Kbytes)\n",
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

#if defined (CONFIG_RT2880_FLASH_AUTO)
	flash_size = flash->mtd.size;
#endif
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	offs = MTD_KERNEL_PART_OFFSET;
	memset(&hdr, 0, sizeof(hdr));
	ramtd_read(NULL, offs, sizeof(hdr), &ret_len, (u_char *)(&hdr));
	if (ret_len == sizeof(hdr) && hdr.ih_ksz != 0)
		kernel_size = ntohl(hdr.ih_ksz);
#endif

	/* calculate partition table */
	recalc_partitions(flash_size, kernel_size);

	/* register the partitions */
	return mtd_device_register(&flash->mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
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
MODULE_DESCRIPTION("MediaTek MTD SPI driver for flash chips");
