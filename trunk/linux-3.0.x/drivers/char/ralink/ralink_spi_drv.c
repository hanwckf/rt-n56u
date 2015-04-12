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
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
#include <asm/system.h>
#endif
#include <linux/delay.h>
#include <linux/wireless.h>

#include <ralink/ralink_gpio.h>
#include <ralink/ralink_spi_drv.h>
#if defined (CONFIG_MAC_TO_MAC_MODE) || defined (CONFIG_P5_RGMII_TO_MAC_MODE)
#include "vtss.h"
#endif

int spidrv_major = 217;
static int spich = 0;

static u32 spi0_reg[] = { RT2880_SPI0_STAT_REG, RT2880_SPI0_CFG_REG, RT2880_SPI0_CTL_REG, RT2880_SPI0_DATA_REG } ;
static u32 spi1_reg[] = { RT2880_SPI1_STAT_REG, RT2880_SPI1_CFG_REG, RT2880_SPI1_CTL_REG, RT2880_SPI1_DATA_REG } ;

static u32* spi_register[2] = { spi0_reg, spi1_reg };

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_chip_select                                         */
/*    INPUTS: ENABLE or DISABLE                                         */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Pull on or Pull down #CS                                  */
/*                                                                      */
/*----------------------------------------------------------------------*/

static inline void spi_chip_select(u8 enable)
{
	int i;
	u32* spireg = spi_register[spich];

	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			/* low active */
			if (enable) {
				RT2880_REG(spireg[SPICTL]) |= SPICTL_SPIENA_ASSERT;
			} else  {
				RT2880_REG(spireg[SPICTL]) &= SPICTL_SPIENA_NEGATE;
			}		
			break;
		}
	}

#ifdef DBG
	if (i == spi_busy_loop) {
		printk("warning : spi_transfer (spi_chip_select) busy !\n");
	}
#endif
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_master_init                                         */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Initialize SPI block to desired state                     */
/*                                                                      */
/*----------------------------------------------------------------------*/
void spi_master_init(void)
{
	u32* spireg = spi_register[spich];

#if 0
	/* reset spi block */
	RT2880_REG(RT2880_RSTCTRL_REG) |= RSTCTRL_SPI_RESET;
	udelay(10);
	RT2880_REG(RT2880_RSTCTRL_REG) &= ~(RSTCTRL_SPI_RESET);
	udelay(10);
#endif

#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)||defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	RT2880_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN|(SPIARB_SPI1_ACTIVE_MODE<<1)| SPIARB_SPI0_ACTIVE_MODE;
#if defined(CONFIG_RALINK_MT7620)
	if (spich > 0)
		RT2880_REG(RT2880_SPI_ARB_REG) |= SPIARB_CS1CTL;
#endif
	RT2880_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;     //disable first
#endif
	RT2880_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;     //disable first

	RT2880_REG(spireg[SPICFG]) = SPICFG_MSBFIRST
					| SPICFG_RXCLKEDGE_FALLING
					| SPICFG_TXCLKEDGE_FALLING
					| SPICFG_SPICLK_DIV128 ;
	spi_chip_select(DISABLE);

#ifdef DBG
	printk("SPICFG = %08x\n", RT2880_REG(RT2880_SPICFG_REG));
	printk("is busy %d\n", IS_BUSY);
#endif
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_write                                               */
/*    INPUTS: 8-bit data                                                */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 8-bit data to SPI                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void spi_write(u8 data)
{
	int i;
	u32* spireg = spi_register[spich];

	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			RT2880_REG(spireg[SPIDATA]) = data;
			/* start write transfer */
			RT2880_REG(spireg[SPICTL]) = SPICTL_HIZSDO | SPICTL_STARTWR ;
			break;
		}
	}

#ifdef DBG
	if (i == spi_busy_loop) {
		printk("warning : spi_transfer (write %02x) busy !\n", data);
	}
#endif
}

