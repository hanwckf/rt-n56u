 /***************************************************************************
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
#include "mgmt/be_internal.h"

/*
*ht phy info related
*/
VOID ht_oper_init(struct ht_op * obj)
{
	/*initial ht_phy_info value*/
	obj->ht_bw = HT_BW_20;
	obj->ext_cha = EXTCHA_NONE;
	obj->frag_thld = DEFAULT_FRAG_THLD;
	obj->len_thld = DEFAULT_RTS_LEN_THLD;
	obj->pkt_thld = DEFAULT_RTS_PKT_THLD;
}

VOID ht_oper_exit(struct ht_op * obj)
{
	os_zero_mem(obj,sizeof(*obj));
}


/*
* Configure loader
*/
VOID config_loader_ht_bw(struct wifi_dev *wdev,struct wlan_config *cfg)
{
	wlan_operate_set_ht_bw(wdev,cfg->ht_conf.ht_bw);
}

VOID config_loader_ext_cha(struct wifi_dev *wdev,struct wlan_config *cfg)
{
	wlan_operate_set_ext_cha(wdev,cfg->ht_conf.ext_cha);
}

VOID config_loader_rts_pkt_thld(struct wifi_dev *wdev, struct wlan_config *cfg)
{
	wlan_operate_set_rts_pkt_thld(wdev, cfg->ht_conf.pkt_thld);
}

VOID config_loader_rts_len_thld(struct wifi_dev *wdev, struct wlan_config *cfg)
{
	wlan_operate_set_rts_len_thld(wdev, cfg->ht_conf.len_thld);
}

/*
* Operate loader
*/
VOID operate_loader_ht_bw(struct wlan_operate *op)
{
	op->ht_status.addht.AddHtInfo.RecomWidth = op->ht_oper.ht_bw;
	operate_loader_bw(op);
}

VOID operate_loader_ext_cha(struct wlan_operate *op)
{
	op->ht_status.addht.AddHtInfo.ExtChanOffset= op->ht_oper.ext_cha;
}

/*
*  export operate function
*/
/*
* Set
*/
INT32 wlan_operate_set_ht_bw(struct wifi_dev *wdev,UCHAR ht_bw)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	UCHAR cap_ht_bw = wlan_config_get_ht_bw(wdev);
	INT32 ret = WLAN_OPER_OK;

	if(ht_bw > cap_ht_bw){
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("%s(): new ht_bw:%d > cap_ht_bw: %d, correct to cap_ht_bw\n",
		__FUNCTION__,
		ht_bw,
		cap_ht_bw
		));
		ht_bw = cap_ht_bw;
		ret = WLAN_OPER_FAIL;
	}
	op->ht_oper.ht_bw = ht_bw;
	/*configure loader*/
	operate_loader_ht_bw(op);
	return ret;
}

INT32 wlan_operate_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_num)
{
	struct wlan_operate *op = (struct wlan_operate*)wdev->wpf_op;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	INT32 ret = WLAN_OPER_OK;

	op->ht_oper.pkt_thld = pkt_num;
	HW_SET_RTS_THLD(ad, wdev, op->ht_oper.pkt_thld, op->ht_oper.len_thld);

	return ret;
}

INT32 wlan_operate_set_rts_len_thld(struct wifi_dev *wdev, UINT32 pkt_len)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	INT32 ret = WLAN_OPER_OK;

	op->ht_oper.len_thld = pkt_len;
	HW_SET_RTS_THLD(ad, wdev, op->ht_oper.pkt_thld, op->ht_oper.len_thld);

	return ret;
}

INT32 wlan_operate_set_non_gf_sta(struct wifi_dev *wdev,UINT16 non_gf_sta)
{
	struct wlan_operate *op;
	/*due to use in MACTableMaintaince*/
	if(wdev && wdev->wpf_op){
		op = (struct wlan_operate*)wdev->wpf_op;
		op->ht_status.non_gf_sta = non_gf_sta;
	}	
	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_ext_cha(struct wifi_dev *wdev,UCHAR ext_cha)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	struct wlan_operate *top;
	struct _RTMP_ADAPTER *ad;
	struct wifi_dev *twdev;
	UCHAR i;

	op->ht_oper.ext_cha = ext_cha;
	/*configure loader*/
	operate_loader_ext_cha(op);
	/*band selection ready.*/
	if(hc_radio_acquire(wdev,op->phy_oper.wdev_bw,op->ht_oper.ext_cha) < 0)
		return WLAN_OPER_OK;

	HcUpdateExtCha(wdev->sys_handle,wdev->channel,ext_cha);

	/*update extcha since radio is changed*/
	ad = (struct _RTMP_ADAPTER*)wdev->sys_handle;
	for(i=0;i<WDEV_NUM_MAX;i++){
		twdev = ad->wdev_list[i];
		if(twdev && twdev->channel == wdev->channel){
			top = (struct wlan_operate*)twdev->wpf_op;
			if(top->ht_oper.ht_bw!=HT_BW_20){
				top->ht_oper.ext_cha = ext_cha;
				operate_loader_ext_cha(top);
			}
		}
	}
	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;

	op->ht_oper.frag_thld = frag_thld;
	return WLAN_OPER_OK;
}
/*
*Get
*/
UCHAR wlan_operate_get_ht_bw(struct wifi_dev *wdev)
{	
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	return op->ht_oper.ht_bw;
}

UCHAR wlan_operate_get_ext_cha(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	return op->ht_oper.ext_cha;
}

UCHAR wlan_operate_get_rts_pkt_thld(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	return op->ht_oper.pkt_thld;
}

UINT32 wlan_operate_get_rts_len_thld(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	return op->ht_oper.len_thld;
}

struct _ADD_HT_INFO_IE* wlan_operate_get_addht(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	return &op->ht_status.addht;
}

UINT16 wlan_operate_get_non_gf_sta(struct wifi_dev *wdev)
{
	struct wlan_operate *op;
	if(wdev && wdev->wpf_op){
		op = (struct wlan_operate*)wdev->wpf_op;
		return op->ht_status.non_gf_sta;
	}
	return 0;
}

UINT32 wlan_operate_get_frag_thld(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;

	return op->ht_oper.frag_thld;
}
