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

#ifdef OS_ABL_SUPPORT
UCHAR    BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#endif // OS_ABL_SUPPORT //


int rt28xx_init(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING pDefaultMac, 
	IN PSTRING pHostName)
{
	UINT					index;
	UCHAR					TmpPhy;
	NDIS_STATUS				Status;
	UINT32 					MacCsr0 = 0;
	UINT16					ChipId = 0;
	


	// reset Adapter flags
	RTMP_CLEAR_FLAGS(pAd);

	// Init BssTab & ChannelInfo tabbles for auto channel select.
#ifdef CONFIG_AP_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		AutoChBssTableInit(pAd);
		ChannelInfoInit(pAd);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef DOT11_N_SUPPORT
	// Allocate BA Reordering memory
	if (ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM) != TRUE)		
		goto err1;
#endif // DOT11_N_SUPPORT //

	// Make sure MAC gets ready.
	index = 0;
	do
	{
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);
		pAd->MACVersion = MacCsr0;

		/* The purpose is to identify RT5390H */
		RT28xx_EEPROM_READ16(pAd, 0x00, ChipId);
		pAd->ChipId = ChipId;

		if ((pAd->MACVersion != 0x00) && (pAd->MACVersion != 0xFFFFFFFF))
			break;

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			goto err1;
		
		RTMPusecDelay(10);
	} while (index++ < 100);
	DBGPRINT(RT_DEBUG_TRACE, ("MAC_CSR0  [ Ver:Rev=0x%08x]\n", pAd->MACVersion));

#ifdef RT5390
	if (IS_RT5390H(pAd))
		DBGPRINT(RT_DEBUG_ERROR, ("The chip belongs to 0x%04x\n", pAd->ChipId));
#endif /* RT5390 */


#ifdef SPECIFIC_BCN_BUF_SUPPORT
	pAd->BcnCB.DYN_MAX_MBSSID_NUM = 1;
	if (IS_RT5390(pAd) || IS_RT3593(pAd))
	{		
		pAd->BcnCB.bHighShareMemSupport = 1;
		pAd->BcnCB.DYN_HW_BEACON_BASE0 = 0x4000;
		pAd->BcnCB.DYN_HW_BEACON_MAX_SIZE = 0x2000;
		pAd->BcnCB.DYN_HW_BEACON_MAX_COUNT = 16;
#ifdef MBSS_SUPPORT
		
#ifdef APCLI_SUPPORT
		pAd->BcnCB.DYN_MAX_MBSSID_NUM = 8 - MAX_MESH_NUM;
#else
		pAd->BcnCB.DYN_MAX_MBSSID_NUM = 16 - MAX_MESH_NUM;
#endif // APCLI_SUPPORT //		
#endif // MBSS_SUPPORT //
	}
	else
	{		
		pAd->BcnCB.bHighShareMemSupport = 0;
		pAd->BcnCB.DYN_HW_BEACON_BASE0 = 0x7800;
		pAd->BcnCB.DYN_HW_BEACON_MAX_SIZE = 0x1000;
		pAd->BcnCB.DYN_HW_BEACON_MAX_COUNT = 8;		
#ifdef MBSS_SUPPORT
		pAd->BcnCB.DYN_MAX_MBSSID_NUM =	(pAd->BcnCB.DYN_HW_BEACON_MAX_COUNT - MAX_MESH_NUM - MAX_APCLI_NUM);
#endif // MBSS_SUPPORT //
	}
#endif // SPECIFIC_BCN_BUF_SUPPORT //

