#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/netdevice.h>
#include <linux/sched.h>

#include <linux/ralink_gpio.h>

#include "raether.h"
#include "mii_mgr.h"

/*  PHY Vender ID list */
#define EV_MARVELL_PHY_ID0		0x0141
#define EV_MARVELL_PHY_ID1		0x0CC2

#define EV_VTSS_PHY_ID0			0x0007
#define EV_VTSS_PHY_ID1			0x0421

extern u32 ralink_asic_rev_id;

#if defined (CONFIG_RAETH_ESW_CONTROL)
extern int esw_control_post_init(void);
#endif

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

static void init_giga_phy(int ge)
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
static void enable_autopoll_phy(int unused)
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
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2+1; // or = addr_s ?
#else
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR+1; // or = addr_s ?
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

static void enable_autopoll_phy(int ge)
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

#if defined (CONFIG_MT7530_GSW)
#if defined (CONFIG_RALINK_MT7621)

#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
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

	regValue = *(volatile u_long *)(RALINK_SYSCTL_BASE + 0x10);
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

	//*Common setting - Set PLLGP_CTRL_4 *//
	///* 1. Bit 31 */
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

/* internal MT7350 (on-die) */
static void mt7530_gsw_init(void)
{
	u32 i, regValue = 0;

	/* turn off all PHY */
	for(i = 0; i <= 4; i++) {
		mii_mgr_read(i, 0, &regValue);
		regValue |= (1<<11);
		mii_mgr_write(i, 0, regValue);
	}

	/* reset switch */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7000, 0x3);
	udelay(10);

#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
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

	// switch to APLL if TRGMII + DDR2
	regValue = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));
	if ((regValue >> 4) & 0x1)
		apll_xtal_enable();
#endif

	/* MT7530 HW-TRAP config */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7804, &regValue);
	regValue &= ~(1<<8);						// Enable Port 6
	regValue |= (1<<16);						// Change HW-TRAP
#if defined (CONFIG_GE2_INTERNAL_GPHY_P0)
	regValue &= ~((1<<13)|(1<<6));					// Enable Port5, set to PHY P0 mode
	regValue |= ((1<<7)|(1<<20));
#elif defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	regValue &= ~((1<<13)|(1<<6)|(1<<20));				// Enable Port5, set to PHY P4 mode
	regValue |= ((1<<7));
#else
	regValue |= (1<<13);						// Port 5 as GMAC, no Internal PHY mode
	regValue |= (1<<6);						// Disable Port 5
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7804, regValue);

#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
	/* enable TRGMII */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7830, 0x1);
#endif

	/* configure switch Port6 */
#if defined (CONFIG_RAETH_GMAC2)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x005f0003);		// P6 set security mode
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#else
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x205f0003);		// P6 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x81000000);		// P6 is user port, admit all frames
#endif
	if ((ralink_asic_rev_id & 0xFFFF) == 0x0101) {
		sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x0005e30b);	// (GE1, Force 1000M/FD, FC OFF)
		mii_mgr_write(MT7530_MDIO_ADDR, 0x3600, 0x5e30b);	// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex, FC OFF)
	} else {
		sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x0005e33b);	// (GE1, Force 1000M/FD, FC ON)
		mii_mgr_write(MT7530_MDIO_ADDR, 0x3600, 0x5e33b);	// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
	}

	/* configure switch Port5 */
#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2504, 0x003f0003);		// P5 set security mode
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2510, 0x810000c0);		// P5 is transparent port, admit all frames
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x00056300);		// P5 AN
#else
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x00008000);		// P5 link down
#endif

	/* config switch PLL */
	regValue = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));
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
#if defined (CONFIG_GE1_TRGMII_FORCE_1200)
		mii_mgr_write(0, 14, 0x0003);	/* TRGMII */
#else
		mii_mgr_write(0, 14, 0x0001);	/* RGMII */
#endif
	} else {
		/* 20Mhz Xtal - todo */
	}

#if !defined (CONFIG_GE1_TRGMII_FORCE_1200)
	/* set MT7530 central align */
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

	/* set MT7530 delay */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, 0x102);		// delay setting for 10/1000M
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b04, 0x14);		// delay setting for 10/1000M

