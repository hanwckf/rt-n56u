/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include <rt_config.h>


#ifdef DBG
VOID dump_tr_entry(RTMP_ADAPTER *pAd, INT tr_idx, const RTMP_STRING *caller, INT line)
{
	STA_TR_ENTRY *tr_entry;
	INT qidx;

	ASSERT(tr_idx < MAX_LEN_OF_TR_TABLE);
	if (tr_idx >= MAX_LEN_OF_TR_TABLE)
		return;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump TR_ENTRY called by function %s(%d)\n", caller, line));
	tr_entry = &pAd->MacTab.tr_entry[tr_idx];
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TR_ENTRY[%d]\n", tr_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tEntryType=%x\n", tr_entry->EntryType));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twdev=%p\n", tr_entry->wdev));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twcid=%d\n", tr_entry->wcid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tfunc_tb_idx=%d\n", tr_entry->func_tb_idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAddr=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tr_entry->Addr)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBSSID=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tr_entry->bssid)));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFlags\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbIAmBadAtheros=%d, isCached=%d, PortSecured=%d, PsMode=%d, LockEntryTx=%d\n",
				tr_entry->bIAmBadAtheros, tr_entry->isCached, tr_entry->PortSecured, tr_entry->PsMode, tr_entry->LockEntryTx));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxRx Characters\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNonQosDataSeq=%d\n", tr_entry->NonQosDataSeq));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxSeq[0]=%d, TxSeq[1]=%d, TxSeq[2]=%d, TxSeq[3]=%d\n",
				tr_entry->TxSeq[0], tr_entry->TxSeq[1], tr_entry->TxSeq[2], tr_entry->TxSeq[3]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCurrTxRate=%x\n", tr_entry->CurrTxRate));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tQueuing Info\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tenq_cap=%d, deq_cap=%d\n", tr_entry->enq_cap, tr_entry->deq_cap));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tQueuedPkt: TxQ[0]=%d, TxQ[1]=%d, TxQ[2]=%d, TxQ[3]=%d, PSQ=%d\n",
							tr_entry->tx_queue[0].Number, tr_entry->tx_queue[1].Number,
							tr_entry->tx_queue[2].Number, tr_entry->tx_queue[3].Number,
							tr_entry->ps_queue.Number));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tdeq_cnt=%d, deq_bytes=%d\n", tr_entry->deq_cnt, tr_entry->deq_bytes));

	for (qidx = 0 ; qidx < 4; qidx++) {
		rtmp_sta_txq_dump(pAd, tr_entry, qidx);
		rtmp_tx_swq_dump(pAd, qidx);
	}
}
#endif /* DBG */

VOID tr_tb_reset_entry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx;

	if (tr_tb_idx >= MAX_LEN_OF_TR_TABLE)
		return;

	tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];
	if (IS_ENTRY_NONE(tr_entry))
		return;

	tr_entry->enq_cap = FALSE;
	tr_entry->deq_cap = FALSE;

#ifdef DATA_QUEUE_RESERVE
	tr_entry->high_pkt_cnt = 0;
	tr_entry->high_pkt_drop_cnt = 0;
#endif /* DATA_QUEUE_RESERVE */

	rtmp_tx_swq_exit(pAd, tr_tb_idx);

	SET_ENTRY_NONE(tr_entry);

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++)
		NdisFreeSpinLock(&tr_entry->txq_lock[qidx]);
	NdisFreeSpinLock(&tr_entry->ps_queue_lock);

	return;
}


VOID tr_tb_set_entry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, MAC_TABLE_ENTRY *pEntry)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx, tid,upId;
    //MAC_TABLE_ENTRY *mac_entry;
    struct wtbl_entry tb_entry;

	if (tr_tb_idx < MAX_LEN_OF_TR_TABLE)
	{
		tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];

		tr_entry->EntryType = pEntry->EntryType;
		tr_entry->wdev = pEntry->wdev;
		tr_entry->func_tb_idx = pEntry->func_tb_idx;

		tr_entry->wcid = pEntry->wcid;
		NdisMoveMemory(tr_entry->Addr, pEntry->Addr, MAC_ADDR_LEN);

		tr_entry->NonQosDataSeq = 0;
		for (tid = 0; tid < NUM_OF_TID; tid++)
			tr_entry->TxSeq[tid] = 0;


		for(upId = 0 ; upId < NUM_OF_UP ; upId ++)
		{
			tr_entry->cacheSn[upId] = -1;
		}

        /* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
        NdisZeroMemory(&tb_entry, sizeof(tb_entry));

#ifdef MT_MAC
        if (mt_wtbl_get_entry234(pAd, tr_entry->wcid, &tb_entry) == FALSE) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Not found WTBL2/3/4 for tr_entry->wcid(%d), !ERROR!\n",
                        __FUNCTION__, tr_entry->wcid));
            return;
        }

        //mac_entry = &pAd->MacTab.Content[tr_entry->wcid];
#endif /* MT_MAC */

		tr_entry->PsMode = PWR_ACTIVE;
		tr_entry->isCached = FALSE;
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		tr_entry->CurrTxRate = pEntry->CurrTxRate;

		for (qidx = 0 ; qidx < WMM_QUE_NUM; qidx++) {
			InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
            tr_entry->TotalPageCount[qidx] = 0;
#endif
#endif
			NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
		}
		InitializeQueueHeader(&tr_entry->ps_queue);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);

		tr_entry->deq_cnt = 0;
		tr_entry->deq_bytes = 0;
		tr_entry->PsQIdleCount = 0;
		tr_entry->enq_cap = TRUE;
		tr_entry->deq_cap = TRUE;
		NdisMoveMemory(tr_entry->bssid, pEntry->wdev->bssid, MAC_ADDR_LEN);
	}
}


