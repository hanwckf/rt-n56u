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
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
extern int (*ra_sw_nat_hook_ec)(int engine_init);
struct FoeEntry *PpeFoeBase = NULL;
dma_addr_t PpeFoeBasePhy = 0;
#endif

#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
#if defined (RA_MTD_RW_BY_NUM)
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);
#else
extern int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);
#endif
#endif

#if defined (CONFIG_RALINK_RT3052_MP2)
extern int32_t mcast_rx(struct sk_buff * skb);
extern int32_t mcast_tx(struct sk_buff * skb);
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
static unsigned int eth_min_pkt_len = ETH_ZLEN;
static int eth_close = 1; /* default disable rx/tx processing while init */

//////////////////////////////////////////////////////////////

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
struct FoeEntry *get_foe_table(dma_addr_t *dma_handle, uint32_t *FoeTblSize)
{
	if (dma_handle)
		*dma_handle = PpeFoeBasePhy;

	if (FoeTblSize)
		*FoeTblSize = FOE_4TB_SIZ;

	return PpeFoeBase;
}
EXPORT_SYMBOL(get_foe_table);
#endif

static void raeth_ring_free(END_DEVICE *ei_local)
{
	int i;

	/* Clear adapter TX/RX rings */
	sysRegWrite(TX_BASE_PTR0, 0);
	sysRegWrite(TX_MAX_CNT0, 0);
	sysRegWrite(RX_BASE_PTR0, 0);
	sysRegWrite(RX_MAX_CNT0,  0);

	/* Free RX buffers */
	for (i = 0; i < NUM_RX_DESC; i++)
	{
		if (ei_local->rx0_skbuf[i]) {
			dev_kfree_skb(ei_local->rx0_skbuf[i]);
			ei_local->rx0_skbuf[i] = NULL;
		}
	}

	/* RX Ring */
	if (ei_local->rx_ring0) {
#if defined (CONFIG_RAETH_32B_DESC)
		kfree(ei_local->rx_ring0);
#else
		dma_free_coherent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring0, ei_local->phy_rx_ring0);
#endif
		ei_local->rx_ring0 = NULL;
	}

	/* TX Ring */
	if (ei_local->tx_ring0) {
		dma_free_coherent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring0, ei_local->phy_tx_ring0);
		ei_local->rx_ring0 = NULL;
	}

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* FoE Table */
	if (PpeFoeBase) {
		dma_free_coherent(NULL, FOE_4TB_SIZ * sizeof(struct FoeEntry), PpeFoeBase, PpeFoeBasePhy);
		PpeFoeBase = NULL;
		PpeFoeBasePhy = 0;
	}
#endif
}

static int raeth_ring_alloc(END_DEVICE *ei_local)
{
	int i;

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* FoE Table */
	PpeFoeBase = (struct FoeEntry *)dma_alloc_coherent(NULL, FOE_4TB_SIZ * sizeof(struct FoeEntry), &PpeFoeBasePhy, GFP_KERNEL);
#endif

	ei_local->tx_ring0 = NULL;
	ei_local->rx_ring0 = NULL;
	for (i = 0; i < NUM_RX_DESC; i++)
		ei_local->rx0_skbuf[i] = NULL;

	/* TX Ring */
	ei_local->tx_ring0 = dma_alloc_coherent(NULL, NUM_TX_DESC * sizeof(struct PDMA_txdesc), &ei_local->phy_tx_ring0, GFP_KERNEL);
	if (!ei_local->tx_ring0)
		goto err_cleanup;

	/* RX Ring */
#if defined (CONFIG_RAETH_32B_DESC)
	ei_local->rx_ring0 = kmalloc(NUM_RX_DESC * sizeof(struct PDMA_rxdesc), GFP_KERNEL);
	ei_local->phy_rx_ring0 = virt_to_phys(ei_local->rx_ring0);
#else
	ei_local->rx_ring0 = dma_alloc_coherent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring0, GFP_KERNEL);
#endif
	if (!ei_local->rx_ring0)
		goto err_cleanup;

	/* receiving packet buffer allocation - NUM_RX_DESC x MAX_RX_LENGTH */
	for (i = 0; i < NUM_RX_DESC; i++)
	{
		ei_local->rx0_skbuf[i] = dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN);
		if (!ei_local->rx0_skbuf[i])
			goto err_cleanup;
#if !defined (RAETH_PDMA_V2)
		skb_reserve(ei_local->rx0_skbuf[i], NET_IP_ALIGN);
#endif
	}

	return 0;

err_cleanup:
	raeth_ring_free(ei_local);
	return -ENOMEM;
}

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
static void fill_hw_vlan_tx(void)
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

static void update_hw_vlan_tx(void)
{
	u32 i, reg_vlan;

	for (i = 0; i < 8; i++) {
		reg_vlan = ((u32)vlan_id_map[(i*2)+1] << 16) | (u32)vlan_id_map[i*2];
#if defined (CONFIG_RALINK_MT7620)
		*(volatile u32 *)(RALINK_FRAME_ENGINE_BASE + 0x430 + i*4) = reg_vlan;
#else
		*(volatile u32 *)(RALINK_FRAME_ENGINE_BASE + 0xa8 + i*4) = reg_vlan;
#endif
	}
}

u32 get_map_hw_vlan_tx(u32 idx)
{
	return (u32)vlan_id_map[(idx & 0xF)];
}

void set_map_hw_vlan_tx(u32 idx, u32 vid)
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

static void fe_reset(void)
{
	u32 val;

	val = sysRegRead(REG_RSTCTRL);

	/* RT5350/MT7628 (SDMA) need to reset ESW and FE at the same to avoid PDMA panic */
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	val |= RALINK_ESW_RST;
#endif

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
		mdelay(1);
	}

	val &= ~(RALINK_ETH_RST);
#endif

#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628) || defined (CONFIG_RALINK_MT7620)
	val &= ~(RALINK_ESW_RST);
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	val &= ~(RALINK_PPE_RST);
#endif

	val &= ~(RALINK_FE_RST);
	sysRegWrite(REG_RSTCTRL, val);
	udelay(10);
}

static void fe_mac1_addr_set(unsigned char p[6])
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
static void fe_mac2_addr_set(unsigned char p[6])
{
	u32 regValue;

	regValue = (p[0] << 8) | (p[1]);
	sysRegWrite(GDMA2_MAC_ADRH, regValue);

	regValue = (p[2] << 24) | (p[3] << 16) | (p[4] << 8) | p[5];
	sysRegWrite(GDMA2_MAC_ADRL, regValue);
}
#endif

