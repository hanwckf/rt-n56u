#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>
#include <linux/delay.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/sched.h>
#endif

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ioctl.h"
#include "ra_rfrw.h"
#ifdef CONFIG_RAETH_NETLINK
#include "ra_netlink.h"
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../net/nat/hw_nat/ra_nat.h"
#endif

#ifdef CONFIG_RAETH_HW_VLAN_TX
__u16 vlan_tx_idx14 = 0xE;
__u16 vlan_tx_idx15 = 0xF;
#endif

#ifdef CONFIG_VLAN_8021Q_DOUBLE_TAG
extern int vlan_double_tag;
static int vlan_offload = 0;
#endif

#ifdef CONFIG_RTL8367_IGMP_SNOOPING
#define ETH_P_REALTEK 0x8899
extern int rtl8367_cpu_port_hook(struct sk_buff *skb);
#endif

#if !defined(CONFIG_RA_NAT_NONE)
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
#include <asm/mipsregs.h>
extern int (*ra_classifier_hook_tx)(struct sk_buff *skb, unsigned long cur_cycle);
extern int (*ra_classifier_hook_rx)(struct sk_buff *skb, unsigned long cur_cycle);
#endif

#ifdef CONFIG_RAETH_READ_MAC_FROM_MTD
#ifdef RA_MTD_RW_BY_NUM
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);
#else
extern int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);
#endif
#endif

#if defined (CONFIG_RALINK_RT3052_MP2)
extern int32_t mcast_rx(struct sk_buff * skb);
extern int32_t mcast_tx(struct sk_buff * skb);
#endif

/* gmac driver feature set config */

#if defined (CONFIG_RAETH_JUMBOFRAME)
#define MAX_RX_LENGTH	4096
#else
#define MAX_RX_LENGTH	1536
#endif

#if defined (CONFIG_ETHTOOL)
#include "ra_ethtool.h"
extern struct ethtool_ops	ra_ethtool_ops;
#ifdef CONFIG_PSEUDO_SUPPORT
extern struct ethtool_ops	ra_virt_ethtool_ops;
#endif
#endif

#ifdef CONFIG_RALINK_VISTA_BASIC
static int is_switch_175c = 1;
#endif

static unsigned int eth_min_pkt_len = ETH_ZLEN;
static int eth_close = 1; /* default disable rx/tx processing while init */

struct net_device *dev_raether = NULL;


#if defined (CONFIG_GIGAPHY) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
static int isICPlusGigaPHY(int ge)
{
	u32 phy_id0 = 0, phy_id1 = 0;

#ifdef CONFIG_GE2_RGMII_AN
	if (ge == 2) {
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 2, &phy_id0)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 3, &phy_id1)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id1 = 0;
		}
	}
	else
#endif
#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
	{
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id1 = 0;
		}
	}
#endif

	if ((phy_id0 == EV_ICPLUS_PHY_ID0) && ((phy_id1 & 0xfff0) == EV_ICPLUS_PHY_ID1))
		return 1;
	return 0;
}

static int isMarvellGigaPHY(int ge)
{
	u32 phy_id0 = 0, phy_id1 = 0;

#if defined (CONFIG_GE2_RGMII_AN)
	if (ge == 2) {
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 2, &phy_id0)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 3, &phy_id1)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id1 = 0;
		}
	}
	else
#endif
#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
	{
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id1 = 0;
		}
	}
#endif
		;
	if ((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1))
		return 1;
	return 0;
}

static int isVtssGigaPHY(int ge)
{
	u32 phy_id0 = 0, phy_id1 = 0;

#if defined (CONFIG_GE2_RGMII_AN)
	if (ge == 2) {
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 2, &phy_id0)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 3, &phy_id1)) {
			printk("\n Read PhyID 1 is Fail!!\n");
			phy_id1 = 0;
		}
	}
	else
#endif
#if defined (CONFIG_GE1_RGMII_AN) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
	{
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id0 =0;
		}
		if (!mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)) {
			printk("\n Read PhyID 0 is Fail!!\n");
			phy_id1 = 0;
		}
	}
#endif
		;
	if ((phy_id0 == EV_VTSS_PHY_ID0) && (phy_id1 == EV_VTSS_PHY_ID1))
		return 1;
	return 0;
}
#endif

static void fe_reset(void)
{
	u32 val;
	val = sysRegRead(RSTCTRL);

// RT5350 need to reset ESW and FE at the same to avoid PDMA panic //	
#if defined (CONFIG_RALINK_RT5350) 
	val = val | RALINK_FE_RST | RALINK_ESW_RST ;
#else
	val = val | RALINK_FE_RST;
#endif
	sysRegWrite(RSTCTRL, val);
#if defined (CONFIG_RALINK_RT5350)
	val = val & ~(RALINK_FE_RST | RALINK_ESW_RST);
#else
	val = val & ~(RALINK_FE_RST);
#endif
	sysRegWrite(RSTCTRL, val);
}


static void fe_sw_init(void)
{
#if defined (CONFIG_GIGAPHY) || defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY)
        unsigned int regValue = 0;
#endif

	// Case1: RT288x/RT3883 GE1 + GigaPhy
#if defined (CONFIG_GE1_RGMII_AN)
	enable_auto_negotiate(1);
	if (isMarvellGigaPHY(1)) {
		printk("\n Reset MARVELL phy\n");
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 20, &regValue);
		regValue |= 1<<7; //Add delay to RX_CLK for RXD Outputs
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 20, regValue);
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 0, &regValue);
		regValue |= 1<<15; //PHY Software Reset
	 	mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 0, regValue);
	}
	if (isVtssGigaPHY(1)) {
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 31, 1);
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 28, &regValue);
		printk("Vitesse phy skew: %x --> ", regValue);
		regValue |= (0x3<<12);
		regValue &= ~(0x3<<14);
		printk("%x\n", regValue);
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 28, regValue);
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, 31, 0);
        }
#endif // CONFIG_GE1_RGMII_AN //

	// Case2: RT3883 GE2 + GigaPhy
#if defined (CONFIG_GE2_RGMII_AN)
	enable_auto_negotiate(2);
	if (isMarvellGigaPHY(2)) {
		printk("\n GMAC2 Reset MARVELL phy\n");
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 20, &regValue);
		regValue |= 1<<7; //Add delay to RX_CLK for RXD Outputs
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 20, regValue);
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 0, &regValue);
		regValue |= 1<<15; //PHY Software Reset
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 0, regValue);
	}
	if (isVtssGigaPHY(2)) {
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 31, 1);
		mii_mgr_read(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 28, &regValue);
		printk("Vitesse phy skew: %x --> ", regValue);
		regValue |= (0x3<<12);
		regValue &= ~(0x3<<14);
		printk("%x\n", regValue);
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 28, regValue);
		mii_mgr_write(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, 31, 0);
	}
#endif // CONFIG_GE2_RGMII_AN //

	// Case3: RT305x/RT335x
#if defined (CONFIG_RT_3052_ESW)
	rt305x_esw_init();
#endif

	// Case4:  RT288x/RT388x GE1 + GigaSW
#if defined (CONFIG_GE1_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
#endif

	// Case5: RT388x GE2 + GigaSW
#if defined (CONFIG_GE2_RGMII_FORCE_1000)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
#endif

	// Case6: RT288x GE1 /RT388x GE1/GE2 + (10/100 Switch or 100PHY)
#if defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_100PHY)

//set GMAC to MII or RvMII mode
#if defined (CONFIG_RALINK_RT3883)
	regValue = sysRegRead(SYSCFG1);
#if defined (CONFIG_GE1_MII_FORCE_100) || defined (CONFIG_GE1_MII_AN)
	regValue &= ~(0x3 << 12);
	regValue |= 0x1 << 12; // GE1 MII Mode
#elif defined (CONFIG_GE1_RVMII_FORCE_100)
	regValue &= ~(0x3 << 12);
	regValue |= 0x2 << 12; // GE1 RvMII Mode
#endif 

#if defined (CONFIG_GE2_MII_FORCE_100) || defined (CONFIG_GE2_MII_AN) 
	regValue &= ~(0x3 << 14);
	regValue |= 0x1 << 14; // GE2 MII Mode
#elif defined (CONFIG_GE2_RVMII_FORCE_100)
	regValue &= ~(0x3 << 14);
	regValue |= 0x2 << 14; // GE2 RvMII Mode
#endif 
	sysRegWrite(SYSCFG1, regValue);
#endif // CONFIG_RALINK_RT3883 //

#if defined (CONFIG_GE1_MII_FORCE_100)
	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_FORCE_100_FD);
#endif
#if defined (CONFIG_GE2_MII_FORCE_100)
	sysRegWrite(MDIO_CFG2, INIT_VALUE_OF_FORCE_100_FD);
#endif
	//add switch configuration here for other switch chips.
#if defined (CONFIG_GE1_MII_FORCE_100) ||  defined (CONFIG_GE2_MII_FORCE_100)
	// IC+ 175x: force IC+ switch cpu port is 100/FD
	mii_mgr_write(29, 22, 0x8420);
#endif

#if defined (CONFIG_GE1_MII_AN)
	enable_auto_negotiate(1);
#endif
#if defined (CONFIG_GE2_MII_AN)
	enable_auto_negotiate(2);
