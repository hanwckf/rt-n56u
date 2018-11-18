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
	rtmp_init_inf.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include	"ft.h"
#endif /* DOT11R_FT_SUPPORT */


#ifdef MULTI_PROFILE
VOID multi_profile_exit(struct _RTMP_ADAPTER *ad);
#endif /*MULTI_PROFILE*/

#ifdef LINUX
#ifdef OS_ABL_FUNC_SUPPORT
/* Utilities provided from NET module */
RTMP_NET_ABL_OPS RtmpDrvNetOps, *pRtmpDrvNetOps = &RtmpDrvNetOps;
RTMP_PCI_CONFIG RtmpPciConfig, *pRtmpPciConfig = &RtmpPciConfig;
RTMP_USB_CONFIG RtmpUsbConfig, *pRtmpUsbConfig = &RtmpUsbConfig;

VOID RtmpDrvOpsInit(
	OUT VOID *pDrvOpsOrg,
	INOUT VOID *pDrvNetOpsOrg,
	IN RTMP_PCI_CONFIG *pPciConfig,
	IN RTMP_USB_CONFIG *pUsbConfig)
{
	RTMP_DRV_ABL_OPS *pDrvOps = (RTMP_DRV_ABL_OPS *)pDrvOpsOrg;


	/* init PCI/USB configuration in different OS */
	if (pPciConfig != NULL)
		RtmpPciConfig = *pPciConfig;

	if (pUsbConfig != NULL)
		RtmpUsbConfig = *pUsbConfig;

	/* init operators provided from us (DRIVER module) */
	pDrvOps->RTMPAllocAdapterBlock = RTMPAllocAdapterBlock;
	pDrvOps->RTMPFreeAdapter = RTMPFreeAdapter;

	pDrvOps->RtmpRaDevCtrlExit = RtmpRaDevCtrlExit;
	pDrvOps->RtmpRaDevCtrlInit = RtmpRaDevCtrlInit;
#ifdef RTMP_MAC_PCI
	pDrvOps->RTMPHandleInterrupt = RTMPHandleInterrupt;
#endif /* RTMP_MAC_PCI */
	pDrvOps->RTMPSendPackets = RTMPSendPackets;

	pDrvOps->RTMP_COM_IoctlHandle = RTMP_COM_IoctlHandle;
#ifdef CONFIG_AP_SUPPORT
	pDrvOps->RTMP_AP_IoctlHandle = RTMP_AP_IoctlHandle;
#endif /* CONFIG_AP_SUPPORT */

	pDrvOps->RTMPDrvOpen = RTMPDrvOpen;
	pDrvOps->RTMPDrvClose = RTMPDrvClose;
	pDrvOps->RTMPInfClose = RTMPInfClose;
	pDrvOps->mt_wifi_init = mt_wifi_init;

	/* init operators provided from us and netif module */
}

RTMP_BUILD_DRV_OPS_FUNCTION_BODY

#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */


INT rtmp_cfg_exit(RTMP_ADAPTER *pAd)
{
	UserCfgExit(pAd);

	return TRUE;
}


INT rtmp_cfg_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
#ifndef MT7622 //wilsonl, skip for temp
	NDIS_STATUS status;
#endif

	UserCfgInit(pAd);



	CfgInitHook(pAd);

	/*
		WiFi system operation mode setting base on following partitions:
		1. Parameters from config file
		2. Hardware cap from EEPROM
		3. Chip capabilities in code
	*/
	if (pAd->RfIcType == 0) {
		/* RfIcType not assigned, should not happened! */
		pAd->RfIcType = RFIC_UNKNOWN;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Invalid RfIcType, reset it first\n",
					__FUNCTION__));
	}

#ifndef MT7622 //wilsonl, skip for temp
	status = RTMPReadParametersHook(pAd);
	if (status != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPReadParametersHook failed, Status[=0x%08x]\n",status));
		return FALSE;
	}
#endif

	/*check all enabled function, decide the max unicast wtbl idx will use.*/
	/*After RTMPReadParameterHook to get MBSSNum & MSTANum*/
    HcSetMaxStaNum(pAd);

#ifdef DOT11_N_SUPPORT
   	/*Init Ba Capability parameters.*/
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	/* Updata to HT IE*/
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
#endif /* DOT11_N_SUPPORT */

	return TRUE;
}


INT rtmp_mgmt_init(RTMP_ADAPTER *pAd)
{

	return TRUE;
}


