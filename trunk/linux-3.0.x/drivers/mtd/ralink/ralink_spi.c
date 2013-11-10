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

#include "ralink_spi.h"
#include "ralink-flash.h"
#include "ralink-flash-map.h"

extern u32 get_surfboard_sysclk(void);

/******************************************************************************
 * SPI FLASH elementray definition and function
 ******************************************************************************/

#define FLASH_PAGESIZE		256

/* Flash opcodes. */
#define OPCODE_WREN		6	/* Write enable */
#define OPCODE_RDSR		5	/* Read status register */
#define OPCODE_WRDI		4	/* Write disable */
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
#define OPCODE_READ_ID		0x90	/* Read Manufacturer and Device ID */

#define OPCODE_P4E		0x20	/* 4KB Parameter Sectore Erase */
#define OPCODE_P8E		0x40	/* 8KB Parameter Sectore Erase */
#define OPCODE_BE		0x60	/* Bulk Erase */
#define OPCODE_BE1		0xC7	/* Bulk Erase */
#define OPCODE_QPP		0x32	/* Quad Page Programing */

#define OPCODE_CLSR		0x30
#define OPCODE_RCR		0x35	/* Read Configuration Register */

// Bank register read/write
#define OPCODE_BRRD		0x16
#define OPCODE_BRWR		0x17

/* Status Register bits. */
#define SR_WIP			1	/* Write in progress */
#define SR_WEL			2	/* Write enable latch */
#define SR_BP0			4	/* Block protect 0 */
#define SR_BP1			8	/* Block protect 1 */
#define SR_BP2			0x10	/* Block protect 2 */
#define SR_EPE			0x20	/* Erase/Program error */
#define SR_SRWD			0x80	/* SR write protect */

#if defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7621)

#define COMMAND_MODE
#define SPI_FIFO_SIZE		16
#define CFG_CLK_DIV		SPICFG_SPICLK_DIV8

#if defined(RD_MODE_QOR) || defined(RD_MODE_QIOR)
#define RD_MODE_QUAD
#endif

#else

//#define MX_FAST_READ		1

#ifdef CONFIG_MTD_SPI_FAST_CLOCK
#define CFG_CLK_DIV		SPICFG_SPICLK_DIV4 /* 166/4 = 41.0 MHz */
#else
#define CFG_CLK_DIV		SPICFG_SPICLK_DIV8 /* 166/8 = 20.5 MHz */
#endif


#endif

static unsigned int spi_wait_nsec = 150;

//#define SPI_DEBUG
#if !defined (SPI_DEBUG)

#define ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))
#define ra_dbg(args...) do {} while(0)
//#define ra_dbg(args...) do { if (1) printk(args); } while(0)

#else

int ranfc_debug = 1;
#define ra_dbg(args...) do { if (ranfc_debug) printk(args); } while(0)
#define _ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define _ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))

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

#endif // SPI_DEBUG

#define ra_aor(addr, a_mask, o_value)  ra_outl(addr, (ra_inl(addr) & (a_mask)) | (o_value))

#define ra_and(addr, a_mask)  ra_aor(addr, a_mask, 0)
#define ra_or(addr, o_value)  ra_aor(addr, -1, o_value)

static void usleep(unsigned int usecs)
{
	unsigned long timeout = usecs_to_jiffies(usecs);

	while (timeout)
		timeout = schedule_timeout_interruptible(timeout);
}

static int spic_busy_wait(void)
{
	do {
		if ((ra_inl(RT2880_SPISTAT_REG) & 0x01) == 0)
			return 0;
	} while (spi_wait_nsec >> 1);

	printk("%s: fail \n", __func__);
	return -1;
}

#define SPIC_READ_BYTES		(1<<0)
#define SPIC_WRITE_BYTES	(1<<1)
#define SPIC_USER_MODE		(1<<2)
#define SPIC_4B_ADDR		(1<<3)

