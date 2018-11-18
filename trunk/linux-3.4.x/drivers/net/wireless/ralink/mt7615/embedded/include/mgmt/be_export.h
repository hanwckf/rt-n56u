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

#ifndef __BE_EXPORT_H__
#define __BE_EXPORT_H__

#include "rtmp_def.h"

struct _RTMP_ADAPTER;

enum {
	WLAN_OPER_STATE_INVALID=0,
	WLAN_OPER_STATE_VALID,
};

/*basic info obj */
/*
struct phy_cfg{
	UCHAR phy_mode;
	UCHAR prim_ch;
	UCHAR tx_stream;
	UCHAR rx_stream;
};

struct phy_op {
	UCHAR phy_mode;
	UCHAR prim_ch;
	UCHAR tx_stream;
	UCHAR rx_stream;
	UCHAR wdev_bw;
	UCHAR central1;
	UCHAR central2;
}

struct ht_info {
	UCHAR 	ext_cha;
	UCHAR	ht_bw;
	UCHAR 	oper_mode;
	UCHAR 	mcs_set[16];
	BOOLEAN ht_en;
	BOOLEAN pre_nht_en;
	BOOLEAN gf;
	BOOLEAN	sgi_20;
	BOOLEAN sgi_40;
	BOOLEAN bss_coexist2040;
	BOOLEAN ldpc;
	BOOLEAN itx_bf;
	BOOLEAN etx_bf;
	BOOLEAN tx_stbc;
	BOOLEAN rx_stbc;
	struct ba_cap ba_cap;
};

struct ht_op_status {
	BOOLEAN obss_non_ht_exist;
	BOOLEAN non_gf_present;
	UCHAR 	central_ch;
	HT_CAPABILITY_IE ht_cap;
	ADD_HT_INFO_IE addht;
	UINT16 	non_gf_sta;
};

struct ba_cap {
	UCHAR mm_ps_mode;
	UCHAR amsdu_size;
	UCHAR mpdu_density;
	UCHAR policy;
	UCHAR tx_ba_win_limit;
	UCHAR rx_ba_win_limit;
	UCHAR max_ra_mpdu_factor;
	BOOLEAN amsdu_en;	
	BOOLEAN auto_ba;
};

struct vht_info {
	BOOLEAN vht_en;
	BOOLEAN force_vht;
	UCHAR vht_bw;
	UCHAR vht_sgi;
	UCHAR vht_stbc;
	UCHAR vht_bw_signal;
	UCHAR vht_cent_ch;
	UCHAR vht_cent_ch2;
	UCHAR vht_mcs_cap;
	UCHAR vht_nss_cap;
	USHORT vht_tx_hrate;
	USHORT vht_rx_hrate;
	BOOLEAN ht20_forbid;
	BOOLEAN vht_ldpc;
	BOOLEAN g_band_256_qam;
};

struct vht_op_status{
};
*/

struct phy_op {
	UCHAR prim_ch;
	/*private attribute*/
	UCHAR wdev_bw;
};

struct ht_op {
	UCHAR 	ext_cha;
	UCHAR	ht_bw;
	UINT32	frag_thld;
	/*rts threshold*/
	UCHAR	pkt_thld;
	UINT32	len_thld;
};

struct ht_op_status {
	/* Useful as AP. */
	ADD_HT_INFO_IE addht;
	/* counters */
	UINT16 	non_gf_sta;
};

struct vht_op {
	UCHAR vht_bw;
};


struct vht_op_status{
};

struct dev_rate_info {
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen;
	/* OID_802_11_DESIRED_RATES */
	UCHAR DesireRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR MaxDesiredRate;
	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR MaxTxRate;
	/* Tx rate index in Rate Switch Table */
	UCHAR TxRateIndex;
	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR MinTxRate;
	/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR TxRate;
	/* MGMT frame PHY rate setting when operatin at Ht rate. */
	HTTRANSMIT_SETTING MlmeTransmit;
};

#define WLAN_OPER_OK 	(0)
#define WLAN_OPER_FAIL 	(-1)

/*
*
Operate GET
*/
struct _ADD_HT_INFO_IE* wlan_operate_get_addht(struct wifi_dev *wdev);
UCHAR wlan_operate_get_bw(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ht_bw(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ext_cha(struct wifi_dev *wdev);
UCHAR wlan_operate_get_vht_bw(struct wifi_dev *wdev);
UINT16 wlan_operate_get_non_gf_sta(struct wifi_dev *wdev);
UCHAR wlan_operate_get_prim_ch(struct wifi_dev *wdev);
UINT32 wlan_operate_get_frag_thld(struct wifi_dev *wdev);
UCHAR wlan_operate_get_rts_pkt_thld(struct wifi_dev *wdev);
UINT32 wlan_operate_get_rts_len_thld(struct wifi_dev *wdev);

/*
* Operate Set
*/
INT32 wlan_operate_set_vht_bw(struct wifi_dev *wdev,UCHAR vht_bw);
INT32 wlan_operate_set_ht_bw(struct wifi_dev *wdev,UCHAR ht_bw);
INT32 wlan_operate_set_ext_cha(struct wifi_dev *wdev,UCHAR ext_cha);
INT32 wlan_operate_set_non_gf_sta(struct wifi_dev *wdev,UINT16 non_gf_sta);
INT32 wlan_operate_set_bw(struct wifi_dev *wdev, UCHAR bw); 
INT32 wlan_operate_set_prim_ch(struct wifi_dev *wdev,UCHAR prim_ch);
INT32 wlan_operate_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld);
INT32 wlan_operate_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_num);
INT32 wlan_operate_set_rts_len_thld(struct wifi_dev *wdev, UINT32 pkt_len);

/*
*
*/
VOID wlan_operate_init(struct wifi_dev *wdev);
VOID wlan_operate_exit(struct wifi_dev *wdev);
UCHAR wlan_operate_set_state(struct wifi_dev *wdev,UCHAR state);
UCHAR wlan_operate_get_state(struct wifi_dev *wdev);



#endif