VOID tr_tb_set_mcast_entry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, struct wifi_dev *wdev)
{
	struct _STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];
	INT qidx, tid;

	NdisZeroMemory(tr_entry, sizeof(struct _STA_TR_ENTRY));

	tr_entry->EntryType = ENTRY_CAT_MCAST;
	tr_entry->wdev = wdev;
	tr_entry->func_tb_idx = wdev->func_idx;
	tr_entry->PsMode = PWR_ACTIVE;
	tr_entry->isCached = FALSE;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	tr_entry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	NdisMoveMemory(tr_entry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);

	// TODO: shiang-usw, for following fields, need better assignment!
	tr_entry->wcid = tr_tb_idx;
	tr_entry->NonQosDataSeq = 0;
	for (tid = 0; tid < NUM_OF_TID; tid++)
		tr_entry->TxSeq[tid] = 0;

	for (qidx = 0 ; qidx < WMM_QUE_NUM; qidx++) {
		InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
		NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
	}
	InitializeQueueHeader(&tr_entry->ps_queue);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
	tr_entry->deq_cnt = 0;
	tr_entry->deq_bytes = 0;
	tr_entry->PsQIdleCount = 0;
	tr_entry->enq_cap = TRUE;
	tr_entry->deq_cap = TRUE;
#ifdef DATA_QUEUE_RESERVE
	tr_entry->high_pkt_cnt = 0;
	tr_entry->high_pkt_drop_cnt = 0;
#endif /* DATA_QUEUE_RESERVE */
	NdisMoveMemory(tr_entry->bssid, wdev->bssid, MAC_ADDR_LEN);

}


// TODO: this function not finish yet!!
VOID mgmt_tb_set_mcast_entry(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];

	pEntry->EntryType = ENTRY_CAT_MCAST;
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index*/
	pEntry->wcid = MCAST_WCID;
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;

	pEntry->Addr[0] = 0x01;
	pEntry->HTPhyMode.field.MODE = MODE_OFDM;
	pEntry->HTPhyMode.field.MCS = 3;

	NdisMoveMemory(pEntry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
		pEntry->wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd){
		pEntry->wdev = &pAd->StaCfg.wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID set_entry_phy_cfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{

	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	}
	else
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}
}


VOID set_sta_ra_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, ULONG ra_ie)
{
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);

	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);

	/* Set cap flags */
	if (ra_ie != 0x0) {
		CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);

#ifdef AGGREGATION_SUPPORT
		if (ra_ie & 0x00000001)
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
#endif /* AGGREGATION_SUPPORT */

#ifdef PIGGYBACK_SUPPORT
		if (ra_ie & 0x00000002)
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
#endif /* PIGGYBACK_SUPPORT */
	}

	/* Set operation status */
	if (ra_ie != 0x0)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);

#ifdef AGGREGATION_SUPPORT
	if ((pAd->CommonCfg.bAggregationCapable) && (ra_ie & 0x00000001))
	{
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RaAggregate= 1\n"));
	}
#endif /* AGGREGATION_SUPPORT */

#ifdef PIGGYBACK_SUPPORT
	if ((pAd->CommonCfg.bPiggyBackCapable) && (ra_ie & 0x00000002))
	{
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PiggyBack= 1\n"));
	}
#endif /* PIGGYBACK_SUPPORT */

}


/*
	==========================================================================
	Description:
		Look up the MAC address in the MAC table. Return NULL if not found.
	Return:
		pEntry - pointer to the MAC entry; NULL is not found
	==========================================================================
*/
MAC_TABLE_ENTRY *MacTableLookup(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry))
	{
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	return pEntry;
}


