#include <common.h>
#include <command.h>
#include <malloc.h>
#include <configs/rt2880.h>
#include <linux/mtd/mtd.h>
#include <asm/errno.h>
#include <nand_api.h>
#include "ralink_nand.h"
#include "../autoconf.h"

#if defined(UBI_SUPPORT)
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <ubi_uboot.h>
#include <asm/errno.h>
#include <jffs2/load_kernel.h>
#endif

#define CONFIG_NUMCHIPS 1

#ifndef ROUNDUP
#define	ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))
#endif

#define ra_inl(addr)  (*(volatile u32 *)(addr))
#define ra_outl(addr, value)  (*(volatile u32 *)(addr) = (value))
#define ra_and(addr, value) ra_outl(addr, (ra_inl(addr) & (value)))
#define ra_or(addr, value) ra_outl(addr, (ra_inl(addr) | (value)))

#define ra_dbg(args...)
//#define ra_dbg(args...) do { if (1) printf(args); } while(0)

#define READ_STATUS_RETRY	1000
#define CLEAR_INT_STATUS()	ra_outl(NFC_INT_ST, ra_inl(NFC_INT_ST))
#define NFC_TRANS_DONE()	(ra_inl(NFC_INT_ST) & INT_ST_ND_DONE)
#define BLOCK_ALIGNED(a) ((a) & (chip->erasesize - 1))

#define BBTTAG_BITS		2
#define BBTTAG_BITS_MASK	((1<<BBTTAG_BITS) -1)
enum BBT_TAG {
	BBT_TAG_UNKNOWN = 0, //2'b01
	BBT_TAG_GOOD	= 3, //2'b11
	BBT_TAG_BAD	= 2, //2'b10
	BBT_TAG_RES	= 1, //2'b01
};

struct ra_nand_chip {
	int	numchips;
	int 	chip_shift;
	int	page_shift;
	int 	erase_shift;
	int 	oob_shift;
	int	badblockpos;
	int	numpage_per_block_bit;
	int	block_size_bit;
	int	ecc_offset;
	int	column_addr_cycle;
	int	addr_cycle;
	int	size;
	int	erasesize;
	int	writesize;
	int	oobsize;
	struct nand_ecclayout	*oob;
	int 	state;
	unsigned int 	buffers_page;
	unsigned char	*buffers; //[chip->writesize + chip->oobsize];
	unsigned char 	*readback_buffers;
	unsigned char 	*bbt;
};

#if defined(UBI_SUPPORT)
static int nand_do_write_ops(struct ra_nand_chip *ra, loff_t to, struct mtd_oob_ops *ops);
static int nand_do_read_ops(struct ra_nand_chip *ra, loff_t from, struct mtd_oob_ops *ops);
int mt7620a_mtd_erase(struct mtd_info *mtd, struct erase_info *instr);
int mt7620a_mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
int mt7620a_mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
int mt7620a_mtd_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
int mt7620a_mtd_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);
int mt7620a_mtd_block_isbad(struct mtd_info *mtd, loff_t ofs);
int mt7620a_mtd_block_markbad(struct mtd_info *mtd, loff_t ofs);
#endif

int nand_block_markbad(struct ra_nand_chip *chip, unsigned int offs);
int nfc_read_page(unsigned char *buf, int page);
int ranand_read(unsigned char *buf, unsigned int from, int datalen);

#if defined(UBI_SUPPORT)
extern int do_mtdparts(cmd_tbl_t *cmdtp, int flag, int argc, char * argv[]);
#endif

unsigned char block_buffer[128 * 1024U];

/*
*	Manufacturer ID list
*/
struct nand_manufacturers nand_manuf_ids[] = {
	{NAND_MFR_TOSHIBA, "Toshiba"},
	{NAND_MFR_SAMSUNG, "Samsung"},
	{NAND_MFR_FUJITSU, "Fujitsu"},
	{NAND_MFR_NATIONAL, "National"},
	{NAND_MFR_RENESAS, "Renesas"},
	{NAND_MFR_STMICRO, "ST Micro"},
	{NAND_MFR_HYNIX, "Hynix"},
	{NAND_MFR_MICRON, "Micron"},
	{NAND_MFR_AMD, "AMD/Spansion"},
	{NAND_MFR_ZENTEL, "Zentel"},
	{0x0, "Unknown manufacturer"}
};

/*
*	Chip ID list
*
*	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
*	options
*
*	Pagesize; 0, 256, 512
*	0	get this information from the extended chip ID
+	256	256 Byte page size
*	512	512 Byte page size
*/
struct nand_flash_dev nand_flash_ids[] = {

#if !defined(UBOOT_STAGE1)
#ifdef CONFIG_MTD_NAND_MUSEUM_IDS
	{"NAND 1MiB 5V 8-bit",		0x6e, 256, 1, 0x1000, 0},
	{"NAND 2MiB 5V 8-bit",		0x64, 256, 2, 0x1000, 0},
	{"NAND 4MiB 5V 8-bit",		0x6b, 512, 4, 0x2000, 0},
	{"NAND 1MiB 3,3V 8-bit",	0xe8, 256, 1, 0x1000, 0},
	{"NAND 1MiB 3,3V 8-bit",	0xec, 256, 1, 0x1000, 0},
	{"NAND 2MiB 3,3V 8-bit",	0xea, 256, 2, 0x1000, 0},
	{"NAND 4MiB 3,3V 8-bit",	0xd5, 512, 4, 0x2000, 0},
	{"NAND 4MiB 3,3V 8-bit",	0xe3, 512, 4, 0x2000, 0},
	{"NAND 4MiB 3,3V 8-bit",	0xe5, 512, 4, 0x2000, 0},
	{"NAND 8MiB 3,3V 8-bit",	0xd6, 512, 8, 0x2000, 0},

	{"NAND 8MiB 1,8V 8-bit",	0x39, 512, 8, 0x2000, 0},
	{"NAND 8MiB 3,3V 8-bit",	0xe6, 512, 8, 0x2000, 0},
	{"NAND 8MiB 1,8V 16-bit",	0x49, 512, 8, 0x2000, NAND_BUSWIDTH_16},
	{"NAND 8MiB 3,3V 16-bit",	0x59, 512, 8, 0x2000, NAND_BUSWIDTH_16},
#endif

	{"NAND 16MiB 1,8V 8-bit",	0x33, 512, 16, 0x4000, 0},
	{"NAND 16MiB 3,3V 8-bit",	0x73, 512, 16, 0x4000, 0},
	{"NAND 16MiB 1,8V 16-bit",	0x43, 512, 16, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 16MiB 3,3V 16-bit",	0x53, 512, 16, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 32MiB 1,8V 8-bit",	0x35, 512, 32, 0x4000, 0},
	{"NAND 32MiB 3,3V 8-bit",	0x75, 512, 32, 0x4000, 0},
	{"NAND 32MiB 1,8V 16-bit",	0x45, 512, 32, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 32MiB 3,3V 16-bit",	0x55, 512, 32, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 64MiB 1,8V 8-bit",	0x36, 512, 64, 0x4000, 0},
	{"NAND 64MiB 3,3V 8-bit",	0x76, 512, 64, 0x4000, 0},
	{"NAND 64MiB 1,8V 16-bit",	0x46, 512, 64, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 64MiB 3,3V 16-bit",	0x56, 512, 64, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 128MiB 1,8V 8-bit",	0x78, 512, 128, 0x4000, 0},
	{"NAND 128MiB 1,8V 8-bit",	0x39, 512, 128, 0x4000, 0},
	{"NAND 128MiB 3,3V 8-bit",	0x79, 512, 128, 0x4000, 0},
	{"NAND 128MiB 1,8V 16-bit",	0x72, 512, 128, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 128MiB 1,8V 16-bit",	0x49, 512, 128, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 128MiB 3,3V 16-bit",	0x74, 512, 128, 0x4000, NAND_BUSWIDTH_16},
	{"NAND 128MiB 3,3V 16-bit",	0x59, 512, 128, 0x4000, NAND_BUSWIDTH_16},

	{"NAND 256MiB 3,3V 8-bit",	0x71, 512, 256, 0x4000, 0},

	/*
	 * These are the new chips with large page size. The pagesize and the
	 * erasesize is determined from the extended id bytes
	 */
#define LP_OPTIONS (NAND_SAMSUNG_LP_OPTIONS | NAND_NO_READRDY | NAND_NO_AUTOINCR)
#define LP_OPTIONS16 (LP_OPTIONS | NAND_BUSWIDTH_16)

	/*512 Megabit */
	{"NAND 64MiB 1,8V 8-bit",	0xA2, 0,  64, 0, LP_OPTIONS},
	{"NAND 64MiB 3,3V 8-bit",	0xF2, 0,  64, 0, LP_OPTIONS},
	{"NAND 64MiB 1,8V 16-bit",	0xB2, 0,  64, 0, LP_OPTIONS16},
	{"NAND 64MiB 3,3V 16-bit",	0xC2, 0,  64, 0, LP_OPTIONS16},

	/* 1 Gigabit */
	{"NAND 128MiB 1,8V 8-bit",	0xA1, 0, 128, 0, LP_OPTIONS},
	{"NAND 128MiB 3,3V 8-bit",	0xF1, 0, 128, 0, LP_OPTIONS},
	{"NAND 128MiB 3,3V 8-bit",	0xD1, 0, 128, 0, LP_OPTIONS},
	{"NAND 128MiB 1,8V 16-bit",	0xB1, 0, 128, 0, LP_OPTIONS16},
	{"NAND 128MiB 3,3V 16-bit",	0xC1, 0, 128, 0, LP_OPTIONS16},
	{"NAND 128MiB 1,8V 16-bit",     0xAD, 0, 128, 0, LP_OPTIONS16},

	/* 2 Gigabit */
	{"NAND 256MiB 1,8V 8-bit",	0xAA, 0, 256, 0, LP_OPTIONS},
	{"NAND 256MiB 3,3V 8-bit",	0xDA, 0, 256, 0, LP_OPTIONS},
	{"NAND 256MiB 1,8V 16-bit",	0xBA, 0, 256, 0, LP_OPTIONS16},
	{"NAND 256MiB 3,3V 16-bit",	0xCA, 0, 256, 0, LP_OPTIONS16},

	/* 4 Gigabit */
	{"NAND 512MiB 1,8V 8-bit",	0xAC, 0, 512, 0, LP_OPTIONS},
	{"NAND 512MiB 3,3V 8-bit",	0xDC, 0, 512, 0, LP_OPTIONS},
	{"NAND 512MiB 1,8V 16-bit",	0xBC, 0, 512, 0, LP_OPTIONS16},
	{"NAND 512MiB 3,3V 16-bit",	0xCC, 0, 512, 0, LP_OPTIONS16},

	/* 8 Gigabit */
	{"NAND 1GiB 1,8V 8-bit",	0xA3, 0, 1024, 0, LP_OPTIONS},
	{"NAND 1GiB 3,3V 8-bit",	0xD3, 0, 1024, 0, LP_OPTIONS},
	{"NAND 1GiB 1,8V 16-bit",	0xB3, 0, 1024, 0, LP_OPTIONS16},
	{"NAND 1GiB 3,3V 16-bit",	0xC3, 0, 1024, 0, LP_OPTIONS16},

	/* 16 Gigabit */
	{"NAND 2GiB 1,8V 8-bit",	0xA5, 0, 2048, 0, LP_OPTIONS},
	{"NAND 2GiB 3,3V 8-bit",	0xD5, 0, 2048, 0, LP_OPTIONS},
	{"NAND 2GiB 1,8V 16-bit",	0xB5, 0, 2048, 0, LP_OPTIONS16},
	{"NAND 2GiB 3,3V 16-bit",	0xC5, 0, 2048, 0, LP_OPTIONS16},

	/* 32 Gigabit */
	{"NAND 4GiB 3,3V 8-bit",	0xD7, 0, 4096, 0, LP_OPTIONS},

	/*
	 * Renesas AND 1 Gigabit. Those chips do not support extended id and
	 * have a strange page/block layout !  The chosen minimum erasesize is
	 * 4 * 2 * 2048 = 16384 Byte, as those chips have an array of 4 page
	 * planes 1 block = 2 pages, but due to plane arrangement the blocks
	 * 0-3 consists of page 0 + 4,1 + 5, 2 + 6, 3 + 7 Anyway JFFS2 would
	 * increase the eraseblock size so we chose a combined one which can be
	 * erased in one go There are more speed improvements for reads and
	 * writes possible, but not implemented now
	 */
	{"AND 128MiB 3,3V 8-bit",	0x01, 2048, 128, 0x4000,
	 NAND_IS_AND | NAND_NO_AUTOINCR |NAND_NO_READRDY | NAND_4PAGE_ARRAY |
	 BBT_AUTO_REFRESH
	},
#endif	/* !UBOOT_STAGE1 */

	{NULL,}
};
struct mtd_info mt7620a_mtdinfo;
struct ra_nand_chip mt7620a_nandinfo;

#if defined(UBI_SUPPORT)
static int ranfc_bbt = 1;
#endif

/* Maximum block address that the skip bad-block mechanism can reach.
 * Assume you want to erase block/write data from a address, length is equal to N blocks,
 * and there are X bad-blocks in the range.
 * 0:
 * 	Inhibit the skip bad-block mechanism. bad-block would be ignored.
 * 	Only N - X blocks would be erased/write, bad-block would be erased.
 * 	Data that should be write to bad-block would be ignored too!
 *
 * otherwise:
 * 	bad-block would be skipped.
 * 	If max_addr is not reached, N blocks would be erased and X blocks would be skipped.
 * 	That is, length is enlarge to N + X blocks.  Data that should be write to a bad-block
 * 	would be write to next non-bad-block.  If max_addr is reached, remained data would not
 * 	be writted.
 */
static unsigned long max_addr = 0;
/* If the dont_erase_bad_block is non-zero, erase bad-block is not allowed.
 * If the dont_erase_bad_block is zero and the page is marked by our NAND Flash driver, erase bad-block is allowed.
 * Whether the dont_erase_bad_block is zero or not, initial invalid block wouldn't be erase due to it's BI byte is not equal to 0x33.
 */
static unsigned int dont_erase_bad_block = 1;

#if defined(UBI_SUPPORT)
struct nand_ecclayout large_page_ecclayout = {
	.eccbytes = 3 * 4,
	.eccpos = { 6, 7, 8, 22, 23, 24, 38, 39, 40, 54, 55, 56},
	.oobavail = 51,
	.oobfree = {
		{ .offset = 1, .length = 5 },
		{ .offset = 9, .length = 13 },
		{ .offset = 25, .length = 13 },
		{ .offset = 41, .length = 13 },
		{ .offset = 57, .length = 7 },
	},
};
#endif

int nand_addrlen = 3;
int is_nand_page_2048 = 0;

const unsigned int nand_size_map[2][3] = {{25, 30, 30}, {20, 27, 30}};

#if defined(UBI_SUPPORT)
const static struct ubi_vol_s {
	const char name[12];
	unsigned int size;	/* volume length in bytes */
	int type;		/* 0: static; otherwise: dynamic */
} ubi_vol[] = {
	{ "nvram",	CFG_UBI_NVRAM_SIZE, 	1 },
	{ "Factory",	CFG_UBI_FACTORY_SIZE,	1 },
	{ "Factory2",	CFG_UBI_FACTORY2_SIZE,	1 },
	{ "linux",	CFG_UBI_FIRMWARE_SIZE,	1 },
	{ "linux2",	CFG_UBI_FIRMWARE2_SIZE,	1 },
	/* last volume size would be auto-resize according to free space */
	{ "jffs2",	CFG_UBI_APP_SIZE,	1 },
	{ "", 0, 0 },
};
#endif

static int nfc_wait_ready(int snooze_ms);

/**
 * reset nand chip
 */
