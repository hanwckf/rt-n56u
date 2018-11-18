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

#include "rt_config.h"

#ifdef SCAN_SUPPORT


INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev)
{
    INT bw, ch;
    UCHAR BandIdx;


	struct wifi_dev *wdev = pwdev;

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		UCHAR  ScanType = pAd->ScanCtrl.ScanType;
#endif /*APCLI_CERT_SUPPORT*/
#endif /*APCLI_SUPPORT*/

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	PAPCLI_STRUCT pApCliEntry = pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];
	struct wifi_dev *p2p_wdev = &pMbss->wdev;

	if(RTMP_CFG80211_VIF_P2P_GO_ON(pAd) )
	{
		p2p_wdev = &pMbss->wdev;
	}
	else if(RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) )
	{
		p2p_wdev = &pApCliEntry->wdev;
	}
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if(!wdev)
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /*CONFIG_STA_SUPPORT*/

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		pAd->hw_cfg.bbp_bw = decide_phy_bw_by_channel(pAd,wdev->channel);
	}
#endif
	if (pAd->hw_cfg.bbp_bw == BW_20)
		ch = HcGetChannelByRf(pAd,(HcGetBandByWdev(wdev)+1));
	else
		ch = HcGetCentralChByRf(pAd,(HcGetBandByWdev(wdev)+1));	
		
#ifdef WH_EZ_SETUP
#ifdef EZ_MOD_SUPPORT
	if (IS_EZ_SETUP_ENABLED(wdev) && ez_handle_scan_channel_restore(pAd->ApCfg.ScanReqwdev)) {
		
	}
#else
	if (IS_EZ_SETUP_ENABLED(wdev) && pAd->ApCfg.ScanReqwdev->ez_security.scan_one_channel) {
		EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("AP SYNC %s() - Only scan ch.%d and keep original BW setting.\n", 
			__FUNCTION__, pAd->ScanCtrl.Channel));
#ifndef EZ_NETWORK_MERGE_SUPPORT	
		pAd->ApCfg.ScanReqwdev->ez_security.scan_one_channel = FALSE;
#endif
	}
#endif
	else
#endif /* WH_EZ_SETUP */
	{		
		HcBbpSetBwByChannel(pAd, pAd->hw_cfg.bbp_bw,ch);
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s,central ch=%d,bw=%d\n\r",__func__,ch,pAd->hw_cfg.bbp_bw));
	}


	BandIdx = HcGetBandByWdev(wdev);

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	if(INFRA_ON(pAd) && (!RTMP_CFG80211_VIF_P2P_GO_ON(pAd)))
	{
		//this should be resotre to infra sta!!
	       HcBbpSetBwByChannel(pAd, wlan_operate_get_ht_bw(wdev),wdev->channel);
	}
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#else				
        if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
                HcBbpSetBwByChannel(pAd, pAd->hw_cfg.bbp_bw,ch);
#endif /* CONFIG_MULTI_CHANNEL */


	switch(pAd->CommonCfg.BBPCurrentBW)
	{
		case BW_80:
			bw = 80;
			break;
		case BW_40:
			bw = 40;
			break;
		case BW_10:
			bw = 10;
			break;
		case BW_20:
		default:
			bw =20;
			break;
	}

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) && (ch != p2p_wdev->channel) && (p2p_wdev->CentralChannel != 0))
	{
		bw = wlan_operate_get_ht_bw(p2p_wdev);
		HcBbpSetBwByChannel(pAd, bw,p2p_wdev->channel);
	}
	else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) && (ch != p2p_wdev->channel) && (p2p_wdev->CentralChannel != 0))
	{
		bw = wlan_operate_get_ht_bw(p2p_wdev);
		HcBbpSetBwByChannel(pAd, bw,wdev->channel);
	}
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("scan ch restore   ch %d  p2p_wdev->CentralChannel%d \n",ch,p2p_wdev->CentralChannel));
/*If GO start, we need to change to GO Channel*/
	if((ch != p2p_wdev->CentralChannel) && (p2p_wdev->CentralChannel != 0))
		ch = p2p_wdev->CentralChannel;
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */

        ASSERT((ch != 0));
        AsicSwitchChannel(pAd, ch, FALSE); 
        AsicLockChannel(pAd, ch);

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SYNC - End of SCAN, restore to %dMHz channel %d, Total BSS[%02d]\n",
				bw, ch, pAd->ScanTab.BssNr));
		

