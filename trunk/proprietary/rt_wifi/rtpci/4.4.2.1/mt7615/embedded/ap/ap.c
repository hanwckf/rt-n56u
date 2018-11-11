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
    soft_ap.c

    Abstract:
    Access Point specific routines and MAC table maintenance routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP

 */

#include "rt_config.h"
#define VLAN_HDR_LEN 	4
#define MCAST_WCID_TO_REMOVE 0 //Pat: TODO
static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad);
char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
	"restart access point",
	"successfully associated",
	"has disassociated",
	"has been aged-out and disassociated" ,
	"active countermeasures",
	"has disassociated with invalid PSK password"};


UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr)
{
	UCHAR apidx;

	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		if (RTMPEqualMemory(addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
			break;
	}

	return apidx;
}

static INT ap_mlme_set_capability(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
    struct wifi_dev *wdev = &pMbss->wdev;
    BOOLEAN SpectrumMgmt = FALSE;

#ifdef A_BAND_SUPPORT
    /* Decide the Capability information field */
    /* In IEEE Std 802.1h-2003, the spectrum management bit is enabled in the 5 GHz band */
    if ((wdev->channel > 14) && pAd->CommonCfg.bIEEE80211H == TRUE)
        SpectrumMgmt = TRUE;
#endif /* A_BAND_SUPPORT */

	pMbss->CapabilityInfo = CAP_GENERATE(1,
									0,
									IS_SECURITY_Entry(wdev),
									(pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
									pAd->CommonCfg.bUseShortSlotTime,
									SpectrumMgmt);

#ifdef DOT11K_RRM_SUPPORT
    if (pMbss->RrmCfg.bDot11kRRMEnable == TRUE)
        pMbss->CapabilityInfo |= RRM_CAP_BIT;
#endif /* DOT11K_RRM_SUPPORT */

    if (pMbss->wdev.bWmmCapable == TRUE)
    {
        /*
            In WMM spec v1.1, A WMM-only AP or STA does not set the "QoS"
            bit in the capability field of association, beacon and probe
            management frames.
        */
/*          pMbss->CapabilityInfo |= 0x0200; */
    }

#ifdef UAPSD_SUPPORT
    if (pMbss->wdev.UapsdInfo.bAPSDCapable == TRUE)
    {
        /*
            QAPs set the APSD subfield to 1 within the Capability
            Information field when the MIB attribute
            dot11APSDOptionImplemented is true and set it to 0 otherwise.
            STAs always set this subfield to 0.
        */
        pMbss->CapabilityInfo |= 0x0800;
    }
#endif /* UAPSD_SUPPORT */

    return TRUE;
}


static VOID ApAutoChannelAtBootUp(RTMP_ADAPTER *pAd)
{

    #define SINGLE_BAND 0
    #define DUAL_BAND   1
    struct wifi_dev *pwdev;
    INT i;
    UCHAR NewChannel;
    UCHAR AutoChannel2GDone = FALSE;
    UCHAR AutoChannel5GDone = FALSE;
    BOOLEAN IsAband;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s----------------->\n", __FUNCTION__));

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: AutoChannelBootup = %d, AutoChannelFlag = %d\n",
    __FUNCTION__, pAd->ApCfg.bAutoChannelAtBootup, pAd->AutoChSelCtrl.AutoChannelFlag));
    
    if (!pAd->ApCfg.bAutoChannelAtBootup || (pAd->AutoChSelCtrl.AutoChannelFlag == 0))
    {
       MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s<-----------------\n", __FUNCTION__));
       return;
    }
    
    /* Now Enable RxTx*/
    RTMPEnableRxTx(pAd); 

    for (i = 0; i < pAd->ApCfg.BssidNum; i++)
    {            
        pwdev = &pAd->ApCfg.MBSSID[i].wdev;
        
        if (pwdev->channel == 0)
        {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
            ("%s: PhyMode: %d\n", __FUNCTION__, pwdev->PhyMode));
            
            if (WMODE_CAP_2G(pwdev->PhyMode) && AutoChannel2GDone == FALSE)
            {
                IsAband = FALSE;
                
                pAd->AutoChSelCtrl.PhyMode = pwdev->PhyMode; 
                
                /* Now we can receive the beacon and do the listen beacon*/
                NewChannel = AP_AUTO_CH_SEL(pAd, pAd->ApCfg.AutoChannelAlg, IsAband);
                
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                ("%s : Auto channel selection for 2G channel =%d\n", __FUNCTION__, NewChannel));
                
                AutoChSelUpdateChannel(pAd, NewChannel, IsAband);
                
                AutoChannel2GDone = TRUE; 
            }   
            else if (WMODE_CAP_5G(pwdev->PhyMode) && AutoChannel5GDone == FALSE)
            {      
                IsAband = TRUE;

                pAd->AutoChSelCtrl.PhyMode = pwdev->PhyMode; 
                
                /* Now we can receive the beacon and do the listen beacon*/
                NewChannel = AP_AUTO_CH_SEL(pAd, pAd->ApCfg.AutoChannelAlg, IsAband);
                
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
                ("%s : Auto channel selection for 5G channel = %d\n", __FUNCTION__, NewChannel));
                
                AutoChSelUpdateChannel(pAd, NewChannel, IsAband);
                
                AutoChannel5GDone = TRUE;                        
            }
        }
          
        if((pAd->CommonCfg.dbdc_mode == DUAL_BAND) && AutoChannel2GDone && AutoChannel5GDone)
        {
            break;
        }
        else if ((pAd->CommonCfg.dbdc_mode == SINGLE_BAND) && (AutoChannel2GDone || AutoChannel5GDone))
        {
            break;
        }
    }

    pAd->ApCfg.bAutoChannelAtBootup = FALSE;
    
    pAd->AutoChSelCtrl.AutoChannelFlag = 0;
    
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s<-----------------\n", __FUNCTION__));
}


/*
	==========================================================================
	Description:
        Check to see the exist of long preamble STA in associated list
    ==========================================================================
 */
static BOOLEAN ApCheckLongPreambleSTA(RTMP_ADAPTER *pAd)
{
    UCHAR   i;

    for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
    {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

        if (!CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
	==========================================================================
	Description:
		Initialize AP specific data especially the NDIS packet pool that's
		used for wireless client bridging.
	==========================================================================
 */
static VOID ApChannelCheckForRfIC(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	UCHAR Channel = HcGetChannelByRf(pAd,RfIC);
	UCHAR PhyMode = HcGetPhyModeByRf(pAd,RfIC);
	if(!HcIsRfSupport(pAd,RfIC))
	{
		return;
	}

#ifdef DOT11_N_SUPPORT
	/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel	*/
	N_ChannelCheck(pAd,PhyMode,Channel);
#endif /*DOT11_N_SUPPORT*/
}


NDIS_STATUS APOneShotSettingInitialize(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)	
	UCHAR Channel = 0;
	INT32	ret=0;
#endif /* BACKGROUND_SCAN_SUPPORT */
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

	/*AP Open*/
	/*initial phymode first*/
	HcSetPhyMode(pAd,wdev->PhyMode);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> APOneShotSettingInitialize\n"));

	RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, GET_TIMER_FUNCTION(CMTimerExec), pAd, FALSE);

#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPInitTimer(pAd, &pAd->ApCfg.IDSTimer, GET_TIMER_FUNCTION(RTMPIdsPeriodicExec), pAd, FALSE);
	pAd->ApCfg.IDSTimerRunning = FALSE;
#endif /* IDS_SUPPORT */


#ifdef WAPI_SUPPORT
	{
		UINT8 apidx = 0;
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
	    	{
	    		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	    		RTMPInitWapiRekeyTimerByWdev(pAd, wdev);
	    	}
	}
#endif /* WAPI_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
	MulticastFilterTableInit(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	initList(&pAd->DMSEntryList);
#endif /* DOT11V_WNM_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
	/* Enable All Bssid Gas Rsp Capability */
	{
		UCHAR APIndex = 0;
		for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
		{
			pAd->ApCfg.MBSSID[APIndex].GASCtrl.b11U_enable = TRUE;
		}	
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	RRM_CfgInit(pAd);
#endif /* DOT11K_RRM_SUPPORT */

	BuildChannelList(pAd);

	/* Init BssTab & ChannelInfo tabbles for auto channel select.*/
	AutoChBssTableInit(pAd);
	ChannelInfoInit(pAd);

        ApAutoChannelAtBootUp(pAd);

        WifiSysOpen(pAd,wdev);

#ifdef MT_DFS_SUPPORT
	DfsDedicatedExclude(pAd);
#endif

	if (CheckNonOccupancyChannel(pAd, wdev->channel) == FALSE) {
#ifdef DFS_DBG_LOG_0		
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Update to first channel\n", __FUNCTION__));
#endif
		HcUpdateChannel(pAd, FirstChannel(pAd));
	}
    
#ifdef BACKGROUND_SCAN_SUPPORT	
	if (pAd->CommonCfg.dbdc_mode == TRUE) {
       		pAd->BgndScanCtrl.BgndScanSupport = 0;
		pAd->BgndScanCtrl.DfsZeroWaitSupport = 0;
#ifdef  MT_DFS_SUPPORT
        UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
#endif 
	}        
#ifdef DOT11_VHT_AC	
	else if (pAd->CommonCfg.vht_bw == VHT_BW_160 || pAd->CommonCfg.vht_bw == VHT_BW_8080 ) {
		pAd->BgndScanCtrl.BgndScanSupport = 0;
		pAd->BgndScanCtrl.DfsZeroWaitSupport = 0;
#ifdef  MT_DFS_SUPPORT
            UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
#endif
	}    
#endif /* DOT11_VHT_AC */	                
	else {
                        pAd->BgndScanCtrl.BgndScanSupport = 1;
	}

#ifdef MT_DFS_SUPPORT 	
	Channel  = HcGetChannelByRf(pAd, RFIC_5GHZ);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Current Channel is %d. DfsZeroWaitSupport=%d\n", Channel, pAd->BgndScanCtrl.DfsZeroWaitSupport));
	if (Channel !=0 && pAd->BgndScanCtrl.DfsZeroWaitSupport && pAd->BgndScanCtrl.BgndScanSupport && RadarChannelCheck(pAd, Channel)) {

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Channel %d is a DFS channel. Trigger Auto Channel selection!\n", Channel));
		pAd->BgndScanCtrl.DfsZeroWaitChannel = Channel; /* Record original channel for DFS zero wait */
		pAd->BgndScanCtrl.SkipDfsChannel = 1; //Notify AP_AUTO_CH_SEL skip DFS channel.
		/* Re-select a non-DFS channel. */
		Channel = AP_AUTO_CH_SEL(pAd, ChannelAlgBusyTime, TRUE);
		pAd->BgndScanCtrl.SkipDfsChannel = 0;//Clear.
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Get a new Channel %d for DFS zero wait (temporary) using!\n", Channel));
		ret = HcUpdateChannel(pAd,Channel);
                        	if(ret < 0 )
                        	{
                               		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                               		("%s(): Update Channel %d faild, not support this RF\n", __FUNCTION__, Channel));
                        	}  
       
       UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INIT_CAC);
	}
  else
  {
      if (pAd->BgndScanCtrl.DfsZeroWaitSupport)
      {
          pAd->BgndScanCtrl.DfsZeroWaitChannel = 0;
   
          UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);  
      }
  }

  /* Update bInitMbssZeroWait for MBSS Zero Wait */
  UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, FALSE);

#endif /* MT_DFS_SUPPORT */  
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	/*per band checking*/
	ApChannelCheckForRfIC(pAd,RFIC_24GHZ);
	ApChannelCheckForRfIC(pAd,RFIC_5GHZ);

	RTMP_11N_D3_TimerInit(pAd);

#endif /* DOT11N_DRAFT3 */
#endif /*DOT11_N_SUPPORT*/



#ifdef AP_QLOAD_SUPPORT
    QBSS_LoadInit(pAd);
#endif /* AP_QLOAD_SUPPORT */

    /*
        Some modules init must be called before APStartUp().
        Or APStartUp() will make up beacon content and call
        other modules API to get some information to fill.
    */


#ifdef MAT_SUPPORT
    MATEngineInit(pAd);
#endif /* MAT_SUPPORT */

#ifdef CLIENT_WDS
    CliWds_ProxyTabInit(pAd);
#endif /* CLIENT_WDS */

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--- APOneShotSettingInitialize\n"));
	return Status;
}


/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID APShutdown(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> APShutdown\n"));

#ifdef MT_MAC
	/*	Disable RX */
	//MtAsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE,0);

#ifdef RTMP_MAC_PCI
	APStop(pAd);
#endif /* RTMP_MAC_PCI */
	//MlmeRadioOff(pAd);
#else
		MlmeRadioOff(pAd);

#ifdef RTMP_MAC_PCI
		APStop(pAd);
#endif /* RTMP_MAC_PCI */
#endif

	/*remove sw related timer and table*/
	rtmp_ap_exit(pAd);


#ifdef DOT11V_WNM_SUPPORT
	DMSTable_Release(pAd);
#endif /* DOT11V_WNM_SUPPORT */

	NdisFreeSpinLock(&pAd->MacTabLock);

#ifdef WDS_SUPPORT
	NdisFreeSpinLock(&pAd->WdsTab.WdsTabLock);
#endif /* WDS_SUPPORT */

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--- APShutdown\n"));
}



/*
	==========================================================================
	Description:
		Update ERP IE and CapabilityInfo based on STA association status.
		The result will be auto updated into the next outgoing BEACON in next
		TBTT interrupt service routine
	==========================================================================
 */

VOID ApUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd,struct _BSS_STRUCT *mbss)
{
	UCHAR  i, ErpIeContent = 0;
	BOOLEAN ShortSlotCapable = pAd->CommonCfg.bUseShortSlotTime;
	BOOLEAN bUseBGProtection;
	BOOLEAN LegacyBssExist;
	MAC_TABLE_ENTRY *pEntry = NULL;
	USHORT *pCapInfo = NULL;
	struct wifi_dev *wdev = &mbss->wdev;
	ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
	UCHAR Channel = wdev->channel;
	UCHAR PhyMode = wdev->PhyMode;
	UCHAR band = HcGetBandByWdev(wdev);

	if (WMODE_EQUAL(PhyMode, WMODE_B))
	{
		return;
	}

    for (i=1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
    {
        pEntry = &pAd->MacTab.Content[i];

        if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
        {
            continue;
        }

        if(wdev != pEntry->wdev)
        {
            continue;
        }

        /* at least one 11b client associated, turn on ERP.NonERPPresent bit */
        /* almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility */
        if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
        {
            ShortSlotCapable = FALSE;
            ErpIeContent |= 0x01;
        }

        /* at least one client can't support short slot */
        if ((pEntry->CapabilityInfo & 0x0400) == 0)
            ShortSlotCapable = FALSE;
    }

    /* legacy BSS exist within 5 sec */
    if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32)
        LegacyBssExist = TRUE;
    else
        LegacyBssExist = FALSE;

    /* decide ErpIR.UseProtection bit, depending on pAd->CommonCfg.UseBGProtection
       AUTO (0): UseProtection = 1 if any 11b STA associated
       ON (1): always USE protection
       OFF (2): always NOT USE protection
       */
    if (pAd->CommonCfg.UseBGProtection == 0)
    {
        ErpIeContent = (ErpIeContent)? 0x03 : 0x00;
        /*if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) // legacy BSS exist within 5 sec */
        if (LegacyBssExist)
        {
            ErpIeContent |= 0x02;                                     /* set Use_Protection bit */
        }
    }
    else if (pAd->CommonCfg.UseBGProtection == 1)
    {
        ErpIeContent |= 0x02;
    }

    bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
        ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(ErpIeContent));

#ifdef A_BAND_SUPPORT
    /* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
    if (Channel > 14)
        bUseBGProtection = FALSE;
#endif /* A_BAND_SUPPORT */

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
            ("-- bUseBGProtection: %s, BG_PROTECT_INUSED: %s, ERP IE Content: 0x%x\n",
             (bUseBGProtection)?"Yes":"No",
             (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))?"Yes":"No",
             ErpIeContent));

    if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
    {
        USHORT OperationMode = 0;
        BOOLEAN	bNonGFExist = 0;

#ifdef DOT11_N_SUPPORT
        OperationMode = addht->AddHtInfo2.OperaionMode;

	bNonGFExist = pAd->MacTab.fAnyStationNonGF[band];
#endif /* DOT11_N_SUPPORT */

        if (bUseBGProtection)
        {
            OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
#if defined(RTMP_MAC) || defined(RLT_MAC)
            if (pAd->chipCap.hif_type == HIF_RTMP
                    || pAd->chipCap.hif_type == HIF_RLT)
            {
                AsicUpdateProtect(pAd, OperationMode,
                        (OFDMSETPROTECT), FALSE, bNonGFExist);
            }
#endif
        }
        else
        {
            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
#if defined(RTMP_MAC) || defined(RLT_MAC)
            if (pAd->chipCap.hif_type == HIF_RTMP
                    || pAd->chipCap.hif_type == HIF_RLT)
            {
                AsicUpdateProtect(pAd, OperationMode,
                        (OFDMSETPROTECT), TRUE, bNonGFExist);
            }
#endif
        }

    }

    /* Decide Barker Preamble bit of ERP IE */
    if ((pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong) || (ApCheckLongPreambleSTA(pAd) == TRUE))
        pAd->ApCfg.ErpIeContent = (ErpIeContent | 0x04);
    else
        pAd->ApCfg.ErpIeContent = ErpIeContent;

#ifdef A_BAND_SUPPORT
    /* Force to use ShortSlotTime at A-band */
    if(Channel> 14)
    {
        ShortSlotCapable = TRUE;
    }
#endif /* A_BAND_SUPPORT */

    pCapInfo = &(mbss->CapabilityInfo);

    /* In A-band, the ShortSlotTime bit should be ignored. */
    if (ShortSlotCapable
#ifdef A_BAND_SUPPORT
            && (Channel <= 14)
#endif /* A_BAND_SUPPORT */
       )
        (*pCapInfo) |= 0x0400;
    else
        (*pCapInfo) &= 0xfbff;


    if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong)
        (*pCapInfo) &= (~0x020);
    else
        (*pCapInfo) |= 0x020;


	/*update slot time only when value is difference*/
	if(pAd->CommonCfg.bUseShortSlotTime!=ShortSlotCapable)
	{
    	HW_SET_SLOTTIME(pAd, ShortSlotCapable, Channel, NULL);
		pAd->CommonCfg.bUseShortSlotTime = ShortSlotCapable;
	}
}



static INT ap_hw_tb_init(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Reset WCID Table\n", __FUNCTION__));
		HW_SET_DEL_ASIC_WCID(pAd, WCID_ALL);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Reset Sec Table\n", __FUNCTION__));
	return TRUE;
}


