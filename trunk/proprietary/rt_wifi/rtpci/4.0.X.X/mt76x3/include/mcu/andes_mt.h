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
	andes_mt.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __ANDES_MT_H__
#define __ANDES_MT_H__

#include "mcu.h"

#ifdef LINUX
#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif
#endif /* LINUX */

struct _RTMP_ADAPTER;
struct cmd_msg;

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
	EXT_CMD_RF_REG_ACCESS = 0x02,
    EXT_CMD_RF_TEST = 0x04,
	EXT_CMD_RADIO_ON_OFF_CTRL = 0x05,
	EXT_CMD_WIFI_RX_DISABLE = 0x06,
	EXT_CMD_CHANNEL_SWITCH = 0x08,
	EXT_CMD_NIC_CAPABILITY = 0x09,
	EXT_CMD_PWR_SAVING = 0x0A,
	EXT_CMD_MULTIPLE_REG_ACCESS = 0x0E,
	EXT_CMD_AP_PWR_SAVING_CAPABILITY = 0xF,
	EXT_CMD_SEC_ADDREMOVE_KEY = 0x10,
	EXT_CMD_SET_TX_POWER_CTRL=0x11,
	EXT_CMD_FW_LOG_2_HOST = 0x13,
	EXT_CMD_PS_RETRIEVE_START = 0x14,
	EXT_CMD_LED_CTRL=0x17,
	EXT_CMD_EFUSE_BUFFER_MODE=0x21,
};

/*
 * Extension Event
 */
enum EXT_EVENT_TYPE {
	EXT_EVENT_CMD_RESULT = 0x0,
	EXT_EVENT_RF_REG_ACCESS = 0x2,
	EXT_EVENT_MULTI_CR_ACCESS = 0x0E,
	EXT_EVENT_FW_LOG_2_HOST = 0x13,
};

/*
 * DownLoad Type
 */
enum {
	DownLoadTypeA,
	DownLoadTypeB,
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


typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
    UINT32         u4FuncIndex;
    UINT32         u4FuncData;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;

typedef struct _CMD_TEST_CTRL_T {
    UINT8          ucAction;
    UINT8          aucReserved[3];
    union {
        UINT32                     u4OpMode;
        UINT32                     u4ChannelFreq;
        PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
    } u;
} CMD_TEST_CTRL_T, *P_CMD_TEST_CTRL_T;


#define WIFI_RX_DISABLE 1
typedef struct _EXT_CMD_WIFI_RX_DISABLE_T {
	UINT8 ucWiFiRxDisableCtrl;
	UINT8 aucReserve[3];
} EXT_CMD_WIFI_RX_DISABLE_T;

#define SKU_SIZE 21
typedef struct _EXT_CMD_CHAN_SWITCH_T {
    UINT8          ucCtrlCh;
    UINT8          ucCentralCh;
    UINT8         ucBW;
    UINT8          ucTxStreamNum;
    UINT8          ucRxStreamNum;
    UINT8          aucReserve0[7];
    UINT8          aucTxPowerSKU[SKU_SIZE];
    UINT8          aucReserve1[3];
} EXT_CMD_CHAN_SWITCH_T, *P_EXT_CMD_CHAN_SWITCH_T;


typedef struct _EXT_CMD_TX_POWER_CTRL_T {
	UINT8 ucCenterChannel;
	UINT8 ucTSSIEnable;
	UINT8 ucTempCompEnable;
	CHAR aucTargetPower[2];
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
	UINT16                  u2Addr;
	UINT8                    ucValue;
	UINT8          ucReserved;
} BIN_CONTENT_T, *P_BIN_CONTENT_T;

typedef struct _EXT_CMD_EFUSE_BUFFER_MODE_T {
UINT8 ucSourceMode;
UINT8 ucCount;
UINT8 aucReserved[2];
BIN_CONTENT_T aBinContent[70];
} EXT_CMD_EFUSE_BUFFER_MODE_T, *P_EXT_CMD_EFUSE_BUFFER_MODE_T;


enum {
	 EEPROM_MODE_EFUSE=0,
	 EEPROM_MODE_BUFFER=1,
};

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


#define MT_UPLOAD_FW_UNIT (1024 * 4)

VOID AndesMTFillCmdHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt);
VOID AndesMTRxEventHandler(struct _RTMP_ADAPTER *pAd, UCHAR *data);
INT32 AndesMTLoadFw(struct _RTMP_ADAPTER *pAd);
INT32 AndesMTEraseFw(struct _RTMP_ADAPTER *pAd);

