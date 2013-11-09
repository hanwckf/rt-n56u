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
	rt3352.h
 
    Abstract:
	2*2 Wireless Chip SoC

    Revision History:
    Who			When			What
    ---------	----------		----------------------------------------------
	SampleLin	20100625		Initial version
 */

#ifndef __RT3352_H__
#define __RT3352_H__

#ifdef RT3352

struct _RTMP_ADAPTER;

/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */

extern REG_PAIR RT3352_RFRegTable[];
extern UCHAR RT3352_NUM_RF_REG_PARMS;
extern REG_PAIR RT3352_BBPRegTable[];
extern UCHAR RT3352_NUM_BBP_REG_PARMS;
extern RTMP_REG_PAIR RT3352_MACRegTable[];
extern UCHAR RT3352_NUM_MAC_REG_PARMS;

VOID RT3352_Init(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT3352MacRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT3352BbpRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT3352RFRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT3352_RxSensitivityTuning(
	IN struct _RTMP_ADAPTER		*pAd);

UCHAR RT3352_ChipAGCAdjust(
	IN struct _RTMP_ADAPTER		*pAd,
	IN CHAR						Rssi,
	IN UCHAR					OrigR66Value);

VOID RT3352_ChipBBPAdjust(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT3352_ChipSwitchChannel(
	IN struct _RTMP_ADAPTER 	*pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan);

VOID *RT3352_AsicTxAlcTxPwrAdjOverRF(
	IN struct _RTMP_ADAPTER		*pAd,
	IN VOID						*pTxPowerTuningEntrySrc);

VOID RT3352_RTMPSetAGCInitValue(
	IN struct _RTMP_ADAPTER		*pAd,
	IN UCHAR					BandWidth);

#ifdef RTMP_INTERNAL_TX_ALC
VOID RT3352_InitDesiredTSSITable(
	IN struct _RTMP_ADAPTER		*pAd);

UCHAR RT3352_GetDesiredTSSI(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT3352_AsicTxAlcGetAutoAgcOffset(
	IN struct _RTMP_ADAPTER		*pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1);
#endif /* RTMP_INTERNAL_TX_ALC */
#endif /* RT3352 */
#endif /*__RT3352_H__ */

/* End of rt3352.h */
