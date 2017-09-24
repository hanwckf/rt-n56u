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
    assoc.c
 
    Abstract:
    Handle association related requests either from WSTA or from local MLME
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"


static void ap_assoc_info_debugshow(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN isReassoc,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS *ie_list)
{
#ifdef DBG
	PUCHAR	sAssoc = isReassoc ? (PUCHAR)"ReASSOC" : (PUCHAR)"ASSOC";
#endif /* DBG */
	MULTISSID_STRUCT *wdev;


	wdev = &pAd->ApCfg.MBSSID[pEntry->apidx];
	DBGPRINT(RT_DEBUG_TRACE, ("%s - \n\tAssign AID=%d to STA %02x:%02x:%02x:%02x:%02x:%02x\n",
		sAssoc, pEntry->Aid, PRINT_MAC(pEntry->Addr)));

#ifdef DOT11_N_SUPPORT
	if (ie_list->ht_cap_len && WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		assoc_ht_info_debugshow(pAd, pEntry, ie_list->ht_cap_len, &ie_list->HTCapability);

		DBGPRINT(RT_DEBUG_TRACE, ("\n%s - Update AP OperaionMode=%d , fAnyStationIsLegacy=%d, fAnyStation20Only=%d, fAnyStationNonGF=%d\n\n",
					sAssoc, 
					pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, 
					pAd->MacTab.fAnyStationIsLegacy,
					pAd->MacTab.fAnyStation20Only, 
					pAd->MacTab.fAnyStationNonGF));

#ifdef DOT11N_DRAFT3
		DBGPRINT(RT_DEBUG_TRACE, ("\tExt Cap Info: \n"));
		DBGPRINT(RT_DEBUG_TRACE, ("\t\tBss2040CoexistMgmt=%d\n", pEntry->BSS2040CoexistenceMgmtSupport));
#endif /* DOT11N_DRAFT3 */
#ifdef DOT11_VHT_AC
		if ((ie_list->vht_cap_len) &&
			WMODE_CAP_N(pAd->CommonCfg.PhyMode) &&
			(pAd->CommonCfg.Channel > 14))
		{
			assoc_vht_info_debugshow(pAd, pEntry, &ie_list->vht_cap, NULL);
		}
#endif /* DOT11_VHT_AC */
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s - legacy STA\n", sAssoc));
		DBGPRINT(RT_DEBUG_TRACE, ("\n%s - MODE=%d, MCS=%d\n", sAssoc,
								pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.MCS));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("\tAuthMode=%d, WepStatus=%d, WpaState=%d, GroupKeyWepStatus=%d\n",
		pEntry->AuthMode, pEntry->WepStatus, pEntry->WpaState, wdev->GroupKeyWepStatus));

	DBGPRINT(RT_DEBUG_TRACE, ("\tWMMCap=%d, RalinkAgg=%d, PiggyBack=%d, RDG=%d, TxAMSDU=%d, IdleTimeout=%d\n",
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE),
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE),
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE),
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE),
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED),
		pEntry->StaIdleTimeout));

}


static USHORT update_associated_mac_entry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS *ie_list,
	IN UCHAR MaxSupportedRate)
{
	MULTISSID_STRUCT *wdev;
#ifdef TXBF_SUPPORT
	BOOLEAN	 supportsETxBF = FALSE;
#endif // TXBF_SUPPORT //


	wdev = &pAd->ApCfg.MBSSID[pEntry->apidx];

	/* Update auth, wep, legacy transmit rate setting . */
	pEntry->Sst = SST_ASSOC;

	pEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxTxRate, MaxSupportedRate);
	
	set_entry_phy_cfg(pAd, pEntry);

	pEntry->CapabilityInfo = ie_list->CapabilityInfo;

	if ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->WpaState = AS_INITPSK;
	}
#ifdef DOT1X_SUPPORT
	else if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) ||
			(pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
			(wdev->IEEE8021X == TRUE))
	{
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->WpaState = AS_AUTHENTICATION;
	}
#endif /* DOT1X_SUPPORT */
#ifdef WAPI_SUPPORT
	else if ((pEntry->AuthMode == Ndis802_11AuthModeWAICERT) || 
			 (pEntry->AuthMode == Ndis802_11AuthModeWAIPSK))
	{
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->WpaState = AS_AUTHENTICATION2;
	}
#endif /* WAPI_SUPPORT */
    
	/*if (ClientRalinkIe & 0x00000004) */
	if (ie_list->RalinkIe != 0x0)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);
	else
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);
	

	/* Ralink proprietary Piggyback and Aggregation support for legacy RT61 chip */
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
#ifdef AGGREGATION_SUPPORT
	if ((pAd->CommonCfg.bAggregationCapable) && (ie_list->RalinkIe & 0x00000001))
	{
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC -RaAggregate= 1\n"));
	}
#endif /* AGGREGATION_SUPPORT */
#ifdef PIGGYBACK_SUPPORT
	if ((pAd->CommonCfg.bPiggyBackCapable) && (ie_list->RalinkIe & 0x00000002))
	{
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC -PiggyBack= 1\n"));
	}
#endif /* PIGGYBACK_SUPPORT */

	/* In WPA or 802.1x mode, the port is not secured, otherwise is secued. */
	if ((pEntry->AuthMode >= Ndis802_11AuthModeWPA) 
#ifdef DOT1X_SUPPORT
		|| (wdev->IEEE8021X == TRUE)
#endif /* DOT1X_SUPPORT */
	)
		pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	else
	{
		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
	}

#ifdef SOFT_ENCRYPT
	/* There are some situation to need to encryption by software 			   	
	   1. The Client support PMF. It shall ony support AES cipher.
	   2. The Client support WAPI.
	   If use RT3883 or later, HW can handle the above.	
	   */
#ifdef WAPI_SUPPORT
	if (!(IS_HW_WAPI_SUPPORT(pAd)) 
		&& (pEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled))
	{
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT);
	}
#endif /* WAPI_SUPPORT */

#endif /* SOFT_ENCRYPT */

#ifdef DOT11_N_SUPPORT
	/* 	
		WFA recommend to restrict the encryption type in 11n-HT mode.
	 	So, the WEP and TKIP are not allowed in HT rate.
	*/
	if (pAd->CommonCfg.HT_DisallowTKIP &&
		IS_INVALID_HT_SECURITY(pEntry->WepStatus))
	{
		/* Force to None-HT mode due to WiFi 11n policy */
		ie_list->ht_cap_len = 0;
#ifdef DOT11_VHT_AC
		ie_list->vht_cap_len = 0;
#endif /* DOT11_VHT_AC */
		DBGPRINT(RT_DEBUG_TRACE, ("%s : Force the STA as Non-HT mode\n", __FUNCTION__));
	}

	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if ((ie_list->ht_cap_len != 0) && 
		(wdev->DesiredHtPhyInfo.bHtEnable) &&
		WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		ht_mode_adjust(pAd, pEntry, &ie_list->HTCapability, &pAd->CommonCfg.DesiredHtPhy);

#ifdef DOT11N_DRAFT3
		if (ie_list->ExtCapInfo.BssCoexistMgmtSupport)
			pEntry->BSS2040CoexistenceMgmtSupport = 1;
#endif /* DOT11N_DRAFT3 */

#ifdef DOT11N_DRAFT3
		/* 40Mhz BSS Width Trigger events */
		if (ie_list->HTCapability.HtCapInfo.Forty_Mhz_Intolerant)
		{
			pEntry->bForty_Mhz_Intolerant = TRUE;
			pAd->MacTab.fAnyStaFortyIntolerant = TRUE;
			if(((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) && 
				(pAd->CommonCfg.Channel <=14)) &&
			    ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
				(pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth != 0) && 
				(pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset != 0))
			)
			{
				pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
				pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
				pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
				pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("pEntry set 40MHz Intolerant as 1\n"));
			Handle_BSS_Width_Trigger_Events(pAd);
		}
