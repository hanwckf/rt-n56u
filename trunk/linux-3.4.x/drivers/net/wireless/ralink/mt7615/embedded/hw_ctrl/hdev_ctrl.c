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
	hdev_ctrl.c
*/
#include	"rt_config.h"
#include "hdev/hdev.h"

/*
* local function
*/
#ifdef DBDC_MODE
static VOID hcGetBandTypeName(UCHAR Type,UCHAR* Str)
{
	switch(Type){
	case DBDC_TYPE_WMM:
		sprintf(Str,"%s","WMM");
	break;
	case DBDC_TYPE_MGMT:
		sprintf(Str,"%s","MGMT");
	break;
	case DBDC_TYPE_BSS:
		sprintf(Str,"%s","BSS");
	break;
	case DBDC_TYPE_MBSS:
		sprintf(Str,"%s","MBSS");
	break;
	case DBDC_TYPE_REPEATER:
		sprintf(Str,"%s","REPEATER");
	break;
	case DBDC_TYPE_MU:
		sprintf(Str,"%s","MU");
	break;
	case DBDC_TYPE_BF:
		sprintf(Str,"%s","BF");
	break;
	case DBDC_TYPE_PTA:
		sprintf(Str,"%s","PTA");
	break;
	}

}
#endif

/*
 *
*/
/*Only this function can use pAd*/
INT32 HcCfgInit(RTMP_ADAPTER *pAd)
{
	HD_CFG  *pHdevCfg = NULL;
	UINT32  ret;

	ret  =  os_alloc_mem(NULL,(UCHAR**)&pHdevCfg,sizeof(HD_CFG));

	if(pHdevCfg==NULL)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): Allocate Hardware device Configure  fail!!\n",__FUNCTION__));

		return -1;
	}

	os_zero_mem(pHdevCfg,sizeof(HD_CFG));

	pHdevCfg->priv  = (VOID*)pAd;

	/*temporal will replace pAd->ChipCap,ChipOp*/
	os_move_mem(&pHdevCfg->chipCap,&pAd->chipCap,sizeof(RTMP_CHIP_CAP));
	os_move_mem(&pHdevCfg->ChipOps,&pAd->chipOps,sizeof(RTMP_CHIP_OP));

	/*initial hardware resource*/
	HdevHwResourceInit(pHdevCfg);

	pAd->pHdevCfg = (VOID*)pHdevCfg;
	return 0;
}


/*
 *
*/
VOID HcCfgExit(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdevCfg = (HD_CFG*) pAd->pHdevCfg;

	/*exist hw resource*/
	HdevHwResourceExit(pHdevCfg);

	/*exist hdevcfg*/
	pAd->pHdevCfg = NULL;
	os_free_mem(pHdevCfg);
}


/*
 *
*/
VOID HcDevExit(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdevCfg = (HD_CFG*) pAd->pHdevCfg;
	UCHAR i;
	HD_RESOURCE_CFG *pHwResourceCfg = &pHdevCfg->HwResourceCfg;

	for(i=0;i< pHwResourceCfg->concurrent_bands;i++)
	{
		HdevExit(pHdevCfg,i);
	}

}



/*
*
*/
VOID HcCfgShow(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HdevCfgShow(pHdCfg);
}


/*
*
*/
INT32 HcAcquireRadioForWdev(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	INT32 ret=0;
	HD_CFG *pHdCfg = (HD_CFG *)pAd->pHdevCfg;
	HD_DEV *pHdev=NULL;
	HD_DEV_OBJ *pObj = NULL;
	UCHAR Channel;

	pObj = RcAcquiredBandForObj(pHdCfg,wdev->wdev_idx,wdev->PhyMode,wdev->channel,wdev->wdev_type);
	if(pObj == NULL)
	{
	    return 1;
	}
	pHdev = pObj->pHdev;

	Channel = RcGetChannel(pHdev);
	if(!wmode_band_equal(wdev->PhyMode,RcGetPhyMode(pHdev)))
	{
		wdev->PhyMode = RcGetPhyMode(pHdev);
		wdev->channel = Channel;
	}
	else
	{
		if(wdev->channel != Channel && wdev->channel!=0)
		{
			if (((wdev->channel <= 14) && (WMODE_5G_ONLY(wdev->PhyMode))) ||
				((wdev->channel > 14) && (WMODE_2G_ONLY(wdev->PhyMode))))
			{
				/* Conflicted channel/phymode setting, align channel to Hdev */
				wdev->channel = Channel;
			}
			else 
			{	
				/* for interface down then change channe on this band should update it,
				 * channel=0 is auto channel selection should not remark it
				 */
				RcUpdateChannel(pHdev,wdev->channel);
			}
		}
	}
	wdev->pHObj = (VOID*)pObj;
	/*temporal set, will be repaced by HcGetOmacIdx*/
	wdev->OmacIdx = pObj->OmacIdx;
	/*re-init operation*/	
	wlan_operate_init(wdev);

	return ret;
}


