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
	rt3883_ate.c

	Abstract:
	Specific ATE funcitons and configurations for RT3883

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3883

#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

extern UCHAR NUM_OF_3883_CHNL; 
extern FREQUENCY_ITEM FreqItems3883[];


VOID RT3883_ATE_TxAntennaSelect(
    IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 MACValue = 0;
	UCHAR RFValue = 0;
	UCHAR Channel;
	CHAR TxAntennaSel;
	BOOLEAN b5GBand;

	Channel = pATEInfo->Channel;
	TxAntennaSel = pATEInfo->TxAntennaSel;
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
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 MACValue = 0;
	UCHAR BbpValue = 0;
	UCHAR RFValue = 0;
	UCHAR Channel;
	CHAR RxAntennaSel;
	BOOLEAN b5GBand;

	Channel = pATEInfo->Channel;
	RxAntennaSel = pATEInfo->RxAntennaSel;
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
			if (pATEInfo->Channel < 132)
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

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

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
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
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


/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT3883 ATE.
    
==========================================================================
*/
VOID RT3883ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;
	CHAR TxPwer3 = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;
	TxPwer3 = pATEInfo->TxPower2;

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

			/* set RF frequency offset */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);
			RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;
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
			if (pATEInfo->TxWI.BW == BW_20)
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
			if (pATEInfo->TxWI.BW == BW_20)
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
			if (pATEInfo->bTxBF == TRUE)
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
			if (pATEInfo->bTxBF == TRUE)
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
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, pATEInfo->TxPower0);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, pATEInfo->TxPower1);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, pATEInfo->TxPower2);
			}
			else
			{
				/* (Gary, 2010-02-12) */
				CHAR power = 0x48 | ((pATEInfo->TxPower0 & 0x18) << 1) | (pATEInfo->TxPower0 & 0x7);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, power);
				/* (Gary, 2010-02-12) */
				power = 0x48 | ((pATEInfo->TxPower1 & 0x18) << 1) | (pATEInfo->TxPower1 & 0x7);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R54, power);
				/* (Gary, 2010-02-12) */
				power = 0x48 | ((pATEInfo->TxPower2 & 0x18) << 1) | (pATEInfo->TxPower2 & 0x7);
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
			if (pATEInfo->bTxBF)
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
		if (pATEInfo->RxAntennaSel == 0)
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
		if (pATEInfo->RxAntennaSel == 0)
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
}


INT RT3883ATETxPwrHandler(
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
#ifdef DOT11N_SS3_SUPPORT
	else if (index == 2)
	{
		TxPower = pATEInfo->TxPower2;
	}
#endif /* DOT11N_SS3_SUPPORT */
	else
	{
#ifdef DOT11N_SS3_SUPPORT
		DBGPRINT_ERR(("%s : Only TxPower0, TxPower1, and TxPower2 are adjustable !\n", __FUNCTION__));
#else
		DBGPRINT_ERR(("%s : Only TxPower0 and TxPower1 are adjustable !\n", __FUNCTION__));
#endif /* DOT11N_SS3_SUPPORT */
		DBGPRINT_ERR(("TxPower%d is out of range !\n", index));
		return -1;
	}

	if (pATEInfo->Channel <= 14)
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


VOID RT3883ATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	
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

	return TRUE;
}


INT	RT3883_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue = 0;
	UCHAR diff = 0;
	UCHAR RFValue2 = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset >= 96)
	{
		DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}

	pATEInfo->RFFreqOffset = RFFreqOffset;

	/* Set RF offset  RF_R17=RF_R23 (RT30xx) */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);
	RFValue2 = (RFValue & 0x80) | pATEInfo->RFFreqOffset;
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
	
	return TRUE;
}

struct _ATE_CHIP_STRUCT RALINK3883 =
{
	/* functions */
	.ChannelSwitch = RT3883ATEAsicSwitchChannel,
	.TxPwrHandler = RT3883ATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT3883ATERxVGAInit,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = DefaultATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = NULL,	
	
	/* command handlers */
	.Set_BW_Proc = RT3883_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT3883_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 9,
#ifdef CONFIG_AP_SUPPORT
	.bBBPStoreTXCARR = FALSE,
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	.bBBPStoreTXCARR = TRUE,
#endif /* CONFIG_STA_SUPPORT */
	.bBBPStoreTXCARRSUPP = FALSE,
	.bBBPStoreTXCONT = FALSE,
#ifdef CONFIG_AP_SUPPORT
	.bBBPLoadATESTOP = FALSE,
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	.bBBPLoadATESTOP = TRUE,
#endif /* CONFIG_STA_SUPPORT */
};

#endif /* RT3883 */

