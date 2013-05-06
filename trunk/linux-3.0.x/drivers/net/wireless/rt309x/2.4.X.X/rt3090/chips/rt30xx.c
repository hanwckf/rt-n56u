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
	rt2880.c

	Abstract:
	Specific funcitons and variables for RT30xx.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#ifdef RT30xx


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //

#include "rt_config.h"


//
// RF register initialization set
//
REG_PAIR   RT3020_RFRegTable[] = {
        {RF_R04,          0x40},
        {RF_R05,          0x03},
        {RF_R06,          0x02},
        {RF_R07,          0x70},      
        {RF_R09,          0x0F},
        {RF_R10,          0x41},
        {RF_R11,          0x21},
        {RF_R12,          0x7B},
        {RF_R14,          0x90},
        {RF_R15,          0x58},
        {RF_R16,          0xB3},
        {RF_R17,          0x92},
        {RF_R18,          0x2C},
        {RF_R19,          0x02},
        {RF_R20,          0xBA},
        {RF_R21,          0xDB},
        {RF_R24,          0x16},      
        {RF_R25,          0x01},
        {RF_R29,          0x1F},
};

UCHAR NUM_RF_3020_REG_PARMS = (sizeof(RT3020_RFRegTable) / sizeof(REG_PAIR));


// Antenna divesity use GPIO3 and EESK pin for control
// Antenna and EEPROM access are both using EESK pin,
// Therefor we should avoid accessing EESK at the same time
// Then restore antenna after EEPROM access
// The original name of this function is AsicSetRxAnt(), now change to 
//VOID AsicSetRxAnt(
VOID RT30xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	UINT32	Value;
#ifdef RTMP_MAC_PCI
	UINT32	x;
#endif // RTMP_MAC_PCI //

	if ((pAd->EepromAccess) ||
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


/*
	========================================================================

	Routine Description:
		For RF filter calibration purpose

	Arguments:
		pAd                          Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID RTMPFilterCalibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR	R55x = 0, value, FilterTarget = 0x1E, BBPValue=0;
	UINT	loop = 0, count = 0, loopcnt = 0, ReTry = 0;
	UCHAR	RF_R24_Value = 0;

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		return;
	}
	else
#endif // RT3593 //
	{
		// Give bbp filter initial value
		pAd->Mlme.CaliBW20RfR24 = 0x1F;
		pAd->Mlme.CaliBW40RfR24 = 0x2F; //Bit[5] must be 1 for BW 40

		do 
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))				
				return;
			if (loop == 1)	//BandWidth = 40 MHz
			{
				// Write 0x27 to RF_R24 to program filter
				RT30xxReadRFRegister(pAd, RF_R24, (PUCHAR)(&RF_R24_Value));
				RF_R24_Value = (RF_R24_Value & 0xC0) | 0x27; // <bit 5>:tx_h20M<bit 5> and <bit 4:0>:tx_agc_fc<bit 4:0>
				RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
				if (IS_RT3071(pAd) || IS_RT3572(pAd))
					FilterTarget = 0x15;
				else
					FilterTarget = 0x19;

				// when calibrate BW40, BBP mask must set to BW40.
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
				BBPValue&= (~0x18);
				BBPValue|= (0x10);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

				// set to BW40
				RT30xxReadRFRegister(pAd, RF_R31, &value);
				value |= 0x20;
				RT30xxWriteRFRegister(pAd, RF_R31, value);
			}
			else			//BandWidth = 20 MHz
			{
				// Write 0x07 to RF_R24 to program filter
				RT30xxReadRFRegister(pAd, RF_R24, (PUCHAR)(&RF_R24_Value));
				RF_R24_Value = (RF_R24_Value & 0xC0) | 0x07; // <bit 5>:tx_h20M<bit 5> and <bit 4:0>:tx_agc_fc<bit 4:0>
				RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
				if (IS_RT3071(pAd) || IS_RT3572(pAd))
					FilterTarget = 0x13;
				else
					FilterTarget = 0x16;

				// set to BW20
				RT30xxReadRFRegister(pAd, RF_R31, &value);
				value &= (~0x20);
				RT30xxWriteRFRegister(pAd, RF_R31, value);
			}

			// Write 0x01 to RF_R22 to enable baseband loopback mode
			RT30xxReadRFRegister(pAd, RF_R22, &value);
			value |= 0x01;
			RT30xxWriteRFRegister(pAd, RF_R22, value);

			// Write 0x00 to BBP_R24 to set power & frequency of passband test tone
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0);

			do
			{
				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))				
					return;
				// Write 0x90 to BBP_R25 to transmit test tone
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x90);

				RTMPusecDelay(1000);
				// Read BBP_R55[6:0] for received power, set R55x = BBP_R55[6:0]
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R55, &value);
				R55x = value & 0xFF;

			} while ((ReTry++ < 100) && (R55x == 0));
			
			// Write 0x06 to BBP_R24 to set power & frequency of stopband test tone
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x06);

			while(TRUE)
			{
				if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))				
					return;
				
				// Write 0x90 to BBP_R25 to transmit test tone
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x90);

				//We need to wait for calibration
				RTMPusecDelay(1000);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R55, &value);
				value &= 0xFF;
				if ((R55x - value) < FilterTarget)
				{
					RF_R24_Value ++;
				}
				else if ((R55x - value) == FilterTarget)
				{
					RF_R24_Value ++;
					count ++;
				}
				else
				{
					break;
				}

				// prevent infinite loop cause driver hang.
				if (loopcnt++ > 100)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("RTMPFilterCalibration - can't find a valid value, loopcnt=%d stop calibrating", loopcnt));
					break;
				}

				// Write RF_R24 to program filter
				RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
			}

			if (count > 0)
			{
				RF_R24_Value = RF_R24_Value - ((count) ? (1) : (0));
			}

			// Store for future usage
			if (loopcnt < 100)
			{
				if (loop++ == 0)
				{
					//BandWidth = 20 MHz
					pAd->Mlme.CaliBW20RfR24 = (UCHAR)RF_R24_Value;
				}
				else
				{
					//BandWidth = 40 MHz
					pAd->Mlme.CaliBW40RfR24 = (UCHAR)RF_R24_Value;
					break;
				}
			}
			else 
				break;

			RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);

			// reset count
			count = 0;
		} while(TRUE);

		//
		// Set back to initial state
		//
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0);

		RT30xxReadRFRegister(pAd, RF_R22, &value);
		value &= ~(0x01);
		RT30xxWriteRFRegister(pAd, RF_R22, value);

		// set BBP back to BW20
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
		BBPValue&= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

		DBGPRINT(RT_DEBUG_TRACE, ("RTMPFilterCalibration - CaliBW20RfR24=0x%x, CaliBW40RfR24=0x%x\n", pAd->Mlme.CaliBW20RfR24, pAd->Mlme.CaliBW40RfR24));
	}
}


// add by johnli, RF power sequence setup
/*
	==========================================================================
	Description:

	Load RF normal operation-mode setup
	
	==========================================================================
 */
VOID RT30xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue, bbpreg = 0;

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		// improve power consumption 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R138, &bbpreg);
		if (pAd->Antenna.field.TxPath == 1)
		{
			// turn off tx DAC_1 & DAC_2
			bbpreg = (bbpreg | 0x60);
		}
		else if (pAd->Antenna.field.TxPath == 2)
		{
			// turn off tx DAC_2
			bbpreg = (bbpreg | 0x40);		
		}

		if (pAd->Antenna.field.RxPath == 1)
		{
			// turn off rx ADC_1 & ADC_2
			bbpreg &= (~0x6);
		}
		else if (pAd->Antenna.field.RxPath == 2)
		{
			// turn off rx ADC_2
			bbpreg &= (~0x4);
		}		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R138, bbpreg);		
	}
	else
