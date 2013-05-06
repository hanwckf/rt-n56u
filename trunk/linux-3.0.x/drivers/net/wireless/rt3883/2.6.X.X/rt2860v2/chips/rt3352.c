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
	rt3352.c

	Abstract:
	Specific funcitons and variables for RT3352

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3352

#include	"rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //

UCHAR	RT3352_EeBuffer[EEPROM_SIZE] = {
	0x52, 0x33, 0x01, 0x01, 0x00, 0x0c, 0x43, 0x30, 0x52, 0x88, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 
	0x43, 0x33, 0x52, 0x77, 0x00, 0x0c, 0x43, 0x33, 0x52, 0x66, 0x22, 0x0c, 0x20, 0x00, 
	0xff, 0xff, 0x2f, 0x01, 0x55, 0x77, 0xa8, 0xaa, 0x8c, 0x88, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0d, 0x0d, 
	0x0d, 0x0d, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x10, 0x10, 
	0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x14, 0x14, 0x14, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x66, 0x66, 
	0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 
	0x88, 0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	} ;

REG_PAIR   RT3352_RFRegTable[] = {
	{RF_R00, 0xF0},
	{RF_R01, 0x23}, /* R1 bit<1,0>=11 Path setting By EEPROM */
	{RF_R02, 0x50}, /* Boot up bit7=1 */
	{RF_R03, 0x18},
	{RF_R04, 0x00},
	{RF_R05, 0x00}, /* Read only */
	{RF_R06, 0x33},
	{RF_R07, 0x00},
	{RF_R08, 0xF1}, /* By Channel Plan */
	{RF_R09, 0x02}, /* By Channel Plan */
	{RF_R10, 0xD2},
	{RF_R11, 0x42},
	{RF_R12, 0x1C},
	{RF_R13, 0x00},
	{RF_R14, 0x5A},
	{RF_R15, 0x00},
	{RF_R16, 0x01},
/*	{RF_R17, 0x1A}, By EEPROM Frequency offset */
	{RF_R18, 0x45},
	{RF_R19, 0x02},
	{RF_R20, 0x00},
	{RF_R21, 0x00},
	{RF_R22, 0x00},
	{RF_R23, 0x00},
	{RF_R24, 0x00},
	{RF_R25, 0x80},
	{RF_R26, 0x00},
	{RF_R27, 0x03},
	{RF_R28, 0x03},
	{RF_R29, 0x00},
	{RF_R30, 0x10}, /* 20MBW=0x10	40MBW=0x13 */
	{RF_R31, 0x80},
	{RF_R32, 0x80},
	{RF_R33, 0x00},
	{RF_R34, 0x01},
	{RF_R35, 0x03},
	{RF_R36, 0xBD},
	{RF_R37, 0x3C},
	{RF_R38, 0x5F},
	{RF_R39, 0xC5},
	{RF_R40, 0x33},
	{RF_R41, 0x5B},
	{RF_R42, 0x5B},
	{RF_R43, 0xDB},
	{RF_R44, 0xDB}, 
	{RF_R45, 0xDB},
	{RF_R46, 0xDD},
	{RF_R47, 0x0D},
	{RF_R48, 0x14},
	{RF_R49, 0x00},
	{RF_R50, 0x2D},
	{RF_R51, 0x7F},
	{RF_R52, 0x00},
	{RF_R53, 0x52},
	{RF_R54, 0x1B},
	{RF_R55, 0x7F},
	{RF_R56, 0x00},
	{RF_R57, 0x52},
	{RF_R58, 0x1B},
	{RF_R59, 0x00},
	{RF_R60, 0x00},
	{RF_R61, 0x00},
	{RF_R62, 0x00},
	{RF_R63, 0x00},
};

UCHAR RT3352_NUM_RF_REG_PARMS = (sizeof(RT3352_RFRegTable) / sizeof(REG_PAIR));