#endif
#endif
}

static void raeth_ring_free(struct net_device *dev)
{
	int i;
	END_DEVICE* ei_local = netdev_priv(dev);

	/* Clear adapter TX/RX rings */
	sysRegWrite(TX_BASE_PTR0, 0);
	sysRegWrite(TX_MAX_CNT0, 0);
	sysRegWrite(RX_BASE_PTR0, 0);
	sysRegWrite(RX_MAX_CNT0,  0);

	/* Free RX buffers */
	for (i = 0; i < NUM_RX_DESC; i++)
	{
		if (ei_local->rx0_skbuf[i]) {
			dev_kfree_skb_any(ei_local->rx0_skbuf[i]);
			ei_local->rx0_skbuf[i] = NULL;
		}
	}

	/* RX Ring */
	if (ei_local->rx_ring0) {
		dma_free_coherent(&dev->dev, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), ei_local->rx_ring0, ei_local->phy_rx_ring0);
		ei_local->rx_ring0 = NULL;
	}

	/* TX Ring */
	if (ei_local->tx_ring0) {
		dma_free_coherent(&dev->dev, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring0, ei_local->phy_tx_ring0);
		ei_local->rx_ring0 = NULL;
	}

	printk("raeth: Free TX/RX Ring Memory!\n");
}

static int raeth_ring_alloc(struct net_device *dev)
{
	int i;
	END_DEVICE* ei_local = netdev_priv(dev);
	
	ei_local->tx_ring0 = NULL;
	ei_local->rx_ring0 = NULL;
	for (i = 0; i < NUM_RX_DESC; i++)
		ei_local->rx0_skbuf[i] = NULL;

	ei_local->tx_ring0 = dma_alloc_coherent(&dev->dev, NUM_TX_DESC * sizeof(struct PDMA_txdesc), &ei_local->phy_tx_ring0, GFP_KERNEL);
	if (!ei_local->tx_ring0)
		goto err_cleanup;

	ei_local->rx_ring0 = dma_alloc_coherent(&dev->dev, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring0, GFP_KERNEL);
	if (!ei_local->rx_ring0)
		goto err_cleanup;

	/* receiving packet buffer allocation - NUM_RX_DESC x MAX_RX_LENGTH */
	for ( i = 0; i < NUM_RX_DESC; i++)
	{
#if defined (CONFIG_RAETH_SKB_RECYCLE_2K)
		ei_local->rx0_skbuf[i] = skbmgr_dev_alloc_skb2k();
#else
		ei_local->rx0_skbuf[i] = dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN);
#endif
		if (!ei_local->rx0_skbuf[i])
			goto err_cleanup;
		
		skb_reserve(ei_local->rx0_skbuf[i], NET_IP_ALIGN);
	}

	return 0;

err_cleanup:
	raeth_ring_free(dev);
	return -ENOMEM;
}

int forward_config(struct net_device *dev)
{
#if defined (CONFIG_RALINK_RT5350)
	/* RT5350: No GDMA, PSE, CDMA, PPE */
	unsigned int sdmVal;
	sdmVal = sysRegRead(SDM_CON);

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
	sdmVal |= 0x7<<16; // UDPCS, TCPCS, IPCS=1
#endif // CONFIG_RAETH_CHECKSUM_OFFLOAD //

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	sdmVal |= 0x1<<20; // TCI_81XX
#endif // CONFIG_RAETH_SPECIAL_TAG //

	sysRegWrite(SDM_CON, sdmVal);

#else //Non RT5350 chipset

	unsigned int	regVal, regCsg;

#ifdef CONFIG_PSEUDO_SUPPORT
	unsigned int	regVal2;
#endif

#ifdef CONFIG_VLAN_8021Q_DOUBLE_TAG
	vlan_offload = (vlan_double_tag) ? 0 : 1;
#endif

#if defined(CONFIG_PSEUDO_SUPPORT)
	eth_min_pkt_len = ETH_ZLEN; // pad to 60 bytes
#else
	eth_min_pkt_len = VLAN_ETH_ZLEN; // pad to 64 bytes
#endif

#ifdef CONFIG_RAETH_HW_VLAN_TX
	/* 
	 * VLAN_IDX 0 = VLAN_ID 0
	 * .........
	 * VLAN_IDX 15 = VLAN ID 15
	 *
	 */
#ifdef CONFIG_VLAN_8021Q_DOUBLE_TAG
    if (vlan_offload)
#endif
    {
	printk("raeth: hardware vlan tx offload enabled\n");
	/* frame engine will push VLAN tag regarding to VIDX feild in Tx desc. */
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xa8) = 0x00010000;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xac) = 0x00030002;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xb0) = 0x00050004;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xb4) = 0x00070006;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xb8) = 0x00090008;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xbc) = 0x000b000a;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xc0) = 0x000d000c;
	*(unsigned long *)(RALINK_FRAME_ENGINE_BASE + 0xc4) = (((__u32)vlan_tx_idx15 << 16) | ((__u32)vlan_tx_idx14));
	eth_min_pkt_len = ETH_ZLEN; // pad to 60 bytes
    }
#endif

	regVal = sysRegRead(GDMA1_FWD_CFG);
	regCsg = sysRegRead(CDMA_CSG_CFG);

#ifdef CONFIG_PSEUDO_SUPPORT
	regVal2 = sysRegRead(GDMA2_FWD_CFG);
#endif

	//set unicast/multicast/broadcast frame to cpu
	regVal &= ~0xFFFF;
	regCsg &= ~0x7;

#if defined (CONFIG_RAETH_SPECIAL_TAG)
	regVal |= (1 << 24); //GDM1_TCI_81xx
#endif

#ifdef CONFIG_RAETH_HW_VLAN_TX
#ifdef CONFIG_VLAN_8021Q_DOUBLE_TAG
    if (!vlan_offload)
	dev->features &= ~(NETIF_F_HW_VLAN_TX);
    else
#endif
	dev->features |= NETIF_F_HW_VLAN_TX;
#endif

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
	//enable ipv4 header checksum check
	regVal |= GDM1_ICS_EN;
	regCsg |= ICS_GEN_EN;

	//enable tcp checksum check
	regVal |= GDM1_TCS_EN;
	regCsg |= TCS_GEN_EN;

	//enable udp checksum check
	regVal |= GDM1_UCS_EN;
	regCsg |= UCS_GEN_EN;

#ifdef CONFIG_PSEUDO_SUPPORT
	regVal2 &= ~0xFFFF;
	regVal2 |= GDM1_ICS_EN;
	regVal2 |= GDM1_TCS_EN;
	regVal2 |= GDM1_UCS_EN;
#endif

	dev->features |= NETIF_F_IP_CSUM; /* Can checksum TCP/UDP over IPv4 */

#else // Checksum offload disabled

	//disable ipv4 header checksum check
	regVal &= ~GDM1_ICS_EN;
	regCsg &= ~ICS_GEN_EN;

	//disable tcp checksum check
	regVal &= ~GDM1_TCS_EN;
	regCsg &= ~TCS_GEN_EN;

	//disable udp checksum check
	regVal &= ~GDM1_UCS_EN;
	regCsg &= ~UCS_GEN_EN;

#ifdef CONFIG_PSEUDO_SUPPORT
	regVal2 &= ~GDM1_ICS_EN;
	regVal2 &= ~GDM1_TCS_EN;
	regVal2 &= ~GDM1_UCS_EN;
#endif

	dev->features &= ~NETIF_F_IP_CSUM; /* disable checksum TCP/UDP over IPv4 */
#endif // CONFIG_RAETH_CHECKSUM_OFFLOAD //

#ifdef CONFIG_RAETH_JUMBOFRAME
	regVal |= GDM1_JMB_EN;
	regVal &= ~0xf0000000; /* clear bit28-bit31 */
	regVal |= (((MAX_RX_LENGTH/1024)&0xf) << 28);
#ifdef CONFIG_PSEUDO_SUPPORT
	regVal2 |= GDM1_JMB_EN;
	regVal2 &= ~0xf0000000; /* clear bit28-bit31 */
	regVal2 |= (((MAX_RX_LENGTH/1024)&0xf) << 28);
#endif
#endif

	sysRegWrite(GDMA1_FWD_CFG, regVal);
	sysRegWrite(CDMA_CSG_CFG, regCsg);
#ifdef CONFIG_PSEUDO_SUPPORT
	sysRegWrite(GDMA2_FWD_CFG, regVal2);
#endif

/*
 * 	PSE_FQ_CFG register definition -
 *
 * 	Define max free queue page count in PSE. (31:24)
 *	RT2883/RT3883 - 0xff908000 (255 pages)
 *	RT3052 - 0x80504000 (128 pages)
 *	RT2880 - 0x80504000 (128 pages)
 *
 * 	In each page, there are 128 bytes in each page.
 *
 *	23:16 - free queue flow control release threshold
 *	15:8  - free queue flow control assertion threshold
 *	7:0   - free queue empty threshold
 *
 *	The register affects QOS correctness in frame engine!
 */

#if defined(CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883)
	sysRegWrite(PSE_FQ_CFG, cpu_to_le32(INIT_VALUE_OF_RT2883_PSE_FQ_CFG));
#elif defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350)
        /*use default value*/
