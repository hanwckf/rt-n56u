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

#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
#define ATE_BBP_REG_NUM	168
UCHAR restore_BBP[ATE_BBP_REG_NUM]={0};
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */

/* 802.11 MAC Header, Type:Data, Length:24bytes + 6 bytes QOS/HTC + 2 bytes padding */
UCHAR TemplateFrame[32] = {0x08,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x00,0xAA,0xBB,0x12,0x34,0x56,0x00,0x11,0x22,0xAA,0xBB,0xCC,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

#if defined (RT305x) || defined (RT5350)
extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];

extern FREQUENCY_ITEM RtmpFreqItems3020[];
#else
extern FREQUENCY_ITEM *FreqItems3020;
#endif /* defined (RT3352) || defined (RT5350) */
extern UCHAR NUM_OF_3020_CHNL;


static UINT32 Default_TX_PIN_CFG;
#define RA_TX_PIN_CFG 0x1328
#define TXCONT_TX_PIN_CFG_A 0x040C0050
#define TXCONT_TX_PIN_CFG_G 0x080C00A0

static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1}; /* CCK Mode. */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1}; /* OFDM Mode. */
#ifdef DOT11N_SS3_SUPPORT
static CHAR HTMIXRateTable3T3R[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1}; /* HT Mix Mode for 3*3. */
#endif /* DOT11N_SS3_SUPPORT */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1}; /* HT Mix Mode. */

#ifdef RTMP_INTERNAL_TX_ALC

/* The desired TSSI over CCK */
extern CHAR desiredTSSIOverCCK[4];

/* The desired TSSI over OFDM */
extern CHAR desiredTSSIOverOFDM[8];

/* The desired TSSI over HT */
extern CHAR desiredTSSIOverHT[16];

/* The desired TSSI over HT using STBC */
extern CHAR desiredTSSIOverHTUsingSTBC[8];

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
	ATE_INFO *pATEInfo = &(pAd->ate);
	UCHAR desiredTSSI = 0;
	UCHAR MCS = 0;
	UCHAR MaxMCS = 7;

	MCS = (UCHAR)(pATEInfo->TxWI.MCS);
	
	if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
	{
		if (MCS > 3) /* boundary verification */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
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
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));

			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverOFDM[MCS];
	}
	else if ((pATEInfo->TxWI.PHYMODE == MODE_HTMIX) || (pATEInfo->TxWI.PHYMODE == MODE_HTGREENFIELD))
	{
		MaxMCS = pAd->chipCap.TxAlcMaxMCS - 1;

		if (MCS > MaxMCS) /* boundary verification */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));

			MCS = 0;
		}

		if (pATEInfo->TxWI.STBC == 1)
		{
			desiredTSSI = desiredTSSIOverHT[MCS];
		}
		else
		{
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

	return desiredTSSI;
}

#endif /* RTMP_INTERNAL_TX_ALC */


/*
==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in ATE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. if current radio is a noisy environment (pAd->DrsCounters.fNoisyEnvironment)
		1. TxPowerPercentage
		2. auto calibration based on TSSI feedback
		3. extra 2 db for CCK
		4. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

	NOTE: Since this routine requires the value of (pAd->DrsCounters.fNoisyEnvironment),
		it should be called AFTER MlmeDynamicTxRateSwitching()
==========================================================================
*/
VOID ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	INT			i, j, maxTxPwrCnt;
	CHAR		DeltaPwr = 0;
	BOOLEAN		bAutoTxAgc = FALSE;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep;
	UCHAR		BbpR49 = 0, idx;
	PCHAR		pTxAgcCompensate;
	ULONG		TxPwr[9];	/* NOTE: the TxPwr array size should be the maxima value of all supported chipset!!!! */
	CHAR		Value;
#ifdef RTMP_INTERNAL_TX_ALC
	/* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */    
	CHAR             TotalDeltaPower = 0; 
	UCHAR desiredTSSI = 0, currentTSSI = 0;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable = pAd->chipCap.TxPowerTuningTable_2G;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	UCHAR RFValue = 0, TmpValue = 0;   
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT3883(pAd))
	{
		maxTxPwrCnt = 9;
	}
	else if (IS_RT2883(pAd))
	{
		maxTxPwrCnt = 7;
	}
	else
#endif /* DOT11N_SS3_SUPPORT */
		maxTxPwrCnt = 5;

	if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel > 14)
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx40MPwrCfgABand[i];	
			}
		}
		else
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx40MPwrCfgGBand[i];	
			}
		}
	}
	else
	{
		if (pAd->ate.Channel > 14)
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx20MPwrCfgABand[i];	
			}
		}
		else
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx20MPwrCfgGBand[i];	
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
				if ((pAd->ate.Channel >= 1) && (pAd->ate.Channel <= 14))	
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s: bExtendedTssiMode = %d, original desiredTSSI = %d, CentralChannel = %d, PerChTxPwrOffset = %d\n", 
						__FUNCTION__, 
						pAd->TxPowerCtrl.bExtendedTssiMode, 
						desiredTSSI, 
						pAd->ate.Channel, 
						pAd->TxPowerCtrl.PerChTxPwrOffset[pAd->ate.Channel]));

					desiredTSSI += pAd->TxPowerCtrl.PerChTxPwrOffset[pAd->ate.Channel];
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
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)(&RFValue));
				RFValue &= ~0x3F; /* clear RF_R49[5:0] */
				RFValue |= pAd->TxPowerCtrl.RF_TX_ALC;
				DBGPRINT(RT_DEBUG_TRACE, ("Write RF_R49 = 0x%x\n", RFValue));
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);
			}
			else if (IS_RT3352(pAd))
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R47, (PUCHAR)(&RFValue));
				RFValue &= ~0x1F; /* clear RF_R47[4:0] */
				RFValue |= pAd->TxPowerCtrl.RF_TX_ALC;
				DBGPRINT(RT_DEBUG_TRACE, ("Write RF_R47 = 0x%x\n", RFValue));
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R47, (UCHAR)RFValue);
			}
			else
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)(&RFValue));
				TmpValue = (RFValue & 0xE0);
				RFValue = (TmpValue | (pAd->TxPowerCtrl.RF_TX_ALC & 0x1F));
				DBGPRINT(RT_DEBUG_TRACE, ("Write RF_R12 = 0x%x\n", RFValue));            
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)(RFValue));
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
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
		if (pAd->ate.Channel <= 14)
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
		if (pAd->ate.Channel <= 14)
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

	/* Calculate delta power based on the percentage specified from UI. */
	/* E2PROM setting is calibrated for maximum TX power (i.e. 100%) */
	/* We lower TX power here according to the percentage specified from UI. */
	if (pAd->CommonCfg.TxPowerPercentage == 0xffffffff)       /* AUTO TX POWER control */
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 90)  /* 91 ~ 100% & AUTO, treat as 100% in terms of mW */
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60)  /* 61 ~ 90%, treat as 75% in terms of mW */
	{
		DeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30)  /* 31 ~ 60%, treat as 50% in terms of mW */
	{
		DeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15)  /* 16 ~ 30%, treat as 25% in terms of mW */
	{
		DeltaPwr -= 6;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9)   /* 10 ~ 15%, treat as 12.5% in terms of mW */
	{
		DeltaPwr -= 9;
	}
	else                                           /* 0 ~ 9 %, treat as MIN(~3%) in terms of mW */
	{
		DeltaPwr -= 12;
	}

	/* Reset different new tx power for different TX rate. */
	for (i=0; i<maxTxPwrCnt; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */

#ifdef RTMP_INTERNAL_TX_ALC
				/*
					The upper bounds of the MAC 0x1314~0x1324 
					are variable when the STA uses the internal Tx ALC.
				*/
				if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
				{
					switch (TX_PWR_CFG_0 + (i * 4))
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
							if ((j >= 0) && (j <= 3))
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
							if ((j == 0) || (j == 2) || (j == 3))
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
							if ((j == 0) || (j == 2) || (j == 3) || 
							((j >= 4) && (j <= 7)))
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
							DBGPRINT(RT_DEBUG_ERROR, ("%s: unknown register = 0x%X\n", 
								__FUNCTION__, 
							(TX_PWR_CFG_0 + (i * 4))));
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
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
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
				if (i == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, TxPwr[i]);
				}
				else if (i == 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, TxPwr[i]);
				}
#ifdef RT3883
				else if ((IS_RT3883(pAd)) && (i == 7))
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, TxPwr[i]);
				}
				else if ((IS_RT3883(pAd)) && (i == 8))
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, TxPwr[i]);
				}
#endif /* RT3883 */
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + (i << 2), (TxPwr[i] & 0xf0f0f0f0) >> 4);
				}
			}
			else
			{
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);
			}	
#else
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);
#endif /* DOT11N_SS3_SUPPORT */

			
		}
	}

}


CHAR ATEConvertToRssi(
	IN PRTMP_ADAPTER pAd,
	IN	CHAR	Rssi,
	IN  UCHAR   RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;

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

	return (-12 - RssiOffset - LNAGain - Rssi);
}


VOID ATESampleRssi(
	IN PRTMP_ADAPTER	pAd,
	IN PRXWI_STRUC		pRxWI)
{
	if (pRxWI->RSSI0 != 0)
	{
		pAd->ate.LastRssi0	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI0, RSSI_0);
		pAd->ate.AvgRssi0X8	= (pAd->ate.AvgRssi0X8 - pAd->ate.AvgRssi0) + pAd->ate.LastRssi0;
		pAd->ate.AvgRssi0  	= pAd->ate.AvgRssi0X8 >> 3;
	}
	if (pRxWI->RSSI1 != 0)
	{
		pAd->ate.LastRssi1	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI1, RSSI_1);
		pAd->ate.AvgRssi1X8	= (pAd->ate.AvgRssi1X8 - pAd->ate.AvgRssi1) + pAd->ate.LastRssi1;
		pAd->ate.AvgRssi1	= pAd->ate.AvgRssi1X8 >> 3;
	}
	if (pRxWI->RSSI2 != 0)
	{
		pAd->ate.LastRssi2	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI2, RSSI_2);
		pAd->ate.AvgRssi2X8	= (pAd->ate.AvgRssi2X8 - pAd->ate.AvgRssi2) + pAd->ate.LastRssi2;
		pAd->ate.AvgRssi2	= pAd->ate.AvgRssi2X8 >> 3;
	}

	pAd->ate.LastSNR0 = (CHAR)(pRxWI->SNR0);/* CHAR ==> UCHAR ? */
	pAd->ate.LastSNR1 = (CHAR)(pRxWI->SNR1);/* CHAR ==> UCHAR ? */
#ifdef DOT11N_SS3_SUPPORT
	pAd->ate.LastSNR2 = (CHAR)(pRxWI->SNR2);/* CHAR ==> UCHAR ? */
#endif /* DOT11N_SS3_SUPPORT */

	pAd->ate.NumOfAvgRssiSample ++;
}


#ifdef RTMP_MAC_PCI
static INT TxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */

	if (GloCfg.field.TxDMABusy)
		result = TRUE;
	else
		result = FALSE;

	return result;
}


static INT RxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */

	if (GloCfg.field.RxDMABusy)
		result = TRUE;
	else
		result = FALSE;

	return result;
}


static VOID RtmpDmaEnable(
	IN PRTMP_ADAPTER pAd,
	IN INT Enable)
{
	BOOLEAN value;
	ULONG WaitCnt;
	WPDMA_GLO_CFG_STRUC GloCfg;
	
	value = Enable > 0 ? 1 : 0;

	/* check if DMA is in busy mode or not. */
	WaitCnt = 0;

	while (TxDmaBusy(pAd) || RxDmaBusy(pAd))
	{
		RTMPusecDelay(10);

		if (WaitCnt++ > 100)
			break;
	}
	
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
	GloCfg.field.EnableTxDMA = value;
	GloCfg.field.EnableRxDMA = value;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);	/* abort all TX rings */
	RtmpOsMsDelay(5);

	return;
}
#endif /* RTMP_MAC_PCI */




VOID rt_ee_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT i;
	USHORT value;

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{	
		rtmp_ee_flash_read_all(pAd, Data);
		return;
	}
#endif /* RTMP_RBUS_SUPPORT */

	for (i = 0 ; i < (EEPROM_SIZE >> 1) ; )
	{
		/* "value" is especially for some compilers... */
		RT28xx_EEPROM_READ16(pAd, (i << 1), value);
		Data[i] = value;
		i++;
	}
}


VOID rt_ee_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT i;
	USHORT value;

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
		rtmp_ee_flash_write_all(pAd, Data);
		return;
	}
#endif /* RTMP_RBUS_SUPPORT */

#ifdef RTMP_FLASH_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_USB)
	{
		/* for RT3352+RT3572 solution */
		rtmp_ee_flash_write_all(pAd, Data);
		return;
	}
#endif /* RTMP_FLASH_SUPPORT */


	for (i = 0 ; i < (EEPROM_SIZE >> 1) ; )
	{
		/* "value" is especially for some compilers... */
		value = Data[i];
		RT28xx_EEPROM_WRITE16(pAd, (i << 1), value);
		i++;
	}

	return;
}


VOID rt_ee_write_bulk(PRTMP_ADAPTER pAd, USHORT *Data, USHORT offset, USHORT length)
{
	USHORT pos;
	USHORT value;
	USHORT len = length;

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
		for (pos = 0; pos < (len >> 1);)
		{
			value = Data[pos];
			rtmp_ee_flash_write(pAd, offset+(pos*2), value);
			pos++;
		}
		
		return;
	}
#endif /* RTMP_RBUS_SUPPORT */

	for (pos = 0; pos < (len >> 1);)
	{
		/* "value" is especially for some compilers... */
		value = Data[pos];
		RT28xx_EEPROM_WRITE16(pAd, offset+(pos*2), value);
		pos++;
	}

	return;
}


static VOID RtmpRfIoWrite(
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



VOID ATEAsicSetTxRxPath(
    IN PRTMP_ADAPTER pAd)
{
}


#ifdef RT3883 
extern UCHAR NUM_OF_3883_CHNL; 
extern FREQUENCY_ITEM FreqItems3883[];


VOID RT3883_ATE_TxAntennaSelect(
    IN PRTMP_ADAPTER pAd) 
{
	UINT32 MACValue = 0;
	UCHAR RFValue = 0;
	UCHAR Channel;
	CHAR TxAntennaSel;
	BOOLEAN b5GBand;

	Channel = pAd->ate.Channel;
	TxAntennaSel = pAd->ate.TxAntennaSel;
	b5GBand = (Channel > 14) ? TRUE : FALSE;

	/* MAC registers */
	/* for RT3883 stream mode */
	if (IS_RT3883(pAd))
	{		
		if (TxAntennaSel == 1)
		{			
			MACValue = 0x0000FFFF;		
		}
		else if (TxAntennaSel == 2)
		{			
			MACValue = 0x0001FFFF;
		}	
		else if (TxAntennaSel == 3)
		{			
			MACValue = 0x0002FFFF;
		}
		else
		{
			/* 3T */
			MACValue = 0x0003FFFF;
		}

		RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_L, 0xFFFFFFFF);
		RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR0_H, MACValue);
	}

	if (TxAntennaSel == 0)
	{
		/* 3T */
		if (b5GBand == TRUE)
		{
			/* A band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x01000005;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
		else
		{
			/* G band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x0200000A;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
	}
	else if (TxAntennaSel == 1)
	{
		/* TX 0 */
		if (b5GBand == TRUE)
		{
			/* A band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x00000001;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
		else
		{
			/* G band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x00000002;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
	}
	else if (TxAntennaSel == 2)
	{
		/* TX 1 */
		if (b5GBand == TRUE)
		{
			/* A band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x00000004;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
		else
		{
			/* G band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x00000008;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
	}
	else if (TxAntennaSel == 3)
	{
		/* TX 2 */
		if (b5GBand == TRUE)
		{
			/* A band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x01000000;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
		else
		{
			/* G band */
			RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
			MACValue &= ~(0x0300000F);
			MACValue |= 0x02000000;
			RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
		}
	}
	else
	{
		DBGPRINT_ERR(("Tx antenna selected does not exist!\n"));
		return;
	}

	/* RF registers */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
	RFValue = (RFValue & 0x57) | 0x03;
	
	if (TxAntennaSel == 1)
	{
		/* TX 0 */
		RFValue = RFValue | (1 << 3);
	}
	else if (TxAntennaSel == 2)
	{
		/* TX 1 */
		RFValue = RFValue | (1 << 5);
	}
	else if (TxAntennaSel == 3)
	{
		/* TX 2 */
		RFValue = RFValue | (1 << 7);
	}
	else
	{
		/* TX All */
		RFValue = RFValue | ((1 << 3) | (1 << 5) | (1 << 7));
	}

	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

	return;
}


VOID RT3883_ATE_RxAntennaSelect(
    IN PRTMP_ADAPTER pAd)
{
	UINT32 MACValue = 0;
	UCHAR BbpValue = 0;
	UCHAR RFValue = 0;
	UCHAR Channel;
	CHAR RxAntennaSel;
	BOOLEAN b5GBand;

	Channel = pAd->ate.Channel;
	RxAntennaSel = pAd->ate.RxAntennaSel;
	b5GBand = (Channel > 14) ? TRUE : FALSE;

	/* MAC registers */
	if ((RxAntennaSel >= 0) && (RxAntennaSel <= 3))
	{
		RTMP_IO_READ32(pAd, TX_PIN_CFG, &MACValue);
		MACValue |= 0x30000F00;
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MACValue);
	}
	else
	{
		DBGPRINT_ERR(("Rx antenna selected does not exist!\n"));
		return;
	}

	/* BBP registers */
	if (pAd->Antenna.field.RxPath == 3)
	{
		if (b5GBand == FALSE)
		{
			BbpValue = 0x10;
		}
		else
		{
			if (pAd->ate.Channel < 132)
			{
				/* lower frequency channel */
				BbpValue = 0x10;
			}
			else
			{
				/* higher frequency channel */
				BbpValue = 0x30;
			}
		}
	}
	else if (pAd->Antenna.field.RxPath == 2)
	{
		BbpValue = 0x08;
	}
	else
	{
		ASSERT(pAd->Antenna.field.RxPath == 1);
		BbpValue = 0x00;
	}

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, (UCHAR)BbpValue);

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);

	/* clear bit 0,1,3,4 */
	BbpValue = BbpValue & 0xE4; 
	if (RxAntennaSel == 1)
	{
		BbpValue |= 0x0;
	}
	else if (RxAntennaSel == 2)
	{
		BbpValue |= 0x1;
	}
	else if (RxAntennaSel == 3)
	{
		BbpValue |= 0x2;
	}
	else
	{
		/* assume that all RxAntenna are enabled */
		/* (Gary, 2010-06-02) */
		BbpValue |= 0x10;
	}
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

	/* RF registers */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
	RFValue = (RFValue & 0xAB) | 0x03;
	
	if (RxAntennaSel == 1)
	{
		/* RX 0 */
		RFValue = RFValue | (1 << 2);
	}
	else if (RxAntennaSel == 2)
	{
		/* RX 1 */
		RFValue = RFValue | (1 << 4);
	}
	else if (RxAntennaSel == 3)
	{
		/* RX 2 */
		RFValue = RFValue | (1 << 6);
	}
	else
	{
		/* RX All */
		RFValue = RFValue | ((1 << 2) | (1 << 4) | (1 << 6));
	}

	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

	return;
}
#endif /* RT3883 */


/*
==========================================================================
    Description:

	Default AsicSwitchChannel() dedicated for ATE.
    
==========================================================================
*/
VOID DefaultATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR BbpValue = 0, R66 = 0x30, Channel = 0;

#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;


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
			if (pAd->ate.TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pAd->ate.TxAntennaSel == 2)
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
		if (pAd->ate.TxWI.BW == BW_20)
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


#ifdef RT3883
/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3883 ATE.
    
==========================================================================
*/
VOID RT3883ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;
	CHAR TxPwer3 = 0;

#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;
	TxPwer3 = pAd->ate.TxPower2;

	RTMPRT3883ABandSel(Channel);

	for (index = 0; index < NUM_OF_3883_CHNL; index++)
	{
		if (Channel == FreqItems3883[index].Channel)
		{
			UCHAR diff = 0;
			UCHAR BbpValue = 0;

			/* RF_R06 for A-Band L:0x80 M:0x80 H:0x40 (Gary, 2010-06-02) */
			if (Channel <= 14)
			{
				RFValue = 0x40;
			}
			else
			{
				if (Channel < 132)
				{
					RFValue = 0x80;
				}
				else
				{
					RFValue = 0x40;
				}
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);
			
			/* programming channel parameters */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, FreqItems3883[index].N);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, FreqItems3883[index].K);
			
			if (Channel <= 14)
				RFValue = 0x46;
			else
				RFValue = 0x48;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)RFValue);

			
			if (Channel <= 14)
				RFValue = 0x1A;/* Gary, 2011-03-10 */
			else
				RFValue = 0x52;/* Gary, 2011-03-10 */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

			RFValue = 0x12;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);
				
			/* antenna selection */
			RT3883_ATE_RxAntennaSelect(pAd);/* (Gary, 2010-11-18) */
			RT3883_ATE_TxAntennaSelect(pAd);/* (Gary, 2010-11-18) */

			if (pAd->ate.TxWI.BW == BW_20)
			{
				/* set BBP R4 = 0x40 for BW = 20 MHz */
		 		BbpValue = 0x40;
		 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpValue);
			}
			else
			{
				/* set BBP R4 = 0x50 for BW = 40 MHz */
		 		BbpValue = 0x50;
		 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpValue);
			}

			/* set RF frequency offset */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);
			RFValue2 = (RFValue & 0x80) | pAd->ate.RFFreqOffset;
			if (RFValue2 > RFValue)
			{
				for (diff = 1; diff <= (RFValue2 - RFValue); diff++)
				{
					RtmpOsMsDelay(1);						
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue + diff));
				}	
			}	
			else
			{
				for (diff = 1; diff <= (RFValue - RFValue2); diff++)
				{
					RtmpOsMsDelay(1);						
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue - diff));
				}
			}

			/* (Gary, 2010-06-02) */
			RFValue = 0x20;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R22, (UCHAR)RFValue);

			/* different default setting for A/BG bands */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);
			if (pAd->ate.TxWI.BW == BW_20)
				/* 20MBW Bit[2:1]=0,0 */
				RFValue &= ~(0x06); 
			else
				RFValue |= 0x06;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

			/* (Gary, 2010-06-02) */
			if (Channel <= 14)
				RFValue = 0xA0;
			else
				RFValue = 0x80;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, (UCHAR)RFValue);
			
			/* (Gary, 2011-08-25) */
			if (pAd->ate.TxWI.BW == BW_20)
				RFValue = 0xD8;
			else
				RFValue = 0x80;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, (UCHAR)RFValue);
			
			/* (Gary, 2011-08-25) */
			RFValue = 0x3F;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R33, (UCHAR)RFValue);

			if (Channel <= 14)
				RFValue = 0x3C;
			else
				RFValue = 0x20;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R34, (UCHAR)RFValue);

			/* loopback RF_BS */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R36, (PUCHAR)&RFValue);
			RFValue &= ~(0x1 << 7);
			if (Channel <= 14)
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R36, (UCHAR)(RFValue  | (0x1 << 7)));
			else
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R36, (UCHAR)RFValue);

			/* RF_R39 for A-Band L:0x36 M:0x32 H:0x30 */
			if (Channel > 14)
			{
				if (Channel < 100)
				{
					RFValue = 0x36;
				}
				else if (Channel < 132)
				{
					RFValue = 0x32;
				}
				else
				{
					RFValue = 0x30;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);
			}
			else
			{
				/* G band */
				RFValue = 0x23;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);
			}

#ifdef TXBF_SUPPORT
			if (pAd->ate.bTxBF == TRUE)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R39, (PUCHAR)&RFValue);
				RFValue |= 0x40;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R39, (UCHAR)RFValue);
			}
#endif /* TXBF_SUPPORT */		

			/* loopback RF_BS */
			if  (Channel <= 14)
				RFValue = 0x93;
			else
				RFValue = 0x9B;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R44, (UCHAR)RFValue);

			/* RF_R45 for A-Band L:0xEB M:0xB3 H:0x9B */
			if (Channel > 14)
			{
				if (Channel < 100)
				{
					RFValue = 0xEB;
				}
				else if (Channel < 132)
				{
					RFValue = 0xB3;
				}
				else
				{
					RFValue = 0x9B;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R45, (UCHAR)RFValue);
			}
			else
			{
				/* G band */
				RFValue = 0xBB;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R45, (UCHAR)RFValue);
			}
			
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = 0x8E;
			else
				RFValue = 0x8A;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);

#ifdef TXBF_SUPPORT
			if (pAd->ate.bTxBF == TRUE)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)&RFValue);
				RFValue |= 0x20;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);
			}
#endif /* TXBF_SUPPORT */			

			RFValue = 0x86;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R50, (UCHAR)RFValue);
			
			/* tx_mx1_ic */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R51, (PUCHAR)&RFValue);
			if  (Channel <= 14)
				RFValue = 0x75;
			else
				RFValue = 0x51;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R51, (UCHAR)RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R52, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = 0x45;
			else
				RFValue = 0x05;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R52, (UCHAR)RFValue);

			/* tx0, tx1, tx2 (0.5db) */
			if (Channel <= 14)
			{
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, pAd->ate.TxPower0);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, pAd->ate.TxPower1);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, pAd->ate.TxPower2);
			}
			else
			{
				/* (Gary, 2010-02-12) */
				CHAR power = 0x48 | ((pAd->ate.TxPower0 & 0x18) << 1) | (pAd->ate.TxPower0 & 0x7);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, power);
				/* (Gary, 2010-02-12) */
				power = 0x48 | ((pAd->ate.TxPower1 & 0x18) << 1) | (pAd->ate.TxPower1 & 0x7);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, power);
				/* (Gary, 2010-02-12) */
				power = 0x48 | ((pAd->ate.TxPower2 & 0x18) << 1) | (pAd->ate.TxPower2 & 0x7);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, power);
			}

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R57, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = 0x6E;
			else
				RFValue = 0x3E;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R57, (UCHAR)RFValue);

			/* Enable RF tuning, this must be in the last, RF_R03=RF_R07 (RT30xx). */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x80; /* bit 7=vcocal_en */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);

			RtmpOsMsDelay(2);

			/* latch channel for future usage */
			pAd->LatchRfRegs.Channel = Channel;

#ifdef TXBF_SUPPORT
			/* Do a Divider Calibration and update BBP registers */
			if (pAd->ate.bTxBF)
			{
				ITxBFLoadLNAComp(pAd);
				ITxBFDividerCalibration(pAd, 2, 0, NULL);
			}
#endif /* TXBF_SUPPORT */
			break;
		}
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* for peak throughput, Henry 2009-12-23 */
		if (pAd->ate.RxAntennaSel == 0)
		{
			/* assume that all RxAntenna are enabled */
			BbpValue = 0x46;
		}
		else
		{
			/* assume that only one RxAntenna is enabled */
			BbpValue = 0x00;
		}
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, BbpValue);

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

		/* G band */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x82);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x8A);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x34);
	}
	/* channel > 14 */
	else
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* for peak throughput, Henry 2009-12-23 */
		if (pAd->ate.RxAntennaSel == 0)
		{
			/* assume that all RxAntenna are enabled */
			BbpValue = 0x46;
		}
		else
		{
			/* assume that only one RxAntenna is enabled */
			BbpValue = 0x00;
		}
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, BbpValue);

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x82);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x9A);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x34);
	}

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	/*
		On 11A, We should delay and wait RF/BBP to be stable
		and the appropriate time should be 1000 micro seconds. 

		2005/06/05 - On 11G, We also need this delay time.
		Otherwise it's difficult to pass the WHQL.
	*/
	RtmpOsMsDelay(1);  

#ifdef RTMP_RF_RW_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("RT3883:SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, Pwr2=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		TxPwer3,
		pAd->Antenna.field.TxPath,
		FreqItems3883[index].N, 
		FreqItems3883[index].K, 
		FreqItems3883[index].R));	
