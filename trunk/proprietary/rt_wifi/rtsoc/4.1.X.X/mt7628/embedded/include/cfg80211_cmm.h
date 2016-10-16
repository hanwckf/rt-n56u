/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All CFG80211 Function Prototype.

***************************************************************************/


#ifndef __CFG80211CMM_H__
#define __CFG80211CMM_H__

#ifdef RT_CFG80211_SUPPORT

#define RTMP_CFG80211_HOSTAPD_ON(__pAd) (__pAd->net_dev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP)

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE 	
#define CFG80211_GetEventDevice(__pAd) __pAd->cfg80211_ctrl.dummy_p2p_net_dev
#else
#define CFG80211_GetEventDevice(__pAd) __pAd->net_dev
#endif	/* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef RT_CFG80211_P2P_SUPPORT
#define CFG_GO_BSSID_IDX (MAIN_MBSSID + 1)

//yiwei debug for P2P 7.1.3
//yiwei tmp hard code
#define IS_SW_NOA_TIMER(_A) (1)
#define P2P_OPPS_BIT		0x80

/*
 *  Macros for bit check
*/

#define CFG80211_P2P_TEST_BIT(_M, _F)      (((_M) & (_F)) != 0)

#define CFG_P2P_DISABLE	0x00000000
#define CFG_P2P_GO_UP		0x00000001
#define CFG_P2P_CLI_UP		0x00000002

#define IS_CFG80211_P2P_ABSENCE(_A)	(((_A)->cfg80211_ctrl.bPreKeepSlient) || ((_A)->cfg80211_ctrl.bKeepSlient))


typedef	struct	_P2PCLIENT_NOA_SCHEDULE	{
	BOOLEAN		bValid;
	BOOLEAN		bInAwake;


	UCHAR		Token;

	
	ULONG		SwTimerTickCounter; /* this Counter os used for sw-base NoA implementation tick counter */

	ULONG		CurrentTargetTimePoint; /* For sw-base method NoA usage */
	ULONG           NextTargetTimePoint;

	UCHAR           Count;
	ULONG           Duration;
	ULONG           Interval;
	ULONG           StartTime;
	ULONG		OngoingAwakeTime; /* this time will keep increasing as time go by. indecate the current awake time point */

	
	ULONG		LastBeaconTimeStamp;
}	P2PCLIENT_NOA_SCHEDULE, *PP2PCLIENT_NOA_SCHEDULE;


typedef struct __CFG_P2P_ENTRY_PARM
{
	UCHAR					CTWindow;	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
	P2PCLIENT_NOA_SCHEDULE	NoADesc[1];	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
}CFG_P2P_ENTRY_PARM, *PCFG_P2P_ENTRY_PARM;


typedef struct {
    UCHAR   Eid;
    UCHAR   Len[2];
    CHAR   Octet[1];
} P2PEID_STRUCT,*PP2PEID_STRUCT;



VOID CFG80211_P2PCTWindowTimer(
	IN PVOID	SystemSpecific1, 
	IN PVOID	FunctionContext, 
	IN PVOID	SystemSpecific2, 
	IN PVOID	SystemSpecific3);

VOID CFG80211_P2pSwNoATimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID CFG80211_P2pPreAbsenTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

#endif /* RT_CFG80211_P2P_SUPPORT */

#ifdef CFG_TDLS_SUPPORT
#define CATEGORY_TDLS				12
#define	PROTO_NAME_TDLS				2
#define TDLS_TIMEOUT				5000	// unit: msec

#define TDLS_ELM_LEN_LINK_IDENTIFIER		18
#define LENGTH_TDLS_H				24
#define LENGTH_TDLS_PAYLOAD_H		3  /* payload type(1 byte) + category(1 byte) + action(1 byte) */

/* Information element ID defined in 802.11z D13.0 specification. */
#define IE_TDLS_LINK_IDENTIFIER			101
#define IE_TDLS_WAKEUP_SCHEDULE			102
#define IE_TDLS_CHANNEL_SWITCH_TIMING	104
#define IE_TDLS_PTI_CONTROL				105
#define IE_TDLS_PU_BUFFER_STATUS		106


