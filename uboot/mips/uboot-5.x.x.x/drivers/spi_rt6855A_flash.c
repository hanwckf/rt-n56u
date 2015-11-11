#include <common.h>
#include <command.h>
#include <version.h>
#include <rt_mmap.h>
#include <configs/rt2880.h>
#include <malloc.h>
#include "bbu_spiflash.h"

#if (CONFIG_COMMANDS & CFG_CMD_SPI) 

/* Choose the SPI flash mode */
#define BBU_MODE		// BBU SPI flash controller
#define MORE_BUF_MODE

#if !defined USER_MODE && !defined COMMAND_MODE && !defined BBU_MODE
#error "Please choose the correct mode of SPI flash controller"
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

#define OPCODE_FAST_READ	0x0B		/* Fast Read */
#define OPCODE_DOR			0x3B	/* Dual Output Read */
#define OPCODE_QOR			0x6B	/* Quad Output Read */
#define OPCODE_DIOR			0xBB	/* Dual IO High Performance Read */
#define OPCODE_QIOR			0xEB	/* Quad IO High Performance Read */
#define OPCODE_READ_ID		0x90	/* Read Manufacturer and Device ID */

#define OPCODE_P4E			0x20	/* 4KB Parameter Sectore Erase */
#define OPCODE_P8E			0x40	/* 8KB Parameter Sectore Erase */
#define OPCODE_BE			0x60	/* Bulk Erase */
#define OPCODE_BE1			0xC7	/* Bulk Erase */
#define OPCODE_QPP			0x32	/* Quad Page Programing */

#define OPCODE_CLSR			0x30
#define OPCODE_RCR			0x35	/* Read Configuration Register */

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

#define ra_dbg(args...)
/*#define ra_dbg(args...) do { if (1) printf(args); } while(0)*/

#define SPI_FIFO_SIZE 16

//#define ADDR_4B		// if all instruction use 4B address mode
//#define RD_MODE_FAST		// use Fast Read instead of normal Read
//#define RD_MODE_DIOR		// use DIOR (0xBB)instead of normal Read
//#define RD_MODE_DOR		// use DOR (0x3B) instead of normal Read
//#define RD_MODE_QIOR		// use QIOR (0xEB) instead of normal Read
//#define RD_MODE_QOR		// use QOR (0x6B) instead of normal Read

#if defined(RD_MODE_QOR) || defined(RD_MODE_QIOR)
#define RD_MODE_QUAD
#endif

#define SPIC_READ_BYTES (1<<0)
#define SPIC_WRITE_BYTES (1<<1)
#define SPIC_USER_MODE (1<<2)
#define SPIC_4B_ADDR (1<<3)

extern void LED_ALERT_BLINK(void);

static int raspi_wait_ready(int sleep_ms);

static int bbu_spic_busy_wait(void)
{
	int n = 100000;
	do {
		if ((ra_inl(SPI_REG_CTL) & SPI_CTL_BUSY) == 0)
			return 0;
		udelay(1);
	} while (--n > 0);

	printf("%s: fail \n", __func__);
	return -1;
}

extern unsigned long mips_bus_feq;

void spic_init(void)
{
#if defined (MT7621_ASIC_BOARD) || defined (MT7628_ASIC_BOARD)
	u32 clk_sys, clk_div, reg;

	clk_sys = mips_bus_feq / 1000000;
#if defined (MT7621_ASIC_BOARD)
	// hclk = 220 MHz
#ifdef SPI_FAST_CLOCK
	clk_div = 5;	/* hclk/5 -> 44.0 MHz */
#else
	clk_div = 7;	/* hclk/7 -> 31.4 MHz */
#endif
#else
	// hclk = 193/191 MHz
#ifdef SPI_FAST_CLOCK
	clk_div = 4;	/* hclk/4 -> 48.3 MHz */
#else
	clk_div = 6;	/* hclk/6 -> 32.2 MHz */
#endif
#endif
	reg = ra_inl(SPI_REG_MASTER);
	reg &=  ~(0x7);
	reg &=  ~(0xfff << 16);
	reg |= ((clk_div - 2) << 16);
	ra_outl(SPI_REG_MASTER, reg);

	printf("%s SPI flash driver, SPI clock: %dMHz\n", RLT_MTK_VENDOR_NAME, clk_sys / clk_div);

#elif defined (RT6855_ASIC_BOARD) || defined (RT6855_FPGA_BOARD)
	// enable SMC bank 0 alias addressing
	ra_or(RALINK_SYSCTL_BASE + 0x38, 0x80000000);
#endif
}

