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


//
// RF register initialization set
//
REG_PAIR RF3053RegTable[] = {
//	{RF_R00,		0x10}, // By default
	{RF_R01,		0x03}, 
	{RF_R02,		0x80},
	{RF_R03,		0x80},
//	{RF_R04,		0x00}, // By default
	{RF_R05,		0x00}, 
	{RF_R06,		0x40}, 
//	{RF_R07,		0x00}, // By default
	{RF_R08,		0xF1}, 
	{RF_R09,		0x02},

	{RF_R10,		0x53},
	{RF_R11,		0x40},
	{RF_R12,		0x4E},
	{RF_R13,		0x12},
//	{RF_R14,		0x00}, // By default
//	{RF_R15,		0x00}, // By default
//	{RF_R16,		0x00}, // By default
	{RF_R17,		0x26},
	{RF_R18,		0x40}, 
//	{RF_R19,		0x00}, // By default

//	{RF_R20,		0x00}, // By default
//	{RF_R21,		0x00}, // By default
	{RF_R22,		0x20},	
//	{RF_R23,		0x00}, // By default
//	{RF_R24,		0x00}, // By default
//	{RF_R25,		0x00}, // By default
//	{RF_R26,		0x00}, // By default
//	{RF_R27,		0x00}, // By default
//	{RF_R28,		0x00}, // By default
//	{RF_R29,		0x00}, // By default
	{RF_R30,		0x10},
	{RF_R31,		0x80}, 

	{RF_R32,		0x80}, 
	{RF_R33,		0x00}, 
	{RF_R34,		0x3C}, 
	{RF_R35,		0xE0}, 
//	{RF_R36,		0x80}, // By default
//	{RF_R37,		0x00}, // By default
	{RF_R38,		0x85}, 
	{RF_R39,		0x23}, 
//	{RF_R40,		0x00}, // By default
//	{RF_R41,		0x00}, // By default
//	{RF_R42,		0x00}, // By default
//	{RF_R43,		0x00}, // By default

	{RF_R44,		0xD3},
	{RF_R45,		0xBB}, 
	{RF_R46,		0x60}, 
//	{RF_R47,		0x00}, // By default
//	{RF_R48,		0x00}, // By default
	{RF_R49,		0x8E},
	{RF_R50,		0x86},
	{RF_R51,		0x75},
	{RF_R52,		0x45},
	{RF_R53,		0x18},
	{RF_R54,		0x18},
	{RF_R55,		0x18},

	{RF_R56,		0xDB}, 
	{RF_R57,		0x6E},
//	{RF_R58,		0x00}, // By default
//	{RF_R59,		0x00}, // By default
//	{RF_R60,		0x00}, // By default
//	{RF_R61,		0x00}, // By default
//	{RF_R62,		0x00}, // By default
//	{RF_R63,		0x00}, // By default
};

UCHAR NUM_RF_3053_REG_PARMS = (sizeof(RF3053RegTable) / sizeof(REG_PAIR));

static VOID RT3593_PostBBPInitialization(
	IN PRTMP_ADAPTER pAd);

static VOID RT3593LoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd);




