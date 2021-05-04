/*
 ***************************************************************************
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

    Module Name:
    twt.h

    Abstract:
    twt spec. related

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef _TWT_H_
#define _TWT_H_

#ifdef MT_MAC

/* TWT definitions for protocol */
#define TWT_SUPPORT_DISABLE 0
#define TWT_SUPPORT_ENABLE 1
#define TWT_SUPPORT_MANDATORY 2
#define TWT_SUPPORT_TYPE_NUM 3

/* TWT Element (802.11ax D3.0) */
#define TWT_CTRL_NDP_PAGING_INDICATOR (1 << 0)
#define TWT_CTRL_RESPONDER_PM_MODE (1 << 1)
#define TWT_CTRL_NEGO_TYPE_SHIFT 2
#define TWT_CTRL_NEGO_TYPE_MASK (0x03 << TWT_CTRL_NEGO_TYPE_SHIFT)
#define TWT_CTRL_INFO_FRM_DIS (1 << 4)
#define TWT_CTRL_WAKE_DUR_UNIT (1 << 5)

#define TWT_REQ_TYPE_REQUEST (1 << 0)
#define TWT_REQ_TYPE_SETUP_COMMAND_SHIFT 1
#define TWT_REQ_TYPE_SETUP_COMMAND_MASK (0x07 << TWT_REQ_TYPE_SETUP_COMMAND_SHIFT)
#define TWT_REQ_TYPE_TRIGGER (1 << 4)
#define TWT_REQ_TYPE_IMPLICIT (1 << 5)
#define TWT_REQ_TYPE_FLOW_TYPE (1 << 6)
#define TWT_REQ_TYPE_FLOW_IDENTIFIER_SHIFT 7
#define TWT_REQ_TYPE_FLOW_IDENTIFIER_MASK (0x3 << TWT_REQ_TYPE_FLOW_IDENTIFIER_SHIFT)
#define TWT_REQ_TYPE_WAKE_INTERVAL_EXPONENT_SHIFT 10
#define TWT_REQ_TYPE_WAKE_INTERVAL_EXPONENT_MASK (0x1f << TWT_REQ_TYPE_WAKE_INTERVAL_EXPONENT_SHIFT)
#define TWT_REQ_TYPE_PROTECTION (1 << 15)

#define TWT_SETUP_CMD_REQUEST 0
#define TWT_SETUP_CMD_SUGGEST 1
#define TWT_SETUP_CMD_DEMAND 2
#define TWT_SETUP_CMD_GROUPING 3
#define TWT_SETUP_CMD_ACCEPT 4
#define TWT_SETUP_CMD_ALTERNATE 5
#define TWT_SETUP_CMD_DICTATE 6
#define TWT_SETUP_CMD_REJECT 7

/* Tsf from which request */
#define TSF_FROM_SETUP_CMD_REQUEST  0
#define TSF_FROM_SETUP_CMD_SUGGEST  1
#define TSF_FROM_SETUP_CMD_DEMAND   2
#define TSF_FROM_TWT_INFO_FRAME     8

/* TWT flow id max num */
#define TWT_FLOW_ID_MAX_NUM			8

#ifdef APCLI_SUPPORT
#define TWT_REQ_TYPE_TWT_REQUEST_OFFSET                0
#define TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET          1
#define TWT_REQ_TYPE_TRIGGER_OFFSET                    4
#define TWT_REQ_TYPE_IMPLICIT_LAST_BCAST_PARAM_OFFSET  5
#define TWT_REQ_TYPE_FLOWTYPE_OFFSET                   6
#define TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET        7
#define TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET        10
#define TWT_REQ_TYPE_TWT_PROTECTION_OFFSET             15

/* TWT element */
#define TWT_REQ_TYPE_TWT_REQUEST                    BIT(0)
#define TWT_REQ_TYPE_TWT_SETUP_COMMAND              BITS(1, 3)
#define TWT_REQ_TYPE_TWT_TRIGGER                        BIT(4)
#define TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM      BIT(5)
#define TWT_REQ_TYPE_TWT_FLOWTYPE                       BIT(6)
#define TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER            BITS(7, 9)
#define TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP            BITS(10, 14)
#define TWT_REQ_TYPE_TWT_PROTECTION                 BIT(15)

