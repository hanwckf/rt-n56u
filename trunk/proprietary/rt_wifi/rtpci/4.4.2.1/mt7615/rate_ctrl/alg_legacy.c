/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All related Dynamic Rate Switch (AP/STA) function body.

	History:

***************************************************************************/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#else
#include "rt_config.h"
#endif

#if defined(RTMP_MAC) || defined(RLT_MAC)

#ifdef CONFIG_AP_SUPPORT
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
VOID APMlmeDynamicTxRateSwitchingLegacy(RTMP_ADAPTER *pAd, UINT i)
{
	PUCHAR pTable;
	UCHAR TableSize = 0, TrainUp, TrainDown;
	UCHAR UpRateIdx, DownRateIdx, CurrRateIdx;
	MAC_TABLE_ENTRY *pEntry;
	RTMP_RA_LEGACY_TB *pCurrTxRate, *pTmpTxRate = NULL;
	CHAR Rssi, TmpIdx = 0;
	ULONG TxTotalCnt, TxErrorRatio = 0, TxSuccess, TxRetransmit, TxFailCount;

	pEntry = &pAd->MacTab.Content[i];
    pTable = pEntry->pTable;

	/* NICUpdateFifoStaCounters(pAd); */

	if (pAd->MacTab.Size == 1)
	{
		TX_STA_CNT1_STRUC StaTx1;
		TX_STA_CNT0_STRUC TxStaCnt0;

		/* Update statistic counter */
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

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
		if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT)) {
			if (pAd->chipCap.FlgHwFifoExtCap)
			{
				if (pEntry->wcid >= 1 && pEntry->wcid <= 8)
				{
					ULONG 	HwTxCnt, HwErrRatio;

					RtAsicGetFifoTxCnt(pAd, pEntry);
					HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
					if (HwTxCnt)
						HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
					else
						HwErrRatio = 0;
						
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL,DBG_LVL_INFO,
							("%s()=>Wcid:%d, MCS:%d, CuTxRaIdx=%d,TxErrRatio(Hw:%ld-%ld%%, Sw:%ld-%ld%%)\n", 
							__FUNCTION__, pEntry->wcid, pEntry->HTPhyMode.field.MCS,
							pEntry->CurrTxRateIndex,
							HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

					TxSuccess = pEntry->fifoTxSucCnt;
					TxRetransmit = pEntry->fifoTxRtyCnt;
					TxTotalCnt = HwTxCnt;
					TxErrorRatio = HwErrRatio;
				}
			}
		}
#endif /* FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	}

	/* Save LastTxOkCount, LastTxPER and last MCS action for APQuickResponeForRateUpExec */
	pEntry->LastTxOkCount = TxSuccess;
	pEntry->LastTxPER = (TxTotalCnt == 0 ? 0 : (UCHAR)TxErrorRatio);
	pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;

	/* different calculation in APQuickResponeForRateUpExec() */
	Rssi = RTMPAvgRssi(pAd, pEntry->wdev, &pEntry->RssiSample);

	CurrRateIdx = UpRateIdx = DownRateIdx = pEntry->CurrTxRateIndex;

	/* decide the next upgrade rate and downgrade rate, if any */
	pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, CurrRateIdx);

	if ((pCurrTxRate->Mode <= MODE_CCK) && (pEntry->SupportRateMode <= SUPPORT_CCK_MODE))
	{
		TmpIdx = CurrRateIdx + 1;
		while(TmpIdx < TableSize)
		{
			pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
			if (pEntry->SupportCCKMCS & (1<<pTmpTxRate->CurrMCS))
			{
				UpRateIdx = TmpIdx;
				break;
			}
			TmpIdx++;
		}

		TmpIdx = CurrRateIdx - 1;
		while(TmpIdx >= 0)
		{
			pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
			if (pEntry->SupportCCKMCS & (1<<pTmpTxRate->CurrMCS))
			{
				DownRateIdx = TmpIdx;
				break;
			}
			TmpIdx--;
		}
	}		
	else if ((pCurrTxRate->Mode <= MODE_OFDM) && (pEntry->SupportRateMode < SUPPORT_HT_MODE))
	{
		TmpIdx = CurrRateIdx + 1;
		while(TmpIdx < TableSize)
		{
			pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
			if (pEntry->SupportOFDMMCS & (1<<pTmpTxRate->CurrMCS))
			{
				UpRateIdx = TmpIdx;
				break;
			}
			TmpIdx++;
		}

		TmpIdx = CurrRateIdx - 1;
		while(TmpIdx >= 0)
		{
			pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
			if (pEntry->SupportOFDMMCS & (1<<pTmpTxRate->CurrMCS))
			{
				DownRateIdx = TmpIdx;
				break;
			}
			TmpIdx--;
		}
	}
	else
	{
		/* decide the next upgrade rate and downgrade rate, if any*/
	    if ((CurrRateIdx > 0) && (CurrRateIdx < (TableSize - 1)))
		{
			TmpIdx = CurrRateIdx + 1;
			while(TmpIdx < TableSize)
			{
				pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
				if (pEntry->SupportHTMCS & (1<<pTmpTxRate->CurrMCS))
				{
					UpRateIdx = TmpIdx;
					break;
				}
				TmpIdx++;
			}

			TmpIdx = CurrRateIdx - 1;
			while(TmpIdx >= 0)
			{
				pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
				if (pEntry->SupportHTMCS & (1<<pTmpTxRate->CurrMCS))
				{
					DownRateIdx = TmpIdx;
					break;
				}
				TmpIdx--;
			}
		}
		else if (CurrRateIdx == 0)
		{
			TmpIdx = CurrRateIdx + 1;
			while(TmpIdx < TableSize)
			{
				pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
				if (pEntry->SupportHTMCS & (1<<pTmpTxRate->CurrMCS))
				{
					UpRateIdx = TmpIdx;
					break;
				}
				TmpIdx++;
			}

			DownRateIdx = CurrRateIdx;
		}
		else if (CurrRateIdx == (TableSize - 1))
		{
			UpRateIdx = CurrRateIdx;

			TmpIdx = CurrRateIdx - 1;
			while(TmpIdx >= 0)
			{
				pTmpTxRate = PTX_RA_LEGACY_ENTRY(pTable, TmpIdx);
				if (pEntry->SupportHTMCS & (1<<pTmpTxRate->CurrMCS))
				{
					DownRateIdx = TmpIdx;
					break;
				}
				TmpIdx--;
			}
		}
	}

