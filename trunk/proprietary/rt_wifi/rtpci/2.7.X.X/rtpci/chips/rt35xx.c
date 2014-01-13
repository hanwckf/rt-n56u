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
	rt35xx.c

	Abstract:
	Specific funcitons and variables for 
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

FREQUENCY_ITEM FreqItems3572[] =
{	
	/* ISM : 2.4 to 2.483 GHz*/
	/*-CH---N-------R---K-----------*/
	{1,    241,  2,  2},
	{2,    241,	 2,  7},
	{3,    242,	 2,  2},
	{4,    242,	 2,  7},
	{5,    243,	 2,  2},
	{6,    243,	 2,  7},
	{7,    244,	 2,  2},
	{8,    244,	 2,  7},
	{9,    245,	 2,  2},
	{10,   245,	 2,  7},
	{11,   246,	 2,  2},
	{12,   246,	 2,  7},
	{13,   247,	 2,  2},
	{14,   248,	 2,  4},

	/* 5 GHz*/
	{36,	0x56,	0,	4},
	{38,	0x56,	0,	6},
	{40,	0x56,	0,	8},
	{44,	0x57,	0,	0},
	{46,	0x57,	0,	2},
	{48,	0x57,	0,	4},
	{52,	0x57,	0,	8},
	{54,	0x57,	0,	10},
	{56,	0x58,	0,	0},
	{60,	0x58,	0,	4},
	{62,	0x58,	0,	6},
	{64,	0x58,	0,	8},

	{100,	0x5B,	0,	8},
	{102,	0x5B,	0,	10},
	{104,	0x5C,	0,	0},
	{108,	0x5C,	0,	4},
	{110,	0x5C,	0,	6},
	{112,	0x5C,	0,	8},
/*	{114,	0x5C,	0,	10},*/
	{116,	0x5D,	0,	0},
	{118,	0x5D,	0,	2},
	{120,	0x5D,	0,	4},
	{124,	0x5D,	0,	8},
	{126,	0x5D,	0,	10},
	{128,	0x5E,	0,	0},
	{132,	0x5E,	0,	4},
	{134,	0x5E,	0,	6},
	{136,	0x5E,	0,	8},
	{140,	0x5F,	0,	0},

	{149,	0x5F,	0,	9},
	{151,	0x5F,	0,	11},
	{153,	0x60,	0,	1},
	{157,	0x60,	0,	5},
	{159,	0x60,	0,	7},
	{161,	0x60,	0,	9},
	{165,	0x61,	0,	1},
	{167,	0x61,	0,	3},
	{169,	0x61,	0,	5},
	{171,	0x61,	0,	7},
	{173,	0x61,	0,	9},
};
UCHAR NUM_OF_3572_CHNL = (sizeof(FreqItems3572) / sizeof(FREQUENCY_ITEM));

REG_PAIR   RF3572_RFRegTable[] = {
	{RF_R00,		0x70},
	{RF_R01,		0x81},
	{RF_R02,		0xF1},
	{RF_R03,		0x02},
	{RF_R04,		0x4C},
	{RF_R05,		0x05},
	{RF_R06,		0x4A},
	{RF_R07,		0xD8},
/*	{RF_R08,		0x80},*/
	{RF_R09,		0xC3},

	{RF_R10,		0xF1},
	{RF_R11,		0xB9},
	{RF_R12,		0x70},
	{RF_R13,		0x65},
	{RF_R14,		0xA0},
	{RF_R15,		0x53},
	{RF_R16,		0x4C},
	{RF_R17,		0x23},
	{RF_R18,		0xAC},
	{RF_R19,		0x93},

	{RF_R20,		0xB3},
	{RF_R21,		0xD0},
	{RF_R22,		0x00},	
	{RF_R23,		0x3C},
	{RF_R24,		0x16},
	{RF_R25,		0x15},
	{RF_R26,		0x85},
	{RF_R27,		0x00},
	{RF_R28,		0x00},
	{RF_R29,		0x9B},
	{RF_R30,		0x09},
	{RF_R31,		0x10},
};