/*
*
*/
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	INT32 ret=0;
	HD_CFG *pHdCfg = (HD_CFG *)pAd->pHdevCfg;
    HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

    OS_SPIN_LOCK(&pObj->RefCntLock);
    if (pObj->RefCnt > 0)
    {
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s(): there are other link reference the Obj\n", __FUNCTION__));
        OS_SPIN_UNLOCK(&pObj->RefCntLock);
        return ret;
    }

    OS_SPIN_UNLOCK(&pObj->RefCntLock);
	RcReleaseBandForObj(pHdCfg,(HD_DEV_OBJ*)wdev->pHObj);

	wdev->pHObj = NULL;

	return ret;
}



/*
*
*/
UCHAR HcGetBandByWdev(struct wifi_dev *wdev)
{
	UCHAR BandIdx=0 ;
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	if(!pObj)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(), wdev_idx %d pObj = NULL !!!\n", 
			__FUNCTION__, wdev->wdev_idx));
		return 0;
	}

	BandIdx = RcGetBandIdx(pObj->pHdev);
	if (BandIdx >= BAND_NUM_MAX)
		BandIdx = 0;
	
	return BandIdx;
}


/*
*
*/
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS CurStat)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	if(!pObj) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(), wdev_idx %d pObj = NULL !!!\n", 
			__FUNCTION__, wdev->wdev_idx));
		return;
	}

	RcSetRadioCurStat(pObj->pHdev, CurStat);
}

/*
*
*/
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *pAd, UCHAR Channel, PHY_STATUS CurStat)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV *pHdev = NULL;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(), Channel %d pHdev = NULL!!!\n", 
			__FUNCTION__, Channel));
		return;
	}

	RcSetRadioCurStat(pHdev, CurStat);
}

/*
*
*/
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
    HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
    HD_RESOURCE_CFG *pHwResourceCfg =&pHdCfg->HwResourceCfg;
    HD_DEV *pHdev = NULL;
    UCHAR i;

    for(i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
        pHdev = &pHdCfg->Hdev[i];
        pHdev->pRadioCtrl->CurStat = PHY_RADIOOFF;
    }
}


/*
*
*/
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd)
{
    HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
    HD_RESOURCE_CFG *pHwResourceCfg =&pHdCfg->HwResourceCfg;
    HD_DEV *pHdev = NULL;
    UCHAR i;

    for(i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
        pHdev = &pHdCfg->Hdev[i];
        pHdev->pRadioCtrl->CurStat = PHY_INUSE;
    }
}

/*
*
*/
BOOLEAN IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	if(!pObj)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(), wdev_idx %d pObj = NULL, return TRUE !!!\n", 
			__FUNCTION__, wdev->wdev_idx));
		return TRUE;
	}

	if (RcGetRadioCurStat(pObj->pHdev) == PHY_RADIOOFF)
	    return TRUE;
	else
	    return FALSE;
}


/*
*
*/
BOOLEAN IsHcRadioCurStatOffByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV *pHdev = NULL;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(), Channel %d pHdev = NULL, return TRUE !!!\n", 
			__FUNCTION__, Channel));
		return TRUE;
	}

    if (RcGetRadioCurStat(pHdev) == PHY_RADIOOFF)
        return TRUE;
    else
        return FALSE;
}


/*
*
*/
BOOLEAN IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
    HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
    HD_RESOURCE_CFG *pHwResourceCfg =&pHdCfg->HwResourceCfg;
    HD_DEV *pHdev = NULL;
    UCHAR i;
    BOOLEAN AllSupportedBandsRadioOff = TRUE;

    for(i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
        pHdev = &pHdCfg->Hdev[i];
        if ((pHdev->pRadioCtrl->CurStat == PHY_INUSE) && (pHdev->pRadioCtrl->CurStat != PHY_RADIOOFF)) {
            AllSupportedBandsRadioOff = FALSE;
            break;
        }
    }

    return AllSupportedBandsRadioOff;
}

#ifdef GREENAP_SUPPORT
/*
*
*/
VOID HcSetGreenAPActiveByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bGreenAPActive)
{
    HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
    HD_DEV *pHdev = NULL;

    if (!pHdCfg) 
        return;

    pHdev = &pHdCfg->Hdev[BandIdx];

    if (!pHdev)
        return;

    pHdev->pRadioCtrl->bGreenAPActive = bGreenAPActive;
}


/*
*
*/
BOOLEAN IsHcGreenAPActiveByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
    HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
    HD_DEV *pHdev = NULL;

    if (!pHdCfg) 
        return FALSE;

    pHdev = &pHdCfg->Hdev[BandIdx];

    if (!pHdev)
        return FALSE;
    
    return pHdev->pRadioCtrl->bGreenAPActive;
}


/*
*
*/
BOOLEAN IsHcGreenAPActiveByWdev(struct wifi_dev *wdev)
{
    HD_DEV_OBJ *pObj = NULL;
    HD_DEV *pHdev = NULL;

    pObj = (HD_DEV_OBJ*)wdev->pHObj;

    if(!pObj)
        return FALSE;

    pHdev = pObj->pHdev;

    if (!pHdev)
        return FALSE;

    return pHdev->pRadioCtrl->bGreenAPActive;
}
#endif /* GREENAP_SUPPORT */

/*
*
*/
UCHAR HcGetChannelByBf(RTMP_ADAPTER *pAd)
{
	HD_DEV *pHdev = RcGetBandIdxByBf((HD_CFG*)pAd->pHdevCfg);
	if(pHdev == NULL)
	{
	    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
	    	    ("%s(): pHdev is NULL!\n", __FUNCTION__));
	    return 0;
	}

	return pHdev->pRadioCtrl->Channel;
}


/*
*
*/
BOOLEAN HcIsBfCapSupport(struct wifi_dev *wdev)
{
	if(!wdev->pHObj)
		return FALSE;

	return RcIsBfCapSupport((HD_DEV_OBJ*)wdev->pHObj);
}



#ifdef MAC_REPEATER_SUPPORT
/*
*
*/
INT32 HcAddRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx)
{
	INT32 ret=0;
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

    if(!pObj)
    {
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s(): wdev:%x has no pHObj!\n", __FUNCTION__, wdev->OmacIdx));
        return NDIS_STATUS_FAILURE;
    }

	/*Acquire Repeater OMACIdx*/
	OcAddRepeaterEntry(pObj,ReptIdx);

    RcUpdateRepeaterEntry(pObj->pHdev,ReptIdx);

	return ret;
}


/*
*
*/
INT32 HcDelRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx)
{
	INT32 ret=0;
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

    if(!pObj)
    {
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s(): wdev:%x has no pHObj!\n", __FUNCTION__, wdev->OmacIdx));
        return NDIS_STATUS_FAILURE;
    }

	/*Acquire Repeater OMACIdx*/
	OcDelRepeaterEntry(pObj,ReptIdx);

	return ret;
}


/*
*
*/
UCHAR HcGetRepeaterOmac(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	HD_REPT_ENRTY *pHReptEntry = NULL;
	UCHAR ReptOmacIdx = 0xff;

    pReptEntry = RTMPLookupRepeaterCliEntry(
                            pAd,
                            FALSE,
                            pEntry->ReptCliAddr,
                            TRUE);

	if(pReptEntry)
	{
		pHReptEntry = OcGetRepeaterEntry(pReptEntry->wdev->pHObj,pReptEntry->MatchLinkIdx);

		if(pHReptEntry)
		{
			ReptOmacIdx = pHReptEntry->ReptOmacIdx;
		}
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			   ("%s(): Get ReptOmacIdx: %d!\n", __FUNCTION__,ReptOmacIdx));

	return ReptOmacIdx;
}
#endif /*#MAC_REPEATER_SUPPORT*/


/*
*
*/
INT32 HcRadioInit(RTMP_ADAPTER *pAd, UCHAR RfIC,UCHAR DbdcMode)
{
	INT32 ret=0;
	HD_CFG *pHdCfg = (HD_CFG *)pAd->pHdevCfg;

	RcRadioInit(pHdCfg,RfIC,DbdcMode);

	return ret;
}


/*
*
*/
INT32 HcUpdatePhyMode(RTMP_ADAPTER *pAd, UCHAR PhyMode)
{
	INT32 ret=0;
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV_OBJ *pObj ;
	struct wifi_dev *wdev= NULL;
	UCHAR ext_cha;

	pHdev = RcGetHdevByPhyMode(pHdCfg,PhyMode);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Update PhyMode %d faild, not support this RF\n",
		__FUNCTION__,PhyMode));
		return -1;
	}

	RcUpdatePhyMode(pHdev,PhyMode);

	/*update all of wdev*/
	DlListForEach(pObj, &pHdev->DevObjList, struct _HD_DEV_OBJ, list) {

		wdev = pAd->wdev_list[pObj->Idx];
		wdev->channel = RcGetChannel(pHdev);
		ext_cha = RcGetExtCha(pHdev);
		wlan_operate_set_ext_cha(wdev,ext_cha);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Update PhyMode for all wdev for this band PhyMode:%d,Channel=%d\n",
			__FUNCTION__,wdev->PhyMode,wdev->channel));
	}


	return ret;
}

/*
* Used for initial flow, will clear original phy mdoe and then set phymode only
*/
INT32 HcSetPhyMode(RTMP_ADAPTER *pAd, UCHAR PhyMode)
{
	INT32 ret=0;
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;

	pHdev = RcGetHdevByPhyMode(pHdCfg,PhyMode);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Update PhyMode %d faild, not support this RF\n",
		__FUNCTION__,PhyMode));
		return -1;
	}
	pHdev->pRadioCtrl->PhyMode = PhyMode;
	return ret;
}



/*
*
*/
INT32 HcUpdateChannel(RTMP_ADAPTER *pAd,UCHAR Channel)
{
	INT32 ret=0;
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV_OBJ *pObj ;
	struct wifi_dev *wdev;
	UCHAR ext_cha = 0;
	UCHAR oldCh = 0, idx = 0;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Update Channel %d faild, not support this RF\n",
		__FUNCTION__,Channel));
		return -1;
	}

	ret  = RcUpdateChannel(pHdev,Channel);

	if(ret < 0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Update Channel %d faild, not support this RF\n",
		__FUNCTION__,Channel));
		return -1;
	}


    pObj = DlListFirst(&pHdev->DevObjList, struct _HD_DEV_OBJ, list);
    if (pObj != NULL)
    {
    	wdev = pAd->wdev_list[pObj->Idx];
    	oldCh = wdev->channel;
        
    	/* force sync the HdevInfo to the wdev with inactive state and not in DevObjList */
    	for (idx=0; idx<WDEV_NUM_MAX; idx++)	
    	{
    		wdev = pAd->wdev_list[idx];
    		if (wdev && (wdev->channel == oldCh))
    		{
    			wdev->PhyMode = RcGetPhyMode(pHdev);
                wdev->channel = RcGetChannel(pHdev);
                ext_cha = RcGetExtCha(pHdev);
                wlan_operate_set_ext_cha(wdev,ext_cha);

                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Update PhyMode for wdev%d for this band PhyMode:%d,Channel=%d\n",
    	            __FUNCTION__, wdev->wdev_idx, wdev->PhyMode, wdev->channel));
    	
    		}
    	}
    }
    
	return ret;
}


/*
*
*/
INT32 HcUpdateRadio(RTMP_ADAPTER *pAd,UCHAR bw,UCHAR central_ch1,UCHAR control_ch2)
{
	INT32 ret=0;
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	struct wifi_dev *wdev;
	HD_DEV_OBJ *pObj;

	pHdev = RcGetHdevByChannel(pHdCfg,central_ch1);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Update Channel %d faild, not support this RF\n",
		__FUNCTION__,central_ch1));
		return -1;
	}

    /*Update Central Ch*/
	ret  = RcUpdateRadio(pHdev,bw,central_ch1,control_ch2);

	DlListForEach(pObj, &pHdev->DevObjList, struct _HD_DEV_OBJ, list) {
        wdev = pAd->wdev_list[pObj->Idx];
     	wdev->CentralChannel = central_ch1;
    }

	return ret;
}

/*
*
*/
INT32 HcUpdateExtCha(RTMP_ADAPTER *pAd,UCHAR Channel,UCHAR ExtCha)
{
	INT32 ret=0;
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);
	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Get Hdev by Channel %d faild, not support this RF\n",
		__FUNCTION__,Channel));
		return -1;
	}

    /*Update ExtCha to radio*/
	ret  = RcUpdateExtCha(pHdev,ExtCha);

	return ret;
}

/*
*
*/
UCHAR HcGetExtCha(RTMP_ADAPTER *pAd,UCHAR Channel)
{
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Get Hdev by Channel %d faild, not support this RF\n",
		__FUNCTION__,Channel));
		return 0;
	}
	return RcGetExtCha(pHdev);
}

/*
*
*/
INT32 HcUpdateCsaCntByChannel(RTMP_ADAPTER *pAd,UCHAR Channel)
{
	INT32 ret = 0;
	HD_DEV *pHdev = NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV_OBJ *pObj;
	struct wifi_dev *wdev;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): Update Channel %d faild, not support this RF\n",
                 __FUNCTION__,Channel));
		return -1;
	}

	DlListForEach(pObj, &pHdev->DevObjList, struct _HD_DEV_OBJ, list) {
        wdev = pAd->wdev_list[pObj->Idx];
        if (pAd->Dot11_H.RDMode != RD_SILENCE_MODE)
        {
            pAd->Dot11_H.wdev_count++;
            wdev->csa_count = pAd->Dot11_H.CSPeriod;
            /* TODO: Hugo 0728 */
            UpdateBeaconHandler(pAd, wdev, IE_CHANGE);
        }
    }

    return ret;
}
#ifdef DBDC_MODE
/*
*
*/
VOID HcShowBandInfo(RTMP_ADAPTER *pAd)
{
	UINT32 i;
	BCTRL_INFO_T BctrlInfo;
	BCTRL_ENTRY_T *pEntry = NULL;
	CHAR TempStr[16]="";
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
#ifdef GREENAP_SUPPORT 
    struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */

	os_zero_mem(&BctrlInfo,sizeof(BCTRL_INFO_T));

	AsicGetDbdcCtrl(pAd,&BctrlInfo);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDbdcEnable: %d\n", BctrlInfo.DBDCEnable));

	HdevCfgShow(pHdCfg);

	for(i=0;i<BctrlInfo.TotalNum;i++)
	{
		pEntry = &BctrlInfo.BctrlEntries[i];
		hcGetBandTypeName(pEntry->Type,TempStr);
		if(DBDC_TYPE_MBSS != pEntry->Type)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t(%s,%d): Band %d \n",TempStr,pEntry->Index,pEntry->BandIdx));
		}else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t(%s,0-%d): Band %d \n",TempStr,pEntry->Index+1,pEntry->BandIdx));
		}
	}

	WcShowEdca(pHdCfg);

#ifdef GREENAP_SUPPORT
	greenap_show(pAd, greenap);
#endif /* GREENAP_SUPPORT */	

}

#endif



/*
*
*/
VOID HcAcquiredEdca(RTMP_ADAPTER *pAd,struct wifi_dev *wdev,EDCA_PARM *pEdca)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
	if(pObj)
	{
		WcAcquiredEdca(pObj,pEdca);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s(): can't find pHdev for wdev=%d \n",__FUNCTION__,wdev->wdev_idx));
	}
}



/*
*
*/
VOID HcReleaseEdca(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	if(pObj)
	{
		WcReleaseEdca(pObj);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s(): can't find pHObj for wdev=%d \n",__FUNCTION__,wdev->wdev_idx));
	}
}


/*
*
*/
VOID HcSetEdca(struct wifi_dev *wdev)
{
    HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
	if(pObj){
		WcSetEdca(pObj);
	}else{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s(): can't find pHdev for wdev=%d \n",__FUNCTION__,wdev->wdev_idx));
	}
}


/*
*
*/
UCHAR HcGetOmacIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	if(pObj)
	{
		return pObj->OmacIdx;
	}

	return 0xff;
}



/*
*  Need refine
*/


/*
* Only temporal usage, should remove when cmm_asic_xxx.c is not apply pAd
*/

UCHAR  HcGetChannelByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
		{
			return pHwResource->PhyCtrl[i].RadioCtrl.Channel;
		}
	}
	return 0;
}


/*
*
*/
UCHAR  HcGetCentralChByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
		{
			return pHwResource->PhyCtrl[i].RadioCtrl.CentralCh;
		}
	}
	return 0;
}


/*
*
*/
UCHAR HcGetPhyModeByRf(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
		{
			return pHwResource->PhyCtrl[i].RadioCtrl.PhyMode;
		}
	}
	return 0;
}

/*
*
*/
CHAR HcGetBwByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
		{
			return pHwResource->PhyCtrl[i].RadioCtrl.Bw;
		}
	}
	return -1;
}

/*
* for Single Band Usage
*/
UCHAR  HcGetRadioChannel(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	return pHwResource->PhyCtrl[0].RadioCtrl.Channel;
}


/*
*
*/
UCHAR HcGetRadioPhyMode(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	return pHwResource->PhyCtrl[0].RadioCtrl.PhyMode;

}


/*
*
*/
BOOLEAN  HcIsRfSupport(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
		{
			return TRUE;
		}
	}
	return FALSE;
}


/*
*
*/
BOOLEAN  HcIsRfRun(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	HD_DEV *pHdev;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		pHdev = &pHdCfg->Hdev[i];
		if(WMODE_CAP_2G(pHdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_24GHZ))
			return TRUE;
		else
		if(WMODE_CAP_5G(pHdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_5GHZ))
			return TRUE;
	}
	return FALSE;
}



#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
/*
*
*/
QLOAD_CTRL* HcGetQloadCtrlByRf(RTMP_ADAPTER *pAd, UINT32 RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
		{
			return &pHwResource->PhyCtrl[i].QloadCtrl;
		}
	}
	return 0;
}


/*
*
*/
QLOAD_CTRL* HcGetQloadCtrl(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;

	return &pHwResource->PhyCtrl[0].QloadCtrl;
}
#endif /*AP_QLOAD_SUPPORT*/


/*
*
*/
AUTO_CH_CTRL* HcGetAutoChCtrlByRf(RTMP_ADAPTER *pAd, UINT32 RfIC)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;
	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
		{
			return &pHwResource->PhyCtrl[i].AutoChCtrl;
		}
	}
	return 0;
}


/*
*
*/
AUTO_CH_CTRL* HcGetAutoChCtrl(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_RESOURCE_CFG *pHwResource =  &pHdCfg->HwResourceCfg;

	return &pHwResource->PhyCtrl[0].AutoChCtrl;
}
#endif /*CONFIG_AP_SUPPORT*/



/*
*
*/
VOID HcBbpSetBwByChannel(RTMP_ADAPTER *pAd,UCHAR Bw, UCHAR Channel)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV *pHdev = RcGetHdevByChannel(pHdCfg,Channel);
	UCHAR BandIdx;
	UCHAR phy_bw;


	phy_bw = decide_phy_bw_by_channel(pAd,Channel);

	if(pHdev)
	{
		BandIdx = RcGetBandIdx(pHdev);
		/*Update BW to MAC/BBP*/
		AsicSetBW(pAd,phy_bw,BandIdx);
	}
	else
	{
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s(): can't find pHdev for Channel=%d\n",
                    __FUNCTION__,Channel));
	}

}

UCHAR HcGetBw(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
    HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
    HD_DEV *pHdev;
    UCHAR BW;

    if(pObj)
    {
        pHdev = pObj->pHdev;
    }
    else
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    ("%s(): can't find pHdev for wdev=%d\n",
                            __FUNCTION__,wdev->wdev_idx));
        return 0xff;
    }

    BW = RcGetBw(pHdev);
    return BW;
}

/*
*
*/
UCHAR HcGetRadioRfIC(RTMP_ADAPTER *pAd)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;

	return pHdCfg->HwResourceCfg.PhyCtrl[0].rf_band_cap;
}


/*
*
*/
UINT32 HcGetBmcQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
	return RcGetBmcQueueIdx(pObj);
}


/*
*
*/
UINT32 HcGetMgmtQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj= (HD_DEV_OBJ*)wdev->pHObj;
	return RcGetMgmtQueueIdx(pObj);
}

/*
*
*/
UINT32 HcGetBcnQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	return RcGetBcnQueueIdx(pObj);
}


/*
*
*/
UINT32 HcGetTxRingIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
	return RcGetTxRingIdx(pObj);
}



/*
*
*/
UINT32 HcGetWmmIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	return RcGetWmmIdx(pObj);
}


/*
*
*/
UCHAR HcGetBandByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV *pHdev = NULL;
	UCHAR BandIdx;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
		return 0;

	BandIdx = RcGetBandIdx(pHdev);
	return BandIdx;
}


/*
*
*/
EDCA_PARM *HcGetEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	EDCA_PARM *pEdca = NULL;
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;

    if (pObj == NULL)
        return NULL;

	pEdca = WcGetWmmByIdx(pHdCfg,pObj->WmmIdx);

	return pEdca;
}


/*
*
*/
VOID HcCrossChannelCheck(RTMP_ADAPTER *pAd,struct wifi_dev *wdev,UCHAR Channel)
{
	UCHAR PhyMode = wdev->PhyMode;
	UCHAR WChannel = wdev->channel;
	/*check channel is belong to differet band*/
	if(Channel > 14 && WChannel > 14)
		return ;
	if(Channel <=14 && WChannel <=14)
		return;
			/*is mixed mode, change default channel and */
			if(!WMODE_5G_ONLY(PhyMode)	|| !WMODE_2G_ONLY(PhyMode))
			{

				/*update wdev channel to new band*/
				wdev->channel = Channel;
				/*need change to other band*/
				HcAcquireRadioForWdev(pAd,wdev);
			}

	return ;
}


/*
 * Description:
 *
 * the function will check all enabled function,
 * check the bssid num is defined,
 *
 * preserve the group key wtbl num will be used.
 * then decide the max station number could be used.
 */
UCHAR HcGetMaxStaNum(RTMP_ADAPTER *pAd)
{
    UCHAR MaxStaNum = WtcGetMaxStaNum(pAd->pHdevCfg);
    if (MaxStaNum > MAX_LEN_OF_MAC_TABLE)
	    MaxStaNum = MAX_NUMBER_OF_MAC;
    return MaxStaNum;
}


UCHAR HcSetMaxStaNum(RTMP_ADAPTER *pAd)
{

	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	UCHAR BssidNum=0,MSTANum=0;

#ifdef CONFIG_AP_SUPPORT
 	BssidNum = pAd->ApCfg.BssidNum;
#endif /*CONFIG_AP_SUPPORT*/

    return WtcSetMaxStaNum(pHdCfg,BssidNum,MSTANum);

}



/*
*
*/
UCHAR HcAcquireGroupKeyWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	wdev->tr_tb_idx = WtcAcquireGroupKeyWcid(pAd->pHdevCfg,pObj);
	return wdev->tr_tb_idx;
}


/*
*
*/
VOID HcReleaseGroupKeyWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx)
{
    HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
	wdev->tr_tb_idx = WtcReleaseGroupKeyWcid(pAd->pHdevCfg,pObj,idx);

}


/*
*
*/
UCHAR HcGetWcidLinkType(RTMP_ADAPTER *pAd, UCHAR Wcid)
{
	return WtcGetWcidLinkType(pAd->pHdevCfg,Wcid);
}


/*
*
*/
UCHAR HcAcquireUcastWcid(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;

	if (wdev->pHObj == NULL || pAd->pHdevCfg == NULL)
	{		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					   ("%s: unexpected NULL please check!!\n",__FUNCTION__));
		dump_stack();
		return INVAILD_WCID;
	}
	

	return WtcAcquireUcastWcid(pAd->pHdevCfg,pObj);
}


/*
*
*/
UCHAR HcReleaseUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx)
{
	HD_DEV_OBJ *pObj = (HD_DEV_OBJ*)wdev->pHObj;
	return WtcReleaseUcastWcid(pAd->pHdevCfg,pObj,idx);
}


/*
*
*/
UCHAR HcHwAcquireWcid(RTMP_ADAPTER *pAd, UCHAR idx)
{
	return WtcHwAcquireWcid(pAd->pHdevCfg,idx);
}


/*
*
*/
UCHAR HcHwReleaseWcid(RTMP_ADAPTER *pAd, UCHAR idx)
{
	return WtcHwReleaseWcid(pAd->pHdevCfg,idx);
}


/*
*
*/
VOID HcWtblRecDump(RTMP_ADAPTER *pAd)
{
	WtcRecDump(pAd->pHdevCfg);
}


/*
*
*/
BOOLEAN HcIsRadioAcq(struct wifi_dev *wdev)
{
	if(wdev->pHObj)
	{
		return TRUE;
	}
	return FALSE;
}


/*
    Description:

    for record Rx Pkt's wlanIdx, TID, Seq.
    it is used for checking if there is the same A2 send to different A1.

    according the record. trigger the scoreboard update.
*/
VOID RxTrackingInit(struct wifi_dev *wdev)
{
    UCHAR j;
    RX_TRACKING_T *pTracking = NULL;
    RX_TA_TID_SEQ_MAPPING *pTaTidSeqMapEntry = NULL;

    pTracking = &wdev->rx_tracking;
    pTaTidSeqMapEntry = &pTracking->LastRxWlanIdx;

    pTracking->TriggerNum = 0;

    pTaTidSeqMapEntry->RxDWlanIdx = 0xff;
    pTaTidSeqMapEntry->MuarIdx = 0xff;
    for (j = 0; j < 8; j++)
    {
        pTaTidSeqMapEntry->TID_SEQ[j] = 0xffff;
    }
    pTaTidSeqMapEntry->LatestTID = 0xff;

    return;
}

