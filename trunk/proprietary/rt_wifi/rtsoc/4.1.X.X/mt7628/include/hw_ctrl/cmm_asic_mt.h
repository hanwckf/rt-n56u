/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic_mt.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_ASIC_MT_H__
#define __CMM_ASIC_MT_H__


struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;




#define PRETBTT_INT_EN 		1 << 7
#define BMC_TIMEOUT_EN 		1 << 6
#define BCN_TIMEOUT_EN 		1 << 5
#define PRETBTT_TIMEUP_EN 	1<<  4 
#define TBTT_TIMEUP_EN   		1<<  3
#define PREDTIM_TRIG_EN 		1<<  2 
#define PRETBTT_TRIG_EN 		1<<  1
#define TBTT_PERIOD_TIMER_EN 1 << 0

typedef enum {
	MT_BSS_MODE_AP=0,
	MT_BSS_MODE_ADHOC,
	MT_BSS_MODE_STA,
}MT_BSS_OP_MODE_T;


typedef struct {
	UINT8	BssSet;	
	UINT8 	PreTbttInterval;
	USHORT BeaconPeriod;
	USHORT DtimPeriod;
	MT_BSS_OP_MODE_T BssOpMode;
} MT_BSS_SYNC_CTRL_T;



#ifdef RTMP_MAC_PCI
VOID MTPciMlmeRadioOn(struct _RTMP_ADAPTER *pAd);
VOID MTPciMlmeRadioOff(struct _RTMP_ADAPTER *pAd);
VOID MTPciPollTxRxEmpty(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_MAC_PCI */


UINT32 MtAsicGetCrcErrCnt(struct _RTMP_ADAPTER *pAd);
UINT32 MtAsicGetPhyErrCnt(struct _RTMP_ADAPTER *pAd);
UINT32 MtAsicGetCCACnt(struct _RTMP_ADAPTER *pAd);
UINT32 MtAsicGetChBusyCnt(struct _RTMP_ADAPTER *pAd, UCHAR ch_idx);
#ifdef CONFIG_STA_SUPPORT
VOID MtAsicUpdateAutoFallBackTable(struct _RTMP_ADAPTER *pAd, UCHAR *pRateTable);
#endif /* CONFIG_STA_SUPPORT */
INT MtAsicSetAutoFallBack(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);

INT32 MtAsicAutoFallbackInit(struct _RTMP_ADAPTER *pAd);

VOID MtAsicUpdateProtect(
	IN struct _RTMP_ADAPTER *pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN BOOLEAN bDisableBGProtect,
	IN BOOLEAN bNonGFExist);

VOID MtAsicSwitchChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel, BOOLEAN bScan);

VOID MtAsicResetBBPAgent(struct _RTMP_ADAPTER *pAd);
VOID MtAsicSetBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR curr_bssid_idx);
INT MtAsicSetDevMac(struct _RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx);

#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssMode(struct _RTMP_ADAPTER *pAd, UCHAR NumOfBcns);
#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT MtAsicSetMacAddrExt(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
VOID MtAsicInsertRepeaterEntry(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *pAddr);
VOID MtAsicRemoveRepeaterEntry(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx);
void insert_repeater_root_entry(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN  UCHAR *pAddr,
	IN UCHAR ReptCliIdx);
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


INT MtAsicSetRxFilter(struct _RTMP_ADAPTER *pAd);

INT MtAsicUpdateTxOP(struct _RTMP_ADAPTER *pAd, UINT32 ac_num, UINT32 txop_val);
INT MtAsicSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
INT MtAsicWtblSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);

VOID MtAsicSetPiggyBack(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);

INT MtAsicSetPreTbtt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR idx);
INT MtAsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
INT MtAsicSetChBusyStat(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT MtAsicGetTsfTime(struct _RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part);

#ifdef CONFIG_AP_SUPPORT
VOID APCheckBcnQHandler(struct _RTMP_ADAPTER *pAd, INT apidx, BOOLEAN *is_pretbtt_int);
#endif /* CONFIG_AP_SUPPORT */

VOID MtAsicDisableSync(struct _RTMP_ADAPTER *pAd);
VOID MtAsicEnableBssSync(struct _RTMP_ADAPTER *pAd,MT_BSS_SYNC_CTRL_T BssSync);

INT MtAsicSetWmmParam(struct _RTMP_ADAPTER *pAd, UINT ac, UINT type, UINT val);
VOID MtAsicSetEdcaParm(struct _RTMP_ADAPTER *pAd, struct _EDCA_PARM *pEdcaParm);

INT MtAsicSetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
UINT32 MtAsicGetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type);

VOID MtAsicSetSlotTime(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime);

INT MtAsicSetMacMaxLen(struct _RTMP_ADAPTER *pAd);

#ifdef CONFIG_AP_SUPPORT
VOID MtAsicGetTxTsc(struct _RTMP_ADAPTER *pAd, UCHAR apidx, UCHAR *pTxTsc);
#endif /* CONFIG_AP_SUPPORT */

VOID MtAsicAddSharedKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx, struct _CIPHER_KEY *pKey);
VOID MtAsicRemoveSharedKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx);
VOID MtAsicUpdateWCIDIVEIV(struct _RTMP_ADAPTER *pAd, USHORT WCID, ULONG uIV, ULONG uEIV);

VOID Wtbl2RateTableUpdate(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid, UINT32 u4Wtbl2D9, UINT32* Rate);
VOID Wtbl2TxRateCounterGet(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid, TX_CNT_INFO *tx_cnt_info);
VOID Wtbl2RcpiGet(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid, union WTBL_2_DW13 *wtbl_2_d13);

