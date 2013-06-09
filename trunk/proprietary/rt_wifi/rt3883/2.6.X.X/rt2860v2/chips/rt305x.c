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
	rt305x.c

	Abstract:
	Specific funcitons and variables for RT305x(i.e., RT3050/RT3051/RT3052)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#ifdef RT305x

#include "rt_config.h"


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //

/* Default EEPROM value for RT3050 */
UCHAR RT3050_EeBuffer[EEPROM_SIZE] = {
	0x50, 0x30, 0x01, 0x01, 0x00, 0x0c, 0x43, 0x30, 0x52, 0x88, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 
	0x43, 0x30, 0x52, 0x77, 0x00, 0x0c, 0x43, 0x30, 0x52, 0x66, 0x11, 0x05, 0x20, 0x00, 
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
	};

/* Default EEPROM value for RT3052 */
UCHAR RT3052_EeBuffer[EEPROM_SIZE] = {
	0x52, 0x30, 0x01, 0x01, 0x00, 0x0c, 0x43, 0x30, 0x52, 0x88, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 
	0x43, 0x30, 0x52, 0x77, 0x00, 0x0c, 0x43, 0x30, 0x52, 0x66, 0x22, 0x08, 0x24, 0x00, 
	0xff, 0xff, 0x2f, 0x01, 0x55, 0x77, 0xa8, 0xaa, 0x8c, 0x88, 0xff, 0xff, 0x0c, 0x00, 
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
	};

REG_PAIR   RT305x_BBPRegTable[] = {
	{BBP_R31,		0x08},		//gary recommend for ACE
	{BBP_R78,		0x0E},
	{BBP_R80,		0x08}, // requested by Gary for high power
	{BBP_R103,		0xC0}, 
};


UCHAR RT305x_NUM_BBP_REG_PARMS = (sizeof(RT305x_BBPRegTable) / sizeof(REG_PAIR));


#ifdef RT3350
//
// RF register initialization set
//
REG_PAIR   RT3350_RFRegTable[] = {
	{RF_R00,		0xA1},
	{RF_R01,		0xC1},
	{RF_R02,		0xF1},
	{RF_R03,		0x72},
	{RF_R04,		0x40},
	{RF_R05,		0xCF},
	{RF_R06,		0x7E},
	{RF_R07,		0xD0},
	{RF_R08,		0x53},
	{RF_R09,		0x01},
	{RF_R10,		0x10},
	{RF_R11,		0x21},
	{RF_R12,		0x26},// QA : 0x2D(TX0_ALC)
	{RF_R13,		0xB0},
	{RF_R14,		0x00},
	{RF_R15,		0x78},
	{RF_R16,		0x44},
	{RF_R17,		0x92},
	{RF_R18,		0x6C},
	{RF_R19,		0xCC},
	{RF_R20,		0xBB},
	{RF_R21,		0x6F},
	{RF_R22,		0x00},
	{RF_R23,		0x12},
	{RF_R24,		0x08},
	{RF_R25,		0x3F},
	{RF_R26,	 	0x0D},
	{RF_R27, 		0x75},
   	{RF_R28, 		0x10},
	{RF_R29,		0x07},
	{RF_R30,		0x00},
	{RF_R31,		0x08},
};
#endif // RT3350 //

