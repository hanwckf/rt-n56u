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
#if defined USER_MODE || defined COMMAND_MODE
static unsigned int spi_wait_nsec = 150;
static int spic_busy_wait(void)
{
	do {
		if ((ra_inl(RT2880_SPI0_STAT_REG) & 0x01) == 0)
			return 0;
	} while (spi_wait_nsec >> 1);

	printf("%s: fail \n", __func__);
	return -1;
}
#elif defined BBU_MODE
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
#endif // USER_MODE || COMMAND_MODE //


#ifdef USER_MODE
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

	// assert CS and we are already CLK normal high
	ra_and(RT2880_SPI0_CTL_REG, ~(SPICTL_SPIENA_HIGH));

	// write command
	for (retval = 0; retval < n_cmd; retval++) {
		ra_outl(RT2880_SPI0_DATA_REG, cmd[retval]);
		ra_or(RT2880_SPI0_CTL_REG, SPICTL_STARTWR);
		if (spic_busy_wait()) {
			retval = -1;
			goto end_trans;
		}
	}

	// read / write  data
	if (flag & SPIC_READ_BYTES) {
		for (retval = 0; retval < n_buf; retval++) {
			ra_or(RT2880_SPI0_CTL_REG, SPICTL_STARTRD);
			if (n_cmd != 1 && (retval & 0xffff) == 0) {
				printf(".");
			}
			if (spic_busy_wait()) {
				printf("\n");
				goto end_trans;
			}
			buf[retval] = (u8) ra_inl(RT2880_SPI0_DATA_REG);
		}

	}
	else if (flag & SPIC_WRITE_BYTES) {
		for (retval = 0; retval < n_buf; retval++) {
			ra_outl(RT2880_SPI0_DATA_REG, buf[retval]);
			ra_or(RT2880_SPI0_CTL_REG, SPICTL_STARTWR);
			if (spic_busy_wait())
				goto end_trans;
		}
	}

end_trans:
	// de-assert CS and
	ra_or (RT2880_SPI0_CTL_REG, (SPICTL_SPIENA_HIGH));

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
#endif // USER_MODE //

extern unsigned long mips_bus_feq;

