/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	btco.c

	Abstract:

	Handling Bluetooth Coexistence Problem

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Sean Wang	2009-08-12		Create
	John Li		2009-11-30		Modified
*/

#include "rt_config.h"
#include "btco.h"

#ifdef BT_COEXISTENCE_SUPPORT
DECLARE_TIMER_FUNCTION(BtCoexistDetectExec);
BUILD_TIMER_FUNCTION(BtCoexistDetectExec);

#ifdef DOT11N_DRAFT3
BOOLEAN BtCheckWifiThroughputOverLimit(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR WifiThroughputLimit)
{
	BOOLEAN  bIssue4020 = FALSE;
	ULONG tmpReceivedByteCount = 0;
	ULONG tmpTransmittedByteCount = 0;
	static ULONG TxByteCount = 0;
	static ULONG RxByteCount = 0;
	static ULONG TxRxThroughputPerSeconds = 0; //Unit: bytes
	LONG diffTX = 0;
	LONG diffRX = 0;

	bIssue4020 = FALSE;
	
	if (pAd == NULL)
	{
		return FALSE;
	}
	
	if (IS_ENABLE_BT_40TO20_BY_TIMER(pAd))
	{	
		tmpReceivedByteCount = pAd->RalinkCounters.ReceivedByteCount; 
		tmpTransmittedByteCount = pAd->RalinkCounters.TransmittedByteCount;		
					
		if ((TxByteCount != 0) || (RxByteCount != 0 ))
		{				
			diffTX = (LONG)(((tmpTransmittedByteCount - TxByteCount)*5) >> 3);
			diffRX = (LONG)(((tmpReceivedByteCount - RxByteCount)* 5) >> 3);		
						
			if ((diffTX > 0) && (diffRX > 0 ))
			{
				TxRxThroughputPerSeconds = diffTX + diffRX ;//Unit: bytes 
			}
			else if ((diffTX < 0) && (diffRX > 0))
			{
				TxRxThroughputPerSeconds = diffRX;
			}
			else if ((diffTX > 0) && (diffRX < 0))
			{
				TxRxThroughputPerSeconds = diffTX;
			}
			else 
			{
				TxRxThroughputPerSeconds = 0;
			}

			DBGPRINT(RT_DEBUG_INFO,("TxRxThroughputPerSeconds = %ld Bps, %ld KBps, %ldKbps, %ldMbps", 
				TxRxThroughputPerSeconds,
				(TxRxThroughputPerSeconds >> 10),
				(TxRxThroughputPerSeconds >> 7),
				(TxRxThroughputPerSeconds >> 17)));
		}
	
		TxByteCount = tmpTransmittedByteCount;
		RxByteCount = tmpReceivedByteCount;

		DBGPRINT(RT_DEBUG_INFO,("btWifiThr = %d, TxByteCount = %ld, RxByteCount = %ld",
			WifiThroughputLimit,
			TxByteCount,
			RxByteCount));

		if ((TxRxThroughputPerSeconds >> 17) > WifiThroughputLimit)
		{
			bIssue4020 = TRUE;
		}
		else
		{
			bIssue4020 = FALSE;
		}
	}
	
	return bIssue4020;
}
#endif // DOT11N_DRAFT3 //

