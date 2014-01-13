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
	Specific funcitons and variables for RT3883

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3883

#include	"rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //

UCHAR EeBuffer[EEPROM_SIZE] = {
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

REG_PAIR   RT3883_RFRegTable[] = {
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
	{RF_R10, 0xD3}, // Gary, 2010-02-12
	{RF_R11, 0x48},
	{RF_R12, 0x1A},	/* Gary, 2011-03-10 */
	{RF_R13, 0x12},
	{RF_R14, 0x00},
	{RF_R15, 0x00},
	{RF_R16, 0x00},
/*	{RF_R17, 0x26}, By EEPROM Frequency offset */
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
	{RF_R38, 0x86},	// Gary, 2010-02-12
	{RF_R39, 0x23},
	{RF_R40, 0x00},
	{RF_R41, 0x00},
	{RF_R42, 0x00},
	{RF_R43, 0x00},
	{RF_R44, 0x93},	// Gary, 2009-12-08
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

UCHAR RT3883_NUM_RF_REG_PARMS = (sizeof(RT3883_RFRegTable) / sizeof(REG_PAIR));

REG_PAIR   RT3883_BBPRegTable[] = {
	{BBP_R4,		0x50},  /* 3883 need to */
	{BBP_R47,		0x48},  /* ALC Functions change from 0x7 to 0x48 Baron suggest */

	{BBP_R86,		0x46},  /* for peak throughput, Henry 2009-12-23 */
	{BBP_R88,		0x90},  /* for rt3883 middle range, Henry 2009-12-31 */

	{BBP_R92,		0x02},  /* middle range issue, Rory @2008-01-28 */

	{BBP_R103,		0xC0},
	{BBP_R104,		0x92},
	{BBP_R105,		0x34},
#ifdef DOT11_N_SUPPORT
	{BBP_R106,		0x12},  /* 40M=2, 20M=2. Fix 20M SGI STBC problem */
#endif /* DOT11_N_SUPPORT */
	{BBP_R120,		0x50},  /* for long range -2db, Gary 2010-01-22 */
	{BBP_R137,		0x0F},  /* julian suggest make the RF output more stable */
	{BBP_R163,		0x9D},  /* Enable TxBf modes by default, Gary, 2010-06-02 */

	{BBP_R179,		0x02},  /* Set ITxBF timeout to 0x9C40=1000msec */
	{BBP_R180,		0x00},
	{BBP_R182,		0x40},
	{BBP_R180,		0x01},
	{BBP_R182,		0x9C},

	{BBP_R179,		0x00},

	{BBP_R142,		0x04},  /* Reprogram the inband interface to put right values in RXWI */
	{BBP_R143,		0x3b},
	{BBP_R142,		0x06},
	{BBP_R143,		0xA0},
	{BBP_R142,		0x07},
	{BBP_R143,		0xA1},
	{BBP_R142,		0x08},
	{BBP_R143,		0xA2},
	{BBP_R148,		0xC8},  /* Gary, 2010-2-12 */
};

UCHAR RT3883_NUM_BBP_REG_PARMS = (sizeof(RT3883_BBPRegTable) / sizeof(REG_PAIR));

VOID NICInitRT3883RFRegisters(IN PRTMP_ADAPTER pAd)
{

	// Init RF calibration
	// Driver should toggle RF R02 bit7 before init RF registers
	UCHAR RfReg = 0;
	int i;

	// Initialize RF register to default value
	for (i = 0; i < RT3883_NUM_RF_REG_PARMS; i++)
	{
		RT30xxWriteRFRegister(pAd, RT3883_RFRegTable[i].Register, RT3883_RFRegTable[i].Value);
	}
	if ((pAd->CommonCfg.CID & 0x0000000f) >= 0x00000005)
	{
		RT30xxWriteRFRegister(pAd, RF_R32, 0xD8);
		RT30xxWriteRFRegister(pAd, RF_R33, 0x3B);
	}

	//RF_R02: Resistor calibration, RF_R02 = RF_R30 (RT30xx)
	RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
	RfReg &= ~(1 << 6); // clear bit6=rescal_bp
	RfReg |= 0x80; // bit7=rescal_en
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
	//DBGPRINT(RT_DEBUG_WARN,("SS \n"));
	RTMPusecDelay(1000);
	RfReg &= 0x7F;
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);     

}

/*
	==========================================================================
	Description:

	Load RF sleep-mode setup
	
	==========================================================================
 */
VOID RT3883LoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;

	// RF_BLOCK_en. RF R1 register Bit 0 to 0
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue &= (~0x01);
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	// VCO_IC, R6[7:6]=[11]
	RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
	RFValue &= (~0xC0);
	RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

	// charge pump current control (cp_ic) RF R22[7:5] = [111]
	RT30xxReadRFRegister(pAd, RF_R22, &RFValue);
	RFValue &= (~0x20);
	RT30xxWriteRFRegister(pAd, RF_R22, RFValue);

	// RX_CTB_en, RF R46[5]=[1]
	RT30xxReadRFRegister(pAd, RF_R46, &RFValue);
	RFValue &= (~0x20);
	RT30xxWriteRFRegister(pAd, RF_R46, RFValue);

	// LDO pll_vc RF_R20[3:1] = [000]
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
VOID RT3883ReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;

	// RF_BLOCK_en, RF R1 register Bit 0 to 1
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue |= 0x01;
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	// VCO_IC, R6[7:6]=[11]
	RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
	RFValue |= 0xC0;
	RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

	// charge pump current control (cp_ic) RF R22[7:5] = [111]
	RT30xxReadRFRegister(pAd, RF_R22, &RFValue);
	RFValue |= 0x20;
	RT30xxWriteRFRegister(pAd, RF_R22, RFValue);

	// RX_CTB_en, RF R46[5]=[1]
	RT30xxReadRFRegister(pAd, RF_R46, &RFValue);
	RFValue |= 0x20;
	RT30xxWriteRFRegister(pAd, RF_R46, RFValue);

	// LDO pll_vc RF_R20[3:1] = [000]
	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue = (RFValue & (~0xEE));
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

	// VCO tuning code readback RF_R03[7]=1
	//RT30xxWriteRFRegister(pAd, RF_R08, 0x80);
}
// end johnli

