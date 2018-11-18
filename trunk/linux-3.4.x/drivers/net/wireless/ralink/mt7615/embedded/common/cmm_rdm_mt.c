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

typedef int (*_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type) (UCHAR SyncNum, UCHAR monitored_Ch, UCHAR Bw);
typedef int (*_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type)(UCHAR SyncNum, UCHAR Bw, UCHAR monitored_Ch);
typedef int (*_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type) (UCHAR Bw80ChNum,PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList, UCHAR Bw40ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList, UCHAR Bw20ChNum,PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList);

_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type radar_detected_callback_func = NULL; 
_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type DfsCacTimeOutCallBack = NULL;
_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type DfsNopTimeOutCallBack = NULL;

void k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type callback_detect_collision_func)
{
    radar_detected_callback_func = callback_detect_collision_func;
}

void k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type callback_CAC_time_meet_func)
{
    DfsCacTimeOutCallBack = callback_CAC_time_meet_func;
}

void k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type callback_NOP_Timeout_func)
{
    DfsNopTimeOutCallBack = callback_NOP_Timeout_func;
}

EXPORT_SYMBOL(k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration);

static VOID ZeroWaitDfsEnable(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR bZeroWaitDfsCtrl;

	bZeroWaitDfsCtrl = msg->zerowait_dfs_ctrl_msg.Enable;
	
#ifdef BACKGROUND_SCAN_SUPPORT	
	DfsDedicatedDynamicCtrl(pAd, bZeroWaitDfsCtrl);
#endif
}

static VOID ZeroWaitDfsInitAvalChListUpdate(
    PRTMP_ADAPTER pAd,
    union dfs_zero_wait_msg *msg
)
{
	//UINT_8 i;
	UCHAR Bw80TotalChNum;
	UCHAR Bw40TotalChNum;
	UCHAR Bw20TotalChNum;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvalChList[DFS_AVAILABLE_LIST_CH_NUM];

	Bw80TotalChNum = msg->aval_channel_list_msg.Bw80TotalChNum;
	Bw40TotalChNum = msg->aval_channel_list_msg.Bw40TotalChNum;
	Bw20TotalChNum = msg->aval_channel_list_msg.Bw20TotalChNum;

	memcpy(Bw80AvalChList, 
		msg->aval_channel_list_msg.Bw80AvalChList, 
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST) 
	); 
    
	memcpy(Bw40AvalChList, 
		msg->aval_channel_list_msg.Bw40AvalChList, 
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST) 
	);   
    
	memcpy(Bw20AvalChList, 
		msg->aval_channel_list_msg.Bw20AvalChList, 
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST) 
	);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20ChNum: %d\n", Bw20TotalChNum));
#ifdef DFS_DBG_LOG_0
	for(i = 0; i < Bw20TotalChNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20 ChList[%d] Channel:%d\n", 
			i, Bw20AvalChList[i].Channel));
	}
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40ChNum: %d\n", Bw40TotalChNum));

#ifdef DFS_DBG_LOG_0
	for(i = 0; i < Bw40TotalChNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40 ChList[%d] Channel:%d\n", 
			i, Bw40AvalChList[i].Channel));
	}
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80ChNum: %d\n", Bw80TotalChNum));	

#ifdef DFS_DBG_LOG_0
	for(i = 0; i < Bw80TotalChNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80 ChList[%d] Channel:%d\n", 
			i, Bw80AvalChList[i].Channel));
	}
#endif
	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80TotalChNum, Bw80AvalChList,
	Bw40TotalChNum, Bw40AvalChList,
	Bw20TotalChNum, Bw20AvalChList);
		
}

static VOID ZeroWaitDfsMonitorChUpdate(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR SynNum;
	UCHAR Channel;
	UCHAR Bw;
	BOOLEAN doCAC;

	//TBD: Update Monitor channel by SyncNum and Bw 
	SynNum = msg->set_monitored_ch_msg.SyncNum; 
	Channel = msg->set_monitored_ch_msg.Channel;
	Bw = msg->set_monitored_ch_msg.Bw;
	doCAC = msg->set_monitored_ch_msg.doCAC;
	
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n", 
	__FUNCTION__, SynNum, Channel, Bw));

#ifdef BACKGROUND_SCAN_SUPPORT
	if(SynNum == RDD_BAND0)
		DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC);
	else
		DfsDedicatedOutBandSetChannel(pAd, Channel, Bw);
#endif	

}

static VOID ZeroWaitDfsMsgHandle(
	PRTMP_ADAPTER pAd, 
	UCHAR *msg
)
{
	switch (*msg)
	{
		case ZERO_WAIT_DFS_ENABLE:
		ZeroWaitDfsEnable(pAd, (union dfs_zero_wait_msg *)msg);
		break;

		case INIT_AVAL_CH_LIST_UPDATE:
		ZeroWaitDfsInitAvalChListUpdate(pAd, (union dfs_zero_wait_msg *)msg);            
		break;

		case MONITOR_CH_ASSIGN:
		ZeroWaitDfsMonitorChUpdate(pAd, (union dfs_zero_wait_msg *)msg);
		break;

		default:
		break;      
	}
}

INT ZeroWaitDfsCmdHandler(
	PRTMP_ADAPTER pAd, 
	RTMP_IOCTL_INPUT_STRUCT *wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;
    
	if (!wrq)
		return NDIS_STATUS_FAILURE;
    
	if (copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length))
	{
		status = -EFAULT;
	}
	else
	{
		ZeroWaitDfsMsgHandle(pAd, (CHAR*)&msg);
	}
    
	return status;
}

INT ZeroWaitDfsQueryCmdHandler(
	PRTMP_ADAPTER pAd, 
	RTMP_IOCTL_INPUT_STRUCT *wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;
	UINT_8 i, j, idx;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];

	UCHAR Bw80TotalChNum;
	UCHAR Bw40TotalChNum;
	UCHAR Bw20TotalChNum;

	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	//TBD: Please add fill the context of aval_channel_list_msg and add debug/trace log


	for(i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++)
	{
		for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;		
	}
	
	if(pAd->Dot11_H.RDMode == RD_SWITCHING_MODE)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel Lsit query fail during channel switch\n"));
		status = -EFAULT;
		return status;
	}
	
	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE);	

	for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
	{
		if(pDfsParam->AvailableBwChIdx[BW_20][j] != 0xff)
		{
			idx = pDfsParam->AvailableBwChIdx[BW_20][j];
			Bw20AvailableChList[j].Channel = pDfsParam->DfsChannelList[idx].Channel;
			Bw20AvailableChList[j].RadarHitCnt = pDfsParam->DfsChannelList[idx].NOPClrCnt;
		}
		else
			break;
	}	
	Bw20TotalChNum = j;

	for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
	{
		if(pDfsParam->AvailableBwChIdx[BW_40][j] != 0xff)
		{
			idx = pDfsParam->AvailableBwChIdx[BW_40][j];
			Bw40AvailableChList[j].Channel = pDfsParam->DfsChannelList[idx].Channel;
			Bw40AvailableChList[j].RadarHitCnt = pDfsParam->DfsChannelList[idx].NOPClrCnt;
		}
		else
			break;
	}	
	Bw40TotalChNum = j;

	for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
	{
		if(pDfsParam->AvailableBwChIdx[BW_80][j] != 0xff)
		{
			idx = pDfsParam->AvailableBwChIdx[BW_80][j];
			Bw80AvailableChList[j].Channel = pDfsParam->DfsChannelList[idx].Channel;
			Bw80AvailableChList[j].RadarHitCnt = pDfsParam->DfsChannelList[idx].NOPClrCnt;
		}
		else
			break;
	}	
	Bw80TotalChNum = j;

	msg.aval_channel_list_msg.Bw80TotalChNum = Bw80TotalChNum;
	msg.aval_channel_list_msg.Bw40TotalChNum = Bw40TotalChNum;
	msg.aval_channel_list_msg.Bw20TotalChNum = Bw20TotalChNum;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20ChNum: %d\n", msg.aval_channel_list_msg.Bw20TotalChNum));
	
#ifdef DFS_DBG_LOG_0
	for(i = 0; i < Bw20TotalChNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20 ChList[%d] Channel:%d, RadarHitCnt:%d\n", 
			i, Bw20AvailableChList[i].Channel, Bw20AvailableChList[i].RadarHitCnt));
	}
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40ChNum: %d\n", msg.aval_channel_list_msg.Bw40TotalChNum));

#ifdef DFS_DBG_LOG_0
	for(i = 0; i < Bw40TotalChNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40 ChList[%d] Channel:%d, RadarHitCnt:%d\n", 
			i, Bw40AvailableChList[i].Channel, Bw40AvailableChList[i].RadarHitCnt));
	}
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80ChNum: %d\n", msg.aval_channel_list_msg.Bw80TotalChNum));	

#ifdef DFS_DBG_LOG_0
	for(i = 0; i < Bw80TotalChNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80 ChList[%d] Channel:%d, RadarHitCnt:%d\n", 
			i, Bw80AvailableChList[i].Channel, Bw80AvailableChList[i].RadarHitCnt));
	}
#endif
	memcpy(msg.aval_channel_list_msg.Bw80AvalChList,
		Bw80AvailableChList, 
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST) 
	); 

	memcpy(msg.aval_channel_list_msg.Bw40AvalChList,
		Bw40AvailableChList, 
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST) 
	); 
	
	memcpy(msg.aval_channel_list_msg.Bw20AvalChList,
		Bw20AvailableChList, 
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST) 
	); 
	
	wrq->u.data.length = sizeof(union dfs_zero_wait_msg);

	if (copy_to_user(wrq->u.data.pointer, &msg, wrq->u.data.length))
	{
		status = -EFAULT;    
	}    

	return status;
}

