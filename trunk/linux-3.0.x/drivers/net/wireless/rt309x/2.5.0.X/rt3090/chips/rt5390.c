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
	rt5390.c

	Abstract:
	Specific funcitons and variables for RT3090

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT53xx

#include "rt_config.h"


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //
#ifdef RTMP_FLASH_SUPPORT
#endif // RTMP_FLASH_SUPPORT //

REG_PAIR RF5390RegTable[] = 
{
//	{RF_R00,		0x20},  //Read only
	{RF_R01,		0x0F}, 
	{RF_R02,		0x80}, 
	{RF_R03,		0x88}, // vcocal_double_step 
//	{RF_R04,		0x51}, // Read only
	{RF_R05,		0x10}, 
	{RF_R06,		0xA0}, 
	{RF_R07,		0x00}, 
//	{RF_R08,		0xF1}, // By channel plan
//	{RF_R09,		0x02}, // By channel plan

	{RF_R10,		0x53},
	{RF_R11,		0x4A},
	{RF_R12,		0x46},
	{RF_R13,		0x9F},
	{RF_R14,		0x00}, 
	{RF_R15,		0x00}, 
	{RF_R16,		0x00}, 
//	{RF_R17,		0x00}, // Bit 7=0, and based on the frequency offset in the EEPROM
	{RF_R18,		0x03}, 
	{RF_R19,		0x00}, // Spare

	{RF_R20,		0x00}, 
	{RF_R21,		0x00}, // Spare
	{RF_R22,		0x20},	
	{RF_R23,		0x00}, // Spare
	{RF_R24,		0x00}, // Spare
	{RF_R25,		0xC0}, 
	{RF_R26,		0x00}, // Spare
	{RF_R27,		0x09}, 
	{RF_R28,		0x00}, 
	{RF_R29,		0x10}, 

	{RF_R30,		0x10},
	{RF_R31,		0x80}, 
	{RF_R32,		0x80}, 
	{RF_R33,		0x00}, // Spare
	{RF_R34,		0x07}, 
	{RF_R35,		0x12}, 
	{RF_R36,		0x00}, 
	{RF_R37,		0x08}, 
	{RF_R38,		0x85}, 
	{RF_R39,		0x1B}, 

	{RF_R40,		0x0B}, 
	{RF_R41,		0xBB}, 
	{RF_R42,		0xD2}, 
	{RF_R43,		0x9A}, 
	{RF_R44,		0x0E},
	{RF_R45,		0xA2}, 
	{RF_R46,		0x7B}, 
	{RF_R47,		0x00}, 
	{RF_R48,		0x10}, 
	{RF_R49,		0x94},

//	{RF_R50,		0x00}, // NC
//	{RF_R51,		0x00}, // NC
	{RF_R52,		0x38}, 
	{RF_R53,		0x84}, //RT5370 only. RT5390, RT5370F and RT5390F will re-write to 0x00
	{RF_R54,		0x78},
	{RF_R55,		0x44}, /* Changed by channel */
	{RF_R56,		0x22}, 
	{RF_R57,		0x80},
	{RF_R58,		0x7F}, 
	{RF_R59,		0x8F}, /* Changed by channel */

	{RF_R60,		0x45}, 
	{RF_R61,		0xDD}, /* 20111207 update */
	{RF_R62,		0x00}, // Spare
	{RF_R63,		0x00}, // Spare
};

#define NUM_RF_5390_REG_PARMS (sizeof(RF5390RegTable) / sizeof(REG_PAIR))

