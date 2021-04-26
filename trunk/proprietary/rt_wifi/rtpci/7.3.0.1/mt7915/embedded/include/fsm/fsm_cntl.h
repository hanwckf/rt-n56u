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

#ifndef __CMM_MGMT_CNTL_H__
#define __CMM_MGMT_CNTL_H__

enum _CNTL_CONNECT_TYPE {
	CNTL_CONNECT_BY_SSID,
	CNTL_CONNECT_BY_BSSID,
	CNTL_CONNECT_BY_CFG,
	CNTL_CONNECT_ROAMING_REQ,
	CNTL_CONNECT_UNDEF,
};

enum _CNTL_DISCONNECT_TYPE {
	CNTL_DEAUTH,
	CNTL_DISASSOC,
	CNTL_DISCONNECT_UNDEF,
};

typedef struct _MLME_AUTH_REQ_STRUCT {
    UCHAR        Addr[MAC_ADDR_LEN];
    USHORT       Alg;
    ULONG        Timeout;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR	BssIdx;
	UCHAR	CliIdx;
#endif /* MAC_REPEATER_SUPPORT */
} MLME_AUTH_REQ_STRUCT, *PMLME_AUTH_REQ_STRUCT;

typedef struct {
    ULONG BssIdx;
	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR SsidLen;
	UCHAR Ssid[MAX_LEN_OF_SSID];
} MLME_JOIN_REQ_STRUCT;

typedef struct _CTRL_JOIN_MSG_STRUCT {
	USHORT Status;
	UCHAR SrcAddr[MAC_ADDR_LEN];
#ifdef MAC_REPEATER_SUPPORT
	UCHAR BssIdx;
	UCHAR CliIdx;
#endif /* MAC_REPEATER_SUPPORT */
} CTRL_JOIN_MSG_STRUCT;

typedef struct _MLME_START_REQ_STRUCT {
    CHAR        Ssid[MAX_LEN_OF_SSID];
    UCHAR       SsidLen;
} MLME_START_REQ_STRUCT, *PMLME_START_REQ_STRUCT;

typedef struct _CNTL_MLME_CONNECT_STRUCT {
	enum _CNTL_CONNECT_TYPE conn_type;
	UCHAR data_len;
	UCHAR data[0];
} CNTL_MLME_CONNECT_STRUCT;

typedef struct _MLME_DISCONNECT_STRUCT {
	UCHAR addr[MAC_ADDR_LEN];
	USHORT reason;
} MLME_DISCONNECT_STRUCT;

typedef struct _CNTL_MLME_DISCONNECT_STRUCT {
	enum _CNTL_DISCONNECT_TYPE cntl_disconn_type;
	MLME_DISCONNECT_STRUCT mlme_disconn;
} CNTL_MLME_DISCONNECT_STRUCT;

struct _cntl_api_ops {
	VOID(*cntl_disconnect_proc)(VOID *elem);
	VOID(*cntl_connect_proc)(struct wifi_dev *wdev, VOID *data, UINT32 data_len);
	VOID(*cntl_scan_proc)(VOID *elem);
	VOID(*cntl_reset_all_fsm_proc)(VOID *elem);
	VOID(*cntl_join_conf)(VOID *elem);
	VOID(*cntl_auth_conf)(VOID *elem);
	VOID(*cntl_auth2_conf)(VOID *elem);
	VOID(*cntl_deauth_conf)(VOID *elem);
	VOID(*cntl_assoc_conf)(VOID *elem);
	VOID(*cntl_reassoc_conf)(VOID *elem);
	VOID(*cntl_disassoc_conf)(VOID *elem);
	VOID(*cntl_scan_conf)(VOID *elem);
	VOID(*cntl_error_handle)(VOID *elem);
};

#ifdef CONFIG_AP_SUPPORT
VOID ap_cntl_init(
	struct wifi_dev *wdev);
#else /* CONFIG_AP_SUPPORT */
#define ap_cntl_init(_wdev)
#endif /* !CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID sta_cntl_init(
	struct wifi_dev *wdev);
#else /* CONFIG_STA_SUPPORT */
#define sta_cntl_init(_wdev)
#endif /* !CONFIG_STA_SUPPORT */

void rept_muar_read(PRTMP_ADAPTER pAd, UINT id);

BOOLEAN cntl_connect_request(
	struct wifi_dev *wdev,
	enum _CNTL_CONNECT_TYPE conn_type,
	UCHAR data_len,
	UCHAR *data);

BOOLEAN cntl_disconnect_request(
	struct wifi_dev *wdev,
	enum _CNTL_DISCONNECT_TYPE disconn_type,
	UCHAR *addr,
	USHORT reason);

BOOLEAN cntl_scan_request(
	struct wifi_dev *wdev,
	MLME_SCAN_REQ_STRUCT *mlme_scan_request);

BOOLEAN cntl_scan_conf(
	struct wifi_dev *wdev,
	USHORT status);

BOOLEAN cntl_join_start_conf(
	struct wifi_dev *wdev,
	USHORT status);


BOOLEAN cntl_auth_assoc_conf(
	struct wifi_dev *wdev,
	enum _CNTL_MLME_EVENT event_type,
	USHORT reason);

BOOLEAN cntl_do_disassoc_now(
	struct wifi_dev *wdev);

BOOLEAN cntl_idle(
	struct wifi_dev *wdev);

VOID cntl_fsm_reset(
	struct wifi_dev *wdev);

BOOLEAN cntl_reset_all_fsm_in_ifdown(
	struct wifi_dev *wdev);

VOID cntl_state_machine_init(
	IN struct wifi_dev *wdev,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

BOOLEAN cntl_fsm_state_transition(
	struct wifi_dev *wdev,
	ULONG next_state,
	const char *caller);


#endif	/* CMM_MGMT_CNTL_H__ */
