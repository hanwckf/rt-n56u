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

#include "rt_config.h"


static char *hif_2_str[]={"HIF_RTMP", "HIF_RLT", "HIF_MT", "Unknown"};
VOID AsicNotSupportFunc(RTMP_ADAPTER *pAd, const RTMP_STRING *caller)
{
	RTMP_STRING *str;

	if (pAd->chipCap.hif_type <= HIF_MAX) 
		str = hif_2_str[pAd->chipCap.hif_type];
	else
		str = hif_2_str[HIF_MAX];

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupportedFunc for this arch(%s)!\n",
				caller, str));
}


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


#ifdef CONFIG_STA_SUPPORT
VOID AsicUpdateAutoFallBackTable(RTMP_ADAPTER *pAd, UCHAR *pRateTable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicUpdateAutoFallBackTable(pAd, pRateTable);
		return;	
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) 
	{
		MtAsicUpdateAutoFallBackTable(pAd, pRateTable);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* CONFIG_STA_SUPPORT */


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
		MtAsicUpdateProtect(pAd, OperationMode, SetMask, bDisableBGProtect, bNonGFExist);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
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

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

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
		MtAsicSwitchChannel(pAd, Channel, bScan);
#endif

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	if (bScan)
		bw = BW_20;
	else {
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


#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
		put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup 
		automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
		the wakeup timer timeout. Driver has to issue a separate command to wake
		PHY up.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TbttNumToNextWakeUp) 
{
	RTMP_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp);
}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever manual wakeup is required
		AsicForceSleep() should only be used when not in INFRA BSS. When
		in INFRA BSS, we should use AsicSleepThenAutoWakeup() instead.
	==========================================================================
 */
VOID AsicForceSleep(RTMP_ADAPTER *pAd)
{

}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever Twakeup timer (set via AsicSleepThenAutoWakeup)
		expired.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
VOID AsicForceWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx)
{
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("--> AsicForceWakeup \n"));
    RTMP_STA_FORCE_WAKEUP(pAd, bFromTx);
}
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		Set My BSSID

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
 /* CFG_TODO */
VOID AsicSetBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR omac_idx)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():SetBSSID=%02x:%02x:%02x:%02x:%02x:%02x,curr_bssid_idx=%d\n",
				__FUNCTION__, PRINT_MAC(pBssid), omac_idx));

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetBssid(pAd, pBssid, omac_idx);
		return;	
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetBssid(pAd, pBssid, omac_idx);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


INT AsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set OwnMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, PRINT_MAC(addr)));

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetDevMac(pAd, addr, omac_idx);	
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetDevMac(pAd, addr, omac_idx);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


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

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT AsicSetMacAddrExt(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetMacAddrExt(pAd, enable);
		return TRUE;	
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetMacAddrExt(pAd, enable);
		return TRUE;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


VOID AsicInsertRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *pAddr)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicInsertRepeaterEntry(pAd, CliIdx, pAddr);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicInsertRepeaterEntry(pAd, CliIdx, pAddr);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


VOID AsicRemoveRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicRemoveRepeaterEntry(pAd, CliIdx);
		return;	
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicRemoveRepeaterEntry(pAd, CliIdx);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
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
		return MtAsicSetRxFilter(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicUpdateTxOP(RTMP_ADAPTER *pAd, UINT32 ac_num, UINT32 txop_val)
{
    INT ret = FALSE;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
    {
        // TODO...
        AsicNotSupportFunc(pAd, __FUNCTION__);
    }
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
    {
        if (pAd->CommonCfg.ManualTxop)
        {    
            return TRUE;
        }    
        ret = MtAsicUpdateTxOP(pAd, ac_num, txop_val);
    }
#endif

    return ret;
}


INT AsicSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	INT ret = FALSE;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		ret = RtAsicSetRDG(pAd, bEnable);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		ret = MtAsicSetRDG(pAd, bEnable);
#endif

	if (ret == TRUE)
	{
		if (bEnable)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		}
		else
		{
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		}
	}

	//AsicNotSupportFunc(pAd, __FUNCTION__);

	return ret;
}


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


