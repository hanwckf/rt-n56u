/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include "rt_config.h"
#include "hdev/hdev.h"


#define INVAILD_WCID 0xff

/*
* local function
*/
static VOID wtc_mcast_complete(WTBL_CFG *cfg)
{
	RTMP_OS_COMPLETE(&cfg->mcast_complete);
}

static VOID wtc_mcast_wait_complete(WTBL_CFG *cfg)
{
    NdisAcquireSpinLock(&cfg->WtblIdxRecLock);
	cfg->mcast_wait  = TRUE;
    NdisReleaseSpinLock(&cfg->WtblIdxRecLock);
	RTMP_OS_INIT_COMPLETION(&cfg->mcast_complete);

	if(!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&cfg->mcast_complete,WTC_WAIT_TIMEOUT))
	{
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
		("(%s) mcast wait can't done.\n", __FUNCTION__));
	}
}

/*
*
*/
static UCHAR wtc_acquire_groupkey_wcid(WTBL_CFG *pWtblCfg,HD_DEV_OBJ *pObj)
{
    UCHAR AvailableWcid = INVAILD_WCID;
    UCHAR OmacIdx, WdevType;
    int i;
    WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR min_wcid = pWtblCfg->MinMcastWcid;

    OmacIdx = pObj->OmacIdx;
    WdevType = pObj->Type;

    NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

    for (i = (MAX_LEN_OF_TR_TABLE - 1); i >= min_wcid; i--)
    {
        pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];
        if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
        {
            continue;
        }
        else
        {
            pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
            pWtblIdxRec->WtblIdx = i;
            /*TODO: Carter, check flow when calling this function, OmacIdx might be erroed.*/
            pWtblIdxRec->LinkToOmacIdx = OmacIdx;
            pWtblIdxRec->LinkToWdevType = WdevType;
            AvailableWcid = (UCHAR)i;

            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s: Found a non-occupied wtbl_idx:%d for WDEV_TYPE:%d\n"
                        " LinkToOmacIdx = %x, LinkToWdevType = %d\n",
                        __FUNCTION__, i, WdevType, OmacIdx, WdevType));

            NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
            return AvailableWcid;
        }
    }
    NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);

    if (i < min_wcid)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s: no available wtbl_idx for WDEV_TYPE:%d\n",
                        __FUNCTION__, WdevType));
    }

    return AvailableWcid;
}


/*Wtable control*/
/*
*
*/
VOID WtcInit(HD_CFG *pHdCfg)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblParm = NULL;
	UCHAR i=0;

	os_zero_mem(pWtblCfg,sizeof(WTBL_CFG));

    NdisAllocateSpinLock(NULL,&pWtblCfg->WtblIdxRecLock);
	
	for(i=0;i<MAX_LEN_OF_TR_TABLE;i++)
	{
		pWtblParm = &pWtblCfg->WtblIdxRec[i];
		pWtblParm->State = WTBL_STATE_NONE_OCCUPIED;
	}
}


/*
*
*/
VOID WtcExit(HD_CFG *pHdCfg)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;

    NdisFreeSpinLock(&pWtblCfg->WtblIdxRecLock);
	os_zero_mem(pWtblCfg,sizeof(WTBL_CFG));
}


/*
*
*/
UCHAR WtcSetMaxStaNum(HD_CFG *pHdCfg,UCHAR BssidNum,UCHAR MSTANum)
{
    UCHAR wtbl_num_resv_for_mcast = 0;
    UCHAR wtbl_num_use_for_ucast = 0;
    UCHAR wtbl_num_use_for_sta = 0;
    UCHAR MaxNumChipRept = 0;
    UCHAR ApcliNum = MAX_APCLI_NUM;
    UCHAR WdsNum = MAX_WDS_ENTRY;
	UCHAR MaxUcastEntryNum=0;

#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	RTMP_CHIP_CAP *pChipCap = &pHdCfg->chipCap;
    MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(pChipCap);
#endif /*MAC_REPEATER_SUPPROT*/
#endif /*CONFIG_AP_SUPPORT*/


    wtbl_num_resv_for_mcast = BssidNum + ApcliNum + MSTANum;
    wtbl_num_use_for_ucast = WdsNum + MaxNumChipRept + ApcliNum + MSTANum;
    wtbl_num_use_for_sta = MAX_LEN_OF_MAC_TABLE -
                            wtbl_num_resv_for_mcast -
                            wtbl_num_use_for_ucast;
    MaxUcastEntryNum = wtbl_num_use_for_sta + wtbl_num_use_for_ucast;
	
	pHdCfg->HwResourceCfg.WtblCfg.MaxUcastEntryNum = MaxUcastEntryNum;
	pHdCfg->HwResourceCfg.WtblCfg.MinMcastWcid = MAX_LEN_OF_MAC_TABLE-wtbl_num_resv_for_mcast;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    ("%s: MaxStaNum:%d, BssidNum:%d, WdsNum:%d, ApcliNum:%d,"
        " MaxNumChipRept:%d,"
        " MinMcastWcid:%d\n",
                        __FUNCTION__,
                        wtbl_num_use_for_sta,
                        BssidNum,
                        WdsNum,
                        ApcliNum,
                        MaxNumChipRept,
                        pHdCfg->HwResourceCfg.WtblCfg.MinMcastWcid));

	return MaxUcastEntryNum;
}