static void fe_forward_config(struct net_device *dev)
{
#if defined (RAETH_SDMA)
	/* RT5350/MT7628 SDMA: No GDMA, PSE, CDMA, PPE */
	u32 sdmVal;

	sdmVal = sysRegRead(SDM_CON);
#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	sdmVal |= (0x7<<16); // UDPCS, TCPCS, IPCS=1
	
	dev->features |= NETIF_F_RXCSUM; /* Can RX checksum */
	printk("%s: HW IP/TCP/UDP checksum %s offload enabled\n", RAETH_DEV_NAME, "RX");
#endif
#if defined (CONFIG_RAETH_SPECIAL_TAG)
	sdmVal |= (0x1<<20); // TCI_81XX
#endif
	sysRegWrite(SDM_CON, sdmVal);

#else /* !RAETH_SDMA */

	u32 regVal, regCsg;
#if defined (CONFIG_PSEUDO_SUPPORT)
	u32 regVal2;
#endif

#if defined (CONFIG_PSEUDO_SUPPORT)
	eth_min_pkt_len = ETH_ZLEN; // pad to 60 bytes (no VLAN tag)
#else
	eth_min_pkt_len = VLAN_ETH_ZLEN; // pad to 64 bytes
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
		eth_min_pkt_len = ETH_ZLEN; // pad to 60 bytes
		dev->features |= NETIF_F_HW_VLAN_CTAG_TX;
		printk("%s: HW VLAN %s offload enabled\n", RAETH_DEV_NAME, "TX");
	}
#endif

	regCsg = sysRegRead(CDMA_CSG_CFG);
	regVal = sysRegRead(GDMA1_FWD_CFG);
#if defined (CONFIG_PSEUDO_SUPPORT)
	regVal2 = sysRegRead(GDMA2_FWD_CFG);
	/* set unicast/multicast/broadcast/other frames forward to cpu */
	regVal2 &= ~0xFFFF;
#endif

#if defined (CONFIG_RALINK_MT7620)
	/* GDMA1 frames destination port is port0 CPU */
	regVal &= ~0x7;
#else
	/* set unicast/multicast/broadcast/other frames forward to cpu */
	regVal &= ~0xFFFF;
#endif

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	regVal |= GDM1_TCI_81XX;
#endif

	regCsg &= ~0x7;

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	regVal |= GDM1_ICS_EN;
	regVal |= GDM1_TCS_EN;
	regVal |= GDM1_UCS_EN;
#if defined (CONFIG_PSEUDO_SUPPORT)
	regVal2 |= GDM1_ICS_EN;
	regVal2 |= GDM1_TCS_EN;
	regVal2 |= GDM1_UCS_EN;
#endif
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

#if defined (CONFIG_PSEUDO_SUPPORT)
	regVal2 &= ~GDM1_ICS_EN;
	regVal2 &= ~GDM1_TCS_EN;
	regVal2 &= ~GDM1_UCS_EN;
#endif

	dev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_RXCSUM); /* disable checksum TCP/UDP over IPv4 */
#endif

#if defined (CONFIG_RAETH_JUMBOFRAME)
	regVal |= GDM1_JMB_EN;
	regVal &= ~0xf0000000; /* clear bit28-bit31 */
	regVal |= (((MAX_RX_LENGTH/1024)&0xf) << 28);
#if defined (CONFIG_PSEUDO_SUPPORT)
	regVal2 |= GDM1_JMB_EN;
	regVal2 &= ~0xf0000000; /* clear bit28-bit31 */
	regVal2 |= (((MAX_RX_LENGTH/1024)&0xf) << 28);
#endif
#endif

	sysRegWrite(GDMA1_FWD_CFG, regVal);
	sysRegWrite(CDMA_CSG_CFG, regCsg);
#if defined (CONFIG_PSEUDO_SUPPORT)
	sysRegWrite(GDMA2_FWD_CFG, regVal2);
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
#elif defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_MT7620) || \
      defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
        /*use default value*/
#else
	sysRegWrite(PSE_FQ_CFG, cpu_to_le32(INIT_VALUE_OF_PSE_FQFC_CFG));
#endif
	/*
	 *FE_RST_GLO register definition -
	 *Bit 0: PSE Rest
	 *Reset PSE after re-programming PSE_FQ_CFG.
	 */
	sysRegWrite(FE_RST_GLO, 1);
	sysRegWrite(FE_RST_GLO, 0);	// update for RSTCTL issue

	regCsg = sysRegRead(CDMA_CSG_CFG);
	regVal = sysRegRead(GDMA1_FWD_CFG);
#if defined (CONFIG_PSEUDO_SUPPORT)
	regVal2 = sysRegRead(GDMA2_FWD_CFG);
#endif

#endif /* RAETH_SDMA */

#if defined (CONFIG_RAETH_BQL)
	printk("%s: Byte Queue Limits (BQL) support\n", RAETH_DEV_NAME);
#endif
}

static void wait_pdma_stop(int cycles_10ms)
{
	int i;
	u32 regVal;

	for (i=0; i < cycles_10ms; i++) {
		regVal = sysRegRead(PDMA_GLO_CFG);
		if ((regVal & RX_DMA_BUSY)) {
			msleep(10);
			continue;
		}
		if ((regVal & TX_DMA_BUSY)) {
			msleep(10);
			continue;
		}
		break;
	}
}

static void fe_pdma_init(struct net_device *dev)
{
	int i;
	unsigned int regVal;
	END_DEVICE* ei_local = netdev_priv(dev);

	wait_pdma_stop(10);

	/* initial TX ring 0 */
	ei_local->tx_free_idx =0;
	for (i=0; i < NUM_TX_DESC; i++) {
		ei_local->tx0_free[i] = NULL;
		ei_local->tx_ring0[i].txd_info1_u32 = 0;
		ei_local->tx_ring0[i].txd_info3_u32 = 0;
#if defined (RAETH_PDMA_V2)
		ei_local->tx_ring0[i].txd_info4_u32 = 0;
#else
		ei_local->tx_ring0[i].txd_info4_u32 = TX4_DMA_QN(3);
#endif
		ei_local->tx_ring0[i].txd_info2_u32 = TX2_DMA_DONE;
	}

	/* initial RX ring 0 */
	for (i = 0; i < NUM_RX_DESC; i++) {
		ei_local->rx_ring0[i].rxd_info1_u32 = (unsigned int)dma_map_single(NULL, ei_local->rx0_skbuf[i]->data, MAX_RX_LENGTH, DMA_FROM_DEVICE);
		ei_local->rx_ring0[i].rxd_info4_u32 = 0;
		ei_local->rx_ring0[i].rxd_info3_u32 = 0;
#if defined (RAETH_PDMA_V2)
		ei_local->rx_ring0[i].rxd_info2_u32 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
#else
		ei_local->rx_ring0[i].rxd_info2_u32 = RX2_DMA_LS0;
#endif
		dma_cache_sync(NULL, &ei_local->rx_ring0[i], sizeof(struct PDMA_rxdesc), DMA_TO_DEVICE);
	}

	/* clear PDMA */
	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= ~(0x000000FF);
	sysRegWrite(PDMA_GLO_CFG, regVal);

	/* tell the adapter where the TX/RX rings are located. */
	sysRegWrite(TX_BASE_PTR0, phys_to_bus((u32)ei_local->phy_tx_ring0));
	sysRegWrite(TX_MAX_CNT0, cpu_to_le32((u32)NUM_TX_DESC));
	sysRegWrite(TX_CTX_IDX0, 0);
	sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX0);
