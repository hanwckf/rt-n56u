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
     sync.c

     Abstract:
     Synchronization state machine related services

     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
     John Chang  08-04-2003    created for 11g soft-AP

 */

#include "rt_config.h"

#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP
extern NDIS_SPIN_LOCK ez_conn_perm_lock;
#endif
#endif
#define OBSS_BEACON_RSSI_THRESHOLD		(-85)


/*
	==========================================================================
	Description:
		Process the received ProbeRequest from clients
	Parameters:
		Elem - msg containing the ProbeReq frame
	==========================================================================
 */
VOID APPeerProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PEER_PROBE_REQ_PARAM ProbeReqParam = { {0} };
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, TmpLen;
	LARGE_INTEGER FakeTimestamp;
	UCHAR DsLen = 1;
	UCHAR ErpIeLen = 1;
	UCHAR apidx = 0, PhyMode, SupRateLen;
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;
#ifdef BAND_STEERING
	BOOLEAN bBndStrgCheck = TRUE;
	BOOLEAN bAllowStaConnectInHt = FALSE;
	BOOLEAN bVHTCap = FALSE;
	UINT8 Nss;
#endif /* BAND_STEERING */
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
    UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif
	CHAR rsne_idx = 0;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	ADD_HT_INFO_IE *addht;
	UCHAR cfg_ht_bw;	
	UCHAR op_ht_bw;

#ifdef CONFIG_HOTSPOT_R2		
	extern UCHAR			OSEN_IE[];
	extern UCHAR			OSEN_IELEN;
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef WH_EZ_SETUP
	BOOLEAN ez_peer = FALSE;
#endif /* WH_EZ_SETUP */

#ifdef WSC_AP_SUPPORT
	UCHAR Addr3[MAC_ADDR_LEN];
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;

	COPY_MAC_ADDR(Addr3, pFrame->Hdr.Addr3);
#endif /* WSC_AP_SUPPORT */

#ifdef WDS_SUPPORT
	/* if in bridge mode, no need to reply probe req. */
	if (pAd->WdsTab.Mode == WDS_BRIDGE_MODE)
		return;
#endif /* WDS_SUPPORT */


	if (PeerProbeReqSanity(pAd, Elem->Msg, Elem->MsgLen, &ProbeReqParam) == FALSE) {
MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():shiang! PeerProbeReqSanity failed!\n", __FUNCTION__));
		return;
	}


	for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		mbss = &pAd->ApCfg.MBSSID[apidx];
		wdev = &mbss->wdev;
		rate = &wdev->rate;
		addht = wlan_operate_get_addht(wdev);
		cfg_ht_bw = wlan_config_get_ht_bw(wdev);
		op_ht_bw = wlan_config_get_ht_bw(wdev);

		if ((wdev->if_dev == NULL) || ((wdev->if_dev != NULL) &&
			!(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev))))
		{
			/* the interface is down, so we can not send probe response */
			continue;
		}

		if(!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, AP (wdev_idx %d) is not ready\n", __FUNCTION__, wdev->wdev_idx));
			continue;
		}
		
		if (Elem->Channel != wdev->channel) {
			continue;
		}

		PhyMode = wdev->PhyMode;

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev)) {
#ifdef EZ_ROAM_SUPPORT
#ifdef EZ_MOD_SUPPORT
			unsigned ez_probe_req_action = 0;
			if (ez_is_roam_blocked_mac(wdev, ProbeReqParam.Addr2))
#else	
			if(MAC_ADDR_EQUAL(wdev->ez_security.ez_ap_roam_blocked_mac,ProbeReqParam.Addr2))
#endif
			{
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" Connection not allowed as roaming ongoing.\n"));
				continue;
			}
#endif
#ifdef EZ_MOD_SUPPORT
			ez_probe_req_action = ez_process_probe_request(pAd, wdev, ProbeReqParam.Addr2, Elem->Msg, Elem->MsgLen);
			if (ez_probe_req_action == 1)
#else
			if (ez_process_probe_request(pAd, wdev, ProbeReqParam.Addr2, Elem->Msg, Elem->MsgLen))
#endif
			{
				ez_peer = TRUE;
				EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("Easy Setup Peer - %02x:%02x:%02x:%02x:%02x:%02x\n", 
					ProbeReqParam.Addr2[0],
					ProbeReqParam.Addr2[1],
					ProbeReqParam.Addr2[2],
					ProbeReqParam.Addr2[3],
					ProbeReqParam.Addr2[4],
					ProbeReqParam.Addr2[5]));
#ifndef EZ_MOD_SUPPORT
#ifdef NEW_CONNECTION_ALGO
				if(ez_is_connection_allowed(wdev) == FALSE){
					ez_peer_table_delete(wdev, ProbeReqParam.Addr2);
					EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" Connection not allowd\n"));
					continue;
				}
#endif
#endif
			}
#ifdef EZ_MOD_SUPPORT
			else if (ez_probe_req_action == 2)
			{
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" Connection not allowd\n"));
				continue;
			} 
#endif
			else
			{
			
#ifdef WH_EZ_SETUP	
				//! if ez_setup is enabled on this wdev and and it is a triband repeater than this AP should accept only EZ connections		
				if(ez_is_triband() && IS_EZ_SETUP_ENABLED(wdev)) {
					if (!ez_check_for_ez_enable(wdev, Elem->Msg, Elem->MsgLen))
					{
					} else {
						continue;
					}
				}
#endif	
			//	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_process_probe_request returned false\n"));
			}
		}
		else{
		//	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_process_probe_request not entered: %d %d\n", wdev->wdev_idx, wdev->enable_easy_setup));
		}
#endif /* WH_EZ_SETUP */

		if ( ((((ProbeReqParam.SsidLen == 0) && (!mbss->bHideSsid)) ||
			   ((ProbeReqParam.SsidLen == mbss->SsidLen) && NdisEqualMemory(ProbeReqParam.Ssid, mbss->Ssid, (ULONG) ProbeReqParam.SsidLen)))
#ifdef CONFIG_HOTSPOT
			   && ProbeReqforHSAP(pAd, apidx, &ProbeReqParam)
#endif
			 )
#ifdef WSC_AP_SUPPORT
            /* buffalo WPS testbed STA send ProbrRequest ssid length = 32 and ssid are not AP , but DA are AP. for WPS test send ProbeResponse */
			|| ((ProbeReqParam.SsidLen == 32) && MAC_ADDR_EQUAL(Addr3, wdev->bssid) && (mbss->bHideSsid == 0))
#endif /* WSC_AP_SUPPORT */
		)
		{
			;
		}
		else {
#ifdef WH_EZ_SETUP
			if (!ez_peer)
#endif /* WH_EZ_SETUP */
				continue; /* check next BSS */
		}


#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering
#ifdef WH_EZ_SETUP
		&& !((wdev != NULL) && (IS_EZ_SETUP_ENABLED(wdev)) && (ez_peer == TRUE))
#endif
	) {
		if (ProbeReqParam.IsHtSupport && WMODE_CAP_N(wdev->PhyMode))
			bAllowStaConnectInHt = TRUE;
		if(ProbeReqParam.IsVhtSupport && WMODE_CAP_AC(wdev->PhyMode))
			bVHTCap = TRUE;
		Nss = GetNssFromHTCapRxMCSBitmask(ProbeReqParam.RxMCSBitmask);
		BND_STRG_CHECK_CONNECTION_REQ(	pAd,
											wdev, 
											ProbeReqParam.Addr2,
											Elem->MsgType,
											Elem->rssi_info,
											bAllowStaConnectInHt,
											bVHTCap,
											Nss,
											&bBndStrgCheck);
		if (bBndStrgCheck == FALSE)
			return;
	}
#endif /* BAND_STEERING */

#ifdef STA_FORCE_ROAM_SUPPORT
	// Enhancement: Block probe Response to a peer on acl list to avoid conenct attempts by peer.
    if (pAd->en_force_roam_supp &&
		! ApCheckAccessControlList(pAd, ProbeReqParam.Addr2, apidx)){
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("ACL reject Peer - %02x:%02x:%02x:%02x:%02x:%02x, block Probe Response.\n",
					PRINT_MAC(ProbeReqParam.Addr2)));
		return;
    }
#endif

#ifdef WH_EVENT_NOTIFIER
        {
            EventHdlr pEventHdlrHook = NULL;
            pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_PROBE_REQ);
            if(pEventHdlrHook && wdev)
                pEventHdlrHook(pAd, wdev, &ProbeReqParam, Elem);
        }
#endif /* WH_EVENT_NOTIFIER */

		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, ProbeReqParam.Addr2,
							wdev->if_addr, wdev->bssid);

		{
		SupRateLen = rate->SupRateLen;
		if (PhyMode == WMODE_B)
			SupRateLen = 4;

		MakeOutgoingFrame(pOutBuffer,                 &FrameLen,
						  sizeof(HEADER_802_11),      &ProbeRspHdr,
						  TIMESTAMP_LEN,              &FakeTimestamp,
						  2,                          &pAd->CommonCfg.BeaconPeriod,
						  2,                          &mbss->CapabilityInfo,
						  1,                          &SsidIe,
						  1,                          &mbss->SsidLen,
						  mbss->SsidLen,     mbss->Ssid,
						  1,                          &SupRateIe,
						  1,                          &SupRateLen,
						  SupRateLen,                 rate->SupRate,
						  1,                          &DsIe,
						  1,                          &DsLen,
						  1,                          &wdev->channel,
						  END_OF_ARGS);
		}

		if ((rate->ExtRateLen) && (PhyMode != WMODE_B))
		{
			MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							  1,                        &ErpIe,
							  1,                        &ErpIeLen,
							  1,                        &pAd->ApCfg.ErpIeContent,
							  1,                        &ExtRateIe,
							  1,                        &rate->ExtRateLen,
							  rate->ExtRateLen,    		rate->ExtRate,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) &&
			(wdev->DesiredHtPhyInfo.bHtEnable))
		{
			ULONG TmpLen;
			UCHAR	HtLen, AddHtLen, NewExtLen;
			HT_CAPABILITY_IE HtCapabilityTmp;
#ifdef RT_BIG_ENDIAN
			ADD_HT_INFO_IE	addHTInfoTmp;
#endif

/* YF@20120419: Fix IOT Issue with Atheros STA on Windows 7 When IEEE80211H flag turn on. */

			HtLen = sizeof(pAd->CommonCfg.HtCapability);
			AddHtLen = sizeof(ADD_HT_INFO_IE);
			NewExtLen = 1;
			/*New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */
#ifndef RT_BIG_ENDIAN
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);

#ifdef TXBF_SUPPORT
            if (HcIsBfCapSupport(wdev) == FALSE)
            {
                UCHAR ucEBfCap;

                ucEBfCap = pAd->CommonCfg.ETxBfEnCond;
                pAd->CommonCfg.ETxBfEnCond = 0;
            
                mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);

                pAd->CommonCfg.ETxBfEnCond = ucEBfCap;
            }
#endif /* TXBF_SUPPORT */ 

			HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;

			MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
							  1,                                &HtCapIe,
							  1,                                &HtLen,
							 sizeof(HT_CAPABILITY_IE),          &HtCapabilityTmp,
							  1,                                &AddHtInfoIe,
							  1,                                &AddHtLen,
							 sizeof(ADD_HT_INFO_IE),          addht,
							  END_OF_ARGS);
#else
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
			HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;

#ifdef TXBF_SUPPORT
            if (HcIsBfCapSupport(wdev) == FALSE)
            {
                UCHAR ucEBfCap;

                ucEBfCap = pAd->CommonCfg.ETxBfEnCond;
                pAd->CommonCfg.ETxBfEnCond = 0;
            
                mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);

                pAd->CommonCfg.ETxBfEnCond = ucEBfCap;
            }
#endif /* TXBF_SUPPORT */ 

			*(UINT32 *)(&HtCapabilityTmp.TxBFCap) = cpu2le32(*(UINT32 *)(&HtCapabilityTmp.TxBFCap));
			HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
			{
				EXT_HT_CAP_INFO extHtCapInfo;

				NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
				NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
			}
#else
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

			NdisMoveMemory(&addHTInfoTmp, addht, AddHtLen);
			*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
			*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

			MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
								1,                           &HtCapIe,
								1,                           &HtLen,
								HtLen,                       &HtCapabilityTmp,
								1,                           &AddHtInfoIe,
								1,                           &AddHtLen,
								AddHtLen,                    &addHTInfoTmp,
								END_OF_ARGS);

