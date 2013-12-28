#ifndef __RAETHER_H__
#define __RAETHER_H__

#include <linux/mii.h>
#include <linux/version.h>
#include <linux/interrupt.h>

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboardint.h>

#include "ra_ethreg.h"

#define RAETH_VERSION		"v3.0.7"
#define RAETH_DEV_NAME		"raeth"

#if defined (CONFIG_RALINK_RT6855A)
#define RAETH_PDMAPTR_FROM_VAR
#endif

#if defined (MEMORY_OPTIMIZATION)
#define NUM_RX_DESC		128
#define NUM_TX_DESC		128
#define NUM_RX_MAX_PROCESS	32
#else
#if defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT3052)
#define NUM_RX_DESC		128
#define NUM_TX_DESC		128
#else
#define NUM_RX_DESC		256
#define NUM_TX_DESC		256
#endif
#define NUM_RX_MAX_PROCESS	16
#endif

#define DEV_NAME		"eth2"
#define DEV2_NAME		"eth3"

#define GMAC2_OFFSET		0x22
#if !defined (CONFIG_RALINK_RT6855A)
#define GMAC0_OFFSET		0x28
#else
#define GMAC0_OFFSET		0xE000
#endif

#define PSE_PORT_CPU		0
#define PSE_PORT_GMAC1		1
#define PSE_PORT_GMAC2		2
#if defined (CONFIG_RALINK_MT7621)
#define PSE_PORT_PPE		4
#else
#define PSE_PORT_PPE		6
#endif

#ifdef RAETH_DEBUG
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif

typedef struct end_device
{
	struct tasklet_struct		rx_tasklet;
	struct timer_list		stat_timer;
	spinlock_t			page_lock;
	spinlock_t			irqe_lock;
	spinlock_t			stat_lock;
#if defined (CONFIG_PSEUDO_SUPPORT)
	spinlock_t			hnat_lock;
	struct net_device		*PseudoDev;
#endif

	dma_addr_t			phy_tx_ring0;
	dma_addr_t			phy_rx_ring0;

#if defined (RAETH_PDMAPTR_FROM_VAR)
	unsigned int			rx_calc_idx;
	unsigned int			tx_calc_idx;
#endif
	unsigned int			tx_free_idx;
	struct PDMA_txdesc		*tx_ring0;
	struct PDMA_rxdesc		*rx_ring0;
	struct sk_buff			*rx0_skbuf[NUM_RX_DESC];
	struct sk_buff			*tx0_free[NUM_TX_DESC];

#if defined (CONFIG_RAETH_HW_VLAN_RX)
	struct vlan_group		*vlgrp;
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	struct rtnl_link_stats64	stat;
#else
	struct net_device_stats		stat;
#endif
#if defined (CONFIG_ETHTOOL)
	struct mii_if_info		mii_info;
#endif
} END_DEVICE, *pEND_DEVICE;


#if defined (CONFIG_PSEUDO_SUPPORT)
typedef struct _PSEUDO_ADAPTER {
	struct net_device		*RaethDev;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	struct rtnl_link_stats64	stat;
#else
	struct net_device_stats		stat;
#endif
#if defined (CONFIG_ETHTOOL)
	struct mii_if_info		mii_info;
#endif
} PSEUDO_ADAPTER, PPSEUDO_ADAPTER;
#endif

int ei_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
#ifdef CONFIG_PSEUDO_SUPPORT
int VirtualIF_ioctl(struct net_device * net_dev, struct ifreq * ifr, int cmd);
#endif

#if defined (CONFIG_RAETH_ESW_CONTROL)
int  esw_ioctl_init(void);
void esw_ioctl_uninit(void);
irqreturn_t esw_interrupt(int irq, void *dev_id);
#endif

#endif