#ifdef RTMP_MAC_PCI
#if defined(RT3090) || defined(RT3592) || defined(RT3390) || defined(RT3593) || defined(RT5390)
	/*Iverson patch PCIE L1 issue to make sure that driver can be read,write ,BBP and RF register  at pcie L.1 level */
	if ((IS_RT3090(pAd) || IS_RT3572(pAd) ||
		IS_RT3390(pAd) || IS_RT3593(pAd) || IS_RT5390(pAd))&&pAd->infType==RTMP_DEV_INF_PCIE)
	{
		RTMP_IO_READ32(pAd, AUX_CTRL, &MacCsr0);
		MacCsr0 |= 0x402;
		RTMP_IO_WRITE32(pAd, AUX_CTRL, MacCsr0);
		DBGPRINT(RT_DEBUG_TRACE, ("AUX_CTRL = 0x%x\n", MacCsr0));
	}
#endif // RT3090 //

	// To fix driver disable/enable hang issue when radio off
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x2);
#endif // RTMP_MAC_PCI //

	// Disable DMA
	RT28XXDMADisable(pAd);


	// Load 8051 firmware
	Status = NICLoadFirmware(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		goto err1;
	}

	NICLoadRateSwitchingParams(pAd);

	// Disable interrupts here which is as soon as possible
	// This statement should never be true. We might consider to remove it later
#ifdef RTMP_MAC_PCI
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}
#endif // RTMP_MAC_PCI //

#ifdef RESOURCE_PRE_ALLOC
	Status = RTMPInitTxRxRingMemory(pAd);
#else
	Status = RTMPAllocTxRxRingMemory(pAd);
#endif // RESOURCE_PRE_ALLOC //

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPAllocTxRxMemory failed, Status[=0x%08x]\n", Status));
		goto err2;
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);

	// initialize MLME
	//

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
#endif // VIDEO_TURBINE_SUPPORT //
#endif // RMTP_RBUS_SUPPORT //

	// Initialize pAd->StaCfg, pAd->ApCfg, pAd->CommonCfg to manufacture default
	//
	UserCfgInit(pAd);

	Status = RtmpNetTaskInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err5;

//	COPY_MAC_ADDR(pAd->ApCfg.MBSSID[apidx].Bssid, netif->hwaddr);
//	pAd->bForcePrintTX = TRUE;

	CfgInitHook(pAd);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		APInitialize(pAd);
#endif // CONFIG_AP_SUPPORT //	

#ifdef BLOCK_NET_IF
	initblockQueueTab(pAd);
#endif // BLOCK_NET_IF //

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

	//
	// Init the hardware, we need to init asic before read registry, otherwise mac register will be reset
	//
	Status = NICInitializeAdapter(pAd, TRUE);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICInitializeAdapter failed, Status[=0x%08x]\n", Status));
		if (Status != NDIS_STATUS_SUCCESS)
		goto err6;
	}	

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
	}
#endif // CONFIG_AP_SUPPORT //

	// Read parameters from Config File 
	/* unknown, it will be updated in NICReadEEPROMParameters */
	pAd->RfIcType = RFIC_UNKNOWN;
	Status = RTMPReadParametersHook(pAd);

	DBGPRINT(RT_DEBUG_OFF, ("1. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPReadParametersHook failed, Status[=0x%08x]\n",Status));
		goto err6;
	}



#ifdef DOT11_N_SUPPORT
   	//Init Ba Capability parameters.
