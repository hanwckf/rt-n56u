/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt5592.c

	Abstract:
	Specific funcitons and variables for RT5572/RT5592

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */



#ifdef RTMP_MAC_PCI
/* RF for A/G band */
const REG_PAIR RF5592Reg_2G_5G[] =
{
	{RF_R01, 0x3F},
	{RF_R02, 0x80},
	{RF_R03, 0x08},
	{RF_R05, 0x10},
	{RF_R06, 0xE0},
	{RF_R07, 0x00},
	{RF_R14, 0x00},
	{RF_R15, 0x00},
	{RF_R16, 0x00},
	{RF_R18, 0x03},
	{RF_R19, 0x4D},
	{RF_R20, 0x10},
	{RF_R21, 0x8D},
	{RF_R26, 0x82},
	{RF_R28, 0x00},
	{RF_R29, 0x10},
	{RF_R33, 0xC0},
	{RF_R34, 0x07},
	{RF_R35, 0x12},
	{RF_R47, 0x0C},
	{RF_R53, 0x44},
	{RF_R63, 0x07},
};

/* RF for G Band */
const REG_PAIR RF5592Reg_2G[] =
{
	{RF_R10, 0x90},
	{RF_R11, 0x4A},
	{RF_R12, 0x52},
	{RF_R13, 0x42},
	{RF_R22, 0x40},
	{RF_R24, 0x4A},
	{RF_R25, 0x80},
	{RF_R27, 0x42},
	{RF_R36, 0x80},
	{RF_R37, 0x08},
	{RF_R38, 0x89},
	{RF_R39, 0x1B},
	{RF_R40, 0x0D},
	{RF_R41, 0x9B},
	{RF_R42, 0xD5},
	{RF_R43, 0x72},
	{RF_R44, 0x0E},
	{RF_R45, 0xA2},
	{RF_R46, 0x6B},
	{RF_R48, 0x10},
	{RF_R51, 0x3E},
	{RF_R52, 0x48},
	{RF_R54, 0x38}, /* 0x48 or 0x38 */
	{RF_R56, 0xA1},
	{RF_R57, 0x00},
	{RF_R58, 0x39},
	{RF_R60, 0x45},
	{RF_R61, 0x91},
	{RF_R62, 0x39},
};

/* RF for A Band */
const REG_PAIR RF5592Reg_5G_Default[] =
{
	{RF_R10, 0x95},
	{RF_R11, 0x40},
	{RF_R25, 0xBF},
	{RF_R27, 0x42},
	{RF_R36, 0x00},
	{RF_R37, 0x04},
	{RF_R38, 0x85},
	{RF_R40, 0x42},
	{RF_R41, 0xAB},
	{RF_R42, 0xD7},
	{RF_R45, 0x01},
	{RF_R48, 0x00},
	{RF_R57, 0x77},
	{RF_R58, 0x19},
	{RF_R60, 0x05},
	{RF_R61, 0x01},
	{RF_R62, 0x19},
};

const REG_PAIR *RF5592Reg_5G = RF5592Reg_5G_Default;
UCHAR NUM_RF5592REG_5G = (sizeof(RF5592Reg_5G_Default) / sizeof(REG_PAIR));

/* RF for G band per channel */
const REG_PAIR_CHANNEL RF5592Reg_Channel_2G_Default[] =
{
	{RF_R23, 1, 10, 0x08},
	{RF_R23, 11, 14, 0x07},
	{RF_R59, 1, 4, 0x06},
	{RF_R59, 5, 14, 0x04},
};

/* RF for A band per channel */
const REG_PAIR_CHANNEL RF5592Reg_Channel_5G_Default[] =
{
	{RF_R12, 36, 64, 0x2E},
	{RF_R12, 100, 165, 0x0E},
	{RF_R13, 36, 64, 0x22},
	{RF_R13, 100, 165, 0x42},
	{RF_R22, 36, 64, 0x60},
	{RF_R22, 100, 165, 0x40},
	{RF_R23, 36, 64, 0x7E},
	{RF_R23, 100, 138, 0x7C},
	{RF_R23, 140, 165, 0x78},
	{RF_R24, 36, 64, 0x07},
	{RF_R24, 100, 114, 0x02},
	{RF_R24, 116, 165, 0x03},
	{RF_R39, 36, 64, 0x1C},
	{RF_R39, 100, 138, 0x1A},
	{RF_R39, 140, 165, 0x18},
	{RF_R43, 36, 64, 0x5B},
	{RF_R43, 100, 138, 0x3B},
	{RF_R43, 140, 165, 0x1B},
	{RF_R44, 36, 50, 0x32},
	{RF_R44, 52, 64, 0x2A},
	{RF_R44, 100, 114, 0x1A},
	{RF_R44, 116, 165, 0x0A},
	{RF_R46, 36, 64, 0x00},
	{RF_R46, 100, 138, 0x18},
	{RF_R46, 140, 165, 0x08},
	{RF_R51, 36, 64, 0xFD},
	{RF_R51, 100, 124, 0xFC},
	{RF_R51, 126, 165, 0xEC},
	{RF_R52, 36, 64, 0x0E},
	{RF_R52, 100, 138, 0x06},
	{RF_R52, 140, 165, 0x06},
	{RF_R54, 36, 50, 0xF9},
	{RF_R54, 52, 64, 0xF8},
	{RF_R54, 100, 114, 0xEA},
	{RF_R54, 116, 165, 0xF9},
	{RF_R55, 36, 64, 0x04},
	{RF_R55, 100, 165, 0x01},
	{RF_R56, 36, 64, 0xBB},
	{RF_R56, 100, 114, 0xB3},
	{RF_R56, 116, 165, 0x9B},
	{RF_R59, 36, 64, 0x7C},
	{RF_R59, 100, 114, 0x7C},
	{RF_R59, 116, 138, 0x7E},
	{RF_R59, 140, 165, 0x7C},
};
const REG_PAIR_CHANNEL *RF5592Reg_Channel_5G = RF5592Reg_Channel_5G_Default;
UCHAR NUM_RF5592REG_CHANNEL_5G = (sizeof(RF5592Reg_Channel_5G_Default) / sizeof(REG_PAIR_CHANNEL));

#ifdef RT5592EP_SUPPORT
const REG_PAIR_CHANNEL RF5592Reg_Channel_5G_EP[] =
{
	{RF_R12, 36, 64, 0x2E},
	{RF_R12, 100, 165, 0x0E},
	{RF_R13, 36, 64, 0x22},
	{RF_R13, 100, 165, 0x42},
	{RF_R22, 36, 64, 0x60},
	{RF_R22, 100, 165, 0x40},
	{RF_R23, 36, 165, 0x11},
	{RF_R24, 36, 64, 0x07},
	{RF_R24, 100, 138, 0x06},
	{RF_R24, 140, 165, 0x04},
	{RF_R39, 36, 64, 0x1C},
	{RF_R39, 100, 138, 0x1A},
	{RF_R39, 140, 165, 0x18},
	{RF_R43, 36, 64, 0xF8},
	{RF_R43, 100, 165, 0x78},
	{RF_R44, 36, 50, 0x42},
	{RF_R44, 52, 64, 0x2A},
	{RF_R44, 100, 138, 0x22},
	{RF_R44, 140, 165, 0x12},
	{RF_R46, 36, 64, 0x00},
	{RF_R46, 100, 140, 0x18},
	{RF_R46, 149, 165, 0x08},
	{RF_R51, 36, 64, 0x8E},
	{RF_R51, 100, 124, 0x8C},
	{RF_R51, 126, 165, 0x8C},
	{RF_R52, 36, 64, 0x0E},
	{RF_R52, 100, 138, 0x06},
	{RF_R52, 140, 165, 0x06},
	{RF_R54, 36, 64, 0x88},
	{RF_R54, 100, 124, 0x8B},
	{RF_R54, 126, 165, 0x89},
	{RF_R55, 36, 64, 0x04},
	{RF_R55, 100, 165, 0x01},
	{RF_R56, 36, 165, 0xC3},
	{RF_R59, 36, 165, 0x11},
};

const REG_PAIR RF5592Reg_5G_EP[] =
{
	{RF_R10, 0x95},
	{RF_R11, 0x40},
	{RF_R25, 0xBF},
	{RF_R27, 0x42},
	{RF_R36, 0x00},
	{RF_R37, 0x00},
	{RF_R38, 0x85},
	{RF_R40, 0x42},
	{RF_R41, 0xAB},
	{RF_R42, 0xD7},
	{RF_R45, 0x00},
	{RF_R48, 0x00},
	{RF_R57, 0x11},
	{RF_R58, 0x11},
	{RF_R60, 0x05},
	{RF_R61, 0x01},
	{RF_R62, 0x11},
};

const REG_PAIR_CHANNEL RF5592Reg_Channel_2G_EP[] = {
	{RF_R23, 1, 14, 0x00},
	{RF_R24, 1, 14, 0x00},
	{RF_R51, 1, 14, 0x10},
	{RF_R54, 1, 14, 0x14},
	{RF_R56, 1, 14, 0x01},
	{RF_R58, 1, 14, 0x10},
	{RF_R59, 1, 14, 0x00},
	{RF_R60, 1, 14, 0xc5},
	{RF_R62, 1, 14, 0x10},
};
#endif /* RT5592EP_SUPPORT */
#endif /* RTMP_MAC_PCI */

/* RF for A/G band BW */
const REG_PAIR_BW RF5592Reg_BW_2G_5G[] =
{
	{RF_R30, BW_20, 0x10},
	{RF_R30, BW_40, 0x16}, 

};

/* RF for CCK */
const REG_PAIR RF5592Reg_CCK[] =
{
	{RF_R31, 0xF8},
	{RF_R32, 0xC0},
	{RF_R55, 0x47},
};

/* RF for A/G band OFDM */
const REG_PAIR RF5592Reg_OFDM_2G_5G[] =
{
	{RF_R31, 0x80},
	{RF_R32, 0x80},
};

/* RF for G band OFDM */
const REG_PAIR RF5592Reg_OFDM_2G[] =
{
	{RF_R55, 0x43},
};

UCHAR NUM_RF5592REG_2G_5G = (sizeof(RF5592Reg_2G_5G) / sizeof(REG_PAIR));
UCHAR NUM_RF5592REG_2G = (sizeof(RF5592Reg_2G) / sizeof(REG_PAIR));
#ifdef RTMP_MAC_PCI
const REG_PAIR_CHANNEL *RF5592Reg_Channel_2G = RF5592Reg_Channel_2G_Default;
UCHAR NUM_RF5592REG_CHANNEL_2G = (sizeof(RF5592Reg_Channel_2G_Default) / sizeof(REG_PAIR_CHANNEL));
#endif /* RTMP_MAC_PCI */
UCHAR NUM_RF5592REG_BW_2G_5G = (sizeof(RF5592Reg_BW_2G_5G) / sizeof(REG_PAIR_BW));
UCHAR NUM_RF5592REG_CCK	= (sizeof(RF5592Reg_CCK) / sizeof(REG_PAIR));
UCHAR NUM_RF5592REG_OFDM_2G_5G = (sizeof(RF5592Reg_OFDM_2G_5G) / sizeof(REG_PAIR));
UCHAR NUM_RF5592REG_OFDM_2G	= (sizeof(RF5592Reg_OFDM_2G) / sizeof(REG_PAIR));

