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
	rt3390.c

	Abstract:
	Specific funcitons and variables for RT3090

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3390

#include "rt_config.h"


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //

VOID NICInitRT3390RFRegisters(IN PRTMP_ADAPTER pAd)
{
	INT i;
	UINT8 RfReg = 0;
	UINT32 data;
	/*CHAR bbpreg;*/

	// Driver must read EEPROM to get RfIcType before initial RF registers
	// Initialize RF register to default value

	// Init RF calibration
	// Driver should toggle RF R30 bit7 before init RF registers

	RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RfReg);
	RfReg |= 0x80;
	RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);
	RTMPusecDelay(1000);
	RfReg &= 0x7F;
	RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);

	// init R24, R31
//	RT30xxWriteRFRegister(pAd, RF_R24, 0x0F);
//	RT30xxWriteRFRegister(pAd, RF_R31, 0x0F);

		if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
		{
			RTMP_IO_READ32(pAd, LDO_CFG0, &data);
			data = ((data & 0xE0FFFFFF) | 0x0D000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
		}
		else
		{
			RTMP_IO_READ32(pAd, LDO_CFG0, &data);
			data = ((data & 0xE0FFFFFF) | 0x0D000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, data);

			RTMPusecDelay(1000);
			data = ((data & 0xE0FFFFFF) | 0x01000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
		}

	if (IS_RT3071(pAd) || IS_RT3390(pAd))
	{
		RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
		data &= ~(0x20);
		RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);


		// RF registers initialization
		for (i = 0; i < NUM_RF_3320_REG_PARMS; i++)
		{
			RT30xxWriteRFRegister(pAd, RF3320_RFRegTable[i].Register, RF3320_RFRegTable[i].Value);
		}

		// Driver should set RF R6 bit6 on before calibration	
		RT30xxReadRFRegister(pAd, RF_R06, (PUCHAR)&RfReg);
		RfReg |= 0x40;
		RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RfReg);

		if (IS_RT3390(pAd)) // Disable RF filter calibration
		{      
			// Disable RF filter calibration
			pAd->Mlme.CaliBW20RfR24 = BW20RFR24;
			pAd->Mlme.CaliBW40RfR24 = BW40RFR24;

			pAd->Mlme.CaliBW20RfR31 = BW20RFR31;
			pAd->Mlme.CaliBW40RfR31 = BW40RFR31;
		}
		else
		{
			//For RF filter Calibration
			//RT33xxFilterCalibration(pAd);
		}

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
		if (pAd->RfIcType == RFIC_3320)
			AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);

/*
		From RT3071 Power Sequence v1.1 document, the Normal Operation Setting Registers as follow :
		BBP_R138 / RF_R1 / RF_R15 / RF_R17 / RF_R20 / RF_R21.
 */
		// RF power sequence setup, load RF normal operation-mode setup
		RT33xxLoadRFNormalModeSetup(pAd);
       }
}
#endif // RT3390 //

