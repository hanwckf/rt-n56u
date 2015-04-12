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
#ifndef __I2CDRV
#define __I2CDRV

#include <asm/rt2880/rt_mmap.h>

#define	RT2880_I2C_READ_STR		"read"	/* I2C read operation */
#define	RT2880_I2C_WRITE_STR		"write"	/* I2C read operation */

#define RT2880_I2C_DUMP			1
#define RT2880_I2C_READ			3
#define RT2880_I2C_WRITE		5
#define RT2880_I2C_SET_ADDR		7
#define RT2880_I2C_SET_ADDR_BYTES	9
#define RT2880_PCIE_PHY_READ		10
#define RT2880_PCIE_PHY_WRITE		8
#define RT2880_I2C_SET_CLKDIV		12

#define I2C_DEV_NAME			"i2cM0"

typedef struct i2c_write_data {
	unsigned long address;
	unsigned long value;
	unsigned long size;
} I2C_WRITE;


/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/

#define	RT2880_REG(x)			(*((volatile u32 *)(x)))
#define	RT2880_RSTCTRL_REG		(RALINK_SYSCTL_BASE+0x34)

#define RT2880_I2C_REG_BASE		(RALINK_I2C_BASE)
#define RT2880_I2C_CONFIG_REG		(RT2880_I2C_REG_BASE+0x00)
#define RT2880_I2C_CLKDIV_REG		(RT2880_I2C_REG_BASE+0x04)
#define RT2880_I2C_DEVADDR_REG		(RT2880_I2C_REG_BASE+0x08)
#define RT2880_I2C_ADDR_REG		(RT2880_I2C_REG_BASE+0x0C)
#define RT2880_I2C_DATAOUT_REG	 	(RT2880_I2C_REG_BASE+0x10)
#define RT2880_I2C_DATAIN_REG  		(RT2880_I2C_REG_BASE+0x14)
#define RT2880_I2C_STATUS_REG  		(RT2880_I2C_REG_BASE+0x18)
#define RT2880_I2C_STARTXFR_REG		(RT2880_I2C_REG_BASE+0x1C)
#define RT2880_I2C_BYTECNT_REG		(RT2880_I2C_REG_BASE+0x20)
#define RT2880_I2C_SM0_IS_AUTOMODE      (RT2880_I2C_REG_BASE+0x28)
#define RT2880_I2C_SM0CTL0              (RT2880_I2C_REG_BASE+0x40)

/* I2C_CFG register bit field */
#define I2C_CFG_ADDRLEN_8				(7<<5)	/* 8 bits */
#define I2C_CFG_DEVADLEN_7				(6<<2)	/* 7 bits */
#define I2C_CFG_ADDRDIS					(1<<1)	/* disable address transmission*/
#define I2C_CFG_DEVADDIS				(1<<0)	/* disable evice address transmission */

#define IS_BUSY		(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x01)
#define IS_SDOEMPTY	(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x02)
#define IS_DATARDY	(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x04)
#define IS_ACK		(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x08)
#define IS_STARTERR	(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x10)


/*
 * max SCLK : 400 KHz
 * CLKDIV < I2C_CLK / SCLK = I2C_CLK / 0.4
 */
#if defined (CONFIG_MTK_NFC_SUPPORT)
#if defined (CONFIG_RALINK_MT7621)
#define CLKDIV_VALUE	100
#elif defined (CONFIG_RALINK_RT3883)
#define CLKDIV_VALUE	50
#else
#error "chip is not support"
#endif
#else
#define CLKDIV_VALUE	333
#endif

#define i2c_busy_loop		(CLKDIV_VALUE*30)
#define max_ee_busy_loop	(CLKDIV_VALUE*25)
						  

/* 
 * AT24C01A/02/04/08A/16A (1K, 2K, 4K, 8K, 16K) 
 *	-- address : 8-bits
 * AT24C512 (512K)
 *  -- address : two 8-bits
 */    

/* 
 * sequential reads
 * because BYTECNT REG max 64 (6-bits)
 * , max READ_BLOCK is 64 
 */
#define READ_BLOCK		16

/*
 * AT24C01A/02 (1K, 2K)  have 8-byte Page
 */
#define WRITE_BLOCK		8


/*
 * ATMEL AT25XXXX Serial EEPROM 
 * access type
 */

/* Instruction codes */
#define READ_CMD	0x01
#define WRITE_CMD	0x00


#define I2C_CFG_DEFAULT		(I2C_CFG_ADDRLEN_8 | \
		I2C_CFG_DEVADLEN_7 | \
		I2C_CFG_ADDRDIS)

#define ATMEL_ADDR		(0xA0>>1)
#define WM8751_ADDR		(0x36>>1)
#define WM8960_ADDR		(0x34>>1)
#define MFI_AUTHIC_ADDR		(0x20>>1)

#endif
