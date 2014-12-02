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
	rtmp_chip.h

	Abstract:
	Ralink Wireless Chip related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef	__RTMP_CHIP_H__
#define	__RTMP_CHIP_H__

#include "rtmp_type.h"

struct _RTMP_ADAPTER;
struct _RSSI_SAMPLE;

#ifdef RTMP_MAC_PCI
#include "chip/mac_pci.h"
#endif /* RTMP_MAC_PCI */

#ifdef RT2880
#include "chip/rt2880.h"
#include "chip/rt28xx.h"
#endif /* RT2880 */

#ifdef RT3883
#include "chip/rt3883.h"
#endif /* RT3883 */

#ifdef RT305x
#include "chip/rt305x.h"
#endif /* RT305x */

#ifdef RT3352
#include "chip/rt3352.h"
#endif /* RT3352 */

#ifdef RT5350
#include "chip/rt5350.h"
#endif /* RT5350 */

#ifdef RT6352
#include "chip/rt6352.h"
#endif /* RT6352 */

#define IS_RT3090A(_pAd)    ((((_pAd)->MACVersion & 0xffff0000) == 0x30900000))

/* We will have a cost down version which mac version is 0x3090xxxx */
#define IS_RT3090(_pAd)     ((((_pAd)->MACVersion & 0xffff0000) == 0x30710000) || (IS_RT3090A(_pAd)))

#define IS_RT3070(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30700000)
#define IS_RT3071(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30710000)
#define IS_RT2070(_pAd)		(((_pAd)->RfIcType == RFIC_2020) || ((_pAd)->EFuseTag == 0x27))

#define IS_RT2860(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x28600000)
#define IS_RT2870(_pAd)		(IS_RT2860(_pAd) && IS_USB_INF(_pAd))
#define IS_RT2872(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x28720000)
#define IS_RT2880(_pAd)		(IS_RT2860(_pAd) && IS_RBUS_INF(_pAd))

#define IS_RT30xx(_pAd)		(((_pAd)->MACVersion & 0xfff00000) == 0x30700000||IS_RT3090A(_pAd)||IS_RT3390(_pAd))

#define IS_RT3052B(_pAd)	(((_pAd)->CommonCfg.CID == 0x102) && (((_pAd)->CommonCfg.CN >> 16) == 0x3033)) 
#define IS_RT3052(_pAd)		(((_pAd)->MACVersion == 0x28720200) && (_pAd->Antenna.field.TxPath == 2))
#define IS_RT3050(_pAd)		(((_pAd)->MACVersion == 0x28720200) && ((_pAd)->RfIcType == RFIC_3020))
#define IS_RT3350(_pAd)		(((_pAd)->MACVersion == 0x28720200) && ((_pAd)->RfIcType == RFIC_3320))
#define IS_RT3352(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x33520000)
#define IS_RT5350(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x53500000)
#define IS_RT305x(_pAd)		(IS_RT3050(_pAd) || IS_RT3052(_pAd) || IS_RT3350(_pAd) || IS_RT3352(_pAd) || IS_RT5350(_pAd))
#define IS_RT3050_3052_3350(_pAd) (\
	((_pAd)->MACVersion == 0x28720200) && \
	((((_pAd)->CommonCfg.CN >> 16) == 0x3333) || (((_pAd)->CommonCfg.CN >> 16) == 0x3033)) \
)
#define IS_RT6352(_pAd)		((((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000) && ((_pAd)->infType == RTMP_DEV_INF_RBUS))


/* RT3572, 3592, 3562, 3062 share the same MAC version */
#define IS_RT3572(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x35720000)

/* Check if it is RT3xxx, or Specified ID in registry for debug */
#define IS_DEV_RT3xxx(_pAd)( \
	(_pAd->DeviceID == NIC3090_PCIe_DEVICE_ID) || \
	(_pAd->DeviceID == NIC3091_PCIe_DEVICE_ID) || \
	(_pAd->DeviceID == NIC3092_PCIe_DEVICE_ID) || \
	(_pAd->DeviceID == NIC3592_PCIe_DEVICE_ID) || \
	((_pAd->DeviceID == NIC3593_PCI_OR_PCIe_DEVICE_ID) && (RT3593OverPCIe(_pAd))) \
)

#define RT3593_DEVICE_ID_CHECK(__DevId)			\
	(0)

#define RT3592_DEVICE_ID_CHECK(__DevId)			\
	(__DevId == NIC3592_PCIe_DEVICE_ID)

#ifdef RT2883
#define IS_RT2883(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x28830000)
#else
#define IS_RT2883(_pAd)		(0)
#endif /* RT2883 */

#ifdef RT3883
#define IS_RT3883(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x38830000)
#else
#define IS_RT3883(_pAd)		(0)
#endif /* RT3883 */

#define IS_VERSION_BEFORE_F(_pAd)			(((_pAd)->MACVersion&0xffff) <= 0x0211)
/* F version is 0x0212, E version is 0x0211. 309x can save more power after F version. */
#define IS_VERSION_AFTER_F(_pAd)			((((_pAd)->MACVersion&0xffff) >= 0x0212) || (((_pAd)->b3090ESpecialChip == TRUE)))

#define IS_RT3290(_pAd)	(((_pAd)->MACVersion & 0xffff0000) == 0x32900000)
#define IS_RT3290LE(_pAd)   ((((_pAd)->MACVersion & 0xffffffff) >= 0x32900011))

/* 3593 */
#define IS_RT3593(_pAd) (((_pAd)->MACVersion & 0xFFFF0000) == 0x35930000)

/* RT5392 */
#define IS_RT5392(_pAd)   ((_pAd->MACVersion & 0xFFFF0000) == 0x53920000) /* Include RT5392, RT5372 and RT5362 */

/* RT5390 */
#define IS_RT5390(_pAd)   ((((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000) || IS_RT5390H(_pAd)) /* Include RT5390, RT5370 and RT5360 */

/* RT5390F */
#define IS_RT5390F(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0502))

/* RT5370G */
#define IS_RT5370G(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0503)) /* support HW PPAD ( the hardware rx antenna diversity ) */

/* RT5390R */
#define IS_RT5390R(_pAd)   ((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) == 0x1502)) /* support HW PPAD ( the hardware rx antenna diversity ) */

/* PCIe interface NIC */
#define IS_MINI_CARD(_pAd) ((_pAd)->Antenna.field.BoardType == BOARD_TYPE_MINI_CARD)

/* 5390U (5370 using PCIe interface) */
#define IS_RT5390U(_pAd)   (IS_MINI_CARD(_pAd) && ((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000)