#ifdef RTMP_FLASH_SUPPORT
static UCHAR RT3572_EeBuffer[EEPROM_SIZE] = {
0x72, 0x35, 0x02, 0x01, 0x00, 0x0c, 0x43, 0x30, 0x92, 0x00, 0x92, 0x30, 0x14, 0x18, 0x01, 0x80,
0x00, 0x00, 0x92, 0x30, 0x14, 0x18, 0x00, 0x00, 0x01, 0x00, 0x6a, 0xff, 0x13, 0x02, 0xff, 0xff,
0xff, 0xff, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0x8e, 0x75, 0x01, 0x43, 0x22, 0x08, 0x27, 0x00, 0xff, 0xff, 0x16, 0x01, 0xff, 0xff, 0xd9, 0xfa,
0xcc, 0x88, 0xff, 0xff, 0x0a, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
0xff, 0xff, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x1d, 0x1a,
0x15, 0x11, 0x0f, 0x0d, 0x0a, 0x07, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x88, 0x88,
0xcc, 0xcc, 0xaa, 0x88, 0xcc, 0xcc, 0xaa, 0x88, 0xcc, 0xcc, 0xaa, 0x88, 0xcc, 0xcc, 0xaa, 0x88,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, };
#endif /* RTMP_FLASH_SUPPORT */

#define	NUM_RF_3572REG_PARMS	(sizeof(RF3572_RFRegTable) / sizeof(REG_PAIR))

RTMP_REG_PAIR	RT35xx_MACRegTable[] =	{
	{TX_SW_CFG0,		0x400},
};

UCHAR RT35xx_NUM_MAC_REG_PARMS = (sizeof(RT35xx_MACRegTable) / sizeof(RTMP_REG_PAIR));

VOID RT35xx_SetAGCInitValue(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth);


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
VOID RT35xx_Init(
	IN PRTMP_ADAPTER		pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	/* init capability */
	pChipCap->MaxNumOfBbpId = 185;
	pChipCap->pRFRegTable = RF3572_RFRegTable;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_1;
#ifdef RTMP_EFUSE_SUPPORT
	pChipCap->EFUSE_USAGE_MAP_START = 0x3c0;
	pChipCap->EFUSE_USAGE_MAP_END = 0x3fb;      
	pChipCap->EFUSE_USAGE_MAP_SIZE = 60;
	DBGPRINT(RT_DEBUG_ERROR, ("(Reassign Efuse for 3x7x/3x9x/53xx) Size=0x%x [%x-%x] \n",pAd->chipCap.EFUSE_USAGE_MAP_SIZE,pAd->chipCap.EFUSE_USAGE_MAP_START,pAd->chipCap.EFUSE_USAGE_MAP_END));
#endif /* RTMP_EFUSE_SUPPORT */
#ifdef RTMP_FLASH_SUPPORT
        pChipCap->eebuf = RT3572_EeBuffer;
#endif /* RTMP_FLASH_SUPPORT */
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 16;
	/* init operator */
	pChipOps->AsicRfTurnOff = RT35xxLoadRFSleepModeSetup;
	pChipOps->AsicRfInit = NICInitRT3572RFRegisters;
	pChipOps->AsicReverseRfFromSleepMode = RT3572ReverseRFSleepModeSetup;
	pChipOps->AsicHaltAction = RT30xxHaltAction;
	pChipOps->AsicMacInit = NICInitRT35xxMacRegisters;
	pChipOps->ChipSwitchChannel = RT35xx_ChipSwitchChannel;
	pChipOps->AsicAdjustTxPower = AsicAdjustTxPower;
	pChipOps->RxSensitivityTuning = RT35xx_RxSensitivityTuning;
	pChipOps->NICInitAsicFromEEPROM = RT35xx_NICInitAsicFromEEPROM;
	pChipOps->ChipAGCInit = RT35xx_SetAGCInitValue;
	pChipOps->AsicGetTxPowerOffset = AsicGetTxPowerOffset;
	pChipOps->AsicTxAlcGetAutoAgcOffset = AsicGetAutoAgcOffsetForExternalTxAlc;
#ifdef CARRIER_DETECTION_SUPPORT
	pChipCap->carrier_func = TONE_RADAR_V2;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v2;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef DFS_SUPPORT
	pChipCap->DfsEngineNum = 4;
#endif /* DFS_SUPPORT */

#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = EnableAPMIMOPSv2;
	pChipOps->DisableAPMIMOPS = DisableAPMIMOPSv2;
#endif /* GREENAP_SUPPORT */

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */

	pChipOps->ChipBBPAdjust = RT35xx_ChipBBPAdjust;
}


