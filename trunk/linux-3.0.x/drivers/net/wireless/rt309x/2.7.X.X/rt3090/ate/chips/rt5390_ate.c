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
	rt5390_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT5370/RT5390/RT5372/RT5392

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)

#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

/* RT5390 RF for ATE OFDM */
const REG_PAIR RF5390ATEReg_OFDM[] =
{
	{RF_R32, 	0x80},
	{RF_R53, 	0x04},
	{RF_R55, 	0x43},
};
UCHAR NUM_RF5390ATEReg_OFDM = (sizeof(RF5390ATEReg_OFDM) / sizeof(REG_PAIR));

/* RT5390 RF for ATE CCK */
const REG_PAIR RF5390ATEReg_CCK[] =
{
	{RF_R32, 	0xC0},
	{RF_R53, 	0x04},
	{RF_R55, 	0x46},
};
UCHAR NUM_RF5390ATEReg_CCK = (sizeof(RF5390ATEReg_CCK) / sizeof(REG_PAIR));

/* RT5390/92 RF for ATE BW */
extern REG_PAIR_BW RF539xReg_BW[];
extern UCHAR NUM_RF539xReg_BW;

/* RT5392 BBP for ATE BW */
extern REG_PAIR_BW BBP5392Reg_BW[];
extern UCHAR NUM_BBP5392Reg_BW;

/* RT5392 RF for ATE OFDM */
extern REG_PAIR RF5392Reg_OFDM[];
extern UCHAR NUM_RF5392Reg_OFDM;

/* RT5392 RF for ATE CCK */
extern REG_PAIR RF5392Reg_CCK[];
extern UCHAR NUM_RF5392Reg_CCK;


#ifdef IQ_CAL_SUPPORT
VOID RT5390_ATEIQCompensation(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR 				Channel)
{
	PATE_INFO pATEInfo = &pAd->ate;
	INT 		BBPIndex = 0;
	UINT16 	E2PValue = 0;

	if (pATEInfo->bIQCompEnable == TRUE)
	{
		/* IQ Calibration */
		if (Channel <= 14)
		{
			for (BBPIndex = 0; BBPIndex <= 7; BBPIndex++)
			{
				RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GLOBAL_BBP_ACCESS_BASE + (BBPIndex * 2), E2PValue);
				
				if (E2PValue == 0xFFFF)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s::EEPROM 0x%X is not calibrated !\n", __FUNCTION__, 
						(EEPROM_IQ_GLOBAL_BBP_ACCESS_BASE + (BBPIndex * 2))));
					continue;
				}
				else
				{
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, (UCHAR)((E2PValue >> 8) & 0x00FF), (UCHAR)(E2PValue & 0x00FF));
				}
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s::IQ compensation is done\n", __FUNCTION__));
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("%s::5 GHz band not supported !\n", __FUNCTION__));
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("%s::Not support IQ compensation since not RT5390 or not enabled !\n", __FUNCTION__));
}

VOID RT5392_ATEIQCompensation(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR 				Channel)
{
	PATE_INFO pATEInfo = &pAd->ate;
	UCHAR  	BBPValue = 0;
	UINT16  	E2PValue = 0;	
	
	if (pATEInfo->bIQCompEnable == TRUE)
	{
		/* IQ Calibration */
		if (Channel <= 14)
		{
			/* TX0 IQ Gain */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2C);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX0, IQ_CAL_GAIN);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			
			/* TX0 IQ Phase */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2D);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX0, IQ_CAL_PHASE);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

			/* TX1 IQ Gain */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4A);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX1, IQ_CAL_GAIN);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			
			/* TX1 IQ Phase */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4B);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX1, IQ_CAL_PHASE);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

			/* RF IQ Compensation Control */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x04);
			RT28xx_EEPROM_READ16(pAd, EEPROM_RF_IQ_COMPENSATION_CONTROL, E2PValue);
			if ((E2PValue & 0x00FF) == 0x00FF)
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0); /* IQ Compensation disabled */
			else
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, E2PValue & 0x00FF);

			/* RF IQ Imbalance Compensation Control */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x03);
			RT28xx_EEPROM_READ16(pAd, EEPROM_RF_IQ_IMBALANCE_COMPENSATION_CONTROL, E2PValue);
			if ((E2PValue & 0x00FF) == 0x00FF)
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0); /* IQ Imbalance Compensation disabled */
			else
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, E2PValue & 0x00FF);

			DBGPRINT(RT_DEBUG_TRACE, ("%s::IQ compensation is done\n", __FUNCTION__));
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("%s::5 GHz band not supported !\n", __FUNCTION__));
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("%s::Not support IQ compensation since not RT5392 or not enabled !\n", __FUNCTION__));
}
#endif /* IQ_CAL_SUPPORT */

VOID RT539x_ATEAsicSwitchChannel(
    	IN PRTMP_ADAPTER 		pAd)
{
	UINT 	i = 0;
	CHAR 	TxPwer = 0, TxPwer2 = 0;
	UCHAR 	index = 0, Channel = 0, TxRxh20M = 0, RefFreqOffset;
	UINT32 	Value = 0;
#ifdef RTMP_RF_RW_SUPPORT
	/* Added to prevent RF register reading error */
	UCHAR 	RFValue = 0; 
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
	
	for (index = 0; index < NUM_OF_3020_CHNL; index++)
	{
		if (Channel == FreqItems3020[index].Channel)
		{
			/* Set the BBP Tx fine power control in 0.1dB step */			

			/* Programming channel parameters */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, FreqItems3020[index].N); /* N */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, (FreqItems3020[index].K & 0x0F)); /* K, N<11:8> is set to zero */

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R11, (PUCHAR)&RFValue);
			RFValue = ((RFValue & ~0x03) | (FreqItems3020[index].R & 0x03)); /* R */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)&RFValue);
			RFValue = ((RFValue & ~0x3F) | (TxPwer & 0x3F)); /* tx0_alc */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);

			if (IS_RT5392(pAd))
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R50, &RFValue);
				RFValue = ((RFValue & ~0x3F) | (TxPwer2 & 0x3F)); /* tx0_alc */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R50, RFValue);
			}

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
			if (IS_RT5392(pAd))
			{
				RFValue = ((RFValue & ~0x3F) | 0x3F);
			}
			else
			{
				RFValue = ((RFValue & ~0x0F) | 0x0F); /* Enable rf_block_en, pll_en, rx0_en and tx0_en */
			}	
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);

			if (!IS_RT5392(pAd))
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R02, (PUCHAR)&RFValue);
				RFValue |= 0x80;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, (UCHAR)RFValue);
				
				RTMPusecDelay(1000);
				
				RFValue &= 0x7F;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, (UCHAR)RFValue);   
			}

			if (IS_RT5392(pAd))
			{
				ATEAsicSetTxRxPath(pAd);
			}

			RefFreqOffset = pAd->ate.RFFreqOffset;
			RTMPAdjustFrequencyOffset(pAd, &RefFreqOffset);

			if (pAd->ate.TxWI.BW == BW_20) /* BW 20 */
			{
				TxRxh20M = ((pAd->Mlme.CaliBW20RfR24 & 0x20) >> 5); /* Tx/Rx h20M */
				
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, 0x40);
			}
			else if (pAd->ate.TxWI.BW == BW_40) /* BW 40 */
			{
				TxRxh20M = ((pAd->Mlme.CaliBW40RfR24 & 0x20) >> 5); /* Tx/Rx h20M */
				
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, 0x50);
			}