#else
	sysRegWrite(PSE_FQ_CFG, cpu_to_le32(INIT_VALUE_OF_PSE_FQFC_CFG));
#endif

	/*
	 *FE_RST_GLO register definition -
	 *Bit 0: PSE Rest
	 *Reset PSE after re-programming PSE_FQ_CFG.
	 */
	regVal = 0x1;
	sysRegWrite(FE_RST_GL, regVal);
	sysRegWrite(FE_RST_GL, 0);	// update for RSTCTL issue

	sysRegRead(CDMA_CSG_CFG);
	sysRegRead(GDMA1_FWD_CFG);
#ifdef CONFIG_PSEUDO_SUPPORT
	sysRegRead(GDMA2_FWD_CFG);
#endif
#endif
	return 1;
}

static void fe_pdma_init(struct net_device *dev)
{
	int i;
	unsigned int regVal;
	END_DEVICE* ei_local = netdev_priv(dev);

	for (i=0; i<10; i++)
	{
		regVal = sysRegRead(PDMA_GLO_CFG);
		if((regVal & RX_DMA_BUSY))
		{
			msleep(10);
			continue;
		}
		if((regVal & TX_DMA_BUSY))
		{
			msleep(10);
			continue;
		}
		break;
	}

	/* Initial TX Ring 0 */
	ei_local->tx_free_idx =0;
	for (i=0; i < NUM_TX_DESC; i++) {
		ei_local->tx0_free[i] = NULL;
		memset(&ei_local->tx_ring0[i], 0, sizeof(struct PDMA_txdesc));
		ei_local->tx_ring0[i].txd_info2.LS0_bit = 1;
		ei_local->tx_ring0[i].txd_info2.DDONE_bit = 1;
		ei_local->tx_ring0[i].txd_info4.QN = 3;
	}

	/* Initial RX Ring 0 */
	for (i = 0; i < NUM_RX_DESC; i++) {
		memset(&ei_local->rx_ring0[i], 0, sizeof(struct PDMA_rxdesc));
		ei_local->rx_ring0[i].rxd_info2.DDONE_bit = 0;
		ei_local->rx_ring0[i].rxd_info2.LS0 = 1;
		ei_local->rx_ring0[i].rxd_info1.PDP0 = dma_map_single(&dev->dev, ei_local->rx0_skbuf[i]->data, MAX_RX_LENGTH, DMA_FROM_DEVICE);
	}

	/*clear PDMA */
	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= 0x000000FF;
	sysRegWrite(PDMA_GLO_CFG, regVal);
	regVal=sysRegRead(PDMA_GLO_CFG);

	/* Tell the adapter where the TX/RX rings are located. */
	sysRegWrite(TX_BASE_PTR0, phys_to_bus((u32) ei_local->phy_tx_ring0));
	sysRegWrite(TX_MAX_CNT0, cpu_to_le32((u32) NUM_TX_DESC));
	sysRegWrite(TX_CTX_IDX0, 0);
	sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX0);

	sysRegWrite(RX_BASE_PTR0, phys_to_bus((u32) ei_local->phy_rx_ring0));
	sysRegWrite(RX_MAX_CNT0,  cpu_to_le32((u32) NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX0, cpu_to_le32((u32) (NUM_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX0);

	/* only the following chipset need to set it */
#if defined (CONFIG_RALINK_RT2880) || defined(CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
	//set 1us timer count in unit of clock cycle
	regVal = sysRegRead(FE_GLO_CFG);
	regVal &= ~(0xff << 8); //clear bit8-bit15
	regVal |= (((get_surfboard_sysclk()/1000000)) << 8);
	sysRegWrite(FE_GLO_CFG, regVal);
#endif
}

static void fe_pdma_start(void)
{
	sysRegWrite(PDMA_GLO_CFG, (TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN | PDMA_BT_SIZE_4DWORDS));
}

static void fe_pdma_stop(void)
{
	unsigned int regValue;

	regValue = sysRegRead(PDMA_GLO_CFG);
	regValue &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regValue);
}

static void inc_rx_drop(END_DEVICE *ei_local, int gmac_no)
{
#ifdef CONFIG_PSEUDO_SUPPORT
	PSEUDO_ADAPTER *pAd;

	if (gmac_no == 2) {
		if (ei_local->PseudoDev != NULL) {
			pAd = netdev_priv(ei_local->PseudoDev);
			pAd->stat.rx_dropped++;
		}
	} else
#endif
		ei_local->stat.rx_dropped++;
}

#ifdef CONFIG_RAETH_NAPI
static int raeth_recv(struct net_device* dev, int *work_done, int work_to_do)
#else
static int raeth_recv(struct net_device* dev)
#endif
{
	struct sk_buff *new_skb, *rx_skb;
	struct PDMA_rxdesc *rx_ring;
	unsigned int length;
	dma_addr_t dma_handle;
	int gmac_no;
	int rx_dma_owner_idx;
#ifndef CONFIG_RAETH_NAPI
	int RxProcessed = 0;
#endif
	int bReschedule = 0;
	END_DEVICE* ei_local = netdev_priv(dev);
#if defined (CONFIG_RAETH_SPECIAL_TAG)
	struct vlan_ethhdr *veth=NULL;
#endif
#ifdef CONFIG_PSEUDO_SUPPORT
	PSEUDO_ADAPTER *pAd;
#endif

	rx_dma_owner_idx = (sysRegRead(RX_CALC_IDX0) + 1) % NUM_RX_DESC;

	for ( ; ; ) {
#ifdef CONFIG_RAETH_NAPI
		if(*work_done >= work_to_do)
			break;
		(*work_done)++;
#else
		if (RxProcessed++ > NUM_RX_MAX_PROCESS)
		{
			// need to reschedule rx handle
			bReschedule = 1;
			break;
		}
#endif
		rx_ring = &ei_local->rx_ring0[rx_dma_owner_idx];
		if (!rx_ring->rxd_info2.DDONE_bit) {
			break;
		}

		rx_skb = ei_local->rx0_skbuf[rx_dma_owner_idx];
		length = rx_ring->rxd_info2.PLEN0;
		gmac_no = rx_ring->rxd_info4.SP;

		/* We have to check the free memory size is big enough
		 * before pass the packet to cpu*/
#if defined (CONFIG_RAETH_SKB_RECYCLE_2K)
		new_skb = skbmgr_dev_alloc_skb2k();
#else
		new_skb = __dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN, GFP_ATOMIC);
#endif
		if (unlikely(new_skb == NULL))
		{
			rx_ring->rxd_info2.DDONE_bit = 0;
			sysRegWrite(RX_CALC_IDX0, rx_dma_owner_idx);
			inc_rx_drop(ei_local, gmac_no);
			bReschedule = 1;
			if (net_ratelimit())
				printk(KERN_ERR "raeth: Failed to alloc new RX skb! (GMAC: %d)\n", gmac_no);
			break;
		}
		skb_reserve(new_skb, NET_IP_ALIGN);
		
		/* try map buffer to DMA */
		dma_handle = dma_map_single(&dev->dev, new_skb->data, MAX_RX_LENGTH, DMA_FROM_DEVICE);
		if (dma_mapping_error(&dev->dev, dma_handle)) {
			rx_ring->rxd_info2.DDONE_bit = 0;
			sysRegWrite(RX_CALC_IDX0, rx_dma_owner_idx);
			kfree_skb(new_skb);
			inc_rx_drop(ei_local, gmac_no);
			bReschedule = 1;
			if (net_ratelimit())
				printk(KERN_ERR "raeth: Failed to map RX DMA! (GMAC: %d)\n", gmac_no);
			break;
		}

		/* unmap filled buffer from ring */
		dma_unmap_single(&dev->dev, rx_ring->rxd_info1.PDP0, MAX_RX_LENGTH, DMA_FROM_DEVICE);

		/* map new buffer to ring */
		rx_ring->rxd_info1.PDP0 = (unsigned long)dma_handle;
		rx_ring->rxd_info2.DDONE_bit = 0;

		/* skb processing */
		skb_put(rx_skb, length);

#ifdef CONFIG_PSEUDO_SUPPORT
		if(gmac_no == 2) {
			if(ei_local->PseudoDev) {
				rx_skb->protocol = eth_type_trans(rx_skb, ei_local->PseudoDev);
			}else {
				printk(KERN_ERR "raeth: PseudoDev is still not initialize but receive packet from GMAC2\n");
			}
		}else{
			rx_skb->protocol = eth_type_trans(rx_skb, dev);
		}
#else
		rx_skb->protocol = eth_type_trans(rx_skb, dev);
#endif

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
		FOE_MAGIC_TAG(rx_skb) = FOE_MAGIC_GE;
		*(uint32_t *)(FOE_INFO_START_ADDR(rx_skb)+2) = *(uint32_t *)&rx_ring->rxd_info4;
		FOE_ALG(rx_skb) = 0;
#endif

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
		if(rx_ring->rxd_info4.IPFVLD_bit) {
			rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
		}else {
			rx_skb->ip_summed = CHECKSUM_NONE;
		}
#else
		rx_skb->ip_summed = CHECKSUM_NONE;
#endif

#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)
		if(ra_classifier_hook_rx!= NULL)
		{
#if defined(CONFIG_RALINK_EXTERNAL_TIMER)
			ra_classifier_hook_rx(rx_skb, (*((volatile u32 *)(0xB0000D08))&0x0FFFF));
#else			
			ra_classifier_hook_rx(rx_skb, read_c0_count());
#endif
		}
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RTL8367_IGMP_SNOOPING)
		if (rx_skb->protocol == htons(ETH_P_REALTEK) ) {
			rtl8367_cpu_port_hook(rx_skb);
		}
#endif

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		// port0: 0x8100 => 0x8100 0001
		// port1: 0x8101 => 0x8100 0002
		// port2: 0x8102 => 0x8100 0003
		// port3: 0x8103 => 0x8100 0004
		// port4: 0x8104 => 0x8100 0005
		// port5: 0x8105 => 0x8100 0006
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
		veth = (struct vlan_ethhdr *)(rx_skb->mac_header);
#else
		veth = (struct vlan_ethhdr *)(rx_skb->mac.raw);
#endif
		if((veth->h_vlan_proto & 0xFF) == 0x81) {
			veth->h_vlan_TCI = htons( (((veth->h_vlan_proto >> 8) & 0xF) + 1) );
			rx_skb->protocol = veth->h_vlan_proto = htons(ETH_P_8021Q);
		}
#endif

/* ra_sw_nat_hook_rx return 1 --> continue
 * ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
 */
#if !defined(CONFIG_RA_NAT_NONE)
		if((ra_sw_nat_hook_rx == NULL) || 
		    (ra_sw_nat_hook_rx!= NULL && ra_sw_nat_hook_rx(rx_skb)))
#endif
		{
#if defined (CONFIG_RALINK_RT3052_MP2)
			if(mcast_rx(rx_skb)==0) {
				kfree_skb(rx_skb);
			}else
#endif
#ifdef CONFIG_RAETH_NAPI
			netif_receive_skb(rx_skb);
#else
			netif_rx(rx_skb);
#endif
		}
		
		ei_local->rx0_skbuf[rx_dma_owner_idx] = new_skb;
		
		/* Move point to next RXD which wants to alloc */
		sysRegWrite(RX_CALC_IDX0, rx_dma_owner_idx);
		
		/* Update to Next packet point that was received. */
		rx_dma_owner_idx = (rx_dma_owner_idx + 1) % NUM_RX_DESC;
		
#ifdef CONFIG_PSEUDO_SUPPORT
		if (gmac_no == 2) {
			if (ei_local->PseudoDev != NULL) {
				pAd = netdev_priv(ei_local->PseudoDev);
				pAd->stat.rx_packets++;
				pAd->stat.rx_bytes += length;
			}
		} else
#endif
		{
			ei_local->stat.rx_packets++;
			ei_local->stat.rx_bytes += length;
		}
	}	/* for */

	return bReschedule;
}


