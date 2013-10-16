/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************
 
    Module Name:
    mlme.c
 
    Abstract:
    Major MLME state machiones here
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"
#include <stdarg.h>


#ifdef DOT11_N_SUPPORT

int DetectOverlappingPeriodicRound;


#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	int apidx;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("Bss2040CoexistTimeOut(): Recovery to original setting!\n"));
	
	// Recovery to original setting when next DTIM Interval.
	pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_TIMER_FIRED);
	NdisZeroMemory(&pAd->CommonCfg.LastBSSCoexist2040, sizeof(BSS_2040_COEXIST_IE));
	pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
	
	if (pAd->CommonCfg.bBssCoexEnable == FALSE)
	{
		// TODO: Find a better way to handle this when the timer is fired and we disable the bBssCoexEable support!!
		DBGPRINT(RT_DEBUG_TRACE, ("Bss2040CoexistTimeOut(): bBssCoexEnable is FALSE, return directly!\n"));
		return;
	}
	
	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID, apidx, 0);
	
}
#endif // DOT11N_DRAFT3 //

#endif // DOT11_N_SUPPORT //


VOID APDetectOverlappingExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
#ifdef DOT11_N_SUPPORT
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	if (DetectOverlappingPeriodicRound == 0)
	{
		// switch back 20/40		
		if ((pAd->CommonCfg.Channel <=14) && (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40))
		{
            pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 1;	
			pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;			
		}
	}
	else
	{
		if ((DetectOverlappingPeriodicRound == 25) || (DetectOverlappingPeriodicRound == 1))
		{   
   			if ((pAd->CommonCfg.Channel <=14) && (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth==BW_40))
			{                                     
				SendBeaconRequest(pAd, 1);			
				SendBeaconRequest(pAd, 2);
                SendBeaconRequest(pAd, 3);
			}

		}
		DetectOverlappingPeriodicRound--;
	}


#endif // DOT11_N_SUPPORT //
}


/*
    ==========================================================================
    Description:
        This routine is executed every second -
        1. Decide the overall channel quality
        2. Check if need to upgrade the TX rate to any client
        3. perform MAC table maintenance, including ageout no-traffic clients, 
           and release packet buffer in PSQ is fail to TX in time.
    ==========================================================================
 */
VOID APMlmePeriodicExec(
    PRTMP_ADAPTER pAd)
{
    // Reqeust by David 2005/05/12
    // It make sense to disable Adjust Tx Power on AP mode, since we can't take care all of the client's situation    
    // ToDo: need to verify compatibility issue with WiFi product.
    // 

	/*
		We return here in ATE mode, because the statistics 
		that ATE need are not collected via this routine.
	*/
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif // RALINK_ATE //
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef TONE_RADAR_DETECT_SUPPORT
	if (isCarrierDetectExist(pAd) == TRUE)
	{
		if (pAd->CommonCfg.CarrierDetect.OneSecIntCount < 10)
		{
			pAd->CommonCfg.CarrierDetect.CD_State = CD_NORMAL;
			pAd->CommonCfg.CarrierDetect.recheck = pAd->CommonCfg.CarrierDetect.recheck1;
			if (pAd->CommonCfg.CarrierDetect.CarrierDebug == 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Carrier gone\n"));
				// start all TX actions.
				APMakeAllBssBeacon(pAd);
				APUpdateAllBeaconFrame(pAd);
				AsicEnableBssSync(pAd);
			}
			else
			{
				printk("Carrier gone\n");
			}
		}
	}
	pAd->CommonCfg.CarrierDetect.OneSecIntCount = 0;
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //

#if defined(RT305x)||defined(RT3070)
	// request by Gary, if Rssi0 > -42, BBP 82 need to be changed from 0x62 to 0x42, , bbp 67 need to be changed from 0x20 to 0x18
	if (!pAd->CommonCfg.HighPowerPatchDisabled)
	{
//#ifdef RT305x
#ifndef RT3350
	UCHAR RFValue;
#endif // #ifndef RT3350 //
//#endif // RT305x //


#ifdef RT305x
		// TODO: need to check chip version here!!!
#ifndef RT3350
		if ((pAd->ApCfg.RssiSample.AvgRssi0 != 0) && (pAd->ApCfg.RssiSample.AvgRssi0 > (pAd->BbpRssiToDbmDelta - 35) ))
		{ 
		    RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)&RFValue);
		    RFValue &= ~0x3;
		    RT30xxWriteRFRegister(pAd, RF_R27, (UCHAR)RFValue);

		    RT30xxReadRFRegister(pAd, RF_R28, (PUCHAR)&RFValue);
		    RFValue &= ~0x3;
		    RT30xxWriteRFRegister(pAd, RF_R28, (UCHAR)RFValue);
		}
        else
		{
		    RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)&RFValue);
		    RFValue |= 0x3;
		    RT30xxWriteRFRegister(pAd, RF_R27, (UCHAR)RFValue);

		    RT30xxReadRFRegister(pAd, RF_R28, (PUCHAR)&RFValue);
		    RFValue |= 0x3;
		    RT30xxWriteRFRegister(pAd, RF_R28, (UCHAR)RFValue);
		}
#endif // RT3350 //

		// request by Gary, if Rssi0 > -42, BBP 82 need to be changed from 0x62 to 0x42,
		// bbp 67 need to be changed from 0x20 to 0x18 (Only 2R have High Power issue)
		if (pAd->Antenna.field.RxPath == 2)
		{
			if ((pAd->ApCfg.RssiSample.AvgRssi0 != 0) && (pAd->ApCfg.RssiSample.AvgRssi0 > (pAd->BbpRssiToDbmDelta - 42) ))
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x42);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, 0x18);
			}
			else
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, 0x20);
			}
		}
#endif // RT305x //
	}

#ifdef RT305x
#ifndef RT3350
	if ((pAd->CommonCfg.CID == 0x103) || ((pAd->CommonCfg.CN >> 16) == 0x3333))
	{
	    RT30xxWriteRFRegister(pAd, RF_R27, 0x21);		    	
	    RT30xxWriteRFRegister(pAd, RF_R28, 0x10);	
	}
#endif // RT3350 //
#endif // RT305x //

#endif // defined(RT305x)||defined(RT3070) //

	// Disable Adjust Tx Power for WPA WiFi-test. 
	// Because high TX power results in the abnormal disconnection of Intel BG-STA.  
//#ifndef WIFI_TEST    
//	if (pAd->CommonCfg.bWiFiTest == FALSE)
	/* for SmartBit 64-byte stream test */
	/* removed based on the decision of Ralink congress at 2011/7/06 */
/*	if (pAd->MacTab.Size > 0) */
		AsicAdjustTxPower(pAd);
//#endif // WIFI_TEST //

	/* BBP TUNING: dynamic tune BBP R66 to find a balance between sensibility
		and noise isolation */
//	AsicBbpTuning2(pAd);

    // walk through MAC table, see if switching TX rate is required

    // MAC table maintenance
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0)
	{
		// one second timer
	    MacTableMaintenance(pAd);

		RTMPMaintainPMKIDCache(pAd);

#ifdef WDS_SUPPORT
		WdsTableMaintenance(pAd);
#endif // WDS_SUPPORT //


#ifdef CLIENT_WDS
	CliWds_ProxyTabMaintain(pAd);
#endif // CLIENT_WDS //
	}
	
	APUpdateCapabilityAndErpIe(pAd);

#ifdef APCLI_SUPPORT
	if (pAd->Mlme.OneSecPeriodicRound % 2 == 0)
		ApCliIfMonitor(pAd);

	if (pAd->Mlme.OneSecPeriodicRound % 2 == 1)
		ApCliIfUp(pAd);

	{
		INT loop;
		ULONG Now32;
		NdisGetSystemUpTime(&Now32);
		for (loop = 0; loop < MAX_APCLI_NUM; loop++)
		{
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[loop];
			if ((pApCliEntry->Valid == TRUE)
				&& (pApCliEntry->MacTabWCID < MAX_LEN_OF_MAC_TABLE))
			{
				// update channel quality for Roaming and UI LinkQuality display
				MlmeCalculateChannelQuality(pAd,
					&pAd->MacTab.Content[pApCliEntry->MacTabWCID], Now32);
			}
		}
	}