static int nfc_chip_reset(void)
{
	int status;

	//ra_dbg("%s:\n", __func__);
	// reset nand flash
	ra_outl(NFC_CMD1, 0x0);
	ra_outl(NFC_CMD2, 0xff);
	ra_outl(NFC_ADDR, 0x0);
	ra_outl(NFC_CONF, 0x0411);

	status = nfc_wait_ready(5);  //erase wait 5us
	if (status & NAND_STATUS_FAIL) {
		printf("%s: fail\n", __func__);
		return -1;
	}
	return 0;
}

/** 
 * clear NFC and flash chip.
 */
static int nfc_all_reset(void)
{
	int retry;

	// reset controller
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x02); //clear data buffer
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) & ~0x02); //clear data buffer

	CLEAR_INT_STATUS();

	retry = READ_STATUS_RETRY;
	while ((ra_inl(NFC_INT_ST) & 0x02) != 0x02 && retry--);
	if (retry <= 0) {
		printf("%s: clean buffer fail\n", __func__);
		return -1;
	}

	retry = READ_STATUS_RETRY;
	while ((ra_inl(NFC_STATUS) & 0x1) != 0x0 && retry--) { //fixme, controller is busy ?
		udelay(1);
	}
	if (retry <= 0) {
		printf("%s: controller is still busy?\n");
		return -1;
	}

	return nfc_chip_reset();
}


/** NOTICE: only called by nfc_wait_ready().
 * @return -1, nfc can not get transction done 
 * @return 0, ok.
 */
static int nfc_read_status(char *status)
{
	unsigned long cmd1, conf;
	int int_st, nfc_st;
	int retry;

	cmd1 = 0x70;
	conf = 0x000101 | (1 << 20);

	//FIXME, should we check nfc status?
	CLEAR_INT_STATUS();

	ra_outl(NFC_CMD1, cmd1); 	
	ra_outl(NFC_CONF, conf); 

	/* FIXME, 
	 * 1. since we have no wired ready signal, directly 
	 * calling this function is not gurantee to read right status under ready state.
	 * 2. the other side, we can not determine how long to become ready, this timeout retry is nonsense.
	 * 3. SUGGESTION: call nfc_read_status() from nfc_wait_ready(),
	 * that is aware about caller (in sementics) and has snooze plused nfc ND_DONE.
	 */
	retry = READ_STATUS_RETRY; 
	do {
		nfc_st = ra_inl(NFC_STATUS);
		int_st = ra_inl(NFC_INT_ST);
		
		udelay(1);
	} while (!(int_st & INT_ST_RX_BUF_RDY) && retry--);

	if (!(int_st & INT_ST_RX_BUF_RDY)) {
		printf("%s: NFC fail, int_st(%x), retry:%x. nfc:%x, reset nfc and flash.\n",
				__func__, int_st, retry, nfc_st);
		nfc_all_reset();
		*status = NAND_STATUS_FAIL;
		return -1;
	}

	*status = (char)(le32_to_cpu(ra_inl(NFC_DATA)) & 0x0ff);
	return 0;
}

/**
 * @return !0, chip protect.
 * @return 0, chip not protected.
 */
static int nfc_check_wp(void)
{
	/* Check the WP bit */
#if !defined CONFIG_NOT_SUPPORT_WP
	return !!(ra_inl(NFC_CTRL) & 0x01);
#else
	char result = 0;
	int ret;

	ret = nfc_read_status(&result);
	//FIXME, if ret < 0

	return !(result & NAND_STATUS_WP);
#endif
}

#if !defined CONFIG_NOT_SUPPORT_RB
/*
 * @return !0, chip ready.
 * @return 0, chip busy.
 */
static int nfc_device_ready(void)
{
	/* Check the ready  */
	return !!(ra_inl(NFC_STATUS) & 0x04);
}
#endif

/* Set addr to max_addr.
 * If addr is not aligned to block address, it would be aligned to next block address and
 * return value would be positive value.
 * @return:
 * 	0:	success. (addr is aligned to block address and is assigned to max_addr)
 *  otherwise:	success. (addr is aligned to next block address and is assigned to max_addr)
 */
int ranand_set_sbb_max_addr(unsigned long addr)
{
	int ret = 0;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	if (BLOCK_ALIGNED(addr)) {
		debug("%s: addr 0x%x is not aligned to block address.\n", __func__, addr);
		addr += chip->erasesize - 1;
		addr &= ~(chip->erasesize - 1);
		ret = 1;
	}

	max_addr = addr;
	return ret;
}

/* Allow/inhibit erase bad-block that taged by our NAND Flash driver */
void ranand_set_allow_erase_badblock(unsigned int allow)
{
	dont_erase_bad_block = !allow;
	return;
}

#if defined(UBI_SUPPORT)
/* Get pointer to ubi_volume by volume name.
 * @ubi:	pointer to ubi_device
 * @name:	volume name
 * @return:
 * 	NULL:	invalid parameter or not found
 *  otherwise:	success
 */
static struct ubi_volume* get_vol_by_name(struct ubi_device *ubi, const char *name)
{
	int i;
	struct ubi_volume *v = NULL;

	if (!ubi || !name)
		return v;

	for (i = 0; i < (ubi->vtbl_slots + 1) && !v; ++i) {
		if (!ubi->volumes[i] || strcmp(name, ubi->volumes[i]->name))
			continue;

		v = ubi->volumes[i];
	}

	return v;
}

/* Find specified volume is X-th volume.
 * @return:
 * 	-1:	invalid parameter
 * 	-2:	UBI device is not attached
 * 	-3:	volume not found
 *     >0:	the volume is at X-th. X start from 1
 *	0:	not defined.
 */
int get_ubi_volume_idseq_by_addr(const char *name)
{
	int i, seq = 0;
	struct ubi_device *ubi;
	struct ubi_volume *v = NULL;

	if (!name || *name == '\0')
		return -1;
	if ((ubi = get_ubi_device()) == NULL)
		return -2;
	if ((v = get_vol_by_name(ubi, name)) == NULL)
		return -3;

	for (i = 0; i < (ubi->vtbl_slots + 1); ++i) {
		if (!ubi->volumes[i] || !strcmp(name, ubi->volumes[i]->name))
			continue;

		if (ubi->volumes[i]->vol_id < v->vol_id)
			seq++;
	}

	seq++;
	return seq;
}

/* Get volume name and it's start offset address.
 * @addr:	relative address to flash
 * @offset:	if success, it would be store start offset of the volume regards to Flash.
 * @return
 *  NULL:	specified address is not belong to any volume.
 *  otherwise:	volume name
 */
char *get_ubi_volume_param(unsigned long addr, unsigned long *offset, unsigned int *size)
{
	char *found = NULL;
	unsigned long flash_offset = CFG_UBI_DEV_OFFSET;
	const struct ubi_vol_s *p;

	if (!offset || !size)
		return NULL;

	for (p = &ubi_vol[0]; !found && p->size; ++p) {
		if (addr >= flash_offset && addr < (flash_offset + p->size)) {
			found = (char*) p->name;
			*offset = flash_offset;
			*size = p->size;
			break;
		}

		flash_offset += p->size;
	}

	return found;
}

/* Initialize UBI volumes
 * @return:
 * 	0:	success
 *  otherwise:	fail
 */
int init_ubi_volumes(struct ubi_device *ubi)
{
	int id, rcode, show=0;
	struct ubi_volume *v;
	unsigned int size;
	const struct ubi_vol_s *p, *q;
	char tmp[] = "ffffffffXXXX", tmp1[] = "ffffffffXXX", type[] = "dynamicXXX";
	char *ubi_create_vol[] = { "ubi", "createvol", NULL, tmp, type };
	char *ubi_info_wlayout[] = { "ubi", "info", "wlayout" };

	if (!ubi) {
		char *mtdparts[] = { "mtdparts" };
		char *ubi_part[] = { "ubi", "part", "UBI_DEV" };

		/* Show MTD partition layout */
		setenv("mtdids", MTDIDS);
		setenv("mtdparts", MTDPARTS);
		do_mtdparts(NULL, 0, ARRAY_SIZE(mtdparts), mtdparts);

		printf("Initialize UBI device area!!\n");
		rcode = do_ubi(NULL, 0, ARRAY_SIZE(ubi_part), ubi_part);

		if (rcode && rcode != 1)
			return rcode;
	}

	for (p = &ubi_vol[0], id = 0; p->size; ++p, ++id) {
		q = p + 1;
		v = get_vol_by_name(ubi, p->name);
		if (v) {
			/* volume already exist. sanity check */
			if (v->vol_id != id) {
				printf("UBI volume [%s] id %d mismatch! (expect %d)\n",
					p->name, v->vol_id, id);
			}

			size = v->reserved_pebs * v->usable_leb_size;
			if (size < p->size && q->size)
				printf("UBI volume [%s] size %x smaller than %x!\n", p->name, size, p->size);
			continue;
		}

		show++;
		ubi_create_vol[2] = (char*) p->name;
		sprintf(tmp, "%x", p->size);
		/* If next volume size is zero, assign all available
		 * space to current volume instead of specified size.
		 */
		if (q->size == 0)
			strcpy(tmp, "0");
		if (p->type)
			strcpy(type, "dynamic");
		else
			strcpy(type, "static");
		rcode = do_ubi(NULL, 0, ARRAY_SIZE(ubi_create_vol), ubi_create_vol);
		if (rcode < 0)
			printf("Create volume %s fail. rcode 0x%x\n", p->name, rcode);

		if (!p->type) {
			char *ubi_write_vol[] = { "ubi", "writevol", tmp, (char*)p->name, tmp1 };

			sprintf(tmp, "%x", CFG_LOAD_ADDR);
			sprintf(tmp1, "%x", p->size);
			memset((void*)CFG_LOAD_ADDR, 0x00, p->size);
			do_ubi(NULL, 0, ARRAY_SIZE(ubi_write_vol), ubi_write_vol);
		}
	}

	if (show) {
		printf("Latest UBI volumes layout.\n");
		do_ubi(NULL, 0, ARRAY_SIZE(ubi_info_wlayout), ubi_info_wlayout);
	}

	return 0;
}
#endif	/* UBI_SUPPORT */

unsigned long ranand_init(void)
{
	int reg, chip_mode;
	struct ra_nand_chip *chip = &mt7620a_nandinfo;
	struct mtd_info *mtd = &mt7620a_mtdinfo;

#if defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD)
	//set NAND_SPI_SHARE to 3b'100
	ra_and(RALINK_SYSCTL_BASE+0x60, ~(0x7<<11));
	ra_or(RALINK_SYSCTL_BASE+0x60, (0x4<<11));
#endif

	//512 bytes per page
	ra_and(NFC_CONF1, ~1);
#if defined (RT6855A_ASIC_BOARD) || defined (RT6855A_FPGA_BOARD)
	reg = ra_inl(RALINK_SYSCTL_BASE+0x8c);
	chip_mode = ((reg>>28) & 0x3)|(((reg>>22) & 0x3)<<2);
#endif		
#if defined (MT7620_ASIC_BOARD) || defined (MT7620_FPGA_BOARD)
	//set NAND_SD_SHARE to 2b'00
	ra_and(RALINK_SYSCTL_BASE+0x60, ~(0x3<<18));
	reg = ra_inl(RALINK_SYSCTL_BASE+0x10);
	chip_mode = reg & 0xF;
#endif	

#if defined(ASUS_RTAC52U) && !defined(CFG_ENV_IS_IN_NAND) && defined(NAND_SUPPORT)
	chip_mode = 1;	/* forced chip mode as 1 */
#endif

	if((chip_mode==1)||(chip_mode==11)) {
		printf("!!! nand page size = 2048, addr len=%d\n", ((chip_mode!=11) ? 4 : 5));
		ra_or(NFC_CONF1, 1);
		is_nand_page_2048 = 1;
		nand_addrlen = ((chip_mode!=11) ? 4 : 5); 
	}
	else {
		printf("!!! nand page size = 512, addr len=%d\n", (chip_mode!=10) ? 3 : 4);
		ra_and(NFC_CONF1, ~1);
		is_nand_page_2048 = 0;
		nand_addrlen = ((chip_mode!=10) ? 3 : 4);
	}

	chip->numchips = CONFIG_NUMCHIPS;
	chip->chip_shift = CONFIG_CHIP_SIZE_BIT;
	chip->page_shift = CONFIG_PAGE_SIZE_BIT;
	chip->oob_shift	= CONFIG_OOBSIZE_PER_PAGE_BIT;
	chip->numpage_per_block_bit = CONFIG_NUMPAGE_PER_BLOCK_BIT;
	chip->block_size_bit = CONFIG_BLOCK_SIZE_BIT;
	chip->erase_shift = (CONFIG_PAGE_SIZE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT);
	chip->badblockpos = CONFIG_BAD_BLOCK_POS;
	chip->ecc_offset = CONFIG_ECC_OFFSET;
	chip->column_addr_cycle = CFG_COLUMN_ADDR_CYCLE;
	chip->addr_cycle = CFG_ADDR_CYCLE;
	chip->size = CFG_CHIPSIZE;
	chip->erasesize = CFG_BLOCKSIZE;
	chip->writesize = CFG_PAGESIZE;
	chip->oobsize = CFG_PAGE_OOBSIZE;
	chip->buffers = block_buffer;
	
	//config ECC location
	ra_and(NFC_CONF1, 0xfff000ff);
	ra_or(NFC_CONF1, ((chip->ecc_offset + 2) << 16) +
			((chip->ecc_offset + 1) << 12) +
			(chip->ecc_offset << 8));

	//maks sure gpio-0 is input
	ra_outl(RALINK_PIO_BASE+0x24, ra_inl(RALINK_PIO_BASE+0x24) & ~0x01);

	//set WP to high
	ra_or(NFC_CTRL, 0x01);

	if (nfc_all_reset() != 0)
		return -1;

	/* Disable the skip bad-block mechanism by default. */
	ranand_set_sbb_max_addr(0);

	/* Connect to MTD */
	memset(mtd, 0, sizeof(*mtd));
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->name = "nand0";
	mtd->size = CFG_CHIPSIZE;
	mtd->erasesize = chip->erasesize;
	mtd->writesize = chip->writesize;
	mtd->oobsize = chip->oobsize;
	mtd->index = 0;
	mtd->priv = chip;

#if defined(UBI_SUPPORT)
	mtd->oobavail = large_page_ecclayout.oobavail;
	mtd->ecclayout = &large_page_ecclayout;

	mtd->erase = mt7620a_mtd_erase;
	mtd->read = mt7620a_mtd_read;
	mtd->write = mt7620a_mtd_write;
	mtd->read_oob = mt7620a_mtd_read_oob;
	mtd->write_oob = mt7620a_mtd_write_oob;
	mtd->block_isbad = mt7620a_mtd_block_isbad;
	mtd->block_markbad = mt7620a_mtd_block_markbad;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->lock = NULL;
	mtd->unlock = NULL;

	add_mtd_device(mtd);
#endif

	return chip->size;
}


/**
 * generic function to get data from flash.
 * @return data length reading from flash.
 */
static int _ra_nand_pull_data(char *buf, int len)
{
#ifdef RW_DATA_BY_BYTE
	char *p = buf;
#else
	__u32 *p = (__u32 *)buf;
#endif
	int retry, int_st;
	unsigned int ret_data;
	int ret_size;

	retry = READ_STATUS_RETRY;
	while (len > 0) {
		int_st = ra_inl(NFC_INT_ST);
		if (int_st & INT_ST_RX_BUF_RDY) {
			ret_data = ra_inl(NFC_DATA);
			ra_outl(NFC_INT_ST, INT_ST_RX_BUF_RDY); 
#ifdef RW_DATA_BY_BYTE
			ret_size = sizeof(unsigned int);
			ret_size = min(ret_size, len);
			len -= ret_size;
			while (ret_size-- > 0) {
				//nfc is little endian 
				*p++ = ret_data & 0x0ff;
				ret_data >>= 8; 
			}
#else
			ret_size = min(len, 4);
			len -= ret_size;
			if (ret_size == 4)
				*p++ = ret_data;
			else {
				__u8 *q = (__u8 *)p;
				while (ret_size-- > 0) {
					*q++ = ret_data & 0x0ff;
					ret_data >>= 8; 
				}
				p = (__u32 *)q;
			}
#endif
			retry = READ_STATUS_RETRY;
		}
		else if (int_st & INT_ST_ND_DONE) {
			break;
		}
		else {
			udelay(1);
			if (retry-- < 0) 
				break;
		}
	}

#ifdef RW_DATA_BY_BYTE
	return (int)(p - buf);
#else
	return ((int)p - (int)buf);
#endif
}