BLUETOOTH_BUSY_DEGREE BtCheckBusy(
	IN PLONG BtHistory, 
	IN UCHAR BtHistorySize)
{
	if (BtHistory == NULL)
	{
		return BUSY_0;
	}

	DBGPRINT(RT_DEBUG_INFO,(" ---> Check Busy %ld %ld %ld %ld %ld",
		*BtHistory,
		*(BtHistory+1),
		*(BtHistory+2),
		*(BtHistory+3),
		*(BtHistory+4)));

#ifdef RTMP_PCI_SUPPORT
	if ((*BtHistory > 40) || 
		(*(BtHistory+1) > 40) || 
		(*(BtHistory+2) > 40) ||
		(*(BtHistory+3) > 40) ||
		(*(BtHistory+4) > 40))
	{
		DBGPRINT(RT_DEBUG_INFO,("0,0,40,0,0 ==> Inquiry + other Profiles ... BT lagging .. BUSY_5\n"));
		return BUSY_5; 
	}

	if ((*BtHistory > 33) || 
		(*(BtHistory+1) > 33) || 
		(*(BtHistory+2) > 33) ||
		(*(BtHistory+3) > 33) ||
		(*(BtHistory+4) > 33))
	{
		DBGPRINT(RT_DEBUG_INFO,("0,0,33,0,0 ==> Inquiry  .. BUSY_5\n"));
		return BUSY_5; 
	}
	
	if (((*BtHistory >= 20) && 
		(*(BtHistory+1) >= 20) && 
		(*(BtHistory+2) >= 20) &&
		(*(BtHistory+3) >= 20) &&
		(*(BtHistory+4) >= 20)))
	{
		DBGPRINT(RT_DEBUG_INFO,("20,20,20,20,20 ==> Multiple Profile... BUSY_4\n"));
		return BUSY_4;	
	}

	if ((*BtHistory >= 8) && 
		(*(BtHistory+1) >= 8) && 
		(*(BtHistory+2) >= 8) &&
		(*(BtHistory+3) >= 8) &&
		(*(BtHistory+4) >= 8))
	{
		DBGPRINT(RT_DEBUG_INFO,("8,8,8,8,8 ==> BT work ... Jabra BT3030 A2DP Playing Music  ... BUSY_3\n"));
		return BUSY_3;
	}

	if ((*BtHistory >= 10) || 
		(*(BtHistory+1) >= 10) || 
		(*(BtHistory+2) >= 10) || 
		(*(BtHistory+3) >= 10) || 
		(*(BtHistory+4) >= 10))
	{
		DBGPRINT(RT_DEBUG_INFO,("0,0,10,0,0 ==> BT works ... CSR mouse suddenly moves ... BUSY_2\n"));
		return BUSY_2;
	}

	if (((*BtHistory >= 4) || 
		(*(BtHistory+1) >= 4) || 
		(*(BtHistory+2) >= 4) || 
		(*(BtHistory+3) >= 4) || 
		(*(BtHistory+4) >= 4))&&
		(*BtHistory >= 2) && 
		(*(BtHistory+1) >= 2) && 
		(*(BtHistory+2) >= 2) && 
		(*(BtHistory+3) >= 2) && 
		(*(BtHistory+4) >= 2))	
	{
		DBGPRINT(RT_DEBUG_INFO,("1,1,3,1,1 ==> BT works ... CSR keyboard typing ... BUSY_2\n"));
		return BUSY_2;
	}

	if (((*BtHistory >= 1) || 
		(*(BtHistory+1) >= 1) || 
		(*(BtHistory+2) >= 1) || 
		(*(BtHistory+3) >= 1) || 
		(*(BtHistory+4) >= 1) ) &&
		((*BtHistory >= 0) && 
		(*(BtHistory+1) >= 0) && 
		(*(BtHistory+2) >= 0) &&
		(*(BtHistory+3) >= 0) &&
		(*(BtHistory+4) >= 0)))
	{
		DBGPRINT(RT_DEBUG_INFO,("0,0,1,0,0 ==> BT idle .. BUSY_1\n"));
		return BUSY_1;
	}	
#endif // RTMP_PCI_SUPPORT //

	DBGPRINT(RT_DEBUG_INFO,("ALL BT Profile Disconnected .. BUSY_0"));
	return BUSY_0;
}