#endif /* RTMP_RF_RW_SUPPORT */
}
#endif /* RT3883 */


#ifdef RT305x
#ifdef RT3050
/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3050 ATE.
    
==========================================================================
*/
VOID RT3050ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
#ifdef RTMP_RF_RW_SUPPORT
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;

	if ((pAd->RfIcType == RFIC_3022) || (pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3020) || \
		(pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3322))
	{
		UINT32 step=0;
		
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			/* Please don't change "RtmpFreqItems3020" to "FreqItems3020" ! */
			if (Channel == RtmpFreqItems3020[index].Channel)
			{
				/* programming channel parameters */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, RtmpFreqItems3020[index].N);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, RtmpFreqItems3020[index].K);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xFC) | RtmpFreqItems3020[index].R;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);

				/* set tx power0 */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

				/* set tx power1 */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R13, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer2;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);

				/* set RF offset */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
				RFValue2 = (RFValue & 0x80) | pAd->ate.RFFreqOffset;

				if (RFValue2 > RFValue)
				{
					for (step = 1; step <= (RFValue2 - RFValue); step++)
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)(RFValue + step));
						RtmpOsMsDelay(10);
					}
				}	
				else
				{
					for (step = 1; step <= (RFValue - RFValue2); step++)
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)(RFValue - step));
						RtmpOsMsDelay(10);
					}
				}

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R24, (PUCHAR)&RFValue);
				RFValue &= 0xDF;
				if (pAd->ate.TxWI.BW == BW_40)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x2F);
					RFValue |= 0x20;
				}
				else
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x0F);
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);

				/* RF Rx antenna selection */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0x03) | 0xC1;

				if (pAd->ate.RxAntennaSel == 1)
				{
					RFValue = RFValue | 0x10;
				}
				else if (pAd->ate.RxAntennaSel == 2)
				{
					RFValue = RFValue | 0x04;
				}

				/* RF Tx antenna selection */					
				if (pAd->ate.TxAntennaSel == 1)
				{
					RFValue = RFValue | 0x20;
				}
				else if (pAd->ate.TxAntennaSel == 2)
				{
					RFValue = RFValue | 0x08;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

				/* BBP Rx antenna selection */
				if (pAd->ate.RxAntennaSel == 1)
				{
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0x0);
				}
				else if (pAd->ate.RxAntennaSel == 2)
				{
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0x1);
				}
				else
				{
					/* RXANT_NUM >= 2, must turn on Bit 0 and 1 for all ADCs */
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0xB);
				}

				/* enable RF tuning */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R07, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x1;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R07, (UCHAR)RFValue);

				/* latch channel for future usage */
				pAd->LatchRfRegs.Channel = Channel;
				break;
			}
		}
	}
	
	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A;/* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* According the Rory's suggestion to solve the middle range issue. */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);	

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
			if (pAd->ate.TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pAd->ate.TxAntennaSel == 2)
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

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

#ifdef RTMP_RF_RW_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("RT3050:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		RtmpFreqItems3020[index].N, 
		RtmpFreqItems3020[index].K, 
		RtmpFreqItems3020[index].R));
#endif /* RTMP_RF_RW_SUPPORT */
}
#endif /* RT3050 */




#ifdef RT3350
/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3350 ATE.
    
==========================================================================
*/
VOID RT3350ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
#ifdef RTMP_RF_RW_SUPPORT
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;

	if ((pAd->RfIcType == RFIC_3022) || (pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3020) || \
		(pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3322))
	{
		UINT32 step=0;
		
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			/* Please don't change "RtmpFreqItems3020" to "FreqItems3020" ! */
			if (Channel == RtmpFreqItems3020[index].Channel)
			{
				/* programming channel parameters */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, RtmpFreqItems3020[index].N);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xF0) | (RtmpFreqItems3020[index].K & 0x0F);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xFC) | RtmpFreqItems3020[index].R;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);

				/* set tx power0 */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

				/* set tx power1 */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R13, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer2;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);

				/* set RF offset */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
				RFValue2 = (RFValue & 0x80) | pAd->ate.RFFreqOffset;

				if (RFValue2 > RFValue)
				{
					for (step = 1; step <= (RFValue2 - RFValue); step++)
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)(RFValue + step));
						RtmpOsMsDelay(10);
					}
				}	
				else
				{
					for (step = 1; step <= (RFValue - RFValue2); step++)
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)(RFValue - step));
						RtmpOsMsDelay(10);
					}
				}

				/* set RF_R24 and RF_R31 */
				if (pAd->ate.TxWI.BW == BW_40)
				{    
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x68);

					if(pAd->ate.TxWI.PHYMODE == MODE_CCK)
						RFValue = 0x3F;
					else
						RFValue = 0x28;
				}
				else
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x48);

					if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
						RFValue = 0x1F;
					else if (pAd->ate.TxWI.PHYMODE == MODE_OFDM)
						RFValue = 0x18;
					else
						RFValue = 0x10;/* Simba:2010.02.04 */
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);

				/* RF Rx antenna selection */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0x03) | 0xC1;

				if (pAd->ate.RxAntennaSel == 1)
				{
					RFValue = RFValue | 0x10;
				}
				else if (pAd->ate.RxAntennaSel == 2)
				{
					RFValue = RFValue | 0x04;
				}

				/* RF Tx antenna selection */					
				if (pAd->ate.TxAntennaSel == 1)
				{
					RFValue = RFValue | 0x20;
				}
				else if (pAd->ate.TxAntennaSel == 2)
				{
					RFValue = RFValue | 0x08;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

				/* BBP Rx antenna selection */
				if (pAd->ate.RxAntennaSel == 1)
				{
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0x0);
				}
				else if (pAd->ate.RxAntennaSel == 2)
				{
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0x1);
				}
				else
				{
					/* RXANT_NUM >= 2, must turn on Bit 0 and 1 for all ADCs */
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0xB);
				}

				/* enable RF tuning */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R07, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x1;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R07, (UCHAR)RFValue);

				/* latch channel for future usage */
				pAd->LatchRfRegs.Channel = Channel;
				break;
			}
		}
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A;/* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* According the Rory's suggestion to solve the middle range issue. */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);	

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
			if (pAd->ate.TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pAd->ate.TxAntennaSel == 2)
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

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

#ifdef RTMP_RF_RW_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("RT3350:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		RtmpFreqItems3020[index].N, 
		RtmpFreqItems3020[index].K, 
		RtmpFreqItems3020[index].R));
#endif /* RTMP_RF_RW_SUPPORT */
}
#endif /* RT3350 */


#ifdef RT3352
/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3352 ATE.
    
==========================================================================
*/
VOID RT3352ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
#ifdef RTMP_RF_RW_SUPPORT
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;

	if ((pAd->RfIcType == RFIC_3022) || (pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3020) || \
		(pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3322))
	{
		UINT32 step=0;
		
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			/* Please don't change "RtmpFreqItems3020" to "FreqItems3020" ! */
			if (Channel == RtmpFreqItems3020[index].Channel)
			{
				/* programming channel parameters */
				step = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

				if (step & (1<<20))
				{ 
					/* Xtal=40M */
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, RtmpFreqItems3020[index].N);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, RtmpFreqItems3020[index].K);
				}
				else
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, FreqItems3020_Xtal20M[index].N);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, FreqItems3020_Xtal20M[index].K);
				}

				RFValue = 0x42;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)RFValue);
				RFValue = 0x1C;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

				RFValue = 0x00;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);

				/* set RF offset RF_R17=RF_R23 */ 
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);
				RFValue2 = (RFValue & 0x80) | pAd->ate.RFFreqOffset;
				if (RFValue2 > RFValue)
				{
					for (step = 1; step <= (RFValue2 - RFValue); step++)
					{
						RtmpOsMsDelay(1);
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue + step));
					}
				}	
				else
				{
					for (step = 1; step <= (RFValue - RFValue2); step++)
					{
						RtmpOsMsDelay(1);
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue - step));
					}
				}

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);

				if (pAd->ate.TxWI.BW == BW_20)
					RFValue &= ~(0x03); /* 20MBW tx_h20M=0, rx_h20M=0 */
				else
					RFValue |= 0x03; /* 40MBW tx_h20M=1, rx_h20M=1 */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R47, pAd->ate.TxPower0);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R48, pAd->ate.TxPower1);

				/* set BW */
				/* RF_R24 is reserved bits */
				RFValue = 0; 
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);

				/* Enable RF tuning, this must be in the last, RF_R03=RF_R07. */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x80; /* bit 7=vcocal_en */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);

				RtmpOsMsDelay(2);

				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, RFValue & 0xfe); /* clear update flag */
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);

				/* Antenna */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0x03) | 0x01;
				if (pAd->ate.RxAntennaSel == 1)
				{
					RFValue = RFValue | 0x04; /* rx0_en */
				}
				else if (pAd->ate.RxAntennaSel == 2)
				{
					RFValue = RFValue | 0x10; /* rx1_en */
				}
				else 
				{
					RFValue = RFValue | 0x14; /* rx0_en and rx1_en */
				}

				if (pAd->ate.TxAntennaSel == 1)
				{
					RFValue = RFValue | 0x08; /* tx0_en */
				}
				else if (pAd->ate.TxAntennaSel == 2)
				{
					RFValue = RFValue | 0x20; /* tx1_en */
				}
				else
				{
					RFValue = RFValue | 0x28; /* tx0_en and tx1_en */
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
				BbpValue = BbpValue & 0x24; /* clear bit 0,1,3,4,6,7 */

				if (pAd->ate.RxAntennaSel == 1)
				{
					BbpValue &= ~0x3; /* BBP_R3[1:0]=00 (ADC0) */
				}
				else if (pAd->ate.RxAntennaSel == 2)
				{
					BbpValue &= ~0x3; 
					BbpValue |= 0x1; /* BBP_R3[1:0]=01 (ADC1) */
				}
				else
				{
					/* RXANT_NUM >= 2, must turn on Bit 0 and 1 for all ADCs */
					BbpValue |= 0x3;  /* BBP_R3[1:0]=11 (All ADCs) */
					if (pAd->Antenna.field.RxPath == 3)
						BbpValue |= (1 << 4);
					else if (pAd->Antenna.field.RxPath == 2)
						BbpValue |= (1 << 3);  /* BBP_R3[3]=1 (2R) */
					else if (pAd->Antenna.field.RxPath == 1)
						BbpValue |= (0x0);
				}

				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

				if (pAd->ate.TxWI.BW == BW_20)
				{
					/* set BBP R4 = 0x40 for BW = 20 MHz */
					BbpValue = 0x40;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpValue);
				}
				else
				{
					/* set BBP R4 = 0x50 for BW = 40 MHz */
					BbpValue = 0x50;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpValue);
				}

				/* calibrate power unbalance issues */
				if (pAd->Antenna.field.TxPath == 2)
				{
					if (pAd->ate.TxAntennaSel == 1)
					{
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7; /* DAC0 (11100111B) */
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
					}
					else if (pAd->ate.TxAntennaSel == 2)
					{
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;	
						BbpValue |= 0x08; /* DAC1 */
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
					}
					else
					{
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
					}
				}

				/* latch channel for future usage */
				pAd->LatchRfRegs.Channel = Channel;
				break;
			}
		}
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A;/* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* Gary : 2010/0721 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38);


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

		if (pAd->ate.TxAntennaSel == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		else if (pAd->ate.TxAntennaSel == 2)
		{
			TxPinCfg &= 0xFFFFFFFC;
		}
		else
		{
			TxPinCfg &= 0xFFFFFFFF;
		}

		if (pAd->ate.RxAntennaSel == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}
		else if (pAd->ate.RxAntennaSel == 2)
		{
			TxPinCfg &= 0xFFFFFCFF;
		}
		else
		{
			TxPinCfg &= 0xFFFFFFFF;
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

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38);/* Gary: 2010/0721 */

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

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

#ifdef RTMP_RF_RW_SUPPORT
	Value = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

	if (Value & (1 << 20))
	{ 
		/* Xtal=40M */
		DBGPRINT(RT_DEBUG_TRACE, ("RT3352:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		RtmpFreqItems3020[index].N, 
		RtmpFreqItems3020[index].K, 
		RtmpFreqItems3020[index].R));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT3352Xtal20M:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		FreqItems3020_Xtal20M[index].N, 
		FreqItems3020_Xtal20M[index].K, 
		FreqItems3020_Xtal20M[index].R));
	}
#endif /* RTMP_RF_RW_SUPPORT */
}
#endif /* RT3352 */


#endif /* RT305x */




#ifdef RT2883
/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT2883 ATE.
    
==========================================================================
*/
VOID RT2883ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	UINT32 R2 = 0, R3 = DEFAULT_RF_TX_POWER, R4 = 0;
	RTMP_RF_REGS *RFRegTable = NULL;
#ifdef DOT11N_SS3_SUPPORT
	CHAR TxPwer3 = 0;
#endif /* DOT11N_SS3_SUPPORT */

#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;
#ifdef DOT11N_SS3_SUPPORT
	TxPwer3 = pAd->ate.TxPower2;
#endif /* DOT11N_SS3_SUPPORT */

	RFRegTable = RF2850RegTable;

	switch (pAd->RfIcType)
	{
		case RFIC_2853:
			for (index = 0; index < NUM_OF_2850_CHNL; index++)
			{
				if (Channel == RFRegTable[index].Channel)
				{
					R2 = RFRegTable[index].R2;

					/* If TX path is 1, bit 14 = 1. */
					if (pAd->Antenna.field.TxPath == 1)
					{
						R2 |= 0x4000;	
					}

					if (pAd->Antenna.field.TxPath < 3)
					{
						R2 |= 0x2000;/* write 1 to bit 13 */
					}

					if (pAd->Antenna.field.TxPath == 3)
					{
						RTMP_IO_READ32(pAd, 0x1048, &Value);
						
						if (pAd->ate.TxAntennaSel == 1)
						{
							/* BBP, MAC control */
							RTMP_IO_READ32(pAd, 0x1048, &Value);
							
							Value &= ~0x000F0000;
							Value |= 0x00000000;    /* 0b00 for bit 16,17 */
							RTMP_IO_WRITE32(pAd, 0x1048, Value); 
							
							RTMP_IO_READ32(pAd, 0x1048, &Value);
							
							/* RF Reg2 bit[15:14:13]=0:1:1 */
							R2 &= ~(0x0000E000);
							R2 |= 0x00006000;
						} 
						else if (pAd->ate.TxAntennaSel == 2)
						{
							/* BBP, MAC control */
							RTMP_IO_READ32(pAd, 0x1048, &Value);
							Value &= ~0x000F0000;
        					Value |= 0x00010000;
							RTMP_IO_WRITE32(pAd, 0x1048, Value); 				
																	
							RTMP_IO_READ32(pAd, 0x1048, &Value);
							
							/* RF Reg2 bit[15:14:13]=1:0:1 */
							R2 &= ~(0x0000E000);
							R2 |= 0x0000A000;
						} 
						else if (pAd->ate.TxAntennaSel == 3)
						{
							/* BBP, MAC control */
							RTMP_IO_READ32(pAd, 0x1048, &Value);
							Value &= ~0x000F0000;
        					Value |= 0x00020000;										
							RTMP_IO_WRITE32(pAd, 0x1048, Value); 									
																		
							RTMP_IO_READ32(pAd, 0x1048, &Value);		
							
							/* RF Reg2 bit[15:14:13]=1:1:0 */
							R2 &= ~(0x0000E000);
							R2 |= 0x0000C000;
						} 
						else
						{
							/* BBP, MAC control */
							RTMP_IO_READ32(pAd, 0x1048, &Value);
							Value |= 0x00030000;
							RTMP_IO_WRITE32(pAd, 0x1048, Value);
							RTMP_IO_READ32(pAd, 0x1048, &Value);
								
							/* RF Reg2 bit[15:14:13]=0:0:0 */	
							R2 &= ~(0x0000E000);
							R2 |= 0x00000000;
						}
					}

					if (pAd->Antenna.field.TxPath == 2)
					{
						if (pAd->ate.TxAntennaSel == 1)
						{
							/* If TX Antenna select is 1 , bit 14 = 1; Disable Ant 2 */
							R2 |= 0x4000;	
							ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
							BbpValue &= 0xE7;		/* 11100111B */
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
						}
						else if (pAd->ate.TxAntennaSel == 2)
						{
							/* If TX Antenna select is 2 , bit 15 = 1; Disable Ant 1 */
							R2 |= 0x8000;	
							ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
							BbpValue &= 0xE7;	
							BbpValue |= 0x08;
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
						}
						else
						{
							ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
							BbpValue &= 0xE7;
							BbpValue |= 0x10;
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
						}
					}

					if (pAd->Antenna.field.RxPath == 2)
					{
						switch (pAd->ate.RxAntennaSel)
						{
							case 1:
								R2 |= 0x20040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x00;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
							case 2:
								R2 |= 0x10040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x01;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									
								break;
							default:	
								R2 |= 0x40;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								/* Only enable two Antenna to receive. */
								BbpValue |= 0x08;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
						}
					}
					else if (pAd->Antenna.field.RxPath == 1)
					{
						/* write 1 to off RxPath */
						R2 |= 0x20040;	
					}

					if (pAd->Antenna.field.RxPath == 3)
					{
						switch (pAd->ate.RxAntennaSel)
						{
							case 1:
								R2 |= 0x20040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x00;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
							case 2:
								R2 |= 0x10040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x01;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									
								break;
							case 3:	
								R2 |= 0x30000;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x02;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
								break;								
							default:	
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x10;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
						}
					}
					
					if (Channel > 14)
					{
						/* initialize R3, R4 */
						R3 = (RFRegTable[index].R3 & 0xffffc1ff);
						R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15);

						/*
							According the Rory's suggestion to solve the middle range issue.

							5.5G band power range : 0xF9~0X0F, TX0 Reg3 bit9/TX1 Reg4 bit6="0"
												means the TX power reduce 7dB.
						*/
						/* R3 */
						if ((TxPwer >= -7) && (TxPwer < 0))
						{
							TxPwer = (7+TxPwer);
							R3 |= (TxPwer << 10);
							DBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer=%d \n", TxPwer));
						}
						else
						{
							TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
							R3 |= (TxPwer << 10) | (1 << 9);
						}

						/* R4 */
						if ((TxPwer2 >= -7) && (TxPwer2 < 0))
						{
							TxPwer2 = (7+TxPwer2);
							R4 |= (TxPwer2 << 7);
							DBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer2=%d \n", TxPwer2));
						}
						else
						{
							TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
							R4 |= (TxPwer2 << 7) | (1 << 6);
						}

						RFRegTable[index].R1 &= 0xff8fffcf; /* clear bit 4,5,20,21,22 */
						/* R1 */
						if ((TxPwer3 >= -7) && (TxPwer3 < 0))
						{
							TxPwer3 = (7+TxPwer3);
							RFRegTable[index].R1 |= (((TxPwer3 & 0xe) << 19) | ((TxPwer3 & 0x1) << 5));
						}
						else
						{
							TxPwer3 = (TxPwer3 > 0xF) ? (0xF) : (TxPwer3);
							RFRegTable[index].R1 |= (((TxPwer3 & 0xe) << 19) | ((TxPwer3 & 0x1) << 5)) | (1 << 4);
						}
						RFRegTable[index].R1 &= 0xfffffdff;
					}
					else
					{
						/* Set TX power0. */
						R3 = (RFRegTable[index].R3 & 0xffffc1ff) | (TxPwer << 9);
						/* Set frequency offset and TX power1. */
						R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15) | (TxPwer2 <<6);

						RFRegTable[index].R1 &= 0xff8fffcf; /* clear bit 4,5,20,21,22 */
						/* R1 */
						RFRegTable[index].R1 |= (((TxPwer3 & 0x1c) << 18) | ((TxPwer3 & 0x3) << 4));
						RFRegTable[index].R1 &= 0xfffffdff;
					}

					/* based on BBP current mode before changing RF channel */
					if (pAd->ate.TxWI.BW == BW_40)
					{
						R4 |=0x200000;
					}
					
					/* Update variables. */
					pAd->LatchRfRegs.Channel = Channel;
					pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
					pAd->LatchRfRegs.R2 = R2;
					pAd->LatchRfRegs.R3 = R3;
					pAd->LatchRfRegs.R4 = R4;

					RtmpRfIoWrite(pAd);
					
					break;
				}
			}
			break;

		default:
			break;
	}


	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A;/* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		TxPinCfg = 0x32050F0A;/* Gary 2007/08/09 0x050A0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* According the Rory's suggestion to solve the middle range issue. */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);	

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
			if (pAd->ate.TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pAd->ate.TxAntennaSel == 2)
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

	    TxPinCfg = 0x31050F05;/* Gary 2007/08/09 0x050A0A */
		
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* According the Rory's suggestion to solve the middle range issue. */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);        

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
	}

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

#ifndef RTMP_RF_RW_SUPPORT
	if (Channel > 14)
	{
		/* When 5.5GHz band the LSB of TxPwr will be used to reduced 7dB or not. */
		DBGPRINT(RT_DEBUG_TRACE, ("RT2883:SwitchChannel#%d(RF=%d, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT2883:SwitchChannel#%d(RF=%d, Pwr0=%u, Pwr1=%u, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  (R3 & 0x00003e00) >> 9,
								  (R4 & 0x000007c0) >> 6,
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
    }
#endif /* !RTMP_RF_RW_SUPPORT */
}    
#endif /* RT2883 */


#if defined(RT28xx) || defined(RT2880)
/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT28xx ATE.
    
==========================================================================
*/
VOID RT28xxATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	UINT32 R2 = 0, R3 = DEFAULT_RF_TX_POWER, R4 = 0;
	RTMP_RF_REGS *RFRegTable = NULL;

#ifdef RALINK_QA
	/* for QA mode, TX power values are passed from UI */
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif /* RALINK_QA */
	Channel = pAd->ate.Channel;

	/* fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;

	RFRegTable = RF2850RegTable;

	switch (pAd->RfIcType)
	{
		/* But only 2850 and 2750 support 5.5GHz band... */
		case RFIC_2820:
		case RFIC_2850:
		case RFIC_2720:
		case RFIC_2750:
			for (index = 0; index < NUM_OF_2850_CHNL; index++)
			{
				if (Channel == RFRegTable[index].Channel)
				{
					R2 = RFRegTable[index].R2;

					/* If TX path is 1, bit 14 = 1. */
					if (pAd->Antenna.field.TxPath == 1)
					{
						R2 |= 0x4000;	
					}

					if (pAd->Antenna.field.TxPath == 2)
					{
						if (pAd->ate.TxAntennaSel == 1)
						{
							/* If TX Antenna select is 1 , bit 14 = 1; Disable Ant 2 */
							R2 |= 0x4000;	
							ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
							BbpValue &= 0xE7;		/* 11100111B */
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
						}
						else if (pAd->ate.TxAntennaSel == 2)
						{
							/* If TX Antenna select is 2 , bit 15 = 1; Disable Ant 1 */
							R2 |= 0x8000;	
							ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
							BbpValue &= 0xE7;	
							BbpValue |= 0x08;
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
						}
						else
						{
							ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
							BbpValue &= 0xE7;
							BbpValue |= 0x10;
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
						}
					}

					if (pAd->Antenna.field.RxPath == 2)
					{
						switch (pAd->ate.RxAntennaSel)
						{
							case 1:
								R2 |= 0x20040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x00;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
							case 2:
								R2 |= 0x10040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x01;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									
								break;
							default:	
								R2 |= 0x40;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								/* Only enable two Antenna to receive. */
								BbpValue |= 0x08;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
						}
					}
					else if (pAd->Antenna.field.RxPath == 1)
					{
						/* write 1 to off RxPath */
						R2 |= 0x20040;	
					}

					if (pAd->Antenna.field.RxPath == 3)
					{
						switch (pAd->ate.RxAntennaSel)
						{
							case 1:
								R2 |= 0x20040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x00;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
							case 2:
								R2 |= 0x10040;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x01;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									
								break;
							case 3:	
								R2 |= 0x30000;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x02;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
								break;								
							default:	
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
								BbpValue &= 0xE4;
								BbpValue |= 0x10;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
								break;
						}
					}
					
					if (Channel > 14)
					{
						/* initialize R3, R4 */
						R3 = (RFRegTable[index].R3 & 0xffffc1ff);
						R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15);

						/*
							According the Rory's suggestion to solve the middle range issue.

							5.5G band power range : 0xF9~0X0F, TX0 Reg3 bit9/TX1 Reg4 bit6="0"
												means the TX power reduce 7dB.
						*/
						/* R3 */
						if ((TxPwer >= -7) && (TxPwer < 0))
						{
							TxPwer = (7+TxPwer);
							R3 |= (TxPwer << 10);
							DBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer=%d \n", TxPwer));
						}
						else
						{
							TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
							R3 |= (TxPwer << 10) | (1 << 9);
						}

						/* R4 */
						if ((TxPwer2 >= -7) && (TxPwer2 < 0))
						{
							TxPwer2 = (7+TxPwer2);
							R4 |= (TxPwer2 << 7);
							DBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer2=%d \n", TxPwer2));
						}
						else
						{
							TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
							R4 |= (TxPwer2 << 7) | (1 << 6);
						}
					}
					else
					{
						/* Set TX power0. */
						R3 = (RFRegTable[index].R3 & 0xffffc1ff) | (TxPwer << 9);
						/* Set frequency offset and TX power1. */
						R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15) | (TxPwer2 <<6);
					}

					/* based on BBP current mode before changing RF channel */
					if (pAd->ate.TxWI.BW == BW_40)
					{
						R4 |=0x200000;
					}
					
					/* Update variables. */
					pAd->LatchRfRegs.Channel = Channel;
					pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
					pAd->LatchRfRegs.R2 = R2;
					pAd->LatchRfRegs.R3 = R3;
					pAd->LatchRfRegs.R4 = R4;

					RtmpRfIoWrite(pAd);
					
					break;
				}
			}
			break;

		default:
			break;
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A;/* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* According the Rory's suggestion to solve the middle range issue. */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);	

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
			if (pAd->ate.TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pAd->ate.TxAntennaSel == 2)
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

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

#ifndef RTMP_RF_RW_SUPPORT
	if (Channel > 14)
	{
		/* When 5.5GHz band the LSB of TxPwr will be used to reduced 7dB or not. */
		DBGPRINT(RT_DEBUG_TRACE, ("RT28xx:SwitchChannel#%d(RF=%d, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT28xx:SwitchChannel#%d(RF=%d, Pwr0=%u, Pwr1=%u, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  (R3 & 0x00003e00) >> 9,
								  (R4 & 0x000007c0) >> 6,
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
    }
#endif /* !RTMP_RF_RW_SUPPORT */
}
#endif /* defined(RT28xx) || defined(RT2880) */


/*
==========================================================================
    Description:

	Default AsicSwitchChannel() dedicated for ATE.
    
==========================================================================
*/
VOID ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	ATE_INFO *pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->ChannelSwitch != NULL)
		pATEInfo->pChipStruct->ChannelSwitch(pAd);

	return;
}


static VOID BbpSoftReset(
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


#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
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
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */


#ifdef RALINK_QA
static NDIS_STATUS TXSTOP(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0, atemode=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PTXD_STRUC		pTxD = NULL;
	PRTMP_TX_RING	pTxRing = &pAd->TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
    TXD_STRUC       TxD;
#endif /* RT_BIG_ENDIAN */
#endif /* RTMP_MAC_PCI */
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));
	
	atemode = pAd->ate.Mode;
	pAd->ate.Mode &= ATE_TXSTOP;
	pAd->ate.bQATxStart = FALSE;

	if (atemode == ATE_TXCARR)
	{
		if (pAd->ate.TxMethod == TX_METHOD_0)
		{
			/* No Carrier Test set BBP R22 bit7=0, bit6=0, bit[5~0]=0x0 */
			ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);
		}
	}
	else if (atemode == ATE_TXCARRSUPP)
	{
		if (pAd->ate.TxMethod == TX_METHOD_0)
		{
			/* No Cont. TX set BBP R22 bit7=0 */
			/* QA will do this in new TXCARRSUPP proposal */
			ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

			/* No Carrier Suppression set BBP R24 bit0=0 */
			ATE_BBP_CTS_TX_SIN_WAVE_DISABLE(pAd, BBP_R24, &BbpData);
		}
	}		

	/*
		We should free some resource which was allocated
		when ATE_TXFRAME, ATE_STOP, and ATE_TXCONT.
	*/
	else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
	{
		if (atemode == ATE_TXCONT)
		{
			if (pAd->ate.TxMethod == TX_METHOD_0)
			{
				/* No Cont. TX set BBP R22 bit7=0 */
				/* QA will do this in new TXCONT proposal */
				ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);
			}
		}

