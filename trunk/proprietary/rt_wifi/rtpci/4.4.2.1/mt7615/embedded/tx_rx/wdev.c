/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"

/**
 * @addtogroup wifi_dev_system
 * @{
 * @name core API
 * @{
 */

struct wifi_dev* get_default_wdev(struct _RTMP_ADAPTER *ad)
{
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(ad->OpMode)
	{
		return &ad->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
	return NULL;
}

INT rtmp_wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;

	if (!wdev)
		return -1;
	
#ifdef WH_EZ_SETUP
	if((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_APCLI))
		ez_exit(wdev);
#endif /* WH_EZ_SETUP */

	NdisAcquireSpinLock(&pAd->WdevListLock);
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("unregister wdev(type:%d, idx:%d) from wdev_list\n",
					wdev->wdev_type, wdev->wdev_idx));
			pAd->wdev_list[idx] = NULL;
			wdev->wdev_idx = WDEV_NUM_MAX;
			break;
		}
	}

	if (idx == WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Cannot found wdev(%p, type:%d, idx:%d) in wdev_list\n",
					wdev, wdev->wdev_type, wdev->wdev_idx));
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump wdev_list:\n"));
		for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Idx %d: 0x%p\n", idx, pAd->wdev_list[idx]));
		}
	}
	NdisReleaseSpinLock(&pAd->WdevListLock);

	return ((idx < WDEV_NUM_MAX) ? 0 : -1);

}


INT32 rtmp_wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 idx;

	if (!wdev)
		return -1;

	NdisAcquireSpinLock(&pAd->WdevListLock);
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("wdev(type:%d) already registered and idx(%d) %smatch\n",
					wdev->wdev_type, wdev->wdev_idx,
					((idx != wdev->wdev_idx) ? "mis" : "")));
			break;
		}

		if (pAd->wdev_list[idx] == NULL) {
			pAd->wdev_list[idx] = wdev;
#ifdef MT_MAC
            RxTrackingInit(wdev);
#endif
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Assign wdev_idx=%d with OmacIdx = %d\n",
																	__FUNCTION__,
																	idx,
																	wdev->OmacIdx));
			break;
		}
	}

	wdev->wdev_idx = idx;

	if (idx < WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Assign wdev_idx=%d\n", idx));
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);

	return ((idx < WDEV_NUM_MAX) ? idx : -1);
}


static INT32 GetBssIdx(RTMP_ADAPTER *pAd)
{
    UINT32 BssInfoIdxBitMap;
    UCHAR i;
    INT32 no_usable_entry = -1;
#ifdef MAC_REPEATER_SUPPORT
    RTMP_CHIP_CAP *cap = &pAd->chipCap;
#endif
    UCHAR BssInfoMax = BSSINFO_NUM_MAX(cap);

    NdisAcquireSpinLock(&pAd->BssInfoIdxBitMapLock);
	BssInfoIdxBitMap = pAd->BssInfoIdxBitMap0;
    for (i = 0; i < 32; i++)
    {
        /* find the first 0 bitfield, then return the bit idx as BssInfoIdx. */
        if ((BssInfoIdxBitMap & (1 << i)) == 0)
        {
            pAd->BssInfoIdxBitMap0 = (BssInfoIdxBitMap | (1 << i));
            MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                    ("%s: found non-used BssInfoIdx: %d\n", __FUNCTION__, i));
            NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
            return i;
        }
    }

    BssInfoIdxBitMap = pAd->BssInfoIdxBitMap1;
    for (i = 32; i < BssInfoMax; i++)
    {
        /* find the first 0 bitfield, then return the bit idx as BssInfoIdx. */
        if ((BssInfoIdxBitMap & (1 << (i-32))) == 0)
        {
            pAd->BssInfoIdxBitMap1 = (BssInfoIdxBitMap | (1 << (i-32)));
            MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                    ("%s: found non-used BssInfoIdx: %d\n", __FUNCTION__, i));
            NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
            return i;
        }
    }

    NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);

    if (i >= BssInfoMax)
    {
        MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s: could not find usable BssInfoIdx\n", __FUNCTION__));
        return no_usable_entry;
    }

    return no_usable_entry;
}