/**
 * generic function to put data into flash.
 * @return data length writing into flash.
 */
static int _ra_nand_push_data(char *buf, int len)
{
#ifdef RW_DATA_BY_BYTE
	char *p = buf;
#else
	__u32 *p = (__u32 *)buf;
#endif
	int retry, int_st;
	unsigned int tx_data = 0;
	int tx_size, iter = 0;

	retry = READ_STATUS_RETRY;
	while (len > 0) {
		int_st = ra_inl(NFC_INT_ST);
		if (int_st & INT_ST_TX_BUF_RDY) {
#ifdef RW_DATA_BY_BYTE
			tx_size = min(len, (int)sizeof(unsigned long));
			for (iter = 0; iter < tx_size; iter++) {
				tx_data |= (*p++ << (8*iter));
			}
#else
			tx_size = min(len, 4);
			if (tx_size == 4)
				tx_data = (*p++);
			else {
				__u8 *q = (__u8 *)p;
				for (iter = 0; iter < tx_size; iter++)
					tx_data |= (*q++ << (8*iter));
				p = (__u32 *)q;
			}
#endif
			ra_outl(NFC_INT_ST, INT_ST_TX_BUF_RDY);
			ra_outl(NFC_DATA, tx_data);
			len -= tx_size;
			retry = READ_STATUS_RETRY;
		}
		else if (int_st & INT_ST_ND_DONE) {
			break;
		}
		else {
			udelay(1);
			if (retry-- < 0)
				break;
		}
	}
	
#ifdef RW_DATA_BY_BYTE
	return (int)(p - buf);
#else
	return ((int)p - (int)buf);
#endif
}

/** wait nand chip becomeing ready and return queried status.
 * @param snooze: sleep time in ms unit before polling device ready.
 * @return status of nand chip
 * @return NAN_STATUS_FAIL if something unexpected.
 */
static int nfc_wait_ready(int snooze_ms)
{
	int retry;
	char status;

	udelay(1000 * snooze_ms);

	// wait nfc idle,
	if (snooze_ms == 0)
		snooze_ms = 1;
	retry = snooze_ms * 1000; //udelay(1)

	while (!NFC_TRANS_DONE() && retry--) {
		udelay(1);
	}
	if (!NFC_TRANS_DONE()) {
		printf("%s: no transaction done\n", __func__);
		return NAND_STATUS_FAIL;
	}

#if !defined (CONFIG_NOT_SUPPORT_RB)
	while (!(status = nfc_device_ready()) && retry--) {
		udelay(1);
	}
	if (status == 0) {
		printf("%s: no device ready.\n", __func__);
		return NAND_STATUS_FAIL;
	}

	nfc_read_status(&status);
	return status;
#else
	while (retry--) {
		nfc_read_status(&status);
		if (status & NAND_STATUS_READY)
			break;
		udelay(1);
	}
	if (retry < 0) {
		printf("%s: no device ready, status(%x).\n", __func__, status);
		return NAND_STATUS_FAIL;
	}
	return status;
#endif
}

/**
 * return 0: erase OK
 * return -EIO: fail 
 */
int nfc_erase_block(int row_addr)
{
	unsigned long cmd1, cmd2, bus_addr, conf;
	char status;

	cmd1 = 0x60;
	cmd2 = 0xd0;
	bus_addr = row_addr;
	conf = 0x00511 | ((CFG_ROW_ADDR_CYCLE)<<16);

	ra_dbg("%s: cmd1: %lx, cmd2:%lx bus_addr: %lx, conf: %lx \n", 
	       __func__, cmd1, cmd2, bus_addr, conf);

	//fixme, should we check nfc status?
	CLEAR_INT_STATUS();

	ra_outl(NFC_CMD1, cmd1); 	
	ra_outl(NFC_CMD2, cmd2);
	ra_outl(NFC_ADDR, bus_addr);
	ra_outl(NFC_CONF, conf); 

	status = nfc_wait_ready(3);  //erase wait 3ms 
	if (status & NAND_STATUS_FAIL) {
		printf("%s: fail\n", __func__);
		return -1;
	}
	
	return 0;

}

static inline int nfc_read_raw_data(int cmd1, int cmd2, int bus_addr, int bus_addr2, int conf, char *buf, int len)
{
	int ret;

	ra_dbg("%s: cmd1 %x, cmd2 %x, addr %x %x, conf %x, len %x\n", __func__,
			cmd1, cmd2, bus_addr2, bus_addr, conf, len);
	CLEAR_INT_STATUS();
	ra_outl(NFC_CMD1, cmd1);
	ra_outl(NFC_CMD2, cmd2);
	ra_outl(NFC_ADDR, bus_addr);
#if defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) || \
	defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) || \
	defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD)	
	ra_outl(NFC_ADDR2, bus_addr2);
#endif	
	ra_outl(NFC_CONF, conf); 

	ret = _ra_nand_pull_data(buf, len);
	if (ret != len) {
		printf("%s: ret:%x (%x) \n", __func__, ret, len);
		return NAND_STATUS_FAIL;
	}

	ret = nfc_wait_ready(0); //wait ready
	/* to prevent the DATA FIFO 's old data from next operation */
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x02); //clear data buffer
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) & ~0x02); //clear data buffer

	if (ret & NAND_STATUS_FAIL) {
		printf("%s: fail\n", __func__);
		return NAND_STATUS_FAIL;
	}

	return 0;
}

static inline int nfc_write_raw_data(int cmd1, int cmd3, int bus_addr, int bus_addr2, int conf, char *buf, int len)
{
	int ret;

	ra_dbg("%s: cmd1 %x, cmd3 %x, addr %x %x, conf %x, len %x\n", __func__,
			cmd1, cmd3, bus_addr2, bus_addr, conf, len);
	CLEAR_INT_STATUS();
	ra_outl(NFC_CMD1, cmd1);
	ra_outl(NFC_CMD3, cmd3);
	ra_outl(NFC_ADDR, bus_addr);
#if defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) || \
	defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) || \
	defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD)	
	ra_outl(NFC_ADDR2, bus_addr2);
#endif	
	ra_outl(NFC_CONF, conf); 

	ret = _ra_nand_push_data(buf, len);
	if (ret != len) {
		printf("%s: ret:%x (%x) \n", __func__, ret, len);
		return NAND_STATUS_FAIL;
	}

	ret = nfc_wait_ready(1); //write wait 1ms
	/* to prevent the DATA FIFO 's old data from next operation */
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x02); //clear data buffer
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) & ~0x02); //clear data buffer

	if (ret & NAND_STATUS_FAIL) {
		printf("%s: fail\n", __func__);
		return NAND_STATUS_FAIL;
	}

	return 0;
}

char *ranand_id(void)
{
	char *type_name = "Unknown type";
	int maf_id, maf_idx, dev_id;
	unsigned char id[4];
	struct nand_flash_dev *type = nand_flash_ids;
	static char buf[256];

	nfc_read_raw_data(0x90, 0, 0, 0, 0x410141, id, 4);
	maf_id = id[0];
	dev_id = id[1];
	/* Try to identify manufacturer */
	for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
		if (nand_manuf_ids[maf_idx].id == maf_id)
			break;
	}

	for (; type->name != NULL; type++) {
		if (dev_id == type->id) {
			type_name = type->name;
                        break;
		}
	}

#if defined(UBOOT_STAGE1)
	sprintf(buf, "0x%02X 0x%02X 0x%02X 0x%02X (%s)",
		maf_id, dev_id, id[2], id[3], nand_manuf_ids[maf_idx].name);
#else
	sprintf(buf, "0x%02X 0x%02X 0x%02X 0x%02X (%s, %s)",
		maf_id, dev_id, id[2], id[3], nand_manuf_ids[maf_idx].name, type_name);
#endif

	return buf;
}

/**
 * @return !0: fail
 * @return 0: OK
 */
int nfc_read_oob(int page, unsigned int offs, char *buf, int len)
{
	unsigned int cmd1 = 0, cmd2 = 0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int status;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	bus_addr = (page << (chip->column_addr_cycle * 8)) | (offs & ((1 << chip->column_addr_cycle * 8) - 1));

	if (is_nand_page_2048) {
		bus_addr += chip->writesize;
		bus_addr2 = page >> (chip->column_addr_cycle*8);
		cmd1 = 0x0;
		cmd2 = 0x30;
		conf = 0x000511| ((chip->addr_cycle) << 16) | (len << 20);
	}
	else {
		cmd1 = 0x50;
		conf = 0x000141| ((chip->addr_cycle) << 16) | (len << 20);
	}
	conf |= (1<<3);

	ra_dbg("%s: cmd1:%x, bus_addr:%x, conf:%x, len:%x\n", __func__, cmd1, bus_addr, conf, len);
	status = nfc_read_raw_data(cmd1, cmd2, bus_addr, bus_addr2, conf, buf, len);
	if (status & NAND_STATUS_FAIL) {
		printf("%s: fail \n", __func__);
		return -1;
	}

	return 0;
}

/**
 * @return !0: fail
 * @return 0: OK
 */
int nfc_write_oob(int page, unsigned int offs, char *buf, int len)
{
	unsigned int cmd1 = 0, cmd3=0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int status;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	bus_addr = (page << (chip->column_addr_cycle * 8)) | (offs & ((1 << chip->column_addr_cycle * 8) - 1));
	if (is_nand_page_2048) {
		cmd1 = 0x80;
		cmd3 = 0x10;
		bus_addr += chip->writesize;
		bus_addr2 = page >> (chip->column_addr_cycle * 8);
		conf = 0x001123 | ((chip->addr_cycle) << 16) | ((len) << 20);
	}
	else {
		cmd1 = 0x08050;
		cmd3 = 0x10;
		conf = 0x001223 | ((chip->addr_cycle) << 16) | ((len) << 20);
	}

	// set NFC
	ra_dbg("%s: cmd1: %x, cmd3: %x bus_addr: %x, conf: %x, len:%x\n",
			__func__, cmd1, cmd3, bus_addr, conf, len);
	status = nfc_write_raw_data(cmd1, cmd3, bus_addr, bus_addr2, conf, buf, len);
	if (status & NAND_STATUS_FAIL) {
		printf("%s: fail \n", __func__);
		return -1;
	}

	return 0;
}

/* @return
 * 	number of bit is equal to 1
 */
static int bit_count(unsigned int val)
{
	int b = 0;

	while (val) {
		if ((val & 1) != 0)
			b++;

		val >>= 1;
	}

	return b;
}

/* verify a page
 * @return
 * 	0:	success
 *    -1:	invalid parameter
 *  -EUCLEAN:	corrected data, success
 *  -EBADMSG:	uncorrectable data, fail
 *  -EIO:	I/O error
 */
int nfc_ecc_verify(unsigned char *buf, int page, int mode)
{
	int ret, i, j, bcnt = -1, byte_addr = -1, bit_addr = -1;
	int clean = 0, empty = 0, base;
	unsigned int corrected = 0, err = 0;
	unsigned int ecc, ecc_err;
	unsigned char *d, *p, *q, *data;
	unsigned char empty_ecc[CONFIG_ECC_BYTES], empty_data[4];
	unsigned char new_ecc[CONFIG_ECC_BYTES], ecc_xor[CONFIG_ECC_BYTES];
	struct mtd_info *mtd = &mt7620a_mtdinfo;
	struct mtd_ecc_stats *ecc_stats = &mtd->ecc_stats;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;
	const int ecc_steps = mtd->writesize / 512;
	const char *dir = (mode == FL_READING)?"read":"write";

	if (!buf)
		return -1;

	//ra_dbg("%s, page:%x mode:%d\n", __func__, page, mode);
	if (mode == FL_WRITING) {
		int len = chip->writesize + chip->oobsize;
		int conf = 0x000141| ((chip->addr_cycle) << 16) | (len << 20);
		unsigned char rbbuf[chip->writesize + chip->oobsize];
		conf |= (1<<3); //(ecc_en)

		p = rbbuf;
		ret = nfc_read_page(p, page);
		if (ret >= 0 || ret == -EUCLEAN)
			goto ecc_check;

		//FIXME, double comfirm
		printf("%s: read back fail, try again. (ret %d)\n",__func__, ret);
		ret = nfc_read_page(p, page);
		if (ret >= 0 || ret == -EUCLEAN) {
			goto ecc_check;
		} else {
			printf("%s: read back fail agian. (ret %d)\n",__func__, ret);
			return -EIO;
		}
	}
	else if (mode == FL_READING) {
		p = buf;
	}
	else {
		printf("%s: mode %d, return -2\n", __func__, mode);
		return -2;
	}

ecc_check:
	data = p;
	p += chip->writesize;
	memset(empty_ecc, 0xFF, sizeof(empty_ecc));
	for (i = 0, base = 0, q = p + chip->ecc_offset;
		i < ecc_steps;
		++i, base += 512, data += 512, p += 512, q += 16)
	{
		if (!i) {
			for (j = 0, clean = ecc_steps; clean == ecc_steps && j < ecc_steps; ++j) {
				if (ra_inl(NFC_ECC + 4*i) != 0)	/* not clean page */
					clean--;
			}
			if (clean == ecc_steps)
				break;

			for (j = 0, empty = ecc_steps; empty == ecc_steps && j < ecc_steps; ++j) {
				if (memcmp(q + 16 * j, empty_ecc, CONFIG_ECC_BYTES))
					empty--;
			}
			if (empty == ecc_steps) {
				/* If all ECC in a page is equal to 0xFFFFFF, check data again.
				 * ECC of some data is equal to 0xFFFFFF too.
				 */
				memset(empty_data, 0xFF, sizeof(empty_data));
				for (j = 0, empty = 1, d = data;
					empty && j < mtd->writesize;
					j+=sizeof(empty_data), d+=sizeof(empty_data))
				{
					if (memcmp(d, empty_data, sizeof(empty_data)))
						empty = 0;
				}
				if (empty) {
					printf("skip ecc 0xff at page %x\n", page);
					break;
				}
			}
		}

		ecc = ra_inl(NFC_ECC_P1 + 4*i);
		new_ecc[0] = ecc & 0xFF;
		new_ecc[1] = (ecc >> 8) & 0xFF;
		new_ecc[2] = (ecc >> 16) & 0xFF;
		if (!memcmp(q, new_ecc, CONFIG_ECC_BYTES))
			continue;

		ecc_xor[0] = *q ^ new_ecc[0];
		ecc_xor[1] = *(q + 1) ^ new_ecc[1];
		ecc_xor[2] = *(q + 2) ^ new_ecc[2];
		bcnt = bit_count(ecc_xor[2] << 16 | ecc_xor[1] << 8 | ecc_xor[0]);
		ecc_err = ra_inl(NFC_ECC_ERR1 + 4*i);

		printf("%s mode:%s, invalid ecc, page: %x, sub-page %d, "
			"read:%02x %02x %02x, ecc:%02x %02x %02x, xor: %02x %02x %02x, bcnt %d \n",
			__func__, dir, page, i, *(q), *(q + 1), *(q + 2),
			new_ecc[0], new_ecc[1], new_ecc[2], ecc_xor[0], ecc_xor[1], ecc_xor[2], bcnt);
		printf("HW:ECC_ERR%d: byte_addr 0x%x bit_addr %d hwecc %d. (0x%08x)\n",
			i+1, base + ((ecc_err >> 6) & 0x1FF), (ecc_err >> 2) & 0x7, ecc_err & 1, ecc_err);
		if (bcnt == 12) {
			/* correctable error */
			bit_addr = ((ecc_xor[2] >> 5) & 0x4) | ((ecc_xor[2] >> 4) & 0x2) | ((ecc_xor[2] >> 3) & 0x1);
			byte_addr = (((unsigned int)ecc_xor[2] << 7) & 0x100) |
				    (ecc_xor[1] & 0x80) |
				    ((ecc_xor[1] << 1) & 0x40) |
				    ((ecc_xor[1] << 2) & 0x20) |
				    ((ecc_xor[1] << 3) & 0x10) |
				    ((ecc_xor[0] >> 4) & 0x08) |
				    ((ecc_xor[0] >> 3) & 0x04) |
				    ((ecc_xor[0] >> 2) & 0x02) |
				    ((ecc_xor[0] >> 1) & 0x01);
			*(data + byte_addr) ^= 1 << bit_addr;
			corrected++;
			printf("SW:ECC     : byte_addr 0x%x bit_addr %d\n", base + byte_addr, bit_addr);
		} else if (bcnt == 1) {
			/* ECC code error */
			printf("ECC code error!\n");
			err++;
		} else {
			/* Uncorrectable error */
			printf("Uncorrectable error!\n");
			err++;
		}
	}

	ecc_stats->failed += err;
	ecc_stats->corrected += corrected;
	if (err)
		return -EBADMSG;
	else if (corrected)
		return -EUCLEAN;

	return 0;
}

