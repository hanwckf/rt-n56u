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
	wifi_sys_info.c
*/
#include	"rt_config.h"



/*Local function*/
static VOID _GetNetworkTypeStr(UINT32 Type,CHAR *str)
{
	if(Type & NETWORK_INFRA)
	{
		sprintf(str,"%s","NETWORK_INFRA");
	}else
	if(Type & NETWORK_P2P)
	{
		sprintf(str,"%s","NETWORK_P2P");
	}else
	if(Type & NETWORK_IBSS)
	{
		sprintf(str,"%s","NETWORK_IBSS");
	}else
	if(Type & NETWORK_MESH)
	{
		sprintf(str,"%s","NETWORK_MESH");
	}else
	if(Type & NETWORK_BOW)
	{
		sprintf(str,"%s","NETWORK_BOW");
	}else
	if(Type & NETWORK_WDS)
	{
		sprintf(str,"%s","NETWORK_WDS");
	}else
	{
		sprintf(str,"%s","UND");
	}
}


static VOID _WifiSysInfoDevInfoDump(WIFI_INFO_CLASS_T *pWifiClass)
{
	DEV_INFO_CTRL_T *pDevInfo = NULL;
	struct wifi_dev *wdev = NULL;
	DlListForEach(pDevInfo,&pWifiClass->Head,DEV_INFO_CTRL_T,list){
		wdev = (struct wifi_dev*)pDevInfo->priv;
		printk("#####WdevIdx (%d)#####\n",wdev->wdev_idx);
		printk("Active: %d\n",pDevInfo->Active);
		printk("BandIdx: %d\n",pDevInfo->BandIdx);
		printk("EnableFeature: %d\n",pDevInfo->EnableFeature);
		printk("OwnMacIdx: %d\n",pDevInfo->OwnMacIdx);
		printk("OwnMacAddr: %x:%x:%x:%x:%x:%x\n",PRINT_MAC(pDevInfo->OwnMacAddr));

	}
}


static VOID _WifiSysInfoBssInfoDump(WIFI_INFO_CLASS_T *pWifiClass)
{
	BSS_INFO_ARGUMENT_T *pBssInfo = NULL;
	struct wifi_dev *wdev = NULL;
	CHAR str[128]="";

	DlListForEach(pBssInfo,&pWifiClass->Head,BSS_INFO_ARGUMENT_T,list){
		wdev = (struct wifi_dev*)pBssInfo->priv;
		printk("#####WdevIdx (%d)#####\n",wdev->wdev_idx);
		printk("State: %d\n",pBssInfo->bss_state);
		printk("Bssid: %x:%x:%x:%x:%x:%x\n",PRINT_MAC(pBssInfo->Bssid));
		printk("CipherSuit: %d\n",pBssInfo->CipherSuit);
		_GetNetworkTypeStr(pBssInfo->NetworkType,str);
		printk("NetworkType: %s\n",str);
		printk("OwnMacIdx: %d\n",pBssInfo->OwnMacIdx);
		printk("BssInfoFeature: %x\n",pBssInfo->u4BssInfoFeature);
		printk("ConnectionType: %d\n",pBssInfo->u4ConnectionType);
		printk("BcMcWlanIdx: %d\n",pBssInfo->ucBcMcWlanIdx);
		printk("BssIndex: %d\n",pBssInfo->ucBssIndex);
		printk("PeerWlanIdx: %d\n",pBssInfo->ucPeerWlanIdx);
		printk("WmmIdx: %d\n",pBssInfo->WmmIdx);
		printk("BcTransmit: (Mode/BW/MCS) %d/%d/%d\n",pBssInfo->BcTransmit.field.MODE,
			pBssInfo->BcTransmit.field.BW,pBssInfo->BcTransmit.field.MCS);
		printk("McTransmit: (Mode/BW/MCS) %d/%d/%d\n",pBssInfo->McTransmit.field.MODE,
			pBssInfo->BcTransmit.field.BW,pBssInfo->BcTransmit.field.MCS);

	}
}


static VOID _WifiSysInfoStaRecDump(WIFI_INFO_CLASS_T *pWifiClass)
{
	STA_REC_CTRL_T *pStaRec = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	DlListForEach(pStaRec,&pWifiClass->Head,STA_REC_CTRL_T,list){
		tr_entry = (STA_TR_ENTRY*)pStaRec->priv;
		printk("#####MacEntry (%d)#####\n",tr_entry->wcid);
		printk("PeerAddr: %x:%x:%x:%x:%x:%x\n",PRINT_MAC(tr_entry->bssid));
		printk("WlanIdx: %d\n",pStaRec->WlanIdx);
		printk("BssIndex: %d\n",pStaRec->BssIndex);
		printk("ConnectionState: %d\n",pStaRec->ConnectionState);
		printk("ConnectionType: %d\n",pStaRec->ConnectionType);
		printk("EnableFeature: %x\n",pStaRec->EnableFeature);
	}
}

VOID WifiSysInfoReset(WIFI_SYS_INFO_T *pWifiSysInfo)
{
	DlListInit(&pWifiSysInfo->DevInfo.Head);
	DlListInit(&pWifiSysInfo->StaRec.Head);
	DlListInit(&pWifiSysInfo->BssInfo.Head);

	pWifiSysInfo->DevInfo.Num = 0;
	pWifiSysInfo->StaRec.Num = 0;
	pWifiSysInfo->BssInfo.Num = 0;
}

/*
*
*/
VOID WifiSysInfoInit(RTMP_ADAPTER *pAd)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;

	NdisAllocateSpinLock(pAd, &pWifiSysInfo->lock);

	WifiSysInfoReset(pWifiSysInfo);
}


/*
*
*/
VOID WifiSysInfoDump(RTMP_ADAPTER *pAd)
{

	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	printk("===============================\n");
	printk("Current DevInfo Num: %d\n",pWifiSysInfo->DevInfo.Num);	
	printk("===============================\n");
	_WifiSysInfoDevInfoDump(&pWifiSysInfo->DevInfo);

	printk("===============================\n");
	printk("Current BssInfo Num: %d\n",pWifiSysInfo->BssInfo.Num);	
	printk("===============================\n");
	_WifiSysInfoBssInfoDump(&pWifiSysInfo->BssInfo);

	printk("===============================\n");
	printk("Current StaRec Num: %d\n",pWifiSysInfo->StaRec.Num);	
	printk("===============================\n");
	_WifiSysInfoStaRecDump(&pWifiSysInfo->StaRec);
}


/*
*
*/
static VOID WifiSysAddDevInfo(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	DEV_INFO_CTRL_T *pDevInfo = &wdev->DevInfo, *pDevInfoTmp=NULL;

	OS_SEM_LOCK(&pWifiSysInfo->lock);

	DlListForEach(pDevInfoTmp,&pWifiSysInfo->DevInfo.Head,DEV_INFO_CTRL_T,list){
		if(pDevInfo==pDevInfoTmp)
		{
			OS_SEM_UNLOCK(&pWifiSysInfo->lock);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("%s(): DevInfo %d already exist",
			__FUNCTION__,pDevInfo->OwnMacIdx));
			return;
		}
	}

	DlListAddTail(&pWifiSysInfo->DevInfo.Head,&pDevInfo->list);
	pDevInfo->priv = (VOID*)wdev;
	pWifiSysInfo->DevInfo.Num++;
	
	OS_SEM_UNLOCK(&pWifiSysInfo->lock);
}


