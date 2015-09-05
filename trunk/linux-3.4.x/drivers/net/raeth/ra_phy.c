#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "ra_eth_reg.h"
#include "mii_mgr.h"

/*  PHY Vender ID list */
#define EV_ICPLUS_PHY_ID0		0x0243
#define EV_ICPLUS_PHY_ID1		0x0D90

#define EV_MARVELL_PHY_ID0		0x0141
#define EV_MARVELL_PHY_ID1		0x0CC2

#define EV_VTSS_PHY_ID0			0x0007
#define EV_VTSS_PHY_ID1			0x0421

#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE) || \
    defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_P4_MAC_TO_PHY_MODE)
void init_ext_giga_phy(int ge)
{
	u32 phy_id0 = 0, phy_id1 = 0, phy_val = 0;
#if defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2) && defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR)
	u32 phy_addr = (ge == 2) ? CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 : CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#elif defined (CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2)
	u32 phy_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#else
	u32 phy_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif

	if (!mii_mgr_read(phy_addr, 2, &phy_id0))
		return;
	if (!mii_mgr_read(phy_addr, 3, &phy_id1))
		return;

	if ((phy_id0 == EV_ICPLUS_PHY_ID0) && ((phy_id1 & 0xfff0) == EV_ICPLUS_PHY_ID1)) {
		printk("%s GigaPHY detected\n", "IC+");
		mii_mgr_read(phy_addr, 4, &phy_val);
		phy_val |= (1<<10);			// enable pause ability
		mii_mgr_write(phy_addr, 4, phy_val);
#if !defined (CONFIG_RAETH_ESW_CONTROL)
		mii_mgr_read(phy_addr, 0, &phy_val);
		phy_val |= (1<<9);			// restart AN
		mii_mgr_write(phy_addr, 0, phy_val);
#endif
	} else
	if ((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1)) {
		printk("%s GigaPHY detected\n", "Marvell");
		mii_mgr_read(phy_addr, 20, &phy_val);
		phy_val |= (1<<7);			// add delay to RX_CLK for RXD Outputs
		mii_mgr_write(phy_addr, 20, phy_val);
		mii_mgr_read(phy_addr, 0, &phy_val);
		phy_val |= (1<<15);			// PHY Software Reset
		mii_mgr_write(phy_addr, 0, phy_val);
	} else
	if ((phy_id0 == EV_VTSS_PHY_ID0) && (phy_id1 == EV_VTSS_PHY_ID1)) {
		printk("%s GigaPHY detected\n", "Vitesse");
		mii_mgr_write(phy_addr, 31, 0x0001);	// extended page
		mii_mgr_read(phy_addr, 28, &phy_val);
		phy_val |=  (0x3<<12);			// RGMII RX skew compensation= 2.0 ns
		phy_val &= ~(0x3<<14);			// RGMII TX skew compensation= 0 ns
		mii_mgr_write(phy_addr, 28, phy_val);
		mii_mgr_write(phy_addr, 31, 0x0000);	// main registers
	} else {
		printk("Unknown EPHY detected (ID0: 0x%04X, ID1: 0x%04X)\n",
			phy_id0, phy_id1);
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

#if defined (CONFIG_P5_MAC_TO_PHY_MODE) || defined (CONFIG_MT7530_GSW) || \
    defined (CONFIG_P4_MAC_TO_PHY_MODE) || defined (CONFIG_GE2_RGMII_AN)
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
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
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

