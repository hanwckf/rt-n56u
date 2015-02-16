#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <linux/ralink_gpio.h>

#include "ra_ethreg.h"
#include "mii_mgr.h"
#include "ra_esw_mt7621.h"
#include "ra_esw_mt7620.h"
#include "ra_esw_rt305x.h"

/*  PHY Vender ID list */
#define EV_MARVELL_PHY_ID0		0x0141
#define EV_MARVELL_PHY_ID1		0x0CC2

#define EV_VTSS_PHY_ID0			0x0007
#define EV_VTSS_PHY_ID1			0x0421

#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE) || \
    defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_P4_MAC_TO_PHY_MODE)
static int is_gigaphy_id(u32 phy_addr, u32 check_phy_id0, u32 check_phy_id1)
{
	u32 phy_id0 = 0, phy_id1 = 0;

	if (!mii_mgr_read(phy_addr, 2, &phy_id0))
		phy_id0 = 0;
	if (!mii_mgr_read(phy_addr, 3, &phy_id1))
		phy_id1 = 0;

	if ((phy_id0 == check_phy_id0) && (phy_id1 == check_phy_id1))
		return 1;

	return 0;
}

void init_giga_phy(int ge)
{
	u32 phy_val = 0;
#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	u32 phy_addr = (ge == 2) ? CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 : CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#else
	u32 phy_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif

	if (is_gigaphy_id(phy_addr, EV_MARVELL_PHY_ID0, EV_MARVELL_PHY_ID1)) {
		printk("%s GigaPHY is found!\n", "Marvell");
		mii_mgr_read(phy_addr, 20, &phy_val);
		phy_val |= (1<<7);			// add delay to RX_CLK for RXD Outputs
		mii_mgr_write(phy_addr, 20, phy_val);
		mii_mgr_read(phy_addr, 0, &phy_val);
		phy_val |= (1<<15);			// PHY Software Reset
		mii_mgr_write(phy_addr, 0, phy_val);
	} else if (is_gigaphy_id(phy_addr, EV_VTSS_PHY_ID0, EV_VTSS_PHY_ID1)) {
		printk("%s GigaPHY is found!\n", "Vitesse");
		mii_mgr_write(phy_addr, 31, 0x0001);	// extended page
		mii_mgr_read(phy_addr, 28, &phy_val);
		phy_val |=  (0x3<<12);			// RGMII RX skew compensation= 2.0 ns
		phy_val &= ~(0x3<<14);			// RGMII TX skew compensation= 0 ns
		mii_mgr_write(phy_addr, 28, phy_val);
		mii_mgr_write(phy_addr, 31, 0x0000);	// main registers
	}
}
#endif

#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR) || defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7620)
void enable_autopoll_phy(int unused)
{
	u32 regValue, addr_s, addr_e;

#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR) && defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
#if (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 > CONFIG_MAC_TO_GIGAPHY_MODE_ADDR)
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#else
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif
#elif defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2+1;
#else
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2-1;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#endif
#else
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR-1;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif

	regValue = sysRegRead(REG_ESW_PHY_POLLING);
	regValue |= (1UL<<31);
	regValue &= ~(0x1f);
	regValue &= ~(0x1f<<8);
	regValue |= (addr_s & 0x1f);		// setup PHY address for auto polling (Start Addr).
	regValue |= ((addr_e & 0x1f) << 8);	// setup PHY address for auto polling (End Addr).
	sysRegWrite(REG_ESW_PHY_POLLING, regValue);
}
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
      defined (CONFIG_RALINK_RT3883)
void enable_autopoll_phy(int ge)
{
	u32 regAddr, regValue;

#if defined (CONFIG_RALINK_RT3883)
	regAddr = MDIO_CFG;
#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	if (ge == 2)
		regAddr = MDIO_CFG2;
#endif
#else
	regAddr = RALINK_ETH_SW_BASE + 0xC8;
#endif

	regValue = sysRegRead(regAddr);
	regValue &= 0xe0ff7fff;			// clear auto polling related field: (MD_PHY1ADDR & GP1_FRC_EN).
	regValue |= 0x20000000;			// force to enable MDC/MDIO auto polling.
#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	if (ge == 2)
		regValue |= ((CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 & 0x1f) << 24);	// setup PHY address for auto polling.
#endif
#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR)
	if (ge != 2)
		regValue |= ((CONFIG_MAC_TO_GIGAPHY_MODE_ADDR & 0x1f) << 24);	// setup PHY address for auto polling.
#endif
	sysRegWrite(regAddr, regValue);
}
#endif
#endif

#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
void ge1_set_mode(int ge_mode, int need_mdio)
{
	u32 reg_cfg1 = sysRegRead(REG_SYSCFG1);
	u32 reg_gpio = sysRegRead(RALINK_REG_GPIOMODE);

	reg_gpio &= ~(RALINK_GPIOMODE_GE1);		// GE1=Normal mode
	reg_cfg1 &= ~(0x3 << 12);			// GE1_MODE=RGMii Mode
	switch (ge_mode)
	{
	case 2:
		reg_cfg1 |= (0x2 << 12);		// GE1_MODE=RvMii Mode
		break;
	case 1:
		reg_cfg1 |= (0x1 << 12);		// GE1_MODE=Mii Mode
		break;
	}
	if (need_mdio)
		reg_gpio &= ~(RALINK_GPIOMODE_MDIO);	// MDIO=Normal mode
	sysRegWrite(REG_SYSCFG1, reg_cfg1);
	sysRegWrite(RALINK_REG_GPIOMODE, reg_gpio);
}
#endif

#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_MT7620) || \
    defined (CONFIG_RALINK_MT7621)
