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

#include "mac_ral/pbf.h"

#include "eeprom.h"

#ifdef RTMP_MAC_PCI
#include "iface/rtmp_reg_pcirbs.h"
#include "mac_ral/rtmp_mac.h"
#include "mac_ral/mac_pci.h"
#endif /* RTMP_MAC_PCI */



















#ifdef RT3290
#include "chip/rt3290.h"
#endif /* RT3290 */

#ifdef RT65xx
#include "chip/mt76x0.h"
#endif /* RT65xx */

#ifdef MT76x0
#include "chip/mt76x0.h"
#endif

#ifdef MT76x2
#include "chip/mt76x2.h"
#endif


#include "mcu/mcu.h"

#define IS_RT3090A(_pAd)    ((((_pAd)->MACVersion & 0xffff0000) == 0x30900000))

/* We will have a cost down version which mac version is 0x3090xxxx */
#define IS_RT3090(_pAd)     ((((_pAd)->MACVersion & 0xffff0000) == 0x30710000) || (IS_RT3090A(_pAd)))

#define IS_RT3070(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30700000)
#define IS_RT3071(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30710000)
#define IS_RT2070(_pAd)		(((_pAd)->RfIcType == RFIC_2020) || ((_pAd)->EFuseTag == 0x27))

#define IS_RT2860(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x28600000)
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

#define IS_RT2883(_pAd)		(0)

#define IS_RT3883(_pAd)		(0)

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
#define REV_RT5592C			0x0221

#define IS_RT65XX(_pAd)		((((_pAd)->MACVersion & 0xFFFF0000) == 0x65900000) ||\
							 (((_pAd)->MACVersion & 0xfffff000) == 0x85592000) ||\
							 (((_pAd)->MACVersion & 0xffff0000) == 0x76500000) ||\
							 (((_pAd)->MACVersion & 0xffff0000) == 0x76620000))

#define IS_MT76x0(_pAd)		((((_pAd)->MACVersion & 0xffff0000) == 0x65900000) ||\
							 (((_pAd)->MACVersion & 0xffff0000) == 0x76500000))
#define IS_MT7650(_pAd)		(((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76500000)
#define IS_MT7650E(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76500000) & (IS_PCIE_INF(_pAd)))
#define IS_MT7650U(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76500000) & (IS_USB_INF(_pAd)))
#define IS_MT7630(_pAd)		(((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76300000)
#define IS_MT7630E(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76300000) & (IS_PCIE_INF(_pAd)))
#define IS_MT7630U(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76300000) & (IS_USB_INF(_pAd)))
#define IS_MT7610(_pAd)		(((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76100000)
#define IS_MT7610E(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76100000) & (IS_PCIE_INF(_pAd)))
#define IS_MT7610U(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76100000) & (IS_USB_INF(_pAd)))

#define IS_MT76x2(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x76620000)
#define IS_MT7662(_pAd)		(((_pAd)->chipCap.ChipID && 0xffff0000) == 0x76620000)
#define IS_MT7662E(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76620000) & (IS_PCIE_INF(_pAd)))
#define IS_MT7662U(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76620000) & (IS_USB_INF(_pAd)))
#define IS_MT7612(_pAd)		(((_pAd)->chipCap.ChipID && 0xffff0000) == 0x76120000)
#define IS_MT7612E(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76120000) & (IS_PCIE_INF(_pAd)))
#define IS_MT7612U(_pAd)	((((_pAd)->chipCap.ChipID & 0xffff0000) == 0x76120000) & (IS_USB_INF(_pAd)))

#define IS_MT76xx(_pAd)		(IS_MT76x0(_pAd) || IS_MT76x2(_pAd))

#define IS_RT8592(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x85590000)

#define IS_RT8592(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x85590000)


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

#define CCA_AVG_MAX_COUNT	5

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


#define EEPROM_COUNTRY_REGION			0x38

#define EEPROM_DEFINE_MAX_TXPWR			0x4e