VOID ReleaseBssIdx(RTMP_ADAPTER *pAd, UINT32 BssIdx)
{
    NdisAcquireSpinLock(&pAd->BssInfoIdxBitMapLock);
    if (BssIdx < 32)
    {
        pAd->BssInfoIdxBitMap0 = pAd->BssInfoIdxBitMap0 & (0xffffffff & ~(1 << BssIdx));
    }
    else
    {
        pAd->BssInfoIdxBitMap1 = pAd->BssInfoIdxBitMap1 & (0xffffffff & ~(1 << (BssIdx - 32)));
    }
    NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
}

VOID BssInfoArgumentLinker(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
    BSS_INFO_ARGUMENT_T *pbss_info_argument = &wdev->bss_info_argument;
	HTTRANSMIT_SETTING HTPhyMode;

    pbss_info_argument->OwnMacIdx = wdev->OmacIdx;
    pbss_info_argument->ucBssIndex = GetBssIdx(pAd);

    os_move_mem(pbss_info_argument->Bssid,wdev->bssid,MAC_ADDR_LEN);
#ifdef NEWSEC
    pbss_info_argument->CipherSuit = SecHWCipherSuitMapping(wdev->SecConfig.PairwiseCipher);
#endif
    pbss_info_argument->WmmIdx = HcGetWmmIdx(pAd,wdev);
    switch (wdev->wdev_type){
    case WDEV_TYPE_STA:
	case WDEV_TYPE_APCLI:
        pbss_info_argument->bssinfo_type = HW_BSSID;
        pbss_info_argument->NetworkType = NETWORK_INFRA;
        pbss_info_argument->u4ConnectionType = CONNECTION_INFRA_STA;
        pbss_info_argument->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(pAd, wdev);
		TRTableInsertMcastEntry(pAd,pbss_info_argument->ucBcMcWlanIdx, wdev);
        break;
    case WDEV_TYPE_ADHOC:
        pbss_info_argument->bssinfo_type = HW_BSSID;
        pbss_info_argument->NetworkType = NETWORK_IBSS;
        pbss_info_argument->u4ConnectionType = CONNECTION_IBSS_ADHOC;
        pbss_info_argument->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(pAd, wdev);
        break;
    case WDEV_TYPE_WDS:
        pbss_info_argument->bssinfo_type = WDS;
        pbss_info_argument->NetworkType = NETWORK_WDS;
        pbss_info_argument->u4ConnectionType = CONNECTION_WDS;
        break;
    case WDEV_TYPE_GO:
        pbss_info_argument->bssinfo_type = HW_BSSID;
        pbss_info_argument->NetworkType = NETWORK_INFRA;
        pbss_info_argument->u4ConnectionType = CONNECTION_P2P_GO;
        /* Get a specific WCID to record this MBSS key attribute */
		pbss_info_argument->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(pAd, wdev);
        TRTableInsertMcastEntry(pAd, pbss_info_argument->ucBcMcWlanIdx, wdev);
        MgmtTableSetMcastEntry(pAd, pbss_info_argument->ucBcMcWlanIdx);
        break;
    case WDEV_TYPE_GC:
        pbss_info_argument->bssinfo_type = HW_BSSID;
        pbss_info_argument->NetworkType = NETWORK_P2P;
        pbss_info_argument->u4ConnectionType = CONNECTION_P2P_GC;
        pbss_info_argument->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(pAd, wdev);
        break;
    case WDEV_TYPE_AP:
    default:
        /* Get a specific WCID to record this MBSS key attribute */
        pbss_info_argument->bssinfo_type = HW_BSSID;
        pbss_info_argument->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(pAd, wdev);
        TRTableInsertMcastEntry(pAd, wdev->bss_info_argument.ucBcMcWlanIdx, wdev);
        MgmtTableSetMcastEntry(pAd, pbss_info_argument->ucBcMcWlanIdx);
        pbss_info_argument->NetworkType = NETWORK_INFRA;
        pbss_info_argument->u4ConnectionType = CONNECTION_INFRA_AP;
        break;
    }

	/* Get a specific Tx rate for BMcast frame */
    os_zero_mem(&HTPhyMode, sizeof(HTTRANSMIT_SETTING));
    
    if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
        (wdev->channel <= 14))
    {
        HTPhyMode.field.MODE = MODE_CCK;
        HTPhyMode.field.BW = BW_20;
        HTPhyMode.field.MCS = RATE_1;
    }
    else
    {
        HTPhyMode.field.MODE = MODE_OFDM;
        HTPhyMode.field.BW = BW_20;
        HTPhyMode.field.MCS = MCS_RATE_6;
    }

