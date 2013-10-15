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
	rt3352_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT3352

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3352

#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];
extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern UCHAR NUM_OF_3020_CHNL;


/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3352 ATE.
    
==========================================================================
*/
VOID RT3352ATEAsicSwitchChannel(
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

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);

				if (pATEInfo->TxWI.BW == BW_20)
				{
					RFValue &= ~(0x03); /* 20MBW tx_h20M=0, rx_h20M=0 */
				}
				else
				{
					RFValue |= 0x03; /* 40MBW tx_h20M=1, rx_h20M=1 */
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

				if (pATEInfo->bAutoTxAlc == FALSE)
				{
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R47, pATEInfo->TxPower0);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R48, pATEInfo->TxPower1);
				}

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
				if (pATEInfo->RxAntennaSel == 1)
				{
					RFValue = RFValue | 0x04; /* rx0_en */
				}
				else if (pATEInfo->RxAntennaSel == 2)
				{
					RFValue = RFValue | 0x10; /* rx1_en */
				}
				else 
				{
					RFValue = RFValue | 0x14; /* rx0_en and rx1_en */
				}

				if (pATEInfo->TxAntennaSel == 1)
				{
					RFValue = RFValue | 0x08; /* tx0_en */
				}
				else if (pATEInfo->TxAntennaSel == 2)
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

				if (pATEInfo->RxAntennaSel == 1)
				{
					BbpValue &= ~0x3; /* BBP_R3[1:0]=00 (ADC0) */
				}
				else if (pATEInfo->RxAntennaSel == 2)
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

				if (pATEInfo->TxWI.BW == BW_20)
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

		if (pATEInfo->TxAntennaSel == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		else if (pATEInfo->TxAntennaSel == 2)
		{
			TxPinCfg &= 0xFFFFFFFC;
		}
		else
		{
			TxPinCfg &= 0xFFFFFFFF;
		}

		if (pATEInfo->RxAntennaSel == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}
		else if (pATEInfo->RxAntennaSel == 2)
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
}


INT RT3352ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR TxPower = 0;

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
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R47, TxPower);
	}

	if (index == 1)
	{
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R48, TxPower);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s : (TxPower%d=%d)\n", __FUNCTION__, index, TxPower));
	
	return 0;
}	


VOID RT3352ATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);

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
		DBGPRINT_ERR(("%s :Ch=%d\n", __FUNCTION__, pATEInfo->Channel));
		DBGPRINT_ERR(("%s :5 GHz band not supported !\n", __FUNCTION__));		

		return;
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	

	return;
}


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

			if ((pATEInfo->TxWI.PHYMODE >= 2) && (pATEInfo->TxWI.MCS == 7))
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

	return TRUE;
}


INT	RT3352_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue = 0;
	UCHAR offset = 0;
	UCHAR RFValue2 = 0;
	
	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset >= 96)
	{
		DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}

	pATEInfo->RFFreqOffset = RFFreqOffset;

	/* set RF frequency offset */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);

	RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;

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
			
	return TRUE;
}