INT AsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetPreTbtt(pAd, enable);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetPreTbtt(pAd, enable, 0);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

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
INT AsicGetTsfTime(RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicGetTsfTime(pAd, high_part, low_part);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicGetTsfTime(pAd, high_part, low_part);
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
VOID AsicDisableSync(RTMP_ADAPTER *pAd) 
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicDisableSync(pAd);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicDisableSync(pAd);
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
VOID AsicEnableBssSync(PRTMP_ADAPTER pAd, USHORT BeaconPeriod)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicEnableBssSync(pAd, BeaconPeriod);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_BSS_SYNC_CTRL_T bssSync;
		NdisZeroMemory(&bssSync,sizeof(MT_BSS_SYNC_CTRL_T));

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			
			struct wifi_dev *wdev = &pAd->StaCfg.wdev;	
			bssSync.BssOpMode = MT_BSS_MODE_STA;
			bssSync.PreTbttInterval = 0x50;
			bssSync.BssSet = 0;
			bssSync.BeaconPeriod = wdev->ucBeaconPeriod;
			bssSync.DtimPeriod = wdev->ucDtimPeriod;
			MtAsicEnableBssSync(pAd, bssSync);
		}
#endif

		
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{

			bssSync.BeaconPeriod = BeaconPeriod;
			bssSync.PreTbttInterval = 0xf0;//Carter, 20140710, sync with 7603
			bssSync.BssSet = 0;
			bssSync.BssOpMode = MT_BSS_MODE_AP;					
			MtAsicEnableBssSync(pAd, bssSync);
		}
#endif
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


/*CFG_TODO*/
VOID AsicEnableApBssSync(RTMP_ADAPTER *pAd, USHORT BeaconPeriod)
{
	UINT32 bitmask = 0;
    UINT32 Value;
    INT bss_idx = 1; // TODO: this index may carried by parameters!
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s():\n", __FUNCTION__));

#ifdef CONFIG_AP_SUPPORT
    //INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif /* CONFIG_AP_SUPPORT */

    /* Configure Beacon interval */
    RTMP_IO_READ32(pAd, LPON_T0TPCR+4, &Value);
    Value = 0;
    Value &= ~BEACONPERIODn_MASK;
    Value |= BEACONPERIODn(BeaconPeriod);
    Value |= TBTTn_CAL_EN;
    RTMP_IO_WRITE32(pAd, LPON_T0TPCR+4, Value);


    /* Enable Pre-TBTT Trigger, and calcuate next TBTT timer by HW*/
    //enable PRETBTT0INT_EN, PRETBTT0TIMEUP_EN
    //and TBTT0PERIODTIMER_EN, TBTT0TIMEUP_EN
   	 RTMP_IO_WRITE32(pAd, LPON_MPTCR0, 0x9900);//TODO: TBTT1, TBTT2.
	// RTMP_IO_WRITE32(pAd, LPON_MPTCR0, (TBTT_TIMEUP_EN |TBTT_PERIOD_TIMER_EN | PRETBTT_TIMEUP_EN | PRETBTT_INT_EN) << 8);//TODO: TBTT1, TBTT2.
    /*
	   Set Pre-TBTT interval :
       each HW BSSID has its own PreTBTT interval,
       unit is 64us, 0x00~0xff is configurable.
       Base on RTMP chip experience,
       Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
	*/
    ASSERT(bss_idx <= 3);
    bitmask = 0xff << (bss_idx * 8);
    RTMP_IO_READ32(pAd, LPON_PISR, &Value);
    Value &= (~bitmask);
    Value |= (0x50 << (bss_idx * 8));
    //mac_val =0x50505050;
    RTMP_IO_WRITE32(pAd, LPON_PISR, Value);

	{
			MAC_IO_READ32(pAd, HWIER0, &Value);
			Value = 0;
			Value |= TBTT1;
			Value |= PRETBTT1;
			MAC_IO_WRITE32(pAd, HWIER0, Value);
			printk("enable HWIER0 bit29 and bit20\n");
		
	}
    /* Config BCN/BMC timoeut, or the normal Tx wil be blocked forever if no beacon frame in Queue */
    Value = 0x01800180;
    //RTMP_IO_WRITE32(pAd, LPON_BCNTR, mac_val);

    /* Configure Beacon Queue Operation mode */
    RTMP_IO_READ32(pAd, ARB_SCR, &Value);

	Value &= (~(1<<30)); // work-around to make BCN need to content with other ACs
   	Value &= (~(1<<31)); // work-around to make BMC need to content with other ACs - 20140109 discussion.
    Value |= (BCNQ_OP_MODE_AP << 2);//TODO, Carter, when use other HWBSSID, shall could choose index to set correcorresponding bit.
	//Value = Value & 0xefffffff;
    RTMP_IO_WRITE32(pAd, ARB_SCR, Value);

    /* Start Beacon Queue */
    RTMP_IO_READ32(pAd, ARB_BCNQCR0, &Value);
    Value |= 0x2;
    RTMP_IO_WRITE32(pAd, ARB_BCNQCR0, Value);
}


