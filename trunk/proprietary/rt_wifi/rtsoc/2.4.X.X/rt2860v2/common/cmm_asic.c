/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC
	
	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


#if defined(RT28xx) || defined(RT2883)
// Reset the RFIC setting to new series    
RTMP_RF_REGS RF2850RegTable[] = {
//		ch	 R1 		 R2 		 R3(TX0~4=0) R4
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

		// 802.11 UNI / HyperLan 2
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
		{64, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed1a3}, // Plugfest#4, Day4, change RFR3 left4th 9->5.

		// 802.11 HyperLan 2
		{100, 0x98402ec8, 0x984c06b2, 0x98178a55, 0x980ed783},
		
		// 2008.04.30 modified 
		// The system team has AN to improve the EVM value 
		// for channel 102 to 108 for the RT2850/RT2750 dual band solution.
		{102, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed793},
		{104, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed1a3},
		{108, 0x98402ecc, 0x985c0a32, 0x98578a55, 0x980ed193},

		{110, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed183},
		{112, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed19b},
		{116, 0x98402ecc, 0x984c0a3a, 0x98178a55, 0x980ed1a3},
		{118, 0x98402ecc, 0x984c0a3e, 0x98178a55, 0x980ed193},
		{120, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed183},
		{124, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed193},
		{126, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed15b}, // 0x980ed1bb->0x980ed15b required by Rory 20070927
		{128, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed1a3},
		{132, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed18b},
		{134, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed193},
		{136, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed19b},
		{140, 0x98402ec4, 0x984c038a, 0x98178a55, 0x980ed183},

		// 802.11 UNII
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

		// Japan
		{184, 0x95002ccc, 0x9500491e, 0x9509be55, 0x950c0a0b},
		{188, 0x95002ccc, 0x95004922, 0x9509be55, 0x950c0a13},
		{192, 0x95002ccc, 0x95004926, 0x9509be55, 0x950c0a1b},
		{196, 0x95002ccc, 0x9500492a, 0x9509be55, 0x950c0a23},
		{208, 0x95002ccc, 0x9500493a, 0x9509be55, 0x950c0a13},
		{212, 0x95002ccc, 0x9500493e, 0x9509be55, 0x950c0a1b},
		{216, 0x95002ccc, 0x95004982, 0x9509be55, 0x950c0a23},

		// still lack of MMAC(Japan) ch 34,38,42,46
};
UCHAR	NUM_OF_2850_CHNL = (sizeof(RF2850RegTable) / sizeof(RTMP_RF_REGS));
#endif // defined(RT28xx) || defined(RT2883) //

#ifdef RT2880
// Reset the RFIC setting to new series    
RTMP_RF_REGS RF2850RegTable[] = {
//		ch	 R1 		 R2 		 R3(TX0~4=0) R4
		{1,  0x98402ecc, 0x984c0786, 0x981ab455, 0x9800510b},
		{2,  0x98402ecc, 0x984c0786, 0x981a8a55, 0x9800519f},
		{3,  0x98402ecc, 0x984c078a, 0x981a8a55, 0x9800518b},
		{4,  0x98402ecc, 0x984c078a, 0x981a8a55, 0x9800519f},
		{5,  0x98402ecc, 0x984c078e, 0x981a8a55, 0x9800518b},
		{6,  0x98402ecc, 0x984c078e, 0x981a8a55, 0x9800519f},
		{7,  0x98402ecc, 0x984c0792, 0x981a8a55, 0x9800518b},
		{8,  0x98402ecc, 0x984c0792, 0x981a8a55, 0x9800519f},
		{9,  0x98402ecc, 0x984c0796, 0x981a8a55, 0x9800518b},
		{10, 0x98402ecc, 0x984c0796, 0x981a8a55, 0x9800519f},
		{11, 0x98402ecc, 0x984c079a, 0x981a8a55, 0x9800518b},
		{12, 0x98402ecc, 0x984c079a, 0x981a8a55, 0x9800519f},
		{13, 0x98402ecc, 0x984c079e, 0x981a8a55, 0x9800518b},
		{14, 0x98402ecc, 0x984c07a2, 0x981a8a55, 0x98005193},

		// 802.11 UNI / HyperLan 2
		{36, 0x98402ecc, 0x984c099a, 0x98198a55, 0x980ed1a3},
		{38, 0x98402ecc, 0x984c099e, 0x98198a55, 0x980ed193},
		{40, 0x98402ec8, 0x984c0682, 0x98198a55, 0x980ed183},
		{44, 0x98402ec8, 0x984c0682, 0x98198a55, 0x980ed1a3},
		{46, 0x98402ec8, 0x984c0686, 0x98198a55, 0x980ed18b},
		{48, 0x98402ec8, 0x984c0686, 0x98198a55, 0x980ed19b},
		{52, 0x98402ec8, 0x984c068a, 0x98198a55, 0x980ed193},
		{54, 0x98402ec8, 0x984c068a, 0x98198a55, 0x980ed1a3},
		{56, 0x98402ec8, 0x984c068e, 0x98198a55, 0x980ed18b},
		{60, 0x98402ec8, 0x984c0692, 0x98198a55, 0x980ed183},
		{62, 0x98402ec8, 0x984c0692, 0x98198a55, 0x980ed193},
		{64, 0x98402ec8, 0x984c0692, 0x98198a55, 0x980ed1a3}, // Plugfest#4, Day4, change RFR3 left4th 9->5.

		// 802.11 HyperLan 2
		{100, 0x98402ec8, 0x984c06b2, 0x981b8a55, 0x980ed783},
		
		// 2008.04.30 modified 
		// The system team has AN to improve the EVM value 
		// for channel 102 to 108 for the RT2850/RT2750 dual band solution.
		{102, 0x98402ec8, 0x985c06b2, 0x985b8a55, 0x980ed793},
		{104, 0x98402ec8, 0x985c06b2, 0x985b8a55, 0x980ed1a3},
		{108, 0x98402ecc, 0x985c0a32, 0x985b8a55, 0x980ed193},

		{110, 0x98402ecc, 0x984c0a36, 0x981b8a55, 0x980ed183},
		{112, 0x98402ecc, 0x984c0a36, 0x981b8a55, 0x980ed19b},
		{116, 0x98402ecc, 0x984c0a3a, 0x981b8a55, 0x980ed1a3},
		{118, 0x98402ecc, 0x984c0a3e, 0x981b8a55, 0x980ed193},
		{120, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed183},
		{124, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed193},
		{126, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed15b}, // 0x980ed1bb->0x980ed15b required by Rory 20070927
		{128, 0x98402ec4, 0x984c0382, 0x981b8a55, 0x980ed1a3},
		{132, 0x98402ec4, 0x984c0386, 0x981b8a55, 0x980ed18b},
		{134, 0x98402ec4, 0x984c0386, 0x981b8a55, 0x980ed193},
		{136, 0x98402ec4, 0x984c0386, 0x981b8a55, 0x980ed19b},
		{140, 0x98402ec4, 0x984c038a, 0x981b8a55, 0x980ed183},

		// 802.11 UNII
		{149, 0x98402ec4, 0x984c038a, 0x981b8a55, 0x980ed1a7},
		{151, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed187},
		{153, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed18f},
		{157, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed19f},
		{159, 0x98402ec4, 0x984c038e, 0x981b8a55, 0x980ed1a7},
		{161, 0x98402ec4, 0x984c0392, 0x981b8a55, 0x980ed187},
		{165, 0x98402ec4, 0x984c0392, 0x981b8a55, 0x980ed197},
		{167, 0x98402ec4, 0x984c03d2, 0x981b9855, 0x9815531f},
		{169, 0x98402ec4, 0x984c03d2, 0x981b9855, 0x98155327},
		{171, 0x98402ec4, 0x984c03d6, 0x981b9855, 0x98155307},
		{173, 0x98402ec4, 0x984c03d6, 0x981b9855, 0x9815530f},

		// Japan
		{184, 0x95002ccc, 0x9500491e, 0x9509be55, 0x950c0a0b},
		{188, 0x95002ccc, 0x95004922, 0x9509be55, 0x950c0a13},
		{192, 0x95002ccc, 0x95004926, 0x9509be55, 0x950c0a1b},
		{196, 0x95002ccc, 0x9500492a, 0x9509be55, 0x950c0a23},
		{208, 0x95002ccc, 0x9500493a, 0x9509be55, 0x950c0a13},
		{212, 0x95002ccc, 0x9500493e, 0x9509be55, 0x950c0a1b},
		{216, 0x95002ccc, 0x95004982, 0x9509be55, 0x950c0a23},

		// still lack of MMAC(Japan) ch 34,38,42,46
};
UCHAR	NUM_OF_2850_CHNL = (sizeof(RF2850RegTable) / sizeof(RTMP_RF_REGS));
#endif // RT2880 //


FREQUENCY_ITEM FreqItems3020[] =
{	
	/**************************************************/
	// ISM : 2.4 to 2.483 GHz                         //
	/**************************************************/
	// 11g
	/**************************************************/
	//-CH---N-------R---K-----------
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
UCHAR	NUM_OF_3020_CHNL = (sizeof(FreqItems3020) / sizeof(FREQUENCY_ITEM));

#ifdef RT3883
FREQUENCY_ITEM FreqItems3883[] =
{
	/**************************************************/
	// ISM : 2.4 to 2.483 GHz                         //
	/**************************************************/
	//-CH---N-------R---K-----------
	//  R : <pll_mode[3:2], pll_R[1:0]>, g band: pll_mode=01, pll_R=10
	{1,    241,  6,  2},
	{2,    241,	 6,  7},
	{3,    242,	 6,  2},
	{4,    242,	 6,  7},
	{5,    243,	 6,  2},
	{6,    243,	 6,  7},
	{7,    244,	 6,  2},
	{8,    244,	 6,  7},
	{9,    245,	 6,  2},
	{10,   245,	 6,  7},
	{11,   246,	 6,  2},
	{12,   246,	 6,  7},
	{13,   247,	 6,  2},
	{14,   248,	 6,  4},

	/**************************************************/
	// 5 GHz
	/**************************************************/
	//  R : <pll_mode[3:2], pll_R[1:0]>, g band: pll_mode=10, pll_R=00
	{36,	0x56,	8,	4},
	{38,	0x56,	8,	6},
	{40,	0x56,	8,	8},
	{44,	0x57,	8,	0},
	{46,	0x57,	8,	2},
	{48,	0x57,	8,	4},
	{52,	0x57,	8,	8},
	{54,	0x57,	8,	10},
	{56,	0x58,	8,	0},
	{60,	0x58,	8,	4},
	{62,	0x58,	8,	6},
	{64,	0x58,	8,	8},

	{100,	0x5B,	8,	8},
	{102,	0x5B,	8,	10},
	{104,	0x5C,	8,	0},
	{108,	0x5C,	8,	4},
	{110,	0x5C,	8,	6},
	{112,	0x5C,	8,	8},
	{114,	0x5C,	8,	10},
	{116,	0x5D,	8,	0},
	{118,	0x5D,	8,	2},
	{120,	0x5D,	8,	4},
	{124,	0x5D,	8,	8},
	{126,	0x5D,	8,	10},
	{128,	0x5E,	8,	0},
	{132,	0x5E,	8,	4},
	{134,	0x5E,	8,	6},
	{136,	0x5E,	8,	8},
	{140,	0x5F,	8,	0},

	{149,	0x5F,	8,	9},
	{151,	0x5F,	8,	11},
	{153,	0x60,	8,	1},
	{157,	0x60,	8,	5},
	{159,	0x60,	8,	7},
	{161,	0x60,	8,	9},
	{165,	0x61,	8,	1},
//	{167,	0x61,	8,	3},
//	{169,	0x61,	8,	5},
//	{171,	0x61,	8,	7},
//	{173,	0x61,	8,	9},
	{184,	0x52,	8,	0},
	{188,	0x52,	8,	4},
	{192,	0x52,	8,	8},
	{196,	0x53,	8,	0},
};

UCHAR NUM_OF_3883_CHNL = (sizeof(FreqItems3883) / sizeof(FREQUENCY_ITEM));
#endif // RT3883 //



#ifdef RTMP_INTERNAL_TX_ALC
// The Tx power tuning entry
TX_POWER_TUNING_ENTRY_STRUCT TxPowerTuningTable[] = 
{
//	idxTxPowerTable		Tx power control over RF		Tx power control over MAC
//	(zero-based array)		{ RF R12[4:0]: Tx0 ALC},		{MAC 0x1314~0x1324}
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

// The desired TSSI over CCK
CHAR desiredTSSIOverCCK[4] = {0};

// The desired TSSI over OFDM
CHAR desiredTSSIOverOFDM[8] = {0};

// The desired TSSI over HT
CHAR desiredTSSIOverHT[8] = {0};

// The desired TSSI over HT using STBC
CHAR desiredTSSIOverHTUsingSTBC[8] = {0};
#endif // RTMP_INTERNAL_TX_ALC //

#ifdef CONFIG_STA_SUPPORT
VOID AsicUpdateAutoFallBackTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pRateTable)
{
	UCHAR					i;
	HT_FBK_CFG0_STRUC		HtCfg0;
	HT_FBK_CFG1_STRUC		HtCfg1;
	LG_FBK_CFG0_STRUC		LgCfg0;
	LG_FBK_CFG1_STRUC		LgCfg1;
#ifdef DOT11N_SS3_SUPPORT
	TX_FBK_CFG_3S_0_STRUC	HtCfg2;
	TX_FBK_CFG_3S_1_STRUC	HtCfg3;
#endif // DOT11N_SS3_SUPPORT //
	PRTMP_TX_RATE_SWITCH	pCurrTxRate, pNextTxRate;

	// set to initial value
	HtCfg0.word = 0x65432100;
	HtCfg1.word = 0xedcba980;
	LgCfg0.word = 0xedcba988;
	LgCfg1.word = 0x00002100;
#ifdef DOT11N_SS3_SUPPORT
	//HtCfg2.word = 0x1211100f;
	//HtCfg3.word = 0x16150f0f;
	HtCfg2.word = 0x12111008;
	HtCfg3.word = 0x16151413;
#endif // DOT11N_SS3_SUPPORT //

#ifdef NEW_RATE_ADAPT_SUPPORT
	// Use standard fallback if using new rate table
	if (ADAPT_RATE_TABLE(pRateTable))
		goto skipUpdate;
#endif // NEW_RATE_ADAPT_SUPPORT //

	pNextTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1;
	for (i = 1; i < *((PUCHAR) pRateTable); i++)
	{
		pCurrTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1+i;
		switch (pCurrTxRate->Mode)
		{
			case 0:		//CCK
				break;
			case 1:		//OFDM
				{
					switch(pCurrTxRate->CurrMCS)
					{
						case 0:
							LgCfg0.field.OFDMMCS0FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 1:
							LgCfg0.field.OFDMMCS1FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 2:
							LgCfg0.field.OFDMMCS2FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 3:
							LgCfg0.field.OFDMMCS3FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 4:
							LgCfg0.field.OFDMMCS4FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 5:
							LgCfg0.field.OFDMMCS5FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 6:
							LgCfg0.field.OFDMMCS6FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 7:
							LgCfg0.field.OFDMMCS7FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
					}
				}
				break;
#ifdef DOT11_N_SUPPORT
			case 2:		//HT-MIX
			case 3:		//HT-GF
				{
					if ((pNextTxRate->Mode >= MODE_HTMIX) && (pCurrTxRate->CurrMCS != pNextTxRate->CurrMCS))
					{
						switch(pCurrTxRate->CurrMCS)
						{
							case 0:
								HtCfg0.field.HTMCS0FBK = pNextTxRate->CurrMCS;
								break;
							case 1:
								HtCfg0.field.HTMCS1FBK = pNextTxRate->CurrMCS;
								break;
							case 2:
								HtCfg0.field.HTMCS2FBK = pNextTxRate->CurrMCS;
								break;
							case 3:
								HtCfg0.field.HTMCS3FBK = pNextTxRate->CurrMCS;
								break;
							case 4:
								HtCfg0.field.HTMCS4FBK = pNextTxRate->CurrMCS;
								break;
							case 5:
								HtCfg0.field.HTMCS5FBK = pNextTxRate->CurrMCS;
								break;
							case 6:
								HtCfg0.field.HTMCS6FBK = pNextTxRate->CurrMCS;
								break;
							case 7:
								HtCfg0.field.HTMCS7FBK = pNextTxRate->CurrMCS;
								break;
							case 8:
								HtCfg1.field.HTMCS8FBK = pNextTxRate->CurrMCS;
								break;
							case 9:
								HtCfg1.field.HTMCS9FBK = pNextTxRate->CurrMCS;
								break;
							case 10:
								HtCfg1.field.HTMCS10FBK = pNextTxRate->CurrMCS;
								break;
							case 11:
								HtCfg1.field.HTMCS11FBK = pNextTxRate->CurrMCS;
								break;
							case 12:
								HtCfg1.field.HTMCS12FBK = pNextTxRate->CurrMCS;
								break;
							case 13:
								HtCfg1.field.HTMCS13FBK = pNextTxRate->CurrMCS;
								break;
							case 14:
								HtCfg1.field.HTMCS14FBK = pNextTxRate->CurrMCS;
								break;
							case 15:
								HtCfg1.field.HTMCS15FBK = pNextTxRate->CurrMCS;
								break;
							default:
								DBGPRINT(RT_DEBUG_ERROR, ("AsicUpdateAutoFallBackTable: not support CurrMCS=%d\n", pCurrTxRate->CurrMCS));
						}
					}
				}
				break;
#endif // DOT11_N_SUPPORT //
		}

		pNextTxRate = pCurrTxRate;
	}

#ifdef NEW_RATE_ADAPT_SUPPORT
skipUpdate:
#endif // NEW_RATE_ADAPT_SUPPORT //

	RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, HtCfg0.word);
	RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, HtCfg1.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG0, LgCfg0.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG1, LgCfg1.word);