REG_PAIR   RT3352_BBPRegTable[] = {
	{BBP_R3,        0x00}, //use 5bit ADC for Acquisition
							/* Power saving on: 5bit mode(BBP R3[7:6]=11)
								Power saving off: 8bit mode(BBP R3[7:6]=00) 
								It shall always select ADC 0 as RX ADC input.BBP_R3[1:0]=0	*/
	{BBP_R4,		0x50}, // 2883 need to
	//The new 8-b ADC applies to the following projects: RT3352/RT3593/RT3290/RT5390 and the coming new projects.
	//BB REG: R31: 0x08. ( bit4:2 ADC buffer current: 010, bit1:0 ADC current: 00 (40uA)).
	{BBP_R31,		0x08},		//gary recommend for ACE
	{BBP_R47,		0x48},  // ALC Functions change from 0x7 to 0x48 Baron suggest 
// turn on find AGC cause QA have Rx problem 2009-10-26 in 3883
	{BBP_R65,		0x2C},		// fix rssi issue
	{BBP_R66,		0x38},	// Also set this default value to pAd->BbpTuning.R66CurrentValue at initial
	{BBP_R68,		0x0B},	// Gary 2009-05-14: for all platform
	{BBP_R69,		0x12},
	{BBP_R70,		0xa},	// BBP_R70 will change to 0x8 in ApStartUp and LinkUp for rt2860C, otherwise value is 0xa
	{BBP_R73,		0x10},
	{BBP_R78,		0x0E},
	{BBP_R80,		0x08}, // requested by Gary for high power
	{BBP_R81,		0x37},
	{BBP_R82,		0x62},
	{BBP_R83,		0x6A},
	{BBP_R84,		0x99},	// 0x19 is for rt2860E and after. This is for extension channel overlapping IOT. 0x99 is for rt2860D and before
	{BBP_R86,		0x38},	/* Gary, 20100721, for 6M sensitivity improvement */
	{BBP_R88,		0x90},	// for rt3883 middle range, Henry 2009-12-31
	{BBP_R91,		0x04},	// middle range issue, Rory @2008-01-28
	{BBP_R92,		0x02},  // middle range issue, Rory @2008-01-28

	{BBP_R103,		0xC0},
	{BBP_R104,		0x92},
	{BBP_R105,		0x34},
	{BBP_R106,		0x05},
	{BBP_R120,		0x50},	// for long range -2db, Gary 2010-01-22
	{BBP_R137,		0x0F},  // julian suggest make the RF output more stable
	{BBP_R163,		0xBD},	// Enable saving of Explicit and Implicit profiles

	{BBP_R179,		0x02},	// Set ITxBF timeout to 0x9C40=1000msec
	{BBP_R180,		0x00},
	{BBP_R182,		0x40},
	{BBP_R180,		0x01},
	{BBP_R182,		0x9C},
	{BBP_R179,		0x00},

	{BBP_R142,		0x04},	// Reprogram the inband interface to put right values in RXWI
	{BBP_R143,		0x3b},
	{BBP_R142,		0x06},
	{BBP_R143,		0xA0},
	{BBP_R142,		0x07},
	{BBP_R143,		0xA1},
	{BBP_R142,		0x08},
	{BBP_R143,		0xA2},

	{BBP_R148,		0xC8},	// Gary, 2010-02-12
};

UCHAR RT3352_NUM_BBP_REG_PARMS = (sizeof(RT3352_BBPRegTable) / sizeof(REG_PAIR));


RTMP_REG_PAIR	RT3352_MACRegTable[] =	{
	{TX_SW_CFG0,		0x402},   // Gary,2010-07-20
	{TX_SW_CFG2,		0x00},   // Gary,2010-08-17
};

UCHAR RT3352_NUM_MAC_REG_PARMS = (sizeof(RT3352_MACRegTable) / sizeof(RTMP_REG_PAIR));


#ifdef RTMP_INTERNAL_TX_ALC
TX_POWER_TUNING_ENTRY_STRUCT RT3352_TxPowerTuningTable[] =
{
// idxTxPowerTable         Tx power control over RF           Tx power control over MAC
//(zero-based array)    { RT3350: RF_R12[4:0]: Tx0 ALC},         {MAC 0x1314~0x1320}
//                      { RT3352: RF_R47[4:0]: Tx0 ALC}
//                      { RT3352: RF_R48[4:0]: Tx1 ALC}
/*  0   */                           {0x00,                                    -15},
/*  1   */                           {0x01,                                    -15},
/*  2   */                           {0x00,                                    -14},
/*  3   */                           {0x01,                                    -14},
/*  4   */                           {0x00,                                    -13},
/*  5   */                           {0x01,                                    -13},
/*  6   */                           {0x00,                                    -12},
/*  7   */                           {0x01,                                    -12},
/*  8   */                           {0x00,                                    -11},
/*  9   */                           {0x01,                                    -11},
/*  10  */                           {0x00,                                    -10},
/*  11  */                           {0x01,                                    -10},
/*  12  */                           {0x00,                                     -9},
/*  13  */                           {0x01,                                     -9},
/*  14  */                           {0x00,                                     -8},
/*  15  */                           {0x01,                                     -8},
/*  16  */                           {0x00,                                     -7},
/*  17  */                           {0x01,                                     -7},
/*  18  */                           {0x00,                                     -6},
/*  19  */                           {0x01,                                     -6},
/*  20  */                           {0x00,                                     -5},
/*  21  */                           {0x01,                                     -5},
/*  22  */                           {0x00,                                     -4},
/*  23  */                           {0x01,                                     -4},
/*  24  */                           {0x00,                                     -3},
/*  25  */                           {0x01,                                     -3},
/*  26  */                           {0x00,                                     -2},
/*  27  */                           {0x01,                                     -2},
/*  28  */                           {0x00,                                     -1},
/*  29  */                           {0x01,                                     -1},
/*  30  */                           {0x00,                                      0},
/*  31  */                           {0x01,                                      0},
/*  32  */                           {0x02,                                      0},
/*  33  */                           {0x03,                                      0},
/*  34  */                           {0x04,                                      0},
/*  35  */                           {0x05,                                      0},
/*  36  */                           {0x06,                                      0},
/*  37  */                           {0x07,                                      0},
/*  38  */                           {0x08,                                      0},
/*  39  */                           {0x09,                                      0},
/*  40  */                           {0x0A,                                      0},
/*  41  */                           {0x0B,                                      0},
/*  42  */                           {0x0C,                                      0},
/*  43  */                           {0x0D,                                      0},
/*  44  */                           {0x0E,                                      0},
/*  45  */                           {0x0F,                                      0},
/*  46  */                           {0x10,                                      0},
/*  47  */                           {0x11,                                      0},
/*  48  */                           {0x12,                                      0},
/*  49  */                           {0x13,                                      0},
/*  50  */                           {0x14,                                      0},
/*  51  */                           {0x15,                                      0},
/*  52  */                           {0x16,                                      0},
/*  53  */                           {0x17,                                      0},
/*  54  */                           {0x18,                                      0},
/*  55  */                           {0x19,                                      0},
/*  56  */                           {0x1A,                                      0},
/*  57  */                           {0x1B,                                      0},
/*  58  */                           {0x1C,                                      0},
/*  59  */                           {0x1D,                                      0},
/*  60  */                           {0x1E,                                      0},
/*  61  */                           {0x1F,                                      0},
/*  62  */                           {0x1e,                                      1},
/*  63  */                           {0x1F,                                      1},
/*  64  */                           {0x1e,                                      2},
/*  65  */                           {0x1F,                                      2},
/*  66  */                           {0x1e,                                      3},
/*  67  */                           {0x1F,                                      3},
/*  68  */                           {0x1e,                                      4},
/*  69  */                           {0x1F,                                      4},
/*  70  */                           {0x1e,                                      5},
/*  71  */                           {0x1F,                                      5},
/*  72  */                           {0x1e,                                      6},
/*  73  */                           {0x1F,                                      6},
/*  74  */                           {0x1e,                                      7},
/*  75  */                           {0x1F,                                      7},
/*  76  */                           {0x1e,                                      8},
/*  77  */                           {0x1F,                                      8},
/*  78  */                           {0x1e,                                      9},
/*  79  */                           {0x1F,                                      9},
/*  80  */                           {0x1e,                                      10},
/*  81  */                           {0x1F,                                      10},
/*  82  */                           {0x1e,                                      11},
/*  83  */                           {0x1F,                                      11},
/*  84  */                           {0x1e,                                      12},
/*  85  */                           {0x1F,                                      12},
/*  86  */                           {0x1e,                                      13},
/*  87  */                           {0x1F,                                      13},
/*  88  */                           {0x1e,                                      14},
/*  89  */                           {0x1F,                                      14},
/*  90  */                           {0x1e,                                      15},
/*  91  */                           {0x1F,                                      15},
};
#endif // RTMP_INTERNAL_TX_ALC //




