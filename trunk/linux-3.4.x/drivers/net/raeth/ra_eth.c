#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/netdevice.h>

#include "ra_compat.h"
#include "ra_eth_reg.h"
#include "ra_phy.h"
#include "ra_esw_rt305x.h"
#include "ra_esw_mt7620.h"
#include "ra_eth_mt7621.h"
#include "ra_gsw_mt7530.h"

void fe_eth_reset(void)
{
	u32 val;

	val = sysRegRead(REG_RSTCTRL);

	/* RT5350/MT7628 (SDMA) need to reset ESW and FE at the same to avoid PDMA panic */
#if defined (RAETH_SDMA)
	val |= RALINK_ESW_RST;
#endif

	/* Reset PPE at this point */
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	val |= RALINK_PPE_RST;
#endif

	/* MT7621 + TRGMII > 1000 need to reset GMAC */
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_GE1_TRGMII_FORCE_1200)
	val |= RALINK_ETH_RST;
#endif

	val |= RALINK_FE_RST;
	sysRegWrite(REG_RSTCTRL, val);
	udelay(10);

	/* set TRGMII_1200 clock */
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_GE1_TRGMII_FORCE_1200)
	{
		u32 val_ddr, val_clk;
		
		val_ddr = sysRegRead(REG_SYSCFG0);
		val_ddr = (val_ddr >> 4) & 0x1;
		
		val_clk = sysRegRead(REG_CLK_CFG_0);
		val_clk &= ~(0x3 << 5);
		if (val_ddr) {
			/* configure APLL to 300MHz if TRGMII_1200 + DDR2 */
			mt7621_apll_trgmii_enable();
			
			val_clk |= (0x2 << 5); // TRGMII clock source = APLL (300MHz)
		} else {
			val_clk |= (0x1 << 5); // TRGMII clock source = DDR3 PLL (300MHz)
		}
		sysRegWrite(REG_CLK_CFG_0, val_clk);
		udelay(1000);
	}
#endif

#if defined (RAETH_SDMA) || defined (CONFIG_RALINK_MT7620)
	val &= ~(RALINK_ESW_RST);
#endif

#if defined (CONFIG_RALINK_MT7620)
	val &= ~(RALINK_EPHY_RST);
#endif

#if defined (CONFIG_RALINK_MT7621)
	val &= ~(RALINK_ETH_RST | RALINK_MCM_RST);
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	val &= ~(RALINK_PPE_RST);
#endif

	val &= ~(RALINK_FE_RST);
	sysRegWrite(REG_RSTCTRL, val);
	udelay(1000);
}


void fe_gdm_init(struct net_device *dev)
{
#if defined (RAETH_SDMA)
	u32 regSDM;

	/* RT5350/MT7628 SDMA: No GDMA, PSE, CDMA, PPE */
	regSDM = sysRegRead(SDM_CON);

	if (dev->features & (NETIF_F_IP_CSUM | NETIF_F_RXCSUM))
		regSDM |=  (SDM_IPCS | SDM_TCPCS | SDM_UDPCS);
	else
		regSDM &= ~(SDM_IPCS | SDM_TCPCS | SDM_UDPCS);

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	regSDM |= SDM_TCI_81XX;
#endif
	sysRegWrite(SDM_CON, regSDM);

#else /* !RAETH_SDMA */

	u32 regGDM;

	regGDM = sysRegRead(GDMA1_FWD_CFG);

#if defined (CONFIG_RALINK_MT7620)
	/* GDMA1 frames destination port is port0 CPU */
	regGDM &= ~0x7;
#else
	/* set unicast/multicast/broadcast/other frames forward to CPU port (PDMA or QDMA) */
	regGDM &= ~0x7777;
#if defined (CONFIG_RAETH_QDMATX_QDMARX)
	regGDM |= (GDM1_UFRC_P_QDMA | GDM1_BFRC_P_QDMA | GDM1_MFRC_P_QDMA | GDM1_OFRC_P_QDMA);
#else
	regGDM |= (GDM1_UFRC_P_PDMA | GDM1_BFRC_P_PDMA | GDM1_MFRC_P_PDMA | GDM1_OFRC_P_PDMA);
#endif
#endif

	if (dev->features & (NETIF_F_IP_CSUM | NETIF_F_RXCSUM))
		regGDM |=  (GDM1_ICS_EN | GDM1_TCS_EN | GDM1_UCS_EN);
	else
		regGDM &= ~(GDM1_ICS_EN | GDM1_TCS_EN | GDM1_UCS_EN);

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	regGDM |= GDM1_TCI_81XX;
#endif

#if defined (CONFIG_RAETH_JUMBOFRAME)
	regGDM |= GDM1_JMB_EN;
	regGDM &= ~0xf0000000; /* clear bit28-bit31 */
	regGDM |= (((MAX_RX_LENGTH/1024)&0xf) << 28);
#endif

	sysRegWrite(GDMA1_FWD_CFG, regGDM);
#if defined (CONFIG_PSEUDO_SUPPORT)
	sysRegWrite(GDMA2_FWD_CFG, regGDM);
#endif

#endif /* RAETH_SDMA */
}

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (RAETH_HW_VLAN4K)
void fe_cdm_update_vlan_tx(const u16 *vlan_id_map)
{
	u32 i, reg_vlan;

	for (i = 0; i < 8; i++) {
		reg_vlan = ((u32)vlan_id_map[(i*2)+1] << 16) | (u32)vlan_id_map[i*2];
		sysRegWrite(CDMA_VLAN_ID0 + i*4, reg_vlan);
	}
}
#endif

