#ifndef RA2882ETHREG_H
#define RA2882ETHREG_H

#include <linux/mii.h>         /* for struct mii_if_info in ra2882ethreg.h */
#include <linux/version.h>	/* check linux version for 2.4 and 2.6 compatibility */
#include <linux/cache.h>       /* for L1 cacheline size */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#endif
#include "raether.h"

#ifdef WORKQUEUE_BH
#include <linux/workqueue.h>
#endif // WORKQUEUE_BH //

#define phys_to_bus(a) (a & 0x1FFFFFFF)

#ifndef BIT
#define BIT(x)	((1 << x))
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN  6
#endif

/*  Phy Vender ID list */

#define EV_MARVELL_PHY_ID0 0x0141  
#define EV_MARVELL_PHY_ID1 0x0CC2  
#define EV_VTSS_PHY_ID0 0x0007
#define EV_VTSS_PHY_ID1 0x0421

/*
     FE_INT_STATUS
*/
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT63365)

#define RX_COHERENT      BIT(31)
#define RX_DLY_INT       BIT(30)
#define TX_COHERENT      BIT(29)
#define TX_DLY_INT       BIT(28)

#define RX_DONE_INT1     BIT(17)
#define RX_DONE_INT0     BIT(16)

#define TX_DONE_INT3     BIT(3)
#define TX_DONE_INT2     BIT(2)
#define TX_DONE_INT1     BIT(1)
#define TX_DONE_INT0     BIT(0)
#else
//#define CNT_PPE_AF       BIT(31)     
//#define CNT_GDM_AF       BIT(29)
#define PSE_P2_FC	 BIT(26)
#define GDM_CRC_DROP     BIT(25)
#define PSE_BUF_DROP     BIT(24)
#define GDM_OTHER_DROP	 BIT(23)
#define PSE_P1_FC        BIT(22)
#define PSE_P0_FC        BIT(21)
#define PSE_FQ_EMPTY     BIT(20)
#define GE1_STA_CHG      BIT(18)
#define TX_COHERENT      BIT(17)
#define RX_COHERENT      BIT(16)

#define TX_DONE_INT3     BIT(11)
#define TX_DONE_INT2     BIT(10)
#define TX_DONE_INT1     BIT(9)
#define TX_DONE_INT0     BIT(8)
#define RX_DONE_INT1     RX_DONE_INT0
#define RX_DONE_INT0     BIT(2)
#define TX_DLY_INT       BIT(1)
#define RX_DLY_INT       BIT(0)
#endif

#define FE_INT_ALL		(TX_DONE_INT3 | TX_DONE_INT2 | \
			         TX_DONE_INT1 | TX_DONE_INT0 | \
	                         RX_DONE_INT0 )
/*
 * SW_INT_STATUS
 */
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
#define PORT0_QUEUE_FULL        BIT(14) //port0 queue full
#define PORT1_QUEUE_FULL        BIT(15) //port1 queue full
#define PORT2_QUEUE_FULL        BIT(16) //port2 queue full
#define PORT3_QUEUE_FULL        BIT(17) //port3 queue full
#define PORT4_QUEUE_FULL        BIT(18) //port4 queue full
#define PORT5_QUEUE_FULL        BIT(19) //port5 queue full
#define PORT6_QUEUE_FULL        BIT(20) //port6 queue full
#define SHARED_QUEUE_FULL       BIT(23) //shared queue full
#define QUEUE_EXHAUSTED         BIT(24) //global queue is used up and all packets are dropped
#define BC_STROM                BIT(25) //the device is undergoing broadcast storm
#define PORT_ST_CHG             BIT(26) //Port status change
#define UNSECURED_ALERT         BIT(27) //Intruder alert
#define ABNORMAL_ALERT          BIT(28) //Abnormal

#define ESW_ISR			(RALINK_ETH_SW_BASE + 0x00)
#define ESW_IMR			(RALINK_ETH_SW_BASE + 0x04)
#define ESW_INT_ALL		(PORT_ST_CHG)

#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT63365)