/*
========================================================================
Routine Description:
	Initialize specific MAC registers.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT35xxMacRegisters(
	IN	PRTMP_ADAPTER		pAd)
{
	UINT32 IdReg;


	for(IdReg=0; IdReg<RT35xx_NUM_MAC_REG_PARMS; IdReg++)
	{
		/* if we want to change the full regiter content */
		RTMP_IO_WRITE32(pAd, (USHORT)RT35xx_MACRegTable[IdReg].Register,
								RT35xx_MACRegTable[IdReg].Value);
	}
}


VOID NICInitRT3572RFRegisters(IN PRTMP_ADAPTER pAd)
{
	INT i;
	UINT8 RfReg = 0;
	UINT32 data;
	
	/*
		Driver must read EEPROM to get RfIcType before initial RF registers
		Initialize RF register to default value
		Init RF calibration
		Driver should toggle RF R30 bit7 before init RF registers
    */
    RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RfReg);
    RfReg |= 0x80;
    RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);
    RTMPusecDelay(1000);
    RfReg &= 0x7F;
    RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);        

	/* Initialize RF register to default value */
	for (i = 0; i < NUM_RF_3572REG_PARMS; i++)
	{
		RT30xxWriteRFRegister(pAd, RF3572_RFRegTable[i].Register, RF3572_RFRegTable[i].Value);
	}

	/* Driver should set RF R6 bit6 on before init RF registers */
	RT30xxReadRFRegister(pAd, RF_R06, (PUCHAR)&RfReg);
	RfReg |= 0x40;
	RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RfReg);

	/* init R31 */
	/*RT30xxWriteRFRegister(pAd, RF_R31, 0x14);*/

	if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
	{
		/* patch tx EVM issue temporarily */
		RTMP_IO_READ32(pAd, LDO_CFG0, &data);
		data = ((data & 0xF0FFFFFF) | 0x0D000000);
		RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
	}
	else
	{
		/* Patch for SRAM, increase voltage to 1.35V on core voltage and down to 1.2V after 1 msec*/
		RTMP_IO_READ32(pAd, LDO_CFG0, &data);
		data = ((data & 0xE0FFFFFF) | 0x0D000000);
		RTMP_IO_WRITE32(pAd, LDO_CFG0, data);

		RTMPusecDelay(1000);

		data = ((data & 0xE0FFFFFF) | 0x01000000);
		RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
	}

	/* patch LNA_PE_G1 (toggle GPIO_SWITCH) is not necessary for 3572 */
	/*
	RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
	data &= ~(0x20);
	RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);
	*/
	
	/* For RF filter Calibration */
	RTMPFilterCalibration(pAd);

	/* save R25, R26 for 2.4GHz */
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R25, &pAd->Bbp25);
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R26, &pAd->Bbp26);

	/* set led open drain enable */
	RTMP_IO_READ32(pAd, OPT_14, &data);
	data |= 0x01;
	RTMP_IO_WRITE32(pAd, OPT_14, data);
}


/*
	==========================================================================
	Description:

	Reverse RF sleep-mode setup
	
	==========================================================================
 */
VOID RT3572ReverseRFSleepModeSetup(
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
	RFValue |= 0x30;
	RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

	/* Idoh, RF R9 register Bit 1, Bit 2 & Bit 3 to 1*/
	RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
	RFValue |= 0x0E;
	RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

	/* RX_CTB_en, RF R21 register Bit 7 to 1*/
	RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
	RFValue |= 0x80;
	RT30xxWriteRFRegister(pAd, RF_R21, RFValue);
	RT30xxWriteRFRegister(pAd,RF_R08,(UCHAR)0x80);
}


