#ifdef MTK_LICENSE
/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    cmm_rdm_mt.c//Jelly20140123
*/
#endif /* MTK_LICENSE */
#ifdef MT_DFS_SUPPORT
//Remember add RDM compiler flag - Shihwei20141104
#include "rt_config.h"
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/



/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S 
********************************************************************************
*/

/*#ifdef MT_DFS_SUPPORT
	BUILD_TIMER_FUNCTION(DfsChannelSwitchTimeOut);
#endif */
static inline BOOLEAN AutoChannelSkipListCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;
	
	for (i=0; i < pAd->ApCfg.AutoChannelSkipListNum ; i++)
	{
		if (Ch == pAd->ApCfg.AutoChannelSkipList[i])
		{
			return TRUE;
		}
	}
	return FALSE;
}

static inline UCHAR Get5GPrimChannel(
    IN PRTMP_ADAPTER	pAd)
{
    
	return HcGetChannelByRf(pAd,RFIC_5GHZ);
	
}

static inline VOID Set5GPrimChannel(
    IN PRTMP_ADAPTER	pAd, UCHAR Channel)
{
	if(HcUpdateChannel(pAd,Channel) !=0)
	{
		;
	}
}

static inline UCHAR CentToPrim(
	UCHAR Channel)
{	
	return Channel-2;
}

static inline BOOLEAN ByPassChannelByBw(
	UCHAR Channel, UCHAR Bw, UCHAR RDDurRegion)
{	
	if(RDDurRegion == FCC) {

		if (Bw == BW_40)
			return ((Channel == 116) || (Channel == 140) || (Channel == 165));
		if ((Bw == BW_80) || (Bw == BW_8080))
			return ((Channel == 116) || (Channel == 132) || (Channel == 136) || (Channel == 140) || (Channel == 165));
		if (Bw == BW_160)
			return ((Channel == 116) || (Channel >= 132));
	}
	else {
		
		if (Bw == BW_40)
			return ((Channel == 140) || (Channel == 165));
		if ((Bw == BW_80) || (Bw == BW_8080))
			return ((Channel == 132) || (Channel == 136) || (Channel == 140) || (Channel == 165));
		if (Bw == BW_160)
			return  (Channel >= 132);
	}
	
	return FALSE;
}


VOID DfsGetSysParameters(
    IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR phy_bw = decide_phy_bw_by_channel(pAd,pDfsParam->Band0Ch); //HcGetBwByRf(pAd,RFIC_5GHZ);
	UINT_8 i;
	
#ifdef DOT11_VHT_AC		
    
	if(phy_bw == BW_8080)
    {
        pDfsParam->PrimCh = Get5GPrimChannel(pAd);	
        if(Get5GPrimChannel(pAd) < CentToPrim(pAd->CommonCfg.vht_cent_ch2))
        { 
			pDfsParam->PrimBand = RDD_BAND0;
        }	
		else
		{	
			pDfsParam->PrimBand = RDD_BAND1;
		}	
		pDfsParam->Band0Ch = (pDfsParam->PrimBand == RDD_BAND0) ? Get5GPrimChannel(pAd) : CentToPrim(pAd->CommonCfg.vht_cent_ch2);
        pDfsParam->Band1Ch = (pDfsParam->PrimBand == RDD_BAND0) ? CentToPrim(pAd->CommonCfg.vht_cent_ch2) : Get5GPrimChannel(pAd);        
    }
	else
#endif
    {   
#ifdef BACKGROUND_SCAN_SUPPORT     
        if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) && 
            (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) ||
             CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INIT_CAC) ||
             CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_RADAR_DETECT) ||
             CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INSERV_MONI)))
        {
         
            pDfsParam->Band0Ch = pAd->BgndScanCtrl.DfsZeroWaitChannel;

            pDfsParam->PrimBand = RDD_BAND0;
            pDfsParam->Band1Ch= 0;
        }
        else
#endif /* BACKGROUND_SCAN_SUPPORT  */            
        {        
            pDfsParam->PrimCh = Get5GPrimChannel(pAd);
	        pDfsParam->PrimBand = RDD_BAND0;
	        pDfsParam->Band0Ch= Get5GPrimChannel(pAd);
	        pDfsParam->Band1Ch= 0;
        }
    }

	pDfsParam->Bw = phy_bw;
	pDfsParam->Dot11_H.RDMode = pAd->Dot11_H.RDMode;
	pDfsParam->bIEEE80211H = pAd->CommonCfg.bIEEE80211H;
	pDfsParam->ChannelListNum = pAd->ChannelListNum;
	pDfsParam->bDBDCMode = pAd->CommonCfg.dbdc_mode;
	pDfsParam->bDfsEnable = pAd->CommonCfg.DfsParameter.bDfsEnable;	
	pDfsParam->RDDurRegion = pAd->CommonCfg.RDDurRegion;
		
	for(i=0; i<pDfsParam->ChannelListNum; i++)
	{
   	    pDfsParam->DfsChannelList[i].Channel = pAd->ChannelList[i].Channel;		
        pDfsParam->DfsChannelList[i].Flags = pAd->ChannelList[i].Flags;
		pDfsParam->DfsChannelList[i].NonOccupancy = pAd->ChannelList[i].RemainingTimeForUse;
	}

}

VOID DfsParamUpdateChannelList(
	IN PRTMP_ADAPTER	pAd)
{
	UINT_8 i;
	for(i=0; i < pAd->ChannelListNum; i++)
		{
			pAd->ChannelList[i].RemainingTimeForUse = 0;
		}
}

VOID DfsParamInit(
		IN PRTMP_ADAPTER	pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->PrimBand = RDD_BAND0;
	pDfsParam->DfsChBand[0] = FALSE; //Smaller channel
	pDfsParam->DfsChBand[1] = FALSE; // Larger channel number
	pDfsParam->RadarDetected[0] = FALSE; //Smaller channel number
	pDfsParam->RadarDetected[1] = FALSE; // larger channel number
	pDfsParam->RadarDetectState = FALSE;
	pDfsParam->IsSetCountryRegion = FALSE;
	pDfsParam->bDfsCheck = FALSE;
	pDfsParam->bShowPulseInfo = FALSE;
	pDfsParam->bNoAvailableCh = FALSE;
	pDfsParam->RddCtrlState = RDD_DEFAULT;
	pDfsParam->bZeroWaitCacSecondHandle = FALSE;
	pDfsParam->bDBDCMode = pAd->CommonCfg.dbdc_mode;
	pAd->Dot11_H.DfsZeroWaitChMovingTime = 3;
	pDfsParam->bDfsIsScanRunning = FALSE;
	pDfsParam->DetRateMode = NO_CH_SWITCH;
	pDfsParam->bClass2DeauthDisable = FALSE;	
	DfsStateMachineInit(pAd, &pAd->CommonCfg.DfsParameter.DfsStatMachine, pAd->CommonCfg.DfsParameter.DfsStateFunc);
}

