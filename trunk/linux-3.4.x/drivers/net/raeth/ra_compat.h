#ifndef __RA_COMPAT_H__
#define __RA_COMPAT_H__

///////////////////////////////////////////////////////////////

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define NETIF_F_HW_VLAN_CTAG_TX			NETIF_F_HW_VLAN_TX
#define NETIF_F_HW_VLAN_CTAG_RX			NETIF_F_HW_VLAN_RX
#define vlan_insert_tag_hwaccel(x,y,z)		__vlan_hwaccel_put_tag(x,z)
#else
#define vlan_insert_tag_hwaccel(x,y,z)		__vlan_hwaccel_put_tag(x,y,z)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
#define skb_vlan_tag_present(x)			vlan_tx_tag_present(x)
#define skb_vlan_tag_get(x)			vlan_tx_tag_get(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)
#define prandom_seed(x)				net_srandom(x)
#define prandom_u32()				net_random()
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
#include <linux/skbuff.h>
static inline unsigned int skb_frag_size(const skb_frag_t *frag)
{
	return frag->size;
}

static inline dma_addr_t skb_frag_dma_map(struct device *dev,
					  const skb_frag_t *frag,
					  size_t offset, size_t size,
					  enum dma_data_direction dir)
{
	return dma_map_page(dev, frag->page, frag->page_offset + offset, size, dir);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
typedef u32 netdev_features_t;
#endif

#if defined (CONFIG_MIPS)
#define phys_to_bus(a)			((a) & 0x1FFFFFFF)
#else
#define phys_to_bus(a)			(a)
#endif

///////////////////////////////////////////////////////////////

#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
#define RAETH_SDMA		/* Switch DMA Engine */
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
#define RAETH_PDMA_V2		/* Packet DMA Engine V2 */
#endif

#if defined (CONFIG_RALINK_MT7620)
//#define RAETH_HW_PADPKT // issue < rev 0205
#endif

#if defined (CONFIG_RALINK_MT7621)
#define RAETH_QDMA		/* QoS DMA Engine */
#define RAETH_HW_VLAN4K		/* FE support VLAN 0..4095 */
#define RAETH_HW_PADPKT		/* FE support padding TX path to 60/64 bytes */
#define RAETH_HW_CL45		/* FE support native MDIO 'clause 45' access */
#endif

#if defined (CONFIG_RALINK_MT7621)
#define MTD_GMAC0_OFFSET	0xE000
#define MTD_GMAC2_OFFSET	0xE006
#else
#define MTD_GMAC0_OFFSET	0x28
#define MTD_GMAC2_OFFSET	0x22
#endif

#if defined (CONFIG_RAETH_SWAP_GDMA)
#define PSE_PORT_GMAC1		2
#define PSE_PORT_GMAC2		1
#else
#define PSE_PORT_GMAC1		1
#define PSE_PORT_GMAC2		2
#endif
#if defined (CONFIG_RALINK_MT7621)
#define PSE_PORT_PPE		4
#else
#define PSE_PORT_PPE		6
#endif

#if defined (CONFIG_RALINK_MT7621)
#if defined (MEMORY_OPTIMIZATION) || defined (CONFIG_RAETH_QDMATX_QDMARX)
#define NUM_RX_DESC		256	/* MT7621 QDMA P5 RX SW support max 256 desc (bug?) */
#else
#define NUM_RX_DESC		512
#endif
#else
#if defined (MEMORY_OPTIMIZATION)
#define NUM_RX_DESC		128
#else
#define NUM_RX_DESC		256
#endif
#endif

#if defined (CONFIG_RAETH_QDMA)
#define NUM_QRX_DESC		16	/* for memory save (P5 SW RX is not used) */
#define NUM_PQ_RESV		4
#define NUM_TX_DESC		1024
#if defined (CONFIG_RA_HW_NAT_QDMA)
#define NUM_QDMA_PAGE		512
#else
#define NUM_QDMA_PAGE		16	/* for memory save (P5 HW TX is not used) */
#endif
#define QDMA_PAGE_SIZE		2048
#elif defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
#define NUM_TX_DESC		512
#else
#define NUM_TX_DESC		256
#endif

#if (NUM_RX_DESC < 256)
#define NUM_RX_MAX_PROCESS	32
#else
#define NUM_RX_MAX_PROCESS	16
#endif

#if defined (CONFIG_RAETH_JUMBOFRAME)
#define MAX_RX_LENGTH		2048
#else
#define MAX_RX_LENGTH		1536
#endif

#if defined (CONFIG_RAETH_NAPI_GRO) && defined (CONFIG_RALINK_MT7621)
#define NAPI_WEIGHT		64
#else
#define NAPI_WEIGHT		32
#endif

///////////////////////////////////////////////////////////////

#endif
