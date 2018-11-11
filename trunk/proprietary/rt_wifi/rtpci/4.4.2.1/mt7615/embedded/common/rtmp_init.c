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
	rtmp_init.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */


#ifdef OS_ABL_FUNC_SUPPORT
/* Os utility link: printk, scanf */
RTMP_OS_ABL_OPS RaOsOps, *pRaOsOps = &RaOsOps;
#endif /* OS_ABL_FUNC_SUPPORT */

UCHAR NUM_BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#ifdef DBG
char *CipherName[] = {"none","wep64","wep128","TKIP","AES","CKIP64","CKIP128","CKIP152","SMS4","WEP152"};
#endif


NDIS_STATUS RTMPAllocGlobalUtility(VOID)
{
#ifdef OS_ABL_FUNC_SUPPORT
	/* must put the function before any print message */
	/* init OS utilities provided from UTIL module */
	RtmpOsOpsInit(&RaOsOps);
#endif /* OS_ABL_FUNC_SUPPORT */

	/* init UTIL module */
	RtmpUtilInit();

	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================

	Routine Description:
		Allocate RTMP_ADAPTER data block and do some initialization

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS RTMPAllocAdapterBlock(VOID *handle, VOID **ppAdapter)
{
	RTMP_ADAPTER *pAd = NULL;
	NDIS_STATUS	 Status;
	INT index;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> RTMPAllocAdapterBlock\n"));

	RTMPAllocGlobalUtility();

	*ppAdapter = NULL;

	do
	{
		/* Allocate RTMP_ADAPTER memory block*/
		Status = AdapterBlockAllocateMemory(handle, (PVOID *)&pAd, sizeof(RTMP_ADAPTER));
		if (Status != NDIS_STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Failed to allocate memory - ADAPTER\n"));
			break;
		}
		else
		{
			/* init resource list (must be after pAd allocation) */
			initList(&pAd->RscTimerMemList);
			initList(&pAd->RscTaskMemList);
			initList(&pAd->RscLockMemList);
			initList(&pAd->RscTaskletMemList);
			initList(&pAd->RscSemMemList);
			initList(&pAd->RscAtomicMemList);

			initList(&pAd->RscTimerCreateList);

			pAd->OS_Cookie = handle;
			((POS_COOKIE)(handle))->pAd_va = (LONG)pAd;
		}
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n=== pAd = %p, size = %zu ===\n\n", pAd, sizeof(RTMP_ADAPTER)));

		if (RtmpOsStatsAlloc(&pAd->stats, &pAd->iw_stats) == FALSE)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		/*Allocate Timer Lock*/
		NdisAllocateSpinLock(pAd, &pAd->TimerSemLock);

		/*Allocate interface lock*/
		NdisAllocateSpinLock(pAd, &pAd->VirtualIfLock);

#ifdef MT_MAC
		NdisAllocateSpinLock(pAd, &pAd->BcnRingLock);
#endif /* MT_MAC */

		/* Init spin locks*/
		NdisAllocateSpinLock(pAd, &pAd->MgmtRingLock);

#if defined(RT3290) || defined(RLT_MAC)
		NdisAllocateSpinLock(pAd, &pAd->WlanEnLock);
#endif /* defined(RT3290) || defined(RLT_MAC) */

		NdisAllocateSpinLock(pAd, &pAd->WdevListLock);
		NdisAllocateSpinLock(pAd, &pAd->BssInfoIdxBitMapLock);

		for (index =0 ; index < NUM_OF_TX_RING; index++)
		{
			NdisAllocateSpinLock(pAd, &pAd->TxSwQueueLock[index]);
			NdisAllocateSpinLock(pAd, &pAd->DeQueueLock[index]);
			pAd->DeQueueRunning[index] = FALSE;
		}

		NdisAllocateSpinLock(pAd, &pAd->irq_lock);


#ifdef CONFIG_FWOWN_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->DriverOwnLock);
#endif /* CONFIG_FWOWN_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
        NdisAllocateSpinLock(pAd, &pAd->ApCfg.ReptCliEntryLock);
		NdisAllocateSpinLock(pAd, &pAd->ApCfg.InsertReptCmdLock);
        NdisAllocateSpinLock(pAd, &pAd->ApCfg.CliLinkMapLock);
#endif
#endif
#endif

#ifdef GREENAP_SUPPORT
        NdisAllocateSpinLock(pAd, &pAd->ApCfg.greenap.lock);
#endif /* GREENAP_SUPPORT */

		/*HIF related initial for pAd*/
		RTMPInitHifAdapterBlock(pAd);

#ifdef RLM_CAL_CACHE_SUPPORT
        rlmCalCacheInit(pAd, &pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */

		*ppAdapter = (VOID *)pAd;
	} while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pAd)
		{
			if (pAd->stats) {
				os_free_mem(pAd->stats);
				pAd->stats = NULL;
			}

			if (pAd->iw_stats) {
				os_free_mem(pAd->iw_stats);
				pAd->iw_stats = NULL;
			}

			RtmpOsVfree(pAd);
		}

		return Status;
	}

	/* Init ProbeRespIE Table */
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++)
	{
		if (os_alloc_mem(pAd,&pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN) == NDIS_STATUS_SUCCESS)
			RTMPZeroMemory(pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN);
		else
			pAd->ProbeRespIE[index].pIe = NULL;
	}

	/*init queue for ip assembly*/
#if defined(MT_MAC)  && defined(IP_ASSEMBLY)
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
		DlListInit(&pAd->assebQueue[index]);
	}
#endif

	/*init WifiSys information structure*/
	WifiSysInfoInit(pAd);

	/*allocate wpf related memory*/
	wpf_config_init(pAd);

#ifdef MULTI_INF_SUPPORT
	Status = multi_inf_adapt_reg((VOID *) pAd);
#endif /* MULTI_INF_SUPPORT */

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- RTMPAllocAdapterBlock, Status=%x\n", Status));

	return Status;
}


BOOLEAN RTMPCheckPhyMode(RTMP_ADAPTER *pAd, UINT8 band_cap, UCHAR *pPhyMode)
{
	BOOLEAN RetVal = TRUE;

	if (band_cap == RFIC_24GHZ)
	{
		if (!WMODE_2G_ONLY(*pPhyMode))
		{
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(): Warning! The board type is 2.4G only!\n",
					__FUNCTION__));
			RetVal =  FALSE;
		}
	}
	else if (band_cap == RFIC_5GHZ)
	{
		if (!WMODE_5G_ONLY(*pPhyMode))
		{
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(): Warning! The board type is 5G only!\n",
					__FUNCTION__));
			RetVal =  FALSE;
		}
	}
	else if (band_cap == RFIC_DUAL_BAND)
	{
		RetVal = TRUE;
	}
	else
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): Unknown supported band (%u), assume dual band used.\n",
				__FUNCTION__, band_cap));

		RetVal = TRUE;
	}

	if (RetVal == FALSE)
	{
#ifdef DOT11_N_SUPPORT
		if (band_cap == RFIC_5GHZ) /*5G ony: change to A/N mode */
			*pPhyMode = PHY_11AN_MIXED;
		else /* 2.4G only or Unknown supported band: change to B/G/N mode */
			*pPhyMode = PHY_11BGN_MIXED;
#else
		if (band_cap == RFIC_5GHZ) /*5G ony: change to A mode */
			*pPhyMode = PHY_11A;
		else /* 2.4G only or Unknown supported band: change to B/G mode */
			*pPhyMode = PHY_11BG_MIXED;
#endif /* !DOT11_N_SUPPORT */

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): Changed PhyMode to %u\n",
				__FUNCTION__, *pPhyMode));
	}

	return RetVal;

}


#ifdef RTMP_INTERNAL_TX_ALC

static VOID RTMPCfgIntTxAlcFromEEPROM(RTMP_ADAPTER *pAd,EEPROM_NIC_CONFIG2_STRUC NicConfig2 )
{
	/*
	Internal Tx ALC support is starting from RT3370 / RT3390, which combine PA / LNA in single chip.
	The old chipset don't have this, add new feature flag RTMP_INTERNAL_TX_ALC.
	*/

	/* Internal Tx ALC */
	if (((NicConfig2.field.DynamicTxAgcControl == 1) &&
            (NicConfig2.field.bInternalTxALC == 1)) ||
            ((!IS_RT3390(pAd)) && (!IS_RT3350(pAd)) &&
            (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)) &&
            (!IS_RT5390(pAd)) && (!IS_RT3290(pAd)) && (!IS_RT6352(pAd)) && (!IS_MT7601(pAd))))
	{
		/*
			If both DynamicTxAgcControl and bInternalTxALC are enabled,
			it is a wrong configuration.
			If the chipset does not support Internal TX ALC, we shall disable it.
		*/
		pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}
	else
	{
		if (NicConfig2.field.bInternalTxALC == 1)
			pAd->TxPowerCtrl.bInternalTxALC = TRUE;
		else
			pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}


	/* Old 5390 NIC always disables the internal ALC */
	if ((pAd->MACVersion == 0x53900501)
	)
		pAd->TxPowerCtrl.bInternalTxALC = FALSE;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: pAd->TxPowerCtrl.bInternalTxALC = %d\n",
		__FUNCTION__, pAd->TxPowerCtrl.bInternalTxALC));
}
#endif

static VOID RTMPCfgPSFromEEPROM(RTMP_ADAPTER *pAd)
{

}

/*
	========================================================================

	Routine Description:
		Set default value from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd)
{

	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> NICInitAsicFromEEPROM\n"));

	NicConfig2.word = pAd->NicConfig2.word;

	/* finally set primary ant */
	AntCfgInit(pAd);

	RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(pAd);