static BOOLEAN AutoChannelSkipListCheck(
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

static UCHAR Get5GPrimChannel(
    IN PRTMP_ADAPTER	pAd)
{
    
	return HcGetChannelByRf(pAd,RFIC_5GHZ);
	
}

static VOID Set5GPrimChannel(
    IN PRTMP_ADAPTER	pAd, UCHAR Channel)
{
	if(HcUpdateChannel(pAd,Channel) !=0)
	{
		;
	}
}

static UCHAR CentToPrim(
	UCHAR Channel)
{	
	return Channel-2;
}

static BOOLEAN DfsCheckChAvailableByBw(
	UCHAR Channel, UCHAR Bw, PDFS_PARAM pDfsParam)
{
	UCHAR i = 0 , j = 0, k = 0;
	UCHAR* pBwChGroup = NULL;
	UCHAR BW40_CH_GROUP[][2] = {
	{36, 40}, {44, 48},
	{52, 56}, {60, 64},
	{100, 104}, {108, 112},
	{116, 120}, {124, 128},
	{132, 136}, {140, 144},
	{149, 153}, {157, 161}, {0,0}
	};

	UCHAR BW80_CH_GROUP[][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{0,0,0,0}
	};

	UCHAR BW160_CH_GROUP[][8] = {
	{36, 40, 44, 48, 52, 56, 60, 64},
	{100, 104, 108, 112, 116, 120, 124, 128}
	};

	if(Bw == BW_20)
		return TRUE;
	else if(Bw == BW_40)
	{
		pBwChGroup = &BW40_CH_GROUP[0][0];
		while (*pBwChGroup != 0)
		{	
			if(*pBwChGroup == Channel)
			{
				break;
			}
			i++;
			if(i > sizeof(BW40_CH_GROUP))
				return FALSE;
			pBwChGroup++;

		}
		for (j = 0; j < pDfsParam->ChannelListNum; j++)
		{
			if(pDfsParam->DfsChannelList[j].Channel == BW40_CH_GROUP[i/2][0])
				break;			
		}
  
		if (j == pDfsParam->ChannelListNum)  
			return FALSE; 
		else if(pDfsParam->DfsChannelList[j+1].Channel == BW40_CH_GROUP[i/2][1]) 
			return TRUE;
 	
	}
	else if(Bw == BW_80 || Bw == BW_8080)
	{
		pBwChGroup = &BW80_CH_GROUP[0][0];
		while (*pBwChGroup != 0)
		{	
			if(*pBwChGroup == Channel)
			{
				break;
			}
			i++;
			if(i > sizeof(BW80_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		for (j = 0; j < pDfsParam->ChannelListNum; j++)
		{
			if(pDfsParam->DfsChannelList[j].Channel == BW80_CH_GROUP[i/4][0])
				break;			
		}
		if (j == pDfsParam->ChannelListNum)
			return FALSE;
		else if( (pDfsParam->DfsChannelList[j+1].Channel == BW80_CH_GROUP[i/4][1])
		&&(pDfsParam->DfsChannelList[j+2].Channel == BW80_CH_GROUP[i/4][2])
		&&(pDfsParam->DfsChannelList[j+3].Channel == BW80_CH_GROUP[i/4][3])
		)
			return TRUE;

	}
	else if(Bw == BW_160)
	{
		pBwChGroup = &BW160_CH_GROUP[0][0];
		while (*pBwChGroup != 0)
		{	
			if(*pBwChGroup == Channel)
			{
				break;
			}
			
			i++;
			if(i > sizeof(BW160_CH_GROUP))
				return FALSE;			
			pBwChGroup++;
		}
		for (j = 0; j < pDfsParam->ChannelListNum; j++)
		{
			if(pDfsParam->DfsChannelList[j].Channel == BW160_CH_GROUP[i/8][0])
				break;			
		}
		if (j == pDfsParam->ChannelListNum)
			return FALSE;
		else
		{
			for(k = 1; k < 7 ; k++)
			{
				if(pDfsParam->DfsChannelList[j+k].Channel != BW160_CH_GROUP[i/8][k])
					return FALSE;
			}
			return TRUE;
		}	
	}	

	return FALSE;
}

static BOOLEAN ByPassChannelByBw(
	UCHAR Channel, UCHAR Bw, PDFS_PARAM pDfsParam)
{	
	UINT_8 i;
	BOOLEAN BwSupport = FALSE;

	for (i = 0; i < pDfsParam->ChannelListNum; i++)
	{
		if(Channel == pDfsParam->DfsChannelList[i].Channel)
		{

			if(Bw == BW_8080)
			{
				BwSupport = (pDfsParam->DfsChannelList[i].SupportBwBitMap) & BIT(BW_80);
			}	
			else
			{
				BwSupport = (pDfsParam->DfsChannelList[i].SupportBwBitMap) & BIT(Bw);
			}	
		}	
	}
	
	if(BwSupport)
		return FALSE;
	else
		return TRUE;

}

UCHAR DfsPrimToCent(
	UCHAR Channel, UCHAR Bw)
{	
	UINT_8 i = 0;

	UCHAR CH_EXT_ABOVE[] = {
	36, 44, 52, 60,
	100, 108, 116, 124,
	132, 140, 149, 157, 0
	};

	UCHAR CH_EXT_BELOW[] = {
	40, 48, 56, 64,
	104, 112, 120, 128,
	136, 144, 153, 161, 0
	};


	if (Bw == BW_20)
		return Channel;
	else if (Bw == BW_40)
	{
		while (CH_EXT_ABOVE[i] != 0)
		{	
			if(Channel == CH_EXT_ABOVE[i])
			{
				return Channel + 2;
	}
			else if(Channel == CH_EXT_BELOW[i])
			{
				return Channel - 2;
			}			
			i++;
		}			
	}
	else if (Bw == BW_80 || Bw == BW_8080)
		return vht_cent_ch_freq(Channel, VHT_BW_80);
		
	else if (Bw == BW_160)
		return vht_cent_ch_freq(Channel, VHT_BW_160);
		
	return Channel;
}

UCHAR DfsGetBgndParameter(
	IN PRTMP_ADAPTER pAd, UCHAR QueryParam)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;	
	switch (QueryParam)
	{
		case INBAND_CH:
			return pDfsParam->Band0Ch;
			break;
		case INBAND_BW:
			return pDfsParam->Bw;
			break;
		case OUTBAND_CH:
			return pDfsParam->OutBandCh;
			break;
		case OUTBAND_BW:	
			return pDfsParam->OutBandBw;
			break;
		case ORI_INBAND_CH:
			return pDfsParam->OrigInBandCh;
			break;
		case ORI_INBAND_BW:
			return pDfsParam->OrigInBandBw;
			break;
		default:
			return pDfsParam->Band0Ch;
			break;
	}
}


VOID DfsGetSysParameters(
    IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	UCHAR phy_bw = decide_phy_bw_by_channel(pAd, Get5GPrimChannel(pAd));
	
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
	pDfsParam->bDfsEnable = pAd->DfsParameter.bDfsEnable;
	pDfsParam->RDDurRegion = pAd->CommonCfg.RDDurRegion;

}

VOID DfsParamInit(
		IN PRTMP_ADAPTER	pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	pDfsParam->PrimBand = RDD_BAND0;
	pDfsParam->DfsChBand[0] = FALSE; //Smaller channel
	pDfsParam->DfsChBand[1] = FALSE; // Larger channel number
	pDfsParam->RadarDetected[0] = FALSE; //Smaller channel number
	pDfsParam->RadarDetected[1] = FALSE; // larger channel number
	pDfsParam->RadarDetectState = FALSE;
	pDfsParam->bNeedSetNewChList = FALSE;
	pDfsParam->bDfsCheck = FALSE;
	pDfsParam->bNoSwitchCh = FALSE;
	pDfsParam->bShowPulseInfo = FALSE;
	pDfsParam->bNoAvailableCh = FALSE;
	pDfsParam->RddCtrlState = RDD_DEFAULT;
	pDfsParam->bZeroWaitCacSecondHandle = FALSE;
	pDfsParam->bDBDCMode = pAd->CommonCfg.dbdc_mode;
	pAd->Dot11_H.DfsZeroWaitChMovingTime = 3;
	pDfsParam->bDfsIsScanRunning = FALSE;
	pDfsParam->bClass2DeauthDisable = FALSE;
	pDfsParam->bDedicatedZeroWaitSupport = FALSE;
	pDfsParam->OutBandCh = 0;
	pDfsParam->OutBandBw = 0;
	pDfsParam->bOutBandAvailable = FALSE;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bSetInBandBwChannelByExt = FALSE;
	pDfsParam->bSetOutBandBwChannelByExt = FALSE;
	
	for(i = 0; i < MAX_NUM_OF_CHANNELS; i++)
	{
		pDfsParam->DfsChannelList[i].SupportBwBitMap = 0x0;
	}	
	DfsStateMachineInit(pAd, &pAd->DfsParameter.DfsStatMachine, pAd->DfsParameter.DfsStateFunc);

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
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	UCHAR phy_bw = HcGetBwByRf(pAd,RFIC_5GHZ);
	value = simple_strtol(arg, 0, 10);

	if(value == 0)
	{	
	    ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
	    ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_NODETSTOP, HW_RDD0, 0);
		pDfsParam->bNoSwitchCh = TRUE;
	}
	else if(value == 1)
	{	     
        ret1= mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
        ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0);
        ret1= mtRddControl(pAd, RDD_NODETSTOP, HW_RDD1, 0);
        pDfsParam->bNoSwitchCh = TRUE;
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
        pDfsParam->bNoSwitchCh = TRUE;
	}
    else if(value == 3)
	{	
		ret1= mtRddControl(pAd, RDD_STOP, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
		ret1= mtRddControl(pAd, RDD_DETSTOP, HW_RDD0, 0);
		pDfsParam->bNoSwitchCh = TRUE;
	}
    else if(value == 4)
    {
        ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
        ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, 0);
        ret1 = mtRddControl(pAd, RDD_DETSTOP, HW_RDD1, 0);
        pDfsParam->bNoSwitchCh = TRUE;
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
		{
			;
		}
	    pDfsParam->bNoSwitchCh = TRUE;
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
			{
				;
			}	
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

	pAd->Dot11_H.ChMovingTime = value;
	return TRUE;
}

INT Set_RDDReport_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg)
{
	ULONG value, ret;
	value = simple_strtol(arg, 0, 10);
	/*WrapDfsRddReportHandle(pAd, value);*/
	ret = mtRddControl(pAd, RDD_RADAR_EMULATE, value, 0);
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
	HcGetBwByRf(pAd, RFIC_5GHZ)));
   
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

INT Show_DfsNonOccupancy_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("[%s]: \n", __FUNCTION__));  
	for(i = 0; i < pDfsParam->ChannelListNum; i++)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("DfsChannelList[%d].Channel = %d, NonOccupancy = %d, NOPClrCnt = %d, NOPSetByBw = %d, NOPSaveForClear is %d, SuuportBwBitMap is %d\n",
		i, 
		pDfsParam->DfsChannelList[i].Channel,
		pDfsParam->DfsChannelList[i].NonOccupancy,
		pDfsParam->DfsChannelList[i].NOPClrCnt,
		pDfsParam->DfsChannelList[i].NOPSetByBw,
		pDfsParam->DfsChannelList[i].NOPSaveForClear,
		pDfsParam->DfsChannelList[i].SupportBwBitMap));
	}

	return TRUE;    
}

INT Set_DfsNOP_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{

	ULONG value;	
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	value = simple_strtol(arg, 0, 10);

	for(i = 0; i < pDfsParam->ChannelListNum; i++)
	{
		pDfsParam->DfsChannelList[i].NonOccupancy= value;
		pDfsParam->DfsChannelList[i].NOPClrCnt = 0;
		pDfsParam->DfsChannelList[i].NOPSetByBw = 0;
	} 
	
	return TRUE;    

}

INT Set_DfsPulseInfoMode_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;	
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	value = simple_strtol(arg, 0, 10);
	pDfsParam->bShowPulseInfo = TRUE;

	mtRddControl(pAd, RDD_PULSEDBG, HW_RDD0, 0); 
	return TRUE;    
}

INT Set_DfsPulseInfoRead_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg)
{
	ULONG value;	
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	value = simple_strtol(arg, 0, 10);
	pDfsParam->bShowPulseInfo = FALSE;
 
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
    PDFS_PARAM pDfsParam = &pAd->DfsParameter;

    Value = (UCHAR) simple_strtol(arg, 0, 10);

    pDfsParam->DfsZeroWaitCacTime = Value;

    return TRUE; 
}

INT Set_DedicatedBwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	UCHAR SynNum = 0, Channel = 0, Bw = 0, doCAC = 1;
	
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]\n", __FUNCTION__));

	for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{	
			case 0: /* Set Syn Num*/
				SynNum = simple_strtol(value, 0, 10);
				break;
			case 1: /* Set InBand ControlChannel */
				Channel = simple_strtol(value, 0, 10);
				break;
			case 2: /* Set InBand Bw*/
				Bw = simple_strtol(value, 0, 10);
				break;		
			case 3: /* Set doCAC*/
				doCAC = simple_strtol(value, 0, 10);
				break;
			default:
				break;
		}
	}

#ifdef BACKGROUND_SCAN_SUPPORT
		if(SynNum == RDD_BAND0)
			DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC);
		else
			DfsDedicatedOutBandSetChannel(pAd, Channel, Bw);
#endif	

	return TRUE;
}
			
INT Set_DfsZeroWaitDynamicCtrl_Proc(
		RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;
			
	Value = (UCHAR) simple_strtol(arg, 0, 10);

#ifdef BACKGROUND_SCAN_SUPPORT
	DfsDedicatedDynamicCtrl(pAd, Value);			
#endif

	return TRUE; 
}

VOID DfsDedicatedExclude(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	DfsGetSysParameters(pAd);
	if(pDfsParam->bDedicatedZeroWaitSupport == TRUE)
	{
		if(pDfsParam->Bw == BW_160 || pDfsParam->Bw == BW_8080)
		{
			pDfsParam->bDedicatedZeroWaitSupport = FALSE;
		}
	}	
	
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
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];		

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst == SST_ASSOC))
		{
			TxTP = pEntry->AvgTxBytes >> 17;
#ifdef DFS_DBG_LOG_3
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Check TXTP: %d. \n", TxTP));
#endif			
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
				MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_TP_HIGH_LOW, 0, NULL, 0);
			}			
			
		}
	}
}

BOOLEAN DfsEnterSilence(
    IN PRTMP_ADAPTER pAd)
{
    PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	return pDfsParam->bDfsEnable;
}

VOID DfsSetCalibration(
	    IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal)
{
	if(!DisableDfsCal)
		;
	else
	{
		mtRddControl(pAd, RDD_DFSCAL, HW_RDD0, 0);    
	}	
}

VOID DfsZeroWaitTPRatio(
    RTMP_ADAPTER *pAd, UCHAR Ratio)
{    
	if(Ratio <= 1)
	{
 		Ratio = 30;
	}	
	if(Ratio >=100)
		Ratio = 100;
	MtCmdRddCtrl(pAd, RDD_THROUGHPUT_CFG, 0, 0, Ratio);
	
#ifdef DFS_DBG_LOG_0
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("[%s] Final Ratio = %d\n", 
				__FUNCTION__,
				Ratio));
#endif
}

VOID DfsSetZeroWaitCacSecond(
	    IN PRTMP_ADAPTER pAd)
{
    PDFS_PARAM pDfsParam = &pAd->DfsParameter;
    pDfsParam->bZeroWaitCacSecondHandle = TRUE;
}

BOOLEAN DfsRadarChannelCheck(
    IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	UCHAR phy_bw = 0;//HcGetBwByRf(pAd,RFIC_5GHZ);
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

VOID DfsSetNewChList(
    IN PRTMP_ADAPTER pAd)
{
    PDFS_PARAM pDfsParam = &pAd->DfsParameter;	
	pDfsParam->bNeedSetNewChList = TRUE;
}

VOID DfsCacEndUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	
	mtRddControl(pAd, RDD_CACEND, HW_RDD0, 0);

	pAd->DfsParameter.DfsStatMachine.CurrState = DFS_INSERVICE_MONITOR;
	if (DfsCacTimeOutCallBack)
	{
		DfsCacTimeOutCallBack(RDD_BAND0, pDfsParam->Band0Ch, pDfsParam->Bw);
	}
}

VOID DfsChannelSwitchTimeoutAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{

#ifdef BACKGROUND_SCAN_SUPPORT
	DedicatedZeroWaitStop(pAd);
#endif

	APStopByRf(pAd, RFIC_5GHZ);
	if(DfsStopWifiCheck(pAd))
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n",__FUNCTION__));
		return;  
	}    
	APStartUpByRf(pAd, RFIC_5GHZ);

	if (pAd->CommonCfg.dbdc_mode)
	{
		MtCmdSetDfsTxStart(pAd, DBDC_BAND1);
        }
	else
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);

	DfsSetClass2DeauthDisable(pAd, FALSE);

	DfsSetCacRemainingTime(pAd);

#ifdef BACKGROUND_SCAN_SUPPORT	
	if(IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd) 
	 && !DfsDedicatedCheckChBwValid(pAd, GET_BGND_PARAM(pAd, ORI_INBAND_CH), GET_BGND_PARAM(pAd, ORI_INBAND_BW)))
	{
		ZeroWait_DFS_collision_report(pAd, HW_RDD0, GET_BGND_PARAM(pAd, ORI_INBAND_CH), GET_BGND_PARAM(pAd, ORI_INBAND_BW));
	}
#endif	
	/*DfsDedicatedScanStart(pAd);*/
}

VOID DfsCacNormalStart(
    IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;	
#ifdef BACKGROUND_SCAN_SUPPORT    
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
#endif

	DfsGetSysParameters(pAd);

	if ((pAd->CommonCfg.RDDurRegion == CE)
	 && DfsCacRestrictBand(pAd, pDfsParam, pDfsParam->Bw, pDfsParam->Band0Ch, pDfsParam->Band1Ch))
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
	IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh)
{
#ifdef DOT11_VHT_AC

	if(Bw == BW_8080)
	{
		return (RESTRICTION_BAND_1(pAd, Ch, Bw) || RESTRICTION_BAND_1(pAd, SecCh, Bw));   		
	}
	else if ((Bw == BW_160) && ((Ch >= GROUP3_LOWER) && (Ch <= RESTRICTION_BAND_HIGH)))
		return TRUE;
	else
#endif		
	{
		if (strncmp(pAd->CommonCfg.CountryCode, "KR", 2) == 0)
		{
			return RESTRICTION_BAND_KOREA(pAd, Ch, Bw);    
		}
		else
		{
			return RESTRICTION_BAND_1(pAd, Ch, Bw);
		}
	}
}

VOID DfsBuildChannelList(
    IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	if(pDfsParam->bNeedSetNewChList == TRUE)
	{
		pDfsParam->ChannelListNum = pAd->ChannelListNum;
		pDfsParam->RDDurRegion = pAd->CommonCfg.RDDurRegion;
		for (i = 0; i < pDfsParam->ChannelListNum; i++)
		{
			pDfsParam->DfsChannelList[i].Channel = pAd->ChannelList[i].Channel;
			pDfsParam->DfsChannelList[i].NonOccupancy = pAd->ChannelList[i].RemainingTimeForUse;
		}

		for (i = 0; i < pDfsParam->ChannelListNum; i++)
		{
	
			if(DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_20, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x01;
			if(DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_40, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x02;
			if(DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_80, pDfsParam)
			|| DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_8080, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x04;
			if(DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_160, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x08;						
		}

	}
	
	DfsBuildChannelGroupByBw(pAd);
	pDfsParam->bNeedSetNewChList = FALSE;
}

VOID DfsBuildChannelGroupByBw(
    IN PRTMP_ADAPTER pAd)
{	
	UINT_8 i/*, j*/;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	INT_8 BW40GroupIdx = -1;
	INT_8 BW80GroupIdx = -1;
	INT_8 BW160GroupIdx = -1;
	INT_8 BW40GroupMemberCnt = 0;
	INT_8 BW80GroupMemberCnt = 0;
	INT_8 BW160GroupMemberCnt = 0;
	UINT_8 PreviousBW40CentCh = 0xff;
	UINT_8 PreviousBW80CentCh = 0xff;
	UINT_8 PreviousBW160CentCh = 0xff;
		
	for (i = 0; i < pDfsParam->ChannelListNum; i++)
	{		
		if(!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
			continue;
		if(!ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_40, pDfsParam))
		{
			if(DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_40) != PreviousBW40CentCh)
			{
				BW40GroupMemberCnt = 0;
				if((++BW40GroupIdx < DFS_BW40_GROUP_NUM) && (BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM))
					pDfsParam->Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = i;
			}
			else
			{
				if((BW40GroupIdx < DFS_BW40_GROUP_NUM) && (++BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM))
					pDfsParam->Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = i;
			}
		
			PreviousBW40CentCh = DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_40);
		}

		if(!ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_80, pDfsParam))
		{
			if(DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_80) != PreviousBW80CentCh)
			{
				BW80GroupMemberCnt = 0;
				if((++BW80GroupIdx < DFS_BW80_GROUP_NUM) && (BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM))
					pDfsParam->Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = i;
			}
			else
			{
				if((BW80GroupIdx < DFS_BW80_GROUP_NUM) && (++BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM))
				pDfsParam->Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = i;
			}
		
			PreviousBW80CentCh = DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_80);
		}		
		if(!ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_160, pDfsParam))
		{
			if(DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_160) != PreviousBW160CentCh)
			{
				BW160GroupMemberCnt = 0;
				if((++BW160GroupIdx < DFS_BW160_GROUP_NUM) && (BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM))
					pDfsParam->Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = i;
			}
			else
			{
				if((BW160GroupIdx < DFS_BW160_GROUP_NUM) && (++BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM))
					pDfsParam->Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = i;
			}
		
			PreviousBW160CentCh = DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_160);
		}

	}

}