#ifdef RTMP_MAC_PCI
		/* Abort Tx, Rx DMA. */
		RtmpDmaEnable(pAd, 0);

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
		/* Enable Tx, Rx DMA */
		RtmpDmaEnable(pAd, 1);
#endif /* RTMP_MAC_PCI */

	}

	/* task Tx status : 0 --> task is idle, 1 --> task is running */
	pAd->ate.TxStatus = 0;

	if (pAd->ate.TxMethod == TX_METHOD_0)
	{
		BbpSoftReset(pAd);/* Soft reset BBP. */
	}

	/* Disable Tx */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS RXSTOP(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	pAd->ate.Mode &= ATE_RXSTOP;
	pAd->ate.bQARxStart = FALSE;


	if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)))
		BbpSoftReset(pAd);/* Soft reset BBP. */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static VOID memcpy_exl(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	UINT32 i, Value = 0;
	UCHAR *pDst = NULL, *pSrc = NULL;
	
	for (i = 0 ; i < (len/4); i++)
	{
		pDst = (dst + i*4);
		pSrc = (src + i*4);
		/* For alignment issue, we need a variable "Value". */
		memmove(&Value, pSrc, 4);
		Value = OS_HTONL(Value); 
		memmove(pDst, &Value, 4);		
		pDst += 4;
		pSrc += 4;
	}

	if ((len % 4) != 0)
	{
		/* wish that it will never reach here */
		memmove(&Value, pSrc, (len % 4));
		Value = OS_HTONL(Value); 
		memmove(pDst, &Value, (len % 4));
	}
}


static VOID memcpy_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i;
#if defined(RT2883) || defined(RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		UCHAR *pDst, *pSrc;
		
		pDst = dst;
		pSrc = src;	

		for (i = 0; i < (len >> 1); i++)
		{
			memmove(pDst, pSrc, 2);
			*((USHORT *)pDst) = OS_HTONS(*((USHORT *)pDst));
			pDst+=2;
			pSrc+=2;
		}

		if ((len % 2) != 0)
		{
			memmove(pDst, pSrc, 1);
		}
	}
	else
#endif /* defined(RT2883) || defined(RT3883) */
	{
		USHORT *pDst, *pSrc;
		
		pDst = (USHORT *) dst;
		pSrc = (USHORT *) src;

		for (i =0; i < (len >> 1); i++)
		{
			*pDst = OS_NTOHS(*pSrc);
			pDst++;
			pSrc++;
		}
		
		if ((len % 2) != 0)
		{
			memcpy(pDst, pSrc, (len % 2));
			*pDst = OS_NTOHS(*pDst);
		}
	}
	return;
}


static VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *dst, UINT32 offset, UINT32 len)
{
	UINT32 i, Value = 0;
	UCHAR *pDst;

	for (i = 0 ; i < (len/4); i++)
	{
		pDst = (dst + i*4);
		RTMP_IO_READ32(pAd, offset, &Value);
		Value = OS_HTONL(Value);
		memmove(pDst, &Value, 4);
		offset += 4;
	}
	return;
}


VOID BubbleSort(INT32 n, INT32 a[])
{ 
	INT32 k, j, temp;

	for (k = n-1;  k>0;  k--)
	{
		for (j = 0; j<k; j++)
		{
			if (a[j] > a[j+1])
			{
				temp = a[j]; 
				a[j]=a[j+1]; 
				a[j+1]=temp;
			}
		}
	}
	return;
} 


VOID CalNoiseLevel(PRTMP_ADAPTER pAd, UCHAR channel, INT32 RSSI[3][10])
{
	INT32		RSSI0, RSSI1, RSSI2;
 	CHAR		Rssi0Offset, Rssi1Offset, Rssi2Offset;
	UCHAR		BbpR50Rssi0 = 0, BbpR51Rssi1 = 0, BbpR52Rssi2 = 0;
	UCHAR		Org_BBP66value = 0, Org_BBP69value = 0, Org_BBP70value = 0, data = 0;
#if defined(RT2883) || defined(RT3883) ||  defined(RT3352) ||  defined(RT5350)
	UCHAR		byteValue = 0;
#endif /* defined(RT2883) || defined(RT3883) ||  defined(RT3352) ||  defined(RT5350) */
	USHORT		LNA_Gain = 0;
	INT32		j = 0;
	UCHAR		Org_Channel = pAd->ate.Channel;
	USHORT	    GainValue = 0, OffsetValue = 0;

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &Org_BBP66value);
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R69, &Org_BBP69value);	
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R70, &Org_BBP70value);

	/************************************************************************/
	/* Read the value of LNA gain and RSSI offset */
	/************************************************************************/
	RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, GainValue);

	/* for Noise Level */
	if (channel <= 14)
	{
		LNA_Gain = GainValue & 0x00FF;		 

		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, OffsetValue);
		Rssi0Offset = OffsetValue & 0x00FF;
		Rssi1Offset = (OffsetValue & 0xFF00) >> 8;

		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_BG_OFFSET + 2)/* 0x48 */, OffsetValue);
		Rssi2Offset = OffsetValue & 0x00FF;
	}
	else
	{
		LNA_Gain = (GainValue & 0xFF00) >> 8;

		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, OffsetValue);
		Rssi0Offset = OffsetValue & 0x00FF;
		Rssi1Offset = (OffsetValue & 0xFF00) >> 8;

		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET + 2)/* 0x4C */, OffsetValue);
		Rssi2Offset = OffsetValue & 0x00FF;
	}
	/***********************************************************************/	
	{
		pAd->ate.Channel = channel;
		ATEAsicSwitchChannel(pAd);
		RtmpOsMsDelay(5);

		data = 0x10;
#if defined (RT2883) || defined (RT3883) || defined (RT3352) || defined (RT5350)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3352(pAd) || IS_RT5350(pAd))
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
			byteValue &= 0x9f;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);
		}
#ifdef RT3352
		if (IS_RT3352(pAd))
		{
			if (pAd->ate.TxWI.BW == BW_20)
			{
				data = GET_LNA_GAIN(pAd)*2 + 0x1C;
			}
			else
			{
				data = GET_LNA_GAIN(pAd)*2 + 0x24;
			}
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);
		}
#endif /* RT3352 */
#if defined (RT2883) || defined (RT3883)
		if (IS_RT2883(pAd) || IS_RT3883(pAd))
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);	
		}
#endif /* defined (RT2883) || defined (RT3883) */
#else
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);	
#endif /* defined(RT2883) || defined(RT3883) ||  defined(RT3352) || defined (RT5350) */
		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, data);
		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, data);
		RtmpOsMsDelay(5);

		/* start Rx */
		pAd->ate.bQARxStart = TRUE;
		Set_ATE_Proc(pAd, "RXFRAME");

		RtmpOsMsDelay(5);

		for (j = 0; j < 10; j++)
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R50, &BbpR50Rssi0);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R51, &BbpR51Rssi1);	
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R52, &BbpR52Rssi2);

			RtmpOsMsDelay(10);

			/* calculate RSSI 0 */
			if (BbpR50Rssi0 == 0)
			{
				RSSI0 = -100;
			}
			else
			{
				RSSI0 = (INT32)(-12 - BbpR50Rssi0 - LNA_Gain - Rssi0Offset);
			}
			RSSI[0][j] = RSSI0;

			if ( pAd->Antenna.field.RxPath >= 2 ) /* 2R */
			{
				/* calculate RSSI 1 */
				if (BbpR51Rssi1 == 0)
				{
					RSSI1 = -100;
				}
				else
				{
					RSSI1 = (INT32)(-12 - BbpR51Rssi1 - LNA_Gain - Rssi1Offset);
				}
				RSSI[1][j] = RSSI1;
			}

			if ( pAd->Antenna.field.RxPath >= 3 ) /* 3R */
			{
				/* calculate RSSI 2 */
				if (BbpR52Rssi2 == 0)
					RSSI2 = -100;
				else
					RSSI2 = (INT32)(-12 - BbpR52Rssi2 - LNA_Gain - Rssi2Offset);

				RSSI[2][j] = RSSI2;
			}
		}

		/* stop Rx */
		Set_ATE_Proc(pAd, "RXSTOP");

		RtmpOsMsDelay(5);

		BubbleSort(10, RSSI[0]); /* 1R */		

		if ( pAd->Antenna.field.RxPath >= 2 ) /* 2R */
		{
			BubbleSort(10, RSSI[1]);
		}

		if ( pAd->Antenna.field.RxPath >= 3 ) /* 3R */
		{
			BubbleSort(10, RSSI[2]);
		}	
	}

	pAd->ate.Channel = Org_Channel;
	ATEAsicSwitchChannel(pAd);

	/* restore original value */	
#if defined (RT2883) || defined (RT3883) || defined (RT3352) || defined (RT5350)
	if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3352(pAd) || IS_RT5350(pAd))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
		byteValue &= 0x9f;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Org_BBP66value);
	}
#ifdef RT3352
	if (IS_RT3352(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Org_BBP66value);
	}
#endif /* RT3352 */
#if defined (RT2883) || defined (RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Org_BBP66value);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));		
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Org_BBP66value);
	}
#endif /* defined (RT2883) || defined (RT3883) */
#endif /* defined (RT2883) || defined (RT3883) || defined (RT3352) || defined (RT5350) */

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, Org_BBP69value);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, Org_BBP70value);

	return;
}




static VOID ATE_BBPRead(
		IN	PRTMP_ADAPTER	pAd,
		IN	UCHAR			offset,
		IN  UCHAR			*pValue)
{
	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, offset, pValue);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, offset, pValue);
	}
}

static VOID ATE_BBPWrite(
		IN	PRTMP_ADAPTER	pAd,
		IN	UCHAR			offset,
		IN  UCHAR			value)
{
	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, offset, value);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, offset, value);
	}
}


static INT ResponseToGUI(
	IN  struct ate_racfghdr *pRaCfg,
	IN	RTMP_IOCTL_INPUT_STRUCT	*pwrq,
	IN  INT Length,
	IN  INT Status)
{
	(pRaCfg)->length = OS_HTONS((Length));
	(pRaCfg)->status = OS_HTONS((Status));
	(pwrq)->u.data.length = sizeof((pRaCfg)->magic_no) + sizeof((pRaCfg)->command_type)
							+ sizeof((pRaCfg)->command_id) + sizeof((pRaCfg)->length)
							+ sizeof((pRaCfg)->sequence) + OS_NTOHS((pRaCfg)->length);
	DBGPRINT(RT_DEBUG_TRACE, ("wrq->u.data.length = %d\n", (pwrq)->u.data.length));

	if (copy_to_user((pwrq)->u.data.pointer, (UCHAR *)(pRaCfg), (pwrq)->u.data.length))
	{
		
		DBGPRINT(RT_DEBUG_ERROR, ("copy_to_user() fail in %s\n", __FUNCTION__));
		return (-EFAULT);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RACFG command(0x%04x)[magic number(0x%08x)] is done\n", 
										OS_NTOHS(pRaCfg->command_id), OS_NTOHL(pRaCfg->magic_no)));
	}

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START\n"));

	pAd->ate.bQAEnabled = TRUE;
	DBGPRINT(RT_DEBUG_TRACE,("pAd->ate.bQAEnabled = %s\n", (pAd->ate.bQAEnabled)? "TRUE":"FALSE"));
	
	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
	
#ifdef	CONFIG_RT2880_ATE_CMD_NEW
	Set_ATE_Proc(pAd, "ATESTART");
#else
	Set_ATE_Proc(pAd, "APSTOP");
#endif /* CONFIG_RT2880_ATE_CMD_NEW */

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_STOP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	INT32 ret;

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_STOP\n"));

	pAd->ate.bQAEnabled = FALSE;
	DBGPRINT(RT_DEBUG_TRACE,("pAd->ate.bQAEnabled = %s\n", (pAd->ate.bQAEnabled)? "TRUE":"FALSE"));

	/*
		Distinguish this command came from QA(via ate agent)
		or ate agent according to the existence of pid in payload.

		No need to prepare feedback if this cmd came directly from ate agent,
		not from QA.
	*/
	pRaCfg->length = OS_NTOHS(pRaCfg->length);

	if (pRaCfg->length == sizeof(pAd->ate.AtePid))
	{
		/*
			This command came from QA.
			Get the pid of ATE agent.
		*/
		memcpy((UCHAR *)&pAd->ate.AtePid,
						(&pRaCfg->data[0]) - 2/* == sizeof(pRaCfg->status) */,
						sizeof(pAd->ate.AtePid));					

		/* Prepare feedback as soon as we can to avoid QA timeout. */
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

		/*
			Kill ATE agent when leaving ATE mode.

			We must kill ATE agent first before setting ATESTOP,
			or Microsoft will report sth. wrong. 
		*/
#ifdef LINUX
		ret = RTMP_THREAD_PID_KILL(pAd->ate.AtePid);
		if (ret)
			DBGPRINT(RT_DEBUG_ERROR, ("%s: unable to kill ate thread\n", 
				RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
#endif /* LINUX */
	}
		

	/* AP/STA might have in ATE_STOP mode due to cmd from QA. */
	if (ATE_ON(pAd))
	{
		/* Someone has killed ate agent while QA GUI is still open. */

#ifdef	CONFIG_RT2880_ATE_CMD_NEW
		Set_ATE_Proc(pAd, "ATESTOP");
#else
		Set_ATE_Proc(pAd, "APSTART");
#endif
		DBGPRINT(RT_DEBUG_TRACE, ("RACFG_CMD_AP_START is done !\n"));
	}
	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_RF_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 R1, R2, R3, R4;
	USHORT channel;
	
	memcpy(&R1, pRaCfg->data-2, 4);
	memcpy(&R2, pRaCfg->data+2, 4);
	memcpy(&R3, pRaCfg->data+6, 4);
	memcpy(&R4, pRaCfg->data+10, 4);
	memcpy(&channel, pRaCfg->data+14, 2);		
	
	pAd->LatchRfRegs.R1 = OS_NTOHL(R1);
	pAd->LatchRfRegs.R2 = OS_NTOHL(R2);
	pAd->LatchRfRegs.R3 = OS_NTOHL(R3);
	pAd->LatchRfRegs.R4 = OS_NTOHL(R4);
	pAd->LatchRfRegs.Channel = OS_NTOHS(channel);

	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R3);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return  NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_READ16(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT16	offset=0, value=0;
	USHORT  tmp=0;				

	offset = OS_NTOHS(pRaCfg->status);

	/* "tmp" is especially for some compilers... */
	RT28xx_EEPROM_READ16(pAd, offset, tmp);
	value = tmp;
	value = OS_HTONS(value);
	
	DBGPRINT(RT_DEBUG_TRACE,("EEPROM Read offset = 0x%04x, value = 0x%04x\n", offset, value));
	memcpy(pRaCfg->data, &value, 2);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+2, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_WRITE16(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset, value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 2);
	value = OS_NTOHS(value);
	RT28xx_EEPROM_WRITE16(pAd, offset, value);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_READ_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE >> 1];

	rt_ee_read_all(pAd,(USHORT *)buffer);
	memcpy_exs(pAd, pRaCfg->data, (UCHAR *)buffer, EEPROM_SIZE);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+EEPROM_SIZE, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE >> 1];

	NdisZeroMemory((UCHAR *)buffer, EEPROM_SIZE);
	memcpy_exs(pAd, (UCHAR *)buffer, (UCHAR *)&pRaCfg->status, EEPROM_SIZE);
	rt_ee_write_all(pAd,(USHORT *)buffer);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_READ(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	UINT32	value;
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);

	/*
		We do not need the base address.
		So just extract the offset out.
	*/
#ifdef RTMP_RBUS_SUPPORT
	if ((offset & 0xFFFF0000) == 0x10000000)
	{
		RTMP_SYS_IO_READ32(offset | 0xa0000000, &value);
	}
	else
#endif /* RTMP_RBUS_SUPPORT */
	{
		offset &= 0x0000FFFF;
		RTMP_IO_READ32(pAd, offset, &value);
	}
	value = OS_HTONL(value);
	memcpy(pRaCfg->data, &value, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_WRITE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset, value;
					
	memcpy(&offset, pRaCfg->data-2, 4);
	memcpy(&value, pRaCfg->data+2, 4);

	offset = OS_NTOHL(offset);

	/*
		We do not need the base address.
		So just extract the offset out.
	*/
	offset &= 0x0000FFFF;
	value = OS_NTOHL(value);
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_IO_WRITE: offset = %x, value = %x\n", offset, value));
	RTMP_IO_WRITE32(pAd, offset, value);
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	USHORT	len;
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);

	/*
		We do not need the base address.
		So just extract the offset out.
	*/
	offset &= 0x0000FFFF;
	memcpy(&len, pRaCfg->data+2, 2);
	len = OS_NTOHS(len);

	if (len > 371)
	{
		DBGPRINT(RT_DEBUG_TRACE,("length requested is too large, make it smaller\n"));
		pRaCfg->length = OS_HTONS(2);
		pRaCfg->status = OS_HTONS(1);

		return -EFAULT;
	}

	RTMP_IO_READ_BULK(pAd, pRaCfg->data, offset, len*4);/* unit in four bytes*/

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(len*4), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_READ8(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;
	
	value = 0;
	offset = OS_NTOHS(pRaCfg->status);

	ATE_BBPRead(pAd, offset, &value);

	pRaCfg->data[0] = value;
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+1, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_WRITE8(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 1);

	ATE_BBPWrite(pAd, offset, value);

	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_READ_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT bbp_reg_index;
	
	for (bbp_reg_index = 0; bbp_reg_index < pAd->chipCap.MaxNumOfBbpId+1; bbp_reg_index++)
	{
		pRaCfg->data[bbp_reg_index] = 0;
		
		ATE_BBPRead(pAd, bbp_reg_index, &pRaCfg->data[bbp_reg_index]);
	}
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+ pAd->chipCap.MaxNumOfBbpId+1, NDIS_STATUS_SUCCESS);
	
	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_GET_NOISE_LEVEL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UCHAR	channel;
	INT32   buffer[3][10];/* 3 : RxPath ; 10 : no. of per rssi samples */

	channel = (OS_NTOHS(pRaCfg->status) & 0x00FF);
	CalNoiseLevel(pAd, channel, buffer);
	memcpy_exl(pAd, (UCHAR *)pRaCfg->data, (UCHAR *)&(buffer[0][0]), (sizeof(INT32)*3*10));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(sizeof(INT32)*3*10), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#if defined(RT2883) || defined(RT3883)
static  INT DO_RACFG_CMD_QUERY_BF_RSP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN struct ate_racfghdr *pRaCfg)
{
	USHORT value, value0, value1, value2, soundingRespSize;
	int i;

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_QUERY_BF_RSP\n"));

	if ((pAd->ate.sounding == 2) && ((jiffies - pAd->ate.sounding_jiffies) < TIME_ONE_SECOND))
	{
		value = OS_HTONS(1);
		value0 = OS_HTONS(pAd->ate.soundingSNR[0]);
		value1 = OS_HTONS(pAd->ate.soundingSNR[1]);
		value2 = OS_HTONS(pAd->ate.soundingSNR[2]);
		soundingRespSize = 0;	/* 0=>sounding response is not returned */
	}
	else
	{
		value = 0;
		value0 = value1 = value2 = 0;
		soundingRespSize = 0;
	}

	/* prepare feedback */
	pRaCfg->length = OS_HTONS(4+6+2+soundingRespSize);
	pRaCfg->status = OS_HTONS(0);
	memcpy(pRaCfg->data, &value, 2);
	memcpy(pRaCfg->data+2, &value0, 2);
	memcpy(pRaCfg->data+4, &value1, 2);
	memcpy(pRaCfg->data+6, &value2, 2);
	value2 = OS_HTONS(soundingRespSize);
	memcpy(pRaCfg->data+8, &value2, 2);

	for (i=0; i<soundingRespSize && i<sizeof(pAd->ate.soundingResp) && i<sizeof(pRaCfg->data)-10; i++)
		pRaCfg->data[10+i] = pAd->ate.soundingResp[i];

	ResponseToGUI(pRaCfg, wrq, 4+6+2+soundingRespSize, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;

}


#define DO_RACFG_CMD_QUERY_EBF_TAG DO_RACFG_CMD_QUERY_IBF_TAG
static  INT DO_RACFG_CMD_QUERY_IBF_TAG(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN struct ate_racfghdr *pRaCfg)
{
	USHORT profileNum;
	int byteIndex, di;
	UCHAR byteVal;
	BOOLEAN eProfile = OS_NTOHS(pRaCfg->command_id)==RACFG_CMD_QUERY_EBF_TAG;

	profileNum = OS_NTOHS(pRaCfg->status);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_QUERY_BF_TAG %d\n", profileNum));

	/* Select Explicit/Implicit profile */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, eProfile? 0x04: 0x00);

	pRaCfg->length = OS_HTONS(2+18);
	pRaCfg->status = OS_HTONS(0);

	/* Read the tagfield and write to data buffer */
	di = 0;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, 0x80);

	for (byteIndex=0; byteIndex<18; byteIndex++)
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R182, &byteVal);
		pRaCfg->data[di++] = byteVal;
	}

	ResponseToGUI(pRaCfg, wrq, 2+18, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#define DO_RACFG_CMD_QUERY_EBF_PROFILE DO_RACFG_CMD_QUERY_IBF_PROFILE
static  INT DO_RACFG_CMD_QUERY_IBF_PROFILE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN struct ate_racfghdr *pRaCfg)
{
	USHORT profileNum, startCarrier;
	USHORT bytesPerRow, numCarriers;
	int byteIndex, carrierIndex, di;
	UCHAR byteVal;
	BOOLEAN eProfile = OS_NTOHS(pRaCfg->command_id)==RACFG_CMD_QUERY_EBF_PROFILE;			
#ifdef LINUX
	struct timeval tval1, tval2;
#endif /* LINUX */

	profileNum = OS_NTOHS(pRaCfg->status);
	memcpy(&startCarrier, pRaCfg->data, 2);
	startCarrier = OS_NTOHS(startCarrier);
	memcpy(&numCarriers, pRaCfg->data+2, 2);
	numCarriers = OS_NTOHS(numCarriers);

	/* Older version have no numCarriers field so length is 4. If so, numCarriers=64 */
	if (OS_NTOHS(pRaCfg->length)==4)
		numCarriers = 64;

	if (startCarrier+numCarriers > 128)
		numCarriers = 128 - startCarrier;

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_QUERY_BF_PROFILE\n"));

#ifdef LINUX
	do_gettimeofday(&tval1);
#endif /* LINUX */
	bytesPerRow = (eProfile? 18: 14);

	/* Select Explicit/Implicit profile */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, eProfile? 0x04: 0x00);

	pRaCfg->length = OS_HTONS(2+bytesPerRow*numCarriers);
	pRaCfg->status = OS_HTONS(0);

	/* Read the data for each carrier and write to data buffer */
	di = 0;
	for (carrierIndex=startCarrier; carrierIndex<startCarrier+numCarriers; carrierIndex++)
	{
		/* Read a row of data */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, carrierIndex);
	
		for (byteIndex=0; byteIndex<bytesPerRow; byteIndex++)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R182, &byteVal);
			pRaCfg->data[di++] = byteVal;
		}
	}

#ifdef LINUX
	do_gettimeofday(&tval2);
	DBGPRINT(RT_DEBUG_WARN, ("BF Read elasped = %ld usec\n", tval2.tv_usec - tval1.tv_usec));
#endif /* LINUX */

	ResponseToGUI(pRaCfg, wrq, 2+bytesPerRow*numCarriers, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#define DO_RACFG_CMD_WRITE_EBF_TAG DO_RACFG_CMD_WRITE_IBF_TAG
static  INT DO_RACFG_CMD_WRITE_IBF_TAG(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN struct ate_racfghdr *pRaCfg)
{
	USHORT profileNum;
	int byteIndex, di;
	BOOLEAN eProfile = OS_NTOHS(pRaCfg->command_id)==RACFG_CMD_WRITE_EBF_TAG;

	profileNum = OS_NTOHS(pRaCfg->status);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_WRITE_EBF_TAG %d\n", profileNum));

	/* Select Explicit/Implicit profile */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, eProfile? 0x04: 0x00);

	pRaCfg->length = OS_HTONS(2);
	pRaCfg->status = OS_HTONS(0);

	/* Read the tagfield and write to data buffer */
	di = 0;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, 0x80);

	for (byteIndex=0; byteIndex<18; byteIndex++)
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, pRaCfg->data[2+di]);
		di++;
	}

	ResponseToGUI(pRaCfg, wrq, 2, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#define DO_RACFG_CMD_WRITE_EBF_PROFILE DO_RACFG_CMD_WRITE_IBF_PROFILE
static  INT DO_RACFG_CMD_WRITE_IBF_PROFILE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN struct ate_racfghdr *pRaCfg)
{
	USHORT profileNum, startCarrier;
	USHORT bytesPerRow, numCarriers;
	int byteIndex, carrierIndex, di;
	BOOLEAN eProfile = OS_NTOHS(pRaCfg->command_id)==RACFG_CMD_WRITE_EBF_PROFILE;
#ifdef LINUX
	struct timeval tval1, tval2;
#endif /* LINUX */
	
	bytesPerRow = (eProfile? 18: 14);

	profileNum = OS_NTOHS(pRaCfg->status);
	memcpy(&startCarrier, pRaCfg->data, 2);
	startCarrier = OS_NTOHS(startCarrier);

	/* Calculate number of carriers from length of data */
	numCarriers = (OS_NTOHS(pRaCfg->length)-4)/bytesPerRow;

	if (startCarrier+numCarriers > 128)
		numCarriers = 128 - startCarrier;

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_WRITE_BF_PROFILE-%d %d\n", startCarrier, numCarriers));

#ifdef LINUX
	do_gettimeofday(&tval1);
#endif /* LINUX */

	/* Select Explicit/Implicit profile */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, eProfile? 0x04: 0x00);

	pRaCfg->length = OS_HTONS(2);
	pRaCfg->status = OS_HTONS(0);

	/* Write the data for each carrier from data buffer */
	di = 0;
	for (carrierIndex=startCarrier; carrierIndex<startCarrier+numCarriers; carrierIndex++)
	{
		/* Write a row of data */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R181, carrierIndex);
	
		for (byteIndex=0; byteIndex<bytesPerRow; byteIndex++)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, (profileNum<<5) | byteIndex);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, pRaCfg->data[2+di]);
			di++;
		}
	}

#ifdef LINUX
	do_gettimeofday(&tval2);
	DBGPRINT(RT_DEBUG_WARN, ("BF Write elasped = %ld usec\n", tval2.tv_usec - tval1.tv_usec));