#ifdef MCAST_RATE_SPECIFIC
    if(wdev->channel > 14)
        wdev->bss_info_argument.McTransmit = pAd->CommonCfg.MCastPhyMode_5G;
    else
        wdev->bss_info_argument.McTransmit = pAd->CommonCfg.MCastPhyMode;
#else
    wdev->bss_info_argument.McTransmit = HTPhyMode;
#endif /* MCAST_RATE_SPECIFIC */

	wdev->bss_info_argument.BcTransmit = HTPhyMode;

    WDEV_BSS_STATE(wdev) = BSS_INITED;
}


VOID BssInfoArgumentUnLink(RTMP_ADAPTER *pAd,struct wifi_dev *wdev)
{
		ReleaseBssIdx(pAd, wdev->bss_info_argument.ucBssIndex);
        HcReleaseGroupKeyWcid(pAd, wdev, wdev->bss_info_argument.ucBcMcWlanIdx);

		WDEV_BSS_STATE(wdev) = BSS_INIT;
}

extern INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
extern INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
/**
 * @param pAd
 * @param wdev wifi device
 * @param wdev_type wifi device type
 * @param IfDev pointer to interface NET_DEV
 * @param func_idx  _STA_TR_ENTRY index for BC/MC packet
 * @param func_dev function device
 * @param sys_handle pointer to pAd
 *
 * Initialize a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum WDEV_TYPE WdevType,
				PNET_DEV IfDev, INT8 func_idx, VOID *func_dev, VOID *sys_handle)
{
	INT32 wdev_idx = 0;

	wdev->wdev_type = WdevType;
	wdev->if_dev = IfDev;
	wdev->func_idx = func_idx;
	wdev->func_dev = func_dev;
	wdev->sys_handle = sys_handle;
	wdev->tr_tb_idx = 0xff;//init value.
	wdev->OpStatusFlags = 0;
	wdev->forbid_data_tx = 0x1 << MSDU_FORBID_CONNECTION_NOT_READY;//init value.
	wdev->bAllowBeaconing = FALSE;
	wdev_protect_init(wdev);
	wlan_operate_init(wdev);
    //wdev->bss_info_argument.ucBssIndex = 0xff;

	switch (wdev->wdev_type)
	{
#ifdef CONFIG_AP_SUPPORT
		case WDEV_TYPE_AP:
		case WDEV_TYPE_GO:
			wdev->tx_pkt_allowed = ApAllowToSendPacket;
			wdev->tx_pkt_handle = APSendPacket;
			wdev->wdev_hard_tx = APHardTransmit;
			wdev->rx_pkt_allowed = APRxPktAllow;
			wdev->rx_ps_handle = APRxPsHandle;
			wdev->rx_pkt_foward = APRxFowardHandle;
			break;
#endif /* CONFIG_AP_SUPPORT */

#ifdef WDS_SUPPORT
		case WDEV_TYPE_WDS:
			wdev->tx_pkt_allowed = ApWdsAllowToSendPacket;
			// TODO: shiang-usw, modify this to WDSSendPacket
			wdev->tx_pkt_handle = APSendPacket;
			wdev->wdev_hard_tx = APHardTransmit;
			wdev->rx_pkt_allowed = APRxPktAllow;
			wdev->rx_pkt_foward = wds_rx_foward_handle;
			break;
#endif



#ifdef APCLI_SUPPORT
		case WDEV_TYPE_APCLI:
			wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
			// TODO: shiang-usw, modify this to STASendPacket!
			wdev->tx_pkt_handle = APSendPacket;
			wdev->wdev_hard_tx = APHardTransmit;
			wdev->rx_pkt_allowed = sta_rx_pkt_allow;
			wdev->rx_pkt_foward = sta_rx_fwd_hnd;
			wdev->func_type = OMAC_TYPE_APCLI;

			/* init MAC address of virtual network interface */
			COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
			break;
#endif

#ifdef RT_CFG80211_SUPPORT
		case WDEV_TYPE_P2P_DEVICE:
			// TODO: check if need to register data tx rx callback
			break;
#endif

		default:
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknow wdev type(%d)\n",
						__FUNCTION__, wdev->wdev_type));
			return FALSE;
	}

#ifdef CUT_THROUGH
#ifdef CUT_THROUGH_FULL_OFFLOAD
	wdev->tx_pkt_ct_handle = FullOffloadFrameTx;
