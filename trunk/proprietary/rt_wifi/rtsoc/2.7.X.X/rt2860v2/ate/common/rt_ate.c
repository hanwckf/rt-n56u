/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 
 	Module Name:
	rt_ate.c

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/
#include "rt_config.h"
 
#define ATE_BBP_REG_NUM	168
UCHAR restore_BBP[ATE_BBP_REG_NUM]={0};

/* 802.11 MAC Header, Type:Data, Length:24bytes + 6 bytes QOS/HTC + 2 bytes padding */
UCHAR TemplateFrame[32] = {0x08,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x00,0xAA,0xBB,0x12,0x34,0x56,0x00,0x11,0x22,0xAA,0xBB,0xCC,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

extern FREQUENCY_ITEM *FreqItems3020;
extern UCHAR NUM_OF_3020_CHNL;

#define TXCONT_TX_PIN_CFG_A 0x040C0050
#define TXCONT_TX_PIN_CFG_G 0x080C00A0

#define ATE_TASK_EXEC_INTV 100
#define ATE_TASK_EXEC_MULTIPLE 10

static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1}; /* CCK Mode. */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1}; /* OFDM Mode. */
#ifdef DOT11N_SS3_SUPPORT
static CHAR HTMIXRateTable3T3R[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1}; /* HT Mix Mode for 3*3. */
#endif /* DOT11N_SS3_SUPPORT */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 32, -1}; /* HT Mix Mode. */

#ifdef RTMP_INTERNAL_TX_ALC

/* The desired TSSI over CCK */
extern CHAR desiredTSSIOverCCK[4];

/* The desired TSSI over OFDM */
extern CHAR desiredTSSIOverOFDM[8];

#ifdef RT3352
/* The desired TSSI over HT */
extern CHAR desiredTSSIOverHT[16];
#else
/* The desired TSSI over HT */
extern CHAR desiredTSSIOverHT[8];
#endif /* RT3352 */

/* The desired TSSI over HT using STBC */
extern CHAR desiredTSSIOverHTUsingSTBC[8];

/* The Tx power tuning entry*/
extern TX_POWER_TUNING_ENTRY_STRUCT TxPowerTuningTable[];

/*
==========================================================================
	Description:
		Get the desired TSSI based on ATE setting.

	Arguments:
		pAd

	Return Value:
		The desired TSSI
==========================================================================
 */
CHAR ATEGetDesiredTSSI(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR desiredTSSI = 0;
	UCHAR MCS = 0;
	UCHAR MaxMCS = 7;

	MCS = (UCHAR)(pATEInfo->TxWI.MCS);
	
	if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
	{
		if (MCS > 3) /* boundary verification */
		{
			DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));
			
			MCS = 0;
		}
	
		desiredTSSI = desiredTSSIOverCCK[MCS];
	}
	else if (pATEInfo->TxWI.PHYMODE == MODE_OFDM)
	{
		if (MCS > 7) /* boundary verification */
		{
			DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));

			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverOFDM[MCS];
	}
	else if ((pATEInfo->TxWI.PHYMODE == MODE_HTMIX) || (pATEInfo->TxWI.PHYMODE == MODE_HTGREENFIELD))
	{
		if (pATEInfo->TxWI.STBC == STBC_NONE)
		{
#ifdef RT3352
			if (IS_RT3352(pAd))
				MaxMCS = 15;
#endif /* RT3352 */
			if (MCS > MaxMCS) /* boundary verification */
			{
				DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n", 
					__FUNCTION__, 
					MCS));

				MCS = 0;
			}

			desiredTSSI = desiredTSSIOverHT[MCS];
		}
		else
		{
			if (MCS > MaxMCS) /* boundary verification */
			{
				DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n", 
					__FUNCTION__, 
					MCS));

				MCS = 0;
			}

			desiredTSSI = desiredTSSIOverHTUsingSTBC[MCS];
		}

		
		/* 
			For HT BW40 MCS 7 with/without STBC configuration, 
			the desired TSSI value should subtract one from the formula.
		*/
		if ((pATEInfo->TxWI.BW == BW_40) && (MCS == MCS_7))
		{
			desiredTSSI -= 1;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, Latest Tx setting: MODE = %d, MCS = %d, STBC = %d\n", 
		__FUNCTION__, 
		desiredTSSI, 
		pATEInfo->TxWI.PHYMODE, 
		pATEInfo->TxWI.MCS, 
		pATEInfo->TxWI.STBC));


	return desiredTSSI;
}


#ifdef RT5350
extern UINT32 RT5350_GetDesiredTSSI(
	IN PRTMP_ADAPTER		pAd,
	OUT PUCHAR				pBbpR49);
#endif /* RT5350 */
#endif /* RTMP_INTERNAL_TX_ALC */


/*
==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in ATE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. TxPowerPercentage
		1. auto calibration based on TSSI feedback
		2. extra 2 db for CCK
		3. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

==========================================================================
*/
VOID DefaultATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO   pATEInfo = &(pAd->ate);
	PATE_CHIP_STRUCT pChipStruct = pATEInfo->pChipStruct;
	INT			index = 0, inner_index = 0, maxTxPwrCnt;
	CHAR		DeltaPwr = 0;
	BOOLEAN		bAutoTxAgc = FALSE;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep;
	UCHAR		BbpR49 = 0, idx;
	PCHAR		pTxAgcCompensate;
	ULONG		TxPwr[9];	/* NOTE: the TxPwr array size should be the maxima value of all supported chipset!!!! */
	CHAR		Value;
#ifdef RTMP_INTERNAL_TX_ALC
	/* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */    
	CHAR TotalDeltaPower = 0; 
	UCHAR desiredTSSI = 0, currentTSSI = 0;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable = pAd->chipCap.TxPowerTuningTable_2G;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	UCHAR RFValue = 0, TmpValue = 0;   
#endif /* RTMP_INTERNAL_TX_ALC */

	maxTxPwrCnt = pChipStruct->maxTxPwrCnt;

	if (pATEInfo->TxWI.BW == BW_40)
	{
		if (pATEInfo->Channel > 14)
		{
			for (index =0 ; index < maxTxPwrCnt; index ++)
			{
				TxPwr[index] = pAd->Tx40MPwrCfgABand[index];	
			}
		}
		else
		{
			for (index =0 ; index < maxTxPwrCnt; index ++)
			{
				TxPwr[index] = pAd->Tx40MPwrCfgGBand[index];	
			}
		}
	}
	else
	{
		if (pATEInfo->Channel > 14)
		{
			for (index =0 ; index < maxTxPwrCnt; index ++)
			{
				TxPwr[index] = pAd->Tx20MPwrCfgABand[index];	
			}
		}
		else
		{
			for (index =0 ; index < maxTxPwrCnt; index ++)
			{
				TxPwr[index] = pAd->Tx20MPwrCfgGBand[index];	
			}
		}
	}

#ifdef RTMP_INTERNAL_TX_ALC
	/* Locate the internal Tx ALC tuning entry */
	if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
	{
		{
			desiredTSSI = ATEGetDesiredTSSI(pAd);

			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);

			currentTSSI = BbpR49 & 0x1F;

			if (pAd->TxPowerCtrl.bExtendedTssiMode == TRUE) /* Per-channel TSSI */
			{
				if ((pATEInfo->Channel >= 1) && (pATEInfo->Channel <= 14))	
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s: bExtendedTssiMode = %d, original desiredTSSI = %d, CentralChannel = %d, PerChTxPwrOffset = %d\n", 
						__FUNCTION__, 
						pAd->TxPowerCtrl.bExtendedTssiMode, 
						desiredTSSI, 
						pATEInfo->Channel, 
						pAd->TxPowerCtrl.PerChTxPwrOffset[pATEInfo->Channel]));

					desiredTSSI += pAd->TxPowerCtrl.PerChTxPwrOffset[pATEInfo->Channel];
				}
			}

			if (desiredTSSI > 0x1F)
			{
				desiredTSSI = 0x1F;
			}

			if (desiredTSSI > currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable++;
			}

			if (desiredTSSI < currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable--;
			}

			if (pAd->TxPowerCtrl.idxTxPowerTable < LOWERBOUND_TX_POWER_TUNING_ENTRY)
			{
				pAd->TxPowerCtrl.idxTxPowerTable = LOWERBOUND_TX_POWER_TUNING_ENTRY;
			}

			if (pAd->TxPowerCtrl.idxTxPowerTable >= UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd))
			{
				pAd->TxPowerCtrl.idxTxPowerTable = UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd);
			}

			/* Valid pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 45 */
			pTxPowerTuningEntry = &TxPowerTuningTable[pAd->TxPowerCtrl.idxTxPowerTable + TX_POWER_TUNING_ENTRY_OFFSET]; /* zero-based array */
			pAd->TxPowerCtrl.RF_TX_ALC = pTxPowerTuningEntry->RF_TX_ALC;
			pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("pAd->TxPowerCtrl.idxTxPowerTable = %d, pAd->TxPowerCtrl.RF_TX_ALC = %d, pAd->TxPowerCtrl.MAC_PowerDelta = %d\n", 
			        pAd->TxPowerCtrl.idxTxPowerTable, pAd->TxPowerCtrl.RF_TX_ALC, pAd->TxPowerCtrl.MAC_PowerDelta  ));

			/* Tx power adjustment over RF */
			/* In ATE mode, only adjust TX0_ALC by default. */
			if (IS_RT5350(pAd))
			{
				RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)(&RFValue));
				RFValue &= ~0x3F; /* clear RF_R49[5:0] */
				RFValue |= pAd->TxPowerCtrl.RF_TX_ALC;
				DBGPRINT(RT_DEBUG_TRACE, ("Write RF_R49 = 0x%x\n", RFValue));
				RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)RFValue);
			}
			else if (IS_RT3352(pAd))
			{
				RT30xxReadRFRegister(pAd, RF_R47, (PUCHAR)(&RFValue));
				RFValue &= ~0x1F; /* clear RF_R47[4:0] */
				RFValue |= pAd->TxPowerCtrl.RF_TX_ALC;
				DBGPRINT(RT_DEBUG_TRACE, ("Write RF_R47 = 0x%x\n", RFValue));
				RT30xxWriteRFRegister(pAd, RF_R47, (UCHAR)RFValue);
			}
			else
			{
				RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)(&RFValue));
				TmpValue = (RFValue & 0xE0);
				RFValue = (TmpValue | (pAd->TxPowerCtrl.RF_TX_ALC & 0x1F));
				DBGPRINT(RT_DEBUG_TRACE, ("Write RF_R12 = 0x%x\n", RFValue));
				RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)(RFValue));
			}

			/* Tx power adjustment over MAC */
 			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, currentTSSI = %d, idxTxPowerTable = %d, {RF_TX_ALC = %d, MAC_PowerDelta = %d}\n", 
			        __FUNCTION__, 
			        desiredTSSI, 
			        currentTSSI, 
			        pAd->TxPowerCtrl.idxTxPowerTable, 
			        pTxPowerTuningEntry->RF_TX_ALC, 
			        pTxPowerTuningEntry->MAC_PowerDelta));
		}
	}
#endif /* RTMP_INTERNAL_TX_ALC */

	/* TX power compensation for temperature variation based on TSSI. */
	/* Do it per 4 seconds. */
	if (pATEInfo->OneSecPeriodicRound % 4 == 0)
	{
		if (pATEInfo->Channel <= 14)
		{
			/* bg channel */
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			TssiRef            = pAd->TssiRefG;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryG[0];
			TxAgcStep          = pAd->TxAgcStepG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			/* a channel */
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			TssiRef            = pAd->TssiRefA;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryA[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryA[0];
			TxAgcStep          = pAd->TxAgcStepA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
		{
			/* BbpR49 is unsigned char. */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);

			/* (p) TssiPlusBoundaryG[0] = 0 = (m) TssiMinusBoundaryG[0] */
			/* compensate: +4     +3   +2   +1    0   -1   -2   -3   -4 * steps */
			/* step value is defined in pAd->TxAgcStepG for tx power value */

			/* [4]+1+[4]   p4     p3   p2   p1   o1   m1   m2   m3   m4 */
			/* ex:         0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
			   above value are examined in mass factory production */
			/*             [4]    [3]  [2]  [1]  [0]  [1]  [2]  [3]  [4] */

			/* plus is 0x10 ~ 0x40, minus is 0x60 ~ 0x90 */
			/* if value is between p1 ~ o1 or o1 ~ s1, no need to adjust tx power */
			/* if value is 0x65, tx power will be -= TxAgcStep*(2-1) */

			if (BbpR49 > pTssiMinusBoundary[1])
			{
				/* Reading is larger than the reference value. */
				/* Check for how large we need to decrease the Tx power. */
				for (idx = 1; idx < 5; idx++)
				{
					/* Found the range. */
					if (BbpR49 <= pTssiMinusBoundary[idx])  
						break;
				}

				/* The index is the step we should decrease, idx = 0 means there is nothing to compensate. */
				*pTxAgcCompensate = -(TxAgcStep * (idx-1));
				
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));                    
			}
			else if (BbpR49 < pTssiPlusBoundary[1])
			{
				/* Reading is smaller than the reference value. */
				/* Check for how large we need to increase the Tx power. */
				for (idx = 1; idx < 5; idx++)
				{
					/* Found the range. */
					if (BbpR49 >= pTssiPlusBoundary[idx])   
						break;
				}

				/* The index is the step we should increase, idx = 0 means there is nothing to compensate. */
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, 0));
			}
		}
	}
	else
	{
		if (pATEInfo->Channel <= 14)
		{
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}

	/* Reset different new tx power for different TX rate. */
	for (index=0; index<maxTxPwrCnt; index++)
	{
		if (TxPwr[index] != 0xffffffff)
		{
			for (inner_index=0; inner_index<8; inner_index++)
			{
				Value = (CHAR)((TxPwr[index] >> inner_index*4) & 0x0F); /* 0 ~ 15 */

#ifdef RTMP_INTERNAL_TX_ALC
				/*
					The upper bounds of the MAC 0x1314~0x1324 
					are variable when the STA uses the internal Tx ALC.
				*/
				if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
				{
					switch (TX_PWR_CFG_0 + (index * 4))
					{
						case TX_PWR_CFG_0: 
						{
							if ((Value + TotalDeltaPower) < 0)
							{
								Value = 0;
							}
							else if ((Value + TotalDeltaPower) > 0xE)
							{
								Value = 0xE;
							}
							else
							{
								Value += TotalDeltaPower;
							}
						}
						break;

						case TX_PWR_CFG_1: 
						{
							if ((inner_index >= 0) && (inner_index <= 3))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_2: 
						{
							if ((inner_index == 0) || (inner_index == 2) || (inner_index == 3))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_3: 
						{
							if ((inner_index == 0) || (inner_index == 2) || (inner_index == 3) || 
							((inner_index >= 4) && (inner_index <= 7)))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_4: 
						{
							if ((Value + TotalDeltaPower) < 0)
							{
								Value = 0;
							}
							else if ((Value + TotalDeltaPower) > 0xC)
							{
								Value = 0xC;
							}
							else
							{
								Value += TotalDeltaPower;
							}
						}
						break;

						default: 
						{                                                      
							/* do nothing */
							DBGPRINT_ERR(("%s : unknown register = 0x%X\n", 
								__FUNCTION__, 
							(TX_PWR_CFG_0 + (index << 2))));
						}
						break;
					}
				}
				else
#endif /* RTMP_INTERNAL_TX_ALC */
				{
					if ((Value + DeltaPwr) < 0)
					{
						Value = 0; /* min */
					}
					else if ((Value + DeltaPwr) > 0xF)
					{
						Value = 0xF; /* max */
					}
					else
					{
						Value += DeltaPwr; /* temperature compensation */
					}
				}

				/* fill new value to CSR offset */
				TxPwr[index] = (TxPwr[index] & ~(0x0000000F << inner_index*4)) | (Value << inner_index*4);
			}

			/* write tx power value to CSR */
			/* 
				TX_PWR_CFG_0 (8 tx rate) for	TX power for OFDM 12M/18M
												TX power for OFDM 6M/9M
												TX power for CCK5.5M/11M
												TX power for CCK1M/2M
			*/
			/* TX_PWR_CFG_1 ~ TX_PWR_CFG_4 */
#ifdef DOT11N_SS3_SUPPORT
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				if (index == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, TxPwr[index]);
				}
				else if (index == 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, TxPwr[index]);
				}
#ifdef RT3883
				else if ((IS_RT3883(pAd)) && (index == 7))
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, TxPwr[index]);
				}
				else if ((IS_RT3883(pAd)) && (index == 8))
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, TxPwr[index]);
				}
#endif /* RT3883 */
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (index << 2), TxPwr[index]);
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + (index << 2), (TxPwr[index] & 0xf0f0f0f0) >> 4);
				}
			}
			else
			{
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (index << 2), TxPwr[index]);
			}	
#else
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (index << 2), TxPwr[index]);
#endif /* DOT11N_SS3_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("ATEAsicAdjustTxPower - DeltaPwr=%d, offset=0x%x, TxPwr=%lx, BbpR1=%x, round=%ld, pTxAgcCompensate=%d \n",
				DeltaPwr, TX_PWR_CFG_0 + (index << 2), TxPwr[index], BbpR49, pATEInfo->OneSecPeriodicRound, *pTxAgcCompensate));
			
		}
	}

}


/*
==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in ATE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. TxPowerPercentage
		1. auto calibration based on TSSI feedback
		2. extra 2 db for CCK
		3. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

==========================================================================
*/
VOID ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->AdjustTxPower != NULL)
		pATEInfo->pChipStruct->AdjustTxPower(pAd);
	else
		DBGPRINT_ERR(("%s: AdjustTxPower() for this chipset does not exist !\n", __FUNCTION__));

	return;
}


CHAR ATEConvertToRssi(
	IN PRTMP_ADAPTER pAd,
	IN	CHAR	Rssi,
	IN  UCHAR   RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;
	CHAR	BaseVal;

	/* Rssi equals to zero should be an invalid value */
	if (Rssi == 0)
		return -99;
	
	LNAGain = GET_LNA_GAIN(pAd);
	if (pAd->LatchRfRegs.Channel > 14)
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->ARssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->ARssiOffset1;
		else
			RssiOffset = pAd->ARssiOffset2;
	}
	else
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->BGRssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->BGRssiOffset1;
		else
			RssiOffset = pAd->BGRssiOffset2;
	}

	BaseVal = -12;

#ifdef RT6352
	if (IS_RT6352(pAd))
		BaseVal = -2;
#endif /* RT6352 */

	return (BaseVal - RssiOffset - LNAGain - Rssi);
}


VOID ATESampleRssi(
	IN PRTMP_ADAPTER	pAd,
	IN PRXWI_STRUC		pRxWI)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pRxWI->RSSI0 != 0)
	{
		pATEInfo->LastRssi0	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI0, RSSI_0);
		pATEInfo->AvgRssi0X8 = (pATEInfo->AvgRssi0X8 - pATEInfo->AvgRssi0) + pATEInfo->LastRssi0;
		pATEInfo->AvgRssi0 = pATEInfo->AvgRssi0X8 >> 3;
	}

	if (pRxWI->RSSI1 != 0)
	{
		pATEInfo->LastRssi1	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI1, RSSI_1);
		pATEInfo->AvgRssi1X8 = (pATEInfo->AvgRssi1X8 - pATEInfo->AvgRssi1) + pATEInfo->LastRssi1;
		pATEInfo->AvgRssi1 = pATEInfo->AvgRssi1X8 >> 3;
	}

	if (pRxWI->RSSI2 != 0)
	{
		pATEInfo->LastRssi2	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI2, RSSI_2);
		pATEInfo->AvgRssi2X8 = (pATEInfo->AvgRssi2X8 - pATEInfo->AvgRssi2) + pATEInfo->LastRssi2;
		pATEInfo->AvgRssi2 = pATEInfo->AvgRssi2X8 >> 3;
	}

	pATEInfo->LastSNR0 = (CHAR)(pRxWI->SNR0);
	pATEInfo->LastSNR1 = (CHAR)(pRxWI->SNR1);
#ifdef DOT11N_SS3_SUPPORT
	pATEInfo->LastSNR2 = (CHAR)(pRxWI->SNR2);
#endif /* DOT11N_SS3_SUPPORT */

	pATEInfo->NumOfAvgRssiSample ++;

	return;
}


VOID rt_ee_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT offset = 0;
	USHORT value;

#ifdef RTMP_FLASH_SUPPORT
	{	
		rtmp_ee_flash_read_all(pAd, Data);
		return;
	}
#endif /* RTMP_FLASH_SUPPORT */

	for (offset = 0; offset < (EEPROM_SIZE >> 1);)
	{
		RT28xx_EEPROM_READ16(pAd, (offset << 1), value);
		Data[offset] = value;
		offset++;
	}

	return;
}


VOID rt_ee_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT offset = 0;
	USHORT value;

#ifdef RTMP_FLASH_SUPPORT
	{
		rtmp_ee_flash_write_all(pAd, Data);
		return;
	}