VOID RT3883HaltAction(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32		TxPinCfg = 0x00050F0F;

	//
	// Turn off LNA_PE or TRSW_POL
	//
#ifdef RTMP_EFUSE_SUPPORT
	if (pAd->bUseEfuse)
	{
		TxPinCfg &= 0xFFFBF0F0; // bit18 off
	}
	else
#endif // RTMP_EFUSE_SUPPORT //
	{
		TxPinCfg &= 0xFFFFF0F0;
	}
	//RT30xxWriteRFRegister(pAd, RF_R08, (UCHAR)0x00);

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
VOID	RTMPRT3883ReadTxPwrPerRate(
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
VOID	RTMPRT3883ReadChannelPwr(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR				i, choffset;
	EEPROM_TX_PWR_STRUC	    	Power;
	EEPROM_TX_PWR_STRUC	    	Power2;
	EEPROM_TX_PWR_STRUC	    	Power3;

	// Read Tx power value for all channels
	// Value from 1 - 0x7f. Default value is 24.
	// Power value : 2.4G 0x00 (0) ~ 0x1F (31)
	//             : 5.5G 0x00 (0) ~ 0x1F (31)
	
	// 0. 11b/g, ch1 - ch 14
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
	
	// 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)
	// 1.1 Fill up channel
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

	// 1.2 Fill up power
	for (i = 0; i < 6; i++)
	{
//		Power.word = RTMP_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2);
//		Power2.word = RTMP_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2);
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
	
	// 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz)
	// 2.1 Fill up channel
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

	// 2.2 Fill up power
	for (i = 0; i < 8; i++)
	{
//		Power.word = RTMP_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2);
//		Power2.word = RTMP_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2);
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

	// 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165 (including central frequency in BW 40MHz)
	// 3.1 Fill up channel
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

	// 3.2 Fill up power
	for (i = 0; i < 4; i++)
	{
//		Power.word = RTMP_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2);
//		Power2.word = RTMP_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2);
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

	// 4. Print and Debug
	choffset = 14 + 12 + 16 + 7;
#ifdef RELASE_EXCLUDE
	for (i = 0; i < choffset; i++)
	{
		DBGPRINT(RT_DEBUG_INFO, ("E2PROM: TxPower[%03d], Channel = %d, Power = %d, Power2 = %d, Power3 = %d\n", i, pAd->TxPower[i].Channel, pAd->TxPower[i].Power, pAd->TxPower[i].Power2, pAd->TxPower[i].Power3 ));
	}
#endif // RELASE_EXCLUDE //
}

// ATE will also call this function to set GPIO, channel=36 to set low and channel=1 to set high
VOID	RTMPRT3883ABandSel(
	IN	UCHAR	Channel)
{
#ifdef BOARD_EXT_SWITCH_LNA_2DOT4_5
	UINT32 value;

	//set GPIO2(gpio#25) to GPIO mode
	value = le32_to_cpu(*(volatile u32 *)(0xb0000014));
	value &= ~(1 << 2);
	*((volatile uint32_t *)(0xb0000014)) = cpu_to_le32(value);

	//config gpio#25 direction to output
	value = le32_to_cpu(*(volatile u32 *)(0xb000064c));
	value |= (1 << 1);
	*((volatile uint32_t *)(0xb000064c)) = cpu_to_le32(value);

	if (Channel > 14)
	{
		//5G band: clear gpio#25 to 0
		*((volatile uint32_t *)(0xb0000658)) = cpu_to_le32(0x2);
	}
	else
	{
		//2.4G band: set gpio#25 to 1
		*((volatile uint32_t *)(0xb0000654)) = cpu_to_le32(0x2);
	}
#endif
}

#endif // RT3883 //