/* BBP for A/G band */
const REG_PAIR BBP5592Reg_2G_5G[] = 
{
	{BBP_R20, 0x06}, /* for RT5592 CISCO IOT issue (20110629) */
	{BBP_R31, 0x08},
	{BBP_R65, 0x2C},
	{BBP_R68, 0xDD},
	{BBP_R69, 0x1A},
	{BBP_R70, 0x05},
	{BBP_R73, 0x13},
	{BBP_R74, 0x0F},
	{BBP_R75, 0x4F},
	{BBP_R76, 0x28},
	{BBP_R77, 0x59},
	{BBP_R84, 0x9A},
	{BBP_R86, 0x38},
	{BBP_R88, 0x90},
	{BBP_R91, 0x04},
	{BBP_R92, 0x02},
	{BBP_R98, 0x12},
	{BBP_R103, 0xC0},
	{BBP_R104, 0x92},
	{BBP_R105, 0x3C},
	{BBP_R106, 0x35}, /* for SGI peak throughput (20111129) */
	{BBP_R128, 0x12},
	{BBP_R134, 0xD0},
	{BBP_R135, 0xF6},
	{BBP_R137, 0x0F}, /* for RT5592 CISCO IOT issue(20110629) */
	{BBP_R148, 0x84}, /* to enhance RX angle sensitivity issue (20120726) */
};

/* BBP for G band */
const REG_PAIR BBP5592Reg_2G[] =
{
	{BBP_R79, 0x1C},
	{BBP_R80, 0x0E},
	{BBP_R81, 0x3A},
	{BBP_R82, 0x62},
	{BBP_R95, 0x9A},
};

/* BBP for A band */
const REG_PAIR BBP5592Reg_5G[] =
{
	{BBP_R79, 0x18},
	{BBP_R80, 0x08},
	{BBP_R81, 0x38},
	{BBP_R82, 0x92},
	{BBP_R95, 0x1A},
};

/* BBP for A/G band GLRT function(BBP_128 ~ BBP_221) */
const UCHAR BBP5592Reg_GLRT_2G_5G[] = 
{
	0xE0, 0x1F, 0X38, 0x32, 0x08, 0x28, 0x19, 0x0A, 0xFF, 0x00, /* 128 ~ 137 */
	0x16, 0x10, 0x10, 0x0B, 0x36, 0x2C, 0x26, 0x24, 0x42, 0x36, /* 138 ~ 147 */
	0x30, 0x2D, 0x4C, 0x46, 0x3D, 0x40, 0x3E, 0x42, 0x3D, 0x40, /* 148 ~ 157 */
	0X3C, 0x34, 0x2C, 0x2F, 0x3C, 0x35, 0x2E, 0x2A, 0x49, 0x41, /* 158 ~ 167 */
	0x36, 0x31, 0x30, 0x30, 0x0E, 0x0D, 0x28, 0x21, 0x1C, 0x16, /* 168 ~ 177 */
	0x50, 0x4A, 0x43, 0x40, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, /* 178 ~ 187 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 188 ~ 197 */
	0x00, 0x00, 0x7D, 0x14, 0x32, 0x2C, 0x36, 0x4C, 0x43, 0x2C, /* 198 ~ 207 */
	0x2E, 0x36, 0x30, 0x6E,							      /* 208 ~ */
};

/* BBP for G band GLRT function */
const REG_PAIR BBP5592Reg_GLRT_2G[] =
{
	{BBP_R128, 0xE0}, /* DLINK-825 low TP IOT issue (Disable GLRT-DCC)(20110629) */
	{BBP_R129, 0x1F},
	{BBP_R130, 0x38},
	{BBP_R131, 0x32},
	{BBP_R133, 0x28},
	{BBP_R134, 0x19},
};

/* BBP for A band GLRT function */
const REG_PAIR BBP5592Reg_GLRT_5G[] = 
{
	{BBP_R128, 0xF0}, /* DLINK-825 low TP IOT issue (Disable GLRT-DCC)(20110629) */
	{BBP_R129, 0x1E},
	{BBP_R130, 0x28},
	{BBP_R131, 0x20},
	{BBP_R133, 0x7F},
	{BBP_R134, 0x7F},
};

/* BBP for A/G band BW GLRT function */
const REG_PAIR_BW BBP5592Reg_GLRT_BW_2G_5G[] =
{
	{BBP_R141, BW_20, 0x1A},
	{BBP_R141, BW_40, 0x10}, 
};

UCHAR NUM_BBP5592REG_2G_5G = (sizeof(BBP5592Reg_2G_5G) / sizeof(REG_PAIR));
UCHAR NUM_BBP5592REG_2G	= (sizeof(BBP5592Reg_2G) / sizeof(REG_PAIR));
UCHAR NUM_BBP5592REG_5G = (sizeof(BBP5592Reg_5G) / sizeof(REG_PAIR));
UCHAR NUM_BBP5592REG_GLRT_2G_5G = (sizeof(BBP5592Reg_GLRT_2G_5G) / sizeof(UCHAR));
UCHAR NUM_BBP5592REG_GLRT_2G = (sizeof(BBP5592Reg_GLRT_2G) / sizeof(REG_PAIR));
UCHAR NUM_BBP5592REG_GLRT_5G = (sizeof(BBP5592Reg_GLRT_5G) / sizeof(REG_PAIR));
UCHAR NUM_BBP5592REG_GLRT_BW_2G_5G = (sizeof(BBP5592Reg_GLRT_BW_2G_5G) / sizeof(REG_PAIR_BW));

/* MAC for A/G band */
static const RTMP_REG_PAIR	MAC5592Reg[] =	{
	{TX_SW_CFG0,		0x0404},
	{MAX_LEN_CFG,		MAX_AGGREGATION_SIZE | 0x00001000},
	{HT_FBK_CFG1,		0xedcba980},	/* Fallback MCS8->MCS0 */
};

#define NUM_MAC5592REG	(sizeof(MAC5592Reg) / sizeof(RTMP_REG_PAIR))


static const RT5592_FREQUENCY_ITEM RT5592_Frequency_Plan_Xtal20M[] =
{
	/* Channel, N, K, mod, R */
	{1, 482, 4, 10, 3},
	{2, 483, 4, 10, 3},
	{3, 484, 4, 10, 3},
	{4, 485, 4, 10, 3},
	{5, 486, 4, 10, 3},
	{6, 487, 4, 10, 3},
	{7, 488, 4, 10, 3},
	{8, 489, 4, 10, 3},
	{9, 490, 4, 10, 3},
	{10, 491, 4, 10, 3},
	{11, 492, 4, 10, 3},
	{12, 493, 4, 10, 3},
	{13, 494, 4, 10, 3},
	{14, 496, 8, 10, 3},
	{36, 172, 8, 12, 1},
	{38, 173, 0, 12, 1},
	{40, 173, 4, 12, 1},
	{42, 173, 8, 12, 1},
	{44, 174, 0, 12, 1},
	{46, 174, 4, 12, 1},
	{48, 174, 8, 12, 1},
	{50, 175, 0, 12, 1},
	{52, 175, 4, 12, 1},
	{54, 175, 8, 12, 1},
	{56, 176, 0, 12, 1},
	{58, 176, 4, 12, 1},
	{60, 176, 8, 12, 1},
	{62, 177, 0, 12, 1},
	{64, 177, 4, 12, 1},
	{100, 183, 4, 12, 1},
	{102, 183, 8, 12, 1},
	{104, 184, 0, 12, 1},
	{106, 184, 4, 12, 1},
	{108, 184, 8, 12, 1},
	{110, 185, 0, 12, 1},
	{112, 185, 4, 12, 1},
	{114, 185, 8, 12, 1},
	{116, 186, 0, 12, 1},
	{118, 186, 4, 12, 1},
	{120, 186, 8, 12, 1},
	{122, 187, 0, 12, 1},
	{124, 187, 4, 12, 1},
	{126, 187, 8, 12, 1},
	{128, 188, 0, 12, 1},
	{130, 188, 4, 12, 1},
	{132, 188, 8, 12, 1},
	{134, 189, 0, 12, 1},
	{136, 189, 4, 12, 1},
	{138, 189, 8, 12, 1},
	{140, 190, 0, 12, 1},
	{149, 191, 6, 12, 1},
	{151, 191, 10, 12, 1},
	{153, 192, 2, 12, 1},
	{155, 192, 6, 12, 1},
	{157, 192, 10, 12, 1},
	{159, 193, 2, 12, 1},
	{161, 193, 6, 12, 1},
	{165, 194, 2, 12, 1},
	{184, 164, 0, 12, 1},
	{188, 164, 4, 12, 1},
	{192, 165, 8, 12, 1},
	{196, 166, 0, 12, 1},
};


static const RT5592_FREQUENCY_ITEM RT5592_Frequency_Plan_Xtal40M[] =
{
	/* Channel, N, K, mod, R */
	{1, 241, 2, 10, 3},
	{2, 241, 7, 10, 3}, 
	{3, 242, 2, 10, 3},
	{4, 242, 7, 10, 3},
	{5, 243, 2, 10, 3},
	{6, 243, 7, 10, 3},
	{7, 244, 2, 10, 3},
	{8, 244, 7, 10, 3},
	{9, 245, 2, 10, 3},
	{10, 245, 7, 10, 3},
	{11, 246, 2, 10, 3},
	{12, 246, 7, 10, 3},
	{13, 247, 2, 10, 3},
	{14, 248, 4, 10, 3},
	{36, 86, 4, 12, 1},
	{38, 86, 6, 12, 1},
	{40, 86, 8, 12, 1},
	{42, 86, 10, 12, 1},
	{44, 87, 0, 12, 1},
	{46, 87, 2, 12, 1},
	{48, 87, 4, 12, 1},
	{50, 87, 6, 12, 1},
	{52, 87, 8, 12, 1},
	{54, 87, 10, 12, 1},
	{56, 88, 0, 12, 1},
	{58, 88, 2, 12, 1},
	{60, 88, 4, 12, 1},
	{62, 88, 6, 12, 1},
	{64, 88, 8, 12, 1},
	{100, 91, 8, 12, 1},
	{102, 91, 10, 12, 1},
	{104, 92, 0, 12, 1},
	{106, 92, 2, 12, 1},
	{108, 92, 4, 12, 1},
	{110, 92, 6, 12, 1},
	{112, 92, 8, 12, 1},
	{114, 92, 10, 12, 1},
	{116, 93, 0, 12, 1},
	{118, 93, 2, 12, 1},
	{120, 93, 4, 12, 1},
	{122, 93, 6, 12, 1},
	{124, 93, 8, 12, 1},
	{126, 93, 10, 12, 1},
	{128, 94, 0, 12, 1},
	{130, 94, 2, 12, 1},
	{132, 94, 4, 12, 1},
	{134, 94, 6, 12, 1},
	{136, 94, 8, 12, 1},
	{138, 94, 10, 12, 1},
	{140, 95, 0, 12, 1},
	{149, 95, 9, 12, 1},
	{151, 95, 11, 12, 1},
	{153, 96, 1, 12, 1},
	{155, 96, 3, 12, 1},
	{157, 96, 5, 12, 1},
	{159, 96, 7, 12, 1},
	{161, 96, 9, 12, 1},
	{165, 97, 1, 12, 1},
	{184, 82, 0, 12, 1},
	{188, 82, 4, 12, 1},
	{192, 82, 8, 12, 1},
	{196, 83, 0, 12, 1},
};