#endif
			FrameLen += TmpLen;
		}
#endif /* DOT11_N_SUPPORT */

		/* Append RSN_IE when  WPA OR WPAPSK, */
		pSecConfig = &wdev->SecConfig;
		
#ifdef CONFIG_HOTSPOT_R2
		if ((mbss->HotSpotCtrl.HotSpotEnable == 0) && (mbss->HotSpotCtrl.bASANEnable == 1)&& (IS_AKM_WPA2_Entry(wdev)))
		{
			/* replace RSN IE with OSEN IE if it's OSEN wdev */
			UCHAR RSNIe = IE_WPA;			
			MakeOutgoingFrame(pOutBuffer+FrameLen,			&TmpLen,
							  1,							&RSNIe,
							  1,							&OSEN_IELEN,
							  OSEN_IELEN,					OSEN_IE,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
		else
#endif /* CONFIG_HOTSPOT_R2 */
		{
			for (rsne_idx=0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++)
			{
				if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;
		
				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
					1, &pSecConfig->RSNE_EID[rsne_idx][0],
					1, &pSecConfig->RSNE_Len[rsne_idx],
					pSecConfig->RSNE_Len[rsne_idx], &pSecConfig->RSNE_Content[rsne_idx][0],
					END_OF_ARGS);

				FrameLen += TmpLen;
			}
		}
#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
		if(pAd->ApCfg.MBSSID[apidx].GASCtrl.b11U_enable)
		{
			ULONG TmpLen;
			/* Interworking element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.InterWorkingIELen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.InterWorkingIE, END_OF_ARGS);

			FrameLen += TmpLen;

			/* Advertisement Protocol element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.AdvertisementProtoIELen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.AdvertisementProtoIE, END_OF_ARGS);

			FrameLen += TmpLen;
		}
#endif /* defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT) */

#ifdef CONFIG_HOTSPOT
		if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HotSpotEnable)
		{
			ULONG TmpLen;
				
			/* Hotspot 2.0 Indication */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HSIndicationIELen, 
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HSIndicationIE, END_OF_ARGS);

			FrameLen += TmpLen;	

			/* Roaming Consortium element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.RoamingConsortiumIELen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.RoamingConsortiumIE, END_OF_ARGS);

			FrameLen += TmpLen;

			/* P2P element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.P2PIELen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.P2PIE, END_OF_ARGS);

			FrameLen += TmpLen;
		}
#endif

		/* Extended Capabilities IE */
		{
			ULONG TmpLen, infoPos;
			PUCHAR pInfo;
			BOOLEAN bNeedAppendExtIE = FALSE;
			EXT_CAP_INFO_ELEMENT extCapInfo = { 0 };
			UCHAR extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);

			//NdisZeroMemory(&extCapInfo, extInfoLen);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			/* P802.11n_D1.10, HT Information Exchange Support */
			if ((pAd->ApCfg.MBSSID[apidx].wdev.PhyMode >= PHY_11ABGN_MIXED) && (pAd->ApCfg.MBSSID[apidx].wdev.channel <= 14) &&
				(pAd->ApCfg.MBSSID[apidx].wdev.DesiredHtPhyInfo.bHtEnable) &&
				(pAd->CommonCfg.bBssCoexEnable == TRUE))
			{
				extCapInfo.BssCoexistMgmtSupport = 1;
			}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.ProxyARPEnable)
				extCapInfo.proxy_arp = 1;
			
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.WNMBTMEnable)
				extCapInfo.BssTransitionManmt = 1;
#ifdef CONFIG_HOTSPOT_R2
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.WNMNotifyEnable)
				extCapInfo.wnm_notification= 1;
			if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QosMapEnable)
				extCapInfo.qosmap= 1;
#endif
#endif

#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
			if(pAd->ApCfg.MBSSID[apidx].GASCtrl.b11U_enable)
				extCapInfo.interworking = 1;
#endif


			pInfo = (PUCHAR)(&extCapInfo);
			for (infoPos = 0; infoPos < extInfoLen; infoPos++)
			{
				if (pInfo[infoPos] != 0)
				{
					bNeedAppendExtIE = TRUE;
					break;
				}
			}
			
			if (bNeedAppendExtIE == TRUE)
			{
				for (infoPos = (extInfoLen - 1); infoPos >= EXT_CAP_MIN_SAFE_LENGTH; infoPos--)
				{
					if (pInfo[infoPos] == 0)
						extInfoLen --;
					else
						break;
				}
			
				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
								1, &ExtCapIe,
								1, &extInfoLen,
								extInfoLen, &extCapInfo,
								END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}

#ifdef AP_QLOAD_SUPPORT
		if (pQloadCtrl->FlgQloadEnable != 0)
		{
#ifdef CONFIG_HOTSPOT_R2
			if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadTestEnable == 1)
				FrameLen += QBSS_LoadElementAppend_HSTEST(pAd, pOutBuffer+FrameLen, apidx);
			else if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadTestEnable == 0)
#endif
			FrameLen += QBSS_LoadElementAppend(pAd, pOutBuffer+FrameLen);
		}
#endif /* AP_QLOAD_SUPPORT */

		/* add WMM IE here */
		if (wdev->bWmmCapable)
		{
			UCHAR i;
			UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};
			struct _EDCA_PARM *pBssEdca = wlan_config_get_ht_edca(wdev);

			if (pBssEdca)
			{
				WmeParmIe[8] = pBssEdca->EdcaUpdateCount & 0x0f;

#ifdef UAPSD_SUPPORT
				UAPSD_MR_IE_FILL(WmeParmIe[8], &wdev->UapsdInfo);
#endif /* UAPSD_SUPPORT */
				for (i=QID_AC_BE; i<=QID_AC_VO; i++)
				{
					WmeParmIe[10+ (i*4)] = (i << 5) + /* b5-6 is ACI */
										   ((UCHAR)pBssEdca->bACM[i] << 4) +     /* b4 is ACM */
										   (pBssEdca->Aifsn[i] & 0x0f);		/* b0-3 is AIFSN */
					WmeParmIe[11+ (i*4)] = (pBssEdca->Cwmax[i] << 4) +	/* b5-8 is CWMAX */
										   (pBssEdca->Cwmin[i] & 0x0f);	/* b0-3 is CWMIN */
					WmeParmIe[12+ (i*4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);        /* low byte of TXOP */
					WmeParmIe[13+ (i*4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);          /* high byte of TXOP */
				}

				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
								  26,                       WmeParmIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}

#ifdef DOT11K_RRM_SUPPORT
		if (IS_RRM_ENABLE(pAd, apidx))
			RRM_InsertRRMEnCapIE(pAd, pOutBuffer+FrameLen, &FrameLen, apidx);

		InsertChannelRepIE(pAd, pOutBuffer+FrameLen, &FrameLen,
						(RTMP_STRING *)pAd->CommonCfg.CountryCode,
						get_regulatory_class(pAd,mbss->wdev.channel,mbss->wdev.PhyMode, &mbss->wdev),
						NULL, PhyMode);	

#ifndef APPLE_11K_IOT
		/* Insert BSS AC Access Delay IE. */
		RRM_InsertBssACDelayIE(pAd, pOutBuffer+FrameLen, &FrameLen);

		/* Insert BSS Available Access Capacity IE. */
		RRM_InsertBssAvailableACIE(pAd, pOutBuffer+FrameLen, &FrameLen);
#endif /* !APPLE_11K_IOT */

#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	 	/* P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE */
	 	if (WMODE_CAP_N(PhyMode) &&
			(wdev->channel <= 14) &&
			(wdev->DesiredHtPhyInfo.bHtEnable) &&
			(cfg_ht_bw == HT_BW_40))
	 	{
			OVERLAP_BSS_SCAN_IE  OverlapScanParam;
			ULONG	TmpLen;
			UCHAR	OverlapScanIE, ScanIELen;

			OverlapScanIE = IE_OVERLAPBSS_SCAN_PARM;
			ScanIELen = 14;
			OverlapScanParam.ScanPassiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveDwell);
			OverlapScanParam.ScanActiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveDwell);
			OverlapScanParam.TriggerScanInt = cpu2le16(pAd->CommonCfg.Dot11BssWidthTriggerScanInt);
			OverlapScanParam.PassiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel);
			OverlapScanParam.ActiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel);
			OverlapScanParam.DelayFactor = cpu2le16(pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
			OverlapScanParam.ScanActThre = cpu2le16(pAd->CommonCfg.Dot11OBssScanActivityThre);

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1,			&OverlapScanIE,
								1,			&ScanIELen,
								ScanIELen,	&OverlapScanParam,
								END_OF_ARGS);

			FrameLen += TmpLen;
	 	}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	    /* add Country IE and power-related IE */
		if (pAd->CommonCfg.bCountryFlag ||
			(wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
				|| IS_RRM_ENABLE(pAd, apidx)
#endif /* DOT11K_RRM_SUPPORT */
			)
		{
			ULONG TmpLen2 = 0;
			UCHAR TmpFrame[256] = { 0 };
			UCHAR CountryIe = IE_COUNTRY;		
			PCH_DESC pChDesc = NULL;
			
			if (WMODE_CAP_2G(wdev->PhyMode)) {
				if (pAd->CommonCfg.pChDesc2G != NULL) 
					pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc2G; 
				else
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
						("%s: pChDesc2G is NULL !!!\n", __FUNCTION__));
			} else if (WMODE_CAP_5G(wdev->PhyMode)) {
				if (pAd->CommonCfg.pChDesc5G!= NULL) 
					pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc5G;
				else
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
						("%s: pChDesc5G is NULL !!!\n", __FUNCTION__));  
			}

			/*
				Only APs that comply with 802.11h or 802.11k are required to include 
				the Power Constraint element (IE=32) and 
				the TPC Report element (IE=35) and
				the VHT Transmit Power Envelope element (IE=195)
				in beacon frames and probe response frames
			*/
			if ((wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
					|| IS_RRM_ENABLE(pAd, apidx)
#endif /* DOT11K_RRM_SUPPORT */
				)				
			{			
				/* prepare power constraint IE */
				MakeOutgoingFrame(pOutBuffer+FrameLen,    &TmpLen,
						3,                 	PowerConstraintIE,
						END_OF_ARGS);
						FrameLen += TmpLen;

				/* prepare TPC Report IE */
				InsertTpcReportIE(pAd, 
					pOutBuffer+FrameLen, 
					&FrameLen,
					GetMaxTxPwr(pAd), 
					0);						

#ifdef DOT11_VHT_AC
				/* prepare VHT Transmit Power Envelope IE */
				if (WMODE_CAP_AC(PhyMode)) {
					ULONG TmpLen;
					UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
					UINT8 ie_len;
					VHT_TXPWR_ENV_IE txpwr_env;

					ie_len = build_vht_txpwr_envelope(pAd,wdev,(UCHAR *)&txpwr_env);
					MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1,							&vht_txpwr_env_ie,
								1,							&ie_len,
								ie_len, 					&txpwr_env,
								END_OF_ARGS);
					FrameLen += TmpLen;
				}
#endif /* DOT11_VHT_AC */
			}

			//NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
			
#ifdef EXT_BUILD_CHANNEL_LIST
			BuildBeaconChList(pAd, wdev, TmpFrame, &TmpLen2);
#else		
			{
				UINT i = 0;         
				UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, wdev, wdev->PhyMode, wdev->channel, op_ht_bw);
			
	            for (i=0; pChDesc[i].FirstChannel!=0; i++)
	            {
	                 MakeOutgoingFrame(TmpFrame+TmpLen2,
					 	&TmpLen,
					 	1,      
					 	&pChDesc[i].FirstChannel,
						1,       
						&pChDesc[i].NumOfCh,
	                  	1,         
	                  	&MaxTxPower,
						END_OF_ARGS);
					 TmpLen2 += TmpLen;
	            }
			}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DOT11K_RRM_SUPPORT
			if (IS_RRM_ENABLE(pAd, apidx))
			{
				UCHAR reg_class = get_regulatory_class(pAd,mbss->wdev.channel,mbss->wdev.PhyMode,&mbss->wdev);
				TmpLen2 = 0;
				NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
				RguClass_BuildBcnChList(pAd, TmpFrame, &TmpLen2, wdev->PhyMode, reg_class);
			}
