#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "ra_ethreg.h"
#include "mii_mgr.h"

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350)

#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + 0xC0)
#define MDIO_PHY_CONTROL_1	(RALINK_ETH_SW_BASE + 0xC4)
#define GPIO_MDIO_BIT		(1<<7)

#elif defined (CONFIG_RALINK_MT7628)

#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + 0xC0)
#define MDIO_PHY_CONTROL_1	(RALINK_ETH_SW_BASE + 0xC4)
#define enable_mdio(x)

#elif defined (CONFIG_RALINK_MT7620)

#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + 0x7004)

#elif defined (CONFIG_RALINK_MT7621)

#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + 0x0004)

#else /* RT288x, RT3883 */

#define MDIO_PHY_CONTROL_0	(RALINK_FRAME_ENGINE_BASE + 0x00)
#define MDIO_PHY_CONTROL_1	(RALINK_FRAME_ENGINE_BASE + 0x04)
#define GPIO_MDIO_BIT		(1<<7)
#define enable_mdio(x)

#endif

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350)
void enable_mdio(int enable)
{
#if !defined (CONFIG_P5_MAC_TO_PHY_MODE) && !defined(CONFIG_GE1_RGMII_AN) && !defined(CONFIG_GE2_RGMII_AN) && \
    !defined (CONFIG_GE1_MII_AN) && !defined (CONFIG_GE2_MII_AN)
	u32 data = sysRegRead(REG_GPIOMODE);
	if (enable)
		data &= ~GPIO_MDIO_BIT;
	else
		data |=  GPIO_MDIO_BIT;
	sysRegWrite(REG_GPIOMODE, data);
#endif
}
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
static u32 __mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile status = 0;
	u32 rc = 0;
	unsigned long volatile t_start = jiffies;
	u32 volatile data = 0;

	// make sure previous read operation is complete
	while (1) {
		if(!( sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			break;
		}
		else if (time_after(jiffies, t_start + 5*HZ)) {
			printk("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}

	data  = (0x01 << 16) | (0x02 << 18) | (phy_addr << 20) | (phy_register << 25);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	// make sure read operation is complete
	t_start = jiffies;
	while (1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			status = sysRegRead(MDIO_PHY_CONTROL_0);
			*read_data = (u32)(status & 0x0000FFFF);
			return 1;
		}
		else if (time_after(jiffies, t_start+5*HZ)) {
			printk("\n MDIO Read operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}

static u32 __mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	unsigned long volatile t_start = jiffies;
	u32 volatile data;

	// make sure previous write operation is complete
	while(1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			break;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			printk("\n MDIO Write operation ongoing\n");
			return 0;
		}
	}

	udelay(1);

	data = (0x01 << 16)| (1<<18) | (phy_addr << 20) | (phy_register << 25) | write_data;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data); //start operation

	t_start = jiffies;

	// make sure write operation is complete
	while (1) {
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			return 1;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			printk("\n MDIO Write operation Time Out\n");
			return 0;
		}
	}
}

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
#if defined (CONFIG_MT7530_GSW)
	if (phy_addr == MT7530_MDIO_ADDR) {
		u32 lo_word = 0;
		u32 hi_word = 0;
		// phase1: write page address phase
		if (__mii_mgr_write(phy_addr, 0x1f, ((phy_register >> 6) & 0x3FF))) {
			// phase2: write address & read low word phase
			if (__mii_mgr_read(phy_addr, (phy_register >> 2) & 0xF, &lo_word)) {
				// phase3: write address & read high word phase
				if (__mii_mgr_read(phy_addr, (0x1 << 4), &hi_word)) {
					*read_data = (hi_word << 16) | (lo_word & 0xFFFF);
					return 1;
				}
			}
		}
	
	} else
#endif
	{
		return __mii_mgr_read(phy_addr, phy_register, read_data);
	}

	return 0;
}

u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
#if defined (CONFIG_MT7530_GSW)
	if (phy_addr == MT7530_MDIO_ADDR) {
		// phase1: write page address phase
		if (__mii_mgr_write(MT7530_MDIO_ADDR, 0x1f, (phy_register >> 6) & 0x3FF)) {
			// phase2: write address & write low word phase
			if (__mii_mgr_write(MT7530_MDIO_ADDR, ((phy_register >> 2) & 0xF), write_data & 0xFFFF)) {
				// phase3: write address & write high word phase
				if (__mii_mgr_write(MT7530_MDIO_ADDR, (0x1 << 4), write_data >> 16))
					return 1;
			}
		}
	} else
#endif
	{
		return __mii_mgr_write(phy_addr, phy_register, write_data);
	}

	return 0;
}

