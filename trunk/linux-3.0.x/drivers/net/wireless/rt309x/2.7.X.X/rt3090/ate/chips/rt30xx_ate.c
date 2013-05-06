/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt30xx_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT307x/RT309x

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT30xx

#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */


VOID RT30xxATEAsicSwitchChannel(
    	IN PRTMP_ADAPTER 		pAd)
{
	CHAR 	TxPwer = 0, TxPwer2 = 0;
	UCHAR 	index = 0, Channel = 0;
	UINT32 	Value = 0;
#ifdef A_BAND_SUPPORT
	UCHAR	BbpValue = 0;
#endif /* A_BAND_SUPPORT */
#ifdef RTMP_RF_RW_SUPPORT
	/* Added to prevent RF register reading error */
	UCHAR RFValue = 0;
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RALINK_QA
	/* For QA mode, TX power values are passed from UI */
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

	/* Fill Tx power value */
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;

	/*
		The RF programming sequence is difference between 3xxx and 2xxx.
		The 3070 is 1T1R. Therefore, we don't need to set the number of Tx/Rx path
		and the only job is to set the parameters of channels.
	*/
	if ((IS_RT30xx(pAd)) && 
		((pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_2020) ||
		(pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022) || (pAd->RfIcType == RFIC_3320)))
	{
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
				/* Programming channel parameters. */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, FreqItems3020[index].N);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xF0) | (FreqItems3020[index].K&(~0xF0));
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);
				               
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xFC) | FreqItems3020[index].R;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);

				/* Set Tx Power. */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

				/* Set RF offset. */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0x80) | pAd->ate.RFFreqOffset;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);

				/* Set BW. */
				if (pAd->ate.TxWI.BW == BW_40)
				{
					RFValue = pAd->Mlme.CaliBW40RfR24;
				}
				else
				{
					RFValue = pAd->Mlme.CaliBW20RfR24;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);

				/* Enable RF tuning */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R07, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x1;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R07, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);
				RFValue |= 0x80;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);
				RTMPusecDelay(1000);
				RFValue &= 0x7F;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);   
								
				/* Latch channel for future usage */
				pAd->LatchRfRegs.Channel = Channel;
				ATEAsicSetTxRxPath(pAd);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);
				RFValue |= 0x80;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);
				RTMPusecDelay(1000);
				RFValue &= 0x7F;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);
				break;				
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s::SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
			__FUNCTION__,
			Channel, 
			pAd->RfIcType, 
			TxPwer,
			TxPwer2,
			pAd->Antenna.field.TxPath,
			FreqItems3020[index].N, 
			FreqItems3020[index].K, 
			FreqItems3020[index].R));
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A; /* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		
		if (IS_RT3352(pAd) || IS_RT5350(pAd))
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);	
		}

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

		/* Calibration power unbalance issues */
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
#ifdef A_BAND_SUPPORT
	else
	{
	    	UINT32 TxPinCfg = 0x00050F05; /* 2007.10.09 by Brian : 0x00050505 ==> 0x00050F05 */
		
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
#endif /* A_BAND_SUPPORT */


	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

#ifndef RTMP_RF_RW_SUPPORT
	if (Channel <= 14)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%u, Pwr1=%u, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  (pAd->LatchRfRegs.R3 & 0x00003e00) >> 9,
								  (pAd->LatchRfRegs.R4 & 0x000007c0) >> 6,
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
    	}
#ifdef A_BAND_SUPPORT
	else
	{
		/* When 5.5GHz band the LSB of TxPwr will be used to reduced 7dB or not */
		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
	}
#endif /* A_BAND_SUPPORT */
#endif /* !RTMP_RF_RW_SUPPORT */
}


INT RT30xxATETxPwrHandler(
	IN PRTMP_ADAPTER 		pAd,
	IN char 					index)
{
	ULONG 	R = 0;
	UCHAR 	Bbp94 = 0;
	CHAR 	TxPower = 0;
#ifdef A_BAND_SUPPORT
	BOOLEAN bPowerReduce = FALSE;
#endif /* A_BAND_SUPPORT */
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR 	RFValue = 0;
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When Tx is controlled by QA, ATE TxPower0/1 and real Tx power
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
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Only TxPower0 and TxPower1 are adjustable\n", __FUNCTION__));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !!!\n", index));
	}

