#ifndef __RALINK_NAND_H__
#define __RALINK_NAND_H__

#include <linux/mtd/nand.h>
#include <rt_mmap.h>

#define CONFIG_NOT_SUPPORT_WP //rt3052 has no WP signal for chip.
//#define CONFIG_NOT_SUPPORT_RB
#define CONFIG_BADBLOCK_CHECK

extern int is_nand_page_2048;
extern nand_addrlen;
extern const unsigned int nand_size_map[2][3];

#define CONFIG_CHIP_SIZE_BIT (nand_size_map[is_nand_page_2048][nand_addrlen-3]) //! (1<<NAND_SIZE_BYTE) MB
#define CONFIG_PAGE_SIZE_BIT (is_nand_page_2048? 11 : 9)	//! (1<<PAGE_SIZE) MB
#define CONFIG_NUMPAGE_PER_BLOCK_BIT (is_nand_page_2048? 6 : 5)	//! order of number of pages a block.
#define CONFIG_OOBSIZE_PER_PAGE_BIT (is_nand_page_2048? 6 : 4)	//! byte number of oob a page.
#define CONFIG_BAD_BLOCK_POS (is_nand_page_2048? 0 : 4)		//! offset of byte to denote bad block.
#define CONFIG_ECC_BYTES 3		//! ecc has 3 bytes
#define CONFIG_ECC_OFFSET (is_nand_page_2048? 6 : 5)		//! ecc starts from offset 5.

#define CFG_COLUMN_ADDR_CYCLE 	(is_nand_page_2048? 2 : 1) 
#define CFG_ROW_ADDR_CYCLE 		(nand_addrlen - CFG_COLUMN_ADDR_CYCLE)  
#define CFG_ADDR_CYCLE (CFG_COLUMN_ADDR_CYCLE + CFG_ROW_ADDR_CYCLE)

#define CFG_CHIPSIZE	(1 << ((CONFIG_CHIP_SIZE_BIT>=32)? 31 : CONFIG_CHIP_SIZE_BIT))
#define CFG_PAGESIZE	(1 << CONFIG_PAGE_SIZE_BIT)
#define CFG_BLOCKSIZE	(CFG_PAGESIZE << CONFIG_NUMPAGE_PER_BLOCK_BIT)
#define CFG_NUMPAGE	(1 << (CONFIG_CHIP_SIZE_BIT - CONFIG_PAGE_SIZE_BIT))
#define CFG_NUMBLOCK	(CFG_NUMPAGE >> CONFIG_NUMPAGE_PER_BLOCK_BIT)
#define CFG_BLOCK_OOBSIZE	(1 << (CONFIG_OOBSIZE_PER_PAGE_BIT + CONFIG_NUMPAGE_PER_BLOCK_BIT))	
#define CFG_PAGE_OOBSIZE	(1 << CONFIG_OOBSIZE_PER_PAGE_BIT)	

#define NAND_BLOCK_ALIGN(addr) ((addr) & (CFG_BLOCKSIZE-1))
#define NAND_PAGE_ALIGN(addr) ((addr) & (CFG_PAGESIZE-1))


#define NFC_BASE 	RALINK_NAND_CTRL_BASE
#define NFC_CTRL	(NFC_BASE + 0x0)
#define NFC_CONF	(NFC_BASE + 0x4)
#define NFC_CMD1	(NFC_BASE + 0x8)
#define NFC_CMD2	(NFC_BASE + 0xc)
#define NFC_CMD3	(NFC_BASE + 0x10)
#define NFC_ADDR	(NFC_BASE + 0x14)
#define NFC_DATA	(NFC_BASE + 0x18)
#if defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) || \
    defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) || \
    defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD)
#define NFC_ECC		(NFC_BASE + 0x30)
#else
#define NFC_ECC		(NFC_BASE + 0x1c)
#endif
#define NFC_STATUS	(NFC_BASE + 0x20)
#define NFC_INT_EN	(NFC_BASE + 0x24)
#define NFC_INT_ST	(NFC_BASE + 0x28)
#if defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) || \
    defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) || \
    defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD)
#define NFC_CONF1	(NFC_BASE + 0x2c)
#define NFC_ECC_P1	(NFC_BASE + 0x30)
#define NFC_ECC_P2	(NFC_BASE + 0x34)
#define NFC_ECC_P3	(NFC_BASE + 0x38)
#define NFC_ECC_P4	(NFC_BASE + 0x3c)
#define NFC_ECC_ERR1	(NFC_BASE + 0x40)
#define NFC_ECC_ERR2	(NFC_BASE + 0x44)
#define NFC_ECC_ERR3	(NFC_BASE + 0x48)
#define NFC_ECC_ERR4	(NFC_BASE + 0x4c)
#define NFC_ADDR2	(NFC_BASE + 0x50)
#endif

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



typedef enum _ra_flags {
	FLAG_NONE	= 0,
	FLAG_ECC_EN 	= (1<<0),
	FLAG_USE_GDMA 	= (1<<1),
	FLAG_VERIFY 	= (1<<2),
} RA_FLAGS;
#endif /* __RALINK_NAND_H__ */