void spic_init(void)
{
#if defined USER_MODE || defined COMMAND_MODE

	// use normal(SPI) mode instead of GPIO mode
	ra_and(RT2880_GPIOMODE_REG, ~(1 << 1));
#if defined (RT6855_ASIC_BOARD) || defined (RT6855_FPGA_BOARD)
	ra_or(RT2880_GPIOMODE_REG, (1 << 11));
#endif
	// reset spi block
	ra_or(RT2880_RSTCTRL_REG, RSTCTRL_SPI_RESET);
	udelay(1);
	ra_and(RT2880_RSTCTRL_REG, ~RSTCTRL_SPI_RESET);
	udelay(1);

#if defined(RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
        /* config ARB and set the low or high active correctly according to the device */
	RALINK_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN | (SPIARB_SPI1_ACTIVE_MODE <<1) | SPIARB_SPI0_ACTIVE_MODE;
        RALINK_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;     //disable first
        RALINK_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;     //disable first
#endif

	// FIXME, clk_div should depend on spi-flash.
#ifdef ADDR_4B // use 4 byte address mode
	ra_outl(RT2880_SPI0_CFG_REG, SPICFG_MSBFIRST | SPICFG_TXCLKEDGE_FALLING | SPICFG_SPICLK_DIV8 | SPICFG_SPICLKPOL | SPICFG_ADDRMODE);
#else
	ra_outl(RT2880_SPI0_CFG_REG, SPICFG_MSBFIRST | SPICFG_TXCLKEDGE_FALLING | SPICFG_SPICLK_DIV8 | SPICFG_SPICLKPOL);
#endif
								
	// set idle state
	ra_outl(RT2880_SPI0_CTL_REG, SPICTL_HIZSDO | SPICTL_SPIENA_HIGH);

	spi_wait_nsec = (8 * 1000 / ((mips_bus_feq / 1000 / 1000 / SPICFG_SPICLK_DIV8) )) >> 1 ;

	printf("spi_wait_nsec: %x \n", spi_wait_nsec);

#elif defined BBU_MODE

#if defined (MT7621_ASIC_BOARD) || defined (MT7628_ASIC_BOARD)
	u32 clk_sys, clk_div, reg;

#if defined (MT7621_ASIC_BOARD)
	clk_sys = 125;	/* bclk = 125 MHz */
#ifdef SPI_FAST_CLOCK
	clk_div = 4;	/* bclk/4 -> 31.25 MHz */
#else
	clk_div = 5;	/* bclk/5 -> 25 MHz */
#endif
#else
	clk_sys = mips_bus_feq / 1000000;
#ifdef SPI_FAST_CLOCK
	clk_div = 5;	/* hclk/5 -> 40 MHz */
#else
	clk_div = 8;	/* hclk/8 -> 25 MHz */
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
	{ "MX25L6405D",         0xc2, 0x2017c220, 64 * 1024, 128, 0 },//MX25L6433F
	{ "MX25L12805D",        0xc2, 0x2018c220, 64 * 1024, 256, 0 },//MX25L12835F
	{ "MX25L25635E",        0xc2, 0x2019c220, 64 * 1024, 512, 1 },//MX25L25635F
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
	{ "W25Q128BV",          0xef, 0x40180000, 64 * 1024, 256, 0 },//W25Q128FV
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


#ifdef COMMAND_MODE
static int raspi_cmd(const u8 cmd, const u32 addr, const u8 mode, u8 *buf, const size_t n_buf, const u32 user, const int flag)
{
	u32 reg, count;
	int retval = 0;

	//printf("code = %x, addr = %x, mode = %x, buf = %x, size = %d, user = %d, flag = %x\n", cmd, addr, mode, buf, n_buf, user, flag);
		
	ra_or(RT2880_SPICFG_REG, (SPICFG_SPIENMODE | SPICFG_RXENVDIS));	
	ra_outl(RT2880_SPIDATA_REG, cmd);
	ra_outl(RT2880_SPIMODE_REG, (mode << 24));
#ifdef ADDR_4B
	ra_outl(RT2880_SPIADDR_REG, addr);
#else
	if (flag & SPIC_4B_ADDR)
	{
		ra_outl(RT2880_SPIADDR_REG, addr);
	}
	else
	{
		ra_outl(RT2880_SPIADDR_REG, (addr << 8));
	}
#endif
	ra_outl(RT2880_SPIBS_REG, n_buf);
	if (flag & SPIC_USER_MODE)
	{
		ra_outl(RT2880_SPIUSER_REG, user);
	}
	else
		ra_outl(RT2880_SPIUSER_REG, 0);
	
	ra_outl(RT2880_SPICTL_REG, SPICTL_START);
	

	if (flag & SPIC_READ_BYTES)
	{

		if (buf == 0)
		{
			printf("NULL pointer\n");
			return -1;
		}

		for (retval = 0; retval < n_buf;)
		{
			do {
				reg = (u32) (ra_inl(RT2880_SPIFIFOSTAT_REG) & 0xff);
			} while (reg == 0);
			
			for (count = reg; count > 0; count--)
			{
				buf[retval++] = (u8) ra_inl(RT2880_SPIRXFIFO_REG);
			}
			
		}
	}
	// write
	else if (flag & SPIC_WRITE_BYTES)
	{
		if (buf == 0)
		{
			printf("NULL pointer\n");
			return -1;
		}

		count = SPI_FIFO_SIZE;
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

static int raspi_set_quad()
{
	int retval = 0;


	// Atmel set quad is not tested yet,
	if (spi_chip_info->id == 0x1f) // Atmel, Write the 7th bit of Configuration register
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
	else if (spi_chip_info->id == 0xc2) //MXIC, 
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
	else if ((spi_chip_info->id == 0x01) || (spi_chip_info->id == 0xef)) // Spansion or WinBond
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
		printf("raspi_set_quad error\n");

	return retval;
}
#endif // COMMAND_MODE //

#ifdef BBU_MODE
#ifdef MORE_BUF_MODE
static int bbu_mb_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag)
{
	u32 reg;
	int i, q, r;
	int rc = -1;

	if (flag != SPIC_READ_BYTES && flag != SPIC_WRITE_BYTES)
		return -1;

	/* step 0. enable more byte mode */
	ra_or(SPI_REG_MASTER, (1 << 2));

	bbu_spic_busy_wait();

	/* step 1. set opcode & address */
	if (spi_chip_info && spi_chip_info->addr4b) {
		ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
		ra_or(SPI_REG_CTL, (code << 24) & SPI_CTL_ADDREXT_MASK);
		ra_outl(SPI_REG_OPCODE, addr);
	}
	else
	{
		reg = (code << 24) | (addr & 0xffffff);
		ra_outl(SPI_REG_OPCODE, reg);
	}

	reg = ra_inl(SPI_REG_MOREBUF);
	reg &= ~SPI_MBCTL_TX_RX_CNT_MASK;
	reg &= ~SPI_MBCTL_CMD_MASK;

	/* step 2. set cmd bit count to 32 (or 40) */
	if (spi_chip_info && spi_chip_info->addr4b)
		reg |= ((5 << 3) << 24);
	else
		reg |= ((4 << 3) << 24);

	/* step 3. set rx (miso_bit_cnt) and tx (mosi_bit_cnt) bit count */
	reg |= ((n_rx << 3) << 12);
	reg |=  (n_tx << 3);

	ra_outl(SPI_REG_MOREBUF, reg);

	/* step 4. write DI/DO data #0 ~ #7 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL)
			goto RET_MB_TRANS;
		reg = 0;
		for (i = 0; i < n_tx; i++) {
			r = i % 4;
			if (r == 0)
				reg = 0;
			reg |= (*(buf + i) << (r * 8));
			if (r == 3 || (i+1) == n_tx) {
				q = i / 4;
				ra_outl(SPI_REG_DATA(q), reg);
			}
		}
	}

	/* step 5. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 6. wait spi_master_busy */
	bbu_spic_busy_wait();

	/* step 7. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL)
			goto RET_MB_TRANS;
		reg = 0;
		for (i = 0; i < n_rx; i++) {
			r = i % 4;
			if (r == 0) {
				q = i / 4;
				reg = ra_inl(SPI_REG_DATA(q));
			}
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
	if (spi_chip_info && spi_chip_info->addr4b) {
		ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
		ra_or(SPI_REG_CTL, addr & SPI_CTL_ADDREXT_MASK);
	}
	ra_outl(SPI_REG_OPCODE, ((addr & 0xffffff) << 8));
	ra_or(SPI_REG_OPCODE, code);

	/* step 2. write DI/DO data #0 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL)
			return -1;
		ra_outl(SPI_REG_DATA0, 0);
		switch (n_tx) {
		case 8:
			ra_or(SPI_REG_DATA0, (*(buf+3) << 24));
		case 7:
			ra_or(SPI_REG_DATA0, (*(buf+2) << 16));
		case 6:
			ra_or(SPI_REG_DATA0, (*(buf+1) << 8));
		case 5:
		case 2:
			ra_or(SPI_REG_DATA0, *buf);
			break;
		default:
			printf("%s: fixme, write of length %d\n", __func__, n_tx);
			return -1;
		}
	}

	/* step 3. set mosi_byte_cnt */
	ra_and(SPI_REG_CTL, ~SPI_CTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_CTL, (n_rx << 4));
	if (spi_chip_info && spi_chip_info->addr4b && n_tx >= 4)
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
		if (buf == NULL)
			return -1;
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
			printf("%s: fixme, read of length %d\n", __func__, n_rx);
			return -1;
		}
	}
	return 0;
}
#endif // BBU_MODE //