#define EEPROM_FREQ_OFFSET			0x3a
#define EEPROM_LEDAG_CONF_OFFSET	0x3c
#define EEPROM_LEDACT_CONF_OFFSET	0x3e
#define EEPROM_LED_POLARITY_OFFSET	0x40
#if defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592)
#define	EEPROM_NIC3_OFFSET			0x42
#endif /* defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) */

#define EEPROM_LNA_OFFSET			0x44

#define EEPROM_RSSI_BG_OFFSET			0x46
#define EEPROM_RSSI_A_OFFSET			0x4a
#define EEPROM_TXMIXER_GAIN_2_4G		0x48
#define EEPROM_TXMIXER_GAIN_5G			0x4c

#define EEPROM_TXPOWER_DELTA			0x50	/* 20MHZ AND 40 MHZ use different power. This is delta in 40MHZ. */

#define EEPROM_G_TX_PWR_OFFSET			0x52
#define EEPROM_G_TX2_PWR_OFFSET			0x60

#define EEPROM_G_TSSI_BOUND1			0x6e
#define EEPROM_G_TSSI_BOUND2			0x70
#define EEPROM_G_TSSI_BOUND3			0x72
#define EEPROM_G_TSSI_BOUND4			0x74
#define EEPROM_G_TSSI_BOUND5			0x76

#define EEPROM_A_TX_PWR_OFFSET      		0x78
#define EEPROM_A_TX2_PWR_OFFSET			0xa6

#define MBSSID_MODE0 0
#define MBSSID_MODE1 1
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

#ifdef IQ_CAL_SUPPORT
/* IQ Calibration */
enum IQ_CAL_CHANNEL_INDEX {
	IQ_CAL_2G,
#ifdef A_BAND_SUPPORT
	IQ_CAL_GROUP1_5G, /* Ch36~Ch64 */
	IQ_CAL_GROUP2_5G, /* Ch100~Ch138 */
	IQ_CAL_GROUP3_5G, /* Ch140~Ch165 */
#endif /* A_BAND_SUPPORT */
	IQ_CAL_CHANNEL_GROUP_NUM,
};

enum IQ_CAL_TXRX_CHAIN {
	IQ_CAL_TX0,
	IQ_CAL_TX1,
	IQ_CAL_CHAIN_NUM,
};

enum IQ_CAL_TYPE {
	IQ_CAL_GAIN,
	IQ_CAL_PHASE,
	IQ_CAL_TYPE_NUM,
};

