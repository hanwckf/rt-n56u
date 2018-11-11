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

/*Radio controller*/

/*
 *
*/

/*Local functions*/
static UCHAR rcGetRfByIdx(HD_CFG *pHdCfg,UCHAR DbdcMode,UCHAR BandIdx)
{
	// TODO: Should remove when antenna move to Hdev
#ifdef DBDC_MODE
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)pHdCfg->priv;
	if(pHdCfg->ChipOps.BandGetByIdx && DbdcMode)
	{
		return pHdCfg->ChipOps.BandGetByIdx(pAd,BandIdx);
	}else
	{
		return RFIC_DUAL_BAND;
	}
#endif /*DBDC_MODE*/
	return RFIC_DUAL_BAND;
}


/*Get RfIC Band from EEPORM content*/
static UINT8 rcGetBandSupport(HD_CFG *pHdCfg,UCHAR DbdcMode,UCHAR BandIdx)
{
	// TODO: Should remove when antenna move to Hdev
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)pHdCfg->priv;

	if (BOARD_IS_5G_ONLY(pAd))
	{
		return RFIC_5GHZ;
	}
	else if (BOARD_IS_2G_ONLY(pAd))
	{
		return RFIC_24GHZ;
	}
	else if (RFIC_IS_5G_BAND(pAd))
	{
		return rcGetRfByIdx(pHdCfg,DbdcMode,BandIdx);
	}
	else
	{
		return RFIC_24GHZ;
	}
}

static UCHAR rcGetDefaultChannel(UCHAR PhyMode)
{

	/*priority must the same as Default PhyMode*/
	if(WMODE_CAP_2G(PhyMode))
	{
		return 1;
	}else
	if(WMODE_CAP_5G(PhyMode))
	{
		return 36;
	}
	return 0;
}


static UCHAR rcGetDefaultPhyMode(UCHAR Channel)
{
	/*priority must the same as Default Channel*/
	if(Channel <=14)
	{
		return WMODE_B;
	}else
	if(Channel > 14)
	{
		return WMODE_A;
	}

	return WMODE_B;
}



static HD_DEV* rcGetHdevByRf(HD_CFG *pHdCfg,UCHAR RfType)
{
	INT i;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;


	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(RfType & (pHwResource->PhyCtrl[i].rf_band_cap))
		{
			return &pHdCfg->Hdev[i];
		}
	}
	return NULL;
}

static BOOLEAN rcCheckIsTheSameBand(UCHAR PhyMode,UCHAR Channel)
{
	if(WMODE_CAP_5G(PhyMode) && WMODE_CAP_2G(PhyMode))
	{
		return TRUE;
	}
	else
	if(WMODE_CAP_5G(PhyMode) && Channel  > 14)
	{
		return TRUE;
	}
	else
	if(WMODE_CAP_2G(PhyMode) && Channel <=14)
	{
		return TRUE;
	}
	return FALSE;
}




#ifdef DBDC_MODE
static RADIO_CTRL*  rcGetRadioCtrlByRf(HD_CFG *pHdCfg,UCHAR RfType)
{
	INT i;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;

	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(RfType &  pHwResource->PhyCtrl[i].rf_band_cap)
		{
			return &pHwResource->PhyCtrl[i].RadioCtrl;
		}
	}
	return NULL;
}


static VOID rcFillEntry(BCTRL_ENTRY_T *pEntry,UINT8 Type,UINT8 BandIdx,UINT8 Index)
{
	pEntry->Type = Type;
	pEntry->BandIdx = BandIdx;
	pEntry->Index = Index;
}



static INT32 rcUpdateBandForMBSS(HD_DEV_OBJ *pObj,BCTRL_ENTRY_T *pEntry)
{
	HD_DEV *pHdev = pObj->pHdev;
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;
	UCHAR MbssIdx;
	HD_CFG *pHdCfg = (HD_CFG*)pHdev->priv;

	if(pObj->OmacIdx == 0)
	{
		rcFillEntry(pEntry,DBDC_TYPE_BSS,pRadioCtrl->BandIdx,0);
	}else
	{
		/*pHdCfg->chipCap.ExtMbssOmacStartIdx+1 since 0x10 will control by 0x10*/
		MbssIdx = pObj->OmacIdx - (pHdCfg->chipCap.ExtMbssOmacStartIdx+1);
		rcFillEntry(pEntry,DBDC_TYPE_MBSS,pRadioCtrl->BandIdx,MbssIdx);
	}
	return 0;
}


