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


// TODO: this function not finish yet!!
VOID mgmt_tb_set_mcast_entry(RTMP_ADAPTER *pAd)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];

	pEntry->EntryType = ENTRY_WDEV;	
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index*/
	pEntry->wcid = MCAST_WCID;
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
		pEntry->wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
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




MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UCHAR apidx,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	int i, FirstWcid;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	BOOLEAN Cancelled;

	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

		FirstWcid = 1;


	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	for (i = FirstWcid; i< MAX_LEN_OF_MAC_TABLE; i++)   /* skip entry#0 so that "entry index == AID" for fast lookup*/
	{
		/* pick up the first available vacancy*/
		if (IS_ENTRY_NONE(&pAd->MacTab.Content[i]))
		{
			pEntry = &pAd->MacTab.Content[i];

			/* ENTRY PREEMPTION: initialize the entry */
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
			RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
			RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

			if (CleanAll == TRUE)
			{
				pEntry->MaxSupportedRate = RATE_11;
				pEntry->CurrTxRate = RATE_11;
				NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
				pEntry->PairwiseKey.KeyLen = 0;
				pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
			}

			do
			{


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
					if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
					{
						SET_ENTRY_APCLI(pEntry);
						break;
					}
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
					if (apidx >= MIN_NET_DEVICE_FOR_WDS)
					{
						SET_ENTRY_WDS(pEntry);
						break;
					}
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
					if (OpMode == OPMODE_AP)
#else
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
					{	/* be a regular-entry*/
						if ((apidx < pAd->ApCfg.BssidNum) &&
							(apidx < MAX_MBSSID_NUM(pAd)) &&
							((apidx < HW_BEACON_MAX_NUM)) &&
							(pAd->ApCfg.MBSSID[apidx].MaxStaNum != 0) &&
							(pAd->ApCfg.MBSSID[apidx].StaCount >= pAd->ApCfg.MBSSID[apidx].MaxStaNum))
						{
							DBGPRINT(RT_DEBUG_WARN, ("%s: The connection table is full in ra%d.\n", __FUNCTION__, apidx));
							NdisReleaseSpinLock(&pAd->MacTabLock);
							return NULL;
						}
					}
#endif /* CONFIG_AP_SUPPORT */
					SET_ENTRY_CLIENT(pEntry);

		} while (FALSE);

			pEntry->wdev = wdev;
			pEntry->wcid = i;
			pEntry->isCached = FALSE;
			pEntry->bIAmBadAtheros = FALSE;

			RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, FALSE);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				if (IS_ENTRY_CLIENT(pEntry)) /* Only Client entry need the retry timer.*/
				{
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
#ifdef DOT11W_PMF_SUPPORT
					RTMPInitTimer(pAd, &pEntry->SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
					RTMPInitTimer(pAd, &pEntry->SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */

	/*				RTMP_OS_Init_Timer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pAd);*/
				}
#ifdef APCLI_SUPPORT
				else if (IS_ENTRY_APCLI(pEntry))
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
#endif /* APCLI_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef TXBF_SUPPORT
			if (pAd->chipCap.FlgHwTxBfCap)
				RTMPInitTimer(pAd, &pEntry->eTxBfProbeTimer, GET_TIMER_FUNCTION(eTxBfProbeTimerExec), pEntry, FALSE);
#endif /* TXBF_SUPPORT */

			pEntry->pAd = pAd;
			pEntry->CMTimerRunning = FALSE;
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			pEntry->RSNIE_Len = 0;
			NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
			pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;

			if (IS_ENTRY_MESH(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_MESH);
#ifdef CONFIG_AP_SUPPORT
			else if (IS_ENTRY_APCLI(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_APCLI);
			else if (IS_ENTRY_WDS(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_WDS);
#endif /* CONFIG_AP_SUPPORT */
			else
				pEntry->apidx = apidx;

#ifdef CONFIG_AP_SUPPORT
			if ((apidx < pAd->ApCfg.BssidNum) &&
				(apidx < MAX_MBSSID_NUM(pAd)) &&
				(apidx < HW_BEACON_MAX_NUM))
				pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];
			else
				pEntry->pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */

			do
			{

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
					if (IS_ENTRY_APCLI(pEntry))
					{
						pEntry->AuthMode = pAd->ApCfg.ApCliTab[pEntry->apidx].wdev.AuthMode;
						pEntry->WepStatus = pAd->ApCfg.ApCliTab[pEntry->apidx].wdev.WepStatus;
						pEntry->wdev_idx = pEntry->apidx;
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
					if (IS_ENTRY_WDS(pEntry))
					{
						pEntry->AuthMode = Ndis802_11AuthModeOpen;
						pEntry->WepStatus = Ndis802_11EncryptionDisabled;
					
						pEntry->wdev_idx = pEntry->apidx;
						break;
					}
#endif /* WDS_SUPPORT */
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
#ifdef RT_CFG80211_P2P_SUPPORT
					if (pEntry->wdev->wdev_type == WDEV_TYPE_AP)
#else
					if (OpMode == OPMODE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
#else
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
					{
						MBSS_MR_APIDX_SANITY_CHECK(pAd, apidx);
						pEntry->AuthMode = pAd->ApCfg.MBSSID[apidx].wdev.AuthMode;
						pEntry->WepStatus = pAd->ApCfg.MBSSID[apidx].wdev.WepStatus;
						pEntry->GroupKeyWepStatus = pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus;
						pEntry->wdev_idx = pEntry->apidx;
					
						if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
							pEntry->WpaState = AS_NOTUSE;
						else
							pEntry->WpaState = AS_INITIALIZE;

						pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
						pEntry->StaIdleTimeout = pAd->ApCfg.StaIdleTimeout;
						pAd->ApCfg.MBSSID[apidx].StaCount++;
						pAd->ApCfg.EntryClientCount++;
						break;
					}
#endif /* CONFIG_AP_SUPPORT */

			} while (FALSE);

			pEntry->GTKState = REKEY_NEGOTIATING;
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
				pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

			pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
			COPY_MAC_ADDR(pEntry->Addr, pAddr);

			if (IS_ENTRY_TDLS(pEntry))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("#########SET_ENTRY_TDLS pEntry->mac %02x:%02x:%02x:%02x:%02x:%02x \n",PRINT_MAC(pEntry->Addr))); //Kyle Debug
			}
			
			do
			{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
				if (IS_ENTRY_APCLI(pEntry))
				{
					COPY_MAC_ADDR(pEntry->bssid, pAddr);
					break;
				}
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
				if (IS_ENTRY_WDS(pEntry))
				{
					COPY_MAC_ADDR(pEntry->bssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.bssid);
					break;
				}
#endif // WDS_SUPPORT //
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
				if (OpMode == OPMODE_AP)
				{
					COPY_MAC_ADDR(pEntry->bssid, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
					break;
				}
#endif // CONFIG_AP_SUPPORT //
			} while (FALSE);

			pEntry->Sst = SST_NOT_AUTH;
			pEntry->AuthState = AS_NOT_AUTH;
			pEntry->Aid = (USHORT)i;
			pEntry->CapabilityInfo = 0;
			pEntry->PsMode = PWR_ACTIVE;
			pEntry->PsQIdleCount = 0;
			pEntry->NoDataIdleCount = 0;
			pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
			pEntry->ContinueTxFailCnt = 0;
			pEntry->StatTxRetryOkCount = 0;
			pEntry->StatTxFailCount = 0;
            {
				int tid;
			 	for (tid=0; tid<NUM_OF_TID; tid++)
			    		pEntry->TxBarSeq[tid] = -1;
			}
#ifdef WDS_SUPPORT
			pEntry->LockEntryTx = FALSE;
#endif /* WDS_SUPPORT */
			pEntry->TimeStamp_toTxRing = 0;
			InitializeQueueHeader(&pEntry->PsQueue);

#ifdef PS_ENTRY_MAITENANCE
			pEntry->continuous_ps_count = 0;
#endif /* PS_ENTRY_MAITENANCE */

#ifdef STREAM_MODE_SUPPORT
			/* Enable Stream mode for first three entries in MAC table */

#endif /* STREAM_MODE_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			if (OpMode == OPMODE_AP)
#else
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
			{
#ifdef UAPSD_SUPPORT
				if (IS_ENTRY_CLIENT(pEntry)) /* Ralink WDS doesn't support any power saving.*/
				{
					/* init U-APSD enhancement related parameters */
					UAPSD_MR_ENTRY_INIT(pEntry);
				}
#endif /* UAPSD_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */

			pAd->MacTab.Size ++;

			/* Set the security mode of this entry as OPEN-NONE in ASIC */
			RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, (UCHAR)i);

			/* Add this entry into ASIC RX WCID search table */
			RTMP_STA_ENTRY_ADD(pAd, pEntry);

#ifdef PEER_DELBA_TX_ADAPT
			Peer_DelBA_Tx_Adapt_Init(pAd, pEntry);
#endif /* PEER_DELBA_TX_ADAPT */

#ifdef DROP_MASK_SUPPORT
			drop_mask_init_per_client(pAd, pEntry);
#endif /* DROP_MASK_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef WSC_AP_SUPPORT
				pEntry->bWscCapable = FALSE;
				pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef TXBF_SUPPORT
			if (pAd->chipCap.FlgHwTxBfCap)
				NdisAllocateSpinLock(pAd, &pEntry->TxSndgLock);
#endif /* TXBF_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("%s(): alloc entry #%d, Total= %d\n",
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

#ifdef WSC_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->apidx < pAd->ApCfg.BssidNum) &&
				MAC_ADDR_EQUAL(pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.EntryAddr))
			{
				NdisZeroMemory(pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.EntryAddr, MAC_ADDR_LEN);
			}
#endif /* WSC_AP_SUPPORT */

		}
#endif /* CONFIG_AP_SUPPORT */


	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MULTI_CLIENT_SUPPORT
	if (pAd->MacTab.Size < MAX_LEN_OF_MAC_TABLE) 
	{	
		USHORT size;

		size = pAd->ApCfg.EntryClientCount;
		asic_change_tx_retry(pAd, size);

		pkt_aggr_num_change(pAd, size);

		if (pAd->CommonCfg.bWmm)
			asic_tune_be_wmm(pAd, size);
	}
#endif /* MULTI_CLIENT_SUPPORT */
#endif // CONFIG_AP_SUPPORT //

	NdisReleaseSpinLock(&pAd->MacTabLock);
	return pEntry;
}


/*
	==========================================================================
	Description:
		Delete a specified client from MAC table
	==========================================================================
 */
BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	BOOLEAN Cancelled;

	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->MacTab.Content[wcid];

	if (pEntry && !IS_ENTRY_NONE(pEntry))
	{
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
			if (pAd->ApCfg.MBSSID[pEntry->apidx].WNMCtrl.ProxyARPEnable)
			{
				RemoveIPv4ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->apidx], pEntry->Addr);
				RemoveIPv6ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->apidx], pEntry->Addr);
			}