#define TWT_REQ_TYPE_TWT_REQUEST_OFFSET                0
#define TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET          1
#define TWT_REQ_TYPE_TRIGGER_OFFSET                    4
#define TWT_REQ_TYPE_IMPLICIT_LAST_BCAST_PARAM_OFFSET  5
#define TWT_REQ_TYPE_FLOWTYPE_OFFSET                   6
#define TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET        7
#define TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET        10
#define TWT_REQ_TYPE_TWT_PROTECTION_OFFSET             15

#define TWT_SETUP_CMD_REQUEST                       0
#define TWT_SETUP_CMD_SUGGEST                       1
#define TWT_SETUP_CMD_DEMAND                        2
#define TWT_SETUP_CMD_GROUPING                      3
#define TWT_SETUP_CMD_ACCEPT                        4
#define TWT_SETUP_CMD_ALTERNATE                     5
#define TWT_SETUP_CMD_DICTATE                       6
#define TWT_SETUP_CMD_REJECT                        7

/* TWT Flow Field in teardown frame */
#define TWT_TEARDOWN_FLOW_ID                        BITS(0, 2)

/* TWT Information Field */
#define TWT_INFO_FLOW_ID                            BITS(0, 2)
#define TWT_INFO_RESP_REQUESTED                     BIT(3)
#define TWT_INFO_NEXT_TWT_REQ                       BIT(4)
#define TWT_INFO_NEXT_TWT_SIZE                      BITS(5, 6)
#define TWT_INFO_BCAST_RESCHED                      BIT(7)

#define TWT_INFO_FLOW_ID_OFFSET                     0
#define TWT_INFO_RESP_REQUESTED_OFFSET              3
#define TWT_INFO_NEXT_TWT_REQ_OFFSET                4
#define TWT_INFO_NEXT_TWT_SIZE_OFFSET               5
#define TWT_INFO_BCAST_RESCHED_OFFSET               7

#define NEXT_TWT_SUBFIELD_ZERO_BIT                  0
#define NEXT_TWT_SUBFIELD_32_BITS                   1
#define NEXT_TWT_SUBFIELD_48_BITS                   2
#define NEXT_TWT_SUBFIELD_64_BITS                   3

/* TWT related definitions */
#define TWT_AGRT_MAX_NUM        16
#define TWT_GRP_MAX_NUM         8
#define TWT_GRP_MAX_MEMBER_CNT  8
#endif /* APCLI_SUPPORT */

struct GNU_PACKED twt_ie {
	UINT8 elem_id;
	UINT8 len;
	UINT8 control;
	UINT16 req_type;
	UINT32 target_wake_time[2];
	/* twt group assignment: not used */
	UINT8 duration;
	UINT16 mantissa;
	/* broadcast twt id: not used */
	UINT8 channel;
	/* ndp paging: not used */
};

/* TWT setup frame action filed format */
#define CATE_S1G_ACTION_TWT_SETUP 6

struct GNU_PACKED frame_twt_setup {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 token;
	struct twt_ie twt_ie;
};

/* TWT teardown frame action filed format */
#define CATE_S1G_ACTION_TWT_TEARDOWN 7

struct GNU_PACKED frame_teardown {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 twt_flow_id;
};

/* TWT information frame action filed format */
#define CATE_S1G_ACTION_TWT_INFO 11
#define TWT_INFO_1_FLOW_IDENTIFIER_SHIFT 0
#define TWT_INFO_1_FLOW_IDENTIFIER_MASK (0x3 << TWT_INFO_1_FLOW_IDENTIFIER_SHIFT)
#define TWT_INFO_1_RSP_REQ 3
#define TWT_INFO_1_NEXT_TWT_REQ 4
#define TWT_INFO_1_NEXT_SUBFIELD_SIZE_SHIFT 5
#define TWT_INFO_1_NEXT_SUBFIELD_SIZE_MASK (0x3 << TWT_INFO_1_NEXT_SUBFIELD_SIZE_SHIFT)
#define TWT_INFO_1_BROADCAST_RESCHEDULE 7

