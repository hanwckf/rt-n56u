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
    cmm_radar.c

    Abstract:
    CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#include "rt_config.h"

/*----- 802.11H -----*/

/* Periodic Radar detection, switch channel will occur in RTMPHandleTBTTInterrupt()*/
/* Before switch channel, driver needs doing channel switch announcement.*/
VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR Channel)
{
	INT i, ChIdx = 0, bAnyUnavailableChannel = FALSE;
	/* 
		1. APStart(), CalBufTime = 0;
		2. if bAnyUnavailableChannel, CalBufTime = DEFAULT_CAL_BUF_TIME;
		3. if Calibrated, CalBufTime = DEFAULT_CAL_BUF_TIME_MAX;
	*/
	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].RemainingTimeForUse != 0)
		{
			bAnyUnavailableChannel = TRUE;
		}

		if (Channel== pAd->ChannelList[i].Channel)
		{
			ChIdx = i;
		}
	}

	if (bAnyUnavailableChannel)
		pAd->Dot11_H.CalBufTime = DEFAULT_CAL_BUF_TIME;

	if (pAd->Dot11_H.RDMode == RD_SILENCE_MODE)
	{
		/* In Silent  Mode, RDCount is use to check with the CAC Time */
	if (pAd->Dot11_H.RDCount++ > pAd->Dot11_H.ChMovingTime &&
		pAd->ChannelList[ChIdx].RemainingTimeForUse == 0)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Not found radar signal, start send beacon and radar detection in service monitor\n\n"));
		pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
		AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
			pAd->Dot11_H.RDCount = 0;
		}
	}
	else if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
	{
		if (!bAnyUnavailableChannel)
		{
			pAd->Dot11_H.RDCount++;
			/* In Normal Mode, RDCount is use to check with the CalBufTime */
			if ((pAd->Dot11_H.RDCount >= pAd->Dot11_H.CalBufTime))
			{
				pAd->Dot11_H.CalBufTime = DEFAULT_CAL_BUF_TIME_MAX;
			}
		}
	}
}

/*
	========================================================================

	Routine Description:
		Radar channel check routine

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		TRUE	need to do radar detect
		FALSE	need not to do radar detect

	========================================================================
*/
BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT 	i;
	BOOLEAN result = FALSE;
	UCHAR phy_bw = decide_phy_bw_by_channel(pAd,Ch);//HcGetBwByRf(pAd,RFIC_5GHZ);

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (Ch == pAd->ChannelList[i].Channel)
		{
			result = pAd->ChannelList[i].DfsReq;
			break;
		}
	}

#ifdef DOT11_VHT_AC
		if((phy_bw == BW_160) && (Ch >= 36 && Ch <= 48))
			return TRUE;		
#endif

	return result;
}


/*
	========================================================================

	Routine Description:
		Determine the current radar state

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:

	========================================================================
*/
VOID RadarStateCheck(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev)
{
#ifdef MT_DFS_SUPPORT
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd,RFIC_5GHZ);
#endif	
	if(wdev->csa_count != 0)
		return;
#ifdef MT_DFS_SUPPORT	
	if ( IsSupport5G &&
	(pAd->CommonCfg.bIEEE80211H == 1) &&
	DfsRadarChannelCheck(pAd)
#ifdef BACKGROUND_SCAN_SUPPORT         
	&&((IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) == FALSE) ||(CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT)))    
#endif /* BACKGROUND_SCAN_SUPPORT */        
	)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]Set into RD_SILENCE_MODE! \x1b[m \n",
								     __FUNCTION__));
		pAd->Dot11_H.RDMode = RD_SILENCE_MODE;
		pAd->Dot11_H.RDCount = 0;
		pAd->Dot11_H.InServiceMonitorCount = 0;
		if(IS_OUTBAND_ABAILABLE(pAd))
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] OutBand Available. Set into RD_NORMAL_MODE \x1b[m \n",__FUNCTION__));
			pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
		}	
	}
	else
#endif
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]Set into RD_NORMAL_MODE \x1b[m \n",__FUNCTION__));
	
		/* DFS Zero wait case, OP CH always is normal mode */
		pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
	}
}

BOOLEAN CheckNonOccupancyChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	INT i; 

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
		{
			if (pAd->ChannelList[i].RemainingTimeForUse > 0)
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("ERROR: previous detection of a radar on this channel(Channel=%d).\n",
				pAd->ChannelList[i].Channel));
				return FALSE;
			}
		}
	}
	
#ifdef MT_DFS_SUPPORT
	return DfsDedicatedCheckChBwValid(pAd, channel, decide_phy_bw_by_channel(pAd, channel));
