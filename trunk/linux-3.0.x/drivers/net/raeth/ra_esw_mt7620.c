#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/string.h>

#include "ra_eth_reg.h"
#include "ra_esw_reg.h"
#include "mii_mgr.h"
#include "ra_eth.h"
#include "ra_phy.h"
#include "ra_gsw_mt7530.h"
#include "ra_esw_mt7620.h"

extern u32 ralink_asic_rev_id;

#if defined (CONFIG_RAETH_ESW)

#if defined (CONFIG_RAETH_HAS_PORT4)
#define MAX_ESW_PHY_ID	3
#else
#define MAX_ESW_PHY_ID	4
#endif

static void mt7620_ephy_init(void)
{
	u32 i;
	u32 is_BGA = (ralink_asic_rev_id >> 16) & 0x1;

	/* PCIE_RC_MODE=1 */
	*(volatile u32 *)(REG_SYSCFG1) |= (0x1 << 8);

	/* keep rx/tx port clock ticking, disable internal clock-gating to avoid switch stuck */
	*(volatile u32 *)(CKGCR) &= ~(0x3 << 4);

	/*
	* Reg 31: Page Control
	* Bit 15     => PortPageSel, 1=local, 0=global
	* Bit 14:12  => PageSel, local:0~3, global:0~4
	*
	* Reg16~30:Local/Global registers
	*
	*/

	/* correct PHY setting L3.0 BGA */
	mii_mgr_write(1, 31, 0x4000); //global, page 4

	mii_mgr_write(1, 17, 0x7444);
	if (is_BGA)
		mii_mgr_write(1, 19, 0x0114);
	else
		mii_mgr_write(1, 19, 0x0117);

	mii_mgr_write(1, 22, 0x10cf);
	mii_mgr_write(1, 25, 0x6212);
	mii_mgr_write(1, 26, 0x0777);
	mii_mgr_write(1, 29, 0x4000);
	mii_mgr_write(1, 28, 0xc077);
	mii_mgr_write(1, 24, 0x0000);

	mii_mgr_write(1, 31, 0x3000); //global, page 3
	mii_mgr_write(1, 17, 0x4838);

	mii_mgr_write(1, 31, 0x2000); //global, page 2
	if (is_BGA) {
		mii_mgr_write(1, 21, 0x0515);
		mii_mgr_write(1, 22, 0x0053);
		mii_mgr_write(1, 23, 0x00bf);
		mii_mgr_write(1, 24, 0x0aaf);
		mii_mgr_write(1, 25, 0x0fad);
		mii_mgr_write(1, 26, 0x0fc1);
	} else {
		mii_mgr_write(1, 21, 0x0517);
		mii_mgr_write(1, 22, 0x0fd2);
		mii_mgr_write(1, 23, 0x00bf);
		mii_mgr_write(1, 24, 0x0aab);
		mii_mgr_write(1, 25, 0x00ae);
		mii_mgr_write(1, 26, 0x0fff);
	}
	mii_mgr_write(1, 31, 0x1000); // global, page 1
	mii_mgr_write(1, 17, 0xe7f8);

	mii_mgr_write(1, 31, 0x8000); // local, page 0
	for (i = 0; i <= MAX_ESW_PHY_ID; i++)
		mii_mgr_write(i, 30, 0xa000);

	/* set default ability */
	for (i = 0; i <= MAX_ESW_PHY_ID; i++)
		mii_mgr_write(i, 4, 0x05e1);

	mii_mgr_write(1, 31, 0xa000); // local, page 2
	mii_mgr_write(0, 16, 0x1111);
	mii_mgr_write(1, 16, 0x1010);
	mii_mgr_write(2, 16, 0x1515);
	mii_mgr_write(3, 16, 0x0f0f);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 16, 0x1313);
#endif

#if !defined (CONFIG_RAETH_ESW_CONTROL)
	/* disable 802.3az EEE by default */
	mt7620_esw_eee_enable(0);
#endif

	mii_mgr_write(1, 31, 0x8000); // local, page 0
}