/*
*
*/
static VOID WifiSysDelDevInfo(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	DEV_INFO_CTRL_T *pDevInfo = &wdev->DevInfo;

	OS_SEM_LOCK(&pWifiSysInfo->lock);

	DlListDel(&pDevInfo->list);
	os_zero_mem(pDevInfo,sizeof(DEV_INFO_CTRL_T));
	pWifiSysInfo->DevInfo.Num--;
	
	OS_SEM_UNLOCK(&pWifiSysInfo->lock);
}


/*
*
*/
static VOID WifiSysAddBssInfo(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	BSS_INFO_ARGUMENT_T *pBssInfo = &wdev->bss_info_argument, *pBssInfoTmp;


	OS_SEM_LOCK(&pWifiSysInfo->lock);

	DlListForEach(pBssInfoTmp,&pWifiSysInfo->BssInfo.Head,BSS_INFO_ARGUMENT_T,list){
		if(pBssInfo==pBssInfoTmp)
		{
			OS_SEM_UNLOCK(&pWifiSysInfo->lock);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("%s(): BssInfo %d already exist",
			__FUNCTION__,pBssInfo->ucBssIndex));			
			return;
		}
	}

	DlListAddTail(&pWifiSysInfo->BssInfo.Head,&pBssInfo->list);
	pBssInfo->priv = (VOID*)wdev;
	pWifiSysInfo->BssInfo.Num++;
	
	OS_SEM_UNLOCK(&pWifiSysInfo->lock);
}


/*
*
*/
static VOID WifiSysDelBssInfo(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	BSS_INFO_ARGUMENT_T *pBssInfo = &wdev->bss_info_argument;

	OS_SEM_LOCK(&pWifiSysInfo->lock);

	DlListDel(&pBssInfo->list);
	os_zero_mem(pBssInfo,sizeof(BSS_INFO_ARGUMENT_T));
	pWifiSysInfo->BssInfo.Num--;
	
	OS_SEM_UNLOCK(&pWifiSysInfo->lock);
}

BSSINFO_STATE_T WifiSysGetBssInfoState(RTMP_ADAPTER *pAd, UINT8 ucBssInfoIdx)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	BSS_INFO_ARGUMENT_T *pBssInfo;

	DlListForEach(pBssInfo,&pWifiSysInfo->BssInfo.Head,BSS_INFO_ARGUMENT_T,list){
		if (pBssInfo->ucBssIndex == ucBssInfoIdx)
		{
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				("%s(): BssInfoIdx %d found, current state = %d\n",
				__FUNCTION__,pBssInfo->ucBssIndex, pBssInfo->bss_state));
			return pBssInfo->bss_state;
		}
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
		("%s(): BssInfoIdx %d not found!!!\n",
		__FUNCTION__, ucBssInfoIdx));

	return BSS_INIT;
}

VOID WifiSysUpdateBssInfoState(RTMP_ADAPTER *pAd, UINT8 ucBssInfoIdx, BSSINFO_STATE_T bss_state)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	BSS_INFO_ARGUMENT_T *pBssInfo;

	OS_SEM_LOCK(&pWifiSysInfo->lock);

	DlListForEach(pBssInfo,&pWifiSysInfo->BssInfo.Head,BSS_INFO_ARGUMENT_T,list){
		if (pBssInfo->ucBssIndex == ucBssInfoIdx)
		{
			pBssInfo->bss_state = bss_state;		
			OS_SEM_UNLOCK(&pWifiSysInfo->lock);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				("%s(): BssInfoIdx %d found, update state to %d\n",
				__FUNCTION__,pBssInfo->ucBssIndex, bss_state));

			return;
		}
	}

	OS_SEM_UNLOCK(&pWifiSysInfo->lock);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
		("%s(): BssInfoIdx %d not found!!!\n",
		__FUNCTION__, ucBssInfoIdx));
}


VOID DbgCheckLinkList(PDL_LIST list2chk, UINT8 chkpoint)
{
	STA_REC_CTRL_T *item = NULL;
	UINT8 item_chked = 0;
	
	if (list2chk->Next == NULL){

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::pWifiSysInfo->StaRec.Head.Next is NULL\n", __FUNCTION__, chkpoint));
	
	}

	item = DlListEntry((list2chk)->Next, STA_REC_CTRL_T, list);

	if (item == NULL){

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::item is NULL\n", __FUNCTION__, chkpoint));

	}

	while (&item->list != list2chk){

		if (item->list.Next == NULL){

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::item->list.Next is NULL, item_chked=%d\n", __FUNCTION__, chkpoint, item_chked));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::item is TREntry %d\n", __FUNCTION__, chkpoint, item->WlanIdx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::BssIndex=%d\n", __FUNCTION__, chkpoint, item->BssIndex));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::ConnectionType=%d\n", __FUNCTION__, chkpoint, item->ConnectionType));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::ConnectionState=%d\n", __FUNCTION__, chkpoint, item->ConnectionState));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::EnableFeature=%d\n", __FUNCTION__, chkpoint, item->EnableFeature));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::IsNewSTARec=%d\n", __FUNCTION__, chkpoint, item->IsNewSTARec));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::priv=%p\n", __FUNCTION__, chkpoint, item->priv));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::Prev=%p\n", __FUNCTION__, chkpoint, item->list.Prev));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR:%s(%d)::Next=%p\n", __FUNCTION__, chkpoint, item->list.Next));
		}

		item_chked++;

		item = DlListEntry(item->list.Next, STA_REC_CTRL_T, list);

	}

}


/*
*
*/
VOID WifiSysAddStaRec(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, STA_REC_CTRL_T *sta_rec_ctrl)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;

	STA_REC_CTRL_T *pStaRec = &tr_entry->StaRec,*pStaRecTmp=NULL;

	OS_SEM_LOCK(&pWifiSysInfo->lock);

	pStaRec->BssIndex = sta_rec_ctrl->BssIndex;
	pStaRec->ConnectionState = sta_rec_ctrl->ConnectionState;
	pStaRec->ConnectionType = sta_rec_ctrl->ConnectionType;
	pStaRec->EnableFeature = sta_rec_ctrl->EnableFeature;
	pStaRec->IsNewSTARec = sta_rec_ctrl->IsNewSTARec;
	pStaRec->WlanIdx = sta_rec_ctrl->WlanIdx;
	/* Do not update pStaRec->list, it could be in the link list already. */
	/* pStaRec->priv is updated later. */

	DbgCheckLinkList(&pWifiSysInfo->StaRec.Head, 1);

	DlListForEach(pStaRecTmp,&pWifiSysInfo->StaRec.Head,STA_REC_CTRL_T,list){

		DbgCheckLinkList(&pWifiSysInfo->StaRec.Head, 2);

		if(pStaRecTmp==pStaRec)
		{
			OS_SEM_UNLOCK(&pWifiSysInfo->lock);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("%s(): STARec %d already exist\n",
			__FUNCTION__,pStaRec->WlanIdx));			
			return;
		}
	}
	
	DlListAddTail(&pWifiSysInfo->StaRec.Head,&pStaRec->list);
	pStaRec->priv = (VOID*)tr_entry;
	pWifiSysInfo->StaRec.Num++;
	
	OS_SEM_UNLOCK(&pWifiSysInfo->lock);
}