VOID RT3352ATEAsicAdjustTxPower(
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
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL, pTxPowerTuningEntry2 = NULL;
	CHAR TotalDeltaPower = 0; /* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */    
	CHAR AntennaDeltaPwr = 0, TotalDeltaPower2 = 0, MAC_PowerDelta2 = 0, TuningTableIndex2 = 0;
	UCHAR RFValue2 = 0;
	CHAR desiredTSSI = 0, currentTSSI = 0, TuningTableIndex = 0;
	UCHAR RFValue = 0, TmpValue = 0;   
	CHAR Value2 = 0;
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

            /* Valid pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 61 */
			/* zero-based array */
            pTxPowerTuningEntry = &RT3352_TxPowerTuningTable[TuningTableIndex + TX_POWER_TUNING_ENTRY_OFFSET];

            pAd->TxPowerCtrl.RF_TX_ALC = pTxPowerTuningEntry->RF_TX_ALC;
            pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

            DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex = %d, pAd->TxPowerCtrl.RF_TX_ALC = %d, pAd->TxPowerCtrl.MAC_PowerDelta = %d\n", 
                    TuningTableIndex, pAd->TxPowerCtrl.RF_TX_ALC, pAd->TxPowerCtrl.MAC_PowerDelta));

			/* Tx power adjustment over RF */
			RFValue = (UCHAR)pAd->TxPowerCtrl.RF_TX_ALC;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R47, (UCHAR)RFValue); /* TX0_ALC */
			DBGPRINT(RT_DEBUG_TRACE, ("RF_R47 = 0x%02x\n", RFValue));

			/* Delta Power between Tx0 and Tx1 */
			if ((pAd->TxPower[pAd->ate.Channel-1].Power) > (pAd->TxPower[pAd->ate.Channel-1].Power2))
			{
				AntennaDeltaPwr = ((pAd->TxPower[pAd->ate.Channel-1].Power)
									- (pAd->TxPower[pAd->ate.Channel-1].Power2));
				TuningTableIndex2 = TuningTableIndex - AntennaDeltaPwr;
			}
			else if ((pAd->TxPower[pAd->ate.Channel-1].Power) < (pAd->TxPower[pAd->ate.Channel-1].Power2))
			{
				AntennaDeltaPwr = ((pAd->TxPower[pAd->ate.Channel-1].Power2)
									- (pAd->TxPower[pAd->ate.Channel-1].Power));
				TuningTableIndex2 = TuningTableIndex + AntennaDeltaPwr;
			}
			else
			{
				TuningTableIndex2 = TuningTableIndex;
			}

			if (TuningTableIndex2 < LOWERBOUND_TX_POWER_TUNING_ENTRY)
			{
				TuningTableIndex2 = LOWERBOUND_TX_POWER_TUNING_ENTRY;
			}

			if (TuningTableIndex2 >= UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd))
			{
				TuningTableIndex2 = UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd);
			}

            pTxPowerTuningEntry2 = &RT3352_TxPowerTuningTable[TuningTableIndex2 + TX_POWER_TUNING_ENTRY_OFFSET];

            RFValue2 = pTxPowerTuningEntry2->RF_TX_ALC;
            MAC_PowerDelta2 = pTxPowerTuningEntry2->MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("Pwr0 = 0x%02x\n", (pAd->TxPower[pAd->ate.Channel-1].Power)));			  
			DBGPRINT(RT_DEBUG_TRACE, ("Pwr1 = 0x%02x\n", (pAd->TxPower[pAd->ate.Channel-1].Power2)));			  
			DBGPRINT(RT_DEBUG_TRACE, ("AntennaDeltaPwr = %d\n", AntennaDeltaPwr));			  
			DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex = %d\n", TuningTableIndex));			  
			DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex2 = %d\n", TuningTableIndex2));			  
			DBGPRINT(RT_DEBUG_TRACE, ("RFValue = %u\n", RFValue));			  
			DBGPRINT(RT_DEBUG_TRACE, ("RFValue2 = %u\n", RFValue2));			  

			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R48, (UCHAR)RFValue2); /* TX1_ALC */
			DBGPRINT(RT_DEBUG_TRACE, ("RF_R48 = 0x%02x\n", RFValue2));

			TotalDeltaPower2 += MAC_PowerDelta2;

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
				TotalDeltaPower2 += 12;
				BbpR1 |= ATE_MDSM_DROP_TX_POWER_BY_12dBm;

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

				DBGPRINT(RT_DEBUG_TRACE, ("TPC: %s: Drop the transmit power by 12 dBm (BBP R1)\n", __FUNCTION__));
			}
			else if ((TotalDeltaPower <= -6) && (TotalDeltaPower > -12))
			{
				TotalDeltaPower += 6;
				TotalDeltaPower2 += 6;
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
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */
#ifdef RTMP_INTERNAL_TX_ALC
				/* Tx power adjustment over MAC */
				if (j & 0x00000001) /* j=1, 3, 5, 7 */
				{
					/* TX1 ALC */
					Value2 = (CHAR)((TxPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */
				}
				else /* j=0, 2, 4, 6 */
				{
					/* TX0 ALC */
					Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */
				}

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

							/* Tx power adjustment over MAC */
							/* TX1 ALC */
							if ((Value2 + TotalDeltaPower2) < 0)
							{
								Value2 = 0;
							}
							else if ((Value2 + TotalDeltaPower2) > 0xE)
							{
								Value2 = 0xE;
							}
							else
							{
								Value2 += TotalDeltaPower2;
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

							/* Tx power adjustment over MAC */
							/* TX1 ALC */
							if ((j >= 0) && (j <= 3))
							{
								if ((Value2 + TotalDeltaPower2) < 0)
								{
									Value2 = 0;
								}
								else if ((Value2 + TotalDeltaPower2) > 0xC)
								{
									Value2 = 0xC;
								}
								else
								{
									Value2 += TotalDeltaPower2;
								}
							}
							else
							{
								if ((Value2 + TotalDeltaPower2) < 0)
								{
									Value2 = 0;
								}
								else if ((Value2 + TotalDeltaPower2) > 0xE)
								{
									Value2 = 0xE;
								}
								else
								{
									Value2 += TotalDeltaPower2;
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

							/* Tx power adjustment over MAC */
							/* TX1 ALC */
							if ((j == 0) || (j == 2) || (j == 3))
							{
								if ((Value2 + TotalDeltaPower2) < 0)
								{
									Value2 = 0;
								}
								else if ((Value2 + TotalDeltaPower2) > 0xC)
								{
									Value2 = 0xC;
								}
								else
								{
									Value2 += TotalDeltaPower2;
								}
							}
							else
							{
								if ((Value2 + TotalDeltaPower2) < 0)
								{
									Value2 = 0;
								}
								else if ((Value2 + TotalDeltaPower2) > 0xE)
								{
									Value2 = 0xE;
								}
								else
								{
									Value2 += TotalDeltaPower2;
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

							/* Tx power adjustment over MAC */
							/* TX1 ALC */
							if ((j == 0) || (j == 2) || (j == 3) || 
							((j >= 4) && (j <= 7)))
							{
								if ((Value2 + TotalDeltaPower2) < 0)
								{
									Value2 = 0;
								}
								else if ((Value2 + TotalDeltaPower2) > 0xC)
								{
									Value2 = 0xC;
								}
								else
								{
									Value2 += TotalDeltaPower2;
								}
							}
							else
							{
								if ((Value2 + TotalDeltaPower2) < 0)
								{
									Value2 = 0;
								}
								else if ((Value2 + TotalDeltaPower2) > 0xE)
								{
									Value2 = 0xE;
								}
								else
								{
									Value2 += TotalDeltaPower2;
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

							/* Tx power adjustment over MAC */
							/* TX1 ALC */
							if ((Value2 + TotalDeltaPower2) < 0)
							{
								Value2 = 0;
							}
							else if ((Value2 + TotalDeltaPower2) > 0xC)
							{
								Value2 = 0xC;
							}
							else
							{
								Value2 += TotalDeltaPower2;
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
#ifdef RTMP_INTERNAL_TX_ALC
				/* Tx power adjustment over MAC */
				if (j & 0x00000001) /* j=1, 3, 5, 7 */
				{
					/* TX1 ALC */
					TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value2 << j*4);
				}
				else /* j=0, 2, 4, 6 */
				{
					/* TX0 ALC */
					TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
				}
#endif /* RTMP_INTERNAL_TX_ALC */
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


struct _ATE_CHIP_STRUCT RALINK3352 =
{
	/* functions */
	.ChannelSwitch = RT3352ATEAsicSwitchChannel,
	.TxPwrHandler = RT3352ATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = RT335xATETssiCalibrationExtend,
	.RxVGAInit = RT3352ATERxVGAInit,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = RT3352ATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT3352_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT3352_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,	
	.bBBPStoreTXCONT = TRUE,
	.bBBPLoadATESTOP = TRUE,
};

#endif /* RT3352 */