#if defined (CONFIG_RT_3052_ESW)
void kill_sig_workq(struct work_struct *work)
{
	struct file *fp;
	char pid[8];
	struct task_struct *p = NULL;

	//read udhcpc pid from file, and send signal USR2,USR1 to get a new IP
	fp = filp_open("/var/run/udhcpc.pid", O_RDONLY, 0);
	if (IS_ERR(fp))
	    return;

	if (fp->f_op && fp->f_op->read) {
	    if (fp->f_op->read(fp, pid, 8, &fp->f_pos) > 0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
		p = pid_task(find_get_pid(simple_strtoul(pid, NULL, 10)),  PIDTYPE_PID);
#else
		p = find_task_by_pid(simple_strtoul(pid, NULL, 10));
#endif

		if (NULL != p) {
		    send_sig(SIGUSR2, p, 0);
		    send_sig(SIGUSR1, p, 0);
		}
	    }
	}
	filp_close(fp, NULL);

}
#endif

///////////////////////////////////////////////////////////////////
/////
///// ei_receive - process the next incoming packet
/////
///// Handle one incoming packet.  The packet is checked for errors and sent
///// to the upper layer.
/////
///// RETURNS: OK on success or ERROR.
///////////////////////////////////////////////////////////////////

#ifndef CONFIG_RAETH_NAPI
#if defined WORKQUEUE_BH
void ei_receive_workq(struct work_struct *work)
#else
void ei_receive(unsigned long ptr)
#endif
{
#if defined WORKQUEUE_BH
	struct net_device *dev = dev_raether;
#else
	struct net_device *dev = (struct net_device *)ptr;
#endif
	END_DEVICE *ei_local = netdev_priv(dev);

	if(eth_close) /* protect eth while init or reinit */
		return;

	if (raeth_recv(dev))
	{
#ifdef WORKQUEUE_BH
		schedule_work(&ei_local->rx_wq);
#else
		tasklet_schedule(&ei_local->rx_tasklet);
#endif
	}else{
		unsigned long reg_int_mask = sysRegRead(FE_INT_ENABLE);
		sysRegWrite(FE_INT_ENABLE, reg_int_mask | RX_DLY_INT);
	}
}

#else

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static int raeth_clean(struct napi_struct *napi, int budget)
#else
static int raeth_clean(struct net_device *netdev, int *budget)
#endif
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	struct net_device *netdev = napi->dev;
	int work_to_do = budget;
#else
	int work_to_do = min(*budget, netdev->quota);
#endif
	END_DEVICE *ei_local = netdev_priv(netdev);
	unsigned long reg_int_mask;
	int work_done = 0;

	raeth_recv(netdev, &work_done, work_to_do);

	/* this could control when to re-enable interrupt, 0-> mean never enable interrupt*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	*budget -= work_done;
	netdev->quota -= work_done;
#endif
        /* if no Tx and not enough Rx work done, exit the polling mode */
	if(( (work_done < work_to_do)) || !netif_running(netdev)) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
		napi_complete(&ei_local->napi);
#else
		netif_rx_complete(netdev);
#endif
		atomic_dec_and_test(&ei_local->irq_sem);
		sysRegWrite(FE_INT_STATUS, RX_DONE_INT0);	// ack all fe RX interrupts
		reg_int_mask=sysRegRead(FE_INT_ENABLE);
		sysRegWrite(FE_INT_ENABLE, reg_int_mask | RX_DONE_INT0);
		return 0;
	}

	return 1;
}
#endif


/**
 * ei_interrupt - handle controler interrupt
 *
 * This routine is called at interrupt level in response to an interrupt from
 * the controller.
 *
 * RETURNS: N/A.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
static irqreturn_t ei_interrupt(int irq, void *dev_id)
#else
static irqreturn_t ei_interrupt(int irq, void *dev_id, struct pt_regs * regs)
#endif
{
	unsigned long reg_int_val;
	unsigned long reg_int_mask;
	struct net_device *dev = (struct net_device *) dev_id;

	END_DEVICE *ei_local = netdev_priv(dev);
	if (!dev)
	{
		return IRQ_NONE;
	}

	reg_int_val = sysRegRead(FE_INT_STATUS);
	if (!reg_int_val)
		return IRQ_NONE;

	sysRegWrite(FE_INT_STATUS, reg_int_val);

	if (reg_int_val & TX_DLY_INT)
		tasklet_hi_schedule(&ei_local->tx_tasklet);

#if defined (CONFIG_RAETH_NAPI)
	if (reg_int_val & RX_DONE_INT0) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
		if(napi_schedule_prep(&ei_local->napi)) {
#else
		if(netif_rx_schedule_prep(dev)) {
#endif
			atomic_inc(&ei_local->irq_sem);
			reg_int_mask = sysRegRead(FE_INT_ENABLE);
			sysRegWrite(FE_INT_ENABLE, reg_int_mask & ~(RX_DONE_INT0));
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
			__napi_schedule(&ei_local->napi);
#else
			__netif_rx_schedule(dev);
#endif
		}
	}
#else
	if (reg_int_val & RX_DLY_INT) {
		reg_int_mask = sysRegRead(FE_INT_ENABLE);
		sysRegWrite(FE_INT_ENABLE, reg_int_mask & ~(RX_DLY_INT));
#ifdef WORKQUEUE_BH
		schedule_work(&ei_local->rx_wq);
#else
		tasklet_schedule(&ei_local->rx_tasklet);
#endif
	}
#endif

	return IRQ_HANDLED;
}

#if defined (CONFIG_RT_3052_ESW)
static irqreturn_t esw_interrupt(int irq, void *dev_id)
{
	unsigned long flags;
	unsigned long reg_int_val;
	static unsigned long stat;
	unsigned long stat_curr;
	
	struct net_device *dev = (struct net_device *) dev_id;
	END_DEVICE *ei_local = netdev_priv(dev);

	spin_lock_irqsave(&(ei_local->page_lock), flags);
	reg_int_val = (*((volatile u32 *)(ESW_ISR))); //Interrupt Status Register

	if (reg_int_val & PORT_ST_CHG) {
		printk("RT305x_ESW: Link Status Changed\n");

		stat_curr = *((volatile u32 *)(RALINK_ETH_SW_BASE+0x80));
#ifdef CONFIG_WAN_AT_P0
		//link down --> link up : send signal to user application
		//link up --> link down : ignore
		if ((stat & (1<<25)) || !(stat_curr & (1<<25)))
#else
		if ((stat & (1<<29)) || !(stat_curr & (1<<29)))
#endif
			goto out;

		schedule_work(&ei_local->kill_sig_wq);
out:
		stat = stat_curr;
	}

	sysRegWrite(ESW_ISR, reg_int_val);

	spin_unlock_irqrestore(&(ei_local->page_lock), flags);
	return IRQ_HANDLED;
}
#endif


static void inc_tx_drop(END_DEVICE *ei_local, int gmac_no)
{
#ifdef CONFIG_PSEUDO_SUPPORT
	PSEUDO_ADAPTER *pAd;

	if (gmac_no == 2) {
		if (ei_local->PseudoDev != NULL) {
			pAd = netdev_priv(ei_local->PseudoDev);
			pAd->stat.tx_dropped++;
		}
	} else
#endif
		ei_local->stat.tx_dropped++;
}

inline int ei_start_xmit(struct sk_buff* skb, struct net_device *dev, int gmac_no)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	struct PDMA_txdesc *tx_ring;
	unsigned int tx_cpu_owner_idx;
	unsigned int tx_cpu_owner_idx_next;
	dma_addr_t dma_handle;
#ifdef CONFIG_PSEUDO_SUPPORT
	PSEUDO_ADAPTER *pAd;
#endif
#ifdef CONFIG_RALINK_VISTA_BASIC
	struct vlan_ethhdr *veth;
#endif

	spin_lock(&ei_local->page_lock);

	/* protect eth while init or reinit */
	if (eth_close) {
		dev_kfree_skb(skb);
		spin_unlock(&ei_local->page_lock);
		return NETDEV_TX_OK;
	}