struct chip_info {
	char		*name;
	u8		id;
	u32		jedec_id;
	unsigned long	sector_size;
	unsigned int	n_sectors;
	char		addr4b;
};
struct chip_info *spi_chip_info = NULL;

static struct chip_info chips_data [] = {
	/* REVISIT: fill in JEDEC ids, for parts that have them */
	{ "AT25DF321",		0x1f, 0x47000000, 64 * 1024, 64,  0 },
	{ "AT26DF161",		0x1f, 0x46000000, 64 * 1024, 32,  0 },
	{ "FL016AIF",		0x01, 0x02140000, 64 * 1024, 32,  0 },
	{ "FL064AIF",		0x01, 0x02160000, 64 * 1024, 128, 0 },
	{ "MX25L1605D",         0xc2, 0x2015c220, 64 * 1024, 32,  0 },//MX25L1606E
	{ "MX25L3205D",         0xc2, 0x2016c220, 64 * 1024, 64,  0 },//MX25L3233F
	{ "MX25L6406E",         0xc2, 0x2017c220, 64 * 1024, 128, 0 },//MX25L6433F
	{ "MX25L12835F",        0xc2, 0x2018c220, 64 * 1024, 256, 0 },//MX25L12835F
	{ "MX25L25635F",        0xc2, 0x2019c220, 64 * 1024, 512, 1 },//MX25L25635F
	{ "MX25L51245G",        0xc2, 0x201ac220, 64 * 1024, 1024, 1 },
	{ "S25FL256S",		0x01, 0x02194D01, 64 * 1024, 512, 1 },
	{ "S25FL128P",		0x01, 0x20180301, 64 * 1024, 256, 0 },
	{ "S25FL129P",          0x01, 0x20184D01, 64 * 1024, 256, 0 },
	{ "S25FL164K",		0x01, 0x40170140, 64 * 1024, 128, 0 },
	{ "S25FL132K",		0x01, 0x40160140, 64 * 1024, 64, 0 },
	{ "S25FL032P",		0x01, 0x02154D00, 64 * 1024, 64,  0 },
	{ "S25FL064P",		0x01, 0x02164D00, 64 * 1024, 128, 0 },
	{ "S25FL116K",          0x01, 0x40150140, 64 * 1024, 32,  0 },
	{ "F25L64QA",           0x8c, 0x41170000, 64 * 1024, 128, 0 }, //ESMT
	{ "F25L32QA",           0x8c, 0x41168c41, 64 * 1024, 64,  0 }, //ESMT
	{ "EN25F16",		0x1c, 0x31151c31, 64 * 1024, 32,  0 },
	{ "EN25Q32B",           0x1c, 0x30161c30, 64 * 1024, 64,  0 },
	{ "EN25F32",		0x1c, 0x31161c31, 64 * 1024, 64,  0 },
	{ "EN25F64",		0x1c, 0x20171c20, 64 * 1024, 128,  0 }, //EN25P64
	{ "EN25Q64",		0x1c, 0x30171c30, 64 * 1024, 128,  0 },
	{ "W25Q32BV",           0xef, 0x40160000, 64 * 1024, 64,  0 },//W25Q32FV
	{ "W25X32VS",           0xef, 0x30160000, 64 * 1024, 64,  0 },
	{ "W25Q64BV",           0xef, 0x40170000, 64 * 1024, 128, 0 }, //S25FL064K //W25Q64FV
	{ "W25Q128FV",          0xef, 0x40180000, 64 * 1024, 256, 0 },//W25Q128FV
	{ "W25Q256FV",          0xef, 0x40190000, 64 * 1024, 512, 1 },
	{ "N25Q032A13ESE40F",   0x20, 0xba161000, 64 * 1024, 64,  0 },
	{ "N25Q064A13ESE40F",   0x20, 0xba171000, 64 * 1024, 128, 0 },
	{ "N25Q128A13ESE40F",   0x20, 0xba181000, 64 * 1024, 256, 0 },
	{ "N25Q256A",       	0x20, 0xba191000, 64 * 1024, 512, 1 },
	{ "MT25QL512AB",    	0x20, 0xba201044, 64 * 1024, 1024, 1 },
	{ "GD25Q32B",		0xC8, 0x40160000, 64 * 1024, 64,  0 },
	{ "GD25Q64B",		0xC8, 0x40170000, 64 * 1024, 128,  0 },
	{ "GD25Q128C",		0xC8, 0x40180000, 64 * 1024, 256,  0 },

};

