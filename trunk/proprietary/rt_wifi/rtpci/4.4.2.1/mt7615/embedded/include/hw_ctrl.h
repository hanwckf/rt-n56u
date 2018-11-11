/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	hw_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HW_CTRL_H__
#define __HW_CTRL_H__
#include "rtmp_type.h"
#include "security/wpa_cmm.h"

struct _RTMP_ADAPTER ;
struct _MAC_TABLE_ENTRY;
struct _EDCA_PARM;
struct _WMM_CFG;
struct wifi_dev;
struct _BSS_INFO_ARGUMENT_T;
struct _MAC_TABLE_ENTRY;
struct _WIFI_SYS_CTRL;
struct _STA_ADMIN_CONFIG;

typedef NTSTATUS (*HwCmdCb)(struct _RTMP_ADAPTER *pAd,VOID *Args);


#define HWCTRL_CMD_TIMEOUT 100
#define HWCTRL_CMD_WAITTIME 2000


enum {
	HWCTRL_STATUS_OK,
	HWCTRL_STATUS_TIMEOUT
};

/*for command classify*/
enum {
	HWCMD_TYPE_FIRST=0,
	HWCMD_TYPE_RADIO = HWCMD_TYPE_FIRST, /*Need Radio Resource Mgmt Related*/
	HWCMD_TYPE_SECURITY,					 /*Security related*/
	HWCMD_TYPE_PERIPHERAL,				/*Peripheral related*/
	HWCMD_TYPE_HT_CAP,						/*HT related*/
	HWCMD_TYPE_PS,							/*Power Saving related*/	
	HWCMD_TYPE_WIFISYS,
	HWCMD_TYPE_WMM,
	HWCMD_TYPE_PROTECT,
	HWCMD_TYPE_END
};


/*for command ID*/
 enum {
	HWCMD_ID_FIRST=0,
	/*Peripheral*/
	HWCMD_ID_GPIO_CHECK=HWCMD_ID_FIRST,
	/*USB related*/
	HWCMD_ID_RESET_BULK_OUT,
	HWCMD_ID_RESET_BULK_IN,
	/*WSC & LED related*/
	HWCMD_ID_SET_LED_STATUS,
	HWCMD_ID_LED_WPS_MODE10,
	/*Security related*/
	HWCMD_ID_SET_WCID_SEC_INFO,
	HWCMD_ID_SET_ASIC_WCID_IVEIV,
	HWCMD_ID_SET_ASIC_WCID_ATTR,
	HWCMD_ID_DEL_ASIC_WCID,
#ifdef HTC_DECRYPT_IOT
	HWCMD_ID_SET_ASIC_AAD_OM,
#endif /* HTC_DECRYPT_IOT */
	HWCMD_ID_ADDREMOVE_ASIC_KEY,
	/*MT_MAC */
	HWCMD_ID_SET_CLIENT_MAC_ENTRY,
	HWCMD_ID_PS_CLEAR,
	HWCMD_ID_PS_RETRIEVE_START,
	HWCMD_ID_SET_TR_ENTRY,
	HWCMD_ID_UPDATE_DAW_COUNTER,
	HWCMD_ID_UPDATE_BEACON,
	HWCMD_ID_GET_TEMPERATURE,
	HWCMD_ID_SET_SLOTTIME,
	HWCMD_ID_SET_TX_BURST,
#ifdef TXBF_SUPPORT
    HWCMD_ID_SET_APCLI_BF_CAP,
    HWCMD_ID_SET_APCLI_BF_REPEATER,
    HWCMD_ID_ADJUST_STA_BF_SOUNDING,
    HWCMD_ID_TXBF_TX_APPLY_CTRL,   
#endif /* TXBF_SUPPORT */
#ifdef ERR_RECOVERY
	HWCMD_ID_MAC_ERROR_DETECT,
#endif /* ERR_RECOVERY */
	/*AP realted*/
	HWCMD_ID_AP_ADJUST_EXP_ACK_TIME,
	HWCMD_ID_AP_RECOVER_EXP_ACK_TIME,
	HWCMD_ID_UPDATE_BSSINFO,
	HWCMD_ID_SET_BA_REC,
	/*STA related*/
	HWCMD_ID_PWR_MGT_BIT_WIFI,
	HWCMD_ID_FORCE_WAKE_UP,
	HWCMD_ID_FORCE_SLEEP_AUTO_WAKEUP,
	HWCMD_ID_MAKE_FW_OWN,
	HWCMD_ID_ENTER_PS_NULL,
#ifdef VOW_SUPPORT
    HWCMD_ID_SET_STA_DWRR,
    HWCMD_ID_SET_STA_DWRR_QUANTUM,
#endif /* VOW_SUPPORT */
	HWCMD_ID_UPDATE_RSSI,
#ifdef MLME_BY_CMDTHREAD
	HWCMD_ID_MLME_BY_CMDTHREAD,
#endif /* MLME_BY_CMDTHREAD */
	HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS,
    HWCMD_ID_SET_BCN_OFFLOAD,
    HWCMD_ID_ADD_REPT_ENTRY,
    HWCMD_ID_REMOVE_REPT_ENTRY,
    HWCMD_ID_WIFISYS_LINKDOWN,
    HWCMD_ID_WIFISYS_LINKUP,
    HWCMD_ID_WIFISYS_OPEN,
    HWCMD_ID_WIFISYS_CLOSE,
    HWCMD_ID_WIFISYS_PEER_LINKUP,
    HWCMD_ID_WIFISYS_PEER_LINKDOWN,
    HWCMD_ID_WIFISYS_PEER_UPDATE,
#ifdef THERMAL_PROTECT_SUPPORT
    HWCMD_ID_THERMAL_PROTECTION_RADIOOFF,
#endif /* THERMAL_PROTECT_SUPPORT */
	HWCMD_ID_GET_TX_STATISTIC,
	HWCMD_ID_PBC_CTRL,
	HWCMD_ID_RADIO_ON_OFF,
#ifdef LINK_TEST_SUPPORT
	HWCMD_ID_AUTO_LINK_TEST,
#endif /* LINK_TEST_SUPPORT */
#ifdef GREENAP_SUPPORT
	HWCMD_ID_GREENAP_ON_OFF,
#endif /* GREENAP_SUPPORT */	
	HWCMD_ID_HT_PROTECT,
	HWCMD_ID_RTS_THLD,
	HWCMD_ID_END,
};


