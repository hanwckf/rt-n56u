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

    Abstract:


 */

#include "ap_autoChSel_cmm.h"

#ifndef __AUTOCHSELECT_H__
#define __AUTOCHSELECT_H__

#define AP_AUTO_CH_SEL(__P, __O, __IsABand)	APAutoSelectChannel((__P), (__O),__IsABand)

ULONG AutoChBssSearchWithSSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR Bssid,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN UCHAR Channel);

VOID APAutoChannelInit(
	IN PRTMP_ADAPTER pAd);

VOID UpdateChannelInfo(
	IN PRTMP_ADAPTER pAd,
	IN int ch,
	IN ChannelSel_Alg Alg);

ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen, 
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi);

VOID AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoInit(
	IN PRTMP_ADAPTER pAd);

VOID AutoChBssTableDestroy(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd);

VOID CheckPhyModeIsABand(
	IN PRTMP_ADAPTER pAd);

UCHAR SelectBestChannel(
	IN PRTMP_ADAPTER pAd,
	IN ChannelSel_Alg Alg);

UCHAR APAutoSelectChannel(
	IN PRTMP_ADAPTER pAd,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand);

UCHAR MTAPAutoSelectChannel(
        IN RTMP_ADAPTER *pAd, 
        IN ChannelSel_Alg Alg, 
        IN BOOLEAN IsABand);

UCHAR RTMPAPAutoSelectChannel(
        IN RTMP_ADAPTER *pAd, 
        IN ChannelSel_Alg Alg, 
        IN BOOLEAN IsABand);

UCHAR RLTAPAutoSelectChannel(
        IN RTMP_ADAPTER *pAd, 
        IN ChannelSel_Alg Alg, 
        IN BOOLEAN IsABand);

#ifdef AP_SCAN_SUPPORT
VOID AutoChannelSelCheck(
	IN PRTMP_ADAPTER pAd);
#endif /* AP_SCAN_SUPPORT */

VOID AutoChSelBuildChannelList(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN IsABand);

VOID AutoChSelBuildChannelListFor2G(
	IN RTMP_ADAPTER *pAd);

VOID AutoChSelBuildChannelListFor5G(
	IN RTMP_ADAPTER *pAd);

VOID AutoChSelUpdateChannel(
    IN RTMP_ADAPTER *pAd, 
    IN UCHAR Channel, 
    IN BOOLEAN IsABand);

VOID AutoChSelSwitchChannel(
	IN RTMP_ADAPTER *pAd, 
	IN PAUTOCH_SEL_CH_LIST pAutoChSelChList, 
	IN BOOLEAN bScan);

CHAR AutoChSelFindScanChIdx(
	IN RTMP_ADAPTER *pAd, 
	IN CHAR LastScanChIdx);

VOID AutoChSelScanNextChannel(
    IN RTMP_ADAPTER *pAd,
    IN struct wifi_dev *pwdev);

VOID AutoChSelScanReqAction(
    IN RTMP_ADAPTER *pAd, 
    IN MLME_QUEUE_ELEM *pElem);

VOID AutoChSelScanTimeoutAction(
    IN RTMP_ADAPTER *pAd, 
    IN MLME_QUEUE_ELEM *pElem);

VOID AutoChSelScanStart(
    IN RTMP_ADAPTER *pAd,
    IN struct wifi_dev *pwdev);

VOID AutoChSelScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AutoChSelStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AutoChSelInit(
	IN PRTMP_ADAPTER pAd);

VOID AutoChSelRelease(
	IN PRTMP_ADAPTER pAd);
#endif /* __AUTOCHSELECT_H__ */