#endif /* CUT_THROUGH_FULL_OFFLOAD */
#endif /* CUT_THROUGH */

	wdev_idx = rtmp_wdev_idx_reg(pAd, wdev);

	if (wdev_idx < 0)
		return FALSE;

#ifdef WH_EZ_SETUP
	if (wdev->wdev_type == WDEV_TYPE_AP)
		ez_init(pAd, wdev, TRUE);
	else if (wdev->wdev_type == WDEV_TYPE_APCLI)
		ez_init(pAd, wdev, FALSE);
#endif /* WH_EZ_SETUP */
	return TRUE;
}



INT32 wdev_attr_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	switch (wdev->wdev_type){
#ifdef CONFIG_AP_SUPPORT
	case WDEV_TYPE_AP:
	case WDEV_TYPE_GO:
		AsicSetMbssWdevIfAddr(pAd, wdev->func_idx, (UCHAR *)wdev->if_addr, OPMODE_AP);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): wdevId%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__,wdev->wdev_idx,PRINT_MAC(wdev->if_addr)));

		if (wdev->if_dev)
		{
			NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev),
				wdev->if_addr, MAC_ADDR_LEN);
		}
		COPY_MAC_ADDR(wdev->bssid, wdev->if_addr);
	break;
#endif /* CONFIG_AP_SUPPORT */
	default:
	break;
	}

	HcAcquireRadioForWdev(pAd,wdev);
	return TRUE;
}


/**
 * @param pAd
 * @param wdev wifi device
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	wlan_operate_exit(wdev);
	rtmp_wdev_idx_unreg(pAd, wdev);

    return TRUE;
}


/**
 * @param pAd
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_config_init(RTMP_ADAPTER *pAd)
{
	UCHAR i;
	struct wifi_dev *wdev;
	for(i=0;i<WDEV_NUM_MAX;i++){
		wdev = pAd->wdev_list[i];
		if(wdev){
			wdev->channel = 0;
			wdev->PhyMode = 0;
		}
	}
    return TRUE;
}


/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *WdevSearchByAddress(RTMP_ADAPTER *pAd, UCHAR *Address)
{
	UINT16 Index;
	struct wifi_dev *wdev;

	NdisAcquireSpinLock(&pAd->WdevListLock);
	for (Index = 0; Index < WDEV_NUM_MAX; Index++)
	{
		wdev = pAd->wdev_list[Index];

		if (wdev)
		{
			if (MAC_ADDR_EQUAL(Address, wdev->if_addr))
			{
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}
	NdisReleaseSpinLock(&pAd->WdevListLock);

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can not find registered wdev\n",
								__FUNCTION__));

	return NULL;
}


/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *WdevSearchByBssid(RTMP_ADAPTER *pAd, UCHAR *Address)
{
    UINT16 Index;
    struct wifi_dev *wdev;

    NdisAcquireSpinLock(&pAd->WdevListLock);
    for (Index = 0; Index < WDEV_NUM_MAX; Index++)
    {
        wdev = pAd->wdev_list[Index];

        if (wdev)
        {
            if (MAC_ADDR_EQUAL(Address, wdev->bssid))
            {
                NdisReleaseSpinLock(&pAd->WdevListLock);
                return wdev;
            }
        }
    }
    NdisReleaseSpinLock(&pAd->WdevListLock);

    MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can not find registered wdev\n",
                                __FUNCTION__));

    return NULL;
}

struct wifi_dev *WdevSearchByOmacIdx(RTMP_ADAPTER *pAd, UINT8 BssIndex)
{
	UINT16 Index;
	struct wifi_dev *wdev;

	NdisAcquireSpinLock(&pAd->WdevListLock);
	for (Index = 0; Index < WDEV_NUM_MAX; Index++)
	{
		wdev = pAd->wdev_list[Index];

		if (wdev)
		{
			if (wdev->OmacIdx == BssIndex)
			{
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}
	NdisReleaseSpinLock(&pAd->WdevListLock);

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: can not find registered wdev\n",
								__FUNCTION__));

	return NULL;
}


struct wifi_dev *WdevSearchByWcid(RTMP_ADAPTER *pAd, UINT8 wcid)
{
	struct _STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];
	struct wifi_dev *wdev = NULL;

	if (tr_entry) {
		wdev = tr_entry->wdev;
	} else {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("%s: can not a valid wdev by wcid (%u)\n",
					__FUNCTION__, wcid));
	}

	return wdev;
}

UCHAR decide_phy_bw_by_channel(struct _RTMP_ADAPTER *ad,UCHAR channel)
{
	int i;
	struct wifi_dev *wdev;
	UCHAR phy_bw = BW_20;
	UCHAR wdev_bw;
	UCHAR rfic;

	if(channel <= 14){
		rfic = RFIC_24GHZ;
	}else{
		rfic = RFIC_5GHZ;
	}

	for(i=0;i<WDEV_NUM_MAX;i++){
		wdev = ad->wdev_list[i];
		/*only when wdev is up & operting init done can join to decision*/
		if(wdev && (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_INVALID) && (rfic & wmode_2_rfic(wdev->PhyMode))){
			wdev_bw = wlan_operate_get_bw(wdev);
			if(wdev_bw > phy_bw)
				phy_bw = wdev_bw;
		}
	}
	if(rfic == RFIC_24GHZ && phy_bw > BW_40)
		phy_bw = BW_40;

	return phy_bw;
}


