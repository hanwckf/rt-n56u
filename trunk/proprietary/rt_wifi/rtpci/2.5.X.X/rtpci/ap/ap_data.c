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
	Paul Lin	08-01-2002		created
	Paul Lin	07-01-2003		add encryption/decryption data flow
	John Chang	08-05-2003		modify 802.11 header for AP purpose
	John Chang	12-20-2004		modify for 2561/2661. merge into STA driver
	Jan Lee	1-20-2006		    modify for 2860.  
*/
#include "rt_config.h"

#ifdef LINUX
/* use native linux mcast/bcast adress checks. */
#include <linux/etherdevice.h>

#define IS_MULTICAST_MAC_ADDR(Addr)			(is_multicast_ether_addr(Addr) && ((Addr[0]) != 0xff))
#define IS_IPV6_MULTICAST_MAC_ADDR(Addr)		(is_multicast_ether_addr(Addr) && ((Addr[0]) == 0x33))
#define IS_BROADCAST_MAC_ADDR(Addr)			(is_broadcast_ether_addr(Addr))
#else
#define IS_MULTICAST_MAC_ADDR(Addr)			((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) != 0xff))
#define IS_IPV6_MULTICAST_MAC_ADDR(Addr)		((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) == 0x33))
#define IS_BROADCAST_MAC_ADDR(Addr)			((((Addr[0]) & 0xff) == 0xff))
#endif

static VOID APFindCipherAlgorithm(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk);

#ifdef DOT11_N_SUPPORT
VOID RTMP_BASetup(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN UINT8 UserPriority)
{
	if (pMacEntry && (pMacEntry->NoBADataCountDown == 0) && IS_HT_STA(pMacEntry))
	{
		// Don't care the status of the portSecured status.
		// 
		if (((pMacEntry->TXBAbitmap & (1<<UserPriority)) == 0) /*&& (pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED)*/
			 // For IOT compatibility, if  
			 // 1. It is Ralink chip or 			  
			 // 2. It is OPEN or AES mode, 
			 // then BA session can be bulit.			 
			 && ((IS_ENTRY_CLIENT(pMacEntry) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) || 
				 IS_ENTRY_MESH(pMacEntry) || IS_ENTRY_WDS(pMacEntry) ||
			 	 (IS_ENTRY_APCLI(pMacEntry) && (pAd->MlmeAux.APRalinkIe != 0x0) && (pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED)) || 
			 	 (pMacEntry->WepStatus == Ndis802_11WEPDisabled || pMacEntry->WepStatus == Ndis802_11Encryption3Enabled
#ifdef WAPI_SUPPORT
					|| pMacEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled
#endif // WAPI_SUPPORT //
				))
			)
		{
			BAOriSessionSetUp(pAd, pMacEntry, UserPriority, 0, 10, FALSE);
		}
	}
}
#endif // DOT11_N_SUPPORT //

static inline BOOLEAN ApAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR		*pWcid)
{
	PACKET_INFO 	PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;
	PMAC_TABLE_ENTRY pEntry = NULL;
	SST 			Sst;
	USHORT			Aid;
	UCHAR			PsMode, Rate;
	BOOLEAN			allowed;
	
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
#ifdef CLIENT_WDS
	{
		PUCHAR pEntryAddr;
		pEntry = APSsPsInquiry(pAd, pSrcBufVA, &Sst, &Aid, &PsMode, &Rate);
		if ((pEntry == NULL)
			&& (pEntryAddr = CliWds_ProxyLookup(pAd, pSrcBufVA)) != NULL)
		{
			pEntry = APSsPsInquiry(pAd, pEntryAddr, &Sst, &Aid, &PsMode, &Rate);
		}
	}
#else
	pEntry = APSsPsInquiry(pAd, pSrcBufVA, &Sst, &Aid, &PsMode, &Rate);
#endif // CLIENT_WDS //


	if ((pEntry && (pEntry->apidx == RTMP_GET_PACKET_IF(pPacket)) && (Sst == SST_ASSOC))
		|| (*pSrcBufVA & 0x01))
	{
		// Record that orignal packet source is from NDIS layer,so that 
		// later on driver knows how to release this NDIS PACKET
		*pWcid = (UCHAR)Aid; //RTMP_SET_PACKET_WCID(pPacket, (UCHAR)Aid);
		allowed = TRUE;
	}
	else
	{
		allowed = FALSE;
	}

	return allowed;
	
}



/*
========================================================================
Routine Description:
    Early checking and OS-depened parsing for Tx packet to AP device.

Arguments:
    NDIS_HANDLE 	MiniportAdapterContext	Pointer refer to the device handle, i.e., the pAd.
	PPNDIS_PACKET	ppPacketArray			The packet array need to do transmission.
	UINT			NumberOfPackets			Number of packet in packet array.
	
Return Value:
	NONE					

Note:
	This function do early checking and classification for send-out packet.
	You only can put OS-depened & AP related code in here.
========================================================================
*/
VOID	APSendPackets(
	IN	NDIS_HANDLE		MiniportAdapterContext,
	IN	PPNDIS_PACKET	ppPacketArray,
	IN	UINT			NumberOfPackets)
{
	UINT			Index;
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER) MiniportAdapterContext;
	PNDIS_PACKET	pPacket;
	BOOLEAN			allowToSend;
	UCHAR			wcid = MCAST_WCID;
	
	
	for (Index = 0; Index < NumberOfPackets; Index++)
	{
		pPacket = ppPacketArray[Index];

   		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		{
			// Drop send request since hardware is in reset state
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		/* The following code do comparison must base on the following sequence: 
				MIN_NET_DEVICE_FOR_APCLI> MIN_NET_DEVICE_FOR_WDS > Normal
		*/
#ifdef APCLI_SUPPORT
		if (RTMP_GET_PACKET_NET_DEVICE(pPacket) >= MIN_NET_DEVICE_FOR_APCLI)
			allowToSend = ApCliAllowToSendPacket(pAd, pPacket, &wcid);
		else
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
		if (RTMP_GET_PACKET_NET_DEVICE(pPacket) >= MIN_NET_DEVICE_FOR_WDS)
			allowToSend = ApWdsAllowToSendPacket(pAd, pPacket, &wcid);
		else
#endif // WDS_SUPPORT //
			allowToSend = ApAllowToSendPacket(pAd, pPacket, &wcid);

		if (allowToSend)
		{
			// For packet send from OS, we need to set the wcid here, it will used directly in APSendPacket.
			RTMP_SET_PACKET_WCID(pPacket, wcid);
			NDIS_SET_PACKET_STATUS(pPacket, NDIS_STATUS_PENDING);
			pAd->RalinkCounters.PendingNdisPacketCount++;
			
			APSendPacket(pAd, pPacket);
		}
		else
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
	}

	// Dequeue outgoing frames from TxSwQueue0..3 queue and process it
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);

}


/*
	========================================================================
	Routine Description:
		This routine is used to do packet parsing and classification for Tx packet 
		to AP device, and it will en-queue packets to our TxSwQueue depends on AC 
		class.
	
	Arguments:
		pAd    Pointer to our adapter
		pPacket 	Pointer to send packet

	Return Value:
		NDIS_STATUS_SUCCESS			If succes to queue the packet into TxSwQueue.
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
NDIS_STATUS APSendPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{
	PACKET_INFO 	PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;
	UINT			AllowFragSize;
	UCHAR			NumberOfFrag;
	UCHAR			RTSRequired;
	UCHAR			QueIdx, UserPriority, apidx = MAIN_MBSSID;
	SST 			Sst = SST_ASSOC;
	UCHAR			PsMode = PWR_ACTIVE, Rate;
	USHORT			Wcid;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MULTISSID_STRUCT *pMbss = NULL;
	unsigned long	IrqFlags;
#ifdef IGMP_SNOOP_SUPPORT
	INT			InIgmpGroup = IGMP_NONE;
	PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry = NULL;
#endif // IGMP_SNOOP_SUPPORT //
	BOOLEAN is_mcast = FALSE;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL)
	{
		// Resourece is low, system did not allocate virtual address
		// return NDIS_STATUS_FAILURE directly to upper layer
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (SrcBufLen <= 14)
	{
		DBGPRINT(RT_DEBUG_ERROR,("APSendPacket --> Ndis Packet buffer error !!!\n"));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return (NDIS_STATUS_FAILURE);
	}

	Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pMacEntry = &pAd->MacTab.Content[Wcid];

	//
	// Check the Ethernet Frame type of this packet, and set the RTMP_SET_PACKET_SPECIFIC flags.
	//		Here we set the PACKET_SPECIFIC flags(LLC, VLAN, DHCP/ARP, EAPOL).
	UserPriority = 0;
	QueIdx		 = QID_AC_BE;
	if (RTMPCheckEtherType(pAd, pPacket, pMacEntry, &UserPriority, &QueIdx) == FALSE)
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

#ifdef APCLI_SUPPORT
	if(IS_ENTRY_APCLI(pMacEntry))
	{
		Rate = pMacEntry->CurrTxRate;
	    if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
			 && (pMacEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
        	 && (RTMP_GET_PACKET_EAPOL(pPacket)== FALSE))
		{
            RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE); 
            return (NDIS_STATUS_FAILURE);
        }
	}
	else
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
	if (IS_ENTRY_WDS(pMacEntry))
	{
		//b7 as WDS bit, b0-6 as WDS index when b7==1
		Rate = pMacEntry->CurrTxRate;
	}
	else
#endif // WDS_SUPPORT //
	if (IS_ENTRY_CLIENT(pMacEntry) || (Wcid == MCAST_WCID))
	{
		//USHORT Aid;
		PsMode = pMacEntry->PsMode;
		Rate = pMacEntry->CurrTxRate;
		Sst = pMacEntry->Sst;

		if (Wcid == MCAST_WCID)
		{
			if (pAd->ApCfg.EntryClientCount == 0)
			{
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}
			
			apidx = RTMP_GET_PACKET_NET_DEVICE_MBSSID(pPacket);
			MBSS_MR_APIDX_SANITY_CHECK(apidx, pAd);
			pMbss = &pAd->ApCfg.MBSSID[apidx];
		}
		else
		{
			apidx = pMacEntry->apidx;
			pMbss = pMacEntry->pMbss;
		}

		// AP does not send packets before port secured.
		if (pMbss != NULL)
		{
			if (((pMbss->AuthMode >= Ndis802_11AuthModeWPA)
#ifdef DOT1X_SUPPORT
				 || (pMbss->IEEE8021X == TRUE)
#endif // DOT1X_SUPPORT //
				) && 
				(RTMP_GET_PACKET_EAPOL(pPacket) == FALSE)
#ifdef WAPI_SUPPORT
				 && (RTMP_GET_PACKET_WAI(pPacket) == FALSE)
#endif // WAPI_SUPPORT //
				)
			{
				// Process for multicast or broadcast frame 
				if ((Wcid == MCAST_WCID) && (pMbss->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					return NDIS_STATUS_FAILURE;			
				}

				// Process for unicast frame 
				if ((Wcid != MCAST_WCID) && pMacEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					return NDIS_STATUS_FAILURE;	
				}
			}
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("I/F(ra%d) APSendPacket --> Drop unknow packet !!!\n", apidx));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (pMbss == NULL)
		pMbss = &pAd->ApCfg.MBSSID[apidx];

	if (*pSrcBufVA & 0x01)
		is_mcast = TRUE;

#ifdef IGMP_SNOOP_SUPPORT
	if (is_mcast && pAd->ApCfg.IgmpSnoopEnable)
	{
		UCHAR FromWhichBSSID, checkIgmpPkt = TRUE;

		if (IS_ENTRY_WDS(pMacEntry))		
			FromWhichBSSID = pMacEntry->MatchWDSTabIdx + MIN_NET_DEVICE_FOR_WDS;		
		else if ((Wcid == MCAST_WCID) || IS_ENTRY_CLIENT(pMacEntry))		
			FromWhichBSSID = apidx;		
		else		
			checkIgmpPkt = FALSE;		
		  
		if (checkIgmpPkt)
		{
			if (IgmpPktInfoQuery(pAd, pSrcBufVA, pPacket, FromWhichBSSID,
						&InIgmpGroup, &pGroupEntry) != NDIS_STATUS_SUCCESS)
				return NDIS_STATUS_FAILURE;
		} 
	}
#endif  // IGMP_SNOOP_SUPPORT //

	// STEP 1. Decide number of fragments required to deliver this MSDU. 
	//	   The estimation here is not very accurate because difficult to 
	//	   take encryption overhead into consideration here. The result 
	//	   "NumberOfFrag" is then just used to pre-check if enough free 
	//	   TXD are available to hold this MSDU.
	if (is_mcast	// fragmentation not allowed on multicast & broadcast
#ifdef IGMP_SNOOP_SUPPORT
		// multicast packets in IgmpSn table should never send to Power-Saving queue.
		&& (InIgmpGroup == IGMP_NONE)
#endif // IGMP_SNOOP_SUPPORT //
		)
		NumberOfFrag = 1;
	else if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)
			&& CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE))
	{
		NumberOfFrag = 1;	// Aggregation overwhelms fragmentation
	}
	else
	{
		// The calculated "NumberOfFrag" is a rough estimation because of various 
		// encryption/encapsulation overhead not taken into consideration. This number is just
		// used to make sure enough free TXD are available before fragmentation takes place.
		// In case the actual required number of fragments of an NDIS packet 
		// excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
		// last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of 
		// resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should 
		// rarely happen and the penalty is just like a TX RETRY fail. Affordable.
		UINT32 Size;

		AllowFragSize = (pAd->CommonCfg.FragmentThreshold) - LENGTH_802_11 - LENGTH_CRC;
		Size = PacketInfo.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H;
		if (Size >= AllowFragSize)
			NumberOfFrag = (Size / AllowFragSize) + 1;
		else
			NumberOfFrag = 1;
	}

	// Save fragment number to Ndis packet reserved field
	RTMP_SET_PACKET_FRAGMENTS(pPacket, NumberOfFrag);  

	// STEP 2. Check the requirement of RTS; decide packet TX rate
	//	   If multiple fragment required, RTS is required only for the first fragment
	//	   if the fragment size large than RTS threshold

	if (NumberOfFrag > 1)
		RTSRequired = (pAd->CommonCfg.FragmentThreshold > pAd->CommonCfg.RtsThreshold) ? 1 : 0;
	else
		RTSRequired = (PacketInfo.TotalPacketLength > pAd->CommonCfg.RtsThreshold) ? 1 : 0;

	// RTS/CTS may also be required in order to protect OFDM frame
	if ((Rate >= RATE_FIRST_OFDM_RATE) && 
		(Rate <= RATE_LAST_OFDM_RATE) && 
		OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
		RTSRequired = 1;

	// Save RTS requirement to Ndis packet reserved field
	RTMP_SET_PACKET_RTS(pPacket, RTSRequired);
	RTMP_SET_PACKET_TXRATE(pPacket, Rate);
	
	//
	// detect AC Category of trasmitting packets
	// to tune AC0(BE) TX_OP (MAC reg 0x1300)
	// 
	detect_wmm_traffic(pAd, UserPriority, 1);

	RTMP_SET_PACKET_UP(pPacket, UserPriority);

	RTMP_SET_PACKET_MGMT_PKT(pPacket, 0x00); /* mark as non-management frame */

#ifdef INF_AMAZON_SE
	pAd->BulkOutDataSizeCount[QueIdx]+=SrcBufLen;