#ifdef RTMP_MAC_PCI
			if (IS_RT5392(pAd)) /* For RT5392 */
			{
				/* do nothing */
			}
			else if (IS_RT5390H(pAd)) /* For RT5390H */
			{
				if ((Channel >= 1) && (Channel <= 3))
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x0F);
				}
				else if ((Channel >= 4) && (Channel <= 5))
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x0E);
				}
				else if (Channel == 6)
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x0C);
				}
				else if (Channel == 7)
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x0B);
				}
				else if ((Channel >= 8) && (Channel <= 11))
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x09);
				}
				else if ((Channel >= 12) && (Channel <= 13))
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x08);
				}
				else if (Channel == 14)
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x06);
				}
			}
			else if (IS_RT5390F(pAd)) /* For >= RT5390F */
			{
				if ((Channel >= 1) && (Channel <=10))
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x07);
				}
				else if (Channel == 11)
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x06);
				}
				else if (Channel == 12)
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x05);
				}
				else if ((Channel >= 13) && (Channel <=14))
				{							
					RT30xxWriteRFRegister(pAd, RF_R59, 0x04);
				}
			}
			else if (IS_RT5390(pAd)) /* For RT5390 */
			{
				if ((Channel >= 1) && (Channel <= 7))
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x8B);
				}
				else if (Channel == 8)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x8A);
				}
				else if (Channel == 9)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x89);
				}
				else if ((Channel >= 10) && (Channel <= 11))
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x88);
				}
				else if (Channel == 12)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x86);
				}
				else if (Channel == 13)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x85);
				}
				else if (Channel == 14)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x84);
				}
			}
#endif /* RTMP_MAC_PCI */


			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
			/* vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration.*/
			RFValue = ((RFValue & ~0x80) | 0x80); 
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);					

			DBGPRINT(RT_DEBUG_TRACE, ("%s::SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
				__FUNCTION__, 
				Channel, 
				pAd->RfIcType, 
				TxPwer, 
				TxPwer2, 
				pAd->ate.TxAntennaSel, 
				FreqItems3020[index].N, 
				FreqItems3020[index].K, 
				FreqItems3020[index].R));

			break;
		}
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A; /* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		/* According the Rory's suggestion to solve the middle range issue */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);	
		

		/* 2.4 G band selection PIN */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Turn off unused PA or LNA when only 1T or 1R */
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		/* Calibration power unbalance issue */
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
	else /* channel > 14 */
	{
		UINT32 	TxPinCfg = 0x00050F05; /* 2007.10.09 by Brian : 0x00050505 ==> 0x00050F05 */
		UCHAR 	BbpValue = 0;
		
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		/* According the Rory's suggestion to solve the middle range issue */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);        
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0xF2);


		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R91, &BbpValue);
		ASSERT((BbpValue == 0x04));

		/* 5 G band selection PIN, bit1 and bit2 are complement */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Turn off unused PA or LNA when only 1T or 1R */
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

	if (IS_RT5390(pAd))
	{
		if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
		{
			/* RF for ATE CCK */
			for (i = 0; i < NUM_RF5390ATEReg_CCK; i++)
				RT30xxWriteRFRegister(pAd, RF5390ATEReg_CCK[i].Register, RF5390ATEReg_CCK[i].Value);
		} 
		else
		{
			/* RF for ATE OFDM */
			for (i = 0; i < NUM_RF5390ATEReg_OFDM; i++)
				RT30xxWriteRFRegister(pAd, RF5390ATEReg_OFDM[i].Register, RF5390ATEReg_OFDM[i].Value);
		}
	}
	else if (IS_RT5392(pAd))
	{
		if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
		{
			/* RF for ATE CCK */
			for (i = 0; i < NUM_RF5392Reg_CCK; i++)
				RT30xxWriteRFRegister(pAd, RF5392Reg_CCK[i].Register, RF5392Reg_CCK[i].Value);
		} 
		else
		{
			/* RF for ATE OFDM */
			for (i = 0; i < NUM_RF5392Reg_OFDM; i++)
				RT30xxWriteRFRegister(pAd, RF5392Reg_OFDM[i].Register, RF5392Reg_OFDM[i].Value);
		}

		/* BBP for ATE BW */
		for (i = 0; i < NUM_BBP5392Reg_BW; i++)
		{
			if (pAd->ate.TxWI.BW == BBP5392Reg_BW[i].BW)
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP5392Reg_BW[i].Register, BBP5392Reg_BW[i].Value);
		}
	}

	/* RT5390/92 RF for ATE BW */
	for (i = 0; i < NUM_RF539xReg_BW; i++)
	{
		if (pAd->ate.TxWI.BW == RF539xReg_BW[i].BW)
			RT30xxWriteRFRegister(pAd, RF539xReg_BW[i].Register, RF539xReg_BW[i].Value);
	}
	
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38);
	
	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

#ifdef IQ_CAL_SUPPORT
	if (IS_RT5390(pAd))
	{
		RT5390_ATEIQCompensation(pAd, pAd->ate.Channel);
	}
	else if (IS_RT5392(pAd))
	{
		RT5392_ATEIQCompensation(pAd, pAd->ate.Channel);	
	}
#endif /* IQ_CAL_SUPPORT */

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

INT RT539x_ATETxPwrHandler(
	IN PRTMP_ADAPTER 		pAd,
	IN char 					index)
{
	ULONG 	R;
	UCHAR 	Bbp94 = 0;
	CHAR 	TxPower = 0;
#ifdef A_BAND_SUPPORT
	BOOLEAN	bPowerReduce = FALSE;
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
	if (IS_RT5390(pAd) || IS_RT5392(pAd))
	{
		UCHAR ANT_POWER_INDEX = RF_R49 + index;

		ATE_RF_IO_READ8_BY_REG_ID(pAd, ANT_POWER_INDEX, (PUCHAR)&RFValue);
		RFValue = ((RFValue & ~0x3F) | (TxPower & 0x3F)); /* tx0_alc */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, ANT_POWER_INDEX, (UCHAR)RFValue);
		
		if (IS_RT5390(pAd))	
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, 0x04);
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

			DBGPRINT(RT_DEBUG_TRACE, ("%s::(TxPower=%d, R=%lu)\n", __FUNCTION__, TxPower, R));
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

