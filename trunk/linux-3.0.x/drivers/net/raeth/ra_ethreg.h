#ifndef __RA_ETHREG_H__
#define __RA_ETHREG_H__

#include <linux/mii.h>		// for struct mii_if_info in ra_ethreg.h
#include <linux/version.h>	/* check linux version for 2.4 and 2.6 compatibility */
#include <linux/interrupt.h>

#include <asm/rt2880/rt_mmap.h>

#include "raether.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#define BIT(x)				((1 << x))
#endif

#define ETHER_ADDR_LEN			6

#define PHYS_TO_K1(physaddr)		KSEG1ADDR(physaddr)
#define phys_to_bus(a)			(a & 0x1FFFFFFF)

#define sysRegRead(phys)		(*(volatile unsigned int *)PHYS_TO_K1(phys))
#define sysRegWrite(phys, val)		((*(volatile unsigned int *)PHYS_TO_K1(phys)) = (val))

#define u_long				unsigned long
#define u32				unsigned int
#define u16				unsigned short

/*  Phy Vender ID list */

#define EV_ICPLUS_PHY_ID0		0x0243
#define EV_ICPLUS_PHY_ID1		0x0D90
#define EV_MARVELL_PHY_ID0		0x0141
#define EV_MARVELL_PHY_ID1		0x0CC2
#define EV_VTSS_PHY_ID0			0x0007
#define EV_VTSS_PHY_ID1			0x0421

/*
     FE_INT_STATUS
*/
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
    defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)

#define RX_COHERENT			BIT(31)
#define RX_DLY_INT			BIT(30)
#define TX_COHERENT			BIT(29)
#define TX_DLY_INT			BIT(28)

#define RX_DONE_INT1			BIT(17)
#define RX_DONE_INT0			BIT(16)

#define TX_DONE_INT3			BIT(3)
#define TX_DONE_INT2			BIT(2)
#define TX_DONE_INT1			BIT(1)
#define TX_DONE_INT0			BIT(0)
#else
//#define CNT_PPE_AF			BIT(31)
//#define CNT_GDM_AF			BIT(29)
#define PSE_P2_FC			BIT(26)
#define GDM_CRC_DROP			BIT(25)
#define PSE_BUF_DROP			BIT(24)
#define GDM_OTHER_DROP			BIT(23)
#define PSE_P1_FC			BIT(22)
#define PSE_P0_FC			BIT(21)
#define PSE_FQ_EMPTY			BIT(20)
#define GE1_STA_CHG			BIT(18)
#define TX_COHERENT			BIT(17)
#define RX_COHERENT			BIT(16)

#define TX_DONE_INT3			BIT(11)
#define TX_DONE_INT2			BIT(10)
#define TX_DONE_INT1			BIT(9)
#define TX_DONE_INT0			BIT(8)
#define RX_DONE_INT1			RX_DONE_INT0
#define RX_DONE_INT0			BIT(2)
#define TX_DLY_INT			BIT(1)
#define RX_DLY_INT			BIT(0)
#endif

/*
 * SW_INT_STATUS
 */
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
#define PORT0_QUEUE_FULL		BIT(14) //port0 queue full
#define PORT1_QUEUE_FULL		BIT(15) //port1 queue full
#define PORT2_QUEUE_FULL		BIT(16) //port2 queue full
#define PORT3_QUEUE_FULL		BIT(17) //port3 queue full
#define PORT4_QUEUE_FULL		BIT(18) //port4 queue full
#define PORT5_QUEUE_FULL		BIT(19) //port5 queue full
#define PORT6_QUEUE_FULL		BIT(20) //port6 queue full
#define SHARED_QUEUE_FULL		BIT(23) //shared queue full
#define QUEUE_EXHAUSTED			BIT(24) //global queue is used up and all packets are dropped
#define BC_STROM			BIT(25) //the device is undergoing broadcast storm
#define PORT_ST_CHG			BIT(26) //Port status change
#define UNSECURED_ALERT			BIT(27) //Intruder alert
#define ABNORMAL_ALERT			BIT(28) //Abnormal

#define ESW_ISR				(RALINK_ETH_SW_BASE + 0x00)
#define ESW_IMR				(RALINK_ETH_SW_BASE + 0x04)
#define ESW_INT_ALL			(PORT_ST_CHG)

#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
      defined (CONFIG_RALINK_MT7620)