static UCHAR channel_bw_rule(struct wifi_dev *wdev, UCHAR bwByCap)
{
	UCHAR bwByRule = BW_20;
	UCHAR bwByCfg = wlan_config_get_vht_bw(wdev);	
    
	if (bwByCfg == VHT_BW_2040)
    {
    	if (wlan_config_get_ht_bw(wdev) == HT_BW_20)
       		bwByCfg = BW_20;
        else
            bwByCfg = BW_40;
    }
    else if (bwByCfg == VHT_BW_80)
        bwByCfg = BW_80;
    else if (bwByCfg == VHT_BW_160)
        bwByCfg = BW_160;
    else if (bwByCfg == VHT_BW_8080)
        bwByCfg = BW_8080;

    /* check per-Wdev Bw with Channel Bw from region */
    if (bwByCfg == BW_8080)
    {
        if (bwByCap == BW_80)
        	bwByRule = BW_8080;
        else if (bwByCap == BW_160)
            bwByRule = BW_8080;
        else
            bwByRule = bwByCap;

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: IN BW8080 wdev(%d)  %d [%d, %d] %d\n"
                ,__FUNCTION__, wdev->wdev_idx, wlan_operate_get_bw(wdev), bwByCfg, bwByCap, bwByRule));
    }
    else
    {
        bwByRule = (bwByCap > bwByCfg) ? bwByCfg : bwByCap;
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wdev(%d)  %d [%d, %d] %d\n"
               ,__FUNCTION__, wdev->wdev_idx, wlan_operate_get_bw(wdev), bwByCfg, bwByCap, bwByRule));
    }

	return bwByRule;

}

static VOID channel_adjust_bw_by_region(RTMP_ADAPTER *pAd, UCHAR RfIC, UCHAR Channel)
{
    UCHAR i = 0;
	BOOLEAN bFoundCh = FALSE;
    BOOLEAN isChanged = FALSE;

    for (i = 0; (i < pAd->ChannelListNum) && (i < MAX_NUM_OF_CHANNELS); i++)
    {
        if (Channel == pAd->ChannelList[i].Channel)
        {
        	bFoundCh = TRUE;
            break;
        }
    }
	
	if (bFoundCh == FALSE) 
		return;

    if (Channel > 14)
    {
        UCHAR bwByCap = BW_20;
        UCHAR oriCurBw = HcGetBwByRf(pAd, RFIC_5GHZ);
        UCHAR bwByRule = BW_20, htBw = HT_BW_20, vhtBw = VHT_BW_2040;

		struct wifi_dev *wdev;
		
    	if (pAd->ChannelList[i].Flags & CHANNEL_160M_CAP)
          	bwByCap = BW_160;
        else if (pAd->ChannelList[i].Flags & CHANNEL_80M_CAP)
            bwByCap = BW_80;
        else if (pAd->ChannelList[i].Flags & CHANNEL_40M_CAP)
            bwByCap = BW_40;
        else
            bwByCap = BW_20;

		for(i=0; i<WDEV_NUM_MAX; i++)
		{
			wdev = pAd->wdev_list[i];

			if (wdev && (RfIC & wmode_2_rfic(wdev->PhyMode)) && 
				WMODE_HT_ONLY(wdev->PhyMode))
			{
			    oriCurBw = wlan_operate_get_bw(wdev);				
				bwByRule = channel_bw_rule(wdev, bwByCap);
	
        		if (oriCurBw != bwByRule)
        		{
            		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                     		("%s: channel(%d) wdev(%d) change bw from %d to %d\n"
                            	,__FUNCTION__, Channel, wdev->wdev_idx, oriCurBw, bwByRule));
	
            		if (bwByRule == BW_40)
            		{
                		vhtBw = VHT_BW_2040;
                		htBw = HT_BW_40;
            		}
            		else if (bwByRule == BW_80)
            		{
                		vhtBw = VHT_BW_80;
                		htBw = HT_BW_40;
            		}
					else if (bwByRule == BW_8080)
					{
						vhtBw = VHT_BW_8080;
                        htBw = HT_BW_40;
					}
					else if (bwByRule == BW_160)
					{
						vhtBw = VHT_BW_160;
                        htBw = HT_BW_40;
					}
					else
					{
                        vhtBw = VHT_BW_2040;
                        htBw = HT_BW_20;
					}

					/* Restore the ext_ch info from config when extend BW from 20 to 40 */
                    if((oriCurBw == BW_20) && (bwByRule > BW_20))
                        wlan_operate_set_ext_cha(wdev, wlan_config_get_ext_cha(wdev));
                    else if (bwByRule == BW_20)
                        wlan_operate_set_ext_cha(wdev, EXTCHA_NONE);

                    wlan_operate_set_ht_bw(wdev, htBw);
                    wlan_operate_set_vht_bw(wdev, vhtBw);   
			
					isChanged = TRUE;
                 }
            }
        }
		
		if (isChanged)
	    {
			UCHAR ht_rf_bw, ext_ch, new_rf_bw, new_vht_bw = VHT_BW_2040;
#ifdef DOT11_N_SUPPORT
        	if (get_ht_cent_ch(pAd, &ht_rf_bw, &ext_ch,Channel) == FALSE)
#endif /* DOT11_N_SUPPORT */
        	{
            	ht_rf_bw = BW_20;
                ext_ch = EXTCHA_NONE;
                pAd->CommonCfg.CentralChannel = Channel;
        	}
			
			new_rf_bw = decide_phy_bw_by_channel(pAd,Channel);
	                
			if (ht_rf_bw == BW_40)
			{	
				switch (new_rf_bw)
				{
					case BW_80:
						new_vht_bw = VHT_BW_80;
						break;
					case BW_160:
						new_vht_bw = VHT_BW_160;
						break;
					case BW_8080:
						new_vht_bw = VHT_BW_8080;
						break;
					default:
						new_vht_bw = VHT_BW_2040;	
						break;
					
				}
			}
		
				pAd->CommonCfg.vht_cent_ch = vht_cent_ch_freq(Channel, new_vht_bw);
	        }
	}

}

#ifndef WH_EZ_SETUP
static
#endif
INT ap_phy_rrm_init_byRf(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	UCHAR PhyMode = HcGetPhyModeByRf(pAd,RfIC);
	UCHAR Channel = HcGetChannelByRf(pAd,RfIC);
	UCHAR BandIdx = HcGetBandByChannel(pAd,Channel);
	UCHAR phy_bw = decide_phy_bw_by_channel(pAd,Channel);

    if (Channel== 0 )
	{
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%sBandIdx %d Channel setting is 0\n", __FUNCTION__,BandIdx));

        return FALSE;
		}

	pAd->CommonCfg.CentralChannel = Channel;

	AsicSetTxStream(pAd, pAd->Antenna.field.TxPath, OPMODE_AP, TRUE,BandIdx);
	AsicSetRxStream(pAd, pAd->Antenna.field.RxPath, BandIdx);

		// TODO: shiang-usw, get from MT7620_MT7610 Single driver, check this!!
	N_ChannelCheck(pAd,PhyMode,Channel);//correct central channel offset
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pAd->CommonCfg.CentralChannel=%d\n",pAd->CommonCfg.CentralChannel));
	AsicBBPAdjust(pAd,Channel);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pAd->CommonCfg.CentralChannel=%d\n",pAd->CommonCfg.CentralChannel));

		channel_adjust_bw_by_region(pAd, RfIC, Channel);
		phy_bw = decide_phy_bw_by_channel(pAd,Channel);
		
#ifdef DOT11_VHT_AC

		if ((phy_bw == BW_80) || (phy_bw == BW_160) || (phy_bw == BW_8080))
			pAd->hw_cfg.cent_ch = pAd->CommonCfg.vht_cent_ch;
		else
#endif /* DOT11_VHT_AC */
			pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;


		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pAd->CommonCfg.CentralChannel=%d\n",pAd->CommonCfg.CentralChannel));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pAd->hw_cfg.cent_ch=%d,BW=%d\n",pAd->hw_cfg.cent_ch,phy_bw));

		AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, FALSE);
		AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);

#ifdef DOT11_VHT_AC
	//+++Add by shiang for debug
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): AP Set CentralFreq at %d(Prim=%d, HT-CentCh=%d, VHT-CentCh=%d, BBP_BW=%d)\n",
						__FUNCTION__, pAd->hw_cfg.cent_ch, Channel,
							pAd->CommonCfg.CentralChannel, pAd->CommonCfg.vht_cent_ch,
							phy_bw));
	//---Add by shiang for debug
#endif /* DOT11_VHT_AC */
	return TRUE;
}

/*
	==========================================================================
	Description:
		Start AP service. If any vital AP parameter is changed, a STOP-START
		sequence is required to disassociate all STAs.

	IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
	IRQL = PASSIVE_LEVEL. (from InitializeHandler)

	Note:
		Can't call NdisMIndicateStatus on this routine.

		RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
		Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

	==========================================================================
 */

VOID APStartUpForMbss(RTMP_ADAPTER *pAd,BSS_STRUCT *pMbss)
{
    struct wifi_dev *wdev = &pMbss->wdev;
    BOOLEAN bWmmCapable = FALSE;
    EDCA_PARM *pEdca = NULL, *pBssEdca = NULL;
    UCHAR phy_mode = pAd->CommonCfg.cfg_wmode;
    UINT8 BandIdx;
    
#ifdef GREENAP_SUPPORT
    struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)	
    BOOLEAN bZeroWaitStop;
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
    bZeroWaitStop = MbssZeroWaitStopValidate(pAd, pMbss->wdev.channel, pMbss->mbss_idx);
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s(), CfgMode:%d\n", __FUNCTION__, phy_mode));

    BandIdx = HcGetBandByWdev(wdev);

    /*Ssid length sanity check.*/
    if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID))
    {
        NdisMoveMemory(pMbss->Ssid, "HT_AP", 5);
        pMbss->Ssid[5] = '0' + pMbss->mbss_idx;
        pMbss->SsidLen = 6;
    }

    if (wdev->func_idx == 0)
    {
       MgmtTableSetMcastEntry(pAd, MCAST_WCID_TO_REMOVE);
    }

    APSecInit(pAd, wdev);

#ifdef STA_FORCE_ROAM_SUPPORT
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("\n[Force Roam] => Force Roam Support = %d\n",pAd->en_force_roam_supp));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[Force Roam] => StaLowRssiThr=%d dBm low_sta_renotify=%d sec StaAgeTime=%d sec\n",pAd->sta_low_rssi, pAd->low_sta_renotify,pAd->sta_age_time)); 	
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[Force Roam] => MntrAgeTime=%d sec mntr_min_pkt_count=%d mntr_min_time=%d sec mntr_avg_rssi_pkt_count=%d\n",
		pAd->mntr_age_time, pAd->mntr_min_pkt_count,pAd->mntr_min_time, pAd->mntr_avg_rssi_pkt_count));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[Force Roam] => AclAgeTime=%d sec AclHoldTime=%d sec\n",pAd->acl_age_time, pAd->acl_hold_time));
#endif

#ifdef MWDS
      if (wdev->bDefaultMwdsStatus == TRUE){
              MWDSEnable(pAd,wdev->func_idx,TRUE,FALSE);
      }
#ifdef IGMP_SNOOP_SUPPORT
	// Deduce IPv6 Link local address for this MBSS & IPv6 format checksum for use in MLD query message
	calc_mldv2_gen_query_chksum(pAd,pMbss);
#endif   
#endif
 
#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev))
		ez_start(wdev, TRUE);
	else 
	{		
		ez_allocate_or_update_non_ez_band(wdev);
	}
#endif /* WH_EZ_SETUP */
	
        ap_mlme_set_capability(pAd, pMbss);

#ifdef WSC_V2_SUPPORT
        if (pMbss->WscControl.WscV2Info.bEnableWpsV2)
        {
            /*
                WPS V2 doesn't support Chiper WEP and TKIP.
            */
        struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
        if (IS_CIPHER_WEP_TKIP_ONLY(pSecConfig->PairwiseCipher)
            || (pMbss->bHideSsid))
            WscOnOff(pAd, wdev->func_idx, TRUE);
        else
            WscOnOff(pAd, wdev->func_idx, FALSE);
        }
#endif /* WSC_V2_SUPPORT */

    /* If any BSS is WMM Capable, we need to config HW CRs */
    if (wdev->bWmmCapable)
    {
        bWmmCapable = TRUE;
    }

    if (WMODE_CAP_N(wdev->PhyMode) || bWmmCapable)
    {
        pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];
        /* EDCA parameters used for AP's own transmission */
        if (pEdca->bValid == FALSE)
            set_default_ap_edca_param(pEdca);

        pBssEdca = wlan_config_get_ht_edca(wdev);
        if (pBssEdca)
        {
            /* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
            if (pBssEdca->bValid == FALSE)
                set_default_sta_edca_param(pBssEdca);
        }
    }

#ifdef DOT11_N_SUPPORT
    RTMPSetPhyMode(pAd,wdev, wdev->PhyMode);
    /*update rate info for wdev*/
    RTMPUpdateRateInfo(wdev->PhyMode,&wdev->rate);

    if (!WMODE_CAP_N(wdev->PhyMode))
    {
        wlan_config_set_ht_bw(wdev,HT_BW_20);
    }

#ifdef DOT11N_DRAFT3
    /*
        We only do this Overlapping BSS Scan when system up, for the
        other situation of channel changing, we depends on station's
        report to adjust ourself.
    */

    if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
                                    pAd->CommonCfg.bBssCoexEnable,
                                    pAd->CommonCfg.bForty_Mhz_Intolerant));
    }
    else if(pAd->CommonCfg.bBssCoexEnable == TRUE)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n",
                    pAd->CommonCfg.bBssCoexEnable));
        APOverlappingBSSScan(pAd,wdev);
    }
#endif /* DOT11N_DRAFT3 */

    if (pAd->CommonCfg.bRdg) {
        AsicSetRDG(pAd, WCID_ALL, 0, 0, 0);
        if (pAd->CommonCfg.dbdc_mode) {
            AsicSetRDG(pAd, WCID_ALL, 1, 0, 0);
        }
    }
#endif /*DOT11_N_SUPPORT*/

    MlmeUpdateTxRates(pAd, FALSE, wdev->func_idx);
#ifdef DOT11_N_SUPPORT
    if (WMODE_CAP_N(wdev->PhyMode))
    {
        MlmeUpdateHtTxRates(pAd, wdev->func_idx);
    }
#endif /* DOT11_N_SUPPORT */

    WifiSysApLinkUp(pAd,wdev);

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
    ZeroWaitUpdateForMbss(pAd, bZeroWaitStop, pMbss->wdev.channel, pMbss->mbss_idx);    
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */

    RadarStateCheck(pAd, wdev);
#ifdef MT_DFS_SUPPORT
     if((pAd->Dot11_H.RDMode == RD_SILENCE_MODE) && (pMbss->wdev.channel > 14)) 
     {
        DfsCacNormalStart(pAd);
     }
#endif

#if defined(MT7615)
    /*==============================================================================*/
#ifdef SINGLE_SKU_V2

#ifdef RF_LOCKDOWN
    /* Check RF lock Status */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
    {
        pAd->CommonCfg.SKUenable[BandIdx] = TRUE;
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! SKUenable = 1!! \n", __FUNCTION__));
    }
#endif /* RF_LOCKDOWN */

    /* enable/disable SKU via profile */
    TxPowerSKUCtrl(pAd, pAd->CommonCfg.SKUenable[BandIdx], BandIdx);

#endif /* SINGLE_SKU_V2*/

    /* enable/disable Power Percentage via profile */
    TxPowerPercentCtrl(pAd, pAd->CommonCfg.PERCENTAGEenable[BandIdx], BandIdx);

#ifdef RF_LOCKDOWN
    /* Check RF lock Status */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
    {
        pAd->CommonCfg.BFBACKOFFenable[BandIdx] = TRUE;      
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! BFBACKOFFenable = 1!! \n", __FUNCTION__));
    }
#endif /* RF_LOCKDOWN */

    /* enable/disable BF Backoff via profile */
    TxPowerBfBackoffCtrl(pAd, pAd->CommonCfg.BFBACKOFFenable[BandIdx], BandIdx);

#ifdef TX_POWER_CONTROL_SUPPORT
	/* config Power boost table via profile */
	TxPwrUpCtrl(pAd, BandIdx, POWER_UP_CATE_CCK_OFDM, pAd->CommonCfg.cPowerUpCckOfdm[BandIdx]);
	TxPwrUpCtrl(pAd, BandIdx, POWER_UP_CATE_HT20, pAd->CommonCfg.cPowerUpHt20[BandIdx]);
	TxPwrUpCtrl(pAd, BandIdx, POWER_UP_CATE_HT40, pAd->CommonCfg.cPowerUpHt40[BandIdx]);
	TxPwrUpCtrl(pAd, BandIdx, POWER_UP_CATE_VHT20, pAd->CommonCfg.cPowerUpVht20[BandIdx]);
	TxPwrUpCtrl(pAd, BandIdx, POWER_UP_CATE_VHT40, pAd->CommonCfg.cPowerUpVht40[BandIdx]);
	TxPwrUpCtrl(pAd, BandIdx, POWER_UP_CATE_VHT80, pAd->CommonCfg.cPowerUpVht80[BandIdx]);
	TxPwrUpCtrl(pAd, BandIdx, POWER_UP_CATE_VHT160, pAd->CommonCfg.cPowerUpVht160[BandIdx]);
#endif /* TX_POWER_CONTROL_SUPPORT */
	
#ifdef NR_PD_DETECTION
    /* enable Link test Spatial Extension config */
    if (pAd->CommonCfg.LinkTestSupport)
    {
        MtCmdLinkTestSeIdxCtrl(pAd, TRUE);
    }
#endif /* NR_PD_DETECTION */

    /*==============================================================================*/
#endif /* defined(MT7615) */    
    
    if(pMbss->wdev.channel > 14)
        ap_phy_rrm_init_byRf(pAd,RFIC_5GHZ);
    else
        ap_phy_rrm_init_byRf(pAd,RFIC_24GHZ);

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef RADIO_LINK_SELECTION
	if (pAd->ApCfg.RadioLinkSelection == FALSE || 
		pAd->ApCfg.RlsTable.bTiggerByUser != TRUE)
#endif
	{
		if (wf_fwd_check_device_hook)
			wf_fwd_check_device_hook(wdev->if_dev, INT_MBSSID, pMbss->mbss_idx, wdev->channel, 1);
	}
#endif /* CONFIG_WIFI_PKT_FWD */
#ifdef RADIO_LINK_SELECTION
	if (pAd->ApCfg.RadioLinkSelection)
		Rls_SetInfInfo(pAd, TRUE, &pMbss->wdev);
#endif /* RADIO_LINK_SELECTION */


#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
        if (pAd->BgndScanCtrl.DfsZeroWaitSupport == 1 
            && pAd->BgndScanCtrl.DfsZeroWaitChannel !=0
            && CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC))
        {
#ifdef DFS_DBG_LOG_0
	    DfsZeroWaitStart(pAd, TRUE);
            MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]DfsZeroWaitSupport=%d, ZeroWaitCh=%d,DfsState=%d \n", 
                                                        __FUNCTION__,
                                                        pAd->BgndScanCtrl.DfsZeroWaitSupport,
                                                        pAd->BgndScanCtrl.DfsZeroWaitChannel,
                                                        GET_MT_ZEROWAIT_DFS_STATE(pAd)));