#define ACL_INT			BIT(15)
#define P5_LINK_CH		BIT(5)
#define P4_LINK_CH		BIT(4)
#define P3_LINK_CH		BIT(3)
#define P2_LINK_CH		BIT(2)
#define P1_LINK_CH		BIT(1)
#define P0_LINK_CH		BIT(0)

#define ESW_IMR			(RALINK_ETH_SW_BASE + 0x7000 + 0x8)
#define ESW_ISR			(RALINK_ETH_SW_BASE + 0x7000 + 0xC)
#define ESW_INT_ALL		(P0_LINK_CH | P1_LINK_CH | P2_LINK_CH | P3_LINK_CH | P4_LINK_CH | P5_LINK_CH | ACL_INT)
#define ESW_AISR		(RALINK_ETH_SW_BASE + 0x8)

#endif // CONFIG_RALINK_RT3052 || CONFIG_RALINK_RT3352 || CONFIG_RALINK_RT5350 //

/*  Align on a cache line */
#define ETHER_BUFFER_ALIGN     L1_CACHE_BYTES

#define ETHER_ALIGNED_RX_SKB_ADDR(addr) \
        ((((unsigned long)(addr) + ETHER_BUFFER_ALIGN - 1) & \
        ~(ETHER_BUFFER_ALIGN - 1)) - (unsigned long)(addr))

#ifdef CONFIG_PSEUDO_SUPPORT
typedef struct _PSEUDO_ADAPTER {
    struct net_device *RaethDev;
    struct net_device *PseudoDev;
    struct net_device_stats stat;
#if defined (CONFIG_ETHTOOL)
	struct mii_if_info	mii_info;
#endif

} PSEUDO_ADAPTER, PPSEUDO_ADAPTER;

#define MAX_PSEUDO_ENTRY               1
#endif



/* Register Categories Definition */
#define RAFRAMEENGINE_OFFSET	0x0000
#define RAGDMA_OFFSET		0x0020
#define RAPSE_OFFSET		0x0040
#define RAGDMA2_OFFSET		0x0060
#define RACDMA_OFFSET		0x0080
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT63365)
#define RAPDMA_OFFSET		0x0800
#define SDM_OFFSET		0x0C00
#else
#define RAPDMA_OFFSET		0x0100
#endif
#define RAPPE_OFFSET		0x0200
#define RACMTABLE_OFFSET	0x0400
#define RAPOLICYTABLE_OFFSET	0x1000


/* Register Map Detail */
/* RT3883 */
#define SYSCFG1			(RALINK_SYSCTL_BASE + 0x14)

#if defined (CONFIG_RALINK_RT5350)

/* 1. PDMA */
#define TX_BASE_PTR0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x000)
#define TX_MAX_CNT0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x004)
#define TX_CTX_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x008)
#define TX_DTX_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x00C)

#define TX_BASE_PTR1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x010)
#define TX_MAX_CNT1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x014)
#define TX_CTX_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x018)
#define TX_DTX_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x01C)

#define TX_BASE_PTR2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x020)
#define TX_MAX_CNT2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x024)
#define TX_CTX_IDX2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x028)
#define TX_DTX_IDX2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x02C)

#define TX_BASE_PTR3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x030)
#define TX_MAX_CNT3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x034)
#define TX_CTX_IDX3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x038)
#define TX_DTX_IDX3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x03C)

#define RX_BASE_PTR0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x100)
#define RX_MAX_CNT0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x104)
#define RX_CALC_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x108)
#define RX_DRX_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x10C)

#define RX_BASE_PTR1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x110)
#define RX_MAX_CNT1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x114)
#define RX_CALC_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x118)
#define RX_DRX_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x11C)

#define PDMA_INFO		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x200)
#define PDMA_GLO_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x204)
#define PDMA_RST_IDX		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x208)
#define PDMA_RST_CFG		(PDMA_RST_IDX)
#define DLY_INT_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x20C)
#define FREEQ_THRES		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x210)
#define INT_STATUS		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x220)
#define FE_INT_STATUS		(INT_STATUS)
#define INT_MASK		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x228)
#define FE_INT_ENABLE		(INT_MASK)
#define PDMA_WRR		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x280)
#define PDMA_SCH_CFG		(PDMA_WRR)

