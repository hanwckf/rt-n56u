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
	rt3883.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT3883_H__
#define __RT3883_H__

#ifndef RTMP_RBUS_SUPPORT
#error "For RT3883, you should define the compile flag -DRTMP_RBUS_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT3883, you should define the compile flag -DRTMP_MAC_PCI"
#endif

struct _RTMP_ADAPTER;

#define RTMP_MAC_CSR_ADDR 0xB0180000
#define RTMP_FLASH_BASE_ADDR	0xbc000000


#define BBP_REG_BF		BBP_R163
#define BBP_REG_SNR0	BBP_R160
#define BBP_REG_SNR1	BBP_R161
#define BBP_REG_SNR2	BBP_R162


#define MAX_RF_TX_POWER		31	// Maximum Tx Power value in RF Register

// Macro to convert Tx Power setting to RF Reg for A Band
#define TX_PWR_TO_RF_REG(p)	(CHAR)(0x48 | (((p) & 0x18) << 1) | ((p) & 0x7))

extern UCHAR NUM_OF_3883_CHNL; 
extern FREQUENCY_ITEM FreqItems3883[];

VOID RTMPRT3883ABandSel(
	IN UCHAR Channel);

VOID RTMPRT3883ReadChannelPwr(
	IN struct _RTMP_ADAPTER *pAd);

VOID RTMPRT3883ReadTxPwrPerRate(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT3883_Init(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT3883_AsicGetTxPowerOffset(
	IN 		struct _RTMP_ADAPTER			*pAd,
	INOUT 	PULONG 						pTxPwr);

#ifdef CONFIG_AP_SUPPORT
int RT3883_ext_pkt_len(
	IN UCHAR *pOutBuffer,
	IN ULONG FrameLen,
	IN UCHAR *RalinkSpecificIe,
	IN UCHAR IeLen);
#endif /* CONFIG_AP_SUPPORT */

VOID RT3883_CWC_ProtectAdjust(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR *pSetMask,
	IN USHORT *pOperationMode);

VOID RT3883_CWC_AsicSet(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN bCwc);

void RT3883_AsicSetFreqOffset(
	IN struct _RTMP_ADAPTER *pAd,
	IN ULONG freqOffset);

#endif /*__RT3883_H__ */

