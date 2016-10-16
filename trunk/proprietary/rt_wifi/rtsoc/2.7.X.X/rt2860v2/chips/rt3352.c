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
	{RF_R02, 0x50}, /* Fix Power DAC Variation.(20110928) */
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
	/* 
		Power saving on: 5bit mode(BBP R3[7:6]=11)
		Power saving off: 8bit mode(BBP R3[7:6]=00)
	*/
	/* 0x01 --> 0x00 for packet detection (2011/7/1) */
	/* It shall always select ADC 0 as RX ADC input.BBP_R3[1:0]=0 */
	{BBP_R3,        0x00}, /* use 5bit ADC for Acquisition */
	{BBP_R4,		0x50}, // 2883 need to
	//The new 8-b ADC applies to the following projects: RT3352/RT3593/RT3290/RT5390 and the coming new projects.
	//BB REG: R31: 0x08. ( bit4:2 ADC buffer current: 010, bit1:0 ADC current: 00 (40uA)).
	{BBP_R31,		0x08},		//gary recommend for ACE
	{BBP_R47,		0x48},  // ALC Functions change from 0x7 to 0x48 Baron suggest 
// turn on find AGC cause QA have Rx problem 2009-10-26 in 3883
	{BBP_R65,		0x2C},		// fix rssi issue
	{BBP_R66,		0x38},	// Also set this default value to pAd->BbpTuning.R66CurrentValue at initial
	{BBP_R68,		0x0B},	// Gary 2009-05-14: for all platform
	{BBP_R69,		0x1C},
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
	{BBP_R106,		0x1D},
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
/*	idxTxPowerTable		Tx power control over RF			Tx power control over MAC */
/*  	(zero-based array)   	{ RT3350: RF_R12[4:0]: Tx0 ALC},  	{MAC 0x1314~0x1320} */
/*                      			{ RT3352: RF_R47[4:0]: Tx0 ALC} */
/*                      			{ RT3352: RF_R48[4:0]: Tx1 ALC} */
/*  0   */                         	{0x00,                                   		-15},
/*  1   */                         	{0x01,                                    		-15},
/*  2   */                         	{0x00,                                    		-14},
/*  3   */                         	{0x01,                                    		-14},
/*  4   */                         	{0x00,                                    		-13},
/*  5   */                         	{0x01,                                    		-13},
/*  6   */                         	{0x00,                                    		-12},
/*  7   */                         	{0x01,                                    		-12},
/*  8   */                         	{0x00,                                    		-11},
/*  9   */                         	{0x01,                                    		-11},
/*  10  */                         	{0x00,                                    		-10},
/*  11  */                         	{0x01,                                    		-10},
/*  12  */                         	{0x00,                                     		-9},
/*  13  */                         	{0x01,                                     		-9},
/*  14  */                         	{0x00,                                     		-8},
/*  15  */                         	{0x01,                                     		-8},
/*  16  */                         	{0x00,                                     		-7},
/*  17  */                         	{0x01,                                     		-7},
/*  18  */                         	{0x00,                                     		-6},
/*  19  */                         	{0x01,                                     		-6},
/*  20  */                         	{0x00,                                     		-5},
/*  21  */                         	{0x01,                                     		-5},
/*  22  */                         	{0x00,                                     		-4},
/*  23  */                         	{0x01,                                     		-4},
/*  24  */                         	{0x00,                                     		-3},
/*  25  */                         	{0x01,                                     		-3},
/*  26  */                         	{0x00,                                     		-2},
/*  27  */                         	{0x01,                                     		-2},
/*  28  */                         	{0x00,                                     		-1},
/*  29  */                          {0x01,                                     		-1},
/*  30  */                          {0x00,                                      	0},
/*  31  */                          {0x01,                                      	0},
/*  32  */                          {0x02,                                      	0},
/*  33  */                          {0x03,                                      	0},
/*  34  */                          {0x04,                                      	0},
/*  35  */                          {0x05,                                      	0},
/*  36  */                          {0x06,                                      	0},
/*  37  */                          {0x07,                                      	0},
/*  38  */                          {0x08,                                      	0},
/*  39  */                          {0x09,                                      	0},
/*  40  */                          {0x0A,                                      	0},
/*  41  */                          {0x0B,                                      	0},
/*  42  */                          {0x0C,                                      	0},
/*  43  */                          {0x0D,                                      	0},
/*  44  */                          {0x0E,                                      	0},
/*  45  */                          {0x0F,                                      	0},
/*  46  */                          {0x10,                                      	0},
/*  47  */                          {0x11,                                      	0},
/*  48  */                          {0x12,                                      	0},
/*  49  */                          {0x13,                                      	0},
/*  50  */                          {0x14,                                      	0},
/*  51  */                          {0x15,                                      	0},
/*  52  */                          {0x16,                                      	0},
/*  53  */                          {0x17,                                      	0},
/*  54  */                          {0x18,                                      	0},
/*  55  */                          {0x19,                                      	0},
/*  56  */                          {0x1A,                                      	0},
/*  57  */                          {0x1B,                                      	0},
/*  58  */                          {0x1C,                                      	0},
/*  59  */                          {0x1D,                                      	0},
/*  60  */                          {0x1E,                                      	0},
/*  61  */                          {0x1F,                                      	0},
/*  62  */                          {0x1e,                                      	1},
/*  63  */                          {0x1F,                                      	1},
/*  64  */                          {0x1e,                                      	2},
/*  65  */                          {0x1F,                                      	2},
/*  66  */                          {0x1e,                                      	3},
/*  67  */                          {0x1F,                                      	3},
/*  68  */                          {0x1e,                                      	4},
/*  69  */                          {0x1F,                                      	4},
/*  70  */                          {0x1e,                                      	5},
/*  71  */                          {0x1F,                                      	5},
/*  72  */                          {0x1e,                                      	6},
/*  73  */                          {0x1F,                                      	6},
/*  74  */                          {0x1e,                                      	7},
/*  75  */                          {0x1F,                                      	7},
/*  76  */                          {0x1e,                                      	8},
/*  77  */                          {0x1F,                                      	8},
/*  78  */                          {0x1e,                                      	9},
/*  79  */                          {0x1F,                                      	9},
/*  80  */                          {0x1e,                                      	10},
/*  81  */                          {0x1F,                                      	10},
/*  82  */                          {0x1e,                                      	11},
/*  83  */                          {0x1F,                                      	11},
/*  84  */                          {0x1e,                                      	12},
/*  85  */                          {0x1F,                                      	12},
/*  86  */                          {0x1e,                                      	13},
/*  87  */                          {0x1F,                                      	13},
/*  88  */                          {0x1e,                                      	14},
/*  89  */                          {0x1F,                                      	14},
/*  90  */                          {0x1e,                                      	15},
/*  91  */                          {0x1F,                                      	15},
};

