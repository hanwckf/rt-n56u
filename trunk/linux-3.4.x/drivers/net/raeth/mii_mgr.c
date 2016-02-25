#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "ra_compat.h"
#include "ra_eth_reg.h"
#include "mii_mgr.h"

#define MDIO_TIMEOUT		(100 * 1000)

static DEFINE_SPINLOCK(mii_mgr_lock);

#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_MT7620) || \
    defined (CONFIG_RALINK_MT7621)
static int __mii_mgr_busy(u32 *mii_data)
{
	u32 i;
	u32 volatile status;

	for (i = 0; i < MDIO_TIMEOUT; i++) {
		status = sysRegRead(REG_MDIO_PHY_CONTROL_0);
		if (!(status & BIT(31))) {
			if (mii_data)
				*mii_data = (u32)(status & 0xffff);
			return 0;
		}
		ndelay(500);
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
#if defined (CONFIG_RALINK_RT3883)
	data = ((phy_addr & 0x1f) << 24) | ((phy_register & 0x1f) << 16);
#else
	data = ((phy_register & 0x1f) << 25) | ((phy_addr & 0x1f) << 20) | (0x02 << 18) | (0x01 << 16);
#endif
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);
	data |= BIT(31);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

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
#if defined (CONFIG_RALINK_RT3883)
	data = BIT(30) | ((phy_addr & 0x1f) << 24) | ((phy_register & 0x1f) << 16) | (write_data & 0xffff);
#else
	data = ((phy_register & 0x1f) << 25) | ((phy_addr & 0x1f) << 20) | (0x01 << 18) | (0x01 << 16) | (write_data & 0xffff);
#endif
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);
	data |= BIT(31);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

	/* make sure write operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "write");
		return 0;
	}

	return 1;
}

#else

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352)
static void enable_mdio(int enable)
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
#else
#define enable_mdio(x)
#endif

static int __mii_mgr_done(u32 *rd_data)
{
	u32 i;
	u32 volatile status;

	for (i = 0; i < MDIO_TIMEOUT; i++) {
		status = sysRegRead(REG_MDIO_PHY_CONTROL_1);
		if (rd_data) {
			/* check RD_RDY */
			if (status & BIT(1)) {
				*rd_data = (u32)(status >> 16);
				return 1;
			}
		} else {
			/* check WT_DONE */
			if (status & BIT(0))
				return 1;
		}
		ndelay(500);
	}

	return 0;
}

u32 __mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile data;

	enable_mdio(1);

	/* clear RD_RDY */
	data = sysRegRead(REG_MDIO_PHY_CONTROL_1);
	if (data & 0x03) {
		;
	}

	/* read 'clause 22' data */
	data = BIT(14) | ((phy_register & 0x1f) << 8) | (phy_addr & 0x1f);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

	/* make sure read operation is complete */
	if (!__mii_mgr_done(read_data)) {
		enable_mdio(0);
		printk("\n MDIO %s operation is ongoing and timeout!\n", "read");
		return 0;
	}

	enable_mdio(0);

	return 1;
}

u32 __mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	u32 volatile data;

	enable_mdio(1);

	/* clear WT_DONE */
	data = sysRegRead(REG_MDIO_PHY_CONTROL_1);
	if (data & 0x03) {
		;
	}

	/* write 'clause 22' data */
	data = ((write_data & 0xffff) << 16) | BIT(13) | ((phy_register & 0x1f) << 8) | (phy_addr & 0x1f);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

	/* make sure write operation is complete */
	if (!__mii_mgr_done(NULL)) {
		enable_mdio(0);
		printk("\n MDIO %s operation is ongoing and timeout!\n", "write");
		return 0;
	}

	enable_mdio(0);

	return 1;
}

#endif

