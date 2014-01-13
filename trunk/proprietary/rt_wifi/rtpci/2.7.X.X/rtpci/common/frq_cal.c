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
	frq_cal.c

	Abstract:
	
	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"

/*
	Sometimes frequency will be shift we need to adjust it when
	the frequencey shift.
*/
VOID InitFrequencyCalibrationMode(
	PRTMP_ADAPTER pAd,
	UINT8 Mode)
{
	BBP_R179_STRUC BbpR179 = {{0}};
	BBP_R180_STRUC BbpR180 = {{0}};
	BBP_R182_STRUC BbpR182 = {{0}};
	
	if (Mode == FREQ_CAL_INIT_MODE0)
	{
		/* Initialize the RX_END_STATUS (1, 5) for "Rx OFDM/CCK frequency offset report"*/
		BbpR179.field.DataIndex1 = 1;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, BbpR179.byte);
		BbpR180.field.DataIndex2 = 5;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, BbpR180.byte);
		BbpR182.field.DataArray = BBP_R57; /* Rx OFDM/CCK frequency offset report*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, BbpR182.byte);
	}
	else if (Mode == FREQ_CAL_INIT_MODE1)
	{
		/* Initialize the RX_END_STATUS (1, 3) for "Rx OFDM/CCK frequency offset report"*/
		BbpR179.field.DataIndex1 = 1;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, BbpR179.byte);
		BbpR180.field.DataIndex2 = 3;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, BbpR180.byte);
		BbpR182.field.DataArray = BBP_R57; /* Rx OFDM/CCK frequency offset report*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, BbpR182.byte);
	}
	else if (Mode == FREQ_CAL_INIT_MODE2)
	{
		/* Initialize the RX_END_STATUS (1) for "Rx OFDM/CCK frequency offset report"*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, 1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, BBP_R57); /* Rx OFDM/CCK frequency offset report*/
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:Unknow mode = %d\n", __FUNCTION__, Mode));
}


/* Initialize the frequency calibration*/
VOID InitFrequencyCalibration(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("---> %s\n", __FUNCTION__));
		
		InitFrequencyCalibrationMode(pAd, pAd->chipCap.FreqCalInitMode);
		
		StopFrequencyCalibration(pAd);

		DBGPRINT(RT_DEBUG_ERROR, ("%s: frequency offset in the EEPROM = %u\n", 
			__FUNCTION__, 
			pAd->RfFreqOffset));

		DBGPRINT(RT_DEBUG_ERROR, ("<--- %s\n", __FUNCTION__));
	}
}


/* To stop the frequency calibration algorithm*/
VOID StopFrequencyCalibration(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

		/* Base on the frequency offset of the EEPROM*/
		pAd->FreqCalibrationCtrl.AdaptiveFreqOffset = (0x7F & (pAd->RfFreqOffset)); /* C1 value control - Crystal calibration*/

		pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon = INVALID_FREQUENCY_OFFSET;
		pAd->FreqCalibrationCtrl.bSkipFirstFrequencyCalibration = TRUE;

		DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->FreqCalibrationCtrl.AdaptiveFreqOffset = 0x%X\n", 
			__FUNCTION__, 
			pAd->FreqCalibrationCtrl.AdaptiveFreqOffset));
		
		DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
	}
}


VOID FrequencyCalibrationMode(
	PRTMP_ADAPTER pAd,
	UINT8 Mode)
{
	UCHAR RFValue = 0;
	UCHAR PreRFValue = 0; 
	
	if (Mode == FREQ_CAL_MODE0)
	{
		RT30xxReadRFRegister(pAd, RF_R23, (PUCHAR)(&RFValue));
		RFValue = ((RFValue & ~0x7F) | (pAd->FreqCalibrationCtrl.AdaptiveFreqOffset & 0x7F));
		RFValue = min(RFValue, 0x5F);
		pAd->FreqCalibrationCtrl.AdaptiveFreqOffset = RFValue; /* Keep modified RF R23 value */
		RT30xxWriteRFRegister(pAd, RF_R23, (UCHAR)RFValue);
		RT30xxReadRFRegister(pAd, RF_R07, (PUCHAR)(&RFValue));
		RFValue = ((RFValue & ~0x01) | 0x01); /* Tune_en (initiate VCO calibration (reset after completion)) */
		RT30xxWriteRFRegister(pAd, RF_R07, (UCHAR)RFValue);
	}
	else if (Mode == FREQ_CAL_MODE1)
	{
		/* Adjust the frequency offset and keep the modified value in AdaptiveFreqOffset */
		RTMPAdjustFrequencyOffset(pAd, &pAd->FreqCalibrationCtrl.AdaptiveFreqOffset);

		/* vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. */
		RTMP_WriteRF(pAd, RF_R03, 0x80, 0x80);		
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("Unknown FrqCalibration Mode\n")); 
}