VOID DfsStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, DFS_MAX_STATE, DFS_MAX_MSG, (STATE_MACHINE_FUNC)Drop, DFS_BEFORE_SWITCH, DFS_MACHINE_BASE);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_CAC_END, (STATE_MACHINE_FUNC)DfsCacEndUpdate);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_CHAN_SWITCH_TIMEOUT, (STATE_MACHINE_FUNC)DfsChannelSwitchTimeoutAction);
	StateMachineSetAction(Sm, DFS_INSERVICE_MONITOR, DFS_TP_LOW_HIGH, (STATE_MACHINE_FUNC)WrapDfsRadarDetectStop);
	StateMachineSetAction(Sm, DFS_INSERVICE_MONITOR, DFS_TP_HIGH_LOW, (STATE_MACHINE_FUNC)DfsTXTPHighToLow);

}

INT Set_RadarDetectStart_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg)
{
	ULONG value, ret1, ret2;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR phy_bw = HcGetBwByRf(pAd,RFIC_5GHZ);
	value = simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStart_Proc: \n"));	

	if(value == 0)
	{	
		ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_NODETSTOP, HW_RDD0, 0);
		pDfsParam->DetRateMode = RDD0_DET_RATE;
	}
	else if(value == 1)
	{	     
		ret1= mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
		ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0);
		ret1= mtRddControl(pAd, RDD_NODETSTOP, HW_RDD1, 0);
		pDfsParam->DetRateMode = RDD1_DET_RATE;
	}	
	else if(value == 2)
	{
#ifdef DOT11_VHT_AC	
		ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_NODETSTOP, HW_RDD0, 0);
		if(phy_bw == BW_8080 || phy_bw == BW_160)
		{
			ret2= mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
			ret2= mtRddControl(pAd, RDD_START, HW_RDD1, 0);
			ret2= mtRddControl(pAd, RDD_NODETSTOP, HW_RDD1, 0);
		}	
		else
#endif	
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStart_Proc: Bandwidth not 80+80 or 160\n"));
		pDfsParam->DetRateMode = RDD_BOTH_DET_RATE;
	}
	else if(value == 3)
	{	
		ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_DETSTOP, HW_RDD0, 0);
		pDfsParam->DetRateMode = RDD0_DET_RATE;
	}
	else if(value == 4)
	{
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, 0);
		ret1 = mtRddControl(pAd, RDD_DETSTOP, HW_RDD1, 0);
		pDfsParam->DetRateMode = RDD1_DET_RATE;
	}	
	else if(value == 5)
	{
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, 0);
		ret1 = mtRddControl(pAd, RDD_DETSTOP, HW_RDD0, 0);
#ifdef DOT11_VHT_AC	
		if(phy_bw == BW_8080 || phy_bw == BW_160)
		{
			ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
			ret2 = mtRddControl(pAd, RDD_START, HW_RDD1, 0);
			ret2 = mtRddControl(pAd, RDD_DETSTOP, HW_RDD1, 0);
		}	
		else
#endif	
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStart_Proc: Bandwidth not 80+80 or 160\n"));
		pDfsParam->DetRateMode = RDD_BOTH_DET_RATE;
	}		
	else
		;
	return TRUE;
}


INT Set_RadarDetectStop_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg)
{
		ULONG value, ret1, ret2;
		UCHAR phy_bw = HcGetBwByRf(pAd,RFIC_5GHZ);
		value = simple_strtol(arg, 0, 10);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStop_Proc: \n")); 

		if(value == 0)
			ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
		else if(value == 1)
		{
			ret1= mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
		}	
		else if(value == 2)
		{
		    ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
#ifdef DOT11_VHT_AC	
			if(phy_bw == BW_8080 || phy_bw == BW_160)
			{
				ret2= mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
			}	
			else
#endif	
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStop_Proc: Bandwidth not 80+80 or 160\n"));		 
		}		
		else
			;
		return TRUE;
}

INT Set_ByPassCac_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg)
{
	ULONG value; //CAC time
	value = simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_ByPassCac_Proc\n"));		 

	pAd->Dot11_H.ChMovingTime = value;
	return TRUE;
}

INT Set_RDDReport_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg)
{
	ULONG value;
	value = simple_strtol(arg, 0, 10);
	WrapDfsRddReportHandle(pAd, value);
	return TRUE;
}

INT Set_DfsChannelShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;
	value = simple_strtol(arg, 0, 10);

    if(!HcGetChannelByRf(pAd,RFIC_5GHZ))
    {
	    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("No 5G band channel\n"));	 
	}
	else	
	{
	    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Channel num is %d\n", 
		HcGetChannelByRf(pAd,RFIC_5GHZ)));
	}
	return TRUE;
}

INT Set_DfsBwShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;
	value = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Current Bw is %d\n", 
	HcGetBwByRf(pAd, RFIC_5GHZ) /*pAd->CommonCfg.BBPCurrentBW*/));
   
	return TRUE;
}

INT Set_DfsRDModeShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;
	value = simple_strtol(arg, 0, 10);

    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("RDMode is %d\n", 
            pAd->Dot11_H.RDMode));  

	return TRUE;
}

INT Set_DfsRDDRegionShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;
	value = simple_strtol(arg, 0, 10);
    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("RDD Region is %d\n", 
            pAd->CommonCfg.RDDurRegion));  
	return TRUE;
}

INT Set_DfsNonOccupancyShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;	
	UINT_8 i;
	value = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_DfsNonOccupancyShow_Proc: \n"));  
	for(i=0; i<pAd->ChannelListNum; i++)
	{
	    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DfsChannelList[%d].Channel = %d, DfsReq = %d, NonOccupancy = %d\n",
	    i, pAd->ChannelList[i].Channel,
	    pAd->ChannelList[i].DfsReq,
	    pAd->ChannelList[i].RemainingTimeForUse));
	}
	return TRUE;    
}

INT Set_DfsNonOccupancyClean_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;	
	UINT_8 i;
	value = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_DfsNonOccupancyClean_Proc:\n"));  
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Set NOP as %ld.\n", value));  
    
	for(i=0; i<pAd->ChannelListNum; i++)
	{
	    pAd->ChannelList[i].RemainingTimeForUse = value;
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("After Set_DfsNonOccupancyClean_Proc:\n"));  
	for(i=0; i<pAd->ChannelListNum; i++)
	{
	    pAd->ChannelList[i].RemainingTimeForUse = value;
	    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DfsChannelList[%d].Channel = %d, DfsReq = %d, NonOccupancy = %d\n",
	    i, pAd->ChannelList[i].Channel,
	    pAd->ChannelList[i].DfsReq,
	    pAd->ChannelList[i].RemainingTimeForUse));
	}    
	return TRUE;    
}

INT Set_DfsPulseInfoMode_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;	
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	value = simple_strtol(arg, 0, 10);
	pDfsParam->bShowPulseInfo = TRUE;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_DfsPulseInfoShow_Proc: \n"));  

    mtRddControl(pAd, RDD_PULSEDBG, HW_RDD0, 0); 
	return TRUE;    
}

INT Set_DfsPulseInfoRead_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;	
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	value = simple_strtol(arg, 0, 10);
	pDfsParam->bShowPulseInfo = FALSE;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_DfsPulseInfoRead_Proc: \n"));  

	MtCmdFwLog2Host(pAd, 0, 2);
	mtRddControl(pAd, RDD_READPULSE, HW_RDD0, 0); 
	MtCmdFwLog2Host(pAd, 0, 0);
	
	if(pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
	{
       pAd->Dot11_H.RDMode = RD_SILENCE_MODE;
	   pAd->Dot11_H.ChMovingTime = 5;
	}
	WrapDfsRadarDetectStart(pAd);
	return TRUE;    
}

