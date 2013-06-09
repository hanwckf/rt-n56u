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
	rt33xx.c

	Abstract:
	Specific funcitons and variables for RT30xx.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#ifdef RT33xx


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

#include "rt_config.h"



/* RF register initialization set*/


#ifdef RT3390
REG_PAIR RT3390_RFRegTable[] = {
	{RF_R00,		0xA0},
	{RF_R01,		0xC1},
	{RF_R02,		0xF1},
	{RF_R03,		0x32},
	{RF_R04,		0x49},
	{RF_R05,		0xCF},
	{RF_R06,		0x4A},
	/* according to HK comment RF_R07 must
	    set to be 0x60 for Max Input Power. */
	{RF_R07,		0x60/*0xE0*/},
	{RF_R08,		0x49}, /* Read only*/
	{RF_R09,		0xC1},

	{RF_R10,		0x41},
	{RF_R11,		0x11},
	{RF_R12,		0x2C},
	{RF_R13,		0xE0},
	{RF_R14,		0x90},
	{RF_R15,		0x73},
	{RF_R16,		0x44},
	{RF_R17,		0x93},
	{RF_R18,		0x5C},
	{RF_R19,		0x86},

	{RF_R20,		0xB2},
	{RF_R21,		0xC7},
	{RF_R22,		0x04},	
	{RF_R23,		0x12},
	{RF_R24,		BW20RFR24},
	{RF_R25,		0x22},
	{RF_R26,		0x85},
	/* according to Joe Jean requirement,
 		1. RF27 value shall change to 0x8f RT3390 error rate issue.
		   mans change RF27 Default value from 0x02 to 0x07. 
		2. Joe Jean request the value change to 0x8c.
		    means chage RT27 Default value from 0x07 to 0x04.
		3. HW request the value change to 0x8f.
		    means chage RT27 Default value from 0x04 to 0x047 */
	{RF_R27,		0x07/*0x02*/},
	{RF_R28,		0x60},
	{RF_R29,		0xB8},
	{RF_R30,		0x29},
	{RF_R31,		BW20RFR31},
};

UCHAR RT3390_NUM_RF_REG_PARMS = (sizeof(RT3390_RFRegTable) / sizeof(REG_PAIR));
#endif /* RT3390*/

#ifdef RTMP_INTERNAL_TX_ALC
/* The Tx power tuning entry */
TX_POWER_TUNING_ENTRY_STRUCT RT33xx_TxPowerTuningTable[] = 
{
/*	idxTxPowerTable		Tx power control over RF		Tx power control over MAC */
/*	(zero-based array)		{ RF R12[4:0]: Tx0 ALC},		{MAC 0x1314~0x1324} */
/*	0	*/				{0x00, 						-15}, 
/*	1	*/				{0x01, 						-15}, 
/*	2	*/				{0x00, 						-14}, 
/*	3	*/				{0x01, 						-14}, 
/*	4	*/				{0x00, 						-13}, 
/*	5	*/				{0x01, 						-13}, 
/*	6	*/				{0x00, 						-12}, 
/*	7	*/				{0x01, 						-12}, 
/*	8	*/				{0x00, 						-11}, 
/*	9	*/				{0x01, 						-11}, 
/*	10	*/				{0x00, 						-10}, 
/*	11	*/				{0x01, 						-10}, 
/*	12	*/				{0x00, 						-9}, 
/*	13	*/				{0x01, 						-9}, 
/*	14	*/				{0x00, 						-8}, 
/*	15	*/				{0x01, 						-8}, 
/*	16	*/				{0x00, 						-7}, 
/*	17	*/				{0x01, 						-7}, 
/*	18	*/				{0x00, 						-6}, 
/*	19	*/				{0x01, 						-6}, 
/*	20	*/				{0x00, 						-5}, 
/*	21	*/				{0x01, 						-5}, 
/*	22	*/				{0x00, 						-4}, 
/*	23	*/				{0x01, 						-4}, 
/*	24	*/				{0x00, 						-3}, 
/*	25	*/				{0x01, 						-3}, 
/*	26	*/				{0x00,						-2}, 
/*	27	*/				{0x01, 						-2}, 
/*	28	*/				{0x00, 						-1}, 
/*	29	*/				{0x01, 						-1}, 
/*	30	*/				{0x00,						0}, 
/*	31	*/				{0x01,						0}, 
/*	32	*/				{0x02,						0}, 
/*	33	*/				{0x03,						0}, 
/*	34	*/				{0x04,						0}, 
/*	35	*/				{0x05,						0}, 
/*	36	*/				{0x06,						0}, 
/*	37	*/				{0x07,						0}, 
/*	38	*/				{0x08,						0}, 
/*	39	*/				{0x09,						0}, 
/*	40	*/				{0x0A,						0}, 
/*	41	*/				{0x0B,						0}, 
/*	42	*/				{0x0C,						0}, 
/*	43	*/				{0x0D,						0}, 
/*	44	*/				{0x0E,						0}, 
/*	45	*/				{0x0F,						0}, 
/*	46	*/				{0x0F-1,						1}, 
/*	47	*/				{0x0F,						1}, 
/*	48	*/				{0x0F-1,						2}, 
/*	49	*/				{0x0F,						2}, 
/*	50	*/				{0x0F-1,						3}, 
/*	51	*/				{0x0F,						3}, 
/*	52	*/				{0x0F-1,						4}, 
/*	53	*/				{0x0F,						4}, 
/*	54	*/				{0x0F-1,						5}, 
/*	55	*/				{0x0F,						5}, 
/*	56	*/				{0x0F-1,						6}, 
/*	57	*/				{0x0F,						6}, 
/*	58	*/				{0x0F-1,						7}, 
/*	59	*/				{0x0F,						7}, 
/*	60	*/				{0x0F-1,						8}, 
/*	61	*/				{0x0F,						8}, 
/*	62	*/				{0x0F-1,						9}, 
/*	63	*/				{0x0F,						9}, 
/*	64	*/				{0x0F-1,						10}, 
/*	65	*/				{0x0F,						10}, 
/*	66	*/				{0x0F-1,						11}, 
/*	67	*/				{0x0F,						11}, 
/*	68	*/				{0x0F-1,						12}, 
/*	69	*/				{0x0F,						12}, 
/*	70	*/				{0x0F-1,						13}, 
/*	71	*/				{0x0F,						13}, 
/*	72	*/				{0x0F-1,						14}, 
/*	73	*/				{0x0F,						14}, 
/*	74	*/				{0x0F-1,						15}, 
/*	75	*/				{0x0F,						15}, 
};

