/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __ASIC_CTRL_H__
#define __ASIC_CTRL_H__

#ifdef MT_MAC
#include "hw_ctrl/cmm_asic_mt.h"
#include "hw_ctrl/cmm_asic_mt_fw.h"
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)
#include "hw_ctrl/cmm_asic_rt.h"
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

struct _RX_BLK;


#define TX_RTY_CFG_RTY_LIMIT_SHORT		0x1
#define TX_RTY_CFG_RTY_LIMIT_LONG		0x2

#define MINIMUM_POWER_VALUE		       -127
#define TX_STREAM_PATH		              4

#ifdef LINK_TEST_SUPPORT
#define CHANNEL_BAND_2G                   0
#define CHANNEL_BAND_5G                   1

#define CMW_RSSI_SOURCE_BBP               0
#define CMW_RSSI_SOURCE_WTBL              1

#define CMW_RCPI_MA_1_1                   1
#define CMW_RCPI_MA_1_2                   2
#define CMW_RCPI_MA_1_4                   4
#define CMW_RCPI_MA_1_8                   8

#define TX_DEFAULT_CSD_STATE              0
#define TX_ZERO_CSD_STATE				  1
#define TX_UNDEFINED_CSD_STATE			  0xFF

#define TX_DEFAULT_POWER_STATE            0
#define TX_BOOST_POWER_STATE			  1
#define TX_UNDEFINED_POWER_STATE		  0xFF

#define TX_DEFAULT_BW_STATE               0
#define TX_SWITCHING_BW_STATE			  1
#define TX_UNDEFINED_BW_STATE		      0xFF

#define TX_DEFAULT_RXSTREAM_STATE         0
#define TX_SIGNLE_RXSTREAM_STATE		  1
#define TX_UNDEFINED_RXSTREAM_STATE       0xFF

#define TX_DEFAULT_MAXIN_STATE            0
#define TX_SPECIFIC_ACR_STATE		      1
#define TX_UNDEFINED_RXFILTER_STATE       0xFF

#define CMW_POWER_UP_RATE_NUM             13
#define CMW_POWER_UP_CATEGORY_NUM         4
#endif /* LINK_TEST_SUPPORT */

VOID AsicNotSupportFunc(struct _RTMP_ADAPTER *pAd, const RTMP_STRING *caller);


VOID AsicUpdateRtsThld(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length);

VOID AsicUpdateProtect(
	struct _RTMP_ADAPTER *pAd,
	IN 		USHORT			OperaionMode,
	IN 		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist);

INT AsicSetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up,UCHAR BandIdx);
INT AsicSetRxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums,UCHAR BandIdx);
INT AsicSetBW(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
INT AsicSetCtrlCh(struct _RTMP_ADAPTER *pAd, UINT8 extch);
#ifdef DOT11_VHT_AC
INT AsicSetRtsSignalTA(struct _RTMP_ADAPTER *pAd);
#endif /* DOT11_VHT_AC */
VOID AsicAntennaSelect(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
VOID AsicResetBBPAgent(struct _RTMP_ADAPTER *pAd);
VOID AsicBBPAdjust(struct _RTMP_ADAPTER *pAd,UCHAR Channel);
VOID AsicSwitchChannel(struct _RTMP_ADAPTER *pAd, UCHAR ch, BOOLEAN bScan);
VOID AsicLockChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
INT AsicSetChannel(struct _RTMP_ADAPTER *pAd, UCHAR ch, UINT8 bw, UINT8 ext_ch, BOOLEAN bScan);


INT AsicSetDevMac(struct _RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx);
VOID AsicSetBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR curr_bssid_idx);
VOID AsicDelWcidTab(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);

#ifdef HTC_DECRYPT_IOT
VOID AsicSetWcidAAD_OM(struct _RTMP_ADAPTER *pAd, UCHAR Wcid , CHAR value);
#endif /* HTC_DECRYPT_IOT */

#ifdef MAC_APCLI_SUPPORT
VOID AsicSetApCliBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index);
#endif /* MAC_APCLI_SUPPORT */

INT AsicSetRxFilter(struct _RTMP_ADAPTER *pAd);

VOID AsicSetTmrCR(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);

#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(struct _RTMP_ADAPTER *pAd, 
        UCHAR wlan_idx, UCHAR band_idx, UCHAR init, UCHAR resp);