#define ACL_INT				BIT(15)
#define P5_LINK_CH			BIT(5)
#define P4_LINK_CH			BIT(4)
#define P3_LINK_CH			BIT(3)
#define P2_LINK_CH			BIT(2)
#define P1_LINK_CH			BIT(1)
#define P0_LINK_CH			BIT(0)

#define ESW_IMR				(RALINK_ETH_SW_BASE + 0x7000 + 0x8)
#define ESW_ISR				(RALINK_ETH_SW_BASE + 0x7000 + 0xC)
#define ESW_INT_ALL			(P0_LINK_CH | P1_LINK_CH | P2_LINK_CH | P3_LINK_CH | P4_LINK_CH | P5_LINK_CH | ACL_INT)
#define ESW_AISR			(RALINK_ETH_SW_BASE + 0x8)

#define ESW_PHY_POLLING			(RALINK_ETH_SW_BASE + 0x7000)

#elif defined (CONFIG_RALINK_MT7621)

#define ESW_PHY_POLLING			(RALINK_ETH_SW_BASE + 0x0000)

#endif

/* Register Categories Definition */
#define RAFRAMEENGINE_OFFSET		0x0000
#define RAGDMA_OFFSET			0x0020
#define RAPSE_OFFSET			0x0040
#define RAGDMA2_OFFSET			0x0060
#define RACDMA_OFFSET			0x0080
#if defined (CONFIG_RALINK_RT5350)
#define RASDM_OFFSET			0x0C00
#endif
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || \
    defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
#define RAPDMA_OFFSET			0x0800
#else
#define RAPDMA_OFFSET			0x0100
#endif
#if defined (CONFIG_RALINK_MT7621)
#define RACMTABLE_OFFSET		0x2000
#elif defined (CONFIG_RALINK_MT7620)
#define RACMTABLE_OFFSET		0x1000
#else
#define RACMTABLE_OFFSET		0x0400
#endif

/* Register Map Detail */
/* RT3883 */
#define SYSCFG1				(RALINK_SYSCTL_BASE + 0x14)

#if defined (CONFIG_RALINK_RT5350)

/* 1. PDMA */
#define TX_BASE_PTR0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x000)
#define TX_MAX_CNT0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x004)
#define TX_CTX_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x008)
#define TX_DTX_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x00C)

#define TX_BASE_PTR1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x010)
#define TX_MAX_CNT1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x014)
#define TX_CTX_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x018)
#define TX_DTX_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x01C)

#define TX_BASE_PTR2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x020)
#define TX_MAX_CNT2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x024)
#define TX_CTX_IDX2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x028)
#define TX_DTX_IDX2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x02C)

#define TX_BASE_PTR3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x030)
#define TX_MAX_CNT3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x034)
#define TX_CTX_IDX3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x038)
#define TX_DTX_IDX3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x03C)

#define RX_BASE_PTR0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x100)
#define RX_MAX_CNT0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x104)
#define RX_CALC_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x108)
#define RX_DRX_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x10C)

#define RX_BASE_PTR1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x110)
#define RX_MAX_CNT1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x114)
#define RX_CALC_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x118)
#define RX_DRX_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x11C)

#define PDMA_INFO			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x200)
#define PDMA_GLO_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x204)
#define PDMA_RST_IDX			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x208)
#define PDMA_RST_CFG			(PDMA_RST_IDX)
#define DLY_INT_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x20C)
#define FREEQ_THRES			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x210)
#define INT_STATUS			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x220)
#define FE_INT_STATUS			(INT_STATUS)
#define INT_MASK			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x228)
#define FE_INT_ENABLE			(INT_MASK)
#define PDMA_WRR			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x280)
#define PDMA_SCH_CFG			(PDMA_WRR)

#define SDM_CON				(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x00)  //Switch DMA configuration
#define SDM_RRING			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x04)  //Switch DMA Rx Ring
#define SDM_TRING			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x08)  //Switch DMA Tx Ring
#define SDM_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x0C)  //Switch MAC address LSB
#define SDM_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x10)  //Switch MAC Address MSB
#define SDM_TPCNT			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x100) //Switch DMA Tx packet count
#define SDM_TBCNT			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x104) //Switch DMA Tx byte count
#define SDM_RPCNT			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x108) //Switch DMA rx packet count
#define SDM_RBCNT			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x10C) //Switch DMA rx byte count
#define SDM_CS_ERR			(RALINK_FRAME_ENGINE_BASE+RASDM_OFFSET+0x110) //Switch DMA rx checksum error count

