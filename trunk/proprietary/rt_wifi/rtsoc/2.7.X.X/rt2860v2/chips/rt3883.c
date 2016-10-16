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
	rt3883.c

	Abstract:
	Specific funcitons and configurations for RT3883

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3883

#include	"rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

UCHAR RT3883_EeBuffer[EEPROM_SIZE] = {
	0x83, 0x38, 0x01, 0x00, 0x00, 0x0c, 0x43, 0x28, 0x83, 0x00, 0x83, 0x28, 0x14, 0x18, 0xff, 0xff,
	0xff, 0xff, 0x83, 0x28, 0x14, 0x18, 0x00, 0x00, 0x01, 0x00, 0x6a, 0xff, 0x00, 0x02, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 0x43, 0x28, 0x83, 0x01, 0x00, 0x0c,
	0x43, 0x28, 0x83, 0x02, 0x33, 0x0a, 0xec, 0x00, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x20, 0x01, 0x55, 0x77, 0xa8, 0xaa, 0x8c, 0x88, 0xff, 0xff, 0x0a, 0x08, 0x08, 0x06,
	0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x66, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa,
	0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa,
	0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa,
	0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa,
	0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	} ;


FREQUENCY_ITEM FreqItems3883[] =
{
	/**************************************************/
	/* ISM : 2.4 to 2.483 GHz                         */
	/**************************************************/
	/*-CH---N-------R---K-----------*/
	/*  R : <pll_mode[3:2], pll_R[1:0]>, g band: pll_mode=01, pll_R=10*/
	{1,    241,  6,  2},
	{2,    241,	 6,  7},
	{3,    242,	 6,  2},
	{4,    242,	 6,  7},
	{5,    243,	 6,  2},
	{6,    243,	 6,  7},
	{7,    244,	 6,  2},
	{8,    244,	 6,  7},
	{9,    245,	 6,  2},
	{10,   245,	 6,  7},
	{11,   246,	 6,  2},
	{12,   246,	 6,  7},
	{13,   247,	 6,  2},
	{14,   248,	 6,  4},

	/**************************************************/
	/* 5 GHz*/
	/**************************************************/
	/*  R : <pll_mode[3:2], pll_R[1:0]>, g band: pll_mode=10, pll_R=00*/
	{36,	0x56,	8,	4},
	{38,	0x56,	8,	6},
	{40,	0x56,	8,	8},
	{44,	0x57,	8,	0},
	{46,	0x57,	8,	2},
	{48,	0x57,	8,	4},
	{52,	0x57,	8,	8},
	{54,	0x57,	8,	10},
	{56,	0x58,	8,	0},
	{60,	0x58,	8,	4},
	{62,	0x58,	8,	6},
	{64,	0x58,	8,	8},

	{100,	0x5B,	8,	8},
	{102,	0x5B,	8,	10},
	{104,	0x5C,	8,	0},
	{108,	0x5C,	8,	4},
	{110,	0x5C,	8,	6},
	{112,	0x5C,	8,	8},
	{114,	0x5C,	8,	10},
	{116,	0x5D,	8,	0},
	{118,	0x5D,	8,	2},
	{120,	0x5D,	8,	4},
	{124,	0x5D,	8,	8},
	{126,	0x5D,	8,	10},
	{128,	0x5E,	8,	0},
	{132,	0x5E,	8,	4},
	{134,	0x5E,	8,	6},
	{136,	0x5E,	8,	8},
	{140,	0x5F,	8,	0},

	{149,	0x5F,	8,	9},
	{151,	0x5F,	8,	11},
	{153,	0x60,	8,	1},
	{157,	0x60,	8,	5},
	{159,	0x60,	8,	7},
	{161,	0x60,	8,	9},
	{165,	0x61,	8,	1},
/*	{167,	0x61,	8,	3},*/
/*	{169,	0x61,	8,	5},*/
/*	{171,	0x61,	8,	7},*/
/*	{173,	0x61,	8,	9},*/
	{184,	0x52,	8,	0},
	{188,	0x52,	8,	4},
	{192,	0x52,	8,	8},
	{196,	0x53,	8,	0},
};
UCHAR NUM_OF_3883_CHNL = (sizeof(FreqItems3883) / sizeof(FREQUENCY_ITEM));

static REG_PAIR   RT3883_RFRegTable[] = {
	{RF_R00, 0xE0},
	{RF_R01, 0x03},
	{RF_R02, 0x50},
	{RF_R03, 0x20},
	{RF_R04, 0x00},
	{RF_R05, 0x00},
	{RF_R06, 0x40},
	{RF_R07, 0x00},
	{RF_R08, 0x5B},
	{RF_R09, 0x08},
	{RF_R10, 0xD3}, /* Gary, 2010-02-12 */
	{RF_R11, 0x48},
	{RF_R12, 0x1A},	/* Gary, 2011-03-10 */
	{RF_R13, 0x12},
	{RF_R14, 0x00},
	{RF_R15, 0x00},
	{RF_R16, 0x00},
/*	{RF_R17, 0x26}, */
	{RF_R18, 0x40},
	{RF_R19, 0x00},
	{RF_R20, 0x00},
	{RF_R21, 0x00},
	{RF_R22, 0x20},
	{RF_R23, 0xC0},
	{RF_R24, 0x00},
	{RF_R25, 0x00},
	{RF_R26, 0x00},
	{RF_R27, 0x00},
	{RF_R28, 0x00},
	{RF_R29, 0x00},
	{RF_R30, 0x10},
	{RF_R31, 0x80},
	{RF_R32, 0x80},
	{RF_R33, 0x00},
	{RF_R34, 0x20},
	{RF_R35, 0x00},
	{RF_R36, 0x00},
	{RF_R37, 0x00},
	{RF_R38, 0x86},	/* Gary, 2010-02-12 */
	{RF_R39, 0x23},
	{RF_R40, 0x00},
	{RF_R41, 0x00},
	{RF_R42, 0x00},
	{RF_R43, 0x00},
	{RF_R44, 0x93},	/* Gary, 2009-12-08 */
	{RF_R45, 0xBB},
	{RF_R46, 0x60},
	{RF_R47, 0x00},
	{RF_R48, 0x00},
	{RF_R49, 0x8E},
	{RF_R50, 0x86},
	{RF_R51, 0x51},
	{RF_R52, 0x05},
	{RF_R53, 0x76},
	{RF_R54, 0x76},
	{RF_R55, 0x76},
	{RF_R56, 0xDB},
	{RF_R57, 0x3E},
	{RF_R58, 0x00},
	{RF_R59, 0x00},
	{RF_R60, 0x00},
	{RF_R61, 0x00},
	{RF_R62, 0x00},
	{RF_R63, 0x00},
};

static UCHAR RT3883_NUM_RF_REG_PARMS = (sizeof(RT3883_RFRegTable) / sizeof(REG_PAIR));


static REG_PAIR   RT3883_BBPRegTable[] = {
	{BBP_R4,		0x50}, /* 3883 need to */
	{BBP_R47,		0x48},  /* ALC Functions change from 0x7 to 0x48 Baron suggest */

	{BBP_R86,		0x46},	/* for peak throughput, Henry 2009-12-23 */
	{BBP_R88,		0x90},	/* for rt3883 middle range, Henry 2009-12-31 */

	{BBP_R92,		0x02},  /* middle range issue, Rory @2008-01-28 */

	{BBP_R103,  		0xC0},
	{BBP_R104,		0x92},
	{BBP_R105,		0x34},
#ifdef DOT11_N_SUPPORT
	{BBP_R106,		0x12},	// 40M=2, 20M=2. Fix 20M SGI STBC problem
#endif /* DOT11_N_SUPPORT */
	{BBP_R120,		0x50},	/* for long range -2db, Gary 2010-01-22 */
	{BBP_R137,		0x0F},  /* Julian suggest make the RF output more stable */
	{BBP_R163,		0x9D}, /* Enable TxBf modes by default, Gary, 2010-06-02 */

	{BBP_R179,		0x02},	/* Set ITxBF timeout to 0x9C40=1000msec */
	{BBP_R180,		0x00},
	{BBP_R182,		0x40},
	{BBP_R180,		0x01},
	{BBP_R182,		0x9C},

	{BBP_R179,		0x00},

	{BBP_R142,		0x04},	/* Reprogram the inband interface to put right values in RXWI */
	{BBP_R143,		0x3b},
	{BBP_R142,		0x06},
	{BBP_R143,		0xA0},
	{BBP_R142,		0x07},
	{BBP_R143,		0xA1},
	{BBP_R142,		0x08},
	{BBP_R143,		0xA2},
	{BBP_R148,		0xC8},	/* Gary, 2010-2-12 */
};

static UCHAR RT3883_NUM_BBP_REG_PARMS = (sizeof(RT3883_BBPRegTable) / sizeof(REG_PAIR));


static RTMP_REG_PAIR	RT3883_MACRegTable[] =	{
	{TX_SW_CFG0,		0x402},   /* Gary,2009-9-21*/
	{TX_SW_CFG1,		0x0},     /* FIXME*/
	{TX_SW_CFG2,		0x40000}, /* FIXME*/
	{TX_TXBF_CFG_0,		0x8000FC21},	/* Force MCS0 for sounding response*/
	{TX_TXBF_CFG_3,		0x00009c40},	/* ETxBF Timeout = 1 sec = 0x9c40*(25 usec)*/

#ifdef RANGE_EXTEND
	{HT_FBK_CFG1,			0xedcba980},	/* Fallback MCS8->MCS0 */
#endif /* RANGE_EXTEND */
	{TX_FBK_CFG_3S_0,	0x12111008},	/* default value*/
	{TX_FBK_CFG_3S_1,	0x16151413},	/* default value*/
};
static UCHAR RT3883_NUM_MAC_REG_PARMS = (sizeof(RT3883_MACRegTable) / sizeof(RTMP_REG_PAIR));

