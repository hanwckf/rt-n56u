#include "rt_config.h"


#ifdef HDR_TRANS_SUPPORT

#ifdef CONFIG_STA_SUPPORT
VOID STABuildWifiInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	TX_WIFI_INFO *pWI;
#ifdef QOS_DLS_SUPPORT
	BOOLEAN bDLSFrame = FALSE;
	INT DlsEntryIndex = 0;
#endif /* QOS_DLS_SUPPORT */
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;
	
	pTxBlk->MpduHeaderLen = TX_WIFI_INFO_SIZE;

	pWI = (TX_WIFI_INFO *) & pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];

	NdisZeroMemory(pWI, TX_WIFI_INFO_SIZE);

	pWI->field.QoS = (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? 1 : 0;

#ifdef QOS_DLS_SUPPORT
	if (INFRA_ON(pAd)) {
		/* Check if the frame can be sent through DLS direct link interface */
		/* If packet can be sent through DLS, then force aggregation disable. (Hard to determine peer STA's capability) */
		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
	}
#endif /* QOS_DLS_SUPPORT */

	if (tr_entry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
			pWI->field.Seq_Num = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
		} else {
			pWI->field.Seq_Num = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq + 1) & MAXSEQ;
		}
	} else {
		pWI->field.Seq_Num = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */
	}

	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	if (pAd->StaCfg.BssType == BSS_INFRA) {
#ifdef QOS_DLS_SUPPORT
		if (bDLSFrame) 
			pWI->field.Mode = 0;	/* IBSS */
		else
#endif /* QOS_DLS_SUPPORT */
			pWI->field.Mode = 2; /* STA*/
	} else if (ADHOC_ON(pAd)) {
		pWI->field.Mode = 0; /* IBSS */
	}

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		pWI->field.WEP = 1;

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		pWI->field.PS = PWR_SAVE;
	else
	{
		pWI->field.PS = (pAd->StaCfg.PwrMgmt.Psm == PWR_SAVE);
	}
}


