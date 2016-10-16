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
	mt76x0.c

	Abstract:
	Specific funcitons and configurations for MT76x0

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

#if defined(MT7650) || defined(MT7630)
#include "mcu/MT7650_firmware.h"
#endif

#ifdef MT7610
#include "mcu/MT7610_firmware.h"
#endif

#define COEXCFG3 0x4C
#define LDO_CTRL_0 			0x006C
#define LDO_CTRL1			0x0070
#define CSR_EE_CFG1 0x0104
#define IOCFG_6  0x0124
#define MT7650_EFUSE_CTRL	0x0024

#ifdef SINGLE_SKU_V2
#define MT76x0_RF_2G_PA_MODE0_DECODE		0
#define MT76x0_RF_2G_PA_MODE1_DECODE		29491	// 3.6 * 8192
#define MT76x0_RF_2G_PA_MODE3_DECODE		4096	// 0.5 * 8192

#define MT76x0_RF_5G_PA_MODE0_DECODE		0
#define MT76x0_RF_5G_PA_MODE1_DECODE		0
#endif /* SINGLE_SKU_V2 */

#ifdef RTMP_FLASH_SUPPORT
UCHAR MT76x0_EeBuffer[1024] = 
#else
UCHAR MT76x0_EeBuffer[EEPROM_SIZE] = 
#endif
	{
	0x83, 0x38, 0x01, 0x00, 0x00, 0x0c, 0x43, 0x28, 0x83, 0x00, 0x83, 0x28, 0x14, 0x18, 0xff, 0xff,
	0xff, 0xff, 0x83, 0x28, 0x14, 0x18, 0x00, 0x00, 0x01, 0x00, 0x6a, 0xff, 0x00, 0x02, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 0x43, 0x28, 0x83, 0x01, 0x00, 0x0c,
	0x43, 0x28, 0x83, 0x02, 0x33, 0x0a, 0xec, 0x00, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x20, 0x01, 0x55, 0x77, 0xa8, 0xaa, 0x8c, 0x88, 0xff, 0xff, 0x0a, 0x08, 0x08, 0x06,
	0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x66, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa,
	0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa,
	0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xcc, 0xaa,
	0x88, 0x66, 0xcc, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xaa, 0xaa,
	0x88, 0x66, 0xaa, 0xaa, 0x88, 0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	} ;

#define BIT0		(1 << 0)
#define BIT1		(1 << 1)
#define BIT2		(1 << 2)
#define BIT3		(1 << 3)
#define BIT4		(1 << 4)
#define BIT5		(1 << 5)
#define BIT6		(1 << 6)
#define BIT7		(1 << 7)
#define BIT8		(1 << 8)
#define BIT9		(1 << 9)
#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)
#define BIT16	(1 << 16)
#define BIT17	(1 << 17)
#define BIT18	(1 << 18)
#define BIT19	(1 << 19)
#define BIT20	(1 << 20)
#define BIT21	(1 << 21)
#define BIT22	(1 << 22)
#define BIT23	(1 << 23)
#define BIT24	(1 << 24)
#define BIT25	(1 << 25)
#define BIT26	(1 << 26)
#define BIT27	(1 << 27)
#define BIT28	(1 << 28)
#define BIT29	(1 << 29)
#define BIT30	(1 << 30)
#define BIT31	(1 << 31)

#define MAX_CHECK_COUNT 200

#define ENABLE_WLAN_FUN(__WlanFunCtrl)\
{\
	__WlanFunCtrl.field.WLAN_CLK_EN = 1;\
	__WlanFunCtrl.field.WLAN_EN = 1;\
}

#define DISABLE_WLAN_FUN(__WlanFunCtrl)\
{\
	__WlanFunCtrl.field.PCIE_APP0_CLK_REQ = 0;\
	__WlanFunCtrl.field.WLAN_EN = 0;\
	__WlanFunCtrl.field.WLAN_CLK_EN = 0;\
}

static RTMP_REG_PAIR	MT76x0_MACRegTable[] = {
	{IOCFG_6,			0xA0040080},
	{PBF_SYS_CTRL,		0x80c00},
	{PBF_CFG,			0x77723c1f},
	{FCE_PSE_CTRL,		0x1},

	{AMPDU_MAX_LEN_20M1S,	0xAAA99887},

	{TX_SW_CFG0,		0x601}, /* Delay bb_tx_pe for proper tx_mcs_pwr update */
	{TX_SW_CFG1,		0x00040000}, /* Set rf_tx_pe deassert time to 1us by Chee's comment @MT7650_CR_setting_1018.xlsx */
	{TX_SW_CFG2,		0x0},

// TODO: shiang-6590, check what tx report will send to us when following default value set as 2
	{0xa44,					0x0}, /* disable Tx info report */


#ifdef HDR_TRANS_SUPPORT
	{HEADER_TRANS_CTRL_REG, 0x2}, /* 0x1: TX, 0x2: RX */
	{TSO_CTRL, 			0x7050},
#else
	{HEADER_TRANS_CTRL_REG, 0x0},
	{TSO_CTRL, 			0x0},
#endif /* HDR_TRANS_SUPPORT */


	/* BB_PA_MODE_CFG0(0x1214) Keep default value @20120903 */
	{BB_PA_MODE_CFG1, 0x00500055},

	/* RF_PA_MODE_CFG0(0x121C) Keep default value @20120903 */
	{RF_PA_MODE_CFG1, 0x00500055},

	{TX_ALC_CFG_0, 0x2F2F000C},
	{TX0_BB_GAIN_ATTEN, 0x00000000}, /* set BBP atten gain = 0 */

	{0x150C, 0x00000002}, /* Enable Tx length > 4095 byte */
	{0x1238, 0x001700C8}, /* Disable bt_abort_tx_en(0x1238[21] = 0) which is not used at MT7650 @MT7650_E3_CR_setting_1115.xlsx */
	{LDO_CTRL_0, 0x00A647B6}, /* PMU_OCLEVEL<5:1> from default <5'b10010> to <5'b11011> for normal driver */
	{LDO_CTRL1, 0x6B006464}, /* Default LDO_DIG supply 1.26V, change to 1.2V */
	{HT_BASIC_RATE, 0x00004003}, /*MT7650_E6_MAC_CR_setting_20140821.xlsx , fix RDG issue with 7628 */
	{HT_CTRL_CFG, 0x000001FF},	/*MT7650_E6_MAC_CR_setting_20140821.xlsx , fix RDG issue with 7628 */

	/* enable HW to autofallback to legacy rate to prevent ping fail in long range */
	{HT_FBK_TO_LEGACY, 0x00001818},
};

static UCHAR MT76x0_NUM_MAC_REG_PARMS = (sizeof(MT76x0_MACRegTable) / sizeof(RTMP_REG_PAIR));

static RTMP_REG_PAIR MT76x0_DCOC_Tab[] = {
	{CAL_R47, 0x000010F0},
	{CAL_R48, 0x00008080},
	{CAL_R49, 0x00000F07},
	{CAL_R50, 0x00000040},
	{CAL_R51, 0x00000404},
	{CAL_R52, 0x00080803},
	{CAL_R53, 0x00000704},
	{CAL_R54, 0x00002828},
	{CAL_R55, 0x00005050},
};
static UCHAR MT76x0_DCOC_Tab_Size = (sizeof(MT76x0_DCOC_Tab) / sizeof(RTMP_REG_PAIR));

static RTMP_REG_PAIR MT76x0_BBP_Init_Tab[] = {
	{CORE_R1, 0x00000002},
	{CORE_R4, 0x00000000},
	{CORE_R24, 0x00000000},
	{CORE_R32, 0x4003000a},
	{CORE_R42, 0x00000000},
	{CORE_R44, 0x00000000},

	{IBI_R11, 0x0FDE8081},
	
	/*
		0x2300[5] Default Antenna:
		0 for WIFI main antenna
		1  for WIFI aux  antenna

	*/
	{AGC1_R0, 0x00021400},
	
	{AGC1_R1, 0x00000003},
	{AGC1_R2, 0x003A6464},

	{AGC1_R15, 0x88A28CB8},
	{AGC1_R22, 0x00001E21},
	{AGC1_R23, 0x0000272C},
	{AGC1_R24, 0x00002F3A},
	{AGC1_R25, 0x8000005A},
	{AGC1_R26, 0x007C2005},
	{AGC1_R33, 0x00003238},
	{AGC1_R34, 0x000A0C0C},
	{AGC1_R37, 0x2121262C},
	{AGC1_R41, 0x38383E45},
	{AGC1_R57, 0x00001010},

	{AGC1_R59, 0xBAA20E96},
	{AGC1_R63, 0x00000001},

	{TXC_R0, 0x00280403},
	{TXC_R1, 0x00000000},

	{RXC_R1, 0x00000012},
	{RXC_R2, 0x00000011},
	{RXC_R3, 0x00000005},
	{RXC_R4, 0x00000000},
	{RXC_R5, 0xF977C4EC},
	{RXC_R7, 0x00000090},

	{TXO_R8, 0x00000000},

	{TXBE_R0, 0x00000000},
	{TXBE_R4, 0x00000004},
	{TXBE_R6, 0x00000000},
	{TXBE_R8, 0x00000014},
	{TXBE_R9, 0x20000000},
	{TXBE_R10, 0x00000000},
	{TXBE_R12, 0x00000000},
	{TXBE_R13, 0x00000000},
	{TXBE_R14, 0x00000000},
	{TXBE_R15, 0x00000000},
	{TXBE_R16, 0x00000000},
	{TXBE_R17, 0x00000000},

	{RXFE_R1, 0x00008800}, /* Add for E3 */
	{RXFE_R3, 0x00000000},
	{RXFE_R4, 0x00000000},

	{RXO_R13, 0x00000192},
	{RXO_R14, 0x00060612},
	{RXO_R15, 0xC8321B18},
	{RXO_R16, 0x0000001E},
	{RXO_R17, 0x00000000},
	{RXO_R18, 0xCC00A993},
	{RXO_R19, 0xB9CB9CB9},
	{RXO_R20, 0x26c00057},
	{RXO_R21, 0x00000001},
	{RXO_R24, 0x00000006},
	{RXO_R28, 0x0000003F},
};
static UCHAR MT76x0_BBP_Init_Tab_Size = (sizeof(MT76x0_BBP_Init_Tab) / sizeof(RTMP_REG_PAIR));

MT76x0_BBP_Table MT76x0_BPP_SWITCH_Tab[] = {
	
	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R4, 0x1FEDA049}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R4, 0x1FECA054}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R6, 0x00000045}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R6, 0x0000000A}},
	
	{RF_G_BAND | RF_BW_20 | RF_BW_40,				{AGC1_R8, 0x16344EF0}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R8, 0x122C54F2}},

	{RF_G_BAND | RF_BW_20,	{AGC1_R12, 0x05052879}},
	{RF_G_BAND | RF_BW_40,	{AGC1_R12, 0x050528F9}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R12, 0x050528F9}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,				{AGC1_R13, 0x35050004}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R13, 0x2C3A0406}},
	
	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R14, 0x310F2E3C}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R14, 0x310F2A3F}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R26, 0x007C2005}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R26, 0x007C2005}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R27, 0x000000E1}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R27, 0x000000EC}},

	{RF_G_BAND | RF_BW_20,	{AGC1_R28, 0x00060806}},
	{RF_G_BAND | RF_BW_40,	{AGC1_R28, 0x00050806}},
	{RF_A_BAND | RF_BW_40,	{AGC1_R28, 0x00060801}},
	{RF_A_BAND | RF_BW_20 | RF_BW_80,	{AGC1_R28, 0x00060806}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,				{AGC1_R31, 0x00000E23}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R31, 0x00000E13}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R32, 0x00003218}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R32, 0x0000181C}},

	{RF_G_BAND | RF_BW_20,	{AGC1_R35, 0x11111616}},
	{RF_G_BAND | RF_BW_40,	{AGC1_R35, 0x11111516}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R35, 0x11111111}},

	{RF_G_BAND | RF_BW_20,	{AGC1_R39, 0x2A2A3036}},
	{RF_G_BAND | RF_BW_40,	{AGC1_R39, 0x2A2A2C36}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,    {AGC1_R39, 0x2A2A2A2A}},
	{RF_G_BAND | RF_BW_20,	{AGC1_R43, 0x27273438}},
	{RF_G_BAND | RF_BW_40,	{AGC1_R43, 0x27272D38}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R43, 0x27271A1A}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R51, 0x17171C1C}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R51, 0xFFFFFFFF}},

	{RF_G_BAND | RF_BW_20,	{AGC1_R53, 0x26262A2F}},
	{RF_G_BAND | RF_BW_40,	{AGC1_R53, 0x2626322F}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R53, 0xFFFFFFFF}},

	{RF_G_BAND | RF_BW_20 | RF_BW_40,				{AGC1_R55, 0x40404040}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R55, 0xFFFFFFFF}},
	
	{RF_G_BAND | RF_BW_20 | RF_BW_40,	{AGC1_R58, 0x00001010}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{AGC1_R58, 0x00000000}},
	{RF_G_BAND | RF_BW_20 | RF_BW_40,				{RXFE_R0, 0x3D5000E0}},
	{RF_A_BAND | RF_BW_20 | RF_BW_40 | RF_BW_80,	{RXFE_R0, 0x895000E0}},
};
UCHAR MT76x0_BPP_SWITCH_Tab_Size = (sizeof(MT76x0_BPP_SWITCH_Tab) / sizeof(MT76x0_BBP_Table));

/* Bank	Register Value(Hex) */
static BANK_RF_REG_PAIR MT76x0_RF_Central_RegTb[] = {
/*
	Bank 0 - For central blocks: BG, PLL, XTAL, LO, ADC/DAC
*/
	{RF_BANK0,	RF_R01, 0x01},
	{RF_BANK0,	RF_R02, 0x11},

	/*
		R3 ~ R7: VCO Cal.
	*/	
	{RF_BANK0,	RF_R03, 0x63}, /* VCO Freq Cal - No Bypass, VCO Amp Cal - No Bypass */
	{RF_BANK0,	RF_R04, 0x30}, /* R4 b<7>=1, VCO cal */
	{RF_BANK0,	RF_R05, 0x00},
	{RF_BANK0,	RF_R06, 0x41}, /* Set the open loop amplitude to middle since bypassing amplitude calibration */
	{RF_BANK0,	RF_R07, 0x00},

	/*
		XO
	*/
	{RF_BANK0,	RF_R08, 0x00}, 
	{RF_BANK0,	RF_R09, 0x00},
	{RF_BANK0,	RF_R10, 0x0C},
	{RF_BANK0,	RF_R11, 0x00},
	{RF_BANK0,	RF_R12, 0x00},

	/*
		BG
	*/
	{RF_BANK0,	RF_R13, 0x00},
	{RF_BANK0,	RF_R14, 0x00},
	{RF_BANK0,	RF_R15, 0x00},

	/*
		LDO
	*/
	{RF_BANK0,	RF_R19, 0x20}, 
	/*
		XO
	*/
	{RF_BANK0,	RF_R20, 0x22},
	{RF_BANK0,	RF_R21, 0x10},
	{RF_BANK0,	RF_R23, 0x00},
	{RF_BANK0,	RF_R24, 0x33}, /* See band selection for R24<1:0> */
	{RF_BANK0,	RF_R25, 0x00},

	/*
		PLL, See Freq Selection
	*/
	{RF_BANK0,	RF_R26, 0x00},
	{RF_BANK0,	RF_R27, 0x00},
	{RF_BANK0,	RF_R28, 0x00},
	{RF_BANK0,	RF_R29, 0x00},
	{RF_BANK0,	RF_R30, 0x00},
	{RF_BANK0,	RF_R31, 0x00},
	{RF_BANK0,	RF_R32, 0x00},
	{RF_BANK0,	RF_R33, 0x00},
	{RF_BANK0,	RF_R34, 0x00},
	{RF_BANK0,	RF_R35, 0x00},
	{RF_BANK0,	RF_R36, 0x00},
	{RF_BANK0,	RF_R37, 0x00},

	/*
		LO Buffer
	*/
	{RF_BANK0,	RF_R38, 0x2F},
	
	/*
		Test Ports
	*/
	{RF_BANK0,	RF_R64, 0x00},
	{RF_BANK0,	RF_R65, 0x80},
	{RF_BANK0,	RF_R66, 0x01},
	{RF_BANK0,	RF_R67, 0x04},

	/*
		ADC/DAC
	*/
	{RF_BANK0,	RF_R68, 0x00},
	{RF_BANK0,	RF_R69, 0x08},
	{RF_BANK0,	RF_R70, 0x08},
	{RF_BANK0,	RF_R71, 0x40},
	{RF_BANK0,	RF_R72, 0xD0},
	{RF_BANK0,	RF_R73, 0x93},
};
static UINT32 MT76x0_RF_Central_RegTb_Size = (sizeof(MT76x0_RF_Central_RegTb) / sizeof(BANK_RF_REG_PAIR));

static BANK_RF_REG_PAIR MT76x0_RF_2G_Channel_0_RegTb[] = {
/*
	Bank 5 - Channel 0 2G RF registers	
*/
	/*
		RX logic operation
	*/
	/* RF_R00 Change in SelectBand6590 */

	{RF_BANK5,	RF_R02, 0x00}, /* 0x1D: 5G+2G+BT(MT7650E),  0x0C: 5G+2G(MT7610U), 0x00: 5G only (MT7610E) */
	{RF_BANK5,	RF_R03, 0x00},

	/*
		TX logic operation
	*/
	{RF_BANK5,	RF_R04, 0x00},
	{RF_BANK5,	RF_R05, 0x84},
	{RF_BANK5,	RF_R06, 0x02},

	/*
		LDO
	*/
	{RF_BANK5,	RF_R07, 0x00},
	{RF_BANK5,	RF_R08, 0x00},
	{RF_BANK5,	RF_R09, 0x00},

	/*
		RX
	*/
	{RF_BANK5,	RF_R10, 0x51},
	{RF_BANK5,	RF_R11, 0x22},
	{RF_BANK5,	RF_R12, 0x22},
	{RF_BANK5,	RF_R13, 0x0F},
	{RF_BANK5,	RF_R14, 0x47}, /* Increase mixer current for more gain */
	{RF_BANK5,	RF_R15, 0x25},
	{RF_BANK5,	RF_R16, 0xC7}, /* Tune LNA2 tank */
	{RF_BANK5,	RF_R17, 0x00},
	{RF_BANK5,	RF_R18, 0x00},
	{RF_BANK5,	RF_R19, 0x30}, /* Improve max Pin */
	{RF_BANK5,	RF_R20, 0x33},
	{RF_BANK5,	RF_R21, 0x02},
	{RF_BANK5,	RF_R22, 0x32}, /* Tune LNA1 tank */
	{RF_BANK5,	RF_R23, 0x00},
	{RF_BANK5,	RF_R24, 0x25},
	{RF_BANK5,	RF_R26, 0x00},
	{RF_BANK5,	RF_R27, 0x12},
	{RF_BANK5,	RF_R28, 0x0F},
	{RF_BANK5,	RF_R29, 0x00},

	/*
		LOGEN
	*/
	{RF_BANK5,	RF_R30, 0x51}, /* Tune LOGEN tank */
	{RF_BANK5,	RF_R31, 0x35},
	{RF_BANK5,	RF_R32, 0x31},
	{RF_BANK5,	RF_R33, 0x31},
	{RF_BANK5,	RF_R34, 0x34},
	{RF_BANK5,	RF_R35, 0x03},
	{RF_BANK5,	RF_R36, 0x00},

	/*
		TX
	*/
	{RF_BANK5,	RF_R37, 0xDD}, /* Improve 3.2GHz spur */
	{RF_BANK5,	RF_R38, 0xB3},
	{RF_BANK5,	RF_R39, 0x33},
	{RF_BANK5,	RF_R40, 0xB1},
	{RF_BANK5,	RF_R41, 0x71},
	{RF_BANK5,	RF_R42, 0xF2},
	{RF_BANK5,	RF_R43, 0x47},
	{RF_BANK5,	RF_R44, 0x77},
	{RF_BANK5,	RF_R45, 0x0E},
	{RF_BANK5,	RF_R46, 0x10},
	{RF_BANK5,	RF_R47, 0x00},
	{RF_BANK5,	RF_R48, 0x53},
	{RF_BANK5,	RF_R49, 0x03},
	{RF_BANK5,	RF_R50, 0xEF},
	{RF_BANK5,	RF_R51, 0xC7},
	{RF_BANK5,	RF_R52, 0x62},
	{RF_BANK5,	RF_R53, 0x62},
	{RF_BANK5,	RF_R54, 0x00},
	{RF_BANK5,	RF_R55, 0x00},
	{RF_BANK5,	RF_R56, 0x0F},
	{RF_BANK5,	RF_R57, 0x0F},
	{RF_BANK5,	RF_R58, 0x16},
	{RF_BANK5,	RF_R59, 0x16},
	{RF_BANK5,	RF_R60, 0x10},
	{RF_BANK5,	RF_R61, 0x10},
	{RF_BANK5,	RF_R62, 0xD0},
	{RF_BANK5,	RF_R63, 0x6C},
	{RF_BANK5,	RF_R64, 0x58},
	{RF_BANK5, 	RF_R65, 0x58},
	{RF_BANK5,	RF_R66, 0xF2},
	{RF_BANK5,	RF_R67, 0xE8},
	{RF_BANK5,	RF_R68, 0xF0},
	{RF_BANK5,	RF_R69, 0xF0},
	{RF_BANK5,	RF_R127, 0x04},
};
static UINT32 MT76x0_RF_2G_Channel_0_RegTb_Size = (sizeof(MT76x0_RF_2G_Channel_0_RegTb) / sizeof(BANK_RF_REG_PAIR));

static BANK_RF_REG_PAIR MT76x0_RF_5G_Channel_0_RegTb[] = {
/*
	Bank 6 - Channel 0 5G RF registers	
*/
	/*
		RX logic operation
	*/
	/* RF_R00 Change in SelectBandMT76x0 */

	{RF_BANK6,	RF_R02, 0x0C},
	{RF_BANK6,	RF_R03, 0x00},

	/*
		TX logic operation
	*/
	{RF_BANK6,	RF_R04, 0x00},
	{RF_BANK6,	RF_R05, 0x84},
	{RF_BANK6,	RF_R06, 0x02},

	/*
		LDO
	*/
	{RF_BANK6,	RF_R07, 0x00},
	{RF_BANK6,	RF_R08, 0x00},
	{RF_BANK6,	RF_R09, 0x00},

	/*
		RX
	*/
	{RF_BANK6,	RF_R10, 0x00},
	{RF_BANK6,	RF_R11, 0x01},
	
	{RF_BANK6,	RF_R13, 0x23},
	{RF_BANK6,	RF_R14, 0x00},
	{RF_BANK6,	RF_R15, 0x04},
	{RF_BANK6,	RF_R16, 0x22},

	{RF_BANK6,	RF_R18, 0x08},
	{RF_BANK6,	RF_R19, 0x00},
	{RF_BANK6,	RF_R20, 0x00},
	{RF_BANK6,	RF_R21, 0x00},
	{RF_BANK6,	RF_R22, 0xFB},

	/*
		LOGEN5G
	*/
	{RF_BANK6,	RF_R25, 0x76},
	{RF_BANK6,	RF_R26, 0x24},
	{RF_BANK6,	RF_R27, 0x04},
	{RF_BANK6,	RF_R28, 0x00},
	{RF_BANK6,	RF_R29, 0x00},

	/*
		TX
	*/
	{RF_BANK6,	RF_R37, 0xBB},
	{RF_BANK6,	RF_R38, 0xB3},

	{RF_BANK6,	RF_R40, 0x33},
	{RF_BANK6,	RF_R41, 0x33},
	
	{RF_BANK6,	RF_R43, 0x03},
	{RF_BANK6,	RF_R44, 0xB3},
	
	{RF_BANK6,	RF_R46, 0x17},
	{RF_BANK6,	RF_R47, 0x0E},
	{RF_BANK6,	RF_R48, 0x10},
	{RF_BANK6,	RF_R49, 0x07},
	
	{RF_BANK6,	RF_R62, 0x00},
	{RF_BANK6,	RF_R63, 0x00},
	{RF_BANK6,	RF_R64, 0xF1},
	{RF_BANK6,	RF_R65, 0x0F},
};
static UINT32 MT76x0_RF_5G_Channel_0_RegTb_Size = (sizeof(MT76x0_RF_5G_Channel_0_RegTb) / sizeof(BANK_RF_REG_PAIR));

static BANK_RF_REG_PAIR MT76x0_RF_VGA_Channel_0_RegTb[] = {
/*
	Bank 7 - Channel 0 VGA RF registers	
*/
	/* E3 CR */
	{RF_BANK7,	RF_R00, 0x47}, /* Allow BBP/MAC to do calibration */
	{RF_BANK7,	RF_R01, 0x00},
	{RF_BANK7,	RF_R02, 0x00},
	{RF_BANK7,	RF_R03, 0x00},
	{RF_BANK7,	RF_R04, 0x00},

	{RF_BANK7,	RF_R10, 0x13},
	{RF_BANK7,	RF_R11, 0x0F},
	{RF_BANK7,	RF_R12, 0x13}, /* For DCOC */
	{RF_BANK7,	RF_R13, 0x13}, /* For DCOC */
	{RF_BANK7,	RF_R14, 0x13}, /* For DCOC */
	{RF_BANK7,	RF_R15, 0x20}, /* For DCOC */
	{RF_BANK7,	RF_R16, 0x22}, /* For DCOC */

	{RF_BANK7,	RF_R17, 0x7C},

	{RF_BANK7,	RF_R18, 0x00},
	{RF_BANK7,	RF_R19, 0x00},
	{RF_BANK7,	RF_R20, 0x00},
	{RF_BANK7,	RF_R21, 0xF1},
	{RF_BANK7,	RF_R22, 0x11},
	{RF_BANK7,	RF_R23, 0xC2},
	{RF_BANK7,	RF_R24, 0x41},
	{RF_BANK7,	RF_R25, 0x20},
	{RF_BANK7,	RF_R26, 0x40},
	{RF_BANK7,	RF_R27, 0xD7},
	{RF_BANK7,	RF_R28, 0xA2},
	{RF_BANK7,	RF_R29, 0x60},
	{RF_BANK7,	RF_R30, 0x49},
	{RF_BANK7,	RF_R31, 0x20},
	{RF_BANK7,	RF_R32, 0x44},
	{RF_BANK7,	RF_R33, 0xC1},
	{RF_BANK7,	RF_R34, 0x60},
	{RF_BANK7,	RF_R35, 0xC0},

	{RF_BANK7,	RF_R61, 0x01},

	{RF_BANK7,	RF_R72, 0x3C},
	{RF_BANK7,	RF_R73, 0x34},
	{RF_BANK7,	RF_R74, 0x00},
};
static UINT32 MT76x0_RF_VGA_Channel_0_RegTb_Size = (sizeof(MT76x0_RF_VGA_Channel_0_RegTb) / sizeof(BANK_RF_REG_PAIR));

