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


#ifdef DOT11_N_SUPPORT

INT ht_support_bw_by_channel_boundary(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, IE_LISTS *peer_ie_list)
{
	struct wifi_dev *wdev = pEntry->wdev;
	UCHAR supported_bw;
	UCHAR ext_cha;

	if(!wdev)
		return HT_BW_20;

	supported_bw = wlan_operate_get_ht_bw(wdev);
	ext_cha = wlan_operate_get_ext_cha(wdev);
	
	if (!peer_ie_list)
		return supported_bw;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(), Channel = %d, supported_bw = %d, extcha = %d, PeerSupChlLen = %d\n", __FUNCTION__, 
		wdev->channel, 
		supported_bw,  
		ext_cha, 
		peer_ie_list->SupportedChlLen));

	/* So far, only check 2.4G band */
	if ((wdev->channel <= 14) && (peer_ie_list->SupportedChlLen >= 2))
	{
		UCHAR i;
		UCHAR peer_sup_chl_min_2g = 1;
		UCHAR peer_sup_chl_max_2g = 14;
		
		/* Get Min/Max 2.4G supported channels */
		for (i=0; i < peer_ie_list->SupportedChlLen; i+=2)
		{
			if (peer_ie_list->SupportedChl[i] <= 14)
			{
				peer_sup_chl_min_2g = peer_ie_list->SupportedChl[i];
				peer_sup_chl_max_2g = peer_ie_list->SupportedChl[i] + peer_ie_list->SupportedChl[i+1] - 1;

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,("  peer_sup_chl_min_2g = %d, peer_sup_chl_max_2g = %d\n", 
					peer_sup_chl_min_2g, peer_sup_chl_max_2g));
			}
		}

		/* Check Min/Max 2.4G channel boundary in BW40 case */
		if ((supported_bw == BW_40) && (ext_cha != EXTCHA_NONE))
		{
			if (((ext_cha == EXTCHA_ABOVE) && ((wdev->channel + 4) > peer_sup_chl_max_2g)) ||
				((ext_cha == EXTCHA_BELOW) && ((wdev->channel - 4) < peer_sup_chl_min_2g)))
				supported_bw = BW_20;
		}
	}

	return supported_bw;
}

INT ht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, IE_LISTS *peer_ie_list, HT_CAPABILITY_IE *peer, RT_HT_CAPABILITY *my)
{
	UCHAR supported_bw = ht_support_bw_by_channel_boundary(pAd, pEntry, peer_ie_list);
	struct wifi_dev *wdev = pEntry->wdev;
	ADD_HT_INFO_IE *addht;
	UCHAR band = 0;

	if(!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	band = HcGetBandByWdev(wdev);

	if ((peer->HtCapInfo.GF) && (my->GF))
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
	}
	else
	{	
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		addht->AddHtInfo2.NonGfPresent = 1;
		pAd->MacTab.fAnyStationNonGF[band] = TRUE;
	}

	if ((peer->HtCapInfo.ChannelWidth) && (wlan_config_get_ht_bw(wdev)) && (wlan_operate_get_ht_bw(wdev)) && (supported_bw == BW_40))
	{
		pEntry->MaxHTPhyMode.field.BW= BW_40;
		pEntry->MaxHTPhyMode.field.ShortGI = ((my->ShortGIfor40) & (peer->HtCapInfo.ShortGIfor40));
	}
	else
	{	
		pEntry->MaxHTPhyMode.field.BW = BW_20;
		pEntry->MaxHTPhyMode.field.ShortGI = ((my->ShortGIfor20) & (peer->HtCapInfo.ShortGIfor20));
		pAd->MacTab.fAnyStation20Only = TRUE;
	}
		
	pEntry->MaxHTPhyMode.field.STBC = (peer->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(), MODE = %d, BW = %d, SGI = %d, STBC = %d\n", __FUNCTION__, 
		pEntry->MaxHTPhyMode.field.MODE, 
		pEntry->MaxHTPhyMode.field.BW, 
		pEntry->MaxHTPhyMode.field.ShortGI, 
		pEntry->MaxHTPhyMode.field.STBC));
		
	return TRUE;
}


