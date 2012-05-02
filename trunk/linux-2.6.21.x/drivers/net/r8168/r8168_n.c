/*
################################################################################
# 
# Copyright(c) Realtek Semiconductor Corp. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation; either version 2 of the License, or (at your option) 
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 
# Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
# The full GNU General Public License is included in this distribution in the
# file called LICENSE.
# 
################################################################################
*/

/*
 *  This product is covered by one or more of the following patents:
 *  US5,307,459, US5,434,872, US5,732,094, US6,570,884, US6,115,776, and US6,327,625.
 */

/*
 * This driver is modified from r8169.c in Linux kernel 2.6.18
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>

#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "r8168.h"
#include "r8168_asf.h"

/* Maximum events (Rx packets, etc.) to handle at each interrupt. */
static const int max_interrupt_work = 20;

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC. */
static const int multicast_filter_limit = 32;

#define _R(NAME,MAC,RCR,MASK, JumFrameSz) \
	{ .name = NAME, .mcfg = MAC, .RCR_Cfg = RCR, .RxConfigMask = MASK, .jumbo_frame_sz = JumFrameSz }

static const struct {
	const char *name;
	u8 mcfg;
	u32 RCR_Cfg;
	u32 RxConfigMask;	/* Clears the bits supported by this chip */
	u32 jumbo_frame_sz;
} rtl_chip_info[] = {
	_R("RTL8168B/8111B",
	   CFG_METHOD_1,
	   (Reserved2_data << Reserved2_shift) | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_4k),

	_R("RTL8168B/8111B",
	   CFG_METHOD_2,
	   (Reserved2_data << Reserved2_shift) | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_4k),

	_R("RTL8168B/8111B",
	   CFG_METHOD_3,
	   (Reserved2_data << Reserved2_shift) | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_4k),

	_R("RTL8168C/8111C",
	   CFG_METHOD_4, RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),	

	_R("RTL8168C/8111C",
	   CFG_METHOD_5,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),	

	_R("RTL8168C/8111C",
	   CFG_METHOD_6,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),

	_R("RTL8168CP/8111CP",
	   CFG_METHOD_7,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),

	_R("RTL8168CP/8111CP",
	   CFG_METHOD_8,
	   RxCfg_128_int_en | RxCfg_fet_multi_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_6k),

	_R("RTL8168D/8111D",
	   CFG_METHOD_9,
	   RxCfg_128_int_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_9k),

	_R("RTL8168D/8111D",
	   CFG_METHOD_10,
	   RxCfg_128_int_en | (RX_DMA_BURST << RxCfgDMAShift),
	   0xff7e1880,
	   Jumbo_Frame_9k),
};
#undef _R

static struct pci_device_id rtl8168_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8168), },
	{0,},
};

MODULE_DEVICE_TABLE(pci, rtl8168_pci_tbl);

static int rx_copybreak = 200;
static int use_dac;
static struct {
	u32 msg_enable;
} debug = { -1 };

/* media options */
#define MAX_UNITS 8
static int speed[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int duplex[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int autoneg[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };

MODULE_AUTHOR("Realtek and the Linux r8168 crew <netdev@vger.kernel.org>");
MODULE_DESCRIPTION("RealTek RTL-8168 Gigabit Ethernet driver");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
MODULE_PARM(speed, "1-" __MODULE_STRING(MAX_UNITS) "i");
MODULE_PARM(duplex, "1-" __MODULE_STRING(MAX_UNITS) "i");
MODULE_PARM(autoneg, "1-" __MODULE_STRING(MAX_UNITS) "i");
#else
static int num_speed = 0;
static int num_duplex = 0;
static int num_autoneg = 0;

module_param_array(speed, int, &num_speed, 0);
module_param_array(duplex, int, &num_duplex, 0);
module_param_array(autoneg, int, &num_autoneg, 0);
#endif

MODULE_PARM_DESC(speed, "force phy operation. Deprecated by ethtool (8).");
MODULE_PARM_DESC(duplex, "force phy operation. Deprecated by ethtool (8).");
MODULE_PARM_DESC(autoneg, "force phy operation. Deprecated by ethtool (8).");

module_param(rx_copybreak, int, 0);
MODULE_PARM_DESC(rx_copybreak, "Copy breakpoint for copy-only-tiny-frames");
module_param(use_dac, int, 0);
MODULE_PARM_DESC(use_dac, "Enable PCI DAC. Unsafe on 32 bit PCI slot.");

module_param_named(debug, debug.msg_enable, int, 0);
MODULE_PARM_DESC(debug, "Debug verbosity level (0=none, ..., 16=all)");

MODULE_LICENSE("GPL");

MODULE_VERSION(RTL8168_VERSION);

static void rtl8168_dsm(struct net_device *dev, int dev_state);

static void rtl8168_esd_timer(unsigned long __opaque);
static void rtl8168_link_timer(unsigned long __opaque);
static void rtl8168_tx_clear(struct rtl8168_private *tp);
static void rtl8168_rx_clear(struct rtl8168_private *tp);

static int rtl8168_open(struct net_device *dev);
static int rtl8168_start_xmit(struct sk_buff *skb, struct net_device *dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8168_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#else
static irqreturn_t rtl8168_interrupt(int irq, void *dev_instance);
#endif
static int rtl8168_init_ring(struct net_device *dev);
static void rtl8168_hw_start(struct net_device *dev);
static int rtl8168_close(struct net_device *dev);
static void rtl8168_set_rx_mode(struct net_device *dev);
static void rtl8168_tx_timeout(struct net_device *dev);
static struct net_device_stats *rtl8168_get_stats(struct net_device *dev);
static int rtl8168_rx_interrupt(struct net_device *, struct rtl8168_private *, void __iomem *, u32 budget);
static int rtl8168_change_mtu(struct net_device *dev, int new_mtu);
static void rtl8168_down(struct net_device *dev);

static int rtl8168_set_mac_address(struct net_device *dev, void *p);
void rtl8168_rar_set(struct rtl8168_private *tp, uint8_t *addr, uint32_t index);
static void rtl8168_tx_desc_init(struct rtl8168_private *tp);
static void rtl8168_rx_desc_init(struct rtl8168_private *tp);

static void rtl8168_nic_reset(struct net_device *dev);

static void rtl8168_phy_power_up (struct net_device *dev);
static void rtl8168_phy_power_down (struct net_device *dev);
static int rtl8168_set_speed(struct net_device *dev, u8 autoneg,  u16 speed, u8 duplex);

#ifdef CONFIG_R8168_NAPI
static int rtl8168_poll(napi_ptr napi, napi_budget budget);
#endif

static u16 rtl8168_intr_mask =
	SYSErr | LinkChg | RxDescUnavail | TxErr | TxOK | RxErr | RxOK;
static const u16 rtl8168_napi_event =
	RxOK | RxDescUnavail | RxFIFOOver | TxOK | TxErr;

//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3)
#if (( LINUX_VERSION_CODE < KERNEL_VERSION(2,4,27) ) || \
     (( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) ) && \
      ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3) )))
/* copied from linux kernel 2.6.20 include/linux/netdev.h */
#define	NETDEV_ALIGN		32
#define	NETDEV_ALIGN_CONST	(NETDEV_ALIGN - 1)

static inline void *netdev_priv(struct net_device *dev)
{
	return (char *)dev + ((sizeof(struct net_device)
					+ NETDEV_ALIGN_CONST)
				& ~NETDEV_ALIGN_CONST);
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3)


//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
#ifndef netif_msg_init
#define netif_msg_init _kc_netif_msg_init
/* copied from linux kernel 2.6.20 include/linux/netdevice.h */
static inline u32 netif_msg_init(int debug_value, int default_msg_enable_bits)
{
	/* use default */
	if (debug_value < 0 || debug_value >= (sizeof(u32) * 8))
		return default_msg_enable_bits;
	if (debug_value == 0)	/* no output */
		return 0;
	/* set low N bits */
	return (1 << debug_value) - 1;
}

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
static inline void eth_copy_and_sum (struct sk_buff *dest,
				     const unsigned char *src,
				     int len, int base)
{
	memcpy (dest->data, src, len);
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
/* copied from linux kernel 2.6.20 /include/linux/time.h */
/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC	1000L

/* copied from linux kernel 2.6.20 /include/linux/jiffies.h */
/*
 * Change timeval to jiffies, trying to avoid the
 * most obvious overflows..
 *
 * And some not so obvious.
 *
 * Note that we don't want to return MAX_LONG, because
 * for various timeout reasons we often end up having
 * to wait "jiffies+1" in order to guarantee that we wait
 * at _least_ "jiffies" - so "jiffies+1" had better still
 * be positive.
 */
#define MAX_JIFFY_OFFSET ((~0UL >> 1)-1)

/*
 * Convert jiffies to milliseconds and back.
 *
 * Avoid unnecessary multiplications/divisions in the
 * two most common HZ cases:
 */
static inline unsigned int _kc_jiffies_to_msecs(const unsigned long j)
{
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
	return (MSEC_PER_SEC / HZ) * j;
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
	return (j + (HZ / MSEC_PER_SEC) - 1)/(HZ / MSEC_PER_SEC);
#else
	return (j * MSEC_PER_SEC) / HZ;
#endif
}

static inline unsigned long _kc_msecs_to_jiffies(const unsigned int m)
{
	if (m > _kc_jiffies_to_msecs(MAX_JIFFY_OFFSET))
		return MAX_JIFFY_OFFSET;
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
	return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
	return m * (HZ / MSEC_PER_SEC);
#else
	return (m * HZ + MSEC_PER_SEC - 1) / MSEC_PER_SEC;
#endif
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

/* copied from linux kernel 2.6.12.6 /include/linux/pm.h */
typedef int __bitwise pci_power_t;

/* copied from linux kernel 2.6.12.6 /include/linux/pci.h */
typedef u32 __bitwise pm_message_t;

#define PCI_D0	((pci_power_t __force) 0)
#define PCI_D1	((pci_power_t __force) 1)
#define PCI_D2	((pci_power_t __force) 2)
#define PCI_D3hot	((pci_power_t __force) 3)
#define PCI_D3cold	((pci_power_t __force) 4)
#define PCI_POWER_ERROR	((pci_power_t __force) -1)

/* copied from linux kernel 2.6.12.6 /drivers/pci/pci.c */
/**
 * pci_choose_state - Choose the power state of a PCI device
 * @dev: PCI device to be suspended
 * @state: target sleep state for the whole system. This is the value
 *	that is passed to suspend() function.
 *
 * Returns PCI power state suitable for given device and given system
 * message.
 */

pci_power_t pci_choose_state(struct pci_dev *dev, pm_message_t state)
{
	if (!pci_find_capability(dev, PCI_CAP_ID_PM))
		return PCI_D0;

	switch (state) {
	case 0: return PCI_D0;
	case 3: return PCI_D3hot;
	default:
		printk("They asked me for state %d\n", state);
//		BUG();
	}
	return PCI_D0;
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
/**
 * msleep_interruptible - sleep waiting for waitqueue interruptions
 * @msecs: Time in milliseconds to sleep for
 */
#define msleep_interruptible _kc_msleep_interruptible
unsigned long _kc_msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = _kc_msecs_to_jiffies(msecs);

	while (timeout && !signal_pending(current)) {
		set_current_state(TASK_INTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
	return _kc_jiffies_to_msecs(timeout);
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
/* copied from linux kernel 2.6.20 include/linux/sched.h */
#ifndef __sched
#define __sched		__attribute__((__section__(".sched.text")))
#endif

/* copied from linux kernel 2.6.20 kernel/timer.c */
signed long __sched schedule_timeout_uninterruptible(signed long timeout)
{
	__set_current_state(TASK_UNINTERRUPTIBLE);
	return schedule_timeout(timeout);
}

/* copied from linux kernel 2.6.20 include/linux/mii.h */
#undef if_mii
#define if_mii _kc_if_mii
static inline struct mii_ioctl_data *if_mii(struct ifreq *rq)
{
	return (struct mii_ioctl_data *) &rq->ifr_ifru;
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)

static void 
mdio_write(void __iomem *ioaddr, 
	   int RegAddr, 
	   int value)
{
	int i;

	RTL_W32(PHYAR, PHYAR_Write | 
		(RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift | 
		(value & PHYAR_Data_Mask));

	for (i = 0; i < 10; i++) {
		/* Check if the RTL8168 has completed writing to the specified MII register */
		if (!(RTL_R32(PHYAR) & PHYAR_Flag)) 
			break;
		udelay(100);
	}
}

static int 
mdio_read(void __iomem *ioaddr, 
	  int RegAddr)
{
	int i, value = -1;

	RTL_W32(PHYAR, 
		PHYAR_Read | (RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift);

	for (i = 0; i < 10; i++) {
		/* Check if the RTL8168 has completed retrieving data from the specified MII register */
		if (RTL_R32(PHYAR) & PHYAR_Flag) {
			value = (int) (RTL_R32(PHYAR) & PHYAR_Data_Mask);
			break;
		}
		udelay(100);
	}
	return value;
}

static void
rtl8168_ephy_write(void __iomem *ioaddr, 
		   int RegAddr, 
		   int value)
{
	int i;

	RTL_W32(EPHYAR, 
		EPHYAR_Write | 
		(RegAddr & EPHYAR_Reg_Mask) << EPHYAR_Reg_shift | 
		(value & EPHYAR_Data_Mask));

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed EPHY write */
		if (!(RTL_R32(EPHYAR) & EPHYAR_Flag)) 
			break;
	}

	udelay(20);
}

static u16
rtl8168_ephy_read(void __iomem *ioaddr, 
		  int RegAddr)
{
	int i;
	u16 value = 0xffff;

	RTL_W32(EPHYAR, 
		EPHYAR_Read | (RegAddr & EPHYAR_Reg_Mask) << EPHYAR_Reg_shift);

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed EPHY read */
		if (RTL_R32(EPHYAR) & EPHYAR_Flag) {
			value = (u16) (RTL_R32(EPHYAR) & EPHYAR_Data_Mask);
			break;
		}
	}

	udelay(20);

	return value;
}

static void
rtl8168_csi_write(void __iomem *ioaddr, 
		   int addr,
		   int value)
{
	int i;

	RTL_W32(CSIDR, value);
	RTL_W32(CSIAR, 
		CSIAR_Write |
		CSIAR_ByteEn << CSIAR_ByteEn_shift |
		(addr & CSIAR_Addr_Mask));

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed CSI write */
		if (!(RTL_R32(CSIAR) & CSIAR_Flag)) 
			break;
	}

	udelay(20);
}

int 
rtl8168_eri_read(void __iomem *ioaddr, 
		 int addr,
		 int len,
		 int type)
{
	int i, val_shift, shift = 0;
	u32 value1 = 0, value2 = 0, mask;

	if (len > 4 || len <= 0)
		return -1;

	while (len > 0) {
		val_shift = addr % ERIAR_Addr_Align;
		addr = addr & ~0x3;

		RTL_W32(ERIAR, 
			ERIAR_Read |
			type << ERIAR_Type_shift |
			ERIAR_ByteEn << ERIAR_ByteEn_shift |
			addr);

		for (i = 0; i < 10; i++) {
			udelay(100);

			/* Check if the RTL8168 has completed ERI read */
			if (RTL_R32(ERIAR) & ERIAR_Flag) 
				break;
		}

		if (len == 1)		mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 2)	mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 3)	mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else			mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

		value1 = RTL_R32(ERIDR) & mask;
		value2 |= (value1 >> val_shift * 8) << shift * 8;

		if (len <= 4 - val_shift)
			len = 0;
		else {
			len -= (4 - val_shift);
			shift = 4 - val_shift;
			addr += 4;
		}
	}

	return value2;
}

int
rtl8168_eri_write(void __iomem *ioaddr, 
		  int addr,
		  int len,
		  int value,
		  int type)
{

	int i, val_shift, shift = 0;
	u32 value1 = 0, mask;

	if (len > 4 || len <= 0)
		return -1;

	while(len > 0) {
		val_shift = addr % ERIAR_Addr_Align;
		addr = addr & ~0x3;

		if (len == 1)		mask = (0xFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 2)	mask = (0xFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else if (len == 3)	mask = (0xFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;
		else			mask = (0xFFFFFFFF << (val_shift * 8)) & 0xFFFFFFFF;

		value1 = rtl8168_eri_read(ioaddr, addr, 4, type) & ~mask;
		value1 |= (((value << val_shift * 8) >> shift * 8));

		RTL_W32(ERIDR, value1);
		RTL_W32(ERIAR, 
			ERIAR_Write |
			type << ERIAR_Type_shift |
			ERIAR_ByteEn << ERIAR_ByteEn_shift |
			addr);

		for (i = 0; i < 10; i++) {
			udelay(100);

			/* Check if the RTL8168 has completed ERI write */
			if (!(RTL_R32(ERIAR) & ERIAR_Flag)) 
				break;
		}

		if (len <= 4 - val_shift)
			len = 0;
		else {
			len -= (4 - val_shift);
			shift = 4 - val_shift;
			addr += 4;
		}
	}

	return 0;
}

static int 
rtl8168_csi_read(void __iomem *ioaddr, 
		 int addr)
{
	int i, value = -1;

	RTL_W32(CSIAR, 
		CSIAR_Read | 
		CSIAR_ByteEn << CSIAR_ByteEn_shift |
		(addr & CSIAR_Addr_Mask));

	for (i = 0; i < 10; i++) {
		udelay(100);

		/* Check if the RTL8168 has completed CSI read */
		if (RTL_R32(CSIAR) & CSIAR_Flag) {
			value = (int)RTL_R32(CSIDR);
			break;
		}
	}

	udelay(20);

	return value;
}

static void 
rtl8168_irq_mask_and_ack(void __iomem *ioaddr)
{
	RTL_W16(IntrMask, 0x0000);
}

static void 
rtl8168_asic_down(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	rtl8168_nic_reset(dev);
	rtl8168_irq_mask_and_ack(ioaddr);
}

static void 
rtl8168_nic_reset(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	int i;

	if ((tp->mcfg != CFG_METHOD_1) && 
	    (tp->mcfg != CFG_METHOD_2) &&
	    (tp->mcfg != CFG_METHOD_3)) {
		RTL_W8(ChipCmd, StopReq | CmdRxEnb | CmdTxEnb);
		udelay(100);
	}

	/* Soft reset the chip. */
	RTL_W8(ChipCmd, CmdReset);

	/* Check that the chip has finished the reset. */
	for (i = 1000; i > 0; i--) {
		if ((RTL_R8(ChipCmd) & CmdReset) == 0)
			break;
		udelay(100);
	}
}

static unsigned int 
rtl8168_xmii_reset_pending(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;
	unsigned int retval;

	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1f, 0x0000);
	retval = mdio_read(ioaddr, MII_BMCR) & BMCR_RESET;
	spin_unlock_irqrestore(&tp->phy_lock, flags);

	return retval;
}

static unsigned int 
rtl8168_xmii_link_ok(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	mdio_write(ioaddr, 0x1f, 0x0000);

	return RTL_R8(PHYstatus) & LinkStatus;
}

static void 
rtl8168_xmii_reset_enable(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;
	int i;

	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1f, 0x0000);
	mdio_write(ioaddr, MII_BMCR, mdio_read(ioaddr, MII_BMCR) | BMCR_RESET);

	for(i = 0; i < 2500; i++) {
		if(!(mdio_read(ioaddr, MII_BMSR) & BMCR_RESET))
			return;

		mdelay(1);
	}
	spin_unlock_irqrestore(&tp->phy_lock, flags);
}

static void 
rtl8168_check_link_status(struct net_device *dev,
			  struct rtl8168_private *tp, 
			  void __iomem *ioaddr)
{
	unsigned long flags;

	spin_lock_irqsave(&tp->lock, flags);
	if (tp->link_ok(dev)) {
		netif_carrier_on(dev);
		if (netif_msg_ifup(tp))
			printk(KERN_INFO PFX "%s: link up\n", dev->name);
	} else {
		if (netif_msg_ifdown(tp))
			printk(KERN_INFO PFX "%s: link down\n", dev->name);
		netif_carrier_off(dev);
	}
	spin_unlock_irqrestore(&tp->lock, flags);
}

static void 
rtl8168_link_option(int idx, 
		    u8 *aut, 
		    u16 *spd, 
		    u8 *dup)
{
	unsigned char opt_speed;
	unsigned char opt_duplex;
	unsigned char opt_autoneg;

	opt_speed = ((idx < MAX_UNITS) && (idx >= 0)) ? speed[idx] : 0xff;
	opt_duplex = ((idx < MAX_UNITS) && (idx >= 0)) ? duplex[idx] : 0xff;
	opt_autoneg = ((idx < MAX_UNITS) && (idx >= 0)) ? autoneg[idx] : 0xff;

	if ((opt_speed == 0xff) |
	    (opt_duplex == 0xff) |
	    (opt_autoneg == 0xff)) {
		*spd = SPEED_1000;
		*dup = DUPLEX_FULL;
		*aut = AUTONEG_ENABLE;
	} else {
		*spd = speed[idx];
		*dup = duplex[idx];
		*aut = autoneg[idx];	
	}
}

static void
rtl8168_powerdown_pll(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	if (tp->wol_enabled == WOL_ENABLED)
		return;

	rtl8168_phy_power_down(dev);

	switch (tp->mcfg) {
	case CFG_METHOD_9:
	case CFG_METHOD_10:
		RTL_W8(PMCH, RTL_R8(PMCH) & ~BIT_7);
		break;
	}
}

static void
rtl8168_powerup_pll(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	switch (tp->mcfg) {
	case CFG_METHOD_9:
	case CFG_METHOD_10:
		RTL_W8(PMCH, RTL_R8(PMCH) | BIT_7);
		break;
	}

	rtl8168_phy_power_up(dev);
	rtl8168_set_speed(dev, tp->autoneg, tp->speed, tp->duplex);
}

static void 
rtl8168_get_wol(struct net_device *dev, 
		struct ethtool_wolinfo *wol)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	u8 options;

	wol->wolopts = 0;

#define WAKE_ANY (WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST)
	wol->supported = WAKE_ANY;

	spin_lock_irq(&tp->lock);

	options = RTL_R8(Config1);
	if (!(options & PMEnable))
		goto out_unlock;

	options = RTL_R8(Config3);
	if (options & LinkUp)
		wol->wolopts |= WAKE_PHY;
	if (options & MagicPacket)
		wol->wolopts |= WAKE_MAGIC;

	options = RTL_R8(Config5);
	if (options & UWF)
		wol->wolopts |= WAKE_UCAST;
	if (options & BWF)
	        wol->wolopts |= WAKE_BCAST;
	if (options & MWF)
	        wol->wolopts |= WAKE_MCAST;

out_unlock:
	spin_unlock_irq(&tp->lock);
}

static int 
rtl8168_set_wol(struct net_device *dev, 
		struct ethtool_wolinfo *wol)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	int i;
	static struct {
		u32 opt;
		u16 reg;
		u8  mask;
	} cfg[] = {
		{ WAKE_ANY,   Config1, PMEnable },
		{ WAKE_PHY,   Config3, LinkUp },
		{ WAKE_MAGIC, Config3, MagicPacket },
		{ WAKE_UCAST, Config5, UWF },
		{ WAKE_BCAST, Config5, BWF },
		{ WAKE_MCAST, Config5, MWF },
		{ WAKE_ANY,   Config5, LanWake }
	};

	spin_lock_irq(&tp->lock);

	RTL_W8(Cfg9346, Cfg9346_Unlock);

	for (i = 0; i < ARRAY_SIZE(cfg); i++) {
		u8 options = RTL_R8(cfg[i].reg) & ~cfg[i].mask;
		if (wol->wolopts & cfg[i].opt)
			options |= cfg[i].mask;
		RTL_W8(cfg[i].reg, options);
	}

	RTL_W8(Cfg9346, Cfg9346_Lock);

	tp->wol_enabled = (wol->wolopts) ? WOL_ENABLED : WOL_DISABLED;

	spin_unlock_irq(&tp->lock);

	return 0;
}

