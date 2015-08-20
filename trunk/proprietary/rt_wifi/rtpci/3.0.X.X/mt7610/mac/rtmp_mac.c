/*

*/


#include "rt_config.h"


/*
	========================================================================
	
	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.
					  
	Arguments:
		pTxWI		Pointer to head of each MPDU to HW.
		Ack 		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs 		Setting for IFS gap
		Rate		Setting for transmit rate
		Service 	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003
		
	Return Value:
		None
	
	See also : BASmartHardTransmit()    !!!
	
	========================================================================
*/
VOID RTMPWriteTxWI(
	IN RTMP_ADAPTER *pAd,
	IN TXWI_STRUC *pOutTxWI,
	IN BOOLEAN FRAG,
	IN BOOLEAN CFACK,
	IN BOOLEAN InsTimestamp,
	IN BOOLEAN AMPDU,
	IN BOOLEAN Ack,
	IN BOOLEAN NSeq,		/* HW new a sequence.*/
	IN UCHAR BASize,
	IN UCHAR WCID,
	IN ULONG Length,
	IN UCHAR PID,
	IN UCHAR TID,
	IN UCHAR TxRate,
	IN UCHAR Txopmode,
	IN BOOLEAN CfAck,
	IN HTTRANSMIT_SETTING *pTransmit)
{
	PMAC_TABLE_ENTRY pMac = NULL;
	TXWI_STRUC TxWI, *pTxWI;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT32 MaxWcidNum = MAX_LEN_OF_MAC_TABLE;

#ifdef MAC_REPEATER_SUPPORT	
	MaxWcidNum = MAX_MAC_TABLE_SIZE_WITH_REPEATER;
#endif /* MAC_REPEATER_SUPPORT */

	if (WCID < MaxWcidNum)
		pMac = &pAd->MacTab.Content[WCID];

	
	/* 
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWISize);
	pTxWI = &TxWI;
	pTxWI->TxWIFRAG= FRAG;
	pTxWI->TxWICFACK = CFACK;
	pTxWI->TxWITS= InsTimestamp;
	pTxWI->TxWIAMPDU = AMPDU;
	pTxWI->TxWIACK = Ack;
	pTxWI->TxWITXOP= Txopmode;
	
	pTxWI->TxWINSEQ = NSeq;
	/* John tune the performace with Intel Client in 20 MHz performance*/
#ifdef DOT11_N_SUPPORT
	BASize = pAd->CommonCfg.TxBASize;
#ifdef RT65xx
	if (IS_RT65XX(pAd))
	{
		if (BASize > 31)
			BASize =31;
	}
	else
#endif /* RT65xx */
	if (pAd->MACVersion == 0x28720200)
	{
		if (BASize > 13)
			BASize =13;
	}
	else
	{
		if( BASize >7 )
			BASize =7;
	}

	pTxWI->TxWIBAWinSize = BASize;
	pTxWI->TxWIShortGI = pTransmit->field.ShortGI;
	pTxWI->TxWISTBC = pTransmit->field.STBC;

#ifdef TXBF_SUPPORT
	if (pMac && pAd->chipCap.FlgHwTxBfCap)
	{
		if (pMac->TxSndgType == SNDG_TYPE_NDP  || pMac->TxSndgType == SNDG_TYPE_SOUNDING || pTxWI->eTxBF)
			pTxWI->TxWISTBC = 0;
	}
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */
		
	pTxWI->TxWIWirelessCliID = WCID;
	pTxWI->TxWIMPDUByteCnt = Length;
	pTxWI->TxWIPacketId = PID;
	
	/* If CCK or OFDM, BW must be 20*/
	pTxWI->TxWIBW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pTxWI->TxWIBW)
		pTxWI->TxWIBW = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	
	pTxWI->TxWIMCS = pTransmit->field.MCS;
	pTxWI->TxWIPHYMODE = pTransmit->field.MODE;
	pTxWI->TxWICFACK = CfAck;

