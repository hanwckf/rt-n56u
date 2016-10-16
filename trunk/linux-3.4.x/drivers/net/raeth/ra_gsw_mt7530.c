#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/string.h>

#include "ra_esw_reg.h"
#include "mii_mgr.h"
#include "ra_gsw_mt7530.h"

extern u32 ralink_asic_rev_id;

static u8 mt7530_xtal_fsel = 0;
static u8 mt7530_standalone = 0;
static u8 mt7530_eee_enabled = 0;
static u8 mt7530_phy_patched[5] = {0};

#if defined (CONFIG_RALINK_MT7620)
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5)
#define MT7530_P5_MODE_GMAC
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
#define MT7530_P5_MODE_GPHY_P4
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0)
#define MT7530_P5_MODE_GPHY_P0
#endif
#elif defined (CONFIG_RALINK_MT7621)
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5)
#define MT7530_P5_MODE_GMAC
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P4)
#define MT7530_P5_MODE_GPHY_P4
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P0)
#define MT7530_P5_MODE_GPHY_P0
#endif
#endif

#if defined (CONFIG_GE1_TRGMII_FORCE_1000) || defined (CONFIG_GE1_TRGMII_FORCE_1200) || \
    defined (CONFIG_GE1_TRGMII_FORCE_2000) || defined (CONFIG_GE1_TRGMII_FORCE_2600)
#define MT7530_P6_MODE_TRGMII
#endif

#if defined (MT7530_P6_MODE_TRGMII)
static void mt7530_gsw_set_trgmii_clock(void)
{
	u32 reg_val;

	// GSW_1X_CLK (RGMII)
	mii_mgr_write_cl45(0, 0x1f, 0x0410, 0x0001);

	reg_val = 0x0780;
	if (mt7530_xtal_fsel == 0x3) {
		/* 25Mhz Xtal */
#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
		// PLL 150MHz
		reg_val = 0x0c00;
#elif defined (CONFIG_GE1_TRGMII_FORCE_2000)
		// PLL 250MHz
		reg_val = 0x1400;
#elif defined (CONFIG_GE1_TRGMII_FORCE_2600)
		// PLL 325MHz
		reg_val = 0x1a00;
#else
		// PLL 125MHz
		reg_val = 0x0a00;
#endif
	} else if (mt7530_xtal_fsel == 0x2) {
		/* 40Mhz Xtal */
#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
		// PLL 150MHz
		reg_val = 0x0780;
#elif defined (CONFIG_GE1_TRGMII_FORCE_2000)
		// PLL 250MHz
		reg_val = 0x0c80;
#elif defined (CONFIG_GE1_TRGMII_FORCE_2600)
		// PLL 325MHz
		reg_val = 0x1040;
#else
		// PLL 125MHz
		reg_val = 0x0640;
#endif
	} else {
		/* 20Mhz Xtal - todo */
	}

	// TRGMII PLL
	mii_mgr_write_cl45(0, 0x1f, 0x0404, reg_val);
//	mii_mgr_write_cl45(0, 0x1f, 0x0405, 0); // MT7623 only?
	mdelay(1);

	if (mt7530_xtal_fsel == 0x3) {
		/* 25Mhz Xtal */
		reg_val = 0x0057;
	} else {
		/* 20/40Mhz Xtal */
		reg_val = 0x0087;
	}

	mii_mgr_write_cl45(0, 0x1f, 0x0409, reg_val);
	mdelay(1);
	mii_mgr_write_cl45(0, 0x1f, 0x040a, reg_val);

	// PLL BIAS en
	mii_mgr_write_cl45(0, 0x1f, 0x0403, 0x1800);
	mdelay(1);

	// BIAS LPF en
	mii_mgr_write_cl45(0, 0x1f, 0x0403, 0x1c00);

	// sys PLL en
	mii_mgr_write_cl45(0, 0x1f, 0x0401, 0xc020);

	// LCDDDS PWDS
	mii_mgr_write_cl45(0, 0x1f, 0x0406, 0xa030);
//	mii_mgr_write_cl45(0, 0x1f, 0x0406, 0xa038); // MT7623 only?
	mdelay(1);

	// GSW_2X_CLK (TRGMII)
	mii_mgr_write_cl45(0, 0x1f, 0x0410, 0x0003);
}
#endif

static void mt7530_gsw_set_pll(void)
{
	if (mt7530_xtal_fsel == 0x3) {
		/* 25Mhz Xtal - do nothing */
	} else if (mt7530_xtal_fsel == 0x2) {
		/* 40Mhz Xtal */
		
		/* disable MT7530 core clock */
		mii_mgr_write_cl45(0, 0x1f, 0x0410, 0x0000);
		
		/* disable MT7530 PLL */
		mii_mgr_write_cl45(0, 0x1f, 0x040d, 0x2020);
		
		/* MT7530 core clock = 500MHz */
		mii_mgr_write_cl45(0, 0x1f, 0x040e, 0x0119);
		
		/* enable MT7530 PLL */
		mii_mgr_write_cl45(0, 0x1f, 0x040d, 0x2820);
		udelay(20);
	} else {
		/* 20Mhz Xtal - todo */
	}

	/* enable MT7530 core clock */
#if defined (MT7530_P6_MODE_TRGMII)
	/* TRGMII */
	mii_mgr_write_cl45(0, 0x1f, 0x0410, 0x0003);
#else
	/* RGMII */
	mii_mgr_write_cl45(0, 0x1f, 0x0410, 0x0001);
#endif
}

static void mt7530_gsw_auto_downshift_phy(u32 port_id, int is_ads_enabled)
{
	u32 regValue = 0x3a14;	// 0x3a14 is default

#if defined (MT7530_P5_MODE_GPHY_P4)
	if (port_id == 4)
		is_ads_enabled = 1;	// EEE on P4 always off
#elif defined (MT7530_P5_MODE_GPHY_P0)
	if (port_id == 0)
		is_ads_enabled = 1;	// EEE on P0 always off
#endif
	/* HW auto downshift control */
	mii_mgr_write(port_id, 31, 0x1);
	mii_mgr_read(port_id, 20, &regValue);
	if (is_ads_enabled)
		regValue |=  (1<<4);
	else
		regValue &= ~(1<<4);
	mii_mgr_write(port_id, 20, regValue);
	mii_mgr_write(port_id, 31, 0x0);
}

static void mt7530_gsw_patch_port_phy(u32 port_id)
{
/*
 *	Note 1: forced slave mode is bad idea, this needed poll 'MASTER/SLAVE configuration fault'
 *	        (reg 10, bit 15), when both link partners is forced to slave, link failed.
 */

#if 0
	/* Increase SlvDPSready time (work only PHY is powered, use link_on interrupt) */
	mii_mgr_write(port_id, 31, 0x52b5);
	mii_mgr_write(port_id, 16, 0xafae);
	mii_mgr_write(port_id, 18, 0x002f);	// 0x0004 is default
	mii_mgr_write(port_id, 16, 0x8fae);
#endif

	/* Increase post_update_timer */
	mii_mgr_write(port_id, 31, 0x3);
	mii_mgr_write(port_id, 17, 0x004b);	// 0x0034 is default

	/* Adjust 100_mse_threshold */
	mii_mgr_write_cl45(port_id, 0x1e, 0x0123, 0xffff);	// 0x80a0 is default

	/* Disable mcc */
	mii_mgr_write_cl45(port_id, 0x1e, 0x00a6, 0x0300);	// 0x03e0 is default

	/* HW auto downshift control */
	mt7530_gsw_auto_downshift_phy(port_id, (mt7530_eee_enabled) ? 0 : 1);

#if 0
	/* Increase 10M mode RX gain for long cable */
	mii_mgr_write(port_id, 31, 0x52b5);
	mii_mgr_write(port_id, 16, 0xaf92);
	mii_mgr_write(port_id, 17, 0x8689);	// 0x3689 is default
	mii_mgr_write(port_id, 16, 0x8f92);
#endif
}

static void mt7530_gsw_patch_phy(void)
{
	u32 i;

	/* may be need for external MT7530 too? */
	if (mt7530_standalone)
		return;

	for (i = 0; i <= 4; i++)
		mt7530_gsw_patch_port_phy(i);
}

void mt7530_gsw_eee_enable(int is_eee_enabled)
{
	u32 i;

	mt7530_eee_enabled = (is_eee_enabled) ? 1 : 0;

	for (i = 0; i <= 4; i++) {
#if defined (MT7530_P5_MODE_GPHY_P4)
		if (i == 4 && is_eee_enabled) continue;
#elif defined (MT7530_P5_MODE_GPHY_P0)
		if (i == 0 && is_eee_enabled) continue;
#endif
		/* EEE 1000/100 LPI */
		mii_mgr_write_cl45(i, 0x07, 0x003c, (is_eee_enabled) ? 0x0006 : 0x0000);
		
		/* HW auto downshift control */
		mt7530_gsw_auto_downshift_phy(i, (is_eee_enabled) ? 0 : 1);
	}

	/* EEE 10Base-Te (global) */
//	mii_mgr_write_cl45(0, 0x1f, 0x027b, (is_eee_enabled) ? 0x1147 : 0x1177);
}

void mt7530_gsw_eee_on_link(u32 port_id, int port_link, int is_eee_enabled)
{
	if (port_id > 4)
		return;

	/* MT7621/MT7623 ESW need update PHY params on link changed */

	/* may be need for external MT7530 too? */
	if (mt7530_standalone)
		return;

	/* Increase SlvDPSready time */
	if (port_link && !mt7530_phy_patched[port_id]) {
		mt7530_phy_patched[port_id] = 1;
		mii_mgr_write(port_id, 31, 0x52b5);
		mii_mgr_write(port_id, 16, 0xafae);
		mii_mgr_write(port_id, 18, 0x002f);
		mii_mgr_write(port_id, 16, 0x8fae);
	}

#if 0
	/* this workaround removed in SDK 5.0.1.0 */
	if (is_eee_enabled) {
#if defined (MT7530_P5_MODE_GPHY_P4)
		if (port_id == 4) return;
#elif defined (MT7530_P5_MODE_GPHY_P0)
		if (port_id == 0) return;
#endif
		mii_mgr_write(port_id, 31, 0x52b5);
		mii_mgr_write(port_id, 16, 0xb780);
		mii_mgr_write(port_id, 17, (port_link) ? 0x00e0 : 0x0000);
		mii_mgr_write(port_id, 16, 0x9780);
	}
#endif
}

void mt7530_gsw_set_csr_delay(int is_link_100)
{
#if defined (CONFIG_RALINK_MT7621)
#if defined (MT7530_P5_MODE_GPHY_P4) || defined (MT7530_P5_MODE_GPHY_P0)
	u32 rx_csr = 0x0102;

	/* only MT7621 E2 has FC bug */
	if ((ralink_asic_rev_id & 0xFFFF) != 0x0101)
		return;

	if (is_link_100) {
		/* P5 RGMII RX Clock Control: delay setting for 100M */
		rx_csr = 0x0008;
	}

	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, rx_csr);
#endif
#endif
}

void mt7530_gsw_set_smac(const u8 *mac)
{
	u32 regValue;

	regValue = ((u32)mac[0] << 8) | mac[1];
	mii_mgr_write(MT7530_MDIO_ADDR, REG_ESW_MAC_SMACCR1, regValue);

	regValue = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) | ((u32)mac[4] << 8) | mac[5];
	mii_mgr_write(MT7530_MDIO_ADDR, REG_ESW_MAC_SMACCR0, regValue);
}

int mt7530_gsw_wait_wt_mac(void)
{
	u32 i, atc_val;

	for (i = 0; i < 200; i++) {
		udelay(100);
		atc_val = 0;
		mii_mgr_read(MT7530_MDIO_ADDR, REG_ESW_WT_MAC_ATC, &atc_val);
		if (!(atc_val & BIT(15)))
			return 0;
	}

	return -1;
}

int mt7530_gsw_mac_table_clear(int static_only)
{
	u32 atc_val;

	/* clear all (non)static MAC entries */
	atc_val = (static_only) ? 0x8602 : 0x8002;

	mii_mgr_write(MT7530_MDIO_ADDR, REG_ESW_WT_MAC_ATC, atc_val);

	return mt7530_gsw_wait_wt_mac();
}

/* MT7350 standalone or MCM embedded (MT7621/MT7623 on die) switch */
void mt7530_gsw_init(void)
{
	u32 regLink, regValue, rgmii_tx_drive;

	/* force mode, Link Up, 1000Mbps, Full-Duplex, FC ON */
	regLink = 0x5e33b;

	/* read xtal_fsel from HWTRAP */
	if (!mt7530_xtal_fsel) {
		u32 hw_trap = 0x400;
		mii_mgr_read(MT7530_MDIO_ADDR, 0x7800, &hw_trap);
		mt7530_xtal_fsel = (unsigned char)((hw_trap >> 9) & 0x3);
	}

#if defined (CONFIG_RALINK_MT7620)
	mt7530_standalone = 1;
#elif defined (CONFIG_RALINK_MT7621)
	/* PKG_ID [16:16], 1: A/S, 0: N (no embedded switch)  */
	if (!(ralink_asic_rev_id & (1UL<<16)))
		mt7530_standalone = 1;

	/* MT7621 E2 has FC bug, disable FC */
	if ((ralink_asic_rev_id & 0xFFFF) == 0x0101)
		regLink &= ~(0x3 << 4);
#endif

#if defined (MT7530_P6_MODE_TRGMII)
	/* enable MT7530 TRGMII clock */
	mt7530_gsw_set_trgmii_clock();
#endif

	/* configure MT7530 HW-TRAP */
	regValue = 0x7c8f;
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7804, &regValue);
	regValue |=  (1<<16);						// Change HW-TRAP
	if (mt7530_standalone)
		regValue |= (1<<24);					// Standalone Switch
	regValue &= ~(1<<15);						// Switch clock = 500MHz
	regValue &= ~(1<<5);						// PHY direct access mode
	regValue &= ~(1<<8);						// Enable Port 6
#if defined (MT7530_P5_MODE_GMAC)
	regValue &= ~(1<<6);						// Enable Port5
	regValue |=  (1<<7);						// Port 5 mode is RGMII
	regValue |=  (1<<13);						// Port 5 connects to GMAC5
#elif defined (MT7530_P5_MODE_GPHY_P0)
	regValue &= ~(1<<6);						// Enable Port5
	regValue |=  (1<<7);						// Port 5 mode is RGMII
	regValue &= ~(1<<13);						// Port 5 connects to GPHY0/4
	regValue |=  (1<<20);						// Port 5 GPHY0 selected
#elif defined (MT7530_P5_MODE_GPHY_P4)
	regValue &= ~(1<<6);						// Enable Port5
	regValue |=  (1<<7);						// Port 5 mode is RGMII
	regValue &= ~(1<<13);						// Port 5 connects to GPHY0/4
	regValue &= ~(1<<20);						// Port 5 GPHY4 selected
#else
	regValue |=  (1<<6);						// Disable Port5
	regValue |=  (1<<13);						// Port 5 connects to GMAC5
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7804, regValue);

	/* configure MT7350 Port6 */
#if defined (MT7530_P5_MODE_GMAC)
	/* do not override P6 security (use external switch control) */
#elif defined (MT7530_P5_MODE_GPHY_P4)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x004f0000);		// P6 has matrix mode (P6|P3|P2|P1|P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#elif defined (MT7530_P5_MODE_GPHY_P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x005e0000);		// P6 has matrix mode (P6|P4|P3|P2|P1)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#elif !defined (CONFIG_RAETH_GMAC2)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x205f0003);		// P6 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x81000000);		// P6 is user port, admit all frames
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3600, regLink);		// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex)

	/* configure MT7350 Port5 */
#if defined (MT7530_P5_MODE_GMAC)
	/* do not override P5 security (use external switch control) */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, regLink);		// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex)
#elif defined (MT7530_P5_MODE_GPHY_P0) || defined (MT7530_P5_MODE_GPHY_P4)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x56300);		// (P5, AN) wdf ???
#else
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x8000);		// (P5, link OFF)
#endif

	/* configure MT7530 PLL */
	mt7530_gsw_set_pll();

#if defined (MT7530_P6_MODE_TRGMII)
	/* TRGMII mode */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7830, &regValue);
	regValue &= ~(0x03);
	regValue |=   0x01;
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7830, regValue);

	/* TRGMII central align */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7a40, &regValue);
	regValue |= (1<<30);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a40, regValue);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a78, 0x0055);
#else
	/* RGMII mode */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7830, &regValue);
	regValue &= ~(0x03);
	regValue |=   0x02;
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7830, regValue);

	/* RGMII central align */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7a40, &regValue);
	regValue &= ~((1<<30)|(1<<28));
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a40, regValue);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a78, 0x0855);
#endif

	/* set P5 RGMII delay setting for 10/1000M */
#if defined (MT7530_P5_MODE_GPHY_P4) || defined (MT7530_P5_MODE_GPHY_P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, 0x102);			// P5 RGMII RX Clock Control
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b04, 0x14);			// P5 RGMII TX Clock Control, delay 4
#elif defined (MT7530_P5_MODE_GMAC)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b04, 0x10);			// P5 RGMII TX Clock Control, delay 0
#endif

	/* reduce P6 RGMII Tx driving */
#if defined (CONFIG_GE1_TRGMII_FORCE_2000) || defined (CONFIG_GE1_TRGMII_FORCE_2600)
	rgmii_tx_drive = 0x88;
#else
	rgmii_tx_drive = 0x44;
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a54, rgmii_tx_drive);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a5c, rgmii_tx_drive);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a64, rgmii_tx_drive);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a6c, rgmii_tx_drive);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a74, rgmii_tx_drive);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a7c, rgmii_tx_drive);

	/* reduce P5 RGMII Tx driving */
#if defined (MT7530_P5_MODE_GMAC) || defined (MT7530_P5_MODE_GPHY_P4) || defined (MT7530_P5_MODE_GPHY_P0)
	rgmii_tx_drive = 0x11; // 8mA
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7810, rgmii_tx_drive);
#endif

	if (mt7530_standalone) {
		/* disable MT7530 CKG_LNKDN_GLB on external MT7530 */
		mii_mgr_write(MT7530_MDIO_ADDR, REG_ESW_MAC_CKGCR, 0x1e02);
	}

	/* TO_CPU check VLAN members */
	mii_mgr_write(MT7530_MDIO_ADDR, REG_ESW_AGC, 0x0007181d);

	/* Set P6 as CPU Port */
	mii_mgr_write(MT7530_MDIO_ADDR, REG_ESW_MFC, 0x7f7f7fe0);

#if !defined (CONFIG_RAETH_ESW_CONTROL)
	/* disable 802.3az EEE by default */
	mt7530_gsw_eee_enable(0);
#endif

	/* MT7621/MT7623 MCM need patch PHY registers */
	mt7530_gsw_patch_phy();

	/* enable switch INTR */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7808, &regValue);
	regValue |= (3<<16);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7808, regValue);
}

