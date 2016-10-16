/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_cmd.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __MT_CMD_H__
#define __MT_CMD_H__



#ifdef RT_BIG_ENDIAN
typedef	union _FW_TXD_0 {
	struct {
		UINT32 pq_id:16;
		UINT32 length:16;
	} field;
	UINT32 word;
} FW_TXD_0;
#else
typedef union _FW_TXD_0 {
	struct {
		UINT32 length:16;
		UINT32 pq_id:16;
	} field;
	UINT32 word;
} FW_TXD_0;
#endif

#define PKT_ID_CMD 0xA0
#define PKT_ID_EVENT 0xE000

#ifdef RT_BIG_ENDIAN
typedef union _FW_TXD_1 {
	struct {
		UINT32 seq_num:8;
		UINT32 set_query:8;
		UINT32 pkt_type_id:8;
		UINT32 cid:8;
	} field;
#ifdef MT7615
	struct {
		UINT32 seq_num:6;
		UINT32 pkt_ft:2;
		UINT32 set_query:8;
		UINT32 pkt_type_id:8;
		UINT32 cid:8;
	} field1;
#endif
	UINT32 word;
} FW_TXD_1;
#else
typedef union _FW_TXD_1 {
	struct {
		UINT32 cid:8;
		UINT32 pkt_type_id:8;
		UINT32 set_query:8;
		UINT32 seq_num:8;
	} field;
#ifdef MT7615
	struct {
		UINT32 cid:8;
		UINT32 pkt_type_id:8;
		UINT32 set_query:8;
		UINT32 pkt_ft:2;
		UINT32 seq_num:6;
	} field1;
#endif
	UINT32 word;
} FW_TXD_1;
#endif


#define EXT_CID_OPTION_NEED_ACK 1
#define EXT_CID_OPTION_NO_NEED_ACK 0


#ifdef RT_BIG_ENDIAN
typedef union _FW_TXD_2 {
	struct {
		UINT32 ext_cid_option:8;
		UINT32 ucD2B2Rev:8;
		UINT32 ext_cid:8;
		UINT32 ucD2B0Rev:8;
	} field;
	UINT32 word;
} FW_TXD_2;
#else
typedef union _FW_TXD_2 {
	struct {
		UINT32 ucD2B0Rev:8;
		UINT32 ext_cid:8;
		UINT32 ucD2B2Rev:8;
		UINT32 ext_cid_option:8;
	} field;
	UINT32 word;
} FW_TXD_2;
#endif /* RT_BIG_ENDIAN */


/*
 * FW TX descriptor
 */
typedef struct GNU_PACKED _FW_TXD_ {
	FW_TXD_0 fw_txd_0;
	FW_TXD_1 fw_txd_1;
	FW_TXD_2 fw_txd_2;
	UINT32 au4D3toD7rev[5];
} FW_TXD;

/*
 * Command type table
 */
enum MT_CMD_TYPE {
	MT_TARGET_ADDRESS_LEN_REQ = 0x01,
	MT_FW_START_REQ = 0x02,
	INIT_CMD_ACCESS_REG = 0x3,
	MT_PATCH_START_REQ = 0x05,
	MT_PATCH_FINISH_REQ = 0x07,
	MT_PATCH_SEM_CONTROL = 0x10,
	MT_HIF_LOOPBACK = 0x20,
	CMD_CH_PRIVILEGE = 0x20,
	CMD_ACCESS_REG = 0xC2,
	EXT_CID = 0xED,
	MT_FW_SCATTER = 0xEE,
	MT_RESTART_DL_REQ = 0xEF,
};

/*
 * Extension Command
 */
enum EXT_CMD_TYPE {
	EXT_CMD_ID_EFUSE_ACCESS  =0x01,
	EXT_CMD_RF_REG_ACCESS = 0x02,
	EXT_CMD_RF_TEST = 0x04,
	EXT_CMD_RADIO_ON_OFF_CTRL = 0x05,
	EXT_CMD_WIFI_RX_DISABLE = 0x06,
    EXT_CMD_PM_STATE_CTRL = 0x07,
	EXT_CMD_CHANNEL_SWITCH = 0x08,
	EXT_CMD_NIC_CAPABILITY = 0x09,
	EXT_CMD_PWR_SAVING = 0x0A,
	EXT_CMD_MULTIPLE_REG_ACCESS = 0x0E,
	EXT_CMD_AP_PWR_SAVING_CAPABILITY = 0xF,
	EXT_CMD_SEC_ADDREMOVE_KEY = 0x10,
	EXT_CMD_SET_TX_POWER_CTRL=0x11,
	EXT_CMD_FW_LOG_2_HOST = 0x13,
	EXT_CMD_PS_RETRIEVE_START = 0x14,
#ifdef CONFIG_MULTI_CHANNEL
       EXT_CMD_ID_MCC_OFFLOAD_START = 0x15,
       EXT_CMD_ID_MCC_OFFLOAD_STOP  = 0x16,
#endif /* CONFIG_MULTI_CHANNEL */
	EXT_CMD_LED_CTRL=0x17,
	EXT_CMD_BT_COEX = 0x19,
	EXT_CMD_ID_PWR_MGT_BIT_WIFI = 0x1B,
	EXT_CMD_EFUSE_BUFFER_MODE = 0x21,
	EXT_CMD_THERMAL_PROTECT = 0x23,
	EXT_CMD_STAREC_UPDATE = 0x25,
	EXT_CMD_ID_EDCA_SET = 0x27,
	EXT_CMD_ID_SLOT_TIME_SET = 0x28,
	EXT_CMD_GET_THEMAL_SENSOR=0x2C,
	EXT_CMD_TMR_CAL = 0x2D,
    EXT_CMD_OBTW = 0x2F,
    EXT_CMD_LOAD_DPD_FROM_FLASH = 0x50,
};