#ifdef DOT11_N_SUPPORT
	if (pMac)
	{
        if (pAd->CommonCfg.bMIMOPSEnable)
        {
		UCHAR MaxMcs_1ss;
	
		/* 10.2.4 SM power save, IEEE 802.11/2012, p.1010
		A STA in static SM power save mode maintains only a single receive chain active.
	
		An HT STA may use the SM Power Save frame to communicate its SM Power Save state. A non-AP HT
		STA may also use SM Power Save bits in the HT Capabilities element of its Association Request to achieve
		the same purpose. The latter allows the STA to use only a single receive chain immediately after association.
		*/
#ifdef DOT11_VHT_AC
		if (IS_VHT_STA(pMac))
			MaxMcs_1ss = 9;
		else
#endif /* DOT11_VHT_AC */
			MaxMcs_1ss = 7; 

    		if ((pMac->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > MaxMcs_1ss))
			{
				/* Dynamic MIMO Power Save Mode*/
				pTxWI->TxWIMIMOps = 1;
			}
			else if (pMac->MmpsMode == MMPS_STATIC)
			{
				/* Static MIMO Power Save Mode*/
				if (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > MaxMcs_1ss)
				{
					pTxWI->TxWIMCS = MaxMcs_1ss;
					pTxWI->TxWIMIMOps = 0;
				}
			}
        }
		/*pTxWI->TxWIMIMOps = (pMac->PsMode == PWR_MMPS)? 1:0;*/
		{
			pTxWI->TxWIMpduDensity = pMac->MpduDensity;
		}
	}
#endif /* DOT11_N_SUPPORT */


	pTxWI->TxWIPacketId = pTxWI->TxWIMCS;
	NdisMoveMemory(pOutTxWI, &TxWI, TXWISize);

#ifdef DBG
//+++Add by shiang for debug
if (0){
	hex_dump("TxWI", (UCHAR *)pOutTxWI, TXWISize);
}
//---Add by shiang for debug
#endif
}


VOID RTMPWriteTxWI_Data(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI, TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit;
	MAC_TABLE_ENTRY *pMacEntry;
#ifdef DOT11_N_SUPPORT
	UCHAR BASize;
#endif /* DOT11_N_SUPPORT */
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	ASSERT(pTxWI);

	pTransmit = pTxBlk->pTransmit;
	pMacEntry = pTxBlk->pMacEntry;

	/*
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(pTxWI, TXWISize);
	
	pTxWI->TxWIFRAG = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag);
	pTxWI->TxWIACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
	pTxWI->TxWITXOP = pTxBlk->FrameGap;

	pTxWI->TxWIWirelessCliID = pTxBlk->Wcid;

	pTxWI->TxWIMPDUByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
	pTxWI->TxWICFACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bPiggyBack);

#ifdef WFA_VHT_PF
	if (pAd->force_noack == TRUE)
		pTxWI->TxWIACK = 0;
#endif /* WFA_VHT_PF */

	pTxWI->TxWIShortGI = pTransmit->field.ShortGI;
	pTxWI->TxWISTBC = pTransmit->field.STBC;
	pTxWI->TxWIMCS = pTransmit->field.MCS;
	pTxWI->TxWIPHYMODE = pTransmit->field.MODE;

	/* If CCK or OFDM, BW must be 20 */
	pTxWI->TxWIBW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pTxWI->TxWIBW)
		pTxWI->TxWIBW = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */

	pTxWI->TxWIAMPDU = ((pTxBlk->TxFrameType == TX_AMPDU_FRAME) ? TRUE : FALSE);
	BASize = pAd->CommonCfg.TxBASize;
	if((pTxBlk->TxFrameType == TX_AMPDU_FRAME) && (pMacEntry))
	{
		UCHAR RABAOriIdx = pTxBlk->pMacEntry->BAOriWcidArray[pTxBlk->UserPriority];

		BASize = pAd->BATable.BAOriEntry[RABAOriIdx].BAWinSize;
	}

	pTxWI->TxWIBAWinSize = BASize;

