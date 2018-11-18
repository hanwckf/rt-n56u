/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_asic.tmh"
#endif
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#include "mcu/mt_cmd.h"
#endif
#include "hdev/hdev.h"
#define MCAST_WCID_TO_REMOVE 0 //Pat: TODO
#define BCAST_WCID         127


static char *hif_2_str[]={"HIF_RTMP", "HIF_RLT", "HIF_MT", "Unknown"};
VOID AsicNotSupportFunc(RTMP_ADAPTER *pAd, const RTMP_STRING *caller)
{
	RTMP_STRING *str;

	if (pAd->chipCap.hif_type <= HIF_MAX)
		str = hif_2_str[pAd->chipCap.hif_type];
	else
		str = hif_2_str[HIF_MAX];

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NotSupportedFunc for this arch(%s)!\n",
				caller, str));
}

#ifndef	COMPOS_TESTMODE_WIN
UINT32 AsicGetCrcErrCnt(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicGetCrcErrCnt(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicGetCrcErrCnt(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return 0;
}


UINT32 AsicGetCCACnt(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicGetCCACnt(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicGetCCACnt(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return 0;
}


UINT32 AsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR ch_idx)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicGetChBusyCnt(pAd, ch_idx);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicGetChBusyCnt(pAd, ch_idx);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return 0;
}




INT AsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetAutoFallBack(pAd, enable);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetAutoFallBack(pAd, enable);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicAutoFallbackInit(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicAutoFallbackInit(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


VOID AsicUpdateRtsThld(
        struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
        UCHAR pkt_num,
        UINT32 length)
{
	if (pAd->chipCap.hif_type == HIF_MT)
	{
#ifdef CONFIG_ATE
		if (ATE_ON(pAd))
		{
			return;
		}
#endif /* CONFIG_ATE */
		if (pAd->archOps.archUpdateRtsThld)
			return pAd->archOps.archUpdateRtsThld(pAd, wdev, pkt_num, length);
	}

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


/*
	========================================================================

	Routine Description:
		Set MAC register value according operation mode.
		OperationMode AND bNonGFExist are for MM and GF Proteciton.
		If MM or GF mask is not set, those passing argument doesn't not take effect.

		Operation mode meaning:
		= 0 : Pure HT, no preotection.
		= 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
		= 0x10: No Transmission in 40M is protected.
		= 0x11: Transmission in both 40M and 20M shall be protected
		if (bNonGFExist)
			we should choose not to use GF. But still set correct ASIC registers.
	========================================================================
*/
VOID AsicUpdateProtect(
	IN PRTMP_ADAPTER pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN BOOLEAN bDisableBGProtect,
	IN BOOLEAN bNonGFExist)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicUpdateProtect(pAd, OperationMode, SetMask, bDisableBGProtect, bNonGFExist);

        return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
#ifdef CONFIG_ATE
		if (ATE_ON(pAd))
		{
			return;
		}
#endif /* CONFIG_ATE */
        HwCtrlSetFlag(pAd, HWFLAG_ID_UPDATE_PROTECT);

        return ;
    }
#endif


}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR Channel, BOOLEAN bScan)
{
	UCHAR bw;
	UCHAR PrimChannel;

	if(Channel >14)
	{
		PrimChannel = HcGetChannelByRf(pAd,RFIC_5GHZ);
	}
	else
	{
		PrimChannel = HcGetChannelByRf(pAd,RFIC_24GHZ);
	}

	if (IsHcRadioCurStatOffByChannel(pAd, Channel))
	    return;

#ifdef MT_WOW_SUPPORT
	if (pAd->WOW_Cfg.bWoWRunning){
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] WoW is running, skip!\n", __func__));
		return;
	}
#endif

	/*check is in the same band*/
	if((PrimChannel > 14 && Channel  <=14) ||
		(PrimChannel <=14 && Channel > 14)
	 )
	{
		UCHAR PhyMode;
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_DUAL_BAND);
		if (WMODE_CAP_5G(PhyMode) && (Channel > 14)) {
			//support
		} else if (WMODE_CAP_2G(PhyMode) && (Channel <= 14)) {
			//support
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Channel is not match!\n", __FUNCTION__));
			return;
		}
	}

	if(bScan)
		bw = BW_20;
	else
		bw = decide_phy_bw_by_channel(pAd,Channel);

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		RtAsicSwitchChannel(pAd, Channel, bScan);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_SWITCH_CHANNEL_CFG SwChCfg;
		os_zero_mem(&SwChCfg,sizeof(MT_SWITCH_CHANNEL_CFG));
		SwChCfg.bScan = bScan;
		SwChCfg.CentralChannel= Channel;
		SwChCfg.BandIdx = HcGetBandByChannel(pAd, Channel);

		if (pAd->CommonCfg.dbdc_mode)
		{
			if (SwChCfg.BandIdx == DBDC_BAND0) {
				SwChCfg.RxStream = pAd->dbdc_2G_rx_stream;
				SwChCfg.TxStream = pAd->dbdc_2G_tx_stream;
			} else {
				SwChCfg.RxStream = pAd->dbdc_5G_rx_stream;
				SwChCfg.TxStream = pAd->dbdc_5G_tx_stream;
			}
		} else {
			SwChCfg.RxStream = pAd->Antenna.field.RxPath;
			SwChCfg.TxStream = pAd->Antenna.field.TxPath;
		}

		SwChCfg.Bw = bw;
		SwChCfg.ControlChannel = (SwChCfg.Bw == BW_20) ? Channel : PrimChannel;
#ifdef DOT11_VHT_AC
		SwChCfg.ControlChannel2 = (SwChCfg.Bw == BW_8080)?pAd->CommonCfg.vht_cent_ch2 : 0;
#endif /* DOT11_VHT_AC */
#ifdef MT_DFS_SUPPORT
		SwChCfg.bDfsCheck = DfsSwitchCheck(pAd, SwChCfg.ControlChannel);
#endif
		HcSuspendMSDUTxByChannel(pAd, SwChCfg.ControlChannel);

		/*update radio info to band*/
        if(!bScan){
			UCHAR ext_cha;
			if(SwChCfg.ControlChannel == Channel)
				ext_cha = EXTCHA_NONE;
			else if(SwChCfg.ControlChannel > Channel)
				ext_cha = EXTCHA_BELOW;
			else
				ext_cha = EXTCHA_ABOVE;
            HcUpdateRadio(pAd,SwChCfg.Bw,SwChCfg.CentralChannel,SwChCfg.ControlChannel2);
			HcUpdateExtCha(pAd,Channel,ext_cha);
			pAd->CommonCfg.BBPCurrentBW = bw;

#ifdef WH_EZ_SETUP
			if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
				EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Band(%x) Switch to BW : %x CentralChannel:%d, ControlChannel: %d, ControlChannel2: %d\n",
					__FUNCTION__,SwChCfg.BandIdx,bw,SwChCfg.CentralChannel,SwChCfg.ControlChannel,SwChCfg.ControlChannel2));
#endif

        }

		MtAsicSwitchChannel(pAd,SwChCfg);

		HcUpdateMSDUTxAllowByChannel(pAd, SwChCfg.ControlChannel);

#if defined(MT_MAC) && defined(TXBF_SUPPORT) && defined(TXBF_BY_CHANNEL)
		SwitchBfByChannel(pAd, SwChCfg.CentralChannel);
#endif /* MT_MAC && TXBF_SUPPORT && TXBF_BY_CHANNEL */
	}
#endif

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	if (bScan)
	{
		bw = BW_20;
	}
	else
	{
		bw = pAd->CommonCfg.BBPCurrentBW;
	}
	RTMPSetAGCInitValue(pAd, bw);
}


/*
	==========================================================================
	Description:
		This function is required for 2421 only, and should not be used during
		site survey. It's only required after NIC decided to stay at a channel
		for a longer period.
		When this function is called, it's always after AsicSwitchChannel().

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicLockChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#ifdef ANT_DIVERSITY_SUPPORT
VOID AsicAntennaSelect(RTMP_ADAPTER *pAd, UCHAR Channel)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicAntennaSelect(pAd, Channel);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicAntennaSelect(pAd, Channel);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* ANT_DIVERSITY_SUPPORT */


VOID AsicResetBBPAgent(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicResetBBPAgent(pAd);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicResetBBPAgent(pAd);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}





#endif/*COMPOS_TESTMODE_WIN*/

// Replaced by AsicDevInfoUpdate()
INT AsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set OwnMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, PRINT_MAC(addr)));

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetDevMac(pAd, addr, omac_idx);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetDevMac(pAd, omac_idx, addr, 0, TRUE, DEVINFO_ACTIVE_FEATURE);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}

#ifndef	COMPOS_TESTMODE_WIN
#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfBcns)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetMbssMode(pAd, NumOfBcns);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetMbssMode(pAd, NumOfBcns);
		return;
	}
#endif
	return;
}
#endif /* CONFIG_AP_SUPPORT */


INT AsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev)
{
    if (pAd->archOps.archDisableBeacon)
        return pAd->archOps.archDisableBeacon(pAd, wdev);
    else {
        AsicNotSupportFunc(pAd, __FUNCTION__);
        return FALSE;
    }
    return FALSE;
}

INT AsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev, UCHAR NumOfBcns)
{
    if (pAd->archOps.archEnableBeacon)
        return pAd->archOps.archEnableBeacon(pAd, wdev, NumOfBcns);
    else {
        AsicNotSupportFunc(pAd, __FUNCTION__);
        return FALSE;
    }
}

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT AsicSetReptFuncEnable(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	if (pAd->archOps.archSetReptFuncEnable) {
        if (enable)
        {
            RepeaterCtrlInit(pAd);
        }
        else
        {
            RepeaterCtrlExit(pAd);
        }

		return pAd->archOps.archSetReptFuncEnable(pAd, enable);
    }
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;
	}
}


VOID AsicInsertRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *pAddr)
{
	if (pAd->archOps.archInsertRepeaterEntry)
		pAd->archOps.archInsertRepeaterEntry(pAd, CliIdx, pAddr);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
	}
}


VOID AsicRemoveRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
	if (pAd->archOps.archRemoveRepeaterEntry)
		pAd->archOps.archRemoveRepeaterEntry(pAd, CliIdx);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
	}
}


VOID AsicInsertRepeaterRootEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	if (pAd->archOps.archInsertRepeaterRootEntry)
		pAd->archOps.archInsertRepeaterRootEntry(pAd, Wcid, pAddr, ReptCliIdx);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
	}
}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