#endif /* LINUX */

	ResponseToGUI(pRaCfg, wrq, 2, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static	INT DO_RACFG_CALIBRATION_CAPTURE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 capCtrl, macCtrl;
	UINT8 r25Value, r27Value, r65Value, r66Value[3], r186Value;
	UINT8 rf36Value, rf37Value;
	int i, j;
	SHORT txAnt, temp;
	UINT8 r65Setting, r66Setting, r25Setting;
	UINT16 r36r37Setting;

	UINT16 papdIQ[3] = {0,0,0};
	BOOLEAN usePapd = FALSE;
#ifdef LINUX
	struct timeval tval0, tval1, tval2;
#endif /* LINUX */

	/* Get Parameters */
	txAnt = OS_NTOHS(pRaCfg->status);
	memcpy(&temp, &(pRaCfg->data[0]), 2);
	r65Setting = OS_NTOHS(temp);
	memcpy(&temp, &(pRaCfg->data[2]), 2);
	r66Setting = OS_NTOHS(temp);
	memcpy(&temp, &(pRaCfg->data[4]), 2);
	r36r37Setting = OS_NTOHS(temp);
	memcpy(&temp, &(pRaCfg->data[6]), 2);
	r25Setting = OS_NTOHS(temp);

	/* 
		For backwards compatibility txAnt:
		0, 1, 2 = only enable Ant0/1/2
		-1 = if no papd parameters then don't use papd.
		Otherwise read papd parameters
	*/
	if (txAnt==0 || txAnt==1 || txAnt==2)
	{
		papdIQ[txAnt] = 0x7F00;
		usePapd = TRUE;
	}
	else if (OS_NTOHS(pRaCfg->length)>10)
	{
		/* papdIQ[3] containing I/Q values for PAPD table */
		memcpy(&temp, &(pRaCfg->data[8]), 2);
		papdIQ[0] = OS_NTOHS(temp);
		memcpy(&temp, &(pRaCfg->data[10]), 2);
		papdIQ[1] = OS_NTOHS(temp);
		memcpy(&temp, &(pRaCfg->data[12]), 2);
		papdIQ[2] = OS_NTOHS(temp);
		usePapd = TRUE;
	}

	/* Read CAP CTRL, MAC CTRL and RF R25 */
	RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &capCtrl);
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &macCtrl);
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R25, &r25Value);

	/* Disable MAC Rx */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);
	/* -> disable all Tx/Rx Queue */
	RTMP_IO_WRITE32(pAd, PBF_CFG, 0x00000000);

	/* Overwrite PAPD table and enable PAPD */
	if (usePapd)
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R186, &r186Value);

		for (i=0; i<3; i++)
		{
			for (j=0; j<32; j++)
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, (i<<6) | (j<<1));
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, (papdIQ[i]>>8) & 0xFF);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, (i<<6) | (j<<1) | 1);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, papdIQ[i] & 0xFF);
			}
		}

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x80);
	}

	/* -> capture mode */
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x00004E80);

	/* Enable Capture and RX */
	capCtrl &= ~0x80000000;		/* set bit[31]=0 */
	capCtrl &= ~(0x1FFF0000);	/* CaptureModeOffset = 0 */

	/* Enable Capture and RX */
	RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, capCtrl | 0x40000000);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x08);
	
#ifdef LINUX	
	do_gettimeofday(&tval0);
#endif /* LINUX */

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R250, &r65Value); /* gaa */

	/* Set BBP R65 - LNA */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &r65Value);
	if (r65Setting != 0xFF)
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, r65Setting);

	/* Set BBP R66 - VGA */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &r27Value);
	for (i=0; i<3; i++)
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (r27Value & ~0x60) | (i<<5));
		ATE_RF_IO_READ8_BY_REG_ID(pAd, BBP_R66, &r66Value[i]);
		if (r66Setting != 0)
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, r66Setting);
	}

	/* Enable RF Loopback */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R36, &rf36Value);
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R36, rf36Value | (r36r37Setting & 0xFF));
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R37, &rf37Value);
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R37, rf37Value | (r36r37Setting >> 8));

		/* Start capture before RF loopback */
		RTMP_IO_WRITE32(pAd, PBF_CAP_CTRL, capCtrl | 0x20000000);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, r25Value | r25Setting);

#ifdef LINUX
	do_gettimeofday(&tval2);
#endif /* LINUX */

	/* Wait for capture buffer to fill */
	i = 0;
	do
	{
		RTMPusecDelay(75);
		RTMP_IO_READ32(pAd, PBF_CAP_CTRL, &capCtrl);
	} while ((capCtrl & 0x40000000)!=0 && ++i<12);
	
	/* Stop RX */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

	/* Restore BBP R65, R66, R27, R186 and RF_R36/37 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, r65Value);

	for (i=0; i<3; i++)
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (r27Value & ~0x60) | (i<<5));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, r66Value[i]);
	}
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, r27Value);

	if (usePapd)
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, r186Value);

	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R36, rf36Value);
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R37, rf37Value);

#ifdef LINUX
	do_gettimeofday(&tval1);
	DBGPRINT(RT_DEBUG_WARN, ("Cal Cap c=%x, t1=%ld t2=%ld\n", capCtrl,
				tval1.tv_usec - tval0.tv_usec, tval2.tv_usec - tval0.tv_usec));
#endif /* LINUX */

	ResponseToGUI(pRaCfg, wrq, 2, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* defined(RT2883) || defined(RT3883) */

static  INT DO_RACFG_CMD_GET_COUNTER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	memcpy_exl(pAd, &pRaCfg->data[0], (UCHAR *)&pAd->ate.U2M, 4);
	memcpy_exl(pAd, &pRaCfg->data[4], (UCHAR *)&pAd->ate.OtherData, 4);
	memcpy_exl(pAd, &pRaCfg->data[8], (UCHAR *)&pAd->ate.Beacon, 4);
	memcpy_exl(pAd, &pRaCfg->data[12], (UCHAR *)&pAd->ate.OtherCount, 4);
	memcpy_exl(pAd, &pRaCfg->data[16], (UCHAR *)&pAd->ate.TxAc0, 4);
	memcpy_exl(pAd, &pRaCfg->data[20], (UCHAR *)&pAd->ate.TxAc1, 4);
	memcpy_exl(pAd, &pRaCfg->data[24], (UCHAR *)&pAd->ate.TxAc2, 4);
	memcpy_exl(pAd, &pRaCfg->data[28], (UCHAR *)&pAd->ate.TxAc3, 4);
	memcpy_exl(pAd, &pRaCfg->data[32], (UCHAR *)&pAd->ate.TxHCCA, 4);
	memcpy_exl(pAd, &pRaCfg->data[36], (UCHAR *)&pAd->ate.TxMgmt, 4);
	memcpy_exl(pAd, &pRaCfg->data[40], (UCHAR *)&pAd->ate.RSSI0, 4);
	memcpy_exl(pAd, &pRaCfg->data[44], (UCHAR *)&pAd->ate.RSSI1, 4);
	memcpy_exl(pAd, &pRaCfg->data[48], (UCHAR *)&pAd->ate.RSSI2, 4);
	memcpy_exl(pAd, &pRaCfg->data[52], (UCHAR *)&pAd->ate.SNR0, 4);
	memcpy_exl(pAd, &pRaCfg->data[56], (UCHAR *)&pAd->ate.SNR1, 4);

#if defined(RT2883) || defined(RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd) )
	{
#ifdef DOT11N_SS3_SUPPORT
		memcpy_exl(pAd, &pRaCfg->data[60], (UCHAR *)&pAd->ate.SNR2, 4);
#endif /* DOT11N_SS3_SUPPORT */
#ifdef TXBF_SUPPORT
		memcpy_exl(pAd, &pRaCfg->data[64], (UCHAR *)&pAd->ate.LastRxRate, 4);
#endif /* TXBF_SUPPORT */
		memcpy_exl(pAd, &pRaCfg->data[68], (UCHAR *)&pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word, 4);
		memcpy_exl(pAd, &pRaCfg->data[72], (UCHAR *)&pAd->ate.BF_SNR[0], 4);
		memcpy_exl(pAd, &pRaCfg->data[76], (UCHAR *)&pAd->ate.BF_SNR[1], 4);
		memcpy_exl(pAd, &pRaCfg->data[80], (UCHAR *)&pAd->ate.BF_SNR[2], 4);
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+84, NDIS_STATUS_SUCCESS);
	}
	else
	{
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+60, NDIS_STATUS_SUCCESS);
	}
#else
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+60, NDIS_STATUS_SUCCESS);
#endif /* defined(RT2883) || defined(RT3883) */

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_CLEAR_COUNTER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	pAd->ate.U2M = 0;
	pAd->ate.OtherData = 0;
	pAd->ate.Beacon = 0;
	pAd->ate.OtherCount = 0;
	pAd->ate.TxAc0 = 0;
	pAd->ate.TxAc1 = 0;
	pAd->ate.TxAc2 = 0;
	pAd->ate.TxAc3 = 0;
	pAd->ate.TxHCCA = 0;
	pAd->ate.TxMgmt = 0;
	pAd->ate.TxDoneCount = 0;
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_TX_START(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT *p;
	USHORT	err = 1;
#if defined(RT2883) || defined(RT3883)
	HEADER_802_11 *pHeader_802_11 = NULL;
	PHT_CONTROL pHTC;
#endif /* defined(RT2883) || defined(RT3883) */

	if ((pAd->ate.TxStatus != 0) && (pAd->ate.Mode & ATE_TXFRAME))
	{
		DBGPRINT(RT_DEBUG_TRACE,("Ate Tx is already running, to run next Tx, you must stop it first\n"));
		err = 2;
		goto tx_start_error;
	}
	else if ((pAd->ate.TxStatus != 0) && !(pAd->ate.Mode & ATE_TXFRAME))
	{
		int i = 0;

		while ((i++ < 10) && (pAd->ate.TxStatus != 0))
		{
			RtmpOsMsDelay(5);
		}

		/* force it to stop */
		pAd->ate.TxStatus = 0;
		pAd->ate.TxDoneCount = 0;
		pAd->ate.bQATxStart = FALSE;
	}

	/*
		Reset ATE mode and set Tx/Rx idle
		for new proposed TXCONT/TXCARR/TXCARRSUPP solution.
	*/
	if ((pAd->ate.Mode & ATE_TXFRAME) && (pAd->ate.TxMethod == TX_METHOD_1))
	{
		TXSTOP(pAd);
	}

	/*
		If pRaCfg->length == 0, this "RACFG_CMD_TX_START"
		is for Carrier test or Carrier Suppression.
	*/
	if (OS_NTOHS(pRaCfg->length) != 0)
	{
		/* get frame info */

		NdisMoveMemory(&pAd->ate.TxWI, pRaCfg->data + 2, 16);						
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, (PUCHAR)&pAd->ate.TxWI, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */

		NdisMoveMemory(&pAd->ate.TxCount, pRaCfg->data + 18, 4);
		pAd->ate.TxCount = OS_NTOHL(pAd->ate.TxCount);

		p = (USHORT *)(&pRaCfg->data[22]);

		/* always use QID_AC_BE */
		pAd->ate.QID = 0;

		p = (USHORT *)(&pRaCfg->data[24]);
		pAd->ate.HLen = OS_NTOHS(*p);

		if (pAd->ate.HLen > 32)
		{
			DBGPRINT(RT_DEBUG_ERROR,("pAd->ate.HLen > 32\n"));
			err = 3;
			goto tx_start_error;
		}

		NdisMoveMemory(&pAd->ate.Header, pRaCfg->data + 26, pAd->ate.HLen);
#if defined(RT2883) || defined(RT3883)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) )
		{			
			/* check Sounding frame */
			pAd->ate.sounding = 0;
			pHeader_802_11 = (HEADER_802_11	*) pAd->ate.Header;
			pHTC = (PHT_CONTROL) &pHeader_802_11->Octet[2];
			if ((pAd->ate.TxWI.Sounding== 1)
					|| ((pHeader_802_11->FC.SubType & 0x08) && (pHeader_802_11->FC.Order == 1) && (pHTC->NDPAnnounce)) )
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Sending sounding frame\n"));
				pAd->ate.sounding = 1;
				pAd->ate.sounding_jiffies = jiffies; /* TIME_ONE_SECOND timeout */
			}
		}
#endif /* defined(RT2883) || defined(RT3883) */

		pAd->ate.PLen = OS_NTOHS(pRaCfg->length) - (pAd->ate.HLen + 28);

		if (pAd->ate.PLen > 32)
		{
			DBGPRINT(RT_DEBUG_ERROR,("pAd->ate.PLen > 32\n"));
			err = 4;
			goto tx_start_error;
		}

		NdisMoveMemory(&pAd->ate.Pattern, pRaCfg->data + 26 + pAd->ate.HLen, pAd->ate.PLen);
		pAd->ate.DLen = pAd->ate.TxWI.MPDUtotalByteCount - pAd->ate.HLen;

#if defined(RT2883) || defined(RT3883)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) )
		{	
			if ((pHeader_802_11->FC.SubType & 0x08) && (pHeader_802_11->FC.Order == 1))
				pAd->ate.DLen += 2; /* compensation for header padding */
		}
#endif /* defined(RT2883) || defined(RT3883) */

	}

	ReadQATxTypeFromBBP(pAd);

	if (pAd->ate.bQATxStart == TRUE)
	{
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
		return NDIS_STATUS_SUCCESS;
	}

tx_start_error:
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), err);

	return err;
}