#ifdef CONFIG_STA_SUPPORT
BOOLEAN StaUpdateMacTableEntry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN HT_CAPABILITY_IE *ht_cap,
	IN UCHAR htcap_len,
	IN ADD_HT_INFO_IE *pAddHtInfo,
	IN UCHAR AddHtInfoLen,
	IN IE_LISTS *ie_list,
	IN USHORT cap_info)
{
	UCHAR MaxSupportedRate = RATE_11;
	BOOLEAN bSupportN = FALSE, bDoTxRateSwitch = FALSE;
	struct wifi_dev *wdev;
	STA_TR_ENTRY *tr_entry;

	if (!pEntry)
		return FALSE;

	if (ADHOC_ON(pAd))
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

	if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_G)
	    && (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
		return FALSE;

#ifdef DOT11_N_SUPPORT
	/* 11n only */
	if (WMODE_HT_ONLY(pAd->CommonCfg.PhyMode)
	    && (htcap_len == 0))
		return FALSE;
#endif /* DOT11_N_SUPPORT */

	NdisAcquireSpinLock(&pAd->MacTabLock);

	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

	NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	if ((MaxSupportedRate < RATE_FIRST_OFDM_RATE) ||
	    WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B)) {
		pEntry->RateLen = 4;
		if (MaxSupportedRate >= RATE_FIRST_OFDM_RATE)
			MaxSupportedRate = RATE_11;
	} else
		pEntry->RateLen = 12;

	pEntry->MaxHTPhyMode.word = 0;
	pEntry->MinHTPhyMode.word = 0;
	pEntry->HTPhyMode.word = 0;
	pEntry->MaxSupportedRate = MaxSupportedRate;

	set_entry_phy_cfg(pAd, pEntry);

	pEntry->CapabilityInfo = cap_info;
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);


	wdev = &pAd->StaCfg.wdev;
#ifdef DOT11_N_SUPPORT
	NdisZeroMemory(&pEntry->HTCapability, sizeof (pEntry->HTCapability));
	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if (((wdev->WepStatus != Ndis802_11WEPEnabled)
	     && (wdev->WepStatus != Ndis802_11TKIPEnable))
	    || (pAd->CommonCfg.HT_DisallowTKIP == FALSE)) {
		if ((pAd->StaCfg.BssType == BSS_INFRA) &&
		    (htcap_len != 0) &&
		    WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			bSupportN = TRUE;
		if ((pAd->StaCfg.BssType == BSS_ADHOC) &&
		    (pAd->StaCfg.bAdhocN == TRUE) &&
		    (htcap_len != 0) &&
		    WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			bSupportN = TRUE;
	}

	if (bSupportN) {
		if (ADHOC_ON(pAd))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

		ht_mode_adjust(pAd, pEntry, ht_cap, &pAd->CommonCfg.DesiredHtPhy);


		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0], &ht_cap->MCSSet[0]);

		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
			set_ht_fixed_mcs(pAd, pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);

		pEntry->MaxHTPhyMode.field.STBC = (ht_cap->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
		pEntry->MpduDensity = ht_cap->HtCapParm.MpduDensity;
		pEntry->MaxRAmpduFactor = ht_cap->HtCapParm.MaxRAmpduFactor;
		pEntry->MmpsMode = (UCHAR) ht_cap->HtCapInfo.MimoPs;
		pEntry->AMsduSize = (UCHAR) ht_cap->HtCapInfo.AMsduSize;
		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

		if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable
		    && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);

		set_sta_ht_cap(pAd, pEntry, ht_cap);

		NdisMoveMemory(&pEntry->HTCapability, ht_cap, htcap_len);

		assoc_ht_info_debugshow(pAd, pEntry, htcap_len, ht_cap);
	} else {
		pAd->MacTab.fAnyStationIsLegacy = TRUE;
	}
#endif /* DOT11_N_SUPPORT */

	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
	pEntry->CurrTxRate = pEntry->MaxSupportedRate;

#ifdef MFB_SUPPORT
	pEntry->lastLegalMfb = 0;
	pEntry->isMfbChanged = FALSE;
	pEntry->fLastChangeAccordingMfb = FALSE;

	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = 0; /* has to increment whenever a mrq is sent */
	pEntry->mrqCnt = 0;

	pEntry->pendingMfsi = 0;

	pEntry->toTxMfb = FALSE;
	pEntry->mfbToTx = 0;
	pEntry->mfb0 = 0;
	pEntry->mfb1 = 0;
