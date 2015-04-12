#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/mtd/mtd.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <linux/sched.h>
#if defined (RANDOM_GEN_BAD_BLOCK)
#include <linux/random.h>
#endif

#include <ralink/ralink_gpio.h>

#include "ralink-flash.h"
#include "ralink_nand.h"
#if defined (CONFIG_MTD_NAND_USE_UBI_PART)
#include "ralink-nand-map-ubi.h"
#else
#include "ralink-nand-map.h"
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
#define MTD_OPS_PLACE_OOB	MTD_OOB_PLACE
#define MTD_OPS_AUTO_OOB	MTD_OOB_AUTO
#define MTD_OPS_RAW		MTD_OOB_RAW
#endif

#if defined (CONFIG_MTD_UBI) || defined (CONFIG_MTD_UBI_MODULE)
#define UBIFS_ECC_0_PATCH
#if defined (CONFIG_MTD_NAND_USE_UBI_PART)
#define UBI_PART_START_OFFSET	NAND_MTD_UBI_PART_OFFSET
#else
#define UBI_PART_START_OFFSET	NAND_MTD_RWFS_PART_OFFSET
#endif
#endif

#define SKIP_BAD_BLOCK
#define NOT_SUPPORT_WP		// no WP signal for chip.
#if !defined (CONFIG_RALINK_MT7620)
#define NOT_SUPPORT_RB
#define USE_NAND_GDMA
#include "gdma.h"
#endif

#define BLOCK_ALIGNED(a)	((a) & (CFG_BLOCKSIZE - 1))
#define READ_STATUS_RETRY	5000
#define CONFIG_NUMCHIPS		1

static struct mtd_info *ranfc_mtd = NULL;

static int ranfc_debug = 0;
static int ranfc_bbt = 1;
static int ranfc_verify = 1;

module_param(ranfc_debug, int, S_IRUGO | S_IWUSR);
module_param(ranfc_bbt, int, S_IRUGO | S_IWUSR);
module_param(ranfc_verify, int, S_IRUGO | S_IWUSR);

#if 0
#define ra_dbg(args...) do { if (ranfc_debug) printk(args); } while(0)
#else
#define ra_dbg(args...)
#endif

#define CLEAR_INT_STATUS()	ra_outl(NFC_INT_ST, ra_inl(NFC_INT_ST))
#define NFC_TRANS_DONE()	(ra_inl(NFC_INT_ST) & INT_ST_ND_DONE)

int nand_addrlen = 4;
int is_nand_page_2048 = 0;
const unsigned int nand_size_map[2][3] = {{25, 30, 30}, {20, 27, 30}};

#ifdef SKIP_BAD_BLOCK
static int page_remap(struct ra_nand_chip *ra, int page);
static int write_next_on_fail(struct ra_nand_chip *ra, char *write_buf, int page, int flags, int * to_blk);
static int is_skip_bad_block(struct ra_nand_chip *ra, int page);
#endif

static int nfc_wait_ready(int snooze_ms);
int nfc_read_page(struct ra_nand_chip *ra, char *buf, int page, int flags);
int nfc_write_page(struct ra_nand_chip *ra, char *buf, int page, int flags);

/*************************************************************
 * nfc functions
 *************************************************************/

static void nfc_init(void)
{
#if defined (CONFIG_RALINK_MT7620)
	u32 reg;
	u32 chip_mode = 12;

	/* use ND Mode */
	ra_and(RALINK_REG_GPIOMODE, ~(0x3<<18));

	reg = ra_inl(RALINK_SYSCTL_BASE+0x10);
	chip_mode = (reg & 0x0F);

	if (chip_mode == 1 || chip_mode==11) {
		ra_or(NFC_CONF1, 1);
		is_nand_page_2048 = 1;
		nand_addrlen = ((chip_mode!=11) ? 4 : 5);
		printk("!!! nand page size=%d, addr len=%d\n", 2048, nand_addrlen);
	} else {
		ra_and(NFC_CONF1, ~1);
		is_nand_page_2048 = 0;
		nand_addrlen = ((chip_mode!=10) ? 3 : 4);
		printk("!!! nand page size=%d, addr len=%d\n", 512, nand_addrlen);
	}

	//config ECC location
	ra_and(NFC_CONF1, 0xfff000ff);
	ra_or(NFC_CONF1, ((CONFIG_ECC_OFFSET + 2) << 16) + ((CONFIG_ECC_OFFSET + 1) << 12) + (CONFIG_ECC_OFFSET << 8));
#else
	is_nand_page_2048 = 0;
	nand_addrlen = 3;
#endif

	//make sure gpio-0 is input
	ra_outl(RALINK_PIO_BASE+0x24, ra_inl(RALINK_PIO_BASE+0x24) & ~0x01);
}

/**
 * reset nand chip
 */
static int nfc_chip_reset(void)
{
	int status;

	// reset nand flash
	ra_outl(NFC_CMD1, 0x0);
	ra_outl(NFC_CMD2, 0xff);
	ra_outl(NFC_ADDR, 0x0);
	ra_outl(NFC_CONF, 0x0411);

	status = nfc_wait_ready(5);  //erase wait 5us
	if (status & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
	}

	return (int)(status & NAND_STATUS_FAIL);
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
		printk("%s: clean buffer fail\n", __func__);
		return -1;
	}

	retry = READ_STATUS_RETRY;
	while ((ra_inl(NFC_STATUS) & 0x1) != 0x0 && retry--) { //fixme, controller is busy ?
		udelay(1);
	}

	nfc_chip_reset();

	return 0;
}

/** NOTICE: only called by nfc_wait_ready().
 * @return -1, nfc can not get transction done 
 * @return 0, ok.
 */
static int _nfc_read_status(char *status)
{
	unsigned long cmd1, conf;
	int int_st, nfc_st;
	int retry;

	cmd1 = 0x70;
	conf = 0x000101 | (1 << 20);

	//fixme, should we check nfc status?
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
		
		ndelay(100);
	} while (!(int_st & INT_ST_RX_BUF_RDY) && retry--);

	if (!(int_st & INT_ST_RX_BUF_RDY)) {
		printk("%s: NFC fail, int_st(%x), retry:%x, nfc:%x, reset nfc and flash\n",
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
#if !defined (NOT_SUPPORT_WP)
	return !!(ra_inl(NFC_CTRL) & 0x01);
#else
	char result = 0;
	int ret;

	ret = _nfc_read_status(&result);
	//FIXME, if ret < 0

	return !(result & NAND_STATUS_WP);
#endif
}

#if !defined (NOT_SUPPORT_RB)
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


/**
 * generic function to get data from flash.
 * @return data length reading from flash.
 */
static int _ra_nand_pull_data(char *buf, int len, int use_gdma)
{
	__u32 *p = (__u32 *)buf;
	int retry, int_st;
	unsigned int ret_data;
	int ret_size;

#if defined (USE_NAND_GDMA)
	// receive data by use_gdma
	if (use_gdma) {
		if (_ra_nand_dma_pull((unsigned long)p, len)) {
			printk("%s: fail \n", __func__);
			len = -1; //return error
		}
		return len;
	}
#endif

	//fixme: retry count size?
	retry = READ_STATUS_RETRY;
	// no gdma
	while (len > 0) {
		int_st = ra_inl(NFC_INT_ST);
		if (int_st & INT_ST_RX_BUF_RDY) {
			ret_data = ra_inl(NFC_DATA);
			ra_outl(NFC_INT_ST, INT_ST_RX_BUF_RDY); 
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

	return ((int)p - (int)buf);
}

/**
 * generic function to put data into flash.
 * @return data length writing into flash.
 */
static int _ra_nand_push_data(char *buf, int len, int use_gdma)
{
	__u32 *p = (__u32 *)buf;
	int retry, int_st;
	unsigned int tx_data = 0;
	int tx_size, iter = 0;

#if defined (USE_NAND_GDMA)
	// receive data by use_gdma
	if (use_gdma) {
		if (_ra_nand_dma_push((unsigned long)p, len))
			len = 0;
		printk("%s: fail \n", __func__);
		return len;
	}
#endif

	// no gdma
	retry = READ_STATUS_RETRY;
	while (len > 0) {
		int_st = ra_inl(NFC_INT_ST);
		if (int_st & INT_ST_TX_BUF_RDY) {
			tx_size = min(len, 4);
			if (tx_size == 4)
				tx_data = (*p++);
			else {
				__u8 *q = (__u8 *)p;
				for (iter = 0; iter < tx_size; iter++)
					tx_data |= (*q++ << (8*iter));
				p = (__u32 *)q;
			}
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
			if (retry-- < 0) {
				ra_dbg("%s p:%p buf:%p \n", __func__, p, buf);
				break;
			}
		}
	}

	return ((int)p - (int)buf);
}

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
		printk("%s: chip select error, offs(%x)\n", __func__, offs);
		return -1;
	}

	if (!read_only)
		return nfc_check_wp();
	
	return 0;
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

	// wait nfc idle,
	if (snooze_ms == 0)
		snooze_ms = 1;
	else
		schedule_timeout(snooze_ms * HZ / 1000);

	snooze_ms = retry = snooze_ms *1000000 / 100 ;  // ndelay(100)

	while (!NFC_TRANS_DONE() && retry--) {
		if (!cond_resched())
			ndelay(100);
	}

	if (!NFC_TRANS_DONE()) {
		printk("nfc_wait_ready: no transaction done \n");
		return NAND_STATUS_FAIL;
	}

#if !defined (NOT_SUPPORT_RB)
	//fixme
	while(!(status = nfc_device_ready()) && retry--) {
		ndelay(100);
	}

	if (status == 0) {
		printk("nfc_wait_ready: no device ready. \n");
		return NAND_STATUS_FAIL;
	}

	_nfc_read_status(&status);
	return status;
#else
	while(retry--) {
		_nfc_read_status(&status);
		if (status & NAND_STATUS_READY)
			break;
		ndelay(100);
	}
	if (retry<0)
		printk("nfc_wait_ready 2: no device ready, status(%x). \n", status);

	return status;
#endif
}

/**
 * return 0: erase OK
 * return -EIO: fail 
 */
static int nfc_erase_block(struct ra_nand_chip *ra, int row_addr)
{
	unsigned long cmd1, cmd2, bus_addr, conf;
	char status;

	cmd1 = 0x60;
	cmd2 = 0xd0;
	bus_addr = row_addr;
	conf = 0x00511 | ((CFG_ROW_ADDR_CYCLE)<<16);

	// set NFC
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
		printk("%s: fail \n", __func__);
		return -EIO;
	}
	
	return 0;

}