#endif	    
        }
        else
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */   
        {
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
                if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)&&
                CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INIT_CAC))
            {   /* update Dfs zero wait state from INIT_CAC to CAC */
                UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC);
#ifdef DFS_DBG_LOG_0		
                 MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Update CAC STATE from INIT_CAC to CAC\n", 
                                                        __FUNCTION__
                                                       ));
#endif		 
            }
            else
#endif  /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */ 
#ifdef MT_DFS_SUPPORT
            {
                if(pAd->Dot11_H.RDMode == RD_NORMAL_MODE)		    
		        {
                    DfsCacNormalStart(pAd);
		        }

  	        if(pMbss->wdev.channel > 14)
		        {	
                    WrapDfsRadarDetectStart(pAd);  
		        }
		/*DfsDedicatedScanStart(pAd);*/	
            }
#endif /* MT_DFS_SUPPORT */
        } 

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
    /* Update bInitMbssZeroWait for MBSS Zero Wait */
    UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, FALSE);
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */

#ifdef VOW_SUPPORT
    vow_mbss_init(pAd, wdev);
#endif /* VOW_SUPPORT */

#ifdef GREENAP_SUPPORT
    greenap_check_allow_status(pAd, greenap);
    if (greenap_get_capability(greenap) && greenap_get_allow_status(greenap)) {
        greenap_check_peer_connection_status(
            pAd, 
            DBDC_BAND0, 
            FALSE,
            greenap_get_allow_status(greenap));

        if (pAd->CommonCfg.dbdc_mode)
            greenap_check_peer_connection_status(
                pAd, 
                DBDC_BAND1, 
                FALSE,
                greenap_get_allow_status(greenap));        
    }
#endif /* GREENAP_SUPPORT */

#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
    if(pAd->ApCfg.BandSteering)
    {
        PBND_STRG_CLI_TABLE table;
        table = Get_BndStrgTable(pAd, pMbss->mbss_idx);
        if(table)
        {
            /* Inform daemon interface ready */
            BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, TRUE);
        }
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */

	if (wdev->bAllowBeaconing)
	{
		LeadTimeForBcn(pAd,wdev);
	}

}


VOID APStartUpByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	UINT32 idx;
	BSS_STRUCT *pMbss = NULL;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s() %d\n", __FUNCTION__, RfIC));

	/*don't gen beacon, reset tsf timer, don't gen interrupt.*/
	AsicDisableSync(pAd, HW_BSSID_0);

#ifdef DOT11_N_SUPPORT
	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);

#ifdef PIGGYBACK_SUPPORT
	AsicSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	/* Workaround start: Let Rx packet can be dequeued from PSE or Tx CMD will fail */
	/* Workaround end */

	/* setup tx preamble */
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);

	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

	/* default NO_PROTECTION */
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[idx];
		if(wmode_2_rfic(pMbss->wdev.PhyMode) != RfIC){
			continue;
		}		

		/* Update devinfo for any phymode change */
		WifiSysOpen(pAd, &pMbss->wdev);

		/* Update ERP */
		ApUpdateCapabilityAndErpIe(pAd,pMbss);
		pMbss->mbss_idx = idx;
		APStartUpForMbss(pAd, pMbss);
#ifdef DOT11_N_SUPPORT
		/* Update HT & GF Protect */
		APUpdateOperationMode(pAd, &pMbss->wdev);
#endif /* DOT11_N_SUPPORT */
	}

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP);
#endif /* LED_CONTROL_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	WifiFwdSet(pAd->CommonCfg.WfFwdDisabled);
#endif /* CONFIG_WIFI_PKT_FWD */

	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);



	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[idx];
		if(wmode_2_rfic(pMbss->wdev.PhyMode) != RfIC){
			continue;
		}
#ifdef MWDS
		if (pMbss->wdev.bDefaultMwdsStatus == TRUE)
			MWDSEnable(pAd,idx,TRUE,TRUE);
#endif
		
	/* start sending BEACON out */
		UpdateBeaconHandler(
		pAd,
		&pMbss->wdev,
		AP_RENEW);

		/*
			Set group re-key timer if necessary.
			It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
		*/
		APStartRekeyTimer(pAd,&pMbss->wdev);

#ifdef RADIO_LINK_SELECTION
	if (pAd->ApCfg.RadioLinkSelection)
		Rls_UpdateDevOpMode(pAd, TRUE, &pMbss->wdev);
#endif /* RADIO_LINK_SELECTION */			

	}

#ifdef DFS_SUPPORT
	if(RfIC & RFIC_5GHZ){
		UCHAR ch = HcGetChannelByRf(pAd,RfIC);
		if (IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE,ch))
			NewRadarDetectionStart(pAd);
	}
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);
#endif /* CARRIER_DETECTION_SUPPORT */

	if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
	{
		AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
	}

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);

#ifdef IDS_SUPPORT
	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable)
	{
#ifdef SYSTEM_LOG_SUPPORT
		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("!!! WARNING !!! The WirelessEvent parameter doesn't be enabled \n"));
#endif /* SYSTEM_LOG_SUPPORT */

		RTMPIdsStart(pAd);
	}
#endif /* IDS_SUPPORT */



#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */


	RTMP_ASIC_INTERRUPT_ENABLE(pAd);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].wdev.bssid)));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s()\n", __FUNCTION__));
}

// TODO: for run time usage should remove it
VOID APStartUp(RTMP_ADAPTER *pAd)
{
	if(pAd->chipCap.phy_caps & RFIC_5GHZ)
		APStartUpByRf(pAd,RFIC_5GHZ);
	if(pAd->chipCap.phy_caps & RFIC_24GHZ)
		APStartUpByRf(pAd,RFIC_24GHZ);
	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].wdev.bssid)));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s()\n", __FUNCTION__));
}


/*Only first time will run it*/
VOID APStartUpForMain(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss = NULL;
	UINT32 apidx = 0;

#if defined(INF_AMAZON_SE) || defined(RTMP_MAC_USB)
	UINT32 idx;
#endif /*INF_AMAZON_SE|| RTMP_MAC_USB*/

#ifdef DFS_SUPPORT
	UCHAR Channel5G = HcGetChannelByRf(pAd,RFIC_5GHZ);
#endif /*DFS_SUPPORT*/

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s()\n", __FUNCTION__));

#ifdef INF_AMAZON_SE
	for (idx=0;idx<NUM_OF_TX_RING;idx++)
	{
		pAd->BulkOutDataSizeLimit[idx]=24576;
	}
#endif /* INF_AMAZON_SE */

	AsicDisableSync(pAd, HW_BSSID_0);//don't gen beacon, reset tsf timer, don't gen interrupt.

	/*reset hw wtbl*/
	ap_hw_tb_init(pAd);

	pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	pMbss->mbss_idx = MAIN_MBSSID;

#ifdef RADIO_LINK_SELECTION
	/* It need to before APStartUpForMbss*/
	if (pAd->ApCfg.RadioLinkSelection)
		Rls_Init(pAd);
#endif	
	/*update main runtime attribute*/
	APStartUpForMbss(pAd, pMbss);

#ifdef DOT11_N_SUPPORT
	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);

#ifdef PIGGYBACK_SUPPORT
	AsicSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
		RtAsicFifoExtSet(pAd);
#endif /* FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	/* Workaround start: Let Rx packet can be dequeued from PSE or Tx CMD will fail */
	/* Workaround end */

	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);


	/* Set the RadarDetect Mode as Normal, bc the APUpdateAllBeaconFram() will refer this parameter. */
	//pAd->Dot11_H.RDMode = RD_NORMAL_MODE;

	/* Disable Protection first. */
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);
	ApUpdateCapabilityAndErpIe(pAd,pMbss);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd, &pMbss->wdev);
#endif /* DOT11_N_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_LINK_UP);
#endif /* LED_CONTROL_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	WifiFwdSet(pAd->CommonCfg.WfFwdDisabled);
#endif /* CONFIG_WIFI_PKT_FWD */

	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);


	/*
		NOTE!!!:
			All timer setting shall be set after following flag be cleared
				fRTMP_ADAPTER_HALT_IN_PROGRESS
	*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	APStartRekeyTimer(pAd,&pMbss->wdev);


	//RadarStateCheck(pAd);




	/* start sending BEACON out */
    	UpdateBeaconHandler(
        pAd,
        NULL,
        AP_RENEW);

#ifdef DFS_SUPPORT
	if (Channel5G > 0 && IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE,Channel5G))
	{
		NewRadarDetectionStart(pAd);
	}
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);
#endif /* CARRIER_DETECTION_SUPPORT */

	if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
	{
        	AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
	}

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[apidx];
		OPSTATUS_SET_FLAG_WDEV(&pMbss->wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	}

#ifdef IDS_SUPPORT
	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable)
	{
#ifdef SYSTEM_LOG_SUPPORT
		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("!!! WARNING !!! The WirelessEvent parameter doesn't be enabled \n"));
#endif /* SYSTEM_LOG_SUPPORT */

		RTMPIdsStart(pAd);
	}
#endif /* IDS_SUPPORT */



#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */


#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering)
		BndStrg_Init(pAd);
#endif /* BAND_STEERING */


	RTMP_ASIC_INTERRUPT_ENABLE(pAd);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].wdev.bssid)));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s()\n", __FUNCTION__));

}


/*
	==========================================================================
	Description:
		disassociate all STAs and stop AP service.
	Note:
	==========================================================================
 */
VOID APStopByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	BOOLEAN Cancelled;
	INT idx;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
#ifdef GREENAP_SUPPORT
    struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! APStopByRf %d !!!\n", RfIC));


	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
	{
		/* make sure CarrierDetect wont send CTS */
		CarrierDetectionStop(pAd);
	}
#endif /* CARRIER_DETECTION_SUPPORT */


#ifdef APCLI_SUPPORT
	{
#ifdef RADIO_LINK_SELECTION
	if (pAd->ApCfg.RadioLinkSelection == FALSE || 
		pAd->ApCfg.RlsTable.bTiggerByUser != TRUE)
#endif
	{
		for(idx=0;idx <MAX_APCLI_NUM;idx++){
			wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
			if(wmode_2_rfic(wdev->PhyMode)==RfIC){
				UINT8 enable = pAd->ApCfg.ApCliTab[idx].Enable;
				if (enable) {
					pAd->ApCfg.ApCliTab[idx].Enable = FALSE;
					ApCliIfDown(pAd);
					pAd->ApCfg.ApCliTab[idx].Enable = enable;
				}
			}
		}
	}
	}
#endif /* APCLI_SUPPORT */

	/*AP mode*/
	for(idx=0;idx < pAd->ApCfg.BssidNum;idx++){
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		if(wmode_2_rfic(wdev->PhyMode)==RfIC){
			MacTableResetWdev(pAd,wdev);
			OPSTATUS_CLEAR_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);			
#ifdef GREENAP_SUPPORT
                        if (greenap_get_capability(greenap) && greenap_get_allow_status(greenap))
                            greenap_exit(pAd, wdev, greenap);
#endif /* GREENAP_SUPPORT */
		}
	}
	CMDHandler(pAd);

	/* Disable pre-tbtt interrupt */
	AsicSetPreTbtt(pAd, FALSE, HW_BSSID_0);

	/* Disable piggyback */
	AsicSetPiggyBack(pAd, FALSE);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		AsicDisableSync(pAd, HW_BSSID_0);

#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif /* LED_CONTROL_SUPPORT */
	}




	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[idx];
		if(wmode_2_rfic(pMbss->wdev.PhyMode)!= RfIC){
			continue;
		}
#ifdef MWDS
		MWDSDisable(pAd, idx, TRUE, TRUE);
#endif /* MWDS */
#ifdef WH_EZ_SETUP
		if(IS_EZ_SETUP_ENABLED(&pMbss->wdev))
			ez_stop(&pMbss->wdev);
#endif /* WH_EZ_SETUP */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef RADIO_LINK_SELECTION
		if (pAd->ApCfg.RadioLinkSelection == FALSE || 
			pAd->ApCfg.RlsTable.bTiggerByUser != TRUE)
#endif
		{
			if (wf_fwd_entry_delete_hook)
				wf_fwd_entry_delete_hook (pAd->net_dev, pMbss->wdev.if_dev, 0);

			if (wf_fwd_check_device_hook)
				wf_fwd_check_device_hook(pMbss->wdev.if_dev, INT_MBSSID, pMbss->mbss_idx, pMbss->wdev.channel, 0);

		}
#endif /* CONFIG_WIFI_PKT_FWD */


#ifdef RADIO_LINK_SELECTION
		if (pAd->ApCfg.RadioLinkSelection)
			Rls_UpdateDevOpMode(pAd, FALSE, &pMbss->wdev);
#endif /* RADIO_LINK_SELECTION */

		/* clear protection to default */
		pMbss->wdev.protection = 0;
		WifiSysApLinkDown(pAd, &pMbss->wdev);

		/* Update devinfo for any phymode change */
		WifiSysClose(pAd, &pMbss->wdev);

		APReleaseRekeyTimer(pAd, &pMbss->wdev);
#ifdef BAND_STEERING
		if(pAd->ApCfg.BandSteering)
		{
			PBND_STRG_CLI_TABLE table;
			table = Get_BndStrgTable(pAd, idx);
			if(table)
			{
				/* Inform daemon interface ready */
				BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, FALSE);
			}
		}
#endif /* BAND_STEERING */
	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
		pAd->ApCfg.BANClass3Data = FALSE;
	}

	if (RfIC & RFIC_5GHZ){
#ifdef DFS_SUPPORT
		NewRadarDetectionStop(pAd);
#endif /* DFS_SUPPORT */
#ifdef MT_DFS_SUPPORT //Jelly20150217
		WrapDfsRadarDetectStop(pAd);
		/* Zero wait hand off recovery for CAC period + interface down case */
		DfsBFSoundingRecovery(pAd); 
#endif
	}

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	FT_Release(pAd);
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	DMSTable_Release(pAd);
#endif /* DOT11V_WNM_SUPPORT */


#ifdef VOW_SUPPORT
	vow_reset(pAd);
#endif /* VOW_SUPPORT */
}

VOID APStop(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! APStop !!!\n"));

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);

	if(HcIsRfSupport(pAd,RFIC_5GHZ))
		APStopByRf(pAd,RFIC_5GHZ);
	if(HcIsRfSupport(pAd,RFIC_24GHZ))
		APStopByRf(pAd,RFIC_24GHZ);

	MacTableReset(pAd);
	CMDHandler(pAd);

}

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID APCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue)
{
	PQUEUE_ENTRY pEntry;
	PNDIS_PACKET pPacket;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): (0x%08lx)...\n", __FUNCTION__, (ULONG)pQueue));

	while (pQueue->Head)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%u...\n", __FUNCTION__, pQueue->Number));

		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}

static VOID avg_pkt_len_reset(struct _RTMP_ADAPTER *ad)
{
	ad->mcli_ctl.pkt_avg_len = 0;
	ad->mcli_ctl.sta_nums = 0;
}

static VOID avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = entry->wdev->sys_handle;
	UINT32 avg_pkt_len = 0 ;
	struct multi_cli_ctl *mctrl = &ad->mcli_ctl;

	if (!ad->CommonCfg.dbdc_mode)
		return;

	if (entry->avg_tx_pkts > 0)
		avg_pkt_len = (UINT32)(entry->AvgTxBytes/entry->avg_tx_pkts);

	if ((avg_pkt_len > VERIWAVE_INVALID_PKT_LEN_HIGH) ||
		(avg_pkt_len < VERIWAVE_INVALID_PKT_LEN_LOW))
		return;
	/*moving average for pkt avg length*/
	mctrl->pkt_avg_len  =
		((mctrl->pkt_avg_len*mctrl->sta_nums)+avg_pkt_len)/(mctrl->sta_nums+1);
	mctrl->sta_nums++;
}

#define FAR_CLIENT_RSSI 	(-70)
#define FAR_CLIENT_DELTA_RSSI	(10)
static VOID CalFarClientNum(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
	UINT8 idx;
	CHAR avg_rssi[4];

	for (idx = 0;idx < 4; idx++) 
		avg_rssi[idx] = ad->ApCfg.RssiSample.LastRssi[idx] - ad->BbpRssiToDbmDelta;

	for (idx = 0;idx < 4; idx++) {
		if ((entry->RssiSample.AvgRssi[idx] < FAR_CLIENT_RSSI) &&
			(entry->RssiSample.AvgRssi[idx] != -127)) {
			if ((avg_rssi[idx] - entry->RssiSample.AvgRssi[idx]) >= FAR_CLIENT_DELTA_RSSI) {
				ad->nearfar_far_client_num++;
				break;
			}
		}
	}
}
static VOID dynamic_amsdu_protect_adjust(struct _RTMP_ADAPTER *ad)
{
	UCHAR amsdu_cnt = 4;
	ULONG per = 0;
	ULONG tx_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;
	BOOLEAN mc_flg = FALSE;
	/*concurrent case, not adjust*/
	if ((ad->txop_ctl.multi_client_nums > 0) &&
		(ad->txop_ctl.multi_client_nums_2g > 0))
		return;

	/*adjust amsdu*/
	if (ad->txop_ctl.multi_client_nums == MULTI_CLIENT_NUMS_TH) {
		if (ad->mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
			amsdu_cnt = 2;

	} else {
		if (ad->txop_ctl.multi_client_nums > 1) {
			if (ad->mcli_ctl.pkt_avg_len  > VERIWAVE_TP_AMSDU_DIS_TH)
				amsdu_cnt = 1;
			else if (ad->mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
				amsdu_cnt = 2;
		} else if (ad->txop_ctl.multi_client_nums_2g > 1) {
			/*2G case*/
			if (ad->mcli_ctl.pkt_avg_len  > VERIWAVE_2G_TP_AMSDU_DIS_TH)
					amsdu_cnt = 1;
				else if (ad->mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
					amsdu_cnt = 2;
		}
	}

	if ((ad->txop_ctl.multi_client_nums == MULTI_CLIENT_NUMS_TH) || 
			(ad->txop_ctl.multi_client_nums_2g == MULTI_CLIENT_2G_NUMS_TH)) 
		mc_flg = TRUE;

	if (ad->mcli_ctl.amsdu_cnt != amsdu_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): change amsdu %d to %d\n",
			__func__,ad->mcli_ctl.amsdu_cnt,amsdu_cnt));
		ad->mcli_ctl.amsdu_cnt = amsdu_cnt;
#ifdef VOW_SUPPORT
        	/* adj airtime quantum only when WATF is not enabled */        
        	if (ad->vow_cfg.en_airtime_fairness && ad->vow_sta_frr_quantum && !vow_watf_is_enabled(ad)) {
		        if ((amsdu_cnt != 4) && (ad->nearfar_far_client_num <= 1) && mc_flg)
		        	RTMP_SET_STA_DWRR_QUANTUM(ad, FALSE, ad->vow_sta_frr_quantum);/* fast round robin */
			else
				RTMP_SET_STA_DWRR_QUANTUM(ad, TRUE, 0);/* restore */
		}
#endif /* VOW_SUPPORT */

		amsdu_cnt = 4;
		MtCmdCr4Set(ad, CR4_SET_ID_CONFIG_STA_AMSDU_MAX_NUM, 255, amsdu_cnt);
	}
	/* reset */
	ad->nearfar_far_client_num = 0;
	/*adjust protection*/
	if (ad->txop_ctl.multi_client_nums > 0) {
		wlan_ct = &ad->WlanCounters[1];
		wdev = ad->txop_ctl.cur_wdev;
	} else {
		wlan_ct = &ad->WlanCounters[0];
		wdev = ad->txop_ctl.cur_wdev_2g;
	}

	tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;
	if (tx_cnt > 0) {
		per = 100*(wlan_ct->AmpduFailCount.u.LowPart)/(wlan_ct->AmpduFailCount.u.LowPart+tx_cnt);
		if ((ad->mcli_ctl.c2s_only == FALSE) && (per < VERIWAVE_PER_RTS_DIS_TH)) {
			AsicRtsOnOff(wdev,FALSE);
			ad->mcli_ctl.c2s_only = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): disable RTS, per=%lu\n",
			__func__,per));
		}else if ((ad->mcli_ctl.c2s_only == TRUE) && (per >= VERIWAVE_PER_RTS_DIS_TH)) {
			AsicRtsOnOff(wdev,TRUE);
			ad->mcli_ctl.c2s_only = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): enable RTS, per=%lu\n",
			__func__,per));
		}
	}
}

