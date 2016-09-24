/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_data.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

 
#include "rt_config.h"

#ifdef LED_SOFT_SUPPORT
#include <ralink/ralink_gpio.h>
#endif

UCHAR	SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR	SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};
UCHAR	EAPOL[] = {0x88, 0x8e};
UCHAR   TPID[] = {0x81, 0x00}; /* VLAN related */

UCHAR	IPX[] = {0x81, 0x37};
UCHAR	APPLE_TALK[] = {0x80, 0xf3};


UCHAR MapUserPriorityToAccessCategory[8] = {QID_AC_BE, QID_AC_BK, QID_AC_BK, QID_AC_BE, QID_AC_VI, QID_AC_VI, QID_AC_VO, QID_AC_VO};



VOID dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo)
{
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));

	DBGPRINT(RT_DEBUG_OFF, ("RxInfo Fields:\n"));

#ifdef RLT_MAC
	DBGPRINT(RT_DEBUG_OFF, ("\tBA=%d\n", pRxInfo->BA));
	DBGPRINT(RT_DEBUG_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	DBGPRINT(RT_DEBUG_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	DBGPRINT(RT_DEBUG_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	DBGPRINT(RT_DEBUG_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	DBGPRINT(RT_DEBUG_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	DBGPRINT(RT_DEBUG_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	DBGPRINT(RT_DEBUG_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	DBGPRINT(RT_DEBUG_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	DBGPRINT(RT_DEBUG_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	DBGPRINT(RT_DEBUG_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	DBGPRINT(RT_DEBUG_OFF, ("\ttcp_sum_bypass=%d\n", pRxInfo->tcp_sum_bypass));
	DBGPRINT(RT_DEBUG_OFF, ("\tip_sum_bypass=%d\n", pRxInfo->ip_sum_bypass));	
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	DBGPRINT(RT_DEBUG_OFF, ("\t", ));
#endif /* RTMP_MAC */
}


#ifdef RLT_MAC
VOID dumpRxFCEInfo(RTMP_ADAPTER *pAd, RXFCE_INFO *pRxFceInfo)
{
	hex_dump("RxFCEInfo Raw Data", (UCHAR *)pRxFceInfo, sizeof(RXFCE_INFO));

	DBGPRINT(RT_DEBUG_OFF, ("RxFCEInfo Fields:\n"));

	DBGPRINT(RT_DEBUG_OFF, ("\tinfo_type=%d\n", pRxFceInfo->info_type));
	DBGPRINT(RT_DEBUG_OFF, ("\ts_port=%d\n", pRxFceInfo->s_port));
	DBGPRINT(RT_DEBUG_OFF, ("\tqsel=%d\n", pRxFceInfo->qsel));
	DBGPRINT(RT_DEBUG_OFF, ("\tpcie_intr=%d\n", pRxFceInfo->pcie_intr));
	DBGPRINT(RT_DEBUG_OFF, ("\tmac_len=%d\n", pRxFceInfo->mac_len));
	DBGPRINT(RT_DEBUG_OFF, ("\tl3l4_done=%d\n", pRxFceInfo->l3l4_done));
	DBGPRINT(RT_DEBUG_OFF, ("\tpkt_80211=%d\n", pRxFceInfo->pkt_80211));
	DBGPRINT(RT_DEBUG_OFF, ("\tip_err=%d\n", pRxFceInfo->ip_err));
	DBGPRINT(RT_DEBUG_OFF, ("\ttcp_err=%d\n", pRxFceInfo->tcp_err));
	DBGPRINT(RT_DEBUG_OFF, ("\tudp_err=%d\n", pRxFceInfo->udp_err));
	DBGPRINT(RT_DEBUG_OFF, ("\tpkt_len=%d\n", pRxFceInfo->pkt_len));
}
#endif /* RLT_MAC */

#ifdef DBG
static UCHAR *txwi_txop_str[]={"HT_TXOP", "PIFS", "SIFS", "BACKOFF", "Invalid"};
#define TXWI_TXOP_STR(_x)	((_x) <= 3 ? txwi_txop_str[(_x)]: txwi_txop_str[4])
#endif

VOID dumpTxWI(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI)
{
	hex_dump("TxWI Raw Data: ", (UCHAR *)pTxWI, sizeof(TXWI_STRUC));

	DBGPRINT(RT_DEBUG_OFF, ("TxWI Fields:\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tPHYMODE=%d(%s)\n", pTxWI->TxWIPHYMODE,  get_phymode_str(pTxWI->TxWIPHYMODE)));
	DBGPRINT(RT_DEBUG_OFF, ("\tSTBC=%d\n", pTxWI->TxWISTBC));
	DBGPRINT(RT_DEBUG_OFF, ("\tShortGI=%d\n", pTxWI->TxWIShortGI));
	DBGPRINT(RT_DEBUG_OFF, ("\tBW=%d(%sMHz)\n", pTxWI->TxWIBW, get_bw_str(pTxWI->TxWIBW)));
	DBGPRINT(RT_DEBUG_OFF, ("\tMCS=%d\n", pTxWI->TxWIMCS));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxOP=%d(%s)\n", pTxWI->TxWITXOP, TXWI_TXOP_STR(pTxWI->TxWITXOP)));
	DBGPRINT(RT_DEBUG_OFF, ("\tMpduDensity=%d\n", pTxWI->TxWIMpduDensity));	
	DBGPRINT(RT_DEBUG_OFF, ("\tAMPDU=%d\n", pTxWI->TxWIAMPDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tTS=%d\n", pTxWI->TxWITS));
	DBGPRINT(RT_DEBUG_OFF, ("\tCF-ACK=%d\n", pTxWI->TxWICFACK));
	DBGPRINT(RT_DEBUG_OFF, ("\tMIMO-PS=%d\n", pTxWI->TxWIMIMOps));
	DBGPRINT(RT_DEBUG_OFF, ("\tNSEQ=%d\n", pTxWI->TxWINSEQ));
	DBGPRINT(RT_DEBUG_OFF, ("\tACK=%d\n", pTxWI->TxWIACK));
	DBGPRINT(RT_DEBUG_OFF, ("\tFRAG=%d\n", pTxWI->TxWIFRAG));
	DBGPRINT(RT_DEBUG_OFF, ("\tWCID=%d\n", pTxWI->TxWIWirelessCliID));
	DBGPRINT(RT_DEBUG_OFF, ("\tBAWinSize=%d\n", pTxWI->TxWIBAWinSize));
	DBGPRINT(RT_DEBUG_OFF, ("\tMPDUtotalByteCnt=%d\n", pTxWI->TxWIMPDUByteCnt));	
	DBGPRINT(RT_DEBUG_OFF, ("\tPID=%d\n", pTxWI->TxWIPacketId));	
}


VOID dump_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI)
{
	hex_dump("RxWI Raw Data", (UCHAR *)pRxWI, sizeof(RXWI_STRUC));

	DBGPRINT(RT_DEBUG_OFF, ("RxWI Fields:\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\tWCID=%d\n", pRxWI->RxWIWirelessCliID));
	DBGPRINT(RT_DEBUG_OFF, ("\tPhyMode=%d(%s)\n", pRxWI->RxWIPhyMode, get_phymode_str(pRxWI->RxWIPhyMode)));
#ifdef RT65xx
	if (IS_RT65XX(pAd) && (pRxWI->RxWIPhyMode == MODE_VHT))
		DBGPRINT(RT_DEBUG_OFF, ("\tMCS=%d(Nss:%d, MCS:%d)\n", pRxWI->RxWIMCS, (pRxWI->RxWIMCS >> 4), (pRxWI->RxWIMCS & 0xf)));
	else
#endif /* RT65xx */
		DBGPRINT(RT_DEBUG_OFF, ("\tMCS=%d\n", pRxWI->RxWIMCS));
	DBGPRINT(RT_DEBUG_OFF, ("\tBW=%d\n", pRxWI->RxWIBW));
	DBGPRINT(RT_DEBUG_OFF, ("\tSGI=%d\n", pRxWI->RxWISGI));
	DBGPRINT(RT_DEBUG_OFF, ("\tMPDUtotalByteCnt=%d\n", pRxWI->RxWIMPDUByteCnt));
	DBGPRINT(RT_DEBUG_OFF, ("\tTID=%d\n", pRxWI->RxWITID));
	DBGPRINT(RT_DEBUG_OFF, ("\tSTBC=%d\n", pRxWI->RxWISTBC));
	DBGPRINT(RT_DEBUG_OFF, ("\tkey_idx=%d\n", pRxWI->RxWIKeyIndex));
	DBGPRINT(RT_DEBUG_OFF, ("\tBSS_IDX=%d\n", pRxWI->RxWIBSSID));	
}

#ifdef DBG
static UCHAR *txinfo_type_str[]={"PKT", "", "CMD", "RSV", "Invalid"};
static UCHAR *txinfo_d_port_str[]={"WLAN", "CPU_RX", "CPU_TX", "HOST", "VIRT_RX", "VIRT_TX", "DROP", "Invalid"};
static UCHAR *txinfo_que_str[]={"MGMT", "HCCA", "EDCA_1", "EDCA_2", "Invalid"};
#define TXINFO_TYPE_STR(_x)  	((_x)<=3 ?  txinfo_type_str[_x] : txinfo_type_str[4])
#define TXINFO_DPORT_STR(_x)	((_x) <= 6 ? txinfo_d_port_str[_x]: txinfo_d_port_str[7])
#define TXINFO_QUE_STR(_x)		((_x) <= 3 ? txinfo_que_str[_x]: txinfo_que_str[4])
#endif

VOID dump_txinfo(RTMP_ADAPTER *pAd, TXINFO_STRUC *pTxInfo)
{
#ifdef DBG
	hex_dump("TxInfo Raw Data: ", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
	
	DBGPRINT(RT_DEBUG_OFF, ("TxInfo Fields:\n"));

#ifdef RLT_MAC
{
	struct _TXINFO_NMAC_PKT *pkt_txinfo = (struct _TXINFO_NMAC_PKT *)pTxInfo;

	DBGPRINT(RT_DEBUG_OFF, ("\tInfo_Type=%d(%s)\n", pkt_txinfo->info_type, TXINFO_TYPE_STR(pkt_txinfo->info_type)));
	DBGPRINT(RT_DEBUG_OFF, ("\td_port=%d(%s)\n", pkt_txinfo->d_port, TXINFO_DPORT_STR(pkt_txinfo->d_port)));
	DBGPRINT(RT_DEBUG_OFF, ("\tQSEL=%d(%s)\n", pkt_txinfo->QSEL, TXINFO_QUE_STR(pkt_txinfo->QSEL)));
	DBGPRINT(RT_DEBUG_OFF, ("\tWIV=%d\n", pkt_txinfo->wiv));
	DBGPRINT(RT_DEBUG_OFF, ("\t802.11=%d\n", pkt_txinfo->pkt_80211));
	DBGPRINT(RT_DEBUG_OFF, ("\tcso=%d\n", pkt_txinfo->cso));
	DBGPRINT(RT_DEBUG_OFF, ("\ttso=%d\n", pkt_txinfo->tso));
	DBGPRINT(RT_DEBUG_OFF, ("\tpkt_len=0x%x\n", pkt_txinfo->pkt_len));
}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
#endif /* RTMP_MAC */
#endif
}


#ifdef DBG_DIAGNOSE
VOID dumpTxBlk(TX_BLK *pTxBlk)
{
	NDIS_PACKET *pPacket;
	int i, frameNum;
	PQUEUE_ENTRY	pQEntry;

	DBGPRINT(RT_DEBUG_TRACE,("Dump TX_BLK Structure:\n"));
	DBGPRINT(RT_DEBUG_TRACE,("\tTxFrameType=%d!\n", pTxBlk->TxFrameType));
	DBGPRINT(RT_DEBUG_TRACE,("\tTotalFrameLen=%d\n", pTxBlk->TotalFrameLen));
	DBGPRINT(RT_DEBUG_TRACE,("\tTotalFrameNum=%ld!\n", pTxBlk->TxPacketList.Number));
	DBGPRINT(RT_DEBUG_TRACE,("\tTotalFragNum=%d!\n", pTxBlk->TotalFragNum));
	DBGPRINT(RT_DEBUG_TRACE,("\tpPacketList=\n"));

	frameNum = pTxBlk->TxPacketList.Number;

	for(i=0; i < frameNum; i++)
	{	int j;
		UCHAR	*pBuf;

		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (pPacket)
		{
			pBuf = GET_OS_PKT_DATAPTR(pPacket);
			DBGPRINT(RT_DEBUG_TRACE,("\t\t[%d]:ptr=0x%x, Len=%d!\n", i, (UINT32)(GET_OS_PKT_DATAPTR(pPacket)), GET_OS_PKT_LEN(pPacket)));
			DBGPRINT(RT_DEBUG_TRACE,("\t\t"));
			for (j =0 ; j < GET_OS_PKT_LEN(pPacket); j++)
			{
				DBGPRINT(RT_DEBUG_TRACE,("%02x ", (pBuf[j] & 0xff)));
				if (j == 16)
					break;
			}
			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
		}
	}
	DBGPRINT(RT_DEBUG_TRACE,("\tWcid=%d!\n", pTxBlk->Wcid));
	DBGPRINT(RT_DEBUG_TRACE,("\tapidx=%d!\n", pTxBlk->apidx));
	DBGPRINT(RT_DEBUG_TRACE,("----EndOfDump\n"));

}
#endif


/*
	========================================================================

	Routine Description:
		API for MLME to transmit management frame to AP (BSS Mode)
	or station (IBSS Mode)
		
	Arguments:
		pAd Pointer to our adapter
		pData		Pointer to the outgoing 802.11 frame
		Length		Size of outgoing management frame
		
	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
NDIS_STATUS MiniportMMRequest(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN UCHAR *pData,
	IN UINT Length)
{
	PNDIS_PACKET pPacket;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	ULONG FreeNum;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UCHAR rtmpHwHdr[40];
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags = 0;
#endif /* RTMP_MAC_PCI */
	BOOLEAN bUseDataQ = FALSE, FlgDataQForce = FALSE, FlgIsLocked = FALSE;
	int retryCnt = 0, hw_len = TXINFO_SIZE + TXWISize + TSO_SIZE;


	ASSERT(Length <= MGMT_DMA_BUFFER_SIZE);

	if ((QueIdx & MGMT_USE_QUEUE_FLAG) == MGMT_USE_QUEUE_FLAG)
	{
		bUseDataQ = TRUE;
		QueIdx &= (~MGMT_USE_QUEUE_FLAG);
	}

#ifdef FPGA_MODE
	if (pAd->fpga_on & 0x1) {
		if (pAd->tx_kick_cnt > 0) {
			if (pAd->tx_kick_cnt < 0xffff) 
				pAd->tx_kick_cnt--;
		}
		else
			return NDIS_STATUS_FAILURE;
		
		QueIdx = 0;
		bUseDataQ = TRUE;
	}
#endif /* FPGA_MODE */

#ifdef RTMP_MAC_PCI
	if (pAd->MACVersion == 0x28600100)
	{
		/* do not care about the version */
		QueIdx = (bUseDataQ ==TRUE ? QueIdx : 3);
		bUseDataQ = TRUE;
	}
	if (bUseDataQ)
	{
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		FlgIsLocked = TRUE;
		retryCnt = MAX_DATAMM_RETRY;		
	}
#endif /* RTMP_MAC_PCI */

	do
	{
		/* Reset is in progress, stop immediately*/
		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
			 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
			)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}


		/* Check Free priority queue*/
		/* Since we use PBF Queue2 for management frame.  Its corresponding DMA ring should be using TxRing.*/
#ifdef RTMP_MAC_PCI
		if (bUseDataQ)
		{
			/* free Tx(QueIdx) resources*/
			RTMPFreeTXDUponTxDmaDone(pAd, QueIdx);
			FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
		}
		else
#endif /* RTMP_MAC_PCI */
		{
			FreeNum = GET_MGMTRING_FREENO(pAd);
		}

		if ((FreeNum > 0))
		{
			/* We need to reserve space for rtmp hardware header. i.e., TxWI for RT2860 and TxInfo+TxWI for RT2870*/
			NdisZeroMemory(&rtmpHwHdr, hw_len);
			Status = RTMPAllocateNdisPacket(pAd, &pPacket, (PUCHAR)&rtmpHwHdr, hw_len, pData, Length);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_WARN, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
				break;
			}


#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				UAPSD_MR_QOS_NULL_HANDLE(pAd, pData, pPacket);
			}
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef RTMP_MAC_PCI
			if (bUseDataQ)
			{
				FlgDataQForce = TRUE;
				retryCnt--;
			}
#endif /* RTMP_MAC_PCI */

			Status = MlmeHardTransmit(pAd, QueIdx, pPacket, FlgDataQForce, FlgIsLocked);
			if (Status == NDIS_STATUS_SUCCESS)
				retryCnt = 0;
			else
				RELEASE_NDIS_PACKET(pAd, pPacket, Status);
		}
		else
		{
			pAd->RalinkCounters.MgmtRingFullCount++;
#ifdef RTMP_MAC_PCI
			if (bUseDataQ)
			{
				retryCnt--;
				DBGPRINT(RT_DEBUG_TRACE, ("retryCnt %d\n", retryCnt));
				if (retryCnt == 0)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Qidx(%d), not enough space in DataRing, MgmtRingFullCount=%ld!\n",
											QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
				}
			}
#endif /* RTMP_MAC_PCI */
			DBGPRINT(RT_DEBUG_ERROR, ("Qidx(%d), not enough space in MgmtRing, MgmtRingFullCount=%ld!\n",
										QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
		}
	} while (retryCnt > 0);

	
#ifdef RTMP_MAC_PCI
	if (bUseDataQ)
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */

	return Status;
}


#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		FlgIsDeltsFrame 1: the frame is a DELTS frame
		
	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
void AP_QueuePsActionPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pMacEntry,
	IN	PNDIS_PACKET	pPacket,
	IN	BOOLEAN			FlgIsDeltsFrame,
	IN	BOOLEAN			FlgIsLocked,
	IN	UCHAR			MgmtQid)
{

#ifdef UAPSD_SUPPORT
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
	PNDIS_PACKET DuplicatePkt = NULL;
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
#endif /* UAPSD_SUPPORT */

	/* Note: for original mode of 4 AC are UAPSD, if station want to change
			the mode of a AC to legacy PS, we dont know where to put the
			response;
			1. send the response;
			2. but the station is in ps mode, so queue the response;
			3. we should queue the reponse to UAPSD queue because the station
				is not yet change its mode to legacy ps AC;
			4. so AP should change its mode to legacy ps AC only when the station
				sends a trigger frame and we send out the reponse;
			5. the mechanism is too complicate; */

#ifdef UAPSD_SUPPORT
	/*
		If the frame is action frame and the VO is UAPSD, we can not send the
		frame to VO queue, we need to send to legacy PS queue; or the frame
		maybe not got from QSTA.
	*/
/*    if ((pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid]) &&*/
/*		(FlgIsDeltsFrame == 0))*/
    if (pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid])
	{
		/* queue the management frame to VO queue if VO is deliver-enabled */
		DBGPRINT(RT_DEBUG_TRACE, ("ps> mgmt to UAPSD queue %d ... (IsDelts: %d)\n",
				MgmtQid, FlgIsDeltsFrame));

#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
		if (!pMacEntry->bAPSDAllAC)
		{
			/* duplicate one packet to legacy PS queue */
			RTMP_SET_PACKET_UAPSD(pPacket, 0, MgmtQid);
			DuplicatePkt = RTMP_DUPLICATE_PACKET(pAd, pPacket, pMacEntry->apidx);
		}
		else
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		{
			RTMP_SET_PACKET_UAPSD(pPacket, 1, MgmtQid);
		}

        UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, MgmtQid);

		if (pMacEntry->bAPSDAllAC)
		{
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->apidx, pMacEntry->Aid);
		}
		else
		{
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
			/* duplicate one packet to legacy PS queue */

			/*
				Sometimes AP will send DELTS frame to STA but STA will not
				send any trigger frame to get the DELTS frame.
				We must force to send it so put another one in legacy PS
				queue.
			*/
			if (DuplicatePkt != NULL)
			{
				pPacket = DuplicatePkt;
				goto Label_Legacy_PS;
			}
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		}
    }
    else
#endif /* UAPSD_SUPPORT */
    {
/*		DuplicatePkt = DuplicatePacket(get_netdev_from_bssid(pAd, pMacEntry->apidx), pPacket, pMacEntry->apidx);*/

#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
Label_Legacy_PS:
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		if (pMacEntry->PsQueue.Number >= MAX_PACKETS_IN_PS_QUEUE)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
			return;
		}
        else
        {
			ULONG IrqFlags=0;

			DBGPRINT(RT_DEBUG_TRACE, ("ps> mgmt to legacy ps queue... (%d)\n", FlgIsDeltsFrame));

			if (FlgIsLocked == FALSE)
				RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		    InsertTailQueue(&pMacEntry->PsQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
			if (FlgIsLocked == FALSE)
				RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
        }

		/* mark corresponding TIM bit in outgoing BEACON frame*/
		WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->apidx, pMacEntry->Aid);
    }
}
#endif /* CONFIG_AP_SUPPORT */


/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function
	
	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		
	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
NDIS_STATUS MlmeHardTransmit(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN BOOLEAN FlgDataQForce,
	IN BOOLEAN FlgIsLocked)
{
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
	PHEADER_802_11	pHeader_802_11;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#endif /* CONFIG_AP_SUPPORT */
	PACKET_INFO 	PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;

	if ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif /* CONFIG_AP_SUPPORT */
#endif /* CARRIER_DETECTION_SUPPORT */
		)
	{
		return NDIS_STATUS_FAILURE;
	}

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXINFO_SIZE + TXWISize + TSO_SIZE);

	/*
		Section 11.2.1.1 STA Power Management modes of IEEE802.11-2007:
		The Power Managment bit shall not be set in any management frame,
		except an Action frame.

		So in the 'baseline' test plan
		(Wi-Fi 802.11 WPA2, WPA, WEP Interoperability Test Plan),
		Section 2.2.6, the following Requirement:
        APs shall ignore the power save bit in any received Authenticate and
		(Re) Associate, and shall assume that the station is awake for the
		response.
	*/

	/*
		IEEE802.11, 11.2.1.4 AP operation during the contention period f)
		A single buffered MSDU or management frame for a STA in the PS mode shall
		be forwarded to the STA after a PS-Poll has been received from that STA.
		The More Data field shall be set to indicate the presence of further
		buffered MSDUs or "management frames" for the polling STA.
	*/

	/*
		IEEE802.11e, 11.2.1.4 Power management with APSD,
		An unscheduled SP ends after the QAP has attempted to transmit at least
		one MSDU or MMPDU associated with a delivery-enabled AC and destined for
		the non-AP QSTA, but no more than the number indicated in the Max SP
		Length field if the field has a nonzero value.
	*/

	if ((pHeader_802_11->FC.Type == BTYPE_DATA) ||
		(pHeader_802_11->FC.Type == BTYPE_MGMT))
	{
		if (pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL)
			pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}



	if ((pEntry != NULL) &&
		(pEntry->PsMode == PWR_SAVE) &&
		(((pHeader_802_11->FC.Type == BTYPE_DATA) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_NULL_FUNC) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL)) ||
		((pHeader_802_11->FC.Type == BTYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION)) ||
		((pHeader_802_11->FC.Type == BTYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION_NO_ACK))))
	{
		/* the peer is in PS mode, we need to queue the management frame */
		UINT8 FlgIsDeltsFrame = 0, MgmtQid = QID_AC_VO;

		/*
			1. Data & Not QoS Null, or
			2. Management & Action, or
			3. Management & Action No ACK;
		*/
		DBGPRINT(RT_DEBUG_TRACE, ("STA in ps mode, queue the mgmt frame\n"));
		RTMP_SET_PACKET_WCID(pPacket, pEntry->Aid);
		RTMP_SET_PACKET_MGMT_PKT(pPacket, 1); /* is management frame */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 0); /* default to management queue */

		if (FlgDataQForce == TRUE)
			RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 1); /* force to data queue */

		if ((pHeader_802_11->FC.Type == BTYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
		{
			FRAME_ADDBA_REQ *pFrameBa = (FRAME_ADDBA_REQ *)pHeader_802_11;
			if (pFrameBa->Category == CATEGORY_BA)
				MgmtQid = QueIdx;
		}


		AP_QueuePsActionPacket(pAd, pEntry, pPacket, FlgIsDeltsFrame,
								FlgIsLocked, MgmtQid);
		return NDIS_STATUS_SUCCESS;
	}
    else
#endif /* CONFIG_AP_SUPPORT */
    {
#ifdef RTMP_MAC_PCI
    	if (FlgDataQForce == TRUE)
    		return MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);
    	else
#endif /* RTMP_MAC_PCI */
    		return MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);
    }
}


NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	HEADER_802_11 *pHeader_802_11;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR MlmeRate;
	TXWI_STRUC *pFirstTxWI;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UCHAR PID;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	/* Make sure MGMT ring resource won't be used by other threads*/
	RTMP_SEM_LOCK(&pAd->MgmtRingLock);
	if (pSrcBufVA == NULL)
	{
		/* The buffer shouldn't be NULL*/
			RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
		return NDIS_STATUS_FAILURE;
	}


	pFirstTxWI = (TXWI_STRUC *)(pSrcBufVA +  TXINFO_SIZE);
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXINFO_SIZE + TSO_SIZE + TXWISize);
	
	if (pHeader_802_11->Addr1[0] & 0x01)
	{
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	}
	else
	{
		MlmeRate = pAd->CommonCfg.MlmeRate;
	}
	
	/* Verify Mlme rate for a / g bands.*/
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;

	if ((pHeader_802_11->FC.Type == BTYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}


	/* Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)*/
	/* Snice it's been set to 0 while on MgtMacHeaderInit*/
	/* By the way this will cause frame to be send on PWR_SAVE failed.*/
	
			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE; /* (pAd->StaCfg.Psm == PWR_SAVE);*/

	


#ifdef CONFIG_AP_SUPPORT
	pHeader_802_11->FC.MoreData = RTMP_GET_PACKET_MOREDATA(pPacket);
#endif /* CONFIG_AP_SUPPORT */


	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == BTYPE_CNTL) /* must be PS-POLL*/
	{
		bAckRequired = FALSE;
	}
	else /* BTYPE_MGMT or BTYPE_DATA(must be NULL frame)*/
	{
		if (pHeader_802_11->Addr1[0] & 0x01) /* MULTICAST, BROADCAST*/
		{
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		}
		else
		{
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
			if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) && (pHeader_802_11->FC.Type == BTYPE_MGMT))
			{
				bInsertTimestamp = TRUE;
				bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Response*/
			}
			else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == BTYPE_MGMT))
			{
				bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Request*/
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence >0xfff)
		pAd->Sequence = 0;

	/*
		Before radar detection done, mgmt frame can not be sent but probe req
		Because we need to use probe req to trigger driver to send probe req in passive scan
	*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->Dot11_H.RDMode != RD_NORMAL_MODE))
	{
		RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
		return NDIS_STATUS_FAILURE;
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#endif

	
	/*
		fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one physical buffer, and the whole frame size equals
		to the first scatter buffer size
	*/
	

	/*
		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement
	*/
	/*pAd->CommonCfg.MlmeTransmit.field.MODE = 1;*/
	
	/*
		management frame doesn't need encryption. 
		so use RESERVED_WCID no matter u are sending to specific wcid or not
	*/
	PID = PID_MGMT;


	if (pMacEntry == NULL)
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE,
						0, RESERVED_WCID, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE), PID, 0,
						(UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_BACKOFF, FALSE,
						&pAd->CommonCfg.MlmeTransmit);
	}
	else
	{
		/* dont use low rate to send QoS Null data frame */
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE,
					bInsertTimestamp, FALSE, bAckRequired, FALSE,
					0, pMacEntry->Aid, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE),
					pMacEntry->MaxHTPhyMode.field.MCS, 0,
					(UCHAR)pMacEntry->MaxHTPhyMode.field.MCS,
					IFS_BACKOFF, FALSE, &pMacEntry->MaxHTPhyMode);
	}

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pFirstTxWI, TYPE_TXWI);
#endif

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("TxMgmtPkt", (UCHAR *)pHeader_802_11, ((SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE) > 7000 ? 7000 : (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE)));
}
//---Add by shiang for debug
#endif

	/* Now do hardware-depened kick out.*/
	HAL_KickOutMgmtTx(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen);

	/* Make sure to release MGMT ring resource*/