static inline int _nfc_read_raw_data(int cmd1, int cmd2, int bus_addr, int bus_addr2, int conf, char *buf, int len, int flags)
{
	int ret;

	CLEAR_INT_STATUS();
	ra_outl(NFC_CMD1, cmd1);
	ra_outl(NFC_CMD2, cmd2);
	ra_outl(NFC_ADDR, bus_addr);
#if defined (CONFIG_RALINK_MT7620)
	ra_outl(NFC_ADDR2, bus_addr2);
#endif
	ra_outl(NFC_CONF, conf);

	ret = _ra_nand_pull_data(buf, len, flags & FLAG_USE_GDMA);
	if (ret != len) {
		ra_dbg("%s: ret:%x (%x) \n", __func__, ret, len);
		return NAND_STATUS_FAIL;
	}

	//FIXME, this section is not necessary
	ret = nfc_wait_ready(0); //wait ready 
	/* to prevent the DATA FIFO 's old data from next operation */
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x02); //clear data buffer
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) & ~0x02); //clear data buffer

	if (ret & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
		return NAND_STATUS_FAIL;
	}

	return 0;
}

static inline int _nfc_write_raw_data(int cmd1, int cmd3, int bus_addr, int bus_addr2, int conf, char *buf, int len, int flags)
{
	int ret;

	CLEAR_INT_STATUS();
	ra_outl(NFC_CMD1, cmd1);
	ra_outl(NFC_CMD3, cmd3);
	ra_outl(NFC_ADDR, bus_addr);
#if defined (CONFIG_RALINK_MT7620)
	ra_outl(NFC_ADDR2, bus_addr2);
#endif
	ra_outl(NFC_CONF, conf);

	ret = _ra_nand_push_data(buf, len, flags & FLAG_USE_GDMA);
	if (ret != len) {
		ra_dbg("%s: ret:%x (%x) \n", __func__, ret, len);
		return NAND_STATUS_FAIL;
	}

	ret = nfc_wait_ready(1); //write wait 1ms
	/* to prevent the DATA FIFO 's old data from next operation */
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x02); //clear data buffer
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) & ~0x02); //clear data buffer

	if (ret & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
		return NAND_STATUS_FAIL;
	}

	return 0;
}

/**
 * @return !0: fail
 * @return 0: OK
 */
int nfc_read_oob(struct ra_nand_chip *ra, int page, unsigned int offs, char *buf, int len, int flags)
{
	unsigned int cmd1 = 0, cmd2 = 0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	unsigned int ecc_en;
	int use_gdma;
	int status;

	int pages_perblock = 1<<(ra->erase_shift - ra->page_shift);
	// constrain of nfc read function 

	BUG_ON (offs >> ra->oob_shift); //page boundry
	BUG_ON ((unsigned int)(((offs + len) >> ra->oob_shift) + page) >
		((page + pages_perblock) & ~(pages_perblock-1))); //block boundry

	use_gdma = flags & FLAG_USE_GDMA;
	ecc_en = flags & FLAG_ECC_EN;
	bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)) | (offs & ((1<<CFG_COLUMN_ADDR_CYCLE*8) - 1));

	if (is_nand_page_2048) {
		bus_addr += CFG_PAGESIZE;
		bus_addr2 = page >> (CFG_COLUMN_ADDR_CYCLE*8);
		cmd1 = 0x0;
		cmd2 = 0x30;
		conf = 0x000511| ((CFG_ADDR_CYCLE)<<16) | (len << 20);
	}
	else {
		cmd1 = 0x50;
		conf = 0x000141| ((CFG_ADDR_CYCLE)<<16) | (len << 20);
	}
	if (ecc_en) 
		conf |= (1<<3); 
#if defined (USE_NAND_GDMA)
	if (use_gdma)
		conf |= (1<<2);
#endif

	ra_dbg("%s: cmd1:%x, bus_addr:%x, conf:%x, len:%x, flag:%x\n",
	       __func__, cmd1, bus_addr, conf, len, flags);

	status = _nfc_read_raw_data(cmd1, cmd2, bus_addr, bus_addr2, conf, buf, len, flags);
	if (status & NAND_STATUS_FAIL) {
		printk("%s: fail\n", __func__);
		return -EIO;
	}

	return 0; 
}

/**
 * @return !0: fail
 * @return 0: OK
 */
int nfc_write_oob(struct ra_nand_chip *ra, int page, unsigned int offs, char *buf, int len, int flags)
{
	unsigned int cmd1 = 0, cmd3=0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int use_gdma;
	int status;

	int pages_perblock = 1<<(ra->erase_shift - ra->page_shift);
	// constrain of nfc read function 

	BUG_ON (offs >> ra->oob_shift); //page boundry
	BUG_ON ((unsigned int)(((offs + len) >> ra->oob_shift) + page) >
		((page + pages_perblock) & ~(pages_perblock-1))); //block boundry 

	use_gdma = flags & FLAG_USE_GDMA;
	bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)) | (offs & ((1<<CFG_COLUMN_ADDR_CYCLE*8) - 1));

	if (is_nand_page_2048) {
		cmd1 = 0x80;
		cmd3 = 0x10;
		bus_addr += CFG_PAGESIZE;
		bus_addr2 = page >> (CFG_COLUMN_ADDR_CYCLE*8);
		conf = 0x001123 | ((CFG_ADDR_CYCLE)<<16) | (len << 20);
	}
	else {
		cmd1 = 0x08050;
		cmd3 = 0x10;
		conf = 0x001223 | ((CFG_ADDR_CYCLE)<<16) | (len << 20);
	}
#if defined (USE_NAND_GDMA)
	if (use_gdma)
		conf |= (1<<2);
#endif

	// set NFC
	ra_dbg("%s: cmd1: %x, cmd3: %x bus_addr: %x, conf: %x, len:%x\n", 
	       __func__, cmd1, cmd3, bus_addr, conf, len);

	status = _nfc_write_raw_data(cmd1, cmd3, bus_addr, bus_addr2, conf, buf, len, flags);
	if (status & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
		return -EIO;
	}

	return 0;
}

