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
	rt5350.c

	Abstract:
	Specific funcitons and variables for RT5350

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT5350

#include	"rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //

UCHAR	RT5350_EeBuffer[EEPROM_SIZE] = {
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

REG_PAIR   RT5350_RFRegTable[] = {
	{RF_R00, 0xF0},
	{RF_R01, 0x23},
	{RF_R02, 0x50},
	{RF_R03, 0x08},
	{RF_R04, 0x49},
	{RF_R05, 0x10},
	{RF_R06, 0xE0},
	{RF_R07, 0x00},
	{RF_R08, 0xF1},
	{RF_R09, 0x02},
	{RF_R10, 0x53},
	{RF_R11, 0x4A},
	{RF_R12, 0x46},
	{RF_R13, 0x9F},
	{RF_R14, 0x00},
	{RF_R15, 0x00},
	{RF_R16, 0xC0}, /* for 40M spur reduction */
/*	{RF_R17, 0x1A}, */
	{RF_R18, 0x03},
	{RF_R19, 0x00},
	{RF_R20, 0x00},
	{RF_R21, 0x00},
	{RF_R22, 0x20},
	{RF_R23, 0x00},
	{RF_R24, 0x00},
	{RF_R25, 0x80},
	{RF_R26, 0x00},
	{RF_R27, 0x03},
	{RF_R28, 0x00},
	{RF_R29, 0xD0}, /* optimize for H/W diversity, do not use 0x10 */
	{RF_R30, 0x10},
	{RF_R31, 0x80},
	{RF_R32, 0x80},
	{RF_R33, 0x00},
	{RF_R34, 0x07},
	{RF_R35, 0x12},
	{RF_R36, 0x00},
	{RF_R37, 0x08},
	{RF_R38, 0x85},
	{RF_R39, 0x1B},
	{RF_R40, 0x0B},
	{RF_R41, 0xBB},
	{RF_R42, 0xD5},
	{RF_R43, 0x9B},
	{RF_R44, 0x0C},
	{RF_R45, 0xA6},
	{RF_R46, 0x73},
	{RF_R47, 0x00},
	{RF_R48, 0x10},
	{RF_R49, 0x80},
	{RF_R50, 0x00},
	{RF_R51, 0x00},
	{RF_R52, 0x38},
	{RF_R53, 0x00},
	{RF_R54, 0x38},
	{RF_R55, 0x43},
	{RF_R56, 0x82},
	{RF_R57, 0x00},
	{RF_R58, 0x39},
	{RF_R59, 0x0B},
	{RF_R60, 0x45},
	{RF_R61, 0xD1},
	{RF_R62, 0x00},
	{RF_R63, 0x00},
};

UCHAR RT5350_RF59[] = { /* based on different 2.4G channels */
	0x0B,
	0x0B,
	0x0B,
	0x0B,
	0x0B,
	0x0B,
	0x0B,
	0x0A,
	0x0A,
	0x09,
	0x08,
	0x07,
	0x07,
	0x06
};

UCHAR RT5350_NUM_RF_REG_PARMS = (sizeof(RT5350_RFRegTable) / sizeof(REG_PAIR));


REG_PAIR   RT5350_BBPRegTable[] = {
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
	{BBP_R73,		0x13},
	{BBP_R75,		0x46},
	{BBP_R76,		0x28},
	{BBP_R77,		0x59},
	{BBP_R78,		0x0E},
	{BBP_R80,		0x08}, // requested by Gary for high power
	{BBP_R81,		0x37},
	{BBP_R82,		0x62},
	{BBP_R83,		0x7A},
	{BBP_R84,		0x9a},
	{BBP_R86,		0x38},	// middle range issue, Rory @2008-01-28 	
	{BBP_R91,		0x04},	// middle range issue, Rory @2008-01-28
	{BBP_R92,		0x02},  // middle range issue, Rory @2008-01-28

	{BBP_R103,		0xC0},
	{BBP_R104,		0x92},
	{BBP_R105,		0x3C},
	{BBP_R106,		0x03},
	{BBP_R128,		0x12},
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

UCHAR RT5350_NUM_BBP_REG_PARMS = (sizeof(RT5350_BBPRegTable) / sizeof(REG_PAIR));


RTMP_REG_PAIR	RT5350_MACRegTable[] =	{
	{TX_SW_CFG0,		0x404},   // 2010-07-20
};

UCHAR RT5350_NUM_MAC_REG_PARMS = (sizeof(RT5350_MACRegTable) / sizeof(RTMP_REG_PAIR));


#ifdef RTMP_INTERNAL_TX_ALC
TX_POWER_TUNING_ENTRY_STRUCT RT5350_TxPowerTuningTable[] =
{
// idxTxPowerTable         Tx power control over RF           Tx power control over MAC
//(zero-based array)         { RF_R49[5:0]: Tx0 ALC},         {MAC 0x1314~0x1320}
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
/*  62  */                           {0x20,                                      0},
/*  63  */                           {0x21,                                      0},
/*  64  */                           {0x22,                                      0},
/*  65  */                           {0x23,                                      0},
/*  66  */                           {0x24,                                      0},
/*  67  */                           {0x25,                                      0},
/*  68  */                           {0x26,                                      0},
/*  69  */                           {0x27,                                      0},
/*  70  */                           {0x26,                                      1},
/*  71  */                           {0x27,                                      1},
/*  72  */                           {0x26,                                      2},
/*  73  */                           {0x27,                                      2},
/*  74  */                           {0x26,                                      3},
/*  75  */                           {0x27,                                      3},
/*  76  */                           {0x26,                                      4},
/*  77  */                           {0x27,                                      4},
/*  78  */                           {0x26,                                      5},
/*  79  */                           {0x27,                                      5},
/*  80  */                           {0x26,                                      6},
/*  81  */                           {0x27,                                      6},
/*  82  */                           {0x26,                                      7},
/*  83  */                           {0x27,                                      7},
/*  84  */                           {0x26,                                      8},
/*  85  */                           {0x27,                                      8},
/*  86  */                           {0x26,                                      9},
/*  87  */                           {0x27,                                      9},
/*  88  */                           {0x26,                                      10},
/*  89  */                           {0x27,                                      10},
/*  90  */                           {0x26,                                      11},
/*  91  */                           {0x27,                                      11},
};

static TssiDeltaInfo TSSI_Set[15];
static TssiDeltaInfo TSSI_x;
static UCHAR TSSI_ref;

UINT32 RT5350_desiredTSSIOverCCK[4] = {0};

// The desired TSSI over OFDM
UINT32 RT5350_desiredTSSIOverOFDM[8] = {0};

// The desired TSSI over HT
UINT32 RT5350_desiredTSSIOverHT[16] = {0};
UINT32 RT5350_desiredTSSIOverHT40[16] = {0};

UINT32 RT5350_GetDesiredTSSI(
	IN PRTMP_ADAPTER		pAd,
	OUT PUCHAR				pBbpR49);

UINT32 TSSIRatioDot85(
	IN INT32 delta_power);

UINT32 TSSIRatioDot7(
	IN INT32 delta_power);

VOID RT5350_ChipAGCInit(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth);
#endif // RTMP_INTERNAL_TX_ALC //




/*
========================================================================
Routine Description:
	Initialize RT5350.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT5350_Init(
	IN PRTMP_ADAPTER		pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	/* ??? */
	pAd->RfIcType = RFIC_3320;

	/* init capability */
	pChipCap->MaxNumOfRfId = 63;
	pChipCap->MaxNumOfBbpId = 255;
	pChipCap->pRFRegTable = RT5350_RFRegTable;
	pChipCap->pBBPRegTable = RT5350_BBPRegTable;
	pChipCap->bbpRegTbSize = RT5350_NUM_BBP_REG_PARMS;
	pChipCap->SnrFormula = SNR_FORMULA2;
	pChipCap->RfReg17WtMethod = RF_REG_WT_METHOD_STEP_ON;

	pChipCap->FlgIsHwWapiSup = TRUE;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_2;	

	if ((pAd->MACVersion & 0xffff) > 0x0102)
		pChipCap->FlgIsHwAntennaDiversitySup = TRUE;
	else
		pChipCap->FlgIsHwAntennaDiversitySup = FALSE;

#ifdef NEW_MBSSID_MODE
	pChipCap->MBSSIDMode = MBSSID_MODE1;
#else
	pChipCap->MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */


	/* init operator */
	pChipOps->AsicRfInit = NICInitRT5350RFRegisters;
	pChipOps->AsicBbpInit = NICInitRT5350BbpRegisters;
	pChipOps->AsicMacInit = NICInitRT5350MacRegisters;
	pChipOps->RxSensitivityTuning = RT5350_RxSensitivityTuning;
#ifdef CONFIG_STA_SUPPORT
	pChipOps->ChipAGCAdjust = RT5350_ChipAGCAdjust;
#endif // CONFIG_STA_SUPPORT //
	pChipOps->ChipBBPAdjust = RT5350_ChipBBPAdjust;
	pChipOps->ChipSwitchChannel = RT5350_ChipSwitchChannel;
	pChipOps->AsicAdjustTxPower = AsicAdjustTxPower;
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 16;
#ifdef RTMP_FLASH_SUPPORT
	pChipCap->eebuf = RT5350_EeBuffer;
#endif /* RTMP_FLASH_SUPPORT */

	pChipOps->AsicGetTxPowerOffset = AsicGetTxPowerOffset;

#ifdef RTMP_INTERNAL_TX_ALC
	pChipCap->TxAlcTxPowerUpperBound_2G = 61;
	pChipCap->TxPowerMaxCompenStep = 4; /* default 2dB (one step is 0.5dB) */
	pChipCap->TxPowerTableMaxIdx = 0; 
	pChipCap->TxPowerTuningTable_2G = RT5350_TxPowerTuningTable;
	pChipOps->InitDesiredTSSITable = RT5350_InitDesiredTSSITable;
	pChipOps->AsicTxAlcGetAutoAgcOffset = RT5350_AsicTxAlcGetAutoAgcOffset;
#endif /* RTMP_INTERNAL_TX_ALC */

	/* 
		5350 have other MAC registers to extra compensate
		Tx power for OFDM 54, HT MCS 7 and STBC MCS 7
	*/
	pChipOps->AsicExtraPowerOverMAC = RT5350_AsicExtraPowerOverMAC;


	pChipOps->SetRxAnt = RT5350SetRxAnt;
	pAd->Mlme.bEnableAutoAntennaCheck = FALSE; /* 1T1R only */


#ifdef CARRIER_DETECTION_SUPPORT
	pAd->chipCap.carrier_func = TONE_RADAR_V2;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v2;
#endif /* CARRIER_DETECTION_SUPPORT */

	pChipOps->ChipAGCInit = RT5350_ChipAGCInit;

	RtmpChipBcnSpecInit(pAd);
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
VOID NICInitRT5350MacRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT32 IdReg;


	for(IdReg=0; IdReg<RT5350_NUM_MAC_REG_PARMS; IdReg++)
	{
		RTMP_IO_WRITE32(pAd, RT5350_MACRegTable[IdReg].Register,
								RT5350_MACRegTable[IdReg].Value);
	}
}


/*
========================================================================
Routine Description:
	Initialize specific BBP registers.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT5350BbpRegisters(
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
	Initialize specific RF registers.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT5350RFRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT8 RfReg = 0;
	UCHAR RFValue, RfValue1;
//	ULONG value = 0;
	int i;

	// Driver should toggle RF R02 bit7 before init RF registers
	//RF_R02: Resistor calibration, RF_R02 = RF_R30 (RT30xx)
	RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
	RfReg &= ~(1 << 6); // clear bit6=rescal_bp
	RfReg |= 0x80; // bit7=rescal_en
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
	RTMPusecDelay(1000);

	// Set RF offset  RF_R17=RF_R23 (RT30xx)
	RFValue = pAd->RfFreqOffset & 0x7F; // bit7 = 0
	RT30xxReadRFRegister(pAd, RF_R17, &RfValue1);
	if (RFValue != (RfValue1 & 0x7f))
	{
		RFValue |= (RfValue1 & 0x80);
		RT30xxWriteRFRegister(pAd, RF_R17, RFValue);
	}

	// Initialize RF register to default value
	for (i = 0; i < RT5350_NUM_RF_REG_PARMS; i++)
	{
		RT30xxWriteRFRegister(pAd, RT5350_RFRegTable[i].Register, RT5350_RFRegTable[i].Value);
	}


	//Gary: Boot up bit7=1
	RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
	RfReg &= ~(1 << 6); // clear bit6=rescal_bp
	RfReg |= 0x80; // bit7=rescal_en
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
	RTMPusecDelay(1000);
	RfReg &= 0x7F;
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);

	/* Gary: 20100827 for 11B-only mode */
	if (pAd->CommonCfg.PhyMode == PHY_11B)
		RT30xxWriteRFRegister(pAd, RF_R55, (UCHAR)0x47);
}