/* DFS Zero Wait */
INT Set_DfsZeroWaitCacTime_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg)
{
    UCHAR Value;
    PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

    Value = (UCHAR) simple_strtol(arg, 0, 10);

    pDfsParam->DfsZeroWaitCacTime = Value;

    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("[%s]CacTime=%d/%d\n", 
                                __FUNCTION__,
                                Value,
                                pDfsParam->DfsZeroWaitCacTime));

    return TRUE; 
}

BOOLEAN WrapDfsByPassChannel(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR Channel)
{	
	UCHAR Bw = 0;
			
	if(wlan_config_get_ht_bw(wdev) == BW_20)
		Bw = BW_20;
	else if(wlan_config_get_ht_bw(wdev) == BW_40 && (pAd->CommonCfg.vht_bw == VHT_BW_2040))
		Bw = BW_40;
	else if(pAd->CommonCfg.vht_bw == VHT_BW_80)
		Bw = BW_80;
	else if(pAd->CommonCfg.vht_bw == VHT_BW_8080)
		Bw = BW_8080;
	else if(pAd->CommonCfg.vht_bw == VHT_BW_160)
		Bw = BW_160;			
	
	return ByPassChannelByBw(Channel, Bw, pAd->CommonCfg.RDDurRegion);
}

VOID DfsTXTPHighToLow(
	IN PRTMP_ADAPTER pAd)
{
	WrapDfsRadarDetectStart(pAd);
}

VOID DfsCheckRDDByTXTP(
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	UINT_32 TxTP;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];		

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst == SST_ASSOC))
		{
			TxTP = pEntry->AvgTxBytes >> 17;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Check TXTP: %d. \n", TxTP));
			/*suspend RDD due to high TP*/
			if ((pDfsParam->RadarDetectState == TRUE) && TxTP > RDD_STOP_TXTP_BW160)
			{
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Suspend RDD due to high TP. \n"));				
				MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_TP_LOW_HIGH, 0, NULL, 0);			    
				RTMP_MLME_HANDLER(pAd);
				pDfsParam->RddCtrlState = TP_LOW_HIGH_SUSPEND;
			}
			
			/*Resume RDD due to low TP*/
			if ((pDfsParam->RddCtrlState == TP_LOW_HIGH_SUSPEND) && TxTP <= RDD_STOP_TXTP_BW160)
			{
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Resume RDD due to low TP. \n"));
				//WrapDfsRadarDetectStart(pAd);
				MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_TP_HIGH_LOW, 0, NULL, 0);
			}			
			
		}
	}
}


BOOLEAN DfsEnterSilence(
    IN PRTMP_ADAPTER pAd)
{
    PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	return pDfsParam->bDfsEnable;
}

VOID DfsSetCalibration(
	    IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal)
{
	if(!DisableDfsCal)
	    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Enable DFS calibration in firmware. \n"));				
	else
	{
	    MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Disable DFS calibration in firmware. \n"));
        mtRddControl(pAd, RDD_DFSCAL, HW_RDD0, 0);    
	}	
}
VOID DFsSetFalseAlarmPrevent(
	IN PRTMP_ADAPTER pAd, UINT_32 DfsFalseAlarmPrevent)
{
	if(DfsFalseAlarmPrevent)
		mtRddControl(pAd, RDD_FALSE_ALARM_PREVENT, 0, 0);
}
VOID DfsSetZeroWaitCacSecond(
	    IN PRTMP_ADAPTER pAd)
{
    PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
    pDfsParam->bZeroWaitCacSecondHandle = TRUE;
}

BOOLEAN DfsRadarChannelCheck(
    IN PRTMP_ADAPTER pAd)
{
    PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR phy_bw = 0; //HcGetBwByRf(pAd,RFIC_5GHZ);
    DfsGetSysParameters(pAd); 
	phy_bw = decide_phy_bw_by_channel(pAd,pDfsParam->Band0Ch);
	
	if(!pDfsParam->bDfsEnable)
	    return FALSE;	
#ifdef DOT11_VHT_AC
    if(phy_bw == BW_8080)
    {
		return (RadarChannelCheck(pAd, pDfsParam->Band0Ch) || RadarChannelCheck(pAd, pDfsParam->Band1Ch));   		
    }
	else
#endif
    return RadarChannelCheck(pAd, pDfsParam->Band0Ch);
}

VOID DfsSetCountryRegion(
    IN PRTMP_ADAPTER pAd)
{
    PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;	
	pDfsParam->IsSetCountryRegion = TRUE;
}

VOID DfsCacEndUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	mtRddControl(pAd, RDD_CACEND, HW_RDD0, 0);		
	
	pAd->CommonCfg.DfsParameter.DfsStatMachine.CurrState = DFS_INSERVICE_MONITOR;
}

VOID DfsChannelSwitchTimeoutAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	if (DfsDetRateModeCheck(pAd, RD_SILENCE_MODE))
		return;

	APStopByRf(pAd, RFIC_5GHZ);
	if (DfsStopWifiCheck(pAd))
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n",__FUNCTION__));
		return;  
	}       
	APStartUpByRf(pAd, RFIC_5GHZ);

	if (pAd->CommonCfg.dbdc_mode)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] SetDfsTxStart\n", __FUNCTION__));

		MtCmdSetDfsTxStart(pAd, DBDC_BAND1);
	}
	else
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
	
	DfsSetClass2DeauthDisable(pAd, FALSE);

}

VOID DfsCacNormalStart(
    IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;	
#ifdef BACKGROUND_SCAN_SUPPORT    
    BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
#endif

    DfsGetSysParameters(pAd);
	if ((pAd->CommonCfg.RDDurRegion == CE) && DfsCacRestrictBand(pAd, pDfsParam))
    {
        /* Weather band channel */
		pAd->Dot11_H.ChMovingTime = 605;
#ifdef BACKGROUND_SCAN_SUPPORT         
        if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
        {
            /* Off-Channel CAC time setup */
            if (pDfsParam->DfsZeroWaitCacTime == ZeroWaitCacApplyDefault) //Follow Spec
            {
                BgndScanCtrl->DfsZeroWaitDuration = WEATHER_OFF_CHNL_CAC_TIME;
            }
            else
            {   //Certification/Test mode using 
                BgndScanCtrl->DfsZeroWaitDuration = pDfsParam->DfsZeroWaitCacTime * BgnScanCacUnit;
            }
        }
#endif /* BACKGROUND_SCAN_SUPPORT  */          
    }
    else
    {
#ifdef BACKGROUND_SCAN_SUPPORT     
        if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
        {   
            /* Non-weather band Channel */
            if (pDfsParam->DfsZeroWaitCacTime == ZeroWaitCacApplyDefault) //Follow Spec 
            {
                BgndScanCtrl->DfsZeroWaitDuration = DEFAULT_OFF_CHNL_CAC_TIME;
            }
	else
            {   //Certification/Test mode using
                BgndScanCtrl->DfsZeroWaitDuration = pDfsParam->DfsZeroWaitCacTime * BgnScanCacUnit;
            }
        }
#endif /* BACKGROUND_SCAN_SUPPORT  */  
		pAd->Dot11_H.ChMovingTime = 65;
    }

#ifdef BACKGROUND_SCAN_SUPPORT
    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]DfsCacTime=%d/BgnCacTime=%ld\n",
                                                     __FUNCTION__,
                                                     pDfsParam->DfsZeroWaitCacTime,
                                                     BgndScanCtrl->DfsZeroWaitDuration));
