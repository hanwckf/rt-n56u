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
	rt35xx_ate.c

	Abstract:
	Specific ATE funcitons and variables for 
		RT3062
		RT3562
		RT3572
		RT3592

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT35xx

#include "rt_config.h"


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

extern FREQUENCY_ITEM FreqItems3572[];
extern UCHAR NUM_OF_3572_CHNL;


/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3572, 3592, 3562, and 3062 ATE.
    
==========================================================================
*/
VOID RT35xxATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;

	for (index = 0; index < NUM_OF_3572_CHNL; index++)
	{
		if (Channel == FreqItems3572[index].Channel)
		{
			/* for 2.4G, restore BBP25, BBP26 */
			if (Channel <= 14)
			{
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, pAd->Bbp25);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, pAd->Bbp26);
			}
			/* hard code for 5GHz, Gary 2008-12-10 */
			else
			{
				/* enable IQ Phase Correction */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x09);
				/* IQ phase correction value */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, 0xFF);
			}

			/* programming channel parameters */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, FreqItems3572[index].N);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, FreqItems3572[index].K);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = (RFValue & 0xF0) | FreqItems3572[index].R | 0x8;
			else
				RFValue = (RFValue & 0xF0) | FreqItems3572[index].R | 0x4;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);

			/* PLL mode for 2.4G or 5G */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R05, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = (RFValue & 0xF3) | 0x4;
			else
				RFValue = (RFValue & 0xF3) | 0x8;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R05, (UCHAR)RFValue);

			/* set Tx0 Power */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = 0x60 | TxPwer;
			else
				RFValue = 0xE0 | (TxPwer & 0x3) | ((TxPwer & 0xC) << 1);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

			/* set Tx1 Power */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R13, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = 0x60 | TxPwer2;
			else
				RFValue = 0xE0 | (TxPwer2 & 0x3) | ((TxPwer2 & 0xC) << 1);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);

			/* Tx/Rx stream setting */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
			RFValue &= 0x03;	/* clear bit[7~2] */
			if (pAd->Antenna.field.TxPath == 1)
				RFValue |= 0xA0;
			else if (pAd->Antenna.field.TxPath == 2)
				RFValue |= 0x80;
			if (pAd->Antenna.field.RxPath == 1)
				RFValue |= 0x50;
			else if (pAd->Antenna.field.RxPath == 2)
				RFValue |= 0x40;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

			ATEAsicSetTxRxPath(pAd);

			/* set RF offset */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
			RFValue = (RFValue & 0x80) | pATEInfo->RFFreqOffset;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);

			/* copy to Set_ATE_TX_BW_Proc() as RT30xx ATE driver. */
			/* set BW */
			if (/*!bScan &&*/(pATEInfo->TxWI.BW == BW_40))
			{
				RFValue = pAd->Mlme.CaliBW40RfR24;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, 0x28);
			}
			else
			{
				RFValue = pAd->Mlme.CaliBW20RfR24;

				if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, 0x1f);
				else
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, 0x08);
			}

			/* R24, R31, one is for tx, the other is for rx. */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, (UCHAR)RFValue);

			/* enable RF tuning */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R07, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = 0xd8;	/* ?? to check 3572?? hardcode */
			else
				RFValue = (RFValue & 0x37) | 0x14;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R07, (UCHAR)RFValue);

			/* TSSI input band select */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R09, (PUCHAR)&RFValue);
			if (Channel <= 14)
			{
				RFValue = 0xC3;
			}
			else
			{
				RFValue = 0xC0;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, (UCHAR)RFValue);

			/* loop filter 1 */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R10, (UCHAR)0xF1);

			/* loop filter 2 */
			if (Channel <= 14)
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)0xB9);
			else
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)0x00);

			/* tx_mx2_ic */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R15, (PUCHAR)&RFValue);
			if (Channel <= 14)
				RFValue = (RFValue & 0x8F) | 0x50;
			else
				RFValue = (RFValue & 0x8F) | 0x50;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R15, (UCHAR)RFValue);


			/* tx_mx1_ic */
			if (Channel <= 14)
			{
				RFValue = 0x4c;
				/* clear bit [2:0] */
				RFValue &= (~0x7);  
				RFValue |= pAd->TxMixerGain24G;
			}
			else 
			{
				RFValue = 0x7a;
				/* clear bit [2:0] */
				RFValue &= (~0x7);  
				RFValue |= pAd->TxMixerGain5G;
			}

			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R16, (UCHAR)RFValue);

			/* tx_lo1 */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)0x23);

			/* tx_lo2 */
			RFValue = ((Channel <= 14) ? (0x93) : ((Channel <= 64) ? (0xb7) : ((Channel <= 128) ? (0x74) : (0x72))));
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R19, (UCHAR)RFValue);

			/* rx_l01 */
			RFValue = ((Channel <= 14) ? (0xB3) : ((Channel <= 64) ? (0xF6) : ((Channel <= 128) ? (0xF4) : (0xF3))));
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R20, (UCHAR)RFValue);

			/* pfd_delay */
			RFValue = ((Channel <= 14) ? (0x15) : ((Channel <= 64) ? (0x3d) : ((Channel <= 128) ? (0x01) : (0x01))));
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R25, (UCHAR)RFValue);

			/* rx_lo2 */
			if (Channel <= 14)
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R26, (UCHAR)0x85);
			else
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R26, (UCHAR)0x87);

			/* drv_cc */
			if (Channel <= 14)
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R29, (UCHAR)0x9B);
			else
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R29, (UCHAR)0x9F);

			/* Band selection */
			RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
			if (pAd->infType == RTMP_DEV_INF_PCIE)
			{
				/* GPIO 5 for RT3592 */
				if  (Channel <= 14)
					Value = ((Value & 0xFFFFDFFF) | 0x00000020);
				else
					Value = (Value & 0xFFFFDFDF);
			}
			else
			{
				/* GPIO 7 for 3562/3062/3572 */
				if (Channel <= 14)
					Value = ((Value & 0xFFFF7FFF) | 0x00000080);
				else
					Value = (Value & 0xFFFF7F7F);
			}
			RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);

			/* enable RF tuning, this must be in the last */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R07, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x1;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R07, (UCHAR)RFValue);

			RtmpOsMsDelay(2);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, (UCHAR)0x80);

			/* latch channel for future usage */
			pAd->LatchRfRegs.Channel = Channel;
			break;
		}
	}

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

		/* Rx High power VGA offset for LNA select */
		if (pAd->NicConfig2.field.ExternalLNAForA)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x94);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x94);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}	

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

	if (pATEInfo->TxWI.BW == BW_20)
	{
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x0f);
	}
	else
	{
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x2f);
	}

	RtmpOsMsDelay(1);  

	DBGPRINT(RT_DEBUG_TRACE, ("RT35xx:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		FreqItems3572[index].N, 
		FreqItems3572[index].K, 
		FreqItems3572[index].R));
}    


