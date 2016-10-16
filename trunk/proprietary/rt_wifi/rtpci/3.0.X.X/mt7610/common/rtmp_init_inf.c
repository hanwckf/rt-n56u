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
#ifdef MBSS_SUPPORT
	pDrvOps->MBSS_PacketSend = MBSS_PacketSend;
#endif /* MBSS_SUPPORT */
#ifdef WDS_SUPPORT
	pDrvOps->WDS_PacketSend = WDS_PacketSend;
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
	pDrvOps->APC_PacketSend = APC_PacketSend;
#endif /* APCLI_SUPPORT */

	pDrvOps->RTMP_COM_IoctlHandle = RTMP_COM_IoctlHandle;
#ifdef CONFIG_AP_SUPPORT
	pDrvOps->RTMP_AP_IoctlHandle = RTMP_AP_IoctlHandle;
#endif /* CONFIG_AP_SUPPORT */

	pDrvOps->RTMPDrvOpen = RTMPDrvOpen;
	pDrvOps->RTMPDrvClose = RTMPDrvClose;
	pDrvOps->RTMPInfClose = RTMPInfClose;
	pDrvOps->rt28xx_init = rt28xx_init;

	/* init operators provided from us and netif module */
}

RTMP_BUILD_DRV_OPS_FUNCTION_BODY

#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */


int rt28xx_init(VOID *pAdSrc, PSTRING pDefaultMac, PSTRING pHostName)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT index;
	NDIS_STATUS Status;

	if (pAd == NULL)
		return FALSE;

#ifdef RT65xx
	if (pAd->WlanFunCtrl.field.WLAN_EN == 0)
		MT76x0_WLAN_ChipOnOff(pAd, TRUE, FALSE);
#endif /* RT65xx */

#ifdef RT3290
	DBGPRINT(RT_DEBUG_OFF, ("MACVersion=0x%x\n", pAd->MACVersion));
	if (IS_RT3290(pAd))
	{
		UINT32 MacRegValue;
		OSCCTL_STRUC osCtrl = {.word = 0};
		CMB_CTRL_STRUC cmbCtrl = {.word = 0};
		WLAN_FUN_CTRL_STRUC WlanFunCtrl = {.word = 0};
			
		RTMPEnableWlan(pAd, TRUE, TRUE);

		RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &WlanFunCtrl.word);
		if (WlanFunCtrl.field.WLAN_EN == TRUE)
		{
			WlanFunCtrl.field.PCIE_APP0_CLK_REQ = TRUE;
			RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
		}
			
		//Enable ROSC_EN first then CAL_REQ
		RTMP_IO_READ32(pAd, OSCCTL, &osCtrl.word);
		osCtrl.field.ROSC_EN = TRUE; /* HW force */
		RTMP_IO_WRITE32(pAd, OSCCTL, osCtrl.word);	
		
		osCtrl.field.ROSC_EN = TRUE; /* HW force */
		osCtrl.field.CAL_REQ = TRUE;
		osCtrl.field.REF_CYCLE = 0x27;
		RTMP_IO_WRITE32(pAd, OSCCTL, osCtrl.word);

		RTMP_IO_READ32(pAd, CMB_CTRL, &cmbCtrl.word);
		pAd->CmbCtrl.word = cmbCtrl.word;

		/* Overwrite default Coex Parameter */
		RTMP_IO_READ32(pAd, COEXCFG0, &MacRegValue);
		MacRegValue &= ~(0xFF000000);
		MacRegValue |= 0x5E000000;
		RTMP_IO_WRITE32(pAd, COEXCFG0, MacRegValue);
	}

	if (IS_RT3290LE(pAd))
	{
		PLL_CTRL_STRUC PllCtrl;
		RTMP_IO_READ32(pAd, PLL_CTRL, &PllCtrl.word);
		PllCtrl.field.VCO_FIXED_CURRENT_CONTROL = 0x1;			
		RTMP_IO_WRITE32(pAd, PLL_CTRL, PllCtrl.word);
	}