#define SDM_CON			(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x00)  //Switch DMA configuration
#define SDM_RRING		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x04)  //Switch DMA Rx Ring
#define SDM_TRING		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x08)  //Switch DMA Tx Ring
#define SDM_MAC_ADRL		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x0C)  //Switch MAC address LSB
#define SDM_MAC_ADRH		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x10)  //Switch MAC Address MSB
#define SDM_TPCNT		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x100) //Switch DMA Tx packet count
#define SDM_TBCNT		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x104) //Switch DMA Tx byte count
#define SDM_RPCNT		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x108) //Switch DMA rx packet count
#define SDM_RBCNT		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x10C) //Switch DMA rx byte count
#define SDM_CS_ERR		(RALINK_FRAME_ENGINE_BASE+SDM_OFFSET+0x110) //Switch DMA rx checksum error count

#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT63365)
/* Old FE with New PDMA */
#define PDMA_RELATED            0x0800
/* 1. PDMA */
#define TX_BASE_PTR0            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x000)
#define TX_MAX_CNT0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x004)
#define TX_CTX_IDX0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x008)
#define TX_DTX_IDX0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x00C)

#define TX_BASE_PTR1            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x010)
#define TX_MAX_CNT1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x014)
#define TX_CTX_IDX1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x018)
#define TX_DTX_IDX1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x01C)

#define TX_BASE_PTR2            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x020)
#define TX_MAX_CNT2             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x024)
#define TX_CTX_IDX2             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x028)
#define TX_DTX_IDX2             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x02C)

#define TX_BASE_PTR3            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x030)
#define TX_MAX_CNT3             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x034)
#define TX_CTX_IDX3             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x038)
#define TX_DTX_IDX3             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x03C)

#define RX_BASE_PTR0            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x100)
#define RX_MAX_CNT0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x104)
#define RX_CALC_IDX0            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x108)
#define RX_DRX_IDX0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x10C)

#define RX_BASE_PTR1            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x110)
#define RX_MAX_CNT1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x114)
#define RX_CALC_IDX1            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x118)
#define RX_DRX_IDX1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x11C)

#define PDMA_INFO               (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x200)
#define PDMA_GLO_CFG            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x204)
#define PDMA_RST_IDX            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x208)
#define PDMA_RST_CFG            (RALINK_FRAME_ENGINE_BASE + PDMA_RST_IDX)
#define DLY_INT_CFG             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x20C)
#define FREEQ_THRES             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x210)
#define INT_STATUS              (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x220)
#define FE_INT_STATUS		(INT_STATUS)
#define INT_MASK                (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x228)
#define FE_INT_ENABLE		(INT_MASK)
#define SCH_Q01_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x280)
#define SCH_Q23_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x284)

#define FE_GLO_CFG          RALINK_FRAME_ENGINE_BASE + 0x08
#define FE_RST_GL           RALINK_FRAME_ENGINE_BASE + 0x0C
#define FE_INT_STATUS2	    RALINK_FRAME_ENGINE_BASE + 0x10
#define FE_INT_ENABLE2	    RALINK_FRAME_ENGINE_BASE + 0x14
#define FC_DROP_STA         RALINK_FRAME_ENGINE_BASE + 0x18
#define FOE_TS_T            RALINK_FRAME_ENGINE_BASE + 0x1C

#define GDMA1_RELATED       0x0020
#define GDMA1_FWD_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x00)
#define GDMA1_SCH_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x04)
#define GDMA1_SHPR_CFG      (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x08)
#define GDMA1_MAC_ADRL      (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x0C)
#define GDMA1_MAC_ADRH      (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x10)

#define GDMA2_RELATED       0x0060
#define GDMA2_FWD_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x00)
#define GDMA2_SCH_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x04)
#define GDMA2_SHPR_CFG      (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x08)
#define GDMA2_MAC_ADRL      (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x0C)
#define GDMA2_MAC_ADRH      (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x10)

