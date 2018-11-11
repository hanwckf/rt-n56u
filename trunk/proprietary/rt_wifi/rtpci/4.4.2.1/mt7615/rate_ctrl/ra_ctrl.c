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

#include "rt_config.h"

#if defined(RTMP_MAC) || defined(RLT_MAC)

/* MlmeGetSupportedMcs - fills in the table of mcs with index into the pTable
		pAd - pointer to adapter
		pTable - pointer to the Rate Table. Assumed to be a table without mcsGroup values
		mcs - table of MCS index into the Rate Table. -1 => not supported
*/
VOID MlmeGetSupportedMcs(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	*pTable,
	OUT CHAR 	mcs[])
{
	CHAR	idx;
	RTMP_RA_LEGACY_TB *pCurrTxRate;

	for (idx = 0; idx < 24; idx++)
		mcs[idx] = -1;

	/*  check the existence and index of each needed MCS */
	for (idx=0; idx<RATE_TABLE_SIZE(pTable); idx++)
	{
		pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, idx);

		/*  Rate Table may contain CCK and MCS rates. Give HT/Legacy priority over CCK */
		if (pCurrTxRate->CurrMCS==MCS_0 && (mcs[0]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[0] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_1 && (mcs[1]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[1] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_2 && (mcs[2]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[2] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_3)
			mcs[3] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_4)
			mcs[4] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_5)
			mcs[5] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_6)
			mcs[6] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))
			mcs[7] = idx;
#ifdef DOT11_N_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_12)
			mcs[12] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_13)
			mcs[13] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_14)
			mcs[14] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))
		{
			mcs[15] = idx;
		}
#ifdef DOT11N_SS3_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_20)
			mcs[20] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_21)
			mcs[21] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_22)
			mcs[22] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_23)
			mcs[23] = idx;
#endif /*  DOT11N_SS3_SUPPORT */
#endif /*  DOT11_N_SUPPORT */
	}

#ifdef DBG_CTRL_SUPPORT
	/*  Debug Option: Disable highest MCSs when picking initial MCS based on RSSI */
	if (pAd->CommonCfg.DebugFlags & DBF_INIT_MCS_DIS1)
		mcs[23] = mcs[15] = mcs[7] = mcs[22] = mcs[14] = mcs[6] = 0;
#endif /* DBG_CTRL_SUPPORT */
}


/*  MlmeClearTxQuality - Clear TxQuality history only for the active BF state */
VOID MlmeClearTxQuality(MAC_TABLE_ENTRY *pEntry)
{
#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
	if (pEntry->phyETxBf || pEntry->phyITxBf)
		os_zero_mem(pEntry->BfTxQuality, sizeof(pEntry->BfTxQuality));
	else
#endif /*  TXBF_AWARD */		
#endif /*  TXBF_SUPPORT */
		os_zero_mem(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	os_zero_mem(pEntry->PER, sizeof(pEntry->PER));
}


/*  MlmeClearAllTxQuality - Clear both BF and non-BF TxQuality history */
VOID MlmeClearAllTxQuality(MAC_TABLE_ENTRY *pEntry)
{
#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
	os_zero_mem(pEntry->BfTxQuality, sizeof(pEntry->BfTxQuality));
#endif /*  TXBF_AWARD */
#endif
	os_zero_mem(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	os_zero_mem(pEntry->PER, sizeof(pEntry->PER));
}


/*  MlmeDecTxQuality - Decrement TxQuality of specified rate table entry */
VOID MlmeDecTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex)
{
#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
	if (pEntry->phyETxBf || pEntry->phyITxBf) {
		if (pEntry->BfTxQuality[rateIndex])
			pEntry->BfTxQuality[rateIndex]--;
	}
	else
#endif /*  TXBF_AWARD */
#endif /*  TXBF_SUPPORT */
	if (pEntry->TxQuality[rateIndex])
		pEntry->TxQuality[rateIndex]--;
}


VOID MlmeSetTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rate_idx, USHORT quality)
{
#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
	if (pEntry->phyETxBf || pEntry->phyITxBf)
		pEntry->BfTxQuality[rate_idx] = quality;
	else
#endif /*  TXBF_AWARD */		
#endif /*  TXBF_SUPPORT */
		pEntry->TxQuality[rate_idx] = quality;
}


USHORT MlmeGetTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex)
{
#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
	if (pEntry->phyETxBf || pEntry->phyITxBf)
		return pEntry->BfTxQuality[rateIndex];
#endif /*  TXBF_AWARD */
#endif /*  TXBF_SUPPORT */
	return pEntry->TxQuality[rateIndex];
}


#ifdef CONFIG_AP_SUPPORT
VOID APMlmeSetTxRate(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate)
{
	UCHAR tx_mode = pTxRate->Mode;

#ifdef DOT11_VHT_AC
	UCHAR tx_bw = pTxRate->BW;
	if ((pAd->chipCap.phy_caps & fPHY_CAP_VHT) && 
		((pEntry->pTable == RateTableVht2S) || (pEntry->pTable == RateTableVht1S) ||
		 (pEntry->pTable == RateTableVht1S_MCS9) ||
		 (pEntry->pTable == RateTableVht2S_BW20) ||
		 (pEntry->pTable == RateTableVht2S_BW40) ||
		 (pEntry->pTable == RateTableVht2S_MCS7)))
	{
		RTMP_RA_GRP_TB *pAdaptTbEntry = (RTMP_RA_GRP_TB *)pTxRate;
		UCHAR bw_cap = BW_20;
			
		if (pEntry->MaxHTPhyMode.field.BW != pAdaptTbEntry->BW)
		{
			switch (pEntry->MaxHTPhyMode.field.BW)
			{
				case BW_80:
					bw_cap = pAdaptTbEntry->BW;
					break;
				case BW_40:
					if (pAdaptTbEntry->BW == BW_80)
						bw_cap = BW_40;
					else
						bw_cap = pAdaptTbEntry->BW;
					break;
				case BW_20:
				default:
					if (pAdaptTbEntry->BW == BW_80 || pAdaptTbEntry->BW == BW_40)
						bw_cap = BW_20;
					else
						bw_cap = pAdaptTbEntry->BW;
					break;
			}
			tx_bw = bw_cap;
		}
		else
			tx_bw = pAdaptTbEntry->BW;

		if ((pEntry->force_op_mode == TRUE))
		{
			switch (pEntry->operating_mode.ch_width) {
				case 1:
					bw_cap = BW_40;
					break;
				case 2:
					bw_cap = BW_80;
					break;
				case 0:
				default:
					bw_cap = BW_20;
					break;
			}
			if ((tx_bw != BW_10) && (tx_bw >= bw_cap))
				tx_bw = bw_cap;
		}


#ifdef WFA_VHT_PF
		if (pAd->CommonCfg.vht_bw_signal && tx_bw == BW_40 &&
			pAdaptTbEntry->Mode == MODE_VHT &&
			(pAd->MacTab.fAnyStation20Only == FALSE))
		{
			// try to use BW_40 for VHT mode!
			tx_mode = pAdaptTbEntry->Mode;
		}
#endif /* WFA_VHT_PF */
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): txbw=%d, txmode=%d\n", __FUNCTION__, tx_bw, tx_mode));
	}