#if defined (CONFIG_MAC_TO_MAC_MODE) || defined (CONFIG_P5_RGMII_TO_MAC_MODE)
//write32 MSB first
static void spi_write32(u32 data)
{
	u8 d0, d1, d2, d3;

	d0 = (u8)((data >> 24) & 0xff);
	d1 = (u8)((data >> 16) & 0xff);
	d2 = (u8)((data >> 8) & 0xff);
	d3 = (u8)(data & 0xff);

	spi_write(d0);
	spi_write(d1);
	spi_write(d2);
	spi_write(d3);
}
#endif

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_read                                                */
/*    INPUTS: None                                                      */
/*   RETURNS: 8-bit data                                                */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): get 8-bit data from SPI                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static u8 spi_read(void) 
{
	int i;
	u32* spireg = spi_register[spich];

	/*
	 * poll busy bit until it is 0 
	 * then start read transfer
	 */

	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			RT2880_REG(spireg[SPIDATA]) = 0;
			/* start read transfer */
			RT2880_REG(spireg[SPICTL]) = SPICTL_STARTRD ;
			break;
		}
	}

	/*
	 * poll busy bit until it is 0 
	 * then get data 
	 */
	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			break;
		}
	}

#ifdef DBG
	if (i == spi_busy_loop) {
		printk("warning : spi_transfer busy !\n");
	}
#endif

	return ((u8)RT2880_REG(spireg[SPIDATA]));
}

#if defined (CONFIG_MAC_TO_MAC_MODE) || defined (CONFIG_P5_RGMII_TO_MAC_MODE)
//read32 MSB first
static u32 spi_read32(void)
{
	u8 d0, d1, d2, d3;
	u32 ret;

	d0 = spi_read();
	d1 = spi_read();
	d2 = spi_read();
	d3 = spi_read();
	ret = (d0 << 24) | (d1 << 16) | (d2 << 8) | d3;

	return ret;
}
#endif

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_get_status_reg                                   */
/*    INPUTS: pointer to status                                         */
/*   RETURNS: None                                                      */
/*   OUTPUTS: status                                                    */
/*   NOTE(S): get the status of eeprom (AT25xxxx)                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void eeprom_get_status_reg(u8 *status) 
{
	spi_chip_select(ENABLE);
	spi_write(RDSR_CMD);
	*status = spi_read();
	spi_chip_select(DISABLE);
}


/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_read                                             */
/*    INPUTS: address - start address to be read                        */ 
/*            nbytes  - number of bytes to be read                      */
/*            dest    - pointer to read buffer                          */
/*   RETURNS: 0 - successful                                            */
/*            or eeprom status                                          */
/*   OUTPUTS: read buffer                                               */
/*   NOTE(S): If the eeprom is busy , the function returns with status  */
/*            register of eeprom                                        */
/*----------------------------------------------------------------------*/
unsigned char spi_eeprom_read(u16 address, u16 nbytes, u8 *dest)
{
	u8 status;
	u16 cnt = 0;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		return (status);

	/* eeprom ready */
	if (!(status & (1<<RDY))) {
		spi_chip_select(ENABLE);
		/* read op */
		spi_write(READ_CMD);
		spi_write((u8)(address >> 8));		/* MSB byte First */
		spi_write((u8)(address & 0x00FF));	/* LSB byte */

		while (cnt < nbytes) {
			*(dest++) = spi_read();
			cnt++;
		}
		status = 0;
		/* deassert cs */
		spi_chip_select(DISABLE);
	}
	return (status);
}


