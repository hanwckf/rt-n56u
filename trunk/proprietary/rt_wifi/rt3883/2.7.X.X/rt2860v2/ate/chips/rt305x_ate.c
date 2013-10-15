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
	rt305x_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT305x(i.e., RT3050/RT3051/RT3052)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT305x

#include "rt_config.h"


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern UCHAR NUM_OF_3020_CHNL;


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
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;

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
				RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;

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
				if (pATEInfo->TxWI.BW == BW_40)
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

				if (pATEInfo->RxAntennaSel == 1)
				{
					RFValue = RFValue | 0x10;
				}
				else if (pATEInfo->RxAntennaSel == 2)
				{
					RFValue = RFValue | 0x04;
				}

				/* RF Tx antenna selection */					
				if (pATEInfo->TxAntennaSel == 1)
				{
					RFValue = RFValue | 0x20;
				}
				else if (pATEInfo->TxAntennaSel == 2)
				{
					RFValue = RFValue | 0x08;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

				/* BBP Rx antenna selection */
				if (pATEInfo->RxAntennaSel == 1)
				{
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0x0);
				}
				else if (pATEInfo->RxAntennaSel == 2)
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

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

	DBGPRINT(RT_DEBUG_TRACE, ("RT3050:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		RtmpFreqItems3020[index].N, 
		RtmpFreqItems3020[index].K, 
		RtmpFreqItems3020[index].R));
}
#endif /* RT3050 */


#ifdef RT3052
/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3052 ATE.
    
==========================================================================
*/
VOID RT3052ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;

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
				RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;

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
				if (pATEInfo->TxWI.BW == BW_40)
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

				if (pATEInfo->RxAntennaSel == 1)
				{
					RFValue = RFValue | 0x10;
				}
				else if (pATEInfo->RxAntennaSel == 2)
				{
					RFValue = RFValue | 0x04;
				}

				/* RF Tx antenna selection */					
				if (pATEInfo->TxAntennaSel == 1)
				{
					RFValue = RFValue | 0x20;
				}
				else if (pATEInfo->TxAntennaSel == 2)
				{
					RFValue = RFValue | 0x08;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

				/* BBP Rx antenna selection */
				if (pATEInfo->RxAntennaSel == 1)
				{
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0x0);
				}
				else if (pATEInfo->RxAntennaSel == 2)
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

				/* calibrate power unbalance issues */
				if (pAd->Antenna.field.TxPath == 2)
				{
					if (pATEInfo->TxAntennaSel == 1)
					{
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7; /* DAC0 (11100111B) */
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
					}
					else if (pATEInfo->TxAntennaSel == 2)
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

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

	DBGPRINT(RT_DEBUG_TRACE, ("RT3052:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		RtmpFreqItems3020[index].N, 
		RtmpFreqItems3020[index].K, 
		RtmpFreqItems3020[index].R));
}
#endif /* RT3052 */


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
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;

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
				RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;

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
				if (pATEInfo->TxWI.BW == BW_40)
				{    
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x68);

					if(pATEInfo->TxWI.PHYMODE == MODE_CCK)
						RFValue = 0x3F;
					else
						RFValue = 0x28;
				}
				else
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x48);

					if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
						RFValue = 0x1F;
					else if (pATEInfo->TxWI.PHYMODE == MODE_OFDM)
						RFValue = 0x18;
					else
						RFValue = 0x10;/* Simba:2010.02.04 */
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);

				/* RF Rx antenna selection */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0x03) | 0xC1;

				if (pATEInfo->RxAntennaSel == 1)
				{
					RFValue = RFValue | 0x10;
				}
				else if (pATEInfo->RxAntennaSel == 2)
				{
					RFValue = RFValue | 0x04;
				}

				/* RF Tx antenna selection */					
				if (pATEInfo->TxAntennaSel == 1)
				{
					RFValue = RFValue | 0x20;
				}
				else if (pATEInfo->TxAntennaSel == 2)
				{
					RFValue = RFValue | 0x08;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

				/* BBP Rx antenna selection */
				if (pATEInfo->RxAntennaSel == 1)
				{
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, 0x0);
				}
				else if (pATEInfo->RxAntennaSel == 2)
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

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

	DBGPRINT(RT_DEBUG_TRACE, ("RT3350:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		RtmpFreqItems3020[index].N, 
		RtmpFreqItems3020[index].K, 
		RtmpFreqItems3020[index].R));
}
#endif /* RT3350 */


#if defined(RT3050) || defined(RT3052) || defined(RT3350)
/* This routine is also for RT3050/RT3052/RT3350 */
INT RT305xATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR TxPower = 0;
	UCHAR RFValue = 0;

