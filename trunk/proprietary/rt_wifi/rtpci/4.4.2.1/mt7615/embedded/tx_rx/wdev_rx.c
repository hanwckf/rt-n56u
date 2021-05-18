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

/**
 * @addtogroup wifi_dev_system
 * @{
 * @name rx core API
 * @{
 */

#include "rt_config.h"

#ifdef CONFIG_HOTSPOT
extern BOOLEAN hotspot_rx_handler(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk);
#endif /* CONFIG_HOTSPOT */


#ifdef AIR_MONITOR
extern VOID Air_Monitor_Pkt_Report_Action(PRTMP_ADAPTER pAd, UCHAR wcid, RX_BLK *pRxBlk);
#endif /* AIR_MONITOR */


// TODO: shiang-usw, temporary put this function here, should remove to other place or re-write!
VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN struct rx_signal_info *signal,
	IN UCHAR phy_mode,
	IN UCHAR bw)
{
	BOOLEAN bInitial = FALSE;
	INT ant_idx, ant_max = 4;

	if (!(pRssi->AvgRssi[0] | pRssi->AvgRssiX8[0] | pRssi->LastRssi[0]))
		bInitial = TRUE;

	// TODO: shiang-usw, shall we check this here to reduce the for loop count?
	if (ant_max > pAd->Antenna.field.RxPath)
		ant_max = pAd->Antenna.field.RxPath;

	for (ant_idx = 0; ant_idx < 4; ant_idx++)
	{
		if (signal->raw_snr[ant_idx] != 0 && phy_mode != MODE_CCK)
		{
			pRssi->LastSnr[ant_idx] = ConvertToSnr(pAd, signal->raw_snr[ant_idx]);
			if (bInitial)
			{
				pRssi->AvgSnrX8[ant_idx] = pRssi->LastSnr[ant_idx] << 3;
				pRssi->AvgSnr[ant_idx] = pRssi->LastSnr[ant_idx];
			}
			else
				pRssi->AvgSnrX8[ant_idx] = (pRssi->AvgSnrX8[ant_idx] - pRssi->AvgSnr[ant_idx]) + pRssi->LastSnr[ant_idx];

			pRssi->AvgSnr[ant_idx] = pRssi->AvgSnrX8[ant_idx] >> 3;
		}

		if (signal->raw_rssi[ant_idx] != 0)
		{
			pRssi->LastRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&signal->raw_rssi[0]), ant_idx);

			if (bInitial)
			{
				pRssi->AvgRssiX8[ant_idx] = pRssi->LastRssi[ant_idx] << 3;
				pRssi->AvgRssi[ant_idx] = pRssi->LastRssi[ant_idx];
			}
			else
				pRssi->AvgRssiX8[ant_idx] = (pRssi->AvgRssiX8[ant_idx] - pRssi->AvgRssi[ant_idx]) + pRssi->LastRssi[ant_idx];

			pRssi->AvgRssi[ant_idx] = pRssi->AvgRssiX8[ant_idx] >> 3;
		}
	}
}


#ifdef DOT11_N_SUPPORT
UINT deaggregate_AMSDU_announce(
	IN RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	IN UCHAR *pData,
	IN ULONG DataSize,
	IN UCHAR OpMode)
{
	USHORT PayloadSize;
	USHORT SubFrameSize;
	HEADER_802_3 *pAMSDUsubheader;
	UINT nMSDU;
	UCHAR Header802_3[14];
	UCHAR *pPayload, *pDA, *pSA, *pRemovedLLCSNAP;
	PNDIS_PACKET pClonePacket;
	struct wifi_dev *wdev;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	UCHAR VLAN_Size;
#ifdef CONFIG_AP_SUPPORT
	USHORT VLAN_VID = 0;
	USHORT VLAN_Priority = 0;
#endif

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():invalud wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}

	wdev = pAd->wdev_list[wdev_idx];

	/* only MBssid support VLAN.*/
	VLAN_Size = (wdev->VLAN_VID != 0) ? LENGTH_802_1Q : 0;
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
			break;

		/*MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%d subframe: Size = %d\n",  nMSDU, PayloadSize));*/
		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;

		/* convert to 802.3 header*/
		CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);


#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
		if (OpMode == OPMODE_AP)
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
		{
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

			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);

			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									Header802_3, LENGTH_802_3, pPayload,
									TPID);
		}
#endif /* CONFIG_AP_SUPPORT */

		pClonePacket = ClonePacket(wdev->if_dev, pPacket, pPayload, PayloadSize);
		if (pClonePacket)
		{
			UCHAR opmode = pAd->OpMode;

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			opmode = OpMode;
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
			Announce_or_Forward_802_3_Packet(pAd, pClonePacket, RTMP_GET_PACKET_WDEV(pPacket), opmode);
		}


		/* A-MSDU has padding to multiple of 4 including subframe header.*/
		/* align SubFrameSize up to multiple of 4*/
		SubFrameSize = (SubFrameSize+3)&(~0x3);


		if (SubFrameSize > 1528 || SubFrameSize < 32)
			break;

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


VOID Indicate_AMSDU_Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	UINT nMSDU;

	RTMP_UPDATE_OS_PACKET_INFO(pAd, pRxBlk, wdev_idx);
	RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		struct _RXD_BASE_STRUCT *rx_base;
		rx_base = (struct _RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		if ((rx_base->RxD1.HdrOffset == 1) && (rx_base->RxD1.PayloadFmt != 0) && (rx_base->RxD1.HdrTranslation == 0)) {
			pRxBlk->pData += 2;
			pRxBlk->DataSize -= 2;
		}
	}
#endif
	nMSDU = deaggregate_AMSDU_announce(pAd, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize, pRxBlk->OpMode);
}
#endif /* DOT11_N_SUPPORT */


VOID Announce_or_Forward_802_3_Packet(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN UCHAR wdev_idx,
	IN UCHAR op_mode)
{
	BOOLEAN to_os = FALSE;
	struct wifi_dev *wdev;


	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():invalid wdev_idx(%d)!\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
		return;
	}
	wdev = pAd->wdev_list[wdev_idx];
#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev))
	{
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
		if (wf_fwd_needed_hook != NULL && wf_fwd_needed_hook() == TRUE)
			set_wf_fwd_cb(pAd, pPacket, wdev);
#endif /* CONFIG_WIFI_PKT_FWD */	

		if((wdev->wdev_type == WDEV_TYPE_APCLI)
#ifdef EZ_API_SUPPORT	
#ifdef EZ_MOD_SUPPORT
			 && (wdev->ez_driver_params.ez_api_mode != CONNECTION_OFFLOAD) 
#else
			&& (wdev->ez_security.ez_api_mode != CONNECTION_OFFLOAD) 
#endif
#endif	 
		){
			if(ez_apcli_rx_grp_pkt_drop(wdev,pPacket)){
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				//EZ_DEBUG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Drop Pkt for NonEz Duplicate link check!\n", __FUNCTION__));
				return;
			}
		}
	}
#endif		
	if (wdev->rx_pkt_foward)
		to_os = wdev->rx_pkt_foward(pAd, wdev, pPacket);

	if (to_os == TRUE)
	{
#ifdef WH_EZ_SETUP
		if (!IS_EZ_SETUP_ENABLED(wdev))
#endif
		{
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
			if (wf_fwd_needed_hook != NULL && wf_fwd_needed_hook() == TRUE)
				set_wf_fwd_cb(pAd, pPacket, wdev);
#endif /* CONFIG_WIFI_PKT_FWD */	
		}
#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev))
			pAd->CurWdevIdx = wdev_idx;
#endif
/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
		pAd->HwnatCurWdevIdx = wdev_idx;
#endif
		announce_802_3_packet(pAd, pPacket,op_mode);
	}
#ifndef RTMP_UDMA_SUPPORT	
	else {
MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): No need to send to OS!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
	}
#endif	
}


VOID Indicate_802_3_Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
#if defined(CONFIG_AP_SUPPORT) && defined(MAC_REPEATER_SUPPORT)
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
#endif

	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */
		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR))
		{
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
			/*For UDP packet, we need to check about the DHCP packet. */
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
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE);
					if (pReptEntry)
					{
						ASSERT(pReptEntry->CliValid == TRUE);
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
					}
					bHdrChanged = TRUE;
				}
				if (bHdrChanged == TRUE) {
					NdisZeroMemory((pUdpHdr+6), 2); /*modify the UDP chksum as zero */
#ifdef DHCP_UC_SUPPORT
					*(UINT16*)(pUdpHdr+6) = RTMP_UDP_Checksum(pRxPacket);
#endif /* DHCP_UC_SUPPORT */
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);

	SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
	Announce_or_Forward_802_3_Packet(pAd, pRxBlk->pRxPacket, wdev_idx, pAd->OpMode);
}

/* Normal legacy Rx packet indication*/
VOID Indicate_Legacy_Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UINT max_pkt_len = MAX_RX_PKT_LEN;
	UCHAR *pData = pRxBlk->pData;
	INT data_len = pRxBlk->DataSize;
	struct wifi_dev *wdev;
	UCHAR opmode = pAd->OpMode;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():invalid wdev_idx(%d)!\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
	wdev = pAd->wdev_list[wdev_idx];

//+++Add by shiang for debug
if (0) {
#ifdef HDR_TRANS_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
	{
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}
#endif /* HDR_TRANS_SUPPORT */
	hex_dump("Indicate_Legacy_Packet", pData, data_len);
}
//---Add by shiang for debug

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
#ifdef HDR_TRANS_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
	{
		max_pkt_len = 1514;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}
	else
#endif /* HDR_TRANS_SUPPORT */

	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
	//hex_dump("802_3_hdr", (UCHAR *)Header802_3, LENGTH_802_3);

	pData = pRxBlk->pData;
	data_len = pRxBlk->DataSize;


	if (data_len > max_pkt_len)
	{
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():data_len(%d) > max_pkt_len(%d)!\n",
			__FUNCTION__, data_len, max_pkt_len));
		return;
	}

	STATS_INC_RX_PACKETS(pAd, wdev_idx);


#ifdef CONFIG_AP_SUPPORT
	WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);
#endif /* CONFIG_AP_SUPPORT */

//+++Add by shiang for debug
if (0) {
	hex_dump("Before80211_2_8023", pData, data_len);
	hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
}
//---Add by shiang for debug

#ifdef HDR_TRANS_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
	{
		struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxPacket);

		pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);
		pOSPkt->data = pRxBlk->pTransData;
		pOSPkt->len = pRxBlk->TransDataSize;
		SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		//printk("%s: rx trans ...%d\n", __FUNCTION__, __LINE__);
	}
	else
#endif /* HDR_TRANS_SUPPORT */
	{
		RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, wdev_idx, TPID);
	}

	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */
		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR))
		{
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

			/*For UDP packet, we need to check about the DHCP packet. */
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
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE);
					if (pReptEntry)
					{
						ASSERT(pReptEntry->CliValid == TRUE);
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
					}
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
					else
					{	
						VOID *opp_band_tbl = NULL;
						VOID *band_tbl = NULL;
						VOID *other_band_tbl = NULL;

						if (wf_fwd_feedback_map_table)
							wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl, &other_band_tbl);

						if (opp_band_tbl != NULL) {
							/* 
								check the ReptTable of the opposite band due to dhcp packet (BC)
									may come-in 2/5G band when STA send dhcp broadcast to Root AP 
							*/
							pReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pCliHwAddr, FALSE);
							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						}
						else
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("cannot find the adapter of the oppsite band\n"));
						
						if (other_band_tbl != NULL) {

							pReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pCliHwAddr, FALSE);
							
							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						}
						else
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("cannot find the adapter of the othersite band\n"));
					}
#endif /* CONFIG_WIFI_PKT_FWD */
					bHdrChanged = TRUE;
				}

				if (bHdrChanged == TRUE)
					NdisZeroMemory((pUdpHdr+6), 2); /*modify the UDP chksum as zero */
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


//+++Add by shiang for debug
if (0) {
	hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxPacket), GET_OS_PKT_LEN(pRxPacket));
}
#ifdef RTMP_UDMA_SUPPORT
/* If received packet is untagged, but the corresponding 'wdev' has a valid VLANID, 
  	 * then insert 4-byte VLAN header with that VLANID.
		 	 */
	if(!(((GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket))[12] == 0X81) && ((GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)[13] = 0x00))))
	{
		USHORT vlanid, VLAN_Priority = 0;
		struct wifi_dev *wdev = pAd->wdev_list[wdev_idx];
		vlanid = wdev->VLAN_VID;
		VLAN_Priority = wdev->VLAN_Priority;
		if(vlanid)
		{
			UCHAR VLAN_Size = LENGTH_802_1Q;
			UCHAR *Header802_3 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
			UCHAR *data_p = OS_PKT_HEAD_BUF_EXTEND(pRxBlk->pRxPacket, VLAN_Size);
			RT_VLAN_8023_HEADER_COPY(pAd, vlanid, VLAN_Priority,
			Header802_3, LENGTH_802_3,data_p, TPID);
		}
	}	
#endif	/* RTMP_UDMA_SUPPORT */	
//---Add by shiang for debug
	Announce_or_Forward_802_3_Packet(pAd, pRxPacket, wdev->wdev_idx, opmode);
}

/* Ralink Aggregation frame */
VOID Indicate_ARalink_Packet(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR Header802_3[LENGTH_802_3];
	UINT16 Msdu2Size;
	UINT16 Payload1Size, Payload2Size;
	PUCHAR pData2;
	PNDIS_PACKET pPacket2 = NULL;
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UCHAR opmode = pAd->OpMode;
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
	wdev = pAd->wdev_list[wdev_idx];

	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData+1) << 8);
	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize))
	{
		/* skip two byte MSDU2 len */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}
	else
	{
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* get 802.3 Header and  remove LLC*/
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);

	ASSERT(pRxBlk->pRxPacket);
	if (!pRxBlk->pRxPacket)
	{
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}


	pAd->RalinkCounters.OneSecRxARalinkCnt++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;

	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;
	pPacket2 = duplicate_pkt_vlan(wdev->if_dev,
							wdev->VLAN_VID, wdev->VLAN_Priority,
							(pData2-LENGTH_802_3), LENGTH_802_3,
							pData2, Payload2Size, TPID);

	if (!pPacket2)
	{
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* update payload size of 1st packet*/
	pRxBlk->DataSize = Payload1Size;
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, wdev_idx, TPID);

	Announce_or_Forward_802_3_Packet(pAd, pRxBlk->pRxPacket, wdev_idx, opmode);
	if (pPacket2)
		Announce_or_Forward_802_3_Packet(pAd, pPacket2, wdev_idx, opmode);
}