void mt7620_esw_eee_enable(int is_eee_enabled)
{
	u32 i, reg_pmsr;

	/* select PHY local page #3 */
	mii_mgr_write(1, 31, 0xb000);

	for (i = 0; i <= MAX_ESW_PHY_ID; i++) {
		/* check port link before touch EEE */
		reg_pmsr = sysRegRead(RALINK_ETH_SW_BASE + REG_ESW_MAC_PMSR_P0 + 0x100*i);
		if (reg_pmsr & 0x01)
			continue;
		
		/* set PHY EEE on/off */
		mii_mgr_write(i, 17, (is_eee_enabled) ? 0x0002 : 0x0000);
	}
}

void mt7620_esw_eee_on_link(u32 port_id, int port_link, int is_eee_enabled)
{
	if (port_id > MAX_ESW_PHY_ID)
		return;

	/* MT7620 ESW need disable EEE on every link down */
	if (is_eee_enabled || port_link)
		return;

	/* select PHY local page #3 */
	mii_mgr_write(port_id, 31, 0xb000);

	/* set PHY EEE off */
	mii_mgr_write(port_id, 17, 0x0000);
}
#endif

#if !defined (CONFIG_RAETH_ESW)
static int mt7620_esw_write_vtcr(u32 vtcr_cmd, u32 vtcr_val)
{
	u32 i, reg_vtcr;

	reg_vtcr = (vtcr_cmd << 12) | vtcr_val | 0x80000000;
	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_VLAN_VTCR, reg_vtcr);

	for (i = 0; i < 200; i++) {
		udelay(100);
		reg_vtcr = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_VLAN_VTCR);
		if (!(reg_vtcr & 0x80000000))
			return 0;
	}

	return -1;
}

int mt7620_esw_vlan_set_idx(u32 idx, u32 cvid, u32 port_member)
{
	u32 reg_val;

	idx &= 0xf;
	cvid &= 0xfff;
	port_member &= 0xff;

	// set vlan identifier
	reg_val = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_VLAN_ID_BASE + 4*(idx/2));
	if ((idx % 2) == 0) {
		reg_val &= 0xfff000;
		reg_val |= cvid;
	} else {
		reg_val &= 0x000fff;
		reg_val |= (cvid << 12);
	}
	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_VLAN_ID_BASE + 4*(idx/2), reg_val);

	// set vlan member
	reg_val = 1;				// VALID
	reg_val |= (1u << 30);			// IVL=1
	reg_val |= (port_member << 16);		// PORT_MEM

	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_VLAN_VAWD1, reg_val);
	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_VLAN_VAWD2, 0);

	return mt7620_esw_write_vtcr(1, idx);
}

int mt7620_esw_vlan_clear_idx(u32 idx)
{
	idx &= 0xf;

	return mt7620_esw_write_vtcr(2, idx);
}

void mt7620_esw_pvid_set(u32 port_id, u32 pvid, u32 prio)
{
	u32 reg_ppbv = (1u << 16) | ((prio & 0x7) << 13) | (pvid & 0xfff);

	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_PORT_PPBV1_P0+0x100*port_id, reg_ppbv);
}
#endif

int mt7620_esw_wait_wt_mac(void)
{
	u32 i, atc_val;

	for (i = 0; i < 200; i++) {
		udelay(100);
		atc_val = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_ATC);
		if (!(atc_val & BIT(15)))
			return 0;
	}

	return -1;
}

int mt7620_esw_mac_table_clear(int static_only)
{
	u32 atc_val;

	/* clear all (non)static MAC entries */
	atc_val = (static_only) ? 0x8602 : 0x8002;

	/* clear all non-static MAC entries */
	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_ATC, atc_val);

	return mt7620_esw_wait_wt_mac();
}

/* MT7620 embedded switch */
void mt7620_esw_init(void)
{
	/* disable internal EPHY 4 */
#if defined (CONFIG_RAETH_ESW) && defined (CONFIG_RAETH_HAS_PORT4)
	sysRegWrite(RALINK_ETH_SW_BASE+0x7014, 0x10e0000c);
#endif

	/* init GPHY first */
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	ge1_set_mode(0, 1);
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR);
#endif
#if defined (CONFIG_P4_MAC_TO_PHY_MODE)
	ge2_set_mode(0, 1);
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2);
#endif