static VOID NICInitRT3883RFRegisters(IN PRTMP_ADAPTER pAd)
{

	/* Init RF calibration */
	/* Driver should toggle RF R02 bit7 before init RF registers */
	UINT8 RfReg = 0;  
	int i;

	/* Initialize RF register to default value */
	for (i = 0; i < RT3883_NUM_RF_REG_PARMS; i++)
	{
		RT30xxWriteRFRegister(pAd, RT3883_RFRegTable[i].Register, RT3883_RFRegTable[i].Value);
	}

	if ((pAd->CommonCfg.CID & 0x0000000f) >= 0x00000005)
	{
		RT30xxWriteRFRegister(pAd, RF_R32, 0xD8);
		RT30xxWriteRFRegister(pAd, RF_R33, 0x3B);
	}
	RT30xxWriteRFRegister(pAd, RF_R33, 0x3f);

	/*RF_R02: Resistor calibration, RF_R02 = RF_R30 (RT30xx) */
	RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
	RfReg &= ~(1 << 6); /* clear bit6=rescal_bp */
	RfReg |= 0x80; /* bit7=rescal_en */
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
	/*DBGPRINT(RT_DEBUG_WARN,("SS \n")); */
	RTMPusecDelay(1000);
	RfReg &= 0x7F;
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);     

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
static VOID NICInitRT3883MacRegisters(
	IN RTMP_ADAPTER				*pAd)
{
	UINT32 IdReg;

	for(IdReg=0; IdReg<RT3883_NUM_MAC_REG_PARMS; IdReg++)
	{
		RTMP_IO_WRITE32(pAd, RT3883_MACRegTable[IdReg].Register,
								RT3883_MACRegTable[IdReg].Value);
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
static VOID NICInitRT3883BbpRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	/* dummy function */
}


static VOID RT3883_AsicAntennaDefaultReset(
	IN struct _RTMP_ADAPTER	*pAd,
	IN EEPROM_ANTENNA_STRUC *pAntenna)
{
	pAntenna->word = 0;
	pAntenna->field.RfIcType = RFIC_3853;
	pAntenna->field.TxPath = 3;
	pAntenna->field.RxPath = 3;
}


static VOID RT3883_RxSensitivityTuning(
	IN struct _RTMP_ADAPTER *pAd)
{
	UINT8 bbp_val;

	bbp_val = 0x26 + GET_LNA_GAIN(pAd);
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x0);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, bbp_val);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x20);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, bbp_val);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x40);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, bbp_val);
	}
	else
#endif /* RALINK_ATE */
		AsicBBPWriteWithRxChain(pAd, BBP_R66, bbp_val, RX_CHAIN_ALL);
}

VOID RT3883_AsicGetTxPowerOffset(
	IN		PRTMP_ADAPTER			pAd,
	INOUT 	PULONG 					pTxPwr)
{
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;

	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	if (IS_RT3883(pAd))
	{
		/* MAC 0x1314, 0x1318, 0x131C, 0x1320 and 1324 */
		CfgOfTxPwrCtrlOverMAC.NumOfEntries = MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS; 

		if (pAd->CommonCfg.BBPCurrentBW == BW_40)
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgABand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx40MPwrCfgABand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgABand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx40MPwrCfgABand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgABand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx40MPwrCfgABand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx40MPwrCfgABand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx40MPwrCfgABand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx40MPwrCfgABand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx40MPwrCfgABand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx40MPwrCfgABand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx40MPwrCfgABand[6];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx40MPwrCfgABand[7];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx40MPwrCfgABand[8];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx40MPwrCfgABand[9];
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgGBand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx40MPwrCfgGBand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgGBand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx40MPwrCfgGBand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgGBand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx40MPwrCfgGBand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx40MPwrCfgGBand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx40MPwrCfgGBand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx40MPwrCfgGBand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx40MPwrCfgGBand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx40MPwrCfgGBand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx40MPwrCfgGBand[6];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx40MPwrCfgGBand[7];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx40MPwrCfgGBand[8];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx40MPwrCfgGBand[9];
			}
		}
		else
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgABand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx20MPwrCfgABand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgABand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx20MPwrCfgABand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgABand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx20MPwrCfgABand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx20MPwrCfgABand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx20MPwrCfgABand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx20MPwrCfgABand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx20MPwrCfgABand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx20MPwrCfgABand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx20MPwrCfgABand[6];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx20MPwrCfgABand[7];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx20MPwrCfgABand[8];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx20MPwrCfgABand[9];
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgGBand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx20MPwrCfgGBand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgGBand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx20MPwrCfgGBand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgGBand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx20MPwrCfgGBand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx20MPwrCfgGBand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx20MPwrCfgGBand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx20MPwrCfgGBand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx20MPwrCfgGBand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx20MPwrCfgGBand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx20MPwrCfgGBand[6];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx20MPwrCfgGBand[7];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx20MPwrCfgGBand[8];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx20MPwrCfgGBand[9];
			}
		}
		NdisCopyMemory(pTxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));
	}
}

static VOID RT3883_ChipBBPAdjust(
	IN RTMP_ADAPTER			*pAd)
{
	UINT32 Value;
	UCHAR byteValue = 0;
	UCHAR r66_val;
				
#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
		(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		/*(pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)*/
	)
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

		/* request by Gary 20070208 for middle and long range G/A Band*/
		r66_val = (pAd->CommonCfg.Channel > 14) ? 0x48 : 0x38;
		AsicBBPWriteWithRxChain(pAd, BBP_R66, r66_val, RX_CHAIN_ALL);

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0a);*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);

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

		/* request by Gary 20070208 for middle and long range A/G Band*/
		r66_val = (pAd->CommonCfg.Channel > 14) ? 0x48 : 0x38;
		AsicBBPWriteWithRxChain(pAd, BBP_R66, r66_val, RX_CHAIN_ALL);
	
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0a);*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);
		
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
		/* request by Gary 20070208*/
		r66_val = (pAd->CommonCfg.Channel > 14) ? 0x40 : 0x38;
		AsicBBPWriteWithRxChain(pAd, BBP_R66, r66_val, RX_CHAIN_ALL);
				 
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0a);*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);

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
		/* Disable CCK packet detection in 5G band */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x00);
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x1D);*/
	}
	else
	{ 	/* request by Gary 20070208 for middle and long range G band*/
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x2D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x2D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x2D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x2D);*/
	}
}


#ifdef CONFIG_STA_SUPPORT
static VOID RT3883_NetDevNickNameInit(
	IN struct _RTMP_ADAPTER *pAd)
{
	snprintf((PSTRING) pAd->nickname, sizeof(pAd->nickname), "RT3883STA");
}


static UCHAR RT3883_ChipAGCAdjust(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi,
	IN UCHAR				OrigR66Value)
{
	UCHAR R66 = OrigR66Value;
	CHAR lanGain = GET_LNA_GAIN(pAd);

	
	if (pAd->LatchRfRegs.Channel <= 14) /* BG band */
		R66 = 0x2E + lanGain; 
	else /* A band */ 
		R66 = 0x20 + (lanGain * 5) /3; 

	if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
		R66 += 0x10;

	if (OrigR66Value != R66)
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);
	
	return R66;
}
#endif /* CONFIG_STA_SUPPORT */


static VOID RT3883_ChipAGCInit(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth)
{
	UCHAR	R66;
	CHAR lanGain = GET_LNA_GAIN(pAd);
	
	if (pAd->LatchRfRegs.Channel <= 14) /* BG band*/
		R66 = (UCHAR)(0x2E + lanGain);
	else /*A band*/
		R66 = (UCHAR)(0x20 + (lanGain * 5) / 3);

	AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


}


// RT3883_AsicSetFreqOffset - set Frequency Offset register
void RT3883_AsicSetFreqOffset(
	IN  PRTMP_ADAPTER   pAd,
	IN	ULONG			freqOffset)
{
	UCHAR RFValue = 0, RFValue2;

	freqOffset &= 0x7f;

	// Set RF offset  RF_R17=RF_R23 (RT30xx)
	RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RFValue);
	if ((UCHAR)freqOffset == (RFValue & 0x7f))
		return;

	RFValue2 = (RFValue & 0x80) | (UCHAR)freqOffset;

	RT30xxWriteRFRegister(pAd, RF_R17, RFValue2);

	RtmpOsMsDelay(1);
}


/*
	==========================================================================
	Description:

	Load RF sleep-mode setup
	
	==========================================================================
 */
static VOID RT3883LoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;

	/* RF_BLOCK_en. RF R1 register Bit 0 to 0 */
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue &= (~0x01);
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	/* VCO_IC, R6[7:6]=[11] */
	RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
	RFValue &= (~0xC0);
	RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

	/* charge pump current control (cp_ic) RF R22[7:5] = [111] */
	RT30xxReadRFRegister(pAd, RF_R22, &RFValue);
	RFValue &= (~0x20);
	RT30xxWriteRFRegister(pAd, RF_R22, RFValue);

	/* RX_CTB_en, RF R46[5]=[1] */
	RT30xxReadRFRegister(pAd, RF_R46, &RFValue);
	RFValue &= (~0x20);
	RT30xxWriteRFRegister(pAd, RF_R46, RFValue);

	/* LDO pll_vc RF_R20[3:1] = [000] */
	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue |= 0xEE;
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

}


/*
	==========================================================================
	Description:

	Reverse RF sleep-mode setup
	
	==========================================================================
 */
