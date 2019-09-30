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
 * @name tx core API 
 * @{
 */


#include "rt_config.h"

BOOLEAN check_if_fragment(struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	PACKET_INFO pkt_info;
	UCHAR *src_buf_va;
	UINT32 src_buf_len, frag_sz, pkt_len;
	
	RTMP_QueryPacketInfo(pPacket, &pkt_info, &src_buf_va, &src_buf_len);
	
	pkt_len = pkt_info.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H;
	frag_sz = wlan_operate_get_frag_thld(wdev);
	frag_sz = frag_sz - LENGTH_802_11 - LENGTH_CRC;
	
	if (pkt_len < frag_sz)
		return FALSE;	
	else
		return TRUE;
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
INT wdev_tx_pkts(NDIS_HANDLE dev_hnd, PPNDIS_PACKET pkt_list, UINT pkt_cnt, struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)dev_hnd;
	PNDIS_PACKET pPacket;
	BOOLEAN allowToSend;
	UCHAR wcid = MAX_LEN_OF_MAC_TABLE;
	UINT Index;
#ifdef MWDS
    INT Ret = 0;
#endif
#ifdef CONFIG_FPGA_MODE
	BOOLEAN force_tx;
#endif /* CONFIG_FPGA_MODE */

	
	for (Index = 0; Index < pkt_cnt; Index++)
	{
		pPacket = pkt_list[Index];

   		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)
					|| !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY))
		{
			/* Drop send request since hardware is in reset state */
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
			/* Drop send request since hardware is in reset state */
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			continue;
		}
#endif /* ERR_RECOVERY */

#ifdef CONFIG_FPGA_MODE
		force_tx = FALSE;
		if (pAd->fpga_ctl.fpga_on & 0x1) {
			if (pAd->fpga_ctl.tx_kick_cnt > 0) {
				if (pAd->fpga_ctl.tx_kick_cnt < 0xffff)  {
					pAd->fpga_ctl.tx_kick_cnt--;
				}
				force_tx = TRUE;
			}
		}
#endif /* CONFIG_FPGA_MODE */

		if (((wdev->forbid_data_tx == 0)
#ifdef CONFIG_FPGA_MODE
			|| (force_tx == TRUE)
#endif /* CONFIG_FPGA_MODE */
			)
			&& (wdev->tx_pkt_allowed))
		{
			allowToSend = wdev->tx_pkt_allowed(pAd, wdev, pPacket, &wcid);
		}
		else if((wdev->forbid_data_tx != 0) && (wdev->wdev_type == WDEV_TYPE_STA)
			&& (wdev->tx_pkt_allowed))
		{
			allowToSend = wdev->tx_pkt_allowed(pAd, wdev, pPacket, &wcid);
		}
		else
		{
			allowToSend = FALSE;
			//RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}

#ifdef CONFIG_FPGA_MODE
		if (force_tx == TRUE && allowToSend == TRUE) {
			//int dump_len = GET_OS_PKT_LEN(pPacket);
			
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
						("%s():send Packet!wcid=%d, wdev_idx=%d, pktLen=%d\n",
						__FUNCTION__, wcid, wdev->wdev_idx, GET_OS_PKT_LEN(pPacket)));
			//hex_dump("wdev_tx_pkts():802.3 packet", GET_OS_PKT_DATAPTR(pPacket), dump_len > 255 ? 255 : dump_len);
		}
#endif /* CONFIG_FPGA_MODE */

		if (allowToSend == TRUE)
		{
			RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);

			/*
				WIFI HNAT need to learn packets going to which interface from skb cb setting.
				@20150325
			*/
#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
	        if (ra_sw_nat_hook_tx != NULL)
			{
#ifdef TCSUPPORT_MT7510_FE
	            if (ra_sw_nat_hook_tx(pPacket, NULL, FOE_MAGIC_WLAN) == 0)
#else
	            if (ra_sw_nat_hook_tx(pPacket, 1) == 0)
#endif
				{
	                RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	                    return 0;
	            }
	        }
#endif

#ifdef CONFIG_RAETH
#if !defined(CONFIG_RA_NAT_NONE)
			if(ra_sw_nat_hook_tx!= NULL)
			{
				unsigned long flags;
		
				RTMP_IRQ_LOCK(&pAd->page_lock, flags);
				ra_sw_nat_hook_tx(pPacket, 0);
				RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
			}
#endif
#endif /* CONFIG_RAETH */			
		


