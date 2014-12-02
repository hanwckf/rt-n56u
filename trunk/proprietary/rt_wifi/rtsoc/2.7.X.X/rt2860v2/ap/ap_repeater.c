/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_repeater.c

    Abstract:
    Support MAC Repeater function.

    Revision History:
    Who             		When              What
    --------------  ----------      ----------------------------------------------
    Arvin				11-16-2012      created
*/

#ifdef MAC_REPEATER_SUPPORT

#include "rt_config.h"

#define IS_MULTICAST_MAC_ADDR(Addr)			((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) != 0xff))
#define IS_BROADCAST_MAC_ADDR(Addr)			((((Addr[0]) & 0xff) == 0xff))

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr)
{
	ULONG HashIdx;
	UCHAR tempMAC[6];
	REPEATER_CLIENT_ENTRY *pEntry = NULL;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry = NULL;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	COPY_MAC_ADDR(tempMAC, pAddr);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	if (bRealMAC == TRUE)
	{
		pMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];
		while (pMapEntry)
		{
			pEntry = pMapEntry->pReptCliEntry;

			if (pEntry->CliValid && MAC_ADDR_EQUAL(pEntry->OriginalAddress, tempMAC))
				break;
			else
			{
				pEntry = NULL;
				pMapEntry = pMapEntry->pNext;
			}
		}
	}
	else
	{
		pEntry = pAd->ApCfg.ReptCliHash[HashIdx];
		while (pEntry)
		{
			if (pEntry->CliValid && MAC_ADDR_EQUAL(pEntry->CurrentAddress, tempMAC))
				break;
			else
				pEntry = pEntry->pNext;
		}
	}
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return pEntry;
}

VOID RTMPInsertRepeaterAsicEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	ULONG offset, Addr;
	UCHAR tempMAC[ETH_LENGTH_OF_ADDRESS];

	DBGPRINT(RT_DEBUG_WARN, (" %s.\n", __FUNCTION__));

	COPY_MAC_ADDR(tempMAC, pAddr);
	
	offset = 0x1480 + (HW_WCID_ENTRY_SIZE * CliIdx);	
	Addr = tempMAC[0] + (tempMAC[1] << 8) +(tempMAC[2] << 16) +(tempMAC[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = tempMAC[4] + (tempMAC[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr); 

	DBGPRINT(RT_DEBUG_ERROR, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n", 
							tempMAC[0],
							tempMAC[1],
							tempMAC[2], 
							tempMAC[3],
							tempMAC[4],
							tempMAC[5],
							CliIdx));

}

VOID RTMPRemoveRepeaterAsicEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx)
{
	ULONG offset, Addr;

	DBGPRINT(RT_DEBUG_WARN, (" %s.\n", __FUNCTION__));

	offset = 0x1480 + (HW_WCID_ENTRY_SIZE * CliIdx);
	Addr = 0;
	RTMP_IO_WRITE32(pAd, offset, Addr);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);
}

