#ifdef MTK_LICENSE
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
#endif /* MTK_LICENSE */
#ifndef _BAND_STEERING_DEF_H_
#define __BAND_STEERING_DEF_H__

#ifdef BAND_STEERING
#ifndef DOT11_N_SUPPORT
#error: "DOT11_N_SUPPORT must be enabled when using band steering"
#endif /* DOT11_N_SUPPORT */

#define BND_STRG_MAX_TABLE_SIZE	256
#define BND_STRG_TIMER_PERIOD	1000
#define BND_STRG_AGE_TIME		150000
#define BND_STRG_HOLD_TIME		90000
#define BND_STRG_CHECK_TIME_5G	30000
#define BND_STRG_RSSI_DIFF		30
#define BND_STRG_RSSI_LOW		-70
#define BND_STRG_AUTO_ONOFF_THRD 4000
#define P_BND_STRG_TABLE	(&pAd->ApCfg.BndStrgTable)
#define SIZE_OF_VHT_CAP_IE 		12

#define BND_STRG_DBG
#define BND_STRG_QA

struct _BNDSTRG_OPS;

enum BND_STRG_PRIORITY_FLAGS {
	fBND_STRG_PRIORITY_RSSI_DIFF		,
	fBND_STRG_PRIORITY_BAND_PERSIST		,
	fBND_STRG_PRIORITY_HT_SUPPORT		,
	fBND_STRG_PRIORITY_5G_RSSI			,
	fBND_STRG_PRIORITY_VHT_SUPPORT		,
	fBND_STRG_PRIORITY_NSS_SUPPORT		,
	fBND_STRG_PRIORITY_LOAD_BALANCE 	,
	fBND_STRG_PRIORITY_DEFAULT_2G		,
	fBND_STRG_PRIORITY_DEFAULT_5G		,
	fBND_STRG_PRIORITY_5G_RSSI_DYNAMIC	,
	fBND_STRG_PRIORITY_MAX				, //always to be the last
};

typedef struct _BND_STRG_ENTRY_STATISTICS{
	CHAR Rssi;
	UINT8 AuthReqCount;
} BND_STRG_ENTRY_STAT, *PBND_STRG_ENTRY_STAT;

typedef struct _BND_STRG_CLI_ENTRY{
	BOOLEAN bValid;
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
	UINT32		ConditionCheck;
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

enum ACTION_CODE{
	CONNECTION_REQ = 1,
	CLI_ADD,
	CLI_UPDATE,
	CLI_DEL,
	CLI_AGING_REQ,
	CLI_AGING_RSP,
	INF_STATUS_QUERY,
	INF_STATUS_RSP_2G,
	INF_STATUS_RSP_5G,
	TABLE_INFO,
	ENTRY_LIST,
	BNDSTRG_ONOFF,
	SET_RSSI_DIFF,
	SET_RSSI_LOW,
	SET_AGE_TIME,
	SET_HOLD_TIME,
	SET_CHECK_TIME,
	SET_MNT_ADDR,
	SET_CHEK_CONDITIONS,
	INF_STATUS_RSP_DBDC,
	SET_CND_PRIORITY,
	NVRAM_UPDATE
};

enum BND_STRG_STA_STATE{
	BNDSTRG_STA_INIT = 0,
	BNDSTRG_STA_ASSOC,
	BNDSTRG_STA_DISASSOC,
};

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
	BOOLEAN b2GInfReady;
	BOOLEAN b5GInfReady;
	CHAR	RssiDiff;	/* if Rssi2.4G > Rssi5G by RssiDiff, then allow client to connect 2.4G */
	CHAR	RssiLow;	/* if Rssi5G < RssiLow, then this client cannot connect to 5G */
	UINT32	AgeTime;		/* Entry Age Time (ms) */
	UINT32	HoldTime;		/* Time for holding 2.4G connection rsp (ms) */
	UINT32	CheckTime_5G;	/* Time for deciding if a client is 2.4G only (ms) */
	UINT32	CheckTime_2G;	/* Time for deciding if a client is 5G only (ms) */
	RALINK_TIMER_STRUCT Timer;
	CHAR	uc2GIfName[32];
	CHAR	uc5GIfName[32];
	UINT8	u2GIdx;
	UINT8	u5GIdx;
	UINT32 sent_action_code_counter[NVRAM_UPDATE]; //NVRAM_UPDATE = 22 in ACTION_CODE
	UINT32 received_action_code_counter[NVRAM_UPDATE]; //NVRAM_UPDATE = 22 in ACTION_CODE
#ifdef BND_STRG_DBG
	UCHAR MonitorAddr[MAC_ADDR_LEN];
#endif /* BND_STRG_DBG */
	UINT8		Band;
	UINT32 		AutoOnOffThrd;   /* Threshold to auto turn bndstrg on/off by 2.4G false CCA */
	UINT32 		DaemonPid;
	BOOLEAN 	bDaemonReady;
	UINT8 BndStrgCndPri[fBND_STRG_PRIORITY_MAX];
	UINT8  BndStrgCndPriSize;
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
	fBND_STRG_CLIENT_SUPPORT_2G			= (1 << 0),
	fBND_STRG_CLIENT_SUPPORT_5G			= (1 << 1),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_2G	= (1 << 2),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_5G	= (1 << 3),
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_2G		= (1 << 4),	
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_5G		= (1 << 5),
	fBND_STRG_CLIENT_LOW_RSSI_2G		= (1 << 6),
	fBND_STRG_CLIENT_LOW_RSSI_5G		= (1 << 7),
	fBND_STRG_CLIENT_IS_2G_ONLY			= (1 << 8),
	fBND_STRG_CLIENT_IS_5G_ONLY			= (1 << 9),
};

