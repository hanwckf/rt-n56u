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
#include "wlan_config/config_internal.h"

/*
*ht phy info related
*/
VOID ht_cfg_init(struct ht_cfg * obj)
{
	/*initial ht_phy_info value*/
	obj->ht_bw = HT_BW_20;
	obj->ext_cha = EXTCHA_NOASSIGN;
	obj->ht_ldpc = TRUE;
	obj->ht_stbc = STBC_USE;
	obj->frag_thld = DEFAULT_FRAG_THLD;
	obj->len_thld = DEFAULT_RTS_LEN_THLD;
	obj->pkt_thld = DEFAULT_RTS_PKT_THLD;
}

VOID ht_cfg_exit(struct ht_cfg * obj)
{
	os_zero_mem(obj,sizeof(struct ht_cfg));
}

/*
* ht operate related
*/
VOID ht_op_status_init(struct ht_op_status *obj)
{	

}

VOID ht_op_status_exit(struct ht_op_status *obj)
{
	os_zero_mem(obj,sizeof(struct ht_op_status));
}

/*
*  export configure function
*/
/*
*Set
*/
VOID wlan_config_set_ht_bw(struct wifi_dev *wdev,UCHAR ht_bw)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->ht_conf.ht_bw = ht_bw;
	/*configure loader*/
	config_loader_ht_bw((void*)wdev,cfg);
}

VOID wlan_config_set_ht_bw_all(struct wpf_ctrl *ctrl,UCHAR ht_bw)
{
	struct wlan_config *cfg;
	struct wlan_operate *op;
	unsigned int i;
	for(i=0;i<WDEV_NUM_MAX;i++){
		cfg = (struct wlan_config*)ctrl->pf[i].conf;
		cfg->ht_conf.ht_bw = ht_bw;
		op = (struct wlan_operate*)ctrl->pf[i].oper;
		op->ht_oper.ht_bw = ht_bw;
		operate_loader_bw(op);
	}
}

VOID wlan_config_set_ext_cha(struct wifi_dev *wdev, UCHAR ext_cha)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->ht_conf.ext_cha = ext_cha;
	config_loader_ext_cha(wdev,cfg);
}

VOID wlan_config_set_ht_stbc(struct wifi_dev *wdev, UCHAR ht_stbc)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->ht_conf.ht_stbc = ht_stbc;
}

VOID wlan_config_set_ht_ldpc(struct wifi_dev *wdev, UCHAR ht_ldpc)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->ht_conf.ht_ldpc = ht_ldpc;
}

VOID wlan_config_set_edca_valid(struct wifi_dev *wdev, BOOLEAN bValid)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	if (cfg)
		cfg->ht_conf.EdcaParm.bValid = bValid;
}

VOID wlan_config_set_edca_valid_all(struct wpf_ctrl *ctrl, BOOLEAN bValid)
{
	struct wlan_config *cfg;
	unsigned int i;
	for(i=0;i<WDEV_NUM_MAX;i++){
		cfg = (struct wlan_config*)ctrl->pf[i].conf;
		if (cfg)
			cfg->ht_conf.EdcaParm.bValid = bValid;
	}
}

VOID wlan_config_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	cfg->ht_conf.frag_thld = frag_thld;
}

VOID wlan_config_set_rts_len_thld(struct wifi_dev *wdev, UINT32 len_thld)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->ht_conf.len_thld = len_thld;
}

VOID wlan_config_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_thld)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->ht_conf.pkt_thld = pkt_thld;
}

/*
*Get
*/
UCHAR wlan_config_get_ht_bw(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->ht_conf.ht_bw;
}

UCHAR wlan_config_get_ext_cha(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->ht_conf.ext_cha;
}

UCHAR wlan_config_get_ht_stbc(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->ht_conf.ht_stbc;
}

UCHAR wlan_config_get_ht_ldpc(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->ht_conf.ht_ldpc;
}

BOOLEAN wlan_config_get_edca_valid(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	if (cfg)
		return cfg->ht_conf.EdcaParm.bValid;
	else
		return FALSE;
}

struct _EDCA_PARM* wlan_config_get_ht_edca(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	if (cfg)
		return &cfg->ht_conf.EdcaParm;
	else
		return NULL;
}

UINT32 wlan_config_get_frag_thld(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	
	return cfg->ht_conf.frag_thld;
}

UINT32 wlan_config_get_rts_len_thld(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->ht_conf.len_thld;
}

UCHAR wlan_config_get_rts_pkt_thld(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->ht_conf.pkt_thld;
}