//
// RF register initialization set
//
REG_PAIR   RT305x_RFRegTable[] = {
	{RF_R00,		0x50}, //
	{RF_R01,		0x01}, //
	{RF_R02,		0xF7}, //
	{RF_R03,		0x75}, //0x02}, //
	{RF_R04,		0x40},
	{RF_R05,		0x03}, //0x00},
	{RF_R06,		0x42}, //pass the crystal auto-tuned
	{RF_R07,		0x50}, //
	{RF_R08,		0x39}, //
	{RF_R09,		0x0F}, //0x04},
	{RF_R10,		0x60}, //0x71}, //0x72},
	{RF_R11,		0x21}, //0x11},
	{RF_R12,		0x75}, //0x7B}
	{RF_R13,		0x75}, //
	{RF_R14,		0x90}, //0xC0},
	{RF_R15,		0x58}, //0x78},
	{RF_R16,		0xB3}, //0xB2},
	{RF_R17,		0x92}, //Mason Hsu 20080624
	{RF_R18,		0x2C},
	{RF_R19,		0x02},
	{RF_R20,		0xBA},
	{RF_R21,		0xDB},
	{RF_R22,		0x00}, //
	{RF_R23,		0x31}, //
	{RF_R24,		0x08}, //
	{RF_R25,		0x01},
	{RF_R26, 		0x25}, //Core Power: 0x25=1.25v
	{RF_R27, 		0x23}, //RF: 1.35v (Mason Hsu 20081023)
	{RF_R28, 		0x13}, //ADC: must consist with R27
	{RF_R29,		0x83}, //0x18},
	{RF_R30,		0x00}, //
	{RF_R31,		0x00}, //
};

UCHAR RT305x_NUM_RF_REG_PARMS = (sizeof(RT305x_RFRegTable) / sizeof(REG_PAIR));

RTMP_REG_PAIR	RT305x_MACRegTable[] =	{
	{TX_SW_CFG0,		0x400},   // Gary,2008-05-21 0x0 for CWC test , 2008-06-19 0x400 for rf reason
	{TX_SW_CFG1,		0x0}, 	  // Gary,2008-06-18 
	{TX_SW_CFG2,		0x30}, 	  // Bruce, CwC IOT issue
};

UCHAR RT305x_NUM_MAC_REG_PARMS = (sizeof(RT305x_MACRegTable) / sizeof(RTMP_REG_PAIR));


static VOID NICInitRT305xRF_R27_R28(IN PRTMP_ADAPTER pAd)
{
	if ((pAd->CommonCfg.CN >> 16) == 0x3033)
	{
		if (pAd->CommonCfg.CID < 0x103)
		{
			RT30xxWriteRFRegister(pAd, RF_R27, 0x23);
			RT30xxWriteRFRegister(pAd, RF_R28, 0x13);
		}
		else
		{
			RT30xxWriteRFRegister(pAd, RF_R27, 0x20);
			RT30xxWriteRFRegister(pAd, RF_R28, 0x10);
		}
	}
	else
	{
		if (pAd->RfIcType == RFIC_3320)
		{
			RT30xxWriteRFRegister(pAd, RF_R27, 0x75);
			RT30xxWriteRFRegister(pAd, RF_R28, 0x10);
		}

		if (pAd->RfIcType == RFIC_3020)
		{
			RT30xxWriteRFRegister(pAd, RF_R27, 0x21);
			RT30xxWriteRFRegister(pAd, RF_R28, 0x10);
		}
	}

}




/* --------------------------- public functions ----------------------------- */

VOID NICInitRT305xRFRegisters(IN PRTMP_ADAPTER pAd)
{
	INT i;

	/* Driver must read EEPROM to get RfIcType before initial RF registers */
	/* Initialize RF register to default value */
	if ((pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3020) ||
		(pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022))
	{
        /* Init RF calibration */
        /* Driver should toggle RF R30 bit7 before init RF registers */
        UINT8 RfReg = 0;
        RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RfReg);
        RfReg |= 0x80;
        RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);
        RTMPusecDelay(1000);
        RfReg &= 0x7F;
        RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);        

		/* Initialize RF register to default value */
		if (!IS_RT3350(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Not RT3350!\n"));
			for (i = 0; i < RT305x_NUM_RF_REG_PARMS; i++)
			{
				RT30xxWriteRFRegister(pAd, RT305x_RFRegTable[i].Register, RT305x_RFRegTable[i].Value);
			}

			if ((pAd->CommonCfg.CID == 0x103) || ((pAd->CommonCfg.CN >> 16) == 0x3333))
			{
				RT30xxWriteRFRegister(pAd, RF_R17, 0x93);
			}
		}
#ifdef RT3350
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Is RT3350!\n"));
			RT305x_NUM_RF_REG_PARMS = (sizeof(RT3350_RFRegTable) / sizeof(REG_PAIR));
			for (i = 0; i < RT305x_NUM_RF_REG_PARMS; i++)
			{
					RT30xxWriteRFRegister(pAd, RT3350_RFRegTable[i].Register, RT3350_RFRegTable[i].Value);
			}
		}