static VOID RT3883ReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd,
	IN BOOLEAN			FlgIsInitState)
{
	UCHAR RFValue;

	/* RF_BLOCK_en, RF R1 register Bit 0 to 1 */
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue |= 0x01;
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	/* VCO_IC, R6[7:6]=[11] */
	RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
	RFValue |= 0xC0;
	RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

	/* charge pump current control (cp_ic) RF R22[7:5] = [111] */
	RT30xxReadRFRegister(pAd, RF_R22, &RFValue);
	RFValue |= 0x20;
	RT30xxWriteRFRegister(pAd, RF_R22, RFValue);

	/* RX_CTB_en, RF R46[5]=[1] */
	RT30xxReadRFRegister(pAd, RF_R46, &RFValue);
	RFValue |= 0x20;
	RT30xxWriteRFRegister(pAd, RF_R46, RFValue);

	/* LDO pll_vc RF_R20[3:1] = [000] */
	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue = (RFValue & (~0xEE));
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

	/* VCO tuning code readback RF_R03[7]=1 */
	/*RT30xxWriteRFRegister(pAd, RF_R08, 0x80); */
}


static VOID RT3883HaltAction(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32		TxPinCfg = 0x00050F0F;

	/* */
	/* Turn off LNA_PE or TRSW_POL */
	/* */
#ifdef RTMP_EFUSE_SUPPORT
	if (pAd->bUseEfuse)
		TxPinCfg &= 0xFFFBF0F0; /* bit18 off */
	else
#endif /* RTMP_EFUSE_SUPPORT */
		TxPinCfg &= 0xFFFFF0F0;
	/*RT30xxWriteRFRegister(pAd, RF_R08, (UCHAR)0x00); */

	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);   
}


