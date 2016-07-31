/*

*/

#include <rt_config.h>


VOID mgmt_tb_set_mcast_entry(RTMP_ADAPTER *pAd)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];
	
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index*/
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate; 
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
MAC_TABLE_ENTRY *MacTableLookup(
	IN PRTMP_ADAPTER pAd,
	PUCHAR pAddr)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;
	
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry))
	{
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{
			break;
		}
		else
			pEntry = pEntry->pNext;
	}

	return pEntry;
}

#ifdef MULTI_CLIENT_SUPPORT
/* for Multi-Clients */
VOID changeTxRetry(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT num)
{		
	UINT32	TxRtyCfg, MacReg = 0;

	if (num < 3)
	{
		/* Tx data retry 31/15 (thres 2000) */
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg);
		TxRtyCfg &= 0xf0000000;
		TxRtyCfg |= 0x07d01f0f;
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg);

		/* Tx RTS retry default 32, disable RTS fallback */
		RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
		MacReg &= 0xFEFFFF00;
		MacReg |= 0x20;
		RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);
	}
	else
	{
		/* Tx data retry 7/10 (thres 256)  */
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg);
		TxRtyCfg &= 0xf0000000;
		TxRtyCfg |= 0x0100070A;
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg);

		/* Tx RTS retry 3, enable RTS fallback */
		RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
		MacReg &= 0xFEFFFF00;
		MacReg |= 0x01000003;
		RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);
	}
}

VOID pktAggrNumChange(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT num)
{
#ifdef NOISE_TEST_ADJUST
	if (num >= 5)
	{
		UCHAR idx;
		MAC_TABLE_ENTRY *pEntry = NULL;

		for (idx = 1; idx < MAX_LEN_OF_MAC_TABLE; idx++)
		{
			pEntry = &pAd->MacTab.Content[idx];

			if (pEntry && IS_ENTRY_CLIENT(pEntry))
			{
				BASessionTearDownALL(pAd, pEntry->Aid);
			}
		}
	}
#endif /* NOISE_TEST_ADJUST */
}

VOID tuneBEWMM(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT num)
{
	UCHAR  bssCwmin = 4, apCwmin = 4, apCwmax = 6;
			
	if (num <= 4)
	{
		/* use profile cwmin */
		if (pAd->CommonCfg.APCwmin > 0 && pAd->CommonCfg.BSSCwmin > 0 && pAd->CommonCfg.APCwmax > 0)
		{
			apCwmin = pAd->CommonCfg.APCwmin;
			apCwmax = pAd->CommonCfg.APCwmax;
			bssCwmin = pAd->CommonCfg.BSSCwmin;
		}
	}
	else if (num > 4 && num <= 8)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 5;
	}
	else if (num > 8 && num <= 16)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 6;
	}
	else if (num > 16 && num <= 64)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 7;
	}
	else if (num > 64 && num <= 128)
	{
		apCwmin = 4;
		apCwmax = 6;
		bssCwmin = 8;
	}
	
	pAd->CommonCfg.APEdcaParm.Cwmin[0] = apCwmin;
	pAd->CommonCfg.APEdcaParm.Cwmax[0] = apCwmax;
	pAd->ApCfg.BssEdcaParm.Cwmin[0] = bssCwmin;
			
	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
}
#endif /* MULTI_CLIENT_SUPPORT */

MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR			pAddr,
	IN	UCHAR			apidx,
	IN	UCHAR			OpMode,
	IN BOOLEAN	CleanAll)
{
	UCHAR HashIdx;
	int i, FirstWcid;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
/*	USHORT	offset;*/
/*	ULONG	addr;*/
	BOOLEAN Cancelled;
	UINT32 MaxWcidNum = MAX_LEN_OF_MAC_TABLE;
#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bAPCLI = FALSE;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT	
	if ((apidx >= MIN_NET_DEVICE_FOR_APCLI) &&
		(apidx < MIN_NET_DEVICE_FOR_MESH) &&
		(pAd->ApCfg.bMACRepeaterEn == TRUE))
	{
		MaxWcidNum = MAX_MAC_TABLE_SIZE_WITH_REPEATER;
		bAPCLI = TRUE;
	}
#endif /* MAC_REPEATER_SUPPORT */

	/* if FULL, return*/
	if (pAd->MacTab.Size >= MaxWcidNum)
		return NULL;

#ifdef MAC_REPEATER_SUPPORT	
	if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
		(apidx < MIN_NET_DEVICE_FOR_WDS) &&
		(pAd->ApCfg.EntryClientCount > MAX_LEN_OF_MAC_TABLE))
		return NULL;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	if (bAPCLI && (pAd->ApCfg.bMACRepeaterEn == TRUE))
		FirstWcid = (MAX_LEN_OF_MAC_TABLE + (MAX_EXT_MAC_ADDR_SIZE * (apidx - MIN_NET_DEVICE_FOR_APCLI)));
	else