VOID STABuildCacheWifiInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pWiInfo)
{
	TX_WIFI_INFO *pWI;
	STA_TR_ENTRY *tr_entry;

	pWI = (TX_WIFI_INFO *)pWiInfo;
	tr_entry = pTxBlk->tr_entry;

	pTxBlk->MpduHeaderLen = TX_WIFI_INFO_SIZE;

	/* More Bit */
	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	/* Sequence */
	pWI->field.Seq_Num = tr_entry->TxSeq[pTxBlk->UserPriority];
	tr_entry->TxSeq[pTxBlk->UserPriority] =
	    (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;

	{
		/* Check if the frame can be sent through DLS direct link interface
		   If packet can be sent through DLS, then force aggregation disable. (Hard to determine peer STA's capability) */
#ifdef QOS_DLS_SUPPORT
		BOOLEAN bDLSFrame = FALSE;
		INT DlsEntryIndex = 0;

		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
#endif /* QOS_DLS_SUPPORT */

		/* The addr3 of normal packet send from DS is Dest Mac address. */
#ifdef QOS_DLS_SUPPORT
		if (bDLSFrame) {
			pWI->field.Mode = 0;	/* IBSS */
		} else
#endif /* QOS_DLS_SUPPORT */
		if (ADHOC_ON(pAd))
			pWI->field.Mode = 0;	/* IBSS */
		else {
			pWI->field.Mode = 2; /* STA*/
		}
	}

	/* 
	   -----------------------------------------------------------------
	   STEP 2. MAKE A COMMON 802.11 HEADER SHARED BY ENTIRE FRAGMENT BURST. Fill sequence later. 
	   -----------------------------------------------------------------
	 */
	if (pAd->CommonCfg.bAPSDForcePowerSave)
		pWI->field.PS = PWR_SAVE;
	else
	{
		/*7636 psm*/
		pWI->field.PS = (pAd->StaCfg.PwrMgmt.Psm == PWR_SAVE);
	}
}


VOID STA_AMPDU_Frame_Tx_Hdr_Trns(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *pHeader_802_11;
	UCHAR *pWiBufPtr;
	USHORT FreeNumber = 0;
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	BOOLEAN			bHTCPlus;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TX_WIFI_INFO *pWI;
	
	ASSERT(pTxBlk);

	while (pTxBlk->TxPacketList.Head) {
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket,
					    NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		pMacEntry = pTxBlk->pMacEntry;
		tr_entry = pTxBlk->tr_entry;
		if ((tr_entry->isCached))
		{
			/* NOTE: Please make sure the size of tr_entry->CachedBuf[] is smaller than pTxBlk->HeaderBuf[]!!!! */
			NdisMoveMemory((PUCHAR)
				       (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
				       (PUCHAR) (&tr_entry->CachedBuf[0]),
				       TXWISize + TX_WIFI_INFO_SIZE);

			pWiBufPtr = (PUCHAR) (&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize]);

			STABuildCacheWifiInfo(pAd, pTxBlk, pWiBufPtr);

		} else {
			STAFindCipherAlgorithm(pAd, pTxBlk);
			STABuildWifiInfo(pAd, pTxBlk);

			pWiBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
		}

		pWI = (TX_WIFI_INFO *)pWiBufPtr;

		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

		if (bVLANPkt)
			pWI->field.VLAN = TRUE;
	
		pWI->field.TID = (pTxBlk->UserPriority & 0x0F);

		{
			/* build HTC+, HTC control field following QoS field */
			bHTCPlus = FALSE;

			if ((pAd->CommonCfg.bRdg == TRUE)
			    && CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)
			)
			{
				if (tr_entry->isCached == FALSE)
				{
					/* mark HTC bit */
					pWI->field.RDG = 1;
				}
				
				bHTCPlus = TRUE;
			}

		}

		if ((tr_entry->isCached)) {
			write_tmac_info_Cache(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);
		}
		else
		{
			write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

			NdisZeroMemory((PUCHAR) (&tr_entry->CachedBuf[0]),
				       sizeof (tr_entry->CachedBuf));
			NdisMoveMemory((PUCHAR) (&tr_entry->CachedBuf[0]),
				       (PUCHAR) (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
				       TXWISize + TX_WIFI_INFO_SIZE);


//+++Mark by shiang test
//			tr_entry->isCached = TRUE;
//---Mark by shiang for test
		}

#ifdef STATS_COUNT_SUPPORT
		pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart++;
		pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;
#endif /* STATS_COUNT_SUPPORT */

		pTxBlk->NeedTrans = TRUE;
		HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)pHeader_802_11);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		/* Kick out Tx */
#ifdef PCIE_PS_SUPPORT
		if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
			HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;
	}

}


VOID STA_Legacy_Frame_Tx_Hdr_Trns(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PUCHAR pHeaderBufPtr;
	USHORT FreeNumber = 0;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TX_WIFI_INFO *pWI;

	ASSERT(pTxBlk);

	//printk("STA_Legacy_Frame_Tx_Hdr_Trns\n");

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

#ifdef STATS_COUNT_SUPPORT
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
	}
#endif /* STATS_COUNT_SUPPORT */


	if (RTMP_GET_PACKET_RTS(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bRtsRequired);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bRtsRequired);

	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	if (pTxBlk->TxRate < pAd->CommonCfg.MinTxRate)
		pTxBlk->TxRate = pAd->CommonCfg.MinTxRate;

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildWifiInfo(pAd, pTxBlk);

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];

	pWI = (TX_WIFI_INFO *)pHeaderBufPtr;

	//hex_dump("wifi info:", pWI, sizeof(TX_WIFI_INFO));

	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	//hex_dump("pSrcBufData" , pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);

	if(bVLANPkt)
		pWI->field.VLAN = TRUE;

	pWI->field.TID = (pTxBlk->UserPriority & 0x0F);



	/*
	   prepare for TXWI
	   use Wcid as Key Index
	 */

	write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);


	pTxBlk->NeedTrans = TRUE;
	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)pHeader_802_11);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/*
	   Kick out Tx
	 */
#ifdef PCIE_PS_SUPPORT
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
#endif /* PCIE_PS_SUPPORT */
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}


VOID STAHandleRxDataFrame_Hdr_Trns(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	BOOLEAN bFragment = FALSE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR wdev_idx = BSS0;
	UCHAR UserPriority = 0;
	UCHAR *pData;

//+++Add by shiang for debug
//---Add by shiangf for debug

	if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1)) {
#ifdef CLIENT_WDS
			if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
			    && IS_ENTRY_CLIENT(&pAd->MacTab.Content[pRxBlk->wcid])) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		} else