#endif // INF_AMAZON_SE //
	
	//
	// 4. put to corrsponding TxSwQueue or Power-saving queue
	//

	// WDS and ApClient link should never go into power-save mode; just send out the frame
	if (pMacEntry && (IS_ENTRY_WDS(pMacEntry) || IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_MESH(pMacEntry)))
	{
#ifdef WDS_SUPPORT
		ULONG Now32;
		NdisGetSystemUpTime(&Now32);
#endif // WDS_SUPPORT //

		if (pAd->TxSwQueue[QueIdx].Number >= MAX_PACKETS_IN_QUEUE)
        {
#ifdef BLOCK_NET_IF
			StopNetIfQueue(pAd, QueIdx, pPacket);
#endif // BLOCK_NET_IF //
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
#ifdef WDS_SUPPORT
		else if(IS_ENTRY_WDS(pMacEntry)  	// when WDS Jam happen, drop following 1min to SWQueue Pkts
			&& (pMacEntry->LockEntryTx == TRUE) 
			&& RTMP_TIME_BEFORE(Now32, pMacEntry->TimeStamp_toTxRing + WDS_ENTRY_RETRY_INTERVAL)) 
		{
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
		}
#endif // WDS_SUPPORT //
		else
		{
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QueIdx], PACKET_TO_QUEUE_ENTRY(pPacket));
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
		}
	}
	// M/BCAST frames are put to PSQ as long as there's any associated STA in power-save mode
	else if (is_mcast && pAd->MacTab.fAnyStationInPsm
#ifdef IGMP_SNOOP_SUPPORT
		// multicast packets in IgmpSn table should never send to Power-Saving queue.
		&& (InIgmpGroup == IGMP_NONE)
#endif // IGMP_SNOOP_SUPPORT //
		)
	{
		// we don't want too many MCAST/BCAST backlog frames to eat up all buffers. So in case number of backlog 
		// MCAST/BCAST frames exceeds a pre-defined watermark within a DTIM period, simply drop coming new 
		// MCAST/BCAST frames. This design is similiar to "BROADCAST throttling in most manageable Ethernet Switch chip.
		if (pAd->MacTab.McastPsQueue.Number >= MAX_PACKETS_IN_MCAST_PS_QUEUE)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			DBGPRINT(RT_DEBUG_TRACE, ("M/BCAST PSQ(=%ld) full, drop it!\n", pAd->MacTab.McastPsQueue.Number));
			return NDIS_STATUS_FAILURE;
		}
		else
		{
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			InsertHeadQueue(&pAd->MacTab.McastPsQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

			WLAN_MR_TIM_BCMC_SET(apidx); // mark MCAST/BCAST TIM bit
		}
	}
	// else if the associted STA in power-save mode, frame also goes to PSQ
	else if ((PsMode == PWR_SAVE) && pMacEntry &&
				IS_ENTRY_CLIENT(pMacEntry) && (Sst == SST_ASSOC))
	{
		if (APInsertPsQueue(pAd, pPacket, pMacEntry, QueIdx)
				!= NDIS_STATUS_SUCCESS)
				return NDIS_STATUS_FAILURE;			
			}
	// 3. otherwise, transmit the frame
	else // (PsMode == PWR_ACTIVE) || (PsMode == PWR_UNKNOWN)
	{
#ifdef IGMP_SNOOP_SUPPORT
		// if it's a mcast packet in igmp gourp.
		// ucast clone it for all members in the gourp.
		if (((InIgmpGroup == IGMP_IN_GROUP) && (pGroupEntry) && (IgmpMemberCnt(&pGroupEntry->MemberList) > 0)) ||
		     (InIgmpGroup == IGMP_PKT))
		{
			NDIS_STATUS PktCloneResult = IgmpPktClone(pAd, pPacket, InIgmpGroup, pGroupEntry, QueIdx, UserPriority);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			return PktCloneResult;
		}
		else
#endif // IGMP_SNOOP_SUPPORT //
		{
			if (pAd->TxSwQueue[QueIdx].Number >= MAX_PACKETS_IN_QUEUE)
			{
#ifdef WMM_ACM_SUPPORT
				/* only for BE traffic & ACM of BE is set */
				if (ACMP_BE_IsReallyToReleaseWhenQueFull(
							pAd,
							pMacEntry,
							QueIdx,
							&pAd->TxSwQueue[QueIdx],
							pPacket) != ACM_RTN_OK)
				{
					/* re-insert to the BE queue */
					RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
					InsertTailQueue(&pAd->TxSwQueue[QueIdx], PACKET_TO_QUEUE_ENTRY(pPacket));
					RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
				}
				else
				{
#endif // WMM_ACM_SUPPORT //

#ifdef BLOCK_NET_IF
				StopNetIfQueue(pAd, QueIdx, pPacket);
#endif // BLOCK_NET_IF //
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;			

#ifdef WMM_ACM_SUPPORT
				}
#endif // WMM_ACM_SUPPORT //
			}
			else
			{			
				RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
				InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QueIdx], PACKET_TO_QUEUE_ENTRY(pPacket));
				RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			}
		}
	}

#ifdef DOT11_N_SUPPORT
	RTMP_BASetup(pAd, pMacEntry, UserPriority);
#endif // DOT11_N_SUPPORT //
//	pAd->RalinkCounters.OneSecOsTxCount[QueIdx]++; // TODO: for debug only. to be removed
	return NDIS_STATUS_SUCCESS;
}


// --------------------------------------------------------
//  FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
//		Find the WPA key, either Group or Pairwise Key
//		LEAP + TKIP also use WPA key.
// --------------------------------------------------------
// Decide WEP bit and cipher suite to be used. Same cipher suite should be used for whole fragment burst
// In Cisco CCX 2.0 Leap Authentication
//		   WepStatus is Ndis802_11Encryption1Enabled but the key will use PairwiseKey
//		   Instead of the SharedKey, SharedKey Length may be Zero.
static inline VOID APFindCipherAlgorithm(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{

	PCIPHER_KEY			pKey = NULL;
	UCHAR				CipherAlg = CIPHER_NONE;		// cipher alogrithm
	UCHAR				apidx;
	UCHAR				RAWcid;
	PMAC_TABLE_ENTRY	pMacEntry;
	UCHAR				KeyIdx = 0;
	MULTISSID_STRUCT	*pMbss;

	apidx = pTxBlk->apidx;
	RAWcid = pTxBlk->Wcid;
	pMacEntry = pTxBlk->pMacEntry;
	pMbss = &pAd->ApCfg.MBSSID[apidx];

#ifdef APCLI_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
	{	
		PAPCLI_STRUCT	pApCliEntry;

		pApCliEntry = pTxBlk->pApCliEntry;

		if (RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket)) 
		{			
			// These EAPoL frames must be clear before 4-way handshaking is completed.
			if ((!(TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame))) && 
				(pMacEntry->PairwiseKey.CipherAlg) &&
				(pMacEntry->PairwiseKey.KeyLen))
			{
				CipherAlg  = pMacEntry->PairwiseKey.CipherAlg;
				if (CipherAlg)
					pKey = &pMacEntry->PairwiseKey;
			}
			else
			{
				CipherAlg = CIPHER_NONE;
				pKey	  = NULL;
			}
		}
		else if (pMacEntry->WepStatus == Ndis802_11Encryption1Enabled)
		{
			CipherAlg  = pApCliEntry->SharedKey[pApCliEntry->DefaultKeyId].CipherAlg;
			if (CipherAlg)
				pKey = &pApCliEntry->SharedKey[pApCliEntry->DefaultKeyId];
		}		
		else if (pMacEntry->WepStatus == Ndis802_11Encryption2Enabled ||
	 			 pMacEntry->WepStatus == Ndis802_11Encryption3Enabled)
		{
			CipherAlg  = pMacEntry->PairwiseKey.CipherAlg;
			if (CipherAlg)
				pKey = &pMacEntry->PairwiseKey;
		}
		else
		{
			CipherAlg = CIPHER_NONE;
			pKey	  = NULL;
		}			
	}
	else
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk,fTX_bWDSEntry))
	{
		if (pAd->WdsTab.WdsEntry[pMacEntry->MatchWDSTabIdx].WepStatus == Ndis802_11Encryption1Enabled ||
			pAd->WdsTab.WdsEntry[pMacEntry->MatchWDSTabIdx].WepStatus == Ndis802_11Encryption2Enabled ||
			pAd->WdsTab.WdsEntry[pMacEntry->MatchWDSTabIdx].WepStatus == Ndis802_11Encryption3Enabled)		
		{
			CipherAlg  = pAd->WdsTab.WdsEntry[pMacEntry->MatchWDSTabIdx].WdsKey.CipherAlg;
			if (CipherAlg)
				pKey = &pAd->WdsTab.WdsEntry[pMacEntry->MatchWDSTabIdx].WdsKey;
		}
		else
		{
			CipherAlg = CIPHER_NONE;
			pKey = NULL;
		}
	}
	else
#endif // WDS_SUPPORT //
#ifdef WAPI_SUPPORT
	if (pMbss->WepStatus == Ndis802_11EncryptionSMS4Enabled)
	{
		if (RTMP_GET_PACKET_WAI(pTxBlk->pPacket))
		{
			// WAI negotiation packet is always clear.
			CipherAlg = CIPHER_NONE;					
			pKey = NULL;
		}	
		else if (!pMacEntry)
		{
			KeyIdx = pMbss->DefaultKeyId; // MSK ID
			CipherAlg  = pAd->SharedKey[apidx][KeyIdx].CipherAlg;			
			if (CipherAlg == CIPHER_SMS4)
			{
				pKey = &pAd->SharedKey[apidx][KeyIdx];	
#ifdef SOFT_ENCRYPT
				if (pMbss->sw_wpi_encrypt)
				{
					TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
					/* TSC increment pre encryption transmittion */				
					inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 1);								
				}
#endif // SOFT_ENCRYPT //
			}
		}	
		else		
		{			
			KeyIdx = pTxBlk->pMacEntry->usk_id; // USK ID
			CipherAlg  = pAd->MacTab.Content[RAWcid].PairwiseKey.CipherAlg;
			if (CipherAlg == CIPHER_SMS4)
			{
				pKey = &pAd->MacTab.Content[RAWcid].PairwiseKey;
#ifdef SOFT_ENCRYPT
				if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))	
				{
					TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
					/* TSC increment pre encryption transmittion */				
					inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 2);				
				}
#endif // SOFT_ENCRYPT //
			}
		}
	}
	else
#endif // WAPI_SUPPORT //
	if ((RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))			||
#ifdef DOT1X_SUPPORT		
		((pMbss->WepStatus == Ndis802_11Encryption1Enabled) && (pMbss->IEEE8021X == TRUE)) || 
#endif // DOT1X_SUPPORT //		
		(pMbss->WepStatus == Ndis802_11Encryption2Enabled)	||
		(pMbss->WepStatus == Ndis802_11Encryption3Enabled)	||
		(pMbss->WepStatus == Ndis802_11Encryption4Enabled))
	{
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame))
		{
			DBGPRINT(RT_DEBUG_TRACE,("APHardTransmit --> clear eap frame !!!\n"));          
			CipherAlg = CIPHER_NONE;
			pKey = NULL;
		}
		else if (!pMacEntry) 	   // M/BCAST to local BSS, use default key in shared key table
		{
			KeyIdx = pMbss->DefaultKeyId;
			CipherAlg  = pAd->SharedKey[apidx][KeyIdx].CipherAlg;			
			if (CipherAlg)
				pKey = &pAd->SharedKey[apidx][KeyIdx];			
		}
		else						// unicast to local BSS
		{
			CipherAlg  = pAd->MacTab.Content[RAWcid].PairwiseKey.CipherAlg;
			if (CipherAlg)
			{
				pKey = &pAd->MacTab.Content[RAWcid].PairwiseKey;
			}

#ifdef SOFT_ENCRYPT
			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))	
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);

				/* TSC increment pre encryption transmittion */				
				INC_TX_TSC(pKey->TxTsc, LEN_WPA_TSC);				
			}
#endif // SOFT_ENCRYPT //
		}
	}
	else if (pMbss->WepStatus == Ndis802_11Encryption1Enabled) // WEP always uses shared key table
	{
		KeyIdx = pMbss->DefaultKeyId;
		CipherAlg  = pAd->SharedKey[apidx][KeyIdx].CipherAlg;
		if (CipherAlg)
			pKey = &pAd->SharedKey[apidx][KeyIdx];
	}
	else
	{
		CipherAlg = CIPHER_NONE;
		pKey	  = NULL;
	}

	pTxBlk->CipherAlg = CipherAlg;
	pTxBlk->pKey = pKey;
	pTxBlk->KeyIdx = KeyIdx;	
}

#ifdef DOT11_N_SUPPORT
static inline VOID APBuildCache802_11Header(
	IN RTMP_ADAPTER		*pAd,
	IN TX_BLK			*pTxBlk,
	IN UCHAR			*pHeader)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	PHEADER_802_11	pHeader80211;

	pHeader80211 = (PHEADER_802_11)pHeader;
	pMacEntry = pTxBlk->pMacEntry;

	// 
	// Update the cached 802.11 HEADER
	// 
	
	// normal wlan header size : 24 octets
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	
	// More Bit
	pHeader80211->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
	
	// Sequence
	pHeader80211->Sequence = pMacEntry->TxSeq[pTxBlk->UserPriority];
	pMacEntry->TxSeq[pTxBlk->UserPriority] = (pMacEntry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;
	
	// SA 
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	if (FALSE
#ifdef WDS_SUPPORT
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif // WDS_SUPPORT //
#ifdef CLIENT_WDS
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif // CLIENT_WDS //
		)
	{	// The addr3 of WDS packet is Destination Mac address and Addr4 is the Source Mac address.
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
		COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
		pTxBlk->MpduHeaderLen += MAC_ADDR_LEN; 
	}
	else
#endif // WDS_SUPPORT || CLIENT_WDS //
#ifdef APCLI_SUPPORT
	if(IS_ENTRY_APCLI(pMacEntry))
	{	// The addr3 of Ap-client packet is Destination Mac address.
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
	}
	else
#endif // APCLI_SUPPORT //
	{	// The addr3 of normal packet send from DS is Src Mac address.
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
	}


}
#endif // DOT11_N_SUPPORT //

static inline VOID APBuildCommon802_11Header(
	IN  PRTMP_ADAPTER   pAd,
	IN  TX_BLK          *pTxBlk)
{

	HEADER_802_11	*pHeader_802_11;

	// 
	// MAKE A COMMON 802.11 HEADER
	// 

	// normal wlan header size : 24 octets
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);

	pHeader_802_11 = (HEADER_802_11 *) &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];

	NdisZeroMemory(pHeader_802_11, sizeof(HEADER_802_11));

	pHeader_802_11->FC.FrDs = 1;
	pHeader_802_11->FC.Type = BTYPE_DATA;
	pHeader_802_11->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

	if (pTxBlk->pMacEntry)
	{
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bForceNonQoS))
		{			
			pHeader_802_11->Sequence = pTxBlk->pMacEntry->NonQosDataSeq;
			pTxBlk->pMacEntry->NonQosDataSeq = (pTxBlk->pMacEntry->NonQosDataSeq+1) & MAXSEQ;
		}
		else
		{		
    	    pHeader_802_11->Sequence = pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority];
    	    pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority] = (pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;
    	}		
	}
	else
	{
		pHeader_802_11->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; // next sequence  
	}
	
	pHeader_802_11->Frag = 0;

	pHeader_802_11->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

#ifdef APCLI_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
	{
		pHeader_802_11->FC.ToDs = 1;
		pHeader_802_11->FC.FrDs = 0;
		COPY_MAC_ADDR(pHeader_802_11->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid));	// to AP2
		COPY_MAC_ADDR(pHeader_802_11->Addr2, pTxBlk->pApCliEntry->CurrentAddress);		// from AP1
		COPY_MAC_ADDR(pHeader_802_11->Addr3, pTxBlk->pSrcBufHeader);					// DA
	}
	else
#endif // APCLI_SUPPORT //
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	if (FALSE
#ifdef WDS_SUPPORT
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif // WDS_SUPPORT //
#ifdef CLIENT_WDS
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif // CLIENT_WDS //
		)
	{
		pHeader_802_11->FC.ToDs = 1;
		COPY_MAC_ADDR(pHeader_802_11->Addr1, pTxBlk->pMacEntry->Addr);					// to AP2
		COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);						// from AP1
		COPY_MAC_ADDR(pHeader_802_11->Addr3, pTxBlk->pSrcBufHeader);					// DA
		COPY_MAC_ADDR(&pHeader_802_11->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);			// ADDR4 = SA
		pTxBlk->MpduHeaderLen += MAC_ADDR_LEN; 
	}
	else
#endif // WDS_SUPPORT || CLIENT_WDS //
	{
		// TODO: how about "MoreData" bit? AP need to set this bit especially for PS-POLL response
#ifdef IGMP_SNOOP_SUPPORT
		if (pTxBlk->Wcid != MCAST_WCID)
		{
			COPY_MAC_ADDR(pHeader_802_11->Addr1, pTxBlk->pMacEntry->Addr); // DA
		}
		else
#endif // IGMP_SNOOP_SUPPORT //
		{
		   	COPY_MAC_ADDR(pHeader_802_11->Addr1, pTxBlk->pSrcBufHeader);					// DA
		}
		COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->ApCfg.MBSSID[pTxBlk->apidx].Bssid);		// BSSID
		COPY_MAC_ADDR(pHeader_802_11->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);			// SA
	}


	if (pTxBlk->CipherAlg != CIPHER_NONE)
		pHeader_802_11->FC.Wep = 1;
}


