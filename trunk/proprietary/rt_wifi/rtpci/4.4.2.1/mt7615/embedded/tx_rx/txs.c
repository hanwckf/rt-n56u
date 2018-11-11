/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	txs.c
*/

/**
 * @addtogroup tx_rx_path Wi-Fi
 * @{
 * @name TxS Control API
 * @{
 */

#include	"rt_config.h"

/**** TxS Call Back Functions ****/
#ifdef CFG_TDLS_SUPPORT
INT32 TdlsTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	TXS_D_4 *TxSD4 = &txs_entry->TxSD4;
	pEntry = &pAd->MacTab.Content[TxSD3->TxS_WlanIdx];
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():txs d0 me : %d\n", __FUNCTION__, TxSD0->ME));
	if(TxSD0->ME == 0)
	{
		pEntry->TdlsTxFailCount=0;
	}
	else
	{
		pEntry->TdlsTxFailCount++;
	}

	if(pEntry->TdlsTxFailCount > 15)
	{
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): TdlsTxFailCount > 15!!  teardown link with (%02X:%02X:%02X:%02X:%02X:%02X)!!\n"
			, __FUNCTION__,PRINT_MAC(pEntry->Addr)));
		pEntry->TdlsTxFailCount=0;
		cfg_tdls_auto_teardown(pAd,pEntry);
	}
}
#endif /*CFG_TDLS_SUPPORT*/


INT32 ActionTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, fix me!
	return 0;

#else
	MAC_TABLE_ENTRY *pEntry = NULL;

	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->TxSD0;
	TXS_D_3 *txs_d3 = &txs_entry->TxSD3;


	pEntry = &pAd->MacTab.Content[txs_d3->TxS_WlanIdx];
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():txs d0 me : %d\n", __FUNCTION__, txs_d0->ME));
	if(txs_d0->ME == 0)
	{
#ifdef RT_CFG80211_SUPPORT
		/* TX completed */
		CFG80211_SendMgmtFrameDone(pAd, 0, TRUE);
#endif /* RT_CFG80211_SUPPORT */

		// TODO: Driver proprietary P2P condition @20140721

	}
	else
	{
#ifdef RT_CFG80211_SUPPORT
		CFG80211_SendMgmtFrameDone(pAd, 0, FALSE);
#endif /* RT_CFG80211_SUPPORT */

		// TODO: Driver proprietary P2P condition @20140721
	}

	return 0;
#endif
}

INT32 BcnTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, fix me!
	return 0;

#else
#ifdef CONFIG_AP_SUPPORT
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
#ifdef DBG
	//TXS_D_0 *txs_d0 = &txs_entry->TxSD0;
	TXS_D_1 *txs_d1 = &txs_entry->TxSD1;
	//TXS_D_2 *txs_d2 = &txs_entry->TxSD2;
	//TXS_D_3 *txs_d3 = &txs_entry->TxSD3;
	TXS_D_4 *txs_d4 = &txs_entry->TxSD4;
#endif

	if ((pAd->OpMode == OPMODE_AP)
#ifdef RT_CFG80211_P2P_SUPPORT
		|| (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
		)
		{
			UCHAR bss_idx = 0;
			BSS_STRUCT *pMbss = NULL;
            struct wifi_dev *wdev = NULL;
			bss_idx = Priv;
			pMbss = &pAd->ApCfg.MBSSID[bss_idx];
            wdev = &pMbss->wdev;
            RTMP_SEM_LOCK(&pAd->BcnRingLock);
            wdev->bcn_buf.bcn_state = BCN_TX_IDLE;
            pMbss->bcn_not_idle_time = 0;
            MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                    ("%s():idx: %x, change state as idle\n", __FUNCTION__, bss_idx));
            RTMP_SEM_UNLOCK(&pAd->BcnRingLock);

#ifdef DBG
			pMbss->TXS_TSF[pMbss->timer_loop] = txs_d1->TimeStamp;
			pMbss->TXS_SN[pMbss->timer_loop] = txs_d4->TxS_SN_TSSI;
			pMbss->timer_loop++;
			if (pMbss->timer_loop >= MAX_TIME_RECORD)
				pMbss->timer_loop = 0;
#endif /* DBG */
		}
#endif /* CONFIG_AP_SUPPORT */

    return 0;
#endif /* defined(MT7615) || defined(MT7622) */
}

INT32 NullFramePM1TxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}

INT32 NullFramePM0TxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}

INT32 PsDataTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, fix me!
	return 0;