INT set_ht_fixed_mcs(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs, UCHAR mcs_bound)
{
	if (fixed_mcs == 32)
	{
		/* Fix MCS as HT Duplicated Mode */
		pEntry->MaxHTPhyMode.field.BW = 1;
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pEntry->MaxHTPhyMode.field.STBC = 0;
		pEntry->MaxHTPhyMode.field.ShortGI = 0;
		pEntry->MaxHTPhyMode.field.MCS = 32;
	}
	else if (pEntry->MaxHTPhyMode.field.MCS > mcs_bound)
	{
		/* STA supports fixed MCS */
		pEntry->MaxHTPhyMode.field.MCS = mcs_bound;
	}

	return TRUE;
}


INT get_ht_max_mcs(RTMP_ADAPTER *pAd, UCHAR *desire_mcs, UCHAR *cap_mcs)
{
	INT i, j;
	UCHAR bitmask;


	for (i=31; i>=0; i--)
	{	
		j = i/8;	
		bitmask = (1<<(i-(j*8)));
		if ((desire_mcs[j] & bitmask) && (cap_mcs[j] & bitmask))
		{
			/*pEntry->MaxHTPhyMode.field.MCS = i; */
			/* find it !!*/
			break;
		}
		if (i==0)
			break;
	}

	return i;
}


INT get_ht_cent_ch(RTMP_ADAPTER *pAd, UINT8 *rf_bw, UINT8 *ext_ch, UCHAR Channel)
{
	UCHAR op_ht_bw = HT_BW_20;
	UCHAR op_ext_cha = EXTCHA_NONE;
	UCHAR i;
	struct wifi_dev *wdev;

	for(i=0;i<WDEV_NUM_MAX;i++){
		wdev = pAd->wdev_list[i];
		if(!wdev)
			continue;
		/*check in the same band*/
		if((Channel == wdev->channel)
            &&(wlan_operate_get_state(wdev) != WLAN_OPER_STATE_INVALID)){
			op_ht_bw = wlan_operate_get_ht_bw(wdev);
			op_ext_cha = wlan_operate_get_ext_cha(wdev);
			if(op_ht_bw == HT_BW_40){
				break;
			}
		}
	}

	if ((op_ht_bw  == BW_40) && 
		(op_ext_cha== EXTCHA_ABOVE)
	)
	{
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_ABOVE;
		pAd->CommonCfg.CentralChannel = Channel + 2;
	}
	else if ((Channel > 2) && 
			(op_ht_bw == BW_40) && 
			(op_ext_cha== EXTCHA_BELOW))
	{
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_BELOW;
		if (Channel == 14)
			pAd->CommonCfg.CentralChannel = Channel - 1;
		else
			pAd->CommonCfg.CentralChannel = Channel- 2;
	} else {
		return FALSE;
	}

	return TRUE;
}