#ifdef MORE_BUF_MODE
static int bbu_mb_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg_mb, reg_ctl, reg_opcode, reg_data, reg_master;
	int i, q, r;
	int rc = -1;

	if (flag != SPIC_READ_BYTES && flag != SPIC_WRITE_BYTES)
		return -1;

	reg_master = ra_inl(SPI_REG_MASTER);
	reg_master &= ~(0x7);

	/* step 0. enable more byte mode */
	ra_outl(SPI_REG_MASTER, (reg_master | (1 << 2)));

	bbu_spic_busy_wait();

	reg_ctl = ra_inl(SPI_REG_CTL);
	reg_ctl &= ~SPI_CTL_TX_RX_CNT_MASK;
	reg_ctl &= ~SPI_CTL_ADDREXT_MASK;

	/* step 1. set opcode & address */
	if (spi_chip_info && spi_chip_info->addr4b) {
		reg_ctl |= ((code << 24) & SPI_CTL_ADDREXT_MASK);
		reg_opcode = addr;
	} else {
		reg_opcode = (code << 24) | (addr & 0xffffff);
	}

	ra_outl(SPI_REG_OPCODE, reg_opcode);

	reg_mb = ra_inl(SPI_REG_MOREBUF);
	reg_mb &= ~SPI_MBCTL_TX_RX_CNT_MASK;
	reg_mb &= ~SPI_MBCTL_CMD_MASK;

	/* step 2. set cmd bit count to 32 (or 40) */
	if (spi_chip_info && spi_chip_info->addr4b)
		reg_mb |= ((5 << 3) << 24);
	else
		reg_mb |= ((4 << 3) << 24);

	/* step 3. set rx (miso_bit_cnt) and tx (mosi_bit_cnt) bit count */
	reg_mb |= ((n_rx << 3) << 12);
	reg_mb |=  (n_tx << 3);

	ra_outl(SPI_REG_MOREBUF, reg_mb);

	/* step 4. write DI/DO data #0 ~ #7 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL)
			goto RET_MB_TRANS;
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
		if (buf == NULL)
			goto RET_MB_TRANS;
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

	rc = 0;

RET_MB_TRANS:
	/* step #. disable more byte mode */
	ra_outl(SPI_REG_MASTER, reg_master);

	return rc;
}
#endif // MORE_BUF_MODE //