VOID RT35xx_ChipSwitchChannel(
	IN PRTMP_ADAPTER 			pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan)
{
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; /*Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER;*/
	UCHAR	index;
	UINT32 	Value = 0; /*BbpReg, Value;*/
	UCHAR 	RFValue;
	UINT32 i = 0;

	i = i; /* avoid compile warning */
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
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: Can't find the Channel#%d \n", Channel));
	}
#ifdef RT35xx
	/* 3562:RFIC_3052/ 3062:RFIC_3022 */
	if (IS_RT3572(pAd) /*&& (pAd->RfIcType == RFIC_3052)*/)
	{
		for (index = 0; index < NUM_OF_3572_CHNL; index++)
		{
			if (Channel == FreqItems3572[index].Channel)
			{
				/* for 2.4G, restore BBP25, BBP26*/
				if (Channel <= 14)
				{
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, pAd->Bbp25);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, pAd->Bbp26);
				}
				/* hard code for 5GGhz, Gary 2008-12-10*/
				else
				{
					/* Enable IQ Phase Correction*/
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x09);
					/* IQ Phase correction value*/
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, 0xFF);
				}

				/* Programming channel parameters*/
				RT30xxWriteRFRegister(pAd, RF_R02, FreqItems3572[index].N);
				RT30xxWriteRFRegister(pAd, RF_R03, FreqItems3572[index].K);

				RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
				if  (Channel <= 14)
					RFValue = (RFValue & 0xF0) | FreqItems3572[index].R | 0x8;
				else
					RFValue = (RFValue & 0xF0) | FreqItems3572[index].R | 0x4;
				RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

				/* Pll mode for 2.4G or 5G*/
				RT30xxReadRFRegister(pAd, RF_R05, &RFValue);
				if  (Channel <= 14)
					RFValue = (RFValue & 0xF3) | 0x4;
				else
					RFValue = (RFValue & 0xF3) | 0x8;
				RT30xxWriteRFRegister(pAd, RF_R05, RFValue);

				/* Set Tx0 Power*/
				RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)&RFValue);
				if  (Channel <= 14)
					RFValue = 0x60 | TxPwer;
				else
					RFValue = 0xE0 | (TxPwer & 0x3) | ((TxPwer & 0xC) << 1);
				RT30xxWriteRFRegister(pAd, RF_R12, RFValue);

				/* Set Tx1 Power*/
				RT30xxReadRFRegister(pAd, RF_R13, (PUCHAR)&RFValue);
				if  (Channel <= 14)
					RFValue = 0x60 | TxPwer2;
				else
					RFValue = 0xE0 | (TxPwer2 & 0x3) | ((TxPwer2 & 0xC) << 1);
				RT30xxWriteRFRegister(pAd, RF_R13, RFValue);

				/* Tx/Rx Stream setting*/
				RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&RFValue);
				RFValue &= 0x03;	/*clear bit[7~2]*/
				if (pAd->Antenna.field.TxPath == 1)
					RFValue |= 0xA0;
				else if (pAd->Antenna.field.TxPath == 2)
					RFValue |= 0x80;
				if (pAd->Antenna.field.RxPath == 1)
					RFValue |= 0x50;
				else if (pAd->Antenna.field.RxPath == 2)
					RFValue |= 0x40;
				RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);

				/* Set RF offset*/
				RT30xxReadRFRegister(pAd, RF_R23, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0x80) | pAd->RfFreqOffset;
				RT30xxWriteRFRegister(pAd, RF_R23, (UCHAR)RFValue);

				/* Set BW*/
				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40))
				{
					RFValue = pAd->Mlme.CaliBW40RfR24;
					/*DISABLE_11N_CHECK(pAd);*/
				}
				else
				{
					RFValue = pAd->Mlme.CaliBW20RfR24;
				}
				/* R24, R31, one is for tx, the other is for rx*/
				RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)RFValue);
				RT30xxWriteRFRegister(pAd, RF_R31, (UCHAR)RFValue);

				/* Enable RF tuning*/
				RT30xxReadRFRegister(pAd, RF_R07, (PUCHAR)&RFValue);
				if  (Channel <= 14)
					/*RFValue = (RFValue & 0x37) | 0xCC;*/
					RFValue = 0xd8;	/*?? to check 3572?? hardcode*/
				else
					RFValue = (RFValue & 0x37) | 0x14;
				RT30xxWriteRFRegister(pAd, RF_R07, (UCHAR)RFValue);

				/* TSSI_BS*/
				RT30xxReadRFRegister(pAd, RF_R09, (PUCHAR)&RFValue);
				if  (Channel <= 14)
					RFValue = 0xC3; /*RFValue = (RFValue & 0xBF) | 0x40;*/
				else
					RFValue = 0xC0; /*RFValue = (RFValue & 0xBF) | 0x40;*/
				RT30xxWriteRFRegister(pAd, RF_R09, (UCHAR)RFValue);

				/* Loop filter 1*/
				RT30xxWriteRFRegister(pAd, RF_R10, (UCHAR)0xF1);

				/* Loop filter 2*/
				if  (Channel <= 14)
					RT30xxWriteRFRegister(pAd, RF_R11, (UCHAR)0xB9);
				else
					RT30xxWriteRFRegister(pAd, RF_R11, (UCHAR)0x00);

				/* tx_mx2_ic*/
				if  (Channel <= 14)
					RT30xxWriteRFRegister(pAd, RF_R15, (UCHAR)0x53);
				else
					RT30xxWriteRFRegister(pAd, RF_R15, (UCHAR)0x43);
				/* tx_mx1_ic*/
				/*RT30xxReadRFRegister(pAd, RF_R16, (PUCHAR)&RFValue);*/
				if  (Channel <= 14)
				{
					RFValue = 0x4c;

					RFValue &= (~0x7);  /* clean bit [2:0]*/
					RFValue |= pAd->TxMixerGain24G;
				}
				else 
				{
					RFValue = 0x7a;

					RFValue &= (~0x7);  /* clean bit [2:0]*/
					RFValue |= pAd->TxMixerGain5G;
				}
				RT30xxWriteRFRegister(pAd, RF_R16, (UCHAR)RFValue);

				/* tx_lo1*/
				RT30xxWriteRFRegister(pAd, RF_R17, (UCHAR)0x23);

				/* tx_lo2*/
				RFValue = ((Channel <= 14) ? (0x93) : ((Channel <= 64) ? (0xb7) : ((Channel <= 128) ? (0x74) : (0x72))));
				RT30xxWriteRFRegister(pAd, RF_R19, (UCHAR)RFValue);

				/* rx_l01*/
				RFValue = ((Channel <= 14) ? (0xB3) : ((Channel <= 64) ? (0xF6) : ((Channel <= 128) ? (0xF4) : (0xF3))));
				RT30xxWriteRFRegister(pAd, RF_R20, (UCHAR)RFValue);

				/* pfd_delay*/
				RFValue = ((Channel <= 14) ? (0x15) : ((Channel <= 64) ? (0x3d) : ((Channel <= 128) ? (0x01) : (0x01))));
				RT30xxWriteRFRegister(pAd, RF_R25, (UCHAR)RFValue);

				/* rx_lo2*/
				if  (Channel <= 14)
					RT30xxWriteRFRegister(pAd, RF_R26, (UCHAR)0x85);
				else
					RT30xxWriteRFRegister(pAd, RF_R26, (UCHAR)0x87);

				/* ldo_rf_vc*/
				if  (Channel <= 14)
					RT30xxWriteRFRegister(pAd, RF_R27, (UCHAR)0x00);
				else
					RT30xxWriteRFRegister(pAd, RF_R27, (UCHAR)0x01);

				/* drv_cc*/
				if  (Channel <= 14)
					RT30xxWriteRFRegister(pAd, RF_R29, (UCHAR)0x9B);
				else
					RT30xxWriteRFRegister(pAd, RF_R29, (UCHAR)0x9F);

				/* band selection */
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
					/* GPIO 7 for RT3562/3572 */
					if  (Channel <= 14)
						Value = ((Value & 0xFFFF7FFF) | 0x00000080);
					else
						Value = (Value & 0xFFFF7F7F);
				}
				RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);

				/* Enable RF tuning, this must be in the last*/
				RT30xxReadRFRegister(pAd, RF_R07, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x1;
				RT30xxWriteRFRegister(pAd, RF_R07, (UCHAR)RFValue);

				RTMPusecDelay(2000);

				/* latch channel for future usage.*/
				pAd->LatchRfRegs.Channel = Channel;
				
				DBGPRINT(RT_DEBUG_TRACE, ("RT35xx: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
					Channel, 
					pAd->RfIcType, 
					TxPwer,
					TxPwer2,
					pAd->Antenna.field.TxPath,
					FreqItems3572[index].N, 
					FreqItems3572[index].K, 
					FreqItems3572[index].R));
				break;
			}
		}
	}
	else