#ifdef RTMP_RF_RW_SUPPORT
	/*Init RFRegisters after read RFIC type from EEPROM*/
	InitRFRegisters(pAd);
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef ANT_DIVERSITY_SUPPORT
	if ((pAd->CommonCfg.bHWRxAntDiversity) &&
		(pAd->CommonCfg.RxAntDiversityCfg == ANT_HW_DIVERSITY_ENABLE) &&
		(pAd->chipOps.HwAntEnable))
		pAd->chipOps.HwAntEnable(pAd);
#endif

	RTMPCfgPSFromEEPROM(pAd);

	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

#ifdef RTMP_INTERNAL_TX_ALC
	RTMPCfgIntTxAlcFromEEPROM(pAd,NicConfig2);
#endif /* RTMP_INTERNAL_TX_ALC */

#ifndef MAC_INIT_OFFLOAD
	AsicSetRxStream(pAd, pAd->Antenna.field.RxPath,0);
#endif


	RTMP_EEPROM_ASIC_INIT(pAd);

 	AsicBbpInitFromEEPROM(pAd);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d\n",
				pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}



INT32 WfHifSysInit(RTMP_ADAPTER *pAd,HIF_INFO_T *pHifInfo)
{

	NDIS_STATUS status;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()-->\n", __FUNCTION__));

	RT28XXDMADisable(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Disable WPDMA\n", __FUNCTION__));

#ifdef CONFIG_WIFI_PAGE_ALLOC_SKB
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use dev_alloc_skb\n"));
#else
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use alloc_skb\n"));
#endif

#ifdef RESOURCE_PRE_ALLOC
	status = RTMPInitTxRxRingMemory(pAd);
#else
	status = RTMPAllocTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */
	if (status != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPAllocTxRxMemory failed, Status[=0x%08x]\n", status));
		goto err;
	}


#ifdef WLAN_SKB_RECYCLE
	skb_queue_head_init(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */

err:
	return status;
}


INT32 WfHifSysExit(RTMP_ADAPTER *pAd)
{
#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif
	return 0;
}


INT32 WfMcuSysInit(RTMP_ADAPTER *pAd)
{
	INT32 status;
	status = RtmpNetTaskInit(pAd);
	if (status != NDIS_STATUS_SUCCESS)
		goto err1;

	MCU_CTRL_INIT(pAd);

#ifdef MT_MAC
	if (pAd->chipOps.FwInit && (pAd->chipCap.hif_type == HIF_MT))
	{
		pAd->chipOps.FwInit(pAd);
	}
#endif /* MT_MAC */

	return NDIS_STATUS_SUCCESS;
err1:
	RtmpNetTaskExit(pAd);
	return NDIS_STATUS_FAILURE;
}

INT32 WfMcuSysExit(RTMP_ADAPTER *pAd)
{
	MCUSysExit(pAd);
	RtmpNetTaskExit(pAd);
	return 0;
}


extern RTMP_STRING *mac;



 VOID  ShowPhyModeByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
 {
	 UCHAR IsRfRun = HcIsRfRun(pAd,RfIC);
	 UCHAR PhyMode = HcGetPhyModeByRf(pAd,RfIC);

	 if(IsRfRun)
	 {
		 MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band Rf: %d, Phy Mode: %d\n", RfIC,PhyMode));
	 }
 }


 INT32 WfEPROMSysInit(RTMP_ADAPTER *pAd)
{

	 UCHAR RfIC;

	 /* hook e2p operation */
	 RtmpChipOpsEepromHook(pAd, pAd->infType,E2P_NONE);

	 /* We should read EEPROM for all cases */
	 // TODO: shiang-7603, revise this!
	 NICReadEEPROMParameters(pAd, (RTMP_STRING *)mac);
	 
	 HcRadioInit(pAd,pAd->RfIcType,pAd->CommonCfg.dbdc_mode);
	 //+++Add by shiang for debug
	 RfIC = HcGetRadioRfIC(pAd);
	 MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s():PhyCtrl=>RfIcType/rf_band_cap = 0x%x/0x%x\n",
				 __FUNCTION__, pAd->RfIcType, RfIC));

	 ShowPhyModeByRf(pAd,RFIC_24GHZ);
	 ShowPhyModeByRf(pAd,RFIC_5GHZ);

	 RTMP_NET_DEV_NICKNAME_INIT(pAd);

	return NDIS_STATUS_SUCCESS;
}


INT32 WfEPROMSysExit(RTMP_ADAPTER *pAd)
{
	return NDIS_STATUS_SUCCESS;
}

/*
	========================================================================

	Routine Description:
		Initialize NIC hardware

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	NICInitializeAdapter(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;


	if((ret = WfMacInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}


	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC Init Done!\n"));

	if((ret = WfPhyInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	ret = NICInitializeAsic(pAd);

err:
	return ret ;

}


INT rtmp_hif_cyc_init(RTMP_ADAPTER *pAd, UINT8 val)
{

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return FALSE;
	}

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		US_CYC_CNT_STRUC USCycCnt;

		RTMP_IO_READ32(pAd, US_CYC_CNT, &USCycCnt.word);
		USCycCnt.field.UsCycCnt = val;
		RTMP_IO_WRITE32(pAd, US_CYC_CNT, USCycCnt.word);
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	return TRUE;
}



/*
	========================================================================

	Routine Description:
		Initialize ASIC

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS NICInitializeAsic(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> NICInitializeAsic\n"));




	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- NICInitializeAsic\n"));

	return NDIS_STATUS_SUCCESS;
}


/*
	========================================================================

	Routine Description:
		Reset NIC from error

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		Reset NIC from error state

	========================================================================
*/
static VOID NICResetFromErrorByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	UCHAR rf_channel;
	BOOLEAN IsRfSupport = HcIsRfSupport(pAd,RfIC);
	UCHAR Channel = HcGetChannelByRf(pAd,RfIC);

	if(IsRfSupport)
	{
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			AsicBBPAdjust(pAd,Channel);
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			rf_channel = pAd->CommonCfg.CentralChannel;
		}
#endif /* CONFIG_AP_SUPPORT */

#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
		AsicSwitchChannel(pAd, rf_channel, FALSE);
		AsicLockChannel(pAd, rf_channel);
#endif /* defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
	}

}

VOID NICResetFromError(RTMP_ADAPTER *pAd)
{

	/* Reset BBP (according to alex, reset ASIC will force reset BBP*/
	/* Therefore, skip the reset BBP*/
	/* RTMP_IO_WRITE32(pAd, MAC_CSR1, 0x2);*/
	// TODO: shaing-7603
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd)) {

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): for MT7603\n", __FUNCTION__));

		NICInitializeAdapter(pAd);

		NICInitAsicFromEEPROM(pAd);

		RTMPEnableRxTx(pAd);

		return;
	}

#ifndef MT_MAC
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
	/* Remove ASIC from reset state*/
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);
#endif /*ndef MT_MAC */

	NICInitializeAdapter(pAd);
	NICInitAsicFromEEPROM(pAd);

	NICResetFromErrorByRf(pAd,RFIC_24GHZ);
	NICResetFromErrorByRf(pAd,RFIC_5GHZ);
}


VOID NICUpdateFifoStaCounters(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_ATE
	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */


#ifdef CONFIG_AP_SUPPORT
#endif

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
//		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
//							__FUNCTION__, __LINE__));
		return;
	}

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
		rtmp_mac_fifo_stat_update(pAd);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
}


int load_patch(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	ULONG Old, New, Diff;

	if (ad->chipOps.load_rom_patch) {
		RTMP_GetCurrentSystemTick(&Old);
		ret = NICLoadRomPatch(ad);
		RTMP_GetCurrentSystemTick(&New);
		Diff = (New - Old) * 1000 / OS_HZ;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("load rom patch spent %ldms\n", Diff));
	}

	return ret;
}




/*
	========================================================================

	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address

	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	/* Equal*/
	return (0);
}


/*
	========================================================================

	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPZeroMemory(VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem[Index] = 0x00;
	}
}


/*
	========================================================================

	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	UINT	Index;

	ASSERT((Length==0) || (pDest && pSrc));

	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1 && pMem2)
			pMem1[Index] = pMem2[Index];
	}
}


VOID UserCfgExit(RTMP_ADAPTER *pAd)
{
#ifdef RT_CFG80211_SUPPORT
	/* Reset the CFG80211 Internal Flag */
	RTMP_DRIVER_80211_RESET(pAd);
#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11_N_SUPPORT
	BATableExit(pAd);
#endif /* DOT11_N_SUPPORT */


	NdisFreeSpinLock(&pAd->MacTabLock);
	RTMP_SEM_EVENT_DESTORY(&pAd->IndirectUpdateLock);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering) {
			BndStrg_Release(pAd);
		}
#endif /* BAND_STEERING */
#ifdef RADIO_LINK_SELECTION
		if (pAd->ApCfg.RadioLinkSelection &&
			pAd->ApCfg.RlsTable.bInitialized)
			Rls_Release(pAd);
#endif /* RADIO_LINK_SELECTION */
#ifdef RADIUS_MAC_ACL_SUPPORT
		{
			PLIST_HEADER pListHeader = NULL;
			RT_LIST_ENTRY *pListEntry = NULL;
			UCHAR apidx = 0;
			
			for (apidx=0; apidx < pAd->ApCfg.BssidNum; apidx++)
			{
				pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;
				if (pListHeader->size == 0)
					continue;
			
				pListEntry = pListHeader->pHead;
				while (pListEntry != NULL)
				{
                			removeHeadList(pListHeader);
                			os_free_mem(pListEntry);
                			pListEntry = pListHeader->pHead;
				}
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Clean [%d] Radius ACL Cache.\n", apidx));	
			}
		}
#endif /* RADIUS_MAC_ACL_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef WSC_INCLUDED
#endif /*WSC_INCLUDED*/

	wdev_config_init(pAd);
}


