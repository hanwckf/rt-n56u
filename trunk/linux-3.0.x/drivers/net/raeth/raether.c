#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "raether.h"
#include "ra_mac.h"
#include "ra_phy.h"
#include "ra_rfrw.h"
#include "ra_ioctl.h"
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
#include "ra_esw_base.h"
#endif
#if defined (CONFIG_RAETH_DHCP_TOUCH)
#include "ra_esw_dhcpc.h"
#endif
#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
#include "ra_esw_mt7621.h"
#endif
#if defined (CONFIG_RAETH_ESW_CONTROL)
#include "ra_esw_ioctl.h"
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../net/nat/hw_nat/ra_nat.h"
#include "../../../net/nat/hw_nat/foe_fdb.h"
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
static u8  vlan_4k_map[VLAN_N_VID];
static u16 vlan_id_map[16];
#endif

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD) && !defined (RAETH_SDMA)
static int hw_offload_csg = 1;
#if defined (CONFIG_RAETH_SG_DMA_TX)
static int hw_offload_gso = 1;
#if defined (CONFIG_RAETH_TSO)
static int hw_offload_tso = 1;
#endif
#endif
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
struct FoeEntry *PpeFoeBase = NULL;
dma_addr_t PpeFoeBasePhy = 0;
int (*ra_sw_nat_hook_rx)(struct sk_buff *skb) = NULL;
int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no) = NULL;
int (*ra_sw_nat_hook_rs)(struct net_device *dev, int hold) = NULL;
int (*ra_sw_nat_hook_ec)(int engine_init) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_rx);
EXPORT_SYMBOL(ra_sw_nat_hook_tx);
EXPORT_SYMBOL(ra_sw_nat_hook_rs);
EXPORT_SYMBOL(ra_sw_nat_hook_ec);
#endif

#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
extern int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);
#endif

#if defined (CONFIG_VLAN_8021Q_DOUBLE_TAG)
extern int vlan_double_tag;
#endif

#if defined (CONFIG_ETHTOOL)
#include "ra_ethtool.h"
extern struct ethtool_ops ra_ethtool_ops;
#if defined (CONFIG_PSEUDO_SUPPORT)
extern struct ethtool_ops ra_virt_ethtool_ops;
#endif
#endif

extern u32 ralink_asic_rev_id;

struct net_device *dev_raether = NULL;

//////////////////////////////////////////////////////////////

#if defined (CONFIG_RAETH_QDMA)
#include "raether_qdma.c"
#else
#include "raether_pdma.c"
#endif

//////////////////////////////////////////////////////////////

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
struct FoeEntry *
get_foe_table(dma_addr_t *dma_handle, uint32_t *FoeTblSize)
{
	if (dma_handle)
		*dma_handle = PpeFoeBasePhy;

	if (FoeTblSize)
		*FoeTblSize = FOE_4TB_SIZ;

	return PpeFoeBase;
}
EXPORT_SYMBOL(get_foe_table);

static void
foe_dma_table_free(void)
{
	/* free PPE FoE table */
	if (PpeFoeBase) {
		dma_free_coherent(NULL, FOE_4TB_SIZ * sizeof(struct FoeEntry), PpeFoeBase, PpeFoeBasePhy);
		PpeFoeBase = NULL;
		PpeFoeBasePhy = 0;
	}
}

static void
foe_dma_table_alloc(void)
{
	/* PPE FoE Table */
	PpeFoeBase = (struct FoeEntry *)dma_alloc_coherent(NULL, FOE_4TB_SIZ * sizeof(struct FoeEntry), &PpeFoeBasePhy, GFP_KERNEL);
}
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
static void
fill_hw_vlan_tx(void)
{
	u32 i;

	/* init vlan_4k map table by index 15 */
	memset(vlan_4k_map, 0x0F, sizeof(vlan_4k_map));

	for (i = 0; i < 16; i++) {
		vlan_id_map[i] = (u16)i;
		vlan_4k_map[i] = (u8)i;
	}

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* map VLAN TX for external offload (use slots 11..15) */
	i = 11;
#if defined (CONFIG_RA_HW_NAT_WIFI)
	vlan_id_map[i] = (u16)DP_RA0;	// IDX: 11
	vlan_4k_map[DP_RA0] = (u8)i;
	i++;
	vlan_id_map[i] = (u16)DP_RA1;	// IDX: 12
	vlan_4k_map[DP_RA1] = (u8)i;
	i++;
#if defined (HWNAT_DP_RAI_AP)
	vlan_id_map[i] = (u16)DP_RAI0;	// IDX: 13
	vlan_4k_map[DP_RAI0] = (u8)i;
	i++;
	vlan_id_map[i] = (u16)DP_RAI1;	// IDX: 14
	vlan_4k_map[DP_RAI1] = (u8)i;
	i++;
#endif
#endif
#if defined (CONFIG_RA_HW_NAT_PCI)
	vlan_id_map[i] = (u16)DP_NIC0;	// IDX: 15
	vlan_4k_map[DP_NIC0] = (u8)i;
#endif
#endif
}

static void
update_hw_vlan_tx(void)
{
	u32 i, reg_vlan;

	for (i = 0; i < 8; i++) {
		reg_vlan = ((u32)vlan_id_map[(i*2)+1] << 16) | (u32)vlan_id_map[i*2];
#if defined (CONFIG_RALINK_MT7620)
		sysRegWrite(RALINK_FRAME_ENGINE_BASE + 0x430 + i*4, reg_vlan);
#else
		sysRegWrite(RALINK_FRAME_ENGINE_BASE + 0x0a8 + i*4, reg_vlan);
#endif
	}
}

u32
get_map_hw_vlan_tx(u32 idx)
{
	return (u32)vlan_id_map[(idx & 0xF)];
}

void
set_map_hw_vlan_tx(u32 idx, u32 vid)
{
	u32 i, vid_old;

	idx &= 0xF;
	vid &= VLAN_VID_MASK;

	vid_old = (u32)vlan_id_map[idx] & VLAN_VID_MASK;

	vlan_id_map[idx] = (u16)vid;
	vlan_4k_map[vid] = (u8)idx;

	/* remap old VID pointer */
	if (vid != vid_old) {
		for (i = 0; i < 16; i++) {
			if ((u32)vlan_id_map[i] == vid_old) {
				vlan_4k_map[vid_old] = (u8)i;
				break;
			}
		}
		if (i > 15)
			vlan_4k_map[vid_old] = 0xF;
	}

	update_hw_vlan_tx();
}
#endif

static void
fe_reset(void)
{
	u32 val;

	val = sysRegRead(REG_RSTCTRL);

	/* RT5350/MT7628 (SDMA) need to reset ESW and FE at the same to avoid PDMA panic */
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	val |= RALINK_ESW_RST;
#endif

	/* Reset PPE at this point */
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	val |= RALINK_PPE_RST;
#endif

	/* MT7621 + TRGMII need to reset GMAC */
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_GE1_TRGMII_FORCE_1200)
	val |= RALINK_ETH_RST;