#endif /* RT3290 */


	/* reset Adapter flags*/
	RTMP_CLEAR_FLAGS(pAd);

	/* Init BssTab & ChannelInfo tabbles for auto channel select.*/
#ifdef CONFIG_AP_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		AutoChBssTableInit(pAd);
		ChannelInfoInit(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* Allocate BA Reordering memory*/
	if (ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM) != TRUE)		
		goto err1;
#endif /* DOT11_N_SUPPORT */

	/* Make sure MAC gets ready.*/
	index = 0;
	if (WaitForAsicReady(pAd) != TRUE)
		goto err1;

	DBGPRINT(RT_DEBUG_TRACE, ("MAC[Ver:Rev=0x%08x : 0x%08x]\n",
				pAd->MACVersion, pAd->MacIcVersion));


	if (MAX_LEN_OF_MAC_TABLE > MAX_AVAILABLE_CLIENT_WCID(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MAX_LEN_OF_MAC_TABLE can not be larger than MAX_AVAILABLE_CLIENT_WCID!!!!\n"));
		goto err1;
	}

#ifdef RTMP_MAC_PCI

#ifdef RT65xx
	if (IS_RT65XX(pAd)) {
		RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x0);
	}
	else
#endif /* RT65xx */
	{
	/* To fix driver disable/enable hang issue when radio off*/
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x2);
	}
#endif /* RTMP_MAC_PCI */

	/* Disable DMA*/
	RT28XXDMADisable(pAd);

	MCUCtrlInit(pAd);

	{
		unsigned long old, new, diff;
		old = jiffies;
		/* Load MCU firmware*/
		Status = NICLoadFirmware(pAd);
		new = jiffies;

		diff = (new - old) * 1000 / HZ;

		printk("load fw spent %ldms\n", diff);
	}

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		goto err1;
	}

	/* Disable interrupts here which is as soon as possible*/
	/* This statement should never be true. We might consider to remove it later*/
#ifdef RTMP_MAC_PCI
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}
#endif /* RTMP_MAC_PCI */

#ifdef RESOURCE_PRE_ALLOC
	Status = RTMPInitTxRxRingMemory(pAd);
#else
	Status = RTMPAllocTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPAllocTxRxMemory failed, Status[=0x%08x]\n", Status));
		goto err2;
	}

#ifdef WLAN_SKB_RECYCLE
    skb_queue_head_init(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);

	/* initialize MLME*/
	Status = RtmpMgmtTaskInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err3;

	Status = MlmeInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MlmeInit failed, Status[=0x%08x]\n", Status));
		goto err4;
	}

#ifdef RMTP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
	VideoConfigInit(pAd);
#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RMTP_RBUS_SUPPORT */

	/* Initialize pAd->StaCfg, pAd->ApCfg, pAd->CommonCfg to manufacture default*/
	UserCfgInit(pAd);


	Status = RtmpNetTaskInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err5;

	CfgInitHook(pAd);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APInitialize(pAd);
#endif /* CONFIG_AP_SUPPORT */	

#ifdef BLOCK_NET_IF
	initblockQueueTab(pAd);
#endif /* BLOCK_NET_IF */

	Status = MeasureReqTabInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MeasureReqTabInit failed, Status[=0x%08x]\n",Status));
		goto err6;	
	}
	Status = TpcReqTabInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("TpcReqTabInit failed, Status[=0x%08x]\n",Status));
		goto err6;	
	}

	/* Init the hardware, we need to init asic before read registry, otherwise mac register will be reset*/
	
	Status = NICInitializeAdapter(pAd, TRUE);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICInitializeAdapter failed, Status[=0x%08x]\n", Status));
		if (Status != NDIS_STATUS_SUCCESS)
		goto err6;
	}

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		goto err1;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Read parameters from Config File */
	/* unknown, it will be updated in NICReadEEPROMParameters */
	pAd->RfIcType = RFIC_UNKNOWN;
	Status = RTMPReadParametersHook(pAd);