INT AsicSetRxFilter(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetRxFilter(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_RX_FILTER_CTRL_T RxFilter;

		os_zero_mem(&RxFilter,sizeof(MT_RX_FILTER_CTRL_T));
		if(FALSE)
		{
			RxFilter.bPromiscuous = TRUE;
		}else
		{
			RxFilter.bPromiscuous = FALSE;
			RxFilter.bFrameReport= FALSE;
			RxFilter.filterMask = RX_NDPA | RX_NOT_OWN_BTIM | RX_NOT_OWN_UCAST |
							 RX_RTS | RX_CTS | RX_CTRL_RSV | RX_BC_MC_DIFF_BSSID_A2 |
							 RX_BC_MC_DIFF_BSSID_A3 | RX_BC_MC_OWN_MAC_A3 | RX_PROTOCOL_VERSION |
							 RX_FCS_ERROR;

		}/*Endof Monitor ON*/

		if (pAd->archOps.archSetRxFilter){
			INT ret = 0;
			ret = pAd->archOps.archSetRxFilter(pAd,RxFilter);
			return ret;
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return FALSE;
		}

	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(RTMP_ADAPTER *pAd,
        UCHAR wlan_idx,
        UCHAR band_idx,
        UCHAR init,
        UCHAR resp)
{
	INT ret = FALSE;
	INT bSupport = FALSE;
    BOOLEAN is_en;

    is_en = (init && resp) ? TRUE : FALSE;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP
            || pAd->chipCap.hif_type == HIF_RLT)
	{
		ret = RtAsicSetRDG(pAd, is_en);
		bSupport = TRUE;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
    {
        MT_RDG_CTRL_T   rdg;
        RTMP_ARCH_OP    *arch_op = &pAd->archOps; 
        RTMP_CHIP_CAP   *chip_cap = &pAd->chipCap;

        if (arch_op->archSetRDG)
        {
            bSupport = TRUE;

            rdg.WlanIdx = wlan_idx;
            rdg.BandIdx = band_idx;
            rdg.Init = init;
            rdg.Resp = resp;
            if (init) {
                rdg.Txop = 0x80;
                chip_cap->CurrentTxOP = rdg.Txop;
                rdg.LongNav = 1;
            }
            else {
                rdg.Txop = chip_cap->CurrentTxOP;
                rdg.LongNav = 0;
            }

            ret = arch_op->archSetRDG(pAd, &rdg);
        }
    }
#endif

	if (ret == TRUE)
	{
		if (is_en)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		}
		else
		{
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		}
	}

	//if (!bSupport)
		//AsicNotSupportFunc(pAd, __FUNCTION__);

	return ret;
}
#endif /* DOT11_N_SUPPORT */


/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter

    Arguments:
        pAd         - Adapter pointer
        bPiggyBack  - Enable / Disable Piggy-Back

    Return Value:
        None

    ========================================================================
*/
VOID AsicSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetPiggyBack(pAd, bPiggyBack);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetPiggyBack(pAd, bPiggyBack);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}

INT AsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx)
{
    if (pAd->archOps.archSetPreTbtt)
    {
        pAd->archOps.archSetPreTbtt(pAd, enable, HwBssidIdx);
        return TRUE;
    }
    else {
        AsicNotSupportFunc(pAd, __FUNCTION__);
        return FALSE;
    }
    return FALSE;
}

INT AsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetGPTimer(pAd, enable, timeout);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetGPTimer(pAd, enable, timeout);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		return RtAsicSetChBusyStat(pAd, enable);
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		return MtAsicSetChBusyStat(pAd, enable);
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


#ifdef LINUX
#ifdef RTMP_WLAN_HOOK_SUPPORT
EXPORT_SYMBOL(AsicGetTsfTime);
#endif /* RTMP_WLAN_HOOK_SUPPORT */
#endif /* LINUX */
INT AsicGetTsfTime(
        RTMP_ADAPTER *pAd,
        UINT32 *high_part,
        UINT32 *low_part,
        UCHAR HwBssidIdx)
{
	if (pAd->archOps.archGetTsfTime)
		return pAd->archOps.archGetTsfTime(pAd, high_part, low_part, HwBssidIdx);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;
	}
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicDisableSync(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	if (pAd->archOps.archDisableSync)
		pAd->archOps.archDisableSync(pAd, HWBssidIdx);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
	}
    return;
}

VOID AsicSetSyncModeAndEnable(
        struct _RTMP_ADAPTER *pAd,
        USHORT BeaconPeriod,
        UCHAR HWBssidIdx,
        UCHAR OPMode)
{
    if (pAd->archOps.archSetSyncModeAndEnable)
        pAd->archOps.archSetSyncModeAndEnable(pAd, BeaconPeriod, HWBssidIdx, OPMode);
    else {
        AsicNotSupportFunc(pAd, __FUNCTION__);
    }
    return;
}

VOID AsicDisableBcnSntReq(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wifiDev)
{
	if (pAd->archOps.archDisableBcnSntReq)
		pAd->archOps.archDisableBcnSntReq(pAd, wifiDev);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
	}
	return;
}

VOID AsicEnableBcnSntReq(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wifiDev)
{
	if (pAd->archOps.archEnableBcnSntReq)
		pAd->archOps.archEnableBcnSntReq(pAd, wifiDev);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
	}
	return;
}





INT AsicSetWmmParam(RTMP_ADAPTER *pAd,UCHAR idx,UINT32 ac, UINT32 type, UINT32 val)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetWmmParam(pAd, ac, type, val);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetWmmParam(pAd,idx, ac, type, val);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return NDIS_STATUS_FAILURE;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm, struct wifi_dev *wdev)
{
	INT i;

	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NoEDCAParam\n", __FUNCTION__));

		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
		{
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) ||
                IS_ENTRY_APCLI(&pAd->MacTab.Content[i]) ||
                IS_ENTRY_REPEATER(&pAd->MacTab.Content[i]))
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);

		if (!ADHOC_ON(pAd))
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM, WMM Set: %d, BandIdx: %d\n",
									pEdcaParm->EdcaUpdateCount,
									pEdcaParm->WmmSet,
									pEdcaParm->BandIdx));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[0],
									 pEdcaParm->Cwmin[0],
									 pEdcaParm->Cwmax[0],
									 pEdcaParm->Txop[0]<<5,
									 pEdcaParm->bACM[0]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[1],
									 pEdcaParm->Cwmin[1],
									 pEdcaParm->Cwmax[1],
									 pEdcaParm->Txop[1]<<5,
									 pEdcaParm->bACM[1]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[2],
									 pEdcaParm->Cwmin[2],
									 pEdcaParm->Cwmax[2],
									 pEdcaParm->Txop[2]<<5,
									 pEdcaParm->bACM[2]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[3],
									 pEdcaParm->Cwmin[3],
									 pEdcaParm->Cwmax[3],
									 pEdcaParm->Txop[3]<<5,
									 pEdcaParm->bACM[3]));
		}
#ifdef APCLI_CERT_SUPPORT
		ApCliCertEDCAAdjust(pAd,wdev,pEdcaParm);
#endif

	}

#ifdef VOW_SUPPORT
    vow_update_om_wmm(pAd, wdev, pEdcaParm);
#endif /* VOW_SUPPORT */

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetEdcaParm(pAd, pEdcaParm);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetEdcaParm(pAd, pEdcaParm);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


VOID AsicTxCntUpdate(RTMP_ADAPTER *pAd, UCHAR Wcid, MT_TX_COUNTER *pTxInfo)
{

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT32 TxSuccess=0;

		MtAsicTxCntUpdate(pAd, Wcid,pTxInfo);

		TxSuccess = pTxInfo->TxCount -pTxInfo->TxFailCount;
		if ( pTxInfo->TxFailCount == 0 )
		{
			pAd->RalinkCounters.OneSecTxNoRetryOkCount += pTxInfo->TxCount;
		}
		else
		{
			pAd->RalinkCounters.OneSecTxRetryOkCount += pTxInfo->TxCount;
		}

		pAd->RalinkCounters.OneSecTxFailCount += pTxInfo->TxFailCount;

#ifdef STATS_COUNT_SUPPORT
		pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart += TxSuccess;
		pAd->WlanCounters[0].FailedCount.u.LowPart += pTxInfo->TxFailCount;
#endif /* STATS_COUNT_SUPPORT */
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

}


VOID AsicRssiUpdate(RTMP_ADAPTER *pAd)
{
	MAC_TABLE_ENTRY *pEntry;
	CHAR RssiSet[3];
	INT i=0;

	NdisZeroMemory(RssiSet,sizeof(RssiSet));

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ( pAd->MacTab.Size == 0 )
		{
			pEntry = &pAd->MacTab.Content[BCAST_WCID];
			MtAsicRssiGet(pAd,pEntry->wcid,&RssiSet[0]);

			for(i=0;i<3;i++)
			{
				pEntry->RssiSample.AvgRssi[i] = RssiSet[i];
				pEntry->RssiSample.LastRssi[i] = RssiSet[i];
				pAd->ApCfg.RssiSample.AvgRssi[i] = RssiSet[i];
				pAd->ApCfg.RssiSample.LastRssi[i] = RssiSet[i];
			}
		}
		else
		{
			INT32 TotalRssi[3];
			INT j;

			NdisZeroMemory(TotalRssi,sizeof(TotalRssi));

			for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
			{
				pEntry = &pAd->MacTab.Content[i];
				if (IS_VALID_ENTRY(pEntry))
				{
					MtAsicRssiGet(pAd,pEntry->wcid,&RssiSet[0]);

					for(j=0;j<3;j++)
					{
						pEntry->RssiSample.AvgRssi[j] = RssiSet[j];
						pEntry->RssiSample.LastRssi[j] = RssiSet[j];
						TotalRssi[j] += RssiSet[j];
					}
				}
			}

			for(i=0;i<3;i++)
			{
				pAd->ApCfg.RssiSample.AvgRssi[i] = pAd->ApCfg.RssiSample.LastRssi[i] = TotalRssi[i] / pAd->MacTab.Size;
			}
		}

	}
#endif /* CONFIG_AP_SUPPORT */

		}
	return ;
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}

INT AsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetRetryLimit(pAd, type, limit);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetRetryLimit(pAd, type, limit);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


UINT32 AsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicGetRetryLimit(pAd, type);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicGetRetryLimit(pAd, type);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime,
	IN UCHAR channel,
	IN struct wifi_dev *wdev)
{
	UINT32 SlotTime = 0;
	UINT32 SifsTime = SIFS_TIME_24G;
	UCHAR BandIdx;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
    	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
    		return;
    	else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
    		return;

    	if (bUseShortSlotTime)
    		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
    	else
    		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

    	SlotTime = (bUseShortSlotTime)? 9 : 20;
    }
#endif


#if defined(RTMP_MAC) || defined(RLT_MAC)
	if ((pAd->chipCap.hif_type == HIF_RTMP) ||(pAd->chipCap.hif_type == HIF_RLT))
	{
		if (channel > 14)
		{
			SifsTime = SIFS_TIME_5G;
		}
	}
#endif


	BandIdx = HcGetBandByChannel(pAd,channel);
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetSlotTime(pAd, SlotTime, SifsTime,BandIdx);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


INT AsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetMacMaxLen(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT){
		if (pAd->archOps.archSetMacMaxLen){
			INT ret = 0;
			ret = pAd->archOps.archSetMacMaxLen(pAd);
			return ret;
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return FALSE;
		}
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


VOID AsicGetTxTsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
#ifdef CONFIG_AP_SUPPORT
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicGetTxTsc(pAd, wdev, pTxTsc);
		return;
	}
#endif
#endif /* CONFIG_AP_SUPPORT */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		if (pAd->archOps.archGetTxTsc)
			pAd->archOps.archGetTxTsc(pAd, wdev, pTxTsc);

		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}

VOID AsicSetSMPS(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		if (pAd->archOps.archSetSMPS)
			pAd->archOps.archSetSMPS(pAd, Wcid, smps);
		else {
			AsicNotSupportFunc(pAd, __FUNCTION__);
		}

		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return;
}



/*
	========================================================================
	Description:
		Add Shared key information into ASIC.
		Update shared key, TxMic and RxMic to Asic Shared key table
		Update its cipherAlg to Asic Shared key Mode.

    Return:
	========================================================================
*/
VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicAddSharedKeyEntry(pAd, BssIndex, KeyIdx, pCipherKey);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicAddSharedKeyEntry(pAd, BssIndex, KeyIdx, pCipherKey);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