#if defined (CONFIG_RAETH_ESW)
	/* init EPHY only internal switch is used */
	mt7620_ephy_init();
#else
	/* disable internal EPHY 0~4, set internal PHY base address to 12 */
	sysRegWrite(RALINK_ETH_SW_BASE+0x7014, 0x1fec000c);
#endif

	if ((ralink_asic_rev_id & 0xf) >= 5)
		sysRegWrite(RALINK_ETH_SW_BASE+0x701c, 0x0800000c); // enlarge FE2SW_IPG

	sysRegWrite(RALINK_ETH_SW_BASE+0x0004, 0x00000007);	// PPE_PORT=7, PPE_EN=0
	sysRegWrite(RALINK_ETH_SW_BASE+0x270c, 0x000fff10);	// disable P7 mac learning
#if defined (CONFIG_RAETH_ESW)
	/* Use internal switch, enable vlan control, enable egress tags */
#if defined (CONFIG_RAETH_HAS_PORT5)
	sysRegWrite(RALINK_ETH_SW_BASE+0x2704, 0x20ff0003);	// P7 has security mode, egress always tagged
	sysRegWrite(RALINK_ETH_SW_BASE+0x2604, 0x20ff0003);	// P6 has security mode, egress always tagged
#else
	sysRegWrite(RALINK_ETH_SW_BASE+0x2704, 0x20df0003);	// P7 has security mode, egress always tagged
	sysRegWrite(RALINK_ETH_SW_BASE+0x2604, 0x20df0003);	// P6 has security mode, egress always tagged
#endif
	sysRegWrite(RALINK_ETH_SW_BASE+0x2710, 0x81000000);	// P7 is user port, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2610, 0x81000000);	// P6 is user port, admit all frames
#else /* !CONFIG_RAETH_ESW */
#if defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_HAS_PORT4)
	/* Use single external P5, disable ports learning and vlan control (full dumb mode) */
	sysRegWrite(RALINK_ETH_SW_BASE+0x2704, 0x00e00000);	// P7 has matrix mode (P7|P6|P5)
	sysRegWrite(RALINK_ETH_SW_BASE+0x2604, 0x00e00000);	// P6 has matrix mode (P7|P6|P5)
	sysRegWrite(RALINK_ETH_SW_BASE+0x2504, 0x00600000);	// P5 has matrix mode (P6|P5)
	sysRegWrite(RALINK_ETH_SW_BASE+0x2710, 0x810080c0);	// P7 is transparent port, disable PVID insert, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2610, 0x810080c0);	// P6 is transparent port, disable PVID insert, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2510, 0x810080c0);	// P5 is transparent port, disable PVID insert, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x260c, 0x000fff10);	// disable P6 mac learning
	sysRegWrite(RALINK_ETH_SW_BASE+0x250c, 0x000fff10);	// disable P5 mac learning
#elif defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_HAS_PORT5)
	/* Use single external P4, disable ports learning and vlan control (full dumb mode) */
	sysRegWrite(RALINK_ETH_SW_BASE+0x2704, 0x00d00000);	// P7 has matrix mode (P7|P6|P4)
	sysRegWrite(RALINK_ETH_SW_BASE+0x2604, 0x00d00000);	// P6 has matrix mode (P7|P6|P4)
	sysRegWrite(RALINK_ETH_SW_BASE+0x2404, 0x00500000);	// P4 has matrix mode (P6|P4)
	sysRegWrite(RALINK_ETH_SW_BASE+0x2710, 0x810080c0);	// P7 is transparent port, disable PVID insert, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2610, 0x810080c0);	// P6 is transparent port, disable PVID insert, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2410, 0x810080c0);	// P4 is transparent port, disable PVID insert, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x260c, 0x000fff10);	// disable P6 mac learning
	sysRegWrite(RALINK_ETH_SW_BASE+0x240c, 0x000fff10);	// disable P4 mac learning