/* The desired TSSI over CCK */
extern CHAR desiredTSSIOverCCK[4];

/* The desired TSSI over OFDM */
extern CHAR desiredTSSIOverOFDM[8];

/* The desired TSSI over HT */
extern CHAR desiredTSSIOverHT[16];

/* The desired TSSI over HT using STBC */
extern CHAR desiredTSSIOverHTUsingSTBC[8];

VOID RT3352_AsicInitDesiredTSSITable(IN PRTMP_ADAPTER pAd);
#endif /* RTMP_INTERNAL_TX_ALC */

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
	pChipOps->AsicGetTxPowerOffset = AsicGetTxPowerOffset;

#ifdef RTMP_INTERNAL_TX_ALC
	pChipCap->TxAlcTxPowerUpperBound_2G = 61;
	pChipCap->TxPowerMaxCompenStep = 8; /* default 4dB (one step is 0.5dB) */
	pChipCap->TxPowerTableMaxIdx = 0; 
	pChipCap->TxPowerTuningTable_2G = RT3352_TxPowerTuningTable;
	pChipOps->InitDesiredTSSITable = RT3352_InitDesiredTSSITable;
	pChipOps->AsicTxAlcGetAutoAgcOffset = RT3352_AsicTxAlcGetAutoAgcOffset;
#endif /* RTMP_INTERNAL_TX_ALC */

	pChipCap->FlgIsHwWapiSup = TRUE;

	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_2;
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 16;
#ifdef RTMP_FLASH_SUPPORT
	pChipCap->eebuf = RT3352_EeBuffer;
#endif /* RTMP_FLASH_SUPPORT */

#ifdef NEW_MBSSID_MODE
	pChipCap->MBSSIDMode = MBSSID_MODE1;
#else
	pChipCap->MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */


	/* init operator */
	pChipOps->AsicRfInit = NICInitRT3352RFRegisters;
	pChipOps->AsicBbpInit = NICInitRT3352BbpRegisters;
	pChipOps->AsicMacInit = NICInitRT3352MacRegisters;

#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = EnableAPMIMOPSv1;
	pChipOps->DisableAPMIMOPS = DisableAPMIMOPSv1;
#endif /* GREENAP_SUPPORT */

	pChipOps->RxSensitivityTuning = RT3352_RxSensitivityTuning;