#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
	}


VOID RTMPDeFragmentDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	PNDIS_PACKET pRetPacket = NULL;
	UCHAR *pFragBuffer = NULL;
	BOOLEAN bReassDone = FALSE;
	UCHAR HeaderRoom = 0;
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	UINT8 RXWISize = pAd->chipCap.RXWISize;

	// TODO: shiang-MT7603, fix me for this function work in MT series chips
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		RXWISize = 0;
#endif /* MT_MAC */

	HeaderRoom = pData - pRxBlk->FC;

	/* Re-assemble the fragmented packets*/
	if (pRxBlk->FN == 0)
	{	/* Frag. Number is 0 : First frag or only one pkt*/
		/* the first pkt of fragment, record it.*/
		if (FC->MoreFrag && pAd->FragFrame.pFragPacket)
		{
			pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items.
			    Copy RxWI content to pFragBuffer.
			*/
			//pAd->FragFrame.RxSize = DataSize + HeaderRoom;
			//NdisMoveMemory(pFragBuffer, pHeader, pAd->FragFrame.RxSize);
#ifdef HDR_TRANS_RX_SUPPORT
			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			{
				pAd->FragFrame.RxSize = DataSize + RXWISize;
				NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
				NdisMoveMemory(pFragBuffer + RXWISize, pData, pAd->FragFrame.RxSize);
			}
			else
#endif /* HDR_TRANS_RX_SUPPORT */
			{
				pAd->FragFrame.RxSize = DataSize + HeaderRoom + RXWISize;
				NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
				NdisMoveMemory(pFragBuffer + RXWISize, FC, pAd->FragFrame.RxSize - RXWISize);
			}

			pAd->FragFrame.Sequence = pRxBlk->SN;
			pAd->FragFrame.LastFrag = pRxBlk->FN;	   /* Should be 0*/

#ifdef HDR_TRANS_RX_SUPPORT
			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
				pAd->FragFrame.Header_802_3 = TRUE;
			else
#endif /* HDR_TRANS_RX_SUPPORT */
				pAd->FragFrame.Header_802_3 = FALSE;

			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	/* end of processing this frame*/
		}
		else if (!pAd->FragFrame.pFragPacket)
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ERR: pAd->FragFrame.pFragPacket is NULL.\n"));
		}
	}
	else
	{	/*Middle & End of fragment*/
		if ((pRxBlk->SN != pAd->FragFrame.Sequence) ||
			(pRxBlk->FN != (pAd->FragFrame.LastFrag + 1)))
		{
			/* Fragment is not the same sequence or out of fragment number order*/
			/* Reset Fragment control blk*/
			if (pRxBlk->SN != pAd->FragFrame.Sequence)
				RESET_FRAGFRAME(pAd->FragFrame);
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fragment is not the same sequence or out of fragment number order.\n"));
			goto done;
		}
		/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
		//else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE + RXWISize)
		{
			/* Fragment frame is too large, it exeeds the maximum frame size.*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fragment frame is too large, it exeeds the maximum frame size.\n"));
			goto done;
		}


		/* Broadcom AP(BCM94704AGR) will send out LLC in fragment's packet, LLC only can accpet at first fragment.*/
		/* In this case, we will drop it.*/
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H)))
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pRxBlk->SN, pRxBlk->FN));
			goto done;
		}

		pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);

		/* concatenate this fragment into the re-assembly buffer*/
		NdisMoveMemory((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pRxBlk->FN;	   /* Update fragment number*/

		/* Last fragment*/
		if (FC->MoreFrag == FALSE)
			bReassDone = TRUE;
	}

done:
	/* always release rx fragmented packet*/
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	pRxBlk->pRxPacket = NULL;

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
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
			//pRxBlk->pHeader = (PHEADER_802_11) GET_OS_PKT_DATAPTR(pRetPacket);
			//pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom;
			//pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom;
			//pRxBlk->pRxPacket = pRetPacket;
			pRxBlk->pRxWI = (RXWI_STRUC *) GET_OS_PKT_DATAPTR(pRetPacket);
#ifdef HDR_TRANS_RX_SUPPORT
			if (pAd->FragFrame.Header_802_3)
			{
				pRxBlk->pData =	GET_OS_PKT_DATAPTR(pRetPacket) + RXWISize;
				pRxBlk->DataSize = pAd->FragFrame.RxSize - RXWISize;
			}
			else
#endif /* HDR_TRANS_RX_SUPPORT */
			{
				pRxBlk->pData =	GET_OS_PKT_DATAPTR(pRetPacket) + HeaderRoom + RXWISize;
				pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom - RXWISize;
			}

			pRxBlk->pRxPacket = pRetPacket;

#ifdef HDR_TRANS_RX_SUPPORT
			if (pAd->FragFrame.Header_802_3)
			{
				struct sk_buff *pOSPkt;

				pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
				pOSPkt->data = pRxBlk->pData;
				pOSPkt->len = pRxBlk->DataSize;

				RX_BLK_SET_FLAG(pRxBlk, fRX_HDR_TRANS);
				pAd->FragFrame.Header_802_3 = FALSE;
			}
#endif /* HDR_TRANS_RX_SUPPORT */

			pAd->FragFrame.pFragPacket = pNewFragPacket;
		}
		else
		{
			RESET_FRAGFRAME(pAd->FragFrame);
		}
	}
}


VOID rx_eapol_frm_handle(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR *pTmpBuf;
	BOOLEAN to_mlme = TRUE, to_daemon = FALSE;
	struct wifi_dev *wdev;


	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX)
		goto done;

	wdev = pAd->wdev_list[wdev_idx];

	if(pRxBlk->DataSize < (LENGTH_802_1_H + LENGTH_EAPOL_H))
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pkts size too small\n"));
		goto done;
	}
	else if (!RTMPEqualMemory(SNAP_802_1H, pRxBlk->pData, 6))
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no SNAP_802_1H parameter\n"));
		goto done;
	}
	else if (!RTMPEqualMemory(EAPOL, pRxBlk->pData+6, 2))
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("no EAPOL parameter\n"));
		goto done;
	}
	else if(*(pRxBlk->pData+9) > EAPOLASFAlert)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown EAP type(%d)\n", *(pRxBlk->pData+9)));
		goto done;
	}

#ifdef CONFIG_AP_SUPPORT
	if (pEntry && IS_ENTRY_CLIENT(pEntry))
	{

#ifdef HOSTAPD_SUPPORT
		if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Hostapd == Hostapd_EXT)
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Indicate_Legacy_Packet\n"));
			Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
			return;
		}
#endif/*HOSTAPD_SUPPORT*/
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef RT_CFG80211_SUPPORT
	if (pEntry)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG80211 EAPOL Indicate_Legacy_Packet\n"));

		{
#if defined(CONFIG_AP_SUPPORT) && defined(APCLI_SUPPORT)
		if(!(IS_ENTRY_APCLI(pEntry)))
#endif
			{
				Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
				return;
			}
		}
	}
#endif /*RT_CFG80211_SUPPORT*/

	if (pEntry && IS_ENTRY_AP(pEntry))
	{
		{
			to_mlme = TRUE;
			to_daemon = FALSE;
		}
	}

#ifdef CONFIG_AP_SUPPORT
	if (pEntry && IS_ENTRY_CLIENT(pEntry))
	{
#ifdef DOT1X_SUPPORT
		/* sent this frame to upper layer TCPIP */
		if ((pEntry->SecConfig.Handshake.WpaState < AS_INITPMK) &&
			(IS_AKM_WPA1(pEntry->SecConfig.AKMMap) ||
			(IS_AKM_WPA2(pEntry->SecConfig.AKMMap) && (pEntry->PMKID_CacheIdx == ENTRY_NOT_FOUND)) ||
			IS_IEEE8021X(&pEntry->SecConfig)))
		{
#ifdef WSC_AP_SUPPORT
			/* report EAP packets to MLME to check this packet is WPS packet or not */
			if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.WscConfMode != WSC_DISABLE) &&
				(!MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.EntryAddr, ZERO_MAC_ADDR)))
			{
				to_mlme = TRUE;
				pTmpBuf = pRxBlk->pData - LENGTH_802_11;
				// TODO: shiang-usw, why we need to change pHeader here??
				pRxBlk->FC = pTmpBuf;
			}
#endif /* WSC_AP_SUPPORT */


			to_daemon = TRUE;
			to_mlme = FALSE;
		}
		else
#endif /* DOT1X_SUPPORT */
		{
			/* sent this frame to WPA state machine */

			/*
				Check Addr3 (DA) is AP or not.
				If Addr3 is AP, forward this EAP packets to MLME
				If Addr3 is NOT AP, forward this EAP packets to upper layer or STA.
			*/
			if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_GO) {
				ASSERT(wdev->func_idx < HW_BEACON_MAX_NUM);
				if (wdev->func_idx < HW_BEACON_MAX_NUM) {
					ASSERT(wdev == (&pAd->ApCfg.MBSSID[wdev->func_idx].wdev));
				}
			}

			// TODO: shiang-usw, why we check this here??
			if ((wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_GO) &&
				(NdisEqualMemory(pRxBlk->Addr3, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid, MAC_ADDR_LEN) == FALSE))
				to_daemon = TRUE;
			else
				to_mlme = TRUE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	/*
	   Special DATA frame that has to pass to MLME
	   1. Cisco Aironet frames for CCX2. We need pass it to MLME for special process
	   2. EAPOL handshaking frames when driver supplicant enabled, pass to MLME for special process
	 */
	if (to_mlme)
	{
		pTmpBuf = pRxBlk->pData - LENGTH_802_11;
		NdisMoveMemory(pTmpBuf, pRxBlk->FC, LENGTH_802_11);

		{
			RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;
			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
										pTmpBuf,
										pRxBlk->DataSize + LENGTH_802_11,
										pRxBlk->rx_signal.raw_rssi[0],
										pRxBlk->rx_signal.raw_rssi[1],
										pRxBlk->rx_signal.raw_rssi[2],
										pRxBlk->rx_signal.raw_rssi[3],
										0,
										(rxd_base != NULL) ? rxd_base->RxD1.ChFreq : 0,
										pRxBlk->OpMode,
										wdev,
										pRxBlk->rx_rate.field.MODE);
		}


		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			     ("!!! report EAPOL DATA to MLME (len=%d) !!!\n",
			      pRxBlk->DataSize));
	}

	if (to_daemon == TRUE)
	{
		Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
		return;
	}

done:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}


VOID Indicate_EAPOL_Packet(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

    if (pAd == NULL)
    {
        MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Indicate_EAPOL_Packet: invalid pAd.\n"));
        RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
        return;
    }
        
    if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Indicate_EAPOL_Packet: invalid wcid.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	if (pEntry == NULL)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Indicate_EAPOL_Packet: drop and release the invalid packet.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
	return;
}


// TODO: shiang-usw, modify the op_mode assignment for this function!!!
VOID dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT op_mode = pRxBlk->OpMode;



MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));

#ifdef RT_CFG80211_SUPPORT
#ifdef CFG_TDLS_SUPPORT
	if (CFG80211_HandleTdlsDiscoverRespFrame(pAd, pRxBlk, op_mode))
		goto done;
#endif /* CFG_TDLS_SUPPORT */

	if (CFG80211_HandleP2pMgmtFrame(pAd, pRxBlk, op_mode))
		goto done;
#endif /* RT_CFG80211_SUPPORT */


#ifdef DOT11W_PMF_SUPPORT
	if (PMF_PerformRxFrameAction(pAd, pRxBlk) == FALSE)
		goto done;
#endif /* DOT11W_PMF_SUPPORT */

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN)) {
#ifdef CONFIG_AP_SUPPORT	
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
#endif
			if (pEntry)
				pRxBlk->wcid = pEntry->wcid;
		}
	} else {
#ifdef CONFIG_AP_SUPPORT	
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
#endif
		if (pEntry)
			pRxBlk->wcid = pEntry->wcid;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;

		op_mode = OPMODE_AP;
#ifdef IDS_SUPPORT
		/*
			Check if a rogue AP impersonats our mgmt frame to spoof clients
			drop it if it's a spoofed frame
		*/
		if (RTMPSpoofedMgmtDetection(pAd, pRxBlk))
			goto done;

		/* update sta statistics for traffic flooding detection later */
		RTMPUpdateStaMgmtCounter(pAd, FC->SubType);
#endif /* IDS_SUPPORT */

		if (!pRxInfo->U2M)
		{
			if ((FC->SubType != SUBTYPE_BEACON) && (FC->SubType != SUBTYPE_PROBE_REQ))
			{
				BOOLEAN bDrop = TRUE;		
#ifdef DOT11W_PMF_SUPPORT
/* For PMF TEST Plan 5.4.3.1 & 5.4.3.2 */
#ifdef APCLI_SUPPORT
				if (pEntry && ((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH)))
				{
					if (IS_ENTRY_APCLI(pEntry))
					{
						bDrop = FALSE;
					}
						
				}
#endif /* APCLI_SUPPORT */
#endif /* DOT11W_PMF_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
				if  (pAd->bApCliCertTest == TRUE)
				{
					if ((FC->SubType == SUBTYPE_ACTION) && (pEntry) && IS_ENTRY_APCLI(pEntry))
						bDrop = FALSE;			
				}
#endif /* APCLI_CERT_SUPPOR */
#endif /* APCLI_SUPPORT */						

#if  defined(FTM_SUPPORT) || defined(CONFIG_HOTSPOT)
				if (IsPublicActionFrame(pAd, (VOID *)FC))
				{
					bDrop = FALSE;					
				}
#endif /* defined(FTM_SUPPORT) || defined(CONFIG_HOTSPOT) */

				if (bDrop == TRUE)
					goto done;

			}
		}

		/* BssInfo not ready, drop frame */
		if ((pEntry) && ((FC->SubType == SUBTYPE_AUTH) || (FC->SubType == SUBTYPE_ASSOC_REQ) ||
			(FC->SubType == SUBTYPE_DEAUTH) || (FC->SubType == SUBTYPE_DISASSOC)))
		{
			if (WDEV_BSS_STATE(pEntry->wdev) != BSS_READY)
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: BSS idx (%d) nor ready. (subtype %d) \n", 
					pEntry->wdev->bss_info_argument.ucBssIndex, FC->SubType));
				goto done;
			}
		}

		/* Software decrypt WEP data during shared WEP negotiation */
		if ((FC->SubType == SUBTYPE_AUTH) &&
			(FC->Wep == 1) && (pRxInfo->Decrypted == 0))
		{
			UCHAR *pMgmt = (PUCHAR)FC;
			UINT16 mgmt_len = pRxBlk->MPDUtotalByteCnt;
			UCHAR DefaultKeyId;

			if (!pEntry)
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: SW decrypt WEP data fails - the Entry is empty.\n"));
				goto done;
			}

			/* Skip 802.11 header */
			pMgmt += LENGTH_802_11;
			mgmt_len -= LENGTH_802_11;

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				DefaultKeyId = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PairwiseKeyId;
			}