#endif /* DOT11N_DRAFT3 */

#ifdef TXBF_SUPPORT
		supportsETxBF = clientSupportsETxBF(pAd, &ie_list->HtCapability.TxBFCap);
#endif /* TXBF_SUPPORT */

		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0],
														&ie_list->HTCapability.MCSSet[0]);
		 
		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("@@@ IF-ra%d DesiredTransmitSetting.field.MCS = %d\n",
								pEntry->apidx,
								wdev->DesiredTransmitSetting.field.MCS));

			set_ht_fixed_mcs(pAd, pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
		}

		pEntry->MaxHTPhyMode.field.STBC = (ie_list->HTCapability.HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
		// TODO: shiang-6590, check if this is necessary here, perforce didn't have this
		if (ie_list->HTCapability.HtCapParm.MpduDensity < 5)
			ie_list->HTCapability.HtCapParm.MpduDensity = 5;

		pEntry->MpduDensity = ie_list->HTCapability.HtCapParm.MpduDensity;
		pEntry->MaxRAmpduFactor = ie_list->HTCapability.HtCapParm.MaxRAmpduFactor;
		pEntry->MmpsMode = (UCHAR)ie_list->HTCapability.HtCapInfo.MimoPs;
		pEntry->AMsduSize = (UCHAR)ie_list->HTCapability.HtCapInfo.AMsduSize;

		if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);
		if (ie_list->HTCapability.HtCapInfo.ShortGIfor20)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);
		if (ie_list->HTCapability.HtCapInfo.ShortGIfor40)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);
		if (ie_list->HTCapability.HtCapInfo.TxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
		if (ie_list->HTCapability.HtCapInfo.RxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
		if (ie_list->HTCapability.ExtHtCapInfo.PlusHTC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HTC_CAPABLE);
		if (pAd->CommonCfg.bRdg && ie_list->HTCapability.ExtHtCapInfo.RDGSupport)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);
		if (ie_list->HTCapability.ExtHtCapInfo.MCSFeedback == 0x03)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);

		/* Record the received capability from association request */
		NdisMoveMemory(&pEntry->HTCapability, &ie_list->HTCapability, sizeof(HT_CAPABILITY_IE));

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
			(pAd->CommonCfg.Channel > 14) &&
			(ie_list->vht_cap_len))
		{
			VHT_CAP_INFO *vht_cap_info = &ie_list->vht_cap.vht_cap;

			vht_mode_adjust(pAd, pEntry, &ie_list->vht_cap, (ie_list->vht_op_len == 0) ? NULL : &ie_list->vht_op);

			if ((ie_list->vht_cap.mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_9) && 
				(pAd->CommonCfg.vht_max_mcs_cap == VHT_MCS_CAP_9))
			{
#ifdef MT76x0
				/* 
					MCS[3:0]=0~8 are supported for BW = VHT 20Mhz
					MCS[3:0]=0~9 are supported for BW = VHT 40 or 80Mhz
				*/
				if (IS_MT76x0(pAd) && 
					(pEntry->MaxHTPhyMode.field.BW == BW_20))
					pEntry->MaxHTPhyMode.field.MCS = 8;
				else
#endif /* MT76x0 */
				pEntry->MaxHTPhyMode.field.MCS = 9;
			}
			else if ((ie_list->vht_cap.mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_8) &&
				(pAd->CommonCfg.vht_max_mcs_cap == VHT_MCS_CAP_8))
				pEntry->MaxHTPhyMode.field.MCS = 8;

			if (vht_cap_info->sgi_80M)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI80_CAPABLE);

			if (vht_cap_info->sgi_160M)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI160_CAPABLE);

			if (pAd->CommonCfg.vht_stbc)
			{
				if (vht_cap_info->tx_stbc)
					CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_TXSTBC_CAPABLE);
				if (vht_cap_info->rx_stbc)
					CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE);
			}
			NdisMoveMemory(&pEntry->vht_cap_ie, &ie_list->vht_cap, sizeof(VHT_CAP_IE));
		}
#endif /* DOT11_VHT_AC */

	}
	else
	{
		pAd->MacTab.fAnyStationIsLegacy = TRUE;
		NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
#ifdef DOT11_VHT_AC
		// TODO: shiang-usw, it's ugly and need to revise it
		NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));
		NdisZeroMemory(&pEntry->SupportVHTMCS, sizeof(pEntry->SupportVHTMCS));
		pEntry->SupportRateMode &= (~SUPPORT_VHT_MODE);
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */

	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

	pEntry->CurrTxRate = pEntry->MaxSupportedRate;

#ifdef MFB_SUPPORT
	pEntry->lastLegalMfb = 0;
	pEntry->isMfbChanged = FALSE;
	pEntry->fLastChangeAccordingMfb = FALSE;

	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = 0;/*has to increment whenever a mrq is sent */
	pEntry->mrqCnt = 0;

	pEntry->pendingMfsi = 0;

	pEntry->toTxMfb = FALSE;
	pEntry->mfbToTx = 0;
	pEntry->mfb0 = 0;
	pEntry->mfb1 = 0;
#endif	/* MFB_SUPPORT */

	pEntry->freqOffsetValid = FALSE;

#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgHwTxBfCap)
		TxBFInit(pAd, pEntry, supportsETxBF);
#endif // TXBF_SUPPORT //

	// Initialize Rate Adaptation
	MlmeRAInit(pAd, pEntry);

	/* Set asic auto fall back */
	if (wdev->bAutoTxRateSwitch == TRUE)
	{
		UCHAR TableSize = 0;

		pEntry->bAutoTxRateSwitch = TRUE;

		MlmeSelectTxRateTable(pAd, pEntry, &pEntry->pTable, &TableSize, &pEntry->CurrTxRateIndex);
		MlmeNewTxRate(pAd, pEntry);


#ifdef NEW_RATE_ADAPT_SUPPORT
		if (! ADAPT_RATE_TABLE(pEntry->pTable))
#endif /* NEW_RATE_ADAPT_SUPPORT */
			pEntry->HTPhyMode.field.ShortGI = GI_800;
	}
	else
	{
		pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
		pEntry->bAutoTxRateSwitch = FALSE;
		
		/* If the legacy mode is set, overwrite the transmit setting of this entry. */
		RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}


	if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
		ApLogEvent(pAd, pEntry->Addr, EVENT_ASSOCIATED);

	APUpdateCapabilityAndErpIe(pAd);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd);