#define PSE_RELATED         0x0040
#define PSE_FQ_CFG        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x00)
#define CDMA_FC_CFG         (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x04)
#define GDMA1_FC_CFG        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x08)
#define GDMA2_FC_CFG        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x0C)
#define CDMA_OQ_STA         (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x10)
#define GDMA1_OQ_STA        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x14)
#define GDMA2_OQ_STA        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x18)
#define PSE_IQ_STA          (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x1C)

#define CDMA_RELATED        0x0080
#define CDMA_CSG_CFG        (RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00)
#define CDMA_SCH_CFG        (RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x04)

#define PDMA_FC_CFG	    (RALINK_FRAME_ENGINE_BASE+0x100)

#define SMACCR0		    (RALINK_ETH_SW_BASE + 0x30E4)
#define SMACCR1		    (RALINK_ETH_SW_BASE + 0x30E8)

#else

/* 1. Frame Engine Global Registers */
#define MDIO_ACCESS		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x00)
#define MDIO_CFG 		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x04)
#define MDIO_CFG2 		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x18)
#define FE_GLO_CFG		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x08)
#define FE_RST_GL		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x0C)
#define FE_INT_STATUS		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x10)
#define FE_INT_ENABLE		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x14)
#define MDIO_CFG2		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x18) //Original:FC_DROP_STA
#define FOC_TS_T		(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x1C)


/* 2. GDMA Registers */
#define	GDMA1_FWD_CFG		(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x00)
#define GDMA1_SCH_CFG		(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x04)
#define GDMA1_SHPR_CFG		(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x08)
#define GDMA1_MAC_ADRL		(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x0C)
#define GDMA1_MAC_ADRH		(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x10)

#define	GDMA2_FWD_CFG		(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x00)
#define GDMA2_SCH_CFG		(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x04)
#define GDMA2_SHPR_CFG		(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x08)
#define GDMA2_MAC_ADRL		(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x0C)
#define GDMA2_MAC_ADRH		(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x10)

/* 3. PSE */
#define PSE_FQ_CFG		(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x00)
#define CDMA_FC_CFG		(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x04)
#define GDMA1_FC_CFG		(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x08)
#define GDMA2_FC_CFG		(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x0C)
#define PDMA_FC_CFG		(RALINK_FRAME_ENGINE_BASE+0x1f0)

/* 4. CDMA */
#define CDMA_CSG_CFG		(RALINK_FRAME_ENGINE_BASE+RACDMA_OFFSET+0x00)
#define CDMA_SCH_CFG		(RALINK_FRAME_ENGINE_BASE+RACDMA_OFFSET+0x04)
/* skip ppoe sid and vlan id definition */


/* 5. PDMA */
#define PDMA_GLO_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x00)
#define PDMA_RST_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x04)
#define PDMA_SCH_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x08)

#define DLY_INT_CFG		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x0C)

#define TX_BASE_PTR0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x10)
#define TX_MAX_CNT0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x14)
#define TX_CTX_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x18)
#define TX_DTX_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x1C)

#define TX_BASE_PTR1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x20)
#define TX_MAX_CNT1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x24)
#define TX_CTX_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x28)
#define TX_DTX_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x2C)

#define TX_BASE_PTR2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x40)
#define TX_MAX_CNT2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x44)
#define TX_CTX_IDX2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x48)
#define TX_DTX_IDX2		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x4C)

#define TX_BASE_PTR3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x50)
#define TX_MAX_CNT3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x54)
#define TX_CTX_IDX3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x58)
#define TX_DTX_IDX3		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x5C)

#define RX_BASE_PTR0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x30)
#define RX_MAX_CNT0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x34)
#define RX_CALC_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x38)
#define RX_DRX_IDX0		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x3C)

#define RX_BASE_PTR1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x40)
#define RX_MAX_CNT1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x44)
#define RX_CALC_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x48)
#define RX_DRX_IDX1		(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x4C)

#endif

#define DELAY_INT_INIT		0x84048404
#define FE_INT_DLY_INIT		(TX_DLY_INT | RX_DLY_INT)


#if !defined (CONFIG_RALINK_RT5350)