#endif /*  CONFIG_AP_SUPPORT */


			/* handle WEP decryption */
			if (RTMPSoftDecryptWEP(
					&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.WepKey[DefaultKeyId],
					pMgmt,
					&mgmt_len) == FALSE)
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: SW decrypt WEP data fails.\n"));
				goto done;
			}

#ifdef RT_BIG_ENDIAN
			/* swap 16 bit fields - Auth Alg No. field */
			*(USHORT *)pMgmt = SWAP16(*(USHORT *)pMgmt);

			/* swap 16 bit fields - Auth Seq No. field */
			*(USHORT *)(pMgmt + 2) = SWAP16(*(USHORT *)(pMgmt + 2));

			/* swap 16 bit fields - Status Code field */
			*(USHORT *)(pMgmt + 4) = SWAP16(*(USHORT *)(pMgmt + 4));
#endif /* RT_BIG_ENDIAN */

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Decrypt AUTH seq#3 successfully\n"));

			/* Update the total length */
			pRxBlk->DataSize -= (LEN_WEP_IV_HDR + LEN_ICV);
		}
	}
#endif /* CONFIG_AP_SUPPORT */



	if (pRxBlk->DataSize > MAX_RX_PKT_LEN) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DataSize=%d\n", pRxBlk->DataSize));
		hex_dump("MGMT ???", pRxBlk->FC, pRxBlk->pData - pRxBlk->FC);
		goto done;
	}

#if defined(CONFIG_AP_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	if (pEntry && (FC->SubType == SUBTYPE_ACTION))
	{
		/* only PM bit of ACTION frame can be set */
		if (((op_mode == OPMODE_AP) && IS_ENTRY_CLIENT(pEntry)) ||
			((op_mode == OPMODE_STA) && (IS_ENTRY_TDLS(pEntry))))
		   	RtmpPsIndicate(pAd, pRxBlk->Addr2, pRxBlk->wcid, FC->PwrMgmt);

		/*
			In IEEE802.11, 11.2.1.1 STA Power Management modes,
			The Power Managment bit shall not be set in any management
			frame, except an Action frame.

			In IEEE802.11e, 11.2.1.4 Power management with APSD,
			If there is no unscheduled SP in progress, the unscheduled SP
			begins when the QAP receives a trigger frame from a non-AP QSTA,
			which is a QoS data or QoS Null frame associated with an AC the
			STA has configured to be trigger-enabled.
			So a management action frame is not trigger frame.
		*/
	}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef CONFIG_AP_SUPPORT
	/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
{
	RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;
	REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						  FC,
						  pRxBlk->DataSize,
						  pRxBlk->rx_signal.raw_rssi[0],
						  pRxBlk->rx_signal.raw_rssi[1],
						  pRxBlk->rx_signal.raw_rssi[2],
						  pRxBlk->rx_signal.raw_rssi[3],
						  min(pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1]),
						  (rxd_base != NULL) ? rxd_base->RxD1.ChFreq : 0,
						  op_mode,
						  pAd->wdev_list[0],
						  pRxBlk->rx_rate.field.MODE);
}
#endif



#ifdef TXBF_SUPPORT
#ifndef MT_MAC
	if (pAd->chipCap.FlgHwTxBfCap)
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
		if (FC->Order)
		{
			handleHtcField(pAd, pRxBlk);
			pRxBlk->pData += 4;
			pRxBlk->DataSize -= 4;
		}

		/* Check for compressed or non-compressed Sounding Response */
		if (((FC->SubType == SUBTYPE_ACTION) || (FC->SubType == SUBTYPE_ACTION_NO_ACK))
			&& (pRxBlk->pData[0] == CATEGORY_HT)
			&& ((pRxBlk->pData[1] == MIMO_N_BEACONFORM) || (pRxBlk ->pData[1] == MIMO_BEACONFORM))
		)
		{
			handleBfFb(pAd, pRxBlk);
		}
	}
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

done:

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s()\n", __FUNCTION__));
	if(pRxPacket)
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


VOID dev_rx_ctrl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;

	switch (FC->SubType)
	{
#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK_REQ:
			{
				FRAME_BA_REQ *bar = (FRAME_BA_REQ *)FC;

#ifdef MT_MAC
				if ((pAd->chipCap.hif_type == HIF_MT) &&
				    (pRxBlk->wcid == RESERVED_WCID))
				{
#ifdef MAC_REPEATER_SUPPORT
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif
#ifdef CONFIG_AP_SUPPORT			
					MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, &pRxBlk->Addr2[0]);
#endif
					if (pEntry)
					{
						pRxBlk->wcid = pEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT
						if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_APCLI(pEntry)))
						{
							pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);
							if (pReptEntry && (pReptEntry->CliValid == TRUE))
							{
								pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
								pRxBlk->wcid = pEntry->wcid;
							} else if ((pReptEntry == NULL) && IS_ENTRY_APCLI(pEntry)) {// this packet is for APCLI
								INT apcli_idx = pEntry->func_tb_idx;
								pEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[apcli_idx].MacTabWCID];
								pRxBlk->wcid = pEntry->wcid;
							} else {
								MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WCID of BAR packet!. A1:%02x:%02x:%02x:%02x:%02x:%02x,A2:%02x:%02x:%02x:%02x:%02x:%02x \n\r",__func__,
									PRINT_MAC(pRxBlk->Addr1),PRINT_MAC(pRxBlk->Addr2)));
								break;
							}

							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%02x:%02x:%02x:%02x:%02x:%02x recv BAR\n\r",__func__,
								PRINT_MAC(pRxBlk->Addr1)));
						}
#endif
					}
					else {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Cannot found WCID of BAR packet!\n",
									__FUNCTION__));
					}
				}
#endif /* MT_MAC */

				CntlEnqueueForRecv(pAd, pRxBlk->wcid, (pRxBlk->MPDUtotalByteCnt),
									(PFRAME_BA_REQ)FC);

				if (bar->BARControl.Compressed == 0) {
					UCHAR tid = bar->BARControl.TID;
					BARecSessionTearDown(pAd, pRxBlk->wcid, tid, FALSE);
				}
			}
			break;
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		case SUBTYPE_PS_POLL:
/*
            This marco is not suitable for P2P GO.
            It is OK to remove this marco here. @20140728
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
*/
			{
				USHORT Aid = pRxBlk->Duration & 0x3fff;
				PUCHAR pAddr = pRxBlk->Addr2;
				MAC_TABLE_ENTRY *pEntry;

#ifdef MT_MAC
				if ((pAd->chipCap.hif_type == HIF_MT) &&
				    (pRxBlk->wcid == RESERVED_WCID))
				{
#ifdef CONFIG_AP_SUPPORT			
					pEntry = MacTableLookup(pAd, &pRxBlk->Addr2[0]);
#endif

					if (pEntry)
						pRxBlk->wcid = pEntry->wcid;
					else {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot found WCID of PS-Poll packet!\n",
									__FUNCTION__));
					}
				}
#endif /* MT_MAC */


               //printk("dev_rx_ctrl_frm0 SUBTYPE_PS_POLL pRxBlk->wcid: %x pEntry->wcid:%x\n",pRxBlk->wcid,pEntry->wcid);
				if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
                                 //printk("dev_rx_ctrl_frm1 SUBTYPE_PS_POLL\n");
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
					if (pEntry->Aid == Aid)
						RtmpHandleRxPsPoll(pAd, pAddr, pRxBlk->wcid, FALSE);
					else {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Aid mismatch(pkt:%d, Entry:%d)!\n",
									__FUNCTION__, Aid, pEntry->Aid));
					}
				}
			}
			break;
#endif /* CONFIG_AP_SUPPORT */

#ifdef WFA_VHT_PF
		case SUBTYPE_RTS:
			if (pAd->CommonCfg.vht_bw_signal && VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
			{
				PLCP_SERVICE_FIELD *srv_field;
				RTS_FRAME *rts = (RTS_FRAME *)pRxBlk->FC;

				if ((rts->Addr1[0] & 0x1) == 0x1) {
					srv_field = (PLCP_SERVICE_FIELD *)&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[15];
					if (srv_field->dyn_bw == 1) {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x, WCID:%d, DYN,BW=%d\n",
									PRINT_MAC(rts->Addr1), pRxBlk->wcid, srv_field->cbw_in_non_ht));
					}
				}
			}
			break;

		case SUBTYPE_CTS:
			break;
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK:
//+++Add by shiang for debug
// TODO: shiang-MT7603, remove this!
			{
				UCHAR *ptr, *ra, *ta;
				BA_CONTROL *ba_ctrl;
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():BlockAck From WCID:%d\n", __FUNCTION__, pRxBlk->wcid));

				ptr = (UCHAR *)pRxBlk->FC;
				ptr += 4;
				ra = ptr;
				ptr += 6;
				ta = ptr;
				ptr += 6;
				ba_ctrl = (BA_CONTROL *)ptr;
				ptr += sizeof(BA_CONTROL);
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRA=%02x:%02x:%02x:%02x:%02x:%02x, TA=%02x:%02x:%02x:%02x:%02x:%02x\n",
							PRINT_MAC(ra), PRINT_MAC(ta)));
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA Control: AckPolicy=%d, MTID=%d, Compressed=%d, TID_INFO=0x%x\n",
							ba_ctrl->ACKPolicy, ba_ctrl->MTID, ba_ctrl->Compressed, ba_ctrl->TID));
				if (ba_ctrl->ACKPolicy == 0 && ba_ctrl->Compressed == 1) {
					BASEQ_CONTROL *ba_seq;
					ba_seq = (BASEQ_CONTROL *)ptr;
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA StartingSeqCtrl:StartSeq=%d, FragNum=%d\n",
									ba_seq->field.StartSeq, ba_seq->field.FragNum));
					ptr += sizeof(BASEQ_CONTROL);
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBA Bitmap:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
								*ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7)));
				}
			}
//---Add by shiang for debug
#endif /* DOT11_N_SUPPORT */
		case SUBTYPE_ACK:
		default:
			break;
	}

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


/*
	========================================================================
	Routine Description:
		Check Rx descriptor, return NDIS_STATUS_FAILURE if any error found
	========================================================================
*/
static INT rtmp_chk_rx_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo;
    FRAME_CONTROL *FC;

#if defined (OUI_CHECK_SUPPORT) || defined (HTC_DECRYPT_IOT)
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif
	int LogDbgLvl = DBG_LVL_ERROR;

	if((pRxBlk == NULL) || (pRxBlk->pRxInfo == NULL)) {
//+++Add by shiang for debug
		if (pRxBlk == NULL)
			printk("%s(): pRxBlk is NULL\n", __FUNCTION__);
		else
			printk("%s(): pRxBlk->pRxInfo is NULL\n", __FUNCTION__);
//---Add by shiang for debug
		return NDIS_STATUS_FAILURE;
	}

    FC = (FRAME_CONTROL *)pRxBlk->FC;
	pRxInfo = pRxBlk->pRxInfo;


#ifdef MT_MAC
	// TODO: shiang-MT7603
	if (pAd->chipCap.hif_type == HIF_MT) {
//+++Add by shiang for work-around, should remove it once we correctly configure the BSSID!
		// TODO: shiang-MT7603 work around!!
		RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		if (rxd_base->RxD2.IcvErr) {
			UCHAR band_idx = (rxd_base->RxD1.ChFreq > 14) ? 1 : 0;

#ifdef OUI_CHECK_SUPPORT
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
			if ((pAd->MacTab.oui_mgroup_cnt > 0)
#ifdef MAC_REPEATER_SUPPORT
				||(pAd->ApCfg.RepeaterCliSize !=0)
#endif
				)
			{
				if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN)) {
					INC_COUNTER64(pAd->WlanCounters[band_idx].RxHWLookupWcidErrCount);
					if (DebugLevel < DBG_LVL_TRACE)
						return NDIS_STATUS_FAILURE;
				}
#ifdef MAC_REPEATER_SUPPORT
				if (pAd->ApCfg.bMACRepeaterEn == TRUE)
				{
					BOOLEAN bwcid_error = FALSE;
					if (IS_ENTRY_REPEATER(pEntry)) {
						UCHAR	orig_wcid = pRxBlk->wcid;
						REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;
						pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);
						if (pReptEntry && (pReptEntry->CliValid == TRUE))
						{
							if (orig_wcid != pReptEntry->MacTabWCID) {
								bwcid_error = TRUE;
							}
						} else {
							bwcid_error = TRUE;
						}
					} else if (IS_ENTRY_APCLI(pEntry)) {
						INT apcli_idx = pEntry->func_tb_idx;
						if (NdisCmpMemory(pAd->ApCfg.ApCliTab[apcli_idx].wdev.if_addr, pRxBlk->Addr1, MAC_ADDR_LEN)) {
							bwcid_error = TRUE;
						}
					}
					if (bwcid_error) {
						INC_COUNTER64(pAd->WlanCounters[band_idx].RxHWLookupWcidErrCount);
						if (DebugLevel < DBG_LVL_TRACE)
							return NDIS_STATUS_FAILURE;
					}
				}
#endif /* MAC_REPEATER_SUPPORT */
			}
#endif
			INC_COUNTER64(pAd->WlanCounters[band_idx].RxICVErrorCount);

#ifdef WH_EZ_SETUP
			if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
				LogDbgLvl = DBG_LVL_TRACE;
#endif