#ifdef APCLI_SUPPORT
#define TSF_OFFSET_FOR_EMU	   (1 * 1000 * 1000)	/* after 1 sec */
#define TSF_OFFSET_FOR_AGRT_ADD	   (5 * 1000 * 1000)	/* after 5 sec */
#define TSF_OFFSET_FOR_AGRT_RESUME (5 * 1000 * 1000)	/* after 5 sec */

#define SET_TWT_RT_REQUEST(fgReq) \
	(((fgReq) << TWT_REQ_TYPE_TWT_REQUEST_OFFSET) & \
		TWT_REQ_TYPE_TWT_REQUEST)

#define SET_TWT_RT_SETUP_CMD(ucSetupCmd) \
	(((ucSetupCmd) << TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET) & \
		TWT_REQ_TYPE_TWT_SETUP_COMMAND)

#define SET_TWT_RT_TRIGGER(fgTrigger) \
	(((fgTrigger) << TWT_REQ_TYPE_TRIGGER_OFFSET) & TWT_REQ_TYPE_TRIGGER)

#define SET_TWT_RT_FLOW_TYPE(fgUnannounced) \
	(((fgUnannounced) << TWT_REQ_TYPE_FLOWTYPE_OFFSET) & \
		TWT_REQ_TYPE_TWT_FLOWTYPE)

#define SET_TWT_RT_FLOW_ID(ucTWTFlowId) \
	(((ucTWTFlowId) << TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET) & \
		TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER)

#define SET_TWT_RT_WAKE_INTVAL_EXP(ucWakeIntvlExponent) \
	(((ucWakeIntvlExponent) << TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET) & \
		TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP)

#define SET_TWT_RT_PROTECTION(fgProtect) \
	(((fgProtect) << TWT_REQ_TYPE_TWT_PROTECTION_OFFSET) & \
		TWT_REQ_TYPE_TWT_PROTECTION)

/* Macros for getting request type bit fields in TWT IE */
#define GET_TWT_RT_REQUEST(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_REQUEST) >> \
		TWT_REQ_TYPE_TWT_REQUEST_OFFSET)

#define GET_TWT_RT_SETUP_CMD(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_SETUP_COMMAND) >> \
		TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET)

#define GET_TWT_RT_TRIGGER(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TRIGGER) >> TWT_REQ_TYPE_TRIGGER_OFFSET)

#define GET_TWT_RT_FLOW_TYPE(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_FLOWTYPE) >> TWT_REQ_TYPE_FLOWTYPE_OFFSET)

#define GET_TWT_RT_FLOW_ID(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER) >> \
		TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET)

#define GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP) >> \
		TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET)

#define GET_TWT_RT_PROTECTION(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_PROTECTION) >> \
		TWT_REQ_TYPE_TWT_PROTECTION_OFFSET)

/* Macros to set TWT info field */
#define SET_TWT_INFO_FLOW_ID(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_FLOW_ID_OFFSET) & TWT_INFO_FLOW_ID)

#define SET_TWT_INFO_RESP_REQUESTED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_RESP_REQUESTED_OFFSET) & \
	TWT_INFO_RESP_REQUESTED)

#define SET_TWT_INFO_NEXT_TWT_REQ(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_NEXT_TWT_REQ_OFFSET) & \
	TWT_INFO_NEXT_TWT_REQ)

#define SET_TWT_INFO_NEXT_TWT_SIZE(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_NEXT_TWT_SIZE_OFFSET) & \
	TWT_INFO_NEXT_TWT_SIZE)

#define SET_TWT_INFO_BCAST_RESCHED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_BCAST_RESCHED_OFFSET) & \
	TWT_INFO_BCAST_RESCHED)

/* Macros to get TWT info field */
#define GET_TWT_INFO_FLOW_ID(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_FLOW_ID) >> TWT_INFO_FLOW_ID_OFFSET)

#define GET_TWT_INFO_RESP_REQUESTED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_RESP_REQUESTED) >> \
	TWT_INFO_RESP_REQUESTED_OFFSET)

#define GET_TWT_INFO_NEXT_TWT_REQ(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_NEXT_TWT_REQ) >> \
	TWT_INFO_NEXT_TWT_REQ_OFFSET)