static int bbu_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg_ctl, reg_opcode, reg_data;
	int addr4b = 0;

	bbu_spic_busy_wait();

	reg_ctl = ra_inl(SPI_REG_CTL);
	reg_ctl &= ~SPI_CTL_TX_RX_CNT_MASK;
	reg_ctl &= ~SPI_CTL_ADDREXT_MASK;

	if ((reg_ctl & SPI_CTL_SIZE_MASK) == SPI_CTL_SIZE_MASK)
		addr4b = 1;

	/* step 1. set opcode & address */
	if (spi_chip_info && spi_chip_info->addr4b && addr4b)
		reg_ctl |= (addr & SPI_CTL_ADDREXT_MASK);

	reg_opcode = ((addr & 0xffffff) << 8) | code;

	/* step 2. write DI/DO data #0 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL)
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
		case 2:
			reg_opcode &= 0xff;
			if (spi_chip_info && spi_chip_info->addr4b && addr4b) {
				reg_ctl &= ~SPI_CTL_ADDREXT_MASK;
				reg_ctl |= (*buf << 24);
			} else {
				reg_opcode |= (*buf << 24);
			}
			break;
		default:
			printf("%s: fixme, write of length %d\n", __func__, n_tx);
			return -1;
		}
		ra_outl(SPI_REG_DATA0, reg_data);
	}

	ra_outl(SPI_REG_OPCODE, reg_opcode);

	/* step 3. set mosi_byte_cnt */
	reg_ctl |= (n_rx << 4);

	if (spi_chip_info && spi_chip_info->addr4b && addr4b && n_tx >= 4)
		reg_ctl |= (n_tx + 1);
	else
		reg_ctl |= n_tx;

	/* step 4. kick */
	ra_outl(SPI_REG_CTL, reg_ctl | SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();

	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL)
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
			printf("%s: fixme, read of length %d\n", __func__, n_rx);
			return -1;
		}
	}

	return 0;
}

/*
 * read status register
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
	// put the value to be written in address register, so it will be transfered
	u32 address = (*val) << 24;
	return bbu_spic_trans(code, address, val, 2, 0, SPIC_WRITE_BYTES);
}

static inline int raspi_read_sr(u8 *val)
{
	return raspi_read_rg(OPCODE_RDSR, val);
}

static inline int raspi_write_sr(u8 *val)
{
	return raspi_write_rg(OPCODE_WRSR, val);
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
 * read SPI flash device ID
 */
static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
	int retval;

	retval = bbu_spic_trans(OPCODE_RDID, 0, rxbuf, 1, 4, SPIC_READ_BYTES);
	if (!retval)
		retval = n_rx;

	if (retval != n_rx) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}
	return retval;
}

static int raspi_read_scur(u8 *val)
{
	int retval = -1;

	retval = bbu_spic_trans(0x2b, 0, val, 1, 1, SPIC_READ_BYTES);
	if (retval != 1) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}
	return 0;
}

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

	if (spi_chip_info->id == 0x1) // Spansion
	{
		u8 br, br_cfn; // bank register
		
		br = (enable)? 0x81 : 0x0;
		raspi_write_rg(OPCODE_BRWR, &br);
		raspi_read_rg(OPCODE_BRRD, &br_cfn);
		if (br_cfn != br) {
			printf("4B mode switch failed %d, %x, %x\n", enable, br_cfn, br);
			return -1;
		}
	}
	else
	{
		u8 code;

		code = (enable)? 0xB7 : 0xE9; /* B7: enter 4B, E9: exit 4B */
		retval = bbu_spic_trans(code, 0, NULL, 1, 0, 0);

		// for Winbond's W25Q256FV, need to clear extend address register
		if ((!enable) && (spi_chip_info->id == 0xef)) {
			code = 0x0;
			raspi_write_enable();
			raspi_write_rg(0xc5, &code);
		}
		if (retval != 0) {
			printf("%s: ret: %x\n", __func__, retval);
			return -1;
		}
	}
	return 0;
}

