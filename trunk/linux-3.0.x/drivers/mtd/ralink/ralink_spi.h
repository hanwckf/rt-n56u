#ifndef __SPIC_H__
#define __SPIC_H__

#if !defined (__UBOOT__)
#include <asm/rt2880/rt_mmap.h>


#define RT2880_RSTCTRL_REG		(RALINK_SYSCTL_BASE+0x34)

#else //__UBOOT__

#include <rt_mmap.h>
#define	EIO		 5	/* I/O error */
#define	EINVAL		22	/* Invalid argument */
#define	ENOMEM		12	/* Out of memory */
#define	EBADMSG		74	/* Not a data message */
#define	EUCLEAN		117	/* Structure needs cleaning */
#define RALINK_SPI_RST			(1<<11)
#endif  //__UBOOT__

#define RSTCTRL_SPI_RESET		RALINK_SPI_RST

#define RT2880_SPI_REG_BASE		(RALINK_SPI_BASE)
#define RT2880_SPISTAT_REG		(RT2880_SPI_REG_BASE+0x00)
#define RT2880_SPICFG_REG		(RT2880_SPI_REG_BASE+0x10)
#define RT2880_SPICTL_REG		(RT2880_SPI_REG_BASE+0x14)
#define RT2880_SPIDATA_REG		(RT2880_SPI_REG_BASE+0x20)


#define RT2880_SPI0_CTL_REG		RT2880_SPICTL_REG
#define RT2880_SPI1_CTL_REG		(RT2880_SPI_REG_BASE+0x54)
#define RT2880_SPI_ARB_REG		(RT2880_SPI_REG_BASE+0xf0)

/* SPICFG register bit field */
#define SPICFG_LSBFIRST				(0<<8)
#define SPICFG_MSBFIRST				(1<<8)

#define SPICFG_RXCLKEDGE_FALLING	(1<<5)		/* rx on the falling edge of the SPICLK signal */
#define SPICFG_TXCLKEDGE_FALLING	(1<<4)		/* tx on the falling edge of the SPICLK signal */

#define SPICFG_SPICLK_DIV2			(0<<0)		/* system clock rat / 2  */
#define SPICFG_SPICLK_DIV4			(1<<0)		/* system clock rat / 4  */
#define SPICFG_SPICLK_DIV8			(2<<0)		/* system clock rat / 8  */
#define SPICFG_SPICLK_DIV16			(3<<0)		/* system clock rat / 16  */
#define SPICFG_SPICLK_DIV32			(4<<0)		/* system clock rat / 32  */
#define SPICFG_SPICLK_DIV64			(5<<0)		/* system clock rat / 64  */
#define SPICFG_SPICLK_DIV128		(6<<0)		/* system clock rat / 128 */

#define SPICFG_SPICLKPOL		(1<<6)		/* spi clk*/

/* SPICTL register bit field */
#define SPICTL_HIZSDO				(1<<3)
#define SPICTL_STARTWR				(1<<2)
#define SPICTL_STARTRD				(1<<1)
#define SPICTL_SPIENA_LOW			(0<<0)		/* #cs low active */
#define SPICTL_SPIENA_HIGH			(1<<0)

/* SPIARB register bit field */
#define SPIARB_ARB_EN			(1<<31)

#if defined(CONFIG_RALINK_SPI_CS0_HIGH_ACTIVE)
#define SPIARB_SPI0_ACTIVE_MODE		1
#else
#define SPIARB_SPI0_ACTIVE_MODE		0
#endif

#if defined(CONFIG_RALINK_SPI_CS1_HIGH_ACTIVE)
#define SPIARB_SPI1_ACTIVE_MODE		1
#else
#define SPIARB_SPI1_ACTIVE_MODE		0
#endif

#define spi_busy_loop 3000
#define max_ee_busy_loop 500


/*
 * ATMEL AT25XXXX Serial EEPROM 
 * access type
 */

/* Instruction codes */
#define WREN_CMD	0x06
#define WRDI_CMD	0x04
#define RDSR_CMD	0x05
#define WRSR_CMD	0x01
#define READ_CMD	0x03
#define WRITE_CMD	0x02

/* STATUS REGISTER BIT */
#define RDY 0	/*  Busy Indicator Bit */
#define WEN 1	/*  Write Enable Bit   */
#define BP0 2	/* Block Write Protect Bit */
#define BP1 3	/* Block Write Protect Bit */
#define WPEN 7	/* Software Write Protect Enable Bit */


#define ENABLE	1
#define DISABLE	0

#define CFG_CLK_DIV SPICFG_SPICLK_DIV8

#define RALINK_SYSCTL_ADDR		RALINK_SYSCTL_BASE	// system control
#define RALINK_REG_GPIOMODE		(RALINK_SYSCTL_ADDR + 0x60)

#endif	//__SPIC_H__