#if defined (CONFIG_RALINK_MT7620)
// 1-bit error detection
static int one_bit_correction(char *ecc1, char *ecc2, int *bytes, int *bits)
{
	// check if ecc and expected are all valid
	char *p, nibble, crumb;
	int i, xor, iecc1 = 0, iecc2 = 0;
	
	printk("correction : %x %x %x\n", ecc1[0], ecc1[1], ecc1[2]);
	printk("correction : %x %x %x\n", ecc2[0], ecc2[1], ecc2[2]);
	
	p = (char *)ecc1;
	for (i = 0; i < CONFIG_ECC_BYTES; i++)
	{
		nibble = *(p+i) & 0xf;
		if ((nibble != 0x0) && (nibble != 0xf) && (nibble != 0x3) && (nibble != 0xc) && 
			(nibble != 0x5) && (nibble != 0xa) && (nibble != 0x6) && (nibble != 0x9))
			return -1;
		nibble = ((*(p+i)) >> 4) & 0xf;
		if ((nibble != 0x0) && (nibble != 0xf) && (nibble != 0x3) && (nibble != 0xc) && 
			(nibble != 0x5) && (nibble != 0xa) && (nibble != 0x6) && (nibble != 0x9))
			return -1;		
	}

	p = (char *)ecc2;
	for (i = 0; i < CONFIG_ECC_BYTES; i++)
	{
		nibble = *(p+i) & 0xf;
		if ((nibble != 0x0) && (nibble != 0xf) && (nibble != 0x3) && (nibble != 0xc) && 
			(nibble != 0x5) && (nibble != 0xa) && (nibble != 0x6) && (nibble != 0x9))
			return -1;
		nibble = ((*(p+i)) >> 4) & 0xf;
		if ((nibble != 0x0) && (nibble != 0xf) && (nibble != 0x3) && (nibble != 0xc) && 
			(nibble != 0x5) && (nibble != 0xa) && (nibble != 0x6) && (nibble != 0x9))
			return -1;		
	}
	
	memcpy(&iecc1, ecc1, 3);
	memcpy(&iecc2, ecc2, 3);
	
	xor = iecc1 ^ iecc2;
	printk("xor = %x (%x %x)\n", xor, iecc1, iecc2);
	
	*bytes = 0;
	for (i = 0; i < 9; i++)
	{
		crumb = (xor >> (2*i)) & 0x3;
		if ((crumb == 0x0) || (crumb == 0x3))
			return -1;
		if (crumb == 0x2)
			*bytes += (1 << i);
	}

	*bits = 0;
	for (i = 0; i < 3; i++)
	{
		crumb = (xor >> (18 + 2*i)) & 0x3;
		if ((crumb == 0x0) || (crumb == 0x3))
			return -1;
		if (crumb == 0x2)
			*bits += (1 << i);
	}
	
	return 0;
}
#endif

