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

*/

#ifndef __HDEV_BASIC_H
#define __HDEV_BASIC_H

#ifdef RTMP_MAC_PCI
//struct _PCI_HIF_T;
#endif /*RTMP_MAC_PCI*/

#include "common/link_list.h"

struct _RTMP_CHIP_CAP;
struct _RTMP_CHIP_OP;
struct _EDCA_PARM;
struct MCU_CTRL;
struct _BCTRL_INFO_T;
struct _BCTRL_ENTRY;


/*
* state machine: 
* case1: NONE_OCCUPIED ->SW_OCCUPIED->NONE_OCCUPIED
* case2: NONE_OCCUPIED ->SW_OCCUPIED->HW_OCCUPIED->WAIT_RELEASE_FOR_HW->NONE_OCCUPIED
*/

enum {
	WTBL_STATE_NONE_OCCUPIED=0,
	WTBL_STATE_SW_OCCUPIED,
	WTBL_STATE_HW_OCCUPIED,
	WTBL_STATE_WAIT_RELEASE_FOR_HW,
};

typedef struct _WTBL_IDX_PARAMETER{
    UCHAR 	State;
    UCHAR   LinkToOmacIdx;
    UCHAR   LinkToWdevType;
    UCHAR   WtblIdx;	
	DL_LIST list;
} WTBL_IDX_PARAMETER, *PWTBL_IDX_PARAMETER;


typedef struct _WTBL_CFG{
	UCHAR MaxUcastEntryNum;
	UCHAR MinMcastWcid;
	BOOLEAN mcast_wait;
	WTBL_IDX_PARAMETER WtblIdxRec[MAX_LEN_OF_MAC_TABLE];
	NDIS_SPIN_LOCK WtblIdxRecLock;
	RTMP_OS_COMPLETION mcast_complete;
}WTBL_CFG;

#define WTC_WAIT_TIMEOUT CMD_MSG_TIMEOUT


typedef struct _OMAC_BSS_CTRL {
	UINT32 OmacBitMap;
	UINT32 HwMbssBitMap;
	UINT32 RepeaterBitMap;
} OMAC_BSS_CTRL, *POMAC_BSS_CTRL;


typedef struct _WMM_CTRL {
	EDCA_PARM *pWmmSet;
}MT_WMMCTRL_T, WMM_CTRL_T;


typedef struct _HD_RESOURCE_CFG {
	struct rtmp_phy_ctrl 	PhyCtrl[DBDC_BAND_NUM];
	struct _WMM_CTRL		WmmCtrl;
	struct _OMAC_BSS_CTRL	OmacBssCtl;
	//struct _REPEATER_CFG	RepeaterCfg;
	struct _WTBL_CFG		WtblCfg;
	UCHAR concurrent_bands;
}HD_RESOURCE_CFG;


typedef struct _HD_DEV {
	UCHAR Idx;
	RADIO_CTRL *pRadioCtrl;
	DL_LIST DevObjList;
	UCHAR DevNum;
	/*implicit point to pHdevCfg for sharing resource*/
	VOID     *priv;
}HD_DEV;


typedef struct _HD_DEV_OBJ {
	UCHAR Idx;
	UCHAR Type;
	UCHAR OmacIdx;
	UCHAR WmmIdx;
	BOOLEAN bWmmAcquired;
	HD_DEV *pHdev;
	DL_LIST RepeaterList;
	DL_LIST list;
    UCHAR RefCnt;
    NDIS_SPIN_LOCK RefCntLock;
}HD_DEV_OBJ;


typedef struct _HD_CFG{
	HD_DEV				Hdev[DBDC_BAND_NUM];
	//PSE_CFG				PseCfg;
	//HIF_CFG				HifCfg;
	RTMP_CHIP_CAP		chipCap;
	RTMP_CHIP_OP		ChipOps;
	HD_RESOURCE_CFG 	HwResourceCfg;
	HD_DEV_OBJ*			HObjList[WDEV_NUM_MAX];
	HD_DEV_OBJ			HObjBody[WDEV_NUM_MAX];
	struct MCU_CTRL 	McuCtrl;
	VOID 				*priv; /*implicit point to pAd*/
}HD_CFG;


typedef struct _HD_REPT_ENRTY{
	UCHAR CliIdx;
	UCHAR ReptOmacIdx;
	DL_LIST list;
}HD_REPT_ENRTY;


/*for hdev base functions*/
VOID HdevObjAdd(HD_DEV *pHdev,HD_DEV_OBJ *pObj);
VOID HdevObjDel(HD_DEV *pHdev,HD_DEV_OBJ *pObj);

INT32 HdevInit(HD_CFG *pHdevCfg,UCHAR HdevIdx,RADIO_CTRL*pRadioCtrl);
INT32 HdevExit(HD_CFG *pHdevCfg,UCHAR HdevIdx);
VOID HdevCfgShow(HD_CFG *pHdevCfg);

VOID HdevHwResourceExit(HD_CFG *pHdevCfg);
VOID HdevHwResourceInit(HD_CFG *pHdevCfg);

#endif /*__HDEV_BASIC_H*/