#define EEPROM_IQ_GLOBAL_BBP_ACCESS_BASE					0xF0
#define EEPROM_IQ_GAIN_CAL_TX0_2G							0x130
#define EEPROM_IQ_PHASE_CAL_TX0_2G							0x131
#define EEPROM_IQ_GROUPDELAY_CAL_TX0_2G						0x132
#define EEPROM_IQ_GAIN_CAL_TX1_2G							0x133
#define EEPROM_IQ_PHASE_CAL_TX1_2G							0x134
#define EEPROM_IQ_GROUPDELAY_CAL_TX1_2G						0x135
#define EEPROM_IQ_GAIN_CAL_RX0_2G							0x136
#define EEPROM_IQ_PHASE_CAL_RX0_2G							0x137
#define EEPROM_IQ_GROUPDELAY_CAL_RX0_2G					0x138
#define EEPROM_IQ_GAIN_CAL_RX1_2G							0x139
#define EEPROM_IQ_PHASE_CAL_RX1_2G							0x13A
#define EEPROM_IQ_GROUPDELAY_CAL_RX1_2G					0x13B
#define EEPROM_RF_IQ_COMPENSATION_CONTROL 					0x13C
#define EEPROM_RF_IQ_IMBALANCE_COMPENSATION_CONTROL 		0x13D
#define EEPROM_IQ_GAIN_CAL_TX0_CH36_TO_CH64_5G				0x144
#define EEPROM_IQ_PHASE_CAL_TX0_CH36_TO_CH64_5G			0x145
#define EEPROM_IQ_GAIN_CAL_TX0_CH100_TO_CH138_5G			0X146
#define EEPROM_IQ_PHASE_CAL_TX0_CH100_TO_CH138_5G			0x147
#define EEPROM_IQ_GAIN_CAL_TX0_CH140_TO_CH165_5G			0x148
#define EEPROM_IQ_PHASE_CAL_TX0_CH140_TO_CH165_5G			0x149
#define EEPROM_IQ_GAIN_CAL_TX1_CH36_TO_CH64_5G				0x14A
#define EEPROM_IQ_PHASE_CAL_TX1_CH36_TO_CH64_5G			0x14B
#define EEPROM_IQ_GAIN_CAL_TX1_CH100_TO_CH138_5G			0X14C
#define EEPROM_IQ_PHASE_CAL_TX1_CH100_TO_CH138_5G			0x14D
#define EEPROM_IQ_GAIN_CAL_TX1_CH140_TO_CH165_5G			0x14E
#define EEPROM_IQ_PHASE_CAL_TX1_CH140_TO_CH165_5G			0x14F
#define EEPROM_IQ_GROUPDELAY_CAL_TX0_CH36_TO_CH64_5G		0x150
#define EEPROM_IQ_GROUPDELAY_CAL_TX1_CH36_TO_CH64_5G		0x151
#define EEPROM_IQ_GROUPDELAY_CAL_TX0_CH100_TO_CH138_5G	0x152
#define EEPROM_IQ_GROUPDELAY_CAL_TX1_CH100_TO_CH138_5G	0x153
#define EEPROM_IQ_GROUPDELAY_CAL_TX0_CH140_TO_CH165_5G	0x154
#define EEPROM_IQ_GROUPDELAY_CAL_TX1_CH140_TO_CH165_5G	0x155
#define EEPROM_IQ_GAIN_CAL_RX0_CH36_TO_CH64_5G				0x156
#define EEPROM_IQ_PHASE_CAL_RX0_CH36_TO_CH64_5G			0x157
#define EEPROM_IQ_GAIN_CAL_RX0_CH100_TO_CH138_5G			0X158
#define EEPROM_IQ_PHASE_CAL_RX0_CH100_TO_CH138_5G			0x159
#define EEPROM_IQ_GAIN_CAL_RX0_CH140_TO_CH165_5G			0x15A
#define EEPROM_IQ_PHASE_CAL_RX0_CH140_TO_CH165_5G			0x15B
#define EEPROM_IQ_GAIN_CAL_RX1_CH36_TO_CH64_5G				0x15C
#define EEPROM_IQ_PHASE_CAL_RX1_CH36_TO_CH64_5G			0x15D
#define EEPROM_IQ_GAIN_CAL_RX1_CH100_TO_CH138_5G			0X15E
#define EEPROM_IQ_PHASE_CAL_RX1_CH100_TO_CH138_5G			0x15F
#define EEPROM_IQ_GAIN_CAL_RX1_CH140_TO_CH165_5G			0x160
#define EEPROM_IQ_PHASE_CAL_RX1_CH140_TO_CH165_5G			0x161
#define EEPROM_IQ_GROUPDELAY_CAL_RX0_CH36_TO_CH64_5G		0x162
#define EEPROM_IQ_GROUPDELAY_CAL_RX1_CH36_TO_CH64_5G		0x163
#define EEPROM_IQ_GROUPDELAY_CAL_RX0_CH100_TO_CH138_5G	0x164
#define EEPROM_IQ_GROUPDELAY_CAL_RX1_CH100_TO_CH138_5G	0x165
#define EEPROM_IQ_GROUPDELAY_CAL_RX0_CH140_TO_CH165_5G	0x166
#define EEPROM_IQ_GROUPDELAY_CAL_RX1_CH140_TO_CH165_5G	0x167
#endif /* IQ_CAL_SUPPORT */

#ifdef MT76x0
#define EEPROM_MT76x0_TEMPERATURE_OFFSET	0xd1 /* signed value */
#define EEPROM_MT76x0_A_BAND_MB			0xdc
#define EEPROM_MT76x0_A_BAND_HB			0xdd