static INT32 rcUpdateBandForBSS(HD_DEV_OBJ *pObj, BCTRL_ENTRY_T *pEntry)
{
	HD_DEV *pHdev = pObj->pHdev;
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;

	rcFillEntry(pEntry,DBDC_TYPE_BSS,pRadioCtrl->BandIdx,pObj->OmacIdx);
	return 0;
}


static INT32 rcUpdateBandByType(HD_DEV_OBJ *pObj, BCTRL_ENTRY_T *pEntry)
{
	switch(pObj->Type){
	case WDEV_TYPE_AP:
	{
		rcUpdateBandForMBSS(pObj,pEntry);
	}
	break;
	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_GO:
	case WDEV_TYPE_GC:
	case WDEV_TYPE_APCLI:
	{
		rcUpdateBandForBSS(pObj,pEntry);
	}
	break;
	case WDEV_TYPE_WDS:
	case WDEV_TYPE_MESH:
	default:
	{
		// TODO: STAR for DBDC
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Current not support this type of WdevType=%d\n",__FUNCTION__,pObj->Type));
		return -1;
	}
	break;
	}
	return 0;
}



/*Must call after update ownmac*/
static INT32 rcUpdateBandForBFMU(HD_CFG *pHdCfg,BCTRL_INFO_T *pBInfo)
{
	HD_DEV *dev;
	RADIO_CTRL *pRadioCtrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	BCTRL_ENTRY_T *pEntry=NULL;
	UINT32 i;

	/*first choice 5G as the BF/MU band*/
	dev = rcGetHdevByRf(pHdCfg,RFIC_5GHZ);

	/*else 2.4G*/
	if(!dev || (dev->DevNum==0)){
		dev = rcGetHdevByRf(pHdCfg,RFIC_24GHZ);
	}

	/*If MU is not enable & 5G not support , else select first dev as bf band*/
	if(!dev){
		dev = &pHdCfg->Hdev[0];
	}
	
	pRadioCtrl = dev->pRadioCtrl;


	/*If get phyCtrl, set bf to this band*/
	if(pRadioCtrl == NULL)
	{
		return -1;
	}

	/*support MU & enable MU, BF & MU should be 5G only*/
	pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
	rcFillEntry(pEntry,DBDC_TYPE_MU, pRadioCtrl->BandIdx,0);
	pBInfo->TotalNum++;


	for(i=0;i<3;i++)
	{
		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry,DBDC_TYPE_BF, pRadioCtrl->BandIdx,i);
		pBInfo->TotalNum++;
	}

	pRadioCtrl->IsBfBand = TRUE;

	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		if(pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand &&
			(pHwResource->PhyCtrl[i].RadioCtrl.BandIdx!=pRadioCtrl->BandIdx))
		{
			pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand = FALSE;
		}
	}

	return 0;
}


static INT32 rcUpdateBandForRepeater(HD_CFG *pHdCfg,BCTRL_INFO_T *pBInfo)
{
	INT32 i;
	BCTRL_ENTRY_T *pEntry;
	HD_DEV *dev;
	HD_DEV_OBJ *pObj;
	HD_REPT_ENRTY *pReptEntry = NULL,*tmp=NULL;
	if (pHdCfg->HwResourceCfg.concurrent_bands == 2) { /* DBDC mode */
		for(i=0;i< pHdCfg->HwResourceCfg.concurrent_bands;i++) {
			/* search repeater entry from all pObj */
			dev = &pHdCfg->Hdev[i];
			DlListForEach(pObj,&dev->DevObjList,struct _HD_DEV_OBJ,list) {
				DlListForEachSafe(pReptEntry,tmp,&pObj->RepeaterList, struct _HD_REPT_ENRTY, list) {
					pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
					rcFillEntry(pEntry,DBDC_TYPE_REPEATER,dev->pRadioCtrl->BandIdx,pReptEntry->CliIdx);
					pBInfo->TotalNum++;
				}
			}
		}
	} else {
		for(i=0; i<pHdCfg->chipCap.MaxRepeaterNum; i++)
		{
			pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
			/*always bind band 0*/
			rcFillEntry(pEntry,DBDC_TYPE_REPEATER,0,i);
			pBInfo->TotalNum++;
		}
	}
	return 0;
}