#else
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;

	if (pAd->MacTab.tr_entry[TxSD3->TxS_WlanIdx].PsDeQWaitCnt) {
		/* After a successfull Tx of dequeued PS data, we clear PsDeQWaitCnt */
		pAd->MacTab.tr_entry[TxSD3->TxS_WlanIdx].PsDeQWaitCnt = 0;
	}
#endif /* defined(MT7615) || defined(MT7622) */

	return 0;
}

#if !defined(MT7615) && !defined(MT7622)
static INT32 MgmtTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}


static INT32 CtrlTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}


static INT32 DataTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{

	return 0;
}
#endif /* !defined(MT7615) && !defined(MT7622) */

/**** End of TxS Call Back Functions ****/


INT32 InitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s %d\n",__FUNCTION__,__LINE__));

	/* Per Pkt */
	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++)
	{
		NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
	}

	/* Per Pkt Type */
	for (Index = 0; Index < 3; Index++)
	{
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++)
		{
			NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
		}
	}

	for (Index = 0; Index < TXS_STATUS_NUM; Index++)
	{
		NdisZeroMemory(&TxSCtl->TxSStatus[Index], sizeof(TXS_STATUS));
	}

	return 0;
}

/*7636 psm*/
INT32 NullFrameTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	return 0;
}


INT32 InitTxSCommonCallBack(RTMP_ADAPTER *pAd)
{
#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, fix me!
	return 0;
#else
	AddTxSTypePerPktType(pAd, FC_TYPE_MGMT, SUBTYPE_BEACON, TXS_FORMAT0, BcnTxSHandler);
	TxSTypeCtlPerPktType(pAd, FC_TYPE_MGMT, SUBTYPE_BEACON, TXS_WLAN_IDX_ALL,
							TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
	AddTxSTypePerPktType(pAd, FC_TYPE_MGMT, SUBTYPE_ALL, TXS_FORMAT0, MgmtTxSHandler);
	AddTxSTypePerPktType(pAd, FC_TYPE_CNTL, SUBTYPE_ALL, TXS_FORMAT0, CtrlTxSHandler);
	AddTxSTypePerPktType(pAd, FC_TYPE_DATA, SUBTYPE_ALL, TXS_FORMAT0, DataTxSHandler);

	/* PsDataTxSHandler */
	AddTxSTypePerPkt(pAd, PID_PS_DATA, TXS_FORMAT0, PsDataTxSHandler);
	TxSTypeCtlPerPkt(pAd, PID_PS_DATA, TXS_FORMAT0, FALSE, TRUE, FALSE, 0);
#endif /* defined(MT7615) || defined(MT7622) */
	return 0;
}


INT32 ExitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[Index],
							TXS_TYPE, List)
		{
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
		}
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		NdisFreeSpinLock(&TxSCtl->TxSTypePerPktLock[Index]);
	}

	for (Index = 0; Index < 3; Index++)
	{
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++)
		{
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[Index][Index1],
								TXS_TYPE, List)
			{
				DlListDel(&TxSType->List);
				os_free_mem(TxSType);
			}
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			NdisFreeSpinLock(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
		}
	}

	return 0;
}


INT32 AddTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format,
						TXS_HANDLER TxSHandler)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *SearchTxSType = NULL;

#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, fix me!
	return 0;
#endif /* defined(MT7615) || defined(MT7622) */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d \n", __FUNCTION__, __LINE__));
	os_alloc_mem(NULL, (PUCHAR *)&TxSType, sizeof(*TxSType));

	if (!TxSType) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("can not allocate TxS Type\n"));
		return -1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(SearchTxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS],
										TXS_TYPE, List)
	{
		if ((SearchTxSType->PktPid == PktPid) && (SearchTxSType->Format == Format))
		{
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("%s: already registered TxSType (PktPid = %d, Format = %d\n",
							 __FUNCTION__, PktPid, Format));
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
										&Flags);
			os_free_mem(TxSType);
			return -1;
		}
	}

	TxSType->Type = TXS_TYPE0;
	TxSType->PktPid = PktPid;
	TxSType->Format = Format;
	TxSType->TxSHandler = TxSHandler;

	DlListAddTail(&TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], &TxSType->List);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);

	return 0;
}


INT32 RemoveTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS],
						TXS_TYPE, List)
	{
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format))
		{
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
											&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);

	return -1;
}


