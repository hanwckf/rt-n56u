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
	band_steering_def.h
*/

#ifndef _BAND_STEERING_DEF_H_
#define __BAND_STEERING_DEF_H__

#ifdef BAND_STEERING
#ifndef DOT11_N_SUPPORT
#error: "DOT11_N_SUPPORT must be enabled when using band steering"
#endif /* DOT11_N_SUPPORT */

#define BND_STRG_MAX_TABLE_SIZE	64
#define BND_STRG_TIMER_PERIOD	1000
#define BND_STRG_AGE_TIME		0
#define BND_STRG_HOLD_TIME		50
#define BND_STRG_CHECK_TIME     30
#define BND_STRG_RSSI_DIFF		30
#define BND_STRG_RSSI_LOW		-70
#define BND_STRG_AUTO_ONOFF_THRD 4000
#define MAX_STEERING_COUNT		10
#define DWELL_TIME				300			// time in sec 
#define MAX_STEER_TIME_WINDOW	2*60*60 	// time in sec  
#define P_BND_STRG_TABLE(_x)	(&pAd->ApCfg.BndStrgTable[_x])
#define SIZE_OF_VHT_CAP_IE 		12
#define IS_2G_BAND(_p)			(((_p)&BAND_24G)==BAND_24G)
#define IS_5G_BAND(_p)			(((_p)&BAND_5G)==BAND_5G)


#define BND_STRG_DBG
#define BND_STRG_QA

struct _BNDSTRG_OPS;
#define BND_STRG_PRIORITY_MAX 32

typedef struct _BND_STRG_ENTRY_STATISTICS{
	CHAR Rssi;
	UINT8 AuthReqCount;
} BND_STRG_ENTRY_STAT, *PBND_STRG_ENTRY_STAT;

typedef struct _BND_STRG_CLI_ENTRY{
	BOOLEAN bValid;
	BOOLEAN bConnStatus;
	UINT8	TableIndex;
	UINT32 Control_Flags;
	ULONG   jiffies;		/* timestamp when insert-entry */
	UINT32  elapsed_time; /* ms */
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR BndStrg_Sta_State;	
	struct _BND_STRG_CLI_ENTRY *pNext;
} BND_STRG_CLI_ENTRY, *PBND_STRG_CLI_ENTRY;

/* for setting different band steering algorithms */
typedef struct _BND_STRG_ALG_CONTROL {
	UINT32 		FrameCheck;
} BND_STRG_ALG_CONTROL, *PBND_STRG_ALG_CONTROL;

#define NVRAM_TABLE_SIZE 		128

typedef struct _bndstrg_nvram_client{
	UINT8 Addr[MAC_ADDR_LEN];
	UINT8 Manipulable;
	UINT8 PhyMode;
	UINT8 Band;
	UINT8 Nss;
}BNDSTRG_NVRAM_CLIENT, *PBNDSTRG_NVRAM_CLIENT;

typedef struct _bndstrg_nvram_list{
	UINT8			Num;
	BNDSTRG_NVRAM_CLIENT nvram_entry[NVRAM_TABLE_SIZE]; 
}BNDSTRG_NVRAM_LIST, *PBNDSTRG_NVRAM_LIST;

enum PhyMode {
	fPhyMode_Legacy,
	fPhyMode_HT,
	fPhyMode_VHT,
};

#define OID_BNDSTRG_GET_NVRAM		0x0951
#define OID_BNDSTRG_SET_NVRAM		0x0952

