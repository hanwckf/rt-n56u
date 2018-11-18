/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hw_ctrl.c
*/
#include	"rt_config.h"

extern NDIS_STATUS HwCtrlEnqueueCmd(
	RTMP_ADAPTER	*pAd,
	HW_CTRL_TXD HwCtrlTxd);


/*Only can used in this file*/
static INT32 HW_CTRL_BASIC_ENQ(RTMP_ADAPTER *pAd,INT32 CmdType,INT32 CmdId,UINT32 Len,VOID *pBuffer)
{
	HW_CTRL_TXD HwCtrlTxd;
	UINT32 ret ;
	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));
	HwCtrlTxd.CmdType = CmdType;
	HwCtrlTxd.CmdId = CmdId;
	HwCtrlTxd.NeedWait = FALSE;
	HwCtrlTxd.wait_time = 0;
	HwCtrlTxd.InformationBufferLength = Len;
	HwCtrlTxd.pInformationBuffer = pBuffer;
	HwCtrlTxd.pRespBuffer = NULL;
	HwCtrlTxd.RespBufferLength = 0;
	HwCtrlTxd.CallbackFun = NULL;
	HwCtrlTxd.CallbackArgs = NULL;
	ret = HwCtrlEnqueueCmd(pAd,HwCtrlTxd);
	return ret;
}

#define HW_CTRL_TXD_BASIC(_pAd,_CmdType,_CmdId,_Len,_pBuffer,_HwCtrlTxd) \
{ \
	_HwCtrlTxd.CmdType = _CmdType; \
	_HwCtrlTxd.CmdId = _CmdId; \
	_HwCtrlTxd.NeedWait = FALSE; \
	_HwCtrlTxd.wait_time = 0;\
	_HwCtrlTxd.InformationBufferLength = _Len; \
	_HwCtrlTxd.pInformationBuffer = _pBuffer; \
	_HwCtrlTxd.pRespBuffer = NULL; \
	_HwCtrlTxd.RespBufferLength = 0; \
	_HwCtrlTxd.CallbackFun = NULL; \
	_HwCtrlTxd.CallbackArgs = NULL; \
}

#define HW_CTRL_TXD_RSP(_pAd,_RspLen,_RspBuffer,_wait_time,_HwCtrlTxd) \
{ \
	_HwCtrlTxd.NeedWait = TRUE; \
	_HwCtrlTxd.wait_time = _wait_time;\
	_HwCtrlTxd.pRespBuffer = _RspBuffer; \
	_HwCtrlTxd.RespBufferLength = _RspLen; \
}

#define HW_CTRL_TXD_CALLBACK(_pAd,_CallbackFun,_CallbackArgs,_HwCtrlTxd) \
{ \
 	_HwCtrlTxd.CallbackFun = _CallbackFun; \
	_HwCtrlTxd.CallbackArgs = _CallbackArgs; \
}



/*CMD Definition Start*/
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
#else
/*Define Export API for common part usage.*/
VOID RTMP_UPDATE_PROTECT(PRTMP_ADAPTER pAd, USHORT OperationMode, UCHAR SetMask, BOOLEAN bDisableBGProtect, BOOLEAN bNonGFExist)
{
	RT_ASIC_PROTECT_INFO AsicProtectInfo;
	INT32 ret;
	AsicProtectInfo.OperationMode = (OperationMode);
	AsicProtectInfo.SetMask = (SetMask);
	AsicProtectInfo.bDisableBGProtect = (bDisableBGProtect);
	AsicProtectInfo.bNonGFExist = (bNonGFExist);
	ret = -1;
}


VOID RTMP_SET_TR_ENTRY(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
	RT_SET_TR_ENTRY Info;
	UINT32 ret;
	Info.WCID = pEntry->wcid;
	Info.pEntry = (VOID *)pEntry;
	ret = HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_RADIO,HWCMD_ID_SET_TR_ENTRY,sizeof(RT_SET_TR_ENTRY),&Info);
}