static int raspi_read_rg(u8 code, u8 *val);
static int raspi_write_rg(u8 code, u8 *val);

static int raspi_read_sr(u8 *val)
{
	return raspi_read_rg(OPCODE_RDSR, val);
}
static int raspi_write_sr(u8 *val)
{
	return raspi_write_rg(OPCODE_WRSR, val);
}

/*
 * read SPI flash device ID
 */
static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
	u8 code = OPCODE_RDID;
	int retval;

#ifdef USER_MODE
	retval = spic_read(&code, 1, rxbuf, n_rx);
#elif defined COMMAND_MODE
	retval = raspi_cmd(code, 0, 0, rxbuf, n_rx, 0, SPIC_READ_BYTES);
#elif defined BBU_MODE
	retval = bbu_spic_trans(code, 0, rxbuf, 1, 4, SPIC_READ_BYTES);
	if (!retval)
		retval = n_rx;
#endif
	if (retval != n_rx) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}
	return retval;
}

/*
 * read status register
 */
static int raspi_read_rg(u8 code, u8 *val)
{
	ssize_t retval;

#ifdef USER_MODE
	retval = spic_read(&code, 1, val, 1);
#elif defined COMMAND_MODE
	retval = raspi_cmd(code, 0, 0, val, 1, 0, SPIC_READ_BYTES);
#elif defined BBU_MODE
	retval = bbu_spic_trans(code, 0, val, 1, 1, SPIC_READ_BYTES);
	return retval;
#endif
	if (retval != 1) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}
	return 0;
}