void fe_cdm_init(struct net_device *dev)
{
#if !defined (RAETH_SDMA)
	u32 regCSG;
#if defined (CONFIG_RAETH_HW_VLAN_RX)
	u32 regCDMP_EG = 0x0;
	u32 regCDMQ_EG = 0x0;

	if (dev->features & NETIF_F_HW_VLAN_CTAG_RX) {
		regCDMP_EG = 0x1; // UNTAG_EN
#if defined (CONFIG_RAETH_QDMATX_QDMARX) && !defined (CONFIG_RA_HW_NAT_QDMA)
		/* PPE HW QoS going via CDMQ too, UNTAG_EN cause lost VLAN tag */
		regCDMQ_EG = 0x1; // UNTAG_EN
#endif
	}
	sysRegWrite(CDMP_EG_CTRL, regCDMP_EG);
	sysRegWrite(CDMQ_EG_CTRL, regCDMQ_EG);
#endif

	regCSG = sysRegRead(CDMA_CSG_CFG);
	regCSG &= ~0x7;

	if (dev->features & NETIF_F_IP_CSUM)
		regCSG |=  (ICS_GEN_EN | TCS_GEN_EN | UCS_GEN_EN);
	else
		regCSG &= ~(ICS_GEN_EN | TCS_GEN_EN | UCS_GEN_EN);

	sysRegWrite(CDMA_CSG_CFG, regCSG);
#endif
}

void fe_pse_init(void)
{
#if !defined (RAETH_SDMA)
/*
 *	PSE_FQ_CFG register definition -
 *
 *	Define max free queue page count in PSE. (31:24)
 *	RT3883 - 0xff908000 (255 pages)
 *	RT3052 - 0x80504000 (128 pages)
 *
 *	In each page, there are 128 bytes in each page.
 *
 *	23:16 - free queue flow control release threshold
 *	15:8  - free queue flow control assertion threshold
 *	7:0   - free queue empty threshold
 *
 *	The register affects QOS correctness in frame engine!
 */
#if defined (CONFIG_RALINK_RT3883)
	sysRegWrite(PSE_FQ_CFG, cpu_to_le32(INIT_VALUE_OF_RT3883_PSE_FQ_CFG));
#elif defined (CONFIG_RALINK_RT3052)
	sysRegWrite(PSE_FQ_CFG, cpu_to_le32(INIT_VALUE_OF_PSE_FQFC_CFG));
#endif
	/*
	 * Reset PSE after re-programming PSE_FQ_CFG.
	 */
	sysRegWrite(FE_RST_GLO, 1);
	sysRegWrite(FE_RST_GLO, 0);	// update for RSTCTL issue
#endif
}