#ifdef STA_FORCE_ROAM_SUPPORT
void load_froam_defaults(RTMP_ADAPTER *pAd)
{
	pAd->en_force_roam_supp = FROAM_SUPP_DEF;
	pAd->sta_low_rssi = (-1) * STA_LOW_RSSI;
	pAd->low_sta_renotify = LOW_RSSI_STA_RENOTIFY_TIME;
	pAd->sta_age_time = STALIST_AGEOUT_TIME;
	pAd->mntr_age_time = MNTRLIST_AGEOUT_TIME;
	pAd->mntr_min_pkt_count = MNTR_MIN_PKT_COUNT;
	pAd->mntr_min_time = MNTR_MIN_TIME;
	pAd->mntr_avg_rssi_pkt_count = AVG_RSSI_PKT_COUNT;
	pAd->sta_good_rssi = (-1) * STA_DETECT_RSSI;
	pAd->acl_age_time = ACLLIST_AGEOUT_TIME;
	pAd->acl_hold_time = ACLLIST_HOLD_TIME;	

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("\n[Force Roam] => Force Roam Support = %d\n",pAd->en_force_roam_supp));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[Force Roam] => StaLowRssiThr=%d dBm low_sta_renotify=%d sec StaAgeTime=%d sec\n",pAd->sta_low_rssi, pAd->low_sta_renotify,pAd->sta_age_time)); 	
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[Force Roam] => MntrAgeTime=%d sec mntr_min_pkt_count=%d mntr_min_time=%d sec mntr_avg_rssi_pkt_count=%d\n",
		pAd->mntr_age_time, pAd->mntr_min_pkt_count,pAd->mntr_min_time, pAd->mntr_avg_rssi_pkt_count));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[Force Roam] => AclAgeTime=%d sec AclHoldTime=%d sec\n",pAd->acl_age_time, pAd->acl_hold_time));

}

static CHAR staMaxRssi(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RSSI_SAMPLE *pRssi)
{
	CHAR Rssi = MINIMUM_POWER_VALUE;
	UINT32	rx_stream;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			rx_stream = pAd->dbdc_2G_rx_stream;
		else
			rx_stream = pAd->dbdc_5G_rx_stream;
	} else {
		rx_stream = pAd->Antenna.field.RxPath;
	}

	if ((rx_stream == 1) && (pRssi->AvgRssi[0] < 0))
	{
		Rssi = pRssi->AvgRssi[0];
	}

	if ((rx_stream >= 2) && (pRssi->AvgRssi[1] < 0))
	{
		Rssi = max(pRssi->AvgRssi[0], pRssi->AvgRssi[1]);
	}

	if ((rx_stream >= 3) && (pRssi->AvgRssi[2] < 0))
	{
		Rssi = max(Rssi, pRssi->AvgRssi[2]);
	}

	if ((rx_stream == 4) && (pRssi->AvgRssi[3] < 0))
	{
		Rssi = max(Rssi, pRssi->AvgRssi[3]);
	}

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
	//	("        rx_stream:%d AvgRssi[0]:%d AvgRssi[1]:%d AvgRssi[2]:%d AvgRssi[3]:%d MaxRssi:%d !!\n",rx_stream,
	//	pRssi->AvgRssi[0],pRssi->AvgRssi[1],pRssi->AvgRssi[2],pRssi->AvgRssi[3],Rssi));

	return Rssi;
}

static void sta_rssi_check(void *ad_obj, void *pEntry)
{
	CHAR maxRssi = MINIMUM_POWER_VALUE;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ad_obj;
	MAC_TABLE_ENTRY *pMacEntry = (MAC_TABLE_ENTRY *)pEntry;

	// average value of rssi for all antenna is not being considered, as it depends on whether all antenna slots as usage
	// have got antenna connected.
	// Instead Max value would be used

	maxRssi = staMaxRssi(pAd, pMacEntry->wdev, &pMacEntry->RssiSample);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
		("STA %02x-%02x-%02x-%02x-%02x-%02x maxRssi:%d !!\n",
		pMacEntry->Addr[0],pMacEntry->Addr[1],pMacEntry->Addr[2],pMacEntry->Addr[3],pMacEntry->Addr[4],pMacEntry->Addr[5],maxRssi));

	if(pMacEntry->low_rssi_notified)	// i.e rssi improved
	{
		if(maxRssi > pAd->sta_low_rssi ){
			froam_event_sta_good_rssi event_data;
			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("issue FROAM_EVT_STA_RSSI_GOOD -> for STA %02x-%02x-%02x-%02x-%02x-%02x\n",
			    pMacEntry->Addr[0],pMacEntry->Addr[1],pMacEntry->Addr[2],pMacEntry->Addr[3],pMacEntry->Addr[4],pMacEntry->Addr[5]));
			
			memset(&event_data,0,sizeof(event_data));

			event_data.hdr.event_id = FROAM_EVT_STA_RSSI_GOOD;	
			event_data.hdr.event_len = sizeof(froam_event_sta_good_rssi) - sizeof(froam_event_hdr);

			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("FROAM_EVT_STA_RSSI_GOOD payload len:%d datLen: %d-> \n",
			//	event_data.hdr.event_len,sizeof(event_data)));
			
			memcpy(event_data.mac,pMacEntry->Addr,MAC_ADDR_LEN);
			
			RtmpOSWrielessEventSend(
						pAd->net_dev,
						RT_WLAN_EVENT_CUSTOM,
						OID_FROAM_EVENT,
						NULL,
						(UCHAR *) &event_data,
						sizeof(event_data));
			
			pMacEntry->low_rssi_notified = FALSE;
			pMacEntry->tick_sec = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Froam event sent <- \n"));
		}
		else{
			pMacEntry->tick_sec++;
			if(pMacEntry->tick_sec >= pAd->low_sta_renotify){
				pMacEntry->tick_sec = 0;
				pMacEntry->low_rssi_notified = FALSE;
			}
		}
	}
	else if((maxRssi != MINIMUM_POWER_VALUE) && (maxRssi < pAd->sta_low_rssi ))	//pAd->ApCfg.EventNotifyCfg.StaRssiDetectThreshold))
	{
		froam_event_sta_low_rssi event_data;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("issue FROAM_EVT_STA_RSSI_LOW -> for STA %02x-%02x-%02x-%02x-%02x-%02x\n",
		    pMacEntry->Addr[0],pMacEntry->Addr[1],pMacEntry->Addr[2],pMacEntry->Addr[3],pMacEntry->Addr[4],pMacEntry->Addr[5]));
		
		memset(&event_data,0,sizeof(event_data));

		event_data.hdr.event_id = FROAM_EVT_STA_RSSI_LOW;	
		event_data.hdr.event_len = sizeof(froam_event_sta_low_rssi) - sizeof(froam_event_hdr);

		event_data.channel = pMacEntry->wdev->channel;
		memcpy(event_data.mac,pMacEntry->Addr,MAC_ADDR_LEN);

		RtmpOSWrielessEventSend(
					pAd->net_dev,
					RT_WLAN_EVENT_CUSTOM,
					OID_FROAM_EVENT,
					NULL,
					(UCHAR *) &event_data,
					sizeof(event_data));

		pMacEntry->low_rssi_notified = TRUE;
		pMacEntry->tick_sec = 0;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Froam event sent <- \n"));
	}

}
#endif

/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/
VOID MacTableMaintenance(RTMP_ADAPTER *pAd)
{
	int wcid, startWcid, i;
#ifdef DOT11_N_SUPPORT
	ULONG MinimumAMPDUSize = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor; /*Default set minimum AMPDU Size to 2, i.e. 32K */
	BOOLEAN	bRdgActive = FALSE;
	BOOLEAN bRalinkBurstMode;
#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags;
#endif /* RTMP_MAC_PCI */
	MAC_TABLE *pMacTable;
#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
	int lastClient=0;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */
	CHAR avgRssi;
	BSS_STRUCT *pMbss;
#ifdef WFA_VHT_PF
	RSSI_SAMPLE *worst_rssi = NULL;
	int worst_rssi_sta_idx = 0;
#endif /* WFA_VHT_PF */
#ifdef MT_MAC
	BOOLEAN bPreAnyStationInPsm = FALSE;
#endif /* MT_MAC */
#ifdef SMART_CARRIER_SENSE_SUPPORT
        UINT    BandIdx=0;
	CHAR	tmpRssi=0;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef APCLI_SUPPORT
	ULONG	apcli_avg_tx = 0;
	ULONG	apcli_avg_rx = 0;
	struct wifi_dev * apcli_wdev = NULL;
#endif /* APCLI_SUPPORT */
	struct wifi_dev * sta_wdev = NULL;
	struct wifi_dev * txop_wdev = NULL; 
	struct wifi_dev *wdev = NULL;
	UCHAR    sta_hit_2g_infra_case_number =0;
#if defined(MWDS) && defined(IGMP_SNOOP_SUPPORT)
	BOOLEAN IgmpQuerySendTickChanged = FALSE;
	BOOLEAN MldQuerySendTickChanged = FALSE;
#endif
	pMacTable = &pAd->MacTab;

#ifdef MT_MAC
	bPreAnyStationInPsm = pMacTable->fAnyStationInPsm;
#endif /* MT_MAC */

	pMacTable->fAnyStationInPsm = FALSE;
	pMacTable->fAnyStationBadAtheros = FALSE;
	pMacTable->fAnyTxOPForceDisable = FALSE;
	pMacTable->fAllStationAsRalink[0] = TRUE;
	pMacTable->fAllStationAsRalink[1] = TRUE;
	pMacTable->fCurrentStaBw40 = FALSE;
#ifdef DOT11_N_SUPPORT
	pMacTable->fAnyStation20Only = FALSE;
	pMacTable->fAnyStationIsLegacy = FALSE;
	pMacTable->fAnyStationMIMOPSDynamic = FALSE;

#ifdef DOT11N_DRAFT3
	pMacTable->fAnyStaFortyIntolerant = FALSE;
#endif /* DOT11N_DRAFT3 */
	pMacTable->fAllStationGainGoodMCS = TRUE;
#endif /* DOT11_N_SUPPORT */

#ifdef WAPI_SUPPORT
	pMacTable->fAnyWapiStation = FALSE;
#endif /* WAPI_SUPPORT */

	startWcid = 1;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Skip the Infra Side */
	startWcid = 2;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef SMART_CARRIER_SENSE_SUPPORT
        for (BandIdx=0; BandIdx< DBDC_BAND_NUM; BandIdx++)
        {
	    pAd->SCSCtrl.SCSMinRssi[BandIdx] =  0; /* (Reset)The minimum RSSI of STA */
	    pAd->SCSCtrl.OneSecRxByteCount[BandIdx] = 0;
	    pAd->SCSCtrl.OneSecTxByteCount[BandIdx] = 0;
   
        }
#endif /* SMART_CARRIER_SENSE_SUPPORT */

	/* step1. scan all wdevs, clean all wdev non_gf_sta counter */
	for (i=0; i<WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];
		if (wdev != NULL) {
			wlan_operate_set_non_gf_sta(wdev,0);
		}
	}

	for (i=0; i<BAND_NUM_MAX; i++) {
		pMacTable->fAnyStationNonGF[i] = FALSE;
	}

	avg_pkt_len_reset(pAd);

    /*TODO: Carter, modification start Wcid, Aid shall not simply equal to WCID.*/
	for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++)
	{
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[wcid];
		STA_TR_ENTRY *tr_entry = &pMacTable->tr_entry[wcid];
		BOOLEAN bDisconnectSta = FALSE;

#ifdef HTC_DECRYPT_IOT
		if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)))
		{
			if(pEntry->HTC_AAD_OM_CountDown > 0) //count down to start all new pEntry->HTC_ICVErrCnt
			{
				pEntry->HTC_AAD_OM_CountDown--;
			}
		}
#endif /* HTC_DECRYPT_IOT */

#ifdef SMART_CARRIER_SENSE_SUPPORT
		if (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			BandIdx = HcGetBandByWdev(pEntry->wdev);

			pAd->SCSCtrl.OneSecRxByteCount[BandIdx] += pEntry->OneSecRxBytes;
			pAd->SCSCtrl.OneSecTxByteCount[BandIdx] += pEntry->OneSecTxBytes;
			if (pAd->SCSCtrl.SCSEnable[BandIdx] == SCS_ENABLE) {
				tmpRssi = RTMPMinRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1], 
	                                                pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);
				if (tmpRssi <pAd->SCSCtrl.SCSMinRssi[BandIdx] )
					pAd->SCSCtrl.SCSMinRssi[BandIdx] = tmpRssi;
			}
		}	
#endif /* SMART_CARRIER_SENSE_SUPPORT */	


#ifdef RACTRL_LIMIT_MAX_PHY_RATE
#ifdef DOT11_VHT_AC
        if ( pAd->MacTab.Size == 0 )
        {
            pAd->fgRaLimitPhyRate = FALSE;
        }
        else if ((pAd->fgRaLimitPhyRate == FALSE) && !IS_ENTRY_NONE(pEntry))
        {
            UINT16 u2TxTP;
            BOOLEAN fgPhyModeCheck = FALSE;
            u2TxTP = pEntry->OneSecTxBytes >> BYTES_PER_SEC_TO_MBPS;

            if (pEntry->SupportRateMode & SUPPORT_VHT_MODE)
            {
                if (pEntry->MaxHTPhyMode.field.BW == BW_160)
                {
                    fgPhyModeCheck = TRUE;
                }
                if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_3SS)
                {
                    if (pEntry->SupportVHTMCS4SS)
                    {
                        fgPhyModeCheck = TRUE;
                    }
                }
                else if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_2SS)
                {
                    if (pEntry->SupportVHTMCS3SS)
                    {
                        fgPhyModeCheck = TRUE;
                    }
                }
                else
                {
                    fgPhyModeCheck = TRUE;
                }
            }

            if ((u2TxTP > LIMIT_MAX_PHY_RATE_THRESHOLD) && fgPhyModeCheck)
            {
                MtCmdSetMaxPhyRate(pAd, RACTRL_LIMIT_MAX_PHY_RATE);
                pAd->fgRaLimitPhyRate = TRUE;
            }
        }
#endif /* DOT11_VHT_AC */
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */



#ifdef APCLI_SUPPORT
		if(IS_ENTRY_APCLI(pEntry) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
		{
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];

			pApCliEntry->OneSecTxBytes = pEntry->OneSecTxBytes;
			pApCliEntry->OneSecRxBytes = pEntry->OneSecRxBytes;

#ifdef APCLI_CERT_SUPPORT
			if (pApCliEntry->NeedFallback == TRUE)
			{
				pApCliEntry->NeedFallback = FALSE;

				if (pAd->bApCliCertTest == TRUE)
				{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					if (pAd->chipCap.fgRateAdaptFWOffload == TRUE)
					{
						CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

						NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
						rRaParam.u4Field = RA_PARAM_HT_2040_COEX;
						RAParamUpdate(pAd, pEntry, &rRaParam);
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("FallBack APClient BW to 20MHz\n"));
					}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
				}
			}
#endif /* APCLI_CERT_SUPPORT */
		}
#endif /* APCLI_SUPPORT */

		pEntry->AvgTxBytes = (pEntry->AvgTxBytes == 0) ? \
							pEntry->OneSecTxBytes : \
							((pEntry->AvgTxBytes + pEntry->OneSecTxBytes) >> 1);
		pEntry->OneSecTxBytes = 0;

		pEntry->AvgRxBytes = (pEntry->AvgRxBytes == 0) ? \
							pEntry->OneSecRxBytes : \
							((pEntry->AvgRxBytes + pEntry->OneSecRxBytes) >> 1);
		pEntry->OneSecRxBytes = 0;

		pEntry->avg_tx_pkts = (pEntry->avg_tx_pkts == 0) ? \
							pEntry->one_sec_tx_pkts: \
							((pEntry->avg_tx_pkts + pEntry->one_sec_tx_pkts) >> 1);

		pEntry->one_sec_tx_pkts = 0;
		
#ifdef APCLI_SUPPORT
	if((IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
            && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
        )
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pEntry->bReptCli)
			{
				pEntry->ReptCliIdleCount++;

				if ((pEntry->bReptEthCli) 
					&& (pEntry->ReptCliIdleCount >= MAC_TABLE_AGEOUT_TIME)
					&& (pEntry->bReptEthBridgeCli == FALSE)) /* Do NOT ageout br0 link. @2016/1/27 */
				{
					REPEATER_CLIENT_ENTRY *pReptCliEntry = NULL;
					pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[pEntry->MatchReptCliIdx];
					if (pReptCliEntry) {
						pReptCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA;
					}
					MlmeEnqueue(pAd,
								APCLI_CTRL_STATE_MACHINE,
 								APCLI_CTRL_DISCONNECT_REQ,
								0,
								NULL,
								(REPT_MLME_START_IDX + pEntry->MatchReptCliIdx));
                    RTMP_MLME_HANDLER(pAd);
					continue;
				}
			}
