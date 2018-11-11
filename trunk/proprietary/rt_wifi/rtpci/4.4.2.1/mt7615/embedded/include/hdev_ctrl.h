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
	hdev_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HDEV_CTRL_H__
#define __HDEV_CTRL_H__

struct _RTMP_ADAPTER ;
struct _EDCA_PARM;
struct _QLOAD_CTRL;
struct _AUTO_CH_CTRL;
struct wifi_dev;
struct _OMAC_BSS_CTRL;
struct _REPEATER_CLIENT_ENTRY;


INT32 HcRadioInit(struct _RTMP_ADAPTER *pAd, UCHAR RfIC,UCHAR DbdcMode);
INT32 HcAcquireRadioForWdev(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev);
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd,struct wifi_dev *wdev);
INT32 HcUpdateChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
INT32 HcUpdateRadio(struct _RTMP_ADAPTER *pAd,UCHAR bw,UCHAR central_ch1,UCHAR control_ch2);
INT32 HcUpdateCsaCntByChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
INT32 HcUpdatePhyMode(struct _RTMP_ADAPTER *pAd, UCHAR PhyMode);
INT32 HcSetPhyMode(struct _RTMP_ADAPTER *pAd, UCHAR PhyMode);
UCHAR HcGetBandByWdev(struct wifi_dev *wdev);
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS CurStat);
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *pAd, UCHAR Channel, PHY_STATUS CurStat);
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd);
BOOLEAN IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev);
BOOLEAN IsHcRadioCurStatOffByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);
BOOLEAN IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
#ifdef GREENAP_SUPPORT
VOID HcSetGreenAPActiveByBand(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bGreenAPActive);
BOOLEAN IsHcGreenAPActiveByBand(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
BOOLEAN IsHcGreenAPActiveByWdev(struct wifi_dev *wdev);
#endif /* GREENAP_SUPPORT */
UCHAR HcGetChannelByBf(struct _RTMP_ADAPTER *pAd);
BOOLEAN HcIsRadioAcq(struct wifi_dev *wdev);
BOOLEAN HcIsBfCapSupport(struct wifi_dev *wdev);
UCHAR HcGetExtCha(struct _RTMP_ADAPTER *pAd,UCHAR Channel);
INT32 HcUpdateExtCha(struct _RTMP_ADAPTER *pAd,UCHAR Channel,UCHAR ExtCha);


/*Wtable Ctrl*/
UCHAR HcAcquireGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcReleaseGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx);
UCHAR HcGetMaxStaNum(struct _RTMP_ADAPTER *pAd);
UCHAR HcSetMaxStaNum(struct _RTMP_ADAPTER *pAd);

UCHAR HcGetWcidLinkType(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);
UCHAR HcAcquireUcastWcid(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev);
UCHAR HcReleaseUcastWcid(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev,UCHAR idx);
UCHAR HcHwAcquireWcid(struct _RTMP_ADAPTER *pAd,UCHAR idx);
UCHAR HcHwReleaseWcid(struct _RTMP_ADAPTER *pAd,UCHAR idx);
VOID HcWtblRecDump(struct _RTMP_ADAPTER *pAd);

VOID RxTrackingInit(struct wifi_dev *wdev);
VOID TaTidRecAndCmp(struct _RTMP_ADAPTER *pAd, struct _RXD_BASE_STRUCT *rx_base, UINT16 SN);


#ifdef DBDC_MODE
VOID HcShowBandInfo(struct _RTMP_ADAPTER *pAd);
#endif /*DBDC_MODE*/

INT32 HcCfgInit(struct _RTMP_ADAPTER *pAd);
VOID HcCfgExit(struct _RTMP_ADAPTER *pAd);
VOID HcCfgShow(struct _RTMP_ADAPTER *pAd);


/*
WMM
*/
VOID HcAcquiredEdca(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev,struct _EDCA_PARM *pEdca);
VOID HcReleaseEdca(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev);


/*
* OmacCtrl
*/
UCHAR HcGetOmacIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);


/*Should remove it*/
UCHAR HcGetChannelByRf(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);
UCHAR  HcGetCentralChByRf(struct _RTMP_ADAPTER *pAd,UCHAR RfIC);
UCHAR HcGetPhyModeByRf(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);
CHAR HcGetBwByRf(struct _RTMP_ADAPTER *pAd,UCHAR RfIC);
UCHAR HcGetRadioPhyMode(struct _RTMP_ADAPTER *pAd);
UCHAR HcGetRadioChannel(struct _RTMP_ADAPTER *pAd);
BOOLEAN  HcIsRfSupport(struct _RTMP_ADAPTER *pAd,UCHAR RfIC);
BOOLEAN  HcIsRfRun(struct _RTMP_ADAPTER *pAd,UCHAR RfIC);

VOID HcBbpSetBwByChannel(struct _RTMP_ADAPTER *pAd,UCHAR Bw, UCHAR Channel);
UCHAR HcGetBw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

UCHAR HcGetRadioRfIC(struct _RTMP_ADAPTER *pAd);

struct _QLOAD_CTRL* HcGetQloadCtrlByRf(struct _RTMP_ADAPTER *pAd, UINT32 RfIC);
struct _AUTO_CH_CTRL* HcGetAutoChCtrlByRf(struct _RTMP_ADAPTER *pAd, UINT32 RfIC);
struct _QLOAD_CTRL* HcGetQloadCtrl(struct _RTMP_ADAPTER *pAd);
struct _AUTO_CH_CTRL* HcGetAutoChCtrl(struct _RTMP_ADAPTER *pAd);
struct _OMAC_BSS_CTRL* HcGetOmacCtrl(struct _RTMP_ADAPTER *pAd);

UINT32 HcGetBmcQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UINT32 HcGetMgmtQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UINT32 HcGetBcnQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UINT32 HcGetTxRingIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UINT32 HcGetWmmIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR HcGetBandByChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
EDCA_PARM *HcGetEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcSetEdca(struct wifi_dev *wdev);
VOID HcCrossChannelCheck(struct _RTMP_ADAPTER *pAd,struct wifi_dev *wdev,UCHAR Channel);

#ifdef MAC_REPEATER_SUPPORT
INT32 HcAddRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx);
INT32 HcDelRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx);
UCHAR HcGetRepeaterOmac(struct _RTMP_ADAPTER *pAd,struct _MAC_TABLE_ENTRY *pEntry);
#endif /*#MAC_REPEATER_SUPPORT*/
UCHAR HcGetAmountOfBand(struct _RTMP_ADAPTER *pAd);
INT32 HcUpdateMSDUTxAllowByChannel(RTMP_ADAPTER *pAd,UCHAR Channel);
INT32 HcSuspendMSDUTxByChannel(RTMP_ADAPTER *pAd,UCHAR Channel);

INT hc_radio_acquire(struct wifi_dev *wdev,UCHAR bw,UCHAR ext_cha);

#endif /*__HDEV_CTRL_H__*/
