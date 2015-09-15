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

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
    	// If dirver doesn't wake up firmware here,
    	// NICLoadFirmware will hang forever when interface is up again.
    	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) &&
        	OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
    	{
        	AUTO_WAKEUP_STRUC AutoWakeupCfg;
			AsicForceWakeup(pAd, TRUE);
        	AutoWakeupCfg.word = 0;
	    	RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
        	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
    	}
	}
#endif // PCIE_PS_SUPPORT //
#endif // CONFIG_STA_SUPPORT //


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

		if ((pAd->MACVersion != 0x00) && (pAd->MACVersion != 0xFFFFFFFF))
			break;

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			goto err1;
		
		RTMPusecDelay(10);
	} while (index++ < 100);
	DBGPRINT(RT_DEBUG_TRACE, ("MAC_CSR0  [ Ver:Rev=0x%08x]\n", pAd->MACVersion));

#ifdef RTMP_MAC_PCI

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

	// Read parameters from Config File 
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
#ifdef CONFIG_STA_SUPPORT
#endif // CONFIG_STA_SUPPORT //

	DBGPRINT(RT_DEBUG_OFF, ("3. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	NICInitAsicFromEEPROM(pAd); //rt2860b
	
#ifdef RTMP_INTERNAL_TX_ALC
	//
	// Initialize the desired TSSI table
	//
	InitDesiredTSSITable(pAd);
#endif // RTMP_INTERNAL_TX_ALC //

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

	}
#endif // CONFIG_AP_SUPPORT //

//		APInitialize(pAd);

#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif // IKANOS_VX_1X0 //


		//
	// Initialize RF register to default value
	//
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);		

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
#ifdef CONFIG_STA_SUPPORT
	ACMP_Init(pAd);
#endif // CONFIG_STA_SUPPORT //
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
//		pAd->IndicateMediaState = NdisMediaStateDisconnected;
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
#ifdef RELASE_EXCLUDE
	/*
	3090, 3090A and 3390 all support hadware tone radar function. But the soluation of those are different.
	3090 is the old one.
	*/
#endif // RELASE_EXCLUDE //
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

			if (pWscControl->WscEnrolleePinCode == 0)
			{
				pWscControl->WscEnrolleePinCode = GenerateWpsPinCode(pAd, FALSE, apidx);
				pWscControl->WscEnrolleePinCodeLen = 8;
			}
		}
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		PWSC_CTRL pWscControl = &pAd->StaCfg.WscControl;
		
		WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], 0, FALSE);
		WscInit(pAd, BSS0);
		if (pWscControl->WscEnrolleePinCode == 0)
		{
			pWscControl->WscEnrolleePinCode = GenerateWpsPinCode(pAd, BSS0);
			pWscControl->WscEnrolleePinCodeLen = 8;
		}
	}
#endif // CONFIG_STA_SUPPORT //

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


#ifdef RTMP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
	VideoTurbineDynamicTune(pAd);
#endif // VIDEO_TURBINE_SUPPORT //
#endif // RTMP_RBUS_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
#ifdef RT3XXX_ANTENNA_DIVERSITY_SUPPORT
	RT3XXX_AntDiversity_Init(pAd);
#endif // RT3XXX_ANTENNA_DIVERSITY_SUPPORT //
#endif // RTMP_RBUS_SUPPORT //

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

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef DOT11Z_TDLS_SUPPORT
		TDLS_Table_Init(pAd);
#endif // DOT11Z_TDLS_SUPPORT //

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		// send wireless event to wpa_supplicant for infroming interface up.
		RtmpOSWrielessEventSend(pAd, IWEVCUSTOM, RT_INTERFACE_UP, NULL, NULL, 0);
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
#endif // WPA_SUPPLICANT_SUPPORT //

	}
#endif // CONFIG_STA_SUPPORT //

#if defined(RT2883) || defined(RT3883)
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (IS_RT2883(pAd)  || IS_RT3883(pAd))
		{
#ifdef RANGE_EXT_SUPPORT
			RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xedcba980);
#endif // RANGE_EXT_SUPPORT //
			RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, 0x12111008);
			RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, 0x16151413);
		}

