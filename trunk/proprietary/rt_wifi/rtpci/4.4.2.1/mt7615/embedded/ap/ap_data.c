/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ap_data.c

	Abstract:
	Data path subroutines

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"

#define FLG_IS_OUTPUT 1
#define FLAG_IS_INPUT 0

INT ApAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pWcid)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	MAC_TABLE_ENTRY *pEntry = NULL;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	/* 0 is main BSS, FIRST_MBSSID = 1 */
	ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
	ASSERT (wdev->wdev_type == WDEV_TYPE_AP);

	if (wdev != &pAd->ApCfg.MBSSID[wdev->func_idx].wdev) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): wdev(0x%p) not equal MBSS(0x%p), func_idx=%d\n",
				__FUNCTION__, wdev, &pAd->ApCfg.MBSSID[wdev->func_idx].wdev, wdev->func_idx));
	}

#ifdef CFG80211_SUPPORT
	//CFG_TODO: POS NO GOOD
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
	{
		RTMP_SET_PACKET_OPMODE(pPacket, OPMODE_AP);
	}
#endif /* CFG80211_SUPPORT */


#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) {
		if (is_looping_packet(pAd, pPacket))
			return FALSE;
	}
#endif /* CONFIG_WIFI_PKT_FWD */

	if (!IS_ASIC_CAP(pAd, fASIC_CAP_WMM_PKTDETECT_OFFLOAD))
	{
	mt_detect_wmm_traffic(pAd, pPacket, QID_AC_BE, FLG_IS_OUTPUT);
	}

	if (MAC_ADDR_IS_GROUP(pSrcBufVA))
	{
		*pWcid = wdev->tr_tb_idx;
#ifdef MWDS
        /* If we check an ethernet source move to this device, we should remove it. */
        MWDSProxyEntryDelete(pAd, wdev->func_idx, (pSrcBufVA + MAC_ADDR_LEN));
#endif /* MWDS */
		if (wdev->PortSecured != WPA_802_1X_PORT_SECURED)
			return FALSE;
		else
			return TRUE;
	}
	else
	{
		pEntry = MacTableLookup(pAd, pSrcBufVA);
		if (pEntry && (pEntry->Sst == SST_ASSOC))
		{
 #ifdef WH_EVENT_NOTIFIER
            if(IS_ENTRY_CLIENT(pEntry)
#ifdef MWDS
               && !IS_MWDS_OPMODE_AP(pEntry)
#endif /* MWDS */
               )
                  pEntry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */ 

			*pWcid = (UCHAR)pEntry->wcid;
			return TRUE;
		}

#ifdef CLIENT_WDS
		if (pEntry == NULL) {
			PUCHAR pEntryAddr = CliWds_ProxyLookup(pAd, pSrcBufVA);
			if (pEntryAddr != NULL) {
                pEntry = MacTableLookup(pAd, pEntryAddr);
				if ((pEntry && (pEntry->Sst == SST_ASSOC)) {
					*pWcid = (UCHAR)pEntry->wcid;
					return TRUE;
				}
			}
		}
#endif /* CLIENT_WDS */

#ifdef MWDS
        if (pEntry == NULL) {
            UCHAR Wcid;
            /* If we check an ethernet source move to this device, we should remove it. */
            MWDSProxyEntryDelete(pAd, wdev->func_idx, (pSrcBufVA + MAC_ADDR_LEN));
            if (MWDSProxyLookup(pAd, wdev->func_idx, pSrcBufVA, FALSE, &Wcid)) {
                if (VALID_WCID(Wcid))
                    pEntry = &pAd->MacTab.Content[Wcid];

                if (pEntry && (pEntry->Sst == SST_ASSOC)){
                    *pWcid = Wcid;
                    return TRUE;
                }
            }
        }
#endif /* MWDS */
	}

	return FALSE;
}


enum pkt_tx_status{
	PKT_SUCCESS = 0,
	INVALID_PKT_LEN = 1,
	INVALID_TR_WCID = 2,
	INVALID_TR_ENTRY = 3,
	INVALID_WDEV = 4,
	INVALID_ETH_TYPE = 5,
	DROP_PORT_SECURE = 6,
	DROP_PSQ_FULL = 7,
	DROP_TXQ_FULL = 8,
	DROP_TX_JAM = 9,
	DROP_TXQ_ENQ_FAIL = 10,
};

struct reason_id_str{
	INT id;
	RTMP_STRING *code_str;
};


/*
	========================================================================
	Routine Description:
		This routine is used to do packet parsing and classification for Tx packet
		to AP device, and it will en-queue packets to our TxSwQ depends on AC
		class.

	Arguments:
		pAd    Pointer to our adapter
		pPacket 	Pointer to send packet

	Return Value:
		NDIS_STATUS_SUCCESS		If succes to queue the packet into TxSwQ.
		NDIS_STATUS_FAILURE			If failed to do en-queue.

	pre: Before calling this routine, caller should have filled the following fields

		pPacket->MiniportReserved[6] - contains packet source
		pPacket->MiniportReserved[5] - contains RA's WDS index (if RA on WDS link) or AID
										(if RA directly associated to this AP)
	post:This routine should decide the remaining pPacket->MiniportReserved[] fields
		before calling APHardTransmit(), such as:

		pPacket->MiniportReserved[4] - Fragment # and User PRiority
		pPacket->MiniportReserved[7] - RTS/CTS-to-self protection method and TX rate

	Note:
		You only can put OS-indepened & AP related code in here.
========================================================================
*/
INT APSendPacket(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen, frag_sz, pkt_len;
	UCHAR NumberOfFrag;
	UCHAR wcid = RESERVED_WCID, QueIdx = QID_AC_BE, UserPriority = 0;
#ifdef IGMP_SNOOP_SUPPORT
	INT InIgmpGroup = IGMP_NONE;
	MULTICAST_FILTER_TABLE_ENTRY *pGroupEntry = NULL;
#endif /* IGMP_SNOOP_SUPPORT */
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev;
	enum pkt_tx_status drop_reason = 0;
#ifdef IP_ASSEMBLY
	INT ret=0;
#endif /* IP_ASSEMBLY */


	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if ((!pSrcBufVA) || (SrcBufLen <= 14)) {
		drop_reason = INVALID_PKT_LEN;
		goto drop_pkt;
	}

	wcid = RTMP_GET_PACKET_WCID(pPacket);
    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): wcid=%d\n", __FUNCTION__, wcid));
	if (!(VALID_TR_WCID(wcid) && IS_VALID_ENTRY(&pAd->MacTab.tr_entry[wcid]))) {
		drop_reason = INVALID_TR_WCID;
		goto drop_pkt;
	}

	tr_entry = &pAd->MacTab.tr_entry[wcid];
	if (!tr_entry->wdev) {
		drop_reason = INVALID_WDEV;
		goto drop_pkt;
	}

	wdev = tr_entry->wdev;
	UserPriority = 0;
	QueIdx = QID_AC_BE;
	if (RTMPCheckEtherType(pAd, pPacket, tr_entry, wdev, &UserPriority, &QueIdx) == FALSE) {
		drop_reason = INVALID_ETH_TYPE;
		goto drop_pkt;
	}
	/* add hook point when enqueue */
	RTMP_OS_TXRXHOOK_CALL(WLAN_TX_ENQUEUE,pPacket,QueIdx,pAd);

#ifdef CONFIG_HOTSPOT
	/*
		Re-check the wcid, it maybe broadcast to unicast by RTMPCheckEtherType.
	*/
	if (wcid != RTMP_GET_PACKET_WCID(pPacket))
	{
		wcid = RTMP_GET_PACKET_WCID(pPacket);
		//pMacEntry = &pAd->MacTab.Content[wcid];
		tr_entry = &pAd->MacTab.tr_entry[wcid];
		wdev = tr_entry->wdev;
	}

	/* Drop broadcast/multicast packet if disable dgaf */
	// TODO: shiang-usw, fix me because MCAST_WCID is not used now!
	if (IS_ENTRY_CLIENT(tr_entry)) {
		BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

		if ((wcid == wdev->bss_info_argument.ucBcMcWlanIdx) &&
			(pMbss->HotSpotCtrl.HotSpotEnable || pMbss->HotSpotCtrl.bASANEnable) &&
			pMbss->HotSpotCtrl.DGAFDisable) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Drop broadcast/multicast packet when dgaf disable\n"));
			goto drop_pkt;
		}
	}
#endif

	/* AP does not send packets before port secured */
	if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
	{
		if (!((IS_AKM_WPA_CAPABILITY_Entry(wdev)
#ifdef DOT1X_SUPPORT
			|| (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */
			) && ((RTMP_GET_PACKET_EAPOL(pPacket) ||
				RTMP_GET_PACKET_WAI(pPacket))))
		)
		{
			drop_reason = DROP_PORT_SECURE;
			goto drop_pkt;
		}
	}

#ifdef MAC_REPEATER_SUPPORT
	if (VALID_UCAST_ENTRY_WCID(pAd, wcid)){
		// TODO: shiang-usw, remove pMacEntry here!
		MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[wcid];
		if (pMacEntry->bReptCli)
			pMacEntry->ReptCliIdleCount = 0;
	}
#endif /* MAC_REPEATER_SUPPORT */

	/*
		STEP 1. Decide number of fragments required to deliver this MSDU.
			The estimation here is not very accurate because difficult to
			take encryption overhead into consideration here. The result
			"NumberOfFrag" is then just used to pre-check if enough free
			TXD are available to hold this MSDU.

			The calculated "NumberOfFrag" is a rough estimation because of various
			encryption/encapsulation overhead not taken into consideration. This number is just
			used to make sure enough free TXD are available before fragmentation takes place.
			In case the actual required number of fragments of an NDIS packet
			excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
			last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of
			resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should
			rarely happen and the penalty is just like a TX RETRY fail. Affordable.

		exception:
			a). fragmentation not allowed on multicast & broadcast
			b). Aggregation overwhelms fragmentation (fCLIENT_STATUS_AGGREGATION_CAPABLE)
			c). TSO/CSO not do fragmentation
	*/
	// TODO: shiang-usw. we need to modify the TxPktClassification to adjust the NumberOfFrag!
	pkt_len = PacketInfo.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H;
	frag_sz = wlan_operate_get_frag_thld(wdev);
	frag_sz = frag_sz - LENGTH_802_11 - LENGTH_CRC;
	if (pkt_len < frag_sz)
		NumberOfFrag = 1;
	else
		NumberOfFrag = (pkt_len / frag_sz) + 1;

	/* Save fragment number to Ndis packet reserved field */
	RTMP_SET_PACKET_FRAGMENTS(pPacket, NumberOfFrag);


	/*
		3. Put to corrsponding TxSwQ or Power-saving queue

		a).WDS/ApClient/Mesh link should never go into power-save mode; just send out the frame
		b).multicast packets in IgmpSn table should never send to Power-Saving queue.
		c). M/BCAST frames are put to PSQ as long as there's any associated STA in power-save mode
	*/
	if (tr_entry->EntryType == ENTRY_CAT_MCAST)
	{
#ifdef IGMP_SNOOP_SUPPORT
		if (wdev->IgmpSnoopEnable &&
			(!((pAd->chipCap.asic_caps & fASIC_CAP_IGMP_SNOOP_OFFLOAD) == fASIC_CAP_IGMP_SNOOP_OFFLOAD)))
		{
			if (IgmpPktInfoQuery(pAd, pSrcBufVA, pPacket, wdev,
									&InIgmpGroup, &pGroupEntry) != NDIS_STATUS_SUCCESS)
				return NDIS_STATUS_FAILURE;
		}

		// TODO: shiang-usw, need to revise for Igmp snooping case!!
		if (InIgmpGroup)
		{
			/* if it's a mcast packet in igmp gourp. ucast clone it for all members in the gourp. */
			if (((InIgmpGroup == IGMP_IN_GROUP)
				&& pGroupEntry
				&& (IgmpMemberCnt(&pGroupEntry->MemberList) > 0)
				)
				|| (InIgmpGroup == IGMP_PKT)
			)
			{
				NDIS_STATUS PktCloneResult = IgmpPktClone(pAd, pPacket, InIgmpGroup, 	pGroupEntry,
													QueIdx, UserPriority, GET_OS_PKT_NETDEV(pPacket));
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
				return PktCloneResult; // need to alway return to prevent skb double free.
			}
		}
		else
#endif /* IGMP_SNOOP_SUPPORT */
		{
			UserPriority = 0;
			QueIdx = QID_AC_BE;
			RTMP_SET_PACKET_UP(pPacket, UserPriority);
		}
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_MCAST_FRAME);
	}
	else
	{
#if defined(RTMP_MAC) || defined(RLT_MAC)
		/* detect AC Category of tx packets to tune AC0(BE) TX_OP (MAC reg 0x1300) */
		// TODO: shiang-usw, check this for REG access, it should not be here!
		if  ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
			detect_wmm_traffic(pAd, UserPriority, 1);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

		RTMP_SET_PACKET_UP(pPacket, UserPriority);
	}

//MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): shiang-dbg, QueIdx=%d, tr_entry=%p\n", __FUNCTION__, __LINE__,  QueIdx, tr_entry));
	if (pAd->TxSwQueue[QueIdx].Number >= pAd->TxSwQMaxLen)
	{
		{
#ifdef BLOCK_NET_IF
			StopNetIfQueue(pAd, QueIdx, pPacket);
#endif /* BLOCK_NET_IF */
			drop_reason = DROP_TXQ_FULL;
			goto drop_pkt;
		}
	}
#ifdef WDS_SUPPORT
	else if(IS_ENTRY_WDS(tr_entry)) {
		/* when WDS Jam happen, drop following 1min to SWQueue Pkts */
		ULONG Now32;
		NdisGetSystemUpTime(&Now32);

		if ((tr_entry->LockEntryTx == TRUE)
			&& RTMP_TIME_BEFORE(Now32, tr_entry->TimeStamp_toTxRing + WDS_ENTRY_RETRY_INTERVAL)) {
			drop_reason = DROP_TX_JAM;
			goto drop_pkt;
		} else {
			if (rtmp_enq_req(pAd, pPacket, QueIdx, tr_entry, FALSE,NULL) == FALSE) {
				drop_reason = DROP_TXQ_ENQ_FAIL;
				goto drop_pkt;
			}
		}
	}