#ifdef FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_init(pAd);
#endif /* CAPTURE_MODE */
#endif /* FPGA_MODE */


	DBGPRINT(RT_DEBUG_OFF, ("1. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPReadParametersHook failed, Status[=0x%08x]\n",Status));
		goto err6;
	}


#ifdef DOT11_N_SUPPORT
   	/*Init Ba Capability parameters.*/
/*	RT28XX_BA_INIT(pAd);*/
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	/* UPdata to HT IE*/
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
#endif /* DOT11_N_SUPPORT */

	/* after reading Registry, we now know if in AP mode or STA mode*/

	DBGPRINT(RT_DEBUG_OFF, ("2. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	/* We should read EEPROM for all cases.  rt2860b*/
	NICReadEEPROMParameters(pAd, (PSTRING)pDefaultMac);

	DBGPRINT(RT_DEBUG_OFF, ("3. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

#ifdef LED_CONTROL_SUPPORT
	/* Send LED Setting to MCU */
	RTMPInitLEDMode(pAd);	
#endif /* LED_CONTROL_SUPPORT */

	NICInitAsicFromEEPROM(pAd); /* rt2860b */

#ifdef RALINK_ATE
	if (ATEInit(pAd) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): ATE initialization failed !\n", __FUNCTION__));
		goto err6;
	}
#endif /* RALINK_ATE */


#ifdef RTMP_INTERNAL_TX_ALC
	/* Initialize the desired TSSI table*/
	RTMP_CHIP_ASIC_TSSI_TABLE_INIT(pAd);
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_COMPENSATION
	/* Temperature compensation, initialize the lookup table */
	DBGPRINT(RT_DEBUG_OFF, ("bAutoTxAgcG = %d\n", pAd->bAutoTxAgcG));
#endif /* RTMP_TEMPERATURE_COMPENSATION */


	/* Set PHY to appropriate mode*/
	RTMPSetPhyMode(pAd, pAd->CommonCfg.PhyMode);

	/* No valid channels.*/
	if (pAd->ChannelListNum == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n"));
		goto err6;
	}

#ifdef DOT11_N_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("MCS Set = %02x %02x %02x %02x %02x\n", pAd->CommonCfg.HtCapability.MCSSet[0],
           pAd->CommonCfg.HtCapability.MCSSet[1], pAd->CommonCfg.HtCapability.MCSSet[2],
           pAd->CommonCfg.HtCapability.MCSSet[3], pAd->CommonCfg.HtCapability.MCSSet[4]));
#endif /* DOT11_N_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef AP_QLOAD_SUPPORT
		QBSS_LoadInit(pAd);
#endif /* AP_QLOAD_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	/* APInitialize(pAd);*/

#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif /* IKANOS_VX_1X0 */


#ifdef RALINK_ATE
#endif /* RALINK_ATE */

#ifdef CONFIG_AP_SUPPORT
	
	/* Initialize RF register to default value*/
	

#ifndef MT76x0
	if (pAd->OpMode == OPMODE_AP)
	{
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
	}
#endif /* !MT76x0 */
#endif /* CONFIG_AP_SUPPORT */

	/*
		Some modules init must be called before APStartUp().
		Or APStartUp() will make up beacon content and call
		other modules API to get some information to fill.
	*/



	if (pAd && (Status != NDIS_STATUS_SUCCESS))
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		{
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
		}
	}
	else if (pAd)
	{
		/* Microsoft HCT require driver send a disconnect event after driver initialization.*/
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MEDIA_STATE_CHANGE);

		DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.bAutoChannelAtBootup || (pAd->CommonCfg.Channel == 0))
			{	
				pAd->CommonCfg.Channel=pAd->ChannelList[0].Channel;
			}