/*
	Wifi adjust according to BT Acitivitity  
	Case 1: LNA Middle Gain 	 @HT40/HT20 and BUSY_1 equal & above
	Case 2: TxPower 				 @HT40/HT20 and BUSY_1 equal & above
	Case 3: BA Window Size		 @HT40/HT20 and BUSY_2 equal & above
	Case 4: BA Density			@HT40/HT20 and BUSY_2 equal & above
	Case 5: MCS Rate				@HT40 and BUSY_2 equal & above
	Case 6: REC BA Request			@HT40/HT20 and BUSY_2 equal & above
	Case 7: 40/20 BSS Coexistence	@HT40 and BUSY_4
*/
VOID BtCoexistAdjust(
	IN PRTMP_ADAPTER	pAd, 
	IN BOOLEAN			bIssue4020, 
	IN ULONG			NoBusyTimeCount)
{
	CHAR	Rssi;

	Rssi = RTMPMaxRssi(pAd, 
					   pAd->StaCfg.RssiSample.AvgRssi0, 
					   pAd->StaCfg.RssiSample.AvgRssi1, 
					   pAd->StaCfg.RssiSample.AvgRssi2);
	
	DBGPRINT(RT_DEBUG_INFO,("RSSI = %d\n", Rssi));

	if (IS_ENABLE_BT_LNA_MID_GAIN_DOWN_BY_TIMER(pAd))
	{
		UCHAR BbpR65 = 0;
		if (pAd->BTBusyDegree >= BUSY_1)
		{
			DBGPRINT(RT_DEBUG_INFO, ("Lower LNA Middle Gain at HT20 or HT40\n"));
			pAd->bBTPermitLnaGainDown = TRUE;

			DBGPRINT(RT_DEBUG_INFO,("RSSI = %d\n", Rssi));

			if (Rssi <= -35)
			{
			/* if RSSI is smaller than -80, then set R65 to High Gain to fix long distance issue */
				if (Rssi <= -80)
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);

					if (BbpR65 == 0x29)
					{
						DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x2C from 0x29 (Highest LNA)\n"));
						BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x2C); /* Highest LNA Gain */
					}
				}
				else
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);

					if (BbpR65 == 0x2C)
					{
						DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x29 from 0x2C (Middle LNA)\n"));
						BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x29); /* Middle LNA Gain */
					}
				}
			}
			else
			{
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);

				if (BbpR65 == 0x29)
				{
					DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x2C from 0x29 (Highest LNA)\n"));
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x2C); /* Highest LNA Gain */
				}
			}
		}
		else
		{
			if (NoBusyTimeCount > BT_IDLE_STATE_THRESHOLD)
			{	
			DBGPRINT(RT_DEBUG_INFO, ("Lower LNA Middle Gain at HT20 or HT40 (Highest LNA)\n"));

			pAd->bBTPermitLnaGainDown = FALSE;
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpR65);
				if (BbpR65 == 0x29)
				{
					DBGPRINT(RT_DEBUG_INFO,("Set R65 to 0x2C from 0x29 (Highest LNA)\n"));
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x2C); /* Highest LNA Gain */
				}
			}
			else
			{
				DBGPRINT(RT_DEBUG_INFO, ("Lower LNA Middle Gain at HT20 or HT40\n"));
			}
		}
	}

	if (IS_ENABLE_BT_TX_POWER_DOWN_BY_TIMER(pAd))
	{
		if (pAd->BTBusyDegree >= BUSY_1)
		{
			DBGPRINT(RT_DEBUG_INFO, ("Lower Tx Power at HT20 or HT40\n"));
				pAd->bBTPermitTxPowerDown= TRUE;
		}
		else
		{
			if (NoBusyTimeCount > BT_IDLE_STATE_THRESHOLD)
			{
				DBGPRINT(RT_DEBUG_INFO, ("Higher Tx Power at HT20 or HT40\n"));
					pAd->bBTPermitTxPowerDown= FALSE;
			}
			else
			{
				DBGPRINT(RT_DEBUG_INFO, ("Lower Tx Power at HT20 or HT40\n"));
			}
		}
	}

#ifdef DOT11_N_SUPPORT
	if (IS_ENABLE_BT_TXWI_AMPDU_SIZE_BY_TIMER(pAd))
	{
	/* Fixed long distance issue */
		if ((pAd->BTBusyDegree >= BUSY_2) && 
		((pAd->CommonCfg.BBPCurrentBW == BW_40) || 
		(pAd->CommonCfg.BBPCurrentBW == BW_20)) && 
		(Rssi <= -80))
		{
			pAd->bBTPermitTxBaSizeDown = TRUE;
		}
		else
		{
			pAd->bBTPermitTxBaSizeDown = FALSE;
		}
	}

	if (IS_ENABLE_BT_TXWI_AMPDU_DENSITY_BY_TIMER(pAd))
	{
	/* Fixed long distance issue */
		if ((pAd->BTBusyDegree >= BUSY_2) && 
		((pAd->CommonCfg.BBPCurrentBW == BW_40) || 
		(pAd->CommonCfg.BBPCurrentBW == BW_20)) && 
		(Rssi <= -80))
		{
			pAd->bBTPermitTxBaDensityDown = TRUE;
		}
		else
		{
			pAd->bBTPermitTxBaDensityDown = FALSE;
		}
	}
		
	if (IS_ENABLE_BT_RATE_ADAPTIVE_BY_TIMER(pAd))
	{
		if ((pAd->BTBusyDegree >= BUSY_2) && 
				(pAd->CommonCfg.BBPCurrentBW == BW_40))
		{
			pAd->bBTPermitMcsDown = TRUE;
		}
		else
		{
			pAd->bBTPermitMcsDown = FALSE;
		}
	}

	if (IS_ENABLE_BT_REJECT_ORE_BA_BY_TIMER(pAd))
	{	
	/* Fixed long distance issue */
		if ((pAd->BTBusyDegree >= BUSY_2) && 
		((pAd->CommonCfg.BBPCurrentBW == BW_40) || 
		(pAd->CommonCfg.BBPCurrentBW == BW_20)) && 
		(Rssi <= -80))
		{
			BASessionTearDownALL(pAd, BSSID_WCID);
			pAd->bBTPermitRecBaDown = TRUE;
		}
		else
		{
			pAd->bBTPermitRecBaDown = FALSE;
		}
	}
	
