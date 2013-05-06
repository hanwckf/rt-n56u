/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt3593.h
 
    Abstract:
	3*3 Wireless Chip PCIe

    Revision History:
    Who			When			What
    ---------	----------		----------------------------------------------
	SampleLin	20091105		Initial version
 */

#ifndef __RT3593_H__
#define __RT3593_H__

#ifdef RT3593

struct _RTMP_ADAPTER;

/*
	MCS 16 ~ 23 Test Note:
	Use fix rate mode, HT_MCS = 23, and set bit 30 of MAC Reg 134C to 0
	(disable auto-fallback mode).
*/
#ifndef RTMP_RF_RW_SUPPORT
#error "For RT3593/RT3573, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#ifndef RT30xx
#error "For RT3593/RT3573, you should define the compile flag -DRT30xx"
#endif

#ifndef RT35xx
#error "For RT3593/RT3573, you should define the compile flag -DRT35xx"
#endif

/* General definition */

/* if you want to support PCIe power save function */
/* 2009/11/06, if open the function, the signal will be bad and sometimes crush */
/*#define PCIE_PS_SUPPORT */

/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */
#define RALINK_3593_VERSION				((UINT32)0x35930400)

#define RT3593_MAC_VERSION_CHECK(__Version)							\
	((__Version & 0xFFFF0000) == 0x35930000)


/* External */
extern REG_PAIR RF3053RegTable[];
extern UCHAR NUM_RF_3053_REG_PARMS;
extern UCHAR RT3593_NUM_BBP_REG_PARMS;


/* MACRO definition */


/* for 3*3 related */
#define RTMP_RF33_SHARED_MEM_SELECT(__pAd)

#define RTMP_RF33_SHARED_MEM_SELECT_RESERVED(__pAd)					\
{																	\
	if (IS_RT3593(__pAd))											\
	{																\
		PBF_SYS_CTRL_STRUC __PbfSysCtrl = {{0}};					\
		RTMP_IO_READ32(__pAd, PBF_SYS_CTRL, &__PbfSysCtrl.word);	\
		__PbfSysCtrl.field.SHR_MSEL = 1;							\
		RTMP_IO_WRITE32(__pAd, PBF_SYS_CTRL, __PbfSysCtrl.word);	\
	}																\
}

#define RTMP_RF33_SHARED_MEM_DESELECT(__pAd)

#define RTMP_RF33_SHARED_MEM_DESELECT_RESERVED(__pAd)				\
{																	\
	if (IS_RT3593(__pAd))											\
	{																\
		PBF_SYS_CTRL_STRUC __PbfSysCtrl = {{0}};					\
		RTMP_IO_WRITE32(pAd, BCN_OFFSET0, 0x18100800); /* 0x0000(00), 0x0200(08), 0x0400(10), 0x0600(18), 512B for each beacon */ \
		RTMP_IO_WRITE32(pAd, BCN_OFFSET1, 0x38302820); /* 0x0800(20), 0x0A00(28), 0x0C00(30), 0x0E00(38), 512B for each beacon */ \
		RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &__PbfSysCtrl.word);		\
		__PbfSysCtrl.field.SHR_MSEL = 0;							\
		RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, __PbfSysCtrl.word);		\
	}																\
}

#define BBP_REG_BF			BBP_R163 /* TxBf control */

/* */
/* Extended EEPROM format (EEPROM_EXT_XXX) */
/* */

/* */
/* NIC configuration #2 */
/* */
#define EEPROM_EXT_NIC_CONFIGURATION_2									0x38

/* */
/* Country region code for 5G band */
/* */
#define EEPROM_EXT_COUNTRY_REGION_CODE_FOR_5G_BAND						0x3F

/* */
/* Maximum Tx power for 2.4 GHz and 5 GHz band */
/* */
#define EEPROM_EXT_MAX_TX_POWER_OVER_2DOT4G_AND_5G						0x40

/* */
/* Frequency offset */
/* */
#define EEPROM_EXT_FREQUENCY_OFFSET										0x44