#ifdef SPI_FAST_CLOCK
static void raspi_drive_strength(void)
{
	u8 code = 0;

	if (spi_chip_info->id == 0xef) {
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
static inline int raspi_unprotect(void)
{
	u8 sr = 0;

	if (raspi_read_sr(&sr) < 0) {
		printf("%s: read_sr fail: %x\n", __func__, sr);
		return -1;
	}

	if ((sr & (SR_BP0 | SR_BP1 | SR_BP2)) != 0) {
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
	for (count = 0; count < ((sleep_ms+1)*1000*50); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;
		udelay(10);
		/* REVISIT sometimes sleeping would be best */
	}

	printf("%s: read_sr fail: %x\n", __func__, sr);
	return -1;
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
		return -1;

	raspi_write_enable();
	bbu_spic_trans(STM_OP_SECTOR_ERASE, offset, NULL, 4, 0, 0);
	raspi_wait_ready(950);

	return 0;
}

struct chip_info *chip_prob(void)
{
	struct chip_info *info, *match;
	u8 buf[5] = {0};
	u32 jedec, weight;
	int i;

	raspi_read_devid(buf, 5);
	jedec = (u32)((u32)(buf[1] << 24) | ((u32)buf[2] << 16) | ((u32)buf[3] << 8) | (u32)buf[4]);

	printf("spi device id: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

	// FIXME, assign default as AT25D
	weight = 0xffffffff;
	match = &chips_data[0];
	for (i = 0; i < sizeof(chips_data)/sizeof(chips_data[0]); i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
			if ((info->jedec_id & 0xffffff00) == jedec)
			{
				printf("find flash: %s\n", info->name);
				return info;
			}

			if (weight > (info->jedec_id ^ jedec)) {
				weight = info->jedec_id ^ jedec;
				match = info;
			}
		}
	}
	printf("Warning: un-recognized chip ID, please update bootloader!\n");

	return match;
}

unsigned long raspi_init(void)
{
	spic_init();
	spi_chip_info = chip_prob();

#ifdef SPI_FAST_CLOCK
	raspi_drive_strength();
#endif

	return spi_chip_info->sector_size * spi_chip_info->n_sectors;
}

int raspi_erase(unsigned int offs, int len)
{
	int ret = 0;

	ra_dbg("%s: offs:%x len:%x\n", __func__, offs, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(10))
		return -1;

	/* Send write enable, then erase commands. */
	raspi_unprotect();

	if (spi_chip_info->addr4b)
		raspi_4byte_mode(1);

	/* now erase those sectors */
	while (len > 0) {
		if (raspi_erase_sector(offs)) {
			ret = -1;
			break;
		}
		offs += spi_chip_info->sector_size;
		len -= spi_chip_info->sector_size;
		LED_ALERT_BLINK();
		printf(".");
	}

	if (spi_chip_info->addr4b)
		raspi_4byte_mode(0);

	printf("\n");

	return ret;
}

int raspi_read(char *buf, unsigned int from, int len)
{
	int more, rdlen = 0;

	ra_dbg("%s: from:%x len:%x \n", __func__, from, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	/* Wait till previous write/erase is done. */
	if (raspi_wait_ready(1)) {
		/* REVISIT status return?? */
		return -1;
	}

	if (spi_chip_info->addr4b)
		raspi_4byte_mode(1);

#ifdef MORE_BUF_MODE
	more = 32;
#else
	more = 4;
#endif

	do {
		int rc;
		if (len - rdlen <= more) {
#ifdef MORE_BUF_MODE
			rc = bbu_mb_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 0, (len-rdlen), SPIC_READ_BYTES);
#else
			rc = bbu_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 4, (len-rdlen), SPIC_READ_BYTES);
#endif
			if (rc != 0) {
				printf("%s: failed\n", __func__);
				break;
			}
			rdlen = len;
		}
		else {
#ifdef MORE_BUF_MODE
			rc = bbu_mb_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 0, more, SPIC_READ_BYTES);
#else
			rc = bbu_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 4, more, SPIC_READ_BYTES);
#endif
			if (rc != 0) {
				printf("%s: failed\n", __func__);
				break;
			}
			rdlen += more;
			from += more;
		}
	} while (rdlen < len);

	if (spi_chip_info->addr4b)
		raspi_4byte_mode(0);

	return rdlen;
}