#define EEPROM_MT76x0_2G_TARGET_POWER	0xd0
#define EEPROM_MT76x0_5G_TARGET_POWER	0xd2
#define EEPROM_MT76x0_2G_SLOPE_OFFSET	0x6E
#define EEPROM_MT76x0_5G_SLOPE_OFFSET	0xf0
#define EEPROM_MT76x0_5G_CHANNEL_BOUNDARY 0xd4
#endif /* MT76x0 */

#define EEPROM_A_TSSI_BOUND1		0xd4
#define EEPROM_A_TSSI_BOUND2		0xd6
#define EEPROM_A_TSSI_BOUND3		0xd8
#define EEPROM_A_TSSI_BOUND4		0xda
#define EEPROM_A_TSSI_BOUND5		0xdc

/* ITxBF calibration values EEPROM locations 0x1a0 to 0x1ab */
#define EEPROM_ITXBF_CAL				0x1a0

#define EEPROM_TXPOWER_BYRATE 			0xde	/* 20MHZ power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G	0xde	/* 20MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G	0xee	/* 40MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G		0xfa	/* 20MHZ 5G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G		0x10a	/* 40MHZ 5G tx power. */

#define EEPROM_BBP_BASE_OFFSET			0xf0	/* The address is from NIC config 0, not BBP register ID */

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


/*
  *   EEPROM operation related marcos
  */
#define RT28xx_EEPROM_READ16(_pAd, _offset, _value)			\
	(_pAd)->chipOps.eeread((RTMP_ADAPTER *)(_pAd), (USHORT)(_offset), (PUSHORT)&(_value))

#define RT28xx_EEPROM_WRITE16(_pAd, _offset, _value)		\
	(_pAd)->chipOps.eewrite((RTMP_ADAPTER *)(_pAd), (USHORT)(_offset), (USHORT)(_value))


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
	UINT32 ChipID;
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
#endif /* RTMP_MAC_PCI */

	enum ASIC_CAP asic_caps;
	enum PHY_CAP phy_caps;
	
#ifdef TXRX_SW_ANTDIV_SUPPORT
	BOOLEAN bTxRxSwAntDiv;
#endif /* TXRX_SW_ANTDIV_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
		BOOLEAN dynamic_vga_support;
		INT32 compensate_level;
		INT32 avg_rssi_0;
		INT32 avg_rssi_1;
		INT32 avg_rssi_all;
		UCHAR dynamic_chE_mode;
#endif


	/* ---------------------------- signal ---------------------------------- */
#define SNR_FORMULA1		0	/* ((0xeb     - pAd->StaCfg.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1	/* (pAd->StaCfg.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2	/* (pAd->StaCfg.LastSNR0) * 3) / 16; */
	UINT8 SnrFormula;

	UINT8 MaxNss;			/* maximum Nss, 3 for 3883 or 3593 */

	BOOLEAN bTempCompTxALC;
	BOOLEAN rx_temp_comp;

	BOOLEAN bLimitPowerRange; /* TSSI compensation range limit */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	UINT8 TxAlcTxPowerUpperBound_2G;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable_2G;
#ifdef A_BAND_SUPPORT
	UINT8 TxAlcTxPowerUpperBound_5G;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable_5G;
#endif /* A_BAND_SUPPORT */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2)
	INT16	PAModeCCK[4];
	INT16	PAModeOFDM[8];
	INT16	PAModeHT[16];
#ifdef DOT11_VHT_AC
	INT16	PAModeVHT[10];
#endif /* DOT11_VHT_AC */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2) */

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



#ifdef RT5592EP_SUPPORT
	UINT32 Priv; /* Flag for RT5592 EP */
#endif /* RT5592EP_SUPPORT */

#ifdef RT65xx
	UINT8 PAType; /* b'00: 2.4G+5G external PA, b'01: 5G external PA, b'10: 2.4G external PA, b'11: Internal PA */
#endif /* RT65xx */

#ifdef CONFIG_ANDES_SUPPORT
	UINT32 WlanMemmapOffset;
	UINT32 InbandPacketMaxLen;
	UINT8 CmdRspRxRing;
	BOOLEAN IsComboChip;