REG_PAIR RF5392RegTable[] = 
{
//	{RF_R00,		0x20}, // Read only
	{RF_R01,		0x17}, /* 20111110 update */
//	{RF_R02,		0x80}, /* Removed since 20111229 update */
	{RF_R03,		0x88}, // vcocal_double_step
//	{RF_R04,		0x51}, // Read only
	{RF_R05,		0x10}, 
	{RF_R06,		0xE0}, //20101018 update.
	{RF_R07,		0x00}, 
//	{RF_R08,		0xF1}, // By channel plan
//	{RF_R09,		0x02}, // By channel plan

	{RF_R10,		0x53},
	{RF_R11,		0x4A},
	{RF_R12,		0x46},
	{RF_R13,		0x9F},
	{RF_R14,		0x00}, 
	{RF_R15,		0x00}, 
	{RF_R16,		0x00}, 
//	{RF_R17,		0x00}, // Based on the frequency offset in the EEPROM
	{RF_R18,		0x03}, 
	{RF_R19,		0x4D}, // Spare

	{RF_R20,		0x00}, 
	{RF_R21,		0x8D}, // Spare 20101018 update.
	{RF_R22,		0x20},	
	{RF_R23,		0x0B}, // Spare 20101018 update.
	{RF_R24,		0x44}, // Spare
	{RF_R25,		0x80}, // 20101018 update.
	{RF_R26,		0x82}, // Spare
	{RF_R27,		0x09}, 
	{RF_R28,		0x00}, 
	{RF_R29,		0x10}, 

	{RF_R30,		0x10},
	{RF_R31,		0x80}, 
	{RF_R32,		0x20}, //20110620 update
	{RF_R33,		0xC0}, // Spare
	{RF_R34,		0x07}, 
	{RF_R35,		0x12}, 
	{RF_R36,		0x00}, 
	{RF_R37,		0x08}, 
	{RF_R38,		0x89}, //20101118 update.
	{RF_R39,		0x1B}, 

	{RF_R40,		0x0F}, //20101118 update.
	{RF_R41,		0xBB}, 
	{RF_R42,		0xD5}, // 20101018 update.
	{RF_R43,		0x9B}, // 20101018 update.
	{RF_R44,		0x0E},
	{RF_R45,		0xA2}, 
	{RF_R46,		0x73}, 
	{RF_R47,		0x0C}, 
	{RF_R48,		0x10}, 
	{RF_R49,		0x94},

	{RF_R50,		0x94}, //5392_todo
	{RF_R51,		0x3A}, // 20101018 update.
	{RF_R52,		0x48}, // 20101018 update.
	{RF_R53,		0x44}, // 20101018 update.
	{RF_R54,		0x38},
	{RF_R55,		0x43},
	{RF_R56,		0xA1}, // 20101018 update.
	{RF_R57,		0x00}, // 20101018 update.
	{RF_R58,		0x39}, 
	{RF_R59,		0x07}, // 20101018 update.

	{RF_R60,		0x45}, // 20101018 update.
	{RF_R61,		0x91}, // 20101018 update.
	{RF_R62,		0x39}, // Spare
	{RF_R63,		0x07}, /* 20111110 update */
};

#define NUM_RF_5392_REG_PARMS (sizeof(RF5392RegTable) / sizeof(REG_PAIR))