#ifdef DOT11_N_SUPPORT
			/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
			N_ChannelCheck(pAd);

#ifdef DOT11N_DRAFT3
        		/* 
         			We only do this Overlapping BSS Scan when system up, for the 
				other situation of channel changing, we depends on station's 
				report to adjust ourself.
			*/
			if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n", 
											pAd->CommonCfg.bBssCoexEnable, 
											pAd->CommonCfg.bForty_Mhz_Intolerant));
			}
			else if(pAd->CommonCfg.bBssCoexEnable == TRUE)
			{	
				DBGPRINT(RT_DEBUG_TRACE, ("Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n", 
							pAd->CommonCfg.bBssCoexEnable));
				APOverlappingBSSScan(pAd);
			}

			RTMP_11N_D3_TimerInit(pAd);
/*			RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);*/
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

			APStartUp(pAd);
			DBGPRINT(RT_DEBUG_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n", 
									PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].Bssid)));

#ifdef DYNAMIC_VGA_SUPPORT
			{
				UCHAR val;
				UINT32 bbp_val, bbp_reg = AGC1_R8;

				RTMP_BBP_IO_READ32(pAd, bbp_reg, &bbp_val);
				val = ((bbp_val & (0x0000ff00)) >> 8) & 0xff;
				pAd->CommonCfg.MO_Cfg.Stored_BBP_R66 = val;
			}
#endif /* DYNAMIC_VGA_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

	}/* end of else*/

	/* Set up the Mac address*/
#ifdef CONFIG_AP_SUPPORT
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], NULL);
#endif /* CONFIG_AP_SUPPORT */

	/* Various AP function init*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MBSS_SUPPORT
		/* the function can not be moved to RT2860_probe() even register_netdev()
		   is changed as register_netdevice().
		   Or in some PC, kernel will panic (Fedora 4) */
/*		RT28xx_MBSS_Init(pAd, pAd->net_dev);  os abl move to rt_main_dev.c*/
#endif /* MBSS_SUPPORT */

#ifdef WDS_SUPPORT
/*		RT28xx_WDS_Init(pAd, pAd->net_dev);*/
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
/*		RT28xx_ApCli_Init(pAd, pAd->net_dev);*/
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef UAPSD_SUPPORT
        UAPSD_Init(pAd);
#endif /* UAPSD_SUPPORT */

	/* assign function pointers*/
#ifdef MAT_SUPPORT
	/* init function pointers, used in OS_ABL */
	RTMP_MATOpsInit(pAd);
#endif /* MAT_SUPPORT */




#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		MATEngineInit(pAd);
#endif /* MAT_SUPPORT */

#ifdef CLIENT_WDS
		CliWds_ProxyTabInit(pAd);
#endif /* CLIENT_WDS */
	}
#endif /* CONFIG_AP_SUPPORT */


	/* auto-fall back settings */
#ifdef RANGE_EXTEND
	RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xedcba980);
#endif // RANGE_EXTEND //
#ifdef DOT11N_SS3_SUPPORT
	if (pAd->CommonCfg.TxStream >= 3)
	{
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, 0x12111008);
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, 0x16151413);
	}
#endif /* DOT11N_SS3_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	RtmpStreamModeInit(pAd);
#endif /* STREAM_MODE_SUPPORT */


#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	if (pAd->CommonCfg.ITxBfTimeout)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, 0x02);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, 0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, pAd->CommonCfg.ITxBfTimeout & 0xFF);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, 1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, (pAd->CommonCfg.ITxBfTimeout>>8) & 0xFF);
	}

	if (pAd->CommonCfg.ETxBfTimeout)
	{
		RTMP_IO_WRITE32(pAd, TX_TXBF_CFG_3, pAd->CommonCfg.ETxBfTimeout);
	}
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */



#ifdef RT3290
	if (IS_RT3290(pAd))
	{
		WLAN_FUN_CTRL_STRUC     WlanFunCtrl = {.word = 0};
		RTMP_MAC_PWRSV_EN(pAd, TRUE, TRUE);	
		//
		// Too much time for reading efuse(enter/exit L1), and our device will hang up
		// Enable L1
		//
		RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &WlanFunCtrl.word);
		if (WlanFunCtrl.field.WLAN_EN == TRUE)
		{
			WlanFunCtrl.field.PCIE_APP0_CLK_REQ = FALSE;
			RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
		}
	}
#endif /* RT3290 */

	DBGPRINT_S(Status, ("<==== rt28xx_init, Status=%x\n", Status));

	return TRUE;

/*err7:
	APStop(pAd);*/
err6:

#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(&pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);
err5:	
	RtmpNetTaskExit(pAd);
	UserCfgExit(pAd);
err4:	
	MlmeHalt(pAd);
	RTMP_TimerListRelease(pAd);
err3:	
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */
err2:
#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

err1:

	MCUCtrlExit(pAd);

#ifdef CONFIG_AP_SUPPORT
	/* Free BssTab & ChannelInfo tabbles.*/
	AutoChBssTableDestroy(pAd);
	ChannelInfoDestroy(pAd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef RT3290
	if (IS_RT3290(pAd))
		RTMPEnableWlan(pAd, FALSE, FALSE);
#endif /* RT3290 */

#ifdef DOT11_N_SUPPORT
	if(pAd->mpdu_blk_pool.mem)
		os_free_mem(pAd, pAd->mpdu_blk_pool.mem); /* free BA pool*/
#endif /* DOT11_N_SUPPORT */

	/* shall not set priv to NULL here because the priv didn't been free yet.*/
	/*net_dev->priv = 0;*/
#ifdef INF_AMAZON_SE
err0:
#endif /* INF_AMAZON_SE */
#ifdef ST
err0:
#endif /* ST */

	DBGPRINT(RT_DEBUG_ERROR, ("!!! rt28xx init fail !!!\n"));
	return FALSE;
}


VOID RTMPDrvOpen(
	IN VOID			*pAdSrc)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)pAdSrc;


#ifdef FPGA_MODE
#ifdef CUSTOMER_DEMO
	set_fpga_mode(pAd, "6");
#endif /* CUSTOMER_DEMO */
#endif /* FPGA_MODE */

	/* Enable Interrupt*/
	RTMP_IRQ_ENABLE(pAd);

	/* Now Enable RxTx*/
	RTMPEnableRxTx(pAd);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

#ifdef MT76x0
	if (IS_MT76x0(pAd))
	{
		/* Select Q2 to receive command response */
		AndesFunSetOP(pAd, Q_SELECT, pAd->chipCap.CmdRspRxRing);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			AsicDisableSync(pAd);
			/* Do full calibration once at power up */
			MT76x0_Calibration(pAd,
								pAd->hw_cfg.cent_ch,
								TRUE,
								((pAd->MCUCtrl.bFwHasBeenLoaded == FALSE) ? TRUE:FALSE));
			
			if (pAd->Dot11_H.RDMode != RD_SILENCE_MODE)
				AsicEnableBssSync(pAd);
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /* MT76x0 */

			if (pAd->ApCfg.bAutoChannelAtBootup || (pAd->CommonCfg.Channel == 0))
			{	
				rtmp_bbp_set_bw(pAd, BW_20);
				DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));

				/* Now we can receive the beacon and do the listen beacon*/
				/* use default BW to select channel*/
				pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, pAd->ApCfg.AutoChannelAlg);
				pAd->ApCfg.bAutoChannelAtBootup = FALSE;
				APStop(pAd);
				APStartUp(pAd);
			}
	{
	UINT32 reg = 0;
	RTMP_IO_READ32(pAd, 0x1300, &reg);  /* clear garbage interrupts*/
	if (reg) {
		DBGPRINT(RT_DEBUG_TRACE, ("0x1300 = %08x\n", reg));
	}
	}

	{
/*	u32 reg;*/
/*	UINT8  byte;*/
/*	u16 tmp;*/

/*	RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &reg);*/

/*	tmp = 0x0805;*/
/*	reg  = (reg & 0xffff0000) | tmp;*/
/*	RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, reg);*/

	}