enum
{
	CH_SWITCH_BY_NORMAL_TX_RX = 0,
	CH_SWITCH_INTERNAL_USED_BY_FW_0 = 1,
	CH_SWITCH_INTERNAL_USED_BY_FW_1 = 1,
	CH_SWITCH_SCAN = 3,
	CH_SWITCH_INTERNAL_USED_BY_FW_3 = 4
};

#ifdef CONFIG_MULTI_CHANNEL
typedef struct _EXT_CMD_MCC_START_T
{
    // Common setting from DW0~3
    // DW0
    UINT16     u2IdleInterval;
    UINT8      ucRepeatCnt;
    UINT8      ucStartIdx;

    // DW1
    UINT32     u4StartInstant;

    // DW2,3
    UINT16     u2FreePSEPageTh;
    UINT8      ucPreSwitchInterval;
    UINT8      aucReserved0[0x5];

    // BSS0 setting from DW4~7
    // DW4
    UINT8      ucWlanIdx0;
    UINT8      ucPrimaryChannel0;
    UINT8      ucCenterChannel0Seg0;
    UINT8      ucCenterChannel0Seg1;

    // DW5
    UINT8      ucBandwidth0;
    UINT8      ucTrxStream0;
    UINT16     u2StayInterval0;

    // DW6
    UINT8     ucRole0;
    UINT8     ucOmIdx0;
    UINT8     ucBssIdx0;
    UINT8     ucWmmIdx0;

    // DW7
    UINT8     aucReserved1[0x4];

    // BSS1 setting from DW8~11
    // DW8
    UINT8     ucWlanIdx1;
    UINT8     ucPrimaryChannel1;
    UINT8     ucCenterChannel1Seg0;
    UINT8     ucCenterChannel1Seg1;

    // DW9
    UINT8     ucBandwidth1;
    UINT8     ucTrxStream1;
    UINT16    u2StayInterval1;

    // DW10
    UINT8     ucRole1;
    UINT8     ucOmIdx1;
    UINT8     ucBssIdx1;
    UINT8     ucWmmIdx1;

    // DW11
    UINT8     aucReserved2[0x4];
} EXT_CMD_MCC_START_T, *P_EXT_CMD_MCC_START_T;

typedef struct _EXT_CMD_MCC_STOP_T
{
    //DW0
    UINT8      ucParkIdx;
    UINT8      ucAutoResumeMode;
    UINT16     u2AutoResumeInterval;

    //DW1
    UINT32     u4AutoResumeInstant;

    //DW2
    UINT16     u2IdleInterval;
    UINT8      aucReserved[2];

    //DW3
    UINT16     u2StayInterval0;
    UINT16     u2StayInterval1;
} EXT_CMD_MCC_STOP_T, *P_EXT_CMD_MCC_STOP_T;


#endif /* CONFIG_MULTI_CHANNEL */





#ifdef MT7636_BTCOEX
/*
 * Coex Sub
 */
enum EXT_BTCOEX_SUB {
	COEX_SET_PROTECTION_FRAME = 0x1,
	COEX_WIFI_STATUS_UPDATE  = 0x2,
	COEX_UPDATE_BSS_INFO = 0x03,
};


/*
 * Coex status bit
 */
enum EXT_BTCOEX_STATUS_bit {
	COEX_STATUS_RADIO_ON 		= 	0x01,
	COEX_STATUS_SCAN_G_BAND  	= 	0x02,
	COEX_STATUS_SCAN_A_BAND 	= 	0x04,
	COEX_STATUS_LINK_UP 			= 	0x08,
	COEX_STATUS_BT_OVER_WIFI 	= 	0x10,
};

enum EXT_BTCOEX_PROTECTION_MODE {
	COEX_Legacy_CCK 		= 	0x00,
	COEX_Legacy_OFDM  	= 	0x01,
	COEX_HT_MIX 	= 	0x02,
	COEX_HT_Green 			= 	0x03,
	COEX_VHT	= 	0x04,
};