#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP)
	{
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if (pwdev &&
            pAd->ApCfg.ApCliAutoConnectRunning[pwdev->func_idx] == TRUE &&
            pAd->ScanCtrl.PartialScan.bScanning == FALSE)
		{
			if (!ApCliAutoConnectExec(pAd,pwdev))
			{
				MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error in  %s\n", __FUNCTION__));
			}
		}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		if (pwdev && pwdev->wdev_type == WDEV_TYPE_APCLI)
		{
			pAd->ApCfg.bPartialScanning[pwdev->func_idx] = FALSE;
		}
#endif /* APCLI_SUPPORT */
		pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;

		/* iwpriv set auto channel selection*/
		/* scanned all channels*/
		if (pAd->ApCfg.bAutoChannelAtBootup==TRUE)
		{
			UCHAR RfIC = wmode_2_rfic(wdev->PhyMode);
			wdev->channel = SelectBestChannel(pAd, pAd->ApCfg.AutoChannelAlg);
			pAd->ApCfg.bAutoChannelAtBootup = FALSE;
#ifdef DOT11_N_SUPPORT
			N_ChannelCheck(pAd,wdev->PhyMode,wdev->channel);
#endif /* DOT11_N_SUPPORT */
			APStopByRf(pAd, RfIC);
			APStartUpByRf(pAd, RfIC);
		}

	        if (((pAd->CommonCfg.Channel > 14) &&
	            (pAd->CommonCfg.bIEEE80211H == TRUE) &&
	            RadarChannelCheck(pAd, pAd->CommonCfg.Channel)) &&
	            pAd->Dot11_H.RDMode != RD_SWITCHING_MODE)
	        {
	            if (pAd->Dot11_H.InServiceMonitorCount)
	            {
	                pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
			AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
			AsicEnableBcnSntReq(pAd, wdev);
	            }
	            else
	            {
	                pAd->Dot11_H.RDMode = RD_SILENCE_MODE;
	            }
	        }
	        else
	        {
			AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
			AsicEnableBcnSntReq(pAd, wdev);
	        }
#ifdef APCLI_SUPPORT
#ifdef WSC_AP_SUPPORT
        if(pwdev && 
           (pwdev->wdev_type == WDEV_TYPE_APCLI) && 
           (pwdev->func_idx < MAX_APCLI_NUM))
        {
            WSC_CTRL *pWpsCtrlTemp = &pAd->ApCfg.ApCliTab[pwdev->func_idx].WscControl;
    
            if ((pWpsCtrlTemp->WscConfMode != WSC_DISABLE) && 
                (pWpsCtrlTemp->bWscTrigger == TRUE) && 
                (pWpsCtrlTemp->WscMode == WSC_PBC_MODE))
            {
                if((pWpsCtrlTemp->WscApCliScanMode == TRIGGER_PARTIAL_SCAN))
                {
                    if((pAd->ScanCtrl.PartialScan.bScanning == FALSE) &&
                        (pAd->ScanCtrl.PartialScan.LastScanChannel == 0))
                    {
                         MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                                  ("[%s] %s AP-Client WPS Partial Scan done!!!\n", 
                                  __FUNCTION__, (ch>14?"5G":"2G")));
    
#ifdef CON_WPS
                        if (pWpsCtrlTemp->conWscStatus != CON_WPS_STATUS_DISABLED)
                        {
                            MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_COMPLETE, 0, NULL, pwdev->func_idx);
                            RTMP_MLME_HANDLER(pAd);
                        }
                        else
#endif /* CON_WPS */
                        {
                            if(!pWpsCtrlTemp->WscPBCTimerRunning) 
                            {
                                RTMPSetTimer(&pWpsCtrlTemp->WscPBCTimer, 1000);
                                pWpsCtrlTemp->WscPBCTimerRunning = TRUE;
                            }
                        }
                    }
                }
                else 
                {
#ifdef CON_WPS
                    if (pWpsCtrlTemp->conWscStatus != CON_WPS_STATUS_DISABLED)
                    {
                        MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_COMPLETE, 0, NULL, pwdev->func_idx);
                        RTMP_MLME_HANDLER(pAd);
                    }
#endif /* CON_WPS*/
                }
            }
        }
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */
	}

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if ((pAd->bApCliCertTest == TRUE) && APCLI_IF_UP_CHECK(pAd, 0) && (ScanType == SCAN_2040_BSS_COEXIST))
		{
			UCHAR Status = 1;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("@(%s)  Scan Done ScanType=%d\n", __FUNCTION__, ScanType));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_SCAN_DONE, 2, &Status, 0);			
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* APCLI_CERT_SUPPORT */		
#endif /* APCLI_SUPPORT */	

