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

#ifndef __HDEV_H
#define __HDEV_H

#include "hdev/hdev_basic.h"

/*Radio Control*/
VOID RcRadioInit(HD_CFG *pHdCfg,UCHAR RfIC, UCHAR DbdcMode);
HD_DEV *RcInit(HD_CFG *pHdCfg);
VOID RcRadioShow(HD_RESOURCE_CFG *pHwResourceCfg);

HD_DEV_OBJ* RcAcquiredBandForObj(HD_CFG *pHdCfg,UCHAR ObjIdx, UCHAR PhyMode, UCHAR Channel, UCHAR ObjType);
VOID RcReleaseBandForObj(HD_CFG *pHdCfg,HD_DEV_OBJ *pObj);

INT32 RcUpdateChannel(HD_DEV *pHdev,UCHAR Channel);
INT32 RcUpdatePhyMode(HD_DEV *pHdev,UCHAR Wmode);
HD_DEV* RcGetHdevByChannel(HD_CFG *pHdCfg,UCHAR Channel);
HD_DEV* RcGetHdevByPhyMode(HD_CFG *pHdCfg,UCHAR Channel);
UCHAR RcGetBandIdxByRf(HD_CFG *pHdCfg,UCHAR RfIC);


INT32 RcUpdateBandCtrl(HD_CFG *pHdCfg);
INT32 RcUpdateWmmEntry(HD_DEV *pHdev,HD_DEV_OBJ *pObj, UINT32 WmmIdx);
INT32 RcUpdateRepeaterEntry(HD_DEV *pHdev, UINT32 ReptIdx);
UCHAR RcUpdateBw(HD_DEV *pHdev,UCHAR Bw);
INT32 RcUpdateRadio(HD_DEV *pHdev,UCHAR bw,UCHAR central_ch1,UCHAR control_ch2);
INT32 RcUpdateExtCha(HD_DEV *pHdev,UCHAR ExtCha);
UCHAR RcGetExtCha(HD_DEV *pHdev);
UINT32 RcGetBmcQueueIdx(HD_DEV_OBJ *pObj);
UINT32 RcGetMgmtQueueIdx(HD_DEV_OBJ *pObj);
UINT32 RcGetBcnQueueIdx(HD_DEV_OBJ *pObj);
UINT32 RcGetTxRingIdx(HD_DEV_OBJ *pObj);
UINT32 RcGetWmmIdx(HD_DEV_OBJ *pObj);
UINT32 RcGetBandIdxByChannel(HD_CFG *pHdCfg,UCHAR Channel);
UCHAR RcGetPhyMode(HD_DEV *pHdev);
UCHAR RcGetChannel(HD_DEV *pHdev);
UCHAR RcGetCentralCh(HD_DEV *pHdev);
UCHAR RcGetBandIdx(HD_DEV *pHdev);
PHY_STATUS RcGetRadioCurStat(HD_DEV *pHdev);
VOID RcSetRadioCurStat(HD_DEV *pHdev, PHY_STATUS CurStat);
UCHAR RcGetBw(HD_DEV *pHdev);
HD_DEV *RcGetBandIdxByBf(HD_CFG *pHdCfg);
BOOLEAN RcIsBfCapSupport(HD_DEV_OBJ *obj);


/*WMM Control*/
VOID WcReleaseEdca(HD_DEV_OBJ *pObj);
VOID  WcAcquiredEdca(HD_DEV_OBJ *pObj,EDCA_PARM *pEdcaParm);
INT32 WcInit(HD_CFG *pHdCfg,WMM_CTRL_T *pWmmCtrl);
INT32 WcExit(WMM_CTRL_T *pWmmCtrl);
VOID WcShowEdca(HD_CFG *pHdCfg);
UINT32 WcGetWmmNum(HD_CFG *pHdCfg);
EDCA_PARM* WcGetWmmByIdx(HD_CFG *pHdCfg,UINT32 Idx);
VOID WcSetEdca(HD_DEV_OBJ *pObj);


/*Omac Control*/
INT32 GetOmacIdx(HD_CFG *pHdCfg, UINT32 OmacType, INT8 Idx);
VOID ReleaseOmacIdx(HD_CFG *pHdCfg, UINT32 OmacType, UINT32 Idx);
VOID OcDelRepeaterEntry(HD_DEV_OBJ *pObj,UCHAR ReptIdx);
INT32 OcAddRepeaterEntry(HD_DEV_OBJ *pObj,UCHAR ReptIdx);
HD_REPT_ENRTY *OcGetRepeaterEntry(HD_DEV_OBJ *pObj,UCHAR ReptIdx);


/*Wctl Control*/
VOID WtcInit(HD_CFG *pHdCfg);
VOID WtcExit(HD_CFG *pHdCfg);
UCHAR WtcSetMaxStaNum(HD_CFG *pHdCfg,UCHAR BssidNum,UCHAR MSTANum);
UCHAR WtcGetMaxStaNum(HD_CFG *pHdCfg);
UCHAR WtcAcquireGroupKeyWcid(HD_CFG *pHdCfg,HD_DEV_OBJ *pObj);
UCHAR WtcReleaseGroupKeyWcid(HD_CFG *pHdCfg,HD_DEV_OBJ *pObj, UCHAR idx);
UCHAR WtcGetWcidLinkType(HD_CFG *pHdCfg,UCHAR idx);
UCHAR WtcAcquireUcastWcid(HD_CFG *pHdCfg, HD_DEV_OBJ *pObj);
UCHAR WtcReleaseUcastWcid(HD_CFG *pHdCfg, HD_DEV_OBJ *pObj,UCHAR idx);
VOID WtcRecDump(HD_CFG *pHdCfg);
UCHAR WtcHwAcquireWcid(HD_CFG *pHdCfg, UCHAR idx);
UCHAR WtcHwReleaseWcid(HD_CFG *pHdCfg, UCHAR idx);

#define INVAILD_WCID 0xff

#endif /*__HDEV_H*/
