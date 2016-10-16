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
static INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType)
{
#ifdef CONFIG_STA_SUPPORT
	USHORT Status;
#endif /* CONFIG_STA_SUPPORT */
	INT bw, ch;
		

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	PAPCLI_STRUCT pApCliEntry = pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];
	struct wifi_dev *p2p_wdev = &pMbss->wdev;
	struct wifi_dev *wdev;

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


#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	if(INFRA_ON(pAd) && (!RTMP_CFG80211_VIF_P2P_GO_ON(pAd)))
	{
		//this should be resotre to infra sta!!
		wdev = &pAd->StaCfg.wdev;
	       bbp_set_bw(pAd, pAd->StaCfg.wdev.bw);
	}
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#else				
        if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
                bbp_set_bw(pAd, pAd->hw_cfg.bbp_bw);
#endif /* CONFIG_MULTI_CHANNEL */



	if (pAd->hw_cfg.bbp_bw == BW_40)
		ch = pAd->CommonCfg.CentralChannel;
        else
		ch = pAd->CommonCfg.Channel;

        ASSERT((ch != 0));
        AsicSwitchChannel(pAd, ch, FALSE); 
        AsicLockChannel(pAd, ch);


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
		bw = p2p_wdev->bw;
		bbp_set_bw(pAd, bw);
	}
	else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) && (ch != p2p_wdev->channel) && (p2p_wdev->CentralChannel != 0))
	{
		bw = p2p_wdev->bw;
		bbp_set_bw(pAd, bw);
	}
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("scan ch restore   ch %d  p2p_wdev->CentralChannel%d \n",ch,p2p_wdev->CentralChannel));
/*If GO start, we need to change to GO Channel*/
	if((ch != p2p_wdev->CentralChannel) && (p2p_wdev->CentralChannel != 0))
		ch = p2p_wdev->CentralChannel;
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */

        ASSERT((ch != 0));
        AsicSwitchChannel(pAd, ch, FALSE); 
        AsicLockChannel(pAd, ch);



	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SYNC - End of SCAN, restore to %dMHz channel %d, Total BSS[%02d]\n",
				bw, ch, pAd->ScanTab.BssNr));
		
#ifdef CONFIG_STA_SUPPORT
	if (OpMode == OPMODE_STA)
	{
		/*
		If all peer Ad-hoc clients leave, driver would do LinkDown and LinkUp.
		In LinkUp, CommonCfg.Ssid would copy SSID from MlmeAux. 
		To prevent SSID is zero or wrong in Beacon, need to recover MlmeAux.SSID here.
		*/
		if (ADHOC_ON(pAd))
		{
			NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pAd->MlmeAux.SsidLen = pAd->CommonCfg.SsidLen;
			NdisMoveMemory(pAd->MlmeAux.Ssid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
		}

		/*
		To prevent data lost.
		Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
		Now, we need to send an NULL data with turned PSM bit off to AP, when scan progress done 
		*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
		{
			/*7636 psm*/
			RTMPSendNullFrame(pAd, 
								pAd->CommonCfg.TxRate, 
								(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
								pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.PwrMgmt.Psm);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s -- Send null frame\n", __FUNCTION__));
		}

		/* keep the latest scan channel, could be 0 for scan complete, or other channel*/
		pAd->StaCfg.LastScanChannel = pAd->ScanCtrl.Channel;

		pAd->StaCfg.ScanChannelCnt = 0;

		/* Suspend scanning and Resume TxData for Fast Scanning*/
		if ((pAd->ScanCtrl.Channel != 0) &&
		(pAd->StaCfg.bImprovedScan))	/* it is scan pending*/
		{
			pAd->Mlme.SyncMachine.CurrState = SCAN_PENDING;
			Status = MLME_SUCCESS;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("bFastRoamingScan ~~~ Get back to send data ~~~\n"));

			RTMPResumeMsduTransmission(pAd);
#ifdef CONFIG_MULTI_CHANNEL
			{
				MLME_SCAN_REQ_STRUCT       ScanReq;
				pAd->StaCfg.LastScanTime = pAd->Mlme.Now32;
				ScanParmFill(pAd, &ScanReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
				MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("bImprovedScan ............. Resume for bImprovedScan, SCAN_PENDING .............. \n"));
				RTMP_MLME_HANDLER(pAd);
			}
#endif /* CONFIG_MULTI_CHANNEL */

		}
		else
		{
			pAd->StaCfg.BssNr = pAd->ScanTab.BssNr;
			pAd->StaCfg.bImprovedScan = FALSE;

			pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
			Status = MLME_SUCCESS;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status, 0);
			RTMP_MLME_HANDLER(pAd);
		}

	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP)
	{
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if (pAd->ApCfg.ApCliAutoConnectRunning == TRUE &&
				pAd->ScanCtrl.PartialScan.bScanning == FALSE)
			{
				if (!ApCliAutoConnectExec(pAd))
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error in  %s\n", __FUNCTION__));
				}
			}			
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */
		pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
		RTMPResumeMsduTransmission(pAd);