#endif

	
	enum MCU_TYPE MCUType;
	UCHAR *FWImageName;
	UCHAR *MACRegisterVer;
	UCHAR *BBPRegisterVer;
	UCHAR *RFRegisterVer;
	
#ifdef MT76x0
	BOOLEAN bDoTemperatureSensor;
	SHORT TemperatureOffset;			/* reference temperature(e2p[D1h]) */  
	SHORT LastTemperatureforVCO;
	SHORT LastTemperatureforCal;
	SHORT NowTemperature;
	UCHAR a_band_mid_ch;
	UCHAR a_band_high_ch;
	UCHAR ext_pa_current_setting;
	MT76x0_RATE_PWR_Table rate_pwr_table;
	UCHAR delta_tw_pwr_bw40_5G;
	UCHAR delta_tw_pwr_bw40_2G;
	UCHAR delta_tw_pwr_bw80;
	BOOLEAN bInternalTxALC; /* Internal Tx ALC */
	CHAR LastTempSensorState;
	BOOLEAN IsTempSensorStateReset;
#ifdef MT76x0_TSSI_CAL_COMPENSATION
	UCHAR tssi_info_1;
	UCHAR tssi_info_2;
	UCHAR tssi_info_3;
	UCHAR tssi_2G_target_power;
	UCHAR tssi_5G_target_power;	
	UCHAR efuse_2G_54M_tx_power;
	UCHAR efuse_5G_54M_tx_power;	
	CHAR tssi_slope_2G;
	CHAR tssi_offset_2G;
	UCHAR tssi_slope_5G[8];
	UCHAR tssi_offset_5G[8];
	UCHAR tssi_5G_channel_boundary[7];
	CHAR tssi_current_DC;
	INT tssi_pre_delta_pwr;
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
#endif /* MT76x0 */
};

typedef VOID (*CHIP_SPEC_FUNC)(VOID *pAd, VOID *pData, ULONG Data);

/* The chip specific function ID */
typedef enum _CHIP_SPEC_ID
{
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
#ifdef CONFIG_ANDES_SUPPORT
	int (*sendCommandToAndesMcu)(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, UCHAR cmd, UCHAR *pData, USHORT DataLen, BOOLEAN FlgIsNeedLocked);
#endif /* CONFIG_ANDES_SUPPORT */

	void (*AsicRfInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicBbpInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicMacInit)(struct _RTMP_ADAPTER *pAd);

	void (*AsicRfTurnOn)(struct _RTMP_ADAPTER *pAd);
	void (*AsicRfTurnOff)(struct _RTMP_ADAPTER *pAd);
	void (*AsicReverseRfFromSleepMode)(struct _RTMP_ADAPTER *pAd, BOOLEAN FlgIsInitState);
	void (*AsicHaltAction)(struct _RTMP_ADAPTER *pAd);

	/* Power save */
	VOID (*EnableAPMIMOPS)(IN struct _RTMP_ADAPTER *pAd, IN BOOLEAN ReduceCorePower);
	VOID (*DisableAPMIMOPS)(IN struct _RTMP_ADAPTER *pAd);

	/* Chip tuning */
	VOID (*RxSensitivityTuning)(IN struct _RTMP_ADAPTER *pAd);

	/* MAC */
	VOID (*BeaconUpdate)(struct _RTMP_ADAPTER *pAd, USHORT Offset, UINT32 Value, UINT8 Unit);

	/* BBP adjust */
	VOID (*ChipBBPAdjust)(IN struct _RTMP_ADAPTER *pAd);

	/* AGC */
	VOID (*ChipAGCInit)(struct _RTMP_ADAPTER *pAd, UCHAR bw);
	UCHAR (*ChipAGCAdjust)(struct _RTMP_ADAPTER *pAd, CHAR Rssi, UCHAR OrigR66Value);
	
	/* Channel */
	VOID (*ChipSwitchChannel)(struct _RTMP_ADAPTER *pAd, UCHAR ch, BOOLEAN bScan);

	/* EDCCA */
	VOID (*ChipSetEDCCA)(struct _RTMP_ADAPTER *pAd, BOOLEAN bOn);

#ifdef DYNAMIC_VGA_SUPPORT
	VOID (*AsicDynamicVgaGainControl)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*UpdateRssiForDynamicVga)(IN struct _RTMP_ADAPTER *pAd);