#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
	if (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD)
	{
		if ((pTxRate->STBC) && (pEntry->MaxHTPhyMode.field.STBC))
			pEntry->HTPhyMode.field.STBC = STBC_USE;
		else
			pEntry->HTPhyMode.field.STBC = STBC_NONE;

		if ((pTxRate->ShortGI || pAd->WIFItestbed.bShortGI) && (pEntry->MaxHTPhyMode.field.ShortGI))
			pEntry->HTPhyMode.field.ShortGI = GI_400;
		else
			pEntry->HTPhyMode.field.ShortGI = GI_800;
	}
		
	/* TODO: will check ldpc if related to rate table */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE) ||
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE)) {
		pEntry->HTPhyMode.field.ldpc = TRUE;
	} else {
		pEntry->HTPhyMode.field.ldpc = FALSE;
	}

#ifdef DOT11_VHT_AC
	if (tx_mode == MODE_VHT)
	{
		if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI80_CAPABLE)) && 
			(pTxRate->ShortGI
#ifdef WFA_VHT_PF
			|| pAd->vht_force_sgi
#endif /* WFA_VHT_PF */
			)
		)
			pEntry->HTPhyMode.field.ShortGI = GI_400;
		else
			pEntry->HTPhyMode.field.ShortGI = GI_800;

		if (pTxRate->STBC && (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE)))
			pEntry->HTPhyMode.field.STBC = STBC_USE;
		else
			pEntry->HTPhyMode.field.STBC = STBC_NONE;
	}
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

	if (pTxRate->CurrMCS < MCS_AUTO)
		pEntry->HTPhyMode.field.MCS = pTxRate->CurrMCS;

	pEntry->HTPhyMode.field.MODE = tx_mode;

#ifdef DOT11_N_SUPPORT
	if ((pAd->WIFItestbed.bGreenField & pEntry->HTCapability.HtCapInfo.GF) && (pEntry->HTPhyMode.field.MODE == MODE_HTMIX))
	{
		/* force Tx GreenField */
		pEntry->HTPhyMode.field.MODE = MODE_HTGREENFIELD;
	}

	/* BW depends on BSSWidthTrigger and Negotiated BW */
	if (pAd->CommonCfg.bRcvBSSWidthTriggerEvents ||
		(pEntry->MaxHTPhyMode.field.BW==BW_20) ||
		(pAd->CommonCfg.BBPCurrentBW==BW_20))
		pEntry->HTPhyMode.field.BW = BW_20;
	else
		pEntry->HTPhyMode.field.BW = BW_40;

#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW==BW_80 &&
		pEntry->MaxHTPhyMode.field.BW == BW_80 &&
		pEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
		pEntry->HTPhyMode.field.BW = BW_80;

#ifdef NEW_RATE_ADAPT_SUPPORT
	if ((pEntry->pTable == RateTableVht2S) ||
		(pEntry->pTable == RateTableVht2S_BW20) ||
		(pEntry->pTable == RateTableVht2S_BW40) ||
		(pEntry->pTable == RateTableVht1S) ||
		(pEntry->pTable == RateTableVht1S_MCS9) ||
		(pEntry->pTable == RateTableVht2S_MCS7))
	{
		RTMP_RA_GRP_TB *pAdaptTbEntry = (RTMP_RA_GRP_TB *)pTxRate;
		pEntry->HTPhyMode.field.MCS = pAdaptTbEntry->CurrMCS | ((pAdaptTbEntry->dataRate -1) <<4);		
		pEntry->HTPhyMode.field.BW = tx_bw;

#ifdef WFA_VHT_PF
		if ((pAd->vht_force_tx_stbc)
			&& (pEntry->HTPhyMode.field.MODE == MODE_VHT)
			&& (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE))
			&& (pEntry->HTPhyMode.field.STBC == STBC_NONE)
		)
		{
			pEntry->HTPhyMode.field.MCS = pAdaptTbEntry->CurrMCS;
			pEntry->HTPhyMode.field.STBC = STBC_USE;
		}
#endif /* WFA_VHT_PF */
	}
	else if (IS_VHT_STA(pEntry))
	{
		UCHAR bw_max = pEntry->MaxHTPhyMode.field.BW;
		if (pEntry->force_op_mode == TRUE)
		{
			switch (pEntry->operating_mode.ch_width) {
				case 1:
					bw_max = BW_40;
					break;
				case 2: /* not support for BW_80 for other rate table */
				case 0:
				default:
					bw_max = BW_20;
					break;
			}
		}

		if ( (bw_max != BW_10) &&
			(bw_max > pAd->CommonCfg.BBPCurrentBW))
		{
			bw_max = pAd->CommonCfg.BBPCurrentBW;
		}
		pEntry->HTPhyMode.field.BW = bw_max;		
	}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
	if (pEntry->pTable == Ags2x2VhtRateTable)
	{
		RTMP_RA_AGS_TB *pAgsTbEntry = (RTMP_RA_AGS_TB *)pTxRate;
		pEntry->HTPhyMode.field.MCS = pAgsTbEntry->CurrMCS | (pAgsTbEntry->Nss <<4);
	}
#endif /* AGS_SUPPORT */
#endif /* DOT11_VHT_AC */

#ifdef RANGE_EXTEND
#ifdef NEW_RATE_ADAPT_SUPPORT
	/* 20 MHz Fallback */
	if ((tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD) &&
	    pEntry->HTPhyMode.field.BW == BW_40 &&
	    ADAPT_RATE_TABLE(pEntry->pTable))
	{
		if (pEntry->HTPhyMode.field.MCS == 32
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0) == 0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map HT Duplicate to 20MHz MCS0 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 0;
		}
		else if (pEntry->HTPhyMode.field.MCS == 0 &&
				(pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ) == 0
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS1) == 0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map 40MHz MCS0 to 20MHz MCS1 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 1;
		}
		else if (pEntry->HTPhyMode.field.MCS == 8
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_ENABLE_20MHZ_MCS8)
#endif /* DBG_CTRL_SUPPORT */
			)
		{
			/* Map 40MHz MCS8 to 20MHz MCS8 */
			pEntry->HTPhyMode.field.BW = BW_20;
		}
	}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
	/* Debug Option: Force BW */
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_40MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_40;
	}
	else if (pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_20;
	}
#endif /* DBG_CTRL_SUPPORT */
#endif /* RANGE_EXTEND */

	/* Reexam each bandwidth's SGI support. */
	if ((pEntry->HTPhyMode.field.BW==BW_20 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
		(pEntry->HTPhyMode.field.BW==BW_40 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)) )
		pEntry->HTPhyMode.field.ShortGI = GI_800;

#ifdef DBG_CTRL_SUPPORT
	/* Debug option: Force Short GI */
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_SGI)
		pEntry->HTPhyMode.field.ShortGI = GI_400;
#endif /* DBG_CTRL_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
		RtAsicFifoExtEntryClean(pAd, pEntry);
#endif /* FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MCS_LUT_SUPPORT
	AsicMcsLutUpdate(pAd, pEntry);
	pEntry->LastTxRate = (USHORT) (pEntry->HTPhyMode.word);
#endif /* MCS_LUT_SUPPORT */


}
#endif /* CONFIG_AP_SUPPORT */