#endif /* RTMP_FLASH_SUPPORT */


	for (offset = 0; offset < (EEPROM_SIZE >> 1);)
	{
		value = Data[offset];
		RT28xx_EEPROM_WRITE16(pAd, (offset << 1), value);
		offset++;
	}

	return;
}


VOID rt_ee_write_bulk(PRTMP_ADAPTER pAd, USHORT *Data, USHORT offset, USHORT length)
{
	USHORT pos;
	USHORT value;
	USHORT len = length;

#ifdef RTMP_FLASH_SUPPORT
	{
		for (pos = 0; pos < (len >> 1);)
		{
			value = Data[pos];
			rtmp_ee_flash_write(pAd, offset+(pos*2), value);
			pos++;
		}
		
		return;
	}
#endif /* RTMP_FLASH_SUPPORT */

	for (pos = 0; pos < (len >> 1);)
	{
		value = Data[pos];
		RT28xx_EEPROM_WRITE16(pAd, offset+(pos*2), value);
		pos++;
	}

	return;
}


VOID RtmpRfIoWrite(
	IN PRTMP_ADAPTER pAd)
{
	/* Set RF value 1's set R3[bit2] = [0] */
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RTMPusecDelay(200);

	/* Set RF value 2's set R3[bit2] = [1] */
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 | 0x04));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RTMPusecDelay(200);

	/* Set RF value 3's set R3[bit2] = [0] */
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	return;
}


VOID DefaultATEAsicSetTxRxPath(
    IN PRTMP_ADAPTER pAd)
{

}


VOID ATEAsicSetTxRxPath(
    IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->AsicSetTxRxPath != NULL)
		pATEInfo->pChipStruct->AsicSetTxRxPath(pAd);

	return;
}


/*
==========================================================================
    Description:

	Default AsicSwitchChannel() dedicated for ATE.
    
==========================================================================
*/
VOID DefaultATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR BbpValue = 0, R66 = 0x30, Channel = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A;/* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* Rx High power VGA offset for LNA select */
		if (pAd->NicConfig2.field.ExternalLNAForG)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x84);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		/* 2.4 G band selection PIN */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Turn off unused PA or LNA when only 1T or 1R. */
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		/* calibration power unbalance issues */
		if (pAd->Antenna.field.TxPath == 2)
		{
			if (pATEInfo->TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pATEInfo->TxAntennaSel == 2)
			{
				TxPinCfg &= 0xFFFFFFFD;
			}
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
	/* channel > 14 */
	else
	{
	    UINT32	TxPinCfg = 0x00050F05;/* 2007.10.09 by Brian : 0x00050505 ==> 0x00050F05 */
		
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* According the Rory's suggestion to solve the middle range issue. */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);        

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0xF2);

		/* Rx High power VGA offset for LNA select */
		if (pAd->NicConfig2.field.ExternalLNAForA)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R91, &BbpValue);
		ASSERT((BbpValue == 0x04));

		/* 5 G band selection PIN, bit1 and bit2 are complement */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Turn off unused PA or LNA when only 1T or 1R. */
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}

		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}

	/* R66 should be set according to Channel. */
	if (Channel <= 14)
	{	
		/* BG band */
		R66 = 0x2E + GET_LNA_GAIN(pAd);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
	}
	else
	{	
		/* A band, BW == 20 */
		if (pATEInfo->TxWI.BW == BW_20)
		{
			R66 = (UCHAR)(0x32 + (GET_LNA_GAIN(pAd)*5)/3);
    		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
		else
		{
			/* A band, BW == 40 */
			R66 = (UCHAR)(0x3A + (GET_LNA_GAIN(pAd)*5)/3);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
	}

	RtmpOsMsDelay(1);  

}


/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for ATE.
    
==========================================================================
*/
VOID ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->ChannelSwitch != NULL)
		pATEInfo->pChipStruct->ChannelSwitch(pAd);

	return;
}


VOID BbpSoftReset(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BbpData = 0;

	/* Soft reset, set BBP R21 bit0=1->0 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpData);
	BbpData |= 0x00000001; /* set bit0=1 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpData);
	BbpData &= ~(0x00000001); /* set bit0=0 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);

	return;
}


static VOID BbpHardReset(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 MacData = 0;


	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData = MacData | 0x00000002;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	RtmpOsMsDelay(10);

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData = MacData & ~(0x00000002);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	return;
}


static int CheckMCSValid(
	IN PRTMP_ADAPTER	pAd, 
	IN UCHAR Mode,
	IN UCHAR Mcs)
{
	int index;
	PCHAR pRateTab = NULL;

	switch (Mode)
	{
		case MODE_CCK:
			pRateTab = CCKRateTable;
			break;
		case MODE_OFDM:
			pRateTab = OFDMRateTable;
			break;
			
		case 2: /*MODE_HTMIX*/
		case 3: /*MODE_HTGREENFIELD*/
#ifdef DOT11N_SS3_SUPPORT
			if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd))
				pRateTab = HTMIXRateTable3T3R;
			else
#endif /* DOT11N_SS3_SUPPORT */
				pRateTab = HTMIXRateTable;
			break;
			
		default: 
			DBGPRINT_ERR(("unrecognizable Tx Mode %d\n", Mode));
			return -1;
			break;
	}

	index = 0;
	while (pRateTab[index] != -1)
	{
		if (pRateTab[index] == Mcs)
			return 0;
		index++;
	}

	return -1;
}


INT DefaultATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR TxPower = 0;
#ifdef RTMP_RF_RW_SUPPORT
#ifndef RF_BANK
	UCHAR RFValue = 0;
#endif /* !RF_BANK */
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RALINK_QA
	if ((pATEInfo->bQATxStart == TRUE) || (pATEInfo->bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pATEInfo->TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */

	if (index == 0)
	{
		TxPower = pATEInfo->TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pATEInfo->TxPower1;
	}
	else
	{
		DBGPRINT_ERR(("%s : Only TxPower0 and TxPower1 are adjustable !\n", __FUNCTION__));
		DBGPRINT_ERR(("TxPower%d is out of range !\n", index));
		return -1;
	}

#ifdef RTMP_RF_RW_SUPPORT

		if ((IS_RT30xx(pAd)  || IS_RT3390(pAd)))
		{
			{
#ifndef RF_BANK			
				/* Set Tx Power */
				UCHAR ANT_POWER_INDEX=RF_R12+index;
				ATE_RF_IO_READ8_BY_REG_ID(pAd, ANT_POWER_INDEX, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPower;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, ANT_POWER_INDEX, (UCHAR)RFValue);
				DBGPRINT(RT_DEBUG_TRACE, ("3070 or 2070:%s (TxPower[%d]=%d, RFValue=%x)\n", __FUNCTION__, index,TxPower, RFValue));
#endif /* RF_BANK */
			}
		}
		else
#endif /* RTMP_RF_RW_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
	
	return 0;
}


INT ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->TxPwrHandler != NULL)
		pATEInfo->pChipStruct->TxPwrHandler(pAd, index);

	return 0;
}


/*
========================================================================

	Routine Description:
		Set Japan filter coefficients if needed.
	Note:
		This routine should only be called when
		entering TXFRAME mode or TXCONT mode.
				
========================================================================
*/
static VOID SetJapanFilter(
	IN		PRTMP_ADAPTER	pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR			BbpData = 0;

	/*
		If Channel=14 and Bandwidth=20M and Mode=CCK, set BBP R4 bit5=1
		(Japan Tx filter coefficients)when (TXFRAME or TXCONT).
	*/
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);

	if ((pATEInfo->TxWI.PHYMODE == MODE_CCK) && (pATEInfo->Channel == 14) && (pATEInfo->TxWI.BW == BW_20))
	{
		BbpData |= 0x20;    /* turn on */
		DBGPRINT(RT_DEBUG_TRACE, ("SetJapanFilter!!!\n"));
	}
	else
	{
		BbpData &= 0xdf;    /* turn off */
		DBGPRINT(RT_DEBUG_TRACE, ("ClearJapanFilter!!!\n"));
	}

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpData);

	return;
}


/*
========================================================================

	Routine Description:
		Disable protection for ATE.
========================================================================
*/
VOID ATEDisableAsicProtect(
	IN		PRTMP_ADAPTER	pAd)
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6];
	USHORT			offset;
	UCHAR			step;
	UINT32 MacReg = 0;

	/* Config ASIC RTS threshold register */
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;
	MacReg |= (0xFFF << 8);
	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	/* Initial common protection settings */
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	/* Handle legacy(B/G) protection */
	ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;
	ProtCfg.field.ProtectCtrl = 0;
	Protect[0] = ProtCfg.word;
	Protect[1] = ProtCfg.word;
	/* CTS-self is not used */
	pAd->FlgCtsEnabled = 0; 

	/*
		NO PROTECT 
			1.All STAs in the BSS are 20/40 MHz HT
			2. in a 20/40MHz BSS
			3. all STAs are 20MHz in a 20MHz BSS
		Pure HT. no protection.
	*/
	/*
		MM20_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 010111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None)
			PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
	*/
	Protect[2] = 0x01744004;	

	/*
		MM40_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 111111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None) 
			PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
	*/
	Protect[3] = 0x03f44084;

	/*
		CF20_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 010111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None)
			PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
	*/
	Protect[4] = 0x01744004;

	/*
		CF40_PROT_CFG
			Reserved (31:27)
			PROT_TXOP(25:20) -- 111111
			PROT_NAV(19:18)  -- 01 (Short NAV protection)
			PROT_CTRL(17:16) -- 00 (None)
			PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
	*/
	Protect[5] = 0x03f44084;

	pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
	
	offset = CCK_PROT_CFG;
	for (step = 0;step < 6;step++)
		RTMP_IO_WRITE32(pAd, offset + step*4, Protect[step]);

	return;
}


#ifdef CONFIG_AP_SUPPORT 
/*
==========================================================================
	Description:
		Used only by ATE to disassociate all STAs and stop AP service.
	Note:
==========================================================================
*/
VOID ATEAPStop(
	IN PRTMP_ADAPTER pAd) 
{
	BOOLEAN     Cancelled;
	UINT32		Value = 0;
	INT         apidx = 0;
		
	DBGPRINT(RT_DEBUG_TRACE, ("!!! ATEAPStop !!!\n"));

	/* To prevent MCU to modify BBP registers w/o indications from driver. */
#ifdef DFS_SUPPORT
	NewRadarDetectionStop(pAd);
#endif /* DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
	{
		CarrierDetectionStop(pAd);
	}
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
	ApCliIfDown(pAd);
#endif /* APCLI_SUPPORT */

	MacTableReset(pAd);

	/* Disable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &=0xe;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	/* Disable piggyback */
	RTMPSetPiggyBack(pAd, FALSE);

	ATEDisableAsicProtect(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		AsicDisableSync(pAd);

#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif /* LED_CONTROL_SUPPORT */
	}


	for (apidx = 0; apidx < MAX_MBSSID_NUM(pAd); apidx++)
	{
		if (pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning == TRUE)
		{
			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].REKEYTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning = FALSE;
		}
	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
	}
	
#ifdef WAPI_SUPPORT
	RTMPCancelWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */
	
	/* Cancel the Timer, to make sure the timer was not queued. */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

	if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE)
		RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */


#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPEnable == TRUE)
	{
		RTMP_CHIP_DISABLE_AP_MIMOPS(pAd);
		pAd->ApCfg.GreenAPLevel=GREENAP_WITHOUT_ANY_STAS_CONNECT;
		pAd->ApCfg.bGreenAPEnable = FALSE;
	}
#endif /* GREENAP_SUPPORT */



#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);

#ifdef DYNAMIC_VGA_SUPPORT
		if (pAd->CommonCfg.MO_Cfg.bDyncVGAEnable == TRUE)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x83);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x32);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x86);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x19);

			ATEBBPWriteWithRxChain(pAd, BBP_R66, pAd->CommonCfg.MO_Cfg.Stored_BBP_R66, RX_CHAIN_ALL);
		}
#endif /* DYNAMIC_VGA_SUPPORT */
	}
#endif /* RT6352 */
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID RTMPStationStop(
    IN  PRTMP_ADAPTER   pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStop\n"));

#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
	}
#endif /* RT6352 */

    /* Do nothing. */

    DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStop\n"));
}


VOID RTMPStationStart(
    IN  PRTMP_ADAPTER   pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStart\n"));

#ifdef RTMP_MAC_PCI
	pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStart\n"));
}
#endif /* CONFIG_STA_SUPPORT */




static NDIS_STATUS ATESTART(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0, atemode=0, temp=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PTXD_STRUC		pTxD = NULL;
#ifdef RT_BIG_ENDIAN
	PTXD_STRUC      pDestTxD = NULL;
	TXD_STRUC       TxD;
#endif /* RT_BIG_ENDIAN */
#endif /* RTMP_MAC_PCI */
	UCHAR			BbpData = 0;
	PATE_CHIP_STRUCT pChipStruct = pATEInfo->pChipStruct;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));


#ifdef RTMP_MAC_PCI
#ifndef RTMP_RBUS_SUPPORT
	/* check if we have removed the firmware */
	if (!(ATE_ON(pAd)))
	{
		NICEraseFirmware(pAd);
	}
#endif /* RTMP_RBUS_SUPPORT */
#endif /* RTMP_MAC_PCI */

	atemode = pATEInfo->Mode;
	pATEInfo->Mode = ATE_START;
#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		UINT32 mac_value;

		/* reset/disable DPD */
		pATEInfo->bDPDEnable = FALSE;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
		pAd->Tx0_DPD_ALC_tag0 = 0;
		pAd->Tx0_DPD_ALC_tag1 = 0;
		pAd->Tx1_DPD_ALC_tag0 = 0;
		pAd->Tx1_DPD_ALC_tag1 = 0;
		pAd->Tx0_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx0_DPD_ALC_tag1_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag1_flag = 0x0;

		if (atemode == ATE_STOP)
		{
		RT6352_RTMPReadTxPwrPerRate(pAd);
		RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &mac_value);
		mac_value &= 0xFFFFFFC0;
		RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, mac_value);
		}
	}
#endif /* RT6352 */

	if ((atemode == ATE_STOP) && (!IS_RT6352(pAd)))
	{
		/* DUT just enters ATE mode from normal mode. */
		/* Only at this moment, we need to switch back to the channel of normal mode. */
    }


	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
	
	/* Disable auto responder */
	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &temp);
	temp = temp & 0xFFFFFFFE;
	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, temp);

	ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	if (atemode == ATE_TXCARR)
	{
		if (pChipStruct->bBBPStoreTXCARR == TRUE)
		{
			UINT32			bbp_index=0;
			UCHAR			RestoreRfICType=pAd->RfIcType;

			BbpHardReset(pAd);

			/* Restore All BBP Value */
			for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

			pAd->RfIcType=RestoreRfICType;
		}

		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			/* No Carrier Test set BBP R22 bit6=0, bit[5~0]=0x0 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF80; /* clear bit6, bit[5~0] */	
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

			BbpSoftReset(pAd);
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, (pATEInfo->Default_TX_PIN_CFG));
		}
		else
		{
			/* No Carrier Test set BBP R22 bit7=0, bit6=0, bit[5~0]=0x0 */
			ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);
		}
	}
	else if (atemode == ATE_TXCARRSUPP)
	{
		if (pChipStruct->bBBPStoreTXCARRSUPP == TRUE)
		{
			UINT32			bbp_index=0;
			UCHAR			RestoreRfICType=pAd->RfIcType;

			BbpHardReset(pAd);

			/* Restore All BBP Value */
			for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

			pAd->RfIcType=RestoreRfICType;			
		}

#ifdef RT6352
		/* No Carrier Suppression set BBP R21 bit4=0 */
		if (IS_RT6352(pAd))
		{
			BbpData = 0x00;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);
		}
#endif /* RT6352 */

		/* No Cont. TX set BBP R22 bit7=0 */
		ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

		/* No Carrier Suppression set BBP R24 bit0=0 */
		ATE_BBP_CTS_TX_SIN_WAVE_DISABLE(pAd, BBP_R24, &BbpData);

		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			BbpSoftReset(pAd);
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, (pATEInfo->Default_TX_PIN_CFG));
		}
	}		

	/*
		We should free some resource which was allocated
		when ATE_TXFRAME , ATE_STOP, and ATE_TXCONT.
	*/
	else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
	{
#ifdef RTMP_MAC_PCI
		PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];
#endif /* RTMP_MAC_PCI */
		if (atemode == ATE_TXCONT)
		{
			if (pChipStruct->bBBPStoreTXCONT == TRUE)
			{
				UINT32			bbp_index=0;
				UCHAR			RestoreRfICType=pAd->RfIcType;

				BbpHardReset(pAd);

				/* Restore All BBP Value */
				for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

				pAd->RfIcType=RestoreRfICType;
			}
#ifdef RT305x
			if (IS_RT3050(pAd))
			{
				/* fix RT3050S ATE SWITCH Mode */
				/* suggestion from Arvin Wang at 01/28/2010 */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
			}
#endif /* RT305x */

			/* Not Cont. TX anymore, so set BBP R22 bit7=0 */
			ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

			if (pATEInfo->TxMethod == TX_METHOD_1)
			{
				BbpSoftReset(pAd);
				RTMP_IO_WRITE32(pAd, TX_PIN_CFG, (pATEInfo->Default_TX_PIN_CFG));
			}
		}

		/* Abort Tx, Rx DMA. */
		RtmpDmaEnable(pAd, 0);
#ifdef RTMP_MAC_PCI
		for (ring_index=0; ring_index<TX_RING_SIZE; ring_index++)
		{
			PNDIS_PACKET  pPacket;

#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[ring_index].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[ring_index].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* !RT_BIG_ENDIAN */
			pTxD->DMADONE = 0;
			pPacket = pTxRing->Cell[ring_index].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[ring_index].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNextNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
#endif /* RTMP_MAC_PCI */

		/* Start Tx, RX DMA */
		RtmpDmaEnable(pAd, 1);
	}


	/* reset Rx statistics. */
	pATEInfo->LastSNR0 = 0;
	pATEInfo->LastSNR1 = 0;
#ifdef DOT11N_SS3_SUPPORT
	pATEInfo->LastSNR2 = 0;
#endif /* DOT11N_SS3_SUPPORT */
	pATEInfo->LastRssi0 = 0;
	pATEInfo->LastRssi1 = 0;
	pATEInfo->LastRssi2 = 0;
	pATEInfo->AvgRssi0 = 0;
	pATEInfo->AvgRssi1 = 0;
	pATEInfo->AvgRssi2 = 0;
	pATEInfo->AvgRssi0X8 = 0;
	pATEInfo->AvgRssi1X8 = 0;
	pATEInfo->AvgRssi2X8 = 0;
	pATEInfo->NumOfAvgRssiSample = 0;

#ifdef RALINK_QA
	/* Tx frame */
	pATEInfo->bQATxStart = FALSE;
	pATEInfo->bQARxStart = FALSE;
	pATEInfo->seq = 0; 

	/* counters */
	pATEInfo->U2M = 0;
	pATEInfo->OtherData = 0;
	pATEInfo->Beacon = 0;
	pATEInfo->OtherCount = 0;
	pATEInfo->TxAc0 = 0;
	pATEInfo->TxAc1 = 0;
	pATEInfo->TxAc2 = 0;
	pATEInfo->TxAc3 = 0;
	pATEInfo->TxHCCA = 0;
	pATEInfo->TxMgmt = 0;
	pATEInfo->RSSI0 = 0;
	pATEInfo->RSSI1 = 0;
	pATEInfo->RSSI2 = 0;
	pATEInfo->SNR0 = 0;
	pATEInfo->SNR1 = 0;
#ifdef DOT11N_SS3_SUPPORT
	pATEInfo->SNR2 = 0;
#endif /* DOT11N_SS3_SUPPORT */

	/* control */
	pATEInfo->TxDoneCount = 0;
	/* TxStatus : 0 --> task is idle, 1 --> task is running */
	pATEInfo->TxStatus = 0;
#endif /* RALINK_QA */

#if !defined(RT3883) && !defined(RT3593)
	if (!IS_RT6352(pAd))
	{
		/* Soft reset BBP. */
		BbpSoftReset(pAd);
	}
#endif /* !defined(RT3883) && !defined(RT3593) */

#ifdef CONFIG_AP_SUPPORT 
	if (atemode == ATE_STOP)
		ATEAPStop(pAd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT 
	AsicDisableSync(pAd);
	ATEDisableAsicProtect(pAd);
	RTMPStationStop(pAd);
#endif /* CONFIG_STA_SUPPORT */

	if ((atemode == ATE_STOP) && (pATEInfo->PeriodicTimer.State == FALSE))
	{
		/* Do it for the first time entering ATE mode */
		pATEInfo->PeriodicTimer.State = TRUE;
	}
	
	if (pATEInfo->PeriodicTimer.State == TRUE)
	{
		/* 
			For rx statistics, we cancel pAd->Mlme.PeriodicTimer
			and set pAd->ate.PeriodicTimer.
		*/
		RTMPCancelTimer(&pAd->Mlme.PeriodicTimer, &Cancelled);
		/* Init ATE periodic timer */
		RTMPInitTimer(pAd, &pAd->ate.PeriodicTimer, GET_TIMER_FUNCTION(ATEPeriodicExec), pAd, TRUE);
		/* Set ATE periodic timer */
		RTMPSetTimer(&pAd->ate.PeriodicTimer, ATE_TASK_EXEC_INTV);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("We are still in ATE mode, "));
		DBGPRINT(RT_DEBUG_TRACE, ("so we keep ATE periodic timer running.\n"));
	}