/*
*
*/
VOID WifiSysDelStaRec(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry)
{
	WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
	STA_REC_CTRL_T *pStaRec = &tr_entry->StaRec;
	
	OS_SEM_LOCK(&pWifiSysInfo->lock);

	if(pStaRec->list.Prev == NULL)
	{
		OS_SEM_UNLOCK(&pWifiSysInfo->lock);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF,
				("%s(): STARec %d Type = %d has been deleted\n",
				__FUNCTION__, tr_entry->wcid, tr_entry->EntryType));			
		return;
	}

	DbgCheckLinkList(&pWifiSysInfo->StaRec.Head, 3);

	DlListDel(&pStaRec->list);			
	os_zero_mem(pStaRec,sizeof(STA_REC_CTRL_T));
	pWifiSysInfo->StaRec.Num--;
	
	OS_SEM_UNLOCK(&pWifiSysInfo->lock);

}


/*
*
*/
#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
static VOID WifiSysUpdateIgmpInfo(RTMP_ADAPTER *pAd,struct wifi_dev *wdev, BOOLEAN bActive)
{
	DEV_INFO_CTRL_T *pDevCtrl = &wdev->DevInfo;

	if (wdev->wdev_type == WDEV_TYPE_AP)
	{
		wdev->IgmpSnoopEnable
			= pAd->ApCfg.IgmpSnoopEnable[pDevCtrl->BandIdx];
		if(bActive == TRUE && wdev->IgmpSnoopEnable == TRUE)
			CmdMcastCloneEnable(pAd, TRUE, pDevCtrl->OwnMacIdx);
		else
			CmdMcastCloneEnable(pAd, FALSE, pDevCtrl->OwnMacIdx);

	}
}
#endif /* IGMP_SNOOP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


/*
*
*/
static VOID WifiSysUpdateDevInfo(RTMP_ADAPTER *pAd,struct wifi_dev *wdev, BOOLEAN bActive,DEV_INFO_CTRL_T *cfgDevInfo)
{
	DEV_INFO_CTRL_T *pDevCtrl = &wdev->DevInfo;

	pDevCtrl->Active = bActive;
	pDevCtrl->OwnMacIdx = wdev->OmacIdx;
	os_move_mem(pDevCtrl->OwnMacAddr,wdev->if_addr,MAC_ADDR_LEN);
	pDevCtrl->EnableFeature = DEVINFO_ACTIVE_FEATURE;
	pDevCtrl->BandIdx = HcGetBandByWdev(wdev);

	os_move_mem(cfgDevInfo,pDevCtrl,sizeof(DEV_INFO_CTRL_T));
	/* Set the current MAC to ASIC */
	if(bActive)
		WifiSysAddDevInfo(pAd,wdev);
	else
		WifiSysDelDevInfo(pAd,wdev);
	
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,("%s(): Active=%d,OwnMacIdx=%d,EnableFeature=%d,BandIdx=%d\n",__FUNCTION__,
	pDevCtrl->Active,pDevCtrl->OwnMacIdx,pDevCtrl->EnableFeature,pDevCtrl->BandIdx));


}

		
/*
*
*/
static VOID WifiSysUpdateBssInfo(RTMP_ADAPTER *pAd,struct wifi_dev *wdev,BSS_INFO_ARGUMENT_T *cfgBssInfo)
{	
	BSS_INFO_ARGUMENT_T *bss_arg = &wdev->bss_info_argument;

	os_move_mem(cfgBssInfo,bss_arg,sizeof(BSS_INFO_ARGUMENT_T));
	
	if(WDEV_BSS_STATE(wdev) >= BSS_ACTIVE)
	{
		WifiSysAddBssInfo(pAd,wdev);
	}else
	{	
	
		BssInfoArgumentUnLink(pAd,wdev);
		WifiSysDelBssInfo(pAd,wdev);
	}
}


/*
*
*/
static VOID WifiSysUpdateStaRec(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, STA_REC_CTRL_T *NewStaRec)
{
	if(NewStaRec->ConnectionState == STATE_DISCONNECT)
	{
		WifiSysDelStaRec(pAd, tr_entry);
	}
	else
	{
		WifiSysAddStaRec(pAd, tr_entry, NewStaRec);
	}
}

/*Below is configuration part*/

/*
*
*/
VOID WifiSysUpdatePortSecur(RTMP_ADAPTER *pAd,MAC_TABLE_ENTRY *pEntry)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	struct wifi_dev *wdev = pEntry->wdev;
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;

	if(tr_entry->StaRec.ConnectionState)
	{
		os_zero_mem(&wifi_sys_ctrl, sizeof(WIFI_SYS_CTRL));

		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->ConnectionState = STATE_PORT_SECURE;
		sta_ctrl->ConnectionType = pEntry->ConnectionType;
		sta_ctrl->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
		sta_ctrl->WlanIdx = pEntry->wcid;
		sta_ctrl->IsNewSTARec = FALSE;

		WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);

		wifi_sys_ctrl.wdev = pEntry->wdev;

		HW_WIFISYS_PEER_UPDATE(pAd,wifi_sys_ctrl);
#ifdef CONFIG_AP_SUPPORT
		CheckBMCPortSecured(pAd, pEntry, TRUE);
#endif /* CONFIG_AP_SUPPORT */
	}
}


/*
*
*/
VOID WifiSysPeerLinkDown(RTMP_ADAPTER *pAd,MAC_TABLE_ENTRY *pEntry)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;
	struct wifi_dev *wdev = pEntry->wdev;

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

	if(tr_entry->StaRec.ConnectionState)
	{
		/* Deactive StaRec in FW */
		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->WlanIdx = pEntry->wcid;
		sta_ctrl->ConnectionType = pEntry->ConnectionType;
		sta_ctrl->ConnectionState = STATE_DISCONNECT;
		sta_ctrl->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
		WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);
	}else{
		wifi_sys_ctrl.StaRecCtrl.WlanIdx = pEntry->wcid;
	}

	if (IS_ENTRY_CLIENT(pEntry)) {
		wifi_sys_ctrl.skip_set_txop = TRUE;/* there is no need to set txop when sta connected */
	} else if (IS_ENTRY_REPEATER(pEntry)) {
		wifi_sys_ctrl.skip_set_txop = TRUE;/* skip disable txop for repeater case since apcli is connected */
	}

	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_PEER_LINKDOWN(pAd,wifi_sys_ctrl);

#ifdef CONFIG_AP_SUPPORT
	CheckBMCPortSecured(pAd, pEntry, FALSE);
#endif /* CONFIG_AP_SUPPORT */
}


/*
*
*/
VOID WifiSysOpen(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	
	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), wdev idx = %d\n", 
		__FUNCTION__, wdev->wdev_idx));

	if(!wdev->DevInfo.Active && (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID))
	{
		wlan_operate_set_state(wdev,WLAN_OPER_STATE_VALID);
		/*acquire wdev related attribute*/
		wdev_attr_update(pAd,wdev);
		
		/* WDS share DevInfo with normal AP */
		if (wdev->wdev_type != WDEV_TYPE_WDS)
			WifiSysUpdateDevInfo(pAd,wdev,TRUE,&wifi_sys_ctrl.DevInfoCtrl);
#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
		WifiSysUpdateIgmpInfo(pAd, wdev, TRUE);
#endif
#endif
		wifi_sys_ctrl.wdev = wdev;
		/*update to hwctrl*/
		HW_WIFISYS_OPEN(pAd,wifi_sys_ctrl);
	}

}