#endif

    
    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]RDMode=%d/ZeroWaitState=%d\n",
                                                     __FUNCTION__,
                                                     pAd->Dot11_H.RDMode,
                                                     GET_MT_ZEROWAIT_DFS_STATE(pAd)));
    if(pAd->Dot11_H.RDMode == RD_SILENCE_MODE)
    {
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[APStartUp][MT_DFS]CAC time start !!!!!\n\n\n\n"));
        mtRddControl(pAd, RDD_CACSTART, HW_RDD0, 0);
    }
    else if(pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
    {
#ifdef BACKGROUND_SCAN_SUPPORT  
        if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
        {
            if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT) ||
                CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE))
            {
                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Recover single band mode! \n",__FUNCTION__));               
            }
            else
            {
                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Enter Zero wait CAC period \n",__FUNCTION__));                
            }
            mtRddControl(pAd, RDD_NORMALSTART, HW_RDD0, 0);
        }
        else
#endif /* BACKGROUND_SCAN_SUPPORT */            
        {
            MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]DFS Normal start! \n",__FUNCTION__));
            mtRddControl(pAd, RDD_NORMALSTART, HW_RDD0, 0);
        }
    }
    else
       ;
}

BOOLEAN DfsCacRestrictBand(
	IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam)
{
	UCHAR phy_bw = decide_phy_bw_by_channel(pAd,pDfsParam->Band0Ch);//HcGetBwByRf(pAd,RFIC_5GHZ);
#ifdef DOT11_VHT_AC
    if(phy_bw == BW_8080)
    {
		return (RESTRICTION_BAND_1(pAd, pDfsParam->Band0Ch,pDfsParam->Bw) || RESTRICTION_BAND_1(pAd, pDfsParam->Band1Ch,pDfsParam->Bw));   		
    }
	else if((phy_bw == BW_160) && (pDfsParam->Band0Ch >= GROUP3_LOWER && pDfsParam->Band0Ch <= RESTRICTION_BAND_HIGH))    
        return TRUE;
	else
#endif
  {
      if (strncmp(pAd->CommonCfg.CountryCode, "KR", 2) == 0)
      {
          return RESTRICTION_BAND_KOREA(pAd, pDfsParam->Band0Ch, phy_bw);    
      }
      else
      {
          return RESTRICTION_BAND_1(pAd, pDfsParam->Band0Ch, phy_bw);
      }       
  }          
}
VOID DfsSaveNonOccupancy(
    IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if(pDfsParam->IsSetCountryRegion == FALSE)
	{
        for(i=0; i<pAd->ChannelListNum; i++)
        {
            pDfsParam->DfsChannelList[i].NonOccupancy = pAd->ChannelList[i].RemainingTimeForUse;
		}
	}
}

VOID DfsRecoverNonOccupancy(
    IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if(pDfsParam->IsSetCountryRegion == FALSE)
	{
        for(i=0; i<pAd->ChannelListNum; i++)
        {
            pAd->ChannelList[i].RemainingTimeForUse = pDfsParam->DfsChannelList[i].NonOccupancy;

        }
	}
    pDfsParam->IsSetCountryRegion = FALSE;
}

BOOLEAN DfsSwitchCheck(
		IN PRTMP_ADAPTER pAd,
		UCHAR Channel)
{		

	if ((pAd->Dot11_H.RDMode == RD_SILENCE_MODE) && (Channel >14))
	{
	    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[DfsSwitchCheck]: DFS ByPass TX calibration.\n"));
	    return TRUE;
	}	
	else
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSwitchCheck]: Not DFS ByPass TX calibration.\n"));
		return FALSE;
	}	

}

BOOLEAN DfsStopWifiCheck(
    IN PRTMP_ADAPTER	pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
    
	return (pDfsParam->bNoAvailableCh == TRUE);
}

VOID DfsNonOccupancyCountDown( /*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	DfsGetSysParameters(pAd);	
	
	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].RemainingTimeForUse > 0)
		{
			pAd->ChannelList[i].RemainingTimeForUse --;	
		}
	}
	if(pDfsParam->bNoAvailableCh == TRUE)
	{
		for (i = 0; i < pAd->ChannelListNum; i++)
		{
			if (pAd->ChannelList[i].Channel == pDfsParam->PrimCh)
			{
				if (pAd->ChannelList[i].RemainingTimeForUse == 0)
				{		
					pDfsParam->bNoAvailableCh = FALSE;
					MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CHAN_SWITCH_TIMEOUT, 0, NULL, 0);
				}
			}
		}
	}	
}

VOID WrapDfsSetNonOccupancy( /*Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;	
    DfsGetSysParameters(pAd);

	DfsSetNonOccupancy(pAd, pDfsParam);	
}

VOID DfsSetNonOccupancy( /*Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam)
{
    UINT_8 i;
	if(pDfsParam->Dot11_H.RDMode == RD_SWITCHING_MODE) 
        return;	
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n[DfsSetNonOccupancy]: pDfsParam->Bw is %d\n", pDfsParam->Bw));	
	if((pDfsParam->Bw == BW_160) && (pDfsParam->DfsChBand[0] || pDfsParam->DfsChBand[1]))
	{
	    for(i=0; i<pDfsParam->ChannelListNum; i++)
	    {
			if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_160) == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_160))	
			    pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;	
		}
	}
	if((pDfsParam->Bw == BW_80) && pDfsParam->DfsChBand[0])
	{
	    for(i=0; i<pDfsParam->ChannelListNum; i++)
	    {
			if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_80) == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_80))	
			    pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;	
		}
	}
	else if(pDfsParam->Bw == BW_40 && pDfsParam->DfsChBand[0])
	{
	    for(i=0; i<pDfsParam->ChannelListNum; i++)
	    {
	        if((pDfsParam->Band0Ch == pDfsParam->DfsChannelList[i].Channel))
	        {
				pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;
			}
			else if(((pDfsParam->Band0Ch)>>2 & 1) && ((pDfsParam->DfsChannelList[i].Channel-pDfsParam->Band0Ch)==4))
			{
				pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;
			}
			else if(!((pDfsParam->Band0Ch)>>2 & 1) && ((pDfsParam->Band0Ch-pDfsParam->DfsChannelList[i].Channel)==4))
			{
				pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;
			}
			else
			    ;
		}		
	}
	else if(pDfsParam->Bw == BW_20 && pDfsParam->DfsChBand[0])
	{
        for(i=0; i<pDfsParam->ChannelListNum; i++)
        {
            if((pDfsParam->Band0Ch == pDfsParam->DfsChannelList[i].Channel))
            {
				pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;
			}
		}
	}
	else if(pDfsParam->Bw == BW_8080 && pDfsParam->DfsChBand[0] && pDfsParam->RadarDetected[0])
	{        
        for(i=0; i<pDfsParam->ChannelListNum; i++)
        {
            if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_8080) == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080))	
            {
                pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;				    
            }
        }	
	}

    else if(pDfsParam->Bw == BW_8080 && pDfsParam->DfsChBand[1] && pDfsParam->RadarDetected[1])
    {
	    for(i=0; i<pDfsParam->ChannelListNum; i++)
		{
		    if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_8080) == vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080))	
		    {
			    pAd->ChannelList[i].RemainingTimeForUse = CHAN_NON_OCCUPANCY;					
			}
		}	
	}
			
	else
	    ;	
}

BOOLEAN DfsDetRateModeCheck(
	IN PRTMP_ADAPTER pAd, UCHAR RDMode)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if (pDfsParam->DetRateMode == NO_CH_SWITCH)
		return FALSE;
	else if((pDfsParam->DetRateMode >= RDD0_DET_RATE) && (pDfsParam->DetRateMode <= RDD_BOTH_DET_RATE))
	{
		if((pDfsParam->bDBDCMode) && (pDfsParam->DetRateMode & BIT(1)))
		{			
			pDfsParam->DfsChBand[0] = TRUE;
		}
		else 
		{
			if(pDfsParam->DetRateMode & BIT(0))
				pDfsParam->DfsChBand[0] = TRUE;
			if(pDfsParam->DetRateMode & BIT(1))
				pDfsParam->DfsChBand[1] = TRUE;
		}
		Set5GPrimChannel(pAd, pAd->Dot11_H.org_ch);
		if(RDMode == RD_SILENCE_MODE)
			pAd->Dot11_H.RDMode = RD_SILENCE_MODE;

		if(RDMode == RD_NORMAL_MODE)
			pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
		return TRUE;
	}	
	return FALSE;
}

VOID WrapDfsRddReportHandle( /*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx)
{
	UCHAR PhyMode;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR NextCh = 0;
#ifdef BACKGROUND_SCAN_SUPPORT
	UCHAR Channel = 0;
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg = &(BgndScanCtrl->CurrentSwChCfg[0]);
#endif /* BACKGROUND_SCAN_SUPPORT */
	DfsSetClass2DeauthDisable(pAd, TRUE);

	if((pDfsParam->Dot11_H.RDMode == RD_SILENCE_MODE) && (pDfsParam->DetRateMode == NO_CH_SWITCH))
		pAd->Dot11_H.RDCount = 0;
	
	pDfsParam->Dot11_H.RDMode = pAd->Dot11_H.RDMode;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRddReportHandle]:  Radar detected !!!!!!!!!!!!!!!!!\n"));

	if(pDfsParam->Band0Ch > 14)
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_5GHZ);
	else
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_24GHZ);
	
	if(DfsRddReportHandle(pDfsParam, ucRddIdx) || (pDfsParam->bZeroWaitCacSecondHandle == TRUE))
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[WrapDfsRddReportHandle]: pDfsParam->RadarDetected[0] is %d\n",pDfsParam->RadarDetected[0]));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[WrapDfsRddReportHandle]: pDfsParam->RadarDetected[1] is %d\n",pDfsParam->RadarDetected[1]));
		WrapDfsSetNonOccupancy(pAd);		