static inline PUCHAR AP_Build_ARalink_Frame_Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK		*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;//, pSaveBufPtr;
	HEADER_802_11	*pHeader_802_11;
	PNDIS_PACKET	pNextPacket;
	UINT32			nextBufLen;
	PQUEUE_ENTRY	pQEntry;
		
	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);


	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	// steal "order" bit to mark "aggregation"
	pHeader_802_11->FC.Order = 1;
	
	// skip common header
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
	{
		//
		// build QOS Control bytes
		// 
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_AP_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif // WDS_SUPPORT //
		)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			 if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif // UAPSD_AP_SUPPORT //
	
		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;
	}

	// padding at front of LLC header. LLC header should at 4-bytes aligment.
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

	
	// For RA Aggregation, 
	// put the 2nd MSDU length(extra 2-byte field) after QOS_CONTROL in little endian format
	pQEntry = pTxBlk->TxPacketList.Head;
	pNextPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	nextBufLen = GET_OS_PKT_LEN(pNextPacket);
	if (RTMP_GET_PACKET_VLAN(pNextPacket))
		nextBufLen -= LENGTH_802_1Q;
	
	*pHeaderBufPtr = (UCHAR)nextBufLen & 0xff;
	*(pHeaderBufPtr+1) = (UCHAR)(nextBufLen >> 8);

	pHeaderBufPtr += 2;
	pTxBlk->MpduHeaderLen += 2;
	
	return pHeaderBufPtr;
	
}


#ifdef DOT11_N_SUPPORT
static inline PUCHAR AP_Build_AMSDU_Frame_Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK		*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;//, pSaveBufPtr;
	HEADER_802_11	*pHeader_802_11;

	
	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	// skip common header
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	//
	// build QOS Control bytes
	// 
	*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_AP_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
		&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif // WDS_SUPPORT //
	)
	{
		/* 
		 * we can not use bMoreData bit to get EOSP bit because
		 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
		 */
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
			*pHeaderBufPtr |= (1 << 4);
	}
#endif // UAPSD_AP_SUPPORT //


	//
	// A-MSDU packet
	// 
	*pHeaderBufPtr |= 0x80;

	*(pHeaderBufPtr+1) = 0;
	pHeaderBufPtr +=2;
	pTxBlk->MpduHeaderLen += 2;


	//pSaveBufPtr = pHeaderBufPtr;

	//
	// padding at front of LLC header
	// LLC header should locate at 4-octets aligment
	// 
	// @@@ MpduHeaderLen excluding padding @@@
	// 
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
		
	return pHeaderBufPtr;

}


VOID AP_AMPDU_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	HEADER_802_11	*pHeader_802_11;
	PUCHAR			pHeaderBufPtr;
//	UCHAR			QueIdx = pTxBlk->QueIdx;
	USHORT			FreeNumber;
	MAC_TABLE_ENTRY	*pMacEntry;
	PQUEUE_ENTRY	pQEntry;
	BOOLEAN			bHTCPlus = FALSE;
	
	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
	{
#ifdef STATS_COUNT_SUPPORT
		MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount ++;
#endif // STATS_COUNT_SUPPORT //

		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pMacEntry = pTxBlk->pMacEntry;
	if ((pMacEntry->isCached))
	{
		//NdisZeroMemory((PUCHAR)(&pTxBlk->HeaderBuf[0]), sizeof(pTxBlk->HeaderBuf)); // It should be cleared!!!
#ifndef VENDOR_FEATURE1_SUPPORT
		NdisMoveMemory((PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (PUCHAR)(&pMacEntry->CachedBuf[0]), TXWI_SIZE + sizeof(HEADER_802_11));
#else
		pTxBlk->HeaderBuf = (UCHAR *)(pMacEntry->HeaderBuf);
#endif // VENDOR_FEATURE1_SUPPORT //
		pHeaderBufPtr = (PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE]);
		APBuildCache802_11Header(pAd, pTxBlk, pHeaderBufPtr);

#ifdef SOFT_ENCRYPT
		RTMPUpdateSwCacheCipherInfo(pAd, pTxBlk, pHeaderBufPtr);
#endif // SOFT_ENCRYPT //
	}
	else 
	{
		APFindCipherAlgorithm(pAd, pTxBlk);
		APBuildCommon802_11Header(pAd, pTxBlk);
			
		pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	}

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
	{				
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE)
		{				
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}	
	}
#endif // SOFT_ENCRYPT //

#ifdef VENDOR_FEATURE1_SUPPORT
	if(pMacEntry->isCached
		&& (pMacEntry->Protocol == (RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket)))
#ifdef SOFT_ENCRYPT
		&& !TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)
#endif // SOFT_ENCRYPT //
	)
	{
		pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;
			
		// skip common header
		pHeaderBufPtr += pTxBlk->MpduHeaderLen;

		//
		// build QOS Control bytes
		// 
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_AP_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif // WDS_SUPPORT //
			)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif // UAPSD_AP_SUPPORT //
		pTxBlk->MpduHeaderLen = pMacEntry->MpduHeaderLen;
		pHeaderBufPtr = ((PUCHAR)pHeader_802_11) + pTxBlk->MpduHeaderLen;

		pTxBlk->HdrPadLen = pMacEntry->HdrPadLen;

		// skip 802.3 header
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen  -= LENGTH_802_3;

		// skip vlan tag
		if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket))
		{
			pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
			pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
		}
	}
	else
#endif // VENDOR_FEATURE1_SUPPORT //
	{
		pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;
			
		// skip common header
		pHeaderBufPtr += pTxBlk->MpduHeaderLen;

		//
		// build QOS Control bytes
		// 
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_AP_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif // WDS_SUPPORT //
			)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif // UAPSD_AP_SUPPORT //

		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;

		//
		// build HTC+ 
		// HTC control filed following QoS field
		// 
		if ((pAd->CommonCfg.bRdg == TRUE) 
			&& (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))
		)
		{
			// mark HTC bit 
			pHeader_802_11->FC.Order = 1;
			NdisZeroMemory(pHeaderBufPtr, 4);
			*(pHeaderBufPtr+3) |= 0x80;

			bHTCPlus = TRUE;
		}

			
	if (bHTCPlus == TRUE)
	{
			pHeaderBufPtr += 4;
			pTxBlk->MpduHeaderLen += 4;
		}


		//pTxBlk->MpduHeaderLen = pHeaderBufPtr - pTxBlk->HeaderBuf - TXWI_SIZE - TXINFO_SIZE;
		ASSERT(pTxBlk->MpduHeaderLen >= 24);

		// skip 802.3 header
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen  -= LENGTH_802_3;

		// skip vlan tag
		if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket))
		{
			pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
			pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
		}

		// The remaining content of MPDU header should locate 
		// at 4-octets aligment. 
		// @@@ MpduHeaderLen excluding padding @@@
		pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
		pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
		pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef VENDOR_FEATURE1_SUPPORT
		pMacEntry->HdrPadLen = pTxBlk->HdrPadLen;
#endif // VENDOR_FEATURE1_SUPPORT //

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		{			
			UCHAR	iv_offset = 0, ext_offset = 0;
			
		
			// if original Ethernet frame contains no LLC/SNAP, 
			// then an extra LLC/SNAP encap is required 	 	
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);
			
			// Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer	
			if (pTxBlk->pExtraLlcSnapEncap)
			{
				// Reserve the front 8 bytes of data for LLC header 
				pTxBlk->pSrcBufData -= LENGTH_802_1_H;
				pTxBlk->SrcBufLen  += LENGTH_802_1_H;

				NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);					
			}	
							
			/* Construct and insert specific IV header to MPDU header */
			RTMPSoftConstructIVHdr(pTxBlk->CipherAlg, 
								   pTxBlk->KeyIdx, 
								   pTxBlk->pKey->TxTsc,
								   pHeaderBufPtr, 
								   &iv_offset);
			pHeaderBufPtr += iv_offset;
			pTxBlk->MpduHeaderLen += iv_offset;

			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd, 
									 pTxBlk->CipherAlg, 
									 (PUCHAR)pHeader_802_11, 
									pTxBlk->pSrcBufData, 
									pTxBlk->SrcBufLen, 
									pTxBlk->KeyIdx,
									   pTxBlk->pKey,
									 &ext_offset);
			pTxBlk->SrcBufLen += ext_offset;
			pTxBlk->TotalFrameLen += ext_offset;
								
		}
		else
#endif // SOFT_ENCRYPT //
		{


			//
			// Insert LLC-SNAP encapsulation - 8 octets
			//
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData-2, pTxBlk->pExtraLlcSnapEncap);
			if (pTxBlk->pExtraLlcSnapEncap)
			{
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);

				pHeaderBufPtr += 6;
				// get 2 octets (TypeofLen)
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);

				pHeaderBufPtr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		}

#ifdef VENDOR_FEATURE1_SUPPORT
		pMacEntry->Protocol = RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket);
		pMacEntry->MpduHeaderLen = pTxBlk->MpduHeaderLen;
#endif // VENDOR_FEATURE1_SUPPORT //
	}

	if ((pMacEntry->isCached))
	{
		RTMPWriteTxWI_Cache(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
	}
	else
	{
		RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);

		NdisZeroMemory((PUCHAR)(&pMacEntry->CachedBuf[0]), sizeof(pMacEntry->CachedBuf));
		NdisMoveMemory((PUCHAR)(&pMacEntry->CachedBuf[0]), (PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (pHeaderBufPtr - (PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE])));

#ifdef VENDOR_FEATURE1_SUPPORT
		/* use space to get performance enhancement */
		NdisZeroMemory((PUCHAR)(&pMacEntry->HeaderBuf[0]), sizeof(pMacEntry->HeaderBuf));
		NdisMoveMemory((PUCHAR)(&pMacEntry->HeaderBuf[0]), (PUCHAR)(&pTxBlk->HeaderBuf[0]), (pHeaderBufPtr - (PUCHAR)(&pTxBlk->HeaderBuf[0])));
#endif // VENDOR_FEATURE1_SUPPORT //

		pMacEntry->isCached = TRUE;
	}

#ifdef STATS_COUNT_SUPPORT
	// calculate Transmitted AMPDU count and ByteCount 	
	{
		pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart ++;
		pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;		
	}

	// calculate Tx count and ByteCount per BSS
#ifdef WAPI_SUPPORT
	if (IS_ENTRY_CLIENT(pMacEntry))
#endif // WAPI_SUPPORT //
	{
		MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;

#ifdef WAPI_SUPPORT
		if (pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
#endif // WAPI_SUPPORT //
			
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;


			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;

		}

		if(pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
		}
	}

#ifdef WDS_SUPPORT
		if (pMacEntry && IS_ENTRY_WDS(pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedFragmentCount);				
			pAd->WdsTab.WdsEntry[pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}	
#endif // WDS_SUPPORT //
#endif // STATS_COUNT_SUPPORT //

	//FreeNumber = GET_TXRING_FREENO(pAd, QueIdx);

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);


	//
	// Kick out Tx
	// 
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
	
}


VOID AP_AMSDU_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;
//	UCHAR			QueIdx = pTxBlk->QueIdx;
	USHORT			FreeNumber;
	USHORT			subFramePayloadLen = 0;	// AMSDU Subframe length without AMSDU-Header / Padding.
	USHORT			totalMPDUSize=0;
	UCHAR			*subFrameHeader;
	UCHAR			padding = 0;
	USHORT			FirstTx = 0, LastTxIdx = 0;
	int 			frameNum = 0;
	PQUEUE_ENTRY	pQEntry;

		
		
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	PAPCLI_STRUCT   pApCliEntry = NULL;
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

	
	ASSERT((pTxBlk->TxPacketList.Number > 1));

	while(pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
		{
#ifdef STATS_COUNT_SUPPORT
			MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;

			if (pMbss != NULL)
				pMbss->TxDropCount++;
#endif // STATS_COUNT_SUPPORT //
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}
		
		// skip 802.3 header
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen  -= LENGTH_802_3;

		// skip vlan tag
		if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket))
		{
			pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
			pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
		}
		
		if (frameNum == 0)
		{
			pHeaderBufPtr = AP_Build_AMSDU_Frame_Header(pAd, pTxBlk);

			// NOTE: TxWI->MPDUtotalByteCount will be updated after final frame was handled.
			RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
		}
		else
		{
			pHeaderBufPtr = &pTxBlk->HeaderBuf[0];
			padding = ROUND_UP(LENGTH_AMSDU_SUBFRAMEHEAD + subFramePayloadLen, 4) - (LENGTH_AMSDU_SUBFRAMEHEAD + subFramePayloadLen);
			NdisZeroMemory(pHeaderBufPtr, padding + LENGTH_AMSDU_SUBFRAMEHEAD);
			pHeaderBufPtr += padding;
			pTxBlk->MpduHeaderLen = padding;
			pTxBlk->HdrPadLen += padding;
		}

		//
		// A-MSDU subframe
		//   DA(6)+SA(6)+Length(2) + LLC/SNAP Encap
		// 
		subFrameHeader = pHeaderBufPtr;
		subFramePayloadLen = pTxBlk->SrcBufLen;

		NdisMoveMemory(subFrameHeader, pTxBlk->pSrcBufHeader, 12);

#ifdef APCLI_SUPPORT
		if(TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[pTxBlk->pMacEntry->MatchAPCLITabIdx];
			if (pApCliEntry->Valid)
				NdisMoveMemory(&subFrameHeader[6] , pApCliEntry->CurrentAddress, 6);
		}
#endif // APCLI_SUPPORT //


		pHeaderBufPtr += LENGTH_AMSDU_SUBFRAMEHEAD;
		pTxBlk->MpduHeaderLen += LENGTH_AMSDU_SUBFRAMEHEAD;



		//
		// Insert LLC-SNAP encapsulation - 8 octets
		// 
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData-2, pTxBlk->pExtraLlcSnapEncap);

		subFramePayloadLen = pTxBlk->SrcBufLen;

		if (pTxBlk->pExtraLlcSnapEncap)
		{
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			// get 2 octets (TypeofLen)
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			subFramePayloadLen += LENGTH_802_1_H;
		}

		// update subFrame Length field
		subFrameHeader[12] = (subFramePayloadLen & 0xFF00) >> 8;
		subFrameHeader[13] = subFramePayloadLen & 0xFF;

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;


		//FreeNumber = GET_TXRING_FREENO(pAd, QueIdx);

		if (frameNum ==0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);
		else
			LastTxIdx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);	

		frameNum++;

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;		

#ifdef STATS_COUNT_SUPPORT
		// calculate Transmitted AMSDU Count and ByteCount		
		{
			pAd->RalinkCounters.TransmittedAMSDUCount.u.LowPart ++;
			pAd->RalinkCounters.TransmittedOctetsInAMSDU.QuadPart += totalMPDUSize;			
		}

		// calculate Tx count and ByteCount per BSS
#ifdef WAPI_SUPPORT
		if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif // WAPI_SUPPORT //
		{
			MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;
			MAC_TABLE_ENTRY	*pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT	
			if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
				pTxBlk->pMacEntry->wapi_usk_rekey_cnt += totalMPDUSize;
#endif // WAPI_SUPPORT //
		
			if (pMbss != NULL)
			{
				pMbss->TransmittedByteCount += totalMPDUSize;
				pMbss->TxCount ++;


				if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->mcPktsTx++;
				else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->bcPktsTx++;
				else
					pMbss->ucPktsTx++;

			}

			if(pMacEntry->Sst == SST_ASSOC)
			{
				INC_COUNTER64(pMacEntry->TxPackets);
				pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
			}
		}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}	
#endif // WDS_SUPPORT //
#endif // STATS_COUNT_SUPPORT //
	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	
	//
	// Kick out Tx
	// 
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}
#endif // DOT11_N_SUPPORT //


VOID AP_Legacy_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	HEADER_802_11	*pHeader_802_11;
	PUCHAR			pHeaderBufPtr;