#if !defined (COMMAND_MODE)
/*
 * @cmd: command and address
 * @n_cmd: size of command, in bytes
 * @buf: buffer into which data will be read/written
 * @n_buf: size of buffer, in bytes
 * @flag: tag as READ/WRITE
 *
 * @return: if write_onlu, -1 means write fail, or return writing counter.
 * @return: if read, -1 means read fail, or return reading counter.
 */
static int spic_transfer(const u8 *cmd, int n_cmd, u8 *buf, int n_buf, int flag)
{
	int retval = -1;

	ra_dbg("cmd(%x): %x %x %x %x , buf:%x len:%x, flag:%s \n",
			n_cmd, cmd[0], cmd[1], cmd[2], cmd[3],
			(buf)? (*buf) : 0, n_buf,
			(flag == SPIC_READ_BYTES)? "read" : "write");
	
#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)||defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	ra_outl(RT2880_SPI_ARB_REG, SPIARB_ARB_EN|(SPIARB_SPI1_ACTIVE_MODE<<1)| SPIARB_SPI0_ACTIVE_MODE);
#if	defined(CONFIG_RALINK_SPI_CS1_HIGH_ACTIVE)
	ra_and(RT2880_SPI1_CTL_REG, (~SPIARB_SPI1_ACTIVE_MODE));
#else
	ra_or(RT2880_SPI1_CTL_REG, (~SPIARB_SPI1_ACTIVE_MODE)&0x01);
#endif
#endif
	ra_outl(RT2880_SPICFG_REG, SPICFG_MSBFIRST | SPICFG_RXCLKEDGE_FALLING | SPICFG_TXCLKEDGE_FALLING | CFG_CLK_DIV);

	// assert CS and we are already CLK normal high
	ra_and(RT2880_SPICTL_REG, ~(SPICTL_SPIENA_HIGH));

	// write command
	for (retval = 0; retval < n_cmd; retval++) {
		ra_outl(RT2880_SPIDATA_REG, cmd[retval]);
		ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
		if (spic_busy_wait()) {
			retval = -1;
			goto end_trans;
		}
	}

	// read / write  data
	if (flag & SPIC_READ_BYTES) {
		for (retval = 0; retval < n_buf; retval++) {
			ra_or(RT2880_SPICTL_REG, SPICTL_STARTRD);
			if (spic_busy_wait())
				goto end_trans;
			buf[retval] = (u8) ra_inl(RT2880_SPIDATA_REG);
		}

	}
	else if (flag & SPIC_WRITE_BYTES) {
		for (retval = 0; retval < n_buf; retval++) {
			ra_outl(RT2880_SPIDATA_REG, buf[retval]);
			ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
			if (spic_busy_wait())
				goto end_trans;
		}
	}

end_trans:
	// de-assert CS and
	ra_or (RT2880_SPICTL_REG, (SPICTL_SPIENA_HIGH));

	return retval;
}

static int spic_read(const u8 *cmd, size_t n_cmd, u8 *rxbuf, size_t n_rx)
{
	return spic_transfer(cmd, n_cmd, rxbuf, n_rx, SPIC_READ_BYTES);
}

static int spic_write(const u8 *cmd, size_t n_cmd, const u8 *txbuf, size_t n_tx)
{
	return spic_transfer(cmd, n_cmd, (u8 *)txbuf, n_tx, SPIC_WRITE_BYTES);
}
#endif

