/* *
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

#ifndef __CONFIG_EXPORT_H__
#define __CONFIG_EXPORT_H__

struct _RTMP_ADAPTER;

struct wpf_data {
	UCHAR idx;
	void *dev;
	void *conf;
	void *oper;
};

struct wpf_ctrl{
	struct wpf_data pf[WDEV_NUM_MAX];
};

struct phy_cfg{
	UCHAR tx_stream;
	UCHAR rx_stream;
	UCHAR ack_policy[WMM_NUM_OF_AC];
};

struct ht_cfg {
	UCHAR 	ext_cha;
	UCHAR	ht_bw;
	UCHAR	ht_stbc;
	UCHAR	ht_ldpc;
	UINT32	frag_thld;
	/*rts threshold*/
	UCHAR	pkt_thld;
	UINT32	len_thld;
	/* EDCA parameters to be announced to its local BSS */
	struct _EDCA_PARM EdcaParm; 
};

struct vht_cfg {
	UCHAR vht_bw;
	UCHAR	vht_stbc;
	UCHAR	vht_ldpc;
};


/*for profile usage*/
VOID wpf_init(struct _RTMP_ADAPTER *ad);
VOID wpf_exit(struct _RTMP_ADAPTER *ad);
VOID wpf_config_exit(struct _RTMP_ADAPTER *ad);
VOID wpf_config_init(struct _RTMP_ADAPTER *ad);

/*
* Configure Get
*/
UCHAR wlan_config_get_ht_bw(struct wifi_dev *wdev);
UCHAR wlan_config_get_ext_cha(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_bw(struct wifi_dev *wdev);
UCHAR wlan_config_get_ext_cha(struct wifi_dev *wdev);
UCHAR wlan_config_get_ack_policy(struct wifi_dev *wdev, UCHAR ac_id);
UCHAR wlan_config_get_tx_stream(struct wifi_dev *wdev);
UCHAR wlan_config_get_rx_stream(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_stbc(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_ldpc(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_stbc(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_ldpc(struct wifi_dev *wdev);
UINT32 wlan_config_get_frag_thld(struct wifi_dev *wdev);
UINT32 wlan_config_get_rts_len_thld(struct wifi_dev *wdev);
UCHAR wlan_config_get_rts_pkt_thld(struct wifi_dev *wdev);
BOOLEAN wlan_config_get_edca_valid(struct wifi_dev *wdev);
struct _EDCA_PARM* wlan_config_get_ht_edca(struct wifi_dev *wdev);
/*
* Configure Set
*/
VOID wlan_config_set_ht_bw(struct wifi_dev *wdev,UCHAR ht_bw);
VOID wlan_config_set_ht_bw_all(struct wpf_ctrl *ctrl,UCHAR ht_bw);
VOID wlan_config_set_ht_ext_cha(struct wifi_dev *wdev,UCHAR ext_cha);
VOID wlan_config_set_ht_ext_cha_all(struct wpf_ctrl *ctrl,UCHAR ext_cha);
VOID wlan_config_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw);
VOID wlan_config_set_vht_bw_all(struct wpf_ctrl *ctrl,UCHAR vht_bw);
VOID wlan_config_set_ext_cha(struct wifi_dev *wdev, UCHAR ext_cha);
VOID wlan_config_set_ack_policy(struct wifi_dev *wdev,UCHAR *policy);
VOID wlan_config_set_ack_policy_all(struct wpf_ctrl *ctrl,UCHAR *policy);
VOID wlan_config_set_tx_stream(struct wifi_dev *wdev,UCHAR tx_stream);
VOID wlan_config_set_rx_stream(struct wifi_dev *wdev,UCHAR rx_stream);
VOID wlan_config_set_ht_stbc(struct wifi_dev *wdev, UCHAR ht_stbc);
VOID wlan_config_set_ht_ldpc(struct wifi_dev *wdev, UCHAR ht_ldpc);
VOID wlan_config_set_vht_stbc(struct wifi_dev *wdev,UCHAR vht_stbc);
VOID wlan_config_set_vht_ldpc(struct wifi_dev *wdev,UCHAR vht_ldpc);
VOID wlan_config_set_rts_len_thld(struct wifi_dev *wdev, UINT32 len_thld);
VOID wlan_config_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_thld);
VOID wlan_config_set_edca_valid(struct wifi_dev *wdev, BOOLEAN bValid);
VOID wlan_config_set_edca_valid_all(struct wpf_ctrl *ctrl, BOOLEAN bValid);
VOID wlan_config_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld);


#endif