VOID RTMPInsertRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
	IN PUCHAR pAddr)
{
	INT CliIdx, idx;
	UCHAR HashIdx;
	BOOLEAN Cancelled;
	UCHAR tempMAC[ETH_LENGTH_OF_ADDRESS];
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL, pCurrEntry = NULL;
	PREPEATER_CLIENT_ENTRY_MAP pReptCliMap;
	UCHAR SPEC_ADDR[6][3] = {{0x02, 0x0F, 0xB5}, {0x02, 0x09, 0x5B},
								{0x02, 0x14, 0x6C}, {0x02, 0x18, 0x4D},
								{0x02, 0x1B, 0x2F}, {0x02, 0x1E, 0x2A}};
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, (" %s.\n", __FUNCTION__));

	pMacEntry = MacTableLookup(pAd, pAddr);
	if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
	{
		if (pMacEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		{
			DBGPRINT(RT_DEBUG_ERROR, (" wireless client is not ready !!!\n"));
			return;
		}
	}

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.RepeaterCliSize >= MAX_EXT_MAC_ADDR_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" Repeater Client Full !!!\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return ;
	}

	for (CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
	{
		pReptCliEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx];

		if ((pReptCliEntry->CliEnable) && 
			(MAC_ADDR_EQUAL(pReptCliEntry->OriginalAddress, pAddr) || MAC_ADDR_EQUAL(pReptCliEntry->CurrentAddress, pAddr)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("\n  receive mac :%02x:%02x:%02x:%02x:%02x:%02x !!!\n", 
						pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
			DBGPRINT(RT_DEBUG_ERROR, (" duplicate Insert !!!\n"));
			NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
			return ;
		}

		if (pReptCliEntry->CliEnable == FALSE)
			break;
	}

	if (CliIdx >= MAX_EXT_MAC_ADDR_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" Repeater Client Full !!!\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return ;
	}

	pReptCliEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx];
	pReptCliMap = &pAd->ApCfg.ApCliTab[apidx].RepeaterCliMap[CliIdx];

	/* ENTRY PREEMPTION: initialize the entry */
	RTMPCancelTimer(&pReptCliEntry->ApCliAuthTimer, &Cancelled);
	RTMPCancelTimer(&pReptCliEntry->ApCliAssocTimer, &Cancelled);
	pReptCliEntry->CtrlCurrState = APCLI_CTRL_DISCONNECTED;
	pReptCliEntry->AuthCurrState = APCLI_AUTH_REQ_IDLE;
	pReptCliEntry->AssocCurrState = APCLI_ASSOC_IDLE;
	pReptCliEntry->CliConnectState = 0;
	pReptCliEntry->CliValid = FALSE;
	pReptCliEntry->bEthCli = FALSE;
	pReptCliEntry->MacTabWCID = 0xFF;
	pReptCliEntry->AuthReqCnt = 0;
	pReptCliEntry->AssocReqCnt = 0;
	pReptCliEntry->CliTriggerTime = 0;
	pReptCliEntry->pNext = NULL;
	pReptCliMap->pReptCliEntry = pReptCliEntry;
	pReptCliMap->pNext = NULL;

	COPY_MAC_ADDR(pReptCliEntry->OriginalAddress, pAddr);
	COPY_MAC_ADDR(tempMAC, pAddr);

	if (pAd->ApCfg.MACRepeaterOuiMode == 1)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" todo !!!\n"));
	}
	else if (pAd->ApCfg.MACRepeaterOuiMode == 2)
	{
		INT IdxToUse;

			for (idx = 0; idx < 6; idx++)
			{
				if (RTMPEqualMemory(SPEC_ADDR[idx], pAddr, 3))
					break;
			}

			/* If there is a matched one, use the next one; otherwise, use the first one. */
			if (idx >= 0  && idx < 5)
				IdxToUse = idx + 1;
			else 
				IdxToUse = 0;
			NdisCopyMemory(tempMAC, SPEC_ADDR[IdxToUse], 3);
	}
	else
	{
		NdisCopyMemory(tempMAC, pAd->ApCfg.ApCliTab[apidx].CurrentAddress, 3);
	}

	COPY_MAC_ADDR(pReptCliEntry->CurrentAddress, tempMAC);
	pReptCliEntry->CliEnable = TRUE;
	pReptCliEntry->CliConnectState = 1;
	pReptCliEntry->pNext = NULL;
	NdisGetSystemUpTime(&pReptCliEntry->CliTriggerTime);

	RTMPInsertRepeaterAsicEntry(pAd, CliIdx, tempMAC);
		
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);
	if (pAd->ApCfg.ReptCliHash[HashIdx] == NULL)
	{
		pAd->ApCfg.ReptCliHash[HashIdx] = pReptCliEntry;
	}
	else
	{
		pCurrEntry = pAd->ApCfg.ReptCliHash[HashIdx];
		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;
		pCurrEntry->pNext = pReptCliEntry;
	}

	HashIdx = MAC_ADDR_HASH_INDEX(pReptCliEntry->OriginalAddress);
	if (pAd->ApCfg.ReptMapHash[HashIdx] == NULL)
		pAd->ApCfg.ReptMapHash[HashIdx] = pReptCliMap;
	else
	{
		PREPEATER_CLIENT_ENTRY_MAP pCurrMapEntry;
	
		pCurrMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];

		while (pCurrMapEntry->pNext != NULL)
			pCurrMapEntry = pCurrMapEntry->pNext;
		pCurrMapEntry->pNext = pReptCliMap;
	}

	pAd->ApCfg.RepeaterCliSize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	NdisZeroMemory(&ApCliCtrlMsg, sizeof(APCLI_CTRL_MSG_STRUCT));
	ApCliCtrlMsg.Status = MLME_SUCCESS;
	COPY_MAC_ADDR(&ApCliCtrlMsg.SrcAddr[0], tempMAC);
	ApCliCtrlMsg.BssIdx = apidx;
	ApCliCtrlMsg.CliIdx = CliIdx;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_MT2_AUTH_REQ,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, apidx);

}