/**
 * @return -EIO, writing size is less than a page 
 * @return
 * 	0:	OK
 *     -1:	invalid parameter
 *  -EUCLEAN:	corrected, ok
 *  -EBADMSG:	uncorrectable error or ECC code error
 *     <0:	error
 */
int nfc_read_page(unsigned char *buf, int page)
{
	unsigned int cmd1 = 0, cmd2 = 0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int size, offs;
	int status = 0;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	if (!buf)
		return -1;

	page = page & (chip->size - 1); // chip boundary
	size = chip->writesize + chip->oobsize; //add oobsize
	offs = 0;

	while (size > 0) {
		int len;

		len = size;
		bus_addr = (page << (chip->column_addr_cycle * 8)) | (offs & ((1 << chip->column_addr_cycle * 8)-1));
		if (is_nand_page_2048) {
			bus_addr2 = page >> (chip->column_addr_cycle * 8);
			cmd1 = 0x0;
			cmd2 = 0x30;
			conf = 0x000511| ((chip->addr_cycle) << 16) | (len << 20);
		}
		else {
			if (offs & ~(chip->writesize - 1))
				cmd1 = 0x50;
			else if (offs & ~((1 << chip->column_addr_cycle * 8)-1))
				cmd1 = 0x01;
			else
				cmd1 = 0;

			conf = 0x000141| ((chip->addr_cycle) << 16) | (len << 20);
		}
		conf |= (1<<3); 

		status = nfc_read_raw_data(cmd1, cmd2, bus_addr, bus_addr2, conf, buf+offs, len);
		if (status & NAND_STATUS_FAIL) {
			printf("%s: fail \n", __func__);
			return -EIO;
		}

		offs += len;
		size -= len;
	}

	// verify and correct ecc
	status = nfc_ecc_verify(buf, page, FL_READING);	
	if (status == -EUCLEAN) {
		printf("%s: corrected, buf:%x, page:%x\n", __func__, buf, page);
		return status;
	} else if (status != 0) {
		printf("%s: fail, buf:%x, page:%x, status %d\n", __func__, buf, page, status);
		return status;
	}

	return 0;
}

/** 
 * Write a page + OOB.
 * @return -EIO, fail to write
 * @return
 * 	0:	OK
 *   -EIO:	Write fail or ECC verify fail, correctable or uncorrectable
 *     <0:	error
 */
int nfc_write_page(unsigned char *buf, int page, int flags)
{
	unsigned int cmd1 = 0, cmd3, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int size;
	int status;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	page = page & (chip->size - 1); //chip boundary
	size = chip->writesize + chip->oobsize; //add oobsize
	bus_addr = (page << (chip->column_addr_cycle * 8)); //write_page always write from offset 0.

	if (is_nand_page_2048) {
		bus_addr2 = page >> (chip->column_addr_cycle * 8);
		cmd1 = 0x80;
		cmd3 = 0x10;
		conf = 0x001123| ((chip->addr_cycle) << 16) | (size << 20);
	}
	else {
		cmd1 = 0x8000;
		cmd3 = 0x10;
		conf = 0x001223| ((chip->addr_cycle) << 16) | (size << 20);
	}

	if (flags & FLAG_ECC_EN)
		conf |= (1<<3); //enable ecc

	status = nfc_write_raw_data(cmd1, cmd3, bus_addr, bus_addr2, conf, buf, size);
	if (status & NAND_STATUS_FAIL)
		return -EIO;

	if (!(flags & FLAG_ECC_EN))
		return 0;
	
	status = nfc_ecc_verify(buf, page, FL_WRITING);
	if (status != 0)
		return -EIO;

	return 0;
}

#if defined(CONFIG_BADBLOCK_CHECK)
int ranand_block_isbad(loff_t offs)
{
	unsigned int tag1, tag2;
	int page, ret1, ret2;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	page = offs >> chip->page_shift;
	ret1 = nfc_read_oob(page, chip->badblockpos, (char*)&tag1, 1);
	ret2 = nfc_read_oob(page + 1, chip->badblockpos, (char*)&tag2, 1);

	if (ret1 || ret2) {
		printf("%s(): read page 0x%x,0x%x error. (%d,%d)\n", __func__, page, page+1, ret1, ret2);
		return -1;
	}

	tag1 &= 0xFF;
	tag2 &= 0xFF;

	if (tag1 != 0xFF || tag2 != 0xFF)
		return 1;

	return 0;
}
#endif

/** Erase blocks.
 * @offs:	block alilgned address
 * @len:	block aligned length
 * @return:
 * 	0:	success
 *     >0:	success and bad-block encountered
 *  -EIO:	I/O error
 *     <0:	invalid parameter or error
 */
int ranand_erase(unsigned int offs, int len)
{
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;
	unsigned int blocksize = chip->erasesize;
	int page, status;
	int ret = 0;

	ra_dbg("%s: start:%x, len:%x \n", __func__, offs, len);
	if (BLOCK_ALIGNED(offs) || BLOCK_ALIGNED(len)) {
		printf("%s: erase block not aligned, addr:%x len:%x %x\n", __func__, offs, len, chip->erasesize);
		return -1;
	}

	while (len > 0) {
		if (offs >= chip->size) {
			printf("%s: over-run. offs 0x%x\n", __func__, offs);
			break;
		} else if (max_addr && offs >= max_addr) {
			printf("%s: sbb exceed max_addr 0x%x\n", __func__, max_addr);
			break;
		}

		page = (int)(offs >> chip->page_shift);

		/* select device and check wp */
		if (nfc_check_wp()) {
			printf("%s: nand is write protected\n", __func__);
			return -1;
		}

#ifdef CONFIG_BADBLOCK_CHECK
		/* if we have a bad block, we do not erase bad blocks */
		if (ranand_block_isbad(offs)) {
			int page, tag1 = 0, ret1;

			printf("%s: attempt to erase a bad block at 0x%08x\n", __func__, offs);
			page = offs >> chip->page_shift;
			ret1 = nfc_read_oob(page, chip->badblockpos, (char*)&tag1, 1);
			tag1 &= 0xFF;
			if (ret1 || (tag1 != 0x33) || (tag1 == 0x33 && dont_erase_bad_block)) {
				ret++;
				offs += blocksize;
				if (!max_addr)
					len -= blocksize;

				continue;
			} else {
				printf("bad block mark equal to 0x33. Erase it!!!\n");
			}
		}
#endif

		status = nfc_erase_block(page);
		/* See if block erase succeeded */
		if (status) {
			printf("%s: failed erase, page 0x%08x\n", __func__, page);
			return -EIO;
		}

		/* Increment page address and decrement length */
		len -= blocksize;
		offs += blocksize;
	}

	return ret;
}

/** Write data to NAND Flash
 * If error occurs during write to page P of block B, this function returns -EIO immediately.
 * Caller of ranand_write() MUST mark block B as bad-block, erase block B + 1 and
 * call this function again, to write data to block B + 1.
 * WARNING:
 * 	If "to" is not aligned to page address, data prior to "to" in same page would be overwritted by 0xFF!
 * 	It is not good to program NAND flash more than one times without erase procedure.
 * @buffer:	pointer to data
 * @to:		start offset address of NAND Flash to be written
 * @datalen:	number of bytes to be written
 * @return:
 *  >=0		length of writted bytes
 *  < 0		error occurs
 *  -EIO:	write page fail.
 *  -EAGAIN:	write-disturb, suggested caller to erase and write again.
 */
int ranand_write(unsigned char *buf, unsigned int to, int datalen)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	struct ra_nand_chip *chip = &mt7620a_nandinfo;
	struct mtd_ecc_stats stats = mtd->ecc_stats;
	uint32_t failed, corrected;
	int blk = to >> chip->block_size_bit, old_blk = -1;
	int page, i = 0;
	int len, ret, offs, rlen = 0;
	size_t retlen = 0;
	int pagemask = (chip->writesize -1), blockmask = (chip->erasesize - 1);
	unsigned int addr = to, addr1, old_blk_addr;
	unsigned char buffers[chip->writesize + chip->oobsize], v_buf[mtd->erasesize];

	debug("%s: to:%x len:%x \n", __func__, to, datalen);
	if (buf == 0)
		datalen = 0;
	
	// page write
	while (datalen > 0) {
		if (addr >= chip->size) {
			printf("%s: over-run. addr 0x%x\n", __func__, addr);
			break;
		} else if (max_addr && addr >= max_addr) {
			printf("%s: sbb exceed max_addr 0x%x\n", __func__, max_addr);
			break;
		}

		page = addr >> chip->page_shift;
		/* check bad-block per block */
		if (blk != old_blk) {
			old_blk = blk;
			if (ranand_block_isbad(blk << chip->block_size_bit)) {
				printf("%s: found bad-block 0x%x page 0x%x\n", __func__, blk, page);
				if (!max_addr) {
					offs = addr & blockmask;
					len = min(datalen, chip->erasesize - offs);

					printf("%s: SKIP %d BYTES!\n", __func__, len);
					buf += len;
					datalen -= len;
					retlen += len;
					addr = (blk+1) << chip->block_size_bit;
					continue;
				} else {
					addr1 = (blk+1) << chip->block_size_bit;
					printf("%s: skip bad block at %x, write to %x instead\n",
						__func__, addr, addr1);
					addr = addr1;
					continue;
				}
			}
		}
		
		ra_dbg("%s (%d): addr:%x, pg:%x, data:%p, datalen:%x\n", 
			__func__, i, addr, page, buf, datalen);
		++i;

		/* select chip, and check if it is write protected */
		if (nfc_check_wp()) {
			printf("%s: nand is write protected\n", __func__);
			return -1;
		}

		memset(buffers, 0x0ff, sizeof(buffers));

		// data write
		offs = addr & pagemask;
		len = min(datalen, chip->writesize - offs);
		if (buf && len > 0)
			memcpy(buffers + offs, buf, len);	// we can not sure ops->buf wether is DMA-able.

		addr = (page+1) << chip->page_shift;
		blk = addr >> chip->block_size_bit;
		ret = nfc_write_page(buffers, page, FLAG_ECC_EN);
		if (ret == -EUCLEAN)
			return -EAGAIN;
		else if (ret < 0)
			return -EIO;

		rlen += mtd->writesize;
		/* If we finish the write action to last page of a block,
		 * read the block back and check whether there are any bit-flips.
		 * If yes, notify caller to erase and write again.
		 * Because correct page may be broken due to write-disturb issue.
		 */
		if (blk != old_blk || (datalen - len) == 0) {
			old_blk_addr = old_blk << chip->block_size_bit;
			if (to > old_blk_addr) {
				/* first block of data and to is not a block-aligned address */
				old_blk_addr = to;
			}
			stats = mtd->ecc_stats;
			ret = ranand_read(v_buf, old_blk_addr, rlen);
			failed = mtd->ecc_stats.failed - stats.failed;
			corrected = mtd->ecc_stats.corrected - stats.corrected;
			if (ret != rlen || failed > 0 || corrected > 0) {
				printf("Verify addr 0x%x, len 0x%x fail. (ret %d, failed %d, corrected %d)\n",
					old_blk_addr, rlen, ret, failed, corrected);
				return (ret != rlen || failed > 0)? -EIO:-EAGAIN;
			}
			rlen = 0;
		}

		buf += len;
		datalen -= len;
		retlen += len;
	}

	return retlen;
}

/* read data from NAND Flash
 * @return:
 *     >0:	success, number of read bytes.
 *     =0:	none of any bytes are read
 *     -1:	invalid parameter
 *  otherwise:	not defined.
 *  If return value is less than datalen, it may be caused by uncorrectable error.
 */
int ranand_read(unsigned char *buf, unsigned int from, int datalen)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;
	int len, ret, offs;
	int blk, blk1 = -1, page, i = 0;
	size_t retlen = 0;
	unsigned int pagemask = (chip->writesize - 1), blockmask = (chip->erasesize - 1);
	loff_t addr = from;
	unsigned char buffers[chip->writesize + chip->oobsize];

	debug("%s: from:%x len:%x \n", __func__, from, datalen);
	if (!buf || from >= mtd->size || datalen <= 0 || (from + datalen) > mtd->size)
		return -1;

	for (addr = from, page = addr >> chip->page_shift;
	     datalen > 0;
	     addr = (page + 1) << chip->page_shift, buf += len, datalen -= len)
	{
		if (addr >= chip->size) {
			printf("%s: over-run. addr 0x%x\n", __func__, addr);
			break;
		} else if (max_addr && addr >= max_addr) {
			printf("%s: sbb exceed max_addr 0x%x\n", __func__, max_addr);
			break;
		}
		ra_dbg("%s (%d): addr:%x, datalen:%x\n", 
			__func__, i, (unsigned int)addr, datalen);
		++i;

		page = addr >> chip->page_shift;
		blk = addr >> chip->block_size_bit;

		/* check bad-block per block */
		if (blk != blk1 && ranand_block_isbad(blk << chip->block_size_bit)) {
			blk1 = blk;
			printf("%s: found bad-block 0x%x page 0x%x\n", __func__, blk, page);
			if (!max_addr) {
				offs = addr & blockmask;
				len = min(datalen, chip->erasesize - offs);

				printf("%s: copy %d bytes fake data (0x00)\n", __func__, len);
				memset(buf, 0, len);
				buf += len;
				datalen -= len;
				retlen += len;
				addr = (blk+1) << chip->block_size_bit;
				page = (addr >> chip->page_shift) - 1;
				continue;
			} else {
				printf("%s: skip reading a bad block %x ->", __func__, (unsigned int)addr);
				addr = (blk+1) << chip->block_size_bit;
				page = (addr >> chip->page_shift) - 1;
				len = 0;
				printf(" %x\n", (unsigned int)addr);
				continue;
			}
		}

		if (datalen > (chip->writesize + chip->oobsize) && (page & 0x1f) == 0)
			printf(".");
		ret = nfc_read_page(buffers, page);
		//FIXME, something strange here, some page needs 2 more tries to guarantee read success.
		if (ret < 0 && ret != -EUCLEAN) {
			printf("%s: read again. (ret %d)\n", __func__, ret);
			ret = nfc_read_page(buffers, page);

			if (ret < 0 && ret != -EUCLEAN) {
				printf("%s: read again fail. (ret %d)\n", __func__, ret);
			}
			else {
				printf("%s: read agian susccess. (ret %d) \n", ret);
			}
		}

		// data read
		offs = addr & pagemask;
		len = min(datalen, chip->writesize - offs);
		if (ret  && ret != -EUCLEAN && !max_addr) {
			printf("%s: read from page 0x%x fail. copy %d bytes fake data (0x00)!!!\n", __func__, page, len);
			memset(buf, 0, len);

			continue;
		}

		if ((!ret || ret == -EUCLEAN) && buf && len > 0)
			memcpy(buf, buffers + offs, len); // we can not sure ops->buf wether is DMA-able.

		retlen += len;
	}

	if (datalen > (chip->writesize + chip->oobsize))
		printf("\n");

	return retlen;
}