#define GET_TWT_INFO_NEXT_TWT_SIZE(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_NEXT_TWT_SIZE) >> \
	TWT_INFO_NEXT_TWT_SIZE_OFFSET)

#define GET_TWT_INFO_BCAST_RESCHED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_BCAST_RESCHED) >> \
	TWT_INFO_BCAST_RESCHED_OFFSET)

enum ENUM_MID_TWT_REQ_FSM_T {
	MID_TWT_REQ_FSM_START = 0,
	MID_TWT_REQ_FSM_TEARDOWN,
	MID_TWT_REQ_FSM_SUSPEND,
	MID_TWT_REQ_FSM_RESUME,
	MID_TWT_REQ_IND_RESULT,
	MID_TWT_REQ_IND_SUSPEND_DONE,
	MID_TWT_REQ_IND_RESUME_DONE,
	MID_TWT_REQ_IND_TEARDOWN_DONE,
	MID_TWT_REQ_IND_INFOFRM,
	MID_TWT_PARAMS_SET,
	MID_TWT_REQ_FSM_NUM,
};

enum ENUM_TWT_REQUESTER_STATE_T {
	TWT_REQ_STATE_IDLE = 0,
	TWT_REQ_STATE_REQTX,
	TWT_REQ_STATE_WAIT_RSP,
	TWT_REQ_STATE_SUSPENDING,
	TWT_REQ_STATE_SUSPENDED,
	TWT_REQ_STATE_RESUMING,
	TWT_REQ_STATE_TEARING_DOWN,
	TWT_REQ_STATE_RX_TEARDOWN,
	TWT_REQ_STATE_RX_INFOFRM,
	TWT_REQ_STATE_NUM
};

/* Definitions for action control of TWT params */
enum {
	TWT_PARAM_ACTION_NONE = 0,
	TWT_PARAM_ACTION_ADD_BYPASS = 1, /* bypass nego & add an agrt */
	TWT_PARAM_ACTION_DEL_BYPASS = 2, /* bypass proto & del an agrt */
	TWT_PARAM_ACTION_MOD_BYPASS = 3, /* bypass proto & modify an agrt */
	TWT_PARAM_ACTION_ADD = 4,
	TWT_PARAM_ACTION_DEL = 5,
	TWT_PARAM_ACTION_SUSPEND = 6,
	TWT_PARAM_ACTION_RESUME = 7,
	TWT_PARAM_ACTION_MAX
};

enum TWT_GET_TSF_REASON {
	TWT_GET_TSF_FOR_ADD_AGRT_BYPASS = 1,
	TWT_GET_TSF_FOR_ADD_AGRT = 2,
	TWT_GET_TSF_FOR_RESUME_AGRT = 3,
	TWT_GET_TSF_REASON_MAX
};

#define TWT_MAX_FLOW_NUM        8
#define TWT_MAX_WAKE_INTVAL_EXP (TWT_REQ_TYPE_WAKE_INTERVAL_EXPONENT_MASK >> \
	TWT_REQ_TYPE_WAKE_INTERVAL_EXPONENT_SHIFT)

#define IS_TWT_PARAM_ACTION_ADD_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_ADD_BYPASS)
#define IS_TWT_PARAM_ACTION_DEL_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_DEL_BYPASS)
#define IS_TWT_PARAM_ACTION_MOD_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_MOD_BYPASS)
#define IS_TWT_PARAM_ACTION_ADD(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_ADD)
#define IS_TWT_PARAM_ACTION_DEL(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_DEL)
#define IS_TWT_PARAM_ACTION_SUSPEND(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_SUSPEND)
#define IS_TWT_PARAM_ACTION_RESUME(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_RESUME)
#endif /* APCLI_SUPPORT */

struct GNU_PACKED twt_info_sub_filed {
	UINT8 twt_info_1;
	UINT32 twt_info_2[2];
};

struct GNU_PACKED twt_info {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 token;
	struct twt_info_sub_filed twtinfo;
};

#ifdef APCLI_SUPPORT
struct twt_params_t {
	UINT8 fgReq;
	UINT8 fgTrigger;
	UINT8 fgProtect;
	UINT8 fgUnannounced;
	UINT8 ucSetupCmd;
	UINT8 ucMinWakeDur;
	UINT8 ucWakeIntvalExponent;
	UINT16 u2WakeIntvalMantiss;
	UINT64 u8TWT;
};