VOID RTMPRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
	IN UCHAR CliIdx)
{
	USHORT HashIdx;
	REPEATER_CLIENT_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry, *pPrevMapEntry, *pProbeMapEntry;
	BOOLEAN bVaildEntry;

	DBGPRINT(RT_DEBUG_ERROR, (" %s.\n", __FUNCTION__));

	RTMPRemoveRepeaterAsicEntry(pAd, CliIdx);

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	bVaildEntry = TRUE;
	pEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx];

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->CurrentAddress);

	pPrevEntry = NULL;
	pProbeEntry = pAd->ApCfg.ReptCliHash[HashIdx];

	if (pProbeEntry == NULL)
	{
		bVaildEntry = FALSE;
		goto done;
	}

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
					pAd->ApCfg.ReptCliHash[HashIdx] = pEntry->pNext;
				}
				else
				{
					pPrevEntry->pNext = pEntry->pNext;
				}

				bVaildEntry = TRUE;
				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);
	}
	/* not found !!!*/

	if (pProbeEntry == NULL)
	{
		bVaildEntry = FALSE;
		goto done;
	}

	ASSERT(pProbeEntry != NULL);

	pMapEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCliMap[CliIdx];

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->OriginalAddress);

	pPrevMapEntry = NULL;
	pProbeMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];
	ASSERT(pProbeMapEntry);
	if (pProbeMapEntry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (pProbeMapEntry == pMapEntry)
			{
				if (pPrevMapEntry == NULL)
				{
					pAd->ApCfg.ReptMapHash[HashIdx] = pMapEntry->pNext;
				}
				else
				{
					pPrevMapEntry->pNext = pMapEntry->pNext;
				}
				break;
			}

			pPrevMapEntry = pProbeMapEntry;
			pProbeMapEntry = pProbeMapEntry->pNext;
		} while (pProbeMapEntry);
	}
	/* not found !!!*/
	ASSERT(pProbeMapEntry != NULL);

done:

	/* set the apcli interface be invalid. */
	pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx].CliValid = FALSE;
	pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx].CliEnable = FALSE;
	pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx].CliConnectState = 0;
	NdisZeroMemory(pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx].OriginalAddress, MAC_ADDR_LEN);

	if (pAd->ApCfg.RepeaterCliSize > 0)
	{
		if (bVaildEntry == TRUE)
			pAd->ApCfg.RepeaterCliSize--;
	}

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return;
}