/* erase and write to nand flash
 * @buf:
 * @offs:
 * @count:
 * @return:
 *    >=0:	success, number of bytes be written to flash
 *     -1:	invalid parameter
 *  otherwise:	error
 */
int ranand_erase_write(unsigned char *buf, unsigned int offs, int count)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	const int blockmask = mtd->erasesize - 1;
	struct ra_nand_chip *chip = &mt7620a_nandinfo;
	unsigned int piece, blockaddr, o, rlen;
	unsigned char *data, *p, blk_buf[mtd->erasesize];
	int len = count, piece_size, ret, retry, wlen = 0;

	debug("%s: offs:%x, count:%x\n", __func__, offs, count);
	if (!buf || offs >= mtd->size || count <= 0 || offs + count > mtd->size)
		return -1;

	for (data = buf, o = offs; len > 0 && o < mtd->size; o += mtd->erasesize) {
		p = data;
		blockaddr = o & ~blockmask;
		if (blockaddr >= mtd->size) {
			printf("%s: over-run. addr 0x%x\n", __func__, blockaddr);
			break;
		} else if (max_addr && blockaddr >= max_addr) {
			printf("%s: sbb exceed max_addr 0x%x\n", __func__, max_addr);
			break;
		}

		if (ranand_block_isbad(blockaddr)) {
			printf("%s: skip bad block at 0x%x\n", __func__, blockaddr);
			continue;
		}

		/* if offset is not aligned to block address or
		 * remain length is smaller than block size,
		 * read data from NAND Flash and update data.
		 */
		rlen = mtd->erasesize;
		if (BLOCK_ALIGNED(o) || (len < mtd->erasesize)) {
			p = blk_buf;
			memset(p, 0, mtd->erasesize);
			/* backup original data */
			ret = ranand_read(p, blockaddr, mtd->erasesize);
			if (ret == mtd->erasesize) {
				/* read data from block ok */
			} else {
				/* mark as bad-block, jump to next block */
				printf("%s: read data from 0x%x fail, mark as bad-block! (ret %d)\n",
					__func__, blockaddr, ret);
				nand_block_markbad(chip, blockaddr);
				continue;
			}

			piece = o & blockmask;
			rlen = piece_size = min(len, mtd->erasesize - piece);
			memcpy(p + piece, data, piece_size);
		}

		retry = 0;
retry_erase_write:
		/* erase block */
		ret = ranand_erase(blockaddr, mtd->erasesize);
#ifdef CONFIG_BADBLOCK_CHECK
		if (ret >= 1) {
			printf("bad block: %x, try next: %x",
				blockaddr, blockaddr + mtd->erasesize);
			continue;
		} else
#endif
		if (ret < 0) {
			/* mark as bad-block, jump to next block */
			printf("%s: erase offset 0x%x fail, mark as bad-block! (ret %d)\n",
				__func__, blockaddr, ret);
			nand_block_markbad(chip, blockaddr);
			continue;
		}

		/* write a block */
		ret = ranand_write(p, blockaddr, mtd->erasesize);
		if (ret == mtd->erasesize) {
			/* success */
			wlen += rlen;
			data += rlen;
			len -= rlen;
			o = blockaddr;
			continue;
		}

		/* error occurs */
		printf("%s: write to block at offset 0x%x, return %d, retry %d\n",
			__func__, blockaddr, ret, retry);
		if (++retry <= 3)
			goto retry_erase_write;

		printf("%s: write to offset 0x%x error, retry %d. mark as bad-block\n",
			__func__, blockaddr, retry);
		nand_block_markbad(chip, blockaddr);
	}

	if (len)
		printf("%s: write %d bytes, remains %d bytes\n", __func__, count, count - len);

	printf("Done!\n");
	return wlen;
}

/**
 * Check whether have enough space in flash to program data or not
 * @offset:	start offset
 * @len:	data length
 * @bound:	upper bound offset
 * @return:
 *     >0:	have enough room to program data, number of bytes to add
 * 	0:	no enough room to program data
 *     -1:	invalid parameter
 */
int ranand_check_space(unsigned int offset, unsigned int len, unsigned int bound)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	const unsigned int blockmask = mtd->erasesize - 1;
	int space = 0;
	unsigned int o, blk_addr, l;

	if (offset >= mtd->size || len >= mtd->size || (offset + len) > mtd->size || bound >= mtd->size)
		return -1;

	if (bound <= offset || len > (bound - offset))
		return 0;
	for (o = offset; o < bound; o += mtd->erasesize) {
		blk_addr = o & ~blockmask;
		if (ranand_block_isbad(blk_addr))
			continue;

		l = mtd->erasesize;
		if (o != blk_addr)
			l -= o - blk_addr;

		space += l;
		o = blk_addr;
	}

	return (space >= len)? space:0;
}

#define NAND_FLASH_DBG_CMD
#ifdef NAND_FLASH_DBG_CMD
int ralink_nand_command(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	int len, i, ret;
	u8 *p = NULL;
	unsigned long off;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	if (!strncmp(argv[1], "id", 3)) {
		printf("flash id: %s\n", ranand_id());
	}
#if !defined(UBOOT_STAGE1)
	else if (!strncmp(argv[1], "read", 5)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		len = (int)simple_strtoul(argv[3], NULL, 16);
		p = (u8 *)malloc(len);
		if (!p) {
			printf("malloc error\n");
			return 0;
		}
		len = ranand_read(p, addr, len); //reuse len
		printf("read len: %d\n", len);
		for (i = 0; i < len; i++) {
			printf("%02x ", p[i]);
		}
		printf("\n");
		free(p);
	}
#endif
	else if (!strncmp(argv[1], "page", 5)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16); //page
		p = (u8 *)malloc(chip->writesize + chip->oobsize);
		nfc_read_page(p, addr);
		printf("page 0x%x:\n", addr);
		for (i = 0; i < chip->writesize; i++)
			printf("%02x%c", p[i], (i%32 == 31)? '\n':' ');
		printf("oob:\n");
		for (; i < chip->writesize + chip->oobsize; i++)
			printf("%02x%c", p[i], (i%32 == 31)? '\n':' ');
		free(p);
		printf("\n");
	}
	else if (!strncmp(argv[1], "erase", 6)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		len = (int)simple_strtoul(argv[3], NULL, 16);
		if (ranand_erase(addr, len) != 0)
			printf("erase failed\n");
		else
			printf("erase succeed\n");
	}
#if !defined(UBOOT_STAGE1)
	else if (!strncmp(argv[1], "write", 6)) {
		unsigned int o, l;
		u8 t[3] = {0};

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
		ret = ranand_write(p, o, l);
		if (ret != l)
			printf("nand write %d bytes to offset 0x%x fail. (ret %d)\n", l, o);
		free(p);
	}
	else if (!strncmp(argv[1], "oob", 4)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16); //page
		p = (u8 *)malloc(chip->oobsize);
		nfc_read_oob(addr, 0, p, chip->oobsize);
		printf("oob page %x (addr %x):\n", addr, (addr << chip->page_shift));
		for (i = 0; i < chip->oobsize; i++)
			printf("%02x%c", p[i], (i%32 == 31)? '\n':' ');
		free(p);
	}
#endif
	else if (!strncmp(argv[1], "init", 5)) {
		ranand_init();
	}
	else if (!strncmp(argv[1], "bad", 3)) {
		printf("CHIPSIZE 0x%x BLOCKSIZE 0x%x PAGESIZE 0x%x\n", chip->size, chip->erasesize, chip->writesize);
		printf("NAND Flash bad-block:\n");
		printf("    0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999\n");
		printf("    0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
		for (i = 0, off = 0; off < chip->size; off += chip->erasesize, ++i) {
			if (!(i%100))
				printf("\n%03d ", i/100);
			if (!ranand_block_isbad(off))
				printf(".");
			else
				printf("B");
		}
		printf("\n");
	}
#if !defined(UBOOT_STAGE1)
	else if (!strncmp(argv[1], "markbad", 7)) {
		int status;
		unsigned int blk, blk_offs, page;

		blk = simple_strtoul(argv[2], NULL, 16);
		page = blk << chip->numpage_per_block_bit;
		blk_offs = blk << chip->block_size_bit;
		printf("Mark block 0x%lx (page 0x%lx, offset 0x%x) as bad-block: ",
			blk, page, blk_offs);

		status = nand_block_markbad(&mt7620a_nandinfo, blk_offs);
		if (status == 0)
			printf("success\n");
		else
			printf("failed\n");
	}
	else if (!strncmp(argv[1], "erasebad", 8)) {
		int status;
		unsigned long blk, blk_offs, page;
		unsigned char buf[chip->oobsize];

		blk = simple_strtoul(argv[2], NULL, 16);
		page = blk << chip->numpage_per_block_bit;
		blk_offs = blk << chip->block_size_bit;
		printf("Erase bad-block 0x%lx (page 0x%lx): ", blk, page);

		status = nfc_read_oob(page, 0, buf, chip->oobsize);
		if (status) {
			printf("read fail. reject\n");
			return 0;
		}

		if (buf[chip->badblockpos] == 0xFF) {
			printf("good block.\n");
			return 0;
		} else if (buf[chip->badblockpos] != 0x33) {
			printf("invalid magic number. reject\n");
			return 0;
		}

		ranand_set_sbb_max_addr(0);
		ranand_set_allow_erase_badblock(1);
		status = ranand_erase(blk_offs, chip->erasesize);
		ranand_set_allow_erase_badblock(0);
		if (!status)
			printf("success\n");
		else
			printf("failed\n");
	}
	else if (!strncmp(argv[1], "load", 4)) {
		/* nand load <from:nand_offset> <to:ram_addr> <len> */
		unsigned long nand_offset, ram_addr;

		nand_offset = simple_strtoul(argv[2], NULL, 16);
		ram_addr = simple_strtoul(argv[3], NULL, 16);
		len = simple_strtoul(argv[4], NULL, 16);

		if (nand_offset >= chip->size || len <= 0 || (nand_offset + len) >= chip->size ||
		    ram_addr >= CFG_FLASH_BASE || (ram_addr + len) >= CFG_FLASH_BASE) {
			printf("Invalid parameter: nand_offset %lx ram_addr %lx len %d\n", nand_offset, ram_addr, len);
			return 0;
		}

		len = ranand_read((unsigned char*) ram_addr, nand_offset, len); //reuse len
		printf("load len: %d 0x%x\n", len, len);
	}
	else if (!strncmp(argv[1], "store", 5)) {
		/* nand store <from:ram_addr> <to:nand_offset> <len> */
		unsigned long nand_offset, ram_addr;

		ram_addr = simple_strtoul(argv[2], NULL, 16);
		nand_offset = simple_strtoul(argv[3], NULL, 16);
		len = simple_strtoul(argv[4], NULL, 16);

		if (nand_offset >= chip->size || len <= 0 || (nand_offset + len) >= chip->size ||
		    ram_addr >= CFG_FLASH_BASE || (ram_addr + len) >= CFG_FLASH_BASE) {
			printf("Invalid parameter: nand_offset %lx ram_addr %lx len %d\n", nand_offset, ram_addr, len);
			return 0;
		}

		ret = ranand_write((unsigned char*) ram_addr, nand_offset, len);
		if (ret == len)
			printf("store len: %d 0x%x\n", len, len);
		else
			printf("nand store: write %d bytes to offset 0x%x fail. (ret %d)\n",
				len, nand_offset, ret);
	}
#endif	/* !UBOOT_STAGE1 */
	else if (!strncmp(argv[1], "erase_store", 11)) {
		/* nand erase_store <from:ram_addr> <to:nand_offset> <len> */
		unsigned long nand_offset, ram_addr;

		ram_addr = simple_strtoul(argv[2], NULL, 16);
		nand_offset = simple_strtoul(argv[3], NULL, 16);
		len = simple_strtoul(argv[4], NULL, 16);

		if (nand_offset >= chip->size || len <= 0 || (nand_offset + len) >= chip->size ||
		    ram_addr >= CFG_FLASH_BASE || (ram_addr + len) >= CFG_FLASH_BASE) {
			printf("Invalid parameter: nand_offset %lx ram_addr %lx len %d\n", nand_offset, ram_addr, len);
			return 0;
		}

		ret = ranand_erase_write((unsigned char*) ram_addr, nand_offset, len);
		if (ret >= 0)
			printf("store %d 0x%x bytes (len %d/0x%x)\n", ret, ret, len, len);
		else
			printf("write 0x%x bytes to offset 0x%x fail. (ret %d)\n",
				len, nand_offset, ret);
	}
#if defined(UBI_SUPPORT)
	else if (!strncmp(argv[1], "write_ubi", 9)) {
		/* nand write_ubi <from:ram_addr> [to:nand_offset] <len> */
		unsigned long nand_offset, ram_addr;

		if (argc < 4)
			return 1;

		if (argc == 4) {
			ram_addr = simple_strtoul(argv[2], NULL, 16);
			nand_offset = CFG_UBI_DEV_OFFSET;
			len = simple_strtoul(argv[3], NULL, 16);
		} else {
			ram_addr = simple_strtoul(argv[2], NULL, 16);
			nand_offset = simple_strtoul(argv[3], NULL, 16);
			len = simple_strtoul(argv[4], NULL, 16);
		}

		if (nand_offset >= chip->size || len <= 0 || ram_addr >= CFG_FLASH_BASE || (ram_addr + len) >= CFG_FLASH_BASE) {
			printf("Invalid parameter: nand_offset %lx ram_addr %lx len %d\n", nand_offset, ram_addr, len);
			return 1;
		}

		ret = __SolveUBI((unsigned char*) ram_addr, nand_offset, len);
		if (!ret)
			printf("store len: %d 0x%x\n", len, len);
		else
			printf("Write UBI image failed. (ret %d)\n", ret);
	}
#endif //UBI_SUPPORT
#if defined(DEBUG_ECC_CORRECTION)
	else if (!strncmp(argv[1], "flipbits", 8)) {
		/* nand flipbits <page_number> byte_addr:bit_addr[,bit_addr][,bit_addr...]
		 * [byte_addr:bit_addr[,bit_addr][,bit_addr...]
		 * [byte_addr:bit_addr[,bit_addr][,bit_addr...]
		 * [byte_addr:bit_addr[,bit_addr][,bit_addr...]
		 * Up to 4 bytes can be alerted.
		 */
		const struct mtd_info *mtd = &mt7620a_mtdinfo;
		const struct ra_nand_chip *chip = &mt7620a_nandinfo;
		const int pages_per_block = mtd->erasesize / mtd->writesize;
		struct ra_nand_chip *ra = mtd->priv;
		struct mtd_oob_ops ops;
		int i, mod_cnt = 0, ret;
		struct mod_s {
			unsigned int byte_addr;
			unsigned int bit_mask;
		} mod_ary[4], *mod = &mod_ary[0];
		unsigned int block, page, start_page, byte_addr, bit;
		unsigned char c, *p, blk_buf[mtd->erasesize + pages_per_block * mtd->oobsize];

		if (argc < 4)
			return 1;

		page = simple_strtoul(argv[2], NULL, 16);
		if (page * mtd->writesize >= mtd->size) {
			printf("invalid page 0x%x\n", page);
			return 1;
		}
		start_page = page & ~(pages_per_block - 1);
		block = start_page >> (chip->erase_shift - chip->page_shift);
		printf("page 0x%x start_page 0x%x block 0x%x\n", page, start_page, block);

		/* parsing byte address, bit address */
		for (i = 3; i < argc; ++i) {
			if ((p = strchr(argv[i], ':')) == NULL) {
				printf("colon symbol not found.\n");
				return 1;
			}

			*p = '\0';
			byte_addr = simple_strtoul(argv[i], NULL, 16);
			if (byte_addr >= (2048 + 64)) {
				printf("invalid byte address 0x%x\n", byte_addr);
				return 1;
			}
			mod->byte_addr = byte_addr;
			mod->bit_mask = 0;

			p++;
			while (p && *p != '\0') {
				if (*p < '0' || *p > '9') {
					printf("invalid character. (%c %x)\n", *p, *p);
					return 1;
				}
				bit = simple_strtoul(p, NULL, 16);
				if (bit >= 8) {
					printf("invalid bit address %d\n", bit);
					return 1;
				}
				mod->bit_mask |= (1 << bit);
				p = strchr(p, ',');
				if (p)
					p++;
			}
			mod_cnt++;
			mod++;
		}

		if (!mod_cnt) {
			printf("byte address/bit address pair is not specified.\n");
			return 1;
		}

		/* read a block from block-aligned address with valid OOB information */
		for (i = 0, p = &blk_buf[0]; i < pages_per_block; ++i) {
			memset(&ops, 0, sizeof(ops));
			ops.datbuf = p;
			ops.len = mtd->writesize;
			ops.oobbuf = p + mtd->writesize;
			ops.ooblen = mtd->oobsize;
			ops.mode = MTD_OOB_PLACE;
			if ((ret = nand_do_read_ops(ra, (start_page + i) << chip->page_shift, &ops)) != 0 && ret != -EUCLEAN)
				printf("read page 0x%x fail. (ret %d)\n", start_page + i, ret);

			p += mtd->writesize + mtd->oobsize;
		}

		/* erase block */
		ranand_erase(block << chip->erase_shift, mtd->erasesize);

		/* flip bits */
		for (i = 0, mod = &mod_ary[0], p = &blk_buf[0] + ((page - start_page) << chip->page_shift) + ((page - start_page) * mtd->oobsize);
			i < mod_cnt;
			++i, ++mod)
		{
			c = *(p + mod->byte_addr);
			*(p + mod->byte_addr) ^= mod->bit_mask;
			printf("flip page 0x%x byte 0x%x bitmask 0x%x: orig val %02x -> %02x\n", page, mod->byte_addr, mod->bit_mask, c, *(p + mod->byte_addr));
		}

		/* use raw write to write back page and oob information */
		for (i = 0, p = &blk_buf[0]; i < pages_per_block; ++i) {
			memset(&ops, 0, sizeof(ops));
			ops.datbuf = p;
			ops.len = mtd->writesize;
			ops.oobbuf = p + mtd->writesize;
			ops.ooblen = mtd->oobsize;
			ops.mode =  MTD_OOB_PLACE;
			if ((ret = nand_do_write_ops(ra, (start_page + i) << chip->page_shift, &ops)) != 0)
				printf("write page 0x%x fail. (ret %d)\n", start_page + i, ret);

			p += mtd->writesize + mtd->oobsize;
		}
	}
#endif
#if !defined(UBOOT_STAGE1)
	else if (!strncmp(argv[1], "sbb", 3)) {
		/* nand sbb <nand_offset> */
		unsigned long nand_offset;

		nand_offset = simple_strtoul(argv[2], NULL, 16);
		if (nand_offset >= chip->size) {
			printf("Invalid parameter: nand_offset %lx\n", nand_offset);
			return 0;
		}

		ranand_set_sbb_max_addr(nand_offset);
		if (!nand_offset)
			printf("Disable SBB mechanism!\n");
		else
			printf("Enable SBB mechanism. Maximum offset %lx\n", max_addr);
	}
#endif
#if 0 //defined(UBOOT_STAGE1)
	else if (!strncmp(argv[1], "s2", 2)) {
		/* nand s2 */
		struct stage2_loc s2;

		ranand_locate_stage2(&s2);
	}
#endif
	else
		printf("Usage:\n%s\n use \"help nand\" for detail!\n", cmdtp->usage);
	return 0;
}