VOID RT5350SetRxAnt(
    IN PRTMP_ADAPTER    pAd,
    IN UCHAR            Ant)
{
	UINT8 BBPValue = 0;

	/*
		main antenna: BBP_R152 bit7=1
		aux antenna: BBP_R152 bit7=0
	*/
	BBPValue = 0x3e;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
	BBPValue = 0x30;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
	BBPValue = 0x00;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);

	if (Ant == 0)
	{
		/* fix to main antenna */
		/* do not care BBP R153, R155, R253 */
		BBPValue = 0x23;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);

		DBGPRINT(RT_DEBUG_TRACE, ("%s: switch to main antenna\n", __FUNCTION__));
	}
	else
	{
		/* fix to aux antenna */
		/* do not care BBP R153, R155, R253 */
		BBPValue = 0xa3;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);

		DBGPRINT(RT_DEBUG_TRACE, ("%s: switch to aux antenna\n", __FUNCTION__));
	}
}


VOID RT5350_RxSensitivityTuning(
	IN PRTMP_ADAPTER			pAd)
{
	UCHAR R66;


	R66 = 0x26 + GET_LNA_GAIN(pAd);
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
#ifdef RTMP_RBUS_SUPPORT
		// TODO: we need to add MACVersion Check here!!!!
#if defined(RT5350)
		if (IS_RT5350(pAd))
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x0);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
		}
		else