/*	if (!IrqState)*/
		RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
	return NDIS_STATUS_SUCCESS;
}


/********************************************************************************
		
	New DeQueue Procedures.

 ********************************************************************************/
#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags) 				\
			do{													\
				if (bIntContext == FALSE)						\
				RTMP_IRQ_LOCK((lock), IrqFlags);		\
			}while(0)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
			do{													\
				if (bIntContext == FALSE)						\
					RTMP_IRQ_UNLOCK((lock), IrqFlags);	\
			}while(0)


/*
	========================================================================
	Tx Path design algorithm:
		Basically, we divide the packets into four types, Broadcast/Multicast, 11N Rate(AMPDU, AMSDU, Normal), B/G Rate(ARALINK, Normal),
		Specific Packet Type. Following show the classification rule and policy for each kinds of packets.
				Classification Rule=>
					Multicast: (*addr1 & 0x01) == 0x01
					Specific : bDHCPFrame, bARPFrame, bEAPOLFrame, etc.
					11N Rate : If peer support HT
								(1).AMPDU  -- If TXBA is negotiated.
								(2).AMSDU  -- If AMSDU is capable for both peer and ourself.
											*). AMSDU can embedded in a AMPDU, but now we didn't support it.
								(3).Normal -- Other packets which send as 11n rate.
								
					B/G Rate : If peer is b/g only.
								(1).ARALINK-- If both of peer/us supprot Ralink proprietary Aggregation and the TxRate is large than RATE_6
								(2).Normal -- Other packets which send as b/g rate.
					Fragment:
								The packet must be unicast, NOT A-RALINK, NOT A-MSDU, NOT 11n, then can consider about fragment.
								
				Classified Packet Handle Rule=>
					Multicast:
								No ACK, 		pTxBlk->bAckRequired = FALSE;
								No WMM, 		pTxBlk->bWMM = FALSE;
								No piggyback,   pTxBlk->bPiggyBack = FALSE;
								Force LowRate,  pTxBlk->bForceLowRate = TRUE;
					Specific :	Basically, for specific packet, we should handle it specifically, but now all specific packets are use
									the same policy to handle it.
								Force LowRate,  pTxBlk->bForceLowRate = TRUE;
								
					11N Rate :
								No piggyback,	pTxBlk->bPiggyBack = FALSE;
								
								(1).AMSDU
									pTxBlk->bWMM = TRUE;
								(2).AMPDU
									pTxBlk->bWMM = TRUE;
								(3).Normal
									
					B/G Rate :
								(1).ARALINK
									
								(2).Normal
	========================================================================
*/
static UCHAR TxPktClassification(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET  pPacket)
{
	UCHAR			TxFrameType = TX_UNKOWN_FRAME;
	UCHAR			Wcid;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;
#ifdef DOT11_N_SUPPORT
	BOOLEAN			bHTRate = FALSE;
#endif /* DOT11_N_SUPPORT */

	Wcid = RTMP_GET_PACKET_WCID(pPacket);
	if (Wcid == MCAST_WCID)
	{	/* Handle for RA is Broadcast/Multicast Address.*/
		return TX_MCAST_FRAME;
	}

	/* Handle for unicast packets*/
	pMacEntry = &pAd->MacTab.Content[Wcid];
	if (RTMP_GET_PACKET_LOWRATE(pPacket))
	{	/* It's a specific packet need to force low rate, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame*/
		TxFrameType = TX_LEGACY_FRAME;
	}
#ifdef DOT11_N_SUPPORT
	else if (IS_HT_RATE(pMacEntry))
	{	/* it's a 11n capable packet*/

		/* Depends on HTPhyMode to check if the peer support the HTRate transmission.*/
		/* 	Currently didn't support A-MSDU embedded in A-MPDU*/
		bHTRate = TRUE;
		if (RTMP_GET_PACKET_MOREDATA(pPacket) || (pMacEntry->PsMode == PWR_SAVE))
			TxFrameType = TX_LEGACY_FRAME;
#ifdef UAPSD_SUPPORT
		else if (RTMP_GET_PACKET_EOSP(pPacket))
			TxFrameType = TX_LEGACY_FRAME;
#endif /* UAPSD_SUPPORT */
#ifdef WFA_VHT_PF
		else if (pAd->force_amsdu == TRUE)
			return TX_AMSDU_FRAME;
#endif /* WFA_VHT_PF */
		else if ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) != 0)
			return TX_AMPDU_FRAME;
		else if(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED)
		)
			return TX_AMSDU_FRAME;
		else
			TxFrameType = TX_LEGACY_FRAME;
	}
#endif /* DOT11_N_SUPPORT */
	else
	{	/* it's a legacy b/g packet.*/
		if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE) && pAd->CommonCfg.bAggregationCapable) &&
			(RTMP_GET_PACKET_TXRATE(pPacket) >= RATE_6) &&
			(!(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)))
		)
		{	/* if peer support Ralink Aggregation, we use it.*/
			TxFrameType = TX_RALINK_FRAME;
		}
		else
		{
			TxFrameType = TX_LEGACY_FRAME;
		}
	}

	/* Currently, our fragment only support when a unicast packet send as NOT-ARALINK, NOT-AMSDU and NOT-AMPDU.*/
	if ((RTMP_GET_PACKET_FRAGMENTS(pPacket) > 1)
		 && (TxFrameType == TX_LEGACY_FRAME)
#ifdef DOT11_N_SUPPORT
		&& ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) == 0)
#endif /* DOT11_N_SUPPORT */
		)
		TxFrameType = TX_FRAG_FRAME;

	return TxFrameType;
}


BOOLEAN RTMP_FillTxBlkInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	pPacket = pTxBlk->pPacket;
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
#ifdef TX_PKT_SG
	NdisMoveMemory( &pTxBlk->pkt_info, &PacketInfo, sizeof(PacketInfo));
#endif /* TX_PKT_SG */
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->apidx = RTMP_GET_PACKET_IF(pPacket);
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
#ifdef CONFIG_AP_SUPPORT
	pTxBlk->pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);

	/* Default to clear this flag*/
	TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bForceNonQoS);

#ifdef WAPI_SUPPORT
	/* Check if this is an WPI data frame*/
	if ((RTMPIsWapiCipher(pAd, pTxBlk->apidx) == TRUE) &&
		 (RTMP_GET_PACKET_WAI(pTxBlk->pPacket) == FALSE))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bWPIDataFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWPIDataFrame);
#endif /* WAPI_SUPPORT */
	
	if (pTxBlk->Wcid == MCAST_WCID)
	{
		pTxBlk->pMacEntry = NULL;
		{
#ifdef MCAST_RATE_SPECIFIC
			PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
			if (((*pDA & 0x01) == 0x01) && (*pDA != 0xff))
				pTxBlk->pTransmit = &pAd->CommonCfg.MCastPhyMode;
			else
#endif /* MCAST_RATE_SPECIFIC */
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;
		}
		
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);	/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAllowFrag);
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
		if (RTMP_GET_PACKET_MOREDATA(pPacket))
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
		}
	}
	else
	{
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;

		pMacEntry = pTxBlk->pMacEntry;
#ifdef CONFIG_AP_SUPPORT
		pTxBlk->pMbss = pMacEntry->pMbss;
#endif /* CONFIG_AP_SUPPORT */
		
		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
		if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		else
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);


		{
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef WDS_SUPPORT
				if(IS_ENTRY_WDS(pMacEntry))
				{
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWDSEntry);
				}
				else
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
				if(IS_ENTRY_APCLI(pMacEntry))
				{
					PNDIS_PACKET 	apCliPkt = NULL;

#ifdef MAC_REPEATER_SUPPORT
					if ((pAd->ApCfg.bMACRepeaterEn) && (pMacEntry->bReptCli))
					{
						UCHAR tmpIdx;

						pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;

						if(pAd->ApCfg.MACRepeaterOuiMode != 1)
						{
						tmpIdx = (64 + (MAX_EXT_MAC_ADDR_SIZE * pMacEntry->MatchAPCLITabIdx) + pMacEntry->MatchReptCliIdx);
						apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
					}
					}
					else
#endif /* MAC_REPEATER_SUPPORT */
					{
						/* For each tx packet, update our MAT convert engine databases.*/
						apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->MatchAPCLITabIdx, pTxBlk->OpMode);
					}

					if(apCliPkt)
					{
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
						pPacket = apCliPkt;
						RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
						pTxBlk->pPacket = apCliPkt;
					}
					
					pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->MatchAPCLITabIdx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);

				}
				else
#endif /* APCLI_SUPPORT */
#ifdef CLIENT_WDS
				if (IS_ENTRY_CLIWDS(pMacEntry))
				{
					PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
					PUCHAR pSA = GET_OS_PKT_DATAPTR(pPacket) + MAC_ADDR_LEN;
					if (((pMacEntry->apidx < MAX_MBSSID_NUM(pAd))
						&& !MAC_ADDR_EQUAL(pSA, pAd->ApCfg.MBSSID[pMacEntry->apidx].Bssid))
						|| !MAC_ADDR_EQUAL(pDA, pMacEntry->Addr)
						)
					{
						TX_BLK_SET_FLAG(pTxBlk, fTX_bClientWDSFrame);
					}
				}
				else
#endif /* CLIENT_WDS */
				if (IS_ENTRY_CLIENT(pMacEntry))
				{ }
				else
					return FALSE;
					
				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
#endif /* CONFIG_AP_SUPPORT */

		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME)
		{
			if ( (RTMP_GET_PACKET_LOWRATE(pPacket)) ||
				((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			)
			{	/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate.*/
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;

#ifdef WAPI_SUPPORT
				/* 	According to WAPIA certification description, WAI packets can not
					include QoS header */
				if (RTMP_GET_PACKET_WAI(pTxBlk->pPacket))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
					TX_BLK_SET_FLAG(pTxBlk, fTX_bForceNonQoS);
				}
#endif /* WAPI_SUPPORT */		
#ifdef DOT11_N_SUPPORT
				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (IS_HT_STA(pTxBlk->pMacEntry) &&
					(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
					((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
					TX_BLK_SET_FLAG(pTxBlk, fTX_bForceNonQoS);
				}
#endif /* DOT11_N_SUPPORT */
			}
			
#ifdef DOT11_N_SUPPORT
			if ( (IS_HT_RATE(pMacEntry) == FALSE) &&
				(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE)))
			{	/* Currently piggy-back only support when peer is operate in b/g mode.*/
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}
#endif /* DOT11_N_SUPPORT */

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
			}
#ifdef UAPSD_SUPPORT
			if (RTMP_GET_PACKET_EOSP(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);
			}
#endif /* UAPSD_SUPPORT */
		}
		else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);
		}
		
		pMacEntry->DebugTxCount++;
	}


	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;

	return TRUE;
}