/*	IRQL = DISPATCH_LEVEL*/
VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicRemoveSharedKeyEntry(pAd, BssIndex, KeyIdx);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicRemoveSharedKeyEntry(pAd, BssIndex, KeyIdx);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicUpdateWCIDIVEIV(pAd, WCID, uIV, uEIV);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicUpdateWCIDIVEIV(pAd, WCID, uIV, uEIV);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


#ifdef MCS_LUT_SUPPORT
VOID AsicMcsLutUpdate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicMcsLutUpdate(pAd, pEntry);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* MCS_LUT_SUPPORT */


UINT16 AsicGetTidSn(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
    {
        struct _STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];

        return tr_entry->TxSeq[tid];
    }
#endif
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
    {
		if (pAd->archOps.archGetTidSn)
		{
			return pAd->archOps.archGetTidSn(pAd, wcid, tid);
        }
    }
#endif

    AsicNotSupportFunc(pAd, __FUNCTION__);

    return 0xffff;
}


VOID AsicUpdateBASession(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicUpdateBASession(pAd, wcid, tid, basize, isAdd, ses_type);
		return;
	}
#endif

#ifdef MT_MAC
    RETURN_IF_PAD_NULL(pAd);
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MAC_TABLE_ENTRY *mac_entry;
		MT_BA_CTRL_T BaCtrl;
		STA_REC_BA_CFG_T StaRecBaCfg;
		VOID *pBaEntry;
		UINT32 i;

		os_zero_mem(&BaCtrl,sizeof(MT_BA_CTRL_T));

		mac_entry = &pAd->MacTab.Content[wcid];

		BaCtrl.BaSessionType = ses_type;
		BaCtrl.BaWinSize = basize;
		BaCtrl.isAdd = isAdd;
		BaCtrl.Sn = sn;
		BaCtrl.Wcid = wcid;
		BaCtrl.Tid = tid;

#if defined(MT7615) || defined(MT7622)
		if (mac_entry && mac_entry->wdev) {
			BaCtrl.band_idx = HcGetBandByWdev(mac_entry->wdev);
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s(): mac_entry=%p!mac_entry->wdev=%p, Set BaCtrl.band_idx=%d\n",
					__FUNCTION__, mac_entry, mac_entry->wdev, BaCtrl.band_idx));
		}
		else
		{
			BaCtrl.band_idx = 0;
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s(): mac_entry=%p!Set BaCtrl.band_idx=%d\n",
					__FUNCTION__, mac_entry, BaCtrl.band_idx));
		}
#endif /* defined(MT7615) || defined(MT7622) */

		if (ses_type == BA_SESSION_RECP)
		{
	       		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
			if (isAdd)
			{
				os_move_mem(&BaCtrl.PeerAddr[0],&mac_entry->Addr[0],MAC_ADDR_LEN);
			}

		}

		if (pAd->archOps.archUpdateBASession)
		{
			pAd->archOps.archUpdateBASession(pAd, BaCtrl);

			if (pAd->archOps.archUpdateStaRecBa)
			{
				if(!mac_entry  || !mac_entry->wdev)
					return ;

				StaRecBaCfg.baDirection = ses_type;
				if(ses_type == ORI_BA)
				{
					i = mac_entry->BAOriWcidArray[tid];
					// TODO: Temporal modify for DBDC mode. Hugo should fix it.
					if(pAd->CommonCfg.dbdc_mode && !WMODE_CAP_AC(mac_entry->wdev->PhyMode))
					{
						pAd->BATable.BAOriEntry[i].amsdu_cap = FALSE;
					}
					pBaEntry = &pAd->BATable.BAOriEntry[i];
				}else
				{
					i = mac_entry->BARecWcidArray[tid];
					// TODO: Temporal modify for DBDC mode. Hugo should fix it.
					if(pAd->CommonCfg.dbdc_mode && !WMODE_CAP_AC(mac_entry->wdev->PhyMode))
					{
						pAd->BATable.BAOriEntry[i].amsdu_cap = FALSE;
					}
					pBaEntry = &pAd->BATable.BARecEntry[i];
				}


				StaRecBaCfg.BaEntry = pBaEntry;
				StaRecBaCfg.BssIdx = mac_entry->wdev->bss_info_argument.ucBssIndex;

                if (IS_ENTRY_REPEATER(mac_entry))
                    StaRecBaCfg.MuarIdx = pAd->MacTab.tr_entry[mac_entry->wcid].OmacIdx;
                else
                    StaRecBaCfg.MuarIdx = mac_entry->wdev->OmacIdx;

				StaRecBaCfg.tid = tid;
				StaRecBaCfg.BaEnable = (isAdd << tid);
				StaRecBaCfg.WlanIdx = wcid;
				pAd->archOps.archUpdateStaRecBa(pAd, StaRecBaCfg);
			}

			return;
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return;
		}
	}
#endif
	AsicNotSupportFunc(pAd, __FUNCTION__);
}

VOID AsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr, BOOLEAN IsBCMCWCID, BOOLEAN IsReset)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicUpdateRxWCIDTable(pAd, WCID, pAddr);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_WCID_TABLE_INFO_T WtblInfo;

		MAC_TABLE_ENTRY *mac_entry = NULL;
        struct _STA_TR_ENTRY *tr_entry = NULL;

		os_zero_mem(&WtblInfo,sizeof(MT_WCID_TABLE_INFO_T));
		WtblInfo.Wcid = WCID;
		WtblInfo.IsReset = IsReset;
		os_move_mem(&WtblInfo.Addr[0],&pAddr[0],6);

        if (VALID_UCAST_ENTRY_WCID(pAd, WCID))
            mac_entry = &pAd->MacTab.Content[WCID];

		if ((IsBCMCWCID == TRUE) || WCID == MAX_LEN_OF_MAC_TABLE)
		{
		    // BC Mgmt or BC/MC data
			WtblInfo.MacAddrIdx = 0xe;
			WtblInfo.WcidType = MT_WCID_TYPE_BMCAST;
			WtblInfo.CipherSuit = WTBL_CIPHER_NONE;

            if (HcGetWcidLinkType(pAd,WCID)== WDEV_TYPE_APCLI)
            {
                WtblInfo.WcidType = MT_WCID_TYPE_APCLI_MCAST;
            }
		}
		else if (mac_entry)
		{
			if (IS_ENTRY_CLIENT(mac_entry))
			{
			    /* FIXME: will fix this when set entry fix for sta mode */
			    if (mac_entry->wdev->wdev_type == WDEV_TYPE_AP)
			    {
				    WtblInfo.WcidType = MT_WCID_TYPE_CLI;
			    }
			    else if (mac_entry->wdev->wdev_type == WDEV_TYPE_STA)
			    {
				    WtblInfo.WcidType = MT_WCID_TYPE_AP;
			    }
			}else if (IS_ENTRY_APCLI(mac_entry))
				WtblInfo.WcidType = MT_WCID_TYPE_APCLI;
            else if (IS_ENTRY_REPEATER(mac_entry))
                WtblInfo.WcidType = MT_WCID_TYPE_REPEATER;
            else if (IS_ENTRY_WDS(mac_entry))
                WtblInfo.WcidType = MT_WCID_TYPE_WDS;
			else
				WtblInfo.WcidType = MT_WCID_TYPE_CLI;

            if (IS_ENTRY_REPEATER(mac_entry))
            {
                tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];
                WtblInfo.MacAddrIdx = tr_entry->OmacIdx;
            }
            else
            {
                WtblInfo.MacAddrIdx = mac_entry->wdev->OmacIdx;
            }

			WtblInfo.Aid = mac_entry->Aid;

#ifdef TXBF_SUPPORT
            WtblInfo.PfmuId    = pAd->rStaRecBf.u2PfmuId;

            if(IS_HT_STA(mac_entry))
            {
			    WtblInfo.fgTiBf    = (mac_entry->rStaRecBf.fgETxBfCap > 0) ? FALSE : TRUE;
			    WtblInfo.fgTiBf    = (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE) ? WtblInfo.fgTiBf : FALSE;
			    WtblInfo.fgTeBf    = mac_entry->rStaRecBf.fgETxBfCap;
			    WtblInfo.fgTeBf    = (pAd->CommonCfg.ETxBfEnCond == TRUE) ? WtblInfo.fgTeBf : FALSE;
            }

            if(IS_VHT_STA(mac_entry))
            {
			    WtblInfo.fgTibfVht = (mac_entry->rStaRecBf.fgETxBfCap > 0) ? FALSE : TRUE;
			    WtblInfo.fgTibfVht = (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE) ? WtblInfo.fgTibfVht : FALSE;
			    WtblInfo.fgTebfVht = mac_entry->rStaRecBf.fgETxBfCap;
			    WtblInfo.fgTebfVht = (pAd->CommonCfg.ETxBfEnCond == TRUE) ? WtblInfo.fgTebfVht : FALSE;
            }
#endif
            if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE)
                    && CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RALINK_CHIPSET))
            {
                WtblInfo.aad_om = 1;
            }

				if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_WMM_CAPABLE))
					WtblInfo.SupportQoS = TRUE;

				if(IS_HT_STA(mac_entry))
				{
					WtblInfo.SupportHT = TRUE;
					if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE))
					{
						WtblInfo.SupportRDG= TRUE;
					}
					WtblInfo.SmpsMode = mac_entry->MmpsMode ;
					WtblInfo.MpduDensity = mac_entry->MpduDensity;
					WtblInfo.MaxRAmpduFactor = mac_entry->MaxRAmpduFactor;
                    if (IS_VHT_STA(mac_entry))
                    {
                        WtblInfo.SupportVHT = TRUE;
#ifdef TXBF_SUPPORT
                        WtblInfo.gid = 63;
#endif
                    }
				}

			if (IS_CIPHER_TKIP_Entry(mac_entry))
			{
				WtblInfo.DisRHTR = 1;
#ifdef MWDS
				if(mac_entry->bEnableMWDS)
					WtblInfo.DisRHTR = 0;
#endif
			}
#ifdef MWDS
			WtblInfo.fgMwdsEnable = mac_entry->bEnableMWDS;
			if(mac_entry->bEnableMWDS)
			    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("AsicUpdateRxWCIDTable: Enable MWDS in WTBLinfo\n"));
#endif
		}
		else
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():mac_entry is NULL!\n", __FUNCTION__));
		}

		if (pAd->archOps.archUpdateRxWCIDTable)
		{
			return pAd->archOps.archUpdateRxWCIDTable(pAd, WtblInfo);
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return;
		}
	}
#endif
	AsicNotSupportFunc(pAd, __FUNCTION__);
}


#ifdef TXBF_SUPPORT
VOID AsicUpdateClientBfCap(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pMacEntry)
{
#ifdef MT_MAC
	if (pAd->archOps.archUpdateClientBfCap)
	{
		return pAd->archOps.archUpdateClientBfCap(pAd, pMacEntry);
	}
	else
	{
		AsicNotSupportFunc(pAd, __FUNCTION__);
		return;
	}
#endif
	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* TXBF_SUPPORT */


/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:

    Note :
		The key table selection rule :
    	1.	Wds-links and Mesh-links always use Pair-wise key table.
		2. 	When the CipherAlg is TKIP, AES, SMS4 or the dynamic WEP is enabled,
			it needs to set key into Pair-wise Key Table.
		3.	The pair-wise key security mode is set NONE, it means as no security.
		4.	In STA Adhoc mode, it always use shared key table.
		5.	Otherwise, use shared key table

	========================================================================
*/
VOID AsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicUpdateWcidAttributeEntry(pAd, BssIdx, KeyIdx, CipherAlg, Wcid, KeyTabFlag);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicUpdateWcidAttributeEntry(pAd, BssIdx, KeyIdx, CipherAlg, Wcid, KeyTabFlag);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR wcid_idx)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicDelWcidTab(pAd, wcid_idx);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		if (pAd->archOps.archDelWcidTab)
		{
			return pAd->archOps.archDelWcidTab(pAd, wcid_idx);
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return;
		}
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return;
}