#endif /* DYNAMIC_VGA_SUPPORT */


	/* IQ Calibration */
	VOID (*ChipIQCalibration)(struct _RTMP_ADAPTER *pAd, UCHAR Channel);

	/* TX ALC */
	UINT32 (*TSSIRatio)(INT32 delta_power);
	VOID (*InitDesiredTSSITable)(IN struct _RTMP_ADAPTER *pAd);
	int (*ATETssiCalibration)(struct _RTMP_ADAPTER *pAd, PSTRING arg);
	int (*ATETssiCalibrationExtend)(struct _RTMP_ADAPTER *pAd, PSTRING arg);
	int (*ATEReadExternalTSSI)(struct _RTMP_ADAPTER *pAd, PSTRING arg);

	VOID (*AsicTxAlcGetAutoAgcOffset)(
				IN struct _RTMP_ADAPTER	*pAd,
				IN PCHAR				pDeltaPwr,
				IN PCHAR				pTotalDeltaPwr,
				IN PCHAR				pAgcCompensate,
				IN PCHAR 				pDeltaPowerByBbpR1);


	
	VOID (*AsicGetTxPowerOffset)(struct _RTMP_ADAPTER *pAd, ULONG *TxPwr);
	VOID (*AsicExtraPowerOverMAC)(struct _RTMP_ADAPTER *pAd);
	
	/* Antenna */
	VOID (*AsicAntennaDefaultReset)(struct _RTMP_ADAPTER *pAd, union _EEPROM_ANTENNA_STRUC *pAntenna);
	VOID (*SetRxAnt)(struct _RTMP_ADAPTER *pAd, UCHAR Ant);

	/* EEPROM */
	VOID (*NICInitAsicFromEEPROM)(IN struct _RTMP_ADAPTER *pAd);

	/* high power tuning */
	VOID (*HighPowerTuning)(struct _RTMP_ADAPTER *pAd, struct _RSSI_SAMPLE *pRssi);
	
	/* Others */
	VOID (*NetDevNickNameInit)(IN struct _RTMP_ADAPTER *pAd);

	/* The chip specific function list */
	CHIP_SPEC_FUNC ChipSpecFunc[CHIP_SPEC_ID_NUM];
	
	VOID (*AsicResetBbpAgent)(IN struct _RTMP_ADAPTER *pAd);

#ifdef CARRIER_DETECTION_SUPPORT
	VOID (*ToneRadarProgram)(struct _RTMP_ADAPTER *pAd, ULONG  threshold);
#endif /* CARRIER_DETECTION_SUPPORT */
	VOID (*CckMrcStatusCtrl)(struct _RTMP_ADAPTER *pAd);
	VOID (*RadarGLRTCompensate)(struct _RTMP_ADAPTER *pAd);
	
	INT (*Calibration)(struct _RTMP_ADAPTER *pAd, UINT32 CalibrationID, UINT32 Parameter);

	INT (*BurstWrite)(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Data, UINT32 Cnt);

	INT (*BurstRead)(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Cnt, UINT32 *Data);

	INT (*RandomRead)(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, UINT32 Num);

	INT (*RFRandomRead)(struct _RTMP_ADAPTER *pAd, BANK_RF_REG_PAIR *RegPair, UINT32 Num);

	INT (*ReadModifyWrite)(struct _RTMP_ADAPTER *pAd, R_M_W_REG *RegPair, UINT32 Num);

	INT (*RFReadModifyWrite)(struct _RTMP_ADAPTER *pAd, RF_R_M_W_REG *RegPair, UINT32 Num);

	INT (*RandomWrite)(struct _RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, UINT32 Num);

	INT (*RFRandomWrite)(struct _RTMP_ADAPTER *pAd, BANK_RF_REG_PAIR *RegPair, UINT32 Num);
};