#ifdef RTMP_RF_RW_SUPPORT
	if ((IS_RT30xx(pAd) || IS_RT3390(pAd)))
	{
		/* Set Tx Power */
		UCHAR ANT_POWER_INDEX = RF_R12 + index;
		ATE_RF_IO_READ8_BY_REG_ID(pAd, ANT_POWER_INDEX, (PUCHAR)&RFValue);
		RFValue = (RFValue & 0xE0) | TxPower;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, ANT_POWER_INDEX, (UCHAR)RFValue);
		DBGPRINT(RT_DEBUG_TRACE, ("%s::(TxPower[%d]=%d, RFValue=%x)\n", __FUNCTION__, index,TxPower, RFValue));
	}
	else
#endif /* RTMP_RF_RW_SUPPORT */
	{
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

			DBGPRINT(RT_DEBUG_TRACE, ("%s::(TxPower=%d, R=%ld, BBP_R94=%d)\n", __FUNCTION__, TxPower, R, Bbp94));
		}
#ifdef A_BAND_SUPPORT
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
#endif /* A_BAND_SUPPORT */

		if (pAd->ate.Channel <= 14)
		{
			if (index == 0)
			{
				/* Shift TX power control to correct RF(R3) register bit position */
				R = R << 9;		
				R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
				pAd->LatchRfRegs.R3 = R;
			}
			else
			{
				/* Shift TX power control to correct RF(R4) register bit position */
				R = R << 6;		
				R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
				pAd->LatchRfRegs.R4 = R;
			}
		}
#ifdef A_BAND_SUPPORT
		else /* 5.5GHz */
		{
			if (bPowerReduce == FALSE)
			{
				if (index == 0)
				{
					/* Shift TX power control to correct RF(R3) register bit position */
					R = (R << 10) | (1 << 9);		
					R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
					pAd->LatchRfRegs.R3 = R;
				}
				else
				{
					/* Shift TX power control to correct RF(R4) register bit position */
					R = (R << 7) | (1 << 6);		
					R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
					pAd->LatchRfRegs.R4 = R;
				}
			}
			else
			{
				if (index == 0)
				{
					/* Shift TX power control to correct RF(R3) register bit position */
					R = (R << 10);		
					R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);

					/* Clear bit 9 of R3 to reduce 7dB */ 
					pAd->LatchRfRegs.R3 = (R & (~(1 << 9)));
				}
				else
				{
					/* Shift TX power control to correct RF(R4) register bit position */
					R = (R << 7);		
					R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);

					/* Clear bit 6 of R4 to reduce 7dB */ 
					pAd->LatchRfRegs.R4 = (R & (~(1 << 6)));
				}
			}
		}
#endif /* A_BAND_SUPPORT */

		RtmpRfIoWrite(pAd);
	}

	return 0;
}	


VOID RT30xxATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &pAd->ate;
	UCHAR R66 = 0x30;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	
	/* R66 should be set according to channel */

	if (pATEInfo->Channel <= 14) /* BG band */
	{	
		R66 = 0x2E + LNAGain;
	}
#ifdef A_BAND_SUPPORT
	else
	{	
		if (pATEInfo->TxWI.BW == BW_20) /* BW 20 */
		{
			R66 = (UCHAR)(0x32 + (LNAGain*5)/3);
		}
		else /* BW 40 */
		{
			R66 = (UCHAR)(0x3A + (LNAGain*5)/3);
		}
	}
#endif /* A_BAND_SUPPORT */

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);

}


INT	RT30xx_Set_ATE_TX_BW_Proc(
	IN PRTMP_ADAPTER		pAd, 
	IN PSTRING				arg)
{
	INT 		powerIndex;
	UCHAR 	BBPCurrentBW, value = 0;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if ((BBPCurrentBW == 0) || IS_RT2070(pAd))
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::(Warning) CCK only supports 20MHZ!!\nBandwidth switch back to 20\n", __FUNCTION__));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW 20 at BG band */
 			for (powerIndex = 0; powerIndex < MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgGBand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}
		}
#ifdef A_BAND_SUPPORT
		else
		{
			/* BW 20 at A band */
 			for (powerIndex = 0; powerIndex < MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
 				RtmpOsMsDelay(5);				
 			}
		}