int spic_init(void)
{
	// GPIO-SPI mode
	ra_and(RALINK_REG_GPIOMODE, ~(1 << 1)); //use normal(SPI) mode instead of GPIO mode

	/* reset spi block */
	ra_or(RT2880_RSTCTRL_REG, RSTCTRL_SPI_RESET);
	udelay(1);
	ra_and(RT2880_RSTCTRL_REG, ~RSTCTRL_SPI_RESET);
	udelay(1);

#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)||defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	ra_outl(RT2880_SPI_ARB_REG, SPIARB_ARB_EN|(SPIARB_SPI1_ACTIVE_MODE<<1)| SPIARB_SPI0_ACTIVE_MODE);
	ra_outl(RT2880_SPI1_CTL_REG, (~SPIARB_SPI1_ACTIVE_MODE)&0x1);
#endif
	ra_outl(RT2880_SPI0_CTL_REG, (~SPIARB_SPI0_ACTIVE_MODE)&0x1);

	ra_outl(RT2880_SPICFG_REG, SPICFG_MSBFIRST | SPICFG_RXCLKEDGE_FALLING | SPICFG_TXCLKEDGE_FALLING | CFG_CLK_DIV);

	// set idle state
	ra_outl(RT2880_SPICTL_REG, SPICTL_HIZSDO | SPICTL_SPIENA_HIGH);

	spi_wait_nsec = (8 * 1000 / (128 / (CFG_CLK_DIV+1)) ) >> 1 ;

	printk("Ralink SPI flash driver, SPI clock: %dMHz\n", (get_surfboard_sysclk() / 1000000) >> (CFG_CLK_DIV+1));

	return 0;
}


/****************************************************************************/
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
	{ "AT25DF321",		0x1f, 0x47000000, 64 * 1024, 64,  0 },
	{ "AT26DF161",		0x1f, 0x46000000, 64 * 1024, 32,  0 },

	{ "FL016AIF",		0x01, 0x02140000, 64 * 1024, 32,  0 },
	{ "FL064AIF",		0x01, 0x02160000, 64 * 1024, 128, 0 },

	{ "MX25L1605D",		0xc2, 0x2015c220, 64 * 1024, 32,  0 },
	{ "MX25L3205D",		0xc2, 0x2016c220, 64 * 1024, 64,  0 },
	{ "MX25L6405D",		0xc2, 0x2017c220, 64 * 1024, 128, 0 },
	{ "MX25L12805D",	0xc2, 0x2018c220, 64 * 1024, 256, 0 },
	{ "MX25L25635E",	0xc2, 0x2019c220, 64 * 1024, 512, 1 },

	{ "S25FL256S",		0x01, 0x02194D01, 64 * 1024, 512, 1 },
	{ "S25FL128P",		0x01, 0x20180301, 64 * 1024, 256, 0 },
	{ "S25FL129P",		0x01, 0x20184D01, 64 * 1024, 256, 0 },
	{ "S25FL032P",		0x01, 0x02154D00, 64 * 1024, 64,  0 },
	{ "S25FL064P",		0x01, 0x02164D00, 64 * 1024, 128, 0 },

	{ "EN25Q32B",		0x1c, 0x30161c30, 64 * 1024, 64,  0 },
	{ "EN25F16",		0x1c, 0x31151c31, 64 * 1024, 32,  0 },
	{ "EN25F32",		0x1c, 0x31161c31, 64 * 1024, 64,  0 },
	{ "EN25F64",		0x1c, 0x20171c20, 64 * 1024, 128, 0 }, // EN25P64
	{ "EN25Q64",		0x1c, 0x30171c30, 64 * 1024, 128, 0 },

	{ "W25X32VS",		0xef, 0x30160000, 64 * 1024, 64,  0 },
	{ "W25Q32BV",		0xef, 0x40160000, 64 * 1024, 64,  0 },
	{ "W25Q64BV",		0xef, 0x40170000, 64 * 1024, 128, 0 }, //S25FL064K
	{ "W25Q128BV",		0xef, 0x40180000, 64 * 1024, 256, 0 },

	{ "STUB",		0x00, 0xffffffff, 64 * 1024, 64,  0 },
};


struct flash_info {
	struct mutex	lock;
	struct mtd_info		mtd;
	struct chip_info	*chip;
	u8			command[5];
};

struct flash_info *flash;

/****************************************************************************/

