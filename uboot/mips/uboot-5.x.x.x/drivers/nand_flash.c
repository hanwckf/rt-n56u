#include <common.h>
#include <command.h>
#include <malloc.h>
#include <configs/rt2880.h>
#include "ralink_nand.h"

//#define MTK_NAND_BMT


#ifdef MTK_NAND_BMT
#define __UBOOT_NAND__
#include "bmt.h"
// BMT can not apply on Uboot, because Rom does not support BMT,
// if Page size is 2048, there are 2 blocks for Uboot, if page size is 512, there are 3 blocks for Uboot
#define BMT_APPLY_START_OFFSET (is_nand_page_2048? (2*CFG_BLOCKSIZE):(3*CFG_BLOCKSIZE))
#define BMT_POOL_SIZE 80
static bmt_struct *g_bmt;

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
#define BLOCK_ALIGNED(a) ((a) & (CFG_BLOCKSIZE - 1))

int nand_addrlen = 3;
int is_nand_page_2048 = 0;

const unsigned int nand_size_map[2][3] = {{25, 30, 30}, {20, 27, 30}};

static int nfc_wait_ready(int snooze_ms);

#ifdef MTK_NAND_BMT
int ranand_erase_bmt(unsigned int offs, int len);
int ranand_write_bmt(char *buf, unsigned int to, int datalen);
int ranand_read_bmt(char *buf, unsigned int from, int datalen);
int ranand_erase_write_bmt(char *buf, unsigned int offs, int count);
#endif
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