#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef DOT11_VHT_AC
UCHAR* SelectVHTTxRateTableGRP(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;
    UCHAR ucNss = 0;
    UCHAR ucBw = pEntry->MaxHTPhyMode.field.BW;
	
    struct wifi_dev *wdev = pEntry->wdev;

    if (WMODE_CAP_AC(wdev->PhyMode) &&
            (pEntry->SupportRateMode & SUPPORT_VHT_MODE))
    {

        if (pEntry->SupportVHTMCS1SS != 0)
            ucNss = 1;
        if (pEntry->SupportVHTMCS2SS != 0)
            ucNss = 2;
        if (pEntry->SupportVHTMCS3SS != 0)
            ucNss = 3;
        if (pEntry->SupportVHTMCS4SS != 0)
            ucNss = 4;

#ifdef WFA_VHT_PF
        if ((pAd->CommonCfg.vht_nss_cap > 0) &&
                (ucNss > pAd->CommonCfg.vht_nss_cap))
        {
            ucNss = pAd->CommonCfg.vht_nss_cap;
        }
#endif /* WFA_VHT_PF */

        if ((pEntry->force_op_mode == TRUE) && 
                (pEntry->operating_mode.rx_nss_type == 0)) 
        {
            if (pEntry->operating_mode.rx_nss < wlan_config_get_tx_stream(pEntry->wdev))
            {
                ucNss = pEntry->operating_mode.rx_nss + 1;
            }

            switch (pEntry->operating_mode.ch_width)
            {
                case 0:
                    ucBw = BW_20;
                    break;
                case 1:
                    ucBw = BW_40;
                    break;
                case 2:
                default:
                    ucBw = BW_80;
                    break;
            }
        }

        if (ucNss == 2) {
            if (pEntry->MaxHTPhyMode.field.BW == BW_20) {
                pTable = RateTableVht2S_BW20;
            } else if (pEntry->MaxHTPhyMode.field.BW == BW_40) {
                pTable = RateTableVht2S_BW40;
            } else if ((pEntry->SupportVHTMCS1SS & (1 << MCS_8)) && 
                    (pEntry->SupportVHTMCS1SS & (1 << MCS_9))) {
                pTable = RateTableVht2S;
            } else {
                pTable = RateTableVht2S_MCS7;
            }
        } else if (ucNss == 1) {
            if ((pEntry->SupportVHTMCS1SS & (1 << MCS_8)) && 
                    (pEntry->SupportVHTMCS1SS & (1 << MCS_9))) {
                pTable = RateTableVht1S_MCS9;
            } else {
                pTable = RateTableVht1S;
            }
            MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Select RateTableVht%dS%s\n",
                    __FUNCTION__, (ucNss == 2 ? 2 : 1),
                    ((pTable == RateTableVht1S_MCS9) ? "_MCS9" : "")));
        } else {
            MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:unknow ss(%d)\n", __FUNCTION__, ucNss));
        }

        if ((IS_RT8592(pAd) && ( ucBw != BW_40)) ||
                (!IS_RT8592(pAd)))
            return pTable;

#ifdef WFA_VHT_PF
        // TODO: shiang, add for Realtek behavior when run in BW signaling mode test and we are the testbed!
        // TODO: add at 11/15!
        if ((pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_DYNAMIC) &&
                (ucBw == BW_40) &&
                (pAd->MacTab.fAnyStation20Only == FALSE)) {
            return pTable;
        }
#endif /* WFA_VHT_PF */
    }

    if (IS_RT8592(pAd) && 
            WMODE_CAP_AC(wdev->PhyMode) && 
            (pEntry->SupportRateMode & SUPPORT_VHT_MODE) &&
            ucBw == BW_40 && (pAd->LatchRfRegs.Channel > 14)
    )
    {
        if (ucNss == 1) {
            if (pAd->rateAlg == RATE_ALG_GRP)
                pTable = RateSwitchTableAdapt11N1S;
            else
                pTable = RateSwitchTable11N1SForABand;

			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Select RateSwitchTable%s11N1S%s\n",
							__FUNCTION__, 
							((pAd->rateAlg == RATE_ALG_GRP) ? "Adapt" : ""),
							((pAd->rateAlg == RATE_ALG_GRP) ? "ForABand" : "")));
        }
        else if (ucNss == 2)
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N2S;
            } else
                pTable = RateSwitchTable11BGN2SForABand;
            MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Select RateSwitchTable%s11N2S%s\n",
                    __FUNCTION__, 
                    ((pAd->rateAlg == RATE_ALG_GRP) ? "Adapt" : ""),
                    ((pAd->rateAlg == RATE_ALG_GRP) ? "ForABand" : "")));

        } else {
            MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Invalid SS!\n", __FUNCTION__));
        }

    }

    return pTable;
}
#endif /* DOT11_VHT_AC */


UCHAR* SelectTxRateTableGRP(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;
    struct wifi_dev *wdev = pEntry->wdev;


#ifdef DOT11_N_SUPPORT
    /*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (wlan_config_get_tx_stream(pEntry->wdev) == 1)))
    {/* 11BGN 1S AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11N1S;
        }
        else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN1S;
        } else {
            pTable = RateSwitchTable11N1SForABand;
        }

        return pTable;
    }


    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
#ifdef THERMAL_PROTECT_SUPPORT
            (pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
            (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (wlan_config_get_tx_stream(pEntry->wdev) == 2)))
    {/* 11BGN 2S AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N2S;
        } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN2S;
        } else {
            pTable = RateSwitchTable11BGN2SForABand;
        }

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (wlan_config_get_tx_stream(pEntry->wdev) == 3))
    {
        if (pAd->rateAlg == RATE_ALG_GRP)
        {
                pTable = RateSwitchTableAdapt11N3S;
        } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11N3S;
        } else {
            pTable = RateSwitchTable11N3SForABand;
        }

        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (wlan_config_get_tx_stream(pEntry->wdev) == 1)
#ifdef THERMAL_PROTECT_SUPPORT
            || (pAd->force_one_tx_stream == TRUE)
#endif /* THERMAL_PROTECT_SUPPORT */
    ))
    {/* 11N 1S AP*/
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N1S;
            } else if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N1S;
            } else {
                pTable = RateSwitchTable11N1SForABand;
            }
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (wlan_config_get_tx_stream(pEntry->wdev) == 2))
    {/* 11N 2S AP*/
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N2S;
            } else if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N2S;
            } else {
                pTable = RateSwitchTable11N2SForABand;			
            }
        }
        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (wlan_config_get_tx_stream(pEntry->wdev) == 3))
    {
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11N3S;
        } else {
            if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N3S;
            } else {
                pTable = RateSwitchTable11N3SForABand;
            }
        }
        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
    if (wlan_config_get_tx_stream(pEntry->wdev) == 3)
    {
        if  (pEntry->HTCapability.MCSSet[0] != 0x00)
        {
            if (pEntry->HTCapability.MCSSet[1] == 0x00)
            {	/* Only support 1SS */
                if (pEntry->RateLen > 0)
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N1S;
                    } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
                        pTable = RateSwitchTable11N1S;
                    } else {
                        pTable = RateSwitchTable11N1SForABand;	
                    }
                }
                else
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N1S;
                    } else if (pAd->LatchRfRegs.Channel <= 14) {
                        pTable = RateSwitchTable11N1S;
                    } else {
                        pTable = RateSwitchTable11N1SForABand;
                    }
                }
                break;
            }
            else if (pEntry->HTCapability.MCSSet[2] == 0x00)
            {	/* Only support 2SS */
                if (pEntry->RateLen > 0)
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N2S;
                    } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
                        pTable = RateSwitchTable11BGN2S;
                    } else {
                        pTable = RateSwitchTable11BGN2SForABand;
                    }
                }
                else
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N2S;
                    } else if (pAd->LatchRfRegs.Channel <= 14) {
                        pTable = RateSwitchTable11N2S;
                    } else {
                        pTable = RateSwitchTable11N2SForABand;
                    }
                }
                return pTable;
            }
				/* For 3SS case, we use the new rate table, so don't care it here */
        }
    }
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

    if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || 
            WMODE_EQUAL(wdev->PhyMode, WMODE_B))