#ifdef TXBF_SUPPORT
	if(pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
		pTxWI->TxWIAMPDU = FALSE;

	if (pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING)
	{
		pTxWI->Sounding = 1;
		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in RTMPWriteTxWI_Data(): sending normal sounding, eTxBF=%d\n", pTxWI->eTxBF));
		pTxWI->iTxBF = 0;
	}
	else if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP)
	{
		if (pTxBlk->TxNDPSndgMcs >= 16)
			pTxWI->NDPSndRate = 2;
		else if (pTxBlk->TxNDPSndgMcs >= 8)
			pTxWI->NDPSndRate = 1;
		else
			pTxWI->NDPSndRate = 0;

		pTxWI->NDPSndBW = pTransmit->field.BW;
		pTxWI->iTxBF = 0;
	}
	else
	{
#ifdef MFB_SUPPORT
		if (pMacEntry && (pMacEntry->mrqCnt >0) && (pMacEntry->toTxMrq == TRUE))
			pTxWI->eTxBF = ~(pTransmit->field.eTxBF);
		else
#endif	/* MFB_SUPPORT */
			pTxWI->eTxBF = pTransmit->field.eTxBF;
		pTxWI->iTxBF = pTransmit->field.iTxBF;
	}

	if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP  || pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING || pTxWI->eTxBF)
		pTxWI->TxWISTBC = 0;
#endif /* TXBF_SUPPORT */

#endif /* DOT11_N_SUPPORT */
	

#ifdef DOT11_N_SUPPORT
	if (pMacEntry)
	{
		UCHAR MaxMcs_1ss;

		/* 10.2.4 SM power save, IEEE 802.11/2012, p.1010
		A STA in static SM power save mode maintains only a single receive chain active.

		An HT STA may use the SM Power Save frame to communicate its SM Power Save state. A non-AP HT
		STA may also use SM Power Save bits in the HT Capabilities element of its Association Request to achieve
		the same purpose. The latter allows the STA to use only a single receive chain immediately after association.
		*/
#ifdef DOT11_VHT_AC
		if (IS_VHT_STA(pMacEntry))
			MaxMcs_1ss = 9;
		else
#endif /* DOT11_VHT_AC */
			MaxMcs_1ss = 7; 

		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > MaxMcs_1ss))
		{
			/* Dynamic MIMO Power Save Mode*/
			pTxWI->TxWIMIMOps = 1;
		}
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			/* Static MIMO Power Save Mode*/
			if ((pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) && 
				(pTransmit->field.MCS > MaxMcs_1ss))
			{
				pTxWI->TxWIMCS = MaxMcs_1ss;
				pTxWI->TxWIMIMOps = 0;
			}
		}

		pTxWI->TxWIMpduDensity = pMacEntry->MpduDensity;
	}
#endif /* DOT11_N_SUPPORT */
	
#ifdef TXBF_SUPPORT
	if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
	{
		pTxWI->TxWIMCS = 0;
		pTxWI->TxWIAMPDU = FALSE;
	}
#endif /* TXBF_SUPPORT */
	
#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
#ifdef DOT11_VHT_AC
		if (pTxWI->TxWIPHYMODE == MODE_VHT) {
			pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_VHT[pTxWI->TxWIMCS]++;
			if (pTxWI->TxWIShortGI)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxSGICnt_VHT[pTxWI->TxWIMCS]++;
		}
		else
#endif /* DOT11_VHT_AC */
			pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[pTxWI->TxWIMCS]++;
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

	/* for rate adapation*/
	pTxWI->TxWIPacketId = pTxWI->TxWIMCS;

#ifdef INF_AMAZON_SE
	/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
	if( RTMP_GET_PACKET_NOBULKOUT(pTxBlk->pPacket))
	{
		if(pTxWI->TxWIPHYMODE == MODE_CCK)
			pTxWI->TxWIPacketId = 6;
	}	
#endif /* INF_AMAZON_SE */	


#ifdef FPGA_MODE
	if (pAd->fpga_on & 0x6)
	{
		pTxWI->TxWIPHYMODE = pAd->data_phy;
		pTxWI->TxWIMCS = pAd->data_mcs;
		pTxWI->TxWIBW = pAd->data_bw;
		pTxWI->TxWIShortGI = pAd->data_gi;
		if (pAd->data_basize)
			pTxWI->TxWIBAWinSize = pAd->data_basize;
	}
#endif /* FPGA_MODE */