VOID NICInitRT5390RFRegisters(IN PRTMP_ADAPTER pAd)
{
	INT 		i;
	ULONG	data;
	UCHAR	RfReg = 0;
	
	/* Init RF calibration */
	/* Driver should toggle RF R30 bit7 before init RF registers */
	if (!IS_RT5392(pAd))
	{		
		RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
		RfReg = ((RfReg & ~0x80) | 0x80); /* rescal_en (initiate calbration) */
		RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);

		RTMPusecDelay(1000);
		
		RfReg = ((RfReg & ~0x80) | 0x00); /* rescal_en (initiate calbration) */
		RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: Initialize the RF registers to the default values", __FUNCTION__));
		
	/* Initialize RF register to default value */
	if (IS_RT5392(pAd))
	{
		/* Initialize RF register to default value */
		for (i = 0; i < NUM_RF_5392_REG_PARMS; i++)
		{
#ifdef RT5370 /* For RT5372 */
			if (RF5392RegTable[i].Register == RF_R23)
			{
				RF5392RegTable[i].Value = 0x0f;
			}
			else if (RF5392RegTable[i].Register == RF_R24)
			{
				RF5392RegTable[i].Value = 0x3e;
			}
			else if (RF5392RegTable[i].Register == RF_R51)
			{
				RF5392RegTable[i].Value = 0x32;
			}
			else if (RF5392RegTable[i].Register == RF_R53)
			{
				RF5392RegTable[i].Value = 0x22;
			}
			else if (RF5392RegTable[i].Register == RF_R56)
			{
				RF5392RegTable[i].Value = 0xc1;
			}
			else if (RF5392RegTable[i].Register == RF_R59)
			{
				RF5392RegTable[i].Value = 0x0f; 
			}
#endif /* RT5370 */	
#ifdef RT5390 /* For RT5392 */
			if (IS_RT5392C(pAd)) /* >= RT5392C */
			{
				if (RF5392RegTable[i].Register == RF_R01)
				{
					RF5392RegTable[i].Value = 0x17;
				}
				else if (RF5392RegTable[i].Register == RF_R06)
				{
					RF5392RegTable[i].Value = 0xE0;
				}
				else if (RF5392RegTable[i].Register == RF_R21)
				{
					RF5392RegTable[i].Value = 0x8D;
				}
				else if (RF5392RegTable[i].Register == RF_R23)
				{
					RF5392RegTable[i].Value = 0x0B;
				}
				else if (RF5392RegTable[i].Register == RF_R25)
				{
					RF5392RegTable[i].Value = 0x80;
				}
				else if (RF5392RegTable[i].Register == RF_R42)
				{
					RF5392RegTable[i].Value = 0xD5;
				}
				else if (RF5392RegTable[i].Register == RF_R43)
				{
					RF5392RegTable[i].Value = 0x9B;
				}
				else if (RF5392RegTable[i].Register == RF_R51)
				{
					RF5392RegTable[i].Value = 0x3A;
				}
				else if (RF5392RegTable[i].Register == RF_R52)
				{
					RF5392RegTable[i].Value = 0x48;
				}
				else if (RF5392RegTable[i].Register == RF_R53)
				{
					RF5392RegTable[i].Value = 0x44;
				}
				else if (RF5392RegTable[i].Register == RF_R57)
				{
					RF5392RegTable[i].Value = 0x00;
				}
				else if (RF5392RegTable[i].Register == RF_R59)
				{
					RF5392RegTable[i].Value = 0x07;
				}
				else if (RF5392RegTable[i].Register == RF_R60)
				{
					RF5392RegTable[i].Value = 0x45;
				}
				else if (RF5392RegTable[i].Register == RF_R61)
				{
					RF5392RegTable[i].Value = 0x91;
				}
			}
#endif /* RT5390 */	

				RT30xxWriteRFRegister(pAd, RF5392RegTable[i].Register, RF5392RegTable[i].Value);
		}
	}
	else
	{
		/* Initialize RF register to default value */
		for (i = 0; i < NUM_RF_5390_REG_PARMS; i++)
		{
#ifdef RT5390 /* For RT5390 */
			if ((IS_RT5390F(pAd) || IS_RT5390H(pAd)) && (RF5390RegTable[i].Register == RF_R06))
			{
				RF5390RegTable[i].Value = 0xE0;
			}
			else if ((IS_RT5390F(pAd) || IS_RT5390H(pAd) || IS_RT5390(pAd)) && (RF5390RegTable[i].Register == RF_R25))
			{
				RF5390RegTable[i].Value = 0x80;
			}
			else if ((IS_RT5390F(pAd) || IS_RT5390H(pAd)) && (RF5390RegTable[i].Register == RF_R46))
			{
				RF5390RegTable[i].Value = 0x73;
			}
			else if ((IS_RT5390F(pAd) || IS_RT5390H(pAd) || IS_RT5390(pAd)) && (RF5390RegTable[i].Register == RF_R53)) 
			{
				RF5390RegTable[i].Value = 0x00;
			}
			else if ((IS_RT5390F(pAd) || IS_RT5390H(pAd)) && (RF5390RegTable[i].Register == RF_R61))
			{
				RF5390RegTable[i].Value = 0xD1;
			}
#endif /* RT5390 */
#ifdef RT5370 /* For RT5370 */
			if (IS_RT5390F(pAd) && (RF5390RegTable[i].Register == RF_R06))
			{
				RF5390RegTable[i].Value = 0xE0;
			}
			else if (IS_RT5390F(pAd) && (RF5390RegTable[i].Register == RF_R25))
			{
				RF5390RegTable[i].Value = 0x80;
			}
			else if (IS_RT5390F(pAd) && (RF5390RegTable[i].Register == RF_R40))
			{
				RF5390RegTable[i].Value = 0x0B;
			}
			else if (IS_RT5390F(pAd) && (RF5390RegTable[i].Register == RF_R46))
			{
				RF5390RegTable[i].Value = 0x73;
			}
			else if (IS_RT5390F(pAd) && (RF5390RegTable[i].Register == RF_R53))
			{
				RF5390RegTable[i].Value = 0x00;
			}
			else if (IS_RT5390F(pAd) && (RF5390RegTable[i].Register == RF_R56))
			{
				RF5390RegTable[i].Value = 0x42;
			}
			else if (IS_RT5390F(pAd) && (RF5390RegTable[i].Register == RF_R61))
			{
				RF5390RegTable[i].Value = 0xD1;
			}
#endif /* RT5370 */

			RT30xxWriteRFRegister(pAd, RF5390RegTable[i].Register, RF5390RegTable[i].Value);
		}
	}

	if (IS_RT5392(pAd))
	{			
		RT30xxWriteRFRegister(pAd, RF_R02, 0x80);
	}
	
	// Where to add the following codes?
	//
	// RT5390BC8, Disable RF_R40 bit[6] to save power consumption

	if (pAd->NicConfig2.field.CoexBit == TRUE)
	{
		RT30xxReadRFRegister(pAd, RF_R40, (PUCHAR)&RfReg);
		RfReg &= (~0x40);
		RT30xxWriteRFRegister(pAd, RF_R40, (UCHAR)RfReg);
	}
	
	// Give bbp filter initial value   Moved here from RTMPFilterCalibration( )
	pAd->Mlme.CaliBW20RfR24 = 0x1F;
	pAd->Mlme.CaliBW40RfR24 = 0x2F; //Bit[5] must be 1 for BW 40
	//For RF filter Calibration
	//RTMPFilterCalibration(pAd);

	// Initialize RF R27 register, set RF R27 must be behind RTMPFilterCalibration()
	if ((pAd->MACVersion & 0xffff) < 0x0211)
		RT30xxWriteRFRegister(pAd, RF_R27, 0x3);

	// set led open drain enable
	RTMP_IO_READ32(pAd, OPT_14, &data);
	data |= 0x01;
	RTMP_IO_WRITE32(pAd, OPT_14, data);

	RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);
	RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0);

	// set default antenna as main
	AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);

	// patch RSSI inaccurate issue, due to design change
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R79, 0x13);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R80, 0x05);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R81, 0x33);

	// enable DC filter
	if ((pAd->MACVersion & 0xffff) >= 0x0211)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xc0);
	}
	
	// From RT3071 Power Sequence v1.1 document, the Normal Operation Setting Registers as follow :
	// BBP_R138 / RF_R1 / RF_R15 / RF_R17 / RF_R20 / RF_R21.
	// add by johnli, RF power sequence setup, load RF normal operation-mode setup
	RT30xxLoadRFNormalModeSetup(pAd);
	
	// adjust some BBP register contents
	// also can put these BBP registers to pBBPRegTable
	RT5390_PostBBPInitialization(pAd);
}