#ifdef CON_WPS
		if (pAd->conWscStatus != CON_WPS_STATUS_DISABLED)
		{
			MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_COMPLETE, 0, NULL,0 );
			RTMP_MLME_HANDLER(pAd);
		}
#endif /* CON_WPS*/

		/* iwpriv set auto channel selection*/
		/* scanned all channels*/
		if (pAd->ApCfg.bAutoChannelAtBootup==TRUE)
		{
			pAd->CommonCfg.Channel = SelectBestChannel(pAd, pAd->ApCfg.AutoChannelAlg);
			pAd->ApCfg.bAutoChannelAtBootup = FALSE;
#ifdef DOT11_N_SUPPORT
			N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */
			APStop(pAd);
			APStartUp(pAd);
		}

		if (!((pAd->CommonCfg.Channel > 14) && (pAd->CommonCfg.bIEEE80211H == TRUE) && (pAd->Dot11_H.RDMode != RD_NORMAL_MODE)))
			AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
	}
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (APCLI_IF_UP_CHECK(pAd, 0) && pAd->bApCliCertTest == TRUE && ScanType == SCAN_2040_BSS_COEXIST)
	{
		UCHAR Status=1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("@(%s)  Scan Done ScanType=%d\n", __FUNCTION__, ScanType));
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_SCAN_DONE, sizeof(Status), &Status, 0);			
	}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* APCLI_CERT_SUPPORT */		
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}



static INT scan_active(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType)
{
	UCHAR *frm_buf = NULL;
	HEADER_802_11 Hdr80211;
	ULONG FrameLen = 0;
	UCHAR SsidLen = 0;
#ifdef CONFIG_STA_SUPPORT
	USHORT Status;
#endif /* CONFIG_STA_SUPPORT */


	if (MlmeAllocateMemory(pAd, &frm_buf) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():allocate memory fail\n", __FUNCTION__));
#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA)
		{
			pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
			Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status, 0);
		}
#endif /* CONFIG_STA_SUPPORT */

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
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("SYNC - SCAN_2040_BSS_COEXIST !! Prepare to send Probe Request\n"));
	}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	
	/* There is no need to send broadcast probe request if active scan is in effect.*/
	SsidLen = 0;
#ifndef APCLI_CONNECTION_TRIAL
	if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE)
#ifdef WSC_STA_SUPPORT
		|| ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_STA))
#endif /* WSC_STA_SUPPORT */
		)
		SsidLen = pAd->ScanCtrl.SsidLen;
#endif /* APCLI_CONNECTION_TRIAL */

//CFG_TODO
	{
#ifdef CONFIG_AP_SUPPORT
		/*IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
		if (OpMode == OPMODE_AP)
		{
#ifdef APCLI_SUPPORT
#ifdef WSC_INCLUDED
			if (ScanType == SCAN_WSC_ACTIVE) 
				MgtMacHeaderInitExt(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, 
								pAd->ApCfg.ApCliTab[0].wdev.if_addr,
								BROADCAST_ADDR);	
			else
#endif /* WSC_INCLUDED */						
#endif /* APCLI_SUPPORT */				
			MgtMacHeaderInitExt(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
								pAd->ApCfg.MBSSID[0].wdev.bssid,
								BROADCAST_ADDR);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		/*IF_DEV_CONFIG_OPMODE_ON_STA(pAd) */
		if (OpMode == OPMODE_STA)
		{
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, 
								pAd->CurrentAddress,
								BROADCAST_ADDR);
		}
#endif /* CONFIG_STA_SUPPORT */

		MakeOutgoingFrame(frm_buf,               &FrameLen,
						  sizeof(HEADER_802_11),    &Hdr80211,
						  1,                        &SsidIe,
						  1,                        &SsidLen,
						  SsidLen,			        pAd->ScanCtrl.Ssid,
						  1,                        &SupRateIe,
						  1,                        &pAd->CommonCfg.SupRateLen,
						  pAd->CommonCfg.SupRateLen,  pAd->CommonCfg.SupRate, 
						  END_OF_ARGS);

		if (pAd->CommonCfg.ExtRateLen)
		{
			ULONG Tmp;
			MakeOutgoingFrame(frm_buf + FrameLen,            &Tmp,
							  1,                                &ExtRateIe,
							  1,                                &pAd->CommonCfg.ExtRateLen,
							  pAd->CommonCfg.ExtRateLen,          pAd->CommonCfg.ExtRate, 
							  END_OF_ARGS);
			FrameLen += Tmp;
		}
	}