#ifdef MCS_LUT_SUPPORT
	if ((RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT)) && 
		(pTxWI->TxWIWirelessCliID < 128) && 
		(pMacEntry && pMacEntry->bAutoTxRateSwitch == TRUE))
	{
		HTTRANSMIT_SETTING rate_ctrl;

		rate_ctrl.field.MODE = pTxWI->TxWIPHYMODE;
#ifdef TXBF_SUPPORT
		rate_ctrl.field.iTxBF = pTxWI->iTxBF;
		rate_ctrl.field.eTxBF = pTxWI->eTxBF;
#endif /* TXBF_SUPPORT */
		rate_ctrl.field.STBC = pTxWI->TxWISTBC;
		rate_ctrl.field.ShortGI = pTxWI->TxWIShortGI;
		rate_ctrl.field.BW = pTxWI->TxWIBW;
		rate_ctrl.field.MCS = pTxWI->TxWIMCS; 
		if (rate_ctrl.word == pTransmit->word)
			pTxWI->TxWILutEn = 1;
		pTxWI->TxWILutEn = 0;
	}

#ifdef PEER_DELBA_TX_ADAPT
	if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket) ||
		(pTxBlk->TxFrameType == TX_MCAST_FRAME) ||
		(pMacEntry && (pMacEntry->MmpsMode == MMPS_STATIC)))
	    pTxWI->TxWILutEn = 0;
	else
	    pTxWI->TxWILutEn = 1;
#endif /* PEER_DELBA_TX_ADAPT */
#endif /* MCS_LUT_SUPPORT */

#ifdef DOT11_VHT_AC
	if (pTxWI->TxWIPHYMODE == MODE_VHT)
	{
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("TxPkt with MODE:VHT, Nss:%d, MCS:%d,BW:%d,basize:%d\n",
										(pTxWI->TxWIMCS >> 4), (pTxWI->TxWIMCS & 0xf),
										pTxWI->TxWIBW, pTxWI->TxWIBAWinSize));
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("TxWIPHYMODE:%d, MCS:%d,BW:%d,basize:%d\n",
										pTxWI->TxWIPHYMODE, pTxWI->TxWIMCS,
										pTxWI->TxWIBW, pTxWI->TxWIBAWinSize));
	}
#endif /* DOT11_VHT_AC */
}


VOID RTMPWriteTxWI_Cache(
	IN RTMP_ADAPTER *pAd,
	INOUT TXWI_STRUC *pTxWI,
	IN TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit;
	MAC_TABLE_ENTRY *pMacEntry;
#ifdef DOT11_N_SUPPORT
#endif /* DOT11_N_SUPPORT */
	
	
	/* update TXWI */
	pMacEntry = pTxBlk->pMacEntry;
	pTransmit = pTxBlk->pTransmit;
	
	if (pMacEntry->bAutoTxRateSwitch)
	{
		pTxWI->TxWITXOP = IFS_HTTXOP;

		/* If CCK or OFDM, BW must be 20*/
		pTxWI->TxWIBW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
		pTxWI->TxWIShortGI = pTransmit->field.ShortGI;
		pTxWI->TxWISTBC = pTransmit->field.STBC;

#ifdef TXBF_SUPPORT
		if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP  || pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING || pTxWI->eTxBF)
			pTxWI->TxWISTBC = 0;
#endif /* TXBF_SUPPORT */

		pTxWI->TxWIMCS = pTransmit->field.MCS;
		pTxWI->TxWIPHYMODE = pTransmit->field.MODE;

		/* set PID for TxRateSwitching*/
		pTxWI->TxWIPacketId = pTransmit->field.MCS;
		
	}

#ifdef DOT11_N_SUPPORT
	pTxWI->TxWIAMPDU = ((pMacEntry->NoBADataCountDown == 0) ? TRUE: FALSE);