/* The desired TSSI over CCK */
CHAR desiredTSSIOverCCK[4] = {0};

/* The desired TSSI over OFDM */
CHAR desiredTSSIOverOFDM[8] = {0};

/* The desired TSSI over HT */
CHAR desiredTSSIOverHT[8] = {0};

/* The desired TSSI over HT using STBC */
CHAR desiredTSSIOverHTUsingSTBC[8] = {0};
#endif /* RTMP_INTERNAL_TX_ALC */

/*
========================================================================
Routine Description:
	Initialize RT35xx.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT33xx_Init(
	IN PRTMP_ADAPTER		pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	/* init capability */
	/* 
		WARNING: 
			Currently following table are shared by all RT30xx based IC, change it carefully when you add a new IC here.
	*/
	pChipCap->pRFRegTable = RT3020_RFRegTable;
	pChipCap->MaxNumOfBbpId = 185;

	/* init operator */

#ifdef RT3390
		if (pAd->infType == RTMP_DEV_INF_PCIE)
		{
			pChipCap->pRFRegTable = RT3390_RFRegTable;
			pChipOps->AsicRfInit = NICInitRT3390RFRegisters;
		}
#endif /* RT3390 */

		pChipOps->AsicHaltAction = RT33xxHaltAction;
		pChipOps->AsicRfTurnOff = RT33xxLoadRFSleepModeSetup;		
		pChipOps->AsicReverseRfFromSleepMode = RT33xxReverseRFSleepModeSetup;
		pChipOps->ChipSwitchChannel = RT33xx_ChipSwitchChannel;
		pChipOps->AsicAdjustTxPower = AsicAdjustTxPower;
		pChipOps->ChipBBPAdjust = RT30xx_ChipBBPAdjust;
		pChipOps->ChipAGCInit = RT30xx_ChipAGCInit;
		/* 1T1R only */
		pChipOps->SetRxAnt = RT33xxSetRxAnt;
		pAd->Mlme.bEnableAutoAntennaCheck = FALSE;
		pChipCap->FlgIsHwAntennaDiversitySup = FALSE; 

		pChipOps->AsicGetTxPowerOffset = AsicGetTxPowerOffset;

		/*pChipOps->ChipResumeMsduTransmission = NULL; */
		/*pChipOps->VdrTuning1 = NULL; */
		pChipOps->RxSensitivityTuning = NULL;
		pChipCap->MaxNumOfBbpId = 185;
		pChipCap->TXWISize = 16;
		pChipCap->RXWISize = 16;

#ifdef RTMP_INTERNAL_TX_ALC
		pChipCap->TxAlcTxPowerUpperBound_2G = 45;
		pChipCap->TxPowerTuningTable_2G = RT33xx_TxPowerTuningTable;
		pChipOps->InitDesiredTSSITable = RT33xx_InitDesiredTSSITable;
		pChipOps->AsicTxAlcGetAutoAgcOffset = RT33xx_AsicTxAlcGetAutoAgcOffset;
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */

#ifdef RT3390
	RT3390ChipHook(pAd);
#endif
 	}

