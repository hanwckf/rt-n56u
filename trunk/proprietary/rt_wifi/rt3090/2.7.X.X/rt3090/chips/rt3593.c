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
	rt3593.c

	Abstract:
	Specific funcitons and variables for RT3593.

	Revision History:
	Who         When			What
	--------    ----------		--------------------------------------------
	Sample Lin	2009/10/29		For RT3593 Initial
	Sample Lin	2009/11/09		Move PostBBPInitialization() here
								Also can use pBBPRegTable, but I think use
								BBPR105.field.xx is easy to compare codes.
*/


#ifdef RT3593

#include "rt_config.h"

UCHAR RT3593_EeBuffer[EEPROM_SIZE] = {};

/* RF register initialization set*/

REG_PAIR RF3053RegTable[] = {
	{RF_R01,		0x03}, 
	{RF_R02,		0x80},
	{RF_R03,		0x80},
	{RF_R05,		0x00}, 
	{RF_R06,		0x40}, 
	{RF_R08,		0xF1}, 
	{RF_R09,		0x02},
	{RF_R10,		0xD3},
	{RF_R11,		0x40},
	{RF_R12,		0x4E},
	{RF_R13,		0x12},
	{RF_R18,		0x40}, 
	{RF_R22,		0x20},	
	{RF_R30,		0x10},
	{RF_R31,		0x80}, 
	{RF_R32,		0x78}, 
	{RF_R33,		0x3B}, 
	{RF_R34,		0x3C}, 
	{RF_R35,		0xE0}, 
	{RF_R38,		0x86}, 
	{RF_R39,		0x23}, 
	{RF_R44,		0xD3},
	{RF_R45,		0xBB}, 
	{RF_R46,		0x60}, 
	{RF_R49,		0x8E},
	{RF_R50,		0x86},
	{RF_R51,		0x75},
	{RF_R52,		0x45},
	{RF_R53,		0x18},
	{RF_R54,		0x18},
	{RF_R55,		0x18},
	{RF_R56,		0xDB}, 
	{RF_R57,		0x6E},
};

UCHAR NUM_RF_3053_REG_PARMS = (sizeof(RF3053RegTable) / sizeof(REG_PAIR));

RTMP_REG_PAIR	RT3593_MACRegTable[] =	{
	{TX_SW_CFG0,		0x402},   /* Gary,2010-07-20*/
	{HT_FBK_CFG1,		0xedcba980},	/* Fallback MCS8->MCS0 */
};

UCHAR RT3593_NUM_MAC_REG_PARMS = (sizeof(RT3593_MACRegTable) / sizeof(RTMP_REG_PAIR));

REG_PAIR   RT3593_BBPRegTable[] = {
	{BBP_R79,		0x13},
	{BBP_R80,		0x05},
	{BBP_R81,		0x33},
	{BBP_R103,		0xC0}, 
	{BBP_R137,		0x0F},  /* Julian suggest make the RF output more stable */
	{BBP_R163,		0xBC},	/* Enable saving of Explicit and Implicit profiles */

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
};

UCHAR RT3593_NUM_BBP_REG_PARMS = (sizeof(RT3593_BBPRegTable) / sizeof(REG_PAIR));

/* The frequency plan of the RT3053*/

FREQUENCY_ITEM FreqItems3053[] =
{
	
	/* 2.4 GHz*/
	

	/*	Channel	N		R		K*/
	{	1,		241,		2,		2}, 
	{	2,		241,		2,		7}, 
	{	3,		242,		2,		2}, 
	{	4,		242,		2,		7}, 
	{	5,		243,		2,		2}, 
	{	6,		243,		2,		7}, 
	{	7,		244,		2,		2}, 
	{	8,		244,		2,		7}, 
	{	9,		245,		2,		2}, 
	{	10,		245,		2,		7}, 
	{	11,		246,		2,		2}, 
	{	12,		246,		2,		7}, 
	{	13,		247,		2,		2}, 
	{	14,		248,		2,		4}, 

	
	/* 5 GHz*/
	

	/*	Channel	N		R		K*/
	{	36,		0x56,	0,		4}, 
	{	38,		0x56,	0,		6}, 
	{	40,		0x56,	0,		8}, 
	{	44,		0x57,	0,		0}, 
	{	46,		0x57,	0,		2}, 
	{	48,		0x57,	0,		4}, 
	{	52,		0x57,	0,		8}, 
	{	54,		0x57,	0,		10}, 
	{	56,		0x58,	0,		0}, 
	{	60,		0x58,	0,		4}, 
	{	62,		0x58,	0,		6}, 
	{	64,		0x58,	0,		8}, 

	{	100,		0x5B,	0,		8}, 
	{	102,		0x5B,	0,		10}, 
	{	104,		0x5C,	0,		0}, 
	{	108,		0x5C,	0,		4}, 
	{	110,		0x5C,	0,		6}, 
	{	112,		0x5C,	0,		8}, 
/*	{	114,		0x5C,	0,		10}, */
	{	116,		0x5D,	0,		0}, 
	{	118,		0x5D,	0,		2}, 
	{	120,		0x5D,	0,		4}, 
	{	124,		0x5D,	0,		8}, 
	{	126,		0x5D,	0,		10}, 
	{	128,		0x5E,	0,		0}, 
	{	132,		0x5E,	0,		4}, 
	{	134,		0x5E,	0,		6}, 
	{	136,		0x5E,	0,		8}, 
	{	140,		0x5F,	0,		0}, 

	{	149,		0x5F,	0,		9}, 
	{	151,		0x5F,	0,		11}, 
	{	153,		0x60,	0,		1}, 
	{	157,		0x60,	0,		5}, 
	{	159,		0x60,	0,		7}, 
	{	161,		0x60,	0,		9}, 
	{	165,		0x61,	0,		1}, 
	{	167,		0x61,	0,		3}, 
	{	169,		0x61,	0,		5}, 
	{	171,		0x61,	0,		7}, 
	{	173,		0x61,	0,		9}, 
};

UCHAR NUM_OF_3053_CHNL = (sizeof(FreqItems3053) / sizeof(FREQUENCY_ITEM));

VOID RT3593LoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR	rfreg;

	RT30xxReadRFRegister(pAd, RF_R01, &rfreg);
	/* vco_en*/
	rfreg = ((rfreg & ~0x01) | 0x00);
	RT30xxWriteRFRegister(pAd, RF_R01, rfreg);

	RT30xxReadRFRegister(pAd, RF_R06, &rfreg);
	/* vco_ic (VCO bias current control, 00: off)*/
	rfreg &= (~0xC0);
	RT30xxWriteRFRegister(pAd, RF_R06, rfreg);

	RT30xxReadRFRegister(pAd, RF_R22, &rfreg);
	/* cp_ic (reference current control, 000: 0.25 mA)*/
	rfreg = ((rfreg & ~0xE0) | 0x00);
	RT30xxWriteRFRegister(pAd, RF_R22, rfreg);

	RT30xxReadRFRegister(pAd, RF_R46, &rfreg);
	rfreg &= (~0x20); /* rx_ctb_en*/
	RT30xxWriteRFRegister(pAd, RF_R46, rfreg);
}