#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
	Note: 
		BEACON frame in shared memory should be built ok before this routine
		can be called. Otherwise, a garbage frame maybe transmitted out every
		Beacon period.

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableIbssSync(RTMP_ADAPTER *pAd)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicEnableIbssSync(pAd);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MT_BSS_SYNC_CTRL_T bssSync;
		NdisZeroMemory(&bssSync,sizeof(MT_BSS_SYNC_CTRL_T));
		bssSync.BeaconPeriod = pAd->CommonCfg.BeaconPeriod;
		bssSync.BssOpMode= MT_BSS_MODE_ADHOC;		
		bssSync.PreTbttInterval = 0x50;
		bssSync.BssSet = 0;
		MtAsicEnableBssSync(pAd, bssSync);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* CONFIG_STA_SUPPORT */




INT AsicSetWmmParam(RTMP_ADAPTER *pAd, UINT ac, UINT type, UINT val)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetWmmParam(pAd, ac, type, val);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetWmmParam(pAd, ac, type, val);
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
VOID AsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm)
{
	INT i;

	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NoEDCAParam\n", __FUNCTION__));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i=0; i < MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) || IS_ENTRY_APCLI(&pAd->MacTab.Content[i]))
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		NdisZeroMemory(&pAd->CommonCfg.APEdcaParm, sizeof(EDCA_PARM));

	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (INFRA_ON(pAd))
				CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_WMM_CAPABLE);
		}