#ifdef DOT11N_DRAFT3
	if (IS_ENABLE_BT_40TO20_BY_TIMER(pAd))
	{
	if (((pAd->BTBusyDegree >= WIFI_2040_SWITCH_THRESHOLD) && 
			(pAd->BTBusyDegree != BUSY_5)) && 
			(pAd->CommonCfg.BBPCurrentBW == BW_40) && 
			(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) &&
			(bIssue4020 == TRUE))
		{
			BSS_2040_COEXIST_IE OldValue;

		DBGPRINT(RT_DEBUG_INFO, ("HT40 --> HT20\n"));
		DBGPRINT(RT_DEBUG_INFO,("ACT - Update2040CoexistFrameAndNotify. BSSCoexist2040 = %x. EventANo = %d. \n", pAd->CommonCfg.BSSCoexist2040.word, pAd->CommonCfg.TriggerEventTab.EventANo));
			OldValue.word = pAd->CommonCfg.BSSCoexist2040.word;
			pAd->CommonCfg.BSSCoexist2040.word = 0;

			//if (pAd->CommonCfg.TriggerEventTab.EventBCountDown > 0)
			pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 1;

		/*
			Need to check !!!!
			How STA will set Intolerant40 if implementation dependent. Now we don't set this bit first!!!!!
			So Only check BSS20WidthReq change.
		*/
			//if (OldValue.field.BSS20WidthReq != pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq)
			{
				Send2040CoexistAction(pAd, BSSID_WCID, TRUE);
			}
		}
	}
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //
}

VOID BtCoexistTxPowerDown(
	IN PRTMP_ADAPTER	pAd, 
	IN CHAR				Rssi,
	INOUT CHAR			*pDeltaPowerByBbpR1, 
	INOUT CHAR			*pDeltaPwr)
{
	/* If bluetooth is busy, to lower Tx Power to balance with BT power */
	if ((pAd->bHWCoexistenceInit == TRUE) && 
		IS_ENABLE_BT_TX_POWER_DOWN_BY_TIMER(pAd) &&
		(pAd->bBTPermitTxPowerDown== TRUE) && 
		(pAd->CommonCfg.TxPowerPercentage == 0xffffffff))
	{
		CHAR DeltaPowerByBbpR1 = *pDeltaPowerByBbpR1;
		CHAR DeltaPwr = *pDeltaPwr;

		DBGPRINT(RT_DEBUG_INFO, (">> ++ DeltaPwr =%d, DeltaPowerByBbpR1= %d, RSSI = %d\n",
			DeltaPwr, DeltaPowerByBbpR1, Rssi));

		if (Rssi <= -80)
		{	
			DBGPRINT(RT_DEBUG_INFO, (">> -1 75% power dbm\n"));
			DeltaPowerByBbpR1 = 0;
			DeltaPwr -= 1;
		}
		else if ((Rssi <= -50) && (Rssi >= -60))
		{
			DeltaPowerByBbpR1 = -6;
			DeltaPwr -= 1;

			DBGPRINT(RT_DEBUG_INFO, (">> -7 18% power dbm\n"));
		}
		else if ((Rssi <= -60) && (Rssi >= -70))
		{
			DeltaPowerByBbpR1 = -6;
			DBGPRINT(RT_DEBUG_INFO, (">> -6 25% power dbm\n"));
		}
		else if ((Rssi <= -70) && (Rssi >= -80)) 
		{
			DBGPRINT(RT_DEBUG_INFO, (">> -3 50% dbm\n"));
			DeltaPowerByBbpR1 = 0;
			DeltaPwr -= 3;
		}
		else // drop into lowest power
		{
			DeltaPowerByBbpR1 = 0;
			DeltaPowerByBbpR1 -= 12;
		}
		
		*pDeltaPowerByBbpR1 = DeltaPowerByBbpR1;
		*pDeltaPwr = DeltaPwr;
			
		DBGPRINT(RT_DEBUG_INFO, (">> -- DeltaPwr =%d, DeltaPowerByBbpR1= %d, RSSI = %d\n", 
			DeltaPwr, DeltaPowerByBbpR1, Rssi));
	}
}