static  INT DO_RACFG_CMD_GET_TX_STATUS(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 count=0;
	
	count = OS_HTONL(pAd->ate.TxDoneCount);
	NdisMoveMemory(pRaCfg->data, &count, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_TX_STOP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_TX_STOP\n"));

	Set_ATE_Proc(pAd, "TXSTOP");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_RX_START(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	pAd->ate.bQARxStart = TRUE;
	Set_ATE_Proc(pAd, "RXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	


static  INT DO_RACFG_CMD_RX_STOP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_STOP\n"));

	Set_ATE_Proc(pAd, "RXSTOP");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_CARRIER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CARRIER\n"));

	Set_ATE_Proc(pAd, "TXCARR");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_CONT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CONT\n"));

	Set_ATE_Proc(pAd, "TXCONT");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_FRAME(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_FRAME\n"));

	Set_ATE_Proc(pAd, "TXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	


static  INT DO_RACFG_CMD_ATE_SET_BW(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_BW\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	Set_ATE_TX_BW_Proc(pAd, str);
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_POWER0(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER0\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_POWER0_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_POWER1(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER1\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_POWER1_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#ifdef DOT11N_SS3_SUPPORT
static  INT DO_RACFG_CMD_ATE_SET_TX_POWER2(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER2\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_POWER2_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* DOT11N_SS3_SUPPORT */


static  INT DO_RACFG_CMD_ATE_SET_FREQ_OFFSET(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_FREQ_OFFSET\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_FREQOFFSET_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_GET_STATISTICS(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_GET_STATISTICS\n"));

	memcpy_exl(pAd, &pRaCfg->data[0], (UCHAR *)&pAd->ate.TxDoneCount, 4);
	memcpy_exl(pAd, &pRaCfg->data[4], (UCHAR *)&pAd->WlanCounters.RetryCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[8], (UCHAR *)&pAd->WlanCounters.FailedCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[12], (UCHAR *)&pAd->WlanCounters.RTSSuccessCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[16], (UCHAR *)&pAd->WlanCounters.RTSFailureCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[20], (UCHAR *)&pAd->WlanCounters.ReceivedFragmentCount.QuadPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[24], (UCHAR *)&pAd->WlanCounters.FCSErrorCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[28], (UCHAR *)&pAd->Counters8023.RxNoBuffer, 4);
	memcpy_exl(pAd, &pRaCfg->data[32], (UCHAR *)&pAd->WlanCounters.FrameDuplicateCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[36], (UCHAR *)&pAd->RalinkCounters.OneSecFalseCCACnt, 4);
	
	if (pAd->ate.RxAntennaSel == 0)
	{
		INT32 RSSI0 = 0;
		INT32 RSSI1 = 0;
		INT32 RSSI2 = 0;

		RSSI0 = (INT32)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta);
		RSSI1 = (INT32)(pAd->ate.LastRssi1 - pAd->BbpRssiToDbmDelta);
		RSSI2 = (INT32)(pAd->ate.LastRssi2 - pAd->BbpRssiToDbmDelta);
		memcpy_exl(pAd, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		memcpy_exl(pAd, &pRaCfg->data[44], (UCHAR *)&RSSI1, 4);
		memcpy_exl(pAd, &pRaCfg->data[48], (UCHAR *)&RSSI2, 4);
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+52, NDIS_STATUS_SUCCESS);
	}
	else
	{
		INT32 RSSI0 = 0;
	
		RSSI0 = (INT32)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta);
		memcpy_exl(pAd, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+44, NDIS_STATUS_SUCCESS);
	}

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_RESET_COUNTER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 1;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_RESET_COUNTER\n"));				

	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ResetStatCounter_Proc(pAd, str);

	pAd->ate.TxDoneCount = 0;

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SEL_TX_ANTENNA(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)	
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_TX_ANTENNA\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_Antenna_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SEL_RX_ANTENNA(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_RX_ANTENNA\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_RX_Antenna_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_PREAMBLE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_PREAMBLE\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_MODE_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_CHANNEL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_CHANNEL\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_CHANNEL_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR1(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR1\n"));

	/*
		Addr is an array of UCHAR,
		so no need to perform endian swap.
	*/
	memcpy(pAd->ate.Addr1, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR2(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR2\n"));

	/*
		Addr is an array of UCHAR,
		so no need to perform endian swap.
	*/
	memcpy(pAd->ate.Addr2, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR3(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR3\n"));

	/*
		Addr is an array of UCHAR,
		so no need to perform endian swap.
	*/
	memcpy(pAd->ate.Addr3, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_RATE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_RATE\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_MCS_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_FRAME_LEN\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_LENGTH_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_FRAME_COUNT\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);

#ifdef RTMP_MAC_PCI
	/* TX_FRAME_COUNT == 0 means tx infinitely */
	if (value == 0)
	{
		/* Use TxCount = 0xFFFFFFFF to approximate the infinity. */
		pAd->ate.TxCount = 0xFFFFFFFF;
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pAd->ate.TxCount));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	}
	else
#endif /* RTMP_MAC_PCI */
	{
		snprintf((char *)str, sizeof(str), "%d", value);
		Set_ATE_TX_COUNT_Proc(pAd, str);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_RX_FRAME(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	Set_ATE_Proc(pAd, "RXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_E2PROM_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE >> 1];
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
	
	rt_ee_read_all(pAd, (USHORT *)buffer);

	if (offset + len <= EEPROM_SIZE)
		memcpy_exs(pAd, pRaCfg->data, (UCHAR *)buffer+offset, len);
	else
		DBGPRINT(RT_DEBUG_ERROR, ("exceed EEPROM size\n"));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE >> 1];
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	memcpy_exs(pAd, (UCHAR *)buffer + offset, (UCHAR *)pRaCfg->data + 2, len);

	if ((offset + len) <= EEPROM_SIZE)
	{
		rt_ee_write_bulk(pAd,(USHORT *)(((UCHAR *)buffer) + offset), offset, len);
	}
	else
	{
		DBGPRINT_ERR(("exceed EEPROM size(%d)\n", EEPROM_SIZE));
		DBGPRINT_ERR(("offset=%d\n", offset));
		DBGPRINT_ERR(("length=%d\n", len));
		DBGPRINT_ERR(("offset+length=%d\n", (offset+len)));
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_IO_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 offset, i, value;
	USHORT len;
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);
	memcpy(&len, pRaCfg->data+2, 2);
	len = OS_NTOHS(len);
	
	for (i = 0; i < len; i += 4)
	{
		memcpy_exl(pAd, (UCHAR *)&value, pRaCfg->data+4+i, 4);
		DBGPRINT(RT_DEBUG_TRACE,("Write %x %x\n", offset + i, value));
		RTMP_IO_WRITE32(pAd, ((offset+i) & (0xffff)), value);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_BBP_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
	
	for (j = offset; j < (offset+len); j++)
	{
		pRaCfg->data[j - offset] = 0;
		
		ATE_BBPRead(pAd, j, &pRaCfg->data[j - offset]);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_BBP_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	UCHAR *value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
					
	for (j = offset; j < (offset+len); j++)
	{
		value = pRaCfg->data + 2 + (j - offset);
		ATE_BBPWrite(pAd, j,  *value);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#ifdef RT3883
static  INT DO_RACFG_CMD_ATE_ETH_EXT_SETTING(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32    value = 0;
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_ETH_EXT_SETTING\n"));
	memcpy((PUCHAR)&value, pRaCfg->data-2, 4);
	value = OS_NTOHL(value);
	
	if (value == 0)
	{	
		/* GPIO pull low */
		RTMPRT3883ABandSel(36);
	}
	else
	{	
		/* GPIO pull high */
		RTMPRT3883ABandSel(1);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* RT3883 */


#if defined (RT3883) || defined (RT3352) || defined (RT5350)
/* busy mode - make CPU in ATE mode as busy as in normal driver */ 
static  INT DO_RACFG_CMD_ATE_RUN_CPUBUSY(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32    mode = 0;
	char *argv_busy[] = {"/sbin/cpubusy.sh", "2", NULL };
	char *argv_idle[] = {"/usr/bin/killall", "cpubusy.sh", "3", NULL };

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_RUN_CPUBUSY\n"));
	memcpy((PUCHAR)&mode, pRaCfg->data-2, 4);
	mode = OS_NTOHL(mode);

	if (mode == 0)
	{ 
		/* idle mode */
		call_usermodehelper(argv_idle[0], argv_idle, NULL, 0);
	}
	else
	{ 
		/* busy mode */
		call_usermodehelper(argv_busy[0], argv_busy, NULL, 0);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* defined (RT3883) || defined (RT3352) || defined (RT5350) */


#ifdef RTMP_RF_RW_SUPPORT
static  INT DO_RACFG_CMD_ATE_RF_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	for (j = offset; j < (offset+len); j++)
	{
		pRaCfg->data[j - offset] = 0;
		ATE_RF_IO_READ8_BY_REG_ID(pAd, j,  &pRaCfg->data[j - offset]);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_RF_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	UCHAR *value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	for (j = offset; j < (offset+len); j++)
	{
		value = pRaCfg->data + 2 + (j - offset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, j,  *value);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* RTMP_RF_RW_SUPPORT */


#ifdef RTMP_RBUS_SUPPORT
#ifdef TXBF_SUPPORT
static  INT DO_RACFG_CMD_ATE_TXBF_DUT_INIT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_DUT_INIT\n"));

	Set_ATE_TXBF_INIT_Proc(pAd, "1");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_LNA_CAL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT band;
	CHAR bandStr[32] = {0};
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_LNA_CAL\n"));

	band = OS_NTOHS(pRaCfg->status);
	DBGPRINT(RT_DEBUG_TRACE, ("%s(): band=0x%x(0x%x)\n", 
					__FUNCTION__, band, pRaCfg->status));
	snprintf(bandStr, sizeof(bandStr), "%d\n", band);
	Set_ATE_TXBF_LNACAL_Proc(pAd, &bandStr[0]);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_DIV_CAL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT band;
	CHAR bandStr[32] = {0};
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_DIV_CAL\n"));

	band = OS_NTOHS(pRaCfg->status);
	DBGPRINT(RT_DEBUG_TRACE, ("%s(): band=0x%x(0x%x)\n", 
				__FUNCTION__, band, pRaCfg->status));
	snprintf(bandStr, sizeof(bandStr), "%d\n", band);
	Set_ATE_TXBF_DIVCAL_Proc(pAd, &bandStr[0]);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_PHASE_CAL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];
	BOOLEAN	result = FALSE;

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_PHASE_CAL\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	result = (BOOLEAN)Set_ATE_TXBF_CAL_Proc(pAd, str);
	pRaCfg->data[0] = result;

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + 1, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_GOLDEN_INIT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_GOLDEN_INIT\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	Set_ATE_TXBF_GOLDEN_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_VERIFY(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];
	BOOLEAN	result;

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_VERIFY\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	result = (BOOLEAN)Set_ATE_TXBF_VERIFY_Proc(pAd, str);

	pRaCfg->data[0] = result;
	pRaCfg->data[1] = pAd->ate.calParams[0];
	pRaCfg->data[2] = pAd->ate.calParams[1];
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + 3, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_VERIFY_NOCOMP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];
	BOOLEAN	result;

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_VERIFY_NOCOMP\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	result = (BOOLEAN)Set_ATE_TXBF_VERIFY_NoComp_Proc(pAd, str);

	pRaCfg->data[0] = result;
	pRaCfg->data[1] = pAd->ate.calParams[0];
	pRaCfg->data[2] = pAd->ate.calParams[1];
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + 3, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* TXBF_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */


static INT32 DO_RACFG_CMD_ATE_SHOW_PARAM(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	UINT32 Len;
	ATE_EX_PARAM ATEExParam;
	PATE_INFO pATEInfo;

	pATEInfo = &pAd->ate;
	
	ATEExParam.mode = pATEInfo->Mode;
	ATEExParam.TxPower0 = pATEInfo->TxPower0;
	ATEExParam.TxPower1 = pATEInfo->TxPower1;

#ifdef DOT11N_SS3_SUPPORT
	ATEExParam.TxPower2 = pATEInfo->TxPower2;
#endif /* DOT11N_SS3_SUPPORT */

	ATEExParam.TxAntennaSel = pATEInfo->TxAntennaSel;
	ATEExParam.RxAntennaSel = pATEInfo->RxAntennaSel;

#ifdef CONFIG_AP_SUPPORT
	NdisMoveMemory(ATEExParam.DA, pATEInfo->Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(ATEExParam.SA, pATEInfo->Addr3, MAC_ADDR_LEN);
	NdisMoveMemory(ATEExParam.BSSID, pATEInfo->Addr2, MAC_ADDR_LEN);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	NdisMoveMemory(ATEExParam.DA, pATEInfo->Addr3, MAC_ADDR_LEN);
	NdisMoveMemory(ATEExParam.SA, pATEInfo->Addr2, MAC_ADDR_LEN);
	NdisMoveMemory(ATEExParam.BSSID, pATEInfo->Addr1, MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */

	ATEExParam.MCS = pATEInfo->TxWI.MCS;
	ATEExParam.PhyMode = pATEInfo->TxWI.PHYMODE;
	ATEExParam.ShortGI = pATEInfo->TxWI.ShortGI;
	ATEExParam.BW = pATEInfo->TxWI.BW;
	ATEExParam.Channel = OS_HTONL(pATEInfo->Channel);
	ATEExParam.TxLength = OS_HTONL(pATEInfo->TxLength);
	ATEExParam.TxCount = OS_HTONL(pATEInfo->TxCount);
	ATEExParam.RFFreqOffset = OS_HTONL(pATEInfo->RFFreqOffset);
	ATEExParam.IPG = OS_HTONL(pATEInfo->IPG);
	ATEExParam.RxTotalCnt = OS_HTONL(pATEInfo->RxTotalCnt);
	ATEExParam.RxCntPerSec = OS_HTONL(pATEInfo->RxCntPerSec);
	ATEExParam.LastSNR0 = pATEInfo->LastSNR0;
	ATEExParam.LastSNR1 = pATEInfo->LastSNR1;
#ifdef DOT11N_SS3_SUPPORT
	ATEExParam.LastSNR2 = pATEInfo->LastSNR2;
#endif /* DOT11N_SS3_SUPPORT */
	ATEExParam.LastRssi0 = pATEInfo->LastRssi0;
	ATEExParam.LastRssi1 = pATEInfo->LastRssi1;
	ATEExParam.LastRssi2 = pATEInfo->LastRssi2;
	ATEExParam.AvgRssi0 = pATEInfo->AvgRssi0;
	ATEExParam.AvgRssi1 = pATEInfo->AvgRssi1;
	ATEExParam.AvgRssi2 = pATEInfo->AvgRssi2;
	ATEExParam.AvgRssi0X8 = OS_HTONS(pATEInfo->AvgRssi0X8);
	ATEExParam.AvgRssi1X8 = OS_HTONS(pATEInfo->AvgRssi1X8);
	ATEExParam.AvgRssi2X8 = OS_HTONS(pATEInfo->AvgRssi2X8);

	Len = sizeof(ATEExParam);
	NdisMoveMemory(pRaCfg->data, &ATEExParam, Len);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + Len, NDIS_STATUS_SUCCESS);
	
	return Status;
}


typedef INT (*RACFG_CMD_HANDLER)(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg);


static RACFG_CMD_HANDLER RACFG_CMD_SET1[] =
{
	/* cmd id start from 0x0 */
	DO_RACFG_CMD_RF_WRITE_ALL,/* 0x0000 */	
	DO_RACFG_CMD_E2PROM_READ16,/* 0x0001 */
	DO_RACFG_CMD_E2PROM_WRITE16,/* 0x0002 */
	DO_RACFG_CMD_E2PROM_READ_ALL,/* 0x0003 */
	DO_RACFG_CMD_E2PROM_WRITE_ALL,/* 0x0004 */
	DO_RACFG_CMD_IO_READ,/* 0x0005 */
	DO_RACFG_CMD_IO_WRITE,/* 0x0006 */
	DO_RACFG_CMD_IO_READ_BULK,/* 0x0007 */
	DO_RACFG_CMD_BBP_READ8,/* 0x0008 */
	DO_RACFG_CMD_BBP_WRITE8,/* 0x0009 */
	DO_RACFG_CMD_BBP_READ_ALL,/* 0x000a */
	DO_RACFG_CMD_GET_COUNTER,/* 0x000b */
	DO_RACFG_CMD_CLEAR_COUNTER,/* 0x000c */
	NULL /* RACFG_CMD_RSV1 */,/* 0x000d */
	NULL /* RACFG_CMD_RSV2 */,/* 0x000e */
	NULL /* RACFG_CMD_RSV3 */,/* 0x000f */
	DO_RACFG_CMD_TX_START,/* 0x0010 */
	DO_RACFG_CMD_GET_TX_STATUS,/* 0x0011 */
	DO_RACFG_CMD_TX_STOP,/* 0x0012 */
	DO_RACFG_CMD_RX_START,/* 0x0013 */
	DO_RACFG_CMD_RX_STOP,/* 0x0014 */
	DO_RACFG_CMD_GET_NOISE_LEVEL,/* 0x0015 */
#if defined(RT2883) || defined(RT3883)
	DO_RACFG_CMD_QUERY_BF_RSP,/* 0x0016 */
	DO_RACFG_CMD_QUERY_IBF_TAG,/* 0x0017 */
	DO_RACFG_CMD_QUERY_IBF_TAG,/* 0x0018 */
	DO_RACFG_CMD_QUERY_IBF_PROFILE,/* 0x0019 */
	DO_RACFG_CMD_QUERY_IBF_PROFILE,/* 0x001a */
	DO_RACFG_CMD_WRITE_IBF_TAG,/* 0x001b */
	DO_RACFG_CMD_WRITE_IBF_TAG,/* 0x001c */
	DO_RACFG_CMD_WRITE_IBF_PROFILE,/* 0x001d */
	DO_RACFG_CMD_WRITE_IBF_PROFILE,/* 0x001e */
	NULL /* RACFG_CMD_RSV4 */,/* 0x001f */	
	DO_RACFG_CALIBRATION_CAPTURE/* 0x0020 */	
#else
	NULL
#endif /* defined(RT2883) || defined(RT3883) */
	/* cmd id end with 0x20 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET2[] =
{
	/* cmd id start from 0x80 */
	DO_RACFG_CMD_ATE_START,/* 0x0080 */
	DO_RACFG_CMD_ATE_STOP/* 0x0081 */
	/* cmd id end with 0x81 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET3[] =
{
	/* cmd id start from 0x100 */
	DO_RACFG_CMD_ATE_START_TX_CARRIER,/* 0x0100 */
	DO_RACFG_CMD_ATE_START_TX_CONT,/* 0x0101 */
	DO_RACFG_CMD_ATE_START_TX_FRAME,/* 0x0102 */
	DO_RACFG_CMD_ATE_SET_BW,/* 0x0103 */
	DO_RACFG_CMD_ATE_SET_TX_POWER0,/* 0x0104 */
	DO_RACFG_CMD_ATE_SET_TX_POWER1,/* 0x0105 */
	DO_RACFG_CMD_ATE_SET_FREQ_OFFSET,/* 0x0106 */
	DO_RACFG_CMD_ATE_GET_STATISTICS,/* 0x0107 */
	DO_RACFG_CMD_ATE_RESET_COUNTER,/* 0x0108 */
	DO_RACFG_CMD_ATE_SEL_TX_ANTENNA,/* 0x0109 */
	DO_RACFG_CMD_ATE_SEL_RX_ANTENNA,/* 0x010a */
	DO_RACFG_CMD_ATE_SET_PREAMBLE,/* 0x010b */
	DO_RACFG_CMD_ATE_SET_CHANNEL,/* 0x010c */
	DO_RACFG_CMD_ATE_SET_ADDR1,/* 0x010d */
	DO_RACFG_CMD_ATE_SET_ADDR2,/* 0x010e */
	DO_RACFG_CMD_ATE_SET_ADDR3,/* 0x010f */
	DO_RACFG_CMD_ATE_SET_RATE,/* 0x0110 */
	DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN,/* 0x0111 */
	DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT,/* 0x0112 */
	DO_RACFG_CMD_ATE_START_RX_FRAME,/* 0x0113 */
	DO_RACFG_CMD_ATE_E2PROM_READ_BULK,/* 0x0114 */
	DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK,/* 0x0115 */
	DO_RACFG_CMD_ATE_IO_WRITE_BULK,/* 0x0116 */
	DO_RACFG_CMD_ATE_BBP_READ_BULK,/* 0x0117 */
	DO_RACFG_CMD_ATE_BBP_WRITE_BULK,/* 0x0118 */
#ifdef RTMP_RF_RW_SUPPORT
	DO_RACFG_CMD_ATE_RF_READ_BULK,/* 0x0119 */
	DO_RACFG_CMD_ATE_RF_WRITE_BULK,/* 0x011a */
#else
	NULL,/* 0x0119 */
	NULL,/* 0x011a */
#endif /* RTMP_RF_RW_SUPPORT */
#ifdef DOT11N_SS3_SUPPORT
	DO_RACFG_CMD_ATE_SET_TX_POWER2,/* 0x011b */
#else
	NULL,/* 0x011b */
#endif /* DOT11N_SS3_SUPPORT */
#ifdef TXBF_SUPPORT
	DO_RACFG_CMD_ATE_TXBF_DUT_INIT,/* 0x011c */
	DO_RACFG_CMD_ATE_TXBF_LNA_CAL,/* 0x011d */
	DO_RACFG_CMD_ATE_TXBF_DIV_CAL,/* 0x011e */
	DO_RACFG_CMD_ATE_TXBF_PHASE_CAL,/* 0x011f */
	DO_RACFG_CMD_ATE_TXBF_GOLDEN_INIT,/* 0x0120 */
	DO_RACFG_CMD_ATE_TXBF_VERIFY,/* 0x0121 */
	DO_RACFG_CMD_ATE_TXBF_VERIFY_NOCOMP/* 0x0122 */
#else
	NULL,/* 0x011c */
	NULL,/* 0x011d */
	NULL,/* 0x011e */
	NULL,/* 0x011f */
	NULL,/* 0x0120 */
	NULL,/* 0x0121 */
	NULL/* 0x0122 */
#endif /* TXBF_SUPPORT */
	/* cmd id end with 0x122 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET4[] =
{
	/* cmd id start from 0x200 */
#ifdef RT3883
	DO_RACFG_CMD_ATE_ETH_EXT_SETTING,/* 0x0200 */
#else
	NULL,/* 0x0200 */
#endif /* RT3883 */
	NULL,/* 0x0201 */
#if defined (RT3883) || defined (RT3352) || defined (RT5350)
	DO_RACFG_CMD_ATE_RUN_CPUBUSY/* 0x0202 */
#else
	NULL/* 0x0202 */
#endif /* defined (RT3883) || defined (RT3352) || defined (RT5350) */
	/* cmd id end with 0x202 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET5[] =
{
	DO_RACFG_CMD_ATE_SHOW_PARAM
};


typedef struct RACFG_CMD_TABLE_{
	RACFG_CMD_HANDLER *cmdSet;
	int	cmdSetSize;
	int	cmdOffset;
}RACFG_CMD_TABLE;


RACFG_CMD_TABLE RACFG_CMD_TABLES[]={
	{
		RACFG_CMD_SET1,
		sizeof(RACFG_CMD_SET1) / sizeof(RACFG_CMD_HANDLER),
		0x0,
	},
	{
		RACFG_CMD_SET2,
		sizeof(RACFG_CMD_SET2) / sizeof(RACFG_CMD_HANDLER),
		0x80,
	},
	{
		RACFG_CMD_SET3,
		sizeof(RACFG_CMD_SET3) / sizeof(RACFG_CMD_HANDLER),
		0x100,
	},
	{
		RACFG_CMD_SET4,
		sizeof(RACFG_CMD_SET4) / sizeof(RACFG_CMD_HANDLER),
		0x200,
	},
	{
		RACFG_CMD_SET5,
		sizeof(RACFG_CMD_SET5) / sizeof(RACFG_CMD_HANDLER),
		0xff00,
	}
	
};


static INT32 RACfgCMDHandler(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN pRACFGHDR pRaCfg)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	USHORT Command_Id;
	UINT32 TableIndex = 0;

	Command_Id = OS_NTOHS(pRaCfg->command_id);
	DBGPRINT(RT_DEBUG_TRACE,("\n%s: Command_Id = 0x%04x !\n", __FUNCTION__, Command_Id));
	
	while (TableIndex < (sizeof(RACFG_CMD_TABLES) / sizeof(RACFG_CMD_TABLE)))
	{
 		int cmd_index = 0;
 		cmd_index = Command_Id - RACFG_CMD_TABLES[TableIndex].cmdOffset;
 		if ((cmd_index >= 0) && (cmd_index < RACFG_CMD_TABLES[TableIndex].cmdSetSize))
 		{
			RACFG_CMD_HANDLER *pCmdSet;

			pCmdSet = RACFG_CMD_TABLES[TableIndex].cmdSet;
			
			if (pCmdSet[cmd_index] != NULL)
				Status = (*pCmdSet[cmd_index])(pAd, wrq, pRaCfg);
			break;
		}
		TableIndex++;
	}

	/* In passive mode, only commands that read registers are allowed. */ 
	if (pAd->ate.PassiveMode)
	{
		int i, allowCmd = FALSE;
		static int allowedCmds[] = {
				RACFG_CMD_E2PROM_READ16, RACFG_CMD_E2PROM_READ_ALL,
				RACFG_CMD_IO_READ, RACFG_CMD_IO_READ_BULK,
				RACFG_CMD_BBP_READ8, RACFG_CMD_BBP_READ_ALL,
				RACFG_CMD_ATE_E2PROM_READ_BULK,
				RACFG_CMD_ATE_BBP_READ_BULK,
				RACFG_CMD_ATE_RF_READ_BULK,
#if defined(RT2883) || defined(RT3883)
				RACFG_CMD_QUERY_IBF_TAG, RACFG_CMD_QUERY_EBF_TAG,
				RACFG_CMD_QUERY_IBF_PROFILE, RACFG_CMD_QUERY_EBF_PROFILE
#endif /* defined(RT2883) || defined(RT3883) */
				};

		for (i=0; i<sizeof(allowedCmds)/sizeof(allowedCmds[0]); i++)
		{
			if (Command_Id==allowedCmds[i])
			{
				allowCmd = TRUE;
				break;
			}
		}

		/* Also allow writes to BF profile registers */
		if (Command_Id==RACFG_CMD_BBP_WRITE8)
		{
			int offset = OS_NTOHS(pRaCfg->status);
			if (offset==BBP_R27 || (offset>=BBP_R174 && offset<=BBP_R182))
				allowCmd = TRUE;
		}

		/* If not allowed, then ignore the command. */
		if (!allowCmd)
		{
			ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
			Status = NDIS_STATUS_FAILURE;
		}
	}

	return Status;
}


INT RtmpDoAte(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT		*wrq,
	IN	PSTRING			wrq_name)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	struct ate_racfghdr *pRaCfg;
	UINT32 ATEMagicNum;

	os_alloc_mem_suspend(pAd, (UCHAR **)&pRaCfg, sizeof(struct ate_racfghdr));

	if (!pRaCfg)
	{
		Status = -ENOMEM;
		goto ERROR0;
	}
				
	NdisZeroMemory(pRaCfg, sizeof(struct ate_racfghdr));
	Status = copy_from_user((PUCHAR)pRaCfg, wrq->u.data.pointer, wrq->u.data.length);
	
	if (Status)
	{
		Status = -EFAULT;
		goto ERROR1;
	}

	ATEMagicNum = OS_NTOHL(pRaCfg->magic_no);
	
	switch(ATEMagicNum)
	{
		case RACFG_MAGIC_NO:
			Status = RACfgCMDHandler(pAd, wrq, pRaCfg);
			break;

		default:
			Status = NDIS_STATUS_FAILURE;
			DBGPRINT(RT_DEBUG_ERROR, ("Unknown magic number of RACFG command = %x\n", ATEMagicNum));
			break;
	}
	
 ERROR1:
	os_free_mem(NULL, pRaCfg);
 ERROR0:
	return Status;
}


VOID ATE_QA_Statistics(
	IN PRTMP_ADAPTER			pAd,
	IN PRXWI_STRUC				pRxWI,
	IN PRT28XX_RXD_STRUC		pRxD,
	IN PHEADER_802_11			pHeader)
{
#if defined(RT2883) || defined(RT3883)
	UINT32 lastRxRate = 0;
#endif /* defined(RT2883) || defined(RT3883) */

	/* update counter first */
	if (pHeader != NULL)
	{
		if (pHeader->FC.Type == BTYPE_DATA)
		{
			if (pRxD->U2M)
			{
				pAd->ate.U2M++;
#if defined(RT2883) || defined(RT3883)
				if (pRxWI->MPDUtotalByteCount >= 32)
					lastRxRate = (pRxWI->MCS) + (pRxWI->BW <<7) + (pRxWI->ShortGI <<8) + (pRxWI->STBC <<9) + (pRxWI->PHYMODE <<14) + (pRxWI->MPDUtotalByteCount<<16);
#endif /* defined(RT2883) || defined(RT3883) */
			}
			else
				pAd->ate.OtherData++;
		}
		else if (pHeader->FC.Type == BTYPE_MGMT)
		{
			if (pHeader->FC.SubType == SUBTYPE_BEACON)
				pAd->ate.Beacon++;
			else
				pAd->ate.OtherCount++;
		}
		else if (pHeader->FC.Type == BTYPE_CNTL)
		{
			pAd->ate.OtherCount++;
		}
	}
	pAd->ate.RSSI0 = pRxWI->RSSI0; 
	pAd->ate.RSSI1 = pRxWI->RSSI1; 
	pAd->ate.RSSI2 = pRxWI->RSSI2; 
	pAd->ate.SNR0 = pRxWI->SNR0;
	pAd->ate.SNR1 = pRxWI->SNR1;

#if defined(RT2883) || defined(RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
#ifdef DOT11N_SS3_SUPPORT
		pAd->ate.SNR2 = pRxWI->SNR2;
#endif /* DOT11N_SS3_SUPPORT */
#ifdef TXBF_SUPPORT
		pAd->ate.LastRxRate = lastRxRate;
#endif /* TXBF_SUPPORT */
		pAd->ate.BF_SNR[0] = BF_SNR_OFFSET + pRxWI->BF_SNR0;
		pAd->ate.BF_SNR[1] = BF_SNR_OFFSET + pRxWI->BF_SNR1;
#ifdef DOT11N_SS3_SUPPORT
		pAd->ate.BF_SNR[2] = BF_SNR_OFFSET + pRxWI->BF_SNR2;
#endif /* DOT11N_SS3_SUPPORT */
	}
#endif /* defined(RT2883) || defined(RT3883) */

}


INT Set_TxStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	DBGPRINT(RT_DEBUG_TRACE,("Set_TxStop_Proc\n"));

	if (Set_ATE_Proc(pAd, "TXSTOP"))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


INT Set_RxStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	DBGPRINT(RT_DEBUG_TRACE,("Set_RxStop_Proc\n"));

	if (Set_ATE_Proc(pAd, "RXSTOP"))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


#ifdef DBG
INT Set_EERead_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT buffer[EEPROM_SIZE >> 1];
	USHORT *p;
	INT i;
	
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


INT Set_EEWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT offset = 0, value;
	PSTRING p2 = arg;
	
	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Hex(offset, arg);
		A2Hex(value, p2 + 1);
	}
	else
	{
		A2Hex(value, arg);
	}
	
	if (offset >= EEPROM_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Offset can not exceed EEPROM_SIZE( == 0x%04x)\n", EEPROM_SIZE));	
		return FALSE;
	}
	
	RT28xx_EEPROM_WRITE16(pAd, offset, value);

	return TRUE;
}


INT Set_BBPRead_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR value = 0, offset;

	A2Hex(offset, arg);	
			
	ATE_BBPRead(pAd, offset, &value);

	DBGPRINT(RT_DEBUG_OFF, ("%x\n", value));
		
	return TRUE;
}


INT Set_BBPWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT offset = 0;
	PSTRING p2 = arg;
	UCHAR value;
	
	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Hex(offset, arg);	
		A2Hex(value, p2 + 1);	
	}
	else
	{
		A2Hex(value, arg);	
	}

	ATE_BBPWrite(pAd, offset, value);

	return TRUE;
}


INT Set_RFWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PSTRING p2, p3, p4;
	UINT32 R1, R2, R3, R4;
	
	p2 = arg;

	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 != ':')
		return FALSE;
	
	p3 = p2 + 1;

	while((*p3 != ':') && (*p3 != '\0'))
	{
		p3++;
	}

	if (*p3 != ':')
		return FALSE;
	
	p4 = p3 + 1;

	while ((*p4 != ':') && (*p4 != '\0'))
	{
		p4++;
	}

	if (*p4 != ':')
		return FALSE;

		
	A2Hex(R1, arg);	
	A2Hex(R2, p2 + 1);	
	A2Hex(R3, p3 + 1);	
	A2Hex(R4, p4 + 1);	
	
	RTMP_RF_IO_WRITE32(pAd, R1);
	RTMP_RF_IO_WRITE32(pAd, R2);
	RTMP_RF_IO_WRITE32(pAd, R3);
	RTMP_RF_IO_WRITE32(pAd, R4);
	
	return TRUE;
}
#endif /* DBG */
#endif /* RALINK_QA */


static int CheckMCSValid(
	IN PRTMP_ADAPTER	pAd, 
	IN UCHAR Mode,
	IN UCHAR Mcs)
{
	int i;
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
			DBGPRINT(RT_DEBUG_ERROR, ("unrecognizable Tx Mode %d\n", Mode));
			return -1;
			break;
	}

	i = 0;
	while (pRateTab[i] != -1)
	{
		if (pRateTab[i] == Mcs)
			return 0;
		i++;
	}

	return -1;
}


#ifdef RTMP_MAC_PCI
/*
========================================================================
	Routine Description:
		Write TxWI for ATE mode.
		
	Return Value:
		None
========================================================================
*/
static VOID ATEWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC 	pOutTxWI,	
	IN	BOOLEAN			FRAG,	
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN 		AMPDU,
	IN	BOOLEAN 		Ack,
	IN	BOOLEAN 		NSeq,		/* HW new a sequence. */
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR 			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,	
	IN	BOOLEAN			CfAck,	
	IN	HTTRANSMIT_SETTING	*pTransmit)
{
	TXWI_STRUC 		TxWI;
	PTXWI_STRUC 	pTxWI;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	/*
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWISize);
	pTxWI = &TxWI;

	pTxWI->FRAG= FRAG;

	pTxWI->CFACK = CFACK;
	pTxWI->TS= InsTimestamp;
	pTxWI->AMPDU = AMPDU;
	pTxWI->ACK = Ack;
	pTxWI->txop= Txopmode;
	
	pTxWI->NSEQ = NSeq;

	/* John tune the performace with Intel Client in 20 MHz performance */
	if ( BASize >7 )
		BASize =7;
		
	pTxWI->BAWinSize = BASize;	
	pTxWI->WirelessCliID = WCID;
	pTxWI->MPDUtotalByteCount = Length; 
	pTxWI->PacketId = PID; 
	
	/* If CCK or OFDM, BW must be 20 */
	pTxWI->BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	pTxWI->ShortGI = pTransmit->field.ShortGI;
	pTxWI->STBC = pTransmit->field.STBC;
	
	pTxWI->MCS = pTransmit->field.MCS;
	pTxWI->PHYMODE = pTransmit->field.MODE;
	pTxWI->CFACK = CfAck;
	pTxWI->MIMOps = 0;
	pTxWI->MpduDensity = 0;

	pTxWI->PacketId = pTxWI->MCS;
	NdisMoveMemory(pOutTxWI, &TxWI, TXWISize);

    return;
}


/* 
==========================================================================
	Description:
		Setup Frame format.
	NOTE:
		This routine should only be used in ATE mode.
==========================================================================
*/
static INT ATESetUpFrame(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx)
{
	UINT j;
	PTXD_STRUC pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	PNDIS_PACKET pPacket=NULL;
	PUCHAR pDest=NULL;
	PVOID AllocVa=NULL;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	HTTRANSMIT_SETTING	TxHTPhyMode;

	PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];
	PTXWI_STRUC pTxWI = (PTXWI_STRUC) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	PUCHAR pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

#ifdef RALINK_QA
	PHEADER_802_11	pHeader80211;
#endif /* RALINK_QA */

	/* fill TxWI */
	TxHTPhyMode.field.BW = pAd->ate.TxWI.BW;
	TxHTPhyMode.field.ShortGI = pAd->ate.TxWI.ShortGI;
	TxHTPhyMode.field.STBC = pAd->ate.TxWI.STBC;
	TxHTPhyMode.field.MCS = pAd->ate.TxWI.MCS;
	TxHTPhyMode.field.MODE = pAd->ate.TxWI.PHYMODE;

	if (pAd->ate.bQATxStart == TRUE) 
	{
		/* always use QID_AC_BE and FIFO_EDCA */
		ATEWriteTxWI(pAd, pTxWI, pAd->ate.TxWI.FRAG, pAd->ate.TxWI.CFACK,
			pAd->ate.TxWI.TS,  pAd->ate.TxWI.AMPDU, pAd->ate.TxWI.ACK, pAd->ate.TxWI.NSEQ, 
			pAd->ate.TxWI.BAWinSize, 0, pAd->ate.TxWI.MPDUtotalByteCount, pAd->ate.TxWI.PacketId, 0, 0,
			pAd->ate.TxWI.txop/*IFS_HTTXOP*/, pAd->ate.TxWI.CFACK/*FALSE*/, &TxHTPhyMode);

#ifdef TXBF_SUPPORT
		if (IS_RT2883(pAd) || IS_RT3883(pAd))
		{
			/* Must copy rsv bits to actual TxWI */
			pTxWI->rsv = pAd->ate.TxWI.rsv;
			pTxWI->iTxBF = pAd->ate.TxWI.iTxBF;	
			pTxWI->Sounding = pAd->ate.TxWI.Sounding;
			pTxWI->eTxBF = pAd->ate.TxWI.eTxBF;
			pTxWI->Autofallback = pAd->ate.TxWI.Autofallback;
			pTxWI->NDPSndBW = pAd->ate.TxWI.NDPSndBW;
			pTxWI->NDPSndRate = pAd->ate.TxWI.NDPSndRate;
		}
#endif /* TXBF_SUPPORT */
	}
	else
	{
		ATEWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE,  FALSE, FALSE, FALSE, 
			4, 0, pAd->ate.TxLength, 0, 0, 0, IFS_HTTXOP, FALSE, &TxHTPhyMode);

#ifdef TXBF_SUPPORT
		if (pAd->ate.bTxBF == 1)
		{
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				pTxWI->rsv = 0;
				pTxWI->iTxBF = pAd->ate.TxWI.iTxBF;	
				pTxWI->Sounding = (pAd->ate.txSoundingMode == 1 ? 1 : 0);
				pTxWI->eTxBF = pAd->ate.TxWI.eTxBF;
				pTxWI->Autofallback = pAd->ate.TxWI.Autofallback;
				pTxWI->NDPSndBW = pAd->ate.TxWI.BW;
				if (pAd->ate.txSoundingMode == 3)
					pTxWI->NDPSndRate = 2;
				else if (pAd->ate.txSoundingMode == 2)
					pTxWI->NDPSndRate = 1;
				else
					pTxWI->NDPSndRate = 0;
			}
		}
#endif /* TXBF_SUPPORT */
	}
	
	/* fill 802.11 header */
#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE) 
	{
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize, pAd->ate.Header, pAd->ate.HLen);
	}
	else
#endif /* RALINK_QA */
	{
		pAd->ate.HLen = LENGTH_802_11;
#ifdef TXBF_SUPPORT
		TemplateFrame[0] = 0x08;	/* Data */
		TemplateFrame[1] = 0x00;
		if (pAd->ate.bTxBF && pAd->ate.txSoundingMode!=0)
		{
			/* QoS Data */
			pAd->ate.HLen = 32;
			TemplateFrame[0] = 0x88;
			TemplateFrame[1] = 0x80;
		
			switch (pAd->ate.txSoundingMode) {
			case 1:
				/* Data Sounding */
				TemplateFrame[28] = pAd->CommonCfg.ETxBfNoncompress? 0x80: 0xc0;
				TemplateFrame[29] = 0x00;	
				break;
			case 2:
			case 3:
				/* 2 or 3 Stream NDP */
				TemplateFrame[28] = pAd->CommonCfg.ETxBfNoncompress? 0x80: 0xc0;
				TemplateFrame[29] = 0x01;	/* NDP Announce */
				break;
			default:
				TemplateFrame[28] = TemplateFrame[29] = 0x0;
			}
		}
#endif /* TXBF_SUPPORT */
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize, TemplateFrame, pAd->ate.HLen);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 4, pAd->ate.Addr1, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 10, pAd->ate.Addr2, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 16, pAd->ate.Addr3, ETH_LENGTH_OF_ADDRESS);
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_READ, FALSE);
#endif /* RT_BIG_ENDIAN */

	/* alloc buffer for payload */
#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) && (pAd->ate.DLen != 0)) 
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->ate.DLen + 0x100, FALSE, &AllocVa, &AllocPa);
	}
	else
#endif /* RALINK_QA */
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->ate.TxLength, FALSE, &AllocVa, &AllocPa);
	}

	if (pPacket == NULL)
	{
		pAd->ate.TxCount = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("%s fail to alloc packet space.\n", __FUNCTION__));
		return -1;
	}

	pTxRing->Cell[TxIdx].pNextNdisPacket = pPacket;
	pDest = (PUCHAR) AllocVa;

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) && (pAd->ate.DLen != 0)) 
	{
		GET_OS_PKT_LEN(pPacket) = pAd->ate.DLen;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pAd->ate.DLen;
#endif /* LIMUX */
	}
	else
#endif /* RALINK_QA */
	{
		GET_OS_PKT_LEN(pPacket) = pAd->ate.TxLength - LENGTH_802_11;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pAd->ate.TxLength - LENGTH_802_11;
#endif /* LINUX */
	}

	/* prepare frame payload */
#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) && (pAd->ate.DLen != 0))
	{
		/* copy pattern */
		if ((pAd->ate.PLen != 0))
		{
			int j;
			
			for (j = 0; j < pAd->ate.DLen; j+=pAd->ate.PLen)
			{
				memcpy(GET_OS_PKT_DATAPTR(pPacket) + j, pAd->ate.Pattern, pAd->ate.PLen);
			}
		}
	}
	else
#endif /* RALINK_QA */
	{
		for(j = 0; j < GET_OS_PKT_LEN(pPacket); j++)
			/* default payload is 0xA5 */
			pDest[j] = pAd->ate.Payload;/* kurtis: 0xAA ATE test EVM will be positive */
	}

	/* build Tx Descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#else
    pDestTxD  = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
    TxD = *pDestTxD;
    pTxD = &TxD;
#endif /* !RT_BIG_ENDIAN */

#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		/* prepare TxD */
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		/* build TX DESC */
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWISize + pAd->ate.HLen;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = GET_OS_PKT_LEN(pPacket);
		pTxD->LastSec0 = (pTxD->SDLen1 == 0) ? 1 : 0;
		pTxD->LastSec1 = 1;

		pDest = (PUCHAR)pTxWI;
		pDest += TXWISize;
		pHeader80211 = (PHEADER_802_11)pDest;
		
		/* modify sequence number... */
		if (pAd->ate.TxDoneCount == 0)
		{
			pAd->ate.seq = pHeader80211->Sequence;
		}
		else
			pHeader80211->Sequence = ++pAd->ate.seq;
	}
	else
