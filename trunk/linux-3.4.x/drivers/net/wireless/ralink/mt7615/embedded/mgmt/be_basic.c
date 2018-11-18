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

/*init radio operate from cfg*/
static VOID radio_operate_init(struct wifi_dev *wdev)
{	
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	UCHAR ext_cha;
	UCHAR vht_bw;

	/*ht part*/
	if(cfg->ht_conf.ht_bw == HT_BW_40){
		wlan_operate_set_ht_bw(wdev,HT_BW_40);
		ext_cha = (cfg->ht_conf.ext_cha) ? 
			(cfg->ht_conf.ext_cha) :  EXTCHA_ABOVE;
	}else
	{
		wlan_operate_set_ht_bw(wdev,HT_BW_20);
		ext_cha = EXTCHA_NONE;
	}
	wlan_operate_set_ext_cha(wdev,ext_cha);

	/*vht part*/
	if(!WMODE_CAP_AC(wdev->PhyMode))
		vht_bw = VHT_BW_2040;
	else
	{
		if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_GO)) 
		{
			INT8 max_vht_bw;
			vht_bw = cfg->vht_conf.vht_bw;
			/* Get the max vht bw by region instead of config only */
			max_vht_bw = get_max_vht_bw_by_region(wdev);
			if (max_vht_bw >= VHT_BW_2040)
			{
				vht_bw = max_vht_bw;
			}
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
						("%s : Error! Check! wdev->channel=%d\n", __FUNCTION__,wdev->channel));
			}
		} 
		else
			vht_bw = cfg->vht_conf.vht_bw;
	}

	wlan_operate_set_vht_bw(wdev,vht_bw);
	/*frag threshold*/
	wlan_operate_set_frag_thld(wdev, cfg->ht_conf.frag_thld);
	/*rts*/
	wlan_operate_set_rts_len_thld(wdev, cfg->ht_conf.len_thld);
	wlan_operate_set_rts_pkt_thld(wdev, cfg->ht_conf.pkt_thld);
}

/*
* private structure definition to prevent direct access
*/
VOID wlan_operate_init(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate*)wdev->wpf_op;

	if(!obj){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("%s : Can't find wlan operate for wdev. \n", __FUNCTION__));
		return;
	}
	/*reset to default*/
	phy_oper_init(&obj->phy_oper);
	ht_oper_init(&obj->ht_oper);
	vht_oper_init(&obj->vht_oper);
	ht_op_status_init(&obj->ht_status);
	vht_op_status_init(&obj->vht_status);
	/*adjust radio operate from configure*/
	radio_operate_init(wdev);
}

VOID wlan_operate_exit(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate*)wdev->wpf_op;

	if(!obj){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s : Can't find wlan operate for wdev. \n", __FUNCTION__));
		return;
	}
	phy_oper_exit(&obj->phy_oper);
	ht_oper_exit(&obj->ht_oper);
	vht_oper_exit(&obj->vht_oper);
	ht_op_status_exit(&obj->ht_status);
	vht_op_status_exit(&obj->vht_status);
}

UCHAR wlan_operate_get_state(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate*)wdev->wpf_op;

	if(!obj){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : Can't find wlan operate for wdev. \n", __FUNCTION__));
		return WLAN_OPER_STATE_INVALID;
	}
	return obj->state;
}

UCHAR wlan_operate_set_state(struct wifi_dev *wdev,UCHAR state)
{
	struct wlan_operate *obj = (struct wlan_operate*)wdev->wpf_op;

	if(!obj){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : Can't find wlan operate for wdev. \n", __FUNCTION__));
		return WLAN_OPER_FAIL;
	}
	obj->state = state;
	return WLAN_OPER_OK;
}