/* 6. Counter and Meter Table */
#define PPE_AC_BCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x000) /* PPE Accounting Group 0 Byte Cnt */
#define PPE_AC_PCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x004) /* PPE Accounting Group 0 Packet Cnt */
/* 0 ~ 63 */

#define PPE_MTR_CNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x200) /* 0 ~ 63 */
/* skip... */
#define PPE_MTR_CNT63		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x2FC)

#define GDMA_TX_GBCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x300) /* Transmit good byte cnt for GEport */
#define GDMA_TX_GPCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x304) /* Transmit good pkt cnt for GEport */
#define GDMA_TX_SKIPCNT0	(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x308) /* Transmit skip cnt for GEport */
#define GDMA_TX_COLCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x30C) /* Transmit collision cnt for GEport */

/* update these address mapping to fit data sheet v0.26, by bobtseng, 2007.6.14 */
#define GDMA_RX_GBCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x320)
#define GDMA_RX_GPCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x324)
#define GDMA_RX_OERCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x328)
#define GDMA_RX_FERCNT0 	(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x32C)
#define GDMA_RX_SERCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x330)
#define GDMA_RX_LERCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x334)
#define GDMA_RX_CERCNT0		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x338)
#define GDMA_RX_FCCNT1		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x33C)

#endif


/* Per Port Packet Counts in RT3052, added by bobtseng 2009.4.17. */
#define	PORT0_PKCOUNT		(0xb01100e8)
#define	PORT1_PKCOUNT		(0xb01100ec)
#define	PORT2_PKCOUNT		(0xb01100f0)
#define	PORT3_PKCOUNT		(0xb01100f4)
#define	PORT4_PKCOUNT		(0xb01100f8)
#define	PORT5_PKCOUNT		(0xb01100fc)


// PHYS_TO_K1
#define PHYS_TO_K1(physaddr) KSEG1ADDR(physaddr)


#define sysRegRead(phys)        \
        (*(volatile unsigned int *)PHYS_TO_K1(phys))

#define sysRegWrite(phys, val)  \
        ((*(volatile unsigned int *)PHYS_TO_K1(phys)) = (val))

#define u_long	unsigned long
#define u32	unsigned int
#define u16	unsigned short


/* ====================================== */
#define GDM1_DISPAD       BIT(18)
#define GDM1_DISCRC       BIT(17)

//GDMA1 uni-cast frames destination port
#define GDM1_ICS_EN   	   (0x1 << 22)
#define GDM1_TCS_EN   	   (0x1 << 21)
#define GDM1_UCS_EN   	   (0x1 << 20)
#define GDM1_JMB_EN   	   (0x1 << 19)
#define GDM1_STRPCRC   	   (0x1 << 16)
#define GDM1_UFRC_P_CPU     (0 << 12)
#define GDM1_UFRC_P_GDMA1   (1 << 12)
#define GDM1_UFRC_P_PPE     (6 << 12)

//GDMA1 broad-cast MAC address frames
#define GDM1_BFRC_P_CPU     (0 << 8)
#define GDM1_BFRC_P_GDMA1   (1 << 8)
#define GDM1_BFRC_P_PPE     (6 << 8)

//GDMA1 multi-cast MAC address frames
#define GDM1_MFRC_P_CPU     (0 << 4)
#define GDM1_MFRC_P_GDMA1   (1 << 4)
#define GDM1_MFRC_P_PPE     (6 << 4)

//GDMA1 other MAC address frames destination port
#define GDM1_OFRC_P_CPU     (0 << 0)
#define GDM1_OFRC_P_GDMA1   (1 << 0)
#define GDM1_OFRC_P_PPE     (6 << 0)

#define ICS_GEN_EN          (1 << 2)
#define UCS_GEN_EN          (1 << 1)
#define TCS_GEN_EN          (1 << 0)

// MDIO_CFG	bit
#define MDIO_CFG_GP1_FC_TX	(1 << 11)
#define MDIO_CFG_GP1_FC_RX	(1 << 10)