/*
 * write status register
 */
static int raspi_write_rg(u8 code, u8 *val)
{
	ssize_t retval;

#ifdef USER_MODE
	retval = spic_write(&code, 1, val, 1);
#elif defined COMMAND_MODE
	retval = raspi_cmd(code, 0, 0, val, 1, 0, SPIC_WRITE_BYTES);
#elif defined BBU_MODE
	{
		// put the value to be written in address register, so it will be transfered
		u32 address = (*val) << 24;
		retval = bbu_spic_trans(code, address, val, 2, 0, SPIC_WRITE_BYTES);
	}
	return retval;
#endif
	if (retval != 1) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}
	return 0;
}

#if 0
static int raspi_clear_sr(void)
{
	u8 code = OPCODE_CLSR;

#ifdef USER_MODE
	spic_read(&code, 1, 0, 0);
#elif defined COMMAND_MODE
	raspi_cmd(code, 0, 0, 0, 0, 0, 0);
#elif defined BBU_MODE
	bbu_spic_trans(code, 0, NULL, 1, 0, 0);
#endif
	return 0;
}
#endif

static int raspi_read_scur(u8 *val)
{
	ssize_t retval = -1;
	u8 code = 0x2b;

#ifdef USER_MODE
	retval = spic_read(&code, 1, val, 1);
#elif defined COMMAND_MODE
	u32 user;

	user = SPIUSR_SINGLE | (SPIUSR_SINGLE << 3) | (SPIUSR_SINGLE << 6) | (SPIUSR_SINGLE << 9) | (SPIUSR_READ_DATA << 12) | (SPIUSR_NO_DUMMY << 14) | (SPIUSR_NO_MODE << 16) | (SPIUSR_NO_ADDR << 17) | (SPIUSR_ONE_INSTRU << 20) | (1 << 21);
	retval = raspi_cmd(code, 0, 0, val, 1, user, SPIC_READ_BYTES | SPIC_USER_MODE);
#elif defined BBU_MODE
	retval = bbu_spic_trans(code, 0, val, 1, 1, SPIC_READ_BYTES);
#endif
	if (retval != 1) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}
	return 0;
}