BOOLEAN CanDoAggregateTransmit(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_PACKET *pPacket,
	IN TX_BLK		*pTxBlk)
{
	int minLen = LENGTH_802_3;

	/*DBGPRINT(RT_DEBUG_TRACE, ("Check if can do aggregation! TxFrameType=%d!\n", pTxBlk->TxFrameType));*/
	
	if (RTMP_GET_PACKET_WCID(pPacket) == MCAST_WCID)
		return FALSE;

	if (RTMP_GET_PACKET_DHCP(pPacket) ||
		RTMP_GET_PACKET_EAPOL(pPacket) ||
		RTMP_GET_PACKET_WAI(pPacket)
	)
		return FALSE;
	
	/* Make sure the first packet has non-zero-length data payload */
	if (RTMP_GET_PACKET_VLAN(pPacket))
		minLen += LENGTH_802_1Q; /* VLAN tag */
	else if (RTMP_GET_PACKET_LLCSNAP(pPacket))
		minLen += 8; /* SNAP hdr Len*/
	if (minLen >= GET_OS_PKT_LEN(pPacket))
		return FALSE;
	
	if ((pTxBlk->TxFrameType == TX_AMSDU_FRAME) &&
		((pTxBlk->TotalFrameLen + GET_OS_PKT_LEN(pPacket))> (RX_BUFFER_AGGRESIZE - 100)))
	{	/* For AMSDU, allow the packets with total length < max-amsdu size*/
		return FALSE;
	}
	
	if ((pTxBlk->TxFrameType == TX_RALINK_FRAME) &&
		(pTxBlk->TxPacketList.Number == 2))
	{	/* For RALINK-Aggregation, allow two frames in one batch.*/
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	if ((MAC_ADDR_EQUAL(GET_OS_PKT_DATAPTR(pTxBlk->pPacket), GET_OS_PKT_DATAPTR(pPacket))) && (pAd->OpMode == OPMODE_AP)) /* unicast to same STA*/
		return TRUE;
	else
#endif /* CONFIG_AP_SUPPORT */
		return FALSE;
	
}


/*
	========================================================================

	Routine Description:
		To do the enqueue operation and extract the first item of waiting
		list. If a number of available shared memory segments could meet
		the request of extracted item, the extracted item will be fragmented
		into shared memory segments.
	
	Arguments:
		pAd Pointer to our adapter
		pQueue		Pointer to Waiting Queue
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
VOID RTMPDeQueuePacket(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN bIntContext,
	IN UCHAR QIdx,
	IN INT Max_Tx_Packets)
{
	PQUEUE_ENTRY pEntry = NULL;
	PNDIS_PACKET pPacket;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UCHAR Count=0;
	PQUEUE_HEADER   pQueue;
	ULONG FreeNumber[NUM_OF_TX_RING];
	UCHAR QueIdx, sQIdx, eQIdx;
	unsigned long	IrqFlags = 0;
	BOOLEAN hasTxDesc = FALSE;
	TX_BLK TxBlk, *pTxBlk;

#ifdef DBG_DIAGNOSE
	BOOLEAN			firstRound;
#if defined(DBG_TX_RING_DEPTH) || defined(DBG_TXQ_DEPTH)
	RtmpDiagStruct	*pDiagStruct = &pAd->DiagStruct;
#endif
#endif
#if defined (LED_SOFT_SUPPORT)
	ULONG HardTxTotal = 0;
#endif

	if (QIdx == NUM_OF_TX_RING)
	{
		sQIdx = 0;
		eQIdx = 3;	/* 4 ACs, start from 0.*/
	}
	else
	{
		sQIdx = eQIdx = QIdx;
	}

	for (QueIdx=sQIdx; QueIdx <= eQIdx; QueIdx++)
	{
		Count=0;

		RTMP_START_DEQUEUE(pAd, QueIdx, IrqFlags);

#ifdef DBG_DIAGNOSE
		firstRound = ((QueIdx == 0) ? TRUE : FALSE);
#endif /* DBG_DIAGNOSE */

		while (1)
		{
			if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS |
										fRTMP_ADAPTER_RADIO_OFF |
										fRTMP_ADAPTER_RESET_IN_PROGRESS |
										fRTMP_ADAPTER_HALT_IN_PROGRESS |
										fRTMP_ADAPTER_NIC_NOT_EXIST |
										fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET)))
				)
			{
				RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);
				return;
			}
			
			if (Count >= Max_Tx_Packets)
				break;

			DEQUEUE_LOCK(&pAd->irq_lock, bIntContext, IrqFlags);
			if (&pAd->TxSwQueue[QueIdx] == NULL)
			{
#ifdef DBG_DIAGNOSE
#ifdef DBG_TXQ_DEPTH
				if (firstRound == TRUE)
					pDiagStruct->diag_info[pDiagStruct->ArrayCurIdx].TxSWQueCnt[0]++;
#endif /* DBG_TXQ_DEPTH */
#endif /* DBG_DIAGNOSE */
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}

#ifdef RTMP_MAC_PCI
			FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);

#ifdef DBG_DIAGNOSE
#if defined(DBG_TX_RING_DEPTH) || defined(DBG_TXQ_DEPTH)
			if (firstRound == TRUE)
			{
				UCHAR	txDescNumLevel, txSwQNumLevel;
#ifdef DBG_TX_RING_DEPTH
				txDescNumLevel = (TX_RING_SIZE - FreeNumber[QueIdx]); /* Number of occupied hw desc.*/
				txDescNumLevel = ((txDescNumLevel <=15) ? txDescNumLevel : 15);
				pDiagStruct->diag_info[pDiagStruct->ArrayCurIdx].TxDescCnt[txDescNumLevel]++;
#endif /* DBG_TX_RING_DEPTH */
#ifdef DBG_TXQ_DEPTH
				txSwQNumLevel = ((pAd->TxSwQueue[QueIdx].Number <=7) ? pAd->TxSwQueue[QueIdx].Number : 8);
				pDiagStruct->diag_info[pDiagStruct->ArrayCurIdx].TxSWQueCnt[txSwQNumLevel]++;
#endif /* DBG_TXQ_DEPTH */
				firstRound = FALSE;
			}
#endif /* defined(DBG_TX_RING_DEPTH) || defined(DBG_TXQ_DEPTH) */
#endif /* DBG_DIAGNOSE */

			/* We should never let more than 64kBytes of tx data not be
			 * cleaned up, or the TCP TX throughput will suffer as the
			 * window will not fill and so expand. 64kBytes ~ 44 full
			 * size ethernet packets. Use 32 as a nice constant. There
			 * is also a timer function which prevents packets getting
			 * "stuck" when data stops flowing
			 */

			if ((TX_RING_SIZE - FreeNumber[QueIdx]) > 32)
			{
				/* free Tx(QueIdx) resources*/
				RTMPFreeTXDUponTxDmaDone(pAd, QueIdx);
				FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
			}
#endif /* RTMP_MAC_PCI */

			/* probe the Queue Head*/
			pQueue = &pAd->TxSwQueue[QueIdx];
			if ((pEntry = pQueue->Head) == NULL)
			{
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}

			pTxBlk = &TxBlk;
			NdisZeroMemory((PUCHAR)pTxBlk, sizeof(TX_BLK));
			
			pTxBlk->QueIdx = QueIdx;

#ifdef VENDOR_FEATURE1_SUPPORT
			pTxBlk->HeaderBuf = (UCHAR *)pTxBlk->HeaderBuffer;
#endif /* VENDOR_FEATURE1_SUPPORT */

			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);

#ifdef CONFIG_AP_SUPPORT
			if (RTMP_GET_PACKET_MGMT_PKT(pPacket) == 1)
			{
				/* this is a management frame */
				NDIS_STATUS status;

				pEntry = RemoveHeadQueue(pQueue);

#ifdef RTMP_MAC_PCI
				if ((pAd->MACVersion == 0x28600100) ||
					(RTMP_GET_PACKET_MGMT_PKT_DATA_QUE(pPacket) == 1))
					status = MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);
				else
#endif /* RTMP_MAC_PCI */
					status = MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);

				if (status != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR,
							("Transmit queued management frame error!\n"));
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}

				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				Count++;
				continue;
			}
			else 
			{
				/*when WDS Jam happen, drop following 1min to HW TxRing Pkts*/
				MAC_TABLE_ENTRY *pMacEntry = NULL;
				UCHAR RAWcid;
				RAWcid = RTMP_GET_PACKET_WCID(pPacket);
				pMacEntry = &pAd->MacTab.Content[RAWcid];

#ifdef WDS_SUPPORT
				/*
					It WDS life checking.
					WDS need to check the peer is come back or not
					by sending few (2 ~3) WDS Packet out to peer.
					It must be checked first.
				*/
				if(IS_ENTRY_WDS(pMacEntry))
				{
					ULONG Now32;
					NdisGetSystemUpTime(&Now32);
					if(pMacEntry->LockEntryTx && RTMP_TIME_BEFORE(Now32, pMacEntry->TimeStamp_toTxRing + WDS_ENTRY_RETRY_INTERVAL))
					{
						pEntry = RemoveHeadQueue(pQueue);
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
						Count++;
						continue;
					}
					else
						NdisGetSystemUpTime(&pMacEntry->TimeStamp_toTxRing);
				}
				else
#endif /* WDS_SUPPORT */
				if (!IS_ENTRY_NONE(pMacEntry)
					&& (pMacEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck))
				{
					/*
						Sample Lin, 20100412

						For non-WDS interface, we need to send packet to detect
						the link periodically; Or when
						pMacEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck,
						no any chance to clear pMacEntry->ContinueTxFailCnt.

						EX: When pMacEntry->ContinueTxFailCnt >=
						pAd->ApCfg.EntryLifeCheck, the condition will not be
						removed and we will drop all packets for the pEntry.
						But maybe the signal becomes better.
						So we try to send a packet periodically and we will
						get the tx status in tx done interrupt.
						If the tx status is success, pMacEntry->ContinueTxFailCnt
						will be cleared to 0.
					*/
#define ENTRY_RETRY_INTERVAL	(100 * OS_HZ / 1000)
					ULONG Now32;
					NdisGetSystemUpTime(&Now32);
					if(RTMP_TIME_BEFORE(Now32, pMacEntry->TimeStamp_toTxRing + ENTRY_RETRY_INTERVAL))
					{
						pEntry = RemoveHeadQueue(pQueue);
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
						Count++;
						continue;
					}
					else
						NdisGetSystemUpTime(&pMacEntry->TimeStamp_toTxRing);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
			
			/* Early check to make sure we have enoguh Tx Resource.*/
			hasTxDesc = RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
			if (!hasTxDesc)
			{
				pAd->PrivateInfo.TxRingFullCnt++;

				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				
				break;
			}

			pTxBlk->TxFrameType = TxPktClassification(pAd, pPacket);
			pEntry = RemoveHeadQueue(pQueue);
			pTxBlk->TotalFrameNum++;
			pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	/* The real fragment number maybe vary*/
			pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);
			pTxBlk->pPacket = pPacket;
			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));

			if (pTxBlk->TxFrameType & (TX_RALINK_FRAME | TX_AMSDU_FRAME))
			{
				// Enhance SW Aggregation Mechanism
				if (NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, FreeNumber[QueIdx], pTxBlk->TxFrameType))
				{
					InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
					DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
					break;
				}
			}


			if (pTxBlk->TxFrameType == TX_RALINK_FRAME || pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			{
				// Enhance SW Aggregation Mechanism
				if (NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, FreeNumber[QueIdx], pTxBlk->TxFrameType))
				{
					InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
					DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
					break;
				}

				do{
					if((pEntry = pQueue->Head) == NULL)
						break;

					/* For TX_AMSDU_FRAME/TX_RALINK_FRAME, Need to check if next pakcet can do aggregation.*/
					pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
					FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
					hasTxDesc = RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
					if ((hasTxDesc == FALSE) || (CanDoAggregateTransmit(pAd, pPacket, pTxBlk) == FALSE))
						break;

					/*Remove the packet from the TxSwQueue and insert into pTxBlk*/
					pEntry = RemoveHeadQueue(pQueue);
					ASSERT(pEntry);
					pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);


					pTxBlk->TotalFrameNum++;
					pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	/* The real fragment number maybe vary*/
					pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);
					InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
				}while(1);

				if (pTxBlk->TxPacketList.Number == 1)
					pTxBlk->TxFrameType = TX_LEGACY_FRAME;
			}

					
			Count += pTxBlk->TxPacketList.Number;

				/* Do HardTransmit now.*/
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				Status = APHardTransmit(pAd, pTxBlk, QueIdx);
#endif /* CONFIG_AP_SUPPORT */

#ifdef RTMP_MAC_PCI
			DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
			/* static rate also need NICUpdateFifoStaCounters() function.*/
			/*if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))*/
#ifdef VENDOR_FEATURE1_SUPPORT
			if (++pAd->FifoUpdateDone >= 4)
			{
				NICUpdateFifoStaCounters(pAd);
				pAd->FifoUpdateDone = 0;
			}
#else
			NICUpdateFifoStaCounters(pAd);
#endif /* VENDOR_FEATURE1_SUPPORT */
#endif /* RTMP_MAC_PCI */

#if defined (LED_SOFT_SUPPORT)
			HardTxTotal++;
#endif
		}

		RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);

		
#ifdef BLOCK_NET_IF
		if ((pAd->blockQueueTab[QueIdx].SwTxQueueBlockFlag == TRUE)
			&& (pAd->TxSwQueue[QueIdx].Number < 1))
		{
			releaseNetIf(&pAd->blockQueueTab[QueIdx]);
		}
#endif /* BLOCK_NET_IF */
	}