VOID RT539x_ATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &pAd->ate;
	UCHAR R66 = 0x30;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	
	/* R66 should be set according to channel */
	
	if (pATEInfo->Channel <= 14) /* BG band */
	{	
		if (pATEInfo->TxWI.BW == BW_20) /* BW 20 */
		{
			R66 = LNAGain*2 + 0x1C;
		}
		else /* BW 40 */
		{
			if (IS_RT5390(pAd))
			{
				R66 = LNAGain*2 + 0x24;
			}
			else
			{
				R66 = LNAGain*2 + 0x1C;
			}
		}
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

VOID RT5392_ATEAsicSetTxRxPath(
	IN PRTMP_ADAPTER 		pAd)
{
	UCHAR BbpValue = 0, RFValue = 0x03;

	/* Set TX path, pAd->TxAntennaSel : 0 -> All, 1 -> TX0, 2 -> TX1 */
	switch (pAd->Antenna.field.TxPath)
	{
		case 2:
			switch (pAd->ate.TxAntennaSel)
			{
				case 1:
					/* Set BBP R1, bit 4:3 = 00 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
					BbpValue &= 0xE7; /* 11100111B */
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

					/* Set RF R1, bit 7:5:3 = 001 */					
					RFValue &=  ~TXPowerEnMask;
					RFValue = RFValue | 0x08;
					break;
				case 2:	
					/* Set BBP R1, bit 4:3 = 01 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
					BbpValue &= 0xE7;	
					BbpValue |= 0x08;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

					/* Set RF R1, bit 7:5:3 = 010 */					
					RFValue &=  ~TXPowerEnMask;
					RFValue = RFValue | 0x20;
					break;
				default:		
					/* Set BBP R1, bit 4:3 = 10 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
					BbpValue &= 0xE7;
					BbpValue |= 0x10;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

					/* Set RF R1, bit 7:5:3 = 011 */
					RFValue &=  ~TXPowerEnMask;
					RFValue = RFValue | 0x28;
					break;						
			}
			break;
	default:
			/* Set RF R1, bit 7:5:3 = 011 */
			RFValue &=  ~TXPowerEnMask;
			RFValue = RFValue | 0x28;
			break;
	}

	/* Set RX path, pAd->RxAntennaSel : 0 -> All, 1 -> RX0, 2 -> RX1, 3 -> RX2 */
	switch (pAd->Antenna.field.RxPath)
	{
		case 3:
			switch (pAd->ate.RxAntennaSel)
			{
				case 1:
					/* Set BBP R3, bit 4:3:1:0 = 0000 */							
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x00;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

					/* Set RF R1, bit 6:4:2 = 110 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x50;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
					break;
				case 2:
					/* Set BBP R3, bit 4:3:1:0 = 0001 */							
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x01;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									

					/* Set RF R1, bit 6:4:2 = 101 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x44;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
					break;
				case 3:	
					/* Set BBP R3, bit 4:3:1:0 = 0002 */								
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x02;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

					/* Set RF R1, bit 6:4:2 = 011 */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
					RFValue = RFValue & 0xAB;
					RFValue = RFValue | 0x14;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
					break;								
				default:	
					/* Set BBP R3, bit 4:3:1:0 = 1000 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x10;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

					break;
			}
			break;

		case 2:						
			switch (pAd->ate.RxAntennaSel)
			{
				case 1:	
					/* Set BBP R3, bit 4:3:1:0 = 0000 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x00;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

					/* Set RF R1, bit 6:4:2 = 001 */				
					RFValue &= ~RXPowerEnMask;
					RFValue |= 0x04;
					break;
				case 2:								
					/* Set BBP R3, bit 4:3:1:0 = 0001 */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x01;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

					/* Set RF R1, bit 6:4:2 = 010 */
					RFValue &= ~RXPowerEnMask;
					RFValue |= 0x10;
					break;							
				default:
					/* Set BBP R3, bit 4:3:1:0 = 0100 */		
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
					BbpValue &= 0xE4;
					BbpValue |= 0x08;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

					/* Set RF R1, bit 6:4:2 = 011 */
					RFValue &=  ~RXPowerEnMask;
					RFValue |= 0x14;
				break;
			}
			break;		

		default:
			/* Set RF R1, bit 6:4:2 = 011 */
			RFValue &=  ~RXPowerEnMask;
			RFValue |= 0x14;			
			break;		
	}

	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
}

VOID RT539x_ATEAsicExtraPowerOverMAC(
	IN PRTMP_ADAPTER 		pAd)
{
	ULONG ExtraPwrOverMAC = 0;
	ULONG ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg8 = 0, ExtraPwrOverTxPwrCfg9 = 0;

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

/* Vx = V0 + t(V1 - V0) ? f(x), where t = (x-x0) / (x1 - x0) */
CHAR RTATEInsertTssi(UCHAR InChannel, UCHAR Channel0, UCHAR Channel1, CHAR Tssi0, CHAR Tssi1)
{
	CHAR InTssi, TssiDelta, ChannelDelta, InChannelDelta;
	
	ChannelDelta = Channel1 - Channel0;
	InChannelDelta = InChannel - Channel0;
	TssiDelta = Tssi1 - Tssi0;

	/* channel delta should not be 0 */
	if (ChannelDelta == 0)
		InTssi = Tssi0;

	DBGPRINT(RT_DEBUG_WARN, ("--->%s\n", __FUNCTION__)); 	
	
	if ((TssiDelta > 0) && (((InChannelDelta * TssiDelta * 10) / ChannelDelta) % 10 >= 5))
	{
		InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);
		InTssi += 1;
	}
	else	if ((TssiDelta < 0) && (((InChannelDelta * TssiDelta * 10) / ChannelDelta) % 10 <= -5))
	{
		InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);
		InTssi -= 1;
	}
	else
	{
		InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);	
	}	

	DBGPRINT(RT_DEBUG_WARN, ("<---%s\n", __FUNCTION__)); 		
	
	return InTssi;
}

UCHAR RTATEGetTssiByChannel(PRTMP_ADAPTER pAd, UCHAR Channel)
{
	UINT	i = 0;
	UCHAR	BbpData =0;
	UCHAR	ChannelPower;
	UCHAR 	BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	USHORT	EEPData;
	BBP_R47_STRUC BBPR47;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPR47.byte);
	BBPR47.field.Adc6On = 1;
	BBPR47.field.TssiMode = 0x02;
	BBPR47.field.TssiUpdateReq = 1;
	BBPR47.field.TssiReportSel = 0;							
	DBGPRINT(RT_DEBUG_WARN, ("Write BBP R47 = 0x%x\n", BBPR47.byte));
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPR47.byte);
		
	/* start TX at 54Mbps */
	/* NdisZeroMemory(&pAd->ate, sizeof(ATE_INFO)); */
	pAd->ate.TxCount = 100;
	pAd->ate.TxLength = 1024;
	pAd->ate.Channel = Channel;
	COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
	COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
	COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);    		

	Set_ATE_TX_MODE_Proc(pAd, "1");		/* MODE_OFDM */
	Set_ATE_TX_MCS_Proc(pAd, "7");		/* 54Mbps */
	Set_ATE_TX_BW_Proc(pAd, "0");		/* 20MHz */
		
	/* read calibrated channel power value from EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET+Channel-1, ChannelPower);
	pAd->ate.TxPower0 = (UCHAR)(ChannelPower & 0xff);
	DBGPRINT(RT_DEBUG_TRACE, ("Channel %d, Calibrated Tx.Power0= 0x%x\n", Channel, pAd->ate.TxPower0));
	
	/* read frequency offset from EEPROM */                        
	RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
	pAd->ate.RFFreqOffset = (UCHAR)(EEPData & 0xff);
		
	Set_ATE_Proc(pAd, "TXFRAME"); 
	RTMPusecDelay(200000);

	while (i < 500)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpData);

		if ((BbpData & 0x04) == 0)
			break;

		RTMPusecDelay(2);
		i++;	
	}

	if (i >= 500)
		DBGPRINT(RT_DEBUG_WARN, ("TSSI status not ready!!! (i=%d)\n", i));

	/* read BBP R49[6:0] and write to EEPROM 0x6E */
	DBGPRINT(RT_DEBUG_WARN, ("Read  BBP_R49\n")); 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
	DBGPRINT(RT_DEBUG_WARN, ("BBP R49 = 0x%x\n", BbpData)); 
	BbpData &= 0x7f;

	/* the upper boundary of 0x6E (TSSI base) is 0x7C */
	if (BbpData > 0x7C)
		BbpData = 0;

	/* back to ATE IDLE state */
	Set_ATE_Proc(pAd, "ATESTART");

	return BbpData;	
}

/* Get the power delta bound */
#define GET_TSSI_RATE_TABLE_INDEX(x) (((x) > UPPER_POWER_DELTA_INDEX) ? (UPPER_POWER_DELTA_INDEX) : (((x) < LOWER_POWER_DELTA_INDEX) ? (LOWER_POWER_DELTA_INDEX) : ((x))))