static inline int raspi_write_enable(void);
static int raspi_4byte_mode(int enable)
{
	ssize_t retval;
	
	raspi_wait_ready(1);

	if (spi_chip_info->id == 0x1) // Spansion
	{
		u8 br, br_cfn; // bank register
#ifdef USER_MODE
		if (enable)
		{
			br = 0x81;
			ra_or(RT2880_SPICFG_REG, SPICFG_ADDRMODE);
		}
		else
		{
			br = 0x0;
			ra_and(RT2880_SPICFG_REG, ~(SPICFG_ADDRMODE));
		}
    	
#elif defined BBU_MODE
		if (enable) {
			ra_or(SPI_REG_CTL, 0x3 << 19);
			ra_or(SPI_REG_Q_CTL, 0x3 << 8);
			br = 0x81;
		}
		else {
			ra_and(SPI_REG_CTL, ~SPI_CTL_SIZE_MASK);
			ra_or(SPI_REG_CTL, 0x2 << 19);
			ra_and(SPI_REG_Q_CTL, ~(0x3 << 8));
			ra_or(SPI_REG_Q_CTL, 0x2 << 8);
			br = 0;
		}

#endif

		raspi_write_rg(OPCODE_BRWR, &br);
		raspi_read_rg(OPCODE_BRRD, &br_cfn);
		if (br_cfn != br)
		{
			printf("4B mode switch failed %d, %x, %x\n", enable, br_cfn, br);
			return -1;
		}

	}
	else
	{
		u8 code;
#ifdef COMMAND_MODE
		u32 user;
#endif

		code = enable? 0xB7 : 0xE9; /* B7: enter 4B, E9: exit 4B */

#ifdef USER_MODE
		if (enable)
			ra_or(RT2880_SPICFG_REG, SPICFG_ADDRMODE);
		else
			ra_and(RT2880_SPICFG_REG, ~(SPICFG_ADDRMODE));
		retval = spic_read(&code, 1, 0, 0);
#elif defined COMMAND_MODE
		if (enable)
			ra_or(RT2880_SPICFG_REG, SPICFG_ADDRMODE);
		else
			ra_and(RT2880_SPICFG_REG, ~(SPICFG_ADDRMODE));
		user = SPIUSR_SINGLE | (SPIUSR_SINGLE << 3) | (SPIUSR_SINGLE << 6) | (SPIUSR_SINGLE << 9) | (SPIUSR_NO_DATA << 12) | (SPIUSR_NO_DUMMY << 14) | (SPIUSR_NO_MODE << 16) | (SPIUSR_NO_ADDR << 17) | (SPIUSR_ONE_INSTRU << 20) | (1 << 21);
		retval = raspi_cmd(code, 0, 0, 0, 0, user, SPIC_USER_MODE);
#elif defined BBU_MODE
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
#endif
		// for Winbond's W25Q256FV, need to clear extend address register
		if ((!enable) && (spi_chip_info->id == 0xef))
		{
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

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int raspi_write_enable(void)
{
	u8 code = OPCODE_WREN;

#ifdef USER_MODE
	return spic_write(&code, 1, NULL, 0);
#elif defined COMMAND_MODE
	return raspi_cmd(code, 0, 0, 0, 0, 0, 0);
#elif defined BBU_MODE
	return bbu_spic_trans(code, 0, NULL, 1, 0, 0);
#endif
}

static inline int raspi_write_disable(void)
{
	u8 code = OPCODE_WRDI;

#ifdef USER_MODE
	return spic_write(&code, 1, NULL, 0);
#elif defined COMMAND_MODE
	return raspi_cmd(code, 0, 0, 0, 0, 0, 0);
#elif defined BBU_MODE
	return bbu_spic_trans(code, 0, NULL, 1, 0, 0);
#endif
}

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

	//udelay(1000 * sleep_ms);

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < ((sleep_ms+1) *1000*500); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & SR_WIP)) {
			return 0;
		}

		udelay(1);
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
#ifdef USER_MODE
	u8 buf[5];
#endif

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(950))
		return -1;

	/* Send write enable, then erase commands. */
	raspi_write_enable();
	raspi_unprotect();

#ifdef USER_MODE
	if (spi_chip_info->addr4b) {
		raspi_4byte_mode(1);
		raspi_write_enable();
		buf[0] = OPCODE_SE;
		buf[1] = offset >> 24;
		buf[2] = offset >> 16;
		buf[3] = offset >> 8;
		buf[4] = offset;
		spic_write(buf, 5, 0, 0);
		raspi_4byte_mode(0);
		return 0;
	}

	/* Set up command buffer. */
	buf[0] = OPCODE_SE;
	buf[1] = offset >> 16;
	buf[2] = offset >> 8;
	buf[3] = offset;

	spic_write(buf, 4, 0, 0);
	raspi_wait_ready(950);