int nfc_ecc_verify(struct ra_nand_chip *ra, char *buf, int page, int mode)
{
	int ret, i;
	char *p, *e;
	int ecc;
	
	//ra_dbg("%s, page:%x mode:%d\n", __func__, page, mode);

	if (mode == FL_WRITING) {
		int len = CFG_PAGESIZE + CFG_PAGE_OOBSIZE;
		int conf = 0x000141| ((CFG_ADDR_CYCLE)<<16) | (len << 20); 
		conf |= (1<<3); //(ecc_en) 
#if defined (USE_NAND_GDMA)
		conf |= (1<<2); // (use_gdma)
#endif

		p = ra->readback_buffers;
		ret = nfc_read_page(ra, ra->readback_buffers, page, FLAG_USE_GDMA | FLAG_ECC_EN);
		if (ret == 0) 
			goto ecc_check;
		
		//FIXME, double comfirm
		printk("%s: read back fail, try again \n",__func__);
		ret = nfc_read_page(ra, ra->readback_buffers, page, FLAG_USE_GDMA | FLAG_ECC_EN);
		if (ret != 0) {
			printk("\t%s: read back fail agian \n",__func__);
			goto bad_block;
		}
	}
	else if (mode == FL_READING) {
		p = buf;
	}	
	else
		return -2;

ecc_check:
	p += CFG_PAGESIZE;
	if (!is_nand_page_2048) {
		ecc = ra_inl(NFC_ECC); 
		if (ecc == 0) //clean page.
			return 0;
		e = (char*)&ecc;
		for (i=0; i<CONFIG_ECC_BYTES; i++) {
			int eccpos = CONFIG_ECC_OFFSET + i;
			if (*(p + eccpos) != (char)0xff)
				break;
			if (i == CONFIG_ECC_BYTES - 1) {
				printk("skip ecc 0xff at page %x\n", page);
				return 0;
			}
		}
		for (i=0; i<CONFIG_ECC_BYTES; i++) {
			int eccpos = CONFIG_ECC_OFFSET + i;
			if (*(p + eccpos) != *(e + i)) {
				printk("%s mode:%s, invalid ecc, page: %x read:%x %x %x, ecc:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,	
						*(p+ CONFIG_ECC_OFFSET), *(p+ CONFIG_ECC_OFFSET+1), *(p+ CONFIG_ECC_OFFSET +2), ecc);
				return -1;
			}
		}
	}
#if defined (CONFIG_RALINK_MT7620)
	else {
		int ecc2, ecc3, ecc4, qsz;
		char *e2, *e3, *e4;
		int correction_flag = 0;
		ecc = ra_inl(NFC_ECC_P1);
		ecc2 = ra_inl(NFC_ECC_P2);
		ecc3 = ra_inl(NFC_ECC_P3);
		ecc4 = ra_inl(NFC_ECC_P4);
		e = (char*)&ecc;
		e2 = (char*)&ecc2;
		e3 = (char*)&ecc3;
		e4 = (char*)&ecc4;
		qsz = CFG_PAGE_OOBSIZE / 4;
		if (ecc == 0 && ecc2 == 0 && ecc3 == 0 && ecc4 == 0)
			return 0;
		for (i=0; i<CONFIG_ECC_BYTES; i++) {
			int eccpos = CONFIG_ECC_OFFSET + i;
			if (*(p + eccpos) != (char)0xff)
				break;
			else if (*(p + eccpos + qsz) != (char)0xff)
				break;
			else if (*(p + eccpos + qsz*2) != (char)0xff)
				break;
			else if (*(p + eccpos + qsz*3) != (char)0xff)
				break;
			if (i == CONFIG_ECC_BYTES - 1) {
				printk("skip ecc 0xff at page %x\n", page);
				return 0;
			}
		}
		for (i=0; i<CONFIG_ECC_BYTES; i++) {
			int eccpos = CONFIG_ECC_OFFSET + i;
			if (*(p + eccpos) != *(e + i)) {
				printk("%s mode:%s, invalid ecc, page: %x read:%x %x %x, ecc:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+ CONFIG_ECC_OFFSET), *(p+ CONFIG_ECC_OFFSET+1), *(p+ CONFIG_ECC_OFFSET +2), ecc);
				correction_flag |= 0x1;
			}
			if (*(p + eccpos + qsz) != *(e2 + i)) {
				printk("%s mode:%s, invalid ecc2, page: %x read:%x %x %x, ecc2:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+CONFIG_ECC_OFFSET+qsz), *(p+ CONFIG_ECC_OFFSET+1+qsz), *(p+ CONFIG_ECC_OFFSET+2+qsz), ecc2);
				correction_flag |= 0x2;
			}
			if (*(p + eccpos + qsz*2) != *(e3 + i)) {
				printk("%s mode:%s, invalid ecc3, page: %x read:%x %x %x, ecc3:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+CONFIG_ECC_OFFSET+qsz*2), *(p+ CONFIG_ECC_OFFSET+1+qsz*2), *(p+ CONFIG_ECC_OFFSET+2+qsz*2), ecc3);
				correction_flag |= 0x4;
			}
			if (*(p + eccpos + qsz*3) != *(e4 + i)) {
				printk("%s mode:%s, invalid ecc4, page: %x read:%x %x %x, ecc4:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+CONFIG_ECC_OFFSET+qsz*3), *(p+ CONFIG_ECC_OFFSET+1+qsz*3), *(p+ CONFIG_ECC_OFFSET+2+qsz*3), ecc4);
				correction_flag |= 0x8;
			}
		}

		if (correction_flag)
		{
			printk("trying to do correction!\n");
			if (correction_flag & 0x1)
			{
				int bytes, bits;
				char *pBuf = p - CFG_PAGESIZE;
			
				if (one_bit_correction(p + CONFIG_ECC_OFFSET, e, &bytes, &bits) == 0)
				{
					pBuf[bytes] = pBuf[bytes] ^ (1 << bits);
					printk("1. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printk("failed to correct!\n");
					return -1;
				}
			}

			if (correction_flag & 0x2)
			{
				int bytes, bits;
				char *pBuf = (p - CFG_PAGESIZE) + CFG_PAGESIZE/4;
			
				if (one_bit_correction((p + CONFIG_ECC_OFFSET + qsz), e2, &bytes, &bits) == 0)
				{
					pBuf[bytes] = pBuf[bytes] ^ (1 << bits);
					printk("2. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printk("failed to correct!\n");
					return -1;
				}
			}
			if (correction_flag & 0x4)
			{
				int bytes, bits;
				char *pBuf = (p - CFG_PAGESIZE) + CFG_PAGESIZE/2;
			
				if (one_bit_correction((p + CONFIG_ECC_OFFSET + qsz * 2), e3, &bytes, &bits) == 0)
				{
					pBuf[bytes] = pBuf[bytes] ^ (1 << bits);
					printk("3. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printk("failed to correct!\n");
					return -1;
				}
			}
			if (correction_flag & 0x8)
			{
				int bytes, bits;
				char *pBuf = (p - CFG_PAGESIZE) + CFG_PAGESIZE*3/4;
			
				if (one_bit_correction((p + CONFIG_ECC_OFFSET + qsz * 3), e4, &bytes, &bits) == 0)
				{
					pBuf[bytes] = pBuf[bytes] ^ (1 << bits);
					printk("4. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printk("failed to correct!\n");
					return -1;
				}
			}
		}

	}
#endif
	return 0;

bad_block:
	return -1;
}

/**
 * @return -EIO, writing size is less than a page 
 * @return 0, OK
 */
int nfc_read_page(struct ra_nand_chip *ra, char *buf, int page, int flags)
{
	unsigned int cmd1 = 0, cmd2 = 0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	unsigned int ecc_en;
	int use_gdma;
	int size, offs;
	int status = 0;

	use_gdma = flags & FLAG_USE_GDMA;
	ecc_en = flags & FLAG_ECC_EN;

	page = page & (CFG_CHIPSIZE - 1); // chip boundary
	size = CFG_PAGESIZE + CFG_PAGE_OOBSIZE; //add oobsize
	offs = 0;

	while (size > 0) {
		int len;
		len = size;
		bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)) | (offs & ((1<<CFG_COLUMN_ADDR_CYCLE*8)-1)); 
		if (is_nand_page_2048) {
			bus_addr2 = page >> (CFG_COLUMN_ADDR_CYCLE*8);
			cmd1 = 0x0;
			cmd2 = 0x30;
			conf = 0x000511| ((CFG_ADDR_CYCLE)<<16) | (len << 20); 
		}
		else {
			if (offs & ~(CFG_PAGESIZE-1))
				cmd1 = 0x50;
			else if (offs & ~((1<<CFG_COLUMN_ADDR_CYCLE*8)-1))
				cmd1 = 0x01;
			else
				cmd1 = 0;

			conf = 0x000141| ((CFG_ADDR_CYCLE)<<16) | (len << 20); 
		}
		if (ecc_en)
			conf |= (1<<3);
#if defined (USE_NAND_GDMA)
		if (use_gdma)
			conf |= (1<<2);
#endif

		status = _nfc_read_raw_data(cmd1, cmd2, bus_addr, bus_addr2, conf, buf+offs, len, flags);
		if (status & NAND_STATUS_FAIL) {
			printk("%s: fail \n", __func__);
			return -EIO;
		}

		offs += len;
		size -= len;
	}

	// verify and correct ecc
	if ((flags & (FLAG_VERIFY | FLAG_ECC_EN)) == (FLAG_VERIFY | FLAG_ECC_EN)) {
		status = nfc_ecc_verify(ra, buf, page, FL_READING);	
		if (status != 0) {
			printk("%s: fail, buf:%x, page:%x, flag:%x\n", 
			       __func__, (unsigned int)buf, page, flags);
			return -EBADMSG;
		}
	}
	else {
		// fix,e not yet support
		ra->buffers_page = -1; //cached
	}

	return 0;
}


/** 
 * @return -EIO, fail to write
 * @return 0, OK
 */
int nfc_write_page(struct ra_nand_chip *ra, char *buf, int page, int flags)
{
	unsigned int cmd1 = 0, cmd3, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	unsigned int ecc_en;
	int use_gdma;
	int size;
	int status;
	uint8_t *oob = buf + (1<<ra->page_shift);

	use_gdma = flags & FLAG_USE_GDMA;
	ecc_en = flags & FLAG_ECC_EN;

	oob[ra->badblockpos] = 0xff;	//tag as good block.
	ra->buffers_page = -1; //cached

	page = page & (CFG_CHIPSIZE-1); //chip boundary
	size = CFG_PAGESIZE + CFG_PAGE_OOBSIZE; //add oobsize
	bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)); //write_page always write from offset 0.

	if (is_nand_page_2048) {
		bus_addr2 = page >> (CFG_COLUMN_ADDR_CYCLE*8);
		cmd1 = 0x80;
		cmd3 = 0x10;
		conf = 0x001123| ((CFG_ADDR_CYCLE)<<16) | (size << 20); 
	}
	else {
		cmd1 = 0x8000;
		cmd3 = 0x10;
		conf = 0x001223| ((CFG_ADDR_CYCLE)<<16) | (size << 20); 
	}
	if (ecc_en) 
		conf |= (1<<3); //enable ecc
#if defined (USE_NAND_GDMA)
	if (use_gdma)
		conf |= (1<<2);
#endif

	// set NFC
	ra_dbg("nfc_write_page: cmd1: %x, cmd3: %x bus_addr: %x, conf: %x, len:%x\n", 
	       cmd1, cmd3, bus_addr, conf, size);

	status = _nfc_write_raw_data(cmd1, cmd3, bus_addr, bus_addr2, conf, buf, size, flags);
	if (status & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
		return -EIO;
	}
	

	if (flags & FLAG_VERIFY) { // verify and correct ecc
		status = nfc_ecc_verify(ra, buf, page, FL_WRITING);

#ifdef RANDOM_GEN_BAD_BLOCK
		if (((random32() & 0x1ff) == 0x0) && (page >= 0x100)) // randomly create bad block
		{
			printk("hmm... create a bad block at page %x\n", (bus_addr >> 16));
			status = -1;
		}
#endif
		if (status != 0) {
			printk("%s: ecc_verify fail: ret:%x \n", __func__, status);
			oob[ra->badblockpos] = 0x33; 
			page -= page % (CFG_BLOCKSIZE/CFG_PAGESIZE);
			printk("create a bad block at page %x\n", page);
			if (!is_nand_page_2048)
				status = nfc_write_oob(ra, page, ra->badblockpos, oob+ra->badblockpos, 1, flags);
			else
			{
				status = _nfc_write_raw_data(cmd1, cmd3, bus_addr, bus_addr2, conf, buf, size, flags);
				nfc_write_oob(ra, page, 0, oob, 16, FLAG_NONE);
			}
			return -EIO;
		}
	}


	ra->buffers_page = page; //cached
	return 0;
}

/*************************************************************
 * nand internal process 
 *************************************************************/

/**
 * nand_release_device - [GENERIC] release chip
 * @mtd:	MTD device structure
 *
 * Deselect, release chip lock and wake up anyone waiting on the device
 */
static void nand_release_device(struct ra_nand_chip *ra)
{
	/* De-select the NAND device */
	nfc_select_chip(ra, -1);

	/* Release the controller and the chip */
	ra->state = FL_READY;

	mutex_unlock(ra->controller);
}

/**
 * nand_get_device - [GENERIC] Get chip for selected access
 * @chip:	the nand chip descriptor
 * @mtd:	MTD device structure
 * @new_state:	the state which is requested
 *
 * Get the device and lock it for exclusive access
 */
static int
nand_get_device(struct ra_nand_chip *ra, int new_state)
{
	int ret = 0;

	ret = mutex_lock_interruptible(ra->controller);
	if (!ret)
		ra->state = new_state;

	return ret;
}

/*************************************************************
 * nand internal process 
 *************************************************************/

int nand_bbt_get(struct ra_nand_chip *ra, int block)
{
	int byte, bits;

	bits = block * BBTTAG_BITS;
	byte = bits / 8;
	bits = bits % 8;

	return (ra->bbt[byte] >> bits) & BBTTAG_BITS_MASK;
}

int nand_bbt_set(struct ra_nand_chip *ra, int block, int tag)
{
	int byte, bits;

	bits = block * BBTTAG_BITS;
	byte = bits / 8;
	bits = bits % 8;

	// If previous tag is bad, dont overwrite it	
	if (((ra->bbt[byte] >> bits) & BBTTAG_BITS_MASK) == BBT_TAG_BAD)
	{
		return BBT_TAG_BAD;
	}

	ra->bbt[byte] = (ra->bbt[byte] & ~(BBTTAG_BITS_MASK << bits)) | ((tag & BBTTAG_BITS_MASK) << bits);

	return tag;
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
		ret = nfc_read_oob(ra, page, ra->badblockpos, (char*)&tag, 1, FLAG_NONE);
		if (ret == 0)
			tag = ((le32_to_cpu(tag) & 0x0ff) == 0x0ff) ? BBT_TAG_GOOD : BBT_TAG_BAD;
		else
			tag = BBT_TAG_BAD;

		nand_bbt_set(ra, block, tag);
	}

#if 0
	ra_dbg("%s offs: %x , ret: %x, tag:%s\n", 
	       __func__, (int)offs, ret, str[tag]);
#endif

	if (tag != BBT_TAG_GOOD) {
		printk("%s: offs:%x tag: %s \n", __func__, (unsigned int)offs, str[tag]);
		return 1;
	}

	return 0;
}

/**
 * nand_block_markbad -
 */
int nand_block_markbad(struct ra_nand_chip *ra, loff_t offs)
{
	int page, block;
	int ret = 4;
	unsigned int tag;
	char *ecc;

	// align with chip
	ra_dbg("%s offs: %x \n", __func__, (int)offs);

	offs = offs & ((1<<ra->chip_shift) -1);

	page = offs >> ra->page_shift;
	block = offs >> ra->erase_shift;

	tag = nand_bbt_get(ra, block);

	if (tag == BBT_TAG_BAD) {
		printk("%s: mark repeatedly \n", __func__);
		return 0;
	}

	// new tag as bad
	tag = BBT_TAG_BAD;
	ret = nfc_read_page(ra, ra->buffers, page, FLAG_NONE);
	if (ret != 0) {
		printk("%s: fail to read bad block tag \n", __func__);
		goto tag_bbt;
	}

	ecc = &ra->buffers[(1<<ra->page_shift)+ra->badblockpos];
	if (*ecc == (char)0x0ff) {
		//tag into flash
		*ecc = (char)tag;
		ret = nfc_write_page(ra, ra->buffers, page, FLAG_USE_GDMA);
		if (ret)
			printk("%s: fail to write bad block tag \n", __func__);
	}

tag_bbt:
	//update bbt
	nand_bbt_set(ra, block, tag);

	return 0;
}

/**
 * nand_erase_nand - [Internal] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 * @allowbbt:	allow erasing the bbt area
 *
 * Erase one ore more blocks
 */
int nand_erase_nand(struct ra_nand_chip *ra, struct erase_info *instr)
{
	int page, len, status, ret;
	unsigned int addr, blocksize = 1<<ra->erase_shift;

	ra_dbg("%s: start:%x, len:%x \n", __func__, 
	       (unsigned int)instr->addr, (unsigned int)instr->len);

	if (BLOCK_ALIGNED(instr->addr) || BLOCK_ALIGNED(instr->len)) {
		ra_dbg("%s: erase block not aligned, addr:%x len:%x\n", __func__, instr->addr, instr->len);
		return -EINVAL;
	}

	instr->fail_addr = 0xffffffff;

	len = instr->len;
	addr = instr->addr;
	instr->state = MTD_ERASING;

	while (len) {

		page = (int)(addr >> ra->page_shift);

		/* select device and check wp */
		if (nfc_enable_chip(ra, addr, 0)) {
			printk("%s: nand is write protected \n", __func__);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}

#ifdef SKIP_BAD_BLOCK
		do {
			int newpage = page_remap(ra, page);

			if (newpage < 0)
			{
				//printk("page_remap failed, page = 0x%x\n", page);
				instr->state = MTD_ERASE_DONE; // the rest blocks in this partition are bad, erase done
				goto erase_exit;
			}

			if (nand_block_checkbad(ra, newpage << ra->page_shift)) {
				if (is_skip_bad_block(ra, newpage))
					continue;
				else {
					instr->state = MTD_ERASE_FAILED;
					goto erase_exit;
				}
			}

			/*
			 * Invalidate the page cache, if we erase the block which
			 * contains the current cached page
			 */
			if (BLOCK_ALIGNED(newpage << ra->page_shift) == BLOCK_ALIGNED(ra->buffers_page << ra->page_shift))
				ra->buffers_page = -1;
		
			status = nfc_erase_block(ra, newpage);
			if (status)
			{
				char *oob;
				nand_bbt_set(ra, newpage >> CONFIG_NUMPAGE_PER_BLOCK_BIT, BBT_TAG_BAD);
				oob = &ra->buffers[(1<<ra->page_shift)];
				memset(oob, 0xff, CFG_PAGE_OOBSIZE);
				oob[ra->badblockpos] = 0x34;
				nfc_write_oob(ra, newpage, 0, oob, CFG_PAGE_OOBSIZE, FLAG_NONE);
			}
			else
				break;

		} while(1);
			
#else
		/* if we have a bad block, we do not erase bad blocks */
		if (nand_block_checkbad(ra, addr)) {
			printk(KERN_WARNING "nand_erase: attempt to erase a "
			       "bad block at 0x%08x\n", addr);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}

		/*
		 * Invalidate the page cache, if we erase the block which
		 * contains the current cached page
		 */
		if (BLOCK_ALIGNED(addr) == BLOCK_ALIGNED(ra->buffers_page << ra->page_shift))
			ra->buffers_page = -1;

		status = nfc_erase_block(ra, page);
#endif

		/* See if block erase succeeded */
		if (status) {
			printk("%s: failed erase, page 0x%08x\n", __func__, page);
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = (page << ra->page_shift);
			goto erase_exit;
		}


		/* Increment page address and decrement length */
		len -= blocksize;
		addr += blocksize;

	}
	instr->state = MTD_ERASE_DONE;

erase_exit:

	ret = ((instr->state == MTD_ERASE_DONE) ? 0 : -EIO);
	/* Do call back function */
	if (!ret)
		mtd_erase_callback(instr);

	if (ret) {
		nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_BAD);
	}

	/* Return more or less happy */
	return ret;
}

static int nand_write_oob_buf(struct ra_nand_chip *ra, uint8_t *buf, uint8_t *oob, size_t size, 
			      int mode, int ooboffs) 
{
	size_t oobsize = 1<<ra->oob_shift;
	int retsize = 0;

	ra_dbg("%s: size:%x, mode:%x, offs:%x  \n", __func__, size, mode, ooboffs);

	switch(mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_RAW:
		if (ooboffs > oobsize)
			return -1;

		size = min(size, oobsize - ooboffs);
		memcpy(buf + ooboffs, oob, size);
		retsize = size;
		break;

	case MTD_OPS_AUTO_OOB:
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
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_RAW:
		if (ooboffs > oobsize)
			return -1;

		size = min(size, oobsize - ooboffs);
		memcpy(oob, buf + ooboffs, size);
		return size;

	case MTD_OPS_AUTO_OOB: {
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

#ifdef UBIFS_ECC_0_PATCH

static int check_ecc_0(struct ra_nand_chip *ra, int page)
{
	char oob_buf[CFG_PAGE_OOBSIZE];
	nfc_read_oob(ra, page, 0, oob_buf, CFG_PAGE_OOBSIZE, FLAG_NONE);
	if ((oob_buf[CONFIG_ECC_OFFSET+1] == 0x0) && (oob_buf[CONFIG_ECC_OFFSET+1] == 0x0) && (oob_buf[CONFIG_ECC_OFFSET+1] == 0x0))
	{
		char page_buf[CFG_PAGESIZE+CFG_PAGE_OOBSIZE];
		int ret, i;

		ret = nfc_read_page(ra, page_buf, page, FLAG_NONE);
		if (ret)
		{
			ret = nfc_read_page(ra, page_buf, page, FLAG_NONE);
			if (ret)
				return 0;
		}
		for (i = 0; i < CFG_PAGESIZE; i++)
			if (page_buf[i] != (char)0xff)
				return 0;
		return 1;
	}
	else
		return 0;
}

static void fix_ecc_0(struct ra_nand_chip *ra, int page)
{
	int i, j, ret;
	int page_is_empty;
	int start_page = (page >> CONFIG_NUMPAGE_PER_BLOCK_BIT) << CONFIG_NUMPAGE_PER_BLOCK_BIT;
	unsigned char *block_buf;

	block_buf = (unsigned char *) kzalloc(CFG_BLOCKSIZE + CFG_BLOCK_OOBSIZE, GFP_KERNEL | GFP_DMA);
	if (!block_buf)
	{
		printk("%s: can not allocate buffer\n", __func__);
		return;
	}
	memset(block_buf, 0xff, CFG_BLOCKSIZE + CFG_BLOCK_OOBSIZE);

	// read all data from block
	for (i = 0; i < (1 << CONFIG_NUMPAGE_PER_BLOCK_BIT); i++)
	{
		ret = nfc_read_page(ra, block_buf + (i * (CFG_PAGESIZE + CFG_PAGE_OOBSIZE)), start_page+i, FLAG_NONE);
		if (ret)
		{
			// read again
			ret = nfc_read_page(ra, block_buf + (i * (CFG_PAGESIZE + CFG_PAGE_OOBSIZE)), start_page+i, FLAG_NONE);
			if (ret)
			{
				printk("%s: read page 0x%x error\n", __func__, start_page+i);
				kfree(block_buf);
				return;
			}
		}
		
	}

	// erase
	ret = nfc_erase_block(ra, start_page);
	if (ret)
	{
		printk("%s: erase failed\n", __func__);
		kfree(block_buf);
		return;
	}

	// program again
	for (i = 0; i < (1 << CONFIG_NUMPAGE_PER_BLOCK_BIT); i++)
	{
		page_is_empty = 1;
		for (j = 0; j < CFG_PAGESIZE; j++)
		{
			if (*(block_buf+j + (i * (CFG_PAGESIZE + CFG_PAGE_OOBSIZE))) != (unsigned char)0xff)
			{
				page_is_empty = 0;
				break;
			}
		}
		
		if (!page_is_empty)
		{
			ret = nfc_write_page(ra, block_buf + (i * (CFG_PAGESIZE+CFG_PAGE_OOBSIZE)), start_page+i, (FLAG_VERIFY | FLAG_ECC_EN));
			if (ret)
			{
				printk("%s: write page start_page=%x i=%x failed\n", __func__, start_page, i);
				nand_bbt_set(ra, start_page >> CONFIG_NUMPAGE_PER_BLOCK_BIT, BBT_TAG_BAD);
				kfree(block_buf);
				return;
			}
		}
	}
	kfree(block_buf);
}

#endif

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
	//int i = 0; //for ra_dbg only
#ifndef SKIP_BAD_BLOCK
#ifdef UBIFS_ECC_0_PATCH
	int page_is_empty;
#endif
#endif

	ra_dbg("%s: to:%x, ops data:%p, oob:%p datalen:%x ooblen:%x, ooboffs:%x oobmode:%x \n", 
	       __func__, (unsigned int)to, data, oob, datalen, ooblen, ops->ooboffs, ops->mode);

	ops->retlen = 0;
	ops->oobretlen = 0;

	/* Invalidate the page cache, when we write to the cached page */
	ra->buffers_page = -1;

	if (data ==0)
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
		if (ops->mode == MTD_OPS_AUTO_OOB) {
			nfc_read_oob(ra, page, 0, ra->buffers, len, FLAG_USE_GDMA);
		}
		//prepare buffers
		nand_write_oob_buf(ra, ra->buffers, oob, ooblen, ops->mode, ops->ooboffs);
		// write out buffer to chip
		nfc_write_oob(ra, page, 0, ra->buffers, len, FLAG_USE_GDMA);

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

		ra_dbg("%s (%d): addr:%x, ops data:%p, oob:%p datalen:%x ooblen:%x, ooboffs:%x \n", 
		       __func__, i++, (unsigned int)addr, data, oob, datalen, ooblen, ops->ooboffs);

		page = (int)((addr & ((1<<ra->chip_shift)-1)) >> ra->page_shift); //chip boundary
		
		/* select chip, and check if it is write protected */
		if (nfc_enable_chip(ra, addr, 0))
			return -EIO;

		// oob write
		if (ops->mode == MTD_OPS_AUTO_OOB) {
			//fixme, this path is not yet varified 
			nfc_read_oob(ra, page, 0, ra->buffers + pagesize, oobsize, FLAG_NONE);
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

#ifdef SKIP_BAD_BLOCK
		do {
			int newpage = page_remap(ra, page);
			if (newpage < 0)
			{
				printk("page_remap failed, page = 0x%x\n", page);
				return -1;
			}

#ifdef UBIFS_ECC_0_PATCH
			if (!is_skip_bad_block(ra, newpage))
			{
				if (check_ecc_0(ra, newpage))
				{
					fix_ecc_0(ra, newpage);
				}
			}
#endif
			ret = nfc_write_page(ra, ra->buffers, newpage, FLAG_USE_GDMA | FLAG_VERIFY |
					     ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0 : ecc_en ));
			if (ret && is_skip_bad_block(ra, newpage)) {
				int new_blk;

				ret = write_next_on_fail(ra, ra->buffers, newpage, FLAG_USE_GDMA | FLAG_VERIFY |
					     ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0 : ecc_en ), &new_blk);
				if (ret)
				{
					nand_bbt_set(ra, newpage >> CONFIG_NUMPAGE_PER_BLOCK_BIT, BBT_TAG_BAD);
					return -1;
				}

				nand_bbt_set(ra, newpage >> CONFIG_NUMPAGE_PER_BLOCK_BIT, BBT_TAG_BAD);
				break;
			}
			else if (ret && !is_skip_bad_block(ra, newpage))
				return -EIO;
			else
				break;
			
		} while(1);
#else
#ifdef UBIFS_ECC_0_PATCH
		{
			int i;
			
			page_is_empty = 1;
			for (i = 0; i < CFG_PAGESIZE; i++)
			{
				if (ra->buffers[i] != (char)0xff)
				{
					page_is_empty = 0;
					break;
				}
			}
		}
		
		if (!page_is_empty)
		{
			if (check_ecc_0(ra, page))
			{
				fix_ecc_0(ra, page);
			}
			
			ret = nfc_write_page(ra, ra->buffers, page, FLAG_USE_GDMA | FLAG_VERIFY |
					     ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0 : ecc_en ));
			if (ret) {
				nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_BAD);
				return ret;
			}
			nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_GOOD);
		}
#else
		ret = nfc_write_page(ra, ra->buffers, page, FLAG_USE_GDMA | FLAG_VERIFY |
				     ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0 : ecc_en ));
		if (ret) {
			nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_BAD);
			return ret;
		}

		nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_GOOD);
