#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "ra_eth_reg.h"
#include "mii_mgr.h"

/* EPHY Vendor ID list */
#define EV_ICPLUS_PHY_ID0		0x0243
#define EV_ICPLUS_PHY_ID1		0x0D90

#define EV_REALTEK_PHY_ID0		0x001C
#define EV_REALTEK_PHY_ID1		0xC910

#define EV_MARVELL_PHY_ID0		0x0141
#define EV_MARVELL_PHY_ID1		0x0CC2

#define EV_VTSS_PHY_ID0			0x0007
#define EV_VTSS_PHY_ID1			0x0421

#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE) || \
    defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_P4_MAC_TO_PHY_MODE)
void ext_gphy_init(u32 phy_addr)
{
	const char *phy_devn = NULL;
	u32 phy_id0 = 0, phy_id1 = 0, phy_val = 0, phy_rev;

	if (!mii_mgr_read(phy_addr, 2, &phy_id0))
		return;
	if (!mii_mgr_read(phy_addr, 3, &phy_id1))
		return;

	phy_rev = phy_id1 & 0xf;

	if ((phy_id0 == EV_ICPLUS_PHY_ID0) && ((phy_id1 & 0xfff0) == EV_ICPLUS_PHY_ID1)) {
		phy_devn = "IC+ IP1001";
		mii_mgr_read(phy_addr, 4, &phy_val);
		phy_val |= (1<<10);			// enable pause ability
		mii_mgr_write(phy_addr, 4, phy_val);
		mii_mgr_read(phy_addr, 0, &phy_val);
		if (!(phy_val & (1<<11))) {
			phy_val |= (1<<9);		// restart AN
			mii_mgr_write(phy_addr, 0, phy_val);
		}
	} else
	if ((phy_id0 == EV_REALTEK_PHY_ID0) && ((phy_id1 & 0xfff0) == EV_REALTEK_PHY_ID1)) {
		phy_devn = "RTL8211";
		if (phy_rev == 0x6) {
			phy_devn = "RTL8211F";
			
			/* Disable response on MDIO addr 0 (!) */
			mii_mgr_read(phy_addr, 24, &phy_val);
			phy_val &= ~(1<<13);		// PHYAD_0 Disable
			mii_mgr_write(phy_addr, 24, phy_val);
			
			/* set RGMII mode */
			mii_mgr_write(phy_addr, 31, 0x0d08);
			mii_mgr_read(phy_addr, 17, &phy_val);
			phy_val |= (1<<8);		// enable TXDLY
			mii_mgr_write(phy_addr, 17, phy_val);
			mii_mgr_write(phy_addr, 31, 0x0000);
			
			/* Disable Green Ethernet */
			mii_mgr_write(phy_addr, 27, 0x8011);
			mii_mgr_write(phy_addr, 28, 0x573f);
		} else if (phy_rev == 0x5) {
			phy_devn = "RTL8211E";
			
			/* Disable Green Ethernet */
			mii_mgr_write(phy_addr, 31, 0x0003);
			mii_mgr_write(phy_addr, 25, 0x3246);
			mii_mgr_write(phy_addr, 16, 0xa87c);
			mii_mgr_write(phy_addr, 31, 0x0000);
		}
		if (phy_rev >= 0x4) {
			/* disable EEE LPI 1000/100 advert (for D/E/F) */
			mii_mgr_write_cl45(phy_addr, 0x07, 0x003c, 0x0000);
		}
	} else
	if ((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1)) {
		phy_devn = "Marvell";
		mii_mgr_read(phy_addr, 20, &phy_val);
		phy_val |= (1<<7);			// add delay to RX_CLK for RXD Outputs
		mii_mgr_write(phy_addr, 20, phy_val);
		mii_mgr_read(phy_addr, 0, &phy_val);
		phy_val |= (1<<15);			// PHY Software Reset
		mii_mgr_write(phy_addr, 0, phy_val);
	} else
	if ((phy_id0 == EV_VTSS_PHY_ID0) && (phy_id1 == EV_VTSS_PHY_ID1)) {
		phy_devn = "Vitesse VSC8601";
		mii_mgr_write(phy_addr, 31, 0x0001);	// extended page
		mii_mgr_read(phy_addr, 28, &phy_val);
		phy_val |=  (0x3<<12);			// RGMII RX skew compensation= 2.0 ns
		phy_val &= ~(0x3<<14);			// RGMII TX skew compensation= 0 ns
		mii_mgr_write(phy_addr, 28, phy_val);
		mii_mgr_write(phy_addr, 31, 0x0000);	// main registers
	}

	if (phy_devn)
		printk("%s GPHY detected on MDIO addr 0x%02X\n", phy_devn, phy_addr);
	else
		printk("Unknown EPHY (%04X:%04X) detected on MDIO addr 0x%02X\n",
			phy_id0, phy_id1, phy_addr);
}