#if defined (RAETH_PDMAPTR_FROM_VAR)
	ei_local->tx_calc_idx = 0;
#endif

	sysRegWrite(RX_BASE_PTR0, phys_to_bus((u32)ei_local->phy_rx_ring0));
	sysRegWrite(RX_MAX_CNT0,  cpu_to_le32((u32)NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX0, cpu_to_le32((u32)(NUM_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX0);
#if defined (RAETH_PDMAPTR_FROM_VAR)
	ei_local->rx_calc_idx = sysRegRead(RX_CALC_IDX0);
#endif

	/* only the following chipset need to set it */
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
	//set 1us timer count in unit of clock cycle
	regVal = sysRegRead(FE_GLO_CFG);
	regVal &= ~(0xff << 8); //clear bit8-bit15
	regVal |= (((get_surfboard_sysclk()/1000000)) << 8);
	sysRegWrite(FE_GLO_CFG, regVal);
#endif
}

static void fe_pdma_start(void)
{
	unsigned int pdma_glo_cfg = (TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
	pdma_glo_cfg |= PDMA_BT_SIZE_16DWORDS;
#else
	pdma_glo_cfg |= PDMA_BT_SIZE_8DWORDS;
#endif
#if defined (RAETH_PDMA_V2)
	pdma_glo_cfg |= RX_2B_OFFSET;
#endif
#if defined (CONFIG_RAETH_32B_DESC)
	pdma_glo_cfg |= DESC_32B_EN;
#endif
	sysRegWrite(PDMA_GLO_CFG, pdma_glo_cfg);
}

static void fe_pdma_stop(void)
{
	unsigned int regValue;

	regValue = sysRegRead(PDMA_GLO_CFG);
	regValue &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regValue);
}

static void read_counters_gdma1(END_DEVICE *ei_local)
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
static void read_counters_gdma2(PSEUDO_ADAPTER *pPseudoAd)
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

static void inc_rx_drop(END_DEVICE *ei_local, int gmac_no)
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

static inline int raeth_recv(struct net_device* dev)
{
	struct sk_buff *new_skb, *rx_skb;
	struct PDMA_rxdesc *rx_ring;
	unsigned int length;
	unsigned int rxd_info4;
#if defined (CONFIG_RAETH_HW_VLAN_RX)
	unsigned int rx_vlan_tag;
	unsigned int rx_vlan_vid;
#endif
	int gmac_no = PSE_PORT_GMAC1;
	int rx_dma_owner_idx;
	int rx_processed = 0;
	int bReschedule = 0;
	END_DEVICE* ei_local = netdev_priv(dev);
#if defined (CONFIG_RAETH_SPECIAL_TAG)
	struct vlan_ethhdr *veth;
#endif

#if defined (RAETH_PDMAPTR_FROM_VAR)
	rx_dma_owner_idx = (ei_local->rx_calc_idx + 1) % NUM_RX_DESC;
#else
	rx_dma_owner_idx = (sysRegRead(RX_CALC_IDX0) + 1) % NUM_RX_DESC;
#endif

#if defined (CONFIG_RAETH_32B_DESC)
	dma_cache_sync(NULL, &ei_local->rx_ring0[rx_dma_owner_idx], sizeof(struct PDMA_rxdesc), DMA_FROM_DEVICE);
#endif

	for ( ; ; ) {
		if (rx_processed++ > NUM_RX_MAX_PROCESS) {
			// need to reschedule rx handle
			bReschedule = 1;
			break;
		}
		rx_ring = &ei_local->rx_ring0[rx_dma_owner_idx];
		if (!(rx_ring->rxd_info2_u32 & RX2_DMA_DONE))
			break;
		
#if defined (CONFIG_32B_DESC)
		prefetch(&ei_local->rx_ring0[((rx_dma_owner_idx + 1) % NUM_RX_DESC)]);
#endif
		length = RX2_DMA_SDL0_GET(rx_ring->rxd_info2_u32);
#if defined (CONFIG_RAETH_HW_VLAN_RX)
		rx_vlan_tag = (rx_ring->rxd_info2_u32 & RX2_DMA_TAG);
		rx_vlan_vid = RX3_DMA_VID(rx_ring->rxd_info3_u32);
#endif
		rxd_info4 = rx_ring->rxd_info4_u32;
#if defined (CONFIG_PSEUDO_SUPPORT)
		gmac_no = RX4_DMA_SP(rxd_info4);
#endif
		
		/* We have to check the free memory size is big enough
		 * before pass the packet to cpu */
		new_skb = __dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN, GFP_ATOMIC);
		if (unlikely(new_skb == NULL))
		{
#if defined (RAETH_PDMA_V2)
			rx_ring->rxd_info2_u32 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
#else
			rx_ring->rxd_info2_u32 = RX2_DMA_LS0;
#endif
			sysRegWrite(RX_CALC_IDX0, rx_dma_owner_idx);
#if defined (RAETH_PDMAPTR_FROM_VAR)
			ei_local->rx_calc_idx = rx_dma_owner_idx;
#endif
			inc_rx_drop(ei_local, gmac_no);
			bReschedule = 1;
			if (net_ratelimit())
				printk(KERN_ERR "%s: Failed to alloc new RX skb! (GMAC: %d)\n", RAETH_DEV_NAME, gmac_no);
			break;
		}
#if !defined (RAETH_PDMA_V2)
		skb_reserve(new_skb, NET_IP_ALIGN);
#endif
		/* map new buffer to ring (unmap is not required on generic mips mm) */
		rx_ring->rxd_info1_u32 = (unsigned int)dma_map_single(NULL, new_skb->data, MAX_RX_LENGTH, DMA_FROM_DEVICE);
#if defined (RAETH_PDMA_V2)
		rx_ring->rxd_info2_u32 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
#else
		rx_ring->rxd_info2_u32 = RX2_DMA_LS0;
#endif
		dma_cache_sync(NULL, rx_ring, sizeof(struct PDMA_rxdesc), DMA_TO_DEVICE);
		
		/* skb processing */
		rx_skb = ei_local->rx0_skbuf[rx_dma_owner_idx];
		
		rx_skb->len = length;
#if defined (RAETH_PDMA_V2)
		rx_skb->data += NET_IP_ALIGN;
#endif
		rx_skb->tail = rx_skb->data + length;
		
#if defined (CONFIG_PSEUDO_SUPPORT)
		if (gmac_no == PSE_PORT_GMAC2)
			rx_skb->protocol = eth_type_trans(rx_skb, ei_local->PseudoDev);
		else
#endif
			rx_skb->protocol = eth_type_trans(rx_skb, dev);

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
		FOE_MAGIC_TAG(rx_skb) = FOE_MAGIC_GE;
		DO_FILL_FOE_DESC(rx_skb, (rxd_info4 & ~(RX4_DMA_ALG_SET)));
#endif

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
		if (rxd_info4 & RX4_DMA_L4FVLD)
			rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
#endif
			rx_skb->ip_summed = CHECKSUM_NONE;

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		// port0: 0x8100 => 0x8100 0001
		// port1: 0x8101 => 0x8100 0002
		// port2: 0x8102 => 0x8100 0003
		// port3: 0x8103 => 0x8100 0004
		// port4: 0x8104 => 0x8100 0005
		// port5: 0x8105 => 0x8100 0006
		veth = vlan_eth_hdr(rx_skb);
		if ((veth->h_vlan_proto & 0xFF) == 0x81) {
			veth->h_vlan_TCI = htons( (((veth->h_vlan_proto >> 8) & 0xF) + 1) );
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
#if defined (CONFIG_RALINK_RT3052_MP2)
			if (mcast_rx(rx_skb)==0)
				kfree_skb(rx_skb);
			else
#else
#if defined (CONFIG_RAETH_HW_VLAN_RX)
			if (rx_vlan_tag)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
				__vlan_hwaccel_put_tag(rx_skb, __constant_htons(ETH_P_8021Q), rx_vlan_vid);
#else
				__vlan_hwaccel_put_tag(rx_skb, rx_vlan_vid);
#endif
#endif
#endif
			netif_rx(rx_skb);
		}
		
		ei_local->rx0_skbuf[rx_dma_owner_idx] = new_skb;
		
		/* move point to next RXD which wants to alloc */
		sysRegWrite(RX_CALC_IDX0, rx_dma_owner_idx);
#if defined (RAETH_PDMAPTR_FROM_VAR)
		ei_local->rx_calc_idx = rx_dma_owner_idx;
#endif
		/* update to next packet point that was received. */
		rx_dma_owner_idx = (rx_dma_owner_idx + 1) % NUM_RX_DESC;
	}

	return bReschedule;
}

///////////////////////////////////////////////////////////////////
/////
///// ei_receive - process the next incoming packet
/////
///// Handle one incoming packet.  The packet is checked for errors and sent
///// to the upper layer.
/////
///// RETURNS: OK on success or ERROR.
///////////////////////////////////////////////////////////////////

void ei_receive(unsigned long ptr)
{
	struct net_device *dev = (struct net_device *)ptr;
	END_DEVICE *ei_local = netdev_priv(dev);
	u32 reg_int_mask;
	unsigned long flags;

	 /* protect eth while init or reinit */
	if (eth_close)
		return;

	if (raeth_recv(dev)) {
		tasklet_schedule(&ei_local->rx_tasklet);
	} else {
		spin_lock_irqsave(&ei_local->irqe_lock, flags);
		reg_int_mask = sysRegRead(FE_INT_ENABLE);
		sysRegWrite(FE_INT_ENABLE, reg_int_mask | RX_DLY_INT);
		spin_unlock_irqrestore(&ei_local->irqe_lock, flags);
	}
}

static void __maybe_unused inc_tx_drop(END_DEVICE *ei_local, int gmac_no)
{
#if defined (CONFIG_PSEUDO_SUPPORT)
	PSEUDO_ADAPTER *pAd;

	if (gmac_no == PSE_PORT_GMAC2) {
		pAd = netdev_priv(ei_local->PseudoDev);
		pAd->stat.tx_dropped++;
	} else
#endif
	if (gmac_no == PSE_PORT_GMAC1)
		ei_local->stat.tx_dropped++;
}

inline int ei_start_xmit(struct sk_buff* skb, struct net_device *dev, END_DEVICE *ei_local, int gmac_no)
{
	struct PDMA_txdesc *tx_ring, *tx_ring_start;
	u32 i;
	u32 nr_slots;
	u32 tx_cpu_owner_idx;
	u32 tx_cpu_owner_idx_next;
	u32 txd_info4;
	unsigned long flags;
#if defined (CONFIG_RAETH_SG_DMA_TX)
	u32 nr_frags, txd_info2;
	const skb_frag_t *tx_frag;
	struct skb_shared_info *shinfo;
#endif

	/* protect eth while init or reinit */
	if (eth_close) {
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if (ra_sw_nat_hook_tx != NULL) {
		if (ra_sw_nat_hook_tx(skb, gmac_no)==0) {
			dev_kfree_skb(skb);
			return NETDEV_TX_OK;
		}
		if (IS_DPORT_PPE_VALID(skb))
			gmac_no = PSE_PORT_PPE;
	}
#endif

#if defined (CONFIG_RALINK_RT3052_MP2)
	mcast_tx(skb);
#endif

#if !defined (CONFIG_RALINK_MT7621)
	if (skb->len < eth_min_pkt_len) {
		if (skb_padto(skb, eth_min_pkt_len)) {
			inc_tx_drop(ei_local, gmac_no);
			if (net_ratelimit())
				printk(KERN_ERR "%s: skb_padto failed\n", RAETH_DEV_NAME);
			return NETDEV_TX_OK;
		}
		skb_put(skb, eth_min_pkt_len - skb->len);
	}
#endif

	spin_lock_irqsave(&ei_local->page_lock, flags);

#if defined (RAETH_PDMAPTR_FROM_VAR)
	tx_cpu_owner_idx = ei_local->tx_calc_idx;
#else
	tx_cpu_owner_idx = sysRegRead(TX_CTX_IDX0);
#endif

#if defined (CONFIG_RAETH_SG_DMA_TX)
	shinfo = skb_shinfo(skb);
	nr_frags = shinfo->nr_frags;
	nr_slots = (nr_frags >> 1) + 1;
#else
	nr_slots = 1;
#endif
	for (i = 0; i <= nr_slots; i++) {
		tx_cpu_owner_idx_next = (tx_cpu_owner_idx + i) % NUM_TX_DESC;
		if (ei_local->tx0_free[tx_cpu_owner_idx_next] ||
		  !(ei_local->tx_ring0[tx_cpu_owner_idx_next].txd_info2_u32 & TX2_DMA_DONE)) {
			netif_stop_queue(dev);
			spin_unlock_irqrestore(&ei_local->page_lock, flags);
#if defined (CONFIG_RAETH_DEBUG)
			if (net_ratelimit())
				printk("%s: tx_ring full! (GMAC: %d)\n", RAETH_DEV_NAME, gmac_no);
#endif
			return NETDEV_TX_BUSY;
		}
	}

#if defined (CONFIG_RALINK_MT7620)
	if (gmac_no == PSE_PORT_PPE)
		txd_info4 = TX4_DMA_FP_BMAP(0x80); /* P7 */
	else
#if defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_ESW)
		txd_info4 = TX4_DMA_FP_BMAP(0x20); /* P5 */
#elif defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_ESW)
		txd_info4 = TX4_DMA_FP_BMAP(0x10); /* P4 */
#else
		txd_info4 = 0; /* routing by DA */
#endif
#elif defined (CONFIG_RALINK_MT7621)
	txd_info4 = TX4_DMA_FPORT(gmac_no);
#else
	txd_info4 = (TX4_DMA_QN(3) | TX4_DMA_PN(gmac_no));
#endif

#if defined (CONFIG_RAETH_TSO)
	/* fill MSS info in tcp checksum field */
	if (shinfo->gso_segs > 1) {
		if (skb_header_cloned(skb)) {
			if (pskb_expand_head(skb, 0, 0, GFP_ATOMIC)) {
				inc_tx_drop(ei_local, gmac_no);
				spin_unlock_irqrestore(&ei_local->page_lock, flags);
				dev_kfree_skb(skb);
				if (net_ratelimit())
					printk(KERN_ERR "%s: pskb_expand_head for TSO failed!\n", RAETH_DEV_NAME);
				return NETDEV_TX_OK;
			}
		}
		if ((shinfo->gso_type & SKB_GSO_TCPV4)
#if defined (CONFIG_RAETH_TSOV6)
		 || (shinfo->gso_type & SKB_GSO_TCPV6)
#endif
		) {
			int hdr_len = (skb_transport_offset(skb) + tcp_hdrlen(skb));
			if (skb->len > hdr_len) {
				tcp_hdr(skb)->check = htons(shinfo->gso_size);
				txd_info4 |= TX4_DMA_TSO;
			}
		}
	}
#endif

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD) && !defined (RAETH_SDMA)
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		txd_info4 |= TX4_DMA_TUI_CO(7);
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX)
	if (vlan_tx_tag_present(skb)) {
#if !defined (CONFIG_RALINK_MT7621)
		unsigned int vlan_tci = vlan_tx_tag_get(skb);
		txd_info4 |= (TX4_DMA_INSV | TX4_DMA_VPRI(vlan_tci));
		txd_info4 |= (u32)vlan_4k_map[(vlan_tci & VLAN_VID_MASK)];
#else
		txd_info4 |= (0x10000 | vlan_tx_tag_get(skb));
#endif
	}
#endif

	ei_local->tx0_free[tx_cpu_owner_idx] = skb;

	/* write DMA TX desc (DDONE must be cleared last) */
	tx_ring = &ei_local->tx_ring0[tx_cpu_owner_idx];
	tx_ring_start = tx_ring;

	tx_ring->txd_info4_u32 = txd_info4;
#if defined (CONFIG_RAETH_SG_DMA_TX)
	if (nr_frags) {
		tx_ring->txd_info1_u32 = dma_map_single(NULL, skb->data, skb_headlen(skb), DMA_TO_DEVICE);
		
		txd_info2 = TX2_DMA_SDL0(skb_headlen(skb));
		for (i = 0; i < nr_frags; i++) {
			tx_frag = &shinfo->frags[i];
			if (i % 2) {
				tx_cpu_owner_idx = (tx_cpu_owner_idx + 1) % NUM_TX_DESC;
				ei_local->tx0_free[tx_cpu_owner_idx] = (struct sk_buff *)0xFFFFFFFF; //MAGIC ID
				tx_ring = &ei_local->tx_ring0[tx_cpu_owner_idx];
				
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
				tx_ring->txd_info1_u32 = skb_frag_dma_map(NULL, tx_frag, 0, skb_frag_size(tx_frag), DMA_TO_DEVICE);
#else
				tx_ring->txd_info1_u32 = dma_map_page(NULL, tx_frag->page, tx_frag->page_offset, tx_frag->size, DMA_TO_DEVICE);
#endif
				tx_ring->txd_info4_u32 = txd_info4;
				if ((i + 1) == nr_frags) { // last segment
					tx_ring->txd_info3_u32 = 0;
					tx_ring->txd_info2_u32 = (TX2_DMA_SDL0(tx_frag->size) | TX2_DMA_LS0);
				} else
					txd_info2 = TX2_DMA_SDL0(tx_frag->size);
			} else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
				tx_ring->txd_info3_u32 = skb_frag_dma_map(NULL, tx_frag, 0, skb_frag_size(tx_frag), DMA_TO_DEVICE);
#else
				tx_ring->txd_info3_u32 = dma_map_page(NULL, tx_frag->page, tx_frag->page_offset, tx_frag->size, DMA_TO_DEVICE);
#endif
				if ((i + 1) == nr_frags) // last segment
					txd_info2 |= (TX2_DMA_SDL1(tx_frag->size) | TX2_DMA_LS1);
				else
					txd_info2 |= (TX2_DMA_SDL1(tx_frag->size) | TX2_DMA_BURST);
				
				tx_ring->txd_info2_u32 = txd_info2;
			}
		}
	} else