#ifdef BACKGROUND_SCAN_SUPPORT
		if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) && 
		CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) &&
		(pDfsParam->bZeroWaitCacSecondHandle == FALSE))
		{
			/* Skip CH uniform spreading and switch to Ori Non-DFS CH by ZeroWaitStop flow */
			UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_RADAR_DETECT);     
			BgndScanCtrl->RadarDetected = TRUE;            
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]:In OffChlCac & CurrCh is Ch=%d, RadarDetected=%d/%d\n",
							__FUNCTION__,
							pDfsParam->Band0Ch,
							BgndScanCtrl->RadarDetected,
							pAd->BgndScanCtrl.RadarDetected));

			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_RDD_TIMEOUT, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);      
			return;
		}
		else
#endif /* BACKGROUND_SCAN_SUPPORT */
		{   
			/* Detected radar & InsrvMonitor state & uniform Ch Sel 
			Get new CH & save into pDfsParam->Band0Ch temporary */
			pDfsParam->bZeroWaitCacSecondHandle = FALSE;    
			WrapDfsSelectChannel(pAd);

#ifdef BACKGROUND_SCAN_SUPPORT
			BackgroundScanCancelAction(pAd, NULL); // Cannel background scan.
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:SrvMonitor State & RadarDetected=%d, UniformCh is Ch=%d,\n",
								__FUNCTION__,
								BgndScanCtrl->RadarDetected,
								pDfsParam->Band0Ch));
			BgndScanCtrl->RadarDetected = TRUE; 
			/* Uniform CH is a DFS CH */ 
			if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) &&
			BgndScanCtrl->BgndScanSupport)
			{
				INT32 ret;
				if (RadarChannelCheck(pAd, pDfsParam->Band0Ch))
				{  
#ifdef MAC_REPEATER_SUPPORT
					//Disable DFS zero wait support  for repeater mode dynamic enable
					if (pAd->ApCfg.bMACRepeaterEn)
					{
						BgndScanCtrl->DfsZeroWaitSupport = FALSE;
						UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
						UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
					}
					else
#endif /* MAC_REPEATER_SUPPORT */                
					{
						UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_RADAR_DETECT);

						/* Save DFS zero wait CH */
						BgndScanCtrl->DfsZeroWaitChannel = pDfsParam->Band0Ch; 
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]New ZeroWait Ch=%d/%d \n", 
									__FUNCTION__, 
									pDfsParam->Band0Ch,
									BgndScanCtrl->DfsZeroWaitChannel));
                                                                                                                                    
						/* Re-select a non-DFS channel. */
						Channel = WrapDfsRandomSelectChannel(pAd, TRUE); /* Skip DFS CH */
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]New Band0 Channel %d for DFS zero wait! \n", __FUNCTION__, Channel));

						ret = HcUpdateChannel(pAd,Channel);
						if(ret < 0)
						{
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
							("[%s]: Update Channel %d faild, not support this RF\n", __FUNCTION__, Channel));                 
						}

						/* Update Control/Central channel into BgndScanCtrl.CurrentSwChCfg */
						CurrentSwChCfg->ControlChannel = Channel;
						CurrentSwChCfg->CentralChannel = DfsGetCentCh(pAd, Channel, CurrentSwChCfg->Bw);
						UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC); 
					}
				}
				else
				{
					/* Update DFS zero wait channel */
					BgndScanCtrl->DfsZeroWaitChannel = 0;
				}
			}
#endif /* BACKGROUND_SCAN_SUPPORT */
		}

#ifdef DOT11_N_SUPPORT
		N_ChannelCheck(pAd,PhyMode,pDfsParam->PrimCh);
#endif 

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[WrapDfsRddReportHandle] RD Mode is %d\n", pDfsParam->Dot11_H.RDMode));		


#ifdef BACKGROUND_SCAN_SUPPORT
		if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) == FALSE || 
		(IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) && 
		(CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) == FALSE)))                
#endif /* BACKGROUND_SCAN_SUPPORT */                    
		{   
			/* Zero wait uniform Ch is NonDfs Ch or Normal DFS uniform Ch */
			NextCh = pDfsParam->PrimCh;
		}
#ifdef BACKGROUND_SCAN_SUPPORT                
		else
		{
			/* Zero wait uniform Ch is DFS Ch */
			NextCh = Channel;
		}