/* */
/* LED mode setting */
/* */
#define EEPROM_EXT_LED_MODE_SETTING										0x45

/* */
/* LED A/G configuration */
/* */
#define EEPROM_EXT_LED_AG_CONFIGURATION									0x46

/* */
/* LED ACT configuration */
/* */
#define EEPROM_EXT_LED_ACT_CONFIGURATION								0x48

/* */
/* LED A/G/ACT polarity */
/* */
#define EEPROM_EXT_LED_AG_ACT_POLARITY									0x4A

/* */
/* External LNA gain for 2.4 GHz band */
/* */
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_2DOT4G							0x4C

/* */
/* External LNA gain for 5 GHz band (channel #36~#64) */
/* */
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH36_TO_CH64			0x4D

/* */
/* External LNA gain for 5 GHz band (channel #100~#128) */
/* */
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH100_TO_CH128			0x4E

/* */
/* External LNA gain for 5 GHz band (channel #132~#165) */
/* */
#define EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH132_TO_CH165			0x4F

/* */
/* RSSI0 offset for 2.4 GHz band */
/* */
#define EEPROM_EXT_RSSI0_OVER_2DOT4G									0x50

/* */
/* RSSI1 offset for 2.4 GHz band */
/* */
#define EEPROM_EXT_RSSI1_OVER_2DOT4G									0x51

/* */
/* RSSI2 offset for 2.4 GHz band */
/* */
#define EEPROM_EXT_RSSI2_OVER_2DOT4G									0x52

/* */
/* RSSI0 offset for 5 GHz band */
/* */
#define EEPROM_EXT_RSSI0_OVER_5G										0x54

/* */
/* RSSI1 offset for 5 GHz band */
/* */
#define EEPROM_EXT_RSSI1_OVER_5G										0x55

/* */
/* RSSI2 offset for 5 GHz band */
/* */
#define EEPROM_EXT_RSSI2_OVER_5G										0x56

/* */
/* Tx0 power over 2.4 GHz */
/* */
#define EEPROM_EXT_TX0_OVER_2DOT4G										0x60

/* */
/* Tx1 power over 2.4 GHz */
/* */
#define EEPROM_EXT_TX1_OVER_2DOT4G										0x6E

/* */
/* Tx2 power over 2.4 GHz */
/* */
#define EEPROM_EXT_TX2_OVER_2DOT4G										0x7C

/* */
/* Tx0 power over 5 GHz */
/* */
#define EEPROM_EXT_TX0_OVER_5G											0x96

/* */
/* Tx1 power over 5 GHz */
/* */
#define EEPROM_EXT_TX1_OVER_5G											0xCA

/* */
/* Tx2 power over 5 GHz */
/* */
#define EEPROM_EXT_TX2_OVER_5G											0xFE

/* */
/* Tx power delta TSSI bounday over 2.4 GHz */
/* */
#define EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G						0x8A

/* */
/* Tx power delta TSSI bounday over 5 GHz */
/* */
#define EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G							0x134

/* */
/* Tx ALC step value for 2.4 GHz */
/* */
#define EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_2DOT4G						0x93

/* */
/* Tx ALC step value for 5 GHz */
/* */
#define EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_5G							0x13D

/* */
/* Tx power control over BW20 at 2.4G */
/* */
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_2DOT4G							0x140

/* */
/* Tx power control over BW40 at 2.4G */
/* */
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_2DOT4G							0x150

/* */
/* Tx power control over BW20 at 5G */
/* */
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW20_5G								0x160

/* */
/* Tx power control over BW40 at 5G */
/* */
#define EEPROM_EXT_TX_PWR_CTRL_OVER_BW40_5G								0x170

/* */
/* The 2.4G manual channel */
/* */
#define EEPROM_EXT_2DOTG_MANUAL_CHANNEL_OFFSET							0x190

/* */
/* The 5G manual channel (part #1) */
/* */
#define EEPROM_EXT_5G_MANUAL_CAHNNEL_PART_ONE_OFFSET					0x192