CHAR GetPowerDeltaFromTssiRatio(CHAR TssiOfChannel, CHAR TssiBase)
{
	LONG	TssiRatio, TssiDelta, MinTssiDelta;
	CHAR	i, PowerDeltaStatIndex, PowerDeltaEndIndex, MinTssiDeltaIndex, PowerDelta;	
	extern ULONG TssiRatioTable[][2];

	// TODO: If 0 is a valid value for TSSI base
	if (TssiBase == 0)
		return 0;
	
	TssiRatio = TssiOfChannel * TssiRatioTable[0][1] / TssiBase;

	DBGPRINT(RT_DEBUG_WARN, ("TssiOfChannel = %d, TssiBase = %d, TssiRatio = %ld\n", TssiOfChannel,  TssiBase, TssiRatio));

	PowerDeltaStatIndex = 4;
	PowerDeltaEndIndex = 19;

	MinTssiDeltaIndex= PowerDeltaStatIndex;
	MinTssiDelta = TssiRatio - TssiRatioTable[MinTssiDeltaIndex][0];
	
	if (MinTssiDelta < 0)
		MinTssiDelta = -MinTssiDelta;

	for (i = PowerDeltaStatIndex+1; i <= PowerDeltaEndIndex; i++)
	{
		TssiDelta = TssiRatio -TssiRatioTable[i][0];
		
		if (TssiDelta < 0)
		{
			TssiDelta = -TssiDelta;
		}

		if (TssiDelta < MinTssiDelta)
		{
			MinTssiDelta = TssiDelta;
			MinTssiDeltaIndex = i;
		}
	}

	PowerDelta = MinTssiDeltaIndex - TSSI_RATIO_TABLE_OFFSET;

	DBGPRINT(RT_DEBUG_WARN, ("MinTssiDeltaIndex = %d, MinTssiDelta = %ld, PowerDelta = %d\n", MinTssiDeltaIndex,  MinTssiDelta, PowerDelta));
	
	return (PowerDelta);
}

INT RT5390_ATETssiCalibration(
	IN PRTMP_ADAPTER		pAd,
	IN PSTRING				arg)
{    	
	UINT 		i = 0;
	UCHAR		BbpData = 0, RFValue, OrgBbp47Value, inputDAC;
	USHORT		EEPData = 0;
	UCHAR 		BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	BBP_R47_STRUC	BBPR47;

	BBPR47.byte = 0;
	
	inputDAC = simple_strtol(arg, 0, 10);

	if (!IS_RT5390(pAd) || !(pAd->TxPowerCtrl.bInternalTxALC))                          
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Not support TSSI calibration since not 5390 chip or EEPROM not set!!!\n"));
		return FALSE;
	}

	/* Set RF R27[3:0] TSSI gain */		
	RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)(&RFValue));			
	RFValue = ((RFValue & 0xF0) | pAd->TssiGain); /* [3:0] = (tssi_gain and tssi_atten) */
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);	

	/* Set RF R28 bit[7:6] = 00 */
	RT30xxReadRFRegister(pAd, RF_R28, &RFValue);
	/* RF28Value = RFValue; */
	RFValue &= (~0xC0); 
	RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

	/* Set BBP R47[7] = 1(ADC6 ON), R47[4:3] = 0x2(new average TSSI mode), R47[2] = 1(TSSI_UPDATE_REQ), R49[1:0] = 0(TSSI info 0 - TSSI) */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPR47.byte);
	OrgBbp47Value = BBPR47.byte;
	BBPR47.field.Adc6On = 1;
	BBPR47.field.TssiMode = 0x02;
	BBPR47.field.TssiUpdateReq = 1;
	BBPR47.field.TssiReportSel = 0;							
	DBGPRINT(RT_DEBUG_TRACE, ("Write BBP R47 = 0x%x\n", BBPR47.byte));
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPR47.byte);		

	/* NdisZeroMemory(&pAd->ate, sizeof(struct _ATE_INFO)); */
	pAd->ate.TxCount = 100;
	pAd->ate.TxLength = 1024;
	 pAd->ate.Channel = 1;
	COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
	COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
	COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);    

	Set_ATE_TX_MODE_Proc(pAd, "1");		/* MODE_OFDM */
	Set_ATE_TX_MCS_Proc(pAd, "7");		/* 54Mbps */
	Set_ATE_TX_BW_Proc(pAd, "0");		/* 20MHz */
			
	if (inputDAC > 1)
	{	
		/* Set power value calibrated DAC */
		pAd->ate.TxPower0 = inputDAC;		
	}
	else
	{
		/* Read calibrated channel power value from EEPROM */
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET, EEPData);
		pAd->ate.TxPower0 = (UCHAR)(EEPData & 0xff);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("(Calibrated) Tx.Power0= 0x%x\n", pAd->ate.TxPower0));
		
	/* Read frequency offset from EEPROM */                       
	RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
	pAd->ate.RFFreqOffset = (UCHAR)(EEPData & 0xff);
		
	Set_ATE_Proc(pAd, "TXFRAME"); 
	RTMPusecDelay(200000);

	while (i < 500)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpData);

		if ((BbpData & 0x04) == 0)
			break;

		RTMPusecDelay(2);
		i++;	
	}

	if (i >= 500)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI status not ready!!! (i=%d)\n", i));
		return FALSE;
	}	

	/* Read BBP R49[6:0] and write to EEPROM 0x6E */
	DBGPRINT(RT_DEBUG_TRACE, ("Read  BBP_R49\n")); 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
	DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%x\n", BbpData)); 
	BbpData &= 0x7f;

	/* The upper boundary of 0x6E (TSSI base) is 0x7C */
	if (BbpData > 0x7C)
		BbpData = 0;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
	EEPData &= 0xff00;
	EEPData |= BbpData;
	DBGPRINT(RT_DEBUG_TRACE, ("Write  E2P 0x6e: 0x%x\n", EEPData)); 		
	RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
	RTMPusecDelay(10);

	/* Restore RF R27 and R28, BBP R47 */
	/* RT30xxWriteRFRegister(pAd, RF_R27, RF27Value); */				
	/* RT30xxWriteRFRegister(pAd, RF_R28, RF28Value); */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, OrgBbp47Value);

	Set_ATE_Proc(pAd, "ATESTART");

	return TRUE;
}