#endif // RT3593 //
	{
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
	}

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
VOID RT30xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;
	UINT32 MACValue;


	if(!IS_RT3572(pAd))
	{
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			UCHAR	rfreg;

			RT30xxReadRFRegister(pAd, RF_R01, &rfreg);
			rfreg = ((rfreg & ~0x01) | 0x00); // vco_en
			RT30xxWriteRFRegister(pAd, RF_R01, rfreg);

			RT30xxReadRFRegister(pAd, RF_R06, &rfreg);
			rfreg &= (~0x40); // vco_ic (VCO bias current control, 00: off)
			RT30xxWriteRFRegister(pAd, RF_R06, rfreg);

			RT30xxReadRFRegister(pAd, RF_R22, &rfreg);
			rfreg = ((rfreg & ~0xE0) | 0x00); // cp_ic (reference current control, 000: 0.25 mA)
			RT30xxWriteRFRegister(pAd, RF_R22, rfreg);

			RT30xxReadRFRegister(pAd, RF_R46, &rfreg);
			rfreg &= (~0x20); // rx_ctb_en
			RT30xxWriteRFRegister(pAd, RF_R46, rfreg);

			RT30xxReadRFRegister(pAd, RF_R20, &rfreg);
			rfreg |= 0xEE; // ldo_pll_vc and ldo_rf_vc (111: -0.15)
			RT30xxWriteRFRegister(pAd, RF_R20, rfreg);
		}
		else