U_BOOT_CMD(
	nand,	7,	1, 	ralink_nand_command,
	"nand	- nand command\n",
	"nand usage:\n"
	"  nand id\n"
#if !defined(UBOOT_STAGE1)
	"  nand read <addr> <len>\n"
	"  nand write <addr> <data...>\n"
#endif
	"  nand page <number>\n"
	"  nand erase <addr> <len>\n"
	"  nand init\n"
#if !defined(UBOOT_STAGE1)
	"  nand oob <page_nr>\n"
#endif
	"  nand bad\n"
#if !defined(UBOOT_STAGE1)
	"  nand markbad <blk_nr>\n"
	"  nand erasebad <blk_nr>\n"
	"  nand load <from:nand_offset> <to:ram_addr> <len>\n"
	"  nand store <from:ram_addr> <to:nand_offset> <len>\n"
#endif
	"  nand erase_store <from:ram_addr> <to:nand_offset> <len>\n"
#if defined(UBI_SUPPORT)
	"  nand write_ubi <from:ram_addr> [to:nand_offset] <len>\n"
#endif
#if defined(DEBUG_ECC_CORRECTION)
	"  nand flipbits <page_number> "	\
		"byte_addr:bit_addr[,bit_addr][,bit_addr...] "	\
		"[byte_addr:bit_addr[,bit_addr][,bit_addr...] "	\
		"[byte_addr:bit_addr[,bit_addr][,bit_addr...] "	\
		"[byte_addr:bit_addr[,bit_addr][,bit_addr...]\n"
#endif
#if !defined(UBOOT_STAGE1)
	"  nand sbb <nand_offset>\n"
#endif
	"  PS: All parameter in hexadecimal.\n"
);
#endif

extern ulong NetBootFileXferSize;

#if defined(CFG_ENV_IS_IN_NAND)
#ifdef RALINK_CMDLINE
int do_cp_mem_to_nand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;
	unsigned int addr, dest, erase_len = 0, max_addr = 0;
	int count, ret;

	addr = CFG_LOAD_ADDR;
	count = (unsigned int)NetBootFileXferSize;

#if (!defined(UBI_SUPPORT)) && !defined(UBOOT_STAGE1)
	if (!strncmp(argv[0], "cp.linux", 9)) {
		dest = CFG_KERN_ADDR - CFG_FLASH_BASE;
		max_addr = (1U << chip->chip_shift);
		erase_len = (count + chip->erasesize - 1) & ~(chip->erasesize - 1);
		printf("\n Erase [%d byte] Copy linux image[%d byte] to NAND Flash[0x%08X].... \n", erase_len, count, dest);
	}
#endif

	if (!strncmp(argv[0], "cp.uboot", 9)) {
		dest = 0;
		max_addr = dest + CFG_BOOTLOADER_SIZE;
		erase_len = (count + chip->erasesize - 1) & ~(chip->erasesize - 1);
		printf("\n Erase [%d byte] & Copy uboot[%d byte] to NAND Flash[0x%08X].... \n", erase_len, count, dest);
	}
	else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	ranand_set_sbb_max_addr(max_addr);
	ret = ranand_erase_write((unsigned char *)addr, dest, count);
	ranand_set_sbb_max_addr(0);
	if (ret != count)
		printf("%s: write %d bytes to offset 0x%x fail. (ret %d)\n",
			__func__, count, dest, ret);
	return 0;
}

U_BOOT_CMD(
	cp,	2,	1,	do_cp_mem_to_nand,
	"cp      - memory copy\n",
	"\ncp.uboot\n    - copy uboot block\n"
#if (!defined(UBI_SUPPORT)) && !defined(UBOOT_STAGE1)
	"cp.linux\n    - copy linux kernel block\n"
#endif
);

int do_flerase_nand (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode, size;
	const struct ra_nand_chip *chip = &mt7620a_nandinfo;

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	size = chip->size;
#if (!defined(UBI_SUPPORT)) && (!defined(UBOOT_STAGE1))
	if (strcmp(argv[1], "linux") == 0)
	{
		printf("\n Erase linux kernel block !!\n");
		printf("From 0x%X length 0x%X\n", CFG_KERN_ADDR - CFG_FLASH_BASE,
				size - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		rcode = ranand_erase(CFG_KERN_ADDR - CFG_FLASH_BASE,
				size - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		return rcode;
	}
#endif
	if (strcmp(argv[1], "env") == 0)
	{
		printf("\n Erase environment block !!\n");
		printf("From 0x%X length 0x%X\n", CFG_ENV_ADDR - CFG_FLASH_BASE, CFG_CONFIG_SIZE);
		rcode = ranand_erase(CFG_ENV_ADDR - CFG_FLASH_BASE, CFG_CONFIG_SIZE);
		return rcode;
	}
	else if (strcmp(argv[1], "uboot") == 0)
	{
		printf("\n Erase u-boot block !!\n");
		printf("From 0x%X length 0x%X\n", 0, CFG_BOOTLOADER_SIZE);
		rcode = ranand_erase(0, CFG_BOOTLOADER_SIZE);
		return rcode;
	}
#if defined(UBOOT_STAGE1) || defined(UBOOG_STAGE2)
	else if (strcmp(argv[1], "stage1") == 0)
	{
		printf("\n Erase u-boot Stage1 block !!\n");
		printf("From 0x%X length 0x%X\n", 0, CFG_BOOTSTAGE1_SIZE);
		rcode = ranand_erase(0, CFG_BOOTSTAGE1_SIZE);
		return rcode;
	}
	else if (strcmp(argv[1], "stage2") == 0)
	{
		printf("\n Erase u-boot Stage2 block !!\n");
		printf("From 0x%X length 0x%X\n", CFG_BOOTSTAGE2_OFFSET, CFG_BOOTSTAGE2_SIZEALL);
		rcode = ranand_erase(CFG_BOOTSTAGE2_OFFSET, CFG_BOOTSTAGE2_SIZEALL);
		return rcode;
	}
#endif
	else if (strcmp(argv[1], "all") == 0) {
		rcode = ranand_erase(0, size);
		return rcode;
	}
#if defined(UBI_SUPPORT)
	else if (strcmp(argv[1], "UBI_DEV") == 0) {
		ulong offset = CFG_UBI_DEV_OFFSET;
		ulong len = size - offset;
		char *ubi_detach[] = { "ubi", "detach"};

		rcode = do_ubi(NULL, 0, ARRAY_SIZE(ubi_detach), ubi_detach);
		printf("Erase UBI device area!!\n");
		printf("From 0x%X length 0x%X\n", offset, len);
		rcode = ranand_erase(offset, len);

		if (rcode < 0)
			return rcode;
		return 0;
	}
	else if (strcmp(argv[1], "ubi") == 0) {
		char *ubi_detach[] = { "ubi", "detach"};

		do_ubi(NULL, 0, ARRAY_SIZE(ubi_detach), ubi_detach);
		return init_ubi_volumes(NULL);
	}
#endif

	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	erase,	2,	1,	do_flerase_nand,
	"erase   - erase NAND FLASH memory\n",
	"\nerase all\n    - erase all FLASH banks\n"
	"erase uboot\n    - erase uboot block\n"
	"erase env\n    - erase environment block\n"
#if defined(UBI_SUPPORT)
	"erase UBI_DEV\n    - erase UBI device block. (lost erase counter!)\n"
	"erase ubi\n    - Initialize UBI device and create volumes.\n"
#else
	"erase linux\n    - erase linux kernel block\n"
#endif
);
#endif // RALINK_CMDLINE //
#endif	/* CFG_ENV_IS_IN_NAND */

int nand_bbt_get(struct ra_nand_chip *ra, int block)
{
	return 0;	/* 0 = BBT_TAG_UNKNOWN */
}

int nand_bbt_set(struct ra_nand_chip *ra, int block, int tag)
{
	return tag;
}

#if defined(UBI_SUPPORT)
static int nfc_select_chip(struct ra_nand_chip *ra, int chipnr)
{
#if (CONFIG_NUMCHIPS == 1)
	if (!(chipnr < CONFIG_NUMCHIPS))
		return -1;
	return 0;
#else
	BUG();
#endif
}

static int nand_write_oob_buf(struct ra_nand_chip *ra, uint8_t *buf, uint8_t *oob, size_t size,
			      int mode, int ooboffs)
{
	size_t oobsize = 1<<ra->oob_shift;
	int retsize = 0;

	ra_dbg("%s: size:%x, mode:%x, offs:%x  \n", __func__, size, mode, ooboffs);

	switch(mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_RAW:
		if (ooboffs > oobsize)
			return -1;

		size = min(size, oobsize - ooboffs);
		memcpy(buf + ooboffs, oob, size);
		retsize = size;
		break;

	case MTD_OOB_AUTO:
	{
		struct nand_oobfree *free;
		uint32_t woffs = ooboffs;

		if (ooboffs > ra->oob->oobavail)
			return -1;

		/* OOB AUTO does not clear buffer */

		while (size) {
			for(free = ra->oob->oobfree; free->length && size; free++) {
				int wlen = free->length - woffs;
				int bytes = 0;

				/* Write request not from offset 0 ? */
				if (wlen <= 0) {
					woffs = -wlen;
					continue;
				}

				bytes = min_t(size_t, size, wlen);
				memcpy (buf + free->offset + woffs, oob, bytes);
				woffs = 0;
				oob += bytes;
				size -= bytes;
				retsize += bytes;
			}

			buf += oobsize;
		}

		break;
	}
	default:
		BUG();
	}

	return retsize;
}

static int nand_read_oob_buf(struct ra_nand_chip *ra, uint8_t *oob, size_t size,
			     int mode, int ooboffs)
{
	size_t oobsize = 1<<ra->oob_shift;
	uint8_t *buf = ra->buffers + (1<<ra->page_shift);
	int retsize=0;

	ra_dbg("%s: size:%x, mode:%x, offs:%x  \n", __func__, size, mode, ooboffs);

	switch(mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_RAW:
		if (ooboffs > oobsize)
			return -1;

		size = min(size, oobsize - ooboffs);
		memcpy(oob, buf + ooboffs, size);
		return size;

	case MTD_OOB_AUTO: {
		struct nand_oobfree *free;
		uint32_t woffs = ooboffs;

		if (ooboffs > ra->oob->oobavail)
			return -1;

		size = min(size, ra->oob->oobavail - ooboffs);
		for(free = ra->oob->oobfree; free->length && size; free++) {
			int wlen = free->length - woffs;
			int bytes = 0;

			/* Write request not from offset 0 ? */
			if (wlen <= 0) {
				woffs = -wlen;
				continue;
			}

			bytes = min_t(size_t, size, wlen);
			memcpy (oob, buf + free->offset + woffs, bytes);
			woffs = 0;
			oob += bytes;
			size -= bytes;
			retsize += bytes;
		}
		return retsize;
	}
	default:
		BUG();
	}

	return -1;
}

/** @return -1: chip_select fail
 *	    0 : both CE and WP==0 are OK
 * 	    1 : CE OK and WP==1
 */
static int nfc_enable_chip(struct ra_nand_chip *ra, unsigned int offs, int read_only)
{
	int chipnr = offs >> ra->chip_shift;

	ra_dbg("%s: offs:%x read_only:%x \n", __func__, offs, read_only);

	chipnr = nfc_select_chip(ra, chipnr);
	if (chipnr < 0) {
		printf("%s: chip select error, offs(%x)\n", __func__, offs);
		return -1;
	}

	if (!read_only)
		return nfc_check_wp();

	return 0;
}

/**
 * nand_do_write_ops - [Internal] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operations description structure
 *
 * NAND write with ECC
 */
static int nand_do_write_ops(struct ra_nand_chip *ra, loff_t to,
			     struct mtd_oob_ops *ops)
{
	int page;
	uint32_t datalen = ops->len;
	uint32_t ooblen = ops->ooblen;
	uint8_t *oob = ops->oobbuf;
	uint8_t *data = ops->datbuf;
	int pagesize = (1<<ra->page_shift);
	int pagemask = (pagesize -1);
	int oobsize = 1<<ra->oob_shift;
	loff_t addr = to;
	struct ra_nand_chip *chip = &mt7620a_nandinfo;
	const int prn = (datalen > (chip->writesize + chip->oobsize))? 1:0;
	//int i = 0; //for ra_dbg only

	ra_dbg("%s: to:%x, ops data:%p, oob:%p datalen:%x ooblen:%x, ooboffs:%x oobmode:%x \n",
	       __func__, (unsigned int)to, data, oob, datalen, ooblen, ops->ooboffs, ops->mode);

	ops->retlen = 0;
	ops->oobretlen = 0;
	/* Invalidate the page cache, when we write to the cached page */
	ra->buffers_page = -1;

	if (data == 0)
		datalen = 0;

	// oob sequential (burst) write
	if (datalen == 0 && ooblen) {
		int len = ((ooblen + ops->ooboffs) + (ra->oob->oobavail - 1)) / ra->oob->oobavail * oobsize;

		/* select chip, and check if it is write protected */
		if (nfc_enable_chip(ra, addr, 0))
			return -EIO;

		//FIXME, need sanity check of block boundary
		page = (int)((to & ((1<<ra->chip_shift)-1)) >> ra->page_shift); //chip boundary
		memset(ra->buffers, 0x0ff, pagesize);
		//fixme, should we reserve the original content?
		if (ops->mode == MTD_OOB_AUTO) {
			nfc_read_oob(page, 0, ra->buffers, len);
		}
		//prepare buffers
		nand_write_oob_buf(ra, ra->buffers, oob, ooblen, ops->mode, ops->ooboffs);
		// write out buffer to chip
		nfc_write_oob(page, 0, ra->buffers, len);

		ops->oobretlen = ooblen;
		ooblen = 0;
	}

	// data sequential (burst) write
	if (datalen && ooblen == 0) {
		// ranfc can not support write_data_burst, since hw-ecc and fifo constraints..
	}

	// page write
	while(datalen || ooblen) {
		int len;
		int ret;
		int offs;
		int ecc_en = 0;
		int i = 0;

		ra_dbg("%s (%d): addr:%x, ops data:%p, oob:%p datalen:%x ooblen:%x, ooboffs:%x \n",
		       __func__, i, (unsigned int)addr, data, oob, datalen, ooblen, ops->ooboffs);
		++i;

		page = (int)((addr & ((1<<ra->chip_shift)-1)) >> ra->page_shift); //chip boundary

		/* select chip, and check if it is write protected */
		if (nfc_enable_chip(ra, addr, 0))
			return -EIO;

		// oob write
		if (ops->mode == MTD_OOB_AUTO) {
			//fixme, this path is not yet varified
			nfc_read_oob(page, 0, ra->buffers + pagesize, oobsize);
		}
		if (oob && ooblen > 0) {
			len = nand_write_oob_buf(ra, ra->buffers + pagesize, oob, ooblen, ops->mode, ops->ooboffs);
			if (len < 0)
				return -EINVAL;

			oob += len;
			ops->oobretlen += len;
			ooblen -= len;
		}

		// data write
		offs = addr & pagemask;
		len = min_t(size_t, datalen, pagesize - offs);
		if (data && len > 0) {
			memcpy(ra->buffers + offs, data, len);	// we can not sure ops->buf wether is DMA-able.

			data += len;
			datalen -= len;
			ops->retlen += len;

			ecc_en = FLAG_ECC_EN;
		}

		ret = nfc_write_page(ra->buffers, page, ((ops->mode == MTD_OOB_RAW || ops->mode == MTD_OOB_PLACE) ? 0:ecc_en));
		if (ret < 0)
			return ret;

		if (prn && !(page & 0x1F))
			printf(".");
		nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_GOOD);

		addr = (page+1) << ra->page_shift;
	}

	return 0;
}

/**
 * nand_do_read_ops - [Internal] Read data with ECC
 *
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob ops structure
 * @return:
 *  -EUCLEAN:	corrected, ok
 *
 * Internal function. Called with chip held.
 */
static int nand_do_read_ops(struct ra_nand_chip *ra, loff_t from,
			    struct mtd_oob_ops *ops)
{
	int page;
	uint32_t datalen = ops->len;
	uint32_t ooblen = ops->ooblen;
	uint8_t *oob = ops->oobbuf;
	uint8_t *data = ops->datbuf;
	int pagesize = (1<<ra->page_shift);
	int pagemask = (pagesize -1);
	loff_t addr = from;
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	struct mtd_ecc_stats stats = mtd->ecc_stats;
	struct ra_nand_chip *chip = &mt7620a_nandinfo;
	const int prn = (datalen > (chip->writesize + chip->oobsize))? 1:0;
	//int i = 0; //for ra_dbg only

	ra_dbg("%s: addr:%x, ops data:%p, oob:%p datalen:%x ooblen:%x, ooboffs:%x \n",
	       __func__, (unsigned int)addr, data, oob, datalen, ooblen, ops->ooboffs);

	ops->retlen = 0;
	ops->oobretlen = 0;
	if (data == 0)
		datalen = 0;

	while(datalen || ooblen) {
		int len;
		int ret;
		int offs;
		int i = 0;

		ra_dbg("%s (%d): addr:%x, ops data:%p, oob:%p datalen:%x ooblen:%x, ooboffs:%x \n",
		       __func__, i, (unsigned int)addr, data, oob, datalen, ooblen, ops->ooboffs);
		++i;

		/* select chip */
		if (nfc_enable_chip(ra, addr, 1) < 0)
			return -EIO;

		page = (int)((addr & ((1<<ra->chip_shift)-1)) >> ra->page_shift);

		ret = nfc_read_page(ra->buffers, page);
		//FIXME, something strange here, some page needs 2 more tries to guarantee read success.
		if (ret < 0 && ret != -EUCLEAN) {
			printf("%s: read page fail (ret %d), again\n", __func__, ret);
			ret = nfc_read_page(ra->buffers, page);

			if (ret >= 0 || ret == -EUCLEAN)
				printf("%s: read again susccess. (ret %d)\n", __func__, ret);
		}

		// oob read
		if (oob && ooblen > 0) {
			len = nand_read_oob_buf(ra, oob, ooblen, ops->mode, ops->ooboffs);
			if (len < 0) {
				printf("nand_read_oob_buf: fail return %x \n", len);
				return -EINVAL;
			}

			oob += len;
			ops->oobretlen += len;
			ooblen -= len;
		}

		// data read
		offs = addr & pagemask;
		len = min_t(size_t, datalen, pagesize - offs);
		if (data && len > 0) {
			memcpy(data, ra->buffers + offs, len);	// we can not sure ops->buf wether is DMA-able.

			data += len;
			datalen -= len;
			ops->retlen += len;
			if (ret && ret != -EUCLEAN)
				return ret;
		}

		if (prn && !(page & 0x1F))
			debug("R");
		nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_GOOD);
		// address go further to next page, instead of increasing of length of write. This avoids some special cases wrong.
		addr = (page+1) << ra->page_shift;
	}

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;

	return  mtd->ecc_stats.corrected - stats.corrected ? -EUCLEAN : 0;
}