#endif /* DOT11K_RRM_SUPPORT */

			/* need to do the padding bit check, and concatenate it */
			if ((TmpLen2%2) == 0)
			{
				UCHAR TmpLen3 = TmpLen2 + 4;
				MakeOutgoingFrame(pOutBuffer+FrameLen, 
					&TmpLen,
					1,                 	
					&CountryIe,
					1,                 	
					&TmpLen3,
					3,                 	
					pAd->CommonCfg.CountryCode,
					TmpLen2+1,				
					TmpFrame,
					END_OF_ARGS);
			}
			else
			{
				UCHAR TmpLen3 = TmpLen2 + 3;
				MakeOutgoingFrame(pOutBuffer+FrameLen, 
					&TmpLen,
					1,                 	
					&CountryIe,
					1,                 	
					&TmpLen3,
					3,                 	
					pAd->CommonCfg.CountryCode,
					TmpLen2,				
					TmpFrame,
					END_OF_ARGS);
			}
			FrameLen += TmpLen;
		}/* Country IE - */

#ifdef A_BAND_SUPPORT
		/* add Channel switch announcement IE */
		if ((wdev->channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		{
			UCHAR CSAIe=IE_CHANNEL_SWITCH_ANNOUNCEMENT;
			UCHAR CSALen=3;
			UCHAR CSAMode=1;

			MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							  1,                        &CSAIe,
							  1,                        &CSALen,
							  1,                        &CSAMode,
							  1,                        &wdev->channel,
							  1,                        &pAd->Dot11_H.CSCount,
							  END_OF_ARGS);
			FrameLen += TmpLen;
#ifdef DOT11_N_SUPPORT
   			if (pAd->CommonCfg.bExtChannelSwitchAnnouncement)
			{
				HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE HtExtChannelSwitchIe;

				build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe, wdev->channel, wdev->PhyMode,wdev);
				MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								  sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE),	&HtExtChannelSwitchIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
#endif /* DOT11_N_SUPPORT */
		}
#endif /* A_BAND_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) &&
			(wdev->DesiredHtPhyInfo.bHtEnable))
		{
			ULONG TmpLen;
			UCHAR	HtLen, AddHtLen;/*, NewExtLen; */
#ifdef RT_BIG_ENDIAN
			HT_CAPABILITY_IE HtCapabilityTmp;
			ADD_HT_INFO_IE	addHTInfoTmp;
#endif
#ifdef DOT11_VHT_AC
            struct _build_ie_info vht_ie_info;
#endif /*DOT11_VHT_AC*/

			HtLen = sizeof(pAd->CommonCfg.HtCapability);
			AddHtLen = sizeof(ADD_HT_INFO_IE);

		if (pAd->bBroadComHT == TRUE)
		{
			UCHAR epigram_ie_len;
			UCHAR BROADCOM_HTC[4] = {0x0, 0x90, 0x4c, 0x33};
			UCHAR BROADCOM_AHTINFO[4] = {0x0, 0x90, 0x4c, 0x34};


			epigram_ie_len = HtLen + 4;
#ifndef RT_BIG_ENDIAN
			MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
						  1,                                &WpaIe,
							  1,                                &epigram_ie_len,
							  4,                                &BROADCOM_HTC[0],
							  HtLen,          					&pAd->CommonCfg.HtCapability,
							  END_OF_ARGS);
#else
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);

#ifdef TXBF_SUPPORT
            if (HcIsBfCapSupport(wdev) == FALSE)
            {
                UCHAR ucEBfCap;

                ucEBfCap = pAd->CommonCfg.ETxBfEnCond;
                pAd->CommonCfg.ETxBfEnCond = 0;
            
                mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);

                pAd->CommonCfg.ETxBfEnCond = ucEBfCap;
            }
#endif /* TXBF_SUPPORT */

			*(UINT32 *)(&HtCapabilityTmp.TxBFCap) = cpu2le32(*(UINT32 *)(&HtCapabilityTmp.TxBFCap));
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
			{
				EXT_HT_CAP_INFO extHtCapInfo;

				NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
				NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
			}
#else
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

				MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
								1,                               &WpaIe,
								1,                               &epigram_ie_len,
								4,                               &BROADCOM_HTC[0],
								HtLen,                           &HtCapabilityTmp,
								END_OF_ARGS);
#endif

				FrameLen += TmpLen;

				epigram_ie_len = AddHtLen + 4;
#ifndef RT_BIG_ENDIAN
				MakeOutgoingFrame(pOutBuffer + FrameLen,          &TmpLen,
								  1,                              &WpaIe,
								  1,                              &epigram_ie_len,
								  4,                              &BROADCOM_AHTINFO[0],
								  AddHtLen, 					  addht,
								  END_OF_ARGS);
#else
				NdisMoveMemory(&addHTInfoTmp, addht, AddHtLen);
				*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
				*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

				MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
								1,                               &WpaIe,
								1,                               &epigram_ie_len,
								4,                               &BROADCOM_AHTINFO[0],
								AddHtLen,                        &addHTInfoTmp,
							  END_OF_ARGS);
#endif

				FrameLen += TmpLen;
			}

#ifdef DOT11_VHT_AC
            vht_ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
            vht_ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
            vht_ie_info.channel = wdev->channel;
            vht_ie_info.phy_mode = wdev->PhyMode;
	    vht_ie_info.wdev = wdev;

#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
        ucETxBfCap = pAd->CommonCfg.ETxBfEnCond;
        if (HcIsBfCapSupport(wdev) == FALSE)
        {
            pAd->CommonCfg.ETxBfEnCond = 0;
        }
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */ 

            FrameLen += build_vht_ies(pAd, &vht_ie_info);

#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		pAd->CommonCfg.ETxBfEnCond = ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */ 
#endif /* DOT11_VHT_AC */

		}
#endif /* DOT11_N_SUPPORT */


#ifdef WSC_AP_SUPPORT
		/* for windows 7 logo test */
		if ((mbss->WscControl.WscConfMode != WSC_DISABLE) &&
#ifdef DOT1X_SUPPORT
				(!IS_IEEE8021X_Entry(wdev)) &&
#endif /* DOT1X_SUPPORT */
				(IS_CIPHER_WEP(wdev->SecConfig.PairwiseCipher)))
		{
			/*
				Non-WPS Windows XP and Vista PCs are unable to determine if a WEP enalbed network is static key based
				or 802.1X based. If the legacy station gets an EAP-Rquest/Identity from the AP, it assume the WEP
				network is 802.1X enabled & will prompt the user for 802.1X credentials. If the legacy station doesn't
				receive anything after sending an EAPOL-Start, it will assume the WEP network is static key based and
				prompt user for the WEP key. <<from "WPS and Static Key WEP Networks">>
				A WPS enabled AP should include this IE in the beacon when the AP is hosting a static WEP key network.
				The IE would be 7 bytes long with the Extended Capability field set to 0 (all bits zero)
				http://msdn.microsoft.com/library/default.asp?url=/library/en-us/randz/protocol/securing_public_wi-fi_hotspots.asp
			*/
			ULONG TempLen1 = 0;
			UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};
			MakeOutgoingFrame(pOutBuffer+FrameLen,        &TempLen1,
								7,                            PROVISION_SERVICE_IE,
								END_OF_ARGS);
			FrameLen += TempLen1;
	    }

        /* add Simple Config Information Element */
        if ((mbss->WscControl.WscConfMode > WSC_DISABLE) && (mbss->WscIEProbeResp.ValueLen))
        {
    		ULONG WscTmpLen = 0;
    		MakeOutgoingFrame(pOutBuffer+FrameLen,                                  &WscTmpLen,
    						  mbss->WscIEProbeResp.ValueLen,   mbss->WscIEProbeResp.Value,
                              END_OF_ARGS);
    		FrameLen += WscTmpLen;
        }
#endif /* WSC_AP_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
		/* The Mobility Domain information element (MDIE) is present in Probe-
		** Request frame when dot11FastBssTransitionEnable is set to true. */
		if (pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable)
		{
			PFT_CFG pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;
			FT_CAP_AND_POLICY FtCap;
			FtCap.field.FtOverDs = pFtCfg->FtCapFlag.FtOverDs;
			FtCap.field.RsrReqCap = pFtCfg->FtCapFlag.RsrReqCap;
			FT_InsertMdIE(pAd, pOutBuffer + FrameLen, &FrameLen,
							pFtCfg->FtMdId, FtCap);
		}
#endif /* DOT11R_FT_SUPPORT */



#ifdef WH_EZ_SETUP
	/*
		To prevent old device has trouble to parse MTK vendor IE,
		insert easy setup IE first.
	*/
	if (IS_EZ_SETUP_ENABLED(wdev) && ez_peer) {
		FrameLen += ez_build_probe_response_ie(wdev, pOutBuffer+FrameLen);

#ifdef NEW_CONNECTION_ALGO

	if (!ez_update_connection_permission(pAd, wdev, EZ_DISALLOW_ALL_ALLOW_ME)) {
				return;
		}
#endif
	}
#endif /* WH_EZ_SETUP */

	/*
		add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back
		                                 Byte0.b3=1 for rssi-feedback
	*/

    FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen)
#ifdef WH_EZ_SETUP
	, SUBTYPE_PROBE_RSP
#endif
	);

	{
        // Question to Rorscha: bit4 in old chip is used? but currently is using for 2.4G 256QAM

#ifdef RSSI_FEEDBACK
        UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
        ULONG TmpLen;

		if (ProbeReqParam.bRequestRssi == TRUE)
		{
		    MAC_TABLE_ENTRY *pEntry=NULL;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
										PRINT_MAC(ProbeReqParam.Addr2)));

			RalinkSpecificIe[5] |= 0x8;
			pEntry = MacTableLookup(pAd, ProbeReqParam.Addr2);

			if (pEntry != NULL)
			{
				RalinkSpecificIe[6] = (UCHAR)pEntry->RssiSample.AvgRssi[0];
				RalinkSpecificIe[7] = (UCHAR)pEntry->RssiSample.AvgRssi[1];
				RalinkSpecificIe[8] = (UCHAR)pEntry->RssiSample.AvgRssi[2];
			}
		}
		MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							9, RalinkSpecificIe,
							END_OF_ARGS);
		FrameLen += TmpLen;
#endif /* RSSI_FEEDBACK */

	}

	/* 802.11n 11.1.3.2.2 active scanning. sending probe response with MCS rate is */
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	//MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory( pOutBuffer);
	
#ifdef WH_EZ_SETUP
		if(IS_EZ_SETUP_ENABLED(wdev) && (ez_peer)) {
			ez_prepare_security_key(wdev, ProbeReqParam.Addr2, TRUE);
		}
#endif /* WH_EZ_SETUP */
	}
}
#ifdef EZ_REGROUP_SUPPORT
regrp_ap_info_struct regrp_ap_info[2][EZ_MAX_DEVICE_SUPPORT];
#endif


/*
	==========================================================================
	Description:
		parse the received BEACON

	NOTE:
		The only thing AP cares about received BEACON frames is to decide
		if there's any overlapped legacy BSS condition (OLBC).
		If OLBC happened, this AP should set the ERP->Use_Protection bit in its
		outgoing BEACON. The result is to tell all its clients to use RTS/CTS
		or CTS-to-self protection to protect B/G mixed traffic
	==========================================================================
 */


typedef struct
{
	ULONG	count;
	UCHAR	bssid[MAC_ADDR_LEN];
} BSSIDENTRY;


VOID APPeerBeaconAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES], *pRates = NULL, RatesLen;
	BOOLEAN LegacyBssExist;
	CHAR RealRssi;
	UCHAR *VarIE = NULL;
	USHORT LenVIE;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	UCHAR MaxSupportedRate = 0;
	BCN_IE_LIST *ie_list = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UCHAR Channel = 0;
#ifdef MWDS
    ULONG BssIdx;