//	UCHAR			QueIdx = pTxBlk->QueIdx;
	USHORT			FreeNumber;
	BOOLEAN			bVLANPkt;
	PQUEUE_ENTRY	pQEntry;

	
	ASSERT(pTxBlk);


	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
	{
#ifdef STATS_COUNT_SUPPORT
		MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount++;
#endif // STATS_COUNT_SUPPORT //
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

#ifdef STATS_COUNT_SUPPORT
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
	{
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
	}
#endif // STATS_COUNT_SUPPORT //

	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
	
	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
	{				
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE)
		{				
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}	
	}
#endif // SOFT_ENCRYPT //

	// skip 802.3 header
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen  -= LENGTH_802_3;

	//DBGPRINT(RT_DEBUG_TRACE, ("Dump Original packet: Len=%d!\n", GET_OS_PKT_LEN(pTxBlk->pTxPacket)));
	//hex_dump("Pkt:", GET_OS_PKT_DATAPTR(pTxBlk->pTxPacket), GET_OS_PKT_LEN(pTxBlk->pTxPacket));

	// skip vlan tag
	if (bVLANPkt)
	{
		pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
		pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
	}

	// record these MCAST_TX frames for group key rekey  
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
	{				
		INT	idx;

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
		{
			if (pAd->ApCfg.MBSSID[idx].REKEYTimerRunning && 
				pAd->ApCfg.MBSSID[idx].WPAREKEY.ReKeyMethod == PKT_REKEY)
			{
				pAd->ApCfg.MBSSID[idx].REKEYCOUNTER += (pTxBlk->SrcBufLen);
			}
		}
#ifdef WAPI_SUPPORT
		if (pAd->CommonCfg.WapiMskRekeyTimerRunning &&
			pAd->CommonCfg.wapi_msk_rekey_method == REKEY_METHOD_PKT)
		{
			
			pAd->CommonCfg.wapi_msk_rekey_cnt += (pTxBlk->SrcBufLen);
		}		
#endif // WAPI_SUPPORT //
	}

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	// skip common header
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
	{
		//
		// build QOS Control bytes
		// 
		*pHeaderBufPtr = ((pTxBlk->UserPriority & 0x0F) | (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx]<<5));
#ifdef UAPSD_AP_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif // WDS_SUPPORT //
		)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif // UAPSD_AP_SUPPORT //
	
		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;
	}

	// The remaining content of MPDU header should locate at 4-octets aligment	
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
	{			
		UCHAR	iv_offset = 0, ext_offset = 0;
	
		// if original Ethernet frame contains no LLC/SNAP, 
		// then an extra LLC/SNAP encap is required 	 	
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);
		
		// Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer	
		if (pTxBlk->pExtraLlcSnapEncap)
		{
			// Reserve the front 8 bytes of data for LLC header 
			pTxBlk->pSrcBufData -= LENGTH_802_1_H;
			pTxBlk->SrcBufLen  += LENGTH_802_1_H;

			NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);					
		}	
						
		/* Construct and insert specific IV header to MPDU header */
		RTMPSoftConstructIVHdr(pTxBlk->CipherAlg, 
							   pTxBlk->KeyIdx, 
							   pTxBlk->pKey->TxTsc,
							   pHeaderBufPtr, 
							   &iv_offset);
		pHeaderBufPtr += iv_offset;
		pTxBlk->MpduHeaderLen += iv_offset;

		/* Encrypt the MPDU data by software */
		RTMPSoftEncryptionAction(pAd, 
								 pTxBlk->CipherAlg, 
								 (PUCHAR)pHeader_802_11, 
								pTxBlk->pSrcBufData, 
								pTxBlk->SrcBufLen, 
								pTxBlk->KeyIdx,
								   pTxBlk->pKey,
								 &ext_offset);
		pTxBlk->SrcBufLen += ext_offset;
		pTxBlk->TotalFrameLen += ext_offset;
					
	}
	else
#endif // SOFT_ENCRYPT //
	{

		//
		// Insert LLC-SNAP encapsulation - 8 octets
		// 
		//
   		// if original Ethernet frame contains no LLC/SNAP, 
		// then an extra LLC/SNAP encap is required 
		// 
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader, pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap)
		{
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			// skip vlan tag
			vlan_size =  (bVLANPkt) ? LENGTH_802_1Q : 0;
			// get 2 octets (TypeofLen)
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufHeader+12+vlan_size, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}

#ifdef STATS_COUNT_SUPPORT
	// calculate Tx count and ByteCount per BSS
#ifdef WAPI_SUPPORT	
	if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif // WAPI_SUPPORT //
	{
		MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;
		MAC_TABLE_ENTRY	*pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT	
		if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pTxBlk->pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
#endif // WAPI_SUPPORT //		
	
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;

		}

		if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
		}
	}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif // WDS_SUPPORT //
#endif // STATS_COUNT_SUPPORT //

	//
	// prepare for TXWI
	// use Wcid as Key Index
	//

	// update Hardware Group Key Index
	if (!pTxBlk->pMacEntry)
	{
		// use Wcid as Hardware Key Index
		GET_GroupKey_WCID(pTxBlk->Wcid, pTxBlk->apidx, pAd);
	}

	RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);

	//FreeNumber = GET_TXRING_FREENO(pAd, QueIdx);

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);
	
	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	//
	// Kick out Tx
	// 
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);


}


VOID AP_Fragment_Frame_Tx(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK		*pTxBlk)
{
	HEADER_802_11	*pHeader_802_11;
	PUCHAR			pHeaderBufPtr;
//	UCHAR			QueIdx = pTxBlk->QueIdx;
	USHORT			FreeNumber;
	UCHAR 			fragNum = 0;
	USHORT			EncryptionOverhead = 0;	
	UINT32			FreeMpduSize, SrcRemainingBytes;
	USHORT			AckDuration;
	UINT 			NextMpduSize;
	BOOLEAN			bVLANPkt;
	PQUEUE_ENTRY	pQEntry;
	PACKET_INFO		PacketInfo;
#ifdef SOFT_ENCRYPT
	PUCHAR			tmp_ptr = NULL;
	UINT32			buf_offset = 0;
#endif // SOFT_ENCRYPT //
	HTTRANSMIT_SETTING	*pTransmit;

	
	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

	if(RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
	{
#ifdef STATS_COUNT_SUPPORT
		MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount++;
#endif//STATS_COUNT_SUPPORT//
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}
	
	ASSERT(TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag));

	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
	
	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);
	
#ifdef SOFT_ENCRYPT
	// Check if the original data has enough buffer 
	// to insert or append extended field.
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
	{		
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE)
		{
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}	
	}
#endif // SOFT_ENCRYPT //
	
	if (pTxBlk->CipherAlg == CIPHER_TKIP)
	{
		pTxBlk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, pTxBlk->pPacket);
		if (pTxBlk->pPacket == NULL)
			return;
		RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	}
	
	// skip 802.3 header
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen  -= LENGTH_802_3;

	// skip vlan tag
	if (bVLANPkt)
	{
		pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
		pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
	}

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *)pHeaderBufPtr;

	// skip common header
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
	{
		//
		// build QOS Control bytes
		// 
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_AP_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif // WDS_SUPPORT //
		)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif // UAPSD_AP_SUPPORT //
	
		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;
	}

	// The remaining content of MPDU header should locate at 4-octets aligment
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
	{				
		UCHAR	iv_offset = 0;
	
		// if original Ethernet frame contains no LLC/SNAP, 
		// then an extra LLC/SNAP encap is required 	 	
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);
		
		// Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer	
		if (pTxBlk->pExtraLlcSnapEncap)
		{
			// Reserve the front 8 bytes of data for LLC header 
			pTxBlk->pSrcBufData -= LENGTH_802_1_H;
			pTxBlk->SrcBufLen  += LENGTH_802_1_H;

			NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);					
		}	
			
		/* Construct and insert specific IV header to MPDU header */
		RTMPSoftConstructIVHdr(pTxBlk->CipherAlg, 
							   pTxBlk->KeyIdx, 
							   pTxBlk->pKey->TxTsc, 
							   pHeaderBufPtr, 
							   &iv_offset);
		pHeaderBufPtr += iv_offset;
		pTxBlk->MpduHeaderLen += iv_offset;

	}
	else
#endif // SOFT_ENCRYPT //
	{

		//
		// Insert LLC-SNAP encapsulation - 8 octets
		// 
		//
   		// if original Ethernet frame contains no LLC/SNAP, 
		// then an extra LLC/SNAP encap is required 
		// 
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader, pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap)
		{
			UCHAR vlan_size;
	
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			// skip vlan tag
			vlan_size =  (bVLANPkt) ? LENGTH_802_1Q : 0;
			// get 2 octets (TypeofLen)
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufHeader+12+vlan_size, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}

	/*  1. If TKIP is used and fragmentation is required. Driver has to
		   append TKIP MIC at tail of the scatter buffer
		2. When TXWI->FRAG is set as 1 in TKIP mode, 
		   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC */
	/*  TKIP appends the computed MIC to the MSDU data prior to fragmentation into MPDUs. */
	if (pTxBlk->CipherAlg == CIPHER_TKIP)
	{
		RTMPCalculateMICValue(pAd, pTxBlk->pPacket, pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey, pTxBlk->apidx);

		// NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
		//			to correct lenght, refer to pTxBlk->SrcBufLen for the packet length in following progress.
		NdisMoveMemory(pTxBlk->pSrcBufData + pTxBlk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		//skb_put((RTPKT_TO_OSPKT(pTxBlk->pPacket))->tail, 8);
		pTxBlk->SrcBufLen += 8;
		pTxBlk->TotalFrameLen += 8;
	}

#ifdef STATS_COUNT_SUPPORT
	// calculate Tx count and ByteCount per BSS
#ifdef WAPI_SUPPORT
	if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif // WAPI_SUPPORT //
	{
		MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;
		MAC_TABLE_ENTRY	*pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT	
		if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pTxBlk->pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
#endif // WAPI_SUPPORT //		
	
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;


			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;

		}

		if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
		}
	}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif // WDS_SUPPORT //
#endif // STATS_COUNT_SUPPORT //

	//
	// calcuate the overhead bytes that encryption algorithm may add. This
	// affects the calculate of "duration" field
	//
	if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128)) 
		EncryptionOverhead = 8; //WEP: IV[4] + ICV[4];
	else if (pTxBlk->CipherAlg == CIPHER_TKIP)
		EncryptionOverhead = 12;//TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength
	else if (pTxBlk->CipherAlg == CIPHER_AES)
		EncryptionOverhead = 16;	// AES: IV[4] + EIV[4] + MIC[8]
#ifdef WAPI_SUPPORT
	else if (pTxBlk->CipherAlg == CIPHER_SMS4)
		EncryptionOverhead = 16;	// SMS4: MIC[16]
#endif // WAPI_SUPPORT //		
	else
		EncryptionOverhead = 0;

	pTransmit = pTxBlk->pTransmit;	
	// Decide the TX rate
	if (pTransmit->field.MODE == MODE_CCK)
		pTxBlk->TxRate = pTransmit->field.MCS;
	else if (pTransmit->field.MODE == MODE_OFDM)
		pTxBlk->TxRate = pTransmit->field.MCS + RATE_FIRST_OFDM_RATE;
	else
		pTxBlk->TxRate = RATE_6_5;

	// decide how much time an ACK/CTS frame will consume in the air
	if (pTxBlk->TxRate <= RATE_LAST_OFDM_RATE)
	AckDuration = RTMPCalcDuration(pAd, pAd->CommonCfg.ExpectedACKRate[pTxBlk->TxRate], 14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6_5, 14);
	//DBGPRINT(RT_DEBUG_INFO, ("!!!Fragment AckDuration(%d), TxRate(%d)!!!\n", AckDuration, pTxBlk->TxRate));

	// Init the total payload length of this frame.
	SrcRemainingBytes = pTxBlk->SrcBufLen;
	
	pTxBlk->TotalFragNum = 0xff;

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
	{
		// store the outgoing frame for calculating MIC per fragmented frame
		os_alloc_mem(pAd, (PUCHAR *)&tmp_ptr, pTxBlk->SrcBufLen);
		if (tmp_ptr == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory for SW MIC calculation !!!\n", 
										__FUNCTION__));
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
		NdisMoveMemory(tmp_ptr, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	}
#endif // SOFT_ENCRYPT //

	do {

		FreeMpduSize = pAd->CommonCfg.FragmentThreshold - LENGTH_CRC;

		FreeMpduSize -= pTxBlk->MpduHeaderLen;

		if (SrcRemainingBytes <= FreeMpduSize)
		{	
			// This is the last or only fragment		
			pTxBlk->SrcBufLen = SrcRemainingBytes;
			
			pHeader_802_11->FC.MoreFrag = 0;
			pHeader_802_11->Duration = pAd->CommonCfg.Dsifs + AckDuration;
			
			// Indicate the lower layer that this's the last fragment.
			pTxBlk->TotalFragNum = fragNum;
		}
		else
		{	// more fragment is required
			pTxBlk->SrcBufLen = FreeMpduSize;
			
			NextMpduSize = min(((UINT)SrcRemainingBytes - pTxBlk->SrcBufLen), ((UINT)pAd->CommonCfg.FragmentThreshold));
			pHeader_802_11->FC.MoreFrag = 1;
			pHeader_802_11->Duration = (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) + RTMPCalcDuration(pAd, pTxBlk->TxRate, NextMpduSize + EncryptionOverhead);
		}
		//DBGPRINT(RT_DEBUG_INFO, ("!!!%s : Frag#%d !!!\n", __FUNCTION__, pHeader_802_11->Frag));

		SrcRemainingBytes -= pTxBlk->SrcBufLen;

		if (fragNum == 0)
			pTxBlk->FrameGap = IFS_HTTXOP;
		else
			pTxBlk->FrameGap = IFS_SIFS;
		
#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		{
			UCHAR	ext_offset = 0;
		
			NdisMoveMemory(pTxBlk->pSrcBufData, tmp_ptr + buf_offset, pTxBlk->SrcBufLen);
			buf_offset += pTxBlk->SrcBufLen;

			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd, 
									 pTxBlk->CipherAlg, 
									 (PUCHAR)pHeader_802_11, 
									pTxBlk->pSrcBufData, 
									pTxBlk->SrcBufLen, 
									pTxBlk->KeyIdx,
									   pTxBlk->pKey,
									 &ext_offset);
			pTxBlk->SrcBufLen += ext_offset;
			pTxBlk->TotalFrameLen += ext_offset;
						
		}	
#endif // SOFT_ENCRYPT //
		
		RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
		
		//FreeNumber = GET_TXRING_FREENO(pAd, QueIdx);

		HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, &FreeNumber);
		
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
#endif // WAPI_SUPPORT //	
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
#endif // SOFT_ENCRYPT //	
		{
			// Update the frame number, remaining size of the NDIS packet payload.
			if (fragNum == 0 && pTxBlk->pExtraLlcSnapEncap)
				pTxBlk->MpduHeaderLen -= LENGTH_802_1_H;	// space for 802.11 header.
		}

		fragNum++;
		//SrcRemainingBytes -= pTxBlk->SrcBufLen;
		pTxBlk->pSrcBufData += pTxBlk->SrcBufLen;
		
		pHeader_802_11->Frag++;	 // increase Frag #
		
	}while(SrcRemainingBytes > 0);

#ifdef SOFT_ENCRYPT
	if (tmp_ptr != NULL)
		os_free_mem(pAd, tmp_ptr);
#endif // SOFT_ENCRYPT //

	//
	// Kick out Tx
	// 
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

		
}