#ifdef RTMP_MAC_PCI
	RTMPInitPCIeLinkCtrlValue(pAd);
#endif /* RTMP_MAC_PCI */


#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Init();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

//+++Add by shiang for debug
	DBGPRINT(RT_DEBUG_OFF, ("%s(1):Check if PDMA is idle!\n", __FUNCTION__));
	AsicWaitPDMAIdle(pAd, 5, 10);
//---Add by shiang for debug

//+++Add by shiang for debug
	DBGPRINT(RT_DEBUG_OFF, ("%s(2):Check if PDMA is idle!\n", __FUNCTION__));
	AsicWaitPDMAIdle(pAd, 5, 10);
//---Add by shiang for debug


#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
	if ((pAd->OpMode == OPMODE_AP)
		)
	{
		INT index;
		for (index = 0; index < pAd->ApCfg.BssidNum; index++)
		{
#ifdef HOSTAPD_SUPPORT
			if (pAd->ApCfg.MBSSID[index].Hostapd == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
			}
			else
#endif /*HOSTAPD_SUPPORT*/
			{
				PWSC_CTRL pWscControl;
				UCHAR zeros16[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
				
				pWscControl = &pAd->ApCfg.MBSSID[index].WscControl;
				DBGPRINT(RT_DEBUG_TRACE, ("Generate UUID for apidx(%d)\n", index));
				if (NdisEqualMemory(&pWscControl->Wsc_Uuid_E[0], zeros16, UUID_LEN_HEX))
					WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], index, FALSE);
				WscInit(pAd, FALSE, index);
			}
		}

#ifdef APCLI_SUPPORT
		for(index = 0; index < MAX_APCLI_NUM; index++)
		{
			PWSC_CTRL pWpsCtrl = &pAd->ApCfg.ApCliTab[index].WscControl;
			
			pWpsCtrl->pAd = pAd;        
			NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);
			pWpsCtrl->WscConfigMethods= 0x018C;
			RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WSC_INIT, 0, (VOID *)&pAd->ApCfg.ApCliTab[index], index);
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Init(pAd);
#endif /* WSC_INCLUDED */

}

VOID RTMPDrvClose(
	IN VOID				*pAdSrc,
	IN VOID				*net_dev)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)pAdSrc;
	BOOLEAN 		Cancelled;
	UINT32			i = 0;


	Cancelled = FALSE;


#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Remove();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF);
#endif /* LED_CONTROL_SUPPORT */
	pAd->MCUCtrl.IsFWReady = FALSE;
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef EXT_BUILD_CHANNEL_LIST
	if (pAd->CommonCfg.pChDesp != NULL)
		os_free_mem(NULL, pAd->CommonCfg.pChDesp);
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
#endif /* EXT_BUILD_CHANNEL_LIST */
	pAd->CommonCfg.bCountryFlag = FALSE;

#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

	RtmpOsMsDelay(20); /* wait for disconnect requests transmitted */

	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RTMPusecDelay(1000);
		}
	}
	

#ifdef CONFIG_AP_SUPPORT

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{

#ifdef DOT11N_DRAFT3
		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED)
		{
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}
#endif /* DOT11N_DRAFT3 */

		/* PeriodicTimer already been canceled by MlmeHalt() API.*/
		/*RTMPCancelTimer(&pAd->PeriodicTimer,	&Cancelled);*/
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Stop Mlme state machine*/
	MlmeHalt(pAd);
	
	/* Close net tasklets*/
	RtmpNetTaskExit(pAd);



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
	}