#endif
	RETURN_IF_PAD_NULL(pAd);
	
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate ie_list fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));

	/* Init Variable IE structure */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate VarIE fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;



	pRates = (PUCHAR)Rates;

	/* Init the DUT's working channel from RX'D param first, actually we need to get the accurate Channel from wdev */
	Channel = Elem->Channel;
	/* PeerBeaconAndProbeRspSanity() may overwrite ie_list->Channel if beacon or  probe resp contain IE_DS_PARM */
	ie_list->Channel = Elem->Channel; 
	
	RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
							ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
							ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));

	if (PeerBeaconAndProbeRspSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								Elem->Channel,
								ie_list,
								&LenVIE,
								pVIE,
								FALSE,
								FALSE))
	{

		if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		{

			pEntry = MacTableLookup(pAd, ie_list->Addr2);//Found the pEntry from Peer Bcn Content

			if(!pEntry || !pEntry->wdev)
			{
				goto __End_Of_APPeerBeaconAction;
			}
	
			Channel = pEntry->wdev->channel;			
		
		}
		
#ifdef WH_EZ_SETUP
			{
				int j;
				for (j = 0; j <  pAd->ApCfg.BssidNum; j++)
				{
					struct wifi_dev * ez_wdev = &pAd->ApCfg.MBSSID[j].wdev;
					if (IS_EZ_SETUP_ENABLED(ez_wdev)) {
#ifdef EZ_MOD_SUPPORT
						ez_ap_peer_beacon_action(ez_wdev, ie_list->Addr2, ie_list->vendor_ie.ez_capability, &RealRssi, ie_list);
#else		
#ifdef NEW_CONNECTION_ALGO
						if (MAC_ADDR_EQUAL(ie_list->Addr2, ez_adapter.device_info.weight_defining_link.peer_ap_mac))
						{
								NdisGetSystemUpTime(&ez_adapter.device_info.weight_defining_link.ap_time_stamp);
								if (!EZ_GET_CAP_CONNECTED(ie_list->vendor_ie.ez_capability) 
									&& !NdisEqualMemory(ez_adapter.device_info.weight_defining_link.peer_ap_mac,ez_adapter.device_info.weight_defining_link.peer_mac,MAC_ADDR_LEN))
								{
									EZ_DEBUG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,("My WDL CLI is not connected\n"));
									ez_send_unicast_deauth(pAd,ez_adapter.device_info.weight_defining_link.peer_mac);
								}
						}
#endif
#endif

					}
				}
			}
#endif

#ifdef APCLI_SUPPORT
		ApCliCheckPeerExistence(pAd, ie_list->Ssid, ie_list->SsidLen, ie_list->Channel);
#endif /* APCLI_SUPPORT */


		/* ignore BEACON not in this channel */
		if (ie_list->Channel != Channel
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
			&& (!RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#ifdef CFG80211_MULTI_STA
			&& (!RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev))
#endif /* CFG80211_MULTI_STA */
			)
		{
			goto __End_Of_APPeerBeaconAction;
		}

#ifdef IDS_SUPPORT
		/* Conflict SSID detection */
		RTMPConflictSsidDetection(pAd, (PUCHAR)ie_list->Ssid, ie_list->SsidLen,
								(CHAR)Elem->rssi_info.raw_rssi[0],
								(CHAR)Elem->rssi_info.raw_rssi[1],
								(CHAR)Elem->rssi_info.raw_rssi[2]);
#endif /* IDS_SUPPORT */

#ifdef DOT11_N_SUPPORT
		/* 40Mhz BSS Width Trigger events Intolerant devices */
		if ((RealRssi > OBSS_BEACON_RSSI_THRESHOLD) && (ie_list->HtCapability.HtCapInfo.Forty_Mhz_Intolerant)) /* || (HtCapabilityLen == 0))) */
		{
			Handle_BSS_Width_Trigger_Events(pAd,Channel);
		}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if ((HcGetBwByRf(pAd,RFIC_24GHZ)== BW_40)
#ifdef DOT11N_DRAFT3
			&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
		   )
		{
			if (Channel<=14)
			{
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
				if(OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED) &&
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
					RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)
#else
					RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev)
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
				  )
				{
					if (ie_list->Channel != Channel)
					{
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Channel=%d is not equal as  band Channel = %d.\n", ie_list->Channel, Channel));
#ifdef WH_EZ_SETUP
						if(IS_ADPTR_EZ_SETUP_ENABLED(pAd)){
							//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("APPeerBeaconAction 3\n"));
							goto __End_Of_APPeerBeaconAction;
						}
#endif
					}
				}
				else
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_CONCURRENT_DEVICE */
				if (((HcGetCentralChByRf(pAd,RFIC_24GHZ)+2) != ie_list->Channel) &&
					((HcGetCentralChByRf(pAd,RFIC_24GHZ)-2) != ie_list->Channel))
				{
/*
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x is a legacy BSS (%d) \n",
								Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], Channel));
*/
                    //EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("APPeerBeaconAction 4\n"));
					//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Central channel = %d\n", HcGetCentralChByRf(pAd,RFIC_24GHZ)));
					goto __End_Of_APPeerBeaconAction;
				}
			}
			else
			{
				if (ie_list->Channel != Channel)
					goto __End_Of_APPeerBeaconAction;
			}
		}
#endif /* DOT11_N_SUPPORT */

                SupportRate(ie_list->SupRate, ie_list->SupRateLen, ie_list->ExtRate, ie_list->ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);

		if ((ie_list->Erp & 0x01) || (RatesLen <= 4))
			LegacyBssExist = TRUE;
		else
			LegacyBssExist = FALSE;

		if (LegacyBssExist && pAd->CommonCfg.DisableOLBCDetect == 0)
		{
			pAd->ApCfg.LastOLBCDetectTime = pAd->Mlme.Now32;

		}

#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.bHTProtect)
			&& (ie_list->HtCapabilityLen == 0) && (RealRssi > OBSS_BEACON_RSSI_THRESHOLD))
		{

			pAd->ApCfg.LastNoneHTOLBCDetectTime = pAd->Mlme.Now32;
		}
#endif /* DOT11_N_SUPPORT */

#ifdef APCLI_SUPPORT
		if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		{
			pEntry = &pAd->MacTab.Content[Elem->Wcid];

			if (pEntry &&
                (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) &&
                (pEntry->func_tb_idx < MAX_APCLI_NUM))
			{
				PAPCLI_STRUCT pApCliEntry = NULL;
				UCHAR op_ht_bw = wlan_operate_get_ht_bw(pEntry->wdev);

				pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];

				pApCliEntry->ApCliRcvBeaconTime = pAd->Mlme.Now32;

				if ((op_ht_bw == HT_BW_40) && (ie_list->vht_cap_len == 0))
				{
					/* Check if root-ap change BW to 20 */
					if ((ie_list->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_NONE) &&
						(ie_list->AddHtInfo.AddHtInfo.RecomWidth == 0))
					{
#ifdef MAC_REPEATER_SUPPORT
						UINT ifIndex;
						UCHAR CliIdx;
						REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
						RTMP_CHIP_CAP   *cap = &pAd->chipCap;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef APCLI_CERT_SUPPORT
						pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];

						if ((pEntry->HTPhyMode.field.BW != BW_20) &&
							(pAd->bApCliCertTest == TRUE))
							pApCliEntry->NeedFallback = TRUE;
#endif /* APCLI_CERT_SUPPORT */

						pEntry->HTPhyMode.field.BW = 0;
#ifdef MAC_REPEATER_SUPPORT
						ifIndex = pEntry->func_tb_idx;

						if (pAd->ApCfg.bMACRepeaterEn)
						{
							for(CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++)
							{
								pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

								if ((pReptEntry->CliEnable) && (pReptEntry->CliValid) && 
									(pReptEntry->MatchApCliIdx == ifIndex))
								{
									pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
									if (pEntry)
										pEntry->HTPhyMode.field.BW = 0;
								}
							}
						}
#endif /* MAC_REPEATER_SUPPORT */
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("FallBack APClient BW to 20MHz\n"));
					}

					/* Check if root-ap change BW to 40 */
					if ((ie_list->AddHtInfo.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
						(ie_list->HtCapabilityLen > 0) &&
						(ie_list->HtCapability.HtCapInfo.ChannelWidth == 1))
					{
#ifdef MAC_REPEATER_SUPPORT
						UINT ifIndex;
						UCHAR CliIdx;
						REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
                        RTMP_CHIP_CAP *cap = &pAd->chipCap;
#endif /* MAC_REPEATER_SUPPORT */
						pEntry->HTPhyMode.field.BW = 1;
#ifdef MAC_REPEATER_SUPPORT
						ifIndex = pEntry->func_tb_idx;

						if (pAd->ApCfg.bMACRepeaterEn)
						{
							for(CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++)
							{
								pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

								if ((pReptEntry->CliEnable) && (pReptEntry->CliValid) &&
									(pReptEntry->MatchApCliIdx == ifIndex))
								{
									pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
									if (pEntry)
										pEntry->HTPhyMode.field.BW = 1;
								}
							}
						}
#endif /* MAC_REPEATER_SUPPORT */
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("FallBack APClient BW to 40MHz\n"));
					}
				}

#ifdef APCLI_CERT_SUPPORT
				if (pAd->bApCliCertTest == TRUE)
				{
					UCHAR RegClass;
					OVERLAP_BSS_SCAN_IE BssScan;
					BOOLEAN brc;
#ifdef DOT11_N_SUPPORT
					ADD_HT_INFO_IE *aux_add_ht = &pApCliEntry->MlmeAux.AddHtInfo;
					ADD_HT_INFO_IE *addht;
					BOOLEAN bNonGFExist = (aux_add_ht->AddHtInfo2.NonGfPresent) ? TRUE : FALSE;
					UINT16 OperationMode = aux_add_ht->AddHtInfo2.OperaionMode;
#endif /* DOT11_N_SUPPORT */
			
					brc = PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
					if (brc == TRUE)
					{
						pAd->CommonCfg.Dot11BssWidthTriggerScanInt = le2cpu16(BssScan.TriggerScanInt); /*APBssScan.TriggerScanInt[1] * 256 + APBssScan.TriggerScanInt[0];*/
						/*DBGPRINT(RT_DEBUG_ERROR,("Update Dot11BssWidthTriggerScanInt=%d \n", pAd->CommonCfg.Dot11BssWidthTriggerScanInt)); */
						/* out of range defined in MIB... So fall back to default value.*/
						if ((pAd->CommonCfg.Dot11BssWidthTriggerScanInt < 10) ||(pAd->CommonCfg.Dot11BssWidthTriggerScanInt > 900))
						{
							/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11BssWidthTriggerScanInt out of range !!!!)  \n"));*/
							pAd->CommonCfg.Dot11BssWidthTriggerScanInt = 900;
						}
					}
#ifdef DOT11_N_SUPPORT
					/* check Ht protection mode. and adhere to the Non-GF device indication by AP. */
					if (ie_list->AddHtInfoLen != 0)
					{
						if ((ie_list->AddHtInfo.AddHtInfo2.OperaionMode != OperationMode)
							|| (ie_list->AddHtInfo.AddHtInfo2.NonGfPresent != bNonGFExist))
						{
							aux_add_ht->AddHtInfo2.OperaionMode = ie_list->AddHtInfo.AddHtInfo2.OperaionMode;
							aux_add_ht->AddHtInfo2.NonGfPresent = ie_list->AddHtInfo.AddHtInfo2.NonGfPresent;

							pApCliEntry->wdev.protection = SET_PROTECT(ie_list->AddHtInfo.AddHtInfo2.OperaionMode);

							OperationMode = aux_add_ht->AddHtInfo2.OperaionMode;
							bNonGFExist = (aux_add_ht->AddHtInfo2.NonGfPresent) ? TRUE : FALSE;

							if (bNonGFExist) {
								pApCliEntry->wdev.protection |= SET_PROTECT(GREEN_FIELD_PROTECT);
							}
							else {
								pApCliEntry->wdev.protection &= ~(SET_PROTECT(GREEN_FIELD_PROTECT));
							}

							if (pApCliEntry->wdev.channel> 14) {
								/* always no BG protection in A-band.
								 * falsely happened when switching A/G band to a dual-band AP */
								pApCliEntry->wdev.protection &= ~(SET_PROTECT(ERP));
							}
							addht = wlan_operate_get_addht(&pApCliEntry->wdev);
							if (addht) { /* sync addht information into wlan operation addht */
								*addht = pApCliEntry->MlmeAux.AddHtInfo;
							}

							AsicUpdateProtect(pAd, (USHORT) OperationMode, ALLN_SETPROTECT, FALSE, bNonGFExist);

							MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
										("SYNC - AP changed N OperaionMode to %d, my protection to %d\n",
															OperationMode, pApCliEntry->wdev.protection));
						}
					}