#else

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile status = 0;
	u32 rc = 0;
	unsigned long volatile t_start = jiffies;
#if !defined (CONFIG_RALINK_RT3052) && !defined (CONFIG_RALINK_RT3352) && \
    !defined (CONFIG_RALINK_RT5350) && !defined (CONFIG_RALINK_MT7628)
	u32 volatile data = 0;
#endif

	/* We enable mdio gpio purpose register, and disable it when exit. */
	enable_mdio(1);

	// make sure previous read operation is complete
	while (1) {
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
		// rd_rdy: read operation is complete
		if(!(sysRegRead(MDIO_PHY_CONTROL_1) & (0x1 << 1)))
#else
		// 0 : Read/write operation complete
		if(!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
#endif
		{
			break;
		}
		else if (time_after(jiffies, t_start + 5*HZ)) {
			enable_mdio(0);
			printk("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	sysRegWrite(MDIO_PHY_CONTROL_0 , (1<<14) | (phy_register << 8) | (phy_addr));
#else
	data  = (phy_addr << 24) | (phy_register << 16);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
#endif

	// make sure read operation is complete
	t_start = jiffies;
	while (1) {
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
		if (sysRegRead(MDIO_PHY_CONTROL_1) & (0x1 << 1)) {
			status = sysRegRead(MDIO_PHY_CONTROL_1);
			*read_data = (u32)(status >>16);
			enable_mdio(0);
			return 1;
		}
#else
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			status = sysRegRead(MDIO_PHY_CONTROL_0);
			*read_data = (u32)(status & 0x0000FFFF);
			enable_mdio(0);
			return 1;
		}
#endif
		else if (time_after(jiffies, t_start+5*HZ)) {
			enable_mdio(0);
			printk("\n MDIO Read operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}

u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	unsigned long volatile t_start = jiffies;
	u32 volatile data;

	enable_mdio(1);

	// make sure previous write operation is complete
	while(1) {
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
		if (!(sysRegRead(MDIO_PHY_CONTROL_1) & (0x1 << 0)))
#else
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
#endif
		{
			break;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			enable_mdio(0);
			printk("\n MDIO Write operation ongoing\n");
			return 0;
		}
	}

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	data = ((write_data & 0xFFFF) << 16);
	data |= (phy_register << 8) | (phy_addr);
	data |= (1<<13);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
#else
	data = (1<<30) | (phy_addr << 24) | (phy_register << 16) | write_data;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data); //start operation
#endif

	t_start = jiffies;

	// make sure write operation is complete
	while (1) {
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
		if (sysRegRead(MDIO_PHY_CONTROL_1) & (0x1 << 0)) //wt_done ?= 1
#else
		if (!(sysRegRead(MDIO_PHY_CONTROL_0) & (0x1 << 31))) //0 : Read/write operation complete
#endif
		{
			enable_mdio(0);
			return 1;
		}
		else if (time_after(jiffies, t_start + 5 * HZ)) {
			enable_mdio(0);
			printk("\n MDIO Write operation Time Out\n");
			return 0;
		}
	}
}
#endif

u32 mii_mgr_init(void)
{
#if !defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_GE1_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#endif
	return 0;
}

EXPORT_SYMBOL(mii_mgr_init);
EXPORT_SYMBOL(mii_mgr_read);
EXPORT_SYMBOL(mii_mgr_write);