/* */
/* The 5G manual channel (part #2) */
/* */
#define EEPROM_EXT_5G_MANUAL_CHANNEL_PART_TWO_OFFSET					0x194

/* work around */
#define RT3593_WA_MONITOR(__pAd)											\
	if (IS_RT3593(__pAd))														\
	{																		\
		RTMPSendNullFrame(__pAd, __pAd->CommonCfg.TxRate,					\
			(OPSTATUS_TEST_FLAG(__pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),\
			(__pAd)->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : (__pAd)->StaCfg.Psm);\
	}

/* receive frequency offset */
#define RT3593_FREQ_OFFSET_RCV_CHECK(__pAd, __pRxD, __pRxWI, __pHeader)		\
	if (IS_RT3593(__pAd))													\
	{																		\
		if ((__pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE) && \
			(INFRA_ON(__pAd)) &&											\
			(__pRxD->Crc == 0) &&											\
			(__pHeader->FC.Type == BTYPE_MGMT) &&							\
			(__pHeader->FC.SubType == SUBTYPE_BEACON) &&					\
			(MAC_ADDR_EQUAL(&__pAd->CommonCfg.Bssid, &__pHeader->Addr2)))	\
		{																	\
			__pAd->FreqCalibrationCtrl.LatestFreqOffsetOverBeacon =			\
										GetFrequencyOffset(pAd, __pRxWI);	\
			DBGPRINT(RT_DEBUG_INFO, ("%s: Beacon, CRC error = %d,			\
					__pHeader->Sequence = %d,								\
					SA = %02X:%02X:%02X:%02X:%02X:%02X,						\
					frequency offset = %d, MCS = %d, BW = %d\n",			\
					__FUNCTION__,											\
					__pRxD->Crc, 											\
					__pHeader->Sequence,									\
					__pHeader->Addr2[0], __pHeader->Addr2[1],				\
					__pHeader->Addr2[2], __pHeader->Addr2[3],				\
					__pHeader->Addr2[4], __pHeader->Addr2[5],				\
					((CHAR)(__pRxWI->FOFFSET)),								\
					__pRxWI->MCS,											\
					__pRxWI->BW));											\
		}																	\
	}

/* select DAC according to HT or Legacy */
#define RT3593_DAC_SELECT(__pAd, __Value)									\
	if (__pAd->Antenna.field.TxPath == 3)									\
		__Value |= 0x10;													\
	else if (__pAd->Antenna.field.TxPath == 2)								\
		__Value |= 0x08;

/* reset Tx/Rx RDG threshold */
/*
	Or the throughput can not be increased due to rate switch function
	in station mode.
*/
#define RT3593_RDG_DISABLE(__pAd)											\
{																			\
	TXOP_THRESHOLD_CFG_STRUC __TxopThCfg = {{0}};							\
	RTMP_IO_READ32(__pAd, TXOP_THRES_CFG, &__TxopThCfg.word);				\
	__TxopThCfg.field.RDG_IN_THRES = 0;										\
	__TxopThCfg.field.RDG_OUT_THRES = 0;									\
	RTMP_IO_WRITE32(__pAd, TXOP_THRES_CFG, __TxopThCfg.word);				\
}

/* write minimum AMPDU size to MAC register */
#define RT3593_MINIMUM_AMPDU_RESET(__pAd, __Size)							\
	if (IS_RT3593(__pAd))													\
	{																		\
		/*	MinimumAMPDUSize = (0x000A0fff | (MinimumAMPDUSize << 12)); */	\
		__Size = (0x00001000 | (__Size << 12));								\
		if (__pAd->ApCfg.MAX_PSDU_LEN != __Size)							\
		{																	\
			DBGPRINT(RT_DEBUG_TRACE, ("Write 0x%x to MAC register "			\
					"MAX_LEN_CFG (offset: 0x%x)\n", __Size, MAX_LEN_CFG));	\
			/* TODO: temp disable the line */								\
			/*		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, MinimumAMPDUSize); */	\
			__pAd->ApCfg.MAX_PSDU_LEN = __Size;								\
			/* pAd->ApCfg.MAX_PSDU_LEN is used in BA win size decision, BA_MaxWinSizeReasign() */\
		}																	\
	}

#define RT3593_MAXIMUM_PSDU_LEN_SET(__pAd)									\
	if (IS_RT3593(__pAd))													\
	{																		\
		/*	RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, 0x000A2fff); */				\
		__pAd->ApCfg.MAX_PSDU_LEN = MAX_AGGREGATION_SIZE | 0x00001000;		\
	}

/* get LED configuration */
#define RT3593_LED_CONFIG_GET(__pAd, __pLedCntl)							\
{																			\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(pAd, EEPROM_EXT_LED_MODE_SETTING - 1, __Value);	\
	__pLedCntl->MCULedCntl.word = ((__Value & 0xFF00) >> 8);				\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_LED_AG_CONFIGURATION, __Value);	\
	__pLedCntl->LedAGCfg = __Value;											\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_LED_ACT_CONFIGURATION, __Value);	\
	__pLedCntl->LedACTCfg = __Value;										\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_LED_AG_ACT_POLARITY, __Value);	\
	__pLedCntl->LedPolarity = __Value;										\
}