VOID AP_ARalink_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;
//	UCHAR			QueIdx = pTxBlk->QueIdx;
	USHORT			FreeNumber;
	USHORT			totalMPDUSize=0;
	USHORT			FirstTx, LastTxIdx;
	int 			frameNum = 0;
	BOOLEAN			bVLANPkt;
	PQUEUE_ENTRY	pQEntry;
	
	ASSERT(pTxBlk);
	
	ASSERT((pTxBlk->TxPacketList.Number== 2));


	FirstTx = LastTxIdx = 0;  // Is it ok init they as 0?
	while(pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
		{
#ifdef STATS_COUNT_SUPPORT
			MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;

			if (pMbss != NULL)
				pMbss->TxDropCount++;
#endif//STATS_COUNT_SUPPORT//

			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;			
		}
		
		//pTxBlk->bVLANPkt = RTMP_GET_PACKET_VLAN(pTxBlk->pPacket);
		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
		
		// skip 802.3 header
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen  -= LENGTH_802_3;

		// skip vlan tag
		if (bVLANPkt)
		{
			pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
			pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
		}
		
		if (frameNum == 0)
		{	// For first frame, we need to create the 802.11 header + padding(optional) + RA-AGG-LEN + SNAP Header
		
			pHeaderBufPtr = AP_Build_ARalink_Frame_Header(pAd, pTxBlk);
			
			// It's ok write the TxWI here, because the TxWI->MPDUtotalByteCount 
			//	will be updated after final frame was handled.
			RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);


			//
			// Insert LLC-SNAP encapsulation - 8 octets
			// 
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData-2, pTxBlk->pExtraLlcSnapEncap);

			if (pTxBlk->pExtraLlcSnapEncap)
			{
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
				pHeaderBufPtr += 6;
				// get 2 octets (TypeofLen)
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);
				pHeaderBufPtr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		}
		else
		{	// For second aggregated frame, we need create the 802.3 header to headerBuf, because PCI will copy it to SDPtr0.
		
			pHeaderBufPtr = &pTxBlk->HeaderBuf[0];
			pTxBlk->MpduHeaderLen = 0;
			
			// A-Ralink sub-sequent frame header is the same as 802.3 header.
			//   DA(6)+SA(6)+FrameType(2)
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufHeader, 12);
			pHeaderBufPtr += 12;
			// get 2 octets (TypeofLen)
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen = LENGTH_ARALINK_SUBFRAMEHEAD;
		}

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		
		//FreeNumber = GET_TXRING_FREENO(pAd, QueIdx);
		if (frameNum ==0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);
		else
			LastTxIdx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);

		frameNum++;
		
		pAd->RalinkCounters.OneSecTxAggregationCount++;
		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

#ifdef STATS_COUNT_SUPPORT
		// calculate Tx count and ByteCount per BSS
#ifdef WAPI_SUPPORT
		if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
#endif // WAPI_SUPPORT //
		{
			MULTISSID_STRUCT *pMbss = pTxBlk->pMbss;
			MAC_TABLE_ENTRY	*pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT	
			if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
				pTxBlk->pMacEntry->wapi_usk_rekey_cnt += totalMPDUSize;
#endif // WAPI_SUPPORT //

			if (pMbss != NULL)
			{
				pMbss->TransmittedByteCount += totalMPDUSize;
				pMbss->TxCount ++;

				if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->mcPktsTx++;
				else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->bcPktsTx++;
				else
					pMbss->ucPktsTx++;
				}

			if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
			{
				INC_COUNTER64(pMacEntry->TxPackets);
				pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
			}

		}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->MatchWDSTabIdx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif // WDS_SUPPORT //

#endif // STATS_COUNT_SUPPORT //
	}


	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);


	//
	// Kick out Tx
	// 
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

}


/*
	========================================================================
	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set 
	appropriate ASIC register to kick hardware encryption before really
	sent out to air.

	Arguments:
		pAd 	   		Pointer to our adapter
		pTxBlk			Pointer to outgoing TxBlk structure.
		QueIdx			Queue index for processing

	Return Value:
		None
	========================================================================
*/
NDIS_STATUS APHardTransmit(	
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			QueIdx)
{
	PQUEUE_ENTRY	pQEntry;
	PNDIS_PACKET	pPacket;
//	PQUEUE_HEADER   pQueue;

	if ((pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif // CARRIER_DETECTION_SUPPORT //
		)
	{
		while(pTxBlk->TxPacketList.Head)
		{
			pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
			pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
		return NDIS_STATUS_FAILURE;
	}

	switch (pTxBlk->TxFrameType)
	{
#ifdef DOT11_N_SUPPORT
		case TX_AMPDU_FRAME:
				AP_AMPDU_Frame_Tx(pAd, pTxBlk);
				break;
#endif // DOT11_N_SUPPORT //
		case TX_LEGACY_FRAME:
		case TX_MCAST_FRAME:
				AP_Legacy_Frame_Tx(pAd, pTxBlk);
				break;
#ifdef DOT11_N_SUPPORT
		case TX_AMSDU_FRAME:
				AP_AMSDU_Frame_Tx(pAd, pTxBlk);
				break;
#endif // DOT11_N_SUPPORT //
		case TX_RALINK_FRAME:
				AP_ARalink_Frame_Tx(pAd, pTxBlk);
				break;
		case TX_FRAG_FRAME:
				AP_Fragment_Frame_Tx(pAd, pTxBlk);
				break;
		default:
			{
				// It should not happened!
				DBGPRINT(RT_DEBUG_ERROR, ("Send a pacekt was not classified!! It should not happen!\n"));
				while(pTxBlk->TxPacketList.Head)
				{	
					pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
					pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
					if (pPacket)
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
			}
			break;
	}

	return (NDIS_STATUS_SUCCESS);
	
}


/*
	========================================================================
	Routine Description:
		Check Rx descriptor, return NDIS_STATUS_FAILURE if any error found
	========================================================================
*/
NDIS_STATUS APCheckRxError(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRT28XX_RXD_STRUC		pRxD,
	IN	UCHAR			Wcid)
{
	if (pRxD->Crc || pRxD->CipherErr)
	{
		// WCID equ to 255 mean MAC couldn't find any matched entry in Asic-MAC table.
		// The incoming packet mays come from WDS or AP-Client link.
		// We need them for further process. Can't drop the packet here.
		if ((pRxD->U2M)
			&& (pRxD->CipherErr)
			&& (Wcid == 255)
#ifdef WDS_SUPPORT
			&& (pAd->WdsTab.Mode == WDS_LAZY_MODE)
#endif // WDS_SUPPORT //
		)
		{
			// pass those packet for further process.
			return NDIS_STATUS_SUCCESS;
		}
		else
			return NDIS_STATUS_FAILURE;
	}
	else
	{
		return NDIS_STATUS_SUCCESS;
	}
}

/*
  ========================================================================
  Description:
	This routine checks if a received frame causes class 2 or class 3
	error, and perform error action (DEAUTH or DISASSOC) accordingly
  ========================================================================
*/
BOOLEAN APCheckClass2Class3Error(
	IN	PRTMP_ADAPTER	pAd,
	IN 	ULONG Wcid, 
	IN	PHEADER_802_11	pHeader)
{
	// software MAC table might be smaller than ASIC on-chip total size.
	// If no mathed wcid index in ASIC on chip, do we need more check???  need to check again. 06-06-2006
	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		APCls2errAction(pAd, MAX_LEN_OF_MAC_TABLE, pHeader);
		return TRUE;
	}

	if (pAd->MacTab.Content[Wcid].Sst == SST_ASSOC)
		; // okay to receive this DATA frame
	else if (pAd->MacTab.Content[Wcid].Sst == SST_AUTH)
	{
		APCls3errAction(pAd, Wcid, pHeader);
		return TRUE; 
	}
	else
	{
		APCls2errAction(pAd, Wcid, pHeader);
		return TRUE; 
	}
	return FALSE;
}

/*
  ========================================================================
  Description:
	This routine frees all packets in PSQ that's destined to a specific DA.
	BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL 
	is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID APHandleRxPsPoll(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pAddr,
	IN	USHORT			Aid,
    IN	BOOLEAN			isActive)
{ 
	PQUEUE_ENTRY	  pEntry;
	PMAC_TABLE_ENTRY  pMacEntry;
	unsigned long		IrqFlags;

	//DBGPRINT(RT_DEBUG_TRACE,("rcv PS-POLL (AID=%d) from %02x:%02x:%02x:%02x:%02x:%02x\n", 
	//	  Aid, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));

	pMacEntry = &pAd->MacTab.Content[Aid];
	if (RTMPEqualMemory(pMacEntry->Addr, pAddr, MAC_ADDR_LEN))
	{
		/*
			Sta is change to Power Active stat.
			Reset ContinueTxFailCnt
		*/
		pMacEntry->ContinueTxFailCnt = 0;

#ifdef UAPSD_AP_SUPPORT
        if (UAPSD_MR_IS_ALL_AC_UAPSD(isActive, pMacEntry))
		{
			/*
				IEEE802.11e spec.
				11.2.1.7 Receive operation for STAs in PS mode during the CP
				When a non-AP QSTA that is using U-APSD and has all ACs
				delivery-enabled detects that the bit corresponding to its AID
				is set in the TIM, the non-AP QSTA shall issue a trigger frame
				or a PS-Poll frame to retrieve the buffered MSDU or management
				frames.

				WMM Spec. v1.1a 070601
				3.6.2	U-APSD STA Operation
				3.6.2.3	In case one or more ACs are not
				delivery-enabled ACs, the WMM STA may retrieve MSDUs and
				MMPDUs belonging to those ACs by sending PS-Polls to the WMM AP.
				In case all ACs are delivery enabled ACs, WMM STA should only
				use trigger frames to retrieve MSDUs and MMPDUs belonging to
				those ACs, and it should not send PS-Poll frames.

				Different definitions in IEEE802.11e and WMM spec.
				But we follow the WiFi WMM Spec.
			*/

			DBGPRINT(RT_DEBUG_TRACE, ("All AC are UAPSD, can not use PS-Poll\n"));
            return; /* all AC are U-APSD, can not use PS-Poll */
        } /* End of if */
#endif // UAPSD_AP_SUPPORT //

		//NdisAcquireSpinLock(&pAd->MacTabLock);
		//NdisAcquireSpinLock(&pAd->TxSwQueueLock);
        RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
        if (isActive == FALSE)
        {
			if (pMacEntry->PsQueue.Head)
			{
#ifdef UAPSD_AP_SUPPORT
				UINT32 NumOfOldPsPkt;
				NumOfOldPsPkt = pAd->TxSwQueue[QID_AC_BE].Number;
#endif // UAPSD_AP_SUPPORT //

				pEntry = RemoveHeadQueue(&pMacEntry->PsQueue);
				if ( pMacEntry->PsQueue.Number >=1 )
					RTMP_SET_PACKET_MOREDATA(RTPKT_TO_OSPKT(pEntry), TRUE);
				InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QID_AC_BE], pEntry);

#ifdef UAPSD_AP_SUPPORT
				/* we need to call RTMPDeQueuePacket() immediately as below */
				if (NumOfOldPsPkt != pAd->TxSwQueue[QID_AC_BE].Number)
				{
					if (RTMP_GET_PACKET_DHCP(RTPKT_TO_OSPKT(pEntry)) ||
						RTMP_GET_PACKET_EAPOL(RTPKT_TO_OSPKT(pEntry)) ||
						RTMP_GET_PACKET_WAI(RTPKT_TO_OSPKT(pEntry)))
					{
						/*
							These packets will use 1M/6M rate to send.
							If you use 1M(2.4G)/6M(5G) to send, no statistics
							count in NICUpdateFifoStaCounters().

							So we can not count it for UAPSD; Or the SP will
							not closed until timeout.
						*/
						;
					}
					else
						UAPSD_MR_MIX_PS_POLL_RCV(pAd, pMacEntry);
				}
#endif // UAPSD_AP_SUPPORT //
			}
			else
			{
				/*
					or transmit a (QoS) Null Frame;

					In addtion, in Station Keep Alive mechanism, we need to
					send a QoS Null frame to detect the station live status.
				*/
				BOOLEAN bQosNull = FALSE;

				if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					bQosNull = TRUE;

	            ApEnqueueNullFrame(pAd, pMacEntry->Addr, pMacEntry->CurrTxRate,
    	                           Aid, pMacEntry->apidx, bQosNull, TRUE, 0);
			}
        }
        else
        {
#ifdef UAPSD_AP_SUPPORT
			/* deliver all queued UAPSD packets */
            UAPSD_AllPacketDeliver(pAd, pMacEntry);

			/* end the SP if exists */
			UAPSD_MR_ENTRY_RESET(pAd, pMacEntry);
#endif // UAPSD_AP_SUPPORT //

			while(pMacEntry->PsQueue.Head)
			{
//				if (pAd->TxSwQueue[QID_AC_BE].Number <=
//                    (pAd->PortCfg.TxQueueSize + (MAX_PACKETS_IN_PS_QUEUE>>1)))
                {
					pEntry = RemoveHeadQueue(&pMacEntry->PsQueue);
					InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QID_AC_BE], pEntry);
				}
//                else
//					break;
				/* End of if */
			} /* End of while */
        } /* End of if */

		//NdisReleaseSpinLock(&pAd->TxSwQueueLock);
		//NdisReleaseSpinLock(&pAd->MacTabLock);

		if ((Aid > 0) && (Aid < MAX_LEN_OF_MAC_TABLE) &&
			(pMacEntry->PsQueue.Number == 0))
		{
			// clear corresponding TIM bit because no any PS packet
			WLAN_MR_TIM_BIT_CLEAR(pAd, pMacEntry->apidx, Aid);
			pMacEntry->PsQIdleCount = 0;
		}

		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

		// Dequeue outgoing frames from TxSwQueue0..3 queue and process it
		// TODO: 2004-12-27 it's not a good idea to handle "More Data" bit here. because the
		// RTMPDeQueue process doesn't guarantee to de-queue the desired MSDU from the corresponding
		// TxSwQueue/PsQueue when QOS in-used. We should consider "HardTransmt" this MPDU
		// using MGMT queue or things like that.
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
		
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE,("rcv PS-POLL (AID=%d not match) from %02x:%02x:%02x:%02x:%02x:%02x\n", 
			  Aid, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
	}
}



//
// detect AC Category of trasmitting packets
// to turn AC0(BE) TX_OP (MAC reg 0x1300)
// 
//static UCHAR is_on;
VOID detect_wmm_traffic(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR			UserPriority,
	IN	UCHAR			FlgIsOutput)
{
	/* For BE & BK case and TxBurst function is disabled */
	if ((pAd->CommonCfg.bEnableTxBurst == FALSE) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->CommonCfg.bRdg == FALSE)
#endif // DOT11_N_SUPPORT //
		&& (FlgIsOutput == 1)
	)
	{
		if (MapUserPriorityToAccessCategory[UserPriority] == QID_AC_BK)
		{
			/* has any BK traffic */
			if (pAd->flg_be_adjust == 0)
			{
				/* yet adjust */
#ifdef RTMP_MAC_PCI
				EDCA_AC_CFG_STRUC Ac0Cfg;

				RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Ac0Cfg.word);
				Ac0Cfg.field.AcTxop = 0x20;
				RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
#endif // RTMP_MAC_PCI //
				pAd->flg_be_adjust = 1;
				NdisGetSystemUpTime(&pAd->be_adjust_last_time);

				DBGPRINT(RT_DEBUG_TRACE, ("wmm> adjust be!\n"));
			}
		}
		else
		{
			if (pAd->flg_be_adjust != 0)
			{
				PQUEUE_HEADER pQueue;

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
#ifdef RTMP_MAC_PCI
						EDCA_AC_CFG_STRUC Ac0Cfg;

						RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Ac0Cfg.word);
						Ac0Cfg.field.AcTxop = 0x00;
						RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
#endif // RTMP_MAC_PCI //
						pAd->flg_be_adjust = 0;

						DBGPRINT(RT_DEBUG_TRACE, ("wmm> recover be!\n"));
					}
				}
				else
					NdisGetSystemUpTime(&pAd->be_adjust_last_time);
			}
		}
	}

	// count packets which priority is more than BE 
	if (UserPriority > 3)
	{
		pAd->OneSecondnonBEpackets++;

		if (pAd->OneSecondnonBEpackets > 100 
#ifdef DOT11_N_SUPPORT
			&& pAd->MacTab.fAnyStationMIMOPSDynamic
#endif // DOT11_N_SUPPORT //
		)
		{
			if (!pAd->is_on)
			{
#ifdef RTMP_MAC_PCI
				RTMP_IO_WRITE32(pAd,  EXP_ACK_TIME,	 0x005400ca );
#endif // RTMP_MAC_PCI //
				pAd->is_on = 1;
			}
		}
		else
		{
			if (pAd->is_on)
			{
#ifdef RTMP_MAC_PCI
				RTMP_IO_WRITE32(pAd,  EXP_ACK_TIME,	 0x002400ca );
#endif // RTMP_MAC_PCI //
				pAd->is_on = 0;
			}
		}
	}
}

//
// Wirte non-zero value to AC0 TXOP to boost performace
// To pass WMM, AC0 TXOP must be zero.
// It is necessary to turn AC0 TX_OP dynamically.
// 

