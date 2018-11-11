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
	ap_band_steering.c
*/

#ifdef BAND_STEERING
#include "rt_config.h"

extern BNDSTRG_OPS D_BndStrgOps;

static inline PBND_STRG_CLI_TABLE Get_BndStrgTableByBand(
    PRTMP_ADAPTER	pAd,
    UINT8           Band)
{
    PBND_STRG_CLI_TABLE table = NULL;
    INT i;

    for(i=0; i<DBDC_BAND_NUM; i++)
    {
        table = P_BND_STRG_TABLE(i);
        if(table->bInitialized && (table->Band == Band))
            return table;
    }
    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (RED("%s:(%d):invalid Band. Band=%d,table->Band=%d\n"),__FUNCTION__, __LINE__, Band,table->Band));
    return NULL;
}

inline PBND_STRG_CLI_TABLE Get_BndStrgTable(
    PRTMP_ADAPTER	pAd,
    INT             apidx)
{
    BSS_STRUCT *pMbss = NULL;
    UINT8 Band;
    
    if(apidx < HW_BEACON_MAX_NUM)
        pMbss = &pAd->ApCfg.MBSSID[apidx];

    if(pMbss)
    {
        Band = WMODE_CAP_5G(pMbss->wdev.PhyMode)?BAND_5G : BAND_24G;
        return Get_BndStrgTableByBand(pAd, Band);
    }
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (RED("%s:(%d):invalid pMbss. apidx=%d\n"),__FUNCTION__, __LINE__,apidx));
    
    return NULL;
}

/* ioctl */
INT Show_BndStrg_Info(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	struct wifi_dev *wdev = NULL;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	if (ifIndex > HW_BEACON_MAX_NUM)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
//#ifdef VENDOR_FEATURE5_SUPPORT
	if(!pAd->ApCfg.BndStrgBssIdx[wdev->func_idx])
        return TRUE;
//#endif
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;

	if (table->Ops)
		table->Ops->ShowTableInfo(table);

	return TRUE;	
}

INT Show_BndStrg_Help(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	struct wifi_dev *wdev = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	if (ifIndex > HW_BEACON_MAX_NUM)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
//#ifdef VENDOR_FEATURE5_SUPPORT
	if(!pAd->ApCfg.BndStrgBssIdx[wdev->func_idx])
        return TRUE;
//#endif
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("set command:\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgEnable=[1|0]\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgRssiDiff=30\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgRssiLow=-70\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgAge=150 (s)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgHoldTime=90 (s)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgCheckTime=30 (s)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgCndChk=0x1 (means RSSI_DIFF)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgCndPriority=\"7;6;5;4;8\"\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("RSSI_DIFF = 0, BAND_PERSIST = 1, HT_SUPPORT = 2, 5G_RSSI = 3, VHT_SUPPORT = 4,"
						" NSS_SUPPORT = 5, LB_QLOAD = 6, LB_STA_COUNT = 7, LB_RSSI = 8, LB_MCS = 9"
						"DEFAULT_2G = 10, DEFAULT_5G = 11,5G_RSSI_DYNAMIC = 12\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("CHK_PRB_REQ = 0, CHK_ATH_REQ = 1, CHK_ASS_REQ = 2\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgFrmChk=0x1 (means CHK_PRB_REQ)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgSteeringNum=64\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgLBAssocThres=20 (%%)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgLBQloadThres=20 (%%)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgLBMinRssiThres=\"-45\"\n"));
//	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
//					  ("LOAD_BALANCE_STA_CONNECTED_COUNT = 0, LOAD_BALANCE_QLOAD = 1, LOAD_BALANCE_RSSI = 2, LOAD_BALANCE_MCS = 3\n"));
//	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
//					  ("iwpriv ra0 set BndStrgLBCnd=0x1 (means LOAD_BALANCE_STA_CONNECTED_COUNT)\n"));
//	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
//				  	("iwpriv ra0 set BndStrgLBCndPriority=\"2;1;0\"\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgNSSThres=\"3\"\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgMntAddr=e4:b3:18:04:af:74\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgStaPollTime=3   (3-20 sec)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgMaxSteerCount=5\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgSteerTimeWindow=300 (time in sec)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgDwellTime=60 (time in sec)\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("BNDSTRG_INIT = 0, BNDSTRG_INF_POLL = 1, BNDSTRG_TBL_EN = 2, BNDSTRG_TBL_READY = 3, BNDSTRG_LEAVE = 4, BNDSTRG_FROZEN = 5\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 set BndStrgDaemonState=5   (0~5)\n"));	
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("show command:\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 show BndStrgInfo\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 show BndStrgList=[0|1|2]  0:all valid entry,1:connectd entry,2:the same band entry\n"));
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					  ("iwpriv ra0 show BndStrgHelp\n"));
	return TRUE;	
}

INT Show_BndStrg_List(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT32	display_type;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
	if (arg == NULL)
		display_type = 0;
	else
		display_type = (UINT32) simple_strtol(arg, 0, 10);
	if (display_type > 3)
		display_type = 0;
	if (table->Ops)
		table->Ops->ShowTableEntries(table,display_type);

	return TRUE;	
}

INT Set_BndStrg_Enable(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    BOOLEAN         enable;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	enable = (BOOLEAN) simple_strtol(arg, 0, 10);
	
	if (!(enable ^ pAd->ApCfg.BandSteering)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (GRN("BndStrg is already %s\n"),pAd->ApCfg.BandSteering?"Enable":"Disable"));
		return TRUE;
	}
	if (enable) {
		BSS_STRUCT *pMbss = NULL;
		INT apidx,IdBss;
		apidx = ifIndex;
		if(apidx < HW_BEACON_MAX_NUM)
			pMbss = &pAd->ApCfg.MBSSID[apidx];
		if (pMbss == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (RED("%s:(%d):invalid pMbss. apidx=%d\n"),__FUNCTION__, __LINE__,apidx));
			return FALSE;
		}
		// bnstrg table does not init yet, call BndStrg_Init directly
		BndStrg_Init(pAd);
		
		//enable all active mbss BndStrg InfFlags to nitify daemon
		for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			table = Get_BndStrgTable(pAd, IdBss);
			if(table)
			{
				/* Inform daemon interface ready */
				BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, TRUE);
			}
		}
		pAd->ApCfg.BandSteering = enable;
	} else {
		BndStrg_Release(pAd);
		pAd->ApCfg.BandSteering = enable;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (GRN("BndStrg %s Success\n"),pAd->ApCfg.BandSteering?"Enable":"Disable"));

	return TRUE;
}

INT Set_BndStrg_RssiDiff(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    CHAR            RssiDiff;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	RssiDiff = (CHAR) simple_strtol(arg, 0, 10);
	if (table->Ops)
		table->Ops->SetRssiDiff(table, RssiDiff);

	table->RssiDiff = RssiDiff;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): RssiCheck = %u\n", __FUNCTION__, table->RssiDiff));

	return TRUE;
}


INT Set_BndStrg_RssiLow(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    CHAR            RssiLow;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	RssiLow = (CHAR) simple_strtol(arg, 0, 10);
	if (table->Ops)
		table->Ops->SetRssiLow(table, RssiLow);

	table->RssiLow = RssiLow;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): RssiLow = %d\n", __FUNCTION__, table->RssiLow));

	return TRUE;
}


INT Set_BndStrg_Age(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT32          AgeTime;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	AgeTime = (UINT32) simple_strtol(arg, 0, 10);
	if (table->Ops)
		table->Ops->SetAgeTime(table, AgeTime);

	return TRUE;
}

INT Set_BndStrg_DwellTime(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT32          dwell_time; // change to dwell_time
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	dwell_time = (UINT32) simple_strtol(arg, 0, 10);
	if (table->Ops)
		table->Ops->SetDwellTime(table, dwell_time);

	return TRUE;
}

INT Set_BndStrg_SteerTimeWindow(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT32          SteerTimeWindow;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	SteerTimeWindow = (UINT32) simple_strtol(arg, 0, 10);
	if (table->Ops)
		table->Ops->SetSteerTimeWindow(table, SteerTimeWindow);

	return TRUE;

}

INT Set_BndStrg_MaxSteerCount(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT8          MAxSteerCnt;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	MAxSteerCnt = (UINT8) simple_strtol(arg, 0, 10);
	if(MAxSteerCnt<1)
		return FALSE;
	if (table->Ops)
		table->Ops->SetMaxSteerCnt(table, MAxSteerCnt);

	return TRUE;
}

INT Set_BndStrg_HoldTime(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT32          HoldTime;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	HoldTime = (UINT32) simple_strtol(arg, 0, 10);
	if (table->Ops)
		table->Ops->SetHoldTime(table, HoldTime);

	return TRUE;
}

INT Set_BndStrg_CheckTime(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING	    *arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT32          CheckTime;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	CheckTime = (UINT32) simple_strtol(arg, 0, 10);
	if (table->Ops)
		table->Ops->SetCheckTime(table, CheckTime);

    return TRUE;
}

INT Set_BndStrg_FrmChkFlag(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    UINT32          FrmChkFlag;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	FrmChkFlag = (UINT32) simple_strtol(arg, 0, 16);
	if (table->Ops)
		table->Ops->SetFrmChkFlag(table, FrmChkFlag);

	return TRUE;
}

#ifdef BND_STRG_DBG
INT Set_BndStrg_MonitorAddr(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UCHAR					MonitorAddr[MAC_ADDR_LEN];
	RTMP_STRING				*value;
	INT						i;
	PBND_STRG_CLI_TABLE     table;
    POS_COOKIE 		        pObj;
	UCHAR 			        ifIndex;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;

	if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&MonitorAddr[i++], 1);
	}

	if (table->Ops)
		table->Ops->SetMntAddr(table, MonitorAddr);

	return TRUE;
}
#endif /* BND_STRG_DBG */
/**** end of ioctl ****/