enum EXT_BTCOEX_CCK_PROTECTION_RATE {
	LONG_PREAMBLE_1M 		= 	0x00,
	LONG_PREAMBLE_2M  	= 	0x01,
	LONG_PREAMBLE_5_5M 	= 	0x02,
	LONG_PREAMBLE_11M 	= 	0x03,
	SHORT_PREAMBLE_1M	= 	0x04,
	SHORT_PREAMBLE_2M 	= 	0x05,
	SHORT_PREAMBLE_5_5M   = 	0x06,
	SHORT_PREAMBLE_11M 	= 	0x07,
};

enum EXT_BTCOEX_OFDM_PROTECTION_RATE {
	PROTECTION_OFDM_6M 	= 	0x00,
	PROTECTION_OFDM_9M  	= 	0x01,
	PROTECTION_OFDM_12M 	= 	0x02,
	PROTECTION_OFDM_18M 	= 	0x03,
	PROTECTION_OFDM_24M	= 	0x04,
	PROTECTION_OFDM_36M 	= 	0x05,
	PROTECTION_OFDM_48M   = 	0x06,
	PROTECTION_OFDM_54M 	= 	0x07,
};
/*
 * Coex status bit
 */

typedef enum _WIFI_STATUS {
	STATUS_RADIO_ON = 0,
	STATUS_RADIO_OFF = 1,
	STATUS_SCAN_G_BAND = 2,
	STATUS_SCAN_G_BAND_END = 3,
	STATUS_SCAN_A_BAND = 4,
	STATUS_SCAN_A_BAND_END = 5,
	STATUS_LINK_UP = 6,
	STATUS_LINK_DOWN = 7,
	STATUS_BT_OVER_WIFI = 8,
	STATUS_BT_MAX,
} WIFI_STATUS;
#endif
/*
 * Extension Event
 */
enum EXT_EVENT_TYPE {
	EXT_EVENT_CMD_RESULT = 0x0,
	EXT_EVENT_RF_REG_ACCESS = 0x2,
#ifdef RTMP_MAC_SDIO
	EXT_EVENT_SLEEPY_NOTIFY = 0x06,
#endif
	EXT_EVENT_MULTI_CR_ACCESS = 0x0E,
	EXT_EVENT_FW_LOG_2_HOST = 0x13,
        EXT_EVENT_BT_COEX = 0x19,
        EXT_EVENT_BEACON_LOSS = 0x1A,
	EXT_EVENT_THERMAL_PROTECT = 0x22,
    EXT_EVENT_TMR_CAL_RESULT = 0x2E,
};

/*
 * DownLoad Type
 */
enum {
	DownLoadTypeA,
	DownLoadTypeB,
#ifdef MT7615
    DownLoadTypeC
#endif /* MT7615 */
};

#ifdef RT_BIG_ENDIAN
typedef union _FW_RXD_0 {
	struct {
		UINT32 pkt_type_id:16;
		UINT32 length:16;
	} field;
	UINT32 word;
} FW_RXD_0;
#else
typedef union _FW_RXD_0 {
	struct {
		UINT32 length:16;
		UINT32 pkt_type_id:16;
	} field;
	UINT32 word;
} FW_RXD_0;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef union _FW_RXD_1 {
	struct {
		UINT32 rsv:16;
		UINT32 seq_num:8;
		UINT32 eid:8;
	} field;
	UINT32 word;
} FW_RXD_1;
#else
typedef union _FW_RXD_1 {
	struct {
		UINT32 eid:8;
		UINT32 seq_num:8;
		UINT32 rsv:16;
	} field;
	UINT32 word;
} FW_RXD_1;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef union _FW_RXD_2 {
	struct {
		UINT32 rsv:24;
		UINT32 ext_eid:8;
	} field;
	UINT32 word;
} FW_RXD_2;
#else
typedef union _FW_RXD_2 {
	struct {
		UINT32 ext_eid:8;
		UINT32 rsv:24;
	} field;
	UINT32 word;
} FW_RXD_2;
#endif /* RT_BIG_ENDIAN */

/*
 * Event structure
 */
typedef struct GNU_PACKED _EVENT_RXD_ {
	FW_RXD_0 fw_rxd_0;
	FW_RXD_1 fw_rxd_1;
	FW_RXD_2 fw_rxd_2;
} EVENT_RXD;

/*
 * Event type table
 */
enum MT_EVENT_TYPE {
	MT_TARGET_ADDRESS_LEN_RSP = 0x01,
	MT_FW_START_RSP = 0x01,
	EVENT_ACCESS_REG = 0x02,
	MT_PATCH_SEM_RSP = 0x04,
	EVENT_CH_PRIVILEGE = 0x18,
	EXT_EVENT = 0xED,
	MT_RESTART_DL_RSP = 0xEF,
};

#define FW_CODE_START_ADDRESS1 0x100000
#define FW_CODE_START_ADDRESS2 0x000000
#define ROM_PATCH_START_ADDRESS 0x8C000

#define REL_PATCH_SEM	0
#define GET_PATCH_SEM	1

#define ROM_PATCH_START_ADDRESS 0x8C000

#define REL_PATCH_SEM	0
#define GET_PATCH_SEM	1

/*
 * CMD w/ target address/length data mode
 */