INT RT35xxATETxPwrHandler(
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

		if (pATEInfo->Channel <= 14)
			RFValue = 0x60 | TxPower;
		else
			RFValue = 0xE0 | (TxPower & 0x3) | ((TxPower & 0xC) << 1);

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);
	}

	if (index == 1)
	{
		/* Set Tx1 Power */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R13, (PUCHAR)&RFValue);

		if (pATEInfo->Channel <= 14)
			RFValue = 0x60 | TxPower;
		else
			RFValue = 0xE0 | (TxPower & 0x3) | ((TxPower & 0xC) << 1);

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);
	}
	
	return 0;
}


VOID RT35xxATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	
	if (pATEInfo->Channel <= 14)
	{
		/* BG band */
		R66 = (UCHAR)(0x22 + (LNAGain*5)/3);
	}
	else 
	{
		/* A band */
		R66 = (UCHAR)(0x1C + (LNAGain*2));
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}


/* 
==========================================================================
    Description:
        Set RT35xx(RT3572, 3592, 3562, 3062) ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT35xx_Set_ATE_TX_BW_Proc(
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
		/* Set BBP R4 bit[4:3]=0:0 */
 		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
 		value &= (~0x18);
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		if (pATEInfo->Channel > 14)
		{
			value = 0x22 + (GET_LNA_GAIN(pAd)*5)/3;
		}
		else
		{
			value = 0x1C + 2*GET_LNA_GAIN(pAd);
		}				

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);		

		/* set BW = 20 MHz */
		/* R24, R31, one is for tx, the other is for rx */
		if (pATEInfo->TxWI.PHYMODE == MODE_CCK)/* CCK */
		{
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, 0x1f);
		}
		else
		{
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, 0x08);
		}
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x0f);

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

		/* Set BBP R4 bit[4:3]=1:0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		value |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		if (pATEInfo->Channel > 14)
		{
			value = 0x22 + (GET_LNA_GAIN(pAd)*5)/3;
		}
		else
		{
			value = 0x1C + 2*GET_LNA_GAIN(pAd);
		}				

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);		

		/* set BW = 40 MHz */
		/* R24, R31, one is for tx, the other is for rx */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, 0x28);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x2f);

		/* set BW = 40 MHz */
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


INT	RT35xx_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset >= 96)
	{
		DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}

	pATEInfo->RFFreqOffset = RFFreqOffset;

	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
	RFValue = ((RFValue & 0x80) | pATEInfo->RFFreqOffset);
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);

	return TRUE;
}