#endif /* A_BAND_SUPPORT */

		/* Set BBP R4 bit[4:3]=0:0 */
 		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
 		value &= (~0x18);
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		/* Set BBP R66=0x3C */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x3C);

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR) pAd->Mlme.CaliBW20RfR24);

		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R31, &value);
		value &= (~0x20);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, value);

		pAd->LatchRfRegs.R4 &= ~0x00200000;
		RtmpRfIoWrite(pAd);

		/* Set BBP R68=0x0B to improve Rx sensitivity */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, 0x0B);
		
		/* Set BBP R69=0x16 */
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);

		/* Set BBP R70=0x08 */
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);

		/* Set BBP R73=0x11 */
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);


		/* BBP_R4 should be overwritten for every chip if the condition matched. */
		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

			if (TxMode == MODE_CCK)
			{
 				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
				value |= 0x20; /* Set bit5=1 */
 				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);				
			}
		}
	}
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			/* BW 40 at BG band */
			for (powerIndex = 0; powerIndex < MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgGBand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}
		}
#ifdef A_BAND_SUPPORT
		else
		{
			/* BW 40 at A band */
			for (powerIndex = 0; powerIndex < MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}		

			if ((pAd->ate.TxWI.PHYMODE >= 2) && (pAd->ate.TxWI.MCS == 7))
			{
    				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, 0x28);
			}
		}
#endif /* A_BAND_SUPPORT */

		/* Set BBP R4 bit[4:3]=1:0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		value |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		/* Set BBP R66=0x3C */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x3C);

		if (IS_RT30xx(pAd))
		{
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR) pAd->Mlme.CaliBW40RfR24);
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R31, &value);
			value |= 0x20;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, value);
		}
		else
		{
			pAd->LatchRfRegs.R4 |= 0x00200000;
			RtmpRfIoWrite(pAd);
		}

		/* Set BBP R68=0x0C to improve Rx sensitivity */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, 0x0C);
		
		/* Set BBP R69=0x1A */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
		
		/* Set BBP R70=0x0A */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
		
		/* Set BBP R73=0x16 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s::Current BW = %d\n", __FUNCTION__, pAd->ate.TxWI.BW));
	
	return TRUE;
}	


VOID RT30xxATEAsicSetTxRxPath(
    	IN PRTMP_ADAPTER 		pAd)
{
	UCHAR RFValue = 0, BbpValue = 0;

	if (pAd->Antenna.field.RxPath > 1)
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
	   
		RFValue = (RFValue & ~(0x17)) | 0xC1;
		BbpValue &= 0xE4;
		if (pAd->ate.RxAntennaSel == 1)
		{
			RFValue = RFValue | 0x10;
			BbpValue |= 0x00;

		}
		else if (pAd->ate.RxAntennaSel == 2)
		{
			RFValue = RFValue | 0x04;
			BbpValue |= 0x01;
		}
		else
		{
			/* Only enable two Antenna to receive */
			BbpValue |= 0x0B;
		}
					
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);  
	}
				
	if (pAd->Antenna.field.TxPath > 1)
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
	       ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
	   
		RFValue = (RFValue & ~(0x2B)) | 0xC1;
		BbpValue &= 0xE7;
		if (pAd->ate.TxAntennaSel == 1)
		{
			RFValue = RFValue | 0x20;
		}
		else if (pAd->ate.TxAntennaSel == 2)
		{
			RFValue = RFValue | 0x08;
			BbpValue |= 0x08;
		}
		else
		{
			BbpValue |= 0x10;
		}
		
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
	}
}


INT	RT30xx_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN PRTMP_ADAPTER		pAd, 
	IN PSTRING				arg)
{
	UCHAR 	RFFreqOffset = 0;
	ULONG 	R4 = 0;
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR 	RFValue = 0;
#endif /* RTMP_RF_RW_SUPPORT */
	
	RFFreqOffset = simple_strtol(arg, 0, 10);

#ifdef RTMP_RF_RW_SUPPORT
	if (RFFreqOffset >= 96)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Out of range(0 ~ 95)\n", __FUNCTION__));
		return FALSE;
	}
#else
	if (RFFreqOffset >= 64)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Out of range(0 ~ 63)\n", __FUNCTION__));
		return FALSE;
	}
#endif /* RTMP_RF_RW_SUPPORT */

	pAd->ate.RFFreqOffset = RFFreqOffset;

#ifdef RTMP_RF_RW_SUPPORT
	if (IS_RT30xx(pAd))
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
		RFValue = ((RFValue & 0x80) | pAd->ate.RFFreqOffset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);
	}
	else