#if !defined(CONFIG_RA_NAT_NONE)
	if(ra_sw_nat_hook_tx!= NULL)
	{
		if(ra_sw_nat_hook_tx(skb, gmac_no)==0){
			dev_kfree_skb(skb);
			spin_unlock(&ei_local->page_lock);
			return NETDEV_TX_OK;
		}
	}
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
	if(ra_classifier_hook_tx!= NULL)
	{
#if defined(CONFIG_RALINK_EXTERNAL_TIMER)
		ra_classifier_hook_tx(skb, (*((volatile u32 *)(0xB0000D08))&0x0FFFF));
#else
		ra_classifier_hook_tx(skb, read_c0_count());
#endif
	}
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RALINK_RT3052_MP2)
	mcast_tx(skb);
#endif

	if (skb->len < eth_min_pkt_len) {
		if (skb_padto(skb, eth_min_pkt_len)) {
			if (net_ratelimit())
				printk(KERN_ERR "raeth: skb_padto failed\n");
			inc_tx_drop(ei_local, gmac_no);
			spin_unlock(&ei_local->page_lock);
			return NETDEV_TX_OK;
		}
		skb_put(skb, eth_min_pkt_len - skb->len);
	}

#ifdef CONFIG_RALINK_VISTA_BASIC
	veth = (struct vlan_ethhdr *)(skb->data);
	if (is_switch_175c && veth->h_vlan_proto == __constant_htons(ETH_P_8021Q)) {
		if ((veth->h_vlan_TCI & __constant_htons(VLAN_VID_MASK)) == 0) {
			veth->h_vlan_TCI |= htons(VLAN_DEV_INFO(dev)->vlan_id);
		}
	}
#endif
	/* try map buffer to DMA */
	dma_handle = dma_map_single(&dev->dev, skb->data, skb->len, DMA_TO_DEVICE);
	if (dma_mapping_error(&dev->dev, dma_handle)) {
		dev_kfree_skb(skb);
		inc_tx_drop(ei_local, gmac_no);
		if (net_ratelimit())
			printk(KERN_ERR "raeth: Failed to map TX DMA! (GMAC: %d)\n", gmac_no);
		spin_unlock(&ei_local->page_lock);
		return NETDEV_TX_OK;
	}

	tx_cpu_owner_idx = sysRegRead(TX_CTX_IDX0);
	tx_cpu_owner_idx_next = (tx_cpu_owner_idx + 1) % NUM_TX_DESC;

	if (ei_local->tx0_free[tx_cpu_owner_idx] ||
	    ei_local->tx0_free[tx_cpu_owner_idx_next] ||
	   !ei_local->tx_ring0[tx_cpu_owner_idx].txd_info2.DDONE_bit ||
	   !ei_local->tx_ring0[tx_cpu_owner_idx_next].txd_info2.DDONE_bit)
	{
		dma_unmap_single(&dev->dev, dma_handle, skb->len, DMA_TO_DEVICE);
		netif_stop_queue(dev);
#ifdef CONFIG_PSEUDO_SUPPORT
		if (ei_local->PseudoDev)
			netif_stop_queue(ei_local->PseudoDev);
#endif
		inc_tx_drop(ei_local, gmac_no);
		if (net_ratelimit())
			printk("raeth: tx_ring full! (GMAC: %d)\n", gmac_no);
		spin_unlock(&ei_local->page_lock);
		return NETDEV_TX_BUSY;
	}

	ei_local->tx0_free[tx_cpu_owner_idx] = skb;

	tx_ring = &ei_local->tx_ring0[tx_cpu_owner_idx];

	tx_ring->txd_info1.SDP0 = (unsigned int)dma_handle;
	tx_ring->txd_info2.SDL0 = skb->len;

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if(FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
		tx_ring->txd_info4.PN = 6; /* PPE */
		gmac_no = 0;
	}
	else
#endif
	{
		tx_ring->txd_info4.PN = gmac_no;
	}

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD) && ! defined(CONFIG_RALINK_RT5350)
	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		tx_ring->txd_info4.TUI_CO = 7;
	}
	else
	{
		tx_ring->txd_info4.TUI_CO = 0;
	}
#endif

#ifdef CONFIG_RAETH_HW_VLAN_TX
#ifdef CONFIG_VLAN_8021Q_DOUBLE_TAG
	if (vlan_offload)
#endif
	{
		if(vlan_tx_tag_present(skb)) {
			__u16 vlan_vid = vlan_tx_tag_get(skb) & VLAN_VID_MASK;
			if (vlan_vid == vlan_tx_idx14)
				tx_ring->txd_info4.VPRI_VIDX = (0x8E | ((vlan_tx_tag_get(skb) & VLAN_PRIO_MASK) >> 9));
			else
			if (vlan_vid == vlan_tx_idx15)
				tx_ring->txd_info4.VPRI_VIDX = (0x8F | ((vlan_tx_tag_get(skb) & VLAN_PRIO_MASK) >> 9));
			else
				tx_ring->txd_info4.VPRI_VIDX = (0x80 | ((vlan_tx_tag_get(skb) & VLAN_PRIO_MASK) >> 9) | (vlan_tx_tag_get(skb) & 0xF));
		} else {
			tx_ring->txd_info4.VPRI_VIDX = 0;
		}
	}
#endif
	tx_ring->txd_info2.DDONE_bit = 0;

	sysRegWrite(TX_CTX_IDX0, tx_cpu_owner_idx_next);

	if (ei_local->tx0_free[((tx_cpu_owner_idx_next + 1) % NUM_TX_DESC)]) {
		netif_stop_queue(dev);
#ifdef CONFIG_PSEUDO_SUPPORT
		netif_stop_queue(ei_local->PseudoDev);
#endif
#ifdef RAETH_DEBUG
		if (net_ratelimit())
			printk("raeth: tx_ring full! (GMAC: %d)\n", gmac_no);
#endif
	}

	spin_unlock(&ei_local->page_lock);

	if (gmac_no) {
#ifdef CONFIG_PSEUDO_SUPPORT
		if (gmac_no == 2) {
			if (ei_local->PseudoDev != NULL) {
				pAd = netdev_priv(ei_local->PseudoDev);
				pAd->stat.tx_packets++;
				pAd->stat.tx_bytes += skb->len;
			}
		} else
#endif
		{
			ei_local->stat.tx_packets++;
			ei_local->stat.tx_bytes += skb->len;
		}
	}

	return NETDEV_TX_OK;
}

void ei_xmit_housekeeping(unsigned long ptr)
{
	struct net_device *dev = (struct net_device *)ptr;
	END_DEVICE *ei_local = netdev_priv(dev);
	struct PDMA_txdesc *tx_ring;
	struct sk_buff *tx_skb;
	int released = 0;

	spin_lock(&ei_local->page_lock);

	for (;;) {
		tx_ring = &ei_local->tx_ring0[ei_local->tx_free_idx];
		tx_skb = ei_local->tx0_free[ei_local->tx_free_idx];
		if (!tx_skb || !tx_ring->txd_info2.DDONE_bit)
			break;
		
		ei_local->tx0_free[ei_local->tx_free_idx] = NULL;
		dma_unmap_single(&dev->dev, tx_ring->txd_info1.SDP0, tx_skb->len, DMA_TO_DEVICE);
		tx_ring->txd_info1.SDP0 = 0;
		tx_ring->txd_info2.SDL0 = 0;
		dev_kfree_skb(tx_skb);
		ei_local->tx_free_idx = (ei_local->tx_free_idx + 1) % NUM_TX_DESC;
		released++;
	}

	if (released) {
		if (dev->flags & IFF_UP) {
			if (netif_queue_stopped(dev))
				netif_wake_queue(dev);
		}
#ifdef CONFIG_PSEUDO_SUPPORT
		if (ei_local->PseudoDev->flags & IFF_UP) {
			if (netif_queue_stopped(ei_local->PseudoDev))
				netif_wake_queue(ei_local->PseudoDev);
		}
#endif
	}

	spin_unlock(&ei_local->page_lock);
}