/*
*
*/
UCHAR WtcAcquireGroupKeyWcid(HD_CFG *pHdCfg,HD_DEV_OBJ *pObj)
{
	UCHAR wcid;
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg=&pResource->WtblCfg;
	
	wcid = wtc_acquire_groupkey_wcid(pWtblCfg,pObj);

	/*try again*/
	if(wcid == INVAILD_WCID){
		/*try again and wait complete*/
		wtc_mcast_wait_complete(pWtblCfg);
		return wtc_acquire_groupkey_wcid(pWtblCfg,pObj);
	}
	return wcid;
}

/*
*
*/
UCHAR WtcReleaseGroupKeyWcid(HD_CFG *pHdCfg,HD_DEV_OBJ *pObj, UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg=&pResource->WtblCfg;
    WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR ReleaseWcid = INVAILD_WCID;

    if(idx >= MAX_LEN_OF_MAC_TABLE)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                        ("%s: idx:%d > MAX_LEN_OF_MAC_TABLE\n",__FUNCTION__, idx));
        return idx;
    }

    NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
    pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

    if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s: try to release non-occupied idx:%d, something wrong?\n",
                        __FUNCTION__, idx));
		ReleaseWcid = idx;
    }
    else
    {
        if(pWtblIdxRec->State == WTBL_STATE_SW_OCCUPIED)
        {
            os_zero_mem(pWtblIdxRec, sizeof(WTBL_IDX_PARAMETER));
            /*make sure entry is cleared to usable one.*/
            pWtblIdxRec->State = WTBL_STATE_NONE_OCCUPIED;
        }else
        {
            pWtblIdxRec->State = WTBL_STATE_WAIT_RELEASE_FOR_HW;
        }
    }
    NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	
	return ReleaseWcid;
}


/*
*
*/
UCHAR WtcGetWcidLinkType(HD_CFG *pHdCfg,UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg=&pResource->WtblCfg;
    WTBL_IDX_PARAMETER *pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

	return pWtblIdxRec->LinkToWdevType;
}



/*
*
*/
UCHAR WtcGetMaxStaNum(HD_CFG *pHdCfg)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg=&pResource->WtblCfg;
    if (pWtblCfg->MaxUcastEntryNum > MAX_LEN_OF_MAC_TABLE){
        
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
        ("%s: MaxUcastEntryNum=%d >= MAX_LEN_OF_MAC_TABLE(%d)\n",
        __FUNCTION__, pWtblCfg->MaxUcastEntryNum, MAX_LEN_OF_MAC_TABLE));
        return MAX_NUMBER_OF_MAC;
    }else{
    
	    return pWtblCfg->MaxUcastEntryNum;
    }
}