//	RT28XX_BA_INIT(pAd);
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	// UPdata to HT IE
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
#endif // DOT11_N_SUPPORT //

	// after reading Registry, we now know if in AP mode or STA mode

	// Load 8051 firmware; crash when FW image not existent
	// Status = NICLoadFirmware(pAd);
	// if (Status != NDIS_STATUS_SUCCESS)
	//    break;

	DBGPRINT(RT_DEBUG_OFF, ("2. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	// We should read EEPROM for all cases.  rt2860b
	NICReadEEPROMParameters(pAd, (PUCHAR)pDefaultMac);	

	DBGPRINT(RT_DEBUG_OFF, ("3. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	NICInitAsicFromEEPROM(pAd); //rt2860b
	

#ifdef RTMP_INTERNAL_TX_ALC
	//
	// Initialize the desired TSSI table
	//
	InitDesiredTSSITable(pAd);
#endif // RTMP_INTERNAL_TX_ALC //

 #if defined(RT5390)  || defined(RT5370)
 	//
	// Temperature compensation, initialize the lookup table
	//
	DBGPRINT(RT_DEBUG_ERROR, ("IS_RT5392 = %d, bAutoTxAgcG = %d\n", IS_RT5392(pAd), pAd->bAutoTxAgcG));
	if (IS_RT5392(pAd) && pAd->bAutoTxAgcG && pAd->CommonCfg.TempComp != 0)
	{
		InitLookupTable(pAd);
	}
#endif // defined(RT5390)  || defined(RT5370) //

	// Set PHY to appropriate mode
	TmpPhy = pAd->CommonCfg.PhyMode;
	pAd->CommonCfg.PhyMode = 0xff;
	RTMPSetPhyMode(pAd, TmpPhy);
#ifdef DOT11_N_SUPPORT
	SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //

	// No valid channels.
	if (pAd->ChannelListNum == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n"));
		goto err6;
	}

#ifdef DOT11_N_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("MCS Set = %02x %02x %02x %02x %02x\n", pAd->CommonCfg.HtCapability.MCSSet[0],
           pAd->CommonCfg.HtCapability.MCSSet[1], pAd->CommonCfg.HtCapability.MCSSet[2],
           pAd->CommonCfg.HtCapability.MCSSet[3], pAd->CommonCfg.HtCapability.MCSSet[4]));
#endif // DOT11_N_SUPPORT //


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef AP_QLOAD_SUPPORT
		/* init QBSS Element */
		QBSS_LoadInit(pAd);
#endif // AP_QLOAD_SUPPORT //

//#ifdef DOT11K_RRM_SUPPORT
//		RRM_CfgInit(pAd);
//#endif // DOT11K_RRM_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //

//		APInitialize(pAd);

#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif // IKANOS_VX_1X0 //


#ifdef CONFIG_AP_SUPPORT
	//
	// Initialize RF register to default value
	//
	if (pAd->OpMode == OPMODE_AP)
	{
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
	}
#endif // CONFIG_AP_SUPPORT //

	/*
		Some modules init must be called before APStartUp().
		Or APStartUp() will make up beacon content and call
		other modules API to get some information to fill.
	*/
#ifdef WMM_ACM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	ACMP_Init(pAd,
			pAd->CommonCfg.APEdcaParm.bACM[0],
			pAd->CommonCfg.APEdcaParm.bACM[1],
			pAd->CommonCfg.APEdcaParm.bACM[2],
			pAd->CommonCfg.APEdcaParm.bACM[3], 0);
#endif // CONFIG_AP_SUPPORT //
#endif // WMM_ACM_SUPPORT //


	if (pAd && (Status != NDIS_STATUS_SUCCESS))
	{
		//
		// Undo everything if it failed
		//
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		{
//			NdisMDeregisterInterrupt(&pAd->Interrupt);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
		}
//		RTMPFreeAdapter(pAd); // we will free it in disconnect()
	}
	else if (pAd)
	{
		// Microsoft HCT require driver send a disconnect event after driver initialization.
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MEDIA_STATE_CHANGE);

		DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.bAutoChannelAtBootup || (pAd->CommonCfg.Channel == 0))
			{
				UINT8 BBPValue = 0;
				
				// Enable Interrupt first due to we need to scan channel to receive beacons.
				RTMP_IRQ_ENABLE(pAd);
				// Now Enable RxTx
				RTMPEnableRxTx(pAd);
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

				// Let BBP register at 20MHz to do scan		
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
				BBPValue &= (~0x18);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
				DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));

				// Now we can receive the beacon and do the listen beacon
				// use default BW to select channel
				pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, pAd->ApCfg.AutoChannelAlg);
				pAd->ApCfg.bAutoChannelAtBootup = FALSE;
			}