#elif defined COMMAND_MODE
	if (spi_chip_info->addr4b)
	{
		raspi_4byte_mode(1);
		raspi_write_enable();
		raspi_cmd(OPCODE_SE, offset, 0, 0, 0, 0, SPIC_4B_ADDR);
	}
	else
		raspi_cmd(OPCODE_SE, offset, 0, 0, 0, 0, 0);
	raspi_wait_ready(950);

	if (spi_chip_info->addr4b)
	{
		raspi_4byte_mode(0);
	}
#elif defined BBU_MODE
	if (spi_chip_info->addr4b)
	{
		raspi_4byte_mode(1);
		raspi_write_enable();
	}
	bbu_spic_trans(STM_OP_SECTOR_ERASE, offset, NULL, 4, 0, 0);
	raspi_wait_ready(950);
	if (spi_chip_info->addr4b)
		raspi_4byte_mode(0);
#endif

	raspi_write_disable();

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

#ifdef BBU_MODE
	printf("spi device id: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
#else
	printf("spi device id: %x %x %x %x %x (%x)\n", buf[0], buf[1], buf[2], buf[3], buf[4], jedec);
#endif

	// FIXME, assign default as AT25D
	weight = 0xffffffff;
	match = &chips_data[0];
	for (i = 0; i < sizeof(chips_data)/sizeof(chips_data[0]); i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
#ifdef BBU_MODE
			if ((info->jedec_id & 0xffffff00) == jedec)
#else
			if (info->jedec_id == jedec)
#endif
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
	return spi_chip_info->sector_size * spi_chip_info->n_sectors;
}

int raspi_erase(unsigned int offs, int len)
{
	ra_dbg("%s: offs:%x len:%x\n", __func__, offs, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	/* now erase those sectors */
	while (len > 0) {
		if (raspi_erase_sector(offs)) {
			return -1;
		}

		offs += spi_chip_info->sector_size;
		len -= spi_chip_info->sector_size;
		LED_ALERT_BLINK();
		printf(".");
	}
	printf("\n");

	return 0;
}

int raspi_read(char *buf, unsigned int from, int len)
{
	int rdlen = 0;
#ifdef USER_MODE
	u8 cmd[5];
#elif defined COMMAND_MODE
	u8 code;
#endif

	ra_dbg("%s: from:%x len:%x \n", __func__, from, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	/* Wait till previous write/erase is done. */
	if (raspi_wait_ready(1)) {
		/* REVISIT status return?? */
		return -1;
	}

#ifdef USER_MODE

	/* Set up the write data buffer. */
	cmd[0] = OPCODE_READ;
	if (spi_chip_info->addr4b) {
		raspi_4byte_mode(1);
		cmd[1] = from >> 24;
		cmd[2] = from >> 16;
		cmd[3] = from >> 8;
		cmd[4] = from;
		rdlen = spic_read(cmd, 5, buf , len);
		raspi_4byte_mode(0);
	}
	else
	{
		cmd[1] = from >> 16;
		cmd[2] = from >> 8;
		cmd[3] = from;
		rdlen = spic_read(cmd, 4, buf, len);
	}
	if (rdlen != len)
		printf("warning: rdlen != len\n");

#elif defined COMMAND_MODE

	/* NOTE: OPCODE_FAST_READ (if available) is faster... */
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

	if (spi_chip_info->addr4b)
	{
		raspi_4byte_mode(1);
		rdlen = raspi_cmd(code, from, 0, buf, len, 0, SPIC_READ_BYTES | SPIC_4B_ADDR);
	}
	else
		rdlen = raspi_cmd(code, from, 0, buf, len, 0, SPIC_READ_BYTES);
	if (spi_chip_info->addr4b)
	{
		raspi_4byte_mode(0);
	}

#elif defined BBU_MODE
	if (spi_chip_info->addr4b)
		raspi_4byte_mode(1);
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
#endif

	return rdlen;
}

int raspi_write(char *buf, unsigned int to, int len)
{
	u32 page_offset, page_size;
	int rc = 0, retlen = 0;
#ifdef USER_MODE
	u8 cmd[5];
#elif defined BBU_MODE
	int wrto, wrlen, more;
	char *wrbuf;
#endif

	ra_dbg("%s: to:%x len:%x \n", __func__, to, len);

	/* sanity checks */
	if (len == 0)
		return 0;
	if (to + len > spi_chip_info->sector_size * spi_chip_info->n_sectors)
		return -1;

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(2))
		return -1;

#ifdef USER_MODE
	/* Set up the opcode in the write buffer. */
	cmd[0] = OPCODE_PP;
	if (spi_chip_info->addr4b) {
		cmd[1] = to >> 24;
		cmd[2] = to >> 16;
		cmd[3] = to >> 8;
		cmd[4] = to;
	}
	else
	{
		cmd[1] = to >> 16;
		cmd[2] = to >> 8;
		cmd[3] = to;
	}
#endif

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	if (spi_chip_info->addr4b)
		raspi_4byte_mode(1);

	/* write everything in PAGESIZE chunks */
	while (len > 0) {
		page_size = min(len, FLASH_PAGESIZE-page_offset);
		page_offset = 0;

		/* write the next page to flash */
#ifdef USER_MODE
		if (spi_chip_info->addr4b) {
			cmd[1] = to >> 24;
			cmd[2] = to >> 16;
			cmd[3] = to >> 8;
			cmd[4] = to;
		}
		else
		{
			cmd[1] = to >> 16;
			cmd[2] = to >> 8;
			cmd[3] = to;
		}
#endif

		raspi_wait_ready(3);
		raspi_write_enable();
		raspi_unprotect();

#ifdef USER_MODE

		if (spi_chip_info->addr4b)
			rc = spic_write(cmd, 5, buf, page_size);
		else
			rc = spic_write(cmd, 4, buf, page_size);

#elif defined COMMAND_MODE

#ifdef RD_MODE_QUAD
		raspi_set_quad();
#endif

		if (spi_chip_info->addr4b)
			rc = raspi_cmd(OPCODE_PP, to, 0, buf, page_size, 0, SPIC_WRITE_BYTES | SPIC_4B_ADDR);
		else
			rc = raspi_cmd(OPCODE_PP, to, 0, buf, page_size, 0, SPIC_WRITE_BYTES);

		//{
		//	u32 user;
			
		//	user = SPIUSR_QUAD | (SPIUSR_SINGLE << 3) | (SPIUSR_SINGLE << 6) | (SPIUSR_SINGLE << 9) | (SPIUSR_WRITE_DATA << 12) | (SPIUSR_NO_DUMMY << 14) | (SPIUSR_NO_MODE << 16) | (SPIUSR_THREE_BYTE_ADDR << 17) | (SPIUSR_ONE_INSTRU << 20) | (1 << 21);
		//	rc = raspi_cmd(OPCODE_QPP, to, 0, buf, page_size, user, (SPIC_WRITE_BYTES | SPIC_USER_MODE));
			
		//}
		//printf("rc = %d\n", rc);
		
#elif defined BBU_MODE
		wrto = to;
		wrlen = page_size;
		wrbuf = buf;
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
				raspi_write_disable();
				raspi_wait_ready(100);
				raspi_write_enable();
			}
		} while (wrlen > 0);
#endif

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
				return retlen;
			}
		}

		len -= page_size;
		to += page_size;
		buf += page_size;
	}
	raspi_wait_ready(100);
	printf("\n");
	if (spi_chip_info->addr4b)
		raspi_4byte_mode(0);

	raspi_write_disable();

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
		raspi_unprotect();
		rcode = raspi_erase(CFG_KERN_ADDR - CFG_FLASH_BASE,
				size - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		return rcode;
	}
	else if (strcmp(argv[1], "uboot") == 0) 
	{
		printf("\n Erase u-boot block !!\n");
		printf("From 0x%X length 0x%X\n", 0, CFG_BOOTLOADER_SIZE);
		raspi_unprotect();
		rcode = raspi_erase(0, CFG_BOOTLOADER_SIZE);
		return rcode;
	}
	else if (strcmp(argv[1], "all") == 0) {
		raspi_unprotect();
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