#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}



static INT scan_active(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType,struct wifi_dev *wdev)
{
	UCHAR *frm_buf = NULL;
	HEADER_802_11 Hdr80211;
	ULONG FrameLen = 0;
	UCHAR SsidLen = 0;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
    UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef CON_WPS
	PWSC_CTRL pWscControl = NULL;
#endif /*CON_WPS*/	


	if (MlmeAllocateMemory(pAd, &frm_buf) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():allocate memory fail\n", __FUNCTION__));

#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
			pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif /* CONFIG_AP_SUPPORT */
		return FALSE;
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (ScanType == SCAN_2040_BSS_COEXIST)
	{
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("SYNC - SCAN_2040_BSS_COEXIST !! Prepare to send Probe Request\n"));
	}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	
	/* There is no need to send broadcast probe request if active scan is in effect.*/
	SsidLen = 0;
	if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE)
#ifdef WSC_STA_SUPPORT
		|| ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_STA))
#endif /* WSC_STA_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
		|| (ScanType == SCAN_P2P)
#endif /* RT_CFG80211_P2P_SUPPORT */
		)
		SsidLen = pAd->ScanCtrl.SsidLen;

#ifdef RT_CFG80211_P2P_SUPPORT
    if (ScanType == SCAN_P2P)
	{
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): this is a p2p scan from cfg80211 layer\n", __FUNCTION__));
		MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
                                      pAd->CurrentAddress, BROADCAST_ADDR);

                MakeOutgoingFrame(frm_buf,               &FrameLen,
                                                  sizeof(HEADER_802_11),    &Hdr80211,
                                                  1,                        &SsidIe,
                                                  1,                        &SsidLen,
                                                  SsidLen,                  pAd->ScanCtrl.Ssid,
                                                  1,                        &SupRateIe,
                                                  1,                        &pAd->cfg80211_ctrl.P2pSupRateLen,
                                                  pAd->cfg80211_ctrl.P2pSupRateLen,  pAd->cfg80211_ctrl.P2pSupRate,
                                                  END_OF_ARGS);
	}
	else
#endif /* RT_CFG80211_P2P_SUPPORT */
	{
#ifdef CONFIG_AP_SUPPORT
		/*IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
		if (OpMode == OPMODE_AP)
		{
			UCHAR *src_mac_addr = NULL;
#ifdef APCLI_SUPPORT
#ifdef WSC_INCLUDED
			if (ScanType == SCAN_WSC_ACTIVE) {
				src_mac_addr = &pAd->ApCfg.ApCliTab[wdev->func_idx].wdev.if_addr[0];
			} 
            else
#endif
#endif
			{//search the first ap interface which use the same band
				INT IdBss = 0;
				for(IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
					if (pAd->ApCfg.MBSSID[IdBss].wdev.DevInfo.Active) {
						if (HcGetBandByWdev(&pAd->ApCfg.MBSSID[IdBss].wdev) == HcGetBandByWdev(wdev))
							break;
					}
				}
				src_mac_addr = &pAd->ApCfg.MBSSID[IdBss].wdev.bssid[0];
			}

			MgtMacHeaderInitExt(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, 
								src_mac_addr,
								BROADCAST_ADDR);

			MakeOutgoingFrame(frm_buf,				 &FrameLen,
							  sizeof(HEADER_802_11),	&Hdr80211,
							  1,						&SsidIe,
							  1,						&SsidLen,
							  SsidLen,					pAd->ScanCtrl.Ssid,
							  1,						&SupRateIe,
							  1,						&wdev->rate.SupRateLen,
							  wdev->rate.SupRateLen,  wdev->rate.SupRate, 
							  END_OF_ARGS);
			
			if (wdev->rate.ExtRateLen)
			{
				ULONG Tmp;
				MakeOutgoingFrame(frm_buf + FrameLen,			 &Tmp,
								  1,								&ExtRateIe,
								  1,								&wdev->rate.ExtRateLen,
								  wdev->rate.ExtRateLen,		  wdev->rate.ExtRate, 
								  END_OF_ARGS);
				FrameLen += Tmp;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}

#ifdef DOT11_N_SUPPORT
	if (WMODE_CAP_N(wdev->PhyMode))
	{
		ULONG	Tmp;
		UCHAR	HtLen;
#ifdef RT_BIG_ENDIAN
		HT_CAPABILITY_IE HtCapabilityTmp;
#endif
#ifdef DOT11_VHT_AC
        struct _build_ie_info vht_ie_info;
#endif /* DOT11_VHT_AC */

		{
			HtLen = sizeof(HT_CAPABILITY_IE);
#ifdef RT_BIG_ENDIAN
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, SIZE_HT_CAP_IE);
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
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

			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &HtCapIe,
							1,                                &HtLen,
							HtLen,                            &HtCapabilityTmp, 
							END_OF_ARGS);
