#include <common.h>
#include <command.h>
#include <rt_mmap.h>
#include <configs/rt2880.h>
#include <malloc.h>
#include "ralink_spi.h"


#if (CONFIG_COMMANDS & CFG_CMD_SPI) 

/******************************************************************************
 * SPI FLASH elementray definition and function
 ******************************************************************************/

#define FLASH_PAGESIZE		256
#define MX_4B_MODE			/* MXIC 4 Byte Mode */

/* Flash opcodes. */
#define OPCODE_WREN		6	/* Write enable */
#define OPCODE_RDSR		5	/* Read status register */
#define OPCODE_WRSR		1	/* Write status register */
#define OPCODE_READ		3	/* Read data bytes */
#define OPCODE_PP		2	/* Page program */
#define OPCODE_SE		0xD8	/* Sector erase */
#define OPCODE_RES		0xAB	/* Read Electronic Signature */
#define OPCODE_RDID		0x9F	/* Read JEDEC ID */

/* Status Register bits. */
#define SR_WIP			1	/* Write in progress */
#define SR_WEL			2	/* Write enable latch */
#define SR_BP0			4	/* Block protect 0 */
#define SR_BP1			8	/* Block protect 1 */
#define SR_BP2			0x10	/* Block protect 2 */
#define SR_EPE			0x20	/* Erase/Program error */
#define SR_SRWD			0x80	/* SR write protect */

#define ra_inl(addr)  (*(volatile u32 *)(addr))
#define ra_outl(addr, value)  (*(volatile u32 *)(addr) = (value))
#define ra_and(addr, value) ra_outl(addr, (ra_inl(addr) & (value)))
#define ra_or(addr, value) ra_outl(addr, (ra_inl(addr) | (value)))

/*#define ra_dbg(args...)*/
#define ra_dbg(args...) do { if (1) printf(args); } while(0)

static unsigned int spi_wait_nsec = 0;


static int spic_busy_wait(void)
{
	do {
		if ((ra_inl(RT2880_SPISTAT_REG) & 0x01) == 0)
			return 0;
	} while (spi_wait_nsec >> 1);

	printf("%s: fail \n", __func__);
	return -1;
}

#define SPIC_READ_BYTES (1<<0)
#define SPIC_WRITE_BYTES (1<<1)

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
	/*
	ra_dbg("cmd(%x): %x %x %x %x , buf:%x len:%x, flag:%s \n",
			n_cmd, cmd[0], cmd[1], cmd[2], cmd[3],
			(buf)? (*buf) : 0, n_buf,
			(flag == SPIC_READ_BYTES)? "read" : "write");
	*/

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
			if (n_cmd != 1 && (retval & 0xffff) == 0) {
				printf(".");
			}
			if (spic_busy_wait()) {
				printf("\n");
				goto end_trans;
			}
			buf[retval] = (u8) ra_inl(RT2880_SPIDATA_REG);
		}

	}
	else if (flag & SPIC_WRITE_BYTES) {
		for (retval = 0; retval < n_buf; retval++) {
			ra_outl(RT2880_SPIDATA_REG, buf[retval]);
			ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
			if (spic_busy_wait()) {
				goto end_trans;
			}
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

extern unsigned long mips_bus_feq;
int spic_init(void)
{
	// use normal(SPI) mode instead of GPIO mode
	ra_and(RT2880_GPIOMODE_REG, ~(1 << 1));

	// reset spi block
	ra_or(RT2880_RSTCTRL_REG, RSTCTRL_SPI_RESET);
	udelay(1);
	ra_and(RT2880_RSTCTRL_REG, ~RSTCTRL_SPI_RESET);

	// FIXME, clk_div should depend on spi-flash.
	ra_outl(RT2880_SPICFG_REG, SPICFG_MSBFIRST | SPICFG_TXCLKEDGE_FALLING | SPICFG_SPICLK_DIV8 | SPICFG_SPICLKPOL);
								
	// set idle state
	ra_outl(RT2880_SPICTL_REG, SPICTL_HIZSDO | SPICTL_SPIENA_HIGH);

	spi_wait_nsec = (8 * 1000 / ((mips_bus_feq / 1000 / 1000 / SPICFG_SPICLK_DIV8) )) >> 1 ;

	printf("spi_wait_nsec: %x \n", spi_wait_nsec);
	return 0;
}



struct chip_info {
	char		*name;
	u8		id;
	u32		jedec_id;
	unsigned long	sector_size;
	unsigned int	n_sectors;
	char		addr4b;
};
struct chip_info *spi_chip_info;

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
#ifdef MX_4B_MODE 
	{ "MX25L25635E",	0xc2, 0x2019c220, 64 * 1024, 512, 1 },
#endif
	{ "S25FL128P",		0x01, 0x20180301, 64 * 1024, 256, 0 },
	{ "S25FL129P",		0x01, 0x20184D01, 64 * 1024, 256, 0 },
	{ "S25FL032P",		0x01, 0x02154D00, 64 * 1024, 64,  0 },
	{ "S25FL064P",		0x01, 0x02164D00, 64 * 1024, 128, 0 },
	{ "EN25F16",		0x1c, 0x31151c31, 64 * 1024, 32,  0 },
	{ "EN25F32",		0x1c, 0x31161c31, 64 * 1024, 64,  0 },
	{ "W25Q32BV",		0xef, 0x40160000, 64 * 1024, 64,  0 },
};


/*
 * read SPI flash device ID
 */
static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
	u8 code = OPCODE_RDID;
	int retval;

	retval = spic_read(&code, 1, rxbuf, n_rx);
	if (retval != n_rx) {
		printf("%s: ret: %x\n", __func__, retval);
		return retval;
	}
	return retval;
}

