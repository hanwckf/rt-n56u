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
	ral_omac_pci.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RAL_OMAC_PCI_H__
#define __RAL_OMAC_PCI_H__


/* INT_SOURCE_CSR: Interrupt source register. Write one to clear corresponding bit */
#define INT_SOURCE_CSR		0x200

#define RxINT				0x00000005	/* Delayed Rx or indivi rx */
#define TxDataInt			0x000000fa	/* Delayed Tx or indivi tx */
#define TxMgmtInt			0x00000102	/* Delayed Tx or indivi tx */
#define TxCoherent			0x00020000	/* tx coherent */
#define RxCoherent			0x00010000	/* rx coherent */
#define TxRxCoherent			0x00000400	/* tx rx coherent */
#define McuCommand			0x00000200	/* mcu */
#define PreTBTTInt			0x00001000	/* Pre-TBTT interrupt */
#define TBTTInt				0x00000800		/* TBTT interrupt */
#define GPTimeOutInt			0x00008000		/* GPtimeout interrupt */
#define AutoWakeupInt		0x00004000		/* AutoWakeupInt interrupt */
#define FifoStaFullInt			0x00002000	/*  fifo statistics full interrupt */



#define RT2860_INT_RX_DLY				(1<<0)		/* bit 0 */
#define RT2860_INT_TX_DLY				(1<<1)		/* bit 1 */
#define RT2860_INT_RX_DONE				(1<<2)		/* bit 2 */
#define RT2860_INT_AC0_DMA_DONE			(1<<3)		/* bit 3 */
#define RT2860_INT_AC1_DMA_DONE			(1<<4)		/* bit 4 */
#define RT2860_INT_AC2_DMA_DONE			(1<<5)		/* bit 5 */
#define RT2860_INT_AC3_DMA_DONE			(1<<6)		/* bit 6 */
#define RT2860_INT_HCCA_DMA_DONE		(1<<7)		/* bit 7 */
#define RT2860_INT_MGMT_DONE			(1<<8)		/* bit 8 */
#ifdef CARRIER_DETECTION_SUPPORT
#define RT2860_INT_TONE_RADAR			(1<<20)		/* bit 20 */
#endif /* CARRIER_DETECTION_SUPPORT*/

#define INT_RX			RT2860_INT_RX_DONE

#define INT_AC0_DLY		(RT2860_INT_AC0_DMA_DONE) /*| RT2860_INT_TX_DLY) */
#define INT_AC1_DLY		(RT2860_INT_AC1_DMA_DONE) /*| RT2860_INT_TX_DLY) */
#define INT_AC2_DLY		(RT2860_INT_AC2_DMA_DONE) /*| RT2860_INT_TX_DLY) */
#define INT_AC3_DLY		(RT2860_INT_AC3_DMA_DONE) /*| RT2860_INT_TX_DLY) */
#define INT_HCCA_DLY 	(RT2860_INT_HCCA_DMA_DONE) /*| RT2860_INT_TX_DLY) */
#define INT_MGMT_DLY	RT2860_INT_MGMT_DONE
#ifdef CARRIER_DETECTION_SUPPORT
#define INT_TONE_RADAR	(RT2860_INT_TONE_RADAR)
#endif /* CARRIER_DETECTION_SUPPORT*/


#ifdef CARRIER_DETECTION_SUPPORT
#define DELAYINTMASK		0x0013fffb
#define INTMASK				0x0013fffb
#define IndMask				0x0013fffc
#define RadarInt			0x00100000
#else
#define DELAYINTMASK		0x0003fffb
#define INTMASK				0x0003fffb
#define IndMask				0x0003fffc
#endif /* CARRIER_DETECTION_SUPPORT */



#ifdef RT_BIG_ENDIAN
typedef	union _INT_SOURCE_CSR_STRUC {
	struct {
#ifdef CARRIER_DETECTION_SUPPORT
		UINT32			:11;
		UINT32			RadarINT:1;
		UINT32       	rsv:2;
#else /* original source code */
		UINT32       	:14;
#endif /* CARRIER_DETECTION_SUPPORT */
		UINT32       	TxCoherent:1;
		UINT32       	RxCoherent:1;
		UINT32       	GPTimer:1;
		UINT32       	AutoWakeup:1;/*bit14 */
		UINT32       	TXFifoStatusInt:1;/*FIFO Statistics is full, sw should read 0x171c */
		UINT32       	PreTBTT:1;
		UINT32       	TBTTInt:1;
		UINT32       	RxTxCoherent:1;
		UINT32       	MCUCommandINT:1;
		UINT32       	MgmtDmaDone:1;
		UINT32       	HccaDmaDone:1;
		UINT32       	Ac3DmaDone:1;
		UINT32       	Ac2DmaDone:1;
		UINT32       	Ac1DmaDone:1;
		UINT32		Ac0DmaDone:1;
		UINT32		RxDone:1;
		UINT32		TxDelayINT:1;	/*delayed interrupt, not interrupt until several int or time limit hit */
		UINT32		RxDelayINT:1; /*dealyed interrupt */
	}field;
	UINT32			word;
}	INT_SOURCE_CSR_STRUC;
#else
typedef	union _INT_SOURCE_CSR_STRUC {
	struct	{
		UINT32		RxDelayINT:1;
		UINT32		TxDelayINT:1;
		UINT32		RxDone:1;
		UINT32		Ac0DmaDone:1;/*4 */
		UINT32       	Ac1DmaDone:1;
		UINT32       	Ac2DmaDone:1;
		UINT32       	Ac3DmaDone:1;
		UINT32       	HccaDmaDone:1; /* bit7 */
		UINT32       	MgmtDmaDone:1;
		UINT32       	MCUCommandINT:1;/*bit 9 */
		UINT32       	RxTxCoherent:1;
		UINT32       	TBTTInt:1;
		UINT32       	PreTBTT:1;
		UINT32       	TXFifoStatusInt:1;/*FIFO Statistics is full, sw should read 0x171c */
		UINT32       	AutoWakeup:1;/*bit14 */
		UINT32       	GPTimer:1;
		UINT32       	RxCoherent:1;/*bit16 */
		UINT32       	TxCoherent:1;
#ifdef CARRIER_DETECTION_SUPPORT
		UINT32       	rsv:2;
		UINT32			RadarINT:1;
		UINT32			:11;
#else
		UINT32       	:14;
#endif /* CARRIER_DETECTION_SUPPORT */
	}	field;
	UINT32			word;
} INT_SOURCE_CSR_STRUC;
#endif