#endif /* MFB_SUPPORT */

	pEntry->freqOffsetValid = FALSE;


	MlmeRAInit(pAd, pEntry);

	/* Set asic auto fall back */
	if (wdev->bAutoTxRateSwitch == TRUE) {
		UCHAR TableSize = 0;

		MlmeSelectTxRateTable(pAd, pEntry, &pEntry->pTable, &TableSize, &pEntry->CurrTxRateIndex);
		bDoTxRateSwitch = TRUE;
	} else {
		pEntry->HTPhyMode.field.MODE = wdev->HTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
		bDoTxRateSwitch = FALSE;

		/* If the legacy mode is set, overwrite the transmit setting of this entry. */
		RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}

	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	pEntry->Sst = SST_ASSOC;
	pEntry->AuthState = AS_AUTH_OPEN;
	pEntry->AuthMode = wdev->AuthMode;
	pEntry->WepStatus = wdev->WepStatus;
	pEntry->wdev = wdev;
	if (pEntry->AuthMode < Ndis802_11AuthModeWPA) {
		pEntry->WpaState = AS_NOTUSE;
		pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	} else {
		pEntry->WpaState = AS_INITPSK;
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	}

	if (pAd->StaCfg.BssType == BSS_INFRA) {
		UCHAR HashIdx = 0;
		MAC_TABLE_ENTRY *pCurrEntry = NULL;
		HashIdx = MAC_ADDR_HASH_INDEX(pAd->MlmeAux.Bssid);
		if (pAd->MacTab.Hash[HashIdx] == NULL) {
			pAd->MacTab.Hash[HashIdx] = pEntry;
		} else {
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL) {
				pCurrEntry = pCurrEntry->pNext;
			}
			pCurrEntry->pNext = pEntry;
		}
		RTMPMoveMemory(pEntry->Addr, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
		pEntry->Aid = BSSID_WCID;
		pEntry->wcid = BSSID_WCID;
		pEntry->pAd = pAd;
		SET_ENTRY_CLIENT(pEntry);
		pAd->MacTab.Size ++;

	}

#ifdef DOT11W_PMF_SUPPORT
        RTMPInitTimer(pAd, &pEntry->SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
        RTMPInitTimer(pAd, &pEntry->SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */
	NdisReleaseSpinLock(&pAd->MacTabLock);

	/*update tx burst, must after unlock pAd->MacTabLock*/
	rtmp_tx_burst_set(pAd);

	if (bDoTxRateSwitch == TRUE)
	{
		pEntry->bAutoTxRateSwitch = TRUE;
#if defined(MT7603) || defined(MT7628) || defined(MT7636)
		// TODO: shiang-MT7603, I add this here because now we relay on "pEntry->bAutoTxRateSwitch" to decide TxD format!
		MlmeNewTxRate(pAd, pEntry);
#endif
	}
	else
	{
		pEntry->bAutoTxRateSwitch = FALSE;
	}
#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
	if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP) {
		SendAssocIEsToWpaSupplicant(pAd->net_dev, pAd->StaCfg.ReqVarIEs,
					    pAd->StaCfg.ReqVarIELen);


		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
					RT_ASSOC_EVENT_FLAG, NULL, NULL, 0);
	}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	{
/*        union iwreq_data    wrqu; */
		wext_notify_event_assoc(pAd->net_dev, pAd->StaCfg.ReqVarIEs,
					pAd->StaCfg.ReqVarIELen);


		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1,
					pAd->MlmeAux.Bssid, NULL, 0);
	}
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */


	return TRUE;
}
#endif /* CONFIG_STA_SUPPORT */


static INT mac_entry_reset(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN clean)
{
	BOOLEAN Cancelled;

	RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
	RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
	RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
	RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */
	NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

	if (clean == TRUE)
	{
		pEntry->MaxSupportedRate = RATE_11;
		pEntry->CurrTxRate = RATE_11;
		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
		pEntry->PairwiseKey.KeyLen = 0;
		pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
	}

	return 0;
}


MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UINT32 ent_type,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	int i, FirstWcid;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	STA_TR_ENTRY *tr_entry = NULL;

	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

	FirstWcid = 1;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	if (pAd->StaCfg.BssType == BSS_INFRA)
		FirstWcid = 2;