#endif /* WDS_SUPPORT */
	else
	{
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
			if ((pAd->MacTab.fAnyStationInPsm == 1) && (tr_entry->EntryType == ENTRY_CAT_MCAST)) {
				if (tr_entry->tx_queue[QID_AC_BE].Number > MAX_PACKETS_IN_MCAST_PS_QUEUE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): BSS tx_queue full\n", __FUNCTION__, __LINE__));
					drop_reason = DROP_TXQ_ENQ_FAIL;
					goto drop_pkt;
				}
			} else if ((tr_entry->EntryType != ENTRY_CAT_MCAST) && (tr_entry->PsMode == PWR_SAVE)) {
				if (tr_entry->tx_queue[QID_AC_BE].Number+tr_entry->tx_queue[QID_AC_BK].Number+tr_entry->tx_queue[QID_AC_VI].Number+tr_entry->tx_queue[QID_AC_VO].Number > MAX_PACKETS_IN_PS_QUEUE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): STA tx_queue full\n", __FUNCTION__, __LINE__));
					drop_reason = DROP_TXQ_ENQ_FAIL;
					goto drop_pkt;
				}
			}
		}
#endif /* MT_MAC */

#ifdef UAPSD_SUPPORT
		if (IS_ENTRY_CLIENT(tr_entry)
			&& (tr_entry->PsMode == PWR_SAVE)
			&& (VALID_UCAST_ENTRY_WCID(pAd, wcid))
			&& (UAPSD_MR_IS_UAPSD_AC(&pAd->MacTab.Content[wcid], QueIdx)))
		{
			UAPSD_PacketEnqueue(pAd, &pAd->MacTab.Content[wcid], pPacket, QueIdx, FALSE);
		}
		else
#endif /* UAPSD_SUPPORT */
#ifdef IP_ASSEMBLY
		if ((pAd->CommonCfg.BACapability.field.AutoBA == FALSE) &&
			(ret = rtmp_IpAssembleHandle(pAd,tr_entry, pPacket,QueIdx,PacketInfo))!=NDIS_STATUS_INVALID_DATA)
		{
			if(ret == NDIS_STATUS_FAILURE)
			{
				goto nofree_drop_pkt;
			}
			/*else if success do normal path means*/

		}else
#endif /* IP_ASSEMBLY */
		if (rtmp_enq_req(pAd, pPacket, QueIdx, tr_entry, FALSE,NULL) == FALSE) {
			drop_reason = DROP_TXQ_ENQ_FAIL;
			goto drop_pkt;
		}

		/* If the data is broadcast/multicast and any stations are in PWR_SAVE, we set BCAST TIM bit. */
		/* If the data is unicast and the station is in PWR_SAVE, we set STA TIM bit */
		if (tr_entry->EntryType == ENTRY_CAT_MCAST)
		{
			if (pAd->MacTab.fAnyStationInPsm == TRUE)
				WLAN_MR_TIM_BCMC_SET(tr_entry->func_tb_idx); /* mark MCAST/BCAST TIM bit */
		}
		else
		{
			if (IS_ENTRY_CLIENT(tr_entry) && (tr_entry->PsMode == PWR_SAVE))
			{
				/* mark corresponding TIM bit in outgoing BEACON frame */
#ifdef UAPSD_SUPPORT
				if ((VALID_UCAST_ENTRY_WCID(pAd, wcid))
					&& (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(&pAd->MacTab.Content[wcid], QueIdx)))
				{
					/*
						1. the station is UAPSD station;
						2. one of AC is non-UAPSD (legacy) AC;
						3. the destinated AC of the packet is UAPSD AC.
					*/
					/* So we can not set TIM bit due to one of AC is legacy AC */
				}
				else
#endif /* UAPSD_SUPPORT */
				{
					WLAN_MR_TIM_BIT_SET(pAd, tr_entry->func_tb_idx, tr_entry->wcid);
				}
			}
		}
	}

//TRTableEntryDump(pAd, wcid, __FUNCTION__, __LINE__);
//MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): shiang-dbg EnQDone done\n", __FUNCTION__, __LINE__));

	// TODO: shiang-usw, backup code here for ACM/WDS->LockTx related functions

	if (!pAd->chipCap.BATriggerOffload)
	{
		RTMP_BASetup(pAd, tr_entry, UserPriority);
	}

	return NDIS_STATUS_SUCCESS;

drop_pkt:
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

#ifdef IP_ASSEMBLY
nofree_drop_pkt:
#endif /* IP_ASSEMBLY */
	/*add hook point when drop*/
	RTMP_OS_TXRXHOOK_CALL(WLAN_TX_DROP,NULL,QueIdx,pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():drop pkt, drop_reason=%d!, wcid = %d\n", __FUNCTION__, drop_reason, wcid));


	return NDIS_STATUS_FAILURE;
}


/*
	--------------------------------------------------------
	FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
		Find the WPA key, either Group or Pairwise Key
		LEAP + TKIP also use WPA key.
	--------------------------------------------------------
	Decide WEP bit and cipher suite to be used.
	Same cipher suite should be used for whole fragment burst
	In Cisco CCX 2.0 Leap Authentication
		WepStatus is Ndis802_11WEPEnabled but the key will use PairwiseKey
		Instead of the SharedKey, SharedKey Length may be Zero.
*/
static inline VOID APFindCipherAlgorithm(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
    MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
    struct wifi_dev *wdev;

//TODO:Eddy, Confirm MESH/Apcli.WAPI

    wdev = pAd->wdev_list[pTxBlk->wdev_idx];

    if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame))
    {
        SET_CIPHER_NONE(pTxBlk->CipherAlg);
        pTxBlk->pKey =  NULL;
    }
    else if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
    {
        pTxBlk->CipherAlg = wdev->SecConfig.GroupCipher;
        pTxBlk->KeyIdx =  wdev->SecConfig.GroupKeyId;
        if (IS_CIPHER_WEP(wdev->SecConfig.GroupCipher))
            pTxBlk->pKey = wdev->SecConfig.WepKey[pTxBlk->KeyIdx].Key;
        else
            pTxBlk->pKey = wdev->SecConfig.GTK;
    }
    else if (pMacEntry)
    {
        pTxBlk->CipherAlg = pMacEntry->SecConfig.PairwiseCipher;
        pTxBlk->KeyIdx =  pMacEntry->SecConfig.PairwiseKeyId;
        if (IS_CIPHER_WEP(pMacEntry->SecConfig.PairwiseCipher))
            pTxBlk->pKey = pMacEntry->SecConfig.WepKey[pTxBlk->KeyIdx].Key;
        else
            pTxBlk->pKey = &pMacEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK];
    }

    /* For  BMcast pMacEntry is not initial */
    if (pTxBlk->CipherAlg == 0x0)
    {
        SET_CIPHER_NONE(pTxBlk->CipherAlg);
    }
}

#ifdef DOT11_N_SUPPORT
static inline VOID APBuildCache802_11Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHeader)
{
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *pHeader80211;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader80211 = (PHEADER_802_11)pHeader;
	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;

	/*
		Update the cached 802.11 HEADER
	*/

	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);

	/* More Bit */
	pHeader80211->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	/* Sequence */
	pHeader80211->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
	tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;

	/* SA */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	if (FALSE
#ifdef WDS_SUPPORT
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
	)
	{	/* The addr3 of WDS packet is Destination Mac address and Addr4 is the Source Mac address. */
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
		COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
		pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
		pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
	}
	else
#endif /* WDS_SUPPORT || CLIENT_WDS */
#ifdef MWDS
	if(TX_BLK_TEST_FLAG(pTxBlk, fTX_bMWDSFrame))
	{
		pHeader80211->FC.ToDs = 1;
		pHeader80211->FC.FrDs = 1;
		if(pTxBlk->pMacEntry)
		{
#ifdef APCLI_SUPPORT
			if(IS_MWDS_OPMODE_APCLI(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(pHeader80211->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);	
			}
			else
#endif /* APCLI_SUPPORT */
			if(IS_MWDS_OPMODE_AP(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(pHeader80211->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pAd->CurrentAddress); /* from AP1 */
			}
			COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN; 
		}
		else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s pTxBlk->pMacEntry == NULL!\n", __FUNCTION__));
	}
	else
#endif /* MWDS */
#ifdef APCLI_SUPPORT
	if(IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry))
	{	/* The addr3 of Ap-client packet is Destination Mac address. */
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
	}
	else
#endif /* APCLI_SUPPORT */
	{	/* The addr3 of normal packet send from DS is Src Mac address. */
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
	}

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		pTxBlk->dot11_type = pHeader80211->FC.Type;
		pTxBlk->dot11_subtype = pHeader80211->FC.SubType;
	}
#endif /* defined(MT7615) || defined(MT7622) */
}


#ifdef HDR_TRANS_TX_SUPPORT
#ifdef RLT_MAC
static inline VOID APBuildCacheWifiInfo(
	IN RTMP_ADAPTER		*pAd,
	IN TX_BLK			*pTxBlk,
	IN UCHAR			*pWiInfo)
{
	STA_TR_ENTRY *tr_entry;

	TX_WIFI_INFO *pWI;

	pWI = (TX_WIFI_INFO *)pWiInfo;
	tr_entry = pTxBlk->tr_entry;

	/* WIFI INFO size : 4 octets */
	pTxBlk->MpduHeaderLen = TX_WIFI_INFO_SIZE;

	/* More Bit */
	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	/* Sequence */
	pWI->field.Seq_Num = tr_entry->TxSeq[pTxBlk->UserPriority];
	tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;
}
#endif /* RLT_MAC */
#endif /* HDR_TRANS_TX_SUPPORT */
#endif /* DOT11_N_SUPPORT */


#ifdef HDR_TRANS_TX_SUPPORT
#ifdef RLT_MAC
static inline VOID APBuildWifiInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TX_WIFI_INFO *pWI;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;

	/* WIFI INFO size : 4 octets */
	pTxBlk->MpduHeaderLen = TX_WIFI_INFO_SIZE;

	pWI = (TX_WIFI_INFO *)&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];

	NdisZeroMemory(pWI, TX_WIFI_INFO_SIZE);

#ifdef APCLI_SUPPORT
	if (IS_ENTRY_APCLI(pTxBlk->pMacEntry))
		pWI->field.Mode = 2;	/* STA */
	else
#endif /* APCLI_SUPPORT */
	pWI->field.Mode = 1;	/* AP */

	pWI->field.QoS = (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? 1 : 0;

	if (pTxBlk->pMacEntry && tr_entry)
	{
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
		{
			pWI->field.Seq_Num = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;
    	} else {
			pWI->field.Seq_Num = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq+1) & MAXSEQ;
    	}
		pWI->field.BssIdx = pTxBlk->pMacEntry->func_tb_idx;
	}
	else
	{
		pWI->field.Seq_Num = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; /* next sequence */
	}

	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		pWI->field.WEP = 1;
}
#endif /* RLT_MAC */
#endif /* HDR_TRANS_TX_SUPPORT */


static inline VOID APBuildCommon802_11Header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;

	/*
		MAKE A COMMON 802.11 HEADER
	*/

	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	// TODO: shiang-7603
	pTxBlk->wifi_hdr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];

	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	NdisZeroMemory(wifi_hdr, sizeof(HEADER_802_11));

	wifi_hdr->FC.FrDs = 1;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

	// TODO: shiang-usw, for BCAST/MCAST, original it's sequence assigned by "pAd->Sequence", how about now?
	if (tr_entry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
			wifi_hdr->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
	    } else {
			wifi_hdr->Sequence = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq + 1) & MAXSEQ;
	    }
	}
	else
	{
		wifi_hdr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ; /* next sequence */
	}

	wifi_hdr->Frag = 0;
	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);


#ifdef MWDS
	if(TX_BLK_TEST_FLAG(pTxBlk, fTX_bMWDSFrame))
	{
		wifi_hdr->FC.ToDs = 1;
		wifi_hdr->FC.FrDs = 1;
		if(pTxBlk->pMacEntry)
		{
#ifdef APCLI_SUPPORT
			if(IS_MWDS_OPMODE_APCLI(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);	
			}
			else
#endif /* APCLI_SUPPORT */
			if(IS_MWDS_OPMODE_AP(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress); /* from AP1 */
			}
			COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(wifi_hdr->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN; 
		}
		else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s pTxBlk->pMacEntry == NULL!\n", __FUNCTION__));
	}
	else
#endif /* MWDS*/

#ifdef APCLI_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
	{
		wifi_hdr->FC.ToDs = 1;
		wifi_hdr->FC.FrDs = 0;
		COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid));	/* to AP2 */
#ifdef MAC_REPEATER_SUPPORT
		if (pTxBlk->pMacEntry && (pTxBlk->pMacEntry->bReptCli == TRUE))
			COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pMacEntry->ReptCliAddr);
		else
#endif /* MAC_REPEATER_SUPPORT */
		COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);		/* from AP1 */
		COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
	}
	else
#endif /* APCLI_SUPPORT */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	if (FALSE
#ifdef WDS_SUPPORT
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
	)
	{
		wifi_hdr->FC.ToDs = 1;
		if (pTxBlk->pMacEntry == NULL)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s pTxBlk->pMacEntry == NULL!\n", __FUNCTION__));
		else
			COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);				/* to AP2 */

		COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);						/* from AP1 */
		COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
		COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
		pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
		pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
	}
	else
#endif /* WDS_SUPPORT || CLIENT_WDS */
	{
		/* TODO: how about "MoreData" bit? AP need to set this bit especially for PS-POLL response */
#if defined(IGMP_SNOOP_SUPPORT) || defined(DOT11V_WNM_SUPPORT)
		if (pTxBlk->tr_entry->EntryType != ENTRY_CAT_MCAST)
		{
			COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr); /* DA */
		}
		else
#endif /* defined(IGMP_SNOOP_SUPPORT) || defined(DOT11V_WNM_SUPPORT) */
		{
		   	COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
		}
		COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid);		/* BSSID */
		COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);			/* SA */
	}


#ifdef RT_CFG80211_P2P_SUPPORT
	/* To not disturb the Opps test, set psm bit if I use power save mode.	*/
	/* P2P Test case 7.1.3 */
	if (CFG_P2PCLI_ON(pAd) && pAd->cfg80211_ctrl.bP2pCliPmEnable &&
		CFG80211_P2P_TEST_BIT(pAd->cfg80211_ctrl.CTWindows, P2P_OPPS_BIT))
	{
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	}
#endif /* P2P_SUPPORT */

		if (!IS_CIPHER_NONE(pTxBlk->CipherAlg))
			wifi_hdr->FC.Wep = 1;

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		pTxBlk->dot11_type = wifi_hdr->FC.Type;
		pTxBlk->dot11_subtype = wifi_hdr->FC.SubType;
	}