#endif // defined(RT5350) //
#endif // RTMP_RBUS_SUPPORT //
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
	}
	else
#endif // RALINK_ATE //
	{
		AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL);
	}
	DBGPRINT(RT_DEBUG_TRACE,("turn off R17 tuning, restore to 0x%02x\n", R66));
}


#ifdef CONFIG_STA_SUPPORT
UCHAR RT5350_ChipAGCAdjust(
	IN PRTMP_ADAPTER		pAd,
	IN CHAR					Rssi,
	IN UCHAR				OrigR66Value)
{
	UCHAR R66 = OrigR66Value;
	CHAR lanGain = GET_LNA_GAIN(pAd);

	if (pAd->LatchRfRegs.Channel <= 14)
	{	//BG band
		if (pAd->CommonCfg.BBPCurrentBW == BW_20)
			R66 = lanGain * 2 + 0x1C;
		else
			R66 = lanGain * 2 + 0x24;
	}
	else
	{	//A band
		if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		{
			R66 = 0x32 + (lanGain * 5) / 3;
			if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				R66 += 0x10;
		}
		else
		{
			R66 = 0x3A + (lanGain * 5) / 3;
			if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				R66 += 0x10;	
		}
	}

	if (OrigR66Value != R66)
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	
	return R66;
}
#endif // CONFIG_STA_SUPPORT //


VOID RT5350_ChipBBPAdjust(
	IN RTMP_ADAPTER			*pAd)
{
	UINT32 Value;
	UCHAR byteValue = 0;


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
#if defined(RT5350)
			UCHAR R66;
			if (IS_RT5350(pAd))
			{
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = GET_LNA_GAIN(pAd)*2 + 0x1C;
				else
					R66 = GET_LNA_GAIN(pAd)*2 + 0x24;
				AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
			}
#endif // RT5350 //
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
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x13);
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
#if defined(RT5350)
			if (IS_RT5350(pAd))
			{
				UCHAR R66;
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = GET_LNA_GAIN(pAd)*2 + 0x1C;
				else
					R66 = GET_LNA_GAIN(pAd)*2 + 0x24;
				AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
			}
#endif // RT5350 //
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
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x13);
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
			//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x30);
			// request by Brian 20070306
#if defined(RT5350)
			if (IS_RT5350(pAd))
			{
				UCHAR R66;
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = GET_LNA_GAIN(pAd)*2 + 0x1C;
				else
					R66 = GET_LNA_GAIN(pAd)*2 + 0x24;
				AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
			}
#endif // RT5350 //
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
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x13);
		}

#ifdef DOT11_N_SUPPORT
		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : 20MHz, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
										pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth, 
										pAd->CommonCfg.Channel, 
										pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
										pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif // DOT11_N_SUPPORT //
	}
}


VOID RT5350_ChipSwitchChannel(
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
#ifdef RT5350
		|| IS_RT5350(pAd)
#endif // RT5350 //
		))
	{
		/* modify by WY for Read RF Reg. error */
		
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
#ifdef RT5350
				Value = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

				// Programming channel parameters
				if(Value & (1<<20)) { //Xtal=40M
					RT30xxWriteRFRegister(pAd, RF_R08, FreqItems3020[index].N);
					RT30xxWriteRFRegister(pAd, RF_R09, FreqItems3020[index].K);
					RFValue = 0x9F;
				}else {
					RT30xxWriteRFRegister(pAd, RF_R08, FreqItems3020_Xtal20M[index].N);
					RT30xxWriteRFRegister(pAd, RF_R09, FreqItems3020_Xtal20M[index].K);
					RFValue = 0x1F;
				}		
				RT30xxWriteRFRegister(pAd, RF_R13, (UCHAR)RFValue);
				RFValue = 0x4A;
				RT30xxWriteRFRegister(pAd, RF_R11, (UCHAR)RFValue);

				for (i = 0; i < MAX_NUM_OF_CHANNELS; i++) {
					if (Channel != pAd->TxPower[i].Channel)
						continue;
	
					/* Bit<7:6>=1:0, TX0 DAC By EEPROM Channel setting */
					RFValue = pAd->TxPower[i].Power & 0x3F; // clesr bit7:6
					RFValue |= 0x80; // bit7:6 = 1:0
				
					RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)RFValue);
					break;
				}

				RFValue = 0x46;
				RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)RFValue);

				RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RFValue);
				if (pAd->CommonCfg.BBPCurrentBW == BW_40)
					RFValue |= 0x06; // 40MBW tx_h20M=1,rx_h20M=1
				else
					RFValue &= ~(0x06); // 20MBW tx_h20M=0,rx_h20M=0
				RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RFValue);

				if(pAd->CommonCfg.PhyMode == PHY_11B)
					RFValue = 0xC0;
				else
					RFValue = 0x80;
				RT30xxWriteRFRegister(pAd, RF_R32, (UCHAR)RFValue);

				RFValue = 0x00;
				RT30xxWriteRFRegister(pAd, RF_R53, (UCHAR)RFValue);
	
				RFValue = 0x43;
				RT30xxWriteRFRegister(pAd, RF_R55, (UCHAR)RFValue);
		
				RFValue = 0x82;
				RT30xxWriteRFRegister(pAd, RF_R56, (UCHAR)RFValue);

				RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x80; // bit 7=vcocal_en
				RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);

				RTMPusecDelay(2000);

				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, RFValue & 0xfe); // clear update flag
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);

				// latch channel for future usage.
				pAd->LatchRfRegs.Channel = Channel;