#endif /* DOT11_N_SUPPORT */
				}
#endif /* APCLI_CERT_SUPPORT */
			}

			if (pEntry && ie_list->NewChannel != 0)
				ApCliPeerCsaAction(pAd, pEntry->wdev, ie_list);
		}

#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
		if (pAd->WdsTab.Mode != WDS_DISABLE_MODE)
		{
			if (pAd->WdsTab.flg_wds_init)
			{
				MAC_TABLE_ENTRY *pEntry;
				BOOLEAN bWmmCapable;

				/* check BEACON does in WDS TABLE. */
				pEntry = WdsTableLookup(pAd, ie_list->Addr2, FALSE);
				bWmmCapable = ie_list->EdcaParm.bValid ? TRUE : FALSE;

				if (pEntry)
				{
					WdsPeerBeaconProc(pAd, pEntry, MaxSupportedRate, RatesLen, bWmmCapable,	ie_list);
				}
			}
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(), ERROR!! Beacon comes before wds_init\n", __FUNCTION__));
			}
		}
#endif /* WDS_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if (pAd->CommonCfg.bOverlapScanning == TRUE)
		{
			INT		index,secChIdx;
			BOOLEAN		found = FALSE;
			ADD_HTINFO *pAdd_HtInfo;

			for (index = 0; index < pAd->ChannelListNum; index++)
			{
				/* found the effected channel, mark that. */
				if(pAd->ChannelList[index].Channel == ie_list->Channel)
				{
					secChIdx = -1;
					if (ie_list->HtCapabilityLen > 0 && ie_list->AddHtInfoLen > 0)
					{	/* This is a 11n AP. */
						pAd->ChannelList[index].bEffectedChannel |= EFFECTED_CH_PRIMARY; /* 2; 	// 2 for 11N 20/40MHz AP with primary channel set as this channel. */
						pAdd_HtInfo = &ie_list->AddHtInfo.AddHtInfo;
						if (pAdd_HtInfo->ExtChanOffset == EXTCHA_BELOW)
						{
#ifdef A_BAND_SUPPORT
							if (ie_list->Channel > 14)
								secChIdx = ((index > 0) ? (index - 1) : -1);
							else
#endif /* A_BAND_SUPPORT */
								secChIdx = ((index >= 4) ? (index - 4) : -1);
						}
						else if (pAdd_HtInfo->ExtChanOffset == EXTCHA_ABOVE)
						{
#ifdef A_BAND_SUPPORT
							if (ie_list->Channel > 14)
								secChIdx = (((index+1) < pAd->ChannelListNum) ? (index + 1) : -1);
							else
#endif /* A_BAND_SUPPORT */
								secChIdx = (((index+4) < pAd->ChannelListNum) ? (index + 4) : -1);
						}

						if (secChIdx >=0)
							pAd->ChannelList[secChIdx].bEffectedChannel |= EFFECTED_CH_SECONDARY; /* 1; */

						if ((Channel != ie_list->Channel) || 
							(pAdd_HtInfo->ExtChanOffset  != HcGetExtCha(pAd,Channel))
						)
							pAd->CommonCfg.BssCoexApCnt++;
					}
					else
					{
						/* This is a legacy AP. */
						pAd->ChannelList[index].bEffectedChannel |=  EFFECTED_CH_LEGACY; /* 4; 1 for legacy AP. */
						pAd->CommonCfg.BssCoexApCnt++;
					}

					found = TRUE;
				}
			}
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef MWDS
        BssIdx = BssTableSearch(&pAd->ScanTab, ie_list->Bssid, ie_list->Channel);
        if (BssIdx != BSS_NOT_FOUND)
        {
            pAd->ScanTab.BssEntry[BssIdx].bSupportMWDS = FALSE;
            if(ie_list->vendor_ie.mtk_cap_found)
            {
                BOOLEAN bSupportMWDS = FALSE;
                if(ie_list->vendor_ie.support_mwds)
                    bSupportMWDS = TRUE;
                if(pAd->ScanTab.BssEntry[BssIdx].bSupportMWDS != bSupportMWDS)
                    pAd->ScanTab.BssEntry[BssIdx].bSupportMWDS = bSupportMWDS;
            }
        }
#endif /* MWDS */
	}
	/* sanity check fail, ignore this frame */

__End_Of_APPeerBeaconAction:
/*#ifdef AUTO_CH_SELECT_ENHANCE */
#ifdef CONFIG_AP_SUPPORT
IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
{
	if (ie_list->Channel == pAd->ApCfg.AutoChannel_Channel)
	{
		AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
		if (pAutoChCtrl->pChannelInfo && AutoChBssSearchWithSSID(pAd, ie_list->Bssid, (PUCHAR)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel) == BSS_NOT_FOUND)
			pAutoChCtrl->pChannelInfo->ApCnt[pAd->ApCfg.current_channel_index]++;
		AutoChBssInsertEntry(pAd, ie_list->Bssid, ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, ie_list->NewExtChannelOffset, RealRssi);
	}
}
#endif /* CONFIG_AP_SUPPORT */
/*#endif // AUTO_CH_SELECT_ENHANCE */

LabelErr:
	if (VarIE != NULL)
		os_free_mem(VarIE);
	if (ie_list != NULL)
		os_free_mem(ie_list);

	return;
}

#ifdef AP_SCAN_SUPPORT
/*
    ==========================================================================
    Description:
    ==========================================================================
 */
VOID APInvalidStateWhenScan(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AYNC - InvalidStateWhenScan(state=%ld). Reset SYNC machine\n", pAd->Mlme.ApSyncMachine.CurrState));
}

/*
    ==========================================================================
    Description:
        Scan timeout handler, executed in timer thread
    ==========================================================================
 */
VOID APScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP SYNC - Scan Timeout \n"));
	MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_SCAN_TIMEOUT, 0, NULL, (ULONG)pAd->ApCfg.ScanReqwdev);
	RTMP_MLME_HANDLER(pAd);
}

/*
    ==========================================================================
    Description:
        Scan timeout procedure. basically add channel index by 1 and rescan
    ==========================================================================
 */
VOID APScanTimeoutAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel);
	/* only scan the channel which binding band supported */
	if (pAd->ApCfg.ScanReqwdev != NULL && (pAd->ScanCtrl.Channel != 0)) {
		while (HcGetBandByChannel(pAd, pAd->ScanCtrl.Channel) != HcGetBandByWdev(pAd->ApCfg.ScanReqwdev)) {
			pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel);
			if (pAd->ScanCtrl.Channel == 0)
				break;
		}
	}
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/*
			iwpriv set auto channel selection
			update the current index of the channel
		*/
		if (pAd->ApCfg.bAutoChannelAtBootup == TRUE)
		{
			/* update current channel info */
			UpdateChannelInfo(pAd, pAd->ApCfg.current_channel_index, pAd->ApCfg.AutoChannelAlg);

			/* move to next channel */
			pAd->ApCfg.current_channel_index++;
			if (pAd->ApCfg.current_channel_index < pAd->ChannelListNum)
			{
				pAd->ApCfg.AutoChannel_Channel = pAd->ChannelList[pAd->ApCfg.current_channel_index].Channel;
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(pAd->ApCfg.ScanReqwdev) 
#ifndef EZ_MOD_SUPPORT
		&& (pAd->ApCfg.ScanReqwdev->ez_security.scan_one_channel || pAd->ApCfg.ScanReqwdev->ez_security.internal_force_connect_bssid)
#else
		&& ez_ap_scan_complete_handle(pAd->ApCfg.ScanReqwdev)
#endif
	) {
		pAd->ScanCtrl.Channel = 0;
		pAd->ScanCtrl.PartialScan.bScanning = FALSE;
	}
#endif /* WH_EZ_SETUP */

	ScanNextChannel(pAd, OPMODE_AP, pAd->ApCfg.ScanReqwdev);
}

#ifdef CON_WPS
#ifdef MULTI_INF_SUPPORT /* Index 0 for 2.4G, 1 for 5Ghz Card */
extern VOID* adapt_list[MAX_NUM_OF_INF];
#endif /* MULTI_INF_SUPPORT */

VOID APMlmeScanCompleteAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem)
{
        PWSC_CTRL   pWscControl;
        PWSC_CTRL   pApCliWscControl;
        UCHAR       apidx;
        INT         IsAPConfigured;
	struct wifi_dev *wdev;
	BOOLEAN     bNeedSetPBCTimer = TRUE;
#if defined(CON_WPS)
	INT currIfaceIdx=0;
	UCHAR ifIdx;
	UCHAR oppifIdx;
	struct wifi_dev *ConWpsdev=NULL;
	PWSC_CTRL   pTriggerApCliWscControl;
	PWSC_CTRL   pOpposApCliWscControl;
	PRTMP_ADAPTER pOpposAd;
	BOOLEAN     bTwoCardConWPS = FALSE;	
	UCHAR apcli_idx;
#ifdef MULTI_INF_SUPPORT /* Index 0 for 2.4G, 1 for 5Ghz Card */
	UINT opposIfaceIdx = !multi_inf_get_idx(pAd);
#endif /* MULTI_INF_SUPPORT */
#endif /*CON_WPS*/


	
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP SYNC - APMlmeScanCompleteAction\n"));
#if defined(CON_WPS)
	pOpposAd = NULL;
	pOpposApCliWscControl = NULL;
	pTriggerApCliWscControl= NULL;
#ifdef MULTI_INF_SUPPORT /* Index 0 for 2.4G, 1 for 5Ghz Card */
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposIfaceIdx];
#endif /* MULTI_INF_SUPPORT */
#endif /*CON_WPS*/

        /* If We catch the SR=TRUE in last scan_res, stop the AP Wsc SM */
	if (Elem)
	{
		ifIdx=(USHORT)(Elem->Priv);

		if(ifIdx < pAd->ApCfg.ApCliNum)
			ConWpsdev=&(pAd->ApCfg.ApCliTab[ifIdx].wdev);

		if (ConWpsdev == NULL)
			return;
	} else {
		return;
	}





	if (ifIdx == BSS0)
		oppifIdx = BSS1;
	else if (ifIdx == BSS1)
		oppifIdx = BSS0;
	else
		return;

	if (ConWpsdev)
	{
		pApCliWscControl = &pAd->ApCfg.ApCliTab[ifIdx].WscControl;
		pAd->ApCfg.ApCliTab[ifIdx].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_FINISH;
	}
	
	
	if (pOpposAd)
	{

		for (apcli_idx=0; apcli_idx < pOpposAd->ApCfg.ApCliNum; apcli_idx++)
		{
			if (pOpposAd->ApCfg.ApCliTab[apcli_idx].WscControl.conWscStatus==CON_WPS_STATUS_APCLI_RUNNING)
			{
				pOpposApCliWscControl = &pOpposAd->ApCfg.ApCliTab[apcli_idx].WscControl;
				bTwoCardConWPS=TRUE;
				break;
			}
		}

		if (apcli_idx == pOpposAd->ApCfg.ApCliNum)
		{
			pOpposApCliWscControl = NULL;
			bTwoCardConWPS=FALSE;
		}
	} 


	if (bTwoCardConWPS== FALSE)
	{
		for (apcli_idx=0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++)
		{
			if (apcli_idx == ifIdx)
				continue;
			else if (pAd->ApCfg.ApCliTab[apcli_idx].WscControl.conWscStatus==CON_WPS_STATUS_APCLI_RUNNING)
			{
				pOpposApCliWscControl = &pAd->ApCfg.ApCliTab[apcli_idx].WscControl;
				break;
			}
		}
	}

        if (pOpposAd && pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO) //2.2G and 5G must trigger scan
        {
        	if (pOpposAd && bTwoCardConWPS)
        	{
	        	for (apcli_idx=0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++)
			{
				if (pOpposAd->ApCfg.ApCliTab[apcli_idx].ConWpsApCliModeScanDoneStatus== CON_WPS_APCLI_SCANDONE_STATUS_ONGOING)
				{
					pApCliWscControl->ConWscApcliScanDoneCheckTimerRunning = TRUE;
					RTMPSetTimer(&pApCliWscControl->ConWscApcliScanDoneCheckTimer, 1000);
					return;
				}
			}
        	} 
	} else {
 
	        for (apcli_idx=0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++)
        {
			if (pAd->ApCfg.ApCliTab[apcli_idx].ConWpsApCliModeScanDoneStatus== CON_WPS_APCLI_SCANDONE_STATUS_ONGOING)
		{
			pApCliWscControl->ConWscApcliScanDoneCheckTimerRunning = TRUE;
				if (pAd->Mlme.ApSyncMachine.CurrState==0)
					WscScanExec(pAd,&(pAd->ApCfg.ApCliTab[apcli_idx].WscControl));
			RTMPSetTimer(&pApCliWscControl->ConWscApcliScanDoneCheckTimer, 1000);
			return;
		}
	}
	}

	if(pOpposApCliWscControl == NULL && pOpposAd)
	{
			pOpposApCliWscControl = &pOpposAd->ApCfg.ApCliTab[BSS0].WscControl;
			bTwoCardConWPS=TRUE;
	}


	if(pOpposApCliWscControl == NULL)
	{
			pOpposApCliWscControl = &pAd->ApCfg.ApCliTab[oppifIdx].WscControl;
			bTwoCardConWPS=FALSE;
	}
	
        WscPBCBssTableSort(pAd, pApCliWscControl);