#endif /* defined(MT7615) || defined(MT7622) */
}


static PUCHAR APBuildARalinkFrameHeader(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *pHeaderBufPtr;
	HEADER_802_11 *wifi_hdr;
	PNDIS_PACKET pNextPacket;
	UINT32 nextBufLen;
	PQUEUE_ENTRY pQEntry;

	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

	pHeaderBufPtr = pTxBlk->wifi_hdr;

	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;

	/* steal "order" bit to mark "aggregation" */
	wifi_hdr->FC.Order = 1;

	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/* build QOS Control bytes */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		)
		{
			/*
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
			 */
			 if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif /* UAPSD_SUPPORT */

		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
		pTxBlk->wifi_hdr_len += 2;
	}

	/* padding at front of LLC header. LLC header should at 4-bytes aligment. */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);


	/*
		For RA Aggregation, put the 2nd MSDU length(extra 2-byte field) after
		QOS_CONTROL in little endian format
	*/
	pQEntry = pTxBlk->TxPacketList.Head;
	pNextPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	nextBufLen = GET_OS_PKT_LEN(pNextPacket);
	if (RTMP_GET_PACKET_VLAN(pNextPacket))
		nextBufLen -= LENGTH_802_1Q;

	*pHeaderBufPtr = (UCHAR)nextBufLen & 0xff;
	*(pHeaderBufPtr + 1) = (UCHAR)(nextBufLen >> 8);

	pHeaderBufPtr += 2;
	pTxBlk->MpduHeaderLen += 2;
	pTxBlk->wifi_hdr_len += 2;

	return pHeaderBufPtr;
}


#ifdef DOT11_N_SUPPORT
static inline BOOLEAN BuildHtcField(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN  MAC_TABLE_ENTRY *pMacEntry,
	IN PUCHAR pHeaderBufPtr)
{
	BOOLEAN bHTCPlus = FALSE;


	return bHTCPlus;
}


static PUCHAR APBuildAmsduFrameHeader(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *buf_ptr;
	HEADER_802_11 *wifi_hdr;

	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

	buf_ptr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)buf_ptr;

	/* skip common header */
	buf_ptr += pTxBlk->wifi_hdr_len;

	/* build QOS Control bytes */
	*buf_ptr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
		&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
	)
	{
		/*
		 * we can not use bMoreData bit to get EOSP bit because
		 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
		 */
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
			*buf_ptr |= (1 << 4);
	}
#endif /* UAPSD_SUPPORT */

	/* A-MSDU packet */
	*buf_ptr |= 0x80;

	*(buf_ptr + 1) = 0;
	buf_ptr += 2;
	pTxBlk->MpduHeaderLen += 2;
	pTxBlk->wifi_hdr_len += 2;

#ifdef TXBF_SUPPORT
#ifndef MT_MAC
	if (pTxBlk->pMacEntry && pAd->chipCap.FlgHwTxBfCap)
	{
		MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
		BOOLEAN bHTCPlus = FALSE;

		pTxBlk->TxSndgPkt = SNDG_TYPE_DISABLE;
		// TODO: shiang-lock, fix ME!
		NdisAcquireSpinLock(&pMacEntry->TxSndgLock);
		if (pMacEntry->TxSndgType >= SNDG_TYPE_SOUNDING)
		{
			NdisZeroMemory(buf_ptr, sizeof(HT_CONTROL));

			if (pMacEntry->TxSndgType == SNDG_TYPE_SOUNDING)
			{
				/* Select compress if supported. Otherwise select noncompress */
				if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
					(pMacEntry->HTCapability.TxBFCap.ExpComBF > 0))
					((PHT_CONTROL)buf_ptr)->CSISTEERING = 3;
				else
					((PHT_CONTROL)buf_ptr)->CSISTEERING = 2;

				/* Clear NDP Announcement */
				((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 0;

			}
			else if (pMacEntry->TxSndgType == SNDG_TYPE_NDP)
			{
				/* Select compress if supported. Otherwise select noncompress */
				if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
					(pMacEntry->HTCapability.TxBFCap.ExpComBF > 0) &&
					(pMacEntry->HTCapability.TxBFCap.ComSteerBFAntSup >= (pMacEntry->sndgMcs / 8))
					)
					((PHT_CONTROL)buf_ptr)->CSISTEERING = 3;
					else
					((PHT_CONTROL)buf_ptr)->CSISTEERING = 2;

					/* Set NDP Announcement */
				((PHT_CONTROL)buf_ptr)->NDPAnnounce = 1;

				pTxBlk->TxNDPSndgBW = pMacEntry->sndgBW;
				pTxBlk->TxNDPSndgMcs = pMacEntry->sndgMcs;
			}

			pTxBlk->TxSndgPkt = pMacEntry->TxSndgType;
			/* arvin add for julian request send NDP */
			pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
			bHTCPlus = TRUE;
		}
		NdisReleaseSpinLock(&pMacEntry->TxSndgLock);

#ifdef MFB_SUPPORT
#if defined(MRQ_FORCE_TX) /* have to replace this by the correct condition!!! */
		pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_MRQ;
#endif

		/*
			Ignore sounding frame because the signal format of sounding frmae may
			be different from normal data frame, which may result in different MFB
		*/
		if ((pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ) &&
			(pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE))
		{
			if (bHTCPlus == FALSE)
			{
				bHTCPlus = TRUE;
				NdisZeroMemory(buf_ptr, sizeof(HT_CONTROL));
			}

			MFB_PerPareMRQ(pAd, buf_ptr, pMacEntry);
		}

		if (pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ && pMacEntry->toTxMfb == 1)
		{
			if (bHTCPlus == FALSE)
			{
				NdisZeroMemory(buf_ptr, sizeof(HT_CONTROL));
				bHTCPlus = TRUE;
			}

			MFB_PerPareMFB(pAd, buf_ptr, pMacEntry); /* not complete yet!!! */
			pMacEntry->toTxMfb = 0;
		}
#endif /* MFB_SUPPORT */

		if (bHTCPlus == TRUE)
		{
			wifi_hdr->FC.Order = 1;
			buf_ptr += 4;
			pTxBlk->MpduHeaderLen += 4;
			pTxBlk->wifi_hdr_len += 4;
		}
	}
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

	/*
		padding at front of LLC header
		LLC header should locate at 4-octets aligment
		@@@ MpduHeaderLen excluding padding @@@
	*/
	pTxBlk->HdrPadLen = (ULONG)buf_ptr;
	buf_ptr = (UCHAR *)(ROUND_UP(buf_ptr, 4));
	pTxBlk->HdrPadLen = (ULONG)(buf_ptr - pTxBlk->HdrPadLen);

	return buf_ptr;

}


static VOID APAmpduFrameTx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr = NULL, *src_ptr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry;
	PQUEUE_ENTRY pQEntry;
	BOOLEAN bHTCPlus = FALSE;
	UINT hdr_offset, cache_sz = 0;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;

	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount++;
#endif /* STATS_COUNT_SUPPORT */

		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;

	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
	{
		if (pAd->chipCap.hif_type == HIF_MT) {
			hdr_offset = tx_hw_hdr_len;
		} else {
			hdr_offset = TXINFO_SIZE + TXWISize + TSO_SIZE;
		}

		if ((tr_entry->isCached)
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			&& (pMacEntry->TxSndgType == SNDG_TYPE_DISABLE)
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
		)
		{
#ifndef VENDOR_FEATURE1_SUPPORT
			NdisMoveMemory((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]),
								(UCHAR *)(&tr_entry->CachedBuf[0]),
								TXWISize + sizeof(HEADER_802_11));
#else
			pTxBlk->HeaderBuf = (UCHAR *)(tr_entry->HeaderBuf);
#endif /* VENDOR_FEATURE1_SUPPORT */

			pHeaderBufPtr = (UCHAR *)(&pTxBlk->HeaderBuf[hdr_offset]);
			APBuildCache802_11Header(pAd, pTxBlk, pHeaderBufPtr);

#ifdef SOFT_ENCRYPT
			RTMPUpdateSwCacheCipherInfo(pAd, pTxBlk, pHeaderBufPtr);
#endif /* SOFT_ENCRYPT */
		}
		else
		{
			APFindCipherAlgorithm(pAd, pTxBlk);
			APBuildCommon802_11Header(pAd, pTxBlk);

			pHeaderBufPtr = &pTxBlk->HeaderBuf[hdr_offset];
		}

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
				RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}
#endif /* SOFT_ENCRYPT */

		wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;

//+++Add by shiang for debug
//---Add by shiang for debug

		/* skip common header */
		pHeaderBufPtr += pTxBlk->MpduHeaderLen;

#ifdef VENDOR_FEATURE1_SUPPORT
		if (tr_entry->isCached
			&& (tr_entry->Protocol == (RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket)))
#ifdef SOFT_ENCRYPT
			&& !TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)
#endif /* SOFT_ENCRYPT */
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			&& (pMacEntry->TxSndgType == SNDG_TYPE_DISABLE)
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
		)
		{
			/* build QOS Control bytes */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
			if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
				&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
				)
			{
				/*
				 * we can not use bMoreData bit to get EOSP bit because
				 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
				 */
				if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
					*pHeaderBufPtr |= (1 << 4);
			}
#endif /* UAPSD_SUPPORT */
			pTxBlk->MpduHeaderLen = tr_entry->MpduHeaderLen;
			pTxBlk->wifi_hdr_len = tr_entry->wifi_hdr_len;
			pHeaderBufPtr = ((UCHAR *)wifi_hdr) + pTxBlk->MpduHeaderLen;

			pTxBlk->HdrPadLen = tr_entry->HdrPadLen;

			/* skip 802.3 header */
			pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;

			/* skip vlan tag */
			bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
			if (bVLANPkt) {
				pTxBlk->pSrcBufData += LENGTH_802_1Q;
				pTxBlk->SrcBufLen -= LENGTH_802_1Q;
			}
		}
		else
#endif /* VENDOR_FEATURE1_SUPPORT */
		{
			/* build QOS Control bytes */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
			if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
				&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
				)
			{
				/*
				 * we can not use bMoreData bit to get EOSP bit because
				 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
				 */
				if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
					*pHeaderBufPtr |= (1 << 4);
			}
#endif /* UAPSD_SUPPORT */

			*(pHeaderBufPtr + 1) = 0;
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += 2;
			pTxBlk->wifi_hdr_len += 2;

#ifndef MT_MAC
			// TODO: Shiang-usw, we need a more proper way to handle this instead of ifndef MT_MAC !
			/* For MT_MAC, SW not to prepare the HTC field for RDG enable */
			/* build HTC control field after QoS field */
			if ((pAd->CommonCfg.bRdg == TRUE)
				&& (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))
#ifdef TXBF_SUPPORT
				&& (pMacEntry->TxSndgType != SNDG_TYPE_NDP)
#endif /* TXBF_SUPPORT */
			)
			{
				NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
				((PHT_CONTROL)pHeaderBufPtr)->RDG = 1;
				bHTCPlus = TRUE;
			}
#endif /* MT_MAC */

#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			if (pAd->chipCap.FlgHwTxBfCap)
			{
				pTxBlk->TxSndgPkt = SNDG_TYPE_DISABLE;
				// TODO: shiang-lock, fix ME!!
				NdisAcquireSpinLock(&pMacEntry->TxSndgLock);
				if (pMacEntry->TxSndgType >= SNDG_TYPE_SOUNDING)
				{
					if (bHTCPlus == FALSE)
					{
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}

					if (pMacEntry->TxSndgType == SNDG_TYPE_SOUNDING)
					{
						/* Select compress if supported. Otherwise select noncompress */
						if ((pAd->CommonCfg.ETxBfNoncompress==0) &&
							(pMacEntry->HTCapability.TxBFCap.ExpComBF>0))
								((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
						else
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

						/* Clear NDP Announcement */
						((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 0;
					}
					else if (pMacEntry->TxSndgType == SNDG_TYPE_NDP)
					{
						/* Select compress if supported. Otherwise select noncompress */
						if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
							(pMacEntry->HTCapability.TxBFCap.ExpComBF>0) &&
							(pMacEntry->HTCapability.TxBFCap.ComSteerBFAntSup >= (pMacEntry->sndgMcs/8))
						)
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
						else
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

						/* Set NDP Announcement */
						((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 1;

						pTxBlk->TxNDPSndgBW = pMacEntry->sndgBW;
						pTxBlk->TxNDPSndgMcs = pMacEntry->sndgMcs;
					}

					pTxBlk->TxSndgPkt = pMacEntry->TxSndgType;
					pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
				}
				NdisReleaseSpinLock(&pMacEntry->TxSndgLock);

#ifdef MFB_SUPPORT
#if defined(MRQ_FORCE_TX)
				/* have to replace this by the correct condition!!! */
				pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_MRQ;
#endif

				/*
					Ignore sounding frame because the signal format of sounding frmae may
					be different from normal data frame, which may result in different MFB
				*/
				if ((pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ) &&
					(pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE))
				{
					if (bHTCPlus == FALSE)
					{
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}
					MFB_PerPareMRQ(pAd, pHeaderBufPtr, pMacEntry);
				}

				if (pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ &&
					pMacEntry->toTxMfb == 1)
				{
					if (bHTCPlus == FALSE)
					{
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}
					MFB_PerPareMFB(pAd, pHeaderBufPtr, pMacEntry);/* not complete yet!!! */
					pMacEntry->toTxMfb = 0;
				}
#endif /* MFB_SUPPORT */
			}
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

			if (bHTCPlus == TRUE)
			{
				wifi_hdr->FC.Order = 1;
				pHeaderBufPtr += 4;
				pTxBlk->MpduHeaderLen += 4;
				pTxBlk->wifi_hdr_len += 4;
			}

			/*pTxBlk->MpduHeaderLen = pHeaderBufPtr - pTxBlk->HeaderBuf - TXWI_SIZE - TXINFO_SIZE; */
			ASSERT(pTxBlk->MpduHeaderLen >= 24);

			/* skip 802.3 header */
			pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;

			/* skip vlan tag */
			if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket))
			{
				pTxBlk->pSrcBufData += LENGTH_802_1Q;
				pTxBlk->SrcBufLen -= LENGTH_802_1Q;
			}

			/*
			   padding at front of LLC header
			   LLC header should locate at 4-octets aligment

			   @@@ MpduHeaderLen excluding padding @@@
			*/
			pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
			pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
			pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef VENDOR_FEATURE1_SUPPORT
			tr_entry->HdrPadLen = pTxBlk->HdrPadLen;
#endif /* VENDOR_FEATURE1_SUPPORT */

#ifdef SOFT_ENCRYPT
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
				tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
			}
			else
#endif /* SOFT_ENCRYPT */
			{

				/*
					Insert LLC-SNAP encapsulation - 8 octets
					if original Ethernet frame contains no LLC/SNAP,
					then an extra LLC/SNAP encap is required
				*/
				EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);
				if (pTxBlk->pExtraLlcSnapEncap) {
					NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
					pHeaderBufPtr += 6;
					/* get 2 octets (TypeofLen) */
					NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2, 2);

					pHeaderBufPtr += 2;
					pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
				}
			}