/**
 * nand_block_checkbad - [GENERIC] Check if a block is marked bad
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 *
 * Check, if the block is bad. Either by reading the bad block table or
 * calling of the scan function.
 */
int nand_block_checkbad(struct ra_nand_chip *ra, loff_t offs)
{
	int page, block;
	int ret = 4;
	unsigned int tag;
	char *str[]= {"UNK", "RES", "BAD", "GOOD"};

	if (ranfc_bbt == 0)
		return 0;

	// align with chip

	offs = offs & ((1<<ra->chip_shift) -1);

	page = offs >> ra->page_shift;
	block = offs >> ra->erase_shift;

	tag = nand_bbt_get(ra, block);

	if (tag == BBT_TAG_UNKNOWN) {
		ret = nfc_read_oob(page, ra->badblockpos, (char*)&tag, 1);
		if (ret == 0)
			tag = ((le32_to_cpu(tag) & 0x0ff) == 0x0ff) ? BBT_TAG_GOOD : BBT_TAG_BAD;
		else
			tag = BBT_TAG_BAD;

		if (tag == BBT_TAG_GOOD) {
			ret = nfc_read_oob(page + 1, ra->badblockpos, (char*)&tag, 1);
			if (ret == 0)
				tag = ((le32_to_cpu(tag) & 0x0ff) == 0x0ff) ? BBT_TAG_GOOD : BBT_TAG_BAD;
			else
				tag = BBT_TAG_BAD;
		}

		nand_bbt_set(ra, block, tag);
	}

	if (tag != BBT_TAG_GOOD) {
		printf("%s: block: 0x%x tag: %s (offs: %x)\n", __func__, block, str[tag], (unsigned int)offs);
		return 1;
	} else
		return 0;
}
#endif	/* UBI_SUPPORT */

/**
 * mark a block own offs
 * @offs:	any address of a block that would be mark as bad block.
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *  otherwise:	error
 */
int nand_block_markbad(struct ra_nand_chip *chip, unsigned int offs)
{
	int status;
	unsigned int page, block, blk_offs;
	unsigned char buf[chip->oobsize];

	// align with chip
	ra_dbg("%s offs: %x \n", __func__, offs);
	offs = offs & ((1 << chip->chip_shift) - 1);
	block = offs >> chip->erase_shift;
	page = block << chip->numpage_per_block_bit;
	blk_offs = block << chip->block_size_bit;
	if (nand_bbt_get(chip, block) == BBT_TAG_BAD) {
		printf("%s: mark block 0x%x repeatedly\n", __func__, block);
		return 0;
	}

	ranand_erase(blk_offs, chip->erasesize);
	memset(buf, 0, sizeof(buf));
	buf[chip->badblockpos] = 0x33;
	status = nfc_write_oob(page, 0, buf, chip->oobsize);
	status |= nfc_write_oob(page + 1, 0, buf, chip->oobsize);
	//update bbt
	nand_bbt_set(chip, block, BBT_TAG_BAD);

	return (!status)? 0:-1;
}

#if defined(UBI_SUPPORT)
/**
 * mt7620a_mtd_erase	[MTD Interface] erase block(s)
 * @mtd:		MTD device structure
 * @instr:		erase instruction
 *
 * Erase one ore more blocks
 */
int mt7620a_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int page, ret = 0;
	loff_t len;
	struct ra_nand_chip *chip = &mt7620a_nandinfo;

	/* Start address must align on block boundary */
	if (BLOCK_ALIGNED(instr->addr)) {
		printf("%s: Unaligned address 0x%lx\n", __func__, (ulong) instr->addr);
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (BLOCK_ALIGNED(instr->len)) {
		printf("%s: Length 0x%lx not block aligned\n", __func__, (ulong) instr->len);
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->addr + instr->len) > mtd->size) {
		printf("%s: Erase past end of device. 0x%lx\n",
			__func__, (ulong) (instr->addr + instr->len));
		return -EINVAL;
	}

	/* Check, if it is write protected */
	if (0) {
		instr->state = MTD_ERASE_FAILED;
		return -EIO;
	}

	page = instr->addr >> chip->page_shift;
	len = instr->len;
	instr->state = MTD_ERASING;

	while (len) {
		if (!instr->scrub && ranand_block_isbad(page << chip->page_shift)) {
			printf("%s: attempt to erase a bad block at page 0x%08x\n", page);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}

		if (ranand_erase(page << chip->page_shift, chip->erasesize)) {
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = page << chip->page_shift;
			goto erase_exit;
		}

		len -= chip->erasesize;
		page += 1 << chip->numpage_per_block_bit;
	}
	instr->state = MTD_ERASE_DONE;

erase_exit:

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;
	return ret;
}

/**
 * mt7620a_mtd_read	[MTD Interface] MTD compability function for nand_do_read_ecc
 * @mtd:		MTD device structure
 * @from:		offset to read from
 * @len:		number of bytes to read
 * @retlen:		pointer to variable to store the number of read bytes
 * @buf:		the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
int mt7620a_mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;
	struct mtd_oob_ops ops;

	ra_dbg("%s: mtd:%p from:%x, len:%x, buf:%p \n", __func__, mtd, (unsigned int)from, len, buf);
	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	memset(&ops, 0, sizeof(ops));
	ops.len = len;
	ops.datbuf = buf;
	ops.oobbuf = NULL;
	ops.mode = MTD_OOB_AUTO;

	ret = nand_do_read_ops(ra, from, &ops);

	*retlen = ops.retlen;

	return ret;
}

/**
 * mt7620a_mtd_write	[MTD Interface] NAND write with ECC
 * @mtd:		MTD device structure
 * @to:			offset to write to
 * @len:		number of bytes to write
 * @retlen:		pointer to variable to store the number of written bytes
 * @buf:		the data to write
 *
 * NAND write with ECC
 */
int mt7620a_mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;
	struct mtd_oob_ops ops;

	ra_dbg("%s: \n", __func__);
	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	memset(&ops, 0, sizeof(ops));
	ops.len = len;
	ops.datbuf = (uint8_t *)buf;
	ops.oobbuf = NULL;
	ops.mode =  MTD_OOB_AUTO;

	ret = nand_do_write_ops(ra, to, &ops);

	*retlen = ops.retlen;

	return ret;
}

/**
 * mt7620a_mtd_read_oob	[MTD Interface] NAND read data and/or out-of-band
 * @mtd:		MTD device structure
 * @from:		offset to read from
 * @ops:		oob operation description structure
 *
 * NAND read data and/or out-of-band data
 */
int mt7620a_mtd_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;

	ra_dbg("%s: \n", __func__);
	ret = nand_do_read_ops(ra, from, ops);

	return ret;
}

/**
 * mt7620a_mtd_write_oob	[MTD Interface] NAND write data and/or out-of-band
 * @mtd:			MTD device structure
 * @to:				offset to write to
 * @ops:			oob operation description structure
 */
int mt7620a_mtd_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;

	ra_dbg("%s: \n", __func__);
	ret = nand_do_write_ops(ra, to, ops);

	return ret;
}

/**
 * nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start
 */
int mt7620a_mtd_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	if (offs > mtd->size)
		return -EINVAL;

	return nand_block_checkbad((struct ra_nand_chip *)mtd->priv, offs);
}

/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
int mt7620a_mtd_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	int ret;
	struct ra_nand_chip *ra = mtd->priv;

	ra_dbg("%s: \n", __func__);
	ret = nand_block_markbad(ra, (unsigned int)ofs);

	return ret;
}
#endif	/* UBI_SUPPORT */

#if defined(UBOOT_STAGE1) || defined(UBOOT_STAGE2)
/**
 * Duplicate image at load_address as many as possible
 * @ptr:	pointer to image source address
 * @len:	image length in bytes
 * @max_len:	maximum length in bytes can be used at ptr.
 * @return:
 *    >=0:	success, number of new copy
 *     -1:	invalid parameter.
 *  otherwise:	error
 */