#elif defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
      defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)

/* Old FE with New PDMA */
#define PDMA_RELATED			0x0800
/* 1. PDMA */
#define TX_BASE_PTR0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x000)
#define TX_MAX_CNT0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x004)
#define TX_CTX_IDX0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x008)
#define TX_DTX_IDX0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x00C)

#define TX_BASE_PTR1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x010)
#define TX_MAX_CNT1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x014)
#define TX_CTX_IDX1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x018)
#define TX_DTX_IDX1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x01C)

#define TX_BASE_PTR2			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x020)
#define TX_MAX_CNT2			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x024)
#define TX_CTX_IDX2			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x028)
#define TX_DTX_IDX2			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x02C)

#define TX_BASE_PTR3			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x030)
#define TX_MAX_CNT3			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x034)
#define TX_CTX_IDX3			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x038)
#define TX_DTX_IDX3			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x03C)

#define RX_BASE_PTR0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x100)
#define RX_MAX_CNT0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x104)
#define RX_CALC_IDX0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x108)
#define RX_DRX_IDX0			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x10C)

#define RX_BASE_PTR1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x110)
#define RX_MAX_CNT1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x114)
#define RX_CALC_IDX1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x118)
#define RX_DRX_IDX1			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x11C)

#define PDMA_INFO			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x200)
#define PDMA_GLO_CFG			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x204)
#define PDMA_RST_IDX			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x208)
#define PDMA_RST_CFG			(PDMA_RST_IDX)
#define DLY_INT_CFG			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x20C)
#define FREEQ_THRES			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x210)
#define INT_STATUS			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x220)
#define FE_INT_STATUS			(INT_STATUS)
#define INT_MASK			(RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x228)
#define FE_INT_ENABLE			(INT_MASK)
#define SCH_Q01_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x280)
#define SCH_Q23_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x284)

#define FE_GLO_CFG			(RALINK_FRAME_ENGINE_BASE + 0x08)
#define FE_RST_GLO			(RALINK_FRAME_ENGINE_BASE + 0x0C)
#define FE_INT_STATUS2			(RALINK_FRAME_ENGINE_BASE + 0x10)
#define FE_INT_ENABLE2			(RALINK_FRAME_ENGINE_BASE + 0x14)
#define FC_DROP_STA			(RALINK_FRAME_ENGINE_BASE + 0x18)
#define FOE_TS_T			(RALINK_FRAME_ENGINE_BASE + 0x1C)

#if defined (CONFIG_RALINK_MT7620)
#define GDMA1_RELATED			0x0600
#define GDMA1_FWD_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x00)
#define GDMA1_SHPR_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x04)
#define GDMA1_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x08)
#define GDMA1_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x0C)
#elif defined (CONFIG_RALINK_MT7621)
#define GDMA1_RELATED			0x0500
#define GDMA1_FWD_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x00)
#define GDMA1_SHPR_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x04)
#define GDMA1_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x08)
#define GDMA1_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x0C)

#define GDMA2_RELATED			0x1500
#define GDMA2_FWD_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x00)
#define GDMA2_SHPR_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x04)
#define GDMA2_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x08)
#define GDMA2_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x0C)
#else
#define GDMA1_RELATED			0x0020
#define GDMA1_FWD_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x00)
#define GDMA1_SCH_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x04)
#define GDMA1_SHPR_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x08)
#define GDMA1_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x0C)
#define GDMA1_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x10)

#define GDMA2_RELATED			0x0060
#define GDMA2_FWD_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x00)
#define GDMA2_SCH_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x04)
#define GDMA2_SHPR_CFG			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x08)
#define GDMA2_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x0C)
#define GDMA2_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x10)
#endif

#if defined (CONFIG_RALINK_MT7620)
#define PSE_RELATED			0x0500
#define PSE_FQFC_CFG			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x00)
#define PSE_IQ_CFG			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x04)
#define PSE_QUE_STA			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x08)
#else
#define PSE_RELATED			0x0040
#define PSE_FQ_CFG			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x00)
#define CDMA_FC_CFG			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x04)
#define GDMA1_FC_CFG			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x08)
#define GDMA2_FC_CFG			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x0C)
#define CDMA_OQ_STA			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x10)
#define GDMA1_OQ_STA			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x14)
#define GDMA2_OQ_STA			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x18)
#define PSE_IQ_STA			(RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x1C)
#endif