#ifdef  HTC_DECRYPT_IOT
VOID AsicSetWcidAAD_OM(RTMP_ADAPTER *pAd, UCHAR wcid_idx , CHAR value)
{

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		if (pAd->archOps.archSetWcidAAD_OM)
		{
			return pAd->archOps.archSetWcidAAD_OM(pAd, wcid_idx, value);
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return;
		}
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return;
}
#endif /* HTC_DECRYPT_IOT */



VOID AsicAddRemoveKeyTab (
    IN PRTMP_ADAPTER pAd,
    IN ASIC_SEC_INFO *pInfo)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
    if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
    {
        if (pInfo->Operation == SEC_ASIC_ADD_PAIRWISE_KEY)
        {
            RtAsicAddPairwiseKeyEntry(pAd, pInfo);
        }
        else if (pInfo->Operation == SEC_ASIC_REMOVE_PAIRWISE_KEY)
        {
            RtAsicRemovePairwiseKeyEntry(pAd, pInfo);
        }
        else if (pInfo->Operation == SEC_ASIC_ADD_GROUP_KEY)
        {
            RtAsicAddSharedKeyEntry(pAd, pInfo);
        }
        else if (pInfo->Operation == SEC_ASIC_REMOVE_GROUP_KEY)
        {
            RtAsicRemoveSharedKeyEntry(pAd, pInfo);
        }
        return;
    }
#endif

#ifdef MT_MAC
    if ((pAd->chipCap.hif_type == HIF_MT)
		&& (pAd->archOps.archAddRemoveKeyTab))
    {
        return pAd->archOps.archAddRemoveKeyTab(pAd, pInfo);
    }
#endif

    AsicNotSupportFunc(pAd, __FUNCTION__);
}


INT AsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


BOOLEAN AsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSendCmdToMcuAndWait(pAd, Command, Token, Arg0, Arg1, in_atomic);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSendCmdToMcuAndWait(pAd, Command, Token, Arg0, Arg1, in_atomic);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


BOOLEAN AsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSendCommandToMcuBBP(pAd, Command, Token, Arg0, Arg1, FlgIsNeedLocked);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSendCommandToMcuBBP(pAd, Command, Token, Arg0, Arg1, FlgIsNeedLocked);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


/*
	========================================================================
	Description:
		For 1x1 chipset : 2070 / 3070 / 3090 / 3370 / 3390 / 5370 / 5390
		Usage :	1. Set Default Antenna as initialize
				2. Antenna Diversity switching used
				3. iwpriv command switch Antenna

    Return:
	========================================================================
 */
VOID AsicSetRxAnt(RTMP_ADAPTER *pAd,UCHAR Ant)
{
	if (pAd->chipOps.SetRxAnt)
		pAd->chipOps.SetRxAnt(pAd, Ant);
}


VOID AsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicTurnOffRFClk(pAd, Channel);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		if (pAd->chipOps.AsicRfTurnOff)
		{
			pAd->chipOps.AsicRfTurnOff(pAd);
		}
		else
		{
		MtAsicTurnOffRFClk(pAd, Channel);
		}
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


#ifdef WAPI_SUPPORT
VOID AsicUpdateWAPIPN(
	IN RTMP_ADAPTER *pAd,
	IN USHORT WCID,
	IN ULONG pn_low,
	IN ULONG pn_high)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicUpdateWAPIPN(pAd, WCID, pn_low, pn_high);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicUpdateWAPIPN(pAd, WCID, pn_low, pn_high);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* WAPI_SUPPORT */




#ifdef STREAM_MODE_SUPPORT
// StreamModeRegVal - return MAC reg value for StreamMode setting
UINT32 StreamModeRegVal(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtStreamModeRegVal(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtStreamModeRegVal(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


/*
	========================================================================
	Description:
		configure the stream mode of specific MAC or all MAC and set to ASIC.

	Prameters:
		pAd		 ---
		pMacAddr ---
		bClear	 --- disable the stream mode for specific macAddr when
						(pMacAddr!=NULL)

    Return:
	========================================================================
*/
VOID AsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetStreamMode(pAd, pMacAddr, chainIdx, bEnabled);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetStreamMode(pAd, pMacAddr, chainIdx, bEnabled);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


VOID AsicStreamModeInit(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicStreamModeInit(pAd);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicStreamModeInit(pAd);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif // STREAM_MODE_SUPPORT //


VOID AsicSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetTxPreamble(pAd, TxPreamble);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetTxPreamble(pAd, TxPreamble);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


#ifdef DOT11_N_SUPPORT
INT AsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicReadAggCnt(pAd, aggCnt, cnt_len);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicReadAggCnt(pAd, aggCnt, cnt_len);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetRalinkBurstMode(pAd, enable);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetRalinkBurstMode(pAd, enable);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}

INT AsicUpdateTxOP(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 TxOpVal)
{
    UINT32 last_txop_val;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{

	    if (pAd->CommonCfg.ManualTxop)
	    {
	        return TRUE;
	    }

	    last_txop_val = MtAsicGetWmmParam(pAd, AcNum, WMM_PARAM_TXOP);

	    if (last_txop_val == TxOpVal)
	    { /* No need to Update TxOP CR */
	        return TRUE;
	    }
	    else if (last_txop_val == 0xdeadbeef)
	    {
	        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error CR value for TxOP = 0x%08x\n", __FUNCTION__, last_txop_val));

	        return FALSE;
	    }
	    else {}

	    MtAsicSetWmmParam(pAd,0, AcNum, WMM_PARAM_TXOP, TxOpVal);

	    return TRUE;
	}
#endif /*MT_MAC*/

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return FALSE;

}


#endif // DOT11_N_SUPPORT //


INT AsicSetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel)
{
//TODO: Shiang-MT7615, remove this from here?? Do we still need it??
	AsicNotSupportFunc(pAd, __FUNCTION__);
	return FALSE;
}




#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.AsicMeasureFalseCCA)
		pAd->chipOps.AsicMeasureFalseCCA(pAd);
}

VOID AsicMitigateMicrowave(RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.AsicMitigateMicrowave)
		pAd->chipOps.AsicMitigateMicrowave(pAd);
}
#endif /* MICROWAVE_OVEN_SUPPORT */


VOID AsicBbpInitFromEEPROM(RTMP_ADAPTER *pAd)
{
	if(pAd->chipOps.BbpInitFromEEPROM)
		pAd->chipOps.BbpInitFromEEPROM(pAd);
}

INT AsicWaitMacTxRxIdle(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicWaitMacTxRxIdle(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		return MtAsicWaitMacTxRxIdle(pAd);
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetMacTxRx(RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetMacTxRx(pAd, txrx, enable);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		INT ret = 0;
		if (pAd->archOps.archSetMacTxRx)
		{
			if((ret = pAd->archOps.archSetMacTxRx(pAd, txrx, enable,0))!=0)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): SetMacTxRx failed!\n",__FUNCTION__));
				return ret;
			}

#ifdef DBDC_MODE
			if(pAd->CommonCfg.dbdc_mode)
			{
				if((ret = pAd->archOps.archSetMacTxRx(pAd, txrx, enable,1))!=0)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): SetMacTxRx failed!\n",__FUNCTION__));
					return ret;
				}
			}
#endif /*DBDC_MODE*/
			return ret;
		}
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetWPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetWPDMA(pAd, TxRx, enable);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetWPDMA(pAd, TxRx, enable,pAd->chipCap.WPDMABurstSIZE);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


BOOLEAN AsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicWaitPDMAIdle(pAd, round, wait_us);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicWaitPDMAIdle(pAd, round, wait_us);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


BOOLEAN AsicCheckDMAIdle(struct _RTMP_ADAPTER *pAd, UINT8 Dir)
{

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicCheckDMAIdle(pAd, Dir);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}



INT AsicSetMacWD(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetMacWD(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetMacWD(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicHIFInit(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT AsicTOPInit(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicTOPInit(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicTOPInit(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT StopDmaRx(RTMP_ADAPTER *pAd, UCHAR Level)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtStopDmaRx(pAd, Level);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtStopDmaRx(pAd, Level);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT StopDmaTx(RTMP_ADAPTER *pAd, UCHAR Level)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtStopDmaTx(pAd, Level);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtStopDmaTx(pAd, Level);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up, UCHAR BandIdx)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetTxStream(pAd, opmode, up);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT){
		if (pAd->archOps.archSetTxStream){
			INT Ret;
			Ret = pAd->archOps.archSetTxStream(pAd, pAd->Antenna.field.TxPath,0);
			return Ret;
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return FALSE;
		}
	}
#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetRxStream(pAd, rx_path);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT){
		if (pAd->archOps.archSetRxStream){
			INT Ret;
			Ret =pAd->archOps.archSetRxStream(pAd, rx_path,BandIdx);
			return Ret;
		}
		else
		{
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return FALSE;
		}
	}
#endif /* MT_MAC */

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetBW(RTMP_ADAPTER *pAd, INT bw,UCHAR BandIdx)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetBW(pAd, bw);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetBW(pAd, bw,BandIdx);
#endif /* MT_MAC */

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetCtrlCh(RTMP_ADAPTER *pAd, UINT8 extch)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
		return rtmp_mac_set_ctrlch(pAd, extch);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return mt_mac_set_ctrlch(pAd, extch);
#endif /* MT_MAC */

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetChannel(RTMP_ADAPTER *pAd, UCHAR ch, UINT8 bw, UINT8 ext_ch, BOOLEAN bScan)
{

	HcBbpSetBwByChannel(pAd, bw,ch);

	/*  Tx/RX : control channel setting */
	bbp_set_ctrlch(pAd, ext_ch);
	AsicSetCtrlCh(pAd, ext_ch);

	/* Let BBP register at 20MHz to do scan */
	AsicSwitchChannel(pAd, ch, bScan);
	AsicLockChannel(pAd, ch);

#ifdef RT28xx
	RT28xx_ch_tunning(pAd, bw);
#endif /* RT28xx */

	return 0;
}

VOID AsicSetTmrCR(RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
    if (pAd->archOps.archSetTmrCR)
    {
        pAd->archOps.archSetTmrCR(pAd, enable, BandIdx);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
    }
}

#ifdef MAC_APCLI_SUPPORT
/*
	==========================================================================
	Description:
		Set BSSID of Root AP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetApCliBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index)
{
	if (pAd->archOps.archSetApCliBssid)
		pAd->archOps.archSetApCliBssid(pAd, pBssid, index);
	else {
		AsicNotSupportFunc(pAd, __FUNCTION__);
	}
}
#endif /* MAC_APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetMbssWdevIfAddr(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode)
{
    if (pAd->archOps.archSetMbssWdevIfAddr)
    {
        pAd->archOps.archSetMbssWdevIfAddr(pAd, idx, if_addr, opmode);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
    }
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    if (pAd->archOps.archSetMbssHwCRSetting)
    {
        pAd->archOps.archSetMbssHwCRSetting(pAd, mbss_idx, enable);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
    }
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    if (pAd->archOps.archSetExtMbssEnableCR)
    {
        pAd->archOps.archSetExtMbssEnableCR(pAd, mbss_idx, enable);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
    }
}

VOID AsicSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    if (pAd->archOps.archSetExtTTTTHwCRSetting)
    {
        pAd->archOps.archSetExtTTTTHwCRSetting(pAd, mbss_idx, enable);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
    }
}
#endif /* CONFIG_AP_SUPPORT */


VOID AsicDMASchedulerInit(RTMP_ADAPTER *pAd, INT mode)
{

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_DMASCH_CTRL_T DmaSchCtrl;
		if (MTK_REV_GTE(pAd, MT7603, MT7603E1) && MTK_REV_LT(pAd, MT7603, MT7603E2))
		{
			DmaSchCtrl.bBeaconSpecificGroup = FALSE;
		}else
		{
			DmaSchCtrl.bBeaconSpecificGroup = TRUE;
		}
		DmaSchCtrl.mode = mode;
#ifdef DMA_SCH_SUPPORT
		MtAsicDMASchedulerInit(pAd, DmaSchCtrl);
#endif
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}

INT32 AsicDevInfoUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): Set OwnMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, PRINT_MAC(OwnMacAddr)));

#if defined(RTMP_MAC) || defined(RLT_MAC)
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
	if (pAd->archOps.archSetDevMac)
	{
		return pAd->archOps.archSetDevMac(pAd, OwnMacIdx, OwnMacAddr, BandIdx, Active, EnableFeature);
}
	else
{
	AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;
	}
}
#endif /* MT_MAC */

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}