#ifdef DOT11_N_SUPPORT
			// If phymode > PHY_11ABGN_MIXED and BW=40 check extension channel, after select channel  
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
//			RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //
#ifdef RT3090
#ifdef TONE_RADAR_DETECT_SUPPORT
	if (IS_RT3090A(pAd) || IS_RT3390(pAd) || IS_RT5390(pAd))
		pAd->CommonCfg.carrier_func=TONE_RADAR_V2;
	else
		pAd->CommonCfg.carrier_func=TONE_RADAR_V1;
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // RT3090 //
			APStartUp(pAd);
			DBGPRINT(RT_DEBUG_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n", 
									PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].Bssid)));
		}
#endif // CONFIG_AP_SUPPORT //

	}// end of else

#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		INT apidx;
#ifdef HOSTAPD_SUPPORT
		if (pAd->ApCfg.Hostapd == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		}
		else
#endif //HOSTAPD_SUPPORT//
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
			PWSC_CTRL pWscControl;
			UCHAR zeros16[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			
			pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
			DBGPRINT(RT_DEBUG_TRACE, ("Generate UUID for apidx(%d)\n", apidx));
			if (NdisEqualMemory(&pWscControl->Wsc_Uuid_E[0], zeros16, UUID_LEN_HEX))
				WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], apidx, FALSE);
			WscInit(pAd, FALSE, apidx);
		}
	}
#endif // CONFIG_AP_SUPPORT //


	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Init(pAd);
#endif // WSC_INCLUDED //

	// Set up the Mac address
	RtmpOSNetDevAddrSet(pAd->net_dev, &pAd->CurrentAddress[0]);

	// Various AP function init
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MBSS_SUPPORT
		/* the function can not be moved to RT2860_probe() even register_netdev()
		   is changed as register_netdevice().
		   Or in some PC, kernel will panic (Fedora 4) */
		RT28xx_MBSS_Init(pAd, pAd->net_dev);
#endif // MBSS_SUPPORT //

#ifdef WDS_SUPPORT
		RT28xx_WDS_Init(pAd, pAd->net_dev);
#endif // WDS_SUPPORT //

#ifdef APCLI_SUPPORT
		RT28xx_ApCli_Init(pAd, pAd->net_dev);
#endif // APCLI_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //



#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		MATEngineInit(pAd);
#endif // MAT_SUPPORT //

#ifdef CLIENT_WDS
	CliWds_ProxyTabInit(pAd);
#endif // CLIENT_WDS //
	}
#endif // CONFIG_AP_SUPPORT //






#ifdef RT33xx
	if (IS_RT3390(pAd))
	{
		RTMP_TxEvmCalibration(pAd);
	}	
#endif // RT33xx //



	DBGPRINT_S(Status, ("<==== rt28xx_init, Status=%x\n", Status));

	return TRUE;

err6:	
	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);
err5:	
	RtmpNetTaskExit(pAd);
	UserCfgExit(pAd);
err4:	
	MlmeHalt(pAd);
err3:	
	RtmpMgmtTaskExit(pAd);
err2:
#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	RTMPFreeTxRxRingMemory(pAd);
#endif // RESOURCE_PRE_ALLOC //

err1:

#ifdef DOT11_N_SUPPORT
	if(pAd->mpdu_blk_pool.mem)
		os_free_mem(pAd, pAd->mpdu_blk_pool.mem); // free BA pool
#endif // DOT11_N_SUPPORT //

	// shall not set priv to NULL here because the priv didn't been free yet.
	//net_dev->priv = 0;
#ifdef INF_AMAZON_SE
err0:
#endif // INF_AMAZON_SE //
#ifdef ST
err0:
#endif // ST //

	DBGPRINT(RT_DEBUG_ERROR, ("!!! rt28xx Initialized fail !!!\n"));
	return FALSE;
}


/* End of rtmp_init_inf.c */