#endif // APCLI_SUPPORT //

#ifdef DOT11_N_SUPPORT
    if (pAd->CommonCfg.bHTProtect)
    {
    	//APUpdateCapabilityAndErpIe(pAd);
    	APUpdateOperationMode(pAd);
		if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
		{
        	AsicUpdateProtect(pAd, (USHORT)pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, pAd->MacTab.fAnyStationNonGF);
    	}
    }
#endif // DOT11_N_SUPPORT //

#ifdef A_BAND_SUPPORT
	if ( (pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		)
	{
#ifdef DFS_SUPPORT
		ApRadarDetectPeriodic(pAd);
#else
		pAd->CommonCfg.RadarDetect.InServiceMonitorCount++;
		if (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
		{
			if (pAd->CommonCfg.RadarDetect.RDCount++ > pAd->CommonCfg.RadarDetect.ChMovingTime)
			{
#ifdef CARRIER_DETECTION_SUPPORT
#ifndef TONE_RADAR_DETECT_SUPPORT
				if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
				{
					// trun on Carrier-Detection. (Carrier-Detect with CTS protection).
					CarrierDetectionStart(pAd, 1);
				}
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //
				AsicEnableBssSync(pAd);
				pAd->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;
			}
		}
#endif // DFS_SUPPORT //
	}

#ifdef DFS_SUPPORT
#if defined(RT2880) || defined(RT2860)
#ifdef DFS_SOFTWARE_SUPPORT
	if ( (pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel)
		&& OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
		&& (pAd->CommonCfg.RadarDetect.RDMode == RD_NORMAL_MODE))
	{
		AdaptRadarDetection(pAd);
	}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // defined(RT2880) || defined(RT2860) //
#endif // DFS_SUPPORT //
#endif // A_BAND_SUPPORT //


}

VOID APMlmeSelectTxRateTable(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PUCHAR				*ppTable,
	IN PUCHAR				pTableSize,
	IN PUCHAR				pInitTxRateIdx)
{
	MlmeSelectTxRateTable(pAd, pEntry, ppTable, pTableSize, pInitTxRateIdx);
}

VOID APMlmeSetTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PRTMP_TX_RATE_SWITCH	pTxRate)
{
#ifdef DOT11_N_SUPPORT
	if ((pTxRate->STBC) && (pEntry->MaxHTPhyMode.field.STBC))
		pEntry->HTPhyMode.field.STBC = STBC_USE;
	else
		pEntry->HTPhyMode.field.STBC = STBC_NONE;

	if (((pTxRate->ShortGI) && (pEntry->MaxHTPhyMode.field.ShortGI))
         || (pAd->WIFItestbed.bShortGI && pEntry->MaxHTPhyMode.field.ShortGI) )
		pEntry->HTPhyMode.field.ShortGI = GI_400;
	else
		pEntry->HTPhyMode.field.ShortGI = GI_800;
#endif // DOT11_N_SUPPORT //

	if (pTxRate->CurrMCS < MCS_AUTO)
		pEntry->HTPhyMode.field.MCS = pTxRate->CurrMCS;

#ifdef DOT11_N_SUPPORT
	if (pTxRate->Mode <= MODE_HTGREENFIELD)
#endif // DOT11_N_SUPPORT //
		pEntry->HTPhyMode.field.MODE = pTxRate->Mode;

#ifdef DOT11_N_SUPPORT
	if ((pAd->WIFItestbed.bGreenField & pEntry->HTCapability.HtCapInfo.GF) && (pEntry->HTPhyMode.field.MODE == MODE_HTMIX))
	{
		// force Tx GreenField 
		pEntry->HTPhyMode.field.MODE = MODE_HTGREENFIELD;
	}

	// BW depends on BSSWidthTrigger and Negotiated BW
	if (pAd->CommonCfg.bRcvBSSWidthTriggerEvents ||
		(pEntry->MaxHTPhyMode.field.BW==BW_20) ||
		(pAd->CommonCfg.BBPCurrentBW==BW_20)
	)
		pEntry->HTPhyMode.field.BW = BW_20;
	else	
		pEntry->HTPhyMode.field.BW = BW_40;

#ifdef RANGE_EXT_SUPPORT
	// 20 MHz Fallback
	if (pTxRate->Mode>=MODE_HTMIX && pEntry->HTPhyMode.field.BW==BW_40 && ADAPT_RATE_TABLE(pEntry->pTable))
	{
		if ((pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0)==0 &&
			pEntry->HTPhyMode.field.MCS==32)
	{
			// Map HT Duplicate to 20MHz MCS0
		pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 0;
		}
		else if ((pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS1)==0 &&
			pEntry->HTPhyMode.field.MCS==0 &&
			(pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)==0)
		{
			// Map 40MHz MCS0 to 20MHz MCS1
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 1;
		}
		else if ((pAd->CommonCfg.DebugFlags & DBF_ENABLE_20MHZ_MCS8) &&
			pEntry->HTPhyMode.field.MCS==8)
		{
			// Map 40MHz MCS8 to 20MHz MCS8
			pEntry->HTPhyMode.field.BW = BW_20;
		}
	}

	// Debug Option: Force BW
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_40MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_40;
	}
	else if (pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_20;
	}

	// Reexam each bandwidth's SGI support.
	if ((pEntry->HTPhyMode.field.BW==BW_20 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
		(pEntry->HTPhyMode.field.BW==BW_40 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)) )
		pEntry->HTPhyMode.field.ShortGI = GI_800;

	// Debug option: Force Short GI
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_SGI)
		pEntry->HTPhyMode.field.ShortGI = GI_400;
#endif // RANGE_EXT_SUPPORT //
#endif // DOT11_N_SUPPORT //

#ifdef FIFO_EXT_SUPPORT
	if (pEntry->Aid >= 1 && pEntry->Aid <= 8)
	{
		WCID_TX_CNT_STRUC wcidTxCnt;
		UINT32 regAddr;

		regAddr = WCID_TX_CNT_0 + (pEntry->Aid - 1) * 4;
		RTMP_IO_READ32(pAd, regAddr, &wcidTxCnt.word);
	}
#endif // FIFO_EXT_SUPPORT //

	
}


#ifdef NEW_RATE_ADAPT_SUPPORT
/*
    ==========================================================================
    Description:
        This routine walks through the MAC table, see if TX rate change is 
        required for each associated client. 
    Output:
        pEntry->CurrTxRate - 
    NOTE:
        call this routine every second
    ==========================================================================
 */