#ifdef VENDOR_FEATURE1_SUPPORT
			tr_entry->Protocol = RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket);
			tr_entry->MpduHeaderLen = pTxBlk->MpduHeaderLen;
			tr_entry->wifi_hdr_len = pTxBlk->wifi_hdr_len;
#endif /* VENDOR_FEATURE1_SUPPORT */
		}
	}
	else
	{
		APFindCipherAlgorithm(pAd, pTxBlk);
		pTxBlk->MpduHeaderLen = 0;
		pTxBlk->HdrPadLen = 2;
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	}

	if ((tr_entry->isCached)
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
		&& (pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE)
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
	)
	{
		write_tmac_info_Cache(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);
	}
	else
	{
		if (write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk) == NDIS_STATUS_FAILURE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
		
		if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
			tr_entry->isCached = FALSE;


		NdisZeroMemory((UCHAR *)(&tr_entry->CachedBuf[0]), sizeof(tr_entry->CachedBuf));

		if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
		{
			cache_sz = (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]));
			src_ptr = (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);
			NdisMoveMemory((UCHAR *)(&tr_entry->CachedBuf[0]), src_ptr, cache_sz);
		}

#ifdef VENDOR_FEATURE1_SUPPORT
		/* use space to get performance enhancement */
		NdisZeroMemory((UCHAR *)(&tr_entry->HeaderBuf[0]), sizeof(tr_entry->HeaderBuf));
		NdisMoveMemory((UCHAR *)(&tr_entry->HeaderBuf[0]),
						(UCHAR *)(&pTxBlk->HeaderBuf[0]),
						(pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[0])));
#endif /* VENDOR_FEATURE1_SUPPORT */

//+++Mark by shiang test
//		tr_entry->isCached = TRUE;
//---Mark by shiang for test
	}

//+++Add by shiang for debug
//---Add by shiang for debug

#ifdef TXBF_SUPPORT
#ifndef MT_MAC
	if (pTxBlk->TxSndgPkt != SNDG_TYPE_DISABLE)
		tr_entry->isCached = FALSE;
#endif
#endif /* TXBF_SUPPORT */

#ifdef STATS_COUNT_SUPPORT
	pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart++;
	pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;

	/* calculate Tx count and ByteCount per BSS */
    if ((pMacEntry)
#ifdef WAPI_SUPPORT
        && (IS_ENTRY_CLIENT(pMacEntry))
#endif /* WAPI_SUPPORT */
    )
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

#ifdef WAPI_SUPPORT
		if (pMacEntry->SecConfig.WapiUskRekeyTimerRunning &&
			pMacEntry->SecConfig.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pMacEntry->SecConfig.wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
#endif /* WAPI_SUPPORT */

		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
            pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
		}
	}

#ifdef WDS_SUPPORT
		if (pMacEntry && IS_ENTRY_WDS(pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &freeCnt);


#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)wifi_hdr);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
}


static VOID APAmsduFrameTx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *pHeaderBufPtr, *subFrameHeader;
	USHORT freeCnt = 1; /* no use */
	USHORT subFramePayloadLen = 0;	/* AMSDU Subframe length without AMSDU-Header / Padding */
	USHORT totalMPDUSize = 0;
	UCHAR padding = 0;
	USHORT FirstTx = 0, LastTxIdx = 0;
	int frameNum = 0;
	PQUEUE_ENTRY pQEntry;
	STA_TR_ENTRY *tr_entry;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	PAPCLI_STRUCT pApCliEntry = NULL;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = NULL;

	ASSERT((pTxBlk->TxPacketList.Number > 1));

	while (pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
			BSS_STRUCT *pMbss = pTxBlk->pMbss;

			if (pMbss != NULL)
				pMbss->TxDropCount++;
#endif /* STATS_COUNT_SUPPORT */
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket)) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}

		if (frameNum == 0)
		{
			pHeaderBufPtr = APBuildAmsduFrameHeader(pAd, pTxBlk);

			/* NOTE: TxWI->TxWIMPDUByteCnt will be updated after final frame was handled. */
#ifdef WFA_VHT_PF
			if (pAd->force_amsdu)
			{
				UCHAR RABAOriIdx;

				if (pMacEntry) {
					 RABAOriIdx = pMacEntry->BAOriWcidArray[pTxBlk->UserPriority];
					if (((pMacEntry->TXBAbitmap & (1<<pTxBlk->UserPriority)) != 0) &&
						(pAd->BATable.BAOriEntry[RABAOriIdx].amsdu_cap == TRUE))
						TX_BLK_SET_FLAG (pTxBlk, fTX_AmsduInAmpdu);
				}
			}
#endif /* WFA_VHT_PF */


			if (write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk) == NDIS_STATUS_FAILURE) {
				RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
				continue;
			}


			if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
				if (pTxBlk->tr_entry)
					pTxBlk->tr_entry->isCached = FALSE;
		}
		else
		{
			// TODO: shiang-usw, check this, original code is use pTxBlk->HeaderBuf[0]
			pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE];
			padding = ROUND_UP(AMSDU_SUBHEAD_LEN + subFramePayloadLen, 4) - (AMSDU_SUBHEAD_LEN + subFramePayloadLen);
			NdisZeroMemory(pHeaderBufPtr, padding + AMSDU_SUBHEAD_LEN);
			pHeaderBufPtr += padding;
			pTxBlk->MpduHeaderLen = padding;
		}

		/*
			A-MSDU subframe
				DA(6)+SA(6)+Length(2) + LLC/SNAP Encap
		*/
		subFrameHeader = pHeaderBufPtr;
		subFramePayloadLen = pTxBlk->SrcBufLen;

		NdisMoveMemory(subFrameHeader, pTxBlk->pSrcBufHeader, 12);

#ifdef APCLI_SUPPORT
		if(TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pTxBlk->pMacEntry->bReptCli)
			{
				pReptEntry = &pAd->ApCfg.pRepeaterCliPool[pTxBlk->pMacEntry->MatchReptCliIdx];
				if (pReptEntry->CliValid)
					NdisMoveMemory(&subFrameHeader[6] , pReptEntry->CurrentAddress, 6);
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pApCliEntry = &pAd->ApCfg.ApCliTab[pTxBlk->pMacEntry->func_tb_idx];
				if (pApCliEntry->Valid)
					NdisMoveMemory(&subFrameHeader[6] , pApCliEntry->wdev.if_addr, 6);
			}
		}
#endif /* APCLI_SUPPORT */


		pHeaderBufPtr += AMSDU_SUBHEAD_LEN;
		pTxBlk->MpduHeaderLen += AMSDU_SUBHEAD_LEN;



		/* Insert LLC-SNAP encapsulation - 8 octets */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);

		subFramePayloadLen = pTxBlk->SrcBufLen;

		if (pTxBlk->pExtraLlcSnapEncap) {
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			subFramePayloadLen += LENGTH_802_1_H;
		}

		/* update subFrame Length field */
		subFrameHeader[12] = (subFramePayloadLen & 0xFF00) >> 8;
		subFrameHeader[13] = subFramePayloadLen & 0xFF;

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;

		if (frameNum == 0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);
		else
			LastTxIdx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), NULL);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		frameNum++;


		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

#ifdef STATS_COUNT_SUPPORT
		{
			/* calculate Transmitted AMSDU Count and ByteCount */
			pAd->RalinkCounters.TxAMSDUCount.u.LowPart++;
		}

		/* calculate Tx count and ByteCount per BSS */
#ifdef WAPI_SUPPORT
		if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif /* WAPI_SUPPORT */
		{
			BSS_STRUCT *pMbss = pTxBlk->pMbss;
			MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
			if (pTxBlk->pMacEntry->SecConfig.WapiUskRekeyTimerRunning && pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_method == REKEY_METHOD_PKT)
				pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_cnt += totalMPDUSize;
#endif /* WAPI_SUPPORT */

			if (pMbss != NULL)
			{
				pMbss->TransmittedByteCount += totalMPDUSize;
				pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
				if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->mcPktsTx++;
				else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->bcPktsTx++;
				else
					pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
			}

			if(pMacEntry->Sst == SST_ASSOC)
			{
				INC_COUNTER64(pMacEntry->TxPackets);
				pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
                pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
			}
		}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */
	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}
#endif /* DOT11_N_SUPPORT */

#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
static VOID vow_clone_legacy_frame(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
    UINT32 i;
    PNDIS_PACKET pkt;
    //UINT32 MpduHeaderLen = pTxBlk->MpduHeaderLen;
    //UINT32 HdrPadL = pTxBlk->HdrPadLen;
    TX_BLK txb;

    //clone packet
    struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
    pkt = DuplicatePacket(wdev->if_dev, pTxBlk->pPacket);

    //backup TXBLK
    os_move_mem(&txb, pTxBlk, sizeof(TX_BLK));

    //transmit the original packet first 
	CutThroughPktTx(pAd, pTxBlk);
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
	
    if (pkt == NULL)
    {
    	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: DuplicatePacket failed!!\n", __FUNCTION__));
        return;
    }
    else
    {
    	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: clone 1 pkt %p, vow_cloned_wtbl_num %d\n", 
            __FUNCTION__, pkt, pAd->vow_cloned_wtbl_max));
    }

    //return BC/MC 
    if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
    {
        UCHAR wmm_set = HcGetWmmIdx(pAd,wdev);

        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\x1b[32m%s: bc/mc packet ........ wcid %d\x1b[m\n", 
                        __FUNCTION__, pTxBlk->Wcid));

    	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wcid %d, wmm set %d\n", 
            __FUNCTION__, pTxBlk->Wcid, wmm_set));

        //RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
        return;
    }

    //restore MPDU header length
    //pTxBlk->MpduHeaderLen = MpduHeaderLen;
    //pTxBlk->HdrPadLen = HdrPadL;
    
    if (pAd->vow_cloned_wtbl_max)
    {
        UINT32 end, start;
	    struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
        UCHAR wmm_set = HcGetWmmIdx(pAd,wdev);

    	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, wmm set %d\n", 
            __FUNCTION__, pTxBlk->Wcid, wmm_set));

        if (pAd->CommonCfg.dbdc_mode)
        {
            //if (wmm_set == 0)
            if (pTxBlk->Wcid == 1)
            {
                start = 3;
                end = pAd->vow_cloned_wtbl_num[0];
            }
            else
            {
                start = pAd->vow_cloned_wtbl_num[0] + 1;
                end = pAd->vow_cloned_wtbl_num[1];
            }
        }
        else
        {
            start = 2;
            end = pAd->vow_cloned_wtbl_num[0];
        }

        //for (i = 3; i <= pAd->vow_cloned_wtbl_num; i++)
        for (i = start; i <= end; i++)
        {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: sta%d, tx_en %d\n", 
                        __FUNCTION__, i, pAd->vow_tx_en[i]));

            if (pAd->vow_tx_en[i] && (GET_TXRING_FREENO(pAd, 0) > 2))
            {
                //clone packet
                PNDIS_PACKET clone = DuplicatePacket(wdev->if_dev, pkt);

                //restore TXBLK 
                os_move_mem(pTxBlk, &txb, sizeof(TX_BLK));
                //printk("%s: clone 2 pkt %p\n", __FUNCTION__, clone);
                pTxBlk->pPacket = clone;
                //pTxBlk->Wcid = 1;
                pTxBlk->Wcid = i;
                //modified DA
                if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
                {
	                HEADER_802_11 * hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	                hdr->Addr1[4] = i; 
                }

	            CutThroughPktTx(pAd, pTxBlk);
            }
        }
    }
    //RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
}
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */

static VOID APLegacyFrameTx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	QUEUE_ENTRY *pQEntry;

	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount++;
#endif /* STATS_COUNT_SUPPORT */

		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("<--%s(%d): ##########Fail#########\n",
			__FUNCTION__, __LINE__));
		return;
	}

	APFindCipherAlgorithm(pAd, pTxBlk);

	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
	{
		APBuildCommon802_11Header(pAd, pTxBlk);

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
				RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}
#endif /* SOFT_ENCRYPT */

		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		if (bVLANPkt) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}

		/* record these MCAST_TX frames for group key rekey */
		if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
		{
			INT	idx;

#ifdef STATS_COUNT_SUPPORT
			INC_COUNTER64(pAd->WlanCounters[0].MulticastTransmittedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

			for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			{
				if (pAd->ApCfg.MBSSID[idx].WPAREKEY.ReKeyMethod == PKT_REKEY)
				{
					pAd->ApCfg.MBSSID[idx].REKEYCOUNTER += (pTxBlk->SrcBufLen);
				}
#ifdef WAPI_SUPPORT
				if (pAd->ApCfg.MBSSID[idx].wdev.SecConfig.WapiMskRekeyTimerRunning &&
					pAd->ApCfg.MBSSID[idx].wdev.SecConfig.wapi_msk_rekey_method == REKEY_METHOD_PKT)
				{
					pAd->ApCfg.MBSSID[idx].wdev.SecConfig.wapi_msk_rekey_cnt += (pTxBlk->SrcBufLen);
				}
#endif /* WAPI_SUPPORT */

			}
		}
#ifdef MT_MAC
		else
		{
			/* Unicast */
			if (pTxBlk->tr_entry && pTxBlk->tr_entry->PsDeQWaitCnt)
				pTxBlk->Pid = PID_PS_DATA;
		}
#endif /* MT_MAC */

		pHeaderBufPtr = pTxBlk->wifi_hdr;
		wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;

		/* skip common header */
		pHeaderBufPtr += pTxBlk->wifi_hdr_len;

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
			struct wifi_dev *wdev = NULL;
			UCHAR ack_policy = pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx];
			wdev = pTxBlk->wdev;
			if(wdev){
				ack_policy = wlan_config_get_ack_policy(wdev,pTxBlk->QueIdx);
			}
			/* build QOS Control bytes */
			*pHeaderBufPtr = ((pTxBlk->UserPriority & 0x0F) | (ack_policy << 5));