VOID HW_SET_WCID_SEC_INFO(PRTMP_ADAPTER pAd, UCHAR BssIdx, UCHAR KeyIdx, UCHAR CipherAlg, UINT8 Wcid, UINT8 KeyTabFlag)
{
	RT_ASIC_WCID_SEC_INFO Info;
	Info.BssIdx = BssIdx;
	Info.KeyIdx = KeyIdx;
	Info.CipherAlg = CipherAlg;
	Info.Wcid = Wcid;
	Info.KeyTabFlag = KeyTabFlag;
	HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_SECURITY,HWCMD_ID_SET_WCID_SEC_INFO,sizeof(RT_ASIC_WCID_SEC_INFO),&Info);
}




#ifdef CONFIG_AP_SUPPORT

VOID RTMP_AP_ADJUST_EXP_ACK_TIME(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
 	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO,HWCMD_ID_AP_ADJUST_EXP_ACK_TIME,0,NULL);
}

VOID RTMP_AP_RECOVER_EXP_ACK_TIME(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
 	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO,HWCMD_ID_AP_RECOVER_EXP_ACK_TIME,0,NULL);
}

#endif /* CONFIG_AP_SUPPORT */


VOID RTMP_SET_LED_STATUS(PRTMP_ADAPTER pAd, UCHAR Status)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PERIPHERAL,HWCMD_ID_SET_LED_STATUS,sizeof(UCHAR),&Status);
}

VOID RTMP_SET_LED(PRTMP_ADAPTER pAd, UINT32 WPSLedMode10)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_PERIPHERAL,HWCMD_ID_LED_WPS_MODE10,sizeof(WPSLedMode10), &WPSLedMode10);
}
#endif  /*defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */


// PCI, USB, SDIO use the same LP function


VOID RTMP_GET_TEMPERATURE(RTMP_ADAPTER *pAd,UINT32 *pTemperature)
{
	UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));

	HW_CTRL_TXD_BASIC(pAd,HWCMD_TYPE_RADIO,HWCMD_ID_GET_TEMPERATURE,0,NULL,HwCtrlTxd);
	HW_CTRL_TXD_RSP(pAd,sizeof(UINT32),pTemperature,0,HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd,HwCtrlTxd);
}

VOID RTMP_RADIO_ON_OFF_CTRL(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio)
{
	HW_CTRL_TXD HwCtrlTxd;
	RADIO_ON_OFF_T RadioOffOn = {0};

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));
	
	RadioOffOn.ucDbdcIdx = ucDbdcIdx;
	RadioOffOn.ucRadio = ucRadio;

	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_RADIO_ON_OFF, sizeof(RADIO_ON_OFF_T), &RadioOffOn, HwCtrlTxd);
	HW_CTRL_TXD_RSP(pAd, 0, NULL, HWCTRL_CMD_WAITTIME, HwCtrlTxd);
	
	HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

#ifdef GREENAP_SUPPORT
VOID RTMP_GREENAP_ON_OFF_CTRL(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAP)
{
	GREENAP_ON_OFF_T GreenAPCtrl = {0};

	GreenAPCtrl.ucDbdcIdx = ucDbdcIdx;
	GreenAPCtrl.ucGreenAPOn = ucGreenAP;

	if (HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_GREENAP_ON_OFF, sizeof(GREENAP_ON_OFF_T), &GreenAPCtrl) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Failed to enqueue cmd\n", __FUNCTION__));
	}	
}
#endif /* GREENAP_SUPPORT */

VOID RTMP_UPDATE_RAW_COUNTER(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_DAW_COUNTER,0,NULL);
}

#ifdef MT_MAC

#ifdef PRETBTT_INT_EVENT_SUPPORT
VOID RTMP_HANDLE_PRETBTT_INT_EVENT(PRTMP_ADAPTER pAd)
{
	UINT32 ret;
    MT_UPDATE_BEACON rMtUpdateBeacon;
    os_zero_mem(&rMtUpdateBeacon, sizeof(MT_UPDATE_BEACON));

    rMtUpdateBeacon.wdev = NULL;
    rMtUpdateBeacon.UpdateReason = PRETBTT_UPDATE;

	ret = HW_CTRL_BASIC_ENQ(pAd,
                        HWCMD_TYPE_RADIO,
                        HWCMD_ID_UPDATE_BEACON,
                        sizeof(MT_UPDATE_BEACON),
                        &rMtUpdateBeacon);
}
#endif