INT BndStrg_Init(PRTMP_ADAPTER pAd)
{
	INT ret_val = BND_STRG_SUCCESS;
    INT max_mbss_check_num;
    INT apidx;
    
	max_mbss_check_num = pAd->ApCfg.BssidNum;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (YLW("%s()\n"), __FUNCTION__));

    for (apidx = 0; apidx < max_mbss_check_num; apidx++)
    {
        ret_val = BndStrg_TableInit(pAd, apidx);
    	if (ret_val != BND_STRG_SUCCESS)
    	{
    		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
    					("Error in %s(), error code = %d on apidx = %d\n",
    					__FUNCTION__, ret_val, apidx));
    	}
    }

	return ret_val;
}


INT BndStrg_TableInit(PRTMP_ADAPTER pAd, INT apidx)
{
    BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
    PBND_STRG_CLI_TABLE table = NULL, init_table = NULL;
    INT i;
    UINT8 Band;

    if(pMbss)
        Band = WMODE_CAP_5G(pMbss->wdev.PhyMode)?BAND_5G : BAND_24G;
    else
    {
        BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
    					("Error in %s(), pMbss is NULL!\n",__FUNCTION__));
        return BND_STRG_UNEXP;
    }

    for(i=0; i<DBDC_BAND_NUM; i++)
    {
        table = P_BND_STRG_TABLE(i);
        if(!table->bInitialized && !init_table)
        {
            init_table = table;
            continue;
        }
        
        if(table->bInitialized && (table->Band == Band))
            return BND_STRG_SUCCESS;
    }

	if(init_table)
    {
    	NdisZeroMemory(init_table, sizeof(BND_STRG_CLI_TABLE));
    	OS_NdisAllocateSpinLock(&init_table->Lock);

    	init_table->Ops = &D_BndStrgOps;
    	init_table->DaemonPid = 0xffffffff;
    	init_table->RssiDiff = BND_STRG_RSSI_DIFF;
    	init_table->RssiLow = pAd->ApCfg.BndStrgRssiLow;
    	init_table->AgeTime = pAd->ApCfg.BndStrgAge;
    	init_table->HoldTime = pAd->ApCfg.BndStrgHoldTime;
    	init_table->CheckTime = pAd->ApCfg.BndStrgCheckTime;
    	init_table->AutoOnOffThrd = BND_STRG_AUTO_ONOFF_THRD;
    	//init_table->AlgCtrl.ConditionCheck = pAd->ApCfg.BndStrgConditionCheck;
    	memcpy(init_table->BndStrgCndPri, pAd->ApCfg.BndStrgCndPri, pAd->ApCfg.BndStrgCndPriSize);
    	init_table->BndStrgCndPriSize = pAd->ApCfg.BndStrgCndPriSize;
    	init_table->AlgCtrl.FrameCheck = fBND_STRG_FRM_CHK_PRB_REQ | fBND_STRG_FRM_CHK_ATH_REQ;
    	init_table->priv = (VOID *) pAd;
        init_table->Band = Band;	
		init_table->dwell_time = pAd->ApCfg.BndStrgDwellTime;
		init_table->max_steer_time_window = pAd->ApCfg.BndStrgSteerTimeWindow;
		init_table->max_steer_count = pAd->ApCfg.BndStrgMaxSteerCount;
        init_table->bInitialized = TRUE;
    }
    
	return BND_STRG_SUCCESS;
}

INT BndStrg_Release(PRTMP_ADAPTER pAd)
{
	INT ret_val = BND_STRG_SUCCESS;
    INT apidx;
    PBND_STRG_CLI_TABLE table = NULL;
    BSS_STRUCT *pMbss = NULL;
    
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (YLW("%s()\n"), __FUNCTION__));
    for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
    {
        table = Get_BndStrgTable(pAd, apidx);
		if (table)
        {
            pMbss = &pAd->ApCfg.MBSSID[apidx];
            BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, FALSE);
        }
    }
	if(table)
		BndStrg_TableRelease(table);
	return ret_val;
}


INT BndStrg_TableRelease(PBND_STRG_CLI_TABLE table)
{
	INT ret_val = BND_STRG_SUCCESS;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;
	
	OS_NdisFreeSpinLock(&table->Lock);
	table->bInitialized = FALSE;

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}

INT BndStrg_InsertEntry(
	PBND_STRG_CLI_TABLE table,
	struct bnd_msg_cli_add *cli_add,
	PBND_STRG_CLI_ENTRY *entry_out)
{
	INT i;
	UCHAR HashIdx;
	PBND_STRG_CLI_ENTRY entry = NULL, this_entry = NULL;
	INT ret_val = BND_STRG_SUCCESS;

	if (table->Size >= BND_STRG_MAX_TABLE_SIZE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): Table is full!\n", __FUNCTION__));
		return BND_STRG_TABLE_FULL;
	}

	NdisAcquireSpinLock(&table->Lock);
	for (i = 0; i< BND_STRG_MAX_TABLE_SIZE; i++)
	{
		entry = &table->Entry[i];

		/* pick up the first available vacancy*/
		if (entry->bValid == FALSE)	{
			NdisZeroMemory(entry, sizeof(BND_STRG_CLI_ENTRY));
			/* Fill Entry */
			RTMP_GetCurrentSystemTick(&entry->jiffies);
			COPY_MAC_ADDR(entry->Addr, cli_add->Addr);
			entry->TableIndex = cli_add->TableIndex;
			entry->BndStrg_Sta_State = BNDSTRG_STA_INIT;
			entry->bValid = TRUE;
			break;
		}
		entry = NULL;
	}

	if (entry) {
		/* add this MAC entry into HASH table */
		HashIdx = MAC_ADDR_HASH_INDEX(cli_add->Addr);
		if (table->Hash[HashIdx] == NULL) {
			table->Hash[HashIdx] = entry;
		} else {
			this_entry = table->Hash[HashIdx];
			while (this_entry->pNext != NULL) {
				this_entry = this_entry->pNext;
			}
			this_entry->pNext = entry;
		}
		
		*entry_out = entry;
		table->Size++;
	}
	NdisReleaseSpinLock(&table->Lock);
	return ret_val;
}


INT BndStrg_DeleteEntry(PBND_STRG_CLI_TABLE table, PUCHAR pAddr, UINT32 Index)
{
	USHORT HashIdx;
	PBND_STRG_CLI_ENTRY entry, pre_entry, this_entry;
	INT ret_val = BND_STRG_SUCCESS;


	NdisAcquireSpinLock(&table->Lock);
	if (Index >= BND_STRG_MAX_TABLE_SIZE)
	{
		if (pAddr == NULL)
			return BND_STRG_INVALID_ARG;
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		entry = table->Hash[HashIdx];
		while (entry) {
			if (MAC_ADDR_EQUAL(pAddr, entry->Addr)) {
				/* this is the entry we're looking for */
				break;
			} else {
				entry = entry->pNext;
			}
		}

		if (entry == NULL)
		{
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s(): Index=%u, %02x:%02x:%02x:%02x:%02x:%02x, "
				"Entry not found.\n",
				__FUNCTION__, Index, PRINT_MAC(pAddr)));
			NdisReleaseSpinLock(&table->Lock);
			return BND_STRG_INVALID_ARG;
		}
	}
	else {
		entry = &table->Entry[Index];
		HashIdx = MAC_ADDR_HASH_INDEX(entry->Addr);
	}
	
	if (entry && entry->bValid) 
	{
		{
			pre_entry = NULL;
			this_entry = table->Hash[HashIdx];
			ASSERT(this_entry);
			if (this_entry != NULL)
			{
				/* update Hash list*/
				do
				{
					if (this_entry == entry)
					{
						if (pre_entry == NULL)
							table->Hash[HashIdx] = entry->pNext;
						else
							pre_entry->pNext = entry->pNext;
						break;
					}

					pre_entry = this_entry;
					this_entry = this_entry->pNext;
				} while (this_entry);
			}
			/* not found !!!*/
			ASSERT(this_entry != NULL);

			NdisZeroMemory(entry->Addr, MAC_ADDR_LEN);
			entry->pNext = NULL;
			entry->bValid = FALSE;
			table->Size--;
		}
	} else {
	}
	NdisReleaseSpinLock(&table->Lock);

	return ret_val;
}


PBND_STRG_CLI_ENTRY BndStrg_TableLookup(PBND_STRG_CLI_TABLE table, PUCHAR pAddr)
{
	ULONG HashIdx;
	BND_STRG_CLI_ENTRY *entry = NULL;
	
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	entry = table->Hash[HashIdx];

	while (entry && entry->bValid)
	{
		if (MAC_ADDR_EQUAL(entry->Addr, pAddr))
			break;
		else
			entry = entry->pNext;
	}

	return entry;
}

BOOLEAN BndStrg_CheckConnectionReq(
		PRTMP_ADAPTER	pAd,
		struct wifi_dev *wdev,
		PUCHAR pSrcAddr,
		UINT8 FrameType,
		PCHAR Rssi,
		BOOLEAN bAllowStaConnectInHt,
		BOOLEAN bVHTCap,
		UINT8 Nss)
{
	PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, wdev->func_idx);

	if (table && table->Ops && (table->bEnabled == TRUE))
    {   
		return table->Ops->CheckConnectionReq(
										pAd,
										wdev,
										table,
										pSrcAddr,
										FrameType,
										Rssi,
										bAllowStaConnectInHt,
										bVHTCap,
										Nss);
	}
	
	return TRUE;
}


INT BndStrg_Tbl_Enable(PBND_STRG_CLI_TABLE table, BOOLEAN enable, CHAR *IfName)
{
	BNDSTRG_MSG msg;
	PRTMP_ADAPTER pAd = NULL;
    struct bnd_msg_onoff *onoff = &msg.data.onoff;

	if (table == NULL)
		return BND_STRG_TABLE_IS_NULL;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;
    
	if (!(table->bEnabled ^ enable))
	{
		/* Already enabled/disabled */
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, /* TRACE */
				(GRN("%s(): Band steering is already %s.\n"),
				__FUNCTION__, (enable ? "enabled" : "disabled")));
		return BND_STRG_SUCCESS;
	}

	if (enable)
    {   
		table->bEnabled = TRUE;
        strcpy(table->ucIfName, IfName); //decide it by daemon
    }
	else
		table->bEnabled = FALSE;

	pAd = (PRTMP_ADAPTER) table->priv;
	msg.Action = BNDSTRG_ONOFF;
	onoff->OnOff = table->bEnabled;
	onoff->Band = table->Band;
    onoff->Channel = table->Channel;
    strcpy(onoff->ucIfName, IfName);
    
	RtmpOSWrielessEventSend(
		pAd->net_dev,
		RT_WLAN_EVENT_CUSTOM,
		OID_BNDSTRG_MSG,
		NULL,
		(UCHAR *)&msg,
		sizeof(msg));

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				(GRN("%s(): Band steering %s running.\n"),
				__FUNCTION__, (enable ? "start" : "stop")));

	return BND_STRG_SUCCESS;
}