#ifdef TXBF_SUPPORT
	if(pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
		pTxWI->TxWIAMPDU = FALSE;
#endif /* TXBF_SUPPORT */

	pTxWI->TxWIMIMOps = 0;

#ifdef DOT11N_DRAFT3
	if (pTxWI->TxWIBW)
		pTxWI->TxWIBW = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */

    if (pAd->CommonCfg.bMIMOPSEnable)
    {
		UCHAR MaxMcs_1ss;

		/* 10.2.4 SM power save, IEEE 802.11/2012, p.1010
		A STA in static SM power save mode maintains only a single receive chain active.

		An HT STA may use the SM Power Save frame to communicate its SM Power Save state. A non-AP HT
		STA may also use SM Power Save bits in the HT Capabilities element of its Association Request to achieve
		the same purpose. The latter allows the STA to use only a single receive chain immediately after association.
		*/
#ifdef DOT11_VHT_AC
		if (IS_VHT_STA(pMacEntry))
			MaxMcs_1ss = 9;
		else
#endif /* DOT11_VHT_AC */
			MaxMcs_1ss = 7;

		/* MIMO Power Save Mode*/
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > MaxMcs_1ss))
		{
			/* Dynamic MIMO Power Save Mode*/
			pTxWI->TxWIMIMOps = 1;
		}
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			/* Static MIMO Power Save Mode*/
			if ((pTransmit->field.MODE >= MODE_HTMIX) && (pTransmit->field.MCS > MaxMcs_1ss))
			{
				pTxWI->TxWIMCS = MaxMcs_1ss;
				pTxWI->TxWIMIMOps = 0;
			}
		}
    }

#endif /* DOT11_N_SUPPORT */

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
#ifdef DOT11_VHT_AC
		if (pTxWI->TxWIPHYMODE == MODE_VHT) {
			pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_VHT[pTxWI->TxWIMCS]++;
			if (pTxWI->TxWIShortGI)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxSGICnt_VHT[pTxWI->TxWIMCS]++;
		}
		else
#endif /* DOT11_VHT_AC */
			pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[pTxWI->TxWIMCS]++;
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

#ifdef TXBF_SUPPORT
	if (pTxBlk->TxSndgPkt == SNDG_TYPE_SOUNDING)
	{
		pTxWI->Sounding = 1;
		pTxWI->eTxBF = 0;
		pTxWI->iTxBF = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("ETxBF in RTMPWriteTxWI_Cache(): sending normal sounding, eTxBF=%d\n", pTxWI->eTxBF));
	}
	else if (pTxBlk->TxSndgPkt == SNDG_TYPE_NDP)
	{
		if (pTxBlk->TxNDPSndgMcs>=16)
			pTxWI->NDPSndRate = 2;
		else if (pTxBlk->TxNDPSndgMcs>=8)
			pTxWI->NDPSndRate = 1;
		else
			pTxWI->NDPSndRate = 0;
		pTxWI->Sounding = 0;
		pTxWI->eTxBF = 0;
		pTxWI->iTxBF = 0;

		pTxWI->NDPSndBW = pTransmit->field.BW;

/*
		DBGPRINT(RT_DEBUG_TRACE,
				("%s():ETxBF, sending ndp sounding(BW=%d, Rate=%d, eTxBF=%d)\n",
				__FUNCTION__, pTxWI->NDPSndBW, pTxWI->NDPSndRate, pTxWI->eTxBF));
*/
	}
	else
	{
		pTxWI->Sounding = 0;
#ifdef MFB_SUPPORT
		if (pMacEntry && pMacEntry->mrqCnt >0 && pMacEntry->toTxMrq == 1)
		{
			pTxWI->eTxBF = ~(pTransmit->field.eTxBF);
			DBGPRINT_RAW(RT_DEBUG_TRACE,("ETxBF in AP_AMPDU_Frame_Tx(): invert eTxBF\n"));
		}
		else
#endif	/* MFB_SUPPORT */
			pTxWI->eTxBF = pTransmit->field.eTxBF;

		pTxWI->iTxBF = pTransmit->field.iTxBF;

		if (pTxWI->eTxBF || pTxWI->iTxBF)
			pTxWI->TxWISTBC = 0;
	}

	if (pTxBlk->TxSndgPkt > SNDG_TYPE_DISABLE)
	{
		pTxWI->TxWIMCS = 0;
		pTxWI->TxWIAMPDU = FALSE;
	}
#endif /* TXBF_SUPPORT */

	pTxWI->TxWIMPDUByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;


#ifdef WFA_VHT_PF
	if (pAd->force_noack == TRUE)
		pTxWI->TxWIACK = 0;
	else
#endif /* WFA_VHT_PF */
		pTxWI->TxWIACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);