#endif /* MAC_REPEATER_SUPPORT */
	FirstWcid = 1;


	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	for (i = FirstWcid; i< MaxWcidNum; i++)   /* skip entry#0 so that "entry index == AID" for fast lookup*/
	{
		/* pick up the first available vacancy*/
		if (IS_ENTRY_NONE(&pAd->MacTab.Content[i]))
		{
			pEntry = &pAd->MacTab.Content[i];

#ifdef RT_CFG80211_SUPPORT
			if (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
			{
				/* ENTRY PREEMPTION: initialize the entry */
				RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
				RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
			}


			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

#ifdef WFA_VHT_PF
#ifdef IP_ASSEMBLY
			if (pEntry->ip_queue_inited == 0) {
				int q_idx, ac_idx;
				struct ip_frag_q *fragQ = &pEntry->ip_fragQ[q_idx];
				
				for (ac_idx = 0; ac_idx < 4; ac_idx++) {
					InitializeQueueHeader(&pEntry->ip_queue1[ac_idx]);
					InitializeQueueHeader(&pEntry->ip_queue2[ac_idx]);
					pEntry->ip_id1[ac_idx] = pEntry->ip_id2[ac_idx] = -1;
					pEntry->ip_id1_FragSize[ac_idx] = pEntry->ip_id2_FragSize[ac_idx] = -1;
				}
				pEntry->ip_queue_inited = 1;
			}
#endif /* IP_ASSEMBLY */
#endif /* WFA_VHT_PF */

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
						pEntry->isCached = FALSE;
						break;
					}
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
					if (apidx >= MIN_NET_DEVICE_FOR_WDS)
					{
						SET_ENTRY_WDS(pEntry);
						pEntry->isCached = FALSE;
						break;
					}
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
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

			pEntry->bIAmBadAtheros = FALSE;

#ifdef RT_CFG80211_SUPPORT
			if (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
			{
				RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, FALSE);
			}


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				if (IS_ENTRY_CLIENT(pEntry)) /* Only Clent entry need the retry timer.*/
				{
#ifdef RT_CFG80211_SUPPORT
					if (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
					{
						RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
					}


				}
#ifdef APCLI_SUPPORT
				else if (IS_ENTRY_APCLI(pEntry))
				{
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
				}
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
			else if (IS_ENTRY_APCLI(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_APCLI);
			else if (IS_ENTRY_WDS(pEntry))
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_WDS);
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
						pEntry->AuthMode = pAd->ApCfg.ApCliTab[pEntry->apidx].AuthMode;
						pEntry->WepStatus = pAd->ApCfg.ApCliTab[pEntry->apidx].WepStatus;
					
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
						pEntry->MatchAPCLITabIdx = pEntry->apidx;
						break;
					}
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
					if (IS_ENTRY_WDS(pEntry))
					{
						pEntry->AuthMode = Ndis802_11AuthModeOpen;
						pEntry->WepStatus = Ndis802_11EncryptionDisabled;
					
						pEntry->MatchWDSTabIdx = pEntry->apidx;
						break;
					}
#endif /* WDS_SUPPORT */
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
						MBSS_MR_APIDX_SANITY_CHECK(pAd, apidx);
						pEntry->AuthMode = pAd->ApCfg.MBSSID[apidx].AuthMode;
						pEntry->WepStatus = pAd->ApCfg.MBSSID[apidx].WepStatus;
						pEntry->GroupKeyWepStatus = pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus;
					
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
			COPY_MAC_ADDR(pEntry->HdrAddr1, pAddr);

			do
			{
#ifdef APCLI_SUPPORT
				if (IS_ENTRY_APCLI(pEntry))
				{
					COPY_MAC_ADDR(pEntry->HdrAddr2, pAd->ApCfg.ApCliTab[pEntry->apidx].CurrentAddress);
					COPY_MAC_ADDR(pEntry->HdrAddr3, pAddr);
					break;
				}
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
				if (IS_ENTRY_WDS(pEntry))
				{
					COPY_MAC_ADDR(pEntry->HdrAddr2, pAd->ApCfg.MBSSID[MAIN_MBSSID].Bssid);
					COPY_MAC_ADDR(pEntry->HdrAddr3, pAd->ApCfg.MBSSID[MAIN_MBSSID].Bssid);
					break;
				}
#endif // WDS_SUPPORT //
#ifdef CONFIG_AP_SUPPORT
				if (OpMode == OPMODE_AP)
				{
					COPY_MAC_ADDR(pEntry->HdrAddr2, pAd->ApCfg.MBSSID[apidx].Bssid);
					COPY_MAC_ADDR(pEntry->HdrAddr3, pAd->ApCfg.MBSSID[apidx].Bssid);
					break;
				}
#endif // CONFIG_AP_SUPPORT //
			} while (FALSE);

			pEntry->Sst = SST_NOT_AUTH;
			pEntry->AuthState = AS_NOT_AUTH;
			pEntry->Aid = (USHORT)i;  /*0;*/
			pEntry->wcid = (UCHAR)i;
			pEntry->CapabilityInfo = 0;
			pEntry->PsMode = PWR_ACTIVE;
			pEntry->PsQIdleCount = 0;
			pEntry->NoDataIdleCount = 0;
			pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
			pEntry->ContinueTxFailCnt = 0;
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
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
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

			DBGPRINT(RT_DEBUG_TRACE, ("MacTableInsertEntry - allocate entry #%d, Total= %d\n",i, pAd->MacTab.Size));
			break;
		}
	}

	/* add this MAC entry into HASH table */
	if (pEntry)
	{

		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (pAd->MacTab.Hash[HashIdx] == NULL)
		{
			pAd->MacTab.Hash[HashIdx] = pEntry;
		}
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
	/* for Multi-Clients */
	if (pAd->MacTab.Size < MAX_LEN_OF_MAC_TABLE) 
	{	
		USHORT size;

		size = pAd->ApCfg.EntryClientCount;
		changeTxRetry(pAd, size);
		
		if (pAd->CommonCfg.bWmm)
			tuneBEWMM(pAd, size);
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
BOOLEAN MacTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid,
	IN PUCHAR pAddr)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	BOOLEAN Cancelled;
	UINT32 MaxWcidNum = MAX_LEN_OF_MAC_TABLE;

#ifdef MAC_REPEATER_SUPPORT	
	if (pAd->ApCfg.bMACRepeaterEn == TRUE)
		MaxWcidNum = MAX_MAC_TABLE_SIZE_WITH_REPEATER;
#endif /* MAC_REPEATER_SUPPORT */

	if (wcid >= MaxWcidNum)
		return FALSE;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	/*pEntry = pAd->MacTab.Hash[HashIdx];*/
	pEntry = &pAd->MacTab.Content[wcid];

	if (pEntry && !IS_ENTRY_NONE(pEntry))
	{
#ifdef RT_CFG80211_SUPPORT
		if (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
		{
			/* ENTRY PREEMPTION: Cancel all timers */
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
		}

		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{
#ifdef RT_CFG80211_SUPPORT 
			CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr);	
#endif /* RT_CFG80211_SUPPORT  */


			/* Delete this entry from ASIC on-chip WCID Table*/
			RTMP_STA_ENTRY_MAC_RESET(pAd, wcid);

#ifdef DOT11_N_SUPPORT
			/* free resources of BA*/
			BASessionTearDownALL(pAd, pEntry->Aid);
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
			if (pAd->chipCap.FlgHwTxBfCap)
				RTMPReleaseTimer(&pEntry->eTxBfProbeTimer, &Cancelled);
#endif /* TXBF_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
			/* Clear Stream Mode register for this client */
			if (pEntry->StreamModeMACReg != 0)
				RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, 0);
#endif // STREAM_MODE_SUPPORT //



#ifdef CONFIG_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry)
			)
			{
#ifdef DOT1X_SUPPORT 
				INT		PmkCacheIdx = -1;
#endif /* DOT1X_SUPPORT */
			
#ifdef RT_CFG80211_SUPPORT
			if (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
			{
				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);
			}

#ifdef DOT1X_SUPPORT    
				/* Notify 802.1x daemon to clear this sta info*/
				if (pEntry->AuthMode == Ndis802_11AuthModeWPA || 
					pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
					pAd->ApCfg.MBSSID[pEntry->apidx].IEEE8021X)
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
				IgmpGroupDelMembers(pAd, (PUCHAR)pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->apidx].MSSIDDev);
#endif /* IGMP_SNOOP_SUPPORT */
				pAd->ApCfg.MBSSID[pEntry->apidx].StaCount--;
				pAd->ApCfg.EntryClientCount--;

#ifdef HOSTAPD_SUPPORT
				if(pEntry && pAd->ApCfg.MBSSID[pEntry->apidx].Hostapd == TRUE )
				{
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED, -1, pEntry->Addr,
						NULL, 0,pEntry->apidx);
				}