#if defined (CONFIG_RALINK_MT7620)
#define CDMA_RELATED			0x0400
#define CDMA_CSG_CFG			(RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00)
#define SMACCR0				(RALINK_ETH_SW_BASE + 0x3FE4)
#define SMACCR1				(RALINK_ETH_SW_BASE + 0x3FE8)
#define CKGCR				(RALINK_ETH_SW_BASE + 0x3FF0)
#elif defined (CONFIG_RALINK_MT7621)
#define CDMA_RELATED			0x0400
#define CDMA_CSG_CFG			(RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00) //fake definition
#define CDMP_IG_CTRL			(RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00)
#define CDMP_EG_CTRL			(RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x04)
#else
#define CDMA_RELATED			0x0080
#define CDMA_CSG_CFG			(RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00)
#define CDMA_SCH_CFG			(RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x04)
#define SMACCR0				(RALINK_ETH_SW_BASE + 0x30E4)
#define SMACCR1				(RALINK_ETH_SW_BASE + 0x30E8)
#define CKGCR				(RALINK_ETH_SW_BASE + 0x30F0)
#endif

#define PDMA_FC_CFG			(RALINK_FRAME_ENGINE_BASE+0x100)

#else

/* 1. Frame Engine Global Registers */
#define MDIO_ACCESS			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x00)
#define MDIO_CFG 			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x04)
#define FE_GLO_CFG			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x08)
#define FE_RST_GLO			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x0C)
#define FE_INT_STATUS			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x10)
#define FE_INT_ENABLE			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x14)
#define MDIO_CFG2			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x18) //Original:FC_DROP_STA
#define FOC_TS_T			(RALINK_FRAME_ENGINE_BASE+RAFRAMEENGINE_OFFSET+0x1C)


/* 2. GDMA Registers */
#define	GDMA1_FWD_CFG			(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x00)
#define GDMA1_SCH_CFG			(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x04)
#define GDMA1_SHPR_CFG			(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x08)
#define GDMA1_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x0C)
#define GDMA1_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE+RAGDMA_OFFSET+0x10)

#define	GDMA2_FWD_CFG			(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x00)
#define GDMA2_SCH_CFG			(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x04)
#define GDMA2_SHPR_CFG			(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x08)
#define GDMA2_MAC_ADRL			(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x0C)
#define GDMA2_MAC_ADRH			(RALINK_FRAME_ENGINE_BASE+RAGDMA2_OFFSET+0x10)

/* 3. PSE */
#define PSE_FQ_CFG			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x00)
#define CDMA_FC_CFG			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x04)
#define GDMA1_FC_CFG			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x08)
#define GDMA2_FC_CFG			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x0C)
#define PDMA_FC_CFG			(RALINK_FRAME_ENGINE_BASE+0x1f0)

/* 4. CDMA */
#define CDMA_CSG_CFG			(RALINK_FRAME_ENGINE_BASE+RACDMA_OFFSET+0x00)
#define CDMA_SCH_CFG			(RALINK_FRAME_ENGINE_BASE+RACDMA_OFFSET+0x04)
/* skip ppoe sid and vlan id definition */

/* 5. PDMA */
#define PDMA_GLO_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x00)
#define PDMA_RST_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x04)
#define PDMA_SCH_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x08)

#define DLY_INT_CFG			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x0C)

#define TX_BASE_PTR0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x10)
#define TX_MAX_CNT0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x14)
#define TX_CTX_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x18)
#define TX_DTX_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x1C)

#define TX_BASE_PTR1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x20)
#define TX_MAX_CNT1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x24)
#define TX_CTX_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x28)
#define TX_DTX_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x2C)

#define TX_BASE_PTR2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x40)
#define TX_MAX_CNT2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x44)
#define TX_CTX_IDX2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x48)
#define TX_DTX_IDX2			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x4C)

#define TX_BASE_PTR3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x50)
#define TX_MAX_CNT3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x54)
#define TX_CTX_IDX3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x58)
#define TX_DTX_IDX3			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x5C)

#define RX_BASE_PTR0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x30)
#define RX_MAX_CNT0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x34)
#define RX_CALC_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x38)
#define RX_DRX_IDX0			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x3C)