VOID RT3593ReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN FlgIsInitState)
{

	UCHAR	rfreg;

	RT30xxReadRFRegister(pAd, RF_R01, &rfreg);
	rfreg = ((rfreg & ~0x01) | 0x01); /* vco_en*/
	RT30xxWriteRFRegister(pAd, RF_R01, rfreg);

	RT30xxReadRFRegister(pAd, RF_R03, &rfreg);
	rfreg |= 0x80; /* vcocal_en (initiate VCO calibration (reset after completion))*/
	RT30xxWriteRFRegister(pAd, RF_R03, rfreg);

	/* 2010/09/20 */
	RT30xxReadRFRegister(pAd, RF_R06, &rfreg);
	rfreg = ((rfreg & ~0xC0) | 0x80); /* vco_ic (VCO bias current control, 10: mid.)*/
	RT30xxWriteRFRegister(pAd, RF_R06, rfreg);

	RT30xxReadRFRegister(pAd, RF_R02, &rfreg);
	rfreg |= 0x80; /* rescal_en (initiate calibration)*/
	RT30xxWriteRFRegister(pAd, RF_R02, rfreg);

	RT30xxReadRFRegister(pAd, RF_R22, &rfreg);
	rfreg = ((rfreg & ~0xE0) | 0x20); /* cp_ic (reference current control, 001: 0.33 mA)*/
	RT30xxWriteRFRegister(pAd, RF_R22, rfreg);

	RT30xxReadRFRegister(pAd, RF_R46, &rfreg);
	rfreg |= 0x20; /* rx_ctb_en*/
	RT30xxWriteRFRegister(pAd, RF_R46, rfreg);

	RT30xxReadRFRegister(pAd, RF_R20, &rfreg);
	rfreg &= (~0xEE); /* ldo_rf_vc and ldo_pll_vc ( 111: +0.15)*/
	RT30xxWriteRFRegister(pAd, RF_R20, rfreg);
}


VOID RT3593HaltAction(
	IN PRTMP_ADAPTER pAd)
{

	UINT32		TxPinCfg = 0x00050F0F;

	/* Fix suspend leakage current
 	 * According to MAC 0x0580 bit[31]. set MAC 0x1328 bit[19]
 	 * during suspend mode.
 	 * If SEL_EFUSE = 0, set TRSW_POL = 0 in suspend mode.
 	 * If SEL_EFUSE = 1, set TRSW_POL = 1 in suspend mode.
	 * Turn off LNA_PE or TRSW_POL
	 */
#ifdef RTMP_EFUSE_SUPPORT
	if(pAd->bUseEfuse)
	{
		TxPinCfg &= 0xFFFBF0F0; /* bit18 off */
	}
	else
#endif /* RTMP_EFUSE_SUPPORT */
	{
		TxPinCfg &= 0xFFFFF0F0;
	}

	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
}


/*
========================================================================
Routine Description:
	Initialize specific MAC registers for RT3593.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT3593MacRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT32 IdReg;

	for(IdReg=0; IdReg<RT3593_NUM_MAC_REG_PARMS; IdReg++)
	{
		RTMP_IO_WRITE32(pAd, RT3593_MACRegTable[IdReg].Register,
								RT3593_MACRegTable[IdReg].Value);
	}

	RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);

	/* RT3071 version E has fixed this issue */
	if ((pAd->MACVersion & 0xffff) < 0x0211)
	{
		if (pAd->NicConfig2.field.DACTestBit == 1)
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x1F);	/* To fix throughput drop drastically */
		}
		else
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0F);	/* To fix throughput drop drastically */
		}
	}
	else
		RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0);
}


/*
========================================================================
Routine Description:
	Initialize specific BBP registers for RT3593.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID NICInitRT3593BbpRegisters(
	IN	PRTMP_ADAPTER					pAd)
{
	UINT32 IdReg;

	for(IdReg=0; IdReg<RT3593_NUM_BBP_REG_PARMS; IdReg++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, RT3593_BBPRegTable[IdReg].Register,
								RT3593_BBPRegTable[IdReg].Value);
	}

}


/*
	==========================================================================
	Description:

	Load RF normal operation-mode setup

	==========================================================================
 */
static VOID RT3593LoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RfReg;

	/* TX_LO2_en */
	RT30xxReadRFRegister(pAd, RF_R50, &RfReg);
	RfReg = ((RfReg & ~0x10) | 0x00); /* tx_lo2_en (both bands, 0: LO2 follows TR switch) */
	RT30xxWriteRFRegister(pAd, RF_R50, RfReg);

	/* TX_LO1_en, RX_MX2_GC*/
	RT30xxReadRFRegister(pAd, RF_R51, &RfReg);
	RfReg = ((RfReg & ~0x1C) | ((pAd->TxMixerGain24G & 0x07) << 2)); /* tx_mx1_cc (RF mixer output tank tuning, both bands) */
	RT30xxWriteRFRegister(pAd, RF_R51, RfReg);

	/* RX_LO1_en */
	RT30xxReadRFRegister(pAd, RF_R38, &RfReg);
	RfReg = ((RfReg & ~0x20) | 0x00); /* rx_lo1_en (enable RX LO1, 0: LO1 follows TR switch) */
	RT30xxWriteRFRegister(pAd, RF_R38, RfReg);

	/* RX_LO2_en */
	RT30xxReadRFRegister(pAd, RF_R39, &RfReg);
	RfReg = ((RfReg & ~0x80) | 0x00); /* rx_lo2_en (enable RX LO2, 0: LO2 follows TR switch) */
	RT30xxWriteRFRegister(pAd, RF_R39, RfReg);

	RT30xxReadRFRegister(pAd, RF_R01, &RfReg);
	RfReg = ((RfReg & ~0x03) | 0x03); /* rf_block_en and pll_en */
	RT30xxWriteRFRegister(pAd, RF_R01, RfReg);

	RT30xxReadRFRegister(pAd, RF_R30, &RfReg);
	RfReg = ((RfReg & ~0x18) | 0x10); /* rxvcm (Rx BB filter VCM) */
	RT30xxWriteRFRegister(pAd, RF_R30, RfReg);

#ifdef STREAM_MODE_SUPPORT
	/* Enable the stream mode */
	AsicEnableStreamMode(pAd);
#endif /* STREAM_MODE_SUPPORT */
}


static VOID RT3593_PostBBPInitialization(
	IN PRTMP_ADAPTER pAd)
{
	BBP_R105_STRUC BBPR105 = { { 0 } };
	UCHAR BbpReg = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R105, &BBPR105.byte);
	
	/* Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)*/
	if (pAd->Antenna.field.RxPath == 1) /* Single RX*/
		BBPR105.field.MLDFor2Stream = 0;
	else
		BBPR105.field.MLDFor2Stream = 1;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, BBPR105.byte);

	/* Avoid data lost and CRC error*/	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpReg);
	BbpReg = ((BbpReg & ~0x40) | 0x40); /* MAC interface control (MAC_IF_80M, 1: 80 MHz)*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpReg);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R92, 0x02);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x82);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, 0x25);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R104, 0x92);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R88, 0x90);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R148, 0xC8);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, 0x48);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R120, 0x50);

#ifdef TXBF_SUPPORT
	if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_REG_BF, 0xBD); /* Enable the profile saving of the explicit TxBf and the implicit TxBf */
	}
	else
#endif /* TXBF_SUPPORT */
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_REG_BF, 0x9D); /* Disable the profile saving of the explicit TxBf and the implicit TxBf*/
	}
		
	/* SNR mapping*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, 6);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, 160);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, 7);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, 161);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, 8);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, 162);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, 0x08); /* ADC/DAC control*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, 0x0B); /* Rx AGC energy lower bound in log2*/

	/* fix the throughput drop issue. AP 20/40M BW with STA 20M(10/06) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x04); /* FEQ/AEQ control*/

	if (pAd->CommonCfg.Channel > 14)
	{
		/* Disable CCK packet detection in 5G band */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x00);
	}
	else
	{ 	
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}