UCHAR get_cent_ch_by_htinfo(
	RTMP_ADAPTER *pAd,
	ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap)
{
	UCHAR cent_ch;
	
	if ((ht_op->ControlChan > 2)&&
		(ht_op->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) &&
		(ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan - 2;
	else if ((ht_op->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) &&
		(ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan + 2;
	else
		cent_ch = ht_op->ControlChan;
	
	return cent_ch;
}


VOID set_sta_ht_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, HT_CAPABILITY_IE *ht_ie)
{
	/* set HT capabilty flag for entry */
	CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HT_CAPABLE);

	if (pAd->CommonCfg.RegTransmitSetting.field.ShortGI == GI_400)
	{      
		if (ht_ie->HtCapInfo.ShortGIfor20)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI20_CAPABLE);

		if (ht_ie->HtCapInfo.ShortGIfor40)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI40_CAPABLE);
	}

	if (ht_ie->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_TxSTBC_CAPABLE);

	if (ht_ie->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_RxSTBC_CAPABLE);

	if (ht_ie->ExtHtCapInfo.PlusHTC) {
		CLIENT_CAP_SET_FLAG(entry, fCLIENT_STATUS_HTC_CAPABLE);

		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HTC_CAPABLE);
	}

	if (pAd->CommonCfg.bRdg && ht_ie->ExtHtCapInfo.RDGSupport)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_RDG_CAPABLE);

	if (ht_ie->ExtHtCapInfo.MCSFeedback == 0x03)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);

	
	if (wlan_config_get_ht_ldpc(entry->wdev) && (pAd->chipCap.phy_caps & fPHY_CAP_LDPC)) {
		if (ht_ie->HtCapInfo.ht_rx_ldpc)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE);
        }

	if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_AMSDU_INUSED);

	
	entry->MpduDensity = ht_ie->HtCapParm.MpduDensity;
	entry->MmpsMode = (UCHAR)ht_ie->HtCapInfo.MimoPs;
    entry->AMsduSize = (UCHAR)ht_ie->HtCapInfo.AMsduSize;
    entry->MaxRAmpduFactor = ht_ie->HtCapParm.MaxRAmpduFactor;
	
}


/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  - 

	========================================================================