#ifdef HTC_DECRYPT_IOT
			if (rxd_base->RxD1.HTC == 1)
			{
				        //if ((rxd_base->RxD2.SecMode != 0))
				        {
						if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
						{
							pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
						}
						
						if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)))
						{
				        		// Rx HTC and FAIL decryp case!
							//if (rxd_base->RxD2.IcvErr == 1) 
							if ((pEntry->HTC_AAD_OM_CountDown == 0) && 
								(pEntry->HTC_AAD_OM_Freeze == 0))
							{
								if (pEntry->HTC_ICVErrCnt++ > pAd->HTC_ICV_Err_TH)
								{
									pEntry->HTC_ICVErrCnt = 0; //reset the history
									pEntry->HTC_AAD_OM_CountDown = 3;
									
									if (pEntry->HTC_AAD_OM_Force == 0)
									{
										pEntry->HTC_AAD_OM_Force = 1;
										MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("@AAD_OM Trigger ! wcid=%u\n",pRxBlk->wcid));
										HW_SET_ASIC_WCID_AAD_OM(pAd,pRxBlk->wcid,1);
									}

								}
							}
						}
						
						if (pEntry->HTC_AAD_OM_Freeze)
						{

							/*
								Wokradound if the ICV Error happened again!
							*/
							if (pEntry->HTC_AAD_OM_Force)
							{
								HW_SET_ASIC_WCID_AAD_OM(pAd,pRxBlk->wcid,1);
								MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("@ICV Error, HTC_AAD_OM_Force already 1, wcid=%u",pRxBlk->wcid));
							}
							else
							{
								MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("ICV Error"));							
							}
								

						
							if(pRxBlk->Addr1 != NULL)
							    MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
									    (", Addr1 = %02x:%02x:%02x:%02x:%02x:%02x",PRINT_MAC(pRxBlk->Addr1)));
							if(pRxBlk->Addr2 != NULL)
							    MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
									    (", Addr2 = %02x:%02x:%02x:%02x:%02x:%02x",PRINT_MAC(pRxBlk->Addr2)));
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("\n"));
							dump_rmac_info_for_ICVERR(pAd, pRxBlk->rmac_info);
						}
						else
						{
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AAD_OM detection in progress!\n"));							
						}
					}
			}
			else
#endif /* HTC_DECRYPT_IOT */
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("ICV Error"));
				if(pRxBlk->Addr1 != NULL)
				    MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
						    (", Addr1 = %02x:%02x:%02x:%02x:%02x:%02x",PRINT_MAC(pRxBlk->Addr1)));
				if(pRxBlk->Addr2 != NULL)
				    MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl,
						    (", Addr2 = %02x:%02x:%02x:%02x:%02x:%02x",PRINT_MAC(pRxBlk->Addr2)));
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, LogDbgLvl, ("\n"));

				dump_rmac_info_for_ICVERR(pAd, pRxBlk->rmac_info);			
			}


			if (DebugLevel >= DBG_LVL_TRACE)
				dump_rxblk(pAd, pRxBlk);
			return NDIS_STATUS_FAILURE;
		}
		if (rxd_base->RxD2.CipherLenMis) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("CM Length Error, WlanIndex = %d\n", rxd_base->RxD2.RxDWlanIdx));
			return NDIS_STATUS_FAILURE;
		}

		if (pRxBlk->DeAmsduFail)
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
				("Deammsdu Fail, WlanIndex = %d\n", rxd_base->RxD2.RxDWlanIdx));
			return NDIS_STATUS_FAILURE;
		}

		if (rxd_base->RxD2.TkipMicErr) {
#ifdef OUI_CHECK_SUPPORT
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
			if(pAd->MacTab.oui_mgroup_cnt > 0) {
				if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN)) {
					RX_BLK_SET_FLAG(pRxBlk, fRX_WCID_MISMATCH);
				}
			}
#endif
			if (!(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH)))
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
					("TKIP MIC Error, WlanIndex = %d\n", rxd_base->RxD2.RxDWlanIdx));
		}
		pRxBlk->CipherMis = rxd_base->RxD2.CipherMis;



#ifdef HTC_DECRYPT_IOT
		if (rxd_base->RxD1.HTC == 1) //focus HTC pkt only!
		{
			if (rxd_base->RxD2.SecMode != 0)
			{
				if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
				{
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				}
				
				if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))) 
				{
					if ((pEntry->HTC_AAD_OM_CountDown == 0) && 
						(pEntry->HTC_AAD_OM_Freeze == 0))
				        {
						//Rx decrypt OK  of HTC
						if (rxd_base->RxD2.IcvErr == 0) 
						{
					            if ((rxd_base->RxD2.CipherMis == 0) &&
					            (rxd_base->RxD2.CipherLenMis == 0) &&
					            (rxd_base->RxD2.TkipMicErr == 0))
					            {
						            	pEntry->HTC_ICVErrCnt = 0; // reset counter!
						            	pEntry->HTC_AAD_OM_Freeze = 1; // decode ok, we treat the entry don't need to count pEntry->HTC_ICVErrCnt any more!
					            }
						}
					}
				}
			}
		}
#endif /* HTC_DECRYPT_IOT */

	}
#endif /* MT_MAC */


	/* Phy errors & CRC errors*/
	if (pRxInfo->Crc) {

		return NDIS_STATUS_FAILURE;
	}


	/* drop decyption fail frame*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pRxInfo->CipherErr)
		{
			/*
				WCID equal to 255 mean MAC couldn't find any matched entry in Asic-MAC table.
				The incoming packet mays come from WDS or AP-Client link.
				We need them for further process. Can't drop the packet here.
			*/
			if ((pRxInfo->U2M) && (pRxBlk->wcid == 255)
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode == WDS_LAZY_MODE)
#endif /* WDS_SUPPORT */
			)
				return NDIS_STATUS_SUCCESS;

			APRxErrorHandle(pAd, pRxBlk);

			/* Increase received error packet counter per BSS */
			if (FC->FrDs == 0 &&
				pRxInfo->U2M &&
				pRxBlk->bss_idx < pAd->ApCfg.BssidNum)
			{
				BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pRxBlk->bss_idx];
				pMbss->RxDropCount ++;
				pMbss->RxErrorCount ++;
			}

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
			if ((FC->FrDs == 1) && (FC->ToDs == 1) &&
				(VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)))
			{
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (IS_ENTRY_WDS(pEntry) && (pEntry->func_tb_idx < MAX_WDS_ENTRY))
					pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].WdsCounter.RxErrorCount++;
			}
#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): pRxInfo:Crc=%d, CipherErr=%d, U2M=%d, Wcid=%d\n",
						__FUNCTION__, pRxInfo->Crc, pRxInfo->CipherErr, pRxInfo->U2M, pRxBlk->wcid));
			return NDIS_STATUS_FAILURE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */


	return NDIS_STATUS_SUCCESS;
}


BOOLEAN dev_rx_no_foward(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	return TRUE;
}



#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	/*
		For STA, direct to OS and no need to forwad the packet to WM
	*/
	return TRUE; /* need annouce to upper layer */
}


INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = FALSE;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-->%s():pRxBlk->wcid=%d\n", __FUNCTION__, pRxBlk->wcid));

	if (!pAd)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): pAd is null", __FUNCTION__));
		return FALSE;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

	if (!pEntry)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): pEntry is null", __FUNCTION__));
		return FALSE;
	}
	wdev = pEntry->wdev;


#ifdef WH_EZ_SETUP
	if(IS_EZ_SETUP_ENABLED(wdev)) 
		hdr_len = LENGTH_802_11;
#endif

	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
	{
#ifdef CLIENT_WDS
		if ((VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
			&& IS_ENTRY_CLIENT(pEntry))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		}
#endif /* CLIENT_WDS */

#ifdef MWDS
#ifdef APCLI_SUPPORT
        if(IS_ENTRY_APCLI(pEntry))
        {
            PAPCLI_STRUCT pApCliEntry = NULL;

			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("sta_rx_pkt_allow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x \n ApCli recvd a MWDS pkt\n",
			//	wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));

            if(pEntry->func_tb_idx < MAX_APCLI_NUM)
                pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
            if(pApCliEntry)
            {
                NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);
                if(MAC_ADDR_EQUAL(pRxBlk->Addr4, pApCliEntry->wdev.if_addr))
                {
                   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ApCli receive a looping packet!\n"));
                   return FALSE;
                }
            }
        }
#endif /* APCLI_SUPPORT */

	    if(IS_MWDS_OPMODE_APCLI(pEntry))
        {


            RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		    hdr_len = LENGTH_802_11_WITH_ADDR4;
        }
#endif /* MWDS */
	}

	/* Drop not my BSS frames */
	if (pRxInfo->MyBss == 0) {
/* CFG_TODO: NEED CHECK for MT_MAC */
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
		/* When the p2p-IF up, the STA own address would be set as my_bssid address.
		   If receiving an "encrypted" broadcast packet(its WEP bit as 1) and doesn't match my BSSID,
		   Asic pass to driver with "Decrypted" marked as 0 in pRxInfo.
		   The condition is below,
		   1. p2p IF is ON,
		   2. the addr2 of the received packet is STA's BSSID,
		   3. broadcast packet,
		   4. from DS packet,
		   5. Asic pass this packet to driver with "pRxInfo->Decrypted=0"
		 */
		 if (
#ifdef RT_CFG80211_P2P_SUPPORT
             TRUE /* The dummy device always present for CFG80211 application*/
#else
             (P2P_INF_ON(pAd))
#endif /* RT_CFG80211_P2P_SUPPORT */
			&& (MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, pHeader->Addr2)) &&
			(pRxInfo->Bcast || pRxInfo->Mcast) &&
			(pFmeCtrl->FrDs == 1) &&
			(pFmeCtrl->ToDs == 0) &&
			(pRxInfo->Decrypted == 0))
		{
			/* set this m-cast frame is my-bss. */
			pRxInfo->MyBss = 1;
		}
		else
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
		{
#ifdef MWDS
            if(IS_MWDS_OPMODE_APCLI(pEntry))
                pRxInfo->MyBss = 1;
            else
#endif /* MWDS */
#ifdef FAST_EAPOL_WAR
#ifdef APCLI_SUPPORT
			if (pEntry &&
				 (IS_ENTRY_APCLI(pEntry) || (IS_ENTRY_REPEATER(pEntry)))
				)
			{
				//Focus the EAP PKT only!
				if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0) && 
					(pFmeCtrl->Type == FC_TYPE_DATA) && (MAC_ADDR_EQUAL(pEntry->wdev->bssid, pRxBlk->Addr3)))
				{
						/*
							update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
						*/
						UCHAR *pData = (UCHAR *)pFmeCtrl;

						/* 1. skip 802.11 HEADER */
						pData += LENGTH_802_11;
						//pRxBlk->DataSize -= hdr_len;

						/* 2. QOS */
						if (pFmeCtrl->SubType & 0x08)
						{
							/* skip QOS contorl field */
							pData += 2;
						}

						/* 3. Order bit: A-Ralink or HTC+ */
						if (pFmeCtrl->Order)
						{
#ifdef AGGREGATION_SUPPORT
							// TODO: shiang-MT7603, fix me, because now we don't have rx_rate.field.MODE can refer
							if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM) &&
								(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE)))
							{
								RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
							}
							else
#endif /* AGGREGATION_SUPPORT */
							{
#ifdef DOT11_N_SUPPORT
								/* skip HTC control field */
								pData += 4;
#endif /* DOT11_N_SUPPORT */
							}
						}

						/* 4. skip HW padding */
						if (pRxInfo->L2PAD)
						{
							pData += 2;
						}

						if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
						/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
						NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
						{
						    	pData += 6;
						}

						if (NdisEqualMemory(EAPOL, pData, 2))
						{
							pRxInfo->MyBss = 1; //correct this
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Hit EAP!\n", __FUNCTION__));
						}
						else
						{
							MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():  Not my bss! pRxInfo->MyBss=%d\n", __FUNCTION__, pRxInfo->MyBss));
							return FALSE;
						}
					}
			}else
#endif /* APCLI_SUPPORT */
#endif /* FAST_EAPOL_WAR */
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():  Not my bss! pRxInfo->MyBss=%d\n", __FUNCTION__, pRxInfo->MyBss));
				return FALSE;
			}
		}
	}


	pAd->RalinkCounters.RxCountSinceLastNULL++;
#ifdef UAPSD_SUPPORT
	if (wdev->UapsdInfo.bAPSDCapable
	    && pAd->CommonCfg.APEdcaParm[0].bAPSDCapable
	    && (pFmeCtrl->SubType & 0x08))
	{
		UCHAR *pData;
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("bAPSDCapable\n"));

		/* Qos bit 4 */
		pData = pRxBlk->FC + LENGTH_802_11;
		if ((*pData >> 4) & 0x01)
		{
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

			/* ccv EOSP frame so the peer can sleep */
			if (pEntry != NULL)
			{
				RTMP_PS_VIRTUAL_SLEEP(pEntry);
			}

#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));
				pAd->CommonCfg.bInServicePeriod = FALSE;

			}
		}

		if ((pFmeCtrl->MoreData) && (pAd->CommonCfg.bInServicePeriod)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MoreData bit=1, Sending trigger frm again\n"));
		}
	}
#endif /* UAPSD_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	/* 1: PWR_SAVE, 0: PWR_ACTIVE */
	if (pEntry != NULL)
	{
		UCHAR OldPwrMgmt;

		OldPwrMgmt = RtmpPsIndicate(pAd, pRxBlk->Addr2, pEntry->wcid, pFmeCtrl->PwrMgmt);
#ifdef UAPSD_SUPPORT
		RTMP_PS_VIRTUAL_TIMEOUT_RESET(pEntry);

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

					In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
					to/from Power Save Mode is not considered to be a Trigger Frame and
					the AP shall not respond with a QoS Null frame.
				*/
				/* Trigger frame must be QoS data or QoS Null frame */
				UCHAR  OldUP;

				if ((*(pRxBlk->pData+LENGTH_802_11) & 0x10) == 0)
				{
					/* this is not a EOSP frame */
					OldUP = (*(pRxBlk->pData+LENGTH_802_11) & 0x07);
					if (OldPwrMgmt == PWR_SAVE)
					{
						//hex_dump("trigger frame", pRxBlk->pData, 26);
						UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
					}
				}
				else
				{
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("This is a EOSP frame, not a trigger frame!\n"));
				}
			}
		}