int raspi_write(char *buf, unsigned int to, int len)
{
	u32 page_offset, page_size;
	int rc = 0, retlen = 0;
	int wrto, wrlen, more;
	char *wrbuf;

	ra_dbg("%s: to:%x len:%x \n", __func__, to, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	if (to + len > spi_chip_info->sector_size * spi_chip_info->n_sectors)
		return -1;

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(2))
		return -1;

	raspi_unprotect();

	if (spi_chip_info->addr4b)
		raspi_4byte_mode(1);

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

#ifdef MORE_BUF_MODE
	more = 32;
#else
	more = 4;
#endif

	/* write everything in PAGESIZE chunks */
	while (len > 0) {
		page_size = min(len, FLASH_PAGESIZE-page_offset);
		page_offset = 0;

		/* write the next page to flash */
		raspi_wait_ready(3);
		raspi_write_enable();

		wrto = to;
		wrlen = page_size;
		wrbuf = buf;
		do {
			if (wrlen <= more) {
#ifdef MORE_BUF_MODE
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, wrlen, 0, SPIC_WRITE_BYTES);
#else
				bbu_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, wrlen+4, 0, SPIC_WRITE_BYTES);
#endif
				retlen += wrlen;
				wrlen = 0;
			}
			else {
#ifdef MORE_BUF_MODE
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, more, 0, SPIC_WRITE_BYTES);
#else
				bbu_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, more+4, 0, SPIC_WRITE_BYTES);
#endif
				retlen += more;
				wrto += more;
				wrlen -= more;
				wrbuf += more;
			}
			if (wrlen > 0) {
				raspi_wait_ready(100);
				raspi_write_enable();
			}
		} while (wrlen > 0);

		//printf("%s:: to:%x page_size:%x ret:%x\n", __func__, to, page_size, rc);
		if ((retlen & 0xffff) == 0)
		{
			LED_ALERT_BLINK();
			printf(".");
		}

		if (rc > 0) {
			retlen += rc;
			if (rc < page_size) {
				printf("%s: rc:%x page_size:%x\n",
						__func__, rc, page_size);
				goto exit_mtd_write;
			}
		}

		len -= page_size;
		to += page_size;
		buf += page_size;
	}

	raspi_wait_ready(100);

exit_mtd_write:

	if (spi_chip_info->addr4b)
		raspi_4byte_mode(0);

	printf("\n");

	return retlen;
}