*/
VOID RTMPSetHT(
	IN RTMP_ADAPTER *pAd,
	IN OID_SET_HT_PHYMODE *pHTPhyMode,
	IN struct wifi_dev *wdev)
{
	UCHAR RxStream = wlan_config_get_rx_stream(wdev);
	INT idx;
#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */

	INT bw;
	RT_HT_CAPABILITY *rt_ht_cap = &pAd->CommonCfg.DesiredHtPhy;
	HT_CAPABILITY_IE *ht_cap= &pAd->CommonCfg.HtCapability;
	
#ifdef CONFIG_AP_SUPPORT
	/* sanity check for extention channel */
	if (CHAN_PropertyCheck(pAd,pHTPhyMode->Channel ,
						CHANNEL_NO_FAT_BELOW | CHANNEL_NO_FAT_ABOVE) == TRUE)
	{
		/* only 20MHz is allowed */
		pHTPhyMode->BW = 0;
	}
	else if (pHTPhyMode->ExtOffset == EXTCHA_BELOW)
	{
		/* extension channel below this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
						CHANNEL_NO_FAT_BELOW) == TRUE)
		{
			pHTPhyMode->ExtOffset = EXTCHA_ABOVE;
		}
	}
	else if (pHTPhyMode->ExtOffset == EXTCHA_ABOVE)
	{
		/* extension channel above this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
						CHANNEL_NO_FAT_ABOVE) == TRUE)
		{
			pHTPhyMode->ExtOffset = EXTCHA_BELOW;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetHT : HT_mode(%d), ExtOffset(%d), MCS(%d), BW(%d), STBC(%d), SHORTGI(%d)\n",
										pHTPhyMode->HtMode, pHTPhyMode->ExtOffset, 
										pHTPhyMode->MCS, pHTPhyMode->BW,
										pHTPhyMode->STBC, pHTPhyMode->SHORTGI));
			
	/* Don't zero supportedHyPhy structure.*/
	RTMPZeroMemory(ht_cap, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(&pAd->CommonCfg.NewExtChanOffset, sizeof(pAd->CommonCfg.NewExtChanOffset));
	RTMPZeroMemory(rt_ht_cap, sizeof(RT_HT_CAPABILITY));

   	if (pAd->CommonCfg.bRdg)
	{
		ht_cap->ExtHtCapInfo.PlusHTC = 1;
		ht_cap->ExtHtCapInfo.RDGSupport = 1;
	}
	else
	{
		ht_cap->ExtHtCapInfo.PlusHTC = 0;
		ht_cap->ExtHtCapInfo.RDGSupport = 0;
	}

	ht_cap->HtCapParm.MaxRAmpduFactor = pAd->chipCap.AMPDUFactor;
	rt_ht_cap->MaxRAmpduFactor = pAd->chipCap.AMPDUFactor;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetHT : RxBAWinLimit = %d\n", pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	/* Mimo power save, A-MSDU size, */
	rt_ht_cap->AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	rt_ht_cap->AmsduSize = (UCHAR)pAd->CommonCfg.BACapability.field.AmsduSize;
	rt_ht_cap->MimoPs = (UCHAR)pAd->CommonCfg.BACapability.field.MMPSmode;
	rt_ht_cap->MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;

	ht_cap->HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	ht_cap->HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	
	if (wlan_config_get_ht_ldpc(wdev) && (pAd->chipCap.phy_caps & fPHY_CAP_LDPC))
		ht_cap->HtCapInfo.ht_rx_ldpc = 1;
	else
		ht_cap->HtCapInfo.ht_rx_ldpc = 0;
	
	ht_cap->HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetHT : AMsduSize = %d, MimoPs = %d, MpduDensity = %d, MaxRAmpduFactor = %d\n", 
													rt_ht_cap->AmsduSize, 
													rt_ht_cap->MimoPs,
													rt_ht_cap->MpduDensity,
													rt_ht_cap->MaxRAmpduFactor));
	
	if(pHTPhyMode->HtMode == HTMODE_GF)
	{
		ht_cap->HtCapInfo.GF = 1;
		rt_ht_cap->GF = 1;
	}
	else
		rt_ht_cap->GF = 0;
	
	/* Decide Rx MCSSet*/
	switch (RxStream)
	{
		case 4:
			ht_cap->MCSSet[3] =  0xff;
		case 3:
			ht_cap->MCSSet[2] =  0xff;
		case 2:
			ht_cap->MCSSet[1] =  0xff;
		case 1:
		default:
			ht_cap->MCSSet[0] =  0xff;
			break;
	}

	if (pAd->CommonCfg.bForty_Mhz_Intolerant && (pHTPhyMode->BW == BW_40))
	{
		pHTPhyMode->BW = BW_20;
		ht_cap->HtCapInfo.Forty_Mhz_Intolerant = 1;
	}

	// TODO: shiang-6590, how about the "bw" when channel 14 for JP region???
	//CFG_TODO
	bw = BW_20;
	if(pHTPhyMode->BW == BW_40)
	{
		ht_cap->MCSSet[4] = 0x1; /* MCS 32*/
		ht_cap->HtCapInfo.ChannelWidth = 1;
		if (pHTPhyMode->Channel <= 14) 		
			ht_cap->HtCapInfo.CCKmodein40 = 1;

		wlan_operate_set_ht_bw(wdev,HT_BW_40);		
		wlan_operate_set_ext_cha(wdev,(pHTPhyMode->ExtOffset == EXTCHA_BELOW)? (EXTCHA_BELOW): EXTCHA_ABOVE);
		/* Set Regsiter for extension channel position.*/

		AsicSetCtrlCh(pAd, pHTPhyMode->ExtOffset);

		/* Turn on BBP 40MHz mode now only as AP . */
		/* Sta can turn on BBP 40MHz after connection with 40MHz AP. Sta only broadcast 40MHz capability before connection.*/
		if ((pAd->OpMode == OPMODE_AP)|| ADHOC_ON(pAd)
		)
		{
			bbp_set_ctrlch(pAd, pHTPhyMode->ExtOffset);	
			bw = BW_40;
		}
	}
	else
	{
		ht_cap->HtCapInfo.ChannelWidth = 0;
		wlan_operate_set_ht_bw(wdev,HT_BW_20);
		wlan_operate_set_ext_cha(wdev,EXTCHA_NONE);
		pAd->CommonCfg.CentralChannel = pHTPhyMode->Channel;
		/* Turn on BBP 20MHz mode by request here.*/
		bw = BW_20;
	}

#ifdef DOT11_VHT_AC
	if (pHTPhyMode->BW == BW_40 &&
		pAd->CommonCfg.vht_bw == VHT_BW_80)
		bw = BW_80;
#endif /* DOT11_VHT_AC */

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	if ((pHTPhyMode->BW == BW_20) && (pHTPhyMode->Channel!= 0))
		HcBbpSetBwByChannel(pAd,pHTPhyMode->BW,pHTPhyMode->Channel);
	else if (INFRA_ON(pAd)){
		struct wifi_dev *p2p_dev = &pAd->StaCfg[0].wdev;
		UCHAR p2p_ht_bw = wlan_operate_get_ht_bw(p2p_dev);
		HcBbpSetBwByChannel(pAd,p2p_ht_bw,pAd->StaCfg[0].wdev.channel);
	}
	else
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
	{
		HcBbpSetBwByChannel(pAd, bw,pHTPhyMode->Channel);
	}

	if(pHTPhyMode->STBC == STBC_USE)
	{
	    UCHAR max_tx_path;

		if (pAd->CommonCfg.dbdc_mode)
		{
			UCHAR band_idx = HcGetBandByWdev(wdev);

			if (band_idx == DBDC_BAND0)
				max_tx_path = pAd->dbdc_2G_tx_stream;
			else
				max_tx_path = pAd->dbdc_5G_tx_stream;
		} else {
			max_tx_path = pAd->Antenna.field.TxPath;
		}

		if (max_tx_path >= 2)
		{
			ht_cap->HtCapInfo.TxSTBC = 1;
			rt_ht_cap->TxSTBC = 1;
		}
		else
		{
			ht_cap->HtCapInfo.TxSTBC = 0;
			rt_ht_cap->TxSTBC = 0; 	
		}
		
		/*
			RxSTBC
				0: not support,
				1: support for 1SS
				2: support for 1SS, 2SS
				3: support for 1SS, 2SS, 3SS
		*/
		ht_cap->HtCapInfo.RxSTBC = 1;
		rt_ht_cap->RxSTBC = 1;
	}
	else
	{
		rt_ht_cap->TxSTBC = 0;
		rt_ht_cap->RxSTBC = 0;
	}

	if(pHTPhyMode->SHORTGI == GI_400)
	{
		ht_cap->HtCapInfo.ShortGIfor20 = 1;
		//ht_cap->HtCapInfo.ShortGIfor40 = 1;
		rt_ht_cap->ShortGIfor20 = 1;
		if(pHTPhyMode->BW == BW_40)
		{
			ht_cap->HtCapInfo.ShortGIfor40 = 1;
			rt_ht_cap->ShortGIfor40 = 1;
		}
		else
		{
			ht_cap->HtCapInfo.ShortGIfor40 = 0;
			rt_ht_cap->ShortGIfor40 = 0;		
		}
	}
	else
	{
		ht_cap->HtCapInfo.ShortGIfor20 = 0;
		ht_cap->HtCapInfo.ShortGIfor40 = 0;
		rt_ht_cap->ShortGIfor20 = 0;
		rt_ht_cap->ShortGIfor40 = 0;
	}
	
	/* We support link adaptation for unsolicit MCS feedback, set to 2.*/
		wlan_operate_set_prim_ch(wdev,pHTPhyMode->Channel);
	/* 1, the extension channel above the control channel. */

#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgHwTxBfCap)
	{
#ifdef MT_MAC
        /* Set ETxBF */
        mt_WrapSetETxBFCap(pAd, wdev, &ht_cap->TxBFCap);

        /* Check ITxBF */
        //pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn &= mt_Wrap_chk_itxbf_calibration(pAd);
#else
		/* Set ETxBF */
		setETxBFCap(pAd, &ht_cap->TxBFCap);

		/* Check ITxBF */
		pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn &= rtmp_chk_itxbf_calibration(pAd);

#endif /* MT_MAC */   
       
		/* Apply to ASIC */

	}
#endif /* TXBF_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			RTMPSetIndividualHT(pAd, idx, wdev->channel);
#ifdef WDS_SUPPORT
		for (idx = 0; idx < MAX_WDS_ENTRY; idx++)
			RTMPSetIndividualHT(pAd, idx + MIN_NET_DEVICE_FOR_WDS, wdev->channel);
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
			RTMPSetIndividualHT(pAd, idx + MIN_NET_DEVICE_FOR_APCLI, wdev->channel);
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


}