#endif /* UAPSD_SUPPORT */
	}
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if ((pFmeCtrl->SubType & 0x04)) /* bit 2 : no DATA */ {
MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():  No DATA!\n", __FUNCTION__));
		return FALSE;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
    if (pEntry &&
         (IS_ENTRY_APCLI(pEntry) || (IS_ENTRY_REPEATER(pEntry)))
        )
    {

#if(defined (WH_EZ_SETUP) && defined(EZ_DUAL_BAND_SUPPORT))
		if((IS_EZ_SETUP_ENABLED(wdev)) 
#ifdef EZ_API_SUPPORT
#ifdef EZ_MOD_SUPPORT
	 		&& (wdev->ez_driver_params.ez_api_mode != CONNECTION_OFFLOAD) 
#else
			&& (wdev->ez_security.ez_api_mode != CONNECTION_OFFLOAD) 
#endif
#endif		
		){ 
			//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Rx Pkt on => wdev_type=0x%x, func_idx=0x%x\n",wdev->wdev_type,wdev->func_idx));
			//hex_dump("sta_rx_pkt_allow: Eth Hdr: ",pRxBlk->pData,14)
			//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("sta_rx_pkt_allow: Eth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
			//    ((PUCHAR)pRxBlk->pData)[0],((PUCHAR)pRxBlk->pData)[1],((PUCHAR)pRxBlk->pData)[2],((PUCHAR)pRxBlk->pData)[3],((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],
			//    ((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],
			//    ((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13]));
			/*if( ( (((PUCHAR)pRxBlk->pData)[4])& 0x1 )== 0x1){
				if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)){ 
					EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("sta_rx_pkt_allow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x \nMWDS PKT Eth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
						wdev->wdev_idx,wdev->wdev_type,wdev->func_idx,
			    		((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],
			    		((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13],((PUCHAR)pRxBlk->pData)[14],((PUCHAR)pRxBlk->pData)[15],
			    		((PUCHAR)pRxBlk->pData)[16],((PUCHAR)pRxBlk->pData)[17]));
				}
				else{ 
					EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("sta_rx_pkt_allow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x \nNON MWDS PKT Eth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
						wdev->wdev_idx,wdev->wdev_type,wdev->func_idx,
			    		((PUCHAR)pRxBlk->pData)[0],((PUCHAR)pRxBlk->pData)[1],((PUCHAR)pRxBlk->pData)[2],((PUCHAR)pRxBlk->pData)[3],((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],
			    		((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],
			    		((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13]));
			
				}
			}*/

#ifdef EZ_MOD_SUPPORT
			if (MAC_ADDR_IS_GROUP(pRxBlk->pData) 
				&& ez_sta_rx_pkt_handle(wdev, pRxBlk))
			{
				return FALSE;
			}
#else
			if((wdev->ez_security.ez_loop_chk_timer_running) && (wdev->ez_security.first_loop_check) &&  (MAC_ADDR_IS_GROUP(pRxBlk->pData))){
                EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("wdev_idx:0x%x=> Drop Rx pkt as loop check trigerred by this source\n",wdev->wdev_idx));
			    return FALSE;
			}

			if(wdev->ez_security.loop_chk_info.loop_chk_role == DEST){
				//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : DEST role APCLi got Rx Pkt\n",
				//	wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));
				if(ez_is_loop_pkt_rcvd(wdev,pRxBlk) == TRUE){
					//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Got a loop detect pkt\n"));
					return FALSE;
				}
			}
#endif			
		}
#endif

		if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0)) {
			ULONG Now32;
			PAPCLI_STRUCT pApCliEntry = NULL;

			if (!(pEntry && APCLI_IF_UP_CHECK(pAd, pEntry->wdev->func_idx)))
				return FALSE;
			
			pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx];
			if (pApCliEntry)
			{
				NdisGetSystemUpTime(&Now32);
				pApCliEntry->ApCliRcvBeaconTime = Now32;
			}

#ifdef MWDS
            if(IS_MWDS_OPMODE_APCLI(pEntry)){
				//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : Non MWDS Pkt not allowed\n",
				//	   wdev->wdev_idx,wdev->wdev_type,wdev->func_idx));
                return FALSE;
            }
#endif /* MWDS */

			if (pApCliEntry != NULL)
			{
				pApCliEntry->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
				pApCliEntry->RxCount ++;
			}

			/* Process broadcast packets */
			if (pRxInfo->Mcast || pRxInfo->Bcast)
			{
				/* Process the received broadcast frame for AP-Client. */
				if (!ApCliHandleRxBroadcastFrame(pAd, pRxBlk, pEntry))			
				{
					return FALSE;
				}
			}

			/* drop received packet which come from apcli */
			/* check if sa is to apcli */
			if (MAC_ADDR_EQUAL(pEntry->wdev->if_addr, pRxBlk->Addr3))
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s[%d]SA is from APCLI=%d\n\r",__func__,__LINE__,pEntry->wdev->func_idx));
				return FALSE;	/* give up this frame */
			}
		}
#ifdef MWDS
        if(!IS_MWDS_OPMODE_APCLI(pEntry))
#endif /* MWDS */
            RX_BLK_SET_FLAG(pRxBlk, fRX_AP);

        goto ret;
    }
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	if (pEntry) {
	}



#ifndef WFA_VHT_PF
#endif /* WFA_VHT_PF */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
ret:
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef WH_EZ_SETUP
	if(!IS_EZ_SETUP_ENABLED(wdev))
#endif
		hdr_len = LENGTH_802_11;

	return hdr_len;
}
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */


static VOID rx_data_frm_announce(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN struct wifi_dev *wdev)
{
	BOOLEAN eth_frame = FALSE;
	UCHAR *pData = pRxBlk->pData;
	UINT data_len = pRxBlk->DataSize;
	UCHAR wdev_idx = wdev->wdev_idx;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

#ifdef HDR_TRANS_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
	{
		eth_frame = TRUE;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}
#endif /* HDR_TRANS_SUPPORT */

	/* non-EAP frame */
	if (!RTMPCheckWPAframe(pAd, pEntry, pData, data_len, wdev_idx, eth_frame))
	{
		if (pRxBlk->CipherMis && FC && (FC->Type == FC_TYPE_DATA))
		{
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: CM, wcid=%d\n", __FUNCTION__, pRxBlk->wcid));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Addr1=%02x:%02x:%02x:%02x:%02x:%02x\t", PRINT_MAC(pRxBlk->Addr1)));
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Addr2=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pRxBlk->Addr2)));
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
#ifdef MWDS
		if (pEntry && IS_ENTRY_MWDS(pEntry))
		{
			if (!((FC->FrDs == 1) && (FC->ToDs == 1)))
			{
				/* release packet */
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}
#endif /* MWDS */


#ifdef WAPI_SUPPORT
		/* report to upper layer if the received frame is WAI frame */
		if (RTMPCheckWAIframe(pRxBlk->pData, pRxBlk->DataSize)) {
			Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
			return;
		}
#endif /* WAPI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		/* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
		if ((pEntry->wdev->wdev_type == WDEV_TYPE_AP || pEntry->wdev->wdev_type == WDEV_TYPE_GO) &&
		    IS_ENTRY_CLIENT(pEntry) && (pEntry->PrivacyFilter == Ndis802_11PrivFilter8021xWEP))
		{
			/*
				If	1) no any EAP frame is received within 5 sec and
					2) an encrypted non-EAP frame from peer associated STA is received,
				AP would send de-authentication to this STA.
			 */
			if (FC != NULL && FC->Wep &&
				pEntry->StaConnectTime > 5 && pEntry->SecConfig.Handshake.WpaState < AS_AUTHENTICATION2)
			{
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("==> De-Auth this STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
							PRINT_MAC(pEntry->Addr)));
				MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
			}

			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */




#ifdef CONFIG_HOTSPOT
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->pMbss) && pEntry->pMbss->HotSpotCtrl.HotSpotEnable) {		
			if (hotspot_rx_handler(pAd, pEntry, pRxBlk) == TRUE)
				return;
		}
#endif /* CONFIG_HOTSPOT */

#ifdef CONFIG_AP_SUPPORT
#ifdef STATS_COUNT_SUPPORT
		if ((IS_ENTRY_CLIENT(pEntry)) && (pEntry->pMbss))
		{
			BSS_STRUCT *pMbss = pEntry->pMbss;
			UCHAR *pDA = pRxBlk->Addr3;
			if (((*pDA) & 0x1) == 0x01) {
				if(IS_BROADCAST_MAC_ADDR(pDA))
					pMbss->bcPktsRx++;
				else
					pMbss->mcPktsRx++;
			} else
				pMbss->ucPktsRx++;
		}
#endif /* STATS_COUNT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) /*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
			Indicate_AMPDU_Packet(pAd, pRxBlk, wdev_idx);
		else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
			Indicate_AMSDU_Packet(pAd, pRxBlk, wdev_idx);
		else
#endif /* DOT11_N_SUPPORT */
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK))
			Indicate_ARalink_Packet(pAd, pEntry, pRxBlk, wdev->wdev_idx);
		else
			Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
	}
	else
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);

#ifdef CONFIG_AP_SUPPORT
		/* Update the WPA STATE to indicate the EAP handshaking is started */
		if (IS_ENTRY_CLIENT(pEntry)) {
			if (pEntry->SecConfig.Handshake.WpaState == AS_AUTHENTICATION)
			pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION2;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)
			/*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
		{
			Indicate_AMPDU_Packet(pAd, pRxBlk, wdev_idx);
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
#ifdef CONFIG_HOTSPOT_R2
			UCHAR *pData = (UCHAR *)pRxBlk->pData;

			if (pEntry)
			{
				BSS_STRUCT *pMbss = pEntry->pMbss;
				if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
			        /* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
        			NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
			    {
			        pData += 6;
			    }

				if (NdisEqualMemory(EAPOL, pData, 2))
    	    		pData += 2;

			    if ((*(pData+1) == EAPOLStart) && (pMbss->HotSpotCtrl.HotSpotEnable == 1) &&  IS_AKM_WPA2(pMbss->wdev.SecConfig.AKMMap) && (pEntry->hs_info.ppsmo_exist == 1))
				{
					UCHAR HS2_Header[4] = {0x50,0x6f,0x9a,0x12};
					memcpy(&pRxBlk->pData[pRxBlk->DataSize], HS2_Header, 4);
					memcpy(&pRxBlk->pData[pRxBlk->DataSize+4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s: hotspot rcv eapol start, %x:%x:%x:%x\n",
					__FUNCTION__,pRxBlk->pData[pRxBlk->DataSize+4], pRxBlk->pData[pRxBlk->DataSize+5],
					pRxBlk->pData[pRxBlk->DataSize+6], pRxBlk->pData[pRxBlk->DataSize+7]));
					
					pRxBlk->DataSize += 8;
				}
			}
#endif
			/* Determin the destination of the EAP frame */
			/*  to WPA state machine or upper layer */
			rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
		}
	}
}

static BOOLEAN amsdu_non_ampdu_sanity(RTMP_ADAPTER *pAd, UINT16 cur_sn, UINT8 cur_amsdu_state,
					UINT16 previous_sn, UINT8 previous_amsdu_state)
{
	BOOLEAN amsdu_miss = FALSE;

	if (cur_sn != previous_sn) {
		if ((previous_amsdu_state == FIRST_AMSDU_FORMAT) ||
			(previous_amsdu_state == MIDDLE_AMSDU_FORMAT)) {
			amsdu_miss = TRUE;
		}
	} else {
		if (((previous_amsdu_state == FIRST_AMSDU_FORMAT) ||
			(previous_amsdu_state == MIDDLE_AMSDU_FORMAT)) &&
				(cur_amsdu_state == FIRST_AMSDU_FORMAT)) {
			amsdu_miss = TRUE;
		}
	}

	return amsdu_miss;
}

static INT rx_chk_duplicate_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR wcid = pRxBlk->wcid;
	STA_TR_ENTRY *trEntry= NULL;
	INT sn = pRxBlk->SN;
	UINT32 WmmIndex = HcGetWmmIdx(pAd, wdev); 
	UCHAR up = 0; 

	/* check if AMPDU frame ignore it, since AMPDU wil handle reorder */
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		return NDIS_STATUS_SUCCESS;

	/*check is vaild sta entry*/
	if(wcid >= MAX_LEN_OF_TR_TABLE)
	{
		return NDIS_STATUS_SUCCESS;
	}

	/*check sta tr entry is exist*/
	trEntry = &pAd->MacTab.tr_entry[wcid];
	
	if(!trEntry)
	{
		return NDIS_STATUS_SUCCESS;
	}

	if ((pFmeCtrl->Type == FC_TYPE_DATA && pFmeCtrl->SubType == SUBTYPE_DATA_NULL) ||
		(pFmeCtrl->Type == FC_TYPE_DATA && pFmeCtrl->SubType == SUBTYPE_QOS_NULL))
	{
		return NDIS_STATUS_SUCCESS;
	}

	up = (WmmIndex * 4) + pRxBlk->UserPriority;
	/*check frame is QoS or Non-QoS frame*/
	if(!(pFmeCtrl->SubType & 0x08) || up >= NUM_OF_UP)
	{
		up = (NUM_OF_UP - 1);
	}
	
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU)) {
		if ((amsdu_non_ampdu_sanity(pAd, sn, pRxBlk->AmsduState,
			trEntry->previous_sn[up], trEntry->previous_amsdu_state[up]))
			|| (pRxBlk->AmsduState == FINAL_AMSDU_FORMAT)) {
			trEntry->cacheSn[up] =  trEntry->previous_sn[up];
		}

		trEntry->previous_amsdu_state[up] = pRxBlk->AmsduState;
		trEntry->previous_sn[up] = sn;

		if (!pFmeCtrl->Retry || trEntry->cacheSn[up] != sn)
			return NDIS_STATUS_SUCCESS;
	} else {
		if (!pFmeCtrl->Retry || trEntry->cacheSn[up] != sn) {
			trEntry->cacheSn[up] = sn;
			return NDIS_STATUS_SUCCESS;
		}
	}


	/* Middle/End of fragment */
	if (pRxBlk->FN && pRxBlk->FN != pAd->FragFrame.LastFrag)
	{
		return NDIS_STATUS_SUCCESS;
	}

	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): pFrameCtrl->Retry=%d, trEntry->cacheSn[%d]=%d, pkt->sn=%d\n", 
			__FUNCTION__, pFmeCtrl->Retry, up, trEntry->cacheSn[up], sn));
	
	/*is duplicate frame, should return failed*/
	return NDIS_STATUS_FAILURE;
}