/*
*
*/
VOID WifiSysClose(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{

	WIFI_SYS_CTRL wifi_sys_ctrl;
	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), wdev idx = %d\n", 
		__FUNCTION__, wdev->wdev_idx));

	if (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_VALID)
	{
		wlan_operate_set_state(wdev,WLAN_OPER_STATE_INVALID);
#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
		WifiSysUpdateIgmpInfo(pAd, wdev, FALSE);
#endif
#endif

		/* WDS share DevInfo with normal AP */
		if (wdev->wdev_type != WDEV_TYPE_WDS)
			WifiSysUpdateDevInfo(pAd,wdev,FALSE,&wifi_sys_ctrl.DevInfoCtrl);
		
		wifi_sys_ctrl.wdev = wdev;
		/*update to hwctrl*/
		HW_WIFISYS_CLOSE(pAd,wifi_sys_ctrl);
	}
	return;
}




#ifdef CONFIG_AP_SUPPORT
/*
*
*/
VOID WifiSysApLinkUp(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	UINT32 StaState;
	STA_TR_ENTRY *tr_entry;
	STA_REC_CTRL_T *sta_ctrl;

    UINT32 enableFeature =
        	STA_REC_BASIC_STA_RECORD_FEATURE | STA_REC_TX_PROC_FEATURE;

	/*if interface down up should not run ap link up (for apstop/apstart check)*/
	if(!HcIsRadioAcq(wdev))
		return ;

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));
	
	if (WDEV_BSS_STATE(wdev) == BSS_INIT)
	{

		/*link up acquire HW edca*/
		HcAcquiredEdca(pAd,wdev,&pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx]);
		/*update bssinfo*/
		BssInfoArgumentLinker(pAd, wdev);
		wdev->bss_info_argument.u4BssInfoFeature = ( BSS_INFO_OWN_MAC_FEATURE |
												  BSS_INFO_BASIC_FEATURE |
												  BSS_INFO_RF_CH_FEATURE |
												  BSS_INFO_BROADCAST_INFO_FEATURE);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (pAd->chipCap.fgRateAdaptFWOffload == TRUE)
			wdev->bss_info_argument.u4BssInfoFeature |= BSS_INFO_RA_FEATURE;
#endif

		if (wdev->OmacIdx > HW_BSSID_MAX)
		{
		  	wdev->bss_info_argument.u4BssInfoFeature = wdev->bss_info_argument.u4BssInfoFeature |
											BSS_INFO_EXT_BSS_FEATURE;
		}else
		{
			wdev->bss_info_argument.u4BssInfoFeature = (wdev->bss_info_argument.u4BssInfoFeature |
	                   BSS_INFO_SYNC_MODE_FEATURE);
		}

		WDEV_BSS_STATE(wdev) = BSS_ACTIVE;

		WifiSysUpdateBssInfo(pAd,wdev,&wifi_sys_ctrl.BssInfoCtrl);

		RTMP_STA_ENTRY_ADD(pAd,
                        wdev->bss_info_argument.ucBcMcWlanIdx,
                        BROADCAST_ADDR,
                        TRUE,
                        TRUE);

		/*update sta rec.*/
		/*1. get tr entry here, since bss info is acquired above */
		tr_entry = &pAd->MacTab.tr_entry[wdev->tr_tb_idx];
		sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;

		/* BC sta record should always set STATE_PORT_SECURE*/
		StaState = STATE_PORT_SECURE;

		if (pAd->chipCap.SupportAMSDU == TRUE) 
		{
			enableFeature |= STA_REC_AMSDU_FEATURE;
		}

		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->WlanIdx = wdev->bss_info_argument.ucBcMcWlanIdx;
		sta_ctrl->ConnectionState = StaState;
		sta_ctrl->ConnectionType = CONNECTION_INFRA_BC;
		sta_ctrl->EnableFeature = enableFeature;
		sta_ctrl->IsNewSTARec = TRUE;
		WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);
		
	    MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
	            ("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d\n",
	            __FUNCTION__,
	            wdev->bss_info_argument.ucBcMcWlanIdx, StaState));
		/*update to hw ctrl task*/
		wifi_sys_ctrl.wdev = wdev;
		HW_WIFISYS_LINKUP(pAd,wifi_sys_ctrl);

		APKeyTableInit(pAd, wdev);

	}

	OPSTATUS_SET_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
}


/*
*
*/
VOID WifiSysApLinkDown(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{

	WIFI_SYS_CTRL wifi_sys_ctrl;
	STA_TR_ENTRY *tr_entry;
	STA_REC_CTRL_T *sta_ctrl;
    	UINT32 enableFeature =
        	STA_REC_BASIC_STA_RECORD_FEATURE | STA_REC_TX_PROC_FEATURE;

	APStopRekeyTimer(pAd, wdev);

#ifdef WH_EZ_SETUP	
#ifndef EZ_MOD_SUPPORT
	if (IS_EZ_SETUP_ENABLED(wdev))
	{
		RTMP_SEM_LOCK(&wdev->ez_security.ez_apcli_list_sem_lock);
		ez_clear_apcli_list(&wdev->ez_security.ez_apcli_list);
		RTMP_SEM_UNLOCK(&wdev->ez_security.ez_apcli_list_sem_lock);
	}
#endif	
#endif /* WH_EZ_SETUP */

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));
	
	if (WDEV_BSS_STATE(wdev) >= BSS_INITED)
	{
		/*update sta rec.*/
		/*1. get tr entry here, since bss info is acquired above */
		tr_entry = &pAd->MacTab.tr_entry[wdev->tr_tb_idx];		
		sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;

		if(tr_entry->StaRec.ConnectionState)
		{
			sta_ctrl->ConnectionState = STATE_DISCONNECT;
			sta_ctrl->EnableFeature = enableFeature;
			sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
			sta_ctrl->ConnectionType = CONNECTION_INFRA_BC;
			sta_ctrl->WlanIdx = wdev->bss_info_argument.ucBcMcWlanIdx;
			WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);
		}
		/* kick out all stas behind the Bss */
		MbssKickOutStas(pAd, wdev->func_idx, REASON_DISASSOC_INACTIVE);
		wdev->bcn_buf.bBcnSntReq = FALSE;
		UpdateBeaconHandler(
			pAd,
			wdev,
			INTERFACE_STATE_CHANGE);

	
		wdev->bss_info_argument.u4BssInfoFeature = (BSS_INFO_OWN_MAC_FEATURE |
													BSS_INFO_BASIC_FEATURE |
													BSS_INFO_RF_CH_FEATURE);

		if(wdev->OmacIdx> 0)
			wdev->bss_info_argument.u4BssInfoFeature |= BSS_INFO_EXT_BSS_FEATURE;


		WDEV_BSS_STATE(wdev) = BSS_INITED;		
		OPSTATUS_CLEAR_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

		WifiSysUpdateBssInfo(pAd,wdev,&wifi_sys_ctrl.BssInfoCtrl);

		/*update to hwctrl for hw seting*/
		wifi_sys_ctrl.wdev = wdev;
		HW_WIFISYS_LINKDOWN(pAd,wifi_sys_ctrl);
	}

}