#endif /* MAC_REPEATER_SUPPORT */

			if (IS_ENTRY_APCLI(pEntry))
				apcli_wdev = pEntry->wdev;

			apcli_avg_tx += pEntry->AvgTxBytes;
			apcli_avg_rx += pEntry->AvgRxBytes;
			

			if (((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
#ifdef CONFIG_MULTI_CHANNEL
				&& (pAd->Mlme.bStartMcc == FALSE)
#endif /* CONFIG_MULTI_CHANNEL */
			)
			{
				/* use Null or QoS Null to detect the ACTIVE station*/
				BOOLEAN ApclibQosNull = FALSE;

				if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
					ApclibQosNull = TRUE;
#ifdef WH_EZ_SETUP
				if (IS_ADPTR_EZ_SETUP_ENABLED(pAd) && (ScanRunning(pAd) == FALSE)){
#endif
					ApCliRTMPSendNullFrame(pAd,pEntry->CurrTxRate, ApclibQosNull, pEntry, PWR_ACTIVE);
#ifdef WH_EZ_SETUP
				}
#endif
				continue;
			}
		}
#endif /* APCLI_SUPPORT */

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;
                                if (pEntry ->fgGband256QAMSupport && (pEntry->RXBAbitmap != 0) && (pEntry->TXBAbitmap != 0) && sta_hit_2g_infra_case_number <= STA_NUMBER_FOR_TRIGGER) {
                                        sta_wdev = pEntry->wdev;
                                        if (WMODE_CAP_2G(sta_wdev->PhyMode)) {
                                               UINT tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
                                               UINT rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS); 
				
                                                if (tx_tp > INFRA_TP_PEEK_BOUND_THRESHOLD && 
                                                        (tx_tp *100)/(tx_tp+rx_tp) >TX_MODE_RATIO_THRESHOLD){
                                                        if (sta_hit_2g_infra_case_number < STA_NUMBER_FOR_TRIGGER) {
                                                                txop_wdev = sta_wdev;
                                                                sta_hit_2g_infra_case_number ++;
                                                        } else 
                                                                sta_hit_2g_infra_case_number ++;
                                                }

                                        }
                                                
                                }                


			if (pEntry->wdev) {
				ULONG avg_tx_b = pEntry->AvgTxBytes;
				ULONG avg_rx_b = pEntry->AvgRxBytes;

#ifdef CONFIG_TX_DELAY
				if ((rx_tp >= 150) && (rx_tp <= 1000))
					pAd->que_agg_en = TRUE;
				else
					pAd->que_agg_en = FALSE;
#endif
					
				if (WMODE_CAP_5G(pEntry->wdev->PhyMode) && (pAd->txop_ctl.multi_client_nums < MULTI_CLIENT_NUMS_TH)) {
					pAd->txop_ctl.cur_wdev = pEntry->wdev;

					if ((avg_tx_b + avg_rx_b) != 0) {
						if ((pEntry->avg_tx_pkts > VERIWAVE_5G_PKT_CNT_TH) &&
						    (((avg_tx_b * 100) / (avg_tx_b + avg_rx_b)) > TX_MODE_RATIO_THRESHOLD))
							pAd->txop_ctl.multi_client_nums++;
					}
				}

				if (WMODE_CAP_2G(pEntry->wdev->PhyMode) && (pAd->txop_ctl.multi_client_nums_2g < MULTI_CLIENT_2G_NUMS_TH)) {
					pAd->txop_ctl.cur_wdev_2g = pEntry->wdev;

					if ((avg_tx_b + avg_rx_b) != 0) {
						if ((pEntry->avg_tx_pkts > VERIWAVE_2G_PKT_CNT_TH) &&
						    (((avg_tx_b * 100) / (avg_tx_b + avg_rx_b)) > TX_MODE_RATIO_THRESHOLD))
							pAd->txop_ctl.multi_client_nums_2g++;
					}
				}

				if (pEntry->avg_tx_pkts > INFRA_KEEP_STA_PKT_TH)
					pEntry->NoDataIdleCount = 0;

				avg_pkt_len_calculate(pEntry);
				CalFarClientNum(pAd, pEntry);
			}

			/* dynamic txop for peak TP */
			if (pEntry->wdev) {
				UINT32 tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT32 rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);

				if (WMODE_CAP_5G(pEntry->wdev->PhyMode)) {
					if (pAd->peak_tp_ctl.cur_wdev == NULL)
						pAd->peak_tp_ctl.cur_wdev = pEntry->wdev;
					pAd->peak_tp_ctl.client_nums++;
					if ((tx_tp + rx_tp) != 0) 
					{
						if(((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD)
						{//the entry is in tx mode
							if ((tx_tp > pAd->peak_tp_ctl.max_tx_tp)) {
								pAd->peak_tp_ctl.max_tx_tp = tx_tp;
								pAd->peak_tp_ctl.cur_wdev = pEntry->wdev;
							}
						}
					}
				}

				if (WMODE_CAP_2G(pEntry->wdev->PhyMode)) {
					if (pAd->peak_tp_ctl.cur_wdev_2g == NULL)
						pAd->peak_tp_ctl.cur_wdev_2g = pEntry->wdev;
					pAd->peak_tp_ctl.client_nums_2g++;
					if ((tx_tp + rx_tp) != 0) 
					{
						if(((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD)
						{//the entry is in tx mode
							if ((tx_tp > pAd->peak_tp_ctl.max_tx_2g_tp)) {
								pAd->peak_tp_ctl.max_tx_2g_tp = tx_tp;
								pAd->peak_tp_ctl.cur_wdev_2g = pEntry->wdev;
							}
						}
					}
				}

				if ((tx_tp + rx_tp) != 0) {
					struct rx_delay_control *rx_delay_ctl = &pAd->tr_ctl.rx_delay_ctl;
					UINT32 i;
					struct rx_dly_ctl_cfg *cfg;
					UINT32 reg_val = RX_DLY_INT_CFG;

					if (rx_delay_ctl->en) {
						if (((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD) {
							for (i = 0; i < rx_delay_ctl->dl_rx_dly_ctl_tbl_size; i++) {
								cfg = rx_delay_ctl->dl_rx_dly_ctl_tbl + i;
								if (tx_tp > cfg->avg_tp) {
									reg_val = cfg->rx_dly_cfg;
									continue;
								} else {
									break;
								}
							}
						}

						if (((rx_tp * 100) / (tx_tp + rx_tp)) > RX_MODE_RATIO_THRESHOLD) {
							for (i = 0; i < rx_delay_ctl->ul_rx_dly_ctl_tbl_size; i++) {
								cfg = rx_delay_ctl->ul_rx_dly_ctl_tbl + i;
								if (rx_tp > cfg->avg_tp) {
									reg_val = cfg->rx_dly_cfg;
									continue;
								} else {
									break;
								}
							}
						}

						HIF_IO_WRITE32(pAd, MT_DELAY_INT_CFG, reg_val);						
					}
				}
			}
#ifdef MT_PS
		CheckSkipTX(pAd, pEntry);
#endif /* MT_PS */

		if (pEntry->NoDataIdleCount == 0)
			pEntry->StationKeepAliveCount = 0;

		pEntry->NoDataIdleCount ++;
		// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
		pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;

		pEntry->StaConnectTime ++;

		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine))
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%02x:%02x:%02x:%02x:%02x:%02x fail to complete ASSOC in %d sec\n",
					PRINT_MAC(pEntry->Addr), MAC_TABLE_ASSOC_TIMEOUT));
#ifdef WSC_AP_SUPPORT
			if (NdisEqualMemory(pEntry->Addr, pMbss->WscControl.EntryAddr, MAC_ADDR_LEN))
				NdisZeroMemory(pMbss->WscControl.EntryAddr, MAC_ADDR_LEN);
#endif /* WSC_AP_SUPPORT */
#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(pEntry->wdev))
			{
#ifdef EZ_MOD_SUPPORT
				void* ez_peer = NULL;
#else
				struct _ez_peer_security_info * ez_peer = NULL;
#endif
				ez_peer = ez_peer_table_search_by_addr(pEntry->wdev,pEntry->Addr);
				if (ez_peer)
				{
#ifdef EZ_MOD_SUPPORT				
					ez_set_delete_peer_in_differed_context(pEntry->wdev, ez_peer, TRUE);
#else
					ez_peer->delete_in_differred_context = TRUE;
#endif
					ez_send_unicast_deauth(pAd,pEntry->Addr);
				} else {
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				}
			}
			else {
#endif
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
#ifdef WH_EZ_SETUP
			}
#endif
			continue;
		}

		/*
			1. check if there's any associated STA in power-save mode. this affects outgoing
				MCAST/BCAST frames should be stored in PSQ till DtimCount=0
		*/
		if (pEntry->PsMode == PWR_SAVE) {
			pMacTable->fAnyStationInPsm = TRUE;
			if (pEntry->wdev &&
					(pEntry->wdev->wdev_type == WDEV_TYPE_AP || pEntry->wdev->wdev_type == WDEV_TYPE_GO)) {
				// TODO: it looks like pEntry->wdev->tr_tb_idx is not assigned?
				pAd->MacTab.tr_entry[pEntry->wdev->tr_tb_idx].PsMode = PWR_SAVE;
				if (tr_entry->PsDeQWaitCnt)
				{
					tr_entry->PsDeQWaitCnt++;
					if (tr_entry->PsDeQWaitCnt > 2)
						tr_entry->PsDeQWaitCnt = 0;
				}
			}
		}

#ifdef DOT11_N_SUPPORT
		if (pEntry->MmpsMode == MMPS_DYNAMIC)
			pMacTable->fAnyStationMIMOPSDynamic = TRUE;

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = TRUE;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD) {
			if (pEntry->wdev) {
				UCHAR band = HcGetBandByWdev(pEntry->wdev);
				UINT16 non_gf_sta = wlan_operate_get_non_gf_sta(wdev);
				non_gf_sta++;				
				wlan_operate_set_non_gf_sta(wdev,non_gf_sta);
				pMacTable->fAnyStationNonGF[band] = TRUE;
			}
		}

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
			pMacTable->fAnyStationIsLegacy = TRUE;

#ifdef DOT11N_DRAFT3
		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = TRUE;
#endif /* DOT11N_DRAFT3 */

		/* Get minimum AMPDU size from STA */
		if (MinimumAMPDUSize > pEntry->MaxRAmpduFactor)
			MinimumAMPDUSize = pEntry->MaxRAmpduFactor;
#endif /* DOT11_N_SUPPORT */

#if defined(RTMP_MAC) || defined(RLT_MAC)
        if (pAd->chipCap.hif_type == HIF_RTMP
                || pAd->chipCap.hif_type == HIF_RLT)
        {
            if (pEntry->bIAmBadAtheros)
            {
                pMacTable->fAnyStationBadAtheros = TRUE;
#ifdef DOT11_N_SUPPORT
                if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE) {
			if (pEntry->wdev) {
				UCHAR band = HcGetBandByWdev(pEntry->wdev);

				AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT,
						FALSE, pMacTable->fAnyStationNonGF[band]);
			}
		}
#endif /* DOT11_N_SUPPORT */
            }
        }
#endif

        /* detect the station alive status */
		/* detect the station alive status */
		if ((pMbss->StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pMbss->StationKeepAliveTime))
		{
			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

		        ......
		        sam> tx ok!
		        sam> count = 1!	 ==> 1 second after the Null Frame is acked
		        sam> count = 2!	 ==> 2 second after the Null Frame is acked
		        sam> count = 3!
		        sam> count = 4!
		        sam> count = 5!
		        sam> count = 6!
		        sam> count = 7!
		        sam> count = 8!
		        sam> count = 9!
		        sam> count = 10!
		        sam> count = 11!
		        sam> count = 12!
		        sam> count = 13!
		        sam> count = 14!
		        sam> count = 15! ==> 15 second after the Null Frame is acked
		        sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
		        sam> count = 1!  ==> 1 second after the Null Frame is acked
		        sam> count = 2!
		        sam> count = 3!
		        sam> count = 4!
		        ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we do not support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/

			if (pEntry->StationKeepAliveCount++ == 0)
			{
					if (pEntry->PsMode == PWR_SAVE)
					{
						/* use TIM bit to detect the PS station */
						WLAN_MR_TIM_BIT_SET(pAd, pEntry->func_tb_idx, pEntry->Aid);
					}
					else
					{
						/* use Null or QoS Null to detect the ACTIVE station */
						BOOLEAN bQosNull = FALSE;

						if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
							bQosNull = TRUE;

						RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
	    	                           						pEntry->Aid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
					}
			}
			else
			{
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime)
					pEntry->StationKeepAliveCount = 0;
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */
		if (pEntry->NoDataIdleCount >= pEntry->StaIdleTimeout)
		{
			bDisconnectSta = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
					PRINT_MAC(pEntry->Addr), pEntry->StaIdleTimeout));
			ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
		}
		else if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
		{
			/*
				AP have no way to know that the PwrSaving STA is leaving or not.
				So do not disconnect for PwrSaving STA.
			*/
			if (pEntry->PsMode != PWR_SAVE)
			{
				bDisconnectSta = TRUE;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
						PRINT_MAC(pEntry->Addr), pEntry->StaIdleTimeout));
				ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
#ifdef WH_EVENT_NOTIFIER
            	if(pEntry)
            	{
                	EventHdlr pEventHdlrHook = NULL;
                	pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_TIMEOUT);
                	if(pEventHdlrHook && pEntry->wdev)
                    	pEventHdlrHook(pAd, pEntry);
            	}
#endif /* WH_EVENT_NOTIFIER */
			}
			else if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
			{
				/*
					AP have no way to know that the PwrSaving STA is leaving or not.
					So do not disconnect for PwrSaving STA.
				*/
				if (pEntry->PsMode != PWR_SAVE)
				{
					bDisconnectSta = TRUE;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("STA-%02x:%02x:%02x:%02x:%02x:%02x had left (%d %lu)\n",
						PRINT_MAC(pEntry->Addr),
						pEntry->ContinueTxFailCnt, pAd->ApCfg.EntryLifeCheck));
#ifdef WH_EVENT_NOTIFIER
                	if(pEntry)
                	{
                    	EventHdlr pEventHdlrHook = NULL;
                    	pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_TIMEOUT);
                    	if(pEventHdlrHook && pEntry->wdev)
                        	pEventHdlrHook(pAd, pEntry);
                	}
#endif /* WH_EVENT_NOTIFIER */
				}
			}
		}
		if ((pMbss->RssiLowForStaKickOut != 0) &&
			  ( (avgRssi=RTMPAvgRssi(pAd, pEntry->wdev, &pEntry->RssiSample)) < pMbss->RssiLowForStaKickOut))
		{
			bDisconnectSta = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Disassoc STA %02x:%02x:%02x:%02x:%02x:%02x , RSSI Kickout Thres[%d]-[%d]\n",
					PRINT_MAC(pEntry->Addr), pMbss->RssiLowForStaKickOut,
					avgRssi));

		}


		if (bDisconnectSta)
		{
			/* send wireless event - for ageout */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0);

			if (pEntry->Sst == SST_ASSOC)
			{
				PUCHAR pOutBuffer = NULL;
				NDIS_STATUS NStatus;
				ULONG FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				USHORT Reason;

				/*  send out a DISASSOC request frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS)
				{
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					/*NdisReleaseSpinLock(&pAd->MacTabLock); */
					continue;
				}
				Reason = REASON_DEAUTH_STA_LEAVING;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Send DEAUTH - Reason = %d frame  TO %x %x %x %x %x %x \n",
										Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
								pMbss->wdev.if_addr,
								pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen,
								sizeof(HEADER_802_11), &DeAuthHdr,
								2, &Reason,
								END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory( pOutBuffer);

#ifdef MAC_REPEATER_SUPPORT
				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
                    IS_ENTRY_CLIENT(pEntry)
#ifdef MWDS
					&& (pEntry->bEnableMWDS == FALSE)
#endif /* MWDS */
                    )
				{
					UCHAR apCliIdx, CliIdx;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					pReptEntry = RTMPLookupRepeaterCliEntry(
                                                pAd,
                                                TRUE,
                                                pEntry->Addr,
                                                TRUE);
					if (pReptEntry &&
                        (pReptEntry->CliConnectState != REPT_ENTRY_DISCONNT))
					{
						pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA;
						apCliIdx = pReptEntry->MatchApCliIdx;
						CliIdx = pReptEntry->MatchLinkIdx;
						MlmeEnqueue(pAd,
									APCLI_CTRL_STATE_MACHINE,
									APCLI_CTRL_DISCONNECT_REQ,
									0,
									NULL,
									(REPT_MLME_START_IDX + CliIdx));
						RTMP_MLME_HANDLER(pAd);
					}
				}
#endif /* MAC_REPEATER_SUPPORT */
			}

#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(pEntry->wdev))
			{
				struct _ez_peer_security_info * ez_peer = NULL;
				ez_peer = ez_peer_table_search_by_addr(pEntry->wdev,pEntry->Addr);
				if (ez_peer)
				{
#ifdef EZ_MOD_SUPPORT
					ez_set_delete_peer_in_differed_context(pEntry->wdev, ez_peer, TRUE);
#else
					ez_peer->delete_in_differred_context = TRUE;
#endif
					ez_send_unicast_deauth(pAd,pEntry->Addr);
				} else {
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				}
			}
			else {
#endif
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
#ifdef WH_EZ_SETUP
			}
#endif
			continue;
		}

#if defined(CONFIG_HOTSPOT_R2) || defined (CONFIG_DOT11V_WNM)
		if (pEntry->BTMDisassocCount == 1)
		{
			PUCHAR      pOutBuffer = NULL;
			NDIS_STATUS NStatus;
			ULONG       FrameLen = 0;
			HEADER_802_11 DisassocHdr;
			USHORT      Reason;

			/*  send out a DISASSOC request frame */
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
			if (NStatus != NDIS_STATUS_SUCCESS)
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
				/*NdisReleaseSpinLock(&pAd->MacTabLock); */
				continue;
			}

			Reason = REASON_DISASSOC_INACTIVE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BTM ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",Reason,pEntry->Addr[0],
				pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));
			MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pMbss->wdev.if_addr, pMbss->wdev.bssid);
			MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
			MiniportMMRequest(pAd, MGMT_USE_PS_FLAG, pOutBuffer, FrameLen);
			MlmeFreeMemory( pOutBuffer);
			//JERRY
			if (!pEntry->IsKeep){
#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(pEntry->wdev))
			{
#ifdef EZ_MOD_SUPPORT
				void * ez_peer = NULL;
#else
				struct _ez_peer_security_info * ez_peer = NULL;
#endif
				ez_peer = ez_peer_table_search_by_addr(pEntry->wdev,pEntry->Addr);
				if (ez_peer)
				{
#ifdef EZ_MOD_SUPPORT
					ez_set_delete_peer_in_differed_context(pEntry->wdev, ez_peer, TRUE);
#else
					ez_peer->delete_in_differred_context = TRUE;
#endif
					ez_send_unicast_deauth(pAd,pEntry->Addr);
				} else {
					MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
				}
			}
			else {
#endif
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
#ifdef WH_EZ_SETUP
			}
#endif
			}
			continue;
		}
		if (pEntry->BTMDisassocCount != 0)
			pEntry->BTMDisassocCount--;