static VOID rx_802_3_data_frm_announce(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk, struct wifi_dev *wdev)
{
#ifdef CONFIG_HOTSPOT
	if (IS_ENTRY_CLIENT(pEntry) && (pEntry->pMbss) && pEntry->pMbss->HotSpotCtrl.HotSpotEnable) {	
			if (hotspot_rx_handler(pAd, pEntry, pRxBlk) == TRUE)
					return;
	}
#endif /* CONFIG_HOTSPOT */
#ifdef RTMP_UDMA_SUPPORT
	/* If received packet is untagged, but the corresponding 'wdev' has a valid VLANID, 
	   	 * then insert 4-byte VLAN header with that VLANID.
	*/
	if(!(((GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket))[12] == 0X81) && ((GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket)[13] = 0x00))))
	{
		USHORT vlanid, VLAN_Priority = 0;
	//	struct wifi_dev *wdev = pAd->wdev_list[wdev_idx];
		vlanid = wdev->VLAN_VID;
		VLAN_Priority = wdev->VLAN_Priority;
		if(vlanid)
		{
			UCHAR VLAN_Size = LENGTH_802_1Q;
			UCHAR *Header802_3 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
			UCHAR *data_p = OS_PKT_HEAD_BUF_EXTEND(pRxBlk->pRxPacket, VLAN_Size);
			RT_VLAN_8023_HEADER_COPY(pAd, vlanid, VLAN_Priority,
				Header802_3, LENGTH_802_3,data_p, TPID);
		}
	}	
#endif	/* RTMP_UDMA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
    if(pAd->ApCfg.bRoamingEnhance)
    {
        if((pAd->ApCfg.ApCliInfRunned > 0)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
            || ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE))
#endif /* CONFIG_WIFI_PKT_FWD */
        )
        {
            if((pEntry->bRoamingRefreshDone == FALSE) && IS_ENTRY_CLIENT(pEntry))
                ApCliDoRoamingRefresh(pAd, pEntry, pRxBlk->pRxPacket, wdev, pRxBlk->Addr3);
        }
    }
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if ((pAd->chipCap.asic_caps & fASIC_CAP_BA_OFFLOAD) == fASIC_CAP_BA_OFFLOAD)
	{
		Indicate_802_3_Packet(pAd, pRxBlk, wdev->wdev_idx);
	}
	else
	{
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))
		{
			BAReorder(pAd, pRxBlk, wdev->wdev_idx);
		}
		else
		{
			Indicate_802_3_Packet(pAd, pRxBlk, wdev->wdev_idx);
		}
	}
}


VOID dev_rx_802_3_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN drop_err = TRUE;
#endif
	UCHAR wdev_idx = BSS0;
	BOOLEAN bFragment = FALSE;

	RETURN_IF_PAD_NULL(pAd);

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
	{
#if defined (OUI_CHECK_SUPPORT) && defined (MAC_REPEATER_SUPPORT)
		BOOLEAN	bCheck_ApCli = FALSE;
		UCHAR	orig_wcid = pRxBlk->wcid;
#endif
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		/*double confirm wcid lookup result*/
#ifdef OUI_CHECK_SUPPORT
		if ((pAd->MacTab.oui_mgroup_cnt > 0)
#ifdef MAC_REPEATER_SUPPORT
				||(pAd->ApCfg.RepeaterCliSize !=0)
#endif
		) {
			if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN)) {
#ifdef CONFIG_AP_SUPPORT	
				pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
#endif
				if (pEntry) {
					pRxBlk->wcid = pEntry->wcid;
				}
			}
#ifdef MAC_REPEATER_SUPPORT
			if (pEntry) {
				if (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_APCLI(pEntry)) {
					bCheck_ApCli = TRUE;
				}
			}
#endif
		}

#ifdef MAC_REPEATER_SUPPORT
		if (bCheck_ApCli == TRUE)
		{
			if (pAd->ApCfg.bMACRepeaterEn == TRUE)
			{
				REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;
				BOOLEAN					entry_found = FALSE;
				pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);
				if (pReptEntry && (pReptEntry->CliValid == TRUE))
				{
					pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
					pRxBlk->wcid = pEntry->wcid;
					entry_found = TRUE;
				} else if ((pReptEntry == NULL) && IS_ENTRY_APCLI(pEntry)) {// this packet is for APCLI
					INT apcli_idx = pEntry->func_tb_idx;
					pEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[apcli_idx].MacTabWCID];
					pRxBlk->wcid = pEntry->wcid;
					entry_found = TRUE;
				}
				if (entry_found) {
					if (orig_wcid != pRxBlk->wcid) {
						pAd->MacTab.repeater_wcid_error_cnt++;
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("orig_wcid=%d,pRxBlk->wcid=%d\n\r",orig_wcid,pRxBlk->wcid));
					}
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s()[%d]:%02x:%02x:%02x:%02x:%02x:%02x recv data\n\r",__func__,__LINE__,
							 PRINT_MAC(pRxBlk->Addr1)));
				} else {
					if ((pRxBlk->Addr1[0] & 0x01) == 0x01) {/* BM packet,find apcli entry */
						if (IS_ENTRY_APCLI(pEntry)) {
							pRxBlk->wcid = pEntry->wcid;
							pAd->MacTab.repeater_bm_wcid_error_cnt++;
						} else if (IS_ENTRY_REPEATER(pEntry)) {
							UCHAR apcli_wcid = 0;
							if (pEntry->wdev && (pEntry->wdev->func_idx < pAd->ApCfg.ApCliNum) ) {
								apcli_wcid = pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx].MacTabWCID;
							} else { // use default apcli0
								apcli_wcid = pAd->ApCfg.ApCliTab[0].MacTabWCID;
							}
							pEntry = &pAd->MacTab.Content[apcli_wcid];
							pRxBlk->wcid = pEntry->wcid;
							pAd->MacTab.repeater_bm_wcid_error_cnt++;
						}
					} else {
						MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()[%d]:Cannot found WCID of data packet!. A1:%02x:%02x:%02x:%02x:%02x:%02x,A2:%02x:%02x:%02x:%02x:%02x:%02x,wcid=%d,orig_wcid=%d,M/B/U=%d/%d/%d\n\r",__func__,
								 __LINE__,PRINT_MAC(pRxBlk->Addr1),PRINT_MAC(pRxBlk->Addr2),pEntry->wcid,orig_wcid,pRxBlk->pRxInfo->Mcast,pRxBlk->pRxInfo->Bcast,pRxBlk->pRxInfo->U2M));
					}
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */

#endif /*OUI_CHECK_SUPPORT*/
	}
	else
	{
#ifdef CONFIG_AP_SUPPORT	
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
#endif
		if (pEntry) {
			pRxBlk->wcid = pEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT
			if (IS_ENTRY_APCLI(pEntry)) {
				pRxBlk->wcid = pEntry->wcid;
			} else if (IS_ENTRY_REPEATER(pEntry)) {
				UCHAR apcli_wcid = 0;
				if (pEntry->wdev && (pEntry->wdev->func_idx < pAd->ApCfg.ApCliNum) ) {
					apcli_wcid = pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx].MacTabWCID;
				} else { // use default apcli0
					apcli_wcid = pAd->ApCfg.ApCliTab[0].MacTabWCID;
				}
				pEntry = &pAd->MacTab.Content[apcli_wcid];
			    pRxBlk->wcid = pEntry->wcid;
	        }
#endif
		}
	}

    if (pEntry && pEntry->wdev && pEntry->wdev->rx_pkt_allowed)
    {
        if (!pEntry->wdev->rx_pkt_allowed(pAd, pRxBlk)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(): rx_pkt_allowed drop this paket!\n", __FUNCTION__));
            goto drop;
        }
    }
    else
    {
        if (pEntry)
        {
            MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("invalid hdr_len, wdev=%p! ", pEntry->wdev));
            if (pEntry->wdev)
            {
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("rx_pkt_allowed=%p!", pEntry->wdev->rx_pkt_allowed));
            }
            MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
        }
        else
        {
#ifdef CONFIG_AP_SUPPORT
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
            if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
            {
                if (MAC_ADDR_EQUAL(pRxBlk->Addr1, pAd->CurrentAddress))
			pEntry = FindWdsEntry(pAd, pRxBlk);
            }
#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

            /* check if Class2 or 3 error */
            if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))
            {
                APChkCls2Cls3Err(pAd, pRxBlk);
            }
#endif /* CONFIG_AP_SUPPORT */
        }

#ifdef WH_EZ_SETUP
		if( !IS_ADPTR_EZ_SETUP_ENABLED(pAd) )
#endif
			printk("%s(): drop this packet as pEntry NULL OR !rx_pkt_allowed !!\n", __FUNCTION__);

        goto drop;
    }

    
	wdev = pEntry->wdev;
	wdev_idx = wdev->wdev_idx;
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
                __FUNCTION__, pEntry->wcid, wdev->wdev_idx,
                pRxBlk->Flags,
                RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
                RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
                RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
                pFmeCtrl->Type, pFmeCtrl->SubType,
                pFmeCtrl->FrDs, pFmeCtrl->ToDs));

   	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
   	/* must be here, before no DATA check */
	if (wdev->rx_ps_handle)
		wdev->rx_ps_handle(pAd, pRxBlk);

	pEntry->NoDataIdleCount = 0;
	pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;

#ifdef CONFIG_AP_SUPPORT
	pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
	pEntry->OneSecRxBytes += pRxBlk->MPDUtotalByteCnt;
	pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
	INC_COUNTER64(pEntry->RxPackets);
#endif /* CONFIG_AP_SUPPORT */
	pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;

	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x08)
	{
		if (pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0)
			pRxInfo->BA = 1;
		else
			pRxInfo->BA = 0;	

		if (pRxBlk->AmsduState)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
		}

		if (pRxInfo->BA)
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}

	/*check if duplicate frame, ignore it and then drop*/
	if(rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): duplicate frame drop it!\n", __FUNCTION__));
		goto drop;
	}
	
	if ((pRxBlk->FN == 0) && (pFmeCtrl->MoreFrag != 0))
	{
		bFragment = TRUE;
		RTMPDeFragmentDataFrame(pAd, pRxBlk);
	}


	if (pRxInfo->U2M)
	{
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);
#ifdef CONFIG_AP_SUPPORT
		//Update_Rssi_Sample(pAd, &pAd->ApCfg.RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
#ifdef SMART_CARRIER_SENSE_SUPPORT
               //Update_Rssi_Sample(pAd, &pEntry->RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
		//pAd->ApCfg.NumOfAvgRssiSample ++;
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef IGMP_SNOOP_SUPPORT
	if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
		&& (wdev->IgmpSnoopEnable)
		&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
	{
		PUCHAR pDA = pRxBlk->Addr3;
		PUCHAR pSA = pRxBlk->Addr2;
		PUCHAR pData = pRxBlk->pData + 12;
		UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));
		if (protoType == ETH_P_IP)
			IGMPSnooping(pAd, pDA, pSA, pData, pEntry, pRxBlk->wcid);
		else if (protoType == ETH_P_IPV6)
			MLDSnooping(pAd, pDA, pSA,  pData, pEntry, pRxBlk->wcid);
	}
#endif /* IGMP_SNOOP_SUPPORT */

	if (pRxBlk->pRxPacket)
	{
		RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);
		rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, pEntry->wdev);
	}

	return;

drop:
#ifdef CONFIG_AP_SUPPORT
	/* Increase received error packet counter per BSS */
	if (pFmeCtrl->FrDs == 0 &&
		pRxInfo->U2M &&
		pRxBlk->bss_idx < pAd->ApCfg.BssidNum)
	{
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxDropCount ++;
		if (drop_err == TRUE)
			pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxErrorCount ++;
	}
#endif /* CONFIG_AP_SUPPORT */

	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);

#ifdef CUT_THROUGH_DBG
	pAd->RxDropPacket++;
#endif

	return;

}


/*
 All Rx routines use RX_BLK structure to hande rx events
 It is very important to build pRxBlk attributes
  1. pHeader pointer to 802.11 Header
  2. pData pointer to payload including LLC (just skip Header)
  3. set payload size including LLC to DataSize
  4. set some flags with RX_BLK_SET_FLAG()
*/
// TODO: shiang-usw, FromWhichBSSID is replaced by "pRxBlk->wdev_idx"
// TODO:
// TODO: FromWhichBSSID = pEntry->apidx // For AP
// TODO: FromWhichBSSID = BSS0; // For STA
// TODO: FromWhichBSSID = pEntry->MatchMeshTabIdx + MIN_NET_DEVICE_FOR_MESH; // For Mesh
// TODO: FromWhichBSSID = pEntry->MatchWDSTabIdx + MIN_NET_DEVICE_FOR_WDS; // For WDS
// TODO: FromWhichBSSID = pEntry->MatchAPCLITabIdx + MIN_NET_DEVICE_FOR_APCLI; // For APCLI
// TODO: FromWhichBSSID = pEntry->apidx + MIN_NET_DEVICE_FOR_P2P_GO;  // For P2P
VOID dev_rx_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN bFragment = FALSE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR wdev_idx = BSS0;
	UCHAR UserPriority = 0;
	INT hdr_len = LENGTH_802_11;
	COUNTER_RALINK *pCounter = &pAd->RalinkCounters;
	UCHAR *pData;
	struct wifi_dev *wdev;
	BOOLEAN drop_err = TRUE;
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	NDIS_STATUS status;
#endif /* defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT) */

    MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-->%s():pRxBlk->wcid=%d, pRxBlk->DataSize=%d\n",
                __FUNCTION__, pRxBlk->wcid, pRxBlk->DataSize));

//dump_rxblk(pAd, pRxBlk);


//+++Add by shiang for debug