/*
 * read status register
 */
static int raspi_read_sr(u8 *val)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;

	retval = spic_read(&code, 1, val, 1);
	if (retval != 1) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
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

	retval = spic_write(&code, 1, val, 1);
	if (retval != 1) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}
	return 0;
}

#ifdef MX_4B_MODE
static int raspi_read_scur(u8 *val)
{
	ssize_t retval;
	u8 code = 0x2b;

	retval = spic_read(&code, 1, val, 1);
	if (retval != 1) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}
	return 0;
}

static int raspi_4byte_mode(int enable)
{
	ssize_t retval;
	u8 code;

	if (enable)
		code = 0xB7; /* EN4B, enter 4-byte mode */
	else
		code = 0xE9; /* EX4B, exit 4-byte mode */
	retval = spic_read(&code, 1, 0, 0);
	if (retval != 0) {
		printf("%s: ret: %x\n", __func__, retval);
		return -1;
	}
	return 0;
}
#endif

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int raspi_write_enable(void)
{
	u8 code = OPCODE_WREN;

	return spic_write(&code, 1, NULL, 0);
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
	for (count = 0;  count < ((sleep_ms+1) *1000); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & (SR_WIP | SR_EPE | SR_WEL))) {
			return 0;
		}

		udelay(500);
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
	u8 buf[5];

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(950))
		return -1;

	/* Send write enable, then erase commands. */
	raspi_write_enable();
	raspi_unprotect();

#ifdef MX_4B_MODE
	if (spi_chip_info->addr4b) {
		raspi_4byte_mode(1);
		buf[0] = OPCODE_SE;
		buf[1] = offset >> 24;
		buf[2] = offset >> 16;
		buf[3] = offset >> 8;
		buf[4] = offset;
		spic_write(buf, 5, 0 , 0);
		raspi_4byte_mode(0);
		return 0;
	}
#endif

	/* Set up command buffer. */
	buf[0] = OPCODE_SE;
	buf[1] = offset >> 16;
	buf[2] = offset >> 8;
	buf[3] = offset;

	spic_write(buf, 4, 0 , 0);
	return 0;
}

struct chip_info *chip_prob(void)
{
	struct chip_info *info, *match;
	u8 buf[5];
	u32 jedec, weight;
	int i;