#ifdef RTMP_MAC_PCI
	/* Disable Tx */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);


#endif /* RTMP_MAC_PCI */


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS ATESTOP(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0, ring_index=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
#ifdef RT_BIG_ENDIAN
	PRXD_STRUC				pDestRxD;
	RXD_STRUC				RxD;
#endif /* RT_BIG_ENDIAN */
	PRXD_STRUC		pRxD = NULL;
#endif /* RTMP_MAC_PCI */
	UCHAR			BbpData = 0;
	PATE_CHIP_STRUCT pChipStruct = pATEInfo->pChipStruct;
	BOOLEAN Cancelled;
	
	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	if (pChipStruct->bBBPLoadATESTOP == TRUE)
	{
		UINT32			bbp_index=0;
		UCHAR			RestoreRfICType=pAd->RfIcType;

		BbpHardReset(pAd);

		/* Supposed that we have had a record in restore_BBP[] */
		/* restore all BBP value */
		for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

		ASSERT(RestoreRfICType != 0);
		pAd->RfIcType=RestoreRfICType;
	}

	/* Default value in BBP R22 is 0x0. */
	ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);

	/* Clear bit4 to stop continuous Tx production test. */
	ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
	
	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
	
	/* Abort Tx, RX DMA */
	RtmpDmaEnable(pAd, 0);

	/* Disable Tx */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd))
	{
		pATEInfo->bFWLoading = TRUE;
		Status = NICLoadFirmware(pAd);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT_ERR(("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
			return Status;
		}
	}
	pATEInfo->Mode = ATE_STOP;
	BbpSoftReset(pAd);

	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
#endif /* RTMP_MAC_PCI */

	if (pATEInfo->PeriodicTimer.State == FALSE)
	{
		/* 
			For rx statistics, we cancel pAd->Mlme.PeriodicTimer
			and set pATEInfo->PeriodicTimer in stead of. 
			Now we recover it before we leave ATE mode.
		*/
		RTMPCancelTimer(&pATEInfo->PeriodicTimer, &Cancelled);
		/* Init MLME periodic timer */
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExec), pAd, TRUE);
		/* Set MLME periodic timer */
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);
	}
	else
	{
		/* ATE periodic timer has been cancelled. */
		Status = NDIS_STATUS_FAILURE;
		DBGPRINT_ERR(("Initialization of MLME periodic timer failed, Status[=0x%08x]\n", Status));

		return Status;
	}	

#ifdef RTMP_MAC_PCI
	NICInitializeAdapter(pAd, TRUE);
	
	for (ring_index = 0; ring_index < RX_RING_SIZE; ring_index++)
	{
#ifdef RT_BIG_ENDIAN
		pDestRxD = (PRXD_STRUC) pAd->RxRing.Cell[ring_index].AllocVa;
		RxD = *pDestRxD;
		pRxD = &RxD;
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
		/* Point to Rx indexed rx ring descriptor */
		pRxD = (PRXD_STRUC) pAd->RxRing.Cell[ring_index].AllocVa;
#endif /* RT_BIG_ENDIAN */

		pRxD->DDONE = 0;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif /* RT_BIG_ENDIAN */
	}

	/* We should read EEPROM for all cases. */  
	NICReadEEPROMParameters(pAd, NULL);
	NICInitAsicFromEEPROM(pAd); 

#ifdef RT6352
	if (IS_RT6352(pAd))
	{
#if defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT)
		{
			ULONG SysRegValue;

			RTMP_SYS_IO_READ32(0xb0000060, &SysRegValue);
			if ((SysRegValue & 0x100000) == 0x0)
			{
				SysRegValue |= 0x100000;
				RTMP_SYS_IO_WRITE32(0xb0000060, SysRegValue);
				DBGPRINT(RT_DEBUG_ERROR,("Change as GPIO Mode(0x%x)\n", SysRegValue));
			}
		}

		RT6352_Restore_RF_BBP(pAd);
#endif /* defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT) */

		/* Do R-Calibration */
		R_Calibration(pAd);

#ifdef RTMP_TEMPERATURE_CALIBRATION
		/* Temperature Init */
		RT6352_Temperature_Init(pAd);		
		RT6352_TemperatureCalibration(pAd);
#endif /* RTMP_TEMPERATURE_CALIBRATION */

		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, TRUE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);

		/* TX Self TX DC Calibration */
		RF_SELF_TXDC_CAL(pAd);

		/* Rx DCOC Calibration */
		RxDCOC_Calibration(pAd);

		/* BandWidth Filter Calibration */
		BW_Filter_Calibration(pAd,TRUE);
		BW_Filter_Calibration(pAd,FALSE);

		/* Do LOFT and IQ Calibration */
		LOFT_IQ_Calibration(pAd);

		/* DPD_Calibration */
#ifdef RT6352_EP_SUPPORT
		if (pAd->bExtPA == FALSE)
#endif /* RT6352_EP_SUPPORT */
		{
			DoDPDCalibration(pAd);
			pAd->DoDPDCurrTemperature = 0x7FFFFFFF;
		}

		/* Rx DCOC Calibration */
		RxDCOC_Calibration(pAd);

		/* Do RXIQ Calibration */
		RXIQ_Calibration(pAd);

#if defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT)
		{
			ULONG SysRegValue;

			/* Check the gpio setting first */
			RTMP_SYS_IO_READ32(0xb0000060, &SysRegValue);
			if ((SysRegValue & 0x100000) == 0x100000)
			{
				SysRegValue &= (~0x100000);
				RTMP_SYS_IO_WRITE32(0xb0000060, SysRegValue);
				DBGPRINT(RT_DEBUG_ERROR,("Change as Normal Mode(0x%x)\n", SysRegValue));
			}
		}

		RT6352_Init_ExtPA_ExtLNA(pAd, TRUE);
#endif /* defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT) */
	}
#endif /* RT6352 */

	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);

	/* empty function */
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);		

#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		if (pATEInfo->bAutoTxAlc == TRUE)
		{
			OS_SEM_LOCK(&(pATEInfo->TssiSemLock));
			RT635xTssiDcCalibration(pAd);
			OS_SEM_UNLOCK(&(pATEInfo->TssiSemLock));
			RT635xInitMcsPowerTable(pAd);
			RT635xInitRfPaModeTable(pAd);
		}
	}
#endif /* RT6352 */
#endif /* RTMP_INTERNAL_TX_ALC */

	/* clear garbage interrupts */
	RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, 0xffffffff);
	/* Enable Interrupt */
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */


#ifdef CONFIG_AP_SUPPORT 
	/* restore RX_FILTR_CFG */
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, APNORMAL);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT 
	/* restore RX_FILTR_CFG due to that QA maybe set it to 0x3 */
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, STANORMAL);
#endif /* CONFIG_STA_SUPPORT */

	/* Enable Tx */
	ATE_MAC_TX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Enable Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);

	/* Enable Rx */
	ATE_MAC_RX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);

#ifdef RTMP_MAC_PCI
#ifdef CONFIG_AP_SUPPORT 
	APStartUp(pAd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT 
	RTMPStationStart(pAd);
#endif /* CONFIG_STA_SUPPORT */
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXCARR(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	UCHAR			BbpData = 0;
	PATE_CHIP_STRUCT pChipStruct = pATEInfo->pChipStruct;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pATEInfo->Mode = ATE_TXCARR;

	if (pChipStruct->bBBPStoreTXCARR == TRUE)
	{
		UINT32 bbp_index=0;

		/* Zero All BBP Value */	
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		restore_BBP[bbp_index]=0;

	/* Record All BBP Value */
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
	}


	/* QA has done the following steps if it is used. */
	if (pATEInfo->bQATxStart == FALSE) 
	{
		if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)) && (!IS_RT3593(pAd)))
			BbpSoftReset(pAd);/* Soft reset BBP. */

		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			if (!IS_RT5592(pAd))			
			{
				/* store the original value of TX_PIN_CFG */
				RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));

				/* give TX_PIN_CFG(0x1328) a proper value. */
				if (pATEInfo->Channel <= 14)
				{
					/* G band */
					MacData = TXCONT_TX_PIN_CFG_G;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
				}
				else
				{
					/* A band */
					MacData = TXCONT_TX_PIN_CFG_A;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
				}
			}

			/* Carrier Test set BBP R22 bit6=1, bit[5~0]=0x01 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF80; /* bit6, bit[5~0] */
			BbpData |= 0x00000041; /* set bit6=1, bit[5~0]=0x01 */
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
		}
		else
		{
			/* Carrier Test set BBP R22 bit7=1, bit6=1, bit[5~0]=0x01 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF00; /* clear bit7, bit6, bit[5~0] */
			BbpData |= 0x000000C1; /* set bit7=1, bit6=1, bit[5~0]=0x01 */
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

			/* Set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1. */
			ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXCONT(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PTXD_STRUC		pTxD = NULL;
	PRTMP_TX_RING 	pTxRing = &pAd->TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
    TXD_STRUC       TxD;
#endif /* RT_BIG_ENDIAN */
#endif /* RTMP_MAC_PCI */
	UCHAR			BbpData = 0;
	PATE_CHIP_STRUCT pChipStruct = pATEInfo->pChipStruct;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pATEInfo->Mode = ATE_TXCONT;
	
	if (pATEInfo->bQATxStart == TRUE)
	{
		/*
			set MAC_SYS_CTRL(0x1004) bit4(Continuous Tx Production Test)
			and bit2(MAC TX enable) back to zero.
		*/ 
		ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
		ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

		/* set BBP R22 bit7=0 */
		ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);
	}
	else
	{
		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			if (!IS_RT5592(pAd))
			{
				/* store the original value of TX_PIN_CFG */
				RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));
			}
		}
	}

	if (pChipStruct->bBBPStoreTXCONT == TRUE)
	{
		UINT32 bbp_index=0;

		/* Zero All BBP Value */
		for(bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			restore_BBP[bbp_index]=0;

		/* Record All BBP Value */
		for(bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
	}

	/* Step 1: send 50 packets first. */
	pATEInfo->TxCount = 50;

	if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)) && (!IS_RT3593(pAd)))
		BbpSoftReset(pAd);/* Soft reset BBP. */

	/* Abort Tx, RX DMA. */
	RtmpDmaEnable(pAd, 0);

#ifdef RTMP_MAC_PCI
	{
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
		pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
		pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);
	}
#endif /* RTMP_MAC_PCI */

	/* Do it after Tx/Rx DMA is aborted. */
	pATEInfo->TxDoneCount = 0;
	
	/* Only needed if we have to send some normal frames. */
	SetJapanFilter(pAd);

#ifdef RTMP_MAC_PCI
	for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pATEInfo->TxCount); ring_index++)
	{
		PNDIS_PACKET pPacket;
		UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
		pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
		pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
		TxD = *pDestTxD;
		pTxD = &TxD;
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* !RT_BIG_ENDIAN */

		/* Clear current cell. */
		pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNdisPacket = NULL;

		pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNextNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

		if (ATESetUpFrame(pAd, TxIdx) != 0)
			return NDIS_STATUS_FAILURE;

		INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
	}

	ATESetUpFrame(pAd, pTxRing->TxCpuIdx);
#endif /* RTMP_MAC_PCI */


	/* Enable Tx */
	ATE_MAC_TX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Start Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);


#ifdef RALINK_QA
	if (pATEInfo->bQATxStart == TRUE)
	{
		pATEInfo->TxStatus = 1;
	}
#endif /* RALINK_QA */

#ifdef RTMP_MAC_PCI
	/* kick Tx Ring */
	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

	RtmpOsMsDelay(5);
#endif /* RTMP_MAC_PCI */


	if (pATEInfo->TxMethod == TX_METHOD_1)
	{
		if (!IS_RT5592(pAd))
		{
			/* give TX_PIN_CFG(0x1328) a proper value. */
			if (pATEInfo->Channel <= 14)
			{
				/* G band */
				MacData = TXCONT_TX_PIN_CFG_G;
				RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
			}
			else
			{
				/* A band */
				MacData = TXCONT_TX_PIN_CFG_A;
				RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
			}
		}

		/* Cont. TX set BBP R22 bit7=1 */
		ATE_BBP_START_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

#ifdef RT5592EP_SUPPORT
		/* enable continuous tx production test */
		if (pAd->chipCap.Priv == RT5592_TYPE_EP)
			ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
#endif /* RT5592EP_SUPPORT */
	}
	else
	{
		/* Step 2: send more 50 packets then start Continuous Tx Mode. */
		/* Abort Tx, RX DMA. */
		RtmpDmaEnable(pAd, 0);

		/* Cont. TX set BBP R22 bit7=1 */
		ATE_BBP_START_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

		pATEInfo->TxCount = 50;
#ifdef RTMP_MAC_PCI
		{
			RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);					
		}
#endif /* RTMP_MAC_PCI */

		pATEInfo->TxDoneCount = 0;
		SetJapanFilter(pAd);

#ifdef RTMP_MAC_PCI
		for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pATEInfo->TxCount); ring_index++)
		{
			PNDIS_PACKET pPacket;
			UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* !RT_BIG_ENDIAN */
			/* clear current cell */
			pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNdisPacket as NULL after clear. */
			pTxRing->Cell[TxIdx].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNextNdisPacket as NULL after clear. */
			pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

			if (ATESetUpFrame(pAd, TxIdx) != 0)
				return NDIS_STATUS_FAILURE;

			INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
		}

		ATESetUpFrame(pAd, pTxRing->TxCpuIdx);
#endif /* RTMP_MAC_PCI */


		/* Enable Tx */
		ATE_MAC_TX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);

		/* Disable Rx */
		ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

		/* Start Tx, Rx DMA. */
		RtmpDmaEnable(pAd, 1);

#ifdef RALINK_QA
		if (pATEInfo->bQATxStart == TRUE)
		{
			pATEInfo->TxStatus = 1;
		}
#endif /* RALINK_QA */

#ifdef RTMP_MAC_PCI
		/* kick Tx Ring */
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);
#endif /* RTMP_MAC_PCI */

		RTMPusecDelay(500);

		/* enable continuous tx production test */
		ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXCARS(
        IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	UCHAR			BbpData = 0;
	PATE_CHIP_STRUCT pChipStruct = pATEInfo->pChipStruct;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pATEInfo->Mode = ATE_TXCARRSUPP;

	if (pChipStruct->bBBPStoreTXCARRSUPP == TRUE)
	{
		UINT32 bbp_index=0;

		/* Zero All BBP Value */
        for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
                restore_BBP[bbp_index]=0;

        /* Record All BBP Value */
        for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
                ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
	}

	/* QA has done the following steps if it is used. */
	if (pATEInfo->bQATxStart == FALSE) 
	{
#if !defined(RT3883) && !defined(RT3593)
		if (!IS_RT3883(pAd) && !IS_RT3593(pAd))
		{
			/* RT3883 and RT3593 do not need BbpSoftReset() */
			/* Soft reset BBP. */
			BbpSoftReset(pAd);
		}
#endif /* !defined(RT3883) && !defined(RT3593) */

		if (pATEInfo->TxMethod == TX_METHOD_1)
		{
			if (!IS_RT5592(pAd))
			{
				/* store the original value of TX_PIN_CFG */
				RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));

				/* give TX_PIN_CFG(0x1328) a proper value. */
				if (pATEInfo->Channel <= 14)
				{
					/* G band */
					MacData = TXCONT_TX_PIN_CFG_G;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
				}
				else
				{
					/* A band */
					MacData = TXCONT_TX_PIN_CFG_A;
					RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacData);
				}
			}

#ifdef RT6352
			if (IS_RT6352(pAd))
			{
				/* Carrier Suppression set BBP R21 bit4=1 (Enable Continuous Tx Mode) */
				BbpData = 0x10;
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);
				BbpData = 0x00;
			}
#endif /* RT6352 */

			/* Carrier Suppression set BBP R22 bit7=1 (Enable Continuous Tx Mode) */
			ATE_BBP_START_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

			/* Carrier Suppression set BBP R24 bit0=1 (TX continuously send out 5.5MHZ sin save) */
			ATE_BBP_CTS_TX_SIN_WAVE_ENABLE(pAd, BBP_R24, &BbpData);
		}
		else
		{
			/* Carrier Suppression set BBP R22 bit7=1 (Enable Continuous Tx Mode) */
			ATE_BBP_START_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

			/* Carrier Suppression set BBP R24 bit0=1 (TX continuously send out 5.5MHZ sin save) */
			ATE_BBP_CTS_TX_SIN_WAVE_ENABLE(pAd, BBP_R24, &BbpData);

			/* Set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1. */
			ATE_MAC_TX_CTS_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXFRAME(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PRTMP_TX_RING 	pTxRing = &pAd->TxRing[QID_AC_BE];
	PTXD_STRUC		pTxD = NULL;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
    TXD_STRUC       TxD;
#endif /* RT_BIG_ENDIAN */
#endif /* RTMP_MAC_PCI */
	UCHAR			BbpData = 0;
	STRING			IPGStr[8] = {0};
#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	UCHAR		RFValue, BBP49Value;
	CHAR		ChannelPower = pATEInfo->TxPower0;
	CHAR		*TssiRefPerChannel = pATEInfo->TssiRefPerChannel;
	UCHAR		CurrentChannel = pATEInfo->Channel;
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef RTMP_TEMPERATURE_COMPENSATION
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	if (pATEInfo->bTSSICalbrEnableG == TRUE)
	{
		if ((!IS_RT3350(pAd)) && (!IS_RT3352(pAd)))                  
		{
			DBGPRINT_ERR(("Not support TSSI calibration since not 3350/3352 chip!!!\n"));
			Status = NDIS_STATUS_FAILURE;

			return Status;
		}

		/* Internal TSSI 0 */
		RFValue = (0x3 | 0x0 << 2 | 0x3 << 4);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R27, RFValue);

		RFValue = (0x3 | 0x0 << 2);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R28, RFValue);

		/* set BBP R49[7] = 1 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
		BBP49Value = BbpData;
		BbpData |= 0x80;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpData);
	}
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_COMPENSATION
#endif /* RTMP_TEMPERATURE_COMPENSATION */
	
	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s(Count=%u)\n", __FUNCTION__, pATEInfo->TxCount));
	pATEInfo->Mode |= ATE_TXFRAME;

	if (pATEInfo->bQATxStart == FALSE)  
	{

		/* set IPG to sync tx power with QA tools */
		/* default value of IPG is 200 */
		snprintf(IPGStr, sizeof(IPGStr), "%u", pATEInfo->IPG);
		DBGPRINT(RT_DEBUG_TRACE, ("IPGstr=%s\n", IPGStr));
		Set_ATE_IPG_Proc(pAd, IPGStr);
	}

#ifdef RTMP_MAC_PCI
	/* Default value in BBP R22 is 0x0. */
	ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);


	if (!(IS_RT3883(pAd) || IS_RT3352(pAd) || IS_RT5350(pAd) || IS_RT3593(pAd) || IS_RT6352(pAd)))
	{
		/* Soft reset BBP. */
		BbpSoftReset(pAd);
	}

	/* clear bit4 to stop continuous Tx production test */
	ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Abort Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 0);

	{
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * RINGREG_DIFF,  &pTxRing->TxDmaIdx);
		pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
		pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pTxRing->TxCpuIdx);					
	}

	pATEInfo->TxDoneCount = 0;

	SetJapanFilter(pAd);
	
	for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pATEInfo->TxCount); ring_index++)
	{
		PNDIS_PACKET pPacket;
		UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
		pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
		pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
		TxD = *pDestTxD;
		pTxD = &TxD;
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* !RT_BIG_ENDIAN */
		/* Clear current cell. */
		pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNdisPacket = NULL;

		pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNextNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

		if (ATESetUpFrame(pAd, TxIdx) != 0)
			return NDIS_STATUS_FAILURE;

		INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);

	}

	ATESetUpFrame(pAd, pTxRing->TxCpuIdx);

	/* Start Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);

	/* Enable Tx */
	ATE_MAC_TX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
#endif /* RTMP_MAC_PCI */


#ifdef RALINK_QA
	/* add this for LoopBack mode */
	if (pATEInfo->bQARxStart == FALSE)  
	{
#ifdef TXBF_SUPPORT
		/* Enable Rx if Sending Sounding. Otherwise Disable */
		if (pATEInfo->txSoundingMode != 0)
		{
			ATE_MAC_RX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
		}
		else
#endif /* TXBF_SUPPORT */
		{
			/* Disable Rx */
			ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
		}
	}

	if (pATEInfo->bQATxStart == TRUE)  
	{
		pATEInfo->TxStatus = 1;
	}
#else
#ifdef TXBF_SUPPORT
	/* Enable Rx if Sending Sounding. Otherwise Disable */
	if (pATEInfo->txSoundingMode != 0)
	{
		ATE_MAC_RX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);
	}
	else