#endif /* CONFIG_STA_SUPPORT */

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	for (i = FirstWcid; i< MAX_LEN_OF_MAC_TABLE; i++)   /* skip entry#0 so that "entry index == AID" for fast lookup*/
	{
		/* pick up the first available vacancy*/
		if (IS_ENTRY_NONE(&pAd->MacTab.Content[i]))
		{
			pEntry = &pAd->MacTab.Content[i];
			tr_entry = &pAd->MacTab.tr_entry[i];

			mac_entry_reset(pAd, pEntry, CleanAll);

			/* ENTRY PREEMPTION: initialize the entry */
			pEntry->wdev = wdev;
			pEntry->wcid = i;
			pEntry->func_tb_idx = wdev->func_idx;
			pEntry->bIAmBadAtheros = FALSE;
			pEntry->pAd = pAd;
			pEntry->CMTimerRunning = FALSE;

			COPY_MAC_ADDR(pEntry->Addr, pAddr);
			pEntry->Sst = SST_NOT_AUTH;
			pEntry->AuthState = AS_NOT_AUTH;
			pEntry->Aid = (USHORT)i;
			pEntry->CapabilityInfo = 0;
			pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;

			pEntry->PsMode = PWR_ACTIVE;
			pEntry->NoDataIdleCount = 0;
			pEntry->ContinueTxFailCnt = 0;
#ifdef WDS_SUPPORT
			pEntry->LockEntryTx = FALSE;
#endif /* WDS_SUPPORT */
			pEntry->TimeStamp_toTxRing = 0;

            pEntry->PsmPktCount = 0;
            pEntry->PsmUpDuration = 0;
			pEntry->PsmResetCount = 0;

			// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
			pAd->MacTab.tr_entry[i].PsMode = PWR_ACTIVE;
			pAd->MacTab.tr_entry[i].NoDataIdleCount = 0;
			pAd->MacTab.tr_entry[i].ContinueTxFailCnt = 0;
			pAd->MacTab.tr_entry[i].LockEntryTx = FALSE;
			pAd->MacTab.tr_entry[i].TimeStamp_toTxRing = 0;
			pAd->MacTab.tr_entry[i].PsDeQWaitCnt = 0;

			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			pEntry->GTKState = REKEY_NEGOTIATING;
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
			pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
			pEntry->RSNIE_Len = 0;
			NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
			pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pEntry->AllowInsPTK = TRUE;

			do
			{
#ifdef CONFIG_STA_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				if (ent_type == ENTRY_TDLS)
				{
					SET_ENTRY_TDLS(pEntry);
					break;
				}
				else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#ifdef QOS_DLS_SUPPORT
				if (ent_type == ENTRY_DLS)
				{
					SET_ENTRY_DLS(pEntry);
					break;
				}
				else
#endif /* QOS_DLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
				if (ent_type == ENTRY_APCLI)
				{
					SET_ENTRY_APCLI(pEntry);
					//SET_ENTRY_AP(pEntry);//Carter, why set entry to APCLI then set to AP????
					COPY_MAC_ADDR(pEntry->bssid, pAddr);
					pEntry->AuthMode = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode;
					pEntry->WepStatus = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.WepStatus;
					if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
					{
						pEntry->WpaState = AS_NOTUSE;
						pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					}
					else
					{
						pEntry->WpaState = AS_PTKSTART;
						pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					}
					break;
				}
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
				if (ent_type == ENTRY_WDS)
				{
					SET_ENTRY_WDS(pEntry);
					COPY_MAC_ADDR(pEntry->bssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.bssid);
					pEntry->AuthMode = Ndis802_11AuthModeOpen;
					pEntry->WepStatus = Ndis802_11EncryptionDisabled;
					break;
				}
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
				if (ent_type == ENTRY_CLIENT)
				{	/* be a regular-entry*/
					if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
						(pEntry->func_tb_idx < MAX_MBSSID_NUM(pAd)) &&
						((pEntry->func_tb_idx < HW_BEACON_MAX_NUM)) &&
						(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum != 0) &&
						(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount >= pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum))
					{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: The connection table is full in ra%d.\n", __FUNCTION__, pEntry->func_tb_idx));
						NdisReleaseSpinLock(&pAd->MacTabLock);
						return NULL;
					}
					ASSERT((wdev == &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev));

					SET_ENTRY_CLIENT(pEntry);
					pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];

					MBSS_MR_APIDX_SANITY_CHECK(pAd, pEntry->func_tb_idx);
					COPY_MAC_ADDR(pEntry->bssid, wdev->bssid);
					pEntry->AuthMode = wdev->AuthMode;
					pEntry->WepStatus = wdev->WepStatus;
					pEntry->GroupKeyWepStatus = wdev->GroupKeyWepStatus;

					if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
						pEntry->WpaState = AS_NOTUSE;
					else
						pEntry->WpaState = AS_INITIALIZE;

					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					pEntry->StaIdleTimeout = pAd->ApCfg.StaIdleTimeout;
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount++;
					pAd->ApCfg.EntryClientCount++;

					break;
				}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
				if (ent_type == ENTRY_AP || ent_type == ENTRY_ADHOC)
				{
					if  (ent_type == ENTRY_ADHOC)
                			{
                        			SET_ENTRY_ADHOC(pEntry);
                			}				
					COPY_MAC_ADDR(pEntry->bssid, pAddr);
					pEntry->AuthMode = pAd->StaCfg.wdev.AuthMode;
					pEntry->WepStatus = pAd->StaCfg.wdev.WepStatus;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}
#endif /* CONFIG_STA_SUPPORT */
			} while (FALSE);