VOID MtAsicTxCntUpdate(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, struct _MT_TX_COUNTER *pTxInfo);
VOID MtAsicRssiUpdate(struct _RTMP_ADAPTER *pAd);
VOID MtAsicRcpiReset(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid);
VOID MtAsicSetSMPS(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR smps);

#ifdef MCS_LUT_SUPPORT
VOID MtAsicMcsLutUpdate(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* MCS_LUT_SUPPORT */

VOID MtAsicUpdateBASession(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type);

VOID MtAsicUpdateRxWCIDTable(struct _RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr);
VOID MtAsicUpdateWcidAttributeEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR BssIdx,
	IN UCHAR KeyIdx,
	IN UCHAR CipherAlg,
	IN UINT8 Wcid,
	IN UINT8 KeyTabFlag);

VOID MtAsicDelWcidTab(struct _RTMP_ADAPTER *pAd, UCHAR wcid_idx);

VOID MtAsicAddPairwiseKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR WCID, struct _CIPHER_KEY *pKey);
VOID MtAsicRemovePairwiseKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);

INT MtAsicSendCommandToMcu(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);
BOOLEAN MtAsicSendCmdToMcuAndWait(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);
BOOLEAN MtAsicSendCommandToMcuBBP(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked);

VOID MtAsicTurnOffRFClk(struct _RTMP_ADAPTER *pAd, UCHAR Channel);

#ifdef WAPI_SUPPORT
VOID MtAsicUpdateWAPIPN(struct _RTMP_ADAPTER *pAd, USHORT WCID, ULONG pn_low, ULONG pn_high);
#endif /* WAPI_SUPPORT */




#ifdef STREAM_MODE_SUPPORT
UINT32 MtStreamModeRegVal(struct _RTMP_ADAPTER *pAd);
VOID MtAsicSetStreamMode(struct _RTMP_ADAPTER *pAd, UCHAR *mac, INT chainIdx, BOOLEAN enable);
VOID MtAsicStreamModeInit(struct _RTMP_ADAPTER *pAd);
#endif // STREAM_MODE_SUPPORT //

VOID MtAsicSetTxPreamble(struct _RTMP_ADAPTER *pAd, USHORT TxPreamble);

#ifdef DOT11_N_SUPPORT
INT MtAsicReadAggCnt(struct _RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len);
INT MtAsicSetRalinkBurstMode(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
#endif // DOT11_N_SUPPORT //

INT MtAsicWaitMacTxRxIdle(struct _RTMP_ADAPTER *pAd);


INT32 MtAsicSetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable);
INT MtAsicSetWPDMA(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable);
BOOLEAN MtAsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us);
INT MtAsicSetMacWD(struct _RTMP_ADAPTER *pAd);
INT MtStopDmaRx(struct _RTMP_ADAPTER *pAd, UCHAR Level);
INT MtStopDmaTx(struct _RTMP_ADAPTER *pAd, UCHAR Level);
INT MtAsicSetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNum);
INT MtAsicSetRxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums);
INT MtAsicSetBW(struct _RTMP_ADAPTER *pAd, INT bw);
INT MtAsicSetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 RxPathSel);


#ifdef CONFIG_ATE
INT MtAsicSetRfFreqOffset(struct _RTMP_ADAPTER *pAd, UINT32 FreqOffset);
INT MtAsicSetTSSI(struct _RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect);
INT MtAsicSetDPD(struct _RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect);
#ifdef CONFIG_QA
UINT32 MtAsicGetRxStat(struct _RTMP_ADAPTER *pAd, UINT type);
#endif /* CONFIG_QA */
INT MtAsicSetTxToneTest(struct _RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR Type);
INT MtAsicStartContinousTx(struct _RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW, UINT32 PriCh, UINT32 Mcs, UINT32 WFSel);
INT MtAsicStopContinousTx(struct _RTMP_ADAPTER *pAd);
#endif /* CONFIG_ATE */


#ifdef MAC_APCLI_SUPPORT
VOID MtAsicSetApCliBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index);
#endif /* MAC_APCLI_SUPPORT */

VOID MtAsicSetRxGroup(struct _RTMP_ADAPTER *pAd, UINT32 Port, UINT32 Group, BOOLEAN Enable);

#ifdef DMA_SCH_SUPPORT
INT32 MtAsicDMASchedulerInit(struct _RTMP_ADAPTER *pAd, INT mode);
#endif /* DMA_SCH_SUPPORT */

VOID MtAsicSetBARTxCntLimit(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count);
VOID MtAsicSetRTSTxCntLimit(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count);
VOID MtAsicSetTxSClassifyFilter(struct _RTMP_ADAPTER *pAd, UINT32 Port, UINT8 DestQ, UINT32 AggNums, UINT32 Filter);
VOID MtAsicSetRxPspollFilter(RTMP_ADAPTER *pAd, CHAR enable);

#define AC_QUEUE_STOP 0
#define AC_QUEUE_FLUSH 1
#define AC_QUEUE_START 2

#if defined(MT7603) || defined(MT7628)
INT32 MtAsicGetThemalSensor(struct _RTMP_ADAPTER *pAd, CHAR type);
VOID MtAsicACQueue(struct _RTMP_ADAPTER *pAd, UINT8 ucation, UINT8 BssidIdx, UINT32 u4AcQueueMap);
#endif /* MT7603 || MT7628  */

								
VOID MtAsicInitMac(struct _RTMP_ADAPTER *pAd);


#endif /* __CMM_ASIC_MT_H__ */