#endif /* DOT11_N_SUPPORT */

	pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;

#ifdef HOSTAPD_SUPPORT
	if((wdev->Hostapd == TRUE) &&
	    ((wdev->AuthMode >= Ndis802_11AuthModeWPA) || wdev->IEEE8021X))
	{
		RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED,
					-1, pEntry->Addr, NULL, 0,
					((pEntry->CapabilityInfo & 0x0010) == 0 ? 0xFFFD : 0xFFFC));
	}
#endif /*HOSTAPD_SUPPORT*/

	return MLME_SUCCESS;
}


/*
    ==========================================================================
    Description:
       assign a new AID to the newly associated/re-associated STA and
       decide its MaxSupportedRate and CurrTxRate. Both rates should not
       exceed AP's capapbility
    Return:
       MLME_SUCCESS - association successfully built
       others - association failed due to resource issue
    ==========================================================================
 */
static USHORT APBuildAssociation(
    IN RTMP_ADAPTER *pAd,
    IN MAC_TABLE_ENTRY *pEntry,
    IN IE_LISTS *ie_list,
    IN UCHAR MaxSupportedRateIn500Kbps,
    OUT USHORT *pAid)
{
	USHORT StatusCode = MLME_SUCCESS;
	UCHAR MaxSupportedRate = RATE_11;
	MULTISSID_STRUCT *wdev;


	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

    if ((WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_G) 
#ifdef DOT11_N_SUPPORT
		|| WMODE_EQUAL(pAd->CommonCfg.PhyMode, (WMODE_G | WMODE_GN))
#endif /* DOT11_N_SUPPORT */
		) 
		&& (MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	)
		return MLME_ASSOC_REJ_DATA_RATE;

#ifdef DOT11_N_SUPPORT
	/* 11n only */
	if (WMODE_HT_ONLY(pAd->CommonCfg.PhyMode)&& (ie_list->ht_cap_len == 0))
		return MLME_ASSOC_REJ_DATA_RATE;
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
		(pAd->CommonCfg.Channel > 14) &&
		(ie_list->vht_cap_len == 0) &&
		(pAd->CommonCfg.bNonVhtDisallow))
		return MLME_ASSOC_REJ_DATA_RATE;
#endif /* DOT11_VHT_AC */

	if (!pEntry)
		return MLME_UNSPECIFY_FAIL;

	if (pEntry && ((pEntry->Sst == SST_AUTH) || (pEntry->Sst == SST_ASSOC)))
	{
		/* TODO: */
		/* should qualify other parameters, for example - capablity, supported rates, listen interval, ... etc */
		/* to decide the Status Code */
		/**pAid = APAssignAid(pAd, pEntry); */
		/*pEntry->Aid = *pAid; */
		*pAid = pEntry->Aid;
		pEntry->NoDataIdleCount = 0;
		pEntry->StaConnectTime = 0;
        
#ifdef WSC_AP_SUPPORT
		if (pEntry->bWscCapable == FALSE)
#endif /* WSC_AP_SUPPORT */
		{
			/* check the validity of the received RSNIE */
			if ((StatusCode = APValidateRSNIE(pAd, pEntry, &ie_list->RSN_IE[0], ie_list->RSNIE_Len)) != MLME_SUCCESS)
				return StatusCode;
		}

		NdisMoveMemory(pEntry->RSN_IE, &ie_list->RSN_IE[0], ie_list->RSNIE_Len);
		pEntry->RSNIE_Len = ie_list->RSNIE_Len;


		wdev = &pAd->ApCfg.MBSSID[pEntry->apidx];
		if (*pAid == 0)
			StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		else if ((pEntry->RSNIE_Len == 0) &&
				(wdev->AuthMode >= Ndis802_11AuthModeWPA) 
#ifdef HOSTAPD_SUPPORT
				&& (wdev->Hostapd == TRUE)
#endif
#ifdef RT_CFG80211_SUPPORT
				&& (wdev->CFG_HOSTAPD == TRUE)
#endif /*RT_CFG80211_SUPPORT*/

		)
		{
#ifdef WSC_AP_SUPPORT
	            if (((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
				pEntry->bWscCapable
#ifdef WSC_V2_SUPPORT
	                && (wdev->WscControl.WscV2Info.bWpsEnable || 
	                	(wdev->WscControl.WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
	                )
#ifdef HOSTAPD_SUPPORT
					|| (wdev->Hostapd == TRUE)
#endif /*HOSTAPD_SUPPORT*/
#ifdef RT_CFG80211_SUPPORT
					|| (wdev->CFG_HOSTAPD== TRUE)
#endif /*RT_CFG80211_SUPPORT*/

	                )
	            {
	                pEntry->Sst = SST_ASSOC;
	                StatusCode = MLME_SUCCESS;
	                /* In WPA or 802.1x mode, the port is not secured. */
	    			if ((pEntry->AuthMode >= Ndis802_11AuthModeWPA) 
#ifdef DOT1X_SUPPORT
					|| (wdev->IEEE8021X == TRUE)
#endif /* DOT1X_SUPPORT */					
				)
	    				pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	    			else
	    				pEntry->PortSecured = WPA_802_1X_PORT_SECURED;

				if ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) ||
					(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
	    			{
	    				pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	    				pEntry->WpaState = AS_INITPSK;
	    			}
#ifdef DOT1X_SUPPORT				
	    			else if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) ||
						(pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
						(wdev->IEEE8021X == TRUE))
	    			{
	    				pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	    				pEntry->WpaState = AS_AUTHENTICATION;
	    			}
#endif /* DOT1X_SUPPORT */					
	            }
	            else
	            {
	                DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - WSC_STATE_MACHINE is OFF.<WscConfMode = %d, apidx =%d>\n",
	                                         wdev->WscControl.WscConfMode,
	                                         pEntry->apidx));
	                StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
	            }
#else  /* WSC_AP_SUPPORT */
			StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
#endif /* WSC_AP_SUPPORT */

#ifdef HOSTAPD_SUPPORT
			if(wdev->Hostapd == TRUE
				&& (wdev->AuthMode >= Ndis802_11AuthModeWPA 
				|| wdev->IEEE8021X))
			{

				RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED,
						-1, pEntry->Addr, NULL, 0,
						((pEntry->CapabilityInfo & 0x0010) == 0 ? 0xFFFD : 0xFFFC));
			}
#endif /*HOSTAPD_SUPPORT*/

		}
		else
		{
			StatusCode = update_associated_mac_entry(pAd, pEntry, ie_list, MaxSupportedRate);
		}
	}
	else /* CLASS 3 error should have been handled beforehand; here should be MAC table full */
		StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;

	if (StatusCode == MLME_SUCCESS)
	{
		if (ie_list->bWmmCapable)
		{
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		}
		else
		{
			CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		}
	}

    return StatusCode;
}