BOOLEAN DfsCheckBwGroupAllAvailable(
    UCHAR CheckChIdx, UCHAR Bw, IN PDFS_PARAM pDfsParam)
{
	UCHAR* pBwxxGroupIdx = NULL;
	UCHAR i, j;
	UCHAR GroupNum = 4, BwxxPrimNum = 4;
	
	if (Bw == BW_20)
		return TRUE;
	else if(Bw == BW_40)
	{
		pBwxxGroupIdx = &pDfsParam->Bw40GroupIdx[0][0];
		GroupNum = DFS_BW40_GROUP_NUM;
		BwxxPrimNum = DFS_BW40_PRIMCH_NUM;
	}	
	else if (Bw == BW_80)
	{
		pBwxxGroupIdx = &pDfsParam->Bw80GroupIdx[0][0];
		GroupNum = DFS_BW80_GROUP_NUM;
		BwxxPrimNum = DFS_BW80_PRIMCH_NUM;
	}	
	else if (Bw == BW_160)
	{
		pBwxxGroupIdx = &pDfsParam->Bw160GroupIdx[0][0];
		GroupNum = DFS_BW160_GROUP_NUM;
		BwxxPrimNum = DFS_BW160_PRIMCH_NUM;
	}
	
	
	for(i = 0; i < GroupNum*BwxxPrimNum; i++)		
	{

			if( *pBwxxGroupIdx == CheckChIdx)
			{
				break;
			}	
			pBwxxGroupIdx++;

	}
	if(i >= GroupNum*BwxxPrimNum)
		return FALSE;

	j = i%BwxxPrimNum;
	i = i/BwxxPrimNum;

	pBwxxGroupIdx = pBwxxGroupIdx - j;

	for(j = 0; j < BwxxPrimNum; j++)
	{
		if(pDfsParam->DfsChannelList[*pBwxxGroupIdx].NonOccupancy != 0)
			return FALSE;
		if((pDfsParam->DfsChannelList[*pBwxxGroupIdx].NonOccupancy == 0)
		 && (pDfsParam->DfsChannelList[*pBwxxGroupIdx].NOPClrCnt!= 0)
		 && (pDfsParam->DfsChannelList[*pBwxxGroupIdx].NOPSetByBw <= Bw)
		 )
			return FALSE;
	 
		
		pBwxxGroupIdx++;	
	}		
	
	return TRUE;			
}


BOOLEAN DfsSwitchCheck(
		IN PRTMP_ADAPTER	pAd,
		UCHAR Channel
		)
{		
	if ((pAd->Dot11_H.RDMode == RD_SILENCE_MODE) && (Channel >14))
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSwitchCheck]: DFS ByPass TX calibration.\n"));
		return TRUE;
	}	
	else
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSwitchCheck]:  Not DFS ByPass TX calibration.\n"));
		return FALSE;
	}	
}

BOOLEAN DfsStopWifiCheck(
    IN PRTMP_ADAPTER	pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	return (pDfsParam->bNoAvailableCh == TRUE);
}

VOID DfsNonOccupancyUpdate(
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		pAd->ChannelList[i].RemainingTimeForUse = pDfsParam->DfsChannelList[i].NonOccupancy;
	}
}

VOID DfsNonOccupancyCountDown( /*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	BOOLEAN Band0Available = FALSE, Band1Available = FALSE;
    
	for (i=0; i < pDfsParam->ChannelListNum; i++)
	{
		if (pDfsParam->DfsChannelList[i].NonOccupancy > 0)
			pDfsParam->DfsChannelList[i].NonOccupancy --;
		
		if (pDfsParam->DfsChannelList[i].NOPSaveForClear > 0)
			pDfsParam->DfsChannelList[i].NOPSaveForClear --;
		
		else if ((pDfsParam->DfsChannelList[i].NOPSaveForClear == 0) && (pDfsParam->DfsChannelList[i].NOPClrCnt!= 0))
			pDfsParam->DfsChannelList[i].NOPClrCnt = 0;		
	}

	if(pDfsParam->bNoAvailableCh == TRUE)
	{
		for (i = 0; i < pAd->ChannelListNum; i++)
		{
			if ((pDfsParam->Bw!= BW_8080) && (pAd->ChannelList[i].Channel == pDfsParam->PrimCh))
			{
				if (pDfsParam->DfsChannelList[i].NonOccupancy == 0)
				{
					Band0Available = TRUE;
				}
			}
			if((pDfsParam->Bw == BW_8080) && (pAd->ChannelList[i].Channel == pDfsParam->Band0Ch))
			{
				if (pDfsParam->DfsChannelList[i].NonOccupancy == 0)
				{
					Band0Available = TRUE;
				}	
			}
			if((pDfsParam->Bw == BW_8080) && (pAd->ChannelList[i].Channel == pDfsParam->Band1Ch))
			{
				if (pDfsParam->DfsChannelList[i].NonOccupancy == 0)
				{
					Band1Available = TRUE;
				}	
			}
		}

		if(((pDfsParam->Bw!= BW_8080) && (Band0Available == TRUE))
		|| ((pDfsParam->Bw == BW_8080) && (Band0Available == TRUE) && (Band1Available == TRUE)))
		{
			pDfsParam->bNoAvailableCh = FALSE;
			pDfsParam->DfsStatMachine.CurrState = DFS_BEFORE_SWITCH;
			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CHAN_SWITCH_TIMEOUT, 0, NULL, 0);
		}
	}
}

VOID WrapDfsSetNonOccupancy( /*Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;	
	DfsGetSysParameters(pAd);

	DfsSetNonOccupancy(pAd, pDfsParam);	
}

VOID DfsSetNonOccupancy( /*Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam)
{
	    UINT_8 i;
	    UINT_8 TargetCh, TargetBw, TargetChDfsBand; /*Only Bw20, Bw40, Bw80 refer to this value*/
		    
	    if(pDfsParam->Dot11_H.RDMode == RD_SWITCHING_MODE) 
		return; 
	
	    if((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (pDfsParam->RadarDetected[1] == TRUE))
	    {
		    TargetCh = pDfsParam->OutBandCh;
		    TargetBw = pDfsParam->OutBandBw;
		    TargetChDfsBand = pDfsParam->DfsChBand[1];
	    }	    
	    else
	    {
		    TargetCh = pDfsParam->Band0Ch;
		    TargetBw = pDfsParam->Bw;
		    TargetChDfsBand = pDfsParam->DfsChBand[0];
	    }
	    
	    if((pDfsParam->Bw == BW_160) && (pDfsParam->DfsChBand[0] || pDfsParam->DfsChBand[1]))
	    {
		for(i=0; i<pDfsParam->ChannelListNum; i++)
		{
			    if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_160) == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_160))	    
			    {
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = pDfsParam->Bw;
		    }
	    }
	    }
	    if((TargetBw == BW_80) && TargetChDfsBand)
	    {
		for(i=0; i<pDfsParam->ChannelListNum; i++)
		{
			    if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_80) == vht_cent_ch_freq(TargetCh, VHT_BW_80))  
			    {
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			    }
		    }
	    }
	    else if(TargetBw == BW_40 && TargetChDfsBand)
	    {
		for(i=0; i<pDfsParam->ChannelListNum; i++)
		{
			    if((TargetCh == pDfsParam->DfsChannelList[i].Channel))
		    {
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			    }
			    else if(((TargetCh)>>2 & 1) && ((pDfsParam->DfsChannelList[i].Channel - TargetCh) == 4))
			    {
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			    }
			    else if(!((TargetCh)>>2 & 1) && ((TargetCh - pDfsParam->DfsChannelList[i].Channel) == 4))
			    {
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			    }
			    else
				;
		    }		    
	    }
	    else if(TargetBw == BW_20 && TargetChDfsBand)
	    {
	    for(i=0; i<pDfsParam->ChannelListNum; i++)
	    {
			    if((TargetCh == pDfsParam->DfsChannelList[i].Channel))
		{
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			    }
		    }
	    }
	    else if(pDfsParam->Bw == BW_8080 && pDfsParam->DfsChBand[0] && pDfsParam->RadarDetected[0])
	    {	     
	    for(i=0; i<pDfsParam->ChannelListNum; i++)
	    {
		if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_8080) == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080))	    
		{
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = pDfsParam->Bw;
		}
	    }	    
	    }
	
	else if(pDfsParam->Bw == BW_8080 && pDfsParam->DfsChBand[1] && pDfsParam->RadarDetected[1])
	{
		for(i=0; i<pDfsParam->ChannelListNum; i++)
		    {
			if(vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_8080) == vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080))	    
			{
				    pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				    pDfsParam->DfsChannelList[i].NOPSetByBw = pDfsParam->Bw;
			    }
		    }	    
	    }
			    
	    else
		;   

}