enum BND_STRG_FRAME_CHECK_FLAGS {
	fBND_STRG_FRM_CHK_PRB_REQ			= (1 << 0),
	fBND_STRG_FRM_CHK_ATH_REQ			= (1 << 1),
	fBND_STRG_FRM_CHK_ASS_REQ			= (1 << 2),
};

enum BND_STRG_CONDITION_CHECK_FLAGS {
	fBND_STRG_CND_RSSI_DIFF			= (1 << 0),
	fBND_STRG_CND_BAND_PERSIST		= (1 << 1),
	fBND_STRG_CND_HT_SUPPORT		= (1 << 2),
	fBND_STRG_CND_5G_RSSI			= (1 << 3),
	fBND_STRG_CND_VHT_SUPPORT		= (1 << 4),
	fBND_STRG_CND_NSS_SUPPORT		= (1 << 5),
	fBND_STRG_CND_LOAD_BALANCE 		= (1 << 6),
	fBND_STRG_CND_DEFAULT_2G		= (1 << 7),
	fBND_STRG_CND_DEFAULT_5G		= (1 << 8),
	fBND_STRG_CND_5G_RSSI_DYNAMIC	= (1 << 9),
};

#define OID_BNDSTRG_MSG				0x0950
/* Use for I/O between driver and daemon */
/* Must make sure the structure is the same as the one in daemon */
typedef struct _BNDSTRG_MSG{
	UINT8	 Action;
	UINT8	 ReturnCode;
	UINT32	 TableIndex;
	BOOLEAN OnOff;
	UINT8	Band;
	BOOLEAN b2GInfReady;
	UINT8	uc2GIfName[32];
	BOOLEAN b5GInfReady;
	UINT8	uc5GIfName[32];
	CHAR 	Rssi[4];
	CHAR 	RssiDiff;
	CHAR 	RssiLow;
	UINT8	FrameType;
	UINT32	Time;
	UINT32	ConditionCheck;
	UCHAR 	Addr[MAC_ADDR_LEN];
	BOOLEAN bAllowStaConnectInHt;
	UINT32  Control_Flags;
	UINT32  elapsed_time; /* ms */
	BOOLEAN bConnStatus;
	UINT8 	bVHTCapable;
	UINT8 	Nss;
	UINT32 	PriorityList[fBND_STRG_PRIORITY_MAX];
	UINT8 	PriorityListSize;
	BNDSTRG_NVRAM_CLIENT nvram_entry;
} BNDSTRG_MSG, *PBNDSTRG_MSG;

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

typedef struct _BNDSTRG_OPS {
	VOID (*ShowTableInfo)(
			PBND_STRG_CLI_TABLE table);
	
	VOID (*ShowTableEntries)(
			PBND_STRG_CLI_TABLE table);
	
	INT (*TableEntryAdd)(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr,
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

	INT (*SetHoldTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);

	INT (*SetCheckTime)(
			PBND_STRG_CLI_TABLE table,
			UINT8 Band,
			UINT32 Time);

	INT (*SetFrmChkFlag)(
			PBND_STRG_CLI_TABLE table,
			UINT32 FrmChkFlag);

	INT (*SetCndChkFlag)(
			PBND_STRG_CLI_TABLE table,
			UINT32 CndChkFlag);

	INT (*SetCndPriority)(
			PBND_STRG_CLI_TABLE table,
			UINT8 *CndPrio,
			UINT8 length);
#ifdef BND_STRG_DBG
	INT (*SetMntAddr)(
			PBND_STRG_CLI_TABLE table,
			PUCHAR Addr);
#endif /* BND_STRG_DBG */

	VOID (*MsgHandle)(
			struct _RTMP_ADAPTER *pAd,
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