VOID WifiSysApPeerLinkUp(RTMP_ADAPTER *pAd,MAC_TABLE_ENTRY *pEntry, IE_LISTS *ie_list)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	UINT32 enableFeature = 0;
	UCHAR PortSecured = STATE_DISCONNECT;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	struct wifi_dev *wdev = pEntry->wdev;
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;
	PEER_LINKUP_HWCTRL *lu_ctrl=NULL;
	
	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));
	
	/* Jeffrey 2015.3.9, OR features instead */
	enableFeature |= STA_REC_BASIC_STA_RECORD_FEATURE;
	enableFeature |= STA_REC_TX_PROC_FEATURE;

	if (pAd->chipCap.APPSMode == APPS_MODE2)
	{
		enableFeature |= STA_REC_AP_PS_FEATURE;
	}
#if defined(MT7615) || defined(MT7622)
	/* Jeffrey 2015.3.9, Sync HT info to F/W */
#ifdef DOT11_N_SUPPORT
	if ((ie_list->ht_cap_len > 0) && WMODE_CAP_N(wdev->PhyMode))
	{
		enableFeature |= STA_REC_BASIC_HT_INFO_FEATURE;
	}
#endif

	/* Jeffrey 2015.3.9, Sync VHT info to F/W */
#ifdef DOT11_VHT_AC
	if ((ie_list->vht_cap_len > 0) && WMODE_CAP_AC(wdev->PhyMode))
	{
		enableFeature |= STA_REC_BASIC_VHT_INFO_FEATURE;
	}
#endif
#endif /* defined(MT7615) || defined(MT7622) */

	if (pAd->chipCap.SupportAMSDU == TRUE) 
	{
		enableFeature |= STA_REC_AMSDU_FEATURE;
	}

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		&& (IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
		|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
		|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
		|| pEntry->bWscCapable))
	{
		PortSecured = STATE_CONNECTED;
	} else 
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
	{

#ifdef FAST_EAPOL_WAR
		/* 
			set STATE_CONNECTED first in open security mode,
			after asso resp is sent out, then set STATE_PORT_SECURE.
		*/
		PortSecured = STATE_CONNECTED;
#else /* FAST_EAPOL_WAR */
		PortSecured = STATE_PORT_SECURE;
		CheckBMCPortSecured(pAd, pEntry, TRUE);
#endif /* !FAST_EAPOL_WAR */
	}

#ifdef TXBF_SUPPORT
    if ((pAd->CommonCfg.ETxBfEnCond == TRUE) || 
        (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE))
    {   
        if ((!IS_ENTRY_NONE(pEntry)) && IS_ENTRY_CLIENT(pEntry))
        {
            if (HcIsBfCapSupport(wdev))
            {                      
                pAd->fgApClientMode = FALSE;	
                enableFeature      |= STA_REC_BF_FEATURE;
            }
        }
    }
#endif /* TXBF_SUPPORT */

	sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
	sta_ctrl->WlanIdx = pEntry->wcid;
	sta_ctrl->ConnectionType = pEntry->ConnectionType;
	sta_ctrl->ConnectionState = PortSecured;
	sta_ctrl->EnableFeature = enableFeature;
	sta_ctrl->IsNewSTARec = TRUE;
	wifi_sys_ctrl.skip_set_txop = TRUE;/* there is no need to set txop when sta connected.*/
	WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);
	
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d\n",
		__FUNCTION__, pEntry->wcid, PortSecured));

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("===> AsicStaRecUpdate called by (%s), AKM=0x%x\n", __FUNCTION__, pEntry->SecConfig.AKMMap));

	os_alloc_mem(NULL,(UCHAR**)&lu_ctrl,sizeof(PEER_LINKUP_HWCTRL));
	os_zero_mem(lu_ctrl,sizeof(PEER_LINKUP_HWCTRL));
 
#ifdef DOT11_N_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE))
	{
		lu_ctrl->bRdgCap = TRUE;
	}
#endif /*DOT11_N_SUPPORT*/

#ifdef TXBF_SUPPORT	
    os_move_mem(&lu_ctrl->ie_list,ie_list,sizeof(IE_LISTS));
#endif /* TXBF_SUPPORT */

	wifi_sys_ctrl.priv = (VOID*)lu_ctrl;
	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_PEER_LINKUP(pAd,wifi_sys_ctrl);

#if defined(MT7615) || defined(MT7637) || defined(MT7622)
    if (IS_MT7615(pAd) || (IS_MT7637(pAd)) || IS_MT7622(pAd))
    {
		    RAInit(pAd, pEntry);
    }
#endif /* defined(MT7615) || defined(MT7637) || defined(MT7622) */


#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) && defined(TXBF_SUPPORT) && defined(MT7615)
/* Disable TXBF apply for DFS zero wait start */
if ((pAd->BgndScanCtrl.BgndScanStatMachine.CurrState == BGND_RDD_DETEC)
    && IS_SUPPORT_MT_ZEROWAIT_DFS(pAd))
{
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s][ZeroWait]In CAC period, Disable TXBF Apply \n", __FUNCTION__));
    HW_AP_TXBF_TX_APPLY(pAd, FALSE);
}
#endif 

}

#ifdef WH_EZ_SETUP

VOID WifiSysApPeerChBwUpdate(RTMP_ADAPTER *pAd,MAC_TABLE_ENTRY *pEntry)//, IE_LISTS *ie_list)  Rakesh: Todo check actual peer cap
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	UINT32 enableFeature = 0;
	UCHAR PortSecured = STATE_DISCONNECT;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	struct wifi_dev *wdev = pEntry->wdev;
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;
	
	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));
	
	/* Jeffrey 2015.3.9, OR features instead */
#if defined(MT7615) || defined(MT7622)
	/* Jeffrey 2015.3.9, Sync HT info to F/W */
#ifdef DOT11_N_SUPPORT
	if ( //(ie_list->ht_cap_len > 0) &&     Rakesh: todo check actual peer cap
		WMODE_CAP_N(wdev->PhyMode))
	{
		enableFeature |= STA_REC_BASIC_HT_INFO_FEATURE;
	}
#endif

	/* Jeffrey 2015.3.9, Sync VHT info to F/W */
#ifdef DOT11_VHT_AC
	if ( //(ie_list->vht_cap_len > 0) && 	Rakesh: todo check actual peer cap
		WMODE_CAP_AC(wdev->PhyMode))
	{
		enableFeature |= STA_REC_BASIC_VHT_INFO_FEATURE;
	}
#endif
#endif /* defined(MT7615) || defined(MT7622) */

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		&& (IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
		|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
		|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
		|| pEntry->bWscCapable))
	{
		PortSecured = STATE_CONNECTED;
	} else 
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
	{
		PortSecured = STATE_PORT_SECURE;
		CheckBMCPortSecured(pAd, pEntry, TRUE);
	}

	sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
	sta_ctrl->WlanIdx = pEntry->wcid;
	sta_ctrl->ConnectionType = pEntry->ConnectionType;
	sta_ctrl->ConnectionState = PortSecured;
	sta_ctrl->EnableFeature = enableFeature;
	sta_ctrl->IsNewSTARec = FALSE;	// Rakesh: set False as updating existing record
	wifi_sys_ctrl.skip_set_txop = TRUE;/* there is no need to set txop when sta connected.*/
	WifiSysUpdateStaRec(pAd,tr_entry, sta_ctrl);
	
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d, AKM=0x%x\n",
		__FUNCTION__, pEntry->wcid, PortSecured, pEntry->SecConfig.AKMMap));

	wifi_sys_ctrl.priv = NULL;
	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_PEER_LINKUP(pAd,wifi_sys_ctrl);