VOID WrapDfsRddReportHandle( /*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx)
{
	UCHAR PhyMode;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	UCHAR NextCh = 0;
	UCHAR NextBw = 0;
	UCHAR KeepBw = 0;
#ifdef BACKGROUND_SCAN_SUPPORT
	UCHAR Channel = 0;
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg = &(BgndScanCtrl->CurrentSwChCfg[0]);
#endif /* BACKGROUND_SCAN_SUPPORT */


	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRddReportHandle]:  Radar detected !!!!!!!!!!!!!!!!!\n")); 		
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRddReportHandle]:  ucRddIdx: %d\n", ucRddIdx)); 

    
	if(pDfsParam->bNoSwitchCh)
	{
		return;
	}
	if(pDfsParam->Band0Ch > 14)
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_5GHZ);
	else
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_24GHZ);

	if((pDfsParam->Bw == BW_8080) && (pDfsParam->PrimBand == RDD_BAND1)) /*Prim in idx 4~7. WorkAround for "Always set BB Prim 0~3 in IBF issue"*/
		ucRddIdx = 1 - ucRddIdx; 
	if((pDfsParam->Bw == BW_160) 
	&& (pDfsParam->PrimCh >= GROUP4_LOWER && pDfsParam->PrimCh <= GROUP4_UPPER))
		ucRddIdx = 1 - ucRddIdx;
	
	if(!(DfsRddReportHandle(pDfsParam, ucRddIdx) || (pDfsParam->bZeroWaitCacSecondHandle == TRUE)))
		return;

	/*ByPass these setting when Dedicated DFS zero wait, SynB/Band1 detect radar*/
	if((pDfsParam->bDedicatedZeroWaitSupport == TRUE)
	&& (pDfsParam->RadarDetected[1] == TRUE))
		;
	else
	{
		DfsSetClass2DeauthDisable(pAd, TRUE);

		if(pDfsParam->Dot11_H.RDMode == RD_SILENCE_MODE)
		pAd->Dot11_H.RDCount = 0;			
	}
		
	WrapDfsSetNonOccupancy(pAd);		

#ifdef BACKGROUND_SCAN_SUPPORT
        if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) && 
            CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) &&
            (pDfsParam->bZeroWaitCacSecondHandle == FALSE))
        {
            /* Skip CH uniform spreading and switch to Ori Non-DFS CH by ZeroWaitStop flow */
            UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_RADAR_DETECT);     
            BgndScanCtrl->RadarDetected = TRUE;            

            MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_RDD_TIMEOUT, 0, NULL, 0);
            RTMP_MLME_HANDLER(pAd);      
            return;
        }
        else
#endif /* BACKGROUND_SCAN_SUPPORT */
        {
#ifdef BACKGROUND_SCAN_SUPPORT
		/*For Dedicated DFS zero wait, SynB/Band1 detect radar, choose another channel as Band1*/
		if(pDfsParam->bDedicatedZeroWaitSupport == TRUE)
		{
			if(pDfsParam->RadarDetected[1] == TRUE)
			{
#ifdef DFS_DBG_LOG			
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDD1 detect. Please switch to another outBand channel\n")); 
#endif				
				/*MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_RADAR_FOUND, 0, NULL, 0); 
				RTMP_MLME_HANDLER(pAd);*/
				ZeroWait_DFS_collision_report(pAd, HW_RDD1, pDfsParam->OutBandCh, pDfsParam->OutBandBw);

				return;
			}				
			if((pDfsParam->RadarDetected[0] == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC))
			{
#ifdef DFS_DBG_LOG			
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDD0 detect. OutBand channel come back to InBand\n"));
#endif
			}			
		}
#endif
		if(pDfsParam->RadarDetected[0] == TRUE)
		{
			pDfsParam->OrigInBandCh = pDfsParam->Band0Ch;
			pDfsParam->OrigInBandBw = pDfsParam->Bw;
		}
		
		/* Detected radar & InsrvMonitor state & uniform Ch Sel 
		Get new CH & save into pDfsParam->Band0Ch temporary */
		pDfsParam->bZeroWaitCacSecondHandle = FALSE;

		/*Keep BW info because the BW may be changed after selecting a new channel*/
		KeepBw = pDfsParam->Bw;

		WrapDfsSelectChannel(pAd);		

#ifdef BACKGROUND_SCAN_SUPPORT
		if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
			BackgroundScanCancelAction(pAd, NULL); // Cannel background scan.

#ifdef DFS_DBG_LOG
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]PrimCh: %d, Band0Ch:%d, Band1Ch:%d\n", 
							__FUNCTION__, pDfsParam->PrimCh, pDfsParam->Band0Ch, pDfsParam->Band1Ch));
#endif

		BgndScanCtrl->RadarDetected = TRUE;
		
		/* Uniform CH is a DFS CH */ 
		if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) && BgndScanCtrl->BgndScanSupport)
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
					Channel = WrapDfsRandomSelectChannel(pAd, TRUE, 0); /* Skip DFS CH */
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]New Band0 Channel %d for DFS zero wait! \n", __FUNCTION__, Channel));
	
					ret = HcUpdateChannel(pAd,Channel);
					if(ret < 0 )
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

		/*Adjust Bw*/
#ifdef BACKGROUND_SCAN_SUPPORT
		if((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		{
			DfsAdjustBwSetting(pAd, pDfsParam->Bw, pDfsParam->OutBandBw);
			NextBw = pDfsParam->OutBandBw;
		}	
		else
#endif		
		{
			DfsAdjustBwSetting(pAd, KeepBw, pDfsParam->Bw);
			NextBw = pDfsParam->Bw;
		}

	if (pDfsParam->Dot11_H.RDMode == RD_NORMAL_MODE)
	{    
		pDfsParam->DfsChBand[0] = FALSE;
		pDfsParam->DfsChBand[1] = FALSE;		
		pDfsParam->RadarDetected[0] = FALSE;
		pDfsParam->RadarDetected[1] = FALSE;	

		pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
		pAd->Dot11_H.CSCount = 0;
		pAd->Dot11_H.new_channel = NextCh; //Zero Wait Band0 Channel
		pAd->Dot11_H.org_ch = HcGetChannelByRf(pAd, RFIC_5GHZ);
		Set5GPrimChannel(pAd, NextCh);

#ifdef BACKGROUND_SCAN_SUPPORT 
		//Set state into radar detected state for uniform CH is Non-DFS CH 
		if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)&&(CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) == FALSE))
		{
			UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_RADAR_DETECT);
		}
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef DFS_DBG_LOG
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" \x1b[1;33m [%s] RD_NORMAL_MODE. Update Uniform Ch=%d, Bw=%d \x1b[m \n", 
						__FUNCTION__, NextCh, NextBw));
#endif
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
		pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
		
#ifdef DFS_DBG_LOG		
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_SILENCE_MODE. Update Uniform Ch=%d, Bw=%d \x1b[m \n", 
				__FUNCTION__, NextCh, NextBw));
#endif		
		pDfsParam->DfsStatMachine.CurrState = DFS_BEFORE_SWITCH;
		MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CHAN_SWITCH_TIMEOUT, 0, NULL, 0);			
		RTMP_MLME_HANDLER(pAd);
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
	
    if(ucRddIdx == 1 && (pDfsParam->RadarDetected[1] == FALSE) && (pDfsParam->DfsChBand[1])
	&&(pDfsParam->Dot11_H.RDMode != RD_SWITCHING_MODE))
    {
        pDfsParam->RadarDetected[1] = TRUE;		
        RadarDetected = TRUE;
	}

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
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
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

	if(pDfsParam->Bw == BW_8080)
	{
		if(pDfsParam->Band0Ch < pDfsParam->Band1Ch)
		{ 
			if(pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0])
			{
				pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->Band1Ch);
				while(vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080) 
				== vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080))
				{
					pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->Band1Ch);
				}
			}
			if(pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[1])
			{
				pDfsParam->Band1Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->Band0Ch);
				while(vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080) 
				== vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080))
					{
					    pDfsParam->Band1Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->Band0Ch);
					}	                
	        	}
		}

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
			pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, 0);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: Single band, selected is %d\n",pDfsParam->Band0Ch));					    
		}
		else if(pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0])
		{
#ifdef BACKGROUND_SCAN_SUPPORT
			if((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC))
				pDfsParam->Band0Ch = pDfsParam->OutBandCh;
			else
#endif  	    
			pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, 0);	
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: Single band, selected is %d\n",pDfsParam->Band0Ch));
		}
		else
			;
		
		if(pDfsParam->bDBDCMode)
		{
			if(pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[0])
			{
				pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, 0);	
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: DBDC band, selected is %d\n",pDfsParam->Band0Ch));            
			}
		}
		/* DFS zero wait case */
		if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
		{
			if(pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[0])
			{
				pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, 0);
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
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh, UCHAR Bw8080ChAvoid)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	DfsGetSysParameters(pAd);

	return DfsRandomSelectChannel(pAd, pDfsParam, bSkipDfsCh, Bw8080ChAvoid);
}