#ifdef DOT11_N_SUPPORT
	/*
		when Rssi > -65, there is a lot of interference usually. therefore, the algorithm
		tends to choose the mcs lower than the optimal one.
		by increasing the thresholds, the chosen mcs will be closer to the optimal mcs
	*/
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		TrainUp		= pCurrTxRate->TrainUp;
		TrainDown	= pCurrTxRate->TrainDown;
	}


#ifdef DBG_CTRL_SUPPORT
	/* Debug option: Concise RA log */
	if (pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG)
		MlmeRALog(pAd, pEntry, RAL_OLD_DRS, TxErrorRatio, TxTotalCnt);
#endif /* DBG_CTRL_SUPPORT */


	/* Check for low traffic case */
	if (TxTotalCnt <= 15)
	{
		UCHAR	TxRateIdx;
		CHAR	mcs[24];

		/* Check existence and get the index of each MCS */
		MlmeGetSupportedMcs(pAd, pTable, mcs);

		/* Select the Tx rate based on the RSSI */
		TxRateIdx = MlmeSelectTxRate(pAd, pEntry, mcs, Rssi, 0);


		if (TxRateIdx != pEntry->CurrTxRateIndex
#ifdef TXBF_SUPPORT
			|| pEntry->phyETxBf || pEntry->phyITxBf
#endif /* TXBF_SUPPORT */
			)
		{
			pEntry->CurrTxRateIndex = TxRateIdx;
#ifdef TXBF_SUPPORT
			pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
#endif /* TXBF_SUPPORT */
			MlmeNewTxRate(pAd, pEntry);
			if (!pEntry->fLastSecAccordingRSSI)
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("DRS: TxTotalCnt <= 15, switch MCS according to RSSI (%d)\n", Rssi));
		}

		MlmeClearAllTxQuality(pEntry);
		pEntry->fLastSecAccordingRSSI = TRUE;

		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);