static void 
rtl8168_get_drvinfo(struct net_device *dev,
		    struct ethtool_drvinfo *info)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	strcpy(info->driver, MODULENAME);
	strcpy(info->version, RTL8168_VERSION);
	strcpy(info->bus_info, pci_name(tp->pci_dev));
}

static int 
rtl8168_get_regs_len(struct net_device *dev)
{
	return R8168_REGS_SIZE;
}

static int 
rtl8168_set_speed_xmii(struct net_device *dev,
		       u8 autoneg, 
		       u16 speed, 
		       u8 duplex)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	int auto_nego = 0;
	int giga_ctrl = 0;
	int bmcr_true_force = 0;
	unsigned long flags;

	if ((speed != SPEED_1000) && 
	    (speed != SPEED_100) && 
	    (speed != SPEED_10)) {
		speed = SPEED_1000;
		duplex = DUPLEX_FULL;
	}

	if ((autoneg == AUTONEG_ENABLE) || (speed == SPEED_1000)) {
		/*n-way force*/
		if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
			auto_nego |= ADVERTISE_10HALF;
		} else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
			auto_nego |= ADVERTISE_10HALF |
				     ADVERTISE_10FULL;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_100FULL |
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		} else if (speed == SPEED_1000) {
			giga_ctrl |= ADVERTISE_1000HALF | 
				     ADVERTISE_1000FULL;

			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_100FULL |
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		}

		//disable flow contorol
		auto_nego &= ~ADVERTISE_PAUSE_CAP;
		auto_nego &= ~ADVERTISE_PAUSE_ASYM;

		tp->phy_auto_nego_reg = auto_nego;
		tp->phy_1000_ctrl_reg = giga_ctrl;

		tp->autoneg = autoneg;
		tp->speed = speed; 
		tp->duplex = duplex; 

		rtl8168_phy_power_up (dev);

		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1f, 0x0000);
		mdio_write(ioaddr, MII_ADVERTISE, auto_nego);
		mdio_write(ioaddr, MII_CTRL1000, giga_ctrl);
		mdio_write(ioaddr, MII_BMCR, BMCR_RESET | BMCR_ANENABLE | BMCR_ANRESTART);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
	} else {
		/*true force*/
#ifndef BMCR_SPEED100
#define BMCR_SPEED100	0x0040
#endif

#ifndef BMCR_SPEED10
#define BMCR_SPEED10	0x0000
#endif
		if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
			bmcr_true_force = BMCR_SPEED10;
		} else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
			bmcr_true_force = BMCR_SPEED10 | BMCR_FULLDPLX;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
			bmcr_true_force = BMCR_SPEED100;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
			bmcr_true_force = BMCR_SPEED100 | BMCR_FULLDPLX;
		}

		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1f, 0x0000);
		mdio_write(ioaddr, MII_BMCR, bmcr_true_force);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
	}

	return 0;
}

static int 
rtl8168_set_speed(struct net_device *dev,
		  u8 autoneg, 
		  u16 speed, 
		  u8 duplex)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	int ret;

	ret = tp->set_speed(dev, autoneg, speed, duplex);

	return ret;
}

static int 
rtl8168_set_settings(struct net_device *dev, 
		     struct ethtool_cmd *cmd)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&tp->lock, flags);
	ret = rtl8168_set_speed(dev, cmd->autoneg, cmd->speed, cmd->duplex);
	spin_unlock_irqrestore(&tp->lock, flags);
	
	return ret;
}

static u32 
rtl8168_get_tx_csum(struct net_device *dev)
{
	return (dev->features & NETIF_F_IP_CSUM) != 0;
}

static u32 
rtl8168_get_rx_csum(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	return tp->cp_cmd & RxChkSum;
}

static int 
rtl8168_set_tx_csum(struct net_device *dev, 
		    u32 data)
{
	if (data)
		dev->features |= NETIF_F_IP_CSUM;
	else
		dev->features &= ~NETIF_F_IP_CSUM;

	return 0;
}

static int 
rtl8168_set_rx_csum(struct net_device *dev, 
		    u32 data)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;

	spin_lock_irqsave(&tp->lock, flags);

	if (data)
		tp->cp_cmd |= RxChkSum;
	else
		tp->cp_cmd &= ~RxChkSum;

	RTL_W16(CPlusCmd, tp->cp_cmd);

	spin_unlock_irqrestore(&tp->lock, flags);

	return 0;
}

#ifdef CONFIG_R8168_VLAN

static inline u32 
rtl8168_tx_vlan_tag(struct rtl8168_private *tp,
		    struct sk_buff *skb)
{
	return (tp->vlgrp && vlan_tx_tag_present(skb)) ?
		TxVlanTag | swab16(vlan_tx_tag_get(skb)) : 0x00;
}

static void 
rtl8168_vlan_rx_register(struct net_device *dev,
			 struct vlan_group *grp)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;

	spin_lock_irqsave(&tp->lock, flags);
	tp->vlgrp = grp;
	if (tp->vlgrp)
		tp->cp_cmd |= RxVlan;
	else
		tp->cp_cmd &= ~RxVlan;
	RTL_W16(CPlusCmd, tp->cp_cmd);
	RTL_R16(CPlusCmd);
	spin_unlock_irqrestore(&tp->lock, flags);
}

static void 
rtl8168_vlan_rx_kill_vid(struct net_device *dev, 
			 unsigned short vid)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	unsigned long flags;

	spin_lock_irqsave(&tp->lock, flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
	if (tp->vlgrp)
		tp->vlgrp->vlan_devices[vid] = NULL;
#else
	vlan_group_set_device(tp->vlgrp, vid, NULL);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
	spin_unlock_irqrestore(&tp->lock, flags);
}

static int 
rtl8168_rx_vlan_skb(struct rtl8168_private *tp, 
		    struct RxDesc *desc,
		    struct sk_buff *skb)
{
	u32 opts2 = le32_to_cpu(desc->opts2);
	int ret;

	if (tp->vlgrp && (opts2 & RxVlanTag)) {
		rtl8168_rx_hwaccel_skb(skb, tp->vlgrp,
				       swab16(opts2 & 0xffff));
		ret = 0;
	} else
		ret = -1;
	desc->opts2 = 0;
	return ret;
}

#else /* !CONFIG_R8168_VLAN */

static inline u32 
rtl8168_tx_vlan_tag(struct rtl8168_private *tp,
		    struct sk_buff *skb)
{
	return 0;
}

static int 
rtl8168_rx_vlan_skb(struct rtl8168_private *tp, 
		    struct RxDesc *desc,
		    struct sk_buff *skb)
{
	return -1;
}

#endif

static void rtl8168_gset_xmii(struct net_device *dev, 
		  struct ethtool_cmd *cmd)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	u8 status;

	cmd->supported = SUPPORTED_10baseT_Half |
			 SUPPORTED_10baseT_Full |
			 SUPPORTED_100baseT_Half |
			 SUPPORTED_100baseT_Full |
			 SUPPORTED_1000baseT_Full |
			 SUPPORTED_Autoneg |
		         SUPPORTED_TP;

	cmd->autoneg = (mdio_read(ioaddr, MII_BMCR) & BMCR_ANENABLE) ? 1 : 0;
	cmd->advertising = ADVERTISED_TP | ADVERTISED_Autoneg;

	if (tp->phy_auto_nego_reg & ADVERTISE_10HALF)
		cmd->advertising |= ADVERTISED_10baseT_Half;
	if (tp->phy_auto_nego_reg & ADVERTISE_10FULL)
		cmd->advertising |= ADVERTISED_10baseT_Full;
	if (tp->phy_auto_nego_reg & ADVERTISE_100HALF)
		cmd->advertising |= ADVERTISED_100baseT_Half;
	if (tp->phy_auto_nego_reg & ADVERTISE_100FULL)
		cmd->advertising |= ADVERTISED_100baseT_Full;
	if (tp->phy_1000_ctrl_reg & ADVERTISE_1000FULL)
		cmd->advertising |= ADVERTISED_1000baseT_Full;

	status = RTL_R8(PHYstatus);

	if (status & _1000bpsF)
		cmd->speed = SPEED_1000;
	else if (status & _100bps)
		cmd->speed = SPEED_100;
	else if (status & _10bps)
		cmd->speed = SPEED_10;

	if (status & TxFlowCtrl)
		cmd->advertising |= ADVERTISED_Asym_Pause;

	if (status & RxFlowCtrl)
		cmd->advertising |= ADVERTISED_Pause;

	cmd->duplex = ((status & _1000bpsF) || (status & FullDup)) ?
		      DUPLEX_FULL : DUPLEX_HALF;

	tp->autoneg = cmd->autoneg; 
	tp->speed = cmd->speed; 
	tp->duplex = cmd->duplex; 

}

static int 
rtl8168_get_settings(struct net_device *dev, 
		     struct ethtool_cmd *cmd)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	unsigned long flags;

	spin_lock_irqsave(&tp->lock, flags);

	tp->get_settings(dev, cmd);

	spin_unlock_irqrestore(&tp->lock, flags);
	return 0;
}

static void rtl8168_get_regs(struct net_device *dev, struct ethtool_regs *regs,
			     void *p)
{
        struct rtl8168_private *tp = netdev_priv(dev);
        unsigned long flags;

        if (regs->len > R8168_REGS_SIZE)
        	regs->len = R8168_REGS_SIZE;

        spin_lock_irqsave(&tp->lock, flags);
        memcpy_fromio(p, tp->mmio_addr, regs->len);
        spin_unlock_irqrestore(&tp->lock, flags);
}

static u32 
rtl8168_get_msglevel(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	return tp->msg_enable;
}

static void 
rtl8168_set_msglevel(struct net_device *dev, 
		     u32 value)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	tp->msg_enable = value;
}

static const char rtl8168_gstrings[][ETH_GSTRING_LEN] = {
	"tx_packets",
	"rx_packets",
	"tx_errors",
	"rx_errors",
	"rx_missed",
	"align_errors",
	"tx_single_collisions",
	"tx_multi_collisions",
	"unicast",
	"broadcast",
	"multicast",
	"tx_aborted",
	"tx_underrun",
};

struct rtl8168_counters {
	u64	tx_packets;
	u64	rx_packets;
	u64	tx_errors;
	u32	rx_errors;
	u16	rx_missed;
	u16	align_errors;
	u32	tx_one_collision;
	u32	tx_multi_collision;
	u64	rx_unicast;
	u64	rx_broadcast;
	u32	rx_multicast;
	u16	tx_aborted;
	u16	tx_underun;
};

static int 
rtl8168_get_stats_count(struct net_device *dev)
{
	return ARRAY_SIZE(rtl8168_gstrings);
}

static void 
rtl8168_get_ethtool_stats(struct net_device *dev,
			  struct ethtool_stats *stats, 
			  u64 *data)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	struct rtl8168_counters *counters;
	dma_addr_t paddr;
	u32 cmd;

	ASSERT_RTNL();

	counters = pci_alloc_consistent(tp->pci_dev, sizeof(*counters), &paddr);
	if (!counters)
		return;

	RTL_W32(CounterAddrHigh, (u64)paddr >> 32);
	cmd = (u64)paddr & DMA_32BIT_MASK;
	RTL_W32(CounterAddrLow, cmd);
	RTL_W32(CounterAddrLow, cmd | CounterDump);

	while (RTL_R32(CounterAddrLow) & CounterDump) {
		if (msleep_interruptible(1))
			break;
	}

	RTL_W32(CounterAddrLow, 0);
	RTL_W32(CounterAddrHigh, 0);

	data[0]	= le64_to_cpu(counters->tx_packets);
	data[1] = le64_to_cpu(counters->rx_packets);
	data[2] = le64_to_cpu(counters->tx_errors);
	data[3] = le32_to_cpu(counters->rx_errors);
	data[4] = le16_to_cpu(counters->rx_missed);
	data[5] = le16_to_cpu(counters->align_errors);
	data[6] = le32_to_cpu(counters->tx_one_collision);
	data[7] = le32_to_cpu(counters->tx_multi_collision);
	data[8] = le64_to_cpu(counters->rx_unicast);
	data[9] = le64_to_cpu(counters->rx_broadcast);
	data[10] = le32_to_cpu(counters->rx_multicast);
	data[11] = le16_to_cpu(counters->tx_aborted);
	data[12] = le16_to_cpu(counters->tx_underun);
