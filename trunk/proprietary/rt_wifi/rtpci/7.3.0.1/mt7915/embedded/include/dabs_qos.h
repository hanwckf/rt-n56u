/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ra_ac_q_mgmt.h
*/

#include    "rt_config.h"

#ifndef _DABS_QOS_H_
#define _DABS_QOS_H_

#ifdef DABS_QOS
#define DABS_DBG_PRN_LV1	(1 << 0)
#define DABS_DBG_PRN_LV2	(1 << 1)
#define DABS_SET_QOS_PARAM	(1 << 2)
#define DABS_DBG_DLY_TIME	(1 << 3)

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define NUM_OF_8021D_COS        8

#define MAX_QOS_PARAM_TBL 32

enum app_type_def {
	GAMING = 0,
	VR,
	CLOUD_VR,
	CLOUD_GAMING,
	VOIP,
	VIDEO_CONFERENCE,
	VIDEO_STREAM,
	IPTV,
	UNKONWN,
	MAX_APP_TYPE
};

struct qos_param_rec {
	unsigned short wlan_idx;
	unsigned int ip_src;
	unsigned int ip_dest;
	unsigned short sport;
	unsigned short dport;
	unsigned short protocol;
	unsigned char priority;
	unsigned short delay_bound; /* drop packet with delay above max delay bound */
	unsigned short delay_req; /* for SmartQoS scheduler use */
	unsigned char delay_weight;
	unsigned int data_rate; /* application data rate */
	unsigned short bw_req;
	unsigned char dir;	/* DL or UL */
	unsigned short drop_thres;
	enum app_type_def app_type;
	bool valid;
	unsigned long long tot_pkt_dly;	/* usec */
	unsigned int tot_pkt_cnt;
	unsigned int some_pkt_dly;
	unsigned int avg_pkt_dly;
	unsigned int max_pkt_dly;
};

INT set_dabs_qos_param(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
unsigned short search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, struct sk_buff *skb);
unsigned short search_qos_param_tbl_idx_by_wlan_idx(unsigned short wlan_idx,
	unsigned short proto, unsigned short dport);
extern struct qos_param_rec qos_param_table[MAX_QOS_PARAM_TBL];
bool set_qos_param_tbl_wlan_idx_by_idx(unsigned short idx, unsigned short wlan_idx);
bool set_qos_param_to_fw(PRTMP_ADAPTER pAd, unsigned short idx);
bool enable_qos_param_tbl_by_idx(unsigned short idx);
bool disable_qos_param_tbl_by_idx(unsigned short idx);
void dabs_host_delay(PRTMP_ADAPTER pAd, NDIS_PACKET *pkt);
void dabs_active_qos_by_ipaddr(PRTMP_ADAPTER pAd, NDIS_PACKET *pkt);
#endif /* DABS_QOS */
#endif /* _DABS_QOS_H_ */