VOID APMlmeDynamicTxRateSwitchingAdapt(
    IN PRTMP_ADAPTER pAd, IN ULONG i)
{
	PUCHAR					pTable;
	UCHAR					UpRateIdx, DownRateIdx, CurrRateIdx;
	ULONG					TxTotalCnt;
	ULONG					TxErrorRatio = 0;
	MAC_TABLE_ENTRY			*pEntry;
	PRTMP_TX_RATE_SWITCH_3S	pCurrTxRate;
	UCHAR					TrainUp, TrainDown;
	CHAR					Rssi;
	ULONG					TxSuccess, TxRetransmit, TxFailCount;
#ifdef DOT11N_PF_DEBUG
	UCHAR					pEntryMaxMCS = 0, selfMaxMCS = 0;
#endif // DOT11N_PF_DEBUG //

	pEntry = &pAd->MacTab.Content[i]; //point to information of the individual station
	pTable = pEntry->pTable;

	if (pAd->MacTab.Size == 1)
	{
		TX_STA_CNT1_STRUC	StaTx1;
		TX_STA_CNT0_STRUC	TxStaCnt0;

		// Update statistic counter
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

		TxRetransmit = StaTx1.field.TxRetransmit;
		TxSuccess = StaTx1.field.TxSuccess;
		TxFailCount = TxStaCnt0.field.TxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;			
	}
	else
	{
		TxRetransmit = pEntry->OneSecTxRetryOkCount;
		TxSuccess = pEntry->OneSecTxNoRetryOkCount;
		TxFailCount = pEntry->OneSecTxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

#ifdef FIFO_EXT_SUPPORT
		if (pEntry->Aid >= 1 && pEntry->Aid <= 8)
		{
			ULONG 	HwTxCnt, HwErrRatio;

			NicGetMacFifoTxCnt(pAd, pEntry);
			HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
			if (HwTxCnt)
				HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
			else
				HwErrRatio = 0;
			DBGPRINT(RT_DEBUG_TRACE | DBG_FUNC_RA,("%s():Aid:%d, MCS:%d, TxErrRatio(Hw:0x%lx-0x%lx, Sw:0x%lx-%lx)\n", 
					__FUNCTION__, pEntry->Aid, pEntry->HTPhyMode.field.MCS, 
					HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

			TxSuccess = pEntry->fifoTxSucCnt;
			TxRetransmit = pEntry->fifoTxRtyCnt;
			TxTotalCnt = HwTxCnt;
			TxErrorRatio = HwErrRatio;
		}
#endif // FIFO_EXT_SUPPORT //
	}

	// Save LastTxOkCount, LastTxPER and last MCS action for APQuickResponeForRateUpExec
	pEntry->LastTxOkCount = TxSuccess;
	pEntry->LastTxPER = (UCHAR)TxErrorRatio;
	pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;

	//different calculation in APQuickResponeForRateUpExec()
	//Rssi = RTMPMaxRssi(pAd, (CHAR)pEntry->RssiSample.AvgRssi0, (CHAR)pEntry->RssiSample.AvgRssi1, (CHAR)pEntry->RssiSample.AvgRssi2);
	Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

	// decide the next upgrade rate and downgrade rate, if any
	CurrRateIdx = pEntry->CurrTxRateIndex;
	pCurrTxRate = PTX_RATE_SWITCH_ENTRY_3S(pTable, CurrRateIdx);
	UpRateIdx = MlmeSelectUpRate(pAd, pEntry, pCurrTxRate);
	DownRateIdx = MlmeSelectDownRate(pAd, pEntry, CurrRateIdx);

#ifdef DOT11N_PF_DEBUG
	if (pEntry->HTCapability.MCSSet[0] == 0xff)
	{
		if (pEntry->HTCapability.MCSSet[1] == 0xff)
		{
			if(pEntry->HTCapability.MCSSet[2] == 0xff)
				pEntryMaxMCS = 23;
			else	
				pEntryMaxMCS = 15;
		}
		else
		{
			pEntryMaxMCS = 7;
		}
	}
	
	if(pAd->CommonCfg.TxStream <= 3)
		selfMaxMCS = 8 * pAd->CommonCfg.TxStream - 1;
#endif // DOT11N_PF_DEBUG //

#ifdef DOT11_N_SUPPORT
	//when Rssi > -65, there is a lot of interference usually. therefore, the algorithm tends to choose the mcs lower than the optimal one.
	//by increasing the thresholds, the chosen mcs will be closer to the optimal mcs
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX) && pEntry->perThrdAdj == 1)
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif // DOT11_N_SUPPORT //
	{
		TrainUp		= pCurrTxRate->TrainUp;
		TrainDown	= pCurrTxRate->TrainDown;
	}


	// Debug option: Concise RA log
	if ((pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG) || (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG))
		MlmeRALog(pAd, pEntry, RAL_NEW_DRS, TxErrorRatio, TxTotalCnt);

#ifdef MFB_SUPPORT
	if (pEntry->fLastChangeAccordingMfb == TRUE)
	{
		PRTMP_TX_RATE_SWITCH pNextTxRate;

		//with this method mfb result can be applied every 500msec, instead of immediately
		NdisAcquireSpinLock(&pEntry->fLastChangeAccordingMfbLock);
		pEntry->fLastChangeAccordingMfb = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		NdisReleaseSpinLock(&pEntry->fLastChangeAccordingMfbLock);
		APMlmeSetTxRate(pAd, pEntry, pEntry->LegalMfbRS);
		DBGPRINT(RT_DEBUG_INFO,("DRS: MCS is according to MFB, and ignore tuning this sec \n"));
		MlmeClearAllTxQuality(pEntry);//clear all history, same as train up, purpose???
		// reset all OneSecTx counters
		RESET_ONE_SEC_TX_CNT(pEntry);

		pEntry->CurrTxRateIndex = (pEntry->LegalMfbRS)->ItemNo;
		pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pEntry->CurrTxRateIndex+1)*10];//actually = pEntry->LegalMfbRS	
		return;
	}
#endif	// MFB_SUPPORT //

#if defined (RT2883) || defined (RT3883)
	// Debug Option: Force the MCS. FixedRate is index into current rate table.
	if ((pAd->CommonCfg.FixedRate != -1) && (pAd->CommonCfg.FixedRate < RATE_TABLE_SIZE(pTable)) )
	{
			pEntry->CurrTxRateIndex = pAd->CommonCfg.FixedRate;
		MlmeNewTxRate(pAd, pEntry);

#ifdef DOT11N_SS3_SUPPORT
		// Turn off RDG when 3s and rx count > tx count*5
		MlmeCheckRDG(pAd, pEntry);
#endif // DOT11N_SS3_SUPPORT //

		// reset all OneSecTx counters
		RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
		eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //

		return;
	}
#endif

	// Handle low traffic case
	if (TxTotalCnt <= 15)
	{
		pEntry->lowTrafficCount++;

		if (pEntry->lowTrafficCount >= pAd->CommonCfg.lowTrafficThrd)
		{
			UCHAR	TxRateIdx;
			CHAR	mcs[24];
			CHAR	RssiOffset = 0;

			pEntry->lowTrafficCount = 0;

			// Check existence and get index of each MCS
			MlmeGetSupportedMcsAdapt(pAd, pEntry, GI_400, mcs);

			if ((pTable == RateSwitchTable11BGN2S) || (pTable == RateSwitchTable11BGN2SForABand) ||
				(pTable == RateSwitchTable11N2S) || (pTable == RateSwitchTable11N2SForABand) ||
				(pTable == RateSwitchTable))
		{
			RssiOffset = 2;
		}
			else if (ADAPT_RATE_TABLE(pTable))
		{
			RssiOffset = 0;
		}
		else
		{
			RssiOffset = 5;
		}

			// Select the Tx rate based on the RSSI
			TxRateIdx = MlmeSelectTxRateAdapt(pAd, pEntry, mcs, Rssi, RssiOffset);
		
			//if (TxRateIdx != pEntry->CurrTxRateIndex)
			{
				pEntry->lastRateIdx = pEntry->CurrTxRateIndex;
				MlmeSetMcsGroup(pAd, pEntry);

				pEntry->CurrTxRateIndex = TxRateIdx;
#ifdef TXBF_SUPPORT
				pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
#endif // TXBF_SUPPORT //
				MlmeNewTxRate(pAd, pEntry);
				if (!pEntry->fLastSecAccordingRSSI)
				{
					DBGPRINT(RT_DEBUG_INFO,("DRS: TxTotalCnt <= 15, switch to MCS%d according to RSSI (%d), RssiOffset=%d\n", pEntry->HTPhyMode.field.MCS, Rssi, RssiOffset));
				}
			}

			MlmeClearAllTxQuality(pEntry);	//clear all history
			pEntry->fLastSecAccordingRSSI = TRUE;
		}

		// reset all OneSecTx counters
		RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
		// In Unaware mode always try to send sounding
		if (pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)
			eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //

		return;
	}

	pEntry->lowTrafficCount = 0;

	//after pEntry->fLastSecAccordingRSSI = TRUE; the for loop continue. this condition is true when RateSwitching() is run next time. 
	//so the next rate adaptation is skipped. This mechanism is deliberately designed by rory.
	if (pEntry->fLastSecAccordingRSSI == TRUE)
	{
		pEntry->fLastSecAccordingRSSI = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		//DBGPRINT(RT_DEBUG_INFO,("DRS: MCS is according to RSSI, and ignore tuning this sec \n"));

		// reset all OneSecTx counters
		RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
		eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //

		return;
	}

	pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

	// Select rate based on PER
	MlmeNewRateAdapt(pAd, pEntry, UpRateIdx, DownRateIdx, TrainUp, TrainDown, TxErrorRatio);