/*for flag ID, is bit mask, 1/2/4/8*/
enum {
	HWFLAG_ID_FIRST=0,
	HWFLAG_ID_UPDATE_PROTECT=1<<0,
	HWFLAG_ID_END,
};


/*HwCtrl CMD structure*/
typedef struct _HwCmdQElmt {
	UINT32 type;
	UINT32 command;
	VOID *buffer;
	UINT32 bufferlength;
	BOOLEAN NeedWait;
	RTMP_OS_COMPLETION ack_done;
	VOID *RspBuffer;
	UINT32 RspBufferLen;
	HwCmdCb CallbackFun;
	VOID *CallbackArgs;
	NDIS_SPIN_LOCK lock;
	UINT32 status;
	struct _HwCmdQElmt *next;
} HwCmdQElmt, *PHwCmdQElmt;


typedef struct _HwCmdQ {
	UINT32 size;
	HwCmdQElmt *head;
	HwCmdQElmt *tail;
	UINT32 CmdQState;
} HwCmdQ, *PHwCmdQ;

typedef struct _HwFlagCtrl {
	BOOLEAN 	IsFlagSet;
	UINT32 		FlagId;
}HwFlagCtrl, *PHwFlagCtrl;


typedef struct _HW_CTRL_TXD {
	INT32 			CmdType;
	INT32			CmdId;
	BOOLEAN			NeedWait;
	UINT32 			wait_time;
	VOID			*pInformationBuffer;
	UINT32			InformationBufferLength;
	VOID			*pRespBuffer;
	UINT32			RespBufferLength;
	HwCmdCb 		CallbackFun;
	VOID			*CallbackArgs;
}HW_CTRL_TXD;


enum {
	SER_TIME_ID_T0=0,
	SER_TIME_ID_T1,
	SER_TIME_ID_T2,
	SER_TIME_ID_T3,	
	SER_TIME_ID_T4,
	SER_TIME_ID_T5,
	SER_TIME_ID_T6,
	SER_TIME_ID_T7,
	SER_TIME_ID_END,
};