#if 0
	/* todo, more documentation is needed */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a54, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a5c, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a64, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a6c, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a74, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a7c, 0x44);		// lower Tx driving
#endif

	/* enable switch INTR */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7808, &regValue);
	regValue |= (3<<16);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7808, regValue);
}

#else

/* external MT7350 */
static void mt7530_gsw_init(void)
{
	u32 i, regValue = 0;

	/* turn off all PHY */
	for(i = 0; i <= 4; i++) {
		mii_mgr_read(i, 0, &regValue);
		regValue |= (1<<11);
		mii_mgr_write(i, 0, regValue);
	}

	/* MT7530 HW-TRAP config */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7804, &regValue);
	regValue &= ~(1<<8);					// Enable Port 6
	regValue |= (1<<16);					// Change HW-TRAP
#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0)
	regValue &= ~((1<<13)|(1<<6)|(1<<5)|(1<<15));		// Enable Port5, set to PHY P0 mode
	regValue |= ((1<<7)|(1<<24)|(1<<20));
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	regValue &= ~((1<<13)|(1<<6)|(1<<5)|(1<<15)|(1<<20));	// Enable Port5, set to PHY P4 mode
	regValue |= ((1<<7)|(1<<24));
#else
	regValue |= (1<<13);					// Port 5 as GMAC, no Internal PHY mode
	regValue |= (1<<6);					// Disable Port 5
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7804, regValue);

	/* configure switch Port6 */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x205f0003);	// P6 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x81000000);	// P6 is user port, admit all frames
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3600, 0x0005e33b);	// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)

	/* configure switch Port5 */
#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	/* todo */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2504, 0x203f0003);	// P5 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2510, 0x81000000);	// P5 is user port, admit all frames
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x00056300);	// P5 AN
#else
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x00008000);	// P5 link down
#endif

	/* set MT7530 central align */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7830, &regValue);
	regValue &= ~1;
	regValue |= (1<<1);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7830, regValue);

	mii_mgr_read(MT7530_MDIO_ADDR, 0x7a40, &regValue);
	regValue &= ~(1<<30);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a40, regValue);

	regValue = 0x855;
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a78, regValue);

#if 0
	/* todo, more documentation is needed */

	/* set MT7530 delay */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, 0x102);		// delay setting for 10/1000M
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b04, 0x14);		// delay setting for 10/1000M

	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a54, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a5c, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a64, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a6c, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a74, 0x44);		// lower Tx driving
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7a7c, 0x44);		// lower Tx driving
#endif

	/* enable switch INTR */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7808, &regValue);
	regValue |= (3<<16);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7808, regValue);
}
#endif
#endif


#if defined (CONFIG_RALINK_MT7620)
static void mt7620_gsw_init(void)
{
#if defined (CONFIG_RAETH_ESW)
	u32 is_BGA = (ralink_asic_rev_id >> 16) & 0x1;

	*(volatile u32 *)(REG_SYSCFG1) |= (0x1 << 8);	// PCIE_RC_MODE=1
	*(volatile u32 *)(CKGCR) &= ~(0x3 << 4);	// keep rx/tx port clock ticking, disable internal clock-gating to avoid switch stuck

	/*
	* Reg 31: Page Control
	* Bit 15     => PortPageSel, 1=local, 0=global
	* Bit 14:12  => PageSel, local:0~3, global:0~4
	*
	* Reg16~30:Local/Global registers
	*
	*/
	/*correct PHY setting L3.0 BGA*/
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
	mii_mgr_write(1, 31, 0x1000); //global, page 1
	mii_mgr_write(1, 17, 0xe7f8);

	mii_mgr_write(1, 31, 0x8000); //local, page 0
	mii_mgr_write(0, 30, 0xa000);
	mii_mgr_write(1, 30, 0xa000);
	mii_mgr_write(2, 30, 0xa000);
	mii_mgr_write(3, 30, 0xa000);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 30, 0xa000);