#endif /* CLIENT_WDS */
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
	else
	{
#ifdef QOS_DLS_SUPPORT
		if (RTMPRcvFrameDLSCheck(pAd, pHeader, pRxBlk->MPDUtotalByteCnt, pRxBlk->pRxInfo)) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
#endif /* QOS_DLS_SUPPORT */

		/* Drop not my BSS frames */
		if (pRxInfo->MyBss == 0) {
			{
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}


		pAd->RalinkCounters.RxCountSinceLastNULL++;
		if (pAd->StaCfg.wdev.UapsdInfo.bAPSDCapable
		    && pAd->CommonCfg.APEdcaParm.bAPSDCapable
		    && (pHeader->FC.SubType & 0x08))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("bAPSDCapable\n"));

			/* Qos bit 4 */
			pData = (PUCHAR) pHeader + LENGTH_802_11;
			if ((*pData >> 4) & 0x01)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));
				pAd->CommonCfg.bInServicePeriod = FALSE;

				/* Force driver to fall into sleep mode when rcv EOSP frame */
				if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
				{
#ifdef RTMP_MAC_PCI
					USHORT TbttNumToNextWakeUp;
					USHORT NextDtim = pAd->StaCfg.DtimPeriod;
					ULONG Now;

					NdisGetSystemUpTime(&Now);
					NextDtim -= (USHORT) (Now - pAd->StaCfg.LastBeaconRxTime) / pAd->CommonCfg.BeaconPeriod;

					TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
						TbttNumToNextWakeUp = NextDtim;

					RTMP_SET_PSM_BIT(pAd, PWR_SAVE);
					/* if WMM-APSD is failed, try to disable following line */
					AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);
#endif /* RTMP_MAC_PCI */

				}
			}

			if ((pHeader->FC.MoreData)
			    && (pAd->CommonCfg.bInServicePeriod)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("Sending another trigger frame when More Data bit is set to 1\n"));
			}
		}

		/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
		if ((pHeader->FC.SubType & 0x04)) /* bit 2 : no DATA */
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		if (pAd->StaCfg.BssType == BSS_INFRA) {
			/* Infrastructure mode, check address 2 for BSSID */
			if (1
#ifdef QOS_DLS_SUPPORT
			    && (!pAd->CommonCfg.bDLSCapable)
#endif /* QOS_DLS_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			    && (!IS_TDLS_SUPPORT(pAd))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

			    ) {
				if (!RTMPEqualMemory(&pHeader->Addr2, &pAd->MlmeAux.Bssid, 6))
				{
					/* Receive frame not my BSSID */
					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
		else
		{	/* Ad-Hoc mode or Not associated */

			/* Ad-Hoc mode, check address 3 for BSSID */
			if (!RTMPEqualMemory(&pHeader->Addr3, &pAd->CommonCfg.Bssid, 6)) {
				/* Receive frame not my BSSID */
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}

		/*/ find pEntry */
		if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) {
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

		} else {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* infra or ad-hoc */
		if (pAd->StaCfg.BssType == BSS_INFRA) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AP);
#if defined(DOT11Z_TDLS_SUPPORT) || defined(QOS_DLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			if ((pHeader->FC.FrDs == 0) && (pHeader->FC.ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_DLS);
			else
#endif
				ASSERT(pRxBlk->wcid == BSSID_WCID);
		}

		// TODO: shiang@PF#2, is this atheros protection still necessary here???
		/* check Atheros Client */
		if ((pEntry->bIAmBadAtheros == FALSE) && (pRxInfo->AMPDU == 1)
		    && (pHeader->FC.Retry)) {
			pEntry->bIAmBadAtheros = TRUE;
			pAd->CommonCfg.IOTestParm.bLastAtheros = TRUE;
			if (!STA_AES_ON(pAd))
				RTMP_UPDATE_PROTECT(pAd, 8 , ALLN_SETPROTECT, TRUE, FALSE);
		}
	}


#ifdef RLT_MAC
#ifdef CONFIG_RX_CSO_SUPPORT
	if (RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_RX_CSO_SUPPORT))
	{
		RXFCE_INFO *pRxFceInfo = pRxBlk->pRxFceInfo;

		if ( pRxFceInfo->l3l4_done )
		{


/*
			if ( (pRxFceInfo->ip_err) || (pRxFceInfo->tcp_err)
				|| (pRxFceInfo->udp_err) ) {
				RTMP_SET_TCP_CHKSUM_FAIL(pRxPacket, TRUE);
			}
*/
			// Linux always do IP header chksum
			if (  (pRxFceInfo->tcp_err) || (pRxFceInfo->udp_err) ) {
				RTMP_SET_TCP_CHKSUM_FAIL(pRxPacket, TRUE);
			}
		}
	}