#ifdef IAPP_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Update Frame to update forwarding table in Layer 2 devices.

 Arguments:
    *mac_p - the STATION MAC address pointer

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
static BOOLEAN IAPP_L2_Update_Frame_Send(
	IN PRTMP_ADAPTER	pAd,
    IN UINT8 *mac_p,
    IN INT  bssid)
{

	NDIS_PACKET	*pNetBuf;

	pNetBuf = RtmpOsPktIappMakeUp(get_netdev_from_bssid(pAd, bssid), mac_p);
	if (pNetBuf == NULL)
		return FALSE;

    /* UCOS: update the built-in bridge, too (don't use gmac.xmit()) */
    announce_802_3_packet(pAd, pNetBuf, OPMODE_AP);

	IAPP_L2_UpdatePostCtrl(pAd, mac_p, bssid);

    return TRUE;
} /* End of IAPP_L2_Update_Frame_Send */
#endif /* IAPP_SUPPORT */


VOID ap_cmm_peer_assoc_req_action(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem,
	IN BOOLEAN isReassoc)
{
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocRspHdr;
	USHORT CapabilityInfoForAssocResp;
	USHORT StatusCode = MLME_SUCCESS;
	USHORT Aid;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	UCHAR MaxSupportedRate = 0;
	UCHAR SupRateLen, PhyMode, FlgIs11bSta;
	UCHAR i;
	MAC_TABLE_ENTRY *pEntry;
#ifdef DBG
	UCHAR *sAssoc = isReassoc ? (PUCHAR)"ReASSOC" : (PUCHAR)"ASSOC";
#endif /* DBG */
	UCHAR SubType;
	BOOLEAN bACLReject = FALSE;
#ifdef DOT1X_SUPPORT
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#endif /* DOT1X_SUPPORT */	
	MULTISSID_STRUCT *wdev;
#ifdef RT_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*RT_BIG_ENDIAN*/

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));
	if (ie_list == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): mem alloc failed\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory(ie_list, sizeof(IE_LISTS));

	if (!PeerAssocReqCmmSanity(pAd, isReassoc, Elem->Msg, Elem->MsgLen, ie_list))
		goto LabelOK;

	/* check if AP address is same as us */
	/* TODO */
	/* goto label_err; */

	pEntry = MacTableLookup(pAd, ie_list->Addr2);
	if (!pEntry) {
		DBGPRINT(RT_DEBUG_ERROR, ("NoAuth MAC - %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(ie_list->Addr2)));
		goto LabelOK;
	}
	
	if (!VALID_MBSS(pAd, pEntry->apidx)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():pEntry bounding invalid wdev(apidx=%d)\n",
					__FUNCTION__, pEntry->apidx));
		goto LabelOK;
	}
	
	wdev = &pAd->ApCfg.MBSSID[pEntry->apidx];
	PhyMode = wdev->PhyMode;

	FlgIs11bSta = 1;
	for(i=0; i<ie_list->SupportedRatesLen; i++)
	{
		if (((ie_list->SupportedRates[i] & 0x7F) != 2) &&
			((ie_list->SupportedRates[i] & 0x7F) != 4) &&
			((ie_list->SupportedRates[i] & 0x7F) != 11) &&
			((ie_list->SupportedRates[i] & 0x7F) != 22))
		{
			FlgIs11bSta = 0;
			break;
		}
	}
    
    
	/* clear the previous Pairwise key table */
    if(pEntry->Aid != 0 &&
	(pEntry->WepStatus >= Ndis802_11Encryption2Enabled 
#ifdef DOT1X_SUPPORT
	|| wdev->IEEE8021X
#endif /* DOT1X_SUPPORT */
	))
    {
		/* clear GTK state */
		pEntry->GTKState = REKEY_NEGOTIATING;
    	
		NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));

		/* clear this entry as no-security mode */
		AsicRemovePairwiseKeyEntry(pAd, pEntry->Aid);

#ifdef DOT1X_SUPPORT
		/* Notify 802.1x daemon to clear this sta info */
		if (pEntry->AuthMode == Ndis802_11AuthModeWPA || 
			pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
			wdev->IEEE8021X)
		DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);
#endif /* DOT1X_SUPPORT */

#ifdef WAPI_SUPPORT
		WAPI_InternalCmdAction(pAd, 
							   pEntry->AuthMode, 
							   pEntry->apidx, 
							   pEntry->Addr, 
							   WAI_MLME_DISCONNECT);

		RTMPCancelWapiRekeyTimerAction(pAd, pEntry);
#endif /* WAPI_SUPPORT */
    }

#ifdef WSC_AP_SUPPORT
    /* since sta has been left, ap should receive EapolStart and EapRspId again. */
	pEntry->Receive_EapolStart_EapRspId = 0;
	pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef WSC_V2_SUPPORT
	if ((wdev->WscControl.WscV2Info.bEnableWpsV2) &&
		(wdev->WscControl.WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
	    if (pEntry->apidx < pAd->ApCfg.BssidNum)
	    {
	        if (MAC_ADDR_EQUAL(pEntry->Addr, wdev->WscControl.EntryAddr))
	        {
	            BOOLEAN Cancelled;
	            RTMPZeroMemory(wdev->WscControl.EntryAddr, MAC_ADDR_LEN);
	            RTMPCancelTimer(&wdev->WscControl.EapolTimer, &Cancelled);
	            wdev->WscControl.EapolTimerRunning = FALSE;
	        }
	    }
			
	    if ((ie_list->RSNIE_Len == 0) &&
		 (wdev->AuthMode >= Ndis802_11AuthModeWPA) &&
		 (wdev->WscControl.WscConfMode != WSC_DISABLE))
	        pEntry->bWscCapable = TRUE;
	}
#endif /* WSC_AP_SUPPORT */

    /* for hidden SSID sake, SSID in AssociateRequest should be fully verified */
	if ((ie_list->SsidLen != wdev->SsidLen) ||
		(NdisEqualMemory(ie_list->Ssid, wdev->Ssid, ie_list->SsidLen)==0))
        goto LabelOK;
        
#ifdef WSC_V2_SUPPORT
	/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
	if ((pEntry->bWscCapable) &&
		(wdev->WscControl.WscConfMode != WSC_DISABLE) &&
		(wdev->WscControl.WscV2Info.bEnableWpsV2) &&
		(wdev->WscControl.WscV2Info.bWpsEnable) &&
		(MAC_ADDR_EQUAL(wdev->WscControl.EntryAddr, ZERO_MAC_ADDR)))
		;
	else
#endif /* WSC_V2_SUPPORT */        
	/* set a flag for sending Assoc-Fail response to unwanted STA later. */
	if (! ApCheckAccessControlList(pAd, ie_list->Addr2, pEntry->apidx))
		bACLReject = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s - MBSS(%d), receive %s request from %02x:%02x:%02x:%02x:%02x:%02x\n",
							  sAssoc, pEntry->apidx, sAssoc, PRINT_MAC(ie_list->Addr2)));
    
	/* supported rates array may not be sorted. sort it and find the maximum rate */
	for (i=0; i<ie_list->SupportedRatesLen; i++)
	{
		if (MaxSupportedRate < (ie_list->SupportedRates[i] & 0x7f))
			MaxSupportedRate = ie_list->SupportedRates[i] & 0x7f;
	}

	/*
		Assign RateLen here or we will select wrong rate table in
		APBuildAssociation() when 11N compile option is disabled.
	*/
	pEntry->RateLen = ie_list->SupportedRatesLen;