int raspi_erase_write(char *buf, unsigned int offs, int count)
{
	int blocksize = spi_chip_info->sector_size;
	int blockmask = blocksize - 1;

	ra_dbg("%s: offs:%x, count:%x\n", __func__, offs, count);

	if (count > (spi_chip_info->sector_size * spi_chip_info->n_sectors) -
			(CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE)) {
		printf("Abort: image size larger than %d!\n\n", (spi_chip_info->sector_size * spi_chip_info->n_sectors) -
				(CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		udelay(10*1000*1000);
		return -1;
	}

	while (count > 0) {
#define BLOCK_ALIGNE(a) (((a) & blockmask))
		if (BLOCK_ALIGNE(offs) || (count < blocksize)) {
			char *block;
			unsigned int piece, blockaddr;
			int piece_size;
			char *temp;
		
			block = malloc(blocksize);
			if (!block)
				return -1;
			temp = malloc(blocksize);
			if (!temp)
				return -1;

			blockaddr = offs & ~blockmask;

			if (raspi_read(block, blockaddr, blocksize) != blocksize) {
				free(block);
				free(temp);
				return -2;
			}

			piece = offs & blockmask;
			piece_size = min(count, blocksize - piece);
			memcpy(block + piece, buf, piece_size);

			if (raspi_erase(blockaddr, blocksize) != 0) {
				free(block);
				free(temp);
				return -3;
			}
			if (raspi_write(block, blockaddr, blocksize) != blocksize) {
				free(block);
				free(temp);
				return -4;
			}
#ifdef RALINK_SPI_UPGRADE_CHECK
			if (raspi_read(temp, blockaddr, blocksize) != blocksize) {
				free(block);
				free(temp);
				return -2;
			}


			if(memcmp(block, temp, blocksize) == 0)
			{    
			   // printf("block write ok!\n\r");
			}
			else
			{
				printf("block write incorrect!\n\r");
				free(block);
				free(temp);
				return -2;
			}
#endif
                        free(temp);
			free(block);

			buf += piece_size;
			offs += piece_size;
			count -= piece_size;
		}
		else {
			unsigned int aligned_size = count & ~blockmask;
			char *temp;
			int i;
			temp = malloc(blocksize);
			if (!temp)
				return -1;

			if (raspi_erase(offs, aligned_size) != 0)
			{
				free(temp);
				return -1;
			}
			if (raspi_write(buf, offs, aligned_size) != aligned_size)
			{
				free(temp);
				return -1;
			}

#ifdef RALINK_SPI_UPGRADE_CHECK
			for( i=0; i< (aligned_size/blocksize); i++)
			{
				if (raspi_read(temp, offs+(i*blocksize), blocksize) != blocksize)
				{
					free(temp);
					return -2;
				}
				if(memcmp(buf+(i*blocksize), temp, blocksize) == 0)
				{
				//	printf("blocksize write ok i=%d!\n\r", i);
				}
				else
				{
					printf("blocksize write incorrect block#=%d!\n\r",i);
					free(temp);
					return -2;
				}
			}
#endif
			free(temp);
	
			buf += aligned_size;
			offs += aligned_size;
			count -= aligned_size;
		}
	}
	printf("Done!\n");
	return 0;
}


extern ulong NetBootFileXferSize;

#ifdef RALINK_CMDLINE
int do_mem_cp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr, dest;
	int count;
	DECLARE_GLOBAL_DATA_PTR;

	addr = CFG_LOAD_ADDR;
	count = (unsigned int)NetBootFileXferSize;

	if (!strncmp(argv[0], "cp.linux", 9)) {
		dest = CFG_KERN_ADDR - CFG_FLASH_BASE;
		printf("\n Copy linux image[%d byte] to SPI Flash[0x%08X].... \n", count, dest);
	}
	else if (!strncmp(argv[0], "cp.uboot", 9)) {
		dest = 0;
		printf("\n Copy uboot[%d byte] to SPI Flash[0x%08X].... \n", count, dest);
	}
	else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	
	raspi_write((char *)addr, dest, count);
	return 0;
}

U_BOOT_CMD(
	cp,	2,	1,	do_mem_cp,
	"cp      - memory copy\n",
	"\ncp.uboot\n    - copy uboot block\n"
	"cp.linux\n    - copy linux kernel block\n"
);

int do_flerase (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode, size;

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	size = spi_chip_info->sector_size * spi_chip_info->n_sectors;
	if (strcmp(argv[1], "linux") == 0) 
	{
		printf("\n Erase linux kernel block !!\n");
		printf("From 0x%X length 0x%X\n", CFG_KERN_ADDR - CFG_FLASH_BASE,
				size - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		rcode = raspi_erase(CFG_KERN_ADDR - CFG_FLASH_BASE,
				size - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		return rcode;
	}
	else if (strcmp(argv[1], "uboot") == 0) 
	{
		printf("\n Erase u-boot block !!\n");
		printf("From 0x%X length 0x%X\n", 0, CFG_BOOTLOADER_SIZE);
		rcode = raspi_erase(0, CFG_BOOTLOADER_SIZE);
		return rcode;
	}
	else if (strcmp(argv[1], "all") == 0) {
		rcode = raspi_erase(0, size);
		return rcode;
	}

	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	erase,	2,	1,	do_flerase,
	"erase   - erase SPI FLASH memory\n",
	"\nerase all\n    - erase all FLASH banks\n"
	"erase uboot\n    - erase uboot block\n"
	"erase linux\n    - erase linux kernel block\n"
);

#define SPI_FLASH_DBG_CMD 
#ifdef SPI_FLASH_DBG_CMD
int ralink_spi_command(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (!strncmp(argv[1], "id", 3)) {
		u8 buf[5];
		raspi_read_devid(buf, 5);
		printf("device id: %x %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
	}
	else if (!strncmp(argv[1], "read", 5)) {
		unsigned int addr, len;
		u8 *p;
		int i;

		addr = simple_strtoul(argv[2], NULL, 16);
		len = simple_strtoul(argv[3], NULL, 16);
		p = (u8 *)malloc(len);
		if (!p) {
			printf("malloc error\n");
			return 0;
		}
		len = raspi_read(p, addr, len); //reuse len
		printf("read len: %d\n", len);
		for (i = 0; i < len; i++) {
			printf("%x ", p[i]);
		}
		printf("\n");
		free(p);
	}
	else if (!strncmp(argv[1], "erase", 6)) {
		unsigned int o, l;
		o = simple_strtoul(argv[2], NULL, 16);
		l = simple_strtoul(argv[3], NULL, 16);
		printf("erase offs 0x%x, len 0x%x\n", o, l);
		raspi_erase(o, l);
	}
	else if (!strncmp(argv[1], "write", 6)) {
		unsigned int o, l;
		u8 *p, t[3] = {0};
		int i;

		o = simple_strtoul(argv[2], NULL, 16);
		l = strlen(argv[3]) / 2;
		p = (u8 *)malloc(l);
		if (!p) {
			printf("malloc error\n");
			return 0;
		}
		for (i = 0; i < l; i++) {
			t[0] = argv[3][2*i];
			t[1] = argv[3][2*i+1];
			*(p + i) = simple_strtoul(t, NULL, 16);
		}
		printf("write offs 0x%x, len 0x%x\n", o, l);
		raspi_write(p, o, l);
		free(p);
	}
	else if (!strncmp(argv[1], "sr", 3)) {
		u8 sr;
		if (!strncmp(argv[2], "read", 5)) {
			if (raspi_read_sr(&sr) < 0)
				printf("read sr failed\n");
			else
				printf("sr %x\n", sr);
		}
		else if (!strncmp(argv[2], "write", 6)) {
			sr = (u8)simple_strtoul(argv[3], NULL, 16);
			printf("trying write sr %x\n", sr);
			if (raspi_write_sr(&sr) < 0)
				printf("write sr failed\n");
			else {
				if (raspi_read_sr(&sr) < 0)
					printf("read sr failed\n");
				else
					printf("sr %x\n", sr);
			}
		}
	}
	else if (!strncmp(argv[1], "scur", 2)) {
		u8 scur;
		if (argv[2][0] == 'r') {
			if (raspi_read_scur(&scur) < 0)
				printf("read scur failed\n");
			else
				printf("scur %d\n", scur);
		}
	}
	else
		printf("Usage:\n%s\n use \"help spi\" for detail!\n", cmdtp->usage);
	return 0;
}

U_BOOT_CMD(
	spi,	4,	1, 	ralink_spi_command,
	"spi	- spi command\n",
	"spi usage:\n"
	"  spi id\n"
	"  spi sr read\n"
	"  spi sr write <value>\n"
	"  spi read <addr> <len>\n"
	"  spi erase <offs> <len>\n"
	"  spi write <offs> <hex_str_value>\n"
);
#endif
#endif // RALINK_CMDLINE //

#endif