#else
			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &HtCapIe,
							1,                                &HtLen,
							HtLen,                            &pAd->CommonCfg.HtCapability, 
							END_OF_ARGS);
#endif /* RT_BIG_ENDIAN */
		}
		FrameLen += Tmp;

#ifdef DOT11N_DRAFT3
		if ((pAd->ScanCtrl.Channel <= 14) && (pAd->CommonCfg.bBssCoexEnable == TRUE))
		{
			ULONG Tmp;
			HtLen = 1;
			MakeOutgoingFrame(frm_buf + FrameLen,            &Tmp,
							  1,					&ExtHtCapIe,
							  1,					&HtLen,
							  1,          			&pAd->CommonCfg.BSSCoexist2040.word, 
							  END_OF_ARGS);

			FrameLen += Tmp;
		}
#endif /* DOT11N_DRAFT3 */

#ifdef APCLI_SUPPORT
#ifdef WSC_INCLUDED
		if ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP))
		{
			BOOLEAN bHasWscIe = FALSE;
			/* 
				Append WSC information in probe request if WSC state is running
			*/
			if (pAd->ApCfg.ApCliTab[wdev->func_idx].WscControl.bWscTrigger)
			{
				bHasWscIe = TRUE;
			}
#ifdef WSC_V2_SUPPORT
			else if (pAd->ApCfg.ApCliTab[wdev->func_idx].WscControl.WscV2Info.bEnableWpsV2)
			{
				bHasWscIe = TRUE;	
			}
#endif /* WSC_V2_SUPPORT */

			if (bHasWscIe)
			{
				UCHAR		*pWscBuf = NULL, WscIeLen = 0;
				ULONG 		WscTmpLen = 0;

				os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);
				if (pWscBuf != NULL)
				{
					NdisZeroMemory(pWscBuf, 512);
#ifdef CON_WPS
					MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("[scan_active: %d] ConWpsApCliMode = %d\n",
					                                                        __LINE__ ,pAd->ApCfg.ConWpsApCliMode));
					pWscControl = &pAd->ApCfg.ApCliTab[wdev->func_idx].WscControl;
					if (pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED ||
					    pAd->ApCfg.ConWpsApCliMode != CON_WPS_APCLI_BAND_AUTO)
#endif /*CON_WPS*/
					{
						WscBuildProbeReqIE(pAd, STA_MODE, wdev->func_idx, pWscBuf, &WscIeLen);
					}

					MakeOutgoingFrame(frm_buf + FrameLen,              &WscTmpLen,
									WscIeLen,                             pWscBuf,
									END_OF_ARGS);

					FrameLen += WscTmpLen;
					os_free_mem( pWscBuf);
				}
				else
					MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));					
			}
		}
#endif /* WSC_INCLUDED */			
#endif /* APCLI_SUPPORT */

#ifdef DOT11_VHT_AC
        vht_ie_info.frame_buf = (UCHAR *)(frm_buf + FrameLen);
        vht_ie_info.frame_subtype = SUBTYPE_PROBE_REQ;
        vht_ie_info.channel = pAd->ScanCtrl.Channel;
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
        modify_vht_ies(pAd, &vht_ie_info, wdev);

		pAd->CommonCfg.ETxBfEnCond = ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */ 
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */

#ifdef WSC_STA_SUPPORT
	if (OpMode == OPMODE_STA)
	{
		BOOLEAN bHasWscIe = FALSE;
		/* 
			Append WSC information in probe request if WSC state is running
		*/
		if ((pAd->StaCfg[0].WscControl.WscEnProbeReqIE) && 
			(pAd->StaCfg[0].WscControl.WscConfMode != WSC_DISABLE) &&
			(pAd->StaCfg[0].WscControl.bWscTrigger == TRUE))
			bHasWscIe = TRUE;
#ifdef WSC_V2_SUPPORT
		else if ((pAd->StaCfg[0].WscControl.WscEnProbeReqIE) && 
			(pAd->StaCfg[0].WscControl.WscV2Info.bEnableWpsV2))
			bHasWscIe = TRUE;
#endif /* WSC_V2_SUPPORT */


		if (bHasWscIe)
		{
			UCHAR *pWscBuf = NULL, WscIeLen = 0;
			ULONG WscTmpLen = 0;

			os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);
			if (pWscBuf != NULL)
			{
				NdisZeroMemory(pWscBuf, 512);
				WscBuildProbeReqIE(pAd, STA_MODE, 0, pWscBuf, &WscIeLen);

				MakeOutgoingFrame(frm_buf + FrameLen,              &WscTmpLen,
								WscIeLen,                             pWscBuf,
								END_OF_ARGS);

				FrameLen += WscTmpLen;
				os_free_mem(pWscBuf);
			}
			else
				MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));
		}
	}

#endif /* WSC_STA_SUPPORT */


#ifdef WH_EZ_SETUP
		if (ez_is_triband()) {
#ifdef EZ_NETWORK_MERGE_SUPPORT	
				ULONG  tmp_len;
#ifdef EZ_MOD_SUPPORT	
				/*
					Insert capability TLV
				*/
				ez_triband_insert_tlv(pAd, EZ_TAG_CAPABILITY_INFO, 
					frm_buf + FrameLen, 
					&tmp_len);
#else
				unsigned int capability;
	
				capability = cpu2be32(ez_adapter.ez_band_info[0].ez_cli_wdev->ez_security.capability);
			
				/*
					Insert capability TLV
				*/
				ez_insert_tlv(EZ_TAG_CAPABILITY_INFO, 
					(unsigned char *)&capability, 
					EZ_CAPABILITY_LEN, 
					frm_buf + FrameLen, 
					&tmp_len);
#endif
				FrameLen += tmp_len;
#endif		
		}
#endif
	MiniportMMRequest(pAd, 0, frm_buf, FrameLen);
#ifdef MT_MAC_BTCOEX
	if (pAd->BtCoexMode == MT7636_COEX_MODE_TDD)
	{
		MiniportMMRequest(pAd, 0, frm_buf, FrameLen);
	}
#endif


	MlmeFreeMemory( frm_buf);

	return TRUE;
}

#ifdef APCLI_SUPPORT
static void FireExtraProbeReq(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType, 
	struct wifi_dev *wdev,  UCHAR *desSsid, UCHAR desSsidLen)
{
	UCHAR backSsid[MAX_LEN_OF_SSID];
	UCHAR backSsidLen = 0;

	NdisZeroMemory(backSsid, MAX_LEN_OF_SSID);

	//1. backup the original MlmeAux
	backSsidLen = pAd->ScanCtrl.SsidLen;
	NdisCopyMemory(backSsid, pAd->ScanCtrl.Ssid, backSsidLen);
	
	//2. fill the desried ssid into SM
	pAd->ScanCtrl.SsidLen = desSsidLen;
	NdisCopyMemory(pAd->ScanCtrl.Ssid, desSsid, desSsidLen);

	//3. scan action
	scan_active(pAd, OpMode, ScanType, wdev);

	//4. restore to ScanCtrl
	pAd->ScanCtrl.SsidLen  = backSsidLen;
	NdisCopyMemory(pAd->ScanCtrl.Ssid, backSsid, backSsidLen);
}
#endif /* APCLI_SUPPORT */
/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID ScanNextChannel(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev)
{
	UCHAR ScanType = SCAN_TYPE_MAX;
	UINT ScanTimeIn5gChannel = SHORT_CHANNEL_TIME;
	BOOLEAN ScanPending = FALSE;
	RALINK_TIMER_STRUCT *sc_timer = NULL;
	UINT stay_time = 0;
	struct wifi_dev *wdev = pwdev;
#ifdef WH_EZ_SETUP
	CHAR apcli_idx;
	MAC_TABLE_ENTRY *pMacEntry;
#endif
	// TODO: Star, fix me when Scan is prepare to modify

	// TODO: Star, fix me when Scan is prepare to modify
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (wdev == NULL)
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif

#ifdef WH_EZ_SETUP
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	apcli_idx = wdev->func_idx;
	pMacEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[apcli_idx].wdev.bssid);
#endif
#endif
#endif

#ifdef CONFIG_ATE
	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */



#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP)
		ScanType = pAd->ScanCtrl.ScanType;