#ifdef MT_MAC
INT AsicWtblSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
INT AsicUpdateTxOP(struct _RTMP_ADAPTER *pAd, UINT32 ac_num, UINT32 txop_val);
#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */

INT AsicSetPreTbtt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx);
INT AsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
INT AsicSetChBusyStat(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT AsicGetTsfTime(
        struct _RTMP_ADAPTER *pAd,
        UINT32 *high_part,
        UINT32 *low_part,
        UCHAR HwBssidIdx);

VOID AsicSetSyncModeAndEnable(
        struct _RTMP_ADAPTER *pAd,
        USHORT BeaconPeriod,
        UCHAR HWBssidIdx,
        UCHAR OPMode);

VOID AsicDisableSync(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);

VOID AsicDisableBcnSntReq(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID AsicEnableBcnSntReq(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);


UINT32 AsicGetWmmParam(struct _RTMP_ADAPTER *pAd, UINT32 ac, UINT32 type);
INT AsicSetWmmParam(struct _RTMP_ADAPTER *pAd,UCHAR idx, UINT ac, UINT type, UINT val);

VOID AsicSetEdcaParm(struct _RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm, struct wifi_dev *wdev);

INT AsicSetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);

UINT32 AsicGetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type);

VOID AsicSetSlotTime(struct _RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR channel, struct wifi_dev *wdev);
INT AsicSetMacMaxLen(struct _RTMP_ADAPTER *pAd);

VOID AsicAddSharedKeyEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR BssIdx,
	IN UCHAR KeyIdx,
	IN PCIPHER_KEY pCipherKey);

VOID AsicRemoveSharedKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx);

VOID AsicUpdateWCIDIVEIV(struct _RTMP_ADAPTER *pAd, USHORT WCID, ULONG uIV, ULONG uEIV);
VOID AsicUpdateRxWCIDTable(struct _RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr, BOOLEAN IsBCMCWCID, BOOLEAN IsReset);
VOID AsicUpdateBASession(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type);
UINT16 AsicGetTidSn(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid);

#ifdef TXBF_SUPPORT
VOID AsicUpdateClientBfCap(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* TXBF_SUPPORT */

VOID AsicUpdateWcidAttributeEntry(
	struct _RTMP_ADAPTER *pAd,
	IN	UCHAR BssIdx,
	IN 	UCHAR KeyIdx,
	IN 	UCHAR CipherAlg,
	IN	UINT8 Wcid,
	IN	UINT8 KeyTabFlag);

BOOLEAN AsicCheckDMAIdle(struct _RTMP_ADAPTER *pAd, UINT8 Dir);

#ifdef MT_MAC
VOID AsicAddRemoveKeyTab (
    IN PRTMP_ADAPTER pAd,
    IN ASIC_SEC_INFO *pInfo);
#endif


#ifdef MCS_LUT_SUPPORT
VOID AsicMcsLutUpdate(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* MCS_LUT_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssMode(struct _RTMP_ADAPTER *pAd, UCHAR NumOfBcns);
VOID AsicSetMbssWdevIfAddr(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
VOID AsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, VOID *wdev_void, INT opmode);

#endif /* CONFIG_AP_SUPPORT */
INT AsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev);
INT AsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev, UCHAR NumOfBcns);

INT32 AsicDevInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucOwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 u4EnableFeature);

INT32 AsicStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 ucBssIndex,
	UINT8 ucWlanIdx,
	UINT32 ConnectionType,
	UINT8 ConnectionState,
	UINT32 u4EnableFeature,
	UINT8 IsNewSTARec);

INT32 AsicRaParamStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 WlanIdx,
	struct _STAREC_AUTO_RATE_UPDATE_T *prParam,
	UINT32 EnableFeature);

INT32 AsicBssInfoUpdate(
    struct _RTMP_ADAPTER *pAd,
    struct _BSS_INFO_ARGUMENT_T bss_info_argument);

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT32 AsicBfStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
    UCHAR                ucPhyMode,
    UCHAR                ucBssIdx,
	UCHAR                ucWlanIdx);

INT32 AsicBfStaRecRelease(
	struct _RTMP_ADAPTER *pAd,
    UCHAR                ucBssIdx,
	UCHAR                ucWlanIdx);

INT32 AsicBfPfmuMemAlloc(
	struct _RTMP_ADAPTER *pAd,
    UCHAR                ucSu_Mu, 
    UCHAR                ucWlanId);