#endif /* RT35xx */
	{
		switch (pAd->RfIcType)
		{
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d : unknown RFIC=%d\n",
					  Channel, pAd->RfIcType));
				break;
		}	
	}

	/* Change BBP setting during siwtch from a->g, g->a*/
	if (Channel <= 14)
	{
		ULONG	TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* Rx High power VGA offset for LNA select*/
		{
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
		}

		/* 5G band selection PIN, bit1 and bit2 are complement*/
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

#ifdef RT35xx
		if (IS_RT3572(pAd))
			RT30xxWriteRFRegister(pAd, RF_R08, (UCHAR)0x00);
#endif /* RT35xx */

		{
			/* Turn off unused PA or LNA when only 1T or 1R*/
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg &= 0xFFFFFFF3;
			}
			if (pAd->Antenna.field.RxPath == 1)
			{
				TxPinCfg &= 0xFFFFF3FF;
			}
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

#ifdef RT35xx
		if (IS_RT3572(pAd))
			RT30xxWriteRFRegister(pAd, RF_R08, (UCHAR)0x80);
#endif /* RT35xx */

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		ULONG	TxPinCfg = 0x00050F05;/*Gary 2007/8/9 0x050505*/
		UINT8	bbpValue;
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);/*(0x44 - GET_LNA_GAIN(pAd))); According the Rory's suggestion to solve the middle range issue.     */

		/* Set the BBP_R82 value here */
		bbpValue = 0xF2;