/*
	========================================================================

	Routine Description:
		Initialize port configuration structure

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID UserCfgInit(RTMP_ADAPTER *pAd)
{
	UINT i;
	UINT key_index, bss_index;
#ifdef GREENAP_SUPPORT	
    struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> UserCfgInit\n"));

	/*init wifi profile*/
	wpf_init(pAd);

	pAd->IndicateMediaState = NdisMediaStateDisconnected;

	/* part I. intialize common configuration */
	pAd->CommonCfg.BasicRateBitmap = 0xF;
	pAd->CommonCfg.BasicRateBitmapOld = 0xF;

#ifdef TX_POWER_CONTROL_SUPPORT
	os_zero_mem(pAd->CommonCfg.cPowerUpCckOfdm, sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpHt20, sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpHt40, sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht20, sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht40, sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht80, sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
	os_zero_mem(pAd->CommonCfg.cPowerUpVht160, sizeof(CHAR) * DBDC_BAND_NUM * POWER_UP_CATEGORY_RATE_NUM);
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef LINK_TEST_SUPPORT

    /* state machine state flag */
    pAd->ucLinkBwState           = TX_UNDEFINED_BW_STATE;
    pAd->ucRxStreamState         = TX_UNDEFINED_RXSTREAM_STATE;
    pAd->ucRxFilterstate         = TX_UNDEFINED_RXFILTER_STATE;
	pAd->ucTxCsdState            = TX_UNDEFINED_CSD_STATE;
	pAd->ucTxPwrBoostState       = TX_UNDEFINED_POWER_STATE;

	/* BW Control Paramter */
	pAd->fgBwInfoUpdate          = FALSE;

    /* Rx Control Parameter */
    pAd->ucRxTestTimeoutCount    =     0;
    pAd->u4TempRxCount           =     0;
    pAd->ucLinkRssiTh            =    20;
    pAd->ucRssiTh                =    10;
    pAd->ucHighPowerRssiTh       =    25;
    pAd->ucLowPowerRssiTh        =    20;
    pAd->cLargePowerTh           =   -90;
    pAd->ucRxCountTh             =     5;
    pAd->ucTimeOutTh             =   200;  // 20s
    pAd->ucPerTh                 =    50;
    pAd->cNrRssiTh               =   -40;
    pAd->cChgTestPathTh          =   -30;

    /* ACR Control Parameter */
    pAd->ucRxFilterConfidenceCnt =     0;
    pAd->ucACRConfidenceCntTh    =    10;
    pAd->ucMaxInConfidenceCntTh  =    10;
    pAd->cMaxInRssiTh            =   -40;

    /* Tx Control Parameter */
    pAd->ucCmwCheckCount         =     0;
    pAd->ucCmwCheckCountTh       =    20;  // 2s
    pAd->fgCmwInstrumBack4T      = FALSE;
    pAd->ucRssiBalanceCount      =     0;
    pAd->ucRssiIBalanceCountTh   =   100;  //10s
    pAd->fgRssiBack4T            = FALSE;
    pAd->ucCableRssiTh           =    25;
    pAd->fgCmwLinkDone           = FALSE;
	pAd->fgApclientLinkUp        = FALSE;
    pAd->ucLinkCount             =     0;
    pAd->ucLinkCountTh           =    30;
    pAd->fgLinkRSSICheck         = FALSE;
    pAd->ucCmwChannelBand[BAND0] = CHANNEL_BAND_2G;
	pAd->ucCmwChannelBand[BAND1] = CHANNEL_BAND_5G;

	/* Tx Power Control Paramter */
	os_zero_mem(pAd->ucTxPwrUpTbl, sizeof(UINT8)*CMW_POWER_UP_RATE_NUM*4);
	
    /* manual command control function enable/disable flag */
    pAd->fgTxSpurEn              =  TRUE;
    pAd->fgNrFloatingEn          =  TRUE;
    pAd->fgACREn                 =  TRUE;
#endif /* LINK_TEST_SUPPORT */


    /* disable QA Effuse Write back status by default */
    pAd->fgQAEffuseWriteBack = FALSE;

    /* Disable EPA flag */
    pAd->fgEPA = FALSE;

#ifdef RF_LOCKDOWN
    /* Apply Cal-Free Effuse value by default */
    pAd->fgCalFreeApply = TRUE;
    pAd->RFlockTempIdx  =    0;
    pAd->CalFreeTempIdx =    0;
#endif /* RF_LOCKDOWN */



	for(key_index=0; key_index<SHARE_KEY_NUM; key_index++)
	{
		for(bss_index = 0; bss_index < MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM; bss_index++)
		{
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

	pAd->bLocalAdminMAC = FALSE;
	pAd->EepromAccess = FALSE;

	pAd->Antenna.word = 0;
	pAd->CommonCfg.BBPCurrentBW = BW_20;

#ifdef RTMP_MAC_PCI
#ifdef LED_CONTROL_SUPPORT
	pAd->LedCntl.LedIndicatorStrength = 0;
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_MAC_PCI */

	pAd->bAutoTxAgcA = FALSE;			/* Default is OFF*/
	pAd->bAutoTxAgcG = FALSE;			/* Default is OFF*/

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	pAd->TxPowerCtrl.bInternalTxALC = FALSE; /* Off by default */
	pAd->TxPowerCtrl.idxTxPowerTable = 0;
	pAd->TxPowerCtrl.idxTxPowerTable2 = 0;
#ifdef RTMP_TEMPERATURE_COMPENSATION
	pAd->TxPowerCtrl.LookupTableIndex = 0;
#endif /* RTMP_TEMPERATURE_COMPENSATION */
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

#ifdef THERMAL_PROTECT_SUPPORT
	pAd->force_one_tx_stream = FALSE;
	pAd->last_thermal_pro_temp = 0;
#endif /* THERMAL_PROTECT_SUPPORT */

	pAd->RfIcType = RFIC_2820;

	/* Init timer for reset complete event*/
	pAd->CommonCfg.CentralChannel = 1;
	pAd->bForcePrintTX = FALSE;
	pAd->bForcePrintRX = FALSE;
	pAd->bStaFifoTest = FALSE;
	pAd->bProtectionTest = FALSE;
	pAd->bHCCATest = FALSE;
	pAd->bGenOneHCCA = FALSE;
	pAd->CommonCfg.Dsifs = 10;      /* in units of usec */
	pAd->CommonCfg.TxPower = 100; /* mW*/
    pAd->CommonCfg.TxPowerPercentage[BAND0] = 0xffffffff; /* AUTO*/
#ifdef DBDC_MODE      
    pAd->CommonCfg.TxPowerPercentage[BAND1] = 0xffffffff; /* AUTO*/
#endif /* DBDC_MODE */
    pAd->CommonCfg.TxPowerDefault[BAND0] = 0xffffffff; /* AUTO*/
#ifdef DBDC_MODE  
    pAd->CommonCfg.TxPowerDefault[BAND1] = 0xffffffff; /* AUTO*/
#endif /* DBDC_MODE */
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; /* use Long preamble on TX by defaut*/
	pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	pAd->bDisableRtsProtect = FALSE;
	pAd->CommonCfg.UseBGProtection = 0;    /* 0: AUTO*/
	pAd->CommonCfg.bEnableTxBurst = TRUE; /* 0;    	*/
	pAd->CommonCfg.SavedPhyMode =0xff;
	pAd->CommonCfg.BandState = UNKNOWN_BAND;

	pAd->wmm_cw_min = 4;
	switch (pAd->OpMode)
	{
		case OPMODE_AP:
			pAd->wmm_cw_max = 6;
			break;
		case OPMODE_STA:
			pAd->wmm_cw_max = 10;
			break;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	os_zero_mem(&pAd->ApCfg.ACSCheckTime, sizeof(UINT32) * DBDC_BAND_NUM);
	os_zero_mem(&pAd->ApCfg.ACSCheckCount, sizeof(UINT32) * DBDC_BAND_NUM);
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	os_zero_mem(&pAd->AutoChSelCtrl, sizeof(AUTOCH_SEL_CTRL));
#endif/* CONFIG_AP_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	pAd->CommonCfg.CarrierDetect.delta = CARRIER_DETECT_DELTA;
	pAd->CommonCfg.CarrierDetect.div_flag = CARRIER_DETECT_DIV_FLAG;
	pAd->CommonCfg.CarrierDetect.criteria = CARRIER_DETECT_CRITIRIA;
	pAd->CommonCfg.CarrierDetect.threshold = CARRIER_DETECT_THRESHOLD;
	pAd->CommonCfg.CarrierDetect.recheck1 = CARRIER_DETECT_RECHECK_TIME;
	pAd->CommonCfg.CarrierDetect.CarrierGoneThreshold = CARRIER_GONE_TRESHOLD;
	pAd->CommonCfg.CarrierDetect.VGA_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Packet_End_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Rx_PE_Mask = CARRIER_DETECT_DEFAULT_MASK;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DFS_SUPPORT
	pAd->CommonCfg.RadarDetect.bDfsInit = FALSE;
#endif /* DFS_SUPPORT */

	pAd->Dot11_H.ChMovingTime = 65;

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
{
	UINT32 IdMbss;

	for(IdMbss=0; IdMbss<HW_BEACON_MAX_NUM; IdMbss++)
		UAPSD_INFO_INIT(&pAd->ApCfg.MBSSID[IdMbss].wdev.UapsdInfo);
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */
	pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = FALSE;
	/*pAd->CommonCfg.bCountryFlag = FALSE;*/
	//pAd->CommonCfg.TxStream = 0;
	//pAd->CommonCfg.RxStream = 0;

#ifdef DOT11_N_SUPPORT
	NdisZeroMemory(&pAd->CommonCfg.HtCapability, sizeof(pAd->CommonCfg.HtCapability));
	pAd->bBroadComHT = FALSE;
	pAd->CommonCfg.bRdg = FALSE;

#ifdef DOT11N_DRAFT3
	pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
	pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
	pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);

	pAd->CommonCfg.bBssCoexEnable = TRUE; /* by default, we enable this feature, you can disable it via the profile or ioctl command*/
	pAd->CommonCfg.BssCoexApCntThr = 0;
	pAd->CommonCfg.Bss2040NeedFallBack = 0;
#endif  /* DOT11N_DRAFT3 */

	pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

	pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_DISABLE;
	pAd->CommonCfg.BACapability.field.MpduDensity = 0;
	pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64; /*32;*/
	pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64; /*32;*/
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> UserCfgInit. BACapability = 0x%x\n", pAd->CommonCfg.BACapability.word));

	pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
	BATableInit(pAd, &pAd->BATable);

	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bHTProtect = 1;
	pAd->CommonCfg.bMIMOPSEnable = TRUE;
	pAd->CommonCfg.bBADecline = FALSE;
	pAd->CommonCfg.bDisableReordering = FALSE;

	if (pAd->MACVersion == 0x28720200)
		pAd->CommonCfg.TxBASize = 13; /*by Jerry recommend*/
	else
		pAd->CommonCfg.TxBASize = 7;

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
#endif /* DOT11_N_SUPPORT */

	pAd->CommonCfg.TxRate = RATE_6;
	pAd->CommonCfg.BeaconPeriod = 100;     /* in mSec*/

#ifdef STREAM_MODE_SUPPORT
	if (pAd->chipCap.FlgHwStreamMode)
	{
		pAd->CommonCfg.StreamMode = 3;
		pAd->CommonCfg.StreamModeMCS = 0x0B0B;
		NdisMoveMemory(&pAd->CommonCfg.StreamModeMac[0][0],
				BROADCAST_ADDR, MAC_ADDR_LEN);
	}
#endif /* STREAM_MODE_SUPPORT */

#ifdef TXBF_SUPPORT
	pAd->CommonCfg.ETxBfNoncompress = 0;
	pAd->CommonCfg.ETxBfIncapable = 0;
#endif /* TXBF_SUPPORT */

#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
	pAd->CommonCfg.lowTrafficThrd = 2;
	pAd->CommonCfg.TrainUpRule = 2; // 1;
	pAd->CommonCfg.TrainUpRuleRSSI = -70; // 0;
	pAd->CommonCfg.TrainUpLowThrd = 90;
	pAd->CommonCfg.TrainUpHighThrd = 110;
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */



#ifdef CFO_TRACK
#endif /* CFO_TRACK */

#ifdef DBG_CTRL_SUPPORT
	pAd->CommonCfg.DebugFlags = 0;
#endif /* DBG_CTRL_SUPPORT */

#ifdef MCAST_RATE_SPECIFIC
	pAd->CommonCfg.MCastPhyMode.word = pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word;
	pAd->CommonCfg.MCastPhyMode_5G.word = pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word;
#endif /* MCAST_RATE_SPECIFIC */

	/* WFA policy - disallow TH rate in WEP or TKIP cipher */
	pAd->CommonCfg.HT_DisallowTKIP = TRUE;

	/* Frequency for rate adaptation */
	pAd->ra_interval = DEF_RA_TIME_INTRVAL;
	pAd->ra_fast_interval = DEF_QUICK_RA_TIME_INTERVAL;

#ifdef AGS_SUPPORT
	if (pAd->rateAlg == RATE_ALG_AGS)
		pAd->ra_fast_interval = AGS_QUICK_RA_TIME_INTERVAL;
#endif /* AGS_SUPPORT */

	/* Tx Sw queue length setting */
	pAd->TxSwQMaxLen = MAX_PACKETS_IN_QUEUE;
	// TODO: shiang-usw, we may need to move this function to better place.
#ifndef MT7622
	rtmp_tx_swq_init(pAd); //wilsonl, duplicated in RTMPInitTxRxRingMemory
#endif

	pAd->CommonCfg.bRalinkBurstMode = FALSE;


	/* global variables mXXXX used in MAC protocol state machines*/
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);

	/* PHY specification*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  /* CCK use LONG preamble*/


	/* Default for extra information is not valid*/
	pAd->ExtraInfo = EXTRA_INFO_CLEAR;

#ifdef CONFIG_AP_SUPPORT
	/* Default Config change flag*/
	pAd->bConfigChanged = FALSE;
#ifdef GREENAP_SUPPORT
    greenap_init(greenap);
#endif /* GREENAP_SUPPORT */	
#endif

	/*
		part III. AP configurations
	*/
#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_APCLI_SUPPORT || RT_CFG80211_P2P_SUPPORT */
	{
		/* Set MBSS Default Configurations*/
        UCHAR j;

		pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
		for(j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		{
			BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[j];
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[j].wdev;

#ifdef DOT11V_WNM_SUPPORT
			WNM_CONFIG *wnm_cfg = &pAd->ApCfg.MBSSID[j].WnmCfg;
			wnm_cfg->bDot11vWNM_BSSEnable = 1;
#endif /* DOT11V_WNM_SUPPORT */	

#ifdef SPECIFIC_TX_POWER_SUPPORT
			if (IS_RT6352(pAd))
				mbss->TxPwrAdj = -1;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef DOT1X_SUPPORT
			/* PMK cache setting*/
			mbss->PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute)*/

			/* dot1x related per BSS */
			mbss->wdev.SecConfig.radius_srv_num = 0;
			mbss->wdev.SecConfig.NasIdLen = 0;
			mbss->wdev.SecConfig.IEEE8021X = FALSE;
			mbss->wdev.SecConfig.PreAuth = FALSE;
#ifdef RADIUS_MAC_ACL_SUPPORT
			NdisZeroMemory(&mbss->wdev.SecConfig.RadiusMacAuthCache, sizeof(RT_802_11_RADIUS_ACL));
			/* Default Timeout Value for 1xDaemon Radius ACL Cache */	
			mbss->wdev.SecConfig.RadiusMacAuthCacheTimeout = 30;
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

			/* VLAN related */
			mbss->wdev.VLAN_VID = 0;

			/* Default MCS as AUTO*/
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			/* Default is zero. It means no limit.*/
			mbss->MaxStaNum = 0;
			mbss->StaCount = 0;

#ifdef WSC_AP_SUPPORT
			mbss->WscSecurityMode = 0xff;
			{
				PWSC_CTRL pWscControl;
				INT idx;
#ifdef WSC_V2_SUPPORT
				PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
				/*
					WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize
					and MLME Initialize is called before UserCfgInit.
				*/
				pWscControl = &mbss->WscControl;
				NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
				NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
				pWscControl->WscMode = 1;
				pWscControl->WscConfStatus = 1;
#ifdef WSC_V2_SUPPORT
				pWscControl->WscConfigMethods= 0x238C;
#else
				pWscControl->WscConfigMethods= 0x0084;
#endif /* WSC_V2_SUPPORT */
#ifdef WSC_NFC_SUPPORT
				pWscControl->WscConfigMethods |= WPS_CONFIG_METHODS_ENT;
#endif /* WSC_NFC_SUPPORT */
				pWscControl->RegData.ReComputePke = 1;
				pWscControl->lastId = 1;
				/* pWscControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | j); */
				pWscControl->pAd = pAd;
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				pAd->CommonCfg.WscPBCOverlap = FALSE;
				pWscControl->WscConfMode = 0;
				pWscControl->WscStatus = 0;
				pWscControl->WscState = 0;
				pWscControl->WscPinCode = 0;
				pWscControl->WscLastPinFromEnrollee = 0;
				pWscControl->WscEnrollee4digitPinCode = FALSE;
				pWscControl->WscEnrolleePinCode = 0;
				pWscControl->WscSelReg = 0;
				pWscControl->WscUseUPnP = 0;
				pWscControl->bWCNTest = FALSE;
				pWscControl->WscKeyASCII = 0; /* default, 0 (64 Hex) */

				/*
					Enrollee 192 random bytes for DH key generation
				*/
				for (idx = 0; idx < 192; idx++)
					pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

				/* Enrollee Nonce, first generate and save to Wsc Control Block*/
				for (idx = 0; idx < 16; idx++)
					pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);

				NdisZeroMemory(&pWscControl->WscDefaultSsid, sizeof(NDIS_802_11_SSID));
				NdisZeroMemory(&pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
				NdisZeroMemory(&pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
				pWscControl->bCheckMultiByte = FALSE;
				pWscControl->bWscAutoTigeer = FALSE;
				pWscControl->bWscFragment = FALSE;
				pWscControl->WscFragSize = 128;
				initList(&pWscControl->WscPeerList);
				NdisAllocateSpinLock(pAd, &pWscControl->WscPeerListSemLock);
				pWscControl->PinAttackCount = 0;
				pWscControl->bSetupLock = FALSE;
#ifdef WSC_V2_SUPPORT
				pWscV2Info = &pWscControl->WscV2Info;
				pWscV2Info->bWpsEnable = TRUE;
				pWscV2Info->ExtraTlv.TlvLen = 0;
				pWscV2Info->ExtraTlv.TlvTag = 0;
				pWscV2Info->ExtraTlv.pTlvData = NULL;
				pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
				pWscV2Info->bEnableWpsV2 = TRUE;
				pWscControl->SetupLockTime = WSC_WPS_AP_SETUP_LOCK_TIME;
				pWscControl->MaxPinAttack = WSC_WPS_AP_MAX_PIN_ATTACK;
#ifdef WSC_NFC_SUPPORT
				pWscControl->NfcPasswdCaculate = 2;
#endif /* WSC_NFC_SUPPORT */

#endif /* WSC_V2_SUPPORT */
			}
#endif /* WSC_AP_SUPPORT */


			for(i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
			 mbss->wdev.bcn_buf.TimBitmaps[i] = 0;

#if defined(MWDS) && defined(IGMP_SNOOP_SUPPORT)
			mbss->IGMPPeriodicQuerySent = FALSE;
			mbss->MLDPeriodicQuerySent = FALSE;
			mbss->IgmpQueryHoldTick = 0;
			mbss->IgmpQueryHoldTickChanged = FALSE;
			mbss->MldQueryHoldTick = 0;
			mbss->MldQueryHoldTickChanged = FALSE;
			mbss->MldQryChkSum = 0x0;
			NdisZeroMemory(&mbss->ipv6LinkLocalSrcAddr[0], 16);			
#endif
		}

#ifdef DOT1X_SUPPORT
		/* PMK cache setting*/
		NdisZeroMemory(&pAd->ApCfg.PMKIDCache, sizeof(NDIS_AP_802_11_PMKID));
#endif /* DOT1X_SUPPORT */

		pAd->ApCfg.DtimCount  = 0;
		pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;

		pAd->ApCfg.ErpIeContent = 0;

		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_AGEOUT_TIME;

		pAd->ApCfg.BANClass3Data = FALSE;
#ifdef IDS_SUPPORT
		/* Default disable IDS threshold and reset all IDS counters*/
		pAd->ApCfg.IdsEnable = FALSE;
		pAd->ApCfg.AuthFloodThreshold = 0;
		pAd->ApCfg.AssocReqFloodThreshold = 0;
		pAd->ApCfg.ReassocReqFloodThreshold = 0;
		pAd->ApCfg.ProbeReqFloodThreshold = 0;
		pAd->ApCfg.DisassocFloodThreshold = 0;
		pAd->ApCfg.DeauthFloodThreshold = 0;
		pAd->ApCfg.EapReqFloodThreshold = 0;
		RTMPClearAllIdsCounter(pAd);
#endif /* IDS_SUPPORT */

#ifdef WDS_SUPPORT
		APWdsInitialize(pAd);
#endif /* WDS_SUPPORT*/

#ifdef WSC_INCLUDED
		pAd->WriteWscCfgToDatFile = 0xFF;
		pAd->WriteWscCfgToAr9DatFile = FALSE;
#ifdef CONFIG_AP_SUPPORT
		pAd->bWscDriverAutoUpdateCfg = TRUE;
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef APCLI_SUPPORT
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = FALSE;
#ifdef APCLI_CERT_SUPPORT
		pAd->bApCliCertTest = FALSE;
#endif /* APCLI_CERT_SUPPORT */
		pAd->ApCfg.ApCliNum = MAX_APCLI_NUM;
		for(j = 0; j < MAX_APCLI_NUM; j++)
		{
			APCLI_STRUCT *apcli_entry = &pAd->ApCfg.ApCliTab[j];
			struct wifi_dev *wdev = &apcli_entry->wdev;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			apcli_entry->AutoConnectFlag = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			apcli_entry->wdev.UapsdInfo.bAPSDCapable = FALSE;
			apcli_entry->bBlockAssoc = FALSE;

#ifdef APCLI_CONNECTION_TRIAL
			apcli_entry->TrialCh = 0;//if the channel is 0, AP will connect the rootap is in the same channel with ra0.
#endif // APCLI_CONNECTION_TRIAL
#ifdef APCLI_CERT_SUPPORT
			apcli_entry->NeedFallback = FALSE;
#endif /* APCLI_CERT_SUPPORT */
#ifdef WSC_AP_SUPPORT
            apcli_entry->WscControl.WscApCliScanMode = TRIGGER_FULL_SCAN;
#endif /* WSC_AP_SUPPORT */
		}
#endif /* APCLI_SUPPORT */
		pAd->ApCfg.EntryClientCount = 0;
#if defined(MWDS) && defined(IGMP_SNOOP_SUPPORT)
		pAd->bIGMPperiodicQuery = TRUE;
		pAd->IgmpQuerySendTick = QUERY_SEND_PERIOD;
		pAd->bMLDperiodicQuery = TRUE;
		pAd->MldQuerySendTick = QUERY_SEND_PERIOD;
#endif
	}

#ifdef DYNAMIC_VGA_SUPPORT
	if (IS_MT76x2(pAd)) {
		pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = FALSE;
		pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 800;
		pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 10;
	}
#endif /* DYNAMIC_VGA_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */



	/*
		part IV. others
	*/

	/* dynamic BBP R66:sensibity tuning to overcome background noise*/
	pAd->BbpTuning.bEnable = TRUE;
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = TRUE;

	/* Also initial R66CurrentValue, RTUSBResumeMsduTransmission might use this value.*/
	/* if not initial this value, the default value will be 0.*/
	pAd->BbpTuning.R66CurrentValue = 0x38;

	/* initialize MAC table and allocate spin lock*/
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
	InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
	NdisAllocateSpinLock(pAd, &pAd->MacTabLock);

	/*RTMPInitTimer(pAd, &pAd->RECBATimer, RECBATimerTimeout, pAd, TRUE);*/
	/*RTMPSetTimer(&pAd->RECBATimer, REORDER_EXEC_INTV);*/


	pAd->CommonCfg.bWiFiTest = FALSE;
#ifdef RTMP_MAC_PCI
    pAd->bPCIclkOff = FALSE;
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

#ifdef DOT11R_FT_SUPPORT
	FT_CfgInitial(pAd);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef TXRX_SW_ANTDIV_SUPPORT
		pAd->chipCap.bTxRxSwAntDiv = FALSE;
#endif /* TXRX_SW_ANTDIV_SUPPORT */


#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
	{
		BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[i];

		if (pAd->ProbeRespIE[i].pIe)
			pBssEntry->pVarIeFromProbRsp = pAd->ProbeRespIE[i].pIe;
		else
			pBssEntry->pVarIeFromProbRsp = NULL;
	}
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */

#ifdef WSC_INCLUDED
	NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
	pAd->CommonCfg.WscPBCOverlap = FALSE;
#endif /* WSC_INCLUDED */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		if (wf_fwd_set_cb_num != NULL)
			wf_fwd_set_cb_num(PACKET_BAND_CB, RECV_FROM_CB);

#endif /* CONFIG_WIFI_PKT_FWD */


#ifdef RMTP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
	VideoConfigInit(pAd);
#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RMTP_RBUS_SUPPORT */




#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	pAd->WOW_Cfg.bEnable = FALSE;
	pAd->WOW_Cfg.bWOWFirmware = FALSE;	/* load normal firmware */
	pAd->WOW_Cfg.bInBand = TRUE;		/* use in-band signal */
	pAd->WOW_Cfg.nSelectedGPIO = 2;
	pAd->WOW_Cfg.nDelay = 3; /* (3+1)*3 = 12 sec */
	pAd->WOW_Cfg.nHoldTime = 1000;	// unit is us
	pAd->WOW_Cfg.nWakeupInterface = pAd->chipCap.nWakeupInterface; //WOW_WAKEUP_BY_USB; /* USB as default */
	pAd->WOW_Cfg.bGPIOHighLow = WOW_GPIO_LOW_TO_HIGH;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WOW Enable %d, WOWFirmware %d\n", pAd->WOW_Cfg.bEnable, pAd->WOW_Cfg.bWOWFirmware));
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */


	/* 802.11H and DFS related params*/
	pAd->Dot11_H.CSCount = 0;
	pAd->Dot11_H.CSPeriod = 10;

	pAd->CommonCfg.ChGrpEn= 0;
	NdisZeroMemory(pAd->CommonCfg.ChGrpChannelList,(MAX_NUM_OF_CHANNELS)*sizeof(UCHAR));
	pAd->CommonCfg.ChGrpChannelNum= 0;
	
#ifdef MT_DFS_SUPPORT
	DfsParamInit(pAd);//Jelly20150311
#endif

	pAd->CommonCfg.ThermalRecalMode = 1;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pAd->Dot11_H.RDMode = RD_SILENCE_MODE;
#endif

    pAd->Dot11_H.wdev_count = 0;
	pAd->Dot11_H.ChMovingTime = 65;
	pAd->Dot11_H.bDFSIndoor = 1;



	PartialScanInit(pAd);

#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	for (i = 0; i < MAX_APCLI_NUM; i++) {	
		pAd->ApCfg.ApCliAutoConnectType[i] = TRIGGER_SCAN_BY_USER; /* User Trigger SCAN by default */
		pAd->ApCfg.ApCliAutoConnectRunning[i] = FALSE;
	}
	pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->ApCfg.bPartialScanEnable[i] = FALSE;
		pAd->ApCfg.bPartialScanning[i] = FALSE;
	}
#endif /* APCLI_SUPPORT */

#ifdef DOT11_VHT_AC
#ifdef WFA_VHT_PF
	pAd->force_amsdu = FALSE;
	pAd->force_noack = FALSE;
	pAd->force_vht_op_mode = FALSE;
	pAd->vht_force_sgi = FALSE;
	pAd->vht_force_tx_stbc = FALSE;
	pAd->CommonCfg.vht_nss_cap = pAd->chipCap.max_nss;
	pAd->CommonCfg.vht_mcs_cap = pAd->chipCap.max_vht_mcs;
	pAd->CommonCfg.vht_cent_ch2 = 0; // we don't support 160MHz BW now!
#endif /* WFA_VHT_PF */
#endif /* DOT11_VHT_AC */

#ifdef CONFIG_FPGA_MODE
	pAd->fpga_ctl.fpga_on = 0x0;
	pAd->fpga_ctl.tx_kick_cnt = 0;
	pAd->fpga_ctl.tx_data_phy = 0;
	pAd->fpga_ctl.tx_data_ldpc = 0;
	pAd->fpga_ctl.tx_data_mcs = 0;
	pAd->fpga_ctl.tx_data_bw = 0;
	pAd->fpga_ctl.tx_data_gi = 0;
	pAd->fpga_ctl.rx_data_phy = 0;
	pAd->fpga_ctl.rx_data_ldpc = 0;
	pAd->fpga_ctl.rx_data_mcs = 0;
	pAd->fpga_ctl.rx_data_bw = 0;
	pAd->fpga_ctl.rx_data_gi = 0;
#ifdef CAPTURE_MODE
	pAd->fpga_ctl.cap_type = 2; /* CAP_MODE_ADC8; */
	pAd->fpga_ctl.cap_trigger = 2; /* CAP_TRIGGER_AUTO; */
	pAd->fpga_ctl.trigger_offset = 200;
	pAd->fpga_ctl.cap_support = 0;
#endif /* CAPTURE_MODE */

#ifdef DMA_SCH_SUPPORT
	pAd->fpga_ctl.dma_mode = DMA_SCH_LMAC;
#endif /* DMA_SCH_SUPPORT */

#if defined(MT7615_FPGA) || defined(MT7622_FPGA)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		pAd->fpga_ctl.no_bcn = 1;
#endif /* defined(MT7615_FPGA) || defined(MT7622_FPGA) */

#endif /* CONFIG_FPGA_MODE */

#ifdef MICROWAVE_OVEN_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;
	else
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;
	pAd->CommonCfg.MO_Cfg.nFalseCCATh = MO_FALSE_CCA_TH;
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
	pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = TRUE;
	pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 600;
	pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 100;
#endif /* DYNAMIC_VGA_SUPPORT */



#ifdef DOT11_VHT_AC
	pAd->CommonCfg.bNonVhtDisallow = FALSE;
#endif /* DOT11_VHT_AC */


#ifdef MT_MAC
    pAd->chipCap.TmrEnable = 0;
	RTMP_SEM_EVENT_INIT(&(pAd->IndirectUpdateLock), &pAd->RscSemMemList);
	pAd->PSEWatchDogEn = 0;
	pAd->RxPseCheckTimes = 0;
	pAd->PSEResetCount = 0;
	pAd->PSETriggerType1Count = 0;
	pAd->PSETriggerType1Count = 0;
	pAd->PSEResetFailCount = 0;
#endif

#ifdef RTMP_MAC_PCI
	pAd->PDMAWatchDogEn = 0;
	pAd->TxDMAResetCount = 0;
	pAd->RxDMAResetCount = 0;
	pAd->TxDMACheckTimes = 0;
	pAd->RxDMACheckTimes = 0;
	pAd->PDMAResetFailCount = 0;
#endif

#ifdef CONFIG_MULTI_CHANNEL
	pAd->Mlme.bStartMcc = FALSE;
	pAd->Mlme.bStartScc = FALSE;
	pAd->Mlme.channel_1st_staytime = 40;
	pAd->Mlme.channel_2nd_staytime = 40;
	pAd->Mlme.switch_idle_time = 10;
	pAd->Mlme.null_frame_count = 1;
	pAd->Mlme.channel_1st_bw = 0;
	pAd->Mlme.channel_2nd_bw = 0;

#endif /* CONFIG_MULTI_CHANNEL */

    pAd->bPS_Retrieve =1;

	pAd->CommonCfg.bTXRX_RXV_ON = 0;

    pAd->parse_rxv_stat_enable = 0;
    pAd->AccuOneSecRxBand0FcsErrCnt = 0;
    pAd->AccuOneSecRxBand0MdrdyCnt = 0;
    pAd->AccuOneSecRxBand1FcsErrCnt = 0; 
    pAd->AccuOneSecRxBand1MdrdyCnt = 0;

    pAd->CommonCfg.ManualTxop = 0;

    pAd->CommonCfg.ManualTxopThreshold = 10; // Mbps

    pAd->CommonCfg.ManualTxopUpBound = 20; // Ratio

    pAd->CommonCfg.ManualTxopLowBound = 5; // Ratio

#ifdef CONFIG_AP_SUPPORT
#ifdef VOW_SUPPORT
    /* VOW control */
    pAd->vow_cfg.en_bw_ctrl = FALSE;
    pAd->vow_cfg.en_bw_refill = TRUE;
    pAd->vow_cfg.en_airtime_fairness = TRUE;
    pAd->vow_cfg.en_txop_no_change_bss = TRUE;
    pAd->vow_cfg.dbdc0_search_rule = VOW_WMM_SET_FIRST;
    pAd->vow_cfg.dbdc1_search_rule = VOW_WMM_SET_FIRST;
    pAd->vow_cfg.refill_period = VOW_REFILL_PERIOD_32US; //32us
    pAd->vow_cfg.per_bss_enable = 0xFFFF; //enable all 16 group
    pAd->vow_cfg.sta_max_wait_time = VOW_DEF_STA_MAX_WAIT_TIME;
    pAd->vow_cfg.group_max_wait_time = VOW_DEF_BSS_MAX_WAIT_TIME;


    pAd->vow_avg_num = 30; //20 packets

	/* VoW bad node detection */
	pAd->vow_badnode.bn_en = TRUE;

    /* group(BSS) configuration */
    for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
    {
        pAd->vow_bss_cfg[i].min_rate = 10; //Mbps
        pAd->vow_bss_cfg[i].max_rate = 30; //Mbps
        pAd->vow_bss_cfg[i].min_airtime_ratio = 5; //%
        pAd->vow_bss_cfg[i].max_airtime_ratio = 10; //%
        pAd->vow_bss_cfg[i].min_ratebucket_size = VOW_DEF_MIN_RATE_BUCKET_SIZE;
        pAd->vow_bss_cfg[i].max_ratebucket_size = VOW_DEF_MAX_RATE_BUCKET_SIZE;
        pAd->vow_bss_cfg[i].max_backlog_size = VOW_DEF_BACKLOG_SIZE;
        pAd->vow_bss_cfg[i].min_airtimebucket_size = VOW_DEF_MIN_AIRTIME_BUCKET_SIZE;
        pAd->vow_bss_cfg[i].max_airtimebucket_size = VOW_DEF_MAX_AIRTIME_BUCKET_SIZE;
        pAd->vow_bss_cfg[i].max_wait_time = VOW_DEF_MAX_WAIT_TIME;
        pAd->vow_bss_cfg[i].dwrr_quantum = VOW_DEF_BSS_DWRR_QUANTUM;
        pAd->vow_bss_cfg[i].min_rate_token = vow_convert_rate_token(pAd, VOW_MIN, i);
        pAd->vow_bss_cfg[i].max_rate_token = vow_convert_rate_token(pAd, VOW_MAX, i);
        pAd->vow_bss_cfg[i].min_airtime_token = vow_convert_airtime_token(pAd, VOW_MIN, i);
        pAd->vow_bss_cfg[i].max_airtime_token = vow_convert_airtime_token(pAd, VOW_MAX, i);
        pAd->vow_bss_cfg[i].bw_on = FALSE;
        pAd->vow_bss_cfg[i].at_on = TRUE;
    }

    /* per station default configuration */
    for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
    {
        pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BK] = VOW_STA_DWRR_IDX2; //4ms
        pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BE] = VOW_STA_DWRR_IDX2; //4ms
        pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VI] = VOW_STA_DWRR_IDX1; //3ms
        pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VO] = VOW_STA_DWRR_IDX0; //1.5ms
        pAd->vow_sta_cfg[i].group = 0;
        pAd->vow_sta_cfg[i].wmm_idx = VOW_WMM_SET0;
        pAd->vow_sta_cfg[i].ac_change_rule = VOW_DEFAULT_AC;
        pAd->vow_sta_cfg[i].paused = FALSE;
    }

    /* station DWRR quantum value */
    pAd->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
    pAd->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
    pAd->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
    pAd->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
    pAd->vow_cfg.vow_sta_dwrr_quantum[4] = VOW_STA_DWRR_QUANTUM4;
    pAd->vow_cfg.vow_sta_dwrr_quantum[5] = VOW_STA_DWRR_QUANTUM5;
    pAd->vow_cfg.vow_sta_dwrr_quantum[6] = VOW_STA_DWRR_QUANTUM6;
    pAd->vow_cfg.vow_sta_dwrr_quantum[7] = VOW_STA_DWRR_QUANTUM7;

	/* for fast round robin */
	pAd->vow_sta_frr_quantum = 2;	/* 0.5ms */

    /* RX airtime */
    pAd->vow_rx_time_cfg.rx_time_en = TRUE;
    pAd->vow_rx_time_cfg.ed_offset = VOW_DEF_ED_OFFSET;
    pAd->vow_rx_time_cfg.obss_backoff = VOW_DEF_ABAND_OBSS_BACKOFF;
    pAd->vow_rx_time_cfg.non_qos_backoff = VOW_DEF_NON_QOS_BACKOFF;

    /*FIXME: depends on real WMM set mapping */
    for (i = 0; i < VOW_MAX_WMM_SET_NUM; i++)
    {

        pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_BK] = VOW_DEF_ABAND_BK_BACKOFF;
        pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_BE] = VOW_DEF_ABAND_BE_BACKOFF;
        pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_VI] = VOW_DEF_ABAND_VI_BACKOFF;
        pAd->vow_rx_time_cfg.wmm_backoff[i][WMM_AC_VO] = VOW_DEF_ABAND_VO_BACKOFF;

        pAd->vow_rx_time_cfg.wmm_backoff_sel[i] = VOW_WMM_ONE2ONE_MAPPING;
    }

    /*FIXME: depends on real WMM set mapping */
    pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK] = VOW_DEF_ABAND_BK_BACKOFF;
    pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE] = VOW_DEF_ABAND_BE_BACKOFF;
    pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI] = VOW_DEF_ABAND_VI_BACKOFF;
    pAd->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO] = VOW_DEF_ABAND_VO_BACKOFF;

    /*FIXME: depends on real WMM set mapping */
    pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK] = VOW_DEF_ABAND_BK_BACKOFF;
    pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE] = VOW_DEF_ABAND_BE_BACKOFF;
    pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI] = VOW_DEF_ABAND_VI_BACKOFF;
    pAd->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO] = VOW_DEF_ABAND_VO_BACKOFF;

    /*FIXME: depends on real BSS mapping */
    for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
    {
        pAd->vow_rx_time_cfg.bssid2wmm_set[i] = VOW_WMM_SET0;   /* default WMM SET0 */
    }

