/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
 
	Abstract:

	All related CFG80211 P2P function body.

	History:

***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data)
{
	//UCHAR lock_channel = CFG80211_getCenCh(pAd, Data);
	UCHAR lock_channel = Data;

	if(RTMP_CFG80211_HOSTAPD_ON(pAd))
		return;
		
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	struct wifi_dev *wdev = &pMbss->wdev;

	if (pAd->Mlme.bStartMcc == TRUE)
		return;

	if(pAd->Mlme.bStartScc == TRUE)
	{
//		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SCC Enabled, Do not switch channel for Tx  %d\n",lock_channel));
		return;
	}

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) && (wdev->channel == lock_channel) && (wlan_operate_get_ht_bw(wdev)==HT_BW_40))
	{
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("40 BW Enabled || GO enable , wait for CLI connect, Do not switch channel for Tx\n"));
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("GO wdev->channel  %d  lock_channel %d \n",wdev->channel,lock_channel));

		return;
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */


#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	if (INFRA_ON(pAd) &&
	   	     (((pAd->LatchRfRegs.Channel != pAd->StaCfg[0].wdev.CentralChannel) && (pAd->StaCfg[0].wdev.CentralChannel != 0))) 
	   	     || (pAd->LatchRfRegs.Channel != lock_channel))
#else
	if (pAd->LatchRfRegs.Channel != lock_channel)
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	{
		AsicSwitchChannel(pAd, lock_channel, FALSE);
		AsicLockChannel(pAd, lock_channel);
		
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Off-Channel Send Packet: From(%d)-To(%d)\n", 
									pAd->LatchRfRegs.Channel, lock_channel));
	}
	else
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Off-Channel Channel Equal: %d\n", pAd->LatchRfRegs.Channel));

}

#ifdef CONFIG_AP_SUPPORT

#ifdef DISABLE_HOSTAPD_PROBE_RESP
/*
	==========================================================================
	Description:
		Process the received ProbeRequest from clients for hostapd
	Parameters:
		apidx
	==========================================================================
 */
VOID ProbeResponseHandler(
	IN PRTMP_ADAPTER pAd,
	IN PEER_PROBE_REQ_PARAM *ProbeReqParam,	
	IN UINT8 apidx)