/*
	========================================================================
	
	Routine Description:
		Read initial Tx power per MCS and BW from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note: RT3883 EEPROM V0.1
		
	========================================================================
*/
VOID RTMPRT3883ReadTxPwrPerRate(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR		D0, D1, D2, D3;
	USHORT		value, value2;
	int		i;
	
	DBGPRINT(RT_DEBUG_TRACE, ("Txpower per Rate\n"));
	/* 2.4G @20M 1M/2M/5.5M/11M/6M/9M/12M/18M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_CCK_OFDM, value); /* EEPROM offset 0x140h */
	D0 = value & 0xF; /* 1M/2M */ 	
	D1 = (value >> 4) & 0xF; /* 5.5M/11M */
	D2 = (value >> 8) & 0xF; /* 6M/9M */
	D3 = (value >> 12) & 0xF; /* 12M/18M */

	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> 5.5M/11M; bit 7:0 -> 1M/2M */
	value2 = (D3 << 12) | (D3 << 8) | (D2 << 4) | D2; /* bit 15:8 -> 12M/18M; bit 7:0 -> 6M/9M */
	
	pAd->Tx20MPwrCfgGBand[0] = (value2 << 16) | value; /* TX_PWR_CFG_0, MAC 0x1314 */

	/* 2.4G @40M 6M/9M/12M/18M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_2_4G, value2); /* EEPROM 150h */
	D0 = value2 & 0xF; /* Reserved */
	D1 = (value2 >> 4) & 0xF; /* Reserved */
	D2 = (value2 >> 8) & 0xF; /* 6M/9M */
	D3 = (value2 >> 12) & 0xF; /* 12M/18M */
	value2 = (D3 << 12) | (D3 << 8) | (D2 << 4) | D2; /* bit 15:8 -> 12M/18M; bit 7:0 -> 6M/9M */

	pAd->Tx40MPwrCfgGBand[0] = (value2 << 16) | value; /* bit 15:0 is reserved, set to default value */

	/* 5G @20M 6M/9M/12M/18M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_OFDM_5G, value2); /* EEPROM 160h */
	D0 = value2 & 0xF; /* Reserved */
	D1 = (value2 >> 4) & 0xF; /* Reserved */
	D2 = (value2 >> 8) & 0xF; /* 6M/9M */
	D3 = (value2 >> 12) & 0xF; /* 12M/18M */
	value2 = (D3 << 12) | (D3 << 8) | (D2 << 4) | D2; /* bit 15:8 -> 12M/18M; bit 7:0 -> 6M/9M */

	pAd->Tx20MPwrCfgABand[0] = (value2 << 16) | value; /* bit 15:0 is reserved, set to default value */

	/* 5G @40M 6M/9M/12M/18M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_5G, value2); /* EEPROM 170h */
	D0 = value2 & 0xF; /* Reserved */
	D1 = (value2 >> 4) & 0xF; /* Reserved */
	D2 = (value2 >> 8) & 0xF; /* 6M/9M */
	D3 = (value2 >> 12) & 0xF; /* 12M/18M */
	value2 = (D3 << 12) | (D3 << 8) | (D2 << 4) | D2; /* bit 15:8 -> 12M/18M; bit 7:0 -> 6M/9M */

	pAd->Tx40MPwrCfgABand[0] = (value2 << 16) | value; /* bit 15:0 is reserved, set to default value */

	/* 2.4G @20M 24M/36M/48M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_CCK_OFDM + 2, value); /* EEPROM 142h */
	D0 = value & 0xF; /* 24M/36M */
	D1 = (value >> 4) & 0xF; /* 48M */
	D2 = (value >> 8) & 0xF; /* 54M */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> 48M; bit 7:0 -> 24M/36M */
	
	pAd->Tx20MPwrCfgGBand[1] = value; /* TX_PWR_CFG_1, MAC 0x1318 */

	/* 2.4G @40M 24M/36M/48M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_2_4G + 2, value); /* EEPROM 152h */
	D0 = value & 0xF; /* 24M/36M */
	D1 = (value >> 4) & 0xF; /* 48M */
	D2 = (value >> 8) & 0xF; /* 54M */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> 48M; bit 7:0 -> 24M/36M */

	pAd->Tx40MPwrCfgGBand[1] = value; /* TX_PWR_CFG_1, MAC 0x1318 */

	/* 5G @20M 24M/36M/48M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_OFDM_5G + 2, value); /* EEPROM 162h */
	D0 = value & 0xF; /* 24M/36M */
	D1 = (value >> 4) & 0xF; /* 48M */
	D2 = (value >> 8) & 0xF; /* 54M */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> 48M; bit 7:0 -> 24M/36M */

	pAd->Tx20MPwrCfgABand[1] = value; /* TX_PWR_CFG_1, MAC 0x1318 */

	/* 5G @20M 24M/36M/48M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_5G + 2, value); /* EEPROM 172h */
	D0 = value & 0xF; /* 24M/36M */
	D1 = (value >> 4) & 0xF; /* 48M */
	D2 = (value >> 8) & 0xF; /* 54M */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> 48M; bit 7:0 -> 24M/36M */

	pAd->Tx40MPwrCfgABand[1] = value; /* TX_PWR_CFG_1, MAC 0x1318 */

	/* 2.4G @20M, MCS0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G, value); /* EEPROM 144h */
	D0 = value & 0xF; /* MCS 0,1 */
	D1 = (value >> 4) & 0xF; /* MCS 2,3 */
	D2 = (value >> 8) & 0xF; /* MCS 4,5 */
	D3 = (value >> 12) & 0xF; /* MCS 6 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 2,3; bit 7:0 -> MCS 0,1 */

	pAd->Tx20MPwrCfgGBand[1] |= (value << 16);

	/* 2.4G @40M, MCS0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G, value); /* EEPROM 154h */
	D0 = value & 0xF; /* MCS 0,1 */
	D1 = (value >> 4) & 0xF; /* MCS 2,3 */
	D2 = (value >> 8) & 0xF; /* MCS 4,5 */
	D3 = (value >> 12) & 0xF; /* MCS 6 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 2,3; bit 7:0 -> MCS 0,1 */

	pAd->Tx40MPwrCfgGBand[1] |= (value << 16);

	/* 5G @20M, MCS0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G, value); /* EEPROM 164h */
	D0 = value & 0xF; /* MCS 0, 1 */
	D1 = (value >> 4) & 0xF; /* MCS 2,3 */
	D2 = (value >> 8) & 0xF; /* MCS 4,5 */
	D3 = (value >> 12) & 0xF; /* MCS 6 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 2,3; bit 7:0 -> MCS 0,1 */

	pAd->Tx20MPwrCfgABand[1] |= (value << 16);

	/* 5G @40M, MCS0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G, value); /* EEPROM 174h */
	D0 = value & 0xF; /* MCS 0,1 */
	D1 = (value >> 4) & 0xF; /* MCS 2,3 */
	D2 = (value >> 8) & 0xF; /* MCS 4,5 */
	D3 = (value >> 12) & 0xF; /* MCS 6 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 2,3; bit 7:0 -> MCS 0,1 */

	pAd->Tx40MPwrCfgABand[1] |= (value << 16);

	/* 2.4G @20M, MCS 4, 5, 6, 7, 8, 9, 10, 11 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 1, value); /* EEPROM 145h */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 3, value2); /* EEPROM 147h */
	D0 = value & 0xF; /* MCS 4,5 */
	D1 = (value >> 4) & 0xF; /* MCS 6 */
	D2 = (value >> 8) & 0xF; /* MCS 7 */
	D3 = (value >> 12) & 0xF; /* MCS 8,9 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 6; bit 7:0 -> MCS 4,5 */

	D0 = value2 & 0xF; /* MCS 10,11 */
	value2 = (D0 << 12) | (D0 << 8) | (D3 << 4) | D3; /* bit 15:8 -> MCS 10,11; bit 7:0 -> MCS 8,9 */

	pAd->Tx20MPwrCfgGBand[2] = (value2 << 16) | value;

	/* 2.4G @40M, MCS 4, 5, 6, 7, 8, 9, 10, 11 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 1, value); /* EEPROM 155h */		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 3, value2); /* EEPROM 157h */
	D0 = value & 0xF; /* MCS 4,5 */
	D1 = (value >> 4) & 0xF; /* MCS 6 */
	D2 = (value >> 8) & 0xF; /* MCS 7 */
	D3 = (value >> 12) & 0xF; /* MCS 8,9 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 6; bit 7:0 -> MCS 4,5 */

	D0 = value2 & 0xF; /* MCS 10,11 */
	value2 = (D0 << 12) | (D0 << 8) | (D3 << 4) | D3; /* bit 15:8 -> MCS 10,11; bit 7:0 -> MCS 8,9 */

	pAd->Tx40MPwrCfgGBand[2] = (value2 << 16) | value;

	/* 5G @20M, MCS 4, 5, 6, 7, 8, 9, 10, 11 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 1, value); /* EEPROM 165h */		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 3, value2); /* EEPROM 167h */
	D0 = value & 0xF; /* MCS 4,5 */
	D1 = (value >> 4) & 0xF; /* MCS 6 */
	D2 = (value >> 8) & 0xF; /* MCS 7 */
	D3 = (value >> 12) & 0xF; /* MCS 8,9 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 6; bit 7:0 -> MCS 4,5 */

	D0 = value2 & 0xF; /* MCS 10,11 */
	value2 = (D0 << 12) | (D0 << 8) | (D3 << 4) | D3; /* bit 15:8 -> MCS 10,11; bit 7:0 -> MCS 8,9 */

	pAd->Tx20MPwrCfgABand[2] = (value2 << 16) | value;

	/* 5G @40M, MCS 4, 5, 6, 7, 8, 9, 10, 11 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 1, value); /* EEPROM 175h */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 3, value2); /* EEPROM 177h */
	D0 = value & 0xF; /* MCS 4,5 */ 	
	D1 = (value >> 4) & 0xF; /* MCS 6 */
	D2 = (value >> 8) & 0xF; /* MCS 7 */
	D3 = (value >> 12) & 0xF; /* MCS 8,9 */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> MCS 6; bit 7:0 -> MCS 4,5 */
	
	D0 = value2 & 0xF; /* MCS 10,11 */
	value2 = (D0 << 12) | (D0 << 8) | (D3 << 4) | D3; /* bit 15:8 -> MCS 10,11; bit 7:0 -> MCS 8,9 */

	pAd->Tx40MPwrCfgABand[2] = (value2 << 16) | value;

	/* 2.4G @20M, MCS 12, 13, 14, STBC 0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 3, value); /* EEPROM 147h */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 8, value2); /* EEPROM 14Ch */
	D0 = value & 0xF; /* MCS 10,11 */
	D1 = (value >> 4) & 0xF; /* MCS 12,13 */
	D2 = (value >> 8) & 0xF; /* MCS 14 */
	D3 = (value >> 12) & 0xF; /* MCS 15 */
	value = (D2 << 12) | (D2 << 8) | (D1 << 4) | D1; /* bit 15:8 -> MCS 14; bit 7:0 -> MCS 12,13 */

	D0 = value2 & 0xF; /* STBC MCS 0,1 */
	D1 = (value2 >> 4) & 0xF; /* STBC MCS 2,3 */
	D2 = (value2 >> 8) & 0xF; /* STBC MCS 4,5 */
	D3 = (value2 >> 12) & 0xF; /* STBC MCS 6 */
	value2 = (D1 << 12) | (D1 << 8) |(D0 << 4) | D0; /* bit 15:8 STBC MCS 2,3; bit 7:0 -> STBC MCS 0,1 */

	pAd->Tx20MPwrCfgGBand[3] = (value2 << 16) | value;

	/* 2.4G @40M, MCS 12, 13, 14, STBC 0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 3, value);	/* EEPROM 157h */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 8, value2); /* EEPROM 15Ch */
	D0 = value & 0xF; /* MCS 10,11 */
	D1 = (value >> 4) & 0xF; /* MCS 12,13 */
	D2 = (value >> 8) & 0xF; /* MCS 14 */
	D3 = (value >> 12) & 0xF; /* MCS 15 */
	value = (D2 << 12) | (D2 << 8) | (D1 << 4) | D1; /* bit 15:8 -> MCS 14; bit 7:0 -> MCS 12,13 */

	D0 = value2 & 0xF; /* STBC MCS 0,1 */
	D1 = (value2 >> 4) & 0xF; /* STBC MCS 2,3 */
	D2 = (value2 >> 8) & 0xF; /* STBC MCS 4,5 */
	D3 = (value2 >> 12) & 0xF; /* STBC MCS 6 */
	value2 = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> STBC MCS 2,3; bit 7:0 -> STBC MCS 0,1 */
	pAd->Tx40MPwrCfgGBand[3] = (value2 << 16) | value;

	/* 5G @20M, MCS 12, 13, 14, STBC 0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 3, value); /* EEPROM 167h */		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 8, value2); /* EEPROM 16Ch */
	D0 = value & 0xF; /* MCS 10,11 */	
	D1 = (value >> 4) & 0xF; /* MCS 12,13 */
	D2 = (value >> 8) & 0xF; /* MCS 14 */
	D3 = (value >> 12) & 0xF; /* MCS 15 */
	value = (D2 << 12) | (D2 << 8) | (D1 << 4) | D1; /* bit 15:8 -> MCS 14; bit 7:0 -> MCS 12,13 */

	D0 = value2 & 0xF; /* STBC MCS 0,1 */
	D1 = (value2 >> 4) & 0xF; /* STBC MCS 2,3 */
	D2 = (value2 >> 8) & 0xF; /* STBC MCS 4,5 */
	D3 = (value2 >> 12) & 0xF; /* STBC MCS 6 */
	value2 = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> STBC MCS 2,3; bit 7:0 -> STBC MCS 0,1 */
	pAd->Tx20MPwrCfgABand[3] = (value2 << 16) | value;

	/* 5G @40M, MCS 12, 13, 14, STBC 0,1,2,3 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 3, value); /* EEPROM 177h */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 8, value2); /* EEPROM 17Ch */
	D0 = value & 0xF; /* MCS 10,11 */
	D1 = (value>>4) & 0xF; /* MCS 12,13 */
	D2 = (value>>8) & 0xF; /* MCS 14 */
	D3 = (value>>12) & 0xF; /* MCS 15 */
	value = (D2 << 12) | (D2 << 8) | (D1 << 4) | D1; /* bit 15:8 -> MCS 14; bit 7:0 -> MCS 12,13 */
	
	D0 = value2 & 0xF; /* STBC MCS 0,1 */
	D1 = (value2 >> 4) & 0xF; /* STBC MCS 2,3 */
	D2 = (value2 >> 8) & 0xF; /* STBC MCS 4,5 */
	D3 = (value2 >> 12) & 0xF; /* STBC MCS 6 */
	value2 = (D1 << 12) |(D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> STBC MCS 2,3; bit 7:0 -> STBC MCS 0,1 */
	pAd->Tx40MPwrCfgABand[3] = (value2 << 16) | value;

	/* 2.4G @20M, STBC 4,5,6 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 9, value); /* EEPROM 14Dh */		
	D0 = value & 0xF; /* STBC MCS 4,5 */
	D1 = (value >> 4) & 0xF; /* STBC MCS 6 */
	D2 = (value >> 8) & 0xF; /* STBC MCS 7 */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> STBC MCS 6; bit 7:0 -> STBC MCS 4,5 */
	pAd->Tx20MPwrCfgGBand[4] = value & 0x0000ffff;

	/* 2.4G @40M, STBC 4,5,6 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 9, value); /* EEPROM 15Dh */
	D0 = value & 0xF; /* STBC MCS 4,5 */
	D1 = (value >> 4) & 0xF; /* STBC MCS 6 */
	D2 = (value >> 8) & 0xF; /* STBC MCS 7 */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> STBC MCS 6; bit 7:0 -> STBC MCS 4,5 */
	pAd->Tx40MPwrCfgGBand[4] = value & 0x0000ffff;

	/* 5G @20M, STBC 4,5,6 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 9, value); /* EEPROM 16Dh */
	D0 = value & 0xF; /* STBC MCS 4,5 */
	D1 = (value >> 4) & 0xF; /* STBC MCS 6 */
	D2 = (value >> 8) & 0xF; /* STBC MCS 7 */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> STBC MCS 6; bit 7:0 -> STBC MCS 4,5 */
	pAd->Tx20MPwrCfgABand[4] = value & 0x0000ffff;

	/* 5G @40M, STBC 4,5,6 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 9, value); /* EEPROM 17Dh */
	D0 = value & 0xF; /* STBC MCS 4,5 */
	D1 = (value >> 4) & 0xF; /* STBC MCS 6 */
	D2 = (value >> 8) & 0xF; /* STBC MCS 7 */
	D3 = (value >> 12) & 0xF; /* Reserved */
	value = (D1 << 12) | (D1 << 8) | (D0 << 4) | D0; /* bit 15:8 -> STBC MCS 6; bit 7:0 -> STBC MCS 4,5 */
	pAd->Tx40MPwrCfgABand[4] = value & 0x0000ffff;


	/* 2.4G @20M, MCS 16,17,18,19,20,21,22 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 5, value); /* EEPROM 149h */
	D0 = value & 0xF; /* MCS 16,17 */
	D1 = (value >> 4) & 0xF; /* MCS 18,19 */
	D2 = (value >> 8) & 0xF; /* MCS 20,21 */
	D3 = (value >> 12) & 0xF; /* MCS 22 */

	/* Bit 11:8 -> MCS 16,17; Bit 7:4 -> MCS 16,17; bit 3:0 -> MCS 16,17 */
	value = (D0 << 8) | (D0 << 4) | D0;	
	/* Bit 27:24 MCS -> 18,19; Bit 23:20 MCS -> MCS 18,19; bit 19:16 -> MCS 18,19 */
	value2 = (D1 << 8) | (D1 << 4) | D1;
	pAd->Tx20MPwrCfgGBand[5] = ((value2 << 16) | value) & 0x0fff0fff;

	/* Bit 11:8 -> MCS 20,21; Bit 7:4 -> MCS 20,21; bit 3:0 -> MCS 20,21 */
	value = (D2 << 8) | (D2 << 4) | D2;
	/* Bit 27:24 -> MCS 22; Bit 23:20 -> MCS 22; bit 19:16 -> MCS 22 */
	value2 = (D3 << 8) | (D3 << 4) | D3;
	pAd->Tx20MPwrCfgGBand[6] = ((value2 << 16) | value) & 0x0fff0fff;

	/* 2.4G @40M, MCS 16,17,18,19,20,21,22 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 5, value); /* EEPROM 159h */		
	D0 = value & 0xF; /* MCS 16,17 */
	D1 = (value >> 4) & 0xF; /* MCS 18,19 */
	D2 = (value >> 8) & 0xF; /* MCS 20,21 */
	D3 = (value >> 12) & 0xF; /* MCS 22 */

	/* Bit 11:8 MCS -> 16,17; Bit 7:4 MCS -> MCS 16,17; bit 3:0 -> MCS 16,17 */
	value = (D0 << 8) | (D0 << 4) | D0; 	
	/* Bit 27:24 MCS -> 18,19; Bit 23:20 MCS -> MCS 18,19; bit 19:16 -> MCS 18,19 */
	value2 = (D1 << 8) | (D1 << 4) |D1;
	pAd->Tx40MPwrCfgGBand[5] = ((value2 << 16) | value) & 0x0fff0fff;
	
	/* Bit 11:8 -> MCS 20,21; Bit 7:4 -> MCS 20,21; bit 3:0 -> MCS 20,21 */
	value = (D2 << 8) | (D2 << 4) | D2;
	/* Bit 27:24 -> MCS 22; Bit 23:20 -> MCS 22; bit 19:16 -> MCS 22 */
	value2 = (D3 << 8) | (D3 << 4) | D3;
	pAd->Tx40MPwrCfgGBand[6] = ((value2 << 16) | value) & 0x0fff0fff;

	/* 5G @20M, MCS 16,17,18,19,20,21,22 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 5, value); /* EEPROM 169h */
	D0 = value & 0xF; /* MCS 16,17 */
	D1 = (value >> 4) & 0xF; /* MCS 18,19 */
	D2 = (value >> 8) & 0xF; /* MCS 20,21 */
	D3 = (value >> 12) & 0xF; /* MCS 22 */

	/* Bit 11:8 MCS -> 16,17; Bit 7:4 MCS -> MCS 16,17; bit 3:0 -> MCS 16,17 */
	value = (D0 << 8) | (D0 << 4) | D0; 	
	/* Bit 27:24 MCS -> 18,19; Bit 23:20 MCS -> MCS 18,19; bit 19:16 -> MCS 18,19 */
	value2 = (D1 << 8) | (D1 << 4) | D1;
	pAd->Tx20MPwrCfgABand[5] = ((value2 << 16) | value) & 0x0fff0fff;

	/* Bit 11:8 -> MCS 20,21; Bit 7:4 -> MCS 20,21; bit 3:0 -> MCS 20,21 */
	value = (D2 << 8) | (D2 << 4) | D2;
	/* Bit 27:24 -> MCS 22; Bit 23:20 -> MCS 22; bit 19:16 -> MCS 22 */
	value2 = (D3 << 8) | (D3 << 4) | D3;
	pAd->Tx20MPwrCfgABand[6] = ((value2 << 16) | value) & 0x0fff0fff;

	/* 5G @40M, MCS 16,17,18,19,20,21,22 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 5, value); /* EEPROM 179h */
	D0 = value & 0xF; /* MCS 16,17 */
	D1 = (value >> 4) & 0xF; /* MCS 18,19 */
	D2 = (value >> 8) & 0xF; /* MCS 20,21 */
	D3 = (value >> 12) & 0xF; /* MCS 22 */

	/* Bit 11:8 MCS -> 16,17; Bit 7:4 MCS -> MCS 16,17; bit 3:0 -> MCS 16,17 */
	value = (D0 << 8) | (D0 << 4) | D0; 	
	/* Bit 27:24 MCS -> 18,19; Bit 23:20 MCS -> MCS 18,19; bit 19:16 -> MCS 18,19 */
	value2 = (D1 << 8) | (D1 << 4) | D1;
	pAd->Tx40MPwrCfgABand[5] = ((value2 << 16) | value) & 0x0fff0fff;

	/* Bit 11:8 -> MCS 20,21; Bit 7:4 -> MCS 20,21; bit 3:0 -> MCS 20,21 */
	value = (D2 << 8) | (D2 << 4) | D2;
	/* Bit 27:24 -> MCS 22; Bit 23:20 -> MCS 22; bit 19:16 -> MCS 22 */
	value2 = (D3 << 8) | (D3 << 4) | D3;
	pAd->Tx40MPwrCfgABand[6] = ((value2 << 16) | value) & 0x0fff0fff;

	/* 2.4G @20M, MCS 7, OFDM 54M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_CCK_OFDM + 3, value); /* EEPROM 143h */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 2, value2); /* EEPROM 146h */		
	D0 = value & 0xF; /* OFDM 54M */
	D1 = (value >> 4) & 0xF; /* Reserved */
	value = (D0 << 8) | (D0 << 4) | D0;
	
	D0 = value2 & 0xF; /* MCS 7 */
	value2 = (D0 << 8) | (D0 << 4) | D0;

	pAd->Tx20MPwrCfgGBand[7] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 2.4G @40M, MCS 7, OFDM 54M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_2_4G + 3, value);
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 2, value2);		
	/* convert TxPwr from RT3883 format to original format for firmware */
	D0 = value & 0xF; 	D1 = (value>>4) & 0xF; D2 = (value>>8) & 0xF; D3 = (value>>12) & 0xF;  
	value = (D0<<8) | (D0<<4) | D0; /* OFDM 54M */
	
	D0 = value2 & 0xF; 	D1 = (value2>>4) & 0xF; D2 = (value2>>8) & 0xF; D3 = (value2>>12) & 0xF;  
	value2 = (D0<<8) | (D0<<4) | D0; /* MCS 7 */
	
	pAd->Tx40MPwrCfgGBand[7] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 5G @20M, MCS 7, OFDM 54M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_OFDM_5G + 3, value);
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 2, value2);		
	/* convert TxPwr from RT3883 format to original format for firmware */
	D0 = value & 0xF; 	D1 = (value>>4) & 0xF; D2 = (value>>8) & 0xF; D3 = (value>>12) & 0xF;  
	value = (D0<<8) | (D0<<4) | D0; /* OFDM 54M */
	
	D0 = value2 & 0xF; 	D1 = (value2>>4) & 0xF; D2 = (value2>>8) & 0xF; D3 = (value2>>12) & 0xF;  
	value2 = (D0<<8) | (D0<<4) | D0; /* MCS 7 */

	pAd->Tx20MPwrCfgABand[7] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 5G @40M, MCS 7 OFDM 54M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_5G + 3, value);
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 2, value2);		
	/* convert TxPwr from RT3883 format to original format for firmware */
	D0 = value & 0xF; 	D1 = (value>>4) & 0xF; D2 = (value>>8) & 0xF; D3 = (value>>12) & 0xF;  
	value = (D0<<8) | (D0<<4) | D0; /* OFDM 54M */
	
	D0 = value2 & 0xF; 	D1 = (value2>>4) & 0xF; D2 = (value2>>8) & 0xF; D3 = (value2>>12) & 0xF;  
	value2 = (D0<<8) | (D0<<4) | D0; /* MCS 7 */

	pAd->Tx40MPwrCfgABand[7] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 2.4G @20M, MCS 15, MCS 23 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4, value); /* EEPROM 148h */		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 6, value2); /* EEPROM 14Ah */		
	D0 = value & 0xF; /* MCS 14 */
	D1 = (value >> 4) & 0xF; /* MCS 15 */
	value = (D1 << 8) | (D1 << 4) | D1;
	
	D2 = (value2 >> 8) & 0xF; /* MCS 23 */
	value2 = (D2 << 8) | (D2 << 4) | D2;

	pAd->Tx20MPwrCfgGBand[8] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 2.4G @40M, MCS 7, OFDM 54M */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 4, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 6, value2);		
	D0 = value & 0xF; /* MCS 14 */
	D1 = (value >> 4) & 0xF; /* MCS 15 */
	value = (D1 << 8) | (D1 << 4) | D1;
	
	D2 = (value2 >> 8) & 0xF; /* MCS 23 */
	value2 = (D2 << 8) | (D2 << 4) | D2;
	
	pAd->Tx40MPwrCfgGBand[8] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 5G @40M, MCS 15, MCS 23 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 4, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 6, value2);		
	D0 = value & 0xF; /* MCS 14 */
	D1 = (value >> 4) & 0xF; /* MCS 15 */
	value = (D1 << 8) | (D1 << 4) | D1;
	
	D2 = (value2 >> 8) & 0xF; /* MCS 23 */
	value2 = (D2 << 8) | (D2 << 4) | D2;

	pAd->Tx20MPwrCfgABand[8] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 5G @40M, MCS 15, MCS 23 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 4, value);		
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 6, value2);		
	D0 = value & 0xF; /* MCS 14 */
	D1 = (value >> 4) & 0xF; /* MCS 15 */
	value = (D1 << 8) | (D1 << 4) | D1;
	
	D2 = (value2 >> 8) & 0xF; /* MCS 23 */
	value2 = (D2 << 8) | (D2 << 4) | D2;

	pAd->Tx40MPwrCfgABand[8] = (((value2 << 16) | value) & 0x0fff0fff);

	/* 2.4G @20M, STBC MCS 7 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 10, value);		
	/* convert TxPwr from RT3883 format to original format for firmware */
	D0 = value & 0xF; 	D1 = (value>>4) & 0xF; D2 = (value>>8) & 0xF; D3 = (value>>12) & 0xF;  

	value = (D0<<8) | (D0<<4) | D0; /* STBC MCS 7 */
	pAd->Tx20MPwrCfgGBand[9] = (value & 0x00000fff);

	/* 2.4G @40M, STBC MCS 7 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 10, value);		
	/* convert TxPwr from RT3883 format to original format for firmware */
	D0 = value & 0xF; 	D1 = (value>>4) & 0xF; D2 = (value>>8) & 0xF; D3 = (value>>12) & 0xF;  

	value = (D0<<8) | (D0<<4) | D0; /* STBC MCS 7 */
	pAd->Tx40MPwrCfgGBand[9] = (value & 0x00000fff);

	/* 5G @20M, STBC MCS 7 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 10, value);		
	/* convert TxPwr from RT3883 format to original format for firmware */
	D0 = value & 0xF; 	D1 = (value>>4) & 0xF; D2 = (value>>8) & 0xF; D3 = (value>>12) & 0xF;  

	value = (D0<<8) | (D0<<4) | D0; /* STBC MCS 7 */
	pAd->Tx20MPwrCfgABand[9] = (value & 0x00000fff);

	/* 5G @40M, STBC MCS 7 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 10, value);		
	/* convert TxPwr from RT3883 format to original format for firmware */
	D0 = value & 0xF; 	D1 = (value>>4) & 0xF; D2 = (value>>8) & 0xF; D3 = (value>>12) & 0xF;  

	value = (D0<<8) | (D0<<4) | D0; /* STBC MCS 7 */
	pAd->Tx40MPwrCfgABand[9] = (value & 0x00000fff);

	for (i = 0; i < MAX_TXPOWER_ARRAY_SIZE; i++) {
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 2.4G band, %d = %lx \n", i, pAd->Tx20MPwrCfgGBand[i]));
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 2.4G band, %d = %lx \n", i, pAd->Tx40MPwrCfgGBand[i]));
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 5G band, %d = %lx \n", i, pAd->Tx20MPwrCfgABand[i]));
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 5G band, %d = %lx \n", i, pAd->Tx40MPwrCfgABand[i]));
	}

	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0, pAd->Tx40MPwrCfgABand[0]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_1, pAd->Tx40MPwrCfgABand[1]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_2, pAd->Tx40MPwrCfgABand[2]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_3, pAd->Tx40MPwrCfgABand[3]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_4, pAd->Tx40MPwrCfgABand[4]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_5, pAd->Tx40MPwrCfgABand[5]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_6, pAd->Tx40MPwrCfgABand[6]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, pAd->Tx40MPwrCfgABand[7]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, pAd->Tx40MPwrCfgABand[8]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, pAd->Tx40MPwrCfgABand[9]);

	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0_EXT, (pAd->Tx40MPwrCfgABand[0] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_1_EXT, (pAd->Tx40MPwrCfgABand[1] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_2_EXT, (pAd->Tx40MPwrCfgABand[2] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_3_EXT, (pAd->Tx40MPwrCfgABand[3] & 0xf0f0f0f0) >> 4);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_4_EXT, (pAd->Tx40MPwrCfgABand[4] & 0xf0f0f0f0) >> 4);
}