//	hex_dump("DataFrameHeader", (UCHAR *)pHeader, sizeof(HEADER_802_11));
//	hex_dump("DataFramePayload", pRxBlk->pData , (pRxBlk->DataSize > 128 ? 128 :pRxBlk->DataSize));
//---Add by shiangf for debug

	// TODO: shiang-usw, check wcid if we are repeater mode! when in Repeater mode, wcid is get by "A2" + "A1"
	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		//Carter, the below segment shall apply to unify mac chipset. 2014-0411
		if (NdisCmpMemory(pEntry->Addr, pRxBlk->Addr2, MAC_ADDR_LEN)) {
#ifdef CONFIG_AP_SUPPORT	
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
#endif
			if (pEntry)
				pRxBlk->wcid = pEntry->wcid;
		}
	} else {
		/* IOT issue with Marvell test bed AP
		    Marvell AP ResetToOOB and do wps.
		    Because of AP send EAP Request too fast and without retransmit.
		    STA not yet add BSSID to WCID search table.
		    So, the EAP Request is dropped.
		    The patch lookup pEntry from MacTable.
		*/
#ifdef CONFIG_AP_SUPPORT	
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

#ifdef FAST_EAPOL_WAR
#ifdef MAC_REPEATER_SUPPORT
		if (pEntry) {
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && 
				(IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_APCLI(pEntry))) {
				REPEATER_CLIENT_ENTRY	*pReptEntry = NULL;
				pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, &pRxBlk->Addr1[0], TRUE);
				if (pReptEntry) {
					if (VALID_WCID(pReptEntry->MacTabWCID)) {
						pRxBlk->wcid = pReptEntry->MacTabWCID;
						pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
					}
				}
			}
		}
#endif /*MAC_REPEATER_SUPPORT*/
#endif /* FAST_EAPOL_WAR */

#endif
		if (pEntry)
			pRxBlk->wcid = pEntry->wcid;
	}


	/*
		if FrameCtrl.type == DATA
			FromDS = 1, ToDS = 1 : send from WDS/MESH
			FromDS = 1, ToDS = 0 : send from STA
			FromDS = 0, ToDS = 1 : send from AP
			FromDS = 0, ToDS = 0 : AdHoc / TDLS

			if pRxBlk->wcid == VALID,
				directly assign to the pEntry[WCID]->wdev->rx

			if pRxBlk->wcid == INVALD,
				FromDS = 1, ToDS = 1 : WDS/MESH Rx
				FromDS = 1, ToDS = 0 : drop
				FromDS = 0, ToDS = 1 : drop
				FromDS = 0, ToDS = 0 : AdHoc/TDLS Rx
	*/
	if (pEntry && pEntry->wdev && pEntry->wdev->rx_pkt_allowed) {
		hdr_len = pEntry->wdev->rx_pkt_allowed(pAd, pRxBlk);
		if (hdr_len == 0) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s()[%d]: drop this packet!\n", __FUNCTION__,__LINE__));
			goto drop;
		}
	} else {
		if (pEntry) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("invalid hdr_len, wdev=%p! ", pEntry->wdev));
			if (pEntry->wdev) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("rx_pkt_allowed=%p!", pEntry->wdev->rx_pkt_allowed));
			}
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}
		else
		{
#ifdef CONFIG_AP_SUPPORT
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
			if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
			{
				if (MAC_ADDR_EQUAL(pRxBlk->Addr1, pAd->CurrentAddress))
					pEntry = FindWdsEntry(pAd, pRxBlk);
			}
#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

			/* check if Class2 or 3 error */
			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))
			{
				APChkCls2Cls3Err(pAd, pRxBlk);
			}
#endif /* CONFIG_AP_SUPPORT */
		}
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()[%d]: drop this packet!\n", __FUNCTION__,__LINE__));
		goto drop;
	}

	wdev = pEntry->wdev;
	wdev_idx = wdev->wdev_idx;
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
                __FUNCTION__, pEntry->wcid, wdev->wdev_idx,
                pRxBlk->Flags,
                RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
                RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
                RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
                pFmeCtrl->Type, pFmeCtrl->SubType,
                pFmeCtrl->FrDs, pFmeCtrl->ToDs));

   	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
   	/* must be here, before no DATA check */
	pData = pRxBlk->FC;

	if (wdev->rx_ps_handle)
		wdev->rx_ps_handle(pAd, pRxBlk);

	pEntry->NoDataIdleCount = 0;
	pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;

	/*
		update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
	*/
	pData = pRxBlk->FC;

	/* 1. skip 802.11 HEADER */
	pData += hdr_len;
	pRxBlk->DataSize -= hdr_len;

	/* 2. QOS */
	if (pFmeCtrl->SubType & 0x08)
	{
		UserPriority = *(pData) & 0x0f;

#ifdef CONFIG_AP_SUPPORT

		/* count packets priroity more than BE */
#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
			detect_wmm_traffic(pAd, UserPriority, 0);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#endif /* CONFIG_AP_SUPPORT */

		if (pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0)
			pRxInfo->BA = 1;
		else
			pRxInfo->BA = 0;

		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pData) & 0x80)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
			pCounter->RxAMSDUCount.u.LowPart++;
		}

#ifdef DOT11_N_SUPPORT
		if (pRxInfo->BA)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);

			/* incremented by the number of MPDUs */
			/* received in the A-MPDU when an A-MPDU is received. */
			pCounter->MPDUInReceivedAMPDUCount.u.LowPart ++;
		}
#endif /* DOT11_N_SUPPORT */

		/* skip QOS contorl field */
		pData += 2;
		pRxBlk->DataSize -= 2;
	}
	pRxBlk->UserPriority = UserPriority;

	/*check if duplicate frame, ignore it and then drop*/
	if(rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE)
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): duplicate frame drop it!\n", __FUNCTION__));
		goto drop;
	}



	/* 3. Order bit: A-Ralink or HTC+ */
	if (pFmeCtrl->Order)
	{
#ifdef AGGREGATION_SUPPORT
		// TODO: shiang-MT7603, fix me, because now we don't have rx_rate.field.MODE can refer
		if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM) &&
			(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE)))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		}
		else
#endif /* AGGREGATION_SUPPORT */
		{
#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			if (pAd->chipCap.FlgHwTxBfCap && (pFmeCtrl->SubType & 0x08))
				handleHtcField(pAd, pRxBlk);
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

			/* skip HTC control field */
			pData += 4;
			pRxBlk->DataSize -= 4;
#endif /* DOT11_N_SUPPORT */
		}
	}

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (pFmeCtrl->SubType & 0x04) /* bit 2 : no DATA */
	{
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Null/QosNull frame!\n", __FUNCTION__));

		drop_err = FALSE;
		goto drop;
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD)
	{
		/* just move pData pointer because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pData += 2;
	}


	pRxBlk->pData = pData;

#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1)
	   and it's passed to driver with "Decrypted" marked as 0 in RxInfo.
	*/
	if (pAd->chipCap.hif_type != HIF_MT)
	{
	if ((pFmeCtrl->Wep == 1) && (pRxInfo->Decrypted == 0))
	{
#ifdef HDR_TRANS_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
		{
			status = RTMPSoftDecryptionAction(pAd,
								 	pRxBlk->FC,
									 UserPriority,
									 &pEntry->PairwiseKey,
								 	 pRxBlk->pTransData + 14,
									 &(pRxBlk->TransDataSize));
		}
		else
#endif /* HDR_TRANS_SUPPORT */
		{
			CIPHER_KEY *pSwKey = &pEntry->PairwiseKey;


			status = RTMPSoftDecryptionAction(pAd,
								 	pRxBlk->FC,
									 UserPriority,
									 pSwKey,
								 	 pRxBlk->pData,
									 &(pRxBlk->DataSize));
		}

		if ( status != NDIS_STATUS_SUCCESS)
		{
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
		/* Record the Decrypted bit as 1 */
		pRxInfo->Decrypted = 1;
	}
	}
#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */



#ifdef DOT11_N_SUPPORT
#ifndef DOT11_VHT_AC
#ifndef WFA_VHT_PF
// TODO: shiang@PF#2, is this atheros protection still necessary here????
	/* check Atheros Client */
	if (!pEntry->bIAmBadAtheros && (pFmeCtrl->Retry) &&
		(pRxBlk->rx_rate.field.MODE < MODE_VHT) &&
		(pRxInfo->AMPDU == 1) && (pAd->CommonCfg.bHTProtect == TRUE)
	)
	{
		if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
			RTMP_UPDATE_PROTECT(pAd, 8 , ALLN_SETPROTECT, FALSE, FALSE);
		pEntry->bIAmBadAtheros = TRUE;

		if (IS_SECURITY_Entry(pEntry))
			pEntry->MpduDensity = 6;
	}
#endif /* WFA_VHT_PF */
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */


	/* update rssi sample */
	//Update_Rssi_Sample(pAd, &pEntry->RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);


	if (pRxInfo->U2M)
	{
#ifdef CONFIG_AP_SUPPORT
		//Update_Rssi_Sample(pAd, &pAd->ApCfg.RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
		//pAd->ApCfg.NumOfAvgRssiSample ++;
#endif /* CONFIG_AP_SUPPORT */

		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#ifdef TXBF_SUPPORT
		if (pRxBlk->rx_rate.field.ShortGI)
			pEntry->OneSecRxSGICount++;
		else
			pEntry->OneSecRxLGICount++;
#endif /* TXBF_SUPPORT */

#ifdef DBG_DIAGNOSE
		if (pAd->DiagStruct.inited) {
			struct dbg_diag_info *diag_info;
			diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
			diag_info->RxDataCnt++;
#ifdef DBG_RX_MCS
			if (pRxBlk->rx_rate.field.MODE == MODE_HTMIX ||
				pRxBlk->rx_rate.field.MODE == MODE_HTGREENFIELD) {
				if (pRxBlk->rx_rate.field.MCS < MAX_MCS_SET)
					diag_info->RxMcsCnt_HT[pRxBlk->rx_rate.field.MCS]++;
			}
#ifdef DOT11_VHT_AC
			if (pRxBlk->rx_rate.field.MODE == MODE_VHT) {
				INT mcs_idx = ((pRxBlk->rx_rate.field.MCS >> 4) * 10) +
								(pRxBlk->rx_rate.field.MCS & 0xf);
				if (mcs_idx < MAX_VHT_MCS_SET)
					diag_info->RxMcsCnt_VHT[mcs_idx]++;
			}
#endif /* DOT11_VHT_AC */
#endif /* DBG_RX_MCS */
		}
#endif /* DBG_DIAGNOSE */
	}

	wdev->LastSNR0 = (UCHAR)(pRxBlk->rx_signal.raw_snr[0]);
	wdev->LastSNR1 = (UCHAR)(pRxBlk->rx_signal.raw_snr[1]);
#ifdef DOT11N_SS3_SUPPORT
	wdev->LastSNR2 = (UCHAR)(pRxBlk->rx_signal.raw_snr[2]);
#endif /* DOT11N_SS3_SUPPORT */
	pEntry->freqOffset = (CHAR)(pRxBlk->rx_signal.freq_offset);
	pEntry->freqOffsetValid = TRUE;

	if ((pRxBlk->FN != 0) || (pFmeCtrl->MoreFrag != 0))
	{
		bFragment = TRUE;
		RTMPDeFragmentDataFrame(pAd, pRxBlk);
	}

	if (pRxBlk->pRxPacket)
	{
		/*
			process complete frame which encrypted by TKIP,
			Minus MIC length and calculate the MIC value
		*/
		if (bFragment && (pFmeCtrl->Wep) && IS_CIPHER_TKIP_Entry(pEntry))
		{
			pRxBlk->DataSize -= 8;
			if (rtmp_chk_tkip_mic(pAd, pEntry, pRxBlk) == FALSE)
				return;
		}

#ifdef CONFIG_AP_SUPPORT
		pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
        pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
		INC_COUNTER64(pEntry->RxPackets);
#endif /* CONFIG_AP_SUPPORT */
        pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;

#ifdef IKANOS_VX_1X0
		RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
#endif /* IKANOS_VX_1X0 */

#ifdef MAC_REPEATER_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
			RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
				&& (wdev->IgmpSnoopEnable)
				&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
			{
				PUCHAR pDA = pRxBlk->Addr3;
				PUCHAR pSA = pRxBlk->Addr2;
				PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
				UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

				if (protoType == ETH_P_IP)
					IGMPSnooping(pAd, pDA, pSA, pData, pEntry, pRxBlk->wcid);
				else if (protoType == ETH_P_IPV6)
					MLDSnooping(pAd, pDA, pSA,  pData, pEntry, pRxBlk->wcid);
			}
#endif /* IGMP_SNOOP_SUPPORT */


			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			{
				rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
			}
			else
			{
				rx_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
			}
	}

	return;

drop:
#ifdef CONFIG_AP_SUPPORT
	/* Increase received error packet counter per BSS */
	if (pFmeCtrl->FrDs == 0 &&
		pRxInfo->U2M &&
		pRxBlk->bss_idx < pAd->ApCfg.BssidNum)
	{
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxDropCount ++;
		if (drop_err == TRUE)
			pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxErrorCount ++;
	}
#endif /* CONFIG_AP_SUPPORT */
	//MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():release packet!\n", __FUNCTION__));

	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);

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
NDIS_STATUS header_packet_process(
    RTMP_ADAPTER *pAd,
    PNDIS_PACKET pRxPacket,
    RX_BLK *pRxBlk)
{

#ifdef MT_MAC
    if (pAd->chipCap.hif_type == HIF_MT) {
        if ((pRxBlk->DataSize == 0) && (pRxPacket)) {
            RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
            MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Packet Length is zero!\n", __FUNCTION__));
#ifdef CUT_THROUGH_DBG
			pAd->RxDropPacket++;
#endif
            return NDIS_STATUS_INVALID_DATA;
        }
    }
#endif /* MT_MAC */

#ifdef RLT_MAC
#ifdef CONFIG_ANDES_SUPPORT
#ifdef RTMP_PCI_SUPPORT
    if ((pAd->chipCap.hif_type == HIF_RLT) &&
        (pRxBlk->pRxFceInfo->info_type == CMD_PACKET))
    {
        MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Receive command packet.\n", __FUNCTION__));
        return NDIS_STATUS_SUCCESS;
    }
#endif /* RTMP_PCI_SUPPORT */
#endif /* CONFIG_ANDES_SUPPORT */

    // TODO: shiang-6590, handle packet from other ports
    if (pAd->chipCap.hif_type == HIF_RLT)
    {
        RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
        RXFCE_INFO *pFceInfo = pRxBlk->pRxFceInfo;

#ifdef HDR_TRANS_SUPPORT
        if ((pFceInfo->info_type == 0) && (pFceInfo->pkt_80211 == 0) &&
            (pRxInfo->hdr_trans_ip_sum_err == 1))
        {
			if (pRxBlk->pRxInfo->hdr_trans_ip_sum_err)
				RX_BLK_SET_FLAG(pRxBlk, fRX_HDR_TRANS)
            pRxBlk->bHdrVlanTaged = pRxBlk->pRxInfo->vlan_taged_tcp_sum_err;
            if (IS_MT7601(pAd))
                pRxBlk->pTransData = pRxBlk->FC +  38; /* 36 byte + 802.3 padding */
            else
                pRxBlk->pTransData = pRxBlk->FC +  36; /* 36 byte RX Wifi Info */
            pRxBlk->TransDataSize = pRxBlk->MPDUtotalByteCnt;
        } else
#endif /* HDR_TRANS_SUPPORT */
        if ((pFceInfo->info_type != 0) || (pFceInfo->pkt_80211 != 1))
        {
            RXD_STRUC *pRxD = (RXD_STRUC *)&pRxBlk->hw_rx_info[0];

            MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s(): GetFrameFromOtherPorts!\n", __FUNCTION__));
            hex_dump("hw_rx_info", &pRxBlk->hw_rx_info[0], sizeof(pRxBlk->hw_rx_info));
            MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Dump the RxD, RxFCEInfo and RxInfo:\n"));
            hex_dump("RxD", (UCHAR *)pRxD, sizeof(RXD_STRUC));
#ifdef RTMP_MAC_PCI
            dump_rxd(pAd, pRxD);
#endif /* RTMP_MAC_PCI */
            dumpRxFCEInfo(pAd, pFceInfo);
            dump_rxinfo(pAd, pRxInfo);
            hex_dump("RxFrame", (UCHAR *)GET_OS_PKT_DATAPTR(pRxPacket), (pFceInfo->pkt_len));
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<==\n"));
            RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
            return NDIS_STATUS_INVALID_DATA;
        }
    }
#endif /* RLT_MAC */


#ifdef RT_BIG_ENDIAN
    RTMPFrameEndianChange(pAd, pRxBlk->FC, DIR_READ, TRUE);
    // TODO: shiang-usw, following endian swap move the GetPacketFromRxRing()
    //RTMPWIEndianChange(pAd , (UCHAR *)pRxWI, TYPE_RXWI);
#endif /* RT_BIG_ENDIAN */

    //+++Add by shiang for debug
    //---Add by shiang for debug

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
    if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RXWI)
    {
        dbQueueEnqueueRxFrame(GET_OS_PKT_DATAPTR(pRxPacket),
                                    (UCHAR *)pHeader,
                                    pAd->CommonCfg.DebugFlags);
    }
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
    if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) &&
        (NdisEqualMemory(pAd->cfg80211_ctrl.P2PCurrentAddress, pHeader->Addr1, MAC_ADDR_LEN) ||
        (pHeader->FC.SubType == SUBTYPE_PROBE_REQ)))
    {
        SET_PKT_OPMODE_AP(pRxBlk);
    }
        else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) &&
        	(((pHeader->FC.SubType == SUBTYPE_BEACON || pHeader->FC.SubType == SUBTYPE_PROBE_RSP) &&
        	   NdisEqualMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].CfgApCliBssid, pHeader->Addr2, MAC_ADDR_LEN)) ||
                        (pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
        	 NdisEqualMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].MlmeAux.Bssid, pHeader->Addr2, MAC_ADDR_LEN)))
    {
		/* 
		   1. Beacon & ProbeRsp for Connecting & Tracking 
                   2. ProbeReq for P2P Search
                   3. Any Packet's Addr2 Equals MlmeAux.Bssid when connected   
                 */
        SET_PKT_OPMODE_AP(pRxBlk);
    }
    else