#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
void dump_phy_reg(int port_no, int from, int to, int is_local)
{
        u32 i=0;
        u32 temp=0;

        if(is_local==0) {
            printk("Global Register\n");
            printk("===============");
            mii_mgr_write(0, 31, 0); //select global register
            for(i=from;i<=to;i++) {
                if(i%8==0) {
                    printk("\n");
                }
                mii_mgr_read(port_no,i, &temp);
                printk("%02d: %04X ",i, temp);
            }
        } else {
            mii_mgr_write(0, 31, 0x8000); //select local register
                printk("\n\nLocal Register Port %d\n",port_no);
                printk("===============");
                for(i=from;i<=to;i++) {
                    if(i%8==0) {
                        printk("\n");
                    }
                    mii_mgr_read(port_no,i, &temp);
                    printk("%02d: %04X ",i, temp);
                }
        }
        printk("\n");
}
#else
void dump_phy_reg(int port_no, int from, int to, int is_local, int page_no)
{

        u32 i=0;
        u32 temp=0;
        u32 r31=0;


        if(is_local==0) {

            printk("\n\nGlobal Register Page %d\n",page_no);
            printk("===============");
            r31 |= 0 << 15; //global
            r31 |= ((page_no&0x7) << 12); //page no
            mii_mgr_write(1, 31, r31); //select global page x
            for(i=16;i<32;i++) {
                if(i%8==0) {
                    printk("\n");
                }
                mii_mgr_read(port_no,i, &temp);
                printk("%02d: %04X ",i, temp);
            }
        }else {
            printk("\n\nLocal Register Port %d Page %d\n",port_no, page_no);
            printk("===============");
            r31 |= 1 << 15; //local
            r31 |= ((page_no&0x7) << 12); //page no
            mii_mgr_write(1, 31, r31); //select local page x
            for(i=16;i<32;i++) {
                if(i%8==0) {
                    printk("\n");
                }
                mii_mgr_read(port_no,i, &temp);
                printk("%02d: %04X ",i, temp);
            }
        }
        printk("\n");
}

#endif

int ei_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
#if defined(CONFIG_RT_3052_ESW)
	esw_reg reg;
#endif
#if defined(CONFIG_RALINK_RT3352) || defined(CONFIG_RALINK_RT5350)
        esw_rate ratelimit;
#endif
#if defined(CONFIG_RT_3052_ESW)
	unsigned int offset = 0;
	unsigned int value = 0;
#endif

	ra_mii_ioctl_data mii;
	switch (cmd) {
		case RAETH_MII_READ:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read(mii.phy_id, mii.reg_num, &mii.val_out);
			//printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;

		case RAETH_MII_WRITE:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			//printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_in);
			mii_mgr_write(mii.phy_id, mii.reg_num, mii.val_in);
			break;
#if defined(CONFIG_RT_3052_ESW)
#define _ESW_REG(x)	(*((volatile u32 *)(RALINK_ETH_SW_BASE + x)))
		case RAETH_ESW_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			reg.val = _ESW_REG(reg.off);
			//printk("read reg off:%x val:%x\n", reg.off, reg.val);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_ESW_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			_ESW_REG(reg.off) = reg.val;
			//printk("write reg off:%x val:%x\n", reg.off, reg.val);
			break;
		case RAETH_ESW_PHY_DUMP:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
			if (reg.val ==32 ) {//dump all phy register
			    /* Global Register 0~31
			     * Local Register 0~31
			     */
			    dump_phy_reg(0, 0, 31, 0); //dump global register
			    for(offset=0;offset<5;offset++) {
				dump_phy_reg(offset, 0, 31, 1); //dump local register
			    }
			} else {
			    dump_phy_reg(reg.val, 0, 31, 0); //dump global register
			    dump_phy_reg(reg.val, 0, 31, 1); //dump local register
			}
#endif
			break;

#if defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
#define _ESW_REG(x)	(*((volatile u32 *)(RALINK_ETH_SW_BASE + x)))
		case RAETH_ESW_INGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			offset = 0x11c + (4 * (ratelimit.port / 2));
                        value = _ESW_REG(offset);

			if((ratelimit.port % 2) == 0)
			{
				value &= 0xffff0000;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 14);
					value |= (0x07 << 10);
					value |= ratelimit.bw;
				}
			}
			else if((ratelimit.port % 2) == 1)
			{
				value &= 0x0000ffff;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 30);
					value |= (0x07 << 26);
					value |= (ratelimit.bw << 16);
				}
			}
			printk("offset = 0x%4x value=0x%x\n\r", offset, value);
			_ESW_REG(offset) = value;
			break;

		case RAETH_ESW_EGRESS_RATE:
			copy_from_user(&ratelimit, ifr->ifr_data, sizeof(ratelimit));
			offset = 0x140 + (4 * (ratelimit.port / 2));
                        value = _ESW_REG(offset);

			if((ratelimit.port % 2) == 0)
			{
				value &= 0xffff0000;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 12);
					value |= (0x03 << 10);
					value |= ratelimit.bw;
				}
			}
			else if((ratelimit.port % 2) == 1)
			{
				value &= 0x0000ffff;
				if(ratelimit.on_off == 1)
				{
					value |= (ratelimit.on_off << 28);
					value |= (0x03 << 26);
					value |= (ratelimit.bw << 16);
				}
			}
			printk("offset = 0x%4x value=0x%x\n\r", offset, value);
			_ESW_REG(offset) = value;
			break;
#endif
#endif
		default:
			return -EOPNOTSUPP;

	}

	return 0;
}

/*
 * Set new MTU size
 * Change the mtu of Raeth Ethernet Device
 */
static int ei_change_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	END_DEVICE *ei_local = netdev_priv(dev);  // get priv ei_local pointer from net_dev structure

	if ( ei_local == NULL ) {
		printk(KERN_EMERG "%s: ei_change_mtu passed a non-existent private pointer from net_dev!\n", dev->name);
		return -ENXIO;
	}

	if ( (new_mtu > MAX_RX_LENGTH) || (new_mtu < 64)) {
		return -EINVAL;
	}

#ifndef CONFIG_RAETH_JUMBOFRAME
	if ( new_mtu > 1500 ) {
		return -EINVAL;
	}
#endif

	spin_lock_irqsave(&ei_local->page_lock, flags);
	dev->mtu = new_mtu;
	spin_unlock_irqrestore(&ei_local->page_lock, flags);

	return 0;
}

#ifdef CONFIG_PSEUDO_SUPPORT
static int VirtualIF_set_mac_addr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	if(netif_running(dev))
		return -EBUSY;

	ra2880Mac2AddressSet(addr->sa_data);
	return 0;
}

int VirtualIF_ioctl(struct net_device * net_dev,
		    struct ifreq * ifr, int cmd)
{
	ra_mii_ioctl_data mii;

	switch (cmd) {
		case RAETH_MII_READ:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read(mii.phy_id, mii.reg_num, &mii.val_out);
			//printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;

		case RAETH_MII_WRITE:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			//printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_in);
			mii_mgr_write(mii.phy_id, mii.reg_num, mii.val_in);
			break;
		default:
			return -EOPNOTSUPP;
	}

	return 0;
}

struct net_device_stats *VirtualIF_get_stats(struct net_device *dev)
{
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);
	return &pPseudoAd->stat;
}

int VirtualIF_open(struct net_device * dev)
{
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);
	printk("%s: ===> VirtualIF_open\n", dev->name);
	netif_start_queue(pPseudoAd->PseudoDev);
	return 0;
}

int VirtualIF_close(struct net_device * dev)
{
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);
	printk("%s: ===> VirtualIF_close\n", dev->name);
	netif_stop_queue(pPseudoAd->PseudoDev);
	return 0;
}

int VirtualIF_start_xmit(struct sk_buff *skb, struct net_device * dev)
{
	END_DEVICE *ei_local;
	PSEUDO_ADAPTER *pPseudoAd = netdev_priv(dev);

	ei_local = netdev_priv(dev);
	if (!(pPseudoAd->RaethDev->flags & IFF_UP)) {
		dev_kfree_skb(skb);
		return 0;
	}

	skb->dev = pPseudoAd->RaethDev;

	return ei_start_xmit(skb, pPseudoAd->RaethDev, 2);
}