#endif // RT3593 //
		{
			// RF_BLOCK_en. RF R1 register Bit 0 to 0
			RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
			RFValue &= (~0x01);
			RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

			// VCO_IC, RF R7 register Bit 4 & Bit 5 to 0
			RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
			RFValue &= (~0x30);
			RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

			// Idoh, RF R9 register Bit 1, Bit 2 & Bit 3 to 0
			RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
			RFValue &= (~0x0E);
			RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

			// RX_CTB_en, RF R21 register Bit 7 to 0
			RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
			RFValue &= (~0x80);
			RT30xxWriteRFRegister(pAd, RF_R21, RFValue);
		}
	}

	// Don't touch LDO_CFG0 for 3090F & 3593, possibly the board is single power scheme
	if (IS_RT3090(pAd) ||	// IS_RT3090 including RT309x and RT3071/72
		IS_RT3572(pAd) ||
		(IS_RT3070(pAd) && ((pAd->MACVersion & 0xffff) < 0x0201)))
	{
		if (!IS_RT3572(pAd))
		{
			RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
			RFValue |= 0x77;
			RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
		}

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
VOID RT30xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;
	UINT32 MACValue;

	if(!IS_RT3572(pAd))
	{
#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			UCHAR	rfreg;

			RT30xxReadRFRegister(pAd, RF_R01, &rfreg);
			rfreg = ((rfreg & ~0x01) | 0x01); // vco_en
			RT30xxWriteRFRegister(pAd, RF_R01, rfreg);

			RT30xxReadRFRegister(pAd, RF_R03, &rfreg);
			rfreg |= 0x80; // vcocal_en (initiate VCO calibration (reset after completion))
			RT30xxWriteRFRegister(pAd, RF_R03, rfreg);

			RT30xxReadRFRegister(pAd, RF_R06, &rfreg);
			rfreg |= 0x40; // vco_ic (VCO bias current control, 11: high)
			RT30xxWriteRFRegister(pAd, RF_R06, rfreg);
			
			RT30xxReadRFRegister(pAd, RF_R02, &rfreg);
			rfreg |= 0x80; // rescal_en (initiate calibration)
			RT30xxWriteRFRegister(pAd, RF_R02, rfreg);

			RT30xxReadRFRegister(pAd, RF_R22, &rfreg);
			rfreg = ((rfreg & ~0xE0) | 0x20); // cp_ic (reference current control, 001: 0.33 mA)
			RT30xxWriteRFRegister(pAd, RF_R22, rfreg);

			RT30xxReadRFRegister(pAd, RF_R46, &rfreg);
			rfreg |= 0x20; // rx_ctb_en
			RT30xxWriteRFRegister(pAd, RF_R46, rfreg);

			RT30xxReadRFRegister(pAd, RF_R20, &rfreg);
			rfreg &= (~0xEE); // ldo_rf_vc and ldo_pll_vc ( 111: +0.15)
			RT30xxWriteRFRegister(pAd, RF_R20, rfreg);
		}
		else
#endif // RT3593 //
		{
			// RF_BLOCK_en, RF R1 register Bit 0 to 1
			RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
			RFValue |= 0x01;
			RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

			// VCO_IC, RF R7 register Bit 4 & Bit 5 to 1
			RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
			RFValue |= 0x30;
			RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

			// Idoh, RF R9 register Bit 1, Bit 2 & Bit 3 to 1
			RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
			RFValue |= 0x0E;
			RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

			// RX_CTB_en, RF R21 register Bit 7 to 1
			RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
			RFValue |= 0x80;
			RT30xxWriteRFRegister(pAd, RF_R21, RFValue);
		}
	}

	if (IS_RT3090(pAd) ||	// IS_RT3090 including RT309x and RT3071/72
		IS_RT3572(pAd) ||
		IS_RT3390(pAd) ||
		IS_RT3593(pAd) ||
		(IS_RT3070(pAd) && ((pAd->MACVersion & 0xffff) < 0x0201)))
	{
		if ((!IS_RT3572(pAd)) && (!IS_RT3593(pAd)))
		{
			RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
			if ((pAd->MACVersion & 0xffff) < 0x0211)
				RFValue = (RFValue & (~0x77)) | 0x3;
			else
				RFValue = (RFValue & (~0x77));
			RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
		}

		// RT3071 version E has fixed this issue
		if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
		{
			// patch tx EVM issue temporarily
			RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
			MACValue = ((MACValue & 0xE0FFFFFF) | 0x0D000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
		}
//		else if ((!IS_RT3090(pAd) && !IS_RT3593(pAd)) || (pAd->CommonCfg.PatchHWControl.field.LDOCfg == 1))
		else if ((!IS_RT3090(pAd) && !IS_RT3593(pAd)))
		{
			RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
			MACValue = ((MACValue & 0xE0FFFFFF) | 0x01000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
		}

	}

	if(IS_RT3572(pAd))
		RT30xxWriteRFRegister(pAd, RF_R08, 0x80);
}
// end johnli

VOID RT30xxHaltAction(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32		TxPinCfg = 0x00050F0F;

	//
	// Turn off LNA_PE or TRSW_POL
	//
	if (IS_RT3070(pAd) || IS_RT3071(pAd) || IS_RT3572(pAd))
	{
		if ((IS_RT3071(pAd) || IS_RT3572(pAd))
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
}

#endif // RT30xx //