INT32 AsicBssInfoUpdate(
    RTMP_ADAPTER *pAd,
    BSS_INFO_ARGUMENT_T bss_info_argument)
{
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                ("%s(): Set Bssid=%02x:%02x:%02x:%02x:%02x:%02x, BssIndex(%d)\n",
                __FUNCTION__,
                PRINT_MAC(bss_info_argument.Bssid),
                bss_info_argument.ucBssIndex));

    if (pAd->archOps.archSetBssid)
    {
        return pAd->archOps.archSetBssid(pAd, bss_info_argument);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
        return FALSE;
    }
}

INT32 AsicExtPwrMgtBitWifi(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit)
{
#ifdef STA_LP_PHASE_2_SUPPORT
#ifdef MT7615
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s::No need for 7615\n", __FUNCTION__));
	return 0;
#else
	MT_PWR_MGT_BIT_WIFI_T rPwtMgtBitWiFi = {0};

	rPwtMgtBitWiFi.ucWlanIdx = ucWlanIdx;
	rPwtMgtBitWiFi.ucPwrMgtBit = ucPwrMgtBit;

	return MtCmdExtPwrMgtBitWifi(pAd, rPwtMgtBitWiFi);
#endif /* MT7615 */
#else
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: This H/W does not support this\n", __FUNCTION__));
	return 0;
#endif /* STA_LP_PHASE_2_SUPPORT */
}

INT32 AsicStaRecUpdate(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 BssIndex,
	UINT8 WlanIdx,
	UINT32 ConnectionType,
	UINT8 ConnectionState,
	UINT32 EnableFeature,
	UINT8 IsNewSTARec)
{
	if (pAd->archOps.archSetStaRec)
	{
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;
		INT32 ret=0;

		os_zero_mem(&StaCfg,sizeof(STA_REC_CFG_T));
		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
		{
			pEntry = NULL;
		}
		else
		{
			pEntry	= &pAd->MacTab.Content[WlanIdx];
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s::Wcid(%d), u4EnableFeature(%d)\n",
			__FUNCTION__, WlanIdx, EnableFeature));

		if (pEntry && !IS_ENTRY_NONE(pEntry))
		{
			if (!pEntry->wdev)
			{
				ASSERT(pEntry->wdev);
				return -1;
			}

            if (IS_ENTRY_REPEATER(pEntry))
                StaCfg.MuarIdx = pAd->MacTab.tr_entry[pEntry->wcid].OmacIdx;
            else
                StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		}
		else
		{
			StaCfg.MuarIdx = 0xe;//TODO: Carter, check this on TX_HDR_TRANS
		}

#ifdef TXBF_SUPPORT
        if (pEntry && !IS_ENTRY_NONE(pEntry) && 
            (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_AP(pEntry)))
		{
		    if (HcIsBfCapSupport(pEntry->wdev) == TRUE)
		    {
		        if (EnableFeature & STA_REC_BF_FEATURE)
		        {
                    AsicBfStaRecUpdate(pAd, pEntry->wdev->PhyMode, BssIndex, WlanIdx);
		        }
		    }
        }
#endif /* TXBF_SUPPORT */

		StaCfg.ConnectionState = ConnectionState;
		StaCfg.ConnectionType = ConnectionType;
		StaCfg.u4EnableFeature = EnableFeature;
		StaCfg.ucBssIndex = BssIndex;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.wdev = wdev;
		StaCfg.pEntry = pEntry;
		StaCfg.IsNewSTARec = IsNewSTARec;

        /*tracking the starec input history*/
        if (pAd->StaRecTracking.RecordLoop >= MAX_STA_REC_HISTORY_RECORD)
        {
            pAd->StaRecTracking.RecordLoop = 0;
        }
        os_move_mem(&pAd->StaRecTracking.sta_rec_cfg_record[pAd->StaRecTracking.RecordLoop],
                    &StaCfg,
                    sizeof(STA_REC_CFG_T));
        pAd->StaRecTracking.StaRecNum++;
        pAd->StaRecTracking.RecordLoop++;

		ret = pAd->archOps.archSetStaRec(pAd,StaCfg);

		/*wait for result*/
		if(StaCfg.ConnectionState==STATE_DISCONNECT)
		{
			HcHwReleaseWcid(pAd,WlanIdx);
		}else
		{					
			HcHwAcquireWcid(pAd,WlanIdx);
		}

		return ret;
	}
	else
	{
		AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;
	}

	return 0;
}


#ifdef MT_MAC
INT32 AsicRaParamStaRecUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 WlanIdx,
    P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam,
	UINT32 EnableFeature)
{
	if (pAd->archOps.archSetStaRec)
	{
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;

		os_zero_mem(&StaCfg,sizeof(STA_REC_CFG_T));
		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
		{
			pEntry = NULL;
		}
		else
		{
			pEntry	= &pAd->MacTab.Content[WlanIdx];
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s::Wcid(%d), u4EnableFeature(%d)\n",
			__FUNCTION__, WlanIdx, EnableFeature));

		if (pEntry && !IS_ENTRY_NONE(pEntry))
		{
			if (!pEntry->wdev)
			{
				ASSERT(pEntry->wdev);
				return -1;
			}

			if (IS_ENTRY_REPEATER(pEntry))
			{
				StaCfg.MuarIdx = pAd->MacTab.tr_entry[pEntry->wcid].OmacIdx;
			}
			else
			{
				StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
			}

			StaCfg.ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
		}
		else
		{
			StaCfg.MuarIdx = 0xe;//TODO: Carter, check this on TX_HDR_TRANS
		}
		StaCfg.ConnectionState = STATE_CONNECTED;
		StaCfg.u4EnableFeature = EnableFeature;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.pRaParam = prParam;

		/*tracking the starec input history*/
		if (pAd->StaRecTracking.RecordLoop >= MAX_STA_REC_HISTORY_RECORD)
		{
			pAd->StaRecTracking.RecordLoop = 0;
		}
		os_move_mem(&pAd->StaRecTracking.sta_rec_cfg_record[pAd->StaRecTracking.RecordLoop],
				&StaCfg,
				sizeof(STA_REC_CFG_T));
		pAd->StaRecTracking.StaRecNum++;
		pAd->StaRecTracking.RecordLoop++;

		return pAd->archOps.archSetStaRec(pAd,StaCfg);
	}
	else
	{
		AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;
	}

	return 0;
}
#endif /* MT_MAC */

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT32 AsicBfStaRecUpdate(
	RTMP_ADAPTER *pAd,
    UCHAR        ucPhyMode,
    UCHAR        ucBssIdx,
	UCHAR        ucWlanIdx)
{
    if (pAd->chipOps.BfStaRecUpdate)
	{
	    return pAd->chipOps.BfStaRecUpdate(
	                                   pAd,
	                                   ucPhyMode,
	                                   ucBssIdx,
	                                   ucWlanIdx);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicBfStaRecRelease(
	RTMP_ADAPTER *pAd,
    UCHAR        ucBssIdx,
	UCHAR        ucWlanIdx)
{
    if (pAd->chipOps.BfStaRecRelease)
	{
	    return pAd->chipOps.BfStaRecRelease(
	                                   pAd,
	                                   ucBssIdx,
	                                   ucWlanIdx);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicBfPfmuMemAlloc(
	RTMP_ADAPTER *pAd,
    UCHAR ucSu_Mu,
    UCHAR ucWlanId)
{
    if (pAd->chipOps.BfPfmuMemAlloc)
	{
	    return pAd->chipOps.BfPfmuMemAlloc(
	                                   pAd,
	                                   ucSu_Mu,
	                                   ucWlanId);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicBfPfmuMemRelease(
	RTMP_ADAPTER *pAd,
    UCHAR ucWlanId)
{
    if (pAd->chipOps.BfPfmuMemRelease)
	{
	    return pAd->chipOps.BfPfmuMemRelease(
	                                   pAd,
	                                   ucWlanId);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicTxBfTxApplyCtrl(
	RTMP_ADAPTER *pAd,
    UCHAR   ucWlanId,
    BOOLEAN fgETxBf,
    BOOLEAN fgITxBf,
    BOOLEAN fgMuTxBf,
    BOOLEAN fgPhaseCali)
{
    if (pAd->chipOps.TxBfTxApplyCtrl)
	{
	    return pAd->chipOps.TxBfTxApplyCtrl(
	                                   pAd,
	                                   ucWlanId,
	                                   fgETxBf,
	                                   fgITxBf,
	                                   fgMuTxBf,
	                                   fgPhaseCali);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicTxBfeeHwCtrl(
	RTMP_ADAPTER *pAd,
    BOOLEAN   fgBfeeHwCtrl)
{
    if (pAd->chipOps.BfeeHwCtrl)
	{
	    return pAd->chipOps.BfeeHwCtrl(
	                                   pAd,
	                                   fgBfeeHwCtrl);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicTxBfApClientCluster(
	RTMP_ADAPTER *pAd,
    UCHAR   ucWlanId,
    UCHAR   ucCmmWlanId)
{
    if (pAd->chipOps.BfApClientCluster)
	{
	    return pAd->chipOps.BfApClientCluster(
	                                   pAd,
	                                   ucWlanId,
	                                   ucCmmWlanId);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicTxBfReptClonedStaToNormalSta(
	RTMP_ADAPTER *pAd,
    UCHAR   ucWlanId,
    UCHAR   ucCliIdx)
{
    if (pAd->chipOps.BfReptClonedStaToNormalSta)
	{
	    return pAd->chipOps.BfReptClonedStaToNormalSta(
	                                   pAd,
	                                   ucWlanId,
	                                   ucCliIdx);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}


INT32 AsicTxBfHwEnStatusUpdate(
	RTMP_ADAPTER *pAd,
    BOOLEAN   fgETxBf,
    BOOLEAN   fgITxBf)
{
    if (pAd->chipOps.BfHwEnStatusUpdate)
	{
	    return pAd->chipOps.BfHwEnStatusUpdate(
	                                   pAd,
	                                   fgETxBf,
	                                   fgITxBf);
    }
    else
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);
		return FALSE;

    }
}
#endif /* MT_MAC && TXBF_SUPPORT */

INT32 AsicRadioOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};

        PmStatCtrl.PmNumber = PM5;
        PmStatCtrl.DbdcIdx = ucDbdcIdx;


        if (ucRadio == WIFI_RADIO_ON){
            PmStatCtrl.PmState = EXIT_PM_STATE;
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DbdcIdx=%d RadioOn\n",
                        __FUNCTION__, ucDbdcIdx));
        }
        else{
    		PmStatCtrl.PmState = ENTER_PM_STATE;
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DbdcIdx=%d RadioOff\n",
                        __FUNCTION__, ucDbdcIdx));
        }

        return  MtCmdExtPmStateCtrl(pAd,PmStatCtrl);
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return 0;
}

#ifdef GREENAP_SUPPORT
INT32 AsicGreenAPOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAPOn)
{
#ifdef MT_MAC
    if (pAd->chipCap.hif_type == HIF_MT)
    {
        MT_GREENAP_CTRL_T GreenAPCtrl = {0};

        GreenAPCtrl.ucDbdcIdx = ucDbdcIdx;
        GreenAPCtrl.ucGreenAPOn = ucGreenAPOn;

        return  MtCmdExtGreenAPOnOffCtrl(pAd, GreenAPCtrl);
    }
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return 0;
}
#endif /* GREENAP_SUPPORT */

INT32 AsicExtPmStateCtrl(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg,
	UINT8 ucPmNumber,
	UINT8 ucPmState)
{
	struct wifi_dev *wdev = NULL;

#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
        wdev = &pAd->ApCfg.MBSSID[0].wdev;
    }
#endif


#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};

		PmStatCtrl.PmNumber = ucPmNumber;
		PmStatCtrl.PmState = ucPmState;

		if (ucPmNumber == PM4)
		{
		}

		return  MtCmdExtPmStateCtrl(pAd, PmStatCtrl);
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return 0;
}

INT32 AsicExtWifiHifCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 PmStatCtrl, VOID *pReslt)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		return  MtCmdWifiHifCtrl(pAd, ucDbdcIdx, PmStatCtrl, pReslt);
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return 0;
}

#ifdef CONFIG_MULTI_CHANNEL

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
    UINT32 start_tsf)
{


#ifdef MT_MAC
	if (ad->chipCap.hif_type == HIF_MT)
	{

		MT_MCC_ENTRT_T entries[2];
		entries[0].BssIdx = 0;
		entries[0].WlanIdx = 1;
		entries[0].WmmIdx = 0;
		entries[0].OwnMACAddressIdx = 0;
		entries[0].Bw = bw_1st;
		entries[0].CentralSeg0 =  central_1st_seg0;
		entries[0].CentralSeg1 =  central_1st_seg1;
		entries[0].Channel = channel_1st;
		entries[0].Role = role_1st;
		entries[0].StayTime = stay_time_1st;
		entries[1].BssIdx = 1;
		entries[1].WlanIdx = 2;
		entries[1].WmmIdx = 1;
		entries[1].OwnMACAddressIdx = 1;
		entries[1].Bw = bw_2nd;
		entries[1].CentralSeg0 =  central_2nd_seg0;
		entries[1].CentralSeg1 =  central_2nd_seg1;
		entries[1].Channel = channel_2nd;
		entries[1].Role = role_2nd;
		entries[1].StayTime = stay_time_2nd;

		return MtCmdMccStart(ad, 2,entries,idle_time,null_repeat_cnt,start_tsf);

	}
#endif
	AsicNotSupportFunc(ad, __FUNCTION__);
	return 0;
}

#endif




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
    UINT8 ucType)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{

		INT32 ret = 0;
		ret = MtCmdThermalProtect(pAd,HighEn,HighTempTh,LowEn,LowTempTh,RechkTimer,RFOffEn,RFOffTh,ucType);

		if(ret == NDIS_STATUS_SUCCESS)
		{
			pAd->thermal_pro_high_criteria = HighTempTh;
			pAd->thermal_pro_high_en = HighEn;
			pAd->thermal_pro_low_criteria = LowTempTh;
			pAd->thermal_pro_low_en = LowEn;
            pAd->recheck_timer = RechkTimer;
            pAd->thermal_pro_RFOff_criteria = RFOffTh;
            pAd->thermal_pro_RFOff_en = RFOffEn;
		}
		return ret;
	}
#endif
	AsicNotSupportFunc(pAd, __FUNCTION__);

    return 0;
}