VOID NICInitRT3593RFRegisters(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR	RfReg = 0;
	UINT32	data;
	USHORT	i;

	/* Init RF calibration */
	/* Driver should toggle RF R30 bit7 before init RF registers */
	RT30xxReadRFRegister(pAd, RF_R02, &RfReg);
	RfReg = ((RfReg & ~0x80) | 0x80); /* rescal_en (initiate calbration) */
	RT30xxWriteRFRegister(pAd, RF_R02, RfReg);
	RTMPusecDelay(1000);
	RfReg = (RfReg & ~0x80); /* rescal_en (initiate calbration) */
	RT30xxWriteRFRegister(pAd, RF_R02, RfReg);

	/* Disable the GPIO #4 and #7 function for LNA PE control */
	RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
	data = (data & ~0x00000090) | 0x00000000;
	RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);

#ifdef HIGH_POWER_SUPPORT
	/* Disable the GPIO #4, #5 and #7 function for LNA PE0/1/2 control */
	RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
	data &= ~(0x000000B0);
	RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);
#endif /* HIGH_POWER_SUPPORT */

	/* Init RF calibration */
	/* Initialize RF register to default value */
	for (i = 0; i < NUM_RF_3053_REG_PARMS; i++)
		RT30xxWriteRFRegister(pAd, RF3053RegTable[i].Register, RF3053RegTable[i].Value);

	RfReg = 0x80; /* rescal_en (initiate calbration) */
	RT30xxWriteRFRegister(pAd, RF_R02, RfReg);

	/* Init RF frequency offset */
	RTMPAdjustFrequencyOffset(pAd, &pAd->RfFreqOffset);

	/* Driver should set RF R18 bit6 on before calibration */
	RT30xxReadRFRegister(pAd, RF_R18, &RfReg);
	RfReg |= ((RfReg & ~0x40) | 0x40); /* xo_tune_bypass (0: XO is auto-tuned and 1: XO tuning bypassed) */
	RT30xxWriteRFRegister(pAd, RF_R18, RfReg);

	/* Patch CCK ok, OFDM failed issue, just toggle and restore LDO_CFG0. */	
	/* Patch SRAM for 3572, increase voltage to 1.35V on core voltage and down to 1.2V after 1 msec */
	RTMP_IO_READ32(pAd, LDO_CFG0, &data);
	data = ((data & 0xE0FFFFFF) | 0x0D000000);
	RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
	RTMPusecDelay(1000);
	data = ((data & 0xE0FFFFFF) | 0x01000000);
	RTMP_IO_WRITE32(pAd, LDO_CFG0, data);

	/* Give bbp filter initial value */
	pAd->Mlme.CaliBW20RfR24 = 0x1F;
	pAd->Mlme.CaliBW40RfR24 = 0x2F; /* Bit[5] must be 1 for BW 40 */

	/* save R25, R26 for 2.4GHz */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R25, &pAd->Bbp25);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R26, &pAd->Bbp26);

	/* set led open drain enable */
	RTMP_IO_READ32(pAd, OPT_14, &data);
	data |= 0x01;
	RTMP_IO_WRITE32(pAd, OPT_14, data);

	/* add by johnli, RF power sequence setup, load RF normal operation-mode setup */
	RT3593LoadRFNormalModeSetup(pAd);

	/* adjust some BBP register contents */
	/* also can put these BBP registers to pBBPRegTable */
	RT3593_PostBBPInitialization(pAd);
	
	/* Enable the stream mode*/
#ifdef STREAM_MODE_SUPPORT
	AsicEnableStreamMode(pAd);
#endif /* STREAM_MODE_SUPPORT */
}


#ifdef STREAM_MODE_SUPPORT
VOID AsicUpdateTxChainAddress(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pMacAddress)
{
	static UCHAR whichTxChain = 0; /* Tx chain 0~3*/

	DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

	switch (whichTxChain + 1) /* Skip the Tx chain #0*/
	{
		case 1: /* Tx chain address 1*/
		{
			TX_CHAIN_ADDR1_L_STRUC TxChainAddr1L = {{0}};
			TX_CHAIN_ADDR1_H_STRUC TxChainAddr1H = {{0}};

			TxChainAddr1L.field.TxChainAddr1L_Byte3 = pMacAddress[3];
			TxChainAddr1L.field.TxChainAddr1L_Byte2 = pMacAddress[2];
			TxChainAddr1L.field.TxChainAddr1L_Byte1 = pMacAddress[1];
			TxChainAddr1L.field.TxChainAddr1L_Byte0 = pMacAddress[0];
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR1_L, TxChainAddr1L.word);

			TxChainAddr1H.field.TxChainAddr1H_Byte5 = pMacAddress[5];
			TxChainAddr1H.field.TxChainAddr1H_Byte4 = pMacAddress[4];
			TxChainAddr1H.field.TxChainSel0 = 0xF;
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR1_H, TxChainAddr1H.word);

			DBGPRINT(RT_DEBUG_TRACE, ("%s: Tx chain address 1, MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n", 
				__FUNCTION__, 
				pMacAddress[0], pMacAddress[1], pMacAddress[2], 
				pMacAddress[3], pMacAddress[4], pMacAddress[5]));
		}
		break;

		case 2: /* Tx chain address 2*/
		{
			TX_CHAIN_ADDR2_L_STRUC TxChainAddr2L = {{0}};
			TX_CHAIN_ADDR2_H_STRUC TxChainAddr2H = {{0}};

			TxChainAddr2L.field.TxChainAddr2L_Byte3 = pMacAddress[3];
			TxChainAddr2L.field.TxChainAddr2L_Byte2 = pMacAddress[2];
			TxChainAddr2L.field.TxChainAddr2L_Byte1 = pMacAddress[1];
			TxChainAddr2L.field.TxChainAddr2L_Byte0 = pMacAddress[0];
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR2_L, TxChainAddr2L.word);

			TxChainAddr2H.field.TxChainAddr2H_Byte5 = pMacAddress[5];
			TxChainAddr2H.field.TxChainAddr2H_Byte4 = pMacAddress[4];
			TxChainAddr2H.field.TxChainSel0 = 0xF;
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR2_H, TxChainAddr2H.word);

			DBGPRINT(RT_DEBUG_TRACE, ("%s: Tx chain address 2, MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n", 
				__FUNCTION__, 
				pMacAddress[0], pMacAddress[1], pMacAddress[2], 
				pMacAddress[3], pMacAddress[4], pMacAddress[5]));
		}
		break;

		case 3: /* Tx chain address 3*/
		{
			TX_CHAIN_ADDR3_L_STRUC TxChainAddr3L = {{0}};
			TX_CHAIN_ADDR3_H_STRUC TxChainAddr3H = {{0}};

			TxChainAddr3L.field.TxChainAddr3L_Byte3 = pMacAddress[3];
			TxChainAddr3L.field.TxChainAddr3L_Byte2 = pMacAddress[2];
			TxChainAddr3L.field.TxChainAddr3L_Byte1 = pMacAddress[1];
			TxChainAddr3L.field.TxChainAddr3L_Byte0 = pMacAddress[0];
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR3_L, TxChainAddr3L.word);

			TxChainAddr3H.field.TxChainAddr3H_Byte5 = pMacAddress[5];
			TxChainAddr3H.field.TxChainAddr3H_Byte4 = pMacAddress[4];
			TxChainAddr3H.field.TxChainSel0 = 0xF;
			RTMP_IO_WRITE32(pAd, TX_CHAIN_ADDR3_H, TxChainAddr3H.word);

			DBGPRINT(RT_DEBUG_TRACE, ("%s: Tx chain address 3, MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n", 
				__FUNCTION__, 
				pMacAddress[0], pMacAddress[1], pMacAddress[2], 
				pMacAddress[3], pMacAddress[4], pMacAddress[5]));
		}
		break;

		default: 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect whichTxChain (=%d)\n", __FUNCTION__, whichTxChain));
		}
		break;
	}

	whichTxChain = whichTxChain + 1;
	whichTxChain = whichTxChain % (NUM_OF_TX_CHAIN - 1); /* Skip the Tx chain #0*/

	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
}
#endif /* STREAM_MODE_SUPPORT */