#endif	
	return TRUE;
}

USHORT CheckLargestNOP( 
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i; 
	USHORT Largest = 0;

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	for (i = 0; i < pAd->ChannelListNum; i++)
	{        
		if (pAd->ChannelList[i].RemainingTimeForUse > Largest)
		{
			Largest = pAd->ChannelList[i].RemainingTimeForUse;
		}        
	}  

	return Largest;
}

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd,RFIC_5GHZ);
	UCHAR Channel5G = HcGetChannelByRf(pAd,RFIC_5GHZ);

	if (pAd->CommonCfg.RDDurRegion != JAP)
	{
		return pAd->CommonCfg.RDDurRegion;
	}

	for (i=0; i<15; i++)
	{
		if (IsSupport5G && Channel5G== Channel[i])
		{
			break;
		}
	}

	if (i < 4)
		return JAP_W53;
	else if (i < 15)
		return JAP_W56;
	else
		return JAP; /* W52*/

}


UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode)
{
	UCHAR ch = 0;
	INT ch_idx;
	
#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	switch (mode)
	{
		case 1: 
		{
			USHORT min_time = 0xFFFF;
			/* select channel with least RemainingTimeForUse */
			for ( ch_idx = 0; ch_idx <  pAd->ChannelListNum; ch_idx++)
			{
				if (pAd->ChannelList[ch_idx].RemainingTimeForUse < min_time)
				{
					min_time = pAd->ChannelList[ch_idx].RemainingTimeForUse;
					ch = pAd->ChannelList[ch_idx].Channel;
				}
			}
			break;
		}

		default:
		{
			ch = FirstChannel(pAd);
			break;
		}
	}	

    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE,("%s(): mode = %u, ch = %u\n",
							 __FUNCTION__, mode, ch));
	return ch;
}
		

#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Channel switching count down process upon radar detection

	Arguments:
		pAd 	Pointer to our adapter

	========================================================================
*/
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd)
{
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s():Channel Switching...(%d/%d)\n",
				__FUNCTION__, pAd->Dot11_H.CSCount, pAd->Dot11_H.CSPeriod));
	
	pAd->Dot11_H.CSCount++;
	if (pAd->Dot11_H.CSCount >= pAd->Dot11_H.CSPeriod)
	{
#ifdef DFS_SUPPORT
		pAd->CommonCfg.RadarDetect.DFSAPRestart = 1;
		schedule_dfs_task(pAd);
#else
		RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, NULL, 0);	

#endif /* !DFS_SUPPORT */		
	}
}

/*
*
*/
NTSTATUS Dot11HCntDownTimeoutAction(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
    BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
    if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
    {
        if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC))
        {            
            /* After ChSwAnn:
               1. Zero wait(InservMonitor)->detected radar->uniform Ch is DFS Ch -> Zero wait(CAC) 
               2. CAC period -> set DFS CH -> ZeroWait Stop ->ZeroWait Start 
               3. Idle/InServ -> set DFS CH */
#ifdef DFS_DBG_LOG_3               
            MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Trigger DFS Zero wait:ZeroWaitCh=%d\n", 
                                                                  __FUNCTION__,
                                                                  BgndScanCtrl->DfsZeroWaitChannel));
#endif           

            pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
            APStopByRf(pAd, RFIC_5GHZ);            
            APStartUpByRf(pAd, RFIC_5GHZ);                                  
        }
        else if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT))
        {    /* Zero wait CAC timeout case */
#ifdef DFS_DBG_LOG_3        
            MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][ZeroWait]DFS_OFF_CHNL_CAC_TIMEOUT\n",__FUNCTION__));
#endif
#ifdef MAC_REPEATER_SUPPORT
            //Disable DFS zero wait support  for repeater mode dynamic enable
            if (pAd->ApCfg.bMACRepeaterEn)
            {
                BgndScanCtrl->DfsZeroWaitSupport = FALSE;
                UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
                UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
            }
#endif /* MAC_REPEATER_SUPPORT */

            pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
            APStopByRf(pAd, RFIC_5GHZ);
            APStartUpByRf(pAd, RFIC_5GHZ);
        }
        else
        {
            /* Zero wait(InservMonitor)->detected radar->uniform Ch is Non-DFS Ch */
#ifdef DFS_DBG_LOG_3	    
            MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][ZeroWait]AP start on NonDFS CH\n",__FUNCTION__));
#endif            
            //Disable DFS zero wait support  for repeater mode dynamic enable