/*
	========================================================================
	
	Routine Description:
		Read initial channel power parameters from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID RTMPRT3883ReadChannelPwr(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR				i, choffset;
	EEPROM_TX_PWR_STRUC	    	Power;
	EEPROM_TX_PWR_STRUC	    	Power2;
	EEPROM_TX_PWR_STRUC	    	Power3;

	/*
		Read Tx power value for all channels
			Value from 1 - 0x7f. Default value is 24
			Power value : 2.4G 0x00 (0) ~ 0x1F (31)
						: 5.5G 0x00 (0) ~ 0x1F (31)
*/
	
	/* 0. 11b/g, ch1 - ch 14 */
	for (i = 0; i < 7; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX3_PWR_OFFSET + i * 2, Power3.word);
		pAd->TxPower[i * 2].Channel = i * 2 + 1;
		pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

		if ((Power.field.Byte0 > 31) || (Power.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power = Power.field.Byte0;

		if ((Power.field.Byte1 > 31) || (Power.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;

		if ((Power2.field.Byte0 > 31) || (Power2.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 > 31) || (Power2.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;

		if ((Power3.field.Byte0 > 31) || (Power3.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power3 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 > 31) || (Power3.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power3 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power3 = Power3.field.Byte1;
	}
	
	/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz) */
	/* 1.1 Fill up channel */
	choffset = 14;
	for (i = 0; i < 4; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 36 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 36 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 36 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
	}

	/* 1.2 Fill up power */
	for (i = 0; i < 6; i++)
	{
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word); */
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word); */
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX3_PWR_OFFSET + i * 2, Power3.word);

		if ((Power.field.Byte0 <= 31) && (Power.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 <= 31) && (Power.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 <= 31) && (Power2.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 <= 31) && (Power2.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

		if ((Power3.field.Byte0 <= 31) && (Power3.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 <= 31) && (Power3.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;			
	}
	
	/* 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz) */
	/* 2.1 Fill up channel */
	choffset = 14 + 12;
	for (i = 0; i < 5; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 100 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 100 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 100 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 5 + choffset + 0].Channel		= 140;
	pAd->TxPower[3 * 5 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 5 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 5 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;

	/* 2.2 Fill up power */
	for (i = 0; i < 8; i++)
	{
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word); */
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word); */
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX3_PWR_OFFSET + (choffset - 14) + i * 2, Power3.word);

		if ((Power.field.Byte0 <= 31) && (Power.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 <= 31) && (Power.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 <= 31) && (Power2.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 <= 31) && (Power2.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

		if ((Power3.field.Byte0 <= 31) && (Power3.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 <= 31) && (Power3.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;			
	}

	/* 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165 (including central frequency in BW 40MHz) */
	/* 3.1 Fill up channel */
	choffset = 14 + 12 + 16;
	for (i = 0; i < 2; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 149 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 149 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 149 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 2 + choffset + 0].Channel		= 165;
	pAd->TxPower[3 * 2 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 2 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 2 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;

	/* 3.2 Fill up power */
	for (i = 0; i < 4; i++)
	{
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word); */
/*		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word); */
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX3_PWR_OFFSET + (choffset - 14) + i * 2, Power3.word);

		if ((Power.field.Byte0 <= 31) && (Power.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 <= 31) && (Power.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

		if ((Power2.field.Byte0 <= 31) && (Power2.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 <= 31) && (Power2.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

		if ((Power3.field.Byte0 <= 31) && (Power3.field.Byte0 >= 0))
			pAd->TxPower[i * 2 + choffset + 0].Power3 = Power3.field.Byte0;

		if ((Power3.field.Byte1 <= 31) && (Power3.field.Byte1 >= 0))
			pAd->TxPower[i * 2 + choffset + 1].Power3 = Power3.field.Byte1;			
	}

	/* 4. Print and Debug */
	choffset = 14 + 12 + 16 + 7;
}


/* ATE will also call this function to set GPIO, channel=36 to set low and channel=1 to set high */
VOID	RTMPRT3883ABandSel(
	IN	UCHAR	Channel)
{
#ifdef BOARD_EXT_SWITCH_LNA_2DOT4_5
	UINT32 value;

	/*set GPIO2(gpio#25) to GPIO mode */
	value = le32_to_cpu(*(volatile u32 *)(0xb0000014));
	value &= ~(1 << 2);
	*((volatile uint32_t *)(0xb0000014)) = cpu_to_le32(value);

	/*config gpio#25 direction to output */
	value = le32_to_cpu(*(volatile u32 *)(0xb000064c));
	value |= (1 << 1);
	*((volatile uint32_t *)(0xb000064c)) = cpu_to_le32(value);

	if (Channel > 14)
	{
		/*5G band: clear gpio#25 to 0 */
		*((volatile uint32_t *)(0xb0000658)) = cpu_to_le32(0x2);
	}
	else
	{
		/*2.4G band: set gpio#25 to 1 */
		*((volatile uint32_t *)(0xb0000654)) = cpu_to_le32(0x2);
	}
#endif
}

static VOID RT3883_ChipSwitchChannel(
	IN PRTMP_ADAPTER 			pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan) 
{
	int index, i;
	CHAR TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER, TxPwer3 = 0, lanGain;
	UINT32 mac_val;
	UINT32 TxPinCfg;
	UINT8 RFValue = 0, BbpValue = 0;;

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
	    		TxPwer3 = pAd->TxPower[index].Power3;
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Can't find the Channel#%d \n", __FUNCTION__, Channel));
	}

		RTMPRT3883ABandSel(Channel);
		for (index = 0; index < NUM_OF_3883_CHNL; index++)
		{
			if (Channel == FreqItems3883[index].Channel)
			{
			/* RF_R06 for A-Band L:0x80 M:0x80 H:0x40 (Gary, 2010-06-02) */
			if (pAd->CommonCfg.Channel <= 14)
				RFValue = 0x40;
			else
			{
				if (pAd->CommonCfg.Channel <132)
					RFValue = 0x80;
				else
					RFValue = 0x40;
			}
			RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RFValue);

			/* Programming channel parameters*/
			RT30xxWriteRFRegister(pAd, RF_R08, FreqItems3883[index].N);
			RT30xxWriteRFRegister(pAd, RF_R09, FreqItems3883[index].K);

			RFValue =  (Channel <= 14) ? 0x46 : 0x48;
			RT30xxWriteRFRegister(pAd, RF_R11, (UCHAR)RFValue);

			/* Gary, 2011-03-10 */
			RFValue = (Channel <= 14) ? 0x1a : 0x52;
			RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)RFValue);

			RT30xxWriteRFRegister(pAd, RF_R13, 0x12);

			/* Tx/Rx Stream setting*/
			RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&RFValue);
			RFValue &= 0x03; /*clear bit[7~2]*/
			RFValue |= 0xFC; /* default 3Tx 3Rx*/
			if (pAd->Antenna.field.TxPath == 1)
				RFValue &= ~(0x5 << 5);
			else if (pAd->Antenna.field.TxPath == 2)
				RFValue &= ~(0x1 << 7);
			else if (pAd->Antenna.field.TxPath == 3) 	/*wayne_note: 090826 need to consider TxPath=3, for 3883 case*/
				RFValue &= ~(0x0 << 7);		

			if (pAd->Antenna.field.RxPath == 1)
				RFValue &= ~(0x5 << 4);
			else if (pAd->Antenna.field.RxPath == 2)
				RFValue &= ~(0x1 << 6);
			else if (pAd->Antenna.field.RxPath == 3)	/*wayne_note: 090826 need to consider TxPath=3, for 3883 case*/
				RFValue &= ~(0x0 << 6);	
			RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);

			RT3883_AsicSetFreqOffset(pAd, pAd->RfFreqOffset);

			/* Different default setting for A/BG bands*/
			RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RFValue);
			if (pAd->CommonCfg.BBPCurrentBW == BW_20)
				RFValue &= ~(0x06); /* 20MBW Bit[2:1]=0,0*/
			else
				RFValue |= 0x06;
			RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RFValue);

			RFValue =  (Channel <= 14) ?  0xA0 : 0x80;
			RT30xxWriteRFRegister(pAd, RF_R31, (UCHAR)RFValue);

			RFValue = ((pAd->CommonCfg.BBPCurrentBW == BW_40) ? 0x80 : 0xd8);
			RT30xxWriteRFRegister(pAd, RF_R32, RFValue);
			
			RFValue = (Channel <= 14) ? 0x3C : 0x20;
			RT30xxWriteRFRegister(pAd, RF_R34, (UCHAR)RFValue);

			/* loopback RF_BS*/
			RT30xxReadRFRegister(pAd, RF_R36, (PUCHAR)&RFValue);
			RFValue &= ~(0x1 << 7);
			if  (Channel <= 14)
				RT30xxWriteRFRegister(pAd, RF_R36, (UCHAR)(RFValue  | (0x1 << 7)));
			else
				RT30xxWriteRFRegister(pAd, RF_R36, (UCHAR)RFValue);

			/* RF_R39 for A-Band L:0x36 M:0x32 H:0x30 (Gary, 2010-02-12)*/
			if (pAd->CommonCfg.Channel <= 14)
				RFValue = 0x23;
			else
			{
				if (pAd->CommonCfg.Channel < 100)
					RFValue = 0x36;
				else if (pAd->CommonCfg.Channel < 132)
					RFValue = 0x32;
				else
					RFValue = 0x30;
				/* RT30xxWriteRFRegister(pAd, RF_R39, (UCHAR)RFValue); */
			}
#ifdef TXBF_SUPPORT
			if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn || pAd->CommonCfg.ETxBfEnCond)
				RFValue |= 0x40;
#endif /* TXBF_SUPPORT */
			RT30xxWriteRFRegister(pAd, RF_R39, (UCHAR)RFValue);

			/* loopback RF_BS*/
			RFValue = (Channel <= 14) ? 0x93 : 0x9b; /* Gary, 2010-02-12*/
			RT30xxWriteRFRegister(pAd, RF_R44, (UCHAR)RFValue);

			/* RF_R45 for A-Band L:0xEB M:0xB3 H:0x9B (Gary, 2010-02-12)*/
			if (pAd->CommonCfg.Channel <= 14)
				RFValue = 0xBB;
			else
			{
				if (pAd->CommonCfg.Channel <100)
					RFValue = 0xEB;
				else if(pAd->CommonCfg.Channel <132)
					RFValue = 0xB3;
				else
					RFValue = 0x9B;
			}
			RT30xxWriteRFRegister(pAd, RF_R45, (UCHAR)RFValue);

			RFValue = (Channel <= 14) ? 0x8e : 0x8a;
#ifdef TXBF_SUPPORT
			if ((pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn) || (pAd->CommonCfg.ETxBfEnCond))
				RFValue |= 0x20;
#endif /* TXBF_SUPPORT */
			RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)RFValue);

			RT30xxWriteRFRegister(pAd, RF_R50, 0x86);
	
			/* tx_mx1_ic*/
			RT30xxReadRFRegister(pAd, RF_R51, (PUCHAR)&RFValue);
			RFValue = (Channel <= 14) ? 0x75 : 0x51;
			RT30xxWriteRFRegister(pAd, RF_R51, (UCHAR)RFValue);

			RT30xxReadRFRegister(pAd, RF_R52, (PUCHAR)&RFValue);
			RFValue = (Channel <= 14) ? 0x45 : 0x05;
			RT30xxWriteRFRegister(pAd, RF_R52, (UCHAR)RFValue);
			
			for (i = 0; i < MAX_NUM_OF_CHANNELS; i++)
			{
				CHAR power[3], ant_idx, delta;
				
				if (Channel != pAd->TxPower[i].Channel)
					continue;

				power[0]= pAd->TxPower[i].Power;
				power[1]= pAd->TxPower[i].Power2;
				power[2]= pAd->TxPower[i].Power3;
				
				if ((pAd->NicConfig2.field.DynamicTxAgcControl == 1) &&
					(power[0] <= 0x1b) && (power[1] <= 0x1b) && (power[2] <= 0x1b))
				{
					pAd->bTxPwrRangeExt = TRUE;
					delta = 4;
				}
				else
				{
					pAd->bTxPwrRangeExt = FALSE;
					delta = 0;
				}

				for ( ant_idx = 0; ant_idx < 3; ant_idx++) {
					power[ant_idx] += delta;
					if (Channel > 14)
						power[ant_idx] = TX_PWR_TO_RF_REG(power[ant_idx]);
				}

				RT30xxWriteRFRegister(pAd, RF_R53, power[0]);
				RT30xxWriteRFRegister(pAd, RF_R54, power[1]);
				RT30xxWriteRFRegister(pAd, RF_R55, power[2]);

				/* tx0, tx1 (0.1db)*/
				BbpValue = (pAd->TxPower[index].Power >> 5) | ((pAd->TxPower[index].Power2 & 0xE0) >> 1);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R109, BbpValue);

				/* tx2 (0.1db)*/
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R110, &BbpValue);
				BbpValue = ((pAd->TxPower[index].Power3 & 0xE0) >> 1) | (BbpValue & 0x0F);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R110, BbpValue);

				break;
			}

			RT30xxReadRFRegister(pAd, RF_R57, (PUCHAR)&RFValue);
			RFValue = (Channel <= 14) ? 0x6E : 0x3E;
			RT30xxWriteRFRegister(pAd, RF_R57, (UCHAR)RFValue);

			/* Enable RF tuning, this must be in the last, RF_R03=RF_R07 (RT30xx)*/
			RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x80; /* bit 7=vcocal_en*/
			RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);

			RTMPusecDelay(2000);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, RFValue & 0xfe); /* clear update flag*/
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);

			/* latch channel for future usage.*/
			pAd->LatchRfRegs.Channel = Channel;