#define TDLS_ACTION_CODE_SETUP_REQUEST				0
#define TDLS_ACTION_CODE_SETUP_RESPONSE				1
#define TDLS_ACTION_CODE_SETUP_CONFIRM				2
#define TDLS_ACTION_CODE_TEARDOWN					3
#define TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION	4 /* for TDLS UAPSD */
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST		5
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE	6
#define TDLS_ACTION_CODE_PEER_PSM_REQUEST			7
#define TDLS_ACTION_CODE_PEER_PSM_RESPONSE			8
#define TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE		9 /* for TDLS UAPSD */
#define TDLS_ACTION_CODE_DISCOVERY_REQUEST			10
#define TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_REQ			11
#define TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_RSP			12
#define TDLS_ACTION_CODE_AUTO_TEARDOWN				13 //  auto teardown

#define TDLS_CHANNEL_SWITCH_TIME		11000 /* micro seconds */
#define TDLS_CHANNEL_SWITCH_TIMEOUT		19000 /* micro seconds */


#define IS_TDLS_SUPPORT(_P) \
                ((_P)->StaCfg.wpa_supplicant_info.CFG_Tdls_info.bCfgTDLSCapable == TRUE)
#define TDLS_TIMESTAMP_GET(__pAd, __TimeStamp)			\
{													\
	UINT32 __CSR=0; UINT64 __Value64;				\
	RTMP_IO_READ32((__pAd), TSF_TIMER_DW0, &__CSR); \
	__TimeStamp = (UINT32)__CSR;					\
}


#define MAX_NUM_OF_CFG_TDLS_ENTRY	4

enum {
	to_AP=0,
	to_PEER
};
typedef struct _CFG_TDLS_ENTRY {
	BOOLEAN			EntryValid;
	UCHAR			MacAddr[MAC_ADDR_LEN];		
	RALINK_TIMER_STRUCT	Timer;			/* Use to time out Peer Traffic Indicator */
	UCHAR TPK[LEN_PMK];	/* TPK-KCK(16 bytes) for MIC + TPK-TP (16 bytes) for data */
	
	UCHAR			Token;			/* Dialog token */
	BOOLEAN			bInitiator;		/* TRUE: I am TDLS Initiator STA, FALSE: I am TDLS Responder STA */
	UCHAR			MacTabMatchWCID;
	PVOID			pAd;
#ifdef UAPSD_SUPPORT
	BOOLEAN			FlgIsWaitingUapsdTraRsp; /* 1: waiting for traffic rsp frame */
	UCHAR			UapsdTraIndDelayCnt; /* used to send a detect traffic ind */
	BOOLEAN			FlgIsUapsdTraRspRcv; /* if we have ever received rsp frame */
	UCHAR			QosCapability;
#endif /* UAPSD_SUPPORT */	
	
	UCHAR					HtCapabilityLen;
	HT_CAPABILITY_IE		HtCapability;

} CFG_TDLS_ENTRY, *PCFG_TDLS_ENTRY;

typedef struct _CFG_TDLS_CHSW_PARAM {
	UCHAR cmd;
	UINT8 basech;
	UINT8 offch;
	UCHAR bw_base;
	UCHAR bw_off;
	UCHAR initiator;
	UINT16 stay_time;
	UCHAR ext_base;
	UCHAR ext_off;
	UINT32 start_time_tsf;
	UINT16 switch_time;
	UINT16 switch_timeout;
} CFG_TDLS_CHSW_PARAM, *PCFG_TDLS_CHSW_PARAM;



enum tdls_operation {
	TDLS_DISCOVERY_REQ,
	TDLS_SETUP,
	TDLS_TEARDOWN,
	TDLS_ENABLE_LINK,
	TDLS_DISABLE_LINK,
};

enum tdls_entry_op
{
	tdls_insert_entry=0,
	tdls_delete_entry
};
#endif /* CFG_TDLS_SUPPORT */


#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211CMM_H__ */