/* Antenna divesity use GPIO3 and EESK pin for control*/
/* Antenna and EEPROM access are both using EESK pin,*/
/* Therefor we should avoid accessing EESK at the same time*/
/* Then restore antenna after EEPROM access*/
/* The original name of this function is AsicSetRxAnt(), now change to */
/*VOID AsicSetRxAnt(*/

VOID RT33xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	UINT32	Value;
	UINT32	x;

	if (/*(!pAd->NicConfig2.field.AntDiversity) ||*/
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		return;
	}

	/* the antenna selection is through firmware and MAC register(GPIO3)*/
	if (IS_RT3390(pAd) && pAd->RfIcType == RFIC_3320)
	{
	if (Ant == 0)
	{
		/* Main antenna*/
		/* E2PROM_CSR only in PCI bus Reg., USB Bus need MCU commad to control the EESK pin.*/
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x |= (EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
#endif /* RTMP_MAC_PCI */

		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to main antenna\n"));
	}
	else
	{
		/* Aux antenna*/
		/* E2PROM_CSR only in PCI bus Reg., USB Bus need MCU commad to control the EESK pin.*/
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x &= ~(EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
#endif /* RTMP_MAC_PCI */
		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		Value |= 0x08;
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to aux antenna\n"));
		}
	}
}



/* add by johnli, RF power sequence setup*/
/*
	==========================================================================
	Description:

	Load RF normal operation-mode setup
	
	==========================================================================
 */
VOID RT33xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue = 0, bbpreg = 0 ;

	/* improve power consumption */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R138, &bbpreg);
	if (pAd->Antenna.field.TxPath == 1)
	{
		/* turn off tx DAC_1*/
		bbpreg = (bbpreg | 0x20);
	}

	if (pAd->Antenna.field.RxPath == 1)
	{
		/* turn off tx ADC_1*/
		bbpreg &= (~0x2);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R138, bbpreg);

	/* RX0_PD & TX0_PD, RF R1 register Bit 2 & Bit 3 to 0 and RF_BLOCK_en,RX1_PD & TX1_PD, Bit0, Bit 4 & Bit5 to 1*/
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue = (RFValue & (~0x0C)) | 0x31;
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	/* TX_LO2_en, RF R15 register Bit 3 to 0*/
	RT30xxReadRFRegister(pAd, RF_R15, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R15, RFValue);

	/* TX_LO1_en, RF R17 register Bit 3 to 0*/
	RT30xxReadRFRegister(pAd, RF_R17, &RFValue);
	RFValue &= (~0x08);
	/* to fix rx long range issue*/
	if (((pAd->MACVersion & 0xffff) >= 0x0211) && (pAd->NicConfig2.field.ExternalLNAForG == 0))
	{
		RFValue |= 0x20;
	}
	/* set RF_R17_bit[2:0] equal to EEPROM setting at 0x48h*/
	if (pAd->TxMixerGain24G >= 2)
	{
		RFValue &= (~0x7);  /* clean bit [2:0]*/
		RFValue |= pAd->TxMixerGain24G;
	}
	RT30xxWriteRFRegister(pAd, RF_R17, RFValue);

	/* RX_LO1_en, RF R20 register Bit 3 to 0*/
	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

	/* RX_LO2_en, RF R21 register Bit 3 to 0*/
	RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R21, RFValue);
}

/*
	==========================================================================
	Description:

	Load RF sleep-mode setup
	
	==========================================================================
 */
VOID RT33xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;

		/* RF_BLOCK_en. RF R1 register Bit 0 to 0*/
		RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
		RFValue &= (~0x01);
		RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

		/* VCO_IC, RF R7 register Bit 4 & Bit 5 to 0*/
		RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
		RFValue &= (~0x30);
		RT30xxWriteRFRegister(pAd, RF_R07, RFValue);


		/* RX_CTB_en, RF R21 register Bit 7 to 0*/
		RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
		RFValue &= (~0x80);
		RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

}

/*
	==========================================================================
	Description:

	Reverse RF sleep-mode setup
	
	==========================================================================
 */
VOID RT33xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd,
	IN BOOLEAN			FlgIsInitState)
{
	UCHAR RFValue;

		/* RF_BLOCK_en, RF R1 register Bit 0 to 1*/
		RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
		RFValue |= 0x01;
		RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

		/* VCO_IC, RF R7 register Bit 4 & Bit 5 to 1*/
		RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
		/* According to HK's comment for Max Input power issue.
		    RF 07 must set to 0x60. */
		RFValue |= 0x20; /* 0x30. */
		RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

		/* RX_CTB_en, RF R21 register Bit 7 to 1*/
		RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
		RFValue |= 0x80;
		RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

}
/* end johnli*/