#ifdef RT_BIG_ENDIAN
	NdisCopyMemory(&tmp_1,&ie_list->vht_cap.vht_cap, 4);
	tmp_1=SWAP32(tmp_1);
	NdisCopyMemory(&ie_list->vht_cap.vht_cap,&tmp_1, 4);
	
	NdisCopyMemory(&tmp_2,&(ie_list->vht_cap.mcs_set), 8);
	tmp_2=SWAP64(tmp_2);
	NdisCopyMemory(&(ie_list->vht_cap.mcs_set),&tmp_2, 8);
	//SWAP32((UINT32)vht_cap_ie.vht_cap);
	//SWAP32((UINT32)vht_cap_ie.mcs_set);
#endif /* RT_BIG_ENDIAN */

	RTMPSetSupportMCS(pAd,
					OPMODE_AP,
					pEntry,
					ie_list->SupportedRates,
					ie_list->SupportedRatesLen,
					NULL,
					0,
#ifdef DOT11_VHT_AC
					ie_list->vht_cap_len,
					&ie_list->vht_cap,
#endif /* DOT11_VHT_AC */
					&ie_list->HTCapability,
					ie_list->ht_cap_len);

	/* 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode */
	StatusCode = APBuildAssociation(pAd, pEntry, ie_list, MaxSupportedRate, &Aid);



#ifdef DOT11_VHT_AC
	if (ie_list->vht_cap_len)
	{
//+++Add by shiang for debug
		if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode)) {
			DBGPRINT(RT_DEBUG_TRACE, ("%s():Peer is VHT capable device!\n", __FUNCTION__));
			//dump_vht_cap(pAd, &ie_list->vht_cap);
		}
//---Add by shiang for debug
	}
#endif /* DOT11_VHT_AC */

	if (StatusCode == MLME_ASSOC_REJ_DATA_RATE)
			RTMPSendWirelessEvent(pAd, IW_STA_MODE_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

    /* 3. send Association Response */
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS)
        goto LabelOK;
        
    DBGPRINT(RT_DEBUG_TRACE, ("%s - Send %s response (Status=%d)...\n", sAssoc, sAssoc, StatusCode));
    Aid |= 0xc000; /* 2 most significant bits should be ON */

	SubType = isReassoc ? SUBTYPE_REASSOC_RSP : SUBTYPE_ASSOC_RSP;

	CapabilityInfoForAssocResp = wdev->CapabilityInfo; /*use AP's cability */
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT
	if ((wdev->WscControl.WscV2Info.bEnableWpsV2) &&
		(wdev->WscControl.WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
			(ie_list->CapabilityInfo & 0x0010))
		{
			CapabilityInfoForAssocResp |= 0x0010;
		}
	}
#endif /* WSC_AP_SUPPORT */
	
	/* fail in ACL checking => send an Assoc-Fail resp. */
	SupRateLen = pAd->CommonCfg.SupRateLen;

	/* TODO: need to check rate in support rate element, not number */
	if (FlgIs11bSta == 1)
		SupRateLen = 4;

	if (bACLReject == TRUE)
	{
	    MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2, 
							wdev->Bssid);
		StatusCode = MLME_UNSPECIFY_FAIL;
	    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
	                      sizeof(HEADER_802_11),    &AssocRspHdr,
	                      2,                        &CapabilityInfoForAssocResp,
	                      2,                        &StatusCode,
	                      2,                        &Aid,
	                      1,                        &SupRateIe,
	                      1,                        &SupRateLen,
	                      SupRateLen,               pAd->CommonCfg.SupRate,
	                      END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, (PVOID) pOutBuffer);

		RTMPSendWirelessEvent(pAd, IW_MAC_FILTER_LIST_EVENT_FLAG, ie_list->Addr2, pEntry->apidx, 0);

#ifdef WSC_V2_SUPPORT
		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
#endif /* WSC_V2_SUPPORT */

		goto LabelOK;
	}

#ifdef RT_CFG80211_SUPPORT
	CFG80211OS_NewSta(wdev->MSSIDDev, ie_list->Addr2, (PUCHAR)Elem->Msg, Elem->MsgLen);
#endif /* RT_CFG80211_SUPPORT */

	MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2, 
						wdev->Bssid);

    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                      sizeof(HEADER_802_11),    &AssocRspHdr,
                      2,                        &CapabilityInfoForAssocResp,
                      2,                        &StatusCode,
                      2,                        &Aid,
                      1,                        &SupRateIe,
                      1,                        &SupRateLen,
                      SupRateLen,               pAd->CommonCfg.SupRate,
                      END_OF_ARGS);

    if ((pAd->CommonCfg.ExtRateLen) && (PhyMode != WMODE_B) && (FlgIs11bSta == 0))
    {
        ULONG TmpLen;
        MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
                          1,                        &ExtRateIe,
                          1,                        &pAd->CommonCfg.ExtRateLen,
                          pAd->CommonCfg.ExtRateLen,    pAd->CommonCfg.ExtRate,
                          END_OF_ARGS);
        FrameLen += TmpLen;
    }



    /* add WMM IE here */
	if (wdev->bWmmCapable && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
	{
		ULONG TmpLen;
		UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};

		WmeParmIe[8] = pAd->ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;
#ifdef UAPSD_SUPPORT
		UAPSD_MR_IE_FILL(WmeParmIe[8], &wdev->UapsdInfo);
#endif /* UAPSD_SUPPORT */
		for (i=QID_AC_BE; i<=QID_AC_VO; i++)
		{
			WmeParmIe[10+ (i*4)] = (i << 5) +     /* b5-6 is ACI */
								((UCHAR)pAd->ApCfg.BssEdcaParm.bACM[i] << 4) + /* b4 is ACM */
								(pAd->ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);              /* b0-3 is AIFSN */
			WmeParmIe[11+ (i*4)] = (pAd->ApCfg.BssEdcaParm.Cwmax[i] << 4) + /* b5-8 is CWMAX */
								(pAd->ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              /* b0-3 is CWMIN */
			WmeParmIe[12+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] & 0xff);        /* low byte of TXOP */
			WmeParmIe[13+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] >> 8);          /* high byte of TXOP */
		}

		MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							26,                       WmeParmIe,
							END_OF_ARGS);
		FrameLen += TmpLen;
	}