/* RT5390H */
#define IS_RT5390H(_pAd)   (((_pAd->MACVersion & 0xFFFF0000) == 0x53910000) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x1500))

/* RT5390BC8 (WiFi + BT) */


/* RT5390D */
#define IS_RT5390D(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0502))

/* RT5392C */
#define IS_RT5392C(_pAd)	((IS_RT5392(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0222)) /* Include RT5392, RT5372 and RT5362 */

#define IS_RT5592(_pAd)		(((_pAd)->MACVersion & 0xFFFF0000) == 0x55920000)
#define REV_RT5592C 0x0221

/* RT3592BC8 (WiFi + BT) */


#define IS_USB_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_USB)
#define IS_PCIE_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_PCIE)
#define IS_PCI_INF(_pAd)		(((_pAd)->infType == RTMP_DEV_INF_PCI) || IS_PCIE_INF(_pAd))
#define IS_PCI_ONLY_INF(_pAd)	((_pAd)->infType == RTMP_DEV_INF_PCI)
#define IS_RBUS_INF(_pAd) ((_pAd)->infType == RTMP_DEV_INF_RBUS)

#define RT_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) < (_rev))

#define RT_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) >= (_rev))

/* Dual-band NIC (RF/BBP/MAC are in the same chip.) */

#define IS_RT_NEW_DUAL_BAND_NIC(_pAd) ((FALSE))


/* Is the NIC dual-band NIC? */

#define IS_DUAL_BAND_NIC(_pAd) (((_pAd->RfIcType == RFIC_2850) || (_pAd->RfIcType == RFIC_2750) || (_pAd->RfIcType == RFIC_3052)		\
								|| (_pAd->RfIcType == RFIC_3053) || (_pAd->RfIcType == RFIC_2853) || (_pAd->RfIcType == RFIC_3853) 	\
								|| IS_RT_NEW_DUAL_BAND_NIC(_pAd)) && !IS_RT5390(_pAd))


/* RT3593 over PCIe bus */
#define RT3593OverPCIe(_pAd) (IS_RT3593(_pAd) && (_pAd->CommonCfg.bPCIeBus == TRUE))

/* RT3593 over PCI bus */
#define RT3593OverPCI(_pAd) (IS_RT3593(_pAd) && (_pAd->CommonCfg.bPCIeBus == FALSE))

/*RT3390,RT3370 */
#define IS_RT3390(_pAd)				(((_pAd)->MACVersion & 0xFFFF0000) == 0x33900000)

/* ------------------------------------------------------ */
/* PCI registers - base address 0x0000 */
/* ------------------------------------------------------ */
#define CHIP_PCI_CFG		0x0000
#define CHIP_PCI_EECTRL		0x0004
#define CHIP_PCI_MCUCTRL	0x0008

#define OPT_14			0x114

#define RETRY_LIMIT		10

/* ------------------------------------------------------ */
/* BBP & RF	definition */
/* ------------------------------------------------------ */
#define	BUSY		                1
#define	IDLE		                0

/*------------------------------------------------------------------------- */
/* EEPROM definition */
/*------------------------------------------------------------------------- */
#define EEDO                        0x08
#define EEDI                        0x04
#define EECS                        0x02
#define EESK                        0x01
#define EERL                        0x80

#define EEPROM_WRITE_OPCODE         0x05
#define EEPROM_READ_OPCODE          0x06
#define EEPROM_EWDS_OPCODE          0x10
#define EEPROM_EWEN_OPCODE          0x13

#define NUM_EEPROM_BBP_PARMS		19	/* Include NIC Config 0, 1, CR, TX ALC step, BBPs */
#define NUM_EEPROM_TX_G_PARMS		7

#define VALID_EEPROM_VERSION        1
#define EEPROM_VERSION_OFFSET       0x02
#define EEPROM_NIC1_OFFSET          0x34	/* The address is from NIC config 0, not BBP register ID */
#define EEPROM_NIC2_OFFSET          0x36	/* The address is from NIC config 1, not BBP register ID */

#if defined(RT2883) || defined(RT3883)
#define	EEPROM_NIC3_OFFSET          0x38	/* The address is from NIC config 2, not BBP register ID */
#endif /* defined(RT2883) || defined(RT3883) */

#if defined(RT2883) || defined(RT3883)
#define EEPROM_COUNTRY_REGION		0x3e
#else
#define EEPROM_COUNTRY_REGION			0x38
#endif /* defined(RT2883) || defined(RT3883) */

#if defined(RT2883) || defined(RT3883)
#define EEPROM_DEFINE_MAX_TXPWR		0x40
#else
#define EEPROM_DEFINE_MAX_TXPWR			0x4e
#endif /* defined(RT2883) || defined(RT3883) */

#if defined(RT2883) || defined(RT3883)
#ifdef RT2883
#define	EEPROM_FREQ_OFFSET			0x42
#define EEPROM_LEDAG_CONF_OFFSET	0x44
#define EEPROM_LEDACT_CONF_OFFSET	0x46
#define EEPROM_LED_POLARITY_OFFSET	0x48
#endif /* RT2883 */
#ifdef RT3883
#define	EEPROM_FREQ_OFFSET			0x44
#define EEPROM_LEDAG_CONF_OFFSET	0x46
#define EEPROM_LEDACT_CONF_OFFSET	0x48
#define EEPROM_LED_POLARITY_OFFSET	0x4a
#endif /* RT3883 */
#else
#define EEPROM_FREQ_OFFSET			0x3a
#define EEPROM_LEDAG_CONF_OFFSET	0x3c
#define EEPROM_LEDACT_CONF_OFFSET	0x3e
#define EEPROM_LED_POLARITY_OFFSET	0x40
#endif /* defined(RT2883) || defined(RT3883) */

#ifdef RT6352
#define	EEPROM_NIC3_OFFSET			0x42
#endif /* RT6352 */

#if defined(RT2883) || defined(RT3883)
#define EEPROM_LNA_OFFSET			0x4c
#define EEPROM_LNA_OFFSET2			0x4e
#else
#define EEPROM_LNA_OFFSET			0x44
#endif /* defined (RT2883) || defined (RT3883) */

#if defined(RT2883) || defined(RT3883)
#define EEPROM_RSSI_BG_OFFSET		0x50
#define EEPROM_RSSI_A_OFFSET		0x54
#else
#define EEPROM_RSSI_BG_OFFSET			0x46
#define EEPROM_RSSI_A_OFFSET			0x4a
#define EEPROM_TXMIXER_GAIN_2_4G		0x48
#define EEPROM_TXMIXER_GAIN_5G			0x4c
#endif /* defined(RT2883) || defined(RT3883) */