#ifdef DOT11N_SS3_SUPPORT
	// Turn off RDG when 3s and rx count > tx count*5
	MlmeCheckRDG(pAd, pEntry);
#endif // DOT11N_SS3_SUPPORT //

	// reset all OneSecTx counters
	RESET_ONE_SEC_TX_CNT(pEntry);
				
#ifdef TXBF_SUPPORT
	eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //
		}
#endif // NEW_RATE_ADAPT_SUPPORT //


/*
    ==========================================================================
    Description:
        This routine walks through the MAC table, see if TX rate change is 
        required for each associated client. 
    Output:
        pEntry->CurrTxRate - 
    NOTE:
        call this routine every second
    ==========================================================================
 */
VOID APMlmeDynamicTxRateSwitching(
    IN PRTMP_ADAPTER pAd)
{
	ULONG					i;
	PUCHAR					pTable;
	UCHAR					TableSize = 0;
	UCHAR					UpRateIdx, DownRateIdx, CurrRateIdx;
	ULONG					TxTotalCnt;
	ULONG					TxErrorRatio = 0;
	MAC_TABLE_ENTRY		*pEntry;
	PRTMP_TX_RATE_SWITCH	pCurrTxRate;
	UCHAR					InitTxRateIdx, TrainUp, TrainDown;
	CHAR					Rssi;
	ULONG					TxSuccess, TxRetransmit, TxFailCount;
	
#ifdef RALINK_ATE
   	if (ATE_ON(pAd))
   	{
		return;
   	}
#endif // RALINK_ATE //

    //
    // walk through MAC table, see if need to change AP's TX rate toward each entry
    //

	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) 
	{
		pEntry = &pAd->MacTab.Content[i]; //point to information of the individual station

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;
#endif // APCLI_SUPPORT //

#ifdef WDS_SUPPORT
		if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->MatchWDSTabIdx))
			continue;
#endif // WDS_SUPPORT //


		// check if this entry need to switch rate automatically
		if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
			continue;


		APMlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
		pEntry->pTable = pTable;

#ifdef NEW_RATE_ADAPT_SUPPORT
		if (ADAPT_RATE_TABLE(pTable))
		{
			APMlmeDynamicTxRateSwitchingAdapt(pAd, i);

			continue;				
		}
#endif // NEW_RATE_ADAPT_SUPPORT //

		//NICUpdateFifoStaCounters(pAd);

		if (pAd->MacTab.Size == 1)
		{
			TX_STA_CNT1_STRUC		StaTx1;
			TX_STA_CNT0_STRUC		TxStaCnt0;

			// Update statistic counter
			NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

			TxRetransmit = StaTx1.field.TxRetransmit;
			TxSuccess = StaTx1.field.TxSuccess;
			TxFailCount = TxStaCnt0.field.TxFailCount;
			TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;			
		}
		else
		{
			TxRetransmit = pEntry->OneSecTxRetryOkCount;
			TxSuccess = pEntry->OneSecTxNoRetryOkCount;
			TxFailCount = pEntry->OneSecTxFailCount;
			TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

#ifdef FIFO_EXT_SUPPORT
			if (pEntry->Aid >= 1 && pEntry->Aid <= 8)
			{
				ULONG 	HwTxCnt, HwErrRatio;

				NicGetMacFifoTxCnt(pAd, pEntry);
				HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
				if (HwTxCnt)
					HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
				else
					HwErrRatio = 0;
				
				DBGPRINT(RT_DEBUG_TRACE | DBG_FUNC_RA,
						("%s():Aid:%d, MCS:%d, CuTxRaIdx=%d,TxErrRatio(Hw:%ld-%ld%%, Sw:%ld-%ld%%)\n", 
						__FUNCTION__, pEntry->Aid, pEntry->HTPhyMode.field.MCS,
						pEntry->CurrTxRateIndex,
						HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

				TxSuccess = pEntry->fifoTxSucCnt;
				TxRetransmit = pEntry->fifoTxRtyCnt;
				TxTotalCnt = HwTxCnt;
				TxErrorRatio = HwErrRatio;
			}
#endif // FIFO_EXT_SUPPORT //
		}

		// Save LastTxOkCount, LastTxPER and last MCS action for APQuickResponeForRateUpExec
		pEntry->LastTxOkCount = TxSuccess;
		pEntry->LastTxPER = (TxTotalCnt == 0 ? 0 : (UCHAR)TxErrorRatio);
		pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;

		//different calculation in APQuickResponeForRateUpExec()
		//Rssi = RTMPMaxRssi(pAd, (CHAR)pEntry->RssiSample.AvgRssi0, (CHAR)pEntry->RssiSample.AvgRssi1, (CHAR)pEntry->RssiSample.AvgRssi2);
		Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

		CurrRateIdx = UpRateIdx = DownRateIdx = pEntry->CurrTxRateIndex;

		// decide the next upgrade rate and downgrade rate, if any
		if ((CurrRateIdx > 0) && (CurrRateIdx < (TableSize - 1)))
		{
			UpRateIdx = CurrRateIdx + 1;
				DownRateIdx = CurrRateIdx -1;
		}
		else if (CurrRateIdx == 0)
		{
			UpRateIdx = CurrRateIdx + 1;
			DownRateIdx = CurrRateIdx;
		}
		else if (CurrRateIdx == (TableSize - 1))
		{
			UpRateIdx = CurrRateIdx;
			DownRateIdx = CurrRateIdx - 1;
		}

		pCurrTxRate = PTX_RATE_SWITCH_ENTRY(pTable, CurrRateIdx);
#ifdef DOT11_N_SUPPORT
		//when Rssi > -65, there is a lot of interference usually. therefore, the algorithm tends to choose the mcs lower than the optimal one.
		//by increasing the thresholds, the chosen mcs will be closer to the optimal mcs
		if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
		{
			TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
			TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
		}
		else
#endif // DOT11_N_SUPPORT //
		{
			TrainUp		= pCurrTxRate->TrainUp;
			TrainDown	= pCurrTxRate->TrainDown;
		}

				

#if defined (RT2883) || defined (RT3883)
		// Debug Option: Force the MCS. FixedRate is index into current rate table.
		if ((pAd->CommonCfg.FixedRate != -1) && (pAd->CommonCfg.FixedRate < RATE_TABLE_SIZE(pTable)))
			{
			pEntry->CurrTxRateIndex = pAd->CommonCfg.FixedRate;
			MlmeNewTxRate(pAd, pEntry);

			// reset all OneSecTx counters
			RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
			eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //

			continue;
		}
#endif

		// Check for low traffic case
        if (TxTotalCnt <= 15)
		{
			UCHAR	TxRateIdx;
			CHAR	mcs[24];

			// Check existence and get the index of each MCS
			MlmeGetSupportedMcs(pAd, pTable, mcs);

			// Select the Tx rate based on the RSSI
			TxRateIdx = MlmeSelectTxRate(pAd, pEntry, mcs, Rssi, 0);


			if (TxRateIdx != pEntry->CurrTxRateIndex
#ifdef TXBF_SUPPORT
				|| pEntry->phyETxBf || pEntry->phyITxBf
#endif // TXBF_SUPPORT //
				)
				{
				pEntry->CurrTxRateIndex = TxRateIdx;
#ifdef TXBF_SUPPORT
				pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
#endif // TXBF_SUPPORT //
				MlmeNewTxRate(pAd, pEntry);
				if (!pEntry->fLastSecAccordingRSSI)
					DBGPRINT(RT_DEBUG_INFO,("DRS: TxTotalCnt <= 15, switch MCS according to RSSI (%d)\n", Rssi));
				}

			MlmeClearAllTxQuality(pEntry);
			pEntry->fLastSecAccordingRSSI = TRUE;

			// reset all OneSecTx counters
			RESET_ONE_SEC_TX_CNT(pEntry);


#ifdef TXBF_SUPPORT
			// In Unaware mode always try to send sounding
			if (pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)
				eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //

			continue;
			}

		if (pEntry->fLastSecAccordingRSSI == TRUE)
				{
			pEntry->fLastSecAccordingRSSI = FALSE;
			pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
			// reset all OneSecTx counters
			RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
			eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //

			continue;
		}

		pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

		// Select rate based on PER
		MlmeOldRateAdapt(pAd, pEntry, CurrRateIdx, UpRateIdx, DownRateIdx, TrainUp, TrainDown, TxErrorRatio);

#ifdef DOT11N_SS3_SUPPORT
		// Turn off RDG when 3s and rx count > tx count*5
		MlmeCheckRDG(pAd, pEntry);
#endif // DOT11N_SS3_SUPPORT //

		// reset all OneSecTx counters
		RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
		eTxBFProbing(pAd, pEntry);
#endif // TXBF_SUPPORT //

    }
}