#endif
	{
		tx_ring->txd_info1_u32 = dma_map_single(NULL, skb->data, skb->len, DMA_TO_DEVICE);
		tx_ring->txd_info2_u32 = (TX2_DMA_SDL0(skb->len) | TX2_DMA_LS0);
	}

	dma_cache_sync(NULL, tx_ring_start, nr_slots * sizeof(struct PDMA_txdesc), DMA_TO_DEVICE);

	/* kick the DMA TX */
	sysRegWrite(TX_CTX_IDX0, tx_cpu_owner_idx_next);
#if defined (RAETH_PDMAPTR_FROM_VAR)
	ei_local->tx_calc_idx = tx_cpu_owner_idx_next;
#endif

#if defined (CONFIG_RAETH_BQL)
	netdev_sent_queue(dev, skb->len);
#endif

	/* check next free descriptor */
	tx_cpu_owner_idx_next = (tx_cpu_owner_idx_next + 1) % NUM_TX_DESC;
	if (ei_local->tx0_free[tx_cpu_owner_idx_next]) {
		netif_stop_queue(dev);
	}

	spin_unlock_irqrestore(&ei_local->page_lock, flags);

	return NETDEV_TX_OK;
}

static inline void ei_xmit_housekeeping(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	struct PDMA_txdesc *tx_ring;
	struct sk_buff *tx_skb;
	unsigned int pkts_sent = 0;
#if defined (CONFIG_RAETH_BQL)
	unsigned int bytes_sent = 0;
#if defined (CONFIG_PSEUDO_SUPPORT)
	unsigned int bytes_sent_virt = 0;
#endif
#endif
	spin_lock(&ei_local->page_lock);

	for (;;) {
		tx_ring = &ei_local->tx_ring0[ei_local->tx_free_idx];
		tx_skb = ei_local->tx0_free[ei_local->tx_free_idx];
		if (!tx_skb || !(tx_ring->txd_info2_u32 & TX2_DMA_DONE))
			break;
		
		tx_ring->txd_info2_u32 = TX2_DMA_DONE;
#if defined (CONFIG_RAETH_SG_DMA_TX)
		if (tx_skb != (struct sk_buff *)0xFFFFFFFF)
#endif
		{
#if defined (CONFIG_RAETH_BQL)
#if defined (CONFIG_PSEUDO_SUPPORT)
			if (tx_skb->dev == ei_local->PseudoDev)
				bytes_sent_virt += tx_skb->len;
			else
#endif
			bytes_sent += tx_skb->len;
#endif
			dev_kfree_skb_irq(tx_skb);
		}
		ei_local->tx0_free[ei_local->tx_free_idx] = NULL;
		ei_local->tx_free_idx = (ei_local->tx_free_idx + 1) % NUM_TX_DESC;
		pkts_sent++;
	}

#if defined (CONFIG_RAETH_BQL)
#if defined (CONFIG_PSEUDO_SUPPORT)
	if (bytes_sent_virt)
		netdev_completed_queue(ei_local->PseudoDev, pkts_sent, bytes_sent_virt);
#endif
	if (bytes_sent)
		netdev_completed_queue(dev, pkts_sent, bytes_sent);
#endif

	if (pkts_sent) {
		if (dev->flags & IFF_UP) {
			if (netif_queue_stopped(dev))
				netif_wake_queue(dev);
		}
#if defined (CONFIG_PSEUDO_SUPPORT)
		if (ei_local->PseudoDev->flags & IFF_UP) {
			if (netif_queue_stopped(ei_local->PseudoDev))
				netif_wake_queue(ei_local->PseudoDev);
		}
#endif
	}

	spin_unlock(&ei_local->page_lock);
}

