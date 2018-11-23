#ifdef MTK_LICENSE
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
#endif /* MTK_LICENSE */
#include "bgnd_scan_cmm.h"

#ifndef __BGND_SCAN_H__
#define __BGND_SCAN_H__

VOID BackgroundScanCancelAction(	RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanStartAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanTimeoutAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundSwitchChannelAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundChannelSwitchAnnouncementAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanPartialAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanWaitAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID BackgroundScanNextChannel (IN PRTMP_ADAPTER pAd, IN UINT8 ScanType);
VOID BackgroundScanInit (IN PRTMP_ADAPTER pAd);
VOID BackgroundScanDeInit (IN PRTMP_ADAPTER pAd);
VOID BackgroundScanStart(IN PRTMP_ADAPTER pAd, IN UINT8 BgndscanType);

void BackgroundScanTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);
void DfsZeroWaitTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);
VOID BackgroundScanTest(IN PRTMP_ADAPTER pAd,	IN MT_BGND_SCAN_CFG BgndScanCfg);
VOID ChannelQualityDetection(IN PRTMP_ADAPTER pAd);

VOID DfsZeroWaitStartAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID DfsZeroWaitStopAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);
VOID DfsZeroWaitStart(IN PRTMP_ADAPTER pAd, IN BOOLEAN DfsZeroWaitEnable);
VOID BfSwitch(IN PRTMP_ADAPTER pAd, IN UCHAR enabled);
VOID MuSwitch(IN PRTMP_ADAPTER pAd, IN UCHAR enabled);
#endif /* __BGND_SCAN_H__ */