VOID BtCoexistMcsDown(
	IN PRTMP_ADAPTER	pAd, 
	IN CHAR				CurrRateIdx, 
	IN PRTMP_TX_RATE_SWITCH	pCurrTxRate, 
	INOUT CHAR			*pUpRateIdx, 
	INOUT CHAR			*pDownRateIdx)
{
	/* If bluetooth is busy, now we decrease rate to MCS Thresdhold */
	if ((pAd->bHWCoexistenceInit == TRUE) && 
		IS_ENABLE_BT_RATE_ADAPTIVE_BY_TIMER(pAd) && 
		(pAd->bBTPermitMcsDown == TRUE))
	{
		UCHAR	btMCSThreshold = 0x00;
		UCHAR	UpRateIdx = *pUpRateIdx;
		UCHAR	DownRateIdx = *pDownRateIdx;

		btMCSThreshold = (UCHAR)(GET_BT_PARAMETER_OF_MCS_THRESHOLD(pAd));
		//0,1,2,3 => MCS= 3, 4, 5, 6
		if (btMCSThreshold <= 0x3)
		{
			btMCSThreshold = btMCSThreshold + 0x3;
		}
		else
		{
			btMCSThreshold = 0x03;
		}
		
		if (CurrRateIdx > 0)
		{
			// Rate is equeal to btMCSThreshold
			if ((pCurrTxRate->CurrMCS == btMCSThreshold)) 
			{
				UpRateIdx = CurrRateIdx;
			}
	
			// Rate be decreased to btMCSThreshold
			else if (pCurrTxRate->CurrMCS > btMCSThreshold)
			{
				UpRateIdx = CurrRateIdx - 1;
			}
			//Rate is under btMCSThreshold
			//else
			//{
			//	UpRateIdx = CurrRateIdx + 1;
			//}
			//DownRateIdx = CurrRateIdx - 1;	
	
			DBGPRINT(RT_DEBUG_INFO,("CurrRateIdx=%d, UpRateIdx=%d, DownRateIdx=%d\n",
				CurrRateIdx, UpRateIdx, DownRateIdx));
		}

		*pUpRateIdx = UpRateIdx;
		*pDownRateIdx = DownRateIdx;
	}
}

VOID BtCoexistMcsDown2(
	IN PRTMP_ADAPTER	pAd, 
	IN UCHAR			MCS3, 
	IN UCHAR			MCS4, 
	IN UCHAR			MCS5, 
	IN UCHAR			MCS6, 
	INOUT UCHAR			*pTxRateIdx)
{
	if ((pAd->bHWCoexistenceInit == TRUE) && 
		IS_ENABLE_BT_RATE_ADAPTIVE_BY_TIMER(pAd) && 
		(pAd->bBTPermitMcsDown == TRUE))
	{
		UCHAR	btMCSThreshold = 0x00;
		UCHAR	TxRateIdx = *pTxRateIdx;

		btMCSThreshold = (UCHAR)(GET_BT_PARAMETER_OF_MCS_THRESHOLD(pAd));
		//0,1,2,3 => MCS= 3, 4, 5, 6
		if (btMCSThreshold <= 0x3)
		{
			btMCSThreshold = btMCSThreshold + 0x3;
		}
		else
		{
			btMCSThreshold = 0x03;
		}
	
		if (btMCSThreshold == 0x03)
		{
			btMCSThreshold = MCS3;
		}
		else if (btMCSThreshold == 0x04)
		{
			btMCSThreshold = MCS4;
		}
		else if (btMCSThreshold == 0x05)
		{
			btMCSThreshold = MCS5;
		}
		else if (btMCSThreshold == 0x06)
		{
			btMCSThreshold = MCS6;
		}
		else
		{
			btMCSThreshold = MCS3;
		}
	
		if (TxRateIdx > btMCSThreshold)
		{
			TxRateIdx = btMCSThreshold; 
		}

		*pTxRateIdx = TxRateIdx;
	}
}

VOID BtCoexistTxBaSizeDown(
	IN PRTMP_ADAPTER	pAd, 
	INOUT PTXWI_STRUC 	pTxWI)
{
	if ((pAd->bHWCoexistenceInit == TRUE) && 
		INFRA_ON(pAd) && 
		(pAd->OpMode == OPMODE_STA) && 
		(pTxWI->BAWinSize != 0) && 
		IS_ENABLE_BT_TXWI_AMPDU_SIZE_BY_TIMER(pAd) && 
		(pAd->bBTPermitTxBaSizeDown == TRUE))
	{
		UCHAR				btAMPDUSize = 0;

		/* When Bluetooh is busy, we set the BA Size to smaller */
		btAMPDUSize = (UCHAR)(GET_BT_PARAMETER_OF_AMPDU_SIZE(pAd));
		if (btAMPDUSize <= 3)
		{
			//0,1,2,3 => ba size=1, 3, 5, 7
			if (btAMPDUSize == 0)
			{
				pTxWI->BAWinSize = 1;
			}
			else
			{
				pTxWI->BAWinSize = 1 + (btAMPDUSize << 1);
			}
		}
		else 
		{
			pTxWI->BAWinSize = 1;
		}
		
		DBGPRINT(RT_DEBUG_INFO, (">>>>> dynamic BAWinSize = %d, profile AMPDU size=%ld\n", 
		pTxWI->BAWinSize, GET_BT_PARAMETER_OF_AMPDU_SIZE(pAd)));	
	}
}