UCHAR DfsRandomSelectChannel( /*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, BOOLEAN bSkipDfsCh, UCHAR Bw8080ChAvoid)
{

	UINT_8 i, cnt, ch;
	UINT_8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	UINT_16 BwChannel;

	cnt = 0;
	
	if(pDfsParam->bIEEE80211H)		
	{
		for (i = 0; i < pDfsParam->ChannelListNum; i++)
		{
			
			if (pDfsParam->DfsChannelList[i].NonOccupancy)
				continue;

			if (AutoChannelSkipListCheck(pAd, pDfsParam->DfsChannelList[i].Channel) == TRUE)
				continue;

			if(!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
				continue;		

			/* Skip DFS channel for DFS zero wait using case */
			if (bSkipDfsCh)
			{
				if (RadarChannelCheck(pAd, pDfsParam->DfsChannelList[i].Channel))
					continue;
			}

			if(ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw, pDfsParam))
				continue;

			if((Bw8080ChAvoid != 0) 
			&& DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_80) == DfsPrimToCent(Bw8080ChAvoid, BW_80))
				continue;

			if(IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd)
			&& !DfsDedicatedCheckChBwValid(pAd, pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw))
				continue;
			
			/* Store available channel to temp list */
			TempChList[cnt++] = pDfsParam->DfsChannelList[i].Channel;

				
		}
		if(cnt)
		{
			ch = TempChList[RandomByte(pAd)%cnt];
		}
		else
		{			

			ch = 0;
			pDfsParam->bNoAvailableCh = FALSE;
			if(pDfsParam->Bw != BW_8080)
			{
				BwChannel = DfsBwChQueryListOrDefault(pAd, BW_160, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, FALSE);
				ch = BwChannel & 0xff;
				pDfsParam->Bw = BwChannel>>8;
				if(ch == 0) /*No available channel to use*/
				{
					pDfsParam->bNoAvailableCh = TRUE;
#ifdef DFS_DBG_LOG_0
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Currently no immediately available Channel.\n",
					__FUNCTION__));
#endif					
					i = RandomByte(pAd) % (pDfsParam->ChannelListNum);
					while(ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_8080, pDfsParam))
					{
						i = RandomByte(pAd) % (pDfsParam->ChannelListNum);
					}
					ch = pDfsParam->DfsChannelList[i].Channel;
#ifdef DFS_DBG_LOG_0
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Select Channel %d.\n",
					__FUNCTION__, ch));
#endif					
				}
					
			}
			else
			{
				pDfsParam->bNoAvailableCh = TRUE;
		
				i = RandomByte(pAd) % (pDfsParam->ChannelListNum);
				while(ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_8080, pDfsParam))
				{
					i = RandomByte(pAd) % (pDfsParam->ChannelListNum);
				}
				ch = pDfsParam->DfsChannelList[i].Channel;	
			
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

USHORT DfsBwChQueryListOrDefault( /*Query current available BW & Channel list or select default*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, UCHAR level, BOOLEAN bDefaultSelect, BOOLEAN SkipNonDfsCh)
{
	USHORT BwChannel = 0;
	UINT_8 ch = 0;
	UINT_8 i,SelectIdx;
	UINT_8 AvailableChCnt = 0;
	
	DfsGetSysParameters(pAd);
	if(pDfsParam->bIEEE80211H == FALSE)
	{
		ch = pDfsParam->DfsChannelList[RandomByte(pAd)%pDfsParam->ChannelListNum].Channel;
		BwChannel |= ch;
		BwChannel |= (Bw << 8);
		return BwChannel;
	}
		
	for (i = 0; i < pDfsParam->ChannelListNum; i++)
	{

		if (AutoChannelSkipListCheck(pAd, pDfsParam->DfsChannelList[i].Channel) == TRUE)
			continue;
		if((SkipNonDfsCh == TRUE) && (!RadarChannelCheck(pAd, pDfsParam->DfsChannelList[i].Channel)))
			continue;
		if(!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
			continue;

		if(ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, Bw, pDfsParam))
			continue;

		if((pDfsParam->DfsChannelList[i].NonOccupancy == 0)
		 && (pDfsParam->DfsChannelList[i].NOPClrCnt!= 0)
		 && (pDfsParam->DfsChannelList[i].NOPSetByBw == Bw)
		)
			continue;

		if(DfsCheckBwGroupAllAvailable(i, Bw, pDfsParam) == FALSE)
		{
			continue;
		}
		if(DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw) == DfsPrimToCent(pDfsParam->Band0Ch, pDfsParam->Bw))
			continue;
		
		if(DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, Bw) == DfsPrimToCent(pDfsParam->Band0Ch, Bw))
			continue;		
		
		if((level == DFS_BW_CH_QUERY_LEVEL1)
		&& ((pDfsParam->DfsChannelList[i].NonOccupancy == 0) && (pDfsParam->DfsChannelList[i].NOPClrCnt == 0)))
			pDfsParam->AvailableBwChIdx[Bw][AvailableChCnt++] = i;
		if((level == DFS_BW_CH_QUERY_LEVEL2)
		&& (pDfsParam->DfsChannelList[i].NonOccupancy == 0))
			pDfsParam->AvailableBwChIdx[Bw][AvailableChCnt++] = i;
	}

	if(AvailableChCnt > 0) 
	{
	
		/*randomly select a ch for this BW*/
		SelectIdx = pDfsParam->AvailableBwChIdx[Bw][RandomByte(pAd)%AvailableChCnt];
		BwChannel |= pDfsParam->DfsChannelList[SelectIdx].Channel;
		BwChannel |= (Bw << 8);
		return BwChannel;
	
	}
	else if(level == DFS_BW_CH_QUERY_LEVEL1)
		BwChannel = DfsBwChQueryListOrDefault(pAd, Bw, pDfsParam, DFS_BW_CH_QUERY_LEVEL2, bDefaultSelect, SkipNonDfsCh);
	else if(level == DFS_BW_CH_QUERY_LEVEL2)
	{
		if(Bw > BW_20)
		{
			/*Clear NOP of the current BW*/
			for(i = 0; i < pDfsParam->ChannelListNum; i++)
			{					
				if((pDfsParam->DfsChannelList[i].NonOccupancy != 0) && (pDfsParam->DfsChannelList[i].NOPSetByBw == Bw))
				{
					pDfsParam->DfsChannelList[i].NOPSaveForClear = pDfsParam->DfsChannelList[i].NonOccupancy;
					pDfsParam->DfsChannelList[i].NonOccupancy = 0;
					pDfsParam->DfsChannelList[i].NOPClrCnt++;
				}
			}
			/*reduce BW*/
			BwChannel = DfsBwChQueryListOrDefault(pAd, Bw - 1, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, bDefaultSelect, SkipNonDfsCh);
		}	
		else
			;/*Will return BwChannel = 0*/
	}
	else
		;
	return BwChannel;
	
}

VOID DfsBwChQueryAllList( /*Query current All available BW & Channel list*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, BOOLEAN SkipWorkingCh)
{
	UINT_8 i;
	UINT_8 AvailableChCnt = 0;
	
	DfsGetSysParameters(pAd);
	if(pDfsParam->bIEEE80211H == FALSE)
		return ;

	for (i = 0; i < pDfsParam->ChannelListNum; i++)
	{
		if (AutoChannelSkipListCheck(pAd, pDfsParam->DfsChannelList[i].Channel) == TRUE)
			continue;

		if(!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
			continue;

		if(ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, Bw, pDfsParam))
			continue;
		
		if((pDfsParam->DfsChannelList[i].NonOccupancy == 0)
		 && (pDfsParam->DfsChannelList[i].NOPClrCnt!= 0)
		 && (pDfsParam->DfsChannelList[i].NOPSetByBw <= Bw)
		)
			continue;
		
		if(DfsCheckBwGroupAllAvailable(i, Bw, pDfsParam) == FALSE)
			continue;				

		if(SkipWorkingCh == TRUE)
		{
			if(DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw) == DfsPrimToCent(pDfsParam->Band0Ch, pDfsParam->Bw))
				continue;
		
			if(DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, Bw) == DfsPrimToCent(pDfsParam->Band0Ch, Bw))
				continue;
		}	

		if(pDfsParam->DfsChannelList[i].NonOccupancy == 0)
			pDfsParam->AvailableBwChIdx[Bw][AvailableChCnt++] = i;
	}

	if(Bw > BW_20)
	{
		/*Clear NOP of the current BW*/
		for(i = 0; i < pDfsParam->ChannelListNum; i++)
		{
			if((pDfsParam->DfsChannelList[i].NonOccupancy != 0) && (pDfsParam->DfsChannelList[i].NOPSetByBw == Bw))
			{
				pDfsParam->DfsChannelList[i].NOPSaveForClear = pDfsParam->DfsChannelList[i].NonOccupancy;
				pDfsParam->DfsChannelList[i].NonOccupancy = 0;
				pDfsParam->DfsChannelList[i].NOPClrCnt++;
			}
		}		
		DfsBwChQueryAllList(pAd, Bw - 1, pDfsParam, SkipWorkingCh);
	}	
	
}

BOOLEAN DfsDedicatedCheckChBwValid( 
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	UINT_8 i, j, idx;

	if(pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return TRUE;

	for(i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++)
	{
		for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;		
	}	
	
	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, FALSE);
	
	for(i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++)
	{
		for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
		{
			if(pDfsParam->AvailableBwChIdx[i][j] != 0xff)
			{
				idx = pDfsParam->AvailableBwChIdx[i][j];
				/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("i: %d, j: %d, ChannelList[%d], Ch %d, Bw:%d, CHannel: %d\n",
				i, j, idx, pDfsParam->DfsChannelList[idx].Channel,
				Bw, Channel));*/
				if((pDfsParam->DfsChannelList[idx].Channel == Channel)
				 && (Bw == i))
				{
					/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
					("Return TRUE\n"));*/
					return TRUE;
				}	
					
			}	
		}	
	}
	return FALSE;
		
}

VOID DfsAdjustBwSetting(
	IN PRTMP_ADAPTER pAd, UCHAR CurrentBw, UCHAR NewBw)
{
	UCHAR HtBw;
	UCHAR VhtBw;
	UINT_8 i;
	//BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[0];
	struct wifi_dev *wdev;// = &pMbss->wdev;
	
	UCHAR rfic = RFIC_5GHZ;
	if(NewBw == CurrentBw)
	{
		return;
	}	
	switch (NewBw)
	{
		case BW_20:
			HtBw = BW_20;
			VhtBw = VHT_BW_2040;
			break;
		case BW_40:
			HtBw = BW_40;
			VhtBw = VHT_BW_2040;			
			break;
		case BW_80:
			HtBw = BW_40;
			VhtBw = VHT_BW_80;
			break;
		case BW_160:
			HtBw = BW_40;
			VhtBw = VHT_BW_160;
			break;
		default:
			return;
			break;
	}


	for(i = 0; i < WDEV_NUM_MAX; i++){
		wdev = pAd->wdev_list[i];
		/*only when wdev is up & operting init done can join to decision*/
		if(wdev && (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_INVALID) && (rfic & wmode_2_rfic(wdev->PhyMode))){
			
			wlan_config_set_ht_bw(wdev,HtBw);
			SetCommonHtVht(pAd,wdev);
			
			pAd->CommonCfg.vht_bw = VhtBw;
			pAd->CommonCfg.cfg_vht_bw = pAd->CommonCfg.vht_bw;
			wlan_config_set_vht_bw(wdev,pAd->CommonCfg.vht_bw);
			SetCommonHtVht(pAd,wdev);	

		}
	}

}

