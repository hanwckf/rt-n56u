#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "ra_ethreg.h"
#include "mii_mgr.h"

#define MDIO_TIMEOUT_US		50000

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)

#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + 0xC0)
#define MDIO_PHY_CONTROL_1	(RALINK_ETH_SW_BASE + 0xC4)
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
#define enable_mdio(x)
#endif

#elif defined (CONFIG_RALINK_MT7620)

#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + 0x7004)

#elif defined (CONFIG_RALINK_MT7621)

#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + 0x0004)

#else /* RT288x, RT3883 */

#define MDIO_PHY_CONTROL_0	(RALINK_FRAME_ENGINE_BASE + 0x00)
#define MDIO_PHY_CONTROL_1	(RALINK_FRAME_ENGINE_BASE + 0x04)
#define enable_mdio(x)

#endif

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352)
void enable_mdio(int enable)
{
	/* do not play with MDIO when autopoll enabled */
#if !defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR) && !defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	u32 data = sysRegRead(RALINK_REG_GPIOMODE);
	if (enable)
		data &= ~RALINK_GPIOMODE_MDIO;
	else
		data |=  RALINK_GPIOMODE_MDIO;
	sysRegWrite(RALINK_REG_GPIOMODE, data);
#endif
}
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
static DEFINE_SPINLOCK(mii_mgr_lock);

static int __mii_mgr_busy(u32 *mii_data)
{
	u32 i;
	u32 volatile status;

	for (i = 0; i < MDIO_TIMEOUT_US; i++) {
		status = sysRegRead(MDIO_PHY_CONTROL_0);
		if (!(status & (1UL << 31))) {
			if (mii_data)
				*mii_data = (u32)(status & 0x0000FFFF);
			return 0;
		}
		udelay(2);
	}

	return 1;
}

/* clause 22 */
static u32 __mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile data;

	/* make sure previous operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing!\n", "read");
		return 0;
	}

	/* read 'clause 22' data */
	data = (phy_register << 25) | (phy_addr << 20) | (0x02 << 18) | (0x01 << 16);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1UL<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	/* make sure read operation is complete */
	if (__mii_mgr_busy(read_data)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "read");
		return 0;
	}

	return 1;
}

static u32 __mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	u32 volatile data;

	/* make sure previous operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing!\n", "write");
		return 0;
	}

	/* write 'clause 22' data */
	data = (phy_register << 25) | (phy_addr << 20) | (0x01 << 18) | (0x01 << 16) | write_data;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1UL<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	/* make sure write operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "write");
		return 0;
	}

	return 1;
}

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	unsigned long flags;
	u32 result = 0;

	spin_lock_irqsave(&mii_mgr_lock, flags);
#if defined (CONFIG_MT7530_GSW)
	if (phy_addr == MT7530_MDIO_ADDR) {
		u32 lo_word = 0;
		u32 hi_word = 0;
		u32 an_state = sysRegRead(REG_ESW_PHY_POLLING);
		
		/* check AN polling On */
		if (an_state & (1UL<<31))
			sysRegWrite(REG_ESW_PHY_POLLING, an_state & ~(1UL<<31));
		
		// phase1: write page address phase
		if (__mii_mgr_write(phy_addr, 0x1f, ((phy_register >> 6) & 0x3FF))) {
			// phase2: write address & read low word phase
			if (__mii_mgr_read(phy_addr, (phy_register >> 2) & 0xF, &lo_word)) {
				// phase3: write address & read high word phase
				if (__mii_mgr_read(phy_addr, (0x1 << 4), &hi_word)) {
					*read_data = (hi_word << 16) | (lo_word & 0xFFFF);
					result = 1;
				}
			}
		}
		
		if (an_state & (1UL<<31))
			sysRegWrite(REG_ESW_PHY_POLLING, an_state | (1UL<<31));
	} else
#endif
	{
		result = __mii_mgr_read(phy_addr, phy_register, read_data);
	}

	spin_unlock_irqrestore(&mii_mgr_lock, flags);

	return result;
}

u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	unsigned long flags;
	u32 result = 0;

	spin_lock_irqsave(&mii_mgr_lock, flags);