pci_free_consistent(tp->pci_dev, sizeof(*counters), counters, paddr);
}

static void 
rtl8168_get_strings(struct net_device *dev, 
		    u32 stringset, 
		    u8 *data)
{
	switch(stringset) {
	case ETH_SS_STATS:
		memcpy(data, *rtl8168_gstrings, sizeof(rtl8168_gstrings));
		break;
	}
}

#undef ethtool_op_get_link
#define ethtool_op_get_link _kc_ethtool_op_get_link
u32 _kc_ethtool_op_get_link(struct net_device *dev)
{
	return netif_carrier_ok(dev) ? 1 : 0;
}

#undef ethtool_op_get_sg
#define ethtool_op_get_sg _kc_ethtool_op_get_sg
u32 _kc_ethtool_op_get_sg(struct net_device *dev)
{
#ifdef NETIF_F_SG
	return (dev->features & NETIF_F_SG) != 0;
#else
	return 0;
#endif
}

#undef ethtool_op_set_sg
#define ethtool_op_set_sg _kc_ethtool_op_set_sg
int _kc_ethtool_op_set_sg(struct net_device *dev, u32 data)
{
#ifdef NETIF_F_SG
	if (data)
		dev->features |= NETIF_F_SG;
	else
		dev->features &= ~NETIF_F_SG;
#endif

	return 0;
}

static struct ethtool_ops rtl8168_ethtool_ops = {
	.get_drvinfo		= rtl8168_get_drvinfo,
	.get_regs_len		= rtl8168_get_regs_len,
	.get_link		= ethtool_op_get_link,
	.get_settings		= rtl8168_get_settings,
	.set_settings		= rtl8168_set_settings,
	.get_msglevel		= rtl8168_get_msglevel,
	.set_msglevel		= rtl8168_set_msglevel,
	.get_rx_csum		= rtl8168_get_rx_csum,
	.set_rx_csum		= rtl8168_set_rx_csum,
	.get_tx_csum		= rtl8168_get_tx_csum,
	.set_tx_csum		= rtl8168_set_tx_csum,
	.get_sg			= ethtool_op_get_sg,
	.set_sg			= ethtool_op_set_sg,
#ifdef NETIF_F_TSO
	.get_tso		= ethtool_op_get_tso,
	.set_tso		= ethtool_op_set_tso,
#endif
	.get_regs		= rtl8168_get_regs,
	.get_wol		= rtl8168_get_wol,
	.set_wol		= rtl8168_set_wol,
	.get_strings		= rtl8168_get_strings,
	.get_stats_count	= rtl8168_get_stats_count,
	.get_ethtool_stats	= rtl8168_get_ethtool_stats,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#ifdef ETHTOOL_GPERMADDR 
	.get_perm_addr		= ethtool_op_get_perm_addr,
#endif
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
};

static void 
rtl8168_get_mac_version(struct rtl8168_private *tp, 
			void __iomem *ioaddr)
{
	u32 reg,val32;
	u32 ICVerID;
		
	val32 = RTL_R32(TxConfig)  ;	
	reg = val32 & 0x7c800000;
	ICVerID = val32 & 0x00700000;

	switch(reg) {
		case 0x30000000:
			tp->mcfg = CFG_METHOD_1;
			break;
		case 0x38000000:
			if(ICVerID == 0x00000000) {
				tp->mcfg = CFG_METHOD_2;
			} else if(ICVerID == 0x00500000) {
				tp->mcfg = CFG_METHOD_3;
			} else {
				tp->mcfg = CFG_METHOD_3;
			}
			break;
		case 0x3C000000:
			if(ICVerID == 0x00000000) {
				tp->mcfg = CFG_METHOD_4;
			} else if(ICVerID == 0x00200000) {
				tp->mcfg = CFG_METHOD_5;
			} else if(ICVerID == 0x00400000) {
				tp->mcfg = CFG_METHOD_6;
			} else {
				tp->mcfg = CFG_METHOD_6;
			}
			break;
		case 0x3C800000:
			if (ICVerID == 0x00100000){
				tp->mcfg = CFG_METHOD_7;
			} else if (ICVerID == 0x00300000){
				tp->mcfg = CFG_METHOD_8;
			} else {
				tp->mcfg = CFG_METHOD_8;
			}
			break;
		case 0x28000000:
			if(ICVerID == 0x00100000) {
				tp->mcfg = CFG_METHOD_9;
			} else if(ICVerID == 0x00300000) {
				tp->mcfg = CFG_METHOD_10;
			} else {
				tp->mcfg = CFG_METHOD_10;
			}
			break;
		default:
			tp->mcfg = 0xFFFFFFFF;
			printk("unknown chip version (%x)\n",reg);
			break;
	}
}

static void 
rtl8168_print_mac_version(struct rtl8168_private *tp)
{
	int i;
	for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--) {
		if (tp->mcfg == rtl_chip_info[i].mcfg){
			dprintk("mcfg == %s (%04d)\n", rtl_chip_info[i].name,
				  rtl_chip_info[i].mcfg);			
			return;
		}
	}
	
	dprintk("mac_version == Unknown\n");
}

static void 
rtl8168_hw_phy_config(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;
	unsigned int gphy_val;

	spin_lock_irqsave(&tp->phy_lock, flags);

	if (tp->mcfg == CFG_METHOD_1) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x0B, 0x94B0);

		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x12, 0x6096);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x0D, 0xF8A0);
	} else if (tp->mcfg == CFG_METHOD_2) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x0B, 0x94B0);

		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x12, 0x6096);

		mdio_write(ioaddr, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_3) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x0B, 0x94B0);

		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x12, 0x6096);

		mdio_write(ioaddr, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_4) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x12, 0x2300);
		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x16, 0x000A);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x12, 0xC096);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x00, 0x88DE);
		mdio_write(ioaddr, 0x01, 0x82B1);
		mdio_write(ioaddr, 0x1F, 0x0000);
	
		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x08, 0x9E30);
		mdio_write(ioaddr, 0x09, 0x01F0);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x0A, 0x5500);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x03, 0x7002);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x0C, 0x00C8);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x14, mdio_read(ioaddr, 0x14) | (1 << 5));
		mdio_write(ioaddr, 0x0D, mdio_read(ioaddr, 0x0D) & ~(1 << 5));
	} else if (tp->mcfg == CFG_METHOD_5) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x12, 0x2300);
		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x16, 0x0F0A);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x00, 0x88DE);
		mdio_write(ioaddr, 0x01, 0x82B1);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x0C, 0x7EB8);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x06, 0x0761);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x03, 0x802F);
		mdio_write(ioaddr, 0x02, 0x4F02);
		mdio_write(ioaddr, 0x01, 0x0409);
		mdio_write(ioaddr, 0x00, 0xF099);
		mdio_write(ioaddr, 0x04, 0x9800);
		mdio_write(ioaddr, 0x04, 0x9000);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x16, mdio_read(ioaddr, 0x16) | (1 << 0));

		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x14, mdio_read(ioaddr, 0x14) | (1 << 5));
		mdio_write(ioaddr, 0x0D, mdio_read(ioaddr, 0x0D) & ~(1 << 5));

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x1D, 0x3D98);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x17, 0x0CC0);
		mdio_write(ioaddr, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_6) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x12, 0x2300);
		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x16, 0x0F0A);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x00, 0x88DE);
		mdio_write(ioaddr, 0x01, 0x82B1);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x0C, 0x7EB8);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x06, 0x0761);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x16, mdio_read(ioaddr, 0x16) | (1 << 0));

		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x14, mdio_read(ioaddr, 0x14) | (1 << 5));
		mdio_write(ioaddr, 0x0D, mdio_read(ioaddr, 0x0D) & ~(1 << 5));

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x1D, 0x3D98);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1f, 0x0001);
		mdio_write(ioaddr, 0x17, 0x0CC0);
		mdio_write(ioaddr, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_7) {
		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x14, mdio_read(ioaddr, 0x14) | (1 << 5));
		mdio_write(ioaddr, 0x0D, mdio_read(ioaddr, 0x0D) | (1 << 5));

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x1D, 0x3D98);

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x14, 0xCAA3);
		mdio_write(ioaddr, 0x1C, 0x000A);
		mdio_write(ioaddr, 0x18, 0x65D0);
		
		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x17, 0xB580);
		mdio_write(ioaddr, 0x18, 0xFF54);
		mdio_write(ioaddr, 0x19, 0x3954);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x0D, 0x310C);
		mdio_write(ioaddr, 0x0E, 0x310C);
		mdio_write(ioaddr, 0x0F, 0x311C);
		mdio_write(ioaddr, 0x06, 0x0761);

		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x18, 0xFF55);
		mdio_write(ioaddr, 0x19, 0x3955);
		mdio_write(ioaddr, 0x18, 0xFF54);
		mdio_write(ioaddr, 0x19, 0x3954);

		mdio_write(ioaddr, 0x1f, 0x0001);
		mdio_write(ioaddr, 0x17, 0x0CC0);

		mdio_write(ioaddr, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_8) {
		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x0D, mdio_read(ioaddr, 0x0D) | (1 << 5));

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x14, 0xCAA3);
		mdio_write(ioaddr, 0x1C, 0x000A);
		mdio_write(ioaddr, 0x18, 0x65D0);
		
		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x17, 0xB580);
		mdio_write(ioaddr, 0x18, 0xFF54);
		mdio_write(ioaddr, 0x19, 0x3954);

		mdio_write(ioaddr, 0x1F, 0x0002);
		mdio_write(ioaddr, 0x0D, 0x310C);
		mdio_write(ioaddr, 0x0E, 0x310C);
		mdio_write(ioaddr, 0x0F, 0x311C);
		mdio_write(ioaddr, 0x06, 0x0761);

		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x18, 0xFF55);
		mdio_write(ioaddr, 0x19, 0x3955);
		mdio_write(ioaddr, 0x18, 0xFF54);
		mdio_write(ioaddr, 0x19, 0x3954);

		mdio_write(ioaddr, 0x1f, 0x0001);
		mdio_write(ioaddr, 0x17, 0x0CC0);

		mdio_write(ioaddr, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_9) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x06, 0x4064);
		mdio_write(ioaddr, 0x07, 0x2863);
		mdio_write(ioaddr, 0x08, 0x059C);
		mdio_write(ioaddr, 0x09, 0x26B4);
		mdio_write(ioaddr, 0x0A, 0x6A19);
		mdio_write(ioaddr, 0x0B, 0xACC0);
		mdio_write(ioaddr, 0x10, 0xF06D);
		mdio_write(ioaddr, 0x14, 0x7F68);
		mdio_write(ioaddr, 0x18, 0x7FD9);
		mdio_write(ioaddr, 0x1C, 0xF0FF);
		mdio_write(ioaddr, 0x1D, 0x3D9C);
		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x12, 0xF49F);
		mdio_write(ioaddr, 0x13, 0x070B);
		mdio_write(ioaddr, 0x1A, 0x05AD);
		mdio_write(ioaddr, 0x14, 0x94C0);

		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x0B) & 0xFF00;
		gphy_val |= 0x10;
		mdio_write(ioaddr, 0x0B, gphy_val);
		gphy_val = mdio_read(ioaddr, 0x0C) & 0x00FF;
		gphy_val |= 0xA200;
		mdio_write(ioaddr, 0x0C, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x06);
		gphy_val |= BIT_14;
		gphy_val &= ~BIT_13;
		gphy_val |= BIT_12;
		gphy_val |= BIT_10;
		gphy_val &= ~BIT_9;
		gphy_val |= BIT_8;
		mdio_write(ioaddr, 0x06, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x0D);
		gphy_val |= BIT_9;
		gphy_val |= BIT_8;
		mdio_write(ioaddr, 0x0D, gphy_val);
		gphy_val = mdio_read(ioaddr, 0x0F);
		gphy_val |= BIT_4;
		mdio_write(ioaddr, 0x0F, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x02);
		gphy_val &= ~BIT_10;
		gphy_val &= ~BIT_9;
		gphy_val |= BIT_8;
		mdio_write(ioaddr, 0x02, gphy_val);
		gphy_val = mdio_read(ioaddr, 0x03);
		gphy_val &= ~BIT_15;
		gphy_val &= ~BIT_14;
		gphy_val &= ~BIT_13;
		mdio_write(ioaddr, 0x03, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x17, 0x0CC0);

		mdio_write(ioaddr, 0x1F, 0x0005);
		mdio_write(ioaddr, 0x05, 0x8200);
		mdio_write(ioaddr, 0x06, 0xF8F9);
		mdio_write(ioaddr, 0x06, 0xFAEF);
		mdio_write(ioaddr, 0x06, 0x59EE);
		mdio_write(ioaddr, 0x06, 0xF8EA);
		mdio_write(ioaddr, 0x06, 0x00EE);
		mdio_write(ioaddr, 0x06, 0xF8EB);
		mdio_write(ioaddr, 0x06, 0x00E0);
		mdio_write(ioaddr, 0x06, 0xF87C);
		mdio_write(ioaddr, 0x06, 0xE1F8);
		mdio_write(ioaddr, 0x06, 0x7D59);
		mdio_write(ioaddr, 0x06, 0x0FEF);
		mdio_write(ioaddr, 0x06, 0x0139);
		mdio_write(ioaddr, 0x06, 0x029E);
		mdio_write(ioaddr, 0x06, 0x06EF);
		mdio_write(ioaddr, 0x06, 0x1039);
		mdio_write(ioaddr, 0x06, 0x089F);
		mdio_write(ioaddr, 0x06, 0x2AEE);
		mdio_write(ioaddr, 0x06, 0xF8EA);
		mdio_write(ioaddr, 0x06, 0x00EE);
		mdio_write(ioaddr, 0x06, 0xF8EB);
		mdio_write(ioaddr, 0x06, 0x01E0);
		mdio_write(ioaddr, 0x06, 0xF87C);
		mdio_write(ioaddr, 0x06, 0xE1F8);
		mdio_write(ioaddr, 0x06, 0x7D58);
		mdio_write(ioaddr, 0x06, 0x409E);
		mdio_write(ioaddr, 0x06, 0x0F39);
		mdio_write(ioaddr, 0x06, 0x46AA);
		mdio_write(ioaddr, 0x06, 0x0BBF);
		mdio_write(ioaddr, 0x06, 0x8251);
		mdio_write(ioaddr, 0x06, 0xD682);
		mdio_write(ioaddr, 0x06, 0x5902);
		mdio_write(ioaddr, 0x06, 0x014F);
		mdio_write(ioaddr, 0x06, 0xAE09);
		mdio_write(ioaddr, 0x06, 0xBF82);
		mdio_write(ioaddr, 0x06, 0x59D6);
		mdio_write(ioaddr, 0x06, 0x8261);
		mdio_write(ioaddr, 0x06, 0x0201);
		mdio_write(ioaddr, 0x06, 0x4FEF);
		mdio_write(ioaddr, 0x06, 0x95FE);
		mdio_write(ioaddr, 0x06, 0xFDFC);
		mdio_write(ioaddr, 0x06, 0x054D);
		mdio_write(ioaddr, 0x06, 0x2000);
		mdio_write(ioaddr, 0x06, 0x024E);
		mdio_write(ioaddr, 0x06, 0x2200);
		mdio_write(ioaddr, 0x06, 0x024D);
		mdio_write(ioaddr, 0x06, 0xDFFF);
		mdio_write(ioaddr, 0x06, 0x014E);
		mdio_write(ioaddr, 0x06, 0xDDFF);
		mdio_write(ioaddr, 0x06, 0x0100);
		mdio_write(ioaddr, 0x06, 0x6010);
		mdio_write(ioaddr, 0x05, 0xFFF6);
		mdio_write(ioaddr, 0x06, 0x00EC);
		mdio_write(ioaddr, 0x05, 0x83D4);
		mdio_write(ioaddr, 0x06, 0x8200);
		mdio_write(ioaddr, 0x1F, 0x0000);

		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x0D, 0xF880);
		mdio_write(ioaddr, 0x1F, 0x0000);
	} else if (tp->mcfg == CFG_METHOD_10) {
		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x06, 0x4064);
		mdio_write(ioaddr, 0x07, 0x2863);
		mdio_write(ioaddr, 0x08, 0x059C);
		mdio_write(ioaddr, 0x09, 0x26B4);
		mdio_write(ioaddr, 0x0A, 0x6A19);
		mdio_write(ioaddr, 0x0B, 0xACC0);
		mdio_write(ioaddr, 0x10, 0xF06D);
		mdio_write(ioaddr, 0x14, 0x7F68);
		mdio_write(ioaddr, 0x18, 0x7FD9);
		mdio_write(ioaddr, 0x1C, 0xF0FF);
		mdio_write(ioaddr, 0x1D, 0x3D9C);
		mdio_write(ioaddr, 0x1F, 0x0003);
		mdio_write(ioaddr, 0x12, 0xF49F);
		mdio_write(ioaddr, 0x13, 0x070B);
		mdio_write(ioaddr, 0x1A, 0x05AD);
		mdio_write(ioaddr, 0x14, 0x94C0);
	
		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x06);
		gphy_val |= BIT_14;
		gphy_val &= ~BIT_13;
		gphy_val |= BIT_12;
		gphy_val |= BIT_10;
		gphy_val &= ~BIT_9;
		gphy_val |= BIT_8;
		mdio_write(ioaddr, 0x06, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x05);
		gphy_val &= ~BIT_15;
		gphy_val &= ~BIT_14;
		gphy_val &= ~BIT_7;
		gphy_val |= BIT_6;
		gphy_val &= ~BIT_5;
		mdio_write(ioaddr, 0x05, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x02);
		gphy_val &= ~BIT_10;
		gphy_val &= ~BIT_9;
		gphy_val |= BIT_8;
		mdio_write(ioaddr, 0x02, gphy_val);
		gphy_val = mdio_read(ioaddr, 0x03);
		gphy_val &= ~BIT_15;
		gphy_val &= ~BIT_14;
		gphy_val &= ~BIT_13;
		mdio_write(ioaddr, 0x03, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0001);
		mdio_write(ioaddr, 0x17, 0x0CC0);

		mdio_write(ioaddr, 0x1F, 0x0002);
		gphy_val = mdio_read(ioaddr, 0x0F);
		gphy_val |= BIT_4;
		gphy_val |= BIT_2;
		gphy_val |= BIT_1;
		gphy_val |= BIT_0;
		mdio_write(ioaddr, 0x0F, gphy_val);

		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(ioaddr, 0x0D, 0xF880);
		mdio_write(ioaddr, 0x1F, 0x0000);
	}

	spin_unlock_irqrestore(&tp->phy_lock, flags);
}