const RT5592_FREQUENCY_PLAN RT5592_Frequency_Plan[] =
{
	{RT5592_Frequency_Plan_Xtal20M, 
	sizeof(RT5592_Frequency_Plan_Xtal20M) / sizeof(RT5592_FREQUENCY_ITEM)},
	{RT5592_Frequency_Plan_Xtal40M,
	sizeof(RT5592_Frequency_Plan_Xtal40M) / sizeof(RT5592_FREQUENCY_ITEM)},
};


#ifdef RTMP_TEMPERATURE_COMPENSATION
/* Power index table for G band */
static const TX_POWER_TUNING_ENTRY_STRUCT RT5592_TxPowerTuningTable_2G[] = {
/* 0  */ {0x00, -15}, 
/* 1  */ {0x01, -15}, 
/* 2  */ {0x00, -14}, 
/* 3  */ {0x01, -14}, 
/* 4  */ {0x00, -13}, 
/* 5  */ {0x01, -13}, 
/* 6  */ {0x00, -12}, 
/* 7  */ {0x01, -12}, 
/* 8  */ {0x00, -11}, 
/* 9  */ {0x01, -11}, 
/* 10 */ {0x00, -10}, 
/* 11 */ {0x01, -10}, 
/* 12 */ {0x00, -9}, 
/* 13 */ {0x01, -9}, 
/* 14 */ {0x00, -8}, 
/* 15 */ {0x01, -8}, 
/* 16 */ {0x00, -7}, 
/* 17 */ {0x01, -7}, 
/* 18 */ {0x00, -6}, 
/* 19 */ {0x01, -6}, 
/* 20 */ {0x00, -5}, 
/* 21 */ {0x01, -5}, 
/* 22 */ {0x00, -4}, 
/* 23 */ {0x01, -4}, 
/* 24 */ {0x00,	-3}, 
/* 25 */ {0x01,	-3}, 
/* 26 */ {0x00,	-2}, 
/* 27 */ {0x01, -2}, 
/* 28 */ {0x00,	-1}, 
/* 29 */ {0x01,	-1}, 
/* 30 */ {0x00,	0}, 
/* 31 */ {0x01, 0}, 
/* 32 */ {0x02,	0}, 
/* 33 */ {0x03,	0}, 
/* 34 */ {0x04,	0}, 
/* 35 */ {0x05,	0}, 
/* 36 */ {0x06, 0}, 
/* 37 */ {0x07, 0}, 
/* 38 */ {0x08,	0}, 
/* 39 */ {0x09,	0}, 
/* 40 */ {0x0A,	0}, 
/* 41 */ {0x0B,	0}, 
/* 42 */ {0x0C,	0}, 
/* 43 */ {0x0D,	0}, 
/* 44 */ {0x0E,	0}, 
/* 45 */ {0x0F, 0}, 
/* 46 */ {0x10,	0}, 
/* 47 */ {0x11,	0}, 
/* 48 */ {0x12,	0}, 
/* 49 */ {0x13,	0}, 
/* 50 */ {0x14,	0}, 
/* 51 */ {0x15, 0}, 
/* 52 */ {0x16,	0}, 
/* 53 */ {0x17,	0}, 
/* 54 */ {0x18,	0}, 
/* 55 */ {0x19,	0}, 
/* 56 */ {0x1A,	0}, 
/* 57 */ {0x1B,	0}, 
/* 58 */ {0x1C,	0}, 
/* 59 */ {0x1D,	0}, 
/* 60 */ {0x1E,	0}, 
/* 61 */ {0x1F,	0}, 
/* 62 */ {0x20,	0}, 
/* 63 */ {0x21,	0}, 
/* 64 */ {0x22,	0}, 
/* 65 */ {0x23, 0}, 
/* 66 */ {0x24,	0}, 
/* 67 */ {0x25,	0}, 
/* 68 */ {0x26,	0}, 
/* 69 */ {0x27,	0}, 
/* 70 */ {0x27-1, 1}, 
/* 71 */ {0x27,	1}, 
/* 72 */ {0x27-1, 2}, 
/* 73 */ {0x27,	2}, 
/* 74 */ {0x27-1, 3}, 
/* 75 */ {0x27,	3}, 
/* 76 */ {0x27-1, 4}, 
/* 77 */ {0x27, 4}, 
/* 78 */ {0x27-1, 5}, 
/* 79 */ {0x27,	5}, 
/* 80 */ {0x27-1, 6}, 
/* 81 */ {0x27, 6}, 
/* 82 */ {0x27-1, 7}, 
/* 83 */ {0x27,	7}, 
/* 84 */ {0x27-1, 8}, 
/* 85 */ {0x27, 8}, 
/* 86 */ {0x27-1, 9}, 
/* 87 */ {0x27, 9}, 
/* 88 */ {0x27-1, 10}, 
/* 89 */ {0x27, 10}, 
/* 90 */ {0x27-1, 11}, 
/* 91 */ {0x27,	11}, 
/* 92 */ {0x27-1, 12}, 
/* 93 */ {0x27,	12}, 
/* 94 */ {0x27-1, 13}, 
/* 95 */ {0x27,	13}, 
/* 96 */ {0x27-1, 14}, 
/* 97 */ {0x27,	14}, 
/* 98 */ {0x27-1, 15}, 
/* 99 */ {0x27, 15}, 
};


/* Power index table for A band */
static const TX_POWER_TUNING_ENTRY_STRUCT RT5592_TxPowerTuningTable_5G[] = {
/* 0  */ {0x00,	-15}, 
/* 1  */ {0x01,	-15}, 
/* 2  */ {0x00,	-14}, 
/* 3  */ {0x01,	-14}, 
/* 4  */ {0x00,	-13}, 
/* 5  */ {0x01,	-13}, 
/* 6  */ {0x00,	-12}, 
/* 7  */ {0x01,	-12}, 
/* 8  */ {0x00,	-11}, 
/* 9  */ {0x01,	-11}, 
/* 10 */ {0x00,	-10}, 
/* 11 */ {0x01,	-10}, 
/* 12 */ {0x00,	-9}, 
/* 13 */ {0x01,	-9}, 
/* 14 */ {0x00,	-8}, 
/* 15 */ {0x01,	-8}, 
/* 16 */ {0x00,	-7}, 
/* 17 */ {0x01,	-7}, 
/* 18 */ {0x00,	-6}, 
/* 19 */ {0x01,	-6}, 
/* 20 */ {0x00, -5}, 
/* 21 */ {0x01, -5}, 
/* 22 */ {0x00,	-4}, 
/* 23 */ {0x01,	-4}, 
/* 24 */ {0x00,	-3}, 
/* 25 */ {0x01, -3}, 
/* 26 */ {0x00,	-2}, 
/* 27 */ {0x01,	-2}, 
/* 28 */ {0x00, -1}, 
/* 29 */ {0x01,	-1}, 
/* 30 */ {0x00,	0}, 
/* 31 */ {0x01,	0}, 
/* 32 */ {0x02,	0}, 
/* 33 */ {0x03, 0}, 
/* 34 */ {0x04,	0}, 
/* 35 */ {0x05,	0}, 
/* 36 */ {0x06,	0}, 
/* 37 */ {0x07,	0}, 
/* 38 */ {0x08, 0}, 
/* 39 */ {0x09, 0}, 
/* 40 */ {0x0A, 0}, 
/* 41 */ {0x0B,	0}, 
/* 42 */ {0x0C,	0}, 
/* 43 */ {0x0D, 0}, 
/* 44 */ {0x0E,	0}, 
/* 45 */ {0x0F,	0}, 
/* 46 */ {0x10,	0}, 
/* 47 */ {0x11,	0}, 
/* 48 */ {0x12,	0}, 
/* 49 */ {0x13,	0}, 
/* 50 */ {0x14,	0}, 
/* 51 */ {0x15,	0}, 
/* 52 */ {0x16,	0}, 
/* 53 */ {0x17,	0}, 
/* 54 */ {0x18,	0}, 
/* 55 */ {0x19,	0}, 
/* 56 */ {0x1A,	0}, 
/* 57 */ {0x1B,	0}, 
/* 58 */ {0x1C,	0}, 
/* 59 */ {0x1D,	0}, 
/* 60 */ {0x1E,	0}, 
/* 61 */ {0x1F,	0}, 
/* 62 */ {0x20,	0}, 
/* 63 */ {0x21,	0}, 
/* 64 */ {0x22,	0}, 
/* 65 */ {0x23, 0}, 
/* 66 */ {0x24,	0}, 
/* 67 */ {0x25,	0}, 
/* 68 */ {0x26,	0}, 
/* 69 */ {0x27,	0}, 
/* 70 */ {0x28,	0},
/* 71 */ {0x29,	0},
/* 72 */ {0x2A,	0},
/* 73 */ {0x2B,	0},
/* 74 */ {0x2B-1, 1}, 
/* 75 */ {0x2B,	1}, 
/* 76 */ {0x2B-1, 2}, 
/* 77 */ {0x2B,	2}, 
/* 78 */ {0x2B-1, 3}, 
/* 79 */ {0x2B,	3}, 
/* 80 */ {0x2B-1, 4}, 
/* 81 */ {0x2B,	4}, 
/* 82 */ {0x2B-1, 5}, 
/* 83 */ {0x2B, 5}, 
/* 84 */ {0x2B-1, 6}, 
/* 85 */ {0x2B, 6}, 
/* 86 */ {0x2B-1, 7}, 
/* 87 */ {0x2B, 7}, 
/* 88 */ {0x2B-1, 8}, 
/* 89 */ {0x2B, 8}, 
/* 90 */ {0x2B-1, 9}, 
/* 91 */ {0x2B, 9}, 
/* 92 */ {0x2B-1, 10}, 
/* 93 */ {0x2B, 10}, 
/* 94 */ {0x2B-1, 11}, 
/* 95 */ {0x2B, 11}, 
/* 96 */ {0x2B-1, 12}, 
/* 97 */ {0x2B, 12}, 
/* 98 */ {0x2B-1, 13}, 
/* 99 */ {0x2B, 13}, 
/* 100 */{0x2B-1, 14}, 
/* 101 */{0x2B, 14}, 
/* 102 */{0x2B-1, 15}, 
/* 103 */{0x2B, 15}, 
};
#endif /* RTMP_TEMPERATURE_COMPENSATION */


#ifdef RTMP_FLASH_SUPPORT
#ifdef RTMP_MAC_PCI
static UCHAR RT5592_EeBuffer[EEPROM_SIZE] = {
0x92, 0x55, 0x02, 0x01, 0x00, 0x0c, 0x43, 0x30, 0x92, 0x00, 0x92, 0x30, 0x14, 0x18, 0x01, 0x80,
0x00, 0x00, 0x92, 0x30, 0x14, 0x18, 0x00, 0x00, 0x01, 0x00, 0x6a, 0xff, 0x13, 0x02, 0xff, 0xff,
0xff, 0xff, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0x8e, 0x75, 0x01, 0x43, 0x22, 0x08, 0x27, 0x00, 0xff, 0xff, 0x16, 0x01, 0xff, 0xff, 0xd9, 0xfa,
0xcc, 0x88, 0xff, 0xff, 0x0a, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
0xff, 0xff, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x1d, 0x1a,
0x15, 0x11, 0x0f, 0x0d, 0x0a, 0x07, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x88, 0x88,
0xcc, 0xcc, 0xaa, 0x88, 0xcc, 0xcc, 0xaa, 0x88, 0xcc, 0xcc, 0xaa, 0x88, 0xcc, 0xcc, 0xaa, 0x88,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, };
#endif /* RTMP_MAC_PCI */