#endif
#endif
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

		ra_dbg("%s (%d): addr:%x, ops data:%p, oob:%p datalen:%x ooblen:%x, ooboffs:%x \n", 
		       __func__, i++, (unsigned int)addr, data, oob, datalen, ooblen, ops->ooboffs);
		/* select chip */
		if (nfc_enable_chip(ra, addr, 1) < 0)
			return -EIO;

		page = (int)((addr & ((1<<ra->chip_shift)-1)) >> ra->page_shift); 
#ifdef SKIP_BAD_BLOCK
		do {
			int newpage = page_remap(ra, page);
			if (newpage < 0)
			{
				printk("page_remap failed, page = 0x%x\n", page);
				return -1;
			}
			ret = nfc_read_page(ra, ra->buffers, newpage, FLAG_USE_GDMA | FLAG_VERIFY | 
					    ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0: FLAG_ECC_EN ));

			if (ret) {
				printk("read again:\n");
				ret = nfc_read_page(ra, ra->buffers, newpage, FLAG_USE_GDMA | FLAG_VERIFY | 
						    ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0: FLAG_ECC_EN ));
				if (ret) {
//					nand_bbt_set(ra, newpage >> CONFIG_NUMPAGE_PER_BLOCK_BIT, BBT_TAG_BAD);
					return ret;
				}
				else
					break;
			}
			else
				break;
			
		} while(1);