static inline void rtl8168_delete_esd_timer(struct net_device *dev, struct timer_list *timer)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	spin_lock_irq(&tp->lock);
	del_timer_sync(timer);
	spin_unlock_irq(&tp->lock);
}

static inline void rtl8168_request_esd_timer(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct timer_list *timer = &tp->esd_timer;

	init_timer(timer);
	timer->expires = jiffies + RTL8168_ESD_TIMEOUT;
	timer->data = (unsigned long)(dev);
	timer->function = rtl8168_esd_timer;
	add_timer(timer);
}

static inline void rtl8168_delete_link_timer(struct net_device *dev, struct timer_list *timer)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	spin_lock_irq(&tp->lock);
	del_timer_sync(timer);
	spin_unlock_irq(&tp->lock);
}

static inline void rtl8168_request_link_timer(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct timer_list *timer = &tp->link_timer;

	init_timer(timer);
	timer->expires = jiffies + RTL8168_LINK_TIMEOUT;
	timer->data = (unsigned long)(dev);
	timer->function = rtl8168_link_timer;
	add_timer(timer);
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void 
rtl8168_netpoll(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;

	disable_irq(pdev->irq);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	rtl8168_interrupt(pdev->irq, dev, NULL);
#else
	rtl8168_interrupt(pdev->irq, dev);
#endif
	enable_irq(pdev->irq);
}
#endif

static void 
rtl8168_release_board(struct pci_dev *pdev, 
		      struct net_device *dev,
		      void __iomem *ioaddr)
{
	iounmap(ioaddr);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	free_netdev(dev);
}

/**
 * rtl8168_set_mac_address - Change the Ethernet Address of the NIC
 * @dev: network interface device structure 
 * @p:   pointer to an address structure
 *
 * Return 0 on success, negative on failure
 **/
static int
rtl8168_set_mac_address(struct net_device *dev, 
			void *p)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	memcpy(tp->mac_addr, addr->sa_data, dev->addr_len);

	rtl8168_rar_set(tp, tp->mac_addr, 0);
	
	return 0;
}

/******************************************************************************
 * rtl8168_rar_set - Puts an ethernet address into a receive address register.
 *
 * tp - The private data structure for driver
 * addr - Address to put into receive address register
 * index - Receive address register to write
 *****************************************************************************/
void
rtl8168_rar_set(struct rtl8168_private *tp, 
		uint8_t *addr, 
		uint32_t index)
{
	void __iomem *ioaddr = tp->mmio_addr;
	uint32_t rar_low = 0;
	uint32_t rar_high = 0;

	rar_low = ((uint32_t) addr[0] |
		  ((uint32_t) addr[1] << 8) |
		  ((uint32_t) addr[2] << 16) | 
		  ((uint32_t) addr[3] << 24));

	rar_high = ((uint32_t) addr[4] | 
		   ((uint32_t) addr[5] << 8));

	RTL_W8(Cfg9346, Cfg9346_Unlock);
	RTL_W32(MAC0, rar_low);
	RTL_W32(MAC4, rar_high);
	RTL_W8(Cfg9346, Cfg9346_Lock);
}

#ifdef ETHTOOL_OPS_COMPAT
static int ethtool_get_settings(struct net_device *dev, void *useraddr)
{
	struct ethtool_cmd cmd = { ETHTOOL_GSET };
	int err;

	if (!ethtool_ops->get_settings)
		return -EOPNOTSUPP;

	err = ethtool_ops->get_settings(dev, &cmd);
	if (err < 0)
		return err;

	if (copy_to_user(useraddr, &cmd, sizeof(cmd)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_settings(struct net_device *dev, void *useraddr)
{
	struct ethtool_cmd cmd;

	if (!ethtool_ops->set_settings)
		return -EOPNOTSUPP;

	if (copy_from_user(&cmd, useraddr, sizeof(cmd)))
		return -EFAULT;

	return ethtool_ops->set_settings(dev, &cmd);
}

static int ethtool_get_drvinfo(struct net_device *dev, void *useraddr)
{
	struct ethtool_drvinfo info;
	struct ethtool_ops *ops = ethtool_ops;

	if (!ops->get_drvinfo)
		return -EOPNOTSUPP;

	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ops->get_drvinfo(dev, &info);

	if (ops->self_test_count)
		info.testinfo_len = ops->self_test_count(dev);
	if (ops->get_stats_count)
		info.n_stats = ops->get_stats_count(dev);
	if (ops->get_regs_len)
		info.regdump_len = ops->get_regs_len(dev);
	if (ops->get_eeprom_len)
		info.eedump_len = ops->get_eeprom_len(dev);

	if (copy_to_user(useraddr, &info, sizeof(info)))
		return -EFAULT;
	return 0;
}

static int ethtool_get_regs(struct net_device *dev, char *useraddr)
{
	struct ethtool_regs regs;
	struct ethtool_ops *ops = ethtool_ops;
	void *regbuf;
	int reglen, ret;

	if (!ops->get_regs || !ops->get_regs_len)
		return -EOPNOTSUPP;

	if (copy_from_user(&regs, useraddr, sizeof(regs)))
		return -EFAULT;

	reglen = ops->get_regs_len(dev);
	if (regs.len > reglen)
		regs.len = reglen;

	regbuf = kmalloc(reglen, GFP_USER);
	if (!regbuf)
		return -ENOMEM;

	ops->get_regs(dev, &regs, regbuf);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &regs, sizeof(regs)))
		goto out;
	useraddr += offsetof(struct ethtool_regs, data);
	if (copy_to_user(useraddr, regbuf, reglen))
		goto out;
	ret = 0;

out:
	kfree(regbuf);
	return ret;
}

static int ethtool_get_wol(struct net_device *dev, char *useraddr)
{
	struct ethtool_wolinfo wol = { ETHTOOL_GWOL };

	if (!ethtool_ops->get_wol)
		return -EOPNOTSUPP;

	ethtool_ops->get_wol(dev, &wol);

	if (copy_to_user(useraddr, &wol, sizeof(wol)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_wol(struct net_device *dev, char *useraddr)
{
	struct ethtool_wolinfo wol;

	if (!ethtool_ops->set_wol)
		return -EOPNOTSUPP;

	if (copy_from_user(&wol, useraddr, sizeof(wol)))
		return -EFAULT;

	return ethtool_ops->set_wol(dev, &wol);
}

static int ethtool_get_msglevel(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata = { ETHTOOL_GMSGLVL };

	if (!ethtool_ops->get_msglevel)
		return -EOPNOTSUPP;

	edata.data = ethtool_ops->get_msglevel(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_msglevel(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata;

	if (!ethtool_ops->set_msglevel)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	ethtool_ops->set_msglevel(dev, edata.data);
	return 0;
}

static int ethtool_nway_reset(struct net_device *dev)
{
	if (!ethtool_ops->nway_reset)
		return -EOPNOTSUPP;

	return ethtool_ops->nway_reset(dev);
}

static int ethtool_get_link(struct net_device *dev, void *useraddr)
{
	struct ethtool_value edata = { ETHTOOL_GLINK };

	if (!ethtool_ops->get_link)
		return -EOPNOTSUPP;

	edata.data = ethtool_ops->get_link(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_get_eeprom(struct net_device *dev, void *useraddr)
{
	struct ethtool_eeprom eeprom;
	struct ethtool_ops *ops = ethtool_ops;
	u8 *data;
	int ret;

	if (!ops->get_eeprom || !ops->get_eeprom_len)
		return -EOPNOTSUPP;

	if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
		return -EFAULT;

	/* Check for wrap and zero */
	if (eeprom.offset + eeprom.len <= eeprom.offset)
		return -EINVAL;

	/* Check for exceeding total eeprom len */
	if (eeprom.offset + eeprom.len > ops->get_eeprom_len(dev))
		return -EINVAL;

	data = kmalloc(eeprom.len, GFP_USER);
	if (!data)
		return -ENOMEM;

	ret = -EFAULT;
	if (copy_from_user(data, useraddr + sizeof(eeprom), eeprom.len))
		goto out;

	ret = ops->get_eeprom(dev, &eeprom, data);
	if (ret)
		goto out;

	ret = -EFAULT;
	if (copy_to_user(useraddr, &eeprom, sizeof(eeprom)))
		goto out;
	if (copy_to_user(useraddr + sizeof(eeprom), data, eeprom.len))
		goto out;
	ret = 0;

out:
	kfree(data);
	return ret;
}

static int ethtool_set_eeprom(struct net_device *dev, void *useraddr)
{
	struct ethtool_eeprom eeprom;
	struct ethtool_ops *ops = ethtool_ops;
	u8 *data;
	int ret;

	if (!ops->set_eeprom || !ops->get_eeprom_len)
		return -EOPNOTSUPP;

	if (copy_from_user(&eeprom, useraddr, sizeof(eeprom)))
		return -EFAULT;

	/* Check for wrap and zero */
	if (eeprom.offset + eeprom.len <= eeprom.offset)
		return -EINVAL;

	/* Check for exceeding total eeprom len */
	if (eeprom.offset + eeprom.len > ops->get_eeprom_len(dev))
		return -EINVAL;

	data = kmalloc(eeprom.len, GFP_USER);
	if (!data)
		return -ENOMEM;

	ret = -EFAULT;
	if (copy_from_user(data, useraddr + sizeof(eeprom), eeprom.len))
		goto out;

	ret = ops->set_eeprom(dev, &eeprom, data);
	if (ret)
		goto out;

	if (copy_to_user(useraddr + sizeof(eeprom), data, eeprom.len))
		ret = -EFAULT;

out:
	kfree(data);
	return ret;
}

static int ethtool_get_coalesce(struct net_device *dev, void *useraddr)
{
	struct ethtool_coalesce coalesce = { ETHTOOL_GCOALESCE };

	if (!ethtool_ops->get_coalesce)
		return -EOPNOTSUPP;

	ethtool_ops->get_coalesce(dev, &coalesce);

	if (copy_to_user(useraddr, &coalesce, sizeof(coalesce)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_coalesce(struct net_device *dev, void *useraddr)
{
	struct ethtool_coalesce coalesce;

	if (!ethtool_ops->get_coalesce)
		return -EOPNOTSUPP;

	if (copy_from_user(&coalesce, useraddr, sizeof(coalesce)))
		return -EFAULT;

	return ethtool_ops->set_coalesce(dev, &coalesce);
}

static int ethtool_get_ringparam(struct net_device *dev, void *useraddr)
{
	struct ethtool_ringparam ringparam = { ETHTOOL_GRINGPARAM };

	if (!ethtool_ops->get_ringparam)
		return -EOPNOTSUPP;

	ethtool_ops->get_ringparam(dev, &ringparam);

	if (copy_to_user(useraddr, &ringparam, sizeof(ringparam)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_ringparam(struct net_device *dev, void *useraddr)
{
	struct ethtool_ringparam ringparam;

	if (!ethtool_ops->get_ringparam)
		return -EOPNOTSUPP;

	if (copy_from_user(&ringparam, useraddr, sizeof(ringparam)))
		return -EFAULT;

	return ethtool_ops->set_ringparam(dev, &ringparam);
}

static int ethtool_get_pauseparam(struct net_device *dev, void *useraddr)
{
	struct ethtool_pauseparam pauseparam = { ETHTOOL_GPAUSEPARAM };

	if (!ethtool_ops->get_pauseparam)
		return -EOPNOTSUPP;

	ethtool_ops->get_pauseparam(dev, &pauseparam);

	if (copy_to_user(useraddr, &pauseparam, sizeof(pauseparam)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_pauseparam(struct net_device *dev, void *useraddr)
{
	struct ethtool_pauseparam pauseparam;

	if (!ethtool_ops->get_pauseparam)
		return -EOPNOTSUPP;

	if (copy_from_user(&pauseparam, useraddr, sizeof(pauseparam)))
		return -EFAULT;

	return ethtool_ops->set_pauseparam(dev, &pauseparam);
}

static int ethtool_get_rx_csum(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata = { ETHTOOL_GRXCSUM };

	if (!ethtool_ops->get_rx_csum)
		return -EOPNOTSUPP;

	edata.data = ethtool_ops->get_rx_csum(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_rx_csum(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata;

	if (!ethtool_ops->set_rx_csum)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	ethtool_ops->set_rx_csum(dev, edata.data);
	return 0;
}

static int ethtool_get_tx_csum(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata = { ETHTOOL_GTXCSUM };

	if (!ethtool_ops->get_tx_csum)
		return -EOPNOTSUPP;

	edata.data = ethtool_ops->get_tx_csum(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_tx_csum(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata;

	if (!ethtool_ops->set_tx_csum)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	return ethtool_ops->set_tx_csum(dev, edata.data);
}

static int ethtool_get_sg(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata = { ETHTOOL_GSG };

	if (!ethtool_ops->get_sg)
		return -EOPNOTSUPP;

	edata.data = ethtool_ops->get_sg(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_sg(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata;

	if (!ethtool_ops->set_sg)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	return ethtool_ops->set_sg(dev, edata.data);
}

static int ethtool_get_tso(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata = { ETHTOOL_GTSO };

	if (!ethtool_ops->get_tso)
		return -EOPNOTSUPP;

	edata.data = ethtool_ops->get_tso(dev);

	if (copy_to_user(useraddr, &edata, sizeof(edata)))
		return -EFAULT;
	return 0;
}

static int ethtool_set_tso(struct net_device *dev, char *useraddr)
{
	struct ethtool_value edata;

	if (!ethtool_ops->set_tso)
		return -EOPNOTSUPP;

	if (copy_from_user(&edata, useraddr, sizeof(edata)))
		return -EFAULT;

	return ethtool_ops->set_tso(dev, edata.data);
}

static int ethtool_self_test(struct net_device *dev, char *useraddr)
{
	struct ethtool_test test;
	struct ethtool_ops *ops = ethtool_ops;
	u64 *data;
	int ret;

	if (!ops->self_test || !ops->self_test_count)
		return -EOPNOTSUPP;

	if (copy_from_user(&test, useraddr, sizeof(test)))
		return -EFAULT;

	test.len = ops->self_test_count(dev);
	data = kmalloc(test.len * sizeof(u64), GFP_USER);
	if (!data)
		return -ENOMEM;

	ops->self_test(dev, &test, data);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &test, sizeof(test)))
		goto out;
	useraddr += sizeof(test);
	if (copy_to_user(useraddr, data, test.len * sizeof(u64)))
		goto out;
	ret = 0;

out:
	kfree(data);
	return ret;
}

static int ethtool_get_strings(struct net_device *dev, void *useraddr)
{
	struct ethtool_gstrings gstrings;
	struct ethtool_ops *ops = ethtool_ops;
	u8 *data;
	int ret;

	if (!ops->get_strings)
		return -EOPNOTSUPP;

	if (copy_from_user(&gstrings, useraddr, sizeof(gstrings)))
		return -EFAULT;

	switch (gstrings.string_set) {
	case ETH_SS_TEST:
		if (!ops->self_test_count)
			return -EOPNOTSUPP;
		gstrings.len = ops->self_test_count(dev);
		break;
	case ETH_SS_STATS:
		if (!ops->get_stats_count)
			return -EOPNOTSUPP;
		gstrings.len = ops->get_stats_count(dev);
		break;
	default:
		return -EINVAL;
	}

	data = kmalloc(gstrings.len * ETH_GSTRING_LEN, GFP_USER);
	if (!data)
		return -ENOMEM;

	ops->get_strings(dev, gstrings.string_set, data);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &gstrings, sizeof(gstrings)))
		goto out;
	useraddr += sizeof(gstrings);
	if (copy_to_user(useraddr, data, gstrings.len * ETH_GSTRING_LEN))
		goto out;
	ret = 0;

out:
	kfree(data);
	return ret;
}

static int ethtool_phys_id(struct net_device *dev, void *useraddr)
{
	struct ethtool_value id;

	if (!ethtool_ops->phys_id)
		return -EOPNOTSUPP;

	if (copy_from_user(&id, useraddr, sizeof(id)))
		return -EFAULT;

	return ethtool_ops->phys_id(dev, id.data);
}

static int ethtool_get_stats(struct net_device *dev, void *useraddr)
{
	struct ethtool_stats stats;
	struct ethtool_ops *ops = ethtool_ops;
	u64 *data;
	int ret;

	if (!ops->get_ethtool_stats || !ops->get_stats_count)
		return -EOPNOTSUPP;

	if (copy_from_user(&stats, useraddr, sizeof(stats)))
		return -EFAULT;

	stats.n_stats = ops->get_stats_count(dev);
	data = kmalloc(stats.n_stats * sizeof(u64), GFP_USER);
	if (!data)
		return -ENOMEM;

	ops->get_ethtool_stats(dev, &stats, data);

	ret = -EFAULT;
	if (copy_to_user(useraddr, &stats, sizeof(stats)))
		goto out;
	useraddr += sizeof(stats);
	if (copy_to_user(useraddr, data, stats.n_stats * sizeof(u64)))
		goto out;
	ret = 0;

out:
	kfree(data);
	return ret;
}

static int ethtool_ioctl(struct ifreq *ifr)
{
	struct net_device *dev = __dev_get_by_name(ifr->ifr_name);
	void *useraddr = (void *) ifr->ifr_data;
	u32 ethcmd;

	/*
	 * XXX: This can be pushed down into the ethtool_* handlers that
	 * need it.  Keep existing behaviour for the moment.
	 */
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!dev || !netif_device_present(dev))
		return -ENODEV;

	if (copy_from_user(&ethcmd, useraddr, sizeof (ethcmd)))
		return -EFAULT;

	switch (ethcmd) {
	case ETHTOOL_GSET:
		return ethtool_get_settings(dev, useraddr);
	case ETHTOOL_SSET:
		return ethtool_set_settings(dev, useraddr);
	case ETHTOOL_GDRVINFO:
		return ethtool_get_drvinfo(dev, useraddr);
	case ETHTOOL_GREGS:
		return ethtool_get_regs(dev, useraddr);
	case ETHTOOL_GWOL:
		return ethtool_get_wol(dev, useraddr);
	case ETHTOOL_SWOL:
		return ethtool_set_wol(dev, useraddr);
	case ETHTOOL_GMSGLVL:
		return ethtool_get_msglevel(dev, useraddr);
	case ETHTOOL_SMSGLVL:
		return ethtool_set_msglevel(dev, useraddr);
	case ETHTOOL_NWAY_RST:
		return ethtool_nway_reset(dev);
	case ETHTOOL_GLINK:
		return ethtool_get_link(dev, useraddr);
	case ETHTOOL_GEEPROM:
		return ethtool_get_eeprom(dev, useraddr);
	case ETHTOOL_SEEPROM:
		return ethtool_set_eeprom(dev, useraddr);
	case ETHTOOL_GCOALESCE:
		return ethtool_get_coalesce(dev, useraddr);
	case ETHTOOL_SCOALESCE:
		return ethtool_set_coalesce(dev, useraddr);
	case ETHTOOL_GRINGPARAM:
		return ethtool_get_ringparam(dev, useraddr);
	case ETHTOOL_SRINGPARAM:
		return ethtool_set_ringparam(dev, useraddr);
	case ETHTOOL_GPAUSEPARAM:
		return ethtool_get_pauseparam(dev, useraddr);
	case ETHTOOL_SPAUSEPARAM:
		return ethtool_set_pauseparam(dev, useraddr);
	case ETHTOOL_GRXCSUM:
		return ethtool_get_rx_csum(dev, useraddr);
	case ETHTOOL_SRXCSUM:
		return ethtool_set_rx_csum(dev, useraddr);
	case ETHTOOL_GTXCSUM:
		return ethtool_get_tx_csum(dev, useraddr);
	case ETHTOOL_STXCSUM:
		return ethtool_set_tx_csum(dev, useraddr);
	case ETHTOOL_GSG:
		return ethtool_get_sg(dev, useraddr);
	case ETHTOOL_SSG:
		return ethtool_set_sg(dev, useraddr);
	case ETHTOOL_GTSO:
		return ethtool_get_tso(dev, useraddr);
	case ETHTOOL_STSO:
		return ethtool_set_tso(dev, useraddr);
	case ETHTOOL_TEST:
		return ethtool_self_test(dev, useraddr);
	case ETHTOOL_GSTRINGS:
		return ethtool_get_strings(dev, useraddr);
	case ETHTOOL_PHYS_ID:
		return ethtool_phys_id(dev, useraddr);
	case ETHTOOL_GSTATS:
		return ethtool_get_stats(dev, useraddr);
	default:
		return -EOPNOTSUPP;
	}

	return -EOPNOTSUPP;
}
#endif //ETHTOOL_OPS_COMPAT

static int 
rtl8168_do_ioctl(struct net_device *dev, 
		 struct ifreq *ifr, 
		 int cmd)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct mii_ioctl_data *data = if_mii(ifr);
	unsigned long flags;
	void __iomem *ioaddr = tp->mmio_addr;

	if (!netif_running(dev))
		return -ENODEV;

	switch (cmd) {
	case SIOCGMIIPHY:
		data->phy_id = 32; /* Internal PHY */
		return 0;

	case SIOCGMIIREG:
		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1F, 0x0000);
		data->val_out = mdio_read(tp->mmio_addr, data->reg_num & 0x1f);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
		return 0;

	case SIOCSMIIREG:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(tp->mmio_addr, data->reg_num & 0x1f, data->val_in);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
		return 0;
#ifdef ETHTOOL_OPS_COMPAT
	case SIOCETHTOOL:
		return ethtool_ioctl(ifr);
#endif
	case SIOCDEVPRIVATE_RTLASF:
		return rtl8168_asf_ioctl(dev, ifr);
	default:
		return -EOPNOTSUPP;		
	}

	return -EOPNOTSUPP;
}

static void
rtl8168_phy_power_up (struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;

	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1F, 0x0000);
	mdio_write(ioaddr, 0x0E, 0x0000);
	mdio_write(ioaddr, MII_BMCR, BMCR_ANENABLE);
	spin_unlock_irqrestore(&tp->phy_lock, flags);
}

static void
rtl8168_phy_power_down (struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;

	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1F, 0x0000);
	mdio_write(ioaddr, 0x0E, 0x0200);
	mdio_write(ioaddr, MII_BMCR, BMCR_PDOWN);
	spin_unlock_irqrestore(&tp->phy_lock, flags);
}

static int __devinit
rtl8168_init_board(struct pci_dev *pdev, 
		   struct net_device **dev_out,
		   void __iomem **ioaddr_out)
{
	void __iomem *ioaddr;
	struct net_device *dev;
	struct rtl8168_private *tp;
	int rc = -ENOMEM, i, acpi_idle_state = 0, pm_cap;

	assert(ioaddr_out != NULL);

	/* dev zeroed in alloc_etherdev */
	dev = alloc_etherdev(sizeof (*tp));
	if (dev == NULL) {
		if (netif_msg_drv(&debug))
			dev_err(&pdev->dev, "unable to alloc new ethernet\n");
		goto err_out;
	}

	SET_MODULE_OWNER(dev);
	SET_NETDEV_DEV(dev, &pdev->dev);
	tp = netdev_priv(dev);
	tp->dev = dev;
	tp->msg_enable = netif_msg_init(debug.msg_enable, R8168_MSG_DEFAULT);

	/* enable device (incl. PCI PM wakeup and hotplug setup) */
	rc = pci_enable_device(pdev);
	if (rc < 0) {
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "enable failure\n");
		goto err_out_free_dev;
	}

	rc = pci_set_mwi(pdev);
	if (rc < 0)
		goto err_out_disable;

	/* save power state before pci_enable_device overwrites it */
	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
	if (pm_cap) {
		u16 pwr_command;

		pci_read_config_word(pdev, pm_cap + PCI_PM_CTRL, &pwr_command);
		acpi_idle_state = pwr_command & PCI_PM_CTRL_STATE_MASK;
	} else {
		if (netif_msg_probe(tp)) {
			dev_err(&pdev->dev, "PowerManagement capability not found.\n");
		}
	}

	/* make sure PCI base addr 1 is MMIO */
	if (!(pci_resource_flags(pdev, 2) & IORESOURCE_MEM)) {
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "region #1 not an MMIO resource, aborting\n");
		rc = -ENODEV;
		goto err_out_mwi;
	}
	/* check for weird/broken PCI region reporting */
	if (pci_resource_len(pdev, 2) < R8168_REGS_SIZE) {
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "Invalid PCI region size(s), aborting\n");
		rc = -ENODEV;
		goto err_out_mwi;
	}

	rc = pci_request_regions(pdev, MODULENAME);
	if (rc < 0) {
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "could not request regions.\n");
		goto err_out_mwi;
	}

	if ((sizeof(dma_addr_t) > 4) &&
	    !pci_set_dma_mask(pdev, DMA_64BIT_MASK) && use_dac) {
		dev->features |= NETIF_F_HIGHDMA;
	} else {
		rc = pci_set_dma_mask(pdev, DMA_32BIT_MASK);
		if (rc < 0) {
			if (netif_msg_probe(tp))
				dev_err(&pdev->dev, "DMA configuration failed.\n");
			goto err_out_free_res;
		}
	}

	pci_set_master(pdev);

	/* ioremap MMIO region */
	ioaddr = ioremap(pci_resource_start(pdev, 2), R8168_REGS_SIZE);
	if (ioaddr == NULL) {
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "cannot remap MMIO, aborting\n");
		rc = -EIO;
		goto err_out_free_res;
	}

	/* Identify chip attached to board */
	rtl8168_get_mac_version(tp, ioaddr);

	rtl8168_print_mac_version(tp);

	for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--) {
		if (tp->mcfg == rtl_chip_info[i].mcfg)
			break;
	}
		
	if (i < 0) {
		/* Unknown chip: assume array element #0, original RTL-8168 */
		if (netif_msg_probe(tp)) {
			dev_printk(KERN_DEBUG, &pdev->dev, "unknown chip version, assuming %s\n", rtl_chip_info[0].name);
		}
		i++;
	}
	
	tp->chipset = i;

	RTL_W8(Cfg9346, Cfg9346_Unlock);
	RTL_W8(Config1, RTL_R8(Config1) | PMEnable);
	RTL_W8(Config5, RTL_R8(Config5) & PMEStatus);
	RTL_W8(Cfg9346, Cfg9346_Lock);

	*ioaddr_out = ioaddr;
	*dev_out = dev;
out:
	return rc;

err_out_free_res:
	pci_release_regions(pdev);

err_out_mwi:
	pci_clear_mwi(pdev);

err_out_disable:
	pci_disable_device(pdev);

err_out_free_dev:
	free_netdev(dev);
err_out:
	*ioaddr_out = NULL;
	*dev_out = NULL;
	goto out;
}

static void
rtl8168_esd_timer(unsigned long __opaque)
{
	struct net_device *dev = (struct net_device *)__opaque;
	struct rtl8168_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;
	struct timer_list *timer = &tp->esd_timer;
	unsigned long timeout = RTL8168_ESD_TIMEOUT;
	u8 cmd;
	u8 cls;
	u16 io_base_l;
	u16 io_base_h;
	u16 mem_base_l;
	u16 mem_base_h;
	u8 ilr;
	u16 resv_0x20_l;
	u16 resv_0x20_h;
	u16 resv_0x24_l;
	u16 resv_0x24_h;

	tp->esd_flag = 0;

	pci_read_config_byte(pdev, PCI_COMMAND, &cmd);
	if (cmd != tp->pci_cfg_space.cmd) {
		pci_write_config_byte(pdev, PCI_COMMAND, tp->pci_cfg_space.cmd);
		tp->esd_flag = 1;
	}

	pci_read_config_byte(pdev, PCI_CACHE_LINE_SIZE, &cls);
	if (cls != tp->pci_cfg_space.cls) {
		pci_write_config_byte(pdev, PCI_CACHE_LINE_SIZE, tp->pci_cfg_space.cls);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_0, &io_base_l);
	if (io_base_l != tp->pci_cfg_space.io_base_l) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_0, tp->pci_cfg_space.io_base_l);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_0 + 2, &io_base_h);
	if (io_base_h != tp->pci_cfg_space.io_base_h) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_0 + 2, tp->pci_cfg_space.io_base_h);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_2, &mem_base_l);
	if (mem_base_l != tp->pci_cfg_space.mem_base_l) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_2, tp->pci_cfg_space.mem_base_l);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, &mem_base_h);
	if (mem_base_h != tp->pci_cfg_space.mem_base_h) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, tp->pci_cfg_space.mem_base_h);
		tp->esd_flag = 1;
	}

	pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &ilr);
	if (ilr != tp->pci_cfg_space.ilr) {
		pci_write_config_byte(pdev, PCI_INTERRUPT_LINE, tp->pci_cfg_space.ilr);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_4, &resv_0x20_l);
	if (resv_0x20_l != tp->pci_cfg_space.resv_0x20_l) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_4, tp->pci_cfg_space.resv_0x20_l);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, &resv_0x20_h);
	if (resv_0x20_h != tp->pci_cfg_space.resv_0x20_h) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, tp->pci_cfg_space.resv_0x20_h);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_5, &resv_0x24_l);
	if (resv_0x24_l != tp->pci_cfg_space.resv_0x24_l) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_5, tp->pci_cfg_space.resv_0x24_l);
		tp->esd_flag = 1;
	}

	pci_read_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, &resv_0x24_h);
	if (resv_0x24_h != tp->pci_cfg_space.resv_0x24_h) {
		pci_write_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, tp->pci_cfg_space.resv_0x24_h);
		tp->esd_flag = 1;
	}

	if (tp->esd_flag != 0) {
		rtl8168_tx_clear(tp);
		rtl8168_rx_clear(tp);
		rtl8168_open(dev);
		tp->esd_flag = 0;
	}

	mod_timer(timer, jiffies + timeout);
}