#ifdef DOT11_N_SUPPORT
	/* HT capability in AssocRsp frame. */
	if ((ie_list->ht_cap_len > 0) && WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		ULONG TmpLen;
		UCHAR HtLen1 = sizeof(pAd->CommonCfg.AddHTInfo);
		HT_CAPABILITY_IE HtCapabilityRsp;
#ifdef RT_BIG_ENDIAN
		HT_CAPABILITY_IE HtCapabilityTmp;
		ADD_HT_INFO_IE	addHTInfoTmp;
#endif

		NdisMoveMemory(&HtCapabilityRsp, &pAd->CommonCfg.HtCapability, ie_list->ht_cap_len);

		/* add HT Capability IE */
#ifndef RT_BIG_ENDIAN
		MakeOutgoingFrame(pOutBuffer+FrameLen,			&TmpLen,
											1,			&HtCapIe,
											1,			&ie_list->ht_cap_len,
							ie_list->ht_cap_len,		&HtCapabilityRsp,
											1,			&AddHtInfoIe,
											1,			&HtLen1,
										HtLen1,			&pAd->CommonCfg.AddHTInfo,
						  END_OF_ARGS);
#else
		NdisMoveMemory(&HtCapabilityTmp, &HtCapabilityRsp, ie_list->ht_cap_len);
		*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
		{
			EXT_HT_CAP_INFO extHtCapInfo;

			NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
			*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
			NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
		}
#else
		*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

		NdisMoveMemory(&addHTInfoTmp, &pAd->CommonCfg.AddHTInfo, HtLen1);
		*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
		*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

		MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
							1,                           &HtCapIe,
							1,                           &ie_list->ht_cap_len,
							ie_list->ht_cap_len,            &HtCapabilityTmp,
							1,                           &AddHtInfoIe,
							1,                           &HtLen1,
							HtLen1,                      &addHTInfoTmp,
							END_OF_ARGS);
#endif
		FrameLen += TmpLen;

		if ((ie_list->RalinkIe) == 0 || (pAd->bBroadComHT == TRUE))
		{
			UCHAR epigram_ie_len;
			UCHAR BROADCOM_HTC[4] = {0x0, 0x90, 0x4c, 0x33};
			UCHAR BROADCOM_AHTINFO[4] = {0x0, 0x90, 0x4c, 0x34};


			epigram_ie_len = ie_list->ht_cap_len + 4;
#ifndef RT_BIG_ENDIAN
			MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
						  1,                                &WpaIe,
						  1,                                &epigram_ie_len,
						  4,                                &BROADCOM_HTC[0],
						  ie_list->ht_cap_len,            		&HtCapabilityRsp,
						  END_OF_ARGS);
#else
			MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
						  1,                                &WpaIe,
						  1,                                &epigram_ie_len,
						  4,                                &BROADCOM_HTC[0],
						  ie_list->ht_cap_len,            		&HtCapabilityTmp,
						  END_OF_ARGS);
#endif

			FrameLen += TmpLen;
			epigram_ie_len = HtLen1 + 4;
#ifndef RT_BIG_ENDIAN
			MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
						  1,                                &WpaIe,
						  1,                                &epigram_ie_len,
						  4,                                &BROADCOM_AHTINFO[0],
						  HtLen1, 							&pAd->CommonCfg.AddHTInfo,
						  END_OF_ARGS);
#else
			MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
						  1,                                &WpaIe,
						  1,                                &epigram_ie_len,
						  4,                                &BROADCOM_AHTINFO[0],
						  HtLen1, 							&addHTInfoTmp,
						  END_OF_ARGS);
#endif
			FrameLen += TmpLen;
		}

#ifdef DOT11N_DRAFT3
	 	/* P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE */
	 	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) &&
			(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == 1))
	 	{
			OVERLAP_BSS_SCAN_IE OverlapScanParam;
			ULONG TmpLen;
			UCHAR OverlapScanIE, ScanIELen;

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

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
			(pAd->CommonCfg.Channel > 14) &&
			(ie_list->vht_cap_len))
		{
			FrameLen += build_vht_ies(pAd, pOutBuffer + FrameLen, SUBTYPE_ASSOC_RSP, pAd->CommonCfg.vht_max_mcs_cap);
		}
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */


		/* 7.3.2.27 Extended Capabilities IE */
		{
		ULONG TmpLen, infoPos;
		PUCHAR pInfo;
		UCHAR extInfoLen;
		BOOLEAN bNeedAppendExtIE = FALSE;
		EXT_CAP_INFO_ELEMENT extCapInfo;

		
		extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
		NdisZeroMemory(&extCapInfo, extInfoLen);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		/* P802.11n_D1.10, HT Information Exchange Support */
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode)
			&& (pAd->CommonCfg.Channel <= 14) 
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		)
		{
			extCapInfo.BssCoexistMgmtSupport = 1;
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


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
			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							1,			&ExtCapIe,
							1,			&extInfoLen,
							extInfoLen,	&extCapInfo,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
	
	/* add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
{
	ULONG TmpLen;
	UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};

	if (pAd->CommonCfg.bAggregationCapable)
		RalinkSpecificIe[5] |= 0x1;
	if (pAd->CommonCfg.bPiggyBackCapable)
		RalinkSpecificIe[5] |= 0x2;
#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.bRdg)
		RalinkSpecificIe[5] |= 0x4;
#endif /* DOT11_N_SUPPORT */
	MakeOutgoingFrame(pOutBuffer+FrameLen,		 &TmpLen,
						9,						 RalinkSpecificIe,
						END_OF_ARGS);
	FrameLen += TmpLen;

}
  
#ifdef WSC_AP_SUPPORT
	if (pEntry->bWscCapable)
	{
		UCHAR *pWscBuf = NULL, WscIeLen = 0;
		ULONG WscTmpLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);
		if(pWscBuf)
		{
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocRespIE(pAd, pEntry->apidx, 0, pWscBuf, &WscIeLen);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &WscTmpLen,
								  WscIeLen, pWscBuf,
								  END_OF_ARGS);

			FrameLen += WscTmpLen;
			os_free_mem(NULL, pWscBuf);
		}
	}
#endif /* WSC_AP_SUPPORT */
  
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, (PVOID) pOutBuffer);


	/* set up BA session */
	if (StatusCode == MLME_SUCCESS)
	{
		pEntry->PsMode = PWR_ACTIVE;

#ifdef IAPP_SUPPORT
		/*PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg; */
/*		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie; */

		/* send association ok message to IAPPD */

		IAPP_L2_Update_Frame_Send(pAd, pEntry->Addr, pEntry->apidx);
		DBGPRINT(RT_DEBUG_TRACE, ("####### Send L2 Frame Mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
								  PRINT_MAC(pEntry->Addr)));

/*		SendSingalToDaemon(SIGUSR2, pObj->IappPid); */

		
#endif /* IAPP_SUPPORT */

		ap_assoc_info_debugshow(pAd, isReassoc, pEntry, ie_list);

		/* send wireless event - for association */
		RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, 0, 0);
    	
		/* This is a reassociation procedure */
		pEntry->IsReassocSta = isReassoc;
		
#ifdef DOT11_N_SUPPORT
		/* clear txBA bitmap */
		pEntry->TXBAbitmap = 0;
		if (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)
		{
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
			if ((pAd->CommonCfg.Channel <=14) &&
				pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset &&
				(ie_list->HTCapability.HtCapInfo.ChannelWidth==BW_40))
			{
				SendBeaconRequest(pAd, pEntry->Aid);
			}
			/*BAOriSessionSetUp(pAd, pEntry, 0, 0, 3000, FALSE); */
		}