VOID HW_ADDREMOVE_KEYTABLE (struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo)
{
	HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_SECURITY,HWCMD_ID_ADDREMOVE_ASIC_KEY,sizeof(ASIC_SEC_INFO),pInfo);
}

VOID HW_SET_DEL_ASIC_WCID(PRTMP_ADAPTER pAd, ULONG Wcid)
{
	RT_SET_ASIC_WCID	SetAsicWcid;
	SetAsicWcid.WCID = Wcid;
	HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_HT_CAP,HWCMD_ID_DEL_ASIC_WCID,sizeof(RT_SET_ASIC_WCID),&SetAsicWcid);
}


#ifdef HTC_DECRYPT_IOT
VOID HW_SET_ASIC_WCID_AAD_OM(PRTMP_ADAPTER pAd, ULONG Wcid, UCHAR value)
{
	RT_SET_ASIC_AAD_OM SetAsicAAD_OM;
	SetAsicAAD_OM.WCID = Wcid;
	SetAsicAAD_OM.Value = value;
	HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_HT_CAP,HWCMD_ID_SET_ASIC_AAD_OM,sizeof(RT_SET_ASIC_AAD_OM),&SetAsicAAD_OM);
}
#endif /* HTC_DECRYPT_IOT */



#ifdef BCN_OFFLOAD_SUPPORT
VOID HW_SET_BCN_OFFLOAD(RTMP_ADAPTER *pAd,
    UINT8 WdevIdx,
    ULONG WholeLength,
    BOOLEAN Enable,
    UCHAR OffloadPktType,
    ULONG TimIePos,
    ULONG CsaIePos)
{
    UINT32 ret;
    MT_SET_BCN_OFFLOAD rMtSetBcnOffload;

    os_zero_mem(&rMtSetBcnOffload,sizeof(MT_SET_BCN_OFFLOAD));

    rMtSetBcnOffload.WdevIdx = WdevIdx;
    rMtSetBcnOffload.WholeLength = WholeLength;
    rMtSetBcnOffload.Enable = Enable;
    rMtSetBcnOffload.OffloadPktType = OffloadPktType;
    rMtSetBcnOffload.TimIePos = TimIePos;
    rMtSetBcnOffload.CsaIePos = CsaIePos;

    ret = HW_CTRL_BASIC_ENQ(pAd,
                            HWCMD_TYPE_RADIO,
                            HWCMD_ID_SET_BCN_OFFLOAD,
                            sizeof(rMtSetBcnOffload),
                            &rMtSetBcnOffload);
}
#endif /*BCN_OFFLOAD_SUPPORT*/

VOID HW_UPDATE_BSSINFO(RTMP_ADAPTER *pAd,BSS_INFO_ARGUMENT_T BssInfoArgs)
{
	UINT32 ret;
 	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_BSSINFO, sizeof(BSS_INFO_ARGUMENT_T), &BssInfoArgs);
}

VOID HW_SET_BA_REC(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type)
{
	UINT32 ret;
	MT_BA_CTRL_T SetBaRec;

	os_zero_mem(&SetBaRec,sizeof(MT_BA_CTRL_T));
	SetBaRec.Wcid = wcid;
	SetBaRec.Tid = tid;
	SetBaRec.Sn = sn;
	SetBaRec.BaWinSize = basize;
	SetBaRec.isAdd = isAdd;
	SetBaRec.BaSessionType = ses_type;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_BA_REC,sizeof(MT_BA_CTRL_T), &SetBaRec);
}

#ifdef MT_PS
VOID RTMP_PS_RETRIVE_CLEAR(PRTMP_ADAPTER pAd, UCHAR Wcid)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS,HWCMD_ID_PS_CLEAR,sizeof(UCHAR), (VOID *)&Wcid);
}

VOID RTMP_PS_RETRIVE_START(PRTMP_ADAPTER pAd, UCHAR Wcid)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PS,HWCMD_ID_PS_RETRIEVE_START, sizeof(UCHAR),(VOID *)&Wcid);
}
#endif /*MT_PS*/