INT32
AsicThermalProtectAdmitDuty(
	RTMP_ADAPTER *pAd,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
	)
{

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{

		INT32 ret = 0;
		ret = MtCmdThermalProtectAdmitDuty(pAd,u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);

		return ret;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

    return 0;
}
#endif /* THERMAL_PROTECT_SUPPORT */


INT32 AsicGetMacInfo(RTMP_ADAPTER *pAd, UINT32 *ChipId,UINT32 *HwVer, UINT32 *FwVer)
{
	INT32 ret;
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		ret = MtAsicGetMacInfo(pAd, ChipId, HwVer,FwVer);
		return ret ;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return 0;
}
#endif/*COMPOS_TESTMODE_WIN*/
INT32 AsicGetAntMode(RTMP_ADAPTER *pAd,UCHAR *AntMode)
{
	INT32 ret;
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		ret = MtAsicGetAntMode(pAd,AntMode);
		return ret ;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return 0;
}

INT32 AsicSetDmaByPassMode(RTMP_ADAPTER *pAd,BOOLEAN isByPass)
{
	INT32 ret;
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		ret = MtAsicSetDmaByPassMode(pAd,isByPass);
		return ret ;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return 0;
}

#ifdef DBDC_MODE
INT32 AsicGetDbdcCtrl(RTMP_ADAPTER *pAd,BCTRL_INFO_T *pBctrlInfo)
{
	INT32 ret=0;

	if (pAd->archOps.archGetDbdcCtrl)
	{
	    ret = pAd->archOps.archGetDbdcCtrl(pAd, pBctrlInfo);
	}
	else
	{
	    AsicNotSupportFunc(pAd, __FUNCTION__);
	}

	return ret;
}

INT32 AsicSetDbdcCtrl(RTMP_ADAPTER *pAd,BCTRL_INFO_T *pBctrlInfo)
{
	INT32 ret=0;
	if (pAd->archOps.archSetDbdcCtrl)
	{
	    ret = pAd->archOps.archSetDbdcCtrl(pAd, pBctrlInfo);
	}
	else
	{
	    AsicNotSupportFunc(pAd, __FUNCTION__);
	}

	return ret;
}

#endif /*DBDC_MODE*/

INT32 AsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP)
{
	INT32 ret=0;
	if(pAd->archOps.archRxHeaderTransCtl){
		ret = pAd->archOps.archRxHeaderTransCtl(pAd, En, ChkBssid, InSVlan, RmVlan, SwPcP);
	}
	return ret;
}

INT32 AsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{
	INT32 ret=0;
	if(pAd->archOps.archRxHeaderTaranBLCtl){
		ret = pAd->archOps.archRxHeaderTaranBLCtl(pAd, Index, En, EthType);
	}
	return ret;
}


UINT32 AsicFillRxBlkAndPktProcess(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket)
{
	UINT32 Len = 0;

	if (pAd->archOps.archFillRxBlkAndPktProcess)
	{
		Len = pAd->archOps.archFillRxBlkAndPktProcess(pAd, pRxBlk, pRxPacket);
	}

	return Len;
}


#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN AsicMcastEntryInsert(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex)
{

	INT32 Ret=0;

	if (pAd->archOps.archMcastEntryInsert)
	{
		Ret = pAd->archOps.archMcastEntryInsert(pAd, GrpAddr, BssIdx, Type, MemberAddr, dev, WlanIndex);
	}

	return Ret;
}


BOOLEAN AsicMcastEntryDelete(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT8 WlanIndex)
{
	INT32 Ret = 0;

	if (pAd->archOps.archMcastEntryDelete)
	{
		Ret = pAd->archOps.archMcastEntryDelete(pAd, GrpAddr, BssIdx, MemberAddr, dev, WlanIndex);
	}

	return Ret;
}
#endif

#ifdef DOT11_VHT_AC
INT AsicSetRtsSignalTA(RTMP_ADAPTER *pAd)
{
    BOOLEAN Enable;

    Enable = (pAd->CommonCfg.vht_bw_signal != BW_SIGNALING_DISABLE) ? TRUE : FALSE;
#if defined(RTMP_MAC) || defined(RLT_MAC)
    if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
    {
        AsicNotSupportFunc(pAd, __FUNCTION__);

        return FALSE;
    }
#endif

#ifdef MT_MAC
    if (pAd->chipCap.hif_type == HIF_MT)
    {
#ifdef DBDC_MODE
        if(pAd->CommonCfg.dbdc_mode)
        {
            pAd->archOps.archSetRtsSignalTA(pAd, 1, Enable);
        }
#endif /*  DBDC_MODE */
        pAd->archOps.archSetRtsSignalTA(pAd, 0, Enable);

        return TRUE;
    }
#endif /*  MT_MAC */

    AsicNotSupportFunc(pAd, __FUNCTION__);

    return FALSE;
}
#endif /*DOT11_VHT_AC*/

VOID RssiUpdate(RTMP_ADAPTER *pAd)
{

	CHAR RSSI[4];
	MAC_TABLE_ENTRY *pEntry;
	INT i = 0;
	



#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if ( pAd->MacTab.Size == 0 )
			{				
				pEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];
				MtRssiGet(pAd,pEntry->wcid,&RSSI[0]);
				
				for( i = 0; i < TX_STREAM_PATH; i++)
				{
					pEntry->RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
					pEntry->RssiSample.LastRssi[i] = MINIMUM_POWER_VALUE;
					pAd->ApCfg.RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
					pAd->ApCfg.RssiSample.LastRssi[i] = MINIMUM_POWER_VALUE;
				}			
			}
			else
			{
				INT32 TotalRssi[4];
				INT j;

				NdisZeroMemory(TotalRssi,sizeof(TotalRssi));

				for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
				{
					pEntry = &pAd->MacTab.Content[i];
					if (IS_VALID_ENTRY(pEntry))
					{
						MtRssiGet(pAd,pEntry->wcid,&RSSI[0]);

						for( j = 0; j < TX_STREAM_PATH; j++)
						{
							pEntry->RssiSample.AvgRssi[j] = RSSI[j];
							pEntry->RssiSample.LastRssi[j] = RSSI[j];
							TotalRssi[j] += RSSI[j];
						}
					}
				}

				for(i=0;i<4;i++)
				{
					pAd->ApCfg.RssiSample.AvgRssi[i] = pAd->ApCfg.RssiSample.LastRssi[i] = TotalRssi[i] / pAd->MacTab.Size;
				}				
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	

}

#ifdef LINK_TEST_SUPPORT
VOID LinkTestRcpiSet(RTMP_ADAPTER *pAd, UCHAR Wcid, UINT8 AntIdx, CHAR cRcpi)
{
	struct wtbl_entry tb_entry;
	union WTBL_DW28 wtbl_wd28;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, Wcid));
		return;
	}

    /* Read RCPI from WTBL DW28 */
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 112, &wtbl_wd28.word);

    switch (AntIdx)
    {
        case BITMAP_WF0:
            wtbl_wd28.field.resp_rcpi_0 = cRcpi;
            break;
        case BITMAP_WF1:
            wtbl_wd28.field.resp_rcpi_1 = cRcpi;
            break;
        case BITMAP_WF2:
            wtbl_wd28.field.resp_rcpi_2 = cRcpi;
            break;
        case BITMAP_WF3:
            wtbl_wd28.field.resp_rcpi_3 = cRcpi;
            break;
    }

    /* Write Back RCPI from WTBL DW28 */
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr + 112, wtbl_wd28.word);

    return;
}