#endif

	val |= RALINK_FE_RST;
	sysRegWrite(REG_RSTCTRL, val);
	udelay(10);

	/* set TRGMII clock */
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_GE1_TRGMII_FORCE_1200)
	{
		u32 val_clk = sysRegRead(REG_CLK_CFG_0);
		val_clk &= 0xffffff9f;
		val_clk |= (0x1 << 5);
		sysRegWrite(REG_CLK_CFG_0, val_clk);
		udelay(1000);
	}
#endif

#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628) || defined (CONFIG_RALINK_MT7620)
	val &= ~(RALINK_ESW_RST);
#endif

#if defined (CONFIG_RALINK_MT7620)
	val &= ~(RALINK_EPHY_RST);
#endif

#if defined (CONFIG_RALINK_MT7621)
	val &= ~(RALINK_ETH_RST | RALINK_HSDMA_RST);
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	val &= ~(RALINK_PPE_RST);
#endif

	val &= ~(RALINK_FE_RST);
	sysRegWrite(REG_RSTCTRL, val);
	udelay(1000);
}

static void
fe_mac1_addr_set(unsigned char p[6])
{
	u32 regValue;

	regValue = (p[0] << 8) | (p[1]);
#if defined (RAETH_SDMA)
	sysRegWrite(SDM_MAC_ADRH, regValue);
#elif defined (CONFIG_RALINK_MT7620)
	sysRegWrite(SMACCR1, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRH, regValue);
#endif

	regValue = (p[2] << 24) | (p[3] << 16) | (p[4] << 8) | p[5];
#if defined (RAETH_SDMA)
	sysRegWrite(SDM_MAC_ADRL, regValue);
#elif defined (CONFIG_RALINK_MT7620)
	sysRegWrite(SMACCR0, regValue);
#else
	sysRegWrite(GDMA1_MAC_ADRL, regValue);
#endif
}

#if defined (CONFIG_PSEUDO_SUPPORT)
static void
fe_mac2_addr_set(unsigned char p[6])
{
	u32 regValue;

	regValue = (p[0] << 8) | (p[1]);
	sysRegWrite(GDMA2_MAC_ADRH, regValue);

	regValue = (p[2] << 24) | (p[3] << 16) | (p[4] << 8) | p[5];
	sysRegWrite(GDMA2_MAC_ADRL, regValue);
}
#endif

static void
fe_forward_config(struct net_device *dev, END_DEVICE *ei_local)
{
#if defined (RAETH_SDMA)
	/* RT5350/MT7628 SDMA: No GDMA, PSE, CDMA, PPE */
	u32 sdmVal;

	sdmVal = sysRegRead(SDM_CON);
#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	sdmVal |= (SDM_IPCS | SDM_TCPCS | SDM_UDPCS);

	dev->features |= NETIF_F_RXCSUM; /* Can RX checksum */
	printk("%s: HW IP/TCP/UDP checksum %s offload enabled\n", RAETH_DEV_NAME, "RX");
#else
	sdmVal &= ~(SDM_IPCS | SDM_TCPCS | SDM_UDPCS);
#endif
#if defined (CONFIG_RAETH_SPECIAL_TAG)
	sdmVal |= SDM_TCI_81XX;
#endif
	sysRegWrite(SDM_CON, sdmVal);

#else /* !RAETH_SDMA */
	u32 regVal, regCsg;

#if defined (CONFIG_PSEUDO_SUPPORT)
	/* pad to 60 bytes (no VLAN tag) */
	ei_local->min_pkt_len = ETH_ZLEN;
#else
	/* pad to 64 bytes (with VLAN tag) */
	ei_local->min_pkt_len = VLAN_ETH_ZLEN;
#endif

#if defined (CONFIG_RAETH_HW_VLAN_RX)
#if defined (CONFIG_VLAN_8021Q_DOUBLE_TAG)
	if (vlan_double_tag) {
		/* disable HW VLAN RX */
		sysRegWrite(CDMP_EG_CTRL, 0);
		dev->features &= ~(NETIF_F_HW_VLAN_CTAG_RX);
	} else
#endif
	{
		/* enable HW VLAN RX */
		sysRegWrite(CDMP_EG_CTRL, 1);
		dev->features |= NETIF_F_HW_VLAN_CTAG_RX;
		printk("%s: HW VLAN %s offload enabled\n", RAETH_DEV_NAME, "RX");
	}
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX)
#if !defined (CONFIG_RALINK_MT7621)
	update_hw_vlan_tx();
#endif
#if defined (CONFIG_VLAN_8021Q_DOUBLE_TAG)
	if (vlan_double_tag) {
		/* disable HW VLAN TX */
		dev->features &= ~(NETIF_F_HW_VLAN_CTAG_TX);
	} else
#endif
	{
		/* enable HW VLAN TX */
		ei_local->min_pkt_len = ETH_ZLEN; // pad to 60 bytes
		dev->features |= NETIF_F_HW_VLAN_CTAG_TX;
		printk("%s: HW VLAN %s offload enabled\n", RAETH_DEV_NAME, "TX");
	}
#endif

	regCsg = sysRegRead(CDMA_CSG_CFG);
	regVal = sysRegRead(GDMA1_FWD_CFG);

#if defined (CONFIG_RALINK_MT7620)
	/* GDMA1 frames destination port is port0 CPU */
	regVal &= ~0x7;
#else
	/* set unicast/multicast/broadcast/other frames forward to CPU (PDMA) */
	regVal &= ~0x7777;
	regVal |= (GDM1_UFRC_P_CPU | GDM1_BFRC_P_CPU | GDM1_MFRC_P_CPU | GDM1_OFRC_P_CPU);
#endif

	regCsg &= ~0x7;

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	regVal |= GDM1_ICS_EN;
	regVal |= GDM1_TCS_EN;
	regVal |= GDM1_UCS_EN;

	dev->features |= NETIF_F_RXCSUM; /* Can handle RX checksum */

	if (hw_offload_csg) {
		regCsg |= ICS_GEN_EN;
		regCsg |= TCS_GEN_EN;
		regCsg |= UCS_GEN_EN;
		
		dev->features |= NETIF_F_IP_CSUM; /* Can generate TX checksum TCP/UDP over IPv4 */
		printk("%s: HW IP/TCP/UDP checksum %s offload enabled\n", RAETH_DEV_NAME, "RX/TX");
	} else {
		dev->features &= ~NETIF_F_IP_CSUM;
		printk("%s: HW IP/TCP/UDP checksum %s offload enabled\n", RAETH_DEV_NAME, "RX");
	}

#if defined (CONFIG_RAETH_SG_DMA_TX)
	if (hw_offload_gso && hw_offload_csg) {
		dev->features |= NETIF_F_SG;
		printk("%s: HW Scatter/Gather TX offload enabled\n", RAETH_DEV_NAME);
#if defined (CONFIG_RAETH_TSO)
		if (hw_offload_tso) {
			dev->features |= NETIF_F_TSO;
#if defined (CONFIG_RAETH_TSOV6)
			dev->features |= NETIF_F_TSO6;
			dev->features |= NETIF_F_IPV6_CSUM; /* Can TX checksum TCP/UDP over IPv6 */
#endif
			printk("%s: HW TCP segmentation offload (TSO) enabled\n", RAETH_DEV_NAME);
		} else {
			dev->features &= ~(NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_IPV6_CSUM);
		}
#endif /* CONFIG_RAETH_TSO */
	} else {
		dev->features &= ~(NETIF_F_SG | NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_IPV6_CSUM);
	}