#if defined (COMMAND_MODE)
static int raspi_cmd(const u8 cmd, const u32 addr, const u8 mode, u8 *buf, const int n_buf, const u32 user, const int flag)
{
	u32 reg;
	int count, retval = 0;

	ra_or(RT2880_SPICFG_REG, (SPICFG_SPIENMODE | SPICFG_RXENVDIS));
	ra_outl(RT2880_SPIDATA_REG, cmd);
	ra_outl(RT2880_SPIMODE_REG, (mode << 24));

	if (flag & SPIC_4B_ADDR)
		ra_outl(RT2880_SPIADDR_REG, addr);
	else
		ra_outl(RT2880_SPIADDR_REG, (addr << 8));

	ra_outl(RT2880_SPIBS_REG, n_buf);

	if (flag & SPIC_USER_MODE)
		ra_outl(RT2880_SPIUSER_REG, user);
	else
		ra_outl(RT2880_SPIUSER_REG, 0);

	ra_outl(RT2880_SPICTL_REG, SPICTL_START);

	if (flag & SPIC_READ_BYTES)
	{
		if (!buf)
			return -1;
		
		for (retval = 0; retval < n_buf;)
		{
			do {
				reg = (u32) (ra_inl(RT2880_SPIFIFOSTAT_REG) & 0xff);
			} while (reg == 0);
			
			for (count = reg; count > 0; count--)
				buf[retval++] = (u8) ra_inl(RT2880_SPIRXFIFO_REG);
		}
	}
	else if (flag & SPIC_WRITE_BYTES)
	{
		if (!buf)
			return -1;
		
		count = min(SPI_FIFO_SIZE, n_buf);
		for (retval = 0; retval < n_buf;)
		{
			while (count--)
			{
				ra_outl(RT2880_SPITXFIFO_REG, buf[retval++]);
			}
			
			do {
				reg = (u32) ((ra_inl(RT2880_SPIFIFOSTAT_REG ) & 0xff00) >> 8);
			} while (reg >= SPI_FIFO_SIZE);
			
			count = SPI_FIFO_SIZE - reg;
			if ((retval + count) > n_buf)
				count = n_buf - retval;
		}
	}

	if (spic_busy_wait())
	{
		retval = -1;
	}

	ra_and(RT2880_SPICFG_REG, ~(SPICFG_SPIENMODE | SPICFG_RXENVDIS));

	return retval;
}

#if defined (RD_MODE_QUAD)
static inline int raspi_write_enable(void);

static int raspi_set_quad()
{
	int retval = 0;

	// Atmel set quad is not tested yet,
	if (flash->chip->id == 0x1f) // Atmel, Write the 7th bit of Configuration register
	{
		u8 sr;
		retval = raspi_cmd(0x3f, 0, 0, &sr, 1, 0, SPIC_READ_BYTES);
		if (retval == -1)
			goto err_end;
		if ((sr & (1 << 7)) == 0)
		{
			sr |= (1 << 7);
			raspi_write_enable();
			retval = raspi_cmd(0x3e, 0, 0, &sr, 1, 0, SPIC_WRITE_BYTES);
		}
	}
	else if (flash->chip->id == 0xc2) //MXIC, 
	{
		u8 sr;
		retval = raspi_cmd(OPCODE_RDSR, 0, 0, &sr, 1, 0, SPIC_READ_BYTES);
		if (retval == -1)
			goto err_end;
		if ((sr & (1 << 6)) == 0)
		{
			sr |= (1 << 6);
			raspi_write_enable();
			retval = raspi_cmd(OPCODE_WRSR, 0, 0, &sr, 1, 0, SPIC_WRITE_BYTES);
		}
	}
	else if ((flash->chip->id == 0x01) || (flash->chip->id == 0xef)) // Spansion or WinBond
	{
		u8 sr[2];
		retval = raspi_cmd(OPCODE_RDSR, 0, 0, sr, 1, 0, SPIC_READ_BYTES);
		if (retval == -1)
			goto err_end;
		retval = raspi_cmd(0x35, 0, 0, &sr[1], 1, 0, SPIC_READ_BYTES);
		if (retval == -1)
			goto err_end;
		if ((sr[1] & (1 << 1)) == 0)
		{
			sr[1] |= (1 << 1);
			//raspi_write_enable();
			retval = raspi_cmd(OPCODE_WRSR, 0, 0, sr, 2, 0, SPIC_WRITE_BYTES);
		}
	}	

err_end:
	if (retval == -1)
		printk("raspi_set_quad error\n");

	return retval;
}
#endif
#endif // COMMAND_MODE