void VirtualIF_reset_statistics(PSEUDO_ADAPTER* pAd)
{
	pAd->stat.tx_packets	= 0;
	pAd->stat.tx_bytes 	= 0;
	pAd->stat.tx_dropped 	= 0;
	pAd->stat.tx_errors	= 0;
	pAd->stat.tx_aborted_errors= 0;
	pAd->stat.tx_carrier_errors= 0;
	pAd->stat.tx_fifo_errors	= 0;
	pAd->stat.tx_heartbeat_errors = 0;
	pAd->stat.tx_window_errors	= 0;

	pAd->stat.rx_packets	= 0;
	pAd->stat.rx_bytes 	= 0;
	pAd->stat.rx_dropped 	= 0;
	pAd->stat.rx_errors	= 0;
	pAd->stat.rx_length_errors = 0;
	pAd->stat.rx_over_errors	= 0;
	pAd->stat.rx_crc_errors	= 0;
	pAd->stat.rx_frame_errors	= 0;
	pAd->stat.rx_fifo_errors	= 0;
	pAd->stat.rx_missed_errors	= 0;

	pAd->stat.collisions	= 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static const struct net_device_ops VirtualIF_netdev_ops = {
	.ndo_open               = VirtualIF_open,
	.ndo_stop               = VirtualIF_close,
	.ndo_start_xmit         = VirtualIF_start_xmit,
	.ndo_get_stats          = VirtualIF_get_stats,
	.ndo_set_mac_address    = VirtualIF_set_mac_addr,
	.ndo_change_mtu         = ei_change_mtu,
	.ndo_do_ioctl           = VirtualIF_ioctl,
	.ndo_validate_addr      = eth_validate_addr,
};
#endif

// Register pseudo interface
void VirtualIF_init(pEND_DEVICE pAd, struct net_device *net_dev)
{
	struct net_device *dev;
	PSEUDO_ADAPTER *pPseudoAd;
#ifdef CONFIG_RAETH_READ_MAC_FROM_MTD
	int i = 0;
	struct sockaddr addr;
	unsigned char zero1[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char zero2[6]={0x00,0x00,0x00,0x00,0x00,0x00};
#endif

	dev = alloc_etherdev(sizeof(PSEUDO_ADAPTER));
	strcpy(dev->name, DEV2_NAME);

	//Get mac2 address from flash
#ifdef CONFIG_RAETH_READ_MAC_FROM_MTD
#ifdef RA_MTD_RW_BY_NUM
	i = ra_mtd_read(2, GMAC2_OFFSET, 6, addr.sa_data);
#else
	i = ra_mtd_read_nm("Factory", GMAC2_OFFSET, 6, addr.sa_data);
#endif

	//If reading mtd failed or mac0 is empty, generate a mac address
	if (i < 0 || (memcmp(addr.sa_data, zero1, 6) == 0) ||
	    (memcmp(addr.sa_data, zero2, 6) == 0)) {
		unsigned char mac_addr01234[5] = {0x00, 0x0C, 0x43, 0x28, 0x80};
		net_srandom(jiffies);
		memcpy(addr.sa_data, mac_addr01234, 5);
		addr.sa_data[5] = net_random()&0xFF;
	}

	VirtualIF_set_mac_addr(dev, &addr);
#endif

	ether_setup(dev);
	pPseudoAd = netdev_priv(dev);
	pPseudoAd->PseudoDev = dev;
	pPseudoAd->RaethDev = net_dev;
	pAd->PseudoDev = dev;

	VirtualIF_reset_statistics(pPseudoAd);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	dev->netdev_ops = &VirtualIF_netdev_ops;
#else
	dev->hard_start_xmit = VirtualIF_start_xmit;
	dev->stop = VirtualIF_close;
	dev->open = VirtualIF_open;
	dev->do_ioctl = VirtualIF_ioctl;
	dev->set_mac_address = VirtualIF_set_mac_addr;
	dev->get_stats = VirtualIF_get_stats;
	dev->change_mtu = ei_change_mtu;
	dev->mtu = 1500;
#endif

#if defined (CONFIG_ETHTOOL)
	dev->ethtool_ops = &ra_virt_ethtool_ops;
	// init mii structure
	pPseudoAd->mii_info.dev = dev;
	pPseudoAd->mii_info.mdio_read = mdio_virt_read;
	pPseudoAd->mii_info.mdio_write = mdio_virt_write;
	pPseudoAd->mii_info.phy_id_mask = 0x1f;
	pPseudoAd->mii_info.reg_num_mask = 0x1f;
	pPseudoAd->mii_info.phy_id = 0x1e;
	pPseudoAd->mii_info.supports_gmii = mii_check_gmii_support(&pPseudoAd->mii_info);
#endif

	// Register this device
	register_netdevice(dev);
}
#endif

int ei_start_xmit_gmac1(struct sk_buff* skb, struct net_device *dev)
{
	return ei_start_xmit(skb, dev, 1);
}

void ei_tx_timeout(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);

	if (ei_local) {
		dev->trans_start = jiffies;  /* prevent tx timeout */
		tasklet_schedule(&ei_local->tx_tasklet);
	}
}

void reset_statistics(END_DEVICE* ei_local)
{
	ei_local->stat.tx_packets	= 0;
	ei_local->stat.tx_bytes 	= 0;
	ei_local->stat.tx_dropped 	= 0;
	ei_local->stat.tx_errors	= 0;
	ei_local->stat.tx_aborted_errors= 0;
	ei_local->stat.tx_carrier_errors= 0;
	ei_local->stat.tx_fifo_errors	= 0;
	ei_local->stat.tx_heartbeat_errors = 0;
	ei_local->stat.tx_window_errors	= 0;

	ei_local->stat.rx_packets	= 0;
	ei_local->stat.rx_bytes 	= 0;
	ei_local->stat.rx_dropped 	= 0;
	ei_local->stat.rx_errors	= 0;
	ei_local->stat.rx_length_errors = 0;
	ei_local->stat.rx_over_errors	= 0;
	ei_local->stat.rx_crc_errors	= 0;
	ei_local->stat.rx_frame_errors	= 0;
	ei_local->stat.rx_fifo_errors	= 0;
	ei_local->stat.rx_missed_errors	= 0;

	ei_local->stat.collisions	= 0;
#ifdef CONFIG_RAETH_NAPI
	atomic_set(&ei_local->irq_sem, 1);
#endif
}

struct net_device_stats *ei_get_stats(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	return &ei_local->stat;
}

/*
 * Set the hardware MAC address.
 */
int ei_set_mac_addr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	if(netif_running(dev))
		return -EBUSY;

	ra2880MacAddressSet(addr->sa_data);
	return 0;
}


/**
 * ei_init - pick up ethernet port at boot time
 * @dev: network device to probe
 *
 * This routine probe the ethernet port at boot time.
 */