#ifdef MAC_REPEATER_SUPPORT
            if (pAd->ApCfg.bMACRepeaterEn)
            {
                BgndScanCtrl->DfsZeroWaitSupport = FALSE;
                UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);               
            }
#endif/*MAC_REPEATER_SUPPORT */			
            UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
            pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
            APStopByRf(pAd, RFIC_5GHZ);            
            APStartUpByRf(pAd, RFIC_5GHZ);
        }
    }
	else
#endif /* MT_DFS_SUPPORT && BACKGROUND_SCAN_SUPPORT*/	        
	{
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)		
		DedicatedZeroWaitStop(pAd);
#endif
		pAd->Dot11_H.RDMode = RD_SILENCE_MODE;	

		APStopByRf(pAd, RFIC_5GHZ);
		
#ifdef MT_DFS_SUPPORT
		if(DfsStopWifiCheck(pAd))
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n",__FUNCTION__));
				return 0;  
        	}
#endif

		APStartUpByRf(pAd, RFIC_5GHZ);	

#ifdef MT_DFS_SUPPORT
		if (pAd->CommonCfg.dbdc_mode)
		{
#ifdef DFS_DBG_LOG_0
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] SetDfsTxStart\n", __FUNCTION__));
#endif
			MtCmdSetDfsTxStart(pAd, DBDC_BAND1);
		}
		else
		{
			MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
		}
		DfsSetClass2DeauthDisable(pAd, FALSE);
		
		DfsSetCacRemainingTime(pAd);

#ifdef BACKGROUND_SCAN_SUPPORT
		if(IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd)
		&& !DfsDedicatedCheckChBwValid(pAd, GET_BGND_PARAM(pAd, ORI_INBAND_CH), GET_BGND_PARAM(pAd, ORI_INBAND_BW)))
		{
			ZeroWait_DFS_collision_report(pAd, HW_RDD0, 
			GET_BGND_PARAM(pAd, ORI_INBAND_CH), GET_BGND_PARAM(pAd, ORI_INBAND_BW));
	
		}
#endif		
		/*DfsDedicatedScanStart(pAd);*/		
#endif		    
	} 

#ifdef MT_DFS_SUPPORT	
        if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) &&
            (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) ||
             CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE)))
        {    	      
            {
                /* DFS zero wait - Previous state is InServMontor need to recover TX queues and disable zero handoff  */
                // 1.Dfs(Inserv) -> Radar detected -> Uniform Ch is Non-DFS Ch (Idle_o)
                // 2.Dfs(Inserv) -> Radar detected -> Uniform Ch is DFS Ch (CAC_o)
                // 3.Dfs(CAC)    -> set DFS CH -> ZeroWait Start (CAC_x)
                // 4.Dfs(Idle/Inserv) -> set DFS CH -> ZeroWait Start (CAC_x)            
               // 5.DFS(Idle/Inserv) -> set Non-DFS CH (Idle_x) 
            
#ifdef BACKGROUND_SCAN_SUPPORT             
                if (!BgndScanCtrl->RadarDetected && IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)) //New add 2016/03/03
                {
#ifdef DFS_DBG_LOG_0                
                    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Bypass ZeroWait case3~5 to do DfsTxStart\n", __FUNCTION__));
#endif
		}
                else
#endif /* BACKGROUND_SCAN_SUPPORT */
               {            
                    if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
                    {
#ifdef BACKGROUND_SCAN_SUPPORT                
                        BgndScanCtrl->RadarDetected = FALSE;
#endif
                    }
    
                    MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
#ifdef DFS_DBG_LOG_0
                    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Do SetDfsTxStart\n", __FUNCTION__));
#endif
		}
            }
	}
	else
	{
#ifdef DFS_DBG_LOG_0	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]ZeroWait don't SetDfsTxStart(State=%d) \n", 
                                                         __FUNCTION__, pAd->DfsParameter.ZeroWaitDfsState));    
#endif
	}
#endif /* MT_DFS_SUPPORT */  
	return 0;
}

#endif /* CONFIG_AP_SUPPORT */