#ifdef DOT11_N_SUPPORT
	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		ULONG	Tmp;
		UCHAR	HtLen;
#ifdef RT_BIG_ENDIAN
		HT_CAPABILITY_IE HtCapabilityTmp;
#endif

#ifdef CONFIG_STA_SUPPORT
		if ((pAd->bBroadComHT == TRUE) && (OpMode == OPMODE_STA))
		{
			UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};

			HtLen = pAd->MlmeAux.HtCapabilityLen + 4;
#ifdef RT_BIG_ENDIAN
			NdisMoveMemory(&HtCapabilityTmp, &pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
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
							1,                                &WpaIe,
							1,                                &HtLen,
							4,                                &BROADCOM[0],
							pAd->MlmeAux.HtCapabilityLen,     &HtCapabilityTmp, 
							END_OF_ARGS);
#else
			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &WpaIe,
							1,                                &HtLen,
							4,                                &BROADCOM[0],
							pAd->MlmeAux.HtCapabilityLen,     &pAd->MlmeAux.HtCapability, 
							END_OF_ARGS);
#endif /* RT_BIG_ENDIAN */
		}
		else
#endif /* CONFIG_STA_SUPPORT */
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
	}
#endif /* DOT11_N_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef WSC_INCLUDED
	if ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_AP))
	{
		BOOLEAN bHasWscIe = FALSE;
		/* 
			Append WSC information in probe request if WSC state is running
		*/
		if (pAd->ApCfg.ApCliTab[0].WscControl.bWscTrigger)
		{
			bHasWscIe = TRUE;
		}
#ifdef WSC_V2_SUPPORT
		else if (pAd->ApCfg.ApCliTab[0].WscControl.WscV2Info.bEnableWpsV2)
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
				WscBuildProbeReqIE(pAd, STA_MODE, pWscBuf, &WscIeLen);

				MakeOutgoingFrame(frm_buf + FrameLen, &WscTmpLen,
						WscIeLen, pWscBuf,
						END_OF_ARGS);

				FrameLen += WscTmpLen;
				os_free_mem(NULL, pWscBuf);
			}
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));
		}
	}
#endif /* WSC_INCLUDED */			
#endif /* APCLI_SUPPORT */

#ifdef WSC_STA_SUPPORT
	if (OpMode == OPMODE_STA)
	{
		BOOLEAN bHasWscIe = FALSE;
		/* 
			Append WSC information in probe request if WSC state is running
		*/
		if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
			(pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
			(pAd->StaCfg.WscControl.bWscTrigger == TRUE))
			bHasWscIe = TRUE;
#ifdef WSC_V2_SUPPORT
		else if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
			(pAd->StaCfg.WscControl.WscV2Info.bEnableWpsV2))
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
				WscBuildProbeReqIE(pAd, STA_MODE, pWscBuf, &WscIeLen);

				MakeOutgoingFrame(frm_buf + FrameLen,              &WscTmpLen,
								WscIeLen,                             pWscBuf,
								END_OF_ARGS);

				FrameLen += WscTmpLen;
				os_free_mem(NULL, pWscBuf);
			}
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));
		}
	}