#else
	/* Use both external P5 & P4, enable vlan control, enable egress tags */
	sysRegWrite(RALINK_ETH_SW_BASE+0x2704, 0x20f00003);	// P7 has security mode, egress always tagged
	sysRegWrite(RALINK_ETH_SW_BASE+0x2604, 0x20f00003);	// P6 has security mode, egress always tagged
	sysRegWrite(RALINK_ETH_SW_BASE+0x2504, 0x00f00003);	// P5 has security mode
	sysRegWrite(RALINK_ETH_SW_BASE+0x2404, 0x00f00003);	// P4 has security mode
	sysRegWrite(RALINK_ETH_SW_BASE+0x2710, 0x81000000);	// P7 is user port, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2610, 0x81000000);	// P6 is user port, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2510, 0x810000c0);	// P5 is transparent port, admit all frames
	sysRegWrite(RALINK_ETH_SW_BASE+0x2410, 0x810000c0);	// P4 is transparent port, admit all frames
	mt7620_esw_vlan_set_idx(0, 1, 0xe0);			// VID=1 members (P7|P6|P5)
	mt7620_esw_vlan_set_idx(1, 2, 0xd0);			// VID=2 members (P7|P6|P4)
	mt7620_esw_pvid_set(5, 1, 0);				// P5 PVID=1
	mt7620_esw_pvid_set(4, 2, 0);				// P4 PVID=2
	mt7620_esw_mac_table_clear(0);
#endif
#endif

	/* Port 6 (CPU) */
	sysRegWrite(RALINK_ETH_SW_BASE+0x3600, 0x0005e33b);	// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
	sysRegWrite(RALINK_ETH_SW_BASE+0x000c, 0x0007181d);	// TO_CPU check VLAN members
	sysRegWrite(RALINK_ETH_SW_BASE+0x0010, 0x7f7f7fe0);	// Set Port6 CPU Port

	/* Port 5 */
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	/* Use P5 for connect to external RGMII MAC */
	ge1_set_mode(0, 0);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3500, 0x0005e33b);	// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P5_RGMII_TO_MT7530_MODE)
	/* Use P5 for connect to external MT7530 (P6) */
	ge1_set_mode(0, 1);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3500, 0x0005e33b);	// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
	/* Initial config MT7530 via MDIO */
	mt7530_gsw_init();
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	/* Use P5 for connect to external MII MAC */
	ge1_set_mode(1, 0);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3500, 0x0005e337);	// (P5, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	/* Use P5 for connect to external RvMII MAC */
	ge1_set_mode(2, 0);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3500, 0x0005e337);	// (P5, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	/* Use P5 for connect to external GigaPHY (with autopolling) */
	sysRegWrite(RALINK_ETH_SW_BASE+0x3500, 0x00056330);	// (P5, AN)
	enable_autopoll_phy(1);
#else
	/* Disable P5 */
	sysRegWrite(RALINK_ETH_SW_BASE+0x3500, 0x00008000);	// P5 link down
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_GE1;	// set GE1 to GPIO mode
#endif

	/* Port 4 */
#if defined (CONFIG_P4_RGMII_TO_MAC_MODE)
	/* Use P4 for connect to external RGMII MAC */
	ge2_set_mode(0, 0);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3400, 0x0005e33b);	// (P4, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5)
	/* Use P4 for connect to external MT7530 GMAC P5 */
	ge2_set_mode(0, 1);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3400, 0x0005e33b);	// (P4, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_MII_TO_MAC_MODE)
	/* Use P4 for connect to external MII MAC */
	ge2_set_mode(1, 0);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3400, 0x0005e337);	// (P4, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_RMII_TO_MAC_MODE)
	/* Use P4 for connect to external RvMII MAC */
	ge2_set_mode(2, 0);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3400, 0x0005e337);	// (P4, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_MAC_TO_PHY_MODE)
	/* Use P4 for connect to external GigaPHY (with autopolling) */
	sysRegWrite(RALINK_ETH_SW_BASE+0x3400, 0x00056330);	// (P4, AN)
	enable_autopoll_phy(1);
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4) || defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0)
	/* Use P4 for connect to external MT7530 GigaPHY P4 or P0 (with autopolling) */
	ge2_set_mode(0, 1);
	sysRegWrite(RALINK_ETH_SW_BASE+0x3400, 0x00056330);	// (P4, AN)
	enable_autopoll_phy(1);
#else
	/* Disable P4 (set RJ-45 mode) */
	ge2_set_mode(3, 0);
#endif
}