#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, HtCfg2.word);
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, HtCfg3.word);
	}
#endif // DOT11N_SS3_SUPPORT //
}
#endif // CONFIG_STA_SUPPORT //

/*
	========================================================================

	Routine Description:
		Set MAC register value according operation mode.
		OperationMode AND bNonGFExist are for MM and GF Proteciton.
		If MM or GF mask is not set, those passing argument doesn't not take effect.
		
		Operation mode meaning:
		= 0 : Pure HT, no preotection.
		= 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
		= 0x10: No Transmission in 40M is protected.
		= 0x11: Transmission in both 40M and 20M shall be protected
		if (bNonGFExist)
			we should choose not to use GF. But still set correct ASIC registers.
	========================================================================
*/
VOID 	AsicUpdateProtect(
	IN		PRTMP_ADAPTER	pAd,
	IN 		USHORT			OperationMode,
	IN 		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist)	
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6];
	USHORT			offset;
	UCHAR			i;
	UINT32 MacReg = 0;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif // RALINK_ATE //

#ifdef DOT11_N_SUPPORT
	if (!(pAd->CommonCfg.bHTProtect) && (OperationMode != 8))
	{
		return;
	}

#ifdef RT3883
	if (pAd->FlgCWC == 1) //enable cwc -> disable RTS/CTS
		;
	else if ((pAd->FlgCWC == 2) && (pAd->MacTab.Size == 1) && 
		((pAd->MacTab.fAnyStationIsLegacy == FALSE) && (pAd->MacTab.fAnyStationBadAtheros == FALSE))
		)
	{
		int macIdx;
		BOOLEAN bNoRTS = FALSE;
		/* 
			Depends on Gary's request, 
				1. only one 11n STA(non-A STA) is connected
				2. MCS is 22 ~ 23
			just let it go!
		*/		
		for (macIdx = 0; macIdx < MAX_LEN_OF_MAC_TABLE; macIdx++)
		{
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[macIdx];
			
			if (!IS_ENTRY_CLIENT(pEntry))
				continue;

			/* Well, I think the first one will be our target, because we only have one valid station connect to us */
			if (pEntry->HTPhyMode.field.MCS >= 22 && pEntry->HTPhyMode.field.MCS<=23)
				bNoRTS = TRUE;
			
			//DBGPRINT(RT_DEBUG_WARN, ("MCS = %d\n", pEntry->HTPhyMode.field.MCS));
			break;
		}

		if ((bNoRTS == FALSE) && (pAd->BATable.numDoneOriginator))
		{
			/* We may need to go original case here if the NoRTS is FALSE */
			SetMask |= ALLN_SETPROTECT;
			OperationMode = 8;	
			RTMP_IO_READ32(pAd, TX_RTY_CFG, &MacReg);
			MacReg &= 0xffff0000;
			MacReg |= 0x1f0f;
			RTMP_IO_WRITE32(pAd, TX_RTY_CFG, MacReg);

			RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &MacReg);
			MacReg &= 0xffff0000;
			MacReg |= 0x1010;
			RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, MacReg);
			//DBGPRINT(RT_DEBUG_WARN, ("RTS/CTS protection ON\n"));
		}		
		else
		{
			RTMP_IO_READ32(pAd, TX_RTY_CFG, &MacReg);
			MacReg &= 0xffff0000;
			MacReg |= 0x3f3f;
			RTMP_IO_WRITE32(pAd, TX_RTY_CFG, MacReg);

			RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &MacReg);
			MacReg &= 0xffff0000;
			MacReg |= 0x0e0e;
			RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, MacReg);
			//DBGPRINT(RT_DEBUG_WARN, ("RTS/CTS protection OFF\n"));
		}
	}
	else
#endif // RT3883 //
	if (pAd->BATable.numDoneOriginator)
	{
		// 
		// enable the RTS/CTS to avoid channel collision
		// 
		SetMask |= ALLN_SETPROTECT;
		OperationMode = 8;
	}
#endif // DOT11_N_SUPPORT //

	// Config ASIC RTS threshold register
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;
	// If the user want disable RtsThreshold and enbale Amsdu/Ralink-Aggregation, set the RtsThreshold as 4096
        if ((
#ifdef DOT11_N_SUPPORT
			(pAd->CommonCfg.BACapability.field.AmsduEnable) || 
#endif // DOT11_N_SUPPORT //
			(pAd->CommonCfg.bAggregationCapable == TRUE))
            && pAd->CommonCfg.RtsThreshold == MAX_RTS_THRESHOLD)
        {
			MacReg |= (0x1000 << 8);
        }
        else
        {
			MacReg |= (pAd->CommonCfg.RtsThreshold << 8);
        }

	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	// Initial common protection settings
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	// update PHY mode and rate
	if (pAd->CommonCfg.Channel > 14)
		ProtCfg.field.ProtectRate = 0x4000;
	ProtCfg.field.ProtectRate |= pAd->CommonCfg.RtsRate;	

	// Handle legacy(B/G) protection
	if (bDisableBGProtect)
	{
		//ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;
		ProtCfg.field.ProtectCtrl = 0;
		Protect[0] = ProtCfg.word;
		Protect[1] = ProtCfg.word;
		pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
	}
	else
	{
		//ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;
		ProtCfg.field.ProtectCtrl = 0;			// CCK do not need to be protected
		Protect[0] = ProtCfg.word;
		ProtCfg.field.ProtectCtrl = ASIC_CTS;	// OFDM needs using CCK to protect
		Protect[1] = ProtCfg.word;
		pAd->FlgCtsEnabled = 1; /* CTS-self is used */
	}

#ifdef DOT11_N_SUPPORT
	// Decide HT frame protection.
	if ((SetMask & ALLN_SETPROTECT) != 0)
	{
		switch(OperationMode)
		{
			case 0x0:
				// NO PROTECT 
				// 1.All STAs in the BSS are 20/40 MHz HT
				// 2. in ai 20/40MHz BSS
				// 3. all STAs are 20MHz in a 20MHz BSS
				// Pure HT. no protection.

				// MM20_PROT_CFG
				//	Reserved (31:27)
				// 	PROT_TXOP(25:20) -- 010111
				//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
				//  PROT_CTRL(17:16) -- 00 (None)
				// 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
				Protect[2] = 0x01744004;	

				// MM40_PROT_CFG
				//	Reserved (31:27)
				// 	PROT_TXOP(25:20) -- 111111
				//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
				//  PROT_CTRL(17:16) -- 00 (None) 
				// 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
				Protect[3] = 0x03f44084;

				// CF20_PROT_CFG
				//	Reserved (31:27)
				// 	PROT_TXOP(25:20) -- 010111
				//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
				//  PROT_CTRL(17:16) -- 00 (None)
				// 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
				Protect[4] = 0x01744004;

				// CF40_PROT_CFG
				//	Reserved (31:27)
				// 	PROT_TXOP(25:20) -- 111111
				//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
				//  PROT_CTRL(17:16) -- 00 (None)
				// 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
				Protect[5] = 0x03f44084;

				if (bNonGFExist)
				{
					// PROT_NAV(19:18)  -- 01 (Short NAV protectiion)
					// PROT_CTRL(17:16) -- 01 (RTS/CTS)
					Protect[4] = 0x01754004;
					Protect[5] = 0x03f54084;
				}
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
				break;
				
 			case 1:
				// This is "HT non-member protection mode."
				// If there may be non-HT STAs my BSS
				ProtCfg.word = 0x01744004;	// PROT_CTRL(17:16) : 0 (None)
				ProtCfg4.word = 0x03f44084; // duplicaet legacy 24M. BW set 1.
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	//ERP use Protection bit is set, use protection rate at Clause 18..
					ProtCfg4.word = 0x03f40003; // Don't duplicate RTS/CTS in CCK mode. 0x03f40083; 
				}
				//Assign Protection method for 20&40 MHz packets
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;
				
			case 2:
				// If only HT STAs are in BSS. at least one is 20MHz. Only protect 40MHz packets
				ProtCfg.word = 0x01744004;  // PROT_CTRL(17:16) : 0 (None)
				ProtCfg4.word = 0x03f44084; // duplicaet legacy 24M. BW set 1.
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	//ERP use Protection bit is set, use protection rate at Clause 18..
					ProtCfg4.word = 0x03f40003; // Don't duplicate RTS/CTS in CCK mode. 0x03f40083; 
				} 
				//Assign Protection method for 40MHz packets
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				if (bNonGFExist)
				{
					ProtCfg.field.ProtectCtrl = ASIC_RTS;
					ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				}
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;

				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
				break;
				
			case 3:
				// HT mixed mode.	 PROTECT ALL!
				// Assign Rate
				ProtCfg.word = 0x01744004;	//duplicaet legacy 24M. BW set 1.
				ProtCfg4.word = 0x03f44084;
				// both 20MHz and 40MHz are protected. Whether use RTS or CTS-to-self depends on the
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	//ERP use Protection bit is set, use protection rate at Clause 18..
					ProtCfg4.word = 0x03f40003; // Don't duplicate RTS/CTS in CCK mode. 0x03f40083
				}
				//Assign Protection method for 20&40 MHz packets
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;	
				
			case 8:
				// Special on for Atheros problem n chip.
				ProtCfg.word = 0x01754004;	//duplicaet legacy 24M. BW set 1.
				ProtCfg4.word = 0x03f54084;
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01750003;	//ERP use Protection bit is set, use protection rate at Clause 18..
					ProtCfg4.word = 0x03f50003; // Don't duplicate RTS/CTS in CCK mode. 0x03f40083
				}
				
				Protect[2] = ProtCfg.word; 	//0x01754004;
				Protect[3] = ProtCfg4.word; //0x03f54084;
				Protect[4] = ProtCfg.word; 	//0x01754004;
				Protect[5] = ProtCfg4.word; //0x03f54084;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;		
		}
	}
#endif // DOT11_N_SUPPORT //
	
	offset = CCK_PROT_CFG;
	for (i = 0;i < 6; i++)
	{
		if ((SetMask & (1<< i)))
		{
			RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);
		}
	}
}


VOID AsicBBPAdjust(RTMP_ADAPTER *pAd)
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
		//  TX : control channel at lower 
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		//  RX : control channel at lower 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		if (pAd->CommonCfg.Channel > 14)
		{ 	// request by Gary 20070208 for middle and long range A Band
#if defined(RT2883) || defined(RT3883)
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
				byteValue &= 0x9f;
				/* Chain 0 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x48);
				/* Chain 1 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x48);
				/* Chain 2 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));
			}
#endif // defined(RT2883) || defined(RT3883) //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x48);
		}
		else
		{	// request by Gary 20070208 for middle and long range G Band
#if defined(RT2883) || defined(RT3883)
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
				byteValue &= 0x9f;
				/* Chain 0 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
				/* Chain 1 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
				/* Chain 2 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
			}
			else
#endif // defined(RT2883) || defined(RT3883) //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
		}	
		// 
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
		//  TX : control channel at upper 
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value |= (0x1);		
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		//  RX : control channel at upper 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue |= (0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		
		if (pAd->CommonCfg.Channel > 14)
		{ 	// request by Gary 20070208 for middle and long range A Band
#if defined(RT2883) || defined(RT3883)
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
				byteValue &= 0x9f;
				/* Chain 0 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x48);
				/* Chain 1 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x48);
				/* Chain 2 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));
			}
#endif // defined(RT2883) || defined(RT3883) //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x48);
		}
		else
		{ 	// request by Gary 20070208 for middle and long range G band
#if defined(RT2883) || defined(RT3883)
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
				byteValue &= 0x9f;
				/* Chain 0 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
				/* Chain 1 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
				/* Chain 2 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));
			}
#endif // defined(RT2883) || defined(RT3883) //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
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
#endif // DOT11_N_SUPPORT //
	{
		pAd->CommonCfg.BBPCurrentBW = BW_20;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		
		//  TX : control channel at lower 
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		
		// 20 MHz bandwidth
		if (pAd->CommonCfg.Channel > 14)
		{	 // request by Gary 20070208
#if defined(RT2883) || defined(RT3883)
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
				byteValue &= 0x9f;
				/* Chain 0 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x40);
				/* Chain 1 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x40);
				/* Chain 2 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));
			}
#endif // defined(RT2883) || defined(RT3883) //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x40);
		}	
		else
		{	// request by Gary 20070208
			//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x30);
			// request by Brian 20070306
#if defined(RT2883) || defined(RT3883)
			if (IS_RT2883(pAd) || IS_RT3883(pAd))
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
				byteValue &= 0x9f;
				/* Chain 0 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, byteValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
				/* Chain 1 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 5)));
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
				/* Chain 2 */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue | (1 << 6)));
			}
#endif // defined(RT2883) || defined(RT3883) //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x38);
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
#endif // DOT11_N_SUPPORT //
	}
	
	if (pAd->CommonCfg.Channel > 14)
	{	// request by Gary 20070208 for middle and long range A Band
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x1D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x1D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x1D);
		//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x1D);
	}
	else
	{ 	// request by Gary 20070208 for middle and long range G band
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, 0x2D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, 0x2D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, 0x2D);
		//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x2D);
	}	

}

#ifdef RT3883
// AsicSetFreqOffset - set Frequency Offset register
void AsicSetFreqOffset(
	IN  PRTMP_ADAPTER   pAd,
	IN	ULONG			freqOffset)
{
	UCHAR RFValue = 0, RFValue2;

	freqOffset &= 0x7f;

	// Set RF offset  RF_R17=RF_R23 (RT30xx)
	RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RFValue);
	if ((UCHAR)freqOffset == (RFValue & 0x7f))
		return;

	RFValue2 = (RFValue & 0x80) | (UCHAR)freqOffset;

	RT30xxWriteRFRegister(pAd, RF_R17, RFValue2);

	RtmpOsMsDelay(1);
}

#endif // RT3883 //
/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSwitchChannel(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			Channel,
	IN	BOOLEAN			bScan) 
{
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; //Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER;
	UCHAR	index;
	UINT32 	Value = 0; //BbpReg, Value;
	UCHAR 	RFValue;
#ifdef DOT11N_SS3_SUPPORT
	CHAR    TxPwer3 = 0;
#endif // DOT11N_SS3_SUPPORT //
#ifdef RTMP_MAC_PCI
#endif // RTMP_MAC_PCI //


#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif // AP_QLOAD_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

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
#ifdef DOT11N_SS3_SUPPORT
			if (IS_RT2883(pAd) || IS_RT3593(pAd) || IS_RT3883(pAd))
		    	TxPwer3 = pAd->TxPower[index].Power3;
#endif // DOT11N_SS3_SUPPORT //


			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: Can't find the Channel#%d \n", Channel));
	}
#ifdef RT3883
	if (IS_RT3883(pAd))
	{
		RTMPRT3883ABandSel(Channel);
		for (index = 0; index < NUM_OF_3883_CHNL; index++)
		{
			if (Channel == FreqItems3883[index].Channel)
			{
				int i;

				// RF_R06 for A-Band L:0x80 M:0x80 H:0x40 (Gary, 2010-06-02)
				if (pAd->CommonCfg.Channel <= 14)
					RFValue = 0x40;
				else if (pAd->CommonCfg.Channel <132)
					RFValue = 0x80;
				else
					RFValue = 0x40;
				RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RFValue);

				// Programming channel parameters
				RT30xxWriteRFRegister(pAd, RF_R08, FreqItems3883[index].N);
				RT30xxWriteRFRegister(pAd, RF_R09, FreqItems3883[index].K);
				
				if  (Channel <= 14)
					RFValue = 0x46;
				else
					RFValue = 0x48;
				RT30xxWriteRFRegister(pAd, RF_R11, (UCHAR)RFValue);

				if  (Channel <= 14)
					RFValue = 0x1A;
				else
					RFValue = 0x52;
				RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)RFValue);

				RFValue = 0x12;
				RT30xxWriteRFRegister(pAd, RF_R13, (UCHAR)RFValue);

				// Tx/Rx Stream setting
				RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&RFValue);
				RFValue &= 0x03; //clear bit[7~2]
				RFValue |= 0xFC; // default 3Tx 3Rx

				if (pAd->Antenna.field.TxPath == 1)
					RFValue &= ~(0x5 << 5);
				else if (pAd->Antenna.field.TxPath == 2)
					RFValue &= ~(0x1 << 7);
				else if (pAd->Antenna.field.TxPath == 3) 	//wayne_note: 090826 need to consider TxPath=3, for 3883 case
					RFValue &= ~(0x0 << 7);		

				if (pAd->Antenna.field.RxPath == 1)
					RFValue &= ~(0x5 << 4);
				else if (pAd->Antenna.field.RxPath == 2)
					RFValue &= ~(0x1 << 6);
				else if (pAd->Antenna.field.RxPath == 3)	//wayne_note: 090826 need to consider TxPath=3, for 3883 case
					RFValue &= ~(0x0 << 6);	

								
				RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)RFValue);

				AsicSetFreqOffset(pAd, pAd->RfFreqOffset);

				// Different default setting for A/BG bands
				RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RFValue);
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					RFValue &= ~(0x06); // 20MBW Bit[2:1]=0,0
				else
					RFValue |= 0x06;
				RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RFValue);

				if  (Channel <= 14)
					RFValue = 0xA0;
				else
					RFValue = 0x80;
				RT30xxWriteRFRegister(pAd, RF_R31, (UCHAR)RFValue);

				if  (Channel <= 14)
					RFValue = 0x3C;
				else
					RFValue = 0x20;
				RT30xxWriteRFRegister(pAd, RF_R34, (UCHAR)RFValue);

				// loopback RF_BS
				RT30xxReadRFRegister(pAd, RF_R36, (PUCHAR)&RFValue);
				RFValue &= ~(0x1 << 7);
				if  (Channel <= 14)
					RT30xxWriteRFRegister(pAd, RF_R36, (UCHAR)(RFValue  | (0x1 << 7)));
				else
					RT30xxWriteRFRegister(pAd, RF_R36, (UCHAR)RFValue);

				// RF_R39 for A-Band L:0x36 M:0x32 H:0x30 (Gary, 2010-02-12)
				if (pAd->CommonCfg.Channel <= 14)
					RFValue = 0x23;
				else
				{
					if (pAd->CommonCfg.Channel < 100)
						RFValue = 0x36;
					else if (pAd->CommonCfg.Channel < 132)
						RFValue = 0x32;
					else
						RFValue = 0x30;
				}
#ifdef TXBF_SUPPORT
				if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn || pAd->CommonCfg.ETxBfEnCond)
					RFValue |= 0x40;
#endif // TXBF_SUPPORT //
				RT30xxWriteRFRegister(pAd, RF_R39, (UCHAR)RFValue);
				
				// loopback RF_BS
				if  (Channel <= 14)
					RFValue = 0x93;
				else
					RFValue = 0x9B;  // Gary, 2010-02-12
				RT30xxWriteRFRegister(pAd, RF_R44, (UCHAR)RFValue);

				// RF_R45 for A-Band L:0xEB M:0xB3 H:0x9B (Gary, 2010-02-12)
				if (pAd->CommonCfg.Channel <= 14)
					RFValue = 0xBB;
				else
				{
					if (pAd->CommonCfg.Channel <100)
						RFValue = 0xEB;
					else if(pAd->CommonCfg.Channel <132)
						RFValue = 0xB3;
					else
						RFValue = 0x9B;
				}
				RT30xxWriteRFRegister(pAd, RF_R45, (UCHAR)RFValue);
				if  (Channel <= 14)
					RFValue = 0x8E;
				else
					RFValue = 0x8A;
#ifdef TXBF_SUPPORT
				if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn || pAd->CommonCfg.ETxBfEnCond)
					RFValue |= 0x20;
#endif // TXBF_SUPPORT //
				RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)RFValue);
	
				RFValue = 0x86;
				RT30xxWriteRFRegister(pAd, RF_R50, (UCHAR)RFValue);
		
				// tx_mx1_ic
				RT30xxReadRFRegister(pAd, RF_R51, (PUCHAR)&RFValue);
				if  (Channel <= 14)
					RFValue = 0x75;
				else
					RFValue = 0x51;
				RT30xxWriteRFRegister(pAd, RF_R51, (UCHAR)RFValue);

				RT30xxReadRFRegister(pAd, RF_R52, (PUCHAR)&RFValue);
				if  (Channel <= 14)
					RFValue = 0x45;
				else
					RFValue = 0x05;
				RT30xxWriteRFRegister(pAd, RF_R52, (UCHAR)RFValue);
			
				for (i = 0; i < MAX_NUM_OF_CHANNELS; i++)
				{
					UCHAR BbpValue = 0;
					CHAR power;

					if (Channel != pAd->TxPower[i].Channel)
						continue;

					if (Channel <= 14)
					{
						power = pAd->TxPower[i].Power+4;
						RT30xxWriteRFRegister(pAd, RF_R53, power>MAX_RF_TX_POWER? MAX_RF_TX_POWER: power);
						power = pAd->TxPower[i].Power2+4;
						RT30xxWriteRFRegister(pAd, RF_R54, power>MAX_RF_TX_POWER? MAX_RF_TX_POWER: power);
						power = pAd->TxPower[i].Power3+4;
						RT30xxWriteRFRegister(pAd, RF_R55, power>MAX_RF_TX_POWER? MAX_RF_TX_POWER: power);
					}
					else
					{
						//(Gary, 2010-02-12)
						power = pAd->TxPower[i].Power+4;
						if (power > MAX_RF_TX_POWER)
							power = MAX_RF_TX_POWER;
						RT30xxWriteRFRegister(pAd, RF_R53, TX_PWR_TO_RF_REG(power));
						power = pAd->TxPower[i].Power2+4;
						if (power > MAX_RF_TX_POWER)
							power = MAX_RF_TX_POWER;
						RT30xxWriteRFRegister(pAd, RF_R54, TX_PWR_TO_RF_REG(power));
						power = pAd->TxPower[i].Power3+4;
						if (power > MAX_RF_TX_POWER)
							power = MAX_RF_TX_POWER;
						RT30xxWriteRFRegister(pAd, RF_R55, TX_PWR_TO_RF_REG(power));
					}


					// tx0, tx1 (0.1db)
					BbpValue = (pAd->TxPower[index].Power >> 5) | ((pAd->TxPower[index].Power2 & 0xE0) >> 1);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R109, BbpValue);

					// tx2 (0.1db)
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R110, &BbpValue);
					BbpValue = ((pAd->TxPower[index].Power3 & 0xE0) >> 1) | (BbpValue & 0x0F);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R110, BbpValue);

					break;
				}

				RT30xxReadRFRegister(pAd, RF_R57, (PUCHAR)&RFValue);
				if  (Channel <= 14)
					RFValue = 0x6E;
				else
					RFValue = 0x3E;
				RT30xxWriteRFRegister(pAd, RF_R57, (UCHAR)RFValue);


				// Enable RF tuning, this must be in the last, RF_R03=RF_R07 (RT30xx)
				RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x80; // bit 7=vcocal_en
				RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);

				RTMPusecDelay(2000);

				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, RFValue & 0xfe); // clear update flag
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
				pAd->RefreshTssi = 1;

				// latch channel for future usage.
				pAd->LatchRfRegs.Channel = Channel;
				
#ifdef TXBF_SUPPORT
				// Do a Divider Calibration and update BBP registers
				if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn &&
					(pAd->CommonCfg.DebugFlags & DBF_DISABLE_CAL)==0)
				{
					ITxBFLoadLNAComp(pAd);
					ITxBFDividerCalibration(pAd, 2, 0, NULL);
				}
#endif // TXBF_SUPPORT //

				DBGPRINT(RT_DEBUG_TRACE, ("RT3883: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
					Channel, 
					pAd->RfIcType, 
					TxPwer,
					TxPwer2,
					pAd->Antenna.field.TxPath,
					FreqItems3883[index].N, 
					FreqItems3883[index].K, 
					FreqItems3883[index].R));
				break;
			}
		}
	}
	else
#endif // RT3883 //
#ifdef RT305x
	// The RF programming sequence is difference between 3xxx and 2xxx
	if ((pAd->MACVersion == 0x28720200) && 
		((pAd->RfIcType == RFIC_3320) || (pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022)))
	{
		/* modify by WY for Read RF Reg. error */
		
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
				// Programming channel parameters
				RT30xxWriteRFRegister(pAd, RF_R02, FreqItems3020[index].N);
#ifdef RT3350 /*kurtis*/
				RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xF0) | (FreqItems3020[index].K & 0x0F);
				RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);
#else
				RT30xxWriteRFRegister(pAd, RF_R03, FreqItems3020[index].K);
#endif // RT3350 //


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
#ifdef RT3350 /*kurtis*/
				/*R24, BW=20M*/
				if(pAd->CommonCfg.PhyMode == PHY_11B)
					RFValue = 0x1F;
				else
					RFValue = 0x18;
#else
				RFValue &= 0xDF;
#endif // RT3350 //


				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT3350 /*kurtis*/
	//				RFValue |= 0x30;
					if(pAd->CommonCfg.PhyMode == PHY_11B)
					    RFValue = 0x3F;
					else
					    RFValue = 0x28;
#else
					RFValue |= 0x20;
#endif // RT3350 //
				}
				RT30xxWriteRFRegister(pAd, RF_R24, RFValue);

				// Rx filter
				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
				)
				{
#ifdef RT3350 /*kurtis*/
					RT30xxWriteRFRegister(pAd, RF_R31, 0x68);
#else
					RT30xxWriteRFRegister(pAd, RF_R31, 0x2F);
#endif // RT3350 //
				}
				else
				{
#ifdef RT3350 /*kurtis*/
					RT30xxWriteRFRegister(pAd, RF_R31, 0x48);
#else
					RT30xxWriteRFRegister(pAd, RF_R31, 0x0F);
#endif // RT3350 //
				}

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



				// latch channel for future usage.
				pAd->LatchRfRegs.Channel = Channel;
				
				break;				
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
			Channel, 
			pAd->RfIcType, 
			TxPwer,
			TxPwer2,
			pAd->Antenna.field.TxPath,
			FreqItems3020[index].N, 
			FreqItems3020[index].K, 
			FreqItems3020[index].R));
	}
	else
#endif // RT305x //
	{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
		ULONG	R2 = 0, R3 = DEFAULT_RF_TX_POWER, R4 = 0;
		RTMP_RF_REGS *RFRegTable;

		RFRegTable = RF2850RegTable;
#endif // Rdefined(RT28xx) || defined(RT2880) || defined(RT2883) //
	
		switch (pAd->RfIcType)
		{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
#if defined(RT28xx) || defined(RT2880) 
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:
#endif // defined(RT28xx) || defined(RT2880) //
#ifdef RT2883
			case RFIC_2853:
#endif // RT2883 //
				for (index = 0; index < NUM_OF_2850_CHNL; index++)
				{
					if (Channel == RFRegTable[index].Channel)
					{
						R2 = RFRegTable[index].R2;
						if (pAd->Antenna.field.TxPath == 1)
						{
							R2 |= 0x4000;	// If TXpath is 1, bit 14 = 1;
						}

						if (pAd->Antenna.field.RxPath == 2
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == FALSE)
#endif // GREENAP_SUPPORT //
)
						{
							R2 |= 0x40;	// write 1 to off Rxpath.
						}
						else if (pAd->Antenna.field.RxPath == 1
#ifdef GREENAP_SUPPORT
			|| (pAd->ApCfg.bGreenAPActive == TRUE)
#endif // GREENAP_SUPPORT //
)
						{
							R2 |= 0x20040;	// write 1 to off RxPath
						}
#ifdef RT2883				
						if (pAd->Antenna.field.TxPath < 3)
						{
							R2 |= 0x2000;// write 1 to bit  13
						}
#endif // RT2883 //

						if (Channel > 14)
						{
							// initialize R3, R4
							R3 = (RFRegTable[index].R3 & 0xffffc1ff);
							R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->RfFreqOffset << 15);

							// 5G band power range: 0xF9~0X0F, TX0 Reg3 bit9/TX1 Reg4 bit6="0" means the TX power reduce 7dB
							// R3
							if ((TxPwer >= -7) && (TxPwer < 0))
							{
								TxPwer = (7+TxPwer);
								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
								R3 |= (TxPwer << 10);
								DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: TxPwer=%d \n", TxPwer));
							}
							else
							{
								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
								R3 |= (TxPwer << 10) | (1 << 9);
							}

							// R4
							if ((TxPwer2 >= -7) && (TxPwer2 < 0))
							{
								TxPwer2 = (7+TxPwer2);
								TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
								R4 |= (TxPwer2 << 7);
								DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: TxPwer2=%d \n", TxPwer2));
							}
							else
							{
								TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
								R4 |= (TxPwer2 << 7) | (1 << 6);
							}                        
#ifdef RT2883
							if (IS_RT2883(pAd))
							{
								pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
								pAd->LatchRfRegs.R1 &= 0xff8fffcf; // clear bit 4,5,20,21,22

								// R1
								if ((TxPwer3 >= -7) && (TxPwer3 < 0))
								{
									TxPwer3 = (7+TxPwer3);
									TxPwer3 = (TxPwer3 > 0xF) ? (0xF) : (TxPwer3);
									pAd->LatchRfRegs.R1 |= (((TxPwer3 & 0xe) << 19) | ((TxPwer3 & 0x1) << 5));
									DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: TxPwer3=%d \n", TxPwer3));
								}
								else
								{
									TxPwer3 = (TxPwer3 > 0xF) ? (0xF) : (TxPwer3);
									pAd->LatchRfRegs.R1 |= (((TxPwer3 & 0xe) << 19) | ((TxPwer3 & 0x1) << 5)) | (1 << 4);
								}
								pAd->LatchRfRegs.R1 &= 0xfffffdff;
							}
#endif // RT2883 //
						}
						else
						{
							R3 = (RFRegTable[index].R3 & 0xffffc1ff) | (TxPwer << 9); // set TX power0
						R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->RfFreqOffset << 15) | (TxPwer2 <<6);// Set freq Offset & TxPwr1
#ifdef RT2883
							if (IS_RT2883(pAd))
							{
							pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
							pAd->LatchRfRegs.R1 &= 0xff8fffcf; // clear bit 4,5,20,21,22
							// R1
							pAd->LatchRfRegs.R1 |= (((TxPwer3 & 0x1c) << 18) | ((TxPwer3 & 0x3) << 4));
							pAd->LatchRfRegs.R1 &= 0xfffffdff;
							}
#endif // RT2883 //
						}

						// Based on BBP current mode before changing RF channel.
						if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef GREENAP_SUPPORT
			&& (pAd->ApCfg.bGreenAPActive == 0)
#endif // GREENAP_SUPPORT //
							)
						{
							R4 |=0x200000;
						}

						// Update variables
						pAd->LatchRfRegs.Channel = Channel;
#ifdef RT2883
						if (!IS_RT2883(pAd))
#endif // RT2883 //
						pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
						pAd->LatchRfRegs.R2 = R2;
						pAd->LatchRfRegs.R3 = R3;
						pAd->LatchRfRegs.R4 = R4;

						// Set RF value 1's set R3[bit2] = [0]
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
						RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

						RTMPusecDelay(200);

						// Set RF value 2's set R3[bit2] = [1]
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
						RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 | 0x04));
						RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

						RTMPusecDelay(200);

						// Set RF value 3's set R3[bit2] = [0]
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
#endif // defined(RT28xx) || defined(RT2880) || defined(RT2883) //
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
#ifdef RT3883
		if (IS_RT3883(pAd))
		{
			if (pAd->CommonCfg.RxStream > 1)
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x46);
			else
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);
		}
		else
#endif
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);//(0x44 - GET_LNA_GAIN(pAd)));	// According the Rory's suggestion to solve the middle range issue.

		//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);

		// Rx High power VGA offset for LNA select
		if (pAd->NicConfig2.field.ExternalLNAForG)
		{
#ifdef RT3883
			//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x82);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
#else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
#endif // RT3883 //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x84);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

#ifdef RT3883
		if (IS_RT3883(pAd))
		{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x8A);
		}
#endif // RT3883 //

		// 5G band selection PIN, bit1 and bit2 are complement
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);


#if defined(RT3593) || defined(RT2883) || defined(RT3883)
		if (IS_RT3593(pAd) || IS_RT2883(pAd) || IS_RT3883(pAd))
		{
			TxPinCfg = 0x32050F0A;

			//Disable unused PA_PE
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg = TxPinCfg & ~0x0300000D; //PA_PE_G2, PA_PE_A2, PA_PE_G1, PA_PE_A1, PA_PE_A0
			}
			else if (pAd->Antenna.field.TxPath == 2)
			{
				TxPinCfg = TxPinCfg & ~0x03000005; //PA_PE_G2, PA_PE_A2, PA_PE_A1, PA_PE_A0
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
#endif // defined(RT3593) || defined(RT2883) || defined(RT3883) //
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
	else
	{
		ULONG	TxPinCfg = 0x00050F05;//Gary 2007/8/9 0x050505
		UINT8	bbpValue;
		

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
#ifdef RT3883
		if (IS_RT3883(pAd))
		{
			if (pAd->CommonCfg.RxStream > 1)
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x46);	// Henry 2009-12-16
			else
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);
		}
		else
#endif // RT3883 //
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);//(0x44 - GET_LNA_GAIN(pAd)));   // According the Rory's suggestion to solve the middle range issue.     

		/* Set the BBP_R82 value here */
		bbpValue = 0xF2;


#ifdef RT2883
		if (IS_RT2883(pAd))
			bbpValue = 0x62;
#endif // RT2883 //
#ifdef RT3883
		if (IS_RT3883(pAd))
		{
			bbpValue = 0x82;
			//bbpValue = 0x96;
		}
#endif // RT3883 //
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, bbpValue);

#ifdef RT3883
		if (IS_RT3883(pAd))
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x9A);
		}
#endif // RT3883 //

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
#if defined(RT3593) || defined(RT2883) || defined(RT3883)
		if (IS_RT3593(pAd) || IS_RT2883(pAd) || IS_RT3883(pAd))
		{
			TxPinCfg = 0x31050F05;

			//Disable unused PA_PE
			if (pAd->Antenna.field.TxPath == 1)
			{
				TxPinCfg = TxPinCfg & ~0x0300000E; //PA_PE_G2, PA_PE_A2, PA_PE_G1, PA_PE_A1, PA_PE_G0
			}
			else if (pAd->Antenna.field.TxPath == 2)
			{
				TxPinCfg = TxPinCfg & ~0x0300000A; //PA_PE_G2, PA_PE_A2, PA_PE_G1, PA_PE_G0
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
#endif // defined(RT3593) || defined(RT2883) || defined(RT3883) //
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

#ifdef RT3883
	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x34);
	else
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x04);
#endif // RT3883 //
	//
	// GPIO control
	//

	// R66 should be set according to Channel and use 20MHz when scanning
	//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x2E + GET_LNA_GAIN(pAd)));
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

/*
	==========================================================================
	Description:
		This function is required for 2421 only, and should not be used during
		site survey. It's only required after NIC decided to stay at a channel
		for a longer period.
		When this function is called, it's always after AsicSwitchChannel().

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicLockChannel(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR Channel) 
{
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */

#ifdef RTMP_INTERNAL_TX_ALC
//
// Initialize the desired TSSI table
//
// Parameters
//	pAd: The adapter data structure
//
// Return Value:
//	None
//
VOID InitDesiredTSSITable(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR TSSIBase = 0; // The TSSI over OFDM 54Mbps
	USHORT TSSIStepOver2dot4G = 0; // The TSSI value/step (0.5 dB/unit)
	UCHAR RFValue = 0;
	BBP_R49_STRUC BbpR49 = {0};
	ULONG i = 0;
	CHAR Tx0PowerSetting = 0;
	USHORT TxPower = 0, TxPowerOFDM54 = 0, temp = 0;

	if (pAd->TxPowerCtrl.bInternalTxALC == FALSE)
	{
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, temp);
	TSSIBase = (temp & 0x000F);
	
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TSSI_STEP_OVER_2DOT4G - 1), TSSIStepOver2dot4G);
	TSSIStepOver2dot4G = (0x000F & (TSSIStepOver2dot4G >> 8));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_OFDM_MCS6_MCS7 - 1), TxPowerOFDM54);
	TxPowerOFDM54 = (0x000F & (TxPowerOFDM54 >> 8));

	DBGPRINT(RT_DEBUG_TRACE, ("%s: TSSIBase = %d, TSSIStepOver2dot4G = %d, TxPowerOFDM54 = %d\n", 
		__FUNCTION__, 
		TSSIBase, 
		TSSIStepOver2dot4G, 
		TxPowerOFDM54));

	//
	// The desired TSSI over CCK
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_CCK_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xDE = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverCCK[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)) + 6);
	desiredTSSIOverCCK[MCS_1] = desiredTSSIOverCCK[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_CCK_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xDF = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverCCK[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)) + 6);
	desiredTSSIOverCCK[MCS_3] = desiredTSSIOverCCK[MCS_2];

	//
	// Boundary verification: the desired TSSI value
	//
	for (i = 0; i < 4; i++) // CCK: MCS 0 ~ MCS 3
	{
		if (desiredTSSIOverCCK[i] < 0x00)
		{
			desiredTSSIOverCCK[i] = 0x00;
		}
		else if (desiredTSSIOverCCK[i] > 0x1F)
		{
			desiredTSSIOverCCK[i] = 0x1F;
		}
		else
		{
			// do nothing
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverCCK[0] = %d, desiredTSSIOverCCK[1] = %d, desiredTSSIOverCCK[2] = %d, desiredTSSIOverCCK[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverCCK[0], 
		desiredTSSIOverCCK[1], 
		desiredTSSIOverCCK[2], 
		desiredTSSIOverCCK[3]));

	//
	// The desired TSSI over OFDM
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_OFDM_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE0 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverOFDM[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverOFDM[MCS_1] = desiredTSSIOverOFDM[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_OFDM_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE1 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverOFDM[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverOFDM[MCS_3] = desiredTSSIOverOFDM[MCS_2];
	RT28xx_EEPROM_READ16(pAd, EEPROM_OFDM_MCS4_MCS5, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE2 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverOFDM[MCS_4] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverOFDM[MCS_5] = desiredTSSIOverOFDM[MCS_4];
	desiredTSSIOverOFDM[MCS_6] = TSSIBase;
	desiredTSSIOverOFDM[MCS_7] = TSSIBase;

	//
	// Boundary verification: the desired TSSI value
	//
	for (i = 0; i < 8; i++) // OFDM: MCS 0 ~ MCS 7
	{
		if (desiredTSSIOverOFDM[i] < 0x00)
		{
			desiredTSSIOverOFDM[i] = 0x00;
		}
		else if (desiredTSSIOverOFDM[i] > 0x1F)
		{
			desiredTSSIOverOFDM[i] = 0x1F;
		}
		else
		{
			// do nothing
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverOFDM[0] = %d, desiredTSSIOverOFDM[1] = %d, desiredTSSIOverOFDM[2] = %d, desiredTSSIOverOFDM[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverOFDM[0], 
		desiredTSSIOverOFDM[1], 
		desiredTSSIOverOFDM[2], 
		desiredTSSIOverOFDM[3]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverOFDM[4] = %d, desiredTSSIOverOFDM[5] = %d, desiredTSSIOverOFDM[6] = %d, desiredTSSIOverOFDM[7] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverOFDM[4], 
		desiredTSSIOverOFDM[5], 
		desiredTSSIOverOFDM[6], 
		desiredTSSIOverOFDM[7]));

	//
	// The desired TSSI over HT
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE4 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_1] = desiredTSSIOverHT[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE5 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_3] = desiredTSSIOverHT[MCS_2];
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_MCS4_MCS5, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE6 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_4] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_5] = desiredTSSIOverHT[MCS_4];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_MCS6_MCS7 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xE7 = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHT[MCS_6] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHT[MCS_7] = desiredTSSIOverHT[MCS_6];

	//
	// Boundary verification: the desired TSSI value
	//
	for (i = 0; i < 8; i++) // HT: MCS 0 ~ MCS 7
	{
		if (desiredTSSIOverHT[i] < 0x00)
		{
			desiredTSSIOverHT[i] = 0x00;
		}
		else if (desiredTSSIOverHT[i] > 0x1F)
		{
			desiredTSSIOverHT[i] = 0x1F;
		}
		else
		{
			// do nothing
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[0] = %d, desiredTSSIOverHT[1] = %d, desiredTSSIOverHT[2] = %d, desiredTSSIOverHT[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[0], 
		desiredTSSIOverHT[1], 
		desiredTSSIOverHT[2], 
		desiredTSSIOverHT[3]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHT[4] = %d, desiredTSSIOverHT[5] = %d, desiredTSSIOverHT[6] = %d, desiredTSSIOverHT[7] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHT[4], 
		desiredTSSIOverHT[5], 
		desiredTSSIOverHT[6], 
		desiredTSSIOverHT[7]));

	//
	// The desired TSSI over HT using STBC
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_USING_STBC_MCS0_MCS1, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xEC = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_0] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_1] = desiredTSSIOverHTUsingSTBC[MCS_0];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_USING_STBC_MCS2_MCS3 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xED = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_2] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_3] = desiredTSSIOverHTUsingSTBC[MCS_2];
	RT28xx_EEPROM_READ16(pAd, EEPROM_HT_USING_STBC_MCS4_MCS5, TxPower);
	TxPower = (TxPower & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xEE = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_4] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_5] = desiredTSSIOverHTUsingSTBC[MCS_4];
	RT28xx_EEPROM_READ16(pAd, (EEPROM_HT_USING_STBC_MCS6_MCS7 - 1), TxPower);
	TxPower = ((TxPower >> 8) & 0x000F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0xEF = 0x%X\n", __FUNCTION__, TxPower));
	desiredTSSIOverHTUsingSTBC[MCS_6] = (TSSIBase + ((TxPower - TxPowerOFDM54) * (TSSIStepOver2dot4G * 2)));
	desiredTSSIOverHTUsingSTBC[MCS_7] = desiredTSSIOverHTUsingSTBC[MCS_6];

	//
	// Boundary verification: the desired TSSI value
	//
	for (i = 0; i < 8; i++) // HT using STBC: MCS 0 ~ MCS 7
	{
		if (desiredTSSIOverHTUsingSTBC[i] < 0x00)
		{
			desiredTSSIOverHTUsingSTBC[i] = 0x00;
		}
		else if (desiredTSSIOverHTUsingSTBC[i] > 0x1F)
		{
			desiredTSSIOverHTUsingSTBC[i] = 0x1F;
		}
		else
		{
			// do nothing
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHTUsingSTBC[0] = %d, desiredTSSIOverHTUsingSTBC[1] = %d, desiredTSSIOverHTUsingSTBC[2] = %d, desiredTSSIOverHTUsingSTBC[3] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHTUsingSTBC[0], 
		desiredTSSIOverHTUsingSTBC[1], 
		desiredTSSIOverHTUsingSTBC[2], 
		desiredTSSIOverHTUsingSTBC[3]));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSIOverHTUsingSTBC[4] = %d, desiredTSSIOverHTUsingSTBC[5] = %d, desiredTSSIOverHTUsingSTBC[6] = %d, desiredTSSIOverHTUsingSTBC[7] = %d\n", 
		__FUNCTION__, 
		desiredTSSIOverHTUsingSTBC[4], 
		desiredTSSIOverHTUsingSTBC[5], 
		desiredTSSIOverHTUsingSTBC[6], 
		desiredTSSIOverHTUsingSTBC[7]));

	RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)(&RFValue));
	RFValue = (RFValue | 0x88); // <7>: IF_Rxout_en, <3>: IF_Txout_en
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

	RT30xxReadRFRegister(pAd, RF_R28, (PUCHAR)(&RFValue));
	RFValue = (RFValue & (~0x60)); // <6:5>: tssi_atten
	RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
	BbpR49.field.adc5_in_sel = 1; // Enable the PSI (internal components, new version - RT3390)
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpR49.byte);		

	DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
}

/*
	==========================================================================
	Description:
		Get the desired TSSI based on the latest packet

	Arguments:
		pAd

	Return Value:
		The desired TSSI
	==========================================================================
 */
UCHAR GetDesiredTSSI(
	IN PRTMP_ADAPTER pAd)
{
	PHTTRANSMIT_SETTING pLatestTxHTSetting = (PHTTRANSMIT_SETTING)(&pAd->LastTxRate);
	UCHAR desiredTSSI = 0;
	UCHAR MCS = 0;

	MCS = (UCHAR)(pLatestTxHTSetting->field.MCS);
	
	if (pLatestTxHTSetting->field.MODE == MODE_CCK)
	{
		if ((MCS < 0) || (MCS > 3)) // boundary verification
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));
			
			MCS = 0;
		}
	
		desiredTSSI = desiredTSSIOverCCK[MCS];
	}
	else if (pLatestTxHTSetting->field.MODE == MODE_OFDM)
	{
		if ((MCS < 0) || (MCS > 7)) // boundary verification
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));

			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverOFDM[MCS];
	}
	else if ((pLatestTxHTSetting->field.MODE == MODE_HTMIX) || (pLatestTxHTSetting->field.MODE == MODE_HTGREENFIELD))
	{
		if ((MCS < 0) || (MCS > 7)) // boundary verification
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));

			MCS = 0;
		}

		if (pLatestTxHTSetting->field.STBC == 1)
		{
			desiredTSSI = desiredTSSIOverHT[MCS];
		}
		else
		{
			desiredTSSI = desiredTSSIOverHTUsingSTBC[MCS];
		}

		//
		// For HT BW40 MCS 7 with/without STBC configuration, the desired TSSI value should subtract one from the formula.
		//
		if ((pLatestTxHTSetting->field.BW == BW_40) && (MCS == MCS_7))
		{
			desiredTSSI -= 1;
		}
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: desiredTSSI = %d, Latest Tx HT setting: MODE = %d, MCS = %d, STBC = %d\n", 
		__FUNCTION__, 
		desiredTSSI, 
		pLatestTxHTSetting->field.MODE, 
		pLatestTxHTSetting->field.MCS, 
		pLatestTxHTSetting->field.STBC));

	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));

	return desiredTSSI;
}
#endif // RTMP_INTERNAL_TX_ALC //

VOID AsicGetAutoAgcOffset(
	IN PRTMP_ADAPTER pAd,
	IN PCHAR pDeltaPwr,
	IN PCHAR pTotalDeltaPwr,
	IN PCHAR pAgcCompensate,
	IN PUCHAR pBbpR49)
{
	CHAR            DeltaPwr = 0;
	BOOLEAN		bAutoTxAgc = FALSE;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep;
//	UCHAR		BbpR49 = 0, idx;
	UCHAR		idx;
	PCHAR		pTxAgcCompensate;
	BBP_R49_STRUC BbpR49;
	CHAR		TotalDeltaPower = 0; // (non-positive number) including the transmit power controlled by the MAC and the BBP R1
#ifdef RTMP_INTERNAL_TX_ALC
	UCHAR desiredTSSI = 0, currentTSSI = 0;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	UCHAR RFValue = 0;
#endif // RTMP_INTERNAL_TX_ALC //
#if defined(RT3090) || defined(RT3572) || defined(RT3390) || defined(RT3593)
    unsigned long flags;
#endif // RT3090 || RT3572 || RT3390 || RT3593 //

	BbpR49.byte = 0;

#ifdef RTMP_INTERNAL_TX_ALC
	// Locate the internal Tx ALC tuning entry
	if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
	{
		if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
		{
			desiredTSSI = GetDesiredTSSI(pAd);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
			currentTSSI = BbpR49.field.TSSI;

			if (desiredTSSI > currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable++;
			}

			if (desiredTSSI < currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable--;
			}

			if (pAd->TxPowerCtrl.idxTxPowerTable < LOWERBOUND_TX_POWER_TUNING_ENTRY)
			{
				pAd->TxPowerCtrl.idxTxPowerTable = LOWERBOUND_TX_POWER_TUNING_ENTRY;
			}

			if (pAd->TxPowerCtrl.idxTxPowerTable >= UPPERBOUND_TX_POWER_TUNING_ENTRY)
			{
				pAd->TxPowerCtrl.idxTxPowerTable = UPPERBOUND_TX_POWER_TUNING_ENTRY;
			}

			//
			// Valide pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 45
			//

			pTxPowerTuningEntry = &TxPowerTuningTable[pAd->TxPowerCtrl.idxTxPowerTable + TX_POWER_TUNING_ENTRY_OFFSET]; // zero-based array
			pAd->TxPowerCtrl.RF_R12_Value = pTxPowerTuningEntry->RF_R12_Value;
			pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

			//
			// Tx power adjustment over RF
			//
			RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)(&RFValue));
			RFValue = ((RFValue & 0xE0) | pAd->TxPowerCtrl.RF_R12_Value);
			RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)(RFValue));

			//
			// Tx power adjustment over MAC
			//
			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, currentTSSI = %d, idxTxPowerTable = %d, {RF_R12_Value = %d, MAC_PowerDelta = %d}\n", 
				__FUNCTION__, 
				desiredTSSI, 
				currentTSSI, 
				pAd->TxPowerCtrl.idxTxPowerTable, 
				pTxPowerTuningEntry->RF_R12_Value, 
				pTxPowerTuningEntry->MAC_PowerDelta));
		}
		else
		{
			//
			// Tx power adjustment over RF
			//
			RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)(&RFValue));
			RFValue = ((RFValue & 0xE0) | pAd->TxPowerCtrl.RF_R12_Value);
			RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)(RFValue));

			//
			// Tx power adjustment over MAC
			//
			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;
		}
	}
	
#endif // RTMP_INTERNAL_TX_ALC //

	// TX power compensation for temperature variation based on TSSI. try every 4 second
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			/* bg channel */
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			TssiRef            = pAd->TssiRefG;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryG[0];
			TxAgcStep          = pAd->TxAgcStepG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			/* a channel */
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			TssiRef            = pAd->TssiRefA;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryA[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryA[0];
			TxAgcStep          = pAd->TxAgcStepA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
		{
			/* BbpR1 is unsigned char */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
			/* (p) TssiPlusBoundaryG[0] = 0 = (m) TssiMinusBoundaryG[0] */
			/* compensate: +4     +3   +2   +1    0   -1   -2   -3   -4 * steps */
			/* step value is defined in pAd->TxAgcStepG for tx power value */

			/* [4]+1+[4]   p4     p3   p2   p1   o1   m1   m2   m3   m4 */
			/* ex:         0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
			   above value are examined in mass factory production */
			/*             [4]    [3]  [2]  [1]  [0]  [1]  [2]  [3]  [4] */

			/* plus (+) is 0x00 ~ 0x45, minus (-) is 0xa0 ~ 0xf0 */
			/* if value is between p1 ~ o1 or o1 ~ s1, no need to adjust tx power */
			/* if value is 0xa5, tx power will be -= TxAgcStep*(2-1) */

			if (BbpR49.byte > pTssiMinusBoundary[1])
			{
				// Reading is larger than the reference value
				// check for how large we need to decrease the Tx power
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49.byte <= pTssiMinusBoundary[idx])  // Found the range
						break;
				}
				// The index is the step we should decrease, idx = 0 means there is nothing to compensate
#ifdef RT3883
				if ((idx == 5) && ((BbpR49.byte) >  pTssiMinusBoundary[4] + 8))
					idx += 1;
#endif // RT3883 //
//				if (R3 > (ULONG) (TxAgcStep * (idx-1)))
					*pTxAgcCompensate = -(TxAgcStep * (idx-1));
//				else
//					*pTxAgcCompensate = -((UCHAR)R3);
				
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
					                BbpR49.byte, TssiRef, TxAgcStep, idx-1));                    
			}
			else if (BbpR49.byte < pTssiPlusBoundary[1])
			{
				// Reading is smaller than the reference value
				// check for how large we need to increase the Tx power
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49.byte >= pTssiPlusBoundary[idx])   // Found the range
						break;
				}
#ifdef RT3883
				if ((idx == 5) && ((BbpR49.byte) <  pTssiPlusBoundary[4] - 16))
					idx += 2;
				else if ((idx == 5) && ((BbpR49.byte) <  pTssiPlusBoundary[4] - 8))
					idx += 1;
#endif // RT3883 //
				// The index is the step we should increase, idx = 0 means there is nothing to compensate
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					                BbpR49.byte, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					                BbpR49.byte, TssiRef, TxAgcStep, 0));
			}
		}
	}
	else
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}

	*pBbpR49 = BbpR49.byte;
#ifdef RT3883
	*pDeltaPwr = (DeltaPwr - 2);
#else
	*pDeltaPwr = DeltaPwr;
#endif // RT3883 //
	*pTotalDeltaPwr = TotalDeltaPower;
	*pAgcCompensate = *pTxAgcCompensate;
}

VOID AsicGetTxPowerOffset(
	IN PRTMP_ADAPTER pAd,
	IN PULONG TxPwr)
{
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;

#if defined(RT2883) || defined (RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		CfgOfTxPwrCtrlOverMAC.NumOfEntries = MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS; // MAC 0x1314, 0x1318, 0x131C, 0x1320 and 1324

		if (pAd->CommonCfg.BBPCurrentBW == BW_40)
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgABand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx40MPwrCfgABand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgABand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx40MPwrCfgABand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgABand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx40MPwrCfgABand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx40MPwrCfgABand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx40MPwrCfgABand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx40MPwrCfgABand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx40MPwrCfgABand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx40MPwrCfgABand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx40MPwrCfgABand[6];
#ifdef RT3883
				if (IS_RT3883(pAd))
				{
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx40MPwrCfgABand[7];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx40MPwrCfgABand[8];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx40MPwrCfgABand[9];
				}
#endif // RT3883 //
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgGBand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx40MPwrCfgGBand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgGBand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx40MPwrCfgGBand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgGBand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx40MPwrCfgGBand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx40MPwrCfgGBand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx40MPwrCfgGBand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx40MPwrCfgGBand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx40MPwrCfgGBand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx40MPwrCfgGBand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx40MPwrCfgGBand[6];
#ifdef RT3883
				if (IS_RT3883(pAd))
				{
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx40MPwrCfgGBand[7];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx40MPwrCfgGBand[8];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx40MPwrCfgGBand[9];
				}
#endif // RT3883 //
				
			}
		}
		else
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgABand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx20MPwrCfgABand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgABand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx20MPwrCfgABand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgABand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx20MPwrCfgABand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx20MPwrCfgABand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx20MPwrCfgABand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx20MPwrCfgABand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx20MPwrCfgABand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx20MPwrCfgABand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx20MPwrCfgABand[6];
#ifdef RT3883
				if (IS_RT3883(pAd))
				{
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx20MPwrCfgABand[7];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx20MPwrCfgABand[8];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx20MPwrCfgABand[9];
				}
#endif // RT3883 //
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgGBand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_0_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = ((pAd->Tx20MPwrCfgGBand[0] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgGBand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_1_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = ((pAd->Tx20MPwrCfgGBand[1] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgGBand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_2_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = ((pAd->Tx20MPwrCfgGBand[2] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[6].RegisterValue = pAd->Tx20MPwrCfgGBand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].MACRegisterOffset = TX_PWR_CFG_3_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[7].RegisterValue = ((pAd->Tx20MPwrCfgGBand[3] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[8].RegisterValue = pAd->Tx20MPwrCfgGBand[4];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].MACRegisterOffset = TX_PWR_CFG_4_EXT;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[9].RegisterValue = ((pAd->Tx20MPwrCfgGBand[4] & 0xf0f0f0f0) >> 4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].MACRegisterOffset = TX_PWR_CFG_5;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[10].RegisterValue = pAd->Tx20MPwrCfgGBand[5];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].MACRegisterOffset = TX_PWR_CFG_6;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[11].RegisterValue = pAd->Tx20MPwrCfgGBand[6];
#ifdef RT3883
				if (IS_RT3883(pAd))
				{
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].MACRegisterOffset = TX_PWR_CFG_7;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[12].RegisterValue = pAd->Tx20MPwrCfgGBand[7];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].MACRegisterOffset = TX_PWR_CFG_8;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[13].RegisterValue = pAd->Tx20MPwrCfgGBand[8];
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].MACRegisterOffset = TX_PWR_CFG_9;
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[14].RegisterValue = pAd->Tx20MPwrCfgGBand[9];
				}
#endif // RT3883 //
			}
		}
		NdisCopyMemory(TxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));
	}
	else
#endif // defined(RT2883) || defined (RT3883) //
	/* non-3593 */
	{
		CfgOfTxPwrCtrlOverMAC.NumOfEntries = 5; // MAC 0x1314, 0x1318, 0x131C, 0x1320 and 1324

		if (pAd->CommonCfg.BBPCurrentBW == BW_40)
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgABand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgABand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgABand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgABand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgABand[4];
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgGBand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgGBand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgGBand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgGBand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgGBand[4];
			}
		}
		else
		{
			if (pAd->CommonCfg.CentralChannel > 14)
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgABand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx20MPwrCfgABand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgABand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx20MPwrCfgABand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgABand[4];
			}
			else
			{
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgGBand[0];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx20MPwrCfgGBand[1];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgGBand[2];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx20MPwrCfgGBand[3];
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgGBand[4];
			}
		}

		NdisCopyMemory(TxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));
	}


}

#ifdef SINGLE_SKU
/*
	==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in LINK UP in INFRASTRUCTURE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. if current radio is a noisy environment (pAd->DrsCounters.fNoisyEnvironment)
		1. TxPowerPercentage
		2. auto calibration based on TSSI feedback
		3. extra 2 db for CCK
		4. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

	NOTE: Since this routine requires the value of (pAd->DrsCounters.fNoisyEnvironment),
		it should be called AFTER MlmeDynamicTxRatSwitching()
	==========================================================================
 */
VOID AsicAdjustSingleSkuTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	INT			i, j;
	CHAR		DeltaPwr = 0;
	UCHAR		BbpR1 = 0, BbpR49 = 0, BbpR1Offset = 0;
	CHAR		TxAgcCompensate;
//	ULONG		TxPwr[MAX_TXPOWER_ARRAY_SIZE];
	ULONG		TotalDeltaPwr[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
	CHAR		Value, MinValue = 127;
	CHAR		TotalDeltaPower = 0; // (non-positive number) including the transmit power controlled by the MAC and the BBP R1
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;
#ifdef CONFIG_STA_SUPPORT
	CHAR		Rssi = -127;
#endif // CONFIG_STA_SUPPORT //

	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC));
	

#ifdef CONFIG_STA_SUPPORT
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) || 
#ifdef RTMP_MAC_PCI
		(pAd->bPCIclkOff == TRUE) ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF) ||
#endif // RTMP_MAC_PCI //
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
{
		return;
}

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		Rssi = RTMPMaxRssi(pAd, 
						   pAd->StaCfg.RssiSample.AvgRssi0, 
						   pAd->StaCfg.RssiSample.AvgRssi1, 
						   pAd->StaCfg.RssiSample.AvgRssi2);
#endif // CONFIG_STA_SUPPORT //

        NdisZeroMemory(TotalDeltaPwr, sizeof(TotalDeltaPwr));
        /* Get Tx Rate Offset Table which from eeprom 0xDEh ~ 0xEFh */
        AsicGetTxPowerOffset(pAd, &CfgOfTxPwrCtrlOverMAC);
        /* Get temperature compensation Delta Power Value */
        AsicGetAutoAgcOffset(pAd, &DeltaPwr, &TotalDeltaPower, &TxAgcCompensate, &BbpR49);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	BbpR1 &= 0xFC;

	// Handle regulatory max tx power constrain
	do
	{
		UCHAR    TxPwrInEEPROM = 0xFF, CountryTxPwr = 0xFF, criterion;
//		UCHAR    AdjustMaxTxPwr[MAX_TXPOWER_ARRAY_SIZE * 8]; 
		UCHAR    AdjustMaxTxPwr[(MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS * 8)]; 

#ifdef RT2883
		if (IS_RT2883(pAd))
		{
			if (pAd->CommonCfg.Channel > 14) // 5G band
				TxPwrInEEPROM = (pAd->CommonCfg.DefineMaxTxPwr & 0x00FF);
			else // 2.4G band
				TxPwrInEEPROM = ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF00) >> 8);
		}
		else
#endif // RT2883 //
		{
			if (pAd->CommonCfg.Channel > 14) // 5G band
				TxPwrInEEPROM = ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF00) >> 8);
			else // 2.4G band
				TxPwrInEEPROM = (pAd->CommonCfg.DefineMaxTxPwr & 0x00FF);
		}

		CountryTxPwr = GetCuntryMaxTxPwr(pAd, pAd->CommonCfg.Channel); 

		//
		// FAE uses OFDM 6M as criterion
		//
//		criterion = (TxPwr[0] >> 16) & 0xF;        // FAE use OFDM 6M as criterion
		criterion = (UCHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue & 0x000F0000) >> 16);

		DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicAdjustSingleSkuTxPower (criterion=%d, TxPwrInEEPROM=%d, CountryTxPwr=%d)\n", criterion, TxPwrInEEPROM, CountryTxPwr));
 
		// Adjust max tx power according to the relationship of tx power in E2PROM
//		for (i=0; i<MAX_TXPOWER_ARRAY_SIZE; i++)
		for (i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
		{
			// CCK will have 4dBm larger than OFDM
			// Therefore, we should separate to parse the tx power field
			if (i == 0)
			{
				for (j=0; j<8; j++)
				{
//					Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F);
					Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);
 
					if (j < 4)
					{
						// CCK will have 4dBm larger than OFDM
						AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion) + 4;
					}
					else
					{
						AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion);
					}
//					DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicAdjustSingleSkuTxPower (i/j=%d/%d, Value=%d, %d)\n", i, j, Value, AdjustMaxTxPwr[i*8+j]));

					DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicAdjustSingleSkuTxPower (offset = 0x%04X, i/j=%d/%d, Value=%d, %d)\n", 
						i, 
						j, 
						Value, 
						AdjustMaxTxPwr[i*8+j], 
						CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset));
				}
			}
			else
			{
				for (j=0; j<8; j++)
				{
					Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);
 
					AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion);
//					DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicAdjustSingleSkuTxPower (i/j=%d/%d, Value=%d, %d)\n", i, j, Value, AdjustMaxTxPwr[i*8+j]));

					DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicAdjustSingleSkuTxPower (offset = 0x%04X, i/j=%d/%d, Value=%d, %d)\n", 
						CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
						i, 
						j, 
						Value, 
						AdjustMaxTxPwr[i*8+j]));
				}
			}
		}
 
		// Adjust tx power according to the relationship
//		for (i=0; i<MAX_TXPOWER_ARRAY_SIZE; i++)
		for (i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
		{
//			if (TxPwr[i] != 0xffffffff)
			if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
			{
				for (j=0; j<8; j++)
				{
//					Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F);
					Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

					// The system tx power is larger than the regulatory, the power should be restrain
					if (AdjustMaxTxPwr[i*8+j] > CountryTxPwr)
					{
						Value = (AdjustMaxTxPwr[i*8+j] - CountryTxPwr);
						if (Value > 0xF)
						{
						    /* If print the Error msg. It means the output power larger than Country Regulatory over 15dBm,
						      * the origianl design has overflow case.
						      */
						    DBGPRINT_RAW(RT_DEBUG_ERROR,("AsicAdjustSingleSkuTxPower: Value overflow - %d\n", Value));
						}
						TotalDeltaPwr[i] = (TotalDeltaPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);

//						DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicAdjustSingleSkuTxPower (i/j=%d/%d, Value=%d, %d)\n", i, j, Value, AdjustMaxTxPwr[i*8+j]));
						DBGPRINT_RAW(RT_DEBUG_INFO,("AsicAdjustSingleSkuTxPower (offset = 0x%04X, i/j=%d/%d, Value=%d, %d)\n", 
							CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
							i, 
							j, 
							Value, 
							AdjustMaxTxPwr[i*8+j]));
					}
					else
					{
//						DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicAdjustSingleSkuTxPower (i/j=%d/%d, Value=%d, %d, no change)\n", i, j, Value, AdjustMaxTxPwr[i*8+j]));
						DBGPRINT_RAW(RT_DEBUG_INFO,("AsicAdjustSingleSkuTxPower (offset = 0x%04X, i/j=%d/%d, Value=%d, %d, no change)\n", 
							CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
							i, 
							j, 
							Value, 
							AdjustMaxTxPwr[i*8+j]));
					}
				}
			}
		}
	} while (FALSE);

//	TotalDeltaPower += DeltaPowerByBbpR1; // the transmit power controlled by the BBP R1
//	TotalDeltaPower += DeltaPwr; // the transmit power controlled by the MAC	
	DeltaPwr += TotalDeltaPower;

	/* calculate delta power based on the percentage specified from UI */
	// E2PROM setting is calibrated for maximum TX power (i.e. 100%)
	// We lower TX power here according to the percentage specified from UI
	if (pAd->CommonCfg.TxPowerPercentage == 0xffffffff)       // AUTO TX POWER control
	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			// to patch high power issue with some APs, like Belkin N1.
			if (Rssi > -35)
			{
				DeltaPwr -= 12;
			}
			else if (Rssi > -40)
			{
				DeltaPwr -= 6;
			}
			else
		;
		}
#endif // CONFIG_STA_SUPPORT //
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 90)  // 91 ~ 100% & AUTO, treat as 100% in terms of mW
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60)  // 61 ~ 90%, treat as 75% in terms of mW		// DeltaPwr -= 1;
	{
		DeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30)  // 31 ~ 60%, treat as 50% in terms of mW		// DeltaPwr -= 3;
	{
		DeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15)  // 16 ~ 30%, treat as 25% in terms of mW		// DeltaPwr -= 6;
	{
		DeltaPwr -= 6;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9)   // 10 ~ 15%, treat as 12.5% in terms of mW		// DeltaPwr -= 9;
	{
		DeltaPwr -= 9;
	}
	else                                           // 0 ~ 9 %, treat as MIN(~3%) in terms of mW		// DeltaPwr -= 12;
	{
		DeltaPwr -= 12;
	}

	/* reset different new tx power for different TX rate */

	/* Calcuate the minimum transmit power */
//	for(i=0; i<MAX_TXPOWER_ARRAY_SIZE; i++)
	for(i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
//		if (TxPwr[i] != 0xffffffff)
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				/* After Single SKU, each data rate offset power value is saved in TotalDeltaPwr[].
				   PwrChange will add DeltaPwr and TotalDeltaPwr[] for each data rate to calculate
				   the final adjust output power value which is saved in MAC Reg. and BBP_R1 */
				CHAR PwrChange;

				/* Value / TxPwr[] is get from eeprom 0xDEh ~ 0xEFh and increase or decrease the  
				   20/40 Bandwidth Delta Value in eeprom 0x50h. */
//				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); /* 0 ~ 15 */

				/* Fix the corner case of Single SKU read eeprom offset 0xF0h ~ 0xFEh which for BBP Instruction configuration */
				if (Value == 0xF)
					continue;

				/* Value_offset is current Pwr comapre with Country Regulation and need adjust delta value */
				PwrChange = (CHAR)((TotalDeltaPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */
				PwrChange -= DeltaPwr;

				Value -= PwrChange;
				
				if(MinValue > Value)
					MinValue = Value;				
			}
		}
	}
	
	/* Depend on the minimum transmit power to adjust static Tx power control 
	   Prevent the value of MAC_TX_PWR_CFG less than 0. */
		
	if((MinValue < 0)&&(MinValue >= -6))
	{
		BbpR1 |= 0x01;
		BbpR1Offset = 6;
	}
	else if ((MinValue < -6)&&(MinValue >= -12))
	{
		BbpR1 |= 0x02;
		BbpR1Offset = 12;
	}
	else if (MinValue < -12)
	{
		DBGPRINT(RT_DEBUG_WARN, ("AsicAdjustTxPower: ASIC limit..\n"));
		BbpR1 |= 0x02;
		BbpR1Offset = 12;
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);


//	for(i=0; i<MAX_TXPOWER_ARRAY_SIZE; i++)
	for (i=0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
//		if (TxPwr[i] != 0xffffffff)
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				/* After Single SKU, each data rate offset power value is saved in TotalDeltaPwr[].
				   PwrChange will add DeltaPwr and TotalDeltaPwr[] for each data rate to calculate
				   the final adjust output power value which is saved in MAC Reg. and BBP_R1 */
				CHAR PwrChange;

				/* Value / TxPwr[] is get from eeprom 0xDEh ~ 0xEFh and increase or decrease the  
				   20/40 Bandwidth Delta Value in eeprom 0x50h. */
//				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); /* 0 ~ 15 */

				/* Value_offset is current Pwr comapre with Country Regulation and need adjust delta value */
				PwrChange = (CHAR)((TotalDeltaPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */
				PwrChange -= DeltaPwr;

				Value -= (PwrChange - BbpR1Offset);

				if (Value < 0)
					Value = 0; /* min */

				if (Value > 0xF)
					Value = 0xF; /* max */

				/* fill new value to CSR offset */
//				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue = (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			/* write tx power value to CSR */
			/* TX_PWR_CFG_0 (8 tx rate) for	TX power for OFDM 12M/18M
											TX power for OFDM 6M/9M
											TX power for CCK5.5M/11M
											TX power for CCK1M/2M */
			/* TX_PWR_CFG_1 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue);

		}
	}



}
#endif // SINGLE_SKU //

/*
	==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in LINK UP in INFRASTRUCTURE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. if current radio is a noisy environment (pAd->DrsCounters.fNoisyEnvironment)
		1. TxPowerPercentage
		2. auto calibration based on TSSI feedback
		3. extra 2 db for CCK
		4. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

	NOTE: Since this routine requires the value of (pAd->DrsCounters.fNoisyEnvironment),
		it should be called AFTER MlmeDynamicTxRatSwitching()
	==========================================================================
 */
#define MDSM_NORMAL_TX_POWER							0x00
#define MDSM_DROP_TX_POWER_BY_6dBm					0x01
#define MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define MDSM_ADD_TX_POWER_BY_6dBm						0x03
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK	0x03
VOID AsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	INT			i, j;
	CHAR		DeltaPwr = 0;
	UCHAR		BbpR1 = 0, BbpR49 = 0;
	CHAR		TxAgcCompensate;
//	ULONG		TxPwr[MAX_TXPOWER_ARRAY_SIZE];
	CHAR		Value;
	CHAR		DeltaPowerByBbpR1 = 0; // non-positive number
	CHAR		TotalDeltaPower = 0; // (non-positive number) including the transmit power controlled by the MAC and the BBP R1	
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;


#ifdef CONFIG_STA_SUPPORT
	CHAR		Rssi = -127;
#endif // CONFIG_STA_SUPPORT //
	
	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC));

#ifdef CONFIG_STA_SUPPORT
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) || 
#ifdef RTMP_MAC_PCI
		(pAd->bPCIclkOff == TRUE) ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF) ||
#endif // RTMP_MAC_PCI //
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		return;
	}

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		Rssi = RTMPMaxRssi(pAd, 
						   pAd->StaCfg.RssiSample.AvgRssi0, 
						   pAd->StaCfg.RssiSample.AvgRssi1, 
						   pAd->StaCfg.RssiSample.AvgRssi2);
#endif // CONFIG_STA_SUPPORT //

#ifdef SINGLE_SKU
	if (pAd->CommonCfg.bSKUMode == TRUE)
	{
		AsicAdjustSingleSkuTxPower(pAd);
		return;
	}
#endif // SINGLE_SKU //

	/* Get Tx Rate Offset Table which from eeprom 0xDEh ~ 0xEFh */
//	AsicGetTxPowerOffset(pAd, (PULONG) &TxPwr);
	AsicGetTxPowerOffset(pAd, (PULONG) &CfgOfTxPwrCtrlOverMAC);
	/* Get temperature compensation Delta Power Value */
	AsicGetAutoAgcOffset(pAd, &DeltaPwr, &TotalDeltaPower, &TxAgcCompensate, &BbpR49);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	BbpR1 &= 0xFC;

	/* calculate delta power based on the percentage specified from UI */
	// E2PROM setting is calibrated for maximum TX power (i.e. 100%)
	// We lower TX power here according to the percentage specified from UI
	if (pAd->CommonCfg.TxPowerPercentage == 0xffffffff)       // AUTO TX POWER control
	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			// to patch high power issue with some APs, like Belkin N1.
			if (Rssi > -35)
			{
				BbpR1 |= 0x02;		// DeltaPwr -= 12;
			}
			else if (Rssi > -40)
			{
				BbpR1 |= 0x01;		// DeltaPwr -= 6;
			}
			else
		;
		}
#endif // CONFIG_STA_SUPPORT //
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 90)  // 91 ~ 100% & AUTO, treat as 100% in terms of mW
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60)  // 61 ~ 90%, treat as 75% in terms of mW		// DeltaPwr -= 1;
	{
		DeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30)  // 31 ~ 60%, treat as 50% in terms of mW		// DeltaPwr -= 3;
	{
		DeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15)  // 16 ~ 30%, treat as 25% in terms of mW		// DeltaPwr -= 6;
	{
		DeltaPowerByBbpR1 -= 6; // -6 dBm
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9)   // 10 ~ 15%, treat as 12.5% in terms of mW		// DeltaPwr -= 9;
	{
		DeltaPowerByBbpR1 -= 6; // -6 dBm
		DeltaPwr -= 3;
	}
	else                                           // 0 ~ 9 %, treat as MIN(~3%) in terms of mW		// DeltaPwr -= 12;
	{
		DeltaPowerByBbpR1 -= 12; // -12 dBm
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);


	TotalDeltaPower += DeltaPowerByBbpR1; // the transmit power controlled by the BBP R1
	TotalDeltaPower += DeltaPwr; // the transmit power controlled by the MAC	

	// The BBP R1 controls the transmit power for all rates
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	BbpR1 &= ~MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;

	if (TotalDeltaPower <= -12)
	{
		TotalDeltaPower += 12;
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

		DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: Drop the transmit power by 12 dBm (BBP R1)\n", __FUNCTION__));
	}
	else if ((TotalDeltaPower <= -6) && (TotalDeltaPower > -12))
	{
		TotalDeltaPower += 6;
		BbpR1 |= MDSM_DROP_TX_POWER_BY_6dBm;		

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

		DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: Drop the transmit power by 6 dBm (BBP R1)\n", __FUNCTION__));
	}
	else
	{
		// Control the the transmit power by using the MAC only
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);
	}

	/* reset different new tx power for different TX rate */
//	for(i=0; i<MAX_TXPOWER_ARRAY_SIZE; i++)
	for (i=0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

#ifdef RTMP_INTERNAL_TX_ALC
				// The upper bounds of the MAC 0x1314~0x1324 are variable when the STA uses the internal Tx ALC.
				if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
				{
					switch (TX_PWR_CFG_0 + (i * 4))
					{
						case TX_PWR_CFG_0: 
						{
							if ((Value + TotalDeltaPower) < 0)
							{
								Value = 0;
							}
							else if ((Value + TotalDeltaPower) > 0xE)
							{
								Value = 0xE;
							}
							else
							{
								Value += TotalDeltaPower;
							}
						}
						break;

						case TX_PWR_CFG_1: 
						{
							if ((j >= 0) && (j <= 3))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_2: 
						{
							if ((j == 0) || (j == 2) || (j == 3))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_3: 
						{
							if ((j == 0) || (j == 2) || (j == 3) || 
							    ((j >= 4) && (j <= 7)))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_4: 
						{
							if ((Value + TotalDeltaPower) < 0)
							{
								Value = 0;
							}
							else if ((Value + TotalDeltaPower) > 0xC)
							{
								Value = 0xC;
							}
							else
							{
								Value += TotalDeltaPower;
							}
						}
						break;

						default: 
						{							
							// do nothing
							DBGPRINT(RT_DEBUG_ERROR, ("%s: unknown register = 0x%X\n", 
								__FUNCTION__, 
								(TX_PWR_CFG_0 + (i * 4))));
						}
						break;
					}
				}
				else
#endif // RTMP_INTERNAL_TX_ALC //
				{
					if ((Value + TotalDeltaPower) < 0)
					{
						Value = 0; /* min */
					}
					else if ((Value + TotalDeltaPower) > 0xF)
					{
						Value = 0xF; /* max */
					}
					else
					{
						Value += TotalDeltaPower; /* temperature compensation */
					}
				}
				/* fill new value to CSR offset */
				CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue = (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			/* write tx power value to CSR */
			/* TX_PWR_CFG_0 (8 tx rate) for	TX power for OFDM 12M/18M
											TX power for OFDM 6M/9M
											TX power for CCK5.5M/11M
											TX power for CCK1M/2M */
			/* TX_PWR_CFG_1 ~ TX_PWR_CFG_4 */

			{
//				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, TxPwr[i]);
				RTMP_IO_WRITE32(pAd, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue);
			}


		}
	}

}


#ifdef CONFIG_STA_SUPPORT
VOID AsicResetBBPAgent(
IN PRTMP_ADAPTER pAd)
{
	BBP_CSR_CFG_STRUC	BbpCsr;
	DBGPRINT(RT_DEBUG_ERROR, ("Reset BBP Agent busy bit.!! \n"));
	// Still need to find why BBP agent keeps busy, but in fact, hardware still function ok. Now clear busy first.	
	RTMP_IO_READ32(pAd, H2M_BBP_AGENT, &BbpCsr.word);
	BbpCsr.field.Busy = 0;
	RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
}
/*
	==========================================================================
	Description:
		put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup 
		automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
		the wakeup timer timeout. Driver has to issue a separate command to wake
		PHY up.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TbttNumToNextWakeUp) 
{
	RTMP_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp);
}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever manual wakeup is required
		AsicForceSleep() should only be used when not in INFRA BSS. When
		in INFRA BSS, we should use AsicSleepThenAutoWakeup() instead.
	==========================================================================
 */
VOID AsicForceSleep(
	IN PRTMP_ADAPTER pAd)
{

}

/*
	==========================================================================
	Description:
		AsicForceWakeup() is used whenever Twakeup timer (set via AsicSleepThenAutoWakeup)
		expired.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
VOID AsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN    bFromTx)
{
    DBGPRINT(RT_DEBUG_INFO, ("--> AsicForceWakeup \n"));
    RTMP_STA_FORCE_WAKEUP(pAd, bFromTx);	
}
#endif // CONFIG_STA_SUPPORT //


/*
	==========================================================================
	Description:
		Set My BSSID

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicSetBssid(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pBssid) 
{
	ULONG		  Addr4;
	DBGPRINT(RT_DEBUG_TRACE, ("==============> AsicSetBssid %x:%x:%x:%x:%x:%x\n",
		pBssid[0],pBssid[1],pBssid[2],pBssid[3], pBssid[4],pBssid[5]));
	
	Addr4 = (ULONG)(pBssid[0])		 | 
			(ULONG)(pBssid[1] << 8)  | 
			(ULONG)(pBssid[2] << 16) |
			(ULONG)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

	Addr4 = 0;
	// always one BSSID in STA mode
	Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);

	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);
}

VOID AsicSetMcastWC(
	IN PRTMP_ADAPTER pAd) 
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];
	USHORT		offset;
	
	pEntry->Sst        = SST_ASSOC;
	pEntry->Aid        = MCAST_WCID;	// Softap supports 1 BSSID and use WCID=0 as multicast Wcid index
	pEntry->PsMode     = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate; 
	offset = MAC_WCID_BASE + BSS0Mcast_WCID * HW_WCID_ENTRY_SIZE;
}

/*
	==========================================================================
	Description:   

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicDelWcidTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	Wcid) 
{
	ULONG		  Addr0 = 0x0, Addr1 = 0x0;
	ULONG 		offset;

	DBGPRINT(RT_DEBUG_TRACE, ("AsicDelWcidTab==>Wcid = 0x%x\n",Wcid));
	offset = MAC_WCID_BASE + Wcid * HW_WCID_ENTRY_SIZE;
	RTMP_IO_WRITE32(pAd, offset, Addr0);
	offset += 4;
	RTMP_IO_WRITE32(pAd, offset, Addr1);
}

#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableRDG(
	IN PRTMP_ADAPTER pAd) 
{
	TX_LINK_CFG_STRUC	TxLinkCfg;
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn = 1;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	Data  &= 0xFFFFFF00;
	Data  |= 0x80;
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

	//OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDisableRDG(
	IN PRTMP_ADAPTER pAd) 
{
	TX_LINK_CFG_STRUC	TxLinkCfg;
	UINT32				Data = 0;


	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn = 0;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	
	Data  &= 0xFFFFFF00;
	//Data  |= 0x20;
#ifndef WIFI_TEST
	//if ( pAd->CommonCfg.bEnableTxBurst )	
	//	Data |= 0x60; // for performance issue not set the TXOP to 0
#endif
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
#endif // DOT11_N_SUPPORT //
	)
	{
		// For CWC test, change txop from 0x30 to 0x20 in TxBurst mode
		if (pAd->CommonCfg.bEnableTxBurst)
		Data |= 0x20;
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);
}
#endif // DOT11_N_SUPPORT //

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDisableSync(
	IN PRTMP_ADAPTER pAd) 
{
	BCN_TIME_CFG_STRUC csr;
	
	DBGPRINT(RT_DEBUG_TRACE, ("--->Disable TSF synchronization\n"));

	// 2003-12-20 disable TSF and TBTT while NIC in power-saving have side effect
	//			  that NIC will never wakes up because TSF stops and no more 
	//			  TBTT interrupts
	pAd->TbttTickCount = 0;
	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
	csr.field.bBeaconGen = 0;
	csr.field.bTBTTEnable = 0;
	csr.field.TsfSyncMode = 0;
	csr.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);

}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableBssSync(
	IN PRTMP_ADAPTER pAd) 
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
//	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, 0x00000000);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; // ASIC register in units of 1/16 TU
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 3; // sync TSF similar as in ADHOC mode?
		csr.field.bBeaconGen  = 1; // AP should generate BEACON
		csr.field.bTBTTEnable = 1;
	}
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; // ASIC register in units of 1/16 TU
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 1; // sync TSF in INFRASTRUCTURE mode
		csr.field.bBeaconGen  = 0; // do NOT generate BEACON
		csr.field.bTBTTEnable = 1;
	}
#endif // CONFIG_STA_SUPPORT //	
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}

/*
	==========================================================================
	Description:
	Note: 
		BEACON frame in shared memory should be built ok before this routine
		can be called. Otherwise, a garbage frame maybe transmitted out every
		Beacon period.

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableIbssSync(
	IN PRTMP_ADAPTER pAd)
{
	BCN_TIME_CFG_STRUC csr9;
	PUCHAR			ptr;
	UINT i;
	ULONG beaconBaseLocation = 0;
#ifdef DBG
	USHORT			beaconLen = pAd->BeaconTxWI.MPDUtotalByteCount;
#endif
#ifdef SPECIFIC_BCN_BUF_SUPPORT	
	unsigned long irqFlag;
#endif // SPECIFIC_BCN_BUF_SUPPORT //

#ifdef RT_BIG_ENDIAN
	TXWI_STRUC		localTxWI;
	
	NdisMoveMemory((PUCHAR)&localTxWI, (PUCHAR)&pAd->BeaconTxWI, TXWI_SIZE);
	RTMPWIEndianChange((PUCHAR)&localTxWI, TYPE_TXWI);
	beaconLen = localTxWI.MPDUtotalByteCount;
#endif // RT_BIG_ENDIAN //

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(MPDUtotalByteCount=%d, beaconLen=%d)\n", pAd->BeaconTxWI.MPDUtotalByteCount, beaconLen));


	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(ADHOC mode. MPDUtotalByteCount = %d)\n", pAd->BeaconTxWI.MPDUtotalByteCount));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr9.word);
	csr9.field.bBeaconGen = 0;
	csr9.field.bTBTTEnable = 0;
	csr9.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);

	{
		beaconBaseLocation = HW_BEACON_BASE0;
	}

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_MAC_SHR_MSEL_LOCK(pAd, HIGHER_SHRMEM, irqFlag);
#endif // SPECIFIC_BCN_BUF_SUPPORT //

#ifdef RTMP_MAC_PCI
	// move BEACON TXD and frame content to on-chip memory
	ptr = (PUCHAR)&pAd->BeaconTxWI;
	for (i=0; i<TXWI_SIZE; i+=4)  // 16-byte TXWI field
	{
		UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_IO_WRITE32(pAd, HW_BEACON_BASE0 + i, longptr);
		ptr += 4;
	}

	// start right after the 16-byte TXWI field
	ptr = pAd->BeaconBuf;
	for (i=0; i< pAd->BeaconTxWI.MPDUtotalByteCount; i+=4)
	{
		UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_IO_WRITE32(pAd, HW_BEACON_BASE0 + TXWI_SIZE + i, longptr);
		ptr +=4;
	}
#endif // RTMP_MAC_PCI //


	{
		// do nothing
	}

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_MAC_SHR_MSEL_UNLOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif // SPECIFIC_BCN_BUF_SUPPORT //

	//
	// For Wi-Fi faily generated beacons between participating stations. 
	// Set TBTT phase adaptive adjustment step to 8us (default 16us)
	// don't change settings 2006-5- by Jerry
	//RTMP_IO_WRITE32(pAd, TBTT_SYNC_CFG, 0x00001010);
	
	// start sending BEACON
	csr9.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; // ASIC register in units of 1/16 TU
	csr9.field.bTsfTicking = 1;
	csr9.field.TsfSyncMode = 2; // sync TSF in IBSS mode
	csr9.field.bTBTTEnable = 1;
	csr9.field.bBeaconGen = 1;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicSetEdcaParm(
	IN PRTMP_ADAPTER pAd,
	IN PEDCA_PARM	 pEdcaParm)
{
	EDCA_AC_CFG_STRUC   Ac0Cfg, Ac1Cfg, Ac2Cfg, Ac3Cfg;
	AC_TXOP_CSR0_STRUC csr0;
	AC_TXOP_CSR1_STRUC csr1;
	AIFSN_CSR_STRUC    AifsnCsr;
	CWMIN_CSR_STRUC    CwminCsr;
	CWMAX_CSR_STRUC    CwmaxCsr;
	int i;

	Ac0Cfg.word = 0;
	Ac1Cfg.word = 0;
	Ac2Cfg.word = 0;
	Ac3Cfg.word = 0;
	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("AsicSetEdcaParm\n"));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) || IS_ENTRY_APCLI(&pAd->MacTab.Content[i]))
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		//========================================================
		//      MAC Register has a copy .
		//========================================================
//#ifndef WIFI_TEST
		if( pAd->CommonCfg.bEnableTxBurst )		
		{
			// For CWC test, change txop from 0x30 to 0x20 in TxBurst mode
			Ac0Cfg.field.AcTxop = 0x20; // Suggest by John for TxBurst in HT Mode
		}
		else
			Ac0Cfg.field.AcTxop = 0;	// QID_AC_BE
//#else
//		Ac0Cfg.field.AcTxop = 0;	// QID_AC_BE
//#endif					
		Ac0Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac0Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac0Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);

		Ac1Cfg.field.AcTxop = 0;	// QID_AC_BK
		Ac1Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac1Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac1Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);

		if (pAd->CommonCfg.PhyMode == PHY_11B)
		{
			Ac2Cfg.field.AcTxop = 192;	// AC_VI: 192*32us ~= 6ms
			Ac3Cfg.field.AcTxop = 96;	// AC_VO: 96*32us  ~= 3ms
		}
		else
		{
			Ac2Cfg.field.AcTxop = 96;	// AC_VI: 96*32us ~= 3ms
			Ac3Cfg.field.AcTxop = 48;	// AC_VO: 48*32us ~= 1.5ms
		}
		Ac2Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac2Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac2Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		Ac3Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac3Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac3Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);

		//========================================================
		//      DMA Register has a copy too.
		//========================================================
		csr0.field.Ac0Txop = 0;		// QID_AC_BE
		csr0.field.Ac1Txop = 0;		// QID_AC_BK
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
		if (pAd->CommonCfg.PhyMode == PHY_11B)
		{
			csr1.field.Ac2Txop = 192;		// AC_VI: 192*32us ~= 6ms
			csr1.field.Ac3Txop = 96;		// AC_VO: 96*32us  ~= 3ms
		}
		else
		{
			csr1.field.Ac2Txop = 96;		// AC_VI: 96*32us ~= 3ms
			csr1.field.Ac3Txop = 48;		// AC_VO: 48*32us ~= 1.5ms
		}
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin1 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin2 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin3 = CW_MIN_IN_BITS;
		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax1 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax2 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax3 = CW_MAX_IN_BITS;
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, 0x00002222);

		NdisZeroMemory(&pAd->CommonCfg.APEdcaParm, sizeof(EDCA_PARM));

#ifdef WMM_ACM_SUPPORT
		/* no ACM function due to no WMM */
		ACMP_EnableFlagReset(pAd, 0, 0, 0, 0);
#endif // WMM_ACM_SUPPORT //
	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		//========================================================
		//      MAC Register has a copy.
		//========================================================
		//
		// Modify Cwmin/Cwmax/Txop on queue[QID_AC_VI], Recommend by Jerry 2005/07/27
		// To degrade our VIDO Queue's throughput for WiFi WMM S3T07 Issue.
		//
		//pEdcaParm->Txop[QID_AC_VI] = pEdcaParm->Txop[QID_AC_VI] * 7 / 10; // rt2860c need this		

		Ac0Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BE];
		Ac0Cfg.field.Cwmin= pEdcaParm->Cwmin[QID_AC_BE];
		Ac0Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BE];
		Ac0Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BE]; //+1;

		Ac1Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BK];
		Ac1Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_BK]; //+2; 
		Ac1Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BK];
		Ac1Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BK]; //+1;


		Ac2Cfg.field.AcTxop = (pEdcaParm->Txop[QID_AC_VI] * 6) / 10;
		if(pAd->Antenna.field.TxPath == 1)
		{
			Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI] + 1;
			Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI] + 1;			
		}
		else
		{
		Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI];
		Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI];
		}
		Ac2Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VI] + 1;
#ifdef CONFIG_STA_SUPPORT
#endif // CONFIG_STA_SUPPORT //
		
#ifdef INF_AMAZON_SE
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			Ac2Cfg.field.Aifsn = 0x3; //for WiFi WMM A1-T07.
#endif // CONFIG_AP_SUPPORT //
#endif // INF_AMAZON_SE //

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			// Tuning for Wi-Fi WMM S06
			if (pAd->CommonCfg.bWiFiTest && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				Ac2Cfg.field.Aifsn -= 1; 

			// Tuning for TGn Wi-Fi 5.2.32
			// STA TestBed changes in this item: conexant legacy sta ==> broadcom 11n sta
			if (STA_TGN_WIFI_ON(pAd) && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				Ac0Cfg.field.Aifsn = 3;
				Ac2Cfg.field.AcTxop = 5;
			}
			
		}
#endif // CONFIG_STA_SUPPORT //

		Ac3Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VO];
		Ac3Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VO];
		Ac3Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VO];
		Ac3Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VO];

//#ifdef WIFI_TEST
		if (pAd->CommonCfg.bWiFiTest)
		{
			if (Ac3Cfg.field.AcTxop == 102)
			{
			Ac0Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BE] ? pEdcaParm->Txop[QID_AC_BE] : 10;
				Ac0Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BE]-1; /* AIFSN must >= 1 */
			Ac1Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BK];
				Ac1Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BK];
			Ac2Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VI];
			} /* End of if */
		}
//#endif // WIFI_TEST //

		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);


		//========================================================
		//      DMA Register has a copy too.
		//========================================================
		csr0.field.Ac0Txop = Ac0Cfg.field.AcTxop;
		csr0.field.Ac1Txop = Ac1Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);

		csr1.field.Ac2Txop = Ac2Cfg.field.AcTxop;
		csr1.field.Ac3Txop = Ac3Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pEdcaParm->Cwmin[QID_AC_BE];
		CwminCsr.field.Cwmin1 = pEdcaParm->Cwmin[QID_AC_BK];
		CwminCsr.field.Cwmin2 = pEdcaParm->Cwmin[QID_AC_VI];
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO];
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO] - 1; //for TGn wifi test
#endif // CONFIG_STA_SUPPORT //
		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pEdcaParm->Cwmax[QID_AC_BE];
		CwmaxCsr.field.Cwmax1 = pEdcaParm->Cwmax[QID_AC_BK];
		CwmaxCsr.field.Cwmax2 = pEdcaParm->Cwmax[QID_AC_VI];
		CwmaxCsr.field.Cwmax3 = pEdcaParm->Cwmax[QID_AC_VO];
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		AifsnCsr.word = 0;
		AifsnCsr.field.Aifsn0 = Ac0Cfg.field.Aifsn; //pEdcaParm->Aifsn[QID_AC_BE];
		AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn; //pEdcaParm->Aifsn[QID_AC_BK];
#ifdef CONFIG_STA_SUPPORT
#endif // CONFIG_STA_SUPPORT //
		AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn; //pEdcaParm->Aifsn[QID_AC_VI];
#ifdef INF_AMAZON_SE
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; //pEdcaParm->Aifsn[QID_AC_VO]
			AifsnCsr.field.Aifsn2 = 0x2; //pEdcaParm->Aifsn[QID_AC_VI]; //for WiFi WMM A1-T07.
		}
#endif // CONFIG_AP_SUPPORT //
#endif // INF_AMAZON_SE //

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			// Tuning for Wi-Fi WMM S06
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn - 4;

			// Tuning for TGn Wi-Fi 5.2.32
			// STA TestBed changes in this item: connexant legacy sta ==> broadcom 11n sta
			if (STA_TGN_WIFI_ON(pAd) && 
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				AifsnCsr.field.Aifsn0 = 3;
				AifsnCsr.field.Aifsn2 = 7;
			}

			if (INFRA_ON(pAd))
				CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_WMM_CAPABLE);
		}
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn; //pEdcaParm->Aifsn[QID_AC_VO]
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn - 1; //pEdcaParm->Aifsn[QID_AC_VO]; //for TGn wifi test
		}
#endif // CONFIG_STA_SUPPORT //
		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, AifsnCsr.word);

		NdisMoveMemory(&pAd->CommonCfg.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
		if (!ADHOC_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM\n", pEdcaParm->EdcaUpdateCount));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[0],
									 pEdcaParm->Cwmin[0],
									 pEdcaParm->Cwmax[0],
									 pEdcaParm->Txop[0]<<5,
									 pEdcaParm->bACM[0]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[1],
									 pEdcaParm->Cwmin[1],
									 pEdcaParm->Cwmax[1],
									 pEdcaParm->Txop[1]<<5,
									 pEdcaParm->bACM[1]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[2],
									 pEdcaParm->Cwmin[2],
									 pEdcaParm->Cwmax[2],
									 pEdcaParm->Txop[2]<<5,
									 pEdcaParm->bACM[2]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[3],
									 pEdcaParm->Cwmin[3],
									 pEdcaParm->Cwmax[3],
									 pEdcaParm->Txop[3]<<5,
									 pEdcaParm->bACM[3]));
		}

#ifdef WMM_ACM_SUPPORT
		ACMP_EnableFlagReset(pAd, pEdcaParm->bACM[0],
							pEdcaParm->bACM[1],
							pEdcaParm->bACM[2],
							pEdcaParm->bACM[3]);
#endif // WMM_ACM_SUPPORT //
	}

	pAd->CommonCfg.RestoreBurstMode = Ac0Cfg.word;
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID 	AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime) 
{
	ULONG	SlotTime;
	UINT32	RegValue = 0;

#ifdef CONFIG_STA_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
		bUseShortSlotTime = TRUE;
#endif // CONFIG_STA_SUPPORT //

	if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
		return;
	else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
		return;

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime)? 9 : 20;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		// force using short SLOT time for FAE to demo performance when TxBurst is ON
		if (((pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
#ifdef DOT11_N_SUPPORT
			|| ((pAd->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))
#endif // DOT11_N_SUPPORT //
			)
		{
			// In this case, we will think it is doing Wi-Fi test
			// And we will not set to short slot when bEnableTxBurst is TRUE.
		}
		else if (pAd->CommonCfg.bEnableTxBurst)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 9;
		}
	}
#endif // CONFIG_STA_SUPPORT //

	//
	// For some reasons, always set it to short slot time.
	// 
	// ToDo: Should consider capability with 11B
	//
#ifdef CONFIG_STA_SUPPORT 
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->StaCfg.BssType == BSS_ADHOC)	
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
			SlotTime = 20;
		}
	}
#endif // CONFIG_STA_SUPPORT //

	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	RegValue = RegValue & 0xFFFFFF00;

	RegValue |= SlotTime;

	RTMP_IO_WRITE32(pAd, BKOFF_SLOT_CFG, RegValue);
}

/*
	========================================================================
	Description:
		Add Shared key information into ASIC. 
		Update shared key, TxMic and RxMic to Asic Shared key table
		Update its cipherAlg to Asic Shared key Mode.
		
    Return:
	========================================================================
*/
VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	ULONG offset; //, csr0;
	SHAREDKEY_MODE_STRUC csr1;
#ifdef RTMP_MAC_PCI
	INT   i;
#endif // RTMP_MAC_PCI //

	PUCHAR		pKey = pCipherKey->Key;
	PUCHAR		pTxMic = pCipherKey->TxMic;
	PUCHAR		pRxMic = pCipherKey->RxMic;
	UCHAR		CipherAlg = pCipherKey->CipherAlg;

	DBGPRINT(RT_DEBUG_TRACE, ("AsicAddSharedKeyEntry BssIndex=%d, KeyIdx=%d\n", BssIndex,KeyIdx));
//============================================================================================

	DBGPRINT(RT_DEBUG_TRACE,("AsicAddSharedKeyEntry: %s key #%d\n", CipherName[CipherAlg], BssIndex*4 + KeyIdx));
	DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
//============================================================================================
	//
	// fill key material - key + TX MIC + RX MIC
	//
#ifdef RTMP_MAC_PCI
	offset = SHARED_KEY_TABLE_BASE + (4*BssIndex + KeyIdx)*HW_KEY_ENTRY_SIZE;
	for (i=0; i<MAX_LEN_OF_SHARE_KEY; i++) 
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}

	offset += MAX_LEN_OF_SHARE_KEY;
	if (pTxMic)
	{
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pTxMic[i]);
		}
	}

	offset += 8;
	if (pRxMic)
	{
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pRxMic[i]);
		}
	}
#endif // RTMP_MAC_PCI //


	//
	// Update cipher algorithm. WSTA always use BSS0
	//
	RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), &csr1.word);
	DBGPRINT(RT_DEBUG_TRACE,("Read: SHARED_KEY_MODE_BASE at this Bss[%d] KeyIdx[%d]= 0x%x \n", BssIndex,KeyIdx, csr1.word));
	if ((BssIndex%2) == 0)
	{
		if (KeyIdx == 0)
			csr1.field.Bss0Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss0Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss0Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss0Key3CipherAlg = CipherAlg;
	}
	else
	{
		if (KeyIdx == 0)
			csr1.field.Bss1Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss1Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss1Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss1Key3CipherAlg = CipherAlg;
	}
	DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
	RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), csr1.word);
		
}

//	IRQL = DISPATCH_LEVEL
VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	//ULONG SecCsr0;
	SHAREDKEY_MODE_STRUC csr1;

	DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyEntry: #%d \n", BssIndex*4 + KeyIdx));

	RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), &csr1.word);
	if ((BssIndex%2) == 0)
	{
		if (KeyIdx == 0)
			csr1.field.Bss0Key0CipherAlg = 0;
		else if (KeyIdx == 1)
			csr1.field.Bss0Key1CipherAlg = 0;
		else if (KeyIdx == 2)
			csr1.field.Bss0Key2CipherAlg = 0;
		else
			csr1.field.Bss0Key3CipherAlg = 0;
	}
	else
	{
		if (KeyIdx == 0)
			csr1.field.Bss1Key0CipherAlg = 0;
		else if (KeyIdx == 1)
			csr1.field.Bss1Key1CipherAlg = 0;
		else if (KeyIdx == 2)
			csr1.field.Bss1Key2CipherAlg = 0;
		else
			csr1.field.Bss1Key3CipherAlg = 0;
	}
	DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
	RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), csr1.word);
	ASSERT(BssIndex < 4);
	ASSERT(KeyIdx < 4);

}

VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
	ULONG	offset;

	offset = MAC_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);

	RTMP_IO_WRITE32(pAd, offset, uIV);
	RTMP_IO_WRITE32(pAd, offset + 4, uEIV);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: wcid(%d) 0x%08lx, 0x%08lx \n", 
									__FUNCTION__, WCID, uIV, uEIV));	
}

VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN PUCHAR        pAddr)
{
	ULONG offset;
	ULONG Addr;
	
	offset = MAC_WCID_BASE + (WCID * HW_WCID_ENTRY_SIZE);	
	Addr = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = pAddr[4] + (pAddr[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);	
}
	
/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:

    Note :
		The key table selection rule :
    	1.	Wds-links and Mesh-links always use Pair-wise key table. 	
		2. 	When the CipherAlg is TKIP, AES, SMS4 or the dynamic WEP is enabled, 
			it needs to set key into Pair-wise Key Table.
		3.	The pair-wise key security mode is set NONE, it means as no security.
		4.	In STA Adhoc mode, it always use shared key table.
		5.	Otherwise, use shared key table

	========================================================================
*/
VOID	AsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag)
{
	WCID_ATTRIBUTE_STRUC WCIDAttri;	
	USHORT		offset;

	/* Initialize the content of WCID Attribue  */
	WCIDAttri.word = 0;

	/* The limitation of HW WCID table */
	if (/*Wcid < 1 ||*/ Wcid > 254)
	{		
		DBGPRINT(RT_DEBUG_WARN, ("%s: Wcid is invalid (%d). \n", 
										__FUNCTION__, Wcid));	
		return;
	}

	/* Update the pairwise key security mode.
	   Use bit10 and bit3~1 to indicate the pairwise cipher mode */	
	WCIDAttri.field.PairKeyModeExt = ((CipherAlg & 0x08) >> 3);
	WCIDAttri.field.PairKeyMode = (CipherAlg & 0x07);

	/* Update the MBSS index.
	   Use bit11 and bit6~4 to indicate the BSS index */	
	WCIDAttri.field.BSSIdxExt = ((BssIdx & 0x08) >> 3);
	WCIDAttri.field.BSSIdx = (BssIdx & 0x07);

#ifdef WAPI_SUPPORT
	/* Update WAPI related information */
	if (CipherAlg == CIPHER_SMS4)
	{
		if (KeyTabFlag == SHAREDKEYTABLE)
			WCIDAttri.field.WAPI_MCBC = 1;
		WCIDAttri.field.WAPIKeyIdx = ((KeyIdx == 0) ? 0 : 1); 
	}
#endif // WAPI_SUPPORT //
	
	/* Assign Key Table selection */		
	WCIDAttri.field.KeyTab = KeyTabFlag;

	/* Update related information to ASIC */
	offset = MAC_WCID_ATTRIBUTE_BASE + (Wcid * HW_WCID_ATTRI_SIZE);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri.word);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : WCID #%d, KeyIndex #%d, Alg=%s\n", __FUNCTION__, Wcid, KeyIdx, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE, ("		WCIDAttri = 0x%x \n", WCIDAttri.word));	
	
}
	

/*
	========================================================================
	Description:
		Add Pair-wise key material into ASIC. 
		Update pairwise key, TxMic and RxMic to Asic Pair-wise key table
				
    Return:
	========================================================================
*/
VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey)
{
	INT i;
	ULONG 		offset;
	PUCHAR		 pKey = pCipherKey->Key;
	PUCHAR		 pTxMic = pCipherKey->TxMic;
	PUCHAR		 pRxMic = pCipherKey->RxMic;
#ifdef DBG
	UCHAR		CipherAlg = pCipherKey->CipherAlg;
#endif
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	unsigned long irqFlag;
#endif // SPECIFIC_BCN_BUF_SUPPORT //

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_MAC_SHR_MSEL_LOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif // SPECIFIC_BCN_BUF_SUPPORT //
	
	// EKEY
	offset = PAIRWISE_KEY_TABLE_BASE + (WCID * HW_KEY_ENTRY_SIZE);
#ifdef RTMP_MAC_PCI
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i++)
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}
#endif // RTMP_MAC_PCI //
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i+=4)
	{
		UINT32 Value;
		RTMP_IO_READ32(pAd, offset + i, &Value);
	}

	offset += MAX_LEN_OF_PEER_KEY;
	
	//  MIC KEY
	if (pTxMic)
	{
#ifdef RTMP_MAC_PCI
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pTxMic[i]);
		}
#endif // RTMP_MAC_PCI //
	}
	offset += 8;
	if (pRxMic)
	{
#ifdef RTMP_MAC_PCI
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pRxMic[i]);
		}
#endif // RTMP_MAC_PCI //
	}

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_MAC_SHR_MSEL_UNLOCK(pAd, LOWER_SHRMEM, irqFlag);
#endif // SPECIFIC_BCN_BUF_SUPPORT

	DBGPRINT(RT_DEBUG_TRACE,("AsicAddPairwiseKeyEntry: WCID #%d Alg=%s\n",WCID, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE,("	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
}


/*
	========================================================================
	Description:
		Remove Pair-wise key material from ASIC. 

    Return:
	========================================================================
*/	
VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Wcid)
{
	/* Set the specific WCID attribute entry as OPEN-NONE */
	AsicUpdateWcidAttributeEntry(pAd, 
							  BSS0,
							  0,
							  CIPHER_NONE, 
							  Wcid,
							  PAIRWISEKEYTABLE);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : Wcid #%d \n", __FUNCTION__, Wcid));
}

BOOLEAN AsicSendCommandToMcu(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1)
{
	if (pAd->chipOps.sendCommandToMcu)
		return pAd->chipOps.sendCommandToMcu(pAd, Command, Token, Arg0, Arg1);
	else
		return FALSE;
}


VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
}


VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER pAd, 
	IN	UCHAR		Channel) 
{
	if (pAd->chipOps.AsicRfTurnOff)
	{
		pAd->chipOps.AsicRfTurnOff(pAd);
	}
	else
	{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
		// RF R2 bit 18 = 0
		UINT32			R1 = 0, R2 = 0, R3 = 0;
		UCHAR			index;
		RTMP_RF_REGS	*RFRegTable;
	
		RFRegTable = RF2850RegTable;
#endif // defined(RT28xx) || defined(RT2880) || defined(RT2883) //

		switch (pAd->RfIcType)
		{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
#if defined(RT28xx) || defined(RT2880)
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:
#endif // defined(RT28xx) || defined(RT2880) //
#ifdef RT2883
			case RFIC_2853:
#endif // RT2883 //
				for (index = 0; index < NUM_OF_2850_CHNL; index++)
				{
					if (Channel == RFRegTable[index].Channel)
					{
						R1 = RFRegTable[index].R1 & 0xffffdfff;
						R2 = RFRegTable[index].R2 & 0xfffbffff;
						R3 = RFRegTable[index].R3 & 0xfff3ffff;

						RTMP_RF_IO_WRITE32(pAd, R1);
						RTMP_RF_IO_WRITE32(pAd, R2);

						// Program R1b13 to 1, R3/b18,19 to 0, R2b18 to 0. 
						// Set RF R2 bit18=0, R3 bit[18:19]=0
						//if (pAd->StaCfg.bRadio == FALSE)
						if (1)
						{
							RTMP_RF_IO_WRITE32(pAd, R3);

							DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x,  R3 = 0x%08x \n",
								Channel, pAd->RfIcType, R2, R3));
						}
						else
							DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x \n",
								Channel, pAd->RfIcType, R2));
						break;
					}
				}
				break;
#endif // defined(RT28xx) || defined(RT2880) || defined(RT2883) //
			default:
				DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d : Unkonwn RFIC=%d\n",
											Channel, pAd->RfIcType));
				break;
		}
	}
}


#ifdef WAPI_SUPPORT
VOID AsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high)
{
	if (IS_HW_WAPI_SUPPORT(pAd))
	{
		ULONG	offset;

		offset = WAPI_PN_TABLE_BASE + (WCID * WAPI_PN_ENTRY_SIZE);

		RTMP_IO_WRITE32(pAd, offset, pn_low);
		RTMP_IO_WRITE32(pAd, offset + 4, pn_high);
	}
	else
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : Not support HW_WAPI_PN_TABLE\n", 
								__FUNCTION__));
	}
	
}
#endif // WAPI_SUPPORT //



#ifdef RT3883
VOID AsicVCORecalibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BbpR49 = 0, Tssi = 0, RFValue = 0;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif
	if (pAd->EnableVCOReCal != TRUE)
		return;

	if (pAd->RefreshTssi == 0)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);
		Tssi = BbpR49 >> 1; // bit 0 is used for update flag
		
		if (abs((pAd->LatchTssi) - Tssi) >= pAd->CommonCfg.VCORecalibrationThreshold)
		{
			RT30xxReadRFRegister(pAd, RF_R03, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x80; // bit 7=vcocal_en
			RT30xxWriteRFRegister(pAd, RF_R03, (UCHAR)RFValue);
			
			RTMPusecDelay(2000);
			
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpR49 & 0xfe); // clear update flag
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);
			pAd->RefreshTssi = 1;
			
			DBGPRINT(RT_DEBUG_TRACE, ("AsicVCORecalibration: vcocal_en=1, TSSI difference=%ld\n", abs((pAd->LatchTssi) - Tssi)));
		}
	}

#ifdef TXBF_SUPPORT
	// Do a Divider Calibration and update BBP registers
	if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn &&
		(pAd->CommonCfg.DebugFlags & DBF_DISABLE_CAL)==0)
	{
		ITxBFDividerCalibration(pAd, 2, 0, NULL);
	}

	if (pAd->CommonCfg.ETxBfEnCond)
	{
		UCHAR	idx;
		for (idx = 1; idx < MAX_LEN_OF_MAC_TABLE; idx++)
		{
			MAC_TABLE_ENTRY		*pEntry;
				pEntry = &pAd->MacTab.Content[idx];
			if ((IS_ENTRY_CLIENT(pEntry)) && (pEntry->eTxBfEnCond))
			{
				BOOLEAN Cancelled;
					RTMPCancelTimer(&pEntry->eTxBfProbeTimer, &Cancelled);
					pEntry->bfState = READY_FOR_SNDG0;
				eTxBFProbing(pAd, pEntry);
			}
		}
	}
#endif // TXBF_SUPPORT //
}
#endif // RT3883 //


#ifdef STREAM_MODE_SUPPORT
// StreamModeRegVal - return MAC reg value for StreamMode setting
ULONG StreamModeRegVal(
	IN RTMP_ADAPTER *pAd)
{
	ULONG streamWord;

	switch (pAd->CommonCfg.StreamMode)
	{
		case 1:
			streamWord = 0x030000;
			break;
		case 2:
			streamWord = 0x0c0000;
			break;
		case 3:
			streamWord = 0x0f0000;
			break;
		default:
			streamWord = 0x0;
			break;
	}

	return streamWord;
}

/*
	========================================================================
	Description:
		configure the stream mode of specific MAC or all MAC and set to ASIC. 

	Prameters:
		pAd		 --- 
		pMacAddr ---
		bClear	 --- disable the stream mode for specific macAddr when
						(pMacAddr!=NULL)
	
    Return:
	========================================================================
*/
VOID AsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN BOOLEAN bEnableSM)
{
	// TODO: not finish yet.
}
#endif // STREAM_MODE_SUPPORT //

#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicEnableRalinkBurstMode(
	IN PRTMP_ADAPTER pAd) 
{
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	pAd->CommonCfg.RestoreBurstMode = Data;
	Data  &= 0xFFF00000;
	Data  |= 0x86380;
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID AsicDisableRalinkBurstMode(
	IN PRTMP_ADAPTER pAd) 
{
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);

	Data = pAd->CommonCfg.RestoreBurstMode;
	Data &= 0xFFFFFF00;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
#endif // DOT11_N_SUPPORT //
	)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
			Data |= 0x80;
		else if (pAd->CommonCfg.bEnableTxBurst)
			Data |= 0x20;
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);
}
#endif // DOT11_N_SUPPORT //