/* get BA maximum size */
#ifdef CONFIG_AP_SUPPORT
#define RT3593_AP_MAX_BW_SIZE_GET(__pAd, __pEntry, __MaxSize)				\
	IF_DEV_CONFIG_OPMODE_ON_AP(__pAd)										\
		if (IS_RT3593(__pAd) && (__pEntry->HTCapability.MCSSet[2] != 0x00))	\
			__MaxSize = 31;
#endif /* CONFIG_AP_SUPPORT */


/* update channel in monitor mode */

/* read BBP R66 value */
#define RT3593_R66_MID_LOW_SENS_GET(__pAd, __Value)							\
	if (IS_RT3593(__pAd))													\
		__Value = (GET_LNA_GAIN(__pAd) * 5 / 3) + 0x20 + 0x10; /* 20MBW over 5GHz: EXT_LNA * 1.66 + 0x20*/

#define RT3593_R66_NON_MID_LOW_SEMS_GET(__pAd, __Value)						\
	if (IS_RT3593(__pAd))													\
		__Value = (GET_LNA_GAIN(pAd) * 5 / 3) + 0x20; /* 40MBW over 5GHz: EXT_LNA * 1.66 + 0x20*/

/* AutoFallback enable/disable */
#define RT3593_AUTO_FALLBACK_ENABLE(__pAd)									\
	if (IS_RT3593(__pAd))													\
	{																		\
		TX_RTY_CFG_STRUC __TxRtyCfg;										\
		RTMP_IO_READ32(__pAd, TX_RTY_CFG, &__TxRtyCfg.word);				\
		__TxRtyCfg.field.TxautoFBEnable = 1;								\
		RTMP_IO_WRITE32(__pAd, TX_RTY_CFG, __TxRtyCfg.word);				\
	}

/* read value from EEPROM */
#define RT3593_EEPROM_COUNTRY_REGION_READ(__pAd)							\
{																			\
	UCHAR __CountryRegion5G = 0, __CountryRegion2Dot4G = 0;					\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_COUNTRY_REGION_CODE_FOR_5G_BAND - 1), __Value);\
	/* Swap*/																\
	__CountryRegion5G = ((__Value & 0xFF00) >> 8);							\
	__CountryRegion2Dot4G = (__Value & 0x00FF);								\
	__Value = ((__CountryRegion2Dot4G << 8) | __CountryRegion5G);			\
	__pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = __Value;			\
}