#endif /* DOT11_N_SUPPORT */


		/* enqueue a EAPOL_START message to trigger EAP state machine doing the authentication */
	    if ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || 
			(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
    	{
#ifdef WSC_AP_SUPPORT
            /*
                In WPA-PSK mode,
                If Association Request of station has RSN/SSN, WPS AP Must Not send EAP-Request/Identity to station
                no matter WPS AP does receive EAPoL-Start from STA or not.
                Marvell WPS test bed(v2.1.1.5) will send AssocReq with WPS IE and RSN/SSN IE.
            */
            if (pEntry->bWscCapable || (ie_list->RSNIE_Len == 0))
            {
			DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - IF(ra%d) This is a WPS Client.\n\n", pEntry->apidx));
			goto LabelOK;
            }
            else
            {
                pEntry->bWscCapable = FALSE;
                pEntry->Receive_EapolStart_EapRspId = (WSC_ENTRY_GET_EAPOL_START | WSC_ENTRY_GET_EAP_RSP_ID);
                /* This STA is not a WPS STA */
                NdisZeroMemory(wdev->WscControl.EntryAddr, 6);
            }
#endif /* WSC_AP_SUPPORT */

			/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
			if ((pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
#ifdef HOSTAPD_SUPPORT			
				&& wdev->Hostapd == FALSE
#endif/*HOSTAPD_SUPPORT*/
				)
#ifdef WSC_AP_SUPPORT
				&& !pEntry->bWscCapable
#endif /* WSC_AP_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
				&& (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
				)
			{
        		/* Enqueue a EAPOL-start message with the pEntry */
        		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;
        		RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER);
			}
    	}
#ifdef DOT1X_SUPPORT
		/*else if (isReassoc && */
		else if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && 
				((pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE, ie_list->RSNIE_Len, PMKID_LIST, &pmkid_count)) != NULL))
		{	/* Key cache */
			INT	CacheIdx;

			if (((CacheIdx = RTMPSearchPMKIDCache(pAd, pEntry->apidx, pEntry->Addr)) != -1)
				&& (RTMPEqualMemory(pPmkid, &wdev->PMKIDCache.BSSIDInfo[CacheIdx].PMKID, LEN_PMKID)))
			{
				/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
				if ((pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
#ifdef HOSTAPD_SUPPORT
					&& wdev->Hostapd == FALSE
#endif /*HOSTAPD_SUPPORT*/
				)
#ifdef WSC_AP_SUPPORT
					&& !pEntry->bWscCapable
#endif /* WSC_AP_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
					&& (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
				)
				{
					/* Enqueue a EAPOL-start message with the pEntry */
					pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;
					RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER);
				}

				pEntry->PMKID_CacheIdx = CacheIdx;
				DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - 2.PMKID matched and start key cache algorithm\n"));
			}
			else
			{
				pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
				DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - 2.PMKID not found \n"));
			}
		}
		else if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) ||
				 (pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
				 ((wdev->IEEE8021X)
#ifdef WSC_AP_SUPPORT
					&& (!pEntry->bWscCapable)
#endif /* WSC_AP_SUPPORT */
				)
		)
		{
			/* Enqueue a EAPOL-start message to trigger EAP SM */
			if (pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
#ifdef HOSTAPD_SUPPORT
				&& wdev->Hostapd == FALSE
#endif/*HOSTAPD_SUPPORT*/
#ifdef RT_CFG80211_SUPPORT
				&& (pAd->VifNextMode == RT_CMD_80211_IFTYPE_NOT_USED)
#endif /* RT_CFG80211_SUPPORT */
			)
			{
      	  			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
       	 		RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER);
			}
		}
#endif /* DOT1X_SUPPORT */
#ifdef WAPI_SUPPORT
		else if (pEntry->AuthMode == Ndis802_11AuthModeWAICERT)
		{
			WAPI_InternalCmdAction(pAd, 
								   pEntry->AuthMode, 
								   pEntry->apidx, 
								   pEntry->Addr, 
								   WAI_MLME_CERT_AUTH_START);	

			RTMPInitWapiRekeyTimerAction(pAd, pEntry);
		}
		else if (pEntry->AuthMode == Ndis802_11AuthModeWAIPSK)
		{
			WAPI_InternalCmdAction(pAd, 
								   pEntry->AuthMode, 
								   pEntry->apidx, 
								   pEntry->Addr, 
								   WAI_MLME_KEY_HS_START);
			RTMPInitWapiRekeyTimerAction(pAd, pEntry);
		}
#endif /* WAPI_SUPPORT */
        else
        {
			if (pEntry->WepStatus == Ndis802_11WEPEnabled)
			{
				/* Set WEP key to ASIC */
				UCHAR KeyIdx = 0;
				UCHAR CipherAlg = 0;

				KeyIdx = wdev->DefaultKeyId;					
				CipherAlg = pAd->SharedKey[pEntry->apidx][KeyIdx].CipherAlg;

				/*
					If WEP is used, set pair-wise cipherAlg into WCID
					attribute table for this entry.
				*/
				RTMP_SET_WCID_SEC_INFO(pAd, 
										pEntry->apidx, 
										KeyIdx, 
										CipherAlg, 
										pEntry->Aid, 
										SHAREDKEYTABLE);
			}	
        }

	}

LabelOK:
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);

	return;	
}



/*
    ==========================================================================
    Description:
        peer assoc req handling procedure
    Parameters:
        Adapter - Adapter pointer
        Elem - MLME Queue Element
    Pre:
        the station has been authenticated and the following information is stored
    Post  :
        -# An association response frame is generated and sent to the air
    ==========================================================================
 */
VOID APPeerAssocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 0);
}

/*
    ==========================================================================
    Description:
        mlme reassoc req handling procedure
    Parameters:
        Elem -
    Pre:
        -# SSID  (Adapter->ApCfg.ssid[])
        -# BSSID (AP address, Adapter->ApCfg.bssid)
        -# Supported rates (Adapter->ApCfg.supported_rates[])
        -# Supported rates length (Adapter->ApCfg.supported_rates_len)
        -# Tx power (Adapter->ApCfg.tx_power)
    ==========================================================================
 */
VOID APPeerReassocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 1);
}

/*
    ==========================================================================
    Description:
        left part of IEEE 802.11/1999 p.374
    Parameters:
        Elem - MLME message containing the received frame
    ==========================================================================
 */
VOID APPeerDisassocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Reason;
	UINT16 SeqNum;		
	MAC_TABLE_ENTRY *pEntry;
	MULTISSID_STRUCT *wdev;

	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - 1 receive DIS-ASSOC request \n"));
    if (! PeerDisassocReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &SeqNum, &Reason))
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - receive DIS-ASSOC(seq-%d) request from %02x:%02x:%02x:%02x:%02x:%02x, reason=%d\n", 
								SeqNum, Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5],Reason));
    
	pEntry = MacTableLookup(pAd, Addr2);

	if (pEntry == NULL)
		return;
		
	if (Elem->Wcid < MAX_LEN_OF_MAC_TABLE)
    {

#ifdef DOT1X_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[pEntry->apidx];
		/* Notify 802.1x daemon to clear this sta info */
		if (pEntry->AuthMode == Ndis802_11AuthModeWPA || 
			pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
			wdev->IEEE8021X)
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);
#endif /* DOT1X_SUPPORT */
	