#define EEPROM_TXPOWER_DELTA			0x50	/* 20MHZ AND 40 MHZ use different power. This is delta in 40MHZ. */

#if defined(RT2883) || defined(RT3883)
#define	EEPROM_G_TX_PWR_OFFSET		0x60
#define	EEPROM_G_TX2_PWR_OFFSET		0x6e
#define	EEPROM_G_TX3_PWR_OFFSET		0x7c
#else
#define EEPROM_G_TX_PWR_OFFSET			0x52
#define EEPROM_G_TX2_PWR_OFFSET			0x60
#endif /* defined(RT2883) || defined(RT3883) */

#if defined(RT2883) || defined(RT3883)
#define EEPROM_G_TSSI_BOUND1		0x8a
#define EEPROM_G_TSSI_BOUND2		0x8c
#define EEPROM_G_TSSI_BOUND3		0x8e
#define EEPROM_G_TSSI_BOUND4		0x90
#define EEPROM_G_TSSI_BOUND5		0x92
#else
#define EEPROM_G_TSSI_BOUND1			0x6e
#define EEPROM_G_TSSI_BOUND2			0x70
#define EEPROM_G_TSSI_BOUND3			0x72
#define EEPROM_G_TSSI_BOUND4			0x74
#define EEPROM_G_TSSI_BOUND5			0x76
#endif /* defined(RT2883) || defined(RT3883) */

#if defined(RT2883) || defined(RT3883)
#define EEPROM_A_TX_PWR_OFFSET      0x96
#define EEPROM_A_TX2_PWR_OFFSET     0xca
#define EEPROM_A_TX3_PWR_OFFSET     0xfe
#else
#define EEPROM_A_TX_PWR_OFFSET      		0x78
#define EEPROM_A_TX2_PWR_OFFSET			0xa6
#endif /* defined (RT2883) || defined (RT3883) */

#define MBSSID_MODE0	0
#define MBSSID_MODE1	1	/* Enhance NEW MBSSID MODE mapping to mode 0 */
#ifdef ENHANCE_NEW_MBSSID_MODE
#define MBSSID_MODE2	2	/* Enhance NEW MBSSID MODE mapping to mode 1 */
#define MBSSID_MODE3	3	/* Enhance NEW MBSSID MODE mapping to mode 2 */
#define MBSSID_MODE4	4	/* Enhance NEW MBSSID MODE mapping to mode 3 */
#define MBSSID_MODE5	5	/* Enhance NEW MBSSID MODE mapping to mode 4 */
#define MBSSID_MODE6	6	/* Enhance NEW MBSSID MODE mapping to mode 5 */
#endif /* ENHANCE_NEW_MBSSID_MODE */

enum FREQ_CAL_INIT_MODE {
	FREQ_CAL_INIT_MODE0,
	FREQ_CAL_INIT_MODE1,
	FREQ_CAL_INIT_MODE2,
	FREQ_CAL_INIT_UNKNOW,
};

enum FREQ_CAL_MODE {
	FREQ_CAL_MODE0,
	FREQ_CAL_MODE1,
};

enum RXWI_FRQ_OFFSET_FIELD {
	RXWI_FRQ_OFFSET_FIELD0, /* SNR1 */
	RXWI_FRQ_OFFSET_FIELD1, /* Frequency Offset */
};


#if defined(RT2883) || defined(RT3883)
#define EEPROM_A_TSSI_BOUND1				0x134
#define EEPROM_A_TSSI_BOUND2				0x136
#define EEPROM_A_TSSI_BOUND3				0x138
#define EEPROM_A_TSSI_BOUND4				0x13a
#define EEPROM_A_TSSI_BOUND5				0x13c
#else
#define EEPROM_A_TSSI_BOUND1		0xd4
#define EEPROM_A_TSSI_BOUND2		0xd6
#define EEPROM_A_TSSI_BOUND3		0xd8
#define EEPROM_A_TSSI_BOUND4		0xda
#define EEPROM_A_TSSI_BOUND5		0xdc
#endif /* defined(RT2883) || defined(RT3883) */

/* ITxBF calibration values EEPROM locations 0x1a0 to 0x1ab */
#define EEPROM_ITXBF_CAL				0x1a0

#if defined(RT2883) || defined(RT3883)
#ifdef RT2883
#define EEPROM_TXPOWER_BYRATE_CCK_OFDM		0x140
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G	0x146	/* 20MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G	0x156	/* 40MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G		0x166	/* 20MHZ 5G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G		0x176	/* 40MHZ 5G tx power. */
#endif /* RT2883 */
#ifdef RT3883
#define EEPROM_TXPOWER_BYRATE_CCK_OFDM		0x140
#define EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_2_4G	0x150
#define EEPROM_TXPOWER_BYRATE_20MHZ_OFDM_5G	0x160
#define EEPROM_TXPOWER_BYRATE_40MHZ_OFDM_5G	0x170
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G	0x144	/* 20MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G	0x154	/* 40MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G		0x164	/* 20MHZ 5G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G		0x174	/* 40MHZ 5G tx power. */
#endif /* RT3883 */
#else
#define EEPROM_TXPOWER_BYRATE 			0xde	/* 20MHZ power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G	0xde	/* 20MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G	0xee	/* 40MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G		0xfa	/* 20MHZ 5G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G		0x10a	/* 40MHZ 5G tx power. */
#endif /* defined(RT2883) || defined(RT3883) */

#if defined(RT2883) || defined(RT3883)
#define	EEPROM_BBP_BASE_OFFSET			0x186
#else
#define EEPROM_BBP_BASE_OFFSET			0xf0	/* The address is from NIC config 0, not BBP register ID */
#endif /* defined(RT2883) || defined(RT3883) */

/* */
/* Bit mask for the Tx ALC and the Tx fine power control */
/* */
#define GET_TX_ALC_BIT_MASK					0x1F	/* Valid: 0~31, and in 0.5dB step */
#define GET_TX_FINE_POWER_CTRL_BIT_MASK	0xE0	/* Valid: 0~4, and in 0.1dB step */
#define NUMBER_OF_BITS_FOR_TX_ALC			5	/* The length, in bit, of the Tx ALC field */


/* TSSI gain and TSSI attenuation */

#define EEPROM_TSSI_GAIN_AND_ATTENUATION	0x76

