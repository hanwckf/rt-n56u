#ifndef RA2882ETHEND_H
#define RA2882ETHEND_H

#define TX_TIMEOUT	(6*HZ)		/* netdev watchdog timeout */

#define DEFAULT_MTU	1500		/* default MTU set to device */

/* mtu and rx sizes */
#if defined (CONFIG_RAETH_JUMBOFRAME)
#define	MAX_RX_LENGTH	4096		/* limit size for rx packets 1Gb */
#else
#define	MAX_RX_LENGTH	1536		/* limit size for rx packets 100Mb */
#endif

#ifdef DSP_VIA_NONCACHEABLE
#define ESRAM_BASE	0xa0800000	/* 0x0080-0000  ~ 0x00807FFF */
#else
#define ESRAM_BASE	0x80800000	/* 0x0080-0000  ~ 0x00807FFF */
#endif

#define RX_RING_BASE	((int)(ESRAM_BASE + 0x7000))
#define TX_RING_BASE	((int)(ESRAM_BASE + 0x7800))

#if defined(CONFIG_RALINK_RT2880)
#define NUM_TX_RINGS 	1
#else
#define NUM_TX_RINGS 	4
#endif

#ifdef CONFIG_RAETH_MEMORY_OPTIMIZATION
#ifdef CONFIG_RAETH_ROUTER
#define NUM_RX_DESC     128
#define NUM_TX_DESC    	128
#elif defined CONFIG_RT_3052_ESW
#define NUM_RX_DESC     64
#define NUM_TX_DESC     64
#else
#define NUM_RX_DESC     128
#define NUM_TX_DESC     128
#endif
#else
#if defined(CONFIG_GE1_RGMII_FORCE_1000) || defined(CONFIG_GE2_RGMII_FORCE_1000)
#define NUM_RX_DESC	256
#define NUM_TX_DESC	256
#elif defined CONFIG_RT_3052_ESW /* for 305x/335x ralink say max=128 */
#define NUM_RX_DESC     128
#define NUM_TX_DESC     128
#else
#define NUM_RX_DESC	256
#define NUM_TX_DESC	256
#endif
#endif

#if defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_RT_3052_ESW)
#if defined(CONFIG_BRIDGE_FASTPATH)
#define	DEV_WEIGHT	64
#else
#define	DEV_WEIGHT	32
#endif
#elif defined(CONFIG_GE1_RGMII_FORCE_1000) || defined(CONFIG_GE2_RGMII_FORCE_1000)
#define	DEV_WEIGHT	128
#else
#define	DEV_WEIGHT	128
#endif

#ifndef CONFIG_RAETH_NAPI
#if defined(CONFIG_RALINK_RT3883) || defined(CONFIG_RALINK_RT6352)
#define NUM_RX_MAX_PROCESS 2
#else
#define NUM_RX_MAX_PROCESS 16
#endif
#endif

#define DEV_NAME        "eth2"
#define DEV2_NAME       "eth3"

#define GMAC2_OFFSET    0x22
#if ! defined (CONFIG_RALINK_RT6855A)
#define GMAC0_OFFSET    0x28 
#else
#define GMAC0_OFFSET    0xE000
#endif
#define GMAC1_OFFSET    0x2E

#if defined(CONFIG_RALINK_RT6855A)
#define IRQ_ENET0	22
#else
#define IRQ_ENET0	3 	/* hardware interrupt #3, defined in RT2880 Soc Design Spec Rev 0.03, pp43 */
#endif

#define FE_INT_STATUS_REG (*(volatile unsigned long *)(FE_INT_STATUS))
#define FE_INT_STATUS_CLEAN(reg) (*(volatile unsigned long *)(FE_INT_STATUS)) = reg

//#define RAETH_DEBUG
#ifdef RAETH_DEBUG
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif

struct net_device_stats *ra_get_stats(struct net_device *dev);

void ei_tx_timeout(struct net_device *dev);
int rather_probe(struct net_device *dev);
int ei_open(struct net_device *dev);
int ei_close(struct net_device *dev);

int ra2882eth_init(void);
void ra2882eth_cleanup_module(void);

#ifdef WORKQUEUE_BH
void ei_xmit_housekeeping(struct work_struct *work);
#else
inline void ei_xmit_housekeeping(unsigned long unused);
#endif // WORKQUEUE_BH //

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);
void fe_sw_init(void);

#endif