void update_att_from_wdev(struct wifi_dev *dev1, struct wifi_dev *dev2)
{
	UCHAR ht_bw = wlan_operate_get_ht_bw(dev2);
	UCHAR vht_bw = wlan_operate_get_vht_bw(dev2);
	UCHAR ext_cha;
	UCHAR stbc;
	UCHAR ldpc;
	UCHAR tx_stream;
	UCHAR rx_stream;

#ifdef WH_EZ_SETUP // Rakesh: fix for issue seen when ext-cha modified from GUI
	ext_cha = wlan_config_get_ext_cha(dev2);

	/*update configure*/
	if(wlan_config_get_ext_cha(dev1)!= ext_cha){

		wlan_config_set_ext_cha(dev1,ext_cha);
	}
#else
	/*update configure*/
	if(wlan_config_get_ext_cha(dev1)== EXTCHA_NOASSIGN){
		ext_cha = wlan_config_get_ext_cha(dev2);
		wlan_config_set_ext_cha(dev1,ext_cha);
	}
#endif

	stbc = wlan_config_get_ht_stbc(dev2);
	wlan_config_set_ht_stbc(dev1, stbc);
	ldpc = wlan_config_get_ht_ldpc(dev2);
	wlan_config_set_ht_ldpc(dev1, ldpc);
	stbc = wlan_config_get_vht_stbc(dev2);
	wlan_config_set_vht_stbc(dev1, stbc);
	ldpc = wlan_config_get_vht_ldpc(dev2);
	wlan_config_set_vht_ldpc(dev1, ldpc);

	ht_bw = wlan_config_get_ht_bw(dev2);
	vht_bw = wlan_config_get_vht_bw(dev2);
    wlan_config_set_ht_bw(dev1,ht_bw);
    wlan_config_set_vht_bw(dev1,vht_bw);

	tx_stream = wlan_config_get_tx_stream(dev2);
	wlan_config_set_tx_stream(dev1, tx_stream);
	rx_stream = wlan_config_get_rx_stream(dev2);
	wlan_config_set_rx_stream(dev1, rx_stream);

	dev1->channel = dev2->channel;
	/*updaet bw the same */
	ht_bw = wlan_operate_get_ht_bw(dev2);
	vht_bw = wlan_operate_get_vht_bw(dev2);
	wlan_operate_set_ht_bw(dev1,ht_bw);
	wlan_operate_set_vht_bw(dev1,vht_bw);	
	ext_cha = wlan_operate_get_ext_cha(dev2);
	wlan_operate_set_ext_cha(dev1,ext_cha);
}

VOID wdev_if_up_down(RTMP_ADAPTER *pAd, VOID *pDev, BOOLEAN if_up_down_state)
{
    UCHAR i = 0;
    struct net_device *pNetDev = (struct net_device *)pDev;
    struct wifi_dev *wdev = NULL;

    if (pNetDev == NULL)
        return;

    for (i = 0; i < WDEV_NUM_MAX; i++)
    {
        wdev = pAd->wdev_list[i];

        if (wdev == NULL)
            continue;

        if (wdev->if_dev == pNetDev) {
            wdev->if_up_down_state = if_up_down_state;
            break;
        }
    }    	
}

/** @} */
/** @} */