#ifdef DOT11_N_SUPPORT
            /*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
            /* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
    )
    {/* B only AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11B;
        } else {
            pTable = RateSwitchTable11B;
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
            (pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* B/G  mixed AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11BG;
        } else {
            pTable = RateSwitchTable11BG;
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* G only AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11G;
        } else {
            pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
#ifdef DOT11N_SS3_SUPPORT
        if (wlan_config_get_tx_stream(pEntry->wdev) >= 3)
        {
            if (pAd->rateAlg == RATE_ALG_GRP)
            {
                if (pEntry->HTCapability.MCSSet[2] == 0) {
                    pTable = RateSwitchTableAdapt11N2S;
                } else
                    pTable = RateSwitchTableAdapt11N3S;
            }
            else
            {
                if (pEntry->HTCapability.MCSSet[2] == 0)
                    pTable = RateSwitchTable11N2S;
                else
                    pTable = RateSwitchTable11N3S;
            }
        }
        else
#endif /* DOT11N_SS3_SUPPORT */
        {
            /*
                Temp solution for:
                EX: when the extend rate only supports 6, 12, 24 in
                the association req frame. So the pEntry->RateLen is 7.
            */
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11BG;
            else
                pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */


    return pTable;
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


#ifdef AGS_SUPPORT
#ifdef DOT11_VHT_AC
UCHAR* SelectVHTTxRateTableAGS(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;
    UCHAR ucMcsIdx;
    UCHAR ucNss = 0;
    UCHAR ucBw = pEntry->MaxHTPhyMode.field.BW;
    struct wifi_dev *wdev = pEntry->wdev;

    if (WMODE_CAP_AC(wdev->PhyMode) &&
            (pEntry->SupportRateMode & SUPPORT_VHT_MODE))
    {
        if (pEntry->SupportVHTMCS1SS != 0)
            ucNss =1;
        if (pEntry->SupportVHTMCS2SS != 0)
            ucNss =2;
        if (pEntry->SupportVHTMCS3SS != 0)
            ucNss =3;
        if (pEntry->SupportVHTMCS4SS != 0)
            ucNss =4;

#ifdef WFA_VHT_PF
        if ((pAd->CommonCfg.vht_nss_cap > 0) &&
                (ucNss > pAd->CommonCfg.vht_nss_cap))
        {
            ucNss = pAd->CommonCfg.vht_nss_cap;
        }
#endif /* WFA_VHT_PF */

        if ((pEntry->force_op_mode == TRUE) && 
                (pEntry->operating_mode.rx_nss_type == 0)) 
        {
            if (pEntry->operating_mode.rx_nss < wlan_config_get_tx_stream(pEntry->wdev))
            {
                ucNss = pEntry->operating_mode.rx_nss + 1;
            }

            switch (pEntry->operating_mode.ch_width)
            {
                case 0:
                    ucBw = BW_20;
                    break;
                case 1:
                    ucBw = BW_40;
                    break;
                case 2:
                default:
                    ucBw = BW_80;
                    break;
            }
        }

        if (ucNss == 2) {
            pTable = Ags2x2VhtRateTable;
        } else {
            pTable = Ags1x1VhtRateTable;
        }
        MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Select Ags%dx%dVhtRateTable\n",
            __FUNCTION__, (ucNss == 2 ? 2 : 1), (ucNss == 2 ? 2 : 1)));

        if ((IS_RT8592(pAd) && ( ucBw != BW_40)) ||
                (!IS_RT8592(pAd)))
            return pTable;

#ifdef WFA_VHT_PF
        // TODO: shiang, add for Realtek behavior when run in BW signaling mode test and we are the testbed!
        // TODO: add at 11/15!
        if ((pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_DYNAMIC) &&
                (ucBw == BW_40) &&
                (pAd->MacTab.fAnyStation20Only == FALSE)) {
            return pTable;
        }
#endif /* WFA_VHT_PF */
    }

    if (IS_RT8592(pAd) && 
            WMODE_CAP_AC(wdev->PhyMode) && 
            (pEntry->SupportRateMode & SUPPORT_VHT_MODE) &&
            ucBw == BW_40 && (pAd->LatchRfRegs.Channel > 14)
    )
    {
        if (ucNss == 1) {
            pTable = RateSwitchTable11N1SForABand;

			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Select RateSwitchTable%s11N1S%s\n",
							__FUNCTION__, 
							((pAd->rateAlg == RATE_ALG_GRP) ? "Adapt" : ""),
							((pAd->rateAlg == RATE_ALG_GRP) ? "ForABand" : "")));
        }
        else if (ucNss == 2)
        {
            pTable = RateSwitchTable11BGN2SForABand;
            MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Select RateSwitchTable%s11N2S%s\n",
                    __FUNCTION__, 
                    ((pAd->rateAlg == RATE_ALG_GRP) ? "Adapt" : ""),
                    ((pAd->rateAlg == RATE_ALG_GRP) ? "ForABand" : "")));

        } else {
            MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Invalid SS!\n", __FUNCTION__));
        }
    }

    return pTable
}
#endif /* DOT11_VHT_AC */

UCHAR* SelectTxRateTableAGS(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;
    struct wifi_dev *wdev = pEntry->wdev;

#ifdef DOT11_N_SUPPORT
    /*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (wlan_config_get_tx_stream(pEntry->wdev) == 1)))
    {/* 11BGN 1S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd)) {
            pTable = AGS1x1HTRateTable;
        } else
#endif /* AGS_SUPPORT */
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN1S;
        } else {
            pTable = RateSwitchTable11N1SForABand;
        }

        return pTable;
    }

#ifdef AGS_SUPPORT
    /* only for station */
    if (SUPPORT_AGS(pAd) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (pEntry->HTCapability.MCSSet[2] != 0x00) && 
            (wlan_config_get_tx_stream(pEntry->wdev) == 3))
    {/* 11N 3S */
        pTable = AGS3x3HTRateTable;
        return pTable;
    }