#endif /* RTMP_RF_RW_SUPPORT */
	{
		R4 = pAd->ate.RFFreqOffset << 15;		
		R4 |= (pAd->LatchRfRegs.R4 & ((~0x001f8000)));
		pAd->LatchRfRegs.R4 = R4;
		RtmpRfIoWrite(pAd);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s::RFFreqOffset = %d\n", __FUNCTION__, pAd->ate.RFFreqOffset));
	
	return TRUE;
}

VOID RT30xx_ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER 		pAd) 
{
	BBP_R49_STRUC	BbpR49;
	BOOLEAN			bAutoTxAgc = FALSE;
	UCHAR			TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep, idx;
	PCHAR			pTxAgcCompensate = NULL;
	ULONG			TxPwr[5];
	CHAR    			DeltaPwr = 0, Value = 0;
	INT				i, j, maxTxPwrCnt = 5;
	
	BbpR49.byte = 0;
	
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
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);

		/* TSSI representation */
		if (IS_RT3071(pAd) || IS_RT3390(pAd) || IS_RT3090A(pAd) || IS_RT3572(pAd)) /* 5-bits */
		{
			BbpR49.byte = (BbpR49.byte & 0x1F);
		}
		
		/* (p) TssiPlusBoundaryG[0] = 0 = (m) TssiMinusBoundaryG[0] */
		/* compensate: +4     +3   +2   +1    0   -1   -2   -3   -4 * steps */
		/* step value is defined in pAd->TxAgcStepG for tx power value */

		/* [4]+1+[4]   p4     p3   p2   p1   o1   m1   m2   m3   m4 */
		/* ex:         0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
		   above value are examined in mass factory production */
		/*             [4]    [3]  [2]  [1]  [0]  [1]  [2]  [3]  [4] */

		/* plus (+) is 0x00 ~ 0x45, minus (-) is 0xa0 ~ 0xf0 */
		/* if value is between p1 ~ o1 or o1 ~ s1, no need to adjust tx power */
		/* if value is 0xa5, tx power will be -= TxAgcStep*(2-1) */

		if (BbpR49.byte > pTssiMinusBoundary[1])
		{
			/* Reading is larger than the reference value */
			/* Check for how large we need to decrease the Tx power */
			for (idx = 1; idx < 5; idx++)
			{
				if (BbpR49.byte <= pTssiMinusBoundary[idx]) /* Found the range */
					break;
			}
			/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */
			*pTxAgcCompensate = -(TxAgcStep * (idx-1));	
			DeltaPwr += (*pTxAgcCompensate);
			DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
				                BbpR49.byte, TssiRef, TxAgcStep, idx-1));                    
		}
		else if (BbpR49.byte < pTssiPlusBoundary[1])
		{
			/* Reading is smaller than the reference value */
			/* Check for how large we need to increase the Tx power */
			for (idx = 1; idx < 5; idx++)
			{
				if (BbpR49.byte >= pTssiPlusBoundary[idx]) /* Found the range */
					break;
			}
			/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
			*pTxAgcCompensate = TxAgcStep * (idx-1);
			DeltaPwr += (*pTxAgcCompensate);
			DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
				                BbpR49.byte, TssiRef, TxAgcStep, idx-1));
		}
		else
		{
			*pTxAgcCompensate = 0;
			DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
				                BbpR49.byte, TssiRef, TxAgcStep, 0));
		}
	}


	/* Set new Tx power for different Tx rates */
	for (i = 0; i < maxTxPwrCnt; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j = 0; j < 8; j++)
			{
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F);

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

				/* Fill new value into the corresponding MAC offset */
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);

			DBGPRINT(RT_DEBUG_TRACE, ("%s::After compensation, Offset = 0x%04X, RegisterValue = 0x%08lX\n",
				__FUNCTION__, TX_PWR_CFG_0 + (i << 2), TxPwr[i]));			
		}
	}
}


struct _ATE_CHIP_STRUCT RALINK30xx =
{
	/* Functions */
	.ChannelSwitch = RT30xxATEAsicSwitchChannel,
	.TxPwrHandler = RT30xxATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT30xxATERxVGAInit,
	.AsicSetTxRxPath = RT30xxATEAsicSetTxRxPath,
	.AdjustTxPower = RT30xx_ATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = NULL,

	/* Command handlers */	
	.Set_BW_Proc = RT30xx_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT30xx_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* Variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,	
	.bBBPStoreTXCONT = TRUE,
	.bBBPLoadATESTOP = TRUE,
};

#endif /* RT30xx */