#endif

	mii_mgr_write(0, 4, 0x05e1);
	mii_mgr_write(1, 4, 0x05e1);
	mii_mgr_write(2, 4, 0x05e1);
	mii_mgr_write(3, 4, 0x05e1);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 4, 0x05e1);
#endif

	mii_mgr_write(1, 31, 0xa000); // local, page 2
	mii_mgr_write(0, 16, 0x1111);
	mii_mgr_write(1, 16, 0x1010);
	mii_mgr_write(2, 16, 0x1515);
	mii_mgr_write(3, 16, 0x0f0f);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 16, 0x1313);
#endif

	/* disable 802.3az EEE by default */
	mii_mgr_write(1, 31, 0xb000); //local, page 3
	mii_mgr_write(0, 17, 0x0000);
	mii_mgr_write(1, 17, 0x0000);
	mii_mgr_write(2, 17, 0x0000);
	mii_mgr_write(3, 17, 0x0000);
#if !defined (CONFIG_RAETH_HAS_PORT4)
	mii_mgr_write(4, 17, 0x0000);
#endif
#endif

	if ((ralink_asic_rev_id & 0xf) >= 5)
		*(volatile u32 *)(RALINK_ETH_SW_BASE+0x701c) = 0x0800000c; // enlarge FE2SW_IPG

	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0004) = 0x00000007;	// PPE_PORT=7, PPE_EN=0
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x270c) = 0x000fff10;	// disable P7 mac learning
#if defined (CONFIG_RAETH_ESW)
	/* Use internal switch, enable vlan control, enable egress tags */
#if defined (CONFIG_RAETH_HAS_PORT5)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2704) = 0x20ff0003;	// P7 has security mode, egress always tagged
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2604) = 0x20ff0003;	// P6 has security mode, egress always tagged
#else
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2704) = 0x20df0003;	// P7 has security mode, egress always tagged
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2604) = 0x20df0003;	// P6 has security mode, egress always tagged
#endif
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2710) = 0x81000000;	// P7 is user port, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2610) = 0x81000000;	// P6 is user port, admit all frames
#else /* !CONFIG_RAETH_ESW */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x7014) = 0x1fec000c;	// disable internal PHY 0~4, set internal PHY base address to 12
#if defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_HAS_PORT4)
	/* Use single external P5, disable ports learning and vlan control */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2704) = 0x00e00000;	// P7 has matrix mode (P7|P6|P5)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2604) = 0x00e00000;	// P6 has matrix mode (P7|P6|P5)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2504) = 0x00600000;	// P5 has matrix mode (P6|P5)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2710) = 0x810080c0;	// P7 is transparent port, disable PVID insert, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2610) = 0x810080c0;	// P6 is transparent port, disable PVID insert, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2510) = 0x810080c0;	// P5 is transparent port, disable PVID insert, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x260c) = 0x000fff10;	// disable P6 mac learning
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x250c) = 0x000fff10;	// disable P5 mac learning
#elif defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_HAS_PORT5)
	/* Use single external P4, disable ports learning and vlan control */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2704) = 0x00d00000;	// P7 has matrix mode (P7|P6|P4)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2604) = 0x00d00000;	// P6 has matrix mode (P7|P6|P4)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2404) = 0x00500000;	// P4 has matrix mode (P6|P4)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2710) = 0x810080c0;	// P7 is transparent port, disable PVID insert, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2610) = 0x810080c0;	// P6 is transparent port, disable PVID insert, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2410) = 0x810080c0;	// P4 is transparent port, disable PVID insert, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x260c) = 0x000fff10;	// disable P6 mac learning
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x240c) = 0x000fff10;	// disable P4 mac learning
#else
	/* Use both external P5 & P4, enable vlan control, enable egress tags */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2704) = 0x20f00003;	// P7 has security mode, egress always tagged
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2604) = 0x20f00003;	// P6 has security mode, egress always tagged
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2710) = 0x81000000;	// P7 is user port, admit all frames
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x2610) = 0x81000000;	// P6 is user port, admit all frames
#endif
#endif

	/* Port 6 (CPU) */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3600) = 0x0005e33b;	// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0010) = 0x7f7f7fe0;	// Set Port6 CPU Port

	/* Port 5 */
