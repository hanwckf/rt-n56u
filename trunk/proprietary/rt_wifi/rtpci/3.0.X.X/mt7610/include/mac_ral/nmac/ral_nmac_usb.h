/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	ral_nmac_usb.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RAL_OMAC_USB_H__
#define __RAL_OMAC_USB_H__
#define CMB_CTRL		0x20
#ifdef RT_BIG_ENDIAN
typedef union _CMB_CTRL_STRUC{
	struct{
		UINT32       	LDO0_EN:1;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	PLL_LD:1;
		UINT32       	XTAL_RDY:1;
#ifdef RT65xx
		UINT32		Rsv:3;
		UINT32		GPIOModeLed2:1;
		UINT32		GPIOModeLed1:1;
		UINT32		CsrUartMode:1;
#else
		UINT32      Rsv:2;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_LEVEL:2;
#endif
		UINT32		AUX_OPT_Bit15_Two_AntennaMode:1;
		UINT32		AUX_OPT_Bit14_TRSW1_as_GPIO:1;
		UINT32		AUX_OPT_Bit13_GPIO7_as_GPIO:1;
		UINT32		AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;
		UINT32		AUX_OPT_Bit11_Rsv:1;
		UINT32		AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1;
		UINT32		AUX_OPT_Bit9_GPIO3_as_GPIO:1;
		UINT32		AUX_OPT_Bit8_AuxPower_Exists:1;
		UINT32		AUX_OPT_Bit7_KeepInterfaceClk:1;
		UINT32		AUX_OPT_Bit6_KeepXtal_On:1;
		UINT32		AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;
		UINT32		AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;
		UINT32		AUX_OPT_Bit3_PLLOn_L1:1;
		UINT32		AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
		UINT32		AUX_OPT_Bit1_PCIePhyClkOn_L1:1;	
		UINT32		AUX_OPT_Bit0_InterfaceClk_40Mhz:1;
	}field;
	UINT32 word;
}CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#else
typedef union _CMB_CTRL_STRUC{
	struct{
		UINT32		AUX_OPT_Bit0_InterfaceClk_40Mhz:1;
		UINT32		AUX_OPT_Bit1_PCIePhyClkOn_L1:1;	
		UINT32		AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
		UINT32		AUX_OPT_Bit3_PLLOn_L1:1;
		UINT32		AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;
		UINT32		AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;
		UINT32		AUX_OPT_Bit6_KeepXtal_On:1;
		UINT32		AUX_OPT_Bit7_KeepInterfaceClk:1;
		UINT32		AUX_OPT_Bit8_AuxPower_Exists:1;
		UINT32		AUX_OPT_Bit9_GPIO3_as_GPIO:1;
		UINT32		AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1;	
		UINT32		AUX_OPT_Bit11_Rsv:1;
		UINT32		AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;
		UINT32		AUX_OPT_Bit13_GPIO7_as_GPIO:1;
		UINT32		AUX_OPT_Bit14_TRSW1_as_GPIO:1;
		UINT32		AUX_OPT_Bit15_Two_AntennaMode:1;
#ifdef RT65xx
		UINT32		CsrUartMode:1;
		UINT32		GPIOModeLed1:1;
		UINT32		GPIOModeLed2:1;
		UINT32		Rsv:3;
#else
		UINT32		LDO25_LEVEL:2;
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32       	Rsv:2;
#endif /* RT65xx */
		UINT32       	XTAL_RDY:1;
		UINT32       	PLL_LD:1;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO0_EN:1;
	}field;
	UINT32 word;
}CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#endif




#define USB_DMA_CFG 0x0238
#ifdef RT_BIG_ENDIAN
typedef	union _USB_DMA_CFG_STRUC {
	struct {
		UINT32 TxBusy:1;			/* USB DMA TX FSM busy. debug only */
		UINT32 RxBusy:1;			/* USB DMA RX FSM busy. debug only */
		UINT32 EpoutValid:6;		/* OUT endpoint data valid. debug only */
		UINT32 TxBulkEn:1;			/* Enable USB DMA Tx */
		UINT32 RxBulkEn:1;			/* Enable USB DMA Rx */
		UINT32 RxBulkAggEn:1;		/* Enable Rx Bulk Aggregation */
		UINT32 TxopHalt:1;			/* Halt TXOP count down when TX buffer is full. */
		UINT32 TxClear:1;        	/* Clear USB DMA TX path */
		UINT32 UDMA_RX_WL_DROP:1 	/* Drop current WL RX packets in UDMA */
		UINT32 WAKEUP_EN:1;			/* USB wakeup host enable */
		UINT32 UDMA_TX_WL_DROP:1;	/* Drop current WL TX packets in UDMA */
		UINT32 RxBulkAggLmt:8;		/* Rx Bulk Aggregation Limit  in unit of 1024 bytes */
		UINT32 RxBulkAggTOut:8;		/* Rx Bulk Aggregation TimeOut  in unit of 33ns */
	} field;
	UINT32 word;
} USB_DMA_CFG_STRUC, *PUSB_DMA_CFG_STRUC;
#else
typedef	union _USB_DMA_CFG_STRUC {
	struct {
		UINT32 RxBulkAggTOut:8;		/* Rx Bulk Aggregation TimeOut  in unit of 33ns */
		UINT32 RxBulkAggLmt:8;		/* Rx Bulk Aggregation Limit  in unit of 256 bytes */
		UINT32 UDMA_TX_WL_DROP:1;	/* Drop current WL TX packets in UDMA */
		UINT32 WAKEUP_EN:1;			/* USB wakeup host enable */
		UINT32 UDMA_RX_WL_DROP:1; 	/* Drop current WL RX packets in UDMA */
		UINT32 TxClear:1;			/* Clear USB DMA TX path */
		UINT32 TxopHalt:1;			/* Halt TXOP count down when TX buffer is full. */
		UINT32 RxBulkAggEn:1;		/* Enable Rx Bulk Aggregation */
		UINT32 RxBulkEn:1;			/* Enable USB DMA Rx */
		UINT32 TxBulkEn:1;			/* Enable USB DMA Tx */
		UINT32 EpoutValid:6;        /* OUT endpoint data valid */
		UINT32 RxBusy:1;			/* USB DMA RX FSM busy */
		UINT32 TxBusy:1;			/* USB DMA TX FSM busy */
	} field;
	UINT32 word;
} USB_DMA_CFG_STRUC, *PUSB_DMA_CFG_STRUC;
#endif

#endif /*__RAL_OMAC_USB_H__ */