static MT76x0_FREQ_ITEM MT76x0_Frequency_Plan[] =
{
	{1,		RF_G_BAND,	0x02, 0x3F, 0x28, 0xDD, 0xE2, 0x40, 0x02, 0x40, 0x02, 0, 0, 1, 0x28, 0, 0x30, 0, 0, 0x3}, /* Freq 2412 */
	{2, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xE4, 0x40, 0x07, 0x40, 0x02, 0, 0, 1, 0xA1, 0, 0x30, 0, 0, 0x1}, /* Freq 2417 */
	{3, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xE2, 0x40, 0x07, 0x40, 0x0B, 0, 0, 1, 0x50, 0, 0x30, 0, 0, 0x0}, /* Freq 2422 */
	{4, 	RF_G_BAND,	0x02, 0x3F, 0x28, 0xDD, 0xD4, 0x40, 0x02, 0x40, 0x09, 0, 0, 1, 0x50, 0, 0x30, 0, 0, 0x0}, /* Freq 2427 */
	{5, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xD4, 0x40, 0x07, 0x40, 0x02, 0, 0, 1, 0xA2, 0, 0x30, 0, 0, 0x1}, /* Freq 2432 */
	{6, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xD4, 0x40, 0x07, 0x40, 0x07, 0, 0, 1, 0xA2, 0, 0x30, 0, 0, 0x1}, /* Freq 2437 */
	{7, 	RF_G_BAND,	0x02, 0x3F, 0x28, 0xDD, 0xE2, 0x40, 0x02, 0x40, 0x07, 0, 0, 1, 0x28, 0, 0x30, 0, 0, 0x3}, /* Freq 2442 */
	{8, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xD4, 0x40, 0x07, 0x40, 0x02, 0, 0, 1, 0xA3, 0, 0x30, 0, 0, 0x1}, /* Freq 2447 */
	{9, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xF2, 0x40, 0x07, 0x40, 0x0D, 0, 0, 1, 0x28, 0, 0x30, 0, 0, 0x3}, /* Freq 2452 */
	{10, 	RF_G_BAND,	0x02, 0x3F, 0x28, 0xDD, 0xD4, 0x40, 0x02, 0x40, 0x09, 0, 0, 1, 0x51, 0, 0x30, 0, 0, 0x0}, /* Freq 2457 */
	{11, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xD4, 0x40, 0x07, 0x40, 0x02, 0, 0, 1, 0xA4, 0, 0x30, 0, 0, 0x1}, /* Freq 2462 */
	{12, 	RF_G_BAND,	0x02, 0x3F, 0x3C, 0xDD, 0xD4, 0x40, 0x07, 0x40, 0x07, 0, 0, 1, 0xA4, 0, 0x30, 0, 0, 0x1}, /* Freq 2467 */
	{13, 	RF_G_BAND,	0x02, 0x3F, 0x28, 0xDD, 0xF2, 0x40, 0x02, 0x40, 0x02, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 2472 */
	{14, 	RF_G_BAND,	0x02, 0x3F, 0x28, 0xDD, 0xF2, 0x40, 0x02, 0x40, 0x04, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 2484 */

	{183, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x70, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x17, 0, 0, 1, 0x28, 0, 0x30, 0, 0, 0x3}, /* Freq 4915 */
	{184, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x00, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 4920 */
	{185, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x01, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 4925 */
	{187, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x03, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 4935 */
	{188, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x02, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 4940 */
	{189, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x05, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 4945 */
	{192, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x04, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 4960 */
	{196, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x06, 0, 0, 1, 0x29, 0, 0x30, 0, 0, 0x3}, /* Freq 4980 */
	
	{36, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x02, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5180 */
	{37, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x05, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5185 */
	{38, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x03, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5190 */
	{39, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x07, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5195 */
	{40, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x04, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5200 */
	{41, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x09, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5205 */
	{42, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x05, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5210 */
	{43, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0B, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5215 */
	{44, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x06, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5220 */
	{45, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0D, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5225 */
	{46, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x07, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5230 */
	{47, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0F, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5235 */
	{48, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x08, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5240 */
	{49, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x11, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5245 */
	{50, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x09, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5250 */
	{51, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x13, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5255 */
	{52, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x0A, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5260 */
	{53, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x15, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5265 */
	{54, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x0B, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5270 */
	{55, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x70, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x17, 0, 0, 1, 0x2B, 0, 0x30, 0, 0, 0x3}, /* Freq 5275 */
	{56, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x00, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5280 */
	{57, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x01, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5285 */
	{58, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x01, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5290 */
	{59, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x03, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5295 */
	{60, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x02, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5300 */
	{61, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x05, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5305 */
	{62, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x03, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5310 */
	{63, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x07, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5315 */
	{64, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x04, 0, 0, 1, 0x2C, 0, 0x30, 0, 0, 0x3}, /* Freq 5320 */

	{100, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x0A, 0, 0, 1, 0x2D, 0, 0x30, 0, 0, 0x3}, /* Freq 5500 */
	{101, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x15, 0, 0, 1, 0x2D, 0, 0x30, 0, 0, 0x3}, /* Freq 5505 */
	{102, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x0B, 0, 0, 1, 0x2D, 0, 0x30, 0, 0, 0x3}, /* Freq 5510 */
	{103, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x70, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x17, 0, 0, 1, 0x2D, 0, 0x30, 0, 0, 0x3}, /* Freq 5515 */
	{104, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x00, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5520 */
	{105, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x01, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5525 */
	{106, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x01, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5530 */
	{107, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x03, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5535 */
	{108, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x02, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5540 */
	{109, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x05, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5545 */
	{110, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x03, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5550 */
	{111, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x07, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5555 */
	{112, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x04, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5560 */
	{113, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x09, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5565 */
	{114, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x05, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5570 */
	{115, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0B, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5575 */
	{116, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x06, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5580 */
	{117, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0D, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5585 */
	{118, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x07, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5590 */
	{119, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0F, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5595 */
	{120, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x08, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5600 */
	{121, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x11, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5605 */
	{122, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x09, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5610 */
	{123, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x13, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5615 */
	{124, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x0A, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5620 */
	{125, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x15, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5625 */
	{126, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x0B, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5630 */
	{127, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x70, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x17, 0, 0, 1, 0x2E, 0, 0x30, 0, 0, 0x3}, /* Freq 5635 */
	{128, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x00, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5640 */
	{129, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x01, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5645 */
	{130, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x01, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5650 */
	{131, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x03, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5655 */
	{132, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x02, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5660 */
	{133, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x05, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5665 */
	{134, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x03, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5670 */
	{135, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x07, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5675 */
	{136, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x04, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5680 */

	{137, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x09, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5685 */
	{138, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x05, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5690 */
	{139, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0B, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5695 */
	{140, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x06, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5700 */
	{141, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0D, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5705 */
	{142, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x07, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5710 */	
	{143, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0F, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5715 */
	{144, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x08, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5720 */
	{145, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x11, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5725 */
	{146, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x09, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5730 */
	{147, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x13, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5735 */
	{148, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x0A, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5740 */
	{149, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x15, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5745 */
	{150, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x0B, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5750 */	
	{151, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x70, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x17, 0, 0, 1, 0x2F, 0, 0x30, 0, 0, 0x3}, /* Freq 5755 */
	{152, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x00, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5760 */
	{153, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x01, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5765 */
	{154, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x01, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5770 */
	{155, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x03, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5775 */
	{156, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x02, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5780 */
	{157, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x05, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5785 */
	{158, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x03, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5790 */
	{159, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x07, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5795 */
	{160, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x04, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5800 */
	{161, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x09, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5805 */
	{162, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x05, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5810 */
	{163, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0B, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5815 */
	{164, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x06, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5820 */
	{165, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0D, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5825 */
	{166, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0xDD, 0xD2, 0x40, 0x04, 0x40, 0x07, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5830 */
	{167, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x0F, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5835 */
	{168, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x08, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5840 */
	{169, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x11, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5845 */
	{170, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x09, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5850 */
	{171, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x13, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5855 */
	{172, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x30, 0x97, 0xD2, 0x40, 0x04, 0x40, 0x0A, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5860 */
	{173, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x68, 0xDD, 0xD2, 0x40, 0x10, 0x40, 0x15, 0, 0, 1, 0x30, 0, 0x30, 0, 0, 0x3}, /* Freq 5865 */
};
UCHAR NUM_OF_MT76x0_CHNL = (sizeof(MT76x0_Frequency_Plan) / sizeof(MT76x0_FREQ_ITEM));


static MT76x0_FREQ_ITEM MT76x0_SDM_Frequency_Plan[] =
{
	{1,		RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0xCCCC,  0x3}, /* Freq 2412 */
	{2, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x12222, 0x3}, /* Freq 2417 */
	{3, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x17777, 0x3}, /* Freq 2422 */
	{4, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x1CCCC, 0x3}, /* Freq 2427 */
	{5, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x22222, 0x3}, /* Freq 2432 */
	{6, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x27777, 0x3}, /* Freq 2437 */
	{7, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x2CCCC, 0x3}, /* Freq 2442 */
	{8, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x32222, 0x3}, /* Freq 2447 */
	{9, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x37777, 0x3}, /* Freq 2452 */
	{10, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x3CCCC, 0x3}, /* Freq 2457 */
	{11, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x2222, 0x3}, /* Freq 2462 */
	{12, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x7777, 0x3}, /* Freq 2467 */
	{13, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0xCCCC, 0x3}, /* Freq 2472 */
	{14, 	RF_G_BAND,	0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x19999, 0x3}, /* Freq 2484 */

	{183, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x28, 0, 0x0, 0x8, 0x3D555, 0x3}, /* Freq 4915 */
	{184, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x0,     0x3}, /* Freq 4920 */
	{185, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x2AAA,  0x3}, /* Freq 4925 */
	{187, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x8000,  0x3}, /* Freq 4935 */
	{188, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0xAAAA,  0x3}, /* Freq 4940 */
	{189, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0xD555,  0x3}, /* Freq 4945 */
	{192, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x15555, 0x3}, /* Freq 4960 */
	{196, 	(RF_A_BAND | RF_A_BAND_11J), 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x29, 0, 0x0, 0x8, 0x20000, 0x3}, /* Freq 4980 */
	
	{36, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0xAAAA,  0x3}, /* Freq 5180 */
	{37, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0xD555,  0x3}, /* Freq 5185 */
	{38, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x10000, 0x3}, /* Freq 5190 */
	{39, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x12AAA, 0x3}, /* Freq 5195 */
	{40, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x15555, 0x3}, /* Freq 5200 */
	{41, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x18000, 0x3}, /* Freq 5205 */
	{42, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x1AAAA, 0x3}, /* Freq 5210 */
	{43, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x1D555, 0x3}, /* Freq 5215 */
	{44, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x20000, 0x3}, /* Freq 5220 */
	{45, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x22AAA, 0x3}, /* Freq 5225 */
	{46, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x25555, 0x3}, /* Freq 5230 */
	{47, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x28000, 0x3}, /* Freq 5235 */
	{48, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x2AAAA, 0x3}, /* Freq 5240 */
	{49, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x2D555, 0x3}, /* Freq 5245 */
	{50, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x30000, 0x3}, /* Freq 5250 */
	{51, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x32AAA, 0x3}, /* Freq 5255 */
	{52, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x35555, 0x3}, /* Freq 5260 */
	{53, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x38000, 0x3}, /* Freq 5265 */
	{54, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x3AAAA, 0x3}, /* Freq 5270 */
	{55, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2B, 0, 0x0, 0x8, 0x3D555, 0x3}, /* Freq 5275 */
	{56, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x00000, 0x3}, /* Freq 5280 */
	{57, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x02AAA, 0x3}, /* Freq 5285 */
	{58, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x05555, 0x3}, /* Freq 5290 */
	{59, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x08000, 0x3}, /* Freq 5295 */
	{60, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x0AAAA, 0x3}, /* Freq 5300 */
	{61, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x0D555, 0x3}, /* Freq 5305 */
	{62, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x10000, 0x3}, /* Freq 5310 */
	{63, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x12AAA, 0x3}, /* Freq 5315 */
	{64, 	(RF_A_BAND | RF_A_BAND_LB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2C, 0, 0x0, 0x8, 0x15555, 0x3}, /* Freq 5320 */

	{100, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2D, 0, 0x0, 0x8, 0x35555, 0x3}, /* Freq 5500 */
	{101, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2D, 0, 0x0, 0x8, 0x38000, 0x3}, /* Freq 5505 */
	{102, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2D, 0, 0x0, 0x8, 0x3AAAA, 0x3}, /* Freq 5510 */
	{103, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2D, 0, 0x0, 0x8, 0x3D555, 0x3}, /* Freq 5515 */
	{104, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x00000, 0x3}, /* Freq 5520 */
	{105, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x02AAA, 0x3}, /* Freq 5525 */
	{106, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x05555, 0x3}, /* Freq 5530 */
	{107, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x08000, 0x3}, /* Freq 5535 */
	{108, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x0AAAA, 0x3}, /* Freq 5540 */
	{109, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x0D555, 0x3}, /* Freq 5545 */
	{110, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x10000, 0x3}, /* Freq 5550 */
	{111, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x12AAA, 0x3}, /* Freq 5555 */
	{112, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x15555, 0x3}, /* Freq 5560 */
	{113, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x18000, 0x3}, /* Freq 5565 */
	{114, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x1AAAA, 0x3}, /* Freq 5570 */
	{115, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x1D555, 0x3}, /* Freq 5575 */
	{116, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x20000, 0x3}, /* Freq 5580 */
	{117, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x22AAA, 0x3}, /* Freq 5585 */
	{118, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x25555, 0x3}, /* Freq 5590 */
	{119, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x28000, 0x3}, /* Freq 5595 */
	{120, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x2AAAA, 0x3}, /* Freq 5600 */
	{121, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x2D555, 0x3}, /* Freq 5605 */
	{122, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x30000, 0x3}, /* Freq 5610 */
	{123, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x32AAA, 0x3}, /* Freq 5615 */
	{124, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x35555, 0x3}, /* Freq 5620 */
	{125, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x38000, 0x3}, /* Freq 5625 */
	{126, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x3AAAA, 0x3}, /* Freq 5630 */
	{127, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2E, 0, 0x0, 0x8, 0x3D555, 0x3}, /* Freq 5635 */
	{128, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x00000, 0x3}, /* Freq 5640 */
	{129, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x02AAA, 0x3}, /* Freq 5645 */
	{130, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x05555, 0x3}, /* Freq 5650 */
	{131, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x08000, 0x3}, /* Freq 5655 */
	{132, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x0AAAA, 0x3}, /* Freq 5660 */
	{133, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x0D555, 0x3}, /* Freq 5665 */
	{134, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x10000, 0x3}, /* Freq 5670 */
	{135, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x12AAA, 0x3}, /* Freq 5675 */
	{136, 	(RF_A_BAND | RF_A_BAND_MB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x15555, 0x3}, /* Freq 5680 */

	{137, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x18000, 0x3}, /* Freq 5685 */
	{138, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x1AAAA, 0x3}, /* Freq 5690 */
	{139, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x1D555, 0x3}, /* Freq 5695 */
	{140, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x20000, 0x3}, /* Freq 5700 */
	{141, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x22AAA, 0x3}, /* Freq 5705 */
	{142, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x25555, 0x3}, /* Freq 5710 */	
	{143, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x28000, 0x3}, /* Freq 5715 */
	{144, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x2AAAA, 0x3}, /* Freq 5720 */
	{145, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x2D555, 0x3}, /* Freq 5725 */
	{146, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x30000, 0x3}, /* Freq 5730 */
	{147, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x32AAA, 0x3}, /* Freq 5735 */
	{148, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x35555, 0x3}, /* Freq 5740 */
	{149, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x38000, 0x3}, /* Freq 5745 */
	{150, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x3AAAA, 0x3}, /* Freq 5750 */	
	{151, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x2F, 0, 0x0, 0x8, 0x3D555, 0x3}, /* Freq 5755 */
	{152, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x00000, 0x3}, /* Freq 5760 */
	{153, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x02AAA, 0x3}, /* Freq 5765 */
	{154, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x05555, 0x3}, /* Freq 5770 */
	{155, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x08000, 0x3}, /* Freq 5775 */
	{156, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x0AAAA, 0x3}, /* Freq 5780 */
	{157, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x0D555, 0x3}, /* Freq 5785 */
	{158, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x10000, 0x3}, /* Freq 5790 */
	{159, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x12AAA, 0x3}, /* Freq 5795 */
	{160, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x15555, 0x3}, /* Freq 5800 */
	{161, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x18000, 0x3}, /* Freq 5805 */
	{162, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x1AAAA, 0x3}, /* Freq 5810 */
	{163, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x1D555, 0x3}, /* Freq 5815 */
	{164, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x20000, 0x3}, /* Freq 5820 */
	{165, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x22AAA, 0x3}, /* Freq 5825 */
	{166, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x25555, 0x3}, /* Freq 5830 */
	{167, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x28000, 0x3}, /* Freq 5835 */
	{168, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x2AAAA, 0x3}, /* Freq 5840 */
	{169, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x2D555, 0x3}, /* Freq 5845 */
	{170, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x30000, 0x3}, /* Freq 5850 */
	{171, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x32AAA, 0x3}, /* Freq 5855 */
	{172, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x35555, 0x3}, /* Freq 5860 */
	{173, 	(RF_A_BAND | RF_A_BAND_HB),	 0x02, 0x3F, 0x7F, 0xDD, 0xC3, 0x40, 0x0, 0x80, 0x0, 0/*0 -> 1*/, 0, 0, 0x30, 0, 0x0, 0x8, 0x38000, 0x3}, /* Freq 5865 */
};
UCHAR NUM_OF_MT76x0_SDM_CHNL = (sizeof(MT76x0_SDM_Frequency_Plan) / sizeof(MT76x0_FREQ_ITEM));

static UINT8 MT76x0_SDM_Channel[] = {
	183, 185, 43, 45, 54, 55, 57, 58, 102, 103, 105, 106, 115, 117, 126, 127, 129, 130, 139, 141, 150, 151, 153, 154, 163, 165
};
static UCHAR MT76x0_SDM_Channel_Size = (sizeof(MT76x0_SDM_Channel) / sizeof(UINT8));

static const MT76x0_RF_SWITCH_ITEM MT76x0_RF_BW_Switch[] =
{
	/*   Bank, 		Register,	Bw/Band, 	Value */
		{RF_BANK0,	RF_R17,		RF_G_BAND | BW_20,	0x00},
		{RF_BANK0,	RF_R17,		RF_G_BAND | BW_40,	0x00},
		{RF_BANK0,	RF_R17,		RF_A_BAND | BW_20,	0x00},
		{RF_BANK0,	RF_R17,		RF_A_BAND | BW_40,	0x00},
		{RF_BANK0,	RF_R17,		RF_A_BAND | BW_80,	0x00},

		// TODO: need to check B7.R6 & B7.R7 setting for 2.4G again @20121112
		{RF_BANK7,	RF_R06,		RF_G_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R06,		RF_G_BAND | BW_40,	0x1C},
		{RF_BANK7,	RF_R06,		RF_A_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R06,		RF_A_BAND | BW_40,	0x20},
		{RF_BANK7,	RF_R06,		RF_A_BAND | BW_80,	0x10},

		{RF_BANK7,	RF_R07,		RF_G_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R07,		RF_G_BAND | BW_40,	0x20},
		{RF_BANK7,	RF_R07,		RF_A_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R07,		RF_A_BAND | BW_40,	0x20},
		{RF_BANK7,	RF_R07,		RF_A_BAND | BW_80,	0x10},

		{RF_BANK7,	RF_R08,		RF_G_BAND | BW_20,	0x03},
		{RF_BANK7,	RF_R08,		RF_G_BAND | BW_40,	0x01},
		{RF_BANK7,	RF_R08,		RF_A_BAND | BW_20,	0x03},
		{RF_BANK7,	RF_R08,		RF_A_BAND | BW_40,	0x01},
		{RF_BANK7,	RF_R08,		RF_A_BAND | BW_80,	0x00},

		// TODO: need to check B7.R58 & B7.R59 setting for 2.4G again @20121112
		{RF_BANK7,	RF_R58,		RF_G_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R58,		RF_G_BAND | BW_40,	0x40},
		
		{RF_BANK7,	RF_R58,		RF_A_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R58,		RF_A_BAND | BW_40,	0x40},
		{RF_BANK7,	RF_R58,		RF_A_BAND | BW_80,	0x10},

		{RF_BANK7,	RF_R59,		RF_G_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R59,		RF_G_BAND | BW_40,	0x40},
		
		{RF_BANK7,	RF_R59,		RF_A_BAND | BW_20,	0x40},
		{RF_BANK7,	RF_R59,		RF_A_BAND | BW_40,	0x40},
		{RF_BANK7,	RF_R59,		RF_A_BAND | BW_80,	0x10},

		{RF_BANK7,	RF_R60,		RF_G_BAND | BW_20,	0xAA},
		{RF_BANK7,	RF_R60,		RF_G_BAND | BW_40,	0xAA},
		{RF_BANK7,	RF_R60,		RF_A_BAND | BW_20,	0xAA},
		{RF_BANK7,	RF_R60,		RF_A_BAND | BW_40,	0xAA},
		{RF_BANK7,	RF_R60,		RF_A_BAND | BW_80,	0xAA},

		{RF_BANK7,	RF_R76,		BW_20,	0x40},
		{RF_BANK7,	RF_R76,		BW_40,	0x40},
		{RF_BANK7,	RF_R76,		BW_80,	0x10},

		{RF_BANK7,	RF_R77,		BW_20,	0x40},
		{RF_BANK7,	RF_R77,		BW_40,	0x40},
		{RF_BANK7,	RF_R77,		BW_80,	0x10},
};
UCHAR MT76x0_RF_BW_Switch_Size = (sizeof(MT76x0_RF_BW_Switch) / sizeof(MT76x0_RF_SWITCH_ITEM));

static const MT76x0_RF_SWITCH_ITEM MT76x0_RF_Band_Switch[] =
{
	/*   Bank, 		Register,	Bw/Band, 		Value */
		{RF_BANK0,	RF_R16,		RF_G_BAND,		0x20},
		{RF_BANK0,	RF_R16,		RF_A_BAND,		0x20},
		
		{RF_BANK0,	RF_R18,		RF_G_BAND,		0x00},
		{RF_BANK0,	RF_R18,		RF_A_BAND,		0x00},

		{RF_BANK0,	RF_R39,		RF_G_BAND,		0x36},
		{RF_BANK0,	RF_R39,		RF_A_BAND_LB,	0x34},
		{RF_BANK0,	RF_R39,		RF_A_BAND_MB,	0x33},
		{RF_BANK0,	RF_R39,		RF_A_BAND_HB,	0x31},
		{RF_BANK0,	RF_R39,		RF_A_BAND_11J,	0x36},

		{RF_BANK6,	RF_R12,		RF_A_BAND_LB,	0x44},
		{RF_BANK6,	RF_R12,		RF_A_BAND_MB,	0x44},
		{RF_BANK6,	RF_R12,		RF_A_BAND_HB,	0x55},
		{RF_BANK6,	RF_R12,		RF_A_BAND_11J,	0x44},

		{RF_BANK6,	RF_R17,		RF_A_BAND_LB,	0x02},
		{RF_BANK6,	RF_R17,		RF_A_BAND_MB,	0x00},
		{RF_BANK6,	RF_R17,		RF_A_BAND_HB,	0x00},
		{RF_BANK6,	RF_R17,		RF_A_BAND_11J,	0x05},

		{RF_BANK6,	RF_R24,		RF_A_BAND_LB,	0xA1},
		{RF_BANK6,	RF_R24,		RF_A_BAND_MB,	0x41},
		{RF_BANK6,	RF_R24,		RF_A_BAND_HB,	0x21},
		{RF_BANK6,	RF_R24,		RF_A_BAND_11J,	0xE1},

		{RF_BANK6,	RF_R39,		RF_A_BAND_LB,	0x36},
		{RF_BANK6,	RF_R39,		RF_A_BAND_MB,	0x34},
		{RF_BANK6,	RF_R39,		RF_A_BAND_HB,	0x32},
		{RF_BANK6,	RF_R39,		RF_A_BAND_11J,	0x37},

		{RF_BANK6,	RF_R42,		RF_A_BAND_LB,	0xFB},
		{RF_BANK6,	RF_R42,		RF_A_BAND_MB,	0xF3},
		{RF_BANK6,	RF_R42,		RF_A_BAND_HB,	0xEB},
		{RF_BANK6,	RF_R42,		RF_A_BAND_11J,	0xEB},

		/* Move R6-R45, R50~R59 to MT76x0_RF_INT_PA_5G_Channel_0_RegTb/MT76x0_RF_EXT_PA_5G_Channel_0_RegTb */
	
		{RF_BANK6,	RF_R127,	RF_G_BAND,		0x84},
		{RF_BANK6,	RF_R127,	RF_A_BAND,		0x04},

		{RF_BANK7,	RF_R05,		RF_G_BAND,		0x40},
		{RF_BANK7,	RF_R05,		RF_A_BAND,		0x00},

		{RF_BANK7,	RF_R09,		RF_G_BAND,		0x00},
		{RF_BANK7,	RF_R09,		RF_A_BAND,		0x00},
		
		{RF_BANK7,	RF_R70,		RF_G_BAND,		0x00},
		{RF_BANK7,	RF_R70,		RF_A_BAND,		0x6D},

		{RF_BANK7,	RF_R71,		RF_G_BAND,		0x00},
		{RF_BANK7,	RF_R71,		RF_A_BAND,		0xB0},

		{RF_BANK7,	RF_R78,		RF_G_BAND,		0x00},
		{RF_BANK7,	RF_R78,		RF_A_BAND,		0x55},

		{RF_BANK7,	RF_R79,		RF_G_BAND,		0x00},
		{RF_BANK7,	RF_R79,		RF_A_BAND,		0x55},
};
UCHAR MT76x0_RF_Band_Switch_Size = (sizeof(MT76x0_RF_Band_Switch) / sizeof(MT76x0_RF_SWITCH_ITEM));

/*
	External PA
*/
static MT76x0_RF_SWITCH_ITEM MT76x0_RF_EXT_PA_RegTb[] = {
	{RF_BANK6,	RF_R45,		RF_A_BAND_LB,	0x63},
	{RF_BANK6,	RF_R45,		RF_A_BAND_MB,	0x43},
	{RF_BANK6,	RF_R45,		RF_A_BAND_HB,	0x33},
	{RF_BANK6,	RF_R45,		RF_A_BAND_11J,	0x73},

	{RF_BANK6,	RF_R50,		RF_A_BAND_LB,	0x02},
	{RF_BANK6,	RF_R50,		RF_A_BAND_MB,	0x02},
	{RF_BANK6,	RF_R50,		RF_A_BAND_HB,	0x02},
	{RF_BANK6,	RF_R50,		RF_A_BAND_11J,	0x02},

	{RF_BANK6,	RF_R51,		RF_A_BAND_LB,	0x02},
	{RF_BANK6,	RF_R51,		RF_A_BAND_MB,	0x02},
	{RF_BANK6,	RF_R51,		RF_A_BAND_HB,	0x02},
	{RF_BANK6,	RF_R51,		RF_A_BAND_11J,	0x02},

	{RF_BANK6,	RF_R52,		RF_A_BAND_LB,	0x08},
	{RF_BANK6,	RF_R52,		RF_A_BAND_MB,	0x08},
	{RF_BANK6,	RF_R52,		RF_A_BAND_HB,	0x08},
	{RF_BANK6,	RF_R52,		RF_A_BAND_11J,	0x08},

	{RF_BANK6,	RF_R53,		RF_A_BAND_LB,	0x08},
	{RF_BANK6,	RF_R53,		RF_A_BAND_MB,	0x08},
	{RF_BANK6,	RF_R53,		RF_A_BAND_HB,	0x08},
	{RF_BANK6,	RF_R53,		RF_A_BAND_11J,	0x08},

	{RF_BANK6,	RF_R54,		RF_A_BAND_LB,	0x0A},
	{RF_BANK6,	RF_R54,		RF_A_BAND_MB,	0x0A},
	{RF_BANK6,	RF_R54,		RF_A_BAND_HB,	0x0A},
	{RF_BANK6,	RF_R54,		RF_A_BAND_11J,	0x0A},

	{RF_BANK6,	RF_R55,		RF_A_BAND_LB,	0x0A},
	{RF_BANK6,	RF_R55,		RF_A_BAND_MB,	0x0A},
	{RF_BANK6,	RF_R55,		RF_A_BAND_HB,	0x0A},
	{RF_BANK6,	RF_R55,		RF_A_BAND_11J,	0x0A},

	{RF_BANK6,	RF_R56,		RF_A_BAND_LB,	0x05},
	{RF_BANK6,	RF_R56,		RF_A_BAND_MB,	0x05},
	{RF_BANK6,	RF_R56,		RF_A_BAND_HB,	0x05},
	{RF_BANK6,	RF_R56,		RF_A_BAND_11J,	0x05},

	{RF_BANK6,	RF_R57,		RF_A_BAND_LB,	0x05},
	{RF_BANK6,	RF_R57,		RF_A_BAND_MB,	0x05},
	{RF_BANK6,	RF_R57,		RF_A_BAND_HB,	0x05},
	{RF_BANK6,	RF_R57,		RF_A_BAND_11J,	0x05},

	{RF_BANK6,	RF_R58,		RF_A_BAND_LB,	0x05},
	{RF_BANK6,	RF_R58,		RF_A_BAND_MB,	0x03},
	{RF_BANK6,	RF_R58,		RF_A_BAND_HB,	0x02},
	{RF_BANK6,	RF_R58,		RF_A_BAND_11J,	0x07},

	{RF_BANK6,	RF_R59,		RF_A_BAND_LB,	0x05},
	{RF_BANK6,	RF_R59,		RF_A_BAND_MB,	0x03},
	{RF_BANK6,	RF_R59,		RF_A_BAND_HB,	0x02},
	{RF_BANK6,	RF_R59,		RF_A_BAND_11J,	0x07},
};
static UINT32 MT76x0_RF_EXT_PA_RegTb_Size = (sizeof(MT76x0_RF_EXT_PA_RegTb) / sizeof(MT76x0_RF_SWITCH_ITEM));

/*
	Internal PA
*/
static MT76x0_RF_SWITCH_ITEM MT76x0_RF_INT_PA_RegTb[] = {
};
static UINT32 MT76x0_RF_INT_PA_RegTb_Size = (sizeof(MT76x0_RF_INT_PA_RegTb) / sizeof(MT76x0_RF_SWITCH_ITEM));

BOOLEAN MT76x0_AsicGetTssiReport(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bResetTssiInfo,
	OUT PCHAR pTssiReport);

static VOID FullCalibration(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Channel,
	IN BOOLEAN bSave);

//
// Initialize FCE
//
VOID InitFce(
	PRTMP_ADAPTER pAd)
{
	L2_STUFFING_STRUC L2Stuffing;

	L2Stuffing.word = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

	RTMP_IO_READ32(pAd, FCE_L2_STUFF, &L2Stuffing.word);
	L2Stuffing.field.FS_WR_MPDU_LEN_EN = 0;
	RTMP_IO_WRITE32(pAd, FCE_L2_STUFF, L2Stuffing.word);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}


/*
	Select 2.4/5GHz band
*/
VOID SelectBandMT76x0(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR Channel)
{
	UCHAR RfValue = 0;
	UINT32 IdReg = 0;

	if (!IS_MT76x0(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect NIC\n", __FUNCTION__));
		
		return;
	}
	
	DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));

	if (Channel <= 14) 
	{

		/*
			Select 2.4GHz 
		*/
		for(IdReg = 0; IdReg < MT76x0_RF_2G_Channel_0_RegTb_Size; IdReg++)
		{
			rlt_rf_write(pAd, 
						MT76x0_RF_2G_Channel_0_RegTb[IdReg].Bank,
						MT76x0_RF_2G_Channel_0_RegTb[IdReg].Register,
						MT76x0_RF_2G_Channel_0_RegTb[IdReg].Value);
		}

		RfValue = 0x45;
		rlt_rf_write(pAd, RF_BANK5, RF_R00, RfValue);

		RfValue = 0x44;
		rlt_rf_write(pAd, RF_BANK6, RF_R00, RfValue);

		rtmp_mac_set_band(pAd, BAND_24G);

		RTMP_IO_WRITE32(pAd, TX_ALC_VGA3, 0x00050007);
		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_CORR, 0x003E0002);
	}
	else
	{
		/*
			Select 5GHz 
		*/
		for(IdReg = 0; IdReg < MT76x0_RF_5G_Channel_0_RegTb_Size; IdReg++)
		{
			rlt_rf_write(pAd, 
				     MT76x0_RF_5G_Channel_0_RegTb[IdReg].Bank,
				     MT76x0_RF_5G_Channel_0_RegTb[IdReg].Register,
				     MT76x0_RF_5G_Channel_0_RegTb[IdReg].Value);
		}

		RfValue = 0x44;
		rlt_rf_write(pAd, RF_BANK5, RF_R00, RfValue);

		RfValue = 0x45;
		rlt_rf_write(pAd, RF_BANK6, RF_R00, RfValue);
				
		rtmp_mac_set_band(pAd, BAND_5G);

		RTMP_IO_WRITE32(pAd, TX_ALC_VGA3, 0x00000005);
		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_CORR, 0x01010102);
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

/*
	Set RF channel frequency parameters:	
	Rdiv: R24[1:0]
	N: R29[7:0], R30[0]
	Nominator: R31[4:0]
	Non-Sigma: !SDM R31[7:5]
	Den: (Denomina - 8) R32[4:0]
	Loop Filter Config: R33, R34
	Pll_idiv: frac comp R35[6:0]
*/
VOID SetRfChFreqParametersMT76x0(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR Channel)
{
	UINT32 i = 0, RfBand = 0, MacReg = 0;
	UCHAR RFValue = 0;
	BOOLEAN bSDM = FALSE;
	MT76x0_FREQ_ITEM *pMT76x0_freq_item = NULL;

	if (!IS_MT76x0(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect NIC\n", __FUNCTION__));		
		return;
	}
	
	DBGPRINT(RT_DEBUG_INFO, ("%s: -->\n", __FUNCTION__));

	for (i = 0; i < MT76x0_SDM_Channel_Size; i++)
	{
		if (Channel == MT76x0_SDM_Channel[i])
		{
			bSDM = TRUE;
			break;
		}
	}

	for (i = 0; i < NUM_OF_MT76x0_CHNL; i++)
	{
		if (Channel == MT76x0_Frequency_Plan[i].Channel)
		{
			RfBand = MT76x0_Frequency_Plan[i].Band;			

			if (bSDM)
				pMT76x0_freq_item = &(MT76x0_SDM_Frequency_Plan[i]);
			else
				pMT76x0_freq_item = &(MT76x0_Frequency_Plan[i]);

			/* 
				R37
			*/
			rlt_rf_write(pAd, RF_BANK0, RF_R37, pMT76x0_freq_item->pllR37);
			
			/*
				R36
			*/
			rlt_rf_write(pAd, RF_BANK0, RF_R36, pMT76x0_freq_item->pllR36);

			/*
				R35
			*/
			rlt_rf_write(pAd, RF_BANK0, RF_R35, pMT76x0_freq_item->pllR35);

			/*
				R34
			*/
			rlt_rf_write(pAd, RF_BANK0, RF_R34, pMT76x0_freq_item->pllR34);

			/*
				R33
			*/
			rlt_rf_write(pAd, RF_BANK0, RF_R33, pMT76x0_freq_item->pllR33);

			/*
				R32<7:5>
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R32, &RFValue);
			RFValue &= ~(0xE0);
			RFValue |= pMT76x0_freq_item->pllR32_b7b5;
			rlt_rf_write(pAd, RF_BANK0, RF_R32, RFValue);
			
			/*
				R32<4:0> pll_den: (Denomina - 8)
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R32, &RFValue);
			RFValue &= ~(0x1F);
			RFValue |= pMT76x0_freq_item->pllR32_b4b0;
			rlt_rf_write(pAd, RF_BANK0, RF_R32, RFValue);

			/*
				R31<7:5>
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R31, &RFValue);
			RFValue &= ~(0xE0);
			RFValue |= pMT76x0_freq_item->pllR31_b7b5;
			rlt_rf_write(pAd, RF_BANK0, RF_R31, RFValue);

			/*
				R31<4:0> pll_k(Nominator)
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R31, &RFValue);
			RFValue &= ~(0x1F);
			RFValue |= pMT76x0_freq_item->pllR31_b4b0;
			rlt_rf_write(pAd, RF_BANK0, RF_R31, RFValue);
			
			/*
				R30<7> sdm_reset_n
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue &= ~(0x80);
			if (bSDM)
			{
				rlt_rf_write(pAd, RF_BANK0, RF_R30, RFValue);
				RFValue |= (0x80);
				rlt_rf_write(pAd, RF_BANK0, RF_R30, RFValue);
			}
			else
			{
				RFValue |= pMT76x0_freq_item->pllR30_b7;
				rlt_rf_write(pAd, RF_BANK0, RF_R30, RFValue);
			}
			
			/*
				R30<6:2> sdmmash_prbs,sin
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue &= ~(0x7C);
			RFValue |= pMT76x0_freq_item->pllR30_b6b2;
			rlt_rf_write(pAd, RF_BANK0, RF_R30, RFValue);
			
			/*
				R30<1> sdm_bp
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue &= ~(0x02);
			RFValue |= (pMT76x0_freq_item->pllR30_b1 << 1);
			rlt_rf_write(pAd, RF_BANK0, RF_R30, RFValue);
			
			/*
				R30<0> R29<7:0> (hex) pll_n
			*/
			RFValue = pMT76x0_freq_item->pll_n & 0x00FF;
			rlt_rf_write(pAd, RF_BANK0, RF_R29, RFValue);

			rlt_rf_read(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue &= ~(0x1);
			RFValue |= ((pMT76x0_freq_item->pll_n >> 8) & 0x0001);
			rlt_rf_write(pAd, RF_BANK0, RF_R30, RFValue);
			
			/*
				R28<7:6> isi_iso
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R28, &RFValue);
			RFValue &= ~(0xC0);
			RFValue |= pMT76x0_freq_item->pllR28_b7b6;
			rlt_rf_write(pAd, RF_BANK0, RF_R28, RFValue);
			
			/*
				R28<5:4> pfd_dly
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R28, &RFValue);
			RFValue &= ~(0x30);
			RFValue |= pMT76x0_freq_item->pllR28_b5b4;
			rlt_rf_write(pAd, RF_BANK0, RF_R28, RFValue);
			
			/*
				R28<3:2> clksel option
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R28, &RFValue);
			RFValue &= ~(0x0C);
			RFValue |= pMT76x0_freq_item->pllR28_b3b2;
			rlt_rf_write(pAd, RF_BANK0, RF_R28, RFValue);

			/*
				R28<1:0> R27<7:0> R26<7:0> (hex) sdm_k
			*/
			RFValue = pMT76x0_freq_item->Pll_sdm_k & 0x000000FF;
			rlt_rf_write(pAd, RF_BANK0, RF_R26, RFValue);

			RFValue = ((pMT76x0_freq_item->Pll_sdm_k >> 8) & 0x000000FF);
			rlt_rf_write(pAd, RF_BANK0, RF_R27, RFValue);
			
			rlt_rf_read(pAd, RF_BANK0, RF_R28, &RFValue);
			RFValue &= ~(0x3);
			RFValue |= ((pMT76x0_freq_item->Pll_sdm_k >> 16) & 0x0003);
			rlt_rf_write(pAd, RF_BANK0, RF_R28, RFValue);
			
			/*
				R24<1:0> xo_div
			*/
			rlt_rf_read(pAd, RF_BANK0, RF_R24, &RFValue);
			RFValue &= ~(0x3);
			RFValue |= pMT76x0_freq_item->pllR24_b1b0;
			rlt_rf_write(pAd, RF_BANK0, RF_R24, RFValue);

			
			pAd->LatchRfRegs.Channel = Channel; /* Channel latch */

			DBGPRINT(RT_DEBUG_TRACE,
				("%s: SwitchChannel#%d(Band = 0x%02X, RF=%d, %dT), "
				"0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, %u, 0x%02X, %u, 0x%02X, 0x%02X, 0x%02X, 0x%04X, 0x%02X, 0x%02X, 0x%02X, 0x%08X, 0x%02X\n",
				__FUNCTION__,
				Channel,
				RfBand,
				pAd->RfIcType,
				pAd->Antenna.field.TxPath,
				pMT76x0_freq_item->pllR37,
				pMT76x0_freq_item->pllR36,
				pMT76x0_freq_item->pllR35,
				pMT76x0_freq_item->pllR34,
				pMT76x0_freq_item->pllR33,
				pMT76x0_freq_item->pllR32_b7b5,
				pMT76x0_freq_item->pllR32_b4b0,
				pMT76x0_freq_item->pllR31_b7b5,
				pMT76x0_freq_item->pllR31_b4b0,
				pMT76x0_freq_item->pllR30_b7,
				pMT76x0_freq_item->pllR30_b6b2,
				pMT76x0_freq_item->pllR30_b1,
				pMT76x0_freq_item->pll_n,
				pMT76x0_freq_item->pllR28_b7b6,
				pMT76x0_freq_item->pllR28_b5b4,
				pMT76x0_freq_item->pllR28_b3b2,
				pMT76x0_freq_item->Pll_sdm_k,
				pMT76x0_freq_item->pllR24_b1b0));
			break;
		}
	}	


	for(i = 0; i < MT76x0_RF_BW_Switch_Size; i++)
	{
		if (pAd->CommonCfg.BBPCurrentBW == MT76x0_RF_BW_Switch[i].BwBand)
		{
			rlt_rf_write(pAd, 
						MT76x0_RF_BW_Switch[i].Bank,
						MT76x0_RF_BW_Switch[i].Register,
						MT76x0_RF_BW_Switch[i].Value);
		}
		else if ((pAd->CommonCfg.BBPCurrentBW == (MT76x0_RF_BW_Switch[i].BwBand & 0xFF)) &&
				 (RfBand & MT76x0_RF_BW_Switch[i].BwBand))
		{
			rlt_rf_write(pAd, 
						MT76x0_RF_BW_Switch[i].Bank,
						MT76x0_RF_BW_Switch[i].Register,
						MT76x0_RF_BW_Switch[i].Value);
		}
	}

	for(i = 0; i < MT76x0_RF_Band_Switch_Size; i++)
	{
		if (MT76x0_RF_Band_Switch[i].BwBand & RfBand)
		{
			rlt_rf_write(pAd, 
						MT76x0_RF_Band_Switch[i].Bank,
						MT76x0_RF_Band_Switch[i].Register,
						MT76x0_RF_Band_Switch[i].Value);
		}
	}
	
	RTMP_IO_READ32(pAd, RF_MISC, &MacReg);
	MacReg &= ~(0xC); /* Clear 0x518[3:2] */
	RTMP_IO_WRITE32(pAd, RF_MISC, MacReg);

	DBGPRINT(RT_DEBUG_INFO, ("\n\n*********** PAType = %d ***********\n\n", pAd->chipCap.PAType));
	if ((pAd->chipCap.PAType == INT_PA_2G_5G) ||
		((pAd->chipCap.PAType == EXT_PA_5G_ONLY) && (RfBand & RF_G_BAND)) ||
		((pAd->chipCap.PAType == EXT_PA_2G_ONLY) && (RfBand & RF_A_BAND)))
	{
		/* Internal PA */
		for(i = 0; i < MT76x0_RF_INT_PA_RegTb_Size; i++)
	{
			if (MT76x0_RF_INT_PA_RegTb[i].BwBand & RfBand)
			{
				rlt_rf_write(pAd, 
							MT76x0_RF_INT_PA_RegTb[i].Bank,
							MT76x0_RF_INT_PA_RegTb[i].Register,
							MT76x0_RF_INT_PA_RegTb[i].Value);

				DBGPRINT(RT_DEBUG_INFO, ("%s: INT_PA_RegTb - B%d.R%02d = 0x%02x\n", 
							__FUNCTION__, 
							MT76x0_RF_INT_PA_RegTb[i].Bank,
							MT76x0_RF_INT_PA_RegTb[i].Register,
							MT76x0_RF_INT_PA_RegTb[i].Value));
			}
		}
	}
	else
	{
		/*
			RF_MISC (offset: 0x0518)
			[2]1'b1: enable external A band PA, 1'b0: disable external A band PA
			[3]1'b1: enable external G band PA, 1'b0: disable external G band PA
		*/
		if (RfBand & RF_A_BAND)
		{
			RTMP_IO_READ32(pAd, RF_MISC, &MacReg);
			MacReg |= (0x4);
			RTMP_IO_WRITE32(pAd, RF_MISC, MacReg);
		}
		else
		{
			RTMP_IO_READ32(pAd, RF_MISC, &MacReg);
			MacReg |= (0x8);
			RTMP_IO_WRITE32(pAd, RF_MISC, MacReg);
		}
		
		/* External PA */
		for(i = 0; i < MT76x0_RF_EXT_PA_RegTb_Size; i++)
		{
			if (MT76x0_RF_EXT_PA_RegTb[i].BwBand & RfBand)
			{
				rlt_rf_write(pAd, 
							MT76x0_RF_EXT_PA_RegTb[i].Bank,
							MT76x0_RF_EXT_PA_RegTb[i].Register,
							MT76x0_RF_EXT_PA_RegTb[i].Value);

				DBGPRINT(RT_DEBUG_INFO, ("%s: EXT_PA_RegTb - B%d.R%02d = 0x%02x\n", 
							__FUNCTION__, 
							MT76x0_RF_EXT_PA_RegTb[i].Bank,
							MT76x0_RF_EXT_PA_RegTb[i].Register,
							MT76x0_RF_EXT_PA_RegTb[i].Value));
			}
		}
	}

	if (RfBand & RF_G_BAND)
	{
		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_ATTEN, 0x63707400);
		RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &MacReg);
		MacReg &= 0x896400FF;
		RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, MacReg); /* Set Atten mode = 2 For G band, Disable Tx Inc DCOC Cal by Chee's comment. @MT7650_CR_setting_1018.xlsx */		
	}
	else
	{
		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_ATTEN, 0x686A7800);
		RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &MacReg);
		MacReg &= 0x890400FF;
		RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, MacReg); /* Set Atten mode = 0 For Ext A band, Disable Tx Inc DCOC Cal by Chee's comment. @MT7650_CR_setting_1018.xlsx */		
	}
	
	DBGPRINT(RT_DEBUG_INFO, ("%s: <--\n", __FUNCTION__));
}

static VOID NICInitMT76x0RFRegisters(RTMP_ADAPTER *pAd)
{

	UINT32 IdReg;
	UCHAR RFValue;


	for(IdReg = 0; IdReg < MT76x0_RF_Central_RegTb_Size; IdReg++)
	{
		if (MT76x0_RF_Central_RegTb[IdReg].Bank == RF_BANK0 &&
			MT76x0_RF_Central_RegTb[IdReg].Register == RF_R21)
		{
			USHORT e2p_val = 0;
			/* Check if we are co-clock mode */
			RT28xx_EEPROM_READ16(pAd, 0x42, e2p_val);
			if (!(e2p_val & (1 << 9)))
				MT76x0_RF_Central_RegTb[IdReg].Value = 0x12;
		}
		rlt_rf_write(pAd, 
					MT76x0_RF_Central_RegTb[IdReg].Bank,
					MT76x0_RF_Central_RegTb[IdReg].Register,
					MT76x0_RF_Central_RegTb[IdReg].Value);
	}

	for(IdReg = 0; IdReg < MT76x0_RF_2G_Channel_0_RegTb_Size; IdReg++)
	{
		rlt_rf_write(pAd, 
					MT76x0_RF_2G_Channel_0_RegTb[IdReg].Bank,
					MT76x0_RF_2G_Channel_0_RegTb[IdReg].Register,
					MT76x0_RF_2G_Channel_0_RegTb[IdReg].Value);
	}

	for(IdReg = 0; IdReg < MT76x0_RF_5G_Channel_0_RegTb_Size; IdReg++)
	{
		rlt_rf_write(pAd, 
					MT76x0_RF_5G_Channel_0_RegTb[IdReg].Bank,
					MT76x0_RF_5G_Channel_0_RegTb[IdReg].Register,
					MT76x0_RF_5G_Channel_0_RegTb[IdReg].Value);
	}

	for(IdReg = 0; IdReg < MT76x0_RF_VGA_Channel_0_RegTb_Size; IdReg++)
	{
		rlt_rf_write(pAd, 
					MT76x0_RF_VGA_Channel_0_RegTb[IdReg].Bank,
					MT76x0_RF_VGA_Channel_0_RegTb[IdReg].Register,
					MT76x0_RF_VGA_Channel_0_RegTb[IdReg].Value);
	}

	for(IdReg = 0; IdReg < MT76x0_RF_BW_Switch_Size; IdReg++)
	{
		if (pAd->CommonCfg.BBPCurrentBW == MT76x0_RF_BW_Switch[IdReg].BwBand)
		{
			rlt_rf_write(pAd, 
						MT76x0_RF_BW_Switch[IdReg].Bank,
						MT76x0_RF_BW_Switch[IdReg].Register,
						MT76x0_RF_BW_Switch[IdReg].Value);
		}
		else if ((BW_20 == (MT76x0_RF_BW_Switch[IdReg].BwBand & 0xFF)) &&
				 (RF_G_BAND & MT76x0_RF_BW_Switch[IdReg].BwBand))
		{
			rlt_rf_write(pAd, 
						MT76x0_RF_BW_Switch[IdReg].Bank,
						MT76x0_RF_BW_Switch[IdReg].Register,
						MT76x0_RF_BW_Switch[IdReg].Value);
		}
	}

	for(IdReg = 0; IdReg < MT76x0_RF_Band_Switch_Size; IdReg++)
	{
		if (MT76x0_RF_Band_Switch[IdReg].BwBand & RF_G_BAND)
		{
			rlt_rf_write(pAd, 
						MT76x0_RF_Band_Switch[IdReg].Bank,
						MT76x0_RF_Band_Switch[IdReg].Register,
						MT76x0_RF_Band_Switch[IdReg].Value);
		}
	}

	/*
		Frequency calibration
		E1: B0.R22<6:0>: xo_cxo<6:0>
		E2: B0.R21<0>: xo_cxo<0>, B0.R22<7:0>: xo_cxo<8:1> 
	*/
	RFValue = (UCHAR)(pAd->RfFreqOffset & 0xFF);
	rlt_rf_write(pAd, RF_BANK0, RF_R22, RFValue);
	
	rlt_rf_read(pAd, RF_BANK0, RF_R22, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: B0.R22 = 0x%02x\n", __FUNCTION__, RFValue));

	/*
		Reset the DAC (Set B0.R73<7>=1, then set B0.R73<7>=0, and then set B0.R73<7>) during power up.
	*/
	rlt_rf_read(pAd, RF_BANK0, RF_R73, &RFValue);
	RFValue |= 0x80;
	rlt_rf_write(pAd, RF_BANK0, RF_R73, RFValue);	
	RFValue &= (~0x80);
	rlt_rf_write(pAd, RF_BANK0, RF_R73, RFValue);	
	RFValue |= 0x80;
	rlt_rf_write(pAd, RF_BANK0, RF_R73, RFValue);	

	/* 
		vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. 
	*/
	rlt_rf_read(pAd, RF_BANK0, RF_R04, &RFValue);
	RFValue = ((RFValue & ~0x80) | 0x80); 
	rlt_rf_write(pAd, RF_BANK0, RF_R04, RFValue);
	return;
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
static VOID NICInitMT76x0MacRegisters(RTMP_ADAPTER *pAd)
{
	UINT32 IdReg;
	UINT32 MacReg = 0;

	/*
		Enable PBF and MAC clock
		SYS_CTRL[11:10] = 0x3
	*/
	for(IdReg=0; IdReg<MT76x0_NUM_MAC_REG_PARMS; IdReg++)
	{
		RTMP_IO_WRITE32(pAd, MT76x0_MACRegTable[IdReg].Register,
								MT76x0_MACRegTable[IdReg].Value);
	}

#ifdef HDR_TRANS_TX_SUPPORT
	/*
		Enable Header Translation TX 
	*/
	RTMP_IO_READ32(pAd, HEADER_TRANS_CTRL_REG, &MacReg);
	MacReg |= 0x1; /* 0x1: TX, 0x2: RX */
	RTMP_IO_WRITE32(pAd, HEADER_TRANS_CTRL_REG, MacReg);
#endif /* HDR_TRANS_TX_SUPPORT */

	/*
		Release BBP and MAC reset
		MAC_SYS_CTRL[1:0] = 0x0
	*/
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacReg);
	MacReg &= ~(0x3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacReg);

#ifdef RTMP_MAC_PCI
	if (IS_MT7610E(pAd))
	{
		//
		// Disable COEX_EN
		//
		RTMP_IO_READ32(pAd, COEXCFG0, &MacReg);
		MacReg &= 0xFFFFFFFE;
		RTMP_IO_WRITE32(pAd, COEXCFG0, MacReg);
	}
#endif /* RTMP_MAC_PCI */

	/*
		Set 0x141C[15:12]=0xF
	*/
	RTMP_IO_READ32(pAd, EXT_CCA_CFG, &MacReg);
	MacReg |= (0x0000F000);
	RTMP_IO_WRITE32(pAd, EXT_CCA_CFG, MacReg);

	InitFce(pAd);

	/*
		TxRing 9 is for Mgmt frame.
		TxRing 8 is for In-band command frame.
		WMM_RG0_TXQMA: This register setting is for FCE to define the rule of TxRing 9.
		WMM_RG1_TXQMA: This register setting is for FCE to define the rule of TxRing 8.
	*/
	RTMP_IO_READ32(pAd, WMM_CTRL, &MacReg);
	MacReg &= ~(0x000003FF);
	MacReg |= (0x00000201);
	RTMP_IO_WRITE32(pAd, WMM_CTRL, MacReg);


	/*
		0x110:	Set PERST_N iopad to NO 75K pull up. (MT7650_E3_CR_setting_0124.xlsx 2013)
	*/
	RTMP_IO_READ32(pAd, 0x110, &MacReg);
	MacReg &= ~(0x00000200);
	RTMP_IO_WRITE32(pAd, 0x110, MacReg);



	return;
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
static VOID NICInitMT76x0BbpRegisters(
	IN	PRTMP_ADAPTER pAd)
{

	INT IdReg;

	for(IdReg = 0; IdReg < MT76x0_BBP_Init_Tab_Size; IdReg++)
	{
		RTMP_BBP_IO_WRITE32(pAd, MT76x0_BBP_Init_Tab[IdReg].Register,
				MT76x0_BBP_Init_Tab[IdReg].Value);
	}
	
	for (IdReg = 0; IdReg < MT76x0_BPP_SWITCH_Tab_Size; IdReg++)
	{
		if (((RF_G_BAND | RF_BW_20) & MT76x0_BPP_SWITCH_Tab[IdReg].BwBand) == (RF_G_BAND | RF_BW_20))
		{
			RTMP_BBP_IO_WRITE32(pAd, MT76x0_BPP_SWITCH_Tab[IdReg].RegDate.Register,
					MT76x0_BPP_SWITCH_Tab[IdReg].RegDate.Value);
		}
	}

	for(IdReg = 0; IdReg < MT76x0_DCOC_Tab_Size; IdReg++)
	{
		RTMP_BBP_IO_WRITE32(pAd, MT76x0_DCOC_Tab[IdReg].Register,
				MT76x0_DCOC_Tab[IdReg].Value);
	}

	return;
}


static VOID MT76x0_AsicAntennaDefaultReset(
	IN struct _RTMP_ADAPTER	*pAd,
	IN EEPROM_ANTENNA_STRUC *pAntenna)
{
	pAntenna->word = 0;
	pAntenna->field.RfIcType = RFIC_7650;
	pAntenna->field.TxPath = 1;
	pAntenna->field.RxPath = 1;
}


static VOID MT76x0_ChipBBPAdjust(RTMP_ADAPTER *pAd)
{
#ifdef DBG
	static char *ext_str[]={"extNone", "extAbove", "", "extBelow"};
#endif
	UCHAR rf_bw, ext_ch;


#ifdef DOT11_N_SUPPORT
	if (get_ht_cent_ch(pAd, &rf_bw, &ext_ch) == FALSE)
#endif /* DOT11_N_SUPPORT */
	{
		rf_bw = BW_20;
		ext_ch = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	}

#ifdef DOT11_VHT_AC
	if (WMODE_CAP(pAd->CommonCfg.PhyMode, WMODE_AC) &&
		(pAd->CommonCfg.Channel > 14) &&
		(rf_bw == BW_40) &&
		(pAd->CommonCfg.vht_bw == VHT_BW_80) &&
		(pAd->CommonCfg.vht_cent_ch != pAd->CommonCfg.CentralChannel))
	{
		rf_bw = BW_80;
		pAd->CommonCfg.vht_cent_ch = vht_cent_ch_freq(pAd, pAd->CommonCfg.Channel);
	}

//+++Add by shiang for debug
	DBGPRINT(RT_DEBUG_OFF, ("%s():rf_bw=%d, ext_ch=%d, PrimCh=%d, HT-CentCh=%d, VHT-CentCh=%d\n",
				__FUNCTION__, rf_bw, ext_ch, pAd->CommonCfg.Channel,
				pAd->CommonCfg.CentralChannel, pAd->CommonCfg.vht_cent_ch));
//---Add by shiang for debug
#endif /* DOT11_VHT_AC */

	rtmp_bbp_set_bw(pAd, rf_bw);

	/* TX/Rx : control channel setting */
	rtmp_mac_set_ctrlch(pAd, ext_ch);
	rtmp_bbp_set_ctrlch(pAd, ext_ch);
		
#ifdef DOT11_N_SUPPORT
	DBGPRINT(RT_DEBUG_TRACE, ("%s() : %s, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
					__FUNCTION__, ext_str[ext_ch],
					pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
					pAd->CommonCfg.Channel,
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
					pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif /* DOT11_N_SUPPORT */
}


void MT76x0_adjust_per_rate_pwr_delta(RTMP_ADAPTER *ad, u8 channel, char delta_pwr)
{
	u32 value = 0;

	value |= TX_PWR_CCK_1_2(ad->chipCap.rate_pwr_table.CCK[0].MCS_Power + delta_pwr);
	value |= TX_PWR_CCK_5_11(ad->chipCap.rate_pwr_table.CCK[2].MCS_Power + delta_pwr);
	value |= TX_PWR_OFDM_6_9(ad->chipCap.rate_pwr_table.OFDM[0].MCS_Power + delta_pwr);
	value |= TX_PWR_OFDM_12_18(ad->chipCap.rate_pwr_table.OFDM[2].MCS_Power + delta_pwr);
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_0, value);

	value |= TX_PWR_OFDM_24_36(ad->chipCap.rate_pwr_table.OFDM[4].MCS_Power + delta_pwr);
	value |= TX_PWR_OFDM_48(ad->chipCap.rate_pwr_table.OFDM[6].MCS_Power + delta_pwr);
	value |= TX_PWR_HT_VHT_1SS_MCS_0_1(ad->chipCap.rate_pwr_table.HT[0].MCS_Power + delta_pwr);
	value |= TX_PWR_HT_VHT_1SS_MCS_2_3(ad->chipCap.rate_pwr_table.HT[2].MCS_Power + delta_pwr);
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_1, value);

	value |= TX_PWR_HT_VHT_1SS_MCS_4_5(ad->chipCap.rate_pwr_table.HT[4].MCS_Power + delta_pwr);
	value |= TX_PWR_HT_VHT_1SS_MCS_6(ad->chipCap.rate_pwr_table.HT[6].MCS_Power + delta_pwr);
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_2, value);


	value |= TX_PWR_HT_VHT_STBC_MCS_0_1(ad->chipCap.rate_pwr_table.STBC[0].MCS_Power + delta_pwr);
	value |= TX_PWR_HT_VHT_STBC_MCS_2_3(ad->chipCap.rate_pwr_table.STBC[2].MCS_Power + delta_pwr);
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_3, value);


	value |= TX_PWR_HT_VHT_STBC_MCS_4_5(ad->chipCap.rate_pwr_table.STBC[4].MCS_Power + delta_pwr);
	value |= TX_PWR_HT_VHT_STBC_MCS_6(ad->chipCap.rate_pwr_table.STBC[6].MCS_Power + delta_pwr);
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_4, value);

	value |= TX_PWR_OFDM_54(ad->chipCap.rate_pwr_table.OFDM[7].MCS_Power + delta_pwr);
	value |= TX_PWR_HT_MCS_7_VHT_1SS_MCS_7(ad->chipCap.rate_pwr_table.HT[7].MCS_Power + delta_pwr);
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_7, value);

	value |= TX_PWR_VHT_1SS_MCS_8(ad->chipCap.rate_pwr_table.VHT[8].MCS_Power + delta_pwr);
	value |= TX_PWR_VHT_1SS_MCS_9(ad->chipCap.rate_pwr_table.VHT[9].MCS_Power + delta_pwr);	
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_8, value);

	value |= TX_PWR_HT_VHT_STBC_MCS_7(ad->chipCap.rate_pwr_table.STBC[7].MCS_Power + delta_pwr);
	RTMP_IO_WRITE32(ad, TX_PWR_CFG_9, value);
}

#define MIN_TSSI_WORKABLE_PWR 20 //10 dB
void percentage_delta_pwr(RTMP_ADAPTER *ad) 
{
	CHAR mac_drop_pwr = 0, tx_alc_ch_init_0 = 0, tx_alc_ch_init_1 = 0, orig_mac_pwr = 0;
	UCHAR mdsm_drop_pwr;
	UINT32 bbp_val = 0;
	CHAR bbp_drop_pwr = 0;
	UINT32 mac_val = 0;
	
	/* 
		Calculate delta power based on the percentage specified from UI.
		EEPROM setting is calibrated for maximum Tx power (i.e. 100%).
		Here lower Tx power according to the percentage specified from UI.
	*/

	RTMP_IO_READ32(ad, TX_ALC_CFG_0, &mac_val);
	orig_mac_pwr = mac_val & 0x3F;
	
	if (ad->CommonCfg.TxPowerPercentage > 90) {
		/* 91 ~ 100% & auto, treat as 100% in terms of mW */
		;
	} else if (ad->CommonCfg.TxPowerPercentage > 60) {
		/* 61 ~ 90%, treat as 75% in terms of mW */
		mac_drop_pwr -= 1;
	} else if (ad->CommonCfg.TxPowerPercentage > 30) {
		/* 31 ~ 60%, treat as 50% in terms of mW */
		mac_drop_pwr -= 3;
	} else if (ad->CommonCfg.TxPowerPercentage > 15) {
		/* 16 ~ 30%, treat as 25% in terms of mW */

		if(orig_mac_pwr - 6*2 > 0)
			mac_drop_pwr -= 6; 	/* 13b0 still positive , drop by mac */
		else
			bbp_drop_pwr -= 6;	/* 13b0 negative, drop by bbp */
	} else if (ad->CommonCfg.TxPowerPercentage > 9) {
		/* 10 ~ 15%, treat as 12.5% in terms of mW */

		if(orig_mac_pwr - 9*2 > 0)
			mac_drop_pwr -= 9; 	/* 13b0 still positive , drop by mac */
		else
		{
			mac_drop_pwr -= 3;
			bbp_drop_pwr -= 6;	/* 13b0 negative, drop by bbp */
		}
		
	} else {
		/* 0 ~ 9 %, treat as MIN(~3%) in terms of mW */

		if(orig_mac_pwr - 12*2 > 0)
			mac_drop_pwr -= 12; /* 13b0 still positive , drop by mac */
		else
			bbp_drop_pwr -= 12;	/* 13b0 negative, drop by bbp */		
	}

	if(((bbp_drop_pwr != 0) || (orig_mac_pwr + mac_drop_pwr*2 < MIN_TSSI_WORKABLE_PWR)) 
	&& ad->chipCap.bInternalTxALC)
		ad->CommonCfg.TxPowerPercentageWithBBP = TRUE;
	else
		ad->CommonCfg.TxPowerPercentageWithBBP = FALSE;
		
	
	tx_alc_ch_init_0 = (mac_val & 0x3F) + mac_drop_pwr*2;
	if (tx_alc_ch_init_0 <= 0)
		tx_alc_ch_init_0 = 0;
	tx_alc_ch_init_1 = ((mac_val & 0x3F00) >> 8) + mac_drop_pwr*2;
	if (tx_alc_ch_init_1<= 0)
		tx_alc_ch_init_1 = 0;
	DBGPRINT(RT_DEBUG_TRACE, ("%s::<Before> TX_ALC_CFG_0=0x%0x, tx_alc_ch_init_0=0x%0x, tx_alc_ch_init_1=0x%0x\n", 
		__FUNCTION__, mac_val, tx_alc_ch_init_0, tx_alc_ch_init_1));
	
	mac_val = mac_val & (~TX_ALC_CFG_0_CH_INT_0_MASK);
	mac_val |= TX_ALC_CFG_0_CH_INT_0(tx_alc_ch_init_0);
	mac_val = mac_val & (~TX_ALC_CFG_0_CH_INT_1_MASK);
	mac_val |= TX_ALC_CFG_0_CH_INT_1(tx_alc_ch_init_1);
	RTMP_IO_WRITE32(ad, TX_ALC_CFG_0, mac_val);
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s::<After> TX_ALC_CFG_0=0x%0x\n", 
		__FUNCTION__, mac_val));

	if (bbp_drop_pwr == -6)
		mdsm_drop_pwr = 0x01;
	else if (bbp_drop_pwr == -12)
		mdsm_drop_pwr = 0x02;
	else
		mdsm_drop_pwr = 0x00;
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s::drop the BBP transmit power by %d dBm!\n",
		__FUNCTION__, 
		(mdsm_drop_pwr == 0x02 ? 12 : \
		(mdsm_drop_pwr == 0x01 ? 6 : 0))));

	RTMP_BBP_IO_READ32(ad, TXBE_R4, &bbp_val);
	bbp_val &= (~0x3);
	bbp_val |= mdsm_drop_pwr;
	RTMP_BBP_IO_WRITE32(ad, TXBE_R4, bbp_val);
	DBGPRINT(RT_DEBUG_ERROR, ("%s::<After> total drop power = %d + %d dBm, TXBE_R4 = 0x%0x , PowerPercentageWithBBP = %d\n", 
		__FUNCTION__, mac_drop_pwr , bbp_drop_pwr, bbp_val , ad->CommonCfg.TxPowerPercentageWithBBP));
}


static VOID MT76x0_ChipSwitchChannel(
	struct _RTMP_ADAPTER *pAd,
	UCHAR Channel,
	BOOLEAN	 bScan)
{
	CHAR TxPwer = 0; /* Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER; */
	UCHAR RFValue = 0;
	UINT32 RegValue = 0;
	UINT32 Index;
	UINT32 rf_phy_mode, rf_bw = RF_BW_20;
	UCHAR bbp_ch_idx;
	UINT32 Value;
#ifdef SINGLE_SKU_V2
	CHAR SkuBasePwr;
	CHAR ChannelPwrAdj;
#endif /* SINGLE_SKU_V2 */
	BOOLEAN	Cancelled;	

	bbp_ch_idx = vht_prim_ch_idx(Channel, pAd->CommonCfg.Channel);

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): MAC_STATUS_CFG = 0x%08x, bbp_ch_idx = %d\n", __FUNCTION__, RegValue, bbp_ch_idx));

	if (Channel > 14)
		rf_phy_mode = RF_A_BAND;
	else
		rf_phy_mode = RF_G_BAND;
	
	RTMP_IO_READ32(pAd, EXT_CCA_CFG, &RegValue);
	RegValue &= ~(0xFFF);
	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
	{
		rf_bw = RF_BW_80;
		if (bbp_ch_idx == 0)
		{
			RegValue |= 0x1e4;
		}
		else if (bbp_ch_idx == 1)
		{
			RegValue |= 0x2e1;
		}
		else if (bbp_ch_idx == 2)
		{
			RegValue |= 0x41e;
		}
		else if (bbp_ch_idx == 3)
		{
			RegValue |= 0x81b;
		}
	}
	else if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
		rf_bw = RF_BW_40;
		if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
			RegValue |= 0x1e4;
		else
			RegValue |= 0x2e1;
	}
	else
	{
		rf_bw = RF_BW_20;
		RegValue |= 0x1e4;
		
	}
	RTMP_IO_WRITE32(pAd, EXT_CCA_CFG, RegValue);

	if (pAd->CommonCfg.BBPCurrentBW == BW_20){
		RTMP_IO_WRITE32(pAd, TX_SW_CFG0, 0x601);
	} else {
		RTMP_IO_WRITE32(pAd, TX_SW_CFG0, 0x201);
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		Configure 2.4/5GHz before accessing other MAC/BB/RF registers
	*/
	SelectBandMT76x0(pAd, Channel);

	/*
		Set RF channel frequency parameters (Rdiv, N, K, D and Ksd)
	*/
	SetRfChFreqParametersMT76x0(pAd, Channel);

	/* 
		vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. 
	*/
	rlt_rf_read(pAd, RF_BANK0, RF_R04, &RFValue);
	RFValue = ((RFValue & ~0x80) | 0x80); 
	rlt_rf_write(pAd, RF_BANK0, RF_R04, RFValue);

	for (Index = 0; Index < MAX_NUM_OF_CHANNELS; Index++)
	{
		if (Channel == pAd->TxPower[Index].Channel)
		{
			TxPwer = pAd->TxPower[Index].Power;
			break;
		}
	}	

	for (Index = 0; Index < MT76x0_BPP_SWITCH_Tab_Size; Index++)
	{
		if (((rf_phy_mode | rf_bw) & MT76x0_BPP_SWITCH_Tab[Index].BwBand) == (rf_phy_mode | rf_bw))
		{
			if ((MT76x0_BPP_SWITCH_Tab[Index].RegDate.Register == AGC1_R8))
			{
				UINT32 eLNAgain = (MT76x0_BPP_SWITCH_Tab[Index].RegDate.Value & 0x0000FF00) >> 8;

				if (Channel > 14)
				{
					if (Channel < pAd->chipCap.a_band_mid_ch)
						eLNAgain -= (pAd->ALNAGain0*2);
					else if (Channel < pAd->chipCap.a_band_high_ch)
						eLNAgain -= (pAd->ALNAGain1*2);
					else
						eLNAgain -= (pAd->ALNAGain2*2);
		
					pAd->chipCap.IsTempSensorStateReset = TRUE;					
				}
				else
				{
					eLNAgain -= (pAd->BLNAGain*2);
				}

#ifdef DYNAMIC_VGA_SUPPORT
				if (!bScan) /* update inital gain if not during site_survey */
				{
					pAd->CommonCfg.MO_Cfg.Stored_BBP_R66 = eLNAgain;
				}
				
				RTMPCancelTimer(&pAd->CommonCfg.MO_Cfg.DyncVgaLockTimer, &Cancelled);

				pAd->chipCap.dynamic_chE_mode = 0xEE; /* reinit chE mode */
#endif /* DYNAMIC_VGA_SUPPORT */
				RTMP_BBP_IO_WRITE32(pAd, MT76x0_BPP_SWITCH_Tab[Index].RegDate.Register,
						(MT76x0_BPP_SWITCH_Tab[Index].RegDate.Value&(~0x0000FF00))|(eLNAgain << 8));
				
			}
			else
			{
				RTMP_BBP_IO_WRITE32(pAd, MT76x0_BPP_SWITCH_Tab[Index].RegDate.Register,
						MT76x0_BPP_SWITCH_Tab[Index].RegDate.Value);
			}
		}
	}

	if (bScan)
	{
		MT76x0_Calibration(pAd, Channel, FALSE, FALSE);
	}


	RTMPusecDelay(1000);

#ifdef MT76x0_TSSI_CAL_COMPENSATION
	if (pAd->chipCap.bInternalTxALC == FALSE)
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
	{
#ifdef SINGLE_SKU_V2
		USHORT ee_val = 0;		
		UCHAR delta_power = 0;
#endif /* SINGLE_SKU_V2 */

		RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &Value);
		Value = Value & (~0x3F3F);
		Value |= TxPwer;
		Value |= (0x2F2F << 16);
		RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, Value);
		
#ifdef SINGLE_SKU_V2		
		if (Channel > 14)
		{			
			RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_TARGET_POWER, ee_val);
			pAd->DefaultTargetPwr = ee_val & 0x00ff;
#ifdef DOT11_VHT_AC
			if (pAd->CommonCfg.BBPCurrentBW == BW_80)
				delta_power = pAd->chipCap.delta_tw_pwr_bw80;
			else
#endif /* DOT11_VHT_AC */
			delta_power = pAd->chipCap.delta_tw_pwr_bw40_5G;
		}
		else
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_2G_TARGET_POWER, ee_val);
			pAd->DefaultTargetPwr = ee_val & 0x00ff;
			delta_power = pAd->chipCap.delta_tw_pwr_bw40_2G;
		}
		
		if ( (pAd->DefaultTargetPwr == 0x00) || (pAd->DefaultTargetPwr == 0xFF) )
		{
			pAd->DefaultTargetPwr = 0x20;
			DBGPRINT(RT_DEBUG_ERROR, ("%s: EEPROM target power Error! Use Default Target Power = 0x%x\n", 
					__FUNCTION__, pAd->DefaultTargetPwr));
		}
		else
		{
			DBGPRINT(RT_DEBUG_OFF, ("%s: DefaultTargetPwr = %d\n", 
					__FUNCTION__, pAd->DefaultTargetPwr));
		}

		/*
			EEPROM 0x50 - Power delta for 2.4G HT40
			EEPROM 0x51 - Power delta for 5G HT40
			EEPROM 0xD3 - Power delta for VHT80
			Bit<7>: Enable/disable power delta of this BW
			Bit<6>: 0: decrease power, 1: increase power
			Bit<5:0>: Each step represents 0.5dB, range from 0 to 4

			Increase or decrease 0x13b0<5:0> when bandwidth is changed
		*/
		if ((pAd->CommonCfg.BBPCurrentBW != BW_20) 
			 && (delta_power & 0x80))
		{
			if (delta_power & 0x40)
				pAd->DefaultTargetPwr += (delta_power & 0x3F);
			else
				pAd->DefaultTargetPwr -= (delta_power & 0x3F);
		}

		SkuBasePwr = MT76x0_GetSkuChannelBasePwr(pAd, Channel);
		
		if ( pAd->DefaultTargetPwr > SkuBasePwr )
			ChannelPwrAdj = SkuBasePwr - pAd->DefaultTargetPwr;
		else
			ChannelPwrAdj = 0;

		if ( ChannelPwrAdj > 31 )
			ChannelPwrAdj = 31;
		if ( ChannelPwrAdj < -32 )
			ChannelPwrAdj = -32;

		RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &RegValue);
		RegValue = (RegValue & ~0x3F) | (ChannelPwrAdj & 0x3F);
		RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, RegValue);
		DBGPRINT(RT_DEBUG_TRACE, ("SkuBasePwr = 0x%x,  DefaultTargetPwr = 0x%x, ChannelPwrAdj 0x13b4: 0x%x\n", 
			SkuBasePwr, pAd->DefaultTargetPwr, RegValue));

		MT76x0_UpdateSkuPwr(pAd, Channel);
#endif /* SINGLE_SKU_V2 */
	}

#ifndef SINGLE_SKU_V2
	/*
		Read per rate power from EEPROM.
	*/
	MT76x0ReadTxPwrPerRate(pAd);
#endif /* !SINGLE_SKU_V2 */

	percentage_delta_pwr(pAd);

	return;
}


VOID MT76x0_NICInitAsicFromEEPROM(
	IN PRTMP_ADAPTER		pAd)
{
	USHORT e2p_value = 0;

	if (IS_MT7610(pAd) || IS_MT7650(pAd))
	{
		/* MT7650_E3_CR_setting_20130416.xlsx */
		RT28xx_EEPROM_READ16(pAd, 0x24, e2p_value);
		if ((e2p_value & 0x30) == 0x30)
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0x00040200);
		}
		else
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0x00040000);
		}
	}
}

/*
	NOTE: MAX_NUM_OF_CHANNELS shall  equal sizeof(txpwr_chlist))
*/
static UCHAR mt76x0_txpwr_chlist[] = {
	1, 2,3,4,5,6,7,8,9,10,11,12,13,14,
	36,38,40,44,46,48,52,54,56,60,62,64,
	100,102,104,108,110,112,116,118,120,124,126,128,132,134,136,140,
	149,151,153,157,159,161,165,167,169,171,173,
	42, 58, 106, 122, 155,
};

INT MT76x0_ReadChannelPwr(RTMP_ADAPTER *pAd)
{
	UINT32 i, choffset, idx, ss_offset_g, ss_num;
	EEPROM_TX_PWR_STRUC Power;
	CHAR tx_pwr1, tx_pwr2;

	DBGPRINT(RT_DEBUG_TRACE, ("%s()--->\n", __FUNCTION__));
	
	choffset = 0;
	ss_num = 1;

	for (i = 0; i < sizeof(mt76x0_txpwr_chlist); i++)
	{
		pAd->TxPower[i].Channel = mt76x0_txpwr_chlist[i];
		pAd->TxPower[i].Power = DEFAULT_RF_TX_POWER;	
	}


	/* 0. 11b/g, ch1 - ch 14, 1SS */
	ss_offset_g = EEPROM_G_TX_PWR_OFFSET;
	for (i = 0; i < 7; i++)
	{
		idx = i * 2;
		RT28xx_EEPROM_READ16(pAd, ss_offset_g + idx, Power.word);

		tx_pwr1 = tx_pwr2 = DEFAULT_RF_TX_POWER;

		if ((Power.field.Byte0 <= 0x3F) && (Power.field.Byte0 >= 0))
			tx_pwr1 = Power.field.Byte0;

		if ((Power.field.Byte1 <= 0x3F) || (Power.field.Byte1 >= 0))
			tx_pwr2 = Power.field.Byte1;

		pAd->TxPower[idx].Power = tx_pwr1;
		pAd->TxPower[idx + 1].Power = tx_pwr2;
		choffset++;
	}



	{
		/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)*/
		choffset = 14;
		ASSERT((pAd->TxPower[choffset].Channel == 36));
		for (i = 0; i < 6; i++)
		{
			idx = i * 2;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + idx, Power.word);

			if ((Power.field.Byte0 <= 0x3F) && (Power.field.Byte0 >= 0))
				pAd->TxPower[idx + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 <= 0x3F) && (Power.field.Byte1 >= 0))
				pAd->TxPower[idx + choffset + 1].Power = Power.field.Byte1;
		}


		/* 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz)*/
		choffset = 14 + 12;
		ASSERT((pAd->TxPower[choffset].Channel == 100));
		for (i = 0; i < 8; i++)
		{

			idx = i * 2;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + idx, Power.word);
			
			if ((Power.field.Byte0 <= 0x3F) && (Power.field.Byte0 >= 0))
				pAd->TxPower[idx + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 <= 0x3F) && (Power.field.Byte1 >= 0))
				pAd->TxPower[idx + choffset + 1].Power = Power.field.Byte1;
		}


		/* 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165, 167, 169; 171, 173 (including central frequency in BW 40MHz)*/
		choffset = 14 + 12 + 16;
		ASSERT((pAd->TxPower[choffset].Channel == 149));
		for (i = 0; i < 6; i++)
		{
			idx = i * 2;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + idx, Power.word);

			if ((Power.field.Byte0 <= 0x3F) && (Power.field.Byte0 >= 0))
				pAd->TxPower[idx + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 <= 0x3F) && (Power.field.Byte1 >= 0))
				pAd->TxPower[idx + choffset + 1].Power = Power.field.Byte1;
		}

		/* choffset = 14 + 12 + 16 + 7; */
		choffset = 14 + 12 + 16 + 11;

#ifdef DOT11_VHT_AC
		ASSERT((pAd->TxPower[choffset].Channel == 42));

		/* For VHT80MHz, we need assign tx power for central channel 42, 58, 106, 122, and 155 */
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Update Tx power control of the central channel (42, 58, 106, 122 and 155) for VHT BW80\n", __FUNCTION__));
		
		NdisMoveMemory(&pAd->TxPower[53], &pAd->TxPower[16], sizeof(CHANNEL_TX_POWER)); // channel 42 = channel 40
		NdisMoveMemory(&pAd->TxPower[54], &pAd->TxPower[22], sizeof(CHANNEL_TX_POWER)); // channel 58 = channel 56
		NdisMoveMemory(&pAd->TxPower[55], &pAd->TxPower[28], sizeof(CHANNEL_TX_POWER)); // channel 106 = channel 104
		NdisMoveMemory(&pAd->TxPower[56], &pAd->TxPower[34], sizeof(CHANNEL_TX_POWER)); // channel 122 = channel 120
		NdisMoveMemory(&pAd->TxPower[57], &pAd->TxPower[44], sizeof(CHANNEL_TX_POWER)); // channel 155 = channel 153

		pAd->TxPower[choffset].Channel = 42;
		pAd->TxPower[choffset+1].Channel = 58;
		pAd->TxPower[choffset+2].Channel = 106;
		pAd->TxPower[choffset+3].Channel = 122;
		pAd->TxPower[choffset+4].Channel = 155;
		
		choffset += 5;		/* the central channel of VHT80 */
		
		choffset = (MAX_NUM_OF_CHANNELS - 1);
#endif /* DOT11_VHT_AC */


		/* 4. Print and Debug*/
		for (i = 0; i < choffset; i++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: TxPower[%03d], Channel=%d, Power[Tx:%d]\n",
						i, pAd->TxPower[i].Channel, pAd->TxPower[i].Power));
		}
	}

	return TRUE;
}

VOID MT76x0_DisableTxRx(
	RTMP_ADAPTER *pAd,
	UCHAR Level)
{
	UINT32 MacReg = 0;
	UINT32 MTxCycle;
	BOOLEAN bResetWLAN = FALSE;

	if (!IS_MT76x0(pAd))
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("----> %s\n", __FUNCTION__));

	if (Level == RTMP_HALT)
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s Tx success = %ld\n", 
		__FUNCTION__, (ULONG)pAd->WlanCounters.TransmittedFragmentCount.u.LowPart));
	DBGPRINT(RT_DEBUG_TRACE, ("%s Rx success = %lld\n", 
		__FUNCTION__, pAd->WlanCounters.ReceivedFragmentCount.QuadPart));

	StopDmaTx(pAd, Level);

	/*
		Check page count in TxQ,
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		BOOLEAN bFree = TRUE;
		RTMP_IO_READ32(pAd, 0x438, &MacReg);
		if (MacReg != 0)
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa30, &MacReg);
		if (MacReg & 0x000000FF)
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa34, &MacReg);
		if (MacReg & 0xFF00FF00)
			bFree = FALSE;
		if (bFree)
			break;
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check TxQ page count max\n"));
		RTMP_IO_READ32(pAd, 0x0a30, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a30 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x0a34, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a34 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x438, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x438 = 0x%08x\n", MacReg));
		bResetWLAN = TRUE;
	}

	/*
		Check MAC Tx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacReg);
		if (MacReg & 0x1)
			RTMPusecDelay(50);
		else
			break;
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check MAC Tx idle max(0x%08x)\n", MacReg));
		bResetWLAN = TRUE;
	}
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) == FALSE)
	{
		if (Level == RTMP_HALT)
		{
			/*
				Disable MAC TX/RX
			*/
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacReg);
			MacReg &= ~(0x0000000c);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacReg);
		}
		else
		{
			/*
				Disable MAC RX
			*/
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacReg);
			MacReg &= ~(0x00000008);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacReg);
		}
	}

	/*
		Check page count in RxQ,
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		BOOLEAN bFree = TRUE;
		RTMP_IO_READ32(pAd, 0x430, &MacReg);
		if (MacReg & (0x00FF0000))
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa30, &MacReg);
		if (MacReg != 0)
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa34, &MacReg);
		if (MacReg != 0)
			bFree = FALSE;
		if (bFree)
			break;
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}
	
	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check RxQ page count max\n"));
		
		RTMP_IO_READ32(pAd, 0x0a30, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a30 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x0a34, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a34 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x0430, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0430 = 0x%08x\n", MacReg));
		bResetWLAN = TRUE;
	}

	/*
		Check MAC Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacReg);
		if (MacReg & 0x2)
			RTMPusecDelay(50);
		else
			break;
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}
	
	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check MAC Rx idle max(0x%08x)\n", MacReg));
		bResetWLAN = TRUE;
	}

	StopDmaRx(pAd, Level);
	
	if ((Level == RTMP_HALT) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) == FALSE))
	{
		if (IS_MT7610E(pAd) && (pAd->CommonCfg.bIEEE80211H == FALSE))
			NICEraseFirmware(pAd);
		
		/*
			Disable RF/MAC
		*/
#ifdef RTMP_MAC_PCI
		MT76x0_WLAN_ChipOnOff(pAd, FALSE, bResetWLAN);
#endif /* RTMP_MAC_PCI */
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<---- %s\n", __FUNCTION__));
}


#ifdef ED_MONITOR
void MT76x0_Set_ED_CCA(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 MacReg = 0;
	if (enable) {
		RTMP_IO_READ32(pAd, CH_TIME_CFG, &MacReg);
		MacReg |= 0x45;   // enable channel status check
		RTMP_IO_WRITE32(pAd, CH_TIME_CFG, MacReg);

		RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &MacReg);
		MacReg |= (1 << 20);
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, MacReg);
	} else {
		RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &MacReg);
		MacReg &= ~(1 << 20);
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, MacReg);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s::0x%x: 0x%08X\n", __FUNCTION__, TXOP_CTRL_CFG, MacReg));

}
#endif /* ED_MONITOR */

#ifdef DYNAMIC_VGA_SUPPORT
VOID DyncVgaLockTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) FunctionContext;

	pAd->CommonCfg.MO_Cfg.bDyncVgaEnable = TRUE;
	DBGPRINT(RT_DEBUG_ERROR, ("%s - locked for 1 min, resume dynamic vga \n",__FUNCTION__));
}

void MT76x0_UpdateRssiForChannelModel(RTMP_ADAPTER * pAd)
{
	INT32 rx0_rssi, rx1_rssi;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		rx0_rssi = (CHAR)(pAd->ApCfg.RssiSample.LastRssi0);
		rx1_rssi = (CHAR)(pAd->ApCfg.RssiSample.LastRssi1);
	}
#endif /* CONFIG_AP_SUPPORT */

	DBGPRINT(RT_DEBUG_INFO, ("%s:: rx0_rssi(%d), rx1_rssi(%d)\n", 
		__FUNCTION__, rx0_rssi, rx1_rssi));	

	/*
		RSSI_DUT(n) = RSSI_DUT(n-1)*15/16 + RSSI_R2320_100ms_sample*1/16
	*/
	pAd->chipCap.avg_rssi_0 = ((pAd->chipCap.avg_rssi_0) * 15)/16 + (rx0_rssi << 8)/16;
	//pAd->chipCap.avg_rssi_1 = ((pAd->chipCap.avg_rssi_1) * 15)/16 + (rx1_rssi << 8)/16;
	//pAd->chipCap.avg_rssi_all = (pAd->chipCap.avg_rssi_0 + pAd->chipCap.avg_rssi_1)/512;
	if (pAd->MacTab.Size == 0)
		pAd->chipCap.avg_rssi_all = -75;  /* needs to do dync VGA even without any STA  - 20150120 */
	else
		pAd->chipCap.avg_rssi_all = pAd->chipCap.avg_rssi_0 / 256;

	DBGPRINT(RT_DEBUG_INFO, ("%s:: update rssi all(%d)\n", 
		__FUNCTION__, pAd->chipCap.avg_rssi_all));
}

#define shift_left16(x)			((x) << 16)
#define shift_left8(x)			((x) << 8)

BOOLEAN dynamic_channel_model_adjust(RTMP_ADAPTER *pAd)
{
	UCHAR mode = 0, default_init_vga = 0, eLNA_lower_init_vga = 0;
	UINT32 value = 0;
	BOOLEAN no_dynamic_vga = FALSE;

	/* dynamic_chE_mode: 
		bit7		0:average RSSI <= threshold	1:average RSSI > threshold
		bit4:6	0:BW_20		1:BW_40		2:BW_80		3~7:Reserved
		bit1:3	Reserved	
		bit0		0: eLNA		1: iLNA
	*/

	if (((pAd->chipCap.avg_rssi_all > -62) && (pAd->CommonCfg.BBPCurrentBW == BW_80))
		|| ((pAd->chipCap.avg_rssi_all > -65) && (pAd->CommonCfg.BBPCurrentBW == BW_40))
		|| ((pAd->chipCap.avg_rssi_all > -68) && (pAd->CommonCfg.BBPCurrentBW == BW_20))) 
	{
		//RTMP_BBP_IO_WRITE32(pAd, RXO_R18, 0xF000A990);
		if (pAd->CommonCfg.BBPCurrentBW == BW_80) {
			mode = 0xA0; /* BW80::eLNA lower VGA/PD */				
		} else if (pAd->CommonCfg.BBPCurrentBW == BW_40) {
			mode = 0x90; /* BW40::eLNA lower VGA/PD */
		} else if (pAd->CommonCfg.BBPCurrentBW == BW_20) {
			mode = 0x80; /* BW20::eLNA lower VGA/PD */
		}
	} else {
		if (pAd->CommonCfg.BBPCurrentBW == BW_80) {
			mode = 0x20; /* BW80::eLNA default */			
		} else if (pAd->CommonCfg.BBPCurrentBW == BW_40) {
			mode = 0x10; /* BW40::eLNA default */
		} else if (pAd->CommonCfg.BBPCurrentBW == BW_20) {
			mode = 0x00; /* BW20::eLNA default */
		}
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s:: dynamic ChE mode(0x%x)\n", 
		__FUNCTION__, mode));

	if (((pAd->chipCap.avg_rssi_all <= -76) && (pAd->CommonCfg.BBPCurrentBW == BW_80))
		|| ((pAd->chipCap.avg_rssi_all <= -79) && (pAd->CommonCfg.BBPCurrentBW == BW_40))
		|| ((pAd->chipCap.avg_rssi_all <= -82) && (pAd->CommonCfg.BBPCurrentBW == BW_20)))
	{
		no_dynamic_vga = TRUE; /* rssi too low, stop dynamic VGA , resume initial gain */
	}
	
	if (((mode & 0xFF) != pAd->chipCap.dynamic_chE_mode) || no_dynamic_vga) {
		default_init_vga = pAd->CommonCfg.MO_Cfg.Stored_BBP_R66;  			//44
		eLNA_lower_init_vga = pAd->CommonCfg.MO_Cfg.Stored_BBP_R66 - 8;		//3C
	
		/* VGA settings : MT7610E_DynamicVGA_plus_micro_wave_20150120.pptx */
		switch (mode & 0xFF)
		{
			case 0xA0: /* BW80::eLNA lower VGA/PD */
				pAd->chipCap.dynamic_chE_mode = 0xA0;
				//RTMP_BBP_IO_WRITE32(pAd, AGC1_R35, 0x08080808); /* BBP 0x238C */
				//RTMP_BBP_IO_WRITE32(pAd, AGC1_R37, 0x08080808); /* BBP 0x2394 */
				value = shift_left16(0x122C) | shift_left8(eLNA_lower_init_vga) | 0xF2;
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, value); /* BBP 0x2320 */
				
				break;
		
			case 0x90: /* BW40::eLNA lower VGA/PD */
				pAd->chipCap.dynamic_chE_mode = 0x90;
				value = shift_left16(0x122C) | shift_left8(eLNA_lower_init_vga) | 0xF2;
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, value); /* BBP 0x2320 */				
				break;
		
			case 0x80: /* BW20::eLNA lower VGA/PD */
				pAd->chipCap.dynamic_chE_mode = 0x80;
				value = shift_left16(0x122C) | shift_left8(eLNA_lower_init_vga) | 0xF2;
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, value); /* BBP 0x2320 */
				break;			
		
			case 0x20: /* BW80::eLNA default */
				pAd->chipCap.dynamic_chE_mode = 0x20;
				value = shift_left16(0x122C) | shift_left8(default_init_vga) | 0xF2;
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, value); /* BBP 0x2320 */
				break;
		
			case 0x10: /* BW40::eLNA default */
				pAd->chipCap.dynamic_chE_mode = 0x10;
				value = shift_left16(0x122C) | shift_left8(default_init_vga) | 0xF2;
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, value); /* BBP 0x2320 */
				break;		
		
			case 0x00: /* BW20::eLNA default */
				pAd->chipCap.dynamic_chE_mode = 0x00;
				value = shift_left16(0x122C) | shift_left8(default_init_vga) | 0xF2;
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, value); /* BBP 0x2320 */
				break;
		
			default:
				DBGPRINT(RT_DEBUG_ERROR, ("%s:: no such dynamic ChE mode(0x%x)\n", 
					__FUNCTION__, mode));
				break;
		}

		DBGPRINT(RT_DEBUG_INFO, ("%s:: updated dynamic_chE_mode(0x%x)\n", 
			__FUNCTION__, pAd->chipCap.dynamic_chE_mode));
	} 

	return no_dynamic_vga;
}

void MT76x0_AsicDynamicVgaGainControl(RTMP_ADAPTER *pAd)
{
	if (dynamic_channel_model_adjust(pAd) == TRUE) {
		DBGPRINT(RT_DEBUG_INFO, ("%s:: no need to do dynamic vga\n", __FUNCTION__));			
		return;
	}	
	
	if ((pAd->CommonCfg.MO_Cfg.bDyncVgaEnable) 
	//&& (pAd->MacTab.Size > 0)   /* needs to do dync VGA even without any STA  - 20150120 */
	&& OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
	{
		UCHAR val;
		UINT32 bbp_val, bbp_reg = AGC1_R8;

		RTMP_BBP_IO_READ32(pAd, bbp_reg, &bbp_val);
		val = ((bbp_val & (0x0000ff00)) >> 8) & 0xff;

		DBGPRINT(RT_DEBUG_TRACE,
			("0x10 one second False CCA=%d, fixed R66 at 0x%x\n", pAd->RalinkCounters.OneSecFalseCCACnt, val));

		if (pAd->RalinkCounters.OneSecFalseCCACnt > pAd->CommonCfg.MO_Cfg.nFalseCCATh)
		{
			if (val > (pAd->CommonCfg.MO_Cfg.Stored_BBP_R66 - 0x10))
			{
				val -= 2;
				bbp_val = (bbp_val & 0xffff00ff) | (val << 8);
				RTMP_BBP_IO_WRITE32(pAd, bbp_reg, bbp_val);
#ifdef DFS_SUPPORT
				pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif

				/* gain down-up-down-up detection */
				if(pAd->CommonCfg.MO_Cfg.bPreviousTuneVgaUP)
					pAd->CommonCfg.MO_Cfg.TuneGainReverseTimes++;						
				else /* tune down 2 times, cancel lock detect */
					pAd->CommonCfg.MO_Cfg.TuneGainReverseTimes = 0;

				pAd->CommonCfg.MO_Cfg.bPreviousTuneVgaUP = FALSE; /* record this time's action */
					
			}
		}
		else if (pAd->RalinkCounters.OneSecFalseCCACnt < pAd->CommonCfg.MO_Cfg.nLowFalseCCATh)
		{
			if (val < pAd->CommonCfg.MO_Cfg.Stored_BBP_R66)
			{
				val += 2;
				bbp_val = (bbp_val & 0xffff00ff) | (val << 8);
				RTMP_BBP_IO_WRITE32(pAd, bbp_reg, bbp_val);
#ifdef DFS_SUPPORT
			    pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif
				/* gain down-up-down-up detection */
				if(pAd->CommonCfg.MO_Cfg.bPreviousTuneVgaUP == FALSE)
					pAd->CommonCfg.MO_Cfg.TuneGainReverseTimes++;							
				else /* tune down 2 times, cancel lock detect */
					pAd->CommonCfg.MO_Cfg.TuneGainReverseTimes = 0;

				pAd->CommonCfg.MO_Cfg.bPreviousTuneVgaUP = TRUE; /* record this time's action */
			}
		}
		else
		{
			pAd->CommonCfg.MO_Cfg.TuneGainReverseTimes = 0; /* no up or down this time, cancel lock detect */
		}

		/* gain down-up-down-up detected */
		if(pAd->CommonCfg.MO_Cfg.TuneGainReverseTimes >= 4)
		{
			ULONG Timeout = 30000; /*30 sec*/
			pAd->CommonCfg.MO_Cfg.bDyncVgaEnable = FALSE;
			pAd->CommonCfg.MO_Cfg.TuneGainReverseTimes = 0;
			RTMPSetTimer(&pAd->CommonCfg.MO_Cfg.DyncVgaLockTimer, Timeout);
			if(pAd->CommonCfg.MO_Cfg.bPreviousTuneVgaUP)
			{
				val -= 2;
				bbp_val = (bbp_val & 0xffff00ff) | (val << 8);
				RTMP_BBP_IO_WRITE32(pAd, bbp_reg, bbp_val);
			}
			DBGPRINT(RT_DEBUG_ERROR, ("%s - Dynamic VGA gain reversed 2 times, lock gain to 0x%x\n",__FUNCTION__,val));
		}
		
#ifdef ED_MONITOR
		//dynamic_ed_cca_threshold_adjust(pAd);
#endif
	}
}

#endif /*DYNAMIC_VGA_SUPPORT*/

VOID MT76x0_AsicExtraPowerOverMAC(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 ExtraPwrOverMAC = 0;
	UINT32 ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	/* 
		For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 
		bit 21:16 -> HT MCS=7, VHT 2SS MCS=7
		bit 5:0 -> OFDM 54
	*/
	RTMP_IO_READ32(pAd, TX_PWR_CFG_1, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x00003F00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, TX_PWR_CFG_2, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x00003F00) << 8; /* Get Tx power for HT MCS 7 */			
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

	/* 
		For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC 
		bit 5:0 -> HT/VHT STBC MCS=7
	*/
	RTMP_IO_READ32(pAd, TX_PWR_CFG_4, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x00003F00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);
	
	DBGPRINT(RT_DEBUG_INFO, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}

static	VOID MT76x0_CalculateTxpower(
	IN  BOOLEAN bMinus,
	IN  USHORT InputTxpower,
	IN  USHORT DeltaTxpower,
	OUT  UCHAR *pTxpower1,
	OUT  UCHAR *pTxpower2)
{
	UCHAR t1, t2;
	
	if (bMinus == FALSE)
	{
		if (InputTxpower & 0x20)
		{
			t1 = (UCHAR)((InputTxpower & 0x3F) +  DeltaTxpower);

			if (t1 >= 0x40)
				t1 -= 0x40;
		}
		else
		{
			t1 = (InputTxpower & 0x1F) + (DeltaTxpower);
			if (t1 > 0x1F)
				t1 = 0x1F;
		}

		if (InputTxpower & 0x2000)
		{
			t2 = (UCHAR)(((InputTxpower & 0x3F00) >> 8) + DeltaTxpower);

			if (t2 >= 0x40)
				t2 -= 0x40;
		}
		else
		{
			t2 = ((InputTxpower & 0x1F00) >> 8) + (DeltaTxpower);
			if (t2 > 0x1F)
				t2 = 0x1F;
		}			
	}
	else
	{
		if (InputTxpower & 0x20)
		{
			t1 = (InputTxpower & 0x3F) - (DeltaTxpower);

			if (t1 < 0x21)
				t1 = 0x21;
		}
		else
		{
			t1 = (InputTxpower & 0x1F) - (DeltaTxpower);

			/* 8-bit representation ==> 6-bit representation (2's complement) */
			t1 = (t1 & 0x3F);
		}

		if (InputTxpower & 0x2000)
		{
			t2 = ((InputTxpower & 0x3F00) >> 8) - (DeltaTxpower);

			if (t2 < 0x21)
				t2 = 0x21;
		}
		else
		{
			t2 = ((InputTxpower & 0x1F00) >> 8) - (DeltaTxpower);

			/* 8-bit representation ==> 6-bit representation (2's complement) */
			t2 = (t2 & 0x3F);
		}			
	}

	*pTxpower1 = t1;
	*pTxpower2 = t2;
}

#define EEPROM_TXPOWER_BYRATE_STBC	(0xEC)
#define EEPROM_TXPOWER_BYRATE_5G	(0x120)
//
// VHT BW80 delta power control (+4~-4dBm) for per-rate Tx power control
//
#define EEPROM_VHT_BW80_TX_POWER_DELTA	(0xD3)

//
// Read per-rate Tx power
//
VOID MT76x0ReadTxPwrPerRate(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 data;
	USHORT value;
	UCHAR TxPwrBw40ABand, TxPwrBw80ABand, TxPwrBw40GBand;
	UCHAR t1, t2, t3, t4;
	BOOLEAN bMinusBw40ABand = FALSE, bMinusBw80ABand = FALSE,bMinusBw40GBand = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s() -->\n", __FUNCTION__));

#ifdef MT76x0_TSSI_CAL_COMPENSATION
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);
	if (value == 0xFFFF) /*EEPROM is empty*/
	   	pAd->chipCap.bInternalTxALC = FALSE;
	else if (value & 1<<13) 
	   	pAd->chipCap.bInternalTxALC = TRUE;
	else 
	   	pAd->chipCap.bInternalTxALC = FALSE;

	DBGPRINT(RT_DEBUG_OFF, ("TXALC> bInternalTxALC = %d\n", pAd->chipCap.bInternalTxALC));
#endif /* MT76x0_TSSI_CAL_COMPENSATION */

	//
	// Get power delta for BW40
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value);
	TxPwrBw40ABand = 0;
	TxPwrBw40GBand = 0;


	pAd->chipCap.delta_tw_pwr_bw40_2G = \
							(value & 0xFF) == 0xFF ? 0 : (value & 0xFF);
	pAd->chipCap.delta_tw_pwr_bw40_5G = \
							(value & 0xFF00) == 0xFF00 ? 0 : ((value >> 8) & 0xFF);

	if (pAd->chipCap.bInternalTxALC == FALSE)

	{
		if ((value & 0xFF) != 0xFF)
		{
			if (value & 0x80)  /* bit7: Enable/Disable */
				TxPwrBw40GBand = (value & 0x1F);
		
			if (value & 0x40)  /* bit6: sign bit */
				bMinusBw40GBand = FALSE;
			else
				bMinusBw40GBand = TRUE;
		}
		
		if ((value & 0xFF00) != 0xFF00)
		{
			if (value & 0x8000) /* bit7: Enable/Disable */
				TxPwrBw40ABand = ((value&0x1F00) >> 8);

			if (value & 0x4000) /* bit6: sign bit */
				bMinusBw40ABand = FALSE;
			else
				bMinusBw40ABand = TRUE;
		}
	}


	//
	// Get power delta for BW80
	//
	RT28xx_EEPROM_READ16(pAd, EEPROM_VHT_BW80_TX_POWER_DELTA, value);
	TxPwrBw80ABand = 0;

	pAd->chipCap.delta_tw_pwr_bw80 = \
							(value & 0xFF) == 0xFF ? 0 : (value & 0xFF);
	if (pAd->chipCap.bInternalTxALC == FALSE)
	{
		if ((value & 0xFF) != 0xFF)
		{
			if (value & 0x80) /* bit7: Enable/Disable */
				TxPwrBw80ABand = (value & 0x1F);
		
			if (value & 0x40) /* bit6: sign bit */
				bMinusBw80ABand = FALSE;
			else
				bMinusBw80ABand = TRUE;
		}
	}

#ifdef SINGLE_SKU_V2
	/*
		We don't need to update bw delta for per rate when SingleSKU is enabled.
	*/
	bMinusBw40ABand = FALSE;
	bMinusBw80ABand = FALSE;
	bMinusBw40GBand = FALSE;
	TxPwrBw40ABand = 0;
	TxPwrBw80ABand = 0;
	TxPwrBw40GBand = 0;
#endif /* SINGLE_SKU_V2 */

	DBGPRINT(RT_DEBUG_TRACE, ("%s: bMinusBw40ABand = %d, bMinusBw80ABand = %d, bMinusBw40GBand = %d\n", 
		__FUNCTION__, 
		bMinusBw40ABand, 
		bMinusBw80ABand, 
		bMinusBw40GBand));

	DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPwrBw40ABand = %d, TxPwrBw80ABand = %d, TxPwrBw40GBand = %d\n", 
		__FUNCTION__, 
		TxPwrBw40ABand, 
		TxPwrBw80ABand, 
		TxPwrBw40GBand));

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G, value);
	MT76x0_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 2), value);
	MT76x0_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t3, &t4);
	/* 
		bit 29:24 -> OFDM 12M/18M
		bit 21:16 -> OFDM 6M/9M
		bit 13:8 -> CCK 5.5M/11M
		bit 5:0 -> CCK 1M/2M
	*/
	data = (t4 << 24) | (t3 << 16) | (t2 << 8) | t1; 
	pAd->Tx20MPwrCfgGBand[0] = data; /* TX_PWR_CFG_0, MAC 0x1314 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx20MPwrCfgGBand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4), value);
	MT76x0_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
#ifdef MT76x0_TSSI_CAL_COMPENSATION
	pAd->chipCap.efuse_2G_54M_tx_power = t2;
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 6), value);
	MT76x0_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t3, &t4);
	/* 
		bit 29:24 -> HT MCS=2,3, VHT 1SS MCS=2,3
		bit 21:16 -> HT MCS=0,1, VHT 1SS MCS=0,1
		bit 13:8 -> OFDM 48M
		bit 5:0 -> OFDM 24M/36M
	*/
	data = (t4 << 24) | (t3 << 16) | (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgGBand[0] = data; /* TX_PWR_CFG_1, MAC 0x1318 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 8), value);
	MT76x0_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
	/*
		bit 13:8 -> HT MCS=6, VHT 1SS MCS=6
		bit 5:0 -> MCS=4,5, VHT 1SS MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgGBand[1] = data; /* TX_PWR_CFG_2, MAC 0x131C */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[1] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 14), value);
	MT76x0_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t3, &t4);
	/* 
		bit 29:24 -> HT/VHT STBC MCS=2, 3
		bit 21:16 -> HT/VHT STBC MCS=0, 1
	*/
	data = (t4 << 24) | (t3 << 16); 
	pAd->Tx40MPwrCfgGBand[2] = data; /* TX_PWR_CFG_3, MAC 0x1320 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[2] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 16), value);
	MT76x0_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
	/* 
		bit 13:8 -> HT/VHT STBC MCS=6
		bit 5:0 -> HT/VHT STBC MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgGBand[3] = data; /* TX_PWR_CFG_4, MAC 0x1324 */			
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[3] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_5G, value);
	MT76x0_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t3, &t4);
	/* 
		bit 29:24 -> OFDM 12M/18M
		bit 21:16 -> OFDM 6M/9M
	*/
	data = (t4 << 24) | (t3 << 16); 
	pAd->Tx20MPwrCfgABand[0] = data; /* TX_PWR_CFG_0, MAC 0x1314 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx20MPwrCfgABand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_5G + 2), value);
	MT76x0_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t1, &t2);
#ifdef MT76x0_TSSI_CAL_COMPENSATION
	pAd->chipCap.efuse_5G_54M_tx_power = t2;
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_5G + 4), value);
	MT76x0_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t3, &t4);
	/* 
		bit 29:24 -> HT MCS=2,3, VHT 1SS MCS=2,3
		bit 21:16 -> HT MCS=0,1, VHT 1SS MCS=0,1
		bit 13:8 -> OFDM 48M
		bit 5:0 -> OFDM 24M/36M
	*/
	data = (t4 << 24) | (t3 << 16) | (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgABand[0] = data; /* TX_PWR_CFG_1, MAC 0x1318 */			
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_5G + 6), value);
	MT76x0_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t1, &t2);
	/*
		bit 13:8 -> HT MCS=6, VHT 1SS MCS=6
		bit 5:0 -> MCS=4,5, VHT 1SS MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgABand[1] = data; /* TX_PWR_CFG_2, MAC 0x131C */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[1] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_STBC), value);
	MT76x0_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t3, &t4);
	/* 
		bit 29:24 -> HT/VHT STBC MCS=2, 3
		bit 21:16 -> HT/VHT STBC MCS=0, 1
	*/
	data = (t4 << 24) | (t3 << 16); 
	pAd->Tx40MPwrCfgABand[2] = data; /* TX_PWR_CFG_3, MAC 0x1320 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[2] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_STBC + 2), value);
	MT76x0_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t1, &t2);
	/* 
		bit 13:8 -> HT/VHT STBC MCS=6
		bit 5:0 -> HT/VHT STBC MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgABand[3] = data; /* TX_PWR_CFG_4, MAC 0x1324 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[3] = 0x%X\n", __FUNCTION__, data));
	
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_5G + 12), value);
	MT76x0_CalculateTxpower(bMinusBw80ABand, value, TxPwrBw80ABand, &t3, &t4);
	/* 
		bit 29:24 -> VHT 1SS MCS=9
		bit 21:16 -> VHT 1SS MCS=8
	*/
	data = (t3 << 24) | (t3 << 16); 
	pAd->Tx80MPwrCfgABand[0] = data; /* TX_PWR_CFG_8, MAC 0x13D8 */			
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx80MPwrCfgABand[0] = 0x%X\n", __FUNCTION__, data));

	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0, pAd->Tx20MPwrCfgABand[0]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_1, pAd->Tx40MPwrCfgABand[0]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_2, pAd->Tx40MPwrCfgABand[1]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_3, pAd->Tx40MPwrCfgABand[2]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_4, pAd->Tx40MPwrCfgABand[3]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, pAd->Tx80MPwrCfgABand[0]);
	MT76x0_AsicExtraPowerOverMAC(pAd);

	MT76x0_MakeUpRatePwrTable(pAd);
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}

#ifdef RTMP_MAC_PCI
#ifdef DBG
VOID MT76x0_ShowDmaIndexCupIndex(
	RTMP_ADAPTER *pAd)
{
	UINT32 RegVal = 0, index;

	/*
		RX
	*/
	for(index = 0; index < RX_RING_NUM; index++)
	{
		UINT32 RegVal = 0;
		RTMP_IO_READ32(pAd, RX_RING_CIDX + index * 0x10, &RegVal);
		printk(">>>> 0x%08x = 0x%08x\n", RX_RING_CIDX + index * 0x10, RegVal);
		RTMP_IO_READ32(pAd, RX_RING_DIDX + index * 0x10, &RegVal);
		printk(">>>> 0x%08x = 0x%08x\n", RX_RING_DIDX + index * 0x10, RegVal);
	}

	/*
		TX
	*/
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
		UINT32 RegVal = 0;
		RTMP_IO_READ32(pAd, TX_RING_CIDX + index * 0x10, &RegVal);
		printk(">>>> 0x%08x = 0x%08x\n", TX_RING_CIDX + index * 0x10, RegVal);
	}

	/*
		MGMT
	*/
	RTMP_IO_READ32(pAd, TX_MGMT_CIDX, &RegVal);
	printk(">>>> 0x%08x = 0x%08x\n", TX_MGMT_CIDX, RegVal);
	RTMP_IO_READ32(pAd, TX_MGMT_DIDX, &RegVal);
	printk(">>>> 0x%08x = 0x%08x\n", TX_MGMT_DIDX, RegVal);

	/*
		CTRL
	*/
	RTMP_IO_READ32(pAd, TX_CTRL_CIDX, &RegVal);
	printk(">>>> 0x%08x = 0x%08x\n", TX_CTRL_CIDX, RegVal);
	RTMP_IO_READ32(pAd, TX_CTRL_DIDX, &RegVal);
	printk(">>>> 0x%08x = 0x%08x\n", TX_CTRL_DIDX, RegVal);
}
#endif /* DBG */
#endif

static VOID MT76x0_AsicGetTxPowerOffset(
	IN PRTMP_ADAPTER pAd,
	INOUT PULONG pTxPwr)
{
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;
	DBGPRINT(RT_DEBUG_INFO, ("-->MT76x0_AsicGetTxPowerOffset\n"));

	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	/* MAC 0x1314, 0x1318, 0x131C, 0x1320, 1324, 0x13D8 */
	CfgOfTxPwrCtrlOverMAC.NumOfEntries = MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS; 

	CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
	CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
	CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
	CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
	CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
	if (pAd->CommonCfg.CentralChannel > 14)
	{
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgABand[0];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgABand[0];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgABand[1];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgABand[2];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgABand[3];
	}
	else
	{
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgGBand[0];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgGBand[0];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgGBand[1]; 
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgGBand[2]; 
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgGBand[3];
	}
	
#ifdef DOT11_VHT_AC
	CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].MACRegisterOffset = TX_PWR_CFG_8;
	CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[5].RegisterValue = pAd->Tx80MPwrCfgABand[0];
#endif /* DOT11_VHT_AC */

	NdisCopyMemory(pTxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));
	DBGPRINT(RT_DEBUG_INFO, ("<--MT76x0_AsicGetTxPowerOffset\n"));
}

/*
========================================================================
Routine Description:
	Initialize MT76x0

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID MT76x0_Init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	UINT32 Value;

	DBGPRINT(RT_DEBUG_TRACE, ("-->%s():\n", __FUNCTION__));

	/* 
		Init chip capabilities
	*/
	RTMP_IO_READ32(pAd, 0x00, &Value);	
	pChipCap->ChipID = Value;

	pChipCap->MaxNss = 1;
	pChipCap->TXWISize = 20;
	pChipCap->RXWISize = 28;
#ifdef RTMP_MAC_PCI
	pChipCap->WPDMABurstSIZE = 3;
#endif

	pChipCap->SnrFormula = SNR_FORMULA2;
	pChipCap->FlgIsHwWapiSup = TRUE;
	pChipCap->VcoPeriod = 10;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_3;
	pChipCap->FlgIsHwAntennaDiversitySup = FALSE;
#ifdef STREAM_MODE_SUPPORT
	pChipCap->FlgHwStreamMode = FALSE;
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	pChipCap->FlgHwTxBfCap = FALSE;
#endif /* TXBF_SUPPORT */
#ifdef FIFO_EXT_SUPPORT
	pChipCap->FlgHwFifoExtCap = TRUE;
#endif /* FIFO_EXT_SUPPORT */


	pChipCap->asic_caps |= (fASIC_CAP_PMF_ENC);
	pChipCap->asic_caps |= (fASIC_CAP_MCS_LUT);
	pChipCap->phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G);
	pChipCap->phy_caps |= (fPHY_CAP_HT | fPHY_CAP_VHT);

	pChipCap->RfReg17WtMethod = RF_REG_WT_METHOD_STEP_ON;
		
	pChipCap->MaxNumOfRfId = MAX_RF_ID;
	pChipCap->pRFRegTable = NULL;

	pChipCap->MaxNumOfBbpId = 200;	
	pChipCap->pBBPRegTable = NULL;
	pChipCap->bbpRegTbSize = 0;

#ifdef DFS_SUPPORT
	pChipCap->DfsEngineNum = 4;
#endif /* DFS_SUPPORT */

#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	pChipCap->MBSSIDMode = MBSSID_MODE4;
#else
	pChipCap->MBSSIDMode = MBSSID_MODE1;
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	pChipCap->MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */



#ifdef RTMP_EFUSE_SUPPORT
	pChipCap->EFUSE_USAGE_MAP_START = 0x1e0;
	pChipCap->EFUSE_USAGE_MAP_END = 0x1FC;      
	pChipCap->EFUSE_USAGE_MAP_SIZE = 29;
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef CONFIG_ANDES_SUPPORT
	pChipCap->WlanMemmapOffset = 0x410000;
	pChipCap->InbandPacketMaxLen = 192;
	pChipCap->CmdRspRxRing = RX_RING1;
	if (IS_MT7610E(pAd))
		pChipCap->IsComboChip = FALSE;
	else
		pChipCap->IsComboChip = TRUE;
#endif

	pChipCap->MCUType = ANDES;

#ifdef MT7650
	if (IS_MT7650(pAd))
		pChipCap->FWImageName = MT7650_FirmwareImage;
#endif

#ifdef MT7630
	if (IS_MT7630(pAd))
		pChipCap->FWImageName = MT7650_FirmwareImage;
#endif

#ifdef MT7610
	if (IS_MT7610(pAd))
		pChipCap->FWImageName = MT7610_FirmwareImage;
#endif

#ifdef DYNAMIC_VGA_SUPPORT
	pChipCap->dynamic_vga_support = TRUE;
	pChipCap->compensate_level = 0;
	pChipCap->avg_rssi_all = -90;
	pChipCap->avg_rssi_0 = -90;
	pChipCap->avg_rssi_1 = -90;
	pChipCap->dynamic_chE_mode = 0xEE;
	pChipOps->AsicDynamicVgaGainControl = MT76x0_AsicDynamicVgaGainControl;
	pChipOps->UpdateRssiForDynamicVga = MT76x0_UpdateRssiForChannelModel;
#endif /* DYNAMIC_VGA_SUPPORT */

	pChipCap->bDoTemperatureSensor = TRUE;

	pChipCap->MACRegisterVer = "MT7650_E6_MAC_CR_setting_20141027.xlsx";
	pChipCap->BBPRegisterVer = "MT7650E6_BBP_CR_20140922.xls";
	pChipCap->RFRegisterVer = "MT7650E6_WiFi_RF_CR_20140114.xls";

	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_GRP);
		
	/*
		Following function configure beacon related parameters
		in pChipCap
			FlgIsSupSpecBcnBuf / BcnMaxHwNum / 
			WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	*/
	rlt_bcn_buf_init(pAd);

	/*
		init operator
	*/

	/* BBP adjust */
	pChipOps->ChipBBPAdjust = MT76x0_ChipBBPAdjust;
	

	/* Channel */
	pChipOps->ChipSwitchChannel = MT76x0_ChipSwitchChannel;
	pChipOps->ChipAGCInit = NULL;

	pChipOps->AsicMacInit = NICInitMT76x0MacRegisters;
	pChipOps->AsicBbpInit = NICInitMT76x0BbpRegisters;
	pChipOps->AsicRfInit = NICInitMT76x0RFRegisters;
	pChipOps->AsicRfTurnOn = NULL;

	pChipOps->AsicHaltAction = NULL;
	pChipOps->AsicRfTurnOff = NULL;
	pChipOps->AsicReverseRfFromSleepMode = NULL;
	pChipOps->AsicResetBbpAgent = NULL;
	
	/* MAC */

	/* EEPROM */
	pChipOps->NICInitAsicFromEEPROM = MT76x0_NICInitAsicFromEEPROM;
	
	/* Antenna */
	pChipOps->AsicAntennaDefaultReset = MT76x0_AsicAntennaDefaultReset;

	/* TX ALC */
	pChipOps->InitDesiredTSSITable = NULL;
 	pChipOps->ATETssiCalibration = NULL;
	pChipOps->ATETssiCalibrationExtend = NULL;
	pChipOps->AsicTxAlcGetAutoAgcOffset = NULL;
	pChipOps->ATEReadExternalTSSI = NULL;
	pChipOps->TSSIRatio = NULL;
	
	/* Others */
#ifdef CARRIER_DETECTION_SUPPORT
	pAd->chipCap.carrier_func = TONE_RADAR_V3;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v3;
#endif /* CARRIER_DETECTION_SUPPORT */

	/* Chip tuning */
	pChipOps->RxSensitivityTuning = NULL;
	pChipOps->AsicTxAlcGetAutoAgcOffset = NULL;
	pChipOps->AsicGetTxPowerOffset = MT76x0_AsicGetTxPowerOffset;
	pChipOps->AsicExtraPowerOverMAC = MT76x0_AsicExtraPowerOverMAC;

/* 
	Following callback functions already initiailized in RtmpChipOpsHook() 
	1. Power save related
*/
#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = NULL;
	pChipOps->DisableAPMIMOPS = NULL;
#endif /* GREENAP_SUPPORT */
	
#ifdef ED_MONITOR
    	pChipOps->ChipSetEDCCA = MT76x0_Set_ED_CCA;
#else
    	pChipOps->ChipSetEDCCA = NULL;
#endif /* ED_MONITOR */


#ifdef HDR_TRANS_SUPPORT
	if (1) {
		/* enable TX/RX Header Translation */
		RTMP_IO_WRITE32(pAd, HT_RX_WCID_EN_BASE , 0xFF);	/* all RX WCID enable */

		/* black list - skip EAP-888e/DLS-890d */
		RTMP_IO_WRITE32(pAd, HT_RX_BL_BASE, 0x888e890d);
		//RTMP_IO_WRITE32(pAd, HT_RX_BL_BASE, 0x08000806);

		/* tsc conrotl */
/*
		RTMP_IO_READ32(pAd, 0x250, &RegVal);
		RegVal |= 0x6000;
		RTMP_IO_WRITE32(pAd, 0x250, RegVal);
*/
	}	
#endif /* HDR_TRANS_SUPPORT */
}

VOID MT76x0_WLAN_ChipOnOff(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN bOn,
	IN BOOLEAN bResetWLAN)
{
	WLAN_FUN_CTRL_STRUC WlanFunCtrl = {.word=0};

#ifdef RTMP_MAC_PCI
#ifdef RTMP_FLASH_SUPPORT
	MT76x0_ReadFlashAndInitAsic(pAd);
#endif /* RTMP_FLASH_SUPPORT */

	RTMP_SEM_LOCK(&pAd->WlanEnLock);
#endif /* RTMP_MAC_PCI */



	RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &WlanFunCtrl.word);
	DBGPRINT(RT_DEBUG_OFF, ("==>%s(): OnOff:%d, pAd->WlanFunCtrl:0x%x, Reg-WlanFunCtrl=0x%x\n",
				__FUNCTION__, bOn, pAd->WlanFunCtrl.word, WlanFunCtrl.word));

	if (bResetWLAN == TRUE)
	{
		WlanFunCtrl.field.GPIO0_OUT_OE_N = 0xFF;
		WlanFunCtrl.field.FRC_WL_ANT_SET = 0;

		if (pAd->WlanFunCtrl.field.WLAN_EN)
		{
			/*
				Restore all HW default value and reset RF.
			*/					
			WlanFunCtrl.field.WLAN_RESET = 1;
			WlanFunCtrl.field.WLAN_RESET_RF = 1;
			DBGPRINT(RT_DEBUG_TRACE, ("Reset(1) WlanFunCtrl.word = 0x%x\n", WlanFunCtrl.word));
			RTMP_IO_FORCE_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);	
			RTMPusecDelay(2);
			WlanFunCtrl.field.WLAN_RESET = 0;
			WlanFunCtrl.field.WLAN_RESET_RF = 0;
			DBGPRINT(RT_DEBUG_TRACE, ("Reset(2) WlanFunCtrl.word = 0x%x\n", WlanFunCtrl.word));
			RTMP_IO_FORCE_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
			RTMPusecDelay(2);
		}
	}

	if (bOn == TRUE)
	{
		/*
			Enable WLAN function and clock
			WLAN_FUN_CTRL[1:0] = 0x3
		*/
		ENABLE_WLAN_FUN(WlanFunCtrl);		
	}
	else
	{
		/*
			Diable WLAN function and clock
			WLAN_FUN_CTRL[1:0] = 0x0
		*/
		DISABLE_WLAN_FUN(WlanFunCtrl);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("WlanFunCtrl.word = 0x%x\n", WlanFunCtrl.word));
	RTMP_IO_FORCE_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);	
	RTMPusecDelay(2);

	if (bOn)
	{
		RTMP_IO_FORCE_READ32(pAd, MAC_CSR0, &pAd->MACVersion);
		DBGPRINT(RT_DEBUG_OFF, ("MACVersion = 0x%08x\n", pAd->MACVersion));
	}

	if (bOn == TRUE)
	{
		UINT index = 0;		
		CMB_CTRL_STRUC CmbCtrl;
		
		CmbCtrl.word = 0;
				
		do
		{
			do 
			{
				RTMP_IO_FORCE_READ32(pAd, CMB_CTRL, &CmbCtrl.word);

				/*
					Check status of PLL_LD & XTAL_RDY.
					HW issue: Must check PLL_LD&XTAL_RDY when setting EEP to disable PLL power down
				*/
				if ((CmbCtrl.field.PLL_LD == 1) && (CmbCtrl.field.XTAL_RDY == 1))
					break;

				RTMPusecDelay(20);
			} while (index++ < MAX_CHECK_COUNT);

			if (index >= MAX_CHECK_COUNT)
			{
				DBGPRINT(RT_DEBUG_ERROR, 
						("Lenny:[boundary]Check PLL_LD ..CMB_CTRL 0x%08x, index=%d!\n",
						CmbCtrl.word, index));
				/*
					Disable WLAN then enable WLAN again
				*/
				DISABLE_WLAN_FUN(WlanFunCtrl);
				RTMP_IO_FORCE_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
				RTMPusecDelay(2);

				ENABLE_WLAN_FUN(WlanFunCtrl);
				RTMP_IO_FORCE_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl.word);
				RTMPusecDelay(2);
				index=0;
			}
			else
			{
				break;
			}
		}			
		while (TRUE);
	}

	pAd->WlanFunCtrl.word = WlanFunCtrl.word;
	RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &WlanFunCtrl.word);
	DBGPRINT(RT_DEBUG_TRACE,
		("<== %s():  pAd->WlanFunCtrl.word = 0x%x, Reg->WlanFunCtrl=0x%x!\n",
		__FUNCTION__, pAd->WlanFunCtrl.word, WlanFunCtrl.word));

#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->WlanEnLock);
#endif /* RTMP_MAC_PCI */

}

VOID MT76x0_AntennaSelCtrl(
	IN RTMP_ADAPTER *pAd)
{
	USHORT e2p_val = 0;
	UINT32 WlanFunCtrl = 0, CmbCtrl = 0, CoexCfg0 = 0, CoexCfg3 = 0;

#ifdef RTMP_MAC_PCI
	RTMP_SEM_LOCK(&pAd->WlanEnLock);
#endif



	RTMP_IO_READ32(pAd, WLAN_FUN_CTRL, &WlanFunCtrl);
	RTMP_IO_READ32(pAd, CMB_CTRL, &CmbCtrl);
	RTMP_IO_READ32(pAd, 0x40, &CoexCfg0);
	RTMP_IO_READ32(pAd, COEXCFG3, &CoexCfg3);

	CoexCfg0 &= ~BIT2;
	CmbCtrl &= ~(BIT14 | BIT12);
	WlanFunCtrl &= ~(BIT6 | BIT5);

	CoexCfg3 &= ~(BIT5 | BIT4 | BIT3 | BIT2);
	
	/*
		0x23[7]
		0x1: Chip is in Dual antenna mode
		0x0: Chip is in single antenna mode
	*/
	RT28xx_EEPROM_READ16(pAd, 0x22, e2p_val);

	if (e2p_val & 0x8000)
	{
		if ((pAd->NicConfig2.field.AntOpt == 0) 
			&& (pAd->NicConfig2.field.AntDiversity == 1))
		{
			CmbCtrl |= BIT12; /* 0x20[12]=1 */
		}
		else
		{
			CoexCfg3 |= BIT4; /* 0x4C[4]=1 */
		}
		CoexCfg3 |= BIT3; /* 0x4C[3]=1 */
		
		if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
		{
			WlanFunCtrl |= BIT6; /* 0x80[6]=1 */
		}
		DBGPRINT(RT_DEBUG_TRACE, ("%s - Dual antenna mode\n", __FUNCTION__));
	}
	else
	{
		if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
		{
			CoexCfg3 |= (BIT3 | BIT4); /* 0x4C[3]=1, 0x4C[4]=1 */
		}
		else
		{
			WlanFunCtrl |= BIT6; /* 0x80[6]=1 */
			CoexCfg3 |= BIT1; /* 0x4C[1]=1 */
		}
		DBGPRINT(RT_DEBUG_TRACE, ("%s - Single antenna mode\n", __FUNCTION__));
	}

	RTMP_IO_WRITE32(pAd, WLAN_FUN_CTRL, WlanFunCtrl);
	RTMP_IO_WRITE32(pAd, CMB_CTRL, CmbCtrl);
	RTMP_IO_WRITE32(pAd, 0x40, CoexCfg0);
	RTMP_IO_WRITE32(pAd, COEXCFG3, CoexCfg3);


#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->WlanEnLock);
#endif /* RTMP_MAC_PCI */
}

VOID MT76x0_VCO_CalibrationMode3(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR 	Channel)
{
	/*Call in-band command to VCO calibration execution.*/
	RTMP_CHIP_CALIBRATION(pAd, VCO_CALIBRATION, Channel);

	return;
}


#ifdef RTMP_FLASH_SUPPORT
static VOID ReloadLowCalResult(
	IN RTMP_ADAPTER *pAd
)
{
	USHORT RFValue;
	UINT32 reg_val, mac_val, i=0;
	UINT16 tmp1,tmp2;
	
	DBGPRINT(RT_DEBUG_ERROR,("<==== %s() \n",__FUNCTION__));
	/* LC-A band */
	
	//RtmpFlashRead(&RFValue, RF_OFFSET + 0x2CC, sizeof(RFValue));
	rtmp_ee_flash_read(pAd, 0x2CC, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B0_R39 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK0, RF_R39, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x2D0, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R24 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R24, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x2D4, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R39 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R39, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x2D8, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R17 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R17, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x2DC, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R58 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R58, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x2E0, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R59 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R59, (UCHAR)RFValue);
	/* TX group delay + TX IQ */
	
	rtmp_ee_flash_read(pAd, 0x2E4, &tmp1);
	DBGPRINT(RT_DEBUG_TRACE,(" reg_val [%x] \n",tmp1));
	
	rtmp_ee_flash_read(pAd, 0x2E6, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C38 value [%x] \n",reg_val));
	RTMP_IO_WRITE32(pAd, 0x2C38, reg_val);
	/* RX group delay + RX IQ */
	
	rtmp_ee_flash_read(pAd, 0x2E8, &tmp1);
	rtmp_ee_flash_read(pAd, 0x2EA, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C60 value [%x] \n",reg_val));
	RTMP_IO_WRITE32(pAd, 0x2C60, reg_val);

	rtmp_ee_flash_read(pAd, 0x2EC, &tmp1);
	rtmp_ee_flash_read(pAd, 0x2EE, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C70 value [%x] \n",reg_val));	
	RTMP_IO_WRITE32(pAd, 0x2C70, reg_val);
	/* LOFT cal */
	for(i=0;i<=15;i++)
	{
		rtmp_ee_flash_read(pAd, (0x2F0 + i*4), &tmp1);
		rtmp_ee_flash_read(pAd, (0x2F0 + i*4 + 2), &tmp2);
		reg_val = (tmp2 << 16) | tmp1;
		DBGPRINT(RT_DEBUG_TRACE,(" read from 0x%x and 0x%x \n",(RF_OFFSET + 0x2F0 + i*4),(RF_OFFSET + 0x2F0 + i*4 + 2)));	
				
		RTMP_IO_WRITE32(pAd, 0x2CF0, i);
		RTMP_IO_READ32(pAd, 0x2CF0, &mac_val);
		DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2CF0 value [%x] \n",mac_val));			
		
		RTMP_IO_WRITE32(pAd, 0x2CF4, reg_val);
		RTMP_IO_READ32(pAd, 0x2CF4, &reg_val);
		DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2CF4 value [%x] \n",reg_val));	
	}
	DBGPRINT(RT_DEBUG_ERROR,("====> %s() \n",__FUNCTION__));
}

static VOID ReloadMidCalResult(
	IN RTMP_ADAPTER *pAd
)
{
	USHORT RFValue;
	UINT32 reg_val, mac_val, i=0;
	UINT16 tmp1,tmp2;
	DBGPRINT(RT_DEBUG_ERROR,("<==== %s() \n",__FUNCTION__));
	/* LC-A band */
		
	//RtmpFlashRead(&RFValue, RF_OFFSET + 0x268, sizeof(RFValue));
	rtmp_ee_flash_read(pAd, 0x268, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B0_R39 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK0, RF_R39, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x26C, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R24 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R24, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x270, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R39 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R39, (UCHAR)RFValue);

	rtmp_ee_flash_read(pAd, 0x274, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R17 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R17, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x278, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R58 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R58, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x27C, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R59 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R59, (UCHAR)RFValue);
	/* TX group delay + TX IQ */

	rtmp_ee_flash_read(pAd, 0x280, &tmp1);
	rtmp_ee_flash_read(pAd, 0x282, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C38 value [%x] \n",reg_val));
	RTMP_IO_WRITE32(pAd, 0x2C38, reg_val);
	/* RX group delay + RX IQ */
	
	rtmp_ee_flash_read(pAd, 0x284, &tmp1);
	rtmp_ee_flash_read(pAd, 0x286, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C60 value [%x] \n",reg_val));
	RTMP_IO_WRITE32(pAd, 0x2C60, reg_val);
	
	rtmp_ee_flash_read(pAd, 0x288, &tmp1);
	rtmp_ee_flash_read(pAd, 0x28A, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C70 value [%x] \n",reg_val));	
	RTMP_IO_WRITE32(pAd, 0x2C70, reg_val);
	/* LOFT cal */
	for(i=0;i<=15;i++)
	{
		rtmp_ee_flash_read(pAd, (0x28C + i*4), &tmp1);
		rtmp_ee_flash_read(pAd, (0x28C + i*4 + 2), &tmp2);
		reg_val = (tmp2 << 16) | tmp1;
		DBGPRINT(RT_DEBUG_TRACE,(" read from 0x%x and 0x%x \n",(RF_OFFSET + 0x28C + i*4),(RF_OFFSET + 0x28C + i*4 + 2)));	
				
		RTMP_IO_WRITE32(pAd, 0x2CF0, i);
		RTMP_IO_READ32(pAd, 0x2CF0, &mac_val);
		DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2CF0 value [%x] \n",mac_val));			
		
		RTMP_IO_WRITE32(pAd, 0x2CF4, reg_val);
		RTMP_IO_READ32(pAd, 0x2CF4, &reg_val);
		DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2CF4 value [%x] \n",reg_val));	
	}
	DBGPRINT(RT_DEBUG_ERROR,("====> %s() \n",__FUNCTION__));
}

static VOID ReloadHighCalResult(
	IN RTMP_ADAPTER *pAd
)
{
	USHORT RFValue;
	UINT32 reg_val, mac_val, i=0;
	UINT16 tmp1,tmp2;
	DBGPRINT(RT_DEBUG_ERROR,("<==== %s() \n",__FUNCTION__));
	/* LC-A band */
	//RtmpFlashRead(&RFValue, RF_OFFSET + 0x204, sizeof(RFValue));
	rtmp_ee_flash_read(pAd, 0x204, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B0_R39 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK0, RF_R39, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x208, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R24 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R24, (UCHAR)RFValue);	
	
	rtmp_ee_flash_read(pAd, 0x20C, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R39 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R39, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x210, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R17 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R17, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x214, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R58 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R58, (UCHAR)RFValue);
	
	rtmp_ee_flash_read(pAd, 0x218, &RFValue);
	DBGPRINT(RT_DEBUG_TRACE,(" B6_R59 value [%x] \n",RFValue));
	rlt_rf_write(pAd, RF_BANK6, RF_R59, (UCHAR)RFValue);
	/* TX group delay + TX IQ */
	
	rtmp_ee_flash_read(pAd, 0x21C, &tmp1);
	rtmp_ee_flash_read(pAd, 0x21E, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C38 value [%x] \n",reg_val));
	RTMP_IO_WRITE32(pAd, 0x2C38, reg_val);
	/* RX group delay + RX IQ */	
	rtmp_ee_flash_read(pAd, 0x220, &tmp1);	
	rtmp_ee_flash_read(pAd, 0x222, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C60 value [%x] \n",reg_val));
	RTMP_IO_WRITE32(pAd, 0x2C60, reg_val);
	
	rtmp_ee_flash_read(pAd, 0x224, &tmp1);
	rtmp_ee_flash_read(pAd, 0x226, &tmp2);
	reg_val = (tmp2 << 16) | tmp1;
	DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2C70 value [%x] \n",reg_val));	
	RTMP_IO_WRITE32(pAd, 0x2C70, reg_val);
	/* LOFT cal */
	for(i=0;i<=15;i++)
	{
		rtmp_ee_flash_read(pAd, (0x228 + i*4), &tmp1);
		rtmp_ee_flash_read(pAd, (0x228 + i*4 + 2), &tmp2);
		reg_val = (tmp2 << 16) | tmp1;
		DBGPRINT(RT_DEBUG_TRACE,(" read from 0x%x and 0x%x \n",(RF_OFFSET + 0x228 + i*4),(RF_OFFSET + 0x228 + i*4 + 2)));	
				
		RTMP_IO_WRITE32(pAd, 0x2CF0, i);
		RTMP_IO_READ32(pAd, 0x2CF0, &mac_val);
		DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2CF0 value [%x] \n",mac_val));			
		
		RTMP_IO_WRITE32(pAd, 0x2CF4, reg_val);
		RTMP_IO_READ32(pAd, 0x2CF4, &reg_val);
		DBGPRINT(RT_DEBUG_TRACE,(" mac 0x2CF4 value [%x] \n",reg_val));	
	}
	
	DBGPRINT(RT_DEBUG_ERROR,("====> %s() \n",__FUNCTION__));
}
#endif /* RTMP_FLASH_SUPPORT */


static VOID FullCalibration(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Channel,
	IN BOOLEAN bSave)
{
#ifdef RTMP_FLASH_SUPPORT
	USHORT doCal1=0,doCal2=0;
	//RtmpFlashRead((UCHAR *)&doCal1, RF_OFFSET + 0x43, sizeof(doCal1));
	rtmp_ee_flash_read(pAd, 0x43, &doCal1);
	doCal2 = doCal1 & 0x10;
	DBGPRINT(RT_DEBUG_OFF,("%s():  docal = [%04x] valid bit[%x]\n",__FUNCTION__,doCal1,doCal2));

	/* if 0x43 bit 4 is 1 , read result from flash,otherwise, do calibration */
	if(doCal2 == 0x10)
	{
		DBGPRINT(RT_DEBUG_ERROR,("[%s] Reload from flash! current channel : %d \n",__FUNCTION__,Channel));
		if((Channel >= 36) && (Channel <= 64))
		{
			ReloadLowCalResult(pAd);
		}
		else if((Channel >= 100) && (Channel <= 128))
		{
			ReloadMidCalResult(pAd);
		}
		else if((Channel >= 132) && (Channel <= 165))
		{			
			ReloadHighCalResult(pAd);			
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR,("This Channel %d Has NO calibration data in flash! \n",Channel));
		}
		
				
	
	}
	else 
#endif /* RTMP_FLASH_SUPPORT */
	{
	UINT32 param, _param1, _param2, paramTXBW, paramRXBW, paramBW;
	UINT32 CalibrationMode;
	/*
		Do calibration.
		The calibration sequence is very important, please do NOT change it.
		1  RX DCOC calibration
		2  LC tank calibration
		3  TX Filter BW --> add @20150105
		4  RX Filter BW --> add @20150105
		5  TX RF LOFT 
		6  TX I/Q
		7  TX Group Delay		
		8  RX I/Q
		9  RX Group Delay
		10 TX 2G DPD
		11 TX 2G IM3 --> not ready yet @20121016
		12 TSSI Zero Reference --> not ready yet @20121016
		13 RX DCOC calibration
	*/

	if (Channel > 14)
	{
		_param1 = (1/*External PA*/) ? 1 : 2;
		
		if (Channel < 100)
			_param2 = (bSave == TRUE) ? 0x3 : 0x7;
		else if (Channel < 140)
			_param2 = (bSave == TRUE) ? 0x4 : 0x8;
		else
			_param2 = (bSave == TRUE) ? 0x5 : 0x9;
	}
	else
	{
		// TODO: 2.4G Need to check parameter [31:16]
		_param1 = 0;
		_param2 = (bSave == TRUE) ? 0x2 : 0x6;
	}

	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
		paramBW = BW_80;
	else if (pAd->CommonCfg.BBPCurrentBW == BW_40)
		paramBW = BW_40;
	else 
		paramBW = BW_20;
	
	param = _param1 | (_param2 << 8);
	paramTXBW = 1 | (paramBW << 8) | (_param2 << 16);
	paramRXBW = 0 | (paramBW << 8) | (_param2 << 16);
	
	if ( IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE))
		CalibrationMode = NON_SIGNAL_CALIBRATION;
	else
		CalibrationMode = FULL_CALIBRATION;

	DBGPRINT(RT_DEBUG_OFF, 
			("%s - Channel = %u, param = 0x%x, bSave = %d, CalibrationMode = %u\n",
			__FUNCTION__, Channel, param, bSave, CalibrationMode));
	
	if (CalibrationMode == FULL_CALIBRATION)
	{
		RTMP_CHIP_CALIBRATION(pAd, CalibrationMode, param);
		if (bSave)
		{
			RtmpOsMsDelay(350);
		}

		if (! ((ScanRunning(pAd) == TRUE) && pAd->CommonCfg.bIEEE80211H == TRUE) )
		{
			RTMP_CHIP_CALIBRATION(pAd, LC_CALIBRATION, 1);
				RtmpOsMsDelay(15);
		}
	}
	else
	{		
		/*
			1. RXDC Calibration parameter
				0:Back Ground Disable
		*/
		RTMP_CHIP_CALIBRATION(pAd, RXDCOC_CALIBRATION, 0);
	
		/*
			2. LC-Calibration parameter
				Bit[0:7]
					0: 2G
					1: 5G + External PA
					2: 5G + Internal PA
				Bit[8:15]
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (CalibrationMode == FULL_CALIBRATION)
		{
			RTMP_CHIP_CALIBRATION(pAd, LC_CALIBRATION, param);
		}
		/*
			3,4. BW-Calibration
				Bit[0:7] (0:RX, 1:TX)
				Bit[8:15] (0:BW20, 1:BW40, 2:BW80)
				Bit[16:23]
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (CalibrationMode == FULL_CALIBRATION)
		{
			RTMP_CHIP_CALIBRATION(pAd, BW_CALIBRATION, paramTXBW);
			RTMP_CHIP_CALIBRATION(pAd, BW_CALIBRATION, paramRXBW);
		}	
		/*
			5. RF LOFT-Calibration parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
	
		*/
		if (CalibrationMode == FULL_CALIBRATION)
		{
			RTMP_CHIP_CALIBRATION(pAd, LOFT_CALIBRATION, param);
		}
		/*
			6. TXIQ-Calibration parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (CalibrationMode == FULL_CALIBRATION)
		{
			RTMP_CHIP_CALIBRATION(pAd, TXIQ_CALIBRATION, param);
		}
		/*			
			7. TX Group-Delay Calibation parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (CalibrationMode == FULL_CALIBRATION)
		{	
			RTMP_CHIP_CALIBRATION(pAd, TX_GROUP_DELAY_CALIBRATION, param);
		}
		/*
			8. RXIQ-Calibration parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
						
			9. RX Group-Delay Calibation parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (CalibrationMode == FULL_CALIBRATION)
		{
			RTMP_CHIP_CALIBRATION(pAd, RXIQ_CALIBRATION, param);
			RTMP_CHIP_CALIBRATION(pAd, RX_GROUP_DELAY_CALIBRATION, param);
		}
		/* 
			10. TX 2G DPD - Only 2.4G needs to do DPD Calibration. 
		*/
		if (Channel <= 14)
			RTMP_CHIP_CALIBRATION(pAd, DPD_CALIBRATION, 0x0);
	}
}
}



VOID MT76x0_Calibration(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Channel,
	IN BOOLEAN bPowerOn,
	IN BOOLEAN bSaveCal)
{
	UINT32 MacReg = 0, reg_val = 0, reg_tx_alc = 0;
	
	DBGPRINT(RT_DEBUG_OFF, 
				("%s - Channel = %d, bPowerOn = %d, bSaveCal = %d\n",
				__FUNCTION__, Channel, bPowerOn, bSaveCal));

#ifdef RTMP_MAC_PCI
	RTMP_SEM_LOCK(&pAd->CalLock);
#endif /* RTMP_MAC_PCI */


	if (bPowerOn)
	{
		/* update PMU_OCLEVEL<5:1> from default 10010 to 11011 */
		UINT32 Value=0;
		RTMP_IO_READ32(pAd, LDO_CTRL_0, &Value);
		Value = Value & (~0x3E);
		Value |= 0x36;
		RTMP_IO_WRITE32(pAd, LDO_CTRL_0, Value);
		RTMP_IO_READ32(pAd, LDO_CTRL_0, &Value);
		DBGPRINT(RT_DEBUG_TRACE, 
				("%s - LDO_CTRL_0 value before RX_IQ calibration: 0x%x\n",
				__FUNCTION__, Value));		

		/*
			Do Power on calibration.
			The calibration sequence is very important, please do NOT change it.
			1 XTAL Setup (already done in AsicRfInit)
			2 R-calibration
			3 VCO calibration
		*/

		/*
			2 R-calibration 
		*/
		RTMP_CHIP_CALIBRATION(pAd, R_CALIBRATION, 0x0);

		MT76x0_VCO_CalibrationMode3(pAd, Channel);
		RTMPusecDelay(1);

#ifdef MT76x0_TSSI_CAL_COMPENSATION
		/* TSSI Calibration */
		if (bPowerOn && pAd->chipCap.bInternalTxALC)
		{
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x8);
			MT76x0_TSSI_DC_Calibration(pAd);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
		}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
	}

	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &reg_tx_alc); /* We need to restore 0x13b0 after calibration. */
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, 0x0);
	RTMPusecDelay(500);
	
	RTMP_IO_READ32(pAd, 0x2124, &reg_val); /* We need to restore 0x2124 after calibration. */
	MacReg = 0xFFFFFF7E; /* Disable 0x2704, 0x2708 controlled by MAC. */
	RTMP_IO_WRITE32(pAd, 0x2124, MacReg);

	if (bSaveCal)
	{
		if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
		{	
			AsicSwitchChannel(pAd, 7, FALSE);
			FullCalibration(pAd, 7, TRUE);
		}
		else if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
		{
			UINT i;
			UCHAR CalChannel[] = {42, 136, 155, 0};
			for (i = 0; CalChannel[i] != 0; i++)
			{
				AsicSwitchChannel(pAd, CalChannel[i], FALSE);
				FullCalibration(pAd, CalChannel[i], TRUE);			
			}
		}

		/* Back to original channel */
		AsicSwitchChannel(pAd, Channel, FALSE);
	}

	/* Restore saved calibration status */
	FullCalibration(pAd, Channel, FALSE);

	/* Restore 0x2124 & TX_ALC_CFG_0 after calibration completed */
	RTMP_IO_WRITE32(pAd, 0x2124, reg_val);
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, reg_tx_alc);

	/*
		14. RXDC Calibration parameter
			1:Back Ground Enable
	*/
	RTMP_CHIP_CALIBRATION(pAd, RXDCOC_CALIBRATION, 1);

	//RTMPusecDelay(100000); // TODO: check response packet from FW

#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->CalLock);
#endif /* RTMP_MAC_PCI */

}


VOID MT76x0_TempSensor(
	IN RTMP_ADAPTER *pAd)
{
	SHORT temperature = 0;
	CHAR Dout = 0;

	if (MT76x0_AsicGetTssiReport( pAd, TRUE, &Dout) == FALSE)
		return;

	/*
		Calculate temperature:
			T=3.5(Dout-D25) + 25
	*/
	temperature = (35*(Dout-pAd->chipCap.TemperatureOffset))/10 + 25;
	DBGPRINT(RT_DEBUG_INFO, 
			("%s - Dout=%d (0x%x), TemperatureOffset = %d (0x%x), temperature = %d (0x%x)\n",
			__FUNCTION__, Dout, Dout, pAd->chipCap.TemperatureOffset, pAd->chipCap.TemperatureOffset, temperature, temperature));
	if (pAd->chipCap.LastTemperatureforVCO == 0x7FFF)
		pAd->chipCap.LastTemperatureforVCO = temperature;
	if (pAd->chipCap.LastTemperatureforCal == 0x7FFF)
		pAd->chipCap.LastTemperatureforCal = temperature;
	pAd->chipCap.NowTemperature = temperature;
	
}

#ifdef RTMP_FLASH_SUPPORT
VOID MT76x0_ReadFlashAndInitAsic(
	IN RTMP_ADAPTER *pAd)
{
	USHORT ee_val = 0;
	UINT32 reg_val = 0;


	pAd->chipCap.eebuf = MT76x0_EeBuffer;
	rtmp_nv_init(pAd);

	rtmp_ee_flash_read(pAd, 0x22, &ee_val);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x22 = 0x%x\n", __FUNCTION__, ee_val));
	RTMP_IO_READ32(pAd, CMB_CTRL, &reg_val);
	reg_val &= 0xFFFF0000;
	reg_val |= ee_val;
	RTMP_IO_WRITE32(pAd, CMB_CTRL, reg_val);

	rtmp_ee_flash_read(pAd, 0x24, &ee_val);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x24 = 0x%x\n", __FUNCTION__, ee_val));
	RTMP_IO_READ32(pAd, 0x104, &reg_val);
	reg_val &= 0xFFFF0000;
	reg_val |= ee_val;
	RTMP_IO_WRITE32(pAd, 0x104, reg_val);
	return;
}
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RTMP_MAC_PCI
VOID MT76x0_InitPCIeLinkCtrlValue(
	IN RTMP_ADAPTER *pAd)
{
	USHORT ee_val = 0;
	POS_COOKIE pObj;
	INT pos = 0;
	USHORT Configuration = 0;
	USHORT vendor_id = 0, device_id = 0;
	UINT32 reg_val = 0;

	if (IS_MT76x0(pAd) == FALSE)
		return;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	pci_read_config_word(pObj->pci_dev, RTMP_OS_PCI_VENDOR_ID, &vendor_id);
	pci_read_config_word(pObj->pci_dev, RTMP_OS_PCI_DEVICE_ID, &device_id);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: vendor_id = 0x%04x, device_id = 0x%04x\n", __FUNCTION__, vendor_id, device_id));
	
	RT28xx_EEPROM_READ16(pAd, 0x26, ee_val);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x26 = 0x%x\n", __FUNCTION__, ee_val));
	RT28xx_EEPROM_READ16(pAd, 0x28, ee_val);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: 0x28 = 0x%x\n", __FUNCTION__, ee_val));

	pos = pci_find_capability(pObj->pci_dev, PCI_CAP_ID_EXP);

	if (pos != 0)
	{
		/* Ralink PCIe Device's Link Control Register Offset */
		pAd->RLnkCtrlOffset = pos + PCI_EXP_LNKCTL;
		pci_read_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, &Configuration);
		pAd->RLnkCtrlConfiguration = Configuration & 0x3;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Read 1 (Ralink PCIe Link Control Register) offset 0x%x = 0x%x\n", 
				__FUNCTION__, pAd->RLnkCtrlOffset, Configuration));

		RTMP_IO_READ32(pAd, 0x64, &reg_val);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Read 0x64 = 0x%x\n", __FUNCTION__, reg_val));
		reg_val |= 0x00000020;
		RTMP_IO_WRITE32(pAd, 0x64, reg_val);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: write 0x64 = 0x%x\n", __FUNCTION__, reg_val));

		RTMP_IO_WRITE32(pAd, 0x1F080, pAd->RLnkCtrlConfiguration); /* Please DON'T read it. */
		DBGPRINT(RT_DEBUG_TRACE, ("%s: write 0x1F080 = 0x%x\n", __FUNCTION__, pAd->RLnkCtrlConfiguration));
		
		Configuration &= 0x103;  /* 0x1: L0s, 0x2: L1, 0x3: L0s+L1 */
		pci_write_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, Configuration);
		pci_read_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, &Configuration);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Read 2 (Ralink PCIe Link Control Register) offset 0x%x = 0x%x\n", 
				__FUNCTION__, pAd->RLnkCtrlOffset, Configuration));
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Cannot locate PCIe Device's Link Control Register Offset!\n", __FUNCTION__));
	return;
}

VOID MT76x0_PciMlmeRadioOn(
	IN  RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif /* CONFIG_AP_SUPPORT */



	/* Clear Radio off flag*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	AndesPwrSavingOP(pAd, RADIO_ON, 0, 0, 0, 0, 0);
	RTMPusecDelay(50);
	
	RTMPRingCleanUp(pAd, QID_AC_BK);
   	RTMPRingCleanUp(pAd, QID_AC_BE);
   	RTMPRingCleanUp(pAd, QID_AC_VI);
   	RTMPRingCleanUp(pAd, QID_AC_VO);
   	RTMPRingCleanUp(pAd, QID_HCCA);
   	RTMPRingCleanUp(pAd, QID_MGMT);
   	RTMPRingCleanUp(pAd, QID_RX);

#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.vht_cent_ch;
	else
#endif /* DOT11_VHT_AC */
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;

	AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, FALSE);
	AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);

	MT76x0_Calibration(pAd, pAd->hw_cfg.cent_ch, FALSE, FALSE);

	/* Enable Tx/Rx*/
	RTMPEnableRxTx(pAd);
	
	/* Clear Radio off flag*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

#ifdef LED_CONTROL_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	/* The LEN_RADIO_ON indicates "Radio on but link down", 
	   so AP shall set LED LINK_UP status */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
    	RTMPSetLED(pAd, LED_LINK_UP);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* LED_CONTROL_SUPPORT */

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);
		
		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].MSSIDDev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].MSSIDDev);
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
}

VOID MT76x0_PciMlmeRadioOFF(
	IN  RTMP_ADAPTER *pAd)
{
	
	UINT32 pwr_level = 5;
	POS_COOKIE 	pObj;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif /* CONFIG_AP_SUPPORT */

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);
		
		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].MSSIDDev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].MSSIDDev);
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	
	DBGPRINT(RT_DEBUG_TRACE, ("\npwr_level = %d\n", pwr_level));

#ifdef AP_SCAN_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BOOLEAN		Cancelled;

		RTMPCancelTimer(&pAd->MlmeAux.APScanTimer, &Cancelled);
	}
#endif /* AP_SCAN_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF);
#endif /* LED_CONTROL_SUPPORT */
	
	MT76x0_DisableTxRx(pAd, GUIRADIO_OFF);
	
	AndesPwrSavingOP(pAd, RADIO_OFF, pwr_level, 0, 0, 0, 0);	

	/*
		Wait for Andes firmware receiving this in-band command packet
	*/			
	RTMPusecDelay(50);
	
	/* Set Radio off flag*/
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}
}
#endif /* RTMP_MAC_PCI */

VOID MT76x0_MakeUpRatePwrTable(
	IN  RTMP_ADAPTER *pAd)
{
	UINT32 reg_val;

	// MCS POWER
	RTMP_IO_READ32(pAd, TX_PWR_CFG_0, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_0, reg_val));
	pAd->chipCap.rate_pwr_table.CCK[0].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.CCK[0].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.CCK[0].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.CCK[1].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.CCK[1].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.CCK[1].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.CCK[2].MCS_Power = (CHAR)((reg_val&0x3F00)>>8);
	if ( pAd->chipCap.rate_pwr_table.CCK[2].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.CCK[2].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.CCK[3].MCS_Power = (CHAR)((reg_val&0x3F00)>>8);
	if ( pAd->chipCap.rate_pwr_table.CCK[3].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.CCK[3].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.OFDM[0].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.OFDM[0].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.OFDM[0].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.OFDM[1].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.OFDM[1].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.OFDM[1].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.OFDM[2].MCS_Power = (CHAR)((reg_val&0x3F000000)>>24);
	if ( pAd->chipCap.rate_pwr_table.OFDM[2].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.OFDM[2].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.OFDM[3].MCS_Power = (CHAR)((reg_val&0x3F000000)>>24);
	if ( pAd->chipCap.rate_pwr_table.OFDM[3].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.OFDM[3].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_1, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_1, reg_val));
	pAd->chipCap.rate_pwr_table.OFDM[4].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.OFDM[4].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.OFDM[4].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.OFDM[5].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.OFDM[5].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.OFDM[5].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.OFDM[6].MCS_Power = (CHAR)((reg_val&0x3F00)>>8);
	if ( pAd->chipCap.rate_pwr_table.OFDM[6].MCS_Power & 0x20 ) // > 32
		pAd->chipCap.rate_pwr_table.OFDM[6].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.HT[0].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.HT[0].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[0].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[0].MCS_Power = pAd->chipCap.rate_pwr_table.HT[0].MCS_Power;
	
	pAd->chipCap.rate_pwr_table.MCS32.MCS_Power = pAd->chipCap.rate_pwr_table.HT[0].MCS_Power;
	if ( pAd->chipCap.rate_pwr_table.MCS32.MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.MCS32.MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.HT[1].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.HT[1].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[1].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[1].MCS_Power = pAd->chipCap.rate_pwr_table.HT[1].MCS_Power;
	
	pAd->chipCap.rate_pwr_table.HT[2].MCS_Power = (CHAR)((reg_val&0x3F000000)>>24);
	if ( pAd->chipCap.rate_pwr_table.HT[2].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[2].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[2].MCS_Power = pAd->chipCap.rate_pwr_table.HT[2].MCS_Power;
	
	pAd->chipCap.rate_pwr_table.HT[3].MCS_Power = (CHAR)((reg_val&0x3F000000)>>24);
	if ( pAd->chipCap.rate_pwr_table.HT[3].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[3].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[3].MCS_Power = pAd->chipCap.rate_pwr_table.HT[3].MCS_Power;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_2, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_2, reg_val));
	pAd->chipCap.rate_pwr_table.HT[4].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.HT[4].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[4].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[4].MCS_Power = pAd->chipCap.rate_pwr_table.HT[4].MCS_Power;
	
	pAd->chipCap.rate_pwr_table.HT[5].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.HT[5].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[5].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[5].MCS_Power = pAd->chipCap.rate_pwr_table.HT[5].MCS_Power;
	
	pAd->chipCap.rate_pwr_table.HT[6].MCS_Power = (CHAR)((reg_val&0x3F00)>>8);
	if ( pAd->chipCap.rate_pwr_table.HT[6].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[6].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[6].MCS_Power = pAd->chipCap.rate_pwr_table.HT[6].MCS_Power;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_3, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_3, reg_val));
	pAd->chipCap.rate_pwr_table.STBC[0].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.STBC[0].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[0].MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.STBC[1].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.STBC[1].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[1].MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.STBC[2].MCS_Power = (CHAR)((reg_val&0x3F000000)>>24);
	if ( pAd->chipCap.rate_pwr_table.STBC[2].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[2].MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.STBC[3].MCS_Power = (CHAR)((reg_val&0x3F000000)>>24);
	if ( pAd->chipCap.rate_pwr_table.STBC[3].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[3].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_4, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_4, reg_val));
	pAd->chipCap.rate_pwr_table.STBC[4].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.STBC[4].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[4].MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.STBC[5].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.STBC[5].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[5].MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.STBC[6].MCS_Power = (CHAR)((reg_val&0x3F00)>>8);
	if ( pAd->chipCap.rate_pwr_table.STBC[6].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[6].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_7, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_7, reg_val));
	pAd->chipCap.rate_pwr_table.OFDM[7].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.OFDM[7].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.OFDM[7].MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.HT[7].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.HT[7].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.HT[7].MCS_Power -= 64;
	pAd->chipCap.rate_pwr_table.VHT[7].MCS_Power = pAd->chipCap.rate_pwr_table.HT[7].MCS_Power;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_8, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_8, reg_val));
	pAd->chipCap.rate_pwr_table.VHT[8].MCS_Power = (CHAR)((reg_val&0x3F0000)>>16);
	if ( pAd->chipCap.rate_pwr_table.VHT[8].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.VHT[8].MCS_Power -= 64;
	
	pAd->chipCap.rate_pwr_table.VHT[9].MCS_Power = (CHAR)((reg_val&0x3F000000)>>24);
	if ( pAd->chipCap.rate_pwr_table.VHT[9].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.VHT[9].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_9, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", TX_PWR_CFG_9, reg_val));
	pAd->chipCap.rate_pwr_table.STBC[7].MCS_Power = (CHAR)(reg_val&0x3F);
	if ( pAd->chipCap.rate_pwr_table.STBC[7].MCS_Power & 0x20 )
		pAd->chipCap.rate_pwr_table.STBC[7].MCS_Power -= 64;

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[0].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.CCK[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[1].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.CCK[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[2].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.CCK[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[3].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.CCK[3].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[0].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[1].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[2].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[3].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[4].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[5].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[6].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[7].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.OFDM[7].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[0].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[1].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[2].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[3].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[4].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[5].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[6].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[7].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.STBC[7].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[0].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[1].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[2].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[3].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[4].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[5].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[6].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[7].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.HT[7].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[0].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[1].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[2].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[3].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[4].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[5].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[6].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[7].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[7].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[8].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[8].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[9].MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.VHT[9].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.MCS32.MCS_Power = %d\n", pAd->chipCap.rate_pwr_table.MCS32.MCS_Power));

	// PA MODE
	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG0, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", RF_PA_MODE_CFG0, reg_val));
	pAd->chipCap.rate_pwr_table.CCK[0].RF_PA_Mode = (UCHAR)(reg_val&0x00000003);
	pAd->chipCap.rate_pwr_table.CCK[1].RF_PA_Mode = (UCHAR)((reg_val&0x0000000C)>>2);
	pAd->chipCap.rate_pwr_table.CCK[2].RF_PA_Mode = (UCHAR)((reg_val&0x00000030)>>4);
	pAd->chipCap.rate_pwr_table.CCK[3].RF_PA_Mode = (UCHAR)((reg_val&0x000000C0)>>6);
	pAd->chipCap.rate_pwr_table.OFDM[0].RF_PA_Mode = (UCHAR)((reg_val&0x00000300)>>8);
	pAd->chipCap.rate_pwr_table.OFDM[1].RF_PA_Mode = (UCHAR)((reg_val&0x00000C00)>>10);
	pAd->chipCap.rate_pwr_table.OFDM[2].RF_PA_Mode = (UCHAR)((reg_val&0x00003000)>>12);
	pAd->chipCap.rate_pwr_table.OFDM[3].RF_PA_Mode = (UCHAR)((reg_val&0x0000C000)>>14);
	pAd->chipCap.rate_pwr_table.OFDM[4].RF_PA_Mode = (UCHAR)((reg_val&0x00030000)>>16);
	pAd->chipCap.rate_pwr_table.OFDM[5].RF_PA_Mode = (UCHAR)((reg_val&0x000C0000)>>18);
	pAd->chipCap.rate_pwr_table.OFDM[6].RF_PA_Mode = (UCHAR)((reg_val&0x00300000)>>20);
	pAd->chipCap.rate_pwr_table.OFDM[7].RF_PA_Mode = (UCHAR)((reg_val&0x00C00000)>>22);
	pAd->chipCap.rate_pwr_table.MCS32.RF_PA_Mode = (UCHAR)((reg_val&0x03000000)>>24);

	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG1, &reg_val);
	DBGPRINT(RT_DEBUG_TRACE, ("0x%x: 0x%x\n", RF_PA_MODE_CFG1, reg_val));
	pAd->chipCap.rate_pwr_table.HT[0].RF_PA_Mode = (UCHAR)(reg_val&0x00000003);
	pAd->chipCap.rate_pwr_table.VHT[0].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[0].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.HT[1].RF_PA_Mode = (UCHAR)((reg_val&0x0000000C)>>2);
	pAd->chipCap.rate_pwr_table.VHT[1].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[1].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.HT[2].RF_PA_Mode = (UCHAR)((reg_val&0x00000030)>>4);
	pAd->chipCap.rate_pwr_table.VHT[2].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[2].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.HT[3].RF_PA_Mode = (UCHAR)((reg_val&0x000000C0)>>6);
	pAd->chipCap.rate_pwr_table.VHT[3].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[3].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.HT[4].RF_PA_Mode = (UCHAR)((reg_val&0x00000300)>>8);
	pAd->chipCap.rate_pwr_table.VHT[4].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[4].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.HT[5].RF_PA_Mode = (UCHAR)((reg_val&0x00000C00)>>10);
	pAd->chipCap.rate_pwr_table.VHT[5].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[5].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.HT[6].RF_PA_Mode = (UCHAR)((reg_val&0x00003000)>>12);
	pAd->chipCap.rate_pwr_table.VHT[6].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[6].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.HT[7].RF_PA_Mode = (UCHAR)((reg_val&0x0000C000)>>14);
	pAd->chipCap.rate_pwr_table.VHT[7].RF_PA_Mode = pAd->chipCap.rate_pwr_table.HT[7].RF_PA_Mode;
	pAd->chipCap.rate_pwr_table.VHT[8].RF_PA_Mode = (UCHAR)((reg_val&0x00030000)>>16);
	pAd->chipCap.rate_pwr_table.VHT[9].RF_PA_Mode = (UCHAR)((reg_val&0x000C0000)>>18);

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[0].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.CCK[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[1].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.CCK[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[2].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.CCK[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.CCK[3].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.CCK[3].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[0].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[1].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[2].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[3].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[4].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[5].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[6].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.OFDM[7].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.OFDM[7].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[0].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[1].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[2].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[3].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[4].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[5].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[6].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.STBC[7].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.STBC[7].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[0].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[1].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[2].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[3].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[4].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[5].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[6].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.HT[7].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.HT[7].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[0].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[1].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[2].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[3].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[4].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[5].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[6].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[7].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[7].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[8].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[8].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.VHT[9].RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.VHT[9].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("rate_pwr_table.MCS32.RF_PA_Mode = %d\n", pAd->chipCap.rate_pwr_table.MCS32.RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
}


/******************************* TSSI *********************************/
#ifdef MT76x0_TSSI_CAL_COMPENSATION
#define DEFAULT_BO              4
#define LIN2DB_ERROR_CODE       (-10000)
INT16 lin2dBd(
	IN	unsigned short linearValue)
{
    short exp;
    unsigned int mantisa;
    int app,dBd;

	/* Default backoff ; to enhance leading bit searching time */
	mantisa = linearValue << DEFAULT_BO;
	exp = -(DEFAULT_BO);

	/* Leading bit searching */
	if (mantisa < (0x8000))
	{
		while (mantisa < (0x8000))
		{
			mantisa = mantisa << 1; /* no need saturation */
			exp--;
			if (exp < -20)
			{
				DBGPRINT_ERR(("input too small\n"));
				DBGPRINT_ERR(("exponent = %d\n",exp));

				return LIN2DB_ERROR_CODE;
			}
		}
	}
	else 
	{
		while (mantisa > (0xFFFF))
		{
			mantisa = mantisa >> 1; /* no need saturation */
			exp ++;
			if (exp > 20)
			{
				DBGPRINT_ERR(("input too large\n"));
				DBGPRINT_ERR(("exponent = %d\n",exp));

				return LIN2DB_ERROR_CODE;
			}
		}
	}
/*	printk("exp=0d%d,mantisa=0x%x\n",exp,mantisa); */

	if (mantisa <= 47104)
	{
		app=(mantisa+(mantisa>>3)+(mantisa>>4)-38400); /* S(15,0) */
		if (app<0)
		{
			app=0;
		}
	}
	else
	{
		app=(mantisa-(mantisa>>3)-(mantisa>>6)-23040); /* S(15,0) */
		if (app<0)
		{
			app=0;
		}
	}

	dBd=((15+exp)<<15)+app; /*since 2^15=1 here */
/*	printk("dBd1=%d\n",dBd); */
	dBd=(dBd<<2)+(dBd<<1)+(dBd>>6)+(dBd>>7);
	dBd=(dBd>>10); /* S10.5 */
/*	printk("app=%d,dBd=%d,dBdF=%f\n",app,dBd,(double)dBd/32); */

	return(dBd);
}

VOID MT76x0_TSSI_DC_Calibration(
	IN  RTMP_ADAPTER *pAd)
{
	UCHAR RF_Value;
	UINT32 MAC_Value, BBP_Value;
	USHORT i = 0;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		pAd->hw_cfg.cent_ch = pAd->ate.Channel;
	}
#endif /* RALINK_ATE */

	if( pAd->hw_cfg.cent_ch > 14 )
	{
		rlt_rf_read(pAd, RF_BANK0, RF_R67, &RF_Value);
		RF_Value &= 0xF0;
		rlt_rf_write(pAd, RF_BANK0, RF_R67, RF_Value);
	}

	// Enable 9bit I channel ADC and get TSSI DC point from BBP
	{
		// Bypass ADDA controls
		MAC_Value = 0x60002237;
		RTMP_IO_WRITE32(pAd, RF_SETTING_0, MAC_Value);
		MAC_Value = 0xFFFFFFFF;
		RTMP_IO_WRITE32(pAd, RF_BYPASS_0, MAC_Value);

		//********************************************************************//
		// BBP Soft Reset
		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value |= 0x00000001;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);

		RTMPusecDelay(1);

		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value &= 0xFFFFFFFE;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);
		//********************************************************************//

		if( pAd->hw_cfg.cent_ch > 14 )
		{
			// EXT TSSI
			// Set avg mode on Q channel
			BBP_Value = 0x00080055;
			RTMP_IO_WRITE32(pAd, CORE_R34, BBP_Value);
		}
		else
		{
			// Set avg mode on I channel
			BBP_Value = 0x00080050;
			RTMP_IO_WRITE32(pAd, CORE_R34, BBP_Value);
		}

        // Enable TX with 0 DAC inputs
        BBP_Value = 0x80000000;
		RTMP_IO_WRITE32(pAd, TXBE_R6, BBP_Value);

		// Wait until avg done
		do
		{
			RTMP_IO_READ32(pAd, CORE_R34, &BBP_Value);

			if ( (BBP_Value&0x10) == 0 )
				break;

			i++;
			if ( i >= 100 )
				break;

			RTMPusecDelay(10);

		} while (TRUE);

		// Read TSSI value
		RTMP_IO_READ32(pAd, CORE_R35, &BBP_Value);
		pAd->chipCap.tssi_current_DC = (CHAR)(BBP_Value&0xFF);

		// stop bypass ADDA
		//              MAC_Value = 0x0;
		//              rtmp.HwMemoryWriteDword(RA_RF_SETTING_0, MAC_Value, 4);
		MAC_Value = 0x0;
		RTMP_IO_WRITE32(pAd, RF_BYPASS_0, MAC_Value);

		// Stop TX
		BBP_Value = 0x0;
		RTMP_IO_WRITE32(pAd, TXBE_R6, BBP_Value);

		//********************************************************************//
		// BBP Soft Reset
		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value |= 0x00000001;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);

		RTMPusecDelay(1);

		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value &= 0xFFFFFFFE;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);
		//********************************************************************//
	}

	// Restore
	{
		if( pAd->hw_cfg.cent_ch > 14 )
		{
			// EXT TSSI
			// Reset tssi_cal

			rlt_rf_read(pAd, RF_BANK0, RF_R67, &RF_Value);
			RF_Value &= 0xF0;
			RF_Value |= 0x04;
			rlt_rf_write(pAd, RF_BANK0, RF_R67, RF_Value);

		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Current_TSSI_DC = %d\n", __FUNCTION__, pAd->chipCap.tssi_current_DC));
}

BOOLEAN MT76x0_Enable9BitIchannelADC(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR Channel,
	IN  SHORT *pTSSI_Linear)
{
	UINT32 bbp_val;
	UINT32 MTxCycle = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Channel = %d\n", __FUNCTION__, Channel));

	if(Channel > 14)
	{
		/*
			EXT TSSI
			Set avg mode on Q channel
		*/
		bbp_val = 0x00080055;
	}
	else
	{
		/*
			Set avg mode on I channel
		*/
		bbp_val = 0x00080050;
	}

	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, bbp_val);

	/*
		Wait until it's done
		wait until 0x2088[4] = 0
	*/
	for (MTxCycle = 0; MTxCycle < 200; MTxCycle++)
	{
		RTMP_BBP_IO_READ32(pAd, CORE_R34, &bbp_val);
		if ((bbp_val & 0x10) == 0)
			break;
		RTMPusecDelay(10);
	}

	if (MTxCycle >= 200)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: We cannot wait too long, give up!\n", __FUNCTION__));
		bbp_val &= ~(0x10);
		RTMP_BBP_IO_WRITE32(pAd, CORE_R34, bbp_val);
		return FALSE;
	}
	
	/* 
		Read TSSI value 
	*/
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &bbp_val);

	*pTSSI_Linear = (CHAR)(bbp_val&0xFF);
	DBGPRINT(RT_DEBUG_TRACE, ("%s: CORE_R35 = 0x%X, TSSI_Linear = (CHAR)(BBP_Value&0xFF) = 0x%X\n", __FUNCTION__, bbp_val, *pTSSI_Linear));

	if (Channel > 14)
	{
		*pTSSI_Linear = *pTSSI_Linear + 128;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: TSSI_Linear = TSSI_Linear + 128 = 0x%X\n", __FUNCTION__, *pTSSI_Linear));
	}

	/*
		Set Packet Info#1 mode
	*/
	bbp_val = 0x00080041;
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, bbp_val);

	/*
		Read Info #1
	*/
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &bbp_val);
	pAd->chipCap.tssi_info_1 = (UCHAR)(bbp_val&0xFF);

	/*
		Set Packet Info#2 mode
	*/
	bbp_val = 0x00080042;
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, bbp_val);

	/*
		Read Info #2
	*/
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &bbp_val);
	pAd->chipCap.tssi_info_2 = (UCHAR)(bbp_val&0xFF);

	/*
		Set Packet Info#3 mode
	*/
	bbp_val = 0x00080043;
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, bbp_val);

	/* 
		Read Info #3
	*/
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &bbp_val);
	pAd->chipCap.tssi_info_3 = (UCHAR)(bbp_val&0xFF);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: TSSI_Linear = 0x%X\n", __FUNCTION__, *pTSSI_Linear));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: INFO_1 = 0x%X\n", __FUNCTION__, pAd->chipCap.tssi_info_1));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: INFO_2 = 0x%X\n", __FUNCTION__, pAd->chipCap.tssi_info_2));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: INFO_3 = 0x%X\n", __FUNCTION__, pAd->chipCap.tssi_info_3));
	return TRUE;
}