typedef struct _BND_STRG_CLI_TABLE{
	BOOLEAN bInitialized;
	BOOLEAN bEnabled;
	UINT32 Size;
	BND_STRG_ALG_CONTROL AlgCtrl;
	BND_STRG_CLI_ENTRY Entry[BND_STRG_MAX_TABLE_SIZE];
	PBND_STRG_CLI_ENTRY Hash[HASH_TABLE_SIZE];
	NDIS_SPIN_LOCK Lock;
	struct _BNDSTRG_OPS *Ops;
	VOID *priv;
	BOOLEAN bInfReady;
	CHAR	RssiDiff;	/* if Rssi2.4G > Rssi5G by RssiDiff, then allow client to connect 2.4G */
	CHAR	RssiLow;	/* if Rssi5G < RssiLow, then this client cannot connect to 5G */
	UINT32	AgeTime;	/* Entry Age Time (ms) */
	UINT32	HoldTime;	/* Time for holding 2.4G connection rsp (ms) */
	UINT32	CheckTime;	/* Time for deciding if a client is 2.4G or 5G only (ms) */
	CHAR	ucIfName[32];
    UINT8	uIdx;
#ifdef BND_STRG_DBG
	UCHAR MonitorAddr[MAC_ADDR_LEN];
#endif /* BND_STRG_DBG */
	UINT8		Band;
    UINT8		Channel;
    BOOLEAN     bVHTCapable;
	UINT8		nss;
    INT8		ActiveCount;
	UINT32 		AutoOnOffThrd;   /* Threshold to auto turn bndstrg on/off by 2.4G false CCA */
	UINT32 		DaemonPid;
	BOOLEAN 	bDaemonReady;
	UINT8 BndStrgCndPri[BND_STRG_PRIORITY_MAX];
	UINT8  BndStrgCndPriSize;
	UINT32		dwell_time; 
	UINT32 		max_steer_time_window;
	UINT8 		max_steer_count;
} BND_STRG_CLI_TABLE, *PBND_STRG_CLI_TABLE;

enum BND_STRG_RETURN_CODE {
	BND_STRG_SUCCESS = 0,
	BND_STRG_INVALID_ARG,
	BND_STRG_RESOURCE_ALLOC_FAIL,
	BND_STRG_TABLE_FULL,
	BND_STRG_TABLE_IS_NULL,
	BND_STRG_NOT_INITIALIZED,
	BND_STRG_2G_INF_NOT_READY,
	BND_STRG_5G_INF_NOT_READY,
	BND_STRG_STA_IS_CONNECTED,
	BND_STRG_UNEXP,
};

enum BND_STRG_CONTROL_FLAGS {
	fBND_STRG_CLIENT_SUPPORT_2G			    = (1 << 0),
	fBND_STRG_CLIENT_SUPPORT_L5G			= (1 << 1),
	fBND_STRG_CLIENT_SUPPORT_H5G			= (1 << 2),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_2G	    = (1 << 3),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_L5G	= (1 << 4),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_H5G	= (1 << 5),
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_2G	    = (1 << 6),	
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_L5G	    = (1 << 7),
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_H5G		= (1 << 8),
	fBND_STRG_CLIENT_LOW_RSSI_2G		    = (1 << 9),
	fBND_STRG_CLIENT_LOW_RSSI_L5G		    = (1 << 10),
	fBND_STRG_CLIENT_LOW_RSSI_H5G		    = (1 << 11),
	fBND_STRG_CLIENT_IS_2G_ONLY			    = (1 << 12),
	fBND_STRG_CLIENT_IS_5G_ONLY			    = (1 << 13),
	fBND_STRG_CLIENT_SUPPORT_VHT		    = (1 << 14),

};

enum BND_STRG_FRAME_CHECK_FLAGS {
	fBND_STRG_FRM_CHK_PRB_REQ			= (1 << 0),
	fBND_STRG_FRM_CHK_ATH_REQ			= (1 << 1),
	fBND_STRG_FRM_CHK_ASS_REQ			= (1 << 2),
};

#define OID_BNDSTRG_MSG				0x0950
/* Use for I/O between driver and daemon */
/* Must make sure the structure is the same as the one in daemon */
typedef struct _BNDSTRG_CLI_EVENT{
	UCHAR		MacAddr[MAC_ADDR_LEN];
	UINT8		Action; /* add or delete table entry */
} BNDSTRG_CLI_EVENT, *PBNDSTRG_CLI_EVENT;

typedef struct _BNDSTRG_PROBE_EVENT{
	UCHAR		MacAddr[MAC_ADDR_LEN];
	UINT8		Band;
	UINT8		FrameType;
	CHAR		Rssi[3];
	BOOLEAN		bAuthCheck;
} BNDSTRG_PROBE_EVENT, *PBNDSTRG_PROBE_EVENT;