#ifdef ERR_RECOVERY
VOID RTMP_MAC_RECOVERY(struct _RTMP_ADAPTER *pAd, UINT32 Status)
{
	UINT32 value;

	value = Status & ERROR_DETECT_MASK;
	/* Trigger error recovery process with fw reload. */

	if (pAd->HwCtrl.ser_func_state != RTMP_TASK_STAT_RUNNING) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s::ERR! SER func not ready(%d)\n", 
			__FUNCTION__, pAd->HwCtrl.ser_func_state));
		// TODO: do we may hit this case?
		return;
	}

	if (value != pAd->HwCtrl.ser_status) {
		pAd->HwCtrl.ser_status = value;
		RTCMDUp(&pAd->HwCtrl.ser_task);
	} else {
		// TODO: do we may hit this case?
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s::ERR! prev state=%x, new stat=%x\n", 
			__FUNCTION__, pAd->HwCtrl.ser_status, value));
	}
}

INT IsStopingPdma(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
    return (pErrRecoveryCtl->errRecovState == ERR_RECOV_STOP_IDLE) ?
        FALSE : TRUE;
}

BOOLEAN IsErrRecoveryInIdleStat(RTMP_ADAPTER *pAd)
{
    UINT32 Stat;
        
    if (pAd == NULL)
        return TRUE;


    Stat = ErrRecoveryCurStat(&pAd->ErrRecoveryCtl);

    if (Stat == ERR_RECOV_STOP_IDLE)
        return TRUE;
    else
        return FALSE;
}

ERR_RECOVERY_STATE ErrRecoveryCurStat(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
    if (pErrRecoveryCtl == NULL)
        return ERR_RECOV_STOP_IDLE;

    return pErrRecoveryCtl->errRecovState;
}
#endif /* ERR_RECOVERY */
#endif /*MT_MAC*/

#ifdef VOW_SUPPORT
VOID RTMP_SET_STA_DWRR(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
    UINT32 ret;
    MT_VOW_STA_GROUP VoW;
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));
	os_zero_mem(&VoW, sizeof(MT_VOW_STA_GROUP));

    VoW.StaIdx = pEntry->wcid;
    VoW.GroupIdx = pEntry->func_tb_idx;

	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_STA_DWRR, sizeof(MT_VOW_STA_GROUP), &VoW, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

VOID RTMP_SET_STA_DWRR_QUANTUM(PRTMP_ADAPTER pAd, BOOLEAN restore, UCHAR quantum)
{
    UINT32 ret;
	MT_VOW_STA_QUANTUM VoW;
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));
	os_zero_mem(&VoW, sizeof(MT_VOW_STA_QUANTUM));

	VoW.restore = restore;
	VoW.quantum = quantum;
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_STA_DWRR_QUANTUM, sizeof(MT_VOW_STA_GROUP), &VoW, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#endif /* VOW_SUPPORT */

#ifdef THERMAL_PROTECT_SUPPORT
VOID RTMP_SET_THERMAL_RADIO_OFF(PRTMP_ADAPTER pAd)
{
    UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));

	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_THERMAL_PROTECTION_RADIOOFF, 0, NULL, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#endif /* THERMAL_PROTECT_SUPPORT */

#ifdef LINK_TEST_SUPPORT
VOID RTMP_AUTO_LINK_TEST(PRTMP_ADAPTER pAd)
{
    UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));

	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_AUTO_LINK_TEST, 0, NULL, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}
#endif /* LINK_TEST_SUPPORT */

VOID RTMP_SET_UPDATE_RSSI(PRTMP_ADAPTER pAd)
{
    UINT32 ret;
    HW_CTRL_TXD HwCtrlTxd;

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));
	
	HW_CTRL_TXD_BASIC(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_UPDATE_RSSI, 0, NULL, HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd, HwCtrlTxd);
}