#endif /* TXBF_SUPPORT */
	{
		/* Disable Rx */
		ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);
	}
#endif /* RALINK_QA */

#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * RINGREG_DIFF, &pAd->TxRing[QID_AC_BE].TxDmaIdx);
	/* kick Tx Ring */
	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

	pAd->RalinkCounters.KickTxCount++;
#endif /* RTMP_MAC_PCI */


#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	if (pATEInfo->bTSSICalbrEnableG == TRUE)
	{
		if ((IS_RT3350(pAd)) || (IS_RT3352(pAd))) 
		{
			if ((pATEInfo->TxWI.MCS == 7)
				&& (pATEInfo->TxWI.BW == BW_20)	&& (pATEInfo->TxAntennaSel == 1))                  
			{
				if (pATEInfo->Channel == 7)
				{
					/* step 1: get calibrated channel 7 TSSI reading as reference */
					RtmpOsMsDelay(500);
					DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));

					/* read BBP R49[4:0] and write to EEPROM 0x6E */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
					DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%02x\n", BbpData)); 
					BbpData &= 0x1f;
					TssiRefPerChannel[CurrentChannel-1] = BbpData;
					DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%02x\n", TssiRefPerChannel[CurrentChannel-1]));  
				}

				/* step 2: calibrate channel 1 and 13 TSSI delta values */
				else if (pATEInfo->Channel == 1)
				{
					/* Channel 1 */
					RtmpOsMsDelay(500);
					DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));

					/* read BBP R49[4:0] */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
					DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%02x\n", BbpData)); 
					BbpData &= 0x1f;
					TssiRefPerChannel[CurrentChannel-1] = BbpData;
					DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%02x\n", TssiRefPerChannel[CurrentChannel-1]));
				}
				else if (pATEInfo->Channel == 13)
				{
					/* Channel 13 */
					RtmpOsMsDelay(500);
					DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));

					/* read BBP R49[4:0] */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
					DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%02x\n", BbpData)); 
					BbpData &= 0x1f;
					TssiRefPerChannel[CurrentChannel-1] = BbpData;
					DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%02x\n", TssiRefPerChannel[CurrentChannel-1]));
				}
				else
				{
					DBGPRINT(RT_DEBUG_OFF, ("Channel %d, Calibrated Tx.Power0= 0x%04x\n", CurrentChannel, ChannelPower));
				}
			}
		}
	}
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_COMPENSATION
#endif /* RTMP_TEMPERATURE_COMPENSATION */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS RXFRAME(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	/* Disable Rx of MAC block */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Default value in BBP R22 is 0x0. */
	ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);

#if defined (RT3883) || defined (RT3352) || defined (RT5350) 
	if (IS_RT3883(pAd) || IS_RT3352(pAd) || IS_RT5350(pAd)) 
	{
		if (pATEInfo->TxWI.BW == BW_20)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xC0); 
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0x00);
		}
	}
#endif /* defined (RT3883) || defined (RT3352) || defined (RT5350) */

	/* Clear bit4 to stop continuous Tx production test. */
	ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	pATEInfo->Mode |= ATE_RXFRAME;


	/* Disable Tx of MAC block. */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);


	/* Enable Rx of MAC block. */
	ATE_MAC_RX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);


#ifdef RT6352
	if (pATEInfo->bQAEnabled == FALSE)
	{
		if (IS_RT6352(pAd) && (pAd->CommonCfg.Chip_VerID > 1) && (pAd->CommonCfg.Chip_E_Number == 2))
		{
			UCHAR 	RFValue;

			if (pATEInfo->RxAntennaSel != 0) /* 1R */
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, 0x08);
			}
			else
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, 0x48);
			}

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R28, &RFValue);
			if ((pATEInfo->Channel == 11) && (pATEInfo->TxWI.BW == BW_40))
				RFValue |= 0x4;
			else
				RFValue &= (~0x4);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R28, RFValue);
		}
	}
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


/*
==========================================================================
    Description:
        Set ATE operation mode to
        0. ATESTART  = Start/Reset ATE Mode
        1. ATESTOP   = Stop ATE Mode
        2. TXCARR    = Transmit Carrier
        3. TXCONT    = Continuous Transmit
        4. TXFRAME   = Transmit Frames
        5. RXFRAME   = Receive Frames
#ifdef RALINK_QA
        6. TXSTOP    = Stop Any Type of Transmition
        7. RXSTOP    = Stop Receiving Frames        
#endif

    Return:
        NDIS_STATUS_SUCCESS if all parameters are OK.
==========================================================================
*/
static NDIS_STATUS	ATECmdHandler(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s\n", __FUNCTION__));

#ifdef CONFIG_RT2880_ATE_CMD_NEW
	if (!strcmp(arg, "ATESTART")) 		
	{
		/* Enter/Reset ATE mode and set Tx/Rx Idle */
		Status = ATESTART(pAd);
	}
	else if (!strcmp(arg, "ATESTOP")) 
	{
		/* Leave ATE mode */
		Status = ATESTOP(pAd);
	}
#else
	if (!strcmp(arg, "APSTOP")) 		
	{
		Status = ATESTART(pAd);
	}
	else if (!strcmp(arg, "APSTART")) 
	{
		Status = ATESTOP(pAd);
	}
#endif
	else if (!strcmp(arg, "TXCARR"))	
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			/* Set ATE Mode in advance for RT6352ATEAsicSetTxRxPath() */
			pAd->ate.Mode &= ATE_RXSTOP;
			pAd->ate.Mode |= ATE_TXCARR;
		}
#endif /* RT6352 */
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pATEInfo->Channel);
		RtmpOsMsDelay(5);

		Status = TXCARR(pAd);
	}
	else if (!strcmp(arg, "TXCARS"))
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			/* Set ATE Mode in advance for RT6352ATEAsicSetTxRxPath() */
			pAd->ate.Mode &= ATE_RXSTOP;
			pAd->ate.Mode |= ATE_TXCARRSUPP;
		}
#endif /* RT6352 */
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pATEInfo->Channel);
		RtmpOsMsDelay(5);

		Status = TXCARS(pAd);
	}
	else if (!strcmp(arg, "TXCONT"))	
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			/* Set ATE Mode in advance for RT6352ATEAsicSetTxRxPath() */
			pAd->ate.Mode &= ATE_RXSTOP;
			pAd->ate.Mode |= ATE_TXCONT;
		}
#endif /* RT6352 */
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pATEInfo->Channel);
		RtmpOsMsDelay(5);

		Status = TXCONT(pAd);
	}
	else if (!strcmp(arg, "TXFRAME")) 
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			/* Set ATE Mode in advance for RT6352ATEAsicSetTxRxPath() */
			pAd->ate.Mode &= ATE_RXSTOP;
			pAd->ate.Mode |= ATE_TXFRAME;
		}
#endif /* RT6352 */
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pATEInfo->Channel);
		RtmpOsMsDelay(5);

		Status = TXFRAME(pAd);
	}
#ifdef RT6352
	else if (!strcmp(arg, "TXAPPLY")) 
	{
		if (IS_RT6352(pAd))
		{
			/* Set ATE Mode in advance for RT6352ATEAsicSetTxRxPath() */
			pAd->ate.Mode &= ATE_RXSTOP;
			pAd->ate.Mode |= ATE_TXFRAME;
		}
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pATEInfo->Channel);
		RtmpOsMsDelay(5);
	}
	else if (!strcmp(arg, "RXAPPLY")) 
	{
		if (IS_RT6352(pAd))
		{
			/* Set ATE Mode in advance for RT6352ATEAsicSetTxRxPath() */
			pAd->ate.Mode &= ATE_TXSTOP;
			pAd->ate.Mode |= ATE_RXFRAME;
		}
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pATEInfo->Channel);
		RTMPusecDelay(5);
	}
#endif /* RT6352 */
	else if (!strcmp(arg, "RXFRAME")) 
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			/* Set ATE Mode in advance for RT6352ATEAsicSetTxRxPath() */
			pAd->ate.Mode &= ATE_TXSTOP;
			pAd->ate.Mode |= ATE_RXFRAME;
		}
#endif /* RT6352 */
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pATEInfo->Channel);
		RTMPusecDelay(5);

		Status = RXFRAME(pAd);
	}
#ifdef RALINK_QA
	/* Enter ATE mode and set Tx/Rx Idle */
	else if (!strcmp(arg, "TXSTOP"))
	{
		Status = TXSTOP(pAd);
	}
	else if (!strcmp(arg, "RXSTOP"))
	{
		Status = RXSTOP(pAd);
	}
#endif /* RALINK_QA */
	else
	{	
		DBGPRINT_ERR(("ATE : Invalid arg in %s!\n", __FUNCTION__));
		Status = NDIS_STATUS_INVALID_DATA;
	}
	RtmpOsMsDelay(5);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s\n", __FUNCTION__));
	return Status;
}


INT	Set_ATE_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	/* Handle ATEACTIVE and ATEPASSIVE commands as a special case */
	if (!strcmp(arg, "ATEACTIVE"))
	{
		pATEInfo->PassiveMode = FALSE;
		return TRUE;
	}

	if (!strcmp(arg, "ATEPASSIVE"))
	{
		pATEInfo->PassiveMode = TRUE;
		return TRUE;
	}

	/* Disallow all other ATE commands in passive mode */
	if (pATEInfo->PassiveMode)
		return TRUE;

	if (ATECmdHandler(pAd, arg) == NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
		return TRUE;
	}
	else
	{
		DBGPRINT_ERR(("Set_ATE_Proc Failed\n"));
		return FALSE;
	}
}


/* 
==========================================================================
    Description:
        Set ATE ADDR1=DA for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR3=DA for TxFrame(STA : To DS = 1 ; From DS = 0)        
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_DA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	PSTRING				value;
	INT					octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */	
	if (strlen(arg) != 17)  
		return FALSE;

	for (octet = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pATEInfo->Addr1[octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pATEInfo->Addr3[octet++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (octet != MAC_ADDR_LEN)
	{
		return FALSE;  
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pATEInfo->Addr1[0], pATEInfo->Addr1[1], pATEInfo->Addr1[2], pATEInfo->Addr1[3],
		pATEInfo->Addr1[4], pATEInfo->Addr1[5]));

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pATEInfo->Addr3[0], pATEInfo->Addr3[1], pATEInfo->Addr3[2], pATEInfo->Addr3[3],
		pATEInfo->Addr3[4], pATEInfo->Addr3[5]));
#endif /* CONFIG_STA_SUPPORT */
	
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_DA_Proc Success\n"));
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE ADDR3=SA for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR2=SA for TxFrame(STA : To DS = 1 ; From DS = 0)
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_SA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	PSTRING				value;
	INT					octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */	
	if (strlen(arg) != 17)  
		return FALSE;

	for (octet=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pATEInfo->Addr3[octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pATEInfo->Addr2[octet++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pATEInfo->Addr3[0], pATEInfo->Addr3[1], pATEInfo->Addr3[2], pATEInfo->Addr3[3],
		pATEInfo->Addr3[4], pATEInfo->Addr3[5]));
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pATEInfo->Addr2[0], pATEInfo->Addr2[1], pATEInfo->Addr2[2], pATEInfo->Addr2[3],
		pATEInfo->Addr2[4], pATEInfo->Addr2[5]));
#endif /* CONFIG_STA_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_SA_Proc Success\n"));

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE ADDR2=BSSID for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR1=BSSID for TxFrame(STA : To DS = 1 ; From DS = 0)

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_BSSID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	PSTRING				value;
	INT					octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */	
	if (strlen(arg) != 17)  
		return FALSE;

	for (octet=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pATEInfo->Addr2[octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pATEInfo->Addr1[octet++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n",	
		pATEInfo->Addr2[0], pATEInfo->Addr2[1], pATEInfo->Addr2[2], pATEInfo->Addr2[3],
		pATEInfo->Addr2[4], pATEInfo->Addr2[5]));

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n",	
		pATEInfo->Addr1[0], pATEInfo->Addr1[1], pATEInfo->Addr1[2], pATEInfo->Addr1[3],
		pATEInfo->Addr1[4], pATEInfo->Addr1[5]));
#endif /* CONFIG_STA_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_BSSID_Proc Success\n"));

	return TRUE;
}




/* 
==========================================================================
    Description:
        Set ATE Tx Channel

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_CHANNEL_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR channel;
	

	channel = simple_strtol(arg, 0, 10);

	/* to allow A band channel : ((channel < 1) || (channel > 14)) */
	if ((channel < 1) || (channel > 216))
	{
		DBGPRINT_ERR(("Set_ATE_CHANNEL_Proc::Out of range, it should be in range of 1~14.\n"));
		return FALSE;
	}

	pATEInfo->Channel = channel;


	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_CHANNEL_Proc (ATE Channel = %d)\n", pATEInfo->Channel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_CHANNEL_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Initialize the channel - set the power and switch to selected channel
			0 => use current value
			else set channel to specified channel
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_INIT_CHAN_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	int index;
	int value;

	/* Get channel parameter */
	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>216)
	{
		DBGPRINT_ERR(("Set_ATE_INIT_CHAN_Proc::Channel out of range\n"));
		return FALSE;
	}

	if (value != 0)
		pATEInfo->Channel = value;

	for (index=0; index<MAX_NUM_OF_CHANNELS; index++)
	{
		if (pATEInfo->Channel == pAd->TxPower[index].Channel)
		{
			pATEInfo->TxPower0 = pAd->TxPower[index].Power;
			pATEInfo->TxPower1 = pAd->TxPower[index].Power2;
#ifdef DOT11N_SS3_SUPPORT
			if (IS_RT2883(pAd) || IS_RT3593(pAd) || IS_RT3883(pAd))
				pATEInfo->TxPower2 = pAd->TxPower[index].Power3;
#endif /* DOT11N_SS3_SUPPORT */
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT_ERR(("Set_ATE_INIT_CHAN_Proc::Channel not found\n"));
		return FALSE;
	}

	/* Force non-QATool mode */
	pATEInfo->bQATxStart = pATEInfo->bQARxStart = FALSE;

	ATETxPwrHandler(pAd, 0);
	ATETxPwrHandler(pAd, 1);
#ifdef DOT11N_SS3_SUPPORT
	ATETxPwrHandler(pAd, 2);
#endif /* DOT11N_SS3_SUPPORT */

#if defined(RT2883) || defined(RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		ATEAsicSwitchChannel(pAd);
	}
#endif /* defined(RT2883) || defined(RT3883) */

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_INIT_CHAN_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx Power
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
static INT ATESetAntennaTxPower(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING 		arg,	
	IN  INT 		Antenna)

{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR TxPower;
	INT  index, maximun_index;

	pATEInfo = &(pAd->ate);
	TxPower = simple_strtol(arg, 0, 10);
	index = Antenna;
	maximun_index = pAd->Antenna.field.TxPath - 1;

	if ((index < 0) || (index > maximun_index))
	{
		DBGPRINT_ERR(("No such antenna! The range is 0~%d.\n", maximun_index));
		return FALSE;
	}

	if (pATEInfo->Channel <= 14) /* 2.4 GHz */
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			if ((TxPower > 47 /* 0x2F */) || (TxPower < 0))
			{
				DBGPRINT_ERR(("Set_ATE_TX_POWER%d_Proc::Out of range! (Value=%d)\n", index, TxPower));
				DBGPRINT_ERR(("TxPower range is 0~47 in G band.\n"));
				return FALSE;
			}
		}
		else
#endif /* RT6352 */
		if (!IS_RT3390(pAd))
		{
			if ((TxPower > 31) || (TxPower < 0))
			{
				DBGPRINT_ERR(("Set_ATE_TX_POWER%d_Proc::Out of range! (Value=%d)\n", index, TxPower));
				DBGPRINT_ERR(("TxPower range is 0~31 in G band.\n"));
				return FALSE;
			}
		}
	}
	else /* 5.5 GHz */
	{
		if ((TxPower > (pATEInfo->MaxTxPowerBandA)) || (TxPower < (pATEInfo->MinTxPowerBandA)))
		{
			DBGPRINT_ERR(("Set_ATE_TX_POWER%d_Proc::Out of range! (Value=%d)\n", index, TxPower));
			DBGPRINT_ERR(("TxPower range is %d~%d in A band.\n", pATEInfo->MinTxPowerBandA, pATEInfo->MaxTxPowerBandA));
			return FALSE;
		}
	}

	switch (index)
	{
		case 0:
			pATEInfo->TxPower0 = TxPower;
			break;
		case 1:
			pATEInfo->TxPower1 = TxPower;
			break;
#ifdef DOT11N_SS3_SUPPORT
		case 2:
			pATEInfo->TxPower2 = TxPower;
			break;	
#endif /* DOT11N_SS3_SUPPORT */
		default: 
			return FALSE;	
	}

	ATETxPwrHandler(pAd, index);

#if defined(RT2883) || defined(RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		ATEAsicSwitchChannel(pAd);
	}
#endif /* defined(RT2883) || defined(RT3883) */

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_POWER%d_Proc Success\n", index));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx Power0
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_POWER0_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT ret;
	
	ret = ATESetAntennaTxPower(pAd, arg, 0);
	return ret;
}


/* 
==========================================================================
    Description:
        Set ATE Tx Power1
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_POWER1_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT ret;
	
	ret = ATESetAntennaTxPower(pAd, arg, 1);
	return ret;
}


#ifdef DOT11N_SS3_SUPPORT
/* 
==========================================================================
    Description:
        Set ATE Tx Power2
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_POWER2_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT ret;
	
	ret = ATESetAntennaTxPower(pAd, arg, 2);
	return ret;
}
#endif /* DOT11N_SS3_SUPPORT */


/* 
==========================================================================
    Description:
        Set ATE Tx Antenna
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR value;
	INT maximun_index = pAd->Antenna.field.TxPath;
	
	value = simple_strtol(arg, 0, 10);

	if ((value > maximun_index) || (value < 0))
	{
		DBGPRINT_ERR(("Set_ATE_TX_Antenna_Proc::Out of range (Value=%d)\n", value));
		DBGPRINT_ERR(("Set_ATE_TX_Antenna_Proc::The range is 0~%d\n", maximun_index));

		return FALSE;
	}

	pATEInfo->TxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_Antenna_Proc (Antenna = %d)\n", pATEInfo->TxAntennaSel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_Antenna_Proc Success\n"));

	/* calibration power unbalance issues */
	ATEAsicSwitchChannel(pAd);

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Rx Antenna
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_RX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR value;
	INT maximun_index = pAd->Antenna.field.RxPath;
	
	value = simple_strtol(arg, 0, 10);

	if ((value > maximun_index) || (value < 0))
	{
		DBGPRINT_ERR(("Set_ATE_RX_Antenna_Proc::Out of range (Value=%d)\n", value));
		DBGPRINT_ERR(("Set_ATE_RX_Antenna_Proc::The range is 0~%d\n", maximun_index));

		return FALSE;
	}

	pATEInfo->RxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_Antenna_Proc (Antenna = %d)\n", pATEInfo->RxAntennaSel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_Antenna_Proc Success\n"));

	/* calibration power unbalance issues */
	ATEAsicSwitchChannel(pAd);

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


VOID DefaultATEAsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg8 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	/* For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 */
	RTMP_IO_READ32(pAd, 0x1318, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, 0x131C, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) << 8; /* Get Tx power for HT MCS 7 */			
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

	/* For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1324, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);

	if (IS_RT5392(pAd))
	{	
		/*  For HT_MCS_15, extra fill the corresponding register value into MAC 0x13DC */
		RTMP_IO_READ32(pAd, 0x1320, &ExtraPwrOverMAC);  
		ExtraPwrOverTxPwrCfg8 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for HT MCS 15 */
		RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, ExtraPwrOverTxPwrCfg8);
		
		DBGPRINT(RT_DEBUG_TRACE, ("Offset =0x13D8, TxPwr = 0x%08X, ", (UINT)ExtraPwrOverTxPwrCfg8));
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}


VOID ATEAsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->AsicExtraPowerOverMAC!= NULL)
		pATEInfo->pChipStruct->AsicExtraPowerOverMAC(pAd);

	return;
}


#ifdef RT3350
/* 
==========================================================================
    Description:
        Set ATE PA bias to improve EVM
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT Set_ATE_PA_Bias_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR PABias = 0;
	UCHAR RFValue;
	
	PABias = simple_strtol(arg, 0, 10);

	if (PABias >= 16)
	{
		DBGPRINT_ERR(("Set_ATE_PA_Bias_Proc::Out of range, it should be in range of 0~15.\n"));
		return FALSE;
	}

	pATEInfo->PABias = PABias;

#ifdef RT6352
// TODO:
/*	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R19, (PUCHAR)&RFValue); */
#else
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R19, (PUCHAR)&RFValue);
#endif
	RFValue = (((RFValue & 0x0F) | (pATEInfo->PABias << 4)));
#ifdef RT6352
// TODO:
/*	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R19, (UCHAR)RFValue); */
#else
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R19, (UCHAR)RFValue);
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_PA_Bias_Proc (PABias = %d)\n", pATEInfo->PABias));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_PA_Bias_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
#endif /* RT3350 */