#define TARGET_ADDR_LEN_NO_RSP 0
#define ENABLE_ENCRY (1 << 0)
#define RESET_SEC_IV (1 << 3)
#define TARGET_ADDR_LEN_NEED_RSP (1 << 31)
#define ENABLE_ENCRY (1 << 0)
#define RESET_SEC_IV (1 << 3)

/*
 * FW feature set
 */
#define FW_FEATURE_SET_ENCRY (1 << 0)
#define FW_FEATURE_SET_KEY_MASK (0x3 << 1)
#define FW_FEATURE_SET_KEY(p) (((p) & 0x03) << 1)
#define FW_FEATURE_RESET_IV (1 << 3)
#define GET_FEATURE_SET_KEY(p) (((p) & FW_FEATURE_SET_KEY_MASK) >> 1)

/*
 * Erro code for target address/length response
 */
enum {
	TARGET_ADDRESS_LEN_SUCCESS,
};

/*
 * Error code for cmd(event) firmware start response
 */
enum {
	WIFI_FW_DOWNLOAD_SUCCESS,
	WIFI_FW_DOWNLOAD_INVALID_PARAM,
	WIFI_FW_DOWNLOAD_INVALID_CRC,
	WIFI_FW_DOWNLOAD_DECRYPTION_FAIL,
	WIFI_FW_DOWNLOAD_UNKNOWN_CMD,
	WIFI_FW_DOWNLOAD_TIMEOUT,
};

struct _INIT_CMD_ACCESS_REG {
	UINT8 ucSetQuery;
	UINT8 aucReserved[3];
	UINT32 u4Address;
	UINT32 u4Data;
};

#define CMD_CH_PRIV_ACTION_REQ 0
#define CMD_CH_PRIV_ACTION_ABORT 1
#define CMD_CH_PRIV_ACTION_BW_REQ 2

#define CMD_CH_PRIV_SCO_SCN 0
#define CMD_CH_PRIV_SCO_SCA 1
#define CMD_CH_PRIV_SCO_SCB 3

#define CMD_CH_PRIV_BAND_G 1
#define CMD_CH_PRIV_BAND_A 2

#define CMD_CH_PRIV_CH_WIDTH_20_40 0
#define CMD_CH_PRIV_CH_WIDTH_80	   1
#define CMD_CH_PRIV_CH_WIDTH_160   2
#define CMD_CH_PRIV_CH_WIDTH_80_80 3

#define CMD_CH_PRIV_REQ_JOIN 0
#define CMD_CH_PRIV_REQ_P2P_LISTEN 1

#ifdef MT7636_BTCOEX
typedef struct _CMD_COEXISTENCE_T {
    UINT8         ucSubOpCode;
    UINT8         aucReserve[3];
    UINT8          aucData[48];
} EXT_CMD_COEXISTENCE_T, *P_EXT_CMD_COEXISTENCE_T;

typedef struct _EVENT_EXT_COEXISTENCE_T {
    UINT8         ucSubOpCode;
    UINT8         aucReserve[3];
    UINT8         aucBuffer[64];
} EVENT_EXT_COEXISTENCE_T, *P_EVENT_EXT_COEXISTENCE_T;


typedef struct _COEX_WIFI_STATUS_UPDATE_T
{
    UINT32      u4WIFIStatus;
} COEX_WIFI_STATUS_UPDATE_T, *P_COEX_WIFI_STATUS_UPDATE_T;


typedef struct _COEX_SET_PROTECTION_FRAME_T
{
    UINT8      ucProFrameMode;
    UINT8      ucProFrameRate;
    UINT8      aucReserve[2];
} COEX_SET_PROTECTION_FRAME_T, *P_COEX_SET_PROTECTION_FRAME_T;

typedef struct _COEX_UPDATE_BSS_INFO_T
{
    UINT8      u4BSSPresence[4];
    UINT8      u4BSSAPMode[4];
    UINT8      u4IsQBSS[4];
} COEX_UPDATE_BSS_INFO_T, *P_COEX_UPDATE_BSS_INFO_T;

typedef struct _EVENT_COEX_CMD_RESPONSE_T {
    UINT32         u4Status;
} EVENT_COEX_CMD_RESPONSE_T, *P_EVENT_COEX_CMD_RESPONSE_T;


typedef struct _EVENT_COEX_REPORT_COEX_MODE_T {
	UINT32         u4SupportCoexMode;
    UINT32         u4CurrentCoexMode;
} EVENT_COEX_REPORT_COEX_MODE_T, *P_EVENT_COEX_REPORT_COEX_MODE_T;


typedef struct _EVENT_COEX_MASK_OFF_TX_RATE_T {
    UINT8         ucOn;
    UINT8         aucReserve[3];
} EVENT_COEX_MASK_OFF_TX_RATE_T, *P_EVENT_COEX_MASK_OFF_TX_RATE_T;



typedef struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T {
    UINT8         ucOn;
    UINT8         ucRXBASize;
    UINT8         aucReserve[2];
}EVENT_COEX_CHANGE_RX_BA_SIZE_T, *P_EVENT_COEX_CHANGE_RX_BA_SIZE_T;