static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
	u8 code = OPCODE_RDID;
	int retval;

#if defined (COMMAND_MODE)
	retval = raspi_cmd(code, 0, 0, rxbuf, n_rx, 0, SPIC_READ_BYTES);
#else
	retval = spic_read(&code, 1, rxbuf, n_rx);
#endif
	if (retval != n_rx) {
		printk("%s: ret: %x\n", __func__, retval);
		return retval;
	}
	return retval;
}

static int raspi_read_rg(u8 *val, u8 opcode)
{
	ssize_t retval;
	u8 code = opcode;

#if defined (COMMAND_MODE)
	u32 user = SPIUSR_SINGLE | (SPIUSR_SINGLE << 3) | (SPIUSR_SINGLE << 6) | (SPIUSR_SINGLE << 9) | (SPIUSR_READ_DATA << 12) | (SPIUSR_NO_DUMMY << 14) | (SPIUSR_NO_MODE << 16) | (SPIUSR_NO_ADDR << 17) | (SPIUSR_ONE_INSTRU << 20) | (1 << 21);
	retval = raspi_cmd(code, 0, 0, val, 1, user, SPIC_READ_BYTES | SPIC_USER_MODE);
#else
	retval = spic_read(&code, 1, val, 1);
#endif

	return 0;
}

static int raspi_write_rg(u8 *val, u8 opcode)
{
	ssize_t retval;
	u8 code = opcode;
	u32 dr;
#if defined (COMMAND_MODE)
	u32 user;
#endif

	dr = ra_inl(RT2880_SPI_DMA);
	ra_outl(RT2880_SPI_DMA, 0); // Set TxBurstSize to 'b00: 1 transfer

#if defined (COMMAND_MODE)
	user = SPIUSR_SINGLE | (SPIUSR_SINGLE << 3) | (SPIUSR_SINGLE << 6) | (SPIUSR_SINGLE << 9) | (SPIUSR_WRITE_DATA << 12) | (SPIUSR_NO_DUMMY << 14) | (SPIUSR_NO_MODE << 16) | (SPIUSR_NO_ADDR << 17) | (SPIUSR_ONE_INSTRU << 20) | (1 << 21);
	retval = raspi_cmd(code, 0, 0, val, 1, user, SPIC_WRITE_BYTES | SPIC_USER_MODE);
#else
	retval = spic_write(&code, 1, val, 1);
#endif
	ra_outl(RT2880_SPI_DMA, dr);

	return 0;
}

/*
 * Read the status register, returning its value in the location
 */
static int raspi_read_sr(u8 *val)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;

#if defined (COMMAND_MODE)
	retval = raspi_cmd(code, 0, 0, val, 1, 0, SPIC_READ_BYTES);
#else
	retval = spic_read(&code, 1, val, 1);
#endif
	if (retval != 1) {
		printk("%s: ret: %x\n", __func__, retval);
		return -EIO;
	}
	return 0;
}

/*
 * write status register
 */
static int raspi_write_sr(u8 *val)
{
	ssize_t retval;
	u8 code = OPCODE_WRSR;

#if defined (COMMAND_MODE)
	retval = raspi_cmd(code, 0, 0, val, 1, 0, SPIC_WRITE_BYTES);
#else
	retval = spic_write(&code, 1, val, 1);
#endif
	if (retval != 1) {
		printk("%s: ret: %x\n", __func__, retval);
		return -EIO;
	}
	return 0;
}