/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  - 

	========================================================================
*/
VOID RTMPSetIndividualHT(RTMP_ADAPTER *pAd, UCHAR apidx, UCHAR channel)
{	
	RT_PHY_INFO *pDesired_ht_phy = NULL;
	UCHAR DesiredMcs = MCS_AUTO;
	UINT32 encrypt_mode = 0;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	EDCA_PARM *pEdca;
	UCHAR cfg_ht_bw;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): apidx=%d\n", __FUNCTION__, apidx));
	do
	{

#ifdef RT_CFG80211_P2P_SUPPORT
        if (apidx >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO)
        {                                                               
            UCHAR idx = apidx - MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO;
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;

			pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
			DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;                      
			encrypt_mode = wdev->SecConfig.PairwiseCipher;
			wdev->bWmmCapable = TRUE; 
			wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
            break;
        }
#endif /* RT_CFG80211_P2P_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT	
			if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			{				
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;
						
				if (idx < MAX_APCLI_NUM)
				{
					wdev= &pAd->ApCfg.ApCliTab[idx].wdev;
					pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
					DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
					encrypt_mode = wdev->SecConfig.PairwiseCipher;
					wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
					break;
				}
				else
				{
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid idx(%d)\n", __FUNCTION__, idx));
					return;
				}
			}
#endif /* APCLI_SUPPORT */

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{				
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;
						
				if (idx < MAX_WDS_ENTRY)
				{
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					pDesired_ht_phy = &wdev->DesiredHtPhyInfo;									
					DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;							
					/*encrypt_mode = pAd->WdsTab.WdsEntry[idx].WepStatus;*/
					wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
					break;
				}
				else
				{
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid apidx(%d)\n", __FUNCTION__, apidx));
					return;
				}
			}
#endif /* WDS_SUPPORT */
			if ((apidx < pAd->ApCfg.BssidNum) && (apidx < MAX_MBSSID_NUM(pAd)) && (apidx < HW_BEACON_MAX_NUM))
			{
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

				pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
				DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
#ifdef WFA_VHT_PF
				// TODO: Sigma, this code segment used to work around for Sigma Automation!
				if (WMODE_CAP_AC(wdev->PhyMode) && (DesiredMcs != MCS_AUTO)) {
					DesiredMcs += ((wlan_config_get_tx_stream(wdev) - 1) << 4);
					pAd->ApCfg.MBSSID[apidx].DesiredTransmitSetting.field.FixedTxMode = FIXED_TXMODE_VHT;
					RT_CfgSetAutoFallBack(pAd, "0");
				} else {
					RT_CfgSetAutoFallBack(pAd, "1");
				}
#endif /* WFA_VHT_PF */
				encrypt_mode = wdev->SecConfig.PairwiseCipher;
				wdev->bWmmCapable = TRUE;
				wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			}

			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid apidx(%d)\n", __FUNCTION__, apidx));
			return;
		}			