typedef struct _EVENT_COEX_LIMIT_BEACON_SIZE_T {
    UINT8         ucOn;
    UINT8         aucReserve[3];
}EVENT_COEX_LIMIT_BEACON_SIZE_T, *P_EVENT_COEX_LIMIT_BEACON_SIZE_T;


typedef struct _EVENT_COEX_EXTEND_BTO_ROAMING_T {
    UINT8         ucOn;
    UINT8         aucReserve[3];
} EVENT_COEX_EXTEND_BTO_ROAMING_T, *P_EVENT_COEX_EXTEND_BTO_ROAMING_T;

#endif /*MT7636_BTCOEX*/


typedef struct _CMD_SEC_ADDREMOVE_KEY_STRUC_T {
	UINT8		ucAddRemove;
	UINT8		ucTxKey;
	UINT8		ucKeyType;
	UINT8		ucIsAuthenticator;
	UINT8		aucPeerAddr[6];
	UINT8		ucBssIndex;
	UINT8		ucAlgorithmId;
	UINT8		ucKeyId;
	UINT8		ucKeyLen;
	UINT8		ucWlanIndex;
	UINT8		ucReverved;
	UINT8		aucKeyRsc[16];
	UINT8		aucKeyMaterial[32];
} CMD_SEC_ADDREMOVE_KEY_STRUC_T, *P_CMD_ADDREMOVE_KEY_STRUC_T;

typedef struct _EVENT_SEC_ADDREMOVE_STRUC_T {
	UINT32		u4WlanIdx;
	UINT32		u4Status;
	UINT32		u4Resv;
} EVENT_SEC_ADDREMOVE_STRUC_T, *P_EVENT_SEC_ADDREMOVE_STRUC_T;

typedef struct _EXT_CMD_AP_PWS_START_T {
    UINT32 u4WlanIdx;
    UINT32 u4Resv;
    UINT32 u4Resv2;
} EXT_CMD_AP_PWS_START_T, *P_EXT_CMD_AP_PWS_START_T;


typedef struct _CMD_AP_PS_CLEAR_STRUC_T {
	UINT32		u4WlanIdx;
	UINT32		u4Status;
	UINT32		u4Resv;
} CMD_AP_PS_CLEAR_STRUC_T, *P_CMD_AP_PS_CLEAR_STRUC_T;

typedef struct _CMD_CH_PRIVILEGE_T {
    UINT8      ucBssIndex;
    UINT8      ucTokenID;
    UINT8      ucAction;
    UINT8      ucPrimaryChannel;
    UINT8      ucRfSco;
    UINT8      ucRfBand;
    UINT8      ucRfChannelWidth;   /* To support 80/160MHz bandwidth */
    UINT8      ucRfCenterFreqSeg1; /* To support 80/160MHz bandwidth */
    UINT8      ucRfCenterFreqSeg2; /* To support 80/160MHz bandwidth */
    UINT8      ucReqType;
    UINT8      aucReserved[2];
    UINT32     u4MaxInterval;      /* In unit of ms */
} CMD_CH_PRIVILEGE_T, *P_CMD_CH_PRIVILEGE_T;


typedef struct _CMD_RF_REG_ACCESS_T {
	UINT32 WiFiStream;
	UINT32 Address;
	UINT32 Data;
} CMD_RF_REG_ACCESS_T;


typedef struct _CMD_ACCESS_REG_T {
	UINT32 u4Address;
	UINT32 u4Data;
} CMD_ACCESS_REG_T;


#define WIFI_RADIO_ON 1
#define WIFI_RADIO_OFF 2
typedef struct _EXT_CMD_RADIO_ON_OFF_CTRL_T {
	UINT8 ucWiFiRadioCtrl;
	UINT8 aucReserve[3];
} EXT_CMD_RADIO_ON_OFF_CTRL_T;

/* EXT_CMD_RF_TEST */
/* ACTION */
#define ACTION_SWITCH_TO_RFTEST 0 /* to switch firmware mode between normal mode or rf test mode */
#define ACTION_IN_RFTEST        1
/* OPMODE */
#define OPERATION_NORMAL_MODE     0
#define OPERATION_RFTEST_MODE     1
#define OPERATION_ICAP_OVERLAP 	  3

typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
    UINT32         u4FuncIndex;
    UINT32         u4FuncData;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;

typedef struct _CMD_TEST_CTRL_T {
    UINT8 ucAction;
	UINT8 ucIcapLen;
    UINT8 aucReserved[2];
    union {
        UINT32 u4OpMode;
        UINT32 u4ChannelFreq;
        PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
    } u;
} CMD_TEST_CTRL_T, *P_CMD_TEST_CTRL_T;


#define WIFI_RX_DISABLE 1
typedef struct _EXT_CMD_WIFI_RX_DISABLE_T {
	UINT8 ucWiFiRxDisableCtrl;
	UINT8 aucReserve[3];
} EXT_CMD_WIFI_RX_DISABLE_T;