#if defined(CON_WPS)
#ifdef MULTI_INF_SUPPORT /* Index 0 for 2.4G, 1 for 5Ghz Card */
	if (pOpposAd && bTwoCardConWPS)
	{
		if (pOpposApCliWscControl)
        WscPBCBssTableSort(pOpposAd, pOpposApCliWscControl);
	}
	else
#endif /* MULTI_INF_SUPPORT */		
	{
		if (pOpposApCliWscControl)
			WscPBCBssTableSort(pAd, pOpposApCliWscControl);
	}
	
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[Iface_Idx: %d] Scan_Completed!!! In APMlmeScanCompleteAction\n", currIfaceIdx));
#endif /*CON_WPS*/

#ifdef MULTI_INF_SUPPORT
	currIfaceIdx = multi_inf_get_idx(pAd);
#else
	currIfaceIdx=(pApCliWscControl->EntryIfIdx & 0x0F);
#endif /* MULTI_INF_SUPPORT */

	
        for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
        {
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
                pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
                IsAPConfigured = pWscControl->WscConfStatus;

                if ((pWscControl->WscConfMode != WSC_DISABLE) &&
                    (pApCliWscControl->WscPBCBssCount > 0))
                {
				                	if (pWscControl->bWscTrigger == TRUE)
				                	{
                        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s CON_WPS[%d]: Stop the AP Wsc Machine\n", __FUNCTION__, apidx));
                        WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
                        WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
                        UpdateBeaconHandler(pAd, wdev, IE_CHANGE);
                        WscStop(pAd, FALSE, pWscControl);
				         		 }   

                        WscConWpsStop(pAd, FALSE, pWscControl);
			                }
	                        continue;
		}

			if (bTwoCardConWPS)
			{
				if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 1)
				{
		        			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] AutoPreferIface = %d\n"
	        			                                           , currIfaceIdx, pAd->ApCfg.ConWpsApcliAutoPreferIface));
					if (currIfaceIdx == 0)
					{
						if (pAd->ApCfg.ConWpsApcliAutoPreferIface == CON_WPS_APCLI_AUTO_PREFER_IFACE1)
						{
							bNeedSetPBCTimer = FALSE;
							WscStop(pAd, TRUE, pApCliWscControl);
								MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! STOP APCLI = %d !!\n", currIfaceIdx));
						} 
						else 
						{
							WscConWpsStop(pAd, TRUE, pApCliWscControl);
								MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! STOP APCLI = %d !!\n", !currIfaceIdx));
						}
					}
					else if (currIfaceIdx == 1)
					{
						if (pAd->ApCfg.ConWpsApcliAutoPreferIface == CON_WPS_APCLI_AUTO_PREFER_IFACE0)
						{
							bNeedSetPBCTimer = FALSE;
							WscStop(pAd, TRUE, pApCliWscControl);
								MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!STOP APCLI = %d !!\n", currIfaceIdx));
						}
						else
						{

							WscConWpsStop(pAd, TRUE, pApCliWscControl);
								MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! STOP APCLI = %d !!\n", !currIfaceIdx));
						}
						}
					}

					if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 0)
					{
						WscConWpsStop(pAd, TRUE, pApCliWscControl);
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! (5)STOP APCLI = %d !!\n", !currIfaceIdx));
					}
			}			
			else
			{
			
				currIfaceIdx=(pApCliWscControl->EntryIfIdx & 0x0F);
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] Registrar_Found,  APCLI_Auto_Mode PreferIface = %d\n", 
						                                                     currIfaceIdx, pAd->ApCfg.ConWpsApcliAutoPreferIface));
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] WscPBCBssCount = %d, opposWscPBCBssCount = %d\n", 
						                                                     currIfaceIdx, 
						                                                     pApCliWscControl->WscPBCBssCount,
						                                                     pOpposApCliWscControl->WscPBCBssCount));	
					if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 1)
					{
			        		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] AutoPreferIface = %d\n"
			        			                                           , currIfaceIdx, pAd->ApCfg.ConWpsApcliAutoPreferIface));

						if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO)
						{
						if (currIfaceIdx != pAd->ApCfg.ConWpsApcliAutoPreferIface)
						{
								bNeedSetPBCTimer = FALSE;
								WscStop(pAd, TRUE, pApCliWscControl);	
								WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
								pTriggerApCliWscControl=pOpposApCliWscControl;
						} else {
								WscConWpsStop(pAd, TRUE, pApCliWscControl);
								pTriggerApCliWscControl=pApCliWscControl;
						}
						} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G ) {
									WscConWpsStop(pAd, TRUE, &(pAd->ApCfg.ApCliTab[BSS0].WscControl));
									pTriggerApCliWscControl=&(pAd->ApCfg.ApCliTab[BSS0].WscControl);
						} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G) {
										WscConWpsStop(pAd, TRUE, &(pAd->ApCfg.ApCliTab[BSS1].WscControl));
									pTriggerApCliWscControl=&(pAd->ApCfg.ApCliTab[BSS1].WscControl);
						}
				}

				/*Only Found 1 Registrar at one interface*/
				if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 0)
				{

						if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO)
						{
					WscConWpsStop(pAd, TRUE, pApCliWscControl);
							pTriggerApCliWscControl=pApCliWscControl;
						} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
						{
							if (currIfaceIdx==0)
							{
					WscConWpsStop(pAd, TRUE, pApCliWscControl);
							pTriggerApCliWscControl=pApCliWscControl;
							}
						} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G)
						{
							if (currIfaceIdx==1)
							{
								WscConWpsStop(pAd, TRUE, pApCliWscControl);
								pTriggerApCliWscControl=pApCliWscControl;							
							}
						}
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! (5)STOP APCLI = %d !!\n", !currIfaceIdx));
				}
						else if (pApCliWscControl->WscPBCBssCount == 0&& pOpposApCliWscControl->WscPBCBssCount == 1)
						{
						if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO)
						{
							WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
							pTriggerApCliWscControl=pOpposApCliWscControl;
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! (6)STOP APCLI = %d !!\n", !currIfaceIdx));
						} else 
						{
							if ((pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G) && (pOpposApCliWscControl->EntryIfIdx & 0x0F)==0)
							{
								WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
								pTriggerApCliWscControl=pOpposApCliWscControl;						
							} else if ((pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G) && (pOpposApCliWscControl->EntryIfIdx & 0x0F)==1)
							{
								WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
								pTriggerApCliWscControl=pOpposApCliWscControl;						
							}
                }
        }
        }

		if (bTwoCardConWPS)
		{
	        if (bNeedSetPBCTimer && pApCliWscControl->WscPBCTimerRunning == FALSE) 
            {
        		if (pApCliWscControl->bWscTrigger) 
        		{
        			pApCliWscControl->WscPBCTimerRunning = TRUE;
        			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!! TwoCardConWPS Trigger %s WPS!!\n", (pApCliWscControl->IfName)));				
        			RTMPSetTimer(&pApCliWscControl->WscPBCTimer, 1000);
        		}
            }
        } else {
			if (pTriggerApCliWscControl != NULL &&
				(pTriggerApCliWscControl->WscPBCTimerRunning == FALSE) &&
				(pTriggerApCliWscControl->bWscTrigger == TRUE)){
	   			pTriggerApCliWscControl->WscPBCTimerRunning = TRUE;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!! One Card DBDC Trigger %s WPS!!\n", (pTriggerApCliWscControl->IfName)));
				RTMPSetTimer(&pTriggerApCliWscControl->WscPBCTimer, 1000); 
				
			} else{ 
				if(pApCliWscControl && 
                   (pApCliWscControl->WscPBCTimerRunning == FALSE) &&
				   (pApCliWscControl->bWscTrigger == TRUE)){
					pAd->ApCfg.ApCliTab[(pApCliWscControl->EntryIfIdx & 0x0F)].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_ONGOING;
					pApCliWscControl->WscPBCTimerRunning = TRUE;
					RTMPSetTimer(&pApCliWscControl->WscPBCTimer, 1000);
					
				} 
				
				if(pOpposApCliWscControl && 
                   (pOpposApCliWscControl->WscPBCTimerRunning == FALSE) &&
				   (pOpposApCliWscControl->bWscTrigger == TRUE)){
					pAd->ApCfg.ApCliTab[(pOpposApCliWscControl->EntryIfIdx & 0x0F)].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_ONGOING;
					pOpposApCliWscControl->WscPBCTimerRunning = TRUE;
					RTMPSetTimer(&pOpposApCliWscControl->WscPBCTimer, 1000);   
				}
			}
        }
}
#endif /* CON_WPS*/

/*
    ==========================================================================
    Description:
        MLME SCAN req state machine procedure
    ==========================================================================
 */
VOID APMlmeScanReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
	UCHAR Ssid[MAX_LEN_OF_SSID], SsidLen, ScanType, BssType;
#ifdef WH_EZ_SETUP
	CHAR apcli_idx;
	MAC_TABLE_ENTRY *pMacEntry;
	struct wifi_dev *wdev;	
	struct wifi_dev *other_inf_wdev = NULL;
#endif

	/* first check the parameter sanity */
	if (MlmeScanReqSanity(pAd, Elem->Msg, Elem->MsgLen, &BssType, (PCHAR)Ssid, &SsidLen, &ScanType))
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP SYNC - MlmeScanReqAction\n"));
		NdisGetSystemUpTime(&pAd->ApCfg.LastScanTime);

		RTMPCancelTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);

		/* record desired BSS parameters */
		pAd->ScanCtrl.BssType = BssType;
		pAd->ScanCtrl.ScanType = ScanType;
		pAd->ScanCtrl.SsidLen = SsidLen;
		NdisMoveMemory(pAd->ScanCtrl.Ssid, Ssid, SsidLen);

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED((struct wifi_dev *)Elem->Priv)) 
		{
			pAd->ApCfg.ScanReqwdev = (struct wifi_dev *)Elem->Priv;
			apcli_idx = pAd->ApCfg.ScanReqwdev->func_idx;
#ifdef WSC_AP_SUPPORT
			if(!((pAd->ApCfg.ApCliTab[apcli_idx].WscControl.WscConfMode != WSC_DISABLE) 
				&& (pAd->ApCfg.ApCliTab[apcli_idx].WscControl.bWscTrigger == TRUE)))
#endif	
			{
				ez_apcli_check_partial_scan(pAd, apcli_idx);
			}
		}