#endif /* CONFIG_RX_CSO_SUPPORT */
#endif /* RLT_MAC */

	pData = (UCHAR *) pHeader;

	/*
	   update RxBlk->pData, DataSize
	   802.11 Header, QOS, HTC, Hw Padding
	 */
	/* 1. skip 802.11 HEADER */
#ifdef CLIENT_WDS
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) {
		pData += LENGTH_802_11_WITH_ADDR4;
	} else
#endif /* CLIENT_WDS */
	{
		pData += LENGTH_802_11;
	}

	/* 2. QOS */
	if (pHeader->FC.SubType & 0x08) {
		RX_BLK_SET_FLAG(pRxBlk, fRX_QOS);
		UserPriority = *(pData) & 0x0f;
		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pData) & 0x80) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
		}

		/* skip QOS contorl field */
		pData += 2;
	}
	pRxBlk->UserPriority = UserPriority;

	/* check if need to resend PS Poll when received packet with MoreData = 1 */
	/*7636 psm*/
	if ((pAd->StaCfg.PwrMgmt.Psm == PWR_SAVE) && (pHeader->FC.MoreData == 1)) {
		if ((((UserPriority == 0) || (UserPriority == 3)) &&
		     pAd->CommonCfg.bAPSDAC_BE == 0) ||
		    (((UserPriority == 1) || (UserPriority == 2)) &&
		     pAd->CommonCfg.bAPSDAC_BK == 0) ||
		    (((UserPriority == 4) || (UserPriority == 5)) &&
		     pAd->CommonCfg.bAPSDAC_VI == 0) ||
		    (((UserPriority == 6) || (UserPriority == 7)) &&
		     pAd->CommonCfg.bAPSDAC_VO == 0)) {
			/* non-UAPSD delivery-enabled AC */
			RTMP_PS_POLL_ENQUEUE(pAd);
		}
	}

	/* 3. Order bit: A-Ralink or HTC+ */
	if (pHeader->FC.Order) {
#ifdef AGGREGATION_SUPPORT
		if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM)
		    && (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE)))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		} else
#endif /* AGGREGATION_SUPPORT */
		{
#ifdef DOT11_N_SUPPORT
			//RX_BLK_SET_FLAG(pRxBlk, fRX_HTC);
			/* skip HTC contorl field */
			pData += 4;
#endif /* DOT11_N_SUPPORT */
		}
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD) {
		/* just move pData pointer, because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pData += 2;
	}
#ifdef DOT11_N_SUPPORT
	if (pRxInfo->BA) {
		RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}
#endif /* DOT11_N_SUPPORT */

#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1) 
	   and it's passed to driver with "Decrypted" marked as 0 in pRxInfo. */
	if ((pHeader->FC.Wep == 1) && (pRxInfo->Decrypted == 0)) {
		PCIPHER_KEY pSwKey = RTMPSwCipherKeySelection(pAd,
						       pRxBlk->pData, pRxBlk,
						       pEntry);

		/* Cipher key table selection */
		if (!pSwKey) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("No vaild cipher key for SW decryption!!!\n"));
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* Decryption by Software */
		if (RTMPSoftDecryptionAction(pAd,
					     (PUCHAR) pHeader,
					     UserPriority,
					     pSwKey,
					     pRxBlk->pTransData + 14,
					     &(pRxBlk->TransDataSize)) !=
		    NDIS_STATUS_SUCCESS) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
		/* Record the Decrypted bit as 1 */
		pRxInfo->Decrypted = 1;
	}