#if defined(TXBF_SUPPORT) && (!defined(MT_MAC))
#ifdef DBG_CTRL_SUPPORT
		/* In Unaware mode always try to send sounding */
		if (pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)
			eTxBFProbing(pAd, pEntry);
#endif /* DBG_CTRL_SUPPORT */
#endif /* TXBF_SUPPORT */
		return;
	}

	if (pEntry->fLastSecAccordingRSSI == TRUE)
	{
		pEntry->fLastSecAccordingRSSI = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);

#if defined(TXBF_SUPPORT) && (!defined(MT_MAC))
		if (pAd->chipCap.FlgHwTxBfCap)
		eTxBFProbing(pAd, pEntry);
#endif /* TXBF_SUPPORT */

		return;
	}

	pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

	/* Select rate based on PER */
	MlmeOldRateAdapt(pAd, pEntry, CurrRateIdx, UpRateIdx, DownRateIdx, TrainUp, TrainDown, TxErrorRatio);

#ifdef DOT11N_SS3_SUPPORT
	/* Turn off RDG when 3s and rx count > tx count*5 */
	MlmeCheckRDG(pAd, pEntry);
#endif /* DOT11N_SS3_SUPPORT */

	/* reset all OneSecTx counters */
	RESET_ONE_SEC_TX_CNT(pEntry);

#if defined(TXBF_SUPPORT) && (!defined(MT_MAC))
	if (pAd->chipCap.FlgHwTxBfCap)
		eTxBFProbing(pAd, pEntry);
#endif /* TXBF_SUPPORT */


}


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
VOID APQuickResponeForRateAdaptLegacy(
    IN PRTMP_ADAPTER pAd,
    IN UINT idx)
{
	PUCHAR					pTable;
	UCHAR					CurrRateIdx;
	ULONG					AccuTxTotalCnt, TxTotalCnt, TxCnt;
	ULONG					TxErrorRatio = 0;
	MAC_TABLE_ENTRY			*pEntry;
	RTMP_RA_LEGACY_TB *pCurrTxRate;
	UCHAR					TrainUp, TrainDown;
	CHAR					Rssi, ratio;
	ULONG					TxSuccess, TxRetransmit, TxFailCount;
#ifdef TXBF_SUPPORT
	BOOLEAN					CurrPhyETxBf, CurrPhyITxBf;
#endif /* TXBF_SUPPORT */


	pEntry = &pAd->MacTab.Content[idx];
    pTable = pEntry->pTable;
	Rssi = RTMPAvgRssi(pAd, pEntry->wdev, &pEntry->RssiSample);

	if (pAd->MacTab.Size == 1)
	{
		TX_STA_CNT1_STRUC		StaTx1;
		TX_STA_CNT0_STRUC		TxStaCnt0;

       	/* Update statistic counter */
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
			Rssi = (pEntry->RssiSample.AvgRssi[0] + pEntry->RssiSample.AvgRssi[1]) >> 1;
		else
			Rssi = pEntry->RssiSample.AvgRssi[0];

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
#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
		if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT)) {
			if (pAd->chipCap.FlgHwFifoExtCap)
			{
				if ((pEntry->wcid >= 1) && (pEntry->wcid <= 8))
				{
					ULONG	HwTxCnt, HwErrRatio;

					RtAsicGetFifoTxCnt(pAd, pEntry);
					HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
					if (HwTxCnt)
						HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
					else
						HwErrRatio = 0;
						
					MTWF_LOG(DBG_CAT_RA,DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s()=>Wcid:%d, MCS:%d, TxErrRation(Hw:0x%lx-0x%lx, Sw:0x%lx-%lx)\n", 
							__FUNCTION__, pEntry->wcid, pEntry->HTPhyMode.field.MCS, 
							HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

					TxSuccess = pEntry->fifoTxSucCnt;
					TxRetransmit = pEntry->fifoTxRtyCnt;
					TxErrorRatio = HwErrRatio;
					TxTotalCnt = HwTxCnt;
					TxCnt = HwTxCnt;
				}
			}
		}
#endif /* FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	}

	CurrRateIdx = pEntry->CurrTxRateIndex;
#ifdef TXBF_SUPPORT
	CurrPhyETxBf = pEntry->phyETxBf;
	CurrPhyITxBf = pEntry->phyITxBf;
#endif /* TXBF_SUPPORT */
	pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, CurrRateIdx);