#define RX_BASE_PTR1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x40)
#define RX_MAX_CNT1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x44)
#define RX_CALC_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x48)
#define RX_DRX_IDX1			(RALINK_FRAME_ENGINE_BASE+RAPDMA_OFFSET+0x4C)

#endif

#if !defined (CONFIG_RALINK_RT5350)

/* 6. Counter and Meter Table */
#define PPE_AC_BCNT0			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x000) /* PPE Accounting Group 0 Byte Cnt */
#define PPE_AC_PCNT0			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x004) /* PPE Accounting Group 0 Packet Cnt */
/* 0 ~ 63 */

#define PPE_MTR_CNT0			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x200) /* 0 ~ 63 */
/* skip... */
#define PPE_MTR_CNT63			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x2FC)

#define GDMA_TX_GBCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x300) /* Transmit good byte cnt for GE port1 */
#define GDMA_TX_GPCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x304) /* Transmit good pkt cnt for GE port1 */
#define GDMA_TX_SKIPCNT1		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x308) /* Transmit skip cnt for GE port1 */
#define GDMA_TX_COLCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x30C) /* Transmit collision cnt for GE port1 */
#define GDMA_RX_GBCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x320)
#define GDMA_RX_GPCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x324)
#define GDMA_RX_OERCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x328)
#define GDMA_RX_FERCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x32C)
#define GDMA_RX_SERCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x330)
#define GDMA_RX_LERCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x334)
#define GDMA_RX_CERCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x338)
#define GDMA_RX_FCCNT1			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x33C)

#define GDMA_TX_GBCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x340) /* Transmit good byte cnt for GE port2 */
#define GDMA_TX_GPCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x344) /* Transmit good pkt cnt for GE port2 */
#define GDMA_TX_SKIPCNT2		(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x348) /* Transmit skip cnt for GE port2 */
#define GDMA_TX_COLCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x34C) /* Transmit collision cnt for GE port2 */
#define GDMA_RX_GBCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x360)
#define GDMA_RX_GPCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x364)
#define GDMA_RX_OERCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x368)
#define GDMA_RX_FERCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x36C)
#define GDMA_RX_SERCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x370)
#define GDMA_RX_LERCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x374)
#define GDMA_RX_CERCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x378)
#define GDMA_RX_FCCNT2			(RALINK_FRAME_ENGINE_BASE+RACMTABLE_OFFSET+0x37C)

#endif


/* Per Port Packet Counts in RT3052, added by bobtseng 2009.4.17. */
#define PORT0_PKCOUNT			(0xb01100e8)
#define PORT1_PKCOUNT			(0xb01100ec)
#define PORT2_PKCOUNT			(0xb01100f0)
#define PORT3_PKCOUNT			(0xb01100f4)
#define PORT4_PKCOUNT			(0xb01100f8)
#define PORT5_PKCOUNT			(0xb01100fc)

/* ====================================== */

#define GDM1_TCI_81XX			BIT(24)
#define GDM1_ICS_EN			BIT(22)
#define GDM1_TCS_EN			BIT(21)
#define GDM1_UCS_EN			BIT(20)
#define GDM1_JMB_EN			BIT(19)
#define GDM1_DISPAD			BIT(18)
#define GDM1_DISCRC			BIT(17)
#define GDM1_STRPCRC			BIT(16)

//GDMA1 uni-cast frames destination port
#define GDM1_UFRC_P_CPU			(0 << 12)
#if defined (CONFIG_RALINK_MT7621)
#define GDM1_UFRC_P_PPE			(4 << 12)
#else
#define GDM1_UFRC_P_PPE			(6 << 12)
#endif

//GDMA1 broad-cast MAC address frames
#define GDM1_BFRC_P_CPU			(0 << 8)
#if defined (CONFIG_RALINK_MT7621)
#define GDM1_BFRC_P_PPE			(4 << 8)
#else
#define GDM1_BFRC_P_PPE			(6 << 8)
#endif

//GDMA1 multi-cast MAC address frames
#define GDM1_MFRC_P_CPU			(0 << 4)
#if defined (CONFIG_RALINK_MT7621)
#define GDM1_MFRC_P_PPE			(4 << 4)
#else
#define GDM1_MFRC_P_PPE			(6 << 4)
#endif