static INT D_BndStrgSendMsg(
	PRTMP_ADAPTER pAd,
	BNDSTRG_MSG *msg)
{
	return	RtmpOSWrielessEventSend(
				pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				OID_BNDSTRG_MSG,
				NULL,
				(UCHAR *) msg,
				sizeof(BNDSTRG_MSG));
}

INT BndStrg_SetInfFlags(
    PRTMP_ADAPTER pAd, 
    struct wifi_dev *wdev, 
    PBND_STRG_CLI_TABLE table, 
    BOOLEAN bInfReady)
{
	INT ret_val = BND_STRG_SUCCESS;
    UINT8 Band;
	BNDSTRG_MSG msg;
    struct bnd_msg_inf_status_rsp *inf_status_rsp = &msg.data.inf_status_rsp;

    if(!wdev)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): wdev is NULL!\n", __FUNCTION__));
        return BND_STRG_UNEXP;
    }

    if(!wdev->if_dev)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): if_dev is NULL!\n", __FUNCTION__));
        return BND_STRG_UNEXP;
    }

    Band = WMODE_CAP_5G(wdev->PhyMode)?BAND_5G : BAND_24G;
    if(!(wdev->bInfReady^bInfReady))
    {
        BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(GRN("%s(): %s Inf %s Band steering is already %s.\n"),__FUNCTION__,
				(IS_5G_BAND(Band) ? "5G" : "2G"), wdev->if_dev->name,
				(bInfReady ? "up" : "down")));
		return BND_STRG_SUCCESS;
    }

    wdev->bInfReady = bInfReady;
    if(bInfReady) /* Exec. by each interface up */
    {
        table->uIdx = wdev->func_idx;
        table->Channel = wdev->channel;
        if(WMODE_CAP_5G(wdev->PhyMode) && WMODE_CAP_AC(wdev->PhyMode))
            table->bVHTCapable = TRUE;
        else
            table->bVHTCapable = FALSE;
        table->ActiveCount++;
    }
    else /* Exec. by each interface down */
    {
        if(table->ActiveCount > 0)
            table->ActiveCount--;
    }

	table->nss = wlan_config_get_tx_stream(wdev);
	msg.Action = INF_STATUS_RSP;
    if(IS_5G_BAND(Band))
        inf_status_rsp->band = BAND_5G;
    else
        inf_status_rsp->band = BAND_24G;

    inf_status_rsp->bInfReady = bInfReady;
    inf_status_rsp->Channel = wdev->channel;
    inf_status_rsp->bVHTCapable = table->bVHTCapable;
    inf_status_rsp->nss = table->nss;
    inf_status_rsp->table_src_addr = (ULONG)table;
	inf_status_rsp->table_size = BND_STRG_MAX_TABLE_SIZE;
    strcpy(inf_status_rsp->ucIfName, wdev->if_dev->name);
	inf_status_rsp->nvram_support = 0;
    D_BndStrgSendMsg(pAd, &msg);

    BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                          (BLUE("%s(): BSS(%02X:%02X:%02X:%02X:%02X:%02X)")
                           BLUE(" set %s Inf %s %s.\n"), __FUNCTION__,
                           PRINT_MAC(wdev->bssid), IS_5G_BAND(Band) ? "5G" : "2G",
                           wdev->if_dev->name, bInfReady ? "ready" : "not ready"));
    
    if(table->bInfReady ^ bInfReady)
    {
        if (bInfReady)
        {
            table->bInfReady = TRUE;
		    table->Band |= Band;
        }
	    else
        {            
            if(!bInfReady && (table->ActiveCount == 0))
            {
		        table->bInfReady = FALSE;
                if(table->bEnabled)
                    BndStrg_Tbl_Enable(table, FALSE, table->ucIfName);
            }
        }
    }
    
	return ret_val;
}

/*
BOOLEAN BndStrg_IsClientStay(
			PRTMP_ADAPTER pAd,
			PMAC_TABLE_ENTRY pEntry)
{
	PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, pEntry->wdev->func_idx);
	CHAR Rssi;

    if(!table || !table->bEnabled)
        return TRUE;

    Rssi = RTMPAvgRssi(pAd, pEntry->wdev, &pEntry->RssiSample);
	//printk("%d,%d,%d,%d   %d\n\r",pEntry->RssiSample.AvgRssi[0],
	//	  pEntry->RssiSample.AvgRssi[1],
	//	 pEntry->RssiSample.AvgRssi[2],
	//	pEntry->RssiSample.AvgRssi[3],Rssi);

	if ((table->AlgCtrl.ConditionCheck & fBND_STRG_CND_5G_RSSI_DYNAMIC) &&
		IS_5G_BAND(table->Band) &&
		(Rssi < (table->RssiLow)))
	{
		BNDSTRG_MSG msg;
        struct bnd_msg_cli_del *cli_del = &msg.data.cli_del;

		msg.Action = CLI_DEL;
		COPY_MAC_ADDR(cli_del->Addr, pEntry->Addr);
		 // we don't know the index, daemon should look it up
		cli_del->TableIndex = BND_STRG_MAX_TABLE_SIZE;

		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				(YLW("%s(): Kick client (%02x:%02x:%02x:%02x:%02x:%02x)")
				 YLW(" due to low Rssi(%d).\n")
				 , __FUNCTION__, PRINT_MAC(pEntry->Addr), Rssi));

		RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_BNDSTRG_MSG,
			NULL,
			(UCHAR *) &msg,
			sizeof(BNDSTRG_MSG));
		table->Ops->TableEntryDel(table, pEntry->Addr, BND_STRG_MAX_TABLE_SIZE);

		return FALSE;
	}

	return TRUE;
}
*/
void BndStrg_UpdateEntry(PRTMP_ADAPTER pAd, 
							MAC_TABLE_ENTRY *pEntry, 
							BOOLEAN bHTCap, 
							BOOLEAN bVHTCap, 
							UINT8 Nss,
							BOOLEAN bConnStatus)
{
	struct wifi_dev *wdev;
	BNDSTRG_MSG msg;
    struct bnd_msg_cli_update *cli_update = &msg.data.cli_update;

	if(!pEntry || 
       !pEntry->wdev
	   || 
       (pAd->ApCfg.BndStrgBssIdx[pEntry->func_tb_idx] != 1)
		)
        return;
	{
		/* check if entry existed or not*/
		PBND_STRG_CLI_ENTRY entry = NULL;
		PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, pEntry->wdev->func_idx);
		if (!table)
			return;
		if (table->Ops)
			entry = table->Ops->TableLookup(table, pEntry->Addr);
		
		if (!entry)
		{
			return;
		}
		entry->bConnStatus = bConnStatus;
		if(bConnStatus)
			entry->BndStrg_Sta_State = BNDSTRG_STA_ASSOC;
		else
			entry->BndStrg_Sta_State = BNDSTRG_STA_DISASSOC;
	}
	wdev = pEntry->wdev;
	if(WMODE_CAP_5G(wdev->PhyMode))
		cli_update->Band = BAND_5G;
	else
		cli_update->Band = BAND_24G;
	cli_update->Channel = wdev->channel;
	cli_update->bAllowStaConnectInHt = bHTCap;
	cli_update->bVHTCapable = bVHTCap;
	cli_update->Nss = Nss;
	cli_update->bConnStatus = bConnStatus;
	COPY_MAC_ADDR(cli_update->Addr, pEntry->Addr);
    msg.Action = CLI_UPDATE;
	D_BndStrgSendMsg(pAd,&msg);
	return;
}