#endif /* CONFIG_HOTSPOT_R2 */

		/* 3. garbage collect the ps_queue if the STA has being idle for a while */
		if ((pEntry->PsMode == PWR_SAVE) && (tr_entry->ps_state == APPS_RETRIEVE_DONE || tr_entry->ps_state == APPS_RETRIEVE_IDLE))
		{
			 if (tr_entry->enqCount > 0)
			{
				tr_entry->PsQIdleCount++;
				if (tr_entry->PsQIdleCount > 5)
				{
					rtmp_tx_swq_exit(pAd, pEntry->wcid);
					tr_entry->PsQIdleCount = 0;
					WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Clear WCID[%d] packets\n",__FUNCTION__, pEntry->wcid));
				}
			}
		}
		else
		{
			tr_entry->PsQIdleCount = 0;
		}

#ifdef UAPSD_SUPPORT
		UAPSD_QueueMaintenance(pAd, pEntry);
#endif /* UAPSD_SUPPORT */

		/* check if this STA is Ralink-chipset */
		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET)) {
            if(pEntry->wdev){
            UCHAR band_idx;

            band_idx = HcGetBandByWdev(pEntry->wdev);
			pMacTable->fAllStationAsRalink[band_idx] = FALSE;
        }
        }

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if ((pEntry->BSS2040CoexistenceMgmtSupport)
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		)
		{
			SendNotifyBWActionFrame(pAd, pEntry->wcid, pEntry->func_tb_idx);
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef WAPI_SUPPORT
		if (IS_CIPHER_WPI_SMS4(pEntry->SecConfig.PairwiseCipher))
			pMacTable->fAnyWapiStation = TRUE;
#endif /* WAPI_SUPPORT */

#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
		lastClient = wcid;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */

		/* only apply burst when run in MCS0,1,8,9,16,17, not care about phymode */
		if ((pEntry->HTPhyMode.field.MCS != 32) &&
			((pEntry->HTPhyMode.field.MCS % 8 == 0) || (pEntry->HTPhyMode.field.MCS % 8 == 1)))
		{
			pMacTable->fAllStationGainGoodMCS = FALSE;
		}

		/* Check Current STA's Operation Mode is BW20 or BW40 */
		pMacTable->fCurrentStaBw40 = (pEntry->HTPhyMode.field.BW == BW_40) ? TRUE : FALSE;

#ifdef WFA_VHT_PF
		if (worst_rssi == NULL) {
			worst_rssi = &pEntry->RssiSample;
			worst_rssi_sta_idx = wcid;
		} else {
			if (worst_rssi->AvgRssi[0] > pEntry->RssiSample.AvgRssi[0]) {
				worst_rssi = &pEntry->RssiSample;
				worst_rssi_sta_idx = wcid;
			}
		}
#endif /* WFA_VHT_PF */


#ifdef STA_FORCE_ROAM_SUPPORT
		if(	IS_ENTRY_CLIENT(pEntry) && ( ((PRTMP_ADAPTER)(pEntry->wdev->sys_handle))->en_force_roam_supp )
			&& tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			 && (!pEntry->is_peer_entry_apcli)
		  )
		{
			sta_rssi_check(pAd, pEntry);
		}
#endif

#ifdef WH_EVENT_NOTIFIER
        if(pAd->ApCfg.EventNotifyCfg.bStaRssiDetect)
        {
            avgRssi = RTMPAvgRssi(pAd, pEntry->wdev, &pEntry->RssiSample);
            if(avgRssi < pAd->ApCfg.EventNotifyCfg.StaRssiDetectThreshold)
            {
                if(pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef MWDS
                    && !IS_MWDS_OPMODE_AP(pEntry)
#endif /* MWDS */
                    && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
                    )
                {
                    EventHdlr pEventHdlrHook = NULL;
                    pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_RSSI_TOO_LOW);
                    if(pEventHdlrHook && pEntry->wdev)
                        pEventHdlrHook(pAd, pEntry->wdev, pEntry->Addr, avgRssi);
                }
            }
        }
#endif       
#if (defined(WH_EVENT_NOTIFIER) || (defined(MWDS) && defined(IGMP_SNOOP_SUPPORT)))       	
        if(pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef MWDS
           && !IS_MWDS_OPMODE_AP(pEntry)
#endif /* MWDS */
           && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
          )
       {
#ifdef WH_EVENT_NOTIFIER	  
            EventHdlr pEventHdlrHook = NULL;
            struct EventNotifierCfg *pEventNotifierCfg = &pAd->ApCfg.EventNotifyCfg;
            if(pEventNotifierCfg->bStaStateTxDetect && (pEventNotifierCfg->StaTxPktDetectPeriod > 0))
            {
               pEventNotifierCfg->StaTxPktDetectRound++;
               if(((pEventNotifierCfg->StaTxPktDetectRound % pEventNotifierCfg->StaTxPktDetectPeriod) == 0))
               {
                   if((pEntry->tx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
                      (pEntry->tx_state.PacketCount < pEventNotifierCfg->StaStateTxThreshold))
                   {
                       pEntry->tx_state.CurrentState = WHC_STA_STATE_IDLE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, TRUE);
                   }
                   else if((pEntry->tx_state.CurrentState == WHC_STA_STATE_IDLE) &&
                           (pEntry->tx_state.PacketCount >= pEventNotifierCfg->StaStateTxThreshold))
                   {
                       
                       pEntry->tx_state.CurrentState = WHC_STA_STATE_ACTIVE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, TRUE);
                   }
                   pEventNotifierCfg->StaTxPktDetectRound = 0;
                   pEntry->tx_state.PacketCount = 0;
               }
            }

            if(pEventNotifierCfg->bStaStateRxDetect && (pEventNotifierCfg->StaRxPktDetectPeriod > 0))
            {
               pEventNotifierCfg->StaRxPktDetectRound++;
               if(((pEventNotifierCfg->StaRxPktDetectRound % pEventNotifierCfg->StaRxPktDetectPeriod) == 0))
               {
                   if((pEntry->rx_state.CurrentState == WHC_STA_STATE_ACTIVE) &&
                      (pEntry->rx_state.PacketCount < pEventNotifierCfg->StaStateRxThreshold))
                   {
                       pEntry->rx_state.CurrentState = WHC_STA_STATE_IDLE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, FALSE);
                   }
                   else if((pEntry->rx_state.CurrentState == WHC_STA_STATE_IDLE) &&
                           (pEntry->rx_state.PacketCount >= pEventNotifierCfg->StaStateRxThreshold))
                   {
                       pEntry->rx_state.CurrentState = WHC_STA_STATE_ACTIVE;
                       pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_ACTIVITY_STATE);
                       if(pEventHdlrHook && pEntry->wdev)
                           pEventHdlrHook(pAd, pEntry, FALSE);
                   }
                   pEventNotifierCfg->StaRxPktDetectRound = 0;
                   pEntry->rx_state.PacketCount = 0;
               }
       } 
#endif /* WH_EVENT_NOTIFIER */
#if defined(MWDS) && defined(IGMP_SNOOP_SUPPORT)
			if(pMbss->wdev.bSupportMWDS && pMbss->wdev.IgmpSnoopEnable){	// If Snooping & MWDS enabled
				if((pAd->Mlme.OneSecPeriodicRound % 10) == 0)	// Per 10 sec check
				{
					// Logic implemented to do Periodic Multicast membership queries to NON-MWDS STAs
					// for update of entries in IGMP table, to avoid aging of interested members.

					// Also, in case another Membership querier detected in network, logic implemented to
					// hold internal query transmission to avoid flooding on network.
					
					// Decrement time tick counters for Query Sent & Query hold as applicable
					// Send the query once on each MBSS, for which both these counters are 0.
					// (Ensured that even with multiple STA on a MBSS, only one multicast query transmitted)

					if(pAd->bIGMPperiodicQuery == TRUE){	// If Periodic IGMP query feature enabled
					
						if((pMbss->IgmpQueryHoldTick > 0) && (pMbss->IgmpQueryHoldTickChanged == FALSE)){
							// Decrement IgmpQueryHoldTick, only once for each MBSS
							pMbss->IgmpQueryHoldTick--;
							pMbss->IgmpQueryHoldTickChanged = TRUE;
						}

						if((pAd->IgmpQuerySendTick > 0) && (IgmpQuerySendTickChanged == FALSE)){
							// Decrement IgmpQuerySendTick, only once for each MBSS
							pAd->IgmpQuerySendTick--;
							IgmpQuerySendTickChanged = TRUE;
						}

						if((pMbss->IGMPPeriodicQuerySent == FALSE)
						   && ((pMbss->IgmpQueryHoldTick == 0) && (pAd->IgmpQuerySendTick == 0)))
						{
							// transmit IGMP query on this MBSS, only once
							send_igmpv3_gen_query_pkt(pAd,pEntry);

							pMbss->IGMPPeriodicQuerySent = TRUE;
						}

					}

					if(pAd->bMLDperiodicQuery == TRUE){	// If Periodic MLD query feature enabled

						if((pMbss->MldQueryHoldTick > 0) && (pMbss->MldQueryHoldTickChanged == FALSE)){
							// Decrement MldQueryHoldTick, only once for each MBSS
							pMbss->MldQueryHoldTick--;
							pMbss->MldQueryHoldTickChanged = TRUE;
						}

						if((pAd->MldQuerySendTick > 0) && (MldQuerySendTickChanged == FALSE)){
							// Decrement MldQuerySendTick, only once for each MBSS
							pAd->MldQuerySendTick--;
							MldQuerySendTickChanged = TRUE;
						}
						if((pMbss->MLDPeriodicQuerySent == FALSE)
						   && ((pMbss->MldQueryHoldTick == 0) && (pAd->MldQuerySendTick == 0)))
						{
							// transmit MLD query on this MBSS, only once
							send_mldv2_gen_query_pkt(pAd,pEntry);

							pMbss->MLDPeriodicQuerySent = TRUE;
						}

					}

				}
				else if((pAd->Mlme.OneSecPeriodicRound % 10) == 1) // Per 11 sec check
				{
					if(pAd->IgmpQuerySendTick == 0) // Set the period for IGMP query again
						pAd->IgmpQuerySendTick= QUERY_SEND_PERIOD;

					if(pAd->MldQuerySendTick == 0) // Set the period for MLD query again
						pAd->MldQuerySendTick= QUERY_SEND_PERIOD;

					if(pMbss->IGMPPeriodicQuerySent == TRUE)
						pMbss->IGMPPeriodicQuerySent = FALSE; // Reset flag for next period query

					if(pMbss->MLDPeriodicQuerySent == TRUE)
						pMbss->MLDPeriodicQuerySent = FALSE; // Reset flag for next period query

					pMbss->IgmpQueryHoldTickChanged = FALSE; // Reset flag for next 10th second counter edit
					pMbss->MldQueryHoldTickChanged = FALSE; // Reset flag for next 10th second counter edit

				}
			}
#endif

		}
#endif
	}


#ifdef MT_MAC
	/* If we check that any preview stations are in Psm and no stations are in Psm now. */
	/* AP will dequeue all buffer broadcast packets */

	if (/*(pAd->chipCap.hif_type == HIF_MT) && */(pMacTable->fAnyStationInPsm == FALSE)) {
		UINT apidx = 0;
        for (apidx = 0; apidx<pAd->ApCfg.BssidNum; apidx++)
        {
            BSS_STRUCT *pMbss;
            UINT wcid = 0;
            STA_TR_ENTRY *tr_entry = NULL;

            pMbss = &pAd->ApCfg.MBSSID[apidx];

            wcid = pMbss->wdev.tr_tb_idx;
            tr_entry = &pAd->MacTab.tr_entry[wcid];

			if ((bPreAnyStationInPsm == TRUE) &&  (tr_entry->tx_queue[QID_AC_BE].Head != NULL)) {
					if (tr_entry->tx_queue[QID_AC_BE].Number > MAX_PACKETS_IN_MCAST_PS_QUEUE)
						RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, wcid, MAX_PACKETS_IN_MCAST_PS_QUEUE);
					else
						RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, wcid, tr_entry->tx_queue[QID_AC_BE].Number);
			}
		}
	}
#endif

#ifdef WFA_VHT_PF
	if (worst_rssi != NULL &&
		((pAd->Mlme.OneSecPeriodicRound % 10) == 5) &&
		(worst_rssi_sta_idx >= 1))
	{
		CHAR gain = 2;
		if (worst_rssi->AvgRssi[0] >= -40)
			gain = 1;
		else if (worst_rssi->AvgRssi[0] <= -50)
			gain = 2;
		rt85592_lna_gain_adjust(pAd, gain);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():WorstRSSI for STA(%02x:%02x:%02x:%02x:%02x:%02x):%d,%d,%d, Set Gain as %s\n",
					__FUNCTION__,
					PRINT_MAC(pMacTable->Content[worst_rssi_sta_idx].Addr),
					worst_rssi->AvgRssi[0], worst_rssi->AvgRssi[1], worst_rssi->AvgRssi[2],
					(gain == 2 ? "Mid" : "Low")));
	}
#endif /* WFA_VHT_PF */

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */

#ifdef CFO_TRACK
#endif /* CFO_TRACK */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);
#endif /* DOT11N_DRAFT3 */

	/* If all associated STAs are Ralink-chipset, AP shall enable RDG. */
	if (pAd->CommonCfg.bRdg)
    {
        if (pMacTable->fAllStationAsRalink[0])
            bRdgActive = TRUE;
        else
            bRdgActive = FALSE;

        if (pAd->CommonCfg.dbdc_mode)
        {
            if (pMacTable->fAllStationAsRalink[1])
                ; // Not support yet...
        }
    }

#ifdef APCLI_SUPPORT
	if (apcli_wdev) {
		UINT tx_tp = (apcli_avg_tx >> BYTES_PER_SEC_TO_MBPS);
		UINT rx_tp = (apcli_avg_rx >> BYTES_PER_SEC_TO_MBPS);
		apcli_dync_txop_alg(pAd, apcli_wdev, tx_tp, rx_tp);
	}
#endif /* APCLI_SUPPORT */

                if (sta_hit_2g_infra_case_number == STA_NUMBER_FOR_TRIGGER ) {
                        if (pAd->G_MODE_INFRA_TXOP_RUNNING == FALSE) {
                                pAd->g_mode_txop_wdev = txop_wdev;
                                pAd->G_MODE_INFRA_TXOP_RUNNING = TRUE;
                                enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
                                
                        } else if (pAd->g_mode_txop_wdev != txop_wdev) {
                                disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
                                enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
                                pAd->g_mode_txop_wdev = txop_wdev;
                                
                        }
                        
                } else {
                        if (pAd->G_MODE_INFRA_TXOP_RUNNING == TRUE) {
                                disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
                                pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE; 
                                pAd->g_mode_txop_wdev = NULL;
                        }
                }
	if (pAd->txop_ctl.multi_client_nums == MULTI_CLIENT_NUMS_TH) {
		if (pAd->txop_ctl.multi_cli_txop_running == FALSE) {
			pAd->txop_ctl.multi_cli_txop_running = TRUE;
			enable_tx_burst(pAd, pAd->txop_ctl.cur_wdev, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}

	} else {
		if (pAd->txop_ctl.multi_cli_txop_running == TRUE) {
			pAd->txop_ctl.multi_cli_txop_running = FALSE;
			disable_tx_burst(pAd, pAd->txop_ctl.cur_wdev, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}
	}

	dynamic_ampdu_efficiency_adjust_all(pAd);

	if (pAd->txop_ctl.multi_client_nums_2g == MULTI_CLIENT_2G_NUMS_TH) {
		if (pAd->txop_ctl.multi_cli_txop_2g_running == FALSE) {
			pAd->txop_ctl.multi_cli_txop_2g_running = TRUE;
			enable_tx_burst(pAd, pAd->txop_ctl.cur_wdev_2g, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}

	} else {
		if (pAd->txop_ctl.multi_cli_txop_2g_running == TRUE) {
			pAd->txop_ctl.multi_cli_txop_2g_running = FALSE;
			disable_tx_burst(pAd, pAd->txop_ctl.cur_wdev_2g, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}
	}
	/*dynamic adjust amsdu & protection mode*/
	dynamic_amsdu_protect_adjust(pAd);

	if (pAd->CommonCfg.bRalinkBurstMode && pMacTable->fAllStationGainGoodMCS)
		bRalinkBurstMode = TRUE;
	else
		bRalinkBurstMode = FALSE;

	if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
	{
        // DBDC not support yet, only using BAND_0
        if (bRdgActive) {
            AsicSetRDG(pAd, WCID_ALL, 0, 1, 1);
        }
        else {
            AsicSetRDG(pAd, WCID_ALL, 0, 0, 0);
        }
        //AsicWtblSetRDG(pAd, bRdgActive);
	}

	if (bRalinkBurstMode != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
		AsicSetRalinkBurstMode(pAd, bRalinkBurstMode);

#if defined(RTMP_MAC) || defined(RLT_MAC)
    if (pAd->chipCap.hif_type == HIF_RTMP
            || pAd->chipCap.hif_type == HIF_RLT)
    {
    	ADD_HT_INFO_IE *addht = NULL;
		UCHAR band = 0;
		if(wdev)
		{
			addht = wlan_operate_get_addht(wdev);
			band = HcGetBandByWdev(wdev);

			if ((pMacTable->fAnyStationBadAtheros == FALSE)
					&& (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == TRUE))
			{
				AsicUpdateProtect(pAd, addht->AddHtInfo2.OperaionMode,
						ALLN_SETPROTECT, FALSE, pMacTable->fAnyStationNonGF[band]);
			}
		}
    }
#endif

#endif /* DOT11_N_SUPPORT */

#ifdef RTMP_MAC_PCI
    RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
    /*
       4.
       garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
       stale in queue. Since MCAST/BCAST frames always been sent out whenever
		DtimCount==0, the only case to let them stale is surprise removal of the NIC,
		so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	*/
	// TODO: shiang-usw. revise this becasue now we have per-BSS McastPsQueue!
	if (pMacTable->McastPsQueue.Head)
	{
		UINT bss_index;

		pMacTable->PsQIdleCount ++;
		if (pMacTable->PsQIdleCount > 1)
		{

			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			pMacTable->PsQIdleCount = 0;

			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);

			/* clear MCAST/BCAST backlog bit for all BSS */
			for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		}
	}
	else
		pMacTable->PsQIdleCount = 0;
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */

	pAd->txop_ctl.multi_client_nums = 0;
	pAd->txop_ctl.multi_client_nums_2g = 0;
}


UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd)
{
	UINT32 num = 0;
	UINT32 i;

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->Sst == SST_ASSOC)
			num ++;
	}

	return num;
}