#endif /* RTMP_FLASH_SUPPORT */


VOID RT5592SetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	UCHAR BbpValue = 0;

	if ((!pAd->NicConfig2.field.AntDiversity) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	if (Ant == 0) /* 0: Main antenna */
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpValue);
		BbpValue = ((BbpValue & ~0x80) | (0x80));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpValue);
		DBGPRINT(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to main antenna\n"));
	}
	else /* 1: Aux. antenna */
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpValue);
		BbpValue = ((BbpValue & ~0x80) | (0x00));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpValue);
		DBGPRINT(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to aux. antenna\n"));
	}
}


/*
	==========================================================================
	Description:

	Load RF normal operation-mode setup

	==========================================================================
 */
VOID RT5592LoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue, bbpreg = 0;

	/* improve power consumption */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R138, &bbpreg);
	if (pAd->Antenna.field.TxPath == 1)
		/* turn off tx DAC_1 */
		bbpreg = (bbpreg | 0x20);
	if (pAd->Antenna.field.RxPath == 1)
		/* turn off tx ADC_1 */
		bbpreg &= (~0x2);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R138, bbpreg);

	RT30xxReadRFRegister(pAd, RF_R38, &RFValue);
	RFValue = ((RFValue & ~0x20) | 0x00); /* rx_lo1_en (enable RX LO1, 0: LO1 follows TR switch) */
	RT30xxWriteRFRegister(pAd, RF_R38, RFValue);

	RT30xxReadRFRegister(pAd, RF_R39, &RFValue);
	RFValue = ((RFValue & ~0x80) | 0x00); /* rx_lo2_en (enable RX LO2, 0: LO2 follows TR switch) */
	RT30xxWriteRFRegister(pAd, RF_R39, RFValue);


	/* Avoid data lost and CRC error */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &bbpreg);
	bbpreg = ((bbpreg & ~0x40) | 0x40); /* MAC interface control (MAC_IF_80M, 1: 80 MHz) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, bbpreg);

	RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
	RFValue = ((RFValue & ~0x18) | 0x10); /* rxvcm (Rx BB filter VCM) */
	RT30xxWriteRFRegister(pAd, RF_R30, RFValue);
}

#if 0
static VOID RT5592FilterCalibration(IN PRTMP_ADAPTER pAd)
{
	UCHAR FilterTarget = 0x13;
	UCHAR RFValue, BBPValue;
	UCHAR CalRF57_PassBand = 0x00;
	UCHAR CalRF57_StopBand = 0x00;
	UINT loop = 0, count = 0, loopcnt = 0, ReTry = 0;
	UCHAR tx_agc_fc = 0x00;

	pAd->Mlme.CaliBW20RfR24 = 0x10;
	pAd->Mlme.CaliBW40RfR24 = 0x10;

	/* Enable abb_test */
	RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
	RFValue &= ~0xC0;
	RFValue |= 0x40;
	RT30xxWriteRFRegister(pAd, RF_R30, RFValue);

	do
	{
		if (loop == 1)
		{
			/*
 			 * tx_h20M = 20MHz
 			 */
			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue |= 0x02;
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);

			tx_agc_fc = 7;
			RT30xxReadRFRegister(pAd, RF_R32, &RFValue);
			RFValue &= ~0XF8;
			RFValue |= (tx_agc_fc << 3);
			RT30xxWriteRFRegister(pAd, RF_R32, RFValue);

			FilterTarget = 0x13;

			/* When calibrate BW40, BBP mask must set to BW40 */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= ~0x18;
			BBPValue |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			
			/*
 			 * rx_h20M = 20MHz
 			 */
			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue |= 0x04;
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);
		}
		else
		{
			/*
 			 * tx_h20M = 10MHz
 			 */
			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue &= ~0x02;
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);

			tx_agc_fc = 0x07;
			RT30xxReadRFRegister(pAd, RF_R32, &RFValue);
			RFValue &= ~0xF8;
			RFValue |= (tx_agc_fc << 3);
			RT30xxWriteRFRegister(pAd, RF_R32, RFValue);

			/*
				The main change is to set 20M BW target value to 0x12.
				That can provide more margin for 20M BW flatness.
			*/
			FilterTarget = 0x12;

			/*
 			 * rx_h20M = 20MHz
 			 */
			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue &= ~0x04;
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);
		}

		/*
 		 * Enable BB loopback
 		 */
		RT30xxReadRFRegister(pAd, RF_R36, &RFValue);
		RFValue |= 0x02;
		RT30xxWriteRFRegister(pAd, RF_R36, RFValue);

		/*
 		 * transmit tone frequency control of passband test tone
 		 */
		BBPValue = 0x02;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		BBPValue = 0x00;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		/*
 		 * Enable RF calibration
 		 */
		BBPValue = 0x00;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		BBPValue = 0x82;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		ReTry = 0;
	
		do
		{
			RTMPusecDelay(1000);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, ("Wait RF calibration done BBP_R0 value = 0x%02x\n", BBPValue));
		} while((ReTry++ < 100 && (BBPValue & 0x80) == 0x80));

		/*
 		 * Read Rx0 signal strnegth for RF loop back RX gain
 		 */
		BBPValue = 0x39;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BBPValue);
		CalRF57_PassBand = BBPValue & 0xFF;

		DBGPRINT(RT_DEBUG_TRACE, ("Retry = %d, CalRF57_PassBand = 0x%02x\n", ReTry, CalRF57_PassBand));
		
		/*
 		 * transmit tone frequency control of stopband test tone
 		 */
		BBPValue = 0x02;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		BBPValue = 0x06;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		while (TRUE)
		{
			
			/*
 			 * Enable RF calibration
 			 */
			BBPValue = 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			BBPValue = 0x82;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

			RTMPusecDelay(1000);
			
			/*
 		 	 * Read Rx0 signal strnegth for RF loop back RX gain
 		 	 */
			BBPValue = 0x39;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BBPValue);
			BBPValue &= 0xFF;
			CalRF57_StopBand = BBPValue;

			DBGPRINT(RT_DEBUG_TRACE, ("loopcnt = %d, CalRF57_StopBand = 0x%x,\
					tx_agc_fc = 0x%0x, CalRF57_PassBand = 0x%0x,\
					FilterTarget = 0x%0x, (CalRF57_PassBand - CalRF57_StopBand) = 0x%0x\n",
					loopcnt, CalRF57_StopBand, tx_agc_fc, CalRF57_PassBand, FilterTarget,
					(CalRF57_PassBand - CalRF57_StopBand)));

			if ((CalRF57_PassBand - CalRF57_StopBand) < FilterTarget)
				tx_agc_fc++;
			else if ((CalRF57_PassBand - CalRF57_StopBand) == FilterTarget)
			{
				tx_agc_fc++;
				count++;
			}
			else
			{
				loopcnt = 0;
				break;
			}

			if (loopcnt++ > 100)
			{
				DBGPRINT(RT_DEBUG_OFF, ("%s - can not find a valid value, loopcnt = %d\
						stop calibration\n", __FUNCTION__,loopcnt));
				break;
			}

			if (loop == 0)
			{
				RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
				RFValue &= ~0x02;
				RT30xxWriteRFRegister(pAd, RF_R30, RFValue);			

			}
			else
			{
				RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
				RFValue |= 0x02;
				RT30xxWriteRFRegister(pAd, RF_R30, RFValue);
			}

			RT30xxReadRFRegister(pAd, RF_R32, &RFValue);
			RFValue &= ~0xF8;
			RFValue |= (tx_agc_fc << 3);
			RT30xxWriteRFRegister(pAd, RF_R32, RFValue);
		}

		if (count > 0)
			tx_agc_fc = tx_agc_fc - ((count) ? 1 : 0);

		/* Store for future usage */
		if (loopcnt < 100)
		{
			if (loop++ == 0)
				pAd->Mlme.CaliBW20RfR24 = tx_agc_fc;
			else
			{
				pAd->Mlme.CaliBW40RfR24 = tx_agc_fc;
				break;
			}	

		}
		else
			break;

		if (loop == 0)
		{
			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue &= ~0x02;
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);
		}
		else
		{
			RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
			RFValue |= 0x02;
			RT30xxWriteRFRegister(pAd, RF_R30, RFValue);
		}

		RT30xxReadRFRegister(pAd, RF_R32, &RFValue);
		RFValue &= ~0xF8;
		RFValue |= (tx_agc_fc << 3);
		RT30xxWriteRFRegister(pAd, RF_R32, RFValue);

		count = 0;
	} while (TRUE);

	/*
 	 * Set back to initial state
 	 */
	BBPValue = 0x02;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
	BBPValue = 0x00;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
	
	/*
 	 * Disable BB loopback
 	 */
	RT30xxReadRFRegister(pAd, RF_R36, &RFValue);
	RFValue &= ~0x02;
	RT30xxWriteRFRegister(pAd, RF_R36, RFValue);

	/*
 	 * Set BBP back to BW20
	 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
	BBPValue &= ~0x18;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

	/*
 	 * Disable abb_test
 	 */
	RT30xxReadRFRegister(pAd, RF_R30, &RFValue);
	RFValue &= ~0xC0;
	RT30xxWriteRFRegister(pAd, RF_R30, RFValue);

	DBGPRINT(RT_DEBUG_TRACE, ("%s CaliBW20RfR24 = 0x%x, CaliBW40RfR24 = 0x%x\n", 
					__FUNCTION__, pAd->Mlme.CaliBW20RfR24, pAd->Mlme.CaliBW40RfR24));
}
#endif

static VOID NICInitRT5592RFRegisters(IN PRTMP_ADAPTER pAd)
{
	UINT32 i;
	ULONG data;
	UCHAR RfReg = 0;
	USHORT e2p_val = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: Initialize the RF registers to the default values", __FUNCTION__));

	pAd->Mlme.CaliBW20RfR24 = 0x1F;/* RF for A/G band */
	for (i = 0; i < NUM_RF5592REG_2G_5G; i++)
		RT30xxWriteRFRegister(pAd, RF5592Reg_2G_5G[i].Register, RF5592Reg_2G_5G[i].Value);

	/* Check if we need to turn xtal_out off */
	RT28xx_EEPROM_READ16(pAd, 0x42, e2p_val);
	if (!(e2p_val & (1 << 9)))
		RTMP_WriteRF(pAd, RF_R06, 0x4, 0x4);  /* Turn OFF xtal_out */
    
	/* Driver should toggle RF R02 bit7 */
	RfReg = 0x80; /* rescal_en (initiate calbration) */
	RT30xxWriteRFRegister(pAd, RF_R02, (UCHAR)RfReg);
	RTMPusecDelay(1000);
	
	//RT5592FilterCalibration(pAd);

	/* Init RF frequency offset */
	RTMPAdjustFrequencyOffset(pAd, &pAd->RfFreqOffset);

	/* Initialize RF R27 register, set RF R27 must be behind RTMPFilterCalibration() */
       if ((pAd->MACVersion & 0xffff) < 0x0211)
			RT30xxWriteRFRegister(pAd, RF_R27, 0x3);

	/* set led open drain enable */
	RTMP_IO_READ32(pAd, OPT_14, &data);
	data |= 0x01;
	RTMP_IO_WRITE32(pAd, OPT_14, data);

	RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0x0);
	RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0);

	/* set default antenna as main */
	RT5592SetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);

	/* enable DC filter */
	if ((pAd->MACVersion & 0xffff) >= 0x0211)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xc0);
	}

	/*
		From RT3071 Power Sequence v1.1 document, the Normal Operation Setting Registers as follow :
	 	BBP_R138 / RF_R1 / RF_R15 / RF_R17 / RF_R20 / RF_R21.
		add by johnli, RF power sequence setup, load RF normal operation-mode setup
	*/
	RT5592LoadRFNormalModeSetup(pAd);
}