#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
			*pHeaderBufPtr |= (pAd->vow_sta_ack[pTxBlk->Wcid] << 5);
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */

#ifdef WFA_VHT_PF
			if (pAd->force_noack)
				*pHeaderBufPtr |= (1 << 5);
#endif /* WFA_VHT_PF */

#ifdef UAPSD_SUPPORT
			if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
				&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
			)
			{
				/*
					we can not use bMoreData bit to get EOSP bit because
					maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
				 */
				if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
					*pHeaderBufPtr |= (1 << 4);
			}
#endif /* UAPSD_SUPPORT */

			*(pHeaderBufPtr + 1) = 0;
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += 2;
			pTxBlk->wifi_hdr_len += 2;

#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			if (pAd->chipCap.FlgHwTxBfCap &&
				(pTxBlk->pMacEntry) &&
				(pTxBlk->pTransmit->field.MODE >= MODE_HTMIX))
			{
				MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
				BOOLEAN bHTCPlus = FALSE;

				pTxBlk->TxSndgPkt = SNDG_TYPE_DISABLE;

				// TODO: shiang-usw, fix ME!!
				NdisAcquireSpinLock(&pMacEntry->TxSndgLock);
				if (pMacEntry->TxSndgType >= SNDG_TYPE_SOUNDING)
				{
					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));

					if (pMacEntry->TxSndgType == SNDG_TYPE_SOUNDING)
					{
						/* Select compress if supported. Otherwise select noncompress */
						if ((pAd->CommonCfg.ETxBfNoncompress==0) &&
							(pMacEntry->HTCapability.TxBFCap.ExpComBF>0))
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
							else
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

						/* Clear NDP Announcement */
						((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 0;

					}
					else if (pMacEntry->TxSndgType == SNDG_TYPE_NDP)
					{
						/* Select compress if supported. Otherwise select noncompress */
						if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
							(pMacEntry->HTCapability.TxBFCap.ExpComBF>0) &&
							(pMacEntry->HTCapability.TxBFCap.ComSteerBFAntSup >= (pMacEntry->sndgMcs/8))
						)
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
						else
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

						/* Set NDP Announcement */
						((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 1;

						pTxBlk->TxNDPSndgBW = pMacEntry->sndgBW;
						pTxBlk->TxNDPSndgMcs = pMacEntry->sndgMcs;
					}

					pTxBlk->TxSndgPkt = pMacEntry->TxSndgType;
					pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
					bHTCPlus = TRUE;
				}
				NdisReleaseSpinLock(&pMacEntry->TxSndgLock);

#ifdef MFB_SUPPORT
#if defined(MRQ_FORCE_TX)
				/* have to replace this by the correct condition!!! */
				pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_MRQ;
#endif

				/*
					Ignore sounding frame because the signal format of sounding frmae may
					be different from normal data frame, which may result in different MFB
				*/
				if ((pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ) &&
					(pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE))
				{
					if (bHTCPlus == FALSE)
					{
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}
					MFB_PerPareMRQ(pAd, pHeaderBufPtr, pMacEntry);
				}

				if (pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback >=MCSFBK_MRQ &&
					pMacEntry->toTxMfb == 1)
				{
					if (bHTCPlus == FALSE)
					{
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}
					MFB_PerPareMFB(pAd, pHeaderBufPtr, pMacEntry);/* not complete yet!!! */
					pMacEntry->toTxMfb = 0;
				}
#endif /* MFB_SUPPORT */

				if (bHTCPlus == TRUE)
				{
					/* mark HTC bit */
					wifi_hdr->FC.Order = 1;
					pHeaderBufPtr += 4;
					pTxBlk->wifi_hdr_len += 4;
				}
			}
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
		}

		/* The remaining content of MPDU header should locate at 4-octets aligment */
		pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
		pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
		pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
		pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
		}
		else
#endif /* SOFT_ENCRYPT */
		{

			/*
				Insert LLC-SNAP encapsulation - 8 octets
				if original Ethernet frame contains no LLC/SNAP,
				then an extra LLC/SNAP encap is required
			*/
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
								pTxBlk->pExtraLlcSnapEncap);
			if (pTxBlk->pExtraLlcSnapEncap) {
				UCHAR vlan_size;

				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
				pHeaderBufPtr += 6;
				/* skip vlan tag */
				vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
				/* get 2 octets (TypeofLen) */
				NdisMoveMemory(pHeaderBufPtr,
							pTxBlk->pSrcBufHeader + 12 + vlan_size,
							2);
				pHeaderBufPtr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		}
	}
	else
	{
		pTxBlk->MpduHeaderLen = 0;
		pTxBlk->HdrPadLen = 0;
		pTxBlk->wifi_hdr_len = 0;
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	}

#ifdef STATS_COUNT_SUPPORT
		/* calculate Tx count and ByteCount per BSS */
#ifdef WAPI_SUPPORT
	if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif /* WAPI_SUPPORT */
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;
		MAC_TABLE_ENTRY *pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
		if (pTxBlk->pMacEntry->SecConfig.WapiUskRekeyTimerRunning && pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
#endif /* WAPI_SUPPORT */

		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
            pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
		}
	}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

	/*
		prepare for TXWI
		use Wcid as Hardware Key Index
	*/

	/* update Hardware Group Key Index */
	if (!pTxBlk->pMacEntry) {
		struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
        UCHAR Wcid = 0xff;
		ASSERT(wdev != NULL);
		if (wdev)
        {
			GET_GroupKey_WCID(wdev, Wcid);
            pTxBlk->Wcid = Wcid;
        }
	}

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
#if defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA))
        vow_clone_legacy_frame(pAd, pTxBlk);
#else
		CutThroughPktTx(pAd, pTxBlk);
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
#endif /* defined(VOW_SUPPORT) && (defined(MT7615_FPGA) || defined(MT7622_FPGA)) */
	}
	else
#endif /* defined(MT7615) || defined(MT7622) */
	{
		if (write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk) == NDIS_STATUS_FAILURE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}

	//hex_dump("Legacy_Frame-FirstBufContent", pTxBlk->HeaderBuf, 128);
	//hex_dump("Legacy_Frame-FirstBufContent - WiFi Hdr Segment", pTxBlk->wifi_hdr, pTxBlk->wifi_hdr_len);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): pTxBlk->MpduHeaderLen=%d, wifi_hdr_len=%d, HdrPadLen=%d, hw_rsv_len=%d\n",
	//			__FUNCTION__, pTxBlk->MpduHeaderLen, pTxBlk->wifi_hdr_len, pTxBlk->HdrPadLen, pTxBlk->hw_rsv_len));
	//dump_tmac_info(pAd, &pTxBlk->HeaderBuf[pTxBlk->hw_rsv_len]);

	if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
		if (pTxBlk->tr_entry)
			pTxBlk->tr_entry->isCached = FALSE;

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &freeCnt);


#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)wifi_hdr);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

#ifdef USE_BMC
	if (pTxBlk->QueIdx == QID_BMC)
	{
		struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];

		HAL_KickOutTxBMC(pAd, pTxBlk, pTxBlk->QueIdx);
		switch (wdev->func_idx)
		{
			case 0:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR2, 0x1);
				break;
			case 1:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR2, 0x10000);
				break;
			case 2:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR2, 0x100000);
				break;
			case 3:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR2, 0x1000000);
				break;
			case 4:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR2, 0x10000000);
				break;
			case 5:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x1);
				break;
			case 6:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x10);
				break;
			case 7:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x100);
				break;
			case 8:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x1000);
				break;
			case 9:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x10000);
				break;
			case 10:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x100000);
				break;
			case 11:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x1000000);
				break;
			case 12:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR3, 0x10000000);
				break;
			case 13:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR4, 0x1);
				break;
			case 14:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR4, 0x10);
				break;
			case 15:
				RTMP_IO_WRITE32(pAd, ARB_BMCQCR4, 0x100);
				break;
			default:
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("<--%s(%d):no func_idx (%d)\n",
								__FUNCTION__, __LINE__, wdev->func_idx));
				break;
		}
	}
	else
#endif /* USE_BMC */
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
}
}


static VOID APFragmentFrameTx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	QUEUE_ENTRY *pQEntry;
	PACKET_INFO PacketInfo;
#ifdef SOFT_ENCRYPT
	UCHAR *tmp_ptr = NULL;
	UINT32 buf_offset = 0;
#endif /* SOFT_ENCRYPT */
	HTTRANSMIT_SETTING *pTransmit;
	UCHAR fragNum = 0;
	USHORT EncryptionOverhead = 0;
	UINT32 FreeMpduSize, SrcRemainingBytes;
	USHORT AckDuration;
	UINT NextMpduSize;

	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount++;
#endif /* STATS_COUNT_SUPPORT */

		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(%d): ##########Fail#########\n", __FUNCTION__, __LINE__));
		return;
	}

	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif /* SOFT_ENCRYPT */

	if (IS_CIPHER_TKIP(pTxBlk->CipherAlg))
	{
		pTxBlk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, pTxBlk->pPacket);
		if (pTxBlk->pPacket == NULL)
			return;
		RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	}

	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;

	/* skip vlan tag */
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	pHeaderBufPtr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Before Frag, pTxBlk->MpduHeaderLen=%d, wifi_hdr_len=%d, HdrPadLen=%d, hw_rsv_len=%d\n",
			__FUNCTION__, pTxBlk->MpduHeaderLen, pTxBlk->wifi_hdr_len, pTxBlk->HdrPadLen, pTxBlk->hw_rsv_len));

	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/* build QOS Control bytes */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
		if (pTxBlk->pMacEntry &&
			CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		)
		{
			/*
				we can not use bMoreData bit to get EOSP bit because
				maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif /* UAPSD_SUPPORT */

		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
		pTxBlk->wifi_hdr_len += 2;
	}

	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		UCHAR iv_offset = 0;

		/*
			If original Ethernet frame contains no LLC/SNAP,
			then an extra LLC/SNAP encap is required
		*/
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
						    pTxBlk->pExtraLlcSnapEncap);

		/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
		if (pTxBlk->pExtraLlcSnapEncap) {
			/* Reserve the front 8 bytes of data for LLC header */
			pTxBlk->pSrcBufData -= LENGTH_802_1_H;
			pTxBlk->SrcBufLen += LENGTH_802_1_H;

			NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);
		}

		/* Construct and insert specific IV header to MPDU header */
		RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
							   pTxBlk->KeyIdx,
							   pTxBlk->pKey->TxTsc,
							   pHeaderBufPtr, &iv_offset);
		pHeaderBufPtr += iv_offset;
		pTxBlk->MpduHeaderLen += iv_offset;

	}
	else
#endif /* SOFT_ENCRYPT */
	{

		/*
			Insert LLC-SNAP encapsulation - 8 octets
			if original Ethernet frame contains no LLC/SNAP,
			then an extra LLC/SNAP encap is required
		*/
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
							pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr,
						pTxBlk->pSrcBufHeader + 12 + vlan_size,
						2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}

	/*  1. If TKIP is used and fragmentation is required. Driver has to
		   append TKIP MIC at tail of the scatter buffer
		2. When TXWI->FRAG is set as 1 in TKIP mode,
		   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC */
	/*  TKIP appends the computed MIC to the MSDU data prior to fragmentation into MPDUs. */
	if (IS_CIPHER_TKIP(pTxBlk->CipherAlg))
	{
		struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
		ASSERT(wdev != NULL);
		if (wdev)
			RTMPCalculateMICValue(pAd, pTxBlk->pPacket, pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey, &pTxBlk->pKey[LEN_TK], wdev->func_idx);

		/*
			NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
				to correct lenght, refer to pTxBlk->SrcBufLen for the packet length in following progress.
		*/
		NdisMoveMemory(pTxBlk->pSrcBufData + pTxBlk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		pTxBlk->SrcBufLen += 8;
		pTxBlk->TotalFrameLen += 8;
	}

#ifdef STATS_COUNT_SUPPORT
	/* calculate Tx count and ByteCount per BSS */
#ifdef WAPI_SUPPORT
	if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif /* WAPI_SUPPORT */
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;
		MAC_TABLE_ENTRY *pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
		if (pTxBlk->pMacEntry->SecConfig.WapiUskRekeyTimerRunning && pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
#endif /* WAPI_SUPPORT */

		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
            pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
		}
	}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

	/*
		calcuate the overhead bytes that encryption algorithm may add. This
		affects the calculate of "duration" field
	*/
	if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128) || (pTxBlk->CipherAlg == CIPHER_WEP152))
		EncryptionOverhead = 8; /* WEP: IV[4] + ICV[4]; */
	else if (pTxBlk->CipherAlg == CIPHER_TKIP)
		EncryptionOverhead = 12; /* TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength */
	else if (pTxBlk->CipherAlg == CIPHER_AES)
		EncryptionOverhead = 16;	/* AES: IV[4] + EIV[4] + MIC[8] */
#ifdef WAPI_SUPPORT
	else if (pTxBlk->CipherAlg == CIPHER_SMS4)
		EncryptionOverhead = 16;	/* SMS4: MIC[16] */
#endif /* WAPI_SUPPORT */
	else
		EncryptionOverhead = 0;

	pTransmit = pTxBlk->pTransmit;
	/* Decide the TX rate */
	if (pTransmit->field.MODE == MODE_CCK)
		pTxBlk->TxRate = pTransmit->field.MCS;
	else if (pTransmit->field.MODE == MODE_OFDM)
		pTxBlk->TxRate = pTransmit->field.MCS + RATE_FIRST_OFDM_RATE;
	else
		pTxBlk->TxRate = RATE_6_5;

	/* decide how much time an ACK/CTS frame will consume in the air */
	if (pTxBlk->TxRate <= RATE_LAST_OFDM_RATE)
		AckDuration = RTMPCalcDuration(pAd, pAd->CommonCfg.ExpectedACKRate[pTxBlk->TxRate], 14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6_5, 14);
	/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("!!!Fragment AckDuration(%d), TxRate(%d)!!!\n", AckDuration, pTxBlk->TxRate)); */

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		/* store the outgoing frame for calculating MIC per fragmented frame */
		os_alloc_mem(pAd, (PUCHAR *)&tmp_ptr, pTxBlk->SrcBufLen);
		if (tmp_ptr == NULL) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():no memory for MIC calculation!\n",
										__FUNCTION__));
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
		NdisMoveMemory(tmp_ptr, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	}