typedef struct _HW_CTRL_T {
	HwCmdQ HwCtrlQ;
	HwFlagCtrl	HwCtrlFlag;
	NDIS_SPIN_LOCK HwCtrlQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK HwCtrlTask;
	UINT32 TotalCnt;
#ifdef ERR_RECOVERY
	RTMP_OS_TASK ser_task;
	INT ser_func_state;
	UINT32 ser_status;
	NDIS_SPIN_LOCK ser_lock;
	UINT32 ser_times[SER_TIME_ID_END];
#endif /* ERR_RECOVERY */
}HW_CTRL_T;



/*CMD structure */
typedef struct _RT_ASIC_RTS_INFO {
	UINT32 PktNumThrd;
	UINT32 PpduLengthThrd;
} RT_ASIC_RTS_INFO, *PRT_ASIC_RTS_INFO;


typedef struct _RT_ASIC_PROTECT_INFO {
	USHORT OperationMode;
	UCHAR SetMask;
	BOOLEAN bDisableBGProtect;
	BOOLEAN bNonGFExist;
} RT_ASIC_PROTECT_INFO, *PRT_ASIC_PROTECT_INFO;


typedef struct _RT_SET_ASIC_WCID {
	ULONG WCID;		/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG SetTid;		/* time-based: seconds, packet-based: kilo-packets */
	ULONG DeleteTid;	/* time-based: seconds, packet-based: kilo-packets */
	UCHAR Addr[MAC_ADDR_LEN];	/* avoid in interrupt when write key */
	UCHAR Tid;
	UINT16 SN;
	UCHAR Basize;
	INT   Ses_type;
	BOOLEAN IsAdd;
	BOOLEAN IsBMC;
	BOOLEAN IsReset;
} RT_SET_ASIC_WCID, *PRT_SET_ASIC_WCID;


#ifdef HTC_DECRYPT_IOT
typedef struct _RT_SET_ASIC_AAD_OM {
	ULONG WCID;
	UCHAR Value; // 0 ==> off, 1 ==> on
} RT_SET_ASIC_AAD_OM, *PRT_SET_ASIC_AAD_OM;
#endif /* HTC_DECRYPT_IOT */


typedef struct _RT_ASIC_WCID_SEC_INFO {
	UCHAR BssIdx;
	UCHAR KeyIdx;
	UCHAR CipherAlg;
	UINT8 Wcid;
	UINT8 KeyTabFlag;
} RT_ASIC_WCID_SEC_INFO, *PRT_ASIC_WCID_SEC_INFO;

typedef struct _RT_ASIC_SHARED_KEY {
	UCHAR BssIndex;
	UCHAR KeyIdx;
	CIPHER_KEY CipherKey;
} RT_ASIC_SHARED_KEY, *PRT_ASIC_SHARED_KEY;


typedef struct _RT_ASIC_PAIRWISE_KEY {
	UINT8 WCID;
	CIPHER_KEY CipherKey;
} RT_ASIC_PAIRWISE_KEY, *PRT_ASIC_PAIRWISE_KEY;


typedef struct _RT_ASIC_WCID_IVEIV_ENTRY {
	UINT8 Wcid;
	UINT32 Iv;
	UINT32 Eiv;
} RT_ASIC_WCID_IVEIV_ENTRY, *PRT_ASIC_WCID_IVEIV_ENTRY;

/*MT MAC Specific*/
typedef struct _MT_ASIC_SEC_INFO {
	UCHAR 			AddRemove;
	UCHAR			BssIdx;
	UCHAR			KeyIdx;
	UCHAR			Wcid;
	UCHAR			KeyTabFlag;
	CIPHER_KEY	 	CipherKey;
	UCHAR 			Addr[MAC_ADDR_LEN];
} MT_ASIC_SEC_INFO, *PMT_ASIC_SEC_INFO;

/*MT MAC Specific*/
typedef enum _SEC_ASIC_KEY_OPERATION {
    SEC_ASIC_ADD_PAIRWISE_KEY,
    SEC_ASIC_REMOVE_PAIRWISE_KEY,
    SEC_ASIC_ADD_GROUP_KEY,
    SEC_ASIC_REMOVE_GROUP_KEY,
} SEC_ASIC_KEY_OPERATION;