#else
		ret = nfc_read_page(ra, ra->buffers, page, FLAG_USE_GDMA | FLAG_VERIFY | 
				    ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0: FLAG_ECC_EN ));
		//FIXME, something strange here, some page needs 2 more tries to guarantee read success.
		if (ret) {
			printk("read again:\n");
			ret = nfc_read_page(ra, ra->buffers, page, FLAG_USE_GDMA | FLAG_VERIFY | 
					    ((ops->mode == MTD_OPS_RAW || ops->mode == MTD_OPS_PLACE_OOB) ? 0: FLAG_ECC_EN ));

			if (ret) {
				printk("read again fail \n");
				nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_BAD);
				if ((ret != -EUCLEAN) && (ret != -EBADMSG)) {
					return ret;
				}
				else {
					/* ecc verification fail, but data need to be returned. */
				}
			}
			else {
				printk(" read agian susccess \n");
			}
		}
#endif

		// oob read
		if (oob && ooblen > 0) {
			len = nand_read_oob_buf(ra, oob, ooblen, ops->mode, ops->ooboffs);
			if (len < 0) {
				printk("nand_read_oob_buf: fail return %x \n", len);
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
			if (ret)
				return ret;
		}


		nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_GOOD);
		// address go further to next page, instead of increasing of length of write. This avoids some special cases wrong.
		addr = (page+1) << ra->page_shift;
	}
	return 0;
}

/************************************************************
 * the following are mtd necessary interface.
 ************************************************************/


/**
 * nand_erase - [MTD Interface] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 *
 * Erase one ore more blocks
 */