/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_write_enable                                     */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): always perform write enable  before any write operation   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void eeprom_write_enable(void)
{
	unsigned char	status;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		return;

	/* eeprom ready */
	if (!(status & (1<<RDY)))
	{	
		spi_chip_select(ENABLE);
		/* always write enable  before any write operation */
		spi_write(WREN_CMD);

		spi_chip_select(DISABLE);
		
		/* wait for write enable */
		do {
			eeprom_get_status_reg(&status);
		} while((status & (1<<RDY)) || !(status & (1<<WEN)));

	}

}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_write                                            */
/*    INPUTS: address - the first byte address to be written            */
/*            nbytes  - the number of bytes to be written               */
/*            src     - the pointer to source buffer                    */
/*   RETURNS: 0  - successful                                           */
/*            or eeprom buy status                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): The different eeprom has various write page size          */
/*            The function don't care write page size so the caller     */
/*            must check the page size of eeprom                        */
/*                                                                      */
/*----------------------------------------------------------------------*/
unsigned char spi_eeprom_write(u16 address, u16 nbytes, u8 *src)
{
	unsigned char	status;
	unsigned int	cnt = 0;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		goto done;


	/* eeprom ready */
	if (!(status & (1<<RDY))) {
		/* always write enable  before any write operation */
		eeprom_write_enable();

		spi_chip_select(ENABLE);
		spi_write(WRITE_CMD);
		spi_write((u8)(address >> 8));		/* MSB byte First */
		spi_write((u8)(address & 0x00FF));	/* LSB byte */

		while (cnt < nbytes) {
			spi_write(src[cnt]);
			cnt++;
		}
		status = 0;
		/* last byte sent then pull #cs high  */
		spi_chip_select(DISABLE);
	}

	i = 0;
	do {
		i++;
		eeprom_get_status_reg(&status);
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

done:
	return (status);
}

#if defined (CONFIG_MAC_TO_MAC_MODE) || defined (CONFIG_P5_RGMII_TO_MAC_MODE)
void spi_vtss_read(u8 blk, u8 subblk, u8 addr, u32 *value)
{
	u8 cmd;
#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
	spich = 1;
#else
	spich = 0;
#endif

	spi_master_init();
	cmd = (u8)((blk << 5) | subblk);
	spi_write(cmd);
	spi_write(addr);
	spi_read(); //dummy byte
	spi_read(); //dummy byte
	*value = spi_read32();

	//printf("rd %x:%x:%x = %x\n", blk, subblk, addr, *value);
	udelay(100);
}

void spi_vtss_write(u8 blk, u8 subblk, u8 addr, u32 value)
{
	u8 cmd;
#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
	spich = 1;
#else
	spich = 0;
#endif

	spi_master_init();
	cmd = (u8)((blk << 5) | subblk | 0x10);
	spi_write(cmd);
	spi_write(addr);
	spi_write32(value);

	//printf("wr %x:%x:%x = %x\n", blk, subblk, addr, value);
	udelay(10);
}

void vtss_reset(void)
{
#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
	spich = 1;
#else
	spich = 0;
#endif

#if defined (CONFIG_RT3883_ASIC)
	RT2880_REG(RALINK_REG_PIO3924DIR) |= 1;

	//Set Gpio pin 24 to low
	RT2880_REG(RALINK_REG_PIO3924DATA) &= ~1;

	mdelay(50);
	//Set Gpio pin 24 to high
	RT2880_REG(RALINK_REG_PIO3924DATA) |= 1;

	mdelay(125);
#elif defined (CONFIG_RT3052_ASIC)
	RT2880_REG(RALINK_REG_GPIOMODE) |= (7 << 2);
	RT2880_REG(RALINK_REG_GPIOMODE) &= ~(1 << 1);

	RT2880_REG(RALINK_REG_PIODIR) |= (1 << 12);
	RT2880_REG(RALINK_REG_PIODIR) &= ~(1 << 7);

	//Set Gpio pin 36 to low
	RT2880_REG(RALINK_REG_PIO3924DATA) &= ~(1 << 12);

	mdelay(50);
	//Set Gpio pin 36 to high
	RT2880_REG(RALINK_REG_PIO3924DATA) |= (1 << 12);

	mdelay(125);
#else
	RT2880_REG(RALINK_REG_GPIOMODE) |= (1 << 1);

	RT2880_REG(RALINK_REG_PIODIR) |= (1 << 10);

	//Set Gpio pin 10 to low
	RT2880_REG(RALINK_REG_PIODATA) &= ~(1 << 10);

	mdelay(50);
	//Set Gpio pin 10 to high
	RT2880_REG(RALINK_REG_PIODATA) |= (1 << 10);

	mdelay(125);
#endif
}

//type 0: no vlan, 1: vlan
void vtss_init(int type)
{
	int i, len;
	u32 tmp;
#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
	spich = 1;
#else
	spich = 0;
#endif

	//HT_WR(SYSTEM, 0, ICPU_CTRL, (1<<7) | (1<<3) | (1<<2) | (0<<0));
	//read it out to be sure the reset was done.
	while (1) {
		spi_vtss_write(7, 0, 0x10, (1<<7) | (1<<3) | (1<<2) | (0<<0));
		spi_vtss_read(7, 0, 0x10, &tmp);
		if (tmp & ((1<<7) | (1<<3) | (1<<2) | (0<<0)))
			break;
		udelay(1000);
	}

	//HT_WR(SYSTEM, 0, ICPU_ADDR, 0); //clear SP_SELECT and ADDR
	spi_vtss_write(7, 0, 0x11, 0);

	if (type == 1) {
		len = sizeof(lutonu_vlan_p0);
		for (i = 0; i < len; i++) {
			//HT_WR(SYSTEM, 0, ICPU_DATA, lutonu[i]);
			spi_vtss_write(7, 0, 0x12, lutonu_vlan_p0[i]);
		}
	}
	else if (type == 2) {
		len = sizeof(lutonu_vlan_p4);
		for (i = 0; i < len; i++) {
			//HT_WR(SYSTEM, 0, ICPU_DATA, lutonu[i]);
			spi_vtss_write(7, 0, 0x12, lutonu_vlan_p4[i]);
		}
	}
	else {
		len = sizeof(lutonu_novlan);
		for (i = 0; i < len; i++) {
			//HT_WR(SYSTEM, 0, ICPU_DATA, lutonu[i]);
			spi_vtss_write(7, 0, 0x12, lutonu_novlan[i]);
		}
	}

	//debug
	/*
	spi_vtss_write(7, 0, 0x11, 0);
	spi_vtss_read(7, 0, 0x11, &tmp);
	printk("ICPU_ADDR %x\n", tmp);
	for (i = 0; i < len; i++) {
		spi_vtss_read(7, 0, 0x12, &tmp);
		printk("%x ", tmp);
	}
	*/

	//HT_WR(SYSTEM, 0, GLORESET, (1<<0)); //MASTER_RESET
	spi_vtss_write(7, 0, 0x14, (1<<0));
	udelay(125000);

	//HT_WR(SYSTEM, 0, ICPU_CTRL, (1<<8) | (1<<3) | (1<<1) | (1<<0));
	spi_vtss_write(7, 0, 0x10, (1<<8) | (1<<3) | (1<<1) | (1<<0));
}
#endif


#if defined (CONFIG_RALINK_PCM) || defined (CONFIG_RALINK_PCM_MODULE)
static unsigned char high8bit (unsigned short data)
{
	return ((data>>8)&(0x0FF));
}

static unsigned char low8bit (unsigned short data)
{
	return ((unsigned char)(data & 0x00FF));
}

static unsigned short SixteenBit (unsigned char hi, unsigned char lo)
{
	unsigned short data = hi;
	data = (data<<8)|lo;
	return data;
}

void spi_si3220_master_init(int ch)
{
	int i;
	u32* spireg;
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	spich = 1;
#else
	spich = 0;
#endif
	spireg = spi_register[spich];
	spi_master_init();

	RT2880_REG(spireg[SPICFG]) = SPICFG_MSBFIRST |
					SPICFG_TXCLKEDGE_FALLING |
					SPICFG_SPICLK_DIV128 | SPICFG_SPICLKPOL;
	spi_chip_select(DISABLE);
#ifdef DBG
	printk("[slic]SPICFG = %08x\n", RT2880_REG(spireg[SPICFG]));
	printk("[slic]is busy %d\n", IS_BUSY);
#endif

#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* Set SPI_CS1_MODE to SPI_CS1 mode */
#if defined(CONFIG_RALINK_RT3883)
	RT2880_REG(RALINK_REG_GPIOMODE) &= 0xFFFFFFFD;
#else	
	RT2880_REG(RALINK_REG_GPIOMODE) &= 0xFF9FFFFD;
#endif
#else
	RT2880_REG(RALINK_REG_GPIOMODE) &= 0xFFFFFFFD; 
#endif
}

u8 spi_si3220_read8(int sid, unsigned char cid, unsigned char reg)
{
	u8 value;

	spi_si3220_master_init(sid);
	spi_chip_select(ENABLE);
	spi_write(cid);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	spi_write(reg);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	value = spi_read();
	spi_chip_select(DISABLE);

#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)||defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	RT2880_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN|(SPIARB_SPI1_ACTIVE_MODE<<1)| SPIARB_SPI0_ACTIVE_MODE;
	RT2880_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;
#endif
	RT2880_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;
	return value;
}

void spi_si3220_write8(int sid, unsigned char cid, unsigned char reg, unsigned char value)
{ 
	spi_si3220_master_init(sid);
	spi_chip_select(ENABLE);
	spi_write(cid);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	spi_write(reg);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	spi_write(value);
	spi_chip_select(DISABLE);

#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)||defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	RT2880_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN|(SPIARB_SPI1_ACTIVE_MODE<<1)| SPIARB_SPI0_ACTIVE_MODE;
	RT2880_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;
#endif
	RT2880_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;
}

unsigned short spi_si3220_read16(int sid, unsigned char cid, unsigned char reg)
{
	unsigned char hi, lo;

	spi_si3220_master_init(sid);
	spi_chip_select(ENABLE);
	spi_write(cid);
	spi_write(reg);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	hi = spi_read();
	lo = spi_read();
	spi_chip_select(DISABLE);

#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)||defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	RT2880_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN|(SPIARB_SPI1_ACTIVE_MODE<<1)| SPIARB_SPI0_ACTIVE_MODE;
	RT2880_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;
#endif
	RT2880_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;

	return SixteenBit(hi,lo);
}

void spi_si3220_write16(int sid, unsigned char cid, unsigned char reg, unsigned short value)
{
	unsigned char hi, lo;

	hi = high8bit(value);
	lo = low8bit(value);
	spi_si3220_master_init(sid);
	spi_chip_select(ENABLE);
	spi_write(cid);
	spi_write(reg);
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	spi_write(hi);
	spi_write(lo);
	spi_chip_select(DISABLE);

#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)||defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	RT2880_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN|(SPIARB_SPI1_ACTIVE_MODE<<1)| SPIARB_SPI0_ACTIVE_MODE;
	RT2880_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;
#endif
	RT2880_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;
}

void spi_si321x_master_init(int ch)
{
	u32* spireg;
#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	spich = 1;
#else
	spich = 0;
#endif
	spireg = spi_register[spich];
	spi_master_init();
	
	RT2880_REG(spireg[SPICFG]) =	SPICFG_MSBFIRST |
					SPICFG_SPICLK_DIV128 | SPICFG_SPICLKPOL |
					SPICFG_TXCLKEDGE_FALLING;
#ifdef DBG
	printk("[slic]SPICFG = %08x\n", RT2880_REG(spireg[SPICFG]));
	printk("[slic]is busy %d\n", IS_BUSY);
#endif

#if defined(CONFIG_RALINK_SLIC_CONNECT_SPI_CS1)
	/* Set SPI_CS1_MODE to SPI_CS1 mode */
#if defined(CONFIG_RALINK_RT3883)
	RT2880_REG(RALINK_REG_GPIOMODE) &= 0xFFFFFFFD;
#else	
	RT2880_REG(RALINK_REG_GPIOMODE) &= 0xFF9FFFFD;
#endif
#else
	RT2880_REG(RALINK_REG_GPIOMODE) &= 0xFFFFFFFD; 
#endif
}

u8 spi_si321x_read8(int sid, unsigned char cid, unsigned char reg)
{
	u8 value;
	spi_si321x_master_init(sid);
	spi_chip_select(ENABLE);
	spi_write(reg|0x80); 
	spi_chip_select(DISABLE);
	spi_chip_select(ENABLE);
	value = spi_read();
	spi_chip_select(DISABLE);
	return value;
}

void spi_si321x_write8(int sid, unsigned char cid, unsigned char reg, unsigned char value)
{
	spi_si321x_master_init(sid);
	spi_chip_select(ENABLE);
	spi_write(reg&0x7F); 
	spi_write(value);
	spi_chip_select(DISABLE);
	return;
}
#endif
#if defined (CONFIG_RALINK_PCM) || defined (CONFIG_RALINK_PCM_MODULE)
EXPORT_SYMBOL(spi_si3220_read8);
EXPORT_SYMBOL(spi_si3220_read16);
EXPORT_SYMBOL(spi_si3220_write8);
EXPORT_SYMBOL(spi_si3220_write16);
EXPORT_SYMBOL(spi_si321x_write8);
EXPORT_SYMBOL(spi_si321x_read8);
EXPORT_SYMBOL(spi_si3220_master_init);
EXPORT_SYMBOL(spi_si321x_master_init);
#endif

static long spidrv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned int address, value, size;
	int eeprom_spich = 0;
	SPI_WRITE *spi_wr;
#if defined (CONFIG_MAC_TO_MAC_MODE) || defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	SPI_VTSS *vtss;
	int vtss_spich = 0;

#if defined(CONFIG_RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
	vtss_spich = 1;
	eeprom_spich = 0;
#else
	vtss_spich = 0;
	eeprom_spich = 1;
#endif
#endif

	switch (cmd) {
	case RT2880_SPI_READ:
		value = 0; address = 0;
		spich = eeprom_spich;
		address = (unsigned int)arg;
		spi_master_init();
		spi_eeprom_read(address, 4, (unsigned char*)&value);
		//printk("0x%04x : 0x%08x\n", address, value);
		break;
	case RT2880_SPI_WRITE:
		spich = eeprom_spich;
		spi_wr = (SPI_WRITE*)arg;
		address = spi_wr->address;
		value   = spi_wr->value;
		size    = spi_wr->size;
		spi_master_init();
		spi_eeprom_write(address, size, (unsigned char*)&value);
		break;
#if defined (CONFIG_MAC_TO_MAC_MODE) || defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	case RT2880_SPI_INIT_VTSS_NOVLAN:
		spich = vtss_spich;
		vtss_reset();
		vtss_init(0);
		break;
	case RT2880_SPI_INIT_VTSS_WANATP0:
		spich = vtss_spich;
		vtss_reset();
		vtss_init(1);
		break;
	case RT2880_SPI_INIT_VTSS_WANATP4:
		spich = vtss_spich;
		vtss_reset();
		vtss_init(2);
		break;
	case RT2880_SPI_VTSS_READ:
		spich = vtss_spich;
		vtss = (SPI_VTSS *)arg;
		//printk("r %x %x %x -> ", vtss->blk, vtss->sub, vtss->reg);
		spi_vtss_read(vtss->blk, vtss->sub, vtss->reg, (u32*)&vtss->value);
		//printk("%x\n", (u32)vtss->value);
		break;
	case RT2880_SPI_VTSS_WRITE:
		spich = vtss_spich;
		vtss = (SPI_VTSS *)arg;
		//printk("w %x %x %x -> %x\n", vtss->blk, vtss->sub, vtss->reg, (u32)vtss->value);
		spi_vtss_write(vtss->blk, vtss->sub, vtss->reg, (u32)vtss->value);
		break;
#endif
	default:
		printk("spi_drv: command format error\n");
	}

	return 0;
}

static int spidrv_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int spidrv_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static const struct file_operations spidrv_fops =
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= spidrv_ioctl,
	.open		= spidrv_open,
	.release	= spidrv_release,
};

int __init spidrv_init(void)
{
	int result = register_chrdev(spidrv_major, SPI_DEV_NAME, &spidrv_fops);
	if (result < 0) {
		printk(KERN_WARNING "spi_drv: can't get major %d\n",spidrv_major);
		return result;
	}

	if (spidrv_major == 0) {
		spidrv_major = result; /* dynamic */
	}

	//use normal(SPI) mode instead of GPIO mode
	RT2880_REG(RALINK_REG_GPIOMODE) &= ~(RALINK_GPIOMODE_SPI);

	printk("Ralink SPI driver initialized.\n");

	return 0;
}

void __exit spidrv_exit(void)
{
	unregister_chrdev(spidrv_major, SPI_DEV_NAME);
}

module_init(spidrv_init);
module_exit(spidrv_exit);

module_param(spidrv_major, int, 0);

MODULE_DESCRIPTION("Ralink SPI Driver");
MODULE_LICENSE("GPL");