#if defined (LED_SOFT_SUPPORT)
	if (HardTxTotal > 0 && pAd->MacTab.Size > 0)
		ralink_gpio_led_blink(LED_SOFT_BLINK_GPIO);
#endif
}


/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.
		
	Arguments:
		pAd 	Pointer to our adapter
		Rate			Transmit rate
		Size			Frame size in units of byte
		
	Return Value:
		Duration number in units of usec

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
USHORT	RTMPCalcDuration(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Rate,
	IN	ULONG			Size)
{
	ULONG	Duration = 0;

	if (Rate < RATE_FIRST_OFDM_RATE) /* CCK*/
	{
		if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
			Duration = 96;	/* 72+24 preamble+plcp*/
		else
			Duration = 192; /* 144+48 preamble+plcp*/

		Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);
		if ((Size << 4) % RateIdTo500Kbps[Rate])
			Duration ++;
	}
	else if (Rate <= RATE_LAST_OFDM_RATE)/* OFDM rates*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
		Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);
		if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
			Duration += 4;
	}
	else	/*mimo rate*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
	}
	
	return (USHORT)Duration;
}


/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission
		
	Arguments:
		pAd 	Pointer to our adapter
		
	Return Value:
		None
		
	Note:
	
	========================================================================
*/
VOID RTMPSuspendMsduTransmission(
	IN PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("SCANNING, suspend MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier */
	/* no carrier detection when scanning */
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
	{
		CarrierDetectionStop(pAd);
	}
#endif
#endif /* CONFIG_AP_SUPPORT */
	
	/*
		Before BSS_SCAN_IN_PROGRESS, we need to keep Current R66 value and
		use Lowbound as R66 value on ScanNextChannel(...)
	*/
	rtmp_bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);

	pAd->hw_cfg.bbp_bw = pAd->CommonCfg.BBPCurrentBW;

	RTMPSetAGCInitValue(pAd, BW_20);
	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
	/* abort all TX rings */
	/*RTMP_IO_WRITE32(pAd, TX_CNTL_CSR, 0x000f0000);	*/
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission
		
	Arguments:
		pAd 	Pointer to our adapter
		
	Return Value:
		None
		
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
VOID RTMPResumeMsduTransmission(
	IN PRTMP_ADAPTER pAd)
{  
	DBGPRINT(RT_DEBUG_TRACE,("SCAN done, resume MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
	/* no carrier detection when scanning*/
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
			CarrierDetectionStart(pAd);
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
		R66 should not be 0
	*/
	if (pAd->BbpTuning.R66CurrentValue == 0)
	{
		pAd->BbpTuning.R66CurrentValue = 0x38;
		DBGPRINT_ERR(("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}
	rtmp_bbp_set_agc(pAd, pAd->BbpTuning.R66CurrentValue, RX_CHAIN_ALL);
	
	pAd->CommonCfg.BBPCurrentBW = pAd->hw_cfg.bbp_bw;
	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
/* sample, for IRQ LOCK to SEM LOCK */
/*
	IrqState = pAd->irq_disabled;
	if (IrqState)
		RTMPDeQueuePacket(pAd, TRUE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	else
*/
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}


#ifdef DOT11_N_SUPPORT
UINT deaggregate_AMSDU_announce(
	IN	PRTMP_ADAPTER	pAd,
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			OpMode)
{
	USHORT 			PayloadSize;
	USHORT 			SubFrameSize;
	PHEADER_802_3 	pAMSDUsubheader;
	UINT			nMSDU;
    UCHAR			Header802_3[14];

	PUCHAR			pPayload, pDA, pSA, pRemovedLLCSNAP;
	PNDIS_PACKET	pClonePacket;

#ifdef CONFIG_AP_SUPPORT
	UCHAR FromWhichBSSID = RTMP_GET_PACKET_IF(pPacket);
	UCHAR VLAN_Size;
	USHORT VLAN_VID = 0, VLAN_Priority = 0;


	if ((FromWhichBSSID < pAd->ApCfg.BssidNum)
		)
		VLAN_Size = (pAd->ApCfg.MBSSID[FromWhichBSSID].VLAN_VID != 0) ? LENGTH_802_1Q : 0;
#ifdef WDS_VLAN_SUPPORT
	else if ((FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS) &&
			(FromWhichBSSID < (MIN_NET_DEVICE_FOR_WDS + MAX_WDS_ENTRY)))
	{
		VLAN_Size = (pAd->WdsTab.\
						WdsEntry[FromWhichBSSID - MIN_NET_DEVICE_FOR_WDS].\
							VLAN_VID != 0) ? LENGTH_802_1Q : 0;
	}
#endif /* WDS_VLAN_SUPPORT */
	else /* only MBssid support VLAN.*/
		VLAN_Size = 0;
#endif /* CONFIG_AP_SUPPORT */

	nMSDU = 0;

	while (DataSize > LENGTH_802_3)
	{

		nMSDU++;

		/*hex_dump("subheader", pData, 64);*/
		pAMSDUsubheader = (PHEADER_802_3)pData;
		/*pData += LENGTH_802_3;*/
		PayloadSize = pAMSDUsubheader->Octet[1] + (pAMSDUsubheader->Octet[0]<<8);
		SubFrameSize = PayloadSize + LENGTH_802_3;


		if ((DataSize < SubFrameSize) || (PayloadSize > 1518 ))
		{
			break;
		}

		/*DBGPRINT(RT_DEBUG_TRACE,("%d subframe: Size = %d\n",  nMSDU, PayloadSize));*/

		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;

		/* convert to 802.3 header*/
        CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UCHAR	WhichBSSID = FromWhichBSSID;
			if (pRemovedLLCSNAP)
			{
				pPayload -= (LENGTH_802_3 + VLAN_Size);
				PayloadSize += (LENGTH_802_3 + VLAN_Size);
				/*NdisMoveMemory(pPayload, &Header802_3, LENGTH_802_3);*/
			}
			else
			{
				pPayload -= VLAN_Size;
				PayloadSize += VLAN_Size;
			}

			MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, WhichBSSID);

#ifdef WDS_VLAN_SUPPORT
			if (VLAN_VID == 0) /* maybe WDS packet */
				WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */

			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									Header802_3, LENGTH_802_3, pPayload,
									FromWhichBSSID, TPID);
		}
#endif /* CONFIG_AP_SUPPORT */

		pClonePacket = ClonePacket(pAd, pPacket, pPayload, PayloadSize);
		if (pClonePacket)
		{
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pClonePacket, RTMP_GET_PACKET_IF(pPacket));
#endif /* CONFIG_AP_SUPPORT */
		}


		/* A-MSDU has padding to multiple of 4 including subframe header.*/
		/* align SubFrameSize up to multiple of 4*/
		SubFrameSize = (SubFrameSize+3)&(~0x3);


		if (SubFrameSize > 1528 || SubFrameSize < 32)
		{
			break;
		}

		if (DataSize > SubFrameSize)
		{
			pData += SubFrameSize;
			DataSize -= SubFrameSize;
		}
		else
		{
			/* end of A-MSDU*/
			DataSize = 0;
		}
	}
	
	/* finally release original rx packet*/
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

	return nMSDU;
}


UINT BA_Reorder_AMSDU_Annnounce(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			OpMode)
{
	PUCHAR			pData;
	USHORT			DataSize;
	UINT			nMSDU = 0;

	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);
	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);

	nMSDU = deaggregate_AMSDU_announce(pAd, pPacket, pData, DataSize, OpMode);

	return nMSDU;
}

VOID Indicate_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	UINT			nMSDU;

	RTMP_UPDATE_OS_PACKET_INFO(pAd, pRxBlk, FromWhichBSSID);
	RTMP_SET_PACKET_IF(pRxBlk->pRxPacket, FromWhichBSSID);
	nMSDU = deaggregate_AMSDU_announce(pAd, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize, pRxBlk->OpMode);
}
#endif /* DOT11_N_SUPPORT */


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID AssocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
	IN PUCHAR                     pAddr,
	IN USHORT                     CapabilityInfo,
	IN ULONG                      Timeout,
	IN USHORT                     ListenIntv)
{
	COPY_MAC_ADDR(AssocReq->Addr, pAddr);
	/* Add mask to support 802.11b mode only */
	AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; /* not cf-pollable, not cf-poll-request*/
	AssocReq->Timeout = Timeout;
	AssocReq->ListenIntv = ListenIntv;
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID DisassocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	IN PUCHAR pAddr,
	IN USHORT Reason)
{
	COPY_MAC_ADDR(DisassocReq->Addr, pAddr);
	DisassocReq->Reason = Reason;
}


BOOLEAN RTMPCheckEtherType(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	PMAC_TABLE_ENTRY pMacEntry,
	IN	UCHAR			OpMode,
	OUT PUCHAR pUserPriority,
	OUT PUCHAR pQueIdx)
{
	USHORT	TypeLen;
	UCHAR	Byte0, Byte1;
	PUCHAR	pSrcBuf;
	UINT32	pktLen;
	UINT16 	srcPort, dstPort;
#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif /* CONFIG_AP_SUPPORT */
	BOOLEAN bWmmReq;

#ifdef CONFIG_AP_SUPPORT
/*	if (IS_ENTRY_CLIENT(pMacEntry))*/
/*		 pMbss = pMacEntry->pMbss;*/
/*	else*/
/*		 pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];*/
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* for APClient, WDS, Mesh, they use MAIN BSS */
		UCHAR apidx;
		apidx = RTMP_GET_PACKET_NET_DEVICE(pPacket);
		if (apidx >= MAX_MBSSID_NUM(pAd))
			apidx = MAIN_MBSSID;
		pMbss = &pAd->ApCfg.MBSSID[apidx];
	}
#endif /* CONFIG_AP_SUPPORT */

	/*
		for bc/mc packets, if it has VLAN tag or DSCP field, we also need
		to get UP for IGMP use.
	*/
	bWmmReq = (
#ifdef CONFIG_AP_SUPPORT
				(
				(pMbss->bWmmCapable)) || 
#endif /* CONFIG_AP_SUPPORT */
				OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED))
				&& ((pMacEntry) &&
					((VALID_WCID(pMacEntry->Aid) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
						|| (pMacEntry->Aid == MCAST_WCID)));

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	pktLen = GET_OS_PKT_LEN(pPacket);

	ASSERT(pSrcBuf);

	RTMP_SET_PACKET_SPECIFIC(pPacket, 0);
	
	/* get Ethernet protocol field*/
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

	pSrcBuf += LENGTH_802_3;	/* Skip the Ethernet Header.*/
	
	if (TypeLen <= 1500)
	{	/* 802.3, 802.3 LLC*/
		/*
			DestMAC(6) + SrcMAC(6) + Lenght(2) +
			DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03)
		{
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			RTMP_SET_PACKET_LLCSNAP(pPacket, 1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += 8; /* Skip this LLC/SNAP header*/
		}
		else
		{
			/*It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it.*/
		}
	}
	
	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (TypeLen == 0x8100)
	{
#ifdef CONFIG_AP_SUPPORT
		USHORT VLAN_VID = 0;

		/* 0x8100 means VLAN packets */

		/* Dest. MAC Address (6-bytes) +
		   Source MAC Address (6-bytes) +
		   Length/Type = 802.1Q Tag Type (2-byte) +
		   Tag Control Information (2-bytes) +
		   Length / Type (2-bytes) +
		   data payload (0-n bytes) +
		   Pad (0-p bytes) +
		   Frame Check Sequence (4-bytes) */

		/* No matter unicast or multicast */
		/*if (IS_ENTRY_CLIENT(pMacEntry))*/
#ifdef WDS_VLAN_SUPPORT
		if (IS_ENTRY_WDS(pMacEntry))
		{
			UINT32 WdsId;

			WdsId = RTMP_GET_PACKET_NET_DEVICE(pPacket) - MIN_NET_DEVICE_FOR_WDS;
			if (WdsId < MAX_WDS_ENTRY)
				VLAN_VID = pAd->WdsTab.WdsEntry[WdsId].VLAN_VID;
		}
		else
#endif /* WDS_VLAN_SUPPORT */
		{
			VLAN_VID = pMbss->VLAN_VID;
		}

		if (VLAN_VID != 0)
		{
			/* check if the packet is my VLAN */
			/* VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID */
			USHORT vlan_id = *(USHORT *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & 0x0FFF; /* 12 bit */
			if (vlan_id != VLAN_VID)
			{
				/* not my VLAN packet, discard it */
				return FALSE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

		RTMP_SET_PACKET_VLAN(pPacket, 1);
		Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (USHORT)((Byte0 << 8) + Byte1);

		/* only use VLAN tag */
		if (bWmmReq)
		{
			*pUserPriority = (*pSrcBuf & 0xe0) >> 5;
			*pQueIdx = MapUserPriorityToAccessCategory[*pUserPriority];
		}

		pSrcBuf += 4; /* Skip the VLAN Header.*/
	}
	else if (TypeLen == 0x0800)
	{
		if (bWmmReq)
		{
			if ((*pSrcBuf & 0xf0) == 0x40) /* IPv4 */
			{
				/*
					Version - 4-bit Internet Protocol version number.
					Length - 4-bit IP header length.
					Traffic Class - 8-bit TOS field.
				*/
				*pUserPriority = (*(pSrcBuf + 1) & 0xe0) >> 5;
			}

			*pQueIdx = MapUserPriorityToAccessCategory[*pUserPriority];
		}
	}
	else if (TypeLen == 0x86dd)
	{
		if (bWmmReq)
		{
			if ((*pSrcBuf & 0xf0) == 0x60) /* IPv6 */
			{
				/*
					Version - 4-bit Internet Protocol version number.
					Traffic Class - 8-bit traffic class field.
				*/
				*pUserPriority = ((*pSrcBuf) & 0x0e) >> 1;
			}

			*pQueIdx = MapUserPriorityToAccessCategory[*pUserPriority];
		}
	}

	switch (TypeLen)
	{
		case 0x0800:
			{
				/* return AC_BE if packet is not IPv4*/
				if (bWmmReq && (*pSrcBuf & 0xf0) != 0x40)
				{
					*pUserPriority = 0;
					*pQueIdx = QID_AC_BE;
				}
				else
					RTMP_SET_PACKET_IPV4(pPacket, 1);

				ASSERT((pktLen > 34));
				if (*(pSrcBuf + 9) == 0x11)
				{	/* udp packet*/
					ASSERT((pktLen > 34));	/* 14 for ethernet header, 20 for IP header*/
					
					pSrcBuf += 20;	/* Skip the IP header*/
					srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
					dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf+2)));
		
					if ((srcPort==0x44 && dstPort==0x43) || (srcPort==0x43 && dstPort==0x44))
					{	/*It's a BOOTP/DHCP packet*/
						RTMP_SET_PACKET_DHCP(pPacket, 1);
					}
				}
			}
			break;
		case 0x86dd:
			{
				/* return AC_BE if packet is not IPv6 */
				if (bWmmReq &&
					((*pSrcBuf & 0xf0) != 0x60))
				{
					*pUserPriority = 0;
					*pQueIdx = QID_AC_BE;
				}
			}
			break;
		case 0x0806:
			{
				/*ARP Packet.*/
				RTMP_SET_PACKET_DHCP(pPacket, 1);
			}
			break;
		case 0x888e:
			{
				/* EAPOL Packet.*/
				RTMP_SET_PACKET_EAPOL(pPacket, 1);
			}
			break;
#ifdef WAPI_SUPPORT
		case 0x88b4:
			{
				/* WAI Packet.*/
				/*hex_dump("WAI Packet", pSrcBuf, pktLen);*/
				RTMP_SET_PACKET_WAI(pPacket, 1);
			}
			break;
#endif /* WAPI_SUPPORT */


		default:
			break;
	}