VOID NICInitRT3593RFRegisters(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG	RfReg = 0;
	ULONG	data;
	USHORT	i;


//	if (IS_RT3593(pAd))
	{
		// Init RF calibration
		// Driver should toggle RF R30 bit7 before init RF registers
		RT30xxReadRFRegister(pAd, RF_R02, (PUCHAR)&RfReg);
		RfReg = ((RfReg & ~0x80) | 0x80); // rescal_en (initiate calbration)
		RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
		
		RTMPusecDelay(1000);
		
		RfReg = (RfReg & ~0x80); // rescal_en (initiate calbration)
		RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);

		// init R24, R31
		RT30xxReadRFRegister(pAd, RF_R32, (PUCHAR)&RfReg);
		RfReg = ((RfReg & ~0xF8) | 0x78); // tx_agc_fc (capacitor control in Tx baseband filter)
		//RT30xxWriteRFRegister(pAd, RF_R32, (UCHAR)RfReg);

		RT30xxReadRFRegister(pAd, RF_R31, (PUCHAR)&RfReg);
		RfReg = ((RfReg & ~0xF8) | 0x78); // rx_agc_fc (capacitor control in Rx baseband filter)
		//RT30xxWriteRFRegister(pAd, RF_R31, (UCHAR)RfReg);

		// RT3071 version E has fixed this issue
		if ((pAd->MACVersion & 0xffff) < 0x0211)
		{
			if (pAd->NicConfig2.field.DACTestBit == 1)
			{
				// patch tx EVM issue temporarily
				RTMP_IO_READ32(pAd, LDO_CFG0, &data);
				data = ((data & 0xE0FFFFFF) | 0x0D000000);
				RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
			}
		}
		else
		{
			// Patch CCK ok, OFDM failed issue, just toggle and restore LDO_CFG0.
			// Patch SRAM for 3572, increase voltage to 1.35V on core voltage and down to 1.2V after 1 msec 

			RTMP_IO_READ32(pAd, LDO_CFG0, &data);
			data = ((data & 0xE0FFFFFF) | 0x0D000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, data);

			RTMPusecDelay(1000);

			data = ((data & 0xE0FFFFFF) | 0x01000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
		}

		// patch LNA_PE_G1 failed issue
		RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
		data &= ~(0x20);
		RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);

		// Initialize RF register to default value
		for (i = 0; i < NUM_RF_3053_REG_PARMS; i++)
		{
			RT30xxWriteRFRegister(pAd, RF3053RegTable[i].Register, RF3053RegTable[i].Value);
		}

		// Driver should set RF R6 bit6 on before calibration
		RT30xxReadRFRegister(pAd, RF_R18, (PUCHAR)&RfReg);
		RfReg |= ((RfReg & ~0x40) | 0x40); // xo_tune_bypass (0: XO is auto-tuned and 1: XO tuning bypassed)
		RT30xxWriteRFRegister(pAd, RF_R18, (UCHAR)RfReg);

		//For RF filter Calibration
		RTMPFilterCalibration(pAd);

		//2 TODO:?
		pAd->Mlme.CaliBW20RfR24 = 0x1F;
		pAd->Mlme.CaliBW40RfR24 = 0x2F;

		// save R25, R26 for 2.4GHz
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R25, &pAd->Bbp25);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R26, &pAd->Bbp26);

		// Initialize RF R27 register, set RF R27 must be behind RTMPFilterCalibration()
		if ((pAd->MACVersion & 0xffff) < 0x0211)
			RT30xxWriteRFRegister(pAd, RF_R27, 0x3);

		// set led open drain enable
		RTMP_IO_READ32(pAd, OPT_14, &data);
		data |= 0x01;
		RTMP_IO_WRITE32(pAd, OPT_14, data);

		// Initialize RT3090 serial MAc registers which is different from RT2860 serial
		RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);
		// RT3071 version E has fixed this issue
		if ((pAd->MACVersion & 0xffff) < 0x0211)
		{
			if (pAd->NicConfig2.field.DACTestBit == 1)
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x1F);	// To fix throughput drop drastically
			}
			else
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0F);	// To fix throughput drop drastically
			}
		}
		else
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0);
		}

		// set default antenna as main
		if (pAd->RfIcType == RFIC_3020)
			AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);

		// add by johnli, RF power sequence setup, load RF normal operation-mode setup
		RT3593LoadRFNormalModeSetup(pAd);

		// adjust some BBP register contents
		// also can put these BBP registers to pBBPRegTable
		RT3593_PostBBPInitialization(pAd);
	}
} /* End of NICInitRT3593RFRegisters */


//
// Post-process the BBP registers based on the chip model
//
// Parameters
//	pAd: The adapter data structure
//
// Return Value
//	None
//
static VOID RT3593_PostBBPInitialization(
	IN PRTMP_ADAPTER pAd)
{
	BBP_R105_STRUC BBPR105 = { { 0 } };
	BBP_R106_STRUC BBPR106 = { { 0 } };
	
	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	//
	// The channel estimation updates based on remodulation of L-SIG and HT-SIG symbols.
	//
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R105, &BBPR105.byte);

