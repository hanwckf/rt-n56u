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
#endif // RTMP_RF_RW_SUPPORT //

#include "rt_config.h"


//
// RF register initialization set
//
#ifdef RT3390
REG_PAIR RF3320_RFRegTable[] = {
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
	{RF_R08,		0x49}, // Read only
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
	{RF_R27,		0x07},
	{RF_R28,		0x60},
	{RF_R29,		0xB8},
	{RF_R30,		0x29},
	{RF_R31,		BW20RFR31},
};
#endif // RT3390 //

UCHAR NUM_RF_3320_REG_PARMS = (sizeof(RF3320_RFRegTable) / sizeof(REG_PAIR));


// Antenna divesity use GPIO3 and EESK pin for control
// Antenna and EEPROM access are both using EESK pin,
// Therefor we should avoid accessing EESK at the same time
// Then restore antenna after EEPROM access
// The original name of this function is AsicSetRxAnt(), now change to 
//VOID AsicSetRxAnt(

VOID RT33xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	UINT32	Value;
	UINT32	x;

	if (//(!pAd->NicConfig2.field.AntDiversity) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		return;
	}

	// the antenna selection is through firmware and MAC register(GPIO3)
	if (Ant == 0)
	{
		// Main antenna
		// E2PROM_CSR only in PCI bus Reg., USB Bus need MCU commad to control the EESK pin.
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x |= (EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
#endif // RTMP_MAC_PCI //

		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to main antenna\n"));
	}
	else
	{
		// Aux antenna
		// E2PROM_CSR only in PCI bus Reg., USB Bus need MCU commad to control the EESK pin.
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x &= ~(EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
#endif // RTMP_MAC_PCI //
		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		Value |= 0x08;
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to aux antenna\n"));
	}
}



// add by johnli, RF power sequence setup
/*
	==========================================================================
	Description:

	Load RF normal operation-mode setup
	
	==========================================================================
 */
VOID RT33xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue, bbpreg = 0;

	// improve power consumption 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R138, &bbpreg);
	if (pAd->Antenna.field.TxPath == 1)
	{
		// turn off tx DAC_1
		bbpreg = (bbpreg | 0x20);
	}

	if (pAd->Antenna.field.RxPath == 1)
	{
		// turn off tx ADC_1
		bbpreg &= (~0x2);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R138, bbpreg);

	// RX0_PD & TX0_PD, RF R1 register Bit 2 & Bit 3 to 0 and RF_BLOCK_en,RX1_PD & TX1_PD, Bit0, Bit 4 & Bit5 to 1
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue = (RFValue & (~0x0C)) | 0x31;
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	// TX_LO2_en, RF R15 register Bit 3 to 0
	RT30xxReadRFRegister(pAd, RF_R15, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R15, RFValue);

	// TX_LO1_en, RF R17 register Bit 3 to 0
	RT30xxReadRFRegister(pAd, RF_R17, &RFValue);
	RFValue &= (~0x08);
	// to fix rx long range issue
	if (((pAd->MACVersion & 0xffff) >= 0x0211) && (pAd->NicConfig2.field.ExternalLNAForG == 0))
	{
		RFValue |= 0x20;
	}
	// set RF_R17_bit[2:0] equal to EEPROM setting at 0x48h
	if (pAd->TxMixerGain24G >= 2)
	{
		RFValue &= (~0x7);  // clean bit [2:0]
		RFValue |= pAd->TxMixerGain24G;
	}
	RT30xxWriteRFRegister(pAd, RF_R17, RFValue);

	// RX_LO1_en, RF R20 register Bit 3 to 0
	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

	// RX_LO2_en, RF R21 register Bit 3 to 0
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
	UINT32 MACValue;

		// RF_BLOCK_en. RF R1 register Bit 0 to 0
		RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
		RFValue &= (~0x01);
		RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

		// VCO_IC, RF R7 register Bit 4 & Bit 5 to 0
		RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
		RFValue &= (~0x30);
		RT30xxWriteRFRegister(pAd, RF_R07, RFValue);


		// RX_CTB_en, RF R21 register Bit 7 to 0
		RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
		RFValue &= (~0x80);
		RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

	if (IS_RT3390(pAd))
	{
		RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
		MACValue |= 0x1D000000;
		RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
	}
}

/*
	==========================================================================
	Description:

	Reverse RF sleep-mode setup
	
	==========================================================================
 */
VOID RT33xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;
	UINT32 MACValue;

		// RF_BLOCK_en, RF R1 register Bit 0 to 1
		RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
		RFValue |= 0x01;
		RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

		// VCO_IC, RF R7 register Bit 4 & Bit 5 to 1
		RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
		/* According to HK's comment for Max Input power issue.
		    RF 07 must set to 0x60. */
		RFValue |= 0x20; /* 0x30. */
		RT30xxWriteRFRegister(pAd, RF_R07, RFValue);


		// RX_CTB_en, RF R21 register Bit 7 to 1
		RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
		RFValue |= 0x80;
		RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

	if (IS_RT3390(pAd))
		{
		// RT3071 version E has fixed this issue
		if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
		{
			// patch tx EVM issue temporarily
			RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
			MACValue = ((MACValue & 0xE0FFFFFF) | 0x0D000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
		}
		else
		{
			RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
			MACValue = ((MACValue & 0xE0FFFFFF) | 0x01000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
		}
	}
}
// end johnli

VOID RT33xxHaltAction(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32		TxPinCfg = 0x00050F0F;

	//
	// Turn off LNA_PE or TRSW_POL
	//
        // Fixed suspend leakage current
                // According to MAC 0x0580 bit [31], set MAC 0x1328 bit[18] during suspend mode.
                // If SEL_EFUSE=0, set TRSW_POL=0 in suspend mode.
                // If SEL_EFUSE=1, set TRSW_POL=1 in suspend mode.
	
		if (IS_RT3390(pAd)
#ifdef RTMP_EFUSE_SUPPORT
			&& (pAd->bUseEfuse)
#endif // RTMP_EFUSE_SUPPORT //
			)
		{
			TxPinCfg &= 0xFFFBF0F0; // bit18 off
		}
		else
		{
			TxPinCfg &= 0xFFFFF0F0;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);   
	}

#endif // RT33xx //