#if defined (CONFIG_RAETH_HAS_PORT5)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=RGMii Mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_GE1;	// set GE1 to Normal mode
#else
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x00008000;	// P5 link down
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |=  RALINK_GPIOMODE_GE1;	// set GE1 to GPIO mode
#endif
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	/* Use P5 for connect to external RGMII MAC */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x0005e33b;	// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P5_RGMII_TO_MT7530_MODE)
	/* Use P5 for connect to external MT7530 (P6) */
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x0005e33b;	// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
	/* Initial config MT7530 via MDIO */
	mt7530_gsw_init();
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	/* Use P5 for connect to external MII MAC */
	*(volatile u32 *)(REG_SYSCFG1) |= (0x1 << 12);			// GE1_MODE=Mii Mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x0005e337;	// (P5, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	/* Use P5 for connect to external RvMII MAC */
	*(volatile u32 *)(REG_SYSCFG1) |= (0x2 << 12);			// GE1_MODE=RvMii Mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x0005e337;	// (P5, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	/* Use P5 for connect to external GigaPHY (with autopolling) */
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3500) = 0x00056330;	// (P5, AN)
	init_giga_phy(1);
	enable_autopoll_phy(1);
#endif

	/* Port 4 */
#if defined (CONFIG_RAETH_HAS_PORT4)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 14);			// GE2_MODE=RGMii Mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_GE2;	// set GE2 to Normal mode
#else
	*(volatile u32 *)(REG_SYSCFG1) |=  (0x3 << 14);			// GE2_MODE=RJ45 Mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |=  RALINK_GPIOMODE_GE2;	// set GE2 to GPIO mode
#endif
#if defined (CONFIG_P4_RGMII_TO_MAC_MODE)
	/* Use P4 for connect to external RGMII MAC */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x0005e33b;	// (P4, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_MII_TO_MAC_MODE)
	/* Use P4 for connect to external MII MAC */
	*(volatile u32 *)(REG_SYSCFG1) |= (0x1 << 14);			// GE2_MODE=Mii Mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x0005e337;	// (P4, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_RMII_TO_MAC_MODE)
	/* Use P4 for connect to external RvMII MAC */
	*(volatile u32 *)(REG_SYSCFG1) |= (0x2 << 14);			// GE2_MODE=RvMii Mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x0005e337;	// (P4, Force mode, Link Up, 100Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_MAC_TO_PHY_MODE)
	/* Use P4 for connect to external GigaPHY (with autopolling) */
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x00056330;	// (P4, AN)
	init_giga_phy(2);
	enable_autopoll_phy(1);
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4) || defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0)
	/* Use P4 for connect to external MT7530 GigaPHY P4 or P0 (with autopolling) */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x3400) = 0x00056330;	// (P4, AN)
	enable_autopoll_phy(1);
#endif
}
#endif


#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
static void rt305x_esw_init(void)
{
	int i=0;
	u32 phy_val=0, val=0;
#if defined (CONFIG_RT3052_ASIC)
	u32 phy_val2;
#endif

#if defined (CONFIG_RT5350_ASIC) || defined (CONFIG_MT7628_ASIC)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0168) = 0x17;
#endif

	/*
	 * FC_RLS_TH=200, FC_SET_TH=160
	 * DROP_RLS=120, DROP_SET_TH=80
	 */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0008) = 0xC8A07850;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00E4) = 0x00000000;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0014) = 0x00405555;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0050) = 0x00002001;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0090) = 0x00007f7f;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0098) = 0x00007f3f; //disable VLAN
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00CC) = 0x0002500c;
#ifndef CONFIG_UNH_TEST
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x009C) = 0x0008a301; //hashing algorithm=XOR48, aging interval=300sec
#else
	/*
	 * bit[30]:1	Backoff Algorithm Option: The latest one to pass UNH test
	 * bit[29]:1	Length of Received Frame Check Enable
	 * bit[8]:0	Enable collision 16 packet abort and late collision abort
	 * bit[7:6]:01	Maximum Packet Length: 1518
	 */
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x009C) = 0x6008a241;
#endif
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x008C) = 0x02404040;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) = 0x3f502b28; //Change polling Ext PHY Addr=0x1F
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0084) = 0x00000000;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x0110) = 0x7d000000; //1us cycle number=125 (FE's clock=125Mhz)
	
	/*
	 * set port 5 force to 1000M/Full when connecting to switch or iNIC
	 */
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3fff;		// force 1000M full duplex
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0xf<<20);	// rxclk_skew, txclk_skew = 0
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;		// force 100M full duplex
#if defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=Mii Mode
	*(volatile u32 *)(REG_SYSCFG1) |= (0x1 << 12);