VOID RT33xxHaltAction(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32		TxPinCfg = 0x00050F0F;

	
	/* Turn off LNA_PE or TRSW_POL*/
	
        /* Fixed suspend leakage current*/
                /* According to MAC 0x0580 bit [31], set MAC 0x1328 bit[18] during suspend mode.*/
                /* If SEL_EFUSE=0, set TRSW_POL=0 in suspend mode.*/
                /* If SEL_EFUSE=1, set TRSW_POL=1 in suspend mode.*/
	
		if (IS_RT3390(pAd)
#ifdef RTMP_EFUSE_SUPPORT
			&& (pAd->bUseEfuse)
#endif /* RTMP_EFUSE_SUPPORT */
			)
		{
			TxPinCfg &= 0xFFFBF0F0; /* bit18 off*/
		}
		else
		{
			TxPinCfg &= 0xFFFFF0F0;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);   
	}

VOID RT33xx_ChipSwitchChannel(
	IN PRTMP_ADAPTER 			pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan)
{
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; /*Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER;*/
	UCHAR	index;
	UINT32 	Value = 0; /*BbpReg, Value;*/
	UCHAR 	RFValue;

#ifdef RT30xx
	UCHAR Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0;
	BBP_R109_STRUC BbpR109 = {{0}};
#endif /* RT30xx */

	RFValue = 0;
	/* Search Tx power value*/

	/*
		We can't use ChannelList to search channel, since some central channl's txpowr doesn't list 
		in ChannelList, so use TxPower array instead.
	*/
	for (index = 0; index < MAX_NUM_OF_CHANNELS; index++)
	{
		if (Channel == pAd->TxPower[index].Channel)
		{
			TxPwer = pAd->TxPower[index].Power;
			TxPwer2 = pAd->TxPower[index].Power2;

#ifdef RT30xx /*RT33xx*/
			if (IS_RT3090A(pAd) || IS_RT3390(pAd))
				/*(pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE))*/
			{
				Tx0FinePowerCtrl = pAd->TxPower[index].Tx0FinePowerCtrl;
				Tx1FinePowerCtrl = pAd->TxPower[index].Tx1FinePowerCtrl;
			}
#endif /* RT30xx */

			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: Can't find the Channel#%d \n", Channel));
	}
#ifdef RT30xx
	/* The RF programming sequence is difference between 3xxx and 2xxx*/
	if ((IS_RT30xx(pAd)) && 
		((pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_2020) ||
		(pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022) || (pAd->RfIcType == RFIC_3320)))
	{
		/* modify by WY for Read RF Reg. error */
		UCHAR	calRFValue;
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
				/* Programming channel parameters*/
				RT30xxWriteRFRegister(pAd, RF_R02, FreqItems3020[index].N);
				/*
					RT3370/RT3390 RF version is 0x3320 RF_R3 [7:4] is not reserved bits
					RF_R3[6:4] (pa1_bc_cck) : PA1 Bias CCK
					RF_R3[7] (pa2_cc_cck) : PA2 Cascode Bias CCK
				 */
				RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)(&RFValue));
				RFValue = (RFValue & 0xF0) | (FreqItems3020[index].K & ~0xF0); /* <bit 3:0>:K<bit 3:0>*/
				RT30xxWriteRFRegister(pAd, RF_R03, RFValue);
				RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
				RFValue = (RFValue & 0xFC) | FreqItems3020[index].R;
				RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

				/* Set Tx0 Power*/
				RT30xxReadRFRegister(pAd, RF_R12, &RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer;
				RT30xxWriteRFRegister(pAd, RF_R12, RFValue);

				/* Set Tx1 Power*/
				RT30xxReadRFRegister(pAd, RF_R13, &RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer2;
				RT30xxWriteRFRegister(pAd, RF_R13, RFValue);

#ifdef RT33xx
#ifdef RTMP_MAC_PCI
				
				/* Set the BBP Tx fine power control in 0.1dB step*/
				
				if ((IS_RT3090A(pAd) || IS_RT3390(pAd)) &&
					(pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE))
				{
					BbpR109.field.Tx0PowerCtrl = Tx0FinePowerCtrl;

					if (pAd->Antenna.field.TxPath >= 2)
					{
						BbpR109.field.Tx1PowerCtrl = Tx1FinePowerCtrl;
					}
					else
					{
						BbpR109.field.Tx1PowerCtrl = 0;
					}
					
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R109, BbpR109.byte);

					DBGPRINT(RT_DEBUG_INFO, ("%s: Channel = %d, BBP_R109 = 0x%X\n", 
						__FUNCTION__, 
						Channel, 
						BbpR109.byte));
				}
#endif /* RTMP_MAC_PCI */
#endif /* RT33xx */

				/* Tx/Rx Stream setting*/
				RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
				/*if (IS_RT3090(pAd))*/
				/*	RFValue |= 0x01;  Enable RF block.*/
				RFValue &= 0xC3;	/*clear bit[7~2]*/
				if (pAd->Antenna.field.TxPath == 1)
					RFValue |= 0x20;
				else if (pAd->Antenna.field.TxPath == 2)
					;
				if (pAd->Antenna.field.RxPath == 1)
					RFValue |= 0x10;
				else if (pAd->Antenna.field.RxPath == 2)
					;
				RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

				/* Set RF offset*/
				RT30xxReadRFRegister(pAd, RF_R23, &RFValue);
				RFValue = (RFValue & 0x80) | pAd->RfFreqOffset;
				RT30xxWriteRFRegister(pAd, RF_R23, RFValue);

				/* Set BW*/
				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40))
				{
					calRFValue = pAd->Mlme.CaliBW40RfR24;
					/*DISABLE_11N_CHECK(pAd);*/
				}
				else
				{
					calRFValue = pAd->Mlme.CaliBW20RfR24;
				}
				/*
					RT3370/RT3390 RF version is 0x3320 RF_R24 [7:6] is not reserved bits
					RF_R24[6] (BB_Rx1_out_en) : enable baseband output and ADC input
					RF_R24[7] (BB_Tx1_out_en) : enable DAC output or baseband input
				 */
				RT30xxReadRFRegister(pAd, RF_R24, (PUCHAR)(&RFValue));
				calRFValue = (RFValue & 0xC0) | (calRFValue & ~0xC0); /* <bit 5>:tx_h20M<bit 5> and <bit 4:0>:tx_agc_fc<bit 4:0>*/
				RT30xxWriteRFRegister(pAd, RF_R24, calRFValue);

				/*
					RT3370/RT3390 RF version is 0x3320 RF_R31 [7:6] is not reserved bits
					RF_R31[4:0] (rx_agc_fc) : capacitor control in baseband filter
					RF_R31[5] (rx_ h20M) : rx_ h20M: 0=10 MHz and 1=20MHz
					RF_R31[7:6] (drv_bc_cck) : Driver Bias CCK
				 */
				/* Set BW*/
				if (IS_RT3390(pAd)) /* RT3390 has different AGC for Tx and Rx*/
				{
					if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40))
					{
						calRFValue = pAd->Mlme.CaliBW40RfR31;
					}
					else
					{
						calRFValue = pAd->Mlme.CaliBW20RfR31;
					}
				}
				RT30xxReadRFRegister(pAd, RF_R31, (PUCHAR)(&RFValue));
				calRFValue = (RFValue & 0xC0) | (calRFValue & ~0xC0); /* <bit 5>:rx_h20M<bit 5> and <bit 4:0>:rx_agc_fc<bit 4:0>				*/
				RT30xxWriteRFRegister(pAd, RF_R31, calRFValue);

				/* Enable RF tuning*/
				RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
				RFValue = RFValue | 0x1;
				RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

				/* latch channel for future usage.*/
				pAd->LatchRfRegs.Channel = Channel;
				
		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
			Channel, 
			pAd->RfIcType, 
			TxPwer,
			TxPwer2,
			pAd->Antenna.field.TxPath,
			FreqItems3020[index].N, 
			FreqItems3020[index].K, 
			FreqItems3020[index].R));

				break;
			}
		}
	}
	