#ifdef RALINK_QA
	if ((pATEInfo->bQATxStart == TRUE) || (pATEInfo->bQARxStart == TRUE))
	{
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
		DBGPRINT_ERR(("%s : TxPower%d is out of range !\n", __FUNCTION__, index));
		return -1;
	}
		
	if (index == 0)
	{
		/* Set Tx0 Power */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
		RFValue = (RFValue & 0xE0) | TxPower;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);
	}

	if (index == 1)
	{
		/* Set Tx1 Power */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R13, (PUCHAR)&RFValue);
		RFValue = (RFValue & 0xE0) | TxPower;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s : (TxPower%d=%d, RFValue=%x)\n", __FUNCTION__, index, TxPower, RFValue));

	return 0;
}


VOID RT305xATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
		R66 = (UCHAR)(0x2E + LNAGain);
	}
	else 
	{
		/* A band. Not supported. */
		DBGPRINT_ERR(("%s :Ch=%d\n", __FUNCTION__, pATEInfo->Channel));
		DBGPRINT_ERR(("%s :5 GHz band not supported !\n", __FUNCTION__));		

		return;
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}
#endif /* defined(RT3050) || defined(RT3052) || defined(RT3350) */


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
	PATE_INFO pATEInfo = &(pAd->ate);
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
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
	PATE_INFO pATEInfo = &(pAd->ate);
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
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

		/* set RF_R24 */
		if(pATEInfo->TxWI.PHYMODE == MODE_CCK)
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

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
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

		/* set RF_R24 */
		if(pATEInfo->TxWI.PHYMODE == MODE_CCK)
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

	return TRUE;
}	
#endif /* RT3350 */


#if defined(RT3050) || defined(RT3052)
INT	RT305x_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue = 0;
	UCHAR step = 0;
	UCHAR RFValue2 = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset >= 96)
	{
		DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}

	pATEInfo->RFFreqOffset = RFFreqOffset;

	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
	RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;

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

	return TRUE;
}
#endif /* defined(RT3050) || defined(RT3052) */


#ifdef RT3350
INT	RT3350_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue = 0;
	UCHAR step = 0;
	UCHAR RFValue2 = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset >= 96)
	{
		DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}

	pATEInfo->RFFreqOffset = RFFreqOffset;

	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
	RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;

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
	
	return TRUE;
}
#endif /* RT3350 */


#ifdef RT3350
VOID RT3350ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd)
{
	INT			i, j, maxTxPwrCnt;
	CHAR		DeltaPwr = 0;
	BOOLEAN		bAutoTxAgc = FALSE;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep;
	UCHAR		BbpR49 = 0, idx;
	UCHAR		BbpR1 = 0;
	PCHAR		pTxAgcCompensate;
	ULONG		TxPwr[9];	/* NOTE: the TxPwr array size should be the maxima value of all supported chipset!!!! */
	CHAR		Value = 0;
#ifdef RTMP_INTERNAL_TX_ALC
	UINT32		MacPwr = 0;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	CHAR TotalDeltaPower = 0; /* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */    
	CHAR desiredTSSI = 0, currentTSSI = 0, TuningTableIndex = 0;
	UCHAR RFValue = 0, TmpValue = 0;   
#endif /* RTMP_INTERNAL_TX_ALC */

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
/*
			if (desiredTSSI == -1)
			{
				return;
			}
*/
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);

            currentTSSI = BbpR49 & 0x1F;

			/* ATE always excutes extended TSSI. */
			if (1 /* pAd->TxPowerCtrl.bExtendedTssiMode == TRUE */) /* Per-channel TSSI */
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

			if (desiredTSSI < 0x00)
			{
				desiredTSSI = 0x00;
			}
			else if (desiredTSSI > 0x1F)
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

			TuningTableIndex = pAd->TxPowerCtrl.idxTxPowerTable
								+ pAd->TxPower[pAd->ate.Channel-1].Power;

			if (TuningTableIndex < LOWERBOUND_TX_POWER_TUNING_ENTRY)
			{
				TuningTableIndex = LOWERBOUND_TX_POWER_TUNING_ENTRY;
			}

			if (TuningTableIndex >= UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd))
			{
				TuningTableIndex = UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd);
			}

            /* Valid pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 45 */
			/* zero-based array */
            pTxPowerTuningEntry = &RT3350_TxPowerTuningTable[TuningTableIndex + TX_POWER_TUNING_ENTRY_OFFSET];
            pAd->TxPowerCtrl.RF_TX_ALC = pTxPowerTuningEntry->RF_TX_ALC;
            pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

            DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex = %d, pAd->TxPowerCtrl.RF_TX_ALC = %d, pAd->TxPowerCtrl.MAC_PowerDelta = %d\n", 
                    TuningTableIndex, pAd->TxPowerCtrl.RF_TX_ALC, pAd->TxPowerCtrl.MAC_PowerDelta  ));

			/* Tx power adjustment over RF */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)(&RFValue));
			TmpValue = (RFValue & 0xE0);
			RFValue = (TmpValue | (pAd->TxPowerCtrl.RF_TX_ALC & 0x1F));
			DBGPRINT(RT_DEBUG_TRACE, ("Write RF_R12 = 0x%02x\n", RFValue));            
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)(RFValue));

            /* Tx power adjustment over MAC */
            TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, currentTSSI = %d, TuningTableIndex = %d, {RF_TX_ALC = %d, MAC_PowerDelta = %d}\n", 
				__FUNCTION__, 
				desiredTSSI, 
				currentTSSI, 
				TuningTableIndex, 
				pTxPowerTuningEntry->RF_TX_ALC, 
				pTxPowerTuningEntry->MAC_PowerDelta));

			/* The BBP R1 controls the transmit power for all rates */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);

			BbpR1 &= ~ATE_MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;

			if (TotalDeltaPower <= -12)
			{
				TotalDeltaPower += 12;
				BbpR1 |= ATE_MDSM_DROP_TX_POWER_BY_12dBm;

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

				DBGPRINT(RT_DEBUG_TRACE, ("TPC: %s: Drop the transmit power by 12 dBm (BBP R1)\n", __FUNCTION__));
			}
			else if ((TotalDeltaPower <= -6) && (TotalDeltaPower > -12))
			{
				TotalDeltaPower += 6;
				BbpR1 |= ATE_MDSM_DROP_TX_POWER_BY_6dBm;		

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

				DBGPRINT(RT_DEBUG_TRACE, ("TPC: %s: Drop the transmit power by 6 dBm (BBP R1)\n", __FUNCTION__));
			}
			else
			{
				/* Control the transmit power by using the MAC only */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);
			}
        }
	}