#endif
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
	init_giga_phy(1);
#if defined (CONFIG_RT3052_ASIC) || defined (CONFIG_RT3352_ASIC)
	enable_autopoll_phy(1);
#endif
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);		// set RGMII to Normal mode
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);	// disable port 5 auto-polling
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;		// force 100M full duplex
#if defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 12);			// GE1_MODE=RvMii Mode
	*(volatile u32 *)(REG_SYSCFG1) |= (0x2 << 12);
#endif
#else // Port 5 Disabled //

#if defined (CONFIG_RALINK_RT3052)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);	// port5 auto polling disable
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_MDIO;	// set MDIO to GPIO mode (GPIO22-GPIO23)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_RGMII;// set RGMII to GPIO mode (GPIO41-GPIO50)
	*(volatile u32 *)(0xb0000674) = 0xFFF;				// GPIO41-GPIO50 output mode
	*(volatile u32 *)(0xb000067C) = 0x0;				// GPIO41-GPIO50 output low
#elif defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);	// port5 auto polling disable
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_MDIO;	// set MDIO to GPIO mode (GPIO22-GPIO23)
	*(volatile u32 *)(0xb0000624) = 0xC0000000;			// GPIO22-GPIO23 output mode
	*(volatile u32 *)(0xb000062C) = 0xC0000000;			// GPIO22-GPIO23 output high
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_GE1;	// set RGMII to GPIO mode (GPIO24-GPIO35)
	*(volatile u32 *)(0xb000064C) = 0xFFF;				// GPIO24-GPIO35 output mode
	*(volatile u32 *)(0xb0000654) = 0xFFF;				// GPIO24-GPIO35 output high
#elif defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	/* do nothing */
#endif
#endif // CONFIG_P5_RGMII_TO_MAC_MODE //

#if defined (CONFIG_RAETH_ESW)
#if defined (CONFIG_RT3052_ASIC)
	rw_rf_reg(0, 0, &phy_val);
	phy_val = phy_val >> 4;

	if(phy_val > 0x5) {
		rw_rf_reg(0, 26, &phy_val);
		phy_val2 = (phy_val | (0x3 << 5));
		rw_rf_reg(1, 26, &phy_val2);
		
		// reset EPHY
		val = sysRegRead(REG_RSTCTRL);
		val = val | RALINK_EPHY_RST;
		sysRegWrite(RSTCTRL, val);
		val = val & ~(RALINK_EPHY_RST);
		sysRegWrite(REG_RSTCTRL, val);
		
		rw_rf_reg(1, 26, &phy_val);
		
		//select local register
		mii_mgr_write(0, 31, 0x8000);
		for(i=0;i<5;i++){
			mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
			mii_mgr_write(i, 29, 0x7058);   //TX100/TX10 AD/DA current bias
			mii_mgr_write(i, 30, 0x0018);   //TX100 slew rate control
		}
		
		//select global register
		mii_mgr_write(0, 31, 0x0);
		mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
		mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
		mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
//#define ENABLE_LDPS
#if defined (ENABLE_LDPS)
		mii_mgr_write(0, 12, 0x7eaa);
		mii_mgr_write(0, 22, 0x252f); //tune TP_IDL tail and head waveform, enable power down slew rate control
#else
		mii_mgr_write(0, 12, 0x0);
		mii_mgr_write(0, 22, 0x052f);
#endif
		mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
		mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
		mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
		mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
		mii_mgr_write(0, 27, 0x2fce); //set PLL/Receive bias current are calibrated
		mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
		mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
		mii_mgr_write(0, 31, 0x8000); //select local register
		
		for(i=0;i<5;i++){
			//LSB=1 enable PHY
			mii_mgr_read(i, 26, &phy_val);
			phy_val |= 0x0001;
			mii_mgr_write(i, 26, phy_val);
		}
	} else {
		//select local register
		mii_mgr_write(0, 31, 0x8000);
		for(i=0;i<5;i++){
			mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
			mii_mgr_write(i, 29, 0x7058);   //TX100/TX10 AD/DA current bias
			mii_mgr_write(i, 30, 0x0018);   //TX100 slew rate control
		}
		
		//select global register
		mii_mgr_write(0, 31, 0x0);
		mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
		mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
		mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
		mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
		mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
		mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
		mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
		mii_mgr_write(0, 22, 0x052f); //tune TP_IDL tail and head waveform
		mii_mgr_write(0, 27, 0x2fce); //set PLL/Receive bias current are calibrated
		mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
		mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
		mii_mgr_write(0, 31, 0x8000); //select local register
		
		for(i=0;i<5;i++){
			//LSB=1 enable PHY
			mii_mgr_read(i, 26, &phy_val);
			phy_val |= 0x0001;
			mii_mgr_write(i, 26, phy_val);
		}
	}