static INT rtmp_sys_exit(RTMP_ADAPTER *pAd)
{

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);

#ifdef DOT11_N_SUPPORT
	if(pAd->mpdu_blk_pool.mem) {
		os_free_mem(pAd->mpdu_blk_pool.mem); /* free BA pool*/
		pAd->mpdu_blk_pool.mem = NULL;
	}
#endif /* DOT11_N_SUPPORT */

	rtmp_cfg_exit(pAd);
	HwCtrlExit(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif
	return TRUE;
}


static INT rtmp_sys_init(RTMP_ADAPTER *pAd,RTMP_STRING *pHostName)
{
	NDIS_STATUS status;

	WifiSysInfoReset(&pAd->WifiSysInfo);

	status = RtmpMgmtTaskInit(pAd);
	if (status != NDIS_STATUS_SUCCESS)
		goto err0;

	status = HwCtrlInit(pAd);
	if(status !=NDIS_STATUS_SUCCESS)
		goto err1;

	/* Initialize pAd->StaCfg[], pAd->ApCfg, pAd->CommonCfg to manufacture default*/
	if (rtmp_cfg_init(pAd, pHostName) != TRUE)
		goto err2;


#ifdef DOT11_N_SUPPORT
	/* Allocate BA Reordering memory*/
	if (ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM) != TRUE)
		goto err2;
#endif /* DOT11_N_SUPPORT */

#ifdef BLOCK_NET_IF
	initblockQueueTab(pAd);
#endif /* BLOCK_NET_IF */

	status = MeasureReqTabInit(pAd);
	if (status != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MeasureReqTabInit failed, Status[=0x%08x]\n", status));
		goto err2;
	}
	status = TpcReqTabInit(pAd);
	if (status != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("TpcReqTabInit failed, Status[=0x%08x]\n", status));
		goto err2;
	}


#ifdef MT_MAC
	/* TxS Setting */
	InitTxSTypeTable(pAd);
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		InitTxSCommonCallBack(pAd);
	}
#endif
#ifdef WH_EZ_SETUP
	pAd->CurWdevIdx = 0;
#endif

	/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
	/* Default set LanNatSpeedUpEn=0 to disable this function */
	pAd->LanNatSpeedUpEn = 0;
	pAd->HwnatCurWdevIdx = 0;
	pAd->isInitBrLan = 0;
	pAd->BrLanIpAddr = 0xffffffff;
	pAd->BrLanMask = 0xffffffff;
#endif

	return TRUE;


err2:
	rtmp_cfg_exit(pAd);
err1:
	HwCtrlExit(pAd);
err0:
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	RtmpMgmtTaskExit(pAd);
	return FALSE;
}