static VOID RT3593_ChipSwitchChannel(
	IN	PRTMP_ADAPTER 				pAd,
	IN	UCHAR						Channel,
	IN	BOOLEAN						bScan)
{
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; /*Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER;*/
	CHAR TxPwer3 = 0;
	UCHAR	index;
	UCHAR 	RFValue = 0;
	UINT32  MacValue;
	UCHAR Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0, Tx2FinePowerCtrl = 0;
	BBP_R109_STRUC BbpR109 = { { 0 } };
	BBP_R110_STRUC BbpR110 = { { 0 } };
	UCHAR TxRxAgcFc = 0, TxRxh20M = 0;
	INTERNAL_1_STRUCT Internal_1 = { { 0 } };
	/*UCHAR PreRFValue = 0;*/
	/*
         * We can't use ChannelList to search channel, since some central channl's 
         * txpowr doesn't list in ChannelList, so use TxPower array instead.
	 */
	for (index = 0; index < MAX_NUM_OF_CHANNELS; index++)
	{
		if (Channel == pAd->TxPower[index].Channel)
		{
			TxPwer = pAd->TxPower[index].Power;
			TxPwer2 = pAd->TxPower[index].Power2;
			TxPwer3 = pAd->TxPower[index].Power3;

			Tx0FinePowerCtrl = pAd->TxPower[index].Tx0FinePowerCtrl;
			Tx1FinePowerCtrl = pAd->TxPower[index].Tx1FinePowerCtrl;
			Tx2FinePowerCtrl = pAd->TxPower[index].Tx2FinePowerCtrl;
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: Can't find the Channel#%d \n", Channel));
	}

	for (index = 0; index < NUM_OF_3053_CHNL; index++)
	{
		if (pAd->RfIcType != RFIC_3053)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect RF IC type, pAd->RfIcType = 0x%X", 
				__FUNCTION__, 
				pAd->RfIcType));
			break;
		}

		if (Channel == FreqItems3053[index].Channel)
		{
			/* Set the BBP Tx fine power control in 0.1dB step*/	
			BbpR109.field.Tx0PowerCtrl = Tx0FinePowerCtrl;

			if (pAd->Antenna.field.TxPath >= 2)
				BbpR109.field.Tx1PowerCtrl = Tx1FinePowerCtrl;
			else
				BbpR109.field.Tx1PowerCtrl = 0;
				
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R109, BbpR109.byte);

			DBGPRINT(RT_DEBUG_INFO, ("%s: Channel = %d, BBP_R109 = 0x%X\n", 
				__FUNCTION__, 
				Channel, 
				BbpR109.byte));

			if (pAd->Antenna.field.TxPath == 3)
			{
				BbpR110.field.Tx2PowerCtrl = Tx2FinePowerCtrl; /* Tx2 power control in 0.1dB step*/
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R110, BbpR110.byte);
			}

			/* for 2.4G, restore BBP25, BBP26*/
			if (Channel <= 14)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, pAd->Bbp25);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, pAd->Bbp26);
			}
			/* hard code for 5GGhz, Gary 2008-12-10*/
			else
			{
				/* Enable IQ Phase Correction*/
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x09);
				/* IQ Phase correction value*/
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R26, 0xFF);
			}

			/* Programming channel parameters*/
			RT30xxWriteRFRegister(pAd, RF_R08, FreqItems3053[index].N); /* N*/
			RT30xxWriteRFRegister(pAd, RF_R09, (FreqItems3053[index].K & 0x0F)); /* K, N<11:8> is set to zero*/

			RT30xxReadRFRegister(pAd, RF_R11, &RFValue);
			RFValue = ((RFValue & ~0x03) | (FreqItems3053[index].R & 0x03)); /* R*/
			RT30xxWriteRFRegister(pAd, RF_R11, RFValue);

			RT30xxReadRFRegister(pAd, RF_R11, &RFValue);
			if  (Channel <= 14)
				RFValue = ((RFValue & ~0x4C) | 0x44); /* pll_idoh (charge pump current control, 1: x2) and pll_mod (choose fractional divide, 01: mod10)*/
			else
				RFValue = ((RFValue & ~0x4C) | 0x48); /* pll_idoh (charge pump current control, 1: x2) and pll_mod (choose fractional divide, 10: mod2)*/

			RT30xxWriteRFRegister(pAd, RF_R11, RFValue);

			RT30xxReadRFRegister(pAd, RF_R53, &RFValue);

			if  (Channel <= 14)
			{
				RFValue = 0x00; /* 000xxxxx*/
				RFValue = (RFValue | (TxPwer & 0x1F)); /* tx0_alc*/
			}
			else
			{

#ifdef RTMP_MAC_PCI
				if (IS_PCI_INF(pAd))
				{
				/*
					20100910, Improve RT3593 TX DAC is too low issue
					a(ch36~ch140): 01xx0xxx 
					a(ch149~ch165): 01xx1xxx
				*/
				if ((Channel >= 36) && (Channel <= 140))
					RFValue = 0x40;
				else
					RFValue = 0x48;
				}
#endif /* RTMP_MAC_PCI */

				
				RFValue = (RFValue | ((TxPwer & 0x18) << 1) | (TxPwer & 0x07)); /* tx0_alc*/
			}

			RT30xxWriteRFRegister(pAd, RF_R53, RFValue);

			RT30xxReadRFRegister(pAd, RF_R55, &RFValue);

			if  (Channel <= 14)
			{
				RFValue = 0x00; /* 000xxxxx*/
				RFValue = (RFValue | (TxPwer2 & 0x1F)); /* tx1_alc*/
			}
			else
			{
#ifdef RTMP_MAC_PCI
				if (IS_PCI_INF(pAd))
				{
				/*
					20100910, Improve RT3593 TX DAC is too low issue
					a(ch36~ch140): 01xx0xxx 
					a(ch149~ch165): 01xx1xxx
				*/
				if ((Channel >= 36) && (Channel <= 140))
					RFValue = 0x40;
				else
					RFValue = 0x48;
				}
#endif /* RTMP_MAC_PCI */

				
				RFValue = (RFValue | ((TxPwer2 & 0x18) << 1) | (TxPwer2 & 0x07)); /* tx1_alc*/
			}

			RT30xxWriteRFRegister(pAd, RF_R55, RFValue);

			RT30xxReadRFRegister(pAd, RF_R54, &RFValue);

			if  (Channel <= 14)
			{
				RFValue = 0x00; /* 000xxxxx*/
				RFValue = (RFValue | (TxPwer3 & 0x1F)); /* tx2_alc*/
			}
			else
			{
#ifdef RTMP_MAC_PCI
				if (IS_PCI_INF(pAd))
				{
				/*
					20100910, Improve RT3593 TX DAC is too low issue
					a(ch36~ch140): 01xx0xxx 
					a(ch149~ch165): 01xx1xxx
				*/
				if ((Channel >= 36) && (Channel <= 140))
					RFValue = 0x40;
				else
					RFValue = 0x48;
				}
#endif /* RTMP_MAC_PCI */

				
				RFValue = (RFValue | ((TxPwer3 & 0x18) << 1) | (TxPwer3 & 0x07)); /* tx2_alc*/
			}
			RT30xxWriteRFRegister(pAd, RF_R54, RFValue);

			RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
			RFValue &= (~0xFC);	/* Clear bit 2~7*/
			RFValue = ((RFValue & ~0x03) | 0x03); /* vco_en and pll_en*/
				
			if (pAd->Antenna.field.TxPath == 1)
			{
				RFValue &= ~0xA0; /* Disable tx1_en and tx2_en*/
				RFValue |= 0x08; /* Enable tx0_en*/
			}
			else if (pAd->Antenna.field.TxPath == 2)
			{
				RFValue &= ~0x80; /* Disable tx2_en*/
				RFValue |= 0x28; /* Enable tx0_en and tx1_en*/
			}
			else if (pAd->Antenna.field.TxPath == 3)
				RFValue |= 0xA8; /* Enable tx0_en, tx1_en and tx2_en*/
				
			if (pAd->Antenna.field.RxPath == 1)
			{
				RFValue &= ~0x50; /* Disable rx1_en and rx2_en*/
				RFValue |= 0x04; /* Enable rx0_en*/
			}
			else if (pAd->Antenna.field.RxPath == 2)
			{
				RFValue &= ~0x40; /* Disable rx2_en*/
				RFValue |= 0x14; /* Enable rx0_en and rx1_en*/
			}
			else if (pAd->Antenna.field.RxPath == 3)
					RFValue |= 0x54; /* Enable rx0_en, rx1_en and  rx2_en*/

			RT30xxWriteRFRegister(pAd, RF_R01, RFValue);


			if ((!bScan) && (pAd->CommonCfg.BBPCurrentBW == BW_40)) /* BW 40*/
			{
				TxRxh20M = ((pAd->Mlme.CaliBW40RfR24 & 0x20) >> 5); /* Tx/Rx h20M*/
				TxRxAgcFc = (pAd->Mlme.CaliBW40RfR24 & 0x1F); /* Tx/Rx agc fc*/
			}
			else /* BW 20*/
			{
				TxRxh20M = ((pAd->Mlme.CaliBW20RfR24 & 0x20) >> 5); /* Tx/Rx h20M*/
				TxRxAgcFc = (pAd->Mlme.CaliBW20RfR24 & 0x1F); /* Tx/Rx agc fc*/
			}

			RT30xxReadRFRegister(pAd, RF_R32, &RFValue);
			RFValue = ((RFValue & ~0xF8) | (TxRxAgcFc << 3)); /* tx_agc_fc (capacitor control in Tx baseband filter)*/
			/*RT30xxWriteRFRegister(pAd, RF_R32, (UCHAR)RFValue);*/
				
			if (Channel <= 14) /* 2.4GHz*/
			{
				RFValue = 0xA0; /* rx_agc_fc (capacitor control in Rx baseband filter)*/
				RT30xxWriteRFRegister(pAd, RF_R31, RFValue);
			}
			else /* 5GHz*/
			{
				RFValue = 0x80; /* rx_agc_fc (capacitor control in Rx baseband filter)*/
				RT30xxWriteRFRegister(pAd, RF_R31, RFValue);
			}

			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue = ((RFValue & ~0x06) | (TxRxh20M << 1) | (TxRxh20M << 2)); /* tx_h20M and rx_h20M*/
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);

			RT30xxReadRFRegister(pAd, RF_R36, &RFValue);
			if  (Channel <= 14)
				RFValue = ((RFValue & ~0x80) | 0x80); /* RF_BS (RF band select, 1: g-band operation)*/
			else
				RFValue = ((RFValue & ~0x80) | 0x00); /* RF_BS (RF band select, 0: a-band operation)*/

			RT30xxWriteRFRegister(pAd, RF_R36, RFValue);
			
			RT30xxReadRFRegister(pAd, RF_R34, &RFValue);

			if  (Channel <= 14)
				RFValue = 0x3C; /* vcolo_bs (VCO buffer ch1, ch2 and ch0 band tuning, 111: g-band) and vcolobuf_ien (VCO buffer current control, 1: high current)*/
			else
				RFValue = 0x20; /* vcolo_bs (VCO buffer ch1, ch2 and ch0 band tuning, 000: a-band) and vcolobuf_ien (VCO buffer current control, 1: high current)*/

			RT30xxWriteRFRegister(pAd, RF_R34, RFValue);

			RT30xxReadRFRegister(pAd, RF_R12, &RFValue);

			if (Channel <= 14)
				RFValue = 0x1A; /* pfd_delay (10: 0.9 ns), pll_r2 (011: 4K) and pll_c1 (010: 17 pF)*/
			else
				RFValue = 0x12; /* pfd_delay (10: 0.9 ns), pll_r2 (100: 5.5 K) and pll_c1 (01: 17 pF)*/

			RT30xxWriteRFRegister(pAd, RF_R12, RFValue);

			RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
			RFValue = ((RFValue & ~0xC0) | 0x80); /* vco_ic (VCO bias current control, 10: mid.)*/
			RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue = ((RFValue & ~0x18) | 0x10); /* rxvcm (Rx BB filter VCM)*/
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);

			RT30xxWriteRFRegister(pAd, RF_R46, 0x60);
			
			if  (Channel <= 14)
			{
				RT30xxWriteRFRegister(pAd, RF_R10, 0xD3); // pll_comp_ic and pll_pre_ic
				RT30xxWriteRFRegister(pAd, RF_R13, 0x12); // pll_r3 and pll_c3
			}
			else
			{
				RT30xxWriteRFRegister(pAd, RF_R10, 0xD8); // pll_comp_ic and pll_pre_ic
				RT30xxWriteRFRegister(pAd, RF_R13, 0x23); // pll_r3 and pll_c3
			}

			RT30xxReadRFRegister(pAd, RF_R51, &RFValue);
			RFValue = ((RFValue & ~0x03) | 0x01); /* transmit IF mixer current control (both bands)*/
			RT30xxWriteRFRegister(pAd, RF_R51, RFValue);

			if  (Channel <= 14)
			{
				RT30xxReadRFRegister(pAd, RF_R51, &RFValue);
/*				RFValue = ((RFValue & ~0x1C) | ((pAd->TxMixerGain24G & 0x03) << 2));   tx_mx1_cc (RF mixer output tank tuning (both bands))*/
				RFValue = ((RFValue & ~0x1C) | (0x05 << 2)); /*  tx_mx1_cc (RF mixer output tank tuning (both bands), 101: g-band)*/
				RT30xxWriteRFRegister(pAd, RF_R51, RFValue);

				RT30xxReadRFRegister(pAd, RF_R51, &RFValue);
				RFValue = ((RFValue & ~0xE0) | 0x60); /* tx_mx1_ic (transmit RF mixer current control (both bands), 011: g-band)*/
				RT30xxWriteRFRegister(pAd, RF_R51, RFValue);
			}
			else
			{
				RT30xxReadRFRegister(pAd, RF_R51, &RFValue);
/*				RFValue = ((RFValue & ~0x1C) | ((pAd->TxMixerGain5G & 0x03) << 2));   tx_mx1_cc (RF mixer output tank tuning (both bands))*/
				RFValue = ((RFValue & ~0x1C) | (0x04 << 2)); /*  tx_mx1_cc (RF mixer output tank tuning (both bands), 100: a-band)*/
				RT30xxWriteRFRegister(pAd, RF_R51, RFValue);

				RT30xxReadRFRegister(pAd, RF_R51, &RFValue);
				RFValue = ((RFValue & ~0xE0) | 0x40); /* tx_mx1_ic (transmit RF mixer current control (both bands), 010: a-band)*/
				RT30xxWriteRFRegister(pAd, RF_R51, RFValue);
			}

			if  (Channel <= 14)
			{
				RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
				RFValue = ((RFValue & ~0x1C) | 0x0C); /* tx_lo1_ic (transmit LO1 current control, 011: g-band)*/
#ifdef TXBF_SUPPORT
				/* 3593 TXBF TODO */
				if ((pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn) || (pAd->CommonCfg.ETxBfEnCond))
				{
					RFValue = ((RFValue & ~0x20) | 0x20); /* Tx divider */
				}
#endif /* TXBF_SUPPORT */
				RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

				RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
				RFValue = ((RFValue & ~0x20) | 0x00); /* tx_lo1_en (0:LO1 follows TR switch)*/
				RT30xxWriteRFRegister(pAd, RF_R50, RFValue);
			}
			else
			{
				RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
				RFValue = ((RFValue & ~0x1C) | 0x08); /* tx_lo1_ic (transmit LO1 current control, 010: g-band)*/
#ifdef TXBF_SUPPORT
				/* 3593 TXBF TODO */
				if ((pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn) || (pAd->CommonCfg.ETxBfEnCond))
				{
					RFValue = ((RFValue & ~0x20) | 0x20); /* Tx divider */
				}
#endif /* TXBF_SUPPORT */
				RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

				RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
				RFValue = ((RFValue & ~0x20) | 0x00); /* tx_lo1_en (0:LO1 follows TR switch)*/
				RT30xxWriteRFRegister(pAd, RF_R50, RFValue);
			}

			if  (Channel <= 14)
			{
				RT30xxReadRFRegister(pAd, RF_R57, &RFValue);
				RFValue = ((RFValue & ~0xFC) | 0x6C); /* drv_cc (balun capacitor calbration, 011011: g-band)*/
				RT30xxWriteRFRegister(pAd, RF_R57, RFValue);
			}
			else
			{
				RT30xxReadRFRegister(pAd, RF_R57, &RFValue);
				RFValue = ((RFValue & ~0xFC) | 0x3C); /* drv_cc (balun capacitor calbration, 001111: a-band)*/
				RT30xxWriteRFRegister(pAd, RF_R57, RFValue);
			}

			if (Channel <= 14)
			{
				RT30xxReadRFRegister(pAd, RF_R44, &RFValue);
				RFValue = 0x93; /* rx_mix1_ic, rxa_lnactr, lna_vc, lna_inbias_en and lna_en*/
				RT30xxWriteRFRegister(pAd, RF_R44, RFValue);

				RT30xxReadRFRegister(pAd, RF_R52, &RFValue);
				RFValue = 0x45; /* drv_gnd_a, tx_vga_cc_a and tx_mx2_gain*/
				RT30xxWriteRFRegister(pAd, RF_R52, RFValue);

				RT30xxReadRFRegister(pAd, RF_R03, &RFValue);
				RFValue = ((RFValue & ~0x80) | 0x80); /* vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration.*/
				RT30xxWriteRFRegister(pAd, RF_R03, RFValue);					
			}
			else
			{
				RT30xxReadRFRegister(pAd, RF_R44, &RFValue);
				RFValue = 0x9B; /* rx_mix1_ic, rxa_lnactr, lna_vc, lna_inbias_en and lna_en*/
				RT30xxWriteRFRegister(pAd, RF_R44, RFValue);

				RT30xxReadRFRegister(pAd, RF_R52, &RFValue);
				RFValue = 0x05; /* drv_gnd_a, tx_vga_cc_a and tx_mx2_gain*/
				RT30xxWriteRFRegister(pAd, RF_R52, RFValue);

				RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = ((RFValue & ~0x80) | 0xBE); /* vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration.*/
				RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);
			}

			if ((Channel >= 1) && (Channel <= 14)) /* 2.4GHz*/
			{
				RFValue = 0x23;
#ifdef TXBF_SUPPORT
				/* 3593 TXBF TODO */
				if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn || pAd->CommonCfg.ETxBfEnCond)
				{
					RFValue = ((RFValue & ~0x40) | 0x40); /* Rx divider */
				}