	raspi_read_devid(buf, 5);
	jedec = (u32)((u32)(buf[1] << 24) | ((u32)buf[2] << 16) | ((u32)buf[3] <<8) | (u32)buf[4]);

	printf("spi device id: %x %x %x %x %x (%x)\n", buf[0], buf[1], buf[2], buf[3], buf[4], jedec);

	// FIXME, assign default as AT25D
	weight = 0xffffffff;
	match = &chips_data[0];
	for (i = 0; i < sizeof(chips_data)/sizeof(chips_data[0]); i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
			if (info->jedec_id == jedec) {
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
		printf(".");
	}
	printf("\n");

	return 0;
}

int raspi_read(char *buf, unsigned int from, int len)
{
	u8 cmd[5];
	int rdlen;

	ra_dbg("%s: from:%x len:%x \n", __func__, from, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	/* Wait till previous write/erase is done. */
	if (raspi_wait_ready(1)) {
		/* REVISIT status return?? */
		return -1;
	}

	/* NOTE: OPCODE_FAST_READ (if available) is faster... */

	/* Set up the write data buffer. */
	cmd[0] = OPCODE_READ;
#ifdef MX_4B_MODE
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
#endif
	{
		cmd[1] = from >> 16;
		cmd[2] = from >> 8;
		cmd[3] = from;
		rdlen = spic_read(cmd, 4, buf, len);
	}
	if (rdlen != len)
		printf("warning: rdlen != len\n");

	return rdlen;
}

int raspi_write(char *buf, unsigned int to, int len)
{
	u32 page_offset, page_size;
	int rc = 0, retlen = 0;
	u8 cmd[5];

	ra_dbg("%s: to:%x len:%x \n", __func__, to, len);

	/* sanity checks */
	if (len == 0)
		return 0 ;
	if (to + len > spi_chip_info->sector_size * spi_chip_info->n_sectors)
		return -1;

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(2)) {
		return -1;
	}

	/* Set up the opcode in the write buffer. */
	cmd[0] = OPCODE_PP;
#ifdef MX_4B_MODE
	if (spi_chip_info->addr4b) {
		cmd[1] = to >> 24;
		cmd[2] = to >> 16;
		cmd[3] = to >> 8;
		cmd[4] = to;
	}
	else
#endif
	{
		cmd[1] = to >> 16;
		cmd[2] = to >> 8;
		cmd[3] = to;
	}

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

#ifdef MX_4B_MODE
	raspi_4byte_mode(1);
#endif
	/* write everything in PAGESIZE chunks */
	while (len > 0) {
		page_size = min(len, FLASH_PAGESIZE-page_offset);
		page_offset = 0;
		/* write the next page to flash */
#ifdef MX_4B_MODE
		if (spi_chip_info->addr4b) {
			cmd[1] = to >> 24;
			cmd[2] = to >> 16;
			cmd[3] = to >> 8;
			cmd[4] = to;
		}
		else
#endif
		{
			cmd[1] = to >> 16;
			cmd[2] = to >> 8;
			cmd[3] = to;
		}

		raspi_wait_ready(3);
		raspi_write_enable();
		raspi_unprotect();

#ifdef MX_4B_MODE
		if (spi_chip_info->addr4b)
			rc = spic_write(cmd, 5, buf, page_size);
		else
#endif
			rc = spic_write(cmd, 4, buf, page_size);
		//printf("%s:: to:%x page_size:%x ret:%x\n", __func__, to, page_size, rc);
		if ((retlen & 0xffff) == 0)
			printf(".");

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
	printf("\n");
#ifdef MX_4B_MODE
	raspi_4byte_mode(0);
#endif

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

int do_mem_cp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr, dest;
	int count;

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

//#define SPI_FLASH_DBG_CMD 
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
#ifdef MX_4B_MODE
	else if (!strncmp(argv[1], "scur", 2)) {
		u8 scur;
		if (argv[2][0] == 'r') {
			if (raspi_read_scur(&scur) < 0)
				printf("read scur failed\n");
			else
				printf("scur %d\n", scur);
		}
	}
#endif
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
);
#endif

#endif
