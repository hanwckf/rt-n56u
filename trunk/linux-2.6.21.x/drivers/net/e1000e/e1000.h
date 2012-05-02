/*******************************************************************************

  Intel PRO/1000 Linux driver
  Copyright(c) 1999 - 2008 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/

/* Linux PRO/1000 Ethernet Driver main header file */

#ifndef _E1000_H_
#define _E1000_H_

#include <linux/types.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <linux/netdevice.h>

#include "kcompat.h"

#include "e1000_hw.h"

struct e1000_info;

#define e_printk(level, adapter, format, arg...) \
	printk(level "%s: %s: " format, pci_name(adapter->pdev), \
	       (strchr(adapter->netdev->name, '%') ? "" : \
	        adapter->netdev->name), ## arg)

#define e_dbg(format, arg...) do { (void)(adapter); } while (0)

#define e_err(format, arg...) \
	e_printk(KERN_ERR, adapter, format, ## arg)
#define e_info(format, arg...) \
	e_printk(KERN_INFO, adapter, format, ## arg)
#define e_warn(format, arg...) \
	e_printk(KERN_WARNING, adapter, format, ## arg)
#define e_notice(format, arg...) \
	e_printk(KERN_NOTICE, adapter, format, ## arg)


#ifdef CONFIG_E1000E_MSIX
/* Interrupt modes, as used by the IntMode paramter */
#define E1000E_INT_MODE_LEGACY		0
#define E1000E_INT_MODE_MSI		1
#define E1000E_INT_MODE_MSIX		2

#endif /* CONFIG_E1000E_MSIX */

#define E1000_MAX_INTR 10

/* Tx/Rx descriptor defines */
#define E1000_DEFAULT_TXD		256
#define E1000_MAX_TXD			4096
#define E1000_MIN_TXD			80

#define E1000_DEFAULT_RXD		256
#define E1000_MAX_RXD			4096
#define E1000_MIN_RXD			80

#define E1000_MIN_ITR_USECS		10 /* 100000 irq/sec */
#define E1000_MAX_ITR_USECS		10000 /* 100    irq/sec */

/* Early Receive defines */
#define E1000_ERT_2048			0x100

#define E1000_FC_PAUSE_TIME		0x0680 /* 858 usec */

/* How many Tx Descriptors do we need to call netif_wake_queue ? */
/* How many Rx Buffers do we bundle into one write to the hardware ? */
#define E1000_RX_BUFFER_WRITE		16 /* Must be power of 2 */

#define AUTO_ALL_MODES			0
#define E1000_EEPROM_APME		0x0400

#define E1000_MNG_VLAN_NONE		(-1)

/* Number of packet split data buffers (not including the header buffer) */
#define PS_PAGE_BUFFERS			(MAX_PS_BUFFERS - 1)

enum e1000_boards {
	board_82571,
	board_82572,
	board_82573,
	board_82574,
	board_80003es2lan,
	board_ich8lan,
	board_ich9lan,
	board_ich10lan,
};

struct e1000_queue_stats {
	u64 packets;
	u64 bytes;
};

struct e1000_ps_page {
	struct page *page;
	u64 dma; /* must be u64 - written to hw */
};

/*
 * wrappers around a pointer to a socket buffer,
 * so a DMA handle can be stored along with the buffer
 */
struct e1000_buffer {
	dma_addr_t dma;
	struct sk_buff *skb;
	union {
		/* Tx */
		struct {
			unsigned long time_stamp;
			u16 length;
			u16 next_to_watch;
		};
		/* Rx */
		/* arrays of page information for packet split */
		struct e1000_ps_page *ps_pages;
	};
	struct page *page;
};

struct e1000_ring {
	void *desc;			/* pointer to ring memory  */
	dma_addr_t dma;			/* phys address of ring    */
	unsigned int size;		/* length of ring in bytes */
	unsigned int count;		/* number of desc. in ring */

	u16 next_to_use;
	u16 next_to_clean;

	u16 head;
	u16 tail;

	/* array of buffer information structs */
	struct e1000_buffer *buffer_info;

#ifdef CONFIG_E1000E_MSIX
	char name[IFNAMSIZ + 5];
	u32 ims_val;
	u32 itr_val;
	u16 itr_register;
	int set_itr;

#endif /* CONFIG_E1000E_MSIX */
	struct sk_buff *rx_skb_top;

	struct e1000_queue_stats stats;
};

#ifdef SIOCGMIIPHY
/* PHY register snapshot values */
struct e1000_phy_regs {
	u16 bmcr;		/* basic mode control register    */
	u16 bmsr;		/* basic mode status register     */
	u16 advertise;		/* auto-negotiation advertisement */
	u16 lpa;		/* link partner ability register  */
	u16 expansion;		/* auto-negotiation expansion reg */
	u16 ctrl1000;		/* 1000BASE-T control register    */
	u16 stat1000;		/* 1000BASE-T status register     */
	u16 estatus;		/* extended status register       */
};
#endif

/* board specific private data structure */
struct e1000_adapter {
	struct timer_list watchdog_timer;
	struct timer_list phy_info_timer;
	struct timer_list blink_timer;

	struct work_struct reset_task;
	struct work_struct watchdog_task;

	const struct e1000_info *ei;

	struct vlan_group *vlgrp;
	u32 bd_number;
	u32 rx_buffer_len;
	u16 mng_vlan_id;
	u16 link_speed;
	u16 link_duplex;

	spinlock_t tx_queue_lock; /* prevent concurrent tail updates */

	/* track device up/down/testing state */
	unsigned long state;

	/* Interrupt Throttle Rate */
	u32 itr;
	u32 itr_setting;
	u16 tx_itr;
	u16 rx_itr;

	/*
	 * Tx
	 */
	struct e1000_ring *tx_ring /* One per active queue */
						____cacheline_aligned_in_smp;

#ifdef CONFIG_E1000E_NAPI
	struct napi_struct napi;
#endif

	unsigned long tx_queue_len;
	unsigned int restart_queue;
	u32 txd_cmd;

	bool detect_tx_hung;
	u8 tx_timeout_factor;

	u32 tx_int_delay;
	u32 tx_abs_int_delay;

	unsigned int total_tx_bytes;
	unsigned int total_tx_packets;
	unsigned int total_rx_bytes;
	unsigned int total_rx_packets;

	/* Tx stats */
	u64 tpt_old;
	u64 colc_old;
	u32 gotc;
	u64 gotc_old;
	u32 tx_timeout_count;
	u32 tx_fifo_head;
	u32 tx_head_addr;
	u32 tx_fifo_size;
	u32 tx_dma_failed;

	/*
	 * Rx
	 */
#ifdef CONFIG_E1000E_NAPI
	bool (*clean_rx) (struct e1000_adapter *adapter,
			  int *work_done, int work_to_do)
						____cacheline_aligned_in_smp;
#else
	bool (*clean_rx) (struct e1000_adapter *adapter)
						____cacheline_aligned_in_smp;
#endif
	void (*alloc_rx_buf) (struct e1000_adapter *adapter,
			      int cleaned_count);
	struct e1000_ring *rx_ring;

	u32 rx_int_delay;
	u32 rx_abs_int_delay;

	/* Rx stats */
	u64 hw_csum_err;
	u64 hw_csum_good;
	u64 rx_hdr_split;
	u32 gorc;
	u64 gorc_old;
	u32 alloc_rx_buff_failed;
	u32 rx_dma_failed;

	unsigned int rx_ps_pages;
	u16 rx_ps_bsize0;
	u32 max_frame_size;
	u32 min_frame_size;

	/* OS defined structs */
	struct net_device *netdev;
	struct pci_dev *pdev;
	struct net_device_stats net_stats;
	spinlock_t stats_lock;      /* prevent concurrent stats updates */

	/* structs defined in e1000_hw.h */
	struct e1000_hw hw;

	struct e1000_hw_stats stats;
	struct e1000_phy_info phy_info;
	struct e1000_phy_stats phy_stats;

#ifdef SIOCGMIIPHY
	/* Snapshot of PHY registers */
	struct e1000_phy_regs phy_regs;
#endif

	struct e1000_ring test_tx_ring;
	struct e1000_ring test_rx_ring;
	u32 test_icr;

	u32 msg_enable;
#ifdef CONFIG_E1000E_MSIX
	struct msix_entry *msix_entries;
	int int_mode;
	u32 eiac_mask;
#endif /* CONFIG_E1000E_MSIX */

	u32 eeprom_wol;
	u32 wol;
	u32 pba;

	bool fc_autoneg;

	unsigned long led_status;

	unsigned int flags;
	u32 *config_space;
	u32 stats_freq_us;		/* stats update freq (microseconds) */
};

struct e1000_info {
	e1000_mac_type		mac;
	unsigned int		flags;
	u32			pba;
	void			(*init_ops)(struct e1000_hw *);
	s32			(*get_variants)(struct e1000_adapter *);
};

/* hardware capability, feature, and workaround flags */
#define FLAG_HAS_AMT                      (1 << 0)
#define FLAG_HAS_FLASH                    (1 << 1)
#define FLAG_HAS_HW_VLAN_FILTER           (1 << 2)
#define FLAG_HAS_WOL                      (1 << 3)
#define FLAG_HAS_ERT                      (1 << 4)
#define FLAG_HAS_CTRLEXT_ON_LOAD          (1 << 5)
#define FLAG_HAS_SWSM_ON_LOAD             (1 << 6)
#define FLAG_HAS_JUMBO_FRAMES             (1 << 7)
#define FLAG_HAS_ASPM                     (1 << 8)
#define FLAG_IS_ICH                       (1 << 9)
#define FLAG_HAS_MSIX                     (1 << 10)
#define FLAG_HAS_SMART_POWER_DOWN         (1 << 11)
#define FLAG_IS_QUAD_PORT_A               (1 << 12)
#define FLAG_IS_QUAD_PORT                 (1 << 13)
#define FLAG_TIPG_MEDIUM_FOR_80003ESLAN   (1 << 14)
#define FLAG_APME_IN_WUC                  (1 << 15)
#define FLAG_APME_IN_CTRL3                (1 << 16)
#define FLAG_APME_CHECK_PORT_B            (1 << 17)
#define FLAG_DISABLE_FC_PAUSE_TIME        (1 << 18)
#define FLAG_NO_WAKE_UCAST                (1 << 19)
#define FLAG_MNG_PT_ENABLED               (1 << 20)
#define FLAG_RESET_OVERWRITES_LAA         (1 << 21)
#define FLAG_TARC_SPEED_MODE_BIT          (1 << 22)
#define FLAG_TARC_SET_BIT_ZERO            (1 << 23)
#define FLAG_RX_NEEDS_RESTART             (1 << 24)
#define FLAG_LSC_GIG_SPEED_DROP           (1 << 25)
#define FLAG_SMART_POWER_DOWN             (1 << 26)
#define FLAG_MSI_ENABLED                  (1 << 27)
#define FLAG_RX_CSUM_ENABLED              (1 << 28)
#define FLAG_TSO_FORCE                    (1 << 29)
#define FLAG_MSI_TEST_FAILED              (1 << 30)
#define FLAG_RX_RESTART_NOW               (1 << 31)


#define E1000_RX_DESC_PS(R, i)	    \
	(&(((union e1000_rx_desc_packet_split *)((R).desc))[i]))
#define E1000_GET_DESC(R, i, type)	(&(((struct type *)((R).desc))[i]))
#define E1000_RX_DESC(R, i)		E1000_GET_DESC(R, i, e1000_rx_desc)
#define E1000_TX_DESC(R, i)		E1000_GET_DESC(R, i, e1000_tx_desc)
#define E1000_CONTEXT_DESC(R, i)	E1000_GET_DESC(R, i, e1000_context_desc)

enum e1000_state_t {
	__E1000_TESTING,
	__E1000_RESETTING,
	__E1000_DOWN
};

enum latency_range {
	lowest_latency = 0,
	low_latency = 1,
	bulk_latency = 2,
	latency_invalid = 255
};

extern char e1000e_driver_name[];
extern const char e1000e_driver_version[];

extern void e1000_check_options(struct e1000_adapter *adapter);
extern void e1000_set_ethtool_ops(struct net_device *netdev);
#ifdef ETHTOOL_OPS_COMPAT
extern int ethtool_ioctl(struct ifreq *ifr);
#endif

extern int e1000_up(struct e1000_adapter *adapter);
extern void e1000_down(struct e1000_adapter *adapter);
extern void e1000_reinit_locked(struct e1000_adapter *adapter);
extern void e1000_reset(struct e1000_adapter *adapter);
extern int e1000_setup_rx_resources(struct e1000_adapter *adapter);
extern int e1000_setup_tx_resources(struct e1000_adapter *adapter);
extern void e1000_free_rx_resources(struct e1000_adapter *adapter);
extern void e1000_free_tx_resources(struct e1000_adapter *adapter);
extern void e1000_update_stats(struct e1000_adapter *adapter);
#ifdef CONFIG_E1000E_MSIX
extern void e1000_set_interrupt_capability(struct e1000_adapter *adapter);
extern void e1000_reset_interrupt_capability(struct e1000_adapter *adapter);
#endif

extern unsigned int copybreak;

static inline u32 __er32(struct e1000_hw *hw, unsigned long reg)
{
	return readl(hw->hw_addr + reg);
}

static inline void __ew32(struct e1000_hw *hw, unsigned long reg, u32 val)
{
	writel(val, hw->hw_addr + reg);
}
#define er32(reg)	E1000_READ_REG(hw, E1000_##reg)
#define ew32(reg,val)	E1000_WRITE_REG(hw, E1000_##reg, (val))
#define e1e_flush()	er32(STATUS)

extern void e1000_init_function_pointers_82571(struct e1000_hw *hw);
extern void e1000_init_function_pointers_80003es2lan(struct e1000_hw *hw);
extern void e1000_init_function_pointers_ich8lan(struct e1000_hw *hw);

static inline s32 e1000_read_mac_addr(struct e1000_hw *hw)
{
        if (hw->mac.ops.read_mac_addr)
                return hw->mac.ops.read_mac_addr(hw);

        return e1000_read_mac_addr_generic(hw);
}

static inline void e1000_power_up_phy(struct e1000_hw *hw)
{
	if(hw->phy.ops.power_up) 
		hw->phy.ops.power_up(hw);
	hw->mac.ops.setup_link(hw);
}

#endif /* _E1000_H_ */