static int ramtd_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret;
	struct ra_nand_chip *ra = (struct ra_nand_chip *)mtd->priv;

	ra_dbg("%s: start:%x, len:%x \n", __func__, 
	       (unsigned int)instr->addr, (unsigned int)instr->len);

	/* Grab the lock and see if the device is available */
	nand_get_device(ra, FL_ERASING);

	ret = nand_erase_nand((struct ra_nand_chip *)mtd->priv, instr);

	/* Deselect and wake up anyone waiting on the device */
	nand_release_device(ra);

	return ret;
}

/**
 * nand_write - [MTD Interface] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * NAND write with ECC
 */
static int ramtd_nand_write(struct mtd_info *mtd, loff_t to, size_t len,
			    size_t *retlen, const uint8_t *buf)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;
	struct mtd_oob_ops ops;

	ra_dbg("%s: to 0x%x len=0x%x\n", __func__, to, len);

	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	nand_get_device(ra, FL_WRITING);

	memset(&ops, 0, sizeof(ops));
	ops.len = len;
	ops.datbuf = (uint8_t *)buf;
	ops.oobbuf = NULL;
	ops.mode =  MTD_OPS_AUTO_OOB;
	ret = nand_do_write_ops(ra, to, &ops);

	*retlen = ops.retlen;

	nand_release_device(ra);

	return ret;
}

/**
 * nand_read - [MTD Interface] MTD compability function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
 * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int ramtd_nand_read(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, uint8_t *buf)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;
	struct mtd_oob_ops ops;

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	nand_get_device(ra, FL_READING);

	memset(&ops, 0, sizeof(ops));
	ops.len = len;
	ops.datbuf = buf;
	ops.oobbuf = NULL;
	ops.mode = MTD_OPS_AUTO_OOB;
	ret = nand_do_read_ops(ra, from, &ops);
	*retlen = ops.retlen;

	nand_release_device(ra);

	return ret;
}

/**
 * nand_read_oob - [MTD Interface] NAND read data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 *
 * NAND read data and/or out-of-band data
 */
static int ramtd_nand_readoob(struct mtd_info *mtd, loff_t from,
			      struct mtd_oob_ops *ops)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;

	nand_get_device(ra, FL_READING);

	ret = nand_do_read_ops(ra, from, ops);

	nand_release_device(ra);

	return ret;
}

/**
 * nand_write_oob - [MTD Interface] NAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 */
static int ramtd_nand_writeoob(struct mtd_info *mtd, loff_t to,
			       struct mtd_oob_ops *ops)
{
	struct ra_nand_chip *ra = mtd->priv;
	int ret;

	nand_get_device(ra, FL_WRITING);

	ret = nand_do_write_ops(ra, to, ops);

	nand_release_device(ra);

	return ret;
}

/**
 * nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start
 */
static int ramtd_nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	/* Check for invalid offset */

	if (offs > mtd->size)
		return -EINVAL;

	return nand_block_checkbad((struct ra_nand_chip *)mtd->priv, offs);
}

/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int ramtd_nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	int ret;
	struct ra_nand_chip *ra = mtd->priv;

	ra_dbg("%s: \n", __func__);

	nand_get_device(ra, FL_WRITING);

	ret = nand_block_markbad(ra, ofs);

	nand_release_device(ra);

	return ret;
}

#ifdef SKIP_BAD_BLOCK

static int get_start_end_block(struct ra_nand_chip *ra, int block, int *start_blk, int *end_blk)
{
	int i, end_blk_last, part_num = ARRAY_SIZE(rt2880_partitions);

	*start_blk = 0;
	end_blk_last = 0;

	for (i = 0; i < part_num; i++) {
		if (rt2880_partitions[i].offset == MTDPART_OFS_APPEND)
			*start_blk = end_blk_last;
		else
			*start_blk = (int)(rt2880_partitions[i].offset >> ra->erase_shift);
		end_blk_last = *start_blk + (int)(rt2880_partitions[i].size >> ra->erase_shift);
		
		if (end_blk_last > *start_blk)
			*end_blk = end_blk_last - 1;
		else
			*end_blk = end_blk_last;
		
		if ((block >= *start_blk) && (block <= *end_blk)) {
#if !defined (CONFIG_MTD_NAND_USE_UBI_PART)
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
			/* use merged partition */
			if (i == NAND_MTD_KERNEL_PART_IDX || i == NAND_MTD_ROOTFS_PART_IDX) {
				*start_blk = (NAND_MTD_KERNEL_PART_OFFSET >> ra->erase_shift);
				*end_blk = *start_blk + (NAND_MTD_KERNEL_PART_SIZE >> ra->erase_shift) - 1;
			}
#endif
#endif
			return 0;
		}
	}

	return -1;
}

static int write_next_on_fail(struct ra_nand_chip *ra, char *write_buf, int page, int flags, int *to_blk)
{
	int i, j, to_page = 0, first_page;
	char *buf, *oob;
	int start_blk = 0, end_blk, block;

	// find next available block in the same MTD partition

	block = (page >> CONFIG_NUMPAGE_PER_BLOCK_BIT);
	if (get_start_end_block(ra, block, &start_blk, &end_blk) < 0)
		return -1;

	buf = kzalloc(CFG_PAGESIZE + CFG_PAGE_OOBSIZE, GFP_KERNEL | GFP_DMA);
	if (buf == NULL)
		return -1;

	oob = buf + CFG_PAGESIZE;

	for ((*to_blk) = block + 1; (*to_blk) <= end_blk; (*to_blk)++)
	{
		if (nand_bbt_get(ra, (*to_blk)) != BBT_TAG_BAD)
		{
			to_page = ((*to_blk) << CONFIG_NUMPAGE_PER_BLOCK_BIT);
			if (nfc_erase_block(ra, to_page))
			{
				nand_bbt_set(ra, (*to_blk), BBT_TAG_BAD);
				memset(oob, 0xff, CFG_PAGE_OOBSIZE);
				oob[ra->badblockpos] = 0x35;
				nfc_write_oob(ra, to_page, 0, oob, CFG_PAGE_OOBSIZE, FLAG_NONE);
				to_page = 0;
				continue;
			}
			break;
		}
	}
	if (!to_page)
	{
		printk("No available block for write_next_on_fail (%x %x)\n", *to_blk, end_blk);
		kfree(buf);
		return -1;
	}
	
// just erase the block, don't need to check now
#if 0
	// next page should be clean
	for (i = 0; i < (1 << CONFIG_NUMPAGE_PER_BLOCK_BIT); i++)
	{
		nfc_read_oob(ra, to_page+i, 0, oob, CFG_PAGE_OOBSIZE, FLAG_NONE);
		for (j = 0; j < CFG_PAGE_OOBSIZE; j++)
		{
			if (*(oob+j) != (char)0xff)
			{
				printk("next block is not clean, erase block\n");
				kfree(buf);
				return -1;
			}
		}
	}
#endif

	// read data from current block and write to next available block
	first_page = block << CONFIG_NUMPAGE_PER_BLOCK_BIT;
	for (i = 0; i < (1 << CONFIG_NUMPAGE_PER_BLOCK_BIT); i++)
	{
		if ((first_page + i) != page)
		{
			if (nfc_read_page(ra, buf, first_page + i, FLAG_ECC_EN))
			{
				// try again
				if (nfc_read_page(ra, ra->buffers, first_page + i, FLAG_ECC_EN))
				{
					printk("write_next_on_fail failed, failed to read\n");
					kfree(buf);
					return -1;
				}
			}

			for (j=0; j<CFG_PAGE_OOBSIZE; j++) {
				if (*(oob + j) != (char)0xff)
					break;
			}
			// oob will not be all 0xff if there are data in this page, then we copy the content of data
			if (j < CFG_PAGE_OOBSIZE)
			{
				memset(oob, 0xff, CFG_PAGE_OOBSIZE);
				// write to page
				if (nfc_write_page(ra, buf, to_page + i, flags))
				{
					int ret, new_blk = 0;
					printk("write_next_on_fail failed, failed to write\n");
					ret =  write_next_on_fail(ra, buf, to_page + i, flags, &new_blk);
					nand_bbt_set(ra, *to_blk, BBT_TAG_BAD);
					if (ret)
					{
						kfree(buf);
						return ret;
					}
					*to_blk = new_blk;
					to_page = ((*to_blk) << CONFIG_NUMPAGE_PER_BLOCK_BIT);
				}
			}
		}
		else
		{
			// write current page
			oob = write_buf + CFG_PAGESIZE;
			memset(oob, 0xff, CFG_PAGE_OOBSIZE);
			if (nfc_write_page(ra, write_buf, to_page+i, flags))
			{
				int ret, new_blk = 0;
				printk("write_next_on_fail failed, failed to write current page\n");
				ret =  write_next_on_fail(ra, write_buf, to_page + i, flags, &new_blk);
				nand_bbt_set(ra, *to_blk, BBT_TAG_BAD);
				if (ret)
				{
					kfree(buf);
					return ret;
				}
				*to_blk = new_blk;
				to_page = ((*to_blk) << CONFIG_NUMPAGE_PER_BLOCK_BIT);
			}
		}
	}

	kfree(buf);
	return 0;
}

