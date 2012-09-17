/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	btco.h

	Abstract:

	Handling Bluetooth Coexistence Problem

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Sean Wang	2009-08-12		Create
	John Li		2009-12-23		Modified
*/

#ifdef BT_COEXISTENCE_SUPPORT


#ifndef __BTCO_H
#define __BTCO_H

#include "btco_cmm.h"

#define IN
#define OUT
#define INOUT

#define BT_CYRSTALL_SHARED			0x01
#define WIFI_2040_SWITCH_THRESHOLD 	BUSY_4
#define BT_HISTORY_RECORD_NUM 		5
#ifdef RTMP_PCI_SUPPORT
#define BT_CHECK_TIME_INTERVAL 		100   //unit : 10ms, must follow timer interval
#define BT_IDLE_STATE_THRESHOLD 	10	//when > BT_IDLE_STATE_THRESHOLD, we treat that Bluetooth is no busy
#define BT_DETECT_TIMEOUT			10
#endif // RTMP_PCI_SUPPORT //

#ifdef DOT11N_DRAFT3
BOOLEAN BtCheckWifiThroughputOverLimit(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR WifiThroughputLimit);
#endif // DOT11N_DRAFT3 //

BLUETOOTH_BUSY_DEGREE BtCheckBusy(
	IN PLONG BtHistory, 
	IN UCHAR BtHistorySize);

VOID BtCoexistAdjust(
	IN PRTMP_ADAPTER	pAd, 
	IN BOOLEAN			bIssue4020, 
	IN ULONG			NoBusyTimeCount);

VOID BtCoexistTxPowerDown(
	IN PRTMP_ADAPTER	pAd, 
	IN CHAR				Rssi,
	INOUT CHAR			*pDeltaPowerByBbpR1, 
	INOUT CHAR			*pDeltaPwr);

VOID BtCoexistMcsDown(
	IN PRTMP_ADAPTER	pAd, 
	IN CHAR				CurrRateIdx, 
	IN PRTMP_TX_RATE_SWITCH	pCurrTxRate, 
	INOUT CHAR			*pUpRateIdx, 
	INOUT CHAR			*pDownRateIdx);

VOID BtCoexistMcsDown2(
	IN PRTMP_ADAPTER	pAd, 
	IN UCHAR			MCS3, 
	IN UCHAR			MCS4, 
	IN UCHAR			MCS5, 
	IN UCHAR			MCS6, 
	INOUT UCHAR			*pTxRateIdx);

VOID BtCoexistTxBaSizeDown(
	IN PRTMP_ADAPTER	pAd, 
	INOUT PTXWI_STRUC 	pTxWI);


VOID BtCoexistTxBaDensityDown(
	IN PRTMP_ADAPTER	pAd, 
	INOUT PTXWI_STRUC 	pTxWI);

VOID BtCoexistReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING pBuffer);

VOID BtCoexistInit(
	IN PRTMP_ADAPTER pAd);

VOID BtCoexistUserCfgInit(
	IN PRTMP_ADAPTER pAd);


VOID BtCoexistDetectExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

#endif /* __BTCO_H */
#endif // BT_COEXISTENCE_SUPPORT //