#endif /* RT_CFG80211_P2P_SUPPORT */
    {
		//todo: Bind to pAd->net_dev
		if (RTMP_CFG80211_HOSTAPD_ON(pAd))
			SET_PKT_OPMODE_AP(pRxBlk);
		else
			SET_PKT_OPMODE_STA(pRxBlk);
    }
#endif /* RT_CFG80211_SUPPORT */



#ifdef CFG80211_MULTI_STA
    if (RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev) &&
                        (((FC->SubType == SUBTYPE_BEACON || FC->SubType == SUBTYPE_PROBE_RSP) &&
                        NdisEqualMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].CfgApCliBssid, pRxBlk->Addr2, MAC_ADDR_LEN)) ||
                        (pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
                        NdisEqualMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].MlmeAux.Bssid, pRxBlk->Addr2, MAC_ADDR_LEN)))
    {
            SET_PKT_OPMODE_AP(pRxBlk);
    }
    else
    {
            SET_PKT_OPMODE_STA(pRxBlk);
    }
#endif /* CFG80211_MULTI_STA */

    /* Increase Total receive byte counter after real data received no mater any error or not */
    pAd->RalinkCounters.ReceivedByteCount += pRxBlk->DataSize;
    pAd->RalinkCounters.OneSecReceivedByteCount += pRxBlk->DataSize;
    pAd->RalinkCounters.RxCount++;
    pAd->RalinkCounters.OneSecRxCount++;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd)) {
		MT_ATERxDoneHandle(pAd, pRxBlk);
		return NDIS_STATUS_SUCCESS;
	}
#endif

#ifdef STATS_COUNT_SUPPORT
    INC_COUNTER64(pAd->WlanCounters[0].ReceivedFragmentCount);
#endif /* STATS_COUNT_SUPPORT */

    /* Check for all RxD errors */
    if (rtmp_chk_rx_err(pAd, pRxBlk) != NDIS_STATUS_SUCCESS)
    {
        pAd->Counters8023.RxErrors++;

        RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
        MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): CheckRxError!\n", __FUNCTION__));
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
        return NDIS_STATUS_INVALID_DATA;
    }


        // TODO: shiang-usw, for P2P, we original has following code, need to check it and merge to correct place!!!

    return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rx_packet_process(
    RTMP_ADAPTER *pAd,
    PNDIS_PACKET pRxPacket,
    RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#ifdef CONFIG_ATE
	if (ATE_ON(pAd)) {
		//TODO::Check if Rx cutthrough stable
		if(pRxPacket)
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_SUCCESS;
	}
#endif /* CONFIG_ATE */



    if (FC == NULL)
    {
        RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
        return NDIS_STATUS_SUCCESS;
    }

	switch (FC->Type)
	{
		case FC_TYPE_DATA:
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("rx_packet_process: wcid=0x%x\n",pRxBlk->wcid));
#ifdef HDR_TRANS_RX_SUPPORT
			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			{
        		dev_rx_802_3_data_frm(pAd, pRxBlk);
    		}
			else
#endif /* HDR_TRANS_RX_SUPPORT */
				dev_rx_data_frm(pAd, pRxBlk);
            break;

        case FC_TYPE_MGMT:
            dev_rx_mgmt_frm(pAd, pRxBlk);
            break;

        case FC_TYPE_CNTL:
            dev_rx_ctrl_frm(pAd, pRxBlk);
            break;

        default:
            RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
            break;
    }

#ifdef CONFIG_TRACE_SUPPORT
	if (FC->Type == FC_TYPE_DATA)
	{
		TRACE_RX0_DATA("rx0_data");
	}
	else if (FC->Type == FC_TYPE_MGMT)
	{
		TRACE_RX0_MGMT("rx0_mgmt");
	}
	else if (FC->Type == FC_TYPE_CNTL)
	{
		TRACE_RX0_CTRL("rx0_ctrl");

	}
#endif


    return NDIS_STATUS_SUCCESS;
}


BOOLEAN rtmp_rx_done_handle(RTMP_ADAPTER *pAd)
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	PNDIS_PACKET pRxPacket;
	RX_BLK rxblk, *pRxBlk;
#ifdef AIR_MONITOR
	MAC_TABLE_ENTRY *pEntry = NULL; 
	FRAME_CONTROL *fc = NULL;
#endif /* AIR_MONITOR */
	RTMP_RX_RING *pRxRing = &pAd->PciHif.RxRing[HIF_RX_IDX0];

#ifdef LINUX
#endif /* LINUX */

	RxProcessed = RxPending = 0;

	while (1)
	{
		if ((RTMP_TEST_FLAG(pAd, (
								fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
				FALSE/*(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))*/
			)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
		{
			break;
		}

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;
#endif /* ERR_RECOVERY */

#ifdef RTMP_MAC_PCI
#ifdef UAPSD_SUPPORT
		UAPSD_TIMING_RECORD_INDEX(RxProcessed);
#endif /* UAPSD_SUPPORT */

		if (RxProcessed++ > MAX_RX_PROCESS_CNT)
		{
#ifdef CUT_THROUGH_DBG
			RxProcessed--;
			if ((RxProcessed >= 1) && (RxProcessed <= 64))
				pAd->RxMaxProcessCntA++;
			else if ((RxProcessed >= 65) && (RxProcessed <= 128))
				pAd->RxMaxProcessCntB++;
			else if ((RxProcessed >= 129) && (RxProcessed <= 192))
				pAd->RxMaxProcessCntC++;
			else if ((RxProcessed >= 193) && (RxProcessed <= 256))
				pAd->RxMaxProcessCntD++;
#endif
			bReschedule = TRUE;
			break;
		}

#ifdef UAPSD_SUPPORT
		/* static rate also need NICUpdateFifoStaCounters() function. */
		/*if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)) */
		UAPSD_MR_SP_SUSPEND(pAd);
#endif /* UAPSD_SUPPORT */

		/*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		if (++pAd->FifoUpdateDone >= FIFO_STAT_READ_PERIOD)
		{
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateDone = 0;
		}
#endif /* RTMP_MAC_PCI */

		/*
			1. allocate a new data packet into rx ring to replace received packet
				then processing the received packet
			2. the callee must take charge of release of packet
			3. As far as driver is concerned, the rx packet must
				a. be indicated to upper layer or
				b. be released if it is discarded
		*/
		os_zero_mem(&rxblk,sizeof(RX_BLK));

		pRxBlk = &rxblk;

		pRxPacket = GetPacketFromRxRing(pAd, &pRxBlk, &bReschedule, &RxPending, 0);

		if (pRxPacket == NULL)
		{
#ifdef CUT_THROUGH_DBG
			if ((RxProcessed >= 1) && (RxProcessed <= 64))
				pAd->RxMaxProcessCntA++;
			else if ((RxProcessed >= 65) && (RxProcessed <= 128))
				pAd->RxMaxProcessCntB++;
			else if ((RxProcessed >= 129) && (RxProcessed <= 192))
				pAd->RxMaxProcessCntC++;
			else if ((RxProcessed >= 193) && (RxProcessed <= 256))
				pAd->RxMaxProcessCntD++;
#endif
#ifdef CUT_THROUGH_DBG
			pAd->RxDropPacket++;
#endif
			break;
		}

#ifdef AIR_MONITOR
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->MntEnable && !pRxBlk->pRxInfo->CipherErr && !pRxBlk->pRxInfo->Crc)
			{
				if (VALID_WCID(pRxBlk->wcid))
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
                else
                    pEntry = NULL;

				if (pEntry && IS_ENTRY_MONITOR(pEntry))
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, 
						("monitor pRxWI->wcid=%d\n", pRxBlk->wcid));
					
					Air_Monitor_Pkt_Report_Action(pAd, pRxBlk->wcid, pRxBlk);

					fc = (FRAME_CONTROL *)pRxBlk->FC;

					if (!((fc->Type == FC_TYPE_MGMT) && (fc->SubType == SUBTYPE_PROBE_REQ)))
                    {
    					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
    					continue;
                    }									
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* AIR_MONITOR */

        /* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
        if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        {
            if (pRxPacket)
            {
#ifdef CUT_THROUGH_DBG
				pAd->RxDropPacket++;
#endif
                RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
                continue;
            }
        }

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_CMD_RSP)) {
			RX_BLK_CLEAR_FLAG(pRxBlk, fRX_CMD_RSP);
			continue;
		}

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_RETRIEVE)) {
			RX_BLK_CLEAR_FLAG(pRxBlk, fRX_RETRIEVE);
			continue;
		}

		if (header_packet_process(pAd, pRxPacket, pRxBlk) != NDIS_STATUS_SUCCESS)
        {
            continue;
        }


#ifdef CUT_THROUGH
        if (CUT_THROUGH_RX_ENABL(pAd->PktTokenCb))
		{
			PKT_TOKEN_CB *PktTokenCB = (PKT_TOKEN_CB *)pAd->PktTokenCb;
			UINT8 Type;
			UINT32 Drop;
			RTMP_SEM_LOCK(&PktTokenCB->rx_order_notify_lock);
			Drop = cut_through_rx_drop(pAd->PktTokenCb, pRxBlk->token_id);
			if (cut_through_rx_mark_rxdone(PktTokenCB, pRxBlk->token_id) &&
					(cut_through_rx_in_order(PktTokenCB, pRxBlk->token_id) ||
						Drop))
			{

				pRxPacket = cut_through_rx_deq(PktTokenCB, pRxBlk->token_id, &Type);
				RTMP_SEM_UNLOCK(&PktTokenCB->rx_order_notify_lock);

				if (pRxPacket)
				{
					if (Drop)
					{
						RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_RESOURCES);
						continue;
					}
					else
					{
						rx_packet_process(pAd, pRxPacket, pRxBlk);
					}
				}
				else
				{
					continue;
				}
			}
			else
			{
				RTMP_SEM_UNLOCK(&PktTokenCB->rx_order_notify_lock);
				continue;
			}
		}
		else
#endif /* CUT_THROUGH */
		{
			rx_packet_process(pAd, pRxPacket, pRxBlk);
		}
    }

	if (pRxRing->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		pRxRing->sw_read_idx_inc = 0;
	}

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT

#ifdef RT_CFG80211_P2P_SUPPORT
        if (IS_PKT_OPMODE_AP(pRxBlk))
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
	{
		/* dont remove the function or UAPSD will fail */
		UAPSD_MR_SP_RESUME(pAd);
		UAPSD_SP_CloseInRVDone(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */

	return bReschedule;
}