//GDMA1 other MAC address frames destination port
#define GDM1_OFRC_P_CPU			(0 << 0)
#if defined (CONFIG_RALINK_MT7621)
#define GDM1_OFRC_P_PPE			(4 << 0)
#else
#define GDM1_OFRC_P_PPE			(6 << 0)
#endif

#if defined (CONFIG_RALINK_MT7621)
/* checksum generator registers are removed */
#define ICS_GEN_EN			(0 << 2)
#define UCS_GEN_EN			(0 << 1)
#define TCS_GEN_EN			(0 << 0)
#else
#define ICS_GEN_EN			(1 << 2)
#define UCS_GEN_EN			(1 << 1)
#define TCS_GEN_EN			(1 << 0)
#endif

// MDIO_CFG	bit
#define MDIO_CFG_GP1_FC_TX		(1 << 11)
#define MDIO_CFG_GP1_FC_RX		(1 << 10)

/* ====================================== */
/* ====================================== */
#define GP1_LNK_DWN			BIT(9)
#define GP1_AN_FAIL			BIT(8)
/* ====================================== */
/* ====================================== */
#define PSE_RESET			BIT(0)
/* ====================================== */
#define PST_DRX_IDX1			BIT(17)
#define PST_DRX_IDX0			BIT(16)
#define PST_DTX_IDX3			BIT(3)
#define PST_DTX_IDX2			BIT(2)
#define PST_DTX_IDX1			BIT(1)
#define PST_DTX_IDX0			BIT(0)

#define RX_2B_OFFSET			BIT(31)
#define DESC_32B_EN			BIT(8)
#define TX_WB_DDONE			BIT(6)
#define RX_DMA_BUSY			BIT(3)
#define TX_DMA_BUSY			BIT(1)
#define RX_DMA_EN			BIT(2)
#define TX_DMA_EN			BIT(0)

#define PDMA_BT_SIZE_4DWORDS		(0<<4)
#define PDMA_BT_SIZE_8DWORDS		(1<<4)
#define PDMA_BT_SIZE_16DWORDS		(2<<4)
#define PDMA_BT_SIZE_32DWORDS		(3<<4)

/* Register bits.
 */

#define MACCFG_RXEN			(1<<2)
#define MACCFG_TXEN			(1<<3)
#define MACCFG_PROMISC			(1<<18)
#define MACCFG_RXMCAST			(1<<19)
#define MACCFG_FDUPLEX			(1<<20)
#define MACCFG_PORTSEL			(1<<27)
#define MACCFG_HBEATDIS			(1<<28)


#define DMACTL_SR			(1<<1)	/* Start/Stop Receive */
#define DMACTL_ST			(1<<13)	/* Start/Stop Transmission Command */

#define DMACFG_SWR			(1<<0)	/* Software Reset */
#define DMACFG_BURST32			(32<<8)

#define DMASTAT_TS			0x00700000	/* Transmit Process State */
#define DMASTAT_RS			0x000e0000	/* Receive Process State */

#define MACCFG_INIT			0 //(MACCFG_FDUPLEX) // | MACCFG_PORTSEL)



/* Descriptor bits.
 */
#define R_OWN				0x80000000	/* Own Bit */
#define RD_RER				0x02000000	/* Receive End Of Ring */
#define RD_LS				0x00000100	/* Last Descriptor */
#define RD_ES				0x00008000	/* Error Summary */
#define RD_CHAIN			0x01000000	/* Chained */

/* Word 0 */
#define T_OWN				0x80000000	/* Own Bit */
#define TD_ES				0x00008000	/* Error Summary */

/* Word 1 */
#define TD_LS				0x40000000	/* Last Segment */
#define TD_FS				0x20000000	/* First Segment */
#define TD_TER				0x08000000	/* Transmit End Of Ring */
#define TD_CHAIN			0x01000000	/* Chained */


#define TD_SET				0x08000000	/* Setup Packet */


#define RSTCTL				(0x34)
#define RSTCTL_RSTENET1			(1<<19)
#define RSTCTL_RSTENET2			(1<<20)

#define INIT_VALUE_OF_RT2883_PSE_FQ_CFG	0xff908000
#define INIT_VALUE_OF_PSE_FQFC_CFG	0x80504000
#define INIT_VALUE_OF_FORCE_100_FD	0x1001BC01
#if defined (CONFIG_RTL8367) || defined (CONFIG_RTL8367_MODULE)
#define INIT_VALUE_OF_FORCE_1000_FD	0x0001DC01
#else
#define INIT_VALUE_OF_FORCE_1000_FD	0x1F01DC01
#endif