VOID LinkTestTimeSlotHandler(RTMP_ADAPTER *pAd)
{
	BOOLEAN  fgCmwLinkStatus = TRUE;
	UINT8	 ucWlanId;
	UINT8    ucBandIdx = BAND0;
	CHAR	 cRssi[4] = {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};
	CHAR	 cMaxRssi = MINIMUM_POWER_VALUE;
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[0];

	/*-------------------------------------------------------------------------------------------------------------------------*/ 
	/* Check Condition:  One STA Connect */
	/*-------------------------------------------------------------------------------------------------------------------------*/ 
	if (1 != pAd->MacTab.Size)
		fgCmwLinkStatus = FALSE;
	
	/*-------------------------------------------------------------------------------------------------------------------------*/ 
	/* Search pEntry Address */
	/*-------------------------------------------------------------------------------------------------------------------------*/ 
	for (ucWlanId = 1; VALID_UCAST_ENTRY_WCID(pAd, ucWlanId); ucWlanId++)
	{
		pEntry = &pAd->MacTab.Content[ucWlanId];
	
		/* APclient and Repeater not apply CMW270 patch */
		if ((IS_ENTRY_REPEATER(pEntry)) || (IS_ENTRY_AP(pEntry)) || (IS_ENTRY_APCLI(pEntry)))
		{
			fgCmwLinkStatus = FALSE;
			break;
		}
		
		if (IS_ENTRY_CLIENT(pEntry))
		{
			UINT8 ucTxStream;
	
			/* Update Rssi value */
			MtRssiGet(pAd, ucWlanId, &cRssi[0]);
			
			for(ucTxStream = 0; ucTxStream < TX_STREAM_PATH; ucTxStream++)
			{
				if (cRssi[ucTxStream] >= 0)
				{
					cRssi[ucTxStream] = MINIMUM_POWER_VALUE;
				}
	
				if (cRssi[ucTxStream] > cMaxRssi)
				{			 
					cMaxRssi = cRssi[ucTxStream];
				}
			}

			/* Check Condition: Support VHT mode Support */
			if (pEntry->SupportRateMode & SUPPORT_VHT_MODE)
				fgCmwLinkStatus = FALSE;

			/* Check condition: 1 Tx Spatial Stream */
			if (pEntry->SupportHTMCS > 0xFF)
				fgCmwLinkStatus = FALSE;

			/* Check condition:  only support BW20 */
			if (pEntry->MaxHTPhyMode.field.BW != BW_20)
				fgCmwLinkStatus = FALSE;

			/* Update Band Index */
			ucBandIdx = HcGetBandByWdev(pEntry->wdev);
			
			break;
		}
	}

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(Link Test) fgCmwLinkStatus: %d \n", fgCmwLinkStatus));
	
	/* Tx Bw Handler */
	LinkTestTxBwCtrl(pAd, fgCmwLinkStatus);

	/* Tx Csd Handler */
	LinkTestTxCsdCtrl(pAd, fgCmwLinkStatus);

	/* Tx Power Up Handler */
	LinkTestTxPowerCtrl(pAd, fgCmwLinkStatus);

	/* Rx Filter Mode control Handler */
	LinkTestAcrCtrl(pAd, fgCmwLinkStatus, cMaxRssi, ucBandIdx);

	/* Rx Stream Handler */
	LinkTestRxStreamCtrl(pAd, fgCmwLinkStatus, cMaxRssi, cRssi, pEntry, ucBandIdx);

}

VOID LinkTestTxBwCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus)
{
	UINT8  ucDbdcBandIdx;
	UINT8  ucBandNum;
					
	/* obtain Number of Band */
	ucBandNum = pAd->CommonCfg.dbdc_mode;

	/* check if Tx Spur workaround enabled */
	if (pAd->fgTxSpurEn)
	{
		/* state transition for enable/disable Tx Spur workaround */
		if ((TX_DEFAULT_BW_STATE != pAd->ucLinkBwState) && (!fgCmwLinkStatus) && (pAd->fgBwInfoUpdate))
		{
			/* Channel switch for change to Original CBW */
			for (ucDbdcBandIdx = BAND0; ucDbdcBandIdx <= ucBandNum; ucDbdcBandIdx++)
			{
				if (pAd->ucOriCBW[ucDbdcBandIdx] != BW_20)
				{
					if (pAd->ucOriCenterChannel[ucDbdcBandIdx] > pAd->ucOriChannel[ucDbdcBandIdx])
						AsicSetChannel(pAd, pAd->ucOriCenterChannel[ucDbdcBandIdx], pAd->ucOriCBW[ucDbdcBandIdx],  EXTCHA_ABOVE, FALSE);
					else
						AsicSetChannel(pAd, pAd->ucOriCenterChannel[ucDbdcBandIdx], pAd->ucOriCBW[ucDbdcBandIdx],  EXTCHA_BELOW, FALSE);
				}
				else
				{
					AsicSetChannel(pAd, pAd->ucOriChannel[ucDbdcBandIdx], pAd->ucOriCBW[ucDbdcBandIdx],  EXTCHA_NONE, TRUE);
				}
			}

			/* reset Bw and channel Info status flag */
			pAd->fgBwInfoUpdate = FALSE;

			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) Back to Original BW!!! \n")); 		   

			/* update State Stautus */
			pAd->ucLinkBwState = TX_DEFAULT_BW_STATE;
		}
		else if ((TX_SWITCHING_BW_STATE != pAd->ucLinkBwState) && (fgCmwLinkStatus))
		{
			UINT8  ucRfBand[BAND_NUM] = {RFIC_24GHZ, RFIC_5GHZ};

			for (ucDbdcBandIdx = BAND0; ucDbdcBandIdx <= ucBandNum; ucDbdcBandIdx++)
			{
				/* obtain 2G/5G Info */
				pAd->ucOriChannel[ucDbdcBandIdx] = HcGetChannelByRf(pAd, ucRfBand[ucDbdcBandIdx]);
				pAd->ucOriCenterChannel[ucDbdcBandIdx] = HcGetCentralChByRf(pAd, ucRfBand[ucDbdcBandIdx]);
				pAd->ucOriCBW[ucDbdcBandIdx] = HcGetBwByRf(pAd, ucRfBand[ucDbdcBandIdx]);

				/* Channel switch for change to CBW20 */
				AsicSetChannel(pAd, pAd->ucOriChannel[ucDbdcBandIdx], BW_20, EXTCHA_NONE, TRUE);

				MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) [BAND%d] ucOriCBW: %d, ucOriChannel: %d, ucOriCenterChannel: %d \n",
																	ucDbdcBandIdx,
																	pAd->ucOriCBW[ucDbdcBandIdx],
																	pAd->ucOriChannel[ucDbdcBandIdx],
																	pAd->ucOriCenterChannel[ucDbdcBandIdx]));
			}

			/* update Bw and channel Info status flag */
			pAd->fgBwInfoUpdate = TRUE;
			
			/* update State Stautus */
			pAd->ucLinkBwState = TX_SWITCHING_BW_STATE;
		}
	}
}


VOID LinkTestTxCsdCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus)
{
	BOOLEAN  fgZeroCsd = FALSE;
	UINT8	 ucDbdcBandIdx;
	UINT8	 ucBandNum;
					
	/* obtain Number of Band */
	ucBandNum = pAd->CommonCfg.dbdc_mode;

	/* check MAC Table size to determine enabled/disabled status of Tx CSD config */
	if ((0 == pAd->MacTab.Size) || ((1 == pAd->MacTab.Size) && (fgCmwLinkStatus)))
		fgZeroCsd = TRUE;

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) fgCmwLinkStatus: %d, fgZeroCsd: %d\n",
															fgCmwLinkStatus, fgZeroCsd));

	/* CSD config for state transition */
	if ((TX_ZERO_CSD_STATE != pAd->ucTxCsdState) && (fgZeroCsd))
	{
		/* Zero CSD config for Link test */
		for (ucDbdcBandIdx = BAND0; ucDbdcBandIdx <= ucBandNum; ucDbdcBandIdx++)
			MtCmdLinkTestTxCsdCtrl(pAd, TRUE, ucDbdcBandIdx, pAd->ucCmwChannelBand[ucDbdcBandIdx]);

		/* update Tx CSD State */
		pAd->ucTxCsdState = TX_ZERO_CSD_STATE;
	}
	else if ((TX_DEFAULT_CSD_STATE != pAd->ucTxCsdState) && (!fgZeroCsd))
	{
		/* Default CSD config for Link test */
		for (ucDbdcBandIdx = BAND0; ucDbdcBandIdx <= ucBandNum; ucDbdcBandIdx++)
			MtCmdLinkTestTxCsdCtrl(pAd, FALSE, ucDbdcBandIdx, pAd->ucCmwChannelBand[ucDbdcBandIdx]);

		/* update Tx CSD State */
		pAd->ucTxCsdState = TX_DEFAULT_CSD_STATE;
	}
}

VOID LinkTestTxPowerCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus)
{
	BOOLEAN fgPowerBoost = FALSE;
	UINT8   ucDbdcBandIdx;
	UINT8   ucBandNum;
					
	/* obtain Number of Band */
	ucBandNum = pAd->CommonCfg.dbdc_mode;

	/* check MAC Table size to determine enabled/disabled status of Tx Power up config */
	if ((1 == pAd->MacTab.Size) && (fgCmwLinkStatus))
		fgPowerBoost = TRUE;

	MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) fgCmwLinkStatus: %d, fgPowerBoost: %d\n",
															fgCmwLinkStatus, fgPowerBoost));

	/* Tx Power config for state transition */
	if ((TX_BOOST_POWER_STATE != pAd->ucTxPwrBoostState) && (fgPowerBoost))
	{
		/* Boost Tx Power config for Link test */
		for (ucDbdcBandIdx = BAND0; ucDbdcBandIdx <= ucBandNum; ucDbdcBandIdx++)
			MtCmdLinkTestTxPwrCtrl(pAd, TRUE, ucDbdcBandIdx, pAd->ucCmwChannelBand[ucDbdcBandIdx]);

		/* update Tx CSD State */
		pAd->ucTxPwrBoostState = TX_BOOST_POWER_STATE;
	}
	else if ((TX_DEFAULT_POWER_STATE != pAd->ucTxPwrBoostState) && (!fgPowerBoost))
	{
		/* Boost Tx Power config for Link test */
		for (ucDbdcBandIdx = BAND0; ucDbdcBandIdx <= ucBandNum; ucDbdcBandIdx++)
			MtCmdLinkTestTxPwrCtrl(pAd, FALSE, ucDbdcBandIdx, pAd->ucCmwChannelBand[ucDbdcBandIdx]);

		/* update Tx CSD State */
		pAd->ucTxPwrBoostState = TX_DEFAULT_POWER_STATE;
	}
}