VOID RT35xxATEAsicSetTxRxPath(
    IN PRTMP_ADAPTER pAd)
{
	PATE_INFO   pATEInfo = &(pAd->ate);
	UCHAR	RFValue = 0, BbpValue = 0;

	/* set TX path, pATEInfo->TxAntennaSel : 0 -> All, 1 -> TX0, 2 -> TX1 */
	switch (pAd->Antenna.field.TxPath)
	{
		case 2:
			switch (pATEInfo->TxAntennaSel)
			{
				case 1:
					/* set BBP R1, bit 4:3 = 00 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
					BbpValue &= 0xE7;		/* 11100111B */
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

					/* set RF R1, bit 7:5:3 = 110 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0x57;
					RFValue = RFValue | 0xA0;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					break;
				case 2:	
					/* set BBP R1, bit 4:3 = 01 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
					BbpValue &= 0xE7;	
					BbpValue |= 0x08;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

					/* set RF R1, bit 7:5:3 = 101 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0x57;
					RFValue = RFValue | 0x88;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					break;
				default:		
					/* set BBP R1, bit 4:3 = 10 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
					BbpValue &= 0xE7;
					BbpValue |= 0x10;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
					break;
			}
			break;

		default:
			break;
	}

	/* set RX path, pATEInfo->RxAntennaSel : 0 -> All, 1 -> RX0, 2 -> RX1, 3 -> RX2 */
	switch (pAd->Antenna.field.RxPath)
	{
		case 3:
			switch (pATEInfo->RxAntennaSel)
			{
				case 1:
					/* set BBP R3, bit 4:3:1:0 = 0000 */							 
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x00;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

					/* set RF R1, bit 6:4:2 = 110 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x50;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					break;
				case 2:
					/* set BBP R3, bit 4:3:1:0 = 0001 */								
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x01;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									

					/* set RF R1, bit 6:4:2 = 101 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x44;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					break;
				case 3:	
					/* set BBP R3, bit 4:3:1:0 = 0002 */								
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x02;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

					/* set RF R1, bit 6:4:2 = 011 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x14;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					break;								
				default:	
					/* set BBP R3, bit 4:3:1:0 = 1000 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x10;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

					break;
			}
			break;

		case 2:						
			switch (pATEInfo->RxAntennaSel)
			{
				case 1:	
					/* set BBP R3, bit 4:3:1:0 = 0000 */		
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x00;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

					/* set RF R1, bit 6:4:2 = 110 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x50;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					break;
				case 2:								
					/* set BBP R3, bit 4:3:1:0 = 0001 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x01;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

					/* set RF R1, bit 6:4:2 = 101 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x44;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					break;							
				default:	
					/* set BBP R3, bit 4:3:1:0 = 0100 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x08;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

					break;						
			}
			break;

		default:
			switch (pATEInfo->RxAntennaSel)
			{
				default:
					break;
			}
			break;		
	}

}


struct _ATE_CHIP_STRUCT RALINK3062 =
{
	/* functions */
	.ChannelSwitch = RT35xxATEAsicSwitchChannel,
	.TxPwrHandler = RT35xxATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT35xxATERxVGAInit,
	.AsicSetTxRxPath = RT35xxATEAsicSetTxRxPath,
	.AdjustTxPower = NULL,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT35xx_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT35xx_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};

struct _ATE_CHIP_STRUCT RALINK3562 =
{
	/* functions */
	.ChannelSwitch = RT35xxATEAsicSwitchChannel,
	.TxPwrHandler = RT35xxATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT35xxATERxVGAInit,
	.AsicSetTxRxPath = RT35xxATEAsicSetTxRxPath,
	.AdjustTxPower = NULL,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT35xx_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT35xx_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};

struct _ATE_CHIP_STRUCT RALINK3592 =
{
	/* functions */
	.ChannelSwitch = RT35xxATEAsicSwitchChannel,
	.TxPwrHandler = RT35xxATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT35xxATERxVGAInit,
	.AsicSetTxRxPath = RT35xxATEAsicSetTxRxPath,
	.AdjustTxPower = NULL,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT35xx_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT35xx_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};

struct _ATE_CHIP_STRUCT RALINK3572 =
{
	/* functions */
	.ChannelSwitch = RT35xxATEAsicSwitchChannel,
	.TxPwrHandler = RT35xxATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT35xxATERxVGAInit,
	.AsicSetTxRxPath = RT35xxATEAsicSetTxRxPath,
	.AdjustTxPower = NULL,
	.AsicExtraPowerOverMAC = NULL,
	
	/* command handlers */
	.Set_BW_Proc = RT35xx_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT35xx_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = TRUE,
};

#endif /* RT35xx */