#endif /* RT30xx */	

	/* Change BBP setting during siwtch from a->g, g->a*/
	if (Channel <= 14)
	{
		ULONG	TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/


		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);/*(0x44 - GET_LNA_GAIN(pAd)));	 According the Rory's suggestion to solve the middle range issue.*/

		/* Rx High power VGA offset for LNA select*/
		
		if (pAd->NicConfig2.field.ExternalLNAForG)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x84);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}	


		

		/* Turn off unused PA or LNA when only 1T or 1R*/
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}		

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);


#if defined(RT3090) || defined(RT3390)
		/* PCIe PHY Transmit attenuation adjustment*/
		if ((IS_RT3090A(pAd) || IS_RT3390(pAd)) && IS_PCIE_INF(pAd))
		{
			INTERNAL_1_STRUCT Internal_1 = { { 0 } };

			RTMP_IO_READ32(pAd, INTERNAL_1, &Internal_1.word);

			if (Channel == 14) /* Channel #14*/
			{
				Internal_1.field.PCIE_PHY_TX_ATTEN_EN = 1; /* Enable PCIe PHY Tx attenuation*/
				Internal_1.field.PCIE_PHY_TX_ATTEN_VALUE = 4; /* 9/16 full drive level*/
			}
			else /* Channel #1~#13*/
			{
				Internal_1.field.PCIE_PHY_TX_ATTEN_EN = 0; /* Disable PCIe PHY Tx attenuation*/
				Internal_1.field.PCIE_PHY_TX_ATTEN_VALUE = 0; /* n/a*/
			}

			RTMP_IO_WRITE32(pAd, INTERNAL_1, Internal_1.word);
		}