VOID RTMPRemoveRepeaterDisconnectEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apIdx,
	IN UCHAR CliIdx)
{
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = apIdx;
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
	MLME_DISASSOC_REQ_STRUCT DisassocReq;
	MLME_DEAUTH_REQ_STRUCT	DeAuthFrame;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_ERROR, ("(%s) Disconnect. ifIndex = %d, CliIdx = %d \n", __FUNCTION__, ifIndex, CliIdx));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;

	RTMPCancelTimer(&pApCliEntry->RepeaterCli[CliIdx].ApCliAssocTimer, &Cancelled);
	RTMPCancelTimer(&pApCliEntry->RepeaterCli[CliIdx].ApCliAuthTimer, &Cancelled);
	RTMPCancelTimer(&pApCliEntry->RepeaterCli[CliIdx].ReptCliResetEntryTimer, &Cancelled);

	if (*pCurrState == APCLI_CTRL_ASSOC)
	{
		*pCurrState = APCLI_CTRL_DEASSOC;

		DisassocParmFill(pAd, &DisassocReq, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING);

		MlmeEnqueue(pAd,
						APCLI_ASSOC_STATE_MACHINE,
						APCLI_MT2_MLME_DISASSOC_REQ,
						sizeof(MLME_DISASSOC_REQ_STRUCT),
						&DisassocReq,
						(64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));
		RTMP_MLME_HANDLER(pAd);
	}
	else if (*pCurrState == APCLI_CTRL_CONNECTED)
	{
		DeAuthFrame.Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
		COPY_MAC_ADDR(DeAuthFrame.Addr, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid);

		MlmeEnqueue(pAd, 
					  APCLI_AUTH_STATE_MACHINE, 
					  APCLI_MT2_MLME_DEAUTH_REQ, 
					  sizeof(MLME_DEAUTH_REQ_STRUCT),
					  &DeAuthFrame, 
					  (64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));
		RTMP_MLME_HANDLER(pAd);

		if (bValid)
			ApCliLinkDown(pAd, (64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));

		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}
	else
	{
		if (bValid)
			ApCliLinkDown(pAd, (64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));

		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}

	return;
}


MAC_TABLE_ENTRY *RTMPInsertRepeaterMacEntry(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pAddr,
	IN  UCHAR apidx,
	IN  UCHAR cliIdx,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	int i;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	BOOLEAN Cancelled;

	/* if FULL, return*/
	if (pAd->MacTab.Size >= MAX_MAC_TABLE_SIZE_WITH_REPEATER)
		return NULL;

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);

	i = (MAX_LEN_OF_MAC_TABLE + ((MAX_EXT_MAC_ADDR_SIZE + 1) * (apidx - MIN_NET_DEVICE_FOR_APCLI)));

	if (cliIdx != 0xFF)
		i  = i + cliIdx + 1;

	pEntry = &pAd->MacTab.Content[i];

	if (pEntry && IS_ENTRY_NONE(pEntry))
	{
		/* ENTRY PREEMPTION: initialize the entry */
		if (pEntry->RetryTimer.Valid)
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
		if (pEntry->EnqueueStartForPSKTimer.Valid)
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);

		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

		if (CleanAll == TRUE)
		{
			pEntry->MaxSupportedRate = RATE_11;
			pEntry->CurrTxRate = RATE_11;
			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		}

		SET_ENTRY_APCLI(pEntry);
		pEntry->isCached = FALSE;
		pEntry->bIAmBadAtheros = FALSE;

		RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, FALSE);
		RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);

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
		pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_APCLI);
		pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];

		pEntry->AuthMode = pAd->ApCfg.ApCliTab[pEntry->apidx].AuthMode;
		pEntry->WepStatus = pAd->ApCfg.ApCliTab[pEntry->apidx].WepStatus;
		pEntry->MatchAPCLITabIdx = pEntry->apidx;

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

		pEntry->GTKState = REKEY_NEGOTIATING;
		pEntry->PairwiseKey.KeyLen = 0;
		pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

		pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
		COPY_MAC_ADDR(pEntry->Addr, pAddr);
		COPY_MAC_ADDR(pEntry->HdrAddr1, pAddr);
		COPY_MAC_ADDR(pEntry->HdrAddr2, pAd->ApCfg.ApCliTab[pEntry->apidx].CurrentAddress);
		COPY_MAC_ADDR(pEntry->HdrAddr3, pAddr);

		pEntry->Sst = SST_NOT_AUTH;
		pEntry->AuthState = AS_NOT_AUTH;
		pEntry->Aid = (USHORT)i;  /*0;*/
		pEntry->CapabilityInfo = 0;
		pEntry->PsMode = PWR_ACTIVE;
		pEntry->PsQIdleCount = 0;
		pEntry->NoDataIdleCount = 0;
		pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
		pEntry->ContinueTxFailCnt = 0;
		pEntry->TimeStamp_toTxRing = 0;
		InitializeQueueHeader(&pEntry->PsQueue);

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