#endif


#ifdef DOT11_N_SUPPORT
			/* free resources of BA*/
			BASessionTearDownALL(pAd, pEntry->wcid);
#endif /* DOT11_N_SUPPORT */

			/* Delete this entry from ASIC on-chip WCID Table*/
			RTMP_STA_ENTRY_MAC_RESET(pAd, wcid);

#ifdef TXBF_SUPPORT
			if (pAd->chipCap.FlgHwTxBfCap)
				RTMPReleaseTimer(&pEntry->eTxBfProbeTimer, &Cancelled);
#endif /* TXBF_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
			/* Clear Stream Mode register for this client */
			if (pEntry->StreamModeMACReg != 0)
				RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, 0);
#endif // STREAM_MODE_SUPPORT //

#ifdef DOT11W_PMF_SUPPORT
			RTMPReleaseTimer(&pEntry->SAQueryTimer, &Cancelled);
			RTMPReleaseTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry)
			)
			{
#ifdef DOT1X_SUPPORT 
				INT PmkCacheIdx = -1;
#endif /* DOT1X_SUPPORT */
			
				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);

#ifdef DELAYED_TCP_ACK
				RTMPCancelTimer(&pEntry->QueueAckTimer, &Cancelled);
				RTMPReleaseTimer(&pEntry->QueueAckTimer, &Cancelled);
				pEntry->QueueAckTimerRunning = FALSE;