/* 
==========================================================================
    Description:
        Set ATE RF BW(default)
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Default_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
#ifdef RTMP_RF_RW_SUPPORT
#ifndef RF_BANK
	UCHAR RFValue = 0;
#endif /* !RF_BANK */
#endif /* RTMP_RF_RW_SUPPORT */

	
	RFFreqOffset = simple_strtol(arg, 0, 10);

#ifdef RTMP_RF_RW_SUPPORT
	/* 2008/08/06: KH modified the limit of offset value from 64 to 96(0x5F + 0x01) */
#ifdef RT6352
	if (!IS_RT6352(pAd))
#endif /* RT6352 */
	{
		if (RFFreqOffset >= 96)
		{
			DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 95).\n"));
			return FALSE;
		}
	}
#else
	if (RFFreqOffset >= 64)
	{
		DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 63).\n"));
		return FALSE;
	}
#endif /* RTMP_RF_RW_SUPPORT */

	pATEInfo->RFFreqOffset = RFFreqOffset;

#ifdef RTMP_RF_RW_SUPPORT
#ifndef RF_BANK
	if (IS_RT30xx(pAd))
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
		RFValue = ((RFValue & 0x80) | pATEInfo->RFFreqOffset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);
	}
#endif /* !RF_BANK */
	if (IS_RT3593(pAd))
	{
		ATEAsicSwitchChannel(pAd);
	}

#endif /* RTMP_RF_RW_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_FREQOFFSET_Proc (RFFreqOffset = %d)\n", pATEInfo->RFFreqOffset));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_FREQOFFSET_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE RF frequence offset
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT ret = FALSE;

	if (pATEInfo->pChipStruct->Set_FREQ_OFFSET_Proc != NULL)
	{
		ret = pATEInfo->pChipStruct->Set_FREQ_OFFSET_Proc(pAd, arg);
	}

	if (ret == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_FREQ_OFFSET_Proc (RFFreqOffset = %d)\n", pATEInfo->RFFreqOffset));
		DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_FREQ_OFFSET_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	}

	return ret;
}


/* 
==========================================================================
    Description:
        Set ATE RF BW(default)
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Default_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if ((BBPCurrentBW == 0)
		)
	{
		pATEInfo->TxWI.BW = BW_20;
	}
	else
	{
		pATEInfo->TxWI.BW = BW_40;
 	}

	if ((pATEInfo->TxWI.PHYMODE == MODE_CCK) && (pATEInfo->TxWI.BW == BW_40))
	{
		DBGPRINT_ERR(("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT_ERR(("Bandwidth switch to 20!!\n"));
		pATEInfo->TxWI.BW = BW_20;
	}

	if (pATEInfo->TxWI.BW == BW_20)
	{
		if (pATEInfo->Channel <= 14)
		{
			/* BW=20;G band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgGBand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}
		}
		else
		{
			/* BW=20;A band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
 				RtmpOsMsDelay(5);				
 			}
		}

		/* set BW = 20 MHz */
		/* Set BBP R4 bit[4:3]=0:0 */
 		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
 		value &= (~0x18);
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		/* Set BBP R66=0x3C */
		value = 0x3C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);

		/* set BW = 20 MHz */
		{
			pAd->LatchRfRegs.R4 &= ~0x00200000;
			RtmpRfIoWrite(pAd);
		}

		/* BW = 20 MHz */
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x16 */
		value = 0x16;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x08 */
		value = 0x08;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x11 */
	        if ( IS_RT5390(pAd) || IS_RT5392(pAd))
		    value = 0x13;
	        else
		    value = 0x11;

 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);


		if (pATEInfo->Channel == 14)
		{
			INT TxMode = pATEInfo->TxWI.PHYMODE;

			if (TxMode == MODE_CCK)
			{
				/* when Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1 */
 				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
				value |= 0x20; /* set bit5=1 */
 				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);				
			}
		}
	}
	/* If bandwidth = 40M, set RF Reg4 bit 21 = 0. */
	else if (pATEInfo->TxWI.BW == BW_40)
	{
		if (pATEInfo->Channel <= 14)
		{
			/* BW=40;G band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgGBand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}
		}
		else
		{
			/* BW=40;A band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}		

			if ((pATEInfo->TxWI.PHYMODE >= 2) && (pATEInfo->TxWI.MCS == 7))
			{
    			value = 0x28;
    			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, value);
			}
		}

		/* Set BBP R4 bit[4:3]=1:0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		value |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		/* Set BBP R66=0x3C */
		value = 0x3C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);


		/* Set BBP R68=0x0C to improve Rx sensitivity. */
		value = 0x0C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x1A */
		value = 0x1A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x0A */
		value = 0x0A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x16 */
	        if (IS_RT5390(pAd) || IS_RT5392(pAd))
		    value = 0x13;
	        else
		    value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pATEInfo->TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->Set_BW_Proc != NULL)
		pATEInfo->pChipStruct->Set_BW_Proc(pAd, arg);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pATEInfo->TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame length
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_LENGTH_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	pATEInfo->TxLength = simple_strtol(arg, 0, 10);

	if ((pATEInfo->TxLength < 24) || (pATEInfo->TxLength > (MAX_FRAME_SIZE - 34/* == 2312 */)))
	{
		pATEInfo->TxLength = (MAX_FRAME_SIZE - 34/* == 2312 */);
		DBGPRINT_ERR(("Set_ATE_TX_LENGTH_Proc::Out of range, it should be in range of 24~%d.\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_LENGTH_Proc (TxLength = %d)\n", pATEInfo->TxLength));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_LENGTH_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame count
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_COUNT_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	pATEInfo->TxCount = simple_strtol(arg, 0, 10);

#ifdef RTMP_MAC_PCI
	if (pATEInfo->TxCount == 0)
	{
		pATEInfo->TxCount = 0xFFFFFFFF;
	}
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pATEInfo->TxCount));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_COUNT_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame MCS
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_MCS_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR MCS;
	INT result;

	MCS = simple_strtol(arg, 0, 10);
	result = CheckMCSValid(pAd, pATEInfo->TxWI.PHYMODE, MCS);

	if (result != -1)
	{
		pATEInfo->TxWI.MCS = (UCHAR)MCS;
	}
	else
	{
		DBGPRINT_ERR(("Set_ATE_TX_MCS_Proc::Out of range, refer to rate table.\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MCS_Proc (MCS = %d)\n", pATEInfo->TxWI.MCS));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MCS_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame STBC
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_STBC_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	pATEInfo->TxWI.STBC = simple_strtol(arg, 0, 10);

	if (pATEInfo->TxWI.STBC > 1)
	{
		pATEInfo->TxWI.STBC = 0;
		DBGPRINT_ERR(("Set_ATE_TX_STBC_Proc::Out of range\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_STBC_Proc (GI = %d)\n", pATEInfo->TxWI.STBC));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_STBC_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame Mode
        0: MODE_CCK
        1: MODE_OFDM
        2: MODE_HTMIX
        3: MODE_HTGREENFIELD
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_MODE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR BbpData = 0;

	pATEInfo->TxWI.PHYMODE = simple_strtol(arg, 0, 10);

	if (pATEInfo->TxWI.PHYMODE > 3)
	{
		pATEInfo->TxWI.PHYMODE = 0;
		DBGPRINT_ERR(("Set_ATE_TX_MODE_Proc::Out of range.\nIt should be in range of 0~3\n"));
		DBGPRINT(RT_DEBUG_OFF, ("0: CCK, 1: OFDM, 2: HT_MIX, 3: HT_GREEN_FIELD.\n"));
		return FALSE;
	}

	/* Turn on BBP 20MHz mode by request here. */
	if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);
		BbpData &= (~0x18);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpData);
		pATEInfo->TxWI.BW = BW_20;
	}

#ifdef RT3350
	if (IS_RT3350(pAd))
	{
		if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
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
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);
		
			RT28xx_EEPROM_READ16(pAd, 0x12a, value);
			rf_value = value & 0x00FF;
			rf_offset = (value & 0xFF00) >> 8;

			if(rf_offset == 0xff)
			    rf_offset = RF_R29;
			if(rf_value == 0xff)
			    rf_value = 0x07;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);
		

			/* set RF_R24 */
			if (pATEInfo->TxWI.BW == BW_40)
			{    
				value = 0x3F;
			}
			else
			{
				value = 0x1F;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
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
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);
		
			RT28xx_EEPROM_READ16(pAd, 0x128, value);
			rf_value = value & 0x00FF;
			rf_offset = (value & 0xFF00) >> 8;

			if(rf_offset == 0xff)
			    rf_offset = RF_R29;
			if(rf_value == 0xff)
			    rf_value = 0x07;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, rf_offset, (UCHAR)rf_value);
		
			/* set RF_R24 */
			if (pATEInfo->TxWI.BW == BW_40)
			{    
				value = 0x28;
			}
			else
			{
				value = 0x18;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
		}
	}
#endif /* RT3350 */

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MODE_Proc (TxMode = %d)\n", pATEInfo->TxWI.PHYMODE));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MODE_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame GI
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_GI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	pATEInfo->TxWI.ShortGI = simple_strtol(arg, 0, 10);

	if (pATEInfo->TxWI.ShortGI > 1)
	{
		pATEInfo->TxWI.ShortGI = 0;
		DBGPRINT_ERR(("Set_ATE_TX_GI_Proc::Out of range\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_GI_Proc (GI = %d)\n", pATEInfo->TxWI.ShortGI));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_GI_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


INT	Set_ATE_RX_FER_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	pATEInfo->bRxFER = simple_strtol(arg, 0, 10);

	if (pATEInfo->bRxFER == 1)
	{
		pATEInfo->RxCntPerSec = 0;
		pATEInfo->RxTotalCnt = 0;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_FER_Proc (bRxFER = %d)\n", pATEInfo->bRxFER));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_FER_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


INT Set_ATE_Read_RF_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef RTMP_RF_RW_SUPPORT
#ifndef RF_BANK
	UCHAR RFValue;
	INT index=0;

	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		for (index = 0; index < 32; index++)
		{
			ATE_RF_IO_READ8_BY_REG_ID(pAd, index, (PUCHAR)&RFValue);
			DBGPRINT(RT_DEBUG_OFF, ("R%d=%d\n",index,RFValue));
		}		
	}
	else
#ifdef RT305x
	if (IS_RT305x(pAd))
	{
		for (index = 0; index < 32; index++)
		{
			ATE_RF_IO_READ8_BY_REG_ID(pAd, index, (PUCHAR)&RFValue);
			DBGPRINT(RT_DEBUG_OFF, ("R%d=%d\n",index,RFValue));
		}		
	}
	else
#endif /* RT305x */
#endif /* !RF_BANK */
#endif /* RTMP_RF_RW_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_OFF, ("R1 = %x\n", pAd->LatchRfRegs.R1));
		DBGPRINT(RT_DEBUG_OFF, ("R2 = %x\n", pAd->LatchRfRegs.R2));
		DBGPRINT(RT_DEBUG_OFF, ("R3 = %x\n", pAd->LatchRfRegs.R3));
		DBGPRINT(RT_DEBUG_OFF, ("R4 = %x\n", pAd->LatchRfRegs.R4));
	}
	return TRUE;
}


#ifndef RTMP_RF_RW_SUPPORT
INT Set_ATE_Write_RF1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);	

	pAd->LatchRfRegs.R1 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}


INT Set_ATE_Write_RF2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

	pAd->LatchRfRegs.R2 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}


INT Set_ATE_Write_RF3_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

	pAd->LatchRfRegs.R3 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}


INT Set_ATE_Write_RF4_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

	pAd->LatchRfRegs.R4 = value;
	RtmpRfIoWrite(pAd);

	return TRUE;
}
#endif /* !RTMP_RF_RW_SUPPORT */


/* 
==========================================================================
    Description:
        Load and Write EEPROM from a binary file prepared in advance.
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT Set_ATE_Load_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN		    	ret = FALSE;
#ifdef RTMP_RBUS_SUPPORT
	PSTRING			src = EEPROM_DEFAULT_FILE_PATH;
#else
	PSTRING			src = EEPROM_BIN_FILE_NAME;
#endif /* RTMP_RBUS_SUPPORT */
	RTMP_OS_FD		srcf;
	INT32 			retval;
	USHORT 			WriteEEPROM[(EEPROM_SIZE >> 1)];
	INT				FileLength = 0;
	UINT32 			value = (UINT32) simple_strtol(arg, 0, 10);
	RTMP_OS_FS_INFO	osFSInfo;

	DBGPRINT(RT_DEBUG_OFF, ("===> %s (value=%d)\n\n", __FUNCTION__, value));

	if (value > 0)
	{
		/* zero the e2p buffer */
		NdisZeroMemory((PUCHAR)WriteEEPROM, EEPROM_SIZE);

		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		do
		{
			/* open the bin file */
			srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(srcf)) 
			{
				DBGPRINT_ERR(("%s - Error opening file %s\n", __FUNCTION__, src));
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(srcf, (PSTRING)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE)
			{
				DBGPRINT_ERR(("%s : error file length (=%d) in e2p.bin\n",
					   __FUNCTION__, FileLength));
				break;
			}
			else
			{
				/* write the content of .bin file to EEPROM */
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)
                {
                    USHORT index=0;
                    USHORT value=0;
                    INT32 e2p_size=512;/* == 0x200 for PCI interface */
                    USHORT tempData=0;
					
                    for (index = 0 ; index < (e2p_size >> 1); )
                    {
                        /* "value" is especially for some compilers... */
                        tempData = le2cpu16(WriteEEPROM[index]);
                        value = tempData;
                        RT28xx_EEPROM_WRITE16(pAd, (index << 1), value);
                        index ++;
                    }
                }
#else
				rt_ee_write_all(pAd, WriteEEPROM);
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
				ret = TRUE;
			}
			break;
		} while(TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(srcf))
		{
			;
		}
		else
		{
			retval = RtmpOSFileClose(srcf);			

			if (retval)
			{
				DBGPRINT_ERR(("--> Error %d closing %s\n", -retval, src));
				
			} 
		}

		/* restore */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);		
	}

    DBGPRINT(RT_DEBUG_OFF, ("<=== %s (ret=%d)\n", __FUNCTION__, ret));

    return ret;
}


INT Set_ATE_Read_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT buffer[EEPROM_SIZE >> 1];
	USHORT *p;
	int i;
	
	rt_ee_read_all(pAd, (USHORT *)buffer);
	p = buffer;
	for (i = 0; i < (EEPROM_SIZE >> 1); i++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", *p));
		if (((i+1) % 16) == 0)
			DBGPRINT(RT_DEBUG_OFF, ("\n"));
		p++;
	}
	return TRUE;
}


#ifdef LED_CONTROL_SUPPORT
#endif /* LED_CONTROL_SUPPORT */

/* 
==========================================================================
    Description:
        Enable ATE auto Tx alc (Tx auto level control).
        According to the chip temperature, auto adjust the transmit power.  
        
        0: disable
        1: enable
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_AUTO_ALC_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 value = simple_strtol(arg, 0, 10);
	UINT32 MacValue;

	if (value > 0)
	{
#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
#ifdef RT6352_EP_SUPPORT
			if (pAd->bExtPA == FALSE)
			{
#endif /* RT6352_EP_SUPPORT */
				OS_SEM_LOCK(&(pATEInfo->TssiSemLock));
				RT635xTssiDcCalibration(pAd);
				OS_SEM_UNLOCK(&(pATEInfo->TssiSemLock));	
#ifdef RT6352_EP_SUPPORT
			}
#endif /* RT6352_EP_SUPPORT */
		}
#endif /* RT6352 */
#endif /* RTMP_INTERNAL_TX_ALC */
		pATEInfo->bAutoTxAlc = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = TRUE , auto alc enabled!\n"));
	}
	else
	{
		pATEInfo->bAutoTxAlc = FALSE;

		if (IS_RT6352(pAd))
		{
			/* clean up MAC 0x13B4 */
			RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);	
			MacValue = MacValue & (~0x3f);
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);
		}

		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = FALSE , auto alc disabled!\n"));
	}	

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/* 
==========================================================================
    Description:
        Enable Tx temperature sensor.
        
        0: disable
        1: enable
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TEMP_SENSOR_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	BOOLEAN value = simple_strtol(arg, 0, 10);
	UINT32 MacValue;

	if (value > 0)
	{
		pATEInfo->bLowTemperature = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATETEMPSENSOR = TRUE , temperature sensor enabled!\n"));
	}
	else
	{
		pATEInfo->bLowTemperature = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATETEMPSENSOR = FALSE , temperature sensor disabled!\n"));
	}	

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


#ifdef TXBF_SUPPORT
/* 
==========================================================================
    Description:
        Set ATE Tx Beamforming mode
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TXBF_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR value;
	
	value = simple_strtol(arg, 0, 10);

	switch (value)
	{
		case 0:
			/* no BF */
			pATEInfo->TxWI.iTxBF = pATEInfo->TxWI.eTxBF = 0;
			break;
		case 1:
			/* ETxBF */
			pATEInfo->TxWI.eTxBF = 1;
			break;
		case 2:
			/* ITxBF */
			pATEInfo->TxWI.iTxBF = 1;
			break;
		case 3:
			/* Enable TXBF support */
		pATEInfo->bTxBF = TRUE;
			break;
		case 4:
			/* Disable TXBF support */
			pATEInfo->bTxBF = FALSE;
			break;
		default:
			DBGPRINT_ERR(("Set_ATE_TXBF_Proc: Invalid parameter %d\n", value));
			break;
	}

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
	}


/* 
==========================================================================
    Description:
        Set ATE Sounding type
			0 => no sounding
			1 => Data sounding
			2 => 2 stream NDP sounding
			3 => 3 stream NDP Sounding
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TXSOUNDING_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR value;
	
	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>3)
	{
		DBGPRINT_ERR(("Set_ATE_TXSOUNDING_Proc: Invalid parameter %d\n", value));
		return FALSE;
	}	

	pATEInfo->txSoundingMode = value;

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/* 
==========================================================================
    Description:
        Do a Divider Calibration on calibration channels and save in EEPROM
			0 => do G and A band
			1 => G band only
			2 => A band only

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TXBF_DIVCAL_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	int value;
	ITXBF_DIV_PARAMS divParams;
	CHAR initChanArg[] = "0";

	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>2)
		return FALSE;

	/* G band */
	if (value==0 || value==1)
	{
		pATEInfo->Channel = 1;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);

		pATEInfo->Channel = 14;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);

		/* Display delta phase information */
		ITxBFGetEEPROM(pAd, NULL, NULL, &divParams);

		DBGPRINT(RT_DEBUG_WARN, ("Divider Cal Done:\n"
						"ch1-ch14 = [%2d, %2d] degrees\n"
						"ant0-ant2 = [%2d, %2d] degrees\n",
				(UCHAR)(divParams.gBeg[0]-divParams.gEnd[0])*360/256,
				(UCHAR)(divParams.gBeg[1]-divParams.gEnd[1])*360/256,
				(UCHAR)(divParams.gBeg[0]-divParams.gBeg[1])*360/256,
				(UCHAR)(divParams.gEnd[0]-divParams.gEnd[1])*360/256) );
	}

	/* A Band */
	if (value==0 || value==2)
	{
		pATEInfo->Channel = 36;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);

		pATEInfo->Channel = 120;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);

		pATEInfo->Channel = 165;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);
	}

	return TRUE;
}


/*
==========================================================================
    Description:
        Do a LNA Calibration on calibration channels and save in EEPROM
			0 => do G and A band
			1 => G band only
			2 => A band only

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TXBF_LNACAL_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	int value;
	int i;
	CHAR initChanArg[] = "0";

	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>2)
		return FALSE;

	/* G Band */
	if (value==0 || value==1)
	{
		pATEInfo->Channel = 1;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFLNACalibration(pAd, 1, 0, TRUE);

		pATEInfo->Channel = 14;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFLNACalibration(pAd, 1, 0, TRUE);
	}

	/* A Band */
	if (value==0 || value==2)
	{
		static UCHAR channels[6] = {36, 64, 100, 128, 132, 165};
		for (i=0; i<6; i++)
		{
			pATEInfo->Channel = channels[i];
			Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
			ITxBFLNACalibration(pAd, 1, 0, FALSE);
		}
	}

	return TRUE;
}


/* 
==========================================================================
    Description:
	Sanity check for the channel of Implicit TxBF calibration.
        	
    Return:
	TRUE if all parameters are OK, FALSE otherwise

    Note: 
	1. This sanity check function only work for Implicit TxBF calibration.
	2. Currently supported channels are:
        	1, 14, 36, 64, 128, 132, 165
==========================================================================
*/
static BOOLEAN rtmp_ate_txbf_cal_valid_ch(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR channel)
{
	BOOLEAN bValidCh;

	/* TODO: shall we check the capability of the chipset here ?? */
	switch (channel)
	{
		case 1:
		case 14:
#ifdef A_BAND_SUPPORT
		case 36:
		case 64:
		case 100:
		case 128:
		case 132:
		case 165:
#endif /* A_BAND_SUPPORT */
			bValidCh = TRUE;
			break;
		default:
			bValidCh = FALSE;
			break;
	}

	return bValidCh;	
}