#ifdef WSC_AP_SUPPORT
		pEntry->bWscCapable = FALSE;
		pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */

#ifdef TXBF_SUPPORT
		if (pAd->chipCap.FlgHwTxBfCap)
			NdisAllocateSpinLock(pAd, &pEntry->TxSndgLock);
#endif /* TXBF_SUPPORT */

		DBGPRINT(RT_DEBUG_TRACE, ("%s - allocate entry #%d, Aid = %d, Total= %d\n",__FUNCTION__, i, pEntry->Aid, pAd->MacTab.Size));
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s - exist entry #%d, Aid = %d, Total= %d\n", __FUNCTION__, i, pEntry->Aid, pAd->MacTab.Size));
		NdisReleaseSpinLock(&pAd->MacTabLock);
		return pEntry;
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
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	return pEntry;
}

VOID RTMPRepeaterReconnectionCheck(
	IN PRTMP_ADAPTER pAd)
{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	INT i;
	PCHAR	pApCliSsid, pApCliCfgSsid;
	UCHAR	CfgSsidLen;
	NDIS_802_11_SSID Ssid;
	
	if (pAd->ApCfg.bMACRepeaterEn &&
		pAd->ApCfg.MACRepeaterOuiMode == 2 &&
		pAd->ApCfg.ApCliAutoConnectRunning == FALSE)
	{
		for (i = 0; i < MAX_APCLI_NUM; i++)
		{
			pApCliSsid = pAd->ApCfg.ApCliTab[i].Ssid;
			pApCliCfgSsid = pAd->ApCfg.ApCliTab[i].CfgSsid;
			CfgSsidLen = pAd->ApCfg.ApCliTab[i].CfgSsidLen;
			if ((pAd->ApCfg.ApCliTab[i].CtrlCurrState < APCLI_CTRL_AUTH ||
				!NdisEqualMemory(pApCliSsid, pApCliCfgSsid, CfgSsidLen)) &&
				pAd->ApCfg.ApCliTab[i].CfgSsidLen > 0 && 
				pAd->Mlme.OneSecPeriodicRound % 23 == 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, (" %s(): Scan channels for AP (%s)\n", 
							__FUNCTION__, pApCliCfgSsid));
				pAd->ApCfg.ApCliAutoConnectRunning = TRUE;
				Ssid.SsidLength = CfgSsidLen;
				NdisCopyMemory(Ssid.Ssid, pApCliCfgSsid, CfgSsidLen);
				ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE);
			}	
		}
	}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
}

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;

	if (pAd->ApCfg.RepeaterCliSize >= MAX_EXT_MAC_ADDR_SIZE)
		return FALSE;

	if(IS_MULTICAST_MAC_ADDR(pAddr))
		return FALSE;

	if(IS_BROADCAST_MAC_ADDR(pAddr))
		return FALSE;

	pEntry = RepeaterInvaildMacLookup(pAd, pAddr);

	if (pEntry)
		return FALSE;
	else
		return TRUE;
}

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	ULONG HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx];

	while (pEntry)
	{
		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
		{
			break;
		}
		else
			pEntry = pEntry->pNext;
	}

	if (pEntry && pEntry->bInsert)
		return pEntry;
	else
		return NULL;
}

VOID RTMPRepeaterInsertInvaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	UCHAR HashIdx, idx = 0;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pCurrEntry = NULL;

	if (pAd->ApCfg.ReptControl.ReptInVaildMacSize >= 32)
		return;

	if (MAC_ADDR_EQUAL(pAddr, ZERO_MAC_ADDR))
		return;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	for (idx = 0; idx< 32; idx++)
	{
		pEntry = &pAd->ApCfg.ReptControl.RepeaterInvaildEntry[idx];

		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
		{
			if (pEntry->bInsert)
			{
				NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
				return;
			}
		}

		/* pick up the first available vacancy*/
		if (pEntry->bInsert == FALSE)
		{
			NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
			COPY_MAC_ADDR(pEntry->MacAddr, pAddr);
			pEntry->bInsert = TRUE;
			break;
		}
	}

	/* add this entry into HASH table */
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		pEntry->pNext = NULL;
		if (pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx] == NULL)
		{
			pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx] = pEntry;
		}
		else
		{
			pCurrEntry = pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}
	}

	DBGPRINT(RT_DEBUG_ERROR, (" Save Need Skip MAC Address = %02X:%02X:%02X:%02X:%02X:%02X. !!!\n",
				PRINT_MAC(pEntry->MacAddr)));

	pAd->ApCfg.ReptControl.ReptInVaildMacSize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return;
}

BOOLEAN RTMPRepeaterRemoveInvaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR idx,
	IN PUCHAR pAddr)
{
	USHORT HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pPrevEntry, *pProbeEntry;
	BOOLEAN bVaildEntry;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	bVaildEntry = FALSE;
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->ApCfg.ReptControl.RepeaterInvaildEntry[idx];

	if (pEntry && pEntry->bInsert)
	{
		pPrevEntry = NULL;
		pProbeEntry = pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx];
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
						pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx] = pEntry->pNext;
					}
					else
					{
						pPrevEntry->pNext = pEntry->pNext;
					}

					bVaildEntry = TRUE;
					break;
				}
		
				pPrevEntry = pProbeEntry;
				pProbeEntry = pProbeEntry->pNext;
			} while (pProbeEntry);
		}
		/* not found !!!*/
		ASSERT(pProbeEntry != NULL);

		if ((pAd->ApCfg.ReptControl.ReptInVaildMacSize > 0) && (bVaildEntry == TRUE))
			pAd->ApCfg.ReptControl.ReptInVaildMacSize--;
	}

	NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
	pEntry->bInsert = FALSE;

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return TRUE;
}

INT	Show_Repeater_Cli_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	INT i;
    UINT32 RegValue;
	ULONG DataRate=0;

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	printk("\n");
	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	printk("BackOff Slot      : %s slot time, BKOFF_SLOT_CFG(0x1104) = 0x%08x\n", 
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED) ? "short" : "long",
 			RegValue);

#ifdef DOT11_N_SUPPORT
	printk("HT Operating Mode : %d\n", pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode);
	printk("\n");
#endif /* DOT11_N_SUPPORT */
	
	printk("\n%-19s%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1", 
		   "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");
	
	for (i = MAX_LEN_OF_MAC_TABLE; i < MAX_MAC_TABLE_SIZE_WITH_REPEATER; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_APCLI(pEntry)&& (pEntry->Sst == SST_ASSOC) && (pEntry->bReptCli))
		{
			DataRate=0;
			getRate(pEntry->HTPhyMode, &DataRate);

			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
					pEntry->ReptCliAddr[0], pEntry->ReptCliAddr[1], pEntry->ReptCliAddr[2],
					pEntry->ReptCliAddr[3], pEntry->ReptCliAddr[4], pEntry->ReptCliAddr[5]);

			printk("%-4d", (int)pEntry->Aid);
			printk("%-4d", (int)pEntry->apidx);
			printk("%-4d", (int)pEntry->PsMode);
			printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
			printk("%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
			printk("%-7d", pEntry->RssiSample.AvgRssi0);
			printk("%-7d", pEntry->RssiSample.AvgRssi1);
			printk("%-7d", pEntry->RssiSample.AvgRssi2);
			printk("%-10s", GetPhyMode(pEntry->HTPhyMode.field.MODE));
			printk("%-6s", GetBW(pEntry->HTPhyMode.field.BW));
			printk("%-6d", pEntry->HTPhyMode.field.MCS);
			printk("%-6d", pEntry->HTPhyMode.field.ShortGI);
			printk("%-6d", pEntry->HTPhyMode.field.STBC);
			printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
			printk("%-7d", (int)DataRate);
			printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount, 
						(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);
			printk("\n");
		}
	} 

	return TRUE;
}
#endif /* MAC_REPEATER_SUPPORT */