#endif /* RT3350 */
	}

	NICInitRT305xRF_R27_R28(pAd);

}


VOID RT305x_PowerTuning(
	IN PRTMP_ADAPTER 			pAd,
	IN RSSI_SAMPLE				*pRssi)
{
	if (!pAd->CommonCfg.HighPowerPatchDisabled)
	{
		UCHAR RFValue;

		if (IS_RT3052B(pAd))
		{
			if ((pRssi->AvgRssi0 != 0) && (pRssi->AvgRssi0 > (pAd->BbpRssiToDbmDelta - 35) ))
			{ 
			    RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)&RFValue);
			    RFValue &= ~0x3;
			    RT30xxWriteRFRegister(pAd, RF_R27, (UCHAR)RFValue);

			    RT30xxReadRFRegister(pAd, RF_R28, (PUCHAR)&RFValue);
			    RFValue &= ~0x3;
			    RT30xxWriteRFRegister(pAd, RF_R28, (UCHAR)RFValue);
			}
	        else
			{
			    RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)&RFValue);
			    RFValue |= 0x3;
			    RT30xxWriteRFRegister(pAd, RF_R27, (UCHAR)RFValue);

			    RT30xxReadRFRegister(pAd, RF_R28, (PUCHAR)&RFValue);
			    RFValue |= 0x3;
			    RT30xxWriteRFRegister(pAd, RF_R28, (UCHAR)RFValue);
			}
		}

		if ((pAd->Antenna.field.RxPath == 2) && (IS_RT3052B(pAd)))
		{
			if ((pRssi->AvgRssi0 != 0) && (pRssi->AvgRssi0 > (pAd->BbpRssiToDbmDelta - 42) ))
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x42);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, 0x18);
			}
			else
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, 0x20);
			}
		}
	}
}


/*
========================================================================
Routine Description:
	Initialize RT305x.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT305x_Init(
	IN PRTMP_ADAPTER		pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	/* init capability */
	pChipCap->MaxNumOfBbpId = 200;
	pChipCap->pRFRegTable = RT305x_RFRegTable;

	/* init capability */
	pChipCap->pBBPRegTable = NULL;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_1;
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 16;
#ifdef RTMP_FLASH_SUPPORT
	pChipCap->eebuf = RT3050_EeBuffer;
	if (IS_RT3052(pAd))
		pChipCap->eebuf = RT3052_EeBuffer;
#endif /* RTMP_FLASH_SUPPORT */
	/* init operator */
	pChipOps->AsicRfInit = NICInitRT305xRFRegisters;
	pChipOps->AsicMacInit = NICInitRT305xMacRegisters;
	pChipOps->ChipSwitchChannel = RT305x_ChipSwitchChannel;
	pChipOps->AsicBbpInit = NICInitRT305xBbpRegisters;
	pChipOps->HighPowerTuning = RT305x_PowerTuning;
#ifdef CARRIER_DETECTION_SUPPORT
	pAd->chipCap.carrier_func = TONE_RADAR_V1;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v1;