VOID dynamic_tune_be_tx_op(
						  IN  PRTMP_ADAPTER   pAd,
						  IN  ULONG           nonBEpackets)
{
	UINT32 RegValue;
	AC_TXOP_CSR0_STRUC csr0;

	if (pAd->CommonCfg.bEnableTxBurst 
#ifdef DOT11_N_SUPPORT
		|| pAd->CommonCfg.bRdg
#endif // DOT11_N_SUPPORT //
	)
	{

		if (
#ifdef DOT11_N_SUPPORT
			(pAd->WIFItestbed.bGreenField && pAd->MacTab.fAnyStationNonGF == TRUE) ||
			((pAd->OneSecondnonBEpackets > nonBEpackets) || pAd->MacTab.fAnyStationMIMOPSDynamic) || 
#endif // DOT11_N_SUPPORT //
			(pAd->MacTab.fAnyTxOPForceDisable))
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE))
			{
				RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &RegValue);
				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
				{
					TX_LINK_CFG_STRUC   TxLinkCfg;

					RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
					TxLinkCfg.field.TxRDGEn = 0;
					RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

					RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
				}
				// disable AC0(BE) TX_OP
				RegValue  &= 0xFFFFFF00; // for WMM test
				//if ((RegValue & 0x0000FF00) == 0x00004300)
				//	RegValue += 0x00001100;
				RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, RegValue);
				if (pAd->CommonCfg.APEdcaParm.Txop[QID_AC_VO] != 102)
				{
					csr0.field.Ac0Txop = 0;		// QID_AC_BE
				}
				else
				{
					// for legacy b mode STA
					csr0.field.Ac0Txop = 10;		// QID_AC_BE
				}
				csr0.field.Ac1Txop = 0;		// QID_AC_BK
				RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
				RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);				
			}
		}
		else
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)==0)
			{
				// enable AC0(BE) TX_OP
				UCHAR	txop_value_burst = 0x20;	// default txop for Tx-Burst
				UCHAR   txop_value;

#ifdef LINUX
#endif // LINUX //

				RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &RegValue);
				
				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
					txop_value = 0x80;
				else if (pAd->CommonCfg.bEnableTxBurst)
					txop_value = txop_value_burst;
				else
					txop_value = 0;

				RegValue  &= 0xFFFFFF00;
				//if ((RegValue & 0x0000FF00) == 0x00005400)
				//	RegValue -= 0x00001100;
				//txop_value = 0;
				RegValue  |= txop_value;  // for performance, set the TXOP to non-zero			
				RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, RegValue);
				csr0.field.Ac0Txop = txop_value;	// QID_AC_BE
				csr0.field.Ac1Txop = 0;				// QID_AC_BK
				RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE);				
			}
		}
	}
	pAd->OneSecondnonBEpackets = 0;
}


VOID APRxDErrorHandle(	
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk)
{
	MAC_TABLE_ENTRY		*pEntry;
	PRT28XX_RXD_STRUC			pRxD = &(pRxBlk->RxD);
	PRXWI_STRUC			pRxWI = pRxBlk->pRxWI;


	if (pRxD->U2M && pRxD->CipherErr)
	{		
		if (pRxWI->WirelessCliID < MAX_LEN_OF_MAC_TABLE)
		{
			pEntry = &pAd->MacTab.Content[pRxWI->WirelessCliID];

			// MIC error
			// Before verifying the MIC, the receiver shall check FCS, ICV and TSC. 
			// This avoids unnecessary MIC failure events. 
			if ((pEntry->WepStatus == Ndis802_11Encryption2Enabled)
					&& (pRxD->CipherErr == 2))
			{
#ifdef HOSTAPD_SUPPORT
				if(pEntry && pAd->ApCfg.Hostapd == TRUE)
				{
					ieee80211_notify_michael_failure(pAd, pRxBlk->pHeader, (UINT32) pRxWI->KeyIndex, 0);
				}
	      			else
#endif//HOSTAPD_SUPPORT//
      				{
      				RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
      				}

			}

			// send wireless event - for icv error
			if (pEntry && ((pRxD->CipherErr & 1) == 1))
				RTMPSendWirelessEvent(pAd, IW_ICV_ERROR_EVENT_FLAG, pEntry->Addr, 0, 0); 
		}

		DBGPRINT(RT_DEBUG_TRACE, ("Rx u2me Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n", 
					pRxWI->MPDUtotalByteCount, pRxWI->WirelessCliID, pRxD->CipherErr));

	}

	pAd->Counters8023.RxErrors++;
	DBGPRINT(RT_DEBUG_TRACE, ("APCheckRxError\n"));

}


BOOLEAN APCheckVaildDataFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	PRXWI_STRUC		pRxWI = pRxBlk->pRxWI;

	BOOLEAN isVaild = FALSE;

	do
	{
#ifndef APCLI_SUPPORT
		// should not drop Ap-Client packet.
		if (pHeader->FC.ToDs == 0)
			break; // give up this frame
#endif // APCLI_SUPPORT //
	
		// check if Class2 or 3 error
		if ((pHeader->FC.FrDs == 0) && (APCheckClass2Class3Error(pAd, pRxWI->WirelessCliID, pHeader))) 
			break; // give up this frame
	
		if(pAd->ApCfg.BANClass3Data == TRUE)
			break; // give up this frame

		isVaild = TRUE;
	} while (0);

	return isVaild;
}

// For TKIP frame, calculate the MIC value						
BOOLEAN APCheckTkipMICValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk)
{
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	UCHAR			*pData = pRxBlk->pData;
	USHORT			DataSize = pRxBlk->DataSize;
	UCHAR			UserPriority = pRxBlk->UserPriority;
	PCIPHER_KEY		pWpaKey;
	UCHAR			*pDA, *pSA;

	pWpaKey = &pEntry->PairwiseKey;

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS))
	{
		pDA = pHeader->Addr3;
		pSA = (PUCHAR)pHeader + sizeof(HEADER_802_11);
	}
	else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_APCLI))
	{
		pDA = pHeader->Addr1;
		pSA = pHeader->Addr3;		
	}
	else 
	{
		pDA = pHeader->Addr3;
		pSA = pHeader->Addr2;
	}

	if (RTMPTkipCompareMICValue(pAd,
								pData,
								pDA,
								pSA,
								pWpaKey->RxMic,
								UserPriority,
								DataSize) == FALSE)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR,("Rx MIC Value error 2\n"));
		RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
		// release packet
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return FALSE;
	}

	return TRUE;
}


VOID APHandleRxMgmtFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
	PRT28XX_RXD_STRUC		pRxD = &(pRxBlk->RxD);
	PRXWI_STRUC		pRxWI = pRxBlk->pRxWI;
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	PNDIS_PACKET	pRxPacket = pRxBlk->pRxPacket;

	do
	{


#ifdef IDS_SUPPORT	
		// Check if a rogue AP impersonats our mgmt frame to spoof clients 	
		if (RTMPSpoofedMgmtDetection(pAd, pHeader, pRxWI->RSSI0, pRxWI->RSSI1, pRxWI->RSSI2))
		{
			// This is a spoofed frame, so give up it.
			break;
		}
#endif // IDS_SUPPORT //

#ifdef IDS_SUPPORT
		// update sta statistics for traffic flooding detection later		
		RTMPUpdateStaMgmtCounter(pAd, pHeader->FC.SubType);
#endif // IDS_SUPPORT //
			
		if (!pRxD->U2M)
		{
			if ((pHeader->FC.SubType != SUBTYPE_BEACON) && (pHeader->FC.SubType != SUBTYPE_PROBE_REQ))
			{
				// give up this frame
#ifdef WMM_ACM_SUPPORT
				if ((pHeader->FC.SubType == SUBTYPE_ACTION) &&
					(ACMP_IsBwAnnounceActionFrame(pAd, (VOID *)pHeader) != ACM_RTN_OK))
#endif // WMM_ACM_SUPPORT //
				break;
			}
		}
	
		if (pAd->ApCfg.BANClass3Data == TRUE)
		{
			// disallow new association
			if ((pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) || (pHeader->FC.SubType == SUBTYPE_AUTH))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("   Disallow new Association \n "));
				// give up this frame
				break;
			}
		}

		/* Software decrypts WEP data during shared WEP negotiation */
		if ((pHeader->FC.SubType == SUBTYPE_AUTH) && 
			(pHeader->FC.Wep == 1) && (pRxD->Decrypted == 0))
		{
			PUCHAR	pMgmt = (PUCHAR)pHeader;
			UINT16	mgmt_len = pRxWI->MPDUtotalByteCount;
			PMAC_TABLE_ENTRY pEntry = NULL;

			/* Skip 802.11 headre */
			pMgmt += LENGTH_802_11;
			mgmt_len -= LENGTH_802_11;

			if (pRxWI->WirelessCliID < MAX_LEN_OF_MAC_TABLE)
			{
				pEntry = &pAd->MacTab.Content[pRxWI->WirelessCliID];
			}

			if (pEntry == NULL)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR : SW decrypt WEP data fails - the Entry is empty.\n"));	
				// give up this frame
				break;
			} 	

			/* handle WEP decryption */
			if (RTMPSoftDecryptWEP(pAd, 
								   &pAd->SharedKey[pEntry->apidx][pRxWI->KeyIndex], 
								   pMgmt, 
								   &mgmt_len) == FALSE)		
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR : SW decrypt WEP data fails.\n"));	
				// give up this frame
				break;
			}        	
#ifdef RT_BIG_ENDIAN
			// swap 16 bit fields - Auth Alg No. field
			*(USHORT *)pMgmt = SWAP16(*(USHORT *)pMgmt);

			// swap 16 bit fields - Auth Seq No. field
			*(USHORT *)(pMgmt + 2) = SWAP16(*(USHORT *)(pMgmt + 2));

			// swap 16 bit fields - Status Code field
			*(USHORT *)(pMgmt + 4) = SWAP16(*(USHORT *)(pMgmt + 4));
#endif // RT_BIG_ENDIAN //
													
			DBGPRINT(RT_DEBUG_TRACE, ("Descrypt AUTH seq#3 successfully\n"));	

			/* Update the total length */
			pRxWI->MPDUtotalByteCount -= (LEN_WEP_IV_HDR + LEN_ICV);			
		}

		if (pRxBlk->DataSize > MAX_RX_PKT_LEN)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("DataSize  = %d\n", pRxBlk->DataSize));
			hex_dump("MGMT ???", (UCHAR *)pHeader, pRxBlk->pData - (UCHAR *) pHeader);
			break;
		}

		if (pHeader->FC.SubType == SUBTYPE_ACTION)
		{
			/* only PM bit of ACTION frame can be set */
			MAC_TABLE_ENTRY *pEntry = NULL;

			pEntry = PACInquiry(pAd, pRxWI->WirelessCliID);
			if (pEntry != NULL)
			   	APPsIndicate(pAd, pHeader->Addr2, pEntry->Aid, pHeader->FC.PwrMgmt);

			/*
				In IEEE802.11, 11.2.1.1 STA Power Management modes,
				The Power Managment bit shall not be set in any management
				frame, except an Action frame.
			*/
			/* In IEEE802.11e, 11.2.1.4 Power management with APSD,
				If there is no unscheduled SP in progress, the unscheduled SP
				begins when the QAP receives a trigger frame from a non-AP QSTA,
				which is a QoS data or QoS Null frame associated with an AC the
				STA has configured to be trigger-enabled. */
			/* So a management action frame is not trigger frame */
		}

		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxWI->WirelessCliID, pHeader, pRxWI->MPDUtotalByteCount, 
								  pRxWI->RSSI0, pRxWI->RSSI1, pRxWI->RSSI2, 0);

	} while (0);

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
	return;
}

VOID APHandleRxControlFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	PNDIS_PACKET	pRxPacket = pRxBlk->pRxPacket;

	switch (pHeader->FC.SubType)
	{
#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK_REQ:
			{
				PRXWI_STRUC		pRxWI = pRxBlk->pRxWI;
				CntlEnqueueForRecv(pAd, pRxWI->WirelessCliID, (pRxWI->MPDUtotalByteCount), (PFRAME_BA_REQ)pHeader);
			}
			break;
#endif // DOT11_N_SUPPORT //
		// handle PS-POLL here
		case SUBTYPE_PS_POLL:
			{
				USHORT Aid = pHeader->Duration & 0x3fff;
				PUCHAR pAddr = pHeader->Addr2;

				if (Aid < MAX_LEN_OF_MAC_TABLE)
					APHandleRxPsPoll(pAd, pAddr, Aid, FALSE);
			}
			break;
#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK:
#endif // DOT11_N_SUPPORT //
		case SUBTYPE_ACK:
		default:		
			break;
	}

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

VOID APRxEAPOLFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	//PRT28XX_RXD_STRUC		pRxD = &(pRxBlk->RxD); // not used
	PRXWI_STRUC		pRxWI = pRxBlk->pRxWI;
	BOOLEAN 		CheckPktSanity = TRUE;
	UCHAR			*pTmpBuf;


	// Sanity Check		
	if(pRxBlk->DataSize < (LENGTH_802_1_H + LENGTH_EAPOL_H))
	{
		CheckPktSanity = FALSE;
		DBGPRINT(RT_DEBUG_ERROR, ("Total pkts size is too small.\n"));
	}	
	else if (!RTMPEqualMemory(SNAP_802_1H, pRxBlk->pData, 6))
	{
		CheckPktSanity = FALSE;	
		DBGPRINT(RT_DEBUG_ERROR, ("Can't find SNAP_802_1H parameter.\n"));
	}	 
	else if (!RTMPEqualMemory(EAPOL, pRxBlk->pData+6, 2))
	{
		CheckPktSanity = FALSE;	
		DBGPRINT(RT_DEBUG_ERROR, ("Can't find EAPOL parameter.\n"));	
	}	
	else if(*(pRxBlk->pData+9) > EAPOLASFAlert)
	{
		CheckPktSanity = FALSE;	
		DBGPRINT(RT_DEBUG_ERROR, ("Unknown EAP type(%d).\n", *(pRxBlk->pData+9)));	
	}

	if(CheckPktSanity == FALSE)
	{
		goto done;
	}


	
#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Indicate_Legacy_Packet\n"));
		Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		return;
	}
#endif//HOSTAPD_SUPPORT//

#ifdef DOT1X_SUPPORT
	// sent this frame to upper layer TCPIP
	if ((pEntry) && (pEntry->WpaState < AS_INITPMK) && 
		((pEntry->AuthMode == Ndis802_11AuthModeWPA) || 
		((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->PMKID_CacheIdx == ENTRY_NOT_FOUND)) || 
		pAd->ApCfg.MBSSID[pEntry->apidx].IEEE8021X == TRUE))
	{
#ifdef WSC_AP_SUPPORT                                
		if ((pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.WscConfMode != WSC_DISABLE) &&
            (!MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.EntryAddr, ZERO_MAC_ADDR)))
		{
			pTmpBuf = pRxBlk->pData - LENGTH_802_11;
			NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);
			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxWI->WirelessCliID, pTmpBuf, pRxBlk->DataSize + LENGTH_802_11, pRxWI->RSSI0, pRxWI->RSSI1, pRxWI->RSSI2, 0);
            pRxBlk->pHeader = (PHEADER_802_11)pTmpBuf;
		}       
#endif // WSC_AP_SUPPORT //


		Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		return; 
	}
	else	// sent this frame to WPA state machine
#endif // DOT1X_SUPPORT //	
	{
		pTmpBuf = pRxBlk->pData - LENGTH_802_11;
		NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);
		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxWI->WirelessCliID, pTmpBuf, pRxBlk->DataSize + LENGTH_802_11, pRxWI->RSSI0, pRxWI->RSSI1, pRxWI->RSSI2, 0);
	}

done:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;

}