#endif // RT5350 //

#ifdef RT5350
				if (IS_RT5350(pAd))
					RFValue = 0; /* RF_R24 is reserved bits */
#endif // RT5350 //


				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT5350
					if (IS_RT5350(pAd))
						RFValue = 0; /* RF_R24 is reserved bits */
#endif // RT5350 //
				}
				RT30xxWriteRFRegister(pAd, RF_R24, RFValue);

				// Rx filter
				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT5350
					if (IS_RT5350(pAd))
						RT30xxWriteRFRegister(pAd, RF_R31, 0x80); //FIXME: I don't know the RF_R31 for BW40 case
#endif // RT5350 //
				}
				else
				{
#ifdef RT5350
					if (IS_RT5350(pAd))
						RT30xxWriteRFRegister(pAd, RF_R31, 0x80);
#endif // RT5350 //
				}

#if defined (RT5350)
				if (IS_RT5350(pAd))
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
#endif // RT5350 //

				// latch channel for future usage.
				pAd->LatchRfRegs.Channel = Channel;
				
				break;				
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
			Channel, 
			pAd->RfIcType, 
			TxPwer,
			TxPwer2,
			pAd->Antenna.field.TxPath,
			FreqItems3020[index].N, 
			FreqItems3020[index].K, 
			FreqItems3020[index].R));
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
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38); // Gary 2010-07-21

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

#if defined (RT5350)
		if (IS_RT5350(pAd))
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x7a);
		}
#endif // RT5350 //

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

		/* different RF R59 for different central channel 20100827 */
		RT30xxWriteRFRegister(pAd, RF_R59, (UCHAR)RT5350_RF59[Channel-1]);

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		ULONG	TxPinCfg = 0x00050F05;//Gary 2007/8/9 0x050505
		UINT8	bbpValue;
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38); // Gary 2010-07-21

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
	/*AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x2E + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL); */
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


VOID RT5350_AsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, /* ExtraPwrOverTxPwrCfg8 = 0, */ExtraPwrOverTxPwrCfg9 = 0;

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

	DBGPRINT(RT_DEBUG_TRACE, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}


#ifdef RTMP_INTERNAL_TX_ALC
VOID RT5350_AsicTxAlcGetAutoAgcOffset(
	IN PRTMP_ADAPTER 			pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1)
{
	CHAR TotalDeltaPower = 0; 
	BBP_R49_STRUC BbpR49;
	UINT32 desiredTSSI = 0, currentTSSI = 0, room_up = 0, room_down = 0;
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable = pAd->chipCap.TxPowerTuningTable_2G;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	UCHAR RFValue = 0;
	CHAR DeltaPwr = 0, TuningTableIndex = 0;
	BOOLEAN bKeepRF = FALSE;

	BbpR49.byte = 0;

	/* TX power compensation for temperature variation based on TSSI. try every 4 second */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
		desiredTSSI = RT5350_GetDesiredTSSI(pAd, &BbpR49.byte);
		room_down = (desiredTSSI >> 5);	  
		room_up = (desiredTSSI >> 2);	  
		currentTSSI = BbpR49.byte * 10000;
	  
		DBGPRINT(RT_DEBUG_TRACE, ("DesiredTSSI = %d, CurrentTSSI = %d (Range: %d ~ %d, BBP_R49=0x%X)\n",  
		desiredTSSI, currentTSSI, desiredTSSI-room_up,  
		desiredTSSI+room_down, BbpR49.byte)); 

		if (desiredTSSI-room_up > currentTSSI)
		{
			pAd->TxPowerCtrl.idxTxPowerTable++;
		} 
		else if (desiredTSSI+room_down < currentTSSI)
		{
			pAd->TxPowerCtrl.idxTxPowerTable--;
		}
		else
		{
			bKeepRF = TRUE;
		}

		/* To reduce the time for TSSI compensated to target value */
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

		if (TuningTableIndex >= UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd))
		{
			TuningTableIndex = UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd);
		}

		/* Valid TuningTableIndex: -30 ~ 61 */
		pTxPowerTuningEntry = &TxPowerTuningTable[TuningTableIndex + TX_POWER_TUNING_ENTRY_OFFSET];
		pAd->TxPowerCtrl.RF_TX_ALC = pTxPowerTuningEntry->RF_TX_ALC;
		pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

		if (bKeepRF == FALSE)
		{
			/* Tx power adjustment over RF is needed */
			RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)(&RFValue)); /* TX0_ALC */
			RFValue &= ~0x3F; /* clear RF_R49[5:0] */
			RFValue |= pAd->TxPowerCtrl.RF_TX_ALC;

			/* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
			if ((RFValue & 0x3F) > 0x27) 
			{
				RFValue = ((RFValue & ~0x3F) | 0x27);
			}
			RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)RFValue); /* TX0_ALC */
			DBGPRINT(RT_DEBUG_TRACE, ("RF_R49 = 0x%X ", RFValue));
		}

		/* Tx power adjustment over MAC */
		TotalDeltaPower = pAd->TxPowerCtrl.MAC_PowerDelta;

		DBGPRINT(RT_DEBUG_TRACE, ("Current index of TuningTable = %d {RF_TX_ALC = 0x%X, MAC_PowerDelta = %d}\n",
			(TuningTableIndex + TX_POWER_TUNING_ENTRY_OFFSET),
			pAd->TxPowerCtrl.RF_TX_ALC,
			pAd->TxPowerCtrl.MAC_PowerDelta));
	}

	*pDeltaPwr = DeltaPwr;
	*pTotalDeltaPwr = TotalDeltaPower;
}