#endif /* defined(RT3090) || defined(RT3390) */

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		ULONG	TxPinCfg = 0x00050F05;/*Gary 2007/8/9 0x050505*/
		UINT8	bbpValue;
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);/*(0x44 - GET_LNA_GAIN(pAd)));    According the Rory's suggestion to solve the middle range issue.     */

		/* Set the BBP_R82 value here */
		bbpValue = 0xF2;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, bbpValue);


		/* Rx High power VGA offset for LNA select*/
		if (pAd->NicConfig2.field.ExternalLNAForA)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		/* 5G band selection PIN, bit1 and bit2 are complement*/
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Turn off unused PA or LNA when only 1T or 1R*/

		/* Turn off unused PA or LNA when only 1T or 1R*/
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

	
	/* GPIO control*/
	

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	/*AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x2E + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL); */
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	
	/* On 11A, We should delay and wait RF/BBP to be stable*/
	/* and the appropriate time should be 1000 micro seconds */
	/* 2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.*/
	
	RTMPusecDelay(1000);  
}

#ifdef RTMP_INTERNAL_TX_ALC
VOID RT33xx_InitDesiredTSSITable(
	IN PRTMP_ADAPTER 		pAd)
{
	UCHAR TSSIBase = 0; /* The TSSI over OFDM 54Mbps */
	USHORT TSSIStepOver2dot4G = 0; /* The TSSI value/step (0.5 dB/unit) */
	UCHAR RFValue = 0;
	BBP_R49_STRUC BbpR49;
	ULONG i = 0;
	USHORT TxPower = 0, TxPowerOFDM54 = 0, temp = 0;

	BbpR49.byte = 0;
	
	if (pAd->TxPowerCtrl.bInternalTxALC == FALSE)
	{
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, temp);
	TSSIBase = (temp & 0x001F);
	
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TSSI_STEP_OVER_2DOT4G - 1), TSSIStepOver2dot4G);
	TSSIStepOver2dot4G = (0x000F & (TSSIStepOver2dot4G >> 8));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_OFDM_MCS6_MCS7 - 1), TxPowerOFDM54);
	TxPowerOFDM54 = (0x000F & (TxPowerOFDM54 >> 8));

	DBGPRINT(RT_DEBUG_TRACE, ("%s: TSSIBase = %d, TSSIStepOver2dot4G = %d, TxPowerOFDM54 = %d\n", 
		__FUNCTION__, 
		TSSIBase, 
		TSSIStepOver2dot4G, 
		TxPowerOFDM54));

	/* The desired TSSI over CCK */
	RT28xx_EEPROM_READ16(pAd, EEPROM_CCK_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xDE = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverCCK[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)) + 6);
	desiredTSSIOverCCK[MCS_1] = desiredTSSIOverCCK[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_CCK_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xDF = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverCCK[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)) + 6);
	desiredTSSIOverCCK[MCS_3] = desiredTSSIOverCCK[MCS_2];

	/* Boundary verification: the desired TSSI value */
	for (i = 0; i < 4; i++) /* CCK: MCS 0 ~ MCS 3 */
	{
		if (desiredTSSIOverCCK[i] < 0x00)
		{
			desiredTSSIOverCCK[i] = 0x00;
		}
		else if (desiredTSSIOverCCK[i] > 0x1F)
		{
			desiredTSSIOverCCK[i] = 0x1F;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverCCK[0] = %d, desiredTSSIOverCCK[1] = %d, desiredTSSIOverCCK[2] = %d, desiredTSSIOverCCK[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverCCK[0], 
		desiredTSSIOverCCK[1], 
		desiredTSSIOverCCK[2], 
		desiredTSSIOverCCK[3]));

	/* The desired TSSI over OFDM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_OFDM_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE0 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverOFDM[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverOFDM[MCS_1] = desiredTSSIOverOFDM[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_OFDM_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE1 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverOFDM[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverOFDM[MCS_3] = desiredTSSIOverOFDM[MCS_2];
	RT28xx_EEPROM_READ16(pAd, EEPROM_OFDM_MCS4_MCS5, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE2 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverOFDM[MCS_4] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverOFDM[MCS_5] = desiredTSSIOverOFDM[MCS_4];
	desiredTSSIOverOFDM[MCS_6] = TSSIBase;
	desiredTSSIOverOFDM[MCS_7] = TSSIBase;

	/* Boundary verification: the desired TSSI value */
	for (i = 0; i < 8; i++) /* OFDM: MCS 0 ~ MCS 7 */
	{
		if (desiredTSSIOverOFDM[i] < 0x00)
		{
			desiredTSSIOverOFDM[i] = 0x00;
		}
		else if (desiredTSSIOverOFDM[i] > 0x1F)
		{
			desiredTSSIOverOFDM[i] = 0x1F;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverOFDM[0] = %d, desiredTSSIOverOFDM[1] = %d, desiredTSSIOverOFDM[2] = %d, desiredTSSIOverOFDM[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverOFDM[0], 
		desiredTSSIOverOFDM[1], 
		desiredTSSIOverOFDM[2], 
		desiredTSSIOverOFDM[3]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverOFDM[4] = %d, desiredTSSIOverOFDM[5] = %d, desiredTSSIOverOFDM[6] = %d, desiredTSSIOverOFDM[7] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverOFDM[4], 
		desiredTSSIOverOFDM[5], 
		desiredTSSIOverOFDM[6], 
		desiredTSSIOverOFDM[7]));

	/* The desired TSSI over HT */
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE4 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_1] = desiredTSSIOverHT[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE5 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_3] = desiredTSSIOverHT[MCS_2];
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS4_MCS5, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE6 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_4] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_5] = desiredTSSIOverHT[MCS_4];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS6_MCS7 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE7 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_6] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_7] = desiredTSSIOverHT[MCS_6];

	/* Boundary verification: the desired TSSI value */
	for (i = 0; i < 8; i++) /* HT: MCS 0 ~ MCS 7 */
	{
		if (desiredTSSIOverHT[i] < 0x00)
		{
			desiredTSSIOverHT[i] = 0x00;
		}
		else if (desiredTSSIOverHT[i] > 0x1F)
		{
			desiredTSSIOverHT[i] = 0x1F;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[0] = %d, desiredTSSIOverHT[1] = %d, desiredTSSIOverHT[2] = %d, desiredTSSIOverHT[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[0], 
		desiredTSSIOverHT[1], 
		desiredTSSIOverHT[2], 
		desiredTSSIOverHT[3]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[4] = %d, desiredTSSIOverHT[5] = %d, desiredTSSIOverHT[6] = %d, desiredTSSIOverHT[7] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[4], 
		desiredTSSIOverHT[5], 
		desiredTSSIOverHT[6], 
		desiredTSSIOverHT[7]));

	/* The desired TSSI over HT using STBC */
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_USING_STBC_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xEC = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_1] = desiredTSSIOverHTUsingSTBC[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_USING_STBC_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xED = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_3] = desiredTSSIOverHTUsingSTBC[MCS_2];
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_USING_STBC_MCS4_MCS5, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xEE = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_4] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_5] = desiredTSSIOverHTUsingSTBC[MCS_4];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_USING_STBC_MCS6_MCS7 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xEF = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_6] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_7] = desiredTSSIOverHTUsingSTBC[MCS_6];

	/* Boundary verification: the desired TSSI value */
	for (i = 0; i < 8; i++) /* HT using STBC: MCS 0 ~ MCS 7 */
	{
		if (desiredTSSIOverHTUsingSTBC[i] < 0x00)
		{
			desiredTSSIOverHTUsingSTBC[i] = 0x00;
		}
		else if (desiredTSSIOverHTUsingSTBC[i] > 0x1F)
		{
			desiredTSSIOverHTUsingSTBC[i] = 0x1F;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHTUsingSTBC[0] = %d, desiredTSSIOverHTUsingSTBC[1] = %d, desiredTSSIOverHTUsingSTBC[2] = %d, desiredTSSIOverHTUsingSTBC[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHTUsingSTBC[0], 
		desiredTSSIOverHTUsingSTBC[1], 
		desiredTSSIOverHTUsingSTBC[2], 
		desiredTSSIOverHTUsingSTBC[3]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHTUsingSTBC[4] = %d, desiredTSSIOverHTUsingSTBC[5] = %d, desiredTSSIOverHTUsingSTBC[6] = %d, desiredTSSIOverHTUsingSTBC[7] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHTUsingSTBC[4], 
		desiredTSSIOverHTUsingSTBC[5], 
		desiredTSSIOverHTUsingSTBC[6], 
		desiredTSSIOverHTUsingSTBC[7]));

	RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)(&RFValue));
	RFValue = (RFValue | 0x88); /* <7>: IF_Rxout_en, <3>: IF_Txout_en */
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

	RT30xxReadRFRegister(pAd, RF_R28, (PUCHAR)(&RFValue));
	RFValue = (RFValue & (~0x60)); /* <6:5>: tssi_atten */
	RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
	BbpR49.field.adc5_in_sel = 1; /* Enable the PSI (internal components, new version - RT3390) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpR49.byte);		

	DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
}

/*
	==========================================================================
	Description:
		Get the desired TSSI based on the latest packet

	Arguments:
		pAd

	Return Value:
		The desired TSSI
	==========================================================================
 */