VOID TaTidRecAndCmp(struct _RTMP_ADAPTER *pAd, struct _RXD_BASE_STRUCT *rx_base, UINT16 SN)
{
    struct wifi_dev *wdev = NULL;
    RX_TRACKING_T *pTracking = NULL;
    RX_TA_TID_SEQ_MAPPING *pTaTidSeqMapEntry = NULL;
    UCHAR Widx = rx_base->RxD2.RxDWlanIdx;
    UCHAR Tid = rx_base->RxD2.RxDTid;
    UCHAR MuarIdx = rx_base->RxD1.RxDBssidIdx;
    UCHAR BandIdx = 0;
    UINT32 cr_value = 0;
    UINT32 cr_addr_0 = 0;
    UINT32 cr_addr_1 = 0;
    struct _STA_TR_ENTRY *tr_entry = NULL;

    if (Widx > MAX_LEN_OF_MAC_TABLE)
        return;

    wdev = WdevSearchByWcid(pAd, Widx);

    if (wdev == NULL)
        wdev = WdevSearchByOmacIdx(pAd, MuarIdx);

    if (wdev == NULL)
        return;

    if ((wdev->wdev_type != WDEV_TYPE_APCLI) &&
            (wdev->wdev_type != WDEV_TYPE_STA))
    {
        return;
    }

    pTracking = &wdev->rx_tracking;
    pTaTidSeqMapEntry = &pTracking->LastRxWlanIdx;

    if (pTaTidSeqMapEntry->RxDWlanIdx == 0xff)
    {/*first Rx pkt, just record it.*/
        pTaTidSeqMapEntry->RxDWlanIdx = Widx;
        pTaTidSeqMapEntry->MuarIdx = MuarIdx;
        pTaTidSeqMapEntry->TID_SEQ[Tid] = SN;
        pTaTidSeqMapEntry->LatestTID = Tid;
    }
    else
    {
        /* compare*/
        if ((pTaTidSeqMapEntry->MuarIdx == MuarIdx) &&
            (pTaTidSeqMapEntry->RxDWlanIdx != Widx) &&
            pTaTidSeqMapEntry->LatestTID == Tid)
        {
            /*condition match, trigger scoreboard update*/
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                    ("last Widx = %d, muar_idx = %x, last TID = %d\n",
                            pTaTidSeqMapEntry->RxDWlanIdx,
                            pTaTidSeqMapEntry->MuarIdx,
                            pTaTidSeqMapEntry->LatestTID));
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
                    ("new Widx = %d, muar_idx = %x, last TID = %d\n",
                                        Widx,
                                        MuarIdx,
                                        Tid));

            BandIdx = HcGetBandByWdev(wdev);
            if (BandIdx > 0)
            {
#ifdef DBDC_MODE
                cr_addr_0 = BSCR0_BAND_1;
                cr_addr_1 = BSCR1_BAND_1;
#endif
            }

            else
            {
#if defined(MT7615) || defined(MT7622)
                if (IS_MT7615(pAd) || IS_MT7622(pAd))
                {
                    cr_addr_0 = BSCR0_BAND_0;
                    cr_addr_1 = BSCR1_BAND_0;
                }
#else
                if (!IS_MT7615(pAd) && !IS_MT7622(pAd))
                {
                    cr_addr_0 = BSCR0;
                    cr_addr_1 = BSCR1;
                }
#endif
            }
            tr_entry = &pAd->MacTab.tr_entry[Widx];
            cr_value = (tr_entry->Addr[0] |
                        (tr_entry->Addr[1] << 8) |
                        (tr_entry->Addr[2] << 16) |
                        (tr_entry->Addr[3] << 24));
            MAC_IO_WRITE32(pAd, cr_addr_0, cr_value);

            cr_value = (START_RST_BA_SB |
                        RST_BA_SEL(RST_BA_MAC_TID_MATCH) |
                        RST_BA_TID(Tid) |
                        (tr_entry->Addr[4]) |
                        (tr_entry->Addr[5] << 8));

            MAC_IO_WRITE32(pAd, cr_addr_1, cr_value);

            pTracking->TriggerNum++;
        }

        /*update lastest rx information.*/
        pTaTidSeqMapEntry->RxDWlanIdx = Widx;
        pTaTidSeqMapEntry->MuarIdx = MuarIdx;
        pTaTidSeqMapEntry->TID_SEQ[Tid] = SN;
        pTaTidSeqMapEntry->LatestTID = Tid;
    }
}

UCHAR HcGetAmountOfBand(struct _RTMP_ADAPTER *pAd)
{
        HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
        return pHdCfg->HwResourceCfg.concurrent_bands;
}

INT32 HcSuspendMSDUTxByChannel(RTMP_ADAPTER *pAd,UCHAR Channel)
{
	INT32 ret=0;
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV_OBJ *pObj ;
	struct wifi_dev *wdev;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Channel %d faild, not support this RF\n",
		__FUNCTION__,Channel));
		return -1;
	}

	/*update all of wdev*/
	DlListForEach(pObj, &pHdev->DevObjList, struct _HD_DEV_OBJ, list) {
		wdev = pAd->wdev_list[pObj->Idx];
		RTMPSuspendMsduTransmission(pAd, wdev);		
	}

	return ret;
}

INT32 HcUpdateMSDUTxAllowByChannel(RTMP_ADAPTER *pAd,UCHAR Channel)
{
	INT32 ret=0;
	HD_DEV *pHdev=NULL;
	HD_CFG *pHdCfg = (HD_CFG*)pAd->pHdevCfg;
	HD_DEV_OBJ *pObj ;
	struct wifi_dev *wdev;

	pHdev = RcGetHdevByChannel(pHdCfg,Channel);

	if(!pHdev)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Channel %d faild, not support this RF\n",
		__FUNCTION__,Channel));
		return -1;
	}

	/*update all of wdev*/
	DlListForEach(pObj, &pHdev->DevObjList, struct _HD_DEV_OBJ, list) {

		wdev = pAd->wdev_list[pObj->Idx];

		if (wdev->channel == Channel)
			RTMPResumeMsduTransmission(pAd, wdev);
		else
			RTMPSuspendMsduTransmission(pAd, wdev);
	}

	return ret;
}

INT hc_radio_acquire(struct wifi_dev *wdev,UCHAR bw,UCHAR ext_cha)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;
	HD_DEV_OBJ *obj = (HD_DEV_OBJ*)wdev->pHObj;
	HD_DEV *dev;
	UCHAR phy_bw;
	UCHAR phy_extcha;

	if(!obj || !ad )
		return -1;

	dev = obj->pHdev;

	if(!dev)
		return -1;

	phy_bw = RcGetBw(dev);
	phy_extcha = RcGetExtCha(dev);

	if((bw == phy_bw) && (ext_cha == phy_extcha))
		return -1;

	if((bw < phy_bw) && (dev->DevNum > 1))
		return -1;

	return 0;
}