#endif

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
						{
							pAd->MacTab.Hash[HashIdx] = pEntry->pNext;
						}
						else
						{
							pPrevEntry->pNext = pEntry->pNext;
						}
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

#ifdef WFA_VHT_PF
#ifdef IP_ASSEMBLY
			if (pAd->ip_assemble == TRUE)
			{
				if (pEntry->ip_queue_inited)
				{
					PQUEUE_ENTRY qe;
					PNDIS_PACKET q_pkt;

					qe = pEntry->ip_queue1;
					while (qe->Head)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("%s():%ld...\n", __FUNCTION__, qe->Number));

						pEntry = RemoveHeadQueue(qe);
						/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
						q_pkt = QUEUE_ENTRY_TO_PACKET(pEntry);
						RELEASE_NDIS_PACKET(pAd, q_pkt, NDIS_STATUS_FAILURE);
					}
					pEntry->ip_queue_inited = 0;
				}
			}
#endif /* IP_ASSEMBLY */
#endif /* WFA_VHT_PF */

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
            UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		if (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
		{
			if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE)
			{
	            RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
				pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
	        }
			RTMPReleaseTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
		}


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

#ifdef PEER_DELBA_TX_ADAPT
			Peer_DelBA_Tx_Adapt_Disable(pAd, pEntry);
			pEntry->bPeerDelBaTxAdaptEn = 0;
			RTMPReleaseTimer(&pEntry->DelBA_tx_AdaptTimer, &Cancelled);