#ifdef NEW_RATE_ADAPT_SUPPORT
/*
    ========================================================================
    Routine Description:
        AP side, Auto TxRate faster train up timer call back function.
        
    Arguments:
        SystemSpecific1         - Not used.
        FunctionContext         - Pointer to our Adapter context.
        SystemSpecific2         - Not used.
        SystemSpecific3         - Not used.
        
    Return Value:
        None
        
    ========================================================================
*/
VOID APQuickResponeForRateUpExecAdapt(//actually for both up and down
    IN PRTMP_ADAPTER pAd,
    IN ULONG idx) 
{
	PUCHAR					pTable;
	UCHAR					CurrRateIdx;
	ULONG					AccuTxTotalCnt, TxTotalCnt, TxCnt;
	ULONG					TxErrorRatio = 0;
	MAC_TABLE_ENTRY			*pEntry;
	PRTMP_TX_RATE_SWITCH_3S	pCurrTxRate;
	UCHAR					TrainUp, TrainDown;
	CHAR					Rssi, ratio;
	ULONG					TxSuccess, TxRetransmit, TxFailCount;
	ULONG					OneSecTxNoRetryOKRationCount;
	BOOLEAN					rateChanged;
#ifdef TXBF_SUPPORT
	BOOLEAN					CurrPhyETxBf, CurrPhyITxBf;
#endif // TXBF_SUPPORT //

	pEntry = &pAd->MacTab.Content[idx];


	pTable = pEntry->pTable;

	//Rssi = RTMPMaxRssi(pAd, (CHAR)pEntry->RssiSample.AvgRssi0, (CHAR)pEntry->RssiSample.AvgRssi1, (CHAR)pEntry->RssiSample.AvgRssi2);
	Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

	if (pAd->MacTab.Size == 1)
	{
		TX_STA_CNT1_STRUC		StaTx1;
		TX_STA_CNT0_STRUC		TxStaCnt0;

		// Update statistic counter
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

		TxRetransmit = StaTx1.field.TxRetransmit;
		TxSuccess = StaTx1.field.TxSuccess;
		TxFailCount = TxStaCnt0.field.TxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		AccuTxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
							pAd->RalinkCounters.OneSecTxRetryOkCount + 
							pAd->RalinkCounters.OneSecTxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

		//Rssi is calculated again with new formula?In rory's code, the average instead of max is used.
		if (pAd->Antenna.field.TxPath > 1)
			Rssi = (pEntry->RssiSample.AvgRssi0 + pEntry->RssiSample.AvgRssi1) >> 1;
		else
			Rssi = pEntry->RssiSample.AvgRssi0;

		TxCnt = AccuTxTotalCnt;
	}
	else
	{
		TxRetransmit = pEntry->OneSecTxRetryOkCount;
		TxSuccess = pEntry->OneSecTxNoRetryOkCount;
		TxFailCount = pEntry->OneSecTxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		TxCnt = TxTotalCnt;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
	
#ifdef FIFO_EXT_SUPPORT
		if ((pEntry->Aid >= 1) && (pEntry->Aid <= 8))
		{
			ULONG 	HwTxCnt, HwErrRatio;

			NicGetMacFifoTxCnt(pAd, pEntry);
			HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
			if (HwTxCnt)
				HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
			else
				HwErrRatio = 0;
			
			DBGPRINT(RT_DEBUG_TRACE | DBG_FUNC_RA,("%s():Aid:%d, MCS:%d, TxErrRation(Hw:0x%lx-0x%lx, Sw:0x%lx-%lx)\n", 
					__FUNCTION__, pEntry->Aid, pEntry->HTPhyMode.field.MCS, 
					HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

			TxSuccess = pEntry->fifoTxSucCnt;
			TxRetransmit = pEntry->fifoTxRtyCnt;
			TxErrorRatio = HwErrRatio;
			TxTotalCnt = HwTxCnt;
			TxCnt = HwTxCnt;
		}
#endif // FIFO_EXT_SUPPORT //
	}

#ifdef MFB_SUPPORT
	if (pEntry->fLastChangeAccordingMfb == TRUE)
	{
		pEntry->fLastChangeAccordingMfb = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("DRS: MCS is according to MFB, and ignore tuning this sec \n"));
		// reset all OneSecTx counters
		RESET_ONE_SEC_TX_CNT(pEntry);		
			return;
	}
#endif	// MFB_SUPPORT //

	// Remember the current rate
	CurrRateIdx = pEntry->CurrTxRateIndex;
#ifdef TXBF_SUPPORT
	CurrPhyETxBf = pEntry->phyETxBf;
	CurrPhyITxBf = pEntry->phyITxBf;
#endif // TXBF_SUPPORT //
	pCurrTxRate = PTX_RATE_SWITCH_ENTRY_3S(pTable, CurrRateIdx);

#ifdef DOT11_N_SUPPORT
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX)  && pEntry->perThrdAdj == 1 )
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif // DOT11_N_SUPPORT //
	{
		TrainUp		= pCurrTxRate->TrainUp;
		TrainDown	= pCurrTxRate->TrainDown;
	}


	// Debug option: Concise RA log
	if ((pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG) || (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG))
		MlmeRALog(pAd, pEntry, RAL_QUICK_DRS, TxErrorRatio, TxTotalCnt);

	// Handle the low traffic case
	if (TxCnt <= 15)
	{
		// Go back to the original rate
		MlmeRestoreLastRate(pEntry);
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("   QuickDRS: TxTotalCnt <= 15, back to original rate \n"));

		MlmeNewTxRate(pAd, pEntry);

	
		// TODO: should we reset all OneSecTx counters?
		//RESET_ONE_SEC_TX_CNT(pEntry);

		return;
	}

	// Compare throughput. LastTxCount is based on a 500 msec or 500-DEF_QUICK_RA_TIME_INTERVAL interval.
	if ((pEntry->LastTimeTxRateChangeAction == RATE_NO_CHANGE)
#ifdef RTMP_RBUS_SUPPORT
		&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif // RTMP_RBUS_SUPPORT //
	)
		ratio = RA_INTERVAL/DEF_QUICK_RA_TIME_INTERVAL;
	else
		ratio = (RA_INTERVAL-DEF_QUICK_RA_TIME_INTERVAL)/DEF_QUICK_RA_TIME_INTERVAL;

	if (pAd->MacTab.Size == 1)
		OneSecTxNoRetryOKRationCount = (TxSuccess * ratio);
		else
		OneSecTxNoRetryOKRationCount = pEntry->OneSecTxNoRetryOkCount * ratio + (pEntry->OneSecTxNoRetryOkCount >> 1);

	/*
		Downgrade TX quality if PER >= Rate-Down threshold
	*/
	//the only situation when pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND but no rate change
		if (TxErrorRatio >= TrainDown)
		MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);

		pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;


	// Perform DRS - consider TxRate Down first, then rate up.
	if (pEntry->LastSecTxRateChangeAction == RATE_UP)
		{
		BOOLEAN useOldRate;

		// TODO: gaa - Finalize the decision criterion
		// 0=>Throughput. Use New Rate if New TP is better than Old TP
		// 1=>PER. Use New Rate if New PER is less than the TrainDown PER threshold
		// 2=>Hybrid. Use rate with best TP if difference > 10%. Otherwise use rate with Best Estimated TP
		// 3=>Hybrid with check that PER<TrainDown Threshold
		if (pAd->CommonCfg.TrainUpRule == 0)
		{
			useOldRate = (pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount;
		}
		else if (pAd->CommonCfg.TrainUpRule==2 && Rssi<=pAd->CommonCfg.TrainUpRuleRSSI)
		{
			useOldRate = MlmeRAHybridRule(pAd, pEntry, pCurrTxRate, OneSecTxNoRetryOKRationCount, TxErrorRatio);
		}
		else if (pAd->CommonCfg.TrainUpRule==3 && Rssi<=pAd->CommonCfg.TrainUpRuleRSSI)
		{
			useOldRate = (TxErrorRatio >= TrainDown) ||
						 MlmeRAHybridRule(pAd, pEntry, pCurrTxRate, OneSecTxNoRetryOKRationCount, TxErrorRatio);
		}
		else
			useOldRate = TxErrorRatio >= TrainDown;
		if (useOldRate)
			{
			// If PER>50% or TP<lastTP/2 then double the TxQuality delay
			if ((TxErrorRatio > 50) || (OneSecTxNoRetryOKRationCount < pEntry->LastTxOkCount/2))
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND*2);
			else
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);

			MlmeRestoreLastRate(pEntry);
			}
			else
			{
			PRTMP_TX_RATE_SWITCH_3S pLastTxRate = PTX_RATE_SWITCH_ENTRY_3S(pTable, pEntry->lastRateIdx);

			// Clear the history if we changed the MCS and PHY Rate
			if ((pCurrTxRate->CurrMCS != pLastTxRate->CurrMCS) &&
				(pCurrTxRate->dataRate != pLastTxRate->dataRate))
				MlmeClearTxQuality(pEntry);

					if (pEntry->mcsGroup == 0)
					MlmeSetMcsGroup(pAd, pEntry);

			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,
						("   QuickDRS: (Up) keep rate-up (L:%ld, C:%ld)\n",
						pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
				}	
			}
	else if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
		{
		if ((TxErrorRatio >= 50) || (TxErrorRatio >= TrainDown)) //there will be train down again
			{
				MlmeSetMcsGroup(pAd, pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) direct train down (TxErrorRatio >= TrainDown)\n"));
			}
		else if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
			{
			MlmeRestoreLastRate(pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) bad tx ok count (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
			}
			else
			{
				MlmeSetMcsGroup(pAd, pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) keep rate-down (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
			}
		}

	// See if we reverted to the old rate
#ifdef TXBF_SUPPORT
	rateChanged = (pEntry->CurrTxRateIndex != CurrRateIdx) ||
				  (pEntry->phyETxBf!=CurrPhyETxBf) || (pEntry->phyITxBf!=CurrPhyITxBf);

	// Remember last good non-BF rate
	if (!pEntry->phyETxBf && !pEntry->phyITxBf)
		pEntry->lastNonBfRate = pEntry->CurrTxRateIndex;
#else
	rateChanged = (pEntry->CurrTxRateIndex != CurrRateIdx);
#endif // TXBF_SUPPORT //


	// Update mcsGroup
	if (pEntry->LastSecTxRateChangeAction == RATE_UP)
	{
		UCHAR UpRateIdx;

		// If RATE_UP failed look for the next group with valid mcs
		if (pEntry->CurrTxRateIndex != CurrRateIdx && pEntry->mcsGroup > 0)
		{
			pEntry->mcsGroup --;				
			pCurrTxRate = PTX_RATE_SWITCH_ENTRY_3S(pTable, pEntry->lastRateIdx);
		}

		switch (pEntry->mcsGroup)
		{
			case 3:
				UpRateIdx = pCurrTxRate->upMcs3;
				break;
			case 2:
				UpRateIdx = pCurrTxRate->upMcs2;
				break;
			case 1:
				UpRateIdx = pCurrTxRate->upMcs1;
				break;
			default:
				UpRateIdx = CurrRateIdx;
				break;
		}

		if (UpRateIdx == pEntry->CurrTxRateIndex)
				pEntry->mcsGroup = 0;
	}
	

	// Handle change back to old rate
	if (rateChanged)
	{
		// Clear Old Rate's TxQuality
		MlmeSetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);

		pEntry->TxRateUpPenalty = 0;	//redundant
		pEntry->PER[pEntry->CurrTxRateIndex] = 0;	//redundant

		// Set new Tx rate
		MlmeNewTxRate(pAd, pEntry);
	}

	// TODO: should we reset all OneSecTx counters?
	//RESET_ONE_SEC_TX_CNT(pEntry);
}
#endif // NEW_RATE_ADAPT_SUPPORT //