#elif defined (CONFIG_RT3352_ASIC)
	//PHY IOT
	// reset EPHY
	val = sysRegRead(REG_RSTCTRL);
	val = val | RALINK_EPHY_RST;
	sysRegWrite(REG_RSTCTRL, val);
	val = val & ~(RALINK_EPHY_RST);
	sysRegWrite(REG_RSTCTRL, val);

	//select local register
	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
		mii_mgr_write(i, 29, 0x7016);   //TX100/TX10 AD/DA current bias
		mii_mgr_write(i, 30, 0x0038);   //TX100 slew rate control
	}

	//select global register
	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	mii_mgr_write(0, 22, 0x253f); //tune TP_IDL tail and head waveform, enable power down slew rate control
	mii_mgr_write(0, 27, 0x2fda); //set PLL/Receive bias current are calibrated
	mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	mii_mgr_write(0, 31, 0x8000); //select local register

	for(i=0;i<5;i++){
		//LSB=1 enable PHY
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	}

#elif defined (CONFIG_RT5350_ASIC) || defined (CONFIG_MT7628_ASIC)
	//PHY IOT
	// reset EPHY
	val = sysRegRead(REG_RSTCTRL);
	val = val | RALINK_EPHY_RST;
	sysRegWrite(REG_RSTCTRL, val);
	val = val & ~(RALINK_EPHY_RST);
	sysRegWrite(REG_RSTCTRL, val);

	//select local register
	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
		mii_mgr_write(i, 29, 0x7015);   //TX100/TX10 AD/DA current bias
		mii_mgr_write(i, 30, 0x0038);   //TX100 slew rate control
	}

	//select global register
	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	mii_mgr_write(0, 22, 0x253f); //tune TP_IDL tail and head waveform, enable power down slew rate control
	mii_mgr_write(0, 27, 0x2fda); //set PLL/Receive bias current are calibrated
	mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	mii_mgr_write(0, 31, 0x8000); //select local register

	for(i=0;i<5;i++){
		//LSB=1 enable PHY
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	}
#else
#error "Chip is not supported"
#endif
#endif
}
#endif

void fe_phy_init(void)
{
#if defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY)
	u32 regValue = 0;
#endif

	/* Case1: RT305x/RT335x/RT5350/MT7620/MT7628 + ESW/GSW/P5/P4 */
#if defined (CONFIG_RALINK_MT7620)
	mt7620_gsw_init();
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
      defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	rt305x_esw_init();
#endif

	/* Case2: RT3883/MT7621 GE1 + GSW */
#if defined (CONFIG_GE1_RGMII_FORCE_1000) || defined (CONFIG_GE1_TRGMII_FORCE_1200)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 12);			// GMAC1 = RGMII mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_GE1;	// GE1 = Normal mode
#if defined (CONFIG_RALINK_MT7621)
	/* MT7621 GE1 + Internal GSW (MT7530) */
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
	mt7530_gsw_init();
#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	/* MT7621 GE2 + Internal GPHY P4 or P0 (with autopolling) */
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 14);			// GMAC2 = RGMII mode
	*(volatile u32 *)(REG_PAD_RGMII2_MDIO_CFG) &= ~(0x3 << 4);	// reduce RGMII2 PAD driving strength
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_GE2;	// GE2 = Normal mode
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x20056300);		// (P1, AN)
	enable_autopoll_phy(1);