/*rename from rt28xx_init*/
int mt_wifi_init(VOID *pAdSrc, RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName)
{
#if defined(SMART_CARRIER_SENSE_SUPPORT) || defined(BACKGROUND_SCAN_SUPPORT)
        UINT32  CrValue;
#endif /* SMART_CARRIER_SENSE_SUPPORT || BACKGROUND_SCAN_SUPPORT */
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	NDIS_STATUS Status;
//    UCHAR EDCCACtrl;
    UCHAR BandIdx = 0;
#ifdef GPIO_CONTROL_SUPPORT
	UCHAR GpioIndex = 0;
#endif /* GPIO_CONTROL_SUPPORT */

	if (!pAd)
		return FALSE;

#ifdef CONFIG_FWOWN_SUPPORT
	DriverOwn(pAd);
#endif

#if defined(RTMP_MAC) || defined(RLT_MAC)
	rtmp_chip_info_show(pAd);
#else
	mt_chip_info_show(pAd);
#endif

	// TODO: shiang-usw, need to check this for RTMP_MAC
	/* Disable interrupts here which is as soon as possible*/
	/* This statement should never be true. We might consider to remove it later*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}
	
#ifdef FWDL_IN_PROBE
	/* bypass flags reset if the FW DL is done in probe */
	if (pAd->MCUCtrl.fwdl_in_probe == FALSE)
#endif
	{
		/* reset Adapter flags */
		RTMP_CLEAR_FLAGS(pAd);
	}

	/*for software system initialize*/
	if (rtmp_sys_init(pAd,pHostName) != TRUE)
		goto err2;



	if((Status = WfInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WfInit faild!!, ret=%d\n",Status));
		goto err2;
	}


	/* initialize MLME*/
	Status = MlmeInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeInit failed, Status[=0x%08x]\n", Status));
		goto err3;
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_init(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */

	NICInitializeAsic(pAd);

#ifdef LED_CONTROL_SUPPORT
	/* Send LED Setting to MCU */
	RTMPInitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */

	tx_pwr_comp_init(pAd);

#ifdef WIN_NDIS
	/* Patch cardbus controller if EEPROM said so. */
	if (pAd->bTest1 == FALSE)
		RTMPPatchCardBus(pAd);
#endif /* WIN_NDIS */

#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif /* IKANOS_VX_1X0 */

#ifdef CONFIG_ATE
	rtmp_ate_init(pAd);
#endif /*CONFIG_ATE*/

	/* Microsoft HCT require driver send a disconnect event after driver initialization.*/
	//STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));

	rtmp_hif_data_init(pAd);

#ifdef SMART_CARRIER_SENSE_SUPPORT
	/* Enable Band0 PD_BLOCKING */
	HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
	CrValue |= (0x1 << PdBlkEnabeOffset); /* Bit[19] */
	HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
	/* Enable Band1 PD_BLOCKING & initail PD_BLOCKING_TH */
	HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
	CrValue |= (0x1 << PdBlkEnabeOffsetB1); /* Bit[25] */
	CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);  /* OFDM PD BLOCKING TH */
	CrValue |= (PdBlkOfmdThDefault <<PdBlkOfmdThOffsetB1);
	HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef MAC_INIT_OFFLOAD
	AsicSetMacTxRx(pAd,ASIC_MAC_TXRX,TRUE);
#endif /*MAC_INIT_OFFLOAD*/

#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(pAd->OpMode)
	{
		rtmp_ap_init(pAd);
	}
#endif

#ifdef DYNAMIC_VGA_SUPPORT
	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable)
	{
		dynamic_vga_enable(pAd);
	}
#endif /* DYNAMIC_VGA_SUPPORT */

	/* Set PHY to appropriate mode and will update the ChannelListNum in this function */

	if (pAd->ChannelListNum == 0)
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n"));
		goto err3;
	}

#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS Set = %02x %02x %02x %02x %02x\n",
				pAd->CommonCfg.HtCapability.MCSSet[0],
				pAd->CommonCfg.HtCapability.MCSSet[1],
				pAd->CommonCfg.HtCapability.MCSSet[2],
				pAd->CommonCfg.HtCapability.MCSSet[3],
				pAd->CommonCfg.HtCapability.MCSSet[4]));
#endif /* DOT11_N_SUPPORT */

#ifdef UAPSD_SUPPORT
        UAPSD_Init(pAd);
#endif /* UAPSD_SUPPORT */

	/* assign function pointers*/
#ifdef MAT_SUPPORT
	/* init function pointers, used in OS_ABL */
	RTMP_MATOpsInit(pAd);
#endif /* MAT_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	AsicStreamModeInit(pAd);
#endif /* STREAM_MODE_SUPPORT */

#ifdef MT_WOW_SUPPORT
	ASIC_WOW_INIT(pAd);
#endif

#ifdef USB_IOT_WORKAROUND2
	pAd->bUSBIOTReady = TRUE;
#endif

#ifdef CONFIG_AP_SUPPORT
    AutoChSelInit(pAd);
#endif /* CONFIG_AP_SUPPORT */
    
#ifdef REDUCE_TCP_ACK_SUPPORT
    ReduceAckInit(pAd);
#endif
	/* Trigger MIB counter update */
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
		pAd->OneSecMibBucket.Enabled[BandIdx] = TRUE;

	pAd->MsMibBucket.Enabled = TRUE;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<==== mt_wifi_init, Status=%x\n", Status));