INT32 AsicBfPfmuMemRelease(
	struct _RTMP_ADAPTER *pAd,
    UCHAR                ucWlanId);

INT32 AsicTxBfTxApplyCtrl(
	struct _RTMP_ADAPTER *pAd,
    UCHAR                ucWlanId, 
    BOOLEAN              fgETxBf, 
    BOOLEAN              fgITxBf, 
    BOOLEAN              fgMuTxBf,
    BOOLEAN              fgPhaseCali);

INT32 AsicTxBfeeHwCtrl(
	RTMP_ADAPTER *pAd,
    BOOLEAN   fgBfeeHwCtrl);

INT32 AsicTxBfApClientCluster(
	struct _RTMP_ADAPTER *pAd,
    UCHAR                ucWlanId,
    UCHAR                ucCmmWlanId);

INT32 AsicTxBfReptClonedStaToNormalSta(
	RTMP_ADAPTER *pAd,
    UCHAR   ucWlanId,
    UCHAR   ucCliIdx);

INT32 AsicTxBfHwEnStatusUpdate(
	struct _RTMP_ADAPTER *pAd,
    BOOLEAN              fgETxBf,
    BOOLEAN              fgITxBf);
#endif /* MT_MAC && TXBF_SUPPORT */

#define AsicBssInfoReNew(pAd, bss_info_argument) AsicBssInfoUpdate(pAd, bss_info_argument)

INT32 AsicExtPwrMgtBitWifi(struct _RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit);
INT32 AsicRadioOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio);
#ifdef GREENAP_SUPPORT
INT32 AsicGreenAPOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAPOn);
#endif /* GREENAP_SUPPORT */
INT32 AsicExtPmStateCtrl(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg, UINT8 ucPmNumber, UINT8 ucPmState);
INT32 AsicExtWifiHifCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 PmStatCtrl, VOID *pResult);

INT32 AsicMccStart(struct _RTMP_ADAPTER *ad,
    UCHAR channel_1st,
    UCHAR channel_2nd,
    UINT32 bw_1st,
    UINT32 bw_2nd,
    UCHAR central_1st_seg0,
    UCHAR central_1st_seg1,
    UCHAR central_2nd_seg0,
    UCHAR central_2nd_seg1,
    UCHAR role_1st,
    UCHAR role_2nd,
    USHORT stay_time_1st,
    USHORT stay_time_2nd,
    USHORT idle_time,
    USHORT null_repeat_cnt,
    UINT32 start_tsf);

INT32 AsicBfSoundingPeriodicTriggerCtrl(struct _RTMP_ADAPTER *pAd, UINT32 WlanIdx, UINT8 On);
#ifdef THERMAL_PROTECT_SUPPORT
INT32
AsicThermalProtect(
    RTMP_ADAPTER *pAd,
    UINT8 HighEn,
    CHAR HighTempTh,
    UINT8 LowEn,
    CHAR LowTempTh,
    UINT32 RechkTimer,
    UINT8 RFOffEn,
    CHAR RFOffTh,
    UINT8 ucType);

INT32
AsicThermalProtectAdmitDuty(
	RTMP_ADAPTER *pAd,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
	);
#endif /* THERMAL_PROTECT_SUPPORT */


INT AsicSendCommandToMcu(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR         Command,
	IN UCHAR         Token,
	IN UCHAR         Arg0,
	IN UCHAR         Arg1,
	IN BOOLEAN in_atomic);

BOOLEAN AsicSendCmdToMcuAndWait(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);

BOOLEAN AsicSendCommandToMcuBBP(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked);


#ifdef WAPI_SUPPORT
VOID AsicUpdateWAPIPN(
	struct _RTMP_ADAPTER *pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high);
#endif /* WAPI_SUPPORT */



#ifdef STREAM_MODE_SUPPORT
VOID AsicSetStreamMode(
	struct _RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled);

VOID AsicStreamModeInit(struct _RTMP_ADAPTER *pAd);

#endif /*STREAM_MODE_SUPPORT*/


VOID AsicUpdateAutoFallBackTable(struct _RTMP_ADAPTER *pAd, UCHAR *pTxRate);
INT AsicSetAutoFallBack(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT AsicAutoFallbackInit(struct _RTMP_ADAPTER *pAd);
VOID AsicSetRxAnt(struct _RTMP_ADAPTER *pAd,UCHAR	Ant);




#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(
	struct _RTMP_ADAPTER *pAd
);