/**
 * ei_interrupt - handle controler interrupt
 *
 * This routine is called at interrupt level in response to an interrupt from
 * the controller.
 *
 * RETURNS: N/A.
 */
static irqreturn_t ei_interrupt(int irq, void *dev_id)
{
	u32 reg_int_val;
	u32 reg_int_mask;
	END_DEVICE *ei_local;
	struct net_device *dev = (struct net_device *) dev_id;
	if (!dev)
		return IRQ_NONE;

	ei_local = netdev_priv(dev);

	reg_int_val = sysRegRead(FE_INT_STATUS);
	if (!reg_int_val)
		return IRQ_NONE;

	sysRegWrite(FE_INT_STATUS, reg_int_val);

	if (reg_int_val & TX_DLY_INT) {
		ei_xmit_housekeeping(dev);
	}

	if (reg_int_val & RX_DLY_INT) {
		spin_lock(&ei_local->irqe_lock);
		reg_int_mask = sysRegRead(FE_INT_ENABLE);
		sysRegWrite(FE_INT_ENABLE, reg_int_mask & ~(RX_DLY_INT));
		spin_unlock(&ei_local->irqe_lock);
		tasklet_hi_schedule(&ei_local->rx_tasklet);
	}

	return IRQ_HANDLED;
}

static int ei_change_mtu(struct net_device *dev, int new_mtu)
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