#ifdef MT_MAC
			RTMP_SET_TR_ENTRY(pAd, i, pEntry);
#endif /* MT_MAC */
#ifdef DATA_QUEUE_RESERVE
			tr_entry->high_pkt_cnt = 0;
			tr_entry->high_pkt_drop_cnt = 0;
#endif /* DATA_QUEUE_RESERVE */
			RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, FALSE);

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			{
				if (IS_ENTRY_CLIENT(pEntry)) /* Only Client entry need the retry timer.*/
				{
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
#ifdef DOT11W_PMF_SUPPORT
					RTMPInitTimer(pAd, &pEntry->SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
					RTMPInitTimer(pAd, &pEntry->SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */
#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)
					RTMPInitTimer(pAd, &pEntry->EapReqIdRetryTimer, GET_TIMER_FUNCTION(WscEapReqIdRetryTimeout), pEntry, TRUE);
					pEntry->bEapReqIdRetryTimerRunning = FALSE;
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT) */

				}

#ifdef APCLI_SUPPORT
				if (IS_ENTRY_APCLI(pEntry))
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
#endif /* APCLI_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
			if (IS_ENTRY_DLS(pEntry))
				pAd->MacTab.tr_entry[pEntry->wcid].PortSecured = WPA_802_1X_PORT_SECURED;
#endif /* QOS_DLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
			/* Enable Stream mode for first three entries in MAC table */

#endif /* STREAM_MODE_SUPPORT */

#ifdef UAPSD_SUPPORT
			/* Ralink WDS doesn't support any power saving.*/
			if (IS_ENTRY_CLIENT(pEntry)
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				|| IS_ENTRY_TDLS(pEntry)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			)
			{
				/* init U-APSD enhancement related parameters */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): INIT UAPSD MR ENTRY",__FUNCTION__));

				UAPSD_MR_ENTRY_INIT(pEntry);
			}
#endif /* UAPSD_SUPPORT */

			pAd->MacTab.Size ++;

			/* Set the security mode of this entry as OPEN-NONE in ASIC */
			RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, (UCHAR)i);
#ifdef MT_MAC
			if (pAd->chipCap.hif_type == HIF_MT)
				MT_ADDREMOVE_KEY(pAd, 1, pEntry->func_tb_idx, 0, pEntry->wcid, PAIRWISEKEYTABLE, &pEntry->PairwiseKey, pEntry->Addr);
#endif

			/* Add this entry into ASIC RX WCID search table */
			RTMP_STA_ENTRY_ADD(pAd, pEntry);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef WSC_AP_SUPPORT
				pEntry->bWscCapable = FALSE;
				pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): alloc entry #%d, Total= %d\n",
						__FUNCTION__, i, pAd->MacTab.Size));
			break;
		}
	}

	/* add this MAC entry into HASH table */
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (pAd->MacTab.Hash[HashIdx] == NULL)
			pAd->MacTab.Hash[HashIdx] = pEntry;
		else
		{
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef DOT11V_WNM_SUPPORT
			pEntry->Beclone = FALSE;
			pEntry->bBSSMantSTASupport = FALSE;
			pEntry->bDMSSTASupport = FALSE;
#endif /* DOT11V_WNM_SUPPORT */

#ifdef WSC_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
				MAC_ADDR_EQUAL(pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.EntryAddr))
			{
				NdisZeroMemory(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.EntryAddr, MAC_ADDR_LEN);
			}
#endif /* WSC_AP_SUPPORT */

		}
#endif /* CONFIG_AP_SUPPORT */


	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	/*update tx burst, must after unlock pAd->MacTabLock*/
	rtmp_tx_burst_set(pAd);
	return pEntry;
}


INT mac_tb_del_from_hash(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY  *pPrevEntry, *pProbeEntry;

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->Addr);
	pPrevEntry = NULL;
	pProbeEntry = pAd->MacTab.Hash[HashIdx];
	ASSERT(pProbeEntry);

	/* update Hash list*/
	while(pProbeEntry)
	{
		if (pProbeEntry == pEntry)
		{
			if (pPrevEntry == NULL)
				pAd->MacTab.Hash[HashIdx] = pEntry->pNext;
			else
				pPrevEntry->pNext = pEntry->pNext;
			break;
		}

		pPrevEntry = pProbeEntry;
		pProbeEntry = pProbeEntry->pNext;
	};

	ASSERT(pProbeEntry != NULL);

	return TRUE;
}