#if 0
// not used
static int raspi_clear_sr(void)
{
	u8 code = OPCODE_CLSR;

#if defined (COMMAND_MODE)
	raspi_cmd(code, 0, 0, 0, 0, 0, 0);
#else
	spic_read(&code, 1, 0, 0);
#endif
	return 0;
}
#endif

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int raspi_wait_ready(int sleep_ms)
{
	int count;
	int sr = 0;

	/*int timeout = sleep_ms * HZ / 1000;
	while (timeout) 
		timeout = schedule_timeout (timeout);*/

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0;  count < ((sleep_ms+1)*1000); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & (SR_WIP | SR_EPE))) {
			return 0;
		}
		udelay(5);
		/* REVISIT sometimes sleeping would be best */
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
	for (count = 0;  count < ((sleep_ms+1)*100); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & (SR_WIP | SR_EPE))) {
			return 0;
		}
		usleep(100);
	}

	printk("%s: read_sr fail: %x\n", __func__, sr);
	return -EIO;
}

static int raspi_4byte_mode(int enable)
{
	raspi_wait_ready(1);

	if (flash->chip->id == 0x01) // Spansion
	{
		u8 br, br_cfn; // bank register
		
		if (enable)
		{
			br = 0x81;
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
			ra_or(RT2880_SPICFG_REG, SPICFG_ADDRMODE);
#endif
		}
		else
		{
			br = 0x0;
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
			ra_and(RT2880_SPICFG_REG, ~(SPICFG_ADDRMODE));
#endif
		}
		
		raspi_write_rg(&br, OPCODE_BRWR);
		raspi_read_rg(&br_cfn, OPCODE_BRRD);
		if (br_cfn != br)
		{
			printk("4B mode switch failed %d, %x, %x\n", enable, br_cfn, br);
			return -1;
		}
	}
	else // if (flash->chip->id == 0xc2) // MXIC
	{
		ssize_t retval;
		u8 code;
		
		if (enable)
		{
			code = 0xB7; /* EN4B, enter 4-byte mode */
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
			ra_or(RT2880_SPICFG_REG, SPICFG_ADDRMODE);
#endif
		}
		else
		{
			code = 0xE9; /* EX4B, exit 4-byte mode */
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
			ra_and(RT2880_SPICFG_REG, ~(SPICFG_ADDRMODE));
#endif
		}

#if defined (COMMAND_MODE)
		{
			u32 user;
			user = SPIUSR_SINGLE | (SPIUSR_SINGLE << 3) | (SPIUSR_SINGLE << 6) | (SPIUSR_SINGLE << 9) | (SPIUSR_NO_DATA << 12) | (SPIUSR_NO_DUMMY << 14) | (SPIUSR_NO_MODE << 16) | (SPIUSR_NO_ADDR << 17) | (SPIUSR_ONE_INSTRU << 20) | (1 << 21);
			retval = raspi_cmd(code, 0, 0, 0, 0, user, SPIC_USER_MODE);
		}
#else
		retval = spic_read(&code, 1, 0, 0);
#endif
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

#if defined (COMMAND_MODE)
	return raspi_cmd(code, 0, 0, 0, 0, 0, 0);
#else
	return spic_write(&code, 1, NULL, 0);
#endif
}

static inline int raspi_write_disable(void)
{
	u8 code = OPCODE_WRDI;

#if defined (COMMAND_MODE)
	return raspi_cmd(code, 0, 0, 0, 0, 0, 0);
#else
	return spic_write(&code, 1, NULL, 0);
#endif
}

/*
 * Set all sectors (global) unprotected if they are protected.
 * Returns negative if error occurred.
 */
static void raspi_unprotect(void)
{
	u8 sr = 0;

	if (raspi_read_sr(&sr) < 0) {
		printk("%s: read_sr fail: %x\n", __func__, sr);
		return;
	}

	if ((sr & (SR_BP0 | SR_BP1 | SR_BP2)) != 0) {
		sr = 0;
		raspi_write_sr(&sr);
	}
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

#if defined (COMMAND_MODE)
	if (flash->chip->addr4b)
	{
		raspi_4byte_mode(1);
		raspi_cmd(OPCODE_SE, offset, 0, 0, 0, 0, SPIC_4B_ADDR);
		raspi_wait_sleep_ready(950);
		raspi_4byte_mode(0);
	}
	else
	{
		raspi_cmd(OPCODE_SE, offset, 0, 0, 0, 0, 0);
		raspi_wait_sleep_ready(950);
	}
#else
	if (flash->chip->addr4b) {
		flash->command[0] = OPCODE_SE;
		flash->command[1] = offset >> 24;
		flash->command[2] = offset >> 16;
		flash->command[3] = offset >> 8;
		flash->command[4] = offset;
		raspi_4byte_mode(1);
		spic_write(flash->command, 5, 0 , 0);
		raspi_wait_sleep_ready(950);
		raspi_4byte_mode(0);
	}

	/* Set up command buffer. */
	flash->command[0] = OPCODE_SE;
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	spic_write(flash->command, 4, 0 , 0);
	raspi_wait_sleep_ready(950);
#endif

	raspi_write_disable();

	return 0;
}

int raspi_set_lock (struct mtd_info *mtd, loff_t to, size_t len, int set)
{
	int retval;

	while (len > 0) {
		/* FIXME: 4b mode ? */
		/* write the next page to flash */
		flash->command[0] = (set == 0)? 0x39 : 0x36;
		flash->command[1] = to >> 16;
		flash->command[2] = to >> 8;
		flash->command[3] = to;

		raspi_wait_ready(1);
		
		raspi_write_enable();

#if defined (COMMAND_MODE)
		{
			u32 user;
		
			user = SPIUSR_SINGLE | (SPIUSR_SINGLE << 3) | (SPIUSR_SINGLE << 6) | (SPIUSR_SINGLE << 9) | (SPIUSR_NO_DATA << 12) | (SPIUSR_NO_DUMMY << 14) | (SPIUSR_NO_MODE << 16) | (SPIUSR_THREE_BYTE_ADDR << 17) | (SPIUSR_ONE_INSTRU << 20) | (1 << 21);
			retval = raspi_cmd(flash->command[0], to, 0, 0, 0, user, SPIC_USER_MODE);
		}
#else
		retval = spic_write(flash->command, 4, 0, 0);
#endif
		if (retval < 0) {
			return -EIO;
		}
		
		len -= mtd->erasesize;
		to += mtd->erasesize;
	}

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
	size_t readlen;
#if defined (COMMAND_MODE)
	size_t code;
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
		/* REVISIT status return?? */
		mutex_unlock(&flash->lock);
		return -EIO;
	}

#if defined (COMMAND_MODE)

#if defined (RD_MODE_QUAD)
	code = OPCODE_QOR;
#ifdef RD_MODE_QIOR
	code = OPCODE_QIOR;
#endif
	raspi_set_quad();
#elif defined (RD_MODE_DOR)
	code = OPCODE_DOR;
#elif defined (RD_MODE_DIOR)
	code = OPCODE_DIOR;
#elif defined (RD_MODE_FAST)
	code = OPCODE_FAST_READ;
#else
	code = OPCODE_READ;
#endif

	if (flash->chip->addr4b)
	{
		raspi_4byte_mode(1);
		readlen = raspi_cmd(code, from, 0, buf, len, 0, SPIC_READ_BYTES | SPIC_4B_ADDR);
		raspi_4byte_mode(0);
	}
	else
		readlen = raspi_cmd(code, from, 0, buf, len, 0, SPIC_READ_BYTES);
#else

	/* Set up the write data buffer. */
	if (flash->chip->addr4b) {
		flash->command[0] = OPCODE_READ;
		flash->command[1] = from >> 24;
		flash->command[2] = from >> 16;
		flash->command[3] = from >> 8;
		flash->command[4] = from;
		raspi_4byte_mode(1);
		readlen = spic_read(flash->command, 5, buf, len);
		raspi_4byte_mode(0);
	}
	else
	{
		flash->command[1] = from >> 16;
		flash->command[2] = from >> 8;
		flash->command[3] = from;
#ifdef MX_FAST_READ
		flash->command[0] = OPCODE_FAST_READ;
		flash->command[4] = 0;
		readlen = spic_read(flash->command, 5, buf, len);
#else
		flash->command[0] = OPCODE_READ;
		readlen = spic_read(flash->command, 4, buf, len);
#endif
	}
#endif

	mutex_unlock(&flash->lock);

	if (retlen) 
		*retlen = readlen;

	if (readlen != len) 
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
	int retval;
	int count = 0;

	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (!len)
		return(0);

	if (to + len > flash->mtd.size)
		return -EINVAL;

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(2)) {
		mutex_unlock(&flash->lock);
		return -1;
	}

	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	if (flash->chip->addr4b) {
		flash->command[1] = to >> 24;
		flash->command[2] = to >> 16;
		flash->command[3] = to >> 8;
		flash->command[4] = to;
	}
	else
	{
		flash->command[1] = to >> 16;
		flash->command[2] = to >> 8;
		flash->command[3] = to;
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
		if (flash->chip->addr4b) {
			flash->command[1] = to >> 24;
			flash->command[2] = to >> 16;
			flash->command[3] = to >> 8;
			flash->command[4] = to;
		}
		else
		{
			flash->command[1] = to >> 16;
			flash->command[2] = to >> 8;
			flash->command[3] = to;
		}

		raspi_wait_ready(3);
		raspi_write_enable();
		raspi_unprotect();

#if defined (COMMAND_MODE)

#if defined (RD_MODE_QUAD)
		raspi_set_quad();
#endif
		if (flash->chip->addr4b)
			retval = raspi_cmd(OPCODE_PP, to, 0, (u8*)buf, page_size, 0, SPIC_WRITE_BYTES | SPIC_4B_ADDR);
		else
			retval = raspi_cmd(OPCODE_PP, to, 0, (u8*)buf, page_size, 0, SPIC_WRITE_BYTES);
#else
		if (flash->chip->addr4b)
			retval = spic_write(flash->command, 5, buf, page_size);
		else
			retval = spic_write(flash->command, 4, buf, page_size);
#endif
		if (retval > 0) {
			if (retlen)
				*retlen += retval;
				
			if (retval < page_size) {
				raspi_write_disable();
				mutex_unlock(&flash->lock);
				printk("%s: retval:%x return:%x page_size:%x \n", 
				       __func__, retval, retval, page_size);
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

	if (flash->chip->addr4b)
		raspi_4byte_mode(0);

	raspi_write_disable();

	mutex_unlock(&flash->lock);

	return 0;
}


/****************************************************************************/

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

	ra_dbg("deice id : %x %x %x %x %x (%x)\n", buf[0], buf[1], buf[2], buf[3], buf[4], jedec);

	// FIXME, assign default as AT25D
	weight = 0xffffffff;
	match = &chips_data[0];
	for (i = 0; i < ARRAY_SIZE(chips_data); i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
			if (info->jedec_id == jedec)
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
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int raspi_probe(void)
{
	struct chip_info *chip;
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	unsigned i;
	loff_t offs;
	struct __image_header {
		uint8_t unused[60];
		uint32_t ih_ksz;
	} hdr;
#endif

	spic_init();

	if (ra_check_flash_type() != BOOT_FROM_SPI)
		return 0;

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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
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
	ramtd_read(NULL, offs, sizeof(hdr), (size_t *)&i, (u_char *)(&hdr));
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
	mtd_device_unregister(&flash->mtd);

	if (flash) {
		kfree(flash);
		flash = NULL;
	}
}

module_init(raspi_init);
module_exit(raspi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven Liu");
MODULE_DESCRIPTION("MTD SPI driver for Ralink flash chips");