#ifdef VENDOR_FEATURE1_SUPPORT
	RTMP_SET_PACKET_PROTOCOL(pPacket, TypeLen);
#endif /* VENDOR_FEATURE1_SUPPORT */

	/* have to check ACM bit. downgrade UP & QueIdx before passing ACM*/
	/* NOTE: AP doesn't have to negotiate TSPEC. ACM is controlled purely via user setup, not protocol handshaking*/
	/*
		Under WMM ACM control, we dont need to check the bit;
		Or when a TSPEC is built for VO but we will change priority to
		BE here and when we issue a BA session, the BA session will
		be BE session, not VO session.
	*/
	if (pAd->CommonCfg.APEdcaParm.bACM[*pQueIdx])
	{
		*pUserPriority = 0;
		*pQueIdx		 = QID_AC_BE;
	}

	return TRUE;
}


VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN RXWI_STRUC *pRxWI)
{
	CHAR rssi[3] = {0};
	UCHAR snr[3] = {0};
	BOOLEAN bInitial = FALSE;
	CHAR Phymode = get_pkt_phymode_by_rxwi(pRxWI);

	if (!(pRssi->AvgRssi0 | pRssi->AvgRssi0X8 | pRssi->LastRssi0))
		bInitial = TRUE;

	get_pkt_rssi_by_rxwi(pRxWI, 3, &rssi[0]);
	get_pkt_snr_by_rxwi(pRxWI, 3, &snr[0]);

	if (rssi[0] != 0)
	{
		pRssi->LastRssi0 = ConvertToRssi(pAd, (CHAR)rssi[0], RSSI_0);
		if (bInitial)
		{
			pRssi->AvgRssi0X8 = pRssi->LastRssi0 << 3;
			pRssi->AvgRssi0  = pRssi->LastRssi0;
		}
		else
			pRssi->AvgRssi0X8 = (pRssi->AvgRssi0X8 - pRssi->AvgRssi0) + pRssi->LastRssi0;

		pRssi->AvgRssi0 = pRssi->AvgRssi0X8 >> 3;
	}

	if (snr[0] != 0 && Phymode != MODE_CCK)
	{
		pRssi->LastSnr0 = ConvertToSnr(pAd, (UCHAR)snr[0]);
		if (bInitial)
		{
			pRssi->AvgSnr0X8 = pRssi->LastSnr0 << 3;
			pRssi->AvgSnr0  = pRssi->LastSnr0;
		}
		else
			pRssi->AvgSnr0X8 = (pRssi->AvgSnr0X8 - pRssi->AvgSnr0) + pRssi->LastSnr0;

		pRssi->AvgSnr0 = pRssi->AvgSnr0X8 >> 3;
	}

	if (rssi[1] != 0)
	{
		pRssi->LastRssi1 = ConvertToRssi(pAd, (CHAR)rssi[1], RSSI_1);
		if (bInitial)
		{
			pRssi->AvgRssi1X8 = pRssi->LastRssi1 << 3;
			pRssi->AvgRssi1  = pRssi->LastRssi1;
		}
		else
			pRssi->AvgRssi1X8 = (pRssi->AvgRssi1X8 - pRssi->AvgRssi1) + pRssi->LastRssi1;

		pRssi->AvgRssi1 = pRssi->AvgRssi1X8 >> 3;
	}

	if (snr[1] != 0 && Phymode != MODE_CCK)
	{
		pRssi->LastSnr1 = ConvertToSnr(pAd, (UCHAR)snr[1]);
		if (bInitial)
		{
			pRssi->AvgSnr1X8 = pRssi->LastSnr1 << 3;
			pRssi->AvgSnr1  = pRssi->LastSnr1;
		}
		else
			pRssi->AvgSnr1X8 = (pRssi->AvgSnr1X8 - pRssi->AvgSnr1) + pRssi->LastSnr1;
		
		pRssi->AvgSnr1 = pRssi->AvgSnr1X8 >> 3;
	}

	if (rssi[2] != 0)
	{
		pRssi->LastRssi2 = ConvertToRssi(pAd, (CHAR)rssi[2], RSSI_2);

		if (bInitial)
		{
			pRssi->AvgRssi2X8 = pRssi->LastRssi2 << 3;
			pRssi->AvgRssi2  = pRssi->LastRssi2;
		}
		else
			pRssi->AvgRssi2X8 = (pRssi->AvgRssi2X8 - pRssi->AvgRssi2) + pRssi->LastRssi2;

		pRssi->AvgRssi2 = pRssi->AvgRssi2X8 >> 3;
	}
}

/* Normal legacy Rx packet indication*/
VOID Indicate_Legacy_Packet(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Indicate_Legacy_Packet", pRxBlk->pData, pRxBlk->DataSize);
	hex_dump("802_11_hdr", (UCHAR *)pRxBlk->pHeader, LENGTH_802_11);
}
//---Add by shiang for debug
#endif

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_AP_SUPPORT */

	if (pRxBlk->DataSize > MAX_RX_PKT_LEN)
	{

		/* release packet*/
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	STATS_INC_RX_PACKETS(pAd, FromWhichBSSID);


#ifdef CONFIG_AP_SUPPORT
	MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Before80211_2_8023", pRxBlk->pData, pRxBlk->DataSize);
	hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
}
//---Add by shiang for debug
#endif
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, FromWhichBSSID, TPID);
#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), GET_OS_PKT_LEN(pRxBlk->pRxPacket));
}
//---Add by shiang for debug
#endif

	
	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */
		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != 1))
		{
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
			
			/*For UDP packet, we need to check about the DHCP packet, to modify the flag of DHCP discovey/request as broadcast. */
			if (*(pLayerHdr + 9) == 0x11)
			{
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;
				
				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr+2)));

				if (srcPort==67 && dstPort==68) /*It's a DHCP packet */
				{
					PUCHAR bootpHdr, dhcpHdr, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					bootpHdr = pUdpHdr + 8;
					dhcpHdr = bootpHdr + 236;
					pCliHwAddr = (bootpHdr+28);

					DBGPRINT(RT_DEBUG_ERROR, ("pCliHwAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
										pCliHwAddr[0], pCliHwAddr[1], pCliHwAddr[2],
										pCliHwAddr[3], pCliHwAddr[4], pCliHwAddr[5]));
					
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr);
					if (pReptEntry)
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);

					DBGPRINT(RT_DEBUG_ERROR, ("pCliHwAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
										pCliHwAddr[0], pCliHwAddr[1], pCliHwAddr[2],
										pCliHwAddr[3], pCliHwAddr[4], pCliHwAddr[5]));
					bHdrChanged = TRUE;
				}
				
				if (bHdrChanged == TRUE)
					NdisZeroMemory((pUdpHdr+6), 2); /*modify the UDP chksum as zero */
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
	}
#endif /* CONFIG_AP_SUPPORT */

}


#ifdef HDR_TRANS_SUPPORT
/* Normal legacy Rx packet indication*/
VOID Indicate_Legacy_Packet_Hdr_Trns(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;

	struct sk_buff *pOSPkt;

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Indicate_Legacy_Packet", pRxBlk->pTransData, pRxBlk->TransDataSize);
	hex_dump("802_11_hdr", pRxBlk->pHeader, LENGTH_802_11);
}
//---Add by shiang for debug
#endif

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/

	if (pRxBlk->TransDataSize > 1514 )
	{

		/* release packet*/
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}


	STATS_INC_RX_PACKETS(pAd, FromWhichBSSID);


#ifdef CONFIG_AP_SUPPORT
	MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Before80211_2_8023", pRxBlk->pData, pRxBlk->TransDataSize);
	hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
}
//---Add by shiang for debug
#endif



	{
		pOSPkt = RTPKT_TO_OSPKT(pRxPacket);

		/*get_netdev_from_bssid(pAd, FromWhichBSSID); */
		pOSPkt->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
		pOSPkt->data = pRxBlk->pTransData;
		pOSPkt->len = pRxBlk->TransDataSize;
		pOSPkt->tail = pOSPkt->data + pOSPkt->len;
		//printk("\x1b[31m%s: rx trans ...%d\x1b[m\n", __FUNCTION__, __LINE__);
	}

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), GET_OS_PKT_LEN(pRxBlk->pRxPacket));
}
//---Add by shiang for debug
#endif

	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */
		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != 1))
		{
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
			
			/*For UDP packet, we need to check about the DHCP packet, to modify the flag of DHCP discovey/request as broadcast. */
			if (*(pLayerHdr + 9) == 0x11)
			{
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;
				
				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr+2)));

				if (srcPort==67 && dstPort==68) /*It's a DHCP packet */
				{
					PUCHAR bootpHdr, dhcpHdr, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					bootpHdr = pUdpHdr + 8;
					dhcpHdr = bootpHdr + 236;
					pCliHwAddr = (bootpHdr+28);

					DBGPRINT(RT_DEBUG_ERROR, ("pCliHwAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
										pCliHwAddr[0], pCliHwAddr[1], pCliHwAddr[2],
										pCliHwAddr[3], pCliHwAddr[4], pCliHwAddr[5]));
					
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr);
					if (pReptEntry)
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);

					DBGPRINT(RT_DEBUG_ERROR, ("pCliHwAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
										pCliHwAddr[0], pCliHwAddr[1], pCliHwAddr[2],
										pCliHwAddr[3], pCliHwAddr[4], pCliHwAddr[5]));
					bHdrChanged = TRUE;
				}
				
				if (bHdrChanged == TRUE)
					NdisZeroMemory((pUdpHdr+6), 2); /*modify the UDP chksum as zero */
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
	}
#endif /* CONFIG_AP_SUPPORT */

}
#endif /* HDR_TRANS_SUPPORT */


/* Normal, AMPDU or AMSDU*/
VOID CmmRxnonRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
#ifdef DOT11_N_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
		{
			/* handle A-MSDU*/
			Indicate_AMSDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
	}
}


/* Normal, AMPDU or AMSDU*/
#ifdef HDR_TRANS_SUPPORT
VOID CmmRxnonRalinkFrameIndicate_Hdr_Trns(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
#ifdef DOT11_N_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		Indicate_AMPDU_Packet_Hdr_Trns(pAd, pRxBlk, FromWhichBSSID);
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
		{
			/* handle A-MSDU*/
			Indicate_AMSDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
			Indicate_Legacy_Packet_Hdr_Trns(pAd, pRxBlk, FromWhichBSSID);
		}
	}
}
#endif /* HDR_TRANS_SUPPORT */