/*
	==========================================================================
	Description:
		Delete a specified client from MAC table
	==========================================================================
 */
BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN Cancelled;

	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	pEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];
	
#ifdef MT_PS
	if (pEntry) {
		MtPsRedirectDisableCheck(pAd, wcid);
		tr_entry->ps_state = APPS_RETRIEVE_IDLE;
	}
#endif /* MT_PS */	

	if (pEntry && !IS_ENTRY_NONE(pEntry))
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC
#ifdef WSC_INCLUDED
		if (pEntry->bEapReqIdRetryTimerRunning)
		{
			RTMPCancelTimer(&pEntry->EapReqIdRetryTimer, &Cancelled);
			pEntry->bEapReqIdRetryTimerRunning = FALSE;
		}
#endif /* WSC_INCLUDED */
#endif /* MT_MAC */

		WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_WIFI_PKT_FWD
		if(wf_fwd_delete_entry_inform_hook)
			wf_fwd_delete_entry_inform_hook(pEntry->Addr);
#endif /* CONFIG_WIFI_PKT_FWD */

		/* ENTRY PREEMPTION: Cancel all timers */
		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
		RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
		RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
		RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_DOT11V_WNM)
			if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WNMCtrl.ProxyARPEnable)
			{
				RemoveIPv4ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx], pEntry->Addr);
				RemoveIPv6ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx], pEntry->Addr);
			}
#ifdef CONFIG_HOTSPOT_R2
			pEntry->IsKeep = 0;
#endif /* CONFIG_HOTSPOT_R2 */
#endif
#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			DeleteDMSEntry(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */

#ifdef DOT11_N_SUPPORT
			/* free resources of BA*/
			BASessionTearDownALL(pAd, pEntry->wcid);
#endif /* DOT11_N_SUPPORT */

			/* Delete this entry from ASIC on-chip WCID Table*/
			RTMP_STA_ENTRY_MAC_RESET(pAd, wcid);


#ifdef STREAM_MODE_SUPPORT
			/* Clear Stream Mode register for this client */
			if (pEntry->StreamModeMACReg != 0)
				RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, 0);
#endif // STREAM_MODE_SUPPORT //

#ifdef DOT11W_PMF_SUPPORT
			RTMPReleaseTimer(&pEntry->SAQueryTimer, &Cancelled);
			RTMPReleaseTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry)
			)
			{
#ifdef DOT1X_SUPPORT
				INT PmkCacheIdx = -1;
#endif /* DOT1X_SUPPORT */

				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);

#ifdef DOT1X_SUPPORT
				/* Notify 802.1x daemon to clear this sta info*/
				if (pEntry->AuthMode == Ndis802_11AuthModeWPA ||
					pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.IEEE8021X)
					DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

				/* Delete the PMK cache for this entry if it exists.*/
				if ((PmkCacheIdx = RTMPSearchPMKIDCache(pAd, pEntry->func_tb_idx, pEntry->Addr)) != -1)
				{
					RTMPDeletePMKIDCache(pAd, pEntry->func_tb_idx, PmkCacheIdx);
				}
#endif /* DOT1X_SUPPORT */

#ifdef WAPI_SUPPORT
				RTMPCancelWapiRekeyTimerAction(pAd, pEntry);
#endif /* WAPI_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
				IgmpGroupDelMembers(pAd, (PUCHAR)pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_dev);
#endif /* IGMP_SNOOP_SUPPORT */
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount--;
				pAd->ApCfg.EntryClientCount--;

#ifdef HOSTAPD_SUPPORT
				if(pEntry && pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Hostapd == TRUE )
				{
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED, -1, pEntry->Addr,
												NULL, 0,pEntry->func_tb_idx);
				}
#endif /* HOSTAPD_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
				if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
					CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr);
#endif /* RT_CFG80211_P2P_SUPPORT */

			}
#ifdef APCLI_SUPPORT
			else if (IS_ENTRY_APCLI(pEntry))
			{
				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);
			}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			mac_tb_del_from_hash(pAd, pEntry);

#ifdef CONFIG_AP_SUPPORT
			APCleanupPsQueue(pAd, &tr_entry->ps_queue); /* return all NDIS packet in PSQ*/
#endif /* CONFIG_AP_SUPPORT */

			tr_tb_reset_entry(pAd, wcid);

			/*RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, wcid);*/

#ifdef UAPSD_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			hex_dump("mac=", pEntry->Addr, 6);
			UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#else
#ifdef CONFIG_AP_SUPPORT
            UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */

			if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE)
			{
				RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
				pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			}
			RTMPReleaseTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);