/*MT MAC Specific*/
typedef enum _SEC_ASIC_KEY_DIRECTION {
    SEC_ASIC_KEY_TX,
    SEC_ASIC_KEY_RX,
    SEC_ASIC_KEY_BOTH,
} SEC_ASIC_KEY_DIRECTION;

typedef struct _SEC_KEY_INFO {
    UCHAR Key[32]; /* TK(32) */
    UCHAR TxMic[8];
    UCHAR RxMic[8];
    UCHAR TxTsc[16]; /* TSC value. Change it from 48bit to 128bit */
    UCHAR RxTsc[16]; /* TSC value. Change it from 48bit to 128bit */
    UCHAR KeyLen; /* Key length for each key, 0: entry is invalid */
} SEC_KEY_INFO, *PSEC_KEY_INFO;

typedef struct _ASIC_SEC_INFO {
    SEC_ASIC_KEY_OPERATION Operation;
    SEC_ASIC_KEY_DIRECTION Direction;
    UINT32 Cipher;
    UCHAR Wcid;
    UCHAR BssIndex;
    UCHAR KeyIdx;
    SEC_KEY_INFO Key;
    UCHAR IGTK[32];
    UCHAR IGTKKeyLen;
    UCHAR PeerAddr[MAC_ADDR_LEN];
} ASIC_SEC_INFO, *PASIC_SEC_INFO;

#define IS_ADDKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_ADD_GROUP_KEY))
#define IS_REMOVEKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_REMOVE_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_GROUP_KEY))
#define IS_PAIRWISEKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_PAIRWISE_KEY))
#define IS_GROUPKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_GROUP_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_GROUP_KEY))


typedef struct _RT_ASIC_WCID_ATTR_ENTRY {
	UCHAR BssIdx;
	UCHAR KeyIdx;
	UCHAR CipherAlg;
	UINT8 Wcid;
	UINT8 KeyTabFlag;
} RT_ASIC_WCID_ATTR_ENTRY, *PRT_ASIC_WCID_ATTR_ENTRY;

typedef struct _MT_SET_BSSINFO {
	UCHAR OwnMacIdx;
	UINT8 ucBssIndex;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 BcMcWlanIdx;
	UINT32 NetworkType;
	UINT32 u4ConnectionType;
	UINT8 Active;
	UINT32 u4EnableFeature;
} MT_SET_BSSINFO, *PMT_SET_BSSINFO;

#ifdef BCN_OFFLOAD_SUPPORT
typedef struct _MT_SET_BCN_OFFLOAD
{
    UINT8 WdevIdx;
    ULONG WholeLength;
    BOOLEAN Enable;
    UCHAR OffloadPktType;
    ULONG TimIePos;
    ULONG CsaIePos;
} MT_SET_BCN_OFFLOAD, *PMT_SET_BCN_OFFLOAD;
#endif

typedef struct _MT_UPDATE_BEACON
{
    struct wifi_dev *wdev;
    UCHAR UpdateReason;
} MT_UPDATE_BEACON, *PMT_UPDATE_BEACON;

typedef struct _MT_SET_STA_REC {
	UINT8 BssIndex;
	UINT8 WlanIdx;
	UINT32 ConnectionType;
	UINT8 ConnectionState;
	UINT32 EnableFeature;
} MT_SET_STA_REC, *PMT_SET_STA_REC;

typedef struct _RT_SET_TR_ENTRY {
	ULONG WCID;
	VOID *pEntry;
} RT_SET_TR_ENTRY, *PRT_SET_TR_ENTRY;

#ifdef VOW_SUPPORT
typedef struct _MT_VOW_STA_GROUP {
        UINT8 StaIdx;
        UINT8 GroupIdx;
} MT_VOW_STA_GROUP, *PMT_VOW_STA_GROUP;

typedef struct _MT_VOW_STA_QUANTUM {
        BOOLEAN restore;
        UINT8 quantum;
} MT_VOW_STA_QUANTUM, *PMT_VOW_STA_QUANTUM;

#endif /* VOW_SUPPORT */

typedef struct _SLOT_CFG {
	BOOLEAN bUseShortSlotTime;
	UCHAR Channel;
    struct wifi_dev *wdev;
}SLOT_CFG;