#endif /* RALINK_QA */
	{
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		/* build TX DESC */
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow (pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWISize + LENGTH_802_11;
		pTxD->LastSec0 = 0;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = GET_OS_PKT_LEN(pPacket);
		pTxD->LastSec1 = 1;
	}

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	return 0;
}


/*=======================End of RTMP_MAC_PCI =======================*/
#endif /* RTMP_MAC_PCI */




INT DefaultATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	CHAR TxPower = 0;
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR RFValue = 0;
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */

	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

#ifdef RTMP_RF_RW_SUPPORT

		if ((IS_RT30xx(pAd)  || IS_RT3390(pAd)))
		{
			{
				/* Set Tx Power */
				UCHAR ANT_POWER_INDEX=RF_R12+index;
				ATE_RF_IO_READ8_BY_REG_ID(pAd, ANT_POWER_INDEX, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPower;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, ANT_POWER_INDEX, (UCHAR)RFValue);
				DBGPRINT(RT_DEBUG_TRACE, ("3070 or 2070:%s (TxPower[%d]=%d, RFValue=%x)\n", __FUNCTION__, index,TxPower, RFValue));
			}
		}
		else
#endif /* RTMP_RF_RW_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
	
	return 0;
}


#ifdef RT3883
INT RT3883ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	CHAR TxPower = 0;

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */
	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
#ifdef DOT11N_SS3_SUPPORT
	else if (index == 2)
	{
		TxPower = pAd->ate.TxPower2;
	}
#endif /* DOT11N_SS3_SUPPORT */
	else
	{
#ifdef DOT11N_SS3_SUPPORT
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0, TxPower1, and TxPower2 are adjustable !\n"));
#else
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
#endif /* DOT11N_SS3_SUPPORT */
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

	if (pAd->ate.Channel <= 14)
	{
		if (index == 0)
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, TxPower);
		else if (index == 1)
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, TxPower);
		else if (index == 2)
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, TxPower);
	}
	else 
	{
		CHAR power; 
		power = 0x48 | ((TxPower & 0x18) << 1) | (TxPower & 0x7);
		if (index == 0)
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, power);
		else if (index == 1)
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, power);
		else if (index == 2)		
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, power);
	}

	return 0;
}
#endif /* RT3883 */


#ifdef RT305x
#if defined(RT3050) || defined(RT3052) || defined(RT3350)
/* This routine is also for RT3050/RT3052/RT3350 */
INT RT305xATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	CHAR TxPower = 0;
	UCHAR RFValue = 0;

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */
	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}
		
	if (index == 0)
	{
		/* Set Tx0 Power */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
		RFValue = (RFValue & 0xE0) | TxPower;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);
	}
	else if (index == 1)
	{
		/* Set Tx1 Power */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R13, (PUCHAR)&RFValue);
		RFValue = (RFValue & 0xE0) | TxPower;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower%d=%d, RFValue=%x)\n", __FUNCTION__, index, TxPower, RFValue));

	return 0;
}
#endif /* defined(RT3050) || defined(RT3052) || defined(RT3350) */


#ifdef RT3352
INT RT3352ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	CHAR TxPower = 0;

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */
	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

	if (index == 0)
	{
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R47, TxPower);
	}
	else if (index == 1)
	{
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R48, TxPower);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("3352:%s (TxPower%d=%d)\n", __FUNCTION__, index, TxPower));
	
	return 0;
}	
#endif /* RT3352 */


#endif /* RT305x */		




#ifdef RT2883
INT RT2883ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	ULONG R;
	CHAR TxPower = 0;
	UCHAR Bbp94 = 0;
	BOOLEAN bPowerReduce = FALSE;

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */
	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
#ifdef DOT11N_SS3_SUPPORT
	else if (index == 2)
	{
		TxPower = pAd->ate.TxPower2;
	}
#endif /* DOT11N_SS3_SUPPORT */
	else
	{
#ifdef DOT11N_SS3_SUPPORT
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0, TxPower1, and TxPower2 are adjustable !\n"));
#else
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
#endif /* DOT11N_SS3_SUPPORT */
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

	if (pAd->ate.Channel <= 14)
	{
		if (TxPower > 31)
		{
			/* R3, R4 can't large than 31 (0x24), 31 ~ 36 used by BBP 94 */
			R = 31;
			if (TxPower <= 36)
				Bbp94 = BBPR94_DEFAULT + (UCHAR)(TxPower - 31);		
		}
		else if (TxPower < 0)
		{
			/* R3, R4 can't less than 0, -1 ~ -6 used by BBP 94 */
			R = 0;
			if (TxPower >= -6)
				Bbp94 = BBPR94_DEFAULT + TxPower;
		}
		else
		{  
			/* 0 ~ 31 */
			R = (ULONG) TxPower;
			Bbp94 = BBPR94_DEFAULT;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%ld, BBP_R94=%d)\n", __FUNCTION__, TxPower, R, Bbp94));
	}
	else /* 5.5 GHz */
	{
		if (TxPower > 15)
		{
			/* R3, R4 can't large than 15 (0x0F) */
			R = 15;
		}
		else if (TxPower < 0)
		{
			/* R3, R4 can't less than 0 */
			/* -1 ~ -7 */
			ASSERT((TxPower >= -7));
			R = (ULONG)(TxPower + 7);
			bPowerReduce = TRUE;
		}
		else
		{  
			/* 0 ~ 15 */
			R = (ULONG) TxPower;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%lu)\n", __FUNCTION__, TxPower, R));
	}

	if (pAd->ate.Channel <= 14)
	{
		if (index == 0)
		{
			/* shift TX power control to correct RF(R3) register bit position */
			R = R << 9;		
			R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
			pAd->LatchRfRegs.R3 = R;
		}
		else
		{
			/* shift TX power control to correct RF(R4) register bit position */
			R = R << 6;		
			R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
			pAd->LatchRfRegs.R4 = R;
		}
	}
	else /* 5.5GHz */
	{
		if (bPowerReduce == FALSE)
		{
			if (index == 0)
			{
				/* shift TX power control to correct RF(R3) register bit position */
				R = (R << 10) | (1 << 9);		
				R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
				pAd->LatchRfRegs.R3 = R;
			}
			else
			{
				/* shift TX power control to correct RF(R4) register bit position */
				R = (R << 7) | (1 << 6);		
				R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
				pAd->LatchRfRegs.R4 = R;
			}
		}
		else
		{
			if (index == 0)
			{
				/* shift TX power control to correct RF(R3) register bit position */
				R = (R << 10);		
				R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);

				/* Clear bit 9 of R3 to reduce 7dB. */ 
				pAd->LatchRfRegs.R3 = (R & (~(1 << 9)));
			}
			else
			{
				/* shift TX power control to correct RF(R4) register bit position */
				R = (R << 7);		
				R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);

				/* Clear bit 6 of R4 to reduce 7dB. */ 
				pAd->LatchRfRegs.R4 = (R & (~(1 << 6)));
			}
		}
	}
	RtmpRfIoWrite(pAd);

	return 0;
}
#endif /* RT2883 */


#if defined(RT28xx) || defined(RT2880)
INT RT28xxATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	ULONG R;
	CHAR TxPower = 0;
	UCHAR Bbp94 = 0;
	BOOLEAN bPowerReduce = FALSE;

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif /* RALINK_QA */
	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

	if (pAd->ate.Channel <= 14)
	{
		if (TxPower > 31)
		{
			/* R3, R4 can't large than 31 (0x24), 31 ~ 36 used by BBP 94 */
			R = 31;
			if (TxPower <= 36)
				Bbp94 = BBPR94_DEFAULT + (UCHAR)(TxPower - 31);		
		}
		else if (TxPower < 0)
		{
			/* R3, R4 can't less than 0, -1 ~ -6 used by BBP 94 */
			R = 0;
			if (TxPower >= -6)
				Bbp94 = BBPR94_DEFAULT + TxPower;
		}
		else
		{  
			/* 0 ~ 31 */
			R = (ULONG) TxPower;
			Bbp94 = BBPR94_DEFAULT;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%ld, BBP_R94=%d)\n", __FUNCTION__, TxPower, R, Bbp94));
	}
	else /* 5.5 GHz */
	{
		if (TxPower > 15)
		{
			/* R3, R4 can't large than 15 (0x0F) */
			R = 15;
		}
		else if (TxPower < 0)
		{
			/* R3, R4 can't less than 0 */
			/* -1 ~ -7 */
			ASSERT((TxPower >= -7));
			R = (ULONG)(TxPower + 7);
			bPowerReduce = TRUE;
		}
		else
		{  
			/* 0 ~ 15 */
			R = (ULONG) TxPower;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%lu)\n", __FUNCTION__, TxPower, R));
	}

	if (pAd->ate.Channel <= 14)
	{
		if (index == 0)
		{
			/* shift TX power control to correct RF(R3) register bit position */
			R = R << 9;		
			R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
			pAd->LatchRfRegs.R3 = R;
		}
		else
		{
			/* shift TX power control to correct RF(R4) register bit position */
			R = R << 6;		
			R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
			pAd->LatchRfRegs.R4 = R;
		}
	}
	else /* 5.5GHz */
	{
		if (bPowerReduce == FALSE)
		{
			if (index == 0)
			{
				/* shift TX power control to correct RF(R3) register bit position */
				R = (R << 10) | (1 << 9);		
				R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
				pAd->LatchRfRegs.R3 = R;
			}
			else
			{
				/* shift TX power control to correct RF(R4) register bit position */
				R = (R << 7) | (1 << 6);		
				R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
				pAd->LatchRfRegs.R4 = R;
			}
		}
		else
		{
			if (index == 0)
			{
				/* shift TX power control to correct RF(R3) register bit position */
				R = (R << 10);		
				R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);

				/* Clear bit 9 of R3 to reduce 7dB. */ 
				pAd->LatchRfRegs.R3 = (R & (~(1 << 9)));
			}
			else
			{
				/* shift TX power control to correct RF(R4) register bit position */
				R = (R << 7);		
				R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);

				/* Clear bit 6 of R4 to reduce 7dB. */ 
				pAd->LatchRfRegs.R4 = (R & (~(1 << 6)));
			}
		}
	}
	RtmpRfIoWrite(pAd);

	return 0;
}
#endif /* defined(RT28xx) || defined(RT2880) */


INT ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	ATE_INFO *pATEInfo = &(pAd->ate);

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
	UCHAR			BbpData = 0;

	/*
		If Channel=14 and Bandwidth=20M and Mode=CCK, set BBP R4 bit5=1
		(Japan Tx filter coefficients)when (TXFRAME or TXCONT).
	*/
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.Channel == 14) && (pAd->ate.TxWI.BW == BW_20))
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
	UCHAR			i;
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
	for (i = 0;i < 6;i++)
		RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);

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
#ifdef DFS_HARDWARE_SUPPORT
	if (pAd->CommonCfg.dfs_func >= HARDWARE_DFS_V1)
		NewRadarDetectionStop(pAd);
#endif /* DFS_HARDWARE_SUPPORT */
#ifdef DFS_SOFTWARE_SUPPORT
	if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1)
	{
		RadarDetectionStop(pAd);
		BbpRadarDetectionStop(pAd);
	}
#endif /* DFS_SOFTWARE_SUPPORT */
#endif /* DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
	{
		/* make sure CarrierDetect wont send CTS */
		CarrierDetectionStop(pAd);
	}
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */



#ifdef APCLI_SUPPORT
	ApCliIfDown(pAd);
#endif /* APCLI_SUPPORT */

	MacTableReset(pAd);

	/* disable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &=0xe;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	/* disable piggyback */
	RTMPSetPiggyBack(pAd, FALSE);

	ATEDisableAsicProtect(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		AsicDisableSync(pAd);
#ifdef LED_CONTROL_SUPPORT
		/* set LED */
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
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
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
	}
#endif /* GREENAP_SUPPORT */

}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID RTMPStationStop(
    IN  PRTMP_ADAPTER   pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStop\n"));

	/* For rx statistics, we need to keep pAd->Mlme.PeriodicTimer running. */

    DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStop\n"));
}


VOID RTMPStationStart(
    IN  PRTMP_ADAPTER   pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStart\n"));

#ifdef RTMP_MAC_PCI
	pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

	/* We did not cancel pAd->Mlme.PeriodicTimer when entering ATE mode. */

#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStart\n"));
}
#endif /* CONFIG_STA_SUPPORT */


static NDIS_STATUS ATESTART(
	IN PRTMP_ADAPTER pAd)
{
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
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined (RT5350)
	UINT32			bbp_index=0;
	UCHAR			RestoreRfICType=pAd->RfIcType;
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined (RT5350) */
	UCHAR			BbpData = 0;

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

	atemode = pAd->ate.Mode;
	pAd->ate.Mode = ATE_START;

    if (atemode == ATE_STOP)
	{
		/* DUT just enters ATE mode from normal mode. */
		/* Only at this moment, we need to switch back to the channel of normal mode. */
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		/* empty function */
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
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
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
		BbpHardReset(pAd);

		/* Restore All BBP Value */
		for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

		pAd->RfIcType=RestoreRfICType;
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */

		if (pAd->ate.TxMethod == TX_METHOD_1)
		{
			/* No Carrier Test set BBP R22 bit6=0, bit[5~0]=0x0 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF80; /* clear bit6, bit[5~0] */	
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

			BbpSoftReset(pAd);
			RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, Default_TX_PIN_CFG);
		}
		else
		{
			/* No Carrier Test set BBP R22 bit7=0, bit6=0, bit[5~0]=0x0 */
			ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);
		}
	}
	else if (atemode == ATE_TXCARRSUPP)
	{
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
		BbpHardReset(pAd);

		/* Restore All BBP Value */
		for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

		pAd->RfIcType=RestoreRfICType;			
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */

		/* No Cont. TX set BBP R22 bit7=0 */
		ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

		/* No Carrier Suppression set BBP R24 bit0=0 */
		ATE_BBP_CTS_TX_SIN_WAVE_DISABLE(pAd, BBP_R24, &BbpData);

		if (pAd->ate.TxMethod == TX_METHOD_1)
		{
			BbpSoftReset(pAd);
			RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, Default_TX_PIN_CFG);
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
#if defined(RT2883) || defined(RT3352) || defined(RT5350)
			if (IS_RT2883(pAd) || IS_RT3352(pAd) || IS_RT5350(pAd))
			{
				BbpHardReset(pAd);

				/* Restore All BBP Value */
				for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

				pAd->RfIcType=RestoreRfICType;
			}
#endif /* defined(RT2883) || defined(RT3352) || defined(RT5350) */

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

			if (pAd->ate.TxMethod == TX_METHOD_1)
			{
				BbpSoftReset(pAd);
				RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, Default_TX_PIN_CFG);
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
	pAd->ate.LastSNR0 = 0;
	pAd->ate.LastSNR1 = 0;
#ifdef DOT11N_SS3_SUPPORT
	pAd->ate.LastSNR2 = 0;
#endif /* DOT11N_SS3_SUPPORT */
	pAd->ate.LastRssi0 = 0;
	pAd->ate.LastRssi1 = 0;
	pAd->ate.LastRssi2 = 0;
	pAd->ate.AvgRssi0 = 0;
	pAd->ate.AvgRssi1 = 0;
	pAd->ate.AvgRssi2 = 0;
	pAd->ate.AvgRssi0X8 = 0;
	pAd->ate.AvgRssi1X8 = 0;
	pAd->ate.AvgRssi2X8 = 0;
	pAd->ate.NumOfAvgRssiSample = 0;

#ifdef RALINK_QA
	/* Tx frame */
	pAd->ate.bQATxStart = FALSE;
	pAd->ate.bQARxStart = FALSE;
	pAd->ate.seq = 0; 

	/* counters */
	pAd->ate.U2M = 0;
	pAd->ate.OtherData = 0;
	pAd->ate.Beacon = 0;
	pAd->ate.OtherCount = 0;
	pAd->ate.TxAc0 = 0;
	pAd->ate.TxAc1 = 0;
	pAd->ate.TxAc2 = 0;
	pAd->ate.TxAc3 = 0;
	pAd->ate.TxHCCA = 0;
	pAd->ate.TxMgmt = 0;
	pAd->ate.RSSI0 = 0;
	pAd->ate.RSSI1 = 0;
	pAd->ate.RSSI2 = 0;
	pAd->ate.SNR0 = 0;
	pAd->ate.SNR1 = 0;
#ifdef DOT11N_SS3_SUPPORT
	pAd->ate.SNR2 = 0;
#endif /* DOT11N_SS3_SUPPORT */
	pAd->ate.IPG = 200;

	/* control */
	pAd->ate.TxDoneCount = 0;
	/* TxStatus : 0 --> task is idle, 1 --> task is running */
	pAd->ate.TxStatus = 0;
#endif /* RALINK_QA */

#ifndef RT3883
	/* Soft reset BBP. */
	BbpSoftReset(pAd);
#endif /* !RT3883 */

#ifdef CONFIG_AP_SUPPORT 
#ifdef RTMP_MAC_PCI
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#endif /* RTMP_MAC_PCI */

	/* Set IPG 200 by default. */
	Set_ATE_IPG_Proc(pAd, "200"); 

   	/* pAd->ate.Mode must HAVE been ATE_START prior to call ATEAPStop(pAd) */
	/* We must disable DFS and Radar Detection, or 8051 will modify BBP registers. */
	ATEAPStop(pAd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT 
	/* LinkDown() has "AsicDisableSync();" and "RTMP_BBP_IO_R/W8_BY_REG_ID();" inside. */
	AsicDisableSync(pAd);
#ifdef RTMP_MAC_PCI
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#endif /* RTMP_MAC_PCI */
	/* 
		If we skip "LinkDown()", we should disable protection
		to prevent from sending out RTS or CTS-to-self.
	*/
	ATEDisableAsicProtect(pAd);
	RTMPStationStop(pAd);
#endif /* CONFIG_STA_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);	
#endif /* LED_CONTROL_SUPPORT */

#ifdef RTMP_MAC_PCI
	/* Disable Tx */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);


#if defined (RT3883) || defined (RT3352) || defined (RT5350)
	if (IS_RT3883(pAd) || IS_RT3352(pAd) || IS_RT5350(pAd))
	{
		int i;
		UCHAR RFValue = 0, RFValue2 = 0;
		USHORT FreqOffset = 0;

		/* Set RF offset  RF_R17=RF_R23 (RT30xx) */
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, FreqOffset);
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);
		RFValue2 = (FreqOffset & 0x80) | (FreqOffset & 0x7f);
 
		if (RFValue2 > RFValue)
		{
			for (i = 1; i <= (RFValue2 - RFValue); i++)
			{
				RtmpOsMsDelay(1);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue + i));
			}	
		}	
		else
		{
			for (i = 1; i <= (RFValue - RFValue2); i++)
			{
				RtmpOsMsDelay(1);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue - i));
			}
		}
	}
#endif /* defined (RT3883) || defined (RT3352) || defined (RT5350) */
#endif /* RTMP_MAC_PCI */


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS ATESTOP(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0, ring_index=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
#ifdef RT_BIG_ENDIAN
	PRXD_STRUC				pDestRxD;
	RXD_STRUC				RxD;
#endif /* RT_BIG_ENDIAN */
	PRXD_STRUC		pRxD = NULL;
#endif /* RTMP_MAC_PCI */
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
	UINT32			bbp_index=0;
	UCHAR			RestoreRfICType=pAd->RfIcType;
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */ 
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
	BbpHardReset(pAd);

	/* Supposed that we have had a record in restore_BBP[] */
	/* restore all BBP value */
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

	ASSERT(RestoreRfICType != 0);
	pAd->RfIcType=RestoreRfICType;
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */ 

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
#ifndef RTMP_RBUS_SUPPORT
	pAd->ate.bFWLoading = TRUE;

	Status = NICLoadFirmware(pAd);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		return Status;
	}
#endif /* RTMP_RBUS_SUPPORT */
	pAd->ate.Mode = ATE_STOP;

	/*
		Even the firmware has been loaded, 
		we still could use ATE_BBP_IO_READ8_BY_REG_ID(). 
		But this is not suggested.
	*/
	BbpSoftReset(pAd);

	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	
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

	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);

	/* empty function */
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);		

	/* clear garbage interrupts */
	RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, 0xffffffff);
	/* Enable Interrupt */
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */


	/* restore RX_FILTR_CFG */
#ifdef CONFIG_AP_SUPPORT 
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

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXCARR(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
	UINT32			bbp_index=0;	 
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pAd->ate.Mode = ATE_TXCARR;

#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
	/* RT35xx ATE will reuse this code segment. */
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		restore_BBP[bbp_index]=0;

	/* Record All BBP Value */
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */


	/* QA has done the following steps if it is used. */
	if (pAd->ate.bQATxStart == FALSE) 
	{
	if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)))
		BbpSoftReset(pAd);/* Soft reset BBP. */

		if (pAd->ate.TxMethod == TX_METHOD_1)
		{
			/* store the original value of RA_TX_PIN_CFG */
			RTMP_IO_READ32(pAd, RA_TX_PIN_CFG, &Default_TX_PIN_CFG);

			/* give RA_TX_PIN_CFG(0x1328) a proper value. */
			if (pAd->ate.Channel <= 14)
			{
				/* G band */
				MacData = TXCONT_TX_PIN_CFG_G;
				RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, MacData);
			}
			else
			{
				/* A band */
				MacData = TXCONT_TX_PIN_CFG_A;
				RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, MacData);
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
#if defined(RT2883) || defined(RT3352) || defined(RT5350)
	UINT32			bbp_index=0;
#endif /* defined(RT2883) || defined(RT3352) || defined(RT5350) */
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	if (pAd->ate.bQATxStart == TRUE)
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
		if (pAd->ate.TxMethod == TX_METHOD_1)
		{
			/* store the original value of RA_TX_PIN_CFG */
			RTMP_IO_READ32(pAd, RA_TX_PIN_CFG, &Default_TX_PIN_CFG);
		}
	}

#if defined(RT2883) || defined(RT3352) || defined(RT5350)
	if (IS_RT2883(pAd) || IS_RT3352(pAd) || IS_RT5350(pAd))
	{
		/* Zero All BBP Value */
		for(bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			restore_BBP[bbp_index]=0;

		/* Record All BBP Value */
		for(bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
	}
#endif /* defined(RT2883) || defined(RT3352) || defined(RT5350) */


	/* Step 1: send 50 packets first. */
	pAd->ate.Mode = ATE_TXCONT;
	pAd->ate.TxCount = 50;

	if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)))
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
	pAd->ate.TxDoneCount = 0;
	
	/* Only needed if we have to send some normal frames. */
	SetJapanFilter(pAd);

#ifdef RTMP_MAC_PCI
	for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pAd->ate.TxCount); ring_index++)
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
	if (pAd->ate.bQATxStart == TRUE)
	{
		pAd->ate.TxStatus = 1;
	}
#endif /* RALINK_QA */

#ifdef RTMP_MAC_PCI
	/* kick Tx Ring */
	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

	RtmpOsMsDelay(5);
#endif /* RTMP_MAC_PCI */


	if (pAd->ate.TxMethod == TX_METHOD_1)
	{
		/* give RA_TX_PIN_CFG(0x1328) a proper value. */
		if (pAd->ate.Channel <= 14)
		{
			/* G band */
			MacData = TXCONT_TX_PIN_CFG_G;
			RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, MacData);
		}
		else
		{
			/* A band */
			MacData = TXCONT_TX_PIN_CFG_A;
			RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, MacData);
		}

		/* Cont. TX set BBP R22 bit7=1 */
		ATE_BBP_START_CTS_TX_MODE(pAd, BBP_R22, &BbpData);
	}
	else
	{
		/* Step 2: send more 50 packets then start Continuous Tx Mode. */
		/* Abort Tx, RX DMA. */
		RtmpDmaEnable(pAd, 0);

		/* Cont. TX set BBP R22 bit7=1 */
		ATE_BBP_START_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

		pAd->ate.TxCount = 50;
#ifdef RTMP_MAC_PCI
		{
			RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);					
		}
#endif /* RTMP_MAC_PCI */

		pAd->ate.TxDoneCount = 0;
		SetJapanFilter(pAd);

#ifdef RTMP_MAC_PCI
		for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pAd->ate.TxCount); ring_index++)
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
		if (pAd->ate.bQATxStart == TRUE)
		{
			pAd->ate.TxStatus = 1;
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
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
	UINT32			bbp_index=0;	 
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pAd->ate.Mode = ATE_TXCARRSUPP;

#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350)
        /* RT35xx ATE will reuse this code segment. */
        for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
                restore_BBP[bbp_index]=0;

        /* Record All BBP Value */
        for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
                ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
#endif /* defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT2883) || defined(RT5350) */


	/* QA has done the following steps if it is used. */
	if (pAd->ate.bQATxStart == FALSE) 
	{
#ifndef RT3883
		/* RT3883 does not need BbpSoftReset() */
		/* Soft reset BBP. */
		BbpSoftReset(pAd);
#endif /* !RT3883 */

		if (pAd->ate.TxMethod == TX_METHOD_1)
		{
			/* store the original value of RA_TX_PIN_CFG */
			RTMP_IO_READ32(pAd, RA_TX_PIN_CFG, &Default_TX_PIN_CFG);

			/* give RA_TX_PIN_CFG(0x1328) a proper value. */
			if (pAd->ate.Channel <= 14)
			{
				/* G band */
				MacData = TXCONT_TX_PIN_CFG_G;
				RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, MacData);
			}
			else
			{
				/* A band */
				MacData = TXCONT_TX_PIN_CFG_A;
				RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, MacData);
			}

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
	
	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s(Count=%d)\n", __FUNCTION__, pAd->ate.TxCount));
	pAd->ate.Mode |= ATE_TXFRAME;

	if (pAd->ate.bQATxStart == FALSE)  
	{
		/* set IPG to sync tx power with QA tools */
		/* default value of IPG is 200 */
		snprintf(IPGStr, sizeof(IPGStr), "%u", pAd->ate.IPG);
		DBGPRINT(RT_DEBUG_TRACE, ("IPGstr=%s\n", IPGStr));
		Set_ATE_IPG_Proc(pAd, IPGStr);
	}

#ifdef RTMP_MAC_PCI
	/* Default value in BBP R22 is 0x0. */
	ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);

#if !defined (RT3883) && !defined (RT3352) && !defined (RT5350)
	/* Soft reset BBP. */
	BbpSoftReset(pAd);
#endif /* !defined (RT3883) && !defined (RT3352) && !defined (RT5350) */

#ifdef RT305x
	/* for RTxx50 single image */
	if (IS_RT3050(pAd) || IS_RT3052(pAd) || IS_RT3350(pAd))
	{
		/* Soft reset BBP. */
		BbpSoftReset(pAd);
	}
#endif /* RT305x */

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

	pAd->ate.TxDoneCount = 0;

	SetJapanFilter(pAd);
	
	for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pAd->ate.TxCount); ring_index++)
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
	if (pAd->ate.bQARxStart == FALSE)  
	{
#ifdef TXBF_SUPPORT
		/* Enable Rx if Sending Sounding. Otherwise Disable */
		if (pAd->ate.txSoundingMode != 0)
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

	if (pAd->ate.bQATxStart == TRUE)  
	{
		pAd->ate.TxStatus = 1;
	}
#else
#ifdef TXBF_SUPPORT
	/* Enable Rx if Sending Sounding. Otherwise Disable */
	if (pAd->ate.txSoundingMode != 0)
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


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS RXFRAME(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	/* Disable Rx of MAC block */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	/* Default value in BBP R22 is 0x0. */
	ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);

#if defined (RT3883) || defined (RT3352) || defined (RT5350) 
	if (pAd->ate.TxWI.BW == BW_20)
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xC0); 
	}
	else
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0x00);
	}
#endif /* defined (RT3883) || defined (RT3352) || defined (RT5350) */

	/* Clear bit4 to stop continuous Tx production test. */
	ATE_MAC_TX_CTS_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	pAd->ate.Mode |= ATE_RXFRAME;


	/* Disable Tx of MAC block. */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);


	/* Enable Rx of MAC block. */
	ATE_MAC_RX_ENABLE(pAd, MAC_SYS_CTRL, &MacData);



	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}

