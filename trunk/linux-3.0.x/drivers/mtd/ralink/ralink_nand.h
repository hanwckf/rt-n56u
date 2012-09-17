#ifndef RT2880_NAND_H
#define RT2880_NAND_H
#include <linux/mtd/mtd.h>

#if !defined (__UBOOT__)
#include <asm/rt2880/rt_mmap.h>
#else
#include <rt_mmap.h>
#define	EIO		 5	/* I/O error */
#define	EINVAL		22	/* Invalid argument */
#define	ENOMEM		12	/* Out of memory */
#define	EBADMSG		74	/* Not a data message */
#define	EUCLEAN		117	/* Structure needs cleaning */
#endif

#include "gdma.h"

#define ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))
#define ra_aor(addr, a_mask, o_value)  ra_outl(addr, (ra_inl(addr) & (a_mask)) | (o_value))


//#define udelay(a) do{} while(0) 

#define NFC_BASE 	RALINK_NAND_CTRL_BASE
#define NFC_CTRL	(NFC_BASE + 0x0)
#define NFC_CONF	(NFC_BASE + 0x4)
#define NFC_CMD1	(NFC_BASE + 0x8)
#define NFC_CMD2	(NFC_BASE + 0xc)
#define NFC_CMD3	(NFC_BASE + 0x10)
#define NFC_ADDR	(NFC_BASE + 0x14)
#define NFC_DATA	(NFC_BASE + 0x18)
#define NFC_ECC		(NFC_BASE + 0x1c)
#define NFC_STATUS	(NFC_BASE + 0x20)
#define NFC_INT_EN	(NFC_BASE + 0x24)
#define NFC_INT_ST	(NFC_BASE + 0x28)

enum _int_stat {
	INT_ST_ND_DONE 	= 1<<0,
	INT_ST_TX_BUF_RDY       = 1<<1,
	INT_ST_RX_BUF_RDY	= 1<<2,
	INT_ST_ECC_ERR		= 1<<3,
	INT_ST_TX_TRAS_ERR	= 1<<4,
	INT_ST_RX_TRAS_ERR	= 1<<5,
	INT_ST_TX_KICK_ERR	= 1<<6,
	INT_ST_RX_KICK_ERR      = 1<<7
};


//#define WORKAROUND_RX_BUF_OV 1

//chip
#define CONFIG_NUMCHIPS 1
#define CONFIG_NOT_SUPPORT_WP //rt3052 has no WP signal for chip.
#define CONFIG_NOT_SUPPORT_RB

// chip geometry: SAMSUNG small size 32MB.
#define CONFIG_CHIP_SIZE_BIT (25) 	//! (1<<NAND_SIZE_BYTE) MB
#define CONFIG_PAGE_SIZE_BIT (9)   	//! (1<<PAGE_SIZE) MB
#define CONFIG_SUBPAGE_BIT 1		//! these bits will be compensate by command cycle
#define CONFIG_NUMPAGE_PER_BLOCK_BIT 5	//! order of number of pages a block. 
#define CONFIG_OOBSIZE_PER_PAGE_BIT 4	//! byte number of oob a page.
#define CONFIG_BAD_BLOCK_POS	4	//! offset of byte to denote bad block.
#define CONFIG_ECC_BYTES 3		//! ecc has 3 bytes
#define CONFIG_ECC_OFFSET 5		//! ecc starts from offset 5.


//this section should not be modified.
#define CFG_COLUMN_ADDR_MASK ((1<<(CONFIG_PAGE_SIZE_BIT-CONFIG_SUBPAGE_BIT)) - 1)
#define CFG_COLUMN_ADDR_CYCLE (((CONFIG_PAGE_SIZE_BIT-CONFIG_SUBPAGE_BIT) + 7)/8) 
#define CFG_ROW_ADDR_CYCLE ((CONFIG_CHIP_SIZE_BIT - CONFIG_PAGE_SIZE_BIT + 7)/8) 
#define CFG_ADDR_CYCLE (CFG_COLUMN_ADDR_CYCLE + CFG_ROW_ADDR_CYCLE)