BOOLEAN MT76x0_GetTargetPower(
	IN  RTMP_ADAPTER *pAd,
	IN  CHAR *pTSSI_Tx_Mode,
	IN  CHAR *pTargetPower,
	IN  CHAR *pTargetPA_mode)
{
	UCHAR Tx_Rate, CurrentPower0;
	USHORT index;
	UINT32 reg_val = 0;
	CHAR Eas_power_adj = 0;

	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &reg_val);
	CurrentPower0 = (UCHAR)(reg_val&0x3F);

	*pTSSI_Tx_Mode = (pAd->chipCap.tssi_info_1 & 0x7);
	Eas_power_adj = (pAd->chipCap.tssi_info_3 & 0xF);

	if (*pTSSI_Tx_Mode == 0)
	{
		/*
			0: 1 Mbps, 1: 2 Mbps, 2: 5.5 Mbps, 3: 11 Mbps
		*/
		Tx_Rate = ((pAd->chipCap.tssi_info_1 & 0x60) >> 5);

		if (Tx_Rate > 3)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s ==> CCK Mode :: Unknown Tx_Rate = %d, return here.\n", __FUNCTION__, Tx_Rate));
			return FALSE;
		}

		*pTargetPower = (CHAR)(CurrentPower0 + pAd->chipCap.rate_pwr_table.CCK[Tx_Rate].MCS_Power);
		*pTargetPA_mode = (CHAR) pAd->chipCap.rate_pwr_table.CCK[Tx_Rate].RF_PA_Mode;

		DBGPRINT(RT_DEBUG_TRACE, ("==> CCK Mode :: TargetPower = %d\n", *pTargetPower));
	}
	else if (*pTSSI_Tx_Mode == 1)
	{
		Tx_Rate = ((pAd->chipCap.tssi_info_1 & 0xF0) >> 4);
		if ( Tx_Rate == 0xB )
			index = 0;
		else if ( Tx_Rate == 0xF )
			index = 1;
		else if ( Tx_Rate == 0xA )
			index = 2;
		else if ( Tx_Rate == 0xE )
			index = 3;
		else if ( Tx_Rate == 0x9 )
			index = 4;
		else if ( Tx_Rate == 0xD )
			index = 5;
		else if ( Tx_Rate == 0x8 )
			index = 6;
		else if ( Tx_Rate == 0xC )
			index = 7;
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s ==> OFDM Mode :: Unknown Tx_Rate = 0x%x, return here.\n", __FUNCTION__, Tx_Rate));
			return FALSE;
		}

		*pTargetPower = (CHAR)(CurrentPower0 + pAd->chipCap.rate_pwr_table.OFDM[index].MCS_Power);
		*pTargetPA_mode = pAd->chipCap.rate_pwr_table.OFDM[index].RF_PA_Mode;

		DBGPRINT(RT_DEBUG_TRACE, ("==> OFDM Mode :: TargetPower0 = %d (MCS%d)\n", *pTargetPower, index));
	}
	else if (*pTSSI_Tx_Mode == 4)
	{
		Tx_Rate = (pAd->chipCap.tssi_info_2 & 0x0F);

		if (Tx_Rate > 9)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s ==> VHT Mode :: Unknown Tx_Rate = %d, return here.\n", __FUNCTION__, Tx_Rate));
			return FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("==> VHT Mode :: CurrentPower0 = %d, pAd->chipCap.tssi_table.VHT[%d].MCS_Power = %d\n", 
			CurrentPower0, Tx_Rate, pAd->chipCap.rate_pwr_table.VHT[Tx_Rate].MCS_Power));
		
		*pTargetPower = (CHAR)(CurrentPower0 + pAd->chipCap.rate_pwr_table.VHT[Tx_Rate].MCS_Power);
		*pTargetPA_mode = (CHAR) pAd->chipCap.rate_pwr_table.VHT[Tx_Rate].RF_PA_Mode;
		
		DBGPRINT(RT_DEBUG_TRACE, ("==> VHT Mode :: TargetPower0 = %d (MCS%d)\n", *pTargetPower, Tx_Rate));
	}
    else
    {
		Tx_Rate = (pAd->chipCap.tssi_info_2 & 0x7F);

		if ( Tx_Rate == 32 ) // MCS32
		{
			*pTargetPower = (CHAR)(CurrentPower0 + pAd->chipCap.rate_pwr_table.MCS32.MCS_Power);
			*pTargetPA_mode = pAd->chipCap.rate_pwr_table.MCS32.RF_PA_Mode;

		}
		else
		{
			if (Tx_Rate > 9)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s ==> HT Mode :: Unknown Tx_Rate = %d, return here.\n", __FUNCTION__, Tx_Rate));
				return FALSE;
			}
			
			*pTargetPower = (CHAR)(CurrentPower0 + pAd->chipCap.rate_pwr_table.HT[Tx_Rate].MCS_Power);
			*pTargetPA_mode = pAd->chipCap.rate_pwr_table.HT[Tx_Rate].RF_PA_Mode;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("==> HT Mode :: TargetPower0 = %d (MCS%d)\n", *pTargetPower, Tx_Rate));
		return TRUE;
	}
	return TRUE;
}