/*
    ========================================================================
    Routine Description:
        AP side, Auto TxRate faster train up timer call back function.
        
    Arguments:
        SystemSpecific1         - Not used.
        FunctionContext         - Pointer to our Adapter context.
        SystemSpecific2         - Not used.
        SystemSpecific3         - Not used.
        
    Return Value:
        None
        
    ========================================================================
*/
VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER			pAd = (PRTMP_ADAPTER)FunctionContext;
	ULONG					i;
	PUCHAR					pTable;
	UCHAR					TableSize = 0;
	UCHAR					CurrRateIdx;
	ULONG					AccuTxTotalCnt, TxTotalCnt, TxCnt;
	ULONG					TxErrorRatio = 0;
	MAC_TABLE_ENTRY			*pEntry;
	PRTMP_TX_RATE_SWITCH	pCurrTxRate;
	UCHAR					InitTxRateIdx, TrainUp, TrainDown;
	CHAR					Rssi, ratio;
	ULONG					TxSuccess, TxRetransmit, TxFailCount;
#ifdef TXBF_SUPPORT
	BOOLEAN					CurrPhyETxBf, CurrPhyITxBf;
#endif // TXBF_SUPPORT //

	pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
	
    //
    // walk through MAC table, see if need to change AP's TX rate toward each entry
    //
	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) 
	{
		pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry) && (pEntry->Sst != SST_ASSOC))
			continue;
#endif // APCLI_SUPPORT //

#ifdef WDS_SUPPORT
		if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->MatchWDSTabIdx))
			continue;
#endif // WDS_SUPPORT //



		// Do nothing if this entry didn't change
		if (pEntry->LastSecTxRateChangeAction == RATE_NO_CHANGE
#ifdef RTMP_RBUS_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif // RTMP_RBUS_SUPPORT //
		)
			continue;

		APMlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
		pEntry->pTable = pTable;

#ifdef NEW_RATE_ADAPT_SUPPORT
		if (ADAPT_RATE_TABLE(pTable))
		{
			APQuickResponeForRateUpExecAdapt(pAd, i);
			continue;				
		}