/*
========================================================================
Routine Description:
	Initialize RT3352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT3352_Init(
	IN PRTMP_ADAPTER		pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	/* init capability */
	pChipCap->MaxNumOfRfId = 63;
	pChipCap->MaxNumOfBbpId = 255;
	pChipCap->pRFRegTable = RT3352_RFRegTable;
	pChipCap->pBBPRegTable = RT3352_BBPRegTable;
	pChipCap->bbpRegTbSize = RT3352_NUM_BBP_REG_PARMS;
	pChipCap->SnrFormula = SNR_FORMULA2;
	pChipCap->RfReg17WtMethod = RF_REG_WT_METHOD_STEP_ON;

#ifdef RTMP_INTERNAL_TX_ALC
	pChipCap->TxAlcTxPowerUpperBound_2G = 61;
	pChipCap->TxAlcMaxMCS = 16;
	pChipCap->TxPowerTuningTable_2G = RT3352_TxPowerTuningTable;
#endif /* RTMP_INTERNAL_TX_ALC */

	pChipCap->FlgIsHwWapiSup = TRUE;

	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_2;
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 16;
#ifdef RTMP_FLASH_SUPPORT
	pChipCap->eebuf = RT3352_EeBuffer;
#endif /* RTMP_FLASH_SUPPORT */

	/* init operator */
	pChipOps->AsicRfInit = NICInitRT3352RFRegisters;
	pChipOps->AsicBbpInit = NICInitRT3352BbpRegisters;
	pChipOps->AsicMacInit = NICInitRT3352MacRegisters;

#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = RT3352_EnableAPMIMOPS;
	pChipOps->DisableAPMIMOPS = RT3352_DisableAPMIMOPS;
#endif /* GREENAP_SUPPORT */

	pChipOps->RxSensitivityTuning = RT3352_RxSensitivityTuning;
#ifdef CONFIG_STA_SUPPORT
	pChipOps->ChipAGCAdjust = RT3352_ChipAGCAdjust;
#endif /* CONFIG_STA_SUPPORT */
	pChipOps->ChipBBPAdjust = RT3352_ChipBBPAdjust;
	pChipOps->ChipSwitchChannel = RT3352_ChipSwitchChannel;
	pChipOps->ChipAGCInit = RT3352_RTMPSetAGCInitValue;
#ifdef CARRIER_DETECTION_SUPPORT
	pAd->chipCap.carrier_func = TONE_RADAR_V2;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v2;
#endif /* CARRIER_DETECTION_SUPPORT */

	RtmpChipBcnSpecInit(pAd);
}