#endif /* AGS_SUPPORT */

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
#ifdef THERMAL_PROTECT_SUPPORT
            (pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
            (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (wlan_config_get_tx_stream(pEntry->wdev) == 2)))
    {/* 11BGN 2S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd))
        {
            pTable = AGS2x2HTRateTable;
        }
        else
#endif /* AGS_SUPPORT */
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11BGN2S;
        else
            pTable = RateSwitchTable11BGN2SForABand;

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (wlan_config_get_tx_stream(pEntry->wdev) == 3))
    {
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11N3S;
        else
            pTable = RateSwitchTable11N3SForABand;

        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (wlan_config_get_tx_stream(pEntry->wdev) == 1)
#ifdef THERMAL_PROTECT_SUPPORT
            || (pAd->force_one_tx_stream == TRUE)
#endif /* THERMAL_PROTECT_SUPPORT */
    ))
    {/* 11N 1S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd))
            pTable = AGS1x1HTRateTable;
        else
#endif /* AGS_SUPPORT */
        {
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11N1S;
            else
                pTable = RateSwitchTable11N1SForABand;
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (wlan_config_get_tx_stream(pEntry->wdev) == 2))
    {/* 11N 2S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd))
            pTable = AGS2x2HTRateTable;
        else
#endif /* AGS_SUPPORT */
        {
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11N2S;
            else
                pTable = RateSwitchTable11N2SForABand;			
        }
        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (wlan_config_get_tx_stream(pEntry->wdev) == 3))
    {
        {
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11N3S;
            else
                pTable = RateSwitchTable11N3SForABand;
        }
        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
    if (wlan_config_get_tx_stream(pEntry->wdev) == 3)
    {
        if  (pEntry->HTCapability.MCSSet[0] != 0x00)
        {
            if (pEntry->HTCapability.MCSSet[1] == 0x00)
            {	/* Only support 1SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;	
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;
                }
                break;
            }
            else if (pEntry->HTCapability.MCSSet[2] == 0x00)
            {	/* Only support 2SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11BGN2S;
                    else
                        pTable = RateSwitchTable11BGN2SForABand;
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N2S;
                    else
                        pTable = RateSwitchTable11N2SForABand;
                }
                return pTable;
            }
				/* For 3SS case, we use the new rate table, so don't care it here */
        }
    }
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

    if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || 
            WMODE_EQUAL(wdev->PhyMode, WMODE_B))
#ifdef DOT11_N_SUPPORT
            /*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
            /* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
    )
    {/* B only AP*/
        pTable = RateSwitchTable11B;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
            (pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* B/G  mixed AP*/
        pTable = RateSwitchTable11BG;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* G only AP*/
        pTable = RateSwitchTable11G;
        return pTable;
    }
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
#ifdef DOT11N_SS3_SUPPORT
        if (wlan_config_get_tx_stream(pEntry->wdev) >= 3)
        {
            if (pEntry->HTCapability.MCSSet[2] == 0)
                pTable = RateSwitchTable11N2S;
            else
                pTable = RateSwitchTable11N3S;
        }
        else
#endif /* DOT11N_SS3_SUPPORT */
        {
            /*
                Temp solution for:
                EX: when the extend rate only supports 6, 12, 24 in
                the association req frame. So the pEntry->RateLen is 7.
            */
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11BG;
            else
                pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */


    return pTable;
}
#endif /* AGS_SUPPORT */


UCHAR* SelectTxRateTable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;
    struct wifi_dev *wdev = pEntry->wdev;

#ifdef DOT11_N_SUPPORT
    /*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (wlan_config_get_tx_stream(pEntry->wdev) == 1)))
    {/* 11BGN 1S AP*/
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN1S;
        } else {
            pTable = RateSwitchTable11N1SForABand;
        }

        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
#ifdef THERMAL_PROTECT_SUPPORT
            (pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
            (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (wlan_config_get_tx_stream(pEntry->wdev) == 2)))
    {/* 11BGN 2S AP*/
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11BGN2S;
        else
            pTable = RateSwitchTable11BGN2SForABand;

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (wlan_config_get_tx_stream(pEntry->wdev) == 3))
    {
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11N3S;
        else
            pTable = RateSwitchTable11N3SForABand;

        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (wlan_config_get_tx_stream(pEntry->wdev) == 1)
#ifdef THERMAL_PROTECT_SUPPORT
            || (pAd->force_one_tx_stream == TRUE)
#endif /* THERMAL_PROTECT_SUPPORT */
    ))
    {/* 11N 1S AP*/
        if (pAd->LatchRfRegs.Channel <= 14)
            pTable = RateSwitchTable11N1S;
        else
            pTable = RateSwitchTable11N1SForABand;

        return pTable;
    }

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (wlan_config_get_tx_stream(pEntry->wdev) == 2))
    {/* 11N 2S AP*/

        if (pAd->LatchRfRegs.Channel <= 14)
            pTable = RateSwitchTable11N2S;
        else
            pTable = RateSwitchTable11N2SForABand;			

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (wlan_config_get_tx_stream(pEntry->wdev) == 3))
    {
        if (pAd->LatchRfRegs.Channel <= 14)
            pTable = RateSwitchTable11N3S;
        else
            pTable = RateSwitchTable11N3SForABand;
        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
    if (wlan_config_get_tx_stream(pEntry->wdev) == 3)
    {
        if  (pEntry->HTCapability.MCSSet[0] != 0x00)
        {
            if (pEntry->HTCapability.MCSSet[1] == 0x00)
            {	/* Only support 1SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;	
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;
                }
                return pTable;
            }
            else if (pEntry->HTCapability.MCSSet[2] == 0x00)
            {	/* Only support 2SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11BGN2S;
                    else
                        pTable = RateSwitchTable11BGN2SForABand;
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N2S;
                    else
                        pTable = RateSwitchTable11N2SForABand;
                }
                return pTable;
            }
				/* For 3SS case, we use the new rate table, so don't care it here */
        }
    }
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

    if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || 
            WMODE_EQUAL(wdev->PhyMode, WMODE_B))
#ifdef DOT11_N_SUPPORT
            /*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
            /* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
    )
    {/* B only AP*/
        pTable = RateSwitchTable11B;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
            (pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* B/G  mixed AP*/
        pTable = RateSwitchTable11BG;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* G only AP*/
        pTable = RateSwitchTable11G;
        return pTable;
    }
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
#ifdef DOT11N_SS3_SUPPORT
        if (wlan_config_get_tx_stream(pEntry->wdev) >= 3)
        {
            if (pEntry->HTCapability.MCSSet[2] == 0)
                pTable = RateSwitchTable11N2S;
            else
                pTable = RateSwitchTable11N3S;
        }
        else
#endif /* DOT11N_SS3_SUPPORT */
        {
            /*
                Temp solution for:
                EX: when the extend rate only supports 6, 12, 24 in
                the association req frame. So the pEntry->RateLen is 7.
            */
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11BG;
            else
                pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

    return pTable;
}


VOID MlmeSelectTxRateTable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR **ppTable,
	IN UCHAR *pTableSize,
	IN UCHAR *pInitTxRateIdx)
{
    *ppTable = NULL;

	do
	{
#ifdef DOT11_VHT_AC

#ifdef NEW_RATE_ADAPT_SUPPORT
        if (pAd->rateAlg == RATE_ALG_GRP) {
            *ppTable = SelectVHTTxRateTableGRP(pAd, pEntry);
        }
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
        if (pAd->rateAlg == RATE_ALG_AGS) {
            *ppTable = SelectVHTTxRateTableAGS(pAd, pEntry);
        }
#endif /* AGS_SUPPORT */

        if (*ppTable) {
            break;
        }
#endif /* DOT11_VHT_AC */

#ifdef NEW_RATE_ADAPT_SUPPORT
        if (pAd->rateAlg == RATE_ALG_GRP) {
            *ppTable = SelectTxRateTableGRP(pAd, pEntry);
        }
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
        if (pAd->rateAlg == RATE_ALG_AGS) {
            *ppTable = SelectTxRateTableAGS(pAd, pEntry);
        }
#endif /* AGS_SUPPORT */

        if (*ppTable == NULL)
            *ppTable = SelectTxRateTable(pAd, pEntry);

	} while(FALSE);

    if ( *ppTable)
    {
        *pTableSize = RATE_TABLE_SIZE(*ppTable);
        *pInitTxRateIdx = RATE_TABLE_INIT_INDEX(*ppTable);
    }

}