#ifdef DOT11_N_SUPPORT
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		TrainUp		= pCurrTxRate->TrainUp;
		TrainDown	= pCurrTxRate->TrainDown;
	}

			
#ifdef DBG_CTRL_SUPPORT
	/* Debug option: Concise RA log */
	if (pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG)
		MlmeRALog(pAd, pEntry, RAL_QUICK_DRS, TxErrorRatio, TxTotalCnt);
#endif /* DBG_CTRL_SUPPORT */

	if (TxCnt <= 15 && pEntry->HTPhyMode.field.MCS > 1)
	{
		MlmeClearAllTxQuality(pEntry);

		/* Set current up MCS at the worst quality */
		if (pEntry->LastSecTxRateChangeAction == RATE_UP)
		{
			MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);
		}

		/* Go back to the original rate */
		MlmeRestoreLastRate(pEntry);

		MlmeNewTxRate(pAd, pEntry);


		// TODO: should we reset all OneSecTx counters?
		/* RESET_ONE_SEC_TX_CNT(pEntry); */

		return;
	}

	pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

	/* Compare throughput */
	do
	{
		ULONG OneSecTxNoRetryOKRationCount;

		/*
			Compare throughput.
			LastTxCount is based on a time interval of "500" msec or "500-pAd->ra_fast_interval" ms.
		*/
		if ((pEntry->LastTimeTxRateChangeAction == RATE_NO_CHANGE)
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
		)
			ratio = RA_INTERVAL/pAd->ra_fast_interval;
		else
			ratio = (RA_INTERVAL-pAd->ra_fast_interval)/pAd->ra_fast_interval;

		/* downgrade TX quality if PER >= Rate-Down threshold */
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

		/* perform DRS - consider TxRate Down first, then rate up. */
		if (pEntry->LastSecTxRateChangeAction == RATE_UP)
		{
// TODO: gaa - use different criterion for train up in Old RA?
			/*if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount) */
			if (TxErrorRatio >= TrainDown)
			{
#ifdef TXBF_SUPPORT
				/* If PER>50% or TP<lastTP/2 then double the TxQuality delay */
				if ((TxErrorRatio > 50) || (OneSecTxNoRetryOKRationCount < pEntry->LastTxOkCount/2))
					MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND*2);
				else
#endif /* TXBF_SUPPORT */
					MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);

				MlmeRestoreLastRate(pEntry);
			}
			else
			{
			}
		}
		else if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
		{
			/* if ((TxErrorRatio >= 50) || (TxErrorRatio >= TrainDown)) */
			if ((TxErrorRatio >= 50) && (TxErrorRatio >= TrainDown))
			{
			}
			else if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
			{
				MlmeRestoreLastRate(pEntry);
			}
			else
			{
				MTWF_LOG(DBG_CAT_RA,DBG_SUBCAT_ALL, DBG_LVL_INFO,("QuickDRS: (Down) keep rate-down (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
			}
		}
	}while (FALSE);

#ifdef TXBF_SUPPORT
	/* Remember last good non-BF rate */
	if (!pEntry->phyETxBf && !pEntry->phyITxBf)
		pEntry->lastNonBfRate = pEntry->CurrTxRateIndex;
#endif /* TXBF_SUPPORT */

	/* If rate changed then update the history and set the new tx rate */
	if ((pEntry->CurrTxRateIndex != CurrRateIdx)
#ifdef TXBF_SUPPORT
		|| (pEntry->phyETxBf!=CurrPhyETxBf) || (pEntry->phyITxBf!=CurrPhyITxBf)
#endif /* TXBF_SUPPORT */
	)
	{
		/* if rate-up happen, clear all bad history of all TX rates */
		if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
		{
			pEntry->TxRateUpPenalty = 0;
			if (pEntry->CurrTxRateIndex != CurrRateIdx)
				MlmeClearTxQuality(pEntry);
		}
		/* if rate-down happen, only clear DownRate's bad history */
		else if (pEntry->LastSecTxRateChangeAction == RATE_UP)
		{
			pEntry->TxRateUpPenalty = 0;           /* no penalty */
			MlmeSetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);
			pEntry->PER[pEntry->CurrTxRateIndex] = 0;
		}

		MlmeNewTxRate(pAd, pEntry);
	}

	// TODO: should we reset all OneSecTx counters?
	/* RESET_ONE_SEC_TX_CNT(pEntry); */
}
#endif /* CONFIG_AP_SUPPORT */