#endif // NEW_RATE_ADAPT_SUPPORT //

    	//Rssi = RTMPMaxRssi(pAd, (CHAR)pEntry->RssiSample.AvgRssi0, (CHAR)pEntry->RssiSample.AvgRssi1, (CHAR)pEntry->RssiSample.AvgRssi2);
		Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);


		if (pAd->MacTab.Size == 1)
		{
            TX_STA_CNT1_STRUC		StaTx1;
			TX_STA_CNT0_STRUC		TxStaCnt0;

			// Update statistic counter
			NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

			TxRetransmit = StaTx1.field.TxRetransmit;
			TxSuccess = StaTx1.field.TxSuccess;
			TxFailCount = TxStaCnt0.field.TxFailCount;
			TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

			AccuTxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
					 pAd->RalinkCounters.OneSecTxRetryOkCount + 
					 pAd->RalinkCounters.OneSecTxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

			if (pAd->Antenna.field.TxPath > 1)
				Rssi = (pEntry->RssiSample.AvgRssi0 + pEntry->RssiSample.AvgRssi1) >> 1;
			else
				Rssi = pEntry->RssiSample.AvgRssi0;

			TxCnt = AccuTxTotalCnt;
		}
		else
		{
			TxRetransmit = pEntry->OneSecTxRetryOkCount;
			TxSuccess = pEntry->OneSecTxNoRetryOkCount;
			TxFailCount = pEntry->OneSecTxFailCount;
			TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;
	
			TxCnt = TxTotalCnt;	

			if (TxTotalCnt)
				TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

#ifdef FIFO_EXT_SUPPORT
			if ((pEntry->Aid >= 1) && (pEntry->Aid <= 8))
		{
				ULONG	HwTxCnt, HwErrRatio;

				NicGetMacFifoTxCnt(pAd, pEntry);
				HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
				if (HwTxCnt)
					HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
				else
					HwErrRatio = 0;
				
				DBGPRINT(RT_DEBUG_TRACE | DBG_FUNC_RA,("%s():Aid:%d, MCS:%d, TxErrRation(Hw:0x%lx-0x%lx, Sw:0x%lx-%lx)\n", 
						__FUNCTION__, pEntry->Aid, pEntry->HTPhyMode.field.MCS, 
						HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

				TxSuccess = pEntry->fifoTxSucCnt;
				TxRetransmit = pEntry->fifoTxRtyCnt;
				TxErrorRatio = HwErrRatio;
				TxTotalCnt = HwTxCnt;
				TxCnt = HwTxCnt;
		}
#endif // FIFO_EXT_SUPPORT //
		}

		CurrRateIdx = pEntry->CurrTxRateIndex;
#ifdef TXBF_SUPPORT
		CurrPhyETxBf = pEntry->phyETxBf;
		CurrPhyITxBf = pEntry->phyITxBf;
#endif // TXBF_SUPPORT //
		pCurrTxRate = PTX_RATE_SWITCH_ENTRY(pTable, CurrRateIdx);

#ifdef DOT11_N_SUPPORT
		if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
		{
			TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
			TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
		}
		else
#endif // DOT11_N_SUPPORT //
		{
			TrainUp		= pCurrTxRate->TrainUp;
			TrainDown	= pCurrTxRate->TrainDown;
		}


        if (TxCnt <= 15)
        {
			MlmeClearAllTxQuality(pEntry);

			// Set current up MCS at the worst quality
			if (pEntry->LastSecTxRateChangeAction == RATE_UP)
			{
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);
			}
			
			// Go back to the original rate
			MlmeRestoreLastRate(pEntry);

			MlmeNewTxRate(pAd, pEntry);


		// TODO: should we reset all OneSecTx counters?
		//RESET_ONE_SEC_TX_CNT(pEntry);

			continue;
        }

		pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

       // Compare throughput
		do
		{
			ULONG		OneSecTxNoRetryOKRationCount;

			// Compare throughput. LastTxCount is based on a 500 msec or 500-DEF_QUICK_RA_TIME_INTERVAL interval.
			if ((pEntry->LastTimeTxRateChangeAction == RATE_NO_CHANGE)
#ifdef RTMP_RBUS_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif // RTMP_RBUS_SUPPORT //
			)
				ratio = RA_INTERVAL/DEF_QUICK_RA_TIME_INTERVAL;
			else
				ratio = (RA_INTERVAL-DEF_QUICK_RA_TIME_INTERVAL)/DEF_QUICK_RA_TIME_INTERVAL;

			// downgrade TX quality if PER >= Rate-Down threshold
			if (TxErrorRatio >= TrainDown)
			{
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);
			}

			if (pAd->MacTab.Size == 1)
			{
				OneSecTxNoRetryOKRationCount = (TxSuccess * ratio);
			}
			else
			{
				OneSecTxNoRetryOKRationCount = pEntry->OneSecTxNoRetryOkCount * ratio + (pEntry->OneSecTxNoRetryOkCount >> 1);
			}

			// perform DRS - consider TxRate Down first, then rate up.
			if (pEntry->LastSecTxRateChangeAction == RATE_UP)
			{
// TODO: gaa - use different criterion for train up in Old RA?
				//if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
				if (TxErrorRatio >= TrainDown)
				{
#ifdef TXBF_SUPPORT
					// If PER>50% or TP<lastTP/2 then double the TxQuality delay
					if ((TxErrorRatio > 50) || (OneSecTxNoRetryOKRationCount < pEntry->LastTxOkCount/2))
						MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND*2);
					else
#endif // TXBF_SUPPORT //
						MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);
					
					MlmeRestoreLastRate(pEntry);
				}
				else
				{
				}
			}
			else if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
			{
				//if ((TxErrorRatio >= 50) || (TxErrorRatio >= TrainDown))
				if ((TxErrorRatio >= 50) && (TxErrorRatio >= TrainDown))
				{
				}
				else if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
				{
					MlmeRestoreLastRate(pEntry);
				}
				else
				{
					DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("QuickDRS: (Down) keep rate-down (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
				}
			}
		}while (FALSE);

#ifdef TXBF_SUPPORT
		// Remember last good non-BF rate
		if (!pEntry->phyETxBf && !pEntry->phyITxBf)
			pEntry->lastNonBfRate = pEntry->CurrTxRateIndex;
#endif // TXBF_SUPPORT //

		// If rate changed then update the history and set the new tx rate
		if ((pEntry->CurrTxRateIndex != CurrRateIdx)
#ifdef TXBF_SUPPORT
			|| (pEntry->phyETxBf!=CurrPhyETxBf) || (pEntry->phyITxBf!=CurrPhyITxBf)
#endif // TXBF_SUPPORT //
		)
		{
		// if rate-up happen, clear all bad history of all TX rates
			if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
		{
			pEntry->TxRateUpPenalty = 0;
				if (pEntry->CurrTxRateIndex != CurrRateIdx)
					MlmeClearTxQuality(pEntry);
		}
		// if rate-down happen, only clear DownRate's bad history
			else if (pEntry->LastSecTxRateChangeAction == RATE_UP)
		{
			pEntry->TxRateUpPenalty = 0;           // no penalty
				MlmeSetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);
			pEntry->PER[pEntry->CurrTxRateIndex] = 0;
		}

			MlmeNewTxRate(pAd, pEntry);
		}

		// TODO: should we reset all OneSecTx counters?
		//RESET_ONE_SEC_TX_CNT(pEntry);
    }
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN APMsgTypeSubst(
    IN PRTMP_ADAPTER pAd,
    IN PFRAME_802_11 pFrame, 
    OUT INT *Machine, 
    OUT INT *MsgType) 
{
    USHORT Seq;
    UCHAR  EAPType;
    BOOLEAN     Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
    PMAC_TABLE_ENTRY pEntry;
#endif // WSC_AP_SUPPORT //

//TODO:
// only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, 
// ignore this frame

    // wpa EAPOL PACKET
    if (pFrame->Hdr.FC.Type == BTYPE_DATA) 
    {    
#ifdef WSC_AP_SUPPORT    
        //WSC EAPOL PACKET        
        pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
        if ((!MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.EntryAddr, ZERO_MAC_ADDR)) &&
            pEntry && 
            IS_ENTRY_CLIENT(pEntry) && 
            (pAd->ApCfg.MBSSID[pEntry->apidx].WscControl.WscConfMode != WSC_DISABLE))
        {
            *Machine = WSC_STATE_MACHINE;
            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
            EAPCode = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
            Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
        }
        if (!Return)
        {
#endif // WSC_AP_SUPPORT //
	        *Machine = WPA_STATE_MACHINE;
        EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
	        Return = WpaMsgTypeSubst(EAPType, (INT *) MsgType);
#ifdef WSC_AP_SUPPORT            
        }
#endif // WSC_AP_SUPPORT //
        return Return;
    }
    
    if (pFrame->Hdr.FC.Type != BTYPE_MGMT)
        return FALSE;
    
    switch (pFrame->Hdr.FC.SubType) 
    {
        case SUBTYPE_ASSOC_REQ:
            *Machine = AP_ASSOC_STATE_MACHINE;
            *MsgType = APMT2_PEER_ASSOC_REQ;
            
            break;
//      case SUBTYPE_ASSOC_RSP:
//          *Machine = AP_ASSOC_STATE_MACHINE;
//          *MsgType = APMT2_PEER_ASSOC_RSP;
//          break;
        case SUBTYPE_REASSOC_REQ:
            *Machine = AP_ASSOC_STATE_MACHINE;
            *MsgType = APMT2_PEER_REASSOC_REQ;
            break;
//      case SUBTYPE_REASSOC_RSP:
//          *Machine = AP_ASSOC_STATE_MACHINE;
//          *MsgType = APMT2_PEER_REASSOC_RSP;
//          break;
        case SUBTYPE_PROBE_REQ:
            *Machine = AP_SYNC_STATE_MACHINE;              
            *MsgType = APMT2_PEER_PROBE_REQ;
            break;
// test for 40Mhz intolerant
		/*
			For Active Scan
		*/
		case SUBTYPE_PROBE_RSP:
          *Machine = AP_SYNC_STATE_MACHINE;
          *MsgType = APMT2_PEER_PROBE_RSP;
          break;
        case SUBTYPE_BEACON:
            *Machine = AP_SYNC_STATE_MACHINE;
            *MsgType = APMT2_PEER_BEACON;
            break;
//      case SUBTYPE_ATIM:
//          *Machine = AP_SYNC_STATE_MACHINE;
//          *MsgType = APMT2_PEER_ATIM;
//          break;
        case SUBTYPE_DISASSOC:
            *Machine = AP_ASSOC_STATE_MACHINE;
            *MsgType = APMT2_PEER_DISASSOC_REQ;
            break;
        case SUBTYPE_AUTH:
            // get the sequence number from payload 24 Mac Header + 2 bytes algorithm
            NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
            
			*Machine = AP_AUTH_STATE_MACHINE;
			if (Seq == 1)
				*MsgType = APMT2_PEER_AUTH_REQ;
			else if (Seq == 3)
				*MsgType = APMT2_PEER_AUTH_CONFIRM;
            else 
            {
                DBGPRINT(RT_DEBUG_TRACE,("wrong AUTH seq=%d Octet=%02x %02x %02x %02x %02x %02x %02x %02x\n", Seq,
                    pFrame->Octet[0], pFrame->Octet[1], pFrame->Octet[2], pFrame->Octet[3], 
                    pFrame->Octet[4], pFrame->Octet[5], pFrame->Octet[6], pFrame->Octet[7]));
                return FALSE;
            }
            break;

        case SUBTYPE_DEAUTH:
            *Machine = AP_AUTH_STATE_MACHINE; /*AP_AUTH_RSP_STATE_MACHINE;*/
            *MsgType = APMT2_PEER_DEAUTH;
            break;

	case SUBTYPE_ACTION:
	case SUBTYPE_ACTION_NO_ACK:
		*Machine = ACTION_STATE_MACHINE;
		//  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support
		if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG) 
		{
			*MsgType = MT2_ACT_INVALID;
		} 
		else
		{
			*MsgType = (pFrame->Octet[0]&0x7F);
		} 
		break;

        default:
            return FALSE;
            break;
    }

    return TRUE;
}