#ifdef CUT_THROUGH
#ifdef CUT_THROUGH_FULL_OFFLOAD
			if (wdev->tx_pkt_ct_handle && !check_if_fragment(wdev, pPacket))
			{
#ifndef MWDS
				INT32 Ret = 0;
#endif
				UCHAR QueIdx = 0, UserPriority = QID_AC_BE;
				STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];
				RTMP_SET_PACKET_WCID(pPacket, wcid);

#ifdef RT_CFG80211_SUPPORT			
				if(RTMP_CFG80211_HOSTAPD_ON(pAd))
				{
					UCHAR *pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
					UINT16 TypeLen = 0;
					if(pSrcBuf)
					{
						TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
						if(TypeLen == ETH_TYPE_EAPOL)
						{
							MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("%s, %u send EAPOL from hostapd\n", __FUNCTION__, __LINE__));
						}
					}
				}
#endif /* RT_CFG80211_SUPPORT */

				if (!pAd->chipCap.BATriggerOffload)
				{
					if (!RTMPCheckEtherType(pAd, pPacket, tr_entry, wdev, &UserPriority, &QueIdx)) 
					{
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						return 0;
					}
				}

#ifdef TRACELOG_TCP_PKT
				if (RTMPIsTcpDataPkt(pPacket))
					pAd->u4TcpTxDataCnt++;
#endif

				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
							("%s():go through wdev->tx_pkt_ct_handle!\n", __FUNCTION__));
#ifdef RTMP_UDMA_SUPPORT
#endif /*RTMP_UDMA_SUPPORT*/

#ifdef REDUCE_TCP_ACK_SUPPORT
				if (ReduceTcpAck(pAd, pPacket) == FALSE)
#endif /* REDUCE_TCP_ACK_SUPPORT */
				{
					Ret = wdev->tx_pkt_ct_handle(pAd, pPacket, QueIdx, UserPriority);
#ifdef MWDS
                    if(wdev->wdev_type != WDEV_TYPE_APCLI)
                    {
                        UCHAR *pDestAddr = GET_OS_PKT_DATAPTR(pPacket);
                        if (pDestAddr && 
                            MAC_ADDR_IS_GROUP(pDestAddr) &&
                            (Ret == NDIS_STATUS_SUCCESS))
                            MWDSSendClonePacket(pAd, wdev->func_idx, pPacket, pDestAddr + MAC_ADDR_LEN);
                    }
#endif /* MWDS */
				}

				return Ret;
			}
			else
#endif /* CUT_THROUGH_FULL_OFFLOAD */
#endif /* CUT_THROUGH */
			if (wdev->tx_pkt_handle)
			{
				RTMP_SET_PACKET_WCID(pPacket, wcid);
				NDIS_SET_PACKET_STATUS(pPacket, NDIS_STATUS_PENDING);
				pAd->RalinkCounters.PendingNdisPacketCount++;
#ifdef REDUCE_TCP_ACK_SUPPORT
				if (ReduceTcpAck(pAd, pPacket) == FALSE)
#endif
				{
#ifndef MWDS
					wdev->tx_pkt_handle(pAd, pPacket);
#else
					Ret = wdev->tx_pkt_handle(pAd, pPacket);
#endif

#ifdef CONFIG_AP_SUPPORT
					IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
					{
#ifdef MWDS
						UCHAR *pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
						if (pSrcBufVA && 
							MAC_ADDR_IS_GROUP(pSrcBufVA) &&
							(Ret == NDIS_STATUS_SUCCESS))
							MWDSSendClonePacket(pAd, wdev->func_idx, pPacket, NULL);
#endif /* MWDS */
					}
#endif
				}
			}
			else
			{
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():tx_pkt_handle not assigned!\n", __FUNCTION__));
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
		}
		else
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}

	RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);

	return 0;
}

#ifdef TX_AGG_ADJUST_WKR
BOOLEAN tx_check_for_agg_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
        BOOLEAN check_result = FALSE;
	BOOLEAN support_four_stream = FALSE;

	if (pAd->TxAggAdjsut == FALSE)
		return FALSE;

	if (!pEntry)
		return FALSE;

	if (pEntry->vendor_ie.is_rlt == TRUE ||
		pEntry->vendor_ie.is_mtk == TRUE){
		return FALSE;		
	}

	if ((pEntry->SupportHTMCS & 0xff000000) != 0 )
		support_four_stream = TRUE;
	
	if (pEntry->SupportVHTMCS4SS != 0 )
		support_four_stream = TRUE;

	if (support_four_stream == FALSE)
		check_result = TRUE;


	return check_result;
}
#endif /* TX_AGG_ADJUST_WKR */

/** @} */
/** @} */
