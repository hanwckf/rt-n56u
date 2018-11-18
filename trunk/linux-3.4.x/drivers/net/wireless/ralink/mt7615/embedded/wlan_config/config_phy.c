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
* define  constructor & deconstructor & method
*/
/*
*basic phy related
*/
VOID phy_cfg_init(struct phy_cfg *obj)
{
}

VOID phy_cfg_exit(struct phy_cfg *obj)
{
	os_zero_mem(obj,sizeof(struct phy_cfg));
}

/*
* Operater loader
*/


/*
* export function
*/
/*
* configure functio
*/
/*
*Set
*/
VOID wlan_config_set_ack_policy(struct wifi_dev *wdev,UCHAR *policy)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	UCHAR i=0;
	for(i = 0; i < WMM_NUM_OF_AC ; i++){
		cfg->phy_conf.ack_policy[i] = policy[i];
	}
}

VOID wlan_config_set_ack_policy_all(struct wpf_ctrl *ctrl,UCHAR *policy)
{
	struct wlan_config *cfg;
	UCHAR i;
	UCHAR j;
	for(i=0;i<WDEV_NUM_MAX;i++){
		cfg = (struct wlan_config*)ctrl->pf[i].conf;
		for(j = 0; j < WMM_NUM_OF_AC ; j++){
			cfg->phy_conf.ack_policy[j] = policy[j];
		}
	}
}

VOID wlan_config_set_tx_stream(struct wifi_dev *wdev,UCHAR tx_stream)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->phy_conf.tx_stream = tx_stream;
}

VOID wlan_config_set_rx_stream(struct wifi_dev *wdev,UCHAR rx_stream)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	cfg->phy_conf.rx_stream = rx_stream;
}

/*
*Get
*/
UCHAR wlan_config_get_ack_policy(struct wifi_dev *wdev, UCHAR ac_id)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->phy_conf.ack_policy[ac_id];
}

UCHAR wlan_config_get_tx_stream(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->phy_conf.tx_stream;
}

UCHAR wlan_config_get_rx_stream(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->phy_conf.rx_stream;
}