#endif /* CONFIG_AP_SUPPORT */


	pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];
	/* EDCA parameters used for AP's own transmission*/
	if (pEdca->bValid == FALSE)
		set_default_ap_edca_param(pEdca);
	} while (FALSE);

	if (wdev->channel != channel)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Only update for wdev on chl-%d, current wdev_idx(%d) is chl-%d\n", __FUNCTION__, 
			channel, wdev->wdev_idx, wdev->channel));
		return;
	}

	if (pDesired_ht_phy == NULL)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid apidx(%d)\n", __FUNCTION__, apidx));
		return;
	}
	RTMPZeroMemory(pDesired_ht_phy, sizeof(RT_PHY_INFO));

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetIndividualHT : Desired MCS = %d\n", DesiredMcs));
	/* Check the validity of MCS */
	if ((wlan_config_get_tx_stream(wdev) == 1) && ((DesiredMcs >= MCS_8) && (DesiredMcs <= MCS_15)))
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: MCS(%d) is invalid in 1S, reset it as MCS_7\n", __FUNCTION__, DesiredMcs));
		DesiredMcs = MCS_7;		
	}

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	if ((cfg_ht_bw == HT_BW_20) && (DesiredMcs == MCS_32))
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: MCS_32 is only supported in 40-MHz, reset it as MCS_0\n", __FUNCTION__));
		DesiredMcs = MCS_0;		
	}
	   		
	/*
		WFA recommend to restrict the encryption type in 11n-HT mode.
	 	So, the WEP and TKIP are not allowed in HT rate. 
	*/
	if (pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(encrypt_mode))
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s : Use legacy rate in WEP/TKIP encryption mode (apidx=%d)\n", 
									__FUNCTION__, apidx));
		return;
	}

	if (pAd->CommonCfg.HT_Disable)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : HT is disabled\n", __FUNCTION__));
		return;
	}
			
	pDesired_ht_phy->bHtEnable = TRUE;

	/* Decide desired Tx MCS*/
	switch (wlan_config_get_tx_stream(wdev))
	{
		case 1:
			if (DesiredMcs == MCS_AUTO)
				pDesired_ht_phy->MCSSet[0]= 0xff;
			else if (DesiredMcs <= MCS_7)
				pDesired_ht_phy->MCSSet[0]= 1<<DesiredMcs;
			break;

		case 2:
			if (DesiredMcs == MCS_AUTO)
			{
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
			}
			else if (DesiredMcs <= MCS_15)
			{
				ULONG mode;
				
				mode = DesiredMcs / 8;
				if (mode < 2)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}			
			break;

		case 3:
			if (DesiredMcs == MCS_AUTO)
			{
				/* MCS0 ~ MCS23, 3 bytes */
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
				pDesired_ht_phy->MCSSet[2]= 0xff;
			}
			else if (DesiredMcs <= MCS_23)
			{
				ULONG mode;

				mode = DesiredMcs / 8;
				if (mode < 3)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}
			break;

		case 4:
			if (DesiredMcs == MCS_AUTO)
			{
				/* MCS0 ~ MCS31, 4 bytes */
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
				pDesired_ht_phy->MCSSet[2]= 0xff;
				pDesired_ht_phy->MCSSet[3]= 0xff;
			}
			else if (DesiredMcs <= MCS_31)
			{
				ULONG mode;

				mode = DesiredMcs / 8;
				if (mode < 4)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}
			break;
	}							

	if(cfg_ht_bw == HT_BW_40)
	{
		if (DesiredMcs == MCS_AUTO || DesiredMcs == MCS_32)
			pDesired_ht_phy->MCSSet[4] = 0x1;
	}

	/* update HT Rate setting */
	if (pAd->OpMode == OPMODE_STA)
	{
			MlmeUpdateHtTxRates(pAd, BSS0);
	}
	else
		MlmeUpdateHtTxRates(pAd, apidx);

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode)) {
		pDesired_ht_phy->bVhtEnable = TRUE;
		rtmp_set_vht(pAd, pDesired_ht_phy);
	}