#endif /* VOW_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
    pAd->ApCfg.bRoamingEnhance= FALSE;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef RED_SUPPORT
	pAd->red_en = TRUE;
#endif /* RED_SUPPORT */
	pAd->cp_support = 2;

#ifdef INTERNAL_CAPTURE_SUPPORT    
    pAd->IcapMode = 0;
    pAd->WifiSpectrumDataCounter = 0;
    pAd->Src_IQ = "/tmp/WifiSpectrum_IQ.txt";
    pAd->Src_Gain = "/tmp/WifiSpectrum_LNA_LPF.txt";
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef PA_TRIM_SUPPORT
    pAd->CalFileOffset = 0;
#endif /* PA_TRIM_SUPPORT */


 //=====================================================
 
#ifdef SMART_CARRIER_SENSE_SUPPORT
    for (i=0; i<DBDC_BAND_NUM; i++)
    {
        pAd->SCSCtrl.SCSEnable[i] = SCS_DISABLE;
        pAd->SCSCtrl.SCSTrafficThreshold[i] = TriggerTrafficeTh; /* 2M */
        pAd->SCSCtrl.SCSStatus[i] = PD_BLOCKING_OFF;
        pAd->SCSCtrl.OneSecTxByteCount[i] = 0;
        pAd->SCSCtrl.OneSecRxByteCount[i] = 0;
	pAd->SCSCtrl.CckPdBlkTh[i] = PdBlkCckThDefault;
	pAd->SCSCtrl.OfdmPdBlkTh[i] = PdBlkOfmdThDefault;	
		pAd->SCSCtrl.SCSThTolerance[i] = ThTolerance;
        pAd->SCSCtrl.SCSMinRssiTolerance[i] = MinRssiTolerance;
        pAd->SCSCtrl.OfdmPdSupport[i] = TRUE;        
	pAd->SCSCtrl.CckFalseCcaUpBond[i] = FalseCcaUpBondDefault;
	pAd->SCSCtrl.CckFalseCcaLowBond[i] = FalseCcaLowBondDefault;
	pAd->SCSCtrl.OfdmFalseCcaUpBond[i] = FalseCcaUpBondDefault;
	pAd->SCSCtrl.OfdmFalseCcaLowBond[i] = FalseCcaLowBondDefault;
	pAd->SCSCtrl.CckFixedRssiBond[i] = CckFixedRssiBondDefault;
	pAd->SCSCtrl.OfdmFixedRssiBond[i] = OfdmFixedRssiBondDefault;	
    }    
    pAd->SCSCtrl.SCSEnable[DBDC_BAND0] = SCS_ENABLE;