#ifdef RT35xx
		if (IS_RT3572(pAd))		
		{
			/* TODO: check if the BBP_R82 value is the same in both of following cases!!!*/
			/* Rx High power VGA offset for LNA select*/
			bbpValue = 0x94;
		}
#endif /* RT35xx */
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
#ifdef RT35xx
		if (IS_RT3572(pAd))
			RT30xxWriteRFRegister(pAd, RF_R08, (UCHAR)0x00);
#endif /* RT35xx */

		{
			/* Turn off unused PA or LNA when only 1T or 1R*/
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg &= 0xFFFFFFF3;
			}
			if (pAd->Antenna.field.RxPath == 1)
			{
				TxPinCfg &= 0xFFFFF3FF;
			}
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

#ifdef RT35xx
		if (IS_RT3572(pAd))
			RT30xxWriteRFRegister(pAd, RF_R08, (UCHAR)0x80);
#endif /* RT35xx */
	}

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x2E + GET_LNA_GAIN(pAd)));*/
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	/* On 11A, We should delay and wait RF/BBP to be stable*/
	/* and the appropriate time should be 1000 micro seconds */
	/* 2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.*/

	RTMPusecDelay(1000);
}


VOID RT35xx_RxSensitivityTuning(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;


	R66 = 0x26 + GET_LNA_GAIN(pAd);
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
	}
	else
#endif /* RALINK_ATE */
#ifdef RT35xx
	if (IS_RT3572(pAd))
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
#endif /* RT35xx */

	DBGPRINT(RT_DEBUG_TRACE,("turn off R17 tuning, restore to 0x%02x\n", R66));
}