/*
========================================================================
Routine Description:
	Initialize specific MAC registers for RT3352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT3352MacRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT32 IdReg;


	for(IdReg=0; IdReg<RT3352_NUM_MAC_REG_PARMS; IdReg++)
	{
		RTMP_IO_WRITE32(pAd, RT3352_MACRegTable[IdReg].Register,
								RT3352_MACRegTable[IdReg].Value);
	}
}


/*
========================================================================
Routine Description:
	Initialize specific BBP registers for RT3352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT3352BbpRegisters(
	IN	PRTMP_ADAPTER pAd)
{
//	UCHAR BBPR3 = 0;


	/*
		For power saving purpose, Gary set BBP_R3[7:6]=11 to save more power
		and he also rewrote the description about BBP_R3 to point out the
		WiFi driver should modify BBP_R3[5] based on Low/High frequency
		channel.(not a fixed value).
	*/
//	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
//	BBPR3 |= 0xe0;	//bit 6 & 7, i.e. Use 5-bit ADC for Acquisition
//	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
}


/*
========================================================================
Routine Description:
	Initialize specific RF registers for RT3352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT3352RFRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT8 RfReg = 0;
	ULONG value = 0;
	int i;

	// Driver should toggle RF R02 bit7 before init RF registers
	//RF_R02: Resistor calibration, RF_R02 = RF_R30 (RT30xx)
	RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
	RfReg &= ~(1 << 6); // clear bit6=rescal_bp
	RfReg |= 0x80; // bit7=rescal_en
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
	RTMPusecDelay(1000);

	// Initialize RF register to default value
	for (i = 0; i < RT3352_NUM_RF_REG_PARMS; i++)
	{
		RT30xxWriteRFRegister(pAd, RT3352_RFRegTable[i].Register, RT3352_RFRegTable[i].Value);
	}

	/* Gary: Boot up bit7=1 */
        RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
        RfReg &= ~(1 << 6); // clear bit6=rescal_bp
        RfReg |= 0x80; // bit7=rescal_en
        RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
        RTMPusecDelay(1000);
        RfReg &= 0x7F;
        RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);

	if (value!=0xFFFF)
	{
		/* EEPROM is empty */
		if (value & (1<<14))
		{ 
			/* TX0: 0=internal PA, 1=external PA */
			RT30xxReadRFRegister(pAd, RF_R34, (PUCHAR)&RfReg);
			RfReg |= (0x1 << 2); /* tx0_lowgain=20db attenuation */
			RT30xxWriteRFRegister(pAd, RF_R34, (UCHAR)RfReg);
		   
			RfReg = 0x52; 
			RT30xxWriteRFRegister(pAd, RF_R41, (UCHAR)RfReg);
			
			RT30xxReadRFRegister(pAd, RF_R50, (PUCHAR)&RfReg);
			RfReg |= 0x7;
			RT30xxWriteRFRegister(pAd, RF_R50, (UCHAR)RfReg);
			
			RfReg = 0x52; 
			RT30xxWriteRFRegister(pAd, RF_R51, (UCHAR)RfReg);
			
			RfReg = 0xC0; 
			RT30xxWriteRFRegister(pAd, RF_R52, (UCHAR)RfReg);
			
			RfReg = 0xD2; 
			RT30xxWriteRFRegister(pAd, RF_R53, (UCHAR)RfReg);
			
			RfReg = 0xC0; 
			RT30xxWriteRFRegister(pAd, RF_R54, (UCHAR)RfReg);
		}
		else
		{
			RT30xxReadRFRegister(pAd, RF_R34, (PUCHAR)&RfReg);
			RfReg &= ~(0x1 << 2);
			RT30xxWriteRFRegister(pAd, RF_R34, (UCHAR)RfReg);
			
			RfReg = 0x5B; 
			RT30xxWriteRFRegister(pAd, RF_R41, (UCHAR)RfReg);
			
			RT30xxReadRFRegister(pAd, RF_R50, (PUCHAR)&RfReg);
			RfReg &= ~0x7;
			RfReg |= 0x5;
			RT30xxWriteRFRegister(pAd, RF_R50, (UCHAR)RfReg);
			
			RfReg = 0x7F; 
			RT30xxWriteRFRegister(pAd, RF_R51, (UCHAR)RfReg);
			
			RfReg = 0x00; 
			RT30xxWriteRFRegister(pAd, RF_R52, (UCHAR)RfReg);
			
			RfReg = 0x52; 
			RT30xxWriteRFRegister(pAd, RF_R53, (UCHAR)RfReg);
			
			RfReg = 0x1B; 
			RT30xxWriteRFRegister(pAd, RF_R54, (UCHAR)RfReg);
		}

		if (value & (1<<15))
		{ 
			/* TX1: 0=internal PA, 1=external PA */
			RT30xxReadRFRegister(pAd, RF_R34, (PUCHAR)&RfReg);
			RfReg |= (0x1 << 3); /* tx1_lowgain=20db attenuation */
			RT30xxWriteRFRegister(pAd, RF_R34, (UCHAR)RfReg);
			
			RfReg = 0x52; 
			RT30xxWriteRFRegister(pAd, RF_R42, (UCHAR)RfReg);
			
			RT30xxReadRFRegister(pAd, RF_R50, (PUCHAR)&RfReg);
			RfReg |= (0x7<<3);
			RT30xxWriteRFRegister(pAd, RF_R50, (UCHAR)RfReg);
			
			RfReg = 0x52; 
			RT30xxWriteRFRegister(pAd, RF_R55, (UCHAR)RfReg);
			
			RfReg = 0xC0; 
			RT30xxWriteRFRegister(pAd, RF_R56, (UCHAR)RfReg);
			
			RfReg = 0x49; 
			RT30xxWriteRFRegister(pAd, RF_R57, (UCHAR)RfReg);
			
			RfReg = 0xC0; 
			RT30xxWriteRFRegister(pAd, RF_R58, (UCHAR)RfReg);
		}
		else
		{
			RT30xxReadRFRegister(pAd, RF_R34, (PUCHAR)&RfReg);
			RfReg &= ~(0x1 << 3); 
			RT30xxWriteRFRegister(pAd, RF_R34, (UCHAR)RfReg);

			RfReg = 0x5B; 
			RT30xxWriteRFRegister(pAd, RF_R42, (UCHAR)RfReg);
			
			RT30xxReadRFRegister(pAd, RF_R50, (PUCHAR)&RfReg);
			RfReg &= ~(0x7<<3);
			RfReg |= (0x5<<3);
			RT30xxWriteRFRegister(pAd, RF_R50, (UCHAR)RfReg);
			
			RfReg = 0x7F; 
			RT30xxWriteRFRegister(pAd, RF_R55, (UCHAR)RfReg);
			
			RfReg = 0x00; 
			RT30xxWriteRFRegister(pAd, RF_R56, (UCHAR)RfReg);
			
			RfReg = 0x52; 
			RT30xxWriteRFRegister(pAd, RF_R57, (UCHAR)RfReg);
			
			RfReg = 0x1B; 
			RT30xxWriteRFRegister(pAd, RF_R58, (UCHAR)RfReg);
		}
	}
}