#define RTMP_CHIP_ENABLE_AP_MIMOPS(__pAd, __ReduceCorePower)				\
		if (__pAd->chipOps.EnableAPMIMOPS != NULL)							\
			__pAd->chipOps.EnableAPMIMOPS(__pAd, __ReduceCorePower)

#define RTMP_CHIP_DISABLE_AP_MIMOPS(__pAd)									\
		if (__pAd->chipOps.DisableAPMIMOPS != NULL)							\
			__pAd->chipOps.DisableAPMIMOPS(__pAd)

#define RTMP_CHIP_RX_SENSITIVITY_TUNING(__pAd)								\
		if (__pAd->chipOps.RxSensitivityTuning != NULL)						\
			__pAd->chipOps.RxSensitivityTuning(__pAd)

#define RTMP_CHIP_ASIC_AGC_ADJUST(__pAd, __Rssi, __R66)					\
		if (__pAd->chipOps.ChipAGCAdjust != NULL)						\
			__R66 = __pAd->chipOps.ChipAGCAdjust(__pAd, __Rssi, __R66)

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

#define RTMP_CHIP_ASIC_GET_TSSI_RATIO(__pAd, __DeltaPwr)					\
			__pAd->chipOps.TSSIRatio(__DeltaPwr)

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

#define RTMP_CHIP_ASIC_SET_EDCCA(__pAd, __bOn)			\
do {	\
		if (__pAd->chipOps.ChipSetEDCCA != NULL)						\
			__pAd->chipOps.ChipSetEDCCA(__pAd, __bOn);	\
} while (0)

#define RTMP_CHIP_SPECIFIC(__pAd, __FuncId, __pData, __Data)				\
		if ((__FuncId >= 0) && (__FuncId < CHIP_SPEC_RESV_FUNC))				\
		{																	\
			if (__pAd->chipOps.ChipSpecFunc[__FuncId] != NULL)					\
				__pAd->chipOps.ChipSpecFunc[__FuncId](__pAd, __pData, __Data);	\
		}

#define RTMP_CHIP_ASIC_RESET_BBP_AGENT(__pAd)	\
		if (__pAd->chipOps.AsicResetBbpAgent != NULL)				\
			__pAd->chipOps.AsicResetBbpAgent(__pAd)

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


#define RTMP_CHIP_CALIBRATION(__pAd, __CalibrationID, __parameter) \
do {	\
	if(__pAd->chipOps.Calibration != NULL) \
		__pAd->chipOps.Calibration(__pAd, __CalibrationID, __parameter); \
} while (0)

#define BURST_WRITE(_pAd, _Offset, _pData, _Cnt)	\
do {												\
		if (_pAd->chipOps.BurstWrite != NULL)		\
			_pAd->chipOps.BurstWrite(_pAd, _Offset, _pData, _Cnt);\
} while (0)

#define BURST_READ(_pAd, _Offset, _Cnt, _pData)	\
do {											\
		if (_pAd->chipOps.BurstRead != NULL)	\
			_pAd->chipOps.BurstRead(_pAd, _Offset, _Cnt, _pData);	\
} while (0)

#define RANDOM_READ(_pAd, _RegPair, _Num)	\
do {										\
		if (_pAd->chipOps.RandomRead != NULL)	\
			_pAd->chipOps.RandomRead(_pAd, _RegPair, _Num);	\
} while (0)

#define RF_RANDOM_READ(_pAd, _RegPair, _Num)	\
do {											\
		if (_pAd->chipOps.RFRandomRead != NULL)	\
			_pAd->chipOps.RFRandomRead(_pAd, _RegPair, _Num); \
} while (0)

#define READ_MODIFY_WRITE(_pAd, _RegPair, _Num)	\
do {	\
		if (_pAd->chipOps.ReadModifyWrite != NULL)	\
			_pAd->chipOps.ReadModifyWrite(_pAd, _RegPair, _Num);	\
} while (0)

#define RF_READ_MODIFY_WRITE(_pAd, _RegPair, _Num)	\
do {	\
		if (_pAd->chipOps.RFReadModifyWrite != NULL)	\
			_pAd->chipOps.RFReadModifyWrite(_pAd, _RegPair, _Num);	\
} while (0)

#define RANDOM_WRITE(_pAd, _RegPair, _Num)	\
do {	\
		if (_pAd->chipOps.RandomWrite != NULL)	\
			_pAd->chipOps.RandomWrite(_pAd, _RegPair, _Num);	\
} while (0)

#define RF_RANDOM_WRITE(_pAd, _RegPair, _Num)	\
do {	\
		if (_pAd->chipOps.RFRandomWrite != NULL)	\
			_pAd->chipOps.RFRandomWrite(_pAd, _RegPair, _Num);	\
} while (0)

#ifdef DYNAMIC_VGA_SUPPORT
#define RTMP_ASIC_DYNAMIC_VGA_GAIN_CONTROL(_pAd)	\
do {	\
	if (_pAd->chipOps.AsicDynamicVgaGainControl != NULL)	\
		_pAd->chipOps.AsicDynamicVgaGainControl(_pAd);	\
} while (0)

#define RTMP_UPDATE_RSSI_FOR_DYNAMIC_VGA(_pAd)	\
	do {	\
		if (_pAd->chipOps.UpdateRssiForDynamicVga != NULL)	\
			_pAd->chipOps.UpdateRssiForDynamicVga(_pAd);	\
	} while (0)
#endif /* DYNAMIC_VGA_SUPPORT */


/* function prototype */
VOID RtmpChipOpsHook(VOID *pCB);

VOID RtmpChipBcnInit(struct _RTMP_ADAPTER *pAd);
VOID RtmpChipBcnSpecInit(struct _RTMP_ADAPTER *pAd);
#ifdef RLT_MAC
VOID rlt_bcn_buf_init(struct _RTMP_ADAPTER *pAd);
#endif /* RLT_MAC */

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

VOID RTMPReadChannelPwr(struct _RTMP_ADAPTER *pAd);
VOID RTMPReadTxPwrPerRate(struct _RTMP_ADAPTER *pAd);
VOID LoadTssiInfoFromEEPROM(struct _RTMP_ADAPTER *pAd);

#ifdef RTMP_TEMPERATURE_COMPENSATION
BOOLEAN LoadTempCompTableFromEEPROM(
		IN	struct _RTMP_ADAPTER	*pAd,
		IN	const USHORT		E2P_OFFSET_START,
		IN	const USHORT		E2P_OFFSET_END,
		OUT	PUCHAR			TssiTable,
		IN	const INT			StartIndex,
		IN	const UINT32		TABLE_SIZE);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#ifdef IQ_CAL_SUPPORT
VOID GetIQCalibration(struct _RTMP_ADAPTER *pAd);
VOID IQCalibration(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
VOID IQCalibrationViaBBPAccessSpace(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
UCHAR IQCal(
	IN enum IQ_CAL_CHANNEL_INDEX ChannelIndex,
	IN enum IQ_CAL_TXRX_CHAIN TxRxChain,
	IN enum IQ_CAL_TYPE IQCalType);
#endif /* IQ_CAL_SUPPORT */

VOID NetDevNickNameInit(IN struct _RTMP_ADAPTER *pAd);



#ifdef GREENAP_SUPPORT
VOID EnableAPMIMOPSv2(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower);
VOID DisableAPMIMOPSv2(struct _RTMP_ADAPTER *pAd);
VOID EnableAPMIMOPSv1(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower);
VOID DisableAPMIMOPSv1(struct _RTMP_ADAPTER *pAd);
#endif /* GREENAP_SUPPORT */


/* global variable */
extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];
extern UCHAR NUM_OF_3020_CHNL;
extern FREQUENCY_ITEM *FreqItems3020;
extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

BOOLEAN AsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us);
INT AsicSetPreTbttInt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT AsicReadAggCnt(struct _RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len);

VOID StopDmaTx(struct _RTMP_ADAPTER *pAd, UCHAR Level);
VOID StopDmaRx(struct _RTMP_ADAPTER *pAd, UCHAR Level);

#endif /* __RTMP_CHIP_H__ */