INT RT5390_ATETssiCalibrationExtend(
	IN PRTMP_ADAPTER		pAd,
	IN PSTRING				arg)
{  
	UCHAR inputData;
	
	inputData = simple_strtol(arg, 0, 10);
	
	if (!(IS_RT5390(pAd) && (pAd->TxPowerCtrl.bInternalTxALC) && (pAd->TxPowerCtrl.bExtendedTssiMode)))			
	{
		DBGPRINT(RT_DEBUG_WARN, ("Not support TSSI calibration since not 5390 chip or EEPROM not set!!!\n"));
		return FALSE;
	}	
	else
	{				
		UCHAR	RFValue;
		CHAR	TssiRefPerChannel[14+1], PowerDeltaPerChannel[14+1], TssiBase;
		USHORT	EEPData;
		UCHAR	CurrentChannel;

		/* step 0: set init register values for TSSI calibration */
		/* Set RF R27[3:2] = 00, R27[1:0] = 11 */
		RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
		/* RF27Value = RFValue; */
		/* RFValue &= (~0x0F); */
		/* RFValue |= 0x02; */ 
		RFValue = ((RFValue & 0xF0) | pAd->TssiGain); /* [3:0] = (tssi_gain and tssi_atten) */
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

		/* Set RF R28 bit[7:6] = 00 */
		RT30xxReadRFRegister(pAd, RF_R28, &RFValue);
		/* RF28Value = RFValue; */
		RFValue &= (~0xC0); 
		RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

		/* step 1: get channel 7 TSSI as reference value */
		CurrentChannel = 7;
		TssiRefPerChannel[CurrentChannel] = RTATEGetTssiByChannel(pAd, CurrentChannel);
		TssiBase = TssiRefPerChannel[CurrentChannel];
		PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase);

		/* Save TSSI ref base to EEPROM 0x6E */
		RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
		EEPData &= 0xff00;
		EEPData |= TssiBase;
		DBGPRINT(RT_DEBUG_WARN, ("Write  E2P 0x6E: 0x%x\n", EEPData)); 
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
		RTMPusecDelay(10); /* delay for twp(MAX)=10ms */
		
		/* step 2: get channel 1 and 13 TSSI values */
		/* start TX at 54Mbps */
		CurrentChannel = 1;
		TssiRefPerChannel[CurrentChannel] = RTATEGetTssiByChannel(pAd, CurrentChannel);
		PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase);

		/* start TX at 54Mbps */
		CurrentChannel = 13;
		TssiRefPerChannel[CurrentChannel] = RTATEGetTssiByChannel(pAd, CurrentChannel);
		PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase);

		/* step 3: insert the power table */
		/* insert channel 2 to 6 TSSI values */
		/*
			for(CurrentChannel = 2; CurrentChannel <7; CurrentChannel++)
				TssiRefPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 1, 7, TssiRefPerChannel[1], TssiRefPerChannel[7]);
		*/
		for (CurrentChannel = 2; CurrentChannel < 7; CurrentChannel++)
			PowerDeltaPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 1, 7, PowerDeltaPerChannel[1], PowerDeltaPerChannel[7]);

		/* insert channel 8 to 12 TSSI values */
		/*
			for(CurrentChannel = 8; CurrentChannel < 13; CurrentChannel++)
				TssiRefPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 7, 13, TssiRefPerChannel[7], TssiRefPerChannel[13]);
		*/
		for (CurrentChannel = 8; CurrentChannel < 13; CurrentChannel++)
			PowerDeltaPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 7, 13, PowerDeltaPerChannel[7], PowerDeltaPerChannel[13]);


		/* channel 14 TSSI equals channel 13 TSSI */
		/* TssiRefPerChannel[14] = TssiRefPerChannel[13]; */
		PowerDeltaPerChannel[14] = PowerDeltaPerChannel[13];

		for (CurrentChannel = 1; CurrentChannel <= 14; CurrentChannel++)
		{
			DBGPRINT(RT_DEBUG_WARN, ("Channel %d, PowerDeltaPerChannel= 0x%x\n", CurrentChannel, PowerDeltaPerChannel[CurrentChannel]));
		
			/* PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase); */

			/* boundary check */
			if (PowerDeltaPerChannel[CurrentChannel] > 7)
				PowerDeltaPerChannel[CurrentChannel] = 7;
			if (PowerDeltaPerChannel[CurrentChannel] < -8)
				PowerDeltaPerChannel[CurrentChannel] = -8;

			/* eeprom only use 4 bit for TSSI delta */
			PowerDeltaPerChannel[CurrentChannel] &= 0x0f;
			DBGPRINT(RT_DEBUG_WARN, ("Channel = %d, PowerDeltaPerChannel=0x%x\n", CurrentChannel, PowerDeltaPerChannel[CurrentChannel]));	
		}
	
		/* step 4: store TSSI delta values to EEPROM 0x6f - 0x75 */
		RT28xx_EEPROM_READ16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);
		EEPData &= 0x00ff;
		EEPData |= (PowerDeltaPerChannel[1] << 8) | (PowerDeltaPerChannel[2] << 12);
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);

		for (CurrentChannel = 3; CurrentChannel <= 14; CurrentChannel += 4)
		{
			/* EEPData = ( TssiDeltaPerChannel[CurrentChannel+2]  << 12) |(  TssiDeltaPerChannel[CurrentChannel+1]  << 8); */
			/* DBGPRINT(RT_DEBUG_TRACE, ("CurrentChannel=%d, TssiDeltaPerChannel[CurrentChannel+2] = 0x%x, EEPData=0x%x\n", CurrentChannel, TssiDeltaPerChannel[CurrentChannel+2], EEPData)); */
			EEPData = (PowerDeltaPerChannel[CurrentChannel + 3] << 12) | (PowerDeltaPerChannel[CurrentChannel + 2] << 8) | 
				(PowerDeltaPerChannel[CurrentChannel + 1] << 4) | PowerDeltaPerChannel[CurrentChannel];
			RT28xx_EEPROM_WRITE16(pAd, (EEPROM_TX_POWER_OFFSET_OVER_CH_3 + ((CurrentChannel - 3) / 2)), EEPData);
			/* DBGPRINT(RT_DEBUG_TRACE, ("offset=0x%x, EEPData = 0x%x\n", (EEPROM_TSSI_DELTA_CH3_CH4 +((CurrentChannel-3)/2)),EEPData));	*/
		}
						
		/* restore RF R27 and R28, BBP R47 */
		/* RT30xxWriteRFRegister(pAd, RF_R27, RF27Value); */				
		/* RT30xxWriteRFRegister(pAd, RF_R28, RF28Value); */

		Set_ATE_Proc(pAd, "ATESTART");
	}

	return TRUE;
}

INT RT5392_ATEReadExternalTSSI(
	IN PRTMP_ADAPTER		pAd,
	IN PSTRING				arg)
{
	UCHAR	inputData = 0, RFValue = 0, BbpData = 0;
	USHORT	EEPData;
	UCHAR 	BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	
	inputData = simple_strtol(arg, 0, 10);
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, EEPData);

	if (!(IS_RT5392(pAd) && ((EEPData & 0x02) != 0)))			
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Not support TSSI calibration since not 5392 chip or EEPROM not set!!!\n"));
		return FALSE;
	}
	else
	{		
		/* NdisZeroMemory(&pAd->ate, sizeof(struct _ATE_INFO)); */
		pAd->ate.TxCount = 1000000;
		pAd->ate.TxLength = 1058;
		pAd->ate.Channel = 1;
		COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
		COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);    

		Set_ATE_TX_MODE_Proc(pAd, "1");		/* MODE_OFDM */
		Set_ATE_TX_MCS_Proc(pAd, "7");		/* 54Mbps */
		Set_ATE_TX_BW_Proc(pAd, "0");		/* 20MHz */
				
		/* Read calibrated channel power value from EEPROM */
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET, EEPData);
		pAd->ate.TxPower0 = (UCHAR)(EEPData & 0xff);
		DBGPRINT(RT_DEBUG_TRACE, ("(Calibrated) TxPower0= 0x%x\n", pAd->ate.TxPower0));
		
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET, EEPData);
		pAd->ate.TxPower1 = (UCHAR)(EEPData & 0xff);
		DBGPRINT(RT_DEBUG_TRACE, ("(Calibrated) TxPower1= 0x%x\n", pAd->ate.TxPower1));

		Set_ATE_TX_Antenna_Proc(pAd, "0");	/* Both Tx0 and Tx1 */
		Set_ATE_IPG_Proc(pAd, "200");		/* IPG 200 */
		Set_ATE_TX_GI_Proc(pAd, "0");		/* GI 0 */
			
		/* Read frequency offset from EEPROM */                       
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
		pAd->ate.RFFreqOffset = (UCHAR)(EEPData & 0xff);
			
		Set_ATE_Proc(pAd, "TXFRAME"); 
		RTMPusecDelay(2000000);
	
		/* BBP R47[7]=1 : ADC 6 on */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpData);
		BbpData |= 0x80;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpData);

		/* RF R27[7:6]=0x1 : Adc_insel 01:Temp */	
		/* Write RF R27[3:0]=EEPROM 0x76 bit[3:0] */
		RT30xxReadRFRegister(pAd, RF_R27, &RFValue);	
		RFValue &= ~0xC0;
		RFValue |= 0x40;
		RFValue = ((RFValue & 0xF0) | pAd->TssiGain); /* [3:0] = (tssi_gain and tssi_atten) */
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);			

		/* Wait 1ms. */					
		RTMPusecDelay(1000);

		/* Read BBP R49 reading as return value. */
		DBGPRINT(RT_DEBUG_TRACE, ("Read  BBP_R49\n")); 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP R49=0x%x\n", BbpData));

		RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_STEP_OVER_2DOT4G, EEPData);
		EEPData &= 0xff00;
		EEPData |= BbpData;
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_STEP_OVER_2DOT4G, EEPData);
		RTMPusecDelay(10);

		/* RF R27[7:6]=0x0 */
		RT30xxReadRFRegister(pAd, RF_R27, &RFValue);	
		RFValue &= ~0xC0;
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);		

		Set_ATE_Proc(pAd, "ATESTART");
	}

	return TRUE;
}