/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID NICUpdateRawCountersNew(
	IN PRTMP_ADAPTER pAd)
{
	if (HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_PS, HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS, 0, NULL) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Failed to enqueue cmd\n", __FUNCTION__));
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief     This routine calculates the acumulated TxPER of eaxh TxRate. And
*            according to the calculation result, change CommonCfg.TxRate which
*            is the stable TX Rate we expect the Radio situation could sustained.
*
* \param[in] pAd
*
* \return    None
*/
/*----------------------------------------------------------------------------*/


VOID HW_SET_SLOTTIME(RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR Channel, struct wifi_dev *wdev)
{
	UINT32 ret;
	SLOT_CFG SlotCfg;

	SlotCfg.bUseShortSlotTime = bUseShortSlotTime;
	SlotCfg.Channel = Channel;
    SlotCfg.wdev = wdev;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_SLOTTIME, sizeof(SLOT_CFG), (VOID *)&SlotCfg);
}


VOID RTMP_SET_TX_BURST(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, BOOLEAN enable)
{
	UINT32 ret;
	WMM_CFG wmm_cfg;
    EDCA_PARM *edca_param = &wmm_cfg.EdcaParm;
	RTMP_CHIP_CAP *chip_cap = &pAd->chipCap;

	os_zero_mem(&wmm_cfg, sizeof(WMM_CFG));
	wmm_cfg.wdev = wdev;

	/*since RDG not ready, for APCLI should set TXOP to 0x80 (remove when RDG ready)*/
#ifdef CONFIG_RALINK_MT7621
	if (wdev->fAnyStationPeekTpBound == TRUE) {
		chip_cap->default_txop = 0x30;
	} else
#endif /* CONFIG_RALINK_MT7621 */
	if(pAd->CommonCfg.bRdg)
	{
	#define RDG_TXOP 0x80
		chip_cap->default_txop = RDG_TXOP;
	}else
	{
	#define TXBURST_TXOP 0x60
		chip_cap->default_txop = TXBURST_TXOP;
	}

    edca_param->Txop[WMM_AC_BE] = (enable) ? (chip_cap->default_txop) : (0x0);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%pS, %s: enable=%x, txop=%x\n",
            __builtin_return_address(0), __FUNCTION__, 
            enable, edca_param->Txop[WMM_AC_BE]));

	ret = HW_CTRL_BASIC_ENQ(pAd, 
            HWCMD_TYPE_RADIO,
            HWCMD_ID_SET_TX_BURST,
            sizeof(WMM_CFG),
            (VOID *)&wmm_cfg);
}


VOID HW_SET_TX_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, 
                                            UINT8 ac_type, UINT8 prio,
                                            UINT16 level, UINT8 enable)
{
	struct _tx_burst_cfg txop_cfg;
	UINT32 ret;

	if (wdev == NULL) {
		return;
	}

	txop_cfg.wdev = wdev;
	txop_cfg.ac_type = ac_type;
	txop_cfg.prio = prio;
	txop_cfg.txop_level = level;
	txop_cfg.enable = enable;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("<caller: %pS>\n -%s: enable=%x, ac_type=%x, prio=%x, txop=%x\n",
			 __builtin_return_address(0), __FUNCTION__,
			 txop_cfg.enable, txop_cfg.ac_type,
			 txop_cfg.prio, txop_cfg.txop_level));

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO, HWCMD_ID_SET_TX_BURST,
			sizeof(struct _tx_burst_cfg), (VOID *)&txop_cfg);
}

#ifdef MAC_REPEATER_SUPPORT	
VOID HW_ADD_REPT_ENTRY(
        PRTMP_ADAPTER pAd,
        struct wifi_dev *wdev,
        PUCHAR pAddr)
{
    UINT32 ret;
    ADD_REPT_ENTRY_STRUC add_rept_entry;
	APCLI_STRUCT *pApCliEntry = wdev->func_dev;

    os_zero_mem(&add_rept_entry, sizeof(ADD_REPT_ENTRY_STRUC));
    add_rept_entry.wdev = wdev;
    os_move_mem(add_rept_entry.arAddr, pAddr, MAC_ADDR_LEN);

    ret = HW_CTRL_BASIC_ENQ(pAd,
            HWCMD_TYPE_RADIO,
		HWCMD_ID_ADD_REPT_ENTRY,
		sizeof(ADD_REPT_ENTRY_STRUC),
		&add_rept_entry);

	if (ret == NDIS_STATUS_SUCCESS)
	{
		NdisAcquireSpinLock(&pAd->ApCfg.InsertReptCmdLock);
		pApCliEntry->InsRepCmdCount++;
		NdisReleaseSpinLock(&pAd->ApCfg.InsertReptCmdLock);
	}
	
}