#if defined (CONFIG_MT7530_GSW)
	if (phy_addr == MT7530_MDIO_ADDR) {
		u32 an_state = sysRegRead(REG_ESW_PHY_POLLING);
		
		/* check AN polling */
		if (an_state & (1UL<<31))
			sysRegWrite(REG_ESW_PHY_POLLING, an_state & ~(1UL<<31));
		
		// phase1: write page address phase
		if (__mii_mgr_write(MT7530_MDIO_ADDR, 0x1f, (phy_register >> 6) & 0x3FF)) {
			// phase2: write address & write low word phase
			if (__mii_mgr_write(MT7530_MDIO_ADDR, ((phy_register >> 2) & 0xF), write_data & 0xFFFF)) {
				// phase3: write address & write high word phase
				if (__mii_mgr_write(MT7530_MDIO_ADDR, (0x1 << 4), write_data >> 16))
					result = 1;
			}
		}
		
		if (an_state & (1UL<<31))
			sysRegWrite(REG_ESW_PHY_POLLING, an_state | (1UL<<31));
	} else
#endif
	{
		result = __mii_mgr_write(phy_addr, phy_register, write_data);
	}

	spin_unlock_irqrestore(&mii_mgr_lock, flags);

	return result;
}

#if defined (CONFIG_RALINK_MT7621)
/* clause 45 */
u32 mii_mgr_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data)
{
	unsigned long flags;
	u32 volatile data;
	u32 result = 0;

	spin_lock_irqsave(&mii_mgr_lock, flags);

	/* make sure previous operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing!\n", "read_cl45");
		goto cl45r_exit;
	}

	/* set 'clause 45' address (00) */
	data = (dev_addr << 25) | (port_num << 20) | (0x00 << 18) | (0x00 << 16) | reg_addr;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1UL<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	/* make sure addr operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "addr_cl45");
		goto cl45r_exit;
	}

	/* read 'clause 45' data (11) */
	data = (dev_addr << 25) | (port_num << 20) | (0x03 << 18) | (0x00 << 16);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1UL<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	/* make sure read operation is complete */
	if (__mii_mgr_busy(read_data)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "read_cl45");
		goto cl45r_exit;
	}

	result = 1;

cl45r_exit:
	spin_unlock_irqrestore(&mii_mgr_lock, flags);

	return result;
}

u32 mii_mgr_write_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data)
{
	unsigned long flags;
	u32 volatile data;
	u32 result = 0;

	spin_lock_irqsave(&mii_mgr_lock, flags);

	/* make sure previous operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing!\n", "write_cl45");
		goto cl45w_exit;
	}

	/* set 'clause 45' address (00) */
	data = (dev_addr << 25) | (port_num << 20) | (0x00 << 18) | (0x00 << 16) | reg_addr;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1UL<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	/* make sure addr operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "addr_cl45");
		goto cl45w_exit;
	}

	/* write 'clause 45' data (01) */
	data = (dev_addr << 25) | (port_num << 20) | (0x01 << 18) | (0x00 << 16) | write_data;
	sysRegWrite(MDIO_PHY_CONTROL_0, data);
	data |= (1UL<<31);
	sysRegWrite(MDIO_PHY_CONTROL_0, data);

	/* make sure write operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "write_cl45");
		goto cl45w_exit;
	}

	result = 1;

cl45w_exit:
	spin_unlock_irqrestore(&mii_mgr_lock, flags);

	return result;
}
#endif

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
	/* early config MDIO port for external switch control */

#if defined (CONFIG_RALINK_RT3883)
#if defined (CONFIG_GE1_RGMII_FORCE_1000)
	/* set MDIO clock to 4 MHz, disable PHY auto-polling */
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#elif defined (CONFIG_RALINK_MT7620)
	/* set MDIO clock to 4.167 MHz, disable PHY auto-polling */
	sysRegWrite(REG_ESW_PHY_POLLING, 0x43000504);
#if !defined (CONFIG_RAETH_ESW)
	/* disable internal PHY 0~4, set internal PHY base address to 12 */
	sysRegWrite(RALINK_ETH_SW_BASE+0x7014, 0x1fec000c);
#endif
#elif defined (CONFIG_RALINK_MT7621)
	/* set MDIO clock to 4.167 MHz, disable PHY auto-polling */
	sysRegWrite(REG_ESW_PHY_POLLING, 0x43000504);
#endif

	/* set MDIO pins to Normal mode */
#if defined (RALINK_GPIOMODE_MDIO)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;
#endif

	return 0;
}

EXPORT_SYMBOL(mii_mgr_init);
EXPORT_SYMBOL(mii_mgr_read);
EXPORT_SYMBOL(mii_mgr_write);