#ifdef STREAM_MODE_SUPPORT
		if (pAd->CommonCfg.StreamMode > 0)
		{
			ULONG streamWord = StreamModeRegVal(pAd);

			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_L, (ULONG)(pAd->CommonCfg.StreamModeMac[0][0]) | (ULONG)(pAd->CommonCfg.StreamModeMac[0][1] << 8)  | 
							(ULONG)(pAd->CommonCfg.StreamModeMac[0][2] << 16) | (ULONG)(pAd->CommonCfg.StreamModeMac[0][3] << 24));
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_H, streamWord | (ULONG)(pAd->CommonCfg.StreamModeMac[0][4]) | (ULONG)(pAd->CommonCfg.StreamModeMac[0][5] << 8));
			
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR1_L, (ULONG)(pAd->CommonCfg.StreamModeMac[1][0]) | (ULONG)(pAd->CommonCfg.StreamModeMac[1][1] << 8)  | 
						(ULONG)(pAd->CommonCfg.StreamModeMac[1][2] << 16) | (ULONG)(pAd->CommonCfg.StreamModeMac[1][3] << 24));
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR1_H, streamWord | (ULONG)(pAd->CommonCfg.StreamModeMac[1][4]) | (ULONG)(pAd->CommonCfg.StreamModeMac[1][5] << 8));

			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR2_L, (ULONG)(pAd->CommonCfg.StreamModeMac[2][0]) | (ULONG)(pAd->CommonCfg.StreamModeMac[2][1] << 8)  | 
						(ULONG)(pAd->CommonCfg.StreamModeMac[2][2] << 16) | (ULONG)(pAd->CommonCfg.StreamModeMac[2][3] << 24));
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR2_H, streamWord | (ULONG)(pAd->CommonCfg.StreamModeMac[2][4]) | (ULONG)(pAd->CommonCfg.StreamModeMac[2][5] << 8));

			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR3_L, (ULONG)(pAd->CommonCfg.StreamModeMac[3][0]) | (ULONG)(pAd->CommonCfg.StreamModeMac[3][1] << 8)  | 
						(ULONG)(pAd->CommonCfg.StreamModeMac[3][2] << 16) | (ULONG)(pAd->CommonCfg.StreamModeMac[3][3] << 24));
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR3_H, streamWord | (ULONG)(pAd->CommonCfg.StreamModeMac[3][4]) | (ULONG)(pAd->CommonCfg.StreamModeMac[3][5] << 8));

		}
#endif // STREAM_MODE_SUPPORT //
	
	}
#endif // CONFIG_AP_SUPPORT //

	if (pAd->CommonCfg.FineAGC)
	{
		UINT8 BBPValue = 0;
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BBPValue);
		BBPValue |= 0x40; // turn on fine AGC
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, BBPValue);
	}
	else
	{
		UINT8 BBPValue = 0;
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BBPValue);
		BBPValue &= ~0x40; // turn off fine AGC
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, BBPValue);
	}
	
#endif // defined(RT2883) || defined(RT3883) //

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
#endif // TXBF_SUPPORT //
#endif // DOT11_N_SUPPORT //

#ifdef RT3350
	if(1)
	{
	        USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x120, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;
		
		if(rf_offset == 0xff)
		        rf_offset = RF_R09;
		if(rf_value == 0xff)
			rf_value = 0x01;

		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
		
		RT28xx_EEPROM_READ16(pAd, 0x122, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		        rf_offset = RF_R19;
		if(rf_value == 0xff)
			rf_value = 0xcc;

		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	}

	if(pAd->CommonCfg.PhyMode == PHY_11B)
	{
	        USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x126, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R21;
		if(rf_value == 0xff)
		    rf_value = 0x4F;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	
		RT28xx_EEPROM_READ16(pAd, 0x12a, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R29;
		if(rf_value == 0xff)
		    rf_value = 0x07;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	

		// set RF_R24
		if(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
		{    
			value = 0x3F;
		}
		else
		{
			value = 0x1F;
		}
		RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)value);


	}
	else
	{
	        USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x124, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R21;
		if(rf_value == 0xff)
		    rf_value = 0x6F;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	
		RT28xx_EEPROM_READ16(pAd, 0x128, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R29;
		if(rf_value == 0xff)
		    rf_value = 0x07;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	
		// set RF_R24
		if(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
		{    
			value = 0x28;
		}
		else
		{
			value = 0x18;
		}
		RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)value);

	}
	
#endif // RT3350 //


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