INT32 TxSTypeCtlPerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format, BOOLEAN TxS2Mcu,
						BOOLEAN TxS2Host, BOOLEAN DumpTxSReport, ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List)
	{
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format))
		{
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2McUStatusPerPkt &= ~(1 << PktPid);

			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2HostStatusPerPkt &= ~(1 << PktPid);

			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxSFormatPerPkt &= ~(1 << PktPid);

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;

			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
											&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF ,
								 ("%s: can not find TxSType(PktPID = %d, Format = %d)\n",
								__FUNCTION__, PktPid, Format));
	return -1;
}


INT32 AddTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType,
							UINT8 Format, TXS_HANDLER TxSHandler)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *SearchTxSType = NULL;

	os_alloc_mem(NULL, (PUCHAR *)&TxSType, sizeof(*TxSType));

	if (!TxSType) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("can not allocate TxS Type\n"));
		return -1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(SearchTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List)
	{
		if ((SearchTxSType->PktType == PktType) && (SearchTxSType->PktSubType == PktSubType)
					&& (SearchTxSType->Format == Format))
		{
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: already registered TxSType (PktType = %d, PktSubType = %d, Format = %d\n", __FUNCTION__, PktType, PktSubType, Format));
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			os_free_mem(TxSType);
			return -1;
		}
	}

	TxSType->Type = TXS_TYPE1;
	TxSType->PktType = PktType;
	TxSType->PktSubType = PktSubType;
	TxSType->Format = Format;
	TxSType->TxSHandler = TxSHandler;
	DlListAddTail(&TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &TxSType->List);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);

	return 0;
}


INT32 RemoveTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType,
										UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List)
	{
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
					&& (TxSType->Format == Format))
		{
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);

	return -1;
}


INT32 TxSTypeCtlPerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType, UINT16 WlanIdx,
							UINT8 Format, BOOLEAN TxS2Mcu, BOOLEAN TxS2Host, BOOLEAN DumpTxSReport,
							ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List)
	{
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
				&& (TxSType->Format == Format))
		{
            /*register the TYPE/SUB_TYPE need to report to MCU.*/
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2McUStatusPerPktType[PktType] &= ~(1 << PktSubType);

            /*register the TYPE/SUB_TYPE need to report to HOST.*/
			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2HostStatusPerPktType[PktType] &= ~(1 << PktSubType);

            /*register the TXS report type*/
			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxSFormatPerPktType[PktType] &= ~(1 << PktSubType);

            /*indicate which widx might be used for send the kinw of type/subtype pkt.*/
			if (WlanIdx < 64)
			{
				TxSCtl->TxSStatusPerWlanIdx[0] |= (1 << WlanIdx);
			}
			else if (WlanIdx >= 64 && WlanIdx < 128)
			{
				TxSCtl->TxSStatusPerWlanIdx[1] |= (1 << WlanIdx);
			}
			else
			{
				TxSCtl->TxSStatusPerWlanIdx[0] = 0xffffffffffffffffLL;
				TxSCtl->TxSStatusPerWlanIdx[1] = 0xffffffffffffffffLL;
			}

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;

			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s: can not find TxSType(PktType = %d, PktSubType = %d, Format = %d)\n",
						__FUNCTION__, PktType, PktSubType, Format));
	return -1;
}


INT32 ParseTxSPacket(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;
	UINT8 Type, PktPid, PktType, PktSubType;
	UINT16 TxRate;
	UINT32 Priv;

#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, fix me!
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
		TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
#ifdef WH_EZ_SETUP
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	BOOLEAN TxError = (TxSD0->ME || TxSD0->RE || TxSD0->LE || TxSD0->BE || TxSD0->TxOp || TxSD0->PSBit || TxSD0->BAFail);
#endif

	if (Format == TXS_FORMAT0)
	{
#ifdef WH_EZ_SETUP
			if ((Pid == PID_P2P_ACTION))
			{
				MAC_TABLE_ENTRY	*pEntry = NULL;
				struct wifi_dev *wdev = NULL;
				pEntry = &pAd->MacTab.Content[TxSD2->TxS_WlanIdx];
				wdev = pEntry->wdev;
				if(IS_EZ_SETUP_ENABLED(wdev)){
					EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Received TXS: Wcid=%d\n", TxSD2->TxS_WlanIdx));
					if(TxError && IS_EZ_SETUP_ENABLED(wdev)){
						ez_handle_action_txstatus(pAd, TxSD2->TxS_WlanIdx);
					}
				}
			}		
#endif

			if (TxSD0->ME || TxSD0->RE || TxSD0->LE || TxSD0->BE || TxSD0->TxOp || TxSD0->PSBit || TxSD0->BAFail)
	{
				DumpTxSFormat(pAd, Format, Data);
				return -1;
	}
}
		return 0;
	}