#endif /* CONFIG_RAETH_SG_DMA_TX */

#else /* !CONFIG_RAETH_CHECKSUM_OFFLOAD */

	regCsg &= ~ICS_GEN_EN;
	regCsg &= ~TCS_GEN_EN;
	regCsg &= ~UCS_GEN_EN;

	regVal &= ~GDM1_ICS_EN;
	regVal &= ~GDM1_TCS_EN;
	regVal &= ~GDM1_UCS_EN;

	dev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_RXCSUM); /* disable checksum TCP/UDP over IPv4 */
#endif

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	regVal |= GDM1_TCI_81XX;
#endif

#if defined (CONFIG_RAETH_JUMBOFRAME)
	regVal |= GDM1_JMB_EN;
	regVal &= ~0xf0000000; /* clear bit28-bit31 */
	regVal |= (((MAX_RX_LENGTH/1024)&0xf) << 28);
#endif

	sysRegWrite(CDMA_CSG_CFG, regCsg);
	sysRegWrite(GDMA1_FWD_CFG, regVal);
#if defined (CONFIG_PSEUDO_SUPPORT)
	sysRegWrite(GDMA2_FWD_CFG, regVal);
#endif

/*
 * 	PSE_FQ_CFG register definition -
 *
 * 	Define max free queue page count in PSE. (31:24)
 *	RT3883 - 0xff908000 (255 pages)
 *	RT3052 - 0x80504000 (128 pages)
 *
 * 	In each page, there are 128 bytes in each page.
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

	regCsg = sysRegRead(CDMA_CSG_CFG);
	regVal = sysRegRead(GDMA1_FWD_CFG);
#if defined (CONFIG_PSEUDO_SUPPORT)
	regVal = sysRegRead(GDMA2_FWD_CFG);
#endif

#endif /* RAETH_SDMA */
}

static void
fe_dma_start(void)
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

	sysRegWrite(PDMA_GLO_CFG, dma_glo_cfg);
#if defined (CONFIG_RAETH_QDMA)
	sysRegWrite(QDMA_GLO_CFG, dma_glo_cfg);
#endif
}

static void
fe_dma_stop(void)
{
	u32 regValue;

	regValue = sysRegRead(PDMA_GLO_CFG);
	regValue &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regValue);

#if defined (CONFIG_RAETH_QDMA)
	regValue = sysRegRead(QDMA_GLO_CFG);
	regValue &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(QDMA_GLO_CFG, regValue);
#endif
}

static void
read_counters_gdma1(END_DEVICE *ei_local)
{
	u32 rx_fcs_bad;
#if !defined (RAETH_SDMA)
	u32 rx_too_sho;
	u32 rx_too_lon;
	u32 tx_skipped;

	ei_local->stat.tx_bytes         += sysRegRead(GDMA1_TX_GBCNT);
	ei_local->stat.tx_packets       += sysRegRead(GDMA1_TX_GPCNT);
	tx_skipped                       = sysRegRead(GDMA1_TX_SKIPCNT);
	ei_local->stat.collisions       += sysRegRead(GDMA1_TX_COLCNT);

	ei_local->stat.rx_bytes         += sysRegRead(GDMA1_RX_GBCNT);
	ei_local->stat.rx_packets       += sysRegRead(GDMA1_RX_GPCNT);
	ei_local->stat.rx_over_errors   += sysRegRead(GDMA1_RX_OERCNT);
	rx_fcs_bad                       = sysRegRead(GDMA1_RX_FERCNT);
	rx_too_sho                       = sysRegRead(GDMA1_RX_SERCNT);
	rx_too_lon                       = sysRegRead(GDMA1_RX_LERCNT);

	if (tx_skipped)
		ei_local->stat.tx_dropped += tx_skipped;

	if (rx_too_sho)
		ei_local->stat.rx_length_errors += rx_too_sho;

	if (rx_too_lon)
		ei_local->stat.rx_length_errors += rx_too_lon;
#else
	ei_local->stat.tx_bytes         += sysRegRead(SDM_TBCNT);
	ei_local->stat.tx_packets       += sysRegRead(SDM_TPCNT);

	ei_local->stat.rx_bytes         += sysRegRead(SDM_RBCNT);
	ei_local->stat.rx_packets       += sysRegRead(SDM_RPCNT);
	rx_fcs_bad                       = sysRegRead(SDM_CS_ERR);
#endif

	if (rx_fcs_bad) {
		ei_local->stat.rx_errors += rx_fcs_bad;
		ei_local->stat.rx_crc_errors += rx_fcs_bad;
	}
}

#if defined (CONFIG_PSEUDO_SUPPORT)
static void
read_counters_gdma2(PSEUDO_ADAPTER *pPseudoAd)
{
	u32 tx_skipped;
	u32 rx_fcs_bad;
	u32 rx_too_sho;
	u32 rx_too_lon;

	pPseudoAd->stat.tx_bytes        += sysRegRead(GDMA2_TX_GBCNT);
	pPseudoAd->stat.tx_packets      += sysRegRead(GDMA2_TX_GPCNT);
	tx_skipped                       = sysRegRead(GDMA2_TX_SKIPCNT);
	pPseudoAd->stat.collisions      += sysRegRead(GDMA2_TX_COLCNT);

	pPseudoAd->stat.rx_bytes        += sysRegRead(GDMA2_RX_GBCNT);
	pPseudoAd->stat.rx_packets      += sysRegRead(GDMA2_RX_GPCNT);
	pPseudoAd->stat.rx_over_errors  += sysRegRead(GDMA2_RX_OERCNT);
	rx_fcs_bad                       = sysRegRead(GDMA2_RX_FERCNT);
	rx_too_sho                       = sysRegRead(GDMA2_RX_SERCNT);
	rx_too_lon                       = sysRegRead(GDMA2_RX_LERCNT);

	if (tx_skipped)
		pPseudoAd->stat.tx_dropped += tx_skipped;

	if (rx_too_sho)
		pPseudoAd->stat.rx_length_errors += rx_too_sho;

	if (rx_too_lon)
		pPseudoAd->stat.rx_length_errors += rx_too_lon;

	if (rx_fcs_bad) {
		pPseudoAd->stat.rx_errors += rx_fcs_bad;
		pPseudoAd->stat.rx_crc_errors += rx_fcs_bad;
	}
}
#endif

static void
wait_dma_stop(int cycles_10ms)
{
	int i;
	u32 regVal;

	for (i = 0; i < cycles_10ms; i++) {
		regVal  = sysRegRead(PDMA_GLO_CFG);
#if defined (CONFIG_RAETH_QDMA)
		regVal |= sysRegRead(QDMA_GLO_CFG);
#endif
		if (!(regVal & (RX_DMA_BUSY | TX_DMA_BUSY)))
			break;
		msleep(10);
	}
}

