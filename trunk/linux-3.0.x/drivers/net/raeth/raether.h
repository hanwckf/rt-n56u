#ifndef __RAETHER_H__
#define __RAETHER_H__

#include <linux/mii.h>
#include <linux/interrupt.h>

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>

#include "ra_ethreg.h"

#define RAETH_VERSION		"v3.2.0"
#define RAETH_DEV_NAME		"raeth"

#define DEV_NAME		"eth2"
#define DEV2_NAME		"eth3"

#if defined (CONFIG_RALINK_RT3052) || defined (MEMORY_OPTIMIZATION)
#define NUM_RX_DESC		128
#define NUM_TX_DESC		256
#else
#define NUM_RX_DESC		256
#define NUM_TX_DESC		512
#endif

#if defined (CONFIG_RAETH_QDMA)
#define NUM_QRX_DESC		16	/* fake, for FQ only */
#define NUM_QDMA_PAGE		NUM_TX_DESC
#define QDMA_PAGE_SIZE		2048
#endif

#if (NUM_RX_DESC < 256)
#define NUM_RX_MAX_PROCESS	32
#else
#define NUM_RX_MAX_PROCESS	16
#endif

#if defined (CONFIG_RAETH_NAPI_GRO) && defined (CONFIG_RALINK_MT7621)
#define NAPI_WEIGHT		64
#else
#define NAPI_WEIGHT		32
#endif

#if defined (CONFIG_RALINK_MT7621)
#define GMAC0_OFFSET		0xE000
#define GMAC2_OFFSET		0xE006
#else
#define GMAC0_OFFSET		0x28
#define GMAC2_OFFSET		0x22
#endif

#define PSE_PORT_CPU		0
#define PSE_PORT_GMAC1		1
#define PSE_PORT_GMAC2		2
#if defined (CONFIG_RALINK_MT7621)
#define PSE_PORT_PPE		4
#else
#define PSE_PORT_PPE		6
#endif

#if defined (CONFIG_RAETH_JUMBOFRAME)
#define MAX_RX_LENGTH		4096
#else
#define MAX_RX_LENGTH		1536
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define NETIF_F_HW_VLAN_CTAG_TX	NETIF_F_HW_VLAN_TX
#define NETIF_F_HW_VLAN_CTAG_RX	NETIF_F_HW_VLAN_RX
#endif

#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
#define RAETH_PDMA_V2
#endif

#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
#define RAETH_SDMA
#endif

#if defined (CONFIG_RAETH_DEBUG)
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif

typedef struct _END_DEVICE
{
#if defined (CONFIG_PSEUDO_SUPPORT)
	struct net_device		*PseudoDev;
#endif
#if defined (CONFIG_RAETH_NAPI)
	struct napi_struct		napi;
#else
	struct tasklet_struct		rx_tasklet;
	struct tasklet_struct		tx_tasklet;
#endif
	spinlock_t			page_lock;
	spinlock_t			stat_lock;

	unsigned int			active;
	unsigned int			min_pkt_len;

	struct PDMA_rxdesc		*rxd_ring;
#if defined (CONFIG_RAETH_QDMA)
	struct QDMA_txdesc		*txd_cpu_ptr;
	struct QDMA_txdesc		*txd_pool;
	unsigned int			txd_pool_free_num;
	unsigned int			txd_pool_free_head;
	unsigned int			txd_pool_free_tail;
#else
	struct PDMA_txdesc		*txd_ring;
	unsigned int			txd_free_idx;
#endif

	struct sk_buff			*rxd_buff[NUM_RX_DESC];
	struct sk_buff			*txd_buff[NUM_TX_DESC];
#if defined (CONFIG_RAETH_QDMA)
	unsigned int			txd_pool_info[NUM_TX_DESC];

#if defined (CONFIG_RA_HW_NAT_QDMA)
	struct QDMA_txdesc		*free_head;
	void				*free_head_page;

	dma_addr_t			free_head_phy;
	dma_addr_t			free_head_page_phy;
#endif
	dma_addr_t			txd_pool_phy;
#else
	dma_addr_t			txd_ring_phy;
#endif
	dma_addr_t			rxd_ring_phy;

	struct work_struct		stat_work;
	struct timer_list		stat_timer;
	struct rtnl_link_stats64	stat;
#if defined (CONFIG_ETHTOOL)
	struct mii_if_info		mii_info;
#endif
} END_DEVICE, *PEND_DEVICE;

#if defined (CONFIG_PSEUDO_SUPPORT)
typedef struct _PSEUDO_ADAPTER
{
	struct net_device		*RaethDev;
	struct rtnl_link_stats64	stat;
#if defined (CONFIG_ETHTOOL)
	struct mii_if_info		mii_info;
#endif
} PSEUDO_ADAPTER, *PPSEUDO_ADAPTER;
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (CONFIG_RALINK_MT7621)
u32  get_map_hw_vlan_tx(u32 idx);
void set_map_hw_vlan_tx(u32 idx, u32 vid);
#endif

int ei_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
#if defined (CONFIG_PSEUDO_SUPPORT)
int VirtualIF_ioctl(struct net_device * net_dev, struct ifreq * ifr, int cmd);
#endif

#if defined (CONFIG_RAETH_ESW_CONTROL)
int  esw_ioctl_init(void);
int  esw_control_post_init(void);
void esw_ioctl_uninit(void);
#endif

#endif