//
// Post-process the BBP registers based on the chip model
//
// Parameters
//	pAd: The adapter data structure
//
// Return Value
//	None
//
 VOID RT5390_PostBBPInitialization(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BbpReg = 0;
	BBP_R105_STRUC BBPR105 = { { 0 } };
//	BBP_R106_STRUC BBPR106 = { { 0 } };
	
	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	//
	// The channel estimation updates based on remodulation of L-SIG and HT-SIG symbols.
	//
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R105, &BBPR105.byte);

	//
	// Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)
	//
	{
	if (pAd->Antenna.field.RxPath == 1) // Single RX
	{
		BBPR105.field.MLDFor2Stream = 0;
	}
	else
	{
		BBPR105.field.MLDFor2Stream = 1;
	}
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, BBPR105.byte);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: BBP_R105: BBPR105.field.EnableSIGRemodulation = %d, BBPR105.field.MLDFor2Stream = %d\n", 
		__FUNCTION__, 
		BBPR105.field.EnableSIGRemodulation, 
		BBPR105.field.MLDFor2Stream));

	{
		//
		// Avoid data lost and CRC error
		//
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpReg);
		BbpReg = ((BbpReg & ~0x40) | 0x40); // MAC interface control (MAC_IF_80M, 1: 80 MHz)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpReg);
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, 0x0B); // Rx AGC energy lower bound in log2
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R77, 0x59); // Rx high/medium power threshold in log2
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62); // Rx AGC LNA select threshold in log2
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x7A); // Rx AGC LNA MM select threshold in log2
		if (IS_RT5392(pAd))
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x9A); // Rx AGC VGA/LNA delay
		else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x19); // Rx AGC VGA/LNA delay
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38); // Rx AGC high gain threshold in dB

		if (IS_RT5392(pAd))
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R88, 0x90); // 2011-0503, add this register, by Gary's comment

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R91, 0x04); // Guard interval delay counter for 20M band
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R92, 0x02); // Guard interval delay counter for 40M band
#ifdef RT53xx
		if (IS_RT5392(pAd))
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, 0x9A); //CCK MRC decode
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R98, 0x12); //TX CCK higher gain 
		}