#ifdef CONFIG_STA_SUPPORT
	pChipOps->ChipAGCAdjust = RT3352_ChipAGCAdjust;
#endif /* CONFIG_STA_SUPPORT */
	pChipOps->ChipBBPAdjust = RT3352_ChipBBPAdjust;
	pChipOps->ChipSwitchChannel = RT3352_ChipSwitchChannel;
	pChipOps->AsicAdjustTxPower = AsicAdjustTxPower;
	pChipOps->ChipAGCInit = RT3352_RTMPSetAGCInitValue;
#ifdef CARRIER_DETECTION_SUPPORT
	pAd->chipCap.carrier_func = TONE_RADAR_V2;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v2;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef RTMP_INTERNAL_TX_ALC
	pChipOps->InitDesiredTSSITable = RT3352_AsicInitDesiredTSSITable;
#endif /* RTMP_INTERNAL_TX_ALC */
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
	UCHAR RFValue, RfValue1;
	ULONG value = 0;
	int i;


	// Set RF offset  RF_R17=RF_R23 (RT30xx)
	RFValue = pAd->RfFreqOffset & 0x7F;
	RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RfValue1);

	if (RFValue != (RfValue1 & 0x7f))
	{
		RFValue |= (RfValue1 & 0x80);
		RT30xxWriteRFRegister(pAd, RF_R17, RFValue);
	}

	// Initialize RF register to default value
	for (i = 0; i < RT3352_NUM_RF_REG_PARMS; i++)
	{
		RT30xxWriteRFRegister(pAd, RT3352_RFRegTable[i].Register, RT3352_RFRegTable[i].Value);
	}


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
					RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RFValue);
					if ((pAd->CommonCfg.BBPCurrentBW == BW_40))
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