VOID MT76x0_EstimateDeltaPower(
	IN  RTMP_ADAPTER *pAd,
	IN  CHAR TSSI_Tx_Mode,
	IN  SHORT TSSI_Linear,
	IN  CHAR TargetPower,
	IN  CHAR TargetPA_mode,
	IN  INT *tssi_delta0)
{
	INT tssi_slope=0;
	INT tssi_offset=0;
	INT tssi_target=0, tssi_delta_tmp;
	INT tssi_meas=0;
	INT tssi_dc;
	INT pkt_type_delta=0, bbp_6db_power=0;
	UINT32 BBP_Value;
	CHAR idx = 0;

	// a.  tssi_dc gotten from Power on calibration

	// b.  Read Slope: u.2.6 (MT7601)
	// c.  Read offset: s.3.4 (MT7601)
	if (pAd->hw_cfg.cent_ch > 14)
	{
		for (idx = 0; idx < 7; idx++)
		{
			if ((pAd->hw_cfg.cent_ch <= pAd->chipCap.tssi_5G_channel_boundary[idx])
				|| (pAd->chipCap.tssi_5G_channel_boundary[idx] == 0))
			{
				tssi_slope = pAd->chipCap.tssi_slope_5G[idx];
				tssi_offset = pAd->chipCap.tssi_offset_5G[idx];
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_5G_channel_boundary[%d] = %d\n", idx, pAd->chipCap.tssi_5G_channel_boundary[idx]));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_slope_5G[%d] = %d\n", idx, pAd->chipCap.tssi_slope_5G[idx]));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_offset_5G[%d] = %d\n", idx, pAd->chipCap.tssi_offset_5G[idx]));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_slope = %d\n", tssi_slope));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_offset = %d\n", tssi_offset));
				break;
			}
		}
		if (idx == 7)
		{
			tssi_slope = pAd->chipCap.tssi_slope_5G[idx];
			tssi_offset = pAd->chipCap.tssi_offset_5G[idx];
			DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_slope_5G[%d] = %d\n", idx, pAd->chipCap.tssi_slope_5G[idx]));
			DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_offset_5G[%d] = %d\n", idx, pAd->chipCap.tssi_offset_5G[idx]));
		}
	}
	else
	{
		tssi_slope = pAd->chipCap.tssi_slope_2G;
		tssi_offset = pAd->chipCap.tssi_offset_2G;
	}

	if ( pAd->hw_cfg.cent_ch > 14 )
	{
		/*
			0x40 ~ 0x7F remapping to -192 ~ -129
		*/
		if ( (tssi_offset >= 0x40) && (tssi_offset <= 0x7F) )
			tssi_offset = tssi_offset - 0x100;
		else
			tssi_offset = (tssi_offset & 0x80) ?  tssi_offset - 0x100 : tssi_offset;
	}
	else
	{
		tssi_offset = (tssi_offset & 0x80) ?  tssi_offset - 0x100 : tssi_offset;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("==> 1) tssi_offset = %d (0x%x)\n", tssi_offset, tssi_offset));

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> EstimateDeltaPower\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_Tx_Mode = %d\n", TSSI_Tx_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TargetPower = %d\n", TargetPower));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_Linear = %d\n", TSSI_Linear));
	DBGPRINT(RT_DEBUG_TRACE, ("==> Current_TSSI_DC = %d\n", pAd->chipCap.tssi_current_DC));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_slope0 = %d\n", tssi_slope));
	DBGPRINT(RT_DEBUG_TRACE, ("==> 2) tssi_offset0 = %d\n", tssi_offset));

	// d.
	// Cal delta0
	tssi_target = (TargetPower << 12);
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = TargetPower0*4096) = %d\n", tssi_target));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TargetPA_mode = %d\n", TargetPA_mode));
	
	switch(TargetPA_mode)
	{
		case 0: 
			tssi_target = tssi_target;
			DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target) = %d\n", tssi_target));
			break;
		case 1: 
			if ( pAd->hw_cfg.cent_ch > 14 )
			{
				tssi_target = tssi_target + 0;
				DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target + 0) = %d\n", tssi_target));
			}
			else
			{
				tssi_target = tssi_target + 29491; // 3.6 * 8192
				DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target + 29491) = %d\n", tssi_target));
			}
			break;
		default: 
			tssi_target = tssi_target +  4424; // 0.54 * 8192
			DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target +  4424) = %d\n", tssi_target));
			break;
	}

	RTMP_BBP_IO_READ32(pAd, CORE_R1, &BBP_Value);
	DBGPRINT(RT_DEBUG_TRACE, ("==> (0x%x = 0x%x)\n", CORE_R1, BBP_Value));
	switch(TSSI_Tx_Mode)
	{
		case 0:
			/*
				CCK power offset	With Japan filter	Without Japan filter
				7630E				+2.3db (2.3 * 8192)	+1.5db (1.5 * 8192)
				Other project		+0.8db (0.8 * 8192)	+0db
			*/
#ifdef RTMP_MAC_PCI
			if (IS_MT7630E(pAd))
			{
				if (BBP_Value&0x20)
					pkt_type_delta = 18841;//2.3 * 8192;
				else
					pkt_type_delta = 12288;//1.5 * 8192;
			}
			else
#endif /* RTMP_MAC_PCI */
			{
				if (BBP_Value&0x20)
					pkt_type_delta = 6554;//0.8 * 8192;
				else
					pkt_type_delta = 0;//0 * 8192;
			}
			break;
		default:
			pkt_type_delta = 0;
			break;
	}

	tssi_target = tssi_target + pkt_type_delta;

	RTMP_BBP_IO_READ32(pAd, TXBE_R4, &BBP_Value);
	DBGPRINT(RT_DEBUG_TRACE, ("==> TXBE_OFFSET+R4 = 0x%X\n", BBP_Value));
	switch( (BBP_Value&0x3) )
	{
		case 0: 
			bbp_6db_power = 0; 
			break;
		case 1: 
			bbp_6db_power = -49152; 
			break; //-6 dB*8192;
		case 2: 
			bbp_6db_power = -98304; 
			break; //-12 dB*8192;
		case 3: 
			bbp_6db_power = 49152; 
			break;  //6 dB*8192;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("==> bbp_6db_power = %d\n", bbp_6db_power));
	
	tssi_target = tssi_target + bbp_6db_power;
	DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target + bbp_6db_power) = %d\n", tssi_target));

	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_target = %d\n", (tssi_target) >> 13));
	tssi_dc = pAd->chipCap.tssi_current_DC;
	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_Linear0 = %d\n", TSSI_Linear)); 
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_dc = %d\n", tssi_dc)); 
	
	tssi_meas = lin2dBd( (TSSI_Linear - tssi_dc));
	DBGPRINT(RT_DEBUG_TRACE, ("==> Linear to dB = %d\n", tssi_meas)); 

	tssi_meas = tssi_meas *tssi_slope;
	DBGPRINT(RT_DEBUG_TRACE, ("==> dB x slope = %d (0x%x), tssi_offset = %d(0x%x)\n", tssi_meas, tssi_meas, tssi_offset, tssi_offset));
	DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_offset-50) = (%d)(0x%x)\n", (tssi_offset-50), (tssi_offset-50)));
	DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_offset-50)<< 10 = (%d)(0x%x)\n", ((tssi_offset-50) << 10), ((tssi_offset-50) << 10)));
	if( pAd->hw_cfg.cent_ch > 14 )
		tssi_meas += ((tssi_offset-50) << 10); // 5G: offset s4.3
	else
		tssi_meas += (tssi_offset << 9); // 2G: offset s3.4
	DBGPRINT(RT_DEBUG_TRACE, ("==> measure db = %d (0x%x) %d\n", tssi_meas, tssi_meas, (tssi_meas) >> 13));
	
	tssi_delta_tmp = tssi_target - tssi_meas;
	DBGPRINT(RT_DEBUG_TRACE, ("==> delta db = %d\n", tssi_delta_tmp));

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_Linear0 = %d\n", TSSI_Linear));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_delta_tmp = %d\n", tssi_delta_tmp));

	if( pAd->hw_cfg.cent_ch > 14 )
	{
		if((TSSI_Linear > 254) && (tssi_delta_tmp > 0)) //upper saturate
			tssi_delta_tmp = 0;
	}
	else
	{
		if((TSSI_Linear > 126) && (tssi_delta_tmp > 0)) //upper saturate
			tssi_delta_tmp = 0;
		if(((TSSI_Linear - tssi_dc) < 1  ) && (tssi_delta_tmp < 0)) //lower saturate
			tssi_delta_tmp = 0;
	}

	// stablize the compensation value
	// if previous compensation result is better than current, skip the compensation
	if( ((pAd->chipCap.tssi_pre_delta_pwr^tssi_delta_tmp) < 0) 
		&& ((tssi_delta_tmp < 4096) && (tssi_delta_tmp > -4096))
		&& ((pAd->chipCap.tssi_pre_delta_pwr < 4096) && (pAd->chipCap.tssi_pre_delta_pwr > -4096)) )
	{
		if((tssi_delta_tmp>0)&&((tssi_delta_tmp +pAd->chipCap.tssi_pre_delta_pwr) <= 0))
		    tssi_delta_tmp = 0;
		else if((tssi_delta_tmp<0)&&((tssi_delta_tmp +pAd->chipCap.tssi_pre_delta_pwr) > 0))
		    tssi_delta_tmp = 0;
		else
		    pAd->chipCap.tssi_pre_delta_pwr = tssi_delta_tmp;
	}
	else
		pAd->chipCap.tssi_pre_delta_pwr = tssi_delta_tmp;

	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_DELTA_PRE = %d\n", pAd->chipCap.tssi_pre_delta_pwr));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_delta_tmp = %d\n", tssi_delta_tmp));

	// make the compensate value to the nearest compensate code
	tssi_delta_tmp = tssi_delta_tmp + ((tssi_delta_tmp > 0 ) ? 2048 : -2048);
	DBGPRINT(RT_DEBUG_TRACE, ("==> delta db = %d\n", tssi_delta_tmp));
	tssi_delta_tmp = tssi_delta_tmp >> 12;
	DBGPRINT(RT_DEBUG_TRACE, ("==> delta db = %d\n", tssi_delta_tmp));

	*tssi_delta0 = *tssi_delta0 + tssi_delta_tmp;
	DBGPRINT(RT_DEBUG_TRACE, ("==> *tssi_delta0 = %d\n", *tssi_delta0));
	if(*tssi_delta0 > 31)
		*tssi_delta0 = 31;
	else if(*tssi_delta0 < -32)
		*tssi_delta0 = -32;

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_delta0 = %d\n", *tssi_delta0));
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
}