#endif /* SMART_CARRIER_SENSE_SUPPORT */

        for (i=0; i<DBDC_BAND_NUM; i++)
        {
                pAd->CCI_ACI_TxOP_Value[i] = 0;
        }    
        pAd->g_mode_txop_wdev = NULL;
        pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE;

        pAd->MUMIMO_TxOP_Value = 0;

	pAd->txop_ctl.multi_client_nums = 0;
	pAd->txop_ctl.cur_wdev = NULL;
	pAd->txop_ctl.multi_cli_txop_running = FALSE;
	pAd->txop_ctl.multi_client_nums_2g = 0;
	pAd->txop_ctl.cur_wdev_2g = NULL;
	pAd->txop_ctl.multi_cli_txop_2g_running = FALSE;


#ifdef PKT_BUDGET_CTRL_SUPPORT
	pAd->pbc_bound[PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
	pAd->pbc_bound[PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
	pAd->pbc_bound[PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
	pAd->pbc_bound[PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
	pAd->pbc_bound[PBC_AC_MGMT] = PBC_WMM_UP_DEFAULT_MGMT;
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef TX_AGG_ADJUST_WKR
	pAd->TxAggAdjsut = TRUE;
#endif /* TX_AGG_ADJUST_WKR */

#ifdef HTC_DECRYPT_IOT
	pAd->HTC_ICV_Err_TH = 5;
#endif /* HTC_DECRYPT_IOT */
#ifdef DHCP_UC_SUPPORT
	pAd->DhcpUcEnable = FALSE; 
#endif /* DHCP_UC_SUPPORT */
	for (i=0; i<DBDC_BAND_NUM; i++)
    	{
    		pAd->CommonCfg.ucEDCCACtrl[i] = TRUE; //EDCCA default is ON.
	}

#ifdef AIR_MONITOR
	pAd->MntRuleBitMap = DEFAULT_MNTR_RULE;
#endif

#ifdef STA_FORCE_ROAM_SUPPORT
	load_froam_defaults(pAd);
#endif

	pAd->cn_cnt = 0xFF;

#ifdef LINUX_NET_TXQ_SUPPORT
	pAd->tx_net_queue_len = LINUX_DEF_TX_QUEUE_LENGTH;/* for veriwave test */
#endif /* LINUX_NET_TXQ_SUPPORT */

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- UserCfgInit\n"));
}


/* IRQL = PASSIVE_LEVEL*/
UCHAR BtoH(RTMP_STRING ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');        /* Handle numerals*/
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  /* Handle capitol hex digits*/
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  /* Handle small hex digits*/
	return(255);
}


/*
	FUNCTION: AtoH(char *, UCHAR *, int)

	PURPOSE:  Converts ascii string to network order hex

	PARAMETERS:
		src    - pointer to input ascii string
		dest   - pointer to output hex
		destlen - size of dest

	COMMENTS:

		2 ascii bytes make a hex byte so must put 1st ascii byte of pair
		into upper nibble and 2nd ascii byte of pair into lower nibble.

	IRQL = PASSIVE_LEVEL
*/
void AtoH(RTMP_STRING *src, PUCHAR dest, int destlen)
{
	RTMP_STRING *srcptr;
	PUCHAR destTemp;

	srcptr = src;
	destTemp = (PUCHAR) dest;

	while(destlen--)
	{
		*destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
		*destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
		destTemp++;
	}
}


/*
========================================================================
Routine Description:
	Add a timer to the timer list.

Arguments:
	pAd				- WLAN control block pointer
	pRsc			- the OS resource

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_TimerListAdd(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;


	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	while(1)
	{
		if (pObj == NULL)
			break;
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
			return;
		pObj = pObj->pNext;
	}

	/* allocate a timer record entry */
	os_alloc_mem(NULL, (UCHAR **)&(pObj), sizeof(LIST_RESOURCE_OBJ_ENTRY));
	if (pObj == NULL)
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: alloc timer obj fail!\n", __FUNCTION__));
		return;
	}
	else
	{
		pObj->pRscObj = pRsc;
		insertTailList(pRscList, (RT_LIST_ENTRY *)pObj);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: add timer obj %lx!\n", __FUNCTION__, (ULONG)pRsc));
	}
}


VOID RTMP_TimerListRelease(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RT_LIST_ENTRY *pListEntry;

	pListEntry = pRscList->pHead;
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;

	while (pObj)
	{
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
		{
			pListEntry = (RT_LIST_ENTRY *)pObj;
			break;
		}

		pListEntry = pListEntry->pNext;
		pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;
	}

	if (pListEntry)
	{
		delEntryList(pRscList, pListEntry);

		/* free a timer record entry */
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: release timer obj %lx!\n", __FUNCTION__, (ULONG)pRsc));
		os_free_mem(pObj);
	}
}



/*
* Show Timer Information
*/
VOID RTMPShowTimerList(RTMP_ADAPTER *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	RALINK_TIMER_STRUCT *pTimer;

	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Timer List Size:%d\n",pRscList->size));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("=====================================\n"));
	
	while(1)
	{
		if (pObj == NULL)
			break;

		pTimer = (RALINK_TIMER_STRUCT*)pObj->pRscObj;
		pObj = pObj->pNext;
		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Valid:%d\n",pTimer->Valid));		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("pObj:%lx\n",(ULONG)pTimer));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("PeriodicType:%d\n",pTimer->PeriodicType));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Repeat:%d\n",pTimer->Repeat));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("State:%d\n",pTimer->State));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("TimerValue:%ld\n",pTimer->TimerValue));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("timer_lock:%lx\n",(ULONG)pTimer->timer_lock));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("pCaller:%pS\n",pTimer->pCaller));		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("=====================================\n"));
	}
}