#endif // RT53xx //
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xC0); // Rx - 11b adaptive equalizer gear down control and signal energy average period
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R104, 0x92); // SIGN detection threshold/GF CDD control
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x3C); // FEQ control
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R128, 0x12); // R/W remodulation control
		if (IS_RT5392(pAd))
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, 0x12); // GI remover, 2011-0503, from 0x05 to 0x12
		else			
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, 0x03); // GI remover
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, 0x08); // ADC/DAC control
		
		if (IS_RT5392(pAd))
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R134, 0xD0); //TX CCK higher gain 
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R135, 0xF6); //TX CCK higher gain 
		}
		
#ifdef RT5390
//KH:?????
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x4E); // Rx high power VGA offset for LNA offset
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R76, 0x30); // Rx medium power VGA offset for LNA offset
		}
#endif // RT5390 //

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46); // Rx high power VGA offset for LNA offset
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R76, 0x28); // Rx medium power VGA offset for LNA offset
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A); // Rx AGC SQ CCK Xcorr threshold
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x13); // Rx AGC SQ ACorr threshold

	}
	//KH Notice:Ian codes has the following part, but Zero remove it. Why?
#ifdef RT5390
#endif // RT5390 //	
	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
} /* End of RT5390_PostBBPInitialization */

/*
	========================================================================
	
	Routine Description: 5392 R66 writing must select BBP_R27

	Arguments:

	Return Value:

	IRQL = 
	
	Note: This function copy from RT3572WriteBBPR66. The content almost the same.
	
	========================================================================
*/
NTSTATUS	RT5392WriteBBPR66(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Value)
{
	NTSTATUS NStatus = STATUS_UNSUCCESSFUL;
	UCHAR	bbpData = 0;

	if (!IS_RT5392(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect MAC version, pAd->MACVersion = 0x%X\n", 
			__FUNCTION__, 
			pAd->MACVersion));
		return NStatus;
	}
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &bbpData);

	// R66 controls the gain of Rx0
	bbpData &= ~(0x60);	//clear bit 5,6
	{
#ifdef RTMP_MAC_PCI
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpData);
#endif // RTMP_MAC_PCI //
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Value);
	}

	// R66 controls the gain of Rx1
	bbpData |= 0x20;		// set bit 5
	{
#ifdef RTMP_MAC_PCI
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, bbpData);
#endif // RTMP_MAC_PCI //
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Value);
		NStatus = STATUS_SUCCESS;
	}
	return NStatus;
}

#endif // RT53xx //