/*#define EEPROM_Japan_TX_PWR_OFFSET      0x90 // 802.11j */
/*#define EEPROM_Japan_TX2_PWR_OFFSET      0xbe */
/*#define EEPROM_TSSI_REF_OFFSET	0x54 */
/*#define EEPROM_TSSI_DELTA_OFFSET	0x24 */
/*#define EEPROM_CCK_TX_PWR_OFFSET  0x62 */
/*#define EEPROM_CALIBRATE_OFFSET	0x7c */

#define EEPROM_NIC_CFG1_OFFSET		0
#define EEPROM_NIC_CFG2_OFFSET		1
#define EEPROM_NIC_CFG3_OFFSET		2
#define EEPROM_COUNTRY_REG_OFFSET	3
#define EEPROM_BBP_ARRAY_OFFSET		4

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 
/* */
/* The TSSI over OFDM 54Mbps */
/* */
#define EEPROM_TSSI_OVER_OFDM_54		0x6E

/* */
/* The TSSI value/step (0.5 dB/unit) */
/* */
#define EEPROM_TSSI_STEP_OVER_2DOT4G	0x77
#define EEPROM_TSSI_STEP_OVER_5DOT5G	0xDD
#define TSSI_READ_SAMPLE_NUM			3

/* */
/* Per-channel Tx power offset (for the extended TSSI mode) */
/* */
#define EEPROM_TX_POWER_OFFSET_OVER_CH_1	0x6F
#define EEPROM_TX_POWER_OFFSET_OVER_CH_3	0x70
#define EEPROM_TX_POWER_OFFSET_OVER_CH_5	0x71
#define EEPROM_TX_POWER_OFFSET_OVER_CH_7	0x72
#define EEPROM_TX_POWER_OFFSET_OVER_CH_9	0x73
#define EEPROM_TX_POWER_OFFSET_OVER_CH_11	0x74
#define EEPROM_TX_POWER_OFFSET_OVER_CH_13	0x75

/* */
/* Tx power configuration (bit3:0 for Tx0 power setting and bit7:4 for Tx1 power setting) */
/* */
#define EEPROM_CCK_MCS0_MCS1				0xDE
#define EEPROM_CCK_MCS2_MCS3				0xDF
#define EEPROM_OFDM_MCS0_MCS1			0xE0
#define EEPROM_OFDM_MCS2_MCS3			0xE1
#define EEPROM_OFDM_MCS4_MCS5			0xE2
#define EEPROM_OFDM_MCS6_MCS7			0xE3
#define EEPROM_HT_MCS0_MCS1				0xE4
#define EEPROM_HT_MCS2_MCS3				0xE5
#define EEPROM_HT_MCS4_MCS5				0xE6
#define EEPROM_HT_MCS6_MCS7				0xE7
#define EEPROM_HT_MCS8_MCS9                     	0xE8
#define EEPROM_HT_MCS10_MCS11                   	0xE9
#define EEPROM_HT_MCS12_MCS13                   	0xEA
#define EEPROM_HT_MCS14_MCS15                   	0xEB
#define EEPROM_HT_USING_STBC_MCS0_MCS1	0xEC
#define EEPROM_HT_USING_STBC_MCS2_MCS3	0xED
#define EEPROM_HT_USING_STBC_MCS4_MCS5	0xEE
#define EEPROM_HT_USING_STBC_MCS6_MCS7	0xEF

/* */
/* Bit mask for the Tx ALC and the Tx fine power control */
/* */

#define DEFAULT_BBP_TX_FINE_POWER_CTRL 	0

#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */


/*
  *   EEPROM operation related marcos
  */
#define RT28xx_EEPROM_READ16(_pAd, _offset, _value)			\
	(_pAd)->chipOps.eeread((RTMP_ADAPTER *)(_pAd), (USHORT)(_offset), (PUSHORT)&(_value))

#define RT28xx_EEPROM_WRITE16(_pAd, _offset, _value)		\
	(_pAd)->chipOps.eewrite((RTMP_ADAPTER *)(_pAd), (USHORT)(_offset), (USHORT)(_value))

/* ------------------------------------------------------------------- */
/*  E2PROM data layout */
/* ------------------------------------------------------------------- */

/* Board type */

#define BOARD_TYPE_MINI_CARD		0	/* Mini card */
#define BOARD_TYPE_USB_PEN		1	/* USB pen */

/* */
/* EEPROM antenna select format */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_ANTENNA_STRUC {
	struct {
		USHORT RssiIndicationMode:1; 	/* RSSI indication mode */
		USHORT Rsv:1;
		USHORT BoardType:2; 		/* 0: mini card; 1: USB pen */
		USHORT RfIcType:4;			/* see E2PROM document */
		USHORT TxPath:4;			/* 1: 1T, 2: 2T, 3: 3T */
		USHORT RxPath:4;			/* 1: 1R, 2: 2R, 3: 3R */
	} field;
	USHORT word;
} EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#else
typedef union _EEPROM_ANTENNA_STRUC {
	struct {
		USHORT RxPath:4;			/* 1: 1R, 2: 2R, 3: 3R */
		USHORT TxPath:4;			/* 1: 1T, 2: 2T, 3: 3T */
		USHORT RfIcType:4;			/* see E2PROM document */
		USHORT BoardType:2; 		/* 0: mini card; 1: USB pen */
		USHORT Rsv:1;
		USHORT RssiIndicationMode:1; 	/* RSSI indication mode */	
	} field;
	USHORT word;
} EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		USHORT DACTestBit:1;	/* control if driver should patch the DAC issue */
		USHORT CoexBit:1;
		USHORT bInternalTxALC:1;	/* Internal Tx ALC */
		USHORT AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		USHORT AntDiversity:1;	/* Antenna diversity */
		USHORT Rsv1:1;	/* must be 0 */
		USHORT BW40MAvailForA:1;	/* 0:enable, 1:disable */
		USHORT BW40MAvailForG:1;	/* 0:enable, 1:disable */
		USHORT EnableWPSPBC:1;	/* WPS PBC Control bit */
		USHORT BW40MSidebandForA:1;
		USHORT BW40MSidebandForG:1;
		USHORT CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		USHORT ExternalLNAForA:1;	/* external LNA enable for 5G */
		USHORT ExternalLNAForG:1;	/* external LNA enable for 2.4G */
		USHORT DynamicTxAgcControl:1;	/* */
		USHORT HardwareRadioControl:1;	/* Whether RF is controlled by driver or HW. 1:enable hw control, 0:disable */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#else
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		USHORT HardwareRadioControl:1;	/* 1:enable, 0:disable */
		USHORT DynamicTxAgcControl:1;	/* */
		USHORT ExternalLNAForG:1;	/* */
		USHORT ExternalLNAForA:1;	/* external LNA enable for 2.4G */
		USHORT CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		USHORT BW40MSidebandForG:1;
		USHORT BW40MSidebandForA:1;
		USHORT EnableWPSPBC:1;	/* WPS PBC Control bit */
		USHORT BW40MAvailForG:1;	/* 0:enable, 1:disable */
		USHORT BW40MAvailForA:1;	/* 0:enable, 1:disable */
		USHORT Rsv1:1;	/* must be 0 */
		USHORT AntDiversity:1;	/* Antenna diversity */
		USHORT AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		USHORT bInternalTxALC:1;	/* Internal Tx ALC */
		USHORT CoexBit:1;
		USHORT DACTestBit:1;	/* control if driver should patch the DAC issue */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#endif