#ifdef GREENAP_SUPPORT
extern REG_PAIR RT305x_RFRegTable[];

VOID RT3352_EnableAPMIMOPS(
	IN PRTMP_ADAPTER			pAd,
	IN BOOLEAN					ReduceCorePower)
{
	UCHAR	BBPR3 = 0,BBPR1 = 0;
	ULONG	TxPinCfg = 0x00050F0A;//Gary 2007/08/09 0x050A0A
	UCHAR	BBPR4=0;

	UCHAR	CentralChannel;
	//UINT32	Value=0;


#ifdef RT305x
	UCHAR 	RFValue=0;
		
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue &= 0x03;	//clear bit[7~2]
	RFValue |= 0x3C; // default 2Tx 2Rx
	// turn off tx1
	RFValue &= ~(0x1 << 5);
	// turn off rx1
	RFValue &= ~(0x1 << 4);
	// Turn off unused PA or LNA when only 1T or 1R
#endif // RT305x //

	if(pAd->CommonCfg.Channel>14)
		TxPinCfg=0x00050F05;
		
	TxPinCfg &= 0xFFFFFFF3;
	TxPinCfg &= 0xFFFFF3FF;
	pAd->ApCfg.bGreenAPActive=TRUE;

	CentralChannel = pAd->CommonCfg.CentralChannel;
#ifdef RTMP_RBUS_SUPPORT
#endif // RTMP_RBUS_SUPPORT //
		DBGPRINT(RT_DEBUG_INFO, ("Run with BW_20\n"));
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		CentralChannel = pAd->CommonCfg.Channel;
	/* Set BBP registers to BW20 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);
		BBPR4 &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPR4);
	/* RF Bandwidth related registers would be set in AsicSwitchChannel() */
		pAd->CommonCfg.BBPCurrentBW = BW_20;
	if (pAd->Antenna.field.RxPath>1||pAd->Antenna.field.TxPath>1)
	{
		//TX Stream
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
		//Rx Stream
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
		
		
	BBPR3 &= (~0x18);
	BBPR1 &= (~0x18);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);

#ifdef RT3352
	/*
		For power saving purpose, Gary set BBP_R3[7:6]=11 to save more power
		and he also rewrote the description about BBP_R3 to point out the
		WiFi driver should modify BBP_R3[5] based on Low/High frequency
		channel.(not a fixed value).
	*/
	BBPR3 |= 0xe0;	//bit 6 & 7, i.e. Use 5-bit ADC for Acquisition
#endif // RT3352 //

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
		
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

#ifdef RT305x
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);
#endif // RT305x //
	}
	AsicSwitchChannel(pAd, CentralChannel, FALSE);

	DBGPRINT(RT_DEBUG_INFO, ("EnableAPMIMOPS, 305x/28xx changes the # of antenna to 1\n"));
}


