#ifndef __RAETHER_H__
#define __RAETHER_H__

#include <linux/mii.h>
#include <linux/interrupt.h>

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>

#include "ra_ethreg.h"

#define RAETH_VERSION		"v3.1.7"
#define RAETH_DEV_NAME		"raeth"

#define DEV_NAME		"eth2"
#define DEV2_NAME		"eth3"

/* RT6856 workaround */
//#define RAETH_PDMAPTR_FROM_VAR

#if defined (CONFIG_PSEUDO_SUPPORT)
#define NUM_TX_RING		2
#else
#define NUM_TX_RING		1
#endif

#if defined (CONFIG_RALINK_RT3052) || defined (MEMORY_OPTIMIZATION)
#define NUM_TX_DESC		128
#define NUM_RX_DESC		128
#define NUM_RX_MAX_PROCESS	32
#else
#define NUM_TX_DESC		256
#define NUM_RX_DESC		256
#define NUM_RX_MAX_PROCESS	16
#endif

#define NAPI_WEIGHT		32

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
	unsigned int			active;
	unsigned int			min_pkt_len;

	unsigned int			tx_free_idx[NUM_TX_RING];
#if defined (RAETH_PDMAPTR_FROM_VAR)
	unsigned int			tx_calc_idx[NUM_TX_RING];
	unsigned int			rx_calc_idx;
#endif

	struct PDMA_txdesc		*tx_ring[NUM_TX_RING];
	struct PDMA_rxdesc		*rx_ring;
	struct sk_buff			*tx_free[NUM_TX_RING][NUM_TX_DESC];
	struct sk_buff			*rx_buff[NUM_RX_DESC];

	dma_addr_t			tx_ring_phy[NUM_TX_RING];
	dma_addr_t			rx_ring_phy;

	spinlock_t			page_lock;

	spinlock_t			stat_lock;
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