typedef struct _REMOVE_REPT_ENTRY_STRUC {
    UCHAR func_tb_idx;
    UCHAR CliIdx;
} REMOVE_REPT_ENTRY_STRUC, *PREMOVE_REPT_ENTRY_STRUC;

typedef struct _ADD_REPT_ENTRY_STRUC {
    struct wifi_dev *wdev;
    UCHAR arAddr[MAC_ADDR_LEN];
} ADD_REPT_ENTRY_STRUC, *PADD_REPT_ENTRY_STRUC;

typedef struct _TX_STAT_STRUC {
    UINT32 Field;	/* Tx Statistic update method from N9 (GET_TX_STAT_XXX) */
    UINT8 Wcid;
    UINT8 Band;
} TX_STAT_STRUC, *PTX_STAT_STRUC;

#ifdef TXBF_SUPPORT
typedef struct _MT_STA_BF_ADJ {
    struct wifi_dev *wdev;
    UCHAR ConnectionState;
} MT_STA_BF_ADJ, *PMT_STA_BF_ADJ;
#endif /* TXBF_SUPPORT */


/*Export API function*/
UINT32 HwCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID HwCtrlExit(struct _RTMP_ADAPTER *pAd);

NDIS_STATUS HwCtrlEnqueueCmd(
	struct _RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd);

NDIS_STATUS HwCtrlSetFlag(
	struct _RTMP_ADAPTER *pAd,
	INT32 FlagId);

INT Show_HwCtrlStatistic_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);


/*Security*/
VOID HW_ADDREMOVE_KEYTABLE(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);
VOID HW_SET_WCID_SEC_INFO(struct _RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx, UCHAR CipherAlg, UINT8 Wcid, UINT8 KeyTabFlag);

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

#define RTMP_UPDATE_PROTECT(_pAd, _OperationMode, _SetMask, _bDisableBGProtect, _bNonGFExist)	\
	AsicUpdateProtect(_pAd, _OperationMode, _SetMask, _bDisableBGProtect, _bNonGFExist);

/* ----------------- Security Related MACRO ----------------- */

/* Set Asic WCID Attribute table */
#define HW_SET_WCID_SEC_INFO(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)	\
	RTMPSetWcidSecurityInfo(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)

/* Set Asic WCID IV/EIV table */
#define RTMP_ASIC_WCID_IVEIV_TABLE(_pAd, _Wcid, _uIV, _uEIV)	\
	AsicUpdateWCIDIVEIV(_pAd, _Wcid, _uIV, _uEIV)

/* Set Asic WCID Attribute table (offset:0x6800) */
#define RTMP_ASIC_WCID_ATTR_TABLE(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)\
	AsicUpdateWcidAttributeEntry(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)

#define RTMP_SET_TR_ENTRY(pAd, pEntry)   \
	TRTableInsertEntry(pAd,pEntry->wcid, pEntry);

#define RTMP_MLME_PRE_SANITY_CHECK(_pAd)

#define RTMP_AP_ADJUST_EXP_ACK_TIME(_pAd) \
	RTMP_IO_WRITE32(_pAd,  EXP_ACK_TIME, 0x005400ca )

#define RTMP_AP_RECOVER_EXP_ACK_TIME(_pAd) \
	RTMP_IO_WRITE32(_pAd,  EXP_ACK_TIME, 0x002400ca )

#define RTMP_SET_LED_STATUS(_pAd, _Status) \
	RTMPSetLEDStatus(_pAd, _Status)

#define RTMP_SET_LED(_pAd,_Mode) \
	RTMPSetLED(_pAd,_Mode)

VOID RTMP_PWR_MGT_BIT_WIFI(struct _RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit);
VOID RTMP_FORCE_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID RTMP_SLEEP_FORCE_AUTO_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);

#else
VOID RTMP_UPDATE_PROTECT(struct _RTMP_ADAPTER *pAd, USHORT OperationMode, UCHAR SetMask, BOOLEAN bDisableBGProtect, BOOLEAN bNonGFExist);
VOID RTMP_MLME_PRE_SANITY_CHECK(struct _RTMP_ADAPTER *pAd);

/*Security*/