#endif /* CONFIG_AP_SUPPORT */
	if (ScanType == SCAN_TYPE_MAX) {
#ifdef WH_EZ_SETUP
		if((IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev))
			&& (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)) 
			pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Incorrect ScanType!\n", __FUNCTION__));
		return;
	}
	if ((pAd->ScanCtrl.Channel == 0) || ScanPending) 
	{
#ifdef RT_CFG80211_SUPPORT
		//pAd->cfg80211_ctrl.Cfg80211CurChanIndex--;
#endif /* RT_CFG80211_SUPPORT */
		scan_ch_restore(pAd, OpMode, wdev);

#ifdef WH_EZ_SETUP
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
			if(pMacEntry && pAd->ApCfg.ApCliTab[apcli_idx].Valid 
				&& IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev))
			{
				ApCliRTMPSendNullFrame(pAd,pMacEntry->CurrTxRate, FALSE, pMacEntry, PWR_ACTIVE);
			}
#ifdef EZ_MOD_SUPPORT			
			if (IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev) && wdev->ez_driver_params.ez_scan == TRUE)
			{
				wdev->ez_driver_params.ez_scan = FALSE;
				EZ_DEBUG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Start Join For  wdev-> type = %d wdev = %d\n", wdev->wdev_type,wdev->wdev_idx));
				ApCliIfUp(pAd);
			} 
#else
			if (pAd->ApCfg.ApCliTab[apcli_idx].wdev.enable_easy_setup == TRUE && wdev->ez_security.ez_scan == TRUE)
			{
				wdev->ez_security.ez_scan = FALSE;
				EZ_DEBUG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Start Join For  wdev-> type = %d wdev = %d\n", wdev->wdev_type,wdev->wdev_idx));
				ApCliIfUp(pAd);
				
			} 
#endif
#ifdef MOBILE_APP_SUPPORT
			else if (IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev)){
				int i=0;
				unsigned char bssid[6] = {0};
						
				for (i = 0; i < pAd->ScanTab.BssNr; i++) 
					{
						BSS_ENTRY *bss_entry = NULL;					
						PAPCLI_STRUCT apcli_entry = &pAd->ApCfg.ApCliTab[apcli_idx];
						bss_entry = &pAd->ScanTab.BssEntry[i];
												

						if (bss_entry->non_ez_beacon)
						{
							continue;
						}
						if (bss_entry->support_easy_setup == TRUE)
						{
							continue;		
						}
						if (SSID_EQUAL(apcli_entry->CfgSsid, apcli_entry->CfgSsidLen,bss_entry->Ssid, bss_entry->SsidLen))
						{
							
							COPY_MAC_ADDR(bssid, bss_entry->Bssid);
						}
					}
					if((pAd->ScanCtrl.PartialScan.bScanning == TRUE) && (pAd->ScanCtrl.PartialScan.LastScanChannel != 0)) {
						MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n Dont send event Partial scan running \n"));	
					} else {
						RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, OID_WH_EZ_MAN_TRIBAND_SCAN_COMPLETE_EVENT,
								NULL, bssid, MAC_ADDR_LEN);
					}	
			}
#endif			
#endif	//APCLI_SUPPORT
#endif	//CONFIG_AP_SUPPORT
#endif	// WH_EZ_SETUP

	} 
	else 
	{

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(pAd->ApCfg.ScanReqwdev) && 
#ifdef EZ_MOD_SUPPORT
		pAd->ApCfg.ScanReqwdev->ez_driver_params.scan_one_channel
#else
		pAd->ApCfg.ScanReqwdev->ez_security.scan_one_channel
#endif
		) {
			EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("AP SYNC %s() - Only scan ch.%d and keep original channel setting.\n", 
				__FUNCTION__, pAd->ScanCtrl.Channel));
		}
		else
#endif /* WH_EZ_SETUP */
		{
			AsicSwitchChannel(pAd, pAd->ScanCtrl.Channel, TRUE);
			AsicLockChannel(pAd, pAd->ScanCtrl.Channel);
		}

		{
			BOOLEAN bScanPassive = FALSE;
			if (pAd->ScanCtrl.Channel > 14)
			{
				if ((pAd->CommonCfg.bIEEE80211H == 1)
					&& RadarChannelCheck(pAd, pAd->ScanCtrl.Channel))
					bScanPassive = TRUE;
			}
#ifdef CARRIER_DETECTION_SUPPORT
			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
				bScanPassive = TRUE;
#endif /* CARRIER_DETECTION_SUPPORT */ 

			if (bScanPassive)
			{
				ScanType = SCAN_PASSIVE;
				ScanTimeIn5gChannel = MIN_CHANNEL_TIME;
			}
		}

		/* Check if channel if passive scan under current regulatory domain */
		if (CHAN_PropertyCheck(pAd, pAd->ScanCtrl.Channel, CHANNEL_PASSIVE_SCAN) == TRUE)
			ScanType = SCAN_PASSIVE;