#endif /* TXBF_SUPPORT */
				RT30xxWriteRFRegister(pAd, RF_R39, RFValue);

				RFValue = 0xBB;
				RT30xxWriteRFRegister(pAd, RF_R45, RFValue);
			}
			else if ((Channel >= 36) && (Channel <= 64)) /* Low channels in 5GHz*/
			{
				RFValue = 0x36;
#ifdef TXBF_SUPPORT
				/* 3593 TXBF TODO */
				if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn || pAd->CommonCfg.ETxBfEnCond)
				{
					RFValue = ((RFValue & ~0x40) | 0x40); /* Rx divider */
				}
#endif /* TXBF_SUPPORT */
				RT30xxWriteRFRegister(pAd, RF_R39, RFValue);

				RFValue = 0xEB;
				RT30xxWriteRFRegister(pAd, RF_R45, RFValue);
			}
			else if ((Channel >= 100) && (Channel <= 128)) /* Middle channels in 5GHz*/
			{
				RFValue = 0x32;
#ifdef TXBF_SUPPORT
				/* 3593 TXBF TODO */
				if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn || pAd->CommonCfg.ETxBfEnCond)
				{
					RFValue = ((RFValue & ~0x40) | 0x40); /* Rx divider */
				}
#endif /* TXBF_SUPPORT */
				RT30xxWriteRFRegister(pAd, RF_R39, RFValue);

				RFValue = 0xB3;
				RT30xxWriteRFRegister(pAd, RF_R45, RFValue);
			}
			else /* High channel in 5GHz*/
			{
				RFValue = 0x30;
#ifdef TXBF_SUPPORT
				/* 3593 TXBF TODO */
				if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn || pAd->CommonCfg.ETxBfEnCond)
				{
					RFValue = ((RFValue & ~0x40) | 0x40); /* Rx divider */
				}