/*
	MlmeSelectTxRate - select the MCS based on the RSSI and the available MCSs
		pAd - pointer to adapter
		pEntry - pointer to MAC table entry
		mcs - table of MCS index into the Rate Table. -1 => not supported
		Rssi - the Rssi value
		RssiOffset - offset to apply to the Rssi
*/
UCHAR MlmeSelectTxRate(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN CHAR		mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset)
{
	UCHAR TxRateIdx = 0;
	UCHAR *pTable = pEntry->pTable;

#ifdef DOT11_N_SUPPORT
#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef DOT11_VHT_AC
	if (pTable == RateTableVht2S || pTable == RateTableVht2S_BW20 || pTable == RateTableVht2S_BW40
		|| (pTable == RateTableVht2S_MCS7))
	{
		/*  VHT mode with 2SS */
		if (mcs[15]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportVHTMCS2SS& (1<<MCS_5)))
			TxRateIdx = mcs[15];
		else if (mcs[14]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportVHTMCS2SS& (1<<MCS_4)))
			TxRateIdx = mcs[14];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportVHTMCS2SS& (1<<MCS_3)))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportVHTMCS2SS& (1<<MCS_2)))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_4)))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_3)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_2)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_1)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else if (pTable == RateTableVht1S_MCS9)
	{	/*  VHT mode with 1SS */
		if (mcs[9]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_9)))
			TxRateIdx = mcs[8];
		else if (mcs[8]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_8)))
			TxRateIdx = mcs[8];
		else if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_7)))
			TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_6)))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_5)))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_4)))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_3)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_2)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_1)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else if (pTable == RateTableVht1S)
	{	/*  VHT mode with 1SS */
		if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_7)))
			TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_6)))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_5)))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_4)))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_3)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_2)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)) && (pEntry->SupportVHTMCS1SS& (1<<MCS_1)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else
#endif /* DOT11_VHT_AC */
#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef DOT11N_SS3_SUPPORT
	if ((pTable == RateSwitchTable11BGN3S) || (pTable == RateSwitchTable11N3S) || (pTable == RateSwitchTable11BGN3SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N3S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 3 stream */
		if (mcs[23]>=0 && (Rssi >= (-66+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_23)))
			TxRateIdx = mcs[23];
		else if (mcs[22]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_22)))
			TxRateIdx = mcs[22];
		else if (mcs[21]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_21)))
			TxRateIdx = mcs[21];
		else if (mcs[20]>=0 && (Rssi >= (-74+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_20)))
			TxRateIdx = mcs[20];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_13)))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_12)))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_4)))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_3)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_2)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_1)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else
#endif /*  DOT11N_SS3_SUPPORT */
	if ((pTable == RateSwitchTable11BGN2S) || (pTable == RateSwitchTable11BGN2SForABand) ||
		(pTable == RateSwitchTable11N2S) || (pTable == RateSwitchTable11N2SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N2S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 2 stream */
		if (mcs[15]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_15)))
			TxRateIdx = mcs[15];
		else if (mcs[14]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_14)))
			TxRateIdx = mcs[14];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_13)))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_12)))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_4)))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_3)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_2)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_1)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else if ((pTable == RateSwitchTable11BGN1S) ||
			 (pTable == RateSwitchTable11N1S) ||
			 (pTable == RateSwitchTable11N1SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
			|| (pTable == RateSwitchTableAdapt11N1S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 1 stream */
		{
			if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_7)))
				TxRateIdx = mcs[7];
			else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_6)))
				TxRateIdx = mcs[6];
			else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_5)))
				TxRateIdx = mcs[5];
			else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_4)))
				TxRateIdx = mcs[4];
			else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_3)))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_2)))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)) && (pEntry->SupportHTMCS & (1<<MCS_1)))
				TxRateIdx = mcs[1];
			else
				TxRateIdx = mcs[0];
		}
	}
	else
#endif /*  DOT11_N_SUPPORT */
	{/*  Legacy mode */
		if (mcs[7]>=0 && (Rssi > -70) && (pEntry->SupportOFDMMCS & (1<<MCS_7)))
		TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > -74) && (pEntry->SupportOFDMMCS & (1<<MCS_7)))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > -78) && (pEntry->SupportOFDMMCS & (1<<MCS_7)))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > -82) && (pEntry->SupportOFDMMCS & (1<<MCS_7)))
			TxRateIdx = mcs[4];
		else if (mcs[4] == -1)							/*  for B-only mode */
		{
			if (mcs[3]>=0 && (Rssi > -85) && (pEntry->SupportCCKMCS & (1<<MCS_3)))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > -87) && (pEntry->SupportCCKMCS & (1<<MCS_2)))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > -90) && (pEntry->SupportCCKMCS & (1<<MCS_1)))
				TxRateIdx = mcs[1];
			else if (pEntry->SupportCCKMCS & (1<<MCS_0))
				TxRateIdx = mcs[0];
			else
			TxRateIdx = mcs[3];
		}
		else if (mcs[3]>=0 && (Rssi > -85) && (pEntry->SupportOFDMMCS & (1<<MCS_3)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > -87) && (pEntry->SupportOFDMMCS & (1<<MCS_2)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > -90) && (pEntry->SupportOFDMMCS & (1<<MCS_1)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}

#ifdef BT_COEXISTENCE_SUPPORT
#ifdef DOT11_N_SUPPORT			
	McsDown2(pAd, mcs[3], mcs[4], mcs[5], mcs[6], &TxRateIdx);
#endif /* DOT11_N_SUPPORT */
#endif /* BT_COEXISTENCE_SUPPORT */

	return TxRateIdx;
}


/*  MlmeRAInit - Initialize Rate Adaptation for this entry */
VOID MlmeRAInit(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	MlmeSetMcsGroup(pAd, pEntry);

	//pEntry->lastRateIdx = 1;
	pEntry->lastRateIdx = 0xFF;
	pEntry->lowTrafficCount = 0;
	pEntry->perThrdAdj = PER_THRD_ADJ;
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef TXBF_SUPPORT
	pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
	pEntry->lastRatePhyTxBf = FALSE;
	pEntry->lastNonBfRate = 0;
#endif /* TXBF_SUPPORT */

	pEntry->fLastSecAccordingRSSI = FALSE;
	pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
	pEntry->CurrTxRateIndex = 0;
	pEntry->TxRateUpPenalty = 0;

	MlmeClearAllTxQuality(pEntry);
}


#ifdef DBG_CTRL_SUPPORT
/* #define TIMESTAMP_RA_LOG	*/ /* Include timestamp in RA Log */