#if defined(MT7615) || defined(MT7637) || defined(MT7622)
    if (IS_MT7615(pAd) || (IS_MT7637(pAd)) || IS_MT7622(pAd))
    {
		    RAInit(pAd, pEntry);
    }
#endif /* defined(MT7615) || defined(MT7637) || defined(MT7622) */

}
#endif

#ifdef WDS_SUPPORT
/*
*
*/
VOID WifiSysWdsLinkUp(RTMP_ADAPTER *pAd,struct wifi_dev *wdev, UCHAR wcid)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
    	UINT32 enableFeature =
        	STA_REC_BASIC_STA_RECORD_FEATURE | STA_REC_TX_PROC_FEATURE;
	MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[wcid];
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pMacEntry->tr_tb_idx];
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;
	
	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), wcid = %d\n", 
		__FUNCTION__, wcid));

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

	/*update bssinfo*/
	if(WDEV_BSS_STATE(wdev) == BSS_INIT)
	{
		BssInfoArgumentLinker(pAd, wdev);

		WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
		wdev->bss_info_argument.ucBcMcWlanIdx = wcid;
		wdev->bss_info_argument.u4BssInfoFeature =(BSS_INFO_OWN_MAC_FEATURE |
											BSS_INFO_BASIC_FEATURE |
											BSS_INFO_BROADCAST_INFO_FEATURE |
											BSS_INFO_RF_CH_FEATURE);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (pAd->chipCap.fgRateAdaptFWOffload == TRUE)
			wdev->bss_info_argument.u4BssInfoFeature |= BSS_INFO_RA_FEATURE;
#endif

		WifiSysUpdateBssInfo(pAd,wdev,&wifi_sys_ctrl.BssInfoCtrl);
	}

	/*update sta rec*/
	if (pAd->chipCap.SupportAMSDU == TRUE) {
		enableFeature |= STA_REC_AMSDU_FEATURE;
	}

#ifdef DOT11_N_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_HT_CAPABLE))
	{
		enableFeature |= STA_REC_BASIC_HT_INFO_FEATURE;
	}
#endif /*DOT11_N_SUPPORT*/

#ifdef DOT11_VHT_AC
	if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_VHT_CAPABLE))
	{
		enableFeature |= STA_REC_BASIC_VHT_INFO_FEATURE;
	}
#endif /*DOT11_VHT_AC*/

	sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
	sta_ctrl->WlanIdx = wcid;
	sta_ctrl->ConnectionType = CONNECTION_WDS;
	sta_ctrl->ConnectionState = STATE_PORT_SECURE;
	sta_ctrl->EnableFeature = enableFeature;
	sta_ctrl->IsNewSTARec = TRUE;
	
	WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d\n",
		__FUNCTION__, wcid, STATE_PORT_SECURE));

	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_LINKUP(pAd,wifi_sys_ctrl);
}


/*
*
*/
VOID WifiSysWdsLinkDown(RTMP_ADAPTER *pAd,struct wifi_dev *wdev, UCHAR wcid)
{

	WIFI_SYS_CTRL wifi_sys_ctrl;
	MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[wcid];
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pMacEntry->tr_tb_idx];
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;
	
	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), wcid = %d\n", 
		__FUNCTION__, wcid));

	if(tr_entry->StaRec.ConnectionState)
	{
		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->WlanIdx = wcid;
		sta_ctrl->ConnectionType = CONNECTION_WDS;
		sta_ctrl->ConnectionState = STATE_DISCONNECT;
		sta_ctrl->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;

		WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
		("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d\n",
		__FUNCTION__, wcid, STATE_DISCONNECT));
	}

	if(WDEV_BSS_STATE(wdev) >= BSS_INITED)
	{
	    WDEV_BSS_STATE(wdev) = BSS_INITED;
		WifiSysUpdateBssInfo(pAd,wdev,&wifi_sys_ctrl.BssInfoCtrl);
	}
	
	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_LINKDOWN(pAd,wifi_sys_ctrl);
}
#endif /*WDS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/




#ifdef APCLI_SUPPORT
/*
*
*/
VOID WifiSysApCliLinkUp(RTMP_ADAPTER *pAd,APCLI_STRUCT *pApCliEntry,UCHAR CliIdx, MAC_TABLE_ENTRY *pMacEntry)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	struct wifi_dev *wdev = &pApCliEntry->wdev;
	UCHAR PortSecured = STATE_DISCONNECT;
    UINT32 enableFeature = 0;
#ifdef MAC_REPEATER_SUPPORT
    RTMP_CHIP_CAP *cap = &pAd->chipCap;
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /*MAC_REPEATER*/
	STA_TR_ENTRY *tr_entry;
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
    {
        if (CliIdx > GET_MAX_REPEATER_ENTRY_NUM(cap))
        {
            MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                ("(%s) CliIdx:%d is incorrect index.\n", __FUNCTION__, CliIdx));
            return;
        }

        pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		tr_entry = &pAd->MacTab.tr_entry[pReptEntry->MacTabWCID];

		//TODO: Carter, check with Eddy.
		if (IS_NO_SECURITY_Entry(pMacEntry) || IS_CIPHER_WEP_Entry(pMacEntry))
		{
 			PortSecured = STATE_PORT_SECURE;
		}
		else
		{
 			PortSecured = STATE_CONNECTED;
		}

        enableFeature |= STA_REC_BASIC_STA_RECORD_FEATURE | STA_REC_TX_PROC_FEATURE;

        if (pAd->chipCap.SupportAMSDU == TRUE) {
            enableFeature |= STA_REC_AMSDU_FEATURE;
        }

#if defined(MT7615) || defined(MT7622)
#ifdef DOT11_N_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_HT_CAPABLE))
		{
			enableFeature |= STA_REC_BASIC_HT_INFO_FEATURE;
		}
#endif /*DOT11_N_SUPPORT*/

#ifdef DOT11_VHT_AC
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_VHT_CAPABLE))
		{
			enableFeature |= STA_REC_BASIC_VHT_INFO_FEATURE;
		}
#endif /*DOT11_VHT_AC*/

		enableFeature |= STA_REC_TX_PROC_FEATURE;
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef TXBF_SUPPORT
        if ((pAd->CommonCfg.ETxBfEnCond == TRUE) || 
            (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE))
        {
            if ((!IS_ENTRY_NONE(pMacEntry)) && 
               (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry) || IS_ENTRY_AP(pMacEntry) || IS_ENTRY_CLIENT(pMacEntry)))
            {
                if (HcIsBfCapSupport(wdev))
                {   
#ifdef APCLI_SUPPORT            
	                pAd->fgApClientMode = TRUE;	  

	                if (pAd->fgApCliBfStaRecRegister == FALSE)
	                {
	                    pAd->fgApCliBfStaRecRegister   = TRUE;
	                    pAd->ApCli_CmmWlanId = pReptEntry->MacTabWCID;
	                    enableFeature       |= STA_REC_BF_FEATURE;
	                }
#endif /* APCLI_SUPPORT */                    
                }
            }
        }
