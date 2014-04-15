#include <rt_mmap.h>

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
#define RSTCTRL_SPI_RESET		(1<<11)
#else
#define RSTCTRL_SPI_RESET		(1<<18)
#endif

#define RT2880_SPI_REG_BASE		(RALINK_SYSCTL_BASE + 0x0B00)
#define RT2880_SPISTAT_REG		(RT2880_SPI_REG_BASE+0x00)
#define RT2880_SPICFG_REG		(RT2880_SPI_REG_BASE+0x10)
#define RT2880_SPICTL_REG		(RT2880_SPI_REG_BASE+0x14)
#define RT2880_SPIDATA_REG		(RT2880_SPI_REG_BASE+0x20)

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

/* SPICTL register bit field */
#define SPICTL_HIZSDO			(1<<3)
#define SPICTL_STARTWR			(1<<2)
#define SPICTL_STARTRD			(1<<1)
#define SPICTL_SPIENA_LOW		(0<<0)		/* #cs low active */
#define SPICTL_SPIENA_HIGH		(1<<0)