/*
	==========================================================================
	Description:
		Look up a STA MAC table. Return its Sst to decide if an incoming
		frame from this STA or an outgoing frame to this STA is permitted.
	Return:
	==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	OUT SST *Sst,
	OUT USHORT *Aid,
	OUT UCHAR *PsMode,
	OUT UCHAR *Rate)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (MAC_ADDR_IS_GROUP(pAddr)) /* mcast & broadcast address */
	{
		*Sst = SST_ASSOC;
		*Aid = MCAST_WCID_TO_REMOVE;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index */
		*PsMode = PWR_ACTIVE;
		*Rate = pAd->CommonCfg.MlmeRate;
	}
	else /* unicast address */
	{
		pEntry = MacTableLookup(pAd, pAddr);
		if (pEntry)
		{
			*Sst = pEntry->Sst;
			*Aid = pEntry->Aid;
			*PsMode = pEntry->PsMode;
			if (IS_AKM_WPA_CAPABILITY_Entry(pEntry)
				&& (pEntry->SecConfig.Handshake.GTKState != REKEY_ESTABLISHED))
			{
				*Rate = pAd->CommonCfg.MlmeRate;
			} else
				*Rate = pEntry->CurrTxRate;
		}
		else
		{
			*Sst = SST_NOT_AUTH;
			*Aid = MCAST_WCID_TO_REMOVE;
			*PsMode = PWR_ACTIVE;
			*Rate = pAd->CommonCfg.MlmeRate;
		}
	}

	return pEntry;
}


#ifdef SYSTEM_LOG_SUPPORT
/*
	==========================================================================
	Description:
		This routine is called to log a specific event into the event table.
		The table is a QUERY-n-CLEAR array that stop at full.
	==========================================================================
 */
VOID ApLogEvent(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Event)
{
	if (pAd->EventTab.Num < MAX_NUM_OF_EVENT)
	{
		RT_802_11_EVENT_LOG *pLog = &pAd->EventTab.Log[pAd->EventTab.Num];
		RTMP_GetCurrentSystemTime(&pLog->SystemTime);
		COPY_MAC_ADDR(pLog->Addr, pAddr);
		pLog->Event = Event;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("LOG#%ld %02x:%02x:%02x:%02x:%02x:%02x %s\n",
			pAd->EventTab.Num, pAddr[0], pAddr[1], pAddr[2],
			pAddr[3], pAddr[4], pAddr[5], pEventText[Event]));
		pAd->EventTab.Num += 1;
	}
}
#endif /* SYSTEM_LOG_SUPPORT */


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		Operationg mode is as defined at 802.11n for how proteciton in this BSS operates.
		Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
		802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

		Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
		40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
		As STA, this obeys the operation mode in ADDHT IE.
		As AP, update protection when setting ADDHT IE and after new STA joined.
	==========================================================================
*/
VOID APUpdateOperationMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN bDisableBGProtect = FALSE, bNonGFExist = FALSE;
	ADD_HT_INFO_IE *addht = NULL;
	UCHAR band = 0;
	UINT32 new_protection = 0;
	UCHAR op_mode = NON_PROTECT;

	if (wdev == NULL)
		return ;

	addht = wlan_operate_get_addht(wdev);
	band = HcGetBandByWdev(wdev);

	/* non HT BSS exist within 5 sec */
	if ((pAd->ApCfg.LastNoneHTOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32 && pAd->Mlme.Now32!=0) {
		op_mode = NONMEMBER_PROTECT;
		bDisableBGProtect = FALSE;
		/* non-HT means nonGF support */
		bNonGFExist = TRUE;
		new_protection = SET_PROTECT(NON_MEMBER_PROTECT);
	}

   	/* If I am 40MHz BSS, and there exist HT-20MHz station. */
	/* Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection. */
	if ((op_mode == NON_PROTECT) &&
		(pAd->MacTab.fAnyStation20Only) &&
		(wlan_config_get_ht_bw(wdev)== HT_BW_40))
	{
		op_mode = BW20_PROTECT;
		bDisableBGProtect = TRUE;
		new_protection = SET_PROTECT(HT20_PROTECT);
	}

	if (pAd->MacTab.fAnyStationIsLegacy)
	{
		op_mode = NONHT_MM_PROTECT;
		bDisableBGProtect = TRUE;
		new_protection = SET_PROTECT(NON_HT_MIXMODE_PROTECT);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			(" --%s:\n OperationMode: %d, bDisableBGProtect: %d, bNonGFExist: %d\n",
			 __FUNCTION__, addht->AddHtInfo2.OperaionMode,
			 bDisableBGProtect, bNonGFExist));

	if ((op_mode != addht->AddHtInfo2.OperaionMode)
			|| (pAd->MacTab.fAnyStationNonGF[band] != addht->AddHtInfo2.NonGfPresent))
	{
		addht->AddHtInfo2.OperaionMode = op_mode;
		addht->AddHtInfo2.NonGfPresent = pAd->MacTab.fAnyStationNonGF[band];

		UpdateBeaconHandler(pAd, wdev, IE_CHANGE);
	}


	if (bNonGFExist == FALSE) {
		bNonGFExist = pAd->MacTab.fAnyStationNonGF[band];
	}
	if (bNonGFExist) {
		new_protection |= SET_PROTECT(GREEN_FIELD_PROTECT);
	}

	if (nonerp_protection(wdev)) {
		new_protection |= SET_PROTECT(ERP);
	}
	if(MTK_REV_GTE(pAd, MT7615, MT7615E1) && MTK_REV_LT(pAd, MT7615, MT7615E3))
	{
		if(pAd->CommonCfg.dbdc_mode && op_mode == NON_PROTECT)
		{
			op_mode = BW20_PROTECT;
			new_protection |= SET_PROTECT(HT20_PROTECT);
		}
	}


#if defined(MT_MAC)
	/*if (pAd->chipCap.hif_type == HIF_MT)*/
	{
		if ((new_protection & wdev->protection) != new_protection) {
			wdev->protection = new_protection;

    			AsicUpdateProtect(pAd,
	            		op_mode,
	            		(ALLN_SETPROTECT),
	            		bDisableBGProtect,
	            		bNonGFExist);
		}

	}
#endif

#if defined(RTMP_MAC) || defined(RLT_MAC)
    if (pAd->chipCap.hif_type == HIF_RTMP
            || pAd->chipCap.hif_type == HIF_RLT) {
	    AsicUpdateProtect(pAd,
	            op_mode,
	            (ALLN_SETPROTECT),
	            bDisableBGProtect,
	            bNonGFExist);
	}
#endif
}
#endif /* DOT11_N_SUPPORT */


/*
	==========================================================================
	Description:
		Check if the specified STA pass the Access Control List checking.
		If fails to pass the checking, then no authentication nor association
		is allowed
	Return:
		MLME_SUCCESS - this STA passes ACL checking

	==========================================================================
*/
BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR Apidx)
{
	BOOLEAN Result = TRUE;

    if (Apidx >= HW_BEACON_MAX_NUM)
        return FALSE;

    if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 0)       /* ACL is disabled */
        Result = TRUE;
    else
    {
        ULONG i;
        if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 1)   /* ACL is a positive list */
            Result = FALSE;
        else                                              /* ACL is a negative list */
            Result = TRUE;
        for (i=0; i<pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++)
        {
            if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Addr))
            {
                Result = !Result;
                break;
            }
        }
    }

    if (Result == FALSE)
    {
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x failed in ACL checking\n",
        			PRINT_MAC(pAddr)));
    }

    return Result;
}


/*
	==========================================================================
	Description:
		This routine update the current MAC table based on the current ACL.
		If ACL change causing an associated STA become un-authorized. This STA
		will be kicked out immediately.
	==========================================================================
*/
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR Apidx)
{
	USHORT   AclIdx, MacIdx;
	BOOLEAN  Matched;
	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DisassocHdr;
	USHORT      Reason;
	MAC_TABLE_ENTRY *pEntry;
	BSS_STRUCT *pMbss;
	BOOLEAN drop;

	ASSERT(Apidx < MAX_MBSSID_NUM(pAd));
	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApUpdateAccessControlList : Apidx = %d\n", Apidx));

	/* ACL is disabled. Do nothing about the MAC table. */
	pMbss = &pAd->ApCfg.MBSSID[Apidx];
	if (pMbss->AccessControlList.Policy == 0)
		return;

	for (MacIdx=0; VALID_UCAST_ENTRY_WCID(pAd, MacIdx); MacIdx++)
	{
		pEntry = &pAd->MacTab.Content[MacIdx];
		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		/* We only need to update associations related to ACL of MBSSID[Apidx]. */
		if (pEntry->func_tb_idx != Apidx)
			continue;

		drop = FALSE;
		Matched = FALSE;
		 for (AclIdx = 0; AclIdx < pMbss->AccessControlList.Num; AclIdx++)
		{
			if (MAC_ADDR_EQUAL(&pEntry->Addr[0], pMbss->AccessControlList.Entry[AclIdx].Addr))
			{
				Matched = TRUE;
				break;
			}
		}

		if ((Matched == FALSE) && (pMbss->AccessControlList.Policy == 1))
		{
			drop = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA not on positive ACL. remove it...\n"));
		}
	       else if ((Matched == TRUE) && (pMbss->AccessControlList.Policy == 2))
		{
			drop = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA on negative ACL. remove it...\n"));
		}

		if (drop == TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Apidx = %d\n", Apidx));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->ApCfg.MBSSID[%d].AccessControlList.Policy = %ld\n", Apidx,
				pMbss->AccessControlList.Policy));

			/* Before delete the entry from MacTable, send disassociation packet to client. */
			if (pEntry->Sst == SST_ASSOC)
			{
				/* send out a DISASSOC frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS)
				{
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					return;
				}

				Reason = REASON_DECLINED;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",
							Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0,
									pEntry->Addr,
									pMbss->wdev.if_addr,
									pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory( pOutBuffer);

				RtmpusecDelay(5000);
			}
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}
	}
}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
/*
	Depends on the 802.11n Draft 4.0, Before the HT AP start a BSS, it should scan some specific channels to
collect information of existing BSSs, then depens on the collected channel information, adjust the primary channel
and secondary channel setting.

	For 5GHz,
		Rule 1: If the AP chooses to start a 20/40 MHz BSS in 5GHz and that occupies the same two channels
				as any existing 20/40 MHz BSSs, then the AP shall ensure that the primary channel of the
				new BSS is identical to the primary channel of the existing 20/40 MHz BSSs and that the
				secondary channel of the new 20/40 MHz BSS is identical to the secondary channel of the
				existing 20/40 MHz BSSs, unless the AP discoverr that on those two channels are existing
				20/40 MHz BSSs with different primary and secondary channels.
		Rule 2: If the AP chooses to start a 20/40MHz BSS in 5GHz, the selected secondary channel should
				correspond to a channel on which no beacons are detected during the overlapping BSS
				scan time performed by the AP, unless there are beacons detected on both the selected
				primary and secondary channels.
		Rule 3: An HT AP should not start a 20 MHz BSS in 5GHz on a channel that is the secondary channel
				of a 20/40 MHz BSS.
	For 2.4GHz,
		Rule 1: The AP shall not start a 20/40 MHz BSS in 2.4GHz if the value of the local variable "20/40
				Operation Permitted" is FALSE.

		20/40OperationPermitted =  (P == OPi for all values of i) AND
								(P == OTi for all values of i) AND
								(S == OSi for all values if i)
		where
			P 	is the operating or intended primary channel of the 20/40 MHz BSS
			S	is the operating or intended secondary channel of the 20/40 MHz BSS
			OPi  is member i of the set of channels that are members of the channel set C and that are the
				primary operating channel of at least one 20/40 MHz BSS that is detected within the AP's
				BSA during the previous X seconds
			OSi  is member i of the set of channels that are members of the channel set C and that are the
				secondary operating channel of at least one 20/40 MHz BSS that is detected within AP's
				BSA during the previous X seconds
			OTi  is member i of the set of channels that comparises all channels that are members of the
				channel set C that were listed once in the Channel List fields of 20/40 BSS Intolerant Channel
				Report elements receved during the previous X seconds and all channels that are members
				of the channel set C and that are the primary operating channel of at least one 20/40 MHz
				BSS that were detected within the AP's BSA during the previous X seconds.
			C	is the set of all channels that are allowed operating channels within the current operational
				regulatory domain and whose center frequency falls within the 40 MHz affected channel
				range given by following equation:
					                                                 Fp + Fs                  Fp + Fs
					40MHz affected channel range = [ ------  - 25MHz,  ------- + 25MHz ]
					                                                      2                          2
					Where
						Fp = the center frequency of channel P
						Fs = the center frequency of channel S

			"==" means that the values on either side of the "==" are to be tested for equaliy with a resulting
				 Boolean value.
			        =>When the value of OPi is the empty set, then the expression (P == OPi for all values of i)
			        	is defined to be TRUE
			        =>When the value of OTi is the empty set, then the expression (P == OTi for all values of i)
			        	is defined to be TRUE
			        =>When the value of OSi is the empty set, then the expression (S == OSi for all values of i)
			        	is defined to be TRUE
*/
INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN BSS_COEX_CH_RANGE *pCoexChRange,
	IN UCHAR Channel)
{
	INT index, cntrCh = 0;
	UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

	memset(pCoexChRange, 0, sizeof(BSS_COEX_CH_RANGE));

	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT
	if (Channel > 14)
	{	/* For 5GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = pAd->ChannelList[index].Channel;

			/* Now check about the secondary and central channel */
			if(op_ext_cha == EXTCHA_ABOVE)
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh + 4;
				pCoexChRange->secondaryCh = pCoexChRange->effectChEnd;
			}
			else
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh -4;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh;
				pCoexChRange->secondaryCh = pCoexChRange->effectChStart;
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("5.0GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
										index,
										((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel,
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			return TRUE;
		}
		else
		{
			/* It should not happened! */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("5GHz: Cannot found the CtrlCh(%d) in ChList, something wrong?\n",
						Channel));
		}
	}
	else
#endif /* A_BAND_SUPPORT */
	{	/* For 2.4GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = index;

			/* Now check about the secondary and central channel */
			if(op_ext_cha == EXTCHA_ABOVE)
			{
				if ((index + 4) < pAd->ChannelListNum)
				{
					cntrCh = index + 2;
					pCoexChRange->secondaryCh = index + 4;
				}
			}
			else
			{
				if ((index - 4) >=0)
				{
					cntrCh = index - 2;
					pCoexChRange->secondaryCh = index - 4;
				}
			}

			if (cntrCh)
			{
				pCoexChRange->effectChStart = (cntrCh - 5) > 0 ? (cntrCh - 5) : 0;
				pCoexChRange->effectChEnd= (cntrCh + 5) < pAd->ChannelListNum ? (cntrCh + 5) : (pAd->ChannelListNum - 1);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("2.4GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
										index,
										((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel,
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			}
			return TRUE;
		}

		/* It should not happened! */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("2.4GHz: Didn't found valid channel range, Ch index=%d, ChListNum=%d, CtrlCh=%d\n",
									index, pAd->ChannelListNum, Channel));
	}

	return FALSE;
}


VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	BOOLEAN needFallBack = FALSE;
	INT chStartIdx, chEndIdx, index,curPriChIdx, curSecChIdx;
	BSS_COEX_CH_RANGE  coexChRange;
	UCHAR PhyMode = wdev->PhyMode;
	UCHAR Channel = wdev->channel;
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	if (!WMODE_CAP_2G(PhyMode))
	{
		return;
	}

	/* We just care BSS who operating in 40MHz N Mode. */
	if ((!WMODE_CAP_N(PhyMode)) ||
		(ht_bw == BW_20)
		)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The wdev->PhyMode=%d, BW=%d, didn't need channel adjustment!\n",
				PhyMode, ht_bw));
		return;
	}
#ifdef APCLI_CERT_SUPPORT
	if (pAd->bApCliCertTest == TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->bApCliCertTest is Enable,ignore APOverlappingBSSScan\n"));
		return;
	}
#endif
	/* Build the effected channel list, if something wrong, return directly. */
	/* For 2.4GHz band */
	for (index = 0; index < pAd->AutoChSelCtrl.ChannelListNum2G; index++)
	{
		if(pAd->ChannelList[index].Channel == Channel)
			break;
	}

	if (index < pAd->AutoChSelCtrl.ChannelListNum2G)
	{

		if(ext_cha == EXTCHA_ABOVE)
		{
			curPriChIdx = index;
			curSecChIdx = ((index + 4) < pAd->AutoChSelCtrl.ChannelListNum2G) ? (index + 4) : (pAd->AutoChSelCtrl.ChannelListNum2G - 1);

			chStartIdx = (curPriChIdx >= 3) ? (curPriChIdx - 3) : 0;
			chEndIdx = ((curSecChIdx + 3) < pAd->AutoChSelCtrl.ChannelListNum2G) ? (curSecChIdx + 3) : (pAd->AutoChSelCtrl.ChannelListNum2G - 1);
		}
		else
		{
			curPriChIdx = index;
			curSecChIdx = ((index - 4) >=0 ) ? (index - 4) : 0;
			chStartIdx =(curSecChIdx >= 3) ? (curSecChIdx - 3) : 0;
			chEndIdx =  ((curPriChIdx + 3) < pAd->AutoChSelCtrl.ChannelListNum2G) ? (curPriChIdx + 3) : (pAd->AutoChSelCtrl.ChannelListNum2G - 1);;
		}
	}
	else
	{
		/* It should not happened! */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("2.4GHz: Cannot found the Control Channel(%d) in ChannelList, something wrong?\n",
					Channel));
		return;
	}

	GetBssCoexEffectedChRange(pAd,wdev,&coexChRange, Channel);

	/* Before we do the scanning, clear the bEffectedChannel as zero for latter use. */
	for (index = 0; index < pAd->AutoChSelCtrl.ChannelListNum2G; index++)
		pAd->ChannelList[index].bEffectedChannel = 0;

	pAd->CommonCfg.BssCoexApCnt = 0;

	/* If we are not ready for Tx/Rx Pakcet, enable it now for receiving Beacons. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP) == 0)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Card still not enable Tx/Rx, enable it now!\n"));

		RTMP_IRQ_ENABLE(pAd);

		/* rtmp_rx_done_handle() API will check this flag to decide accept incoming packet or not. */
		/* Set the flag be ready to receive Beacon frame for autochannel select. */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}

	RTMPEnableRxTx(pAd);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Ready to do passive scanning for Channel[%d] to Channel[%d]!\n",
			pAd->ChannelList[chStartIdx].Channel, pAd->ChannelList[chEndIdx].Channel));

	/* Now start to do the passive scanning. */
	pAd->CommonCfg.bOverlapScanning = TRUE;
	for (index = chStartIdx; index<=chEndIdx; index++)
	{
		Channel = pAd->ChannelList[index].Channel;
		AsicSetChannel(pAd, Channel, BW_20,  EXTCHA_NONE, TRUE);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP OBSS SYNC - BBP R4 to 20MHz.l\n"));
		/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Passive scanning for Channel %d.....\n", Channel)); */
		OS_WAIT(300); /* wait for 200 ms at each channel. */
	}
	pAd->CommonCfg.bOverlapScanning = FALSE;

	/* After scan all relate channels, now check the scan result to find out if we need fallback to 20MHz. */
	for (index = chStartIdx; index <= chEndIdx; index++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Channel[Idx=%d, Ch=%d].bEffectedChannel=0x%x!\n",
					index, pAd->ChannelList[index].Channel, pAd->ChannelList[index].bEffectedChannel));
		if ((pAd->ChannelList[index].bEffectedChannel & (EFFECTED_CH_PRIMARY | EFFECTED_CH_LEGACY))  && (index != curPriChIdx) )
		{
			needFallBack = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("needFallBack=TRUE due to OP/OT!\n"));
		}
		if ((pAd->ChannelList[index].bEffectedChannel & EFFECTED_CH_SECONDARY)  && (index != curSecChIdx))
		{
			needFallBack = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("needFallBack=TRUE due to OS!\n"));
		}
	}

	/* If need fallback, now do it. */
	if ((needFallBack == TRUE)
		&& (pAd->CommonCfg.BssCoexApCnt > pAd->CommonCfg.BssCoexApCntThr)
	)
	{
#if (defined(WH_EZ_SETUP) && defined(EZ_NETWORK_MERGE_SUPPORT))
		if(IS_EZ_SETUP_ENABLED(wdev)){
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("\nAPOverlappingBSSScan: Fallback at bootup ****\n"));
			ez_set_ap_fallback_context(wdev,TRUE,wdev->channel);
		}
