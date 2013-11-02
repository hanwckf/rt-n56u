#if defined (__UBOOT__)
#include <common.h>
#include <malloc.h>
#include <linux/stddef.h>
#include <linux/mtd/compat.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/mtd-abi.h>
#include <linux/mtd/partitions.h>
#include "ralink_nand_rt3052.h"
#include "ralink-flash.h"

#define	EIO		 5	/* I/O error */
#define	EINVAL		22	/* Invalid argument */
#define	ENOMEM		12	/* Out of memory */

#define NULL_DEFINED( ... )		do{}while(0)
#define NULL_DEF_RET_1( ... ) 	(1)
#define NULL_DEF_RET_0( ... )	(0)

#define HZ 1
#define schedule_timeout(a) 	udelay(1000000*(a))
#define cond_resched()		NULL_DEF_RET_0()

#else // !defined (__UBOOT__)

#define DEBUG
#include <linux/device.h>
#undef DEBUG
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include "ralink_nand_rt3052.h"
#include "ralink-flash.h"

#endif// !defined (__UBOOT__)

#define READ_STATUS_RETRY	1000

struct mtd_info *ranfc_mtd = NULL;

int skipbbt = 0;
int ranfc_debug = 1;
static int ranfc_bbt = 1;
static int ranfc_verify = 1;

#if !defined (__UBOOT__)
module_param(ranfc_debug, int, 0644);
module_param(ranfc_bbt, int, 0644);
module_param(ranfc_verify, int, 0644);
#endif

#if 0
#define ra_dbg(args...) do { if (ranfc_debug) printk(args); } while(0)
#else
#define ra_dbg(args...)
#endif

#define CLEAR_INT_STATUS() 	ra_outl(NFC_INT_ST, ra_inl(NFC_INT_ST))
#define NFC_TRANS_DONE() 	(ra_inl(NFC_INT_ST) & INT_ST_ND_DONE)


static struct mtd_partition rt2880_partitions[] = {
	{
                name:           "ALL",
                size:           MTDPART_SIZ_FULL,
                offset:         0,
        },
	/* Put your own partition definitions here */
        {
                name:           "Bootloader",
                size:           MTD_BOOT_PART_SIZE,
                offset:         0,
        }, {
                name:           "Config",
                size:           MTD_CONFIG_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
        }, {
                name:           "Factory",
                size:           MTD_FACTORY_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
        }, {
                name:           "Kernel",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS",
                size:           MTD_ROOTFS_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
        }, {
                name:           "Kernel_RootFS",
                size:           MTD_KERN_PART_SIZE + MTD_ROOTFS_PART_SIZE,
                offset:         MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE,
#endif
#else //CONFIG_RT2880_ROOTFS_IN_RAM
        }, {
                name:           "Kernel",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#endif
#ifdef CONFIG_DUAL_IMAGE
        }, {
                name:           "Kernel2",
                size:           MTD_KERN2_PART_SIZE,
                offset:         MTD_KERN2_PART_OFFSET,
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
        }, {
                name:           "RootFS2",
                size:           MTD_ROOTFS2_PART_SIZE,
                offset:         MTD_ROOTFS2_PART_OFFSET,
#endif
#endif
        }
};


/*************************************************************
 * nfc functions 
 *************************************************************/
static int nfc_wait_ready(int snooze_ms);

unsigned int nfc_addr_translate(struct ra_nand_chip *ra, unsigned int addr, unsigned int *column, unsigned int *row)
{
	unsigned int _col, _row;

	_row = (addr >> ra->page_shift);
	_col = addr & ((1<<ra->page_shift) - CONFIG_SUBPAGE_BIT);
	
	if (column) 
		*column = _col;
	if (row) 
		*row = _row;
	
	return  ((_row) << (CFG_COLUMN_ADDR_CYCLE * 8)) | (_col & ((1<<(CFG_COLUMN_ADDR_CYCLE * 8))-1)); 
}

/**
 * reset nand chip
 */