/*
	MlmeOldRateAdapt - perform Rate Adaptation based on PER using old RA algorithm
		pEntry - the MAC table entry
		CurrRateIdx - the index of the current rate
		UpRateIdx, DownRateIdx - UpRate and DownRate index
		TrainUp, TrainDown - TrainUp and Train Down threhsolds
		TxErrorRatio - the PER

		On exit:
			pEntry->LastSecTxRateChangeAction = RATE_UP or RATE_DOWN if there was a change
			pEntry->CurrTxRateIndex = new rate index
			pEntry->TxQuality is updated
*/
VOID MlmeOldRateAdapt(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN UCHAR			CurrRateIdx,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio)
{
	BOOLEAN	bTrainUp = FALSE;
#ifdef TXBF_SUPPORT
	UCHAR *pTable = pEntry->pTable;
	BOOLEAN invertTxBf = FALSE;
#endif /* TXBF_SUPPORT */

	pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;

	/* Downgrade TX quality if PER >= Rate-Down threshold */
	if (TxErrorRatio >= TrainDown)
	{
		MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);
#ifdef TXBF_SUPPORT
		/*
			Need to train down. If BF and last Non-BF isn't too much lower then
			go to last Non-BF rate. Otherwise just go to the down rate
		*/
		if ((pEntry->phyETxBf || pEntry->phyITxBf) &&
			(DownRateIdx - pEntry->lastNonBfRate)<2 
#ifdef DBG_CTRL_SUPPORT
			&& ((pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)==0)
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Go directly to last non-BF rate without 100 msec check */
			pEntry->CurrTxRateIndex = pEntry->lastNonBfRate;
			pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
			MlmeNewTxRate(pAd, pEntry);
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO,("DRS: --TX rate from %d to %d \n", CurrRateIdx, pEntry->CurrTxRateIndex));
			return;
		}
		else