{
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, TmpLen;
	LARGE_INTEGER FakeTimestamp;
	UCHAR DsLen = 1;
	UCHAR ErpIeLen = 1;
	UCHAR PhyMode, SupRateLen;
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;

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
	BSS_STRUCT *pMbss;
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef WDS_SUPPORT
	/* if in bridge mode, no need to reply probe req. */
	if (pAd->WdsTab.Mode == WDS_BRIDGE_MODE)
		return;
#endif /* WDS_SUPPORT */

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
						return;
		}

		PhyMode = wdev->PhyMode;

					if ( ((((ProbeReqParam->SsidLen == 0) && (!mbss->bHideSsid)) ||
			   ((ProbeReqParam->SsidLen == mbss->SsidLen) && NdisEqualMemory(ProbeReqParam->Ssid, mbss->Ssid, (ULONG) ProbeReqParam->SsidLen)))
#ifdef CONFIG_HOTSPOT
			   && ProbeReqforHSAP(pAd, apidx, &ProbeReqParam)
#endif
			 )
		)
		{
			;
		}
		else {
						return;
		}

		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, ProbeReqParam->Addr2,
							wdev->if_addr, wdev->bssid);

		{
		SupRateLen = rate->SupRateLen;
		if (PhyMode == WMODE_B)
			SupRateLen = 4;

		MakeOutgoingFrame(pOutBuffer,				  &FrameLen,
						  sizeof(HEADER_802_11),	  &ProbeRspHdr,
						  TIMESTAMP_LEN,			  &FakeTimestamp,
						  2,						  &pAd->CommonCfg.BeaconPeriod,
						  2,						  &mbss->CapabilityInfo,
						  1,						  &SsidIe,
						  1,						  &mbss->SsidLen,
						  mbss->SsidLen,	 mbss->Ssid,
						  1,						  &SupRateIe,
						  1,						  &SupRateLen,
						  SupRateLen,				  rate->SupRate,
						  1,						  &DsIe,
						  1,						  &DsLen,
						  1,						  &wdev->channel,
						  END_OF_ARGS);
		}

		if ((rate->ExtRateLen) && (PhyMode != WMODE_B))
		{
			MakeOutgoingFrame(pOutBuffer+FrameLen,		&TmpLen,
							  1,						&ErpIe,
							  1,						&ErpIeLen,
							  1,						&pAd->ApCfg.ErpIeContent,
							  1,						&ExtRateIe,
							  1,						&rate->ExtRateLen,
							  rate->ExtRateLen, 		rate->ExtRate,
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

			MakeOutgoingFrame(pOutBuffer + FrameLen,			&TmpLen,
							  1,								&HtCapIe,
							  1,								&HtLen,
							 sizeof(HT_CAPABILITY_IE),			&HtCapabilityTmp,
							  1,								&AddHtInfoIe,
							  1,								&AddHtLen,
							 sizeof(ADD_HT_INFO_IE),		  addht,
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

			MakeOutgoingFrame(pOutBuffer + FrameLen,		 &TmpLen,
								1,							 &HtCapIe,
								1,							 &HtLen,
								HtLen,						 &HtCapabilityTmp,
								1,							 &AddHtInfoIe,
								1,							 &AddHtLen,
								AddHtLen,					 &addHTInfoTmp,
								END_OF_ARGS);

#endif
			FrameLen += TmpLen;
		}
#endif /* DOT11_N_SUPPORT */

		/* Append RSN_IE when  WPA OR WPAPSK, */
		pSecConfig = &wdev->SecConfig;
		
#ifdef CONFIG_HOTSPOT_R2
		pMbss=&pAd->ApCfg.MBSSID[wdev->func_idx];
		if ((pMbss->HotSpotCtrl.HotSpotEnable == 0) && (pMbss->HotSpotCtrl.bASANEnable == 1)&& (IS_AKM_WPA2_Entry(wdev)))
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
#ifdef DISABLE_HOSTAPD_PROBE_RESP
				BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
				if(mbss->RSNIE_Len[rsne_idx] != 0)
				{
					MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
	                1, &mbss->RSNIE_ID[rsne_idx],
	                1, &mbss->RSNIE_Len[rsne_idx],
	                mbss->RSNIE_Len[rsne_idx],&mbss->RSN_IE[rsne_idx][0],
	                END_OF_ARGS);
					FrameLen += TmpLen;
		
				}

#else
				if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;
		
				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
					1, &pSecConfig->RSNE_EID[rsne_idx][0],
					1, &pSecConfig->RSNE_Len[rsne_idx],
					pSecConfig->RSNE_Len[rsne_idx], &pSecConfig->RSNE_Content[rsne_idx][0],
					END_OF_ARGS);

				FrameLen += TmpLen;
#endif  /*DISABLE_HOSTAPD_PROBE_RESP */
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
			ULONG TmpLen;
			EXT_CAP_INFO_ELEMENT extCapInfo;
			UCHAR extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);

			NdisZeroMemory(&extCapInfo, extInfoLen);

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
#ifdef CONFIG_HOTSPOT_R2
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.WNMNotifyEnable)
				extCapInfo.wnm_notification= 1;
			if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QosMapEnable)
				extCapInfo.qosmap= 1;
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.WNMBTMEnable)
				extCapInfo.BssTransitionManmt = 1;
#endif
#endif

#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
			if(pAd->ApCfg.MBSSID[apidx].GASCtrl.b11U_enable)
				extCapInfo.interworking = 1;