#ifdef FPGA_MODE
	if (pAd->fpga_on & 0x6)
	{
		pTxWI->TxWIPHYMODE = pAd->data_phy;
		pTxWI->TxWIMCS = pAd->data_mcs;
		pTxWI->TxWIBW = pAd->data_bw;
		pTxWI->TxWIShortGI = pAd->data_gi;
		if (pAd->data_basize)
			pTxWI->TxWIBAWinSize = pAd->data_basize;
	}
#endif /* FPGA_MODE */

#ifdef MCS_LUT_SUPPORT
	if (RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) && 
		(pTxWI->TxWIWirelessCliID < 128) && 
		(pMacEntry && pMacEntry->bAutoTxRateSwitch == TRUE))
	{
		HTTRANSMIT_SETTING rate_ctrl;
		
		rate_ctrl.field.MODE = pTxWI->TxWIPHYMODE;
#ifdef TXBF_SUPPORT
		rate_ctrl.field.iTxBF = pTxWI->iTxBF;
		rate_ctrl.field.eTxBF = pTxWI->eTxBF;
#endif /* TXBF_SUPPORT */
		rate_ctrl.field.STBC = pTxWI->TxWISTBC;
		rate_ctrl.field.ShortGI = pTxWI->TxWIShortGI;
		rate_ctrl.field.BW = pTxWI->TxWIBW;
		rate_ctrl.field.MCS = pTxWI->TxWIMCS; 
		if (rate_ctrl.word == pTransmit->word)
			pTxWI->TxWILutEn = 1;
		pTxWI->TxWILutEn = 0;
	}

#ifdef PEER_DELBA_TX_ADAPT
	if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket) || 
		(pTxBlk->TxFrameType == TX_MCAST_FRAME) ||
		(pMacEntry && (pMacEntry->MmpsMode == MMPS_STATIC)))
		pTxWI->TxWILutEn = 0;
	else
		pTxWI->TxWILutEn = 1;
#endif /* PEER_DELBA_TX_ADAPT */
#endif /* MCS_LUT_SUPPORT */

#ifdef DOT11_VHT_AC
	if (pTxWI->TxWIPHYMODE == MODE_VHT)
	{
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("TxPkt with MODE:VHT, Nss:%d, MCS:%d,BW:%d,basize:%d\n",
										(pTxWI->TxWIMCS >> 4), (pTxWI->TxWIMCS & 0xf),
										pTxWI->TxWIBW, pTxWI->TxWIBAWinSize));
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("TxWIPHYMODE:%d, MCS:%d,BW:%d,basize:%d\n",
										pTxWI->TxWIPHYMODE, pTxWI->TxWIMCS,
										pTxWI->TxWIBW, pTxWI->TxWIBAWinSize));
	}
#endif /* DOT11_VHT_AC */
}


INT rtmp_mac_set_band(RTMP_ADAPTER *pAd, int  band)
{
	UINT32 val, band_cfg;


	RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
	val = band_cfg & (~0x6);
	switch (band)
	{
		case BAND_5G:
			val |= 0x02;
			break;
		case BAND_24G:
		default:
			val |= 0x4;
			break;
	}

	if (val != band_cfg)
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, val);

	return TRUE;
}


INT rtmp_mac_set_ctrlch(RTMP_ADAPTER *pAd, INT extch)
{
	UINT32 val, band_cfg;


	RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
	val = band_cfg & (~0x1);
	switch (extch)
	{
		case EXTCHA_ABOVE:
			val &= (~0x1);
			break;
		case EXTCHA_BELOW:
			val |= (0x1);
			break;
		case EXTCHA_NONE:
			val &= (~0x1);
			break;
	}

	if (val != band_cfg)
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, val);
	
	return TRUE;
}


INT rtmp_mac_set_mmps(RTMP_ADAPTER *pAd, INT ReduceCorePower)
{
	UINT32 mac_val, org_val;

	RTMP_IO_READ32(pAd, 0x1210, &org_val);
	mac_val = org_val;
	if (ReduceCorePower)
		mac_val |= 0x09;
	else
		mac_val &= ~0x09;

	if (mac_val != org_val)
		RTMP_IO_WRITE32(pAd, 0x1210, mac_val);

	return TRUE;
}