VOID RT35xx_NICInitAsicFromEEPROM(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR bbpreg = 0;
	UCHAR RFValue = 0;


	if (IS_RT3572(pAd))
	{	
		/* enable DC filter*/
		if ((pAd->MACVersion & 0xffff) >= 0x0201)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xc0);
		}

		/* improve power consumption */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R138, &bbpreg);
		if (pAd->Antenna.field.TxPath == 1)
		{
			/* turn off tx DAC_1			*/
			bbpreg = (bbpreg | 0x20);
		}

		if (pAd->Antenna.field.RxPath == 1)
		{
			/* turn off tx ADC_1*/
			bbpreg &= (~0x2);
		}
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R138, bbpreg);

		if ((pAd->MACVersion & 0xffff) >= 0x0211)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R31, &bbpreg);
			bbpreg &= (~0x3);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, bbpreg);
		}

		/* TX_LO1_en*/
		RT30xxReadRFRegister(pAd, RF_R16, &RFValue);

		/* set RF_R16_bit[2:0] equal to EEPROM setting at 0x48h and the value should start from 2.*/
		/*if (pAd->TxMixerGain24G >= 2)*/
		{
			RFValue &= (~0x7);  /* clean bit [2:0]*/
			RFValue |= pAd->TxMixerGain24G;
		}
		RT30xxWriteRFRegister(pAd, RF_R16, RFValue);
	}
}

VOID RT35xx_SetAGCInitValue(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth)
{
	UCHAR	R66;
	CHAR lanGain = GET_LNA_GAIN(pAd);
	
	if (pAd->LatchRfRegs.Channel <= 14)
		R66 = 0x1C + 2 * lanGain;
	else
		R66 = (UCHAR)(0x22 + (lanGain * 5) / 3);
	AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);

}


/*
	==========================================================================
	Description:
		dynamic tune BBP R66 to find a balance between sensibility and 
		noise isolation

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
UCHAR RT35xx_ChipAGCAdjust(
	IN RTMP_ADAPTER		*pAd,
	IN CHAR				Rssi,
	IN UCHAR 			OrigR66Value)
{
	UCHAR R66 = OrigR66Value;
	CHAR lanGain = GET_LNA_GAIN(pAd);

	
	if (!(IS_RT3572(pAd) || IS_RT3593(pAd)))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RT35xx_ChipAGCAdjust - Mismatch MACVersion = 0x%x \n", pAd->MACVersion));
		return R66;
	}

	if (pAd->LatchRfRegs.Channel <= 14)
	{	/*BG band*/
		R66 = 0x1C + 2 * lanGain;
		if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
			R66 += 0x20;
	}
	else
	{	/*A band*/
		if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		{
			R66 = 0x32 + (lanGain*5) / 3;
			if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				R66 += 0x10;
		}
		else
		{
			R66 = 0x3A + (lanGain*5)/3;
			if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				R66 += 0x10;
		}
		
#ifdef RT3593
		if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY) {
			RT3593_R66_MID_LOW_SENS_GET(pAd, R66);
		} else {
			RT3593_R66_NON_MID_LOW_SEMS_GET(pAd, R66);
		}
#endif /* RT3593 */
	}

	if (OrigR66Value != R66)
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	
	return R66;
}

VOID	RT35xx_ChipBBPAdjust(
	IN  RTMP_ADAPTER *pAd)
{
	UINT32 Value;
	UCHAR byteValue = 0;

#ifdef RT3593
	/* 3x3 device will not run AsicEvaluateRxAnt*/
	if (IS_RT3593(pAd))
	{
		UCHAR	BBPValue = 0;

		/* Receiver Antenna Selection*/
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPValue);
		if(pAd->Antenna.field.RxPath == 3)
		{
			BBPValue |= (0x10);
		}
		else if(pAd->Antenna.field.RxPath == 2)
		{
			BBPValue |= (0x8);
		}
		else if(pAd->Antenna.field.RxPath == 1)
		{
			BBPValue |= (0x0);
		}
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPValue);

		/*Number of transmitter chains*/
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPValue);
		BBPValue &= (~0x18);
		if (pAd->Antenna.field.TxPath == 3)
			BBPValue |= 0x10;
		else if (pAd->Antenna.field.TxPath == 2)
			BBPValue |= 0x08;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPValue);		
	}