static void
rtl8168_link_timer(unsigned long __opaque)
{
	struct net_device *dev = (struct net_device *)__opaque;
	struct rtl8168_private *tp = netdev_priv(dev);
	struct timer_list *timer = &tp->link_timer;
	unsigned long timeout = RTL8168_LINK_TIMEOUT;

	if (tp->link_ok(dev) != tp->old_link_status)
		rtl8168_check_link_status(dev, tp, tp->mmio_addr);

	tp->old_link_status = tp->link_ok(dev);

	mod_timer(timer, jiffies + timeout);
}

/* Cfg9346_Unlock assumed. */
static unsigned rtl8168_try_msi(struct pci_dev *pdev, void __iomem *ioaddr)
{
	unsigned msi = 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
	if (pci_enable_msi(pdev)) {
			dev_info(&pdev->dev, "no MSI. Back to INTx.\n");
	} else {
			msi |= RTL_FEATURE_MSI;
	}
#endif

	return msi;
}

static void rtl8168_disable_msi(struct pci_dev *pdev, struct rtl8168_private *tp)
{
	if (tp->features & RTL_FEATURE_MSI) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
		pci_disable_msi(pdev);
#endif
		tp->features &= ~RTL_FEATURE_MSI;
	}
}

static int __devinit
rtl8168_init_one(struct pci_dev *pdev, 
		 const struct pci_device_id *ent)
{
	struct net_device *dev = NULL;
	struct rtl8168_private *tp;
	void __iomem *ioaddr = NULL;
	static int board_idx = -1;
	u8 autoneg, duplex;
	u16 speed;
	int i, rc;

	assert(pdev != NULL);
	assert(ent != NULL);

	board_idx++;

	if (netif_msg_drv(&debug)) {
		printk(KERN_INFO "%s Gigabit Ethernet driver %s loaded\n",
		       MODULENAME, RTL8168_VERSION);
	}

	rc = rtl8168_init_board(pdev, &dev, &ioaddr);
	if (rc)
		return rc;

	tp = netdev_priv(dev);
	assert(ioaddr != NULL);

	tp->set_speed = rtl8168_set_speed_xmii;
	tp->get_settings = rtl8168_gset_xmii;
	tp->phy_reset_enable = rtl8168_xmii_reset_enable;
	tp->phy_reset_pending = rtl8168_xmii_reset_pending;
	tp->link_ok = rtl8168_xmii_link_ok;

	RTL_W8(Cfg9346, Cfg9346_Unlock);
	tp->features |= rtl8168_try_msi(pdev, ioaddr);
	RTL_W8(Cfg9346, Cfg9346_Lock);

	/* Get MAC address.  FIXME: read EEPROM */
	for (i = 0; i < MAC_ADDR_LEN; i++)
		dev->dev_addr[i] = RTL_R8(MAC0 + i);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
#endif
	memcpy(dev->dev_addr, dev->dev_addr, dev->addr_len);

	dev->open = rtl8168_open;
	dev->hard_start_xmit = rtl8168_start_xmit;
	dev->get_stats = rtl8168_get_stats;
	SET_ETHTOOL_OPS(dev, &rtl8168_ethtool_ops);
	dev->stop = rtl8168_close;
	dev->tx_timeout = rtl8168_tx_timeout;
	dev->set_multicast_list = rtl8168_set_rx_mode;
	dev->watchdog_timeo = RTL8168_TX_TIMEOUT;
	dev->irq = pdev->irq;
	dev->base_addr = (unsigned long) ioaddr;
	dev->change_mtu = rtl8168_change_mtu;
	dev->set_mac_address = rtl8168_set_mac_address;
	dev->do_ioctl = rtl8168_do_ioctl;

#ifdef CONFIG_R8168_NAPI
	RTL_NAPI_CONFIG(dev, tp, rtl8168_poll, R8168_NAPI_WEIGHT);
#endif

#ifdef CONFIG_R8168_VLAN
	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
	dev->vlan_rx_register = rtl8168_vlan_rx_register;
	dev->vlan_rx_kill_vid = rtl8168_vlan_rx_kill_vid;
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
	dev->poll_controller = rtl8168_netpoll;
#endif

	dev->features |= NETIF_F_IP_CSUM;
	tp->cp_cmd |= RxChkSum;
	tp->cp_cmd |= RTL_R16(CPlusCmd);

	tp->intr_mask = rtl8168_intr_mask;
	tp->pci_dev = pdev;
	tp->mmio_addr = ioaddr;

	tp->max_jumbo_frame_size = rtl_chip_info[tp->chipset].jumbo_frame_sz;

	spin_lock_init(&tp->lock);
	spin_lock_init(&tp->phy_lock);

	rc = register_netdev(dev);
	if (rc) {
		rtl8168_release_board(pdev, dev, ioaddr);
		return rc;
	}

	printk(KERN_INFO "%s: This product is covered by one or more of the following patents: US5,307,459, US5,434,872, US5,732,094, US6,570,884, US6,115,776, and US6,327,625.\n", MODULENAME);

	if (netif_msg_probe(tp)) {
		printk(KERN_DEBUG "%s: Identified chip type is '%s'.\n",
		       dev->name, rtl_chip_info[tp->chipset].name);
	}

	pci_set_drvdata(pdev, dev);

	if (netif_msg_probe(tp)) {
		printk(KERN_INFO "%s: %s at 0x%lx, "
		       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, "
		       "IRQ %d\n",
		       dev->name,
		       rtl_chip_info[ent->driver_data].name,
		       dev->base_addr,
		       dev->dev_addr[0], dev->dev_addr[1],
		       dev->dev_addr[2], dev->dev_addr[3],
		       dev->dev_addr[4], dev->dev_addr[5], dev->irq);
	}

	rtl8168_hw_phy_config(dev);

	pci_write_config_byte(pdev, PCI_LATENCY_TIMER, 0x40); 

	rtl8168_link_option(board_idx, &autoneg, &speed, &duplex);

	rtl8168_set_speed(dev, autoneg, speed, duplex);

	return 0;
}