#endif
#else
	/* RT3883 GE1 + External GSW */
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#endif

	/* Case3: RT3883/MT7621 GE2 + External GSW */
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 14);			// GMAC2 = RGMII mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_GE2;	// GE2 = Normal mode
#if defined (CONFIG_RALINK_MT7621)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x0005633b);		// (GE2, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#else
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#endif
#endif

	/* Case4: RT3883/MT7621 GE1 + GigaPhy */
#if defined (CONFIG_GE1_RGMII_AN)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 12);			// GMAC1 = RGMII mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_GE1;	// GE1 = Normal mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
#if defined (CONFIG_RALINK_MT7621)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x20056300);		// (GE1, AN)
#endif
	init_giga_phy(1);
	enable_autopoll_phy(1);
#endif

	/* Case5: RT3883/MT7621 GE2 + GigaPhy */
#if defined (CONFIG_GE2_RGMII_AN)
	*(volatile u32 *)(REG_SYSCFG1) &= ~(0x3 << 14);			// GMAC2 = RGMII mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_GE2;	// GE2 = Normal mode
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;// set MDIO to Normal mode
#if defined (CONFIG_RALINK_MT7621)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x20056300);		// (GE2, AN)
#endif
	init_giga_phy(2);
	enable_autopoll_phy(2);
#endif

	/* Case6: RT3883, MT7621 GE1/GE2 + (10/100 Switch or 100PHY) */
#if defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY)

#if defined (CONFIG_RALINK_RT3883)

	// set RT3883 GMAC to MII or RvMII mode
	regValue = sysRegRead(REG_SYSCFG1);
#if defined (CONFIG_GE1_MII_FORCE_100) || defined (CONFIG_GE1_MII_AN)
	regValue &= ~(0x3 << 12);
	regValue |=  (0x1 << 12);					// GE1 MII Mode
#elif defined (CONFIG_GE1_RVMII_FORCE_100)
	regValue &= ~(0x3 << 12);
	regValue |=  (0x2 << 12);					// GE1 RvMII Mode
#endif
#if defined (CONFIG_GE2_MII_FORCE_100) || defined (CONFIG_GE2_MII_AN)
	regValue &= ~(0x3 << 14);
	regValue |=  (0x1 << 14);					// GE2 MII Mode
#elif defined (CONFIG_GE2_RVMII_FORCE_100)
	regValue &= ~(0x3 << 14);
	regValue |=  (0x2 << 14);					// GE2 RvMII Mode
#endif
	sysRegWrite(REG_SYSCFG1, regValue);
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
#if defined (CONFIG_GE1_MII_AN) || defined (CONFIG_GE2_MII_AN)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(0x3 << 12);		// set MDIO to Normal mode
	enable_autopoll_phy(1);
#endif
#if defined (CONFIG_GE1_MII_AN)
	sysRegWrite(RALINK_ETH_SW_BASE+0x100, 0x20056300);		// (GE1, AN)
#endif
#if defined (CONFIG_GE2_MII_AN)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x20056300);		// (GE2, AN)
#endif

#endif /* CONFIG_RALINK_MT7621 */

#endif /* CONFIG_RAETH_ROUTER || CONFIG_100PHY */

#if defined (CONFIG_RALINK_MT7621) && !defined (CONFIG_RAETH_GMAC2)
	sysRegWrite(RALINK_ETH_SW_BASE+0x200, 0x00008000);		// (GE2, Link down)
#endif

#if defined (CONFIG_RAETH_ESW_CONTROL)
	esw_control_post_init();
#endif
}