VOID MT76x0_IntTxAlcProcess(
	IN  RTMP_ADAPTER *pAd)
{
	INT tssi_delta0 = 0;
	UINT32 reg_val = 0;
	CHAR tssi_write = 0;
	CHAR TargetPower = 0, TargetPA_mode = 0;
	SHORT TSSI_Linear = 0;
	CHAR TSSI_Tx_Mode = 0;

	if (MT76x0_Enable9BitIchannelADC(pAd, pAd->hw_cfg.cent_ch, &TSSI_Linear) == FALSE)
		return;

	if (MT76x0_GetTargetPower(pAd, &TSSI_Tx_Mode, &TargetPower, &TargetPA_mode) == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s ==> Get target power failed, return here.\n", __FUNCTION__));
		return;
	}

	RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &reg_val);
	DBGPRINT(RT_DEBUG_ERROR, ("(0x13B4) Before compensation 0x%08X\n", reg_val));
	tssi_delta0 = (CHAR)(reg_val&0x3F);
	if ( (tssi_delta0 &0x20) )
		tssi_delta0 -= 0x40;

	MT76x0_EstimateDeltaPower(pAd, TSSI_Tx_Mode, TSSI_Linear, TargetPower, TargetPA_mode, &tssi_delta0);

	tssi_write = tssi_delta0;

	reg_val &= 0xFFFFFFC0;
	reg_val |= (tssi_write&0x3F);
	DBGPRINT(RT_DEBUG_ERROR, ("%s ==> reg_val = 0x%08X\n", __FUNCTION__, reg_val));
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, reg_val);
	RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &reg_val);
	DBGPRINT(RT_DEBUG_ERROR, ("(0x13B4) After compensation 0x%08X\n", reg_val));
}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
/******************************* TSSI end **********************************/