#endif /* DELAYED_TCP_ACK */

#ifdef DOT1X_SUPPORT    
				/* Notify 802.1x daemon to clear this sta info*/
				if (pEntry->AuthMode == Ndis802_11AuthModeWPA || 
					pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
					pAd->ApCfg.MBSSID[pEntry->apidx].wdev.IEEE8021X)
					DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

				/* Delete the PMK cache for this entry if it exists.*/
				if ((PmkCacheIdx = RTMPSearchPMKIDCache(pAd, pEntry->apidx, pEntry->Addr)) != -1)
		    		{
					RTMPDeletePMKIDCache(pAd, pEntry->apidx, PmkCacheIdx);
				}
#endif /* DOT1X_SUPPORT */

#ifdef WAPI_SUPPORT
				RTMPCancelWapiRekeyTimerAction(pAd, pEntry);
#endif /* WAPI_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
				IgmpGroupDelMembers(pAd, (PUCHAR)pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->apidx].wdev.if_dev);
#endif /* IGMP_SNOOP_SUPPORT */
				pAd->ApCfg.MBSSID[pEntry->apidx].StaCount--;
				pAd->ApCfg.EntryClientCount--;

#ifdef HOSTAPD_SUPPORT
				if(pEntry && pAd->ApCfg.MBSSID[pEntry->apidx].Hostapd == Hostapd_EXT )
				{
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED, -1, pEntry->Addr,
						NULL, 0,pEntry->apidx);
				}