#endif /* BACKGROUND_SCAN_SUPPORT */

		pAd->Dot11_H.org_ch = HcGetChannelByRf(pAd, RFIC_5GHZ);

		if (pDfsParam->Dot11_H.RDMode == RD_NORMAL_MODE)
		{	    
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;		
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;	

			pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
			pAd->Dot11_H.CSCount = 0;
			pAd->Dot11_H.new_channel = NextCh; //Zero Wait Band0 Channel
			Set5GPrimChannel(pAd, NextCh);

#ifdef BACKGROUND_SCAN_SUPPORT 
			//Set state into radar detected state for uniform CH is Non-DFS CH 
			if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)&&(CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) == FALSE))
			{
				UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_RADAR_DETECT);
			}
#endif /* BACKGROUND_SCAN_SUPPORT */

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Update Uniform Ch=%d \n", 
									__FUNCTION__, 
									NextCh));
			if(HcUpdateCsaCntByChannel(pAd, NextCh) != 0)
			{
				;
			}			
		}
		else if (pDfsParam->Dot11_H.RDMode == RD_SILENCE_MODE)
		{
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;		
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;	
 	        	Set5GPrimChannel(pAd, NextCh);
			if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
			{
#ifdef BACKGROUND_SCAN_SUPPORT                
				BgndScanCtrl->RadarDetected = FALSE;
#endif
			}            
			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CHAN_SWITCH_TIMEOUT, 0, NULL, 0);			
 			RTMP_MLME_HANDLER(pAd);
		}
	}	
}

BOOLEAN DfsRddReportHandle( /*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PDFS_PARAM pDfsParam, UCHAR ucRddIdx)
{
    BOOLEAN RadarDetected = FALSE;

    if(ucRddIdx == 0 && (pDfsParam->RadarDetected[0] == FALSE) && (pDfsParam->DfsChBand[0])
	&& (pDfsParam->Dot11_H.RDMode != RD_SWITCHING_MODE))
    {    
        pDfsParam->RadarDetected[0] = TRUE;	
		RadarDetected = TRUE;
    }	

#ifdef DOT11_VHT_AC	
    if(ucRddIdx == 1 && (pDfsParam->RadarDetected[1] == FALSE) && (pDfsParam->DfsChBand[1])
	&&(pDfsParam->Dot11_H.RDMode != RD_SWITCHING_MODE))
    {
        pDfsParam->RadarDetected[1] = TRUE;		
        RadarDetected = TRUE;
	}
#endif
    if(pDfsParam->bDBDCMode)
    {
		if(ucRddIdx == 1 && (pDfsParam->RadarDetected[1] == FALSE) && (pDfsParam->DfsChBand[0])
		&&(pDfsParam->Dot11_H.RDMode != RD_SWITCHING_MODE))
		{
			pDfsParam->RadarDetected[1] = TRUE; 	
			RadarDetected = TRUE;
		}
    }

    /* DFS zero wait case */
    if (pDfsParam->bZeroWaitSupport)
    {
        if((ucRddIdx == 1) && 
            (pDfsParam->RadarDetected[1] == FALSE) && 
            (pDfsParam->DfsChBand[0])&&
            (pDfsParam->ZeroWaitDfsState != DFS_RADAR_DETECT))
        {
            pDfsParam->RadarDetected[1] = TRUE;
            RadarDetected = TRUE;
            MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][ZeroWait] Rdd%d have radar! \n",
                                                       __FUNCTION__,
                                                       ucRddIdx));
        }
    }

    return RadarDetected;
}

VOID DfsCacEndHandle( /*handle the event of EXT_EVENT_ID_CAC_END*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DfsCacEndHandle]:\n")); 	
}

VOID WrapDfsSelectChannel(  /*Select new channel*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR phy_bw = HcGetBwByRf(pAd,RFIC_5GHZ);
    DfsGetSysParameters(pAd);
	DfsSelectChannel(pAd,pDfsParam);
	
#ifdef DOT11_VHT_AC		
    
	if(phy_bw == BW_8080)
    {
	
        if(pDfsParam->PrimBand == RDD_BAND0)
        { 
			pAd->CommonCfg.vht_cent_ch2 
		    = vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080);//Central channel 2;
        }	
		else
		{	
			pAd->CommonCfg.vht_cent_ch2 
		    = vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080);//Central channel 2;;
		}	
    }

#endif
	
}

VOID DfsSelectChannel( /*Select new channel*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam)
{
    UCHAR tempCh=0;

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]:RadarDetected[1]=%d, pDfsParam->DfsChBand[0]=%d\n",
                                                       __FUNCTION__, 
                                                       pDfsParam->RadarDetected[1],
                                                       pDfsParam->DfsChBand[0])); 
    if(pDfsParam->Bw == BW_8080)
    {
	    if(pDfsParam->Band0Ch < pDfsParam->Band1Ch)
	    { 
	        if(pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0])
            {
                pDfsParam->Band0Ch = 124; //WrapDfsRandomSelectChannel(pAd);
                while(vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080) 
				   == vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080))
				{
				    pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE);
				}
	        }
	        if(pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[1])
            {
                pDfsParam->Band1Ch = WrapDfsRandomSelectChannel(pAd, FALSE);
				while(vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080) 
				   == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080))
				{
				    pDfsParam->Band1Ch = WrapDfsRandomSelectChannel(pAd, FALSE);
				}	                
	        }
        }	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: 80+80MHz band, selected is %d, %d\n",pDfsParam->Band0Ch, pDfsParam->Band1Ch));

        if(pDfsParam->PrimBand == RDD_BAND0)
        {
		    pDfsParam->PrimCh = pDfsParam->Band0Ch;		
        }
		else
		{
            pDfsParam->PrimCh = pDfsParam->Band1Ch; 
		}
		if(pDfsParam->Band1Ch < pDfsParam->Band0Ch)
		{
            tempCh = pDfsParam->Band1Ch;
			pDfsParam->Band1Ch = pDfsParam->Band0Ch;
			pDfsParam->Band0Ch = tempCh;
		}
		if(pDfsParam->PrimCh == pDfsParam->Band0Ch)
		{
            pDfsParam->PrimBand = RDD_BAND0;
		}
		else
		{
			pDfsParam->PrimBand = RDD_BAND1;
		}			
    }
    else
    {
    	if((pDfsParam->Bw == BW_160) 
		&& ((pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0]) 
		    || (pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[1])))
    	{
		    pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: Single band, selected is %d\n",pDfsParam->Band0Ch));					    
		}
	    else if(pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0])
  	    {
			pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE);	
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: Single band, selected is %d\n",pDfsParam->Band0Ch));
	    }
		else
            ;	
        if(pDfsParam->bDBDCMode)
        {
		    if(pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[0])
            {
			    pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE);	
                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: DBDC band, selected is %d\n",pDfsParam->Band0Ch));            
		    }
        }

        /* DFS zero wait case */
        if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
        {
            if(pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[0])
            {
                pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE);
                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]:DFS zero wait,New Ch=%d\n",
                                                                 __FUNCTION__, 
                                                                 pDfsParam->Band0Ch));  
		    }
        }	

        pDfsParam->PrimCh = pDfsParam->Band0Ch;
        pDfsParam->PrimBand = RDD_BAND0;
    }	    
}