#if defined(TXBF_SUPPORT) && defined(MT_MAC) && (!defined(MT7636))
    mt_Trigger_Sounding_Packet(pAd,
                               TRUE,
	                              0,
	                  BF_PROCESSING,
	                              0,
	                           NULL);

    AsicTxBfHwEnStatusUpdate(pAd,
                             pAd->CommonCfg.ETxBfEnCond,
                             pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
#ifdef TXBF_DYNAMIC_DISABLE
	pAd->CommonCfg.ucAutoSoundingCtrl = 0;/* After interface down up, BF disable will be cancelled */
#endif /* TXBF_DYNAMIC_DISABLE */
#endif /* TXBF_SUPPORT */

    /* EDCCA support */
    //EDCCACtrl = GetEDCCASupport(pAd);

    for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
    {
        //pAd->CommonCfg.ucEDCCACtrl[BandIdx] = EDCCACtrl;    
        EDCCACtrlCmd(pAd, BandIdx, pAd->CommonCfg.ucEDCCACtrl[BandIdx]);
    }



#ifdef BACKGROUND_SCAN_SUPPORT
	BackgroundScanInit(pAd);

	/* Enable band0 IPI SW control */
        HW_IO_READ32(pAd, PHY_RXTD_12, &CrValue);
        CrValue |= (1 << B0IrpiSwCtrlResetOffset);
        CrValue |= (1 << B0IrpiSwCtrlOnlyOffset);
        HW_IO_WRITE32(pAd, PHY_RXTD_12, CrValue);
        HW_IO_WRITE32(pAd, PHY_RXTD_12, CrValue);
        /* Enable badn0 IPI control */
        HW_IO_READ32(pAd, PHY_BAND0_PHYMUX_5, &CrValue);
        CrValue |= (B0IpiEnableCtrlValue << B0IpiEnableCtrlOffset);
        HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5, CrValue);
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef NR_PD_DETECTION
    if (pAd->CommonCfg.LinkTestSupport)
    {
        /* Enable 4T ACK Mechanism */
        pAd->fgWifiInitDone = TRUE;
    }
#endif /* NR_PD_DETECTION */
#ifdef CONFIG_AP_SUPPORT
#ifdef GPIO_CONTROL_SUPPORT
	for(GpioIndex = 0; GpioIndex < pAd->ApCfg.NoOfGPIOOutput; GpioIndex++)
	{
		if(IS_GPIO_AVAILABLE(pAd->ApCfg.GPIOOutputPin[GpioIndex]))
			GPIODirectionOuput(pAd,pAd->ApCfg.GPIOOutputPin[GpioIndex],pAd->ApCfg.GPIOOutputData[GpioIndex]);
	}
#endif /* GPIO_CONTROL_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
err3:
	MlmeHalt(pAd);
	RTMP_AllTimerListRelease(pAd);
err2:
	rtmp_sys_exit(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!! mt_wifi_init  fail !!!\n"));
	return FALSE;

}


VOID RTMPDrvOpen(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;

	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);

#ifdef RTMP_MAC
	// TODO: shiang-usw, check this for RMTP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		/* Enable Interrupt*/
		RTMP_IRQ_ENABLE(pAd);

		/* Now Enable RxTx*/
		RTMPEnableRxTx(pAd);
	}
#endif /* RTMP_MAC */


	/*check all enabled function, decide the max unicast wtbl idx will use.*/
	HcSetMaxStaNum(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);





#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Init();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */



#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */


	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Init(pAd);
#endif /* WSC_INCLUDED */

#ifdef MT_MAC_BTCOEX
	//SendAndesWLANStatus(pAd,WLAN_Device_ON,0);
	if (IS_MT76x6(pAd)||IS_MT7637(pAd))
		MT7636MLMEHook(pAd, MT7636_WLAN_Device_ON, 0);
#endif /*MT_MAC_BTCOEX*/

#ifdef MT_WOW_SUPPORT
	pAd->WOW_Cfg.bWoWRunning = FALSE;
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef VOW_SUPPORT
    if (IS_MT7615(pAd) || IS_MT7622(pAd))
        vow_init(pAd);
#else
    vow_atf_off_init(pAd);
#endif /* VOW_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef RED_SUPPORT
	if(pAd->OpMode == OPMODE_AP)
		red_is_enabled(pAd);
#endif /* RED_SUPPORT */

	cp_support_is_enabled(pAd);

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	if (IS_MT7615(pAd)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Trigger DFS Zero wait procedure Support=%d, DfsZeroWaitChannel=%d", pAd->BgndScanCtrl.DfsZeroWaitSupport, pAd->BgndScanCtrl.DfsZeroWaitChannel));
		if (pAd->BgndScanCtrl.DfsZeroWaitSupport == 1 && pAd->BgndScanCtrl.DfsZeroWaitChannel !=0)
			DfsZeroWaitStart(pAd, TRUE);
		/*DfsDedicatedScanStart(pAd);*/
	}
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */

#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
    if(pAd->ApCfg.BandSteering)
    {
        PBND_STRG_CLI_TABLE table;
        table = Get_BndStrgTable(pAd, BSS0);
        if(table)
        {
            /* Inform daemon interface ready */
            struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[BSS0].wdev;
            BndStrg_SetInfFlags(pAd, wdev, table, TRUE);
        }
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */
}


