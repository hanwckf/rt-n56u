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
	wifi_sys_info.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __WIFI_SYS_INFO_H__
#define __WIFI_SYS_INFO_H__

#include "common/link_list.h"

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _IE_lists;
struct _STA_TR_ENTRY;

#ifdef APCLI_SUPPORT
struct _APCLI_STRUCT;
#endif

typedef struct _DEV_INFO_CTRL_T{	
	UINT8 OwnMacIdx;
	UINT8 OwnMacAddr[MAC_ADDR_LEN];
	UINT8 BandIdx;
	UINT8 Active;
	UINT32 EnableFeature;
	VOID *priv;
	DL_LIST list;
}DEV_INFO_CTRL_T;


typedef struct _STA_REC_CTRL_T {
	UINT8 BssIndex;
	UINT8 WlanIdx;
	UINT32 ConnectionType;
	UINT8 ConnectionState;
	UINT32 EnableFeature;
	UINT8 IsNewSTARec;
	VOID *priv;
	DL_LIST list;
}STA_REC_CTRL_T;


#define WDEV_BSS_STATE(__wdev) \
	(__wdev->bss_info_argument.bss_state)
	
typedef enum _BSSINFO_LINK_TO_OMAC_T
{
    HW_BSSID = 0,
    EXTEND_MBSS,
    WDS,
    REPT
} BSSINFO_TYPE_T;

typedef enum _BSSINFO_STATE_T
{
	BSS_INIT	= 0,	// INIT state
	BSS_INITED 	= 1,	// BSS Argument Link done
	BSS_ACTIVE 	= 2,	// The original flag - Active
	BSS_READY 	= 3		// BssInfo updated to FW done and ready for beaconing
} BSSINFO_STATE_T;

typedef struct _BSS_INFO_ARGUMENT_T
{
    BSSINFO_TYPE_T bssinfo_type;
	BSSINFO_STATE_T bss_state;
	UCHAR OwnMacIdx;
	UINT8 ucBssIndex;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 ucBcMcWlanIdx;
	UINT8 ucPeerWlanIdx;
	UINT32 NetworkType;
	UINT32 u4ConnectionType;
	UINT8 CipherSuit;
	UINT8 WmmIdx;
    UINT32 prio_bitmap;
    UINT16 txop_level[MAX_PRIO_NUM];
	UINT32 u4BssInfoFeature;
	CMD_BSSINFO_PM_T rBssInfoPm;
	HTTRANSMIT_SETTING BcTransmit;
	HTTRANSMIT_SETTING McTransmit;
	VOID *priv;
	DL_LIST list;
} BSS_INFO_ARGUMENT_T, *PBSS_INFO_ARGUMENT_T;


typedef struct _WIFI_SYS_CTRL{
	struct wifi_dev *wdev;
	DEV_INFO_CTRL_T DevInfoCtrl;
	STA_REC_CTRL_T StaRecCtrl;
	BSS_INFO_ARGUMENT_T BssInfoCtrl;
	VOID *priv;
	BOOLEAN skip_set_txop;
}WIFI_SYS_CTRL;


typedef struct _WIFI_INFO_CLASS{
	UINT32 Num;
	DL_LIST Head;
}WIFI_INFO_CLASS_T;


/*for FW related information sync.*/
typedef struct _WIFI_SYS_INFO{
	WIFI_INFO_CLASS_T DevInfo;
	WIFI_INFO_CLASS_T BssInfo;
	WIFI_INFO_CLASS_T StaRec;
	NDIS_SPIN_LOCK lock;
}WIFI_SYS_INFO_T;


typedef struct _PEER_LINKUP_HWCTRL{
#ifdef TXBF_SUPPORT
	struct _IE_lists ie_list;
	BOOLEAN bMu;
	BOOLEAN bETxBf;
	BOOLEAN bITxBf;
	BOOLEAN bMuTxBf;
#endif /*TXBF_SUPPORT*/
	BOOLEAN bRdgCap;
}PEER_LINKUP_HWCTRL;

typedef struct _LINKUP_HWCTRL{
#ifdef TXBF_SUPPORT
	BOOLEAN bMu;
	BOOLEAN bETxBf;
	BOOLEAN bITxBf;
	BOOLEAN bMuTxBf;
#endif /*TXBF_SUPPORT*/
	BOOLEAN bRdgCap;
}LINKUP_HWCTRL;

/*Export function*/
VOID WifiSysInfoReset(struct _WIFI_SYS_INFO *pWifiSysInfo);
VOID WifiSysInfoInit(struct _RTMP_ADAPTER *pAd);
VOID WifiSysInfoDump(struct _RTMP_ADAPTER *pAd);


VOID WifiSysOpen(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev);
VOID WifiSysClose(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev);


#ifdef CONFIG_AP_SUPPORT
VOID WifiSysApLinkDown(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev);
VOID WifiSysApLinkUp(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev);
VOID WifiSysApPeerLinkUp(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list);
#ifdef WH_EZ_SETUP
VOID WifiSysApPeerChBwUpdate(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry);//, struct _IE_lists *ie_list);
#endif
#ifdef WDS_SUPPORT
VOID WifiSysWdsLinkUp(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev, UCHAR wcid);
VOID WifiSysWdsLinkDown(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev, UCHAR wcid);
#endif /*WDS_SUPPORT*/

#endif /*CONFIG_AP_SUPPORT*/


#ifdef APCLI_SUPPORT
VOID WifiSysApCliLinkUp(struct _RTMP_ADAPTER *pAd,struct _APCLI_STRUCT *pApCliEntry,UCHAR CliIdx, struct _MAC_TABLE_ENTRY *pMacEntry);
VOID WifiSysApCliLinkDown(struct _RTMP_ADAPTER *pAd,struct _APCLI_STRUCT *pApCliEntry,UCHAR CliIdx);
#ifdef WH_EZ_SETUP
VOID WifiSysApCliChBwUpdate(struct _RTMP_ADAPTER *pAd,struct _APCLI_STRUCT *pApCliEntry,UCHAR CliIdx, struct _MAC_TABLE_ENTRY *pMacEntry);
#endif
#endif /*APCLI*/


#ifdef RT_CFG80211_SUPPORT
#ifdef CFG_TDLS_SUPPORT
VOID WifiSysTdlsPeerLinkDown(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry);
#endif
#endif



#ifdef RACTRL_FW_OFFLOAD_SUPPORT
VOID WifiSysRaInit(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry);
VOID WifiSysUpdateRa(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry, struct _STAREC_AUTO_RATE_UPDATE_T *prParam);
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

VOID WifiSysUpdatePortSecur(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry);
BSSINFO_STATE_T WifiSysGetBssInfoState(struct _RTMP_ADAPTER *pAd, UINT8 ucBssInfoIdx);
VOID WifiSysUpdateBssInfoState(struct _RTMP_ADAPTER *pAd, UINT8	ucBssInfoIdx, BSSINFO_STATE_T bss_state);
VOID WifiSysPeerLinkDown(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry);


#endif
