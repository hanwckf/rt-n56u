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
* define  constructor & deconstructor & method
*/
VOID phy_oper_init(struct phy_op *obj)
{
	obj->wdev_bw = BW_20;
}

VOID phy_oper_exit(struct phy_op *obj)
{
	os_zero_mem(obj,sizeof(*obj));
}

INT freq_cfg_adjust(UCHAR ht_bw,UCHAR vht_bw,struct freq_cfg *freq)
{
	UCHAR wdev_bw;

	if(ht_bw == HT_BW_40) {
		ht_bw = HT_BW_40;
	} else {
		ht_bw = HT_BW_20;
	}

	if(ht_bw == HT_BW_20)
		wdev_bw = BW_20;
	else {
		if(vht_bw == VHT_BW_80)
			wdev_bw = BW_80;
		else if(vht_bw == VHT_BW_160)
			wdev_bw = BW_160;
		else if(vht_bw == VHT_BW_8080)
			wdev_bw = BW_8080;
		else
			wdev_bw = BW_40;
	}	
	freq->bw = wdev_bw;
	return TRUE;
}


/*
* Configure loader
*/
VOID operate_loader_bw(struct wlan_operate *op)
{
	UCHAR ht_bw = op->ht_oper.ht_bw;
	UCHAR vht_bw = op->vht_oper.vht_bw;
	struct freq_cfg freq;

	freq_cfg_adjust(ht_bw,vht_bw,&freq);
	
	op->phy_oper.wdev_bw = freq.bw;
}


VOID operate_loader_phy_bw(struct wlan_operate *op)
{
	UCHAR ht_bw = op->ht_oper.ht_bw;
	UCHAR vht_bw = op->vht_oper.vht_bw;
	UCHAR phy_bw = (!ht_bw) ? BW_20 :
						((vht_bw==VHT_BW_2040)	? BW_40 :
						((vht_bw==VHT_BW_80)	? BW_80 :
						((vht_bw==VHT_BW_160) 	? BW_160 : BW_8080)));
	if(phy_bw != op->phy_oper.wdev_bw){
		/*adjust vht_bw & ht_bw, due to modify phy_bw directly!*/
		switch(op->phy_oper.wdev_bw){
		case BW_20:
			ht_bw = HT_BW_20;
			vht_bw = VHT_BW_2040;
			break;
		case BW_40:
			ht_bw = HT_BW_40;
			vht_bw = VHT_BW_2040;
			break;
		case BW_80:
			ht_bw = HT_BW_40;
			vht_bw = VHT_BW_80;
			break;
		case BW_160:
			ht_bw = HT_BW_40;
			vht_bw = VHT_BW_160;
			break;
		case BW_8080:
			ht_bw = HT_BW_40;
			vht_bw = VHT_BW_8080;
			break;
		}
		op->ht_oper.ht_bw = ht_bw;
		op->vht_oper.vht_bw = vht_bw;
		/*ht operate loader*/
		op->ht_status.addht.AddHtInfo.RecomWidth = op->ht_oper.ht_bw;
	}

}

VOID operate_loader_prim_ch(struct wlan_operate *op)
{
	UCHAR prim_ch = op->phy_oper.prim_ch;
	op->ht_status.addht.ControlChan = prim_ch;
}

/*
* Operater loader
*/


/*
* export function
*/

/*
* operation function
*/
UCHAR wlan_operate_get_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	return op->phy_oper.wdev_bw;
}

INT32 wlan_operate_set_bw(struct wifi_dev *wdev, UCHAR bw)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	op->phy_oper.wdev_bw = bw;	
	return WLAN_OPER_OK;
}

UCHAR wlan_operate_get_prim_ch(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	return op->phy_oper.prim_ch;
}

INT32 wlan_operate_set_prim_ch(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct wlan_operate *op = (struct wlan_operate*) wdev->wpf_op;
	op->phy_oper.prim_ch = prim_ch;
	operate_loader_prim_ch(op);
	return WLAN_OPER_OK;
}