#ifdef SINGLE_SKU_V2
VOID MT76x0_InitPAModeTable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	INT32 PAMode;
	UINT32 Value = 0;
	UINT16 index, offset;

	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG0, &Value);
	
	if (Channel <= 14)
	{
		for ( index = 0, offset = 0; index < 4 ; index++, offset+= 2 )
		{
			PAMode = (Value >> offset ) & 0x3;
			if ( PAMode == 3 )
				pAd->chipCap.PAModeCCK[index] = MT76x0_RF_2G_PA_MODE3_DECODE;
			else if ( PAMode == 1 )
				pAd->chipCap.PAModeCCK[index] = MT76x0_RF_2G_PA_MODE1_DECODE;
			else if ( PAMode == 0 )
				pAd->chipCap.PAModeCCK[index] = MT76x0_RF_2G_PA_MODE0_DECODE;
			else
				DBGPRINT(RT_DEBUG_TRACE, ("Un-expect PA mode(=%d)\n", PAMode));
			DBGPRINT(RT_DEBUG_TRACE, ("PAModeCCK[%d] = %d\n", index, pAd->chipCap.PAModeCCK[index]));
		}
	}

	for ( index = 0, offset = 8; index < 8 ; index++, offset+= 2 )
	{
		PAMode = (Value >> offset ) & 0x3;
		if ( PAMode == 3 )
		{
			if (Channel <= 14)
				pAd->chipCap.PAModeOFDM[index] = MT76x0_RF_2G_PA_MODE3_DECODE;
			else
				DBGPRINT(RT_DEBUG_TRACE, ("Ch%d: Un-expect PA mode(=%d)\n", Channel, PAMode));
		}
		else if ( PAMode == 1 )
		{
			if (Channel <= 14)
				pAd->chipCap.PAModeOFDM[index] = MT76x0_RF_2G_PA_MODE1_DECODE;
			else
				pAd->chipCap.PAModeOFDM[index] = MT76x0_RF_5G_PA_MODE1_DECODE;
		}
		else if ( PAMode == 0 )
		{
			if (Channel <= 14)
				pAd->chipCap.PAModeOFDM[index] = MT76x0_RF_2G_PA_MODE0_DECODE;
			else
				pAd->chipCap.PAModeOFDM[index] = MT76x0_RF_5G_PA_MODE0_DECODE;
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("Un-expect PA mode(=%d)\n", PAMode));
		DBGPRINT(RT_DEBUG_TRACE, ("PAModeOFDM[%d] = %d\n", index, pAd->chipCap.PAModeOFDM[index]));
	}

	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG1, &Value);  

	for ( index = 0, offset = 0; index < 16 ; index++, offset+= 2 )
	{
		PAMode = (Value >> offset ) & 0x3;
		if ( PAMode == 3 )
		{
			if (Channel <= 14)
				pAd->chipCap.PAModeHT[index] = MT76x0_RF_2G_PA_MODE3_DECODE;
			else
				DBGPRINT(RT_DEBUG_TRACE, ("Ch%d: Un-expect PA mode(=%d)\n", Channel, PAMode));
		}
		else if ( PAMode == 1 )
		{
			if (Channel <= 14)
				pAd->chipCap.PAModeHT[index] = MT76x0_RF_2G_PA_MODE1_DECODE;
			else
				pAd->chipCap.PAModeHT[index] = MT76x0_RF_5G_PA_MODE1_DECODE;
		}
		else if ( PAMode == 0 )
		{
			if (Channel <= 14)
				pAd->chipCap.PAModeHT[index] = MT76x0_RF_2G_PA_MODE0_DECODE;
			else
				pAd->chipCap.PAModeHT[index] = MT76x0_RF_5G_PA_MODE0_DECODE;
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("Un-expect PA mode(=%d)\n", PAMode));
		DBGPRINT(RT_DEBUG_TRACE, ("PAModeHT[%d] = %d\n", index, pAd->chipCap.PAModeHT[index]));
	}