// Disable re-modulation until further verificaiton (poor Rx throughput with Atheros APs)
/*
	if (IS_RT3090A(pAd))
	{
		BBPR105.field.EnableSIGRemodulation = 1; // The channel estimation updates based on remodulation of L-SIG and HT-SIG symbols.
	}
	else
	{
		BBPR105.field.EnableSIGRemodulation = 0;
	}
*/

	//
	// Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)
	//
	if (pAd->Antenna.field.RxPath == 1) // Single RX
	{
		BBPR105.field.MLDFor2Stream = 0;
	}
	else
	{
		BBPR105.field.MLDFor2Stream = 1;
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, BBPR105.byte);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: BBP_R105: BBPR105.field.EnableSIGRemodulation = %d, BBPR105.field.MLDFor2Stream = %d\n", 
		__FUNCTION__, 
		BBPR105.field.EnableSIGRemodulation, 
		BBPR105.field.MLDFor2Stream));

//	if (IS_RT3593(pAd))
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R106, &BBPR106.byte);

		BBPR106.field.ShortGI_Offset20 = 0x06; // Delay GI remover when the short GI is detected in 20MHz band (20M sampling rate)
		BBPR106.field.ShortGI_Offset40 = 0x08; // Delay GI remover when the short GI is detected in 40MHz band (40M sampling rate)
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, BBPR106.byte);

		DBGPRINT(RT_DEBUG_TRACE, ("%s: BBPR106: BBPR106.field.ShortGI_Offset20 = %d, BBPR106.field.ShortGI_Offset40 = %d\n", 
			__FUNCTION__, 
			BBPR106.field.ShortGI_Offset20, 
			BBPR106.field.ShortGI_Offset40));

//		if (IS_RT3593(pAd))
		{
			// Patch wrong default value for SNR2 report
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, 0x04);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, 0x3b);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
} /* End of RT3593_PostBBPInitialization */


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
	CHAR bbpreg = 0;


	// TX_LO2_en
	RT30xxReadRFRegister(pAd, RF_R50, (PUCHAR)&RfReg);
	RfReg = ((RfReg & ~0x10) | 0x00); // tx_lo2_en (both bands, 0: LO2 follows TR switch)
	RT30xxWriteRFRegister(pAd, RF_R50, (UCHAR)RfReg);

	// TX_LO1_en, RX_MX2_GC
	RT30xxReadRFRegister(pAd, RF_R51, (PUCHAR)&RfReg);
	RfReg = ((RfReg & ~0x1C) | ((pAd->TxMixerGain24G & 0x07) << 2)); // tx_mx1_cc (RF mixer output tank tuning, both bands)
	RT30xxWriteRFRegister(pAd, RF_R51, (UCHAR)RfReg);

	// RX_LO1_en
	RT30xxReadRFRegister(pAd, RF_R38, (PUCHAR)&RfReg);
	RfReg = ((RfReg & ~0x20) | 0x00); // rx_lo1_en (enable RX LO1, 0: LO1 follows TR switch)
	RT30xxWriteRFRegister(pAd, RF_R38, (UCHAR)RfReg);

	// RX_LO2_en
	RT30xxReadRFRegister(pAd, RF_R39, (PUCHAR)&RfReg);
	RfReg = ((RfReg & ~0x80) | 0x00); // rx_lo2_en (enable RX LO2, 0: LO2 follows TR switch)
	RT30xxWriteRFRegister(pAd, RF_R39, (UCHAR)RfReg);

	//
	// Avoid data lost and CRC error
	//
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &bbpreg);
	bbpreg = ((bbpreg & ~0x40) | 0x40); // MAC interface control (MAC_IF_80M, 1: 80 MHz)
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, bbpreg);

	RT30xxReadRFRegister(pAd, RF_R32, (PUCHAR)&RfReg);
	RfReg = ((RfReg & ~0x07) | 0x07); // BB_rx_out_en (enable DAC output or baseband input)
	//RT30xxWriteRFRegister(pAd, RF_R32, (UCHAR)RfReg);

	RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&RfReg);
	RfReg = ((RfReg & ~0x03) | 0x03); // rf_block_en and pll_en
	RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RfReg);

	RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RfReg);
	RfReg = ((RfReg & ~0x18) | 0x10); // rxvcm (Rx BB filter VCM)
	RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);
} /* End of RT3593LoadRFNormalModeSetup */

#endif // RT3593 //

/* End of rt3593.c */