static void
inc_rx_drop(END_DEVICE *ei_local, int gmac_no)
{
#if defined (CONFIG_PSEUDO_SUPPORT)
	PSEUDO_ADAPTER *pAd;

	if (gmac_no == PSE_PORT_GMAC2) {
		pAd = netdev_priv(ei_local->PseudoDev);
		pAd->stat.rx_dropped++;
	} else
#endif
	if (gmac_no == PSE_PORT_GMAC1)
		ei_local->stat.rx_dropped++;
}

static inline int
pdma_recv(struct net_device* dev, END_DEVICE* ei_local, int work_todo)
{
	struct PDMA_rxdesc *rxd_ring;
	struct sk_buff *new_skb, *rx_skb;
	int gmac_no = PSE_PORT_GMAC1;
	int work_done = 0;
	u32 rxd_dma_owner_idx;
	u32 rxd_info2, rxd_info4;
#if defined (CONFIG_RAETH_HW_VLAN_RX)
	u32 rxd_info3;
#endif
#if defined (CONFIG_RAETH_SPECIAL_TAG)
	struct vlan_ethhdr *veth;
#endif

	rxd_dma_owner_idx = le32_to_cpu(sysRegRead(RX_CALC_IDX0));

	while (work_done < work_todo) {
		rxd_dma_owner_idx = (rxd_dma_owner_idx + 1) % NUM_RX_DESC;
		rxd_ring = &ei_local->rxd_ring[rxd_dma_owner_idx];
		
		if (!(rxd_ring->rxd_info2 & RX2_DMA_DONE))
			break;
		
		/* load completed skb pointer */
		rx_skb = ei_local->rxd_buff[rxd_dma_owner_idx];
		
		/* copy RX desc to CPU */
		rxd_info2 = rxd_ring->rxd_info2;
#if defined (CONFIG_RAETH_HW_VLAN_RX)
		rxd_info3 = rxd_ring->rxd_info3;
#endif
		rxd_info4 = rxd_ring->rxd_info4;
		
#if defined (CONFIG_PSEUDO_SUPPORT)
		gmac_no = RX4_DMA_SP(rxd_info4);
#endif
		/* We have to check the free memory size is big enough
		 * before pass the packet to cpu */
		new_skb = __dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN, GFP_ATOMIC);
		if (unlikely(new_skb == NULL)) {
#if defined (RAETH_PDMA_V2)
			rxd_ring->rxd_info2 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
#else
			rxd_ring->rxd_info2 = RX2_DMA_LS0;
#endif
			/* move CPU pointer to next RXD */
			sysRegWrite(RX_CALC_IDX0, cpu_to_le32(rxd_dma_owner_idx));
			
			inc_rx_drop(ei_local, gmac_no);
#if !defined (CONFIG_RAETH_NAPI)
			/* mean need reschedule */
			work_done = work_todo;
#endif
#if defined (CONFIG_RAETH_DEBUG)
			if (net_ratelimit())
				printk(KERN_ERR "%s: Failed to alloc new RX skb! (GMAC: %d)\n", RAETH_DEV_NAME, gmac_no);
#endif
			break;
		}
#if !defined (RAETH_PDMA_V2)
		skb_reserve(new_skb, NET_IP_ALIGN);
#endif
		/* store new empty skb pointer */
		ei_local->rxd_buff[rxd_dma_owner_idx] = new_skb;
		
		/* map new skb to ring (unmap is not required on generic mips mm) */
		rxd_ring->rxd_info1 = (u32)dma_map_single(NULL, new_skb->data, MAX_RX_LENGTH, DMA_FROM_DEVICE);
#if defined (RAETH_PDMA_V2)
		rxd_ring->rxd_info2 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
#else
		rxd_ring->rxd_info2 = RX2_DMA_LS0;
#endif
		wmb();
		
		/* move CPU pointer to next RXD */
		sysRegWrite(RX_CALC_IDX0, cpu_to_le32(rxd_dma_owner_idx));
		
		/* skb processing */
		rx_skb->len = RX2_DMA_SDL0_GET(rxd_info2);
#if defined (RAETH_PDMA_V2)
		rx_skb->data += NET_IP_ALIGN;
#endif
		rx_skb->tail = rx_skb->data + rx_skb->len;

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
		FOE_MAGIC_TAG(rx_skb) = FOE_MAGIC_GE;
		DO_FILL_FOE_DESC(rx_skb, (rxd_info4 & ~(RX4_DMA_ALG_SET)));
#endif

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
		if (rxd_info4 & RX4_DMA_L4FVLD)
			rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
#endif

#if defined (CONFIG_RAETH_HW_VLAN_RX)
		if ((rxd_info2 & RX2_DMA_TAG) && rxd_info3) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
			__vlan_hwaccel_put_tag(rx_skb, __constant_htons(ETH_P_8021Q), RX3_DMA_VID(rxd_info3));
#else
			__vlan_hwaccel_put_tag(rx_skb, RX3_DMA_VID(rxd_info3));
#endif
		}
#endif

#if defined (CONFIG_PSEUDO_SUPPORT)
		if (gmac_no == PSE_PORT_GMAC2)
			rx_skb->protocol = eth_type_trans(rx_skb, ei_local->PseudoDev);
		else
#endif
			rx_skb->protocol = eth_type_trans(rx_skb, dev);

#if defined (CONFIG_RAETH_SPECIAL_TAG)
#if defined (CONFIG_MT7530_GSW)
#define ESW_TAG_ID	0x00
#else
#define ESW_TAG_ID	0x81
#endif
		// port0: 0x8100 => 0x8100 0001
		// port1: 0x8101 => 0x8100 0002
		// port2: 0x8102 => 0x8100 0003
		// port3: 0x8103 => 0x8100 0004
		// port4: 0x8104 => 0x8100 0005
		// port5: 0x8105 => 0x8100 0006
		veth = vlan_eth_hdr(rx_skb);
		if ((veth->h_vlan_proto & 0xFF) == ESW_TAG_ID) {
			veth->h_vlan_TCI = htons((((veth->h_vlan_proto >> 8) & 0xF) + 1));
			veth->h_vlan_proto = __constant_htons(ETH_P_8021Q);
			rx_skb->protocol = veth->h_vlan_proto;
		}
#endif

/* ra_sw_nat_hook_rx return 1 --> continue
 * ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
 */
#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
		if((ra_sw_nat_hook_rx == NULL) ||
		   (ra_sw_nat_hook_rx != NULL && ra_sw_nat_hook_rx(rx_skb)))
#endif
		{
#if defined (CONFIG_RAETH_NAPI)
#if defined (CONFIG_RAETH_NAPI_GRO)
			if (rx_skb->ip_summed == CHECKSUM_UNNECESSARY)
				napi_gro_receive(&ei_local->napi, rx_skb);
			else
#endif
			netif_receive_skb(rx_skb);
#else
			netif_rx(rx_skb);
#endif
		}
		
		work_done++;
	}

	return work_done;
}