INT Set_BndStrg_BssIdx(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UINT8 i;
	RTMP_STRING *macptr;
	for (i = 0, macptr = rstrtok(arg,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	{
//#ifdef VENDOR_FEATURE5_SUPPORT
		pAd->ApCfg.BndStrgBssIdx[i] = simple_strtoul(macptr,0,10);
//#endif
	}
	return TRUE;
}

INT Set_BndStrg_CndPriority(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
    RTMP_STRING     *macptr;
	UINT8           CndPri[BND_STRG_PRIORITY_MAX]={0},i;
    
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	for (i = 0, macptr = rstrtok(arg,";"); macptr; macptr = rstrtok(NULL,";"), i++)
	{
		CndPri[i] = simple_strtoul(macptr,0,10);
	}
	if (table->Ops)
		table->Ops->SetCndPriority(table, CndPri, i);
	return TRUE;
}

INT Set_BndStrg_Steering_Num(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	UINT32			steering_num;
   
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	table = Get_BndStrgTable(pAd, ifIndex);
	if(!table)
            return FALSE;
	steering_num = (UINT32) simple_strtol(arg, 0, 10);
	if (steering_num > 64)
            return FALSE;

	if ((table->Ops) && table->Ops->SetParam)
		table->Ops->SetParam(table, (UINT8*)&steering_num, BND_SET_STEERING_NUM);
	return TRUE;
}

INT Set_BndStrg_LB_Assoc_Thres(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	UINT32			assoc_bl_thrs;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    assoc_bl_thrs = (UINT32) simple_strtol(arg, 0, 10);
	
	if (assoc_bl_thrs >100)
		return FALSE;

	if ((table->Ops) && table->Ops->SetParam)
		table->Ops->SetParam(table, (UINT8*)&assoc_bl_thrs, BND_SET_ASSOC_BL_TH);
	return TRUE;
}

INT Set_BndStrg_LB_Qload_Thres(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	UINT32			qload_thrs;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    qload_thrs = (UINT32) simple_strtol(arg, 0, 10);
	
	if (qload_thrs >100)
		return FALSE;

	if ((table->Ops) && table->Ops->SetParam)
		table->Ops->SetParam(table, (UINT8*)&qload_thrs, BND_SET_QLOAD_TH);
	return TRUE;
}

INT Set_BndStrg_LB_Min_Rssi_Thres(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	RTMP_STRING     *rssiptr;
	INT32           rssi[3]={0},i;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    
	for (i = 0, rssiptr = rstrtok(arg,";"); rssiptr; rssiptr = rstrtok(NULL,";"), i++)
	{
		rssi[i] = simple_strtol(rssiptr,0,10);
	}

	if (table->Ops)
		table->Ops->SetParam(table, (UINT8*)&rssi[0], BND_SET_MIN_RSSI_TH);
	return TRUE;
}

INT Set_BndStrg_NSS_Thres(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	RTMP_STRING     *nssptr;
	UINT32			nss_thrs[2];
	INT32			i;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
	
	for (i = 0, nssptr = rstrtok(arg,";"); (nssptr && (i<2)); nssptr = rstrtok(NULL,";"), i++)
	{
		nss_thrs[i] = (UINT32) simple_strtol(nssptr, 0, 10);
		if (nss_thrs[i] > 4)
			return FALSE;
	}
	
	if ((table->Ops) && table->Ops->SetParam)
		table->Ops->SetParam(table, (UINT8*)&nss_thrs, BND_SET_NSS_TH);
	return TRUE;
}

INT Set_BndStrg_Sta_Poll_Period(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	UINT32			period;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    period = (UINT32) simple_strtoul(arg, 0, 10);
	
	if ((period < 3) || (period > 20))
		return FALSE;

	if ((table->Ops) && table->Ops->SetParam)
		table->Ops->SetParam(table, (UINT8*)&period, BND_SET_STA_POLL_PRD);
	return TRUE;
}

INT Set_BndStrg_Daemon_State(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
    PBND_STRG_CLI_TABLE table;
    POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	UINT32			daemon_state;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
    table = Get_BndStrgTable(pAd, ifIndex);
    if(!table)
        return FALSE;
    daemon_state = (UINT32) simple_strtoul(arg, 0, 10);
	
	if (daemon_state > 5) {
		printk("daemon_state is 0 ~ 5 \n\r");
		return FALSE;
	}

	if ((table->Ops) && table->Ops->SetParam)
		table->Ops->SetParam(table, (UINT8*)&daemon_state, BND_SET_DAEMON_STATE);
	return TRUE;
}

UINT8 GetNssFromHTCapRxMCSBitmask(UINT32 RxMCSBitmask)
{
	UCHAR	RxMCS[4];
	UINT8	nss;
	*((UINT32 *)RxMCS) = RxMCSBitmask;
	if(RxMCS[3] != 0)
		nss = 4;
	else if(RxMCS[2] != 0)
		nss = 3;
	else if(RxMCS[1] != 0)
		nss = 2;
	else
		nss = 1;
	return nss;
}

INT BndStrg_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, INT apidx)
{
	BNDSTRG_MSG msg;
	PBND_STRG_CLI_TABLE table = NULL;
	INT Status = NDIS_STATUS_SUCCESS;

    table = Get_BndStrgTable(pAd, apidx);
	if (!table ||(table->bInitialized == FALSE)){
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						("%s: BND_STRG_NOT_INITIALIZED on apidex[%d]!\n",__FUNCTION__, apidx));
		return BND_STRG_NOT_INITIALIZED;
	}	

	if (wrq->u.data.length != sizeof(BNDSTRG_MSG))
    {   
        BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s: The length of message not match!\n",__FUNCTION__));
		return BND_STRG_INVALID_ARG;
    }
	else
	{
		Status = copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length);
		if (table->Ops->MsgHandle)
			table->Ops->MsgHandle(pAd, table, &msg);
	}

	return BND_STRG_SUCCESS;
}

static VOID D_ShowTableInfo(PBND_STRG_CLI_TABLE table)
{
	BNDSTRG_MSG msg;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	UINT8 idx=0;
	
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\t BndStrgBssIdx"));
	for (idx =0; idx <	pAd->ApCfg.BssidNum; idx++)
	{
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%d",pAd->ApCfg.BndStrgBssIdx[idx]));
	}
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
	
	msg.Action = TABLE_INFO;

	D_BndStrgSendMsg(pAd, &msg);
}

static VOID D_ShowTableEntries(PBND_STRG_CLI_TABLE table,UINT32	display_type)
{
	BNDSTRG_MSG msg;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	CHAR	band_str[4][10]={"","5G","2.4G","2.4G/5G"};
	INT i;
	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\t%s Accessible Clients:  %d\n",band_str[table->Band],table->Size));

	for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++)
	{
		if (table->Entry[i].bValid)
		{
			if (MAC_ADDR_EQUAL(table->MonitorAddr, table->Entry[i].Addr)) {

				BND_STRG_PRINTQAMSG(table, table->Entry[i].Addr,
									(YLW("\t%d: %02x:%02x:%02x:%02x:%02x:%02x [TblIdx:%d]\n"), 
									i, PRINT_MAC(table->Entry[i].Addr),table->Entry[i].TableIndex));
			} else {
				BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\t%d: %02x:%02x:%02x:%02x:%02x:%02x [TblIdx:%d]\n",
							 i, PRINT_MAC(table->Entry[i].Addr),table->Entry[i].TableIndex));
			}
		}
	}

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\tBndStrg Table Entries:\n"));
	msg.Action = ENTRY_LIST;
	msg.data.display_type.display_type = display_type;
	msg.data.display_type.filer_band = table->Band;
	msg.data.display_type.channel = table->Channel;
	D_BndStrgSendMsg(pAd, &msg);
}

static BOOLEAN D_CheckConnectionReq(
			PRTMP_ADAPTER pAd,
			struct wifi_dev *wdev,
			PBND_STRG_CLI_TABLE table,
			PUCHAR pSrcAddr,
			UINT8 FrameType,
			PCHAR Rssi,
			BOOLEAN bAllowStaConnectInHt,
			BOOLEAN bVHTCap,
			UINT8 Nss)
{
	BNDSTRG_MSG msg;
    struct bnd_msg_conn_req *conn_req = &msg.data.conn_req;
	UINT32 frame_type_to_frame_check_flags[] = { \
								fBND_STRG_FRM_CHK_PRB_REQ,
								0,
								fBND_STRG_FRM_CHK_ASS_REQ,
								fBND_STRG_FRM_CHK_ATH_REQ};
	UINT32 frame_check_flags = 0;
	CHAR i, rssi_max;
	BOOLEAN IsInf2G = FALSE;

//#ifdef VENDOR_FEATURE5_SUPPORT
    if(!pAd->ApCfg.BndStrgBssIdx[wdev->func_idx])
        return TRUE;
//#endif
	/* Send to daemon */
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			rssi_max = pAd->dbdc_2G_rx_stream;
		else
			rssi_max = pAd->dbdc_5G_rx_stream;
	} else
#endif
		rssi_max = pAd->Antenna.field.RxPath;

	memset(conn_req->Rssi,0x80,sizeof(conn_req->Rssi));
	for ( i = 0; i < rssi_max; i++)
	{
		conn_req->Rssi[i] = Rssi[i];
	}

	msg.Action = CONNECTION_REQ;
	if (WMODE_CAP_2G(wdev->PhyMode) &&
			wdev->channel <= 14) {
		conn_req->Band = BAND_24G;
		IsInf2G = TRUE;
	}
	if (WMODE_CAP_5G(wdev->PhyMode) &&
			wdev->channel > 14) {
	    conn_req->Band = BAND_5G;
	}
    conn_req->Channel = wdev->channel;
	conn_req->FrameType = FrameType;
	conn_req->bAllowStaConnectInHt = bAllowStaConnectInHt;
	conn_req->bVHTCapable = bVHTCap;
	conn_req->Nss = Nss;
	COPY_MAC_ADDR(conn_req->Addr, pSrcAddr);


	D_BndStrgSendMsg(pAd, &msg);

	if (FrameType < (sizeof(frame_type_to_frame_check_flags)/sizeof(UINT32)))
		frame_check_flags = frame_type_to_frame_check_flags[FrameType];
	else
		{/* invalid frame type */}

	if (table->bEnabled == TRUE &&
		frame_check_flags & table->AlgCtrl.FrameCheck)
	{
		PBND_STRG_CLI_ENTRY entry = NULL;

		if (table->Ops)
			entry = table->Ops->TableLookup(table, pSrcAddr);

		if(entry && (FrameType == APMT2_PEER_AUTH_REQ) && (entry->BndStrg_Sta_State == BNDSTRG_STA_ASSOC)){
			BND_STRG_PRINTQAMSG(table, pSrcAddr,
			(RED("%s: (ch%d)  check %s request failed. client's (%02x:%02x:%02x:%02x:%02x:%02x)"
			" request is ignored. Client disconnected without DeAuth.!!Waiting for bndstrg result!!\n"), 
			(table->Band == BAND_24G ? "2.4G" : "5G"),table->Channel,"Auth",PRINT_MAC(pSrcAddr)));
			return FALSE;
		}

		if (entry) {
#ifdef BND_STRG_QA
			BND_STRG_PRINTQAMSG(table, pSrcAddr,
								(GRN("%s: (ch%d)  check %s request ok. client's (%02x:%02x:%02x:%02x:%02x:%02x)"
									 " request is accepted. \n"), (table->Band == BAND_24G ? "2.4G" : "5G"),table->Channel,
								 FrameType == 0 ? ("probe") : (FrameType == 3 ? "auth" : "unknow"),
								 PRINT_MAC(pSrcAddr)));
#endif
			return TRUE;
		} else {
#ifdef BND_STRG_QA
			BND_STRG_PRINTQAMSG(table, pSrcAddr,
			(RED("%s: (ch%d)  check %s request failed. client's (%02x:%02x:%02x:%02x:%02x:%02x)"
			" request is ignored. \n"), (table->Band == BAND_24G ? "2.4G" : "5G"),table->Channel,
			FrameType == 0 ? ("probe") : (FrameType == 3 ? "auth" : "unknow"),
			PRINT_MAC(pSrcAddr)));
#endif
			return FALSE;
		}			

	}
	
	return TRUE;
}