enum ACTION_CODE{
	CONNECTION_REQ = 1,
	CLI_ADD,
	CLI_UPDATE,
	CLI_DEL,
	CLI_AGING_REQ,
	CLI_AGING_RSP,
	CLI_STATUS_REQ,
	CLI_STATUS_RSP,
	QLOAD_STATUS_REQ,
	QLOAD_STATUS_RSP,
	INF_STATUS_QUERY,
	INF_STATUS_RSP,
//	INF_STATUS_RSP_2G,
//	INF_STATUS_RSP_5G,
	TABLE_INFO,
	ENTRY_LIST,
	BNDSTRG_ONOFF,
	SET_RSSI_DIFF,
	SET_RSSI_LOW,
	SET_AGE_TIME,
	SET_HOLD_TIME,
	SET_CHECK_TIME,
	SET_MNT_ADDR,
//	SET_CHEK_CONDITIONS,
	SET_CND_PRIORITY,
	SET_PARAM,
	NVRAM_UPDATE,
	REJECT_EVENT,
	SET_DWELL_TIME,
	SET_MAX_STEER,
	SET_MAX_STEER_TIME_WINDOW,
	HEARTBEAT_MONITOR
};

enum BND_STRG_STA_STATE{
	BNDSTRG_STA_INIT = 0,
	BNDSTRG_STA_ASSOC,
	BNDSTRG_STA_DISASSOC,
};

enum BND_SET_PARAM_TYPE{
	BND_SET_STEERING_NUM,
	BND_SET_ASSOC_BL_TH,
	BND_SET_QLOAD_TH,
	BND_SET_MIN_RSSI_TH,
	//BND_SET_LB_CND,
	//BND_SET_LB_PRI,
	BND_SET_NSS_TH,
	BND_SET_STA_POLL_PRD,
	BND_SET_DAEMON_STATE,//debug purpose
};

/* HEARTBEAT_MONITOR */
struct bnd_msg_heartbeat {
    CHAR ucIfName[32];
};

/* CONNECTION_REQ */
struct bnd_msg_conn_req {  
    UINT8	Band;
    UINT8   Channel;
    UINT8	FrameType;
    BOOLEAN bAllowStaConnectInHt;
    UINT8 	bVHTCapable;
	UINT8 	Nss;
    CHAR 	Rssi[4];
    UCHAR 	Addr[MAC_ADDR_LEN];
	UINT8 	qload;
	UINT8	chan_busy_load;
	UINT8	extchan_busy_load;
	UINT8	obss_load;
	UINT8	edcca_load;
	UINT8	myair_load;
	UINT8	mytxair_load;
	UINT8	myrxair_load;
};

/* CLI_ADD */
struct bnd_msg_cli_add {
    UINT8 TableIndex;
    UCHAR Addr[MAC_ADDR_LEN];
};

/* CLI_UPDATE */
struct bnd_msg_cli_update {
    UINT8 Band;
	UINT8 Channel;
    UINT8 TableIndex;
    BOOLEAN bConnStatus;
    BOOLEAN bAllowStaConnectInHt;
    UINT8 	bVHTCapable;
	UINT8 	Nss;
    UINT32 Control_Flags;
	UINT32 elapsed_time; /* ms */
    UCHAR Addr[MAC_ADDR_LEN];
};

/* CLI_DEL */
struct bnd_msg_cli_del {
    UINT8 TableIndex;
    UCHAR Addr[MAC_ADDR_LEN];
};

/* CLI_AGING_REQ */
struct bnd_msg_cli_aging_req {
    UINT8 TableIndex;
    UCHAR Addr[MAC_ADDR_LEN];
};

/* CLI_AGING_RSP */
struct bnd_msg_cli_aging_rsp {
    UINT8 Band;
    UINT8 TableIndex;
    UINT8 ReturnCode;
    UCHAR Addr[MAC_ADDR_LEN];
	UINT8 channel;
};

/* CLI_STATUS_REQ */
struct bnd_msg_cli_status_req {
    //UINT8 TableIndex;
    //UCHAR Addr[MAC_ADDR_LEN];
};

/* CLI_STATUS_RSP */
struct bnd_msg_cli_status_rsp {
    UINT8 TableIndex;
	UINT8 ReturnCode;
	UCHAR 	Addr[MAC_ADDR_LEN];
	char 	data_Rssi;
	UINT32 	data_tx_Rate;
	UINT32 	data_rx_Rate;
	UINT64	data_tx_Byte;
	UINT64	data_rx_Byte;
	UINT8 	data_tx_Phymode;
	UINT8 	data_rx_Phymode;
	UINT8 	data_tx_mcs;
	UINT8 	data_rx_mcs;
	UINT8 	data_tx_bw;
	UINT8 	data_rx_bw;
	UINT8 	data_tx_sgi;
	UINT8 	data_rx_sgi;
	UINT8 	data_tx_stbc;
	UINT8 	data_rx_stbc;
	UINT8 	data_tx_ant;
	UINT8 	data_rx_ant;
	UINT64	data_tx_packets;
	UINT64	data_rx_packets;
};