/* */
/* TX_PWR Value valid range 0xFA(-6) ~ 0x24(36) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte1;	/* High Byte */
		signed char Byte0;	/* Low Byte */
	} field;
	USHORT word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#else
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte0;	/* Low Byte */
		signed char Byte1;	/* High Byte */
	} field;
	USHORT word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_VERSION_STRUC {
	struct {
		UCHAR Version;	/* High Byte */
		UCHAR FaeReleaseNumber;	/* Low Byte */
	} field;
	USHORT word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#else
typedef union _EEPROM_VERSION_STRUC {
	struct {
		UCHAR FaeReleaseNumber;	/* Low Byte */
		UCHAR Version;	/* High Byte */
	} field;
	USHORT word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_LED_STRUC {
	struct {
		USHORT Rsvd:3;	/* Reserved */
		USHORT LedMode:5;	/* Led mode. */
		USHORT PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		USHORT PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		USHORT PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		USHORT PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		USHORT PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		USHORT PolarityACT:1;	/* Polarity ACT setting. */
		USHORT PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		USHORT PolarityRDY_G:1;	/* Polarity RDY_G setting. */
	} field;
	USHORT word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#else
typedef union _EEPROM_LED_STRUC {
	struct {
		USHORT PolarityRDY_G:1;	/* Polarity RDY_G setting. */
		USHORT PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		USHORT PolarityACT:1;	/* Polarity ACT setting. */
		USHORT PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		USHORT PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		USHORT PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		USHORT PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		USHORT PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		USHORT LedMode:5;	/* Led mode. */
		USHORT Rsvd:3;	/* Reserved */
	} field;
	USHORT word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		UCHAR TxPowerEnable:1;	/* Enable */
		UCHAR Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		UCHAR DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
	} field;
	UCHAR value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#else
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		UCHAR DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
		UCHAR Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		UCHAR TxPowerEnable:1;	/* Enable */
	} field;
	UCHAR value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		UCHAR	Byte1;	/* High Byte */
		UCHAR	Byte0;	/* Low Byte */
	} field;
	
	USHORT		word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#else
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		UCHAR	Byte0;	/* Low Byte */
		UCHAR	Byte1;	/* High Byte */
	} field;

	USHORT		word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#endif /* RT_BIG_ENDIAN */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 
/* The Tx power tuning entry */
typedef struct _TX_POWER_TUNING_ENTRY_STRUCT {
	CHAR	RF_TX_ALC; 		/* 3390: RF R12[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC */
	CHAR 	MAC_PowerDelta;	/* Tx power control over MAC 0x1314~0x1324 */
} TX_POWER_TUNING_ENTRY_STRUCT, *PTX_POWER_TUNING_ENTRY_STRUCT;
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

/*
	2860: 28xx
	2870: 28xx

	30xx:
		3090
		3070
		2070 3070

	33xx:	30xx
		3390 3090
		3370 3070

	35xx:	30xx
		3572, 2870, 28xx
		3062, 2860, 28xx
		3562, 2860, 28xx

	3593, 28xx, 30xx, 35xx

	< Note: 3050, 3052, 3350 can not be compiled simultaneously. >
	305x:
		3052
		3050
		3350, 3050

	3352: 305x

	2880: 28xx
	2883:
	3883:
*/

struct _RTMP_CHIP_CAP_ {
	/* register */
	REG_PAIR *pRFRegTable;
	REG_PAIR *pBBPRegTable;
	UCHAR bbpRegTbSize;

	UINT32 MaxNumOfRfId;
	UINT32 MaxNumOfBbpId;

#define RF_REG_WT_METHOD_NONE			0
#define RF_REG_WT_METHOD_STEP_ON		1
	UCHAR RfReg17WtMethod;

	/* beacon */
	BOOLEAN FlgIsSupSpecBcnBuf;	/* SPECIFIC_BCN_BUF_SUPPORT */
	UINT8 BcnMaxNum;	/* software use */
	UINT8 BcnMaxHwNum;	/* hardware limitation */
	UINT8 WcidHwRsvNum;	/* hardware available WCID number */
	UINT16 BcnMaxHwSize;	/* hardware maximum beacon size */
	UINT16 BcnBase[HW_BEACON_MAX_NUM];	/* hardware beacon base address */

	/* function */
	/* use UINT8, not bit-or to speed up driver */
	BOOLEAN FlgIsHwWapiSup;

	/* VCO calibration mode */
	UINT8	VcoPeriod; /* default 10s */
#define VCO_CAL_DISABLE		0	/* not support */
#define VCO_CAL_MODE_1		1	/* toggle RF7[0] */
#define VCO_CAL_MODE_2		2	/* toggle RF3[7] */
#define VCO_CAL_MODE_3		3	/* toggle RF4[7] */	

	UINT8	FlgIsVcoReCalMode;

	BOOLEAN FlgIsHwAntennaDiversitySup;
#ifdef STREAM_MODE_SUPPORT
	BOOLEAN FlgHwStreamMode;
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	BOOLEAN FlgHwTxBfCap;
#endif /* TXBF_SUPPORT */
#ifdef FIFO_EXT_SUPPORT
	BOOLEAN FlgHwFifoExtCap;
#endif /* FIFO_EXT_SUPPORT */

#ifdef RTMP_MAC_PCI
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN HW_PCIE_PS_SUPPORT;
	BOOLEAN HW_PCIE_PS_L3_ENABLE;
#endif /* CONFIG_STA_SUPPORT */
#endif /* RTMP_MAC_PCI */


#ifdef TXRX_SW_ANTDIV_SUPPORT
	BOOLEAN bTxRxSwAntDiv;
#endif /* TXRX_SW_ANTDIV_SUPPORT */