#endif


			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
								1,			&ExtCapIe,
								1,			&extInfoLen,
								extInfoLen, 	&extCapInfo,
								END_OF_ARGS);

			FrameLen += TmpLen;
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
				WmeParmIe[8] =  pBssEdca->EdcaUpdateCount & 0x0f;
#ifdef UAPSD_SUPPORT
				UAPSD_MR_IE_FILL(WmeParmIe[8], &wdev->UapsdInfo);
#endif /* UAPSD_SUPPORT */
				for (i=QID_AC_BE; i<=QID_AC_VO; i++)
				{
					WmeParmIe[10+ (i*4)] = (i << 5) + /* b5-6 is ACI */
										   ((UCHAR)pBssEdca->bACM[i] << 4) +	  /* b4 is ACM */
										   (pBssEdca->Aifsn[i] & 0x0f);		/* b0-3 is AIFSN */
					WmeParmIe[11+ (i*4)] = (pBssEdca->Cwmax[i] << 4) + /* b5-8 is CWMAX */
										   (pBssEdca->Cwmin[i] & 0x0f);	/* b0-3 is CWMIN */
					WmeParmIe[12+ (i*4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);		  /* low byte of TXOP */
					WmeParmIe[13+ (i*4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);		  /* high byte of TXOP */
				}

				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
								  26,						WmeParmIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}

#ifdef DOT11K_RRM_SUPPORT
		if (IS_RRM_ENABLE(pAd, apidx))
			RRM_InsertRRMEnCapIE(pAd, pOutBuffer+FrameLen, &FrameLen, apidx);

		{
			INT loop;
			for (loop=0; loop<MAX_NUM_OF_REGULATORY_CLASS; loop++)
			{
				if (pAd->CommonCfg.RegulatoryClass[loop] == 0)
					break;
				InsertChannelRepIE(pAd, pOutBuffer+FrameLen, &FrameLen,
									(RTMP_STRING *)pAd->CommonCfg.CountryCode,
									pAd->CommonCfg.RegulatoryClass[loop],
									NULL);
			}
		}

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
			UCHAR TmpFrame[256];
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
				MakeOutgoingFrame(pOutBuffer+FrameLen,	  &TmpLen,
						3,					PowerConstraintIE,
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

			NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
			
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
			if (IS_RRM_ENABLE(pAd, apidx) && 
				(pAd->CommonCfg.RegulatoryClass[0] != 0))
			{
				TmpLen2 = 0;
				NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
				RguClass_BuildBcnChList(pAd, TmpFrame, &TmpLen2);
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

			MakeOutgoingFrame(pOutBuffer+FrameLen,		&TmpLen,
							  1,						&CSAIe,
							  1,						&CSALen,
							  1,						&CSAMode,
							  1,						&wdev->channel,
							  1,						&pAd->Dot11_H.CSCount,
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
			MakeOutgoingFrame(pOutBuffer + FrameLen,		&TmpLen,
						  1,								&WpaIe,
							  1,								&epigram_ie_len,
							  4,								&BROADCOM_HTC[0],
							  HtLen,							&pAd->CommonCfg.HtCapability,
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

				MakeOutgoingFrame(pOutBuffer + FrameLen,		 &TmpLen,
								1,								 &WpaIe,
								1,								 &epigram_ie_len,
								4,								 &BROADCOM_HTC[0],
								HtLen,							 &HtCapabilityTmp,
								END_OF_ARGS);
#endif

				FrameLen += TmpLen;

				epigram_ie_len = AddHtLen + 4;
#ifndef RT_BIG_ENDIAN
				MakeOutgoingFrame(pOutBuffer + FrameLen,		  &TmpLen,
								  1,							  &WpaIe,
								  1,							  &epigram_ie_len,
								  4,							  &BROADCOM_AHTINFO[0],
								  AddHtLen, 					  addht,
								  END_OF_ARGS);
#else
				NdisMoveMemory(&addHTInfoTmp, addht, AddHtLen);
				*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
				*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

				MakeOutgoingFrame(pOutBuffer + FrameLen,		 &TmpLen,
								1,								 &WpaIe,
								1,								 &epigram_ie_len,
								4,								 &BROADCOM_AHTINFO[0],
								AddHtLen,						 &addHTInfoTmp,
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
			MakeOutgoingFrame(pOutBuffer+FrameLen,		  &TempLen1,
								7,							  PROVISION_SERVICE_IE,
								END_OF_ARGS);
			FrameLen += TempLen1;
		}

		/* add Simple Config Information Element */
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		if (mbss->WscIEProbeResp.ValueLen)
#else
		if ((mbss->WscControl.WscConfMode > WSC_DISABLE) && (mbss->WscIEProbeResp.ValueLen))
#endif
		{
			ULONG WscTmpLen = 0;
			MakeOutgoingFrame(pOutBuffer+FrameLen,									&WscTmpLen,
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


	/*
		add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back
										 Byte0.b3=1 for rssi-feedback
	*/

	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen));

	{
		// Question to Rorscha: bit4 in old chip is used? but currently is using for 2.4G 256QAM

#ifdef RSSI_FEEDBACK
		UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
		ULONG TmpLen;

		if (ProbeReqParam->bRequestRssi == TRUE)
		{
			MAC_TABLE_ENTRY *pEntry=NULL;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
										PRINT_MAC(ProbeReqParam->Addr2)));

			RalinkSpecificIe[5] |= 0x8;
			pEntry = MacTableLookup(pAd, ProbeReqParam->Addr2);

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
	}
}

VOID CFG80211_SyncPacketWpsIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen, UINT8 apidx, UINT8 *da)
{

	const UCHAR *ssid_ie=NULL, *wsc_ie=NULL;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	EID_STRUCT *eid;
	const UINT WFA_OUI = 0x0050F2;
	PEER_PROBE_REQ_PARAM ProbeReqParam;

	ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, pData, dataLen);
	
	if(ssid_ie != NULL)
	{
	 	eid = (EID_STRUCT*)ssid_ie;
		ProbeReqParam.SsidLen = eid->Len;
		NdisCopyMemory(ProbeReqParam.Ssid, ssid_ie+2, eid->Len);
		NdisCopyMemory(ProbeReqParam.Addr2, da, MAC_ADDR_LEN);
		
	}
	wsc_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, 4,  pData , dataLen);
	if(wsc_ie != NULL)
	{
	
		eid = (EID_STRUCT*)wsc_ie;
		if(eid->Len + 2 <= 500)
		{
			NdisCopyMemory(pMbss->WscIEProbeResp.Value,wsc_ie,eid->Len+2);
			pMbss->WscIEProbeResp.ValueLen = eid->Len + 2;
		}

	}
	ProbeResponseHandler(pAd, &ProbeReqParam,apidx);	
}