#define CFG_PAGESIZE	(1<<CONFIG_PAGE_SIZE_BIT)
#define CFG_BLOCKSIZE 	(CFG_PAGESIZE << CONFIG_NUMPAGE_PER_BLOCK_BIT)	//32 pages a block
#define CFG_CHIPSIZE  	(1<<CONFIG_CHIP_SIZE_BIT)
#define CFG_NUMPAGE	(1<<(CONFIG_CHIP_SIZE_BIT - CONFIG_PAGE_SIZE_BIT))
#define CFG_NUMBLOCK	(CFG_NUMPAGE >> CONFIG_NUMPAGE_PER_BLOCK_BIT)
#define CFG_BLOCK_OOBSIZE	(1 << (CONFIG_OOBSIZE_PER_PAGE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT))	
#define CFG_PAGE_OOBSIZE	(1 << CONFIG_OOBSIZE_PER_PAGE_BIT)	

#define NAND_BLOCK_ALIGN(addr) ((addr) & (CFG_BLOCKSIZE-1))
#define NAND_PAGE_ALIGN(addr) ((addr) & (CFG_PAGESIZE-1))



/*************************************************************
 * stolen from nand.h
 *************************************************************/

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_RNDOUT		5
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_RNDIN		0x85
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_RESET		0xff

/* Extended commands for large page devices */
#define NAND_CMD_READSTART	0x30
#define NAND_CMD_RNDOUTSTART	0xE0
#define NAND_CMD_CACHEDPROG	0x15

/* Extended commands for AG-AND device */
/*
 * Note: the command for NAND_CMD_DEPLETE1 is really 0x00 but
 *       there is no way to distinguish that from NAND_CMD_READ0
 *       until the remaining sequence of commands has been completed
 *       so add a high order bit and mask it off in the command.
 */
#define NAND_CMD_DEPLETE1	0x100
#define NAND_CMD_DEPLETE2	0x38
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_STATUS_ERROR	0x72
/* multi-bank error status (banks 0-3) */
#define NAND_CMD_STATUS_ERROR0	0x73
#define NAND_CMD_STATUS_ERROR1	0x74
#define NAND_CMD_STATUS_ERROR2	0x75
#define NAND_CMD_STATUS_ERROR3	0x76
#define NAND_CMD_STATUS_RESET	0x7f
#define NAND_CMD_STATUS_CLEAR	0xff

#define NAND_CMD_NONE		-1

/* Status bits */
#define NAND_STATUS_FAIL	0x01
#define NAND_STATUS_FAIL_N1	0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_WP		0x80

typedef enum {
	FL_READY,
	FL_READING,
	FL_WRITING,
	FL_ERASING,
	FL_SYNCING,
	FL_CACHEDPRG,
	FL_PM_SUSPENDED,
} nand_state_t;

/*************************************************************/



typedef enum _ra_flags {
	FLAG_NONE	= 0,
	FLAG_ECC_EN 	= (1<<0),
	FLAG_USE_GDMA 	= (1<<1),
	FLAG_VERIFY 	= (1<<2),
} RA_FLAGS;


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
#if !defined (__UBOOT__)
	struct mutex hwcontrol;
	struct mutex *controller;
#endif
	struct nand_ecclayout	*oob;
	int 	state;
	unsigned int 	buffers_page;
	char	*buffers; //[CFG_PAGESIZE + CFG_PAGE_OOBSIZE];
	char 	*readback_buffers;
	unsigned char 	*bbt;
#if defined (WORKAROUND_RX_BUF_OV)
	unsigned int	 sandbox_page;	// steal a page (block) for read ECC verification
#endif

};



//fixme, gdma api 
int nand_dma_sync(void);
void release_dma_buf(void);
int set_gdma_ch(unsigned long dst, 
		unsigned long src, unsigned int len, int burst_size,
		int soft_mode, int src_req_type, int dst_req_type,
		int src_burst_mode, int dst_burst_mode);




#endif