VOID AsicMitigateMicrowave(
	struct _RTMP_ADAPTER *pAd
);
#endif /* MICROWAVE_OVEN_SUPPORT */

VOID AsicBbpInitFromEEPROM(struct _RTMP_ADAPTER *pAd);


VOID AsicSetPiggyBack(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
VOID AsicGetTxTsc(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc);
VOID AsicSetSMPS(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps);
VOID AsicTurnOffRFClk(struct _RTMP_ADAPTER *pAd, UCHAR Channel);

#ifdef MAC_REPEATER_SUPPORT

VOID AsicInsertRepeaterEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr);

VOID AsicRemoveRepeaterEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx);

#ifdef MT_MAC
VOID AsicInsertRepeaterRootEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN  UCHAR *pAddr,
	IN UCHAR ReptCliIdx);
#endif /* MT_MAC */
#endif /* MAC_REPEATER_SUPPORT*/


INT32 AsicRxHeaderTransCtl(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
INT32 AsicRxHeaderTaranBLCtl(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID AsicSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID AsicSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
#endif /* CONFIG_AP_SUPPORT */
INT32 AsicGetMacInfo(struct _RTMP_ADAPTER *pAd, UINT32 *ChipId,UINT32 *HwVer, UINT32 *FwVer);
INT32 AsicGetAntMode(struct _RTMP_ADAPTER *pAd,UCHAR *AntMode);

INT32 AsicSetDmaByPassMode(struct _RTMP_ADAPTER *pAd,BOOLEAN isByPass);


#ifdef DBDC_MODE
INT32 AsicSetDbdcCtrl(struct _RTMP_ADAPTER *pAd,struct _BCTRL_INFO_T *pBctrlInfo);
INT32 AsicGetDbdcCtrl(struct _RTMP_ADAPTER *pAd,struct _BCTRL_INFO_T *pBctrlInfo);
#endif /*DBDC_MODE*/



VOID AsicNotSupportFunc(struct _RTMP_ADAPTER *pAd, const RTMP_STRING *caller);

UINT32 AsicFillRxBlkAndPktProcess(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket);

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN AsicMcastEntryInsert(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);

BOOLEAN AsicMcastEntryDelete(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);
#endif



VOID RssiUpdate(struct _RTMP_ADAPTER *pAd);

INT AsicAmpduEfficiencyAdjust(struct wifi_dev *wdev, UCHAR	aifs_adjust);
INT AsicRtsOnOff(struct wifi_dev *wdev, BOOLEAN rts_en);

typedef struct _RTMP_ARCH_OP {
	UINT32 (*archGetCrcErrCnt)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*archGetCCACnt)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*archGetChBusyCnt)(struct _RTMP_ADAPTER *pAd, UCHAR ch_idx);
#ifdef FIFO_EXT_SUPPORT
	BOOLEAN (*archGetFifoTxCnt)(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
	VOID (*archFifoExtSet)(struct _RTMP_ADAPTER *pAd);
	VOID (*archFifoExtEntryClean)(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* FIFO_EXT_SUPPORT */

	INT (*archSetAutoFallBack)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
	INT (*archAutoFallbackInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*archUpdateProtect)(struct _RTMP_ADAPTER *pAd, MT_PROTECT_CTRL_T *Protect);
	VOID (*archUpdateRtsThld)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_thld, UINT32 len_thld);
#ifdef DOT11_N_SUPPORT
	INT (*archSetRDG)(struct _RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg);
#endif /* DOT11_N_SUPPORT */
	VOID (*archSwitchChannel)(struct _RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg);
#ifdef ANT_DIVERSITY_SUPPORT
	VOID (*archAntennaSelect)(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
#endif /* ANT_DIVERSITY_SUPPORT */

	VOID (*archResetBBPAgent)(struct _RTMP_ADAPTER *pAd);


	INT32 (*archSetDevMac)(
            struct _RTMP_ADAPTER *pAd,
            UINT8 OwnMacIdx,
            UINT8 *OwnMacAddr,
            UINT8 BandIdx,
            UINT8 Active,
            UINT32 EnableFeature);

    INT32 (*archSetBssid)(
            struct _RTMP_ADAPTER *pAd,
            struct _BSS_INFO_ARGUMENT_T bss_info_argument);

	INT32 (*archSetStaRec)(struct _RTMP_ADAPTER *pAd,STA_REC_CFG_T StaCfg);

	VOID (*archDelWcidTab)(struct _RTMP_ADAPTER *pAd, UCHAR wcid_idx);

#ifdef HTC_DECRYPT_IOT
	VOID (*archSetWcidAAD_OM)(struct _RTMP_ADAPTER *pAd, UCHAR wcid_idx, UCHAR value);
#endif /* HTC_DECRYPT_IOT */

	VOID (*archAddRemoveKeyTab)(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);

    INT (*archEnableBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void, UCHAR NumOfBcns);
    INT (*archDisableBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);

#ifdef CONFIG_AP_SUPPORT
	VOID (*archSetMbssMode)(struct _RTMP_ADAPTER *pAd, UCHAR NumOfBcns);
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	INT (*archSetReptFuncEnable)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);

	VOID (*archInsertRepeaterEntry)(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx, PUCHAR pAddr);
	VOID (*archRemoveRepeaterEntry)(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx);
	VOID (*archInsertRepeaterRootEntry)(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR *pAddr, UCHAR ReptCliIdx);
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


	INT (*archSetRxFilter)(struct _RTMP_ADAPTER *pAd,MT_RX_FILTER_CTRL_T RxFilter);

	VOID (*archSetPiggyBack)(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
	INT (*archSetPreTbtt)(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR HwBssidIdx);
	INT (*archSetGPTimer)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
	INT (*archSetChBusyStat)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
	INT (*archGetTsfTime)(
            struct _RTMP_ADAPTER *pAd,
            UINT32 *high_part,
            UINT32 *low_part,
            UCHAR HwBssidIdx);

	VOID (*archDisableSync)(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);

    VOID (*archSetSyncModeAndEnable)(
            struct _RTMP_ADAPTER *pAd,
            USHORT BeaconPeriod,
            UCHAR HWBssidIdx,
            UCHAR OPMode);

	VOID (*archDisableBcnSntReq)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wifiDev);
	VOID (*archEnableBcnSntReq)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wifiDev);


	INT (*archSetWmmParam)(struct _RTMP_ADAPTER *pAd,UCHAR idx, UINT ac, UINT type, UINT val);
	VOID (*archSetEdcaParm)(struct _RTMP_ADAPTER *pAd, struct _EDCA_PARM *pEdcaParm);
	UINT32 (*archGetWmmParam)(struct _RTMP_ADAPTER *pAd,  UINT32 AcNum, UINT32 EdcaType);
	INT (*archSetRetryLimit)(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
	UINT32 (*archGetRetryLimit)(struct _RTMP_ADAPTER *pAd, UINT32 type);
	VOID (*archSetSlotTime)(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime,UCHAR BandIdx);
	INT (*archSetMacMaxLen)(struct _RTMP_ADAPTER *pAd);
	VOID (*archGetTxTsc)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc);
	VOID (*archAddSharedKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx, struct _CIPHER_KEY *pCipherKey);
	VOID (*archRemoveSharedKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx);
	VOID (*archAddPairwiseKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR WCID, PCIPHER_KEY pCipherKey);
	VOID (*archRemovePairwiseKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);

	VOID (*archUpdateWCIDIVEIV)(struct _RTMP_ADAPTER *pAd, USHORT WCID, ULONG uIV, ULONG uEIV);
	VOID (*archUpdateWcidAttributeEntry)(struct _RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx, UCHAR CipherAlg, UINT8 Wcid, UINT8 KeyTabFlag);

#ifdef MCS_LUT_SUPPORT
	VOID (*archMcsLutUpdate)(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* MCS_LUT_SUPPORT */

    INT (*archSetRtsSignalTA)(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, BOOLEAN Enable);
	INT (*archSetRxStream)(struct _RTMP_ADAPTER *pAd, UINT32 rx_path,UCHAR BandIdx);
	INT (*archSetTxStream)(struct _RTMP_ADAPTER *pAd, UINT32 rx_path,UCHAR BandIdx);
	INT (*archSetBW)(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
	INT (*archSetCtrlCh)(struct _RTMP_ADAPTER *pAd, UINT8 extch);
	VOID (*archSetApCliBssid)(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index);

	INT (*archWaitMacTxRxIdle)(struct _RTMP_ADAPTER *pAd);
	INT (*archSetMacTxRx)(struct _RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable,UCHAR BandIdx);
	INT (*archSetMacTxQ)(struct _RTMP_ADAPTER *pAd, INT WmmSet, INT band, BOOLEAN Enable);
	INT (*archSetWPDMA)(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable);
	BOOLEAN (*archWaitPDMAIdle)(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us);
	BOOLEAN (*archCheckDMAIdle)(struct _RTMP_ADAPTER *pAd,UINT8 Dir);
	INT (*archSetMacWD)(struct _RTMP_ADAPTER *pAd);

	INT (*archTOPInit)(struct _RTMP_ADAPTER *pAd);

    VOID (*archSetTmrCR)(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);

#ifdef CONFIG_AP_SUPPORT
    VOID (*archSetMbssWdevIfAddr)(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
    VOID (*archSetMbssHwCRSetting)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
    VOID (*archSetExtTTTTHwCRSetting)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
    VOID (*archSetExtMbssEnableCR)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
#endif

#ifdef DBDC_MODE
	INT (*archSetDbdcCtrl)(struct _RTMP_ADAPTER *pAd,struct _BCTRL_INFO_T *pBctrInfo);
	INT (*archGetDbdcCtrl)(struct _RTMP_ADAPTER *pAd,struct _BCTRL_INFO_T *pBctrInfo);
#endif

VOID (*archUpdateBcnToAsicMethod)(struct _RTMP_ADAPTER *pAd, INT apidx, ULONG FrameLen, ULONG UpdatePos);

	VOID (*archUpdateRxWCIDTable)(struct _RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo);

#ifdef TXBF_SUPPORT
    VOID (*archUpdateClientBfCap)(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif

#if defined(RTMP_MAC) || defined(RLT_MAC)
	INT32 (* archUpdateBASession)(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UCHAR basize, BOOLEAN isAdd, INT ses_type);
#endif /* RTMP_MAC || RLT_MAC */
#ifdef MT_MAC
	INT32 (* archUpdateBASession)(struct _RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl);
    UINT16 (* archGetTidSn)(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid);
#endif /*MT_MAC */
	INT32 (* archUpdateStaRecBa)(struct _RTMP_ADAPTER *pAd, STA_REC_BA_CFG_T StaRecCfg);
	VOID (* archSetSMPS)(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps);
#ifdef MT_PS
	VOID (* archSetIgnorePsm)(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, UCHAR value);
#endif /* MT_PS */
	INT32 (*archRxHeaderTransCtl)(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
	INT32 (*archRxHeaderTaranBLCtl)(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);
	UINT32 (*archFillRxBlkAndPktProcess)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, VOID *pRxPacket);

#ifdef IGMP_SNOOP_SUPPORT
	BOOLEAN (*archMcastEntryInsert)(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);

	BOOLEAN (*archMcastEntryDelete)(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex);
#endif
	INT (*asic_ampdu_efficiency_on_off)(struct _RTMP_ADAPTER *ad, UCHAR	wmm_idx, UCHAR aifs_adjust);
	INT (*asic_rts_on_off)(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en);
}RTMP_ARCH_OP;

#ifdef LINK_TEST_SUPPORT
VOID LinkTestRcpiSet(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UINT8 AntIdx, CHAR cRcpi);
VOID LinkTestTimeSlotHandler(struct _RTMP_ADAPTER *pAd);
VOID LinkTestRxStreamCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, CHAR cMaxRssi, CHAR *cRssi, struct _MAC_TABLE_ENTRY *pEntry, UINT8 ucBandIdx);
VOID LinkTestTxBwCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus);
VOID LinkTestTxCsdCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus);
VOID LinkTestTxPowerCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus);
VOID LinkTestAcrCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, CHAR cMaxRssi, UINT8 ucBandIdx);
UINT8 LinkTestRssiCheck(struct _RTMP_ADAPTER *pAd, CHAR *cRssi, UCHAR ucRSSIThManual, UINT8 ucRSSISource, UINT8 ucBandIdx);
UINT8 LinkTestRssiComp(struct _RTMP_ADAPTER *pAd, CHAR *cRssi, UCHAR ucRSSIThManual, UINT8 ucStart, UINT8 ucEnd);
#endif /* LINK_TEST_SUPPORT */

#endif /* __ASIC_CTRL_H_ */
