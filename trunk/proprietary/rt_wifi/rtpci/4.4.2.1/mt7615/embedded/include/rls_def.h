/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rls_def.h
*/

#ifndef __RLS_DEF_H__
#define __RLS_DEF_H__

#ifdef RADIO_LINK_SELECTION

struct _RLS_OPS;
#define P_RLS_TABLE	(&pAd->ApCfg.RlsTable)
#define OID_RLS_MSG				0x0953

enum ACT_CODE{
	INF_STATUS_QUERY_INFO,/* 0 */
	INF_CHANNEL_UPDATE,
	INF_OPMODE_UPDATE,
	INF_STATUS_2G_RSP,
	INF_STATUS_5G_RSP,
	RLS_STATE_RSP,		/* 5 */
	IF_CLI_SCAN_REQ, 
	IF_CLI_DISABLE_REQ,
	IF_CLI_LINKUP_RSP, 
	IF_CLI_LINKDOWN_RSP, 
	STATE_QUERY,    /* 10 */
	RLS_ONOFF,      
	INF_OFF,		
	INF_ON,		
	INF_RESET,
	CLI_SCAN_PERIOD_TIME,
	DEBUG_INFORMATION
};

enum RLS_STATUS {
	RLS_NO_INIT_STAT	,
	RLS_INIT_STATE		,
	RLS_SCAN_STATE		,
	RLS_LINKUP_STATE
};

union rls_msg {
	struct _rls_on_off_msg{
		u8	Action;
		u8 bEnable;
		u8 state;
	} rls_on_off_msg;

	struct _rls_action_msg{
		u8	Action;
	} rls_action_msg;

	struct _rls_period_msg{
		u8	Action;
		u8 Time;
	} rls_period_msg;

	 struct _rls_inf_msg{
		u8	Action;
		u8 bInfReady;
		u8 bInfOp;
		u8 isUP;
		//BOOLEAN bEnabled;
		u8	Channel;
		s8	ucIfName[32];
		u16 	ucType;
	} rls_inf_msg;

	 struct _rls_cli_link_msg{
		u8	Action;
		u8	Channel;
		u8	ucIfName[32];
		u16 	ucType;
	} rls_cli_link_msg;

	 struct _rls_channel_update_msg{
		u8	Action;
		u8	Old_Channel;
		u8	New_Channel;
	} rls_channel_update_msg;

	 struct _rls_opmode_update_msg{
		u8	Action;
		u8	Channel;
		u8	OpMode;
	} rls_opmode_update_msg;
};

typedef struct _RLS_INF_INFO {
	BOOLEAN bInfReady;	/*Driver aleady has this information*/
	BOOLEAN bInfOp;		/*Interface is workable ,ex: apcli link up*/
	UINT8	Channel;	
	UCHAR	ucIfName[32];
	struct wifi_dev *wdev; 
} RLS_INF_INFO, *PRLS_INF_INFO;

typedef struct _RLS_CLI_TABLE {
	BOOLEAN bInitialized;	
	//BOOLEAN bEnabled;	/*Daemin is already enable*/
	UCHAR status;	
	UINT8	Band;
	BOOLEAN bTiggerByUser; /* If  rls trigger to apstop , it don't need control apcli interface */
	BOOLEAN bRlsControl;  /* If  rls trigger to apstop/apstart , it need update operation mode to daemon */

	struct _RLS_OPS *Ops;
	VOID *priv;
	RLS_INF_INFO Inf_2G;
	RLS_INF_INFO Inf_5G;
	
	RLS_INF_INFO Apcli_2G;
	RLS_INF_INFO Apcli_5G;
} RLS_CLI_TABLE, *PRLS_CLI_TABLE;

enum RLS_RETURN_CODE {
	RLS_SUCCESS = 0,
	RLS_FAILURE,
	RLS_NOT_INITIALIZED,
};

typedef struct _RLS_OPS {

	VOID (*ShowTableInfo)(
		struct _RTMP_ADAPTER *pAd,
		PRLS_CLI_TABLE table);

	INT (*SetEnable)(
		PRLS_CLI_TABLE table,
		BOOLEAN enable);

	INT (*CliLinkRsp)(
		struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev,
		BOOLEAN enable);

} RLS_OPS;

#endif /* RADIO_LINK_SELECTION */
#endif /* _RLS_DEF_H_ */