#endif /* defined(MT7615) || defined(MT7622) */

	RemoveTxSStatus(pAd, Pid, &Type, &PktPid, &PktType, &PktSubType, &TxRate, &Priv);

	if (Type == TXS_TYPE0)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
		DlListForEach(TxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List)
		{
			if (TxSType->PktPid == PktPid && TxSType->Format == Format)
			{
				if (TxSType->DumpTxSReport)
				{
					if (TxSType->DumpTxSReportTimes > 0 || TxSType->DumpTxSReportTimes == TXS_DUMP_REPEAT)
					{
						MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\tPktPid = 0x%x, Orignal TxRate = 0x%x, Priv = 0x%x\n",
												PktPid, TxRate, Priv));
						DumpTxSFormat(pAd, Format, Data);

						if (TxSType->DumpTxSReportTimes != TXS_DUMP_REPEAT)
							TxSType->DumpTxSReportTimes--;
					}
				}

				TxSType->TxSHandler(pAd, Data, Priv);
				RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
														&Flags);
				return 0;
			}
		}
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	}
	else if (Type == TXS_TYPE1)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
		DlListForEach(TxSType,
			&TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List)
		{
			if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
					&& (TxSType->Format == Format))
			{
				if (TxSType->DumpTxSReport)
				{
					if (TxSType->DumpTxSReportTimes > 0 || TxSType->DumpTxSReportTimes
											== TXS_DUMP_REPEAT)
					{
						MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPktType = 0x%x, PktSubType = 0x%x, Orignal TxRate = 0x%x, Priv = 0x%x\n", PktType, PktSubType, TxRate, Priv));
						DumpTxSFormat(pAd, Format, Data);

						if (TxSType->DumpTxSReportTimes != TXS_DUMP_REPEAT)
							TxSType->DumpTxSReportTimes--;
					}
				}

				RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
				TxSType->TxSHandler(pAd, Data, Priv);
				return 0;
			}
		}
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	}

	return -1;
}


UINT8 AddTxSStatus(RTMP_ADAPTER *pAd, UINT8 Type, UINT8 PktPid, UINT8 PktType,
						UINT8 PktSubType, UINT16 TxRate, UINT32 Priv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	INT idx;

	for (idx = 0; idx < TXS_STATUS_NUM; idx++)
	{
		if (TxSCtl->TxSStatus[idx].State == TXS_UNUSED)
		{
			TxSCtl->TxSPid = idx;
			TxSCtl->TxSStatus[idx].TxSPid = TxSCtl->TxSPid;
			TxSCtl->TxSStatus[idx].State = TXS_USED;
			TxSCtl->TxSStatus[idx].Type = Type;
			TxSCtl->TxSStatus[idx].PktPid = PktPid;
			TxSCtl->TxSStatus[idx].PktType = PktType;
			TxSCtl->TxSStatus[idx].PktSubType = PktSubType;
			TxSCtl->TxSStatus[idx].TxRate = TxRate;
			TxSCtl->TxSStatus[idx].Priv = Priv;
			break;
		}
	}

	if (idx >= TXS_STATUS_NUM)
	{
		TxSCtl->TxSFailCount++;
		idx = TXS_STATUS_NUM - 1;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s():Cannot get empty TxSPid, use default(%d)\n",
			__FUNCTION__, idx));
	}
	return idx;
}


INT32 RemoveTxSStatus(RTMP_ADAPTER *pAd, UINT8 TxSPid, UINT8 *Type, UINT8 *PktPid,
								UINT8 *PktType, UINT8 *PktSubType, UINT16 *TxRate, UINT32 *TxSPriv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;

	*Type = TxSCtl->TxSStatus[TxSPid].Type;
	*PktPid = TxSCtl->TxSStatus[TxSPid].PktPid;
	*PktType = TxSCtl->TxSStatus[TxSPid].PktType;
	*PktSubType = TxSCtl->TxSStatus[TxSPid].PktSubType;
	*TxRate = TxSCtl->TxSStatus[TxSPid].TxRate;
	*TxSPriv = TxSCtl->TxSStatus[TxSPid].Priv;

	TxSCtl->TxSStatus[TxSPid].State = TXS_UNUSED;

    return 0;
}

/** @} */
/** @} */