#ifdef TXBF_SUPPORT
			/* Do a Divider Calibration and update BBP registers */
			if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn 
#ifdef DBG_CTRL_SUPPORT
			   && (pAd->CommonCfg.DebugFlags & DBF_DISABLE_CAL)==0
#endif /* DBG_CTRL_SUPPORT */
			)
			{
				ITxBFLoadLNAComp(pAd);
				ITxBFDividerCalibration(pAd, 2, 0, NULL);
			}
#endif /* TXBF_SUPPORT */
		
			DBGPRINT(RT_DEBUG_TRACE, ("RT3883: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
				Channel, pAd->RfIcType, 
				TxPwer, TxPwer2,
				pAd->Antenna.field.TxPath,
				FreqItems3883[index].N, 
				FreqItems3883[index].K, 
				FreqItems3883[index].R));
			break;
		}
	}

	/* Change BBP setting during siwtch from a->g, g->a*/
	lanGain = GET_LNA_GAIN(pAd);
	if (Channel <= 14)
	{
		BbpValue = 0x37 - lanGain;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, BbpValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, BbpValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, BbpValue);

		BbpValue =  (pAd->CommonCfg.RxStream > 1) ? 0x46 : 0;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, BbpValue);

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

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x8A);

		/* 5G band selection PIN, bit1 and bit2 are complement*/
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &mac_val);
		mac_val &= (~0x6);
		mac_val |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, mac_val);

		/* Turn off unused PA or LNA when only 1T/1R, 2T/2R */
		TxPinCfg = 0x32050F0A;
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg  &= (~0x0300000D);
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg  &= (~0x03000005);
		
		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg &= (~0x30000C00);
		else if (pAd->Antenna.field.RxPath == 2)
			TxPinCfg &= (~0x30000000);
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - lanGain));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - lanGain));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - lanGain));

		/* Henry 2009-12-16 */
		BbpValue = (pAd->CommonCfg.RxStream > 1) ? 0x46 : 0;		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86,BbpValue);

		/* Set the BBP_R82/BBP_R83 value here */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x82);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x9A);

		/* Rx High power VGA offset for LNA select*/
		BbpValue = pAd->NicConfig2.field.ExternalLNAForA ? 0x46 : 0x50;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, BbpValue);

		/* 5G band selection PIN, bit1 and bit2 are complement*/
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &mac_val);
		mac_val &= (~0x6);
		mac_val |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, mac_val);

		/* Turn off unused PA or LNA when only 1T/1R, 2T/2R */
		TxPinCfg = 0x31050F05;
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg &= (~0x0300000E);
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg &= (~0x0300000A);
		
		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg &= (~0x30000C00);
		else if (pAd->Antenna.field.RxPath == 2)
			TxPinCfg &= (~0x30000000);
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}

	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x34);
	else
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x04);
		
	/* R66 should be set according to Channel and use 20MHz when scanning*/
	/*AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x2E + lanGain), RX_CHAIN_ALL);*/
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	
	/* On 11A, We should delay and wait RF/BBP to be stable*/
	/* and the appropriate time should be 1000 micro seconds */
	/* 2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.*/
	RTMPusecDelay(1000);
}