/* CLI_STATUS_REQ */
struct bnd_msg_qload_status_req {
    CHAR ucIfName[32];
//	u8 band;
//	u8 Channel;
};
/* QLOAD_STATUS_RSP */
struct bnd_msg_qload_status_rsp {
	UINT8 ReturnCode;
	UINT8 band;
	UINT8 Channel;
	//TBD
	UINT8 	qload;
	UINT8	chan_busy_load;
	UINT8	obss_load;
	UINT8	edcca_load;
	UINT8	myair_load;
	UINT8	mytxair_load;
	UINT8	myrxair_load;
};

/* INF_STATUS_QUERY */
struct bnd_msg_inf_status_req {
    CHAR ucIfName[32];
};


/* INF_STATUS_RSP_5G / INF_STATUS_RSP_2G */
struct bnd_msg_inf_status_rsp {
    BOOLEAN bInfReady;
    UINT8 Channel;
    BOOLEAN bVHTCapable;
    ULONG table_src_addr;
    CHAR ucIfName[32];
	UINT8 nvram_support;
	UINT8 nss;
	UINT8 band;
	UINT32 table_size;
};

/* TABLE_INFO */
/* ENTRY_LIST */

/* BNDSTRG_ONOFF */
struct bnd_msg_onoff {
    UINT8 Band;
    UINT8 Channel;
    BOOLEAN OnOff;
    CHAR ucIfName[32];
};

/* SET_RSSI_DIFF / SET_RSSI_LOW */
struct bnd_msg_rssi {
    CHAR Rssi;
};

/* SET_AGE_TIME / SET_HOLD_TIME / SET_CHECK_TIME */
struct bnd_msg_time {
    UINT8 Band;
    UINT32 Time;
};

/* set max steer count */
struct bnd_msg_max_steer_count {
    UINT8 max_count;
	UINT8 Band;
};

/* SET_MNT_ADDR */
struct bnd_msg_mnt_addr {
    UCHAR Addr[MAC_ADDR_LEN];
};

/* SET_CND_PRIORITY */
struct bnd_msg_cnd_priority {
	UINT8 	PriorityListSize;
    UINT32 	PriorityList[BND_STRG_PRIORITY_MAX];
};

struct bnd_set_steering_num {
	UINT32 	total_num;
};
struct bnd_set_assoc_bl_th {
	UINT32 	assoc_bl_th; /* percentage, must <=100*/
};
struct bnd_set_qload_th {
	UINT32 	qload_th; /* percentage, must <=100*/
};
struct bnd_set_min_rssi_th {
	INT32 	min_rssi;
};
struct bnd_set_nss_th {
	UINT32 	nss_th;
};
struct bnd_set_sta_poll_period {
	UINT32 	period;
};
enum bndstrg_state {
	BNDSTRG_INIT,
	BNDSTRG_INF_POLL,
	BNDSTRG_TBL_EN,
	BNDSTRG_TBL_READY,
	BNDSTRG_LEAVE,
	BNDSTRG_FROZEN,//debug mode
};
struct bnd_set_daemon_state {
	enum bndstrg_state	state;
};
/* SET_PARAM */
struct bnd_msg_set_param {
	UINT32 	type;
	UINT8	channel;
	UINT8 	band;
	union {
        struct bnd_set_steering_num steering;
		struct bnd_set_assoc_bl_th assoc;
		struct bnd_set_qload_th qload;
		struct bnd_set_min_rssi_th min_rssi;
		struct bnd_set_nss_th nss;
		struct bnd_set_sta_poll_period period;
		struct bnd_set_daemon_state daemon_state;
	} data;
};

/* NVRAM_UPDATE */
struct bnd_msg_nvram_entry_update {
    UCHAR Addr[MAC_ADDR_LEN];
	BNDSTRG_NVRAM_CLIENT nvram_entry;
};

struct bnd_msg_reject_body {
    UINT32 DaemonPid;
};