VOID Announce_or_Forward_802_3_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID)
{
	if (APFowardWirelessStaToWirelessSta(pAd, pPacket, FromWhichBSSID))
	{
		announce_802_3_packet(pAd, pPacket);
	}
	else
	{
		// release packet
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}


VOID APRxDataFrameAnnounce(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{

	// non-EAP frame
	if (!RTMPCheckWPAframe(pAd, pEntry, pRxBlk->pData, pRxBlk->DataSize, FromWhichBSSID))
	{
#ifdef WAPI_SUPPORT
		// report to upper layer if the received frame is WAI frame
		if (RTMPCheckWAIframe(pRxBlk->pData, pRxBlk->DataSize))
		{
			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
			return;
		}			
#endif // WAPI_SUPPORT //

		/* 	
			drop all non-EAP DATA frame before
			this client's Port-Access-Control is secured
		 */
		if (pEntry->PrivacyFilter == Ndis802_11PrivFilter8021xWEP)
		{
			/*  				
				If	1) no any EAP frame is received within 5 sec and 
					2) an encrypted non-EAP frame from peer associated STA is received,
				AP would send de-authentication to this STA.
			 */
			if (IS_ENTRY_CLIENT(pEntry) && pRxBlk->pHeader->FC.Wep && 
				pEntry->StaConnectTime > 5 && pEntry->WpaState < AS_AUTHENTICATION2)
			{		
				DBGPRINT(RT_DEBUG_WARN, ("==> De-Auth this STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));	
				MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
			}
		
			// release packet
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}


#ifdef IGMP_SNOOP_SUPPORT
		if (pEntry
			&& (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
			&& (pAd->ApCfg.IgmpSnoopEnable) 
			&& IS_MULTICAST_MAC_ADDR(pRxBlk->pHeader->Addr3))
		{
			PUCHAR pDA = pRxBlk->pHeader->Addr3;
			PUCHAR pSA = pRxBlk->pHeader->Addr2;
			PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
			UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

			if (protoType == ETH_P_IP)
				IGMPSnooping(pAd, pDA, pSA, pData, get_netdev_from_bssid(pAd, FromWhichBSSID));
			else if (protoType == ETH_P_IPV6)
				MLDSnooping(pAd, pDA, pSA,  pData, get_netdev_from_bssid(pAd, FromWhichBSSID));
		}
#endif // IGMP_SNOOP_SUPPORT //

#ifdef STATS_COUNT_SUPPORT
		if (pEntry
			&& (IS_ENTRY_CLIENT(pEntry))
			&& (pEntry->pMbss))
		{
			MULTISSID_STRUCT *pMbss = pEntry->pMbss;
			if(IS_MULTICAST_MAC_ADDR(pRxBlk->pHeader->Addr3) || IS_MULTICAST_MAC_ADDR(pRxBlk->pHeader->Addr1))
			{
					pMbss->mcPktsRx++;
			}
			else if(IS_BROADCAST_MAC_ADDR(pRxBlk->pHeader->Addr3) || IS_BROADCAST_MAC_ADDR(pRxBlk->pHeader->Addr1))
			{
					pMbss->bcPktsRx++;
			}
			else
			{
					pMbss->ucPktsRx++;
			}
		}
#endif // STATS_COUNT_SUPPORT //
		RX_BLK_CLEAR_FLAG(pRxBlk, fRX_EAP);
		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK))
		{
			// Normal legacy, AMPDU or AMSDU
			CmmRxnonRalinkFrameIndicate(pAd, pRxBlk, FromWhichBSSID);
		}
		else
		{
			// ARALINK
			CmmRxRalinkFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		}
	}
	else 
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);

		/* Update the WPA STATE to indicate the EAP handshaking is started */
		if (pEntry->WpaState == AS_AUTHENTICATION)
			pEntry->WpaState = AS_AUTHENTICATION2;
		
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0)) 
		{			
			Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		} 
		else
#endif // DOT11_N_SUPPORT //
		{
			// Determin the destination of the EAP frame
			//  to WPA state machine or upper layer
			APRxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		}
	}
}



//
// All Rx routines use RX_BLK structure to hande rx events
// It is very important to build pRxBlk attributes
//  1. pHeader pointer to 802.11 Header
//  2. pData pointer to payload including LLC (just skip Header)
//  3. set payload size including LLC to DataSize
//  4. set some flags with RX_BLK_SET_FLAG()
// 
VOID APHandleRxDataFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
	PRT28XX_RXD_STRUC				pRxD = &(pRxBlk->RxD);
	PRXWI_STRUC						pRxWI = pRxBlk->pRxWI;
	PHEADER_802_11					pHeader = pRxBlk->pHeader;
	PNDIS_PACKET					pRxPacket = pRxBlk->pRxPacket;
	BOOLEAN 						bFragment = FALSE;
	MAC_TABLE_ENTRY	    			*pEntry = NULL;
	UCHAR							FromWhichBSSID = BSS0;
	UCHAR							OldPwrMgmt = PWR_ACTIVE;	// UAPSD AP SUPPORT
	UCHAR							UserPriority = 0;
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	BOOLEAN							bWdsPacket = FALSE;
#endif // WDS_SUPPORT || CLIENT_WDS //
	FRAME_CONTROL					*pFmeCtrl = &pHeader->FC;
	COUNTER_RALINK					*pCounter = &pAd->RalinkCounters;


	if (APCheckVaildDataFrame(pAd, pRxBlk) != TRUE)
	{
		goto err;		
	}

#ifdef IDS_SUPPORT
	// replay attack detection
	// Detect a spoofed data frame from a rogue AP, ignore it.
	if (pFmeCtrl->FrDs == 1 && 
		(RTMPReplayAttackDetection(pAd, pHeader->Addr2, pRxWI->RSSI0, pRxWI->RSSI1, pRxWI->RSSI2) == TRUE))
	{
		goto err;
	}
#endif // IDS_SUPPORT //

	//
	// handle WDS
	//
	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
	{
		do
		{
#ifdef CLIENT_WDS
			pEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pEntry != NULL)
			{
				if (IS_ENTRY_CLIWDS(pEntry))
					;
				else if (IS_ENTRY_CLIENT(pEntry)
						&& (pEntry->Sst == SST_ASSOC))
					SET_ENTRY_CLIWDS(pEntry);
				else
					pEntry = NULL;
			}

			if (pEntry != NULL)
			{
				FromWhichBSSID = pEntry->apidx;

				// Increase received byte counter per BSS
				if (FromWhichBSSID < pAd->ApCfg.BssidNum)
				{
					MULTISSID_STRUCT *pMbss = pEntry->pMbss;
					if (pMbss != NULL)
					{
						pMbss->ReceivedByteCount += pRxWI->MPDUtotalByteCount;
						pMbss->RxCount ++;
					}
				}
				RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
				bWdsPacket = TRUE;
				CliWds_ProxyTabUpdate(pAd, pEntry->Aid, pHeader->Octet);
				break;
			}
#endif // CLIENT_WDS //


#ifdef WDS_SUPPORT
			// handle WDS
			{
				bWdsPacket = TRUE;
				if (MAC_ADDR_EQUAL(pHeader->Addr1, pAd->CurrentAddress))
					pEntry = FindWdsEntry(pAd, pRxWI->WirelessCliID, pHeader->Addr2, pRxWI->PHYMODE);
				else
					pEntry = NULL;


				// have no valid wds entry exist 
				// then discard the incoming packet.
				if (!(pEntry && WDS_IF_UP_CHECK(pAd, pEntry->MatchWDSTabIdx)))
				{
					// drop the packet
					goto err;
				}

				//receive corresponding WDS packet, disable TX lock state (fix WDS jam issue)
				if(pEntry && (pEntry->LockEntryTx == TRUE)) 
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Receive WDS packet, disable TX lock state!\n"));
					pEntry->ContinueTxFailCnt = 0;
					pEntry->LockEntryTx = FALSE;
				}
		
				RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
				FromWhichBSSID = pEntry->MatchWDSTabIdx + MIN_NET_DEVICE_FOR_WDS;
				break;
			}
#endif // WDS_SUPPORT //
		} while(FALSE);

		if (pEntry == NULL)
		{
			// have no WDS or MESH support
			// drop the packet
			goto err;
		}
	}
	// handle APCLI.
	else if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0))
	{
#ifdef APCLI_SUPPORT
		if (VALID_WCID(pRxWI->WirelessCliID))
			pEntry = ApCliTableLookUpByWcid(pAd, pRxWI->WirelessCliID, pHeader->Addr2);
		else
			pEntry = MacTableLookup(pAd, pHeader->Addr2);

		if (pEntry && IS_ENTRY_APCLI(pEntry))
		{
			PAPCLI_STRUCT pApCliEntry;

			if (!(APCLI_IF_UP_CHECK(pAd, pEntry->MatchAPCLITabIdx)))
			{
				goto err;
			}

			pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->MatchAPCLITabIdx];

			/* APCli reconnect workaround - update ApCliRcvBeaconTime on RX activity too */
			pApCliEntry->ApCliRcvBeaconTime = pAd->Mlme.Now32;

			FromWhichBSSID = pEntry->MatchAPCLITabIdx + MIN_NET_DEVICE_FOR_APCLI;
			RX_BLK_SET_FLAG(pRxBlk, fRX_APCLI);

			// Process broadcast packets
			if (pRxD->Mcast || pRxD->Bcast)
			{
				// Process the received broadcast frame for AP-Client.			
				if (!ApCliHandleRxBroadcastFrame(pAd, pRxBlk, pEntry, FromWhichBSSID))			
				{
					// release packet
					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				}
				return;
			}
		}
		else
#endif // APCLI_SUPPORT //
		{
			// no APCLI support
			// release packet
			goto err;
		}
	}
	else
	{
		pEntry = PACInquiry(pAd, pRxWI->WirelessCliID);

		//	can't find associated STA entry then filter invlid data frame 
		if (!pEntry)
		{		
			goto err;
		}

		FromWhichBSSID = pEntry->apidx;

#ifdef STATS_COUNT_SUPPORT
		// Increase received byte counter per BSS
		if (pHeader->FC.FrDs == 0 &&
			pRxD->U2M &&
			FromWhichBSSID < pAd->ApCfg.BssidNum)
		{
			MULTISSID_STRUCT *pMbss = pEntry->pMbss;
			if (pMbss != NULL)
			{
				pMbss->ReceivedByteCount += pRxWI->MPDUtotalByteCount;
				pMbss->RxCount ++;
			}
		}
#endif // STATS_COUNT_SUPPORT //		
	}

	ASSERT(pEntry->Aid == pRxWI->WirelessCliID);

#ifdef STATS_COUNT_SUPPORT
	if(pEntry != NULL)
	{
		pEntry->RxBytes+=pRxWI->MPDUtotalByteCount;
		INC_COUNTER64(pEntry->RxPackets);
	}
#endif // STATS_COUNT_SUPPORT //


#ifdef DOT11_N_SUPPORT
	// check Atheros Client
	if (!pEntry->bIAmBadAtheros && (pFmeCtrl->Retry ) 
		&& (pRxD->AMPDU == 1) && (pAd->CommonCfg.bHTProtect == TRUE)
	)
	{
		if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
			AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, FALSE);
		pEntry->bIAmBadAtheros = TRUE;
	}
#endif // DOT11_N_SUPPORT //

   	// update rssi sample 
   	Update_Rssi_Sample(pAd, &pEntry->RssiSample, pRxWI);
#ifdef RT30xx
#ifdef ANT_DIVERSITY_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (((pAd->NicConfig2.field.AntDiversity)
#ifdef TXRX_SW_ANTDIV_SUPPORT
			|| (pAd->chipCap.bTxRxSwAntDiv)	
#endif
			) && ((pAd->CommonCfg.bRxAntDiversity != ANT_DIVERSITY_DISABLE)))
   		{	
			INT RssiSample;

			RssiSample = ConvertToRssi(pAd, (UCHAR)pRxWI->RSSI0, RSSI_0);
			AP_COLLECT_RX_ANTENNA_AVERAGE_RSSI(pAd, RssiSample, 0, 0);
   		}
	}
#endif // ANT_DIVERSITY_SUPPORT //
#endif // RT30xx //


   	// Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461
   	// must be here, before no DATA check 


	pRxBlk->pData = (UCHAR *)pHeader;


   	// 1: PWR_SAVE, 0: PWR_ACTIVE
   	OldPwrMgmt = APPsIndicate(pAd, pHeader->Addr2, pEntry->Aid, pFmeCtrl->PwrMgmt);
#ifdef UAPSD_AP_SUPPORT
	if (pFmeCtrl->PwrMgmt)
	{
	   	if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
			(pFmeCtrl->SubType & 0x08))
	   	{
			/*
				In IEEE802.11e, 11.2.1.4 Power management with APSD,
				If there is no unscheduled SP in progress, the unscheduled SP begins
				when the QAP receives a trigger frame from a non-AP QSTA, which is a
				QoS data or QoS Null frame associated with an AC the STA has
				configured to be trigger-enabled.
			*/
			/*
				In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
				to/from Power Save Mode is not considered to be a Trigger Frame and
				the AP shall not respond with a QoS Null frame.
			*/
			/* Trigger frame must be QoS data or QoS Null frame */
	   		UCHAR  OldUP;

			OldUP = (*(pRxBlk->pData+LENGTH_802_11) & 0x07);
	    	if (OldPwrMgmt == PWR_SAVE)
	    		UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
	    	/* End of if */
		}
    } /* End of if */
#endif // UAPSD_AP_SUPPORT //

	// Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame
	if ((pFmeCtrl->SubType & 0x04) && (pFmeCtrl->Order == 0)) // bit 2 : no DATA
	{
		// Increase received drop packet counter per BSS
		if (pFmeCtrl->FrDs == 0 &&
			pRxD->U2M &&
			pRxWI->BSSID < pAd->ApCfg.BssidNum)
		{
			pAd->ApCfg.MBSSID[pRxWI->BSSID].RxDropCount ++;			
		}

		// release packet
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	//
	// update RxBlk->pData, DataSize
	// 802.11 Header, QOS, HTC, Hw Padding
	// 

	// 1. skip 802.11 HEADER
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(MESH_SUPPORT)
	if (FALSE
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
		|| bWdsPacket
#endif // WDS_SUPPORT || CLIENT_WDS //
		)
	{
		pRxBlk->pData += LENGTH_802_11_WITH_ADDR4;
		pRxBlk->DataSize -= LENGTH_802_11_WITH_ADDR4;
	}
	else
#endif // WDS_SUPPORT || CLIENT_WDS || MESH_SUPPORT //
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
	}

	// 2. QOS
	if (pFmeCtrl->SubType & 0x08)
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_QOS);
		UserPriority = *(pRxBlk->pData) & 0x0f;

#ifdef WMM_ACM_SUPPORT
		ACMP_DATA_NULL_HANDLE(pAd, pEntry, pHeader, UserPriority);
#endif // WMM_ACM_SUPPORT //

		// count packets priroity more than BE
		detect_wmm_traffic(pAd, UserPriority, 0);
		// bit 7 in QoS Control field signals the HT A-MSDU format
		if ((*pRxBlk->pData) & 0x80)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);

			// calculate received AMSDU count and ByteCount
			pCounter->ReceivedAMSDUCount.u.LowPart ++;

#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(MESH_SUPPORT)
			if (FALSE
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
				|| bWdsPacket
#endif // WDS_SUPPORT || CLIENT_WDS//
				)
			{
				pCounter->ReceivedOctesInAMSDUCount.QuadPart += (pRxBlk->DataSize + LENGTH_802_11_WITH_ADDR4);
			}
			else
#endif // WDS_SUPPORT || CLIENT_WDS || MESH_SUPPORT //
			{
				pCounter->ReceivedOctesInAMSDUCount.QuadPart += (pRxBlk->DataSize + LENGTH_802_11);
			}
		}

		// skip QOS contorl field
		pRxBlk->pData += 2;
		pRxBlk->DataSize -=2;
	}
	pRxBlk->UserPriority = UserPriority;


	// 3. Order bit: A-Ralink or HTC+
	if (pFmeCtrl->Order)
	{
#ifdef AGGREGATION_SUPPORT
		if (
#ifdef DOT11_N_SUPPORT
			(pRxWI->PHYMODE < MODE_HTMIX) && 
#endif // DOT11_N_SUPPORT //
			(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE))
		)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		}
		else
#endif
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_HTC);
			// skip HTC contorl field
			pRxBlk->pData += 4;
			pRxBlk->DataSize -= 4;
		}
	}

	// 4. skip HW padding 
	if (pRxD->L2PAD)
	{
		// just move pData pointer 
		// because DataSize excluding HW padding 
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pRxBlk->pData += 2;
	}

	if (pRxD->BA)
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);

		// incremented by the number of MPDUs
		// received in the A-MPDU when an A-MPDU is received.
		pCounter->MPDUInReceivedAMPDUCount.u.LowPart ++;
	}