VOID WrapDfsRadarDetectStart( /*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	UCHAR phy_bw = HcGetBwByRf(pAd,RFIC_5GHZ);
	if(pDfsParam->bShowPulseInfo)
		return;	

	DfsGetSysParameters(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRadarDetectStart]: Band0Ch is %d\n", pDfsParam->Band0Ch));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRadarDetectStart]: Band1Ch is %d\n", pDfsParam->Band1Ch));
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
	|| (IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd) && RadarChannelCheck(pAd, pDfsParam->Band0Ch))
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
				/*Prim in idx 4~7. WorkAround for "Always set BB Prim 0~3 in IBF issue*/
				if((pDfsParam->PrimCh >= GROUP2_LOWER && pDfsParam->PrimCh <= GROUP2_UPPER))
				{
					ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
				}
				else
				{
					if((pDfsParam->Band0Ch >= GROUP1_LOWER && pDfsParam->Band0Ch <= GROUP2_UPPER))
						;
					else
						ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);
					
					ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0);				
				}
			}
			else if(pDfsParam->Bw == BW_8080)
			{
				if(pDfsParam->PrimBand == RDD_BAND0) /*Prim in idx 0~3*/
				{
					if(pDfsParam->DfsChBand[0])
						ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);				
					if(pDfsParam->DfsChBand[1])
						ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0);				
				}
				else /*Prim in idx 4~7. WorkAround for "Always set BB Prim 0~3 in IBF issue"*/
				{
					if(pDfsParam->DfsChBand[1])
					{
						ret1= mtRddControl(pAd, RDD_START, HW_RDD0, 0);	
					}	
					if(pDfsParam->DfsChBand[0])
					{
						ret1= mtRddControl(pAd, RDD_START, HW_RDD1, 0);	
					}	
						
				}			
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
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
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
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	return (pDfsParam->bDfsIsScanRunning == TRUE);	
}

VOID DfsSetScanRunning(
	IN PRTMP_ADAPTER pAd, BOOLEAN bIsScan)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	if(bIsScan)
		pDfsParam->bDfsIsScanRunning = TRUE;	
	else
		pDfsParam->bDfsIsScanRunning = FALSE;	
}

BOOLEAN DfsIsClass2DeauthDisable(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	return (pDfsParam->bClass2DeauthDisable == TRUE);	
}

VOID DfsSetClass2DeauthDisable(
	IN PRTMP_ADAPTER pAd, BOOLEAN bEnable)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	if(bEnable)
		pDfsParam->bClass2DeauthDisable = TRUE;	
	else
		pDfsParam->bClass2DeauthDisable = FALSE;	
}

VOID DfsDedicatedOutBandRDDStart(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	
	pDfsParam->RadarDetected[1]= FALSE;
	pDfsParam->DfsChBand[1] = RadarChannelCheck(pAd, pDfsParam->OutBandCh);
	if(pDfsParam->DfsChBand[1])
	{
		mtRddControl(pAd, RDD_START, HW_RDD1, 0);
		DfsOutBandCacReset(pAd);
		
		if ((pAd->CommonCfg.RDDurRegion == CE) 
		 && DfsCacRestrictBand(pAd, pDfsParam, pDfsParam->OutBandBw, pDfsParam->OutBandCh, 0))
		 	pDfsParam->DedicatedOutBandCacTime = 605;
		else
			pDfsParam->DedicatedOutBandCacTime = 65;

#ifdef DFS_DBG_LOG		
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]: Dedicated CAC time: %d \x1b[m \n",
			__FUNCTION__, pDfsParam->DedicatedOutBandCacTime));
#endif
	}
}

VOID DfsDedicatedOutBandRDDRunning(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	USHORT BwChannel;
	
	mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);
	
	if(pDfsParam->bSetOutBandBwChannelByExt == FALSE)
	{
		BwChannel = DfsBwChQueryListOrDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE);
		pDfsParam->OutBandCh = BwChannel & 0xff;
		pDfsParam->OutBandBw = BwChannel>>8;
	}

#ifdef DFS_DBG_LOG_0	
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] New selected Channel and Bw is %d, %d \x1b[m \n", 
			__FUNCTION__, pDfsParam->OutBandCh, pDfsParam->OutBandBw)); 	
#endif	
	pDfsParam->bSetOutBandBwChannelByExt = FALSE;
}

VOID DfsDedicatedOutBandRDDStop(
	IN PRTMP_ADAPTER pAd)
{
	mtRddControl(pAd, RDD_STOP, HW_RDD1, 0);	
}

VOID DfsOutBandCacReset(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bOutBandAvailable = FALSE;
}

VOID DfsSetCacRemainingTime(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	if(pDfsParam->bDedicatedZeroWaitSupport == TRUE)
	{	
		if((pAd->Dot11_H.RDMode == RD_SILENCE_MODE) && (pDfsParam->bSetInBandBwChannelByExt == FALSE))
		{
			pAd->Dot11_H.RDCount = pDfsParam->DedicatedOutBandCacCount;
#ifdef DFS_DBG_LOG
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Remaining CAC time is %d \x1b[m \n", 
			__FUNCTION__, pAd->Dot11_H.ChMovingTime - pAd->Dot11_H.RDCount)); 
#endif
		}	
	}
	
	pDfsParam->bSetInBandBwChannelByExt = FALSE;
	pDfsParam->bSetOutBandBwChannelByExt = FALSE;
	DfsOutBandCacReset(pAd);

}

VOID DfsOutBandCacCountUpdate(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

#ifdef BACKGROUND_SCAN_SUPPORT
	if(!GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		return;
#endif
	if((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (pDfsParam->bOutBandAvailable == FALSE))
	{
		if(pDfsParam->DedicatedOutBandCacCount++ > pDfsParam->DedicatedOutBandCacTime)
		{
			pDfsParam->bOutBandAvailable = TRUE;
#ifdef DFS_DBG_LOG			
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] OutBand(SynB) CAC complete and is available now.\n", __FUNCTION__));
#endif			
			if (DfsCacTimeOutCallBack)
			{
				DfsCacTimeOutCallBack(RDD_BAND1, pDfsParam->OutBandBw, pDfsParam->OutBandCh);
			}
		}
	}
		
}

BOOLEAN DfsIsInBandBwChaByExt(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

	return (pDfsParam->bSetInBandBwChannelByExt == TRUE);
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
    ret = MtCmdRddCtrl(pAd, ucRddCtrl, ucRddIdex, ucRddInSel, 0);
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
            Channel = WrapDfsRandomSelectChannel(pAd, TRUE, 0); /* Skip DFS CH */
            HcUpdateChannel(pAd, Channel);
           
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
           
        }           
    }   
}

VOID DfsDedicatedScanStart(IN PRTMP_ADAPTER pAd)
{
	USHORT BwChannel;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	if(pDfsParam->bDedicatedZeroWaitSupport == TRUE)
	{
		if(pDfsParam->bSetOutBandBwChannelByExt == FALSE)
		{			
			BwChannel = DfsBwChQueryListOrDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE);
			pDfsParam->OutBandCh = BwChannel & 0xff;
			pDfsParam->OutBandBw = BwChannel>>8;
		}
		else		
			;

		if(pDfsParam->OutBandCh == 0)
		{
			return;
		}

		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);	
		RTMP_MLME_HANDLER(pAd);
	}

	pDfsParam->bSetOutBandBwChannelByExt = FALSE;
}

VOID DfsDedicatedInBandSetChannel(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, BOOLEAN doCAC)
{
	
	UCHAR NextCh;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;

#ifdef DFS_DBG_LOG
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n", 
		__FUNCTION__, 0, Channel, Bw));
#endif

	DfsGetSysParameters(pAd);
	
	if(pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return;

	if(!DfsDedicatedCheckChBwValid(pAd, Channel, Bw))
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Not a Valid InBand Channel. Fail. \x1b[m \n", __FUNCTION__)); 
		return;
	}

	if(Channel == 0 || ((Channel == pDfsParam->OutBandCh)  &&  (Bw == pDfsParam->OutBandBw)))
	{
		Channel = pDfsParam->OutBandCh;
		Bw = pDfsParam->OutBandBw;
#ifdef DFS_DBG_LOG		
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m InBand set to OutBand Channel %d, Bw :%d \x1b[m \n", Channel, Bw)); 		
#endif
	}
	else
	{
#ifdef DFS_DBG_LOG	
		pDfsParam->bSetInBandBwChannelByExt = TRUE;
#endif
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m InBand set to non-OutBand Channel %d, Bw %d \x1b[m \n", Channel, Bw)); 
	}

	if(doCAC == FALSE)
	{
		pDfsParam->bSetInBandBwChannelByExt = FALSE;
		pDfsParam->bOutBandAvailable = TRUE;
	}
	
	pDfsParam->OrigInBandCh = pDfsParam->Band0Ch;
	pDfsParam->OrigInBandBw = pDfsParam->Bw;		
	pDfsParam->Band0Ch = Channel;
	pDfsParam->PrimCh = pDfsParam->Band0Ch;
	pDfsParam->PrimBand = RDD_BAND0;

		
	/*Adjust Bw*/
	DfsAdjustBwSetting(pAd, pDfsParam->Bw, Bw);

	NextCh = pDfsParam->PrimCh;
	
	if (pDfsParam->Dot11_H.RDMode == RD_NORMAL_MODE)
	{
				    
		pDfsParam->DfsChBand[0] = FALSE;
		pDfsParam->DfsChBand[1] = FALSE;		
		pDfsParam->RadarDetected[0] = FALSE;
		pDfsParam->RadarDetected[1] = FALSE;	
	
		pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
		pAd->Dot11_H.CSCount = 0;
		pAd->Dot11_H.new_channel = NextCh;
		pAd->Dot11_H.org_ch = HcGetChannelByRf(pAd, RFIC_5GHZ);
		Set5GPrimChannel(pAd, NextCh);

#ifdef DFS_DBG_LOG	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_NORMAL_MODE Update InBand Ch = %d \x1b[m \n",
			__FUNCTION__, NextCh));
#endif		
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
		
		pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
		
#ifdef DFS_DBG_LOG		
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_SILENCE_MODE Update InBand Ch = %d \x1b[m \n",
			__FUNCTION__, NextCh));		
#endif
		pDfsParam->DfsStatMachine.CurrState = DFS_BEFORE_SWITCH;
	
		MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CHAN_SWITCH_TIMEOUT, 0, NULL, 0);			
		RTMP_MLME_HANDLER(pAd);
	}
}