static VOID D_CLIStatusRsp(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	if (table->bInitialized == TRUE)
	{
		BNDSTRG_MSG new_msg;
		//struct bnd_msg_cli_status_req *cli_status_req = &msg->data.cli_status_req;
		struct bnd_msg_cli_status_rsp *cli_status_rsp = &new_msg.data.cli_status_rsp;
		MAC_TABLE_ENTRY *pEntry = NULL;
		PBND_STRG_CLI_ENTRY entry = NULL;
		int i = 0;
		/* Send to daemon */
		new_msg.Action = CLI_STATUS_RSP;
		//cli_status_rsp->TableIndex = cli_status_req->TableIndex;
		memset(cli_status_rsp,0x00, sizeof(struct bnd_msg_cli_status_rsp));
		//pEntry = MacTableLookup(pAd, cli_status_req->Addr);
		for (i=0; i < BND_STRG_MAX_TABLE_SIZE; i++)
		{
			entry = &table->Entry[i];
			/* pick up the first available vacancy*/
			if (!entry || entry->bValid == FALSE || !entry->bConnStatus){
				continue;
			}
			cli_status_rsp->TableIndex = entry->TableIndex;
			pEntry = MacTableLookup(pAd, entry->Addr);
			if(pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)&& (pEntry->pMbss != NULL))
			{
			INT32	avgrssi=0;//,i,avg_cnt=0;
			//get information
			//RSSI
			avgrssi=RTMPAvgRssi(pAd, pEntry->wdev, &pEntry->RssiSample);
			{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				if (pAd->chipCap.fgRateAdaptFWOffload == TRUE && (pEntry->bAutoTxRateSwitch == TRUE))
				{
					ULONG DataRate=0;
					ULONG DataRate_r=0;
					UCHAR phy_mode, rate, bw, sgi, stbc;
					UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
					UCHAR vht_nss;
					UCHAR vht_nss_r;
#endif
					UINT32 RawData;
					UINT32 RawData_r;
					UINT32 lastTxRate = pEntry->LastTxRate;
					UINT32 lastRxRate = pEntry->LastRxRate;
					if (pEntry->bAutoTxRateSwitch == TRUE)
					{
						EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
						HTTRANSMIT_SETTING LastTxRate;
						HTTRANSMIT_SETTING LastRxRate;
						MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
				
						LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
						LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
						LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1:0;
						LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1:0;
						LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;
						if (LastTxRate.field.MODE == MODE_VHT)
						{
							LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
						}
						else if (LastTxRate.field.MODE == MODE_OFDM)
						{
							LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
						}
						else
						{
							LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;
						}
						lastTxRate = (UINT32)(LastTxRate.word);
						LastRxRate.word = (USHORT)lastRxRate;
						RawData = lastTxRate;
						phy_mode = (RawData>>13) & 0x7;
						rate = RawData & 0x3F;
						bw = (RawData>>7) & 0x3;
						sgi = (RawData>>9) & 0x1;
						stbc = ((RawData>>10) & 0x1);
//----
						RawData_r = lastRxRate;
						phy_mode_r = (RawData_r>>13) & 0x7;
						rate_r = RawData_r & 0x3F;
						bw_r = (RawData_r>>7) & 0x3;
						sgi_r = (RawData_r>>9) & 0x1;
						stbc_r = ((RawData_r>>10) & 0x1);
#ifdef DOT11_VHT_AC
						if ( phy_mode == MODE_VHT ) {
							vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
							rate = rate & 0xF;
			//				snprintf(tmp_str,temp_str_len,"%dS-M%d/",vht_nss, rate);
						} else
#endif /* DOT11_VHT_AC */
						{
							//snprintf(tmp_str,temp_str_len,"%d/",rate);
						}
						
#ifdef DOT11_VHT_AC
						if ( phy_mode_r == MODE_VHT ) {
							vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
							rate_r = rate_r & 0xF;
						//	snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%dS-M%d",vht_nss_r, rate_r);
						} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
							if ( phy_mode_r >= MODE_HTMIX ){
								//snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",rate_r);
							} else
#endif
								if ( phy_mode_r == MODE_OFDM ) {
									if ( rate_r == TMI_TX_RATE_OFDM_6M )
										LastRxRate.field.MCS = 0;
									else if ( rate_r == TMI_TX_RATE_OFDM_9M )
										LastRxRate.field.MCS = 1;
									else if ( rate_r == TMI_TX_RATE_OFDM_12M )
										LastRxRate.field.MCS = 2;
									else if ( rate_r == TMI_TX_RATE_OFDM_18M )
										LastRxRate.field.MCS = 3;
									else if ( rate_r == TMI_TX_RATE_OFDM_24M )
										LastRxRate.field.MCS = 4;
									else if ( rate_r == TMI_TX_RATE_OFDM_36M )
										LastRxRate.field.MCS = 5;
									else if ( rate_r == TMI_TX_RATE_OFDM_48M )
										LastRxRate.field.MCS = 6;
									else if ( rate_r == TMI_TX_RATE_OFDM_54M )
										LastRxRate.field.MCS = 7;
									else
										LastRxRate.field.MCS = 0;
									//snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",LastRxRate.field.MCS);
								} else if ( phy_mode_r == MODE_CCK ) {	
									if ( rate_r == TMI_TX_RATE_CCK_1M_LP )
										LastRxRate.field.MCS = 0;
									else if ( rate_r == TMI_TX_RATE_CCK_2M_LP )
										LastRxRate.field.MCS = 1;
									else if ( rate_r == TMI_TX_RATE_CCK_5M_LP )
										LastRxRate.field.MCS = 2;
									else if ( rate_r == TMI_TX_RATE_CCK_11M_LP )
										LastRxRate.field.MCS = 3;
									else if ( rate_r == TMI_TX_RATE_CCK_2M_SP )
										LastRxRate.field.MCS = 1;
									else if ( rate_r == TMI_TX_RATE_CCK_5M_SP )
										LastRxRate.field.MCS = 2;
									else if ( rate_r == TMI_TX_RATE_CCK_11M_SP )
										LastRxRate.field.MCS = 3;
									else
										LastRxRate.field.MCS = 0;
									//snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",LastRxRate.field.MCS);
								}
						getRate(LastTxRate, &DataRate);
						getRate(LastRxRate, &DataRate_r);
						cli_status_rsp->data_tx_Rate = DataRate;
						cli_status_rsp->data_rx_Rate = DataRate_r;
						cli_status_rsp->data_tx_Phymode = phy_mode;
						cli_status_rsp->data_rx_Phymode = phy_mode_r;
						if (LastTxRate.field.MODE >= MODE_VHT) {
							cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS & 0xf;
							cli_status_rsp->data_tx_ant = (LastTxRate.field.MCS>>4) + 1;
						} else if (LastTxRate.field.MODE >= MODE_HTMIX) {
							cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS;
							cli_status_rsp->data_tx_ant = (LastTxRate.field.MCS >> 3)+1;
							if(cli_status_rsp->data_tx_mcs > 7)
								cli_status_rsp->data_tx_mcs %= 8;
						} else if (LastTxRate.field.MODE >= MODE_OFDM) {
							cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS;
							cli_status_rsp->data_tx_ant = 1;
						} else if (LastTxRate.field.MODE >= MODE_CCK) {
							cli_status_rsp->data_tx_mcs = LastTxRate.field.MCS;
							cli_status_rsp->data_tx_ant = 1;
						}
						if (LastRxRate.field.MODE >= MODE_VHT) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS & 0xf;
							cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS>>4) + 1;
						} else if (LastRxRate.field.MODE >= MODE_HTMIX) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
							cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS >> 3)+1;
							if(cli_status_rsp->data_rx_mcs > 7)
								cli_status_rsp->data_rx_mcs %= 8;
						} else if (LastRxRate.field.MODE >= MODE_OFDM) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
							cli_status_rsp->data_rx_ant = 1;
						} else if (LastRxRate.field.MODE >= MODE_CCK) {
							cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
							cli_status_rsp->data_rx_ant = 1;
						}
						cli_status_rsp->data_tx_bw = LastTxRate.field.BW;
						cli_status_rsp->data_rx_bw = LastRxRate.field.BW;
						cli_status_rsp->data_tx_sgi = LastTxRate.field.ShortGI;
						cli_status_rsp->data_rx_sgi = LastRxRate.field.ShortGI;
						cli_status_rsp->data_tx_stbc = LastTxRate.field.STBC;
						cli_status_rsp->data_rx_stbc = LastRxRate.field.STBC;

						cli_status_rsp->data_tx_packets = pEntry->TxPackets.QuadPart;
						cli_status_rsp->data_rx_packets = pEntry->RxPackets.QuadPart;
					}
				} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */		
				{
					ULONG DataRate=0;
					ULONG DataRate_r=0;
					UINT32 lastRxRate = pEntry->LastRxRate;
					HTTRANSMIT_SETTING LastRxRate;
					UCHAR phy_mode_r,rate_r;
					
					LastRxRate.word = (USHORT)lastRxRate;
					phy_mode_r = LastRxRate.field.MODE;
					rate_r = LastRxRate.word & 0x3F;
					
					if ( phy_mode_r == MODE_OFDM ) {
						if ( rate_r == TMI_TX_RATE_OFDM_6M )
							LastRxRate.field.MCS = 0;
						else if ( rate_r == TMI_TX_RATE_OFDM_9M )
							LastRxRate.field.MCS = 1;
						else if ( rate_r == TMI_TX_RATE_OFDM_12M )
							LastRxRate.field.MCS = 2;
						else if ( rate_r == TMI_TX_RATE_OFDM_18M )
							LastRxRate.field.MCS = 3;
						else if ( rate_r == TMI_TX_RATE_OFDM_24M )
							LastRxRate.field.MCS = 4;
						else if ( rate_r == TMI_TX_RATE_OFDM_36M )
							LastRxRate.field.MCS = 5;
						else if ( rate_r == TMI_TX_RATE_OFDM_48M )
							LastRxRate.field.MCS = 6;
						else if ( rate_r == TMI_TX_RATE_OFDM_54M )
							LastRxRate.field.MCS = 7;
						else
							LastRxRate.field.MCS = 0;
					} else if ( phy_mode_r == MODE_CCK ) {	
						if ( rate_r == TMI_TX_RATE_CCK_1M_LP )
							LastRxRate.field.MCS = 0;
						else if ( rate_r == TMI_TX_RATE_CCK_2M_LP )
							LastRxRate.field.MCS = 1;
						else if ( rate_r == TMI_TX_RATE_CCK_5M_LP )
							LastRxRate.field.MCS = 2;
						else if ( rate_r == TMI_TX_RATE_CCK_11M_LP )
							LastRxRate.field.MCS = 3;
						else if ( rate_r == TMI_TX_RATE_CCK_2M_SP )
							LastRxRate.field.MCS = 1;
						else if ( rate_r == TMI_TX_RATE_CCK_5M_SP )
							LastRxRate.field.MCS = 2;
						else if ( rate_r == TMI_TX_RATE_CCK_11M_SP )
							LastRxRate.field.MCS = 3;
						else
							LastRxRate.field.MCS = 0;
					}
					
					
					getRate(pEntry->HTPhyMode, &DataRate);
					getRate(LastRxRate, &DataRate_r);
					cli_status_rsp->data_tx_Rate = DataRate;
					cli_status_rsp->data_rx_Rate = DataRate_r;
					cli_status_rsp->data_tx_Phymode = pEntry->HTPhyMode.field.MODE;
					cli_status_rsp->data_rx_Phymode = 0;
					if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
						cli_status_rsp->data_tx_mcs = (pEntry->HTPhyMode.field.MCS & 0xf);
						cli_status_rsp->data_tx_ant = (pEntry->HTPhyMode.field.MCS>>4) + 1;
					} else if (pEntry->HTPhyMode.field.MODE >= MODE_HTMIX) {
						cli_status_rsp->data_tx_mcs = pEntry->HTPhyMode.field.MCS;
						cli_status_rsp->data_tx_ant = (pEntry->HTPhyMode.field.MCS >> 3)+1;
						if(cli_status_rsp->data_tx_mcs > 7)
							cli_status_rsp->data_tx_mcs %= 8;
					} else if (pEntry->HTPhyMode.field.MODE >= MODE_OFDM) {
						cli_status_rsp->data_tx_mcs = pEntry->HTPhyMode.field.MCS;
						cli_status_rsp->data_tx_ant = 1;
					} else if (pEntry->HTPhyMode.field.MODE >= MODE_CCK) {
						cli_status_rsp->data_tx_mcs = pEntry->HTPhyMode.field.MCS;
						cli_status_rsp->data_tx_ant = 1;
					}

					if (LastRxRate.field.MODE >= MODE_VHT) {
						cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS & 0xf;
						cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS>>4) + 1;
					} else if (LastRxRate.field.MODE >= MODE_HTMIX) {
						cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
						cli_status_rsp->data_rx_ant = (LastRxRate.field.MCS >> 3)+1;
						if(cli_status_rsp->data_rx_mcs > 7)
							cli_status_rsp->data_rx_mcs %= 8;
					} else if (LastRxRate.field.MODE >= MODE_OFDM) {
						cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
						cli_status_rsp->data_rx_ant = 1;
					} else if (LastRxRate.field.MODE >= MODE_CCK) {
						cli_status_rsp->data_rx_mcs = LastRxRate.field.MCS;
						cli_status_rsp->data_rx_ant = 1;
					}
					cli_status_rsp->data_tx_bw = pEntry->HTPhyMode.field.BW;
					cli_status_rsp->data_rx_bw = LastRxRate.field.BW;
					cli_status_rsp->data_tx_sgi = pEntry->HTPhyMode.field.ShortGI;
					cli_status_rsp->data_rx_sgi = LastRxRate.field.ShortGI;
					cli_status_rsp->data_tx_stbc = pEntry->HTPhyMode.field.STBC;
					cli_status_rsp->data_rx_stbc = LastRxRate.field.STBC;
					cli_status_rsp->data_tx_packets = pEntry->TxPackets.QuadPart;
					cli_status_rsp->data_rx_packets = pEntry->RxPackets.QuadPart;
				}				
			}
			cli_status_rsp->data_Rssi = (char)avgrssi;
			//cli_status_rsp->data_tx_TP = pEntry->AvgTxBytes >> 17; //Mbps
			//cli_status_rsp->data_rx_TP = pEntry->AvgRxBytes >> 17; //Mbps
			cli_status_rsp->data_tx_Byte = pEntry->AvgTxBytes;
			cli_status_rsp->data_rx_Byte = pEntry->AvgRxBytes;
			memcpy(cli_status_rsp->Addr, entry->Addr, MAC_ADDR_LEN);
			}
			cli_status_rsp->ReturnCode = BND_STRG_SUCCESS;
	        D_BndStrgSendMsg(pAd, &new_msg);
		}
	}
	return;
}