static INT32 rcUpdateBandForWMM(HD_CFG *pHdCfg,BCTRL_INFO_T *pBInfo)
{
	INT32 i, WmmNum =WcGetWmmNum(pHdCfg) ;
	EDCA_PARM *pEdca = NULL;
	BCTRL_ENTRY_T *pEntry;

	for( i=0; i<WmmNum; i++ )
	{
		pEdca = WcGetWmmByIdx(pHdCfg,i);
		if(!pEdca->bValid)
		{
			continue;
		}

		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry,DBDC_TYPE_WMM,pEdca->BandIdx,i);
		pBInfo->TotalNum++;
	}

	return 0;
}


static INT32 rcUpdateBandForMGMT(HD_CFG *pHdCfg,BCTRL_INFO_T *pBInfo)
{
	INT32 i;
	BCTRL_ENTRY_T *pEntry;

	for(i=0;i<2;i++)
	{
		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry,DBDC_TYPE_MGMT,i,i);
		pBInfo->TotalNum++;
	}

	return 0;
}


static INT32 rcUpdateBandForPTA(HD_CFG *pHdCfg,BCTRL_INFO_T *pBInfo)
{
	BCTRL_ENTRY_T *pEntry;
	RADIO_CTRL *pRadioCtrl;

	pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
	/*fix to bind band 0 for 2.4G band*/
	if((pRadioCtrl = rcGetRadioCtrlByRf(pHdCfg,RFIC_24GHZ))!=NULL)
	{
		rcFillEntry(pEntry,DBDC_TYPE_PTA,pRadioCtrl->BandIdx,0);
	}else
	{
		rcFillEntry(pEntry,DBDC_TYPE_PTA,0,0);
	}
	pBInfo->TotalNum++;

	return 0;
}

static INT32 rcUpdateBandForOwnMac(HD_CFG *pHdCfg,BCTRL_INFO_T *pBInfo)
{
	INT32 i,ret=0;
	HD_DEV_OBJ *pObj;
	HD_DEV *pHdev;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	BCTRL_ENTRY_T *pEntry=NULL;

	for(i=0; i < pHwResource->concurrent_bands; i++)
	{
		pHdev = &pHdCfg->Hdev[i];

		DlListForEach(pObj,&pHdev->DevObjList,struct _HD_DEV_OBJ,list){
			pEntry= &pBInfo->BctrlEntries[pBInfo->TotalNum];
			rcUpdateBandByType(pObj,pEntry);
			pBInfo->TotalNum++;
		}

	}

	return ret;
}
#endif /*DBDC_MODE*/


/*Export functions*/
/*
*
*/
INT32 RcUpdateBandCtrl(HD_CFG *pHdCfg)
{
#ifdef DBDC_MODE
	INT32 ret=0;
	BCTRL_INFO_T BctrlInfo;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)pHdCfg->priv;
	os_zero_mem(&BctrlInfo,sizeof(BCTRL_INFO_T));

	BctrlInfo.DBDCEnable = pAd->CommonCfg.dbdc_mode ;

	/*if enable dbdc, run band selection algorithm*/
	if(IS_CAP_DBDC(pHdCfg) && BctrlInfo.DBDCEnable)
	{
		/*Since phyctrl  need to update */
		rcUpdateBandForOwnMac(pHdCfg,&BctrlInfo);
		rcUpdateBandForBFMU(pHdCfg,&BctrlInfo);
		rcUpdateBandForWMM(pHdCfg,&BctrlInfo);
		rcUpdateBandForMGMT(pHdCfg,&BctrlInfo);
		rcUpdateBandForPTA(pHdCfg,&BctrlInfo);
		rcUpdateBandForRepeater(pHdCfg,&BctrlInfo);
		/*Since will add one more time, must minus 1*/
		BctrlInfo.TotalNum = (BctrlInfo.TotalNum-1);

		if(BctrlInfo.TotalNum >MAX_BCTRL_ENTRY)
		{
			BctrlInfo.TotalNum = MAX_BCTRL_ENTRY;
		}
	}

	if((ret = AsicSetDbdcCtrl(pAd,&BctrlInfo))!=NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Error for conifgure dbdc, ret = %d !\n",__FUNCTION__,ret));
	}
