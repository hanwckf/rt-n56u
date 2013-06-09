/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All Dynamic Rate Switch Related Structure & Definition

***************************************************************************/

#ifndef __DRS_EXTR_H__
#define __DRS_EXTR_H__

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
#ifdef NEW_RATE_ADAPT_SUPPORT
struct _RTMP_TX_RATE_SWITCH_3S;
#endif /* NEW_RATE_ADAPT_SUPPORT */
	
enum RATE_ADAPT_ALG{
	RATE_ADAPT_LEGACY = 1,
	RATE_ADAPT_GRP = 2,
	RATE_ADAPT_AGS = 3,
	RATE_ALG_MAX_NUM
};


extern UCHAR RateSwitchTable[];
extern UCHAR RateSwitchTable11B[];
extern UCHAR RateSwitchTable11G[];
extern UCHAR RateSwitchTable11BG[];

#ifdef DOT11_N_SUPPORT
extern UCHAR RateSwitchTable11BGN1S[];
extern UCHAR RateSwitchTable11BGN2S[];
extern UCHAR RateSwitchTable11BGN2SForABand[];
extern UCHAR RateSwitchTable11N1S[];
extern UCHAR RateSwitchTable11N1SForABand[];
extern UCHAR RateSwitchTable11N2S[];
extern UCHAR RateSwitchTable11N2SForABand[];
extern UCHAR RateSwitchTable11BGN3S[];
extern UCHAR RateSwitchTable11BGN3SForABand[];

#ifdef NEW_RATE_ADAPT_SUPPORT
extern UCHAR RateSwitchTableAdapt11N1S[];
extern UCHAR RateSwitchTableAdapt11N2S[];
extern UCHAR RateSwitchTableAdapt11N3S[];

#define PER_THRD_ADJ			1

#define RTMP_DRS_GROUP_ALG_IS_SELECTED(__pTable)	\
	((__pTable) == RateSwitchTableAdapt11N3S)

// ADAPT_RATE_TABLE - true if pTable is one of the Adaptive Rate Switch tables
#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11N1S || (pTable)==RateSwitchTableAdapt11N2S || (pTable)==RateSwitchTableAdapt11N3S)
#endif // NEW_RATE_ADAPT_SUPPORT //
#endif /* DOT11_N_SUPPORT */


/* FUNCTION */
VOID MlmeGetSupportedMcs(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR	*pTable,
	OUT CHAR 	mcs[]);

UCHAR MlmeSelectTxRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN CHAR	mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset);

VOID MlmeClearTxQuality(
	IN struct _MAC_TABLE_ENTRY *pEntry);

VOID MlmeClearAllTxQuality(
	IN struct _MAC_TABLE_ENTRY *pEntry);

VOID MlmeDecTxQuality(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			rateIndex);

VOID MlmeSetTxQuality(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			rateIndex,
	IN USHORT			txQuality);

USHORT MlmeGetTxQuality(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			rateIndex);

VOID MlmeOldRateAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			CurrRateIdx,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio);

VOID MlmeRestoreLastRate(
	IN struct _MAC_TABLE_ENTRY *pEntry);

VOID MlmeCheckRDG(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry);

#ifdef NEW_RATE_ADAPT_SUPPORT
VOID MlmeSetMcsGroup(
	IN struct _RTMP_ADAPTER *pAd,
	OUT struct _MAC_TABLE_ENTRY *pEntry);

UCHAR MlmeSelectUpRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN struct _RTMP_TX_RATE_SWITCH_3S *pCurrTxRate);

UCHAR MlmeSelectDownRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			CurrRateIdx);

VOID MlmeGetSupportedMcsAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR	mcs23GI,
	OUT CHAR 	mcs[]);

UCHAR MlmeSelectTxRateAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN CHAR		mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset);

BOOLEAN MlmeRAHybridRule(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN struct _RTMP_TX_RATE_SWITCH_3S *pCurrTxRate,
	IN ULONG			NewTxOkCount,
	IN ULONG			TxErrorRatio);

VOID MlmeNewRateAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio);

INT	Set_PerThrdAdj_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING arg);

INT	Set_LowTrafficThrd_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpRule_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpRuleRSSI_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpLowThrd_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpHighThrd_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT Set_RateTable_Proc(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  PSTRING arg);

#ifdef CONFIG_AP_SUPPORT
VOID APMlmeDynamicTxRateSwitchingAdapt(
    IN struct _RTMP_ADAPTER *pAd,
    IN ULONG					idx);

VOID APQuickResponeForRateUpExecAdapt(
    IN struct _RTMP_ADAPTER *pAd,
    IN ULONG					idx);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID StaQuickResponeForRateUpExecAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN ULONG i,
	IN CHAR  Rssi);

VOID MlmeDynamicTxRateSwitchingAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN ULONG i,
	IN ULONG TxSuccess,
	IN ULONG TxRetransmit,
	IN ULONG TxFailCount);
#endif /* CONFIG_STA_SUPPORT */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID APMlmeDynamicTxRateSwitching(
    IN struct _RTMP_ADAPTER *pAd);

VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID MlmeDynamicTxRateSwitching(
	IN struct _RTMP_ADAPTER *pAd);

VOID StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);
#endif /* CONFIG_STA_SUPPORT */

VOID MlmeRAInit(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry);


/* normal rate switch */
#define RTMP_DRS_ALG_INIT(__pAd, __Alg)										\
	(__pAd)->rateAlg = __Alg;

#endif /* __DRS_EXTR_H__ */

/* End of drs_extr.h */