#endif /* TXBF_SUPPORT */
				RT30xxWriteRFRegister(pAd, RF_R39, RFValue);

				RFValue = 0x9B;
				RT30xxWriteRFRegister(pAd, RF_R45, RFValue);
			}

			/* TODO: maybe need to check all MBSS phymode ? */
			if ((pAd->CommonCfg.BBPCurrentBW == BW_40) // BW 40
#ifdef CONFIG_AP_SUPPORT
				|| (pAd->ApCfg.MBSSID[0].PhyMode <= PHY_11G) // non-11n
#endif /* CONFIG_AP_SUPPORT */
				)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x04); // FEQ/AEQ control
			}
			else // BW 20
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x34); // FEQ/AEQ control

			pAd->LatchRfRegs.Channel = Channel; /* Channel latch*/

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
			DBGPRINT(RT_DEBUG_TRACE, ("%s: RT3053: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
				__FUNCTION__, 
				Channel, 
				pAd->RfIcType, 
				TxPwer, 
				TxPwer2, 
				pAd->Antenna.field.TxPath, 
				FreqItems3053[index].N, 
				FreqItems3053[index].K, 
				FreqItems3053[index].R));

			break;
		}
	}

	/* Change BBP setting during siwtch from a->g, g->a*/
	if (Channel <= 14)
	{
		UINT32	TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* Rx High power VGA offset for LNA select*/
		//RT30xxWriteRFRegister(pAd, RF_R10, 0xD3);
		//RT30xxWriteRFRegister(pAd, RF_R13, 0x12);
	
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R77, 0x98);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x8A);

		if (pAd->NicConfig2.field.ExternalLNAForG)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		/* 5G band selection PIN, bit1 and bit2 are complement*/
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &MacValue);
		MacValue &= (~0x6);
		MacValue |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, MacValue);

		TxPinCfg = 0x32050F0A;

		/*Disable unused PA_PE*/
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg = TxPinCfg & ~0x0300000D; /*PA_PE_G2, PA_PE_A2, PA_PE_G1, PA_PE_A1, PA_PE_A0*/
		else if (pAd->Antenna.field.TxPath == 2)
				TxPinCfg = TxPinCfg & ~0x03000005; /*PA_PE_G2, PA_PE_A2, PA_PE_A1, PA_PE_A0*/

		/*Disable unused LNA_PE*/
		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg = TxPinCfg & ~0x30000C00; 
		else if (pAd->Antenna.field.RxPath == 2)
				TxPinCfg = TxPinCfg & ~0x30000000;
			
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		/* PCIe PHY Transmit attenuation adjustment*/
	        if (IS_PCIE_INF(pAd))
		{
			/* PCIe PHY Transmit attenuation adjustment*/
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

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		UINT32	TxPinCfg = 0x00050F05;/*Gary 2007/8/9 0x050505*/
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		//RT30xxWriteRFRegister(pAd, RF_R10, 0xD8);
		//RT30xxWriteRFRegister(pAd, RF_R13, 0x23);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R77, 0x98);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x9A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x82);

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
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &MacValue);
		MacValue &= (~0x6);
		MacValue |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, MacValue);

		/* Turn off unused PA or LNA when only 1T or 1R*/
		TxPinCfg = 0x31050F05;

		/*Disable unused PA_PE*/
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg = TxPinCfg & ~0x0300000E; /*PA_PE_G2, PA_PE_A2, PA_PE_G1, PA_PE_A1, PA_PE_G0*/
		else if (pAd->Antenna.field.TxPath == 2)
			TxPinCfg = TxPinCfg & ~0x0300000A; /*PA_PE_G2, PA_PE_A2, PA_PE_G1, PA_PE_G0*/

		/*Disable unused LNA_PE*/
		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg = TxPinCfg & ~0x30000C00; 
		else if (pAd->Antenna.field.RxPath == 2)
			TxPinCfg = TxPinCfg & ~0x30000000;

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
	
	/* GPIO control*/
	RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &MacValue);