UCHAR RT33xx_GetDesiredTSSI(
	IN PRTMP_ADAPTER 		pAd)
{
	PHTTRANSMIT_SETTING pLatestTxHTSetting = (PHTTRANSMIT_SETTING)(&pAd->LastTxRate);
	UCHAR desiredTSSI = 0;
	UCHAR MCS = 0;

	MCS = (UCHAR)(pLatestTxHTSetting->field.MCS);
	
	if (pLatestTxHTSetting->field.MODE == MODE_CCK)
	{
		if ((MCS < 0) || (MCS > 3)) /* boundary verification */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", __FUNCTION__, MCS));
			MCS = 0;
		}
	
		desiredTSSI = desiredTSSIOverCCK[MCS];
	}
	else if (pLatestTxHTSetting->field.MODE == MODE_OFDM)
	{
		if ((MCS < 0) || (MCS > 7)) /* boundary verification */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", __FUNCTION__, MCS));
			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverOFDM[MCS];
	}
	else if ((pLatestTxHTSetting->field.MODE == MODE_HTMIX) || (pLatestTxHTSetting->field.MODE == MODE_HTGREENFIELD))
	{
		if ((MCS < 0) || (MCS > 7)) /* boundary verification */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", __FUNCTION__, MCS));
			MCS = 0;
		}

		if (pLatestTxHTSetting->field.STBC == 1)
		{
			desiredTSSI = desiredTSSIOverHT[MCS];
		}
		else
		{
			desiredTSSI = desiredTSSIOverHTUsingSTBC[MCS];
		}

		/* For HT BW40 MCS 7 with/without STBC configuration, the desired TSSI value should subtract one from the formula */
		if ((pLatestTxHTSetting->field.BW == BW_40) && (MCS == MCS_7))
		{
			desiredTSSI -= 1;
		}
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: desiredTSSI = %d, Latest Tx HT setting: MODE = %d, MCS = %d, STBC = %d\n", 
		__FUNCTION__, 
		desiredTSSI, 
		pLatestTxHTSetting->field.MODE, 
		pLatestTxHTSetting->field.MCS, 
		pLatestTxHTSetting->field.STBC));

	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));

	return desiredTSSI;
}