#endif /* DOT11_VHT_AC */
}

/*
	========================================================================
	Routine Description:
		Clear the desire HT info per interface
		
	Arguments:
	
	========================================================================
*/
VOID RTMPDisableDesiredHtInfo(RTMP_ADAPTER *pAd, UCHAR Channel)
{
#ifdef CONFIG_AP_SUPPORT
	UINT8 idx = 0;
    struct wifi_dev *wdev = NULL;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
		{
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
            if (Channel == wdev->channel)
            {
                RTMPZeroMemory(&wdev->DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
            }
		}
#ifdef WDS_SUPPORT
		for (idx = 0; idx < MAX_WDS_ENTRY; idx++)
		{				
            wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
            if (Channel == wdev->channel)
            {
                RTMPZeroMemory(&wdev->DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
            }
		}
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
		{				
            wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
            if (Channel == wdev->channel)
            {
                RTMPZeroMemory(&wdev->DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
            }
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


}


INT	SetCommonHT(RTMP_ADAPTER *pAd, UCHAR PhyMode, UCHAR Channel,struct wifi_dev *wdev)
{
	OID_SET_HT_PHYMODE SetHT;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	if (!WMODE_CAP_N(PhyMode))
	{
		/* Clear previous HT information */
		RTMPDisableDesiredHtInfo(pAd, Channel);
		return FALSE;
	}

	SetHT.PhyMode = (RT_802_11_PHY_MODE)PhyMode;
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.ExtOffset = ext_cha;
	SetHT.MCS = MCS_AUTO;
	SetHT.BW = (UCHAR)op_ht_bw;
	SetHT.STBC = (UCHAR)wlan_config_get_ht_stbc(wdev);
	SetHT.SHORTGI = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;		
	SetHT.Channel = Channel;
	SetHT.BandIdx=0;

	RTMPSetHT(pAd, &SetHT,wdev);

#ifdef DOT11N_DRAFT3
	if(pAd->CommonCfg.bBssCoexEnable && pAd->CommonCfg.Bss2040NeedFallBack)
	{
		wlan_operate_set_ht_bw(wdev,HT_BW_20);
		wlan_operate_set_ext_cha(wdev,EXTCHA_NONE);
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
		pAd->CommonCfg.Bss2040NeedFallBack = 1;
	}
#endif /* DOT11N_DRAFT3 */

	return TRUE;
}

/*
	========================================================================
	Routine Description:
		Update HT IE from our capability.
		
	Arguments:
		Send all HT IE in beacon/probe rsp/assoc rsp/action frame.
		
	
	========================================================================
*/
VOID RTMPUpdateHTIE(
	IN RT_HT_CAPABILITY	*pRtHt,
	IN UCHAR *pMcsSet,
	IN struct wifi_dev *wdev,
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT ADD_HT_INFO_IE *pAddHtInfo)
{
	UCHAR cfg_ht_bw  = wlan_config_get_ht_bw(wdev);
	UCHAR op_ht_bw = wlan_config_get_ht_bw(wdev);
	RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	
		pHtCapability->HtCapInfo.ChannelWidth = cfg_ht_bw;
		pHtCapability->HtCapInfo.MimoPs = pRtHt->MimoPs;
		pHtCapability->HtCapInfo.GF = pRtHt->GF;
		pHtCapability->HtCapInfo.ShortGIfor20 = pRtHt->ShortGIfor20;
		pHtCapability->HtCapInfo.ShortGIfor40 = pRtHt->ShortGIfor40;
		pHtCapability->HtCapInfo.TxSTBC = pRtHt->TxSTBC;
		pHtCapability->HtCapInfo.RxSTBC = pRtHt->RxSTBC;
		pHtCapability->HtCapInfo.AMsduSize = pRtHt->AmsduSize;
		pHtCapability->HtCapParm.MaxRAmpduFactor = pRtHt->MaxRAmpduFactor;
		pHtCapability->HtCapParm.MpduDensity = pRtHt->MpduDensity;

		pAddHtInfo->AddHtInfo.ExtChanOffset = pRtHt->ExtChanOffset ;
		pAddHtInfo->AddHtInfo.RecomWidth = op_ht_bw;
		pAddHtInfo->AddHtInfo2.OperaionMode = pRtHt->OperaionMode;
		pAddHtInfo->AddHtInfo2.NonGfPresent = pRtHt->NonGfPresent;
		RTMPMoveMemory(pAddHtInfo->MCSSet, /*pRtHt->MCSSet*/pMcsSet, 4); /* rt2860 only support MCS max=32, no need to copy all 16 uchar.*/
	
        MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTMPUpdateHTIE <== \n"));
}

#endif /* DOT11_N_SUPPORT */