#ifdef DOT11_VHT_AC
	if (Channel > 14)
	{
		for ( index = 0; index < 10; index++ )
		{
			pAd->chipCap.PAModeVHT[index] = pAd->chipCap.PAModeHT[index];
			DBGPRINT(RT_DEBUG_TRACE, ("PAModeVHT[%d] = %d\n", index, pAd->chipCap.PAModeVHT[index]));
		}
	}
#endif /* DOT11_VHT_AC */
}

UCHAR MT76x0_GetSkuChannelBasePwr(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			channel)
{
	CH_POWER *ch, *ch_temp;
	UCHAR base_pwr = pAd->DefaultTargetPwr;
	int i;
	
	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s ==> channel = %d, ch->channel = %d\n", __FUNCTION__, channel, ch->channel));
		if (channel == ch->channel)
		{
			if (channel <= 14)
			{
				for ( i= 0 ; i < SINGLE_SKU_TABLE_CCK_LENGTH ; i++ )
				{
					if ( base_pwr > ch->PwrCCK[i] )
						base_pwr = ch->PwrCCK[i];
				}
			}

			for ( i= 0 ; i < SINGLE_SKU_TABLE_OFDM_LENGTH ; i++ )
			{
				if ( base_pwr > ch->PwrOFDM[i] )
					base_pwr = ch->PwrOFDM[i];
			}

			for ( i= 0 ; i < SINGLE_SKU_TABLE_HT_LENGTH ; i++ )
			{
				if ( base_pwr > ch->PwrHT20[i] )
					base_pwr = ch->PwrHT20[i];
			}
				
			if (pAd->CommonCfg.BBPCurrentBW == BW_40)
			{
				for ( i= 0 ; i < SINGLE_SKU_TABLE_HT_LENGTH ; i++ )
				{
					if ( ch->PwrHT40[i] == 0 )
						break;

					if ( base_pwr > ch->PwrHT40[i] )
						base_pwr = ch->PwrHT40[i];
				}
			}
			if (pAd->CommonCfg.BBPCurrentBW == BW_80)
			{
				for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
				{
					if ( ch->PwrVHT80[i] == 0 )
						break;

					if ( base_pwr > ch->PwrVHT80[i] )
						base_pwr = ch->PwrVHT80[i];
				}
			}
			break;
		}
	}

	return base_pwr;

}

VOID MT76x0_WriteNewPerRatePwr(
	IN  RTMP_ADAPTER *pAd)
{
	UINT32 data;
	UCHAR t1, t2, t3, t4;

	/* 
		bit 29:24 -> OFDM 12M/18M
		bit 21:16 -> OFDM 6M/9M
		bit 13:8 -> CCK 5.5M/11M
		bit 5:0 -> CCK 1M/2M
	*/
	t1 = pAd->chipCap.rate_pwr_table.CCK[0].MCS_Power;
	t1 = (t1 & 0x80) ? ((t1 & 0x1f) | 0x20) : (t1 & 0x3f);
	
	t2 = pAd->chipCap.rate_pwr_table.CCK[2].MCS_Power;
	t2 = (t2 & 0x80) ? ((t2 & 0x1f) | 0x20) : (t2 & 0x3f);

	t3 = pAd->chipCap.rate_pwr_table.OFDM[0].MCS_Power;
	t3 = (t3 & 0x80) ? ((t3 & 0x1f) | 0x20) : (t3 & 0x3f);
	
	t4 = pAd->chipCap.rate_pwr_table.OFDM[2].MCS_Power;
	t4 = (t4 & 0x80) ? ((t4 & 0x1f) | 0x20) : (t4 & 0x3f);
	data = (t4 << 24)|(t3 << 16)|(t2 << 8)|t1; 

	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_0, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_0, data));

	/* 
		bit 29:24 -> HT MCS=2,3, VHT 1SS MCS=2,3
		bit 21:16 -> HT MCS=0,1, VHT 1SS MCS=0,1
		bit 13:8 -> OFDM 48M
		bit 5:0 -> OFDM 24M/36M
	*/	
	t1 = pAd->chipCap.rate_pwr_table.OFDM[4].MCS_Power;
	t1 = (t1 & 0x80) ? ((t1 & 0x1f) | 0x20) : (t1 & 0x3f);
	
	t2 = pAd->chipCap.rate_pwr_table.OFDM[6].MCS_Power;
	t2 = (t2 & 0x80) ? ((t2 & 0x1f) | 0x20) : (t2 & 0x3f);

	t3 = pAd->chipCap.rate_pwr_table.HT[0].MCS_Power;
	t3 = (t3 & 0x80) ? ((t3 & 0x1f) | 0x20) : (t3 & 0x3f);
	
	t4 = pAd->chipCap.rate_pwr_table.HT[2].MCS_Power;
	t4 = (t4 & 0x80) ? ((t4 & 0x1f) | 0x20) : (t4 & 0x3f);
	data = (t4 << 24)|(t3 << 16)|(t2 << 8)|t1; 

	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_1, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_1, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_1, data));

	/*
		bit 13:8 -> HT MCS=6, VHT 1SS MCS=6
		bit 5:0 -> MCS=4,5, VHT 1SS MCS=4,5
	*/
	t1 = pAd->chipCap.rate_pwr_table.HT[4].MCS_Power;
	t1 = (t1 & 0x80) ? ((t1 & 0x1f) | 0x20) : (t1 & 0x3f);
	
	t2 = pAd->chipCap.rate_pwr_table.HT[6].MCS_Power;
	t2 = (t2 & 0x80) ? ((t2 & 0x1f) | 0x20) : (t2 & 0x3f);
	data = (t2 << 8)|t1; 
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_2, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_2, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_2, data));

	/* 
		bit 29:24 -> HT/VHT STBC MCS=2, 3
		bit 21:16 -> HT/VHT STBC MCS=0, 1
	*/
	t3 = pAd->chipCap.rate_pwr_table.STBC[0].MCS_Power;
	t3 = (t3 & 0x80) ? ((t3 & 0x1f) | 0x20) : (t3 & 0x3f);
	
	t4 = pAd->chipCap.rate_pwr_table.STBC[2].MCS_Power;
	t4 = (t4 & 0x80) ? ((t4 & 0x1f) | 0x20) : (t4 & 0x3f);
	data = (t4 << 24)|(t3 << 16);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_3, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_3, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_3, data));

	/* 
		bit 13:8 -> HT/VHT STBC MCS=6
		bit 5:0 -> HT/VHT STBC MCS=4,5
	*/
	t1 = pAd->chipCap.rate_pwr_table.STBC[4].MCS_Power;
	t1 = (t1 & 0x80) ? ((t1 & 0x1f) | 0x20) : (t1 & 0x3f);
	
	t2 = pAd->chipCap.rate_pwr_table.STBC[6].MCS_Power;
	t2 = (t2 & 0x80) ? ((t2 & 0x1f) | 0x20) : (t2 & 0x3f);	
	data = (t2 << 8)|t1; 
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_4, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_4, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_4, data));

	/* 
		For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 
		bit 21:16 -> HT MCS=7, VHT 2SS MCS=7
		bit 5:0 -> OFDM 54
	*/
	t1 = pAd->chipCap.rate_pwr_table.OFDM[7].MCS_Power;
	t1 = (t1 & 0x80) ? ((t1 & 0x1f) | 0x20) : (t1 & 0x3f);
	
	t3 = pAd->chipCap.rate_pwr_table.HT[7].MCS_Power;
	t3 = (t3 & 0x80) ? ((t3 & 0x1f) | 0x20) : (t3 & 0x3f);
	data = (t3 << 16)|t1; 
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_7, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_7, data));

	/* 
		bit 29:24 -> VHT 1SS MCS=9
		bit 21:16 -> VHT 1SS MCS=8
	*/
	t3 = pAd->chipCap.rate_pwr_table.VHT[8].MCS_Power;
	t3 = (t3 & 0x80) ? ((t3 & 0x1f) | 0x20) : (t3 & 0x3f);
	
	t4 = pAd->chipCap.rate_pwr_table.VHT[9].MCS_Power;
	t4 = (t4 & 0x80) ? ((t4 & 0x1f) | 0x20) : (t4 & 0x3f);
	data = (t4 << 24)|(t3 << 16);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_8, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_8, data));

	/* 
		For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC 
		bit 5:0 -> HT/VHT STBC MCS=7
	*/
	data = pAd->chipCap.rate_pwr_table.STBC[7].MCS_Power;
	data = (data & 0x80) ? ((data & 0x1f) | 0x20) : (data & 0x3f);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, data);
	RTMP_IO_READ32(pAd, TX_PWR_CFG_9, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("%s - 0x%x: 0x%x\n", __FUNCTION__, TX_PWR_CFG_9, data));
}