/*
	==========================================================================
	Description:

	Load RF sleep-mode setup

	==========================================================================
 */
static VOID RT5592LoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR	rfreg;

	/* Disabe rf_block */
	RT30xxReadRFRegister(pAd, RF_R01, &rfreg);
	rfreg = ((rfreg & ~0x01) | 0x00);
	RT30xxWriteRFRegister(pAd, RF_R01, rfreg);

	RT30xxReadRFRegister(pAd, RF_R06, &rfreg);
	rfreg = ((rfreg & ~0xC0) | 0x00); /* vco_ic (VCO bias current control, 00: off) */
	RT30xxWriteRFRegister(pAd, RF_R06, rfreg);

	RT30xxReadRFRegister(pAd, RF_R22, &rfreg);
	rfreg = ((rfreg & ~0xE0) | 0x00); /* cp_ic (reference current control, 000: 0.25 mA) */
	RT30xxWriteRFRegister(pAd, RF_R22, rfreg);

	RT30xxReadRFRegister(pAd, RF_R42, &rfreg);
	rfreg = ((rfreg & ~0x40) | 0x00); /* rx_ctb_en */
	RT30xxWriteRFRegister(pAd, RF_R42, rfreg);

	RT30xxReadRFRegister(pAd, RF_R20, &rfreg);
	rfreg = ((rfreg & ~0x77) | 0x77); /* ldo_pll_vc and ldo_rf_vc (111: -0.15) */
	RT30xxWriteRFRegister(pAd, RF_R20, rfreg);
}


static VOID RT5592HaltAction(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT32		TxPinCfg = 0x00050F0F;

	TxPinCfg &= 0xFFFFF0F0;

	/* Turn off LNA_PE or TRSW_POL */
#ifdef RTMP_EFUSE_SUPPORT
	if (pAd->bUseEfuse)
#endif /* RTMP_EFUSE_SUPPORT */
		TxPinCfg &= 0xFFFBF0F0; /* bit18 off */

	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
}


/*
	==========================================================================
	Description:

	Reverse RF sleep-mode setup

	==========================================================================
 */
static VOID RT5592ReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd,
	IN BOOLEAN			FlgIsInitState)
{
	UCHAR	RFValue;

	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue = ((RFValue & ~0x3F) | 0x3F);
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
	//RFValue = 0xE4;
	RFValue = ((RFValue & ~0xC0) | 0xC0); /* vco_ic (VCO bias current control, 11: high) */
	RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

	RT30xxReadRFRegister(pAd, RF_R02, &RFValue);
	RFValue = ((RFValue & ~0x80) | 0x80); /* rescal_en (initiate calibration) */
	RT30xxWriteRFRegister(pAd, RF_R02, RFValue);
	
	RT30xxReadRFRegister(pAd, RF_R22, &RFValue);
	RFValue = ((RFValue & ~0xE0) | 0x20); /* cp_ic (reference current control, 001: 0.33 mA) */
	RT30xxWriteRFRegister(pAd, RF_R22, RFValue);

	RT30xxReadRFRegister(pAd, RF_R42, &RFValue);
	RFValue = ((RFValue & ~0x40) | 0x40); /* rx_ctb_en */
	RT30xxWriteRFRegister(pAd, RF_R42, RFValue);

	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue = ((RFValue & ~0x77) | 0x10); /* ldo_rf_vc(0) and ldo_pll_vc(111: +0.05) */
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

	RT30xxReadRFRegister(pAd, RF_R03, &RFValue);
	RFValue = ((RFValue & ~0x80) | 0x80); /* vcocal_en (initiate VCO calibration (reset after completion)) */
	RT30xxWriteRFRegister(pAd, RF_R03, RFValue);
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
static VOID NICInitRT5592MacRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT32 i;
	UINT32 MACValue;

	for(i = 0; i < NUM_MAC5592REG; i++)
	{
		RTMP_IO_WRITE32(pAd, MAC5592Reg[i].Register,
								MAC5592Reg[i].Value);
	}

	RTMP_IO_READ32(pAd, TXOP_HLDR_ET, &MACValue);
	MACValue = ((MACValue & ~0x00000080) | 0x00000080);
	RTMP_IO_WRITE32(pAd, TXOP_HLDR_ET, MACValue); 

#ifdef RT5592EP_SUPPORT
	/* For 5592EP, enable EXT_TRSW */
	if (pAd->chipCap.Priv == RT5592_TYPE_EP)
	{
		RTMP_IO_READ32(pAd, GPIO_SWITCH, &MACValue);
		MACValue |= 0x100000;
		RTMP_IO_WRITE32(pAd, GPIO_SWITCH, MACValue);
	}
#endif /* RT5592EP_SUPPORT */

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
static VOID NICInitRT5592BbpRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UCHAR BbpReg = 0;
	UINT32 i;
	BBP_R105_STRUC BBPR105 = { { 0 } };

	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	/*  The channel estimation updates based on remodulation of L-SIG and HT-SIG symbols. */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R105, &BBPR105.byte);

	 /* Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX) */
	if (pAd->Antenna.field.RxPath == 1) /* Single RX */
		BBPR105.field.MLDFor2Stream = 0;
	else
		BBPR105.field.MLDFor2Stream = 1;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, BBPR105.byte);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: BBP_R105: BBPR105.field.EnableSIGRemodulation = %d, BBPR105.field.MLDFor2Stream = %d\n",
		__FUNCTION__,
		BBPR105.field.EnableSIGRemodulation,
		BBPR105.field.MLDFor2Stream));

	/*   Avoid data lost and CRC error */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpReg);
	BbpReg = ((BbpReg & ~0x40) | 0x40); /* MAC interface control (MAC_IF_80M, 1: 80 MHz) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpReg);

	/* BBP for A/G band  */
	for (i = 0; i < NUM_BBP5592REG_2G_5G; i++)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP5592Reg_2G_5G[i].Register, BBP5592Reg_2G_5G[i].Value);

	/* BBP for A/G band GLRT function */
	for (i = 0; i < NUM_BBP5592REG_GLRT_2G_5G; i++)
	{
		/* Write index into BBP_R195 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, i + BBP_R128);

		/* Write value into bbp_196 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BBP5592Reg_GLRT_2G_5G[i]);
	}

	/*   Avoid data lost and CRC error */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpReg);
	BbpReg = ((BbpReg & ~0x40) | 0x40); /* MAC interface control (MAC_IF_80M, 1: 80 MHz) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpReg);

	if (pAd->NicConfig2.field.AntOpt == 1)
	{
		if (pAd->NicConfig2.field.AntDiversity == 0) // 0: Main antenna
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpReg);
			BbpReg = ((BbpReg & ~0x80) | (0x80));
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpReg);

			DBGPRINT(RT_DEBUG_TRACE, ("%s, switch to main antenna\n", __FUNCTION__));
		}
		else // 1: Aux. antenna
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpReg);
			BbpReg = ((BbpReg & ~0x80) | (0x00));
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpReg);

			DBGPRINT(RT_DEBUG_TRACE, ("%s, switch to aux. antenna\n", __FUNCTION__));
		}
	}
	else if (pAd->NicConfig2.field.AntDiversity == 0)	// Diversity is Off, set to Main Antenna as default
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R152, &BbpReg);
		BbpReg = ((BbpReg & ~0x80) | (0x80));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R152, BbpReg);

		DBGPRINT(RT_DEBUG_TRACE, ("%s, switch to main antenna as default ...... 3\n", __FUNCTION__));
	}

	if (RT_REV_GTE(pAd, RT5592, REV_RT5592C))
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R254, &BbpReg);
		BbpReg = ((BbpReg & ~0x80) | (0x80));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R254, BbpReg);
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R254, &BbpReg);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP_R254 = %x\n", BbpReg));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}




static VOID RT5592_ChipBBPAdjust(
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
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;

		/* TX : control channel at lower */
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


#ifdef DOT11_N_SUPPORT
		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : 20MHz, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
										pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
										pAd->CommonCfg.Channel,
										pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
										pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif /* DOT11_N_SUPPORT */
	}
}

#ifdef IQ_CAL_SUPPORT
VOID RT5592_IQCalibration(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel)
{
	UCHAR 	BBPValue;
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
	}
	else
	{
		/* TX0 IQ Gain */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2C);
		if (Channel >= 36 && Channel <= 64)
			BBPValue = IQCal(IQ_CAL_GROUP1_5G, IQ_CAL_TX0, IQ_CAL_GAIN);
		else if (Channel >= 100 && Channel <= 138)
			BBPValue = IQCal(IQ_CAL_GROUP2_5G, IQ_CAL_TX0, IQ_CAL_GAIN);
		else if (Channel >= 140 && Channel <= 165)
			BBPValue = IQCal(IQ_CAL_GROUP3_5G, IQ_CAL_TX0, IQ_CAL_GAIN);
		else
			BBPValue = 0;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
		
		/* TX0 IQ Phase */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2D);
		if (Channel >= 36 && Channel <= 64)
			BBPValue = IQCal(IQ_CAL_GROUP1_5G, IQ_CAL_TX0, IQ_CAL_PHASE);
		else if (Channel >= 100 && Channel <= 138)
			BBPValue = IQCal(IQ_CAL_GROUP2_5G, IQ_CAL_TX0, IQ_CAL_PHASE);
		else if (Channel >= 140 && Channel <= 165)
			BBPValue = IQCal(IQ_CAL_GROUP3_5G, IQ_CAL_TX0, IQ_CAL_PHASE);
		else
			BBPValue = 0;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		/* TX1 IQ Gain */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4A);
		if (Channel >= 36 && Channel <= 64)
			BBPValue = IQCal(IQ_CAL_GROUP1_5G, IQ_CAL_TX1, IQ_CAL_GAIN);
		else if (Channel >= 100 && Channel <= 138)
			BBPValue = IQCal(IQ_CAL_GROUP2_5G, IQ_CAL_TX1, IQ_CAL_GAIN);
		else if (Channel >= 140 && Channel <= 165)
			BBPValue = IQCal(IQ_CAL_GROUP3_5G, IQ_CAL_TX1, IQ_CAL_GAIN);
		else
			BBPValue = 0;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
		
		/* TX1 IQ Phase */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4B);
		if (Channel >= 36 && Channel <= 64)
			BBPValue = IQCal(IQ_CAL_GROUP1_5G, IQ_CAL_TX1, IQ_CAL_PHASE);
		else if (Channel >= 100 && Channel <= 138)
			BBPValue = IQCal(IQ_CAL_GROUP2_5G, IQ_CAL_TX1, IQ_CAL_PHASE);
		else if (Channel >= 140 && Channel <= 165)
			BBPValue = IQCal(IQ_CAL_GROUP3_5G, IQ_CAL_TX1, IQ_CAL_PHASE);
		else
			BBPValue = 0;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
	}

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
#endif /* IQ_CAL_SUPPORT */


static VOID RT5592_AsicAntennaDefaultReset(
	IN PRTMP_ADAPTER pAd,
	IN EEPROM_ANTENNA_STRUC *pAntenna)
{
		pAntenna->word = 0;
		pAntenna->field.RfIcType = RFIC_5592;
		pAntenna->field.TxPath = 2;
		pAntenna->field.RxPath = 2;
}


static VOID RT5592_ChipSwitchChannel(
	IN PRTMP_ADAPTER 			pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan)
{
	CHAR TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; /* Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER; */
	UINT i;
	UCHAR RFValue = 0;
	/*UCHAR PreRFValue;*/
	UINT32 MacValue;
	enum XTAL Xtal;
	const struct _RT5592_FREQUENCY_ITEM *pFrequencyItem;
	INTERNAL_1_STRUCT Internal_1 = { { 0 } };

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		We can't use ChannelList to search channel, since some central channl's txpowr doesn't list
		in ChannelList, so use TxPower array instead.
	*/
	for (i = 0; i < MAX_NUM_OF_CHANNELS; i++)
	{
		if (Channel == pAd->TxPower[i].Channel)
		{
			TxPwer = pAd->TxPower[i].Power;
			TxPwer2 = pAd->TxPower[i].Power2;
			break;
		}
	}

	if (i == MAX_NUM_OF_CHANNELS)
		DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: Can't find the Channel#%d \n", Channel));
	
	RTMP_IO_READ32(pAd, LDO_CFG0, &MacValue);

	if (Channel <= 14)
	{
		if (pAd->CommonCfg.BBPCurrentBW == BW_40)
			MacValue = ((MacValue & ~0x1C000000) | 0x14000000);
		else		
			MacValue = ((MacValue & ~0x1C000000) | 0x00000000);
	}
	else
		MacValue = ((MacValue & ~0x1C000000) | 0x14000000);

	RTMP_IO_WRITE32(pAd, LDO_CFG0, MacValue);

	
	RTMP_IO_READ32(pAd, DEBUG_INDEX, &MacValue);
	if (MacValue & 0x80000000)	
		Xtal = XTAL40M;
	else
		Xtal = XTAL20M;

	pFrequencyItem = RT5592_Frequency_Plan[Xtal].pFrequencyPlan;
	for (i = 0; i < RT5592_Frequency_Plan[Xtal].totalFreqItem; i++, pFrequencyItem++)
	{
		if (Channel == pFrequencyItem->Channel)
		{
			/* Frequeny plan setting */
			/*  
 			 * N setting
 			 * R9[4], R8[7:0] (RF PLL freq selection) 
 			 */
			RT30xxReadRFRegister(pAd, RF_R08, &RFValue);
			RFValue = (pFrequencyItem->N & 0x00ff);
			RT30xxWriteRFRegister(pAd, RF_R08, RFValue);

			RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
			RFValue = RFValue & ~0x10;
			RFValue |= ((pFrequencyItem->N & 0x0100) >> 8) << 4;
			RT30xxWriteRFRegister(pAd, RF_R09, RFValue);
			
			/* 
 			 * K setting 
 			 * R9[3:0] (RF PLL freq selection)
 			 */
			RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
			RFValue = RFValue & ~0x0f;
			RFValue |= (pFrequencyItem->K & 0x0f);
			RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

			/* 
 			 * mode setting 
 			 * R9[7] (RF PLL freq selection)
 			 * R11[3:2] (RF PLL)
 			 * mod=8 => 0x0
 			 * mod=10 => 0x2
 			 */
			RT30xxReadRFRegister(pAd, RF_R11, &RFValue);
			RFValue = RFValue & ~0x0c;
			RFValue |= ((pFrequencyItem->mod - 0x8) & 0x3) << 2;
			RT30xxWriteRFRegister(pAd, RF_R11, RFValue);

			RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
			RFValue = RFValue & ~0x80;
			RFValue |= (((pFrequencyItem->mod - 0x8) & 0x4) >> 2) << 7;
			RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

			/* 
 			 * R setting 
 			 * R11[1:0]
 			 * R=1 => 0x0
 			 * R=3 => 0X2
 			 */
			RT30xxReadRFRegister(pAd, RF_R11, &RFValue);
			RFValue = RFValue & ~0x03;
			RFValue |= (pFrequencyItem->R - 0x1);
			RT30xxWriteRFRegister(pAd, RF_R11, RFValue);
		
			/* RF setting */
			if (Channel <= 14)
			{
				/* RF for G band */
				for (i = 0; i < NUM_RF5592REG_2G; i++)
					RT30xxWriteRFRegister(pAd, RF5592Reg_2G[i].Register, 
							                   RF5592Reg_2G[i].Value);

#ifdef RTMP_TEMPERATURE_COMPENSATION
				if (pAd->chipCap.bTempCompTxALC)
				{
					/*  Set RF_R27 */
					RT30xxReadRFRegister(pAd, RF_R27, &RFValue);

					/* Set [3:0] to TssiGain */
					RFValue = (RFValue & 0xf0);
					RFValue = (RFValue | pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_2G]);

					RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
				}
#endif /* RTMP_TEMPERATURE_COMPENSATION */

				/* RF for G band per channel */
				for (i = 0; i < NUM_RF5592REG_CHANNEL_2G; i++)
				{
					if ((Channel >= RF5592Reg_Channel_2G[i].FirstChannel) && 
						(Channel <= RF5592Reg_Channel_2G[i].LastChannel))
						RT30xxWriteRFRegister(pAd, RF5592Reg_Channel_2G[i].Register, 
					                     	       RF5592Reg_Channel_2G[i].Value);
				}

				if (pAd->CommonCfg.PhyMode == PHY_11B)
				{
					/* RF for CCK */
					for (i = 0; i < NUM_RF5592REG_CCK; i++)
						RT30xxWriteRFRegister(pAd, RF5592Reg_CCK[i].Register,
							                       RF5592Reg_CCK[i].Value);
#ifdef RT5592EP_SUPPORT
					if (pAd->chipCap.Priv == RT5592_TYPE_EP)
						RT30xxWriteRFRegister(pAd, RF_R55, 0x06);
#endif /* RT5592EP_SUPPORT */
				} 
				else
				{
					/* RF for G band OFDM */
					for (i = 0; i < NUM_RF5592REG_OFDM_2G; i++)
						RT30xxWriteRFRegister(pAd, RF5592Reg_OFDM_2G[i].Register,
									               RF5592Reg_OFDM_2G[i].Value);
#ifdef RT5592EP_SUPPORT
					if (pAd->chipCap.Priv == RT5592_TYPE_EP)
						RT30xxWriteRFRegister(pAd, RF_R55, 0x03);
#endif /* RT5592EP_SUPPORT */
				}

				/* 
 			 	 * R49 CH0 TX power ALC code(RF DAC value) 
 			 	 * G-band bit<7:6>=1:0, bit<5:0> range from 0x0~0x27
 			 	 */
				RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
				RFValue = RFValue & ~0xC0;
#ifdef RT5592EP_SUPPORT
				if (pAd->chipCap.Priv != RT5592_TYPE_EP)
#endif /* RT5592EP_SUPPORT */
				RFValue |= (0x2 << 6);

				RFValue = ((RFValue & ~0x3F) | (TxPwer & 0x3F));
				
				if ((RFValue & 0x3F) > 0x27)
					RFValue = ((RFValue & ~0x3F) | 0x27);
				
				RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

				/* 
 			 	 * R50 CH0 TX power ALC code(RF DAC value) 
 			 	 * G-band bit<7:6>=1:0, bit<5:0> range from 0x0~0x27
 			 	 */
				RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
				RFValue = RFValue & ~0xC0;
#ifdef RT5592EP_SUPPORT
				if (pAd->chipCap.Priv != RT5592_TYPE_EP)
#endif /* RT5592EP_SUPPORT */
				RFValue |= (0x2 << 6);

				RFValue = ((RFValue & ~0x3F) | (TxPwer2 & 0x3F));
				
				if ((RFValue & 0x3F) > 0x27)
					RFValue = ((RFValue & ~0x3F) | 0x27);
				
				RT30xxWriteRFRegister(pAd, RF_R50, RFValue);

			}
			else
			{
				/* RF for A band */
				for (i = 0; i < NUM_RF5592REG_5G; i++)
					RT30xxWriteRFRegister(pAd, RF5592Reg_5G[i].Register, 
							                   RF5592Reg_5G[i].Value);

#ifdef RTMP_TEMPERATURE_COMPENSATION
				if (pAd->chipCap.bTempCompTxALC)
				{
					/*  Set RF_R27 */
					RT30xxReadRFRegister(pAd, RF_R27, &RFValue);

					/* Set [3:0] to TssiGain */
					RFValue = (RFValue & 0xf0);
					RFValue = (RFValue | pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_5G]);

					RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
				}
#endif /* RTMP_TEMPERATURE_COMPENSATION */

				/* RF for A band per channel */
				for (i = 0; i < NUM_RF5592REG_CHANNEL_5G; i++)
				{
					if ((Channel >= RF5592Reg_Channel_5G[i].FirstChannel) && 
						(Channel <= RF5592Reg_Channel_5G[i].LastChannel))
						RT30xxWriteRFRegister(pAd, RF5592Reg_Channel_5G[i].Register, 
					                     	       RF5592Reg_Channel_5G[i].Value);
				}
					
				/* 
 			 	 * R49 CH0 TX power ALC code(RF DAC value) 
 			 	 * A-band bit<7:6>=1:1, bit<5:0> range from 0x0~0x2B
 			 	 */
				RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
				RFValue = RFValue & ~0xC0;

#ifdef RT5592EP_SUPPORT
				if (pAd->chipCap.Priv != RT5592_TYPE_EP)
#endif /* RT5592EP_SUPPORT */
					RFValue |= (0x3 << 6);

				RFValue = ((RFValue & ~0x3F) | (TxPwer & 0x3F));
				
				if ((RFValue & 0x3F) > 0x2B)
					RFValue = ((RFValue & ~0x3F) | 0x2B);
				
				RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

				/* 
 			 	 * R50 CH1 TX power ALC code(RF DAC value) 
 			 	 * A-band bit<7:6>=1:1, bit<5:0> range from 0x0~0x2B
 			 	 */
				RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
				RFValue = RFValue & ~0xC0;

#ifdef RT5592EP_SUPPORT
				if (pAd->chipCap.Priv != RT5592_TYPE_EP)
#endif /* RT5592EP_SUPPORT */
					RFValue |= (0x3 << 6);

				RFValue = ((RFValue & ~0x3F) | (TxPwer2 & 0x3F));
				
				if ((RFValue & 0x3F) > 0x2B)
					RFValue = ((RFValue & ~0x3F) | 0x2B);
				
				RT30xxWriteRFRegister(pAd, RF_R50, RFValue);

			}

			/* Enable RF block */
			RT30xxReadRFRegister(pAd, RF_R01, &RFValue);

			/* Enable rf_block_en, pll_en */
			RFValue = ((RFValue & ~0x3) | 0x3);

			if (pAd->Antenna.field.TxPath == 2)
			{
				/* Enable tx0_en, tx1_en */
				RFValue = ((RFValue & ~0x28) | 0x28);
			}
			else if (pAd->Antenna.field.TxPath == 1)
			{
				/* Enable tx0_en */
				RFValue = ((RFValue & ~0x28) | 0x08);
			}

			if (pAd->Antenna.field.RxPath == 2)
			{
				/* Enable rx0_en, rx1_en */
				RFValue = ((RFValue & ~0x14) | 0x14);
			}
			else if (pAd->Antenna.field.RxPath == 1)
			{
				/* Enable rx0_en */
				RFValue = ((RFValue & ~0x14) | 0x04);
			}

			RT30xxWriteRFRegister(pAd, RF_R01, RFValue);
			
			/* RF for A/G band BW */
			for (i = 0; i < NUM_RF5592REG_BW_2G_5G; i++)
			{
				if(pAd->CommonCfg.BBPCurrentBW == RF5592Reg_BW_2G_5G[i].BW)
				{
					RT30xxWriteRFRegister(pAd, RF5592Reg_BW_2G_5G[i].Register,
				                               RF5592Reg_BW_2G_5G[i].Value);
				}
			}

			/* RF for A/G band OFDM */
			if (pAd->CommonCfg.PhyMode != PHY_11B)
			{
				for (i = 0; i < NUM_RF5592REG_OFDM_2G_5G; i++)
					RT30xxWriteRFRegister(pAd, RF5592Reg_OFDM_2G_5G[i].Register,
							                   RF5592Reg_OFDM_2G_5G[i].Value);
			}


			/* vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. */
			RTMP_WriteRF(pAd, RF_R03, 0x80, 0x80);	
			pAd->LatchRfRegs.Channel = Channel; /* Channel latch */

			DBGPRINT(RT_DEBUG_TRACE,
				("%s: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), "
				"N=0x%02X, K=0x%02X, R=0x%02X, Xtal=%d\n",
				__FUNCTION__,
				Channel,
				pAd->RfIcType,
				TxPwer,
				TxPwer2,
				pAd->Antenna.field.TxPath,
				pFrequencyItem->N,
				pFrequencyItem->K,
				pFrequencyItem->R,
				Xtal));

			break;
		}
	}

	/* BBP setting */
	if (Channel <= 14)
	{
		ULONG	TxPinCfg = 0x00050F0A;/* Gary 2007/08/09 0x050A0A */

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* BBP for G band */
		for (i = 0; i < NUM_BBP5592REG_2G; i++)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP5592Reg_2G[i].Register, BBP5592Reg_2G[i].Value);

		/* BBP for G band GLRT */
		for (i = 0; i < NUM_BBP5592REG_GLRT_2G; i++)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP5592Reg_GLRT_2G[i].Register);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BBP5592Reg_GLRT_2G[i].Value);
		}

		/* 5G band selection PIN, bit1 and bit2 are complement */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &MacValue);
		MacValue &= (~0x6);
		MacValue |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, MacValue);