#endif /* SOFT_ENCRYPT */

	/* Init the total payload length of this frame. */
	SrcRemainingBytes = pTxBlk->SrcBufLen;
	pTxBlk->TotalFragNum = 0xff;

	do {
		FreeMpduSize = wlan_operate_get_frag_thld(&pTxBlk->pMbss->wdev);
		FreeMpduSize -= LENGTH_CRC;
		FreeMpduSize -= pTxBlk->MpduHeaderLen;

		if (SrcRemainingBytes <= FreeMpduSize)
		{
			/* This is the last or only fragment */
			pTxBlk->SrcBufLen = SrcRemainingBytes;

			wifi_hdr->FC.MoreFrag = 0;
			wifi_hdr->Duration = pAd->CommonCfg.Dsifs + AckDuration;

			/* Indicate the lower layer that this's the last fragment. */
			pTxBlk->TotalFragNum = fragNum;
#ifdef MT_MAC
			pTxBlk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_NO : TX_FRAG_ID_LAST);
#endif /* MT_MAC */
		}
		else
		{	/* more fragment is required */
			pTxBlk->SrcBufLen = FreeMpduSize;

			NextMpduSize = min(((UINT)SrcRemainingBytes - pTxBlk->SrcBufLen),
								((UINT)wlan_operate_get_frag_thld(&pTxBlk->pMbss->wdev)));
			wifi_hdr->FC.MoreFrag = 1;
			wifi_hdr->Duration = (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) +
								RTMPCalcDuration(pAd, pTxBlk->TxRate, NextMpduSize + EncryptionOverhead);
#ifdef MT_MAC
			pTxBlk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_FIRST : TX_FRAG_ID_MIDDLE);
#endif /* MT_MAC */
		}

		SrcRemainingBytes -= pTxBlk->SrcBufLen;

		if (fragNum == 0)
			pTxBlk->FrameGap = IFS_HTTXOP;
		else
			pTxBlk->FrameGap = IFS_SIFS;

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		{
			UCHAR ext_offset = 0;

			NdisMoveMemory(pTxBlk->pSrcBufData, tmp_ptr + buf_offset, pTxBlk->SrcBufLen);
			buf_offset += pTxBlk->SrcBufLen;

			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd,
									pTxBlk->CipherAlg,
									(UCHAR *)wifi_hdr,
									pTxBlk->pSrcBufData,
									pTxBlk->SrcBufLen,
									pTxBlk->KeyIdx,
									   pTxBlk->pKey,
									 &ext_offset);
			pTxBlk->SrcBufLen += ext_offset;
			pTxBlk->TotalFrameLen += ext_offset;
		}
#endif /* SOFT_ENCRYPT */

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
		CutThroughPktTx(pAd, pTxBlk);
	}
	else
#endif /* defined(MT7615) || defined(MT7622) */
	{
		if (write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk) == NDIS_STATUS_FAILURE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}


		HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, &freeCnt);
	}
	
	pTxBlk->DmaMapping = FALSE;

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)wifi_hdr);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		{
#ifdef WAPI_SUPPORT
			if (pTxBlk->CipherAlg == CIPHER_SMS4)
			{
				/* incease WPI IV for next MPDU */
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WAPI_TSC, 2);
				/* Construct and insert WPI-SMS4 IV header to MPDU header */
				RTMPConstructWPIIVHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
								 pHeaderBufPtr - (LEN_WPI_IV_HDR));
			}
			else
#endif /* WAPI_SUPPORT */
			if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128))
			{
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
				/* Construct and insert 4-bytes WEP IV header to MPDU header */
				RTMPConstructWEPIVHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										pHeaderBufPtr - (LEN_WEP_IV_HDR));
			}
			else if (pTxBlk->CipherAlg == CIPHER_TKIP)
				;
			else if (pTxBlk->CipherAlg == CIPHER_AES)
			{
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
				/* Construct and insert 8-bytes CCMP header to MPDU header */
				RTMPConstructCCMPHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										pHeaderBufPtr - (LEN_CCMP_HDR));
			}
		}
		else
#endif /* SOFT_ENCRYPT */
		{
			/* Update the frame number, remaining size of the NDIS packet payload. */
			if (fragNum == 0 && pTxBlk->pExtraLlcSnapEncap)
				pTxBlk->MpduHeaderLen -= LENGTH_802_1_H;	/* space for 802.11 header. */
		}

		fragNum++;
		/* SrcRemainingBytes -= pTxBlk->SrcBufLen; */
		pTxBlk->pSrcBufData += pTxBlk->SrcBufLen;

		wifi_hdr->Frag++;	/* increase Frag # */

	} while (SrcRemainingBytes > 0);

#ifdef SOFT_ENCRYPT
	if (tmp_ptr != NULL)
		os_free_mem( tmp_ptr);
#endif /* SOFT_ENCRYPT */

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}


static VOID APARalinkFrameTx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *buf_ptr;
	USHORT freeCnt = 1;
	USHORT totalMPDUSize = 0;
	USHORT FirstTx, LastTxIdx;
	int frameNum = 0;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;

	ASSERT(pTxBlk);
	ASSERT((pTxBlk->TxPacketList.Number == 2));

	FirstTx = LastTxIdx = 0;  /* Is it ok init they as 0? */
	while (pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
			BSS_STRUCT *pMbss = pTxBlk->pMbss;

			if (pMbss != NULL)
				pMbss->TxDropCount++;
#endif /* STATS_COUNT_SUPPORT */

			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
		if (bVLANPkt) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}

		/*
			For first frame, we need to create:
				802.11 header + padding(optional) + RA-AGG-LEN + SNAP Header
			For second aggregated frame, we need create:
				the 802.3 header to headerBuf, because PCI will copy it to SDPtr0
		*/
		if (frameNum == 0)
		{
			buf_ptr = APBuildARalinkFrameHeader(pAd, pTxBlk);

			/*
				It's ok write the TxWI here, because the TxWI->TxWIMPDUByteCnt
				will be updated after final frame was handled.
			*/
			if (write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk) == NDIS_STATUS_FAILURE) {
				RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
				continue;
			}


			/* Insert LLC-SNAP encapsulation - 8 octets */
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
								pTxBlk->pExtraLlcSnapEncap);

			if (pTxBlk->pExtraLlcSnapEncap) {
				NdisMoveMemory(buf_ptr, pTxBlk->pExtraLlcSnapEncap, 6);
				buf_ptr += 6;

				/* get 2 octets (TypeofLen) */
				NdisMoveMemory(buf_ptr, pTxBlk->pSrcBufData - 2, 2);
				buf_ptr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		}
		else
		{
			buf_ptr = &pTxBlk->HeaderBuf[0];
			pTxBlk->MpduHeaderLen = 0;

			/*
				A-Ralink sub-sequent frame header is the same as 802.3 header.
					DA(6)+SA(6)+FrameType(2)
			*/
			NdisMoveMemory(buf_ptr, pTxBlk->pSrcBufHeader, 12);
			buf_ptr += 12;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(buf_ptr, pTxBlk->pSrcBufData - 2, 2);
			buf_ptr += 2;
			pTxBlk->MpduHeaderLen = ARALINK_SUBHEAD_LEN;
		}

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;

		if (frameNum == 0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);
		else
			LastTxIdx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);


#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), NULL);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		frameNum++;

		pAd->RalinkCounters.OneSecTxARalinkCnt++;
		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

#ifdef STATS_COUNT_SUPPORT
		/* calculate Tx count and ByteCount per BSS */
#ifdef WAPI_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif /* WAPI_SUPPORT */
		{
			BSS_STRUCT *pMbss = pTxBlk->pMbss;
			MAC_TABLE_ENTRY *pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
			if (pTxBlk->pMacEntry->SecConfig.WapiUskRekeyTimerRunning && pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_method == REKEY_METHOD_PKT)
				pTxBlk->pMacEntry->SecConfig.wapi_usk_rekey_cnt += totalMPDUSize;
#endif /* WAPI_SUPPORT */

			if (pMbss != NULL)
			{
				pMbss->TransmittedByteCount += totalMPDUSize;
				pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
				if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->mcPktsTx++;
				else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->bcPktsTx++;
				else
					pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
			}

			if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
			{
				INC_COUNTER64(pMacEntry->TxPackets);
				pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
                pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
			}

		}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */

#endif /* STATS_COUNT_SUPPORT */
	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

}


#ifdef VHT_TXBF_SUPPORT
static VOID APNdpaFrameTx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *buf;
	VHT_NDPA_FRAME *vht_ndpa;
	struct wifi_dev *wdev;
	UINT frm_len, sta_cnt;
	SNDING_STA_INFO *sta_info;
	MAC_TABLE_ENTRY *pMacEntry;

	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pTxBlk->pPacket);
	pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry)
	{
		wdev = pMacEntry->wdev;

		if (MlmeAllocateMemory(pAd, &buf) != NDIS_STATUS_SUCCESS)
			return;

		NdisZeroMemory(buf, MGMT_DMA_BUFFER_SIZE);

		vht_ndpa = (VHT_NDPA_FRAME *)buf;
		frm_len = sizeof(VHT_NDPA_FRAME);
		vht_ndpa->fc.Type = FC_TYPE_CNTL;
		vht_ndpa->fc.SubType = SUBTYPE_VHT_NDPA;
		COPY_MAC_ADDR(vht_ndpa->ra, pMacEntry->Addr);
		COPY_MAC_ADDR(vht_ndpa->ta, wdev->if_addr);

		/* Currnetly we only support 1 STA for a VHT DNPA */
		sta_info = vht_ndpa->sta_info;
		for (sta_cnt = 0; sta_cnt < 1; sta_cnt++) {
			sta_info->aid12 = pMacEntry->Aid;
			sta_info->fb_type = SNDING_FB_SU;
			sta_info->nc_idx = 0;
			vht_ndpa->token.token_num = pMacEntry->snd_dialog_token;
			frm_len += sizeof(SNDING_STA_INFO);
			sta_info++;
			if (frm_len >= (MGMT_DMA_BUFFER_SIZE - sizeof(SNDING_STA_INFO))) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): len(%d) too large!cnt=%d\n",
							__FUNCTION__, frm_len, sta_cnt));
				break;
			}
		}
		if (pMacEntry->snd_dialog_token & 0xc0)
			pMacEntry->snd_dialog_token = 0;
		else
			pMacEntry->snd_dialog_token++;

		vht_ndpa->duration = 100;

		//MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Send VHT NDPA Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
		//						PRINT_MAC(pMacEntry->Addr)));
		//hex_dump("VHT NDPA Frame", buf, frm_len);

		// NDPA's BW needs to sync with Tx BW
		wdev->rate.MlmeTransmit.field.BW = pMacEntry->HTPhyMode.field.BW;

		pTxBlk->Flags = FALSE; // No Acq Request

		// TODO: shiang-lock, fix ME!!
		MiniportMMRequest(pAd, 0, buf, frm_len);
		MlmeFreeMemory( buf);
	

		pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
	}
}
#endif /* VHT_TXBF_SUPPORT */


NDIS_STATUS APHardTransmit(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PQUEUE_ENTRY pQEntry;
	PNDIS_PACKET pPacket = NULL;
	struct wifi_dev *wdev = pTxBlk->wdev;
#ifdef VHT_TXBF_SUPPORT
	struct dev_rate_info *rate = &wdev->rate;
#endif /*VHT_TXBF_SUPPORT*/
	if(!wdev){
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}
//MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d)-->\n", __FUNCTION__, __LINE__));

	if ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
		)
	{
		while(pTxBlk->TxPacketList.Head)
		{
			pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
			pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
        }
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<--%s(%d)\n", __FUNCTION__, __LINE__));
		return NDIS_STATUS_FAILURE;
	}

	if (wdev->bVLAN_Tag == TRUE)
	{
		RTMP_SET_PACKET_VLAN(pTxBlk->pPacket, FALSE);
	}

	/*add hook point when dequeue*/
	RTMP_OS_TXRXHOOK_CALL(WLAN_TX_DEQUEUE,pPacket,pTxBlk->QueIdx,pAd);

#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
{
	if ((wdev->func_idx < pAd->ApCfg.BssidNum)
		&& IS_RRM_QUIET(pAd, wdev->func_idx))
	{
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}
}
#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

#ifdef HDR_TRANS_TX_SUPPORT
#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) /* need LLC, not yet generated */
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_HDR_TRANS);
	else
#endif /* SOFT_ENCRYPT */
#ifdef WDS_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry))
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_HDR_TRANS);
	else