VOID HW_REMOVE_REPT_ENTRY(
    PRTMP_ADAPTER pAd,
    UCHAR func_tb_idx,
    UCHAR CliIdx)
{
    UINT32 ret;
    REMOVE_REPT_ENTRY_STRUC remove_rept_entry;
    os_zero_mem(&remove_rept_entry, sizeof(REMOVE_REPT_ENTRY_STRUC));
    remove_rept_entry.func_tb_idx = func_tb_idx;
    remove_rept_entry.CliIdx = CliIdx;

    ret = HW_CTRL_BASIC_ENQ(pAd,
        HWCMD_TYPE_RADIO,
        HWCMD_ID_REMOVE_REPT_ENTRY,
        sizeof(REMOVE_REPT_ENTRY_STRUC),
        &remove_rept_entry);
}
#endif /* MAC_REPEATER_SUPPORT */


VOID HW_BECON_UPDATE(
	RTMP_ADAPTER *pAd,
	MT_UPDATE_BEACON rMtUpdateBeacon)
{
	UINT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd,
	    HWCMD_TYPE_RADIO,
	    HWCMD_ID_UPDATE_BEACON,
	    sizeof(MT_UPDATE_BEACON),
	    &rMtUpdateBeacon);
}

VOID RTMP_STA_ENTRY_ADD(PRTMP_ADAPTER pAd, ULONG Wcid,UCHAR *pAddr,BOOLEAN IsBMC, BOOLEAN IsReset)
{
	RT_SET_ASIC_WCID Info;
	UINT32 ret;

#ifdef FAST_EAPOL_WAR
	UINT32 wait_time = 2000;
	HW_CTRL_TXD HwCtrlTxd;
	MAC_TABLE_ENTRY *pEntry = NULL;
	pEntry = &pAd->MacTab.Content[Wcid];
#endif /* FAST_EAPOL_WAR */

	Info.WCID = Wcid;
	Info.IsBMC = IsBMC;
	Info.IsReset = IsReset;
	NdisMoveMemory(Info.Addr, pAddr, MAC_ADDR_LEN);

#ifdef FAST_EAPOL_WAR
	if (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_APCLI(pEntry))
	{
		HW_CTRL_TXD_BASIC(
			pAd,
			HWCMD_TYPE_RADIO,
			HWCMD_ID_SET_CLIENT_MAC_ENTRY,
			sizeof(RT_SET_ASIC_WCID),
			&Info,
			HwCtrlTxd
		);
		HW_CTRL_TXD_RSP(pAd,0,NULL,wait_time,HwCtrlTxd);	
		ret = HwCtrlEnqueueCmd(pAd,HwCtrlTxd);
	}
	else
#endif /* FAST_EAPOL_WAR */
	{
		ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_RADIO,HWCMD_ID_SET_CLIENT_MAC_ENTRY,sizeof(RT_SET_ASIC_WCID),&Info);
	}
	return;

}

#ifdef PKT_BUDGET_CTRL_SUPPORT
VOID HW_SET_PBC_CTRL(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UCHAR type)
{
	struct pbc_ctrl pbc;
	UINT32 ret;

	pbc.entry = entry;
	pbc.wdev = wdev;
	pbc.type = type;
	
	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_WMM, HWCMD_ID_PBC_CTRL,
			sizeof(struct pbc_ctrl), (VOID *)&pbc);
}
#endif

/*
 * set RTS Threshold per wdev
 */
VOID HW_SET_RTS_THLD(
		struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev,
		UCHAR pkt_num, UINT32 length)
{
	struct rts_thld rts;
	UINT32 ret;

	rts.wdev = wdev;
	rts.pkt_thld = pkt_num;
	rts.len_thld = length;

	ret = HW_CTRL_BASIC_ENQ(pAd, HWCMD_TYPE_PROTECT,
			HWCMD_ID_RTS_THLD, sizeof(rts), (VOID *)&rts);
}