VOID RT3352_DisableAPMIMOPS(
	IN PRTMP_ADAPTER			pAd)
{
	UCHAR	BBPR3=0,BBPR1=0;
	ULONG	TxPinCfg = 0x00050F0A;//Gary 2007/08/09 0x050A0A

	UCHAR	CentralChannel;
	UINT32	Value=0;


#ifdef RT305x
	UCHAR 	RFValue=0;

	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue &= 0x03;	//clear bit[7~2]
	RFValue |= 0x3C; // default 2Tx 2Rx
#endif // RT305x //

	if(pAd->CommonCfg.Channel>14)
		TxPinCfg=0x00050F05;
	// Turn off unused PA or LNA when only 1T or 1R
	if (pAd->Antenna.field.TxPath == 1)
	{
		TxPinCfg &= 0xFFFFFFF3;
	}
	if (pAd->Antenna.field.RxPath == 1)
	{
		TxPinCfg &= 0xFFFFF3FF;
	}


	pAd->ApCfg.bGreenAPActive=FALSE;

#ifdef RTMP_RBUS_SUPPORT
#endif // RTMP_RBUS_SUPPORT //
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) && (pAd->CommonCfg.Channel != 14))
		{
			DBGPRINT(RT_DEBUG_INFO, ("Run with BW_40\n"));
			/* Set CentralChannel to work for BW40 */
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		{
				pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		
			//  TX : control channel at lower 
			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
			Value &= (~0x1);
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

			//  RX : control channel at lower 
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
		}
		else if ((pAd->CommonCfg.Channel > 2) && (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW)) 
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
			
			//  TX : control channel at upper 
			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
			Value |= (0x1);		
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
			
			//  RX : control channel at upper 
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value |= (0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
		}
		CentralChannel = pAd->CommonCfg.CentralChannel;

		/* Set BBP registers to BW40 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		Value |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);
		/* RF Bandwidth related registers would be set in AsicSwitchChannel() */
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, CentralChannel, FALSE);
	}
	//Rx Stream
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
	//Tx Stream
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= (~0x18);

	//RX Stream
	if(pAd->Antenna.field.RxPath == 3)
	{
		BBPR3 |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		BBPR3 |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		BBPR3 |= (0x0);
	}

	//Tx Stream
	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->Antenna.field.TxPath == 2))
	{
		BBPR1 &= (~0x18);
		BBPR1 |= 0x10;
	}
	else
	{
		BBPR1 &= (~0x18);
	}

#ifdef RT3352
	/*
		For power saving purpose, Gary set BBP_R3[7:6]=11 to save more power
		and he also rewrote the description about BBP_R3 to point out the
		WiFi driver should modify BBP_R3[5] based on Low/High frequency
		channel.(not a fixed value).
	*/
	BBPR3 &= (~0xe0);	//bit 6 & 7, i.e. Use 5-bit ADC for Acquisition
#endif // RT3352 //

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

#ifdef RT305x
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);
#endif // RT305x //

	DBGPRINT(RT_DEBUG_INFO, ("DisableAPMIMOPS, 305x/28xx reserve only one antenna\n"));
}
#endif // GREENAP_SUPPORT //


VOID RT3352_RxSensitivityTuning(
	IN PRTMP_ADAPTER			pAd)
{
	UCHAR R66;


	R66 = 0x26 + GET_LNA_GAIN(pAd);
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
#ifdef RTMP_RBUS_SUPPORT
		// TODO: we need to add MACVersion Check here!!!!
#if defined(RT3352)
		if (IS_RT3352(pAd))
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x0);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x20);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
		}
		else
#endif // defined(RT3352) //
#endif // RTMP_RBUS_SUPPORT //
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
	}
	else
#endif // RALINK_ATE //
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);

	DBGPRINT(RT_DEBUG_TRACE,("turn off R17 tuning, restore to 0x%02x\n", R66));
}


#ifdef CONFIG_STA_SUPPORT
UCHAR RT3352_ChipAGCAdjust(
	IN PRTMP_ADAPTER		pAd,
	IN CHAR					Rssi,
	IN UCHAR				OrigR66Value)
{
	UCHAR R66 = OrigR66Value;
	CHAR lanGain = GET_LNA_GAIN(pAd);
	
	
	if (pAd->LatchRfRegs.Channel <= 14)
	{	//BG band
		if (IS_RT3352(pAd))
		{
			if (pAd->CommonCfg.BBPCurrentBW == BW_20)
			    R66 = (lanGain * 2 +0x1C);
			else
			    R66 = (lanGain * 2 +0x24);
		}
		else
		{
			R66 = 0x2E + lanGain;
			if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				R66 += 0x10;
		}
	}
	else
	{	//A band
		if (pAd->CommonCfg.BBPCurrentBW == BW_20)
			R66 = 0x32 + (lanGain * 5)/3;
		else
			R66 = 0x3A + (lanGain * 5)/3;
		
		if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
			R66 += 0x10;
	}

	if (OrigR66Value != R66)
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	
	return R66;
}
#endif // CONFIG_STA_SUPPORT //