#endif /* RTMP_INTERNAL_TX_ALC */

	/* TX power compensation for temperature variation based on TSSI. */
	/* Do it per 4 seconds. */
	if (pAd->ate.OneSecPeriodicRound % 4 == 0)
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

	/* Reset different new tx power for different TX rate. */
	for (i=0; i<maxTxPwrCnt; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				/* Tx power adjustment over MAC */
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
							/* TX0 ALC */
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
							/* TX0 ALC */
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
							/* TX0 ALC */
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
							/* TX0 ALC */
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
							/* TX0 ALC */
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
				/* Tx power adjustment over MAC */
				/* TX0 ALC only */
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			/* write tx power value to CSR */
			/* TX_PWR_CFG_0 (8 tx rate) for	TX power for OFDM 12M/18M
											TX power for OFDM 6M/9M
											TX power for CCK5.5M/11M
											TX power for CCK1M/2M */
			/* TX_PWR_CFG_1 ~ TX_PWR_CFG_4 */
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);

#ifdef RTMP_INTERNAL_TX_ALC
			/* Tx power adjustment over MAC */
			RTMP_IO_READ32(pAd, TX_PWR_CFG_0 + (i << 2), &MacPwr);

			DBGPRINT(RT_DEBUG_TRACE, ("%s: MAC register = 0x%X, MacPwr = 0x%X\n", 
				__FUNCTION__, 
				(TX_PWR_CFG_0 + (i << 2)), MacPwr));
#endif /* RTMP_INTERNAL_TX_ALC */
		}
	}
}
#endif /* RT3350 */


#ifdef RT3050
struct _ATE_CHIP_STRUCT RALINK3050 =
{
	/* functions */
	.ChannelSwitch = RT3050ATEAsicSwitchChannel,
	.TxPwrHandler = RT305xATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT305xATERxVGAInit,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = DefaultATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT305x_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT305x_Set_ATE_TX_FREQ_OFFSET_Proc,
	
	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};
#endif /* RT3050 */

#ifdef RT3052
struct _ATE_CHIP_STRUCT RALINK3052 =
{
	/* functions */
	.ChannelSwitch = RT3052ATEAsicSwitchChannel,
	.TxPwrHandler = RT305xATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT305xATERxVGAInit,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = DefaultATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT305x_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT305x_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};
#endif /* RT3052 */

#ifdef RT3350
struct _ATE_CHIP_STRUCT RALINK3350 =
{
	/* functions */
	.ChannelSwitch = RT3350ATEAsicSwitchChannel,
	.TxPwrHandler = RT305xATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = RT335xATETssiCalibrationExtend,
	.RxVGAInit = RT305xATERxVGAInit,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = RT3350ATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT3350_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT3350_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};
#endif /* RT3350 */


#endif /* RT305x */