#ifdef WAPI_SUPPORT
		WAPI_InternalCmdAction(pAd, 
							   pEntry->AuthMode, 
							   pEntry->apidx, 
							   pEntry->Addr, 
							   WAI_MLME_DISCONNECT);		
#endif /* WAPI_SUPPORT */
	
		/* send wireless event - for disassociation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, Addr2, 0, 0);
        ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);

		MacTableDeleteEntry(pAd, Elem->Wcid, Addr2);

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn == TRUE)
		{
			UCHAR apCliIdx, CliIdx;
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, Addr2);
			if (pReptEntry && (pReptEntry->CliConnectState != 0))
			{
				apCliIdx = pReptEntry->MatchApCliIdx;
				CliIdx = pReptEntry->MatchLinkIdx;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL,
								(64 + MAX_EXT_MAC_ADDR_SIZE*apCliIdx + CliIdx));
				RTMP_MLME_HANDLER(pAd);
				RTMPRemoveRepeaterEntry(pAd, apCliIdx, CliIdx);
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
    }
}

/*
    ==========================================================================
    Description:
        delete it from STA and disassoc s STA
    Parameters:
        Elem -
    ==========================================================================
 */
VOID MbssKickOutStas(
    IN PRTMP_ADAPTER pAd,
    IN INT apidx,
    IN USHORT Reason)
{
	INT i;
	PMAC_TABLE_ENTRY pEntry;

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->apidx == apidx)
			APMlmeKickOutSta(pAd, pEntry->Addr, pEntry->Aid, Reason);
	}

	return;
}

/*
    ==========================================================================
    Description:
        delete it from STA and disassoc s STA
    Parameters:
        Elem -
    ==========================================================================
 */
VOID APMlmeKickOutSta(
    IN PRTMP_ADAPTER pAd,
	IN PUCHAR pStaAddr,
	IN UCHAR Wcid,
	IN USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR Aid;
	UCHAR ApIdx;

	pEntry = MacTableLookup(pAd, pStaAddr);

	if (pEntry == NULL)
	{
		return;
	}
	Aid = pEntry->Aid;
	ApIdx = pEntry->apidx;

	ASSERT(Aid == Wcid);

	if (ApIdx >= pAd->ApCfg.BssidNum)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid Apidx=%d\n",
			__FUNCTION__, ApIdx));
		return;
	}

	if (Aid < MAX_LEN_OF_MAC_TABLE)
	{
		/* send wireless event - for disassocation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, pStaAddr, 0, 0);
        ApLogEvent(pAd, pStaAddr, EVENT_DISASSOCIATED);

	    /* 2. send out a DISASSOC request frame */
	    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	    if (NStatus != NDIS_STATUS_SUCCESS)
	        return;
	    
	    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MLME disassociates %02x:%02x:%02x:%02x:%02x:%02x; Send DISASSOC request\n",
	        pStaAddr[0],pStaAddr[1],pStaAddr[2], pStaAddr[3],pStaAddr[4],pStaAddr[5]));
	    MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pStaAddr, 
							pAd->ApCfg.MBSSID[ApIdx].Bssid);
	    MakeOutgoingFrame(pOutBuffer,            &FrameLen,
	                      sizeof(HEADER_802_11), &DisassocHdr,
	                      2,                     &Reason,
	                      END_OF_ARGS);
	    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	    MlmeFreeMemory(pAd, pOutBuffer);
		MacTableDeleteEntry(pAd, Aid, pStaAddr);
    }
}




/*
    ==========================================================================
    Description:
        Upper layer orders to disassoc s STA
    Parameters:
        Elem -
    ==========================================================================
 */
VOID APMlmeDisassocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    MLME_DISASSOC_REQ_STRUCT *DisassocReq = (MLME_DISASSOC_REQ_STRUCT *)(Elem->Msg);

	APMlmeKickOutSta(pAd, DisassocReq->Addr, Elem->Wcid, DisassocReq->Reason);
}


/*
    ==========================================================================
    Description:
        right part of IEEE 802.11/1999 page 374
    Note:
        This event should never cause ASSOC state machine perform state
        transition, and has no relationship with CNTL machine. So we separate
        this routine as a service outside of ASSOC state transition table.
    ==========================================================================
 */
VOID APCls3errAction(
    IN PRTMP_ADAPTER pAd,
	IN 	ULONG Wcid,
    IN	PHEADER_802_11	pHeader)
{
    HEADER_802_11         DisassocHdr;
    PUCHAR                pOutBuffer = NULL;
    ULONG                 FrameLen = 0;
    NDIS_STATUS           NStatus;
    USHORT                Reason = REASON_CLS3ERR;
    MAC_TABLE_ENTRY       *pEntry = NULL;

	if (Wcid < MAX_LEN_OF_MAC_TABLE)
	{
      pEntry = &(pAd->MacTab.Content[Wcid]);
	}

    if (pEntry)
    {
        /*ApLogEvent(pAd, pAddr, EVENT_DISASSOCIATED); */
		MacTableDeleteEntry(pAd, pEntry->Aid, pHeader->Addr2);
    }
    
    /* 2. send out a DISASSOC request frame */
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Class 3 Error, Send DISASSOC frame to %02x:%02x:%02x:%02x:%02x:%02x\n",
    		PRINT_MAC(pHeader->Addr2)));
    MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pHeader->Addr2, 
						pHeader->Addr1);
    MakeOutgoingFrame(pOutBuffer,            &FrameLen,
                      sizeof(HEADER_802_11), &DisassocHdr,
                      2,                     &Reason,
                      END_OF_ARGS);
    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
    MlmeFreeMemory(pAd, pOutBuffer);
}


/*
    ==========================================================================
    Description:
        association state machine init, including state transition and timer init
    Parameters:
        S - pointer to the association state machine
    Note:
        The state machine looks like the following
        
                                    AP_ASSOC_IDLE
        APMT2_MLME_DISASSOC_REQ    mlme_disassoc_req_action
        APMT2_PEER_DISASSOC_REQ    peer_disassoc_action
        APMT2_PEER_ASSOC_REQ       drop
        APMT2_PEER_REASSOC_REQ     drop
        APMT2_CLS3ERR              cls3err_action
    ==========================================================================
 */
VOID APAssocStateMachineInit(
    IN  PRTMP_ADAPTER   pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[])
{
    StateMachineInit(S, (STATE_MACHINE_FUNC*)Trans, AP_MAX_ASSOC_STATE, AP_MAX_ASSOC_MSG, (STATE_MACHINE_FUNC)Drop, AP_ASSOC_IDLE, AP_ASSOC_MACHINE_BASE);

    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)APMlmeDisassocReqAction);
    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)APPeerDisassocReqAction);
    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_ASSOC_REQ,    (STATE_MACHINE_FUNC)APPeerAssocReqAction);
    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_REASSOC_REQ,  (STATE_MACHINE_FUNC)APPeerReassocReqAction);
/*  StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_CLS3ERR,           APCls3errAction); */
}