//Power Management Level
#define PM2         2
#define PM4         4
#define PM5         5
#define PM6         6
#define PM7         7
#define ENTER_PM_STATE 1
#define EXIT_PM_STATE 2
typedef struct _EXT_CMD_PM_STATE_CTRL_T {
	UINT8 ucPmNumber;
	UINT8 ucPmState;
    UINT8 aucBssid[6];
    UINT8 ucDtimPeriod;
    UINT8 ucWlanIdx;
    UINT16 u2BcnInterval;
    UINT32 u4Aid;
    UINT32 u4RxFilter;
    UINT8 aucReserve0[4];
    UINT32 u4Feature;
    UINT8 ucOwnMacIdx;
    UINT8 ucWmmIdx;
    UINT8 ucBcnLossCount;
    UINT8 ucBcnSpDuration;
} EXT_CMD_PM_STATE_CTRL_T, *P_EXT_CMD_PM_STATE_CTRL_T;

enum {
	STA_REC_BASIC_STA_RECORD = 0,
	STA_REC_AUTO_RATE_PARAMETER = 1,
	STA_REC_BE_PARAMETER = 2,
	STA_REC_MAX_NUM = 3
};

typedef struct _CMD_STAREC_UPDATE_T
{
    UINT8	ucBssIndex;
    UINT8	ucWlanIdx;
    UINT16	u2TotalElementNum;
    UINT32	u4Reserve;
    UINT8	aucBuffer[];
} CMD_STAREC_UPDATE_T, *P_CMD_STAREC_UPDATE_T;

typedef union _HTTRANSMIT_SETTING_T {
    struct {
        UINT16 MCS:6;
        UINT16 LDPC:1;
        UINT16 BW:2;
        UINT16 ShortGI:1;
        UINT16 STBC:1;
        UINT16 eTxBF:1;
        UINT16 iTxBF:1;
        UINT16 MODE:3;
	} field;
	UINT16 u2Reserve;
} HTTRANSMIT_SETTING_T, *PHTTRANSMIT_SETTING_T;


typedef struct _STAREC_AUTO_RATE__T
{
    /* Auto Rate (Group1) */
    UINT16	u2Tag;		// Tag = 0x01
    UINT16	u2Length;
    BOOLEAN		fgRaValid;
    UINT8	ucRaTable;
    BOOLEAN		fgRaMcs32Supported;
    BOOLEAN		fgRaHtCapInfoGF;
    HTTRANSMIT_SETTING_T	rRaHtTransmitSetting;	/* 4 bytes struct */
	UINT16       u2RaReserve;
    UINT8	aucRaHtCapMCSSet[3];
    UINT8	ucRaReserve;
} CMD_STAREC_AUTO_RATE_T, *P_CMD_STAREC_AUTO_RATE_T;

// TODO: Need to add other item for new STA Rec Tag
#define	MAX_BUF_SIZE_OF_STA_REC	(sizeof(CMD_STAREC_UPDATE_T) + sizeof(CMD_STAREC_AUTO_RATE_T))


#define SKU_SIZE 21
typedef struct _EXT_CMD_CHAN_SWITCH_T {
    UINT8          ucCtrlCh;
    UINT8          ucCentralCh;
    UINT8         ucBW;
    UINT8          ucTxStreamNum;
    UINT8          ucRxStreamNum;
    UINT8          ucSwitchReason;
    UINT8          aucReserve0[6];
    UINT8          aucTxPowerSKU[SKU_SIZE];
    UINT8          aucReserve1[3];
} EXT_CMD_CHAN_SWITCH_T, *P_EXT_CMD_CHAN_SWITCH_T;


typedef struct _EXT_CMD_TX_POWER_CTRL_T {
	UINT8 ucCenterChannel;
	UINT8 ucTSSIEnable;
	UINT8 ucTempCompEnable;
	UINT8 aucTargetPower[2];
	UINT8 aucRatePowerDelta[14];
	UINT8 ucBWPowerDelta;
	UINT8 aucCHPowerDelta[6];
	UINT8 aucTempCompPower[17];
	UINT8 ucReserved;
} EXT_CMD_TX_POWER_CTRL_T, *P_EXT_CMD_TX_POWER_CTRL_T;

typedef struct _INIT_EVENT_ACCESS_REG {
	UINT32 u4Address;
	UINT32 u4Data;
} INIT_EVENT_ACCESS_REG, *P_INIT_EVENT_ACCESS_REG;

#define EVENT_EXT_CMD_RESULT_SUCCESS 0
typedef struct _EVENT_EXT_CMD_RESULT_T {
	UINT8 ucExTenCID;
	UINT8 aucReserve[3];
	UINT32 u4Status;
} EVENT_EXT_CMD_RESULT_T;

typedef struct _EXT_EVENT_NIC_CAPABILITY_T {
	UINT8 aucDateCode[16];
	UINT8 aucVersionCode[12];
} EXT_EVENT_NIC_CAPABILITY;

enum {
	MAC_CR,
	RF_CR,
};