#ifdef RTMP_INTERNAL_TX_ALC
VOID RT3352_InitDesiredTSSITable(
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
	desiredTSSIOverHT[MCS_7] = desiredTSSIOverHT[MCS_6] - 1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS8_MCS9, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS9_MCS9(0xE8) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_8] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_9] = desiredTSSIOverHT[MCS_8];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS10_MCS11 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS10_MCS11(0xE9) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_10] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_11] = desiredTSSIOverHT[MCS_10];
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS12_MCS13, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS12_MCS13(0xEA) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_12] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_13] = desiredTSSIOverHT[MCS_12];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS14_MCS15 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS14_MCS15(0xEB) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_14] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_15] = desiredTSSIOverHT[MCS_14] - 1;

	/* Boundary verification: the desired TSSI value */
	for (i = 0; i < 16; i++) /* HT: MCS 0 ~ MCS 15 */
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
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[8] = %d, desiredTSSIOverHT[9] = %d, desiredTSSIOverHT[10] = %d, desiredTSSIOverHT[11] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[8], 
		desiredTSSIOverHT[9], 
		desiredTSSIOverHT[10], 
		desiredTSSIOverHT[11]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[12] = %d, desiredTSSIOverHT[13] = %d, desiredTSSIOverHT[14] = %d, desiredTSSIOverHT[15] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[12], 
		desiredTSSIOverHT[13], 
		desiredTSSIOverHT[14], 
		desiredTSSIOverHT[15]));
	
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

	/* 
	  *		  		  | 	RF_R28[5:4] 	|  RF_R27[4] 	| RF_R27[5]
	  *         			  |  	 Adc5b_sel 	|  Rf_tssi_sel	| rf_tssi_en
	  *-----------------------+---------------------+---------------+-------------
	  * 	Internal 	TSSI0 |         00         	|        1        	|        1
	  * 	Internal 	TSSI1 |         10         	|        0        	|        1
 	  * 	External 	TSSI0 |         00       	|        0        	|        0
	  * 	External 	TSSI1 |         10       	|        1        	|        0 
	  *
	*/

	/* Internal TSSI0 */
	/* RFValue = (0x3 | 0x0<<2 | 0x1<<4 | 0x1 << 5); // tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=1, rf_tssi_en=1 */
	/* External TSSI0 */
	/* RFValue = (0x3 | 0x0<<2 | 0x0<<4 | 0x1 << 5); // tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=0, rf_tssi_en=1 */
	RFValue = (0x3 | 0x0<<2 | 0x3<<4);/* tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=0, rf_tssi_en=1 */
	/* Internal TSSI1 */
	/* RFValue = (0x3 | 0x0<<2 | 0x0<<4 | 0x1 << 5); // tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=0, rf_tssi_en=1 */
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

	/* Internal TSSI0 */
	/* RFValue = (0x3 | 0x0<<2 | 0x2 << 4); // tssi_gain1:x9, tssi_atten1:-17db, Adc5b_sel=10 (Internal TSSI1) */
	RFValue = (0x3 | 0x0<<2);/* tssi_gain1:x9, tssi_atten1:-17db, Adc5b_sel=10 (Internal TSSI1) */
	/* Internal TSSI1 */
	/* RFValue = (0x3 | 0x0<<2 | 0x2 << 4); // tssi_gain1:x9, tssi_atten1:-17db, Adc5b_sel=10 (Internal TSSI1) */
	RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
	BbpR49.field.adc5_in_sel = 1; /* PSI */
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
UCHAR RT3352_GetDesiredTSSI(
	IN PRTMP_ADAPTER 		pAd)
{
	PHTTRANSMIT_SETTING pLatestTxHTSetting = (PHTTRANSMIT_SETTING)(&pAd->LastTxRate);
	UCHAR desiredTSSI = 0;
	UCHAR MCS = 0;
#ifdef CONFIG_AP_SUPPORT
	UCHAR BBPValue = 0;
	UCHAR BBPRate = 0;
	UCHAR BBPMode = 0;
#endif /* CONFIG_AP_SUPPORT */

	MCS = (UCHAR)(pLatestTxHTSetting->field.MCS);
	
	if (pLatestTxHTSetting->field.MODE == MODE_CCK)
	{
		if (/* (MCS < 0) || */(MCS > 3)) /* boundary verification */
		{
			DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n", __FUNCTION__, MCS));
			MCS = 0;
		}
	
		desiredTSSI = desiredTSSIOverCCK[MCS];
	}
	else if (pLatestTxHTSetting->field.MODE == MODE_OFDM)
	{
		if (/* (MCS < 0) || */(MCS > 7)) /* boundary verification */
		{
			DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n", __FUNCTION__, MCS));
			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverOFDM[MCS];
	}
	else if ((pLatestTxHTSetting->field.MODE == MODE_HTMIX) || (pLatestTxHTSetting->field.MODE == MODE_HTGREENFIELD))
	{
		if (/* (MCS < 0) || */(MCS > 15)) /* boundary verification */
		{
			DBGPRINT_ERR(("%s: incorrect MCS: MCS = %d\n", __FUNCTION__, MCS));
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

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* For beacon power */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R2, &BBPValue);
		BBPMode = ((BBPValue >> 6) & (0x03));

		if (BBPMode == 0x00) /* CCK */
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R10, &BBPValue);
			BBPRate = (BBPValue & 0x03);	

			if (BBPRate == 0x00) /* MCS */
			{
				MCS = 0;		
				desiredTSSI = desiredTSSIOverCCK[MCS];
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	DBGPRINT(RT_DEBUG_INFO, ("%s: desiredTSSI = %d, Latest Tx HT setting: MODE = %d, MCS = %d, STBC = %d\n", 
		__FUNCTION__, 
		desiredTSSI, 
		pLatestTxHTSetting->field.MODE, 
		pLatestTxHTSetting->field.MCS, 
		pLatestTxHTSetting->field.STBC));

	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));

	return desiredTSSI;
}