UCHAR WrapDfsRandomSelectChannel( /*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh)
{
	//UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	DfsGetSysParameters(pAd);

	return DfsRandomSelectChannel(pAd, pDfsParam, bSkipDfsCh);
}


UCHAR DfsRandomSelectChannel( /*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, BOOLEAN bSkipDfsCh)
{
	UINT_8 i, cnt, ch, flag;
	UINT_8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	cnt = 0;
	
	if(pDfsParam->bIEEE80211H)		
	{
		for (i = 0; i < pDfsParam->ChannelListNum; i++)
		{
	        	if (pDfsParam->DfsChannelList[i].NonOccupancy)
				continue;
			if (AutoChannelSkipListCheck(pAd, pDfsParam->DfsChannelList[i].Channel) == TRUE)
				continue;
			if(pDfsParam->bDBDCMode)
			{
				if (HcGetBandByChannel(pAd, pDfsParam->DfsChannelList[i].Channel) == 0)
					continue;
			}		
            
			/* Skip DFS channel for DFS zero wait using case */
			if (bSkipDfsCh)
			{
				if (RadarChannelCheck(pAd, pDfsParam->DfsChannelList[i].Channel))
					continue;
			}
	
#ifdef DOT11_N_SUPPORT
			if (pDfsParam->Bw == BW_40 
			&& ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_40, pDfsParam->RDDurRegion))
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]: New BW40 Bypass Ch%d\n",
										 __FUNCTION__, 
										 pDfsParam->DfsChannelList[i].Channel));	

				continue;
			}
#endif /* DOT11_N_SUPPORT */
	
#ifdef DOT11_VHT_AC
			if ((pDfsParam->Bw == BW_80 || pDfsParam->Bw == BW_8080) 
			&& ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_80, pDfsParam->RDDurRegion))
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]: New BW80 Bypass Ch%d\n",
										 __FUNCTION__, 
										 pDfsParam->DfsChannelList[i].Channel));	

				continue;
			}
			if ((pDfsParam->Bw == BW_160) && (pDfsParam->DfsChannelList[i].Channel > UPPER_BW_160(pAd)))
				continue;
#endif /* DOT11_VHT_AC */

			/* Store available channel to temp list */
			TempChList[cnt++] = pDfsParam->DfsChannelList[i].Channel;
		}
		if(cnt)
		{
			ch = TempChList[RandomByte(pAd)%cnt];
		}
		else
		{			
			USHORT MinTime = 0xFFFF;
			ch = 0;
			flag = 0; 
			pDfsParam->bNoAvailableCh = TRUE;

			for(i=0; i<pDfsParam->ChannelListNum; i++)
			{
				if(pDfsParam->DfsChannelList[i].NonOccupancy < MinTime)
				{
					MinTime = pDfsParam->DfsChannelList[i].NonOccupancy;
					ch = pDfsParam->DfsChannelList[i].Channel;
					flag = pDfsParam->DfsChannelList[i].Flags;
				}
			}
		}
	}

	else
	{
		ch = pDfsParam->DfsChannelList[RandomByte(pAd)%pDfsParam->ChannelListNum].Channel;
		if(ch==0)
		{
			ch = pDfsParam->DfsChannelList[0].Channel;
		}
		/* Don't care IEEE80211 disable when bSkipDfsCh is FALSE */
	}

	return ch;

}

VOID WrapDfsRadarDetectStart( /*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR phy_bw = HcGetBwByRf(pAd,RFIC_5GHZ);
	if(pDfsParam->bShowPulseInfo)
	    return;	

	DfsGetSysParameters(pAd);
    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRadarDetectStart]: Band0Ch is %d", pDfsParam->Band0Ch));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRadarDetectStart]: Band1Ch is %d", pDfsParam->Band1Ch));
	pDfsParam->DfsChBand[0] = RadarChannelCheck(pAd, pDfsParam->Band0Ch);	

#ifdef DOT11_VHT_AC	
	if(phy_bw == BW_8080)
	{
        pDfsParam->DfsChBand[1] = RadarChannelCheck(pAd, pDfsParam->Band1Ch);
	}
	if(phy_bw == BW_160)
	{
        pDfsParam->DfsChBand[1] = pDfsParam->DfsChBand[0];	
	}
#endif

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[WrapDfsRadarDetectStart]: "));
    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Bandwidth: %d, RDMode: %d\n"
	,pDfsParam->Bw, pDfsParam->Dot11_H.RDMode));
	DfsRadarDetectStart(pAd, pDfsParam);
}

VOID DfsRadarDetectStart( /*Start Radar Detection or not*/
   IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam)
{
	INT ret1 = TRUE;	
	if(DfsIsScanRunning(pAd) || (pDfsParam->Dot11_H.RDMode == RD_SWITCHING_MODE))
	{
		pDfsParam->bDfsIsScanRunning = FALSE;
		return;	    
	}

    if((pDfsParam->Dot11_H.RDMode == RD_SILENCE_MODE) 
    	|| (pDfsParam->RddCtrlState == TP_LOW_HIGH_SUSPEND)
#ifdef BACKGROUND_SCAN_SUPPORT        
        || (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) 
        && (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE) == FALSE))
#endif        
        )
    {
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]:ZeroWaitState:%d\n",
                                                        __FUNCTION__,
                                                        GET_MT_ZEROWAIT_DFS_STATE(pAd)));     
        if(pDfsParam->RadarDetectState == FALSE)
	    {
		if(pDfsParam->RddCtrlState == TP_LOW_HIGH_SUSPEND)
		pDfsParam->RddCtrlState = TP_HIGH_LOW_RESUME;
            if(pDfsParam->bDBDCMode 
#ifdef BACKGROUND_SCAN_SUPPORT                 
                ||(IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) 
                &&(CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC)
                ||CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INIT_CAC)))
#endif                 
                )
	        {
                /* DBDC mode DFS and Zero wait DFS case */
                ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0); //RddSel=0: Use band1/RX2 to detect radar
	        }	
#ifdef DOT11_VHT_AC				
	        else if (pDfsParam->Bw == BW_160)
	        {
	            if(pDfsParam->Bw == BW_160 
		    &&(pDfsParam->Band0Ch >= GROUP1_LOWER && pDfsParam->Band0Ch <= GROUP2_UPPER))
		        ;
		    else
	            ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
		    
		        ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0);
	        }
            else if(pDfsParam->Bw == BW_8080)
            {
                if(pDfsParam->DfsChBand[0])
		            ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);				
		        if(pDfsParam->DfsChBand[1])
		            ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0);				
            }
#endif			
#ifdef BACKGROUND_SCAN_SUPPORT 
            else if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) &&
                     CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT))
            {
                /* Zero wait timeout case: skip CAC time check */
                mtRddControl(pAd, RDD_START, HW_RDD0, 0);
                UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INSERV_MONI);
            }
#endif            
	    else
	    {
               ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
	    }				
	}
        pDfsParam->RadarDetectState = TRUE;
    }	
}

VOID WrapDfsRadarDetectStop( /*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	DfsGetSysParameters(pAd);
	DfsRadarDetectStop(pAd, pDfsParam);
}

VOID DfsRadarDetectStop( /*Start Radar Detection or not*/
   IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam)
{
    INT ret1 = TRUE, ret2 = TRUE;	
	pDfsParam->RadarDetectState = FALSE;
	if(!pDfsParam->bDfsEnable)
	    return;	
	ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
	ret2= mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);		
}

BOOLEAN DfsIsScanRunning(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	return (pDfsParam->bDfsIsScanRunning == TRUE);	
}