INT32 TSSIDelta2PowDelta(UINT32 TSSI_x_10000, UINT32 TSSI_ref)
{
	UINT32 ratio_x; 
	INT32 power_delta;

	ratio_x = TSSI_x_10000 / TSSI_ref;

	if (ratio_x <= 1903) {
		DBGPRINT(RT_DEBUG_TRACE, ("TSSIRatio2Delta: ratio_x(%d) is ourt range(7 ~ -8)\n", ratio_x)); 
		power_delta = -8;
	}
	else if (ratio_x <= 2315)
		power_delta = -8;
	else if (ratio_x <= 2815)
		power_delta = -7;
	else if (ratio_x <= 3424)
		power_delta = -6;
	else if (ratio_x <= 4164)
		power_delta = -5;
	else if (ratio_x <= 5065)
		power_delta = -4;
	else if (ratio_x <= 6160)
		power_delta = -3;
	else if (ratio_x <= 7491)
		power_delta = -2;
	else if (ratio_x <= 9111)
		power_delta = -1;
	else if (ratio_x <= 11081)
		power_delta = 0;
	else if (ratio_x <= 13476)
		power_delta = 1;
	else if (ratio_x <= 16390)
		power_delta = 2;
	else if (ratio_x <= 19933)
		power_delta = 3;
	else if (ratio_x <= 24242)
		power_delta = 4;
	else if (ratio_x <= 29483)
		power_delta = 5;
	else if (ratio_x <= 35857)
		power_delta = 6;
	else if (ratio_x <= 43609)
		power_delta = 7;
	else {
		DBGPRINT(RT_DEBUG_TRACE, ("TSSIRatio2Delta: ratio_x(%d) is ourt range(7 ~ -8)\n", ratio_x)); 
		power_delta = 7;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("TSSIRatio2Delta: TSSI_x_10000=%u, TSSI_ref=%u ratio_x=%u power_delta=%d\n", TSSI_x_10000, TSSI_ref, ratio_x, power_delta)); 

	return power_delta;
}


UINT32 TSSIRatioDot85(INT32 delta_power)
{
	UINT32 ratio = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));
      
	switch(delta_power)
	{
		case -12:
			ratio = 955;
			break;
		case -11:
			ratio = 1161;
			break;
		case -10:
			ratio = 1413;
			break;
		case -9:
			ratio = 1718;
			break;
		case -8:
			ratio = 2089;
			break;
		case -7:
			ratio = 2541;
			break;
		case -6:
			ratio = 3090;
			break;
		case -5:
			ratio = 3758;
			break;
		case -4:
			ratio = 4571;
			break;
		case -3:
			ratio = 5559;
			break;
		case -2:
			ratio = 6761;
			break;
		case -1:
			ratio = 8222;
			break;
		case 0:
			ratio = 10000;
			break;
		case 1:
			ratio = 12162;
			break;
		case 2:
			ratio = 14791;
			break;
		case 3:
			ratio = 17989;
			break;
		case 4:
			ratio = 21878;
			break;
		case 5:
			ratio = 26607;
			break;
		case 6:
			ratio = 32359;
			break;
		case 7:
			ratio = 39355;
			break;
		case 8:
			ratio = 47863;
			break;
		case 9:
			ratio = 58210;
			break;
		case 10:
			ratio = 70795;
			break;
		case 11:
			ratio = 86099;
			break;
		case 12:
			ratio = 104713;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("Invalid delta_power %d\n", delta_power));
			break;

	}

	return ratio;
}


UINT32 TSSIRatioDot7(INT32 delta_power)
{
	UINT32 ratio = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

	switch(delta_power)
	{
		case -12:
			ratio = 1445;
			break;
		case -11:
			ratio = 1698;
			break;
		case -10:
			ratio = 1995;
			break;
		case -9:
			ratio = 2344;
			break;
        case -8:
			ratio = 2754;
			break;
        case -7:
			ratio = 3236;
			break;
        case -6:
			ratio = 3802;
			break;
        case -5:
			ratio = 4467;
			break;
        case -4:
			ratio = 5248;
			break;
        case -3:
			ratio = 6166;
			break;
        case -2:
			ratio = 7244;
			break;
		case -1:
			ratio = 8511;
			break;
		case 0:
			ratio = 10000;
			break;
		case 1:
			ratio = 11749;
			break;
		case 2:
			ratio = 13804;
			break;
		case 3:
			ratio = 16218;
			break;
		case 4:
			ratio = 19055;
			break;
		case 5:
			ratio = 22387;
			break;
		case 6:
			ratio = 26303;
			break;
		case 7:
			ratio = 30903;
			break;
		case 8:
			ratio = 36308;
			break;
		case 9:
			ratio = 42658;
			break;
		case 10:
			ratio = 50119;
			break;
		case 11:
			ratio = 58884;
			break;
		case 12:
			ratio = 69183;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("Invalid delta_power %d\n", delta_power));
			break;

	}

	return ratio;
}