#endif /* WSC_STA_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT
	if ((OpMode == OPMODE_STA) &&
		(pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
		(pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen != 0))
	{
		ULONG 		WpsTmpLen = 0;
		
		MakeOutgoingFrame(frm_buf + FrameLen,              &WpsTmpLen,
						pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen,
						pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe,
						END_OF_ARGS);

		FrameLen += WpsTmpLen;
	}
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	if ((OpMode == OPMODE_STA) &&
		(pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
		CFG80211DRV_OpsScanRunning(pAd))
	{
		ULONG 		ExtraIeTmpLen = 0;
		
		MakeOutgoingFrame(frm_buf + FrameLen,              &ExtraIeTmpLen,
						pAd->cfg80211_ctrl.ExtraIeLen,	pAd->cfg80211_ctrl.pExtraIe,
						END_OF_ARGS);

		FrameLen += ExtraIeTmpLen;	
	}
#endif /* CONFIG_STA_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */


	MiniportMMRequest(pAd, 0, frm_buf, FrameLen);

#ifdef CONFIG_STA_SUPPORT
	if (OpMode == OPMODE_STA)
	{
		/*
			To prevent data lost.
			Send an NULL data with turned PSM bit on to current associated AP when SCAN in the channel where
			associated AP located.
		*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && 
			(INFRA_ON(pAd)) &&
			(pAd->CommonCfg.Channel == pAd->ScanCtrl.Channel))
		{
			RTMPSendNullFrame(pAd, 
						  pAd->CommonCfg.TxRate, 
						  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
						  PWR_SAVE);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Send PWA NullData frame to notify the associated AP!\n", __FUNCTION__));
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	MlmeFreeMemory(pAd, frm_buf);

	return TRUE;
}


/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID ScanNextChannel(RTMP_ADAPTER *pAd, UCHAR OpMode)
{
	UCHAR ScanType = SCAN_TYPE_MAX;
	UINT ScanTimeIn5gChannel = SHORT_CHANNEL_TIME;
	BOOLEAN ScanPending = FALSE;
	RALINK_TIMER_STRUCT *sc_timer = NULL;
	UINT stay_time = 0;


#ifdef CONFIG_ATE
	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (MONITOR_ON(pAd))
			return;
	}

	ScanPending = ((pAd->StaCfg.bImprovedScan) && (pAd->StaCfg.ScanChannelCnt>=7));
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP)
		ScanType = pAd->ScanCtrl.ScanType;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (OpMode == OPMODE_STA)
		ScanType = pAd->MlmeAux.ScanType;
#endif /* CONFIG_STA_SUPPORT */
	if (ScanType == SCAN_TYPE_MAX) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Incorrect ScanType!\n", __FUNCTION__));
		return;
	}

#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	/* Since the Channel List is from Upper layer */
	if (CFG80211DRV_OpsScanRunning(pAd))
		pAd->ScanCtrl.Channel = CFG80211DRV_OpsScanGetNextChannel(pAd);
#endif /* CONFIG_STA_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

	if ((pAd->ScanCtrl.Channel == 0) || ScanPending) 
	{
		scan_ch_restore(pAd, OpMode, ScanType);
	} 
	else 
	{
#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA)
		{
			/* BBP and RF are not accessible in PS mode, we has to wake them up first*/
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
				AsicForceWakeup(pAd, TRUE);

			/* leave PSM during scanning. otherwise we may lost ProbeRsp & BEACON*/
			/*7636 psm*/
			if (pAd->StaCfg.PwrMgmt.Psm == PWR_SAVE)
			{
				RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
			}
		}
#endif /* CONFIG_STA_SUPPORT */

		AsicSwitchChannel(pAd, pAd->ScanCtrl.Channel, TRUE);
		AsicLockChannel(pAd, pAd->ScanCtrl.Channel);

#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA)
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

#endif /* CONFIG_STA_SUPPORT */

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
#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA)
			sc_timer = &pAd->MlmeAux.ScanTimer;
#endif /* CONFIG_STA_SUPPORT */
		if (!sc_timer) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():ScanTimer not assigned!\n", __FUNCTION__));
			return;
		}
			
		/* We need to shorten active scan time in order for WZC connect issue */
		/* Chnage the channel scan time for CISCO stuff based on its IAPP announcement */
		if (ScanType == FAST_SCAN_ACTIVE)
			stay_time = FAST_ACTIVE_SCAN_TIME;
		else /* must be SCAN_PASSIVE or SCAN_ACTIVE*/
		{
#ifdef CONFIG_STA_SUPPORT
			pAd->StaCfg.ScanChannelCnt++;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			if ((OpMode == OPMODE_AP) && (pAd->ApCfg.bAutoChannelAtBootup))
				stay_time = AUTO_CHANNEL_SEL_TIMEOUT;
			else
#endif /* CONFIG_AP_SUPPORT */
			if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode) &&
				WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
			{
				if (pAd->ScanCtrl.Channel > 14)
					stay_time = ScanTimeIn5gChannel;
				else
					stay_time = MIN_CHANNEL_TIME;
			}
			else
				stay_time = MAX_CHANNEL_TIME;
		}

#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#ifdef CONFIG_MULTI_CHANNEL
		if(RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
		{
			stay_time = FAST_ACTIVE_SCAN_TIME;
		}
#endif /* CONFIG_MULTI_CHANNEL */
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef CONFIG_STA_SUPPORT
	//CFG_TODO: for testing.
	/* Since the Channel List is from Upper layer */
	if (CFG80211DRV_OpsScanRunning(pAd) && 
	    (pAd->cfg80211_ctrl.Cfg80211ChanListLen == 1))
		stay_time = 500;
#endif /* CONFIG_STA_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */			
				
		RTMPSetTimer(sc_timer, stay_time);
			
		if (SCAN_MODE_ACT(ScanType))
		{

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#if defined(AP_PARTIAL_SCAN_SUPPORT) || defined(APCLI_AUTO_CONNECT_SUPPORT)

			BOOLEAN needUnicastScan = FALSE;
#endif /* defined(AP_PARTIAL_SCAN_SUPPORT) || defined(APCLI_AUTO_CONNECT_SUPPORT) */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			if (scan_active(pAd, OpMode, ScanType) == FALSE)
				return;

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#if defined(AP_PARTIAL_SCAN_SUPPORT) || defined(APCLI_AUTO_CONNECT_SUPPORT)
			
#ifdef APCLI_AUTO_CONNECT_SUPPORT			
			needUnicastScan = pAd->ApCfg.ApCliAutoConnectRunning;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */			

#ifdef AP_PARTIAL_SCAN_SUPPORT
			needUnicastScan |= pAd->ApCfg.bPartialScanning;
#endif /* AP_PARTIAL_SCAN_SUPPORT */			

			if (needUnicastScan)
			{
				/* Enhance Connectivity & for Hidden Ssid Scanning*/
				CHAR backSsid[MAX_LEN_OF_SSID];
				UCHAR desiredSsidLen, backSsidLen;
			    UINT index;

			    for(index = 0; index < MAX_APCLI_NUM; index++)
				{
					NdisZeroMemory(backSsid, MAX_LEN_OF_SSID);
					desiredSsidLen= pAd->ApCfg.ApCliTab[index].CfgSsidLen;

					if (desiredSsidLen  > 0)
					{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: specific the %s scanning\n", __FUNCTION__, 
							pAd->ApCfg.ApCliTab[index].CfgSsid));

						/* 1. backup the original MlmeAux */
						backSsidLen = pAd->ScanCtrl.SsidLen;
						NdisCopyMemory(backSsid, pAd->ScanCtrl.Ssid, backSsidLen);
					
						/* 2. fill the desried ssid into SM */
						pAd->ScanCtrl.SsidLen = desiredSsidLen;
						NdisCopyMemory(pAd->ScanCtrl.Ssid, pAd->ApCfg.ApCliTab[index].CfgSsid, desiredSsidLen);

						/* 3. scan action */
						scan_active(pAd, OpMode, ScanType);
			
						/* 4. restore to MlmeAux */
						pAd->ScanCtrl.SsidLen = backSsidLen;
						NdisCopyMemory(pAd->ScanCtrl.Ssid, backSsid, backSsidLen);
					}
				}
			}
	
#endif /* AP_PARTIAL_SCAN_SUPPORT || APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		}

		/* For SCAN_CISCO_PASSIVE, do nothing and silently wait for beacon or other probe reponse*/
		
#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA)
			pAd->Mlme.SyncMachine.CurrState = SCAN_LISTEN;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
			pAd->Mlme.ApSyncMachine.CurrState = AP_SCAN_LISTEN;
#endif /* CONFIG_AP_SUPPORT */
	}
}