#endif /* CONFIG_STA_SUPPORT */

		NdisMoveMemory(&pAd->CommonCfg.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
		if (!ADHOC_ON(pAd))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM\n", pEdcaParm->EdcaUpdateCount));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[0],
									 pEdcaParm->Cwmin[0],
									 pEdcaParm->Cwmax[0],
									 pEdcaParm->Txop[0]<<5,
									 pEdcaParm->bACM[0]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[1],
									 pEdcaParm->Cwmin[1],
									 pEdcaParm->Cwmax[1],
									 pEdcaParm->Txop[1]<<5,
									 pEdcaParm->bACM[1]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[2],
									 pEdcaParm->Cwmin[2],
									 pEdcaParm->Cwmax[2],
									 pEdcaParm->Txop[2]<<5,
									 pEdcaParm->bACM[2]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[3],
									 pEdcaParm->Cwmin[3],
									 pEdcaParm->Cwmax[3],
									 pEdcaParm->Txop[3]<<5,
									 pEdcaParm->bACM[3]));
		}

	}

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
	IN UCHAR channel) 
{
	UINT32 SlotTime = 0;
	UINT32 SifsTime;

#ifdef CONFIG_STA_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
		bUseShortSlotTime = TRUE;
#endif /* CONFIG_STA_SUPPORT */

	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
		return;
	else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
		return;

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime)? 9 : 20;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* force using short SLOT time for FAE to demo performance when TxBurst is ON*/
		if (((pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
#ifdef DOT11_N_SUPPORT
			|| ((pAd->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))
#endif /* DOT11_N_SUPPORT */
			)
		{
			/* In this case, we will think it is doing Wi-Fi test*/
			/* And we will not set to short slot when bEnableTxBurst is TRUE.*/
		}
		else if (pAd->CommonCfg.bEnableTxBurst)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 9;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	
	/* For some reasons, always set it to short slot time.*/
	/* ToDo: Should consider capability with 11B*/
#ifdef CONFIG_STA_SUPPORT 
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->StaCfg.BssType == BSS_ADHOC)
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 20;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (channel > 14)
		SifsTime = SIFS_TIME_5G;
	else
		SifsTime = SIFS_TIME_24G;


#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetSlotTime(pAd, SlotTime, SifsTime);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetSlotTime(pAd, SlotTime, SifsTime);
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
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetMacMaxLen(pAd);
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


#ifdef CONFIG_AP_SUPPORT
VOID AsicGetTxTsc(RTMP_ADAPTER *pAd, UCHAR apidx, UCHAR *pTxTsc)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicGetTxTsc(pAd, apidx, pTxTsc);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicGetTxTsc(pAd, apidx, pTxTsc);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* CONFIG_AP_SUPPORT */


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

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicMcsLutUpdate(pAd, pEntry);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* MCS_LUT_SUPPORT */


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
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicUpdateBASession(pAd, wcid, tid, sn, basize, isAdd, ses_type);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


VOID AsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr)
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
		MtAsicUpdateRxWCIDTable(pAd, WCID, pAddr);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
	

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
		MtAsicDelWcidTab(pAd, wcid_idx);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
	

/*
	========================================================================
	Description:
		Add Pair-wise key material into ASIC. 
		Update pairwise key, TxMic and RxMic to Asic Pair-wise key table
				
    Return:
	========================================================================
*/
VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicAddPairwiseKeyEntry(pAd, WCID, pCipherKey);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicAddPairwiseKeyEntry(pAd, WCID, pCipherKey);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}


/*
	========================================================================
	Description:
		Remove Pair-wise key material from ASIC. 

    Return:
	========================================================================
*/	
VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Wcid)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicRemovePairwiseKeyEntry(pAd, Wcid);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicRemovePairwiseKeyEntry(pAd, Wcid);
		return;
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
		MtAsicTurnOffRFClk(pAd, Channel);
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
#endif // DOT11_N_SUPPORT //




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
		return MtAsicSetMacTxRx(pAd, txrx, enable);
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
		return MtAsicSetWPDMA(pAd, TxRx, enable);
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


INT AsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetTxStream(pAd, opmode, up);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetTxStream(pAd, pAd->Antenna.field.TxPath);
#endif /* MT_MAC */

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 rx_path)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetRxStream(pAd, rx_path);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetRxStream(pAd, rx_path);
#endif /* MT_MAC */

	AsicNotSupportFunc(pAd, __FUNCTION__);

	return FALSE;
}


INT AsicSetBW(RTMP_ADAPTER *pAd, INT bw)
{
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicSetBW(pAd, bw);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicSetBW(pAd, bw);
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
	bbp_set_bw(pAd, bw);

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
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
	{
		RtAsicSetApCliBssid(pAd, pBssid, index);
		return;
	}
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtAsicSetApCliBssid(pAd, pBssid, index);
		return;
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
}
#endif /* MAC_APCLI_SUPPORT */