void ge2_set_mode(int ge_mode, int need_mdio)
{
	u32 reg_cfg1 = sysRegRead(REG_SYSCFG1);
	u32 reg_gpio = sysRegRead(RALINK_REG_GPIOMODE);

	reg_gpio &= ~(RALINK_GPIOMODE_GE2);		// GE2=Normal mode
	reg_cfg1 &= ~(0x3 << 14);			// GE2_MODE=RGMii Mode
	switch (ge_mode)
	{
	case 3:
		reg_gpio |= (RALINK_GPIOMODE_GE2);	// GE2=GPIO mode
		reg_cfg1 |= (0x3 << 14);		// GE2_MODE=RJ-45 Mode (MT7620)
		break;
	case 2:
		reg_cfg1 |= (0x2 << 14);		// GE2_MODE=RvMii Mode
		break;
	case 1:
		reg_cfg1 |= (0x1 << 14);		// GE2_MODE=Mii Mode
		break;
	}
	if (need_mdio)
		reg_gpio &= ~(RALINK_GPIOMODE_MDIO);	// MDIO=Normal mode
	sysRegWrite(REG_SYSCFG1, reg_cfg1);
	sysRegWrite(RALINK_REG_GPIOMODE, reg_gpio);
}
#endif

void fe_phy_init(void)
{
	/* Case1: RT305x/RT335x/RT5350/MT7620/MT7628 + ESW/GSW/P5/P4 */
#if defined (CONFIG_RALINK_MT7620)
	mt7620_esw_init();
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
      defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	rt305x_esw_init();
#endif

	/* Case2: RT3883/MT7621 GE1 + GSW */
#if defined (CONFIG_GE1_RGMII_FORCE_1000) || defined (CONFIG_GE1_TRGMII_FORCE_1200)
#if defined (CONFIG_RALINK_MT7621)
	/* MT7621 GE1 + Internal GSW */
	ge1_set_mode(0, 1);
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	/* MT7621 GE2 + Internal GSW */
	ge2_set_mode(0, 1);
	*(volatile u32 *)(REG_PAD_RGMII2_MDIO_CFG) &= ~(0x3 << 4);	// reduce RGMII2 PAD driving strength
#endif
	mt7621_esw_init();
#else
	/* RT3883 GE1 + External GSW (MDIO mode set by mii_mgr_init) */
	ge1_set_mode(0, 0);
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#endif

	/* Case3: RT3883/MT7621 GE2 + External GSW */
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
#if defined (CONFIG_RALINK_MT7621)
	/* MT7621 GE2 + External GSW */
	ge2_set_mode(0, 1);
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x2005e33b);		// (GE2, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#else
	/* RT3883 GE2 + External GSW (MDIO mode set by mii_mgr_init) */
	ge2_set_mode(0, 0);
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#endif

	/* Case4: RT3883/MT7621 GE1 + GigaPhy */
#if defined (CONFIG_GE1_RGMII_AN)
	ge1_set_mode(0, 1);
#if defined (CONFIG_RALINK_MT7621)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x20056300);		// (GE1, AN)
#endif
	init_giga_phy(1);
	enable_autopoll_phy(1);
#endif

	/* Case5: RT3883/MT7621 GE2 + GigaPhy */
#if defined (CONFIG_GE2_RGMII_AN)
	ge2_set_mode(0, 1);
#if defined (CONFIG_RALINK_MT7621)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x20056300);		// (GE2, AN)
#endif
	init_giga_phy(2);
	enable_autopoll_phy(2);
#endif

	/* Case6: RT3883, MT7621 GE1/GE2 + (10/100 Switch or 100PHY) */
#if defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY)

#if defined (CONFIG_GE1_MII_AN)
	ge1_set_mode(1, 1);
#elif defined (CONFIG_GE1_MII_FORCE_100)
	ge1_set_mode(1, 0);
#elif defined (CONFIG_GE1_RVMII_FORCE_100)
	ge1_set_mode(2, 0);
#endif
#if defined (CONFIG_GE2_MII_AN)
	ge2_set_mode(1, 1);
#elif defined (CONFIG_GE2_MII_FORCE_100)
	ge2_set_mode(1, 0);
#elif defined (CONFIG_GE2_RVMII_FORCE_100)
	ge2_set_mode(2, 0);
#endif

#if defined (CONFIG_RALINK_RT3883)
#if defined (CONFIG_GE1_MII_FORCE_100)
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_100_FD);
#endif
#if defined (CONFIG_GE2_MII_FORCE_100)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_100_FD);
#endif
#if defined (CONFIG_GE1_MII_AN)
	enable_autopoll_phy(1);
#endif
#if defined (CONFIG_GE2_MII_AN)
	enable_autopoll_phy(2);
#endif
#elif defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_GE1_MII_FORCE_100)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x0005e337);		// (GE1, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#endif
#if defined (CONFIG_GE2_MII_FORCE_100)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x0005e337);		// (GE2, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#endif
#if defined (CONFIG_GE1_MII_AN)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x20056300);		// (GE1, AN)
#endif
#if defined (CONFIG_GE2_MII_AN)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x20056300);		// (GE2, AN)
#endif
#if defined (CONFIG_GE1_MII_AN) || defined (CONFIG_GE2_MII_AN)
	enable_autopoll_phy(1);
#endif
#endif /* CONFIG_RALINK_MT7621 */

#endif /* CONFIG_RAETH_ROUTER || CONFIG_100PHY */

#if defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_GE1_RGMII_NONE)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x00008000);		// (GE1, Link down)
#endif
#if !defined (CONFIG_RAETH_GMAC2)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x00008000);		// (GE2, Link down)
#endif
#endif
}