VOID CmmRxRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	UCHAR			Header802_3[LENGTH_802_3];
	UINT16			Msdu2Size;
	UINT16 			Payload1Size, Payload2Size;
	PUCHAR 			pData2;
	PNDIS_PACKET	pPacket2 = NULL;
	USHORT			VLAN_VID = 0, VLAN_Priority = 0;


	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData+1) << 8);

	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize))
	{
		/* skip two byte MSDU2 len */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}
	else
	{
		/* release packet*/
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* get 802.3 Header and  remove LLC*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_AP_SUPPORT */


	ASSERT(pRxBlk->pRxPacket);

	/* Ralink Aggregation frame*/
	pAd->RalinkCounters.OneSecRxAggregationCount ++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;

	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		USHORT VLAN_VID = 0, VLAN_Priority = 0;

		MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */

		RT_VLAN_PKT_DUPLICATE(pPacket2, pAd, VLAN_VID, VLAN_Priority,
							(pData2-LENGTH_802_3), LENGTH_802_3, pData2,
							Payload2Size, FromWhichBSSID, TPID);
	}
#endif /* CONFIG_AP_SUPPORT */

	if (!pPacket2)
	{
		/* release packet*/
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* update payload size of 1st packet*/
	pRxBlk->DataSize = Payload1Size;
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
		if (VLAN_VID == 0) /* maybe WDS packet */
			WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, FromWhichBSSID, TPID);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxBlk->pRxPacket, FromWhichBSSID);
#endif /* CONFIG_AP_SUPPORT */

	if (pPacket2)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pPacket2, FromWhichBSSID);
#endif /* CONFIG_AP_SUPPORT */
	}
}


#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
	}


PNDIS_PACKET RTMPDeFragmentDataFrame(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	PNDIS_PACKET pRetPacket = NULL;
	UCHAR *pFragBuffer = NULL;
	BOOLEAN bReassDone = FALSE;
	UCHAR HeaderRoom = 0;


	ASSERT(pHeader);

	HeaderRoom = pData - (UCHAR *)pHeader;

	/* Re-assemble the fragmented packets*/
	if (pHeader->Frag == 0)		/* Frag. Number is 0 : First frag or only one pkt*/
	{
		/* the first pkt of fragment, record it.*/
		if (pHeader->FC.MoreFrag)
		{
			ASSERT(pAd->FragFrame.pFragPacket);
			pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
			pAd->FragFrame.RxSize   = DataSize + HeaderRoom;
			NdisMoveMemory(pFragBuffer,	 pHeader, pAd->FragFrame.RxSize);
			pAd->FragFrame.Sequence = pHeader->Sequence;
			pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Should be 0*/
			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	/* end of processing this frame*/
		}
	}
	else	/*Middle & End of fragment*/
	{
		if ((pHeader->Sequence != pAd->FragFrame.Sequence) ||
			(pHeader->Frag != (pAd->FragFrame.LastFrag + 1)))
		{
			/* Fragment is not the same sequence or out of fragment number order*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment is not the same sequence or out of fragment number order.\n"));
			goto done; /* give up this frame*/
		}
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
		{
			/* Fragment frame is too large, it exeeds the maximum frame size.*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment frame is too large, it exeeds the maximum frame size.\n"));
			goto done; /* give up this frame*/
		}

        
		/* Broadcom AP(BCM94704AGR) will send out LLC in fragment's packet, LLC only can accpet at first fragment.*/
		/* In this case, we will dropt it.*/
		
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pHeader->Sequence, pHeader->Frag));
			goto done; /* give up this frame*/
		}

		pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);

		/* concatenate this fragment into the re-assembly buffer*/
		NdisMoveMemory((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Update fragment number*/

		/* Last fragment*/
		if (pHeader->FC.MoreFrag == FALSE)
		{
			bReassDone = TRUE;
		}
	}

done:
	/* always release rx fragmented packet*/
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

	/* return defragmented packet if packet is reassembled completely*/
	/* otherwise return NULL*/
	if (bReassDone)
	{
		PNDIS_PACKET pNewFragPacket;

		/* allocate a new packet buffer for fragment*/
		pNewFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
		if (pNewFragPacket)
		{
			/* update RxBlk*/
			pRetPacket = pAd->FragFrame.pFragPacket;
			pAd->FragFrame.pFragPacket = pNewFragPacket;
			pRxBlk->pHeader = (PHEADER_802_11) GET_OS_PKT_DATAPTR(pRetPacket);
			pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom;
			pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom;
			pRxBlk->pRxPacket = pRetPacket;
		}
		else
		{
			RESET_FRAGFRAME(pAd->FragFrame);
		}
	}

	return pRetPacket;
}

VOID Indicate_EAPOL_Packet(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT32 MaxWcidNum = MAX_LEN_OF_MAC_TABLE;

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn)
		MaxWcidNum = MAX_MAC_TABLE_SIZE_WITH_REPEATER;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (pRxBlk->pRxWI->RxWIWirelessCliID >= MaxWcidNum)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: invalid wcid.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->pRxWI->RxWIWirelessCliID];
	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: drop and release the invalid packet.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{		
		APRxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		return;
	}
#endif /* CONFIG_AP_SUPPORT */
	

}


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(
	IN RTMP_ADAPTER *pAd)
{
	UINT32  Offset;


	Offset = (pAd->TbttTickCount) % (BCN_TBTT_OFFSET);
	pAd->TbttTickCount++;

	/*
		The updated BeaconInterval Value will affect Beacon Interval after two TBTT
		beacasue the original BeaconInterval had been loaded into next TBTT_TIMER
	*/
	if (Offset == (BCN_TBTT_OFFSET-2))
	{
		BCN_TIME_CFG_STRUC csr;
		RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod << 4) - 1 ;	/* ASIC register in units of 1/16 TU = 64us*/
		RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
	}
	else
	{
		if (Offset == (BCN_TBTT_OFFSET-1))
		{
			BCN_TIME_CFG_STRUC csr;

			RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
			csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod) << 4; /* ASIC register in units of 1/16 TU*/
			RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
		}
	}
}

#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(
	IN  PRTMP_ADAPTER   pAd,
	IN	PTX_BLK			pTxBlk)
{
	PACKET_INFO		PacketInfo;
	UINT32	ex_head = 0, ex_tail = 0;
	UCHAR 	NumberOfFrag = RTMP_GET_PACKET_FRAGMENTS(pTxBlk->pPacket);

#ifdef WAPI_SUPPORT
	if (pTxBlk->CipherAlg == CIPHER_SMS4)
		ex_tail = LEN_WPI_MIC;
	else
#endif /* WAPI_SUPPORT */			
	if (pTxBlk->CipherAlg == CIPHER_AES)
		ex_tail = LEN_CCMP_MIC;

	ex_tail = (NumberOfFrag * ex_tail);

	pTxBlk->pPacket = ExpandPacket(pAd, pTxBlk->pPacket, ex_head, ex_tail);
	if (pTxBlk->pPacket == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: out of resource.\n", __FUNCTION__));
		return FALSE;
	}
	RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);									

	return TRUE;
}

VOID RTMPUpdateSwCacheCipherInfo(	
	IN  PRTMP_ADAPTER   pAd,
	IN	PTX_BLK			pTxBlk,
	IN	PUCHAR			pHdr)
{
	PHEADER_802_11 		pHeader_802_11;
	PMAC_TABLE_ENTRY	pMacEntry;

	pHeader_802_11 = (HEADER_802_11 *) pHdr;
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry && pHeader_802_11->FC.Wep && 
		CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))	
	{
		PCIPHER_KEY pKey = &pMacEntry->PairwiseKey;
	
		TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);

		pTxBlk->CipherAlg = pKey->CipherAlg;
		pTxBlk->pKey = pKey;
#ifdef WAPI_SUPPORT
		pTxBlk->KeyIdx = pMacEntry->usk_id;

		/* TSC increment pre encryption transmittion */
		if (pKey->CipherAlg == CIPHER_SMS4)						
			inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 2);			
		else 
#endif /* WAPI_SUPPORT */		
		if ((pKey->CipherAlg == CIPHER_WEP64) || (pKey->CipherAlg == CIPHER_WEP128))
			inc_iv_byte(pKey->TxTsc, LEN_WEP_TSC, 1);
		else if ((pKey->CipherAlg == CIPHER_TKIP) || (pKey->CipherAlg == CIPHER_AES))
			inc_iv_byte(pKey->TxTsc, LEN_WPA_TSC, 1);
		
	}

}

#endif /* SOFT_ENCRYPT */

/* 
	==========================================================================
	Description:
		Send out a NULL frame to a specified STA at a higher TX rate. The 
		purpose is to ensure the designated client is okay to received at this
		rate.
	==========================================================================
 */
VOID RtmpEnqueueNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         TxRate,
	IN UCHAR         PID,
	IN UCHAR         apidx,
    IN BOOLEAN       bQosNull,
    IN BOOLEAN       bEOSP,
    IN UCHAR         OldUP)
{
	NDIS_STATUS    NState;
	PHEADER_802_11 pNullFr;
	PUCHAR pFrame;
	ULONG		   Length;

	/* since TxRate may change, we have to change Duration each time */
	NState = MlmeAllocateMemory(pAd, (PUCHAR *)&pFrame);
	pNullFr = (PHEADER_802_11) pFrame;
    Length = sizeof(HEADER_802_11);

	if (NState == NDIS_STATUS_SUCCESS) 
	{
#ifdef CONFIG_AP_SUPPORT
		MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_NULL_FUNC, 0, pAddr, 
							pAd->ApCfg.MBSSID[apidx].Bssid);
#endif /* CONFIG_AP_SUPPORT */

		pNullFr->FC.Type = BTYPE_DATA;
		pNullFr->FC.FrDs = 1;
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, 14);


#ifdef UAPSD_SUPPORT
		if (bQosNull)
		{
			UCHAR *qos_p = ((UCHAR *)pNullFr) + Length;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			Length += 2;
		} /* End of if */
#endif /* UAPSD_SUPPORT */

		DBGPRINT(RT_DEBUG_INFO, ("send NULL Frame @%d Mbps to AID#%d...\n", RateIdToMbps[TxRate], PID & 0x3f));
		MiniportMMRequest(pAd, MapUserPriorityToAccessCategory[7], (PUCHAR)pNullFr, Length);
		MlmeFreeMemory(pAd, pFrame);
	}
}


#ifdef RLT_MAC
BOOLEAN CmdRspEventCallbackHandle(PRTMP_ADAPTER pAd, PUCHAR pRspBuffer)
{
	struct MCU_CTRL *MCtrl = &pAd->MCUCtrl;
	struct CMD_RSP_EVENT *CmdRspEvent, *CmdRspEventTmp;
	RXFCE_INFO_CMD *pFceInfo = (RXFCE_INFO_CMD *)pRspBuffer;



	if ((pFceInfo->info_type != CMD_PACKET))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Packet is not command response/event\n"));
		return NDIS_STATUS_FAILURE;
	} 

	if (pFceInfo->self_gen)
	{
		/* if have callback function */
		RTEnqueueInternalCmd(pAd, CMDTHREAD_RESPONSE_EVENT_CALLBACK, 
									pRspBuffer, 
									sizeof(*pFceInfo) + pFceInfo->pkt_len);
	}
	else
	{
		unsigned long IrqFlags;
		RTMP_IRQ_LOCK(&MCtrl->CmdRspEventListLock, IrqFlags);
		DlListForEachSafe(CmdRspEvent, CmdRspEventTmp, &MCtrl->CmdRspEventList, struct CMD_RSP_EVENT, List)
		{
			if (CmdRspEvent->CmdSeq == pFceInfo->cmd_seq)
			{
				
				if ((*CmdRspEvent->RspPayloadLen == pFceInfo->pkt_len) && (*CmdRspEvent->RspPayloadLen != 0))
				{
						NdisMoveMemory(*CmdRspEvent->RspPayload, pRspBuffer + sizeof(*pFceInfo),
												pFceInfo->pkt_len);
				}
				else if ((*CmdRspEvent->RspPayloadLen == 0) && (pFceInfo->pkt_len == 8))
				{
					DBGPRINT(RT_DEBUG_INFO, ("Command response(ack) success\n"));
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Expect response len(%d), Command response len(%d) invalid\n", *CmdRspEvent->RspPayloadLen, pFceInfo->pkt_len));
					*CmdRspEvent->RspPayloadLen = pFceInfo->pkt_len;
				}

				if (CmdRspEvent->NeedWait)
				{
#ifdef RTMP_PCI_SUPPORT
					CmdRspEvent->bAckDone = TRUE;
#endif /* RTMP_PCI_SUPPORT */
				}
				else
				{
					DlListDel(&CmdRspEvent->List);
					os_free_mem(NULL, CmdRspEvent);
				}

				break;
			}
		}
		RTMP_IRQ_UNLOCK(&MCtrl->CmdRspEventListLock, IrqFlags);
	}
	
	return NDIS_STATUS_SUCCESS;
}
#endif /* RLT_MAC */


#ifdef WFA_VHT_PF
#ifdef IP_ASSEMBLY
typedef union ip_flags_frag_offset {
	struct {
#ifdef RT_BIG_ENDIAN
		USHORT flags_reserved:1;
		USHORT flags_may_frag:1;
		USHORT flags_more_frag:1;
		USHORT frag_offset:13;
#else
		USHORT frag_offset:13;
		USHORT flags_more_frag:1;
		USHORT flags_may_frag:1;
		USHORT flags_reserved:1;
#endif
	} field;
	USHORT word;
} IP_FLAGS_FRAG_OFFSET;

typedef struct ip_v4_hdr {
#ifdef RT_BIG_ENDIAN
	UCHAR version:4, ihl:4;
#else
	UCHAR ihl:4, version:4;
#endif
	UCHAR tos;
	USHORT tot_len;
	USHORT identifier;
} IP_V4_HDR;