VOID DfsSetScanRunning(
	IN PRTMP_ADAPTER pAd, BOOLEAN bIsScan)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if(bIsScan)
		pDfsParam->bDfsIsScanRunning = TRUE;	
	else
		pDfsParam->bDfsIsScanRunning = FALSE;	
}

BOOLEAN DfsIsClass2DeauthDisable(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	return (pDfsParam->bClass2DeauthDisable == TRUE);	
}

VOID DfsSetClass2DeauthDisable(
	IN PRTMP_ADAPTER pAd, BOOLEAN bEnable)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if(bEnable)
		pDfsParam->bClass2DeauthDisable = TRUE;	
	else
		pDfsParam->bClass2DeauthDisable = FALSE;	
}

/*----------------------------------------------------------------------------*/
/*!
* \brief     Configure (Enable/Disable) HW RDD and RDD wrapper module
*
* \param[in] ucRddCtrl
*            ucRddIdex
*
*
* \return    None
*/
/*----------------------------------------------------------------------------*/

INT mtRddControl(
        IN struct _RTMP_ADAPTER *pAd,
        IN UCHAR ucRddCtrl,
        IN UCHAR ucRddIdex,
        IN UCHAR ucRddInSel)
{
    INT ret = TRUE;

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[mtRddControl]RddCtrl=%d, RddIdx=%d, RddInSel=%d\n", ucRddCtrl, ucRddIdex, ucRddInSel));
    ret = MtCmdRddCtrl(pAd, ucRddCtrl, ucRddIdex, ucRddInSel);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[mtRddControl]complete\n"));

    return ret;
}

UCHAR DfsGetCentCh(IN PRTMP_ADAPTER pAd,IN UCHAR Channel, IN UCHAR bw)
{
    UCHAR CentCh = 0;

	if (bw == BW_20)
	{
	    CentCh = Channel;
	}
#ifdef DOT11_N_SUPPORT
    else if ((bw == BW_40)
//#ifdef DOT11_VHT_AC	
//        && (bw == VHT_BW_2040)  
//#endif /* DOT11_VHT_AC */
        && N_ChannelGroupCheck(pAd, Channel))
    {
#ifdef A_BAND_SUPPORT
	    if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
	       (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
	    {
		    CentCh = Channel + 2;
	    }
	    else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			    (Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
	    {
		    CentCh = Channel -2;
	    }
#endif /* A_BAND_SUPPORT */        
    }
#ifdef DOT11_VHT_AC
    else if (bw == BW_80)
    {
        if (vht80_channel_group(pAd, Channel))
        {
            CentCh = vht_cent_ch_freq (Channel, VHT_BW_80); 
        }
    }
	else
	{
	    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s]Error!Unexpected Bw=%d!!\n",
		                                                __FUNCTION__, 
	                                                    bw));
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Control/Central Ch=%d/%d;Bw=%d\n",
		                                                __FUNCTION__, 
	                                                    Channel,
	                                                    CentCh,
	                                                    bw));       
#endif /* DOT11_VHT_AC */		                                            
#endif /* DOT11_N_SUPPORT */		                                            
	return CentCh;
}

#ifdef BACKGROUND_SCAN_SUPPORT
/* MBSS Zero Wait */
BOOLEAN MbssZeroWaitStopValidate(PRTMP_ADAPTER pAd, UCHAR MbssCh, INT MbssIdx)
{
	
    BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
    BOOLEAN ZeroWaitStop = FALSE;    
 

    if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) && 
        CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) && 
        (pAd->ApCfg.BssidNum > 1)
            )
    {

        if (GET_MT_MT_INIT_ZEROWAIT_MBSS(pAd) && (CheckLargestNOP(pAd) < RADAR_DETECT_NOP_TH))
        {
            ZeroWaitStop = TRUE;
            /* Only mbss0 to do ZeroWaitStop for non-mbss init scenarios */
            MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Mbss ZeroWait Stop, Mbss/CurCh=%d/%d,mbssIdx=%d,BgnStat=%ld \n", 
	                                                    __FUNCTION__, 
	                                                    MbssCh,
	                                                    HcGetChannelByRf(pAd,RFIC_5GHZ),
	                                                    MbssIdx,
	                                                    pAd->BgndScanCtrl.BgndScanStatMachine.CurrState));
        
            UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_MBSS_CAC);
        
            /* Terminate Zero wait flow */
            DfsZeroWaitStopAction(pAd, NULL);
          
            /* Update DfsZeroWait channel */
            if (MbssCh != 0 && MbssIdx != 0)
            {
                BgndScanCtrl->DfsZeroWaitChannel = MbssCh;
            }              
        }
        else
        {   
            if (pAd->ApCfg.BssidNum > 1)
                UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
        }    
    }

    return ZeroWaitStop;
}

VOID ZeroWaitUpdateForMbss(PRTMP_ADAPTER pAd, BOOLEAN bZeroWaitStop, UCHAR MbssCh, INT MbssIdx)
{
#ifdef BACKGROUND_SCAN_SUPPORT
    BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
#endif
    UINT8 Channel;
    
    if(IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)
        && (!CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INSERV_MONI))
        && (pAd->ApCfg.BssidNum > 1)       
        && (((MbssIdx == pAd->ApCfg.BssidNum -1) || bZeroWaitStop) ||
            (GET_MT_MT_INIT_ZEROWAIT_MBSS(pAd) && CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE)))
        )
    {
        /* Non-Dfs CH selection for new  Zero Wait trigger if ineed */
        if (RadarChannelCheck(pAd, MbssCh))
        {
            BgndScanCtrl->DfsZeroWaitChannel = MbssCh;
            Channel = WrapDfsRandomSelectChannel(pAd, TRUE); /* Skip DFS CH */
            HcUpdateChannel(pAd, Channel);
           
            MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][Dfs]New Mbss/CurCh=%d/%d \n", 
	                                                    __FUNCTION__, 
	                                                    Channel,
	                                                    HcGetChannelByRf(pAd,RFIC_5GHZ)));
            UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC);
        }
        else
        {
            if (RadarChannelCheck(pAd, BgndScanCtrl->DfsZeroWaitChannel))
            {                
                if (MbssIdx != pAd->ApCfg.BssidNum -1)
                {
                    /* Temporary Zero wait state for MBSS nonDFS CH */
                    UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
                }
                else
                {
                    /* For latest MBSS DFS CH */
                    UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC);
                }
            }
            else
            {
                BgndScanCtrl->DfsZeroWaitChannel = 0;
                UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
            }            
           
            MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][NonDfs]New Mbss/CurCh=%d/%d,ZeroWaitCh=%d,stat=%d \n", 
	                                                    __FUNCTION__, 
	                                                    MbssCh,
	                                                    HcGetChannelByRf(pAd,RFIC_5GHZ),
	                                                    BgndScanCtrl->DfsZeroWaitChannel,
	                                                    GET_MT_ZEROWAIT_DFS_STATE(pAd)));
        }           
    }   
}
#endif /* BACKGROUND_SCAN_SUPPORT */

VOID DfsBFSoundingRecovery(
    IN PRTMP_ADAPTER pAd)
{
     if (pAd->Dot11_H.RDMode == RD_SILENCE_MODE)
     {
         mtRddControl(pAd, RDD_RESUME_BF, HW_RDD0, 0);
         MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Complete! \n", __FUNCTION__));
     }
}

#endif /*MT_DFS_SUPPORT*/