#ifdef RTMP_MAC_PCI
	if (IS_PCI_ONLY_INF(pAd))
	{
		/* Band selection (one GPIO pin controls three paths): GPIO #7 (output) */
		if  (Channel <= 14)
		{
			 MacValue = ((MacValue & ~0x00008080) | 0x00000080);
		}
		else
		{
			MacValue = ((MacValue & ~0x00008080) | 0x00000000);
		}

		/* LNA PE control (one GPIO pin controls three LNA PEs): GPIO #4 (output) */
		MacValue = ((MacValue & ~0x00001010) | 0x00000010);
	}

	if (IS_PCIE_INF(pAd))
	{
		/* Band selection (one GPIO pin controls three paths): GPIO #8 (output) */
		if  (Channel <= 14)
		{
			MacValue = ((MacValue & ~0x01010000) | 0x00010000);
		}
		else
		{
			MacValue = ((MacValue & ~0x01010000) | 0x00000000);
		}

		/* LNA PE control (one GPIO pin controls three LNA PEs): GPIO #4 (output) */
		MacValue = ((MacValue & ~0x00001010) | 0x00000010);
	}
#endif /* RTMP_MAC_PCI */


	RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, MacValue);

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


/* The verification of the Tx power per rate*/
VOID RTMPVerifyTxPwrPerRateExt(
	IN		PRTMP_ADAPTER			pAd, 
	INOUT	PUCHAR					pTxPwr)
{
	DBGPRINT(RT_DEBUG_INFO, ("--> %s\n", __FUNCTION__));

	if (*pTxPwr > 0xC)
		*pTxPwr = 0xC;
	
	DBGPRINT(RT_DEBUG_INFO, ("<-- %s\n", __FUNCTION__));
}


/* Read the Tx power (per data rate) based on the extended EEPROM format*/
VOID RTMPReadTxPwrPerRateExt(
	IN	PRTMP_ADAPTER				pAd)
{
	USHORT value = 0;
	ULONG TxPwrOverMAC = 0;
	UCHAR t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0, t7 = 0, t8 = 0;
	UCHAR t9 = 0, t10 = 0, t11 = 0, t12 = 0, t13 = 0, t14 = 0, t15 = 0, t16 = 0;
	UCHAR t17 = 0, t18 = 0, t19 = 0, t20 = 0, t21 = 0, t22 = 0, t23 = 0, t24 = 0;
	UCHAR t25 = 0, t26 = 0, t27 = 0, t28 = 0, t29 = 0, t30 = 0, t31 = 0, t32 = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	
	/* (a) Default Tx power for BW20 over 2.4G */
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (t2 << 12) | (t2 << 8) | (t1 << 4) | (t1));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (t2 << 8) | (t1));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9));

	
	/* (b) Default Tx power for BW40 over 2.4G */
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (DEFAULT_TX_POWER << 12) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER << 4) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("\n%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9));

	
	/* (c) Default Tx power for BW20 over 5G */
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (DEFAULT_TX_POWER << 12) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER << 4) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("\n%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9));

	
	/* (d) Default Tx power for BW40 over 5G */
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G, value);
	t1 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t1);
	t2 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t2);
	t3 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t3);
	t4 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t4);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 2), value);
	t5 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t5);
	t6 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t6);
	t7 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t7);
	t8 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 4), value);
	t9 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t9);
	t10 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t10);
	t11 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t11);
	t12 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t12);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 6), value);
	t13 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t13);
	t14 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t14);
	t15 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t15);
	t16 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t16);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 8), value);
	t17 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t17);
	t18 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t18);
	t19 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t19);
	t20 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t20);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 10), value);
	t21 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t21);
	t22 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t22);
	t23 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t23);
	t24 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t24);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 12), value);
	t25 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t25);
	t26 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t26);
	t27 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t27);
	t28 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t28);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G + 14), value);
	t29 = (value & 0x000F);
	RTMPVerifyTxPwrPerRateExt(pAd, &t29);
	t30 = ((value & 0x00F0) >> 4);
	RTMPVerifyTxPwrPerRateExt(pAd, &t30);
	t31 = ((value & 0x0F00) >> 8);
	RTMPVerifyTxPwrPerRateExt(pAd, &t31);
	t32 = ((value & 0xF000) >> 12);
	RTMPVerifyTxPwrPerRateExt(pAd, &t32);

	TxPwrOverMAC = ((t4 << 28) | (t4 << 24) | (t3 << 20) | (t3 << 16) | 
	                             (DEFAULT_TX_POWER << 12) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER << 4) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("\n%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0));

	TxPwrOverMAC = ((t4 << 24) | (t3 << 16) | (DEFAULT_TX_POWER << 8) | (DEFAULT_TX_POWER));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext));

	TxPwrOverMAC = ((t10 << 28) | (t10 << 24) | (t9 << 20) | (t9 << 16) | 
	                             (t6 << 12) | (t6 << 8) | (t5 << 4) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1));

	TxPwrOverMAC = ((t10 << 24) | (t9 << 16) | (t6 << 8) | (t5));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext));

	TxPwrOverMAC = ((t15 << 28) | (t15 << 24) | (t14 << 20) | (t14 << 16) | 
	                             (t12 << 12) | (t12 << 8) | (t11 << 4) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2));

	TxPwrOverMAC = ((t15 << 24) | (t14 << 16) | (t12 << 8) | (t11));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext));

	TxPwrOverMAC = ((t26 << 28) | (t26 << 24) | (t25 << 20) | (t25 << 16) | 
	                             (t17 << 12) | (t17 << 8) | (t16 << 4) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3));

	TxPwrOverMAC = ((t26 << 24) | (t25 << 16) | (t17 << 8) | (t16));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext));

	TxPwrOverMAC = ((t28 << 12) | (t28 << 8) | (t27 << 4) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4));

	TxPwrOverMAC = ((t28 << 8) | (t27));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext));

	TxPwrOverMAC = ((t20 << 24) | (t20 << 20) | (t20 << 16) | 
	                            (t19 << 8) | (t19 << 4) | (t19));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5));

	TxPwrOverMAC = ((t22 << 24) | (t22 << 20) | (t22 << 16) | 
	                            (t21 << 8) | (t21 << 4) | (t21));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6));

	TxPwrOverMAC = ((t13 << 24) | (t13 << 20) | (t13 << 16) | 
	                            (t7 << 8) | (t7 << 4) | (t7));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7));

	TxPwrOverMAC = ((t23 << 24) | (t23 << 20) | (t23 << 16) | 
	                            (t18 << 8) | (t18 << 4) | (t18));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8));

	TxPwrOverMAC = ((t29 << 8) | (t29 << 4) | (t29));
	pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9 = TxPwrOverMAC;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9 = 0x%08lX\n", 
		__FUNCTION__, 
		pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9));


	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}