#if defined (CONFIG_RAETH_HW_VLAN_RX)
static int ei_vlan_rx_add_vid(struct net_device *dev, u16 vid)
{
	// not need add vlan id to hardware
	return 0;
}

static int ei_vlan_rx_kill_vid(struct net_device *dev, u16 vid)
{
	// not need delete vlan id from hardware
	return 0;
}
#endif

static void fill_dev_features(struct net_device *dev)
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
struct rtnl_link_stats64 *VirtualIF_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);
	END_DEVICE *ei_local = netdev_priv(pPseudoAd->RaethDev);

	spin_lock(&ei_local->stat_lock);
	read_counters_gdma2(pPseudoAd);
	memcpy(stats, &pPseudoAd->stat, sizeof(struct rtnl_link_stats64));
	spin_unlock(&ei_local->stat_lock);

	return stats;
}

int VirtualIF_open(struct net_device *dev)
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

int VirtualIF_close(struct net_device *dev)
{
	netif_stop_queue(dev);

#if defined (CONFIG_RAETH_BQL)
	netdev_reset_queue(dev);
#endif

	printk("%s: ===> VirtualIF_close\n", dev->name);
	return 0;
}

int VirtualIF_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	END_DEVICE *ei_local;
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);

	if (!pPseudoAd->RaethDev || !(pPseudoAd->RaethDev->flags & IFF_UP)) {
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	ei_local = netdev_priv(pPseudoAd->RaethDev);

	return ei_start_xmit(skb, dev, ei_local, PSE_PORT_GMAC2);
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
#if defined (CONFIG_RAETH_HW_VLAN_RX)
	.ndo_vlan_rx_add_vid	= ei_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= ei_vlan_rx_kill_vid,
#endif
};

// Register pseudo interface
static int VirtualIF_init(struct net_device *dev_parent)
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

	ether_setup(dev);
	strcpy(dev->name, DEV2_NAME);

	//Get mac2 address from flash
#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
	i = ra_mtd_read_nm("Factory", GMAC2_OFFSET, 6, addr.sa_data);

	//If reading mtd failed or mac0 is empty, generate a mac address
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
	dev->ethtool_ops = &ra_virt_ethtool_ops;
#endif

	fill_dev_features(dev);

	/* Register this device */
	if (register_netdev(dev) != 0) {
		free_netdev(dev);
		return -ENXIO;
	}

	pPseudoAd = netdev_priv(dev);
	pPseudoAd->RaethDev = dev_parent;

	ei_local = netdev_priv(dev_parent);
	ei_local->PseudoDev = dev;

	memset(&pPseudoAd->stat, 0, sizeof(pPseudoAd->stat));

#if defined (CONFIG_ETHTOOL)
	// init mii structure
	pPseudoAd->mii_info.dev = dev;
	pPseudoAd->mii_info.mdio_read = mdio_virt_read;
	pPseudoAd->mii_info.mdio_write = mdio_virt_write;
	pPseudoAd->mii_info.phy_id_mask = 0x1f;
	pPseudoAd->mii_info.reg_num_mask = 0x1f;
	pPseudoAd->mii_info.phy_id = 0x1e;
	pPseudoAd->mii_info.supports_gmii = mii_check_gmii_support(&pPseudoAd->mii_info);