typedef struct _CMD_MULTI_CR_ACCESS_T {
	UINT32 u4Type;
	UINT32 u4Addr;
	UINT32 u4Data;
} CMD_MULTI_CR_ACCESS_T;

typedef struct _EXT_EVENT_MULTI_CR_ACCESS_WR_T {
	UINT32 u4Status;
	UINT32 u4Resv;
	UINT32 u4Resv2;
} EXT_EVENT_MULTI_CR_ACCESS_WR_T;

typedef struct _EXT_EVENT_MULTI_CR_ACCESS_RD_T {
	UINT32 u4Type;
	UINT32 u4Addr;
	UINT32 u4Data;
} EXT_EVENT_MULTI_CR_ACCESS_RD_T;

enum {
	ANDES_LOG_DISABLE,
	ANDES_LOG_TO_UART,
	ANDES_LOG_TO_EVENT,
};

typedef struct _EXT_CMD_FW_LOG_2_HOST_CTRL_T {
	UINT8 ucFwLog2HostCtrl;
	UINT8 ucReserve[3];
} EXT_CMD_FW_LOG_2_HOST_CTRL_T;


typedef struct _CMD_AP_PS_RETRIEVE_T {
    UINT32 u4Option; /* 0: AP_PWS enable, 1: redirect disable */
    UINT32 u4Param1; /* for 0: enable/disable. for 1: wlan idx */
    UINT32 u4Resv;
} CMD_AP_PS_RETRIEVE_STRUC_T, *P_CMD_AP_PS_RETRIEVE_STRUC_T;

typedef struct _EXT_EVENT_AP_PS_RETRIEVE_T {
    UINT32 u4Param1; /* for 0: enable/disable. for 1: wlan idx */
    UINT32 u4Resv;
    UINT32 u4Resv2;
} EXT_EVENT_AP_PS_RETRIEVE_T, *P_EXT_EVENT_AP_PS_RETRIEVE_T;


typedef struct _BIN_CONTENT_T {
	UINT16 u2Addr;
	UINT8 ucValue;
	UINT8 ucReserved;
} BIN_CONTENT_T, *P_BIN_CONTENT_T;

typedef struct _EXT_CMD_GET_SENSOR_RESULT_T {
    UINT8 ucActionIdx;
    UINT8 aucReserved[3];
} EXT_CMD_GET_SENSOR_RESULT_T, *P_EXT_CMD_GET_SENSOR_RESULT_T;

typedef struct _EXT_EVENT_GET_SENSOR_RESULT_T
{
    UINT32 u4SensorResult;
    UINT32 u4Reserved;
} EXT_EVENT_GET_SENSOR_RESULT_T, *P_EXT_EVENT_GET_SENSOR_RESULT_T;

#define EFUSE_CONTENT_BUFFER_SIZE 0xff
typedef struct _EXT_CMD_EFUSE_BUFFER_MODE_T {
	UINT8 ucSourceMode;
	UINT8 ucCount;
	UINT8 aucReserved[2];
	BIN_CONTENT_T aBinContent[EFUSE_CONTENT_BUFFER_SIZE];
} EXT_CMD_EFUSE_BUFFER_MODE_T, *P_EXT_CMD_EFUSE_BUFFER_MODE_T;



#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _LED_NMAC_CMD{
	UINT32  rsv:8;
	UINT32 CmdID:8;
	UINT32 Arg0:8;
	UINT32 Arg1:8;
}LED_NMAC_CMD;
#else
typedef struct GNU_PACKED _LED_NMAC_CMD{
	UINT32 Arg1:8;
	UINT32 Arg0:8;
	UINT32 CmdID:8;
	UINT32 rsv:8;	
}LED_NMAC_CMD;
#endif /* RT_BIG_ENDIAN */


typedef enum _EXT_ENUM_PM_FEATURE_T
{
    PM_CMD_FEATURE_PSPOLL_OFFLOAD       = 0x00000001,
    PM_CMD_FEATURE_PS_TX_REDIRECT        = 0x00000002,
    PM_CMD_FEATURE_SMART_BCN_SP          = 0x00000004
} EXT_ENUM_PM_FEATURE_T;

#ifdef RTMP_MAC_SDIO
typedef struct _EXT_EVENT_SLEEPY_NOTIFY_T
{
	UINT8		ucSleepState;
	UINT8		aucReserves[3];
} EXT_EVENT_SLEEPY_NOTIFY_T, *P_EXT_EVENT_SLEEPY_NOTIFY_T;
#endif

typedef struct _EXT_EVENT_BEACON_LOSS_T
{
	UINT8		aucBssid[6];
	UINT8		aucReserves[2];
} EXT_EVENT_BEACON_LOSS_T, *P_EXT_EVENT_BEACON_LOSS_T;

typedef struct _EXT_EVENT_TMR_CAL_RESULT_T
{
    UINT32      u4TmrCPCnt;//cp cnt * 25 already.
    UINT32      u4TmrResult;
    UINT32      u4TmrTxFE_5;
    UINT8       ucTMRPhase;
    UINT8       aucReserved[3];
} EXT_EVENT_TMR_CAL_RESULT_T, *P_EXT_EVENT_TMR_CAL_RESULT_T;