#endif

		/* start from the first channel */
		if (pAd->ScanCtrl.PartialScan.bScanning == TRUE) {
			/* only scan the channel which binding band supported */
			pAd->ApCfg.ScanReqwdev = (struct wifi_dev *)Elem->Priv;
find_next_channel:
		pAd->ScanCtrl.Channel = FindScanChannel(pAd, 0);
			if (pAd->ScanCtrl.PartialScan.bScanning == TRUE) {
				if (HcGetBandByChannel(pAd, pAd->ScanCtrl.Channel) != HcGetBandByWdev(pAd->ApCfg.ScanReqwdev)) {
					pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel);
					if (pAd->ScanCtrl.Channel == 0) {
						if (pAd->ScanCtrl.PartialScan.bScanning == TRUE) {
							pAd->ScanCtrl.PartialScan.NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
							goto find_next_channel;
						}
					}
				}
			}
		} else {
			pAd->ScanCtrl.Channel = FindScanChannel(pAd, 0);
			/* only scan the channel which binding band supported */
			pAd->ApCfg.ScanReqwdev = (struct wifi_dev *)Elem->Priv;
			if (pAd->ApCfg.ScanReqwdev != NULL) {
				while (HcGetBandByChannel(pAd, pAd->ScanCtrl.Channel) != HcGetBandByWdev(pAd->ApCfg.ScanReqwdev)) {
					pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel);
					if (pAd->ScanCtrl.Channel == 0)
						break;
				}
			}
		}

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED((struct wifi_dev *)Elem->Priv)) 
		{
			wdev = (struct wifi_dev *)Elem->Priv;
			apcli_idx = wdev->func_idx;
#ifdef WSC_AP_SUPPORT
			if(!((pAd->ApCfg.ApCliTab[apcli_idx].WscControl.WscConfMode != WSC_DISABLE) 
				&& (pAd->ApCfg.ApCliTab[apcli_idx].WscControl.bWscTrigger == TRUE)))
#endif	
			{
				if (IS_SINGLE_CHIP_DBDC(pAd))
				{
					if (pAd->ApCfg.BssidNum == 2 && pAd->CommonCfg.dbdc_mode == 1)
					{
						if (pAd->ScanCtrl.PartialScan.LastScanChannel != 0)
						{
							wdev->ez_driver_params.bPartialScanRunning = TRUE;	
							if(wdev->wdev_type == WDEV_TYPE_AP)
							{
								other_inf_wdev = &pAd->ApCfg.ApCliTab[wdev->func_idx].wdev;
								other_inf_wdev->ez_driver_params.bPartialScanRunning = TRUE;
							} else {
								other_inf_wdev = &pAd->ApCfg.MBSSID[wdev->func_idx].wdev;
								other_inf_wdev->ez_driver_params.bPartialScanRunning = TRUE;
							}	
						} else {
							wdev->ez_driver_params.bPartialScanRunning = FALSE;
							if(wdev->wdev_type == WDEV_TYPE_AP)
							{
								other_inf_wdev = &pAd->ApCfg.ApCliTab[wdev->func_idx].wdev;
								other_inf_wdev->ez_driver_params.bPartialScanRunning = FALSE;
							} else {
								other_inf_wdev = &pAd->ApCfg.MBSSID[wdev->func_idx].wdev;
								other_inf_wdev->ez_driver_params.bPartialScanRunning = FALSE;
							}								
						}
					}
				}
			}
		}
		if (IS_EZ_SETUP_ENABLED(pAd->ApCfg.ScanReqwdev) 
#ifndef EZ_MOD_SUPPORT			
			&& (pAd->ApCfg.ScanReqwdev->ez_security.scan_one_channel || pAd->ApCfg.ScanReqwdev->ez_security.internal_force_connect_bssid)
#else
			&& ez_ap_scan_complete_handle(pAd->ApCfg.ScanReqwdev)
#endif
		) {
			pAd->ScanCtrl.Channel = pAd->ApCfg.ScanReqwdev->channel;
			EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("AP SYNC - Only scan ch.%d and keep original BW setting.\n", pAd->ScanCtrl.Channel));
		}
		else
#endif /* WH_EZ_SETUP */
		{
			/* Let BBP register at 20MHz to do scan */
			HcBbpSetBwByChannel(pAd,BW_20,pAd->ScanCtrl.Channel);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.bAutoChannelAtBootup == TRUE)/* iwpriv set auto channel selection */
			{
				APAutoChannelInit(pAd);
				pAd->ApCfg.AutoChannel_Channel = pAd->ChannelList[0].Channel;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef WH_EZ_SETUP		
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
			apcli_idx = pAd->ApCfg.ScanReqwdev->func_idx;
			pMacEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[apcli_idx].wdev.bssid);
					
			if(pMacEntry && pAd->ApCfg.ApCliTab[apcli_idx].Valid 
				&& IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev))
			{
				pAd->Mlme.ApSyncMachine.CurrState = AP_SCAN_LISTEN;
				ApCliRTMPSendNullFrame(pAd,pMacEntry->CurrTxRate, FALSE, pMacEntry, PWR_SAVE);
			}		
#endif	//APCLI_SUPPORT
#endif	//CONFIG_AP_SUPPORT
#endif	//WH_EZ_SETUP

		ScanNextChannel(pAd, OPMODE_AP, (struct wifi_dev *)Elem->Priv);
	}
	else
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP SYNC - MlmeScanReqAction() sanity check fail. BUG!!!\n"));
		pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
	}
}


/*
    ==========================================================================
    Description:
        peer sends beacon back when scanning
    ==========================================================================
 */
VOID APPeerBeaconAtScanAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFrame;
	UCHAR *VarIE = NULL;
	USHORT LenVIE;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	CHAR RealRssi = -127;
	BCN_IE_LIST *ie_list = NULL;


	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (!ie_list) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Alloc memory for ie_list fail!!!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory((UCHAR *)ie_list, sizeof(BCN_IE_LIST));

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}

	pFrame = (PFRAME_802_11) Elem->Msg;
	/* Init Variable IE structure */
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;


	if (PeerBeaconAndProbeRspSanity(pAd,
					Elem->Msg, Elem->MsgLen, Elem->Channel,
					ie_list, &LenVIE, pVIE, FALSE, FALSE))
    {
		ULONG Idx;
		CHAR  Rssi = -127;
		MAC_TABLE_ENTRY *pEntry = NULL;
		UCHAR Channel;
		
		pEntry = MacTableLookup(pAd, ie_list->Addr2);//Found the pEntry from Peer Bcn Content


		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));

		if(ie_list->Channel > 14)
		{
			Channel = HcGetChannelByRf(pAd,RFIC_5GHZ);
		}
		else
		{
			Channel = HcGetChannelByRf(pAd,RFIC_24GHZ);
		}



#ifdef WH_EZ_SETUP
		if( !IS_ADPTR_EZ_SETUP_ENABLED(pAd) )
#endif
		{
			/* ignore BEACON not in this channel */
			if (ie_list->Channel != pAd->ScanCtrl.Channel
	#ifdef DOT11_N_SUPPORT
	#ifdef DOT11N_DRAFT3
				&& (pAd->CommonCfg.bOverlapScanning == FALSE)
	#endif /* DOT11N_DRAFT3 */
	#endif /* DOT11_N_SUPPORT */
			   )
			{
				goto __End_Of_APPeerBeaconAtScanAction;
			}
		}

#ifdef DOT11_N_SUPPORT
   		if ((RealRssi > OBSS_BEACON_RSSI_THRESHOLD) && (ie_list->HtCapability.HtCapInfo.Forty_Mhz_Intolerant)) /* || (HtCapabilityLen == 0))) */
		{
			if ((pAd->ScanCtrl.ScanType == SCAN_2040_BSS_COEXIST) &&
					(pAd->ApCfg.ScanReqwdev->wdev_type == WDEV_TYPE_APCLI)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:Ignore BW 40->20\n",__func__));
			} else
				Handle_BSS_Width_Trigger_Events(pAd,Channel);
		}
#endif /* DOT11_N_SUPPORT */

#ifdef IDS_SUPPORT
		/* Conflict SSID detection */
		if (ie_list->Channel == Channel)
			RTMPConflictSsidDetection(pAd, ie_list->Ssid, ie_list->SsidLen,
							Elem->rssi_info.raw_rssi[0],
							Elem->rssi_info.raw_rssi[1],
							Elem->rssi_info.raw_rssi[2]);
#endif /* IDS_SUPPORT */

		/*
			This correct im-proper RSSI indication during SITE SURVEY issue.
			Always report bigger RSSI during SCANNING when receiving multiple BEACONs from the same AP.
			This case happens because BEACONs come from adjacent channels, so RSSI become weaker as we
			switch to more far away channels.
		*/
        Idx = BssTableSearch(&pAd->ScanTab, ie_list->Bssid, ie_list->Channel);
		if (Idx != BSS_NOT_FOUND && Idx < MAX_LEN_OF_BSS_TABLE)
            		Rssi = pAd->ScanTab.BssEntry[Idx].Rssi;



        /* TODO: 2005-03-04 dirty patch. we should change all RSSI related variables to SIGNED SHORT for easy/efficient reading and calaulation */
		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
        if ((RealRssi + pAd->BbpRssiToDbmDelta) > Rssi)
            Rssi = RealRssi + pAd->BbpRssiToDbmDelta;

		Idx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, Rssi, LenVIE, pVIE);

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
				/* Check if this scan channel is the effeced channel */
				if (APCLI_IF_UP_CHECK(pAd, 0) &&
					(pAd->bApCliCertTest == TRUE) &&
					(pAd->CommonCfg.bBssCoexEnable == TRUE) 
					&& ((ie_list->Channel > 0) && (ie_list->Channel <= 14)))
				{
					int chListIdx;
		
					/* 
						First we find the channel list idx by the channel number
					*/
					for (chListIdx = 0; chListIdx < pAd->ChannelListNum; chListIdx++)
					{
						if (ie_list->Channel == pAd->ChannelList[chListIdx].Channel)
							break;
					}
		
					if (chListIdx < pAd->ChannelListNum)
					{
						/* 
							If this channel is effected channel for the 20/40 coex operation. Check the related IEs.
						*/
						if (pAd->ChannelList[chListIdx].bEffectedChannel == TRUE)
						{
							UCHAR RegClass;
							OVERLAP_BSS_SCAN_IE BssScan;
		
							/* Read Beacon's Reg Class IE if any. */
							PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
							//printk("\x1b[31m TriEventTableSetEntry \x1b[m\n");
							TriEventTableSetEntry(pAd, &pAd->CommonCfg.TriggerEventTab, ie_list->Bssid, &ie_list->HtCapability, ie_list->HtCapabilityLen, RegClass, ie_list->Channel );
						}
					}
				}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */

		if (Idx != BSS_NOT_FOUND)
		{
			NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
		}

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		if (RTMPEqualMemory(ie_list->Ssid, "DIRECT-", 7))
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s P2P_SCANNING: %s [%lu], channel =%d\n"
									, __FUNCTION__, ie_list->Ssid, Idx,Elem->Channel));

		/* Determine primary channel by IE's DSPS rather than channel of received frame */
        if (ie_list->Channel != 0)
            Elem->Channel = ie_list->Channel;

        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APPeerBeaconAtScanAction : Update the SSID %s in Kernel Table, Elem->Channel=%u\n", ie_list->Ssid,Elem->Channel));
        RT_CFG80211_SCANNING_INFORM(pAd, Idx, /*ie_list->Channel*/Elem->Channel, (UCHAR *)Elem->Msg, Elem->MsgLen, RealRssi);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef MWDS
        if (Idx != BSS_NOT_FOUND)
        {
            pAd->ScanTab.BssEntry[Idx].bSupportMWDS = FALSE;
            if(ie_list->vendor_ie.mtk_cap_found)
            {
                BOOLEAN bSupportMWDS = FALSE;
                if(ie_list->vendor_ie.support_mwds)
                    bSupportMWDS = TRUE;
                
                if(pAd->ScanTab.BssEntry[Idx].bSupportMWDS != bSupportMWDS)
                    pAd->ScanTab.BssEntry[Idx].bSupportMWDS = bSupportMWDS;
            }
        }
#endif /* MWDS */
#ifdef APCLI_SUPPORT
#ifdef WH_EVENT_NOTIFIER
        if(pFrame && (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP))
        {
            EventHdlr pEventHdlrHook = NULL;
            pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_AP_PROBE_RSP);
            if(pEventHdlrHook && pAd->ApCfg.ScanReqwdev)
                pEventHdlrHook(pAd, pAd->ApCfg.ScanReqwdev, ie_list, Elem);
        }
#endif /* WH_EVENT_NOTIFIER */
#endif /* APCLI_SUPPORT */
	}

	/* sanity check fail, ignored */
__End_Of_APPeerBeaconAtScanAction:
	/*scan beacon in pastive */
#ifdef CONFIG_AP_SUPPORT
IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
{
	if (ie_list->Channel == pAd->ApCfg.AutoChannel_Channel)
	{
	
		AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
		if (AutoChBssSearchWithSSID(pAd, ie_list->Bssid, (PUCHAR)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel) == BSS_NOT_FOUND)
			pAutoChCtrl->pChannelInfo->ApCnt[pAd->ApCfg.current_channel_index]++;

		AutoChBssInsertEntry(pAd, ie_list->Bssid, (CHAR *)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, ie_list->NewExtChannelOffset, RealRssi);
	}
}
#endif /* CONFIG_AP_SUPPORT */