#endif /* RT3593 */

#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
		(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE))
	{
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		/*  TX : control channel at lower */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/*  RX : control channel at lower */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		if (pAd->CommonCfg.Channel > 14)
		{ 	/* request by Gary 20070208 for middle and long range A Band*/
			if (IS_RT3572(pAd))
				AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x48, RX_CHAIN_ALL);
		}
		else
		{	/* request by Gary 20070208 for middle and long range G Band*/
			if (IS_RT3572(pAd))
				AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x38, RX_CHAIN_ALL);
		}	

		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);
		}	

		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : ExtAbove, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
									pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth, 
									pAd->CommonCfg.Channel, 
									pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
									pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
	}
	else if ((pAd->CommonCfg.Channel > 2) && 
			(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
			(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW))
	{
		pAd->CommonCfg.BBPCurrentBW = BW_40;

		if (pAd->CommonCfg.Channel == 14)
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 1;
		else
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
		/*  TX : control channel at upper */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value |= (0x1);		
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/*  RX : control channel at upper */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue |= (0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		
		if (pAd->CommonCfg.Channel > 14)
		{ 	/* request by Gary 20070208 for middle and long range A Band*/
			if (IS_RT3572(pAd))
				AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x48, RX_CHAIN_ALL);
		}
		else
		{ 	/* request by Gary 20070208 for middle and long range G band*/
			if (IS_RT3572(pAd))
				AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x38, RX_CHAIN_ALL);
		}	
			
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
		}
		else
		{	
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : ExtBlow, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
									pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth, 
									pAd->CommonCfg.Channel, 
									pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
									pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		pAd->CommonCfg.BBPCurrentBW = BW_20;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		
		/*  TX : control channel at lower */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		
		/* 20 MHz bandwidth*/
		if (pAd->CommonCfg.Channel > 14)
		{	 /* request by Gary 20070208*/
			if (IS_RT3572(pAd))
				AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x40, RX_CHAIN_ALL);
		}	
		else
		{	/* request by Gary 20070208*/
			/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x30);*/
			/* request by Brian 20070306*/
			if (IS_RT3572(pAd))
				AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x38, RX_CHAIN_ALL);
		}	
				 
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0a);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);
		}

#ifdef DOT11_N_SUPPORT
		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : 20MHz, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
										pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth, 
										pAd->CommonCfg.Channel, 
										pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
										pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif /* DOT11_N_SUPPORT */
	}
	
	if (pAd->CommonCfg.Channel > 14)
	{	/* request by Gary 20070208 for middle and long range A Band*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x1D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x1D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x1D);
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x1D);*/

		/* Disable CCK packet detection in 5G band */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x00);
			
	}
	else
	{ 	/* request by Gary 20070208 for middle and long range G band*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x2D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x2D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x2D);
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x2D);*/

		if (IS_RT3572(pAd))
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
	}	
}
VOID RT35xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;
#ifdef RT35xx
	if(IS_RT3572(pAd))
		RT30xxWriteRFRegister(pAd, RF_R08, 0x00);
#endif /* RT35xx */

	if(!IS_RT3572(pAd))
	{
			/* RF_BLOCK_en. RF R1 register Bit 0 to 0*/
			RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
			RFValue &= (~0x01);
			RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

			/* VCO_IC, RF R7 register Bit 4 & Bit 5 to 0*/
			RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
			RFValue &= (~0x30);
			RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

			/* Idoh, RF R9 register Bit 1, Bit 2 & Bit 3 to 0*/
			RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
			RFValue &= (~0x0E);
			RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

			/* RX_CTB_en, RF R21 register Bit 7 to 0*/
			RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
			RFValue &= (~0x80);
			RT30xxWriteRFRegister(pAd, RF_R21, RFValue);
	}
}
#endif /* RT35xx */