VOID RT3352_ChipBBPAdjust(
	IN RTMP_ADAPTER			*pAd)
{
	UINT32 Value;
	UCHAR byteValue = 0;
	UCHAR R66;

#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
		(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		/*(pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)*/
	)
	{
		{
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		}
		//  TX : control channel at lower 
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		//  RX : control channel at lower 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		if (pAd->CommonCfg.Channel > 14)
		{ 	// request by Gary 20070208 for middle and long range A Band
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x48, RX_CHAIN_ALL);
		}
		else
		{	// request by Gary 20070208 for middle and long range G Band
			if (IS_RT3352(pAd))
			{
				/* Gary 20100714: Update BBP R66 programming: */
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = GET_LNA_GAIN(pAd)*2 + 0x1C;
				else
					R66 = GET_LNA_GAIN(pAd)*2 + 0x24;

				AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
			}
		}	
		// 
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
			(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW)
			/*(pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW)*/)
	{
		pAd->CommonCfg.BBPCurrentBW = BW_40;

		if (pAd->CommonCfg.Channel == 14)
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 1;
		else
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
		//  TX : control channel at upper 
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value |= (0x1);		
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		//  RX : control channel at upper 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue |= (0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		
		if (pAd->CommonCfg.Channel > 14)
		{ 	// request by Gary 20070208 for middle and long range A Band
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x48, RX_CHAIN_ALL);
		}
		else
		{ 	// request by Gary 20070208 for middle and long range G band
			if (IS_RT3352(pAd))
			{
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = GET_LNA_GAIN(pAd)*2 + 0x1C;
				else
					R66 = GET_LNA_GAIN(pAd)*2 + 0x24;
				AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
			}
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
#endif // DOT11_N_SUPPORT //
	{
		pAd->CommonCfg.BBPCurrentBW = BW_20;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		
		//  TX : control channel at lower 
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		
		// 20 MHz bandwidth
		if (pAd->CommonCfg.Channel > 14)
		{	 // request by Gary 20070208
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x40, RX_CHAIN_ALL);
		}	
		else
		{	// request by Gary 20070208
			//AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x30, RX_CHAIN_ALL);
			// request by Brian 20070306
			if (IS_RT3352(pAd))
			{
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = GET_LNA_GAIN(pAd)*2 + 0x1C;
				else
					R66 = GET_LNA_GAIN(pAd)*2 + 0x24;
				AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
			}
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
#endif // DOT11_N_SUPPORT //
	}
	
	if (pAd->CommonCfg.Channel > 14)
	{	// request by Gary 20070208 for middle and long range A Band
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x1D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x1D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x1D);
		//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x1D);
	}
	else
	{ 	// request by Gary 20070208 for middle and long range G band
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x2D);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x2D);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x2D);
			//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x2D);
	}	
}


VOID RT3352_ChipSwitchChannel(
	IN PRTMP_ADAPTER 			pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan) 
{
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; //Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER;
	UCHAR	index;
	UINT32 	Value = 0; //BbpReg, Value;
	UCHAR 	RFValue;
	UINT32 i = 0;

	i = i; /* avoid compile warning */
	RFValue = 0;
	// Search Tx power value

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

#ifdef RT305x
	// The RF programming sequence is difference between 3xxx and 2xxx
	if (((pAd->MACVersion == 0x28720200)
#ifdef RT3352
		|| IS_RT3352(pAd)
#endif // RT3352 //
		) && 
		((pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3322) ||
		(pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022)))
	{
		/* modify by WY for Read RF Reg. error */
		
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
#if defined (RT3352)
				if (IS_RT3352(pAd))
				{
					// Programming channel parameters
					Value = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

					if(Value & (1<<20)) { //Xtal=40M
						RT30xxWriteRFRegister(pAd, RF_R08, FreqItems3020[index].N);
						RT30xxWriteRFRegister(pAd, RF_R09, FreqItems3020[index].K);
					}else {
						RT30xxWriteRFRegister(pAd, RF_R08, FreqItems3020_Xtal20M[index].N);
						RT30xxWriteRFRegister(pAd, RF_R09, FreqItems3020_Xtal20M[index].K);
					}

					RFValue = 0x42;
					RT30xxWriteRFRegister(pAd, RF_R11, (UCHAR)RFValue);

					RFValue = 0x1C;
					RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)RFValue);

					RFValue = 0x00;
					RT30xxWriteRFRegister(pAd, RF_R13, (UCHAR)RFValue);
					
					// Set RF offset  RF_R17=RF_R23 (RT30xx)
					RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RFValue);
					RFValue = (RFValue & 0x80) | pAd->RfFreqOffset;
					RT30xxWriteRFRegister(pAd, RF_R17, (UCHAR)RFValue);

					RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RFValue);
					if ((pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef RTMP_RBUS_SUPPORT
#endif // RTMP_RBUS_SUPPORT //
					)
						RFValue |= 0x03; // 40MBW tx_h20M=1,rx_h20M=1
					else
						RFValue &= ~(0x03); // 20MBW tx_h20M=0,rx_h20M=0
					RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RFValue);


					for (i = 0; i < MAX_NUM_OF_CHANNELS; i++) {
						if (Channel != pAd->TxPower[i].Channel)
							continue;

						RT30xxWriteRFRegister(pAd, RF_R47, pAd->TxPower[i].Power);
						RT30xxWriteRFRegister(pAd, RF_R48, pAd->TxPower[i].Power2);
						break;
					}
					
					RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
					RFValue = RFValue | 0x80; // bit 7=vcocal_en
					RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);

					RTMPusecDelay(2000);

					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, RFValue & 0xfe); // clear update flag
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);

					// latch channel for future usage.
					pAd->LatchRfRegs.Channel = Channel;
				}
#endif // RT3352 //

#ifdef RT3352
				if (IS_RT3352(pAd))
					RFValue = 0; /* RF_R24 is reserved bits */