#endif

	return 0;
}
#endif

int ei_start_xmit_gmac1(struct sk_buff* skb, struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	return ei_start_xmit(skb, dev, ei_local, PSE_PORT_GMAC1);
}

struct rtnl_link_stats64 *ei_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	END_DEVICE *ei_local = netdev_priv(dev);

	spin_lock(&ei_local->stat_lock);
	read_counters_gdma1(ei_local);
	memcpy(stats, &ei_local->stat, sizeof(struct rtnl_link_stats64));
	spin_unlock(&ei_local->stat_lock);

	return stats;
}

static void stat_counters_update(unsigned long ptr)
{
	struct net_device *dev = (struct net_device *)ptr;
	END_DEVICE *ei_local = netdev_priv(dev);

	spin_lock(&ei_local->stat_lock);
	read_counters_gdma1(ei_local);
#if defined (CONFIG_PSEUDO_SUPPORT)
	read_counters_gdma2(netdev_priv(ei_local->PseudoDev));
#endif
	spin_unlock(&ei_local->stat_lock);

	if (!eth_close)
		mod_timer(&ei_local->stat_timer, jiffies + (15 * HZ));
}

/**
 * ei_init - pick up ethernet port at boot time
 * @dev: network device to probe
 *
 * This routine probe the ethernet port at boot time.
 */
int ei_init(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
	int i;
	struct sockaddr addr;
	unsigned char zero1[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char zero2[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
#endif

#if defined (CONFIG_PSEUDO_SUPPORT)
	ei_local->PseudoDev = NULL;
#endif
	spin_lock_init(&ei_local->stat_lock);
	spin_lock_init(&ei_local->page_lock);
	spin_lock_init(&ei_local->irqe_lock);
	init_timer(&ei_local->stat_timer);

	// Get MAC0 address from flash
#if defined (CONFIG_RAETH_READ_MAC_FROM_MTD)
	i = ra_mtd_read_nm("Factory", GMAC0_OFFSET, 6, addr.sa_data);

	//If reading mtd failed or mac0 is empty, generate a mac address
	if (i < 0 || ((memcmp(addr.sa_data, zero1, 6) == 0) || (addr.sa_data[0] & 0x1)) ||
	    (memcmp(addr.sa_data, zero2, 6) == 0)) {
		unsigned char mac_addr01234[5] = {0x00, 0x0C, 0x43, 0x28, 0x80};
		net_srandom(jiffies);
		memcpy(addr.sa_data, mac_addr01234, 5);
		addr.sa_data[5] = net_random()&0xFF;
	}

	memcpy(dev->dev_addr, addr.sa_data, dev->addr_len);
#endif

	if (raeth_ring_alloc(ei_local) != 0) {
		printk(KERN_WARNING "raeth_ring_alloc FAILED!\n");
		return -ENOMEM;
	}

	memset(&ei_local->stat, 0, sizeof(ei_local->stat));

#if defined (CONFIG_ETHTOOL)
	// init mii structure
	ei_local->mii_info.dev = dev;
	ei_local->mii_info.mdio_read = mdio_read;
	ei_local->mii_info.mdio_write = mdio_write;
	ei_local->mii_info.phy_id_mask = 0x1f;
	ei_local->mii_info.reg_num_mask = 0x1f;
	ei_local->mii_info.phy_id = 1;
	ei_local->mii_info.supports_gmii = mii_check_gmii_support(&ei_local->mii_info);
#endif

	return 0;
}

void ei_uninit(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);

#if defined (CONFIG_PSEUDO_SUPPORT)
	if (ei_local->PseudoDev) {
		unregister_netdev(ei_local->PseudoDev);
		free_netdev(ei_local->PseudoDev);
		ei_local->PseudoDev = NULL;
	}
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* down PPE engine before free ring */
	if (ra_sw_nat_hook_ec != NULL)
		ra_sw_nat_hook_ec(0);
#endif

	raeth_ring_free(ei_local);
}

/**
 * ei_open - Open/Initialize the ethernet port.
 * @dev: network device to initialize
 *
 * This routine goes all-out, setting everything
 * up a new at each open, even though many of these registers should only need to be set once at boot.
 */
int ei_open(struct net_device *dev)
{
	int err;
	unsigned long flags;
	END_DEVICE *ei_local;

	if (!try_module_get(THIS_MODULE))
	{
		printk("%s: Cannot reserve module\n", __FUNCTION__);
		return -1;
	}

	if (!dev)
	{
		printk(KERN_EMERG "%s: ei_open passed a non-existent device!\n", dev->name);
		return -ENXIO;
	}

	ei_local = netdev_priv(dev);

	spin_lock_irqsave(&ei_local->page_lock, flags);

	tasklet_init(&ei_local->rx_tasklet, ei_receive, (unsigned long)dev);

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* down PPE engine before FE reset */
	if (ra_sw_nat_hook_ec != NULL)
		ra_sw_nat_hook_ec(0);
#endif

	fe_reset();
	fe_pdma_init(dev);
	fe_phy_init();

	err = request_irq(dev->irq, ei_interrupt, IRQF_DISABLED, dev->name, dev);
	if (err) {
		spin_unlock_irqrestore(&ei_local->page_lock, flags);
		return err;
	}

	fe_forward_config(dev);

	/* set MAC address to ASIC */
	fe_mac1_addr_set(dev->dev_addr);
#if defined (CONFIG_PSEUDO_SUPPORT)
	fe_mac2_addr_set(ei_local->PseudoDev->dev_addr);
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	/* up PPE engine after FE init */
	if (ra_sw_nat_hook_ec != NULL)
		ra_sw_nat_hook_ec(1);
#endif

#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_MT7530_GSW)
	/* prepare switch for INT handling */
	esw_irq_init();
#if defined (CONFIG_RAETH_DHCP_TOUCH)
	esw_dhcpc_init();
#endif
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_RALINK_MT7621)
	/* request interrupt line for MT7620 ESW or MT7621 GSW */
	err = request_irq(SURFBOARDINT_ESW, esw_interrupt, IRQF_DISABLED, "ralink_esw", dev);

	/* enable MT7620 ESW or MT7621 GSW interrupt */
	*((volatile u32 *)(RALINK_INTENA)) = RALINK_INTCTL_ESW;
#elif defined (CONFIG_MT7530_INT_GPIO)
	// todo, needed capture GPIO interrupt for external MT7530
#endif
#endif

	/* delay IRQ to 4 interrupts, delay max 4*20us */
	sysRegWrite(DLY_INT_CFG, 0x84048404);
	sysRegWrite(FE_INT_ENABLE, RX_DLY_INT | TX_DLY_INT);

	eth_close = 0; /* set flag to open protect eth while init or reinit */

#if defined (CONFIG_RAETH_BQL)
	netdev_reset_queue(dev);
#endif
	netif_start_queue(dev);

	fe_pdma_start();

	ei_local->stat_timer.data = (unsigned long)dev;
	ei_local->stat_timer.function = stat_counters_update;
	ei_local->stat_timer.expires = jiffies + (15 * HZ); // 15 s
	add_timer(&ei_local->stat_timer);

	spin_unlock_irqrestore(&ei_local->page_lock, flags);

	return 0;
}