/* ====================================== */
/* ====================================== */
#define GP1_LNK_DWN     BIT(9) 
#define GP1_AN_FAIL     BIT(8) 
/* ====================================== */
/* ====================================== */
#define PSE_RESET       BIT(0)
/* ====================================== */
#define PST_DRX_IDX1       BIT(17)
#define PST_DRX_IDX0       BIT(16)
#define PST_DTX_IDX3       BIT(3)
#define PST_DTX_IDX2       BIT(2)
#define PST_DTX_IDX1       BIT(1)
#define PST_DTX_IDX0       BIT(0)

#define TX_WB_DDONE       BIT(6)
#define RX_DMA_BUSY       BIT(3)
#define TX_DMA_BUSY       BIT(1)
#define RX_DMA_EN         BIT(2)
#define TX_DMA_EN         BIT(0)

#define PDMA_BT_SIZE_4DWORDS     (0<<4)
#define PDMA_BT_SIZE_8DWORDS     (1<<4)
#define PDMA_BT_SIZE_16DWORDS    (2<<4)
#define PDMA_BT_SIZE_32DWORDS    (3<<4)

/* Register bits.
 */

#define MACCFG_RXEN		(1<<2)
#define MACCFG_TXEN		(1<<3)
#define MACCFG_PROMISC		(1<<18)
#define MACCFG_RXMCAST		(1<<19)
#define MACCFG_FDUPLEX		(1<<20)
#define MACCFG_PORTSEL		(1<<27)
#define MACCFG_HBEATDIS		(1<<28)


#define DMACTL_SR		(1<<1)	/* Start/Stop Receive */
#define DMACTL_ST		(1<<13)	/* Start/Stop Transmission Command */

#define DMACFG_SWR		(1<<0)	/* Software Reset */
#define DMACFG_BURST32		(32<<8)

#define DMASTAT_TS		0x00700000	/* Transmit Process State */
#define DMASTAT_RS		0x000e0000	/* Receive Process State */

#define MACCFG_INIT		0 //(MACCFG_FDUPLEX) // | MACCFG_PORTSEL)



/* Descriptor bits.
 */
#define R_OWN		0x80000000	/* Own Bit */
#define RD_RER		0x02000000	/* Receive End Of Ring */
#define RD_LS		0x00000100	/* Last Descriptor */
#define RD_ES		0x00008000	/* Error Summary */
#define RD_CHAIN	0x01000000	/* Chained */

/* Word 0 */
#define T_OWN		0x80000000	/* Own Bit */
#define TD_ES		0x00008000	/* Error Summary */

/* Word 1 */
#define TD_LS		0x40000000	/* Last Segment */
#define TD_FS		0x20000000	/* First Segment */
#define TD_TER		0x08000000	/* Transmit End Of Ring */
#define TD_CHAIN	0x01000000	/* Chained */


#define TD_SET		0x08000000	/* Setup Packet */


#define POLL_DEMAND 1

#define RSTCTL	(0x34)
#define RSTCTL_RSTENET1	(1<<19)
#define RSTCTL_RSTENET2	(1<<20)

#define INIT_VALUE_OF_RT2883_PSE_FQ_CFG		0xff908000
#define INIT_VALUE_OF_PSE_FQFC_CFG		0x80504000
#define INIT_VALUE_OF_FORCE_100_FD		0x1001BC01
#define INIT_VALUE_OF_FORCE_1000_FD		0x1F01DC01

// Define Whole FE Reset Register
#define RSTCTRL         (RALINK_SYSCTL_BASE + 0x34)

/*=========================================
      PDMA RX Descriptor Format define
=========================================*/

//-------------------------------------------------
typedef struct _PDMA_RXD_INFO1_  PDMA_RXD_INFO1_T;

struct _PDMA_RXD_INFO1_
{
    unsigned int    PDP0;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO2_    PDMA_RXD_INFO2_T;

struct _PDMA_RXD_INFO2_
{
    unsigned int    PLEN1                 : 14;
    unsigned int    LS1                   : 1;
    unsigned int    UN_USED               : 1;
    unsigned int    PLEN0                 : 14;
    unsigned int    LS0                   : 1;
    unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO3_  PDMA_RXD_INFO3_T;

struct _PDMA_RXD_INFO3_
{
    unsigned int    UN_USE1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO4_    PDMA_RXD_INFO4_T;

struct _PDMA_RXD_INFO4_
{

