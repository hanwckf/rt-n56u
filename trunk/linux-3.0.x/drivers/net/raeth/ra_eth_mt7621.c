#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/sched.h>

#include "ra_eth_reg.h"
#include "ra_eth.h"
#include "ra_phy.h"
#include "ra_esw_base.h"
#include "ra_gsw_mt7530.h"

extern u32 ralink_asic_rev_id;

#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_RGMII_AN) || \
    defined (CONFIG_GE2_INTERNAL_GPHY_P4)
static void ge2_int2_wq_handler(struct work_struct *work)
{
#if defined (CONFIG_GE2_INTERNAL_GPHY_P0)
	u32 port_id = 0;
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	u32 port_id = 4;
#else
	u32 port_id = 5;
#endif
	u32 link_state = sysRegRead(REG_ETH_GE2_MAC_STATUS);

#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	if (link_state & 0x1) {
		u32 link_speed = (link_state >> 2) & 0x3;
		mt7530_gsw_set_csr_delay((link_speed == 1) ? 1 : 0);
	}
#endif

	if (esw_link_status_hook)
		esw_link_status_hook(port_id, link_state & 0x1);
}

static DECLARE_WORK(ge2_int2_wq, ge2_int2_wq_handler);

void ge2_int2_schedule_wq(void)
{
	schedule_work(&ge2_int2_wq);
}

void ge2_int2_cancel_wq(void)
{
	cancel_work_sync(&ge2_int2_wq);
}
#endif

#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
void mt7621_apll_trgmii_enable(void)
{
#define REGBIT(x, n)	(x << n)

	u32 data = 0;
	u32 xtal;

	xtal = sysRegRead(REG_SYSCFG0);
	xtal = (xtal >> 6) & 0x7;

	/* Firstly, reset all required register to default value */
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0000, 0x00008000);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0014, 0x01401d61);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0018, 0x38233d0e);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, 0x80120004);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0020, 0x1c7dbf48);

	/* toggle RG_XPTL_CHG */
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0000, 0x00008800);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0000, 0x00008c00);

	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x0014);
	data &= ~(0x0000ffc0);
	if (xtal < 6) {
		// XTAL 20/40MHz
		data |= REGBIT(0x1, 6);		// PREDIV = 1/2
		data |= REGBIT(0x1d, 8);	// FBDIV = 30
	} else {
		// XTAL 25MHz
		data |= REGBIT(0x17, 8);	// FBDIV = 24
	}
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0014, data);

	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x0018);
	data &= ~(0xf0773f00);
	data |= REGBIT(0x3, 28);
	data |= REGBIT(0x2, 20);
	if (xtal < 6) {
		// XTAL 20/40MHz
		data |= REGBIT(0x3, 16);
	} else {
		// XTAL 25MHz
		data |= REGBIT(0x2, 16);
	}
	data |= REGBIT(0x3, 12);
	if (xtal < 6) {
		// XTAL 20/40MHz
		data |= REGBIT(0xd, 8);
	} else {
		// XTAL 25MHz
		data |= REGBIT(0x7, 8);
	}
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0018, data);

	if (xtal < 6) {
		// XTAL 20/40MHz
		data = 0x1c7dbf48;
	} else {
		// XTAL 25MHz
		data = 0x1697cc39;
	}
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0020, data);

	/* Common setting - Set PLLGP_CTRL_4 */
	/* 1. Bit 31 */
	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x001c);
	data &= ~(REGBIT(0x1, 31));
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, data);

	/* 2. Bit 0 */
	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 0);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, data);

	/* 3. Bit 3 */
	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 3);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, data);

	/* 4. Bit 8 */
	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 8);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, data);

	/* 5. Bit 6 */
	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 6);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, data);

	/* 6. Bit 7 */
	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 5);
	data |= REGBIT(0x1, 7);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, data);

	/* 7. Bit 17 */
	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x001c);
	data &= ~REGBIT(0x1, 17);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, data);
}
#endif

void mt7621_eth_gdma_vlan_pvid(int ge2, u32 vlan_id, u32 prio)
{
	u32 gdm_addr = (ge2) ? GDMA2_VLAN_GEN : GDMA1_VLAN_GEN;
	u32 gdm_vlan_gen = 0x81000000;

	vlan_id &= 0xfff;
	prio &= 0x7;

	if (vlan_id > 0)
		gdm_vlan_gen |= (prio << 13) | vlan_id;

	sysRegWrite(gdm_addr, gdm_vlan_gen);
}

void mt7621_eth_gdma_vlan_insv(int ge2, int insv_en)
{
	u32 gdm_addr = (ge2) ? GDMA2_FWD_CFG : GDMA1_FWD_CFG;
	u32 gdm_ig_ctrl;

	gdm_ig_ctrl = sysRegRead(gdm_addr);
	if (insv_en)
		gdm_ig_ctrl |=  BIT(25);
	else
		gdm_ig_ctrl &= ~BIT(25);
	sysRegWrite(gdm_addr, gdm_ig_ctrl);
}