static int duplicate_image(unsigned char *ptr, unsigned int len, unsigned int max_len)
{
	int c = 0;
	unsigned char *next_ptr, *max_ptr;
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	const unsigned int unit_len = ROUNDUP(len, mtd->erasesize);

	if (!ptr || ptr >= (unsigned char*) CFG_FLASH_BASE || !len || len >= max_len ||
	    !max_len || (ptr + len) > (unsigned char*) CFG_FLASH_BASE ||
	    (ptr + max_len) > (unsigned char*) CFG_FLASH_BASE)
	{
		printf("%s: invalid parameter. (ptr %x len %x max_len %x)\n",
			__func__, ptr, len, max_len);
		return -1;
	}

	next_ptr = ptr + unit_len;
	max_ptr = ptr + max_len;
	for (; (next_ptr + unit_len) <= max_ptr; next_ptr+=unit_len, c++) {
		memcpy(next_ptr, ptr, len);
		memset(next_ptr + len, 0, unit_len - len);
	}

	debug("duplicate %d new copy of %x\n", c, ptr);
	return c;
}

/**
 * Duplicate image at addr as many as possible
 * @addr:	start address of image source, flash address can be used.
 * @offset:	offset of flash to write image
 * @len:	image length in bytes
 * @max_len:	maximum length
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	read image from flash fail
 *     -3:	write fail
 *    -12:	-ENOMEM, allocate memory fail.
 *  otherwise:	error
 */
int ranand_dup_erase_write_image(unsigned int addr, unsigned int offset, unsigned int len, unsigned int max_len)
{
	int ret = 0, c = 0, w = 0;
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	const unsigned int unit_len = ROUNDUP(len, mtd->erasesize);
	unsigned char *src, *ptr;

	if (!addr || !offset  || offset >= CFG_FLASH_BASE || !len || !max_len || len > max_len) {
		printf("%s: invalid parameter. (addr %x, offset %x, len %x, max_len %x)\n",
			__func__, addr, offset, len, max_len);
		return -1;
	}

	src = (unsigned char*) addr;
	if (src >= (unsigned char*) CFG_FLASH_BASE) {
		if ((src = malloc(len)) == NULL) {
			printf("%s: allocate 0x%x bytes memory fail!\n",
				__func__, len);
			ret = -12;
			goto exit_dup_write_0;
		}
		ret = ranand_read(src, addr, len);
		if (ret != len) {
			printf("%s: read 0x%x bytes image from flash fail! (ret %d)\n",
				__func__, len, ret);
			ret = -2;
			goto exit_dup_write_0;
		}
	}
	if ((ptr = malloc(max_len)) == NULL) {
		printf("%s: allocate 0x%x bytes memory fail! Use %x instead\n",
			__func__, max_len, CFG_LOAD_ADDR_2);
		ptr = (unsigned char*) CFG_LOAD_ADDR_2;
	}

	memcpy(ptr, src, len);
	c = duplicate_image(ptr, len, max_len);
	if (c >= 0)
		c++;
	ranand_set_sbb_max_addr(offset + max_len);
	w = ranand_erase_write(ptr, offset, max_len);
	ranand_set_sbb_max_addr(0);

	if (ptr != (unsigned char*) CFG_LOAD_ADDR_2)
		free(ptr);

	if (src != (unsigned char*) addr)
		free(src);

exit_dup_write_0:

	if (w < 0)
		return -3;

	printf("write %d copies to flash. (total %d copies)\n", w / unit_len, c);

	return ret;
}

/**
 * Duplicate stage2 image at addr and write to stage2 area as many as possible
 * @addr:	start address of image source, flash address can be used.
 * @len:	image length in bytes
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *  otherwise:	error
 */
int ranand_write_stage2(unsigned int addr, unsigned int len)
{
	return ranand_dup_erase_write_image(addr, CFG_BOOTSTAGE2_OFFSET, len, CFG_BOOTSTAGE2_SIZEALL);
}
#endif	/* UBOOT_STAGE1 || UBOOT_STAGE2 */

#if defined(UBOOT_STAGE1)
/**
 * Do CRC32 check on a U-Boot image
 * @hdr:	pointer to RAM address of header of a U-Boot image
 * @type:	0: check magic number and header checksum only
 * 		1: check all items
 * @return:
 * 	0:	successful
 *     -1:	invalid param
 *     -2:	invalid U-Boot magic number
 *     -3:	header checksum mismatch
 *     -4:	data checksum mismatch
 */
static int check_uboot_image(image_header_t *hdr, int type)
{
	size_t len;
	uint32_t hcrc;
	unsigned char *d;
	image_header_t header;

	if (!hdr || hdr >= (image_header_t*) CFG_FLASH_BASE)
		return -1;

	if (ntohl(hdr->ih_magic) != IH_MAGIC)
		return -2;

	memcpy(&header, hdr, sizeof(image_header_t));
	header.ih_hcrc = 0;
	hcrc = ntohl(hdr->ih_hcrc);
	d = (unsigned char*) &header;
	len = sizeof(image_header_t);
	if (crc32(0, (char*) d, len) != hcrc)
		return -3;

	if (!type)
		return 0;

	d = (unsigned char*) hdr + sizeof(image_header_t);
	len = ntohl(hdr->ih_size);
	if (crc32(0, (char*) d, len) != ntohl(hdr->ih_dcrc))
		return -4;

	return 0;
}

/**
 * Fill a stage2 descriptor.
 * @desc:
 * @id:
 * @offset:
 * @boundary:
 * @code:
 * @failed:
 * @corrected:
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 */
static int __fill_stage2_desc(struct stage2_desc *desc, unsigned int id, unsigned int offset, unsigned int boundary, unsigned char *code, uint32_t failed, uint32_t corrected)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	image_header_t *hdr = (image_header_t*) code;

	if (!desc || offset >= CFG_BOOTLOADER_SIZE || boundary > CFG_BOOTLOADER_SIZE || !code || code >= (unsigned char*) CFG_FLASH_BASE) {
		debug("%s: invalid parameter (desc %p, id %x, offset %x, boundary %x, code %p, failed %x, corrected %x)\n",
			__func__, desc, id, offset, boundary, code, failed, corrected);
		return -1;
	}

	sprintf(desc->name, "stage2-%d", id);
	desc->offset = offset;
	desc->boundary = boundary;
	desc->code = code;
	desc->failed = failed;
	desc->corrected = corrected;

	desc->crc_error = 1;
	desc->len = CFG_MAX_BOOTLOADER_BINARY_SIZE;
	if (!check_uboot_image(hdr, 1)) {
		desc->len = sizeof(image_header_t) + ntohl(hdr->ih_size);
		desc->crc_error = 0;
	}
	desc->blk_len = (desc->len + (mtd->erasesize - 1)) & ~(mtd->erasesize - 1);

	return 0;
}

/**
 * Check stage1 and fix it if necessary.
 * @return:
 * 	0:	success
 *     -1:	first block is bad-block.
 *     -2:	read header from block0 fail
 *     -3:	incorrect magic number or header checksum
 *     -4:	image length larger than 1 block
 *     -5:	CRC32 checksum mismatch
 *  otherwise:	error
 */
int ranand_check_and_fix_stage1(void)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	int ret;
	struct mtd_ecc_stats stats;
	uint32_t failed, corrected;
	image_header_t header, *hdr = &header;
	unsigned char buf[CFG_BOOTSTAGE2_SIZEALL]  __attribute__((aligned(4)));

	if (ranand_block_isbad(0)) {
		debug("block0 is bad-block, skip!!!\n");
		return -1;
	}

	memset(hdr, 0, sizeof(image_header_t));
	ret = ranand_read((unsigned char*)hdr, 0, sizeof(image_header_t));
	if (ret != sizeof(image_header_t)) {
		debug("read header from block0 fail! (ret %d)\n", ret);
		return -2;
	}

	if (check_uboot_image(hdr, 0)) {
		debug("invalid magic number or header checksum!\n");
		return -3;
	}

	if ((sizeof(image_header_t) + ntohl(hdr->ih_size)) > CFG_BOOTSTAGE1_SIZE) {
		debug("stage1 length %x too large!\n", sizeof(image_header_t) + ntohl(hdr->ih_size));
		return -4;
	}

	stats = mtd->ecc_stats;
	ret = ranand_read(buf, 0, mtd->erasesize);
	failed = mtd->ecc_stats.failed - stats.failed;
	corrected = mtd->ecc_stats.corrected - stats.corrected;

	if (check_uboot_image((image_header_t*) buf, 1)) {
		debug("stage1 CRC32 checksum mismatch! (image data length: %x)\n", ntohl(hdr->ih_size));
		return -4;
	}

	if (!failed && !corrected)
		return 0;

	/* reprogram stage1 code */
	printf("failed %d corrected %d ==> reprogram stage1\n", failed, corrected);
	ret = ranand_erase_write(buf, 0, mtd->erasesize);
	if (ret != mtd->erasesize) {
		printf("Reprogram stage1 fail! (ret %d)\n", ret);
		return -5;
	}

	return 0;
}


/**
 * Try to assemble a good stage2 from fragments.
 * @s2:		pointer to a struct stage2_loc
 * @type:	0: fast method.
 *		   assemble stage2 code depends on partition defined in s2 parameter.
 * 		otherwise: aggresive method.
 * 		   ogmpre partition defined in s2 parameter.
 * @return:
 * 	0:	success, s2->good points to a descriptor of good stage2 code.
 * 	1:	fail.
 *     -1:	invalid parameter
 *  otherwise:	not defined
 */
static int assemble_stage2(struct stage2_loc *s2, int type)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	static unsigned char s2_buf[CFG_BOOTSTAGE2_SIZEALL]  __attribute__((aligned(4)));
	struct stage2_desc *desc;
	unsigned char *p, *code;
	int lvar[(CFG_BOOTSTAGE2_SIZEALL + mtd->erasesize - 1) / mtd->erasesize];
	int i, v, val, base, *q, tmp, skip, bound, nr_blks = 2;
	int nr_copy = s2->count;
	unsigned int o;

	if (!s2)
		return -1;

	/* take first reasonable image length of stage2 */
	for (i = 0; i < s2->count; ++i) {
		tmp = s2->desc[i].blk_len / mtd->erasesize;
		if (tmp <= 0 || tmp >= (sizeof(lvar)/sizeof(lvar[0])))
			continue;

		nr_blks = tmp;
		break;
	}

	/* use all read blocks to assemble stage2 */
	if (type)
		nr_copy = s2->nr_blk_read;

	bound = 1;
	for (i = 0; i < nr_blks; ++i)
		bound *= nr_copy;
	printf("assemble stage2: %d,%d/%d/%d ...\n", type, nr_blks, nr_copy, bound);
	desc = &s2->desc[MAX_NR_STAGE2 - 1];
	for (val = 0, base = nr_copy; !s2->good && val < bound; val++) {
		/* calculate loop-variables */
		memset(lvar, 0, sizeof(lvar));
		for (v = val, q = &lvar[nr_blks - 1]; v > 0; v /= base)
			*q-- = v % base;

		/* if all loop-variables are equal to each other, skip. */
		for (v = lvar[0], i = 1, q = &lvar[i], skip = 1; skip && i < nr_blks; ++i, ++q) {
			if (v == *q)
				continue;

			skip = 0;
		}
		if (skip)
			continue;

		skip = 0;
		for (i = 0, o = 0, p = s2_buf, q = &lvar[i];
		     !skip && i < nr_blks;
		     ++i, o += mtd->erasesize, p += mtd->erasesize, ++q)
		{
			if (!type)
				code = s2->desc[*q].code + o;
			else
				code = s2->code + (*q) * mtd->erasesize;

			if (!i && check_uboot_image((image_header_t*) code, 0)) {
				skip = 1;
				continue;
			}
			if (type && i && !check_uboot_image((image_header_t*) code, 0)) {
				skip = 1;
				continue;
			}

			memcpy(p, code, mtd->erasesize);
		}
		if (skip)
			continue;

		/* create dummy stage2 code descriptor and check image */
		__fill_stage2_desc(desc, MAX_NR_STAGE2 - 1, CFG_BOOTSTAGE2_OFFSET, CFG_BOOTLOADER_SIZE, s2_buf, 0, 0);
		if (desc->crc_error)
			continue;

		debug("Assemble good stage2 code from multiple fragments successful.\n");
		s2->good = desc;
	}

	return (s2->good)? 0:1;
}

/**
 * Find all stage2 code, include bad one, and record to a table.
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	read error
 *    -12:	-ENOMEM, allocate memory fail.
 *  otherwise:	error
 */
int ranand_locate_stage2(struct stage2_loc *s2)
{
	const struct mtd_info *mtd = &mt7620a_mtdinfo;
	int i, ret;
	struct stage2_desc *desc;
	struct mtd_ecc_stats stats;
	uint32_t failed, corrected, magic;
	unsigned int o, offset, s2_len, space;
	unsigned char *code, *buf;
	image_header_t header, *hdr = &header;

	if (!s2)
		return -1;

	memset(s2, 0, sizeof(struct stage2_loc));
	for (offset = CFG_BOOTSTAGE2_OFFSET,
		o = space = 0, failed = corrected = 0,
		desc = &s2->desc[0], s2_len = CFG_MAX_BOOTLOADER_BINARY_SIZE,
		buf = code = s2->code;
	     offset < CFG_BOOTLOADER_SIZE;
	     offset += mtd->erasesize)
	{
		if (space >= s2_len) {
			if (o && o != offset) {
				__fill_stage2_desc(desc, s2->count, o, offset, code, failed, corrected);
				desc++;
				s2->count++;
			}

			code = buf;
			o = space = 0;
			failed = corrected = 0;
			s2_len = CFG_MAX_BOOTLOADER_BINARY_SIZE;
		}
		if (ranand_block_isbad(offset))
			continue;

		if (!o)
			o = offset;
		memset(hdr, 0, sizeof(image_header_t));
		ret = ranand_read((unsigned char*)hdr, offset, sizeof(image_header_t));
		if ((magic = ntohl(hdr->ih_magic)) == IH_MAGIC || space >= s2_len) {
			if (o && o != offset) {
				__fill_stage2_desc(desc, s2->count, o, offset, code, failed, corrected);
				desc++;
				s2->count++;
			}

			code = buf;
			o = space = 0;
			failed = corrected = 0;
			s2_len = CFG_MAX_BOOTLOADER_BINARY_SIZE;
			if (magic == IH_MAGIC) {
				o = offset;
				s2_len = sizeof(image_header_t) + ntohl(hdr->ih_size);
			}
		}
		space += mtd->erasesize;
		if (ret != sizeof(image_header_t))
			continue;

		stats = mtd->ecc_stats;
		ret = ranand_read(buf, offset, mtd->erasesize);
		buf += mtd->erasesize;
		failed += mtd->ecc_stats.failed - stats.failed;
		corrected += mtd->ecc_stats.corrected - stats.corrected;
	}
	s2->nr_blk_read = ((unsigned int)(buf - s2->code)) / mtd->erasesize;
	if (o && o != offset) {
		__fill_stage2_desc(desc, s2->count, o, offset, code, failed, corrected);
		s2->count++;
	}

	/* looking for and choose a good stage2 code */
	s2->good = NULL;
	for (i = 0, desc = &s2->desc[0]; i < s2->count; ++i, ++desc) {
		char *s = "unknown status";

		if (desc->crc_error)
			s = "CRC32 error";
		else if (desc->failed)
			s = "Uncorrectable ECC error";
		else if (desc->corrected)
			s = "Correctable ECC error";
		else if (!desc->crc_error)
			s = "OK";
		printf("%s: 0x%x-%x, len %x/%x, buf %p: %s\n",
			desc->name, desc->offset, desc->boundary,
			desc->len, desc->blk_len, desc->code, s);
		if (!s2->good && !desc->crc_error && desc->code) {
			s2->good = desc;
		}
	}

	/* Try to assembly a good stage2 from several fragments. */
	if (!s2->good && s2->count > 1)
		assemble_stage2(s2, 0);

	if (!s2->good && s2->nr_blk_read > 1)
		assemble_stage2(s2, 1);

	if (s2->good)
		printf("choose %s at %p, length %x bytes\n",
			s2->good->name, s2->good->code, s2->good->len);

	return 0;
}
#endif