/*
========================================================================
Routine Description:
	Cancel all timers in the timer list.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_AllTimerListRelease(RTMP_ADAPTER *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj, *pObjOld;
	BOOLEAN Cancel;
	RALINK_TIMER_STRUCT *pTimer;

	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s: Size=%d\n",__FUNCTION__,pRscList->size));
	while(1)
	{
		if (pObj == NULL)
			break;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Cancel timer obj %lx!\n", __FUNCTION__, (ULONG)(pObj->pRscObj)));
		pObjOld = pObj;
		pObj = pObj->pNext;
		pTimer = (RALINK_TIMER_STRUCT*)pObjOld->pRscObj;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s: Timer is allocated by %pS,Valid:%d,Lock:%lx,State:%d\n",__FUNCTION__,
			pTimer->pCaller,pTimer->Valid,(ULONG)pTimer->timer_lock,pTimer->State));
		RTMPReleaseTimer(pObjOld->pRscObj, &Cancel);
	}

	/* reset TimerList */
	initList(&pAd->RscTimerCreateList);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pAd			Pointer to our adapter
		pTimer				Timer structure
		pTimerFunc			Function to execute when timer expired
		Repeat				Ture for period timer

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPInitTimer(
	IN	RTMP_ADAPTER *pAd,
	IN	RALINK_TIMER_STRUCT *pTimer,
	IN	VOID *pTimerFunc,
	IN	VOID *pData,
	IN	BOOLEAN	 Repeat)
{

	pTimer->timer_lock = &pAd->TimerSemLock;

	RTMP_SEM_LOCK(pTimer->timer_lock);
	RTMP_TimerListAdd(pAd, pTimer);


	/* Set Valid to TRUE for later used.*/
	/* It will crash if we cancel a timer or set a timer */
	/* that we haven't initialize before.*/
	/* */
	pTimer->Valid      = TRUE;

	pTimer->PeriodicType = Repeat;
	pTimer->State      = FALSE;
	pTimer->cookie = (ULONG) pData;
	pTimer->pAd = pAd;
	pTimer->pCaller = (VOID*)OS_TRACE;

	RTMP_OS_Init_Timer(pAd, &pTimer->TimerObj,	pTimerFunc, (PVOID) pTimer, &pAd->RscTimerMemList);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	if (!pTimer->timer_lock)
		return;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid)
	{
		RTMP_ADAPTER *pAd;

		pAd = (RTMP_ADAPTER *)pTimer->pAd;
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPSetTimer failed, Halt in Progress!\n"));
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
			return;
		}

		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		}
		else
		{
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPSetTimer failed, Timer hasn't been initialize!\n"));
	}
	RTMP_SEM_UNLOCK(pTimer->timer_lock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	BOOLEAN	Cancel;

	if (!pTimer->timer_lock)
		return;

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid)
	{
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		}
		else
		{
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
			RTMP_SEM_UNLOCK(pTimer->timer_lock);
		}
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPModTimer failed, Timer hasn't been initialize!\n"));
		RTMP_SEM_UNLOCK(pTimer->timer_lock);
	}
}


