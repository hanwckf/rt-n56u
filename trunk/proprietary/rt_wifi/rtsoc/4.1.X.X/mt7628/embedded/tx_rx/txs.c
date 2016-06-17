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

#include	"rt_config.h"

/**** TxS Call Back Functions ****/
#ifdef CFG_TDLS_SUPPORT
INT32 TdlsTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
	TXS_D_2 *txs_d2 = &txs_entry->txs_d2;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	TXS_D_4 *txs_d4 = &txs_entry->txs_d4;
	pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():txs d0 me : %d\n", __FUNCTION__, txs_d0->ME));
	if(txs_d0->ME == 0)
	{
		pEntry->TdlsTxFailCount=0;
	}
	else
	{	
		pEntry->TdlsTxFailCount++;
	}

	if(pEntry->TdlsTxFailCount > 15)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): TdlsTxFailCount > 15!!  teardown link with (%02X:%02X:%02X:%02X:%02X:%02X)!!\n"
			, __FUNCTION__,PRINT_MAC(pEntry->Addr)));
		pEntry->TdlsTxFailCount=0;
		cfg_tdls_auto_teardown(pAd,pEntry);
	}
}
#endif /*CFG_TDLS_SUPPORT*/

INT32 BcnTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{	
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;

#ifdef CONFIG_AP_SUPPORT
	if ((pAd->OpMode == OPMODE_AP) 
#ifdef RT_CFG80211_P2P_SUPPORT							
		|| (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
		)
	{
		TXS_D_4 *txs_d4 = &txs_entry->txs_d4;
		UCHAR bss_idx = 0;
		BSS_STRUCT *pMbss = NULL;
		bss_idx = txs_d4->pid - PID_BEACON;
		pMbss = &pAd->ApCfg.MBSSID[bss_idx];
		pMbss->bcn_buf.bcn_state = BCN_TX_IDLE;
		pMbss->bcn_not_idle_time = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("%s():idx: %x, change state as idle\n", __FUNCTION__, bss_idx));

		{
#ifdef DBG
			TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
			
			pMbss->TXS_TSF[pMbss->timer_loop] = txs_d1->timestamp;
			pMbss->TXS_SN[pMbss->timer_loop] = txs_d4->sn;
#endif /* DBG */
			if (pMbss->timer_loop < MAX_TIME_RECORD - 1)
				pMbss->timer_loop++;
			else
				pMbss->timer_loop = 0;
		}

	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA && (pAd->StaCfg.BssType == BSS_ADHOC)) 
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("got TXS, MakeIbssBeacon\n"));
		MakeIbssBeacon(pAd);
	}
#endif /* CONFIG_STA_SUPPORT */

	return 0;
}

#ifdef CONFIG_STA_SUPPORT
/*7636 psm*/
INT32 NullFrameTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{	
#if defined(MT7636) || defined(MT7628)
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
    TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_4 *txs_d4 = &txs_entry->txs_d4;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, line(%d)\n", __FUNCTION__, __LINE__));

	if ((pAd->OpMode == OPMODE_STA) && 
        (pAd->StaCfg.BssType == BSS_INFRA) && 
        (pAd->StaCfg.WindowsPowerMode != Ndis802_11PowerModeCAM))
	{	
		if (txs_d4->pid == PID_NULL_FRAME_PWR_SAVE) 
		{
		    if ((txs_d0->LE == 0) && (txs_d0->RE == 0) && (txs_d0->ME == 0))
    		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("got TXS, RTMPSendNullFrame(PM=1)\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d)::Enter RTMPOffloadPm(pAd, 0x04, 1);\n", __FUNCTION__, __LINE__));

			RTEnqueueInternalCmd(pAd, CMDTHREAD_FORCE_SLEEP_AUTO_WAKEUP, NULL, 0);
		}
    		else
    		{
    			RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
    			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Got TXS, ERROR, Peer didn't get NullFrame(PM=1), LE(%d, RE(%d), ME(%d))\n",
    										txs_d0->LE,
    										txs_d0->RE,
    										txs_d0->ME));
    		}
		}
		else if (txs_d4->pid == PID_NULL_FRAME_PWR_ACTIVE) 
		{
			if ((txs_d0->LE == 0) && (txs_d0->RE == 0) && (txs_d0->ME == 0))
    		{
    			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Got TXS, RTMPSendNullFrame(PM=0)\n"));
    			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d)::Enter RTMPOffloadPm(pAd, 0x04, 0);\n", __FUNCTION__, __LINE__));
    		}
    		else
    		{
    			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Got TXS, ERROR, Peer didn't get NullFrame(PM=0)\n"));
		}	
	}
	}