void ext_gphy_eee_enable(u32 phy_addr, int is_eee_enabled)
{
	u32 phy_id0 = 0, phy_id1 = 0, phy_rev;

	if (!mii_mgr_read(phy_addr, 2, &phy_id0))
		return;
	if (!mii_mgr_read(phy_addr, 3, &phy_id1))
		return;

	phy_rev = phy_id1 & 0xf;

	if ((phy_id0 == EV_REALTEK_PHY_ID0) && ((phy_id1 & 0xfff0) == EV_REALTEK_PHY_ID1)) {
		if (phy_rev >= 0x4) {
			/* EEE LPI 1000/100 advert (for D/E/F) */
			mii_mgr_write_cl45(phy_addr, 0x07, 0x003c, (is_eee_enabled) ? 0x0006 : 0x0000);
		}
	}
}
#endif

#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR) || defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7620)
void enable_autopoll_phy(int unused)
{
	u32 regValue, addr_s, addr_e;


#if defined (CONFIG_RALINK_MT7621)
	// PHY_ST_ADDR  = always GE1->EPHY address
	// PHY_END_ADDR = always GE2->EPHY address
#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2) && defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR)
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#elif defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2-1;	// not used
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#else
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR+1;	// not used
#endif
#elif defined (CONFIG_RALINK_MT7620)
	// PHY_ST_ADDR  = always P4->EPHY address
	// PHY_END_ADDR = always P5->EPHY address
#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2) && defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR)
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#elif defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2+1;	// not used
#else
	addr_s = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR-1;	// not used
	addr_e = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif
#endif

	regValue = sysRegRead(REG_MDIO_PHY_POLLING);
	regValue |= (1UL<<31);
	regValue &= ~(0x1f);
	regValue &= ~(0x1f<<8);
	regValue |= (addr_s & 0x1f);		// setup PHY address for auto polling (Start Addr).
	regValue |= ((addr_e & 0x1f) << 8);	// setup PHY address for auto polling (End Addr).
	sysRegWrite(REG_MDIO_PHY_POLLING, regValue);
}
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
      defined (CONFIG_RALINK_RT3883)
void enable_autopoll_phy(int ge)
{
	u32 regAddr, regValue;

	regAddr = REG_MDIO_PHY_POLLING;
#if defined (CONFIG_RALINK_RT3883) && defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	if (ge == 2)
		regAddr = MDIO_CFG2;
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

/* called once on driver load */
void early_phy_init(void)
{
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
#if defined (CONFIG_P5_MAC_TO_PHY_MODE) || defined (CONFIG_GE2_RGMII_AN)
#define MAX_PHY_NUM	6
#else
#define MAX_PHY_NUM	5
#endif
	u32 i, phy_mdio_addr, phy_reg_mcr;
#endif

#if defined (CONFIG_P5_MAC_TO_PHY_MODE) || defined (CONFIG_GE1_RGMII_AN) || \
    defined (CONFIG_P4_MAC_TO_PHY_MODE) || defined (CONFIG_GE2_RGMII_AN) || \
    defined (CONFIG_MT7530_GSW)
	/* enable MDIO port */
	mii_mgr_init();
#endif

#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
	/* early down all switch PHY (please enable from user-level) */
	for (i = 0; i < MAX_PHY_NUM; i++) {
		phy_mdio_addr = i;
#if defined (CONFIG_P4_MAC_TO_PHY_MODE)
		if (i == 4)
			phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#endif
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
		if (i == 5)
			phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#elif defined (CONFIG_GE2_RGMII_AN)
		if (i == 5)
			phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#endif
		phy_reg_mcr = 0x3100;
		if (mii_mgr_read(phy_mdio_addr, 0, &phy_reg_mcr)) {
			if (phy_reg_mcr & (1<<11))
				continue;
			phy_reg_mcr &= ~(1<<9);
			phy_reg_mcr |= ((1<<12)|(1<<11));
			mii_mgr_write(phy_mdio_addr, 0, phy_reg_mcr);
		}
	}
#endif

#if defined (CONFIG_MT7530_GSW)
	/* early P5/P6 MAC link down */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x8000);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3600, 0x8000);
#endif
}