#endif /* TXBF_SUPPORT */		

		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->WlanIdx = pReptEntry->MacTabWCID;
		sta_ctrl->ConnectionType = CONNECTION_INFRA_AP;
		sta_ctrl->ConnectionState = PortSecured;
		sta_ctrl->EnableFeature = enableFeature;
		sta_ctrl->IsNewSTARec = TRUE;
		
		WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);
		
    }
    else
#endif /* MAC_REPEATER_SUPPORT */
	{
		if(WDEV_BSS_STATE(wdev) == BSS_INIT)
		{
			HcAcquiredEdca(pAd,wdev,&pApCliEntry->MlmeAux.APEdcaParm);
	        BssInfoArgumentLinker(pAd,wdev);
			pApCliEntry->Valid = TRUE;
			MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
			wdev->PortSecured = WPA_802_1X_PORT_SECURED;

			WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
			wdev->bss_info_argument.ucPeerWlanIdx = pApCliEntry->MacTabWCID;

			wdev->bss_info_argument.u4BssInfoFeature =
									(BSS_INFO_OWN_MAC_FEATURE |
									BSS_INFO_BASIC_FEATURE |
									BSS_INFO_RF_CH_FEATURE |
									BSS_INFO_BROADCAST_INFO_FEATURE |
									BSS_INFO_SYNC_MODE_FEATURE);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (pAd->chipCap.fgRateAdaptFWOffload == TRUE)
			wdev->bss_info_argument.u4BssInfoFeature |= BSS_INFO_RA_FEATURE;
#endif

			WifiSysUpdateBssInfo(pAd,wdev,&wifi_sys_ctrl.BssInfoCtrl);
		}
		/*sta rec update*/
		if (IS_NO_SECURITY_Entry(pMacEntry) || IS_CIPHER_WEP_Entry(pMacEntry))
		{
 			PortSecured = STATE_PORT_SECURE;
		}
		else
		{
 			PortSecured = STATE_CONNECTED;
		}

        enableFeature |= STA_REC_BASIC_STA_RECORD_FEATURE | STA_REC_TX_PROC_FEATURE;

#if defined(MT7615) || defined(MT7622)
#ifdef DOT11_N_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_HT_CAPABLE))
		{
			enableFeature |= STA_REC_BASIC_HT_INFO_FEATURE;
		}
#endif /*DOT11_N_SUPPORT*/

#ifdef DOT11_VHT_AC
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_VHT_CAPABLE))
		{
			enableFeature |= STA_REC_BASIC_VHT_INFO_FEATURE;
		}
#endif /*DOT11_VHT_AC*/

		enableFeature |= STA_REC_TX_PROC_FEATURE;
#endif /* defined(MT7615) || defined(MT7622) */

	    if (pAd->chipCap.SupportAMSDU == TRUE) {
	        enableFeature |= STA_REC_AMSDU_FEATURE;
	    }

#ifdef TXBF_SUPPORT
        if ((pAd->CommonCfg.ETxBfEnCond == TRUE) || 
            (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE))
        {
            if ((!IS_ENTRY_NONE(pMacEntry)) && 
               (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry) || IS_ENTRY_AP(pMacEntry)))
            {
                if (HcIsBfCapSupport(wdev))
                {   
#ifdef APCLI_SUPPORT            
	                pAd->fgApClientMode = TRUE;	  

	                if (pAd->fgApCliBfStaRecRegister == FALSE)
	                {
	                    pAd->fgApCliBfStaRecRegister   = TRUE;
	                    pAd->ApCli_CmmWlanId = pMacEntry->wcid;
	                    enableFeature       |= STA_REC_BF_FEATURE; 
	                }
                    else
#endif /* APCLI_SUPPORT */                    
                    {
                        enableFeature |= STA_REC_BF_FEATURE;
                    }
                }
            }
        }
#endif /* TXBF_SUPPORT */	    
		
		tr_entry = &pAd->MacTab.tr_entry[pMacEntry->tr_tb_idx];

		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->WlanIdx = pMacEntry->wcid;
		sta_ctrl->ConnectionType = CONNECTION_INFRA_AP;
		sta_ctrl->ConnectionState = PortSecured;
		sta_ctrl->EnableFeature = enableFeature;
		sta_ctrl->IsNewSTARec = TRUE;
		
		WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d\n",
				__FUNCTION__, pMacEntry->wcid, PortSecured));
	}

	/*update to hw ctrl*/
	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_LINKUP(pAd,wifi_sys_ctrl);
	
}


/*
*
*/
VOID WifiSysApCliLinkDown(RTMP_ADAPTER *pAd,APCLI_STRUCT *pApCliEntry,UCHAR CliIdx)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	struct wifi_dev *wdev = &pApCliEntry->wdev;
	MAC_TABLE_ENTRY *pEntry=&pAd->MacTab.Content[pApCliEntry->MacTabWCID];

#ifdef MAC_REPEATER_SUPPORT
    RTMP_CHIP_CAP *cap = &pAd->chipCap;
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /*MAC_REPEATER_SUPPORT*/

	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		if (CliIdx > GET_MAX_REPEATER_ENTRY_NUM(cap))
		{
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			("(%s) CliIdx:%d is incorrect index.\n", __FUNCTION__, CliIdx));
			return;
		}
	        pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		tr_entry = &pAd->MacTab.tr_entry[pReptEntry->MacTabWCID];

		if(tr_entry->StaRec.ConnectionState)
		{
			sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
			sta_ctrl->WlanIdx = pReptEntry->MacTabWCID;
			sta_ctrl->ConnectionType = CONNECTION_INFRA_AP;
			sta_ctrl->ConnectionState = STATE_DISCONNECT;
			sta_ctrl->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;
			WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);
		}
		wifi_sys_ctrl.skip_set_txop = TRUE;/* skip disable txop for repeater case since apcli is connected */
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{

		if (WDEV_BSS_STATE(wdev) >= BSS_INITED)
		{
			tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
			
			if(tr_entry->StaRec.ConnectionState)
			{
				/*update sta rec*/
				sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
				sta_ctrl->WlanIdx = pApCliEntry->MacTabWCID;
				sta_ctrl->ConnectionType = CONNECTION_INFRA_AP;
				sta_ctrl->ConnectionState = STATE_DISCONNECT;
				sta_ctrl->EnableFeature = STA_REC_BASIC_STA_RECORD_FEATURE;			
				WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);
			}
			/*update bssinfo*/
			WDEV_BSS_STATE(wdev) = BSS_INITED;
			wdev->bss_info_argument.u4BssInfoFeature = (BSS_INFO_OWN_MAC_FEATURE |
							                BSS_INFO_BASIC_FEATURE |
							                BSS_INFO_SYNC_MODE_FEATURE);
			
			WifiSysUpdateBssInfo(pAd,wdev,&wifi_sys_ctrl.BssInfoCtrl);
		}
	}

	/*update to hw ctrl*/
	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_LINKDOWN(pAd,wifi_sys_ctrl);
}