/* INT_MASK_CSR:   Interrupt MASK register.   1: the interrupt is mask OFF */
#define INT_MASK_CSR        0x204
#ifdef RT_BIG_ENDIAN
typedef	union _INT_MASK_CSR_STRUC {
	struct	{
		UINT32       	TxCoherent:1;
		UINT32       	RxCoherent:1;
#ifdef CARRIER_DETECTION_SUPPORT
		UINT32			:9;
		UINT32			RadarINT:1;
		UINT32       	rsv:10;
#else
		UINT32       	:20;
#endif /* CARRIER_DETECTION_SUPPORT */
		UINT32       	MCUCommandINT:1;
		UINT32       	MgmtDmaDone:1;
		UINT32       	HccaDmaDone:1;
		UINT32       	Ac3DmaDone:1;
		UINT32       	Ac2DmaDone:1;
		UINT32       	Ac1DmaDone:1;
		UINT32		Ac0DmaDone:1;
		UINT32		RxDone:1;
		UINT32		TxDelay:1;
		UINT32		RXDelay_INT_MSK:1;
	}	field;
	UINT32			word;
}INT_MASK_CSR_STRUC, *PINT_MASK_CSR_STRUC;
#else
typedef	union _INT_MASK_CSR_STRUC {
	struct {
		UINT32		RXDelay_INT_MSK:1;
		UINT32		TxDelay:1;
		UINT32		RxDone:1;
		UINT32		Ac0DmaDone:1;
		UINT32       	Ac1DmaDone:1;
		UINT32       	Ac2DmaDone:1;
		UINT32       	Ac3DmaDone:1;
		UINT32       	HccaDmaDone:1;
		UINT32       	MgmtDmaDone:1;
		UINT32       	MCUCommandINT:1;
#ifdef CARRIER_DETECTION_SUPPORT
		UINT32       	rsv:10;
		UINT32			RadarINT:1;
		UINT32			:9;
#else
		UINT32       	:20;
#endif /* CARRIER_DETECTION_SUPPORT */
		UINT32       	RxCoherent:1;
		UINT32       	TxCoherent:1;
	}	field;
	UINT32			word;
} INT_MASK_CSR_STRUC, *PINT_MASK_CSR_STRUC;
#endif


#define RINGREG_DIFF			0x10
#define TX_BASE_PTR0     0x0230	/*AC_BK base address */
#define TX_MAX_CNT0      0x0234
#define TX_CTX_IDX0       0x0238
#define TX_DTX_IDX0      0x023c
#define TX_BASE_PTR1     0x0240 	/*AC_BE base address */
#define TX_MAX_CNT1      0x0244
#define TX_CTX_IDX1       0x0248
#define TX_DTX_IDX1      0x024c
#define TX_BASE_PTR2     0x0250 	/*AC_VI base address */
#define TX_MAX_CNT2      0x0254
#define TX_CTX_IDX2       0x0258
#define TX_DTX_IDX2      0x025c
#define TX_BASE_PTR3     0x0260 	/*AC_VO base address */
#define TX_MAX_CNT3      0x0264
#define TX_CTX_IDX3       0x0268
#define TX_DTX_IDX3      0x026c
#define TX_BASE_PTR4     0x0270 	/*HCCA base address */
#define TX_MAX_CNT4      0x0274
#define TX_CTX_IDX4       0x0278
#define TX_DTX_IDX4      0x027c
#define TX_BASE_PTR5     0x0280 	/*MGMT base address */
#define  TX_MAX_CNT5     0x0284
#define TX_CTX_IDX5       0x0288
#define TX_DTX_IDX5      0x028c
#define TX_MGMTMAX_CNT      TX_MAX_CNT5
#define TX_MGMTCTX_IDX       TX_CTX_IDX5
#define TX_MGMTDTX_IDX      TX_DTX_IDX5
#define RX_BASE_PTR     0x0290 	/*RX base address */
#define RX_MAX_CNT      0x0294
#define RX_CRX_IDX       0x0298
#define RX_DRX_IDX      0x029c


#define US_CYC_CNT      0x02a4
#ifdef BIG_ENDIAN
typedef	union _US_CYC_CNT_STRUC {
	struct {
	    UINT32  rsv2:7;
	    UINT32  TestEn:1;
	    UINT32  TestSel:8;
	    UINT32  rsv1:7;
	    UINT32  MiscModeEn:1;
	    UINT32  UsCycCnt:8;
	} field;
	UINT32 word;
} US_CYC_CNT_STRUC;
#else
typedef	union _US_CYC_CNT_STRUC {
	struct {
		UINT32  UsCycCnt:8;
		UINT32  MiscModeEn:1;
		UINT32  rsv1:7;
		UINT32  TestSel:8;
		UINT32  TestEn:1;
		UINT32  rsv2:7;
	} field;
	UINT32 word;
} US_CYC_CNT_STRUC;
#endif


#endif /*__RAL_OMAC_PCI_H__ */