/* 
==========================================================================
    Description:
        Set to start the initialization procedures of iTxBf calibration in DUT side
			0 => do nothing
			1 => do following initializations
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    Note: 
   	This cmd shall only used in DUT side for calibration
==========================================================================
*/
INT Set_ATE_TXBF_INIT_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	int val;
	USHORT eepromVal;
	UCHAR cmdStr[32];
	
	val = simple_strtol(arg, 0, 10);
	if (val != 1)
		return FALSE;

	/* Do ATESTART */
#ifdef CONFIG_RT2880_ATE_CMD_NEW
	Set_ATE_Proc(pAd, "ATESTART");
#else
	Set_ATE_Proc(pAd, "APSTOP");
#endif /* CONFIG_RT2880_ATE_CMD_NEW */

	/* set ATETXBF=3 */
	Set_ATE_TXBF_Proc(pAd, "3");


	/* Set self mac address as 22:22:22:22:22:22 */
	RTMP_IO_WRITE32(pAd, 0x1008, 0x22222222);
	RTMP_IO_WRITE32(pAd, 0x100c, 0x00002222);

	/* set ATEDA=11:11:11:11:11:11 */
	/* set ATESA=22:22:22:22:22:22 */
	/* set ATEBSSID=22:22:22:22:22:22 */
	for (val = 0; val < MAC_ADDR_LEN; val++)
	{
		pATEInfo->Addr1[val] = 0x11; /* the RA */
		pATEInfo->Addr2[val] = 0x22; /* the TA */
		pATEInfo->Addr3[val] = 0x22;	/* the BSSID */
	}

	/* set ATETXMODE=2 */
	Set_ATE_TX_MODE_Proc(pAd, "2");
	
	/* set ATETXMCS=16 */
	Set_ATE_TX_MCS_Proc(pAd, "16");
	
	/* set ATETXBW=0 */
	Set_ATE_TX_BW_Proc(pAd, "0");
	
	/* set ATETXGI=0 */
	Set_ATE_TX_GI_Proc(pAd, "0");
	
	/* set ATETXANT=0 */
	Set_ATE_TX_Antenna_Proc(pAd, "0");
	
	/* set ATERXANT=0 */
	Set_ATE_RX_Antenna_Proc(pAd, "0");
	
	/* set ATETXFREQOFFSET=eeprom */
	/* read EEPROM Frequency offset from EEPROM and set it to BBP */
	RT28xx_EEPROM_READ16(pAd, 0x44, eepromVal);
	snprintf(cmdStr, sizeof(cmdStr), "%d\n", (eepromVal & 0xff));
	Set_ATE_TX_FREQ_OFFSET_Proc(pAd, cmdStr);
	
	/* bbp 65=29 */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x29);
	
	/* bbp 163=bd */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, 0xbd);
	
	/* bbp 173=28 */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, 0x28);

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set to do iTxBF calibration procedures for specific channel, following show us the supported channels.
        	1, 14, 36, 64, 128, 132, 165

    Return:
        TRUE if all parameters are OK, FALSE otherwise

    Note: 
   	This cmd shall only used in DUT side for calibration
==========================================================================
*/
INT Set_ATE_TXBF_CAL_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR ch;
	UCHAR cmdStr[32];
	
	ch = simple_strtol(arg, 0, 10);
	if (rtmp_ate_txbf_cal_valid_ch(pAd, ch) == FALSE)
		return FALSE;

	/* iwpriv ra0 set ATECHANNEL=Channel */
	snprintf(cmdStr, sizeof(cmdStr), "%d\n", ch);
	if (Set_ATE_CHANNEL_Proc(pAd, cmdStr) == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATEINITCHAN =0 */
	if (Set_ATE_INIT_CHAN_Proc(pAd, "0") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXSOUNDING=3 */
	if (Set_ATE_TXSOUNDING_Proc(pAd, "3") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ETxBfNoncompress=0 */
	if (Set_ETxBfNoncompress_Proc(pAd, "0") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXMCS=0 */
	if (Set_ATE_TX_MCS_Proc(pAd, "0") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXCNT=1 */
	if (Set_ATE_TX_COUNT_Proc(pAd, "1") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXLEN=258 */
	if (Set_ATE_TX_LENGTH_Proc(pAd, "258") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set InvTxBfTag=0 */
	if (Set_InvTxBfTag_Proc(pAd, "0") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATE=TXFRAME */
	if (Set_ATE_Proc(pAd, "TXFRAME") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ITxBfCal=1 */
	return Set_ITxBfCal_Proc(pAd, "1");
	
}


/* 
==========================================================================
    Description:
        Set to start the initialization procedures of iTxBf calibration in Golden side at specified channel
			arg => valid values are "1, 14, 36, 64, 128, 132, 165"
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    Note: 
   	This cmd shall only used in GOLDEN side for calibration feedback
==========================================================================
*/
INT Set_ATE_TXBF_GOLDEN_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR ch;
	UCHAR cmdStr[32];
	USHORT eepromVal;
	
	ch = simple_strtol(arg, 0, 10);
	if (rtmp_ate_txbf_cal_valid_ch(pAd, ch) == FALSE)
		return FALSE;	

	/* iwpriv ra0 set ATE=ATESTART */
#ifdef	CONFIG_RT2880_ATE_CMD_NEW
	Set_ATE_Proc(pAd, "ATESTART");
#else
	Set_ATE_Proc(pAd, "APSTOP");
#endif // CONFIG_RT2880_ATE_CMD_NEW //

	/* set the ate channel and read txpower from EEPROM and set to bbp */
	/* iwpriv ra0 set ATECHANNEL=Channel */
	/* iwpriv ra0 set ATETXPOWER=0 */
	snprintf(cmdStr, sizeof(cmdStr), "%d\n", ch);
	Set_ATE_INIT_CHAN_Proc(pAd, cmdStr);
	

	/* Set self mac address as 11:11:11:11:11:11 */
	/* iwpriv ra0 set ATESA=11:11:11:11:11:11 */
	RTMP_IO_WRITE32(pAd, 0x1008, 0x11111111);
	RTMP_IO_WRITE32(pAd, 0x100c, 0x00001111);
	
	/* iwpriv ra0 set ATETXMODE=2 */
	Set_ATE_TX_MODE_Proc(pAd, "2");
	
	/* iwpriv ra0 set ATETXBW=0 */
	Set_ATE_TX_BW_Proc(pAd, "0");
	
	/* iwpriv ra0 set ATETXGI=0 */
	Set_ATE_TX_GI_Proc(pAd, "0");
	
	/* iwpriv ra0 set ATETXANT=1 */
	Set_ATE_TX_Antenna_Proc(pAd, "1");
	
	/* iwpriv ra0 set ATERXANT=1 */
	Set_ATE_RX_Antenna_Proc(pAd, "1");

	/* iwpriv ra0 set ATETXFREQOFFSET=ValueOfEEPROM */
	RT28xx_EEPROM_READ16(pAd, 0x44, eepromVal);
	snprintf(cmdStr, sizeof(cmdStr), "%d\n", (eepromVal & 0xff));
	Set_ATE_TX_FREQ_OFFSET_Proc(pAd, cmdStr);
	
	/* iwpriv ra0 bbp 65=29 */
	/* iwpriv ra0 bbp 163=9d */
	/* iwpriv ra0 bbp 173=00 */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x29);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, 0x9d);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, 0x00);
	
	/* iwpriv ra0 set ATE=RXFRAME */
	Set_ATE_Proc(pAd, "RXFRAME");
	
	/* reset the BBP_R173 as 0 to eliminate the compensation */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, 0x00);
	
	return TRUE;

}


/* 
==========================================================================
    Description:
	Set to do iTxBF calibration verification procedures at sepcified channel, following show us the supported channels.
		args=> valid values are "1, 14, 36, 64, 128, 132, 165"

    Return:
	TRUE if all parameters are OK, FALSE otherwise

    Note: 
	This cmd shall only used in GOLDEN side for calibration verification
==========================================================================
*/
INT Set_ATE_TXBF_VERIFY_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR ch;
	UCHAR cmdStr[32];
	
	ch = simple_strtol(arg, 0, 10);
	if (rtmp_ate_txbf_cal_valid_ch(pAd, ch) == FALSE)
		return FALSE;

	/* iwpriv ra0 set ATECHANNEL=Channel */
	snprintf(cmdStr, sizeof(cmdStr), "%d\n", ch);
	if (Set_ATE_CHANNEL_Proc(pAd, cmdStr) == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXSOUNDING=3 */
	if (Set_ATE_TXSOUNDING_Proc(pAd, "3") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ETxBfNoncompress=0 */
	if (Set_ETxBfNoncompress_Proc(pAd, "0") == FALSE)
		return FALSE;

	/* iwpriv ra0 set ATETXMCS=0 */	
	if (Set_ATE_TX_MCS_Proc(pAd, "0") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXCNT=1 */
	if (Set_ATE_TX_COUNT_Proc(pAd, "1") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXLEN=258 */
	if (Set_ATE_TX_LENGTH_Proc(pAd, "258") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set InvTxBfTag=0 */
	if (Set_InvTxBfTag_Proc(pAd, "0") == FALSE)
		return FALSE;

	/* iwpriv ra0 set ATE=TXFRAME */
	if (Set_ATE_Proc(pAd, "TXFRAME") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ITxBfCal=0 */
	return Set_ITxBfCal_Proc(pAd, "0");
}


INT Set_ATE_ForceBBP_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR bbpReg;

	bbpReg = simple_strtol(arg, 0, 10);

	/*
		0: no any restriction for BBP writing
		1~255: force to not allow to change this specific BBP register.
		
		Note: 
			BBP_R0 is not write-able, so use 0 as the rest operation shall be safe enough
	*/
	pATEInfo->forceBBPReg = bbpReg;
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_ForceBBP_Proc:(forceBBPReg value=%d)\n", pATEInfo->forceBBPReg));

	return TRUE;
}


/* 
==========================================================================
    Description:
	Set to do iTxBF calibration verification without R173 compensation procedures at sepcified channel, following show us the supported channels.
		args=> valid values are "1, 14, 36, 64, 128, 132, 165"

    Return:
	TRUE if all parameters are OK, FALSE otherwise

    Note: 
	This cmd shall only used in GOLDEN side for calibration verification
==========================================================================
*/
INT Set_ATE_TXBF_VERIFY_NoComp_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR ch;
	UCHAR cmdStr[32];
	UCHAR bbpR173 = 0;
	int retval;
	
	ch = simple_strtol(arg, 0, 10);
	if (rtmp_ate_txbf_cal_valid_ch(pAd, ch) == FALSE)
		return FALSE;

	/* iwpriv ra0 set ATECHANNEL=Channel */
	snprintf(cmdStr, sizeof(cmdStr), "%d\n", ch);
	if (Set_ATE_CHANNEL_Proc(pAd, cmdStr) == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXSOUNDING=3 */
	if (Set_ATE_TXSOUNDING_Proc(pAd, "3") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ETxBfNoncompress=0 */
	if (Set_ETxBfNoncompress_Proc(pAd, "0") == FALSE)
		return FALSE;

	/* iwpriv ra0 set ATETXMCS=0 */	
	if (Set_ATE_TX_MCS_Proc(pAd, "0") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXCNT=1 */
	if (Set_ATE_TX_COUNT_Proc(pAd, "1") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set ATETXLEN=258 */
	if (Set_ATE_TX_LENGTH_Proc(pAd, "258") == FALSE)
		return FALSE;
	
	/* iwpriv ra0 set InvTxBfTag=0 */
	if (Set_InvTxBfTag_Proc(pAd, "0") == FALSE)
		return FALSE;

	/* save current BBP_R173 value and reset it as 0 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R173, &bbpR173);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, 0);

	/* force BBP_R173 value when do following procedures. */
	Set_ATE_ForceBBP_Proc(pAd, "173");
	
	/* iwpriv ra0 set ATE=TXFRAME */
	if (Set_ATE_Proc(pAd, "TXFRAME") == FALSE)
	{
		Set_ATE_ForceBBP_Proc(pAd, "0");
		return FALSE;
	}

	/* enable the update of BBP_R173 */
	Set_ATE_ForceBBP_Proc(pAd, "0");
	
	/* iwpriv ra0 set ITxBfCal=0 */
	retval = Set_ITxBfCal_Proc(pAd, "0");

	/* recovery the BBP_173 to original value */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, bbpR173);

	/* done and return */
	return retval;
	
}
#endif /* TXBF_SUPPORT */	


/* 
==========================================================================
    Description:
        Set ATE Tx frame IPG
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_IPG_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32           data, value;

	pATEInfo->IPG = simple_strtol(arg, 0, 10);
	value = pATEInfo->IPG;

	RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &data);

	if (value <= 0)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Set_ATE_IPG_Proc::IPG is disabled(IPG == 0).\n"));
		return TRUE;
	}

	ASSERT(value > 0);

	if ((value > 0) && (value < 256))
	{               
	    RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC1_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC2_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC3_CFG, &data);
	    data &= 0x0;
	    RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, data);
	}
	else
	{
	    UINT32 aifsn, slottime;

	    RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &slottime);
	    slottime &= 0x000000FF;

	    aifsn = value / slottime;                  
	    value = value % slottime;

	    RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC1_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC2_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, data);

	    RTMP_IO_READ32(pAd, EDCA_AC3_CFG, &data);
	    data &= 0x0;
	    data |= (aifsn << 8);
	    RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, data);
	}

	data = (value & 0xFFFF0000) | value | (value << 8);
	RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, data);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_IPG_Proc (IPG = %u)\n", pATEInfo->IPG));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_IPG_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE payload pattern for TxFrame
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_Payload_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	PSTRING				value;

	value = arg;

	/* only one octet acceptable */	
	if (strlen(value) != 2)  
		return FALSE;

	AtoH(value, &(pATEInfo->Payload), 1);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Payload_Proc (repeated pattern = 0x%2x)\n", pATEInfo->Payload));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Payload_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}




INT	Set_ATE_Show_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	PSTRING Mode_String = NULL;
	PSTRING TxMode_String = NULL;
	
	switch (pATEInfo->Mode)
	{
#ifdef CONFIG_RT2880_ATE_CMD_NEW
		case (fATE_IDLE):
			Mode_String = "ATESTART";
			break;
		case (fATE_EXIT):
			Mode_String = "ATESTOP";
			break;
#else
		case (fATE_IDLE):
			Mode_String = "APSTOP";
			break;
		case (fATE_EXIT):
			Mode_String = "APSTART";
			break;
#endif /* CONFIG_RT2880_ATE_CMD_NEW */
		case ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE)):
			Mode_String = "TXCONT";
			break;
		case ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE)):
			Mode_String = "TXCARR";
			break;
		case ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE)):
			Mode_String = "TXCARS";
			break;
		case (fATE_TX_ENABLE):
			Mode_String = "TXFRAME";
			break;
		case (fATE_RX_ENABLE):
			Mode_String = "RXFRAME";
			break;
		default:
		{
			Mode_String = "Unknown ATE mode";
			DBGPRINT(RT_DEBUG_OFF, ("ERROR! Unknown ATE mode!\n"));
			break;
		}
	}	
	DBGPRINT(RT_DEBUG_OFF, ("ATE Mode=%s\n", Mode_String));
#ifdef RT3350
	if (IS_RT3350(pAd))
		DBGPRINT(RT_DEBUG_OFF, ("PABias=%u\n", pATEInfo->PABias));
#endif /* RT3350 */
	DBGPRINT(RT_DEBUG_OFF, ("TxPower0=%d\n", pATEInfo->TxPower0));
	DBGPRINT(RT_DEBUG_OFF, ("TxPower1=%d\n", pATEInfo->TxPower1));
#ifdef DOT11N_SS3_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("TxPower2=%d\n", pATEInfo->TxPower2));
#endif /* DOT11N_SS3_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("TxAntennaSel=%d\n", pATEInfo->TxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("RxAntennaSel=%d\n", pATEInfo->RxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("BBPCurrentBW=%u\n", pATEInfo->TxWI.BW));
	DBGPRINT(RT_DEBUG_OFF, ("GI=%u\n", pATEInfo->TxWI.ShortGI));
	DBGPRINT(RT_DEBUG_OFF, ("MCS=%u\n", pATEInfo->TxWI.MCS));

	switch (pATEInfo->TxWI.PHYMODE)
	{
		case 0:
			TxMode_String = "CCK";
			break;
		case 1:
			TxMode_String = "OFDM";
			break;
		case 2:
			TxMode_String = "HT-Mix";
			break;
		case 3:
			TxMode_String = "GreenField";
			break;
		default:
		{
			TxMode_String = "Unknown TxMode";
			DBGPRINT(RT_DEBUG_OFF, ("ERROR! Unknown TxMode!\n"));
			break;
		}
	}
	
	DBGPRINT(RT_DEBUG_OFF, ("TxMode=%s\n", TxMode_String));
	DBGPRINT(RT_DEBUG_OFF, ("Addr1=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pATEInfo->Addr1[0], pATEInfo->Addr1[1], pATEInfo->Addr1[2], pATEInfo->Addr1[3], pATEInfo->Addr1[4], pATEInfo->Addr1[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr2=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pATEInfo->Addr2[0], pATEInfo->Addr2[1], pATEInfo->Addr2[2], pATEInfo->Addr2[3], pATEInfo->Addr2[4], pATEInfo->Addr2[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr3=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pATEInfo->Addr3[0], pATEInfo->Addr3[1], pATEInfo->Addr3[2], pATEInfo->Addr3[3], pATEInfo->Addr3[4], pATEInfo->Addr3[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Channel=%u\n", pATEInfo->Channel));
	DBGPRINT(RT_DEBUG_OFF, ("TxLength=%u\n", pATEInfo->TxLength));
	DBGPRINT(RT_DEBUG_OFF, ("TxCount=%u\n", pATEInfo->TxCount));
	DBGPRINT(RT_DEBUG_OFF, ("RFFreqOffset=%u\n", pATEInfo->RFFreqOffset));
	DBGPRINT(RT_DEBUG_OFF, ("bAutoTxAlc=%d\n", pATEInfo->bAutoTxAlc));
	DBGPRINT(RT_DEBUG_OFF, ("IPG=%u\n", pATEInfo->IPG));
	DBGPRINT(RT_DEBUG_OFF, ("Payload=0x%02x\n", pATEInfo->Payload));
#ifdef TXBF_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("bTxBF=%d\n", pATEInfo->bTxBF));
	DBGPRINT(RT_DEBUG_OFF, ("txSoundingMode=%d\n", pATEInfo->txSoundingMode));
#endif /* TXBF_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("Set_ATE_Show_Proc Success\n"));
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}


INT	Set_ATE_Help_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef CONFIG_RT2880_ATE_CMD_NEW
	DBGPRINT(RT_DEBUG_OFF, ("ATE=ATESTART, ATESTOP, TXCONT, TXCARR, TXCARS, TXFRAME, RXFRAME\n"));
#else
	DBGPRINT(RT_DEBUG_OFF, ("ATE=APSTOP, APSTART, TXCONT, TXCARR, TXCARS, TXFRAME, RXFRAME\n"));
#endif /* CONFIG_RT2880_ATE_CMD_NEW */
	DBGPRINT(RT_DEBUG_OFF, ("ATEDA\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATESA\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEBSSID\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATECHANNEL, range:0~14(unless A band !)\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXPOW0, set power level of antenna 1.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXPOW1, set power level of antenna 2.\n"));
#ifdef DOT11N_SS3_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATETXPOW2, set power level of antenna 3.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXANT, set TX antenna. 0:all, 1:antenna one, 2:antenna two, 3:antenna three.\n"));
#else
	DBGPRINT(RT_DEBUG_OFF, ("ATETXANT, set TX antenna. 0:all, 1:antenna one, 2:antenna two.\n"));
#endif /* DOT11N_SS3_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("ATERXANT, set RX antenna.0:all, 1:antenna one, 2:antenna two, 3:antenna three.\n"));
#ifdef RT3350
	if (IS_RT3350(pAd))
		DBGPRINT(RT_DEBUG_OFF, ("ATEPABIAS, set power amplifier bias for EVM, range 0~15\n"));
#endif /* RT3350 */
#ifdef RTMP_RF_RW_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATETXFREQOFFSET, set frequency offset, range 0~95\n"));
#else
	DBGPRINT(RT_DEBUG_OFF, ("ATETXFREQOFFSET, set frequency offset, range 0~63\n"));
#endif /* RTMP_RF_RW_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("ATETXBW, set BandWidth, 0:20MHz, 1:40MHz.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXLEN, set Frame length, range 24~%d\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXCNT, set how many frame going to transmit.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXMCS, set MCS, reference to rate table.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXMODE, set Mode 0:CCK, 1:OFDM, 2:HT-Mix, 3:GreenField, reference to rate table.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXGI, set GI interval, 0:Long, 1:Short\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERXFER, 0:disable Rx Frame error rate. 1:enable Rx Frame error rate.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERRF, show all RF registers.\n"));
#ifndef RTMP_RF_RW_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF1, set RF1 register.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF2, set RF2 register.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF3, set RF3 register.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF4, set RF4 register.\n"));
#endif /* !RTMP_RF_RW_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("ATELDE2P, load EEPROM from .bin file.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERE2P, display all EEPROM content.\n"));
#ifdef LED_CONTROL_SUPPORT
#endif /* LED_CONTROL_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("ATEAUTOALC, enable ATE auto Tx alc (Tx auto level control).\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEIPG, set ATE Tx frame IPG.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEPAYLOAD, set ATE payload pattern for TxFrame.\n"));
#ifdef TXBF_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATETXBF, enable ATE Tx beam forming.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXSOUNDING, Sounding mode 0:none, 1:Data sounding, 2:2 stream NDP, 3:3 stream NDP.\n"));
#endif /* TXBF_SUPPORT */ 
#ifdef RTMP_INTERNAL_TX_ALC
	DBGPRINT(RT_DEBUG_OFF, ("ATETSSICBA, start internal TSSI calibration.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETSSICBAEX, start extended internal TSSI calibration.\n"));
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef RTMP_TEMPERATURE_COMPENSATION
	DBGPRINT(RT_DEBUG_OFF, ("ATEREADEXTSSI, start advanced temperature TSSI calibration.\n"));
#endif /* RTMP_TEMPERATURE_COMPENSATION */
	DBGPRINT(RT_DEBUG_OFF, ("ATESHOW, display all parameters of ATE.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEHELP, online help.\n"));

	return TRUE;
}


#ifdef RT6352
#ifdef RTMP_TEMPERATURE_CALIBRATION
INT RT6352_ATETempCalibration(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	//PATE_INFO pATEInfo = &(pAd->ate);
	INT32 ref25 = 0, curr_reading = 0, curr_temperature = 0;
	INT ret = TRUE;
	UINT16 e2p_data = 0;

	curr_temperature = (INT32)simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("curr_temperature = 0x%x\n", curr_temperature));

	/* to get bbp r49 */
	RT6352_Temperature_Init(pAd);	
	curr_reading = pAd->CurrTemperature;
	DBGPRINT(RT_DEBUG_TRACE, ("curr_reading = 0x%x\n", curr_reading));

	ref25 = curr_reading + ((25 - curr_temperature)*10)/19;
	/* write calibration result */
	RT28xx_EEPROM_READ16(pAd, 0xD0, e2p_data);
	e2p_data &= ~(0xFF00);
	e2p_data |= (((ref25 & 0xFF) << 8) & 0xFF00);	
	RT28xx_EEPROM_WRITE16(pAd, 0xD0, e2p_data);

	return ret;
}
#endif /* RTMP_TEMPERATURE_CALIBRATION */
#endif /* RT6352 */	