#define RT3593_EEPROM_TSSI_24G_READ(__pAd)									\
{																			\
	EEPROM_TX_PWR_STRUC __Power;											\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G, __Power.word);\
	__pAd->TssiMinusBoundaryG[4] = __Power.field.Byte0;						\
	__pAd->TssiMinusBoundaryG[3] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 2), __Power.word);\
	__pAd->TssiMinusBoundaryG[2] = __Power.field.Byte0;						\
	__pAd->TssiMinusBoundaryG[1] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 4), __Power.word);\
	__pAd->TssiRefG = __Power.field.Byte0;									\
	__pAd->TssiPlusBoundaryG[1] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 6), __Power.word);\
	__pAd->TssiPlusBoundaryG[2] = __Power.field.Byte0;						\
	__pAd->TssiPlusBoundaryG[3] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_2DOT4G + 8), __Power.word);\
	__pAd->TssiPlusBoundaryG[4] = __Power.field.Byte0;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_2DOT4G - 1), __Value);\
	__pAd->TxAgcStepG = ((__Value & 0xFF00) >> 8);							\
	__pAd->TxAgcCompensateG = 0;											\
	__pAd->TssiMinusBoundaryG[0] = __pAd->TssiRefG;							\
	__pAd->TssiPlusBoundaryG[0]  = __pAd->TssiRefG;							\
	/* Disable TxAgc if the based value is not right */						\
	if (__pAd->TssiRefG == 0xFF)											\
		__pAd->bAutoTxAgcG = FALSE;											\
}

#define RT3593_EEPROM_TSSI_5G_READ(__pAd)									\
{																			\
	EEPROM_TX_PWR_STRUC __Power;											\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G, __Power.word);\
	__pAd->TssiMinusBoundaryA[4] = __Power.field.Byte0;						\
	__pAd->TssiMinusBoundaryA[3] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 2), __Power.word);\
	__pAd->TssiMinusBoundaryA[2] = __Power.field.Byte0;						\
	__pAd->TssiMinusBoundaryA[1] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 4), __Power.word);\
	__pAd->TssiRefA = __Power.field.Byte0;									\
	__pAd->TssiPlusBoundaryA[1] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 6), __Power.word);\
	__pAd->TssiPlusBoundaryA[2] = __Power.field.Byte0;						\
	__pAd->TssiPlusBoundaryA[3] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_DELTA_TSSI_BOUNDARY_OVER_5G + 8), __Power.word);\
	__pAd->TssiPlusBoundaryA[4] = __Power.field.Byte0;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_TX_ALC_STEP_VALUE_OVER_5G - 1), __Value);\
	__pAd->TxAgcStepA = ((__Value & 0xFF00) >> 8);							\
	__pAd->TxAgcCompensateA = 0;											\
	__pAd->TssiMinusBoundaryA[0] = __pAd->TssiRefA;							\
	__pAd->TssiPlusBoundaryA[0]  = __pAd->TssiRefA;							\
	/* Disable TxAgc if the based value is not right */						\
	if (__pAd->TssiRefA == 0xFF)											\
		__pAd->bAutoTxAgcA = FALSE;											\
}

#define RT3593_EEPROM_RSSI01_OFFSET_24G_READ(__pAd)							\
{																			\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_RSSI0_OVER_2DOT4G, __Value);		\
	__pAd->BGRssiOffset0 = (__Value & 0x00FF);								\
	__pAd->BGRssiOffset1 = ((__Value >> 8) & 0x00FF);						\
}

#define RT3593_EEPROM_RSSI2_OFFSET_ALNAGAIN1_24G_READ(__pAd)					\
{																			\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_RSSI2_OVER_2DOT4G, __Value);		\
	__pAd->BGRssiOffset2 = (__Value & 0x00FF);								\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH100_TO_CH128, __Value);\
	__pAd->ALNAGain1 = (__Value & 0x00FF);									\
}

#define RT3593_EEPROM_BLNA_ALNA_GAIN0_24G_READ(__pAd)						\
{																			\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_2DOT4G, __Value);\
	__pAd->BLNAGain = (__Value & 0x00FF);									\
	__pAd->ALNAGain0 = (__Value >> 8);										\
}