#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
static NDIS_STATUS ANTDIVCBA(
	IN PRTMP_ADAPTER pAd);
#endif /* HW_ANTENNA_DIVERSITY_SUPPORT */ 


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
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pAd->ate.Channel);
		RtmpOsMsDelay(5);

		Status = TXCARR(pAd);
	}
	else if (!strcmp(arg, "TXCARS"))
	{
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pAd->ate.Channel);
		RtmpOsMsDelay(5);

		Status = TXCARS(pAd);
	}
	else if (!strcmp(arg, "TXCONT"))	
	{
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pAd->ate.Channel);
		RtmpOsMsDelay(5);

		Status = TXCONT(pAd);
	}
	else if (!strcmp(arg, "TXFRAME")) 
	{
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pAd->ate.Channel);
		RtmpOsMsDelay(5);

		Status = TXFRAME(pAd);
	}
	else if (!strcmp(arg, "RXFRAME")) 
	{
		ATEAsicSwitchChannel(pAd);
		/* AsicLockChannel() is empty function so far in fact */
		AsicLockChannel(pAd, pAd->ate.Channel);
		RTMPusecDelay(5);

		Status = RXFRAME(pAd);
	}
#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
	else if (!strcmp(arg, "ANTDIVCBA")) 
	{
		Status = ANTDIVCBA(pAd);
	}
#endif /* HW_ANTENNA_DIVERSITY_SUPPORT */
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
		DBGPRINT(RT_DEBUG_TRACE, ("ATE : Invalid arg !\n"));
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
	/* Handle ATEACTIVE and ATEPASSIVE commands as a special case */
	if (!strcmp(arg, "ATEACTIVE"))
	{
		pAd->ate.PassiveMode = FALSE;
		return TRUE;
	}

	if (!strcmp(arg, "ATEPASSIVE"))
	{
		pAd->ate.PassiveMode = TRUE;
		return TRUE;
	}

	/* Disallow all other ATE commands in passive mode */
	if (pAd->ate.PassiveMode)
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
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Proc Failed\n"));
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
	PSTRING				value;
	INT					i;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */	
	if (strlen(arg) != 17)  
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pAd->ate.Addr1[i++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pAd->ate.Addr3[i++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (i != MAC_ADDR_LEN)
	{
		return FALSE;  
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pAd->ate.Addr1[0], pAd->ate.Addr1[1], pAd->ate.Addr1[2], pAd->ate.Addr1[3], pAd->ate.Addr1[4], pAd->ate.Addr1[5]));

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pAd->ate.Addr3[0], pAd->ate.Addr3[1], pAd->ate.Addr3[2], pAd->ate.Addr3[3], pAd->ate.Addr3[4], pAd->ate.Addr3[5]));
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
	PSTRING				value;
	INT					i;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */	
	if (strlen(arg) != 17)  
		return FALSE;

	for (i=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pAd->ate.Addr3[i++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pAd->ate.Addr2[i++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (i != MAC_ADDR_LEN)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pAd->ate.Addr3[0], pAd->ate.Addr3[1], pAd->ate.Addr3[2], pAd->ate.Addr3[3], pAd->ate.Addr3[4], pAd->ate.Addr3[5]));
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", 
		pAd->ate.Addr2[0], pAd->ate.Addr2[1], pAd->ate.Addr2[2], pAd->ate.Addr2[3], pAd->ate.Addr2[4], pAd->ate.Addr2[5]));
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
	PSTRING				value;
	INT					i;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */	
	if (strlen(arg) != 17)  
		return FALSE;

	for (i=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pAd->ate.Addr2[i++], 1);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pAd->ate.Addr1[i++], 1);
#endif /* CONFIG_STA_SUPPORT */
	}

	/* sanity check */
	if (i != MAC_ADDR_LEN)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n",	
		pAd->ate.Addr2[0], pAd->ate.Addr2[1], pAd->ate.Addr2[2], pAd->ate.Addr2[3], pAd->ate.Addr2[4], pAd->ate.Addr2[5]));

#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n",	
		pAd->ate.Addr1[0], pAd->ate.Addr1[1], pAd->ate.Addr1[2], pAd->ate.Addr1[3], pAd->ate.Addr1[4], pAd->ate.Addr1[5]));
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
	UCHAR channel;
	

	channel = simple_strtol(arg, 0, 10);

	/* to allow A band channel : ((channel < 1) || (channel > 14)) */
	if ((channel < 1) || (channel > 216))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_CHANNEL_Proc::Out of range, it should be in range of 1~14.\n"));
		return FALSE;
	}

	pAd->ate.Channel = channel;


	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_CHANNEL_Proc (ATE Channel = %d)\n", pAd->ate.Channel));
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
	int index;
	int value;

	/* Get channel parameter */
	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>216)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_INIT_CHAN_Proc::Channel out of range\n"));
		return FALSE;
	}

	if (value != 0)
		pAd->ate.Channel = value;

	for (index=0; index<MAX_NUM_OF_CHANNELS; index++)
	{
		if (pAd->ate.Channel == pAd->TxPower[index].Channel)
		{
			pAd->ate.TxPower0 = pAd->TxPower[index].Power;
			pAd->ate.TxPower1 = pAd->TxPower[index].Power2;
#ifdef DOT11N_SS3_SUPPORT
			if (IS_RT2883(pAd) || IS_RT3593(pAd) || IS_RT3883(pAd))
				pAd->ate.TxPower2 = pAd->TxPower[index].Power3;
#endif /* DOT11N_SS3_SUPPORT */
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_INIT_CHAN_Proc::Channel not found\n"));
		return FALSE;
	}

	/* Force non-QATool mode */
	pAd->ate.bQATxStart = pAd->ate.bQARxStart = FALSE;

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
	CHAR TxPower;
	INT  index, maximun_index;
	ATE_INFO *pATEInfo;

	pATEInfo = &(pAd->ate);
	TxPower = simple_strtol(arg, 0, 10);
	index = Antenna;
	maximun_index = pAd->Antenna.field.TxPath - 1;

	if ((index < 0) || (index > maximun_index))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("No such antenna! The range is 0~%d.\n", maximun_index));
		return FALSE;
	}

	if (pATEInfo->Channel <= 14) /* 2.4 GHz */
	{
		if (!IS_RT3390(pAd))
		{
			if ((TxPower > 31) || (TxPower < 0))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER%d_Proc::Out of range! (Value=%d)\n", index, TxPower));
				DBGPRINT(RT_DEBUG_ERROR, ("TxPower range is 0~31 in G band.\n"));
				return FALSE;
			}
		}
	}
	else /* 5.5 GHz */
	{
		/* 
			New TxPower range is 0~31(5 bits) in A band;
			RT28xx and RT2883 TxPower range is -7~15 in A band.
		*/
		if ((TxPower > (pATEInfo->MaxTxPowerBandA)) || (TxPower < (pATEInfo->MinTxPowerBandA)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER%d_Proc::Out of range! (Value=%d)\n", index, TxPower));
			DBGPRINT(RT_DEBUG_ERROR, ("TxPower range is %d~%d in A band.\n", pATEInfo->MinTxPowerBandA, pATEInfo->MaxTxPowerBandA));
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
	CHAR value;
	INT maximun_index = pAd->Antenna.field.TxPath;
	
	value = simple_strtol(arg, 0, 10);

	if ((value > maximun_index) || (value < 0))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_Antenna_Proc::Out of range (Value=%d)\n", value));
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_Antenna_Proc::The range is 0~%d\n", maximun_index));

		return FALSE;
	}

	pAd->ate.TxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_Antenna_Proc (Antenna = %d)\n", pAd->ate.TxAntennaSel));
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
	CHAR value;
	INT maximun_index = pAd->Antenna.field.RxPath;
	
	value = simple_strtol(arg, 0, 10);

	if ((value > maximun_index) || (value < 0))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_RX_Antenna_Proc::Out of range (Value=%d)\n", value));
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_RX_Antenna_Proc::The range is 0~%d\n", maximun_index));

		return FALSE;
	}

	pAd->ate.RxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_Antenna_Proc (Antenna = %d)\n", pAd->ate.RxAntennaSel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_Antenna_Proc Success\n"));

	/* calibration power unbalance issues */
	ATEAsicSwitchChannel(pAd);

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
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
	UCHAR PABias = 0;
	UCHAR RFValue;
	
	PABias = simple_strtol(arg, 0, 10);

	if (PABias >= 16)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_PA_Bias_Proc::Out of range, it should be in range of 0~15.\n"));
		return FALSE;
	}

	pAd->ate.PABias = PABias;

	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R19, (PUCHAR)&RFValue);
	RFValue = (((RFValue & 0x0F) | (pAd->ate.PABias << 4)));
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R19, (UCHAR)RFValue);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_PA_Bias_Proc (PABias = %d)\n", pAd->ate.PABias));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_PA_Bias_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
#endif /* RT3350 */


/* 
==========================================================================
    Description:
        Set ATE RF frequence offset
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_FREQOFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR RFFreqOffset = 0;
	ULONG R4 = 0;
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR RFValue = 0;
#endif /* RTMP_RF_RW_SUPPORT */

	
	RFFreqOffset = simple_strtol(arg, 0, 10);

#ifdef RTMP_RF_RW_SUPPORT
	/* RT35xx ATE will reuse this code segment. */
	if (RFFreqOffset >= 96)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_FREQOFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}
#else
	if (RFFreqOffset >= 64)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_FREQOFFSET_Proc::Out of range(0 ~ 63).\n"));
		return FALSE;
	}
#endif /* RTMP_RF_RW_SUPPORT */

	pAd->ate.RFFreqOffset = RFFreqOffset;

#ifdef RTMP_RF_RW_SUPPORT
	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
		RFValue = ((RFValue & 0x80) | pAd->ate.RFFreqOffset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);
	}
	else
#ifdef RT305x
	if (IS_RT305x(pAd))
	{
#if defined (RT3352) || defined (RT5350)
		if (IS_RT3352(pAd) || IS_RT5350(pAd))
		{
			UCHAR offset = 0;
			UCHAR RFValue2 = 0;
			
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);

			if (IS_RT3352(pAd))
			{
				RFValue2 = (RFValue & 0x80) | pAd->ate.RFFreqOffset;
			}
			else
			{
				/* RT5350 */
				RFValue2 = pAd->ate.RFFreqOffset & 0x7F;/* bit7 = 0 */
			}

			if (RFValue2 > RFValue)
			{
				for (offset = 1; offset <= (RFValue2 - RFValue); offset++)
				{
					RtmpOsMsDelay(1);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue + offset));
				}
			}	
			else
			{
				for (offset = 1; offset <= (RFValue - RFValue2); offset++)
				{
					RtmpOsMsDelay(1);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue - offset));
				}
			}
		}
		else
#endif /* RT3352 || RT5350 */
		{
			/* for RT305x and RT3350 */
			UCHAR step = 0;
			UCHAR RFValue2 = 0;

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
			RFValue2 = (RFValue & 0x80) | pAd->ate.RFFreqOffset;

			if (RFValue2 > RFValue)
			{
				for (step = 1; step <= (RFValue2 - RFValue); step++)
				{

					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)(RFValue + step));
					RtmpOsMsDelay(10);
				}
			}	
			else
			{
				for (step = 1; step <= (RFValue - RFValue2); step++)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)(RFValue - step));
					RtmpOsMsDelay(10);
				}
			}
		}
	}
	else
#endif /* RT305x */
#ifdef RT3883
	if (IS_RT3883(pAd))
	{
		UCHAR diff = 0;
		UCHAR RFValue2 = 0;

		/* Set RF offset  RF_R17=RF_R23 (RT30xx) */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);
		RFValue2 = (RFValue & 0x80) | pAd->ate.RFFreqOffset;
		if (RFValue2 > RFValue)
		{
			for (diff = 1; diff <= (RFValue2 - RFValue); diff++)
			{
				RtmpOsMsDelay(1);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue + diff));
			}	
		}	
		else
		{
			for (diff = 1; diff <= (RFValue - RFValue2); diff++)
			{
				RtmpOsMsDelay(1);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue - diff));
			}
		}
	}
	else
#endif /* RT3883 */
#endif /* RTMP_RF_RW_SUPPORT */
	{
		/* RT28xx */
		/* shift TX power control to correct RF register bit position */
		R4 = pAd->ate.RFFreqOffset << 15;		
		R4 |= (pAd->LatchRfRegs.R4 & ((~0x001f8000)));
		pAd->LatchRfRegs.R4 = R4;
		
		RtmpRfIoWrite(pAd);
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_FREQOFFSET_Proc (RFFreqOffset = %d)\n", pAd->ate.RFFreqOffset));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_FREQOFFSET_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
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
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if ((BBPCurrentBW == 0)
		)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Bandwidth switch to 20!!\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
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


		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

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
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
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

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
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

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


#ifdef RT3883
/* 
==========================================================================
    Description:
        Set RT3883 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT3883_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Bandwidth switch to 20!!\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=20;G band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_9 */
				/* TX_PWR_CFG_0_EXT ~ TX_PWR_CFG_4_EXT */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx20MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex== 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx20MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 7)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, pAd->Tx20MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 8)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, pAd->Tx20MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 9)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, pAd->Tx20MPwrCfgGBand[powerIndex]);
				}
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgGBand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx20MPwrCfgGBand[powerIndex] & 0xf0f0f0f0) >> 4);
				}
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_9 */
				/* TX_PWR_CFG_0_EXT ~ TX_PWR_CFG_4_EXT */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx20MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 6)
 				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx20MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 7)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, pAd->Tx20MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 8)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, pAd->Tx20MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 9)
 				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, pAd->Tx20MPwrCfgABand[powerIndex]);
				}
				else
 				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx20MPwrCfgABand[powerIndex] & 0xf0f0f0f0) >> 4);
 				}
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

		/* set BBP R4 = 0x40 for BW = 20 MHz */
 		value = 0x40;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x12 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x0A */
		value = 0x0A;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x10 */
		value = 0x10;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

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
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=40;G band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_9 */
				/* TX_PWR_CFG_0_EXT ~ TX_PWR_CFG_4_EXT */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx40MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx40MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 7)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, pAd->Tx40MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 8)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, pAd->Tx40MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 9)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, pAd->Tx40MPwrCfgGBand[powerIndex]);
				}
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgGBand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx40MPwrCfgGBand[powerIndex] & 0xf0f0f0f0) >> 4);
				}
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_9 */
				/* TX_PWR_CFG_0_EXT ~ TX_PWR_CFG_4_EXT */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx40MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx40MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 7)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, pAd->Tx40MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 8)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, pAd->Tx40MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 9)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, pAd->Tx40MPwrCfgABand[powerIndex]);
				}
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx40MPwrCfgABand[powerIndex] & 0xf0f0f0f0) >> 4);
				}
				RtmpOsMsDelay(5);				
			}		

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
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

		/* set BBP R4 = 0x50 for BW = 40 MHz */
 		value = 0x50;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x12 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x0A */
		value = 0x0A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x10 */
		value = 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);		

	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}
#endif /* RT3883 */


#ifdef RT305x
#if defined(RT3050) || defined(RT3052)
/* 
==========================================================================
    Description:
        Set RT3050/RT3052 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT305x_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Bandwidth switch to 20!!\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=20;G band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
 				RtmpOsMsDelay(5);				
 			}
		}

		/* BW = 20 MHz */
		/* Set BBP R4 bit[4:3]=0:0 */
 		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
 		value &= (~0x18);
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		/* Set BBP R66=0x3C */
		value = 0x3C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);

		/* set BW */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R24, (PUCHAR)&value);
		value &= 0xDF;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);

		/* Rx filter */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x0F);

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
	    value = 0x11;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

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
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
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

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
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

		/* set BW */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R24, (PUCHAR)&value);
		value &= 0xDF;
		value |= 0x20;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);

		/* Rx filter */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x2F);

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
	    value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}
#endif /* defined(RT3050) || defined(RT3052) */


#ifdef RT3350
/* 
==========================================================================
    Description:
        Set RT3350 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT3350_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Bandwidth switch to 20!!\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
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

		/* set RF_R24 */
		if(pAd->ate.TxWI.PHYMODE == MODE_CCK)
			value = 0x1F;
		else
			value = 0x18;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);

		/* Rx filter */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x48);

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
	    value = 0x11;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

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
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}		

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
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

		/* set RF_R24 */
		if(pAd->ate.TxWI.PHYMODE == MODE_CCK)
			value = 0x3F;
		else
			value = 0x28;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);

		/* Rx filter */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x68);

		/* BW = 40 MHz */
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
	    value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}	
#endif /* RT3350 */


#ifdef RT3352
/* 
==========================================================================
    Description:
        Set RT3352 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT3352_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Bandwidth switch to 20!!\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=20;G band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
 				RtmpOsMsDelay(5);				
 			}
		}

		/* set BW = 20 MHz */
 		value = 0x40;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		value = 0; /* RF_R24 is reserved bits */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
		/* Rx filter */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);

		/* BW = 20 MHz */
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x16 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x08 */
		value = 0x0A;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x11 */
		value = 0x10;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
		
		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

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
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=40;G band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}		

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
			{
    			value = 0x28;
    			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, value);
			}
		}

 		value = 0x50;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		if (IS_RT3352(pAd) || IS_RT5350(pAd))
		{
			value = 0x00;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);
		}
		else
		{
			/* set BW */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R24, (PUCHAR)&value);
			value &= 0xDF;
			value |= 0x20;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);

			/* Rx filter */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x2F);
		}

		/* BW = 40 MHz */
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x16 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x08 */
		value = 0x0A;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x11 */
		value = 0x10;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}
#endif /* RT3352 */


#endif /* RT305x */




#ifdef RT2883
/* 
==========================================================================
    Description:
        Set RT2883 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT2883_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Bandwidth switch to 20!!\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=20;G band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_6 */
				/* TX_PWR_CFG_0_EXT ~ TX_PWR_CFG_4_EXT */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx20MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx20MPwrCfgGBand[powerIndex]);
				}
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgGBand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx20MPwrCfgGBand[powerIndex] & 0xf0f0f0f0) >> 4);
				}
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_6 */
				/* TX_PWR_CFG_0_EXT ~ TX_PWR_CFG_4_EXT */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx20MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 6)
 				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx20MPwrCfgABand[powerIndex]);
				}
				else
 				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx20MPwrCfgABand[powerIndex] & 0xf0f0f0f0) >> 4);
 				}
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

		pAd->LatchRfRegs.R4 &= ~0x00200000;
		RtmpRfIoWrite(pAd);

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
	    value = 0x11;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

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
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=40;G band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_6 */
				/* TX_PWR_CFG_0_EXT ~ TX_PWR_CFG_4_EXT */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx40MPwrCfgGBand[powerIndex]);
				}
				else if (powerIndex == 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx40MPwrCfgGBand[powerIndex]);
				}
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgGBand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx40MPwrCfgGBand[powerIndex] & 0xf0f0f0f0) >> 4);
				}
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_6 */
				if (powerIndex == 5)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx40MPwrCfgABand[powerIndex]);
				}
				else if (powerIndex == 6)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx40MPwrCfgABand[powerIndex]);
				}
				else
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT + powerIndex*4, (pAd->Tx40MPwrCfgABand[powerIndex] & 0xf0f0f0f0) >> 4);
				}
				RtmpOsMsDelay(5);				
			}		

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
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

		/* set BW = 40 MHz */
		pAd->LatchRfRegs.R4 |= 0x00200000;
		RtmpRfIoWrite(pAd);

		/* BW = 40 MHz */
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
	    value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}
#endif /* RT2883 */


#if defined(RT28xx) || defined(RT2880)
/* 
==========================================================================
    Description:
        Set RT28xx/RT2880 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT28xx_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Bandwidth switch to 20!!\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=20;G band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
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
		pAd->LatchRfRegs.R4 &= ~0x00200000;
		RtmpRfIoWrite(pAd);

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
	    value = 0x11;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

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
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW=40;G band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}		

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
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

		/* set BW = 40 MHz */
		pAd->LatchRfRegs.R4 |= 0x00200000;
		RtmpRfIoWrite(pAd);

		/* BW = 40 MHz */
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
	    value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}	
#endif /* defined(RT28xx) || defined(RT2880) */


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
	ATE_INFO *pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->Set_BW_Proc != NULL)
		pATEInfo->pChipStruct->Set_BW_Proc(pAd, arg);

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
	pAd->ate.TxLength = simple_strtol(arg, 0, 10);

	if ((pAd->ate.TxLength < 24) || (pAd->ate.TxLength > (MAX_FRAME_SIZE - 34/* == 2312 */)))
	{
		pAd->ate.TxLength = (MAX_FRAME_SIZE - 34/* == 2312 */);
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_LENGTH_Proc::Out of range, it should be in range of 24~%d.\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_LENGTH_Proc (TxLength = %d)\n", pAd->ate.TxLength));
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
	pAd->ate.TxCount = simple_strtol(arg, 0, 10);

#ifdef RTMP_MAC_PCI
	if (pAd->ate.TxCount == 0)
	{
		pAd->ate.TxCount = 0xFFFFFFFF;
	}
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pAd->ate.TxCount));
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
	UCHAR MCS;
	INT result;

	MCS = simple_strtol(arg, 0, 10);
	result = CheckMCSValid(pAd, pAd->ate.TxWI.PHYMODE, MCS);

	if (result != -1)
	{
		pAd->ate.TxWI.MCS = (UCHAR)MCS;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MCS_Proc::Out of range, refer to rate table.\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MCS_Proc (MCS = %d)\n", pAd->ate.TxWI.MCS));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MCS_Proc Success\n"));

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
	UCHAR BbpData = 0;

	pAd->ate.TxWI.PHYMODE = simple_strtol(arg, 0, 10);

	if (pAd->ate.TxWI.PHYMODE > 3)
	{
		pAd->ate.TxWI.PHYMODE = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MODE_Proc::Out of range.\nIt should be in range of 0~3\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("0: CCK, 1: OFDM, 2: HT_MIX, 3: HT_GREEN_FIELD.\n"));
		return FALSE;
	}

	/* Turn on BBP 20MHz mode by request here. */
	if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);
		BbpData &= (~0x18);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpData);
		pAd->ate.TxWI.BW = BW_20;
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MODE_Proc::CCK Only support 20MHZ. Switch to 20MHZ.\n"));
	}

#ifdef RT3350
	if (IS_RT3350(pAd))
	{
		if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
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
			if (pAd->ate.TxWI.BW == BW_40)
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
			if (pAd->ate.TxWI.BW == BW_40)
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

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MODE_Proc (TxMode = %d)\n", pAd->ate.TxWI.PHYMODE));
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
	pAd->ate.TxWI.ShortGI = simple_strtol(arg, 0, 10);

	if (pAd->ate.TxWI.ShortGI > 1)
	{
		pAd->ate.TxWI.ShortGI = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_GI_Proc::Out of range\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_GI_Proc (GI = %d)\n", pAd->ate.TxWI.ShortGI));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_GI_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


INT	Set_ATE_RX_FER_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->ate.bRxFER = simple_strtol(arg, 0, 10);

	if (pAd->ate.bRxFER == 1)
	{
		pAd->ate.RxCntPerSec = 0;
		pAd->ate.RxTotalCnt = 0;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_FER_Proc (bRxFER = %d)\n", pAd->ate.bRxFER));
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
	/* modify by WY for Read RF Reg. error */
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

	DBGPRINT(RT_DEBUG_ERROR, ("===> %s (value=%d)\n\n", __FUNCTION__, value));

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
				DBGPRINT(RT_DEBUG_ERROR, ("%s - Error opening file %s\n", __FUNCTION__, src));
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(srcf, (PSTRING)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: error file length (=%d) in e2p.bin\n",
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
				DBGPRINT(RT_DEBUG_ERROR, ("--> Error %d closing %s\n", -retval, src));
				
			} 
		}

		/* restore */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);		
	}

    DBGPRINT(RT_DEBUG_ERROR, ("<=== %s (ret=%d)\n", __FUNCTION__, ret));

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
	UINT32 value = simple_strtol(arg, 0, 10);

	if (value > 0)
	{
		pAd->ate.bAutoTxAlc = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = TRUE , auto alc enabled!\n"));
	}
	else
	{
		pAd->ate.bAutoTxAlc = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = FALSE , auto alc disabled!\n"));
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
	CHAR value;
	
	value = simple_strtol(arg, 0, 10);

	switch (value)
	{
		case 0:
			/* no BF */
			pAd->ate.TxWI.iTxBF = pAd->ate.TxWI.eTxBF = 0;
			break;
		case 1:
			/* ETxBF */
			pAd->ate.TxWI.eTxBF = 1;
			break;
		case 2:
			/* ITxBF */
			pAd->ate.TxWI.iTxBF = 1;
			break;
		case 3:
			/* Enable TXBF support */
		pAd->ate.bTxBF = TRUE;
			break;
		case 4:
			/* Disable TXBF support */
			pAd->ate.bTxBF = FALSE;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TXBF_Proc: Invalid parameter %d\n", value));
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
	CHAR value;
	
	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>3)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TXSOUNDING_Proc: Invalid parameter %d\n", value));
		return FALSE;
	}	

	pAd->ate.txSoundingMode = value;

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
	int value;
	ITXBF_DIV_PARAMS divParams;
	CHAR initChanArg[] = "0";

	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>2)
		return FALSE;

	/* G band */
	if (value==0 || value==1)
	{
		pAd->ate.Channel = 1;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);

		pAd->ate.Channel = 14;
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
		pAd->ate.Channel = 36;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);

		pAd->ate.Channel = 120;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFDividerCalibration(pAd, 1, 0, NULL);

		pAd->ate.Channel = 165;
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
	int value;
	int i;
	CHAR initChanArg[] = "0";

	value = simple_strtol(arg, 0, 10);

	if (value<0 || value>2)
		return FALSE;

	/* G Band */
	if (value==0 || value==1)
	{
		pAd->ate.Channel = 1;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFLNACalibration(pAd, 1, 0, TRUE);

		pAd->ate.Channel = 14;
		Set_ATE_INIT_CHAN_Proc(pAd, initChanArg);
		ITxBFLNACalibration(pAd, 1, 0, TRUE);
	}

	/* A Band */
	if (value==0 || value==2)
	{
		static UCHAR channels[6] = {36, 64, 100, 128, 132, 165};
		for (i=0; i<6; i++)
		{
			pAd->ate.Channel = channels[i];
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
		pAd->ate.Addr1[val] = 0x11; /* the RA */
		pAd->ate.Addr2[val] = 0x22; /* the TA */
		pAd->ate.Addr3[val] = 0x22;	/* the BSSID */
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
	Set_ATE_TX_FREQOFFSET_Proc(pAd, cmdStr);
	
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
	Set_ATE_TX_FREQOFFSET_Proc(pAd, cmdStr);
	
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
	UCHAR bbpReg;

	bbpReg = simple_strtol(arg, 0, 10);

	/*
		0: no any restriction for BBP writing
		1~255: force to not allow to change this specific BBP register.
		
		Note: 
			BBP_R0 is not write-able, so use 0 as the rest operation shall be safe enough
	*/
	pAd->ate.forceBBPReg = bbpReg;
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_ForceBBP_Proc:(forceBBPReg value=%d)\n", pAd->ate.forceBBPReg));

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
	UCHAR bbpR173;
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
	UINT32           data, value;

	pAd->ate.IPG = simple_strtol(arg, 0, 10);
	value = pAd->ate.IPG;

	RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &data);

	if (value <= 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_IPG_Proc::IPG is disabled(IPG == 0).\n"));
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

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_IPG_Proc (IPG = %u)\n", pAd->ate.IPG));
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
	PSTRING				value;

	value = arg;

	/* only one octet acceptable */	
	if (strlen(value) != 2)  
		return FALSE;

	AtoH(value, &(pAd->ate.Payload), 1);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Payload_Proc (repeated pattern = 0x%2x)\n", pAd->ate.Payload));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Payload_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


INT	Set_ATE_Show_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PSTRING Mode_String = NULL;
	PSTRING TxMode_String = NULL;
	ATE_INFO *pATEInfo = &(pAd->ate);
	
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
#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATE=ANTDIVCBA\n"));
#endif /* HW_ANTENNA_DIVERSITY_SUPPORT */ 
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
#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATEANTDIV, enable hardware antenna diversity.\n"));
#endif /* HW_ANTENNA_DIVERSITY_SUPPORT */
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


#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
static  INT DO_RACFG_CMD_DIV_ANTENNA_CALIBRATION(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_DIV_ANTENNA_CALIBRATION\n"));

	pAd->ate.bQARxStart = TRUE;
	Set_ATE_Proc(pAd, "ANTDIVCBA");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	


static NDIS_STATUS ANTDIVCBA(
	IN PRTMP_ADAPTER pAd)
{
	int i;
	UINT8 BBPValue = 0, RFValue = 0;
	UINT8 rf_r29;

	/* Step 1 */
	/* RF_R29 bit7:6 = 11 */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R29, &RFValue);
	RFValue |= 0xC0;			
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R29, RFValue);

	/* BBP_R47 bit7=1 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue |= 0x80;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
	
	/* BBP_R150 = 0x81 */
	BBPValue = 0x81;			
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);

	/* clear BBP_R154 bit4 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R154, &BBPValue);
	BBPValue &= 0xef; /* clear bit4 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
	/* end Step 1 */

	/* Step 2 */
	rf_r29 = 0x3; /* bit7:6 = 11 */ 
	while ((rf_r29 == 0x3) || (rf_r29 == 0x2))
	{
		/* Step 3 */
		/* main antenna: BBP_R152 bit7=1 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BBPValue);
		BBPValue |= 0x80;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);

		/* BBP_R60 bit6=1 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R60, &BBPValue);
		BBPValue |= 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R60, BBPValue);

		for (i = 0; i < 100; i++)
		{
			/* check if BBP_R60[5:0] < 2 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R60, &BBPValue);
			BBPValue &= 0x3f; /* clear bit7:6 */
			if (BBPValue >= 2)
			{
				rf_r29 = rf_r29 - 1;
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R29, &RFValue);
				RFValue |= (rf_r29 << 6);			
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R29, RFValue);
				break;
			}
			RtmpOsMsDelay(1);
		}

		if (i != 100)
			continue;
			
		/* Step 4 */
		/* main antenna: BBP_R152 bit7=0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BBPValue);
		BBPValue &= 0x7f;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);

		/* BBP_R60 bit6=0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R60, &BBPValue);
		BBPValue &= 0xbf;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R60, BBPValue);
		for (i = 0; i < 100; i++)
		{
			/* check if BBP_R60[5:0] < 2 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R60, &BBPValue);
			BBPValue &= 0x3f; /* clear bit7:6 */
			if (BBPValue >= 2)
			{
				rf_r29 = rf_r29 - 1;
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R29, &RFValue);
				RFValue |= (rf_r29 << 6);			
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R29, RFValue);
				break;; /* goto Step 2 */
			}
			RtmpOsMsDelay(1);
		}

		if (i != 100)
			continue;
			
		/* goto Step 5 */	
		break;
	}

	/* Step 5 */
	RT28xx_EEPROM_WRITE16(pAd, EEPROM_RSSI_GAIN, rf_r29);

	return NDIS_STATUS_SUCCESS;
}