#endif /*DISABLE_HOSTAPD_PROBE_RESP */


BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen)
{
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[BSS0].wdev;
	EDCA_PARM *pBssEdca = NULL;

	//hex_dump("probe_rsp_in:", pData, dataLen);
	wmm_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pData, dataLen);

	if (wmm_ie != NULL)
        {
		UINT i = QID_AC_BE;
#ifdef UAPSD_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
				wdev = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX].wdev;
                if (wdev->UapsdInfo.bAPSDCapable == TRUE)
                {
                        wmm_ie[8] |= 0x80;
                }
#endif /* RT_CFG80211_P2P_SUPPORT */
#endif /* UAPSD_SUPPORT */

				pBssEdca = wlan_config_get_ht_edca(wdev);
				if (pBssEdca)
				{
	                /* WMM: sync from driver's EDCA paramter */
	                for (i = QID_AC_BE; i <= QID_AC_VO; i++)
	                {

	                        wmm_ie[10+ (i*4)] = (i << 5) +                                     /* b5-6 is ACI */
	                                            ((UCHAR)pBssEdca->bACM[i] << 4) + /* b4 is ACM */
	                                            (pBssEdca->Aifsn[i] & 0x0f);      /* b0-3 is AIFSN */

	                        wmm_ie[11+ (i*4)] = (pBssEdca->Cwmax[i] << 4) +       /* b5-8 is CWMAX */
	                                            (pBssEdca->Cwmin[i] & 0x0f);      /* b0-3 is CWMIN */
	                        wmm_ie[12+ (i*4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);/* low byte of TXOP */
	                        wmm_ie[13+ (i*4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);  /* high byte of TXOP */
	                }
				}

		return TRUE;
        }
	else
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can't find the wmm ie\n", __FUNCTION__));

	return FALSE;	
}
#endif /* CONFIG_AP_SUPPORT */


INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	if (pAd->Mlme.bStartMcc == TRUE)
	{
//		return;
	}
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */


	if (pData != NULL) 
	{
#ifdef CONFIG_AP_SUPPORT
		struct ieee80211_mgmt *mgmt;
#endif /* CONFIG_AP_SUPPORT */
		{		
			PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

			pCfg80211_ctrl->TxStatusInUsed = TRUE;
			pCfg80211_ctrl->TxStatusSeq = pAd->Sequence;

			if (pCfg80211_ctrl->pTxStatusBuf != NULL)
			{
				os_free_mem(pCfg80211_ctrl->pTxStatusBuf);
				pCfg80211_ctrl->pTxStatusBuf = NULL;
			}

			os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->pTxStatusBuf, Data);
			if (pCfg80211_ctrl->pTxStatusBuf != NULL)
			{
				NdisCopyMemory(pCfg80211_ctrl->pTxStatusBuf, pData, Data);
				pCfg80211_ctrl->TxStatusBufLen = Data;
			}
			else
			{
				pCfg80211_ctrl->TxStatusBufLen = 0;
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG_TX_STATUS: MEM ALLOC ERROR\n"));
				return NDIS_STATUS_FAILURE;
			}
			CFG80211_CheckActionFrameType(pAd, "TX", pData, Data);

#ifdef CONFIG_AP_SUPPORT
    		mgmt = (struct ieee80211_mgmt *)pData;
    		if (ieee80211_is_probe_resp(mgmt->frame_control))
			{
				INT offset = sizeof(HEADER_802_11) + 12;
#ifdef DISABLE_HOSTAPD_PROBE_RESP			
				UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);
				CFG80211_SyncPacketWpsIe(pAd, pData + offset , Data - offset, apidx, mgmt->da);
				return 0;
#endif		
				CFG80211_SyncPacketWmmIe(pAd, pData + offset , Data - offset);
			}
#endif /* CONFIG_AP_SUPPORT */

			MiniportMMRequest(pAd, 0, pData, Data);
		}
	}
	return 0;
}

VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence, BOOLEAN ack)
{
//RTMP_USB_SUPPORT/RTMP_PCI_SUPPORT
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_ctrl->TxStatusInUsed && pCfg80211_ctrl->pTxStatusBuf 
		/*&& (pAd->TxStatusSeq == pHeader->Sequence)*/)
	{
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG_TX_STATUS: REAL send %d\n", Sequence));
		
		CFG80211OS_TxStatus(CFG80211_GetEventDevice(pAd), 5678, 
							pCfg80211_ctrl->pTxStatusBuf, pCfg80211_ctrl->TxStatusBufLen, 
							ack);
		pCfg80211_ctrl->TxStatusSeq = 0;
		pCfg80211_ctrl->TxStatusInUsed = FALSE;
	} 


}
#ifdef CONFIG_AP_SUPPORT
VOID CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, struct wifi_dev *wdev,UCHAR *wpa_ie,UCHAR *rsn_ie)
{
	PEID_STRUCT 		 pEid;
	PUCHAR				pTmp;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	NDIS_802_11_ENCRYPTION_STATUS	PairCipher;		/* Unicast cipher 1, this one has more secured cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	PairCipherAux;	/* Unicast cipher 2 if AP announce two unicast cipher suite */
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	BOOLEAN bWPA = FALSE;
	BOOLEAN bWPA2 = FALSE;
	BOOLEAN bMix = FALSE;

#ifdef DISABLE_HOSTAPD_BEACON
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0))
	const UCHAR CFG_WPA_EID = WLAN_EID_VENDOR_SPECIFIC;