/*
	========================================================================

	Routine Description:
		Cancel timer objects

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		1.) To use this routine, must call RTMPInitTimer before.
		2.) Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{

	if(!pTimer->timer_lock)
	{
		goto end;
	}

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid)
	{
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}

	RTMP_SEM_UNLOCK(pTimer->timer_lock);
	return ;
end:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
}


VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{

	if(!pTimer->timer_lock)
	{
		goto end;
	}

	RTMP_SEM_LOCK(pTimer->timer_lock);

	if (pTimer->Valid)
	{
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_SEM_UNLOCK(pTimer->timer_lock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(pTimer->timer_lock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */

		/* release timer */
		RTMP_OS_Release_Timer(&pTimer->TimerObj);

		pTimer->Valid = FALSE;
		// TODO: shiang-usw, merge this from NXTC, make sure if that's necessary here!!
		RTMP_TimerListRelease(pTimer->pAd, pTimer);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}

	RTMP_SEM_UNLOCK(pTimer->timer_lock);

end:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPReleasefailed, Timer hasn't been initialize!\n"));
}


/*
	========================================================================

	Routine Description:
		Enable RX

	Arguments:
		pAd						Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL

	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID RTMPEnableRxTx(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> RTMPEnableRxTx\n"));

	RT28XXDMAEnable(pAd);

	AsicSetRxFilter(pAd);

	if (pAd->CommonCfg.bTXRX_RXV_ON)
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE);
	else
		AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== RTMPEnableRxTx\n"));
}


void CfgInitHook(RTMP_ADAPTER *pAd)
{
	/*pAd->bBroadComHT = TRUE;*/
}