#if defined (CONFIG_MT7530_GSW)
static u32 __mt7530_read(u32 gsw_reg, u32 *read_data)
{
	u32 result = 0;
	u32 lo_word = 0;
	u32 hi_word = 0;

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	/* check AN polling On */
	u32 an_state = sysRegRead(REG_MDIO_PHY_POLLING);
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state & ~BIT(31));
#endif

	// phase1: write page address phase
	if (__mii_mgr_write(MT7530_MDIO_ADDR, 0x1f, ((gsw_reg >> 6) & 0x3FF))) {
		// phase2: write address & read low word phase
		if (__mii_mgr_read(MT7530_MDIO_ADDR, (gsw_reg >> 2) & 0xF, &lo_word)) {
			// phase3: write address & read high word phase
			if (__mii_mgr_read(MT7530_MDIO_ADDR, (0x1 << 4), &hi_word)) {
				*read_data = (hi_word << 16) | (lo_word & 0xFFFF);
				result = 1;
			}
		}
	}

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state | BIT(31));
#endif

	return result;
}

static u32 __mt7530_write(u32 gsw_reg, u32 write_data)
{
	u32 result = 0;

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	/* check AN polling */
	u32 an_state = sysRegRead(REG_MDIO_PHY_POLLING);
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state & ~BIT(31));
#endif

	// phase1: write page address phase
	if (__mii_mgr_write(MT7530_MDIO_ADDR, 0x1f, (gsw_reg >> 6) & 0x3FF)) {
		// phase2: write address & write low word phase
		if (__mii_mgr_write(MT7530_MDIO_ADDR, ((gsw_reg >> 2) & 0xF), write_data & 0xFFFF)) {
			// phase3: write address & write high word phase
			if (__mii_mgr_write(MT7530_MDIO_ADDR, (0x1 << 4), write_data >> 16))
				result = 1;
		}
	}

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	/* restore AN polling */
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state | BIT(31));
#endif

	return result;
}
#endif

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 result;

	spin_lock(&mii_mgr_lock);

#if defined (CONFIG_MT7530_GSW)
	if (phy_addr == MT7530_MDIO_ADDR)
		result = __mt7530_read(phy_register, read_data);
	else
#endif
		result = __mii_mgr_read(phy_addr, phy_register, read_data);

	spin_unlock(&mii_mgr_lock);

	return result;
}

u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	u32 result;

	spin_lock(&mii_mgr_lock);

#if defined (CONFIG_MT7530_GSW)
	if (phy_addr == MT7530_MDIO_ADDR)
		result = __mt7530_write(phy_register, write_data);
	else
#endif
		result = __mii_mgr_write(phy_addr, phy_register, write_data);

	spin_unlock(&mii_mgr_lock);

	return result;
}

#if defined (RAETH_HW_CL45)

/* clause 45 pure on MT7621/MT7623 */
u32 mii_mgr_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data)
{
	u32 volatile data;
	u32 result = 0;
	u32 an_state;

	spin_lock(&mii_mgr_lock);

	/* check AN polling On */
	an_state = sysRegRead(REG_MDIO_PHY_POLLING);
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state & ~BIT(31));

	/* make sure previous operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing!\n", "read_cl45");
		goto cl45r_exit;
	}

	dev_addr &= 0x1f;
	port_num &= 0x1f;

	/* set 'clause 45' address (00) */
	data = (dev_addr << 25) | (port_num << 20) | (0x00 << 18) | (0x00 << 16) | (reg_addr & 0xffff);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);
	data |= BIT(31);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

	/* make sure addr operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "addr_cl45");
		goto cl45r_exit;
	}

	/* read 'clause 45' data (11) */
	data = (dev_addr << 25) | (port_num << 20) | (0x03 << 18) | (0x00 << 16);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);
	data |= BIT(31);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

	/* make sure read operation is complete */
	if (__mii_mgr_busy(read_data)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "read_cl45");
		goto cl45r_exit;
	}

	result = 1;

cl45r_exit:

	/* restore AN polling */
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state | BIT(31));

	spin_unlock(&mii_mgr_lock);

	return result;
}