#if defined(DPA_T) || defined(WIFI_REGION32_HIDDEN_SSID_SUPPORT)
		/* Ch 12~14 is passive scan, No matter DFS and 80211H setting is y or n */
		if ((pAd->ScanCtrl.Channel >= 12) && (pAd->ScanCtrl.Channel <= 14))
			ScanType = SCAN_PASSIVE;
#endif /* DPA_T */

#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
			sc_timer = &pAd->ScanCtrl.APScanTimer;
#endif /* CONFIG_AP_SUPPORT */
		if (!sc_timer) {
#ifdef WH_EZ_SETUP
			if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev)
				&& (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)) 
				pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif			
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():ScanTimer not assigned!\n", __FUNCTION__));
			return;
		}
			
		/* We need to shorten active scan time in order for WZC connect issue */
		/* Chnage the channel scan time for CISCO stuff based on its IAPP announcement */
		if (ScanType == FAST_SCAN_ACTIVE)
			stay_time = FAST_ACTIVE_SCAN_TIME;
		else /* must be SCAN_PASSIVE or SCAN_ACTIVE*/
		{

#ifdef CONFIG_AP_SUPPORT
			if ((OpMode == OPMODE_AP) && (pAd->ApCfg.bAutoChannelAtBootup))
				stay_time = AUTO_CHANNEL_SEL_TIMEOUT;
			else
#endif /* CONFIG_AP_SUPPORT */
			if ((WMODE_CAP_2G(wdev->PhyMode) &&
				WMODE_CAP_5G(wdev->PhyMode))
			)
			{
				if (pAd->ScanCtrl.Channel > 14)
					stay_time = ScanTimeIn5gChannel;
				else
					stay_time = MIN_CHANNEL_TIME;

				
#ifdef WH_EZ_SETUP
				if ((IS_EZ_SETUP_ENABLED(wdev)) && ScanType != SCAN_PASSIVE)
					stay_time = FAST_ACTIVE_SCAN_TIME;
#endif
			}
			else {
#if defined (EZ_NETWORK_MERGE_SUPPORT) && defined (WH_EZ_SETUP)
				if (IS_EZ_SETUP_ENABLED(wdev))
				{
					if (ScanType != SCAN_PASSIVE)
						stay_time = FAST_ACTIVE_SCAN_TIME;
					else
						stay_time = MIN_CHANNEL_TIME;
				}
				else
#endif
					stay_time = MAX_CHANNEL_TIME;
			}
		}
		RTMPSetTimer(sc_timer, stay_time);
			
		if (SCAN_MODE_ACT(ScanType))
		{
#ifdef APCLI_SUPPORT
#ifdef WH_EZ_SETUP
            PAPCLI_STRUCT pApCliEntry = NULL;
            UINT index = 0;
#endif
#endif			
			if (scan_active(pAd, OpMode, ScanType, wdev) == FALSE)
			{
#ifdef WH_EZ_SETUP
				if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev)
					&& (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)) 
					pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif
			
				return;
			}

#ifdef APCLI_SUPPORT
#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(wdev)) {
                for(index = 0; index < MAX_APCLI_NUM; index++)
                {
                    pApCliEntry = &pAd->ApCfg.ApCliTab[index];
                    if(pApCliEntry->CfgHideSsidLen > 0){
                        FireExtraProbeReq(pAd,  OpMode, ScanType, wdev,
                            pApCliEntry->CfgHideSsid, pApCliEntry->CfgHideSsidLen);
                    }
                }
			}
#endif
#endif

			{
#ifdef APCLI_SUPPORT
				PAPCLI_STRUCT pApCliEntry = NULL;
				UINT index = 0;
				BOOLEAN needUnicastScan = FALSE;
#ifdef APCLI_AUTO_CONNECT_SUPPORT			
				needUnicastScan = pAd->ApCfg.ApCliAutoConnectRunning[wdev->func_idx];
#endif /* APCLI_AUTO_CONNECT_SUPPORT */			

#ifdef AP_PARTIAL_SCAN_SUPPORT
				needUnicastScan |= pAd->ApCfg.bPartialScanning;
#endif /* AP_PARTIAL_SCAN_SUPPORT */	

				for(index = 0; index < MAX_APCLI_NUM; index++)
				{
					pApCliEntry = &pAd->ApCfg.ApCliTab[index];
					if (needUnicastScan && pApCliEntry->CfgSsidLen > 0)
					{
	    					FireExtraProbeReq(pAd,  OpMode, ScanType, wdev, 
								pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);		
					}
				}
#endif /* APCLI_SUPPORT */
			}

		}

		/* For SCAN_CISCO_PASSIVE, do nothing and silently wait for beacon or other probe reponse*/
		