/*
*
*/
UCHAR WtcAcquireUcastWcid(HD_CFG *pHdCfg, HD_DEV_OBJ *pObj)
{
	UCHAR FirstWcid = 1;
	UCHAR i;
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;

	if (pObj == NULL || pResource == NULL || pWtblCfg == NULL)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					   ("%s: unexpected NULL please check!!\n",__FUNCTION__));
		return INVAILD_WCID;
	}

    NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

	/* skip entry#0 so that "entry index == AID" for fast lookup*/
	for (i = FirstWcid; i < pWtblCfg->MaxUcastEntryNum ; i++)   
	{
		/* sanity check to avoid out of bound with pAd->MacTab.Content */
		if (i >= MAX_LEN_OF_MAC_TABLE)
			continue;
		
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];
		if (pWtblIdxRec == NULL)
		{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					   ("%s: unexpected NULL please check!!\n",__FUNCTION__));
			return INVAILD_WCID;
		}
		
		if(pWtblIdxRec->State!=WTBL_STATE_NONE_OCCUPIED)
			continue;

		pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
		pWtblIdxRec->WtblIdx = i;
		/*TODO: Carter, check flow when calling this function, OmacIdx might be erroed.*/
		pWtblIdxRec->LinkToOmacIdx = pObj->OmacIdx;
		pWtblIdxRec->LinkToWdevType = pObj->Type;
		
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return i;
	}
	
    NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	return INVAILD_WCID;
}


/*
*
*/
UCHAR WtcReleaseUcastWcid(HD_CFG *pHdCfg, HD_DEV_OBJ *pObj, UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR ReleaseWcid=INVAILD_WCID;

	if(idx >= MAX_LEN_OF_MAC_TABLE)
	{
	   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					   ("%s: idx:%d > MAX_LEN_OF_MAC_TABLE\n",__FUNCTION__, idx));
	   return idx;
	}
	
	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

	if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED)
	{
	   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				   ("%s: try to release non-occupied idx:%d, something wrong?\n",
					   __FUNCTION__, idx));
	   ReleaseWcid = idx;
	}
	else
	{
	    if(pWtblIdxRec->State == WTBL_STATE_SW_OCCUPIED)
        {   
		    os_zero_mem(pWtblIdxRec, sizeof(WTBL_IDX_PARAMETER));
            /*make sure entry is cleared to usable one.*/
		    pWtblIdxRec->State = WTBL_STATE_NONE_OCCUPIED;
        }else
        {
            pWtblIdxRec->State = WTBL_STATE_WAIT_RELEASE_FOR_HW;
        }
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	
	return ReleaseWcid;
}


/*
*
*/
UCHAR WtcHwAcquireWcid(HD_CFG *pHdCfg,UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

    if(pWtblIdxRec->State!=WTBL_STATE_SW_OCCUPIED && pWtblIdxRec->State!=WTBL_STATE_HW_OCCUPIED)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
        ("%s: try to acquire non-occupied idx:%d, something wrong?\n",
        __FUNCTION__, idx));
        
        NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
        return idx;
    }

    pWtblIdxRec->State = WTBL_STATE_HW_OCCUPIED;
    
	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	
	return idx;
}


/*
*
*/
UCHAR WtcHwReleaseWcid(HD_CFG *pHdCfg,UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;

	if(idx >= MAX_LEN_OF_MAC_TABLE){
		return idx;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

    if(!pWtblIdxRec || pWtblIdxRec->State!=WTBL_STATE_WAIT_RELEASE_FOR_HW)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
        ("%s: try to release non hw occupied idx:%d, something wrong?\n",
        __FUNCTION__, idx));
        
        NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
        return idx;
    }

    os_zero_mem(pWtblIdxRec,sizeof(WTBL_IDX_PARAMETER));
    pWtblIdxRec->State = WTBL_STATE_NONE_OCCUPIED;

	/*idx is owned by mcast, check is wait need to complete*/
	if((idx >= pWtblCfg->MinMcastWcid) && pWtblCfg->mcast_wait){
		wtc_mcast_complete(pWtblCfg);
	}
	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	return idx;
}



/*
*
*/
VOID WtcRecDump(HD_CFG *pHdCfg)
{
	HD_RESOURCE_CFG *pResource = &pHdCfg->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR i;


   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			   ("\tWtblRecDump, Max Ucast is %d\n",pWtblCfg->MaxUcastEntryNum));
	for(i=0;i<MAX_LEN_OF_MAC_TABLE;i++)
	{
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];
		if(pWtblIdxRec->State)
		{
		   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			   ("\tIdx:%d,State:%d,Omac:%d,Type:%d,Wcid:%d\n",i,pWtblIdxRec->State,pWtblIdxRec->LinkToOmacIdx,
			   pWtblIdxRec->LinkToWdevType,pWtblIdxRec->WtblIdx));
		}
	}
	
}