#endif /* PEER_DELBA_TX_ADAPT */


//   			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			NdisZeroMemory(pEntry->Addr, MAC_ADDR_LEN);
			/* invalidate the entry */
			SET_ENTRY_NONE(pEntry);
			pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pAd->MacTab.Size --;
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
	/* for Multi-Clients */
	if (pAd->MacTab.Size < MAX_LEN_OF_MAC_TABLE) 
	{	
		USHORT size;

		size = pAd->ApCfg.EntryClientCount;
		changeTxRetry(pAd, size);
		
		if (pAd->CommonCfg.bWmm)
			tuneBEWMM(pAd, size);
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
VOID MacTableReset(
	IN  PRTMP_ADAPTER  pAd)
{
	int         i;
	BOOLEAN     Cancelled;    
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags=0;
#endif /* RTMP_MAC_PCI */
	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT      Reason;
    UCHAR       apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	UINT32		MaxWcidNum = MAX_LEN_OF_MAC_TABLE;

	DBGPRINT(RT_DEBUG_TRACE, ("MacTableReset\n"));
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/


#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn)
		MaxWcidNum = MAX_MAC_TABLE_SIZE_WITH_REPEATER;
#endif /* MAC_REPEATER_SUPPORT */

	for (i=1; i<MaxWcidNum; i++)
	{
		if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]))
	   {
	   		/* Delete a entry via WCID */

			/*MacTableDeleteEntry(pAd, i, pAd->MacTab.Content[i].Addr);*/
#ifdef RT_CFG80211_SUPPORT
			if (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
			{
				RTMPReleaseTimer(&pAd->MacTab.Content[i].EnqueueStartForPSKTimer, &Cancelled);
			}

            pAd->MacTab.Content[i].EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pAd->MacTab.Content[i].Sst == SST_ASSOC)
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
					DBGPRINT(RT_DEBUG_WARN, ("Send DEAUTH - Reason = %d frame tO %02x:%02x:%02x:%02x:%02x:%02x \n",Reason, PRINT_MAC(pAd->MacTab.Content[i].Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pAd->MacTab.Content[i].Addr, 
										pAd->ApCfg.MBSSID[pAd->MacTab.Content[i].apidx].Bssid);
			    	MakeOutgoingFrame(pOutBuffer,            &FrameLen,
			    	                  sizeof(HEADER_802_11), &DeAuthHdr,
			    	                  2,                     &Reason,
			    	                  END_OF_ARGS);

			    	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			    	MlmeFreeMemory(pAd, pOutBuffer);
					RTMPusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */

			/* Delete a entry via WCID */
			MacTableDeleteEntry(pAd, i, pAd->MacTab.Content[i].Addr);
		}
		else
		{
			/* Delete a entry via WCID */
			MacTableDeleteEntry(pAd, i, pAd->MacTab.Content[i].Addr);
		}
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
		DBGPRINT(RT_DEBUG_TRACE, ("McastPsQueue.Number %ld...\n",pAd->MacTab.McastPsQueue.Number));
		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);
		DBGPRINT(RT_DEBUG_TRACE, ("2McastPsQueue.Number %ld...\n",pAd->MacTab.McastPsQueue.Number));

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

