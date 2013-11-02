/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************
 *
 */
#ifndef __SPIDRV
#define __SPIDRV

#include <asm/rt2880/rt_mmap.h>

#define	RT2880_SPI_DUMP_STR		"dump"	/* Dump Content Command Prompt    */
#define	RT2880_SPI_READ_STR		"read"	/* SPI read operation */
#define	RT2880_SPI_WRITE_STR		"write"	/* SPI read operation */

#define RT2880_SPI_DUMP        2
#define RT2880_SPI_READ        3
#define RT2880_SPI_WRITE       5
#define RT2880_SPI_INIT_VTSS_NOVLAN   7
#define RT2880_SPI_INIT_VTSS_WANATP0  9
#define RT2880_SPI_INIT_VTSS_WANATP4  10
#define RT2880_SPI_VTSS_READ   11
#define RT2880_SPI_VTSS_WRITE  13

#define SPI_DEV_NAME	"spiS0"

typedef struct spi_write_data {
	unsigned long address;
	unsigned long value;
	unsigned long size;
} SPI_WRITE;

typedef struct spi_vtss_data {
	unsigned int blk;
	unsigned int sub;
	unsigned int reg;
	unsigned long value;
} SPI_VTSS;

/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/
#define RT2880_REG(x)		(*((volatile u32 *)(x)))

#define RT2880_RSTCTRL_REG		(RALINK_SYSCTL_BASE+0x34)
#define RSTCTRL_SPI_RESET		RALINK_SPI_RST

#define RT2880_SPI_REG_BASE		(RALINK_SPI_BASE)
#define RT2880_SPI0_STAT_REG	(RT2880_SPI_REG_BASE+0x00)
#define RT2880_SPI0_CFG_REG		(RT2880_SPI_REG_BASE+0x10)
#define RT2880_SPI0_CTL_REG		(RT2880_SPI_REG_BASE+0x14)
#define RT2880_SPI0_DATA_REG	(RT2880_SPI_REG_BASE+0x20)

#define RT2880_SPI1_STAT_REG	(RT2880_SPI_REG_BASE+0x40)
#define RT2880_SPI1_CFG_REG		(RT2880_SPI_REG_BASE+0x50)
#define RT2880_SPI1_CTL_REG		(RT2880_SPI_REG_BASE+0x54)
#define RT2880_SPI1_DATA_REG	(RT2880_SPI_REG_BASE+0x60)

#define RT2880_SPI_ARB_REG		(RT2880_SPI_REG_BASE+0xf0)

#define	SPISTAT		0
#define SPICFG		1
#define SPICTL		2
#define SPIDATA		3	

/* SPIARB register bit field */
#define SPIARB_ARB_EN			(1<<31)
#define SPIARB_CS1CTL			(1<<16)

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

/* SPICFG register bit field */
#define SPICFG_LSBFIRST				(0<<8)
#define SPICFG_MSBFIRST				(1<<8)

#define SPICFG_RXCLKEDGE_FALLING	(1<<5)		/* rx on the falling edge of the SPICLK signal */
#define SPICFG_TXCLKEDGE_FALLING	(1<<4)		/* tx on the falling edge of the SPICLK signal */

#define SPICFG_HIZSPI				(1<<3)

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
#define SPICTL_SPIENA_NEGATE		(~(1<<0))
#define SPICTL_SPIENA_ASSERT		(1<<0)

#define IS_BUSY				(RT2880_REG(spireg[SPISTAT]) & 0x01)

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

#endif