#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
			pAd->Mlme.ApSyncMachine.CurrState = AP_SCAN_LISTEN;
#endif /* CONFIG_AP_SUPPORT */
	}
}


BOOLEAN ScanRunning(RTMP_ADAPTER *pAd)
{
	BOOLEAN	rv = FALSE;

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			rv = ((pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN) ? TRUE : FALSE);
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	return rv;
}


/* 
	==========================================================================
	Description:

	Return:
		scan_channel - channel to scan.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR FindScanChannel(
	RTMP_ADAPTER *pAd, 
	UINT8 LastScanChannel)
{
	UCHAR scan_channel = 0;

	if (pAd->ScanCtrl.PartialScan.bScanning == TRUE)
	{
		scan_channel = FindPartialScanChannel(pAd);
		return scan_channel;
	}

	if (LastScanChannel == 0)
		scan_channel = FirstChannel(pAd);
	else
		scan_channel = NextChannel(pAd, LastScanChannel);

	return scan_channel;
}


/* 
	==========================================================================
	Description:

	Return:
		scan_channel - channel to scan.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR FindPartialScanChannel(RTMP_ADAPTER *pAd)
{
	UCHAR scan_channel = 0;
	PARTIAL_SCAN *PartialScanCtrl = &pAd->ScanCtrl.PartialScan;
	
	if (PartialScanCtrl->NumOfChannels > 0)
	{
		PartialScanCtrl->NumOfChannels--;
		
		if (PartialScanCtrl->LastScanChannel == 0)
			scan_channel = FirstChannel(pAd);
		else
			scan_channel = NextChannel(pAd, PartialScanCtrl->LastScanChannel);
		
		/* update last scanned channel */
		PartialScanCtrl->LastScanChannel = scan_channel;
		if (scan_channel == 0)
		{
			PartialScanCtrl->BreakTime = 0;
			PartialScanCtrl->bScanning = FALSE;
			PartialScanCtrl->pwdev = NULL;
			PartialScanCtrl->NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
		}
	}
	else
	{
		/* Pending for next partial scan */
		scan_channel = 0;
		PartialScanCtrl->NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	}

#ifdef WH_EZ_SETUP
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd)){
		EZ_DEBUG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, %u, scan_channel = %u,\n", __FUNCTION__, __LINE__, scan_channel));
	} else
#endif // WH_EZ_SETUP
	{
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, %u, scan_channel = %u, NumOfChannels = %u, LastScanChannel = %u, bScanning = %u\n",
			__FUNCTION__, __LINE__,
			scan_channel,
			PartialScanCtrl->NumOfChannels,
			PartialScanCtrl->LastScanChannel,
			PartialScanCtrl->bScanning));
	}
	return scan_channel;
}


INT PartialScanInit(RTMP_ADAPTER *pAd)
{
	PARTIAL_SCAN *PartialScanCtrl = &pAd->ScanCtrl.PartialScan;

	PartialScanCtrl->bScanning = FALSE;
	PartialScanCtrl->NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	PartialScanCtrl->LastScanChannel = 0;
	PartialScanCtrl->BreakTime = 0;
#ifdef WH_EZ_SETUP
	PartialScanCtrl->bPartialScanAllowed = FALSE;
#endif

	return 0;
}

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID ScanParmFill(
	IN RTMP_ADAPTER *pAd,
	IN OUT MLME_SCAN_REQ_STRUCT *ScanReq,
	IN RTMP_STRING Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR BssType,
	IN UCHAR ScanType)
{
	NdisZeroMemory(ScanReq->Ssid, MAX_LEN_OF_SSID);
	ScanReq->SsidLen = (SsidLen > MAX_LEN_OF_SSID) ? MAX_LEN_OF_SSID : SsidLen;
	NdisMoveMemory(ScanReq->Ssid, Ssid, ScanReq->SsidLen);
	ScanReq->BssType = BssType;
	ScanReq->ScanType = ScanType;
}
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */
#endif /* SCAN_SUPPORT */