	/* ---------------------------- signal ---------------------------------- */
#define SNR_FORMULA1		0	/* ((0xeb     - pAd->StaCfg.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1	/* (pAd->StaCfg.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2	/* (pAd->StaCfg.LastSNR0) * 3) / 16; */
	UINT8 SnrFormula;

	UINT8 MaxNss;			/* maximum Nss, 3 for 3883 or 3593 */

	BOOLEAN bTempCompTxALC;

	BOOLEAN bLimitPowerRange; /* TSSI compensation range limit */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	UINT8 TxAlcTxPowerUpperBound_2G;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable_2G;
#ifdef RT305x
#if defined(RT3352) || defined(RT5350)
	UINT8 TxPowerMaxCompenStep;
	UINT8 TxPowerTableMaxIdx;
#endif /* defined(RT3352) || defined(RT5350) */
#endif /* RT305x */
#ifdef A_BAND_SUPPORT
	UINT8 TxAlcTxPowerUpperBound_5G;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable_5G;
#endif /* A_BAND_SUPPORT */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

	/* ---------------------------- packet ---------------------------------- */
	UINT8 TXWISize;
	UINT8 RXWISize;

	/* ---------------------------- others ---------------------------------- */
#ifdef RTMP_EFUSE_SUPPORT
	UINT16 EFUSE_USAGE_MAP_START;
	UINT16 EFUSE_USAGE_MAP_END;
	UINT8 EFUSE_USAGE_MAP_SIZE;
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef RTMP_FLASH_SUPPORT
	UCHAR *eebuf;
	BOOLEAN ee_inited;
#endif /* RTMP_FLASH_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	UCHAR carrier_func;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef DFS_SUPPORT
	UINT8 DfsEngineNum;
#endif /* DFS_SUPPORT */

	/*
		Define the burst size of WPDMA of PCI
		0 : 4 DWORD (16bytes)
		1 : 8 DWORD (32 bytes)
		2 : 16 DWORD (64 bytes)
		3 : 32 DWORD (128 bytes)
	*/
	UINT8 WPDMABurstSIZE;

	/* 
 	 * 0: MBSSID_MODE0 
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit2:bit0] of byte5) 
 	 * 1: MBSSID_MODE1
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit4:bit2] of byte0) 
 	 */
	UINT8 MBSSIDMode;


#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

#ifdef MCS_LUT_SUPPORT
	BOOLEAN bFlgHwTxLuCap;
#endif /* MCS_LUT_SUPPORT */
	INT16 DPDCalPassThres;
	INT16 DPDCalPassLowThresTX0;
	INT16 DPDCalPassLowThresTX1;
	INT16 DPDCalPassHighThresTX0;
	INT16 DPDCalPassHighThresTX1;
#ifdef HW_CALIBRATION_SUPPORT
	BOOLEAN FlgRCalCap;
	BOOLEAN FlgBwFilterCalCap;
	BOOLEAN FlgDPDCalCap;
	BOOLEAN FlgLoftTxIQCalCap;
#endif /* RF_CALIBRATION_SUPPORT */
};

typedef VOID (*CHIP_SPEC_FUNC)(VOID *Adaptor, VOID *pData, ULONG	Data);

/* The chip specific function ID */
typedef enum _CHIP_SPEC_ID
{		
#ifdef RT305x
	RT305x_WLAN_MODE_CHANGE,
	RT305x_INITIALIZATION,
	RT305x_HT_MODE_CHANGE,	
#endif /* RT305x */
	CHIP_SPEC_RESV_FUNC
} CHIP_SPEC_ID;

#define CHIP_SPEC_ID_NUM 	CHIP_SPEC_RESV_FUNC

struct _RTMP_CHIP_OP_ {
	/*  Calibration access related callback functions */
	int (*eeinit)(struct _RTMP_ADAPTER *pAd);
	int (*eeread)(struct _RTMP_ADAPTER *pAd, USHORT offset, PUSHORT pValue);
	int (*eewrite)(struct _RTMP_ADAPTER *pAd, USHORT offset, USHORT value);

	/* MCU related callback functions */
	int (*loadFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*eraseFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*sendCommandToMcu)(struct _RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1, BOOLEAN FlgIsNeedLocked);	/* int (*sendCommandToMcu)(RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1); */

	void (*AsicRfInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicBbpInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicMacInit)(struct _RTMP_ADAPTER *pAd);

	void (*AsicRfTurnOn)(struct _RTMP_ADAPTER *pAd);
	void (*AsicRfTurnOff)(struct _RTMP_ADAPTER *pAd);
	void (*AsicReverseRfFromSleepMode)(struct _RTMP_ADAPTER *pAd, BOOLEAN FlgIsInitState);
	void (*AsicHaltAction)(struct _RTMP_ADAPTER *pAd);

	/* Power save */
	VOID (*EnableAPMIMOPS)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*DisableAPMIMOPS)(IN struct _RTMP_ADAPTER *pAd);

	/* Chip tuning */
	VOID (*RxSensitivityTuning)(IN struct _RTMP_ADAPTER *pAd);

	/* MAC */
	VOID (*BeaconUpdate)(
				IN struct _RTMP_ADAPTER *pAd,
				IN USHORT		Offset,
				IN UINT32		Value,
				IN UINT8		Unit);

	/* BBP adjust */
	VOID (*ChipBBPAdjust)(IN struct _RTMP_ADAPTER *pAd);

	/* AGC */
	VOID (*ChipAGCInit)(
				IN struct _RTMP_ADAPTER *pAd,
				IN UCHAR				BandWidth);
	UCHAR (*ChipAGCAdjust)(
				IN struct _RTMP_ADAPTER *pAd,
				IN CHAR					Rssi,
				IN UCHAR				OrigR66Value);
	
	/* Channel */
	VOID (*ChipSwitchChannel)(
				IN struct _RTMP_ADAPTER *pAd,
				IN UCHAR				Channel,
				IN BOOLEAN				bScan);

	/* IQ Calibration */
	VOID (*ChipIQCalibration)(
				IN struct _RTMP_ADAPTER *pAd,
				IN UCHAR 				Channel);

	UINT32 (*ChipGetCurrentTemp)(
				IN struct _RTMP_ADAPTER *pAd);
		
#ifdef THERMAL_PROTECT_SUPPORT
	VOID (*ThermalProDefaultCond)(
				IN struct _RTMP_ADAPTER *pAd);

	VOID (*ThermalPro1stCond)(
				IN struct _RTMP_ADAPTER *pAd);

	VOID (*ThermalPro2ndCond)(
				IN struct _RTMP_ADAPTER *pAd);