#ifdef TXBF_SUPPORT
/*
*
*/
VOID HW_APCLI_BF_CAP_CONFIG(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pMacEntry)
{
	INT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, 
	                        HWCMD_TYPE_RADIO,
	                        HWCMD_ID_SET_APCLI_BF_CAP,
	                        sizeof(MAC_TABLE_ENTRY),
	                        pMacEntry);
}

/*
*
*/
VOID HW_APCLI_BF_REPEATER_CONFIG(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pMacEntry)
{
	INT32 ret;
	ret = HW_CTRL_BASIC_ENQ(pAd, 
	                        HWCMD_TYPE_RADIO,
	                        HWCMD_ID_SET_APCLI_BF_REPEATER,
	                        sizeof(MAC_TABLE_ENTRY),
	                        pMacEntry);
}

VOID HW_STA_BF_SOUNDING_ADJUST(PRTMP_ADAPTER pAd, UCHAR connState,struct wifi_dev *wdev)
{
        INT32 ret;
	MT_STA_BF_ADJ rMtStaBfAdj;

	os_zero_mem(&rMtStaBfAdj, sizeof(MT_STA_BF_ADJ));

	rMtStaBfAdj.wdev = wdev;
	rMtStaBfAdj.ConnectionState = connState;

        ret = HW_CTRL_BASIC_ENQ(pAd, 
	                        HWCMD_TYPE_RADIO,
	                        HWCMD_ID_ADJUST_STA_BF_SOUNDING,
	                        sizeof(MT_STA_BF_ADJ),
	                        &rMtStaBfAdj);	
}

VOID HW_AP_TXBF_TX_APPLY(struct _RTMP_ADAPTER *pAd,UCHAR enable)
{
    INT32 ret;
    UCHAR BfApply;
    

    BfApply = enable;
    ret = HW_CTRL_BASIC_ENQ(pAd, 
	                        HWCMD_TYPE_RADIO,
	                        HWCMD_ID_TXBF_TX_APPLY_CTRL,
	                        sizeof(UCHAR),
	                        &BfApply);	    
}



#endif /* TXBF_SUPPORT */


/*WIFI_SYS related HwCtrl CMD*/
/*
*
*/
VOID HW_WIFISYS_OPEN(
    RTMP_ADAPTER *pAd,
    WIFI_SYS_CTRL wifi_sys_ctrl)
{
    UINT32 ret;
	UINT32 wait_time = 2000;
	HW_CTRL_TXD HwCtrlTxd;
	HW_CTRL_TXD_BASIC(
		pAd,
		HWCMD_TYPE_WIFISYS,
		HWCMD_ID_WIFISYS_OPEN,
		sizeof(WIFI_SYS_CTRL),
		&wifi_sys_ctrl,
		HwCtrlTxd
	);
	HW_CTRL_TXD_RSP(pAd,0,NULL,wait_time,HwCtrlTxd);	
	ret = HwCtrlEnqueueCmd(pAd,HwCtrlTxd);
}


/*
*
*/
VOID HW_WIFISYS_CLOSE(
    RTMP_ADAPTER *pAd,
    WIFI_SYS_CTRL wifi_sys_ctrl)
{
    UINT32 ret;
	UINT32 wait_time = 2000;
	HW_CTRL_TXD HwCtrlTxd;
	HW_CTRL_TXD_BASIC(
		pAd,
		HWCMD_TYPE_WIFISYS,
		HWCMD_ID_WIFISYS_CLOSE,
		sizeof(WIFI_SYS_CTRL),
		&wifi_sys_ctrl,HwCtrlTxd
	);
	HW_CTRL_TXD_RSP(pAd,0,NULL,wait_time,HwCtrlTxd);	
	ret = HwCtrlEnqueueCmd(pAd,HwCtrlTxd);
}