#define RT3593_EEPROM_RSSI01_OFFSET_5G_READ(__pAd)							\
{																			\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_RSSI0_OVER_5G, __Value);			\
	__pAd->ARssiOffset0 = (__Value & 0x00FF);								\
	__pAd->ARssiOffset1 = ((__Value >> 8) & 0x00FF);						\
}

#define RT3593_EEPROM_RSSI2_OFFSET_ALNAGAIN2_5G_READ(__pAd)					\
{																			\
	USHORT __Value;															\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_EXT_RSSI2_OVER_5G, __Value);			\
	__pAd->ARssiOffset2 = (__Value & 0x00FF);								\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_EXT_EXTERNAL_LNA_GAIN_FOR_5G_OVER_CH132_TO_CH165 - 1), __Value);\
	__pAd->ALNAGain2 = ((__Value >> 8) & 0x00FF);							\
}

/* config some registers by antenna number */
#define RT3593_CONFIG_SET_BY_ANTENNA(__pAd)									\
{																			\
	if (__pAd->Antenna.field.RxPath == 1) /* Rx = 1 antenna */				\
	{																		\
		RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R86, 0x00);					\
	}																		\
	else if ((__pAd->Antenna.field.RxPath == 2) ||							\
			(__pAd->Antenna.field.RxPath == 3)) /* Rx = 2/3 antennas*/		\
	{																		\
		RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R86, 0x46);					\
	}																		\
	else																	\
	{																		\
		RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R86, 0x46);					\
	}																		\
}

/* adjust BBP R1 initialization value based on antenna number */
#define RT3593_BBPR1_INIT(__pAd, __BBPR1)									\
{																			\
	if (__pAd->Antenna.field.TxPath == 3)									\
	{																		\
		/* Number of transmitter chains availabe (N_TXER, 2: N_TXER = 3) */	\
		__BBPR1 = ((__BBPR1 & ~0x18) | 0x10);								\
	}																		\
	else if (__pAd->Antenna.field.TxPath == 2)								\
	{																		\
		/* Number of transmitter chains availabe (N_TXER, 1: N_TXER = 2) */	\
		__BBPR1 = ((__BBPR1 & ~0x18) | 0x08);								\
	}																		\
	else if (__pAd->Antenna.field.TxPath == 1)								\
	{																		\
		/* Number of transmitter chains availabe (N_TXER, 0: N_TXER = 1) */	\
		__BBPR1 = ((__BBPR1 & ~0x18) | 0x00);								\
	}																		\
}

/* SNR mapping */
#define RT3593_SNR_MAPPING_INIT(__pAd)										\
{																			\
	RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R142, 6);						\
	RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R143, 160);						\
	RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R142, 7);						\
	RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R143, 161);						\
	RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R142, 8);						\
	RTMP_BBP_IO_WRITE8_BY_REG_ID(__pAd, BBP_R143, 162);						\
}


/* Public functions */
VOID RT3593_Init(
	IN struct _RTMP_ADAPTER			*pAd);

VOID NICInitRT3593MacRegisters(
	IN struct _RTMP_ADAPTER			*pAd);

VOID NICInitRT3593BbpRegisters(
	IN struct _RTMP_ADAPTER			*pAd);

VOID NICInitRT3593RFRegisters(
	IN struct _RTMP_ADAPTER			*pAd);

VOID RTMPVerifyTxPwrPerRateExt(
	IN		struct _RTMP_ADAPTER	*pAd, 
	INOUT	PUCHAR					pTxPwr);

VOID RTMPReadTxPwrPerRateExt(
	IN struct _RTMP_ADAPTER			*pAd);

VOID RT3593_AsicGetTxPowerOffset(
	IN 		struct _RTMP_ADAPTER			*pAd,
	INOUT 	PULONG 						pTxPwr);

void RT3593_AsicSetFreqOffset(
	IN 		struct _RTMP_ADAPTER *pAd,
	IN 		ULONG 			freqOffset);
#endif /* RT3593 */
#endif /*__RT3593_H__ */

/* End of rt3593.h */