VOID BtCoexistTxBaDensityDown(
	IN PRTMP_ADAPTER	pAd, 
	INOUT PTXWI_STRUC 	pTxWI)
{
	if ((pAd->bHWCoexistenceInit == TRUE) && 
		INFRA_ON(pAd) && 
		(pAd->OpMode == OPMODE_STA) && 
		IS_ENABLE_BT_TXWI_AMPDU_DENSITY_BY_TIMER(pAd) && 
		(pAd->bBTPermitTxBaDensityDown == TRUE))
	{
		UCHAR				btAMPDUDensity = 0;

		/* When Bluetooh is busy, we set the BA density to larger */
		btAMPDUDensity = (UCHAR)(GET_BT_PARAMETER_OF_AMPDU_DENSITY(pAd));
		if (btAMPDUDensity <= 3)
		{
			//0,1,2,3 => BA density=0x01, 0x03, 0x05, 0x07 => 0.25u, 1u, 4u, 16u
			if (btAMPDUDensity == 0)
			{
				pTxWI->MpduDensity = 0x01;
			}
			pTxWI->MpduDensity = 1 + (btAMPDUDensity << 1);
		}
		else 
		{
			pTxWI->MpduDensity = 0x07;
		}
		
		DBGPRINT(RT_DEBUG_INFO, (">>>>> dynamic BA density= %d, profile AMPDU density=%ld\n", 
			pTxWI->MpduDensity, GET_BT_PARAMETER_OF_AMPDU_DENSITY(pAd)));
	}
}

VOID BtCoexistReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING pBuffer)
{
	if (RTMPGetKeyParameter("BTCoexistenceOn", tmpbuf, 10, pBuffer, TRUE))
	{
		if (simple_strtol(tmpbuf, 0, 10) == 0)
			pAd->bBTCoexistenceOn = FALSE;
		else
			pAd->bBTCoexistenceOn = TRUE;
	
		DBGPRINT(RT_DEBUG_TRACE, ("BTCoexistenceOn=%d\n", pAd->bBTCoexistenceOn));
	}
	if (RTMPGetKeyParameter("BTConfiguration", tmpbuf, 10, pBuffer, TRUE))
	{
		ULONG BTConfig = simple_strtol(tmpbuf, 0, 10);
		pAd->ulBTConfiguration = BTConfig;
	
		DBGPRINT(RT_DEBUG_TRACE, ("BTConfiguration=0x%04lx\n", pAd->ulBTConfiguration));
	}
}

VOID BtCoexistInit(
	IN PRTMP_ADAPTER pAd)
{
#ifdef RTMP_PCI_SUPPORT
	if (((pAd->NicConfig3.field.BTCrystalShared == BT_CYRSTALL_SHARED) || 
		IS_ENABLE_BT_SINGLE_CRYSTAL_SHARING_BY_FORCE(pAd)) &&
		(pAd->StaCfg.PSControl.field.rt30xxPowerMode == 0x3))
	{	
		UCHAR  LinkCtrlSetting = 0;
		USHORT PCIePowerSetting = 0;
		
		PCIePowerSetting = 2;
		DBGPRINT(RT_DEBUG_TRACE, ("before PCIePower Save Level = %d due to single crystall\n", pAd->StaCfg.PSControl.field.rt30xxPowerMode));
		DBGPRINT(RT_DEBUG_TRACE, ("after PCIePower Save Level = 2 due to single crystall\n"));
		
		pAd->PCIePowerSaveLevel = (USHORT)PCIePowerSetting;
		if ((pAd->Rt3xxRalinkLinkCtrl & 0x2) && (pAd->Rt3xxHostLinkCtrl & 0x2))
		{
			LinkCtrlSetting = 1;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("====> rt30xxF LinkCtrlSetting = 0x%x.\n", LinkCtrlSetting));
		AsicSendCommandToMcu(pAd, 0x83, 0xff, (UCHAR)PCIePowerSetting, LinkCtrlSetting);
	}
#endif // RTMP_PCI_SUPPORT //
	if (IS_ENABLE_BT_TIMER(pAd))
	{
		/* Read Bluetooth priority or active counter every BT_DETECT_TIMEOUT ms */
		RTMPInitTimer(pAd, &pAd->Mlme.BtCoexistDetectTimer, GET_TIMER_FUNCTION(BtCoexistDetectExec), pAd, TRUE);
		RTMPSetTimer(&pAd->Mlme.BtCoexistDetectTimer, BT_DETECT_TIMEOUT);
		DBGPRINT(RT_DEBUG_TRACE,("Set BT Coexistence detect timer\n"));
	}

	DBGPRINT(RT_DEBUG_TRACE,("nicConfig2 =0x%04x\n", pAd->NicConfig2.word));
	DBGPRINT(RT_DEBUG_TRACE,("nicConfig3 =0x%04x\n", pAd->NicConfig3.word));
	DBGPRINT(RT_DEBUG_TRACE,("BT configuration = %08lx\n", pAd->ulBTConfiguration));
}