#ifdef RTMP_INTERNAL_TX_ALC
BOOLEAN RT5390_ATEGetDesiredTssiAndCurrentTssi(
	IN PRTMP_ADAPTER 		pAd, 
	INOUT PCHAR 			pDesiredTssi, 
	INOUT PCHAR 			pCurrentTssi)
{
	extern CHAR RT5390_desiredTSSIOverCCKExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][4];
	extern CHAR RT5390_desiredTSSIOverOFDMExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][8];
	extern CHAR RT5390_desiredTSSIOverHTExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][8];
	extern CHAR RT5390_desiredTSSIOverHT40Ext[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][8];
	extern CHAR RT5390_desiredTSSIOverHTUsingSTBCExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][8];

	UCHAR BbpR47 = 0;
	UCHAR RateInfo = 0;
	CCK_TSSI_INFO cckTssiInfo = {{0}};
	OFDM_TSSI_INFO ofdmTssiInfo = {{0}};
	HT_TSSI_INFO htTssiInfo = {
			.PartA.value= 0,
			.PartB.value = 0,
		};

	UCHAR ch = 0;

	DBGPRINT(RT_DEBUG_INFO, ("--->%s\n", __FUNCTION__));

	if (IS_RT5390(pAd))
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		if ((BbpR47 & 0x04) == 0x04) /* The TSSI INFO is not ready. */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::BBP TSSI INFO is not ready. (BbpR47 = 0x%X)\n", __FUNCTION__, BbpR47));

			return FALSE;
		}
		
		/* Get TSSI */
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)(pCurrentTssi));

		if ((*pCurrentTssi < 0) || (*pCurrentTssi > 0x7C))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Out of range, *pCurrentTssi = %d\n", __FUNCTION__, *pCurrentTssi));
			
			*pCurrentTssi = 0;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s::*pCurrentTssi = %d\n", __FUNCTION__, *pCurrentTssi));
		
		/* Get packet information */
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		BbpR47 = ((BbpR47 & ~0x03) | 0x01); /* TSSI_REPORT_SEL (TSSI INFO 1 - Packet infomation) */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)(&RateInfo));

		if ((RateInfo & 0x03) == MODE_CCK) /* CCK */
		{
			cckTssiInfo.value = RateInfo;

			DBGPRINT(RT_DEBUG_TRACE, ("%s::CCK, cckTssiInfo.field.Rate = %d\n", 
				__FUNCTION__, 
				cckTssiInfo.field.Rate));

			DBGPRINT(RT_DEBUG_INFO, ("%s::RateInfo = 0x%X\n", __FUNCTION__, RateInfo));

			if (cckTssiInfo.field.Rate >= 4) /* boundary verification */
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::incorrect MCS: cckTssiInfo.field.Rate = %d\n", 
					__FUNCTION__, 
					cckTssiInfo.field.Rate));
				
				return FALSE;
			}

			if ((pAd->ate.Channel >= 1) && (pAd->ate.Channel <= 14))
			{
				ch = pAd->ate.Channel;
			}
			else
			{
				ch = 1;

				DBGPRINT(RT_DEBUG_ERROR, ("%s::Incorrect channel #%d\n", __FUNCTION__, pAd->ate.Channel));
			}
		
			*pDesiredTssi = RT5390_desiredTSSIOverCCKExt[ch][cckTssiInfo.field.Rate];

			DBGPRINT(RT_DEBUG_TRACE, ("%s::ch = %d, *pDesiredTssi = %d\n", __FUNCTION__, ch, *pDesiredTssi));

		}
		else if ((RateInfo & 0x03) == MODE_OFDM) /* OFDM */
		{
			ofdmTssiInfo.value = RateInfo;
			
			/* BBP OFDM rate format ==> MAC OFDM rate format */
			
			switch (ofdmTssiInfo.field.Rate)
			{
				case 0x0B: /* 6 Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_0;
				}
				break;

				case 0x0F: /* 9 Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_1;
				}
				break;

				case 0x0A: /* 12 Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_2;
				}
				break;

				case 0x0E: /* 18  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_3;
				}
				break;

				case 0x09: /* 24  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_4;
				}
				break;

				case 0x0D: /* 36  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_5;
				}
				break;

				case 0x08: /* 48  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_6;
				}
				break;

				case 0x0C: /* 54  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_7;
				}
				break;

				default: 
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s::Incorrect OFDM rate = 0x%X\n", __FUNCTION__, ofdmTssiInfo.field.Rate));
					
					return FALSE;
				}
				break;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s::OFDM, ofdmTssiInfo.field.Rate = %d\n", 
				__FUNCTION__, 
				ofdmTssiInfo.field.Rate));

			if (ofdmTssiInfo.field.Rate > 7) /* boundary verification */
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::incorrect MCS: ofdmTssiInfo.field.Rate = %d\n", 
					__FUNCTION__, 
					ofdmTssiInfo.field.Rate));

				return FALSE;
			}

			if ((pAd->ate.Channel >= 1) && (pAd->ate.Channel <= 14))
			{
				ch = pAd->ate.Channel;
			}
			else
			{
				ch = 1;

				DBGPRINT(RT_DEBUG_ERROR, ("%s::Incorrect channel #%d\n", __FUNCTION__, pAd->ate.Channel));
			}

			*pDesiredTssi = RT5390_desiredTSSIOverOFDMExt[ch][ofdmTssiInfo.field.Rate];

			DBGPRINT(RT_DEBUG_TRACE, ("%s::ch = %d, *pDesiredTssi = %d\n", __FUNCTION__, ch, *pDesiredTssi));
			
			DBGPRINT(RT_DEBUG_INFO, ("%s::RateInfo = 0x%X\n", __FUNCTION__, RateInfo));
		}
		else /* Mixed mode or green-field mode */
		{
			htTssiInfo.PartA.value = RateInfo;
			DBGPRINT(RT_DEBUG_INFO, ("%s::RateInfo = 0x%X\n", __FUNCTION__, RateInfo));

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
			BbpR47 = ((BbpR47 & ~0x03) | 0x02); /* TSSI_REPORT_SEL (TSSI INFO 2 - Packet infomation) */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, 0x92);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)(&RateInfo));

			htTssiInfo.PartB.value = RateInfo;
			DBGPRINT(RT_DEBUG_INFO, ("%s::RateInfo = 0x%X\n", __FUNCTION__, RateInfo));

			DBGPRINT(RT_DEBUG_TRACE, ("%s::HT, htTssiInfo.PartA.field.STBC = %d, htTssiInfo.PartB.field.MCS = %d\n", 
				__FUNCTION__, 
				htTssiInfo.PartA.field.STBC, 
				htTssiInfo.PartB.field.MCS));

			if ((htTssiInfo.PartB.field.MCS < 0) || (htTssiInfo.PartB.field.MCS > 7)) /* boundary verification */
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::incorrect MCS: htTssiInfo.PartB.field.MCS = %d\n", 
					__FUNCTION__, 
					htTssiInfo.PartB.field.MCS));

				return FALSE;
			}

			if ((pAd->ate.Channel >= 1) && (pAd->ate.Channel <= 14))
			{
				ch = pAd->ate.Channel;
			}
			else
			{
				ch = 1;

				DBGPRINT(RT_DEBUG_ERROR, ("%s::Incorrect channel #%d\n", __FUNCTION__, pAd->ate.Channel));
			}

			if (htTssiInfo.PartA.field.STBC == 0)
			{
				if ((htTssiInfo.PartB.field.BW == BW_40) && 
				     ((htTssiInfo.PartB.field.MCS == MCS_5) || (htTssiInfo.PartB.field.MCS == MCS_6) || (htTssiInfo.PartB.field.MCS == MCS_7)))
				{
					*pDesiredTssi = RT5390_desiredTSSIOverHT40Ext[ch][htTssiInfo.PartB.field.MCS];
				}
				else
				{
					*pDesiredTssi = RT5390_desiredTSSIOverHTExt[ch][htTssiInfo.PartB.field.MCS];
				}
			}
			else
			{
				*pDesiredTssi = RT5390_desiredTSSIOverHTUsingSTBCExt[ch][htTssiInfo.PartB.field.MCS];
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s::ch = %d, *pDesiredTssi = %d\n", __FUNCTION__, ch, *pDesiredTssi));			
		}	

		if (*pDesiredTssi < 0x00)
		{
			*pDesiredTssi = 0x00;
		}	
		else if (*pDesiredTssi > 0x7C)
		{
			*pDesiredTssi = 0x7C;
		}

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		BbpR47 = ((BbpR47 & ~0x07) | 0x04); /* TSSI_REPORT_SEL (TSSI INFO 0 - TSSI) and enable TSSI INFO udpate */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);
	}

	DBGPRINT(RT_DEBUG_INFO, ("<---%s\n", __FUNCTION__));

	return TRUE;
}
#endif /* RTMP_INTERNAL_TX_ALC */