#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry))
			{
				PWSC_CTRL	pWscControl = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl;
				if (MAC_ADDR_EQUAL(pEntry->Addr, pWscControl->EntryAddr))
				{
					/*
					Some WPS Client will send dis-assoc close to WSC_DONE.
					If AP misses WSC_DONE, WPS Client still sends dis-assoc to AP.
					Do not cancel timer if WscState is WSC_STATE_WAIT_DONE.
					*/
					if ((pWscControl->EapolTimerRunning == TRUE) &&
						(pWscControl->WscState != WSC_STATE_WAIT_DONE))
					{
						RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
						pWscControl->EapolTimerRunning = FALSE;
						pWscControl->EapMsgRunning = FALSE;
						NdisZeroMemory(&(pWscControl->EntryAddr[0]), MAC_ADDR_LEN);
					}
				}
				pEntry->Receive_EapolStart_EapRspId = 0;
				pEntry->bWscCapable = FALSE;
			}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


			//   			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			NdisZeroMemory(pEntry->Addr, MAC_ADDR_LEN);
			/* invalidate the entry */
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			SET_ENTRY_NONE(pEntry);

			pAd->MacTab.Size--;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab.Size));
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%s: Impossible Wcid = %d !!!!!\n", __FUNCTION__, wcid));
		}
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
#ifdef RTMP_MAC_PCI
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
#endif /* RTMP_MAC_PCI */
		rtmp_tx_burst_set(pAd);
	/*Reset operating mode when no Sta.*/
	if (pAd->MacTab.Size == 0)
	{
#ifdef DOT11_N_SUPPORT
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
#endif /* DOT11_N_SUPPORT */
		RTMP_UPDATE_PROTECT(pAd, 0, ALLN_SETPROTECT, TRUE, 0);
	}

#ifdef CONFIG_AP_SUPPORT
	/*APUpdateCapabilityAndErpIe(pAd);*/
	RTMP_AP_UPDATE_CAPABILITY_AND_ERPIE(pAd);  /* edit by johnli, fix "in_interrupt" error when call "MacTableDeleteEntry" in Rx tasklet*/
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
	==========================================================================
	Description:
		This routine reset the entire MAC table. All packets pending in
		the power-saving queues are freed here.
	==========================================================================
 */
VOID MacTableReset(RTMP_ADAPTER *pAd)
{
	int i;
	BOOLEAN Cancelled;
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	ULONG IrqFlags=0;
#endif /* RTMP_MAC_PCI */
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	UCHAR apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacTableReset\n"));
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/


	for (i=1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE 
        		if (INFRA_ON(pAd) && i == 1)
        		{
                		continue;
        		}
#endif /* #ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE */	
		pMacEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pMacEntry))
		{
			RTMPReleaseTimer(&pMacEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC)
				{
					/*  send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
					if (NStatus != NDIS_STATUS_SUCCESS)
					{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
								Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
										pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.if_addr,
										pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
										sizeof(HEADER_802_11), &DeAuthHdr,
										2, &Reason,
										END_OF_ARGS);

					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pAd, pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
#ifdef WSC_AP_SUPPORT
			BOOLEAN Cancelled;

			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].WscControl.EapolTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].WscControl.EntryAddr, MAC_ADDR_LEN);
			pAd->ApCfg.MBSSID[apidx].WscControl.EapMsgRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
			pAd->ApCfg.MBSSID[apidx].StaCount = 0;
		}
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));
		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("2McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));

		/* ENTRY PREEMPTION: Zero Mac Table but entry's content */
/*		NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));*/
		NdisZeroMemory(&pAd->MacTab.Size,
							sizeof(MAC_TABLE)-
							Offsetof(MAC_TABLE, Size));

		InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
		/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}


#ifdef MAC_REPEATER_SUPPORT
MAC_TABLE_ENTRY *InsertMacRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR IfIdx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	PAPCLI_STRUCT pApCliEntry = NULL;

	os_alloc_mem(NULL, (UCHAR **)&pEntry, sizeof(MAC_TABLE_ENTRY));

	if (pEntry)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[IfIdx];
		pEntry->Aid = pApCliEntry->MacTabWCID + 1; // TODO: We need to record count of STAs
		COPY_MAC_ADDR(pEntry->Addr, pApCliEntry->MlmeAux.Bssid);
#ifdef DBG
		printk("sn - InsertMacRepeaterEntry: Aid = %d\n", pEntry->Aid);
		hex_dump("sn - InsertMacRepeaterEntry pEntry->Addr", pEntry->Addr, 6);
#endif
		/* Add this entry into ASIC RX WCID search table */
		RTMP_STA_ENTRY_ADD(pAd, pEntry);
		os_free_mem(NULL, pEntry);
	}

	return pEntry;
}

#endif /* MAC_REPEATER_SUPPORT */