#ifdef SOFT_ENCRYPT
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1) 
	   and it's passed to driver with "Decrypted" marked as 0 in RxD. */
	if ((pHeader->FC.Wep == 1) && (pRxD->Decrypted == 0))
	{	
		if (RTMPSoftDecryptionAction(pAd, 
								 	(PUCHAR)pHeader, 
									 UserPriority, 
									 &pEntry->PairwiseKey, 
								 	 pRxBlk->pData, 
									 &(pRxBlk->DataSize)) != NDIS_STATUS_SUCCESS)
		{
			// release packet			
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
		/* Record the Decrypted bit as 1 */
		pRxD->Decrypted = 1;
	}
#endif // SOFT_ENCRYPT //

	if (!((pHeader->Frag == 0) && (pFmeCtrl->MoreFrag == 0)))
	{
		// re-assemble the fragmented packets
		// return complete frame (pRxPacket) or NULL   
		bFragment = TRUE;
		pRxPacket = RTMPDeFragmentDataFrame(pAd, pRxBlk);
	}

	if (pRxPacket)
	{
		// process complete frame
		if (bFragment && (pFmeCtrl->Wep) && (pEntry->WepStatus == Ndis802_11Encryption2Enabled))
		{
			// Minus MIC length
			pRxBlk->DataSize -= 8;

			// For TKIP frame, calculate the MIC value						
			if (APCheckTkipMICValue(pAd, pEntry, pRxBlk) == FALSE)
			{
				return;
			}
		}

#ifdef IKANOS_VX_1X0
		RTMP_SET_PACKET_IF(pRxPacket, FromWhichBSSID);
#endif // IKANOS_VX_1X0 //
		APRxDataFrameAnnounce(pAd, pEntry, pRxBlk, FromWhichBSSID);
	}
	else
	{
		// just return 
		// because RTMPDeFragmentDataFrame() will release rx packet, 
		// if packet is fragmented
		return;
	}
	return;

err:
	// Increase received error packet counter per BSS
	if (pFmeCtrl->FrDs == 0 &&
		pRxD->U2M &&
		pRxWI->BSSID < pAd->ApCfg.BssidNum)
	{
		pAd->ApCfg.MBSSID[pRxWI->BSSID].RxDropCount ++;
		pAd->ApCfg.MBSSID[pRxWI->BSSID].RxErrorCount ++;
	}

	// release packet
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
	return;
	
}

/*
		========================================================================
		Routine Description:
			Process RxDone interrupt, running in DPC level

		Arguments:
			pAd    Pointer to our adapter

		Return Value:
			None

		Note:
			This routine has to maintain Rx ring read pointer.
	========================================================================
*/


#undef	MAX_RX_PROCESS_CNT
#define MAX_RX_PROCESS_CNT	(32)

BOOLEAN APRxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd) 
{
	UINT32			RxProcessed, RxPending;
	BOOLEAN			bReschedule = FALSE;
	RT28XX_RXD_STRUC		*pRxD;
	UCHAR			*pData;
	PRXWI_STRUC		pRxWI;
	PNDIS_PACKET	pRxPacket;
	PHEADER_802_11	pHeader;
	RX_BLK			RxCell, *pRxCell;
	MULTISSID_STRUCT *pMbss;
#ifdef WDS_SUPPORT
	MAC_TABLE_ENTRY	    			*pEntry = NULL;
#endif

#ifdef LINUX
#endif // LINUX //

	RxProcessed = RxPending = 0;

	// process whole rx ring
	while (1)
	{

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
									fRTMP_ADAPTER_HALT_IN_PROGRESS)) || 
			!RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_START_UP))
		{
			break;
		}

#ifdef UAPSD_AP_SUPPORT
		UAPSD_TIMING_RECORD_INDEX(RxProcessed);
#endif // UAPSD_AP_SUPPORT //

#ifdef RTMP_MAC_PCI
		if (RxProcessed++ > MAX_RX_PROCESS_CNT)
		{
			// need to reschedule rx handle 
			bReschedule = TRUE;
			break;
		}

#ifdef UAPSD_AP_SUPPORT
		// static rate also need NICUpdateFifoStaCounters() function.
		//if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
		UAPSD_MR_SP_SUSPEND(pAd);
#endif // UAPSD_AP_SUPPORT //

#ifdef VENDOR_FEATURE1_SUPPORT
		/*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		if (++pAd->FifoUpdateDone >= 4)
		{
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateDone = 0;
		}
#else
		NICUpdateFifoStaCounters(pAd);
#endif // VENDOR_FEATURE1_SUPPORT //
#endif // RTMP_MAC_PCI //

		// 1. allocate a new data packet into rx ring to replace received packet 
		//    then processing the received packet
		// 2. the callee must take charge of release of packet
		// 3. As far as driver is concerned ,
		//    the rx packet must 
		//      a. be indicated to upper layer or 
		//      b. be released if it is discarded
		pRxCell = &RxCell;
		pRxPacket = GetPacketFromRxRing(pAd, &(pRxCell->RxD), &bReschedule, &RxPending);
		if (pRxPacket == NULL)
		{
			// no more packet to process
			break;
		}

		// get rx ring descriptor
		pRxD = &(pRxCell->RxD);
		// get rx data buffer
		pData	= GET_OS_PKT_DATAPTR(pRxPacket);
		pRxWI	= (PRXWI_STRUC)pData;
		pHeader = (PHEADER_802_11)(pData+RXWI_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (PUCHAR)pHeader, DIR_READ, TRUE);
		RTMPWIEndianChange((PUCHAR)pRxWI, TYPE_RXWI);
#endif
		// build RxCell
		pRxCell->pRxWI = pRxWI;
		pRxCell->pHeader = pHeader;
		pRxCell->pRxPacket = pRxPacket;
		pRxCell->pData = (UCHAR *) pHeader;
		pRxCell->DataSize = pRxWI->MPDUtotalByteCount;
		pRxCell->Flags = 0;

		// Increase Total receive byte counter after real data received no mater any error or not
		pAd->RalinkCounters.ReceivedByteCount += pRxWI->MPDUtotalByteCount;
		pAd->RalinkCounters.OneSecReceivedByteCount += pRxWI->MPDUtotalByteCount;
		pAd->RalinkCounters.RxCount ++;
		pAd->RalinkCounters.OneSecRxCount ++;

#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.ReceivedFragmentCount);
#endif // STATS_COUNT_SUPPORT //


#ifdef WDS_SUPPORT
		if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1))
		{
			if (MAC_ADDR_EQUAL(pHeader->Addr1, pAd->CurrentAddress))
				pEntry = FindWdsEntry(pAd, pRxWI->WirelessCliID, pHeader->Addr2, pRxWI->PHYMODE);

#ifdef STATS_COUNT_SUPPORT
			if(pEntry)
			{
				pAd->WdsTab.WdsEntry[pEntry->MatchWDSTabIdx].WdsCounter.ReceivedByteCount += pRxWI->MPDUtotalByteCount;
				INC_COUNTER64(pAd->WdsTab.WdsEntry[pEntry->MatchWDSTabIdx].WdsCounter.ReceivedFragmentCount);

				if(IS_MULTICAST_MAC_ADDR(pHeader->Addr3))
					INC_COUNTER64(pAd->WdsTab.WdsEntry[pEntry->MatchWDSTabIdx].WdsCounter.MulticastReceivedFrameCount);

			}
#endif // STATS_COUNT_SUPPORT //
		}
#endif

#ifdef RALINK_ATE
		if (ATE_ON(pAd))
		{
			pAd->ate.RxCntPerSec++;
			ATESampleRssi(pAd, pRxWI);
#ifdef RALINK_QA
			if (pAd->ate.bQARxStart == TRUE)
			{
				/* GetPacketFromRxRing() has copy the endian-changed RxD if it is necessary. */
				ATE_QA_Statistics(pAd, pRxWI, pRxD,	pHeader);
			}

#endif // RALINK_QA //
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		}
#endif // RALINK_ATE //
		
		// Check for all RxD errors
		if (APCheckRxError(pAd, pRxD, pRxWI->WirelessCliID) != NDIS_STATUS_SUCCESS)
		{
			APRxDErrorHandle(pAd, &RxCell);

			// Increase received error packet counter per BSS
			if (pHeader->FC.FrDs == 0 &&
				pRxD->U2M &&
				pRxWI->BSSID < pAd->ApCfg.BssidNum)
			{
				pMbss = &pAd->ApCfg.MBSSID[pRxWI->BSSID];
				pMbss->RxDropCount ++;
				pMbss->RxErrorCount ++;
			}

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
			if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1))
			{
				if (MAC_ADDR_EQUAL(pHeader->Addr1, pAd->CurrentAddress))
					pEntry = FindWdsEntry(pAd, pRxWI->WirelessCliID, pHeader->Addr2, pRxWI->PHYMODE);			
				if(pEntry)
					pAd->WdsTab.WdsEntry[pEntry->MatchWDSTabIdx].WdsCounter.RxErrors++;
			}
#endif // STATS_COUNT_SUPPORT //
#endif // WDS_SUPPORT //
			// discard this frame
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		// All frames to AP are directed except probe_req. IEEE 802.11/1999 - p.463
		// Do this before checking "duplicate frame".
		// 2003-08-20 accept BEACON to decide if OLBC (Overlapping Legacy BSS Condition) happens
		// TODO: consider move this code to be inside "APCheckRxError()"
		switch (pHeader->FC.Type)
		{
			// CASE I, receive a DATA frame
			case BTYPE_DATA:
				{
					if (pRxD->U2M)
					{
						Update_Rssi_Sample(pAd, &pAd->ApCfg.RssiSample, pRxWI);
						pAd->ApCfg.NumOfAvgRssiSample ++;
#ifdef DBG_DIAGNOSE
//						if (pRxWI->MCS < 16)
						if (pRxWI->MCS < 24) // 3*3
						{	// The RxMCS must be smaller than 16.
							pAd->DiagStruct.RxDataCnt[pAd->DiagStruct.ArrayCurIdx]++;
							pAd->DiagStruct.RxMcsCnt[pAd->DiagStruct.ArrayCurIdx][pRxWI->MCS]++;
						}
#endif // DBG_DIAGNOSE //
					}
					// process DATA frame
					APHandleRxDataFrame(pAd, &RxCell);
				}
				break;
			// CASE II, receive a MGMT frame
			case BTYPE_MGMT:
				{
					APHandleRxMgmtFrame(pAd, &RxCell);
				}
				break;
			// CASE III. receive a CNTL frame
			case BTYPE_CNTL:
				{
					APHandleRxControlFrame(pAd, &RxCell);
				}
				break;
			// discard other type
			default:
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				break;
		}
	}

#ifdef UAPSD_AP_SUPPORT
	/* dont remove the function or UAPSD will fail */
	UAPSD_MR_SP_RESUME(pAd);
    UAPSD_SP_CloseInRVDone(pAd);
#endif // UAPSD_AP_SUPPORT //

	return bReschedule;
}


BOOLEAN APFowardWirelessStaToWirelessSta(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	ULONG			FromWhichBSSID)
{
    MAC_TABLE_ENTRY	*pEntry = NULL;
    BOOLEAN			bAnnounce, bDirectForward;
	UCHAR			*pHeader802_3;
	PNDIS_PACKET	pForwardPacket;

#ifdef INF_AMAZON_SE
	/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
	RTMP_SET_PACKET_NOBULKOUT(pPacket, FALSE);
#endif // INF_AMAZON_SE //

#ifdef APCLI_SUPPORT
	// have no need to forwad the packet to WM
	if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_APCLI)
	{
		// need annouce to upper layer
		return TRUE;
	}
	else
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
	// have no need to forwad the packet to WM
	if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS)
	{
		// need annouce to upper layer
		return TRUE;
	}
#endif // WDS_SUPPORT //

	pEntry = NULL;
	bAnnounce = TRUE;
	bDirectForward = FALSE;

	pHeader802_3 = GET_OS_PKT_DATAPTR(pPacket);

	if (pHeader802_3[0] & 0x01) 
	{
		/*
		** In the case, the BSS have only one STA behind.
		** AP have no necessary to forward the M/Bcase packet back to STA again.
		*/
		if (pAd->ApCfg.MBSSID[FromWhichBSSID].StaCount > 1)
			bDirectForward  = TRUE;

		/* tell caller to deliver the packet to upper layer */
		bAnnounce = TRUE;
	}		
	else
	{
		// if destinated STA is a associated wireless STA
		pEntry = MacTableLookup(pAd, pHeader802_3);

		if (pEntry && pEntry->Sst == SST_ASSOC)
		{
			bDirectForward = TRUE;
			bAnnounce = FALSE;

			if (FromWhichBSSID == pEntry->apidx)
			{// STAs in same SSID
				if ((pAd->ApCfg.MBSSID[pEntry->apidx].IsolateInterStaTraffic == 1))
				{
					// release the packet
					bDirectForward = FALSE;
					bAnnounce = FALSE;
				}
			}
			else
			{// STAs in different SSID
				if (pAd->ApCfg.IsolateInterStaTrafficBTNBSSID == 1 ||
					(pAd->ApCfg.MBSSID[pEntry->apidx].VLAN_VID != pAd->ApCfg.MBSSID[FromWhichBSSID].VLAN_VID))
				{
					bDirectForward = FALSE;
					bAnnounce = FALSE;
				}
			}
		}
		else
		{
			// announce this packet to upper layer (bridge)
			bDirectForward = FALSE;
			bAnnounce = TRUE;
		}
	}

	if (bDirectForward)
	{
		// build an NDIS packet
		pForwardPacket = DuplicatePacket(pAd, pPacket, FromWhichBSSID);

		if (pForwardPacket == NULL)
		{
			return bAnnounce;
		}

		{
			// 1.1 apidx != 0, then we need set packet mbssid attribute.
			RTMP_SET_PACKET_NET_DEVICE_MBSSID(pForwardPacket, MAIN_MBSSID);	// set a default value
			if(pEntry && (pEntry->apidx != 0))
				RTMP_SET_PACKET_NET_DEVICE_MBSSID(pForwardPacket, pEntry->apidx);

			/* send bc/mc frame back to the same bss */
			if (!pEntry)
				RTMP_SET_PACKET_NET_DEVICE_MBSSID(pForwardPacket, FromWhichBSSID);

			RTMP_SET_PACKET_WCID(pForwardPacket, pEntry ? pEntry->Aid : MCAST_WCID);
			RTMP_SET_PACKET_MOREDATA(pForwardPacket, FALSE);

#ifdef INF_AMAZON_SE
			/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
			RTMP_SET_PACKET_NOBULKOUT(pForwardPacket, TRUE);
#endif // INF_AMAZON_SE //

			APSendPacket(pAd, pForwardPacket);
		}
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS); 	// Dequeue outgoing frames from TxSwQueue0..3 queue and process it
	}
	
	return bAnnounce;
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
		NDIS_STATUS_SUCCESS:If succes to queue the packet into TxSwQueue.
		NDIS_STATUS_FAILURE: If failed to do en-queue.
========================================================================
*/
NDIS_STATUS APInsertPsQueue(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR QueIdx)
{
	ULONG IrqFlags;
#ifdef UAPSD_AP_SUPPORT
	/* put the U-APSD packet to its U-APSD queue by AC ID */
	UINT32 ac_id = QueIdx - QID_AC_BE; /* should be >= 0 */


	if (UAPSD_MR_IS_UAPSD_AC(pMacEntry, ac_id))
		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, ac_id);
	else
#endif // UAPSD_AP_SUPPORT //
	{
		if (pMacEntry->PsQueue.Number >= MAX_PACKETS_IN_PS_QUEUE)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);			
			return NDIS_STATUS_FAILURE;			
		}
		else
		{
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			InsertTailQueue(&pMacEntry->PsQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
		}
	}

	// mark corresponding TIM bit in outgoing BEACON frame
#ifdef UAPSD_AP_SUPPORT
	if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(pMacEntry, QueIdx))
	{
		/* 1. the station is UAPSD station;
		2. one of AC is non-UAPSD (legacy) AC;
		3. the destinated AC of the packet is UAPSD AC. */
		/* So we can not set TIM bit due to one of AC is legacy AC */
	}
	else
#endif // UAPSD_AP_SUPPORT //
	{
		WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->apidx, pMacEntry->Aid);
	}
	return NDIS_STATUS_SUCCESS;
}