LabelErr:
	if (VarIE != NULL)
		os_free_mem(VarIE);
	if (ie_list != NULL)
		os_free_mem(ie_list);

}

/*
    ==========================================================================
    Description:
        MLME Cancel the SCAN req state machine procedure
    ==========================================================================
 */
VOID APScanCnclAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;

	RTMPCancelTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);
	pAd->ScanCtrl.Channel = 0;
	ScanNextChannel(pAd, OPMODE_AP, pAd->ApCfg.ScanReqwdev);

	return;
}

/*
    ==========================================================================
    Description:
        if ChannelSel is false,
        	AP scans channels and lists the information of channels.
        if ChannelSel is true,
        	AP scans channels and selects an optimal channel.

    NOTE:
    ==========================================================================
*/
VOID ApSiteSurvey(
	IN	PRTMP_ADAPTER  		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel)
{
    	MLME_SCAN_REQ_STRUCT    ScanReq;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
	    	/*  
                 Still scanning, ignore this scan.
	    	*/
	    	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Scanning now!\n", __FUNCTION__));
	    	return;
	}

	AsicDisableSync(pAd, HW_BSSID_0);
	AsicDisableBcnSntReq(pAd, NULL);

	/* Don't clear the scan table if we are doing partial scan */
	if ((pAd->ScanCtrl.PartialScan.bScanning == TRUE && pAd->ScanCtrl.PartialScan.LastScanChannel == 0) ||
		pAd->ScanCtrl.PartialScan.bScanning == FALSE)
	{
	    	BssTableInit(&pAd->ScanTab);
	}

    	pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;

	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;
	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
	    ScanReq.SsidLen = pSsid->SsidLength;
	    NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}
	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	pAd->ApCfg.bAutoChannelAtBootup = ChannelSel;

	MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
	RTMP_MLME_HANDLER(pAd);
}

VOID ApSiteSurvey_by_wdev(
	IN	PRTMP_ADAPTER  		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel,
	struct wifi_dev 		*wdev)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
#ifdef CON_WPS		
	UCHAR ifIdx;
#endif /*ifIdx*/	

#ifdef WH_EZ_SETUP
struct wifi_dev* other_band_wdev;
struct wifi_dev* other_band_wdev_ap;
#endif

#ifdef WH_EZ_SETUP

	if (IS_EZ_SETUP_ENABLED(wdev) && RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
	    /*  
                 Still scanning, ignore this scan.
	    	*/
    	    EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Scanning now!\n", __FUNCTION__));
    	    return;
	}
	
	if (IS_EZ_SETUP_ENABLED(wdev) && (ScanRunning(pAd) == TRUE))
	{
		EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Scan Already Running! \n", __FUNCTION__));
		return;
	}

	if (IS_SINGLE_CHIP_DBDC(pAd) && IS_EZ_SETUP_ENABLED(wdev))
	{
		if (pAd->ApCfg.BssidNum == 2 && pAd->CommonCfg.dbdc_mode == 1)
		{
			other_band_wdev = &pAd->ApCfg.ApCliTab[wdev->func_idx ^ 1].wdev;
			other_band_wdev_ap = &pAd->ApCfg.MBSSID[wdev->func_idx ^ 1].wdev;
			if ((other_band_wdev->ez_driver_params.bPartialScanRunning == TRUE) || (other_band_wdev_ap->ez_driver_params.bPartialScanRunning == TRUE))
			{
				EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Partial Scan Already Running on other Band! \n", __FUNCTION__));
				return;
			}
		}
	}
		
	if(IS_EZ_SETUP_ENABLED(wdev))
#ifdef EZ_MOD_SUPPORT
		increment_best_ap_rssi_threshold(&pAd->ApCfg.ApCliTab[wdev->func_idx].wdev);
#else
		increment_best_ap_rssi_threshold(&pAd->ApCfg.ApCliTab[wdev->func_idx].wdev.ez_security);
#endif
#endif

	AsicDisableSync(pAd, HW_BSSID_0);
	AsicDisableBcnSntReq(pAd, wdev);

	/* Don't clear the scan table if we are doing partial scan */
#ifdef CON_WPS	
	ifIdx = wdev->func_idx;
	if ((ifIdx < MAX_APCLI_NUM)
		&&(pAd->ApCfg.ConWpsApCliDisabled==FALSE)
		&&(pAd->ApCfg.ApCliTab[ifIdx].WscControl.conWscStatus & CON_WPS_STATUS_APCLI_RUNNING))
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\033[1;32m ApSiteSurvey_by_wdev don't need Init BSS table\033[0m\n"));
		} else
#endif /*CON_WPS*/
	if ((pAd->ScanCtrl.PartialScan.bScanning == TRUE && pAd->ScanCtrl.PartialScan.LastScanChannel == 0) ||
			pAd->ScanCtrl.PartialScan.bScanning == FALSE)
	{
		#if defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT)
		if (pAd->CommonCfg.dbdc_mode == TRUE)
		{
			UCHAR Band = 0;
			if (WMODE_CAP_2G(wdev->PhyMode))
				Band =0;
			
			if (WMODE_CAP_5G(wdev->PhyMode))
				Band =1;

			/*
				backup the other band's scan result.
			*/
			BssTableInitByBand(&pAd->ScanTab, Band);
		}
		else
		#endif /* defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT) */
		{
			BssTableInit(&pAd->ScanTab);
		}
	}

	pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;

	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;
	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
	    ScanReq.SsidLen = pSsid->SsidLength;
	    NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}
	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	pAd->ApCfg.bAutoChannelAtBootup = ChannelSel;

	MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, (ULONG)wdev);
	RTMP_MLME_HANDLER(pAd);
}

BOOLEAN ApScanRunning(RTMP_ADAPTER *pAd)
{
	if ((pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN) 
		|| (pAd->AutoChSelCtrl.AutoChScanStatMachine.CurrState == AUTO_CH_SEL_SCAN_LISTEN))
		return TRUE;
	else
		return FALSE;
}
#endif /* AP_SCAN_SUPPORT */

#ifdef WDS_SUPPORT
VOID APWdsRecvUcDataAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
	MAC_TABLE_ENTRY *pEntry;
	RT_802_11_WDS_ENTRY *wds_entry;

	RETURN_IF_PAD_NULL(pAd);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): PhyMode = %d\n", __FUNCTION__, Elem->RxPhyMode));

	/* lookup the match wds entry for the incoming packet. */
	pEntry = WdsTableLookupByWcid(pAd, Elem->Wcid, pFrame->Hdr.Addr2, TRUE);
	if (pEntry == NULL)
		pEntry = WdsTableLookup(pAd, pFrame->Hdr.Addr2, TRUE);

	/* Only Lazy mode will auto learning, match with FrDs=1 and ToDs=1 */
	if((pEntry == NULL) && (pAd->WdsTab.Mode >= WDS_LAZY_MODE))
	{
		INT WdsIdx = WdsEntryAlloc(pAd, pFrame->Hdr.Addr2);
		if (WdsIdx >= 0 && WdsIdx < MAX_WDS_ENTRY)
		{
			wds_entry = &pAd->WdsTab.WdsEntry[WdsIdx];

			/* user doesn't specific a phy mode for WDS link. */
			if (wds_entry->PhyOpMode == 0xff)
			{
			    UINT32 encrypt_mode = wds_entry->wdev.SecConfig.PairwiseCipher;

			    if (pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(encrypt_mode))
					wds_entry->PhyOpMode = (wds_entry->PhyOpMode >= MODE_OFDM)? MODE_OFDM : MODE_CCK;
				else
					wds_entry->PhyOpMode = Elem->RxPhyMode;
				wds_entry->wdev.PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, wds_entry);
			}
			pEntry = MacTableInsertWDSEntry(pAd, pFrame->Hdr.Addr2, (UCHAR)WdsIdx);

			if(!pEntry)
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s(): can't insert new pEntry \n", __FUNCTION__));
				return;
			}

			pEntry->SupportRateMode = WdsPhyOpModeToSuppRateMode(pAd, wds_entry);
			RAInit(pAd, pEntry);
		}
	}
}
#endif


/*
	==========================================================================
	Description:
		The sync state machine,
	Parameters:
		Sm - pointer to the state machine
	Note:
		the state machine looks like the following

							AP_SYNC_IDLE
	APMT2_PEER_PROBE_REQ	peer_probe_req_action
	==========================================================================
 */
VOID APSyncStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AP_MAX_SYNC_STATE, AP_MAX_SYNC_MSG, (STATE_MACHINE_FUNC)Drop, AP_SYNC_IDLE, AP_SYNC_MACHINE_BASE);

	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_PROBE_REQ, (STATE_MACHINE_FUNC)APPeerProbeReqAction);
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_BEACON, (STATE_MACHINE_FUNC)APPeerBeaconAction);
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)APPeerBeaconAtScanAction);
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */
#ifdef AP_SCAN_SUPPORT
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)APMlmeScanReqAction);
#ifdef CON_WPS
        StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_MLME_SCAN_COMPLETE, (STATE_MACHINE_FUNC)APMlmeScanCompleteAction);
#endif /* CON_WPS */
#ifdef WDS_SUPPORT
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_WDS_RECV_UC_DATA, (STATE_MACHINE_FUNC)APWdsRecvUcDataAction);
#endif /* WDS_SUPPORT */

	/* scan_listen state */
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)APInvalidStateWhenScan);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_PEER_BEACON, (STATE_MACHINE_FUNC)APPeerBeaconAtScanAction);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)APPeerBeaconAtScanAction);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)APScanTimeoutAction);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_MLME_SCAN_CNCL, (STATE_MACHINE_FUNC)APScanCnclAction);

	RTMPInitTimer(pAd, &pAd->ScanCtrl.APScanTimer, GET_TIMER_FUNCTION(APScanTimeout), pAd, FALSE);
#endif /* AP_SCAN_SUPPORT */
}


VOID SupportRate(
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN PUCHAR ExtRate,
	IN UCHAR ExtRateLen,
	OUT PUCHAR *ppRates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate)
{
	INT i;

	*pMaxSupportRate = 0;

	if ((SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES) && (SupRateLen > 0))
	{
		NdisMoveMemory(*ppRates, SupRate, SupRateLen);
		*RatesLen = SupRateLen;
	}
	else
	{
		/* HT rate not ready yet. return true temporarily. rt2860c */
		/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerAssocReqSanity - wrong IE_SUPP_RATES\n")); */
		*RatesLen = 8;
		*(*ppRates + 0) = 0x82;
		*(*ppRates + 1) = 0x84;
		*(*ppRates + 2) = 0x8b;
		*(*ppRates + 3) = 0x96;
		*(*ppRates + 4) = 0x12;
		*(*ppRates + 5) = 0x24;
		*(*ppRates + 6) = 0x48;
		*(*ppRates + 7) = 0x6c;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SUPP_RATES., Len=%d\n", SupRateLen));
	}

	if (ExtRateLen + *RatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
	{
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ExtRate, ExtRateLen);
		*RatesLen = (*RatesLen) + ExtRateLen;
	}
	else
	{
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ExtRate, MAX_LEN_OF_SUPPORTED_RATES - (*RatesLen));
		*RatesLen = MAX_LEN_OF_SUPPORTED_RATES;
	}



	for (i = 0; i < *RatesLen; i++)
	{
		if(*pMaxSupportRate < (*(*ppRates + i) & 0x7f))
			*pMaxSupportRate = (*(*ppRates + i) & 0x7f);
	}

	return;
}

#ifdef DOT11_N_SUPPORT
void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE,
	IN UCHAR Channel,
	IN UCHAR PhyMode,
	IN struct wifi_dev *wdev)
{

	pIE->ID = IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT;
	pIE->Length = 4;
	pIE->ChannelSwitchMode = 1;	/*no further frames */
	pIE->NewRegClass = get_regulatory_class(pAd, Channel, PhyMode, wdev);
	pIE->NewChannelNum = Channel;
    pIE->ChannelSwitchCount = (pAd->Dot11_H.CSPeriod - pAd->Dot11_H.CSCount - 1);
}
#endif /* DOT11_N_SUPPORT */