BOOLEAN ScanRunning(RTMP_ADAPTER *pAd)
{
	BOOLEAN	rv = FALSE;

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if ((pAd->Mlme.SyncMachine.CurrState == SCAN_LISTEN) || (pAd->Mlme.SyncMachine.CurrState == SCAN_PENDING))
				rv = TRUE;
		}
#endif /* CONFIG_STA_SUPPORT */
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
			PartialScanCtrl->bScanning = FALSE;
			PartialScanCtrl->NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
		}
	}
	else
	{
		/* Pending for next partial scan */
		scan_channel = 0;
		PartialScanCtrl->NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, %u, scan_channel = %u, NumOfChannels = %u, LastScanChannel = %u, bScanning = %u\n",
			__FUNCTION__, __LINE__,
			scan_channel,
			PartialScanCtrl->NumOfChannels,
			PartialScanCtrl->LastScanChannel,
			PartialScanCtrl->bScanning));
	return scan_channel;
}


INT PartialScanInit(RTMP_ADAPTER *pAd)
{
	PARTIAL_SCAN *PartialScanCtrl = &pAd->ScanCtrl.PartialScan;

	PartialScanCtrl->bScanning = FALSE;
	PartialScanCtrl->NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	PartialScanCtrl->LastScanChannel = 0;
	PartialScanCtrl->BreakTime = 0;

	return 0;
}
#endif /* SCAN_SUPPORT */