#endif /* THERMAL_PROTECT_SUPPORT */

	/* TX ALC */
	UINT32 (*TSSIRatio)(IN INT32 delta_power);

	VOID (*InitDesiredTSSITable)(IN struct _RTMP_ADAPTER *pAd);

	int (*ATETssiCalibration)(
				IN struct _RTMP_ADAPTER 	*pAd,
				IN PSTRING				arg);

	int (*ATETssiCalibrationExtend)(
				IN struct _RTMP_ADAPTER 	*pAd,
				IN PSTRING				arg);
	
	VOID (*AsicTxAlcGetAutoAgcOffset)(
				IN struct _RTMP_ADAPTER	*pAd,
				IN PCHAR				pDeltaPwr,
				IN PCHAR				pTotalDeltaPwr,
				IN PCHAR				pAgcCompensate,
				IN PCHAR 				pDeltaPowerByBbpR1);

	int (*ATEReadExternalTSSI)(
				IN struct _RTMP_ADAPTER 	*pAd,
				IN PSTRING				arg);

	VOID (*AsicGetTxPowerOffset)(
				IN struct _RTMP_ADAPTER	*pAd,
				IN PULONG 				TxPwr);

	VOID (*AsicExtraPowerOverMAC)(
				IN struct _RTMP_ADAPTER	*pAd);

	VOID (*AsicAdjustTxPower)(
				IN struct _RTMP_ADAPTER	*pAd);

	/* Antenna */
	VOID (*AsicAntennaDefaultReset)(
				IN struct _RTMP_ADAPTER	*pAd,
				IN EEPROM_ANTENNA_STRUC	*pAntenna);

	VOID (*SetRxAnt)(
				IN struct _RTMP_ADAPTER	*pAd,
				IN UCHAR			Ant);

	/* EEPROM */
	VOID (*NICInitAsicFromEEPROM)(IN struct _RTMP_ADAPTER *pAd);

	/* high power tuning */
	VOID (*HighPowerTuning)(
				IN struct _RTMP_ADAPTER	*pAd,
				IN struct _RSSI_SAMPLE	*pRssi);
	
	/* Others */
	VOID (*NetDevNickNameInit)(IN struct _RTMP_ADAPTER *pAd);

	/* The chip specific function list */
	CHIP_SPEC_FUNC ChipSpecFunc[CHIP_SPEC_ID_NUM];
	
	VOID (*AsicResetBbpAgent)(IN struct _RTMP_ADAPTER *pAd);

#ifdef CARRIER_DETECTION_SUPPORT
	VOID (*ToneRadarProgram)(
				IN struct _RTMP_ADAPTER *pAd,
				IN ULONG  threshold);
#endif /* CARRIER_DETECTION_SUPPORT */
	VOID (*CckMrcStatusCtrl)(
				IN struct _RTMP_ADAPTER *pAd);
	VOID (*RadarGLRTCompensate)(
				IN struct _RTMP_ADAPTER *pAd);

#ifdef MICROWAVE_OVEN_SUPPORT
	VOID (*AsicMeasureFalseCCA)(
				IN struct _RTMP_ADAPTER *pAd);
	VOID (*AsicMitigateMicrowave)(
				IN struct _RTMP_ADAPTER *pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */
};

#define RTMP_CHIP_ENABLE_AP_MIMOPS(__pAd)				\
		if (__pAd->chipOps.EnableAPMIMOPS != NULL)							\
			__pAd->chipOps.EnableAPMIMOPS(__pAd)

#define RTMP_CHIP_DISABLE_AP_MIMOPS(__pAd)									\
		if (__pAd->chipOps.DisableAPMIMOPS != NULL)							\
			__pAd->chipOps.DisableAPMIMOPS(__pAd)

#define RTMP_CHIP_RX_SENSITIVITY_TUNING(__pAd)								\
		if (__pAd->chipOps.RxSensitivityTuning != NULL)						\
			__pAd->chipOps.RxSensitivityTuning(__pAd)

#define RTMP_CHIP_ASIC_BBP_ADJUST(__pAd)									\
		if (__pAd->chipOps.ChipBBPAdjust != NULL)							\
			__pAd->chipOps.ChipBBPAdjust(__pAd)

#define RTMP_CHIP_ASIC_AGC_ADJUST(__pAd, __Rssi, __R66)					\
		if (__pAd->chipOps.ChipAGCAdjust != NULL)						\
			__R66 = __pAd->chipOps.ChipAGCAdjust(__pAd, __Rssi, __R66)

#define RTMP_CHIP_ASIC_AGC_INIT(__pAd, __Bandwidth)				\
		if (__pAd->chipOps.ChipAGCInit != NULL)						\
			__pAd->chipOps.ChipAGCInit(__pAd, __Bandwidth)

#define RTMP_CHIP_ASIC_SWITCH_CHANNEL(__pAd, __Channel, __bScan)			\
		if (__pAd->chipOps.ChipSwitchChannel != NULL)						\
			__pAd->chipOps.ChipSwitchChannel(__pAd, __Channel, __bScan);	\
		else																\
			DBGPRINT(RT_DEBUG_ERROR, ("No switch channel function!!!\n"))

#define RTMP_CHIP_ASIC_TSSI_TABLE_INIT(__pAd)								\
		if (__pAd->chipOps.InitDesiredTSSITable != NULL)					\
			__pAd->chipOps.InitDesiredTSSITable(__pAd)

#define RTMP_CHIP_ATE_TSSI_CALIBRATION(__pAd, __pData)					\
		if (__pAd->chipOps.ATETssiCalibration != NULL)					\
			__pAd->chipOps.ATETssiCalibration(__pAd, __pData)

#define RTMP_CHIP_ATE_TSSI_CALIBRATION_EXTEND(__pAd, __pData)			\
		if (__pAd->chipOps.ATETssiCalibrationExtend != NULL)				\
			__pAd->chipOps.ATETssiCalibrationExtend(__pAd, __pData)	

#define RTMP_CHIP_ATE_READ_EXTERNAL_TSSI(__pAd, __pData)					\
		if (__pAd->chipOps.ATEReadExternalTSSI != NULL)					\
			__pAd->chipOps.ATEReadExternalTSSI(__pAd, __pData)	

#define RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(__pAd, __pCfgOfTxPwrCtrlOverMAC)					\
		if (__pAd->chipOps.AsicGetTxPowerOffset != NULL)					\
			__pAd->chipOps.AsicGetTxPowerOffset(__pAd, __pCfgOfTxPwrCtrlOverMAC)	
		