static VOID D_QLOADStatusRsp(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	if (table->bInitialized == TRUE)
	{
		BNDSTRG_MSG new_msg;
		//QLOAD_CTRL *pQloadCtrl;
		struct bnd_msg_qload_status_rsp *qload_status_rsp = &new_msg.data.qload_status_rsp;
		/* Send to daemon */
		new_msg.Action = QLOAD_STATUS_RSP;
		qload_status_rsp->ReturnCode= BND_STRG_SUCCESS;
		qload_status_rsp->band = table->Band;
		qload_status_rsp->Channel= table->Channel;
		
		{
			UINT32  ChanBusyTime[DBDC_BAND_NUM] = {0};
			UINT32  ObssAirTime[DBDC_BAND_NUM] = {0};
			UINT32  MyTxAirTime[DBDC_BAND_NUM] = {0};
			UINT32  MyRxAirTime[DBDC_BAND_NUM] = {0};
			UINT32  EDCCATime[DBDC_BAND_NUM] = {0};
			UCHAR   ChanBusyOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   ObssAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   MyAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   MyTxAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   MyRxAirOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR   EdccaOccupyPercentage[DBDC_BAND_NUM] = {0};
			UCHAR	i;
			i = HcGetBandByChannel(pAd, table->Channel);

			ChanBusyTime[i] = pAd->OneSecMibBucket.ChannelBusyTime[i];
			
			ObssAirTime[i] = Get_OBSS_AirTime(pAd, i);
			MyTxAirTime[i] = Get_My_Tx_AirTime(pAd, i);
			MyRxAirTime[i] = Get_My_Rx_AirTime(pAd, i);
			EDCCATime[i] = Get_EDCCA_Time(pAd, i);
			
			if (ChanBusyTime[i] != 0)
				ChanBusyOccupyPercentage[i] = (ChanBusyTime[i]*100)/ONE_SEC_2_US; 
			if (ObssAirTime[i] != 0)
				ObssAirOccupyPercentage[i] = (ObssAirTime[i]*100)/ONE_SEC_2_US; 
			if (MyTxAirTime[i] != 0 || MyRxAirTime [i] != 0 )
				MyAirOccupyPercentage[i] = ((MyTxAirTime[i] + MyRxAirTime[i]) *100)/ONE_SEC_2_US;
			if (MyTxAirTime[i] != 0) 
				MyTxAirOccupyPercentage[i] = (MyTxAirTime[i] * 100) /ONE_SEC_2_US;
			if (MyRxAirTime[i] != 0) 
				MyRxAirOccupyPercentage[i] = (MyRxAirTime[i] * 100) /ONE_SEC_2_US;
			if (EDCCATime[i] != 0)
				EdccaOccupyPercentage[i] = (EDCCATime[i] * 100) / ONE_SEC_2_US;
			
			//qload_status_rsp->chan_busy_load = ChanBusyOccupyPercentage[i];
			//qload_status_rsp->obss_load = ObssAirOccupyPercentage[i];
			//qload_status_rsp->edcca_load = EdccaOccupyPercentage[i];
			//qload_status_rsp->myair_load = MyAirOccupyPercentage[i];
			//qload_status_rsp->mytxair_load =  MyTxAirOccupyPercentage[i];
			//qload_status_rsp->myrxair_load =  MyRxAirOccupyPercentage[i];
			qload_status_rsp->qload = MyAirOccupyPercentage[i]+ ObssAirOccupyPercentage[i];
		}
		D_BndStrgSendMsg(pAd, &new_msg);
	}
}
static VOID D_InfStatusRsp(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
    struct bnd_msg_inf_status_req *inf_status_req = &msg->data.inf_status_req;
    if (table->bInitialized == TRUE)
    {
        BNDSTRG_MSG new_msg;
        struct bnd_msg_inf_status_rsp *inf_status_rsp = &new_msg.data.inf_status_rsp;
        
		/* Send to daemon */
		new_msg.Action = INF_STATUS_RSP;		
		if (IS_2G_BAND(table->Band))
			inf_status_rsp->band = BAND_24G;
		else if (IS_5G_BAND(table->Band)) 
			inf_status_rsp->band = BAND_5G;	
		inf_status_rsp->bInfReady = table->bInfReady;
        inf_status_rsp->Channel = table->Channel;
        inf_status_rsp->bVHTCapable = table->bVHTCapable;
		inf_status_rsp->nss = table->nss;
        inf_status_rsp->table_src_addr = (ULONG)table;
		inf_status_rsp->table_size = BND_STRG_MAX_TABLE_SIZE;
		strcpy(inf_status_rsp->ucIfName, inf_status_req->ucIfName);

		inf_status_rsp->nvram_support = 0;
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%s:INF [%s]STATUS QUERY ON\n",__FUNCTION__, inf_status_req->ucIfName));
		D_BndStrgSendMsg(pAd, &new_msg);
    }
}

/* For IOCTL */
static INT D_SetEnable(
			PBND_STRG_CLI_TABLE table,
			BOOLEAN enable)
{
	INT ret_val = BND_STRG_SUCCESS;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;

	if (!(enable ^ pAd->ApCfg.BandSteering)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				 ("%s(): BandSteeringenable is already %d\n", __FUNCTION__,  enable));
		return FALSE;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): enable = %u\n", __FUNCTION__,  enable));
	pAd->ApCfg.BandSteering = enable;

	if (enable)
		ret_val = BndStrg_Init(pAd);
	else
		ret_val = BndStrg_Release(pAd);

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return TRUE;
}