INT Set_ATE_DIV_ANTENNA_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT32 value;

	value = simple_strtol(arg, 0, 10);
	if (value == 0)
	{
		/* enable hardware antenna diversity */	
		UINT8 BBPValue = 0, RFValue = 0;

		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DIV_ANTENNA_Proc: enable hardware antenna diversity\n"));
		/* RF_R29 bit7:6 = 11 */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R29, &RFValue);
		RFValue |= 0xC0;			
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R29, RFValue);
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R29, &RFValue);

		/* BBP_R47 bit7=1 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
		BBPValue |= 0x80;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
	
		BBPValue = 0xbe;			
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
		BBPValue = 0xb0;			
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
		BBPValue = 0x23;			
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
		BBPValue = 0x3a;			
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R153, BBPValue);
		BBPValue = 0x10;			
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
		BBPValue = 0x3b;			
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R155, BBPValue);
		BBPValue = 0x04;			
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R253, BBPValue);
	}
	else if (value == 1)
	{
		/* fix to main antenna */
		UINT8 BBPValue = 0;

		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DIV_ANTENNA_Proc: fix to main antenna\n"));

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R150, &BBPValue);
		BBPValue &= 0x7f; /* clear bit7 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R151, &BBPValue);
		BBPValue &= 0x7f; /* clear bit7 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R154, &BBPValue);
		BBPValue &= 0xef; /* clear bit4 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BBPValue);

		/* main antenna: BBP_R152 bit7=1 */
		BBPValue |= 0x80;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
	}
	else
	{
		/* fix to aux antenna */
		UINT8 BBPValue = 0;

		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DIV_ANTENNA_Proc: fix to aux antenna\n"));
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R150, &BBPValue);
		BBPValue &= 0x7f; /* clear bit7 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R151, &BBPValue);
		BBPValue &= 0x7f; /* clear bit7 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R154, &BBPValue);
		BBPValue &= 0xef; /* clear bit4 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BBPValue);
		/* aux antenna: BBP_R152 bit7=0 */
		BBPValue &= 0x7f;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
	}

	return TRUE;
}
#endif /* HW_ANTENNA_DIVERSITY_SUPPORT */


#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3370) || defined(RT3390) || defined(RT3350) || defined(RT3352)
CHAR RTUSBInsertTssi(UCHAR InChannel, UCHAR Channel0, UCHAR Channel1,CHAR Tssi0, CHAR Tssi1)
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
#endif /* defined(RT3370) || defined(RT3390) || defined(RT3350) || defined(RT3352) */


INT Set_ATE_TSSI_CALIBRATION_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{    
	RTMP_CHIP_ATE_TSSI_CALIBRATION(pAd, arg);
	return TRUE;
}


INT Set_ATE_TSSI_CALIBRATION_EX_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{    
	ATE_INFO *pATEInfo = &(pAd->ate);

	if (pATEInfo->pChipStruct->ExtendedTssiCalibration != NULL)
	{
		pATEInfo->pChipStruct->ExtendedTssiCalibration(pAd, arg);
		return TRUE;
	}
	else
	{
		RTMP_CHIP_ATE_TSSI_CALIBRATION_EXTEND(pAd, arg);
	}

	return TRUE;
}

extern UCHAR CCK_Rate2MCS(PRTMP_ADAPTER pAd);

extern UCHAR OFDM_Rate2MCS(PRTMP_ADAPTER pAd);

extern VOID	RTMPReadChannelPwr(PRTMP_ADAPTER pAd);

extern INT32 TSSIDelta2PowDelta(UINT32 TSSI_x_10000, UINT32 TSSI_ref);


INT ATE_GET_TSSI(
	IN PRTMP_ADAPTER pAd,
	IN	INT	MODE,
	IN	INT	MCS,
	OUT PUCHAR  pBbpR49)
{
	UCHAR		BbpR47=0;
	UCHAR		TssiInfo0=0;
	UCHAR		TssiInfo1=0;
	UCHAR		TssiInfo2=0;
	UCHAR		i;

	return TRUE;
}




#if defined(RT3350) || defined(RT3352)
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
	UCHAR		BbpData = 0, RFValue, BBP49Value;
	CHAR		TssiRefPerChannel[14], TssiDeltaPerChannel[14];
	USHORT		EEPData;
	USHORT		ChannelPower;
	UCHAR		BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	UCHAR		CurrentChannel;

	if ((!IS_RT3350(pAd)) && (!IS_RT3352(pAd)))                
	{
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI calibration not supported due to not 3350/3352 chip!!!\n"));
		return (FALSE);
	}

	/* Internal TSSI 0 */

	/* step 1: calibrate channel 7 TSSI as reference value */
	RFValue = (0x3 | 0x0 << 2 | 0x3 << 4);
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R27, RFValue);

	RFValue = (0x3 | 0x0 << 2);
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R28, RFValue);

	/* set BBP R49[7] = 1 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
	BBP49Value = BbpData;
	BbpData |= 0x80;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpData);

	/* start TX at 54Mbps */
	NdisZeroMemory(&pAd->ate, sizeof(struct _ATE_INFO));
	/* prevent the potential bug that if ATE_START != 0x00 */
	pAd->ate.Mode = ATE_START;
	CurrentChannel = 7;
	pAd->ate.TxCount = 100000;
	pAd->ate.TxLength = 1024;
	pAd->ate.Channel = CurrentChannel;

	COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
	COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
	COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);                     

	Set_ATE_TX_MODE_Proc(pAd, "1");		/* MODE_OFDM */
	Set_ATE_TX_MCS_Proc(pAd, "7");		/* 54Mbps */
	Set_ATE_TX_BW_Proc(pAd, "0");		/* 20MHz */

	/* read frequency offset from EEPROM */                        
	RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
	pAd->ate.RFFreqOffset = (UCHAR) (EEPData & 0xff);

	/* read calibrated channel power value from EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + CurrentChannel - 1, ChannelPower);
	pAd->ate.TxPower0 =	(UCHAR) (ChannelPower & 0xff);

	/* Only Tx0 is calibrated by default.*/
	Set_ATE_TX_Antenna_Proc(pAd, "1");		
	DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%x\n", CurrentChannel, pAd->ate.TxPower0));

	Set_ATE_Proc(pAd, "TXFRAME"); 
	RTMPusecDelay(500000);
		
	/* read BBP R49[4:0] and write to EEPROM 0x6E */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
	DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%x\n", BbpData)); 
	BbpData &= 0x1f;
	TssiRefPerChannel[CurrentChannel-1] = BbpData;
	DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%x\n", TssiRefPerChannel[CurrentChannel-1]));  
	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
	EEPData &= 0xff00;
	EEPData |= BbpData;
	DBGPRINT(RT_DEBUG_TRACE, ("Write  E2P 0x6e: 0x%x\n", EEPData));  

	RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
	RTMPusecDelay(10);
	 
	/* stop TX */
	Set_ATE_Proc(pAd, "ATESTART"); 

	/* step 2: calibrate channel 1 and 13 TSSI delta values */
	/* Channel 1 */
	/* start TX at 54Mbps */
	CurrentChannel = 1;
	pAd->ate.TxCount = 100000;
	pAd->ate.TxLength = 1024;
	pAd->ate.Channel = CurrentChannel;

	COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
	COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
	COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);  

	Set_ATE_TX_MODE_Proc(pAd, "1");		/* MODE_OFDM */
	Set_ATE_TX_MCS_Proc(pAd, "7");		/* 54Mbps */
	Set_ATE_TX_BW_Proc(pAd, "0");		/* 20MHz */
	
	/* read calibrated channel power value from EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + CurrentChannel - 1, ChannelPower);
	pAd->ate.TxPower0 =	(UCHAR) (ChannelPower & 0xff);			

	/* Only Tx0 is calibrated by default.*/
	Set_ATE_TX_Antenna_Proc(pAd, "1");		
	DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%x\n", CurrentChannel, pAd->ate.TxPower0));

	Set_ATE_Proc(pAd, "TXFRAME"); 
	RTMPusecDelay(500000);

	/* read BBP R49[4:0] */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
	BbpData &= 0x1f;
	TssiRefPerChannel[CurrentChannel-1] = BbpData;
	DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%x\n", TssiRefPerChannel[CurrentChannel-1]));

	/* stop TX */
	Set_ATE_Proc(pAd, "ATESTART");

	/* Channel 13 */
	/* start TX at 54Mbps */
	CurrentChannel = 13;
	pAd->ate.TxCount = 100000;
	pAd->ate.TxLength = 1024;
	pAd->ate.Channel = CurrentChannel;

	COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
	COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
	COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);  

	Set_ATE_TX_MODE_Proc(pAd, "1");		/* MODE_OFDM */
	Set_ATE_TX_MCS_Proc(pAd, "7");		/* 54Mbps */
	Set_ATE_TX_BW_Proc(pAd, "0");		/* 20MHz */
	
	/* read calibrated channel power value from EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + CurrentChannel - 1, ChannelPower);
	pAd->ate.TxPower0 =	(UCHAR) (ChannelPower & 0xff);			

	/* Only Tx0 is calibrated by default.*/
	Set_ATE_TX_Antenna_Proc(pAd, "1");
		
	DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%x\n", CurrentChannel, pAd->ate.TxPower0));

	Set_ATE_Proc(pAd, "TXFRAME"); 
	RTMPusecDelay(500000);

	/* read BBP R49[4:0] */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
	BbpData &= 0x1f;
	TssiRefPerChannel[CurrentChannel-1] = BbpData;
	DBGPRINT(RT_DEBUG_TRACE, ("TSSI = 0x%x\n", TssiRefPerChannel[CurrentChannel-1]));

	/* stop TX */
	Set_ATE_Proc(pAd, "ATESTART");

	/* step 3: insert the TSSI table */
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
		TssiDeltaPerChannel[CurrentChannel-1] = TssiRefPerChannel[CurrentChannel-1] -TssiRefPerChannel[6];

        /* boundary check */
        if(TssiDeltaPerChannel[CurrentChannel-1] > 7 )
		TssiDeltaPerChannel[CurrentChannel-1] = 7;
        if(TssiDeltaPerChannel[CurrentChannel-1] < -8 )
		TssiDeltaPerChannel[CurrentChannel-1] = -8;

        /* eeprom only use 4 bit for TSSI delta */
        TssiDeltaPerChannel[CurrentChannel-1]  &= 0x0f;
        DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, TSSI= 0x%x, TssiDelta=0x%x\n", 
		CurrentChannel, TssiRefPerChannel[CurrentChannel-1], TssiDeltaPerChannel[CurrentChannel-1]));    
	}

	/* step 4: store TSSI delta values to EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);
	EEPData &= 0x00ff;
	EEPData |= (TssiDeltaPerChannel[0] << 8) | (TssiDeltaPerChannel[1] << 12);
	RT28xx_EEPROM_WRITE16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);
	RTMPusecDelay(10);
	
	for (CurrentChannel = 3; CurrentChannel<=14; CurrentChannel+=4)
	{
		EEPData = ( TssiDeltaPerChannel[CurrentChannel+2]  << 12) |(  TssiDeltaPerChannel[CurrentChannel+1]  << 8) | 
			(TssiDeltaPerChannel[CurrentChannel] << 4) | TssiDeltaPerChannel[CurrentChannel-1];
		RT28xx_EEPROM_WRITE16(pAd, (EEPROM_TX_POWER_OFFSET_OVER_CH_3 + ((CurrentChannel-3)/2)), EEPData);
		RTMPusecDelay(10);
	}

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



#ifdef RT28xx
#ifdef RT2860
struct _ATE_CHIP_STRUCT RALINK2860 =
{
	.ChannelSwitch = RT28xxATEAsicSwitchChannel,
	.TxPwrHandler = RT28xxATETxPwrHandler,
	.ExtendedTssiCalibration = NULL,
	.RxVariableGainAmplifierInit = RT28xxATERxVariableGainAmplifierInit,
	.Set_BW_Proc = RT28xx_Set_ATE_TX_BW_Proc,
};
#endif /* RT2860 */


#ifdef RT2880
struct _ATE_CHIP_STRUCT RALINK2880 =
{
	.ChannelSwitch = RT28xxATEAsicSwitchChannel,
	.TxPwrHandler = RT28xxATETxPwrHandler,
	.ExtendedTssiCalibration = NULL,
	.RxVariableGainAmplifierInit = RT28xxATERxVariableGainAmplifierInit,
	.Set_BW_Proc = RT28xx_Set_ATE_TX_BW_Proc,
};
#endif /* RT2880 */
#endif /* RT28xx */

#ifdef RT2883
struct _ATE_CHIP_STRUCT RALINK2883 =
{
	.ChannelSwitch = RT2883ATEAsicSwitchChannel,
	.TxPwrHandler = RT2883ATETxPwrHandler,
	.ExtendedTssiCalibration = NULL,
	.RxVariableGainAmplifierInit = RT2883ATERxVariableGainAmplifierInit,
	.Set_BW_Proc = RT2883_Set_ATE_TX_BW_Proc,
};
#endif /* RT2883 */

#ifdef RT3883
struct _ATE_CHIP_STRUCT RALINK3883 =
{
	.ChannelSwitch = RT3883ATEAsicSwitchChannel,
	.TxPwrHandler = RT3883ATETxPwrHandler,
	.ExtendedTssiCalibration = NULL,
	.RxVariableGainAmplifierInit = RT3883ATERxVariableGainAmplifierInit,
	.Set_BW_Proc = RT3883_Set_ATE_TX_BW_Proc,
};
#endif /* RT3883 */


#ifdef RT305x
#ifdef RT3050
struct _ATE_CHIP_STRUCT RALINK3050 =
{
	.ChannelSwitch = RT3050ATEAsicSwitchChannel,
	.TxPwrHandler = RT305xATETxPwrHandler,
	.ExtendedTssiCalibration = NULL,
	.RxVariableGainAmplifierInit = RT3050ATERxVariableGainAmplifierInit,
	.Set_BW_Proc = RT305x_Set_ATE_TX_BW_Proc,
};
#endif /* RT3050 */


#ifdef RT3350
struct _ATE_CHIP_STRUCT RALINK3350 =
{
	.ChannelSwitch = RT3350ATEAsicSwitchChannel,
	.TxPwrHandler = RT305xATETxPwrHandler,
	.ExtendedTssiCalibration = RT335xATETssiCalibrationExtend,
	.RxVariableGainAmplifierInit = RT3350ATERxVariableGainAmplifierInit,
	.Set_BW_Proc = RT3350_Set_ATE_TX_BW_Proc,
};
#endif /* RT3350 */

#ifdef RT3352
struct _ATE_CHIP_STRUCT RALINK3352 =
{
	.ChannelSwitch = RT3352ATEAsicSwitchChannel,
	.TxPwrHandler = RT3352ATETxPwrHandler,
	.ExtendedTssiCalibration = RT335xATETssiCalibrationExtend,
	.RxVariableGainAmplifierInit = RT3352ATERxVariableGainAmplifierInit,
	.Set_BW_Proc = RT3352_Set_ATE_TX_BW_Proc,
};
#endif /* RT3352 */

#endif /* RT305x */

struct _ATE_CHIP_STRUCT RALINKDefault =
{
	.ChannelSwitch = DefaultATEAsicSwitchChannel,
	.TxPwrHandler = DefaultATETxPwrHandler,
	.ExtendedTssiCalibration = NULL,
	.RxVariableGainAmplifierInit = NULL,
	.Set_BW_Proc = Default_Set_ATE_TX_BW_Proc,
};


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
	ATE_INFO *pATEInfo = &(pAd->ate);

	pATEInfo->pChipStruct = &RALINKDefault;

#ifdef RT28xx
#ifdef RT2860
	if (IS_PCI_ONLY_INF(pAd))
	{
		pATEInfo->pChipStruct = &RALINK2860;
	}
	else
#endif /* RT2860 */
#ifdef RT2880
	if (IS_RBUS_INF(pAd))
	{
		pATEInfo->pChipStruct = &RALINK2880;
	}
	else
#endif /* RT2880 */
	if (pATEInfo->pChipStruct == &RALINKDefault)
	{
		DBGPRINT_ERR(("Error - Unknown chipset !!!\n"));
		DBGPRINT_ERR(("The interface type is %d\n", pAd->infType));

		return NDIS_STATUS_FAILURE;
	}
#endif /* RT28xx */

#ifdef RT2883
	if (IS_RT2883(pAd))
		pATEInfo->pChipStruct = &RALINK2883;
#endif /* RT2883 */

#ifdef RT3883
	if (IS_RT3883(pAd))
		pATEInfo->pChipStruct = &RALINK3883;
#endif /* RT3883 */


#ifdef RT305x
#ifdef RT3050
	if (IS_RT3050(pAd))
		pATEInfo->pChipStruct = &RALINK3050;
#endif /* RT3050 */


#ifdef RT3350
	if (IS_RT3350(pAd))
		pATEInfo->pChipStruct = &RALINK3350;
#endif /* RT3350 */

#ifdef RT3352
	if (IS_RT3352(pAd))
		pATEInfo->pChipStruct = &RALINK3352;
#endif /* RT3352 */

#endif /* RT305x */

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
	ATE_INFO *pATEInfo = &(pAd->ate);

	NdisZeroMemory(pATEInfo, sizeof(ATE_INFO));

	if (ChipStructAssign(pAd) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("%s() failed !\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}
	
	pATEInfo->Mode = ATE_STOP;
#ifdef RT3350
	pATEInfo->PABias = 0;
#endif /* RT3350  */
	pATEInfo->TxCount = 1000;/* to sync with QA and to exceed TX_RING_SIZE ... */
	pATEInfo->TxDoneCount = 0;
	pATEInfo->RFFreqOffset = 0;
	pATEInfo->Payload = 0xA5;/* to be backward compatible */	
	pATEInfo->IPG = 200;/* 200 : sync with QA */	
	pATEInfo->TxLength = 1024;
	pATEInfo->TxWI.ShortGI = 0;/* LONG GI : 800 ns*/
	pATEInfo->TxWI.PHYMODE = MODE_CCK;
	pATEInfo->TxWI.MCS = 3;
	pATEInfo->TxWI.BW = BW_20;
	/* please do not change this default channel value */
	pATEInfo->Channel = 1;


#ifdef RTMP_MAC_PCI 
#endif /* RTMP_MAC_PCI */
	pATEInfo->QID = QID_AC_BE;

#if defined (RT2883) || defined (RT3883)
	/* For stream mode in 3T/3R ++ */
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
#endif /* defined (RT2883) || defined (RT3883) */

	pATEInfo->bRxFER = 0;
	pATEInfo->bQAEnabled = FALSE;
	pATEInfo->bQATxStart = FALSE;
	pATEInfo->bQARxStart = FALSE;
	pATEInfo->bAutoTxAlc = FALSE;

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
#ifndef RTMP_RBUS_SUPPORT
	pATEInfo->bFWLoading = FALSE;
#endif /* RTMP_RBUS_SUPPORT */
#endif /* RTMP_MAC_PCI */


#ifdef RALINK_QA
	pATEInfo->TxStatus = 0;
	RtmpOsTaskPidInit(&(pATEInfo->AtePid));
/*	pATEInfo->AtePid = THREAD_PID_INIT_VALUE; */
#endif /* RALINK_QA */

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
	UCHAR   Bbp22Value = 0, Bbp24Value = 0;

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &Bbp22Value);

	switch (Bbp22Value)
	{
		case BBP22_TXFRAME:
			{
#ifdef RTMP_MAC_PCI
				if (pAd->ate.TxCount == 0)
				{
					pAd->ate.TxCount = 0xFFFFFFFF;
				}
#endif /* RTMP_MAC_PCI */
				DBGPRINT(RT_DEBUG_TRACE,("START TXFRAME\n"));
				pAd->ate.bQATxStart = TRUE;
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
							pAd->ate.bQATxStart = TRUE;

							if (pAd->ate.TxMethod == TX_METHOD_0)
							{
								Set_ATE_Proc(pAd, "TXCONT");
							}
						}
						break;

					case BBP24_CARRSUPP:
						{
							DBGPRINT(RT_DEBUG_TRACE,("START TXCARRSUPP\n"));
							pAd->ate.bQATxStart = TRUE;

							if (pAd->ate.TxMethod == TX_METHOD_0)
							{
								Set_ATE_Proc(pAd, "TXCARS");
							}
						}
						break;

					default:
						{
							DBGPRINT(RT_DEBUG_ERROR,("Unknown TX subtype !"));
						}
						break;
				}
			}
			break;	

		case BBP22_TXCARR:
			{
				DBGPRINT(RT_DEBUG_TRACE,("START TXCARR\n"));
				pAd->ate.bQATxStart = TRUE;

				if (pAd->ate.TxMethod == TX_METHOD_0)
				{
					Set_ATE_Proc(pAd, "TXCARR");
				}
			}
			break;							

		default:
			{
				DBGPRINT(RT_DEBUG_ERROR,("Unknown Start TX subtype !"));
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


/*
==========================================================================
	Description:
		This routine sets initial value of VGA in the RX chain.
		AGC is the abbreviation of "Automatic Gain Controller",
		while VGA (Variable Gain Amplifier) is a part of AGC loop.
		(i.e., VGA + level detector + feedback loop = AGC)

    Return:
        VOID
==========================================================================
*/
#ifdef RT3883
VOID RT3883ATERxVariableGainAmplifierInit(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	ATE_INFO *pATEInfo = &(pAd->ate);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
/*		R66 = ((pATEInfo->TxWI.BW == BW_20) ? (0x30) : (0x38)); */
		R66 = (UCHAR)(0x2E + LNAGain);
	}
	else 
	{
		/* A band */
		R66 = (UCHAR)(0x20 + (LNAGain * 5)/3);
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}
#endif /* RT3883 */


#ifdef RT305x
#ifdef RT3050
VOID RT3050ATERxVariableGainAmplifierInit(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	ATE_INFO *pATEInfo = &(pAd->ate);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
		R66 = (UCHAR)(0x2E + LNAGain);
	}
	else 
	{
		/* A band. Not supported. */
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Ch=%d\n", __FUNCTION__, pATEInfo->Channel));
		DBGPRINT(RT_DEBUG_ERROR, ("%s():5 GHz band not supported !\n", __FUNCTION__));		

		return;
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}
#endif /* RT3050 */




#ifdef RT3350
VOID RT3350ATERxVariableGainAmplifierInit(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	ATE_INFO *pATEInfo = &(pAd->ate);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
		R66 = (UCHAR)(0x2E + LNAGain);
	}
	else 
	{
		/* A band. Not supported. */
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Ch=%d\n", __FUNCTION__, pATEInfo->Channel));
		DBGPRINT(RT_DEBUG_ERROR, ("%s():5 GHz band not supported !\n", __FUNCTION__));		

		return;
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}
#endif /* RT3350 */


#ifdef RT3352
VOID RT3352ATERxVariableGainAmplifierInit(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	ATE_INFO *pATEInfo = &(pAd->ate);

	if (pATEInfo->Channel <= 14)
	{	
		/* BG band */
		if (pATEInfo->TxWI.BW == BW_20)
		{
			R66 = (UCHAR)(0x1C + (LNAGain*2));
		}
		else
		{
			R66 = (UCHAR)(0x24 + (LNAGain*2));
		}
	}
	else
	{	
		/* A band. Not supported. */
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Ch=%d\n", __FUNCTION__, pATEInfo->Channel));
		DBGPRINT(RT_DEBUG_ERROR, ("%s():5 GHz band not supported !\n", __FUNCTION__));		

		return;
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	

	return;
}
#endif /* RT3352 */


#endif /* RT305x */




#ifdef RT2883
VOID RT2883ATERxVariableGainAmplifierInit(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	ATE_INFO *pATEInfo = &(pAd->ate);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
		R66 = (UCHAR)(0x2E + LNAGain);
	}
	else 
	{
		/* A band */
		if (pATEInfo->TxWI.BW == BW_20)
		{
			/* A band, BW == 20 */
			R66 = (UCHAR)(0x32 + (LNAGain*5)/3);
 		}
		else
		{
			/* A band, BW == 40 */
			R66 = (UCHAR)(0x3A + (LNAGain*5)/3);
		}
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}
#endif /* RT2883 */


#if defined(RT28xx) || defined(RT2880)
VOID RT28xxATERxVariableGainAmplifierInit(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	ATE_INFO *pATEInfo = &(pAd->ate);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
		R66 = (UCHAR)(0x2E + LNAGain);
	}
	else 
	{
		/* A band */
		if (pATEInfo->TxWI.BW == BW_20)
		{
			/* A band, BW == 20 */
			R66 = (UCHAR)(0x32 + (LNAGain*5)/3);
		}
		else
		{
			/* A band, BW == 40 */
			R66 = (UCHAR)(0x3A + (LNAGain*5)/3);
		}
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}
#endif /* defined(RT28xx) || defined(RT2880) */

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