// Define Whole FE Reset Register
#define RSTCTRL				(RALINK_SYSCTL_BASE + 0x34)

/*=========================================
      PDMA RX Descriptor Format define
=========================================*/

struct PDMA_rxdesc {
	unsigned int rxd_info1_u32;
	unsigned int rxd_info2_u32;
	unsigned int rxd_info3_u32;
	unsigned int rxd_info4_u32;
#if defined (CONFIG_RAETH_32B_DESC)
	unsigned int rxd_info5_u32;
	unsigned int rxd_info6_u32;
	unsigned int rxd_info7_u32;
	unsigned int rxd_info8_u32;
#endif
};

#define RX2_DMA_SDL0_GET(_x)		(((_x) >> 16) & 0x3fff)
#define RX2_DMA_SDL0_SET(_x)		(((_x) & 0x3fff) << 16)
#define RX2_DMA_LS1			BIT(14)
#define RX2_DMA_TAG			BIT(15)
#define RX2_DMA_LS0			BIT(30)
#define RX2_DMA_DONE			BIT(31)

#define RX3_DMA_VID(_x)			((_x) & 0xffff)
#define RX3_DMA_TPID(_x)		(((_x) >> 16) & 0xffff)

#if defined (CONFIG_RALINK_MT7621)
#define RX4_DMA_SP(_x)			(((_x) >> 19) & 0xf)
#define RX4_DMA_ALG_SET			(0xFF800000)
#define RX4_DMA_L4F			BIT(23)
#define RX4_DMA_L4FVLD			BIT(24)
#define RX4_DMA_TACK			BIT(25)
#define RX4_DMA_IP4F			BIT(26)
#define RX4_DMA_IP4			BIT(27)
#define RX4_DMA_IP6			BIT(28)
#elif defined (CONFIG_RALINK_MT7620)
#define RX4_DMA_SP(_x)			(((_x) >> 19) & 0x7)
#define RX4_DMA_ALG_SET			(0xFFC00000)
#define RX4_DMA_L4F			BIT(22)
#define RX4_DMA_L4FVLD			BIT(23)
#define RX4_DMA_TACK			BIT(24)
#define RX4_DMA_IP4F			BIT(25)
#define RX4_DMA_IP4			BIT(26)
#define RX4_DMA_IP6			BIT(27)
#else
#define RX4_DMA_ALG_SET			BIT(15)
#define RX4_DMA_SP(_x)			(((_x) >> 24) & 0x7)
#define RX4_DMA_L4F			BIT(28)
#define RX4_DMA_IPF			BIT(29)
#define RX4_DMA_L4FVLD			BIT(30)
#define RX4_DMA_IPFVLD			BIT(31)
#endif

/*=========================================
      PDMA TX Descriptor Format define
=========================================*/

struct PDMA_txdesc {
	unsigned int txd_info1_u32;
	unsigned int txd_info2_u32;
	unsigned int txd_info3_u32;
	unsigned int txd_info4_u32;
#if defined (CONFIG_RAETH_32B_DESC)
	unsigned int txd_info5_u32;
	unsigned int txd_info6_u32;
	unsigned int txd_info7_u32;
	unsigned int txd_info8_u32;
#endif
};

#define TX2_DMA_SDL1(_x)		((_x) & 0x3fff)
#define TX2_DMA_LS1			BIT(14)
#define TX2_DMA_BURST			BIT(15)
#define TX2_DMA_SDL0(_x)		(((_x) & 0x3fff) << 16)
#define TX2_DMA_LS0			BIT(30)
#define TX2_DMA_DONE			BIT(31)