    unsigned int    FOE_Entry           : 14;
    unsigned int    FVLD                : 1;
    unsigned int    UN_USE1             : 1;
    unsigned int    AI                  : 8;
    unsigned int    SP                  : 3;
    unsigned int    AIS                 : 1;
    unsigned int    L4F                 : 1;
    unsigned int    IPF                  : 1;
    unsigned int    L4FVLD_bit           : 1;
    unsigned int    IPFVLD_bit           : 1;
};


struct PDMA_rxdesc {
	PDMA_RXD_INFO1_T rxd_info1;
	PDMA_RXD_INFO2_T rxd_info2;
	PDMA_RXD_INFO3_T rxd_info3;
	PDMA_RXD_INFO4_T rxd_info4;
};

/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO1_  PDMA_TXD_INFO1_T;

struct _PDMA_TXD_INFO1_
{
    unsigned int    SDP0;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO2_    PDMA_TXD_INFO2_T;

struct _PDMA_TXD_INFO2_
{
    unsigned int    SDL1                  : 14;
    unsigned int    LS1_bit               : 1;
    unsigned int    BURST_bit             : 1;
    unsigned int    SDL0                  : 14;
    unsigned int    LS0_bit               : 1;
    unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO3_  PDMA_TXD_INFO3_T;

struct _PDMA_TXD_INFO3_
{
    unsigned int    SDP1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO4_    PDMA_TXD_INFO4_T;

struct _PDMA_TXD_INFO4_
{