/*
    ========================================================================
    Routine Description:
        Periodic evaluate antenna link status
        
    Arguments:
        pAd         - Adapter pointer
        
    Return Value:
        None
        
    ========================================================================
*/
VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR	BBPR3 = 0;
	ULONG	TxTotalCnt;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif // RALINK_ATE //
#ifdef CARRIER_DETECTION_SUPPORT
	if(pAd->CommonCfg.CarrierDetect.CD_State == CD_SILENCE)
	return;
#endif // CARRIER_DETECTION_SUPPORT //
	
#ifdef TXBF_SUPPORT
	// TODO: we didn't do RxAnt evaluate for 3x3 chips
	if (IS_RT3883(pAd) || IS_RT2883(pAd))
		return;
#endif // TXBF_SUPPORT //

	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= (~0x18);
	if(pAd->Antenna.field.RxPath == 3 
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
		&& pAd->ApCfg.bGreenAPActive == FALSE
#endif // GREENAP_SUPPORT //
#endif // DOT11_N_SUPPORT //
		)
	{
		BBPR3 |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2 
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
		&& pAd->ApCfg.bGreenAPActive == FALSE
#endif // GREENAP_SUPPORT //
#endif // DOT11_N_SUPPORT //
		)
	{
		BBPR3 |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		BBPR3 |= (0x0);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
					pAd->RalinkCounters.OneSecTxRetryOkCount + 
					pAd->RalinkCounters.OneSecTxFailCount;

	if (TxTotalCnt > 50)
	{
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
		pAd->Mlme.bLowThroughput = FALSE;
	}
	else
	{
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
		pAd->Mlme.bLowThroughput = TRUE;
	}
}

/*
    ========================================================================
    Routine Description:
        After evaluation, check antenna link status
        
    Arguments:
        pAd         - Adapter pointer
        
    Return Value:
        None
        
    ========================================================================
*/
VOID APAsicRxAntEvalTimeout(
	PRTMP_ADAPTER	pAd) 
{
	UCHAR			BBPR3 = 0;
	CHAR			larger = -127, rssi0, rssi1, rssi2;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif // RALINK_ATE //

	// if the traffic is low, use average rssi as the criteria
	if (pAd->Mlme.bLowThroughput == TRUE)
	{
		rssi0 = pAd->ApCfg.RssiSample.LastRssi0;
		rssi1 = pAd->ApCfg.RssiSample.LastRssi1;
		rssi2 = pAd->ApCfg.RssiSample.LastRssi2;
	}
	else
	{
		rssi0 = pAd->ApCfg.RssiSample.AvgRssi0;
		rssi1 = pAd->ApCfg.RssiSample.AvgRssi1;
		rssi2 = pAd->ApCfg.RssiSample.AvgRssi2;
	}

	if(pAd->Antenna.field.RxPath == 3)
	{
		larger = max(rssi0, rssi1);
#ifdef DOT11N_SS3_SUPPORT
		if (IS_RT2883(pAd) || IS_RT3883(pAd))
			pAd->Mlme.RealRxPath = 3;
		else
#endif // DOT11N_SS3_SUPPORT //
		{
		if (larger > (rssi2 + 20))
			pAd->Mlme.RealRxPath = 2;
		else
			pAd->Mlme.RealRxPath = 3;
		}
	}
	// Disable the below to fix 1T/2R issue. It's suggested by Rory at 2007/7/11.

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= (~0x18);
	if(pAd->Mlme.RealRxPath == 3 
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
		&& pAd->ApCfg.bGreenAPActive == FALSE
#endif // GREENAP_SUPPORT //
#endif // DOT11_N_SUPPORT //
		)
	{
		BBPR3 |= (0x10);
	}
	else if(pAd->Mlme.RealRxPath == 2
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
		&& pAd->ApCfg.bGreenAPActive == FALSE
#endif // GREENAP_SUPPORT //
#endif // DOT11_N_SUPPORT //
		)
	{
		BBPR3 |= (0x8);
	}
	else if(pAd->Mlme.RealRxPath == 1)
	{
		BBPR3 |= (0x0);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
}

/*
    ========================================================================
    Routine Description:
        After evaluation, check antenna link status
        
    Arguments:
        pAd         - Adapter pointer
        
    Return Value:
        None
        
    ========================================================================
*/
VOID	APAsicAntennaAvg(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	              AntSelect,
	IN	SHORT*	              RssiAvg)  
{
		    SHORT	realavgrssi;
		    LONG         realavgrssi1;
		    ULONG	recvPktNum = pAd->RxAnt.RcvPktNum[AntSelect];

		    realavgrssi1 = pAd->RxAnt.Pair1AvgRssiGroup1[AntSelect];

		    if(realavgrssi1 == 0)
		    {      
		        *RssiAvg = 0;
		        return;
		    }

		    realavgrssi = (SHORT) (realavgrssi1 / recvPktNum);

		    pAd->RxAnt.Pair1AvgRssiGroup1[0] = 0;
		    pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
		    pAd->RxAnt.Pair1AvgRssiGroup2[0] = 0;
		    pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
		    pAd->RxAnt.RcvPktNum[0] = 0;
		    pAd->RxAnt.RcvPktNum[1] = 0;
		    *RssiAvg = realavgrssi - 256;
}



/* End of ap_mlme.c */