#ifdef RTMP_TEMPERATURE_CALIBRATION
INT Set_ATE_TEMP_CAL_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT ret = TRUE;

#ifdef RT6352
	if (IS_RT6352(pAd))
		ret = RT6352_ATETempCalibration(pAd, arg);		
#endif /* RT6352 */	

	return ret;
}


INT Set_ATE_SHOW_TSSI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT ret = TRUE;
#ifdef RT6352
	UCHAR wait, BBPValue, RFValue, RFB0R2Value;
	CHAR BbpR49 = 0;
#endif /* RT6352 */

#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		/* 1. set BBP R47[4]=1 to enable TSSI for next reading */
		/* enable RF before reading temperature sensor */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &RFValue);
		RFB0R2Value = RFValue; /* keep it for recovery later */
		RFValue |= 0x10;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue);

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

		if (!(BBPValue & (1<<4)))
		{
			BBPValue |= (1<<4);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);		
		}

		/* 2. polling BBP_R47 until BBP R47[4]==0 */
		for (wait=0; wait<200; wait++)
		{
			RTMPusecDelay(100);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

			if (!(BBPValue & (1<<4)))
			{
				break;
			}
		}

		if ((BBPValue & 0x10) != 0)
		{
			ret = FALSE;
			printk("Read TSSI failed !\n");
			return ret;
		}

		/* 3. set BBP R47[3:0]=4 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
		BBPValue &= ~(0x0F);
		BBPValue |= (0x04);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

		/* 4. fetch temperature reading */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);

		/* 5. recover RF after reading temperature sensor */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFB0R2Value);

		printk("BBP R49: 0x%02X\n", BbpR49);
	}
#endif /* RT6352 */	

	return ret;
}
#endif /* RTMP_TEMPERATURE_CALIBRATION */


#ifdef RTMP_INTERNAL_TX_ALC


INT Set_ATE_TSSI_CALIBRATION_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{    
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->TssiCalibration != NULL)
	{
		pATEInfo->pChipStruct->TssiCalibration(pAd, arg);
	}
	else
	{
		RTMP_CHIP_ATE_TSSI_CALIBRATION(pAd, arg);
	}

	return TRUE;
}


INT Set_ATE_TSSI_CALIBRATION_EX_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{    
	PATE_INFO pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->ExtendedTssiCalibration != NULL)
	{
		pATEInfo->pChipStruct->ExtendedTssiCalibration(pAd, arg);
	}
	else
	{
		RTMP_CHIP_ATE_TSSI_CALIBRATION_EXTEND(pAd, arg);
	}
	
	return TRUE;
}


#if defined(RT3350) || defined(RT3352)
INT RT335x2_Set_ATE_TSSI_CALIBRATION_ENABLE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	BOOLEAN	bTSSICalbrEnableG = FALSE;
	
	if (pAd->TxPowerCtrl.bInternalTxALC == FALSE)                  
	{
		DBGPRINT_ERR(("Please set e2p 0x36 as 0x2024!!!\n"));
		return FALSE;
	}

	if ((!IS_RT3350(pAd)) && (!IS_RT3352(pAd)))                  
	{
		DBGPRINT_ERR(("Not support TSSI calibration since not 3350/3352 chip!!!\n"));
		return FALSE;
	}

	if (strcmp(arg, "0") == 0)
	{
		bTSSICalbrEnableG = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI calibration disabled!\n"));
	}
	else if (strcmp(arg, "1") == 0)
	{
		bTSSICalbrEnableG = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI calibration enabled!\n"));
	}
	else
	{
		return FALSE;
	}

	pAd->ate.bTSSICalbrEnableG = bTSSICalbrEnableG;

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


CHAR InsertTssi(UCHAR InChannel, UCHAR Channel0, UCHAR Channel1,CHAR Tssi0, CHAR Tssi1)
{
	CHAR     InTssi;
	CHAR     ChannelDelta, InChannelDelta;
	CHAR     TssiDelta;

	ChannelDelta = Channel1 - Channel0;
	InChannelDelta = InChannel - Channel0;
	TssiDelta = Tssi1 - Tssi0;

	InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);

	return InTssi;
}


INT RT335xATETssiCalibrationExtend(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{    
	CHAR		TssiRefPerChannel[CFG80211_NUM_OF_CHAN_2GHZ], TssiDeltaPerChannel[CFG80211_NUM_OF_CHAN_2GHZ];
	UCHAR		CurrentChannel;
	UCHAR		BbpData = 0;
	USHORT		EEPData;

	if (pAd->ate.bTSSICalbrEnableG == FALSE)
	{
		DBGPRINT_ERR(("No TSSI readings obtained !!!\n"));
		DBGPRINT_ERR(("TSSI calibration failed !!!\n"));

		return FALSE;
	}
	else
	{
		pAd->ate.bTSSICalbrEnableG = FALSE;
	}

	NdisCopyMemory(TssiRefPerChannel, pAd->ate.TssiRefPerChannel, CFG80211_NUM_OF_CHAN_2GHZ);
	NdisCopyMemory(TssiDeltaPerChannel, pAd->ate.TssiDeltaPerChannel, CFG80211_NUM_OF_CHAN_2GHZ);

	/* step 1: write TSSI_ref to EEPROM 0x6E */
	CurrentChannel = 7;
	BbpData = TssiRefPerChannel[CurrentChannel-1];
	DBGPRINT(RT_DEBUG_TRACE, ("TSSI_ref = 0x%02x\n", TssiRefPerChannel[CurrentChannel-1]));  
	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
	EEPData &= 0xff00;
	EEPData |= BbpData;
	DBGPRINT(RT_DEBUG_TRACE, ("Write  E2P 0x6e: 0x%04x\n", EEPData));  
#ifdef RTMP_EFUSE_SUPPORT
	if(pAd->bUseEfuse)
	{
		if(pAd->bFroceEEPROMBuffer)
			NdisMoveMemory(&(pAd->EEPROMImage[EEPROM_TSSI_OVER_OFDM_54]), (PUCHAR) (&EEPData) ,2);
		else
			eFuseWrite(pAd, EEPROM_TSSI_OVER_OFDM_54, (PUCHAR) (&EEPData), 2);
	}
	else
#endif /* RTMP_EFUSE_SUPPORT */
	{
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
		RTMPusecDelay(10);
	}

	/* step 2: insert the TSSI table */
	/* insert channel 2 to 6 TSSI values */
	for (CurrentChannel = 2; CurrentChannel < 7; CurrentChannel++)
		TssiRefPerChannel[CurrentChannel-1] = InsertTssi(CurrentChannel, 1, 7, TssiRefPerChannel[0], TssiRefPerChannel[6]);

	/* insert channel 8 to 12 TSSI values */
	for (CurrentChannel = 8; CurrentChannel < 13; CurrentChannel++)
		TssiRefPerChannel[CurrentChannel-1] = InsertTssi(CurrentChannel, 7, 13, TssiRefPerChannel[6], TssiRefPerChannel[12]);

	/* channel 14 TSSI equals channel 13 TSSI */
	TssiRefPerChannel[13] = TssiRefPerChannel[12];

	for (CurrentChannel = 1; CurrentChannel <= 14; CurrentChannel++)
	{
		TssiDeltaPerChannel[CurrentChannel-1] = TssiRefPerChannel[CurrentChannel-1] - TssiRefPerChannel[6];

		/* boundary check */
		if(TssiDeltaPerChannel[CurrentChannel-1] > 7 )
			TssiDeltaPerChannel[CurrentChannel-1]  = 7;
		if(TssiDeltaPerChannel[CurrentChannel-1] < -8 )
			TssiDeltaPerChannel[CurrentChannel-1]  = -8;

		/* eeprom only use 4 bit for TSSI delta */
		TssiDeltaPerChannel[CurrentChannel-1]  &= 0x0f;
		DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, TSSI= 0x%x, TssiDelta=0x%x\n", 
		CurrentChannel, TssiRefPerChannel[CurrentChannel-1], TssiDeltaPerChannel[CurrentChannel-1]));    
	}

	/* step 3: store TSSI delta values to EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);
	EEPData &= 0x00ff;
	EEPData |= (TssiDeltaPerChannel[0] << 8) | (TssiDeltaPerChannel[1] << 12);

#ifdef RTMP_EFUSE_SUPPORT
	if (pAd->bUseEfuse)
	{
		if (pAd->bFroceEEPROMBuffer)
			NdisMoveMemory(&(pAd->EEPROMImage[EEPROM_TX_POWER_OFFSET_OVER_CH_1-1]), (PUCHAR)(&EEPData), 2);
		else
			eFuseWrite(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, (PUCHAR) (&EEPData), 2);
	}
	else
#endif /* RTMP_EFUSE_SUPPORT */
	{
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);
		RTMPusecDelay(10);
	}

	for (CurrentChannel = 3; CurrentChannel <= 14; CurrentChannel += 4)
	{
		EEPData = (TssiDeltaPerChannel[CurrentChannel+2] << 12) |(TssiDeltaPerChannel[CurrentChannel+1] << 8)
			| (TssiDeltaPerChannel[CurrentChannel] << 4) | TssiDeltaPerChannel[CurrentChannel-1];
#ifdef RTMP_EFUSE_SUPPORT
		if (pAd->bUseEfuse)
		{
			if (pAd->bFroceEEPROMBuffer)
				NdisMoveMemory(&(pAd->EEPROMImage[(EEPROM_TX_POWER_OFFSET_OVER_CH_3 +((CurrentChannel-3)/2))]), (PUCHAR)(&EEPData), 2);
			else
				eFuseWrite(pAd, (EEPROM_TX_POWER_OFFSET_OVER_CH_3 +((CurrentChannel-3)/2)), (PUCHAR) (&EEPData), 2);
		}
		else
#endif /* RTMP_EFUSE_SUPPORT */
		{
			RT28xx_EEPROM_WRITE16(pAd, (EEPROM_TX_POWER_OFFSET_OVER_CH_3 +((CurrentChannel-3)/2)), EEPData);
			RTMPusecDelay(10);
		}
	}

	/* step 4: disable legacy ALC and set TSSI enabled and TSSI extend mode to EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_ENABLE, EEPData);
	/* disable legacy ALC */
	EEPData &= ~(1 << 1);
	/* enable TSSI */
	EEPData |= (1 << 13);
#ifdef RTMP_EFUSE_SUPPORT
	if (pAd->bUseEfuse)
	{
		if (pAd->bFroceEEPROMBuffer)
			NdisMoveMemory(&(pAd->EEPROMImage[EEPROM_TSSI_ENABLE]), (PUCHAR)(&EEPData), 2);
		else
			eFuseWrite(pAd, EEPROM_TSSI_ENABLE, (PUCHAR)(&EEPData), 2);
	}
	else
#endif /* RTMP_EFUSE_SUPPORT */
	{
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_ENABLE, EEPData);
		RTMPusecDelay(10);
	}

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_MODE_EXTEND, EEPData);
	/* set extended TSSI mode */
	EEPData |= (1 << 15);
#ifdef RTMP_EFUSE_SUPPORT
	if (pAd->bUseEfuse)
	{
		if (pAd->bFroceEEPROMBuffer)
			NdisMoveMemory(&(pAd->EEPROMImage[EEPROM_TSSI_MODE_EXTEND]), (PUCHAR)(&EEPData), 2);
		else
			eFuseWrite(pAd, EEPROM_TSSI_MODE_EXTEND, (PUCHAR)(&EEPData), 2);
	}
	else
#endif /* RTMP_EFUSE_SUPPORT */
	{
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_MODE_EXTEND, EEPData);
		RTMPusecDelay(10);
	}

	/* step 5: synchronize ATE private data structure with the values written to EEPROM */
	NdisCopyMemory(pAd->ate.TssiRefPerChannel, TssiRefPerChannel, CFG80211_NUM_OF_CHAN_2GHZ);
	NdisCopyMemory(pAd->ate.TssiDeltaPerChannel, TssiDeltaPerChannel, CFG80211_NUM_OF_CHAN_2GHZ);

	return TRUE;
}

#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */


#ifdef RTMP_TEMPERATURE_COMPENSATION