#endif /* WH_EZ_SETUP */
		
		wlan_operate_set_ht_bw(wdev,HT_BW_20);
		wlan_operate_set_ext_cha(wdev,EXTCHA_NONE);
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;

        /*update hw setting.*/
        HcBbpSetBwByChannel(pAd,BW_20,Channel);
	}

	return;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#define DBDC_AP_5G_PEAK_TP	610
#define DBDC_AP_2G_PEAK_TP	270
#define DBDC_PEAK_TP_PER_THRESHOLD 5
static VOID dynamic_ampdu_efficiency_adjust_5G(struct _RTMP_ADAPTER *ad)
{
	EDCA_PARM *edcaparam;
	ULONG per = 0;
	ULONG tx_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;

	if (ad->peak_tp_ctl.client_nums == 0)
		goto ignore_5g_ampdu_efficiency_check;

	if (ad->CommonCfg.dbdc_mode)
		wlan_ct = &ad->WlanCounters[1];
	else
		wlan_ct = &ad->WlanCounters[0];

	wdev = ad->peak_tp_ctl.cur_wdev;

	tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;
	if (tx_cnt > 0) 
	{
		per = 100*(wlan_ct->AmpduFailCount.u.LowPart)/(wlan_ct->AmpduFailCount.u.LowPart+tx_cnt);
	}
	if (per >= DBDC_PEAK_TP_PER_THRESHOLD) 
	{// do no apply patch, it is in noise environment
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]per=%lu\n", __func__,__LINE__, per));
		goto ignore_5g_ampdu_efficiency_check;
	}

	/* scenario detection */
	if (ad->peak_tp_ctl.max_tx_tp > DBDC_AP_5G_PEAK_TP)
	{
		ad->peak_tp_ctl.cli_peak_tp_running = 1;
	} 
	else
	{
		ad->peak_tp_ctl.cli_peak_tp_running = 0;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]per=%lu,tx=%d M,(%d,%d,%d)\n\r", 
			 __func__,__LINE__,per,ad->peak_tp_ctl.max_tx_tp,
			 ad->peak_tp_ctl.cli_peak_tp_running,
			 ad->peak_tp_ctl.cli_ampdu_efficiency_running,
			 ad->peak_tp_ctl.cli_peak_tp_txop_enable));

	/* increase ampdu efficiency if running peak T.P */
	if (ad->peak_tp_ctl.cli_peak_tp_running)
	{
		if (!ad->peak_tp_ctl.cli_ampdu_efficiency_running) 
		{
			if (query_tx_burst_prio(ad, wdev) <= PRIO_PEAK_TP)
			{
				AsicAmpduEfficiencyAdjust(wdev,0xf);
				ad->peak_tp_ctl.cli_ampdu_efficiency_running = TRUE;
			}
		}
		if (!ad->peak_tp_ctl.cli_peak_tp_txop_enable) 
		{
			enable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, TXOP_FE);
			ad->peak_tp_ctl.cli_peak_tp_txop_enable = TRUE;
		}
	}
	else
	{//restore to original
		if (ad->peak_tp_ctl.cli_ampdu_efficiency_running) 
		{
			edcaparam = HcGetEdca(ad, wdev);
			if (edcaparam)
				AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);
			ad->peak_tp_ctl.cli_ampdu_efficiency_running = FALSE;
		}
		if (ad->peak_tp_ctl.cli_peak_tp_txop_enable) 
		{
			disable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, TXOP_FE);
			ad->peak_tp_ctl.cli_peak_tp_txop_enable = FALSE;
		}
	}
	
ignore_5g_ampdu_efficiency_check:
	//restore aifs adjust since dynamic txop owner is not peak throughput
	if (ad->peak_tp_ctl.cli_ampdu_efficiency_running) 
	{
		if (query_tx_burst_prio(ad, wdev) > PRIO_PEAK_TP)
		{
			edcaparam = HcGetEdca(ad, ad->peak_tp_ctl.cur_wdev);
			if (edcaparam)
				AsicAmpduEfficiencyAdjust(ad->peak_tp_ctl.cur_wdev,edcaparam->Aifsn[0]);
			ad->peak_tp_ctl.cli_ampdu_efficiency_running = FALSE;
		}
	}
	
	/* clear some record */
	ad->peak_tp_ctl.client_nums = 0;
	ad->peak_tp_ctl.max_tx_tp = 0;
}
static VOID dynamic_ampdu_efficiency_adjust_2G(struct _RTMP_ADAPTER *ad)
{
	EDCA_PARM *edcaparam;
	ULONG per = 0;
	ULONG tx_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;
	
	if (ad->peak_tp_ctl.client_nums_2g == 0)
		goto ignore_2g_ampdu_efficiency_check;

	if (ad->CommonCfg.dbdc_mode)
		wlan_ct = &ad->WlanCounters[0];
	else
		wlan_ct = &ad->WlanCounters[0];

	wdev = ad->peak_tp_ctl.cur_wdev_2g;

	tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;
	if (tx_cnt > 0) 
	{
		per = 100*(wlan_ct->AmpduFailCount.u.LowPart)/(wlan_ct->AmpduFailCount.u.LowPart+tx_cnt);
	}
	if (per >= DBDC_PEAK_TP_PER_THRESHOLD) 
	{// do no apply patch, it is in noise environment
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]per=%lu\n", __func__,__LINE__, per));
		goto ignore_2g_ampdu_efficiency_check;
	}
	
	/* scenario detection */
	if (ad->peak_tp_ctl.max_tx_2g_tp > DBDC_AP_2G_PEAK_TP)
	{
		ad->peak_tp_ctl.cli_peak_tp_2g_running = 1;
	} 
	else
	{
		ad->peak_tp_ctl.cli_peak_tp_2g_running = 0;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]per=%lu,tx=%d M,(%d,%d,%d)\n\r", 
			 __func__,__LINE__,per,ad->peak_tp_ctl.max_tx_2g_tp,
			 ad->peak_tp_ctl.cli_peak_tp_2g_running,
			 ad->peak_tp_ctl.cli_2g_ampdu_efficiency_running,
			 ad->peak_tp_ctl.cli_2g_peak_tp_txop_enable));

	/* increase ampdu efficiency if running peak T.P */
	if (ad->peak_tp_ctl.cli_peak_tp_2g_running)
	{
		if (!ad->peak_tp_ctl.cli_2g_ampdu_efficiency_running) 
		{
			if (query_tx_burst_prio(ad, wdev) <= PRIO_PEAK_TP)
			{
				AsicAmpduEfficiencyAdjust(wdev,0xf);
				ad->peak_tp_ctl.cli_2g_ampdu_efficiency_running = TRUE;
			}
		}
		if (!ad->peak_tp_ctl.cli_2g_peak_tp_txop_enable) 
		{
			enable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, TXOP_FE);
			ad->peak_tp_ctl.cli_2g_peak_tp_txop_enable = TRUE;
		}
	}
	else
	{//restore to original
		if (ad->peak_tp_ctl.cli_2g_ampdu_efficiency_running) 
		{
			edcaparam = HcGetEdca(ad, wdev);
			if (edcaparam)
				AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);
			ad->peak_tp_ctl.cli_2g_ampdu_efficiency_running = FALSE;
		}
		if (ad->peak_tp_ctl.cli_2g_peak_tp_txop_enable) 
		{
			disable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, TXOP_FE);
			ad->peak_tp_ctl.cli_2g_peak_tp_txop_enable = FALSE;
		}
	}
	
ignore_2g_ampdu_efficiency_check:
	//restore aifs adjust since dynamic txop owner is not peak throughput
	if (ad->peak_tp_ctl.cli_2g_ampdu_efficiency_running) 
	{
		if (query_tx_burst_prio(ad, wdev) > PRIO_PEAK_TP)
		{
			edcaparam = HcGetEdca(ad, wdev);
			if (edcaparam)
				AsicAmpduEfficiencyAdjust(wdev,edcaparam->Aifsn[0]);
			ad->peak_tp_ctl.cli_2g_ampdu_efficiency_running = FALSE;
		}
	}
	
	/* clear some record */
	ad->peak_tp_ctl.client_nums_2g = 0;
	ad->peak_tp_ctl.max_tx_2g_tp = 0;
}
static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad)
{
	/* only work with dbdc mode, and one sta connected*/
	if (!ad->CommonCfg.dbdc_mode)
	{
		ad->peak_tp_ctl.client_nums_2g = 0;
		ad->peak_tp_ctl.max_tx_2g_tp = 0;
		ad->peak_tp_ctl.client_nums = 0;
		ad->peak_tp_ctl.max_tx_tp = 0;
		return;
	}
	dynamic_ampdu_efficiency_adjust_2G(ad);
	dynamic_ampdu_efficiency_adjust_5G(ad);
}

#ifdef DOT1X_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_InternalCmdAction(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN UINT8 cmd)
{
	INT				apidx = MAIN_MBSSID;
	UCHAR 			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	UCHAR			s_addr[MAC_ADDR_LEN];
	UCHAR			EAPOL_IE[] = {0x88, 0x8e};
#ifdef RADIUS_ACCOUNTING_SUPPORT
	DOT1X_QUERY_STA_DATA	data;
#define FRAME_BUF_LEN 	LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN + sizeof(DOT1X_QUERY_STA_DATA)
#else
#define FRAME_BUF_LEN 	LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
	UINT8			frame_len = 0; 
	UCHAR			FrameBuf[FRAME_BUF_LEN];
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT			TCI;

	/* Init the frame buffer */
	NdisZeroMemory(FrameBuf, frame_len);

#ifdef RADIUS_ACCOUNTING_SUPPORT
	NdisMoveMemory(data.StaAddr, pEntry->Addr, MAC_ADDR_LEN);
	data.rx_bytes = pEntry->RxBytes;
	data.tx_bytes = pEntry->TxBytes;
	data.rx_packets = pEntry->RxPackets.u.LowPart;
	data.tx_packets = pEntry->TxPackets.u.LowPart;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::rx_byte:%lu  tx_byte:%lu rx_pkt:%lu tx_pkt:%lu\n",
						__FUNCTION__, data.rx_bytes, data.tx_bytes, data.rx_packets, data.rx_packets ));
#endif /* RADIUS_ACCOUNTING_SUPPORT */

	if (pEntry)
	{
		apidx = pEntry->func_tb_idx;
		NdisMoveMemory(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	}
	else
	{
		/* Fake a Source Address for transmission */
		NdisMoveMemory(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
		s_addr[0] |= 0x80;
	}

	/* Assign internal command for Ralink dot1x daemon */
	RalinkIe[5] = cmd;
	
	if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID)
	{
		bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
		bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
		frame_len = FRAME_BUF_LEN;

		MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid,s_addr,EAPOL_IE);
		offset += LENGTH_802_3 - 2;
		NdisMoveMemory((FrameBuf + offset), TPID, 2);
		offset += 2;
		TCI = (bss_Vlan & 0x0FFF) | ((bss_Vlan_Priority & 0x7)<<13);
#ifndef		RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif
		*(USHORT*)(FrameBuf + offset) = TCI;
		offset += 2;
		NdisMoveMemory((FrameBuf + offset), EAPOL_IE, 2);
		offset += 2;
	}
	else
	{
		frame_len = FRAME_BUF_LEN - VLAN_HDR_LEN;
		/* Prepare the 802.3 header */
		MAKE_802_3_HEADER(FrameBuf,
						  pAd->ApCfg.MBSSID[apidx].wdev.bssid,
						  s_addr,
						  EAPOL_IE);
		offset += LENGTH_802_3;
	}
	
	/* Prepare the specific header of internal command */
	NdisMoveMemory(&FrameBuf[offset], RalinkIe, sizeof(RalinkIe));

#ifdef RADIUS_ACCOUNTING_SUPPORT
	offset += sizeof(RalinkIe);
	
	/*add accounting info*/
	NdisMoveMemory(&FrameBuf[offset], (unsigned char *)&data, sizeof(DOT1X_QUERY_STA_DATA));
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
	/* Report to upper layer */
	if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
		return FALSE;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s done. (cmd=%d)\n", __FUNCTION__, cmd));

	return TRUE;
}


/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to trigger 802.1x EAP state machine.

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	// TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx
	INT				apidx = MAIN_MBSSID;
	UCHAR 			eapol_start_1x_hdr[4] = {0x01, 0x01, 0x00, 0x00};
	UINT8			frame_len = LENGTH_802_3 + sizeof(eapol_start_1x_hdr);
	UCHAR			FrameBuf[frame_len+32];
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT 			TCI;

	if(IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(&pAd->ApCfg.MBSSID[apidx].wdev))
	{
		/* Init the frame buffer */
		NdisZeroMemory(FrameBuf, frame_len);

		/* Assign apidx */
		apidx = pEntry->func_tb_idx;
		
		if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID)
		{	
			/*Prepare 802.3 header including VLAN tag*/
			bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
			bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
			frame_len += VLAN_HDR_LEN ;

			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL)
			offset += LENGTH_802_3 - 2;

			NdisMoveMemory((FrameBuf + offset), TPID , 2);
			offset += 2;

			TCI = (bss_Vlan & 0x0fff) | ((bss_Vlan_Priority & 0x7)<<13);

#ifndef RT_BIG_ENDIAN
			TCI = SWAP16(TCI);
#endif
			*(USHORT*)(FrameBuf + offset) = TCI;
			offset += 2;

			NdisMoveMemory((FrameBuf + offset), EAPOL ,2);
			offset += 2;
		} 
		else 
		{
			/* Prepare the 802.3 header */
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL);
			offset += LENGTH_802_3;
		}
		
		/* Prepare a fake eapol-start body */
		NdisMoveMemory(&FrameBuf[offset], eapol_start_1x_hdr, sizeof(eapol_start_1x_hdr));

#ifdef CONFIG_HOTSPOT_R2
		if (pEntry)
		{
        	BSS_STRUCT *pMbss = pEntry->pMbss;
			if ((pMbss->HotSpotCtrl.HotSpotEnable == 1) && (IS_AKM_WPA2_Entry(&pMbss->wdev)) && (pEntry->hs_info.ppsmo_exist == 1))
			{
            	UCHAR HS2_Header[4] = {0x50,0x6f,0x9a,0x12};
				memcpy(&FrameBuf[offset+sizeof(eapol_start_1x_hdr)], HS2_Header, 4);
				memcpy(&FrameBuf[offset+sizeof(eapol_start_1x_hdr)+4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
				frame_len += 4+sizeof(struct _sta_hs_info);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DOT1X_EapTriggerAction: event eapol start, %x:%x:%x:%x\n",
						FrameBuf[offset+sizeof(eapol_start_1x_hdr)+4],FrameBuf[offset+sizeof(eapol_start_1x_hdr)+5],
						FrameBuf[offset+sizeof(eapol_start_1x_hdr)+6],FrameBuf[offset+sizeof(eapol_start_1x_hdr)+7]));
            }
		}
#endif
		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
			return FALSE;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Notify 8021.x daemon to trigger EAP-SM for this sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));

	}

	return TRUE;
}

#endif /* DOT1X_SUPPORT */


INT rtmp_ap_init(RTMP_ADAPTER *pAd)
{
#ifdef WSC_AP_SUPPORT
    UCHAR j;
    BSS_STRUCT *mbss = NULL;
    struct wifi_dev *wdev = NULL;
    PWSC_CTRL pWscControl;

    for(j = BSS0; j < pAd->ApCfg.BssidNum; j++)
    {
        mbss = &pAd->ApCfg.MBSSID[j];
        wdev = &pAd->ApCfg.MBSSID[j].wdev;
        {
            pWscControl = &mbss->WscControl;

            pWscControl->WscRxBufLen = 0;
            pWscControl->pWscRxBuf = NULL;
            os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MGMT_DMA_BUFFER_SIZE);
            if (pWscControl->pWscRxBuf)
                NdisZeroMemory(pWscControl->pWscRxBuf, MGMT_DMA_BUFFER_SIZE);
            pWscControl->WscTxBufLen = 0;
            pWscControl->pWscTxBuf = NULL;
            os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MGMT_DMA_BUFFER_SIZE);
            if (pWscControl->pWscTxBuf)
                NdisZeroMemory(pWscControl->pWscTxBuf, MGMT_DMA_BUFFER_SIZE);
        }
    }
#endif /* WSC_AP_SUPPORT */

	APOneShotSettingInitialize(pAd);

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("apstart up %02x:%02x:%02x:%02x:%02x:%02x\n",
                PRINT_MAC(pAd->CurrentAddress)));

	APStartUpForMain(pAd);

	MlmeRadioOn(pAd, &pAd->ApCfg.MBSSID[BSS0].wdev);


	/* Set up the Mac address*/
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], NULL);
//	ap_func_init(pAd);

	if (pAd->chipCap.hif_type == HIF_MT)
	{
		/* Now Enable MacRxTx*/
		RTMP_IRQ_ENABLE(pAd);
		RTMPEnableRxTx(pAd);
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}

#ifdef CFG_SUPPORT_MU_MIMO_RA
	/*Send In-Band Command to N9 about Platform Type 7621/7623*/
	SetMuraPlatformTypeProc(pAd);
#endif

#ifdef MT_FDB
	fdb_enable(pAd);
#endif /* MT_FDB */

	return NDIS_STATUS_SUCCESS;
}


 VOID rtmp_ap_exit(RTMP_ADAPTER *pAd)
{
	
	BOOLEAN Cancelled;
	
	RTMPReleaseTimer(&pAd->ApCfg.CounterMeasureTimer,&Cancelled);
	
#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPReleaseTimer(&pAd->ApCfg.IDSTimer,&Cancelled);
#endif /* IDS_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	RTMP_11N_D3_TimerRelease(pAd);

#endif /*DOT11N_DRAFT3*/
#endif /*DOT11_N_SUPPORT*/

	/* Free BssTab & ChannelInfo tabbles.*/
	AutoChBssTableDestroy(pAd);
	ChannelInfoDestroy(pAd);

#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */
}