VOID RTMPDrvClose(VOID *pAdSrc, VOID *net_dev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT32 i = 0;
	struct MCU_CTRL *prCtl = NULL;
	prCtl = &pAd->MCUCtrl;


#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Remove();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);


#ifdef MT_MAC
	if (pAd->chipCap.hif_type != HIF_MT)
#endif /* MT_MAC */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef EXT_BUILD_CHANNEL_LIST
	if (pAd->CommonCfg.pChDesp != NULL)
		os_free_mem(pAd->CommonCfg.pChDesp);
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
	pAd->CommonCfg.bCountryFlag = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
	pAd->CommonCfg.bCountryFlag = FALSE;



	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RtmpusecDelay(1000);
		}
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BOOLEAN Cancelled = FALSE;

#ifdef DOT11N_DRAFT3
		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED)
		{
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}
#endif /* DOT11N_DRAFT3 */
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */

#ifdef CLIENT_WDS
		CliWds_ProxyTabDestory(pAd);
#endif /* CLIENT_WDS */
		/* Shutdown Access Point function, release all related resources */
		APShutdown(pAd);

/*#ifdef AUTO_CH_SELECT_ENHANCE*/
		/* Free BssTab & ChannelInfo tabbles.*/
/*		AutoChBssTableDestroy(pAd); */
/*		ChannelInfoDestroy(pAd); */
/*#endif  AUTO_CH_SELECT_ENHANCE */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_ATE
    ATEExit(pAd);
#endif /*CONFIG_ATE*/

	/* Stop Mlme state machine*/
	MlmeHalt(pAd);                

	/* Close net tasklets*/
	NICRestartFirmware(pAd);


	/*  Disable Interrupt */
	if (IS_MT7637(pAd)) //workaround for MtCmdRestartDLReq
	{
#ifdef RTMP_PCI_SUPPORT
		/*  Polling TX/RX path until packets empty */
		MTPciPollTxRxEmpty(pAd);
		/*  Disable PDMA */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 0);
		/*  Polling TX/RX path until packets empty */
		MTPciPollTxRxEmpty(pAd);
#endif
		
		HcSetAllSupportedBandsRadioOff(pAd);

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
		{
			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
		}
	}

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);

#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);
#endif // LED_CONTROL_SUPPORT

	/* Close kernel threads*/
	RtmpMgmtTaskExit(pAd);

#ifdef RTMP_MAC_PCI
	{
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			{
				DISABLE_TX_RX(pAd, RTMP_HALT);
				RTMP_ASIC_INTERRUPT_DISABLE(pAd);
			}
		}

		/* Receive packets to clear DMA index after disable interrupt. */
		/*RTMPHandleRxDoneInterrupt(pAd);*/
		/* put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all */
		/* register access before Radio off.*/

#ifdef RTMP_PCI_SUPPORT
		if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
		{
			BOOLEAN brc = TRUE;

				brc = RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);

/*In  solution 3 of 3090F, the bPCIclkOff will be set to TRUE after calling RT28xxPciAsicRadioOff*/
#ifdef PCIE_PS_SUPPORT
			pAd->bPCIclkOff = FALSE;
#endif /* PCIE_PS_SUPPORT */

			if (brc==FALSE)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__));
			}
		}
#endif /* RTMP_PCI_SUPPORT */
	}

#endif /* RTMP_MAC_PCI */

	/* Free IRQ*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
	{
#ifdef RTMP_MAC_PCI
		/* Deregister interrupt function*/
		RTMP_OS_IRQ_RELEASE(pAd, net_dev);
#endif /* RTMP_MAC_PCI */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	}

#ifdef SINGLE_SKU_V2
	RTMPResetSingleSKUParameters(pAd);
    RTMPResetBfBackOffTable(pAd);
#endif

	/*remove hw related system info*/
	WfSysPosExit(pAd);

	/* Free FW file allocated buffer */
	NICEraseFirmware(pAd);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef WLAN_SKB_RECYCLE
	skb_queue_purge(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */

#ifdef DOT11_N_SUPPORT
	/* Free BA reorder resource*/
	ba_reordering_resource_release(pAd);
#endif /* DOT11_N_SUPPORT */

	UserCfgExit(pAd); /* must after ba_reordering_resource_release */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		ExitTxSTypeTable(pAd);
#endif


#ifdef BACKGROUND_SCAN_SUPPORT
    BackgroundScanDeInit(pAd); 
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
    AutoChSelRelease(pAd);    
#endif/* CONFIG_AP_SUPPORT */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);

/*+++Modify by woody to solve the bulk fail+++*/

	/* clear MAC table */
	/* TODO: do not clear spin lock, such as fLastChangeAccordingMfbLock */
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));

	/* release all timers */
	RtmpusecDelay(2000);
	RTMP_AllTimerListRelease(pAd);

	/* WCNCR00034259: moved from RTMP{Reset, free}TxRxRingMemory() */
	NdisFreeSpinLock(&pAd->CmdQLock);