VOID RTMP_SET_TR_ENTRY(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID RTMP_AP_ADJUST_EXP_ACK_TIME(struct _RTMP_ADAPTER *pAd);
VOID RTMP_AP_RECOVER_EXP_ACK_TIME(struct _RTMP_ADAPTER *pAd);
VOID RTMP_SET_LED_STATUS(struct _RTMP_ADAPTER *pAd, UCHAR Status);
VOID RTMP_SET_LED(struct _RTMP_ADAPTER *pAd, UINT32 WPSLedMode10);

/*STA*/
 

#endif /*defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)*/

/*Common*/
VOID RTMP_UPDATE_RAW_COUNTER(struct _RTMP_ADAPTER *pAd);
VOID RTMP_PS_RETRIVE_START(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);
VOID RTMP_PS_RETRIVE_CLEAR(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);
VOID RTMP_HANDLE_PRETBTT_INT_EVENT(struct _RTMP_ADAPTER *pAd);

VOID RTMP_SET_TX_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN enable);

VOID HW_SET_TX_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 ac_type,
                                              UINT8 prio, UINT16 level, UINT8 enable);

#ifdef MAC_REPEATER_SUPPORT	
VOID HW_ADD_REPT_ENTRY(
    struct _RTMP_ADAPTER *pAd,
    struct wifi_dev *wdev,
    PUCHAR pAddr);

VOID HW_REMOVE_REPT_ENTRY(
    struct _RTMP_ADAPTER *pAd,
    UCHAR func_tb_idx,
    UCHAR CliIdx);
#endif /* MAC_REPEATER_SUPPORT */

VOID HW_SET_BCN_OFFLOAD(
    struct _RTMP_ADAPTER *pAd,
    UINT8 WdevIdx,
    ULONG WholeLength,
    BOOLEAN Enable,
    UCHAR OffloadPktType,
    ULONG TimIePos,
    ULONG CsaIePos);

VOID HW_UPDATE_BSSINFO(struct _RTMP_ADAPTER *pAd,struct _BSS_INFO_ARGUMENT_T BssInfoArgs);

VOID RTMP_SET_BA_REC(struct _RTMP_ADAPTER *pAd, VOID *Buffer, UINT32 Len);
VOID HW_SET_BA_REC(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type);
VOID HW_SET_DEL_ASIC_WCID(struct _RTMP_ADAPTER *pAd, ULONG Wcid);

#ifdef HTC_DECRYPT_IOT
VOID HW_SET_ASIC_WCID_AAD_OM(struct _RTMP_ADAPTER *pAd, ULONG Wcid, UCHAR value);
#endif /* HTC_DECRYPT_IOT */

