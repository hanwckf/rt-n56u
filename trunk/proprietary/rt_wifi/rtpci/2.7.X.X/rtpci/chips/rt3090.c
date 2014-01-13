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
	rt3090.c

	Abstract:
	Specific funcitons and variables for RT3090

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT3090

#include "rt_config.h"


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

VOID NICInitRT3090RFRegisters(IN PRTMP_ADAPTER pAd)
{
	INT i;
	/*
	   Driver must read EEPROM to get RfIcType before initial RF registers
	   Initialize RF register to default value
	*/
	if (IS_RT3090(pAd))
	{
		/* 
		   Init RF calibration
		   Driver should toggle RF R30 bit7 before init RF registers
		*/
		UINT8 RfReg = 0, data;
		
		RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RfReg);
		RfReg |= 0x80;
		RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);
		RTMPusecDelay(1000);
		RfReg &= 0x7F;
		RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);

		/* init R24, R31 */
/*		RT30xxWriteRFRegister(pAd, RF_R24, 0x0F); */
/*		RT30xxWriteRFRegister(pAd, RF_R31, 0x0F); */

		/* RT309x version E has fixed this issue */
		if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
		{
			/* patch tx EVM issue temporarily */
			RTMP_IO_READ32(pAd, LDO_CFG0, &data);
			data = ((data & 0xE0FFFFFF) | 0x0D000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
		}
		else
		{
			RTMP_IO_READ32(pAd, LDO_CFG0, &data);
			data = ((data & 0xE0FFFFFF) | 0x01000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
		}

		/* patch LNA_PE_G1 failed issue */
		RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
		data &= ~(0x20);
		RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);

		/* Initialize RF register to default value */
		for (i = 0; i < NUM_RF_3020_REG_PARMS; i++)
		{
			RT30xxWriteRFRegister(pAd, RT3020_RFRegTable[i].Register, RT3020_RFRegTable[i].Value);
		}

		RT30xxWriteRFRegister(pAd, RF_R31, 0x14);

		/* Driver should set RF R6 bit6 on before calibration */
		RT30xxReadRFRegister(pAd, RF_R06, (PUCHAR)&RfReg);
		RfReg |= 0x40;
		RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RfReg);

		/* For RF filter Calibration */
		RTMPFilterCalibration(pAd);

		/* Initialize RF R27 register, set RF R27 must be behind RTMPFilterCalibration() */
		if ((pAd->MACVersion & 0xffff) < 0x0211)
			RT30xxWriteRFRegister(pAd, RF_R27, 0x3);

		/* set led open drain enable */
		RTMP_IO_READ32(pAd, OPT_14, &data);
		data |= 0x01;
		RTMP_IO_WRITE32(pAd, OPT_14, data);
		
		/* set default antenna as main */
		if (pAd->RfIcType == RFIC_3020)
			AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);

		/*
		   From RT3071 Power Sequence v1.1 document, the Normal Operation Setting Registers as follow :
		   BBP_R138 / RF_R1 / RF_R15 / RF_R17 / RF_R20 / RF_R21.
		   add by johnli, RF power sequence setup, load RF normal operation-mode setup
		*/
		RT30xxLoadRFNormalModeSetup(pAd);
	}
}

VOID RT3090ChipHook(IN PRTMP_ADAPTER pAd)
{
#if defined(CARRIER_DETECTION_SUPPORT) || defined(GREENAP_SUPPORT)
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
#endif
#if defined(CARRIER_DETECTION_SUPPORT)
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	if (IS_RT3090A(pAd)) {
		pChipCap->carrier_func = TONE_RADAR_V2;
		pChipOps->ToneRadarProgram = ToneRadarProgram_v2;
	}
	else {
		pChipCap->carrier_func = TONE_RADAR_V1;
		pChipOps->ToneRadarProgram = ToneRadarProgram_v1;
	}
#endif /* CARRIER_DETECTION_SUPPORT */
#if defined(GREENAP_SUPPORT)
	pChipOps->EnableAPMIMOPS = EnableAPMIMOPSv2;
	pChipOps->DisableAPMIMOPS = DisableAPMIMOPSv2;
#endif /* GREENAP_SUPPORT */
}
#endif /* RT3090 */