VOID RT3593_AsicGetTxPowerOffset(
	IN		PRTMP_ADAPTER			pAd,
	INOUT 	PULONG 					pTxPwr)
{
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;

	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	if (IS_RT3593(pAd))
	{
		CfgOfTxPwrCtrlOverMAC.NumOfEntries = MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS;

		if (pAd->CommonCfg.BBPCurrentBW == BW_40)
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg0Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg1Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg2Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg3Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg4Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over5G.TxPwrCfg9;
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg0Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg1Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg2Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg3Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg4Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW40Over2Dot4G.TxPwrCfg9;
			}
		}
		else
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg0Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg1Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg2Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg3Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg4Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over5G.TxPwrCfg9;
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg0Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg1Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg2Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg3Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg4Ext;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg7;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg8;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->TxPwrCtrlExtOverMAC.BW20Over2Dot4G.TxPwrCfg9;
			}
		}

		NdisCopyMemory(pTxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));
	}
}


/* RT3593_AsicSetFreqOffset - set Frequency Offset register */
void RT3593_AsicSetFreqOffset(
	IN  PRTMP_ADAPTER   pAd,
	IN	ULONG			freqOffset)
{
	UCHAR diff;
	UCHAR RFValue, RFValue2 = 0;

	/* Set RF offset RF_R17 */
	RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RFValue);
	RFValue2 = (RFValue & 0x80) | freqOffset;

	if (RFValue2 > RFValue)
	{
		for (diff = 1; diff <= (RFValue2 - RFValue); diff++)
			RT30xxWriteRFRegister(pAd, RF_R17, (UCHAR)(RFValue + diff));
	}
	else
	{
		for (diff = 1; diff <= (RFValue - RFValue2); diff++)
			RT30xxWriteRFRegister(pAd, RF_R17, (UCHAR)(RFValue - diff));
	}
}


/*
========================================================================
Routine Description:
	Initialize operators and capability for RT3593.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT3593_Init(
	IN	PRTMP_ADAPTER pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	pChipCap->pRFRegTable = RF3053RegTable;
	pChipCap->MaxNumOfRfId = 63;
	pChipCap->MaxNumOfBbpId = 185;
	pChipCap->pBBPRegTable = NULL;
	pChipCap->bbpRegTbSize = RT3593_NUM_BBP_REG_PARMS;
	pChipCap->RfReg17WtMethod = RF_REG_WT_METHOD_STEP_ON;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_2;
	pChipCap->VcoPeriod = 5;
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_GRP);
#ifdef RTMP_EFUSE_SUPPORT
	pChipCap->EFUSE_USAGE_MAP_START = 0x3c0;
	pChipCap->EFUSE_USAGE_MAP_END = 0x3fb;
	pChipCap->EFUSE_USAGE_MAP_SIZE = 60;
	DBGPRINT(RT_DEBUG_ERROR, ("(Reassign Efuse for 3x7x/3x9x/53xx) Size=0x%x [%x-%x] \n",pAd->chipCap.EFUSE_USAGE_MAP_SIZE,pAd->chipCap.EFUSE_USAGE_MAP_START,pAd->chipCap.EFUSE_USAGE_MAP_END));
#endif /* RTMP_EFUSE_SUPPORT */
#ifdef RTMP_FLASH_SUPPORT
	pChipCap->eebuf = RT3593_EeBuffer,
#endif /* RTMP_FLASH_SUPPORT */
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 20;
	pChipCap->FlgIsHwWapiSup = TRUE;
#ifdef NEW_MBSSID_MODE
	pChipCap->MBSSIDMode = MBSSID_MODE1;
#else
	pChipCap->MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */


	pChipOps->AsicRfTurnOff = RT3593LoadRFSleepModeSetup;
	pChipOps->AsicRfInit = NICInitRT3593RFRegisters;
	pChipOps->AsicReverseRfFromSleepMode = RT3593ReverseRFSleepModeSetup;
	pChipOps->AsicHaltAction = RT3593HaltAction;
	pChipOps->AsicMacInit = NICInitRT3593MacRegisters;
	pChipOps->AsicBbpInit = NICInitRT3593BbpRegisters;
	RtmpChipBcnSpecInit(pAd);
	pChipOps->ChipSwitchChannel = RT3593_ChipSwitchChannel;
	pChipOps->AsicAdjustTxPower = AsicAdjustTxPower;
	pChipOps->RxSensitivityTuning = RT35xx_RxSensitivityTuning;
	pChipOps->ChipBBPAdjust = RT35xx_ChipBBPAdjust;
	pChipOps->AsicTxAlcGetAutoAgcOffset = AsicGetAutoAgcOffsetForExternalTxAlc;
	pChipOps->AsicGetTxPowerOffset = RT3593_AsicGetTxPowerOffset;
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
#ifdef STREAM_MODE_SUPPORT
	pChipCap->FlgHwStreamMode = TRUE;
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	pChipCap->FlgHwTxBfCap = TRUE;
#endif /* TXBF_SUPPORT */
#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */

}
#endif /* RT3593 */