VOID RT539x_ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER 		pAd) 
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	INT	LookupTableIndex = pAd->TxPowerCtrl.LookupTableIndex + TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable;
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry0 = NULL; /* Ant0 */
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry1 = NULL; /* Ant1 */
	CHAR	TuningTableUpperBound = 0, TuningTableIndex0 = 0, TuningTableIndex1 = 0;
	CHAR 	desiredTssi = 0, currentTssi = 0, TotalDeltaPower = 0, DeltaPwr = 0, Value = 0;
	PCHAR	pTxAgcCompensate = NULL;
	UCHAR 	RFValue = 0;
	ULONG	TxPwr[5];
	INT 		i, j, maxTxPwrCnt = 5, *LookupTable, RefTemp, CurrentTemp = 0;
	BBP_R49_STRUC BbpR49;
	BOOLEAN	bAutoTxAgc = FALSE;
		
	/* BG band channel */
	bAutoTxAgc = pAd->bAutoTxAgcG;
	pTxAgcCompensate = &pAd->TxAgcCompensateG;
	TxPowerTuningTable = pChipCap->TxPowerTuningTable_2G;
	RefTemp = pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G];
	LookupTable = &pAd->TxPowerCtrl.LookupTable[IEEE80211_BAND_2G][0];
	TuningTableUpperBound = pChipCap->TxAlcTxPowerUpperBound_2G;

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

#ifdef RTMP_INTERNAL_TX_ALC
	/* Locate the internal Tx ALC tuning entry */
	if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE) && IS_RT5390(pAd))
	{
		if (RT5390_ATEGetDesiredTssiAndCurrentTssi(pAd, &desiredTssi, &currentTssi) == FALSE)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Incorrect desired TSSI or current TSSI\n", __FUNCTION__));
				
			/* Tx power adjustment over RF */
			RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)(&RFValue));
			RFValue = ((RFValue & ~0x3F) | pAd->TxPowerCtrl.RF_TX_ALC);
			if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
			{
				RFValue = ((RFValue & ~0x3F) | 0x27);
			}
			RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)(RFValue));

			/* Tx power adjustment over MAC */
			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;
		}
		else
		{
			if (desiredTssi > currentTssi)
			{
				pAd->TxPowerCtrl.idxTxPowerTable++;
			}

			if (desiredTssi < currentTssi)
			{
				pAd->TxPowerCtrl.idxTxPowerTable--;
			}

			TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPower[pAd->ate.Channel-1].Power;

			TuningTableIndex0 = (TuningTableIndex0 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
								LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex0;
			TuningTableIndex0 = (TuningTableIndex0 > UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd)) ? 
								UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd) : TuningTableIndex0;

			/* Valide pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 69 */
			TxPowerTuningTableEntry0 = &TxPowerTuningTable[TuningTableIndex0 + TX_POWER_TUNING_ENTRY_OFFSET]; 

			pAd->TxPowerCtrl.RF_TX_ALC = TxPowerTuningTableEntry0->RF_TX_ALC;
			pAd->TxPowerCtrl.MAC_PowerDelta = TxPowerTuningTableEntry0->MAC_PowerDelta;

			/* Tx power adjustment over RF */
			RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)(&RFValue));
			RFValue = ((RFValue & ~0x3F) | pAd->TxPowerCtrl.RF_TX_ALC);
			if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x1F */
			{
				RFValue = ((RFValue & ~0x3F) | 0x27);
			}
			RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)(RFValue));

			/* Tx power adjustment over MAC */
			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("%s::desiredTSSI = %d, currentTSSI = %d, idxTxPowerTable = %d, TuningTableIndex = %d, {RF_TX_ALC = 0x%X, MAC_PowerDelta = %d}\n", 
				__FUNCTION__, 
				desiredTssi, 
				currentTssi, 
				pAd->TxPowerCtrl.idxTxPowerTable, 
				TuningTableIndex0,
				TxPowerTuningTableEntry0->RF_TX_ALC, 
				TxPowerTuningTableEntry0->MAC_PowerDelta));
		}
	}
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_COMPENSATION
	if (bAutoTxAgc && IS_RT5392(pAd))
	{ 
		/* Current temperature */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
		CurrentTemp = (CHAR)BbpR49.byte;
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] BBP_R49 = %02x, current temp = %d\n", BbpR49.byte, CurrentTemp));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] RefTemp = %d\n", RefTemp));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] index = %d\n", pAd->TxPowerCtrl.LookupTableIndex));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex - 1, LookupTable[LookupTableIndex - 1]));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex, LookupTable[LookupTableIndex]));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex + 1, LookupTable[LookupTableIndex + 1]));
		if (CurrentTemp > RefTemp + LookupTable[LookupTableIndex + 1] + ((LookupTable[LookupTableIndex + 1] - LookupTable[LookupTableIndex]) >> 2) &&
			LookupTableIndex < 32)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ++\n"));
			LookupTableIndex++;
			pAd->TxPowerCtrl.LookupTableIndex++;
		}
		else if (CurrentTemp < RefTemp + LookupTable[LookupTableIndex] - ((LookupTable[LookupTableIndex] - LookupTable[LookupTableIndex - 1]) >> 2) &&
			LookupTableIndex > 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] --\n"));
			LookupTableIndex--;
			pAd->TxPowerCtrl.LookupTableIndex--;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ==\n"));
		}

		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] idxTxPowerTable=%d, idxTxPowerTable2=%d, TuningTableUpperBound=%d\n",
			pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPowerCtrl.LookupTableIndex,
			pAd->TxPowerCtrl.idxTxPowerTable2 + pAd->TxPowerCtrl.LookupTableIndex,
			TuningTableUpperBound));

		TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable 
								+ pAd->TxPowerCtrl.LookupTableIndex 
								+ pAd->TxPower[pAd->ate.Channel-1].Power;

		/* The boundary verification */ 
		TuningTableIndex0 = (TuningTableIndex0 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex0;
		TuningTableIndex0 = (TuningTableIndex0 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex0;
		TxPowerTuningTableEntry0 = &TxPowerTuningTable[TuningTableIndex0 + TX_POWER_TUNING_ENTRY_OFFSET];
		
		TuningTableIndex1 = pAd->TxPowerCtrl.idxTxPowerTable2 
									+ pAd->TxPowerCtrl.LookupTableIndex 
									+ pAd->TxPower[pAd->ate.Channel-1].Power2;
		/* The boundary verification */
		TuningTableIndex1 = (TuningTableIndex1 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex1;
		TuningTableIndex1 = (TuningTableIndex1 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex1;
		TxPowerTuningTableEntry1 = &TxPowerTuningTable[TuningTableIndex1 + TX_POWER_TUNING_ENTRY_OFFSET];
			
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx0)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry0->RF_TX_ALC, TxPowerTuningTableEntry0->MAC_PowerDelta, TuningTableIndex0));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx1)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry1->RF_TX_ALC, TxPowerTuningTableEntry1->MAC_PowerDelta, TuningTableIndex1));

		/* Update RF_R49 [0:5] */
		RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
		RFValue = ((RFValue & ~0x3F) | TxPowerTuningTableEntry0->RF_TX_ALC);
		if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
		{
			RFValue = ((RFValue & ~0x3F) | 0x27);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Update RF_R49[0:5] to 0x%x\n", TxPowerTuningTableEntry0->RF_TX_ALC));
		RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

		/* Update RF_R50 [0:5] */
		RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
		RFValue = ((RFValue & ~0x3F) | TxPowerTuningTableEntry1->RF_TX_ALC);
		if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
		{
			RFValue = ((RFValue & ~0x3F) | 0x27);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Update RF_R50[0:5] to 0x%x\n", TxPowerTuningTableEntry1->RF_TX_ALC));
		RT30xxWriteRFRegister(pAd, RF_R50, RFValue);
		
		TotalDeltaPower = TxPowerTuningTableEntry0->MAC_PowerDelta;
	}
#endif /* RTMP_TEMPERATURE_COMPENSATION */


	/* Set new Tx power for different Tx rates */
	for (i = 0; i < maxTxPwrCnt; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j = 0; j < 8; j++)
			{
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F);

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
				/* The upper bounds of MAC 0x1314 ~ 0x1324 are variable */
				if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE)^(pAd->chipCap.bTempCompTxALC == TRUE))
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
							DBGPRINT(RT_DEBUG_ERROR, ("%s::unknown register = 0x%X\n", __FUNCTION__, (TX_PWR_CFG_0 + (i * 4))));
						}
						break;
					}
				}
				else
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */
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

				/* Fill new value into the corresponding MAC offset */
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);

			DBGPRINT(RT_DEBUG_TRACE, ("%s::After compensation, Offset = 0x%04X, RegisterValue = 0x%08lX\n",
				__FUNCTION__, TX_PWR_CFG_0 + (i << 2), TxPwr[i]));			
		}
	}
		
	/* Extra set MAC registers to compensate Tx power if any */
	RT539x_ATEAsicExtraPowerOverMAC(pAd);	
}

