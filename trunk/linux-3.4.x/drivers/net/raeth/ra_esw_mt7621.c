#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/string.h>

#include "ra_ethreg.h"
#include "mii_mgr.h"
#include "ra_phy.h"
#include "ra_esw_reg.h"
#include "ra_esw_mt7621.h"

extern u32 ralink_asic_rev_id;

#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
static void turbo_rgmii_set_pll(void)
{
	// PLL to 150Mhz
	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x404);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0x0780);	//40Mhz XTAL for 150Mhz CLK
	mdelay(1);

	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x409);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0x57);
	mdelay(1);

	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x40a);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0x57);

	// PLL BIAS en
	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x403);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0x1800);
	mdelay(1);

	// BIAS LPF en
	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x403);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0x1c00);

	// sys PLL en
	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x401);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0xc020);

	// LCDDDS PWDS
	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x406);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0xa030);
	mdelay(1);

	// GSW_2X_CLK
	mii_mgr_write(0, 13, 0x1f);
	mii_mgr_write(0, 14, 0x410);
	mii_mgr_write(0, 13, 0x401f);
	mii_mgr_write(0, 14, 0x0003);
}

#define REGBIT(x, n)	(x << n)
static void apll_xtal_enable(void)
{
	u32 data = 0;
	u32 regValue = 0;

	/* Firstly, reset all required register to default value */
	sysRegWrite(RALINK_ANA_CTRL_BASE, 0x00008000);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0014, 0x01401d61);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0018, 0x38233d0e);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x001c, 0x80120004);
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0020, 0x1c7dbf48);

	/* toggle RG_XPTL_CHG */
	sysRegWrite(RALINK_ANA_CTRL_BASE, 0x00008800);
	sysRegWrite(RALINK_ANA_CTRL_BASE, 0x00008c00);

	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x0014);
	data &= ~(0x0000ffc0);

	regValue = sysRegRead(RALINK_SYSCTL_BASE + 0x10);
	regValue = (regValue >> 6) & 0x7;
	if (regValue < 6) { //20/40Mhz Xtal
		data |= REGBIT(0x1d, 8);
	} else {
		data |= REGBIT(0x17, 8);
	}
	if (regValue < 6) { //20/40Mhz Xtal
		data |= REGBIT(0x1, 6);
	}
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0014, data);

	data = sysRegRead(RALINK_ANA_CTRL_BASE+0x0018);
	data &= ~(0xf0773f00);
	data |= REGBIT(0x3, 28);
	data |= REGBIT(0x2, 20);
	if (regValue < 6) { //20/40Mhz Xtal
		data |= REGBIT(0x3, 16);
	} else {
		data |= REGBIT(0x2, 16);
	}
	data |= REGBIT(0x3, 12);
	if (regValue < 6) { //20/40Mhz Xtal
		data |= REGBIT(0xd, 8);
	} else {
		data |= REGBIT(0x7, 8);
	}
	sysRegWrite(RALINK_ANA_CTRL_BASE+0x0018, data);

	if (regValue < 6) { //20/40Mhz Xtal
		sysRegWrite(RALINK_ANA_CTRL_BASE+0x0020, 0x1c7dbf48);
	} else {
		sysRegWrite(RALINK_ANA_CTRL_BASE+0x0020, 0x1697cc39);
	}

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

	/* 8. TRGMII TX CLK SEL APLL */
	data = sysRegRead(0xbe00002c);
	data &= 0xffffff9f;
	data |= 0x40;
	sysRegWrite(0xbe00002c, data);
}
#endif

static void gsw_set_pll(void)
{
	u32 regValue;

	regValue = sysRegRead(RALINK_SYSCTL_BASE + 0x10);
	regValue = (regValue >> 6) & 0x7;
	if (regValue >= 6) {
		/* 25Mhz Xtal - do nothing */
	} else if (regValue >= 3) {
		/* 40Mhz Xtal */
		mii_mgr_write(0, 13, 0x1f);	// disable MT7530 core clock
		mii_mgr_write(0, 14, 0x410);
		mii_mgr_write(0, 13, 0x401f);
		mii_mgr_write(0, 14, 0x0);
		
		mii_mgr_write(0, 13, 0x1f);	// disable MT7530 PLL
		mii_mgr_write(0, 14, 0x40d);
		mii_mgr_write(0, 13, 0x401f);
		mii_mgr_write(0, 14, 0x2020);
		
		mii_mgr_write(0, 13, 0x1f);	// for MT7530 core clock = 500Mhz
		mii_mgr_write(0, 14, 0x40e);
		mii_mgr_write(0, 13, 0x401f);
		mii_mgr_write(0, 14, 0x119);
		
		mii_mgr_write(0, 13, 0x1f);	// enable MT7530 PLL
		mii_mgr_write(0, 14, 0x40d);
		mii_mgr_write(0, 13, 0x401f);
		mii_mgr_write(0, 14, 0x2820);
		udelay(20);
		
		mii_mgr_write(0, 13, 0x1f);	// enable MT7530 core clock
		mii_mgr_write(0, 14, 0x410);
		mii_mgr_write(0, 13, 0x401f);
	} else {
		/* 20Mhz Xtal - todo */
	}

#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
	/* set TRGMII clock mode */
	mii_mgr_write(0, 14, 0x0003);

	/* enable MT7530 TRGMII */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7830, 0x1);
#else
	/* set RGMII clock mode */
	mii_mgr_write(0, 14, 0x0001);
#endif
}

void mt7621_esw_fc_delay_set(int is_link_100)
{
	if (is_link_100) {
		/* delay setting for 100M */
		mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, 0x008);
	} else {
		/* delay setting for 10/1000M */
		mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, 0x102);
	}
}

/* MT7621 embedded switch (aka MT7350) */
void mt7621_esw_init(void)
{
	u32  __maybe_unused i, regLink, regValue = 0;

	/* MT7621 E2 has FC bug */
	if ((ralink_asic_rev_id & 0xFFFF) == 0x0101)
		regLink = 0x5e30b; // Force mode, Link Up, 1000Mbps, Full-Duplex, FC OFF
	else
		regLink = 0x5e33b; // Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON

#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
	turbo_rgmii_set_pll();

	// switch to APLL if TRGMII + DDR2
	regValue = sysRegRead(RALINK_SYSCTL_BASE + 0x10);
	if ((regValue >> 4) & 0x1)
		apll_xtal_enable();
#endif

	/* configure MT7530 HW-TRAP */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7804, &regValue);
	regValue |= (1<<16);						// Change HW-TRAP
	regValue &= ~(1<<8);						// Enable Port 6
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	regValue &= ~((1<<6));						// Enable Port5
	regValue |= ((1<<13)|(1<<7));					// Port 5 as GMAC, no Internal PHY mode
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P0)
	regValue &= ~((1<<6)|(1<<13));					// Enable Port5, set to PHY P0 mode
	regValue |= ((1<<7)|(1<<20));
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	regValue &= ~((1<<6)|(1<<13)|(1<<20));				// Enable Port5, set to PHY P4 mode
	regValue |= ((1<<7));
#else
	regValue |= ((1<<6)|(1<<13));					// Disable Port 5, set as GMAC, no Internal PHY mode
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7804, regValue);

	/* configure MT7350 Port6 */
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x004f0003);		// P6 set security mode
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x004f0000);		// P6 has matrix mode (P6|P3|P2|P1|P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x005e0000);		// P6 has matrix mode (P6|P4|P3|P2|P1)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#else
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x205f0003);		// P6 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x81000000);		// P6 is user port, admit all frames
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3600, regLink);		// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, (0x20000000|regLink));	// (GE1, Force 1000M/FD)

	/* configure MT7350 Port5 */
#if defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2504, 0x00300003);		// P5 set security mode
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2510, 0x810000c0);		// P5 is transparent port, admit all frames
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2514, 0x00010002);		// P5 PVID=2
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, regLink);		// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, (0x20000000|regLink));	// (GE2, Force 1000M/FD)
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P4) || defined (CONFIG_GE2_INTERNAL_GPHY_P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x56300);		// (P5, AN) wdf ???
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x20056300);		// (GE2, AN)
#endif

	/* config switch PLL */
	gsw_set_pll();

	/* set MT7530 central align */
#if !defined (CONFIG_GE1_TRGMII_FORCE_1200)
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7830, &regValue);
	regValue &= ~1;
	regValue |= (1<<1);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7830, regValue);

	mii_mgr_read(MT7530_MDIO_ADDR, 0x7a40, &regValue);
	regValue &= ~(1<<30);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a40, regValue);

	regValue = 0x855;
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a78, regValue);
#endif

	/* set MT7530 delay setting for 10/1000M */
#if defined (CONFIG_GE2_INTERNAL_GPHY_P4) || defined (CONFIG_GE2_INTERNAL_GPHY_P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, 0x102);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b04, 0x14);
#endif

	/* set lower Tx driving */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a54, 0x44);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a5c, 0x44);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a64, 0x44);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a6c, 0x44);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a74, 0x44);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a7c, 0x44);

	/* disable EEE */
#if defined (CONFIG_GE2_INTERNAL_GPHY_P4) || defined (CONFIG_GE2_INTERNAL_GPHY_P0)
	for (i = 0; i <= 4; i++) {
		mii_mgr_write(i, 13, 0x7);
		mii_mgr_write(i, 14, 0x3C);
		mii_mgr_write(i, 13, 0x4007);
		mii_mgr_write(i, 14, 0x0);
	}

	/* disable EEE 10Base-T */
	for (i = 0; i <= 4; i++) {
		mii_mgr_write(i, 13, 0x1f);
		mii_mgr_write(i, 14, 0x027b);
		mii_mgr_write(i, 13, 0x401f);
		mii_mgr_write(i, 14, 0x1177);
	}
#endif

	/* enable switch INTR */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7808, &regValue);
	regValue |= (3<<16);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7808, regValue);

#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	/* autopoll GPHY P4/P0 */
	enable_autopoll_phy(1);
#endif
}