INT ip_assembly_timeout(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, ULONG Now, UCHAR QueIdx)
{
	QUEUE_HEADER *queue;
	ULONG *ip_pkt_jiffies;
	INT i;
	struct ip_frag_q *fragQ;
	
	if ((pEntry->ip_queue1[QueIdx].Number != 0)
		&& (RTMP_TIME_AFTER(Now, (pEntry->ip_pkt1_jiffies[QueIdx]) + (1000 * OS_HZ))))
	{
		PQUEUE_ENTRY pBackupPktEntry;
		PNDIS_PACKET pBackupPkt;

		while (1) {
			pBackupPktEntry = RemoveHeadQueue(&pEntry->ip_queue1[QueIdx]);
			if (pBackupPktEntry == NULL)
				break;
			pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
			RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
		}
		pEntry->ip_id1[QueIdx] = -1;
		pEntry->ip_id1_FragSize[QueIdx] - -1;
		pEntry->ip_pkt1_jiffies[QueIdx] = 0;
	}


	NdisGetSystemUpTime(&Now);
	if ((pEntry->ip_queue2[QueIdx].Number != 0)
		&& (RTMP_TIME_AFTER(Now, (pEntry->ip_pkt2_jiffies[QueIdx]) + (1000 * OS_HZ))))
	{
		PQUEUE_ENTRY pBackupPktEntry;
		PNDIS_PACKET pBackupPkt;

		while (1) {
			pBackupPktEntry = RemoveHeadQueue(&pEntry->ip_queue2[QueIdx]);
			if (pBackupPktEntry == NULL)
				break;
			pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
			RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
		}

		pEntry->ip_id2[QueIdx] = -1;
		pEntry->ip_id2_FragSize[QueIdx] - -1;
		pEntry->ip_pkt2_jiffies[QueIdx] = 0;
	}

	return TRUE;
}


INT ip_assembly(
	RTMP_ADAPTER *pAd,
	UCHAR QueIdx, 
	PNDIS_PACKET pPacket,
	PACKET_INFO PacketInfo,
	MAC_TABLE_ENTRY *pEntry)
{
	QUEUE_HEADER *pAC_Queue1, *pAC_Queue2;
	INT32 *pAC_ID1, *pAC_ID2, *pAC_ID1_FragSize, *pAC_ID2_FragSize;
	ULONG *pAC_Jiffies1, *pAC_Jiffies2;
	IP_V4_HDR *pIpv4Hdr, Ipv4Hdr;
	IP_FLAGS_FRAG_OFFSET *pFlags_frag_offset, Flags_frag_offset;
	ULONG Now;
	UINT32 i;
	QUEUE_HEADER *pTempqueue;
	PQUEUE_ENTRY pBackupPktEntry;
	PNDIS_PACKET pBackupPkt;


	pAC_Queue1 = &pEntry->ip_queue1[QueIdx];
	pAC_Queue2 = &pEntry->ip_queue2[QueIdx];
	pAC_ID1 = &pEntry->ip_id1[QueIdx];
	pAC_ID2 = &pEntry->ip_id2[QueIdx];
	pAC_ID1_FragSize = &pEntry->ip_id1_FragSize[QueIdx];
	pAC_ID2_FragSize = &pEntry->ip_id2_FragSize[QueIdx];
	pAC_Jiffies1 = &pEntry->ip_pkt1_jiffies[QueIdx];
	pAC_Jiffies2 = &pEntry->ip_pkt2_jiffies[QueIdx];

	NdisGetSystemUpTime(&Now);
	ip_assembly_timeout(pAd, pEntry, Now, QueIdx);

	if (RTMP_GET_PACKET_IPV4(pPacket) &&
	    ((pAd->TxSwQueue[QueIdx].Number >= (pAd->TxSwQMaxLen >> 1)) ||
	    	((pAC_Queue1->Number) || (pAC_Queue2->Number)))
	)
	{
		pIpv4Hdr = (IP_V4_HDR *)(PacketInfo.pFirstBuffer + LENGTH_802_3);
		pFlags_frag_offset = (IP_FLAGS_FRAG_OFFSET *) (PacketInfo.pFirstBuffer + LENGTH_802_3 + 6);
		Flags_frag_offset.word = ntohs(pFlags_frag_offset->word);
		Ipv4Hdr.identifier = ntohs(pIpv4Hdr->identifier);
		Ipv4Hdr.tot_len = ntohs(pIpv4Hdr->tot_len);
		Ipv4Hdr.ihl = pIpv4Hdr->ihl;

		/* check if 1st fragment */
		if ((Flags_frag_offset.field.flags_more_frag == 1) &&
		    (Flags_frag_offset.field.frag_offset == 0))
		{
			NdisGetSystemUpTime(&Now);
			if ((*pAC_ID1) == -1) {
				*pAC_ID1 = Ipv4Hdr.identifier;
				*pAC_Jiffies1 = Now;
				*pAC_ID1_FragSize = Ipv4Hdr.tot_len - (Ipv4Hdr.ihl * 4);
				InsertTailQueue(pAC_Queue1, PACKET_TO_QUEUE_ENTRY(pPacket));
				return 0;
			} else if ((*pAC_ID2) == -1) {
				*pAC_ID2 = Ipv4Hdr.identifier;
				*pAC_Jiffies2 = Now;
				*pAC_ID2_FragSize = Ipv4Hdr.tot_len - (Ipv4Hdr.ihl * 4);
				InsertTailQueue(pAC_Queue2, PACKET_TO_QUEUE_ENTRY(pPacket));
				return 0;
			}

			/* free backup fragments */
			if ((*pAC_Jiffies1) > (*pAC_Jiffies2)) {
				pTempqueue = pAC_Queue2;
				*pAC_ID2 = Ipv4Hdr.identifier;
			} else {
				pTempqueue = pAC_Queue1;
				*pAC_ID1 = Ipv4Hdr.identifier;
			}

			if (pTempqueue->Number != 0) {
				while (1) {
					pBackupPktEntry = RemoveHeadQueue(pTempqueue);
					if (pBackupPktEntry == NULL)
						break;
					pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
					RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
				}
			}
			goto go_original_path;
		}
		else
		{
			/* check if last fragment */
			if ((Ipv4Hdr.identifier == (*pAC_ID1)) || (Ipv4Hdr.identifier == (*pAC_ID2)))
			{
				if (Ipv4Hdr.identifier == (*pAC_ID1)) {
					InsertTailQueue(pAC_Queue1, PACKET_TO_QUEUE_ENTRY(pPacket));
					pTempqueue = pAC_Queue1;
				} else {
					InsertTailQueue(pAC_Queue2, PACKET_TO_QUEUE_ENTRY(pPacket));
					pTempqueue = pAC_Queue2;
				}

				/* the last fragment */
				if ((Flags_frag_offset.field.flags_more_frag == 0)
				    && (Flags_frag_offset.field.frag_offset != 0))
				{
					UINT32 fragment_count = 0;
					BOOLEAN bDrop = FALSE;
					if (Ipv4Hdr.identifier == (*pAC_ID1)) {
						fragment_count = ((Flags_frag_offset.field.frag_offset * 8) / (*pAC_ID1_FragSize)) + 1;
						if (pTempqueue->Number != fragment_count)
							bDrop = TRUE;
						*pAC_ID1 = -1;
					}

					if (Ipv4Hdr.identifier == (*pAC_ID2)) {
						fragment_count = ((Flags_frag_offset.field.frag_offset * 8) / (*pAC_ID2_FragSize)) + 1;
						if (pTempqueue->Number != fragment_count)
							bDrop = TRUE;
						*pAC_ID2 = -1;
					}

					/*
						if number does not equal coreect fragment count or no enough space,
						free backup fragments to SwTxQueue[]
					*/
					if ((bDrop == TRUE) ||
					    (pAd->TxSwQueue[QueIdx].Number > (pAd->TxSwQMaxLen - pTempqueue->Number)))
					{
						while (1) {
							pBackupPktEntry = RemoveHeadQueue(pTempqueue);
							if (pBackupPktEntry == NULL)
								break;
							pBackupPkt = QUEUE_ENTRY_TO_PACKET(pBackupPktEntry);
							RELEASE_NDIS_PACKET(pAd, pBackupPkt, NDIS_STATUS_FAILURE);
						}
						return 0;
					}

					/* move backup fragments to SwTxQueue[] */
					NdisAcquireSpinLock(&pAd->TxSwQueueLock[QueIdx]);
					while (1) {
						pBackupPktEntry = RemoveHeadQueue(pTempqueue);
						if (pBackupPktEntry == NULL)
							break;
						InsertTailQueueAc(pAd, pEntry, &pAd->TxSwQueue[QueIdx], pBackupPktEntry);
					}
					NdisReleaseSpinLock(&pAd->TxSwQueueLock[QueIdx]);

					return 0;
				}
			}
			else
			{
				/*
				   bypass none-fist fragmented packets (we should not drop this packet) when 
				   1. (pAd->TxSwQueue[QueIdx].Number >= (pAd->TxSwQMaxLen[QueIdx] >> 1) and 
				   2. two fragmented buffer are empty 
				 */
				if (((*pAC_ID1) == -1) && ((*pAC_ID2) == -1))
					goto go_original_path;

				if ((Flags_frag_offset.field.flags_more_frag != 0)
				    && (Flags_frag_offset.field.frag_offset != 0))
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					return 0;
				}
				goto go_original_path;
			}
		}
	}

go_original_path:
	return -1;
}

#endif /* IP_ASSEMBLY */
#endif /* WFA_VHT_PF */

VOID StopDmaRx(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Level)
{
	PNDIS_PACKET	pRxPacket;
	RX_BLK			RxBlk, *pRxBlk;
	UINT32 RxPending = 0, MacReg = 0, MTxCycle = 0;
	BOOLEAN bReschedule = FALSE;
	BOOLEAN bCmdRspPacket = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));

	/*
		process whole rx ring
	*/
	while (1)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return;
		pRxBlk = &RxBlk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, &bCmdRspPacket);
		if ((RxPending == 0) && (bReschedule == FALSE))
			break;
		else
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
	}

	/*
		Check DMA Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI

		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		if (MacReg & 0x8)
		{
			RTMPusecDelay(50);
		}
		else
			break;
		
#endif

		
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:RX DMA busy!! DMA_CFG = 0x%08x\n", __FUNCTION__, MacReg));
	}

	if (Level == RTMP_HALT)
	{
		/*
			Disable DMA RX
		*/

#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x4);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, MacReg);
#endif
	
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==== %s\n", __FUNCTION__));
}

VOID StopDmaTx(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Level)
{
	UINT32 MacReg = 0, MTxCycle = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __FUNCTION__));


	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		if ((MacReg & 0x2) == 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("==>  DMA Tx Idle, MacReg=0x%x\n", MacReg));
			break;
		}
#endif

		
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}
	
	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("TX DMA busy!! DMA_CFG(%x)\n", MacReg));
	}

	if (Level == RTMP_HALT)
	{
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x00000001);
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, MacReg);
#endif

	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<==== %s\n", __FUNCTION__));
}

#ifdef DROP_MASK_SUPPORT
VOID drop_mask_init_per_client(
	PRTMP_ADAPTER	ad,
	PMAC_TABLE_ENTRY entry)
{
	BOOLEAN cancelled = FALSE;

	if (entry->dropmask_timer.Valid)
		RTMPCancelTimer(&entry->dropmask_timer, &cancelled);

	RTMPInitTimer(ad, &entry->dropmask_timer, GET_TIMER_FUNCTION(drop_mask_timer_action), entry, FALSE);

	asic_set_drop_mask(ad, entry->Aid, FALSE);

#ifdef NOISE_TEST_ADJUST
	if ((ad->ApCfg.EntryClientCount >= 3) && IS_ENTRY_CLIENT(entry))
		entry->pMbss->WPAREKEY.ReKeyMethod |= MAX_REKEY;
#endif /* NOISE_TEST_ADJUST */
}

VOID drop_mask_release_per_client(
	PRTMP_ADAPTER	ad,
	PMAC_TABLE_ENTRY entry)
{
	BOOLEAN cancelled = FALSE;

	RTMPCancelTimer(&entry->dropmask_timer, &cancelled);
	RTMPReleaseTimer(&entry->dropmask_timer, &cancelled);
}

VOID drop_mask_set_per_client(
	PRTMP_ADAPTER		ad,
	PMAC_TABLE_ENTRY 	entry,
	BOOLEAN				enable)
{
	BOOLEAN cancelled = FALSE;

#ifdef NOISE_TEST_ADJUST
	if (ad->ApCfg.EntryClientCount < 3)
		return;
#endif /* NOISE_TEST_ADJUST */

	RTMPCancelTimer(&entry->dropmask_timer, &cancelled);

	asic_set_drop_mask(ad, entry->Aid, enable);

	if (enable)
		RTMPSetTimer(&entry->dropmask_timer, 1000);
}

VOID drop_mask_timer_action(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PMAC_TABLE_ENTRY entry = (MAC_TABLE_ENTRY *)FunctionContext;
	PRTMP_ADAPTER ad = (PRTMP_ADAPTER)entry->pAd;

	/* Disable drop mask */
	asic_set_drop_mask(ad, entry->Aid, FALSE);
}

#endif /* DROP_MASK_SUPPORT */