#endif /*DBDC_MODE*/
	return 0;
}


/*
*
*/
INT32 RcUpdateRepeaterEntry(HD_DEV *pHdev, UINT32 ReptIdx)
{
	INT32 ret=0;
#ifdef DBDC_MODE

	BCTRL_ENTRY_T *pEntry;
	BCTRL_INFO_T BandInfoValue;
	HD_CFG *pHdCfg = (HD_CFG*) pHdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)pHdCfg->priv;

	if(IS_CAP_DBDC(pHdCfg) && pAd->CommonCfg.dbdc_mode)
	{
		os_zero_mem(&BandInfoValue,sizeof(BCTRL_INFO_T));
		BandInfoValue.DBDCEnable = pAd->CommonCfg.dbdc_mode;
		pEntry = &BandInfoValue.BctrlEntries[0];
		/*fix to bind band 0 currently*/

		rcFillEntry(pEntry,DBDC_TYPE_REPEATER,pHdev->pRadioCtrl->BandIdx,ReptIdx);
		BandInfoValue.TotalNum++;

		if((ret = AsicSetDbdcCtrl(pAd,&BandInfoValue))!=NDIS_STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Error for conifgure dbdc, ret = %d !\n",__FUNCTION__,ret));
		}
	}
#endif /*DBDC_MODE*/
	return ret;
}


/*
*
*/
INT32 RcUpdateWmmEntry(HD_DEV *pHdev,HD_DEV_OBJ *pObj, UINT32 WmmIdx)
{
	INT32 ret=0;
#ifdef DBDC_MODE

	BCTRL_ENTRY_T *pEntry;
	BCTRL_INFO_T BandInfoValue;
	HD_CFG *pHdCfg = (HD_CFG*) pHdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)pHdCfg->priv;

	if(pObj && IS_CAP_DBDC(pHdCfg) && pAd->CommonCfg.dbdc_mode)
	{
		os_zero_mem(&BandInfoValue,sizeof(BCTRL_INFO_T));
		BandInfoValue.DBDCEnable = pAd->CommonCfg.dbdc_mode;
		pEntry = &BandInfoValue.BctrlEntries[0];
		/*fix to bind band 0 currently*/
		pObj->WmmIdx = WmmIdx;
		rcFillEntry(pEntry,DBDC_TYPE_WMM,pHdev->pRadioCtrl->BandIdx,WmmIdx);
		BandInfoValue.TotalNum++;

		if((ret = AsicSetDbdcCtrl(pAd,&BandInfoValue))!=NDIS_STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Error for conifgure dbdc, ret = %d !\n",__FUNCTION__,ret));
		}
	}
#endif /*DBDC_MODE*/
	return ret;
}


/*
* Used for DATA path can get
*/
UINT32 RcGetBmcQueueIdx(HD_DEV_OBJ *pObj)
{
#if defined(MT7615) || defined(MT7622)
	HD_DEV *pHdev;

	if(pObj)
	{
		pHdev = pObj->pHdev;
		if(pHdev && pHdev->pRadioCtrl &&pHdev->pRadioCtrl->BandIdx)
			return TxQ_IDX_BMC1;
	}
	return TxQ_IDX_BMC0;
#else
		return Q_IDX_BMC;
#endif
}


/*
*
*/
UINT32 RcGetMgmtQueueIdx(HD_DEV_OBJ *pObj)
{
#if defined(MT7615) || defined(MT7622)
	HD_DEV *pHdev;

	if(pObj)
	{
		pHdev = pObj->pHdev;
		if(pHdev && pHdev->pRadioCtrl && pHdev->pRadioCtrl->BandIdx)
		{
			return TxQ_IDX_ALTX1;
		}
	}
	return TxQ_IDX_ALTX0;
#else
	return Q_IDX_AC4;
#endif
}