u32 mii_mgr_write_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data)
{
	u32 volatile data;
	u32 result = 0;
	u32 an_state;

	spin_lock(&mii_mgr_lock);

	/* check AN polling On */
	an_state = sysRegRead(REG_MDIO_PHY_POLLING);
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state & ~BIT(31));

	/* make sure previous operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing!\n", "write_cl45");
		goto cl45w_exit;
	}

	dev_addr &= 0x1f;
	port_num &= 0x1f;

	/* set 'clause 45' address (00) */
	data = (dev_addr << 25) | (port_num << 20) | (0x00 << 18) | (0x00 << 16) | (reg_addr & 0xffff);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);
	data |= BIT(31);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

	/* make sure addr operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "addr_cl45");
		goto cl45w_exit;
	}

	/* write 'clause 45' data (01) */
	data = (dev_addr << 25) | (port_num << 20) | (0x01 << 18) | (0x00 << 16) | (write_data & 0xffff);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);
	data |= BIT(31);
	sysRegWrite(REG_MDIO_PHY_CONTROL_0, data);

	/* make sure write operation is complete */
	if (__mii_mgr_busy(NULL)) {
		printk("\n MDIO %s operation is ongoing and timeout!\n", "write_cl45");
		goto cl45w_exit;
	}

	result = 1;

cl45w_exit:

	/* restore AN polling */
	if (an_state & BIT(31))
		sysRegWrite(REG_MDIO_PHY_POLLING, an_state | BIT(31));

	spin_unlock(&mii_mgr_lock);

	return result;
}

#elif defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)

/* clause 45 via clause 22 */
u32 mii_mgr_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data)
{
	u32 result = 0;

	spin_lock(&mii_mgr_lock);

	dev_addr &= 0x1f;

	if (!__mii_mgr_write(port_num, 13, dev_addr))
		goto cl45r_exit;
	if (!__mii_mgr_write(port_num, 14, reg_addr))
		goto cl45r_exit;
	if (!__mii_mgr_write(port_num, 13, dev_addr | 0x6000))
		goto cl45r_exit;

	result = __mii_mgr_read(port_num, 14, read_data);

cl45r_exit:
	spin_unlock(&mii_mgr_lock);

	return result;
}

u32 mii_mgr_write_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data)
{
	u32 result = 0;

	spin_lock(&mii_mgr_lock);

	dev_addr &= 0x1f;

	if (!__mii_mgr_write(port_num, 13, dev_addr))
		goto cl45w_exit;
	if (!__mii_mgr_write(port_num, 14, reg_addr))
		goto cl45w_exit;
	if (!__mii_mgr_write(port_num, 13, dev_addr | 0x4000))
		goto cl45w_exit;

	result = __mii_mgr_write(port_num, 14, write_data);

cl45w_exit:
	spin_unlock(&mii_mgr_lock);

	return result;
}

#endif

u32 mii_mgr_init(void)
{
	/* early config MDIO port for external switch/PHY control */

#if defined (CONFIG_RALINK_RT3883)
	/* set MDIO clock to 4 MHz, disable PHY auto-polling */
#if defined (CONFIG_GE1_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#elif defined (CONFIG_GE1_RGMII_NONE)
	sysRegWrite(MDIO_CFG, 0x1d201);
#endif
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#elif defined (CONFIG_RALINK_MT7620)
	/* set MDIO clock to 4.167 MHz, disable PHY auto-polling */
	sysRegWrite(REG_MDIO_PHY_POLLING, 0x43000504);
#if !defined (CONFIG_RAETH_ESW)
	/* disable internal PHY 0~4, set internal PHY base address to 12 */
	sysRegWrite(RALINK_ETH_SW_BASE+0x7014, 0x1fec000c);
#endif
#elif defined (CONFIG_RALINK_MT7621)
	/* set MDIO clock to 4.167 MHz, disable PHY auto-polling */
	sysRegWrite(REG_MDIO_PHY_POLLING, 0x43000504);
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