#endif /* defined(MT7636) || defined(MT7628) */		
	return 0;
}
#endif /* CONFIG_STA_SUPPORT */

INT32 PsDataTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;

        if ((txs_d3 == NULL) || (txs_d3->wlan_idx >= MAX_LEN_OF_TR_TABLE))
        {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("---->%s INVALID_TR_WCID(WlanIndex)\n", __FUNCTION__));        	
                return 0;
        }

	if (pAd->MacTab.tr_entry[txs_d3->wlan_idx].PsDeQWaitCnt) {
		/* After a successfull Tx of dequeued PS data, we clear PsDeQWaitCnt */
		pAd->MacTab.tr_entry[txs_d3->wlan_idx].PsDeQWaitCnt = 0;
	}

	return 0;
}



#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)
INT32 EapReqIdTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	MAC_TABLE_ENTRY *pEntry;
 
	if ((txs_d3 == NULL) || (txs_d3->wlan_idx >= MAX_LEN_OF_TR_TABLE))
	{      	
		return 0;
	}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR | DBG_FUNC_PS, ("%s: (RE=%d, LE=%d, ME=%d), wlan_idx = %d\n",__FUNCTION__, txs_d0->RE, txs_d0->LE, txs_d0->ME, txs_d3->wlan_idx));        	
	if ((txs_d0->RE == 0) && (txs_d0->LE == 0) && (txs_d0->ME == 0))
	{
		pEntry = &pAd->MacTab.Content[txs_d3->wlan_idx];
		if (pEntry->bEapReqIdRetryTimerRunning)
		{
			pEntry->bEapReqIdRetryTimerRunning = FALSE;
		}
	}

	return 0;
}
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT) */

/**** End of TxS Call Back Functions ****/


INT32 InitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	
	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++)
	{
		NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypeLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypeLock[Index], &Flags);
		DlListInit(&TxSCtl->TxSType[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Index], &Flags);
	}

	return 0;
}


INT32 InitTxSCommonCallBack(RTMP_ADAPTER *pAd)
{

	/* PsDataTxSHandler */
	AddTxSType(pAd, PID_PS_DATA, TXS_FORMAT0, PsDataTxSHandler, FALSE); 
	TxSTypeCtl(pAd, PID_PS_DATA, TXS_FORMAT0, FALSE, TRUE);
#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)	
	AddTxSType(pAd, PID_WSC_EAP, TXS_FORMAT0, EapReqIdTxSHandler, FALSE); 
	TxSTypeCtl(pAd, PID_WSC_EAP, TXS_FORMAT0, FALSE, TRUE);
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)	 */


	return 0;
}


INT32 ExitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypeLock[Index], &Flags);
		DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSType[Index], 
							TXS_TYPE, List) 
		{
			DlListDel(&TxSType->List);
			os_free_mem(NULL, TxSType);
		}
		DlListInit(&TxSCtl->TxSType[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Index], &Flags);
		NdisFreeSpinLock(&TxSCtl->TxSTypeLock[Index]);
	}
	
	return 0;
}


INT32 AddTxSType(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, 
					TXS_HANDLER TxSHandler, BOOLEAN DumpTxSReport)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *SearchTxSType = NULL;
	
	os_alloc_mem(NULL, (PUCHAR *)&TxSType, sizeof(*TxSType));

	if (!TxSType) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("can not allocate TxS Type\n"));
		return -1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(SearchTxSType, &TxSCtl->TxSType[Pid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List) 
	{
		if ((SearchTxSType->Pid == Pid) && (SearchTxSType->Format == Format))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: already registered TxSType (PID = %d, Format = %d\n",
									 __FUNCTION__, Pid, Format));
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
			os_free_mem(NULL, TxSType);
			return -1;
		}
	}

	TxSType->Pid = Pid;
	TxSType->Format = Format;
	TxSType->TxSHandler = TxSHandler;
	TxSType->DumpTxSReport = DumpTxSReport;
	DlListAddTail(&TxSCtl->TxSType[Pid % TOTAL_PID_HASH_NUMS], &TxSType->List);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);

	return 0;
}


