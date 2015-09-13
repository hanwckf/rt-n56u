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
#ifdef RT5390
	if (IS_RT5390R(pAd))
		pChipCap->EFUSE_USAGE_MAP_SIZE = 29;
	else
#endif /* RT5390 */
       	pChipCap->EFUSE_USAGE_MAP_SIZE = 45;

	DBGPRINT(RT_DEBUG_TRACE, ("Efuse Size=0x%x [%x-%x] \n",pAd->chipCap.EFUSE_USAGE_MAP_SIZE,pAd->chipCap.EFUSE_USAGE_MAP_START,pAd->chipCap.EFUSE_USAGE_MAP_END));
#endif /* RTMP_EFUSE_SUPPORT */

	pChipCap->VcoPeriod = 10;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_DISABLE;
	pChipCap->WPDMABurstSIZE = 3; /* default 128B */
	pChipCap->MBSSIDMode = MBSSID_MODE0; 


	RtmpChipBcnInit(pAd);

	pChipOps->RxSensitivityTuning = RxSensitivityTuning;
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
#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = EnableAPMIMOPSv1;
	pChipOps->DisableAPMIMOPS = DisableAPMIMOPSv1;
#endif /* GREENAP_SUPPORT */

	pChipOps->CckMrcStatusCtrl = NULL;
	pChipOps->RadarGLRTCompensate = NULL;


	/* We depends on RfICType and MACVersion to assign the corresponding operation callbacks. */

#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
	if (IS_RT5390(pAd) || IS_RT5392(pAd))
	{
		RT5390_Init(pAd);
	}
	else
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */
#ifdef RT30xx
#ifdef RT35xx
	if (IS_RT3572(pAd))
	{
		RT35xx_Init(pAd);
	}
	else
#endif /* RT35xx */

#ifdef RT3593
	if (IS_RT3593(pAd))
	{
		RT3593_Init(pAd);
	}
	else
#endif /* RT3593 */
	if (IS_RT30xx(pAd))
	{
#ifdef RT33xx
		if (IS_RT3390(pAd))
			RT33xx_Init(pAd);
		else
#endif /* RT33xx */
			RT30xx_Init(pAd);
	}
#endif /* RT30xx */

#ifdef RT5592
	if (IS_RT5592(pAd))
		RT5592_Init(pAd);
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("Chip specific bbpRegTbSize=%d!\n", pChipCap->bbpRegTbSize));
	DBGPRINT(RT_DEBUG_TRACE, ("Chip VCO calibration mode = %d!\n", pChipCap->FlgIsVcoReCalMode));
}

#ifdef GREENAP_SUPPORT
VOID EnableAPMIMOPSv2(
	IN PRTMP_ADAPTER		pAd,
	IN BOOLEAN				ReduceCorePower)
{
	UCHAR	BBPR3 = 0;
	UINT32 	macdata=0;

	/* enable MMPS BBP control register*/
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 |= 0x04;	/*bit 2*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

	/* enable MMPS MAC control register*/
	RTMP_IO_READ32(pAd, 0x1210, &macdata);
	macdata |= 0x09;	/*bit 0, 3*/
	RTMP_IO_WRITE32(pAd, 0x1210, macdata);
	
#if defined(RT35xx) || defined(RT5592)
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
	UCHAR	BBPR3=0;
	UINT32 	macdata=0;
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= ~(0x04);	/*bit 2*/
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

	/* enable MMPS MAC control register*/
	RTMP_IO_READ32(pAd, 0x1210, &macdata);
	macdata &= ~(0x09);	/*bit 0, 3*/
	RTMP_IO_WRITE32(pAd, 0x1210, macdata);
	
#if defined(RT35xx) || defined(RT5592)
	/* disable one-PAPE mode */
	RTMP_IO_READ32(pAd, TXOP_HLDR_ET, &macdata);
	macdata &= ~(0x18);
	RTMP_IO_WRITE32(pAd, TXOP_HLDR_ET, macdata);
#endif
	
	DBGPRINT(RT_DEBUG_INFO, ("DisableAPMIMOPSNew, 30xx reserve only one antenna\n"));
}

VOID EnableAPMIMOPSv1(
	IN PRTMP_ADAPTER		pAd,
	IN BOOLEAN				ReduceCorePower)
{
	UCHAR	BBPR3 = 0,BBPR1 = 0;
	ULONG	TxPinCfg = 0x00050F0A; /*Gary 2007/08/09 0x050A0A*/
	UCHAR	BBPR4=0;

	UCHAR	CentralChannel;
	/*UINT32	Value=0;*/

	if(pAd->CommonCfg.Channel>14)
		TxPinCfg=0x00050F05;
	
	TxPinCfg &= 0xFFFFFFF3;
	TxPinCfg &= 0xFFFFF3FF;
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
	if (pAd->Antenna.field.RxPath>1||pAd->Antenna.field.TxPath>1)
	{
		/*TX Stream*/
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
		/*Rx Stream*/
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
		
		BBPR3 &= (~0x18);
		BBPR1 &= (~0x18);
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);
		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
		
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
	AsicSwitchChannel(pAd, CentralChannel, FALSE);

	DBGPRINT(RT_DEBUG_INFO, ("EnableAPMIMOPS, 305x/28xx changes the # of antenna to 1\n"));
}

VOID DisableAPMIMOPSv1(
	IN PRTMP_ADAPTER		pAd)
{
	UCHAR	BBPR3=0,BBPR1=0,BBPVal=0;
	ULONG	TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/

	UCHAR	CentralChannel;
	UINT32	Value=0;

	if(pAd->CommonCfg.Channel>14)
		TxPinCfg=0x00050F05;

	/* Turn off unused PA or LNA when only 1T or 1R*/
	if (pAd->Antenna.field.TxPath == 1)
	{
		TxPinCfg &= 0xFFFFFFF3;
	}

	if (pAd->Antenna.field.RxPath == 1)
	{
		TxPinCfg &= 0xFFFFF3FF;
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
	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->Antenna.field.TxPath == 2))
	{
		BBPR1 &= (~0x18);
		BBPR1 |= 0x10;
	}
	else
	{
		BBPR1 &= (~0x18);
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

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
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
	}
	else
#endif /* RALINK_ATE */
	{
		AsicBBPWriteWithRxChain(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)), RX_CHAIN_ALL);
	}
	DBGPRINT(RT_DEBUG_TRACE,("turn off R17 tuning, restore to 0x%02x\n", R66));
}


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
	
	if (pAd->LatchRfRegs.Channel <= 14)
	{	// BG band
#ifdef RT30xx
		/* Gary was verified Amazon AP and find that RT307x has BBP_R66 invalid default value */
		if (IS_RT3070(pAd)||IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd) || IS_RT3593(pAd))
			R66 = 0x1C + 2*GET_LNA_GAIN(pAd);
		else
#endif // RT30xx //
		{
			R66 = 0x2E + GET_LNA_GAIN(pAd);
		}
	}
	else
	{	//A band
#ifdef RT35xx
		if (IS_RT3572(pAd) || IS_RT3593(pAd))
		{
			if (pAd->LatchRfRegs.Channel >= 100 && pAd->LatchRfRegs.Channel <= 128)
				R66 = (UCHAR)(0x27 + (GET_LNA_GAIN(pAd)*5)/3);
			else
				R66 = (UCHAR)(0x22 + (GET_LNA_GAIN(pAd)*5)/3);
		}
		else
#endif // RT35xx //
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
#ifdef RT30xx
	if(IS_RT3090(pAd))
	{
		pAntenna->word = 0;
		pAntenna->field.RfIcType = RFIC_3020;
		pAntenna->field.TxPath = 1;
		pAntenna->field.RxPath = 1;		
	}
	else
#endif /* RT30xx */
#ifdef RT35xx
	if(IS_RT3572(pAd))
	{
		pAntenna->word = 0;
		pAntenna->field.RfIcType = RFIC_3052;
		pAntenna->field.TxPath = 2;
		pAntenna->field.RxPath = 2;		
	}
	else
#endif /* RT35xx */
#ifdef RT33xx
	if (IS_RT3390(pAd))
	{
		pAntenna->word = 0;
		pAntenna->field.RfIcType = RFIC_3320;
		pAntenna->field.TxPath = 1;
		pAntenna->field.RxPath = 1;
	}
	else
#endif /* RT33xx */
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
		if (IS_RT5390(pAd) || IS_RT5392(pAd))
		{
			pAntenna->word = 0;
			pAntenna->field.RfIcType = 0xF; /* Reserved */
			pAntenna->field.BoardType = 1;
			if (IS_RT5392(pAd))
			{
				pAntenna->field.TxPath = 2;
				pAntenna->field.RxPath = 2;
			}
			else
			{
				pAntenna->field.TxPath = 1;
				pAntenna->field.RxPath = 1;
			}
		}
		else
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */
#if defined(RT2883) || defined(RT3593)
	if (IS_RT2883(pAd))
	{
		pAntenna->word = 0;
#ifdef RT3593
	if (IS_RT3593(pAd))
		pAntenna->field.RfIcType = RFIC_3053;
#endif /* RT3593 */
		pAntenna->field.TxPath = 3;
		pAntenna->field.RxPath = 3;
	}
	else
#endif /* defined(RT2883) || defined(RT3593) */
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
#ifdef RT3593
	EEPROM_TX_PWR_STRUC	    Power3;
/*	UCHAR Tx0ALC = 0, Tx1ALC = 0, Tx2ALC = 0, Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0, Tx2FinePowerCtrl = 0*/;
	UCHAR  Tx2ALC = 0, Tx2FinePowerCtrl = 0;
	
	EEPROM_ANTENNA_STRUC NICConfig0 = {{ 0 }};

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, NICConfig0.word);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: NICConfig0.field.TxPath = %d, NICConfig0.field.RxPath = %d\n", 
		__FUNCTION__, 
		NICConfig0.field.TxPath, 
		NICConfig0.field.RxPath));
#endif /* RT3593 */

	/* Read Tx power value for all channels*/
	/* Value from 1 - 0x7f. Default value is 24.*/
	/* Power value : 2.4G 0x00 (0) ~ 0x1F (31)*/
	/*             : 5.5G 0xF9 (-7) ~ 0x0F (15)*/

	/* 0. 11b/g, ch1 - ch 14*/
	for (i = 0; i < 7; i++)
	{
#ifdef RT30xx
#ifdef RTMP_MAC_PCI
		/* Tx power control over RF R12, RF R13 and BBP R109*/
		if ((IS_RT3090A(pAd) || IS_RT3390(pAd)) && pAd->infType==RTMP_DEV_INF_PCIE)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

			
			/* Tx0 power control*/
			
			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte0;
			Tx0FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

			if (IS_RT3390(pAd))
			{
				pAd->TxPower[i * 2].Power = Power.field.Byte0;
			}
			else
			{
				if (Tx0ALC > 31)
				{
					pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power = Tx0ALC;
				}
			}	

			if (Tx0FinePowerCtrl > 4)
			{
				pAd->TxPower[i * 2].Tx0FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
			}
			else
			{
				pAd->TxPower[i * 2].Tx0FinePowerCtrl = Tx0FinePowerCtrl;
			}

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte1;
			Tx0FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

			if (IS_RT3390(pAd))
			{
				pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;
			}
			else
			{
			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Power = Tx0ALC;
			}
			}

			if (Tx0FinePowerCtrl > 4)
			{
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl = Tx0FinePowerCtrl;
			}

			
			/* Tx1 power control*/
			
			if (pAd->Antenna.field.TxPath >= 2)
			{
				Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte0;
				Tx1FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power2.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx1ALC > 31)
				{
					pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power2 = Tx1ALC;
				}

				if (Tx1FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2].Tx1FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2].Tx1FinePowerCtrl = Tx1FinePowerCtrl;
				}

				Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte1;
				Tx1FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power2.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx1ALC > 31)
				{
					pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Power2 = Tx1ALC;
				}

				if (Tx1FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl = Tx1FinePowerCtrl;
				}
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPower[%d].Power = 0x%02X, pAd->TxPower[%d].Tx0FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power = 0x%02X, pAd->TxPower[%d].Tx0FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power2 = 0x%02X, pAd->TxPower[%d].Tx1FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power2 = 0x%02X, pAd->TxPower[%d].Tx1FinePowerCtrl = 0x%02X, \n", 
				__FUNCTION__, 
				i * 2, 
				pAd->TxPower[i * 2].Power, 
				i * 2, 
				pAd->TxPower[i * 2].Tx0FinePowerCtrl, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl, 
				i * 2, 
				pAd->TxPower[i * 2].Power2, 
				i * 2, 
				pAd->TxPower[i * 2].Tx1FinePowerCtrl, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power2, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl));
		}
		else /* Tx power control over RF R12 and RF R13*/
#endif /* RTMP_MAC_PCI */
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) || defined(RT5592) || defined(RT3290)
		if (IS_RT5390(pAd) || IS_RT5392(pAd) || IS_RT5592(pAd) || IS_RT3290(pAd))
		{
			 RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2,Power.word);
			if (IS_RT5392(pAd) || IS_RT5592(pAd))
			{
				RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2,Power2.word);
			}
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;
	
			if ((Power.field.Byte0 > 0x27) || (Power.field.Byte0 < 0))
			{
				pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2].Power = Power.field.Byte0;
			}
	
			if ((Power.field.Byte1 > 0x27) || (Power.field.Byte1 < 0))
			{
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;
			}
	
			if (IS_RT5392(pAd) || IS_RT5592(pAd))
			{
				if ((Power2.field.Byte0 > 0x27) || (Power2.field.Byte0 < 0))
				{
					pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;
				}
		
				if ((Power2.field.Byte1 > 0x27) || (Power2.field.Byte1 < 0))
				{
					pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
				}
			}
			
			DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPower[%d].Power = 0x%02X, TxPower[%d].Power = 0x%02X\n", 
				__FUNCTION__, 
				i * 2, 
				pAd->TxPower[i * 2].Power, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power));
			
			if (IS_RT5392(pAd) || IS_RT5592(pAd))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPower[%d].Power2 = 0x%02X, TxPower[%d].Power2 = 0x%02X\n", 
					__FUNCTION__, 
					i * 2, 
					pAd->TxPower[i * 2].Power2, 
					i * 2 + 1, 
					pAd->TxPower[i * 2 + 1].Power2));
			}
		}
		else
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) || defined(RT5592) */
#endif /* RT30xx */

#ifdef RT3593
		if (IS_RT3593(pAd))
		{
			if (NICConfig0.field.TxPath == 3)
			{
				RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX0_OVER_2DOT4G + (i * 2)), Power.word);
				RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX1_OVER_2DOT4G + (i * 2)), Power2.word);
				RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX2_OVER_2DOT4G + (i * 2)), Power3.word);
			}
			else
			{
				RT28xx_EEPROM_READ16(pAd, (EEPROM_G_TX_PWR_OFFSET + i * 2), Power.word);
				RT28xx_EEPROM_READ16(pAd, (EEPROM_G_TX2_PWR_OFFSET + i * 2), Power2.word);
			}
			
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

			
			/* Tx0 power control*/
			
			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte0;
			Tx0FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2].Power = Tx0ALC;
			}

			if (Tx0FinePowerCtrl > 4)
			{
				pAd->TxPower[i * 2].Tx0FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
			}
			else
			{
				pAd->TxPower[i * 2].Tx0FinePowerCtrl = Tx0FinePowerCtrl;
			}

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte1;
			Tx0FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Power = Tx0ALC;
			}

			if (Tx0FinePowerCtrl > 4)
			{
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl = Tx0FinePowerCtrl;
			}

			
			/* Tx1 power control*/
			
			if (NICConfig0.field.TxPath >= 2)
			{
				Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte0;
				Tx1FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power2.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx1ALC > 31)
				{
					pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power2 = Tx1ALC;
				}

				if (Tx1FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2].Tx1FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2].Tx1FinePowerCtrl = Tx1FinePowerCtrl;
				}

				Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte1;
				Tx1FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power2.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx1ALC > 31)
				{
					pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Power2 = Tx1ALC;
				}

				if (Tx1FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl = Tx1FinePowerCtrl;
				}
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPower[%d].Power = 0x%02X, pAd->TxPower[%d].Tx0FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power = 0x%02X, pAd->TxPower[%d].Tx0FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power2 = 0x%02X, pAd->TxPower[%d].Tx1FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power2 = 0x%02X, pAd->TxPower[%d].Tx1FinePowerCtrl = 0x%02X, \n", 
				__FUNCTION__, 
				i * 2, 
				pAd->TxPower[i * 2].Power, 
				i * 2, 
				pAd->TxPower[i * 2].Tx0FinePowerCtrl, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Tx0FinePowerCtrl, 
				i * 2, 
				pAd->TxPower[i * 2].Power2, 
				i * 2, 
				pAd->TxPower[i * 2].Tx1FinePowerCtrl, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power2, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Tx1FinePowerCtrl));

			
			/* Tx2 power control*/
			
			if (NICConfig0.field.TxPath == 3)
			{
				Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte0;
				Tx2FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power3.field.Byte0) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx2ALC > 31)
				{
					pAd->TxPower[i * 2].Power3 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power3 = Tx2ALC;
				}

				if (Tx2FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2].Tx2FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2].Tx2FinePowerCtrl = Tx2FinePowerCtrl;
				}

				Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte1;
				Tx2FinePowerCtrl = ((GET_TX_FINE_POWER_CTRL_BIT_MASK & Power3.field.Byte1) >> NUMBER_OF_BITS_FOR_TX_ALC);

				if (Tx2ALC > 31)
				{
					pAd->TxPower[i * 2 + 1].Power3 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Power3 = Tx2ALC;
				}

				if (Tx2FinePowerCtrl > 4)
				{
					pAd->TxPower[i * 2 + 1].Tx2FinePowerCtrl = DEFAULT_BBP_TX_FINE_POWER_CTRL;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Tx2FinePowerCtrl = Tx2FinePowerCtrl;
				}

				DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPower[%d].Power3 = 0x%02X, pAd->TxPower[%d].Tx2FinePowerCtrl = 0x%02X, pAd->TxPower[%d].Power3 = 0x%02X, pAd->TxPower[%d].Tx2FinePowerCtrl = 0x%02X\n", 
					__FUNCTION__, 
					i * 2, 
					pAd->TxPower[i * 2].Power3, 
					i * 2, 
					pAd->TxPower[i * 2].Tx2FinePowerCtrl, 
					i * 2 + 1, 
					pAd->TxPower[i * 2 + 1].Power3, 
					i * 2 + 1, 
					pAd->TxPower[i * 2 + 1].Tx2FinePowerCtrl));
			}
		}
		else /* Tx power control over RF R12 and RF R13*/
#endif /* RT3593 */
		{ /* Default routine. RT3070 and RT3370 run here. */
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

			pAd->TxPower[i * 2].Power = Power.field.Byte0;
			if(!IS_RT3390(pAd))  // 3370 has different Tx power range
			{
			if ((Power.field.Byte0 > 31) || (Power.field.Byte0 < 0))
				pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			}				

			pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;
			if(!IS_RT3390(pAd)) // 3370 has different Tx power range
			{
			if ((Power.field.Byte1 > 31) || (Power.field.Byte1 < 0))
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}				

			if ((Power2.field.Byte0 > 31) || (Power2.field.Byte0 < 0))
				pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 > 31) || (Power2.field.Byte1 < 0))
				pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
		}
	}
	
#ifdef RT5592
	if (IS_RT5592(pAd))
	{
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
			DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: Power = %x\n", Power.word));
			DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: Power2 = %x\n", Power2.word));

			if ((Power.field.Byte0 < 0x2B) && (Power.field.Byte0 >= 0))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;



			if ((Power.field.Byte1 < 0x2B) && (Power.field.Byte1 >= 0))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			



			if ((Power2.field.Byte0 < 0x2B) && (Power2.field.Byte0 >= 0))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;



			if ((Power2.field.Byte1 < 0x2B) && (Power2.field.Byte1 >= 0))
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



			if ((Power.field.Byte0 < 0X2b) && (Power.field.Byte0 >= 0))

				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;



			if ((Power.field.Byte1 < 0X2B) && (Power.field.Byte1 >= 0))

				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			



			if ((Power2.field.Byte0 < 0X2B) && (Power2.field.Byte0 >= 0))

				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;



			if ((Power2.field.Byte1 < 0X2B) && (Power2.field.Byte1 >= 0))

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



			if ((Power.field.Byte0 < 0x2B) && (Power.field.Byte0 >= 0))

				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;



			if ((Power.field.Byte1 < 0x2b) && (Power.field.Byte1 >= 0))

				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			



			if ((Power2.field.Byte0 < 0x2b) && (Power2.field.Byte0 >= 0))

				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;



			if ((Power2.field.Byte1 < 0x2b) && (Power2.field.Byte1 >= 0))

				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			

		}
		/* 4. Print and Debug*/
		/*choffset = 14 + 12 + 16 + 7;*/
		choffset = 14 + 12 + 16 + 11;
		for (i = 0; i < choffset; i++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: TxPower[%03d], Channel = %d, Power = %d, Power2 = %d\n", i, pAd->TxPower[i].Channel, pAd->TxPower[i].Power, pAd->TxPower[i].Power2 ));
		}
	}

#endif

#ifdef RT3593
	//
	// 5GHz Tx power for 3593
	//
	if (IS_RT3593(pAd))
	{
		// 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)
		// 1.1 Fill up channel
		choffset = 14;
		for (i = 0; i < 4; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel  = 36 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel  = 36 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel  = 36 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
		}

		// 1.2 Fill up power
		for (i = 0; i < 6; i++)
		{
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX0_OVER_5G + i * 2), Power.word);
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX1_OVER_5G + i * 2), Power2.word);
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX2_OVER_5G + i * 2), Power3.word);

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte0;
			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power = Tx0ALC;
			}

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte1;
			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power = Tx0ALC;
			}

			Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte0;
			if (Tx1ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power2 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Tx1ALC;
			}

			Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte1;
			if (Tx1ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power2 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Tx1ALC;
			}

			Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte0;
			if (Tx2ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power3 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power3 = Tx2ALC;
			}

			Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte1;
			if (Tx2ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power3 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power3 = Tx2ALC;
			}
		}

		// 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz)
		// 2.1 Fill up channel
		choffset = 14 + 12;
		for (i = 0; i < 5; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel  = 100 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel  = 100 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel  = 100 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
		}
		
		pAd->TxPower[3 * 5 + choffset + 0].Channel         = 140;
		pAd->TxPower[3 * 5 + choffset + 0].Power            = DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 5 + choffset + 0].Power2          = DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 5 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;

		// 2.2 Fill up power
		for (i = 0; i < 8; i++)
		{
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX0_OVER_5G + (choffset - 14) + i * 2), Power.word);
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX1_OVER_5G + (choffset - 14) + i * 2), Power2.word);
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX2_OVER_5G + (choffset - 14) + i * 2), Power3.word);

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte0;
			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power = Tx0ALC;
			}

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte1;
			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power = Tx0ALC;
			}

			Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte0;
			if (Tx1ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power2 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Tx1ALC;
			}

			Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte1;
			if (Tx1ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power2 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Tx1ALC;
			}

			Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte0;
			if (Tx2ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power3 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power3 = Tx2ALC;
			}

			Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte1;
			if (Tx2ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power3 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power3 = Tx2ALC;
			}
		}

		// 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165; 167; 169; 171; 173 (including central frequency in BW 40MHz)
		// 3.1 Fill up channel
		choffset = 14 + 12 + 16;
		for (i = 0; i < 3; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel  = 149 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power3	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel  = 149 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power3	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel  = 149 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power     = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2   = DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power3	= DEFAULT_RF_TX_POWER;
		}
		
		pAd->TxPower[3 * 3 + choffset + 0].Channel         = 171;
		pAd->TxPower[3 * 3 + choffset + 0].Power            = DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 0].Power2          = DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 0].Power3		= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * 3 + choffset + 1].Channel		= 173;
		pAd->TxPower[3 * 3 + choffset + 1].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 1].Power2		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 1].Power3		= DEFAULT_RF_TX_POWER;

		// 3.2 Fill up power
		for (i = 0; i < 6; i++)
		{
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX0_OVER_5G + (choffset - 14) + i * 2), Power.word);
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX1_OVER_5G + (choffset - 14) + i * 2), Power2.word);
			RT28xx_EEPROM_READ16(pAd, (EEPROM_EXT_TX2_OVER_5G + (choffset - 14) + i * 2), Power3.word);

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte0;
			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power = Tx0ALC;
			}

			Tx0ALC = GET_TX_ALC_BIT_MASK & Power.field.Byte1;
			if (Tx0ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power = Tx0ALC;
			}

			Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte0;
			if (Tx1ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power2 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Tx1ALC;
			}

			Tx1ALC = GET_TX_ALC_BIT_MASK & Power2.field.Byte1;
			if (Tx1ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power2 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Tx1ALC;
			}

			Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte0;
			if (Tx2ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 0].Power3 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 0].Power3 = Tx2ALC;
			}

			Tx2ALC = GET_TX_ALC_BIT_MASK & Power3.field.Byte1;
			if (Tx2ALC > 31)
			{
				pAd->TxPower[i * 2 + choffset + 1].Power3 = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + choffset + 1].Power3 = Tx2ALC;
			}
		}
	}
	else
#endif /* RT3593 */
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

	if (((pAd->MACVersion & 0xffff0000) <= 0x30900000) ||
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

	if (((pAd->MACVersion & 0xffff0000) <= 0x30900000) ||
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

#ifdef IQ_CAL_SUPPORT
static UCHAR IQCalValue[IQ_CAL_CHANNEL_GROUP_NUM][IQ_CAL_CHAIN_NUM][IQ_CAL_TYPE_NUM];

VOID GetIQCalibration(IN PRTMP_ADAPTER pAd)
{
	UINT16 E2PValue;	
	
	/* 2G IQ Calibration for TX0 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX0_2G, E2PValue);
	IQCalValue[IQ_CAL_2G][IQ_CAL_TX0][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX0_2G, E2PValue);
	IQCalValue[IQ_CAL_2G][IQ_CAL_TX0][IQ_CAL_PHASE] = E2PValue & 0x00FF;
	
	/* 2G IQ Calibration for TX1 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX1_2G, E2PValue);
	IQCalValue[IQ_CAL_2G][IQ_CAL_TX1][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX1_2G, E2PValue);
	IQCalValue[IQ_CAL_2G][IQ_CAL_TX1][IQ_CAL_PHASE] = E2PValue & 0x00FF;

#ifdef A_BAND_SUPPORT
	/* 5G IQ Calibration Value for TX0 of Ch36~Ch64 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX0_CH36_TO_CH64_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP1_5G][IQ_CAL_TX0][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX0_CH36_TO_CH64_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP1_5G][IQ_CAL_TX0][IQ_CAL_PHASE] = E2PValue & 0x00FF;

	/* 5G IQ Calibration Value for TX1 of Ch36~Ch64 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX1_CH36_TO_CH64_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP1_5G][IQ_CAL_TX1][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX1_CH36_TO_CH64_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP1_5G][IQ_CAL_TX1][IQ_CAL_PHASE] = E2PValue & 0x00FF;

	/* 5G IQ Calibration Value for TX0 of Ch100~Ch138 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX0_CH100_TO_CH138_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP2_5G][IQ_CAL_TX0][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX0_CH100_TO_CH138_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP2_5G][IQ_CAL_TX0][IQ_CAL_PHASE] = E2PValue & 0x00FF;

	/* 5G IQ Calibration Value for TX1 of Ch100~Ch138 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX1_CH100_TO_CH138_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP2_5G][IQ_CAL_TX1][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX1_CH100_TO_CH138_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP2_5G][IQ_CAL_TX1][IQ_CAL_PHASE] = E2PValue & 0x00FF;

	/* 5G IQ Calibration Value for TX0 of Ch140~Ch165 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX0_CH140_TO_CH165_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP3_5G][IQ_CAL_TX0][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX0_CH140_TO_CH165_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP3_5G][IQ_CAL_TX0][IQ_CAL_PHASE] = E2PValue & 0x00FF;

	/* 5G IQ Calibration Value for TX1 of Ch140~Ch165 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GAIN_CAL_TX1_CH140_TO_CH165_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP3_5G][IQ_CAL_TX1][IQ_CAL_GAIN] = E2PValue & 0x00FF;
	RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_PHASE_CAL_TX1_CH140_TO_CH165_5G, E2PValue);
	IQCalValue[IQ_CAL_GROUP3_5G][IQ_CAL_TX1][IQ_CAL_PHASE] = E2PValue & 0x00FF;
	
#endif /* A_BAND_SUPPORT */
}

inline UCHAR IQCal(
	IN enum IQ_CAL_CHANNEL_INDEX 	ChannelIndex,
	IN enum IQ_CAL_TXRX_CHAIN 		TxRxChain,
	IN enum IQ_CAL_TYPE 				IQCalType)
{
	if (IQCalValue[ChannelIndex][TxRxChain][IQCalType] != 0xFF)
	return IQCalValue[ChannelIndex][TxRxChain][IQCalType];
	else
		return 0;
}

VOID IQCalibration(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel)
{
	UCHAR BBPValue;
	UINT16 	E2PValue;	

	/* IQ Calibration */
	if (Channel <= 14)
	{
		/* TX0 IQ Gain */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2C);
		BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX0, IQ_CAL_GAIN);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
		
		/* TX0 IQ Phase */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2D);
		BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX0, IQ_CAL_PHASE);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

			/* TX1 IQ Gain */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4A);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX1, IQ_CAL_GAIN);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			
			/* TX1 IQ Phase */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4B);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX1, IQ_CAL_PHASE);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		/* RF IQ Compensation Control */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x04);
		RT28xx_EEPROM_READ16(pAd, EEPROM_RF_IQ_COMPENSATION_CONTROL, E2PValue);
		if ((E2PValue & 0x00FF) == 0x00FF)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0); /* IQ Compensation disabled */
		else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, E2PValue & 0x00FF);

		/* RF IQ Imbalance Compensation Control */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x03);
		RT28xx_EEPROM_READ16(pAd, EEPROM_RF_IQ_IMBALANCE_COMPENSATION_CONTROL, E2PValue);
		if ((E2PValue & 0x00FF) == 0x00FF)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0); /* IQ Imbalance Compensation disabled */
		else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, E2PValue & 0x00FF);
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("%s() : 5 GHz band not supported !\n", __FUNCTION__));
}

VOID IQCalibrationViaBBPAccessSpace(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel)
{
	INT 		BBPIndex;
	UINT16 	E2PValue;


	/* IQ Calibration */
	if (Channel <= 14)
	{
		for (BBPIndex = 0; BBPIndex <= 7; BBPIndex++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GLOBAL_BBP_ACCESS_BASE + (BBPIndex * 2), E2PValue);
			
			if (E2PValue == 0xFFFF)
			{
				DBGPRINT(RT_DEBUG_INFO, ("%s() : EEPROM 0x%X is not calibrated !\n", __FUNCTION__, 
					(EEPROM_IQ_GLOBAL_BBP_ACCESS_BASE + (BBPIndex * 2))));
				continue;
			}
			else
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, (UCHAR)((E2PValue >> 8) & 0x00FF), (UCHAR)(E2PValue & 0x00FF));
			}
		}
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s() : 5 GHz band not supported !\n", __FUNCTION__));
}

#endif /* IQ_CAL_SUPPORT */

#ifdef CONFIG_TSO_SUPPORT
VOID RTMPTsoEnable(IN PRTMP_ADAPTER pAd)
{
	INTERNAL_1_STRUCT Internal_1;

	RTMP_IO_READ32(pAd, INTERNAL_1, &Internal_1.word);
	Internal_1.field.CSO_RX_IPV6_CHKSUM_EN = 1;
	Internal_1.field.CSO_TX_IPV6_CHKSUM_EN = 1;
	Internal_1.field.CSO_HW_PARSE_TCP = 1;
	Internal_1.field.CSO_HW_PARSE_IP = 1;
	Internal_1.field.CSO_RX_CHKSUM_EN = 1;
	Internal_1.field.CSO_TX_CHKSUM_EN = 1;
	RTMP_IO_WRITE32(pAd, INTERNAL_1, Internal_1.word);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Enable TSO Support\n"));
};

VOID RTMPTsoDisable(IN PRTMP_ADAPTER pAd)
{
	INTERNAL_1_STRUCT Internal_1;

	RTMP_IO_READ32(pAd, INTERNAL_1, &Internal_1.word);
	Internal_1.field.CSO_RX_IPV6_CHKSUM_EN = 0;
	Internal_1.field.CSO_TX_IPV6_CHKSUM_EN = 0;
	Internal_1.field.CSO_HW_PARSE_TCP = 0;
	Internal_1.field.CSO_HW_PARSE_IP = 0;
	Internal_1.field.CSO_RX_CHKSUM_EN = 0;
	Internal_1.field.CSO_TX_CHKSUM_EN = 0;
	RTMP_IO_WRITE32(pAd, INTERNAL_1, Internal_1.word);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Disable TSO Support\n"));
};
#endif /* CONFIG_TSO_SUPPORT */



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