    unsigned int    VIDX                : 4;
    unsigned int    VPRI                : 3;
    unsigned int    INSV                : 1;
    unsigned int    SIDX                : 4;
    unsigned int    INSP                : 1;
    unsigned int    RESV            	: 1;
    unsigned int    UN_USE3             : 2;
    unsigned int    QN                  : 3;
    unsigned int    UN_USE2             : 1;
    unsigned int    UDF			: 4;
    unsigned int    PN                  : 3;
#ifdef CONFIG_RAETH_TSO
    unsigned int    UN_USE1             : 1;
    unsigned int    TSO			: 1;
#else
    unsigned int    UN_USE1             : 2;
#endif
    unsigned int    TCO                 : 1;
    unsigned int    UCO			: 1;
    unsigned int    ICO		        : 1;
};


struct PDMA_txdesc {
	PDMA_TXD_INFO1_T txd_info1;
	PDMA_TXD_INFO2_T txd_info2;
	PDMA_TXD_INFO3_T txd_info3;
	PDMA_TXD_INFO4_T txd_info4;
};

#define phys_to_bus(a) (a & 0x1FFFFFFF)

#define PHY_Enable_Auto_Nego		0x1000
#define PHY_Restart_Auto_Nego		0x0200

/* PHY_STAT_REG = 1; */
#define PHY_Auto_Neco_Comp	0x0020
#define PHY_Link_Status		0x0004

/* PHY_AUTO_NEGO_REG = 4; */
#define PHY_Cap_10_Half  0x0020
#define PHY_Cap_10_Full  0x0040
#define	PHY_Cap_100_Half 0x0080
#define	PHY_Cap_100_Full 0x0100

/* proc definition */

#if !defined (CONFIG_RALINK_RT6855) && !defined(CONFIG_RALINK_RT63365)
#define CDMA_OQ_STA	(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x4c)
#define GDMA1_OQ_STA	(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x50)
#define PPE_OQ_STA	(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x54)
#define PSE_IQ_STA	(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x58)
#endif

#define PROCREG_CONTROL_FILE      "/var/run/procreg_control"
#if defined (CONFIG_RALINK_RT2880)
#define PROCREG_DIR             "rt2880"
#elif defined (CONFIG_RALINK_RT3052)
#define PROCREG_DIR             "rt3052"
#elif defined (CONFIG_RALINK_RT3352)
#define PROCREG_DIR             "rt3352"
#elif defined (CONFIG_RALINK_RT5350)
#define PROCREG_DIR             "rt5350"
#elif defined (CONFIG_RALINK_RT2883)
#define PROCREG_DIR             "rt2883"
#elif defined (CONFIG_RALINK_RT3883)
#define PROCREG_DIR             "rt3883"
#elif defined (CONFIG_RALINK_RT6855)
#define PROCREG_DIR             "rt6855"
#elif defined (CONFIG_RALINK_RT63365)
#define PROCREG_DIR             "rt63365"
#else
#define PROCREG_DIR             "rt2880"
#endif
#define PROCREG_GMAC		"gmac"
#define PROCREG_CP0		"cp0"
#define PROCREG_RAQOS		"qos"
#define PROCREG_READ_VAL	"regread_value"
#define PROCREG_WRITE_VAL	"regwrite_value"
#define PROCREG_ADDR	  	"reg_addr"
#define PROCREG_CTL		"procreg_control"
#define PROCREG_RXDONE_INTR	"rxdone_intr_count"
#define PROCREG_ESW_INTR	"esw_intr_count"
#define PROCREG_ESW_CNT		"esw_cnt"
#define PROCREG_SNMP		"snmp"

struct rt2880_reg_op_data {
  char	name[64];
  unsigned int reg_addr;
  unsigned int op;
  unsigned int reg_value;
};        

typedef struct end_device
{

    unsigned int        tx_cpu_owner_idx0;
    unsigned int        rx_cpu_owner_idx0;
    unsigned int        fe_int_status;
#if defined (CONFIG_RAETH_QOS)
    unsigned int        tx0_full, tx1_full, tx2_full, tx3_full, tx_full;
    unsigned int	phy_tx_ring0, phy_tx_ring1, phy_tx_ring2, phy_tx_ring3;
#else
    unsigned int        tx_full;
    unsigned int	phy_tx_ring0;
#endif

    unsigned int	phy_rx_ring0, phy_rx_ring1;

#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT63365)
    //send signal to user application to notify link status changed
    struct work_struct  kill_sig_wq;
#endif

    struct work_struct  reset_task;
#ifdef WORKQUEUE_BH
    struct work_struct  rx_wq;
    struct work_struct  tx_wq;
#else
    struct              tasklet_struct     rx_tasklet;
    struct              tasklet_struct     tx_tasklet;
#endif // WORKQUEUE_BH //

#if defined(CONFIG_RAETH_QOS)
    struct		sk_buff *	   skb_free[NUM_TX_RINGS][NUM_TX_DESC];
    unsigned int	free_idx[NUM_TX_RINGS];
#else
    struct		sk_buff*	   skb_free[NUM_TX_DESC];
    unsigned int	free_idx;
#endif

    struct              net_device_stats stat;  /* The new statistics table. */
    spinlock_t          page_lock;              /* Page register locks */
    struct PDMA_txdesc *tx_ring0;
#if defined(CONFIG_RAETH_QOS)
    struct PDMA_txdesc *tx_ring1;
    struct PDMA_txdesc *tx_ring2;
    struct PDMA_txdesc *tx_ring3;
#endif
    struct PDMA_rxdesc *rx_ring0;
    struct sk_buff     *netrx0_skbuf[NUM_RX_DESC];
#if defined (CONFIG_RAETH_MULTIPLE_RX_RING)
    struct PDMA_rxdesc *rx_ring1;
    struct sk_buff     *netrx1_skbuf[NUM_RX_DESC];
#endif

#ifdef CONFIG_RAETH_SKB_RECYCLE
    struct sk_buff_head rx0_recycle;
#endif

#ifdef CONFIG_RAETH_NAPI
    atomic_t irq_sem;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    struct napi_struct napi;
#endif
#endif
#ifdef CONFIG_PSEUDO_SUPPORT
    struct net_device *PseudoDev;
    unsigned int isPseudo;
#endif
#if defined (CONFIG_ETHTOOL)
	struct mii_if_info	mii_info;
#endif
} END_DEVICE, *pEND_DEVICE;

#define RAETH_VERSION	"v2.1"

#endif