#if defined (CONFIG_RALINK_MT7621)
#define TX4_DMA_VLAN_TAG(_x)		((_x) & 0x1FFFF)
#define TX4_DMA_UDF(_x)			((_x) << 19)
#define TX4_DMA_FPORT(_x)		((_x) << 25)
#define TX4_DMA_TSO			BIT(28)
#define TX4_DMA_TUI_CO(_x)		((_x) << 29)
#elif defined (CONFIG_RALINK_MT7620)
#define TX4_DMA_VIDX(_x)		((_x) & 0xf)
#define TX4_DMA_VPRI(_x)		(((_x) & VLAN_PRIO_MASK) >> 9)
#define TX4_DMA_INSV			BIT(7)
#define TX4_DMA_UDF(_x)			((_x) << 15)
#define TX4_DMA_FP_BMAP(_x)		((_x) << 20)
#define TX4_DMA_TSO			BIT(28)
#define TX4_DMA_TUI_CO(_x)		((_x) << 29)
#else
#define TX4_DMA_VIDX(_x)		((_x) & 0xf)
#define TX4_DMA_VPRI(_x)		(((_x) & VLAN_PRIO_MASK) >> 9)
#define TX4_DMA_INSV			BIT(7)
#define TX4_DMA_QN(_x)			((_x) << 16)
#define TX4_DMA_PN(_x)			((_x) << 24)
#define TX4_DMA_TUI_CO(_x)		((_x) << 29)
#endif


#define PHY_Enable_Auto_Nego		0x1000
#define PHY_Restart_Auto_Nego		0x0200

/* PHY_STAT_REG = 1; */
#define PHY_Auto_Neco_Comp		0x0020
#define PHY_Link_Status			0x0004

/* PHY_AUTO_NEGO_REG = 4; */
#define PHY_Cap_10_Half			0x0020
#define PHY_Cap_10_Full			0x0040
#define	PHY_Cap_100_Half		0x0080
#define	PHY_Cap_100_Full		0x0100

/* proc definition */

#if !defined (CONFIG_RALINK_RT6855) && !defined (CONFIG_RALINK_RT6855A) && \
    !defined (CONFIG_RALINK_MT7620) && !defined (CONFIG_RALINK_MT7621)
#define CDMA_OQ_STA			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x4c)
#define GDMA1_OQ_STA			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x50)
#define PPE_OQ_STA			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x54)
#define PSE_IQ_STA			(RALINK_FRAME_ENGINE_BASE+RAPSE_OFFSET+0x58)
#endif

#define PROCREG_CONTROL_FILE		"/var/run/procreg_control"
#if defined (CONFIG_RALINK_RT2880)
#define PROCREG_DIR			"rt2880"
#elif defined (CONFIG_RALINK_RT3052)
#define PROCREG_DIR			"rt3052"
#elif defined (CONFIG_RALINK_RT3352)
#define PROCREG_DIR			"rt3352"
#elif defined (CONFIG_RALINK_RT5350)
#define PROCREG_DIR			"rt5350"
#elif defined (CONFIG_RALINK_RT2883)
#define PROCREG_DIR			"rt2883"
#elif defined (CONFIG_RALINK_RT3883)
#define PROCREG_DIR			"rt3883"
#elif defined (CONFIG_RALINK_RT6855)
#define PROCREG_DIR			"rt6855"
#elif defined (CONFIG_RALINK_MT7620)
#define PROCREG_DIR			"mt7620"
#elif defined (CONFIG_RALINK_MT7621)
#define PROCREG_DIR			"mt7621"
#elif defined (CONFIG_RALINK_RT6855A)
#define PROCREG_DIR			"rt6855a"
#else
#define PROCREG_DIR			"rt2880"
#endif
#define PROCREG_TXRING			"tx_ring"
#define PROCREG_RXRING			"rx_ring"
#define PROCREG_NUM_OF_TXD		"num_of_txd"
#define PROCREG_TSO_LEN			"tso_len"
#define PROCREG_LRO_STATS		"lro_stats"
#define PROCREG_GMAC			"gmac"
#define PROCREG_CP0			"cp0"
#define PROCREG_RAQOS			"qos"
#define PROCREG_READ_VAL		"regread_value"
#define PROCREG_WRITE_VAL		"regwrite_value"
#define PROCREG_ADDR			"reg_addr"
#define PROCREG_CTL			"procreg_control"
#define PROCREG_RXDONE_INTR		"rxdone_intr_count"
#define PROCREG_ESW_INTR		"esw_intr_count"
#define PROCREG_ESW_CNT			"esw_cnt"
#define PROCREG_SNMP			"snmp"
#define PROCREG_VLAN_TX			"vlan_tx"


typedef struct end_device
{
#if defined (CONFIG_RAETH_ESW) && defined (CONFIG_RAETH_ESW_DHCP_TOUCH)
	struct work_struct		kill_sig_wq;
#endif
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


#endif