#ifdef WH_EZ_SETUP
VOID WifiSysApCliChBwUpdate(RTMP_ADAPTER *pAd,APCLI_STRUCT *pApCliEntry,UCHAR CliIdx, MAC_TABLE_ENTRY *pMacEntry) // Rakesh : refer WifiSysApCliLinkUp
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	struct wifi_dev *wdev = &pApCliEntry->wdev;
	UCHAR PortSecured = STATE_DISCONNECT;
    UINT32 enableFeature = 0;
#ifdef MAC_REPEATER_SUPPORT
    //RTMP_CHIP_CAP *cap = &pAd->chipCap;
    //REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /*MAC_REPEATER*/
	STA_TR_ENTRY *tr_entry;
	STA_REC_CTRL_T *sta_ctrl;

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
    {
		// Rakesh: not handling this case		
    }
    else
#endif /* MAC_REPEATER_SUPPORT */
	{
		/*sta rec update*/
		if (IS_NO_SECURITY_Entry(pMacEntry) || IS_CIPHER_WEP_Entry(pMacEntry))
		{
 			PortSecured = STATE_PORT_SECURE;
		}
		else
		{
 			PortSecured = STATE_CONNECTED;
		}

#if defined(MT7615) || defined(MT7622)
#ifdef DOT11_N_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_HT_CAPABLE))
		{
			enableFeature |= STA_REC_BASIC_HT_INFO_FEATURE;
		}
#endif /*DOT11_N_SUPPORT*/

#ifdef DOT11_VHT_AC
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_VHT_CAPABLE))
		{
			enableFeature |= STA_REC_BASIC_VHT_INFO_FEATURE;
		}
#endif /*DOT11_VHT_AC*/
#endif
		tr_entry = &pAd->MacTab.tr_entry[pMacEntry->tr_tb_idx];
		sta_ctrl = &tr_entry->StaRec;

		sta_ctrl->BssIndex = wdev->bss_info_argument.ucBssIndex;
		sta_ctrl->WlanIdx = pMacEntry->wcid;
		sta_ctrl->ConnectionType = CONNECTION_INFRA_AP;
		sta_ctrl->ConnectionState = PortSecured;
		sta_ctrl->EnableFeature = enableFeature;
		sta_ctrl->IsNewSTARec = FALSE; // Rakesh: as existing peer record is being updated, use FALSE
		
		WifiSysUpdateStaRec(pAd,tr_entry,&wifi_sys_ctrl.StaRecCtrl);

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				("===> AsicStaRecUpdate called by (%s), wcid=%d, PortSecured=%d\n",
				__FUNCTION__, pMacEntry->wcid, PortSecured));
	}

	/*update to hw ctrl*/
	wifi_sys_ctrl.wdev = wdev;
	HW_WIFISYS_LINKUP(pAd,wifi_sys_ctrl);
	
}
#endif

#endif /*APCLI_SUPPORT*/


#ifdef RT_CFG80211_SUPPORT
#ifdef CFG_TDLS_SUPPORT
/*
*
*/
VOID WifiSysTdlsPeerLinkDown(RTMP_ADAPTER *pAd,MAC_TABLE_ENTRY *pEntry)
{

	WIFI_SYS_CTRL wifi_sys_ctrl;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;


#if defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622)
    UINT32 enableFeature =
        STA_REC_BASIC_STA_RECORD_FEATURE | STA_REC_TX_PROC_FEATURE;
    UCHAR PortSecured = STATE_DISCONNECT;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));
	
    if (pAd->chipCap.SupportAMSDU == TRUE) {
        enableFeature |= STA_REC_AMSDU_FEATURE;
    }

    if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
        && (IS_AKM_WPA_CAPABILITY_Entry(pEntry)))
    {
        PortSecured = STATE_CONNECTED;
    } else if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
        PortSecured = STATE_PORT_SECURE;

#ifdef DOT11_N_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_HT_CAPABLE))
	{
		enableFeature |= STA_REC_BASIC_HT_INFO_FEATURE;
	}
#endif /*DOT11_N_SUPPORT*/

#ifdef DOT11_VHT_AC
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_CAPABLE))
	{
		enableFeature |= STA_REC_BASIC_VHT_INFO_FEATURE;
	}
#endif /*DOT11_VHT_AC*/

	sta_ctrl->BssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	sta_ctrl->WlanIdx = pEntry->wcid;
	sta_ctrl->ConnectionState = PortSecured;
	sta_ctrl->ConnectionType = pEntry->ConnectionType;
	sta_ctrl->EnableFeature = enableFeature;

	WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR,("%s() - AsicStaRecUpdate WCID: %d\n", __FUNCTION__,pEntry->wcid));

	wifi_sys_ctrl.wdev = pEntry->wdev;
	HW_WIFISYS_PEER_LINKDOWN(pAd,wifi_sys_ctrl);
#else
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO,("%s() Not support this function\n"),__FUNCTION__);
#endif /* MT7636 || MT7615 || MT7637 || MT7622 */
}
#endif
#endif




#ifdef RACTRL_FW_OFFLOAD_SUPPORT
/*
*
*/
VOID WifiSysRaInit(RTMP_ADAPTER *pAd,MAC_TABLE_ENTRY *pEntry)
{
	WIFI_SYS_CTRL wifi_sys_ctrl;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;

	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

	sta_ctrl->BssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	sta_ctrl->WlanIdx = pEntry->wcid;
	sta_ctrl->ConnectionType = pEntry->ConnectionType;
	sta_ctrl->ConnectionState = STATE_CONNECTED;
	//sta_rec->EnableFeature = STA_REC_RA_COMMON_INFO_FEATURE | STA_REC_RA_FEATURE;
	sta_ctrl->EnableFeature = STA_REC_RA_FEATURE;
	wifi_sys_ctrl.wdev = pEntry->wdev;

	WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);

	HW_WIFISYS_PEER_UPDATE(pAd,wifi_sys_ctrl);
}


/*
*
*/
VOID WifiSysUpdateRa(RTMP_ADAPTER *pAd,
    MAC_TABLE_ENTRY *pEntry,
    P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam
)
{
	
	WIFI_SYS_CTRL wifi_sys_ctrl;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	STA_REC_CTRL_T *sta_ctrl = &wifi_sys_ctrl.StaRecCtrl;
	CMD_STAREC_AUTO_RATE_UPDATE_T *ra_parm=NULL;
	
	os_zero_mem(&wifi_sys_ctrl,sizeof(WIFI_SYS_CTRL));

	sta_ctrl->BssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
	sta_ctrl->WlanIdx = pEntry->wcid;
	sta_ctrl->ConnectionType = pEntry->ConnectionType;
	sta_ctrl->ConnectionState = STATE_CONNECTED;
	sta_ctrl->EnableFeature = STA_REC_RA_UPDATE_FEATURE;

	WifiSysUpdateStaRec(pAd, tr_entry, sta_ctrl);

	os_alloc_mem(NULL,(UCHAR**)&ra_parm,sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
	os_move_mem(ra_parm,prParam,sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
	
	wifi_sys_ctrl.priv = (VOID*)ra_parm;
	wifi_sys_ctrl.wdev = pEntry->wdev;

	HW_WIFISYS_PEER_UPDATE(pAd,wifi_sys_ctrl);

}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/