static INT D_SetRssiDiff(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiDiff)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_rssi *rssi = &msg.data.rssi;

	table->RssiDiff = RssiDiff;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): RssiCheck = %u\n", __FUNCTION__, table->RssiDiff));

	msg.Action = SET_RSSI_DIFF;
	rssi->Rssi = RssiDiff;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetRssiLow(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiLow)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_rssi *rssi = &msg.data.rssi;

	table->RssiLow = RssiLow;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): RssiLow = %d\n", __FUNCTION__, table->RssiLow));

	msg.Action = SET_RSSI_LOW;
	rssi->Rssi = RssiLow;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetAgeTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_time *time = &msg.data.time;

	table->AgeTime = Time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): AgeTime = %u\n", __FUNCTION__, table->AgeTime));

	msg.Action = SET_AGE_TIME;
	time->Time = table->AgeTime;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}

static INT D_SetMaxSteerCnt(
			PBND_STRG_CLI_TABLE table,
			UINT8 count)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_max_steer_count *max_steer_count = &msg.data.max_steer_count;
	msg.Action = SET_MAX_STEER;
	table->max_steer_count = count;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): Max_Steer_Cnt = %u\n", __FUNCTION__, count));
	max_steer_count->max_count = table->max_steer_count;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}

static INT D_SetDwellTime(
				PBND_STRG_CLI_TABLE table,
				UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
	struct bnd_msg_time *time = &msg.data.time;
	table->dwell_time = Time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): DwellTime = %u\n", __FUNCTION__, table->dwell_time));
	msg.Action = SET_DWELL_TIME;
	time->Time = table->dwell_time;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}

static INT D_SetSteerTimeWindow(
				PBND_STRG_CLI_TABLE table,
				UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
	struct bnd_msg_time *time = &msg.data.time;
	table->max_steer_time_window = Time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): DwellTime = %u\n", __FUNCTION__, table->max_steer_time_window));
	msg.Action = SET_MAX_STEER_TIME_WINDOW;
	time->Time = Time;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}

static INT D_SetHoldTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_time *time = &msg.data.time;
	
	table->HoldTime= Time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): HoldTime = %u\n", __FUNCTION__, table->HoldTime));

	msg.Action = SET_HOLD_TIME;
	time->Time = table->HoldTime;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetCheckTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_time *time = &msg.data.time;
    
	table->CheckTime = Time;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("%s(): CheckTime = %u\n", __FUNCTION__, table->CheckTime));

	msg.Action = SET_CHECK_TIME;
	time->Time = Time;
	time->Band = table->Band;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetFrmChkFlag(
			PBND_STRG_CLI_TABLE table,
			UINT32	FrmChkFlag)
{
	table->AlgCtrl.FrameCheck = FrmChkFlag;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s(): FrameCheck = 0x%x\n", __FUNCTION__, table->AlgCtrl.FrameCheck));

	return TRUE;
}

static INT D_SetCndPriority(
			PBND_STRG_CLI_TABLE table,
			UINT8 *CndPri,
			UINT8 size)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_cnd_priority *cnd_priority = &msg.data.cnd_priority;
	UINT8 i;
    
	for(i=0; i < size; i++)
		cnd_priority->PriorityList[i] = (UINT32)CndPri[i];
    
	cnd_priority->PriorityListSize = size;
	msg.Action = SET_CND_PRIORITY;
	D_BndStrgSendMsg(pAd, &msg);
	return TRUE;
}

static INT D_SetParam(
	PBND_STRG_CLI_TABLE table,
	UINT8 *ptr,
	UINT32 type)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_set_param *set_param = &msg.data.set_param;

	msg.Action = SET_PARAM;
	switch (type) {
		case BND_SET_STEERING_NUM:
			{
				UINT32	*input_data = (UINT32*)ptr;
				set_param->type = type;
				set_param->data.steering.total_num = *input_data;
			}
			break;
		case BND_SET_ASSOC_BL_TH:
			{
				UINT32	*input_data = (UINT32*)ptr;
				set_param->type = type;
				set_param->data.assoc.assoc_bl_th = *input_data;
			}
			break;
		case BND_SET_QLOAD_TH:
			{
				UINT32	*input_data = (UINT32*)ptr;
				set_param->type = type;
				set_param->data.qload.qload_th = *input_data;
			}
			break;
		case BND_SET_MIN_RSSI_TH:
			{
				INT32	*input_data = (INT32*)ptr;
				set_param->type = type;
				set_param->data.min_rssi.min_rssi = *input_data;
			}
			break;
		case BND_SET_NSS_TH:
			{
				UINT32	*input_data = (UINT32*)ptr;
				set_param->type = type;
				set_param->data.nss.nss_th = *input_data;
			}
			break;
		case BND_SET_STA_POLL_PRD:
			{
				UINT32	*input_data = (UINT32*)ptr;
				set_param->type = type;
				set_param->data.period.period = *input_data;
			}
			break;
		case BND_SET_DAEMON_STATE:
			{
				UINT32	*input_data = (UINT32*)ptr;
				set_param->type = type;
				set_param->data.daemon_state.state = *input_data;
			}
			break;
		default:
			break;
	}
	set_param->channel = table->Channel;
	set_param->band = table->Band;
	D_BndStrgSendMsg(pAd, &msg);
	return TRUE;
}

#ifdef BND_STRG_DBG
static INT D_SetMntAddr(
			PBND_STRG_CLI_TABLE table,
			PUCHAR Addr)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
    struct bnd_msg_mnt_addr *mnt_addr = &msg.data.mnt_addr;
	
	COPY_MAC_ADDR(table->MonitorAddr, Addr);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(table->MonitorAddr)));

	msg.Action = SET_MNT_ADDR;
	COPY_MAC_ADDR(mnt_addr->Addr, table->MonitorAddr);
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}
#endif /* BND_STRG_DBG */

static VOID D_MsgHandle(
			PRTMP_ADAPTER	pAd,
			PBND_STRG_CLI_TABLE table,
			BNDSTRG_MSG *msg)
{
	PBND_STRG_CLI_ENTRY entry = NULL;

	BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: action code (%d)\n",__FUNCTION__, msg->Action));

	if ((table->DaemonPid != 0xffffffff) && (table->DaemonPid != current->pid)) {
		BNDSTRG_MSG new_msg;
		new_msg.Action = REJECT_EVENT;
		new_msg.data.reject_body.DaemonPid = table->DaemonPid;
		D_BndStrgSendMsg(pAd, &new_msg);
		BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Unknown BndStrg PID\n",__FUNCTION__));
		return;
	}

	switch (msg->Action)
	{
		case CLI_ADD:
            {
                struct bnd_msg_cli_add *cli_add = &msg->data.cli_add;
			    entry = table->Ops->TableLookup(table, cli_add->Addr);
				if (entry == NULL)
					table->Ops->TableEntryAdd(table, cli_add, &entry);
				else
					entry->BndStrg_Sta_State = BNDSTRG_STA_INIT;
            }
			break;

		case CLI_DEL:
            {
                struct bnd_msg_cli_del *cli_del = &msg->data.cli_del;
				MAC_TABLE_ENTRY *pEntry = NULL;
				
				if (table->Ops->TableLookup(table, cli_del->Addr)) {
					/*remove sta if sta is existed*/
					pEntry = MacTableLookup(pAd, cli_del->Addr);
					if (pEntry)
					{
#ifdef BND_STRG_QA
						BND_STRG_PRINTQAMSG(table, cli_del->Addr,
											(RED("\n\r\n\r%s[%d]: kick out client's (%02x:%02x:%02x:%02x:%02x:%02x)\n\r\n\r"),
											 __func__,__LINE__,PRINT_MAC(cli_del->Addr)));
#endif
						MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
						MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
					}
					table->Ops->TableEntryDel(table, cli_del->Addr, 0xFF);

#ifdef BND_STRG_QA
					BND_STRG_PRINTQAMSG(table, cli_del->Addr,
										(RED("\n\r\n\r%s[%d]: CLI_DEL client's (%02x:%02x:%02x:%02x:%02x:%02x),channel=%d\n\r\n\r"),
										 __func__,__LINE__,PRINT_MAC(cli_del->Addr),table->Channel));
#endif
				}
            }
			break;

		case CLI_AGING_REQ:
            {
                struct bnd_msg_cli_aging_req *cli_aging_req = &msg->data.cli_aging_req;
                BNDSTRG_MSG new_msg;
                struct bnd_msg_cli_aging_rsp *cli_aging_rsp = &new_msg.data.cli_aging_rsp;
                MAC_TABLE_ENTRY *entry = NULL;
                
    			new_msg.Action = CLI_AGING_RSP;
    			cli_aging_rsp->Band = table->Band;
                cli_aging_rsp->TableIndex = cli_aging_req->TableIndex;
				cli_aging_rsp->channel = table->Channel;
				memcpy(cli_aging_rsp->Addr,cli_aging_req->Addr,6);
				entry = MacTableLookup(pAd, cli_aging_req->Addr);
    			if ((entry == NULL) || 
					((entry != NULL) && (table->Channel != entry->wdev->channel)))
    			{
    				/* 
					 * It can remove bndstrg entry because the mac entry is not 
					 * related with bndstrg entry. this mac entry belog to another 
					 * bndstrg table for DBDC case.
					 */
    				cli_aging_rsp->ReturnCode = BND_STRG_SUCCESS;
					memcpy(cli_aging_rsp->Addr,cli_aging_req->Addr,6);
    				table->Ops->TableEntryDel(table, cli_aging_req->Addr, 0xFF);
    			}
    			else
    			{
    				cli_aging_rsp->ReturnCode = BND_STRG_STA_IS_CONNECTED;
    			}

    			D_BndStrgSendMsg(pAd, &new_msg);
            }

			break;
		case CLI_STATUS_REQ:
			//TBD,return RSSI,qload,latest tx/rx rate,
			if(table->bInfReady)
			    D_CLIStatusRsp(pAd, table, msg);
			break;
		case QLOAD_STATUS_REQ:
			//TBD,return RSSI,qload,latest tx/rx rate,
			if(table->bInfReady)
			    D_QLOADStatusRsp(pAd, table, msg);
			break;
		case INF_STATUS_QUERY:
            if(table->bInfReady)
			    D_InfStatusRsp(pAd, table, msg);
			break;

		case HEARTBEAT_MONITOR:
            if(table->bInfReady)
			    pAd->ApCfg.BndStrgHeartbeatCount++;
			break;

		case BNDSTRG_ONOFF:
            if(table && table->bInitialized)
            {
                struct bnd_msg_onoff *onoff = &msg->data.onoff;
				UINT32	ap_idx;
				UINT32 i;

				if (onoff->OnOff && (table->bEnabled ^ onoff->OnOff))
				{
					/* disconnect all connected STA to keep the link status
					 * between bndstrg daemon and driver
					 */
					for(ap_idx=MAIN_MBSSID; ap_idx < pAd->ApCfg.BssidNum; ap_idx++)
					{
						if(pAd->ApCfg.BndStrgBssIdx[ap_idx] == TRUE)
							APMlmeKickOutAllSta(pAd, ap_idx, REASON_DEAUTH_STA_LEAVING);
					}
					
					for (i=1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
					{
						MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
						if (IS_ENTRY_CLIENT(pEntry) && 
							pAd->ApCfg.BndStrgBssIdx[pEntry->func_tb_idx] == TRUE)
						{
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s: MacTableDeleteEntry %x:%x:%x:%x:%x:%x\n",
								__FUNCTION__, PRINT_MAC(pEntry->Addr)));
							MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
						}
					}
					
			        D_SetCndPriority(table, table->BndStrgCndPri, table->BndStrgCndPriSize);
					D_SetDwellTime(table, table->dwell_time);
					D_SetSteerTimeWindow(table, table->max_steer_time_window);
					D_SetMaxSteerCnt(table, table->max_steer_count);
					D_SetAgeTime(table,table->AgeTime);
					D_SetCheckTime(table,table->CheckTime);
					D_SetHoldTime(table,table->HoldTime);
					D_SetRssiLow(table,table->RssiLow);
                }
				else if ((onoff->OnOff == 0) && (table->bEnabled ^ onoff->OnOff))
				{
					if(table->Size > 0)
					{
						PBND_STRG_CLI_ENTRY entry = NULL;
						for (i=0;i<BND_STRG_MAX_TABLE_SIZE;i++) {
							entry = &table->Entry[i];
							if (entry->bValid == TRUE) {
								table->Ops->TableEntryDel(table, entry->Addr, i);
							}
						}
					}
				}
                BndStrg_Tbl_Enable(table, onoff->OnOff, onoff->ucIfName);
                if(table->bEnabled)
                {
					table->DaemonPid = current->pid;
                }
                else
                {
    				table->DaemonPid = 0xffffffff;
                }
            }
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: unknown action code. (%d)\n",__FUNCTION__, msg->Action));
			break;
	}
}