/* 
    ==========================================================================
    Description:
        Set channel switch Period
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_STRING *arg)
{
	pAd->Dot11_H.CSPeriod = (USHORT) simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->Dot11_H.CSPeriod));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
		change channel moving time for DFS testing.

	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	USHORT Value;

	Value = (USHORT) simple_strtol(arg, 0, 10);

	pAd->Dot11_H.ChMovingTime = Value;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s: %d\n", __FUNCTION__,
		pAd->Dot11_H.ChMovingTime));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
		Reset channel block status.
	Arguments:
	    pAd				Pointer to our adapter
	    arg				Not used

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_BlockChReset_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	INT i;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s: Reset channel block status.\n", __FUNCTION__));	
	
	for (i=0; i<pAd->ChannelListNum; i++)
		pAd->ChannelList[i].RemainingTimeForUse = 0;

	return TRUE;
}


#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)

INT	Set_RadarShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_STRING *arg)
{
#ifdef DFS_SUPPORT
	UINT8 idx;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;
	PCHAR RDMode[]= {"Normal State", "Switching State", "Silent State"};

		printk("DFSUseTasklet = %d\n", pRadarDetect->use_tasklet);
		printk("McuRadarDebug = %x\n", (unsigned int)pRadarDetect->McuRadarDebug);
		printk("PollTime = %d\n", pRadarDetect->PollTime);
		printk("ChEnable = %d (0x%x)\n", pDfsProgramParam->ChEnable, pDfsProgramParam->ChEnable);
		printk("DeltaDelay = %d\n", pDfsProgramParam->DeltaDelay);
		printk("PeriodErr = %d\n", pDfsSwParam->dfs_period_err);
		printk("MaxPeriod = %d\n", (unsigned int)pDfsSwParam->dfs_max_period);
		printk("Ch0LErr = %d\n", pDfsSwParam->dfs_width_ch0_err_L);
		printk("Ch0HErr = %d\n", pDfsSwParam->dfs_width_ch0_err_H);
		printk("Ch1Shift = %d\n", pDfsSwParam->dfs_width_diff_ch1_Shift);
		printk("Ch2Shift = %d\n", pDfsSwParam->dfs_width_diff_ch2_Shift);
		printk("DfsRssiHigh = %d\n", pRadarDetect->DfsRssiHigh);
		printk("DfsRssiLow = %d\n", pRadarDetect->DfsRssiLow);
		printk("DfsSwDisable = %u\n", pRadarDetect->bDfsSwDisable);
		printk("CheckLoop = %d\n", pDfsSwParam->dfs_check_loop);
		printk("DeclareThres = %d\n", pDfsSwParam->dfs_declare_thres);
		for (idx=0; idx < pAd->chipCap.DfsEngineNum; idx++)
			printk("sw_idx[%u] = %u\n", idx, pDfsSwParam->sw_idx[idx]);
		for (idx=0; idx < pAd->chipCap.DfsEngineNum; idx++)
			printk("hw_idx[%u] = %u\n", idx, pDfsSwParam->hw_idx[idx]);

	printk("pAd->Dot11_H.ChMovingTime = %d\n", pAd->Dot11_H.ChMovingTime);
	printk("pAd->Dot11_H.RDMode = %s\n", RDMode[pAd->Dot11_H.RDMode]);
	printk("pAd->Dot11_H.RDCount = %d\n", pAd->Dot11_H.RDCount);
	printk("pAd->Dot11_H.CalBufTime = %d\n", pAd->Dot11_H.CalBufTime);
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	printk("pAd->CommonCfg.CarrierDetect.CD_State = %d\n", pAd->CommonCfg.CarrierDetect.CD_State);
	printk("pAd->CommonCfg.CarrierDetect.criteria = %d\n", pAd->CommonCfg.CarrierDetect.criteria);
	printk("pAd->CommonCfg.CarrierDetect.Delta = %d\n", pAd->CommonCfg.CarrierDetect.delta);
	printk("pAd->CommonCfg.CarrierDetect.DivFlag = %d\n", pAd->CommonCfg.CarrierDetect.div_flag);
	printk("pAd->CommonCfg.CarrierDetect.Threshold = %d(0x%x)\n", pAd->CommonCfg.CarrierDetect.threshold, pAd->CommonCfg.CarrierDetect.threshold);
#endif /* CARRIER_DETECTION_SUPPORT */

	return TRUE;
}

/*
	========================================================================
       Routine Description:
               Control CCK_MRC Status
       Arguments:
               pAd     Pointer to our adapter
       Return Value:

       ========================================================================
*/
VOID CckMrcStatusCtrl(IN PRTMP_ADAPTER pAd)
{
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd,RFIC_5GHZ);
	BOOLEAN IsSupport2G = HcIsRfSupport(pAd,RFIC_24GHZ);
	
}


/*
       ========================================================================
       Routine Description:
               Enhance DFS/CS when using GLRT.
       Arguments:
               pAd     Pointer to our adapter
       Return Value:

       ========================================================================
*/
VOID RadarGLRTCompensate(IN PRTMP_ADAPTER pAd)
{
}
#endif /*defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) */