static void __devexit
rtl8168_remove_one(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rtl8168_private *tp = netdev_priv(dev);

	assert(dev != NULL);
	assert(tp != NULL);

	flush_scheduled_work();

	unregister_netdev(dev);
	rtl8168_disable_msi(pdev, tp);
	rtl8168_release_board(pdev, dev, tp->mmio_addr);
	pci_set_drvdata(pdev, NULL);
}

static void 
rtl8168_set_rxbufsize(struct rtl8168_private *tp,
		      struct net_device *dev)
{
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned int mtu = dev->mtu;

	tp->rx_buf_sz = (mtu > ETH_DATA_LEN) ? mtu + ETH_HLEN + 8 : RX_BUF_SIZE;

	RTL_W16(RxMaxSize, tp->rx_buf_sz + 1);
}

static int 
rtl8168_open(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;
	int retval;

	rtl8168_set_rxbufsize(tp, dev);

	retval = request_irq(dev->irq, rtl8168_interrupt, (tp->features & RTL_FEATURE_MSI) ? 0 : SA_SHIRQ, dev->name, dev);

	if (retval < 0)
		goto out;

	retval = -ENOMEM;

	/*
	 * Rx and Tx desscriptors needs 256 bytes alignment.
	 * pci_alloc_consistent provides more.
	 */
	tp->TxDescArray = pci_alloc_consistent(pdev, R8168_TX_RING_BYTES,
					       &tp->TxPhyAddr);
	if (!tp->TxDescArray)
		goto err_free_irq;

	tp->RxDescArray = pci_alloc_consistent(pdev, R8168_RX_RING_BYTES,
					       &tp->RxPhyAddr);
	if (!tp->RxDescArray)
		goto err_free_tx;

	retval = rtl8168_init_ring(dev);
	if (retval < 0)
		goto err_free_rx;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&tp->task, NULL, dev);
#else
	INIT_DELAYED_WORK(&tp->task, NULL);
#endif

#ifdef	CONFIG_R8168_NAPI
	RTL_NAPI_ENABLE(dev, &tp->napi);
#endif

	rtl8168_hw_start(dev);

	if (tp->esd_flag == 0) {
		rtl8168_request_esd_timer(dev);
	}

	rtl8168_request_link_timer(dev);

	rtl8168_dsm(dev, DSM_IF_UP);

	rtl8168_check_link_status(dev, tp, tp->mmio_addr);
out: return retval;

err_free_rx:
	pci_free_consistent(pdev, R8168_RX_RING_BYTES, tp->RxDescArray,
			    tp->RxPhyAddr);
err_free_tx:
	pci_free_consistent(pdev, R8168_TX_RING_BYTES, tp->TxDescArray,
			    tp->TxPhyAddr);
err_free_irq:
	free_irq(dev->irq, dev);
	goto out;
}

static void 
rtl8168_hw_reset(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	/* Disable interrupts */
	rtl8168_irq_mask_and_ack(ioaddr);

	rtl8168_nic_reset(dev);
}

static void
rtl8168_dsm(struct net_device *dev, int dev_state)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	switch (dev_state) {
	case DSM_MAC_INIT:
		if ((tp->mcfg == CFG_METHOD_5) || (tp->mcfg == CFG_METHOD_6)) {
			if (RTL_R8(MACDBG) & 0x80) {
				RTL_W8(GPIO, RTL_R8(GPIO) | GPIO_en);
			} else {
				RTL_W8(GPIO, RTL_R8(GPIO) & ~GPIO_en);
			}
		}

		break;
	case DSM_NIC_GOTO_D3:
	case DSM_IF_DOWN:
		if ((tp->mcfg == CFG_METHOD_5) || (tp->mcfg == CFG_METHOD_6))
			if (RTL_R8(MACDBG) & 0x80)
				RTL_W8(GPIO, RTL_R8(GPIO) & ~GPIO_en);

		break;
	case DSM_NIC_RESUME_D3:
	case DSM_IF_UP:
		if ((tp->mcfg == CFG_METHOD_5) || (tp->mcfg == CFG_METHOD_6))
			if (RTL_R8(MACDBG) & 0x80)
				RTL_W8(GPIO, RTL_R8(GPIO) | GPIO_en);

		break;
	}

}

static void
rtl8168_hw_start(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	struct pci_dev *pdev = tp->pci_dev;
	u8 device_control, options1, options2;
	u16 ephy_data;
	u32 csi_tmp;

	rtl8168_nic_reset(dev);

	RTL_W8(Cfg9346, Cfg9346_Unlock);

	RTL_W8(Reserved1, Reserved1_data);

	tp->cp_cmd |= PktCntrDisable | INTT_1;
	RTL_W16(CPlusCmd, tp->cp_cmd);

	RTL_W16(IntrMitigate, 0x5151);

	//Work around for RxFIFO overflow
	if (tp->mcfg == CFG_METHOD_1) {
		rtl8168_intr_mask |= RxFIFOOver | PCSTimeout;
		rtl8168_intr_mask &= ~RxDescUnavail;
	}

	RTL_W32(TxDescStartAddrLow, ((u64) tp->TxPhyAddr & DMA_32BIT_MASK));
	RTL_W32(TxDescStartAddrHigh, ((u64) tp->TxPhyAddr >> 32));
	RTL_W32(RxDescAddrLow, ((u64) tp->RxPhyAddr & DMA_32BIT_MASK));
	RTL_W32(RxDescAddrHigh, ((u64) tp->RxPhyAddr >> 32));

	/* Set Rx Config register */
	rtl8168_set_rx_mode(dev);

	/* Set DMA burst size and Interframe Gap Time */
	if (tp->mcfg == CFG_METHOD_1) {
		RTL_W32(TxConfig, (TX_DMA_BURST_512 << TxDMAShift) | 
				  (InterFrameGap << TxInterFrameGapShift));
	} else {
		RTL_W32(TxConfig, (TX_DMA_BURST_unlimited << TxDMAShift) | 
				  (InterFrameGap << TxInterFrameGapShift));
	}

	/* Clear the interrupt status register. */
	RTL_W16(IntrStatus, 0xFFFF);

	if (tp->rx_fifo_overflow == 0) {
		/* Enable all known interrupts by setting the interrupt mask. */
		RTL_W16(IntrMask, rtl8168_intr_mask);
		netif_start_queue(dev);
	}

	if (tp->mcfg == CFG_METHOD_4) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(ioaddr, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(ioaddr, 0x70c, csi_tmp | 0x27000000);

		RTL_W8(DBG_reg, (0x0E << 4) | Fix_Nak_1 | Fix_Nak_2);

		/*Set EPHY registers	begin*/
		/*Set EPHY register offset 0x02 bit 11 to 0 and bit 12 to 1*/
		ephy_data = rtl8168_ephy_read(ioaddr, 0x02);
		ephy_data &= ~(1 << 11);
		ephy_data |= (1 << 12);
		rtl8168_ephy_write(ioaddr, 0x02, ephy_data);

		/*Set EPHY register offset 0x03 bit 1 to 1*/
		ephy_data = rtl8168_ephy_read(ioaddr, 0x03);
		ephy_data |= (1 << 1);
		rtl8168_ephy_write(ioaddr, 0x03, ephy_data);

		/*Set EPHY register offset 0x06 bit 7 to 0*/
		ephy_data = rtl8168_ephy_read(ioaddr, 0x06);
		ephy_data &= ~(1 << 7);
		rtl8168_ephy_write(ioaddr, 0x06, ephy_data);
		/*Set EPHY registers	end*/

		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		//disable clock request.
		pci_write_config_byte(pdev, 0x81, 0x00);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) | Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) & ~Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		}
	} else if (tp->mcfg == CFG_METHOD_5) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(ioaddr, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(ioaddr, 0x70c, csi_tmp | 0x27000000);

		/******set EPHY registers for RTL8168CP	begin******/
		//Set EPHY register offset 0x01 bit 0 to 1.
		ephy_data = rtl8168_ephy_read(ioaddr, 0x01);
		ephy_data |= (1 << 0);
		rtl8168_ephy_write(ioaddr, 0x01, ephy_data);

		//Set EPHY register offset 0x03 bit 10 to 0, bit 9 to 1 and bit 5 to 1.
		ephy_data = rtl8168_ephy_read(ioaddr, 0x03);
		ephy_data &= ~(1 << 10);
		ephy_data |= (1 << 9);
		ephy_data |= (1 << 5);
		rtl8168_ephy_write(ioaddr, 0x03, ephy_data);
		/******set EPHY registers for RTL8168CP	end******/

		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		//disable clock request.
		pci_write_config_byte(pdev, 0x81, 0x00);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) | Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) & ~Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		}
	} else if (tp->mcfg == CFG_METHOD_6) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(ioaddr, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(ioaddr, 0x70c, csi_tmp | 0x27000000);

		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) | Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) & ~Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		}
	} else if (tp->mcfg == CFG_METHOD_7) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(ioaddr, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(ioaddr, 0x70c, csi_tmp | 0x27000000);
		rtl8168_eri_write(ioaddr, 0x1EC, 1, 0x07, ERIAR_ASF);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) | Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) & ~Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		}
	} else if (tp->mcfg == CFG_METHOD_8) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(ioaddr, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(ioaddr, 0x70c, csi_tmp | 0x27000000);
		rtl8168_eri_write(ioaddr, 0x1EC, 1, 0x07, ERIAR_ASF);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		RTL_W8(0xD1, 0x20);

		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) | Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) | Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x20
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload disable
			dev->features &= ~NETIF_F_IP_CSUM;

			//rx checksum offload disable
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) & ~Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) & ~Jumbo_En1);

			//Set PCI configuration space offset 0x79 to 0x50
			/*Increase the Tx performance*/
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_write_config_byte(pdev, 0x79, device_control);

			//tx checksum offload enable
			dev->features |= NETIF_F_IP_CSUM;

			//rx checksum offload enable
			tp->cp_cmd |= RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		}

	} else if (tp->mcfg == CFG_METHOD_9) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(ioaddr, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(ioaddr, 0x70c, csi_tmp | 0x13000000);

		/* disable clock request. */
		pci_write_config_byte(pdev, 0x81, 0x00);

		RTL_W8(Config3, RTL_R8(Config3) & ~BIT_4);
		RTL_W8(DBG_reg, RTL_R8(DBG_reg) | BIT_7 | BIT_1);

		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) | Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) | Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x20 */
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			pci_write_config_byte(pdev, 0x79, device_control);

			/* tx checksum offload disable */
			dev->features &= ~NETIF_F_IP_CSUM;

			/* rx checksum offload disable */
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) & ~Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) & ~Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x50 */
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_write_config_byte(pdev, 0x79, device_control);

			/* tx checksum offload enable */
			dev->features |= NETIF_F_IP_CSUM;

			/* rx checksum offload enable */
			tp->cp_cmd |= RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		}

		/* set EPHY registers */
		rtl8168_ephy_write(ioaddr, 0x01, 0x7C7D);
		rtl8168_ephy_write(ioaddr, 0x02, 0x091F);
		rtl8168_ephy_write(ioaddr, 0x06, 0xB271);
		rtl8168_ephy_write(ioaddr, 0x07, 0xCE00);
	} else if (tp->mcfg == CFG_METHOD_10) {
		/*set PCI configuration space offset 0x70F to 0x27*/
		/*When the register offset of PCI configuration space larger than 0xff, use CSI to access it.*/
		csi_tmp = rtl8168_csi_read(ioaddr, 0x70c) & 0x00ffffff;
		rtl8168_csi_write(ioaddr, 0x70c, csi_tmp | 0x13000000);

		RTL_W8(DBG_reg, RTL_R8(DBG_reg) | BIT_7 | BIT_1);

		if (dev->mtu > ETH_DATA_LEN) {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) | Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) | Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x20 */
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x20;
			pci_write_config_byte(pdev, 0x79, device_control);

			/* tx checksum offload disable */
			dev->features &= ~NETIF_F_IP_CSUM;

			/* rx checksum offload disable */
			tp->cp_cmd &= ~RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		} else {
			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config3, RTL_R8(Config3) & ~Jumbo_En0);
			RTL_W8(Config4, RTL_R8(Config4) & ~Jumbo_En1);

			/* Set PCI configuration space offset 0x79 to 0x50 */
			pci_read_config_byte(pdev, 0x79, &device_control);
			device_control &= ~0x70;
			device_control |= 0x50;
			pci_write_config_byte(pdev, 0x79, device_control);

			/* tx checksum offload enable */
			dev->features |= NETIF_F_IP_CSUM;

			/* rx checksum offload enable */
			tp->cp_cmd |= RxChkSum;
			RTL_W16(CPlusCmd, tp->cp_cmd);
		}

		RTL_W8(Config1, 0xDF);

		/* set EPHY registers */
		rtl8168_ephy_write(ioaddr, 0x01, 0x6C7F);
		rtl8168_ephy_write(ioaddr, 0x02, 0x011F);
		rtl8168_ephy_write(ioaddr, 0x03, 0xC1B2);
		rtl8168_ephy_write(ioaddr, 0x1A, 0x0546);
		rtl8168_ephy_write(ioaddr, 0x1C, 0x80C4);
		rtl8168_ephy_write(ioaddr, 0x1D, 0x78E4);
		rtl8168_ephy_write(ioaddr, 0x0A, 0x8100);

		/* disable clock request. */
		pci_write_config_byte(pdev, 0x81, 0x00);

		RTL_W8(0xF3, RTL_R8(0xF3) | (1 << 2));

	} else if (tp->mcfg == CFG_METHOD_1) {
		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		if (dev->mtu > ETH_DATA_LEN) {
			pci_read_config_byte(pdev, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x28;
			pci_write_config_byte(pdev, 0x69, device_control);
		} else {
			pci_read_config_byte(pdev, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x58;
			pci_write_config_byte(pdev, 0x69, device_control);
		}
	} else if (tp->mcfg == CFG_METHOD_2) {
		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		if (dev->mtu > ETH_DATA_LEN) {
			pci_read_config_byte(pdev, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x28;
			pci_write_config_byte(pdev, 0x69, device_control);

			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config4, RTL_R8(Config4) | (1 << 0));
		} else {
			pci_read_config_byte(pdev, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x58;
			pci_write_config_byte(pdev, 0x69, device_control);

			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config4, RTL_R8(Config4) & ~(1 << 0));
		}
	} else if (tp->mcfg == CFG_METHOD_3) {
		RTL_W8(Config3, RTL_R8(Config3) & ~Beacon_en);

		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd) & 
			~(EnableBist | Macdbgo_oe | Force_halfdup | Force_rxflow_en | Force_txflow_en | 
			  Cxpl_dbg_sel | ASF | PktCntrDisable | Macdbgo_sel));

		if (dev->mtu > ETH_DATA_LEN) {
			pci_read_config_byte(pdev, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x28;
			pci_write_config_byte(pdev, 0x69, device_control);

			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config4, RTL_R8(Config4) | (1 << 0));
		} else {
			pci_read_config_byte(pdev, 0x69, &device_control);
			device_control &= ~0x70;
			device_control |= 0x58;
			pci_write_config_byte(pdev, 0x69, device_control);

			RTL_W8(Reserved1, Reserved1_data);
			RTL_W8(Config4, RTL_R8(Config4) & ~(1 << 0));
		}
	}

	if ((tp->mcfg == CFG_METHOD_1) || (tp->mcfg == CFG_METHOD_2) || (tp->mcfg == CFG_METHOD_3)) {
		/* csum offload command for RTL8168B/8111B */
		tp->tx_tcp_csum_cmd = TxIPCS | TxTCPCS;
		tp->tx_udp_csum_cmd = TxIPCS | TxUDPCS;
		tp->tx_ip_csum_cmd = TxIPCS;
	} else {
		/* csum offload command for RTL8168C/8111C and RTL8168CP/8111CP */
		tp->tx_tcp_csum_cmd = TxIPCS_C | TxTCPCS_C;
		tp->tx_udp_csum_cmd = TxIPCS_C | TxUDPCS_C;
		tp->tx_ip_csum_cmd = TxIPCS_C;
	}

	RTL_W8(ChipCmd, CmdTxEnb | CmdRxEnb);

	RTL_W8(Cfg9346, Cfg9346_Lock);

	if (!tp->pci_cfg_is_read) {
		pci_read_config_byte(pdev, PCI_COMMAND, &tp->pci_cfg_space.cmd);
		pci_read_config_byte(pdev, PCI_CACHE_LINE_SIZE, &tp->pci_cfg_space.cls);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_0, &tp->pci_cfg_space.io_base_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_0 + 2, &tp->pci_cfg_space.io_base_h);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_2, &tp->pci_cfg_space.mem_base_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, &tp->pci_cfg_space.mem_base_h);
		pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &tp->pci_cfg_space.ilr);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_4, &tp->pci_cfg_space.resv_0x20_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, &tp->pci_cfg_space.resv_0x20_h);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_5, &tp->pci_cfg_space.resv_0x24_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, &tp->pci_cfg_space.resv_0x24_h);

		tp->pci_cfg_is_read = 1;
	}

	rtl8168_dsm(dev, DSM_MAC_INIT);

	options1 = RTL_R8(Config3);
	options2 = RTL_R8(Config5);
	if ((options1 & LinkUp) || (options1 & MagicPacket) || (options2 & UWF) || (options2 & BWF) || (options2 & MWF))
		tp->wol_enabled = WOL_ENABLED;
	else
		tp->wol_enabled = WOL_DISABLED;

	udelay(10);
}

static int 
rtl8168_change_mtu(struct net_device *dev, 
		   int new_mtu)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	int ret = 0;

	if (new_mtu < ETH_ZLEN || new_mtu > tp->max_jumbo_frame_size)
		return -EINVAL;

	if (!netif_running(dev))
		goto out;

	rtl8168_down(dev);

	dev->mtu = new_mtu;

	rtl8168_set_rxbufsize(tp, dev);

	ret = rtl8168_init_ring(dev);

	if (ret < 0)
		goto out;