#endif /* WDS_SUPPORT */
	{
#if defined(MT7615) || defined(MT7622)
		if ((IS_MT7615(pAd) || IS_MT7622(pAd)) &&
			(pTxBlk->TxFrameType == TX_LEGACY_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
		else
#endif /* defined(MT7615) || defined(MT7622) */
		if ((pTxBlk->TxFrameType != TX_MCAST_FRAME) && (pTxBlk->TxFrameType != TX_FRAG_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
		else
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_HDR_TRANS);
	}
#endif /* HDR_TRANS_TX_SUPPORT */

#ifdef VHT_TXBF_SUPPORT
	if ((pTxBlk->TxFrameType & TX_NDPA_FRAME) > 0)
	{
		UCHAR mlmeMCS, mlmeBW, mlmeMode;

		mlmeMCS  = rate->MlmeTransmit.field.MCS;
		mlmeBW   = rate->MlmeTransmit.field.BW;
		mlmeMode = rate->MlmeTransmit.field.MODE;

		pAd->NDPA_Request = TRUE;

		APNdpaFrameTx(pAd, pTxBlk);

		pAd->NDPA_Request = FALSE;
		pTxBlk->TxFrameType &= ~TX_NDPA_FRAME;

		// Finish NDPA and then recover to mlme's own setting
		rate->MlmeTransmit.field.MCS  = mlmeMCS;
		rate->MlmeTransmit.field.BW   = mlmeBW;
		rate->MlmeTransmit.field.MODE = mlmeMode;
	}
#endif

//printk("%s(): TxFrameType=%d\n", __FUNCTION__, pTxBlk->TxFrameType);

	switch (pTxBlk->TxFrameType)
	{
#ifdef DOT11_N_SUPPORT
		case TX_AMPDU_FRAME:
			APAmpduFrameTx(pAd, pTxBlk);
			break;
#endif /* DOT11_N_SUPPORT */
		case TX_LEGACY_FRAME:
			APLegacyFrameTx(pAd, pTxBlk);
			break;
		case TX_MCAST_FRAME:
			APLegacyFrameTx(pAd, pTxBlk);
			break;
#ifdef DOT11_N_SUPPORT
		case TX_AMSDU_FRAME:
			APAmsduFrameTx(pAd, pTxBlk);
			break;
#endif /* DOT11_N_SUPPORT */
		case TX_RALINK_FRAME:
			APARalinkFrameTx(pAd, pTxBlk);
			break;
		case TX_FRAG_FRAME:
			APFragmentFrameTx(pAd, pTxBlk);
			break;
#if defined(MT7615) || defined(MT7622)
#ifdef CUT_THROUGH
        case TX_OFFLOAD_FRAME:
		printk("%s(): Call CutThroughPktTx for Transmission!\n", __FUNCTION__);
                break;
#endif /* CUT_THROUGH */
#endif /* defined(MT7615) || defined(MT7622) */

		default:
			{
				/* It should not happened! */
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Send a pacekt was not classified!!\n"));
				while (pTxBlk->TxPacketList.Head) {
					pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
					pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
					if (pPacket)
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
			}
			break;
	}

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<--%s(%d)\n", __FUNCTION__, __LINE__));

	return (NDIS_STATUS_SUCCESS);
}


/*
  ========================================================================
  Description:
	This routine checks if a received frame causes class 2 or class 3
	error, and perform error action (DEAUTH or DISASSOC) accordingly
  ========================================================================
*/
BOOLEAN APChkCls2Cls3Err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	/* software MAC table might be smaller than ASIC on-chip total size. */
	/* If no mathed wcid index in ASIC on chip, do we need more check???  need to check again. 06-06-2006 */
	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
	{
		MAC_TABLE_ENTRY *pEntry;

		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
		if (pEntry)
			return FALSE;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Rx a frame from %02x:%02x:%02x:%02x:%02x:%02x with WCID(%d) > %d\n",
					__FUNCTION__, PRINT_MAC(pRxBlk->Addr2),
					pRxBlk->wcid, GET_MAX_UCAST_NUM(pAd)));

		APCls2errAction(pAd, pRxBlk);
		return TRUE;
	}

	if (pAd->MacTab.Content[pRxBlk->wcid].Sst == SST_ASSOC)
		; /* okay to receive this DATA frame */
	else if (pAd->MacTab.Content[pRxBlk->wcid].Sst == SST_AUTH)
	{
		APCls3errAction(pAd, pRxBlk);
		return TRUE;
	}
	else
	{
		APCls2errAction(pAd, pRxBlk);
		return TRUE;
	}
	return FALSE;
}


/*
	detect AC Category of trasmitting packets
	to turn AC0(BE) TX_OP (MAC reg 0x1300)
*/
// TODO: shiang-usw, this function should move to other place!!
VOID detect_wmm_traffic(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR UserPriority,
	IN UCHAR FlgIsOutput)
{
	if (!pAd)
		return;
	/* For BE & BK case and TxBurst function is disabled */
	if ((pAd->CommonCfg.bEnableTxBurst == FALSE)
#ifdef DOT11_N_SUPPORT
		&& (pAd->CommonCfg.bRdg == FALSE)
		&& (pAd->CommonCfg.bRalinkBurstMode == FALSE)
#endif /* DOT11_N_SUPPORT */
		&& (FlgIsOutput == 1)
	)
	{
		if (WMM_UP2AC_MAP[UserPriority] == QID_AC_BK)
		{
			/* has any BK traffic */
			if (pAd->flg_be_adjust == 0)
			{
				/* yet adjust */
                //TODO: here need YF check!
				//RTMP_SET_TX_BURST(pAd, wdev);
				pAd->flg_be_adjust = 1;
				NdisGetSystemUpTime(&pAd->be_adjust_last_time);

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("wmm> adjust be!\n"));
			}
		}
		else
		{
			if (pAd->flg_be_adjust != 0)
			{
				QUEUE_HEADER *pQueue;

				/* has adjusted */
				pQueue = &pAd->TxSwQueue[QID_AC_BK];

				if ((pQueue == NULL) ||
					((pQueue != NULL) && (pQueue->Head == NULL)))
				{
					ULONG	now;
					NdisGetSystemUpTime(&now);
					if ((now - pAd->be_adjust_last_time) > TIME_ONE_SECOND)
					{
						/* no any BK traffic */
                        //TODO: here need YF check!
                        //RTMP_SET_TX_BURST(pAd, wdev);

						pAd->flg_be_adjust = 0;

						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("wmm> recover be!\n"));
					}
				}
				else
					NdisGetSystemUpTime(&pAd->be_adjust_last_time);
			}
		}
	}

	/* count packets which priority is more than BE */
	if (UserPriority > 3)
	{
		pAd->OneSecondnonBEpackets++;
		if (pAd->OneSecondnonBEpackets > 100
#ifdef DOT11_N_SUPPORT
			&& pAd->MacTab.fAnyStationMIMOPSDynamic
#endif /* DOT11_N_SUPPORT */
		)
		{
			if (!pAd->is_on)
			{
				RTMP_AP_ADJUST_EXP_ACK_TIME(pAd);
				pAd->is_on = 1;
			}
		}
		else
		{
			if (pAd->is_on)
			{
				RTMP_AP_RECOVER_EXP_ACK_TIME(pAd);
				pAd->is_on = 0;
			}
		}
	}
}


VOID APRxErrorHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;

	if (pRxInfo->CipherErr) {
		INC_COUNTER64(pAd->WlanCounters[0].WEPUndecryptableCount);

		if ((pRxInfo->U2M) && VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
#ifdef APCLI_SUPPORT
#if defined(APCLI_CERT_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
			UCHAR Wcid;

			Wcid = pRxBlk->wcid;
			if (VALID_UCAST_ENTRY_WCID(pAd, Wcid))
				pEntry = ApCliTableLookUpByWcid(pAd, Wcid, pRxBlk->Addr2);
			else
				pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

			if (pEntry &&
				(IS_ENTRY_APCLI(pEntry)
#ifdef MAC_REPEATER_SUPPORT
				|| IS_ENTRY_REPEATER(pEntry)
#endif /* MAC_REPEATER_SUPPORT */
				))
			{
				if ((IS_CIPHER_TKIP_Entry(pEntry)) 
					&& (pRxInfo->CipherErr == 2)
					&& !(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH)))
				{
					ApCliRTMPReportMicError(pAd, 1, pEntry->func_tb_idx);	

					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Rx MIC Value error\n"));
				}
			}
			else
#endif /* defined(APCLI_CERT_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT) */
#endif /* APCLI_SUPPORT */
			{
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				/*
					MIC error
					Before verifying the MIC, the receiver shall check FCS, ICV and TSC.
					This avoids unnecessary MIC failure events.
				*/
				if ((IS_CIPHER_TKIP_Entry(pEntry))
					&& (pRxInfo->CipherErr == 2)
					&& !(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH)))
				{
#ifdef HOSTAPD_SUPPORT
					if(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Hostapd == Hostapd_EXT)
						ieee80211_notify_michael_failure(pAd, pRxBlk->pHeader, (UINT32)pRxBlk->key_idx, 0);
			      		else
#endif/*HOSTAPD_SUPPORT*/
		      			{
		      				RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
		      			}
				}

				/* send wireless event - for icv error */
				if ((pRxInfo->CipherErr & 1) == 1)
					RTMPSendWirelessEvent(pAd, IW_ICV_ERROR_EVENT_FLAG, pEntry->Addr, 0, 0);
			}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rx u2me Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n",
					pRxBlk->MPDUtotalByteCnt, pRxBlk->wcid, pRxInfo->CipherErr));

	}
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		else if (pRxInfo->Mcast || pRxInfo->Bcast)
		{
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

			if (pEntry &&
				(IS_ENTRY_APCLI(pEntry)
#ifdef MAC_REPEATER_SUPPORT
				|| IS_ENTRY_REPEATER(pEntry)
#endif /* MAC_REPEATER_SUPPORT */
				)) {
				if ((pRxInfo->CipherErr == 2)
					&& !(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH)))
				{
					ApCliRTMPReportMicError(pAd, 0, pEntry->func_tb_idx);	
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Rx MIC Value error\n"));
				}
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rx bc/mc Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n",
					pRxBlk->MPDUtotalByteCnt, pRxBlk->wcid, pRxInfo->CipherErr));
		}
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */
	}
}

#ifdef RLT_MAC_DBG
static int dump_next_valid = 0;
#endif /* RLT_MAC_DBG */
BOOLEAN APCheckVaildDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN isVaild = FALSE;

	do
	{
		if (FC->ToDs == 0)
			break;

#ifdef IDS_SUPPORT
		if ((FC->FrDs == 0) && (pRxBlk->wcid == RESERVED_WCID)) /* not in RX WCID MAC table */
		{
			if (++pAd->ApCfg.RcvdMaliciousDataCount > pAd->ApCfg.DataFloodThreshold)
				break;
		}
#endif /* IDS_SUPPORT */

		/* check if Class2 or 3 error */
		if ((FC->FrDs == 0) && (APChkCls2Cls3Err(pAd, pRxBlk)))
			break;

//+++Add by shiang for debug
#ifdef RLT_MAC_DBG
		if (pAd->chipCap.hif_type == HIF_RLT) {
			if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
				MAC_TABLE_ENTRY *pEntry = NULL;

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("ErrWcidPkt: seq=%d\n", pHeader->Sequence));
				pEntry = MacTableLookup(pAd, pHeader->Addr2);
				if (pEntry && (pEntry->Sst == SST_ASSOC) && IS_ENTRY_CLIENT(pEntry))
					pRxBlk->wcid = pEntry->wcid;

				dump_next_valid = 1;
			}
			else if (dump_next_valid)
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("NextValidWcidPkt: seq=%d\n", pHeader->Sequence));
				dump_next_valid = 0;
			}
		}
#endif /* RLT_MAC_DBG */
//---Add by shiang for debug

		if(pAd->ApCfg.BANClass3Data == TRUE)
			break;

		isVaild = TRUE;
	} while (0);

	return isVaild;
}


INT APRxPktAllow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = 0;

	pEntry = PACInquiry(pAd, pRxBlk->wcid);

#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(MWDS)
	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
	{
#ifdef CLIENT_WDS
		if (pEntry) {
			/* The CLIENT WDS must be a associated STA */
			if (IS_ENTRY_CLIWDS(pEntry))
				;
			else if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
				SET_ENTRY_CLIWDS(pEntry);
			else
				return FALSE;

			CliWds_ProxyTabUpdate(pAd, pEntry->Aid, pRxBlk->Addr4);
		}
#endif /* CLIENT_WDS */
#ifdef MWDS
		if(!pEntry)
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
        
		if(pEntry && IS_MWDS_OPMODE_AP(pEntry))
        {
            MAC_TABLE_ENTRY *pMovedEntry = NULL;
            UINT16 ProtoType = 0;
            UINT32 ARPSenderIP = 0;
            UCHAR *Pos = (pRxBlk->pData + 12);
            BOOLEAN bTAMatchSA = MAC_ADDR_EQUAL(pEntry->Addr, pRxBlk->Addr4);

	    	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			//    ("APRxPktAllow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x Recvd MWDS Pkt\n",
			//    pEntry->wdev->wdev_idx,pEntry->wdev->wdev_type,pEntry->wdev->func_idx);

			/*if((((PUCHAR)pRxBlk->pData)[4])& 0x1 == 0x1)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("APRxPktAllow: MWDS Pkt=> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x \nEth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
				pEntry->wdev->wdev_idx,pEntry->wdev->wdev_type,pEntry->wdev->func_idx,
				((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],
				((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13],((PUCHAR)pRxBlk->pData)[14],((PUCHAR)pRxBlk->pData)[15],
				((PUCHAR)pRxBlk->pData)[16],((PUCHAR)pRxBlk->pData)[17]));
			}*/

            ProtoType = OS_NTOHS(*((UINT16*)Pos));
            if(ProtoType == 0x0806) /* ETH_P_ARP */
                NdisCopyMemory(&ARPSenderIP, (Pos + 16), 4);

            /* 
                           It means this source entry has moved to another one and hidden behind it. 
                           So delete this source entry! 
                    */
            if(!bTAMatchSA) /* TA isn't same with SA case*/
            {
                pMovedEntry = MacTableLookup(pAd, pRxBlk->Addr4);
                if(pMovedEntry
#ifdef AIR_MONITOR
                    && !IS_ENTRY_MONITOR(pMovedEntry)
#endif /* AIR_MONITOR */
                    && IS_ENTRY_CLIENT(pMovedEntry)
                    )
                {
                     MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                        ("APRxPktAllow: AP found a entry(%02X:%02X:%02X:%02X:%02X:%02X) who has moved to another side! Delete it from MAC table.\n",
                        PRINT_MAC(pMovedEntry->Addr)));
                     
#ifdef WH_EVENT_NOTIFIER
                    {
                        EventHdlr pEventHdlrHook = NULL;
                        pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
                        if(pEventHdlrHook && pMovedEntry->wdev)
                            pEventHdlrHook(pAd, pMovedEntry->wdev, pMovedEntry->Addr, pMovedEntry->wdev->channel);
                    }
#endif /* WH_EVENT_NOTIFIER */
                    MacTableDeleteEntry(pAd, pMovedEntry->wcid, pMovedEntry->Addr);
                }
            }
			MWDSProxyTabUpdate(pAd, pEntry->func_tb_idx, pEntry->wcid, pRxBlk->Addr4, ARPSenderIP);
        }
        else
            pEntry = NULL;
#endif /* MWDS */