VOID RT5350_InitDesiredTSSITable(
	IN PRTMP_ADAPTER			pAd)
{
    USHORT TSSIBase = 0;/* The TSSI over OFDM 54Mbps */
    USHORT TSSIDelta = 0;/* The TSSI value/step (0.5 dB/unit) */
    UCHAR  RFValue = 0;
    UCHAR  BbpR47 = 0;
    UINT32 i = 0;
    USHORT TxPower = 0, TxPowerOFDM54 = 0;
    USHORT  TSSIParm=0;
    UCHAR  TSSIGain=0;
    UCHAR  TSSIAttenuation=0;
    USHORT E2PValue = 0;
    RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
    USHORT Value = 0;
    CHAR BWPowerDelta = 0;

    if (pAd->TxPowerCtrl.bInternalTxALC == FALSE)
    {
		DBGPRINT(RT_DEBUG_OFF, ("pAd->TxPowerCtrl.bInternalTxALC == FALSE !\n"));
		return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

    RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, Value);
            
	if ((Value & 0xFF) == 0xFF) /* 20/40M BW Power Delta */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: 20/40M BW Delta Power is disabled since EEPROM is not calibrated.\n", __FUNCTION__));
	}
	else
	{
		if ((Value & 0xC0) == 0xC0)
		{
			BWPowerDelta += (Value & 0x3F); /* increase 40M BW TX power with the delta value */
		}
		else if ((Value & 0xC0) == 0x80)
		{
			BWPowerDelta -= (Value & 0x3F); /* decrease 40M BW TX power with the delta value */
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("%s: 20/40M BW Delta Power is not enabled, Value = 0x%X\n", __FUNCTION__, Value));
	}

	/* initialize the ratio used in non-linear TSSI */
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TSSI_STEP_OVER_2DOT4G - 1), E2PValue);

	/* 
		Bit 2 of EEPROM 0x77 is 1, 
		updated internal TSSI ratio table
		(slope is 0.7 instead of 0.85) is selected.
	*/
	if (((E2PValue >> 8) & 0x04) == 0x04)
	{
		pChipOps->TSSIRatio = TSSIRatioDot7;
		DBGPRINT(RT_DEBUG_OFF, ("%s : slope of TSSI is 0.7\n", __FUNCTION__));
	}
	else
	{
		pChipOps->TSSIRatio = TSSIRatioDot85;
		DBGPRINT(RT_DEBUG_OFF, ("%s : slope of TSSI is 0.85\n", __FUNCTION__));
	}

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, TSSIBase);
	TSSI_ref = TSSIBase = (TSSIBase & 0x003F);

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, TSSIDelta);
    TSSI_Set[1].delta = ((TSSIDelta >> 8)  & 0xF);/* channel 1 TSSI delta */
    TSSI_Set[2].delta = ((TSSIDelta >> 12) & 0xF);/* channel 2 TSSI delta */

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_DELTA_CH3_CH4, TSSIDelta);
    TSSI_Set[3].delta = ((TSSIDelta >> 0)  & 0xF);/* channel 3 TSSI delta */
    TSSI_Set[4].delta = ((TSSIDelta >> 4)  & 0xF);/* channel 4 TSSI delta */
    TSSI_Set[5].delta = ((TSSIDelta >> 8)  & 0xF);/* channel 5 TSSI delta */
    TSSI_Set[6].delta = ((TSSIDelta >> 12) & 0xF);/* channel 6 TSSI delta */
    
	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_DELTA_CH7_CH8, TSSIDelta);
    TSSI_Set[7].delta = ((TSSIDelta >> 0) & 0xF);/* channel 7 TSSI delta */
    TSSI_Set[8].delta = ((TSSIDelta >> 4) & 0xF);/* channel 8 TSSI delta */
    TSSI_Set[9].delta = ((TSSIDelta >> 8) & 0xF);/* channel 9 TSSI delta */
    TSSI_Set[10].delta = ((TSSIDelta >> 12)  & 0xF);/* channel 10 TSSI delta */

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_DELTA_CH11_CH12, TSSIDelta);
    TSSI_Set[11].delta = ((TSSIDelta >> 0)  & 0xF);/*channel 11 TSSI delta */
    TSSI_Set[12].delta = ((TSSIDelta >> 4)  & 0xF);/* channel 12 TSSI delta */
    TSSI_Set[13].delta = ((TSSIDelta >> 8)  & 0xF);/* channel 13 TSSI delta */
    TSSI_Set[14].delta = ((TSSIDelta >> 12)  & 0xF);/* channel 14 TSSI delta */

	for(i=1;i<15;i++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Channel %d TSSI delta=%d\n", i, TSSI_Set[i].delta));
	}

	TSSI_x.delta = TSSI_Set[pAd->CommonCfg.Channel].delta;

	RT28xx_EEPROM_READ16(pAd, (EEPROM_OFDM_MCS6_MCS7 - 1), TxPowerOFDM54);
	TxPowerOFDM54 = (0x000F & (TxPowerOFDM54 >> 8));

	DBGPRINT(RT_DEBUG_OFF, ("TSSIBase(0x6E) = %X, TxPowerOFDM54 = %X\n", TSSIBase, TxPowerOFDM54));

    /* The desired TSSI over CCK */
    RT28xx_EEPROM_READ16(pAd, EEPROM_CCK_MCS0_MCS1, TxPower);
    TxPower = (TxPower & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_CCK_MCS0_MCS1(0xDE) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverCCK[MCS_0] = TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + 3 + TSSI_x.delta);
    RT5350_desiredTSSIOverCCK[MCS_1] = RT5350_desiredTSSIOverCCK[MCS_0];

    RT28xx_EEPROM_READ16(pAd, (EEPROM_CCK_MCS2_MCS3 - 1), TxPower);
    TxPower = ((TxPower >> 8) & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_CCK_MCS2_MCS3(0xDF) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverCCK[MCS_2] = TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + 3 + TSSI_x.delta);
    RT5350_desiredTSSIOverCCK[MCS_3] = RT5350_desiredTSSIOverCCK[MCS_2];

    /* Boundary verification: the desired TSSI value */
    for (i = 0; i < 4; i++)/* CCK: MCS 0 ~ MCS 3 */
    {
		if(RT5350_desiredTSSIOverCCK[i] > 0x7C*10000)
		{
			RT5350_desiredTSSIOverCCK[i] = 0x7C*10000;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("RT5350_desiredTSSIOverCCK[%d] = %d\n", i, RT5350_desiredTSSIOverCCK[i]));
    }

    /* The desired TSSI over OFDM */
    RT28xx_EEPROM_READ16(pAd, EEPROM_OFDM_MCS0_MCS1, TxPower);
    TxPower = (TxPower & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_OFDM_MCS0_MCS1(0xE0) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverOFDM[MCS_0] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta));
    RT5350_desiredTSSIOverOFDM[MCS_1] = RT5350_desiredTSSIOverOFDM[MCS_0];


    RT28xx_EEPROM_READ16(pAd, (EEPROM_OFDM_MCS2_MCS3-1), TxPower);
    TxPower = ((TxPower >> 8) & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_OFDM_MCS2_MCS3(0xE1) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverOFDM[MCS_2] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta));
    RT5350_desiredTSSIOverOFDM[MCS_3] = RT5350_desiredTSSIOverOFDM[MCS_2];

    RT28xx_EEPROM_READ16(pAd, EEPROM_OFDM_MCS4_MCS5, TxPower);
    TxPower = (TxPower & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_OFDM_MCS4_MCS5(0xE2) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverOFDM[MCS_4] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta));
    RT5350_desiredTSSIOverOFDM[MCS_5] = RT5350_desiredTSSIOverOFDM[MCS_4];

    RT28xx_EEPROM_READ16(pAd, EEPROM_OFDM_MCS6_MCS7-1, TxPower);
    TxPower = ((TxPower >> 8) & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_OFDM_MCS6_MCS7(0xE3) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverOFDM[MCS_6] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, 0));
    RT5350_desiredTSSIOverOFDM[MCS_7] = RT5350_desiredTSSIOverOFDM[MCS_6];

    /* Boundary verification: the desired TSSI value */
    for (i = 0; i < 8; i++)/* OFDM: MCS 0 ~ MCS 7 */
    {
		if (RT5350_desiredTSSIOverOFDM[i] > 0x7C*10000)
		{
			RT5350_desiredTSSIOverOFDM[i] = 0x7C*10000;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("RT5350_desiredTSSIOverOFDM[%d] = %d\n", i, RT5350_desiredTSSIOverOFDM[i]));
    }

    /* The desired TSSI over HT */
    RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS0_MCS1, TxPower);
    TxPower = (TxPower & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS0_MCS1(0xE4) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverHT[MCS_0] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta));
    RT5350_desiredTSSIOverHT[MCS_1] = RT5350_desiredTSSIOverHT[MCS_0];
    RT5350_desiredTSSIOverHT40[MCS_0] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta + BWPowerDelta));
    RT5350_desiredTSSIOverHT40[MCS_1] = RT5350_desiredTSSIOverHT40[MCS_0];

    RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS2_MCS3-1), TxPower);
    TxPower = ((TxPower >> 8) & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS2_MCS3(0xE5) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverHT[MCS_2] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta));
    RT5350_desiredTSSIOverHT[MCS_3] = RT5350_desiredTSSIOverHT[MCS_2];
    RT5350_desiredTSSIOverHT40[MCS_2] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta + BWPowerDelta));
    RT5350_desiredTSSIOverHT40[MCS_3] = RT5350_desiredTSSIOverHT40[MCS_2];

    RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS4_MCS5, TxPower);
    TxPower = (TxPower & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS4_MCS5(0xE6) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverHT[MCS_4] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta));
    RT5350_desiredTSSIOverHT[MCS_5] = RT5350_desiredTSSIOverHT[MCS_4];
    RT5350_desiredTSSIOverHT40[MCS_4] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta + BWPowerDelta));
    RT5350_desiredTSSIOverHT40[MCS_5] = TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta - 1 + BWPowerDelta);

    RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS6_MCS7-1), TxPower);
    TxPower = ((TxPower >> 8) & 0x000F);
    DBGPRINT(RT_DEBUG_TRACE, ("EEPROM_HT_MCS6_MCS7(0xE7) = 0x%X\n", TxPower));
    RT5350_desiredTSSIOverHT[MCS_6] = (TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta));
    RT5350_desiredTSSIOverHT[MCS_7] = RT5350_desiredTSSIOverHT[MCS_6];
    RT5350_desiredTSSIOverHT40[MCS_6] = TSSIBase * RTMP_CHIP_ASIC_GET_TSSI_RATIO(pAd, TxPower - TxPowerOFDM54 + TSSI_x.delta - 1 + BWPowerDelta);
    RT5350_desiredTSSIOverHT40[MCS_7] = RT5350_desiredTSSIOverHT40[MCS_6];

    /* Boundary verification: the desired TSSI value */
    for (i = 0; i < 8; i++)/* HT: MCS 0 ~ MCS 7 */
    {
		if (RT5350_desiredTSSIOverHT[i] > 0x7C*10000)
		{
			RT5350_desiredTSSIOverHT[i] = 0x7C*10000;
		}

		if (RT5350_desiredTSSIOverHT40[i] > 0x7C*10000)
		{
			RT5350_desiredTSSIOverHT40[i] = 0x7C*10000;
		}

		DBGPRINT(RT_DEBUG_OFF, ("desiredTSSIOverHT[%d] = %d\n", i, RT5350_desiredTSSIOverHT[i]));
		DBGPRINT(RT_DEBUG_OFF, ("desiredTSSIOverHT40[%d] = %d\n", i, RT5350_desiredTSSIOverHT40[i]));
    }

	/* STBC is not supported by 1T1R chipset */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_GAIN_ATTENUATION, TSSIParm);
   
	/* if EEPROM 0x76 is 0xFF or 0x00, sw will ignore the EEPROM & use sw default value */
	if (TSSIParm==0x00 || TSSIParm==0xFF)
	{
		/* tssi_gain<1:0>:
		*   11: 12db
		*   10: 9.5db
		*   01: 6db (default)
		*   00: 0db
		* tssi_atten<3:2>:
		*   00: -16db
		*   01: -19db
		*   10: -21 (default)
		*   11: -23
		*/
		RFValue = (0x3 | 0x0<<2); 
	}
	else
	{
		TSSIGain = (TSSIParm & 0x3);
		TSSIAttenuation = (TSSIParm >> 2) & 0x3;
		RFValue |= TSSIGain;
		RFValue |= (TSSIAttenuation << 2);
	}

	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

	RFValue = 0x0;
	RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

	RT30xxReadRFRegister(pAd, RF_R29, &RFValue);
	RFValue = 0x0;
	RT30xxWriteRFRegister(pAd, RF_R29, RFValue);

	/* ADC6_on=1, TSSI_MODE=2(new averaged TSSI mode) */
	BbpR47 = (0x2<<3 | 0x1 << 7);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);

	/* Initialize the index of the internal Tx ALC table */
	pAd->TxPowerCtrl.idxTxPowerTable = pAd->TxPower[pAd->CommonCfg.Channel].Power;

}