/**
 * ei_close - shut down network device
 * @dev: network device to clear
 *
 * This routine shut down network device.
 */
int ei_close(struct net_device *dev)
{
	int i;
	unsigned long flags;
	END_DEVICE *ei_local = netdev_priv(dev);	// device pointer

	spin_lock_irqsave(&ei_local->page_lock, flags);

	eth_close = 1; /* set closed flag protect eth while init or reinit */

#if defined (CONFIG_PSEUDO_SUPPORT)
	VirtualIF_close(ei_local->PseudoDev);
#endif

	netif_stop_queue(dev);

	fe_pdma_stop();
	msleep(10);
	wait_pdma_stop(50);

	sysRegWrite(FE_INT_ENABLE, 0);

	tasklet_kill(&ei_local->rx_tasklet);

	del_timer_sync(&ei_local->stat_timer);
#if defined (CONFIG_RAETH_DHCP_TOUCH)
	esw_dhcpc_cancel();
#endif

	free_irq(dev->irq, dev);
#if defined (CONFIG_RAETH_ESW) || defined (CONFIG_RALINK_MT7621)
	free_irq(SURFBOARDINT_ESW, dev);
#elif defined (CONFIG_MT7530_INT_GPIO)
	// todo, needed capture GPIO interrupt for external MT7530
#endif

	for (i = 0; i < NUM_RX_DESC; i++) {
		if (ei_local->rx_ring0[i].rxd_info1_u32) {
			ei_local->rx_ring0[i].rxd_info1_u32 = 0;
#if defined (RAETH_PDMA_V2)
			ei_local->rx_ring0[i].rxd_info2_u32 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
#else
			ei_local->rx_ring0[i].rxd_info2_u32 = RX2_DMA_LS0;
#endif
		}
	}

	for (i = 0; i < NUM_TX_DESC; i++) {
		if (ei_local->tx0_free[i]) {
			ei_local->tx_ring0[i].txd_info1_u32 = 0;
#if defined (CONFIG_RAETH_SG_DMA_TX)
			ei_local->tx_ring0[i].txd_info3_u32 = 0;
			if (ei_local->tx0_free[i] != (struct sk_buff *)0xFFFFFFFF)
#endif
				dev_kfree_skb_any(ei_local->tx0_free[i]);
			ei_local->tx0_free[i] = NULL;
			ei_local->tx_ring0[i].txd_info2_u32 = TX2_DMA_DONE;
		}
	}

#if defined (CONFIG_RAETH_BQL)
	netdev_reset_queue(dev);
#endif

	ei_local->tx_free_idx = 0;
#if defined (RAETH_PDMAPTR_FROM_VAR)
	ei_local->rx_calc_idx = NUM_RX_DESC - 1;
	ei_local->tx_calc_idx = 0;
#endif

	spin_unlock_irqrestore(&ei_local->page_lock, flags);

	/* fetch pending FE counters */
	spin_lock(&ei_local->stat_lock);
	read_counters_gdma1(ei_local);
#if defined (CONFIG_PSEUDO_SUPPORT)
	read_counters_gdma2(netdev_priv(ei_local->PseudoDev));
#endif
	spin_unlock(&ei_local->stat_lock);

	module_put(THIS_MODULE);

	return 0;
}

static const struct net_device_ops ei_netdev_ops = {
	.ndo_init		= ei_init,
	.ndo_uninit		= ei_uninit,
	.ndo_open		= ei_open,
	.ndo_stop		= ei_close,
	.ndo_start_xmit		= ei_start_xmit_gmac1,
	.ndo_get_stats64	= ei_get_stats64,
	.ndo_do_ioctl		= ei_ioctl,
	.ndo_change_mtu		= ei_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
#if defined (CONFIG_RAETH_HW_VLAN_RX)
	.ndo_vlan_rx_add_vid	= ei_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= ei_vlan_rx_kill_vid,
#endif
};

/**
 * raeth_init - Module Init code
 *
 * Called by kernel to register net_device
 *
 */
int __init raeth_init(void)
{
	int ret;
	struct net_device *dev;

#if defined (CONFIG_RALINK_MT7620)
	/* MT7620 has CDM bugs at least ECO_ID=3
	  1. TX Checksum Gen raise abnormal TX flood and FrameEngine hungs
	  2. TSO stuck */
	if ((ralink_asic_rev_id & 0xf) < 5) {
#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
		hw_offload_csg = 0;
#if defined (CONFIG_RAETH_SG_DMA_TX)
		hw_offload_gso = 0;
#if defined (CONFIG_RAETH_TSO)
		hw_offload_tso = 0;
#endif
#endif
#endif
	}
#endif
	dev = alloc_etherdev(sizeof(END_DEVICE));
	if (!dev)
		return -ENOMEM;

	ether_setup(dev);
	strcpy(dev->name, DEV_NAME);

	dev->irq		= SURFBOARDINT_FE;
	dev->base_addr		= RALINK_FRAME_ENGINE_BASE;
	dev->netdev_ops		= &ei_netdev_ops;
#if defined (CONFIG_ETHTOOL)
	dev->ethtool_ops	= &ra_ethtool_ops;
#endif
	dev->watchdog_timeo	= 5*HZ;

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

#if defined (CONFIG_PSEUDO_SUPPORT)
	/* Register virtual net device (eth3) for the driver */
	ret = VirtualIF_init(dev);
	if (ret != 0) {
		unregister_netdev(dev);
		free_netdev(dev);
		return ret;
	}
#endif

	dev_raether = dev;

	debug_proc_init();

#if defined (CONFIG_RAETH_ESW_CONTROL)
	esw_ioctl_init();
#endif

	printk("Ralink APSoC Ethernet Driver %s. Rx/Tx Ring: %d/%d. Max packet size: %d.\n", RAETH_VERSION, NUM_RX_DESC, NUM_TX_DESC, MAX_RX_LENGTH);

	return 0;
}

/**
 * raeth_uninit - Module Exit code
 *
 * Cmd 'rmmod' will invode the routine to exit the module
 *
 */
void __exit raeth_exit(void)
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