VOID RT3352_AsicTxAlcGetAutoAgcOffset(
	IN PRTMP_ADAPTER 			pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1)
{
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable = pAd->chipCap.TxPowerTuningTable_2G;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL, pTxPowerTuningEntry2 = NULL;
	static UCHAR	LastChannel = 0;
	BBP_R49_STRUC 	BbpR49;
	UCHAR 			RFValue = 0;
	UCHAR 			RFValue2 = 0;
	UCHAR 			TssiChannel = 0;
	CHAR 			desiredTSSI = 0;
	CHAR 			currentTSSI = 0;
	CHAR			room = 0;
	CHAR 			TotalDeltaPower = 0; /* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */ 
	CHAR			TuningTableIndex = 0;
	CHAR			TuningTableIndex2 = 0;
	CHAR			AntennaDeltaPwr = 0;

#ifdef DOT11_N_SUPPORT				
	TssiChannel = pAd->CommonCfg.CentralChannel;
#else
	TssiChannel = pAd->CommonCfg.Channel;
#endif /* DOT11_N_SUPPORT */

	BbpR49.byte = 0;
	
	/* Locate the Internal Tx ALC tuning entry */
	if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
	{
		if ((pAd->Mlme.OneSecPeriodicRound % 4 == 0) && (*pDeltaPowerByBbpR1 == 0/* ??? */))
		{
			desiredTSSI = RT3352_GetDesiredTSSI(pAd);

			if (desiredTSSI == -1)
			{
				goto LabelFail;
			}

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
			currentTSSI = BbpR49.field.TSSI;

			if (pAd->TxPowerCtrl.bExtendedTssiMode == TRUE) /* Per-channel TSSI */
			{
				if ((TssiChannel >= 1) && (TssiChannel <= 14))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s: bExtendedTssiMode = %d, original desiredTSSI = %d, CentralChannel = %d, PerChTxPwrOffset = %d\n", 
							__FUNCTION__, 
							pAd->TxPowerCtrl.bExtendedTssiMode, 
							desiredTSSI, 
							TssiChannel, 
							pAd->TxPowerCtrl.PerChTxPwrOffset[TssiChannel]));

					desiredTSSI += pAd->TxPowerCtrl.PerChTxPwrOffset[TssiChannel];
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

			if (LastChannel != TssiChannel)
			{
				DBGPRINT(RT_DEBUG_OFF, ("******************************************\n"));
				DBGPRINT(RT_DEBUG_OFF, ("idxTxPowerTable reset to per-channel DAC!\n"));
				pAd->TxPowerCtrl.idxTxPowerTable = pAd->TxPower[TssiChannel-1].Power;
				DBGPRINT(RT_DEBUG_OFF, ("******************************************\n"));
			}

			if ((pAd->chipCap.TxPowerMaxCompenStep == 4) || (pAd->chipCap.TxPowerMaxCompenStep == 8))
			{
				pAd->chipCap.TxPowerTableMaxIdx = pAd->TxPower[TssiChannel-1].Power
									+ pAd->chipCap.TxPowerMaxCompenStep;

				DBGPRINT(RT_DEBUG_TRACE, ("--------------------------------------------\n"));
				DBGPRINT(RT_DEBUG_TRACE, ("TxPowerTableMaxIdx = %d\n", pAd->chipCap.TxPowerTableMaxIdx));
				DBGPRINT(RT_DEBUG_TRACE, ("TxPowerMaxCompenStep = %d\n", pAd->chipCap.TxPowerMaxCompenStep));

				room = (desiredTSSI >> 3);
					
				if (((desiredTSSI - room) > currentTSSI) && 
					(pAd->TxPowerCtrl.idxTxPowerTable
					< pAd->chipCap.TxPowerTableMaxIdx))
				{
					pAd->TxPowerCtrl.idxTxPowerTable++;
				}
			}
			else if (pAd->chipCap.TxPowerMaxCompenStep == 0)
			{
				/* TxPowerMaxCompenStep == 0 means TSSI range control is disabled. */
				room = (desiredTSSI >> 3);
					
				if ((desiredTSSI - room) > currentTSSI)
				{
					pAd->TxPowerCtrl.idxTxPowerTable++;
				}
			}
			else
			{
				DBGPRINT_ERR(("Wrong TxPowerMaxCompenStep value %u\n", pAd->chipCap.TxPowerMaxCompenStep));

				return;
			}

			if (desiredTSSI < currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable--;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("========================================\n"));
			DBGPRINT(RT_DEBUG_TRACE, ("Channel-Power = %d\n", pAd->TxPower[TssiChannel-1].Power));
			DBGPRINT(RT_DEBUG_TRACE, ("idxTxPowerTable = %d\n", pAd->TxPowerCtrl.idxTxPowerTable));

			TuningTableIndex = pAd->TxPowerCtrl.idxTxPowerTable;

			LastChannel = TssiChannel;

			if (TuningTableIndex < LOWERBOUND_TX_POWER_TUNING_ENTRY)
			{
				TuningTableIndex = LOWERBOUND_TX_POWER_TUNING_ENTRY;
			}

			if (TuningTableIndex >= UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd))
			{
				TuningTableIndex = UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd);
			}

			/* Valid pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 61 */
			pTxPowerTuningEntry = &TxPowerTuningTable[TuningTableIndex + TX_POWER_TUNING_ENTRY_OFFSET];
			pAd->TxPowerCtrl.RF_TX_ALC = pTxPowerTuningEntry->RF_TX_ALC;
			pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex = %d, pAd->TxPowerCtrl.RF_TX_ALC = %d, pAd->TxPowerCtrl.MAC_PowerDelta = %d\n", 
					TuningTableIndex, pAd->TxPowerCtrl.RF_TX_ALC, pAd->TxPowerCtrl.MAC_PowerDelta));

			/* Tx power adjustment over RF */
			RFValue = pAd->TxPowerCtrl.RF_TX_ALC;
			RT30xxWriteRFRegister(pAd, RF_R47, (UCHAR)RFValue); /* TX0_ALC */
			DBGPRINT(RT_DEBUG_TRACE, ("RF_R47 = 0x%02x ", RFValue));

			/* Delta Power between Tx0 and Tx1 */
			if ((pAd->TxPower[TssiChannel-1].Power) > (pAd->TxPower[TssiChannel-1].Power2))
			{
				AntennaDeltaPwr = ((pAd->TxPower[TssiChannel-1].Power)
									- (pAd->TxPower[TssiChannel-1].Power2));
				TuningTableIndex2 = TuningTableIndex - AntennaDeltaPwr;
			}
			else if ((pAd->TxPower[TssiChannel-1].Power) < (pAd->TxPower[TssiChannel-1].Power2))
			{
				AntennaDeltaPwr = ((pAd->TxPower[TssiChannel-1].Power2)
									- (pAd->TxPower[TssiChannel-1].Power));
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
			pAd->TxPowerCtrl.MAC_PowerDelta2 = pTxPowerTuningEntry2->MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("Channel DAC0 = 0x%02x\n", (pAd->TxPower[TssiChannel-1].Power)));			  
			DBGPRINT(RT_DEBUG_TRACE, ("Channel DAC1 = 0x%02x\n", (pAd->TxPower[TssiChannel-1].Power2)));			  
			DBGPRINT(RT_DEBUG_TRACE, ("AntennaDeltaPwr = 0x%02x\n", AntennaDeltaPwr));			  
			DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex = %d\n", TuningTableIndex));			  
			DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex2 = %d\n", TuningTableIndex2));			  
			DBGPRINT(RT_DEBUG_TRACE, ("RFValue = %u\n", RFValue));			  
			DBGPRINT(RT_DEBUG_TRACE, ("RFValue2 = %u\n", RFValue2));		

			RT30xxWriteRFRegister(pAd, RF_R48, RFValue2); /* TX1_ALC */
			DBGPRINT(RT_DEBUG_TRACE, ("RF_R48 = 0x%02x\n", RFValue2));

			/* Tx power adjustment over MAC */
			TotalDeltaPower = pAd->TxPowerCtrl.MAC_PowerDelta;
			pAd->TxPowerCtrl.TotalDeltaPower2 = pAd->TxPowerCtrl.MAC_PowerDelta2;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, currentTSSI = %d, TuningTableIndex = %d, {RF_TX_ALC = %d, MAC_PowerDelta = %d}\n", 
				__FUNCTION__, 
				desiredTSSI, 
				currentTSSI, 
				TuningTableIndex, 
				pTxPowerTuningEntry->RF_TX_ALC, 
				pTxPowerTuningEntry->MAC_PowerDelta));
		}
	}