#ifdef RT5592EP_SUPPORT
		/* For 5592EP, use PA_PE_A0/A1 instead of G0/G1 */
		if (pAd->chipCap.Priv == RT5592_TYPE_EP)
			TxPinCfg |= 0x5;
#endif /* RT5592EP_SUPPORT */
		/* Turn off unused PA or LNA when only 1T or 1R */
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg &= 0xFFFFFFF3;
		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg &= 0xFFFFF3FF;

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		if (IS_PCIE_INF(pAd))
		{
			/* CH#14 channel interference */
			RTMP_IO_READ32(pAd, INTERNAL_1, &Internal_1.word);

			if (Channel == 14) /* Channel #14 */
			{
				Internal_1.field.PCIE_PHY_TX_ATTEN_EN = 1; /* Enable PCIe PHY Tx attenuation */
				Internal_1.field.PCIE_PHY_TX_ATTEN_VALUE = 4; /* 9/16 full drive level */
			}
			else /* Channel #1~#13 */
			{
				Internal_1.field.PCIE_PHY_TX_ATTEN_EN = 0; /* Disable PCIe PHY Tx attenuation */
				Internal_1.field.PCIE_PHY_TX_ATTEN_VALUE = 0; /* n/a */
			}

			RTMP_IO_WRITE32(pAd, INTERNAL_1, Internal_1.word);
		}

		RtmpUpdateFilterCoefficientControl(pAd, Channel);
	}
	else
	{
		ULONG	TxPinCfg = 0x00050F05;/* Gary 2007/8/9 0x050505 */

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		
		/* BBP for A band */
		for (i = 0; i < NUM_BBP5592REG_5G; i++)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP5592Reg_5G[i].Register, BBP5592Reg_5G[i].Value);

		/* BBP for A band GLRT */
		for (i = 0; i < NUM_BBP5592REG_GLRT_5G; i++)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP5592Reg_GLRT_5G[i].Register);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BBP5592Reg_GLRT_5G[i].Value);
		}

		/* 5G band selection PIN, bit1 and bit2 are complement */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &MacValue);
		MacValue &= (~0x6);
		MacValue |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, MacValue);

		/* Turn off unused PA or LNA when only 1T or 1R */
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg &= 0xFFFFFFF3;

		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg &= 0xFFFFF3FF;

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}

	/* BBP 2G_5G_GLRT for different BW */
	for (i = 0; i < NUM_BBP5592REG_GLRT_BW_2G_5G; i++)
	{
		if(pAd->CommonCfg.BBPCurrentBW == BBP5592Reg_GLRT_BW_2G_5G[i].BW)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP5592Reg_GLRT_BW_2G_5G[i].Register);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BBP5592Reg_GLRT_BW_2G_5G[i].Value);
		}
	}

	if (pAd->Antenna.field.RxPath == 1)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R128);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, ((Channel <= 14) ? 0xa0 : 0xb0));

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R170);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x12);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R171);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x10);
	}

	/* AGC VGA init value, and use 20MHz when scanning */
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

#ifdef IQ_CAL_SUPPORT
	/* IQ Calibration */
	RTMP_CHIP_IQ_CAL(pAd, Channel);
#endif /* IQ_CAL_SUPPORT */

	/* maybe the content will be changed by firmware */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x9a);

	/*
	  On 11A, We should delay and wait RF/BBP to be stable
	  and the appropriate time should be 1000 micro seconds
	  2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.
	*/
	RTMPusecDelay(1000);
}


VOID RT5592_AsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg8 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	/* For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 */
	RTMP_IO_READ32(pAd, 0x1318, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, 0x131C, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) << 8; /* Get Tx power for HT MCS 7 */			
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

	/* For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1324, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);

	/*  For HT_MCS_15, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1320, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg8 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for HT MCS 15 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, ExtraPwrOverTxPwrCfg8);
		
	DBGPRINT(RT_DEBUG_INFO, ("Offset =0x13D8, TxPwr = 0x%08X, ", (UINT)ExtraPwrOverTxPwrCfg8));
	
	DBGPRINT(RT_DEBUG_INFO, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}


static VOID RT5592_AsicGetAutoAgcOffsetForTemperatureSensor(
	IN PRTMP_ADAPTER 		pAd,
	IN PCHAR				pDeltaPwr,
	IN PCHAR				pTotalDeltaPwr,
	IN PCHAR				pAgcCompensate,
	IN PCHAR 				pDeltaPowerByBbpR1)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry0 = NULL; /* Ant0 */
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry1 = NULL; /* Ant1 */
	BBP_R49_STRUC	BbpR49;
	BOOLEAN			bAutoTxAgc = FALSE;
	PCHAR			pTxAgcCompensate = NULL;
	UCHAR 			RFValue = 0;
	CHAR			TuningTableUpperBound = 0, TuningTableIndex0 = 0, TuningTableIndex1 = 0;
	INT 				CurrentTemp = 0;
	INT RefTemp;
	INT *LookupTable;
	INT	LookupTableIndex = pAd->TxPowerCtrl.LookupTableIndex + TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET;
	INT channel_index = 0;
	UCHAR Channel;
	CHAR TxPower0 = 0, TxPower1 = 0;

	DBGPRINT(RT_DEBUG_INFO, ("-->%s\n", __FUNCTION__));
	
	BbpR49.byte = 0;
	*pTotalDeltaPwr = 0;

#ifdef DOT11_N_SUPPORT
	Channel = pAd->CommonCfg.CentralChannel;
#else
	Channel = pAd->CommonCfg.Channel;
#endif /* DOT11_N_SUPPORT */

	/* Get calibrated per channel DAC. */
	for (channel_index=0; channel_index<MAX_NUM_OF_CHANNELS; channel_index++)
	{
		if (Channel == pAd->TxPower[channel_index].Channel)
		{
			TxPower0 = pAd->TxPower[channel_index].Power;
			TxPower1 = pAd->TxPower[channel_index].Power2;
			break;
		}
	}

	if (channel_index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT_ERR(("Channel DAC not found\n"));
		return;
	}

#ifdef A_BAND_SUPPORT
	if (Channel > 14)
	{
		/* a band channel */
		bAutoTxAgc = pAd->bAutoTxAgcA;
		pTxAgcCompensate = &pAd->TxAgcCompensateA;
		TxPowerTuningTable = pChipCap->TxPowerTuningTable_5G;
		RefTemp = pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_5G];
		LookupTable = &pAd->TxPowerCtrl.LookupTable[IEEE80211_BAND_5G][0];
		TuningTableUpperBound = pChipCap->TxAlcTxPowerUpperBound_5G;
	}
	else