UCHAR CCK_Rate2MCS(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR mcs = 0, dot11b_rate;


	UCHAR  BbpR47 = 0;
	UCHAR  TssiInfo1 = 0;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
	BbpR47 &= ~0x3;
	BbpR47 |= 1;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo1);
	dot11b_rate = (TssiInfo1 >> 4) & 0xF; // tssi_report[13:12]=11b rate

	switch(dot11b_rate) {
                case 8:
                        mcs = 0;        /* Long preamble CCK 1Mbps */
                        break;
                case 9:
                        mcs = 1;        /* Long preamble CCK 2Mbps */
                        break;
                case 10:
                        mcs = 2;        /* Long preamble CCK 5.5Mbps */
                        break;
                case 11:
                        mcs = 3;        /* Long preamble CCK 11Mbps */
                        break;
                default:
                        mcs = 0;
        };

        return mcs;
}


UCHAR OFDM_Rate2MCS(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR mcs = 0, ofdm_rate;


	UCHAR  BbpR47 = 0;
	UCHAR  TssiInfo1 = 0;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
	BbpR47 &= ~0x3;
	BbpR47 |= 1;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo1);
	ofdm_rate = (TssiInfo1 >> 4) & 0xF; //rssi_15:12]=ofdm_rate

	switch (ofdm_rate) {
			case 0xb:
					mcs = 0; /* 6Mbps Rate */
					break;
			case 0xf:
					mcs = 1; /* 9Mbps Rate */
					break;
			case 0xa:
					mcs = 2; /* 12Mbps Rate */
					break;
			case 0xe:
					mcs = 3; /* 18Mbps Rate */
					break;
			case 0x9:
					mcs = 4; /* 24Mbps Rate */
					break;
			case 0xd:
					mcs = 5; /* 36Mbps Rate */
					break;
			case 0x8:
					mcs = 6; /* 48Mbps Rate */
					break;
			case 0xc:
					mcs = 7; /* 54Mbps Rate */
					break;
			default:
					mcs = 6;
	};

	return mcs;
}