VOID BtCoexistUserCfgInit(
	IN PRTMP_ADAPTER pAd)
{
	/* To enable BT Coexistence Function */ 
	pAd->bBTCoexistenceOn = FALSE;
#ifdef DOT11_N_SUPPORT
	pAd->bBTPermitRecBaDown = FALSE;
	pAd->bBTPermitMcsDown = FALSE;
	pAd->bBTPermitTxBaSizeDown = FALSE;
	pAd->bBTPermitTxBaDensityDown = FALSE;
#endif // DOT11_N_SUPPORT //
	pAd->bBTPermitTxPowerDown = FALSE;
	pAd->bBTPermitLnaGainDown = FALSE;
#ifdef RTMP_PCI_SUPPORT
	pAd->ulBTConfiguration = 0x08DD;
#endif // RTMP_PCI_SUPPORT //
	pAd->ulBTActiveCountPastPeriod = 0;
	pAd->BTBusyDegree = BUSY_0;
}


#ifdef RTMP_PCI_SUPPORT
VOID BtCoexistDetectExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	BOOLEAN 				bIssue4020 = FALSE;
	BOOLEAN 				bPowerSaving = FALSE;
	ULONG					data = 0;
	UCHAR					j = 0;
#ifdef DOT11N_DRAFT3
	UCHAR					btWifiThr = 0;