/* 
==========================================================================
    Description:
        Set  RT5370 and RT5372 and RT5390 and RT5392  ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT539x_Set_ATE_TX_BW_Proc(
	IN PRTMP_ADAPTER		pAd, 
	IN PSTRING				arg)
{
	INT 		powerIndex;
	UCHAR 	BBPCurrentBW, value = 0;
	
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

		/* Set BBP R4 bit[4:3]=0:0 to be BW 20 */
 		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
 		value &= (~0x18);
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, 0x10);
		
		/* Set BW = 20 MHz */
		pAd->LatchRfRegs.R4 &= ~0x00200000;
		RtmpRfIoWrite(pAd);


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
    				value = 0x28;
    				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, value);
			}
		}
#endif /* A_BAND_SUPPORT */

		/* Set BBP R4 bit[4:3]=1:0 to be BW 40 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		value |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, 0x16);

		/* set BW = 40 MHz */
		pAd->LatchRfRegs.R4 |= 0x00200000;
		RtmpRfIoWrite(pAd);
	}

	/* Set BBP R68=0x0B to improve Rx sensitivity */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, 0x0B);
	
	/* Set BBP R69=0x0D */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x0D);
	
	/* Set BBP R70=0x06 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x06);
	
	/* Set BBP R73=0x13 */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x13);

	DBGPRINT(RT_DEBUG_TRACE, ("%s::Current BW = %d\n", __FUNCTION__, pAd->ate.TxWI.BW));

	return TRUE;
}	

/* 
==========================================================================
    Description:
        Set RT5370 and RT5372 and RT5390 and RT5392   ATE RF central frequency offset
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT539x_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN PRTMP_ADAPTER		pAd, 
	IN PSTRING				arg)
{	
	UCHAR  RFFreqOffset = 0, RefFreqOffset = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset >= 96)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Out of range(0 ~ 95)\n", __FUNCTION__));
		return FALSE;
	}

	pAd->ate.RFFreqOffset = RFFreqOffset;

	RefFreqOffset = pAd->ate.RFFreqOffset;
	
	RTMPAdjustFrequencyOffset(pAd, &RefFreqOffset);
	DBGPRINT(RT_DEBUG_TRACE, ("%s::RFFreqOffset = %d\n", __FUNCTION__, RefFreqOffset));
	
	return TRUE;
}


struct _ATE_CHIP_STRUCT RALINK5390 =
{
	/* Functions */
	.ChannelSwitch = RT539x_ATEAsicSwitchChannel,
	.TxPwrHandler = RT539x_ATETxPwrHandler,
	.TssiCalibration = RT5390_ATETssiCalibration,
	.ExtendedTssiCalibration = RT5390_ATETssiCalibrationExtend,
	.RxVGAInit = RT539x_ATERxVGAInit,
	.AsicSetTxRxPath = NULL,	
	.AdjustTxPower = RT539x_ATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = RT539x_ATEAsicExtraPowerOverMAC,

	/* Command handlers */	
	.Set_BW_Proc = RT539x_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT539x_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* Variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,	
	.bBBPStoreTXCONT = TRUE,
	.bBBPLoadATESTOP = TRUE,
};

struct _ATE_CHIP_STRUCT RALINK5392 =
{
	/* Functions */
	.ChannelSwitch = RT539x_ATEAsicSwitchChannel,
	.TxPwrHandler = RT539x_ATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = RT5392_ATEReadExternalTSSI,
	.RxVGAInit = RT539x_ATERxVGAInit,
	.AsicSetTxRxPath = RT5392_ATEAsicSetTxRxPath,	
	.AdjustTxPower = RT539x_ATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = RT539x_ATEAsicExtraPowerOverMAC,

	/* Command handlers */	
	.Set_BW_Proc = RT539x_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT539x_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* Variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,	
	.bBBPStoreTXCONT = TRUE,
	.bBBPLoadATESTOP = TRUE,
};

#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */