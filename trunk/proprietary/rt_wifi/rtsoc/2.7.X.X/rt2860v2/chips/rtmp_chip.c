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
	rt_chip.c

	Abstract:
	Ralink Wireless driver CHIP related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"



FREQUENCY_ITEM RtmpFreqItems3020[] =
{	
	/* ISM : 2.4 to 2.483 GHz                         */
	/* 11g*/
	/*-CH---N-------R---K-----------*/
	{1,    241,  2,  2},
	{2,    241,	 2,  7},
	{3,    242,	 2,  2},
	{4,    242,	 2,  7},
	{5,    243,	 2,  2},
	{6,    243,	 2,  7},
	{7,    244,	 2,  2},
	{8,    244,	 2,  7},
	{9,    245,	 2,  2},
	{10,   245,	 2,  7},
	{11,   246,	 2,  2},
	{12,   246,	 2,  7},
	{13,   247,	 2,  2},
	{14,   248,	 2,  4},
};

FREQUENCY_ITEM FreqItems3020_Xtal20M[] =
{	
	/*
	 * RF_R08:
	 * <7:0>: pll_N<7:0>
	 *
	 * RF_R09:
	 * <3:0>: pll_K<3:0>
	 * <4>: pll_N<8>
	 * <7:5>pll_N<11:9>
	 *
	 */
	/*-CH---N--------R---N[7:4]K[3:0]------*/
	{1,    0xE2,     2,  0x14},
	{2,    0xE3,	 2,  0x14},
	{3,    0xE4,	 2,  0x14},
	{4,    0xE5,	 2,  0x14},
	{5,    0xE6,	 2,  0x14},
	{6,    0xE7,	 2,  0x14},
	{7,    0xE8,	 2,  0x14},
	{8,    0xE9,	 2,  0x14},
	{9,    0xEA,	 2,  0x14},
	{10,   0xEB,	 2,  0x14},
	{11,   0xEC,	 2,  0x14},
	{12,   0xED,	 2,  0x14},
	{13,   0xEE,	 2,  0x14},
	{14,   0xF0,	 2,  0x18},
};

UCHAR	NUM_OF_3020_CHNL = (sizeof(RtmpFreqItems3020) / sizeof(FREQUENCY_ITEM));

FREQUENCY_ITEM *FreqItems3020 = RtmpFreqItems3020;

#ifndef RT2880
#if defined(RT28xx) || defined(RT2883) 
/* Reset the RFIC setting to new series    */
RTMP_RF_REGS RF2850RegTable[] = {
/*		ch	 R1 		 R2 		 R3(TX0~4=0) R4*/
		{1,  0x98402ecc, 0x984c0786, 0x9816b455, 0x9800510b},
		{2,  0x98402ecc, 0x984c0786, 0x98168a55, 0x9800519f},
		{3,  0x98402ecc, 0x984c078a, 0x98168a55, 0x9800518b},
		{4,  0x98402ecc, 0x984c078a, 0x98168a55, 0x9800519f},
		{5,  0x98402ecc, 0x984c078e, 0x98168a55, 0x9800518b},
		{6,  0x98402ecc, 0x984c078e, 0x98168a55, 0x9800519f},
		{7,  0x98402ecc, 0x984c0792, 0x98168a55, 0x9800518b},
		{8,  0x98402ecc, 0x984c0792, 0x98168a55, 0x9800519f},
		{9,  0x98402ecc, 0x984c0796, 0x98168a55, 0x9800518b},
		{10, 0x98402ecc, 0x984c0796, 0x98168a55, 0x9800519f},
		{11, 0x98402ecc, 0x984c079a, 0x98168a55, 0x9800518b},
		{12, 0x98402ecc, 0x984c079a, 0x98168a55, 0x9800519f},
		{13, 0x98402ecc, 0x984c079e, 0x98168a55, 0x9800518b},
		{14, 0x98402ecc, 0x984c07a2, 0x98168a55, 0x98005193},

		/* 802.11 UNI / HyperLan 2*/
		{36, 0x98402ecc, 0x984c099a, 0x98158a55, 0x980ed1a3},
		{38, 0x98402ecc, 0x984c099e, 0x98158a55, 0x980ed193},
		{40, 0x98402ec8, 0x984c0682, 0x98158a55, 0x980ed183},
		{44, 0x98402ec8, 0x984c0682, 0x98158a55, 0x980ed1a3},
		{46, 0x98402ec8, 0x984c0686, 0x98158a55, 0x980ed18b},
		{48, 0x98402ec8, 0x984c0686, 0x98158a55, 0x980ed19b},
		{52, 0x98402ec8, 0x984c068a, 0x98158a55, 0x980ed193},
		{54, 0x98402ec8, 0x984c068a, 0x98158a55, 0x980ed1a3},
		{56, 0x98402ec8, 0x984c068e, 0x98158a55, 0x980ed18b},
		{60, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed183},
		{62, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed193},
		{64, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed1a3}, /* Plugfest#4, Day4, change RFR3 left4th 9->5.*/

		/* 802.11 HyperLan 2*/
		{100, 0x98402ec8, 0x984c06b2, 0x98178a55, 0x980ed783},
		
		/* 2008.04.30 modified */
		/* The system team has AN to improve the EVM value */
		/* for channel 102 to 108 for the RT2850/RT2750 dual band solution.*/
		{102, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed793},
		{104, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed1a3},
		{108, 0x98402ecc, 0x985c0a32, 0x98578a55, 0x980ed193},

		{110, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed183},
		{112, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed19b},
		{116, 0x98402ecc, 0x984c0a3a, 0x98178a55, 0x980ed1a3},
		{118, 0x98402ecc, 0x984c0a3e, 0x98178a55, 0x980ed193},
		{120, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed183},
		{124, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed193},
		{126, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed15b}, /* 0x980ed1bb->0x980ed15b required by Rory 20070927*/
		{128, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed1a3},
		{132, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed18b},
		{134, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed193},
		{136, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed19b},
		{140, 0x98402ec4, 0x984c038a, 0x98178a55, 0x980ed183},

		/* 802.11 UNII*/
		{149, 0x98402ec4, 0x984c038a, 0x98178a55, 0x980ed1a7},
		{151, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed187},
		{153, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed18f},
		{157, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed19f},
		{159, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed1a7},
		{161, 0x98402ec4, 0x984c0392, 0x98178a55, 0x980ed187},
		{165, 0x98402ec4, 0x984c0392, 0x98178a55, 0x980ed197},
		{167, 0x98402ec4, 0x984c03d2, 0x98179855, 0x9815531f},
		{169, 0x98402ec4, 0x984c03d2, 0x98179855, 0x98155327},
		{171, 0x98402ec4, 0x984c03d6, 0x98179855, 0x98155307},
		{173, 0x98402ec4, 0x984c03d6, 0x98179855, 0x9815530f},

		/* Japan*/
		{184, 0x95002ccc, 0x9500491e, 0x9509be55, 0x950c0a0b},
		{188, 0x95002ccc, 0x95004922, 0x9509be55, 0x950c0a13},
		{192, 0x95002ccc, 0x95004926, 0x9509be55, 0x950c0a1b},
		{196, 0x95002ccc, 0x9500492a, 0x9509be55, 0x950c0a23},
		{208, 0x95002ccc, 0x9500493a, 0x9509be55, 0x950c0a13},
		{212, 0x95002ccc, 0x9500493e, 0x9509be55, 0x950c0a1b},
		{216, 0x95002ccc, 0x95004982, 0x9509be55, 0x950c0a23},

		/* still lack of MMAC(Japan) ch 34,38,42,46*/
};
UCHAR	NUM_OF_2850_CHNL = (sizeof(RF2850RegTable) / sizeof(RTMP_RF_REGS));
#endif /* defined(RT28xx) || defined(RT2883) */
#endif /* !RT2880 */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 

/* The Tx power tuning entry*/
const TX_POWER_TUNING_ENTRY_STRUCT TxPowerTuningTableOrg[] = 
{
/*	idxTxPowerTable		Tx power control over RF			Tx power control over MAC*/
/*	(zero-based array)		{ RF R12[4:0]: Tx0 ALC},			{MAC 0x1314~0x1324}*/
/*	0	*/				{0x00, 							-15}, 
/*	1	*/				{0x01, 							-15}, 
/*	2	*/				{0x00, 							-14}, 
/*	3	*/				{0x01, 							-14}, 
/*	4	*/				{0x00, 							-13}, 
/*	5	*/				{0x01, 							-13}, 
/*	6	*/				{0x00, 							-12}, 
/*	7	*/				{0x01, 							-12}, 
/*	8	*/				{0x00, 							-11}, 
/*	9	*/				{0x01, 							-11}, 
/*	10	*/				{0x00, 							-10}, 
/*	11	*/				{0x01, 							-10}, 
/*	12	*/				{0x00, 							-9}, 
/*	13	*/				{0x01, 							-9}, 
/*	14	*/				{0x00, 							-8}, 
/*	15	*/				{0x01, 							-8}, 
/*	16	*/				{0x00, 							-7}, 
/*	17	*/				{0x01, 							-7}, 
/*	18	*/				{0x00, 							-6}, 
/*	19	*/				{0x01, 							-6}, 
/*	20	*/				{0x00, 							-5}, 
/*	21	*/				{0x01, 							-5}, 
/*	22	*/				{0x00, 							-4}, 
/*	23	*/				{0x01, 							-4}, 
/*	24	*/				{0x00, 							-3}, 
/*	25	*/				{0x01, 							-3}, 
/*	26	*/				{0x00,							-2}, 
/*	27	*/				{0x01, 							-2}, 
/*	28	*/				{0x00, 							-1}, 
/*	29	*/				{0x01, 							-1}, 
/*	30	*/				{0x00,							0}, 
/*	31	*/				{0x01,							0}, 
/*	32	*/				{0x02,							0}, 
/*	33	*/				{0x03,							0}, 
/*	34	*/				{0x04,							0}, 
/*	35	*/				{0x05,							0}, 
/*	36	*/				{0x06,							0}, 
/*	37	*/				{0x07,							0}, 
/*	38	*/				{0x08,							0}, 
/*	39	*/				{0x09,							0}, 
/*	40	*/				{0x0A,							0}, 
/*	41	*/				{0x0B,							0}, 
/*	42	*/				{0x0C,							0}, 
/*	43	*/				{0x0D,							0}, 
/*	44	*/				{0x0E,							0}, 
/*	45	*/				{0x0F,							0}, 
/*	46	*/				{0x0F-1,							1}, 
/*	47	*/				{0x0F,							1}, 
/*	48	*/				{0x0F-1,							2}, 
/*	49	*/				{0x0F,							2}, 
/*	50	*/				{0x0F-1,							3}, 
/*	51	*/				{0x0F,							3}, 
/*	52	*/				{0x0F-1,							4}, 
/*	53	*/				{0x0F,							4}, 
/*	54	*/				{0x0F-1,							5}, 
/*	55	*/				{0x0F,							5}, 
/*	56	*/				{0x0F-1,							6}, 
/*	57	*/				{0x0F,							6}, 
/*	58	*/				{0x0F-1,							7}, 
/*	59	*/				{0x0F,							7}, 
/*	60	*/				{0x0F-1,							8}, 
/*	61	*/				{0x0F,							8}, 
/*	62	*/				{0x0F-1,							9}, 
/*	63	*/				{0x0F,							9}, 
/*	64	*/				{0x0F-1,							10}, 
/*	65	*/				{0x0F,							10}, 
/*	66	*/				{0x0F-1,							11}, 
/*	67	*/				{0x0F,							11}, 
/*	68	*/				{0x0F-1,							12}, 
/*	69	*/				{0x0F,							12}, 
/*	70	*/				{0x0F-1,							13}, 
/*	71	*/				{0x0F,							13}, 
/*	72	*/				{0x0F-1,							14}, 
/*	73	*/				{0x0F,							14}, 
/*	74	*/				{0x0F-1,							15}, 
/*	75	*/				{0x0F,							15}, 
};

#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

/* private function prototype */

static VOID RxSensitivityTuning(
	IN PRTMP_ADAPTER		pAd);

#ifdef CONFIG_STA_SUPPORT
static UCHAR ChipAGCAdjust(
	IN PRTMP_ADAPTER		pAd,
	IN CHAR					Rssi,
	IN UCHAR				OrigR66Value);
#endif /* CONFIG_STA_SUPPORT */

static VOID ChipBBPAdjust(
	IN RTMP_ADAPTER			*pAd);

static VOID ChipSwitchChannel(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR				Channel,
	IN BOOLEAN				bScan);

static VOID Default_ChipAGCInit(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth);

static VOID AsicAntennaDefaultReset(
	IN PRTMP_ADAPTER		pAd,
	IN EEPROM_ANTENNA_STRUC	*pAntenna);


/*
========================================================================
Routine Description:
	Initialize specific beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipBcnSpecInit(
	IN RTMP_ADAPTER			*pAd)
{
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	pChipCap->FlgIsSupSpecBcnBuf = TRUE;
	pChipCap->BcnMaxHwNum = 16;
	pChipCap->WcidHwRsvNum = 255;

/* 	In 16-MBSS support mode, if AP-Client is enabled, 
	the last 8-MBSS would be occupied for AP-Client using. */
#ifdef APCLI_SUPPORT
	pChipCap->BcnMaxNum = (8 - MAX_MESH_NUM);
#else
	pChipCap->BcnMaxNum = (16 - MAX_MESH_NUM);
#endif /* APCLI_SUPPORT */

	pChipCap->BcnMaxHwSize = 0x2000;

	/* It's allowed to use the higher(secordary) 8KB shared memory */
	pChipCap->BcnBase[0] = 0x4000;
	pChipCap->BcnBase[1] = 0x4200;
	pChipCap->BcnBase[2] = 0x4400;
	pChipCap->BcnBase[3] = 0x4600;
	pChipCap->BcnBase[4] = 0x4800;
	pChipCap->BcnBase[5] = 0x4A00;
	pChipCap->BcnBase[6] = 0x4C00;
	pChipCap->BcnBase[7] = 0x4E00;
	pChipCap->BcnBase[8] = 0x5000;
	pChipCap->BcnBase[9] = 0x5200;
	pChipCap->BcnBase[10] = 0x5400;
	pChipCap->BcnBase[11] = 0x5600;
	pChipCap->BcnBase[12] = 0x5800;
	pChipCap->BcnBase[13] = 0x5A00;
	pChipCap->BcnBase[14] = 0x5C00;
	pChipCap->BcnBase[15] = 0x5E00;

	pAd->chipOps.BeaconUpdate = RtmpChipWriteHighMemory;

	DBGPRINT(RT_DEBUG_TRACE, ("< Beacon Spec Information: >\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwNum = \t%d\n", pChipCap->BcnMaxHwNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxNum = \t%d\n", pChipCap->BcnMaxNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwSize = \t0x%x\n", pChipCap->BcnMaxHwSize));
	DBGPRINT(RT_DEBUG_TRACE, ("\tWcidHwRsvNum = \t%d\n", pChipCap->WcidHwRsvNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[0] = \t0x%x\n", pChipCap->BcnBase[0]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[8] = \t0x%x\n", pChipCap->BcnBase[8]));
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
}


/*
========================================================================
Routine Description:
	Initialize normal beacon frame architecture.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipBcnInit(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	pChipCap->FlgIsSupSpecBcnBuf = FALSE;
	pChipCap->BcnMaxHwNum = 8;
	pChipCap->BcnMaxNum = (pChipCap->BcnMaxHwNum - MAX_MESH_NUM - MAX_APCLI_NUM);
	pChipCap->BcnMaxHwSize = 0x1000;

	pChipCap->BcnBase[0] = 0x7800;
	pChipCap->BcnBase[1] = 0x7A00;
	pChipCap->BcnBase[2] = 0x7C00;
	pChipCap->BcnBase[3] = 0x7E00;
	pChipCap->BcnBase[4] = 0x7200;
	pChipCap->BcnBase[5] = 0x7400;
	pChipCap->BcnBase[6] = 0x5DC0;
	pChipCap->BcnBase[7] = 0x5BC0;

	/* If the MAX_MBSSID_NUM is larger than 6, */
	/* it shall reserve some WCID space(wcid 222~253) for beacon frames. */
	/* -	these wcid 238~253 are reserved for beacon#6(ra6).*/
	/* -	these wcid 222~237 are reserved for beacon#7(ra7).*/
	if (pChipCap->BcnMaxNum == 8)
		pChipCap->WcidHwRsvNum = 222;
	else if (pChipCap->BcnMaxNum == 7)
		pChipCap->WcidHwRsvNum = 238;
	else
		pChipCap->WcidHwRsvNum = 255;
	
	pAd->chipOps.BeaconUpdate = RtmpChipWriteMemory;

	DBGPRINT(RT_DEBUG_TRACE, ("< Beacon Information: >\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwNum = \t%d\n", pChipCap->BcnMaxHwNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxNum = \t%d\n", pChipCap->BcnMaxNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwSize = \t0x%x\n", pChipCap->BcnMaxHwSize));
	DBGPRINT(RT_DEBUG_TRACE, ("\tWcidHwRsvNum = \t%d\n", pChipCap->WcidHwRsvNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[0] = \t0x%x\n", pChipCap->BcnBase[0]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[1] = \t0x%x\n", pChipCap->BcnBase[1]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[2] = \t0x%x\n", pChipCap->BcnBase[2]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[3] = \t0x%x\n", pChipCap->BcnBase[3]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[4] = \t0x%x\n", pChipCap->BcnBase[4]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[5] = \t0x%x\n", pChipCap->BcnBase[5]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[6] = \t0x%x\n", pChipCap->BcnBase[6]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnBase[7] = \t0x%x\n", pChipCap->BcnBase[7]));
}

/*
========================================================================
Routine Description:
	write high memory.
	if firmware do not support auto high/low memory switching, we should switch to high memory by ourself.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Memory offsets
	Value			- Written value
	Unit				- Unit in "Byte"

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipWriteHighMemory(
	IN	RTMP_ADAPTER	*pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit)
{
#ifdef RTMP_MAC_PCI
#ifdef SPECIFIC_BCN_BUF_SUPPORT
unsigned long irqFlag = 0;
	RTMP_MAC_SHR_MSEL_LOCK(pAd, HIGHER_SHRMEM, irqFlag);
	RtmpChipWriteMemory(pAd, Offset, Value, Unit);
	RTMP_MAC_SHR_MSEL_UNLOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
#endif /* RTMP_MAC_PCI */
}

/*
========================================================================
Routine Description:
	write memory

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Memory offsets
	Value			- Written value
	Unit				- Unit in "Byte"
Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipWriteMemory(
	IN	RTMP_ADAPTER	*pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit)
{
	switch(Unit)
	{
		case 1:
			RTMP_IO_WRITE8(pAd, Offset, Value);
			break;
		case 2:
			RTMP_IO_WRITE16(pAd, Offset, Value);
			break;
		case 4:
			RTMP_IO_WRITE32(pAd, Offset, Value);
		default:
			break;
	}
}

/*
========================================================================
Routine Description:
	Initialize chip related information.

Arguments:
	pCB				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpChipOpsHook(
	IN VOID			*pCB)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pCB;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	UINT32 MacValue;
	UCHAR i = 0;


	/* sanity check */
	do
	{
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacValue);

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return;

		if ((MacValue != 0x00) && (MacValue != 0xFFFFFFFF))
			break;

		RTMPusecDelay(10);
	} while (i++ < 100);

	pAd->MACVersion = MacValue;

	/* default init */
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_LEGACY);

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
#ifdef RT2880
		RTMP_SYS_IO_READ32(0xa030000c, &pAd->CommonCfg.CID);
#else
		RTMP_SYS_IO_READ32(0xb000000c, &pAd->CommonCfg.CID);
		RTMP_SYS_IO_READ32(0xb0000000, &pAd->CommonCfg.CN);
#endif /* RT2880 */

#ifdef RT6352
		pAd->CommonCfg.PKG_ID = (UCHAR)((pAd->CommonCfg.CID >> 16) & 0x0001);
		pAd->CommonCfg.Chip_VerID = (UCHAR)((pAd->CommonCfg.CID >> 8) & 0x0f);
		pAd->CommonCfg.Chip_E_Number = (UCHAR)((pAd->CommonCfg.CID) & 0x0f);
#endif /* RT6352 */
		DBGPRINT(RT_DEBUG_TRACE, ("CN: %lx\tCID = %lx\n",
				pAd->CommonCfg.CN, pAd->CommonCfg.CID));
	}
#endif /* RTMP_RBUS_SUPPORT */

#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = EnableAPMIMOPSv1;
	pChipOps->DisableAPMIMOPS = DisableAPMIMOPSv1;
#endif /* GREENAP_SUPPORT */

#ifdef RT3883
	if (IS_RT3883(pAd))
	{
		RT3883_Init(pAd);
		goto done;
	}
#endif /* RT3883 */


	/* init default value whatever chipsets */
	/* default pChipOps content will be 0x00 */
	pChipCap->bbpRegTbSize = 0;
	pChipCap->MaxNumOfRfId = 31;
	pChipCap->MaxNumOfBbpId = 136;
	pChipCap->SnrFormula = SNR_FORMULA1;
	pChipCap->RfReg17WtMethod = RF_REG_WT_METHOD_NONE;
	pChipCap->TXWISize = 16;
	pChipCap->RXWISize = 16;
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	pChipCap->TxPowerTuningTable_2G = TxPowerTuningTableOrg;
#ifdef A_BAND_SUPPORT
	pChipCap->TxPowerTuningTable_5G = TxPowerTuningTableOrg;
#endif /* A_BAND_SUPPORT */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */
	pChipOps->AsicMacInit = NULL;
	pChipOps->AsicBbpInit = NULL;
	pChipOps->AsicRfInit = NULL;

#ifdef RTMP_EFUSE_SUPPORT
	pChipCap->EFUSE_USAGE_MAP_START = 0x2d0;
	pChipCap->EFUSE_USAGE_MAP_END = 0x2fc;      
       	pChipCap->EFUSE_USAGE_MAP_SIZE = 45;

	DBGPRINT(RT_DEBUG_TRACE, ("Efuse Size=0x%x [%x-%x] \n",pAd->chipCap.EFUSE_USAGE_MAP_SIZE,pAd->chipCap.EFUSE_USAGE_MAP_START,pAd->chipCap.EFUSE_USAGE_MAP_END));
#endif /* RTMP_EFUSE_SUPPORT */

	pChipCap->VcoPeriod = 10;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_DISABLE;
	pChipCap->WPDMABurstSIZE = 2; /* default 64B */
	pChipCap->MBSSIDMode = MBSSID_MODE0; 


	RtmpChipBcnInit(pAd);

	pChipOps->RxSensitivityTuning = RxSensitivityTuning;
#ifdef CONFIG_STA_SUPPORT
	pChipOps->ChipAGCAdjust = ChipAGCAdjust;
#endif /* CONFIG_STA_SUPPORT */
	pChipOps->ChipBBPAdjust = ChipBBPAdjust;
	pChipOps->ChipSwitchChannel = ChipSwitchChannel;

	/* TX ALC */
	pChipCap->bTempCompTxALC = FALSE;
	pChipOps->AsicGetTxPowerOffset = NULL;
	pChipOps->InitDesiredTSSITable = NULL;
	pChipOps->AsicTxAlcGetAutoAgcOffset = NULL;
	pChipOps->AsicExtraPowerOverMAC = NULL;
	pChipOps->AsicAdjustTxPower = NULL;

	pChipOps->ChipAGCInit = Default_ChipAGCInit;
	pChipOps->AsicAntennaDefaultReset = AsicAntennaDefaultReset;
	pChipOps->NetDevNickNameInit = NetDevNickNameInit;
	/* Init value. If pChipOps->AsicResetBbpAgent==NULL, "AsicResetBbpAgent" as default. If your chipset has specific routine, please re-hook it at self init function */
	pChipOps->AsicResetBbpAgent = NULL;


#ifdef RT28xx
	pChipOps->ChipSwitchChannel = RT28xx_ChipSwitchChannel;
#endif /* RT28xx */
#ifdef CARRIER_DETECTION_SUPPORT
	pChipCap->carrier_func = DISABLE_TONE_RADAR;
	pChipOps->ToneRadarProgram = NULL;
#endif /* CARRIER_DETECTOIN_SUPPORT */
#ifdef DFS_SUPPORT
	pChipCap->DfsEngineNum = 4;
#endif /* DFS_SUPPORT */
	pChipOps->CckMrcStatusCtrl = NULL;
	pChipOps->RadarGLRTCompensate = NULL;


	/* We depends on RfICType and MACVersion to assign the corresponding operation callbacks. */
#ifdef RT2880
	if (IS_RT2880(pAd))
		RT2880_Init(pAd);
#endif /* RT2880 */

#ifdef RT305x
#ifdef RT3352
	/*FIXME by Steven: RFIC=RFIC_3022 in some RT3352 board*/
/*	if (pAd->RfIcType == RFIC_3322) {*/
	if (IS_RT3352(pAd))
	{
		RT3352_Init(pAd);
	} 
	else
#endif /* RT3352 */
#ifdef RT5350
	if (IS_RT5350(pAd))
	{
		RT5350_Init(pAd);
	}
	else
#endif /* RT5350 */
/* comment : the RfIcType is not ready yet, because EEPROM doesn't be initialized. */
/*	if ((pAd->MACVersion == 0x28720200) && 
		((pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022))) */
	if (IS_RT3050_3052_3350(pAd))
	{
		RT305x_Init(pAd);
	}
	else
#endif /* RT305x */
#ifdef RT2883
	if (IS_RT2883(pAd) && (pAd->infType == RTMP_DEV_INF_RBUS))
	{
		pChipOps->AsicBbpInit = NICInitRT2883BbpRegisters;
		pChipCap->SnrFormula = SNR_FORMULA2;
		pChipCap->MaxNumOfBbpId = 180;
		pChipCap->TXWISize = 16;
		pChipCap->RXWISize = 20;
		pChipOps->AsicMacInit = NICInitRT2883MacRegisters;
	}
	else
#endif /* RT2883 */


#ifdef RT6352
	if (IS_RT6352(pAd))
		RT6352_Init(pAd);
#endif /* RT6352 */

#if defined(RT3883) || defined(RT3290)
done:
#endif /* defined(RT3883) || defined(RT3290) */
	DBGPRINT(RT_DEBUG_TRACE, ("Chip specific bbpRegTbSize=%d!\n", pChipCap->bbpRegTbSize));
	DBGPRINT(RT_DEBUG_TRACE, ("Chip VCO calibration mode = %d!\n", pChipCap->FlgIsVcoReCalMode));
}

#ifdef GREENAP_SUPPORT
VOID EnableAPMIMOPSv2(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR	BBPR3 = 0;
	UINT32 	macdata = 0;

	/* enable MMPS BBP control register*/
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 |= 0x04;	/*bit 2*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

#if defined(RT6352)
	if (IS_RT6352(pAd))
	{
		UCHAR BBPR95 = 0;
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R95, &BBPR95);
		BBPR95 &= ~(0x80); /* bit 7 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BBPR95);
	}
#endif

	/* enable MMPS MAC control register*/
	RTMP_IO_READ32(pAd, 0x1210, &macdata);
	macdata |= 0x09;	/*bit 0, 3*/
	RTMP_IO_WRITE32(pAd, 0x1210, macdata);

#if defined(RT3883) || defined(RT6352)
	/* swith to one-PAPE mode */
	RTMP_IO_READ32(pAd, TXOP_HLDR_ET, &macdata);
	macdata = (macdata & (~0x18)) | 0x8;
	RTMP_IO_WRITE32(pAd, TXOP_HLDR_ET, macdata);
#endif

	DBGPRINT(RT_DEBUG_INFO, ("EnableAPMIMOPSNew, 30xx changes the # of antenna to 1\n"));
}

VOID DisableAPMIMOPSv2(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR	BBPR3 = 0;
	UINT32 	macdata = 0;
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= ~(0x04);	/*bit 2*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

#if defined(RT6352)
	if (IS_RT6352(pAd))
	{
		if (pAd->Antenna.field.RxPath > 1)
		{
			UCHAR BBPR95 = 0;
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R95, &BBPR95);
			BBPR95 |= 0x80;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BBPR95);
		}
	}
#endif

	/* enable MMPS MAC control register*/
	RTMP_IO_READ32(pAd, 0x1210, &macdata);
	macdata &= ~(0x09);	/*bit 0, 3*/
	RTMP_IO_WRITE32(pAd, 0x1210, macdata);

#if defined(RT3883) || defined(RT6352)
	/* disable one-PAPE mode */
	RTMP_IO_READ32(pAd, TXOP_HLDR_ET, &macdata);
	macdata &= ~(0x18);
	RTMP_IO_WRITE32(pAd, TXOP_HLDR_ET, macdata);
#endif

	DBGPRINT(RT_DEBUG_INFO, ("DisableAPMIMOPSNew, 30xx reserve only one antenna\n"));
}

VOID EnableAPMIMOPSv1(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR	BBPR3 = 0,BBPR1 = 0;
	ULONG	TxPinCfg;
	UCHAR	BBPR4=0;

	UCHAR	CentralChannel;
	/*UINT32	Value=0;*/

#ifdef RT305x
	UCHAR 	RFValue=0;
		
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue &= 0x03;	//clear bit[7~2]
	RFValue |= 0xF0;
	// Turn off unused PA or LNA when only 1T or 1R
#endif // RT305x //

	if(pAd->CommonCfg.Channel <= 14)
	{
		TxPinCfg = 0x00050F0A;
		
		// Turn off unused PA or LNA when only 1T/1R
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd))
		{
			TxPinCfg = 0x32050F0A;
			
			//Disable unused PA_PE
			TxPinCfg = TxPinCfg & ~0x0300000D;
			
			//Disable unused LNA_PE
			TxPinCfg = TxPinCfg & ~0x30000C00;
		}
		else
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) */
		{
			TxPinCfg &= 0xFFFFFFF3;
			TxPinCfg &= 0xFFFFF3FF;
		}
	}
	else
	{
		TxPinCfg = 0x00050F05;
		
		// Turn off unused PA or LNA when only 1T/1R
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd))
		{
			TxPinCfg = 0x31050F05;
			
			//Disable unused PA_PE
			TxPinCfg = TxPinCfg & ~0x0300000E;
			
			//Disable unused LNA_PE
			TxPinCfg = TxPinCfg & ~0x30000C00;
		}
		else
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) */
		{
			TxPinCfg &= 0xFFFFFFF3;
			TxPinCfg &= 0xFFFFF3FF;
		}
	}

	pAd->ApCfg.bGreenAPActive=TRUE;

	CentralChannel = pAd->CommonCfg.CentralChannel;

	DBGPRINT(RT_DEBUG_INFO, ("Run with BW_20\n"));
	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	CentralChannel = pAd->CommonCfg.Channel;

	/* Set BBP registers to BW20 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);
	BBPR4 &= (~0x18);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPR4);

	/* RF Bandwidth related registers would be set in AsicSwitchChannel() */
	pAd->CommonCfg.BBPCurrentBW = BW_20;

	if (pAd->Antenna.field.RxPath > 1 || pAd->Antenna.field.TxPath > 1)
	{
		/*TX Stream*/
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
		/*Rx Stream*/
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
		
		BBPR3 &= (~0x18);
		BBPR1 &= (~0x18);
		
#ifdef RT3352
		/*
			For power saving purpose, Gary set BBP_R3[7:6]=11 to save more power
			and he also rewrote the description about BBP_R3 to point out the
			WiFi driver should modify BBP_R3[5] based on Low/High frequency
			channel.(not a fixed value).
		*/
		BBPR3 |= 0xe0;	//bit 6 & 7, i.e. Use 5-bit ADC for Acquisition
#endif /* RT3352 */

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
		
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
#ifdef RT305x
		RT30xxWriteRFRegister(pAd, RF_R01, RFValue);
#endif /* RT305x */
	}

	AsicSwitchChannel(pAd, CentralChannel, FALSE);

	DBGPRINT(RT_DEBUG_INFO, ("EnableAPMIMOPS, 305x/28xx changes the # of antenna to 1\n"));
}

VOID DisableAPMIMOPSv1(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR	BBPR3=0,BBPR1=0,BBPVal=0;
	ULONG	TxPinCfg;
	UCHAR	CentralChannel;
	UINT32	Value=0;

#ifdef RT305x
	UCHAR 	RFValue=0;

	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue &= 0x03;	//clear bit[7~2]
	if (pAd->Antenna.field.TxPath == 1)
		RFValue |= 0xA0;
	else if (pAd->Antenna.field.TxPath == 2)
		RFValue |= 0x80;
	if (pAd->Antenna.field.RxPath == 1)
		RFValue |= 0x50;
	else if (pAd->Antenna.field.RxPath == 2)
		RFValue |= 0x40;
#endif // RT305x //

	if(pAd->CommonCfg.Channel <= 14)
	{
		TxPinCfg = 0x00050F0A;
		
		// Turn off unused PA or LNA when only 1T/1R, 2T/2R
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd))
		{
			TxPinCfg = 0x32050F0A;
			
			// Disable unused PA_PE
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg = TxPinCfg & ~0x0300000D;
			}
			else if (pAd->Antenna.field.TxPath == 2)
			{
				TxPinCfg = TxPinCfg & ~0x03000005;
			}
			
			// Disable unused LNA_PE
			if (pAd->Antenna.field.RxPath == 1)
			{
				TxPinCfg = TxPinCfg & ~0x30000C00;
			}
			else if (pAd->Antenna.field.RxPath == 2)
			{
				TxPinCfg = TxPinCfg & ~0x30000000;
			}
		}
		else
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) */
		{
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg &= 0xFFFFFFF3;
			}
			
			if (pAd->Antenna.field.RxPath == 1)
			{
				TxPinCfg &= 0xFFFFF3FF;
			}
		}
	}
	else
	{
		TxPinCfg = 0x00050F05;
		
		// Turn off unused PA or LNA when only 1T/1R, 2T/2R
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd))
		{
			TxPinCfg = 0x31050F05;
			
			//Disable unused PA_PE
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg = TxPinCfg & ~0x0300000E;
			}
			else if (pAd->Antenna.field.TxPath == 2)
			{
				TxPinCfg = TxPinCfg & ~0x0300000A;
			}
			
			//Disable unused LNA_PE
			if (pAd->Antenna.field.RxPath == 1)
			{
				TxPinCfg = TxPinCfg & ~0x30000C00;
			}
			else if (pAd->Antenna.field.RxPath == 2)
			{
				TxPinCfg = TxPinCfg & ~0x30000000;
			}
		}
		else
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) */
		{
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg &= 0xFFFFFFF3;
			}
			
			if (pAd->Antenna.field.RxPath == 1)
			{
				TxPinCfg &= 0xFFFFF3FF;
			}
		}
	}

	pAd->ApCfg.bGreenAPActive=FALSE;

	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) && (pAd->CommonCfg.Channel != 14))
	{
		DBGPRINT(RT_DEBUG_INFO, ("Run with BW_40\n"));
		
		/* Set CentralChannel to work for BW40 */
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
			
			/*  TX : control channel at lower */
			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
			Value &= (~0x1);
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
			
			/*  RX : control channel at lower */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPVal);
			BBPVal &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPVal);
		}
		else if ((pAd->CommonCfg.Channel > 2) && (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW)) 
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
			
			/*  TX : control channel at upper */
			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
			Value |= (0x1);
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
			
			/*  RX : control channel at upper */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPVal);
			BBPVal |= (0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPVal);
		}
		
		CentralChannel = pAd->CommonCfg.CentralChannel;
		
		/* Set BBP registers to BW40 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPVal);
		BBPVal &= (~0x18);
		BBPVal |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPVal);
		
		/* RF Bandwidth related registers would be set in AsicSwitchChannel() */
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, CentralChannel, FALSE);
	}
	
	/*Rx Stream*/
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
	BBPR1 &= (~0x18);

	/*Tx Stream*/
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= (~0x18);

	/*RX Stream*/
	if(pAd->Antenna.field.RxPath == 3)
	{
		BBPR3 |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		BBPR3 |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		BBPR3 |= (0x0);
	}

	/*Tx Stream*/
	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->Antenna.field.TxPath >= 2))
	{
		BBPR1 |= 0x10;
	}

#ifdef RT3352
	/*
		For power saving purpose, Gary set BBP_R3[7:6]=11 to save more power
		and he also rewrote the description about BBP_R3 to point out the
		WiFi driver should modify BBP_R3[5] based on Low/High frequency
		channel.(not a fixed value).
	*/
	BBPR3 &= (~0xe0);	//bit 6 & 7, i.e. Use 5-bit ADC for Acquisition
#endif /* RT3352 */

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

#ifdef RT305x
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);
#endif /* RT305x */

	DBGPRINT(RT_DEBUG_INFO, ("DisableAPMIMOPS, 305x/28xx reserve only one antenna\n"));
}
#endif /* GREENAP_SUPPORT */


static VOID RxSensitivityTuning(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR R66;


	R66 = 0x26 + GET_LNA_GAIN(pAd);
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
#ifdef RT2883
		if (IS_RT2883(pAd))
		{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x0);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x20);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x40);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
		}
		else
#endif /* RT2883*/
#ifdef RT3352
		if (IS_RT3352(pAd))
		{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x0);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x20);
		}
		else
#endif /* RT3352 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
	}
	else
#endif /* RALINK_ATE */
	{
		AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL);
	}
	DBGPRINT(RT_DEBUG_TRACE,("turn off R17 tuning, restore to 0x%02x\n", R66));
}


#ifdef CONFIG_STA_SUPPORT
static UCHAR ChipAGCAdjust(
	IN PRTMP_ADAPTER		pAd,
	IN CHAR					Rssi,
	IN UCHAR				OrigR66Value)
{
	UCHAR R66 = OrigR66Value;
	CHAR lanGain = GET_LNA_GAIN(pAd);
	
	if (pAd->LatchRfRegs.Channel <= 14)
	{	/*BG band*/
		R66 = 0x2E + lanGain;
		if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
			R66 += 0x10;
	}
	else
	{	/*A band*/
		if (pAd->CommonCfg.BBPCurrentBW == BW_20)
			R66 = 0x32 + (lanGain * 5) / 3;
		else
			R66 = 0x3A + (lanGain * 5) / 3;

		if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				R66 += 0x10;
	}

	if (OrigR66Value != R66)
		AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);	

	return R66;
}
#endif /* CONFIG_STA_SUPPORT */


static VOID ChipBBPAdjust(
	IN RTMP_ADAPTER			*pAd)
{
	UINT32 Value;
	UCHAR byteValue = 0;


#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
		(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		/*(pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)*/
	)
	{
		{
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		}
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

		if (pAd->CommonCfg.Channel > 14)
		{ 	/* request by Gary 20070208 for middle and long range A Band*/
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x48, RX_CHAIN_ALL);
		}
		else
		{	/* request by Gary 20070208 for middle and long range G Band*/
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x38, RX_CHAIN_ALL);
		}	
		/* */
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);
		}	

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
		
		if (pAd->CommonCfg.Channel > 14)
		{ 	/* request by Gary 20070208 for middle and long range A Band*/
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x48, RX_CHAIN_ALL);
		}
		else
		{ 	/* request by Gary 20070208 for middle and long range G band*/
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x38, RX_CHAIN_ALL);
		}	
	
		
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
		}
		else
		{	
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);
		}
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
		if (pAd->CommonCfg.Channel > 14)
		{	 /* request by Gary 20070208*/
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x40, RX_CHAIN_ALL);
		}	
		else
		{	/* request by Gary 20070208*/
			/*AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x30, RX_CHAIN_ALL); */
			/* request by Brian 20070306*/
			AsicBBPWriteWithRxChain(pAd, BBP_R66, 0x38, RX_CHAIN_ALL);
		}	
				 
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x12);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0a);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x10);
		}

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
		/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x1D);*/
	}
	else
	{ 	/* request by Gary 20070208 for middle and long range G band*/
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x2D);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x2D);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x2D);
			/*RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x2D);*/
	}	
}


static VOID ChipSwitchChannel(
	IN PRTMP_ADAPTER 		pAd,
	IN UCHAR				Channel,
	IN BOOLEAN				bScan) 
{
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; /*Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER;*/
	UCHAR	index;
	UINT32 	Value = 0; /*BbpReg, Value;*/
	UCHAR 	RFValue;
	UINT32 i = 0;

	i = i; /* avoid compile warning */
	RFValue = 0;
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
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: Can't find the Channel#%d \n", Channel));
	}

	{
#if defined(RT28xx)
		ULONG	R2 = 0, R3 = DEFAULT_RF_TX_POWER, R4 = 0;
		RTMP_RF_REGS *RFRegTable;

		RFRegTable = RF2850RegTable;
#endif /* Rdefined(RT28xx) */
	
		switch (pAd->RfIcType)
		{
#if defined(RT28xx)
#if defined(RT28xx)
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:
#endif /* defined(RT28xx) */
				for (index = 0; index < NUM_OF_2850_CHNL; index++)
				{
					if (Channel == RFRegTable[index].Channel)
					{
						R2 = RFRegTable[index].R2;
						if (pAd->Antenna.field.TxPath == 1)
						{
							R2 |= 0x4000;	/* If TXpath is 1, bit 14 = 1;*/
						}

						if (pAd->Antenna.field.RxPath == 2
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == FALSE)
#endif /* GREENAP_SUPPORT */
)
						{
							R2 |= 0x40;	/* write 1 to off Rxpath.*/
						}
						else if (pAd->Antenna.field.RxPath == 1
#ifdef GREENAP_SUPPORT
			|| (pAd->ApCfg.bGreenAPActive == TRUE)
#endif /* GREENAP_SUPPORT */
)
						{
							R2 |= 0x20040;	/* write 1 to off RxPath*/
						}

						if (Channel > 14)
						{
							/* initialize R3, R4*/
							R3 = (RFRegTable[index].R3 & 0xffffc1ff);
							R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->RfFreqOffset << 15);

							/* 5G band power range: 0xF9~0X0F, TX0 Reg3 bit9/TX1 Reg4 bit6="0" means the TX power reduce 7dB*/
							/* R3*/
							if ((TxPwer >= -7) && (TxPwer < 0))
							{
								TxPwer = (7+TxPwer);

								/* TxPwer is not possible larger than 15 */
/*								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);*/

								R3 |= (TxPwer << 10);
								DBGPRINT(RT_DEBUG_TRACE, ("AsicSwitchChannel: TxPwer=%d \n", TxPwer));
							}
							else
							{
								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
								R3 |= (TxPwer << 10) | (1 << 9);
							}

							/* R4*/
							if ((TxPwer2 >= -7) && (TxPwer2 < 0))
							{
								TxPwer2 = (7+TxPwer2);
								R4 |= (TxPwer2 << 7);
								DBGPRINT(RT_DEBUG_TRACE, ("AsicSwitchChannel: TxPwer2=%d \n", TxPwer2));
							}
							else
							{
								TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
								R4 |= (TxPwer2 << 7) | (1 << 6);
							}                        
						}
						else
						{
							R3 = (RFRegTable[index].R3 & 0xffffc1ff) | (TxPwer << 9); /* set TX power0*/
							R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->RfFreqOffset << 15) | (TxPwer2 <<6);/* Set freq Offset & TxPwr1*/
						}

						/* Based on BBP current mode before changing RF channel.*/
						if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif /* GREENAP_SUPPORT */
							)
						{
							R4 |=0x200000;
						}

						/* Update variables*/
						pAd->LatchRfRegs.Channel = Channel;
						pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
						pAd->LatchRfRegs.R2 = R2;
						pAd->LatchRfRegs.R3 = R3;
						pAd->LatchRfRegs.R4 = R4;

						/* Set RF value 1's set R3[bit2] = [0]*/
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
						RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

						RTMPusecDelay(200);

						/* Set RF value 2's set R3[bit2] = [1]*/
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
						RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 | 0x04));
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

						RTMPusecDelay(200);

						/* Set RF value 3's set R3[bit2] = [0]*/
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
						RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

						break;
					}
				}

				DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%lu, Pwr1=%lu, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
							  Channel, 
							  pAd->RfIcType, 
							  (R3 & 0x00003e00) >> 9,
							  (R4 & 0x000007c0) >> 6,
							  pAd->Antenna.field.TxPath,
							  pAd->LatchRfRegs.R1, 
							  pAd->LatchRfRegs.R2, 
							  pAd->LatchRfRegs.R3, 
							  pAd->LatchRfRegs.R4));
			
				break;
#endif /* defined(RT28xx) */
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d : unknown RFIC=%d\n",
					  Channel, pAd->RfIcType));
				break;
		}	
	}

	/* Change BBP setting during siwtch from a->g, g->a*/
	if (Channel <= 14)
	{
		ULONG	TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);/*(0x44 - GET_LNA_GAIN(pAd)));	 According the Rory's suggestion to solve the middle range issue.*/

		/* Rx High power VGA offset for LNA select*/
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

		/* 5G band selection PIN, bit1 and bit2 are complement*/
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		{
			/* Turn off unused PA or LNA when only 1T or 1R*/
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
	else
	{
		ULONG	TxPinCfg = 0x00050F05;/*Gary 2007/8/9 0x050505*/
		UINT8	bbpValue;
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);/*(0x44 - GET_LNA_GAIN(pAd)));    According the Rory's suggestion to solve the middle range issue.     */

		/* Set the BBP_R82 value here */
		bbpValue = 0xF2;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, bbpValue);


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
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/* Turn off unused PA or LNA when only 1T or 1R*/
		{
			/* Turn off unused PA or LNA when only 1T or 1R*/
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

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	/* AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x2E + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL);*/
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	
	/* On 11A, We should delay and wait RF/BBP to be stable*/
	/* and the appropriate time should be 1000 micro seconds */
	/* 2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.*/
	
	RTMPusecDelay(1000);  
}

static VOID Default_ChipAGCInit(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				BandWidth)
{
	UCHAR	R66 = 0x30;
#ifdef RT2883
	UCHAR	byteValue = 0;
#endif // RT2883 //
	
	if (pAd->LatchRfRegs.Channel <= 14)
	{	// BG band
		{
			R66 = 0x2E + GET_LNA_GAIN(pAd);
#if defined(RT3352)
			if (IS_RT3352(pAd))
			{
				/* Gary 20100714: Update BBP R66 programming: */
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					R66 = (GET_LNA_GAIN(pAd)*2+0x1C);
				else
					R66 = (GET_LNA_GAIN(pAd)*2+0x24);
			}
#endif // defined(RT3352) //
		}
	}
	else
	{	//A band
		{	
			if (BandWidth == BW_20)
				R66 = (UCHAR)(0x32 + (GET_LNA_GAIN(pAd)*5)/3);
#ifdef DOT11_N_SUPPORT
			else
				R66 = (UCHAR)(0x3A + (GET_LNA_GAIN(pAd)*5)/3);
#endif // DOT11_N_SUPPORT //
		}
	}
	AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);

}


#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
UINT32 SetHWAntennaDivsersity(
	IN PRTMP_ADAPTER		pAd,
	IN BOOLEAN				Enable)
{
	if (Enable == TRUE)
	{
		UINT8 BBPValue = 0, RFValue = 0;
		USHORT value;

		// RF_R29 bit7:6
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_GAIN, value);
		
		RT30xxReadRFRegister(pAd, RF_R29, &RFValue);
		RFValue &= 0x3f; // clear bit7:6
		RFValue |= (value << 6);			
		RT30xxWriteRFRegister(pAd, RF_R29, RFValue);

		// BBP_R47 bit7=1
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
		BBPValue |= 0x80;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
	
		BBPValue = 0xbe;			
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
		BBPValue = 0xb0;			
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
		BBPValue = 0x23;			
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
		BBPValue = 0x3a;			
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R153, BBPValue);
		BBPValue = 0x10;			
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
		BBPValue = 0x3b;			
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R155, BBPValue);
		BBPValue = 0x04;			
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R253, BBPValue);

		DBGPRINT(RT_DEBUG_TRACE, ("HwAnDi> Enable!\n"));
	}
	else
	{
		UINT8 BBPValue = 0;

		/*
			main antenna: BBP_R152 bit7=1
			aux antenna: BBP_R152 bit7=0
		 */
		if (pAd->FixDefaultAntenna == 0)
		{
			/* fix to main antenna */
			/* do not care BBP R153, R155, R253 */
			BBPValue = 0x3e;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
			BBPValue = 0x30;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
			BBPValue = 0x23;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
			BBPValue = 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
		}
		else
		{
			/* fix to aux antenna */
			/* do not care BBP R153, R155, R253 */
			BBPValue = 0x3e;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R150, BBPValue);
			BBPValue = 0x30;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R151, BBPValue);
			BBPValue = 0xa3;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BBPValue);
			BBPValue = 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R154, BBPValue);
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HwAnDi> Disable!\n"));
	}

	return 0;
}
#endif // HW_ANTENNA_DIVERSITY_SUPPORT // 

static VOID AsicAntennaDefaultReset(
	IN PRTMP_ADAPTER		pAd,
	IN EEPROM_ANTENNA_STRUC	*pAntenna)
{
#if defined(RT2883) || defined(RT3593)
	if (IS_RT2883(pAd))
	{
		pAntenna->word = 0;
#ifdef RT2883
	if (IS_RT2883(pAd))
		pAntenna->field.RfIcType = RFIC_2853;
#endif /* RT2883 */
		pAntenna->field.TxPath = 3;
		pAntenna->field.RxPath = 3;
	}
	else
#endif /* defined(RT2883) || defined(RT3593) */
#ifdef RT5350
	if (IS_RT5350(pAd))
	{
		pAntenna->word = 0;
		pAntenna->field.RfIcType = RFIC_3320;
		pAntenna->field.TxPath = 1;
		pAntenna->field.RxPath = 1;
	}
	else
#endif /* RT5350 */
	{

		pAntenna->word = 0;
		pAntenna->field.RfIcType = RFIC_2820;
		pAntenna->field.TxPath = 1;
		pAntenna->field.RxPath = 2;
	}
	DBGPRINT(RT_DEBUG_WARN, ("E2PROM error, hard code as 0x%04x\n", pAntenna->word));	
}


VOID NetDevNickNameInit(
	IN PRTMP_ADAPTER		pAd)
{
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
		snprintf((PSTRING) pAd->nickname, sizeof(pAd->nickname), "RT2860STA");
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */
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
VOID	RTMPReadChannelPwr(
	IN	PRTMP_ADAPTER	pAd)
{
	UINT32					i, choffset;
	EEPROM_TX_PWR_STRUC	    Power;
	EEPROM_TX_PWR_STRUC	    Power2;
#if (defined(RT30xx) && defined(RTMP_MAC_PCI)) || defined(RT3593)
	UCHAR Tx0ALC = 0, Tx1ALC = 0, Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0;
#endif /* (defined(RT30xx) && defined(RTMP_MAC_PCI)) || defined(RT3593) */

	/* Read Tx power value for all channels*/
	/* Value from 1 - 0x7f. Default value is 24.*/
	/* Power value : 2.4G 0x00 (0) ~ 0x1F (31)*/
	/*             : 5.5G 0xF9 (-7) ~ 0x0F (15)*/

	/* 0. 11b/g, ch1 - ch 14*/
	for (i = 0; i < 7; i++)
	{

		{ /* Default routine. RT3070 and RT3370 run here. */
			UCHAR max_pwr = 31;

#ifdef RT6352
			if (IS_RT6352(pAd))
				max_pwr = 47;
#endif /* RT6352 */

			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

			pAd->TxPower[i * 2].Power = Power.field.Byte0;
			if(!IS_RT3390(pAd))  // 3370 has different Tx power range
			{
				if ((Power.field.Byte0 > max_pwr) || (Power.field.Byte0 < 0))
					pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			}				

			pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;
			if(!IS_RT3390(pAd)) // 3370 has different Tx power range
			{
				if ((Power.field.Byte1 > max_pwr) || (Power.field.Byte1 < 0))
					pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}				

			if ((Power2.field.Byte0 > max_pwr) || (Power2.field.Byte0 < 0))
				pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 > max_pwr) || (Power2.field.Byte1 < 0))
				pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
		}
	}
	

	{
		if (IS_RT5592(pAd))
			return;
		
		/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)*/
		/* 1.1 Fill up channel*/
		choffset = 14;
		for (i = 0; i < 4; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel	= 36 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel	= 36 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel	= 36 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}

		/* 1.2 Fill up power*/
		for (i = 0; i < 6; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			
		}
		
		/* 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz)*/
		/* 2.1 Fill up channel*/
		choffset = 14 + 12;
		for (i = 0; i < 5; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel	= 100 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel	= 100 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel	= 100 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}
		pAd->TxPower[3 * 5 + choffset + 0].Channel		= 140;
		pAd->TxPower[3 * 5 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 5 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;

		/* 2.2 Fill up power*/
		for (i = 0; i < 8; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			
		}

		/* 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165, 167, 169; 171, 173 (including central frequency in BW 40MHz)*/
		/* 3.1 Fill up channel*/
		choffset = 14 + 12 + 16;
		/*for (i = 0; i < 2; i++)*/
		for (i = 0; i < 3; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel	= 149 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel	= 149 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel	= 149 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}
		pAd->TxPower[3 * 3 + choffset + 0].Channel		= 171;
		pAd->TxPower[3 * 3 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * 3 + choffset + 1].Channel		= 173;
		pAd->TxPower[3 * 3 + choffset + 1].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 1].Power2		= DEFAULT_RF_TX_POWER;

		/* 3.2 Fill up power*/
		/*for (i = 0; i < 4; i++)*/
		for (i = 0; i < 6; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			
		}
	}


	/* 4. Print and Debug*/
	/*choffset = 14 + 12 + 16 + 7;*/
	choffset = 14 + 12 + 16 + 11;
	

}


NDIS_STATUS AsicBBPWriteWithRxChain(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR bbpId,
	IN CHAR bbpVal,
	IN RX_CHAIN_IDX rx_ch_idx)
{
	UCHAR idx = 0, val = 0;

	if (((pAd->MACVersion & 0xf0000000) < 0x28830000) || 
		(pAd->Antenna.field.RxPath == 1))
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpVal);
		return NDIS_STATUS_SUCCESS;
	}
	
	while (rx_ch_idx != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;
		
		if (rx_ch_idx & 0x01)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &val);
			val = (val & (~0x60)) | (idx << 5);

#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, val);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpVal);
			}
#endif /* RTMP_MAC_PCI */

			DBGPRINT(RT_DEBUG_INFO, 
					("%s(Idx):Write(R%d,val:0x%x) to Chain(0x%x, idx:%d)\n",
						__FUNCTION__, bbpId, bbpVal, rx_ch_idx, idx));
		}
		rx_ch_idx >>= 1;
		idx++;
	}

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS AsicBBPReadWithRxChain(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR bbpId, 
	IN CHAR *pBbpVal,
	IN RX_CHAIN_IDX rx_ch_idx)
{
	UCHAR idx, val;

	if (((pAd->MACVersion & 0xffff0000) < 0x28830000) || 
		(pAd->Antenna.field.RxPath == 1))
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, pBbpVal);
		return NDIS_STATUS_SUCCESS;
	}

	idx = 0;
	while(rx_ch_idx != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;

		if (rx_ch_idx & 0x01)
		{
			val = 0;
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &val);
			val = (val & (~0x60)) | (idx << 5);

#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, val);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, pBbpVal);
			}
#endif /* RTMP_MAC_PCI */
			break;
		}
		rx_ch_idx >>= 1;
		idx++;
	}

	return NDIS_STATUS_SUCCESS;
}





INT WaitForAsicReady(
	IN RTMP_ADAPTER *pAd)
{
	UINT32 mac_val = 0, reg = MAC_CSR0;
	int idx = 0;

	do
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return FALSE;
		
		RTMP_IO_READ32(pAd, reg, &mac_val);
		if ((mac_val != 0x00) && (mac_val != 0xFFFFFFFF))
			return TRUE;

		RTMPusecDelay(10);
	} while (idx++ < 100);

	DBGPRINT(RT_DEBUG_ERROR,
				("%s(0x%x):AsicNotReady!\n",
				__FUNCTION__, mac_val));
	
	return TRUE;
}


INT AsicGetMacVersion(
	IN RTMP_ADAPTER *pAd)
{
	UINT32 reg = MAC_CSR0;


	if (WaitForAsicReady(pAd) == TRUE)
	{
		RTMP_IO_READ32(pAd, reg, &pAd->MACVersion);
		DBGPRINT(RT_DEBUG_OFF, ("MACVersion[Ver:Rev]=0x%08x\n",
					pAd->MACVersion));
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() failed!\n", __FUNCTION__));
		return FALSE;
	}
}

/* End of rtmp_chip.c */