#endif // RT3352 //

				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT3352
					if (IS_RT3352(pAd))
						RFValue = 0; /* RF_R24 is reserved bits */
#endif // RT3352 //
				}
				RT30xxWriteRFRegister(pAd, RF_R24, RFValue);

				// Rx filter
				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT3352
					if (IS_RT3352(pAd))
						RT30xxWriteRFRegister(pAd, RF_R31, 0x80); //FIXME: I don't know the RF_R31 for BW40 case
#endif // RT3352 //
				}
				else
				{
#ifdef RT3352
					if (IS_RT3352(pAd))
						RT30xxWriteRFRegister(pAd, RF_R31, 0x80);
#endif // RT3352 //
				}

#if defined (RT3352)
				if (IS_RT3352(pAd))
				{
					// Enable RF tuning, this must be in the last, RF_R03=RF_R07 (RT30xx)
					RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
					RFValue = RFValue | 0x80; // bit 7=vcocal_en
					RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);

					RTMPusecDelay(2000);
					
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, RFValue & 0xfe); // clear update flag
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);

					// Antenna
					RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue &= 0x03; //clear bit[7~2]
					RFValue |= 0x3C; // default 2Tx 2Rx

					if (pAd->Antenna.field.TxPath == 1)
						RFValue &= ~(0x1 << 5);

					if (pAd->Antenna.field.RxPath == 1)
						RFValue &= ~(0x1 << 4);

					RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);
				}
#endif // RT3352 //

				// latch channel for future usage.
				pAd->LatchRfRegs.Channel = Channel;
				
				break;				
			}
		}

#if defined (RT3352)
		Value = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

		if(Value & (1<<20)) { //Xtal=40M
		    DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
				Channel, 
				pAd->RfIcType, 
				TxPwer,
				TxPwer2,
				pAd->Antenna.field.TxPath,
				FreqItems3020[index].N, 
				FreqItems3020[index].K, 
				FreqItems3020[index].R));
		}else {
		    DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
				Channel, 
				pAd->RfIcType, 
				TxPwer,
				TxPwer2,
				pAd->Antenna.field.TxPath,
				FreqItems3020_Xtal20M[index].N, 
				FreqItems3020_Xtal20M[index].K, 
				FreqItems3020_Xtal20M[index].R));
		}
#endif // RT3352 //
	}
	else
#endif // RT305x //
	{
		switch (pAd->RfIcType)
		{
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d : unknown RFIC=%d\n",
					  Channel, pAd->RfIcType));
				break;
		}	
	}

	// Change BBP setting during siwtch from a->g, g->a
	if (Channel <= 14)
	{
		ULONG	TxPinCfg = 0x00050F0A;//Gary 2007/08/09 0x050A0A

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
#if defined(RT3352)
		if (IS_RT3352(pAd))
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38); // Gary 2010-07-21
#endif // RT3352 //

		// Rx High power VGA offset for LNA select
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

#if defined (RT3352)
		if (IS_RT3352(pAd))
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x6a);
		}
#endif // RT3352 //

		// 5G band selection PIN, bit1 and bit2 are complement
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		{
			// Turn off unused PA or LNA when only 1T or 1R
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

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		ULONG	TxPinCfg = 0x00050F05;//Gary 2007/8/9 0x050505
		UINT8	bbpValue;
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);//(0x44 - GET_LNA_GAIN(pAd)));   // According the Rory's suggestion to solve the middle range issue.     

		/* Set the BBP_R82 value here */
		bbpValue = 0xF2;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, bbpValue);

		// Rx High power VGA offset for LNA select
		if (pAd->NicConfig2.field.ExternalLNAForA)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		// 5G band selection PIN, bit1 and bit2 are complement
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		// Turn off unused PA or LNA when only 1T or 1R
		{
			// Turn off unused PA or LNA when only 1T or 1R
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
	}

	// R66 should be set according to Channel and use 20MHz when scanning
	//AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x2E + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL);
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	//
	// On 11A, We should delay and wait RF/BBP to be stable
	// and the appropriate time should be 1000 micro seconds 
	// 2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.
	//
	RTMPusecDelay(1000);  
}


VOID RT3352_RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth)
{
	UCHAR	R66 = 0x30;
	CHAR lanGain = GET_LNA_GAIN(pAd);
	
	if (pAd->LatchRfRegs.Channel <= 14)
	{	// BG band
		{
			R66 = 0x2E + lanGain;
#if defined(RT3352)
			if (IS_RT3352(pAd))
			{
				/* Gary 20100714: Update BBP R66 programming: */
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = (lanGain * 2 + 0x1C);
				else
					R66 = (lanGain * 2 + 0x24);
			}
#endif /* RT3352 */
			AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
		}
	}
	else
	{	//A band
		{	
			if (BandWidth == BW_20)
				R66 = (UCHAR)(0x32 + (lanGain * 5) / 3);
#ifdef DOT11_N_SUPPORT
			else
				R66 = (UCHAR)(0x3A + (lanGain * 5) / 3);
#endif // DOT11_N_SUPPORT //
			AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
		}		
	}

}

#endif // RT3352 //

/* End of rt3352.c */