int __init ei_init(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
#ifdef CONFIG_RAETH_READ_MAC_FROM_MTD
	int i;
	struct sockaddr addr;
	unsigned char zero1[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char zero2[6]={0x00,0x00,0x00,0x00,0x00,0x00};
#endif

	fe_reset();

	//Get mac0 address from flash
#ifdef CONFIG_RAETH_READ_MAC_FROM_MTD
#ifdef RA_MTD_RW_BY_NUM
	i = ra_mtd_read(2, GMAC0_OFFSET, 6, addr.sa_data);
#else
	i = ra_mtd_read_nm("Factory", GMAC0_OFFSET, 6, addr.sa_data);
#endif

	//If reading mtd failed or mac0 is empty, generate a mac address
	if (i < 0 || (memcmp(addr.sa_data, zero1, 6) == 0) || 
	    (memcmp(addr.sa_data, zero2, 6) == 0)) {
		unsigned char mac_addr01234[5] = {0x00, 0x0C, 0x43, 0x28, 0x80};
		net_srandom(jiffies);
		memcpy(addr.sa_data, mac_addr01234, 5);
		addr.sa_data[5] = net_random()&0xFF;
	}

	ei_set_mac_addr(dev, &addr);
#endif

	spin_lock_init(&ei_local->page_lock);

#ifdef CONFIG_RAETH_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	netif_napi_add(dev, &ei_local->napi, raeth_clean, 128);
#endif
#endif
	ether_setup(dev);
	reset_statistics(ei_local);

#if defined (CONFIG_ETHTOOL)
	// init mii structure
	ei_local->mii_info.dev = dev;
	ei_local->mii_info.mdio_read = mdio_read;
	ei_local->mii_info.mdio_write = mdio_write;
	ei_local->mii_info.phy_id_mask = 0x1f;
	ei_local->mii_info.reg_num_mask = 0x1f;
	ei_local->mii_info.supports_gmii = mii_check_gmii_support(&ei_local->mii_info);
	// TODO:   phy_id: 0~4
	ei_local->mii_info.phy_id = 1;
#endif

	return 0;
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	if (!try_module_get(THIS_MODULE))
	{
		printk("%s: Cannot reserve module\n", __FUNCTION__);
		return -1;
	}
#else
	MOD_INC_USE_COUNT;
#endif

	printk("Raeth %s (",RAETH_VERSION);
#if defined (CONFIG_RAETH_NAPI)
	printk("NAPI");
#elif defined (CONFIG_RA_NETWORK_TASKLET_BH)
	printk("Tasklet");
#elif defined (CONFIG_RA_NETWORK_WORKQUEUE_BH)
	printk("Workqueue");
#endif

#if defined (CONFIG_RAETH_SKB_RECYCLE_2K)
	printk(",SkbRecycle");
#endif
	printk(")\n");

	ei_local = netdev_priv(dev); // get device pointer from System
	if (ei_local == NULL)
	{
		printk(KERN_EMERG "%s: ei_open passed a non-existent device!\n", dev->name);
		return -ENXIO;
	}

	spin_lock_irqsave(&(ei_local->page_lock), flags);

	err = request_irq(dev->irq, ei_interrupt, IRQF_DISABLED, dev->name, dev);	// try to fix irq in open
	if (err) {
		spin_unlock_irqrestore(&(ei_local->page_lock), flags);
		return err;
	}

	fe_pdma_init(dev);

	fe_sw_init();

	ra2880MacAddressSet(dev->dev_addr);

	forward_config(dev);

#if defined (CONFIG_RT_3052_ESW)
	*((volatile u32 *)(RALINK_INTCL_BASE + 0x34)) = (1<<17);
	*((volatile u32 *)(ESW_IMR)) &= ~(ESW_INT_ALL);
	INIT_WORK(&ei_local->kill_sig_wq, kill_sig_workq);
	err = request_irq(SURFBOARDINT_ESW, esw_interrupt, IRQF_DISABLED, "Ralink_ESW", dev);
	if (err) {
		free_irq(dev->irq, dev);
		spin_unlock_irqrestore(&(ei_local->page_lock), flags);
		return err;
	}
#endif // CONFIG_RT_3052_ESW //

#ifndef CONFIG_RAETH_NAPI
#ifdef WORKQUEUE_BH
 	INIT_WORK(&ei_local->rx_wq, ei_receive_workq);
#else
	tasklet_init(&ei_local->rx_tasklet, ei_receive, (unsigned long)dev);
#endif
#endif
	tasklet_init(&ei_local->tx_tasklet, ei_xmit_housekeeping, (unsigned long)dev);

	netif_start_queue(dev);

#ifdef CONFIG_PSEUDO_SUPPORT
	if(!ei_local->PseudoDev) {
		VirtualIF_init(ei_local, dev);
	}else {
		ra2880Mac2AddressSet(ei_local->PseudoDev->dev_addr);
	}
	if(ei_local->PseudoDev) {
		VirtualIF_open(ei_local->PseudoDev);
	}
#endif

	/* delay IRQ to 4 interrupts, delay max 4*20us */
	sysRegWrite(DLY_INT_CFG, 0x84048404);

#ifdef CONFIG_RAETH_NAPI
	atomic_dec(&ei_local->irq_sem);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	napi_enable(&ei_local->napi);
#else
	netif_poll_enable(dev);
#endif
	sysRegWrite(FE_INT_ENABLE, RX_DONE_INT0 | TX_DLY_INT);
#else
	sysRegWrite(FE_INT_ENABLE, RX_DLY_INT | TX_DLY_INT);
#endif

	eth_close=0; /* set flag to open protect eth while init or reinit */

	fe_pdma_start();

	spin_unlock_irqrestore(&(ei_local->page_lock), flags);

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
	END_DEVICE *ei_local = netdev_priv(dev);	// device pointer
	unsigned long flags;

	spin_lock_irqsave(&(ei_local->page_lock), flags);

	eth_close = 1; /* set closed flag protect eth while init or reinit */

#ifdef CONFIG_PSEUDO_SUPPORT
	VirtualIF_close(ei_local->PseudoDev);
#endif

	netif_stop_queue(dev);

	fe_pdma_stop();
	msleep(10);

	sysRegWrite(FE_INT_ENABLE, 0);

#ifndef CONFIG_RAETH_NAPI
#ifdef WORKQUEUE_BH
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	cancel_work_sync(&ei_local->rx_wq);
#endif
#else
	tasklet_kill(&ei_local->rx_tasklet);
#endif
#endif
	tasklet_kill(&ei_local->tx_tasklet);

	free_irq(dev->irq, dev);

#if defined (CONFIG_RT_3052_ESW)
	free_irq(SURFBOARDINT_ESW, dev);
#endif

	for (i = 0; i < NUM_RX_DESC; i++)
	{
		if (ei_local->rx_ring0[i].rxd_info1.PDP0) {
			dma_unmap_single(&dev->dev, ei_local->rx_ring0[i].rxd_info1.PDP0, MAX_RX_LENGTH, DMA_FROM_DEVICE);
			ei_local->rx_ring0[i].rxd_info1.PDP0 = 0;
		}
	}

	for (i = 0; i < NUM_TX_DESC; i++)
	{
		if (ei_local->tx0_free[i]) {
			dma_unmap_single(&dev->dev, ei_local->tx_ring0[i].txd_info1.SDP0, ei_local->tx0_free[i]->len, DMA_TO_DEVICE);
			ei_local->tx_ring0[i].txd_info1.SDP0 = 0;
			ei_local->tx_ring0[i].txd_info2.SDL0 = 0;
			dev_kfree_skb_any(ei_local->tx0_free[i]);
			ei_local->tx0_free[i] = NULL;
		}
	}

	ei_local->tx_free_idx = 0;

#ifdef CONFIG_RAETH_NAPI
	atomic_inc(&ei_local->irq_sem);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	napi_disable(&ei_local->napi);
#else
	netif_poll_disable(dev);
#endif
#endif
	fe_reset();

	spin_unlock_irqrestore(&(ei_local->page_lock), flags);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	module_put(THIS_MODULE);
#else
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static const struct net_device_ops ei_netdev_ops = {
        .ndo_init               = ei_init,
        .ndo_open               = ei_open,
        .ndo_stop               = ei_close,
        .ndo_start_xmit         = ei_start_xmit_gmac1,
        .ndo_get_stats          = ei_get_stats,
        .ndo_change_mtu         = ei_change_mtu,
        .ndo_do_ioctl           = ei_ioctl,
        .ndo_set_mac_address    = eth_mac_addr,
        .ndo_validate_addr      = eth_validate_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
        .ndo_poll_controller    = raeth_clean,
#endif
//	.ndo_tx_timeout		= ei_tx_timeout,
};
#endif

void raeth_setup_dev_fptable(struct net_device *dev)
{
	RAETH_PRINT(__FUNCTION__ "is called!\n");

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	dev->netdev_ops		= &ei_netdev_ops;
#else
	dev->open		= ei_open;
	dev->stop		= ei_close;
	dev->hard_start_xmit	= ei_start_xmit_gmac1;
	dev->get_stats		= ei_get_stats;
	dev->set_mac_address	= ei_set_mac_addr;
	dev->change_mtu		= ei_change_mtu;
	dev->mtu		= 1500;
	dev->do_ioctl		= ei_ioctl;
//	dev->tx_timeout		= ei_tx_timeout;
#ifdef CONFIG_RAETH_NAPI
        dev->poll		= &raeth_clean;
#if defined (CONFIG_RAETH_ROUTER)
	dev->weight		= 32;
#else
	dev->weight		= 128;
#endif
#endif
#endif

#if defined (CONFIG_ETHTOOL)
	dev->ethtool_ops	= &ra_ethtool_ops;
#endif

#define TX_TIMEOUT (5*HZ)
	dev->watchdog_timeo	= TX_TIMEOUT;
}


/**
 * raeth_init - Module Init code
 *
 * Called by kernel to register net_device
 *
 */
int __init raeth_init(void)
{
	int ret;
	struct net_device *dev = alloc_etherdev(sizeof(END_DEVICE));

#ifdef CONFIG_RALINK_VISTA_BASIC
	int sw_id=0;
	mii_mgr_read(29, 31, &sw_id);
	is_switch_175c = (sw_id == 0x175c) ? 1:0;
#endif

	if (!dev)
		return -ENOMEM;

	dev_raether = dev;

	strcpy(dev->name, DEV_NAME);
	dev->irq = IRQ_ENET0;
	dev->addr_len = 6;
	dev->base_addr = RALINK_FRAME_ENGINE_BASE;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
	dev->init = ei_init;
#endif
	raeth_setup_dev_fptable(dev);

	/* Register net device for the driver */
	if ( register_netdev(dev) != 0) {
		printk(KERN_WARNING " " __FILE__ ": No ethernet port found.\n");
		return -ENXIO;
	}

	if (raeth_ring_alloc(dev) != 0) {
		printk(KERN_WARNING "raeth_ring_alloc FAILED!\n");
		return -ENOMEM;
	}

#ifdef CONFIG_RAETH_NETLINK
	csr_netlink_init();
#endif
	ret = debug_proc_init();

	printk("Ralink APSoC Ethernet Driver Initialized. %s. Rx Ring: %d, Tx Ring: %d. Max packet size: %d.\n", RAETH_VERSION, NUM_RX_DESC, NUM_TX_DESC, MAX_RX_LENGTH);

	return ret;
}


/**
 * raeth_uninit - Module Exit code
 *
 * Cmd 'rmmod' will invode the routine to exit the module
 *
 */
void __exit raeth_uninit(void)
{
	END_DEVICE *ei_local;
	struct net_device *dev = dev_raether;
	if (!dev)
		return;

	ei_local = netdev_priv(dev);

	raeth_ring_free(dev);

#ifdef CONFIG_PSEUDO_SUPPORT
	unregister_netdev(ei_local->PseudoDev);
	free_netdev(ei_local->PseudoDev);
#endif

	unregister_netdev(dev);

	free_netdev(dev);
	debug_proc_exit();

#ifdef CONFIG_RAETH_NETLINK
	csr_netlink_end();
#endif

	dev_raether = NULL;
}

module_init(raeth_init);
module_exit(raeth_uninit);
MODULE_LICENSE("GPL");