#ifdef CONFIG_R8168_NAPI
	RTL_NAPI_ENABLE(dev, &tp->napi);
#endif//CONFIG_R8168_NAPI 

	rtl8168_hw_start(dev);

out:
	return ret;
}

static inline void 
rtl8168_make_unusable_by_asic(struct RxDesc *desc)
{
	desc->addr = 0x0badbadbadbadbadull;
	desc->opts1 &= ~cpu_to_le32(DescOwn | RsvdMask);
}

static void 
rtl8168_free_rx_skb(struct rtl8168_private *tp,
		    struct sk_buff **sk_buff, 
		    struct RxDesc *desc)
{
	struct pci_dev *pdev = tp->pci_dev;

	pci_unmap_single(pdev, le64_to_cpu(desc->addr), tp->rx_buf_sz,
			 PCI_DMA_FROMDEVICE);
	dev_kfree_skb(*sk_buff);
	*sk_buff = NULL;
	rtl8168_make_unusable_by_asic(desc);
}

static inline void 
rtl8168_mark_to_asic(struct RxDesc *desc, 
		     u32 rx_buf_sz)
{
	u32 eor = le32_to_cpu(desc->opts1) & RingEnd;

	desc->opts1 = cpu_to_le32(DescOwn | eor | rx_buf_sz);
}

static inline void 
rtl8168_map_to_asic(struct RxDesc *desc, 
		    dma_addr_t mapping,
		    u32 rx_buf_sz)
{
	desc->addr = cpu_to_le64(mapping);
	wmb();
	rtl8168_mark_to_asic(desc, rx_buf_sz);
}

static int 
rtl8168_alloc_rx_skb(struct pci_dev *pdev, 
		     struct sk_buff **sk_buff,
		     struct RxDesc *desc, 
		     int rx_buf_sz)
{
	struct sk_buff *skb;
	dma_addr_t mapping;
	int ret = 0;

	skb = dev_alloc_skb(rx_buf_sz + NET_IP_ALIGN);
	if (!skb)
		goto err_out;

	skb_reserve(skb, NET_IP_ALIGN);
	*sk_buff = skb;

	mapping = pci_map_single(pdev, skb->data, rx_buf_sz,
				 PCI_DMA_FROMDEVICE);

	rtl8168_map_to_asic(desc, mapping, rx_buf_sz);

out:
	return ret;

err_out:
	ret = -ENOMEM;
	rtl8168_make_unusable_by_asic(desc);
	goto out;
}

static void 
rtl8168_rx_clear(struct rtl8168_private *tp)
{
	int i;

	for (i = 0; i < NUM_RX_DESC; i++) {
		if (tp->Rx_skbuff[i]) {
			rtl8168_free_rx_skb(tp, tp->Rx_skbuff + i,
					    tp->RxDescArray + i);
		}
	}
}

static u32 
rtl8168_rx_fill(struct rtl8168_private *tp, 
		struct net_device *dev,
		u32 start, 
		u32 end)
{
	u32 cur;
	
	for (cur = start; end - cur > 0; cur++) {
		int ret, i = cur % NUM_RX_DESC;

		if (tp->Rx_skbuff[i])
			continue;
			
		ret = rtl8168_alloc_rx_skb(tp->pci_dev, tp->Rx_skbuff + i,
					   tp->RxDescArray + i, tp->rx_buf_sz);
		if (ret < 0)
			break;
	}
	return cur - start;
}

static inline void 
rtl8168_mark_as_last_descriptor(struct RxDesc *desc)
{
	desc->opts1 |= cpu_to_le32(RingEnd);
}

static void 
rtl8168_init_ring_indexes(struct rtl8168_private *tp)
{
	tp->dirty_tx = 0;
	tp->dirty_rx = 0;
	tp->cur_tx = 0;
	tp->cur_rx = 0;
}

static void
rtl8168_tx_desc_init(struct rtl8168_private *tp)
{
	int i = 0;
	
	memset(tp->TxDescArray, 0x0, NUM_TX_DESC * sizeof(struct TxDesc));

	for (i = 0; i < NUM_TX_DESC; i++)
		if(i == (NUM_TX_DESC - 1))
			tp->TxDescArray[i].opts1 = cpu_to_le32(RingEnd);
}

static void
rtl8168_rx_desc_init(struct rtl8168_private *tp)
{
	int i = 0;

	memset(tp->RxDescArray, 0x0, NUM_RX_DESC * sizeof(struct RxDesc));

	for (i = 0; i < NUM_RX_DESC; i++) {
		if(i == (NUM_RX_DESC - 1))
			tp->RxDescArray[i].opts1 = 
				cpu_to_le32((DescOwn | RingEnd) | 
					    (unsigned long)tp->rx_buf_sz);
		else
			tp->RxDescArray[i].opts1 = 
				cpu_to_le32(DescOwn | 
					    (unsigned long)tp->rx_buf_sz);
	}
}

static int 
rtl8168_init_ring(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	rtl8168_init_ring_indexes(tp);

	memset(tp->tx_skb, 0x0, NUM_TX_DESC * sizeof(struct ring_info));
	memset(tp->Rx_skbuff, 0x0, NUM_RX_DESC * sizeof(struct sk_buff *));

	rtl8168_tx_desc_init(tp);
	rtl8168_rx_desc_init(tp);

	if (rtl8168_rx_fill(tp, dev, 0, NUM_RX_DESC) != NUM_RX_DESC)
		goto err_out;

	rtl8168_mark_as_last_descriptor(tp->RxDescArray + NUM_RX_DESC - 1);

	return 0;

err_out:
	rtl8168_rx_clear(tp);
	return -ENOMEM;
}

static void 
rtl8168_unmap_tx_skb(struct pci_dev *pdev, 
		     struct ring_info *tx_skb,
		     struct TxDesc *desc)
{
	unsigned int len = tx_skb->len;

	pci_unmap_single(pdev, le64_to_cpu(desc->addr), len, PCI_DMA_TODEVICE);
	desc->opts1 = 0x00;
	desc->opts2 = 0x00;
	desc->addr = 0x00;
	tx_skb->len = 0;
}

static void 
rtl8168_tx_clear(struct rtl8168_private *tp)
{
	unsigned int i;
	struct net_device *dev = tp->dev;

	for (i = tp->dirty_tx; i < tp->dirty_tx + NUM_TX_DESC; i++) {
		unsigned int entry = i % NUM_TX_DESC;
		struct ring_info *tx_skb = tp->tx_skb + entry;
		unsigned int len = tx_skb->len;

		if (len) {
			struct sk_buff *skb = tx_skb->skb;

			rtl8168_unmap_tx_skb(tp->pci_dev, tx_skb,
					     tp->TxDescArray + entry);
			if (skb) {
				dev_kfree_skb(skb);
				tx_skb->skb = NULL;
			}
			RTLDEV->stats.tx_dropped++;
		}
	}
	tp->cur_tx = tp->dirty_tx = 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8168_schedule_work(struct net_device *dev, void (*task)(void *))
{
	struct rtl8168_private *tp = netdev_priv(dev);

	PREPARE_WORK(&tp->task, task, dev);
	schedule_delayed_work(&tp->task, 4);
}
#else
static void rtl8168_schedule_work(struct net_device *dev, work_func_t task)
{
	struct rtl8168_private *tp = netdev_priv(dev);

	PREPARE_DELAYED_WORK(&tp->task, task);
	schedule_delayed_work(&tp->task, 4);
}
#endif

static void 
rtl8168_wait_for_quiescence(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	synchronize_irq(dev->irq);

	/* Wait for any pending NAPI task to complete */
#ifdef CONFIG_R8168_NAPI
	RTL_NAPI_DISABLE(dev, &tp->napi);
#endif//CONFIG_R8168_NAPI

	rtl8168_irq_mask_and_ack(ioaddr);

#ifdef CONFIG_R8168_NAPI
	RTL_NAPI_ENABLE(dev, &tp->napi);
#endif//CONFIG_R8168_NAPI
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8168_reinit_task(void *_data)
#else
static void rtl8168_reinit_task(struct work_struct *work)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	struct net_device *dev = _data;
#else
	struct rtl8168_private *tp =
		container_of(work, struct rtl8168_private, task.work);
	struct net_device *dev = tp->dev;
#endif
	int ret;

	if (netif_running(dev)) {
		rtl8168_wait_for_quiescence(dev);
		rtl8168_close(dev);
	}

	ret = rtl8168_open(dev);
	if (unlikely(ret < 0)) {
		if (net_ratelimit()) {
			struct rtl8168_private *tp = netdev_priv(dev);

			if (netif_msg_drv(tp)) {
				printk(PFX KERN_ERR
				       "%s: reinit failure (status = %d)."
				       " Rescheduling.\n", dev->name, ret);
			}
		}
		rtl8168_schedule_work(dev, rtl8168_reinit_task);
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8168_reset_task(void *_data)
{
	struct net_device *dev = _data;
	struct rtl8168_private *tp = netdev_priv(dev);
#else
static void rtl8168_reset_task(struct work_struct *work)
{
	struct rtl8168_private *tp =
		container_of(work, struct rtl8168_private, task.work);
	struct net_device *dev = tp->dev;
#endif

	if (!netif_running(dev))
		return;

	rtl8168_wait_for_quiescence(dev);

	rtl8168_rx_interrupt(dev, tp, tp->mmio_addr, ~(u32)0);
	rtl8168_tx_clear(tp);

	if (tp->dirty_rx == tp->cur_rx) {
		rtl8168_init_ring_indexes(tp);
		rtl8168_hw_start(dev);
		netif_wake_queue(dev);
	} else {
		if (net_ratelimit()) {
			struct rtl8168_private *tp = netdev_priv(dev);

			if (netif_msg_intr(tp)) {
				printk(PFX KERN_EMERG
				       "%s: Rx buffers shortage\n", dev->name);
			}
		}
		rtl8168_schedule_work(dev, rtl8168_reset_task);
	}
}

static void 
rtl8168_tx_timeout(struct net_device *dev)
{
	rtl8168_hw_reset(dev);

	/* Let's wait a bit while any (async) irq lands on */
	rtl8168_schedule_work(dev, rtl8168_reset_task);
}

static int 
rtl8168_xmit_frags(struct rtl8168_private *tp, 
		   struct sk_buff *skb,
		   u32 opts1)
{
	struct skb_shared_info *info = skb_shinfo(skb);
	unsigned int cur_frag, entry;
	struct TxDesc *txd = NULL;

	entry = tp->cur_tx;
	for (cur_frag = 0; cur_frag < info->nr_frags; cur_frag++) {
		skb_frag_t *frag = info->frags + cur_frag;
		dma_addr_t mapping;
		u32 status, len;
		void *addr;

		entry = (entry + 1) % NUM_TX_DESC;

		txd = tp->TxDescArray + entry;
		len = frag->size;
		addr = ((void *) page_address(frag->page)) + frag->page_offset;
		mapping = pci_map_single(tp->pci_dev, addr, len, PCI_DMA_TODEVICE);

		/* anti gcc 2.95.3 bugware (sic) */
		status = opts1 | len | (RingEnd * !((entry + 1) % NUM_TX_DESC));

		txd->opts1 = cpu_to_le32(status);
		txd->addr = cpu_to_le64(mapping);

		tp->tx_skb[entry].len = len;
	}

	if (cur_frag) {
		tp->tx_skb[entry].skb = skb;
		txd->opts1 |= cpu_to_le32(LastFrag);
	}

	return cur_frag;
}

static inline u32 
rtl8168_tso(struct sk_buff *skb, 
	    struct net_device *dev)
{
	if (dev->features & NETIF_F_TSO) {
		u32 mss = skb_shinfo(skb)->gso_size;

		if (mss)
			return LargeSend | ((mss & MSSMask) << MSSShift);
	}

	return 0;
}

static inline u32 
rtl8168_tx_csum(struct sk_buff *skb, 
		struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
	const struct iphdr *ip = skb->nh.iph;
#else
	const struct iphdr *ip = ip_hdr(skb);
#endif

	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		if (ip->protocol == IPPROTO_TCP)
			return tp->tx_tcp_csum_cmd;
		else if (ip->protocol == IPPROTO_UDP)
			return tp->tx_udp_csum_cmd;
		else if (ip->protocol == IPPROTO_IP)
			return tp->tx_ip_csum_cmd;

		WARN_ON(1);	/* we need a WARN() */
	}

	return 0;
}

static int 
rtl8168_start_xmit(struct sk_buff *skb, 
		   struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	unsigned int frags, entry = tp->cur_tx % NUM_TX_DESC;
	struct TxDesc *txd = tp->TxDescArray + entry;
	void __iomem *ioaddr = tp->mmio_addr;
	dma_addr_t mapping;
	u32 status1, status2, len;
	u32 opts1 = 0;
	u32 opts2 = 0;
	int ret = NETDEV_TX_OK;

	//Work around for rx fifo overflow
	if (tp->rx_fifo_overflow == 1)
		goto err_stop;

	if (unlikely(TX_BUFFS_AVAIL(tp) < skb_shinfo(skb)->nr_frags)) {
		if (netif_msg_drv(tp)) {
			printk(KERN_ERR
			       "%s: BUG! Tx Ring full when queue awake!\n",
			       dev->name);
		}
		goto err_stop;
	}

	if (unlikely(le32_to_cpu(txd->opts1) & DescOwn))
		goto err_stop;

	opts1 = DescOwn | rtl8168_tso(skb, dev);

	if (dev->features & NETIF_F_IP_CSUM) {
		if ((tp->mcfg == CFG_METHOD_1) || (tp->mcfg == CFG_METHOD_2) || (tp->mcfg == CFG_METHOD_3))
			opts1 |= rtl8168_tx_csum(skb, dev);
		else
			opts2 = rtl8168_tx_csum(skb, dev);
	}

	frags = rtl8168_xmit_frags(tp, skb, opts1);
	if (frags) {
		len = skb_headlen(skb);
		opts1 |= FirstFrag;
	} else {
		len = skb->len;

		opts1 |= FirstFrag | LastFrag;
		tp->tx_skb[entry].skb = skb;
	}

	mapping = pci_map_single(tp->pci_dev, skb->data, len, PCI_DMA_TODEVICE);

	tp->tx_skb[entry].len = len;
	txd->addr = cpu_to_le64(mapping);
	txd->opts2 = cpu_to_le32(rtl8168_tx_vlan_tag(tp, skb));

	wmb();

	/* anti gcc 2.95.3 bugware (sic) */
	status1 = opts1 | len | (RingEnd * !((entry + 1) % NUM_TX_DESC));
	status2 = opts2;
	txd->opts1 = cpu_to_le32(status1);
	txd->opts2 = cpu_to_le32(status2);

	dev->trans_start = jiffies;

	tp->cur_tx += frags + 1;

	smp_wmb();

	RTL_W8(TxPoll, NPQ);	/* set polling bit */

	if (TX_BUFFS_AVAIL(tp) < MAX_SKB_FRAGS) {
		netif_stop_queue(dev);
		smp_rmb();
		if (TX_BUFFS_AVAIL(tp) >= MAX_SKB_FRAGS)
			netif_wake_queue(dev);
	}

out:
	return ret;
err_stop:
	netif_stop_queue(dev);
	ret = NETDEV_TX_BUSY;
	RTLDEV->stats.tx_dropped++;
	goto out;
}

static void 
rtl8168_pcierr_interrupt(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;
	u16 pci_status, pci_cmd;

	pci_read_config_word(pdev, PCI_COMMAND, &pci_cmd);
	pci_read_config_word(pdev, PCI_STATUS, &pci_status);

	if (netif_msg_intr(tp)) {
		printk(KERN_ERR
		       "%s: PCI error (cmd = 0x%04x, status = 0x%04x).\n",
		       dev->name, pci_cmd, pci_status);
	}

	/*
	 * The recovery sequence below admits a very elaborated explanation:
	 * - it seems to work;
	 * - I did not see what else could be done.
	 *
	 * Feel free to adjust to your needs.
	 */
	pci_write_config_word(pdev, PCI_COMMAND,
			      pci_cmd | PCI_COMMAND_SERR | PCI_COMMAND_PARITY);

	pci_write_config_word(pdev, PCI_STATUS,
		pci_status & (PCI_STATUS_DETECTED_PARITY |
		PCI_STATUS_SIG_SYSTEM_ERROR | PCI_STATUS_REC_MASTER_ABORT |
		PCI_STATUS_REC_TARGET_ABORT | PCI_STATUS_SIG_TARGET_ABORT));

	rtl8168_hw_reset(dev);
}

static void
rtl8168_tx_interrupt(struct net_device *dev, 
		     struct rtl8168_private *tp,
		     void __iomem *ioaddr)
{
	unsigned int dirty_tx, tx_left;

	assert(dev != NULL);
	assert(tp != NULL);
	assert(ioaddr != NULL);

	dirty_tx = tp->dirty_tx;
	smp_rmb();
	tx_left = tp->cur_tx - dirty_tx;

	while (tx_left > 0) {
		unsigned int entry = dirty_tx % NUM_TX_DESC;
		struct ring_info *tx_skb = tp->tx_skb + entry;
		u32 len = tx_skb->len;
		u32 status;

		rmb();
		status = le32_to_cpu(tp->TxDescArray[entry].opts1);
		if (status & DescOwn)
			break;

		RTLDEV->stats.tx_bytes += len;
		RTLDEV->stats.tx_packets++;

		rtl8168_unmap_tx_skb(tp->pci_dev, 
				     tx_skb, 
				     tp->TxDescArray + entry);

		if (status & LastFrag) {
			dev_kfree_skb_irq(tx_skb->skb);
			tx_skb->skb = NULL;
		}
		dirty_tx++;
		tx_left--;
	}

	if (tp->dirty_tx != dirty_tx) {
		tp->dirty_tx = dirty_tx;
		smp_wmb();
		if (netif_queue_stopped(dev) &&
		    (TX_BUFFS_AVAIL(tp) >= MAX_SKB_FRAGS)) {
			netif_wake_queue(dev);
		}
	}
}

static inline int 
rtl8168_fragmented_frame(u32 status)
{
	return (status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag);
}

static inline void 
rtl8168_rx_csum(struct rtl8168_private *tp,
		struct sk_buff *skb, 
		struct RxDesc *desc)
{
	u32 opts1 = le32_to_cpu(desc->opts1);
	u32 opts2 = le32_to_cpu(desc->opts2);
	u32 status = opts1 & RxProtoMask;

	if ((tp->mcfg == CFG_METHOD_1) ||
	    (tp->mcfg == CFG_METHOD_2) ||
	    (tp->mcfg == CFG_METHOD_3)) {
		/* rx csum offload for RTL8168B/8111B */
		if (((status == RxProtoTCP) && !(opts1 & RxTCPF)) ||
		    ((status == RxProtoUDP) && !(opts1 & RxUDPF)) ||
		    ((status == RxProtoIP) && !(opts1 & RxIPF)))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb->ip_summed = CHECKSUM_NONE;
	} else {
		/* rx csum offload for RTL8168C/8111C and RTL8168CP/8111CP */
		if (((status == RxTCPT) && !(opts1 & RxTCPF)) ||
		    ((status == RxUDPT) && !(opts1 & RxUDPF)) ||
		    ((status == 0) && (opts2 & RxV4F) && !(opts1 & RxIPF)))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb->ip_summed = CHECKSUM_NONE;
	}
}

static inline int 
rtl8168_try_rx_copy(struct sk_buff **sk_buff, 
		    int pkt_size,
		    struct RxDesc *desc, 
		    int rx_buf_sz)
{
	int ret = -1;

	if (pkt_size < rx_copybreak) {
		struct sk_buff *skb;

		skb = dev_alloc_skb(pkt_size + NET_IP_ALIGN);
		if (skb) {
			skb_reserve(skb, NET_IP_ALIGN);
			eth_copy_and_sum(skb, sk_buff[0]->data, pkt_size, 0);
			*sk_buff = skb;
			rtl8168_mark_to_asic(desc, rx_buf_sz);
			ret = 0;
		}
	}
	return ret;
}

static int
rtl8168_rx_interrupt(struct net_device *dev, 
		     struct rtl8168_private *tp,
		     void __iomem *ioaddr, u32 budget)
{
	unsigned int cur_rx, rx_left;
	unsigned int delta, count = 0;
	u32 rx_quota = RTL_RX_QUOTA(dev, budget);

	assert(dev != NULL);
	assert(tp != NULL);
	assert(ioaddr != NULL);

	cur_rx = tp->cur_rx;
	rx_left = NUM_RX_DESC + tp->dirty_rx - cur_rx;
	rx_left = rtl8168_rx_quota(rx_left, (u32) rx_quota);

	if ((tp->RxDescArray == NULL) || (tp->Rx_skbuff == NULL)) {
		goto rx_out;
	}

	for (; rx_left > 0; rx_left--, cur_rx++) {
		unsigned int entry = cur_rx % NUM_RX_DESC;
		struct RxDesc *desc = tp->RxDescArray + entry;
		u32 status;

		rmb();
		status = le32_to_cpu(desc->opts1);

		if (status & DescOwn)
			break;
		if (unlikely(status & RxRES)) {
			if (netif_msg_rx_err(tp)) {
				printk(KERN_INFO
				       "%s: Rx ERROR. status = %08x\n",
				       dev->name, status);
			}

			RTLDEV->stats.rx_errors++;

			if (status & (RxRWT | RxRUNT))
				RTLDEV->stats.rx_length_errors++;
			if (status & RxCRC)
				RTLDEV->stats.rx_crc_errors++;
			rtl8168_mark_to_asic(desc, tp->rx_buf_sz);
		} else {
			struct sk_buff *skb = tp->Rx_skbuff[entry];
			int pkt_size = (status & 0x00003FFF) - 4;
			void (*pci_action)(struct pci_dev *, dma_addr_t,
				size_t, int) = pci_dma_sync_single_for_device;

			/*
			 * The driver does not support incoming fragmented
			 * frames. They are seen as a symptom of over-mtu
			 * sized frames.
			 */
			if (unlikely(rtl8168_fragmented_frame(status))) {
				RTLDEV->stats.rx_dropped++;
				RTLDEV->stats.rx_length_errors++;
				rtl8168_mark_to_asic(desc, tp->rx_buf_sz);
				continue;
			}

			if (tp->cp_cmd & RxChkSum)
				rtl8168_rx_csum(tp, skb, desc);
			
			pci_dma_sync_single_for_cpu(tp->pci_dev,
				le64_to_cpu(desc->addr), tp->rx_buf_sz,
				PCI_DMA_FROMDEVICE);

			if (rtl8168_try_rx_copy(&skb, pkt_size, desc,
						tp->rx_buf_sz)) {
				pci_action = pci_unmap_single;
				tp->Rx_skbuff[entry] = NULL;
			}

			pci_action(tp->pci_dev, le64_to_cpu(desc->addr),
				   tp->rx_buf_sz, PCI_DMA_FROMDEVICE);

			skb->dev = dev;
			skb_put(skb, pkt_size);
			skb->protocol = eth_type_trans(skb, dev);

			if (rtl8168_rx_vlan_skb(tp, desc, skb) < 0)
				rtl8168_rx_skb(skb);

			dev->last_rx = jiffies;
			RTLDEV->stats.rx_bytes += pkt_size;
			RTLDEV->stats.rx_packets++;
		}
	}

	count = cur_rx - tp->cur_rx;
	tp->cur_rx = cur_rx;

	delta = rtl8168_rx_fill(tp, dev, tp->dirty_rx, tp->cur_rx);
	if (!delta && count && netif_msg_intr(tp))
		printk(KERN_INFO "%s: no Rx buffer allocated\n", dev->name);
	tp->dirty_rx += delta;

	/*
	 * FIXME: until there is periodic timer to try and refill the ring,
	 * a temporary shortage may definitely kill the Rx process.
	 * - disable the asic to try and avoid an overflow and kick it again
	 *   after refill ?
	 * - how do others driver handle this condition (Uh oh...).
	 */
	if ((tp->dirty_rx + NUM_RX_DESC == tp->cur_rx) && netif_msg_intr(tp))
		printk(KERN_EMERG "%s: Rx buffers exhausted\n", dev->name);

rx_out:
	return count;
}

/* 
 *The interrupt handler does all of the Rx thread work and cleans up after 
 *the Tx thread.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8168_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#else
static irqreturn_t rtl8168_interrupt(int irq, void *dev_instance)
#endif
{
	struct net_device *dev = (struct net_device *) dev_instance;
	struct rtl8168_private *tp = netdev_priv(dev);
	int boguscnt = max_interrupt_work;
	void __iomem *ioaddr = tp->mmio_addr;
	int status;
	int handled = 0;
	u16 intr_clean_mask = SYSErr | PCSTimeout | SWInt | 
			      LinkChg | RxDescUnavail | 
			      TxErr | TxOK | RxErr | RxOK;

	RTL_W16(IntrMask, 0x0000);

	do {
		status = RTL_R16(IntrStatus);

		/* hotplug/major error/no more work/shared irq */
		if ((status == 0xFFFF) || !status)
			break;

		handled = 1;

		if (unlikely(!netif_running(dev))) {
			rtl8168_asic_down(dev);
			goto out;
		}

		status &= (tp->intr_mask | TxDescUnavail);
		RTL_W16(IntrStatus, intr_clean_mask);

		if (!(status & rtl8168_intr_mask))
			break;

		//Work around for rx fifo overflow
		if (unlikely(status & RxFIFOOver))
			if (tp->mcfg == CFG_METHOD_1) {
				tp->rx_fifo_overflow = 1;
				netif_stop_queue(dev);
				udelay(300);
				rtl8168_rx_clear(tp);
				rtl8168_init_ring(dev);
				rtl8168_hw_start(dev);
				RTL_W16(IntrStatus, RxFIFOOver);
				netif_wake_queue(dev);
				tp->rx_fifo_overflow = 0;
			}

		if (unlikely(status & SYSErr)) {
			rtl8168_pcierr_interrupt(dev);
			break;
		}

		if (status & LinkChg)
			rtl8168_check_link_status(dev, tp, ioaddr);

		if ((status & TxOK) && (status & TxDescUnavail)) {
			RTL_W8(TxPoll, NPQ);	/* set polling bit */
			RTL_W16(IntrStatus, TxDescUnavail);
		}
#ifdef CONFIG_R8168_NAPI
		if (status & rtl8168_napi_event) {
			tp->intr_mask = rtl8168_intr_mask & ~rtl8168_napi_event;
			RTL_W16(IntrMask, rtl8168_intr_mask & tp->intr_mask);

			if (likely(RTL_NETIF_RX_SCHEDULE_PREP(dev, &tp->napi))) {
				__RTL_NETIF_RX_SCHEDULE(dev, &tp->napi);
			} else if (netif_msg_intr(tp)) {
				printk(KERN_INFO "%s: interrupt %04x in poll\n",
				       dev->name, status);
			}
		}
		break;
#else
		/* Rx interrupt */
		if (status & (RxOK | RxDescUnavail | RxFIFOOver)) {
			rtl8168_rx_interrupt(dev, tp, tp->mmio_addr, ~(u32)0);
		}
		/* Tx interrupt */
		if (status & (TxOK | TxErr))
			rtl8168_tx_interrupt(dev, tp, ioaddr);
#endif

		boguscnt--;
	} while (boguscnt > 0);

	if (boguscnt <= 0) {
		if (netif_msg_intr(tp) && net_ratelimit() ) {
			printk(KERN_WARNING
			       "%s: Too much work at interrupt!\n", dev->name);
		}
		/* Clear all interrupt sources. */
		RTL_W16(IntrStatus, 0xffff);
	}

out:
	RTL_W16(IntrMask, tp->intr_mask);

	return IRQ_RETVAL(handled);
}

#ifdef CONFIG_R8168_NAPI
static int rtl8168_poll(napi_ptr napi, napi_budget budget)
{
	struct rtl8168_private *tp = RTL_GET_PRIV(napi, struct rtl8168_private);
	void __iomem *ioaddr = tp->mmio_addr;
	RTL_GET_NETDEV(tp)
	unsigned int work_to_do = RTL_NAPI_QUOTA(budget, dev);
	unsigned int work_done;

	work_done = rtl8168_rx_interrupt(dev, tp, ioaddr, (u32) budget);
	rtl8168_tx_interrupt(dev, tp, ioaddr);

	RTL_NAPI_QUOTA_UPDATE(dev, work_done, budget);

	if (work_done < work_to_do) {
		RTL_NETIF_RX_COMPLETE(dev, napi);
		tp->intr_mask = rtl8168_intr_mask;
		/*
		 * 20040426: the barrier is not strictly required but the
		 * behavior of the irq handler could be less predictable
		 * without it. Btw, the lack of flush for the posted pci
		 * write is safe - FR
		 */
		smp_wmb();
		RTL_W16(IntrMask, rtl8168_intr_mask);
	}

	return RTL_NAPI_RETURN_VALUE;
}
#endif//CONFIG_R8168_NAPI

static void 
rtl8168_down(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned int poll_locked = 0;

	rtl8168_dsm(dev, DSM_IF_DOWN);

	rtl8168_powerdown_pll(dev);

	netif_stop_queue(dev);

	rtl8168_delete_esd_timer(dev, &tp->esd_timer);
	rtl8168_delete_link_timer(dev, &tp->link_timer);

	flush_scheduled_work();

#ifdef CONFIG_R8168_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
	napi_disable(&tp->napi);
#endif
#endif//CONFIG_R8168_NAPI

core_down:
	spin_lock_irq(&tp->lock);

	rtl8168_asic_down(dev);

	spin_unlock_irq(&tp->lock);

	synchronize_irq(dev->irq);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,23)) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
	if (!poll_locked) {
		netif_poll_disable(dev);
		poll_locked++;
	}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
	/* Give a racing hard_start_xmit a few cycles to complete. */
	synchronize_sched();  /* FIXME: should this be synchronize_irq()? */