#ifdef WDS_SUPPORT
		if (!pEntry)
		{
			/*
				The WDS frame only can go here when in auto learning mode and
				this is the first trigger frame from peer

				So we check if this is un-registered WDS entry by call function
					"FindWdsEntry()"
			*/
			if (MAC_ADDR_EQUAL(pRxBlk->Addr1, pAd->CurrentAddress))
				pEntry = FindWdsEntry(pAd, pRxBlk);

			/* have no valid wds entry exist, then discard the incoming packet.*/
			if (!(pEntry && WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx)))
				return FALSE;

			/*receive corresponding WDS packet, disable TX lock state (fix WDS jam issue) */
			if(pEntry && (pEntry->LockEntryTx == TRUE))
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Receive WDS packet, disable TX lock state!\n"));
				pEntry->ContinueTxFailCnt = 0;
				pEntry->LockEntryTx = FALSE;
				// TODO: shiang-usw, remove upper setting because we need to mirgate to tr_entry!
				pAd->MacTab.tr_entry[pEntry->wcid].ContinueTxFailCnt = 0;
				pAd->MacTab.tr_entry[pEntry->wcid].LockEntryTx = FALSE;
			}
		}
#endif /* WDS_SUPPORT */

		if (pEntry)
		{
#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
			RT_802_11_WDS_ENTRY *pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

			pWdsEntry->WdsCounter.ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			INC_COUNTER64(pWdsEntry->WdsCounter.ReceivedFragmentCount);

			if(IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
				INC_COUNTER64(pWdsEntry->WdsCounter.MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
			return hdr_len;
		}

		return FALSE;
	}
#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(MWDS) */

	if (!pEntry) {
#ifdef IDS_SUPPORT
		if ((pFmeCtrl->FrDs == 0) && (pRxBlk->wcid == RESERVED_WCID)) /* not in RX WCID MAC table */
			pAd->ApCfg.RcvdMaliciousDataCount++;
#endif /* IDS_SUPPORT */
        printk("%s(): pEntry is NULL!\n", __FUNCTION__);
		return FALSE;
	}

	if (!((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
#ifdef IDS_SUPPORT
		/*
			Replay attack detection,
			drop it if detect a spoofed data frame from a rogue AP
		*/
		if (pFmeCtrl->FrDs == 1)
			RTMPReplayAttackDetection(pAd, pRxBlk->Addr2, pRxBlk);
#endif /* IDS_SUPPORT */

		return FALSE;
	}

#ifdef MWDS
    if (((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
		if((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL)) {
        	if(pEntry && IS_MWDS_OPMODE_AP(pEntry)){
            	return FALSE;
        	}
			else{
				/*if((((PUCHAR)pRxBlk->pData)[4])& 0x1 == 0x1)
				{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("APRxPktAllow: Non MWDS Pkt=> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x \nEth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
				pEntry->wdev->wdev_idx,pEntry->wdev->wdev_type,pEntry->wdev->func_idx,
				((PUCHAR)pRxBlk->pData)[0],((PUCHAR)pRxBlk->pData)[1],((PUCHAR)pRxBlk->pData)[2],((PUCHAR)pRxBlk->pData)[3],((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],
				((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],
				((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13]));
				}*/
			}
		}
	}
#endif /* MWDS */

	/* check if Class2 or 3 error */
	if (APChkCls2Cls3Err(pAd, pRxBlk)) {
		return FALSE;
	}

#ifdef RLT_MAC_DBG
	if (pAd->chipCap.hif_type == HIF_RLT) {
		if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("ErrWcidPkt: seq=%d\n", pRxBlk->SN));
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
			if (pEntry && (pEntry->Sst == SST_ASSOC) && IS_ENTRY_CLIENT(pEntry))
				pRxBlk->wcid = pEntry->wcid;

			dump_next_valid = 1;
		}
		else if (dump_next_valid)
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("NextValidWcidPkt: seq=%d\n", pRxBlk->SN));
			dump_next_valid = 0;
		}
	}
#endif /* RLT_MAC_DBG */
//---Add by shiang for debug

	if(pAd->ApCfg.BANClass3Data == TRUE) {
        printk("%s(): BanClass3Data\n", __FUNCTION__);
		return FALSE;
	}

#ifdef STATS_COUNT_SUPPORT
	/* Increase received byte counter per BSS */
	if (pFmeCtrl->FrDs == 0 && pRxInfo->U2M)
	{
		BSS_STRUCT *pMbss = pEntry->pMbss;
		if (pMbss != NULL)
		{
			pMbss->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			pMbss->RxCount ++;
		}
	}

	/* update multicast counter */
        if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
                INC_COUNTER64(pAd->WlanCounters[0].MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

 #ifdef WH_EVENT_NOTIFIER
    if(pEntry && IS_ENTRY_CLIENT(pEntry)
 #ifdef MWDS
       && !IS_MWDS_OPMODE_AP(pEntry)
 #endif /* MWDS */
       && ((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL))
        )
        pEntry->rx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */  

	hdr_len = LENGTH_802_11;
	RX_BLK_SET_FLAG(pRxBlk, fRX_STA);

	ASSERT(pEntry->wcid == pRxBlk->wcid);

//printk("%s(): hdr_len=%d\n", __FUNCTION__, hdr_len);
	return hdr_len;
}


INT APRxPsHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR OldPwrMgmt = PWR_ACTIVE; /* 1: PWR_SAVE, 0: PWR_ACTIVE */

   	/* 1: PWR_SAVE, 0: PWR_ACTIVE */
   	OldPwrMgmt = RtmpPsIndicate(pAd, pRxBlk->Addr2, pEntry->wcid, FC->PwrMgmt);
#ifdef UAPSD_SUPPORT
	if ((FC->PwrMgmt == PWR_SAVE) &&
		(OldPwrMgmt == PWR_SAVE) &&
		(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
		(FC->SubType & 0x08))
	   	{
			/*
				In IEEE802.11e, 11.2.1.4 Power management with APSD,
				If there is no unscheduled SP in progress, the unscheduled SP begins
				when the QAP receives a trigger frame from a non-AP QSTA, which is a
				QoS data or QoS Null frame associated with an AC the STA has
				configured to be trigger-enabled.

				In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
				to/from Power Save Mode is not considered to be a Trigger Frame and
				the AP shall not respond with a QoS Null frame.
			*/
			/* Trigger frame must be QoS data or QoS Null frame */
	   		UCHAR  OldUP;

			// TODO: shiang-usw, check this!!!
#ifdef HDR_TRANS_SUPPORT
			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			{
				// TODO: different chip has different position @20130129. (+32 is for MT7650)
				OldUP = (*(pData+32) & 0x07);
			}
			else
#endif /* HDR_TRANS_SUPPORT */
				OldUP = (*(pRxBlk->pData+LENGTH_802_11) & 0x07);
#ifdef MT_PS
		if (pEntry->i_psm == I_PSM_DISABLE)
                {
			MtSetIgnorePsm(pAd, pEntry, I_PSM_ENABLE);
                }
#endif /* MT_PS */
		UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
    }
#endif /* UAPSD_SUPPORT */

	return TRUE;
}


INT APRxFowardHandle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	BOOLEAN to_os, to_air;
	UCHAR *pHeader802_3;
	PNDIS_PACKET pForwardPacket;
	BSS_STRUCT *pMbss;
	struct wifi_dev *dst_wdev = NULL;
    INT Ret;
    UCHAR wcid;
#ifdef MWDS
    UCHAR *pSrcAddr = NULL;
#endif /* MWDS */
 
	if (wdev->func_idx >= HW_BEACON_MAX_NUM)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid func_idx(%d), type(%d)!\n",
					__FUNCTION__, wdev->func_idx, wdev->wdev_type));
		return FALSE;
	}

	/* only one connected sta, directly to upper layer */
	if (pAd->MacTab.Size <= 1)
		return TRUE;

	// TODO: shiang-usw, remove pMbss structure here to make it more generic!
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	pHeader802_3 = GET_OS_PKT_DATAPTR(pPacket);

	/* by default, announce this pkt to upper layer (bridge) and not to air */
	to_os = TRUE;
	to_air = FALSE;

	if (pHeader802_3[0] & 0x01)
	{
		if ((pMbss->StaCount > 1)
		) {
			/* forward the M/Bcast packet back to air if connected STA > 1 */
			to_air = TRUE;
		}
	}
	else
	{
		/* if destinated STA is a associated wireless STA */
		pEntry = MacTableLookup(pAd, pHeader802_3);
		if (pEntry && pEntry->Sst == SST_ASSOC && pEntry->wdev)
		{
			dst_wdev = pEntry->wdev;
			if (wdev == dst_wdev)
			{
				/*
					STAs in same SSID, default send to air and not to os,
					but not to air if match following case:
						a). pMbss->IsolateInterStaTraffic == TRUE
				*/
				to_air = TRUE;
				to_os = FALSE;
				if (pMbss->IsolateInterStaTraffic == 1)
					to_air = FALSE;
			}
			else
			{
				/*
					STAs in different SSID, default send to os and not to air
					but not to os if match any of following cases:
						a). destination VLAN ID != source VLAN ID
						b). pAd->ApCfg.IsolateInterStaTrafficBTNBSSID
				*/
				to_os = TRUE;
				to_air = FALSE;
				if (pAd->ApCfg.IsolateInterStaTrafficBTNBSSID == 1 ||
					(wdev->VLAN_VID != dst_wdev->VLAN_VID))
					to_os = FALSE;
			}
#ifdef WH_EVENT_NOTIFIER
            if(to_air && IS_ENTRY_CLIENT(pEntry)
#ifdef MWDS
               && !IS_MWDS_OPMODE_AP(pEntry)
#endif /* MWDS */
               )
                 pEntry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */
		}
#ifdef MWDS
		else if(!pEntry && MWDSProxyLookup(pAd, wdev->func_idx, pHeader802_3, FALSE, &wcid))
		{
			if (VALID_WCID(wcid))
				pEntry = &pAd->MacTab.Content[wcid];
            
			if(pEntry && 
               (pEntry->Sst == SST_ASSOC) && 
                pEntry->wdev && 
                IS_MWDS_OPMODE_AP(pEntry))
            {
                to_os = FALSE;
				to_air = TRUE;
                dst_wdev = pEntry->wdev;
            }
		}
#endif /* MWDS */	
	}

	if (to_air)
	{
		pForwardPacket = DuplicatePacket(wdev->if_dev, pPacket);
#ifdef RTMP_UDMA_SUPPORT
		if(to_os)
		{
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
			if (wf_fwd_needed_hook != NULL && wf_fwd_needed_hook() == TRUE)
				set_wf_fwd_cb(pAd, pPacket, wdev);
#endif /* CONFIG_WIFI_PKT_FWD */
			announce_802_3_packet(pAd, pPacket,pAd->OpMode);
		}	
#endif
		if (pForwardPacket == NULL)
			return to_os;

		/* 1.1 apidx != 0, then we need set packet mbssid attribute. */
		if (pEntry) {
			wcid = pEntry->wcid;
			RTMP_SET_PACKET_WDEV(pForwardPacket, dst_wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		}
		else /* send bc/mc frame back to the same bss */
		{
			wcid = wdev->tr_tb_idx;
			RTMP_SET_PACKET_WDEV(pForwardPacket, wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		}

		RTMP_SET_PACKET_MOREDATA(pForwardPacket, FALSE);

#ifdef RT_CFG80211_P2P_SUPPORT
		RTMP_SET_PACKET_OPMODE(pForwardPacket, OPMODE_AP);
#endif /* RT_CFG80211_P2P_SUPPORT */


		if (wdev->tx_pkt_ct_handle && !check_if_fragment(wdev, pForwardPacket)) {
			UCHAR que_idx = 0, user_prio = QID_AC_BE;
			STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];

			if (!pAd->chipCap.BATriggerOffload)
			{
				if (!RTMPCheckEtherType(pAd, pForwardPacket, tr_entry, wdev, &user_prio, &que_idx)) 
				{
					RELEASE_NDIS_PACKET(pAd, pForwardPacket, NDIS_STATUS_FAILURE);
					return to_os;
				}
			}
#ifdef WH_EZ_SETUP
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("APRxFowardHandle: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x\nEth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
			//	wdev->wdev_idx,wdev->wdev_type,wdev->func_idx,
			//	((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[0],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[1],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[2],
			//	((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[3],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[4],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[5],
			//	((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[6],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[7],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[8],
			//	((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[9],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[10],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[11],
			//	((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[12],((PUCHAR)GET_OS_PKT_DATAPTR(pForwardPacket))[13]));
#endif
			Ret = wdev->tx_pkt_ct_handle(pAd, pForwardPacket, que_idx, user_prio);
#ifdef MWDS
			/* send bc/mc frame back to the same bss */
			if ((pHeader802_3[0] & 0x01))
			{
			    pSrcAddr = pHeader802_3 + MAC_ADDR_LEN;
			    if(ISMWDSValid(pAd, wdev->func_idx))
			    	MWDSSendClonePacket(pAd, wdev->func_idx, pPacket, pSrcAddr);
			}
#endif /* MWDS */
		} else if(wdev->tx_pkt_handle) {
#ifdef REDUCE_TCP_ACK_SUPPORT
			ReduceAckUpdateDataCnx(pAd, pForwardPacket);
			if (ReduceTcpAck(pAd, pForwardPacket) == FALSE)
#endif
			{
#ifndef MWDS
				wdev->tx_pkt_handle(pAd, pForwardPacket);
#else
				Ret = wdev->tx_pkt_handle(pAd, pForwardPacket);
				/* send bc/mc frame back to the same bss */
				if ((pHeader802_3[0] & 0x01) && (Ret == NDIS_STATUS_SUCCESS))
				{
				    pSrcAddr = pHeader802_3 + MAC_ADDR_LEN;
			        if(ISMWDSValid(pAd, wdev->func_idx))
				   		 MWDSSendClonePacket(pAd, wdev->func_idx, pPacket, pSrcAddr);
				}
#endif /* MWDS */
			}
		
			RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
		}
	}
#ifdef RTMP_UDMA_SUPPORT
	if(to_os == FALSE)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): No need to send to OS!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
	}
	return (to_os & (!to_air));
#endif

	return to_os;
}


/*
	========================================================================
	Routine Description:
		This routine is used to do insert packet into power-saveing queue.

	Arguments:
		pAd: Pointer to our adapter
		pPacket: Pointer to send packet
		pMacEntry: portint to entry of MacTab. the pMacEntry store attribute of client (STA).
		QueIdx: Priority queue idex.

	Return Value:
		NDIS_STATUS_SUCCESS:If succes to queue the packet into TxSwQ.
		NDIS_STATUS_FAILURE: If failed to do en-queue.
========================================================================
*/
NDIS_STATUS APInsertPsQueue(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN STA_TR_ENTRY *tr_entry,
	IN UCHAR QueIdx)
{
	return NDIS_STATUS_SUCCESS;
}