VOID RT3883_CWC_AsicSet(
	IN RTMP_ADAPTER *pAd, 
	IN BOOLEAN bCwc)
{
	UINT32 MacReg, rty_val, xifs_val;

	if (bCwc == 0)
	{ /* CwC Off */
		rty_val = 0x1f0f;
		xifs_val = 0x1010;
	}
	else if (bCwc == 1)
	{
		/* CwC On */
		rty_val = 0x3f3f;
		xifs_val = 0x0e0e;
	}
	else
		return;

	RTMP_IO_READ32(pAd, TX_RTY_CFG, &MacReg);
	MacReg &= 0xffff0000;
	MacReg |= rty_val;
	RTMP_IO_WRITE32(pAd, TX_RTY_CFG, MacReg);

	RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &MacReg);
	MacReg &= 0xffff0000;
	MacReg |= xifs_val;
	RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, MacReg);

}


VOID RT3883_CWC_ProtectAdjust(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pSetMask,
	IN USHORT *pOperationMode)
{
	int macIdx;
	BOOLEAN bNoRTS = FALSE;
	
		
	if (pAd->FlgCWC == 1) /* enable cwc -> disable RTS/CTS */
		return;
	
	if ((pAd->FlgCWC == 2) && 
		(pAd->MacTab.Size == 1) && 
		((pAd->MacTab.fAnyStationIsLegacy == FALSE) && (pAd->MacTab.fAnyStationBadAtheros == FALSE))
	)
	{
		/* 
			Depends on Gary's request, 
				1. only one 11n STA(non-A STA) is connected
				2. MCS is 22 ~ 23
			just let it go!
		*/		
		for (macIdx = 0; macIdx < MAX_LEN_OF_MAC_TABLE; macIdx++)
		{
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[macIdx];
			
			if (!IS_ENTRY_CLIENT(pEntry))
				continue;

			/* Well, I think the first one will be our target, because we only have one valid station connect to us */
			if (pEntry->HTPhyMode.field.MCS >= 22 && pEntry->HTPhyMode.field.MCS <= 23)
				bNoRTS = TRUE;
			
			/* DBGPRINT(RT_DEBUG_WARN, ("MCS = %d\n", pEntry->HTPhyMode.field.MCS)); */
			break;
		}

		if ((bNoRTS == FALSE) && (pAd->BATable.numDoneOriginator))
		{
			/* We may need to go original case here if the NoRTS is FALSE */
			*pSetMask |= ALLN_SETPROTECT;
			*pOperationMode = 8;
			RT3883_CWC_AsicSet(pAd, 0);
			/* DBGPRINT(RT_DEBUG_WARN, ("RTS/CTS protection ON\n")); */
		}		
		else
		{
			RT3883_CWC_AsicSet(pAd, 1);
			/* DBGPRINT(RT_DEBUG_WARN, ("RTS/CTS protection OFF\n")); */
		}
	}
}