void mt7621_eth_gdma_vlan_untag(int ge2, int untag_en)
{
	u32 gdm_addr = (ge2) ? GDMA2_SHPR_CFG : GDMA1_SHPR_CFG;
	u32 gdm_eg_ctrl;

	gdm_eg_ctrl = sysRegRead(gdm_addr);
	if (untag_en)
		gdm_eg_ctrl |=  BIT(30);
	else
		gdm_eg_ctrl &= ~BIT(30);
	sysRegWrite(gdm_addr, gdm_eg_ctrl);
}

void mt7621_eth_init(void)
{
	u32 reg_val;

#if defined (CONFIG_GE1_MII_AN) || defined (CONFIG_GE2_MII_AN)
	/* set MDIO clock to 2.500 MHz */
	sysRegWrite(REG_MDIO_PHY_POLLING, 0x45000504);
#elif defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_GE2_RGMII_AN)
	/* set MDIO clock to 4.167 MHz */
	sysRegWrite(REG_MDIO_PHY_POLLING, 0x43000504);
#else
	/* set MDIO clock to 6.250 MHz (MT7530/MCM support up to 12.5 MHz) */
	sysRegWrite(REG_MDIO_PHY_POLLING, 0x42000504);
#endif

	/* reduce RGMII2 and MDIO PAD driving strength */
	reg_val = sysRegRead(REG_PAD_RGMII2_MDIO_CFG);
	reg_val &= ~(0x3 << 20);
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	reg_val |=  (0x1 << 20);	// RGMII2 driving = 8mA
#else
	reg_val |=  (0x2 << 20);	// RGMII2 driving = 12mA
#endif
	reg_val &= ~(0x3 <<  4);	// MDIO driving = 2mA
	sysRegWrite(REG_PAD_RGMII2_MDIO_CFG, reg_val);

	/* Init GPHY first */
#if defined (CONFIG_GE1_RGMII_AN)
	ge1_set_mode(0, 1);
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR);
#endif
#if defined (CONFIG_GE2_RGMII_AN)
	ge2_set_mode(0, 1);
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2);
#endif

	/* MT7621 GE1 + Internal MCM (MT7530) GSW */
#if defined (CONFIG_GE1_RGMII_FORCE_1000) || defined (CONFIG_GE1_TRGMII_FORCE_1000) || defined (CONFIG_GE1_TRGMII_FORCE_1200)
	ge1_set_mode(0, 1);
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	ge2_set_mode(0, 1);
#endif
	/* GE1/GE2 force mode, Link Up, 1000Mbps, Full-Duplex, FC ON */
	reg_val = 0x2105e33b;

	/* MT7621 E2 has FC bug, disable FC */
	if ((ralink_asic_rev_id & 0xFFFF) == 0x0101)
		reg_val &= ~(0x3 << 4);
	sysRegWrite(REG_ETH_GE1_MAC_CONTROL, reg_val);

	/* init MT7530 via MDIO */
	mt7530_gsw_init();
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, reg_val);
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, 0x21056300);
#endif
#endif

	/* MT7621 GE2 + External GSW */
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	ge2_set_mode(0, 1);
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, 0x2105e33b);
#endif

	/* MT7621 GE1 + External PHY or CPU */
#if defined (CONFIG_GE1_RGMII_AN)
	sysRegWrite(REG_ETH_GE1_MAC_CONTROL, 0x21056300);
#elif defined (CONFIG_GE1_MII_AN)
	ge1_set_mode(1, 1);
	sysRegWrite(REG_ETH_GE1_MAC_CONTROL, 0x21056300);
#elif defined (CONFIG_GE1_MII_FORCE_100)
	ge1_set_mode(1, 0);
	sysRegWrite(REG_ETH_GE1_MAC_CONTROL, 0x0005e337);
#elif defined (CONFIG_GE1_RVMII_FORCE_100)
	ge1_set_mode(2, 0);
	sysRegWrite(REG_ETH_GE1_MAC_CONTROL, 0x0005e337);
#elif defined (CONFIG_GE1_RGMII_NONE)
	sysRegWrite(REG_ETH_GE1_MAC_CONTROL, 0x00008000);
#endif

	/* MT7621 GE2 + External PHY or CPU */
#if defined (CONFIG_GE2_RGMII_AN)
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, 0x21056300);
#elif defined (CONFIG_GE2_MII_AN)
	ge2_set_mode(1, 1);
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, 0x21056300);
#elif defined (CONFIG_GE2_MII_FORCE_100)
	ge2_set_mode(1, 0);
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, 0x0005e337);
#elif defined (CONFIG_GE2_RVMII_FORCE_100)
	ge2_set_mode(2, 0);
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, 0x0005e337);
#endif

#if !defined (CONFIG_RAETH_GMAC2)
	sysRegWrite(REG_ETH_GE2_MAC_CONTROL, 0x00008000);
#endif

#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_GE1_MII_AN) || \
    defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_GE2_MII_AN) || \
    defined (CONFIG_GE2_INTERNAL_GPHY_P0) || \
    defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	enable_autopoll_phy(1);
#endif
}