#endif // DOT11N_DRAFT3 //
	BLUETOOTH_BUSY_DEGREE	BusyDegree = BUSY_0;
	static UCHAR			ulPowerSavingTimeCount = 0; // per 0.01 second, count it.
	static ULONG			TimeCount = 0; // per 0.01 second, count it.
	static ULONG			NoBusyTimeCount = 0; // per second, count it.
	static ULONG			VeryBusyTimeCount = 0;		// per second, count it.
	static ULONG			BtHistory[BT_HISTORY_RECORD_NUM] = {0};
	PRTMP_ADAPTER			pAd = NULL;

	pAd = (RTMP_ADAPTER *)FunctionContext;
	
	if (pAd == NULL)
	{
		return;
	}
	
	if ((pAd->bHWCoexistenceInit == TRUE) && 
		(!IDLE_ON(pAd))&& 
		(pAd->OpMode == OPMODE_STA))
	{
		TimeCount++;

		if ((pAd->bPCIclkOff == TRUE) ||
			RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND) || 
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF) ||
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
			if (pAd->bPCIclkOff == TRUE)
			{
				DBGPRINT(RT_DEBUG_INFO,("pAd->bPCIclkOff == TRUE\n"));
			}
			if (RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND))
			{
				DBGPRINT(RT_DEBUG_INFO,("fRTMP_PS_SET_PCI_CLK_OFF_COMMAND == TRUE\n"));
			}
			if (RTMP_TEST_PSFLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
			{
				DBGPRINT(RT_DEBUG_INFO,("fRTMP_ADAPTER_IDLE_RADIO_OFF == TRUE\n"));
			}
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
			{
				DBGPRINT(RT_DEBUG_INFO,("fOP_STATUS_DOZE == TRUE\n"));
			}
			bPowerSaving = TRUE;
			
			ulPowerSavingTimeCount ++;
		}

		if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))	||
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))	||
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))			||
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			DBGPRINT(RT_DEBUG_INFO,("error RESET, HALT, RADIO_OFF, NIC_NOT_EXIST\n"));
			if( TimeCount >= BT_CHECK_TIME_INTERVAL) 
			{
				pAd->ulBTActiveCountPastPeriod=0;
				TimeCount = 0;
				NoBusyTimeCount = 0;

#ifdef DOT11_N_SUPPORT
				pAd->bBTPermitRecBaDown = FALSE;
				pAd->bBTPermitMcsDown = FALSE;
				pAd->bBTPermitTxBaSizeDown = FALSE;
				pAd->bBTPermitTxBaDensityDown = FALSE;
#endif // DOT11_N_SUPPORT //
				pAd->bBTPermitTxPowerDown = FALSE;
				pAd->bBTPermitLnaGainDown = FALSE;
			}
			return;
		}
		
		if (bPowerSaving == FALSE)
		{
			RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
	
			if (data & 0x0010)
			{
				pAd->ulBTActiveCountPastPeriod++;
			}
		}

		if (TimeCount >= BT_CHECK_TIME_INTERVAL) // check if BT is busy per 1 second, timer interval is 10ms
		{

			DBGPRINT(RT_DEBUG_INFO,("<--- WATCH TIME\n"));	
			DBGPRINT(RT_DEBUG_INFO,("-->BW=%d, bt active per sec=%ld, No Busy Time Count =%ld, Very Busy Time Count = %ld, PowerSavingTimeCount =%d\n", 
					pAd->CommonCfg.BBPCurrentBW,
					pAd->ulBTActiveCountPastPeriod,
					NoBusyTimeCount, 
					VeryBusyTimeCount,
					ulPowerSavingTimeCount
					));

			for (j = BT_HISTORY_RECORD_NUM-1; j >= 1; j--)
			{
				BtHistory[j]=BtHistory[j-1];
			}

			if (ulPowerSavingTimeCount == 0)
			{
				BtHistory[0] = pAd->ulBTActiveCountPastPeriod;
			}
			else if (ulPowerSavingTimeCount < BT_CHECK_TIME_INTERVAL)
			{
				BtHistory[0] = (pAd->ulBTActiveCountPastPeriod * BT_CHECK_TIME_INTERVAL )/(BT_CHECK_TIME_INTERVAL - ulPowerSavingTimeCount);
			}
			else 
			{
				BtHistory[0] = 0;
			}
	
			BusyDegree = BtCheckBusy(&BtHistory[0],BT_HISTORY_RECORD_NUM);
			pAd->BTBusyDegree = BusyDegree;
			
			if (pAd->BTBusyDegree == BUSY_0)
			{	
				NoBusyTimeCount++;
			}
			else
			{
				NoBusyTimeCount = 0;
			}
		
			if (pAd->BTBusyDegree >= BUSY_4)
			{	
				VeryBusyTimeCount++;
			}
			else
			{
				VeryBusyTimeCount = 0;
			}
			
#ifdef DOT11N_DRAFT3
			btWifiThr = (UCHAR)(GET_BT_PARAMETER_OF_TXRX_THR_THRESHOLD(pAd));

			//0,1,2,3 => 0, 6, 12, 18
			if ((btWifiThr <= 3) && IS_ENABLE_BT_40TO20_BY_TIMER(pAd))
			{
				bIssue4020 = (BtCheckWifiThroughputOverLimit(pAd,(btWifiThr*6)) == TRUE) & (VeryBusyTimeCount > 15);
			}
			else 
#endif // DOT11N_DRAFT3 //
			bIssue4020 = FALSE;

			DBGPRINT(RT_DEBUG_INFO, ("-->VeryBusyTimeCount = %ld, bIssue4020 = %d\n", VeryBusyTimeCount, bIssue4020));
			
			pAd->ulBTActiveCountPastPeriod = 0;
			TimeCount = 0;
			ulPowerSavingTimeCount = 0;
		}
		
		if (TimeCount == 0)
		{
			BtCoexistAdjust(pAd, bIssue4020, NoBusyTimeCount);
		}	
	}
	else
	{
		pAd->ulBTActiveCountPastPeriod = 0;
		TimeCount = 0;
		NoBusyTimeCount = 0;
		VeryBusyTimeCount = 0;

#ifdef DOT11_N_SUPPORT
		pAd->bBTPermitRecBaDown = FALSE;
		pAd->bBTPermitMcsDown = FALSE;
		pAd->bBTPermitTxBaSizeDown = FALSE;
		pAd->bBTPermitTxBaDensityDown = FALSE;
#endif // DOT11_N_SUPPORT //
		pAd->bBTPermitTxPowerDown = FALSE;
		pAd->bBTPermitLnaGainDown = FALSE;
	}
}
#endif // RTMP_PCI_SUPPORT //

#endif // BT_COEXISTENCE_SUPPORT //