static inline void
dispatch_int_status2(END_DEVICE *ei_local)
{
#if defined (CONFIG_GE2_INTERNAL_GPHY_P0) || defined (CONFIG_GE2_INTERNAL_GPHY_P4)
	u32 reg_int_val;

	reg_int_val = sysRegRead(FE_INT_STATUS2);
	if (unlikely(!reg_int_val))
		return;

	sysRegWrite(FE_INT_STATUS2, reg_int_val);

	if (!ei_local->active)
		return;

	if (reg_int_val & GE2_LINK_INT) {
#if defined (CONFIG_GE2_INTERNAL_GPHY_P0)
		u32 port_id = 0;
#else
		u32 port_id = 4;
#endif
		u32 link_state = sysRegRead(RALINK_ETH_SW_BASE+0x0208);
		if (link_state & 0x1) {
			/* MT7621 E2 has FC bug */
			if ((ralink_asic_rev_id & 0xFFFF) == 0x0101) {
				u32 link_speed = (link_state >> 2) & 0x3;
				mt7621_esw_fc_delay_set((link_speed == 1) ? 1 : 0);
			}
		}
		
		esw_link_status_changed(port_id, link_state & 0x1);
	}
#endif
}

#if defined (CONFIG_RAETH_NAPI)
static int
ei_napi_poll(struct napi_struct *napi, int budget)
{
	END_DEVICE *ei_local = netdev_priv(napi->dev);
	int work_done;

	/* cleanup TX queue */
	dma_xmit_clean(napi->dev, ei_local);

	/* process RX queue */
	work_done = pdma_recv(napi->dev, ei_local, budget);

	if (work_done < budget) {
		unsigned long flags;
		
		dispatch_int_status2(ei_local);
		
#if defined (CONFIG_RAETH_NAPI_GRO)
		napi_gro_flush(napi);
#endif
		/* exit from NAPI poll mode, ack and enable TX/RX interrupts */
		local_irq_save(flags);
		__napi_complete(napi);
		sysRegWrite(FE_INT_STATUS, FE_INT_INIT_VALUE);
#if defined (CONFIG_RAETH_QDMA)
		sysRegWrite(QFE_INT_STATUS, QFE_INT_INIT_VALUE);
#endif
		if (ei_local->active) {
#if (defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)) && FE_INT_INIT2_VALUE
			sysRegWrite(FE_INT_ENABLE2, FE_INT_INIT2_VALUE);
#endif
#if defined (CONFIG_RAETH_QDMA)
			sysRegWrite(QFE_INT_ENABLE, QFE_INT_INIT_VALUE);
#endif
			sysRegWrite(FE_INT_ENABLE, FE_INT_INIT_VALUE);
		}
		local_irq_restore(flags);
	}

	return work_done;
}

#else

static void
ei_receive(unsigned long ptr)
{
	struct net_device *dev = (struct net_device *)ptr;
	END_DEVICE *ei_local = netdev_priv(dev);
	int work_done;

	/* process RX queue */
	work_done = pdma_recv(dev, ei_local, NUM_RX_MAX_PROCESS);

	if (work_done < NUM_RX_MAX_PROCESS) {
		unsigned long flags;
		
		/* ack and enable RX interrupts */
		local_irq_save(flags);
		sysRegWrite(FE_INT_STATUS, FE_INT_MASK_RX);
		if (ei_local->active)
			sysRegWrite(FE_INT_ENABLE, FE_INT_INIT_VALUE);
		local_irq_restore(flags);
	} else {
		/* reschedule again */
		if (ei_local->active)
			tasklet_schedule(&ei_local->rx_tasklet);
	}
}

static void
ei_xmit_clean(unsigned long ptr)
{
	struct net_device *dev = (struct net_device *)ptr;
	END_DEVICE *ei_local = netdev_priv(dev);

	dma_xmit_clean(dev, ei_local);
}
#endif

/**
 * ei_interrupt - handle controler interrupt
 *
 * This routine is called at interrupt level in response to an interrupt from
 * the controller.
 */
static irqreturn_t
ei_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	END_DEVICE *ei_local;
	u32 __maybe_unused reg_int_val;

	if (!dev)
		return IRQ_NONE;

	ei_local = netdev_priv(dev);

#if defined (CONFIG_RAETH_NAPI)
	if (napi_schedule_prep(&ei_local->napi)) {
		/* disable all interrupts */
		sysRegWrite(FE_INT_ENABLE, 0);
#if (defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)) && FE_INT_INIT2_VALUE
		sysRegWrite(FE_INT_ENABLE2, 0);
#endif
#if defined (CONFIG_RAETH_QDMA)
		sysRegWrite(QFE_INT_ENABLE, 0);
#endif
		/* enter to NAPI poll mode */
		__napi_schedule(&ei_local->napi);
	}
#else
	reg_int_val  = sysRegRead(FE_INT_STATUS);
#if defined (CONFIG_RAETH_QDMA)
	reg_int_val |= sysRegRead(QFE_INT_STATUS);
	if (reg_int_val)
		sysRegWrite(QFE_INT_STATUS, reg_int_val);

	if (reg_int_val & QFE_INT_MASK_TX)
		tasklet_schedule(&ei_local->tx_tasklet);
#else
	if (reg_int_val & FE_INT_MASK_TX)
		tasklet_schedule(&ei_local->tx_tasklet);
#endif

	if (reg_int_val & FE_INT_MASK_RX) {
		u32 reg_int_mask = sysRegRead(FE_INT_ENABLE);
		if (reg_int_mask & FE_INT_MASK_RX) {
			/* disable RX interrupt */
			sysRegWrite(FE_INT_ENABLE, reg_int_mask & ~(FE_INT_MASK_RX));
			tasklet_hi_schedule(&ei_local->rx_tasklet);
		}
	}

	if (reg_int_val)
		sysRegWrite(FE_INT_STATUS, reg_int_val);

	dispatch_int_status2(ei_local);
#endif /* CONFIG_RAETH_NAPI */

	return IRQ_HANDLED;
}

static int
ei_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 || new_mtu > MAX_RX_LENGTH)
		return -EINVAL;

#if !defined (CONFIG_RAETH_JUMBOFRAME)
	if (new_mtu > ETH_DATA_LEN)
		return -EINVAL;
#endif

	dev->mtu = new_mtu;

	return 0;
}