#endif /* CARRIER_DETECTION_SUPPORT */

	RT305x_ChipSpecInit(pAd);
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
VOID NICInitRT305xMacRegisters(
	IN RTMP_ADAPTER				*pAd)
{
	UINT32 IdReg;


	for(IdReg=0; IdReg<RT305x_NUM_MAC_REG_PARMS; IdReg++)
	{
		RTMP_IO_WRITE32(pAd, RT305x_MACRegTable[IdReg].Register,
								RT305x_MACRegTable[IdReg].Value);
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
VOID NICInitRT305xBbpRegisters(
	IN PRTMP_ADAPTER		pAd)
{
	UINT32 IdReg;

	DBGPRINT(RT_DEBUG_TRACE, ("%s --->\n", __FUNCTION__));

	for (IdReg = 0; IdReg < RT305x_NUM_BBP_REG_PARMS; IdReg++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, RT305x_BBPRegTable[IdReg].Register, 
									RT305x_BBPRegTable[IdReg].Value);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP_R%d=%d\n", RT305x_BBPRegTable[IdReg].Register, 
									RT305x_BBPRegTable[IdReg].Value));
	}

	if ((pAd->CommonCfg.CN >> 16) == 0x3333)
	{
		UINT8 BBPValue = 0;
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R104, &BBPValue);
		BBPValue |= 0x80;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R104, BBPValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x31);

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, 0x46);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x0d);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R114, 0x64);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R115, 0x34);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R116, 0x67);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R117, 0x37);
	}	

	DBGPRINT(RT_DEBUG_TRACE, ("%s <---\n", __FUNCTION__));
	
}

VOID RT305x_ChipSwitchChannel(
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

	// The RF programming sequence is difference between 3xxx and 2xxx
	if (((pAd->MACVersion == 0x28720200)) && 
		((pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3322) ||
		(pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_3021) ||
		(pAd->RfIcType == RFIC_3022)))
	{
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
					// Programming channel parameters
					RT30xxWriteRFRegister(pAd, RF_R02, FreqItems3020[index].N);
#ifdef RT3350
					if (IS_RT3350(pAd))
					{
						RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
						RFValue = (RFValue & 0xF0) | (FreqItems3020[index].K & 0x0F);
						RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);
					}
					else
#endif // RT3350 //
						RT30xxWriteRFRegister(pAd, RF_R03, FreqItems3020[index].K);


					RT30xxReadRFRegister(pAd, RF_R06, (PUCHAR)&RFValue);
					RFValue = (RFValue & 0xFC) | FreqItems3020[index].R;
					RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RFValue);

					// Set Tx Power
					RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)&RFValue);
					RFValue = (RFValue & 0xE0) | TxPwer;
					RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)RFValue);

					// Set Tx1 Power
					RT30xxReadRFRegister(pAd, RF_R13, (PUCHAR)&RFValue);
					RFValue = (RFValue & 0xE0) | TxPwer2;
					RT30xxWriteRFRegister(pAd, RF_R13, (UCHAR)RFValue);

					// Set RF offset
					RT30xxReadRFRegister(pAd, RF_R23, (PUCHAR)&RFValue);
					RFValue = (RFValue & 0x80) | pAd->RfFreqOffset;
					RT30xxWriteRFRegister(pAd, RF_R23, (UCHAR)RFValue);

					// Set BW
					RT30xxReadRFRegister(pAd, RF_R24, &RFValue);

#ifdef RT3350
				/*R24, BW=20M*/
				if (IS_RT3350(pAd))
				{
					if(pAd->CommonCfg.PhyMode == PHY_11B)
						RFValue = 0x1F;
					else
						RFValue = 0x18;
				}
				else
#endif // RT3350 //
				RFValue &= 0xDF;


				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT3350
					if (IS_RT3350(pAd))
					{
						if(pAd->CommonCfg.PhyMode == PHY_11B)
						    RFValue = 0x3F;
						else
						    RFValue = 0x28;
					}
					else
#endif // RT3350 //
						RFValue |= 0x20;
				}
				RT30xxWriteRFRegister(pAd, RF_R24, RFValue);

				// Rx filter
				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT3350
					if (IS_RT3350(pAd))
						RT30xxWriteRFRegister(pAd, RF_R31, 0x68);
					else