/*
*
*/
VOID HW_WIFISYS_LINKDOWN(
    RTMP_ADAPTER *pAd,
    WIFI_SYS_CTRL wifi_sys_ctrl)
{
    UINT32 ret;
    ret = HW_CTRL_BASIC_ENQ(pAd,
        HWCMD_TYPE_WIFISYS,
        HWCMD_ID_WIFISYS_LINKDOWN,
        sizeof(WIFI_SYS_CTRL),
        &wifi_sys_ctrl);
}


/*
*
*/
VOID HW_WIFISYS_LINKUP(
    RTMP_ADAPTER *pAd,
    WIFI_SYS_CTRL wifi_sys_ctrl)
{
    UINT32 ret;
    ret = HW_CTRL_BASIC_ENQ(pAd,
        HWCMD_TYPE_WIFISYS,
        HWCMD_ID_WIFISYS_LINKUP,
        sizeof(WIFI_SYS_CTRL),
        &wifi_sys_ctrl);
}


/*
*
*/
VOID HW_WIFISYS_PEER_LINKUP(
    RTMP_ADAPTER *pAd,
    WIFI_SYS_CTRL wifi_sys_ctrl)
{
    UINT32 ret;
    ret = HW_CTRL_BASIC_ENQ(pAd,
        HWCMD_TYPE_WIFISYS,
        HWCMD_ID_WIFISYS_PEER_LINKUP,
        sizeof(WIFI_SYS_CTRL),
        &wifi_sys_ctrl);
}


/*
*
*/
VOID HW_WIFISYS_PEER_LINKDOWN(
    RTMP_ADAPTER *pAd,
    WIFI_SYS_CTRL wifi_sys_ctrl)
{
    UINT32 ret;
    ret = HW_CTRL_BASIC_ENQ(pAd,
        HWCMD_TYPE_WIFISYS,
        HWCMD_ID_WIFISYS_PEER_LINKDOWN,
        sizeof(WIFI_SYS_CTRL),
        &wifi_sys_ctrl);
}


/*
*
*/
VOID HW_WIFISYS_PEER_UPDATE(
    RTMP_ADAPTER *pAd,
    WIFI_SYS_CTRL wifi_sys_ctrl)
{
	UINT32 ret;

#ifdef FAST_EAPOL_WAR
	UINT32 wait_time = 2000;
	HW_CTRL_TXD HwCtrlTxd;
#endif /* FAST_EAPOL_WAR */

#ifdef FAST_EAPOL_WAR
	HW_CTRL_TXD_BASIC(
		pAd,
		HWCMD_TYPE_WIFISYS,
		HWCMD_ID_WIFISYS_PEER_UPDATE,
		sizeof(WIFI_SYS_CTRL),
		&wifi_sys_ctrl,
		HwCtrlTxd);

	HW_CTRL_TXD_RSP(pAd,0,NULL,wait_time,HwCtrlTxd);
	ret = HwCtrlEnqueueCmd(pAd,HwCtrlTxd);
#else /* FAST_EAPOL_WAR */
	ret = HW_CTRL_BASIC_ENQ(pAd,
		HWCMD_TYPE_WIFISYS,
		HWCMD_ID_WIFISYS_PEER_UPDATE,
		sizeof(WIFI_SYS_CTRL),
		&wifi_sys_ctrl);
#endif /* !FAST_EAPOL_WAR */

}

VOID HW_GET_TX_STATISTIC(
	RTMP_ADAPTER *pAd, 
	UINT32 Field,
    UINT8 Wcid)
{
	UINT32 ret;
	HW_CTRL_TXD HwCtrlTxd;
	TX_STAT_STRUC TxStat;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): Field = 0x%x, Wcid = %d\n", 
		__FUNCTION__, Field, Wcid));

	os_zero_mem(&HwCtrlTxd,sizeof(HW_CTRL_TXD));
	os_zero_mem(&TxStat,sizeof(TX_STAT_STRUC));

	TxStat.Field = Field;
	TxStat.Wcid = Wcid;

	ret = HW_CTRL_BASIC_ENQ(pAd,HWCMD_TYPE_WIFISYS, HWCMD_ID_GET_TX_STATISTIC, sizeof(TX_STAT_STRUC), (VOID *)&TxStat);
}