#endif /* A_BAND_SUPPORT */
	{
		/* bg band channel */
		bAutoTxAgc = pAd->bAutoTxAgcG;
		pTxAgcCompensate = &pAd->TxAgcCompensateG;
		TxPowerTuningTable = pChipCap->TxPowerTuningTable_2G;
		RefTemp = pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G];
		LookupTable = &pAd->TxPowerCtrl.LookupTable[IEEE80211_BAND_2G][0];
		TuningTableUpperBound = pChipCap->TxAlcTxPowerUpperBound_2G;
	}

	/* AutoTxAgc in EEPROM means temperature compensation enabled/diablded. */
	if (bAutoTxAgc)
	{ 
		/* Current temperature */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
		CurrentTemp = (CHAR)BbpR49.byte;
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] BBP_R49 = %02x, current temp = %d\n", BbpR49.byte, CurrentTemp));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] RefTemp = %d\n", RefTemp));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] index = %d\n", pAd->TxPowerCtrl.LookupTableIndex));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex - 1, LookupTable[LookupTableIndex - 1]));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex, LookupTable[LookupTableIndex]));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex + 1, LookupTable[LookupTableIndex + 1]));
		if (CurrentTemp > RefTemp + LookupTable[LookupTableIndex + 1] + ((LookupTable[LookupTableIndex + 1] - LookupTable[LookupTableIndex]) >> 2) &&
			LookupTableIndex < 32)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ++\n"));
			LookupTableIndex++;
			pAd->TxPowerCtrl.LookupTableIndex++;
		}
		else if (CurrentTemp < RefTemp + LookupTable[LookupTableIndex] - ((LookupTable[LookupTableIndex] - LookupTable[LookupTableIndex - 1]) >> 2) &&
			LookupTableIndex > 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] --\n"));
			LookupTableIndex--;
			pAd->TxPowerCtrl.LookupTableIndex--;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ==\n"));
		}

		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] idxTxPowerTable=%d, idxTxPowerTable2=%d, TuningTableUpperBound=%d\n",
			pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPowerCtrl.LookupTableIndex,
			pAd->TxPowerCtrl.idxTxPowerTable2 + pAd->TxPowerCtrl.LookupTableIndex,
			TuningTableUpperBound));

		TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable 
									+ pAd->TxPowerCtrl.LookupTableIndex 
									+ TxPower0;
		/* The boundary verification */ 
		TuningTableIndex0 = (TuningTableIndex0 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex0;
		TuningTableIndex0 = (TuningTableIndex0 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex0;
		TxPowerTuningTableEntry0 = &TxPowerTuningTable[TuningTableIndex0 + TX_POWER_TUNING_ENTRY_OFFSET];
		
		TuningTableIndex1 = pAd->TxPowerCtrl.idxTxPowerTable2 
									+ pAd->TxPowerCtrl.LookupTableIndex 
									+ TxPower1;
		/* The boundary verification */
		TuningTableIndex1 = (TuningTableIndex1 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex1;
		TuningTableIndex1 = (TuningTableIndex1 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex1;
		TxPowerTuningTableEntry1 = &TxPowerTuningTable[TuningTableIndex1 + TX_POWER_TUNING_ENTRY_OFFSET];
			
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx0)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry0->RF_TX_ALC, TxPowerTuningTableEntry0->MAC_PowerDelta, TuningTableIndex0));
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx1)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry1->RF_TX_ALC, TxPowerTuningTableEntry1->MAC_PowerDelta, TuningTableIndex1));

		/* Update RF_R49 [0:5] */
		RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
		RFValue = ((RFValue & ~0x3F) | TxPowerTuningTableEntry0->RF_TX_ALC);
		if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
		{
			RFValue = ((RFValue & ~0x3F) | 0x27);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Update RF_R49[0:5] to 0x%x\n", TxPowerTuningTableEntry0->RF_TX_ALC));
		RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

		/* Update RF_R50 [0:5] */
		RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
		RFValue = ((RFValue & ~0x3F) | TxPowerTuningTableEntry1->RF_TX_ALC);
		if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
		{
			RFValue = ((RFValue & ~0x3F) | 0x27);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Update RF_R50[0:5] to 0x%x\n", TxPowerTuningTableEntry1->RF_TX_ALC));
		RT30xxWriteRFRegister(pAd, RF_R50, RFValue);
		
		*pTotalDeltaPwr = TxPowerTuningTableEntry0->MAC_PowerDelta;
		
	}

	*pAgcCompensate = *pTxAgcCompensate;
	DBGPRINT(RT_DEBUG_INFO, ("<--%s\n", __FUNCTION__));
}


static VOID RT5592_RTMPAGCInit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR BandWidth)
{
	UCHAR	R66;

	if (pAd->LatchRfRegs.Channel <= 14)
		R66 = 0x1C + 2 * GET_LNA_GAIN(pAd);
	else
		R66 = 0x24 + 2 * GET_LNA_GAIN(pAd);

	AsicBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);

}


VOID RT5592_AsicBBPIQReCal(
	IN PRTMP_ADAPTER pAd)
{
	ULONG loop = 0;
	UINT8 BBPValue = 0;
	UINT32 MacValue = 0;

	DBGPRINT(RT_DEBUG_OFF, ("<-- %s\n", __FUNCTION__));
	
	/* Disable PA. */
	RTMP_IO_READ32(pAd, TX_PIN_CFG, &MacValue);
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, (MacValue & 0xfffff0f0));

	/* Re-calibarate BBP IQ. */
	BBP_IO_WRITE8_BY_REG_ID(pAd, 158, 0x00);
	BBP_IO_WRITE8_BY_REG_ID(pAd, 159, 0x80);

	/* Check re-calibarate BBP IQ done. */
	do
	{
		BBP_IO_READ8_BY_REG_ID(pAd, 159, &BBPValue);
		RTMPusecDelay(5);
	} while ((BBPValue != 0) && (loop++ <= 100));

	if (loop == 101)
		DBGPRINT(RT_DEBUG_OFF, ("BBP re-calibaration fail! \n"));
	else
	{
#ifdef IQ_CAL_SUPPORT
		/* IQ Calibration */
		RTMP_CHIP_IQ_CAL(pAd, pAd->CommonCfg.Channel);
#endif /* IQ_CAL_SUPPORT */
	}

	/* Enable PA */
	RTMP_IO_READ32(pAd, TX_PIN_CFG, &MacValue);
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, (MacValue | 0x00050f0f));

	DBGPRINT(RT_DEBUG_OFF, ("--> %s\n", __FUNCTION__));
}


static const RTMP_CHIP_CAP RT5592_ChipCap = {
	.MaxNumOfRfId = 63,
	.MaxNumOfBbpId = 248,
	.pRFRegTable = RF5592Reg_2G_5G,
	.bbpRegTbSize = 0,
	.TXWISize = 20,
	.RXWISize = 24,
	.SnrFormula = SNR_FORMULA3,
	.RfReg17WtMethod = RF_REG_WT_METHOD_STEP_ON,
	.FlgIsHwWapiSup = TRUE,
	.VcoPeriod = 10,
	.FlgIsVcoReCalMode = VCO_CAL_MODE_2,
	.FlgIsHwAntennaDiversitySup = FALSE,
#ifdef RTMP_EFUSE_SUPPORT
	.EFUSE_USAGE_MAP_START = 0x3c0,
	.EFUSE_USAGE_MAP_END = 0x3fb,
	.EFUSE_USAGE_MAP_SIZE = 60,
#endif /* RTMP_EFUSE_SUPPORT */
#ifdef CONFIG_TSO_SUPPORT
	.TCPChkOffload = TRUE,
#endif /* CONFIG_TSO_SUPPORT */
#ifdef RTMP_TEMPERATURE_COMPENSATION
	.bTempCompTxALC = TRUE,
	.TxAlcTxPowerUpperBound_2G = 69,
	.TxPowerTuningTable_2G = RT5592_TxPowerTuningTable_2G,
#ifdef A_BAND_SUPPORT
	.TxAlcTxPowerUpperBound_5G = 73,
	.TxPowerTuningTable_5G = RT5592_TxPowerTuningTable_5G,
#endif /* A_BAND_SUPPORT */
#endif /* RTMP_TEMPERATURE_COMPENSATION */
#ifdef RTMP_FLASH_SUPPORT
	.eebuf = RT5592_EeBuffer,
#endif /* RTMP_FLASH_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	.carrier_func = TONE_RADAR_V2,
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef DFS_SUPPORT
	.DfsEngineNum = 5,
#endif /* DFS_SUPPORT */
	.WPDMABurstSIZE = 3,
#ifdef NEW_MBSSID_MODE
	.MBSSIDMode = MBSSID_MODE1,
#else
	.MBSSIDMode = MBSSID_MODE0,
#endif /* NEW_MBSSID_MODE */
#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#endif /* RTMP_FREQ_CALIBRATION_SUPPORT */
};

 
static const RTMP_CHIP_OP RT5592_ChipOp = {
	.AsicRfInit = NICInitRT5592RFRegisters,
	.AsicBbpInit = NICInitRT5592BbpRegisters,
	.AsicMacInit = NICInitRT5592MacRegisters,
	.AsicHaltAction = RT5592HaltAction,
	.AsicRfTurnOff = RT5592LoadRFSleepModeSetup,
	.AsicReverseRfFromSleepMode = RT5592ReverseRFSleepModeSetup,
	.ChipBBPAdjust = RT5592_ChipBBPAdjust,
	.AsicAntennaDefaultReset = RT5592_AsicAntennaDefaultReset,
	.ChipSwitchChannel = RT5592_ChipSwitchChannel,
	.AsicAdjustTxPower = AsicAdjustTxPower,
	.ChipAGCInit = RT5592_RTMPAGCInit,
	.NetDevNickNameInit = NetDevNickNameInit,
#ifdef IQ_CAL_SUPPORT
	.ChipIQCalibration = RT5592_IQCalibration,
#endif /* IQ_CAL_SUPPORT */
	.AsicGetTxPowerOffset = AsicGetTxPowerOffset,
	.AsicExtraPowerOverMAC = RT5592_AsicExtraPowerOverMAC,
#ifdef RTMP_TEMPERATURE_COMPENSATION
	.AsicTxAlcGetAutoAgcOffset = RT5592_AsicGetAutoAgcOffsetForTemperatureSensor,
	.ATEReadExternalTSSI = NULL,
#endif /* RTMP_TEMPERATURE_COMPENSATION */
	.AsicResetBbpAgent = RT5592_AsicBBPIQReCal,
#if defined(CARRIER_DETECTION_SUPPORT) || defined(DFS_SUPPORT)
	.CckMrcStatusCtrl = CckMrcStatusCtrl,
	.RadarGLRTCompensate = RadarGLRTCompensate,
#endif /* defined(CARRIER_DETECTION_SUPPORT) || defined(DFS_SUPPORT) */
#ifdef CARRIER_DETECTION_SUPPORT
	.ToneRadarProgram = ToneRadarProgram_v2,
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef GREENAP_SUPPORT
	.EnableAPMIMOPS = EnableAPMIMOPSv2,
	.DisableAPMIMOPS = DisableAPMIMOPSv2,
#endif /* GREENAP_SUPPORT */
};


VOID RT5592_Init(
	IN PRTMP_ADAPTER		pAd)
{
#ifdef RT5592EP_SUPPORT
	UINT32 MacValue;
#endif

	memcpy(&pAd->chipCap, &RT5592_ChipCap, sizeof(RTMP_CHIP_CAP));
	memcpy(&pAd->chipOps, &RT5592_ChipOp, sizeof(RTMP_CHIP_OP));

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RtmpChipBcnSpecInit(pAd);
#else
	RtmpChipBcnInit(pAd);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */

#ifdef RT5592EP_SUPPORT
	/* check if this is RT5592 EP version */
	RTMP_IO_READ32(pAd, INTERNAL_1, &MacValue);
	if ((MacValue & 0x30) == 0x30) /* bit4&5 = 1 */
	{
		RF5592Reg_5G = RF5592Reg_5G_EP;
		NUM_RF5592REG_5G = (sizeof(RF5592Reg_5G_EP) / sizeof(REG_PAIR));
		RF5592Reg_Channel_5G = RF5592Reg_Channel_5G_EP;
		NUM_RF5592REG_CHANNEL_5G = (sizeof(RF5592Reg_Channel_5G_EP) / sizeof(REG_PAIR_CHANNEL));
		RF5592Reg_Channel_2G = RF5592Reg_Channel_2G_EP;
		NUM_RF5592REG_CHANNEL_2G = (sizeof(RF5592Reg_Channel_2G_EP) / sizeof(REG_PAIR_CHANNEL));
		pAd->chipCap.Priv = RT5592_TYPE_EP;
		DBGPRINT(RT_DEBUG_OFF, ("This is RT5592 EP version!\n"));
	}
#endif /* RT5592EP_SUPPORT */
}