struct next_twt_info_t {
	UINT64 u8NextTWT;
	UINT8 ucNextTWTSize;
};

struct twt_ctrl_t {
	UINT8 ucBssIdx;
	UINT8 ucCtrlAction;
	UINT8 ucTWTFlowId;
	struct twt_params_t rTWTParams;
};

struct twt_get_tsf_context_t {
	enum TWT_GET_TSF_REASON ucReason;
	UINT8 ucBssIdx;
	UINT8 ucTWTFlowId;
	struct twt_params_t rTWTParams;
};

struct twt_flow_t {
	struct twt_params_t rTWTParams;
	struct twt_params_t rTWTPeerParams;
	UINT64 u8NextTWT;
};

struct twt_agrt_t {
	UINT8 fgValid;
	UINT8 ucAgrtTblIdx;
	UINT8 ucBssIdx;
	UINT8 ucFlowId;
	struct twt_params_t rTWTAgrt;
};

struct twt_planner_t {
	struct twt_agrt_t arTWTAgrtTbl[TWT_AGRT_MAX_NUM];
};

/* 11ax TWT Information frame format */
struct GNU_PACKED frame_twt_info {
	/* MAC header */
	UINT16 u2FrameCtrl;	/* Frame Control */
	UINT16 u2Duration;	/* Duration */
	UINT8 aucDestAddr[MAC_ADDR_LEN];	/* DA */
	UINT8 aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	UINT8 aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	UINT16 u2SeqCtrl;	/* Sequence Control */
	/* TWT Information frame body */
	UINT8 ucCategory;	/* Category */
	UINT8 ucAction;	/* Action Value */
	UINT8 ucNextTWTCtrl;
	UINT8 aucNextTWT[0];
};

struct GNU_PACKED msg_twt_fsm_t {
	UCHAR eMsgId;	/* Must be the first member */
	struct wifi_dev *wdev;
	UINT8 ucTWTFlowId;
	struct twt_ctrl_t rtwtCtrl;
};

/*
 * Important: Used for Communication between Host and WM-CPU,
 * should be packed and DW-aligned and in little-endian format
 */
struct GNU_PACKED cmd_twt_agrt_update_t {
	/* DW0 */
	UINT8 ucAgrtTblIdx;
	UINT8 ucAgrtCtrlFlag;
	UINT8 ucOwnMacId;
	UINT8 ucFlowId;		/* It is set to 0xff when peerGrpId is a group ID */
	/* DW1 */
	UINT16 u2PeerIdGrpId;	/* Specify the peer ID (MSB=0) or group ID (MSB=1)
				 * (10 bits for StaIdx, MSB to identify if it is for groupId)
				 */
	UINT8  ucAgrtSpDuration;	/* Same as SPEC definition. 8 bits, in unit of 256 us */
	UINT8  ucBssIndex;		/* So that we know which BSS TSF should be used for this AGRT */
	/* DW2, DW3, DW4 */
	UINT32 u4AgrtSpStartTsfLow;
	UINT32 u4AgrtSpStartTsfHigh;
	UINT16 u2AgrtSpWakeIntvlMantissa;
	UINT8  ucAgrtSpWakeIntvlExponent;
	UINT8  ucIsRoleAp;		/* 1: AP, 0: STA */
	/* DW5 */
	UINT8  ucAgrtParaBitmap;	/* For Bitmap definition, please refer to
					 * TWT_AGRT_PARA_BITMAP_IS_TRIGGER and etc
					 */
	UINT8  ucReserved_a;
	UINT16 u2Reserved_b;		/* Following field is valid ONLY when peerIdGrpId is a group ID */
	/* DW6 */
	UINT8  ucGrpMemberCnt;
	UINT8  ucReserved_c;
	UINT16 u2Reserved_d;
	/* DW7 ~ DW10 */
	UINT16 au2StaList[TWT_GRP_MAX_MEMBER_CNT];
};
#endif /* APCLI_SUPPORT */
#endif /* MT_MAC */

#endif /* _TWT_H_ */