INT Set_ATE_READ_EXTERNAL_TSSI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{

	RTMP_CHIP_ATE_READ_EXTERNAL_TSSI(pAd, arg);
	return TRUE;
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */




struct _ATE_CHIP_STRUCT RALINKDefault =
{
	/* functions */
	.ChannelSwitch = DefaultATEAsicSwitchChannel,
	.TxPwrHandler = DefaultATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = NULL,
	.AsicSetTxRxPath = DefaultATEAsicSetTxRxPath,
	.AdjustTxPower = DefaultATEAsicAdjustTxPower,	
	.AsicExtraPowerOverMAC = NULL,

	/* command handlers */
	.Set_BW_Proc = Default_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = Default_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,	
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};

#ifdef RT28xx
#ifdef RTMP_MAC_PCI
extern ATE_CHIP_STRUCT RALINK2860;
extern ATE_CHIP_STRUCT RALINK2880;
#endif /* RTMP_MAC_PCI */
#endif /* RT28xx */


#ifdef RT2883
extern ATE_CHIP_STRUCT RALINK2883;
#endif /* RT2883 */

#ifdef RT3883
extern ATE_CHIP_STRUCT RALINK3883;
#endif /* RT3883 */	

#ifdef RT305x
#ifdef RT3050
extern ATE_CHIP_STRUCT RALINK3050;
#endif /* RT3050 */

#ifdef RT3052
extern ATE_CHIP_STRUCT RALINK3052;
#endif /* RT3052 */

#ifdef RT3350
extern ATE_CHIP_STRUCT RALINK3350;
#endif /* RT3350 */

#ifdef RT3352
extern ATE_CHIP_STRUCT RALINK3352;
#endif /* RT3352 */

#ifdef RT5350
extern ATE_CHIP_STRUCT RALINK5350;
#endif /* RT5350 */
#endif /* RT305x */






#ifdef RT6352
extern ATE_CHIP_STRUCT RALINK6352;
#endif /* RT6352 */


/*
==========================================================================
	Description:
		Assign chip structure when initialization.
		This routine is specific for ATE.

==========================================================================
*/
NDIS_STATUS ChipStructAssign(
	IN	PRTMP_ADAPTER	pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);


	pATEInfo->pChipStruct = &RALINKDefault;

#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		/* 
			Notice: IS_RT6352(_pAd) is currently defined 
			as (((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000)
		*/
	pATEInfo->pChipStruct = &RALINK6352;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* IS_RT6352 */


#ifdef RT2883
	if (IS_RT2883(pAd))
	{
		pATEInfo->pChipStruct = &RALINK2883;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT2883 */

#ifdef RT3883
	if (IS_RT3883(pAd))
	{
		pATEInfo->pChipStruct = &RALINK3883;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT3883 */


#ifdef RT305x
#ifdef RT3050
	if (IS_RT3050(pAd))
	{
		pATEInfo->pChipStruct = &RALINK3050;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT3050 */

#ifdef RT3052
	if (IS_RT3052(pAd))
	{
		pATEInfo->pChipStruct = &RALINK3052;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT3052 */

#ifdef RT3350
	if (IS_RT3350(pAd))
	{
		pATEInfo->pChipStruct = &RALINK3350;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT3350 */

#ifdef RT3352
	if (IS_RT3352(pAd))
	{
		pATEInfo->pChipStruct = &RALINK3352;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT3352 */

#ifdef RT5350
	if (IS_RT5350(pAd))
	{
		pATEInfo->pChipStruct = &RALINK5350;
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RT5350 */
#endif /* RT305x */


#ifdef RT28xx
#ifdef A_BAND_SUPPORT
	if (IS_PCI_ONLY_INF(pAd) || IS_USB_INF(pAd) || IS_RBUS_INF(pAd))
	{
#ifdef RT2860
		if (IS_PCI_ONLY_INF(pAd))
		{
			pATEInfo->pChipStruct = &RALINK2860;
			return NDIS_STATUS_SUCCESS;
		}
#endif /* RT2860 */


#ifdef RT2880
		if (IS_RBUS_INF(pAd))
		{
			pATEInfo->pChipStruct = &RALINK2880;
			return NDIS_STATUS_SUCCESS;
		}
#endif /* RT2880 */
	}
#endif /* A_BAND_SUPPORT */
#endif /* RT28xx */




	/* sanity check */
	if (pATEInfo->pChipStruct == &RALINKDefault)
	{
		/* Unknown chipset ! */
		DBGPRINT_ERR(("Warning - Unknown chipset !!!\n"));
		DBGPRINT_ERR(("MAC Version is %u\n", ((pAd->MACVersion & 0xFFFF0000) >> 16)));
		DBGPRINT_ERR(("Interface type is %d\n", pAd->infType));

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
==========================================================================
	Description:
		Initialize ATE_INFO structure.
		This routine is specific for ATE.

==========================================================================
*/
NDIS_STATUS ATEInit(
	IN	PRTMP_ADAPTER	pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	NdisZeroMemory(pATEInfo, sizeof(ATE_INFO));

	if (ChipStructAssign(pAd) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("%s failed !\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	OS_NdisAllocateSpinLock(&(pATEInfo->TssiSemLock));

	pATEInfo->Mode = ATE_STOP;
#ifdef RT3350
	pATEInfo->PABias = 0;
#endif /* RT3350  */
	pATEInfo->TxCount = 0xFFFFFFFF; /* to sync with QA and to exceed TX_RING_SIZE ... */
	pATEInfo->TxDoneCount = 0;
	pATEInfo->RFFreqOffset = 0;
#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		pATEInfo->Payload = 0xAA; /* new chip: sync with QA */	
		pATEInfo->bDPDEnable = FALSE;		
	}
	else
#endif /* RT6352 */
		pATEInfo->Payload = 0xA5;/* to be backward compatible */	
	pATEInfo->IPG = 200;/* 200 : sync with QA */	
	pATEInfo->TxLength = 1058;/* 1058 : sync with QA */
	pATEInfo->TxWI.ShortGI = 0;/* LONG GI : 800 ns*/
	pATEInfo->TxWI.PHYMODE = MODE_CCK;
	pATEInfo->TxWI.MCS = 3;
	pATEInfo->TxWI.BW = BW_20;
	/* please do not change these default values */
	pATEInfo->Channel = 1;
	pATEInfo->TxAntennaSel = 1;
	pATEInfo->RxAntennaSel = 0;


#ifdef RTMP_MAC_PCI 
#endif /* RTMP_MAC_PCI */
	pATEInfo->QID = QID_AC_BE;

#ifdef DOT11N_SS3_SUPPORT
	/* For 3T/3R ++ */
	/* use broadcast address as default value */
	pATEInfo->Addr1[0] = 0xFF;
	pATEInfo->Addr1[1] = 0xFF;
	pATEInfo->Addr1[2] = 0xFF;
	pATEInfo->Addr1[3] = 0xFF;
	pATEInfo->Addr1[4] = 0xFF;
	pATEInfo->Addr1[5] = 0xFF;

	pATEInfo->Addr2[0] = 0x00;
	pATEInfo->Addr2[1] = 0x11;
	pATEInfo->Addr2[2] = 0x22;
	pATEInfo->Addr2[3] = 0xAA;
	pATEInfo->Addr2[4] = 0xBB;
	pATEInfo->Addr2[5] = 0xCC;

	NdisMoveMemory(pATEInfo->Addr3, pATEInfo->Addr2, ETH_LENGTH_OF_ADDRESS);

	{		
		UINT32 data;

		data = 0xFFFFFFFF;
    	RTMP_IO_WRITE32(pAd, 0x1044, data); 
    	RTMP_IO_READ32(pAd, 0x1048, &data); 

    	data = data | 0x0000FFFF;
    	RTMP_IO_WRITE32(pAd, 0x1048, data); 
	}
	/* For stream mode in 3T/3R -- */
#else
	pATEInfo->Addr1[0] = 0x00;
	pATEInfo->Addr1[1] = 0x11;
	pATEInfo->Addr1[2] = 0x22;
	pATEInfo->Addr1[3] = 0xAA;
	pATEInfo->Addr1[4] = 0xBB;
	pATEInfo->Addr1[5] = 0xCC;

	NdisMoveMemory(pATEInfo->Addr2, pATEInfo->Addr1, ETH_LENGTH_OF_ADDRESS);
	NdisMoveMemory(pATEInfo->Addr3, pATEInfo->Addr1, ETH_LENGTH_OF_ADDRESS);
#endif /* DOT11N_SS3_SUPPORT */

	pATEInfo->bRxFER = 0;
	pATEInfo->bQAEnabled = FALSE;
	pATEInfo->bQATxStart = FALSE;
	pATEInfo->bQARxStart = FALSE;
	pATEInfo->bAutoTxAlc = FALSE;
	pATEInfo->bLowTemperature = FALSE;
#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	pATEInfo->bTSSICalbrEnableG = FALSE;
	NdisZeroMemory((PUCHAR)&(pATEInfo->TssiRefPerChannel), CFG80211_NUM_OF_CHAN_2GHZ);
	NdisZeroMemory((PUCHAR)&(pATEInfo->TssiDeltaPerChannel), CFG80211_NUM_OF_CHAN_2GHZ);
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */
	/* Default TXCONT/TXCARR/TXCARS mechanism is TX_METHOD_1 */
	pATEInfo->TxMethod = TX_METHOD_1;
	if ((IS_RT2070(pAd) || IS_RT2860(pAd) || IS_RT2872(pAd) || IS_RT2883(pAd)))
	{
		/* Early chipsets must be applied original TXCONT/TXCARR/TXCARS mechanism. */
		pATEInfo->TxMethod = TX_METHOD_0;
	}

	/* Power range is 0~31 in A band. */
	pATEInfo->MinTxPowerBandA = 0;
	pATEInfo->MaxTxPowerBandA = 31;
	if ((IS_RT2860(pAd)) || (IS_RT2872(pAd)) || (IS_RT2883(pAd)))
	{
		/* Power range of early chipsets is -7~15 in A band. */
		pATEInfo->MinTxPowerBandA = -7;
		pATEInfo->MaxTxPowerBandA = 15;
	}

#ifdef TXBF_SUPPORT
	pATEInfo->bTxBF = FALSE;	
#endif /* TXBF_SUPPORT */
#ifdef RTMP_MAC_PCI
	pATEInfo->bFWLoading = FALSE;
#endif /* RTMP_MAC_PCI */



#ifdef RALINK_QA
	pATEInfo->TxStatus = 0;
	RtmpOsTaskPidInit(&(pATEInfo->AtePid));
/*	pATEInfo->AtePid = THREAD_PID_INIT_VALUE; */
#endif /* RALINK_QA */
	pATEInfo->OneSecPeriodicRound = 0;
	pATEInfo->PeriodicRound = 0;

	return NDIS_STATUS_SUCCESS;
}


#ifdef RALINK_QA
/*
==========================================================================
	Description:
		This routine is specific for ATE.
		When we start tx from QA GUI, it will modify BBP registers without
		notify ATE driver what the tx subtype is.

    Return:
        VOID
==========================================================================
*/
VOID ReadQATxTypeFromBBP(
	IN	PRTMP_ADAPTER	pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR   Bbp22Value = 0, Bbp24Value = 0;

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &Bbp22Value);

	switch (Bbp22Value)
	{
		case BBP22_TXFRAME:
			{
#ifdef RTMP_MAC_PCI
				if (pATEInfo->TxCount == 0)
				{
					pATEInfo->TxCount = 0xFFFFFFFF;
				}
#endif /* RTMP_MAC_PCI */
				DBGPRINT(RT_DEBUG_TRACE,("START TXFRAME\n"));
				pATEInfo->bQATxStart = TRUE;
				Set_ATE_Proc(pAd, "TXFRAME");
			}
			break;

		case BBP22_TXCONT_OR_CARRSUPP:
			{
				DBGPRINT(RT_DEBUG_TRACE,("BBP22_TXCONT_OR_CARRSUPP\n"));
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R24, &Bbp24Value);

				switch (Bbp24Value)
				{
					case BBP24_TXCONT:
						{
							DBGPRINT(RT_DEBUG_TRACE,("START TXCONT\n"));
							pATEInfo->bQATxStart = TRUE;

							if (pATEInfo->TxMethod == TX_METHOD_0)
							{
								Set_ATE_Proc(pAd, "TXCONT");
							}
						}
						break;

					case BBP24_CARRSUPP:
						{
							DBGPRINT(RT_DEBUG_TRACE,("START TXCARRSUPP\n"));
							pATEInfo->bQATxStart = TRUE;

							if (pATEInfo->TxMethod == TX_METHOD_0)
							{
								Set_ATE_Proc(pAd, "TXCARS");
							}
						}
						break;

					default:
						{
							DBGPRINT_ERR(("Unknown TX subtype !\n"));
						}
						break;
				}
			}
			break;	

		case BBP22_TXCARR:
			{
				DBGPRINT(RT_DEBUG_TRACE,("START TXCARR\n"));
				pATEInfo->bQATxStart = TRUE;

				if (pATEInfo->TxMethod == TX_METHOD_0)
				{
					Set_ATE_Proc(pAd, "TXCARR");
				}
			}
			break;							

		default:
			{
				DBGPRINT_ERR(("Unknown Start TX subtype !\n"));
			}
			break;
	}

	return;
}
#endif /* RALINK_QA */


NDIS_STATUS ATEBBPWriteWithRxChain(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR bbpId,
	IN CHAR bbpVal,
	IN RX_CHAIN_IDX rx_ch_idx)
{
	UCHAR idx = 0, val = 0;

	if (((pAd->MACVersion & 0xffff0000) < 0x28830000) || 
		(pAd->Antenna.field.RxPath == 1))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpVal);
		return NDIS_STATUS_SUCCESS;
	}
	
	while (rx_ch_idx != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;
		
		if (rx_ch_idx & 0x01)
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &val);
			val = (val & (~0x60)/* clear bit5 and bit6 */) | (idx << 5);

#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, val);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpVal);
			}
#endif /* RTMP_MAC_PCI */

			DBGPRINT(RT_DEBUG_INFO, 
					("%s(Idx):Write(R%d,val:0x%x) to Chain(0x%x, idx:%d)\n",
						__FUNCTION__, bbpId, bbpVal, rx_ch_idx, idx));
		}
		rx_ch_idx >>= 1;
		idx++;
	}

	return NDIS_STATUS_SUCCESS;
}


#define SMM_BASEADDR                      0x4000
#define PKT_BASEADDR                      0x8000


INT Set_ADCDump_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR BBP_R21_Ori=0,BBP_R60_Ori=0,BBP_R142_ORI=0,BBP_R143_ORI=0;
	UINT32 MACValue=0,PBF_SYS_CTRL_ORI=0,PBF_CAP_CTRL_ORI=0;
	UINT32 CaptureModeOffset=0,CaptureStartAddr=0;
	UINT32 SMM_Addr;
	UINT32 PKT_Addr;
	int i = 0; 
	PSTRING					src = "ADCDump.txt";
	RTMP_OS_FD				srcf;
	RTMP_OS_FS_INFO			osFSInfo;
	UCHAR				msg[128];
	UCHAR				msg1[128];
	CAPTURE_MODE_SHARE_MEMORY     SMMValued;
	CAPTURE_MODE_PACKET_BUFFER    PKTValue1d;
	CAPTURE_MODE_PACKET_BUFFER    PKTValue2d;
	UCHAR retval=0;
	UCHAR DataSourceADC6=simple_strtol(arg, 0, 10);
	
	pAd->ate.Mode = ATE_START;

	/* Disable Tx/Rx */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BBP_R21_Ori);

	/* Disable BBP power saving */
	   
	/* disable all Tx/Rx Queue */
	RTMP_IO_WRITE32(pAd, PBF_CFG, 0x00000000);

	/* capture mode */
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &MACValue);
	PBF_SYS_CTRL_ORI=MACValue;
	MACValue |= 0x00004000; /* bit[14]=1 */
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, MACValue);

	/* capture setting */
	if (DataSourceADC6 == 1)
	{
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R60, &BBP_R60_Ori);                              
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R60, 0x80);
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R142, &BBP_R142_ORI);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, 0x10);
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R143, &BBP_R143_ORI);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, 0x05);

		RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &MACValue);
		PBF_CAP_CTRL_ORI=MACValue;
		MACValue |= 0x00008000; /* set bit[15]=1 for ADC 6 */
		MACValue &= ~0x80000000; /* set bit[31]=0 */
		RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, MACValue);
	}
	else
	{
		RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &MACValue);
		PBF_CAP_CTRL_ORI=MACValue;
		MACValue &= ~0x80008000; /* set bit[31]=0, bit[15]=0 for ADC 8 */
		RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, MACValue);
	}

	/* trigger offset */
	RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &MACValue);
	MACValue &= ~(0x1FFF0000);
	RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, MACValue);
						
	if ((CaptureModeOffset > 0) && (CaptureModeOffset <= 0x1FFF))
	{
		RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &MACValue);
		MACValue |= CaptureModeOffset << 16;
		RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, MACValue);
	}

	/* start capture */
	RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &MACValue);
	MACValue = MACValue | 0x40000000; /* set bit[30]=1 */
	RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, MACValue);

	if (0)
	{
		/* start TX */
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x4);
	}
	else
	{
		/* start RX */
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x8);
	}
                        
	/* Wait until [0x440] bit30=0 */
	do
	{
		i++;
		RTMPusecDelay(10);
		RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &MACValue);
		MACValue = MACValue & 0x40000000; /* bit[30] */

		if (MACValue == 0)
		{
			break;
		}

		if (i == 1000) /* 3 sec */
		{
			printk("Error, over 3s\n");
			break;
		}
	} while (MACValue != 0);

	if (DataSourceADC6 == 1)
	{
		/* restore BBP R60 */
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R60, BBP_R60_Ori);
	}

	/* Stop TX/RX */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

	/* Read [0x440] bit[12:0] */
	RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &CaptureStartAddr);
	CaptureStartAddr = CaptureStartAddr & 0x00001FFF;

	/* Dump data from MAC memory */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	SMM_Addr=SMM_BASEADDR+CaptureStartAddr*2;
	PKT_Addr=PKT_BASEADDR+CaptureStartAddr*4;
	
	/* SMM Address must be four byte alignment*/
	SMM_Addr=(SMM_Addr/4)*4;

	/* open file */
	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_WRONLY|O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("--> Error opening %s\n", src));
			return FALSE;
		}
		else 
		{
			memset(msg, 0x00, 128);
			memset(msg1, 0x00, 128);

			for (i=0;i<0x1000;i++)
			{
				RTMP_IO_READ32(pAd,SMM_Addr, &SMMValued.Value);
				SMM_Addr += 4;

				if(SMM_Addr >= 0x8000)
					SMM_Addr = SMM_Addr - SMM_BASEADDR;

				RTMP_IO_READ32(pAd,PKT_Addr, &PKTValue1d.Value);
				PKT_Addr += 4;

				if(PKT_Addr >= 0x10000)
					PKT_Addr = PKT_Addr - PKT_BASEADDR;

				RTMP_IO_READ32(pAd,PKT_Addr, &PKTValue2d.Value);
				PKT_Addr += 4;

				if(PKT_Addr >= 0x10000)
					PKT_Addr = PKT_Addr - PKT_BASEADDR;

				sprintf(msg, "%d %d %d %d %d %d\n",SMMValued.field.LOW_BYTE1,SMMValued.field.LOW_BYTE0
				              ,PKTValue1d.field.BYTE3,PKTValue1d.field.BYTE2
				              ,PKTValue1d.field.BYTE1,PKTValue1d.field.BYTE0);
				sprintf(msg1, "%d %d %d %d %d %d\n",SMMValued.field.LOW_BYTE1,SMMValued.field.LOW_BYTE0
				              ,PKTValue2d.field.BYTE3,PKTValue2d.field.BYTE2
				              ,PKTValue2d.field.BYTE1,PKTValue2d.field.BYTE0);

				retval=RtmpOSFileWrite(srcf, (PSTRING)msg, strlen(msg));
				retval=RtmpOSFileWrite(srcf, (PSTRING)msg1, strlen(msg1));
			}           
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("--> Error src  or srcf is null\n"));
		return FALSE;
	}

	retval=RtmpOSFileClose(srcf);
			
	if (retval)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> Error %d closing %s\n", -retval, src));
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);

	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BBP_R21_Ori);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R60, BBP_R60_Ori); 

	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, BBP_R142_ORI);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, BBP_R142_ORI);
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, PBF_SYS_CTRL_ORI);
	RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, PBF_CAP_CTRL_ORI);

	/* Finish */
	/* normal mode */
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &MACValue);
	MACValue &= ~0x00004000;
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, MACValue);

	/* reset packet buffer */
	RTMP_IO_WRITE32(pAd, PBF_CTRL,0x00000020 );

	/* enable Tx/Rx Queue */
	RTMP_IO_WRITE32(pAd, PBF_CFG, 0x00F40016);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0C);
	pAd->ate.Mode = ATE_STOP;

	return TRUE;
}


/* 0.1-second periodic execution */
VOID ATEPeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (RTMP_ADAPTER *)FunctionContext;
	PATE_INFO pATEInfo = &(pAd->ate);
	
	if (ATE_ON(pAd))
	{
		pATEInfo->PeriodicRound++;

#ifdef RT6352
		if (IS_RT6352(pAd))
		{
#ifdef RTMP_TEMPERATURE_CALIBRATION
			if ((pATEInfo->bAutoTxAlc == FALSE) && (pATEInfo->bQAEnabled == FALSE))
			{
				if (pATEInfo->bLowTemperature == TRUE)
				{
					INT32 wait;
					CHAR BbpR49;			
					UINT8 BBPValue;

					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

					if (!(BBPValue & (1<<4)))
					{
						BBPValue |= (1<<4);
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);		
					}

					/* polling BBP_R47 until BBP R47[4]==0 */
					for (wait=0; wait<200; wait++)
					{
						RTMPusecDelay(200);
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

						if (!(BBPValue & (1<<4)))
						{
							break;
						}
					}

					if ((BBPValue & 0x10) != 0)
						return;

					/* set BBP R47[3:0]=4 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
					BBPValue &= ~(0x0F);
					BBPValue |= (0x04);
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

					/* fetch temperature reading */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);
					pAd->CurrTemperature = (INT32)BbpR49;
					DBGPRINT(RT_DEBUG_INFO, ("%s: Current Temperature from BBP_R49=0x%x\n", __FUNCTION__, pAd->CurrTemperature));
				}
			}
			else
#endif /* RTMP_TEMPERATURE_CALIBRATION */
			if ((pATEInfo->bAutoTxAlc == TRUE) && (pATEInfo->Mode == ATE_TXFRAME))
			{
				OS_SEM_LOCK(&(pATEInfo->TssiSemLock));
				/* For MT7620, let QA come in for TSSI */
				ATEAsicAdjustTxPower(pAd);
				OS_SEM_UNLOCK(&(pATEInfo->TssiSemLock));
				ATEAsicExtraPowerOverMAC(pAd);
#ifdef RT6352_EP_SUPPORT
				if (pAd->bExtPA == FALSE)
				{
#endif /* RT6352_EP_SUPPORT */
					if (pATEInfo->bDPDEnable == TRUE)
					{
						if (pATEInfo->TxAntennaSel == ANT_0) /* TX0 */
							Set_TestDPDCalibrationTX0_Proc(pAd, "9");
						else if (pATEInfo->TxAntennaSel == ANT_1) /* TX1 */
							Set_TestDPDCalibrationTX1_Proc(pAd, "9");
						else /* both TX */
							Set_TestDPDCalibration_Proc(pAd, "9"); /* both TX */
					}
#ifdef RT6352_EP_SUPPORT
				}
#endif /* RT6352_EP_SUPPORT */
			}

#ifdef RTMP_TEMPERATURE_CALIBRATION
			if (pATEInfo->bQAEnabled == FALSE)
			{
				if (pATEInfo->bLowTemperature == TRUE)
				{
				RT6352_TemperatureCalibration(pAd);
				}
			}
#endif /* RTMP_TEMPERATURE_CALIBRATION */
		}
#endif /* RT6352 */

		if ((pATEInfo->bQAEnabled == FALSE) && (!IS_RT6352(pAd)))
		{
			if ((pATEInfo->bAutoTxAlc == TRUE) && (pATEInfo->Mode == ATE_TXFRAME))
			{
				ATEAsicAdjustTxPower(pAd);
			}
		}
	
		ATEAsicExtraPowerOverMAC(pAd);	

		/* Normal 1 second ATE PeriodicExec.*/
		if (pATEInfo->PeriodicRound % (ATE_TASK_EXEC_MULTIPLE) == 0)
		{
			pATEInfo->OneSecPeriodicRound++;

			/* for performace enchanement */
			NdisZeroMemory(&pAd->RalinkCounters,
							(UINT32)&pAd->RalinkCounters.OneSecEnd -
							(UINT32)&pAd->RalinkCounters.OneSecStart);
			NICUpdateRawCounters(pAd);

			if (pATEInfo->bRxFER == 1)
			{
				pATEInfo->RxTotalCnt += pATEInfo->RxCntPerSec;
		    	ate_print(KERN_EMERG "ATEPeriodicExec: Rx packet cnt = %d/%d\n",
							pATEInfo->RxCntPerSec, pATEInfo->RxTotalCnt);
				pATEInfo->RxCntPerSec = 0;

				if (pATEInfo->RxAntennaSel == 0)
					ate_print(KERN_EMERG "ATEPeriodicExec: Rx AvgRssi0=%d, AvgRssi1=%d, AvgRssi2=%d\n\n",
								pATEInfo->AvgRssi0, pATEInfo->AvgRssi1, pATEInfo->AvgRssi2);
				else
					ate_print(KERN_EMERG "ATEPeriodicExec: Rx AvgRssi=%d\n\n", pATEInfo->AvgRssi0);
			}

			MlmeResetRalinkCounters(pAd);

			/* In QA Mode, QA will handle all registers. */
			if (pATEInfo->bQAEnabled == TRUE)
			{
				return;
			}

#if (defined(RT3052) && !defined(RT3352)) || defined(RT3070)
			/* request by Gary, if Rssi0 > -42, BBP 82 need to be changed from 0x62 to 0x42, , bbp 67 need to be changed from 0x20 to 0x18*/
			if ((!pAd->CommonCfg.HighPowerPatchDisabled)
#ifdef RT3052
				&& (IS_RT3052B(pAd))
#endif /* RT3052 */
			)
			{
				{
					if ((pATEInfo->AvgRssi0 != 0) && (pATEInfo->AvgRssi0 > (pAd->BbpRssiToDbmDelta - 35))) 
					{ 
						RT30xxWriteRFRegister(pAd, RF_R27, 0x20); 
					} 
					else 
					{ 
						RT30xxWriteRFRegister(pAd, RF_R27, 0x23); 
					}
				}

#ifdef RT3052
				if (pAd->Antenna.field.RxPath == 2)
				{
					if ((pATEInfo->AvgRssi0 != 0) && (pATEInfo->AvgRssi0 > (pAd->BbpRssiToDbmDelta - 42) ))
					{
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x42);
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, 0x18);
					}
					else
					{
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, 0x20);
					}
				}
#endif /* RT3052 */
			}
#endif /* RT305x */
		}
	}
	else
	{
		DBGPRINT_ERR(("%s is NOT called in ATE mode.\n", __FUNCTION__));
	}

	return;
}