void fe_dma_start(void)
{
	u32 dma_glo_cfg = (TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	dma_glo_cfg |= PDMA_BT_SIZE_16DWORDS;
#else
	dma_glo_cfg |= PDMA_BT_SIZE_8DWORDS;
#endif
#if defined (RAETH_PDMA_V2)
	dma_glo_cfg |= RX_2B_OFFSET;
#endif

#if defined (CONFIG_RAETH_QDMA)
	sysRegWrite(QDMA_GLO_CFG, dma_glo_cfg);
	dma_glo_cfg &= ~(TX_DMA_EN);	// PDMA use RX only
#if defined (CONFIG_RAETH_QDMATX_QDMARX)
	dma_glo_cfg &= ~(RX_DMA_EN);
	dma_glo_cfg |= CSR_CLKGATE;	// PDMA unused, enable power save
#endif
#endif
	sysRegWrite(PDMA_GLO_CFG, dma_glo_cfg);
}

void fe_dma_stop(void)
{
	int i;
	u32 regVal;

	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regVal);

#if defined (CONFIG_RAETH_QDMA)
	regVal = sysRegRead(QDMA_GLO_CFG);
	regVal &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(QDMA_GLO_CFG, regVal);
#endif

	/* wait FE stopped */
	for (i = 0; i < 50; i++) {
		msleep(10);
		regVal  = sysRegRead(PDMA_GLO_CFG);
#if defined (CONFIG_RAETH_QDMA)
		regVal |= sysRegRead(QDMA_GLO_CFG);
#endif
		if (!(regVal & (RX_DMA_BUSY | TX_DMA_BUSY)))
			break;
	}
}

void fe_irq_enable(void)
{
	/* clear pending INT and enable INT */
	sysRegWrite(FE_INT_STATUS, 0xffffffff);
	sysRegWrite(FE_INT_ENABLE, FE_INT_INIT_VALUE);
#if defined (FE_INT_ENABLE2)
	sysRegWrite(FE_INT_STATUS2, 0xffffffff);
	sysRegWrite(FE_INT_ENABLE2, FE_INT_INIT2_VALUE);
#endif
#if defined (CONFIG_RAETH_QDMA)
	sysRegWrite(QFE_INT_STATUS, 0xffffffff);
	sysRegWrite(QFE_INT_ENABLE, QFE_INT_INIT_VALUE);
#endif
}

void fe_irq_disable(void)
{
	sysRegWrite(FE_INT_ENABLE, 0);
#if defined (FE_INT_ENABLE2)
	sysRegWrite(FE_INT_ENABLE2, 0);
#endif
#if defined (CONFIG_RAETH_QDMA)
	sysRegWrite(QFE_INT_ENABLE, 0);
#endif
}

void fe_gdm1_set_mac(const u8 *mac)
{
	u32 regValue;

	regValue = ((u32)mac[0] << 8) | mac[1];
#if defined (RAETH_SDMA)
	sysRegWrite(SDM_MAC_ADRH, regValue);
#elif defined (CONFIG_RALINK_MT7620)
	sysRegWrite(SMACCR1, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRH, regValue);
#endif

	regValue = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) | ((u32)mac[4] << 8) | mac[5];
#if defined (RAETH_SDMA)
	sysRegWrite(SDM_MAC_ADRL, regValue);
#elif defined (CONFIG_RALINK_MT7620)
	sysRegWrite(SMACCR0, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRL, regValue);
#endif

#if defined (CONFIG_MT7530_GSW) && !defined (CONFIG_RALINK_MT7620)
	mt7530_gsw_set_smac(mac);
#endif
}

void fe_gdm1_fetch_mib(struct rtnl_link_stats64 *stat)
{
	u32 rx_fcs_bad;
#if !defined (RAETH_SDMA)
	u32 rx_too_sho;
	u32 rx_too_lon;
	u32 tx_skipped;

	stat->tx_bytes		+= sysRegRead(GDMA1_TX_GBCNT);
	stat->tx_packets	+= sysRegRead(GDMA1_TX_GPCNT);
	tx_skipped		 = sysRegRead(GDMA1_TX_SKIPCNT);
	stat->collisions	+= sysRegRead(GDMA1_TX_COLCNT);

	stat->rx_bytes		+= sysRegRead(GDMA1_RX_GBCNT);
	stat->rx_packets	+= sysRegRead(GDMA1_RX_GPCNT);
	stat->rx_over_errors	+= sysRegRead(GDMA1_RX_OERCNT);
	rx_fcs_bad		 = sysRegRead(GDMA1_RX_FERCNT);
	rx_too_sho		 = sysRegRead(GDMA1_RX_SERCNT);
	rx_too_lon		 = sysRegRead(GDMA1_RX_LERCNT);

	if (tx_skipped)
		stat->tx_dropped += tx_skipped;
	if (rx_too_sho)
		stat->rx_length_errors += rx_too_sho;
	if (rx_too_lon)
		stat->rx_length_errors += rx_too_lon;
#else
	stat->tx_bytes		+= sysRegRead(SDM_TBCNT);
	stat->tx_packets	+= sysRegRead(SDM_TPCNT);

	stat->rx_bytes		+= sysRegRead(SDM_RBCNT);
	stat->rx_packets	+= sysRegRead(SDM_RPCNT);
	rx_fcs_bad		 = sysRegRead(SDM_CS_ERR);
#endif

	if (rx_fcs_bad) {
		stat->rx_errors += rx_fcs_bad;
		stat->rx_crc_errors += rx_fcs_bad;
	}
}