#endif /* HOSTAPD_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT 
				if (CFG_P2PGO_ON(pAd) || (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP))	
					CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr);
#else 
				CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr);	
#endif /* RT_CFG80211_P2P_SUPPORT */
#endif /*RT_CFG80211_SUPPORT*/
			}
#ifdef APCLI_SUPPORT
			else if (IS_ENTRY_APCLI(pEntry))
			{
				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);
			}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
           
			pPrevEntry = NULL;
			pProbeEntry = pAd->MacTab.Hash[HashIdx];
			ASSERT(pProbeEntry);
			if (pProbeEntry != NULL)
			{
				/* update Hash list*/
				do
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
				} while (pProbeEntry);
			}

			/* not found !!!*/
			ASSERT(pProbeEntry != NULL);

#ifdef CONFIG_AP_SUPPORT
			APCleanupPsQueue(pAd, &pEntry->PsQueue); /* return all NDIS packet in PSQ*/
#endif /* CONFIG_AP_SUPPORT */
			/*RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, wcid);*/

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
            UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */
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
            	PWSC_CTRL	pWscControl = &pAd->ApCfg.MBSSID[pEntry->apidx].WscControl;
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

#ifdef DROP_MASK_SUPPORT
			drop_mask_release_per_client(pAd, pEntry);
#endif /* DROP_MASK_SUPPORT */


//   			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			NdisZeroMemory(pEntry->Addr, MAC_ADDR_LEN);
			/* invalidate the entry */
			SET_ENTRY_NONE(pEntry);
			pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

			pAd->MacTab.Size--;

#ifdef TXBF_SUPPORT
			if (pAd->chipCap.FlgHwTxBfCap)
				NdisFreeSpinLock(&pEntry->TxSndgLock);
#endif /* TXBF_SUPPORT */
			DBGPRINT(RT_DEBUG_TRACE, ("MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab.Size));
		}
		else
		{
			DBGPRINT(RT_DEBUG_OFF, ("\n%s: Impossible Wcid = %d !!!!!\n", __FUNCTION__, wcid));
		}
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);

	/*Reset operating mode when no Sta.*/
	if (pAd->MacTab.Size == 0)
	{
#ifdef DOT11_N_SUPPORT
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
#endif /* DOT11_N_SUPPORT */
		RTMP_UPDATE_PROTECT(pAd, 0, ALLN_SETPROTECT, TRUE, 0);
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MULTI_CLIENT_SUPPORT
		if (pAd->MacTab.Size < MAX_LEN_OF_MAC_TABLE) 
		{	
			USHORT size;
	
			size = pAd->ApCfg.EntryClientCount;
			asic_change_tx_retry(pAd, size);
	
			pkt_aggr_num_change(pAd, size);

			if (pAd->CommonCfg.bWmm)
				asic_tune_be_wmm(pAd, size);
		}
#endif /* MULTI_CLIENT_SUPPORT */

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
	unsigned long	IrqFlags=0;
#endif /* RTMP_MAC_PCI */
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	UCHAR apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	DBGPRINT(RT_DEBUG_TRACE, ("MacTableReset\n"));
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/


	for (i=1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pMacEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pMacEntry))
		{
			RTMPReleaseTimer(&pMacEntry->EnqueueStartForPSKTimer, &Cancelled);
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
						DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					DBGPRINT(RT_DEBUG_WARN, ("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
								Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr, 
										pAd->ApCfg.MBSSID[pMacEntry->apidx].wdev.if_addr,
										pAd->ApCfg.MBSSID[pMacEntry->apidx].wdev.bssid);
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
		DBGPRINT(RT_DEBUG_TRACE, ("McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));
		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);
		DBGPRINT(RT_DEBUG_TRACE, ("2McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));

		/* ENTRY PREEMPTION: Zero Mac Table but entry's content */
/*		NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));*/
		NdisZeroMemory(&pAd->MacTab.Size,
							sizeof(MAC_TABLE)-
							sizeof(pAd->MacTab.Hash)-
							sizeof(pAd->MacTab.Content));

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
		printk("sn - InsertMacRepeaterEntry: Aid = %d\n", pEntry->Aid);
		hex_dump("sn - InsertMacRepeaterEntry pEntry->Addr", pEntry->Addr, 6);
		/* Add this entry into ASIC RX WCID search table */
		RTMP_STA_ENTRY_ADD(pAd, pEntry);
		os_free_mem(NULL, pEntry);
	}

	return pEntry;
}

#endif /* MAC_REPEATER_SUPPORT */