VOID RTMP_GET_TEMPERATURE(struct _RTMP_ADAPTER *pAd,UINT32 *pTemperature);
VOID RTMP_RADIO_ON_OFF_CTRL(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio);
#ifdef GREENAP_SUPPORT
VOID RTMP_GREENAP_ON_OFF_CTRL(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAP);
#endif /* GREENAP_SUPPORT */
VOID HW_SET_SLOTTIME(struct _RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR Channel, struct wifi_dev *wdev);
VOID HW_ENTER_PS_NULL(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID HW_BECON_UPDATE(struct _RTMP_ADAPTER *pAd,MT_UPDATE_BEACON rMtUpdateBeacon);

/* add this entry into ASIC RX WCID search table */
VOID RTMP_STA_ENTRY_ADD(struct _RTMP_ADAPTER *pAd, ULONG Wcid,UCHAR *pAddr,BOOLEAN IsBMC, BOOLEAN IsReset);

	
#ifdef PKT_BUDGET_CTRL_SUPPORT
VOID HW_SET_PBC_CTRL(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UCHAR type);
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

VOID HW_SET_RTS_THLD(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length);

	/* Insert the BA bitmap to ASIC for the Wcid entry */
#define RTMP_ADD_BA_SESSION_TO_ASIC(_pAd, _wcid, _TID, _SN, _basize, _type)	\
	HW_SET_BA_REC(_pAd, _wcid, _TID, _SN, _basize, 1, _type);


	/* Remove the BA bitmap from ASIC for the Wcid entry */
	/*		bitmap field starts at 0x10000 in ASIC WCID table */
#define RTMP_DEL_BA_SESSION_FROM_ASIC(_pAd, _wcid, _TID, _type) \
	HW_SET_BA_REC(_pAd, _wcid, _TID, 0, 0, 0, _type);

#ifdef TXBF_SUPPORT
VOID HW_APCLI_BF_CAP_CONFIG(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID HW_APCLI_BF_REPEATER_CONFIG(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID HW_STA_BF_SOUNDING_ADJUST(struct _RTMP_ADAPTER *pAd, UCHAR connState,struct wifi_dev *wdev);
VOID HW_AP_TXBF_TX_APPLY(struct _RTMP_ADAPTER *pAd,UCHAR enable);
#endif /* TXBF_SUPPORT */


#ifdef VOW_SUPPORT
VOID RTMP_SET_STA_DWRR(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID RTMP_SET_STA_DWRR_QUANTUM(struct _RTMP_ADAPTER *pAd, BOOLEAN restore, UCHAR quantum);
#endif /* VOW_SUPPORT */

VOID RTMP_SET_UPDATE_RSSI(struct _RTMP_ADAPTER *pAd);

#ifdef THERMAL_PROTECT_SUPPORT
VOID RTMP_SET_THERMAL_RADIO_OFF(struct _RTMP_ADAPTER *pAd);
#endif /* THERMAL_PROTECT_SUPPORT */



VOID NICUpdateRawCountersNew(
	struct _RTMP_ADAPTER *pAd);

#ifdef ERR_RECOVERY
typedef enum _ERR_RECOVERY_STATE {
    ERR_RECOV_STOP_IDLE = 0,
    ERR_RECOV_STOP_PDMA0,
    ERR_RECOV_RESET_PDMA0,
    ERR_RECOV_WAIT_N9_NORMAL,
    ERR_RECOV_EVENT_REENTRY,
    ERR_RECOV_STATE_NUM
} ERR_RECOVERY_STATE, *P_ERR_RECOVERY_STATE;

typedef struct _ERR_RECOVERY_CTRL_T
{
    ERR_RECOVERY_STATE errRecovState;
    UINT32 status;
} ERR_RECOVERY_CTRL_T;

VOID RTMP_MAC_RECOVERY(struct _RTMP_ADAPTER *pAd, UINT32 Status);
INT IsStopingPdma(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl);
ERR_RECOVERY_STATE ErrRecoveryCurStat(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl);
BOOLEAN IsErrRecoveryInIdleStat(struct _RTMP_ADAPTER *pAd);
VOID ser_sys_reset(RTMP_STRING *arg);
NTSTATUS HwRecoveryFromError(struct _RTMP_ADAPTER *pAd);
void SerTimeLogDump(struct _RTMP_ADAPTER *pAd);
#endif /* ERR_RECOVERY */



VOID HW_WIFISYS_OPEN(struct _RTMP_ADAPTER *pAd,struct _WIFI_SYS_CTRL wifi_sys_ctrl);
VOID HW_WIFISYS_CLOSE(struct _RTMP_ADAPTER *pAd,struct _WIFI_SYS_CTRL wifi_sys_ctrl);
VOID HW_WIFISYS_LINKUP(struct _RTMP_ADAPTER *pAd,struct _WIFI_SYS_CTRL wifi_sys_ctrl);
VOID HW_WIFISYS_LINKDOWN(struct _RTMP_ADAPTER *pAd, struct _WIFI_SYS_CTRL wifi_sys_ctrl);
VOID HW_WIFISYS_PEER_LINKUP(struct _RTMP_ADAPTER *pAd,struct _WIFI_SYS_CTRL wifi_sys_ctrl);
VOID HW_WIFISYS_PEER_LINKDOWN(struct _RTMP_ADAPTER *pAd,struct _WIFI_SYS_CTRL wifi_sys_ctrl);
VOID HW_WIFISYS_PEER_UPDATE(struct _RTMP_ADAPTER *pAd,struct _WIFI_SYS_CTRL wifi_sys_ctrl);

VOID HW_GET_TX_STATISTIC(struct _RTMP_ADAPTER *pAd, UINT32 Field, UINT8 Wcid);

#ifdef LINK_TEST_SUPPORT
VOID RTMP_AUTO_LINK_TEST(struct _RTMP_ADAPTER *pAd);
#endif /* LINK_TEST_SUPPORT */

#endif