#endif

	/*
	 * And now for the 50k$ question: are IRQ disabled or not ?
	 *
	 * Two paths lead here:
	 * 1) dev->close
	 *    -> netif_running() is available to sync the current code and the
	 *       IRQ handler. See rtl8168_interrupt for details.
	 * 2) dev->change_mtu
	 *    -> rtl8168_poll can not be issued again and re-enable the
	 *       interruptions. Let's simply issue the IRQ down sequence again.
	 */
	if (RTL_R16(IntrMask))
		goto core_down;

	rtl8168_tx_clear(tp);

	rtl8168_rx_clear(tp);
}

static int 
rtl8168_close(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;

	rtl8168_down(dev);

	free_irq(dev->irq, dev);

	pci_free_consistent(pdev, R8168_RX_RING_BYTES, tp->RxDescArray,
			    tp->RxPhyAddr);
	pci_free_consistent(pdev, R8168_TX_RING_BYTES, tp->TxDescArray,
			    tp->TxPhyAddr);
	tp->TxDescArray = NULL;
	tp->RxDescArray = NULL;

	return 0;
}

static void
rtl8168_set_rx_mode(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;
	u32 mc_filter[2];	/* Multicast hash filter */
	int i, j, k, rx_mode;
	u32 tmp = 0;

	if (dev->flags & IFF_PROMISC) {
		/* Unconditionally log net taps. */
		if (netif_msg_link(tp)) {
			printk(KERN_NOTICE "%s: Promiscuous mode enabled.\n",
			       dev->name);
		}
		rx_mode =
		    AcceptBroadcast | AcceptMulticast | AcceptMyPhys |
		    AcceptAllPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else if ((dev->mc_count > multicast_filter_limit)
		   || (dev->flags & IFF_ALLMULTI)) {
		/* Too many to filter perfectly -- accept all multicasts. */
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		struct dev_mc_list *mclist;
		rx_mode = AcceptBroadcast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
		     i++, mclist = mclist->next) {
			int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;
			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
			rx_mode |= AcceptMulticast;
		}
	}

	spin_lock_irqsave(&tp->lock, flags);


	tp->rtl8168_rx_config = rtl_chip_info[tp->chipset].RCR_Cfg;
	tmp = tp->rtl8168_rx_config | rx_mode | (RTL_R32(RxConfig) & rtl_chip_info[tp->chipset].RxConfigMask);

	for (j = 0; j < 2; j++) {
		u32 mask = 0x000000ff;
		u32 tmp1 = 0;
		u32 tmp2 = 0;
		int x = 0;
		int y = 0;

		for (k = 0; k < 4; k++) {
			tmp1 = mc_filter[j] & mask;
			x = 32 - (8 + 16 * k);
			y = x - 2 * x;
			
			if (x > 0)
				tmp2 = tmp2 | (tmp1 << x);
			else
				tmp2 = tmp2 | (tmp1 >> y);

			mask = mask << 8;
		}
		mc_filter[j] = tmp2;
	}

	RTL_W32(RxConfig, tmp);
	RTL_W32(MAR0 + 0, mc_filter[1]);
	RTL_W32(MAR0 + 4, mc_filter[0]);

	spin_unlock_irqrestore(&tp->lock, flags);
}

/**
 *  rtl8168_get_stats - Get rtl8168 read/write statistics
 *  @dev: The Ethernet Device to get statistics for
 *
 *  Get TX/RX statistics for rtl8168
 */
static struct 
net_device_stats *rtl8168_get_stats(struct net_device *dev)
{
	struct rtl8168_private *tp = netdev_priv(dev);
	unsigned long flags;

	if (netif_running(dev)) {
		spin_lock_irqsave(&tp->lock, flags);
		spin_unlock_irqrestore(&tp->lock, flags);
	}
		
	return &RTLDEV->stats;
}

#ifdef CONFIG_PM

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
static int 
rtl8168_suspend(struct pci_dev *pdev, 
		u32 state)
#else
static int 
rtl8168_suspend(struct pci_dev *pdev, 
		pm_message_t state)
#endif
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rtl8168_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	u32 pci_pm_state = pci_choose_state(pdev, state);
#endif

	if (!netif_running(dev))
		goto out;

	rtl8168_dsm(dev, DSM_NIC_GOTO_D3);

	rtl8168_powerdown_pll(dev);

	netif_device_detach(dev);
	netif_stop_queue(dev);

	spin_lock_irq(&tp->lock);

	rtl8168_asic_down(dev);

	if ((tp->mcfg == CFG_METHOD_1) || (tp->mcfg == CFG_METHOD_2))
		RTL_W8(ChipCmd, CmdRxEnb);

	spin_unlock_irq(&tp->lock);

out:

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	pci_save_state(pdev, &pci_pm_state);
#else
	pci_save_state(pdev);
#endif
	pci_enable_wake(pdev, pci_choose_state(pdev, state), tp->wol_enabled);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));

	return 0;
}

static int 
rtl8168_resume(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	u32 pci_pm_state = PCI_D0;
#endif

	pci_set_power_state(pdev, PCI_D0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	pci_restore_state(pdev, &pci_pm_state);
#else
	pci_restore_state(pdev);
#endif
	pci_enable_wake(pdev, PCI_D0, 0);

	if (!netif_running(dev))
		goto out;

	rtl8168_dsm(dev, DSM_NIC_RESUME_D3);

	netif_device_attach(dev);

	rtl8168_schedule_work(dev, rtl8168_reset_task);

	rtl8168_powerup_pll(dev);
out:
	return 0;
}

#endif /* CONFIG_PM */

static struct pci_driver rtl8168_pci_driver = {
	.name		= MODULENAME,
	.id_table	= rtl8168_pci_tbl,
	.probe		= rtl8168_init_one,
	.remove		= __devexit_p(rtl8168_remove_one),
#ifdef CONFIG_PM
	.suspend	= rtl8168_suspend,
	.resume		= rtl8168_resume,
#endif
};

static int __init
rtl8168_init_module(void)
{
	return pci_register_driver(&rtl8168_pci_driver);
}

static void __exit
rtl8168_cleanup_module(void)
{
	pci_unregister_driver(&rtl8168_pci_driver);
}

module_init(rtl8168_init_module);
module_exit(rtl8168_cleanup_module);