// 1-bit error detection
static int one_bit_correction(char *ecc1, char *ecc2, int *bytes, int *bits)
{
	char *p, nibble, crumb;
	int i, xor, iecc1 = 0, iecc2 = 0;

	printf("correction : %x %x %x\n", ecc1[0], ecc1[1], ecc1[2]);
	printf("correction : %x %x %x\n", ecc2[0], ecc2[1], ecc2[2]);

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
	printf("xor = %x (%x %x)\n", xor, iecc1, iecc2);

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


unsigned long ranand_init(void)
{
	int reg, chip_mode;
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
	
	//config ECC location
	ra_and(NFC_CONF1, 0xfff000ff);
	ra_or(NFC_CONF1, ((CONFIG_ECC_OFFSET + 2) << 16) +
			((CONFIG_ECC_OFFSET + 1) << 12) +
			(CONFIG_ECC_OFFSET << 8));
	

	//maks sure gpio-0 is input
	ra_outl(RALINK_PIO_BASE+0x24, ra_inl(RALINK_PIO_BASE+0x24) & ~0x01);

	//set WP to high
	ra_or(NFC_CTRL, 0x01);

	if (nfc_all_reset() != 0)
		return -1;

#if defined(MTK_NAND_BMT)
    if (!g_bmt)
    {
        if ( !(g_bmt = init_bmt(BMT_POOL_SIZE)) )
        {
            printf("Error: init bmt failed\n");
            // ASSERT(0);
            return 0;
        }
    }
#endif	

	return CFG_CHIPSIZE;
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
		ra_dbg("%s: ret:%x (%x) \n", __func__, ret, len);
		return NAND_STATUS_FAIL;
	}

	ret = nfc_wait_ready(3); //wait ready
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
		ra_dbg("%s: ret:%x (%x) \n", __func__, ret, len);
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

/**
 * @return !0: fail
 * @return 0: OK
 */
int nfc_read_oob(int page, unsigned int offs, char *buf, int len)
{
	unsigned int cmd1 = 0, cmd2 = 0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int status;

#if 0
	int pages_perblock = 1<<(ra->erase_shift - ra->page_shift);
#if defined (WORKAROUND_RX_BUF_OV)
	BUG_ON (len > 60);      //problem of rx-buffer overrun
#endif
	BUG_ON (offs >> ra->oob_shift); //page boundry
	BUG_ON ((unsigned int)(((offs + len) >> ra->oob_shift) + page) >
			((page + pages_perblock) & ~(pages_perblock-1))); //block boundry
#endif

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

#if 0
	int pages_perblock = 1<<(ra->erase_shift - ra->page_shift);
	// constrain of nfc read function
	BUG_ON (offs >> ra->oob_shift); //page boundry
	BUG_ON ((unsigned int)(((offs + len) >> ra->oob_shift) + page) >
			((page + pages_perblock) & ~(pages_perblock-1))); //block boundry
#endif

	bus_addr = (page << (CFG_COLUMN_ADDR_CYCLE*8)) | (offs & ((1<<CFG_COLUMN_ADDR_CYCLE*8) - 1));

	if (is_nand_page_2048) {
		cmd1 = 0x80;
		cmd3 = 0x10;
		bus_addr += CFG_PAGESIZE;
		bus_addr2 = page >> (CFG_COLUMN_ADDR_CYCLE*8);
		conf = 0x001123 | ((CFG_ADDR_CYCLE)<<16) | ((len) << 20);
	}
	else {
		cmd1 = 0x08050;
		cmd3 = 0x10;
		conf = 0x001223 | ((CFG_ADDR_CYCLE)<<16) | ((len) << 20);
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

int nfc_ecc_verify(char *buf, int page, int mode)
{
	int ret, i;
	char *p, *e;
	int ecc;

	//ra_dbg("%s, page:%x mode:%d\n", __func__, page, mode);

	if (mode == FL_WRITING) {
		int len = CFG_PAGESIZE + CFG_PAGE_OOBSIZE;
		int conf = 0x000141| ((CFG_ADDR_CYCLE)<<16) | (len << 20);
		char rbbuf[CFG_PAGESIZE+CFG_PAGE_OOBSIZE];
		conf |= (1<<3); //(ecc_en)

		p = rbbuf;
		ret = nfc_read_page(p, page);
		if (ret == 0)
			goto ecc_check;

		//FIXME, double comfirm
		printf("%s: read back fail, try again \n",__func__);
		ret = nfc_read_page(p, page);
		if (ret != 0) {
			printf("\t%s: read back fail agian \n",__func__);
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
				printf("skip ecc 0xff at page %x hwecc=%X\n", page,ecc);					
				return 0;
			}
		}
		
		
		for (i=0; i<CONFIG_ECC_BYTES; i++) {
			int eccpos = CONFIG_ECC_OFFSET + i;
			if (*(p + eccpos) != *(e + i)) {
				printf("%s mode:%s, invalid ecc, page: %x read:%x %x %x, ecc:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+ CONFIG_ECC_OFFSET), *(p+ CONFIG_ECC_OFFSET+1), *(p+ CONFIG_ECC_OFFSET +2), ecc);
				return -1;
			}
		}
	}
#if defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) || \
    defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) || \
    defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD)	
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
				printf("skip ecc 0xff at page %x\n", page);
				return 0;
			}
		}
		for (i=0; i<CONFIG_ECC_BYTES; i++) {
			int eccpos = CONFIG_ECC_OFFSET + i;
			if (*(p + eccpos) != *(e + i)) {
				printf("%s mode:%s, invalid ecc, page: %x read:%x %x %x, ecc:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+ CONFIG_ECC_OFFSET), *(p+ CONFIG_ECC_OFFSET+1), *(p+ CONFIG_ECC_OFFSET +2), ecc);
				correction_flag |= 0x1;
			}
			if (*(p + eccpos + qsz) != *(e2 + i)) {
				printf("%s mode:%s, invalid ecc2, page: %x read:%x %x %x, ecc2:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+CONFIG_ECC_OFFSET+qsz), *(p+ CONFIG_ECC_OFFSET+1+qsz), *(p+ CONFIG_ECC_OFFSET+2+qsz), ecc2);
				correction_flag |= 0x2;
			}
			if (*(p + eccpos + qsz*2) != *(e3 + i)) {
				printf("%s mode:%s, invalid ecc3, page: %x read:%x %x %x, ecc3:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+CONFIG_ECC_OFFSET+qsz*2), *(p+ CONFIG_ECC_OFFSET+1+qsz*2), *(p+ CONFIG_ECC_OFFSET+2+qsz*2), ecc3);
				correction_flag |= 0x4;
			}
			if (*(p + eccpos + qsz*3) != *(e4 + i)) {
				printf("%s mode:%s, invalid ecc4, page: %x read:%x %x %x, ecc4:%x \n",
						__func__, (mode == FL_READING)?"read":"write", page,
						*(p+CONFIG_ECC_OFFSET+qsz*3), *(p+ CONFIG_ECC_OFFSET+1+qsz*3), *(p+ CONFIG_ECC_OFFSET+2+qsz*3), ecc4);
				correction_flag |= 0x8;
			}
		}

		if (correction_flag)
		{
			printf("trying to do correction!\n");
			if (correction_flag & 0x1)
			{
				int bytes, bits;
				char *pBuf = p - CFG_PAGESIZE;

				if (one_bit_correction(p + CONFIG_ECC_OFFSET, e, &bytes, &bits) == 0)
				{
					pBuf[bytes] = pBuf[bytes] ^ (1 << bits);
					printf("1. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printf("failed to correct!\n");
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
					printf("2. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printf("failed to correct!\n");
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
					printf("3. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printf("failed to correct!\n");
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
					printf("4. correct byte %d, bit %d!\n", bytes, bits);
				}
				else
				{
					printf("failed to correct!\n");
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
int nfc_read_page(char *buf, int page)
{
	unsigned int cmd1 = 0, cmd2 = 0, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int size, offs;
	int status = 0;

	page = page & (CFG_CHIPSIZE - 1); // chip boundary
	size = CFG_PAGESIZE + CFG_PAGE_OOBSIZE; //add oobsize
	offs = 0;

	while (size > 0) {
		int len;
#if defined (WORKAROUND_RX_BUF_OV)
		len = min(60, size);
#else
		len = size;
#endif		
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
#if !defined (WORKAROUND_RX_BUF_OV)
		conf |= (1<<3); 
#endif

		status = nfc_read_raw_data(cmd1, cmd2, bus_addr, bus_addr2, conf, buf+offs, len);
		if (status & NAND_STATUS_FAIL) {
			printf("%s: fail \n", __func__);
			return -1;
		}

		offs += len;
		size -= len;
	}

	// verify and correct ecc
	status = nfc_ecc_verify(buf, page, FL_READING);	
	if (status != 0) {
		printf("%s: fail, buf:%x, page:%x, \n", __func__, buf, page);
		return -2;
	}

	return 0;
}

/** 
 * @return -EIO, fail to write
 * @return 0, OK
 */
int nfc_write_page(char *buf, int page)
{
	unsigned int cmd1 = 0, cmd3, conf = 0;
	unsigned int bus_addr = 0, bus_addr2 = 0;
	int size;
	char status;


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
	conf |= (1<<3); //enable ecc

#ifdef MTK_NAND_BMT_DEBUG
	if ((bus_addr == 0xe8130000) || (bus_addr == 0xe9910000) || (bus_addr == 0xe9f20000) || (bus_addr == 0xec450000)) 
	{
		printf("hmm... create a bad block %x\n", bus_addr);
		*(buf+CFG_PAGESIZE+CONFIG_BAD_BLOCK_POS) = 0x11;
	}
#endif
#if 0 //winfred testing for bad block
	if (bus_addr == 0x0)
	{
		printf("hmm... create a bad block %x\n", bus_addr);
		*(buf+2048+5) = 0x11;
	}
#endif

	status = nfc_write_raw_data(cmd1, cmd3, bus_addr, bus_addr2, conf, buf, size);
	if (status & NAND_STATUS_FAIL) {
		printf("%s: fail\n", __func__);
		return -1;
	}

	status = nfc_ecc_verify(buf, page, FL_WRITING);
	if (status != 0) {
		printf("%s: ecc_verify fail: ret:%x \n", __func__, status);
		*(buf+CFG_PAGESIZE+CONFIG_BAD_BLOCK_POS) = 0x33;
		page -= page % (CFG_BLOCKSIZE/CFG_PAGESIZE);
		printf("create a bad block at page %x\n", page);
		status = nfc_write_oob(page, CONFIG_BAD_BLOCK_POS, buf+CFG_PAGESIZE+CONFIG_BAD_BLOCK_POS, 1);
		if (status == 0)
			printf("bad block acknowledged, please write again\n");
		else
			printf("failed to create a bad block\n");
		return -2;
	}

	return 0;
}

#if defined(CONFIG_BADBLOCK_CHECK) || defined(MTK_NAND_BMT)
int ranand_block_isbad(loff_t offs)
{
	unsigned int tag;
	int page, ret;

	page = offs >> CONFIG_PAGE_SIZE_BIT;
	ret = nfc_read_oob(page, CONFIG_BAD_BLOCK_POS, (char*)&tag, 1);
	if (ret == 0 && (tag & 0xff) == 0xff)
		return 0;
	return 1;
}
#endif

int ranand_erase(unsigned int offs, int len)
{
	int page, status;
	unsigned int blocksize = CFG_BLOCKSIZE;
	int ret = 0;

#ifdef MTK_NAND_BMT
	if (offs >= BMT_APPLY_START_OFFSET)
		return ranand_erase_bmt(offs, len);
#endif


	ra_dbg("%s: start:%x, len:%x \n", __func__, offs, len);


	if (BLOCK_ALIGNED(offs) || BLOCK_ALIGNED(len)) {
		ra_dbg("%s: erase block not aligned, addr:%x len:%x %x\n", __func__, offs, len, CFG_BLOCKSIZE);
		return -1;
	}

	while (len) {
		page = (int)(offs >> CONFIG_PAGE_SIZE_BIT);

		/* select device and check wp */
		if (nfc_check_wp()) {
			printf("%s: nand is write protected\n", __func__);
			return -1;
		}

#ifdef CONFIG_BADBLOCK_CHECK
		/* if we have a bad block, we do not erase bad blocks */
		if (ranand_block_isbad(offs)) {
			printf("%s: attempt to erase a bad block at 0x%08x\n", __func__, offs);
			ret++;
			offs += blocksize;
			continue;
		}
#endif

		status = nfc_erase_block(page);

		/* See if block erase succeeded */
		if (status) {
			printf("%s: failed erase, page 0x%08x\n", __func__, page);
			return -1;
		}

		/* Increment page address and decrement length */
		len -= blocksize;
		offs += blocksize;
	}

	return ret;
}

int ranand_write(char *buf, unsigned int to, int datalen)
{
	int page, i = 0;
	size_t retlen = 0;
	int pagemask = (CFG_PAGESIZE -1);
	loff_t addr = to;
	char buffers[CFG_PAGESIZE + CFG_PAGE_OOBSIZE];

#ifdef MTK_NAND_BMT
	if (to >= BMT_APPLY_START_OFFSET)
		return ranand_write_bmt(buf, to, datalen);
#endif


#if 0
	ops->retlen = 0;
	ops->oobretlen = 0;

	/* Invalidate the page cache, when we write to the cached page */
	ra->buffers_page = -1;
#endif

	if (buf == 0)
		datalen = 0;
	
#if 0
	// oob sequential (burst) write
	if (datalen == 0 && ooblen) {
		int len = ((ooblen + ops->ooboffs) + (ra->oob->oobavail - 1)) / ra->oob->oobavail * oobsize;

		//FIXME, need sanity check of block boundary
		page = (int)((to & ((1<<ra->chip_shift)-1)) >> ra->page_shift); //chip boundary
		memset(ra->buffers, 0x0ff, CFG_PAGESIZE);
		//fixme, should we reserve the original content?
		if (ops->mode == MTD_OOB_AUTO) {
			nfc_read_oob(ra, page, 0, ra->buffers, len, FLAG_USE_GDMA );
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
		// ranfc can not support write_data_burst, since hw-rcr and fifo constraints..
	}
#endif

	// page write
	while (datalen) {
		int len;
		int ret;
		int offs;

		page = (int)((addr & (CFG_CHIPSIZE-1)) >> CONFIG_PAGE_SIZE_BIT); //chip boundary
		
		ra_dbg("%s (%d): addr:%x, pg:%x, data:%p, datalen:%x\n", 
				__func__, i++, (unsigned int)addr, page, buf, datalen);

		/* select chip, and check if it is write protected */
		if (nfc_check_wp()) {
			printf("%s: nand is write protected\n", __func__);
			return -1;
		}

		memset(buffers, 0x0ff, sizeof(buffers));

#if 0
		// oob write
		if (ops->mode == MTD_OOB_AUTO) {
			nfc_read_oob(ra, page, 0, ra->buffers + CFG_PAGESIZE, oobsize, FLAG_NONE);
		}
		if (oob && ooblen > 0) {
			len = nand_write_oob_buf(ra, ra->buffers + CFG_PAGESIZE, oob, ooblen, ops->mode, ops->ooboffs);
			if (len < 0) 
				return -EINVAL;
			
			oob += len;
			ops->oobretlen += len;
			ooblen -= len;
		}
#endif

		// data write
		offs = addr & pagemask;
		len = min(datalen, CFG_PAGESIZE - offs);
		if (buf && len > 0) {
			memcpy(buffers + offs, buf, len);	// we can not sure ops->buf wether is DMA-able.

			buf += len;
			datalen -= len;
			retlen += len;
		}
		
		ret = nfc_write_page(buffers, page);
		if (ret) {
/*                        nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_BAD);*/
			return -1;
		}

		addr = (page+1) << CONFIG_PAGE_SIZE_BIT;
	}
	return retlen;
}

int ranand_read(char *buf, unsigned int from, int datalen)
{
	int page, i = 0;
	size_t retlen = 0;
	int pagemask = (CFG_PAGESIZE -1);
	loff_t addr = from;
	char buffers[CFG_PAGESIZE + CFG_PAGE_OOBSIZE];

#ifdef MTK_NAND_BMT
	if (from >= BMT_APPLY_START_OFFSET)
		return ranand_read_bmt(buf, from, datalen);
#endif

	if (buf == 0)
		return 0;

/*        while (datalen || ooblen) {*/
	while (datalen) {
		int len;
		int ret;
		int offs;

		ra_dbg("%s (%d): addr:%x, datalen:%x\n", 
				__func__, i++, (unsigned int)addr, datalen);

		page = (int)((addr & (CFG_CHIPSIZE-1)) >> CONFIG_PAGE_SIZE_BIT); 

#ifdef CONFIG_BADBLOCK_CHECK
		/* if we have a bad block, read from next block instead */
		if (!is_nand_page_2048 && ranand_block_isbad(addr)) {
			printf("%s: skip reading a bad block %x ->", __func__, (unsigned int)addr);
			addr += (1 << (CONFIG_PAGE_SIZE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT));;
			printf(" %x\n", (unsigned int)addr);
			continue;
		}
#endif

		if (datalen > (CFG_PAGESIZE+CFG_PAGE_OOBSIZE) && (page & 0x1f) == 0)
			printf(".");
		ret = nfc_read_page(buffers, page);
		//FIXME, something strange here, some page needs 2 more tries to guarantee read success.
		if (ret) {
			printf("read again:\n");
			ret = nfc_read_page(buffers, page);

			if (ret) {
				printf("read again fail \n");
#if 0
				nand_bbt_set(ra, addr >> (CONFIG_PAGE_SIZE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT), BBT_TAG_BAD);
				if ((ret != -EUCLEAN) && (ret != -EBADMSG)) {
					return ret;
				}
				else {
					/* ecc verification fail, but data need to be returned. */
				}
#endif
			}
			else {
				printf(" read agian susccess \n");
			}
		}
#ifdef CONFIG_BADBLOCK_CHECK
		/* if is_nand_page_2048, check after reading page */
		if (is_nand_page_2048) {
			if (buffers[CFG_PAGESIZE+CONFIG_BAD_BLOCK_POS] != (char)0xff) {
				printf("%s: skip reading a bad block %x ->", __func__, (unsigned int)addr);
				addr += (1 << (CONFIG_PAGE_SIZE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT));;
				printf(" %x\n", (unsigned int)addr);
				continue;
			}
		}
#endif

		// oob read
#if 0
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
#endif

		// data read
		offs = addr & pagemask;
		len = min(datalen, CFG_PAGESIZE - offs);
		if (buf && len > 0) {
			memcpy(buf, buffers + offs, len); // we can not sure ops->buf wether is DMA-able.

			buf += len;
			datalen -= len;
			retlen += len;
			if (ret)
				return -1;
		}

/*                nand_bbt_set(ra, addr >> ra->erase_shift, BBT_TAG_GOOD);*/
		// address go further to next page, instead of increasing of length of write. This avoids some special cases wrong.
		addr = (page+1) << CONFIG_PAGE_SIZE_BIT;
	}
	if (datalen > (CFG_PAGESIZE+CFG_PAGE_OOBSIZE))
		printf("\n");
	return retlen;
}

int ranand_erase_write(char *buf, unsigned int offs, int count)
{
	int blocksize = CFG_BLOCKSIZE;
	int blockmask = blocksize - 1;
	int rc;
#ifdef RALINK_NAND_UPGRADE_CHECK
	int i = 0;
	char *temp;
#endif

#ifdef MTK_NAND_BMT
	if (offs >= BMT_APPLY_START_OFFSET)
		return ranand_erase_write_bmt(buf, offs, count);
#endif

	printf("%s: offs:%x, count:%x\n", __func__, offs, count);

	if (count > (CFG_CHIPSIZE - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))) {
		printf("Abort: image size larger than %d!\n\n", CFG_CHIPSIZE -
					(CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		udelay(10*1000*1000);
		return -1;
	}

	while (count > 0) {
#define BLOCK_ALIGNE(a) (((a) & blockmask))
#ifdef CONFIG_BADBLOCK_CHECK
		if (BLOCK_ALIGNE(offs)) {
			printf("%s: offs %x is not aligned\n", __func__, offs);
			return -1;
		}
		if (count < blocksize)
#else
		if (BLOCK_ALIGNE(offs) || (count < blocksize))
#endif
		{
			char *block;
			unsigned int piece, blockaddr;
			int piece_size;

			block = malloc(blocksize);
			if (!block) {
				printf("%s: malloc block failed\n", __func__);
				return -1;
			}

			blockaddr = offs & ~blockmask;
			if (ranand_read(block, blockaddr, blocksize) != blocksize) {
				free(block);
				return -2;
			}

			piece = offs & blockmask;
			piece_size = min(count, blocksize - piece);
			memcpy(block + piece, buf, piece_size);

try_next_0:
			rc = ranand_erase(blockaddr, blocksize);
#ifdef CONFIG_BADBLOCK_CHECK
			if (rc >= 1) {
				printf("bad block: %x, try next: ", blockaddr);
				blockaddr += (rc * blocksize);
				printf("%x\n", blockaddr);
				//goto try_next_0;
			}
			else
#endif
			if (rc != 0) {
				free(block);
				return -3;
			}
#ifdef CONFIG_BADBLOCK_CHECK
			if (ranand_write(block, blockaddr, blocksize) != blocksize) {
				printf("bad block: %x, try next: ", blockaddr);
				blockaddr += blocksize;
				printf("%x\n", blockaddr);
				goto try_next_0;
			}
#else
			if (ranand_write(block, blockaddr, blocksize) != blocksize) {
				free(block);
				return -4;
			}
#endif
#ifdef RALINK_NAND_UPGRADE_CHECK
			temp = malloc(blocksize);
			if (!temp) {
				printf("%s: malloc temp failed\n", __func__);
				return -1;
			}

			if (ranand_read(temp, blockaddr, blocksize) != blocksize) {
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
                        free(temp);
#endif
			free(block);

			buf += piece_size;
			offs += piece_size;
			count -= piece_size;
		}
		else {
			unsigned int aligned_size = blocksize;

try_next_1:
			rc = ranand_erase(offs, aligned_size);
#ifdef CONFIG_BADBLOCK_CHECK
			if (rc >= 1) {
				printf("bad block: %x, try next: ", offs);
				offs += (rc * blocksize);
				printf("%x\n", offs);
				//goto try_next_1;
			}
			else
#endif
			if (rc != 0)
			{
				return -1;
			}
#ifdef CONFIG_BADBLOCK_CHECK
			if (ranand_write(buf, offs, aligned_size) != aligned_size)
			{
				printf("bad block: %x, try next: ", offs);
				offs += blocksize;
				printf("%x\n", offs);
				goto try_next_1;
			}
#else
			if (ranand_write(buf, offs, aligned_size) != aligned_size)
			{
				return -1;
			}
#endif
#ifdef RALINK_NAND_UPGRADE_CHECK
			temp = malloc(blocksize);
			if (!temp) {
				printf("%s: malloc temp failed\n", __func__);
				return -1;
			}
			//udelay(10);
			for( i=0; i< (aligned_size/blocksize); i++)
			{
				if (ranand_read(temp, offs+(i*blocksize), blocksize) != blocksize)
				{
					free(temp);
					return -2;
				}
				if(memcmp(buf+(i*blocksize), temp, blocksize) == 0)
				{
					//printf("blocksize write ok i=%d!\n\r", i);
				}
				else
				{
					printf("blocksize write incorrect block#=%d!\n\r",i);
					free(temp);
					return -2;
				}
			}
			free(temp);
#endif
			
			printf(".");

			buf += aligned_size;
			offs += aligned_size;
			count -= aligned_size;
		}
	}
	printf("Done!\n");
	return 0;
}


#ifdef MTK_NAND_BMT_DEBUG
int ranand_erase_raw(unsigned int offs, int len)
{
	int page, status;
	unsigned int blocksize = CFG_BLOCKSIZE;
	int ret = 0;

	ra_dbg("%s: start:%x, len:%x \n", __func__, offs, len);


	if (BLOCK_ALIGNED(offs) || BLOCK_ALIGNED(len)) {
		ra_dbg("%s: erase block not aligned, addr:%x len:%x %x\n", __func__, offs, len, CFG_BLOCKSIZE);
		return -1;
	}

	while (len) {
		page = (int)(offs >> CONFIG_PAGE_SIZE_BIT);

		/* select device and check wp */
		if (nfc_check_wp()) {
			printf("%s: nand is write protected\n", __func__);
			return -1;
		}

		status = nfc_erase_block(page);

		/* See if block erase succeeded */
		if (status) {
			printf("%s: failed erase, page 0x%08x\n", __func__, page);
			return -1;
		}

		/* Increment page address and decrement length */
		len -= blocksize;
		offs += blocksize;
	}

	return ret;
}
#endif

#ifdef MTK_NAND_BMT
int ranand_erase_bmt(unsigned int offs, int len)
{
	int page, status;
	unsigned int blocksize = CFG_BLOCKSIZE;
	int ret = 0;
    int block;
    u16 page_in_block;
    int mapped_block;
    u8 oob[16];

	if (offs < BMT_APPLY_START_OFFSET)
	{
		return ranand_erase(offs, len);
	}

	ra_dbg("%s: start:%x, len:%x \n", __func__, offs, len);

	if (BLOCK_ALIGNED(offs) || BLOCK_ALIGNED(len)) {
		ra_dbg("%s: erase block not aligned, addr:%x len:%x %x\n", __func__, offs, len, CFG_BLOCKSIZE);
		return -1;
	}

	while (len) {
		page = (int)(offs >> CONFIG_PAGE_SIZE_BIT);

		block = page >> CONFIG_NUMPAGE_PER_BLOCK_BIT;
		page_in_block = page & ((1 << CONFIG_NUMPAGE_PER_BLOCK_BIT) - 1);
		mapped_block = get_mapping_block_index(block);
			
		if (mapped_block != block)
		{
			page = page_in_block + (mapped_block << CONFIG_NUMPAGE_PER_BLOCK_BIT);
		}

		/* select device and check wp */
		if (nfc_check_wp()) {
			printf("%s: nand is write protected\n", __func__);
			return -1;
		}

		// read oob before erase
		nfc_read_oob(page, 0, oob, 16);
		oob[CONFIG_ECC_OFFSET] = 0xff;
		oob[CONFIG_ECC_OFFSET+1] = 0xff;
		oob[CONFIG_ECC_OFFSET+2] = 0xff;

		if (oob[CONFIG_BAD_BLOCK_POS] != 0xff)
		{
			if (update_bmt(page << CONFIG_PAGE_SIZE_BIT, UPDATE_ERASE_FAIL, NULL, NULL))
			{
				printf("%s: found bad block at page 0x%08x, update_bmt\n", __func__, page);
			}
			else
			{
				printf("%s: update bmt failed \n", __func__);
				return -1;
			}
			oob[CONFIG_BAD_BLOCK_POS] = 0xff;
			page = (int)(offs >> CONFIG_PAGE_SIZE_BIT);
			status = update_bmt_page(&page, oob);			
		}
		else
		{
			status = nfc_erase_block(page);
		}

		nfc_write_oob(page, 0, oob, 16);

		/* See if block erase succeeded */
		if (status) {
			if (update_bmt(page << CONFIG_PAGE_SIZE_BIT, UPDATE_ERASE_FAIL, NULL, NULL))
			{
				printf("%s: failed erase, page 0x%08x, update_bmt\n", __func__, page);
			}
			else
				return -1;
		}

		/* Increment page address and decrement length */
		len -= blocksize;
		offs += blocksize;
	}

	return ret;
}

int ranand_write_bmt(char *buf, unsigned int to, int datalen)
{
	int page, i = 0;
	size_t retlen = 0;
	int pagemask = (CFG_PAGESIZE -1);
	loff_t addr = to;
	char buffers[CFG_PAGESIZE + CFG_PAGE_OOBSIZE];
    int block, newpage;
    u16 page_in_block;
    int mapped_block;


	if (to < BMT_APPLY_START_OFFSET)
	{
		return ranand_write(buf, to, datalen);
	}

	if (buf == 0)
		datalen = 0;

	// page write
	while (datalen) {
		int len;
		int ret;
		int offs;

		page = (int)((addr & (CFG_CHIPSIZE-1)) >> CONFIG_PAGE_SIZE_BIT); //chip boundary
		
		ra_dbg("%s (%d): addr:%x, pg:%x, data:%p, datalen:%x\n", 
				__func__, i++, (unsigned int)addr, page, buf, datalen);

		/* select chip, and check if it is write protected */
		if (nfc_check_wp()) {
			printf("%s: nand is write protected\n", __func__);
			return -1;
		}

		memset(buffers, 0x0ff, sizeof(buffers));

		block = page >> CONFIG_NUMPAGE_PER_BLOCK_BIT;
		page_in_block = page & ((1 << CONFIG_NUMPAGE_PER_BLOCK_BIT) - 1);
		mapped_block = get_mapping_block_index(block);
		
		if (mapped_block != block)
		{
			newpage = page_in_block + (mapped_block << CONFIG_NUMPAGE_PER_BLOCK_BIT);
		}
		else
			newpage = page;

		// data write
		offs = addr & pagemask;
		len = min(datalen, CFG_PAGESIZE - offs);
		if (buf && len > 0) {
			memcpy(buffers + offs, buf, len);	// we can not sure ops->buf wether is DMA-able.

			buf += len;
			datalen -= len;
			retlen += len;
		}
		
		do {
			ret = nfc_write_page(buffers, newpage);
			if (ret) {
				if (update_bmt((newpage << CONFIG_PAGE_SIZE_BIT), UPDATE_WRITE_FAIL, buffers, buffers + CFG_PAGESIZE))
				{
					break;// Update BMT success
				}
				else
				{
					// Update BMT fail
				}
			}
			else
				break;
		}while (1);


		addr = (page+1) << CONFIG_PAGE_SIZE_BIT;
	}
	return retlen;
}

int ranand_read_bmt(char *buf, unsigned int from, int datalen)
{
	int page, i = 0;
	size_t retlen = 0;
	int pagemask = (CFG_PAGESIZE -1);
	loff_t addr = from;
	char buffers[CFG_PAGESIZE + CFG_PAGE_OOBSIZE];
    int block, newpage;
    u16 page_in_block;
    int mapped_block;

	if (from < BMT_APPLY_START_OFFSET)
		return ranand_read(buf, from, datalen);

	if (buf == 0)
		return 0;

	while (datalen) {
		int len;
		int ret;
		int offs;

		ra_dbg("%s (%d): addr:%x, datalen:%x\n", 
				__func__, i++, (unsigned int)addr, datalen);

		page = (int)((addr & (CFG_CHIPSIZE-1)) >> CONFIG_PAGE_SIZE_BIT); 

		block = page >> CONFIG_NUMPAGE_PER_BLOCK_BIT;
		page_in_block = page & ((1 << CONFIG_NUMPAGE_PER_BLOCK_BIT) - 1);
		mapped_block = get_mapping_block_index(block);
		
		if (mapped_block != block)
		{
			newpage = page_in_block + (mapped_block << CONFIG_NUMPAGE_PER_BLOCK_BIT);
		}
		else
			newpage = page;

		if (datalen > (CFG_PAGESIZE+CFG_PAGE_OOBSIZE) && (page & 0x1f) == 0)
			printf(".");
		ret = nfc_read_page(buffers, newpage);
		//FIXME, something strange here, some page needs 2 more tries to guarantee read success.
		if (ret) {
			printf("read again:\n");
			ret = nfc_read_page(buffers, newpage);
			if (ret) {
				printf("read again fail \n");
			}
			else {
				printf(" read agian susccess \n");
			}
		}

		// data read
		offs = addr & pagemask;
		len = min(datalen, CFG_PAGESIZE - offs);
		if (buf && len > 0) {
			memcpy(buf, buffers + offs, len); // we can not sure ops->buf wether is DMA-able.

			buf += len;
			datalen -= len;
			retlen += len;
			if (ret)
				return -1;
		}

		// address go further to next page, instead of increasing of length of write. This avoids some special cases wrong.
		addr = (page+1) << CONFIG_PAGE_SIZE_BIT;
	}
	if (datalen > (CFG_PAGESIZE+CFG_PAGE_OOBSIZE))
		printf("\n");
	return retlen;
}


int ranand_erase_write_bmt(char *buf, unsigned int offs, int count)
{
	int blocksize = CFG_BLOCKSIZE;
	int blockmask = blocksize - 1;
	int rc;
#ifdef RALINK_NAND_UPGRADE_CHECK
	int i = 0;
	char *temp;
#endif

	if (offs < BMT_APPLY_START_OFFSET)
		return ranand_erase_write(buf, offs, count);

	printf("%s: offs:%x, count:%x\n", __func__, offs, count);

	if (count > (CFG_CHIPSIZE - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))) {
		printf("Abort: image size larger than %d!\n\n", CFG_CHIPSIZE -
					(CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
		udelay(10*1000*1000);
		return -1;
	}

	while (count > 0) {
#define BLOCK_ALIGNE(a) (((a) & blockmask))
//#ifdef CONFIG_BADBLOCK_CHECK
#if 0
		if (BLOCK_ALIGNE(offs)) {
			printf("%s: offs %x is not aligned\n", __func__, offs);
			return -1;
		}
		if (count < blocksize)
#else
		if (BLOCK_ALIGNE(offs) || (count < blocksize))
#endif
		{
			char *block;
			unsigned int piece, blockaddr;
			int piece_size;

			block = malloc(blocksize);
			if (!block) {
				printf("%s: malloc block failed\n", __func__);
				return -1;
			}

			blockaddr = offs & ~blockmask;
			if (ranand_read_bmt(block, blockaddr, blocksize) != blocksize) {
				free(block);
				return -2;
			}

			piece = offs & blockmask;
			piece_size = min(count, blocksize - piece);
			memcpy(block + piece, buf, piece_size);

			rc = ranand_erase_bmt(blockaddr, blocksize);
			if (rc != 0) {
				free(block);
				return -3;
			}
			if (ranand_write_bmt(block, blockaddr, blocksize) != blocksize) {
				free(block);
				return -4;
			}
#ifdef RALINK_NAND_UPGRADE_CHECK
			temp = malloc(blocksize);
			if (!temp) {
				printf("%s: malloc temp failed\n", __func__);
				return -1;
			}

			if (ranand_read_bmt(temp, blockaddr, blocksize) != blocksize) {
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
                        free(temp);
#endif
			free(block);

			buf += piece_size;
			offs += piece_size;
			count -= piece_size;
		}
		else {
			unsigned int aligned_size = blocksize;

			rc = ranand_erase_bmt(offs, aligned_size);
			if (rc != 0)
			{
				return -1;
			}
			if (ranand_write_bmt(buf, offs, aligned_size) != aligned_size)
			{
				return -1;
			}
#ifdef RALINK_NAND_UPGRADE_CHECK
			temp = malloc(blocksize);
			if (!temp) {
				printf("%s: malloc temp failed\n", __func__);
				return -1;
			}
			//udelay(10);
			for( i=0; i< (aligned_size/blocksize); i++)
			{
				if (ranand_read_bmt(temp, offs+(i*blocksize), blocksize) != blocksize)
				{
					free(temp);
					return -2;
				}
				if(memcmp(buf+(i*blocksize), temp, blocksize) == 0)
				{
					//printf("blocksize write ok i=%d!\n\r", i);
				}
				else
				{
					printf("blocksize write incorrect block#=%d!\n\r",i);
					free(temp);
					return -2;
				}
			}
			free(temp);
#endif
			
			printf(".");

			buf += aligned_size;
			offs += aligned_size;
			count -= aligned_size;
		}
	}
	printf("Done!\n");
	return 0;
}

#endif


#define NAND_FLASH_DBG_CMD
#ifdef NAND_FLASH_DBG_CMD
int ralink_nand_command(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	int len, i;
	u8 *p = NULL;

	if (!strncmp(argv[1], "id", 3)) {
		u8 id[4];
		i = nfc_read_raw_data(0x90, 0, 0, 0, 0x410141, id, 4);
		printf("flash id: %x %x %x %x\n", id[0], id[1], id[2], id[3]);
	}
	else if (!strncmp(argv[1], "read", 5)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		len = (int)simple_strtoul(argv[3], NULL, 16);
		p = (u8 *)malloc(len);
		if (!p) {
			printf("malloc error\n");
			return 0;
		}
#ifdef MTK_NAND_BMT
		len = ranand_read_bmt(p, addr, len); //reuse len
#else
		len = ranand_read(p, addr, len); //reuse len
#endif
		printf("read len: %d\n", len);
		for (i = 0; i < len; i++) {
			printf("%02x ", p[i]);
		}
		printf("\n");
		free(p);
	}
	else if (!strncmp(argv[1], "page", 5)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16); //page
		p = (u8 *)malloc(CFG_PAGESIZE+CFG_PAGE_OOBSIZE);
		nfc_read_page(p, addr);
		printf("page 0x%x:\n", addr);
		for (i = 0; i < CFG_PAGESIZE; i++)
			printf("%02x%c", p[i], (i%32 == 31)? '\n':' ');
		printf("oob:\n");
		for (; i < CFG_PAGESIZE+CFG_PAGE_OOBSIZE; i++)
			printf("%02x%c", p[i], (i%32 == 31)? '\n':' ');
		free(p);
		printf("\n");
	}
	else if (!strncmp(argv[1], "erase", 6)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		len = (int)simple_strtoul(argv[3], NULL, 16);
#ifdef MTK_NAND_BMT
		if (ranand_erase_bmt(addr, len) != 0)
#else
		if (ranand_erase(addr, len) != 0)
#endif
			printf("erase failed\n");
		else
			printf("erase succeed\n");
	}
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
#ifdef MTK_NAND_BMT
		ranand_write_bmt(p, o, l);
#else
		ranand_write(p, o, l);
#endif
		free(p);
	}
	else if (!strncmp(argv[1], "oob", 4)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16); //page
		p = (u8 *)malloc(CFG_PAGE_OOBSIZE);
		nfc_read_oob(addr, 0, p, CFG_PAGE_OOBSIZE);
		printf("oob page %x (addr %x):\n", addr, (addr << CONFIG_PAGE_SIZE_BIT));
		for (i = 0; i < CFG_PAGE_OOBSIZE; i++)
			printf("%02x%c", p[i], (i%32 == 31)? '\n':' ');
		free(p);
	}
	else if (!strncmp(argv[1], "init", 5)) {
		ranand_init();
	}
#ifdef MTK_NAND_BMT
	else if (!strncmp(argv[1], "bmt", 4)) {
		dump_bmt();
	}
	// erase + write
	else if (!strncmp(argv[1], "erawr", 6)) {
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
		ranand_erase_write_bmt(p, o, l);
		free(p);
	}
#endif	
#ifdef MTK_NAND_BMT_DEBUG
	// raw operation without block mapping
	else if (!strncmp(argv[1], "rawe", 5)) {
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		len = (int)simple_strtoul(argv[3], NULL, 16);
		if (ranand_erase_raw(addr, len) != 0)
			printf("erase failed\n");
		else
			printf("erase succeed\n");
	}
	else if (!strncmp(argv[1], "markb", 6)) {
		u8 bad = 0x33;
		addr = (unsigned int)simple_strtoul(argv[2], NULL, 16); //page
		printf("mark page 0x%x bad\n", addr);
		nfc_write_oob(addr, CONFIG_BAD_BLOCK_POS, &bad, 1);
		printf("\n");
	}
#endif
	else
		printf("Usage:\n%s\n use \"help nand\" for detail!\n", cmdtp->usage);
	return 0;
}

U_BOOT_CMD(
	nand,	4,	1, 	ralink_nand_command,
	"nand	- nand command\n",
	"nand usage:\n"
	"  nand id\n"
	"  nand read <addr> <len>\n"
	"  nand write <addr> <data...>\n"
	"  nand page <number>\n"
	"  nand erase <addr> <len>\n"
#ifdef MTK_NAND_BMT
	"  nand erawr <addr> <data...>\n"
#ifdef MTK_NAND_BMT_DEBUG
	"  nand rawe <addr> <len>\n"
#endif
	"  nand bmt\n"
#endif
	"  nand init\n"
);
#endif
