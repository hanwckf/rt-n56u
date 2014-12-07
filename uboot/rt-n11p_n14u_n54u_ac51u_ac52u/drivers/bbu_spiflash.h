#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#include <rt_mmap.h>

#define STM_PAGE_SIZE		256

#define SPI_WRITE_ENABLE	0
#define SPI_WRITE_DISABLE	1
#define SPI_RD_STATUS		2
#define SPI_WR_STATUS		3
#define SPI_RD_DATA		4
#define SPI_FAST_RD_DATA	5
#define SPI_PAGE_PROGRAM	6
#define SPI_SECTOR_ERASE	7
#define SPI_BULK_ERASE		8
#define SPI_DEEP_PWRDOWN	9
#define SPI_RD_SIG		10
#define SPI_RD_ID		11
#define SPI_RD_STATUS2		12
#define SPI_HPM_ENABLE		13
#define SPI_MAX_OPCODES		14

#define SFI_WRITE_BUFFER_SIZE	4
#define SFI_FLASH_ADDR_MASK	0x00ffffff

/*
 * ST Microelectronics Opcodes for Serial Flash
 */

#define STM_OP_WR_ENABLE	0x06	/* Write Enable */
#define STM_OP_WR_DISABLE	0x04	/* Write Disable */
#define STM_OP_RD_STATUS	0x05	/* Read Status */
#define STM_OP_RD_STATUS2	0x35	/* Read Status 2 */
#define STM_OP_WR_STATUS	0x01	/* Write Status */
#define STM_OP_RD_DATA		0x03	/* Read Data */
#define STM_OP_FAST_RD_DATA	0x0b	/* Fast Read Data */
#define STM_OP_PAGE_PGRM	0x02	/* Page Program */
#define STM_OP_SECTOR_ERASE	0xd8	/* Sector Erase */
#define STM_OP_BULK_ERASE	0xc7	/* Bulk Erase */
#define STM_OP_DEEP_PWRDOWN	0xb9	/* Deep Power-Down Mode */
#define STM_OP_RD_SIG		0xab	/* Read Electronic Signature */
#define STM_OP_RD_ID		0x9f	/* Read JEDEC ID */
#define STM_OP_HPM		0xa3	/* High Performance Mode */

#define STM_STATUS_WIP		0x01	/* Write-In-Progress */
#define STM_STATUS_WEL		0x02	/* Write Enable Latch */
#define STM_STATUS_BP0		0x04	/* Block Protect 0 */
#define STM_STATUS_BP1		0x08	/* Block Protect 1 */
#define STM_STATUS_BP2		0x10	/* Block Protect 2 */
#define STM_STATUS_SRWD		0x80	/* Status Register Write Disable */

#define STM_STATUS_QE		0x02	/* Quad Enable */

#define SPI_REG_CTL		(RALINK_SPI_BASE + 0x00)
#define SPI_REG_OPCODE		(RALINK_SPI_BASE + 0x04)
#define SPI_REG_DATA0		(RALINK_SPI_BASE + 0x08)
#define SPI_REG_DATA(x)		(SPI_REG_DATA0 + (x * 4))
#define SPI_REG_MASTER		(RALINK_SPI_BASE + 0x28)
#define SPI_REG_MOREBUF		(RALINK_SPI_BASE + 0x2c)
#define SPI_REG_Q_CTL		(RALINK_SPI_BASE + 0x30)
#define SPI_REG_SPACE_CR	(RALINK_SPI_BASE + 0x3c)

#define SPI_CTL_START		0x00000100
#define SPI_CTL_BUSY		0x00010000
#define SPI_CTL_TXCNT_MASK	0x0000000f
#define SPI_CTL_RXCNT_MASK	0x000000f0
#define SPI_CTL_TX_RX_CNT_MASK	0x000000ff
#define SPI_CTL_SIZE_MASK	0x00180000
#define SPI_CTL_ADDREXT_MASK	0xff000000

#define SPI_MBCTL_TXCNT_MASK		0x000001ff
#define SPI_MBCTL_RXCNT_MASK		0x001ff000
#define SPI_MBCTL_TX_RX_CNT_MASK	(SPI_MBCTL_TXCNT_MASK | SPI_MBCTL_RXCNT_MASK)
#define SPI_MBCTL_CMD_MASK		0x2f000000

#define SPI_CTL_CLK_SEL_MASK	0x03000000
#define SPI_OPCODE_MASK		0x000000ff

#define SPI_STATUS_WIP		STM_STATUS_WIP

#ifdef CONFIG_DUAL_IMAGE
struct spi_flash_info {
	const u16 mfr_id;
	const u16 dev_id;
	const u16 extra_id;
	const char *name;
	const int DeviceSize;
	const int EraseSize;
	int mode;
};
#endif
extern int spiflash_init (unsigned long rom_base);
extern int spiflash_erase(unsigned long addr, unsigned long size);
extern int spiflash_read(unsigned long from, unsigned long len, 
	unsigned long *retlen, unsigned char *buf);
extern int spiflash_write(unsigned long to, unsigned long len, 
	unsigned long *retlen, const unsigned char *buf);

/* Mapping of generic opcodes to STM serial flash opcodes */
struct opcodes {
	__u16 code;
	__s8 tx_cnt;
	__s8 rx_cnt;
} stm_opcodes[] = {
	{STM_OP_WR_ENABLE, 1, 0},
	{STM_OP_WR_DISABLE, 1, 0},
	{STM_OP_RD_STATUS, 1, 1},
	{STM_OP_WR_STATUS, 2, 0},
	{STM_OP_RD_DATA, 4, 4},
	{STM_OP_FAST_RD_DATA, 1, 0},
	{STM_OP_PAGE_PGRM, 8, 0},
	{STM_OP_SECTOR_ERASE, 4, 0},
	{STM_OP_BULK_ERASE, 1, 0},
	{STM_OP_DEEP_PWRDOWN, 1, 0},
	{STM_OP_RD_SIG, 4, 1},
	{STM_OP_RD_ID, 1, 3},
	{STM_OP_RD_STATUS2, 1, 1},
	{STM_OP_HPM, 1, 0}
};

#endif