#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */

	/* Case I  Process Broadcast & Multicast data frame */
	if (pRxInfo->Bcast || pRxInfo->Mcast) {
#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

		/* Drop Mcast/Bcast frame with fragment bit on */
		if (pHeader->FC.MoreFrag) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		/* Filter out Bcast frame which AP relayed for us */
		if (pHeader->FC.FrDs
		    && MAC_ADDR_EQUAL(pHeader->Addr3, pAd->CurrentAddress)) {
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		if (ADHOC_ON(pAd)) {
			MAC_TABLE_ENTRY *pAdhocEntry = NULL;
			pAdhocEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pAdhocEntry)
				Update_Rssi_Sample(pAd, &pAdhocEntry->RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE);
		}

		Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
		return;
	}
	else if (pRxInfo->U2M)
	{
		pAd->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#if defined(DOT11Z_TDLS_SUPPORT) || defined(QOS_DLS_SUPPORT)
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_DLS)) {
			MAC_TABLE_ENTRY *pDlsEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			if (pDlsEntry && (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)) {
				Update_Rssi_Sample(pAd, &pDlsEntry->RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE);
				NdisAcquireSpinLock(&pAd->MacTabLock);
				pDlsEntry->NoDataIdleCount = 0;
				// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
				pAd->MacTab.tr_entry[pDlsEntry->wcid].NoDataIdleCount = 0;
				NdisReleaseSpinLock(&pAd->MacTabLock);
			}
		}
		else
#endif
		if (ADHOC_ON(pAd)) {
			MAC_TABLE_ENTRY *pAdhocEntry = NULL;
			pAdhocEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pAdhocEntry)
				Update_Rssi_Sample(pAd, &pAdhocEntry->RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE);
		}

		Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE);

		pAd->StaCfg.wdev.LastSNR0 = (UCHAR) (pRxBlk->rx_signal.raw_snr[0]);
		pAd->StaCfg.wdev.LastSNR1 = (UCHAR) (pRxBlk->rx_signal.raw_snr[1]);
#ifdef DOT11N_SS3_SUPPORT
		if (pAd->CommonCfg.RxStream == 3)
			pAd->StaCfg.wdev.LastSNR2 = (UCHAR) (pRxBlk->rx_signal.raw_snr[2]);
#endif /* DOT11N_SS3_SUPPORT */

		pAd->RalinkCounters.OneSecRxOkDataCnt++;

		if (pEntry != NULL)
		{
			pEntry->LastRxRate = pAd->LastRxRate;

			pEntry->freqOffset = (CHAR)(pRxBlk->rx_signal.freq_offset);
			pEntry->freqOffsetValid = TRUE;

		}

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */

//#if defined(RTMP_MAC_USB) || defined(RTMP_SDIO_SUPPORT)
		/* there's packet sent to me, keep awake for 1200ms */
		if (pAd->CountDowntoPsm < 12)
		{
			pAd->CountDowntoPsm = 12;
		}

		if (!((pHeader->Frag == 0) && (pHeader->FC.MoreFrag == 0))) {
			/* re-assemble the fragmented packets */
			/* return complete frame (pRxPacket) or NULL */
			bFragment = TRUE;
			pRxPacket = RTMPDeFragmentDataFrame(pAd, pRxBlk);
		}

		if (pRxPacket) {
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

			/* process complete frame */
			if (bFragment && (pRxInfo->Decrypted)
			    && (pEntry->WepStatus == Ndis802_11TKIPEnable)) {
				/* Minus MIC length */
				pRxBlk->DataSize -= 8;

				/* For TKIP frame, calculate the MIC value */
				if (rtmp_chk_tkip_mic(pAd, pEntry, pRxBlk) == FALSE)
					return;
			}

			rx_data_frm_announce(pAd, pEntry, pRxBlk, wdev_idx);
			return;
		} else {
			/*
			   just return because RTMPDeFragmentDataFrame() will release rx packet, 
			   if packet is fragmented
			 */
			return;
		}
	}
#ifdef XLINK_SUPPORT
	else if (pAd->StaCfg.PSPXlink) {
		Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
		return;
	}
#endif /* XLINK_SUPPORT */

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

	return;
}
#endif /* CONFIG_STA_SUPPORT */

#endif /* HDR_TRANS_SUPPORT */