static INT RtmpChipOpsRegister(RTMP_ADAPTER *pAd, INT infType)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	int ret = 0;

	NdisZeroMemory(pChipOps, sizeof(RTMP_CHIP_OP));
	NdisZeroMemory(&pAd->chipCap, sizeof(RTMP_CHIP_CAP));

	ret = RtmpChipOpsHook(pAd);

	if (ret) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("chipOps hook error\n"));
		return ret;
	}
	/* MCU related */
	ChipOpsMCUHook(pAd, pAd->chipCap.MCUType);

	get_dev_config_idx(pAd);

	return ret;
}




PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR wdev_idx)
{
	PNET_DEV dev_p = NULL;

	if (wdev_idx < WDEV_NUM_MAX)
		dev_p = pAd->wdev_list[wdev_idx]->if_dev;

	ASSERT((dev_p != NULL));
	return dev_p;
}


INT RtmpRaDevCtrlInit(VOID *pAdSrc, RTMP_INF_TYPE infType)
{
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)pAdSrc;

#ifdef FW_DUMP_SUPPORT
	pAd->fw_dump_max_size = MAX_FW_DUMP_SIZE;
    RTMP_OS_FWDUMP_PROCINIT(pAd);
#endif

	/* Assign the interface type. We need use it when do register/EEPROM access.*/
	pAd->infType = infType;


#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP Driver version-%s\n", AP_DRIVER_VERSION));
#endif /* CONFIG_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->infType=%d\n", pAd->infType));

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	pAd->OpMode = OPMODE_STA;
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */

	RTMP_SEM_EVENT_INIT(&(pAd->AutoRateLock), &pAd->RscSemMemList);

#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
/*	if ((IS_PCIE_INF(pAd))) */
	{
		/* specific for RT6855/RT6856 */
		pAd->E2P_OFFSET_IN_FLASH[0] = 0x40000;
		pAd->E2P_OFFSET_IN_FLASH[1] = 0x48000;
	}
#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */

	if (RtmpChipOpsRegister(pAd, infType))
	{
		return FALSE;
	}

	AsicGetMacInfo(pAd,&pAd->ChipID,&pAd->HWVersion,&pAd->FWVersion);

	/*for HdevCfg*/
	HcCfgInit(pAd);

#ifdef RESOURCE_PRE_ALLOC
	/*
		move this function from mt_wifi_init() to here. now this function only allocate memory and
		leave the initialization job to RTMPInitTxRxRingMemory() which called in mt_wifi_init().
	*/
#ifndef MT7622 //wilsonl, to adjust the position
	if (RTMPAllocTxRxRingMemory(pAd) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Failed to allocate memory - TxRxRing\n"));
		return FALSE;
	}
#endif
#endif /* RESOURCE_PRE_ALLOC */



#ifdef MULTIPLE_CARD_SUPPORT
{
	extern BOOLEAN RTMP_CardInfoRead(PRTMP_ADAPTER pAd);

	/* find its profile path*/
	pAd->MC_RowID = -1; /* use default profile path*/
	RTMP_CardInfoRead(pAd);

	if (pAd->MC_RowID == -1)
#ifdef CONFIG_AP_SUPPORT
		strcpy(pAd->MC_FileName, AP_PROFILE_PATH);
#endif /* CONFIG_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MC> ROW = %d, PATH = %s\n", pAd->MC_RowID, pAd->MC_FileName));
}
#endif /* MULTIPLE_CARD_SUPPORT */


#ifdef MCS_LUT_SUPPORT
	if (pAd->chipCap.asic_caps & fASIC_CAP_MCS_LUT) {
		if (MAX_LEN_OF_MAC_TABLE < 128) {
			RTMP_SET_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT);
		} else {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): MCS_LUT not used becasue MacTb size(%d) > 128!\n",
						__FUNCTION__, MAX_LEN_OF_MAC_TABLE));
		}
	}
#endif /* MCS_LUT_SUPPORT */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type != HIF_MT)
#endif
	{
		if (load_patch(pAd) != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("load patch failed!\n"));
			return FALSE;
		}
	}

	return 0;
}


BOOLEAN RtmpRaDevCtrlExit(IN VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	INT index;

#ifdef MULTIPLE_CARD_SUPPORT
extern UINT8  MC_CardUsed[MAX_NUM_OF_MULTIPLE_CARD];

	if ((pAd->MC_RowID >= 0) && (pAd->MC_RowID <= MAX_NUM_OF_MULTIPLE_CARD))
		MC_CardUsed[pAd->MC_RowID] = 0; /* not clear MAC address*/
#endif /* MULTIPLE_CARD_SUPPORT */


#ifdef RLT_MAC
	if ((IS_MT76x0(pAd) || IS_MT76x2(pAd))&& (pAd->WlanFunCtrl.field.WLAN_EN == 1))
	{
		rlt_wlan_chip_onoff(pAd, FALSE, FALSE);
	}
#endif /* RLT_MAC */

    RTMP_SEM_EVENT_DESTORY(&(pAd->AutoRateLock));



	/*
		Free ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++)
	{
		if (pAd->ProbeRespIE[index].pIe)
			os_free_mem(pAd->ProbeRespIE[index].pIe);
	}

#ifdef RESOURCE_PRE_ALLOC
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

#ifdef FW_DUMP_SUPPORT

	RTMP_OS_FWDUMP_PROCREMOVE(pAd);

	if (pAd->fw_dump_buffer)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Free FW dump buffer\n", __func__));

		os_free_mem(pAd->fw_dump_buffer);
		pAd->fw_dump_buffer = 0;
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;
	}
#endif

	HcCfgExit(pAd);	
	wpf_config_exit(pAd);
	RTMPFreeAdapter(pAd);

	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(RTMP_ADAPTER *pAd)
{
	RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);
}

VOID RTMP_11N_D3_TimerRelease(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancel;
	RTMPReleaseTimer(&pAd->CommonCfg.Bss2040CoexistTimer,&Cancel);
}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef VENDOR_FEATURE3_SUPPORT
VOID RTMP_IO_WRITE32(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT32 Value)
{
	_RTMP_IO_WRITE32(pAd, Offset, Value);
}
#endif /* VENDOR_FEATURE3_SUPPORT */


VOID AntCfgInit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
						__FUNCTION__, __LINE__));
	return;
}