/*
	MlmeRALog - Prints concise Rate Adaptation log entry
		The BF percentage counters are also updated
*/
VOID MlmeRALog(
	IN PRTMP_ADAPTER	pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN RA_LOG_TYPE		raLogType,
	IN ULONG			TxErrorRatio,
	IN ULONG			TxTotalCnt)
{
#ifdef TXBF_SUPPORT
	UINT ETxCount = pEntry->TxBFCounters.ETxSuccessCount + pEntry->TxBFCounters.ETxFailCount;
	UINT ITxCount = pEntry->TxBFCounters.ITxSuccessCount + pEntry->TxBFCounters.ITxFailCount;
	UINT TxCount = pEntry->TxBFCounters.TxSuccessCount + pEntry->TxBFCounters.TxFailCount + ETxCount + ITxCount;
	ULONG bfRatio = 0;
#endif /*  TXBF_SUPPORT */
#ifdef TIMESTAMP_RA_LOG
	ULONG newTime;
	static ULONG saveRATime;
	struct timeval tval;

	do_gettimeofday(&tval);
	newTime = (tval.tv_sec*1000000L + tval.tv_usec);
#endif

	if (TxTotalCnt !=0 || raLogType==RAL_QUICK_DRS
#ifdef DBG_CTRL_SUPPORT
		|| (pAd->CommonCfg.DebugFlags & DBF_SHOW_ZERO_RA_LOG)
#endif /* DBG_CTRL_SUPPORT */
	)
	{
		BOOLEAN stbc, csd=FALSE;
		ULONG tp;

		/*  Get STBC and StreamMode state */
		stbc = (pEntry->HTPhyMode.field.STBC && pEntry->HTPhyMode.field.MCS<8);

#ifdef STREAM_MODE_SUPPORT
		if (pEntry->StreamModeMACReg != 0)
		{
			ULONG streamWord;

			RTMP_IO_READ32(pAd, pEntry->StreamModeMACReg+4, &streamWord);
			if (pEntry->HTPhyMode.field.MCS < 8)
				csd = (streamWord & 0x30000)==0x30000;
			else if (pEntry->HTPhyMode.field.MCS < 16)
				csd = (streamWord & 0xC0000)==0xC0000;
		}
#endif /* STREAM_MODE_SUPPORT */

		/*  Normalized throughput - packets per RA Interval */
		if (raLogType==RAL_QUICK_DRS)
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*pAd->ra_fast_interval);
		else if (pEntry->LastSecTxRateChangeAction==RATE_NO_CHANGE
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
		)
			tp = (100-TxErrorRatio)*TxTotalCnt/100;
		else
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*(RA_INTERVAL-pAd->ra_fast_interval));

#ifdef TXBF_SUPPORT
		/*  Compute BF ratio in the last interval */
		if ((TxCount - pEntry->LastTxCount)>0)
		{
			if (pEntry->HTPhyMode.field.eTxBF)
				bfRatio = 100*(ETxCount-pEntry->LastETxCount)/(TxCount - pEntry->LastTxCount);
			else if (pEntry->HTPhyMode.field.iTxBF)
				bfRatio = 100*(ITxCount-pEntry->LastITxCount)/(TxCount - pEntry->LastTxCount);
		}

		if ((pEntry->HTPhyMode.field.eTxBF || pEntry->HTPhyMode.field.iTxBF)
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG)==0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s[%d]: M=%d %c%c%c%c%c PER=%ld%% TP=%ld BF=%ld%% ",
				raLogType==RAL_QUICK_DRS? " Q": (raLogType==RAL_NEW_DRS? "\nRA": "\nra"),
				pEntry->wcid, pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.MODE==MODE_CCK? 'C': (pEntry->HTPhyMode.field.ShortGI? 'S': 'L'),
				pEntry->HTPhyMode.field.BW? '4': '2',
				stbc? 'S': 's',
				csd? 'C': 'c',
				pEntry->HTPhyMode.field.eTxBF? 'E': (pEntry->HTPhyMode.field.iTxBF? 'I': '-'),
				TxErrorRatio, tp, bfRatio) );
		}
		else
#endif /* TXBF_SUPPORT */
#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG)
		{
			struct {
				USHORT phyMode;
				USHORT per;
				USHORT tp;
				USHORT bfRatio;
			} raLogInfo;

			raLogInfo.phyMode = pEntry->HTPhyMode.word;
			raLogInfo.per = TxErrorRatio;
			raLogInfo.tp = tp;
#ifdef TXBF_SUPPORT
			raLogInfo.bfRatio = bfRatio;
#endif /* TXBF_SUPPORT */
			dbQueueEnqueue(0x7e, (UCHAR *)&raLogInfo);
		}
		else
#endif /*  INCLUDE_DEBUG_QUEUE */
#endif /*  DBG_CTRL_SUPPORT */
		{
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s[%d]: M=%d %c%c%c%c- PER=%ld%% TP=%ld ",
				raLogType==RAL_QUICK_DRS? " Q": (raLogType==RAL_NEW_DRS? "\nRA": "\nra"),
				pEntry->wcid, pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.MODE==MODE_CCK? 'C': (pEntry->HTPhyMode.field.ShortGI? 'S': 'L'),
				pEntry->HTPhyMode.field.BW? '4': '2',
				stbc? 'S': 's',
				csd? 'C': 'c',
				TxErrorRatio, tp) );
		}
	}

#ifdef TXBF_SUPPORT
	/*  Remember previous counts */
	pEntry->LastETxCount = ETxCount;
	pEntry->LastITxCount = ITxCount;
	pEntry->LastTxCount = TxCount;
#endif /*  TXBF_SUPPORT */
#ifdef TIMESTAMP_RA_LOG
	saveRATime = newTime;
#endif
}
#endif /* DBG_CTRL_SUPPORT */


/*  MlmeRestoreLastRate - restore last saved rate */
VOID MlmeRestoreLastRate(
	IN PMAC_TABLE_ENTRY	pEntry)
{
	pEntry->CurrTxRateIndex = pEntry->lastRateIdx;
#ifdef TXBF_SUPPORT
	if (pEntry->eTxBfEnCond>0)
		pEntry->phyETxBf = pEntry->lastRatePhyTxBf;
	else
		pEntry->phyITxBf = pEntry->lastRatePhyTxBf;
#endif /*  TXBF_SUPPORT */
}


#ifdef DOT11N_SS3_SUPPORT
/*  MlmeCheckRDG - check if RDG should be enabled or disabled */
VOID MlmeCheckRDG(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	PUCHAR pTable = pEntry->pTable;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return;
	}

	/*  Turn off RDG when 3s and rx count > tx count*5 */
	if (((pTable == RateSwitchTable11BGN3S) || 
		(pTable == RateSwitchTable11BGN3SForABand) || 
		(pTable == RateSwitchTable11N3S)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N3S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
		) && pAd->RalinkCounters.OneSecReceivedByteCount > 50000 &&
		pAd->RalinkCounters.OneSecTransmittedByteCount > 50000 &&
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE))
	{
		TX_LINK_CFG_STRUC TxLinkCfg;
		UINT32 TxOpThres;
		UCHAR				TableStep;
		RTMP_RA_LEGACY_TB *pTempTxRate;

#ifdef NEW_RATE_ADAPT_SUPPORT
		TableStep = ADAPT_RATE_TABLE(pTable)? 10: 5;
#else
		TableStep = 5;
#endif

		pTempTxRate = (RTMP_RA_LEGACY_TB *)(&pTable[(pEntry->CurrTxRateIndex + 1)*TableStep]);
		RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
		if (pAd->RalinkCounters.OneSecReceivedByteCount > (pAd->RalinkCounters.OneSecTransmittedByteCount * 5) &&
				pTempTxRate->CurrMCS != 23 && pTempTxRate->ShortGI != 1)
		{
			if (TxLinkCfg.field.TxRDGEn == 1)
			{
				TxLinkCfg.field.TxRDGEn = 0;
				RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
				RTMP_IO_READ32(pAd, TXOP_THRES_CFG, &TxOpThres);
				TxOpThres |= 0xff00;
				RTMP_IO_WRITE32(pAd, TXOP_THRES_CFG, TxOpThres);
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_WARN,("DRS: RDG off!\n"));
			}
		}
		else
		{
			if (TxLinkCfg.field.TxRDGEn == 0)
			{
				TxLinkCfg.field.TxRDGEn = 1;
				RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
				RTMP_IO_READ32(pAd, TXOP_THRES_CFG, &TxOpThres);
				TxOpThres &= 0xffff00ff;
				RTMP_IO_WRITE32(pAd, TXOP_THRES_CFG, TxOpThres);
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_WARN,("DRS: RDG on!\n"));
			}
		}
	}
}
#endif /*  DOT11N_SS3_SUPPORT */