/*display type for list*/
struct bnd_msg_display_entry_list {
    UINT32 display_type;
	UINT8 filer_band;
	UINT8 channel;
};
typedef struct _BNDSTRG_MSG{
   UINT8   Action;
   union {
        struct bnd_msg_conn_req conn_req;
        struct bnd_msg_cli_add cli_add;
        struct bnd_msg_cli_update cli_update;
        struct bnd_msg_cli_del cli_del;
        struct bnd_msg_cli_aging_req cli_aging_req;
        struct bnd_msg_cli_aging_rsp cli_aging_rsp;
        struct bnd_msg_cli_status_req cli_status_req;
        struct bnd_msg_cli_status_rsp cli_status_rsp;
		struct bnd_msg_qload_status_req qload_status_req;
		struct bnd_msg_qload_status_rsp qload_status_rsp;
        struct bnd_msg_inf_status_req inf_status_req;
        struct bnd_msg_inf_status_rsp inf_status_rsp;
        struct bnd_msg_onoff onoff;
        struct bnd_msg_rssi rssi;
        struct bnd_msg_time time;
        struct bnd_msg_mnt_addr mnt_addr;
        struct bnd_msg_cnd_priority cnd_priority;
        struct bnd_msg_set_param set_param;
        struct bnd_msg_nvram_entry_update entry_update;
        struct bnd_msg_reject_body reject_body;
        struct bnd_msg_display_entry_list display_type;
		struct bnd_msg_max_steer_count max_steer_count;
		struct bnd_msg_heartbeat heartbeat;
   } data;
} BNDSTRG_MSG, *PBNDSTRG_MSG;

typedef struct _BNDSTRG_OPS {  
	VOID (*ShowTableInfo)(
			PBND_STRG_CLI_TABLE table);
	
	VOID (*ShowTableEntries)(
			PBND_STRG_CLI_TABLE table,UINT32 display_type);
        
	INT (*TableEntryAdd)(
			BND_STRG_CLI_TABLE *table,
			struct bnd_msg_cli_add *cli_add,
			PBND_STRG_CLI_ENTRY *entry_out);
	
	INT (*TableEntryDel)(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr,
			UINT32 Index);
	
	PBND_STRG_CLI_ENTRY (*TableLookup)(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr);
	
	BOOLEAN (*CheckConnectionReq)(
			struct _RTMP_ADAPTER *pAd,
			struct wifi_dev *wdev,
			PBND_STRG_CLI_TABLE table,
			PUCHAR pSrcAddr,
			UINT8 FrameType,
			PCHAR Rssi,
			BOOLEAN bAllowStaConnectInHt,
			BOOLEAN bVHTCap,
			UINT8 nss);

	INT (*SetEnable)(
			PBND_STRG_CLI_TABLE table,
			BOOLEAN enable);

	INT (*SetRssiDiff)(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiDiff);

	INT (*SetRssiLow)(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiLow);

	INT (*SetAgeTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);	

	INT	(*SetMaxSteerCnt)(
			PBND_STRG_CLI_TABLE table,
			UINT8 MAxSteerCnt);
		
	INT	(*SetDwellTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);
	
	INT (*SetSteerTimeWindow)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);

	INT (*SetHoldTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);

	INT (*SetCheckTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);

	INT (*SetFrmChkFlag)(
			PBND_STRG_CLI_TABLE table,
			UINT32 FrmChkFlag);

	INT (*SetCndPriority)(
			PBND_STRG_CLI_TABLE table,
			UINT8 *CndPrio,
			UINT8 length);

	INT (*SetParam)(
		PBND_STRG_CLI_TABLE table,
		UINT8 *ptr,
		UINT32 type);

#ifdef BND_STRG_DBG
	INT (*SetMntAddr)(
			PBND_STRG_CLI_TABLE table,
			PUCHAR Addr);
#endif /* BND_STRG_DBG */

	VOID (*MsgHandle)(
			struct _RTMP_ADAPTER *pAd,
			PBND_STRG_CLI_TABLE table,
			BNDSTRG_MSG *msg);

	BOOLEAN (*NvramTableLookup)(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr);
	
	BOOLEAN (*NvramTableEntryAdd)(
			BND_STRG_CLI_TABLE *table,
			PBNDSTRG_NVRAM_CLIENT msg);

} BNDSTRG_OPS;

#endif /* BAND_STEERING */
#endif /* _BAND_STEERING_DEF_H_ */