#if defined (CONFIG_PSEUDO_SUPPORT)
void fe_gdm2_set_mac(const u8 *mac)
{
	u32 regValue;

	regValue = ((u32)mac[0] << 8) | mac[1];
	sysRegWrite(GDMA2_MAC_ADRH, regValue);

	regValue = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) | ((u32)mac[4] << 8) | mac[5];
	sysRegWrite(GDMA2_MAC_ADRL, regValue);
}

void fe_gdm2_fetch_mib(struct rtnl_link_stats64 *stat)
{
	u32 tx_skipped;
	u32 rx_fcs_bad;
	u32 rx_too_sho;
	u32 rx_too_lon;

	stat->tx_bytes		+= sysRegRead(GDMA2_TX_GBCNT);
	stat->tx_packets	+= sysRegRead(GDMA2_TX_GPCNT);
	tx_skipped		 = sysRegRead(GDMA2_TX_SKIPCNT);
	stat->collisions	+= sysRegRead(GDMA2_TX_COLCNT);

	stat->rx_bytes		+= sysRegRead(GDMA2_RX_GBCNT);
	stat->rx_packets	+= sysRegRead(GDMA2_RX_GPCNT);
	stat->rx_over_errors	+= sysRegRead(GDMA2_RX_OERCNT);
	rx_fcs_bad		 = sysRegRead(GDMA2_RX_FERCNT);
	rx_too_sho		 = sysRegRead(GDMA2_RX_SERCNT);
	rx_too_lon		 = sysRegRead(GDMA2_RX_LERCNT);

	if (tx_skipped)
		stat->tx_dropped += tx_skipped;

	if (rx_too_sho)
		stat->rx_length_errors += rx_too_sho;

	if (rx_too_lon)
		stat->rx_length_errors += rx_too_lon;

	if (rx_fcs_bad) {
		stat->rx_errors += rx_fcs_bad;
		stat->rx_crc_errors += rx_fcs_bad;
	}
}
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

#if defined (CONFIG_RALINK_RT3883)
static void rt3883_eth_init(void)
{
	/* RT3883 GE1 */
#if defined (CONFIG_GE1_RGMII_FORCE_1000)
	ge1_set_mode(0, 0);
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#elif defined (CONFIG_GE1_RGMII_AN)
	ge1_set_mode(0, 1);
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR);
#elif defined (CONFIG_GE1_MII_AN)
	ge1_set_mode(1, 1);
#elif defined (CONFIG_GE1_MII_FORCE_100)
	ge1_set_mode(1, 0);
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_100_FD);
#elif defined (CONFIG_GE1_RVMII_FORCE_100)
	ge1_set_mode(2, 0);
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_100_FD);
#elif defined (CONFIG_GE1_RGMII_NONE)
	sysRegWrite(MDIO_CFG, 0x1d201);
#endif

	/* RT3883 GE2 */
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	ge2_set_mode(0, 0);
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#elif defined (CONFIG_GE2_RGMII_AN)
	ge2_set_mode(0, 1);
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2);
#elif defined (CONFIG_GE2_MII_AN)
	ge2_set_mode(1, 1);
#elif defined (CONFIG_GE2_MII_FORCE_100)
	ge2_set_mode(1, 0);
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_100_FD);
#elif defined (CONFIG_GE2_RVMII_FORCE_100)
	ge2_set_mode(2, 0);
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_100_FD);
#endif

#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_GE1_MII_AN)
	enable_autopoll_phy(1);
#endif
#if defined (CONFIG_GE2_RGMII_AN) || defined (CONFIG_GE2_MII_AN)
	enable_autopoll_phy(2);
#endif
}
#endif

void fe_esw_init(void)
{
	/* init GE/ESW */
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	rt305x_esw_init();
#elif defined (CONFIG_RALINK_RT3883)
	rt3883_eth_init();
#elif defined (CONFIG_RALINK_MT7620)
	mt7620_esw_init();
#elif defined (CONFIG_RALINK_MT7621)
	mt7621_eth_init();
#endif
}