#endif // RT3350 //
					RT30xxWriteRFRegister(pAd, RF_R31, 0x2F);
				}
				else
				{
#ifdef RT3350
					if (IS_RT3350(pAd))
						RT30xxWriteRFRegister(pAd, RF_R31, 0x48);
					else
#endif // RT3350 //
					RT30xxWriteRFRegister(pAd, RF_R31, 0x0F);
				}

				{
					// Enable RF tuning
					RT30xxReadRFRegister(pAd, RF_R07, (PUCHAR)&RFValue);
					RFValue = RFValue | 0x1;
					RT30xxWriteRFRegister(pAd, RF_R07, (UCHAR)RFValue);

					// Antenna
					RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0xab;
					if (pAd->Antenna.field.RxPath == 2)
					{
						RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);
					}
					else if (pAd->Antenna.field.RxPath == 1)
					{
						RFValue = RFValue | 0x10;
						RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);
					}

					RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&RFValue);
					RFValue = RFValue & 0x57;
					if (pAd->Antenna.field.TxPath == 2)
					{
						RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);
					}
					else if (pAd->Antenna.field.TxPath == 1)
					{
						RFValue = RFValue | 0x20;
						RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);
					}
				}

				// latch channel for future usage.
				pAd->LatchRfRegs.Channel = Channel;
				break;				
			}
		}
	}
	else
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

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		ULONG	TxPinCfg = 0x00050F05;//Gary 2007/8/9 0x050505
		UINT8	bbpValue;
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

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
	//AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x2E + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL);
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


/* --------------------------- local functions ------------------------------ */

static VOID RT305x_WlanModeChange(
	IN RTMP_ADAPTER			*pAd, 
	IN VOID 				*pData, 
	IN ULONG				Data)
{
#ifdef RT3350
	if (!IS_RT3350(pAd))
		return;

	if(pAd->CommonCfg.PhyMode == PHY_11B)
	{
		USHORT i;
			USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x126, value);
		rf_value = value & 0x00FF;
				rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
			rf_offset = RF_R21;
		if(rf_value == 0xff)
			rf_value = 0x4F;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);

		RT28xx_EEPROM_READ16(pAd, 0x12a, value);
		rf_value = value & 0x00FF;
				rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
			rf_offset = RF_R29;
		if(rf_value == 0xff)
			rf_value = 0x07;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);


		// set RF_R24
		if(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
		{    
			value = 0x3F;
		}
		else
		{
			value = 0x1F;
		}
		RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)value);
	}
	else
	{
		USHORT i;
		USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x124, value);
		rf_value = value & 0x00FF;
				rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
			rf_offset = RF_R21;
		if(rf_value == 0xff)
			rf_value = 0x6F;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);

		RT28xx_EEPROM_READ16(pAd, 0x128, value);
		rf_value = value & 0x00FF;
				rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
			rf_offset = RF_R29;
		if(rf_value == 0xff)
			rf_value = 0x07;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);

		// set RF_R24
		if(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
		{    
			value = 0x28;
		}
		else
		{
			value = 0x18;
		}
		RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)value);
	}
#endif /* RT3350 */
}


static VOID RT305x_HtModeChange(
	IN RTMP_ADAPTER			*pAd,
	IN VOID 				*pData, 
	IN ULONG				Data)
{
	UINT8	Value = 0;
	UCHAR	BW = Data;

	if (BW == BW_40)
	{
		if (!IS_RT3350(pAd))
		{
			RT30xxReadRFRegister(pAd, RF_R24, (PUCHAR)&Value);
#ifdef GREENAP_SUPPORT
			if (pAd->ApCfg.bGreenAPActive == 1)
				Value &= 0xDF;
			else
#endif // GREENAP_SUPPORT //
				Value |= 0x20;
			RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)Value);


#ifdef GREENAP_SUPPORT
			if (pAd->ApCfg.bGreenAPActive == 1)
				RT30xxWriteRFRegister(pAd, RF_R31, 0x0F);
			else
#endif // GREENAP_SUPPORT //
				RT30xxWriteRFRegister(pAd, RF_R31, 0x2F);
		}
		else
		{
			RT30xxReadRFRegister(pAd, RF_R24, (PUCHAR)&Value);
#ifdef GREENAP_SUPPORT
			if (pAd->ApCfg.bGreenAPActive == 1)
				Value &= 0xCF;
			else
#endif // GREENAP_SUPPORT //
				Value = 0x28;	/*kurtis: RT3350 non CCK Mode, BW=40M  => RF_R24=0x28*/
			RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)Value);


#ifdef GREENAP_SUPPORT
			if (pAd->ApCfg.bGreenAPActive == 1)
				RT30xxWriteRFRegister(pAd, RF_R31, 0x48);
			else
#endif // GREENAP_SUPPORT //
				RT30xxWriteRFRegister(pAd, RF_R31, 0x68);
		}
	}
	else
	{
		if (!IS_RT3350(pAd))
		{
			RT30xxReadRFRegister(pAd, RF_R24, (PUCHAR)&Value);
			Value &= 0xDF;
			RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)Value);
			RT30xxWriteRFRegister(pAd, RF_R31, 0x0F);
		}
		else
		{
			/*kurtis: RT3350 non CCK Mode, BW=20M  => RF_R24=0x18*/
			Value = 0x18;
			RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)Value);

			RT30xxWriteRFRegister(pAd, RF_R31, 0x48);
		}
	}
}


static VOID RT305x_SpecificInit(
	IN RTMP_ADAPTER			*pAd, 
	IN VOID 				*pData, 
	IN ULONG				Data)
{
#ifdef RT3350
	if (!IS_RT3350(pAd))
		return;

	if(1)
	{
		USHORT i;
	        USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x120, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;
		
		if(rf_offset == 0xff)
		        rf_offset = RF_R09;
		if(rf_value == 0xff)
			rf_value = 0x01;

		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
		
		RT28xx_EEPROM_READ16(pAd, 0x122, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		        rf_offset = RF_R19;
		if(rf_value == 0xff)
			rf_value = 0xcc;

		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	}

	if(pAd->CommonCfg.PhyMode == PHY_11B)
	{
		USHORT i;
	        USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x126, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R21;
		if(rf_value == 0xff)
		    rf_value = 0x4F;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	
		RT28xx_EEPROM_READ16(pAd, 0x12a, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R29;
		if(rf_value == 0xff)
		    rf_value = 0x07;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	

		// set RF_R24
		if(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
		{    
			value = 0x3F;
		}
		else
		{
			value = 0x1F;
		}
		RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)value);


	}
	else
	{
		USHORT i;
	        USHORT value;
		UCHAR  rf_offset;
		UCHAR  rf_value;

		RT28xx_EEPROM_READ16(pAd, 0x124, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R21;
		if(rf_value == 0xff)
		    rf_value = 0x6F;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	
		RT28xx_EEPROM_READ16(pAd, 0x128, value);
		rf_value = value & 0x00FF;
                rf_offset = (value & 0xFF00) >> 8;

		if(rf_offset == 0xff)
		    rf_offset = RF_R29;
		if(rf_value == 0xff)
		    rf_value = 0x07;
		RT30xxWriteRFRegister(pAd, rf_offset, (UCHAR)rf_value);
	
		// set RF_R24
		if(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
		{    
			value = 0x28;
		}
		else
		{
			value = 0x18;
		}
		RT30xxWriteRFRegister(pAd, RF_R24, (UCHAR)value);

	}

#endif /* RT3350 */
}


VOID RT305x_ChipSpecInit(
	IN RTMP_ADAPTER				*pAd)
{	
	INT	i;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

	printk("%s: CHIP_SPEC_ID_NUM(%d)\n", __FUNCTION__, CHIP_SPEC_ID_NUM);

	/* Default as NULL function */
	for (i = 0; i < CHIP_SPEC_ID_NUM; i++)
	{		
		pChipOps->ChipSpecFunc[i] = NULL;
	}

	/* re-assign the corresponding routine */
	pChipOps->ChipSpecFunc[RT305x_WLAN_MODE_CHANGE] = 
						(CHIP_SPEC_FUNC)RT305x_WlanModeChange;
	pChipOps->ChipSpecFunc[RT305x_INITIALIZATION] = 
						(CHIP_SPEC_FUNC)RT305x_SpecificInit;
	pChipOps->ChipSpecFunc[RT305x_HT_MODE_CHANGE] = 
						(CHIP_SPEC_FUNC)RT305x_HtModeChange;
			


}


#endif // RT305x //

/* End of rt305x.c */