static void
fill_dev_features(struct net_device *dev)
{
#if defined (RAETH_SDMA)

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	dev->hw_features |= NETIF_F_RXCSUM; /* Can handle RX checksum */
#endif

#else /* !RAETH_SDMA */

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	dev->hw_features |= NETIF_F_RXCSUM; /* Can handle RX checksum */
	if (hw_offload_csg)
		dev->hw_features |= NETIF_F_IP_CSUM; /* Can generate TX checksum TCP/UDP over IPv4 */
#if defined (CONFIG_RAETH_SG_DMA_TX)
	if (hw_offload_gso && hw_offload_csg) {
		dev->hw_features |= NETIF_F_SG;
#if defined (CONFIG_RAETH_TSO)
		if (hw_offload_tso) {
			dev->hw_features |= NETIF_F_TSO;
#if defined (CONFIG_RAETH_TSOV6)
			dev->hw_features |= NETIF_F_TSO6;
			dev->hw_features |= NETIF_F_IPV6_CSUM; /* Can generate TX checksum TCP/UDP over IPv6 */
#endif
		}
#endif /* CONFIG_RAETH_TSO */
	}
#endif /* CONFIG_RAETH_SG_DMA_TX */
#endif /* CONFIG_RAETH_CHECKSUM_OFFLOAD */

#if defined (CONFIG_RAETH_HW_VLAN_RX)
	dev->hw_features |= NETIF_F_HW_VLAN_CTAG_RX;
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX)
	dev->hw_features |= NETIF_F_HW_VLAN_CTAG_TX;
#endif

#endif /* RAETH_SDMA */

	dev->features = dev->hw_features;
	dev->vlan_features = dev->features & ~(NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX);
}

#if defined (CONFIG_PSEUDO_SUPPORT)
static struct rtnl_link_stats64 *
VirtualIF_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);
	END_DEVICE *ei_local = netdev_priv(pPseudoAd->RaethDev);

	spin_lock(&ei_local->stat_lock);
	read_counters_gdma2(pPseudoAd);
	memcpy(stats, &pPseudoAd->stat, sizeof(struct rtnl_link_stats64));
	spin_unlock(&ei_local->stat_lock);

	return stats;
}

static int
VirtualIF_open(struct net_device *dev)
{
#if defined (CONFIG_RAETH_HW_VLAN_RX)
#if defined (CONFIG_VLAN_8021Q_DOUBLE_TAG)
	if (vlan_double_tag)
		dev->features &= ~(NETIF_F_HW_VLAN_CTAG_TX);
	else
#endif
		dev->features |= NETIF_F_HW_VLAN_CTAG_RX;
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX)
#if defined (CONFIG_VLAN_8021Q_DOUBLE_TAG)
	if (vlan_double_tag)
		dev->features &= ~(NETIF_F_HW_VLAN_CTAG_TX);
	else
#endif
		dev->features |= NETIF_F_HW_VLAN_CTAG_TX;
#endif

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	dev->features |= NETIF_F_RXCSUM;

	if (hw_offload_csg)
		dev->features |= NETIF_F_IP_CSUM;
	else
		dev->features &= ~NETIF_F_IP_CSUM;

#if defined (CONFIG_RAETH_SG_DMA_TX)
	if (hw_offload_gso && hw_offload_csg) {
		dev->features |= NETIF_F_SG;
#if defined (CONFIG_RAETH_TSO)
		if (hw_offload_tso) {
			dev->features |= NETIF_F_TSO;
#if defined (CONFIG_RAETH_TSOV6)
			dev->features |= NETIF_F_TSO6;
			dev->features |= NETIF_F_IPV6_CSUM; /* Can checksum TCP/UDP over IPv6 */
#endif
		} else {
			dev->features &= ~(NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_IPV6_CSUM);
		}
#endif /* CONFIG_RAETH_TSO */
	} else {
		dev->features &= ~(NETIF_F_SG | NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_IPV6_CSUM);
	}
#endif /* CONFIG_RAETH_SG_DMA_TX */

#else /* !CONFIG_RAETH_CHECKSUM_OFFLOAD */

	dev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_RXCSUM); /* disable checksum TCP/UDP over IPv4 */

#endif /* CONFIG_RAETH_CHECKSUM_OFFLOAD */

#if defined (CONFIG_RAETH_BQL)
	netdev_reset_queue(dev);
#endif
	netif_start_queue(dev);

	printk("%s: ===> VirtualIF_open\n", dev->name);
	return 0;
}

static int
VirtualIF_close(struct net_device *dev)
{
	netif_tx_disable(dev);

	printk("%s: ===> VirtualIF_close\n", dev->name);
	return 0;
}

static int
VirtualIF_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	END_DEVICE *ei_local;
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);

	if (!netif_running(pPseudoAd->RaethDev)) {
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	ei_local = netdev_priv(pPseudoAd->RaethDev);

	return dma_xmit(skb, dev, ei_local, PSE_PORT_GMAC2);
}