/* The frequency calibration algorithm*/
VOID FrequencyCalibration(
	IN PRTMP_ADAPTER pAd)
{
	/*BOOLEAN bUpdateRFR = FALSE;*/
	CHAR HighFreqTriggerPoint = 0, LowFreqTriggerPoint = 0;
	CHAR DecreaseFreqOffset = 0, IncreaseFreqOffset = 0;
	
	/* Frequency calibration period: */
	/* a) 10 seconds: Check the reported frequency offset*/
	/* b) 500 ms: Update the RF frequency if possible*/
	if ((pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE) && 
	     (((pAd->FreqCalibrationCtrl.bApproachFrequency == FALSE) && ((pAd->Mlme.PeriodicRound % FREQUENCY_CALIBRATION_PERIOD) == 0)) || 
	       ((pAd->FreqCalibrationCtrl.bApproachFrequency == TRUE) && ((pAd->Mlme.PeriodicRound % (FREQUENCY_CALIBRATION_PERIOD / 20)) == 0))))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

		if (pAd->FreqCalibrationCtrl.bSkipFirstFrequencyCalibration == TRUE)
		{
			pAd->FreqCalibrationCtrl.bSkipFirstFrequencyCalibration = FALSE;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: Skip cuurent frequency calibration (avoid calibrating frequency at the time the STA is just link-up)\n", __FUNCTION__));
		}
		else
		{
			if (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon != INVALID_FREQUENCY_OFFSET)
			{	
				if (pAd->FreqCalibrationCtrl.BeaconPhyMode == MODE_CCK) /* CCK*/
				{
					HighFreqTriggerPoint = HIGH_FREQUENCY_TRIGGER_POINT_CCK;
					LowFreqTriggerPoint = LOW_FREQUENCY_TRIGGER_POINT_CCK;

					DecreaseFreqOffset = DECREASE_FREQUENCY_OFFSET_CCK;
					IncreaseFreqOffset = INCREASE_FREQUENCY_OFFSET_CCK;
				}
				else /* OFDM*/
				{
					HighFreqTriggerPoint = HIGH_FREQUENCY_TRIGGER_POINT_OFDM;
					LowFreqTriggerPoint = LOW_FREQUENCY_TRIGGER_POINT_OFDM;

					DecreaseFreqOffset = DECREASE_FREQUENCY_OFFSET_OFDM;
					IncreaseFreqOffset = INCREASE_FREQUENCY_OFFSET_OFDM;
				}
				
				if ((pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon >= HighFreqTriggerPoint) || 
				     (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon <= LowFreqTriggerPoint))
				{
					pAd->FreqCalibrationCtrl.bApproachFrequency = TRUE;
				}
				
				if (pAd->FreqCalibrationCtrl.bApproachFrequency == TRUE)
				{
					if ((pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon <= DecreaseFreqOffset) && 
					      (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon >= IncreaseFreqOffset))
					{
						pAd->FreqCalibrationCtrl.bApproachFrequency = FALSE; /* Stop approaching frquency if -10 < reported frequency offset < 10*/
					}
					else if (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon > DecreaseFreqOffset)
					{
						pAd->FreqCalibrationCtrl.AdaptiveFreqOffset--;
						DBGPRINT(RT_DEBUG_TRACE, ("%s: -- frequency offset = 0x%X\n", __FUNCTION__, pAd->FreqCalibrationCtrl.AdaptiveFreqOffset));
						FrequencyCalibrationMode(pAd, pAd->chipCap.FreqCalMode);
					}
					else if (pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon < IncreaseFreqOffset)
					{
						pAd->FreqCalibrationCtrl.AdaptiveFreqOffset++;
						DBGPRINT(RT_DEBUG_TRACE, ("%s: ++ frequency offset = 0x%X\n", __FUNCTION__, pAd->FreqCalibrationCtrl.AdaptiveFreqOffset));
						FrequencyCalibrationMode(pAd, pAd->chipCap.FreqCalMode);
					}
				}

				DBGPRINT(RT_DEBUG_TRACE, ("%s: AdaptiveFreqOffset = %d, LatestFreqOffsetOverBeacon = %d, bApproachFrequency = %d\n", 
					__FUNCTION__, 
					pAd->FreqCalibrationCtrl.AdaptiveFreqOffset, 
					pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon, 
					pAd->FreqCalibrationCtrl.bApproachFrequency));
			}
		}
		
		DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
	}
}


inline CHAR GetFrequencyOffsetField(
	PRTMP_ADAPTER pAd,
	PRXWI_STRUC pRxWI,
	UINT8 RxWIFrqOffsetField)
{
	CHAR FreqOffset = 0;

	if (RxWIFrqOffsetField == RXWI_FRQ_OFFSET_FIELD0)
	{
		FreqOffset = (CHAR)(pRxWI->SNR1);
	}
	else if (RxWIFrqOffsetField == RXWI_FRQ_OFFSET_FIELD1)
	{
		FreqOffset = (CHAR)(pRxWI->FOFFSET);
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:Unknow Frequency Offset location(%d)\n", __FUNCTION__, RxWIFrqOffsetField));

	return FreqOffset;		
}


/* Get the frequency offset*/
CHAR GetFrequencyOffset(
	IN PRTMP_ADAPTER pAd, 
	IN PRXWI_STRUC pRxWI)
{
	CHAR FreqOffset = 0;
	
	if (pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration)
	{
		DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

		FreqOffset = GetFrequencyOffsetField(pAd, pRxWI, pAd->chipCap.RxWIFrqOffset);

		if ((FreqOffset < LOWERBOUND_OF_FREQUENCY_OFFSET) || 
		     (FreqOffset > UPPERBOUND_OF_FREQUENCY_OFFSET))
		{
			FreqOffset = INVALID_FREQUENCY_OFFSET;

			DBGPRINT(RT_DEBUG_ERROR, ("%s: (out-of-range) FreqOffset = %d\n", 
				__FUNCTION__, 
				FreqOffset));
		}

		DBGPRINT(RT_DEBUG_INFO, ("%s: FreqOffset = %d\n", 
			__FUNCTION__, 
			FreqOffset));

		DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
	}

	return FreqOffset;
}