LabelFail:

	*pTotalDeltaPwr = TotalDeltaPower;
}

/* The desired TSSI over CCK */
extern CHAR desiredTSSIOverCCK[4];

/* The desired TSSI over OFDM */
extern CHAR desiredTSSIOverOFDM[8];

/* The desired TSSI over HT */
extern CHAR desiredTSSIOverHT[16];

/* The desired TSSI over HT using STBC */
extern CHAR desiredTSSIOverHTUsingSTBC[8];


VOID RT3352_AsicInitDesiredTSSITable(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR TSSIBase = 0; /* The TSSI over OFDM 54Mbps */
	USHORT TSSIStepOver2dot4G = 0; /* The TSSI value/step (0.5 dB/unit) */
	UCHAR RFValue = 0;
	BBP_R49_STRUC BbpR49;
	ULONG i = 0;
	USHORT TxPower = 0, TxPowerOFDM54 = 0;
	USHORT Value = 0;
	CHAR BWPowerDelta = 0;
	BOOLEAN	bExtendedTssiMode = FALSE;
	EEPROM_TX_PWR_OFFSET_STRUC TxPwrOffset = {{0}};
	
	BbpR49.byte = 0;
	
	if (pAd->TxPowerCtrl.bInternalTxALC == FALSE)
	{
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, Value);
	
	if ((Value & 0xFF) == 0xFF) /* 20M/40M BW Power Delta for 2.4GHz band */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Don't considerate 20M/40M BW Delta Power since EEPROM is not calibrated.\n", __FUNCTION__));
	}
		else
		{
		if ((Value & 0xC0) == 0xC0)
		{
			BWPowerDelta += (Value & 0x3F); /* increase 40M BW Tx power with the delta value */
		}
		else if ((Value & 0xC0) == 0x80)
		{
			BWPowerDelta -= (Value & 0x3F); /* decrease 40M BW Tx power with the delta value */
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("%s: 20/40M BW Delta Power is not enabled, Value = 0x%X\n", __FUNCTION__, Value));
	}

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, Value);
	TSSIBase = (Value & 0x001F);

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TSSI_STEP_OVER_2DOT4G - 1), Value);

	if (((Value >> 8) & 0x80) == 0x80) /* Enable the extended TSSI mode */
	{
		bExtendedTssiMode = TRUE;
	}
	else
	{
		bExtendedTssiMode = FALSE;
	}

	if (bExtendedTssiMode == TRUE) /* Tx power offset for the extended TSSI mode */
	{
		pAd->TxPowerCtrl.bExtendedTssiMode = TRUE;

		/* Get the per-channel Tx power offset */
		
		RT28xx_EEPROM_READ16(pAd, (EEPROM_TX_POWER_OFFSET_OVER_CH_1 - 1), TxPwrOffset.word);
		pAd->TxPowerCtrl.PerChTxPwrOffset[1] = (TxPwrOffset.field.Byte1 & 0x0F); /* Tx power offset over channel 1 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[2] = (((TxPwrOffset.field.Byte1 & 0xF0) >> 4) & 0x0F); /* Tx power offset over channel 2 */

		RT28xx_EEPROM_READ16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_3, TxPwrOffset.word);
		pAd->TxPowerCtrl.PerChTxPwrOffset[3] = (TxPwrOffset.field.Byte0 & 0x0F); /* Tx power offset over channel 3 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[4] = (((TxPwrOffset.field.Byte0 & 0xF0) >> 4) & 0x0F); /* Tx power offset over channel 4 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[5] = (TxPwrOffset.field.Byte1 & 0x0F); /* Tx power offset over channel 5 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[6] = (((TxPwrOffset.field.Byte1 & 0xF0) >> 4) & 0x0F); /* Tx power offset over channel 6 */

		RT28xx_EEPROM_READ16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_7, TxPwrOffset.word);
		pAd->TxPowerCtrl.PerChTxPwrOffset[7] = (TxPwrOffset.field.Byte0 & 0x0F); /* Tx power offset over channel 7 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[8] = (((TxPwrOffset.field.Byte0 & 0xF0) >> 4) & 0x0F); /* Tx power offset over channel 8 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[9] = (TxPwrOffset.field.Byte1 & 0x0F); /* Tx power offset over channel 9 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[10] = (((TxPwrOffset.field.Byte1 & 0xF0) >> 4) & 0x0F); /* Tx power offset over channel 10 */

		RT28xx_EEPROM_READ16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_11, TxPwrOffset.word);
		pAd->TxPowerCtrl.PerChTxPwrOffset[11] = (TxPwrOffset.field.Byte0 & 0x0F); /* Tx power offset over channel 11 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[12] = (((TxPwrOffset.field.Byte0 & 0xF0) >> 4) & 0x0F); /* Tx power offset over channel 12 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[13] = (TxPwrOffset.field.Byte1 & 0x0F); /* Tx power offset over channel 13 */
		pAd->TxPowerCtrl.PerChTxPwrOffset[14] = (((TxPwrOffset.field.Byte1 & 0xF0) >> 4) & 0x0F); /* Tx power offset over channel 14 */

		/* 4-bit representation ==> 8-bit representation (2's complement) */
		
		for (i = 1; i <= NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET; i++)
		{
			if ((pAd->TxPowerCtrl.PerChTxPwrOffset[i] & 0x08) == 0x00) /* Positive number */
			{
				pAd->TxPowerCtrl.PerChTxPwrOffset[i] = (pAd->TxPowerCtrl.PerChTxPwrOffset[i] & ~0xF8);
			}
			else /* 0x08: Negative number */
			{
				pAd->TxPowerCtrl.PerChTxPwrOffset[i] = (pAd->TxPowerCtrl.PerChTxPwrOffset[i] | 0xF0);
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPwrOffset[1] = %d, TxPwrOffset[2] = %d, TxPwrOffset[3] = %d, TxPwrOffset[4] = %d\n", 
			__FUNCTION__, 
			pAd->TxPowerCtrl.PerChTxPwrOffset[1], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[2], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[3], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[4]));
		DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPwrOffset[5] = %d, TxPwrOffset[6] = %d, TxPwrOffset[7] = %d, TxPwrOffset[8] = %d\n", 
			__FUNCTION__, 
			pAd->TxPowerCtrl.PerChTxPwrOffset[5], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[6], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[7], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[8]));
		DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPwrOffset[9] = %d, TxPwrOffset[10] = %d, TxPwrOffset[11] = %d, TxPwrOffset[12] = %d\n", 
			__FUNCTION__, 
			pAd->TxPowerCtrl.PerChTxPwrOffset[9], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[10], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[11], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[12]));
		DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPwrOffset[13] = %d, TxPwrOffset[14] = %d\n", 
			__FUNCTION__, 
			pAd->TxPowerCtrl.PerChTxPwrOffset[13], 
			pAd->TxPowerCtrl.PerChTxPwrOffset[14]));
			}
			else 
			{
		pAd->TxPowerCtrl.bExtendedTssiMode = FALSE;
		RTMPZeroMemory(pAd->TxPowerCtrl.PerChTxPwrOffset, sizeof (pAd->TxPowerCtrl.PerChTxPwrOffset));
			}

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
	desiredTSSIOverHT[MCS_7] = desiredTSSIOverHT[MCS_6] - 1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS8_MCS9, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS9_MCS9(0xE8) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_8] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_9] = desiredTSSIOverHT[MCS_8];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS10_MCS11 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS10_MCS11(0xE9) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_10] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_11] = desiredTSSIOverHT[MCS_10];
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS12_MCS13, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS12_MCS13(0xEA) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_12] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_13] = desiredTSSIOverHT[MCS_12];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS14_MCS15 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS14_MCS15(0xEB) = 0x%X\n", TxPower));
	desiredTSSIOverHT[MCS_14] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_15] = desiredTSSIOverHT[MCS_14] - 1;

	/* Boundary verification: the desired TSSI value */
	for (i = 0; i < 16; i++) /* HT: MCS 0 ~ MCS 15 */
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
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[8] = %d, desiredTSSIOverHT[9] = %d, desiredTSSIOverHT[10] = %d, desiredTSSIOverHT[11] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[8], 
		desiredTSSIOverHT[9], 
		desiredTSSIOverHT[10], 
		desiredTSSIOverHT[11]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[12] = %d, desiredTSSIOverHT[13] = %d, desiredTSSIOverHT[14] = %d, desiredTSSIOverHT[15] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[12], 
		desiredTSSIOverHT[13], 
		desiredTSSIOverHT[14], 
		desiredTSSIOverHT[15]));
	
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

	/* 
						|   RF_R28[5:4]  	|  	 RF_R27[4]	 	| RF_R27[5]
						|    Adc5b_sel  	|  	 Rf_tssi_sel	| rf_tssi_en
	--------------------+-------------------+-------------------+--------------
	Internal 	TSSI0 	|         00       	|        1        	|        1
	Internal 	TSSI1 	|         10       	|        0        	|        1
	External 	TSSI0 	|         00       	|        0        	|        0
	External 	TSSI1 	|         10       	|        1        	|        0 
	  
	*/

	/* Internal TSSI0 */
	/* RFValue = (0x3 | 0x0<<2 | 0x1<<4 | 0x1 << 5); // tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=1, rf_tssi_en=1 */
	/* External TSSI0 */
	/* RFValue = (0x3 | 0x0<<2 | 0x0<<4 | 0x1 << 5); // tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=0, rf_tssi_en=1 */
	RFValue = (0x3 | 0x0<<2 | 0x3<<4);/* tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=0, rf_tssi_en=1 */
	/* Internal TSSI1 */
	/* RFValue = (0x3 | 0x0<<2 | 0x0<<4 | 0x1 << 5); // tssi_gain0:x9, tssi_atten0:-17db, rf_tssi_sel=0, rf_tssi_en=1 */
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

	/* Internal TSSI0 */
	/* RFValue = (0x3 | 0x0<<2 | 0x2 << 4); // tssi_gain1:x9, tssi_atten1:-17db, Adc5b_sel=10 (Internal TSSI1) */
	RFValue = (0x3 | 0x0<<2);/* tssi_gain1:x9, tssi_atten1:-17db, Adc5b_sel=10 (Internal TSSI1) */
	/* Internal TSSI1 */
	/* RFValue = (0x3 | 0x0<<2 | 0x2 << 4); // tssi_gain1:x9, tssi_atten1:-17db, Adc5b_sel=10 (Internal TSSI1) */
	RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
	BbpR49.field.adc5_in_sel = 1; /* PSI */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpR49.byte);	

	DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
}
#endif /* RTMP_INTERNAL_TX_ALC */
#endif /* RT3352 */

/* End of rt3352.c */