VOID RT33xx_AsicTxAlcGetAutoAgcOffset(
	IN PRTMP_ADAPTER 			pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1)
{
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable = pAd->chipCap.TxPowerTuningTable_2G;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	BBP_R49_STRUC 	BbpR49;
	UCHAR 			RFValue = 0;
	CHAR 			desiredTssi = 0;
	CHAR 			currentTssi = 0;
	CHAR 			TotalDeltaPower = 0; 
	CHAR			TuningTableIndex = 0;

	BbpR49.byte = 0;
	
	/* Locate the Internal Tx ALC tuning entry */
	if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE) && (IS_RT3390(pAd)))
	{
		if ((pAd->Mlme.OneSecPeriodicRound % 4 == 0) && (*pDeltaPowerByBbpR1 == 0))
		{
			desiredTssi = RT33xx_GetDesiredTSSI(pAd);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
			currentTssi = BbpR49.field.TSSI;

			if (desiredTssi > currentTssi)
			{
				pAd->TxPowerCtrl.idxTxPowerTable++;
			}

			if (desiredTssi < currentTssi)
			{
				pAd->TxPowerCtrl.idxTxPowerTable--;
			}

			TuningTableIndex = pAd->TxPowerCtrl.idxTxPowerTable
#ifdef DOT11_N_SUPPORT				
								+ pAd->TxPower[pAd->CommonCfg.CentralChannel-1].Power;
#else
								+ pAd->TxPower[pAd->CommonCfg.Channel-1].Power;
#endif /* DOT11_N_SUPPORT */

			if (TuningTableIndex < LOWERBOUND_TX_POWER_TUNING_ENTRY)
			{
				TuningTableIndex = LOWERBOUND_TX_POWER_TUNING_ENTRY;
			}

			if (TuningTableIndex > UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd))
			{
				TuningTableIndex = UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd);
			}
			
			/* Valide pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 45 */
			pTxPowerTuningEntry = &TxPowerTuningTable[TuningTableIndex + TX_POWER_TUNING_ENTRY_OFFSET];
			pAd->TxPowerCtrl.RF_TX_ALC = pTxPowerTuningEntry->RF_TX_ALC;
			pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

			/* Tx power adjustment over RF */
			RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)(&RFValue));
			RFValue = ((RFValue & 0xE0) | pAd->TxPowerCtrl.RF_TX_ALC);
			RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)(RFValue));

			/* Tx power adjustment over MAC */
			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, currentTSSI = %d, idxTxPowerTable = %d, TuningTableIndex = %d, {RF_TX_ALC = %d, MAC_PowerDelta = %d}\n", 
				__FUNCTION__, 
				desiredTssi, 
				currentTssi, 
				pAd->TxPowerCtrl.idxTxPowerTable, 
				TuningTableIndex,
				pTxPowerTuningEntry->RF_TX_ALC, 
				pTxPowerTuningEntry->MAC_PowerDelta));
		}
		else
		{
			/* Tx power adjustment over RF */
			RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)(&RFValue));
			RFValue = ((RFValue & 0xE0) | pAd->TxPowerCtrl.RF_TX_ALC);
			RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)(RFValue));

			/* Tx power adjustment over MAC */
			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;
		}
	}

	*pTotalDeltaPwr = TotalDeltaPower;
}
#endif /* RTMP_INTERNAL_TX_ALC */
#endif /* RT33xx */

