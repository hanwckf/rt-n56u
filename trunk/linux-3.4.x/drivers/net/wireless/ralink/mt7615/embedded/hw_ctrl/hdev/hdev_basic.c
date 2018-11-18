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
	hdev.c
*/
#include "rt_config.h"
#include "hdev/hdev.h"

/*Local functions*/
VOID HdevHwResourceInit(HD_CFG *pHdevCfg)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &pHdevCfg->HwResourceCfg;

	os_zero_mem(pHwResourceCfg,sizeof(HD_RESOURCE_CFG));
	/*initial radio control*/
	RcInit(pHdevCfg);
	/*initial wmm control*/
	WcInit(pHdevCfg,&pHwResourceCfg->WmmCtrl);
	/*initial wtbl control*/	
	WtcInit(pHdevCfg);

}

VOID HdevHwResourceExit(HD_CFG *pHdevCfg)
{
	UCHAR i;
	HD_RESOURCE_CFG *pHwResourceCfg = &pHdevCfg->HwResourceCfg;

	WtcExit(pHdevCfg);

	for(i=0;i< pHwResourceCfg->concurrent_bands;i++)
	{	
		HdevExit(pHdevCfg,i);
	}

	WcExit(&pHwResourceCfg->WmmCtrl);

}


static VOID HdevHwResourceShow(HD_RESOURCE_CFG *pHwResourceCfg)
{	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tConconcurrent Bands:%d\n",pHwResourceCfg->concurrent_bands));	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"));
	RcRadioShow(pHwResourceCfg);	

}


/*
*
*/
static VOID HdevObjShow(HD_DEV_OBJ *pObj)
{
	HD_REPT_ENRTY *pEntry;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tObjIdx:%d, OMACIdx:%d\n",pObj->Idx, pObj->OmacIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tType: %d\n",pObj->Type));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tbWmmAcquired:%d, WmmIdx:%d\n",pObj->bWmmAcquired,pObj->WmmIdx));
	DlListForEach(pEntry,&pObj->RepeaterList,struct _HD_REPT_ENRTY, list)
	{		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRepeaterIdx:%d, OmacIdx:%x\n",pEntry->CliIdx,pEntry->ReptOmacIdx));
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"));
}


/*
*
*/
static VOID HdevObjListShow(HD_CFG *pHdevCfg)
{
	UINT32 i;
	HD_DEV_OBJ *pObj;

	for(i=0;i<WDEV_NUM_MAX;i++)
	{
		pObj = pHdevCfg->HObjList[i];

		if(pObj)
		{			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tArrayIdx:%d",i));
			HdevObjShow(pObj);
		}
	}
}


/*
*
*/
static VOID HdevShow(HD_DEV *pHdev)
{
	HD_DEV_OBJ *pObj = NULL;
	RADIO_CTRL *pRadioCtrl = pHdev->pRadioCtrl;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHdevIdx:%d\n",pHdev->Idx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBandIdx:%d,CurStat:%d\n",pRadioCtrl->BandIdx,pRadioCtrl->CurStat));	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPhyMode:%d,BW:%d\n",
		pRadioCtrl->PhyMode,pRadioCtrl->Bw));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannel:%d,Central Channel:%d\n",
		pRadioCtrl->Channel,pRadioCtrl->CentralCh));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannel2:%d\n",
		pRadioCtrl->Channel2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tExtCha:%d\n",
		pRadioCtrl->ExtCha));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tObjNum:%d\n",pHdev->DevNum));
	DlListForEach(pObj,&pHdev->DevObjList,struct _HD_DEV_OBJ,list) {
		HdevObjShow(pObj);
	}	
}


/*
*
*/
static VOID HdevDel(HD_DEV *pHdev)
{
	HD_DEV_OBJ *pObj,*pTemp ; 
	HD_CFG *pHdCfg = (HD_CFG*)pHdev->priv;
	
	DlListForEachSafe(pObj,pTemp,&pHdev->DevObjList, struct _HD_DEV_OBJ, list) {
		DlListDel(&pObj->list);		
		pHdCfg->HObjList[pObj->Idx] = NULL;
		os_free_mem(pObj);
		pHdev->DevNum--;
	}
	
	DlListInit(&pHdev->DevObjList);
}


/*Export function*/
/*
 *
*/
VOID HdevObjAdd(HD_DEV *pHdev,HD_DEV_OBJ *pObj)
{
	HD_CFG *pHdCfg = (HD_CFG*)pHdev->priv;
	
	DlListAddTail(&pHdev->DevObjList,&pObj->list);
	pObj->pHdev = pHdev;
	DlListInit(&pObj->RepeaterList);	
	pHdev->DevNum++;
	pHdCfg->HObjList[pObj->Idx] = pObj;
}


/*
 *
*/
VOID HdevObjDel(HD_DEV *pHdev,HD_DEV_OBJ *pObj)
{
	HD_CFG *pHdCfg = (HD_CFG*)pHdev->priv;
	
	pHdCfg->HObjList[pObj->Idx] = NULL;
	DlListDel(&pObj->list);	
	DlListInit(&pObj->RepeaterList);	
	pHdev->DevNum--;
}



/*
*
*/

INT32 HdevInit(HD_CFG *pHdevCfg,UCHAR HdevIdx,RADIO_CTRL *pRadioCtrl)
{
	HD_DEV *pHdev = NULL;
	if(HdevIdx >= DBDC_BAND_NUM)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		        ("%s: HdevIdx:%d >= %d\n",__FUNCTION__, HdevIdx, DBDC_BAND_NUM));
		return 0;
	}
	pHdev = &pHdevCfg->Hdev[HdevIdx];
	os_zero_mem(pHdev,sizeof(HD_DEV));
	DlListInit(&pHdev->DevObjList);
	pHdev->pRadioCtrl = pRadioCtrl;
	pHdev->priv =  pHdevCfg;
	pHdev->Idx= HdevIdx;
	pHdev->DevNum = 0;
	return 0;
}


/*
*
*/

INT32 HdevExit(HD_CFG *pHdevCfg,UCHAR HdevIdx)
{
	HD_DEV *pHdev = NULL;
	if(HdevIdx >= DBDC_BAND_NUM)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		        ("%s: HdevIdx:%d >= %d\n",__FUNCTION__, HdevIdx, DBDC_BAND_NUM));
		return 0;
	}
	pHdev = &pHdevCfg->Hdev[HdevIdx];
	HdevDel(pHdev);
	os_zero_mem(pHdev,sizeof(HD_DEV));
	return 0;
}



VOID HdevCfgShow(HD_CFG *pHdevCfg)
{
	UCHAR i;
	HD_RESOURCE_CFG *pHwResource = &pHdevCfg->HwResourceCfg;

	HdevHwResourceShow(pHwResource);

	for(i=0;i<pHwResource->concurrent_bands;i++)
	{
		HdevShow(&pHdevCfg->Hdev[i]);
	}
	
	HdevObjListShow(pHdevCfg);
}