#else
	const UCHAR CFG_WPA_EID = WLAN_EID_WPA;
#endif /* LINUX_VERSION_CODE: 3.8.0 */
#endif

		/* Security */
	PairCipher	 = Ndis802_11WEPDisabled;
	PairCipherAux = Ndis802_11WEPDisabled;

	CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
	CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
	CLEAR_GROUP_CIPHER(&wdev->SecConfig);
	
	if ((wpa_ie == NULL) && (rsn_ie == NULL)) //open case
	{
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:: Open/None case\n", __FUNCTION__));
		//wdev->AuthMode = Ndis802_11AuthModeOpen;
		//wdev->WepStatus = Ndis802_11WEPDisabled;			
		//wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
		
        SET_AKM_OPEN(wdev->SecConfig.AKMMap);
        SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);                
        SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);		
	}

	if ((wpa_ie != NULL)) //wpapsk/tkipaes case
	{
		pEid = (PEID_STRUCT)wpa_ie;
		pTmp = (PUCHAR)pEid;
		if (os_equal_mem(pEid->Octet, WPA_OUI, 4))
		{
			//wdev->AuthMode = Ndis802_11AuthModeOpen;
			//SET_AKM_OPEN(wdev->SecConfig.AKMMap);
			
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:: WPA case\n", __FUNCTION__));
			bWPA = TRUE;
			pTmp   += 11;
				switch (*pTmp)
				{
					case 1:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Group Ndis802_11GroupWEP40Enabled\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled;
						SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
						break;
					case 5:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Group Ndis802_11GroupWEP104Enabled\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled;
						SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
						break;
					case 2:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Group Ndis802_11TKIPEnable\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable;
						SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
						break;
					case 4:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,(" Group Ndis802_11AESEnable\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11AESEnable;
						SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
						break;
					default:
						break;
				}
				/* number of unicast suite*/
				pTmp   += 1;

				/* skip all unicast cipher suites*/
				/*Count = *(PUSHORT) pTmp;				*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;
							break;
						default:
							break;
					}
					if (TmpCipher > PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						PairCipherAux = PairCipher;
						PairCipher	= TmpCipher;
					}
					else
					{
						PairCipherAux = TmpCipher;
					}
					pTmp++;
					Count--;
				}
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Auth Count in WPA = %d ,we only parse the first for AKM\n",Count));
				pTmp   += 3; /* parse first AuthOUI for AKM */
				switch (*pTmp)
				{
					case 1:
						/* Set AP support WPA-enterprise mode*/
						//	wdev->AuthMode = Ndis802_11AuthModeWPA;
						SET_AKM_WPA1(wdev->SecConfig.AKMMap);
						break;
					case 2:
						/* Set AP support WPA-PSK mode*/
						//	wdev->AuthMode = Ndis802_11AuthModeWPAPSK;
						SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
						break;
					default:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("UNKNOWN AKM 0x%x IN WPA,please check!\n",*pTmp));
						break;
				}
				
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("AuthMode = 0x%x\n",wdev->SecConfig.AKMMap));
					//if (wdev->GroupKeyWepStatus == PairCipher)
					if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
						(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
						(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
						(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher))
					)
					{
						//wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
						//pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus;
						wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
					}
					else
					{
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("WPA Mix TKIPAES\n"));

						bMix = TRUE;
					}
				
				pMbss->RSNIE_Len[0] = wpa_ie[1];
				os_move_mem(pMbss->RSN_IE[0], wpa_ie+2, wpa_ie[1]);//copy rsn ie			
#ifdef DISABLE_HOSTAPD_BEACON
				pMbss->RSNIE_ID[0] = CFG_WPA_EID;
#endif
		}
		else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:: wpa open/none case\n", __FUNCTION__));
			//wdev->AuthMode = Ndis802_11AuthModeOpen;		
			//wait until wpa/wpa2 all not exist , then set open/none	
		}	
	}
	if ((rsn_ie != NULL))
	{
		PRSN_IE_HEADER_STRUCT			pRsnHeader;
		PCIPHER_SUITE_STRUCT			pCipher;

		pEid = (PEID_STRUCT)rsn_ie;
		pTmp = (PUCHAR)pEid;
		pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
				
				/* 0. Version must be 1*/
		if (le2cpu16(pRsnHeader->Version) == 1)
		{
			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

			/* 1. Check group cipher*/
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;		

			if (os_equal_mem(pTmp, RSN_OUI, 3))
			{	
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:: WPA2 case\n", __FUNCTION__));
				bWPA2 = TRUE;
				//wdev->AuthMode = Ndis802_11AuthModeOpen;
				//SET_AKM_OPEN(wdev->SecConfig.AKMMap);
					switch (pCipher->Type)
					{
					case 1:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Group Ndis802_11GroupWEP40Enabled\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled;
						SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
						break;
					case 5:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Group Ndis802_11GroupWEP104Enabled\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled;
						SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
						break;
					case 2:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Group Ndis802_11TKIPEnable\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable;
						SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
						break;
					case 4:
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,(" Group Ndis802_11AESEnable\n"));
						//wdev->GroupKeyWepStatus  = Ndis802_11AESEnable;
						SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);   
						break;
					default:
						break;
				}

					/* set to correct offset for next parsing*/
					pTmp   += sizeof(CIPHER_SUITE_STRUCT);

					/* 2. Get pairwise cipher counts*/
					/*Count = *(PUSHORT) pTmp;*/
					Count = (pTmp[1]<<8) + pTmp[0];
					pTmp   += sizeof(USHORT);			

					/* 3. Get pairwise cipher*/
					/* Parsing all unicast cipher suite*/
					while (Count > 0)
					{
						/* Skip OUI*/
						pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
						TmpCipher = Ndis802_11WEPDisabled;
						switch (pCipher->Type)
						{
							case 1:
							case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
								TmpCipher = Ndis802_11WEPEnabled;
								break;
							case 2:
								TmpCipher = Ndis802_11TKIPEnable;
								break;
							case 4:
								TmpCipher = Ndis802_11AESEnable;
								break;
							default:
								break;
						}

						//pMbss->wdev.WepStatus = TmpCipher;
						if (TmpCipher > PairCipher)
						{
							/* Move the lower cipher suite to PairCipherAux*/
							PairCipherAux = PairCipher;
							PairCipher	 = TmpCipher;
						}
						else
						{
							PairCipherAux = TmpCipher;
						}
						pTmp += sizeof(CIPHER_SUITE_STRUCT);
						Count--;
					}

					/* 4. get AKM suite counts*/
					/*Count	= *(PUSHORT) pTmp;*/
					Count = (pTmp[1]<<8) + pTmp[0];
					pTmp   += sizeof(USHORT);

					/* 5. Get AKM ciphers*/
					/* Parsing all AKM ciphers*/
					while (Count > 0)
					{
						pAKM = (PAKM_SUITE_STRUCT) pTmp;
						if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
							break;

						switch (pAKM->Type)
						{
							case 0:
									//wdev->AuthMode = Ndis802_11AuthModeWPANone;
									SET_AKM_OPEN(wdev->SecConfig.AKMMap);
								break;                                                        
							case 1:
								/* Set AP support WPA-enterprise mode*/
									//wdev->AuthMode = Ndis802_11AuthModeWPA2;
									SET_AKM_WPA2(wdev->SecConfig.AKMMap);
								break;
							case 2:                                                      
								/* Set AP support WPA-PSK mode*/
									//wdev->AuthMode = Ndis802_11AuthModeWPA2PSK;
									SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
								break;
							default:
									//wdev->AuthMode = Ndis802_11AuthModeMax;
									SET_AKM_OPEN(wdev->SecConfig.AKMMap);  
								break;
						}
						pTmp   += sizeof(AKM_SUITE_STRUCT);
						Count--;
					}		


#ifdef DISABLE_HOSTAPD_BEACON
					/*check for no pairwise, pmf, ptksa, gtksa counters */
					{
						memcpy(wdev->SecConfig.RsnCap, pTmp, 2);
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Copied Rsn cap %02x %02x \n",wdev->SecConfig.RsnCap[0],wdev->SecConfig.RsnCap[1]));
					}
#endif	
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("AuthMode = 0x%x\n",wdev->SecConfig.AKMMap));
					if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
						(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
						(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
						(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher))
					)
					{
						//wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
						//pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus;
						wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
					}
					else
					{
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("WPA2 Mix TKIPAES\n"));

						bMix = TRUE;
					}			
					
					if (bWPA2 && bWPA)
					{
						pMbss->RSNIE_Len[1] = rsn_ie[1];
						NdisMoveMemory(pMbss->RSN_IE[1], rsn_ie+2, rsn_ie[1]);//copy rsn ie
#ifdef DISABLE_HOSTAPD_BEACON
						pMbss->RSNIE_ID[1] = WLAN_EID_RSN;
#endif
					}
					else
					{
					pMbss->RSNIE_Len[0] = rsn_ie[1];
					os_move_mem(pMbss->RSN_IE[0], rsn_ie+2, rsn_ie[1]);//copy rsn ie			
#ifdef DISABLE_HOSTAPD_BEACON
					pMbss->RSNIE_ID[0] = WLAN_EID_RSN;
#endif
					}
			}
			else {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:: wpa2 Open/None case\n", __FUNCTION__));
				//wdev->AuthMode = Ndis802_11AuthModeOpen;
				//wait until wpa/wpa2 all not exist , then set open/none					
			}
		}
	}

	if (bWPA2 && bWPA)
			{
				//wdev->AuthMode = Ndis802_11AuthModeWPA1PSKWPA2PSK;
				SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
				SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
				if (bMix)
				{
					//wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;
					//wdev->WepStatus = Ndis802_11TKIPAESMix;				
					SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
					SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
				}
			} else if (bWPA2) {
				if (bMix)
				{
					//wdev->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
					//wdev->WepStatus = Ndis802_11TKIPAESMix;
					SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
					SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
					SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
				}		
			} else if (bWPA) {
				if (bMix)
				{
					//wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
					//wdev->WepStatus = Ndis802_11TKIPAESMix;
					SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
					SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
					SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
				}
			}
			else{
				SET_AKM_OPEN(wdev->SecConfig.AKMMap);
				SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);				
				SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);	
			}

		if(IS_AKM_WPA1(wdev->SecConfig.AKMMap) || IS_AKM_WPA2(wdev->SecConfig.AKMMap))
		{
			wdev->SecConfig.IEEE8021X = TRUE;
		}
		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\nCFG80211 BEACON => bwpa2 %d, bwpa %d, bmix %d,AuthMode = %s ,wdev->PairwiseCipher = %s wdev->SecConfig.GroupCipher = %s\n"
		,bWPA2,bWPA,bMix
		,GetAuthModeStr(wdev->SecConfig.AKMMap),GetEncryModeStr(wdev->SecConfig.PairwiseCipher),GetEncryModeStr(wdev->SecConfig.GroupCipher)));
	
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