#ifdef RTMP_PCI_SUPPORT
INT32 AndesMTPciKickOutCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
VOID AndesMTPciFwInit(struct _RTMP_ADAPTER *pAd);
VOID AndesMTPciFwExit(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_PCI_SUPPORT */


#ifdef RTMP_SDIO_SUPPORT
INT32 AndesMTSdioKickOutCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
VOID AndesMTSdioFwInit(struct _RTMP_ADAPTER *pAd);
VOID AndesMTSdioFwExit(struct _RTMP_ADAPTER *pAd);
INT32 AndesMTSendCmdMsgToSdio(struct _RTMP_ADAPTER *pAd);
INT32 AndesMTSdioChkCrc(struct _RTMP_ADAPTER *pAd, UINT32 checksum_len);
UINT16 AndesMTSdioGetCrc(struct _RTMP_ADAPTER *pAd);
#endif

INT32 CmdInitAccessRegWrite(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 data);
INT32 CmdInitAccessRegRead(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data);
INT32 CmdChPrivilege(struct _RTMP_ADAPTER *pAd, UINT8 Action, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream);
INT32 CmdAccessRegWrite(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 data);
INT32 CmdAccessRegRead(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data);
INT32 CmdRFRegAccessWrite(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value);
INT32 CmdRFRegAccessRead(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value);
INT32 CmdRadioOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 On);
INT32 CmdWiFiRxDisable(struct _RTMP_ADAPTER *pAd, UINT RxDisable);
INT32 CmdChannelSwitch(struct _RTMP_ADAPTER *pAd, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream);
INT32 CmdNicCapability(struct _RTMP_ADAPTER *pAd);
INT32 CmdPsRetrieveReq(struct _RTMP_ADAPTER *pAd, UINT32 enable);

#ifdef MT_PS
INT32 CmdPsRetrieveStartReq(struct _RTMP_ADAPTER *pAd, UINT32 WlanIdx);
INT32 CmdPsClearReq(struct _RTMP_ADAPTER *pAd, UINT32 wlanidx, BOOLEAN p_wait);
#endif /* MT_PS */
INT32 CmdSecKeyReq(struct _RTMP_ADAPTER *pAd, UINT8 AddRemove, UINT8 Keytype, UINT8 *pAddr, UINT8 Alg, UINT8 KeyID, UINT8 KeyLen, UINT8 WlanIdx, UINT8 *KeyMaterial);
INT32 CmdRfTest(struct _RTMP_ADAPTER *pAd, UINT8 Action, UINT8 Mode, UINT8 CalItem);
NDIS_STATUS AndesMTLoadRomPatch(struct _RTMP_ADAPTER *ad);
INT32 CmdMultipleMacRegAccessWrite(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, UINT32 Num);
INT32 CmdMultiPleMacRegAccessRead(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, UINT32 Num);
INT32 CmdMultipleRfRegAccessWrite(struct _RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair, UINT32 Num); 
INT32 CmdMultiPleRfRegAccessRead(struct _RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair, UINT32 Num);
INT32 CmdFwLog2Host(struct _RTMP_ADAPTER *pAd, UINT8 FWLog2HostCtrl);
VOID CmdIOWrite32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value);
VOID CmdIORead32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value);
VOID CmdEfusBufferModeSet(struct _RTMP_ADAPTER *pAd);
VOID CmdSetTxPowerCtrl(struct _RTMP_ADAPTER *pAd, UINT8 central_chl);
INT AndesLedOP(struct _RTMP_ADAPTER *pAd,UCHAR LedIdx,UCHAR LinkStatus);
#endif /* __ANDES_MT_H__ */