#define RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(									\
		__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1)	\
		if (__pAd->chipOps.AsicTxAlcGetAutoAgcOffset != NULL)				\
			__pAd->chipOps.AsicTxAlcGetAutoAgcOffset(						\
		__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1)

#define RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(__pAd)					\
		if (__pAd->chipOps.AsicExtraPowerOverMAC != NULL)					\
			__pAd->chipOps.AsicExtraPowerOverMAC(__pAd)

#define RTMP_CHIP_ASIC_ADJUST_TX_POWER(__pAd)					\
				if (__pAd->chipOps.AsicAdjustTxPower != NULL)					\
					__pAd->chipOps.AsicAdjustTxPower(__pAd)

#define RTMP_CHIP_ASIC_GET_TSSI_RATIO(__pAd, __DeltaPwr)					\
			__pAd->chipOps.TSSIRatio(__DeltaPwr)

#define RTMP_CHIP_GET_CURRENT_TEMP(__pAd)					\
		if (__pAd->chipOps.ChipGetCurrentTemp != NULL)					\
			__pAd->chipOps.ChipGetCurrentTemp(__pAd)
			
#ifdef THERMAL_PROTECT_SUPPORT
#define RTMP_CHIP_THERMAL_PRO_DEFAULT_COND(__pAd)					\
		if (__pAd->chipOps.ThermalProDefaultCond != NULL)					\
			__pAd->chipOps.ThermalProDefaultCond(__pAd)

#define RTMP_CHIP_THERMAL_PRO_1st_COND(__pAd)					\
		if (__pAd->chipOps.ThermalPro1stCond != NULL)					\
			__pAd->chipOps.ThermalPro1stCond(__pAd)

#define RTMP_CHIP_THERMAL_PRO_2nd_COND(__pAd)					\
		if (__pAd->chipOps.ThermalPro2ndCond != NULL)					\
			__pAd->chipOps.ThermalPro2ndCond(__pAd)
#endif /* THERMAL_PROTECT_SUPPORT */
			
#define RTMP_CHIP_ASIC_FREQ_CAL_STOP(__pAd)									\
		if (__pAd->chipOps.AsicFreqCalStop != NULL)							\
			__pAd->chipOps.AsicFreqCalStop(__pAd)

#define RTMP_CHIP_IQ_CAL(__pAd, __pChannel)										\
		if (__pAd->chipOps.ChipIQCalibration != NULL)								\
			 __pAd->chipOps.ChipIQCalibration(__pAd, __pChannel)

#define RTMP_CHIP_HIGH_POWER_TUNING(__pAd, __pRssi)							\
		if (__pAd->chipOps.HighPowerTuning != NULL)							\
			__pAd->chipOps.HighPowerTuning(__pAd, __pRssi)

#define RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(__pAd, __pAntenna)				\
		if (__pAd->chipOps.AsicAntennaDefaultReset != NULL)					\
			__pAd->chipOps.AsicAntennaDefaultReset(__pAd, __pAntenna)

#define RTMP_NET_DEV_NICKNAME_INIT(__pAd)									\
		if (__pAd->chipOps.NetDevNickNameInit != NULL)						\
			__pAd->chipOps.NetDevNickNameInit(__pAd)

#define RTMP_EEPROM_ASIC_INIT(__pAd)										\
		if (__pAd->chipOps.NICInitAsicFromEEPROM != NULL)					\
			__pAd->chipOps.NICInitAsicFromEEPROM(__pAd)

#define RTMP_CHIP_SPECIFIC(__pAd, __FuncId, __pData, __Data)				\
		if ((__FuncId >= 0) && (__FuncId < CHIP_SPEC_RESV_FUNC))				\
		{																	\
			if (__pAd->chipOps.ChipSpecFunc[__FuncId] != NULL)					\
				__pAd->chipOps.ChipSpecFunc[__FuncId](__pAd, __pData, __Data);	\
		}

#define RTMP_CHIP_ASIC_RESET_BBP_AGENT(									\
		__pAd)	\
		if (__pAd->chipOps.AsicResetBbpAgent != NULL)				\
			__pAd->chipOps.AsicResetBbpAgent(						\
		__pAd)
#define RTMP_CHIP_UPDATE_BEACON(__pAd, Offset, Value, Unit)		\
		if (__pAd->chipOps.BeaconUpdate != NULL)					\
			__pAd->chipOps.BeaconUpdate(__pAd, Offset, Value, Unit)
#ifdef CARRIER_DETECTION_SUPPORT
#define	RTMP_CHIP_CARRIER_PROGRAM(__pAd, threshold)							\
		if(__pAd->chipOps.ToneRadarProgram != NULL)					\
			__pAd->chipOps.ToneRadarProgram(__pAd, threshold)
#endif /* CARRIER_DETECTION_SUPPORT */
#define RTMP_CHIP_CCK_MRC_STATUS_CTRL(__pAd)						\
		if(__pAd->chipOps.CckMrcStatusCtrl != NULL)				\
			__pAd->chipOps.CckMrcStatusCtrl(__pAd)
#define RTMP_CHIP_RADAR_GLRT_COMPENSATE(__pAd)						\
					if(__pAd->chipOps.RadarGLRTCompensate != NULL)				\
						__pAd->chipOps.RadarGLRTCompensate(__pAd)


/* function prototype */
VOID RtmpChipOpsHook(
	IN VOID *pCB);

VOID RtmpChipBcnInit(
	IN struct _RTMP_ADAPTER *pAd);

VOID RtmpChipBcnSpecInit(
	IN struct _RTMP_ADAPTER *pAd);

VOID RtmpChipWriteHighMemory(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit);

VOID RtmpChipWriteMemory(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit);

VOID RTMPReadChannelPwr(
	IN struct _RTMP_ADAPTER		*pAd);


VOID NetDevNickNameInit(IN struct _RTMP_ADAPTER *pAd);

#ifdef GREENAP_SUPPORT
VOID EnableAPMIMOPSv2(
	IN struct _RTMP_ADAPTER		*pAd);

VOID DisableAPMIMOPSv2(
	IN struct _RTMP_ADAPTER		*pAd);

VOID EnableAPMIMOPSv1(
	IN struct _RTMP_ADAPTER		*pAd);

VOID DisableAPMIMOPSv1(
	IN struct _RTMP_ADAPTER		*pAd);
#endif /* GREENAP_SUPPORT */


/* global variable */
extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];
extern UCHAR NUM_OF_3020_CHNL;
extern FREQUENCY_ITEM *FreqItems3020;
extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

#endif /* __RTMP_CHIP_H__ */