/*
 Antenna divesity use GPIO3 and EESK pin for control
 Antenna and EEPROM access are both using EESK pin,
 Therefor we should avoid accessing EESK at the same time
 Then restore antenna after EEPROM access
 The original name of this function is AsicSetRxAnt(), now change to 
*/
VOID RT5390SetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	UINT32	Value;
#ifdef RTMP_MAC_PCI
	UINT32	x;
#endif /* RTMP_MAC_PCI */

	if (/*(!pAd->NicConfig2.field.AntDiversity) || */
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		return;
	}

	if ((IS_RT5390(pAd)) && (!IS_RT5392(pAd))
		)
	{
		UCHAR BbpValue = 0;

#ifdef TXRX_SW_ANTDIV_SUPPORT
        /* EEPROM 34h bit 13 = 1, support SW antenna diverity TX/RX boundle switch */

        if (pAd->chipCap.bTxRxSwAntDiv) /* Mini card with TX/RX Diversity (RT5390U) & USB with TX/RX Diversity (RT5370) */
        {
            RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpValue);
            BbpValue = (BbpValue  | 0x80); /* MSB =1 , TX/RX is the same path */
            RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpValue);


            if (Ant == 0) /* 0: Main antenna */
            {
                RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
                DBGPRINT(RT_DEBUG_ERROR, ("AsicSetRxTxAnt, before switch to main antenna(%X)\n", Value));
                Value &= ~(0x04000808); /* GPIO3 = 0, GIPO10 = 1 */
                Value |= (0x00040000);
                RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
                DBGPRINT(RT_DEBUG_ERROR, ("AsicSetRxTxAnt, after switch to main antenna(%X)\n", Value));
            }
            else if (Ant == 1) /* 1: Aux. antenna */
            {
                RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
                DBGPRINT(RT_DEBUG_ERROR, ("AsicSetRxTxAnt, before switch to aux antenna(%X)\n", Value));
                Value &= ~(0x04040800); /* GPIO3 = 1, GIPO10 = 0 */
                Value |= (0x8);
                RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
                DBGPRINT(RT_DEBUG_ERROR, ("AsicSetRxTxAnt, after switch to aux antenna(%X)\n", Value));
            }
        }else
#endif  /* TXRX_SW_ANTDIV_SUPPORT */

		if (IS_RT5370G(pAd) || IS_RT5390R(pAd)) /*PPAD support */
		{
    		/* For PPAD Debug, BBP R153[7] = 1 --> Main Ant, R153[7] = 0 --> Aux Ant */
			if (Ant == 0)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, 0x00); // Disable ANTSW_OFDM		
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, 0x00); // Disable ANTSW_CCK			
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, 0x80); // Main Ant				
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, 0x00); // Clear R154[4], Rx Ant is not bound to the previous rx packet selected Ant

				DBGPRINT(RT_DEBUG_OFF, ("\x1b[31m%s: rt5370G/rt5390R --> switch to main\x1b[m\n", __FUNCTION__));
			}
			else
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, 0x00); // Disable ANTSW_OFDM		
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, 0x00); // Disable ANTSW_CCK			
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, 0x00); // Aux Ant				
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, 0x00); // Clear R154[4], Rx Ant is not bound to the previous rx packet selected Ant
				DBGPRINT(RT_DEBUG_OFF, ("\x1b[31m%s: rt5370G/rt5390R --> switch to aux\x1b[m\n", __FUNCTION__));
			}
			
			
		}	
		else
		{
			if (Ant == 0) /* 0: Main antenna */
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpValue);
				BbpValue = ((BbpValue & ~0x80) | (0x80));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpValue);

				DBGPRINT(RT_DEBUG_OFF, ("AsicSetRxAnt, switch to main antenna\n"));
			}
			else /* 1: Aux. antenna */
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpValue);
				BbpValue = ((BbpValue & ~0x80) | (0x00));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpValue);

				DBGPRINT(RT_DEBUG_OFF, ("AsicSetRxAnt, switch to aux. antenna\n"));
			}
		}	
	}
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
}