#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_deinit(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */
#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
	/* Close Hw ctrl*/
	HwCtrlExit(pAd);

#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckExit(pAd);
#endif

#ifdef PRE_CAL_TRX_SET1_SUPPORT
	if(pAd->E2pAccessMode == E2P_FLASH_MODE || pAd->E2pAccessMode == E2P_BIN_MODE)
	{		
		if(pAd->CalDCOCImage != NULL)
			os_free_mem(pAd->CalDCOCImage);
		if(pAd->CalDPDAPart1Image != NULL)
			os_free_mem(pAd->CalDPDAPart1Image);
		if(pAd->CalDPDAPart2GImage != NULL)
			os_free_mem(pAd->CalDPDAPart2GImage);
	}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	if(pAd->PreCalStoreBuffer != NULL)
	{
		os_free_mem(pAd->PreCalStoreBuffer);
		pAd->PreCalStoreBuffer = NULL;
	}
	if(pAd->PreCalReStoreBuffer != NULL)
	{
		os_free_mem(pAd->PreCalReStoreBuffer);
		pAd->PreCalReStoreBuffer = NULL;
	}
#endif/* PRE_CAL_TRX_SET2_SUPPORT */

	/*multi profile release*/
#ifdef MULTI_PROFILE
	multi_profile_exit(pAd);
#endif /*MULTI_PROFILE*/
}


VOID RTMPInfClose(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct wifi_dev *wdev;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		wdev->bAllowBeaconing = FALSE;

#ifdef RADIO_LINK_SELECTION
		if (pAd->ApCfg.RadioLinkSelection)
			Rls_SetInfInfo(pAd, FALSE, wdev);
#endif	/* RADIO_LINK_SELECTION */	
		WifiSysApLinkDown(pAd,wdev);
		WifiSysClose(pAd,wdev);

#ifdef BAND_STEERING
        if(pAd->ApCfg.BandSteering)
        {
            PBND_STRG_CLI_TABLE table;
            table = Get_BndStrgTable(pAd, BSS0);
            if(table)
            {
                /* Inform daemon interface down */
                BndStrg_SetInfFlags(pAd, wdev, table, FALSE);
            }
        }
#endif /* BAND_STEERING */
	}
#endif /*CONFIG_AP_SUPPROT*/


#ifdef RT_CFG80211_SUPPORT			
	pAd->cfg80211_ctrl.beaconIsSetFromHostapd = FALSE;
#endif

}


PNET_DEV RtmpPhyNetDevMainCreate(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	PNET_DEV pDevNew;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;

#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
	IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

	dev_name = get_dev_name_prefix(pAd, INT_MAIN);
	pDevNew = RtmpOSNetDevCreate((INT32)MC_RowID, (UINT32 *)&IoctlIF,
					INT_MAIN, 0, sizeof(struct mt_dev_priv), dev_name);

#ifdef HOSTAPD_SUPPORT
	pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

	return pDevNew;
}



#ifdef ERR_RECOVERY
INT	Set_ErrDetectOn_Proc(
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *arg)
{
	UINT32 Enable;

	Enable = simple_strtol(arg, 0, 10);
    CmdExtGeneralTestOn(pAd, (Enable == 0) ? (FALSE) : (TRUE));

    return TRUE;
}

INT	Set_ErrDetectMode_Proc(
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *arg)
{
    UINT8 mode = 0;
    UINT8 sub_mode = 0;
    PCHAR seg_str;

    if ((seg_str = strsep((char **)&arg, "_")) != NULL)
    {
        mode = (BOOLEAN) simple_strtol(seg_str, 0, 10);
    }

    if ((seg_str = strsep((char **)&arg, "_")) != NULL)
    {
        sub_mode = (BOOLEAN) simple_strtol(seg_str, 0, 10);
    }

    CmdExtGeneralTestMode(pAd, mode, sub_mode);

    return TRUE;
}
#endif /* ERR_RECOVERY */