static const struct net_device_ops VirtualIF_netdev_ops = {
	.ndo_open		= VirtualIF_open,
	.ndo_stop		= VirtualIF_close,
	.ndo_start_xmit		= VirtualIF_start_xmit,
	.ndo_get_stats64	= VirtualIF_get_stats64,
	.ndo_do_ioctl		= VirtualIF_ioctl,
	.ndo_change_mtu		= ei_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

static int
VirtualIF_init(struct net_device *dev_parent)
{
	struct net_device *dev;
	PSEUDO_ADAPTER *pPseudoAd;
	END_DEVICE *ei_local;
#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
	int i = 0;
	struct sockaddr addr;
	unsigned char zero1[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char zero2[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
#endif

	dev = alloc_etherdev(sizeof(PSEUDO_ADAPTER));
	if (!dev)
		return -ENOMEM;

	pPseudoAd = netdev_priv(dev);
	pPseudoAd->RaethDev = dev_parent;

	ether_setup(dev);
	strcpy(dev->name, DEV2_NAME);

	/* Get mac2 address from flash */
#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
	i = ra_mtd_read_nm("Factory", GMAC2_OFFSET, 6, addr.sa_data);

	/* If reading mtd failed or mac0 is empty, generate a mac address */
	if (i < 0 || ((memcmp(addr.sa_data, zero1, 6) == 0) || (addr.sa_data[0] & 0x1)) ||
	    (memcmp(addr.sa_data, zero2, 6) == 0)) {
		unsigned char mac_addr01234[5] = {0x00, 0x0C, 0x43, 0x28, 0x80};
		net_srandom(jiffies);
		memcpy(addr.sa_data, mac_addr01234, 5);
		addr.sa_data[5] = net_random()&0xFF;
	}

	memcpy(dev->dev_addr, addr.sa_data, dev->addr_len);
#endif

	dev->netdev_ops = &VirtualIF_netdev_ops;
#if defined (CONFIG_ETHTOOL)
	/* init mii structure */
	pPseudoAd->mii_info.dev = dev;
	pPseudoAd->mii_info.mdio_read = mdio_virt_read;
	pPseudoAd->mii_info.mdio_write = mdio_virt_write;
	pPseudoAd->mii_info.phy_id_mask = 0x1f;
	pPseudoAd->mii_info.reg_num_mask = 0x1f;
	pPseudoAd->mii_info.phy_id = 0x1e;
	pPseudoAd->mii_info.supports_gmii = mii_check_gmii_support(&pPseudoAd->mii_info);
	dev->ethtool_ops = &ra_virt_ethtool_ops;
#endif

	fill_dev_features(dev);

	/* Register this device */
	if (register_netdevice(dev) != 0) {
		free_netdev(dev);
		return -ENXIO;
	}

	ei_local = netdev_priv(dev_parent);
	ei_local->PseudoDev = dev;

	return 0;
}
#endif

static int
ei_start_xmit(struct sk_buff* skb, struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);

	return dma_xmit(skb, dev, ei_local, PSE_PORT_GMAC1);
}

static struct rtnl_link_stats64 *
ei_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	END_DEVICE *ei_local = netdev_priv(dev);

	spin_lock(&ei_local->stat_lock);
	read_counters_gdma1(ei_local);
	memcpy(stats, &ei_local->stat, sizeof(struct rtnl_link_stats64));
	spin_unlock(&ei_local->stat_lock);

	return stats;
}

static void
stat_counters_update(struct work_struct *work)
{
	END_DEVICE *ei_local = container_of(work, END_DEVICE, stat_work);

	if (!ei_local->active)
		return;

	spin_lock(&ei_local->stat_lock);
	read_counters_gdma1(ei_local);
#if defined (CONFIG_PSEUDO_SUPPORT)
	read_counters_gdma2(netdev_priv(ei_local->PseudoDev));
#endif
	spin_unlock(&ei_local->stat_lock);

	if (ei_local->active)
		mod_timer(&ei_local->stat_timer, jiffies + (20 * HZ));
}

static void
stat_timer_handler(unsigned long ptr)
{
	struct net_device *dev = (struct net_device *)ptr;
	END_DEVICE *ei_local = netdev_priv(dev);

	if (ei_local->active)
		schedule_work(&ei_local->stat_work);
}

/**
 * ei_init - pick up ethernet port at boot time
 * @dev: network device to probe
 *
 * This routine probe the ethernet port at boot time.
 */
static int
ei_init(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
	int i;
	struct sockaddr addr;
	unsigned char zero1[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char zero2[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
#endif

	ei_local->active = 0;
	ei_local->min_pkt_len = ETH_ZLEN;

	spin_lock_init(&ei_local->stat_lock);
	spin_lock_init(&ei_local->page_lock);

	INIT_WORK(&ei_local->stat_work, stat_counters_update);

	setup_timer(&ei_local->stat_timer, stat_timer_handler, (unsigned long)dev);

#if defined (CONFIG_RAETH_NAPI)
	netif_napi_add(dev, &ei_local->napi, ei_napi_poll, NAPI_WEIGHT);
#else
	tasklet_init(&ei_local->rx_tasklet, ei_receive, (unsigned long)dev);
	tasklet_init(&ei_local->tx_tasklet, ei_xmit_clean, (unsigned long)dev);
#endif

	/* Get MAC0 address from flash */
#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
	i = ra_mtd_read_nm("Factory", GMAC0_OFFSET, 6, addr.sa_data);

	/* If reading mtd failed or mac0 is empty, generate a mac address */
	if (i < 0 || ((memcmp(addr.sa_data, zero1, 6) == 0) || (addr.sa_data[0] & 0x1)) ||
	    (memcmp(addr.sa_data, zero2, 6) == 0)) {
		unsigned char mac_addr01234[5] = {0x00, 0x0C, 0x43, 0x28, 0x80};
		net_srandom(jiffies);
		memcpy(addr.sa_data, mac_addr01234, 5);
		addr.sa_data[5] = net_random()&0xFF;
	}

	memcpy(dev->dev_addr, addr.sa_data, dev->addr_len);
#endif

	if (fe_dma_ring_alloc(ei_local) != 0) {
		printk(KERN_WARNING "%s: ring_alloc FAILED!\n", RAETH_DEV_NAME);
		return -ENOMEM;
	}

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	foe_dma_table_alloc();
#endif

#if defined (CONFIG_PSEUDO_SUPPORT)
	/* Register virtual net device (eth3) for the driver */
	return VirtualIF_init(dev);
#else
	return 0;
#endif
}

static void
ei_uninit(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);

#if defined (CONFIG_PSEUDO_SUPPORT)
	if (ei_local->PseudoDev) {
		unregister_netdevice(ei_local->PseudoDev);
		free_netdev(ei_local->PseudoDev);
		ei_local->PseudoDev = NULL;
	}
#endif

#if defined (CONFIG_RAETH_NAPI)
	netif_napi_del(&ei_local->napi);
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* down PPE engine before free ring */
	if (ra_sw_nat_hook_ec != NULL)
		ra_sw_nat_hook_ec(0);

	foe_dma_table_free();
#endif

	fe_dma_ring_free(ei_local);
}

/**
 * ei_open - Open/Initialize the ethernet port.
 * @dev: network device to initialize
 *
 * This routine goes all-out, setting everything
 * up a new at each open, even though many of these registers should only need to be set once at boot.
 */
static int
ei_open(struct net_device *dev)
{
	int err;
	END_DEVICE *ei_local;

	if (!dev) {
		printk(KERN_EMERG "%s: ei_open passed a non-existent device!\n", dev->name);
		return -ENXIO;
	}

	ei_local = netdev_priv(dev);

	spin_lock_bh(&ei_local->page_lock);

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* down PPE engine before FE reset */
	if (ra_sw_nat_hook_ec != NULL)
		ra_sw_nat_hook_ec(0);
#endif

	fe_reset();
	fe_dma_init(ei_local);
	fe_phy_init();
	fe_forward_config(dev, ei_local);
	fe_mac1_addr_set(dev->dev_addr);
#if defined (CONFIG_PSEUDO_SUPPORT)
	fe_mac2_addr_set(ei_local->PseudoDev->dev_addr);
#endif

#if defined (CONFIG_RAETH_ESW_CONTROL)
	esw_control_post_init();
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* up PPE engine after FE init */
	if (ra_sw_nat_hook_ec != NULL)
		ra_sw_nat_hook_ec(1);
#endif

	spin_unlock_bh(&ei_local->page_lock);

	/* request interrupt line for FE */
	err = request_irq(dev->irq, ei_interrupt, IRQF_DISABLED, dev->name, dev);
	if (err)
		return err;

#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
	/* prepare switch for INT handling */
	esw_irq_init();
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_RALINK_MT7621)
	/* request interrupt line for MT7620 ESW or MT7621 GSW */
	err = request_irq(SURFBOARDINT_ESW, esw_interrupt, IRQF_DISABLED, "ralink_esw", dev);
	
	/* enable ESW interrupts */
	sysRegWrite(RALINK_INTENA, RALINK_INTCTL_ESW);
#elif defined (CONFIG_MT7530_INT_GPIO)
	// todo, needed capture GPIO interrupt for external MT7530
#endif
#endif

	/* clear pending INT and enable INT */
	sysRegWrite(FE_INT_STATUS, 0xffffffff);
	sysRegWrite(FE_INT_ENABLE, FE_INT_INIT_VALUE);
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	sysRegWrite(FE_INT_STATUS2, 0xffffffff);
	sysRegWrite(FE_INT_ENABLE2, FE_INT_INIT2_VALUE);
#endif
#if defined (CONFIG_RAETH_QDMA)
	sysRegWrite(QFE_INT_STATUS, 0xffffffff);
	sysRegWrite(QFE_INT_ENABLE, QFE_INT_INIT_VALUE);
#endif

#if defined (CONFIG_RAETH_NAPI)
	napi_enable(&ei_local->napi);
#endif

	/* allow processing */
	ei_local->active = 1;

	fe_dma_start();

#if defined (CONFIG_RAETH_BQL)
	netdev_reset_queue(dev);
#endif
	netif_start_queue(dev);

	/* counters overflow after ~34s for 1Gbps speed, use 20s period for safe */
	mod_timer(&ei_local->stat_timer, jiffies + (20 * HZ));

	return 0;
}

/**
 * ei_close - shut down network device
 * @dev: network device to clear
 *
 * This routine shut down network device.
 */
static int
ei_close(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);

	/* block processing */
	ei_local->active = 0;

#if defined (CONFIG_PSEUDO_SUPPORT)
	netif_tx_disable(ei_local->PseudoDev);
#endif
	netif_tx_disable(dev);

	fe_dma_stop();
	msleep(10);
	wait_dma_stop(50);

	sysRegWrite(FE_INT_ENABLE, 0);
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	sysRegWrite(FE_INT_ENABLE2, 0);
#endif
#if defined (CONFIG_RAETH_QDMA)
	sysRegWrite(QFE_INT_ENABLE, 0);
#endif

#if defined (CONFIG_RAETH_NAPI)
	napi_disable(&ei_local->napi);
#else
	tasklet_kill(&ei_local->rx_tasklet);
	tasklet_kill(&ei_local->tx_tasklet);
#endif

	del_timer_sync(&ei_local->stat_timer);
	cancel_work_sync(&ei_local->stat_work);
#if defined (CONFIG_RAETH_DHCP_TOUCH)
	esw_dhcpc_cancel();
#endif

	free_irq(dev->irq, dev);
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_RALINK_MT7621)
	free_irq(SURFBOARDINT_ESW, dev);