/*
        ==========================================================================
        Description:
                Get the desired TSSI based on the latest packet

        Arguments:
                pAd
		pBbpR49

        Return Value:
                The desired TSSI
        ==========================================================================
 */
UINT32 RT5350_GetDesiredTSSI(
	IN PRTMP_ADAPTER		pAd,
	OUT PUCHAR				pBbpR49)
{
	UINT32			desiredTSSI = 0;
	UCHAR			MCS = 0;
	UCHAR			BbpR47 = 0;
	UCHAR			TssiInfo0 = 0;
	UCHAR			TssiInfo1 = 0;
	UCHAR			TssiInfo2 = 0;
	UCHAR			i;
	UCHAR			BBP_Bandwidth = 0;
	UCHAR			ofdm_rate = 0, dot11b_rate = 0;

	BBP_Bandwidth = pAd->CommonCfg.BBPCurrentBW;

	/* Get TSSI_INFO */
	for (i=0;i<100;i++)
	{
	    RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);

	    if (!(BbpR47 & (1<<2)))
	    { 
			/* self-cleared when the TSSI_INFO is updated */
			/* Get TSSI_INFO0 = tssi_report[7:0] */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
			BbpR47 &= ~0x3;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo0);

			/* If TSSI reading is not within 0x0~0x7C, then treated it as 0. */
			if (TssiInfo0 > 0x7C)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("TSSI: BBP_R49=%X is native value\n", TssiInfo0));
				*pBbpR49 = TssiInfo0 = 0;
			}
			else
			{
				*pBbpR49 = TssiInfo0;
			}

			/* Get TSSI_INFO1 = tssi_report[15:8] */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
			BbpR47 &= ~0x3;
			BbpR47 |= 1;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo1);

			switch (TssiInfo1 & 0x03)
			{
				case 0: /* CCK */
					dot11b_rate = (TssiInfo1 >> 4) & 0xF; /* tssi_report[13:12]=11b rate */

					switch(dot11b_rate)
					{
						case 8:
							MCS = 0;        /* Long preamble CCK 1Mbps */
							break;
						case 9:
							MCS = 1;        /* Long preamble CCK 2Mbps */
							break;
						case 10:
							MCS = 2;        /* Long preamble CCK 5.5Mbps */
							break;
						case 11:
							MCS = 3;        /* Long preamble CCK 11Mbps */
							break;
						default:
							MCS = 0;
							break;
					}
					desiredTSSI = RT5350_desiredTSSIOverCCK[MCS];
				    DBGPRINT(RT_DEBUG_INFO, ("CCK: desiredTSSI = %d, MCS = %d\n", desiredTSSI, MCS));
					break;

				case 1: /* OFDM */
					ofdm_rate = (TssiInfo1 >> 4) & 0xF; /* rssi_15:12]=ofdm_rate */

					switch (ofdm_rate)
					{
						case 0xb:
							MCS = 0; /* 6Mbps Rate */
							break;
						case 0xf:
							MCS = 1; /* 9Mbps Rate */
							break;
						case 0xa:
							MCS = 2; /* 12Mbps Rate */
							break;
						case 0xe:
							MCS = 3; /* 18Mbps Rate */
							break;
						case 0x9:
							MCS = 4; /* 24Mbps Rate */
							break;
						case 0xd:
							MCS = 5; /* 36Mbps Rate */
							break;
						case 0x8:
							MCS = 6; /* 48Mbps Rate */
							break;
						case 0xc:
							MCS = 7; /* 54Mbps Rate */
							break;
						default:
							MCS = 6;
	                        break;
					};
					desiredTSSI = RT5350_desiredTSSIOverOFDM[MCS];
				    DBGPRINT(RT_DEBUG_INFO, ("OFDM: desiredTSSI = %d, MCS = %d\n", desiredTSSI, MCS));
					break;

				case 2: /* HT */
				    DBGPRINT(RT_DEBUG_INFO, ("mixed mode or green-field mode\n"));

					/* Get TSSI_INFO2 = tssi_report[23:16] */
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
					BbpR47 &= ~0x3;
					BbpR47 |= 1<<1;
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo2);
					MCS = TssiInfo2 & 0x7F; /* tssi_report[22:16]=MCS */
					if ((BBP_Bandwidth == BW_40)
						&& ((MCS == 5) || (MCS == 6) || (MCS == 7)))
					{
						desiredTSSI = RT5350_desiredTSSIOverHT40[MCS];
					}
					else
						desiredTSSI = RT5350_desiredTSSIOverHT[MCS];

				    DBGPRINT(RT_DEBUG_INFO, ("HT: desiredTSSI = %d, MCS = %d\n", desiredTSSI, MCS));
					break;
			}
			break;
	    }
	}

	/* clear TSSI_UPDATE_REQ first */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
	BbpR47 &= ~0x7;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

	/* write 1 to enable TSSI_INFO update */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
	BbpR47 |= (1<<2); /* TSSI_UPDATE_REQ */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

	return desiredTSSI;
}
#endif /* RTMP_INTERNAL_TX_ALC */


VOID RT5350_ChipAGCInit(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth)
{
	UCHAR	R66 = 0x30;
	
	if (pAd->LatchRfRegs.Channel <= 14)
	{	// BG band
		R66 = 0x2E + GET_LNA_GAIN(pAd);
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	}
	else
	{	//A band
		if (BandWidth == BW_20)
			R66 = (UCHAR)(0x32 + (GET_LNA_GAIN(pAd)*5)/3);
#ifdef DOT11_N_SUPPORT
		else
			R66 = (UCHAR)(0x3A + (GET_LNA_GAIN(pAd)*5)/3);
#endif // DOT11_N_SUPPORT //
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	}

}

#endif // RT5350 //

/* End of rt5350.c */