VOID DfsDedicatedOutBandSetChannel(IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	
	DfsGetSysParameters(pAd);
	
#ifdef DFS_DBG_LOG
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n", 
		__FUNCTION__, 1, Channel, Bw));
#endif	
	if(pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return;

	if(!DfsDedicatedCheckChBwValid(pAd, Channel, Bw))
	{
#ifdef DFS_DBG_LOG	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Not a Valid OutBand Channel. Fail. \x1b[m \n", __FUNCTION__)); 
#endif
		return;
	}
	if(!RadarChannelCheck(pAd, Channel))
	{
#ifdef DFS_DBG_LOG	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Not a DFS Channel. No need to split out for Radar Detection. \x1b[m \n", __FUNCTION__)); 
#endif
		return;		
	}
	
	if(Channel != 0)
	{
		pDfsParam->bSetOutBandBwChannelByExt = TRUE;
		pDfsParam->OutBandCh = Channel;
		pDfsParam->OutBandBw = Bw;
	}
	else
	{
#ifdef DFS_DBG_LOG	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Pick OutBand Ch by internal Alogorithm \x1b[m \n")); 
#endif
	}


	
	if(GET_BGND_STATE(pAd, BGND_RDD_DETEC))
	{
#ifdef DFS_DBG_LOG
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Dediated Running: OutBand set Channel to %d \x1b[m \n", Channel)); 
#endif
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_SWITCH, 0, NULL, 0); 
		RTMP_MLME_HANDLER(pAd); 
	}
	else if(GET_BGND_STATE(pAd, BGND_SCAN_IDLE))
	{
#ifdef DFS_DBG_LOG
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Dedicated Start: OutBand set Channel to %d \x1b[m \n", Channel)); 
#endif
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0); 
		RTMP_MLME_HANDLER(pAd);
	}
	else
	{
#ifdef DFS_DBG_LOG	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Wrong state. OutBand Set Channel Fail \x1b[m \n")); 		
#endif
	}			
}

VOID DfsDedicatedDynamicCtrl(IN PRTMP_ADAPTER pAd, UINT_32 DfsDedicatedOnOff)
{
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	
#ifdef DFS_DBG_LOG
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] DfsDedicatedOnOff: %d \x1b[m \n", 
		__FUNCTION__, DfsDedicatedOnOff));
#endif
		
	if(DfsDedicatedOnOff == DYNAMIC_ZEROWAIT_OFF)
	{
		if(GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		{
			pDfsParam->OrigInBandCh= pDfsParam->PrimCh;
			pDfsParam->OrigInBandBw= pDfsParam->Bw;
			DedicatedZeroWaitStop(pAd);
			DfsOutBandCacReset(pAd);
		}
#ifdef DFS_DBG_LOG
		else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Already in 4x4 mode \x1b[m \n", __FUNCTION__));
#endif		
	}	
	else		
	{
		if(GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		{
#ifdef DFS_DBG_LOG
	
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Already in 2x2 mode \x1b[m \n", __FUNCTION__));
#else			
			;
#endif
		}
		else if(pDfsParam->OutBandCh == 0)
		{
#ifdef DFS_DBG_LOG
	
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] No SynB Info Recorded. Fail. \x1b[m \n", __FUNCTION__));
#else			
			;
#endif
		}		
		else
		{

			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);	
			RTMP_MLME_HANDLER(pAd);
		}	
	}

}

#endif /* BACKGROUND_SCAN_SUPPORT */

INT Set_ModifyChannelList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;
	UCHAR Bw80Num = 4;
	UCHAR Bw40Num = 10;
	UCHAR Bw20Num = 11;
	
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[4]
	= {{116, 0}, {120, 0}, {124, 0}, {128,0}};
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[10]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128,0}, {132,0}, {136,0}};
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[11]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128,0}, {132,0}, {136,0}, {140,0}};
	
	Value = (UCHAR) simple_strtol(arg, 0, 10);

	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80Num, &Bw80AvailableChList[0],
	Bw40Num, &Bw40AvailableChList[0],
	Bw20Num, &Bw20AvailableChList[0]);
	
	return TRUE; 

}

INT Show_available_BwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	DfsProvideAvailableChList(pAd);
	return TRUE;
}

VOID ZeroWait_DFS_Initialize_Candidate_List(
	IN PRTMP_ADAPTER pAd,
	UCHAR Bw80Num, PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList,
	UCHAR Bw40Num, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList,
	UCHAR Bw20Num, PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList)
{
	UINT_8 i = 0,j = 0,k = 0;
	UINT_8 ChIdx;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	UCHAR SupportBwBitMap[MAX_NUM_OF_CHS] = {0};
	UCHAR OrigSupportBwBitMap[MAX_NUM_OF_CHS] = {0};

	if(pAd->Dot11_H.RDMode == RD_SWITCHING_MODE)
	{
#ifdef DFS_DBG_LOG	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel Lsit init fail during channel switch\n"));
#endif
		return;
	}

	for(ChIdx = 0; ChIdx < pDfsParam->ChannelListNum; ChIdx ++)
	{
		if(pBw80AvailableChList->Channel == pDfsParam->DfsChannelList[ChIdx].Channel)
		{
			SupportBwBitMap[ChIdx]|= 0x04;
			if(i++ < Bw80Num)
				pBw80AvailableChList++;
		}
		if(pBw40AvailableChList->Channel == pDfsParam->DfsChannelList[ChIdx].Channel)
		{
			SupportBwBitMap[ChIdx]|= 0x02;
			if(j++ < Bw40Num)
				pBw40AvailableChList++;
		}
		if(pBw20AvailableChList->Channel == pDfsParam->DfsChannelList[ChIdx].Channel)
		{
			SupportBwBitMap[ChIdx]|= 0x01;
			if(k++ < Bw20Num)
				pBw20AvailableChList++;
		}
		//pDfsParam->DfsChannelList[ChIdx].SupportBwBitMap = SupportBwBitMap[ChIdx];
	}
	for(ChIdx = 0; ChIdx < pDfsParam->ChannelListNum; ChIdx ++)
	{
		OrigSupportBwBitMap[ChIdx] = pDfsParam->DfsChannelList[ChIdx].SupportBwBitMap;
		
		if(OrigSupportBwBitMap[ChIdx] >= 0x07)
		{
			if(SupportBwBitMap[ChIdx] == 0x07)
				;
			else if(SupportBwBitMap[ChIdx] == 0x03)
			{
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_80;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			}
			else if(SupportBwBitMap[ChIdx] == 0x01)
			{
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_40;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;	
			}
			else if(SupportBwBitMap[ChIdx] == 0x0)
			{
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_20;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			}
			else
				;
				
			
		}
		else if(OrigSupportBwBitMap[ChIdx] == 0x03)
		{
			if(SupportBwBitMap[ChIdx] == 0x03)
				;
			else if(SupportBwBitMap[ChIdx] == 0x01)
			{
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_40;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;	
			}
			else if(SupportBwBitMap[ChIdx] == 0x0)
			{
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_20;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			}
			else
				;

		}
		else if(OrigSupportBwBitMap[ChIdx] == 0x01)
		{
			if(SupportBwBitMap[ChIdx] == 0x01)
				;
			else if(SupportBwBitMap[ChIdx] == 0x0)
			{
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_20;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			}
			else
				;

		}
		else
			;
	}	

	for(i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++)
	{
		for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;		
	}
	
	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE);	

	for(i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++)
	{
#ifdef DFS_DBG_LOG_0	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw: %d\n",i));
#endif
		for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
		{
			if(pDfsParam->AvailableBwChIdx[i][j] != 0xff)
			{
				ChIdx = pDfsParam->AvailableBwChIdx[i][j];
#ifdef DFS_DBG_LOG_0				
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
				ChIdx, pDfsParam->DfsChannelList[ChIdx].Channel,
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt));
#endif				
			}	
		}	
	}

}

VOID DfsProvideAvailableChList(
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i, j, idx;
	PDFS_PARAM pDfsParam = &pAd->DfsParameter;
	for(i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++)
	{
		for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;		
	}
	
	if(pAd->Dot11_H.RDMode == RD_SWITCHING_MODE)
	{
		return;
	}
	
	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE);	

	for(i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++)
	{
#ifdef DFS_DBG_LOG	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw: %d\n",i));
#endif
		for(j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
		{
			if(pDfsParam->AvailableBwChIdx[i][j] != 0xff)
			{
				idx = pDfsParam->AvailableBwChIdx[i][j];
#ifdef DFS_DBG_LOG
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
				idx, pDfsParam->DfsChannelList[idx].Channel,
				pDfsParam->DfsChannelList[idx].NOPClrCnt));
#endif				
			}	
		}	
	}

}

VOID ZeroWait_DFS_collision_report(
	IN PRTMP_ADAPTER pAd,IN UCHAR SynNum, IN UCHAR Channel, UCHAR Bw)
{
#ifdef DFS_DBG_LOG
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m[%s] SynNum: %d, Channel: %d, Bw:%d \x1b[m \n", __FUNCTION__, SynNum, Channel, Bw));
#endif

	if (radar_detected_callback_func)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m[%s] Call back func \x1b[m \n", __FUNCTION__));
		radar_detected_callback_func(SynNum, 
					Channel,
					Bw
					);
	}

}



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