VOID LinkTestRxStreamCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, CHAR cMaxRssi, CHAR *cRssi, struct _MAC_TABLE_ENTRY *pEntry, UINT8 ucBandIdx)
{
	UINT8  ucMaxRssiIdx;

	/* Validate Rssi value */
	if (MINIMUM_POWER_VALUE == cMaxRssi)
		return;

	/* Config RSSI Moving Average Ratio 1/2 */
    MtCmdLinkTestRcpiMACtrl(pAd, CMW_RCPI_MA_1_2);

	/* check if Nr Floating workaround enabled */
	if (pAd->fgNrFloatingEn)
	{
		MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) Rssi: (%d : %d : %d : %d) \n",
																cRssi[0], cRssi[1], cRssi[2], cRssi[3]));

		/* only allow potentail state transition for Max Rssi < -40dB for change channel scenario */
		if ((TX_SIGNLE_RXSTREAM_STATE != pAd->ucRxStreamState) && (cMaxRssi < pAd->cNrRssiTh) && (fgCmwLinkStatus))
		{	
			/* RSSI Significance Check */
			ucMaxRssiIdx = LinkTestRssiCheck(pAd, cRssi, 0xFF, CMW_RSSI_SOURCE_WTBL, ucBandIdx);		
			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) MaxRssiIdx: %d \n", ucMaxRssiIdx));

			/* Check Significant RSSI value Antenna Index */
			if (0x0 != ucMaxRssiIdx)
			{
				MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(Link Test) 1R mode after link up!!! RX index: %d \n", ucMaxRssiIdx));

				/* Clear RSSI Value in WTBL */
				MtAsicRcpiReset(pAd, pEntry->wcid);

				/* Enter 1R mode */
				MtCmdLinkTestRxCtrl(pAd, ucMaxRssiIdx);
				
				/* Update 1R PD Detection Status */
				pAd->ucRxStreamState = TX_SIGNLE_RXSTREAM_STATE;
			}
		}
		else if ((TX_DEFAULT_RXSTREAM_STATE != pAd->ucRxStreamState) && (cMaxRssi > pAd->cChgTestPathTh))
		{
			INT64  c8RxCount;

			/* Read Rx Count Info */
	        c8RxCount = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;

			/* check Rx test timeout count */
	        if (c8RxCount - pAd->u4TempRxCount <= pAd->ucRxCountTh)
	            pAd->ucRxTestTimeoutCount++;
	        else
	            pAd->ucRxTestTimeoutCount = 0;

			/* update Rx Count to temp buffer */
			pAd->u4TempRxCount = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;

			/* Restore to default Rx Stream config for Timeout condition or not Link Test scenario */
			if ((pAd->ucRxTestTimeoutCount > pAd->ucTimeOutTh) || (!fgCmwLinkStatus))
	        {
	            /* Clear RSSI Value in WTBL */
	            MtAsicRcpiReset(pAd, pEntry->wcid);
	            
	            /* Restore to 4R Config */
	            MtCmdLinkTestRxCtrl(pAd, BITMAP_WF_ALL);

	            /* Update 1R PD Detection Status */
	            pAd->ucRxStreamState = TX_DEFAULT_RXSTREAM_STATE;

				/* Clear Timeout Count */
	            pAd->ucRxTestTimeoutCount = 0;
	        }
		}
	}
}

VOID LinkTestAcrCtrl(RTMP_ADAPTER *pAd, BOOLEAN fgCmwLinkStatus, CHAR cMaxRssi, UINT8 ucBandIdx)
{
    /* Validate Cmw Instrument */
    if (!fgCmwLinkStatus)
        return;

	/* Check ACI patch enable/disable status */
	if (pAd->fgACREn)
    {
		if ((TX_SPECIFIC_ACR_STATE != pAd->ucRxFilterstate) && (cMaxRssi <= pAd->cMaxInRssiTh))
		{
			if (pAd->ucRxFilterConfidenceCnt >= pAd->ucACRConfidenceCntTh)
            {
                /* Fw command to apply ACI patch */
                MtCmdLinkTestACRCtrl(pAd, TRUE, ucBandIdx);   // ACR patch

				/* update Rx Filter State */
                pAd->ucRxFilterstate = TX_SPECIFIC_ACR_STATE;

                /* Clear ACR confidence count */
                pAd->ucRxFilterConfidenceCnt = 0;

				MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) ACR patch!!! \n"));
            }
            else
            {
                /* ACI confidence count increment */
                pAd->ucRxFilterConfidenceCnt++;
            }
		}
		else if ((TX_DEFAULT_MAXIN_STATE != pAd->ucRxFilterstate) && (cMaxRssi > pAd->cMaxInRssiTh))
		{
			if (pAd->ucRxFilterConfidenceCnt >= pAd->ucMaxInConfidenceCntTh)
            {
                /* Fw command to apply MaxIn patch */
                MtCmdLinkTestACRCtrl(pAd, FALSE, ucBandIdx);  // Max Input patch

				/* update Rx Filter State */
                pAd->ucRxFilterstate = TX_DEFAULT_MAXIN_STATE;

                /* Clear MaxIn confidence count */
                pAd->ucRxFilterConfidenceCnt = 0;

				MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(Link Test) MaxIn patch!!! \n"));
            }
            else
            {
                /* MaxIn confidence count increment */
                pAd->ucRxFilterConfidenceCnt++;
            }
		}
	}
}

/*
 *  Function: check RSSI Significance path
 *
 *  Parameter:
 *
 *      @ pAd
 *
 *      @ pcRSSI: pointer of array of RSSI values
 *
 *      @ ucRSSIThManual: RSSI Significance Threshold. If this value is 0xFF, program will use dynamic threshold.
 *
 *
 *  Return: 
 *
 *      @ ucResult: RSSI Significant path index bitmap. 0x5 means WF0 and WF2. 0x0 mean no RSSI significant path.
 */

UINT8 LinkTestRssiCheck(RTMP_ADAPTER *pAd, CHAR *cRssi, UCHAR ucRSSIThManual, UINT8 ucRSSISource, UINT8 ucBandIdx)
{
    UINT8   ucRxIdx1 = 0, ucRxIdx2 = 0;
    UINT8   ucRxBitmap;
    
    if(pAd->CommonCfg.dbdc_mode)
    {
        if (CMW_RSSI_SOURCE_BBP == ucRSSISource)
        {
            /* DBDC Band0 RSSI Check */
            ucRxIdx1 = LinkTestRssiComp(pAd, cRssi, ucRSSIThManual, WF0, WF1);

            if (0xFF != ucRxIdx1)
                ucRxIdx1 = 1 << (ucRxIdx1);
            else
                ucRxIdx1 = 0;

            /* DBDC Band1 RSSI Check */
            ucRxIdx2 = LinkTestRssiComp(pAd, cRssi, ucRSSIThManual, WF2, WF3);

            if (0xFF != ucRxIdx2)
                ucRxIdx2 = 1 << (ucRxIdx2);
            else
                ucRxIdx2 = 0;

            /* update Rx Bitmap */
            ucRxBitmap = ucRxIdx1 + ucRxIdx2;
        }
        else
        {
            if (BAND0 == ucBandIdx)
            {
                ucRxIdx1 = LinkTestRssiComp(pAd, cRssi, ucRSSIThManual, WF0, WF1);

                if (0xFF != ucRxIdx1)
                    ucRxIdx1 = 1 << (ucRxIdx1);
                else
                    ucRxIdx1 = 0;
            }
            else if (BAND1 == ucBandIdx)
            {
                ucRxIdx1 = LinkTestRssiComp(pAd, cRssi, ucRSSIThManual, WF0, WF1);

                if (0xFF != ucRxIdx1)
                    ucRxIdx1 = 1 << (ucRxIdx1 + 2);
                else
                    ucRxIdx1 = 0;
            }

            /* update Rx Bitmap */
            ucRxBitmap = ucRxIdx1;
        }
    }
    else
    {
        /* Single Band RSSI Check */
        ucRxIdx1 = LinkTestRssiComp(pAd, cRssi, ucRSSIThManual, WF0, WF3);

        if (0xFF != ucRxIdx1)
            ucRxIdx1 = 1 << (ucRxIdx1);
        else
            ucRxIdx1 = 0;

        /* update Rx Bitmap */
        ucRxBitmap = ucRxIdx1;
    }

    return ucRxBitmap;
}

UINT8 LinkTestRssiComp(RTMP_ADAPTER *pAd, CHAR *cRssi, UCHAR ucRSSIThManual, UINT8 ucStart, UINT8 ucEnd)
{
    CHAR    cMaxRssi = MINIMUM_POWER_VALUE;
    CHAR    cSecRssi = MINIMUM_POWER_VALUE;
    UINT8   ucMaxRssiIdx = 0;
    UINT8   ucSecRssiIdx = 0;
    UINT8   ucTxStreamMap[4] = {0,0,0,0};
    UINT8   ucTxStream;
    UINT8   ucRssiTh;
    UINT8   ucRxIdx;

    /* Find Largest RSSI Port */
    for(ucTxStream = ucStart; ucTxStream <= ucEnd; ucTxStream++)
    {
        if (cRssi[ucTxStream] > cMaxRssi)
        {            
            cMaxRssi = cRssi[ucTxStream];
            ucMaxRssiIdx = ucTxStream;
        }
    }

    /* Find Second RSSI Port */
    for(ucTxStream = ucStart; ucTxStream <= ucEnd; ucTxStream++)
    {
        if (ucTxStream != ucMaxRssiIdx)
        {
            if (cRssi[ucTxStream] > cSecRssi)
            {            
                cSecRssi = cRssi[ucTxStream];
                ucSecRssiIdx = ucTxStream;
            }
        }
    }

    /* Update RSSI Threshold for check */
    if (0xFF == ucRSSIThManual)
    {
        if (cSecRssi > pAd->cLargePowerTh)
        {
            ucRssiTh = pAd->ucHighPowerRssiTh;
        }
        else
        {
            ucRssiTh = pAd->ucLowPowerRssiTh;
        }
    }
    else
    {
        ucRssiTh = ucRSSIThManual;
    }
    
    /* Map notation for Max RSSI Antenna Index */
    ucTxStreamMap[ucMaxRssiIdx] = 1;              

    /* Assign Max RSSI Antenna Index */
    ucRxIdx = ucMaxRssiIdx;

    /* Check RSSI Threshold */
    for(ucTxStream = ucStart; ucTxStream <= ucEnd; ucTxStream++)
    {
        if (0 == ucTxStreamMap[ucTxStream])
        {            
            if (cMaxRssi - cRssi[ucTxStream] < ucRssiTh)
            {
                ucRxIdx = 0xFF;
            }
        }
    }

    return ucRxIdx;
}
#endif /* LINK_TEST_SUPPORT */

INT AsicAmpduEfficiencyAdjust(struct wifi_dev *wdev, UCHAR	aifs_adjust)
{
	struct _RTMP_ADAPTER *ad;
	UINT32	wmm_idx;
	if(!wdev)
		return 0;

	ad = wdev->sys_handle;
	wmm_idx = HcGetWmmIdx(ad, wdev);

#ifdef MT_MAC
	if (ad->archOps.asic_ampdu_efficiency_on_off){
		return ad->archOps.asic_ampdu_efficiency_on_off(ad, wmm_idx, aifs_adjust);
	}
#endif

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

INT AsicRtsOnOff(struct wifi_dev *wdev, BOOLEAN rts_en)
{
	struct _RTMP_ADAPTER *ad;
	UCHAR band_idx;
	UINT32 rts_num;
	UINT32 rts_len;

	if(!wdev)
		return 0;

	ad = wdev->sys_handle;
	band_idx = HcGetBandByWdev(wdev);
#ifdef MT_MAC
	if (ad->archOps.asic_rts_on_off){
		if (rts_en) {
			rts_num = wlan_operate_get_rts_pkt_thld(wdev);
			rts_len = wlan_operate_get_rts_len_thld(wdev);
			return ad->archOps.asic_rts_on_off(ad,band_idx, rts_num, rts_len, rts_en);
		} else {
			rts_num = 0xff;
			rts_len = 0xffffff;
			return ad->archOps.asic_rts_on_off(ad,band_idx, rts_num, rts_len, rts_en);
		}
	}
#endif

	AsicNotSupportFunc(ad, __func__);
	return 0;
}