static int nfc_chip_reset(void)
{
	int status;

	// reset nand flash
	ra_outl(NFC_CMD1, 0xff);
	ra_outl(NFC_ADDR, 0xfffffff);
	ra_outl(NFC_CONF, 0x0141 | (CFG_ADDR_CYCLE << 16));

	status = nfc_wait_ready(0);  //erase wait 5us
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

	ra_dbg("%s: \n", __func__);

	// reset controller
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x02); //clear data buffer
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) & ~0x02); //clear data buffer

	CLEAR_INT_STATUS();

	retry = READ_STATUS_RETRY;
	while ((ra_inl(NFC_INT_ST) & 0x02) != 0x02 && retry--);
	if (retry <= 0) {
		printk("nfc_all_reset: clean buffer fail \n");
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
		
		ndelay(10);
	} while (!(int_st & INT_ST_RX_BUF_RDY) && retry--);

	if (!(int_st & INT_ST_RX_BUF_RDY)) {
		printk("nfc_read_status: NFC fail, int_st(%x), retry:%x. nfc:%x, reset nfc and flash. \n", 
		       int_st, retry, nfc_st);
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

	ret = _nfc_read_status(&result);
	//fixme, if ret < 0

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


/**
 * generic function to get data from flash.
 * @return data length reading from flash.
 */
static int _ra_nand_pull_data(char *buf, int len, int use_gdma)
{
	char *p = buf;
	int retry;

	// receive data by use_gdma 
	if (use_gdma) { 
		if (_ra_nand_dma_pull((unsigned long)p, len)) {
			printk("%s: fail \n", __func__);
			len = -1; //return error
		}

		return len;
	}

	//fixme: retry count size?
	retry = READ_STATUS_RETRY;
	// no gdma
	while (len > 0) {
		int int_st = ra_inl(NFC_INT_ST);
		if (int_st & INT_ST_RX_BUF_RDY) {
			unsigned int ret_data;
			int ret_size;

#if 1 //!fixme, need optimal by endian
			ret_data = ra_inl(NFC_DATA);
			ra_outl(NFC_INT_ST, INT_ST_RX_BUF_RDY); 
				
			ret_size = sizeof(unsigned int);
			ret_size = min(ret_size, len);
			len -= ret_size;
			while (ret_size-- > 0) {
				//nfc is little endian 
				*p++ = ret_data & 0x0ff;
				ret_data >>= 8; 
			}
#else
			//optimaize
#endif
		}
		else if (int_st & INT_ST_ND_DONE) {
			printk("!! done\n");
			break;
		}
		else {
			ndelay(2);
			if (retry-- < 0) 
				break;
		}
	}

	return (p-buf);
}

/**
 * generic function to put data into flash.
 * @return data length writing into flash.
 */
static int _ra_nand_push_data(char *buf, int len, int use_gdma)
{
	char *p = buf;
	int retry;

	// receive data by use_gdma 
	if (use_gdma) { 
		if (_ra_nand_dma_push((unsigned long)p, len))
			len = 0;		
		return len;
	}


	// no gdma
	retry = 528;
	while(len > 0) {
		int int_st = ra_inl(NFC_INT_ST);
		if (int_st & INT_ST_TX_BUF_RDY) {
			unsigned int tx_data = 0;
			int tx_size, iter = 0;

#if 1 //fixme, need optimaize in words	
			tx_size = min(len, (int)sizeof(unsigned long));
			for (iter = 0; iter < tx_size; iter++) {
				tx_data |= (*p++ << (8*iter));
			}
			ra_outl(NFC_INT_ST, INT_ST_TX_BUF_RDY);
			ra_outl(NFC_DATA, tx_data);

			len -= tx_size;
#else
			//optimaize
#endif
		}
		
		if (int_st & 0x01) {
			break;
		}
		else {
			ndelay(2);
			if (retry-- < 0) {
				ra_dbg("%s p:%p buf:%p \n", __func__, p, buf);
				break;
			}
		}
	}

	
	return (int)(p-buf);

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

#if !defined (CONFIG_NOT_SUPPORT_RB)
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

static inline int _nfc_write_raw_data(int cmd1, int cmd3, int bus_addr, int conf, char *buf, int len, int flags)
{
	int ret;

	CLEAR_INT_STATUS();
	ra_outl(NFC_CMD1, cmd1); 	
	ra_outl(NFC_CMD3, cmd3); 	
	ra_outl(NFC_ADDR, bus_addr);
	ra_outl(NFC_CONF, conf); 

	ret = _ra_nand_push_data(buf, len, flags & FLAG_USE_GDMA);
	if (ret != len) {
		ra_dbg("%s: ret:%x (%x) \n", __func__, ret, len);
		return NAND_STATUS_FAIL;
	}

	ret = nfc_wait_ready(1); //write wait 1ms
	if (ret & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
		return NAND_STATUS_FAIL;
	}

	return 0;
}


static inline int _nfc_read_raw_data(int cmd1, int bus_addr, int conf, char *buf, int len, int flags)
{
	int ret;

	CLEAR_INT_STATUS();
	ra_outl(NFC_CMD1, cmd1); 	
	ra_outl(NFC_ADDR, bus_addr);
	ra_outl(NFC_CONF, conf); 

	ret = _ra_nand_pull_data(buf, len, flags & FLAG_USE_GDMA);
	if (ret != len) {
		ra_dbg("%s: ret:%x (%x) \n", __func__, ret, len);
		return NAND_STATUS_FAIL;
	}

	//FIXME, this section is not necessary
	ret = nfc_wait_ready(0); //wait ready 
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
	unsigned int cmd1 = 0, conf = 0;
	unsigned int bus_addr = 0;
	unsigned int ecc_en;
	int use_gdma;
	int status;

	int pages_perblock = 1<<(ra->erase_shift - ra->page_shift);
	// constrain of nfc read function 

#if defined (WORKAROUND_RX_BUF_OV)
	BUG_ON (len > 60); 	//problem of rx-buffer overrun 
#endif
	BUG_ON (offs >> ra->oob_shift); //page boundry
	BUG_ON ((unsigned int)(((offs + len) >> ra->oob_shift) + page) >
		((page + pages_perblock) & ~(pages_perblock-1))); //block boundry

	use_gdma = flags & FLAG_USE_GDMA;
	ecc_en = flags & FLAG_ECC_EN;
	bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)) | (offs & ((1<<CFG_COLUMN_ADDR_CYCLE*8) - 1));
	
	cmd1 = 0x50;
	conf = 0x000141| ((CFG_ADDR_CYCLE)<<16) | ((len) << 20); 
	if (ecc_en) 
		conf |= (1<<3); 
	if (use_gdma)
		conf |= (1<<2);

	ra_dbg("%s: cmd1: %x, bus_addr: %x, conf: %x, len:%x, flag:%x\n", 
	       __func__, cmd1, bus_addr, conf, len, flags);

	status = _nfc_read_raw_data(cmd1, bus_addr, conf, buf, len, flags);
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
	unsigned int bus_addr = 0;
	int use_gdma;
	int status;

	int pages_perblock = 1<<(ra->erase_shift - ra->page_shift);
	// constrain of nfc read function 

	BUG_ON (offs >> ra->oob_shift); //page boundry
	BUG_ON ((unsigned int)(((offs + len) >> ra->oob_shift) + page) >
		((page + pages_perblock) & ~(pages_perblock-1))); //block boundry 

	use_gdma = flags & FLAG_USE_GDMA;
	bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)) | (offs & ((1<<CFG_COLUMN_ADDR_CYCLE*8) - 1));
	
	cmd1 = 0x08050;
	cmd3 = 0x10;
	conf = 0x001223 | ((CFG_ADDR_CYCLE)<<16) | ((len) << 20); 
	if (use_gdma)
		conf |= (1<<2);

	// set NFC
	ra_dbg("%s: cmd1: %x, cmd3: %x bus_addr: %x, conf: %x, len:%x\n", 
	       __func__, cmd1, cmd3, bus_addr, conf, len);

	status = _nfc_write_raw_data(cmd1, cmd3, bus_addr, conf, buf, len, flags);
	if (status & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
		return -EIO;
	}

	return 0; 
}