#endif /* TXBF_SUPPORT */
		if (CurrRateIdx != DownRateIdx)
		{
			pEntry->CurrTxRateIndex = DownRateIdx;
			pEntry->LastSecTxRateChangeAction = RATE_DOWN;
		}
	}
	else
	{
		/* Upgrade TX quality if PER <= Rate-Up threshold */
		if (TxErrorRatio <= TrainUp)
		{
			bTrainUp = TRUE;
			MlmeDecTxQuality(pEntry, CurrRateIdx);  /* quality very good in CurrRate */

			if (pEntry->TxRateUpPenalty)
				pEntry->TxRateUpPenalty --;
			else
				MlmeDecTxQuality(pEntry, UpRateIdx);    /* may improve next UP rate's quality */
		}

		if (bTrainUp)
		{
			/* Train up if up rate quality is 0 */
			if ((CurrRateIdx != UpRateIdx) && (MlmeGetTxQuality(pEntry, UpRateIdx) <= 0))
			{
				pEntry->CurrTxRateIndex = UpRateIdx;
				pEntry->LastSecTxRateChangeAction = RATE_UP;
			}
#ifdef TXBF_SUPPORT
			else if (((CurrRateIdx != UpRateIdx) || (TxErrorRatio > TrainUp))
#ifdef DBG_CTRL_SUPPORT
					&& ((pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)==0)
#endif /* DBG_CTRL_SUPPORT */
			)
			{
				/* UpRate TxQuality is not 0. Try to invert BF state */
				if (pEntry->phyETxBf || pEntry->phyITxBf)
				{
					/* BF tries same MCS, non-BF */
					if (pEntry->TxQuality[CurrRateIdx])
						pEntry->TxQuality[CurrRateIdx]--;

					if (pEntry->TxQuality[CurrRateIdx]==0)
					{
						invertTxBf = TRUE;
						pEntry->CurrTxRateIndex = CurrRateIdx;
						pEntry->LastSecTxRateChangeAction = RATE_UP;
					}
				}
				else if (pEntry->eTxBfEnCond>0 || pEntry->iTxBfEn)
				{
					RTMP_RA_LEGACY_TB *pUpRate = PTX_RA_LEGACY_ENTRY(pTable, UpRateIdx);
					RTMP_RA_LEGACY_TB *pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, CurrRateIdx);

					/* First try Up Rate with BF */
					if ((CurrRateIdx != UpRateIdx) && MlmeTxBfAllowed(pAd, pEntry, pUpRate))
					{
						if (pEntry->BfTxQuality[UpRateIdx])
							pEntry->BfTxQuality[UpRateIdx]--;

						if (pEntry->BfTxQuality[UpRateIdx]==0)
						{
							invertTxBf = TRUE;
							pEntry->CurrTxRateIndex = UpRateIdx;
							pEntry->LastSecTxRateChangeAction = RATE_UP;
						}
					}

					/* Try Same Rate if Up Rate failed */
					if (pEntry->LastSecTxRateChangeAction==RATE_NO_CHANGE &&
						MlmeTxBfAllowed(pAd, pEntry, pCurrTxRate))
					{
						if (pEntry->BfTxQuality[CurrRateIdx])
							pEntry->BfTxQuality[CurrRateIdx]--;

						if (pEntry->BfTxQuality[CurrRateIdx]==0)
						{
							invertTxBf = TRUE;
							pEntry->CurrTxRateIndex = CurrRateIdx;
							pEntry->LastSecTxRateChangeAction = RATE_UP;
						}
					}
				}
			}
#endif /* TXBF_SUPPORT */
		}
	}

	/* Handle the rate change */
	if (pEntry->LastSecTxRateChangeAction != RATE_NO_CHANGE)
	{
		pEntry->TxRateUpPenalty = 0;

		/* Save last rate information */
		pEntry->lastRateIdx = CurrRateIdx;
#ifdef TXBF_SUPPORT
		if (pEntry->eTxBfEnCond>0)
		{
			pEntry->lastRatePhyTxBf = pEntry->phyETxBf;
			pEntry->phyETxBf ^= invertTxBf;
		}
		else
		{
			pEntry->lastRatePhyTxBf = pEntry->phyITxBf;
			pEntry->phyITxBf ^= invertTxBf;
		}
#endif /* TXBF_SUPPORT */

		/* Update TxQuality */
		if (pEntry->LastSecTxRateChangeAction == RATE_UP)
		{
			/* Clear history if normal train up */
			if (pEntry->lastRateIdx != pEntry->CurrTxRateIndex)
				MlmeClearTxQuality(pEntry);
		}
		else
		{
			/* Clear the down rate history */
			MlmeSetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);
			pEntry->PER[pEntry->CurrTxRateIndex] = 0;
		}

		/* Set timer for check in 100 msec */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (!pAd->ApCfg.ApQuickResponeForRateUpTimerRunning)
			{
				RTMPSetTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, pAd->ra_fast_interval);
				pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

		/* Update PHY rate */
		MlmeNewTxRate(pAd, pEntry);
	}
}

#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