#elif defined (CONFIG_MT7530_INT_GPIO)
	// todo, needed capture GPIO interrupt for external MT7530
#endif

	spin_lock_bh(&ei_local->page_lock);
	fe_dma_uninit(ei_local);
	spin_unlock_bh(&ei_local->page_lock);

	/* fetch pending FE counters */
	spin_lock(&ei_local->stat_lock);
	read_counters_gdma1(ei_local);
#if defined (CONFIG_PSEUDO_SUPPORT)
	read_counters_gdma2(netdev_priv(ei_local->PseudoDev));
#endif
	spin_unlock(&ei_local->stat_lock);

	return 0;
}

static const struct net_device_ops ei_netdev_ops = {
	.ndo_init		= ei_init,
	.ndo_uninit		= ei_uninit,
	.ndo_open		= ei_open,
	.ndo_stop		= ei_close,
	.ndo_start_xmit		= ei_start_xmit,
	.ndo_get_stats64	= ei_get_stats64,
	.ndo_do_ioctl		= ei_ioctl,
	.ndo_change_mtu		= ei_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

/**
 * raeth_init - Module Init code
 *
 * Called by kernel to register net_device
 *
 */
static int
__init raeth_init(void)
{
	int ret;
	struct net_device *dev;
	END_DEVICE *ei_local;

	/* MT7620 has FrameEngine CDM bugs: */
#if defined (CONFIG_RALINK_MT7620)
	/* ECO_ID < 4: TX Csum_Gen raise abnormal TX flood and FrameEngine hungs */
	if ((ralink_asic_rev_id & 0xf) < 4) {
#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
		hw_offload_csg = 0;
#if defined (CONFIG_RAETH_SG_DMA_TX)
		hw_offload_gso = 0;
#endif
#endif
	}
	/* ECO_ID < 5: TSO stuck */
	if ((ralink_asic_rev_id & 0xf) < 5) {
#if defined (CONFIG_RAETH_TSO)
		hw_offload_tso = 0;
#endif
	}
#endif

	dev = alloc_etherdev(sizeof(END_DEVICE));
	if (!dev)
		return -ENOMEM;

	ei_local = netdev_priv(dev);

	ether_setup(dev);
	strcpy(dev->name, DEV_NAME);

	dev->irq = SURFBOARDINT_FE;
	dev->base_addr = RALINK_FRAME_ENGINE_BASE;
	dev->watchdog_timeo = 5*HZ;
	dev->netdev_ops = &ei_netdev_ops;
#if defined (CONFIG_ETHTOOL)
	/* init mii structure */
	ei_local->mii_info.dev = dev;
	ei_local->mii_info.mdio_read = mdio_read;
	ei_local->mii_info.mdio_write = mdio_write;
	ei_local->mii_info.phy_id_mask = 0x1f;
	ei_local->mii_info.reg_num_mask = 0x1f;
	ei_local->mii_info.phy_id = 1;
	ei_local->mii_info.supports_gmii = mii_check_gmii_support(&ei_local->mii_info);
	dev->ethtool_ops = &ra_ethtool_ops;
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
	fill_hw_vlan_tx();
#endif
	fill_dev_features(dev);

	/* Register net device (eth2) for the driver */
	ret = register_netdev(dev);
	if (ret != 0) {
		free_netdev(dev);
		printk(KERN_WARNING " " __FILE__ ": No ethernet port found.\n");
		return ret;
	}

	dev_raether = dev;

	debug_proc_init();

	fe_phy_init_early();

#if defined (CONFIG_RAETH_ESW_CONTROL)
	esw_ioctl_init();
#endif

	printk("Ralink APSoC Ethernet Driver %s (%s)\n", RAETH_VERSION, RAETH_DEV_NAME);
#if defined (CONFIG_RAETH_QDMA)
	printk("%s: PDMA RX ring %d, QDMA TX pool %d. Max packet size %d\n", RAETH_DEV_NAME, NUM_RX_DESC, NUM_TX_DESC, MAX_RX_LENGTH);
#else
	printk("%s: PDMA RX ring %d, PDMA TX ring %d. Max packet size %d\n", RAETH_DEV_NAME, NUM_RX_DESC, NUM_TX_DESC, MAX_RX_LENGTH);
#endif
#if defined (CONFIG_RAETH_NAPI)
#if defined (CONFIG_RAETH_NAPI_GRO)
	printk("%s: NAPI & GRO support, weight %d\n", RAETH_DEV_NAME, NAPI_WEIGHT);
#else
	printk("%s: NAPI support, weight %d\n", RAETH_DEV_NAME, NAPI_WEIGHT);
#endif
#endif
#if defined (CONFIG_RAETH_BQL)
	printk("%s: Byte Queue Limits (BQL) support\n", RAETH_DEV_NAME);
#endif

	return 0;
}

/**
 * raeth_uninit - Module Exit code
 *
 * Cmd 'rmmod' will invode the routine to exit the module
 *
 */
static void
__exit raeth_exit(void)
{
	struct net_device *dev = dev_raether;
	if (!dev)
		return;

#if defined (CONFIG_RAETH_ESW_CONTROL)
	esw_ioctl_uninit();
#endif

	unregister_netdev(dev);
	free_netdev(dev);

	debug_proc_exit();

	dev_raether = NULL;
}

module_init(raeth_init);
module_exit(raeth_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ralink APSoC Ethernet Driver");