/*
*
*/
UINT32 RcGetBcnQueueIdx(HD_DEV_OBJ *pObj)
{
#if defined(MT7615) || defined(MT7622)
	HD_DEV *pHdev;

	if(pObj)
	{
		pHdev = pObj->pHdev;
		if(pHdev && pHdev->pRadioCtrl->BandIdx)
		{
			return TxQ_IDX_BCN1;
		}
	}
	return TxQ_IDX_BCN0;
#else
	return Q_IDX_BCN;
#endif

}


/*
*
*/
UINT32 RcGetTxRingIdx(HD_DEV_OBJ *pObj)
{
	HD_DEV *pHdev;
	if(pObj)
	{
		pHdev = pObj->pHdev;
		if(pHdev && pHdev->pRadioCtrl && pHdev->pRadioCtrl->BandIdx)
		{
			return  1;
		}
	}

	return 0;
}



/*
*
*/
UINT32 RcGetWmmIdx(HD_DEV_OBJ *pObj)
{
	HD_DEV *pHdev;
	if(pObj)
	{
		pHdev = pObj->pHdev;
		if(pHdev && pHdev->pRadioCtrl)
		{
			return pObj->WmmIdx;
		}
	}
	return 0;
}



/*
*
*/
UINT32 RcGetBandIdxByChannel(HD_CFG *pHdCfg,UCHAR Channel)
{
#ifdef DBDC_MODE

	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*) pHdCfg->priv;
	RADIO_CTRL *pRadioCtrl = NULL;
	/*not enable dbdc mode band should always in band0*/
	if(!pAd->CommonCfg.dbdc_mode)
	{
		return 0;
	}
	/*enable dbdc mode, chose bandIdx from channel*/
	if(Channel > 14)
	{
		pRadioCtrl = rcGetRadioCtrlByRf(pHdCfg,RFIC_5GHZ);
	}else
	{
		pRadioCtrl = rcGetRadioCtrlByRf(pHdCfg,RFIC_24GHZ);
	}

	if(pRadioCtrl)
	{
		return pRadioCtrl->BandIdx;
	}
#endif /*DBDC_MODE*/

	return 0;
}


/*
*
*/
VOID RcRadioInit(HD_CFG *pHdCfg,UCHAR RfIC, UCHAR DbdcMode)
{
	RADIO_CTRL *pRadioCtrl = NULL;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
	UCHAR i;

	if ((pHdCfg->chipCap.asic_caps & fASIC_CAP_DBDC) && DbdcMode)
	{
		pHwResource->concurrent_bands = 2;
	}else
	{
		pHwResource->concurrent_bands = 1;
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): DbdcMode=%d, ConcurrentBand=%d\n",
		__FUNCTION__,DbdcMode,pHwResource->concurrent_bands));
	/*Allocate PhyCtrl for HwResource*/
	for(i=0; i<pHwResource->concurrent_bands; i++)
	{
		pPhyCtrl =  &pHwResource->PhyCtrl[i];
		pRadioCtrl =&pPhyCtrl->RadioCtrl;
		pPhyCtrl->rf_band_cap = rcGetBandSupport(pHdCfg,DbdcMode,i);
		pRadioCtrl->BandIdx = i;
		pRadioCtrl->ExtCha = EXTCHA_NOASSIGN;
		if( (pPhyCtrl->rf_band_cap) & RFIC_24GHZ )
		{
			pRadioCtrl->Channel = rcGetDefaultChannel(WMODE_B);
		}else
		{
			pRadioCtrl->Channel = rcGetDefaultChannel(WMODE_A);
		}
		pRadioCtrl->PhyMode = rcGetDefaultPhyMode(pRadioCtrl->Channel);

		pRadioCtrl->CurStat = PHY_IDLE;

		/*if only one band, band to band 0*/
#ifdef TXBF_SUPPORT
		if(pHwResource->concurrent_bands==1){
			pRadioCtrl->IsBfBand = 1;
		}
#endif /*TXBF_SUPPORT*/

#ifdef GREENAP_SUPPORT
                pRadioCtrl->bGreenAPActive = FALSE;
#endif /* GREENAP_SUPPORT */

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): pRadioCtrl=%p,Band=%d,rfcap=%d,channel=%d,PhyMode=%d extCha=0x%x\n",
			__FUNCTION__,pRadioCtrl,i,pPhyCtrl->rf_band_cap,pRadioCtrl->Channel,pRadioCtrl->PhyMode,pRadioCtrl->ExtCha));

		HdevInit(pHdCfg,i,pRadioCtrl);
	}

	RcUpdateBandCtrl(pHdCfg);
}


/*
*
*/
VOID RcReleaseBandForObj( HD_CFG *pHdCfg,HD_DEV_OBJ *pObj)
{
	HD_DEV *pHdev = NULL;

	if(!pObj)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s():can't find pObj\n",__FUNCTION__));
		return ;
	}

	ReleaseOmacIdx(pHdCfg,pObj->Type,pObj->OmacIdx);

	pHdev = pObj->pHdev;

	if(pHdev)
	{
		if(pObj->bWmmAcquired)
		{
			WcReleaseEdca(pObj);
		}
		HdevObjDel(pHdev,pObj);
        NdisFreeSpinLock(&pObj->RefCntLock);
		//pObj is not dynamically allocated, so we can't free it.
		//os_free_mem(pObj);
	}
	return;
}


/*
* Refine when OmacIdx is ready
*/
HD_DEV_OBJ* RcAcquiredBandForObj(HD_CFG *pHdCfg,UCHAR ObjIdx, UCHAR PhyMode, UCHAR Channel, UCHAR ObjType)
{

	HD_DEV *pHdev = NULL;
	HD_DEV_OBJ *pHdevObj = pHdCfg->HObjList[ObjIdx];
	RADIO_CTRL *pRadioCtrl = NULL;

	/*Release first*/
	if(pHdevObj)
	{
		RcReleaseBandForObj(pHdCfg,pHdevObj);
	}

	pHdev = RcGetHdevByPhyMode(pHdCfg,PhyMode);
    if (!pHdev)
        return NULL;

    pRadioCtrl = pHdev->pRadioCtrl;

	//os_alloc_mem(NULL,(UCHAR**)&pHdevObj,sizeof(HD_DEV_OBJ));
	//os_zero_mem(pHdevObj,sizeof(HD_DEV_OBJ));
	// Instead of allocate new memory, we use pre-allocated memory
	pHdevObj = &pHdCfg->HObjBody[ObjIdx];
	/*Can get Hdev. change phyCtrl to INUSED state*/
	pRadioCtrl->CurStat = PHY_INUSE;
	/*if mixed mode*/
	if ((ObjType == WDEV_TYPE_STA) && (!WMODE_5G_ONLY(PhyMode) || !WMODE_2G_ONLY(PhyMode)))
	{
		pRadioCtrl->PhyMode = PhyMode;
		pRadioCtrl->Channel=  Channel;
	}
	else
	/*Make phymode of band should be the maxize*/
	if(wmode_band_equal(pRadioCtrl->PhyMode,PhyMode))
	{
		pRadioCtrl->PhyMode |=  PhyMode;
	}
	else
	if(ObjIdx==0)
	{
		pRadioCtrl->PhyMode = PhyMode;
		pRadioCtrl->Channel= Channel;
	}

	if(pRadioCtrl->Channel== rcGetDefaultChannel(pRadioCtrl->PhyMode))
	{
		/* apcli always follows phy mode channel*/
		if (ObjType != WDEV_TYPE_APCLI)
			pRadioCtrl->Channel = Channel ? Channel : rcGetDefaultChannel(pRadioCtrl->PhyMode);
	}
	pHdevObj->Idx = ObjIdx;
	pHdevObj->Type = ObjType;
	pHdevObj->OmacIdx = GetOmacIdx(pHdCfg,ObjType,ObjIdx);
	pHdevObj->WmmIdx = 0;

	HdevObjAdd(pHdev,pHdevObj);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(): BandIdx:%d, PhyMode=%d,Channel=%d,pHdev=%p,pHdevObj=%p\n",
		__FUNCTION__,pRadioCtrl->BandIdx,pRadioCtrl->PhyMode,pRadioCtrl->Channel,
		pHdev,pHdevObj));

	RcUpdateBandCtrl(pHdCfg);
    NdisAllocateSpinLock(NULL, &pHdevObj->RefCntLock);

	return pHdevObj;
}



/*
*
*/
HD_DEV* RcGetHdevByChannel(HD_CFG *pHdCfg,UCHAR Channel)
{
	HD_DEV *pHdev;

	if(Channel <= 14 && (pHdev=rcGetHdevByRf(pHdCfg,RFIC_24GHZ)))
	{
		return pHdev;
	}else
	if((pHdev=rcGetHdevByRf(pHdCfg,RFIC_5GHZ)))
	{
		return pHdev;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s():Err! Update PhyMode failed, no phyctrl support this channel=%d!\n",__FUNCTION__, Channel));
	return NULL;
}


/*
*
*/
HD_DEV* RcGetHdevByPhyMode(HD_CFG *pHdCfg, UCHAR PhyMode)
{
	UCHAR i;
	HD_DEV *pHdev = NULL;
	HD_RESOURCE_CFG *pHwResourceCfg =&pHdCfg->HwResourceCfg;
	CHAR *str = NULL;

	RTMP_PHY_CTRL *pPhyCtrl = NULL;

	for(i=0;i<pHwResourceCfg->concurrent_bands;i++)
	{
		pPhyCtrl = &pHwResourceCfg->PhyCtrl[i];

		if(WMODE_CAP_2G(PhyMode) &&  (pPhyCtrl->rf_band_cap & RFIC_24GHZ))
		{
			pHdev = &pHdCfg->Hdev[i];
			break;

		}
		else
		if(WMODE_CAP_5G(PhyMode) && (pPhyCtrl->rf_band_cap & RFIC_5GHZ))
		{
			pHdev = &pHdCfg->Hdev[i];
			break;
		}
	}

	if(!pHdev)
	{
		str = wmode_2_str(PhyMode);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s():Err! chip not support this PhyMode:%s !\n",__FUNCTION__, str));
		if (str)
			os_free_mem(str);
	}

	return pHdev;
}




/*
*
*/
INT32 RcUpdateChannel(HD_DEV *pHdev,UCHAR Channel)
{
	INT32 ret=0;
	UCHAR RfIC;
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;

	pRadioCtrl->Channel= Channel;
	/*band is not changed or not*/
	RfIC = wmode_2_rfic(pRadioCtrl->PhyMode);
	if(rcCheckIsTheSameBand(pRadioCtrl->PhyMode,Channel))
	{

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("Is the same band,.not need to do more change\n"));
		return ret;
	}
	return -1;
}



/*
*
*/
INT32 RcUpdateRadio(HD_DEV *pHdev,UCHAR bw,UCHAR central_ch1,UCHAR control_ch2)
{
	INT32 ret=0;
	UCHAR RfIC;
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;

	pRadioCtrl->CentralCh = central_ch1;
    pRadioCtrl->Bw = bw;
    pRadioCtrl->Channel2 = control_ch2;
	/*band is not changed or not*/
	RfIC = wmode_2_rfic(pRadioCtrl->PhyMode);
	if(rcCheckIsTheSameBand(pRadioCtrl->PhyMode,central_ch1))
	{

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("Is the same band,.not need to do more change\n"));
		return ret;
	}

	return -1;
}


/*
*
*/
INT32 RcUpdatePhyMode(HD_DEV *pHdev,UCHAR PhyMode)
{
	INT32 ret=0;
	UCHAR RfIC;
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;

	/*band is not changed or not*/
	RfIC = wmode_2_rfic(PhyMode);
	if(rcCheckIsTheSameBand(PhyMode,pRadioCtrl->Channel))
	{
		pRadioCtrl->PhyMode |= PhyMode;
		return ret;
	}

	/*band is changed*/
	pRadioCtrl->PhyMode = PhyMode;
	pRadioCtrl->Channel = rcGetDefaultChannel(PhyMode);

	RcUpdateBandCtrl((HD_CFG*)pHdev->priv);
	return -1;
}


/*
*
*/
UCHAR RcUpdateBw(HD_DEV *pHdev,UCHAR Bw)
{
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;

	/*Legacy mode, only can support BW20*/
	if(!WMODE_CAP_N(pRadioCtrl->PhyMode) && Bw > BW_20)
	{
		pRadioCtrl->Bw = BW_20;
	}else
	if(!WMODE_CAP_AC(pRadioCtrl->PhyMode) && Bw > BW_40)
	{
		pRadioCtrl->Bw = BW_40;
	}else
	{
		pRadioCtrl->Bw = Bw;
	}
	
	return Bw;
}

/*
*
*/
INT32 RcUpdateExtCha(HD_DEV *pHdev,UCHAR ExtCha)
{
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;
	pRadioCtrl->ExtCha = ExtCha;
	return -1;
}

/*
*
*/
UCHAR RcGetExtCha(HD_DEV *pHdev)
{
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;
	return pRadioCtrl->ExtCha;
}

/*
*
*/
UCHAR RcGetPhyMode(HD_DEV *pHdev)
{
	return pHdev->pRadioCtrl->PhyMode;
}


/*
*
*/
UCHAR RcGetChannel(HD_DEV *pHdev)
{
	return pHdev->pRadioCtrl->Channel;
}


/*
*
*/
UCHAR RcGetCentralCh(HD_DEV *pHdev)
{
	return pHdev->pRadioCtrl->CentralCh;
}


/*
*
*/
UCHAR RcGetBandIdx(HD_DEV *pHdev)
{
	return pHdev->pRadioCtrl->BandIdx;
}

/*
*
*/
PHY_STATUS RcGetRadioCurStat(HD_DEV *pHdev)
{
	return pHdev->pRadioCtrl->CurStat;
}

/*
*
*/
VOID RcSetRadioCurStat(HD_DEV *pHdev, PHY_STATUS CurStat)
{
	pHdev->pRadioCtrl->CurStat = CurStat;
}


/*
*
*/
UCHAR RcGetBw(HD_DEV *pHdev)
{
	return pHdev->pRadioCtrl->Bw;
}


/*
*
*/
UCHAR RcGetBandIdxByRf(HD_CFG *pHdCfg,UCHAR RfIC)
{
	HD_DEV *pHdev = rcGetHdevByRf(pHdCfg,RfIC);

	if(pHdev)
	{
		return RcGetBandIdx(pHdev);
	}
	return 0;
}


/*
*
*/
HD_DEV *RcGetBandIdxByBf(HD_CFG *pHdCfg)
{
	HD_RESOURCE_CFG *pHwResourceCfg =&pHdCfg->HwResourceCfg;
	RADIO_CTRL *pRadioCtrl = NULL;
	UCHAR i;

	for(i=0;i<pHwResourceCfg->concurrent_bands;i++)
	{
		pRadioCtrl = &pHwResourceCfg->PhyCtrl[i].RadioCtrl;
		if(pRadioCtrl->IsBfBand)
		{
			return &pHdCfg->Hdev[i];
		}
	}
	return NULL;
}


/*
*
*/
HD_DEV *RcInit(HD_CFG *pHdCfg)
{
	HD_RESOURCE_CFG *pHwResourceCfg =&pHdCfg->HwResourceCfg;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	UCHAR i;

	for(i=0;i<DBDC_BAND_NUM;i++)
	{
		pPhyCtrl = &pHwResourceCfg->PhyCtrl[i];
		os_zero_mem(pPhyCtrl,sizeof(RTMP_PHY_CTRL));

	}
	return NULL;
}


/*
*
*/
VOID RcRadioShow(HD_RESOURCE_CFG *pHwResourceCfg)
{
	UCHAR i;

	for(i=0;i<pHwResourceCfg->concurrent_bands;i++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBand: %d,RfIC: %d, IsBfCap: %d\n",
		i, pHwResourceCfg->PhyCtrl[i].rf_band_cap,pHwResourceCfg->PhyCtrl[i].RadioCtrl.IsBfBand));
		
	}
}

/*
*
*/
BOOLEAN RcIsBfCapSupport(HD_DEV_OBJ *obj)
{	
	HD_DEV *dev = obj->pHdev;
	RADIO_CTRL *rc;

	if(!dev)
		return FALSE;

	rc = dev->pRadioCtrl;

	return rc->IsBfBand;
}
	