static int page_remap(struct ra_nand_chip *ra, int page)
{
	int block, start_blk, end_blk;
	int j, block_offset, byte, off;
	int page_in_block;

	if (!is_skip_bad_block(ra, page))
		return page;

	block = (page >> CONFIG_NUMPAGE_PER_BLOCK_BIT);
	if (get_start_end_block(ra, block, &start_blk, &end_blk) < 0)
		return page;

	page_in_block = page & ((1 << CONFIG_NUMPAGE_PER_BLOCK_BIT) - 1);

	block_offset = block - start_blk;
	for (j = start_blk; j <= end_blk; j++)
	{
		byte = j >> 2;
		off = j & 0x3;
		if (((ra->bbt[byte] >> (off*2)) & BBTTAG_BITS_MASK) != BBT_TAG_BAD)
		{
			if (!block_offset)
				break;
			block_offset--;
		}
	}

	if (j <= end_blk)
	{
		//printk("old page = %x, new page = %x\n", page, page_in_block + (j << CONFIG_NUMPAGE_PER_BLOCK_BIT));
		return (page_in_block + (j << CONFIG_NUMPAGE_PER_BLOCK_BIT));
	}

	printk("too many bad blocks\n");
	return -1;
}

static int is_skip_bad_block(struct ra_nand_chip *ra, int page)
{
#if defined (CONFIG_MTD_UBI) || defined (CONFIG_MTD_UBI_MODULE)
	if ((page << ra->page_shift) >= UBI_PART_START_OFFSET)
		return 0;
#endif
	return 1;
}

#endif

/************************************************************
 * the init/exit section.
 */

static struct nand_ecclayout ra_oob_layout = {
	.eccbytes = CONFIG_ECC_BYTES,
	.eccpos = {5, 6, 7},
	.oobfree = {
		 {.offset = 0, .length = 4},
		 {.offset = 8, .length = 8},
		 {.offset = 0, .length = 0}
	 },
	.oobavail = 16 - (CONFIG_ECC_BYTES + 1),
};

static struct nand_ecclayout ra_oob_layout_2k = {
	.eccbytes = CONFIG_ECC_BYTES,
	.eccpos = {6, 7, 8},
	.oobfree = {
		 {.offset = 1, .length = 5},
		 {.offset = 9, .length = 7},
		 {.offset = 0, .length = 0}
	 },
	.oobavail = 16 - (CONFIG_ECC_BYTES + 1),
};

static int __init ra_nand_init(void)
{
	struct ra_nand_chip *ra;
	int alloc_size, bbt_size, buffers_size;
#if !defined (CONFIG_MTD_NAND_USE_UBI_PART)
	uint32_t kernel_size = 0x200000;
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	_ihdr_t hdr;
	loff_t offs;
	size_t ret_len = 0;
	int check;
#endif
#endif

	if (ra_check_flash_type() != BOOT_FROM_NAND)
		return 0;

	nfc_init();

#define ALIGNE_16(a) (((unsigned long)(a)+15) & ~15)
	buffers_size = ALIGNE_16((1<<CONFIG_PAGE_SIZE_BIT) + (1<<CONFIG_OOBSIZE_PER_PAGE_BIT)); //ra->buffers
	bbt_size = BBTTAG_BITS * (1<<(CONFIG_CHIP_SIZE_BIT - (CONFIG_PAGE_SIZE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT))) / 8; //ra->bbt
	bbt_size = ALIGNE_16(bbt_size);

	alloc_size = buffers_size + bbt_size;
	alloc_size += buffers_size; //for ra->readback_buffers
	alloc_size += sizeof(*ra);
	alloc_size += sizeof(*ranfc_mtd);

	ra = (struct ra_nand_chip *)kzalloc(alloc_size, GFP_KERNEL | GFP_DMA);
	if (!ra) {
		printk("%s: mem alloc fail \n", __func__);
		return -ENOMEM;
	}

	//dynamic
	ra->buffers = (char *)((char *)ra + sizeof(*ra));
	ra->readback_buffers = ra->buffers + buffers_size;
	ra->bbt = ra->readback_buffers + buffers_size;
	ranfc_mtd = (struct mtd_info *)(ra->bbt + bbt_size);

	//static
	ra->numchips		= CONFIG_NUMCHIPS;
	ra->chip_shift		= CONFIG_CHIP_SIZE_BIT;
	ra->page_shift		= CONFIG_PAGE_SIZE_BIT;
	ra->oob_shift		= CONFIG_OOBSIZE_PER_PAGE_BIT;
	ra->erase_shift		= (CONFIG_PAGE_SIZE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT);
	ra->badblockpos		= CONFIG_BAD_BLOCK_POS;
	if (is_nand_page_2048)
		ra->oob		= &ra_oob_layout_2k;
	else
		ra->oob		= &ra_oob_layout;
	ra->buffers_page	= -1;

	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x01); //set wp to high
	nfc_all_reset();

	ranfc_mtd->name		= "ra_nfc";
	ranfc_mtd->type		= MTD_NANDFLASH;
	ranfc_mtd->flags	= MTD_CAP_NANDFLASH;
	ranfc_mtd->size		= CONFIG_NUMCHIPS * CFG_CHIPSIZE;
	ranfc_mtd->erasesize	= CFG_BLOCKSIZE;
	ranfc_mtd->writesize	= CFG_PAGESIZE;
	ranfc_mtd->oobsize	= CFG_PAGE_OOBSIZE;
	ranfc_mtd->oobavail	= ra->oob->oobavail;
	ranfc_mtd->ecclayout	= ra->oob;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
	ranfc_mtd->_erase	= ramtd_nand_erase;
	ranfc_mtd->_read	= ramtd_nand_read;
	ranfc_mtd->_write	= ramtd_nand_write;
	ranfc_mtd->_read_oob	= ramtd_nand_readoob;
	ranfc_mtd->_write_oob	= ramtd_nand_writeoob;
	ranfc_mtd->_block_isbad		= ramtd_nand_block_isbad;
	ranfc_mtd->_block_markbad	= ramtd_nand_block_markbad;
#else
	ranfc_mtd->erase 	= ramtd_nand_erase;
	ranfc_mtd->read		= ramtd_nand_read;
	ranfc_mtd->write	= ramtd_nand_write;
	ranfc_mtd->read_oob	= ramtd_nand_readoob;
	ranfc_mtd->write_oob	= ramtd_nand_writeoob;
	ranfc_mtd->block_isbad		= ramtd_nand_block_isbad;
	ranfc_mtd->block_markbad	= ramtd_nand_block_markbad;
#endif
	ranfc_mtd->priv = ra;

	ranfc_mtd->owner = THIS_MODULE;
	ra->controller = &ra->hwcontrol;
	mutex_init(ra->controller);

#if !defined (CONFIG_MTD_NAND_USE_UBI_PART)
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	/* try to trigger nand_bbt_set, make the following read operation success */
	check = NAND_MTD_KERNEL_PART_OFFSET;
	while (nand_block_checkbad(ra, check)) {
		check += ranfc_mtd->erasesize;
	}
	offs = NAND_MTD_KERNEL_PART_OFFSET;
	memset(&hdr, 0, sizeof(hdr));
	ramtd_nand_read(ranfc_mtd, offs, sizeof(hdr), &ret_len, (u_char *)(&hdr));
	if (ret_len == sizeof(hdr) && hdr.ih_ksz != 0)
		kernel_size = ntohl(hdr.ih_ksz);
#endif
	/* calculate partition table */
	recalc_partitions(ranfc_mtd->size, kernel_size);
#else
	/* calculate partition table for UBIFS */
	recalc_partitions(ranfc_mtd->size);
#endif

	/* register the partitions */
	return mtd_device_register(ranfc_mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
}

static void __exit ra_nand_exit(void)
{
	struct ra_nand_chip *ra;

	if (ranfc_mtd) {
		ra = (struct ra_nand_chip  *)ranfc_mtd->priv;
		mtd_device_unregister(ranfc_mtd);
		if (ra)
			kfree(ra);
		ranfc_mtd = NULL;
	}
}

module_init(ra_nand_init);
module_exit(ra_nand_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ralink MTD NAND driver for flash chips");
