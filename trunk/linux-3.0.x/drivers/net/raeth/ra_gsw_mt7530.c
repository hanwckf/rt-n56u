#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/string.h>

#include "mii_mgr.h"
#include "ra_esw_reg.h"
#include "ra_gsw_mt7530.h"

/* external GSW MT7350 */
void mt7530_gsw_init(void)
{
	u32 i, regValue = 0;

	/* configure MT7530 HW-TRAP */
	mii_mgr_read(MT7530_MDIO_ADDR, 0x7804, &regValue);
	regValue |= (1<<16);						// Change HW-TRAP
	regValue |= (1<<24);						// Standalone Switch
	regValue &= ~(1<<8);						// Enable Port 6
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5)
	regValue &= ~((1<<6)|(1<<5)|(1<<15));				// Enable Port5
	regValue |= ((1<<13)|(1<<7));					// Port 5 as GMAC, no Internal PHY mode
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0)
	regValue &= ~((1<<6)|(1<<5)|(1<<15)|(1<<13));			// Enable Port5, set to PHY P0 mode
	regValue |= ((1<<7)|(1<<20));
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	regValue &= ~((1<<6)|(1<<5)|(1<<15)|(1<<13)|(1<<20));		// Enable Port5, set to PHY P4 mode
	regValue |= ((1<<7));
#else
	regValue |= ((1<<6)|(1<<13));					// Disable Port 5, set as GMAC, no Internal PHY mode
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7804, regValue);

	/* configure MT7530 Port6 */
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x204f0003);		// P6 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x81000000);		// P6 is user port, admit all frames
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x004f0000);		// P6 has matrix mode (P6|P3|P2|P1|P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x005e0000);		// P6 has matrix mode (P6|P4|P3|P2|P1)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x810000c0);		// P6 is transparent port, admit all frames
#else
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2604, 0x205f0003);		// P6 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2610, 0x81000000);		// P6 is user port, admit all frames
#endif
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3600, 0x0005e33b);		// (P6, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)

	/* configure MT7530 Port5 */
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2504, 0x20300003);		// P5 set security mode, egress always tagged
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2510, 0x81000000);		// P5 is user port, admit all frames
	mii_mgr_write(MT7530_MDIO_ADDR, 0x2514, 0x00010002);		// P5 PVID=2
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x0005e33b);		// (P5, Force mode, Link Up, 1000Mbps, Full-Duplex, FC ON)
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4) || defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0)
	mii_mgr_write(MT7530_MDIO_ADDR, 0x3500, 0x00056300);		// (P5, AN) ???
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

	/* set MT7530 delay setting for 10/1000M */
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b00, 0x102);
	mii_mgr_write(MT7530_MDIO_ADDR, 0x7b04, 0x14);

	/* disable EEE */
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
}

