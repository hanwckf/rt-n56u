#ifndef __RAETHER_H__
#define __RAETHER_H__

#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/skbuff.h>

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>

#include "ra_compat.h"
#include "ra_eth_reg.h"

///////////////////////////////////////////////////////////////

#define RAETH_VERSION		"v3.2.4"
#define RAETH_DEV_NAME		"raeth"

#define DEV_NAME		"eth2"
#define DEV2_NAME		"eth3"

///////////////////////////////////////////////////////////////

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

	/* RX path */
	unsigned int			active;
	struct PDMA_rxdesc		*rxd_ring;
	struct sk_buff			*rxd_buff[NUM_RX_DESC];

	/* TX path */
	spinlock_t			page_lock;
#if !defined (RAETH_HW_PADPKT)
	unsigned int			min_pkt_len;
#endif
#if defined (CONFIG_RAETH_QDMA)
	struct QDMA_txdesc		*txd_pool;
	dma_addr_t			 txd_pool_phy;
	unsigned int			 txd_last_idx;
	unsigned int			 txd_pool_free_num;
	unsigned int			 txd_pool_free_head;
	unsigned int			 txd_pool_free_tail;
	unsigned int			 txd_pool_info[NUM_TX_DESC];
#else
	struct PDMA_txdesc		*txd_ring;
	unsigned int			 txd_last_idx;
	unsigned int			 txd_free_idx;
#endif
	struct sk_buff			*txd_buff[NUM_TX_DESC];

	/* VLAN TX maps */
#if defined (CONFIG_RAETH_HW_VLAN_TX) && !defined (RAETH_HW_VLAN4K)
	unsigned char			vlan_4k_map[VLAN_N_VID];
	unsigned short			vlan_id_map[16];
#endif

	/* QDMA HW staff & phys addr (unused in processing) */
#if defined (CONFIG_RAETH_QDMA)
	struct QDMA_txdesc		*fq_head;
	dma_addr_t			 fq_head_phy;
	unsigned char			*fq_head_page;
	dma_addr_t			 fq_head_page_phy;
#if !defined (CONFIG_RAETH_QDMATX_QDMARX)
	struct PDMA_rxdesc		*qrx_ring;
	dma_addr_t			 qrx_ring_phy;
	struct sk_buff			*qrx_buff;
#endif
#else
	dma_addr_t			txd_ring_phy;
#endif
	dma_addr_t			rxd_ring_phy;

	/* stats */
	spinlock_t			stat_lock;
	struct work_struct		stat_wq;
	struct timer_list		stat_timer;
	struct rtnl_link_stats64	stat;

	/* ethtool */
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

///////////////////////////////////////////////////////////////

int  raeth_ioctl(struct ifreq *ifr, int cmd);
#if defined (CONFIG_RAETH_ESW_CONTROL)
int  esw_ioctl_init_post(void);
int  esw_ioctl_init(void);
void esw_ioctl_uninit(void);
#elif defined (CONFIG_RAETH_DHCP_TOUCH)
void esw_dhcpc_init(void);
#endif

///////////////////////////////////////////////////////////////

#if defined (CONFIG_RAETH_DEBUG)
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...)
#endif

#endif
