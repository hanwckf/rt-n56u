#include <rt_mmap.h>

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
#define RSTCTRL_SPI_RESET		(1<<11)
#else
#define RSTCTRL_SPI_RESET		(1<<18)
#endif

#if defined(RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
#define RT2880_SPI_CS1_OFFSET 0x40
#else  
#define RT2880_SPI_CS1_OFFSET 0
#endif

#define RT2880_SPI_REG_BASE		(RALINK_SYSCTL_BASE + 0x0B00)
#define RT2880_SPISTAT_REG		(RT2880_SPI_REG_BASE+ RT2880_SPI_CS1_OFFSET+ 0x00)
#define RT2880_SPICFG_REG		(RT2880_SPI_REG_BASE+ RT2880_SPI_CS1_OFFSET+ 0x10)
#define RT2880_SPICTL_REG		(RT2880_SPI_REG_BASE+ RT2880_SPI_CS1_OFFSET+ 0x14)
#define RT2880_SPIDATA_REG		(RT2880_SPI_REG_BASE+ RT2880_SPI_CS1_OFFSET+ 0x20)

#define RT2880_SPIUSER_REG		(RT2880_SPI_REG_BASE+0x2C)
#define RT2880_SPIADDR_REG		(RT2880_SPI_REG_BASE+0x24)
#define RT2880_SPIMODE_REG		(RT2880_SPI_REG_BASE+0x3c)
#define RT2880_SPIBS_REG		(RT2880_SPI_REG_BASE+0x28)
#define RT2880_SPITXFIFO_REG	(RT2880_SPI_REG_BASE+0x30)
#define RT2880_SPIRXFIFO_REG	(RT2880_SPI_REG_BASE+0x34)
#define RT2880_SPIFIFOSTAT_REG	(RT2880_SPI_REG_BASE+0x38)

#define RT2880_SPI0_STAT_REG	(RT2880_SPI_REG_BASE+ 0x00)
#define RT2880_SPI0_CFG_REG		(RT2880_SPI_REG_BASE+ 0x10)
#define RT2880_SPI0_CTL_REG		(RT2880_SPI_REG_BASE+ 0x14)
#define RT2880_SPI0_DATA_REG	(RT2880_SPI_REG_BASE+ 0x20)

#define RT2880_SPI1_STAT_REG	(RT2880_SPI_REG_BASE+ 0x40)
#define RT2880_SPI1_CFG_REG		(RT2880_SPI_REG_BASE+ 0x50)
#define RT2880_SPI1_CTL_REG		(RT2880_SPI_REG_BASE+ 0x54)
#define RT2880_SPI1_DATA_REG	(RT2880_SPI_REG_BASE+ 0x60)

#define RT2880_SPI_DMA			(RT2880_SPI_REG_BASE+ 0x80)
#define RT2880_SPI_ARB_REG		(RT2880_SPI_REG_BASE+ 0xf0)

/* SPICFG register bit field */
#define SPICFG_LSBFIRST			(0<<8)
#define SPICFG_MSBFIRST			(1<<8)

#define SPICFG_RXCLKEDGE_FALLING	(1<<5)		/* rx on the falling edge of the SPICLK signal */
#define SPICFG_TXCLKEDGE_FALLING	(1<<4)		/* tx on the falling edge of the SPICLK signal */

#define SPICFG_SPICLK_DIV2		(0<<0)		/* system clock rat / 2  */
#define SPICFG_SPICLK_DIV4		(1<<0)		/* system clock rat / 4  */
#define SPICFG_SPICLK_DIV8		(2<<0)		/* system clock rat / 8  */
#define SPICFG_SPICLK_DIV16		(3<<0)		/* system clock rat / 16  */
#define SPICFG_SPICLK_DIV32		(4<<0)		/* system clock rat / 32  */
#define SPICFG_SPICLK_DIV64		(5<<0)		/* system clock rat / 64  */
#define SPICFG_SPICLK_DIV128		(6<<0)		/* system clock rat / 128 */

#define SPICFG_SPICLKPOL		(1<<6)		/* spi clk*/

#define SPICFG_ADDRMODE			(1 << 12)
#define SPICFG_RXENVDIS			(1<<11)
#define SPICFG_RXCAP			(1<<10)
#define SPICFG_SPIENMODE		(1<<9)

/* SPICTL register bit field */
#define SPICTL_HIZSDO			(1<<3)
#define SPICTL_STARTWR			(1<<2)
#define SPICTL_STARTRD			(1<<1)
#define SPICTL_SPIENA_LOW		(0<<0)		/* #cs low active */
#define SPICTL_SPIENA_HIGH		(1<<0)

/* SPI COMMAND MODE */
#define SPICTL_START				(1<<4)
#define SPIFIFO_TX_FULL				(1 << 17)
#define SPIFIFO_RX_EMPTY			(1 << 18)
#define SPIINT_SPIDONE				(1<<0)
#define SPIINT_ILLSPI				(1<<1)
#define SPIINT_RX_EMPTY_RD			(1<<2)
#define SPIINT_TX_FULL_WR			(1<<3)
#define SPIINT_DMA_EMPTY_RD			(1<<4)
#define SPIINT_DMA_FULL_WR			(1<<5)
/* SPI USER MODE */
#define SPIUSR_SINGLE				0x1
#define SPIUSR_DUAL					0x2
#define SPIUSR_QUAD					0x4
#define SPIUSR_NO_DATA				0x0
#define SPIUSR_READ_DATA			0x1
#define SPIUSR_WRITE_DATA			0x2
#define SPIUSR_NO_DUMMY				0x0
#define SPIUSR_ONE_DUMMY			0x1
#define SPIUSR_TWO_DUMMY			0x2
#define SPIUSR_THREE_DUMMY			0x3
#define SPIUSR_NO_MODE				0x0
#define SPIUSR_ONE_MODE				0x1
#define SPIUSR_NO_ADDR				0x0
#define SPIUSR_ONE_BYTE_ADDR		0x1
#define SPIUSR_TWO_BYTE_ADDR		0x2
#define SPIUSR_THREE_BYTE_ADDR		0x3
#define SPIUSR_FOUR_BYTE_ADDR		0x4
#define SPIUSR_NO_INSTRU			0x0
#define SPIUSR_ONE_INSTRU			0x1

/* SPIARB register bit field */
#define SPIARB_ARB_EN			(1<<31)

#ifdef RALINK_SPI_CS0_HIGH_ACTIVE
#define SPIARB_SPI0_ACTIVE_MODE		1
#else
#define SPIARB_SPI0_ACTIVE_MODE		0
#endif

#ifdef RALINK_SPI_CS1_HIGH_ACTIVE
#define SPIARB_SPI1_ACTIVE_MODE		1
#else
#define SPIARB_SPI1_ACTIVE_MODE		0
#endif