void BndStrgHeartBeatMonitor(PRTMP_ADAPTER	pAd)
{
	if(pAd->ApCfg.BndStrgTable[BAND0].bEnabled
#ifdef DBDC_MODE			
		|| pAd->ApCfg.BndStrgTable[BAND1].bEnabled
#endif			
	){
		
		if(pAd->ApCfg.BndStrgHeartbeatMonitor != pAd->ApCfg.BndStrgHeartbeatCount) {
			pAd->ApCfg.BndStrgHeartbeatMonitor = pAd->ApCfg.BndStrgHeartbeatCount;
			pAd->ApCfg.BndStrgHeartbeatNoChange = 0;
			return;
		}
		else {
			pAd->ApCfg.BndStrgHeartbeatNoChange++;
		}
		if(pAd->ApCfg.BndStrgHeartbeatNoChange == 20) {
			BND_STRG_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:BndStrg Daemon Killed\n",__FUNCTION__));
			pAd->ApCfg.BndStrgHeartbeatNoChange = 0;
			BndStrg_Release(pAd);
		}
	}
}

BNDSTRG_OPS D_BndStrgOps = {
	.ShowTableInfo = D_ShowTableInfo,
	.ShowTableEntries = D_ShowTableEntries,
	.TableEntryAdd = BndStrg_InsertEntry,
	.TableEntryDel = BndStrg_DeleteEntry,
	.TableLookup = BndStrg_TableLookup,
	.CheckConnectionReq = D_CheckConnectionReq,
	.SetEnable = D_SetEnable,
	.SetRssiDiff = D_SetRssiDiff,
	.SetRssiLow = D_SetRssiLow,
	.SetAgeTime = D_SetAgeTime,
	.SetMaxSteerCnt = D_SetMaxSteerCnt,
	.SetDwellTime = D_SetDwellTime,
	.SetSteerTimeWindow = D_SetSteerTimeWindow,
	.SetHoldTime = D_SetHoldTime,
	.SetCheckTime = D_SetCheckTime,
	.SetFrmChkFlag = D_SetFrmChkFlag,
//	.SetCndChkFlag = D_SetCndChkFlag,
	.SetCndPriority = D_SetCndPriority,
	.SetParam = D_SetParam,
#ifdef BND_STRG_DBG
	.SetMntAddr = D_SetMntAddr,
#endif /* BND_STRG_DBG */
	.MsgHandle= D_MsgHandle,
};

void BndStrgSetProfileParam(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	INT	i = 0;	
	RTMP_STRING *macptr = NULL;
	if(RTMPGetKeyParameter("BandSteering", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BandSteering = (UCHAR) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF, ("BandSteering=%d\n", pAd->ApCfg.BandSteering));
	}
	if(RTMPGetKeyParameter("BndStrgBssIdx", tmpbuf, 50, pBuffer, TRUE))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF, ("BndStrgBssIdx=%s\n", tmpbuf));
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			pAd->ApCfg.BndStrgBssIdx[i] = simple_strtoul(macptr,0,10);		
		}
		if(i==0)
			pAd->ApCfg.BndStrgBssIdx[MAIN_MBSSID] = 1;
	}else{
		pAd->ApCfg.BndStrgBssIdx[MAIN_MBSSID] = 1;
	}
	if(RTMPGetKeyParameter("BndStrgCndPriority", tmpbuf, 50, pBuffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			pAd->ApCfg.BndStrgCndPri[i] = simple_strtoul(macptr,0,10);
		}
		pAd->ApCfg.BndStrgCndPriSize = i;
	}
	else
	{
		/* 
		 RSSI_DIFF = 0, BAND_PERSIST = 1, HT_SUPPORT = 2, 5G_RSSI = 3, VHT_SUPPORT = 4,
		 NSS_SUPPORT = 5, LB_QLOAD = 6, LB_STA_COUNT = 7, LB_RSSI = 8, LB_MCS = 9,
		 DEFAULT_2G = 10, DEFAULT_5G = 11,5G_RSSI_DYNAMIC = 12
		 */
		UINT8	priority_list[]={0,3,2,11};
		for (i = 0; i < (sizeof(priority_list)/sizeof(priority_list[0]));i++)
		{
			pAd->ApCfg.BndStrgCndPri[i] = priority_list[i];
		}
		pAd->ApCfg.BndStrgCndPriSize = i;
	}
	if(RTMPGetKeyParameter("BndStrgDwellTime", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgDwellTime = (UINT32) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("BndStrgDwellTime=%u\n", pAd->ApCfg.BndStrgDwellTime));
	}else
	{
		pAd->ApCfg.BndStrgDwellTime = DWELL_TIME;
	}
	if(RTMPGetKeyParameter("BndStrgSteerTimeWindow", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgSteerTimeWindow = (UINT32) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("BndStrgSteerTimeWindow=%u\n", pAd->ApCfg.BndStrgSteerTimeWindow));
	}else
	{
		pAd->ApCfg.BndStrgSteerTimeWindow = MAX_STEER_TIME_WINDOW;

	}if(RTMPGetKeyParameter("BndStrgMaxSteerCount", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgMaxSteerCount = (UINT8) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_OFF, ("BndStrgMaxSteerCount=%u\n", pAd->ApCfg.BndStrgMaxSteerCount));
	}else
	{
		pAd->ApCfg.BndStrgMaxSteerCount = MAX_STEERING_COUNT;
	}
	if(RTMPGetKeyParameter("BndStrgAge", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgAge = (UINT8) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("BndStrgAge=%x\n", pAd->ApCfg.BndStrgAge));
	}else
	{
		pAd->ApCfg.BndStrgAge = BND_STRG_AGE_TIME;
	}
	if(RTMPGetKeyParameter("BndStrgAge", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgAge = (UINT32) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("BndStrgAge=%x\n", pAd->ApCfg.BndStrgAge));
	}else
	{
		pAd->ApCfg.BndStrgAge = BND_STRG_AGE_TIME;
	}
	if(RTMPGetKeyParameter("BndStrgCheckTime", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgCheckTime = (UINT32) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("BndStrgCheckTime=%x\n", pAd->ApCfg.BndStrgCheckTime));
	}else
	{
		pAd->ApCfg.BndStrgCheckTime = BND_STRG_CHECK_TIME;
	}
	if(RTMPGetKeyParameter("BndStrgHoldTime", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgCheckTime = (UINT32) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("BndStrgHoldTime=%x\n", pAd->ApCfg.BndStrgHoldTime));
	}else
	{
		pAd->ApCfg.BndStrgHoldTime = BND_STRG_HOLD_TIME;
	}
	if(RTMPGetKeyParameter("BndStrgRssiLow", tmpbuf, 10, pBuffer, TRUE))
	{
		pAd->ApCfg.BndStrgRssiLow = (CHAR) simple_strtol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_TRACE, ("BndStrgRssiLow=%x\n", pAd->ApCfg.BndStrgRssiLow));
	}else
	{
		pAd->ApCfg.BndStrgRssiLow = BND_STRG_RSSI_LOW;
	}
}

#endif /* BAND_STEERING */