typedef struct _EXT_CMD_PWR_MGT_BIT_T
{
	UINT8		ucWlanIdx;
	UINT8		ucPwrMgtBit;
	UINT8		aucReserve[2];
} EXT_CMD_PWR_MGT_BIT_T, *P_EXT_CMD_PWR_MGT_BIT_T;

typedef struct _EXT_CMD_OBTW_T
{
    UINT8       ucOption;
    UINT8       aucReserve0[3];
    UINT8       ucOBTWDelta[21];
    UINT8       aucReserve1[3];
} EXT_CMD_OBTW_T, *P_EXT_CMD_OBTW_T;

enum {
	 EEPROM_MODE_EFUSE = 0,
	 EEPROM_MODE_BUFFER = 1,
};

typedef struct _CMD_ACCESS_EFUSE_T
{
    UINT32         u4Address;
    UINT32         u4Valid;
    UINT8          aucData[16];
} CMD_ACCESS_EFUSE_T, *P_CMD_ACCESS_EFUSE_T, _EXT_EVENT_ACCESS_EFUSE_T;


typedef struct _EFUSE_ACCESS_DATA_T{
	PUINT pIsValid;
	PUCHAR pValue;
}EFUSE_ACCESS_DATA_T;


enum {
	HIGH_TEMP_THD = 0,
	LOW_TEMP_THD = 1,
};

typedef struct _EXT_CMD_THERMAL_PROTECT_T {
	UINT8 ucHighEnable;
	CHAR cHighTempThreshold;
	UINT8 ucLowEnable;
	CHAR cLowTempThreshold;
} EXT_CMD_THERMAL_PROTECT_T, *P_EXT_CMD_THERMAL_PROTECT_T;

typedef struct _EXT_EVENT_THERMAL_PROTECT_T {
	UINT8 ucHLType;
	CHAR cCurrentTemp;
	UINT8 aucReserve[2];
} EXT_EVENT_THERMAL_PROTECT_T, *P_EXT_EVENT_THERMAL_PROTECT_T;

typedef struct _EXT_CMD_TMR_CAL_T {
	UINT8 ucEnable;
	UINT8 ucBand;//0: 2G, 1: 5G
	UINT8 ucBW;//0: 20MHz, 1: 40MHz, 2: 80MHz, 3: 160MHz, 4: 10MHz, 5: 5MHz
	UINT8 ucAnt;//0: Atn0, 1: Ant1

	UINT8 ucRole;//0: initiator, 1: responder
	UINT8 aucReserve[3];
} EXT_CMD_TMR_CAL_T, *P_EXT_CMD_TMR_CAL_T;

#define MT_UPLOAD_FW_UNIT (1024 * 4)


#define CMD_EDCA_AIFS_BIT 		1 << 0
#define CMD_EDCA_WIN_MIN_BIT 	1<<1
#define CMD_EDCA_WIN_MAX_BIT 	1 <<2
#define CMD_EDCA_TXOP_BIT 		1<<3
#define CMD_EDCA_ALL_BITS 		CMD_EDCA_AIFS_BIT | CMD_EDCA_WIN_MIN_BIT | CMD_EDCA_WIN_MAX_BIT |CMD_EDCA_TXOP_BIT

#define CMD_EDCA_AC_MAX 4

typedef struct _TX_AC_PARAM_T
{
	UINT8    ucAcNum;
	UINT8    ucVaildBit;
	UINT8	ucAifs;
	UINT8  	ucWinMin;
	UINT16  u2WinMax;
	UINT16  u2Txop;

}TX_AC_PARAM_T,*P_TX_AC_PARAM_T;


typedef struct _CMD_EDCA_SET_T {

	UINT8		 	 ucTotalNum;
	UINT8			 aucReserve[3];
	TX_AC_PARAM_T rAcParam[CMD_EDCA_AC_MAX];

} CMD_EDCA_SET_T, *P_CMD_EDCA_SET_T;


typedef struct _CMD_SLOT_TIME_SET_T
{
    UINT8   ucSlotTime;
    UINT8   ucSifs;
    UINT8   ucRifs;
    UINT8   ucReserve1;
    UINT16  u2Eifs;
    UINT16  u2Reserve2;
}CMD_SLOT_TIME_SET_T,*P_CMD_SLOT_TIME_SET_T;

typedef struct  _EXT_CMD_ID_LOAD_DPD_T
{
    UINT8        ucReload; // 0: No reload, 1: do reload
    UINT8        ucChannel;
    UINT8        aucReserve[2];
    UINT32       au4WF0CR[17];
    UINT32       au4WF1CR[17];
} EXT_CMD_ID_LOAD_DPD_T, *P_EXT_CMD_ID_LOAD_DPD_T;


INT32 CmdExtPwrMgtBitWifi(struct _RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit);
INT32 CmdExtPmStateCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPmNumber, UINT8 ucPmState);

#endif /* __MT_CMD_H__ */