INT32 RemoveTxSType(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSType[Pid % TOTAL_PID_HASH_NUMS], 
						TXS_TYPE, List) 
	{
		if ((TxSType->Pid == Pid) && (TxSType->Format == Format))
		{
			DlListDel(&TxSType->List);
			os_free_mem(NULL, TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);

	return -1;
}


INT32 TxSTypeCtl(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, 
						BOOLEAN TxS2Mcu, BOOLEAN TxS2Host)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSType[Pid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List) 
	{
		if ((TxSType->Pid == Pid) && (TxSType->Format == Format))
		{
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatus |= (1 << Pid);
			else
				TxSCtl->TxS2McUStatus &= ~(1 << Pid);

			if (TxS2Host)
				TxSCtl->TxS2HostStatus |= (1 << Pid);
			else
				TxSCtl->TxS2HostStatus &= ~(1 << Pid);

			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormat |= (1 << Pid);
			else
				TxSCtl->TxSFormat &= ~(1 << Pid);	 
		
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: can not find TxSType(PID = %d, Format = %d)\n", 
								__FUNCTION__, Pid, Format));
	return -1;
}


static VOID DumpTxSFormat(RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data)
{ 
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->txs_d0;
	TXS_D_1 *txs_d1 = &txs_entry->txs_d1;
	TXS_D_2 *txs_d2 = &txs_entry->txs_d2;
	TXS_D_3 *txs_d3 = &txs_entry->txs_d3;
	TXS_D_4 *txs_d4 = &txs_entry->txs_d4;

	if (Format == TXS_FORMAT0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tType=TimeStamp/FrontTime Mode(TXSFM=%d, \
									TXS2M=%d, TXS2H=%d)\n", txs_d0->txsfm, txs_d0->txs2m, txs_d0->txs2h));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFR=%d, TxRate=0x%x\n", txs_d0->fr, txs_d0->tx_rate));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tME=%d, RE=%d, LE=%d, BE=%d, TxOPLimitErr=%d\n",\
                                    txs_d0->ME, txs_d0->RE, txs_d0->LE, txs_d0->BE, txs_d0->txop));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPS=%d, BA Fail=%d, tid=%d, Ant_Id=%d\n",\
                                    txs_d0->ps, txs_d0->baf, txs_d0->tid, txs_d0->ant_id));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTimeStamp=0x%x, FrontTime=0x%x, TxPwr=0x%x\n",\
                                    txs_d1->timestamp, txs_d2->field_ft.front_time, txs_d2->field_ft.tx_pwr_dBm));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxDelay=0x%x, RxVSeqNum=0x%x, Wlan Idx=0x%x\n",\
                                    txs_d3->transmission_delay, txs_d3->rxv_sn, txs_d3->wlan_idx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSN=0x%x, TxBW=0x%x, AMPDU=%d, PID=0x%x, MPDU TxCnt=%d, MCS Idx=%d\n",
                                    txs_d4->sn, txs_d4->tbw, txs_d4->am, txs_d4->pid, txs_d4->mpdu_tx_cnt, txs_d4->last_tx_rate_idx));

	}
	else if (Format == TXS_FORMAT1)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tType=Noisy/RCPI Mode\n"));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown TxSFormat(%d)\n", __FUNCTION__, Format)); 
	}
}
 

INT32 ParseTxSPacket(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSType[Pid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List) 
	{
		if (TxSType->Pid == Pid && TxSType->Format == Format)
		{
			if (TxSType->DumpTxSReport)
			{
				DumpTxSFormat(pAd, Format, Data);
			}

			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);
			TxSType->TxSHandler(pAd, Data);	
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypeLock[Pid % TOTAL_PID_HASH_NUMS], &Flags);

	return -1;
} 