int nfc_read_page(struct ra_nand_chip *ra, char *buf, int page, int flags);
int nfc_write_page(struct ra_nand_chip *ra, char *buf, int page, int flags);
	

#if !defined (WORKAROUND_RX_BUF_OV)	
int nfc_ecc_verify(struct ra_nand_chip *ra, char *buf, int page, int mode)
{
	int ret, i;
	char *p, *e;
	int ecc;
	
//	printk("%s, page:%x mode:%d\n", __func__, page, mode);

	if (mode == FL_WRITING) {
		int len = (1<<ra->page_shift) + (1<<ra->oob_shift);
		int conf = 0x000141| ((CFG_ADDR_CYCLE)<<16) | (len << 20); 
		conf |= (1<<3); //(ecc_en) 
		conf |= (1<<2); // (use_gdma)

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
	p += (1<<ra->page_shift);
	ecc = ra_inl(NFC_ECC); 
	if (ecc == 0) //clean page.
		return 0;
	e = (char*)&ecc;
	for (i=0; i<CONFIG_ECC_BYTES; i++) {
		int eccpos = CONFIG_ECC_OFFSET + i;
		if (*(p + eccpos) != *(e + i)) {
			printk("%s mode:%s, invalid ecc, page: %x read:%x %x %x, ecc:%x \n",
			       __func__, (mode == FL_READING)?"read":"write", page,	
			       *(p+ CONFIG_ECC_OFFSET), *(p+ CONFIG_ECC_OFFSET+1), *(p+ CONFIG_ECC_OFFSET +2), ecc);
			return -1;
		}
	}
	return 0;

bad_block:
	return -1;
}

#else

void ranfc_dump(void) 
{	
	int i;
	for (i=0; i<11; i++) {
		if (i==6) 
			continue;
		printk("%x: %x \n", NFC_BASE + i*4, ra_inl(NFC_BASE + i*4));
	}
}

/**
 * @return 0, ecc OK or corrected.
 * @return NAND_STATUS_FAIL, ecc fail.   
 */

int nfc_ecc_verify(struct ra_nand_chip *ra, char *buf, int page, int mode)
{
	int ret, i;
	char *p, *e;
	int ecc;
	
	if (ranfc_verify == 0)
		return 0;

	ra_dbg("%s, page:%x mode:%d\n", __func__, page, mode);

	if (mode == FL_WRITING) { // read back and memcmp
		ret = nfc_read_page(ra, ra->readback_buffers, page, FLAG_USE_GDMA); 
		if (ret != 0) //double comfirm
			ret = nfc_read_page(ra, ra->readback_buffers, page, FLAG_USE_GDMA); 

		if (ret != 0) {
			printk("%s: mode:%x read back fail \n", __func__, mode);
			return -1;
		}
		return memcmp(buf, ra->readback_buffers, 1<<ra->page_shift);
	}
	
	if (mode == FL_READING) { 
#if 0
		if (ra->sandbox_page == 0)
			return 0;

		ret = nfc_write_page(ra, buf, ra->sandbox_page, FLAG_USE_GDMA | FLAG_ECC_EN);
		if (ret != 0) {
			printk("%s, fail write sandbox_page \n", __func__);
			return -1;
		}
#else
		/** @note: 
		 * The following command is actually not 'write' command to drive NFC to write flash.
		 * However, it can make NFC to calculate ECC, that will be used to compare with original ones.
		 * --YT
		 */
		unsigned int conf = 0x001223| (CFG_ADDR_CYCLE<<16) | (0x200 << 20) | (1<<3) | (1<<2); 
		_nfc_write_raw_data(0xff, 0xff, ra->sandbox_page<<ra->page_shift, conf, buf, 0x200, FLAG_USE_GDMA);
#endif

		ecc = ra_inl(NFC_ECC); 
		if (ecc == 0) //clean page.
			return 0;
		e = (char*)&ecc;
		p = buf + (1<<ra->page_shift);
		for (i=0; i<CONFIG_ECC_BYTES; i++) {
			int eccpos = CONFIG_ECC_OFFSET + i;
			if (*(p + eccpos) != *(e + i)) {
				printk("%s mode:%s, invalid ecc, page: %x read:%x %x %x, write:%x \n",
				       __func__, (mode == FL_READING)?"read":"write", page,	
				       *(p+ CONFIG_ECC_OFFSET), *(p+ CONFIG_ECC_OFFSET+1), *(p+ CONFIG_ECC_OFFSET +2), ecc);

				for (i=0; i<528; i++)
					printk("%-2x \n", *(buf + i));
				return -1;
			}
		}
		return 0;
	}

	return -1;

}

#endif


/**
 * @return -EIO, writing size is less than a page 
 * @return 0, OK
 */
int nfc_read_page(struct ra_nand_chip *ra, char *buf, int page, int flags)
{
	unsigned int cmd1 = 0, conf = 0;
	unsigned int bus_addr = 0;
	unsigned int ecc_en;
	int use_gdma;
	int pagesize, size, offs;
	int status = 0;

	use_gdma = flags & FLAG_USE_GDMA;
	ecc_en = flags & FLAG_ECC_EN;

	page = page & ((1<<ra->chip_shift)-1); // chip boundary
	pagesize = (1 << ra->page_shift);
	size = pagesize + (1<<ra->oob_shift); //add oobsize
	offs = 0;

	while (size > 0) {
		int len;
#if defined (WORKAROUND_RX_BUF_OV)
		len = min(60, size);
#else
		len = size;
#endif		
		bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)) | (offs & ((1<<CFG_COLUMN_ADDR_CYCLE*8)-1)); 
		if (unlikely((offs & ~((1<<ra->page_shift)-1))))
			cmd1 = 0x50;
		else if (offs & ~((1<<CFG_COLUMN_ADDR_CYCLE*8)-1))
			cmd1 = 0x01;
		else
			cmd1 = 0;

		conf = 0x000141| ((CFG_ADDR_CYCLE)<<16) | (len << 20); 
#if !defined (WORKAROUND_RX_BUF_OV)
		if (ecc_en) 
			conf |= (1<<3); 
#endif
		if (use_gdma)
			conf |= (1<<2);

		status = _nfc_read_raw_data(cmd1, bus_addr, conf, buf+offs, len, flags);
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
	unsigned int bus_addr = 0;
	unsigned int ecc_en;
	int use_gdma;
	int pagesize;
	char status;
	uint8_t *oob = buf + (1<<ra->page_shift);

	use_gdma = flags & FLAG_USE_GDMA;
	ecc_en = flags & FLAG_ECC_EN;

	oob[ra->badblockpos] = 0xff;	//tag as good block.
	ra->buffers_page = -1; //cached

	page = page & ((1<<ra->chip_shift)-1); // chip boundary
	pagesize = (1 << ra->page_shift);
	pagesize = pagesize + (1<<ra->oob_shift);
	bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)); //write_page always write from offset 0.
	
	cmd1 = 0x8000;
	cmd3 = 0x10;
	conf = 0x001223| ((CFG_ADDR_CYCLE)<<16) | (pagesize << 20); 
	if (ecc_en) 
		conf |= (1<<3); 
	if (use_gdma)
		conf |= (1<<2);

	// set NFC
	ra_dbg("nfc_write_page: cmd1: %x, cmd3: %x bus_addr: %x, conf: %x, len:%x\n", 
	       cmd1, cmd3, bus_addr, conf, pagesize);

	status = _nfc_write_raw_data(cmd1, cmd3, bus_addr, conf, buf, pagesize, flags);
	if (status & NAND_STATUS_FAIL) {
		printk("%s: fail \n", __func__);
		return -EIO;
	}
	

	if (flags & FLAG_VERIFY) { // verify and correct ecc
		status = nfc_ecc_verify(ra, buf, page, FL_WRITING);
		if (status != 0) {
			printk("%s: ecc_verify fail: ret:%x \n", __func__, status);
			oob[ra->badblockpos] = 0x33;
			page -= page % (CFG_BLOCKSIZE/pagesize);
			printk("create a bad block at page %x\n", page);
			status = nfc_write_oob(ra, page, ra->badblockpos, oob+ra->badblockpos, 1, flags);
			if (status == 0)
				printk("bad block acknowledged, please write again\n");
			else
				printk("failed to create a bad block\n");
			return -EBADMSG;
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

#if !defined (__UBOOT__)
	mutex_unlock(ra->controller);
#endif ///
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

#if !defined (__UBOOT__)
	ret = mutex_lock_interruptible(ra->controller);
#endif ///
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
	else 
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
	tag =BBT_TAG_BAD;
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


#if defined (WORKAROUND_RX_BUF_OV)
/**
 * to find a bad block for ecc verify of read_page
 */
unsigned int nand_bbt_find_sandbox(struct ra_nand_chip *ra)
{
	loff_t offs = 0;
	int chipsize = 1 << ra->chip_shift;
	int blocksize = 1 << ra->erase_shift;

	
	while (offs < chipsize) {
		if (nand_block_checkbad(ra, offs)) //scan and verify the unknown tag
			break;
		offs += blocksize;
	}

	if (offs >= chipsize) {
		offs = chipsize - blocksize;
	}

	nand_bbt_set(ra, (unsigned int)offs>>ra->erase_shift, BBT_TAG_RES);	 // tag bbt only, instead of update badblockpos of flash.
	return (offs >> ra->page_shift);
}
#endif



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

#define BLOCK_ALIGNED(a) ((a) & (blocksize - 1))

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
#if !defined  (__UBOOT__)
	/* Do call back function */
	if (!ret)
		mtd_erase_callback(instr);
#endif

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
//	uint8_t *buf = ra->buffers + (1<<ra->page_shift);
	int retsize = 0;

	ra_dbg("%s: size:%x, mode:%x, offs:%x  \n", __func__, size, mode, ooboffs);

	switch(mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_RAW:
		if (ooboffs > oobsize)
			return -1;

#if 0		//* clear buffer */
		if (ooboffs || ooboffs+size < oobsize) 
			memset (ra->buffers + oobsize, 0x0ff, 1<<ra->oob_shift);
#endif

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
		if (ops->mode == MTD_OOB_AUTO) {
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
		if (ops->mode == MTD_OOB_AUTO) {
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
		
		ret = nfc_write_page(ra, ra->buffers, page, FLAG_USE_GDMA | FLAG_VERIFY |
				     ((ops->mode == MTD_OOB_RAW || ops->mode == MTD_OOB_PLACE) ? 0 : ecc_en ));
		if (ret) {
			nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_BAD);
			return ret;
		}

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

		ret = nfc_read_page(ra, ra->buffers, page, FLAG_USE_GDMA | FLAG_VERIFY | 
				    ((ops->mode == MTD_OOB_RAW || ops->mode == MTD_OOB_PLACE) ? 0: FLAG_ECC_EN ));
		//FIXME, something strange here, some page needs 2 more tries to guarantee read success.
		if (ret) {
			printk("read again:\n");
			ret = nfc_read_page(ra, ra->buffers, page, FLAG_USE_GDMA | FLAG_VERIFY | 
					    ((ops->mode == MTD_OOB_RAW || ops->mode == MTD_OOB_PLACE) ? 0: FLAG_ECC_EN ));

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

	ra_dbg("%s: \n", __func__);

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

	ra_dbg("%s: \n", __func__);

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
	ops.mode =  MTD_OOB_AUTO;

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

	ra_dbg("%s: mtd:%p from:%x, len:%x, buf:%p \n", __func__, mtd, (unsigned int)from, len, buf);

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
	ops.mode = MTD_OOB_AUTO;

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

	ra_dbg("%s: \n", __func__);

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

	ra_dbg("%s: \n", __func__);

	nand_get_device(ra, FL_READING);

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
	ra_dbg("%s: \n", __func__);

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


/************************************************************
 * the init/exit section.
 */

static struct nand_ecclayout ra_oob_layout = {
	.eccbytes = CONFIG_ECC_BYTES,
	.eccpos = {CONFIG_ECC_OFFSET, CONFIG_ECC_OFFSET+1, CONFIG_ECC_OFFSET+2},
	.oobfree = {
		 {.offset = 0, .length = 4},
		 {.offset = 8, .length = 8},
		 {.offset = 0, .length = 0}
	 },
#define RA_CHIP_OOB_AVAIL (4+8)
	.oobavail = RA_CHIP_OOB_AVAIL,
	// 5th byte is bad-block flag.
};

static int __init ra_nand_init(void) 
{
	struct ra_nand_chip *ra;
	int alloc_size, bbt_size, buffers_size;
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
	loff_t offs;
	struct __image_header {
		uint8_t unused[60];
		uint32_t ih_ksz;
	} hdr;
#endif
	
	if(ra_check_flash_type()!=BOOT_FROM_NAND) { /* NAND */
		return 0;
	}

#define ALIGNE_16(a) (((unsigned long)(a)+15) & ~15)
	buffers_size = ALIGNE_16((1<<CONFIG_PAGE_SIZE_BIT) + (1<<CONFIG_OOBSIZE_PER_PAGE_BIT)); //ra->buffers
	bbt_size = BBTTAG_BITS * (1<<(CONFIG_CHIP_SIZE_BIT - (CONFIG_PAGE_SIZE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT))) / 8; //ra->bbt
	bbt_size = ALIGNE_16(bbt_size);

	alloc_size = buffers_size + bbt_size;
	alloc_size += buffers_size; //for ra->readback_buffers
	alloc_size += sizeof(*ra); 
	alloc_size += sizeof(*ranfc_mtd);

	//make sure gpio-0 is input
	ra_outl(RALINK_PIO_BASE+0x24, ra_inl(RALINK_PIO_BASE+0x24) & ~0x01);

#if !defined (__UBOOT__)
	ra = (struct ra_nand_chip *)kzalloc(alloc_size, GFP_KERNEL | GFP_DMA);
#else
	ra = (struct ra_nand_chip *)malloc(alloc_size);
#endif
	if (!ra) {
		printk("%s: mem alloc fail \n", __func__);
		return -ENOMEM;
	}
	memset(ra, 0, alloc_size);

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
	ra->oob			= &ra_oob_layout;
	ra->buffers_page	= -1;

#if defined (WORKAROUND_RX_BUF_OV)
	if (ranfc_verify) {
		ra->sandbox_page = nand_bbt_find_sandbox(ra);
	}
#endif
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x01); //set wp to high
	nfc_all_reset();

	ranfc_mtd->type		= MTD_NANDFLASH;
	ranfc_mtd->flags	= MTD_CAP_NANDFLASH;
	ranfc_mtd->size		= CONFIG_NUMCHIPS * CFG_CHIPSIZE;
	ranfc_mtd->erasesize	= CFG_BLOCKSIZE;
	ranfc_mtd->writesize	= CFG_PAGESIZE;
	ranfc_mtd->oobsize 	= CFG_PAGE_OOBSIZE;
	ranfc_mtd->oobavail	= RA_CHIP_OOB_AVAIL;
	ranfc_mtd->name		= "ra_nfc";
	//ranfc_mtd->index
	ranfc_mtd->ecclayout	= &ra_oob_layout;
	//ranfc_mtd->numberaseregions
	//ranfc_mtd->eraseregions
	//ranfc_mtd->bansize
	ranfc_mtd->erase 	= ramtd_nand_erase;
	//ranfc_mtd->point
	//ranfc_mtd->unpoint
	ranfc_mtd->read		= ramtd_nand_read;
	ranfc_mtd->write	= ramtd_nand_write;
	ranfc_mtd->read_oob	= ramtd_nand_readoob;
	ranfc_mtd->write_oob	= ramtd_nand_writeoob;
	//ranfc_mtd->get_fact_prot_info; ranfc_mtd->read_fact_prot_reg; 
	//ranfc_mtd->get_user_prot_info; ranfc_mtd->read_user_prot_reg;
	//ranfc_mtd->write_user_prot_reg; ranfc_mtd->lock_user_prot_reg;
	//ranfc_mtd->writev; ranfc_mtd->sync; ranfc_mtd->lock; ranfc_mtd->unlock; ranfc_mtd->suspend; ranfc_mtd->resume;
	ranfc_mtd->block_isbad		= ramtd_nand_block_isbad;
	ranfc_mtd->block_markbad	= ramtd_nand_block_markbad;
	//ranfc_mtd->reboot_notifier
	//ranfc_mtd->ecc_stats;
	// subpage_sht;

	//ranfc_mtd->get_device; ranfc_mtd->put_device
	ranfc_mtd->priv = ra;

#if !defined (__UBOOT__)
	ranfc_mtd->owner = THIS_MODULE;
	ra->controller = &ra->hwcontrol;
	mutex_init(ra->controller);

	printk("%s: alloc %x, at %p , btt(%p, %x), ranfc_mtd:%p\n", 
	       __func__ , alloc_size, ra, ra->bbt, bbt_size, ranfc_mtd);

#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	offs = MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE;
	ramtd_nand_read(ranfc_mtd, offs, sizeof(hdr), (size_t *)&alloc_size, (u_char *)(&hdr));
	//warning: reuse alloc_size for dummy returning length
	if (hdr.ih_ksz != 0) {
		rt2880_partitions[4].size = ntohl(hdr.ih_ksz);
		rt2880_partitions[5].size = IMAGE1_SIZE - (MTD_BOOT_PART_SIZE +
				MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE +
				ntohl(hdr.ih_ksz));
	}
#endif
	/* Register the partitions */
	return mtd_device_register(ranfc_mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
#else
	return ranfc_mtd;
#endif
}

static void __exit ra_nand_remove(void)
{
	struct ra_nand_chip *ra;

	if (ranfc_mtd) {
		ra = (struct ra_nand_chip  *)ranfc_mtd->priv;
#if !defined (__UBOOT__)
		mtd_device_unregister(ranfc_mtd);
		if (ra)
			kfree(ra);
#else
		if (ra)
			free(ra);
#endif
	}
}

#if !defined (__UBOOT__)
module_init(ra_nand_init);
module_exit(ra_nand_remove);

MODULE_LICENSE("GPL");
#endif