UCHAR MT76x0_UpdateSkuPwr(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			channel)
{
	CH_POWER *ch, *ch_temp;
	INT32 i, pwr_delta = 0;
	UINT32 reg_val; 
	UCHAR ch_init_pwr = 0;
	CHAR ch_delta_pwr = 0;
	INT32 rate_pwr = 0, rate_pwr_before_adjust = 0, sku_pwr = 0;
	BOOLEAN bFound = FALSE;
	CHAR SkuBasePwr;
	const CHAR DefaultTargetPwr = pAd->DefaultTargetPwr;

	/*
		Get channel initial transmission gain.
	*/
	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &reg_val);
	ch_init_pwr = (UCHAR)(reg_val & 0x3F);
	DBGPRINT(RT_DEBUG_TRACE, ("%s ==> 0x%08X = 0x%08X, ch_init_pwr = %d\n", __FUNCTION__, TX_ALC_CFG_0, reg_val, ch_init_pwr));

	RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &reg_val);
	ch_delta_pwr = (UCHAR)(reg_val & 0x3F);
	if ( ch_delta_pwr & 0x20 )
		ch_delta_pwr -= 64;	
	DBGPRINT(RT_DEBUG_TRACE, ("%s ==> 0x%08X = 0x%08X, ch_delta_pwr = %d\n", __FUNCTION__, TX_ALC_CFG_1, reg_val, ch_delta_pwr));

	SkuBasePwr = MT76x0_GetSkuChannelBasePwr(pAd, channel);
	
	/*
		Read per rate power from EEPROM.
	*/
	MT76x0ReadTxPwrPerRate(pAd);

	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s ==> channel = %d, ch->channel = %d\n", __FUNCTION__, channel, ch->channel));
		if (channel == ch->channel)
		{

			for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++)
			{
				pwr_delta = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> ch->PwrOFDM[%d] = %d\n", 
					__FUNCTION__, i, ch->PwrOFDM[i]));
				
				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> OFDM[%d].MCS_Power = %d, ch_init_pwr = %d\n", 
					__FUNCTION__, i, pAd->chipCap.rate_pwr_table.OFDM[i].MCS_Power, ch_init_pwr));
				rate_pwr_before_adjust = pAd->chipCap.rate_pwr_table.OFDM[i].MCS_Power + DefaultTargetPwr;
				rate_pwr = rate_pwr_before_adjust + ch_delta_pwr;
				sku_pwr = (ch->PwrOFDM[i] > rate_pwr_before_adjust) ? \
							rate_pwr_before_adjust : ch->PwrOFDM[i];
				pwr_delta = sku_pwr - rate_pwr;
				pAd->chipCap.rate_pwr_table.OFDM[i].MCS_Power += pwr_delta;

				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> rate_delta = %d, rate_pwr_before_adjust = %d\n\n",
							__FUNCTION__, pAd->chipCap.rate_pwr_table.OFDM[i].MCS_Power, rate_pwr_before_adjust));
			}

			for (i = 0; i < (SINGLE_SKU_TABLE_HT_LENGTH >> 1); i++)
			{
				pwr_delta = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> ch->PwrHT20[%d] = %d\n", 
					__FUNCTION__, i, ch->PwrHT20[i]));
								
				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> HT[%d].MCS_Power = %d, ch_init_pwr = %d\n", 
					__FUNCTION__, i, pAd->chipCap.rate_pwr_table.HT[i].MCS_Power, ch_init_pwr));

				rate_pwr_before_adjust = pAd->chipCap.rate_pwr_table.HT[i].MCS_Power + DefaultTargetPwr;
				rate_pwr = rate_pwr_before_adjust + ch_delta_pwr;
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
				{
					sku_pwr = (ch->PwrHT20[i] > rate_pwr_before_adjust) ? \
								rate_pwr_before_adjust : ch->PwrHT20[i];
				}
				else if (pAd->CommonCfg.BBPCurrentBW == BW_40)
				{
					sku_pwr = (ch->PwrHT40[i] > rate_pwr_before_adjust) ? \
								rate_pwr_before_adjust : ch->PwrHT40[i];
				}
				else if (pAd->CommonCfg.BBPCurrentBW == BW_80)
				{
					sku_pwr = (ch->PwrVHT80[i] > rate_pwr_before_adjust) ? \
								rate_pwr_before_adjust : ch->PwrVHT80[i];
				}
				pwr_delta = sku_pwr - rate_pwr;
				pAd->chipCap.rate_pwr_table.HT[i].MCS_Power += pwr_delta;

				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> rate_delta = %d, rate_pwr_before_adjust = %d\n\n",
							__FUNCTION__, pAd->chipCap.rate_pwr_table.HT[i].MCS_Power, rate_pwr_before_adjust));
			}

			for (i = 0; i < (SINGLE_SKU_TABLE_VHT_LENGTH >> 1); i++)
			{
				pwr_delta = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> ch->PwrVHT80[%d] = %d\n", 
					__FUNCTION__, i, ch->PwrVHT80[i]));
								
				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> VHT[%d].MCS_Power = %d, ch_init_pwr = %d\n", 
					__FUNCTION__, i, pAd->chipCap.rate_pwr_table.VHT[i].MCS_Power, ch_init_pwr));

				rate_pwr_before_adjust = pAd->chipCap.rate_pwr_table.VHT[i].MCS_Power + DefaultTargetPwr;
				rate_pwr =  rate_pwr_before_adjust + ch_delta_pwr;
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
				{
					sku_pwr = (ch->PwrHT20[i] > rate_pwr_before_adjust) ? \
								rate_pwr_before_adjust : ch->PwrHT20[i];
				}
				else if (pAd->CommonCfg.BBPCurrentBW == BW_40)
				{
					sku_pwr = (ch->PwrHT40[i] > rate_pwr_before_adjust) ? \
								rate_pwr_before_adjust : ch->PwrHT40[i];
				}
				else if (pAd->CommonCfg.BBPCurrentBW == BW_80)
				{
					sku_pwr = (ch->PwrVHT80[i] > rate_pwr_before_adjust) ? \
								rate_pwr_before_adjust : ch->PwrVHT80[i];
				}
				pwr_delta = sku_pwr - rate_pwr;
				pAd->chipCap.rate_pwr_table.VHT[i].MCS_Power += pwr_delta;

				DBGPRINT(RT_DEBUG_TRACE, ("%s ==> rate_delta = %d, rate_pwr_before_adjust = %d\n\n",
							__FUNCTION__, pAd->chipCap.rate_pwr_table.VHT[i].MCS_Power, rate_pwr_before_adjust));
			}

			bFound = TRUE;
			break;
		}
	}


	if (bFound)
	{
		MT76x0_WriteNewPerRatePwr(pAd);
		return TRUE;
	}
	else
		return 0;
}

#endif /* SINGLE_SKU_V2 */


BOOLEAN MT76x0_AsicGetTssiReport(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bResetTssiInfo,
	OUT PCHAR pTssiReport)
{
	INT		wait;
	UINT32 	reg_val = 0;
	UCHAR rf_b7_73 = 0, rf_b7_28 = 0, rf_b0_66 = 0, rf_b0_67 = 0;
	BOOLEAN Status;

#ifdef RTMP_MAC_PCI
	RTMP_SEM_LOCK(&pAd->CalLock);
#endif /* RTMP_MAC_PCI */
	
	rlt_rf_read(pAd, RF_BANK7, RF_R73, &rf_b7_73);
	rlt_rf_read(pAd, RF_BANK7, RF_R28, &rf_b7_28);
	rlt_rf_read(pAd, RF_BANK0, RF_R66, &rf_b0_66);
	rlt_rf_read(pAd, RF_BANK0, RF_R67, &rf_b0_67);
	
	/*
		1. Set 0dB Gain:
			WIFI_RF_CR_WRITE(7,73,0x42) 
	*/
	rlt_rf_write(pAd, RF_BANK7, RF_R73, 0x02);

	/*
		3. Calibration Switches:
			WIFI_RF_CR_WRITE(0,66,0x23)
	*/
	rlt_rf_write(pAd, RF_BANK0, RF_R66, 0x23);

	/*
		4. Offset-measurement configuration:
			WIFI_RF_CR_WRITE(0,67,0x01) 
	*/
	rlt_rf_write(pAd, RF_BANK0, RF_R67, 0x01);


	// TODO: Check bResetTssiInfo ?
	/*
		Select Level meter from ADC.q:
			WIFI_BBP_CR_WRITE(0x2088,0x00080055)
	*/
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, 0x00080055);

	/*
		Wait until it's done
		wait until 0x2088[4] = 0
	*/
	for (wait = 0; wait < 2000; wait++)
	{
		RTMP_BBP_IO_READ32(pAd, CORE_R34, &reg_val);
		if ((reg_val & 0x10) == 0)
			break;
		RTMPusecDelay(3);
	}

	if (wait >= 2000)
	{
		reg_val &= ~(0x10);
		RTMP_BBP_IO_WRITE32(pAd, CORE_R34, reg_val);
		Status = FALSE;
		goto done;
	}

	/*
		Read TssiReport
		(0x0041208c<7:0>=adc_out<8:1>)
	*/
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &reg_val);
	*pTssiReport = (reg_val & 0xFF);
	Status =  TRUE;

done:	
	/*
		Restore RF CR
			B7.R73, B7.R28, B0.R66, B0.R67
	*/
	rlt_rf_write(pAd, RF_BANK7, RF_R73, rf_b7_73);
	rlt_rf_write(pAd, RF_BANK7, RF_R28, rf_b7_28);
	rlt_rf_write(pAd, RF_BANK0, RF_R66, rf_b0_66);
	rlt_rf_write(pAd, RF_BANK0, RF_R67, rf_b0_67);

#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->CalLock);
#endif /* RTMP_MAC_PCI */
	return Status;
}


#ifdef RTMP_TEMPERATURE_COMPENSATION
/* MaxBoundaryLevel MUST not be greater than the array size of TssiBoundarys */
static BOOLEAN MT76x0_GetTemperatureCompensationLevel(
	IN		PRTMP_ADAPTER	pAd,
	IN		BOOLEAN			bAutoTxAgc,
	IN		CHAR			TssiRef, /* e2p[75h]: the zero reference */
	IN		PCHAR			pTssiMinusBoundary,
	IN		PCHAR			pTssiPlusBoundary,
	IN		UINT8			MaxBoundaryLevel,
	IN		UINT8			TxAgcStep,
	IN		CHAR			CurrTemperature,
	OUT		PCHAR			pCompensationLevel)
{
	INT			idx;

	/* Sanity Check */
	if (pTssiMinusBoundary == NULL ||
		pTssiPlusBoundary == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
					("%s(): pTssiBoundary is NULL!\n",
					__FUNCTION__)); 
		return FALSE;
	}
	if (bAutoTxAgc)
	{
		if (CurrTemperature < pTssiMinusBoundary[1])
		{
			/* 	Reading is larger than the reference value check
				for how large we need to decrease the Tx power		*/
			for (idx = 1; idx < MaxBoundaryLevel; idx++)
			{
				if (CurrTemperature >= pTssiMinusBoundary[idx]) 
					break;	/* level range found */
			}

			/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */
			*pCompensationLevel = -(TxAgcStep * (idx-1));
			DBGPRINT(RT_DEBUG_TRACE, 
						("-- Tx Power, CurrTemperature=%d, TssiRef=%d, TxAgcStep=%d, step = -%d, CompensationLevel = %d\n",
			    			CurrTemperature, TssiRef, TxAgcStep, idx-1, *pCompensationLevel));                    
		}
		else if (CurrTemperature > pTssiPlusBoundary[1])
		{
			/*	Reading is smaller than the reference value check
				for how large we need to increase the Tx power		*/
			for (idx = 1; idx < MaxBoundaryLevel; idx++)
			{
				if (CurrTemperature <= pTssiPlusBoundary[idx])
					break; /* level range found */
			}

			/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
			*pCompensationLevel = TxAgcStep * (idx-1);
			DBGPRINT(RT_DEBUG_TRACE,
						("++ Tx Power, CurrTemperature=%d, TssiRef=%d, TxAgcStep=%d, step = +%d, , CompensationLevel = %d\n",
				    		CurrTemperature, TssiRef, TxAgcStep, idx-1, *pCompensationLevel));
		}
		else
		{
			*pCompensationLevel = 0;
			DBGPRINT(RT_DEBUG_TRACE,
						("  Tx Power, CurrTemperature=%d, TssiRef=%d, TxAgcStep=%d, step = +%d\n",
						CurrTemperature, TssiRef, TxAgcStep, 0));
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, 
					("%s(): bAutoTxAgc = %s\n",
					__FUNCTION__,
					(bAutoTxAgc) == TRUE ? "TRUE" : "FALSE")); 
		return FALSE;
	}

	return TRUE;
}


VOID MT76x0_TemperatureCompensation(
		IN PRTMP_ADAPTER pAd)
{
	BOOLEAN bResetTssiInfo = TRUE;
	PUCHAR pTssiMinusBoundary, pTssiPlusBoundary;
	
#ifdef MT76x0_TSSI_CAL_COMPENSATION
	if (pAd->chipCap.bInternalTxALC == TRUE ||
		pAd->TxPowerCtrl.bInternalTxALC == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
				("%s(): Errro! InternalTxALC is ON\n", __FUNCTION__));
		return;
	}
	else
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
	if ((WMODE_CAP_5G(pAd->CommonCfg.PhyMode)) ? 
		(pAd->bAutoTxAgcA == FALSE) : (pAd->bAutoTxAgcG == FALSE))
		return;

	if (pAd->CommonCfg.Channel <= pAd->ChBndryIdx)
	{
		/* Use table of 5G group 1 */
		pTssiMinusBoundary = pAd->TssiMinusBoundaryA[0];
		pTssiPlusBoundary = pAd->TssiPlusBoundaryA[0];
	}
	else
	{
		/* Use table of 5G group 2 */
		pTssiMinusBoundary = pAd->TssiMinusBoundaryA[1];
		pTssiPlusBoundary = pAd->TssiPlusBoundaryA[1];
	}

	if (MT76x0_AsicGetTssiReport(pAd,
								bResetTssiInfo,
								&pAd->CurrTemperature) == TRUE)
	{
		if (MT76x0_GetTemperatureCompensationLevel(
							pAd,
							pAd->bAutoTxAgcA,
							pAd->TssiRefA,
							pTssiMinusBoundary,
							pTssiPlusBoundary,
							8, /* to do: make a definition */
							2, /*pAd->TxAgcStepA,*/
							pAd->CurrTemperature,
							&pAd->TxAgcCompensateA) == TRUE)
		{
			UINT32 MacValue;
			CHAR LastTempCompDeltaPwr;
			CHAR delta_pwr = 0;
			
			/* adjust compensation value by MP temperature readings(i.e., e2p[77h]) */
			delta_pwr = pAd->TxAgcCompensateA - pAd->mp_delta_pwr;
			RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &MacValue);
			/* 6-bit representation ==> 8-bit representation (2's complement) */
			pAd->DeltaPwrBeforeTempComp = (MacValue & 0x20) ? \
											((MacValue & 0x3F) | 0xC0): (MacValue & 0x3f);		

			/* get last delta pwr */
			LastTempCompDeltaPwr = pAd->LastTempCompDeltaPwr;
			/* save the current delta pwr */
			pAd->LastTempCompDeltaPwr = delta_pwr;
			pAd->DeltaPwrBeforeTempComp -= LastTempCompDeltaPwr;
			delta_pwr += pAd->DeltaPwrBeforeTempComp;
			/* 8-bit representation ==> 6-bit representation (2's complement) */
			delta_pwr = (delta_pwr & 0x80) ? \
							((delta_pwr & 0x1f) | 0x20) : (delta_pwr & 0x3f);						
			/*	write compensation value into TX_ALC_CFG_1, 
				delta_pwr (unit: 0.5dB) will be compensated by TX_ALC_CFG_1 */     
			RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &MacValue);
			MacValue = (MacValue & (~0x3f)) | delta_pwr;
			RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, MacValue);

			DBGPRINT(RT_DEBUG_TRACE, 
						("%s - delta_pwr = %d, TssiCalibratedOffset = %d, TssiMpOffset = %d, Mac 0x13B4 = 0x%08x, TxAgcCompensateA = %d, DeltaPwrBeforeTempComp = %d, LastTempCompDeltaPwr =%d\n",
						__FUNCTION__,
						pAd->LastTempCompDeltaPwr,
						pAd->TssiCalibratedOffset,
						pAd->mp_delta_pwr,
						MacValue	, pAd->TxAgcCompensateA, pAd->DeltaPwrBeforeTempComp, LastTempCompDeltaPwr));	
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, 
						("%s(): Failed to get a compensation level!\n",
						__FUNCTION__)); 
		}
	}
}

/* Note that TssiMinusBoundary is a 2-dimension matrix */
static VOID MT76x0_TssiTableAdjust(
			IN	RTMP_ADAPTER *pAd,
			INOUT	PCHAR pTssiMinusBoundary,
			INOUT	PCHAR pTssiPlusBoundary,
			IN		CHAR  TssiCalibratedOffset)
{
	INT			idx;
	CHAR 		upper_bound = 127, lower_bound = -128;

	DBGPRINT(RT_DEBUG_OFF,("%s: upper_bound = 0x%02X decimal: %d\n",
		__FUNCTION__, upper_bound, upper_bound));
	DBGPRINT(RT_DEBUG_OFF,("%s: lower_bound = 0x%02X decimal: %d\n",
		__FUNCTION__, lower_bound, lower_bound));

	DBGPRINT(RT_DEBUG_OFF,("*** %s: A Tssi[-7 .. +7] = %d %d %d %d %d %d %d * %d * %d %d %d %d %d %d %d, offset=%d, tuning=%d\n",
		__FUNCTION__,
		pTssiMinusBoundary[7], pTssiMinusBoundary[6], pTssiMinusBoundary[5],
		pTssiMinusBoundary[4], pTssiMinusBoundary[3], pTssiMinusBoundary[2], pTssiMinusBoundary[1],
		(CHAR) pAd->TssiRefA,
		pTssiPlusBoundary[1], pTssiPlusBoundary[2], pTssiPlusBoundary[3], pTssiPlusBoundary[4],
		pTssiPlusBoundary[5], pTssiPlusBoundary[6], pTssiPlusBoundary[7],
		TssiCalibratedOffset, pAd->bAutoTxAgcA));

	for (idx = 0; idx < 8; idx++ )
	{
		if ((lower_bound - pTssiMinusBoundary[idx]) <= TssiCalibratedOffset)
		{
			pTssiMinusBoundary[idx] += TssiCalibratedOffset;
		}
		else
		{
			pTssiMinusBoundary[idx] = lower_bound;
		}

		if ((upper_bound - pTssiPlusBoundary[idx]) >= TssiCalibratedOffset)
		{
			pTssiPlusBoundary[idx] += TssiCalibratedOffset;
		}
		else
		{
			pTssiPlusBoundary[idx] = upper_bound;			
		}

		ASSERT(pTssiMinusBoundary[idx] >= lower_bound);
		ASSERT(pTssiPlusBoundary[idx] <= upper_bound);
	}

	pAd->TssiRefA = pTssiMinusBoundary[0];

	DBGPRINT(RT_DEBUG_OFF,("%s: A Tssi[-7 .. +7] = %d %d %d %d %d %d %d * %d * %d %d %d %d %d %d %d, offset=%d, tuning=%d\n",
		__FUNCTION__,
		pTssiMinusBoundary[7], pTssiMinusBoundary[6], pTssiMinusBoundary[5],
		pTssiMinusBoundary[4], pTssiMinusBoundary[3], pTssiMinusBoundary[2], pTssiMinusBoundary[1],
		(CHAR) pAd->TssiRefA,
		pTssiPlusBoundary[1], pTssiPlusBoundary[2], pTssiPlusBoundary[3], pTssiPlusBoundary[4],
		pTssiPlusBoundary[5], pTssiPlusBoundary[6], pTssiPlusBoundary[7],
		TssiCalibratedOffset, pAd->bAutoTxAgcA));
}

static VOID MT76x0_TssiMpAdjust(
			IN	RTMP_ADAPTER *pAd,
			IN	PCHAR pTssiMinusBoundary,
			IN	PCHAR pTssiPlusBoundary)
{
	EEPROM_TX_PWR_STRUC e2p_value;
	CHAR mp_temperature, idx, TxAgcMpOffset = 0;
	RT28xx_EEPROM_READ16(pAd, 0x10D, e2p_value);

	mp_temperature = e2p_value.field.Byte0;

	DBGPRINT(RT_DEBUG_TRACE, ("0x10D e2p_value.field.Byte0=0x%02x\n",
	    mp_temperature)); 

	if (mp_temperature < pTssiMinusBoundary[1])
	{
		/* mp_temperature is larger than the reference value */
		/* check for how large we need to adjust the Tx power */
		for (idx = 1; idx < 8; idx++)
		{
			if (mp_temperature >= pTssiMinusBoundary[idx]) /* the range has been found */
				break;
		}

		/* The index is the step we should decrease, idx = 0 means there is nothing to adjust */
		TxAgcMpOffset = -(2 * (idx-1));
		pAd->mp_delta_pwr = (TxAgcMpOffset);
		DBGPRINT(RT_DEBUG_OFF, ("mp_temperature=0x%02x, step = -%d\n",
		    mp_temperature, idx-1));                    
	}
	else if (mp_temperature > pTssiPlusBoundary[1])
	{
		/* mp_temperature is smaller than the reference value */
		/* check for how large we need to adjust the Tx power */
		for (idx = 1; idx < 8; idx++)
		{
		    if (mp_temperature <= pTssiPlusBoundary[idx]) /* the range has been found */
	            break;
		}

		/* The index is the step we should increase, idx = 0 means there is nothing to adjust */
		TxAgcMpOffset = 2 * (idx-1);
		pAd->mp_delta_pwr = (TxAgcMpOffset);
		DBGPRINT(RT_DEBUG_OFF, ("mp_temperature=0x%02x, step = +%d\n",
			    mp_temperature, idx-1));
	}
	else
	{
		pAd->mp_delta_pwr = 0;
		DBGPRINT(RT_DEBUG_OFF, ("mp_temperature=0x%02x, step = +%d\n",
				mp_temperature, 0));
	}
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */

VOID MT76x0_Read_TSSI_From_EEPROM( 
		IN PRTMP_ADAPTER pAd)
{
	EEPROM_TX_PWR_STRUC Power;
	USHORT e2p_value = 0;
#ifdef RTMP_TEMPERATURE_COMPENSATION
	BOOLEAN Status = TRUE;
#endif

	Power.word = 0;

	if (IS_MT76x0(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, 0xd0 /*EEPROM_TEMPERATURE_OFFSET*/, e2p_value);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: EEPROM_MT76x0_TEMPERATURE_OFFSET = 0x%x\n", __FUNCTION__, e2p_value));
		//pAd->chipCap.TemperatureOffset = (CHAR)( (e2p_value >> 8) & 0x00FF); // TODO: --(CHAR)

		if (((e2p_value >> 8) & 0x00FF) == 0xFF)
			pAd->chipCap.TemperatureOffset = -10;
		else
		{
			pAd->chipCap.TemperatureOffset = ((e2p_value >> 8) & 0x007F);
			if ((e2p_value & 0x8000))
				pAd->chipCap.TemperatureOffset |= 0xFF80;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("%s: TemperatureOffset = 0x%x\n", __FUNCTION__, pAd->chipCap.TemperatureOffset));
	}

#ifdef MT76x0_TSSI_CAL_COMPENSATION
	if (pAd->chipCap.bInternalTxALC)
	{
		
		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_2G_TARGET_POWER, e2p_value);
		pAd->chipCap.tssi_2G_target_power = e2p_value & 0x00ff;
		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_TARGET_POWER, e2p_value);
		pAd->chipCap.tssi_5G_target_power = e2p_value & 0x00ff;
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_2G_target_power = %d, tssi_5G_target_power = %d\n", 
			__FUNCTION__, pAd->chipCap.tssi_2G_target_power, pAd->chipCap.tssi_5G_target_power));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_2G_SLOPE_OFFSET, e2p_value);
		pAd->chipCap.tssi_slope_2G = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_2G = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_2G = 0x%x, tssi_offset_2G = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_2G, pAd->chipCap.tssi_offset_2G));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET, e2p_value);
		pAd->chipCap.tssi_slope_5G[0] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[0] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_1 = 0x%x, tssi_offset_5G_Group_1 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[0], pAd->chipCap.tssi_offset_5G[0]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET+2, e2p_value);
		pAd->chipCap.tssi_slope_5G[1] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[1] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_2 = 0x%x, tssi_offset_5G_Group_2 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[1], pAd->chipCap.tssi_offset_5G[1]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET+4, e2p_value);
		pAd->chipCap.tssi_slope_5G[2] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[2] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_3 = 0x%x, tssi_offset_5G_Group_3 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[2], pAd->chipCap.tssi_offset_5G[2]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET+6, e2p_value);
		pAd->chipCap.tssi_slope_5G[3] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[3] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_4 = 0x%x, tssi_offset_5G_Group_4 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[3], pAd->chipCap.tssi_offset_5G[3]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET+8, e2p_value);
		pAd->chipCap.tssi_slope_5G[4] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[4] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_5 = 0x%x, tssi_offset_5G_Group_5 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[4], pAd->chipCap.tssi_offset_5G[4]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET+10, e2p_value);
		pAd->chipCap.tssi_slope_5G[5] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[5] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_6 = 0x%x, tssi_offset_5G_Group_6 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[5], pAd->chipCap.tssi_offset_5G[5]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET+12, e2p_value);
		pAd->chipCap.tssi_slope_5G[6] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[6] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_7 = 0x%x, tssi_offset_5G_Group_7 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[6], pAd->chipCap.tssi_offset_5G[6]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_SLOPE_OFFSET+14, e2p_value);
		pAd->chipCap.tssi_slope_5G[7] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_offset_5G[7] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_slope_5G_Group_8 = 0x%x, tssi_offset_5G_Group_8 = 0x%x\n", 
			__FUNCTION__, pAd->chipCap.tssi_slope_5G[7], pAd->chipCap.tssi_offset_5G[7]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_CHANNEL_BOUNDARY, e2p_value);
		pAd->chipCap.tssi_5G_channel_boundary[0] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_5G_channel_boundary[1] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_5G_channel_boundary_1 = %d, tssi_5G_channel_boundary_2 = %d\n", 
			__FUNCTION__, pAd->chipCap.tssi_5G_channel_boundary[0], pAd->chipCap.tssi_5G_channel_boundary[1]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_CHANNEL_BOUNDARY+2, e2p_value);
		pAd->chipCap.tssi_5G_channel_boundary[2] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_5G_channel_boundary[3] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_5G_channel_boundary_3 = %d, tssi_5G_channel_boundary_4 = %d\n", 
			__FUNCTION__, pAd->chipCap.tssi_5G_channel_boundary[2], pAd->chipCap.tssi_5G_channel_boundary[3]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_CHANNEL_BOUNDARY+4, e2p_value);
		pAd->chipCap.tssi_5G_channel_boundary[4] = e2p_value & 0x00ff;
		pAd->chipCap.tssi_5G_channel_boundary[5] = (e2p_value >> 8);
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_5G_channel_boundary_5 = %d, tssi_5G_channel_boundary_6 = %d\n", 
			__FUNCTION__, pAd->chipCap.tssi_5G_channel_boundary[4], pAd->chipCap.tssi_5G_channel_boundary[5]));

		RT28xx_EEPROM_READ16(pAd, EEPROM_MT76x0_5G_CHANNEL_BOUNDARY+6, e2p_value);
		pAd->chipCap.tssi_5G_channel_boundary[6] = e2p_value & 0x00ff;
		DBGPRINT(RT_DEBUG_OFF, ("%s: tssi_5G_channel_boundary_7 = %d\n", 
			__FUNCTION__, pAd->chipCap.tssi_5G_channel_boundary[6]));
	}
	else
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
#ifdef RTMP_TEMPERATURE_COMPENSATION
	if (pAd->bAutoTxAgcG | pAd->bAutoTxAgcA)
	{	
		if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
		{
			RT28xx_EEPROM_READ16(pAd, 0xD0, e2p_value);
			pAd->TssiCalibratedOffset = (e2p_value >> 8);
		
			/* TX 5G power compensation channel boundary index */
			RT28xx_EEPROM_READ16(pAd, 0x10C, e2p_value);
			pAd->ChBndryIdx = (e2p_value & 0xFF);
			DBGPRINT(RT_DEBUG_TRACE, 
					("%s(): channel boundary index = %u, TssiCalibratedOffset = %d\n",
					__FUNCTION__, pAd->ChBndryIdx, pAd->TssiCalibratedOffset));

			/* Load Group1 settings */
			Status = LoadTempCompTableFromEEPROM(pAd,
													0xF6,
													0xF0,
													pAd->TssiMinusBoundaryA[0],
													1,
													sizeof(pAd->TssiMinusBoundaryA[0]));

			Status = LoadTempCompTableFromEEPROM(pAd,
													0xF7,
													0xFD,
													pAd->TssiPlusBoundaryA[0],
													1,
													sizeof(pAd->TssiPlusBoundaryA[0]));

			/* Load Group2 settings */
			Status = LoadTempCompTableFromEEPROM(pAd,
													0x104,
													0xFE,
													pAd->TssiMinusBoundaryA[1],
													1,
													sizeof(pAd->TssiMinusBoundaryA[1]));

			Status = LoadTempCompTableFromEEPROM(pAd,
													0x105,
													0x10B,
													pAd->TssiPlusBoundaryA[1],
													1,
													sizeof(pAd->TssiPlusBoundaryA[1]));	

			RT28xx_EEPROM_READ16(pAd, 0x10E, e2p_value);
			pAd->TxAgcStepA = (UCHAR) (e2p_value & 0xFF);

			pAd->TxAgcCompensateA = 0;
			// TODO: Do these smarter
			pAd->TssiRefA = 0;
			pAd->TssiMinusBoundaryA[0][0] = 0;
			pAd->TssiPlusBoundaryA[0][0] = 0;
			pAd->TssiMinusBoundaryA[1][0] = 0;
			pAd->TssiPlusBoundaryA[1][0] = 0;
			pAd->DeltaPwrBeforeTempComp = 0;
			pAd->LastTempCompDeltaPwr = 0;
		}
		else
		{
			/* No 2.4G Settings */
		}

		/*
			reference temperature(e2p[D1h])
		*/
		/* adjust the boundary table by pAd->chipCap.TemperatureOffset */
		MT76x0_TssiTableAdjust(pAd, pAd->TssiMinusBoundaryA[0], pAd->TssiPlusBoundaryA[0], pAd->TssiCalibratedOffset);
		MT76x0_TssiMpAdjust(pAd, pAd->TssiMinusBoundaryA[0], pAd->TssiPlusBoundaryA[0]);

		MT76x0_TssiTableAdjust(pAd, pAd->TssiMinusBoundaryA[1], pAd->TssiPlusBoundaryA[1], pAd->TssiCalibratedOffset);
		MT76x0_TssiMpAdjust(pAd, pAd->TssiMinusBoundaryA[1], pAd->TssiPlusBoundaryA[1]);

	}
	else
#endif /* RTMP_TEMPERATURE_COMPENSATION */
	{
		DBGPRINT(RT_DEBUG_WARN, ("No TSSI or Temperature Compensation used.\n"));
	}
}


/******************************* Command API *******************************/
INT Set_AntennaSelect_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN PSTRING			arg)
{
	UINT8 val = (UINT8)simple_strtol(arg, 0, 10);
	UINT32 reg_val = 0;

	/*
		0x2300[5] Default Antenna:
		0 for WIFI main antenna
		1 for WIFI aux  antenna

	*/
	RTMP_IO_READ32(pAd, AGC1_R0, &reg_val);
	reg_val &= ~(0x00000020);
	if (val != 0)
		reg_val |= 0x20;
	RTMP_IO_WRITE32(pAd, AGC1_R0, reg_val);

	RTMP_IO_READ32(pAd, AGC1_R0, &reg_val);

	RTMP_CHIP_CALIBRATION(pAd, RXDCOC_CALIBRATION, 1);
	DBGPRINT(RT_DEBUG_TRACE, ("Set_AntennaSelect_Proc:: AntennaSelect = %d (0x%08x=0x%08x)\n", val, AGC1_R0, reg_val));

	return TRUE;
}
/******************************* Command API end ***************************/