#endif /* CONFIG_AP_SUPPORT */

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);

#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);
#endif // LED_CONTROL_SUPPORT


	/* Close kernel threads*/
	RtmpMgmtTaskExit(pAd);

	MCUCtrlExit(pAd);
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* must after RtmpMgmtTaskExit(); Or pAd->pChannelInfo will be NULL */
		/* Free BssTab & ChannelInfo tabbles.*/
		AutoChBssTableDestroy(pAd);
		ChannelInfoDestroy(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef RTMP_MAC_PCI
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
		{
#ifdef MT76x0
			MT76x0_DisableTxRx(pAd, RTMP_HALT);
#endif /* MT76x0 */

			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
		}

		/* Receive packets to clear DMA index after disable interrupt. */
		/*RTMPHandleRxDoneInterrupt(pAd);*/
		/* put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all */
		/* register access before Radio off.*/

#ifdef RTMP_PCI_SUPPORT
		if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
		{
#ifndef MT76x0
			BOOLEAN brc;
			brc = RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);
#endif /* !MT76x0 */

/*In  solution 3 of 3090F, the bPCIclkOff will be set to TRUE after calling RT28xxPciAsicRadioOff*/
#ifdef PCIE_PS_SUPPORT
			pAd->bPCIclkOff = FALSE;    
#endif /* PCIE_PS_SUPPORT */

#ifndef MT76x0
			if (brc==FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__)); 
			}
#endif /* !MT76x0 */
		}
#endif /* RTMP_PCI_SUPPORT */
	}

#endif /* RTMP_MAC_PCI */

	/* Free IRQ*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
#ifdef RTMP_MAC_PCI
		/* Deregister interrupt function*/
		RTMP_OS_IRQ_RELEASE(pAd, net_dev);
#endif /* RTMP_MAC_PCI */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}

#ifdef SINGLE_SKU_V2
	{
		CH_POWER *ch, *ch_temp;
		DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
		{
			DlListDel(&ch->List);
			os_free_mem(NULL, ch);
		}
	}
#endif /* SINGLE_SKU_V2 */

#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	/* Free Ring or USB buffers*/
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef WLAN_SKB_RECYCLE
	skb_queue_purge(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */

#ifdef DOT11_N_SUPPORT
	/* Free BA reorder resource*/
	ba_reordering_resource_release(pAd);
#endif /* DOT11_N_SUPPORT */

	UserCfgExit(pAd); /* must after ba_reordering_resource_release */


	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);

/*+++Modify by woody to solve the bulk fail+++*/

	/* clear MAC table */
	/* TODO: do not clear spin lock, such as fLastChangeAccordingMfbLock */
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));

	/* release all timers */
	RTMPusecDelay(2000);
	RTMP_TimerListRelease(pAd);

#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */

#ifdef FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_deinit(pAd);
#endif /* CAPTURE_MODE */
#endif /* FPGA_MODE */

}


VOID RTMPInfClose(
	IN VOID				*pAdSrc)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)pAdSrc;


#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = FALSE;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* kick out all STAs behind the bss.*/
		MbssKickOutStas(pAd, MAIN_MBSSID, REASON_DISASSOC_INACTIVE);
	}

	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);
#endif /* CONFIG_AP_SUPPORT */



}




PNET_DEV RtmpPhyNetDevMainCreate(
	IN VOID				*pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	PNET_DEV pDevNew;
	UINT32 MC_RowID = 0, IoctlIF = 0;


	pAd = pAd;

#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
	IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

	pDevNew = RtmpOSNetDevCreate((INT32)MC_RowID, (UINT32 *)&IoctlIF,
					INT_MAIN, 0, sizeof(PRTMP_ADAPTER), INF_MAIN_DEV_NAME);

#ifdef HOSTAPD_SUPPORT
	pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

	return pDevNew;
}