#ifdef TXBF_SUPPORT
VOID txbf_rate_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	RTMP_RA_LEGACY_TB *pNextTxRate;
	UCHAR *pTable = pEntry->pTable;


	/*  Get pointer to CurrTxRate entry */
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, pEntry->CurrTxRateIndex);
	else
#endif /*  NEW_RATE_ADAPT_SUPPORT */
		pNextTxRate = PTX_RA_LEGACY_ENTRY(pTable, pEntry->CurrTxRateIndex);

	/*  If BF has been disabled then force a non-BF rate */
	if (pEntry->eTxBfEnCond==0)
		pEntry->phyETxBf = 0;

	if (pEntry->iTxBfEn==0)
		pEntry->phyITxBf = 0;


   	/*  Set BF options */
	pEntry->HTPhyMode.field.eTxBF = pEntry->phyETxBf;
	pEntry->HTPhyMode.field.iTxBF = pEntry->phyITxBf;

	/*  Give ETxBF priority over ITxBF */
	if (pEntry->HTPhyMode.field.eTxBF)
		pEntry->HTPhyMode.field.iTxBF = 0;

	/*  In ITxBF mode force GI if we have no choice */
	if (pEntry->HTPhyMode.field.iTxBF &&
		(pEntry->OneSecRxLGICount + pEntry->OneSecRxSGICount) > 10)
	{
		if (pEntry->OneSecRxSGICount==0)
			pEntry->HTPhyMode.field.ShortGI = GI_800;

		if (pEntry->OneSecRxLGICount==0)
		{
			if ((pEntry->HTPhyMode.field.BW==BW_20 && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
		    	(pEntry->HTPhyMode.field.BW==BW_40 && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)))
					pEntry->HTPhyMode.field.ShortGI = GI_400;
		}
	}

	/*  Disable STBC if BF is enabled */
	if (pEntry->HTPhyMode.field.eTxBF || pEntry->HTPhyMode.field.iTxBF)
		pEntry->HTPhyMode.field.STBC = STBC_NONE;
}
#endif /* TXBF_SUPPORT */


INT rtmp_get_rate_from_rate_tb(UCHAR *table, INT idx, RTMP_TX_RATE *tx_rate)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(table)) {
		RTMP_RA_GRP_TB *rate_entry;

		rate_entry = PTX_RA_GRP_ENTRY(table, idx);
		tx_rate->mode = rate_entry->Mode;
		tx_rate->bw = rate_entry->BW;
		tx_rate->mcs = rate_entry->CurrMCS;
		tx_rate->sgi = rate_entry->ShortGI;
		tx_rate->stbc = rate_entry->STBC;
#ifdef DOT11_VHT_AC
		if (table == RateTableVht1S || table == RateTableVht2S || 
					table == RateTableVht2S_BW40 || 
					table == RateTableVht2S_BW20 || table == RateTableVht1S_MCS9
					|| (table == RateTableVht2S_MCS7))
			tx_rate->nss = rate_entry->dataRate;
		else
#endif /* DOT11_VHT_AC */
			tx_rate->nss = 0;
	}
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
	{
		RTMP_RA_LEGACY_TB *rate_entry;

		rate_entry = PTX_RA_LEGACY_ENTRY(table, idx);
		tx_rate->mode = rate_entry->Mode;
		tx_rate->bw = rate_entry->BW;
		tx_rate->mcs = rate_entry->CurrMCS;
		tx_rate->sgi = rate_entry->ShortGI;
		tx_rate->stbc = rate_entry->STBC;
		tx_rate->nss = 0;
	}

	return TRUE;
}


/*
	MlmeNewTxRate - called when a new TX rate was selected. Sets TX PHY to
		rate selected by pEntry->CurrTxRateIndex in pTable;
*/
VOID MlmeNewTxRate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	RTMP_RA_LEGACY_TB *pNextTxRate;
	UCHAR *pTable;

	if ((pEntry == NULL) || (pEntry->pTable == NULL))
		return;
	else
		pTable = pEntry->pTable;

	/*  Get pointer to CurrTxRate entry */
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, pEntry->CurrTxRateIndex);
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		pNextTxRate = PTX_RA_LEGACY_ENTRY(pTable, pEntry->CurrTxRateIndex);

#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef WAPI_SUPPORT
    if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd) || IS_MT7637(pAd))
    {
        if ((pEntry->AuthMode == Ndis802_11AuthModeWAICERT) || (pEntry->AuthMode == Ndis802_11AuthModeWAIPSK))
        {
            if (pTable == RateSwitchTableAdapt11N2S)
            {
                if ((pEntry->CurrTxRateIndex >= 14) && (pEntry->CurrTxRateIndex <= 16))
                {
                    pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, 13);
                }
            }
        }
    }
#endif /* WAPI_SUPPORT */
#endif /* NEW_RATE_ADAPT_SUPPORT */

	/*  Set new rate */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		APMlmeSetTxRate(pAd, pEntry, pNextTxRate);
	}
#endif /*  CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/*  Disable invalid HT Duplicate modes to prevent PHY error */
	if (pEntry->HTPhyMode.field.MCS==32)
	{
		if ((pEntry->HTPhyMode.field.BW!=BW_40) && (pEntry->HTPhyMode.field.BW!=BW_80))
			pEntry->HTPhyMode.field.MCS = 0;
		else
			pEntry->HTPhyMode.field.STBC = 0;
	}
#endif /*  DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgHwTxBfCap)
		txbf_rate_adjust(pAd, pEntry);
#endif /*  TXBF_SUPPORT */

	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);

#ifdef STREAM_MODE_SUPPORT
	/*  Enable/disable stream mode based on MCS */
	if (pAd->CommonCfg.StreamMode!=0 &&
		pEntry->StreamModeMACReg!=0)
	{
		UINT streamWord;
		BOOLEAN mcsDisable;

		/* OFDM: depends on StreamModeMCS, CCK: always applies stream-mode */
		mcsDisable = (pEntry->HTPhyMode.field.MCS < 16) &&
				(pAd->CommonCfg.StreamModeMCS & (1<<pEntry->HTPhyMode.field.MCS))==0 &&
				(pEntry->HTPhyMode.field.MODE != MODE_CCK);

		streamWord = mcsDisable ? 0 : StreamModeRegVal(pAd);

		/*  Update Stream Mode control reg */
		RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, streamWord | (ULONG)(pEntry->Addr[4]) | (ULONG)(pEntry->Addr[5] << 8));
	}
#endif /* STREAM_MODE_SUPPORT */
}

#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