#ifdef CONFIG_AP_SUPPORT
int RT3883_ext_pkt_len(
	IN UCHAR *pOutBuffer,
	IN ULONG FrameLen,
	IN UCHAR *RalinkSpecificIe,
	IN UCHAR IeLen)
{
	ULONG totalLen, appendLen;
	
	totalLen = FrameLen;
	while(totalLen < 150)
	{
		appendLen = 0;
		MakeOutgoingFrame(pOutBuffer+totalLen, &appendLen,
							IeLen, RalinkSpecificIe,
							END_OF_ARGS);
		totalLen += appendLen;
	}

	return totalLen - FrameLen;
}
#endif /* CONFIG_AP_SUPPORT */


/*
========================================================================
Routine Description:
	Initialize RT3883.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT3883_Init(
	IN PRTMP_ADAPTER		pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	/* 
		Init chip capabilities
	*/
	pChipCap->SnrFormula = SNR_FORMULA2;
	pChipCap->FlgIsHwWapiSup = TRUE;
	pChipCap->VcoPeriod = 10;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_2;
	pChipCap->FlgIsHwAntennaDiversitySup = FALSE;
#ifdef STREAM_MODE_SUPPORT
	pChipCap->FlgHwStreamMode = TRUE;
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	pChipCap->FlgHwTxBfCap = TRUE;
#endif /* TXBF_SUPPORT */
#ifdef FIFO_EXT_SUPPORT
	pChipCap->FlgHwFifoExtCap = TRUE;
#endif /* FIFO_EXT_SUPPORT */

	pChipCap->RfReg17WtMethod= RF_REG_WT_METHOD_STEP_ON;
		
	pChipCap->MaxNumOfRfId = 63;
	pChipCap->pRFRegTable = RT3883_RFRegTable;

	pChipCap->MaxNumOfBbpId = 200;	
	pChipCap->pBBPRegTable = RT3883_BBPRegTable;
	pChipCap->bbpRegTbSize = RT3883_NUM_BBP_REG_PARMS;

	pChipCap->MaxNss = 3;
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 20;
#ifdef RTMP_FLASH_SUPPORT
	pChipCap->eebuf = RT3883_EeBuffer;
#endif /* RTMP_FLASH_SUPPORT */

#ifdef NEW_MBSSID_MODE
	pChipCap->MBSSIDMode = MBSSID_MODE1;
#else
	pChipCap->MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */


	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_GRP);
		
	/*
		Following function configure beacon related parameters
		in pChipCap
			FlgIsSupSpecBcnBuf / BcnMaxHwNum / 
			WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	*/
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RtmpChipBcnSpecInit(pAd);
#else
	RtmpChipBcnInit(pAd);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */

	/*
		init operator
	*/
	
	/* BBP adjust */
	pChipOps->ChipBBPAdjust = RT3883_ChipBBPAdjust;
#ifdef CONFIG_STA_SUPPORT
	pChipOps->ChipAGCAdjust = RT3883_ChipAGCAdjust;
#endif /* CONFIG_STA_SUPPORT */

	/* Channel */
	pChipOps->ChipSwitchChannel = RT3883_ChipSwitchChannel;
	pChipOps->ChipAGCInit = RT3883_ChipAGCInit;
	pChipOps->AsicAdjustTxPower = AsicAdjustTxPower;

	pChipOps->AsicMacInit = NICInitRT3883MacRegisters;
	pChipOps->AsicBbpInit = NICInitRT3883BbpRegisters;
	pChipOps->AsicRfInit = NICInitRT3883RFRegisters;
	pChipOps->AsicRfTurnOn = NULL;

	pChipOps->AsicHaltAction = RT3883HaltAction;
	pChipOps->AsicRfTurnOff = RT3883LoadRFSleepModeSetup;
	pChipOps->AsicReverseRfFromSleepMode = RT3883ReverseRFSleepModeSetup;
	pChipOps->AsicResetBbpAgent = NULL;
	
	/* MAC */

	/* EEPROM */
	pChipOps->NICInitAsicFromEEPROM = NULL;
	
	/* Antenna */
	pChipOps->AsicAntennaDefaultReset = RT3883_AsicAntennaDefaultReset;

	/* Frequence Calibration */

	/* TX ALC */
	pChipOps->InitDesiredTSSITable = NULL;
 	pChipOps->ATETssiCalibration = NULL;
	pChipOps->ATETssiCalibrationExtend = NULL;
	pChipOps->AsicTxAlcGetAutoAgcOffset = NULL;
	pChipOps->ATEReadExternalTSSI = NULL;
	pChipOps->TSSIRatio = NULL;
	
	/* Others */
#ifdef CONFIG_STA_SUPPORT
	pChipOps->NetDevNickNameInit = RT3883_NetDevNickNameInit;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	pAd->chipCap.carrier_func = TONE_RADAR_V2;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v2;
#endif /* CARRIER_DETECTION_SUPPORT */

	/* Chip tuning */
	pChipOps->RxSensitivityTuning = RT3883_RxSensitivityTuning;
	pChipOps->AsicTxAlcGetAutoAgcOffset = AsicGetAutoAgcOffsetForExternalTxAlc;
	pChipOps->AsicGetTxPowerOffset = RT3883_AsicGetTxPowerOffset;

	
/* Following callback functions already initiailized in RtmpChipOpsEepromHook( ) */
	/*  Calibration access related callback functions */
/*
	int (*eeinit)(struct _RTMP_ADAPTER *pAd);
	int (*eeread)(struct _RTMP_ADAPTER *pAd, USHORT offset, PUSHORT pValue);
	int (*eewrite)(struct _RTMP_ADAPTER *pAd, USHORT offset, USHORT value);
*/
	/* MCU related callback functions */
/*
	int (*loadFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*eraseFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*sendCommandToMcu)(struct _RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1, BOOLEAN FlgIsNeedLocked);
*/

/* 
	Following callback functions already initiailized in RtmpChipOpsHook() 
	1. Power save related
*/
#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = EnableAPMIMOPSv2;
	pChipOps->DisableAPMIMOPS = DisableAPMIMOPSv2;
#endif /* GREENAP_SUPPORT */
}


#endif /* RT3883 */

