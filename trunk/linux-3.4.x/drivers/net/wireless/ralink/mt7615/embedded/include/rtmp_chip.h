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
#include "eeprom.h"
#include "tx_power.h"

#include "hif/hif.h"

#include "mac/mac.h"
#include "mcu/mcu.h"


























#ifdef MT7615
#include "chip/mt7615.h"
#endif /* MT7615 */

#ifdef MT7622
#include "chip/mt7622.h"
#endif /* MT7622 */

#include "hw_ctrl/hw_init.h"

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
	(_pAd->PciHif.DeviceID == NIC3090_PCIe_DEVICE_ID) || \
	(_pAd->PciHif.DeviceID == NIC3091_PCIe_DEVICE_ID) || \
	(_pAd->PciHif.DeviceID == NIC3092_PCIe_DEVICE_ID) || \
	(_pAd->PciHif.DeviceID == NIC3592_PCIe_DEVICE_ID) || \
	((_pAd->PciHif.DeviceID == NIC3593_PCI_OR_PCIe_DEVICE_ID) && (RT3593OverPCIe(_pAd))) \
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
#define IS_RT5390(_pAd)   (((((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000) || IS_RT5390H(_pAd)) && ((_pAd)->infType != RTMP_DEV_INF_RBUS)) /* Include RT5390, RT5370 and RT5360 */

/* RT5390F */
#define IS_RT5390F(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0502))

/* RT5370G */
#define IS_RT5370G(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0503)) /* support HW PPAD ( the hardware rx antenna diversity ) */

/* RT5390R */
#define IS_RT5390R(_pAd)   ((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) == 0x1502)) /* support HW PPAD ( the hardware rx antenna diversity ) */

/* PCIe interface NIC */
#define IS_MINI_CARD(_pAd) ((_pAd)->Antenna.field.BoardType == BOARD_TYPE_MINI_CARD)

/* 5390U (5370 using PCIe interface) */
#define IS_RT5390U(_pAd)   (IS_MINI_CARD(_pAd) && (((_pAd)->MACVersion & 0xFFFF0000) == 0x53900000) && ((_pAd)->infType != RTMP_DEV_INF_RBUS))

/* RT5390H */
#define IS_RT5390H(_pAd)   (((_pAd->MACVersion & 0xFFFF0000) == 0x53910000) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x1500))

/* RT5390BC8 (WiFi + BT) */

/* RT5390D */
#define IS_RT5390D(_pAd)	((IS_RT5390(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0502))

/* RT5392C */
#define IS_RT5392C(_pAd)	((IS_RT5392(_pAd)) && (((_pAd)->MACVersion & 0x0000FFFF) >= 0x0222)) /* Include RT5392, RT5372 and RT5362 */

#define IS_RT5592(_pAd)		(((_pAd)->MACVersion & 0xFFFF0000) == 0x55920000)
#define REV_RT5592C			0x0221

#define IS_RT8592(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x85590000)

#define IS_MT7601(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x76010000)
#define IS_MT7601U(_pAd)	(IS_MT7601(_pAd) && (IS_USB_INF(_pAd)))

#define IS_MT7650(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76500000)
#define IS_MT7650E(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76500000) && (IS_PCIE_INF(_pAd)))
#define IS_MT7650U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76500000) && (IS_USB_INF(_pAd)))
#define IS_MT7630(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76300000)
#define IS_MT7630E(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76300000) && (IS_PCIE_INF(_pAd)))
#define IS_MT7630U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76300000) && (IS_USB_INF(_pAd)))
#define IS_MT7610(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76100000)
#define IS_MT7610E(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76100000) && (IS_PCIE_INF(_pAd)))
#define IS_MT7610U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76100000) && (IS_USB_INF(_pAd)))
#define IS_MT76x0(_pAd)		(IS_MT7610(_pAd) || IS_MT7630(_pAd) || IS_MT7650(_pAd))
#define IS_MT76x0E(_pAd)	(IS_MT7650E(_pAd) || IS_MT7630E(_pAd) || IS_MT7610E(_pAd))
#define IS_MT76x0U(_pAd)	(IS_MT7650U(_pAd) || IS_MT7630U(_pAd) || IS_MT7610U(_pAd))

#define IS_MT7662(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76620000)
#define IS_MT7662E(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76620000) && (IS_PCIE_INF(_pAd)))
#define IS_MT7662U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76620000) && (IS_USB_INF(_pAd)))
#define IS_MT7632(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76320000)
#define IS_MT7632E(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76320000) && (IS_PCIE_INF(_pAd)))
#define IS_MT7632U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76320000) && (IS_USB_INF(_pAd)))
#define IS_MT7612(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76120000)
#define IS_MT7612E(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76120000) && (IS_PCIE_INF(_pAd)))
#define IS_MT7612U(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76120000) && (IS_USB_INF(_pAd)))
#define IS_MT7602(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76020000)
#define IS_MT7602E(_pAd)	((((_pAd)->ChipID & 0xffff0000) == 0x76020000) && (IS_PCIE_INF(_pAd)))
#define IS_MT76x2(_pAd)		(IS_MT7662(_pAd) || IS_MT7632(_pAd) || IS_MT7612(_pAd) || IS_MT7602(_pAd))
#define IS_MT76x2E(_pAd)	(IS_MT7662E(_pAd) || IS_MT7632E(_pAd) || IS_MT7612E(_pAd) || IS_MT7602E(_pAd))
#define IS_MT76x2U(_pAd)	(IS_MT7662U(_pAd) || IS_MT7632U(_pAd) || IS_MT7612U(_pAd))
#define REV_MT76x2E3        0x0022
#define REV_MT76x2E4        0x0033

#define IS_MT76xx(_pAd)		(IS_MT76x0(_pAd) || IS_MT76x2(_pAd))

#define IS_RT65XX(_pAd)		((((_pAd)->MACVersion & 0xFFFF0000) == 0x65900000) ||\
							 (IS_RT8592(_pAd))||\
							 (IS_MT76x0(_pAd)) ||\
							 (IS_MT76x2(_pAd)))


#define IS_MT7603(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007603)
#define IS_MT7603E(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_PCIE_INF(_pAd))
#define IS_MT7603U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_USB_INF(_pAd) && (_pAd->AntMode == 0))
#define IS_MT76031U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_USB_INF(_pAd) && (_pAd->AntMode == 1))

#define MT7603E1 0x0000
#define MT7603E2 0x0010

/*
#define IS_MT7636(_pAd)     ((((_pAd)->ChipID & 0x0000ffff) == 0x00007606)||\
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007636) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007611))
*/

/*	MT76x6 series: 7636/7606/7611
 	1. For definition/compiler option, use MT7636 for all 7636/7603/7611
 	2. For run time case (specific H/W), please use IS_MT7636(), IS_MT7606(), MT7611()
 	3. For run time case (all H/Ws), please use IS_MT76x6()
 	4. For different interface, please use IS_MT7636U(), IS_MT7636S() */
#define IS_MT76x6(_pAd)     ((((_pAd)->ChipID & 0x0000ffff) == 0x00007606)||\
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007636) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007611))

#define IS_MT7636U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_USB_INF(_pAd))
#define IS_MT7636S(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_SDIO_INF(_pAd))

#define IS_MT7636(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007636)
#define IS_MT7606(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007606)
#define IS_MT7611(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007611)

/* MT7636 Hardeware Version */
#define MT76x6E1 0x0000		/* MT7636 E1	0x8A00 */
#define MT76x6E2 0x0001		/* MT7636 E2 0x8A01 */
#define MT76x6E3 0x0010		/* MT7636 E3 0x8A10 */

/* MT7636 Firmware Version */
#define MT76x6_FW_VER_E1		0x00		/* MT7636 FW Ver E1	0x8A00, use E1 rom patch */
#define MT76x6_FW_VER_E3		0x10		/* MT7636 FW Ver E3	0x8A10, use E3 rom patch */

#define IS_MT76x6_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT76x6_FW_VER_E1)
#define IS_MT76x6_FW_VER_E3(_pAd)		(((_pAd)->FWVersion & 0xff) == MT76x6_FW_VER_E3)


/*	MT7637 series: TBD */
#define IS_MT7637(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007637)
#define IS_MT7637U(_pAd)	(IS_MT7637(_pAd) && IS_USB_INF(_pAd))
#define IS_MT7637S(_pAd)	(IS_MT7637(_pAd)&& IS_SDIO_INF(_pAd))
#define IS_MT7637E(_pAd)	(IS_MT7637(_pAd)&& IS_PCIE_INF(_pAd))


/* MT7637 Hardeware Version */
#define MT7637E1 0x0000		/* MT7637 E1	0x8A00 */
#define MT7637E2 0x0001		/* MT7637 E2 0x8A01 */
#define MT7637E3 0x0010		/* MT7637 E3 0x8A10 */

/* MT7637 Firmware Version */
#define MT7637_FW_VER_E1		0x00		/* MT7637 FW Ver E1	0x8A00, use E1 rom patch */
#define MT7637_FW_VER_E3		0x10		/* MT7637 FW Ver E3	0x8A10, use E3 rom patch */

#define IS_MT7637_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7637_FW_VER_E1)
#define IS_MT7637_FW_VER_E3(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7637_FW_VER_E3)

/* MT7615 Firmware Version */
#define MT7615_FW_VER_E1		0x00		/* MT7615 FW Ver E1	0x8A00, use E1 RAM IMAGE */
#define MT7615_FW_VER_E3		0x10		/* MT7615 FW Ver E3	0x8A10, use E3 RAM IMAGE */

#define IS_MT7615_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7615_FW_VER_E1)
#define IS_MT7615_FW_VER_E3(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7615_FW_VER_E3)


#define IS_MT7628(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007628)

#define MT7628E1 0x0000
#define MT7628E2 0x0010

#define IS_MT7615(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007615)

#define MT7615E1 0x0000       /* MT7615 E1 0x8A00 */
#define MT7615E2 0x0001       /* MT7615 E2 0x8A01 */
#define MT7615E3 0x0010       /* MT7615 E3 0x8A10 */

#define IS_MT7622(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007622)

/* RT3592BC8 (WiFi + BT) */
#define RT_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) < (_rev))

#define RT_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) >= (_rev))

#define MT_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) < (_rev))

#define MT_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) >= (_rev))

#define MTK_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) < (_rev))

#define MT_REV_ET(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) == (_rev))

#define MTK_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) >= (_rev))

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
#define COUNTRY_REGION_A_BAND_MASK (0xff)
#define COUNTRY_REGION_G_BAND (0xff << 8)

#define EEPROM_DEFINE_MAX_TXPWR			0x4e
#define MAX_EIRP_TX_PWR_G_BAND_MASK (0xff)
#define MAX_EIRP_TX_PWR_A_BAND_MASK (0xff << 8)

#define EEPROM_FREQ_OFFSET			0x3a
#define FREQ_OFFSET_MASK (0x7f)
#define FREQ_OFFSET_DIP (1 << 7)
#define LED_MODE_MASK (0xff << 8)

#define EEPROM_LEDAG_CONF_OFFSET	0x3c
#define EEPROM_LEDACT_CONF_OFFSET	0x3e
#define EEPROM_LED_POLARITY_OFFSET	0x40

#define EEPROM_LNA_OFFSET			0x44
#define LNA_GAIN_G_BAND_MASK (0x7f)
#define LNA_GAIN_G_BAND_EN (1 << 7)
#define LNA_GAIN_A_BAND_CH36_64_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH36_64_EN (1 << 15)

#define EEPROM_RSSI_BG_OFFSET			0x46
#define RSSI0_OFFSET_G_BAND_MASK (0x3f)
#define RSSI0_OFFSET_G_BAND_SIGN (1 << 6)
#define RSSI0_OFFSET_G_BAND_EN (1 << 7)
#define RSSI1_OFFSET_G_BAND_MASK (0x3f << 8)
#define RSSI1_OFFSET_G_BAND_SIGN (1 << 14)
#define RSSI1_OFFSET_G_BAND_EN (1 << 15)

#define EEPROM_TXMIXER_GAIN_2_4G		0x48
#define LNA_GAIN_A_BAND_CH100_128_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH100_128_EN (1 << 15)

#define EEPROM_RSSI_A_OFFSET			0x4a
#define RSSI0_OFFSET_A_BAND_MASK (0x3f)
#define RSSI0_OFFSET_A_BAND_SIGN (1 << 6)
#define RSSI0_OFFSET_A_BANE_EN (1 << 7)
#define RSSI1_OFFSET_A_BAND_MASK (0x3f << 8)
#define RSSI1_OFFSET_A_BAND_SIGN (1 << 14)
#define RSSI1_OFFSET_A_BAND_EN (1 << 15)

#define EEPROM_TXMIXER_GAIN_5G			0x4c
#define LNA_GAIN_A_BAND_CH132_165_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH132_165_EN (1 << 15)

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


enum FREQ_CAL_INIT_MODE {
	FREQ_CAL_INIT_MODE0,
	FREQ_CAL_INIT_MODE1,
	FREQ_CAL_INIT_MODE2,
	FREQ_CAL_INIT_UNKNOW,
};

enum FREQ_CAL_MODE {
	FREQ_CAL_MODE0,
	FREQ_CAL_MODE1,
	FREQ_CAL_MODE2,
};

enum RXWI_FRQ_OFFSET_FIELD {
	RXWI_FRQ_OFFSET_FIELD0, /* SNR1 */
	RXWI_FRQ_OFFSET_FIELD1, /* Frequency Offset */
};



#define EEPROM_A_TSSI_BOUND1		0xd4
#define EEPROM_A_TSSI_BOUND2		0xd6
#define EEPROM_A_TSSI_BOUND3		0xd8
#define EEPROM_A_TSSI_BOUND4		0xda
#define EEPROM_A_TSSI_BOUND5		0xdc



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
#define RT28xx_EEPROM_READ16(_pAd, _offset, _val)   rt28xx_eeprom_read16(_pAd, _offset, &(_val))

#define RT28xx_EEPROM_WRITE16(_pAd, _offset, _val)		\
	do {\
		if ((_pAd)->chipOps.eewrite) \
			(_pAd)->chipOps.eewrite(_pAd, (_offset), (USHORT)(_val));\
	} while(0)

#define RT28xx_EEPROM_READ_WITH_RANGE(_pAd, _start, _length, _pbuf)   rt28xx_eeprom_read_with_range(_pAd, _start, _length, _pbuf)

#define RT28xx_EEPROM_WRITE_WITH_RANGE(_pAd, _start, _length, _pbuf)		\
	do {\
		if ((_pAd)->chipOps.eewrite_range) \
			(_pAd)->chipOps.eewrite_range(_pAd, _start, _length, _pbuf);\
	} while(0)		

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
/* The Tx power tuning entry */
typedef struct _TX_POWER_TUNING_ENTRY_STRUCT {
	CHAR	RF_TX_ALC; 		/* 3390: RF R12[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC */
	CHAR 	MAC_PowerDelta;	/* Tx power control over MAC 0x1314~0x1324 */
} TX_POWER_TUNING_ENTRY_STRUCT, *PTX_POWER_TUNING_ENTRY_STRUCT;
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

struct RF_BANK_OFFSET {
	UINT8 RFBankIndex;
	UINT16 RFStart;
	UINT16 RFEnd;
};

struct RF_INDEX_OFFSET {
	UINT8 RFIndex;
	UINT16 RFStart;
	UINT16 RFEnd;
};



struct _RSSI_SAMPLE;
struct _EXT_CMD_EFUSE_BUFFER_MODE_T;


#define RTMP_CHIP_ENABLE_GREENAP(__pAd, __greenap_on_off)	\
do {	\
		if (__pAd->chipOps.EnableAPMIMOPS != NULL)	\
			__pAd->chipOps.EnableAPMIMOPS(__pAd, __greenap_on_off);	\
} while (0)

#define RTMP_CHIP_DISABLE_GREENAP(__pAd, __greenap_on_off)	\
do {	\
		if (__pAd->chipOps.DisableAPMIMOPS != NULL)	\
			__pAd->chipOps.DisableAPMIMOPS(__pAd, __greenap_on_off);	\
} while (0)


#define PWR_SAVING_OP(__pAd, __PwrOP, __PwrLevel, __ListenInterval, \
						__PreTBTTLeadTime, __TIMByteOffset, __TIMBytePattern)	\
do {	\
		if (__pAd->chipOps.PwrSavingOP != NULL)	\
			__pAd->chipOps.PwrSavingOP(__pAd, __PwrOP, __PwrLevel,	\
										__ListenInterval,__PreTBTTLeadTime, \
										__TIMByteOffset, __TIMBytePattern);	\
} while (0)

#define RTMP_CHIP_RX_SENSITIVITY_TUNING(__pAd)	\
do {	\
		if (__pAd->chipOps.RxSensitivityTuning != NULL)	\
			__pAd->chipOps.RxSensitivityTuning(__pAd);	\
} while (0)

#define RTMP_CHIP_ASIC_AGC_ADJUST(__pAd, __Rssi, __R66)	\
do {	\
		if (__pAd->chipOps.ChipAGCAdjust != NULL)	\
			__R66 = __pAd->chipOps.ChipAGCAdjust(__pAd, __Rssi, __R66);	\
} while (0)

#define RTMP_CHIP_ASIC_TSSI_TABLE_INIT(__pAd)	\
do {	\
		if (__pAd->chipOps.InitDesiredTSSITable != NULL)	\
			__pAd->chipOps.InitDesiredTSSITable(__pAd);	\
} while (0)

#define RTMP_CHIP_ATE_TSSI_CALIBRATION(__pAd, __pData)	\
do {	\
		if (__pAd->chipOps.ATETssiCalibration != NULL)	\
			__pAd->chipOps.ATETssiCalibration(__pAd, __pData);	\
} while (0)

#define RTMP_CHIP_ATE_TSSI_CALIBRATION_EXTEND(__pAd, __pData)	\
do {	\
		if (__pAd->chipOps.ATETssiCalibrationExtend != NULL)	\
			__pAd->chipOps.ATETssiCalibrationExtend(__pAd, __pData);	\
} while (0)

#define RTMP_CHIP_ATE_READ_EXTERNAL_TSSI(__pAd, __pData)	\
do {	\
		if (__pAd->chipOps.ATEReadExternalTSSI != NULL)	\
			__pAd->chipOps.ATEReadExternalTSSI(__pAd, __pData);	\
} while (0)

#define RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(__pAd, __pCfgOfTxPwrCtrlOverMAC)	\
do {	\
		if (__pAd->chipOps.AsicGetTxPowerOffset != NULL)	\
			__pAd->chipOps.AsicGetTxPowerOffset(__pAd, __pCfgOfTxPwrCtrlOverMAC);	\
} while (0)

#define RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(	\
		__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1, __Channel)	\
do {	\
		if (__pAd->chipOps.AsicTxAlcGetAutoAgcOffset != NULL)	\
			__pAd->chipOps.AsicTxAlcGetAutoAgcOffset(	\
		__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1, __Channel);	\
} while (0)

#define RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(__pAd)	\
do {	\
		if (__pAd->chipOps.AsicExtraPowerOverMAC != NULL)	\
			__pAd->chipOps.AsicExtraPowerOverMAC(__pAd);	\
} while (0)

#define RTMP_CHIP_ASIC_ADJUST_TX_POWER(__pAd)					\
do {	\
		if (__pAd->chipOps.AsicAdjustTxPower != NULL)					\
			__pAd->chipOps.AsicAdjustTxPower(__pAd);	\
} while (0)

#define RTMP_CHIP_ASIC_GET_TSSI_RATIO(__pAd, __DeltaPwr)	\
do {	\
		if (__pAd->chipOps.AsicFreqCalStop != NULL)	\
			__pAd->chipOps.TSSIRatio(__DeltaPwr);	\
} while (0)

#define RTMP_CHIP_ASIC_FREQ_CAL_STOP(__pAd)	\
do {	\
		if (__pAd->chipOps.AsicFreqCalStop != NULL)	\
			__pAd->chipOps.AsicFreqCalStop(__pAd);	\
} while (0)

#define RTMP_CHIP_IQ_CAL(__pAd, __pChannel)	\
do {	\
		if (__pAd->chipOps.ChipIQCalibration != NULL)	\
			 __pAd->chipOps.ChipIQCalibration(__pAd, __pChannel);	\
} while (0)

#define RTMP_CHIP_HIGH_POWER_TUNING(__pAd, __pRssi)	\
do {	\
		if (__pAd->chipOps.HighPowerTuning != NULL)	\
			__pAd->chipOps.HighPowerTuning(__pAd, __pRssi);	\
} while (0)

#define RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(__pAd, __pAntenna)	\
do {	\
		if (__pAd->chipOps.AsicAntennaDefaultReset != NULL)	\
			__pAd->chipOps.AsicAntennaDefaultReset(__pAd, __pAntenna);	\
} while (0)

#define RTMP_NET_DEV_NICKNAME_INIT(__pAd)	\
do {	\
		if (__pAd->chipOps.NetDevNickNameInit != NULL)	\
			__pAd->chipOps.NetDevNickNameInit(__pAd);	\
} while (0)

#ifdef CAL_FREE_IC_SUPPORT
#define RTMP_CAL_FREE_IC_CHECK(__pAd, __is_cal_free)	\
do {	\
		if (__pAd->chipOps.is_cal_free_ic != NULL)	\
			__is_cal_free = __pAd->chipOps.is_cal_free_ic(__pAd);	\
		else		\
			__is_cal_free = FALSE;	\
} while (0)

#define RTMP_CAL_FREE_DATA_GET(__pAd)	\
do {	\
		if (__pAd->chipOps.cal_free_data_get != NULL) {	\
			__pAd->chipOps.cal_free_data_get(__pAd);	\
			__pAd->E2pCtrl.e2pSource |= E2P_SRC_FROM_EFUSE; \
		} \
} while (0)
#endif /* CAL_FREE_IC_SUPPORT */

#define RTMP_EEPROM_ASIC_INIT(__pAd)	\
do {	\
		if (__pAd->chipOps.NICInitAsicFromEEPROM != NULL)	\
			__pAd->chipOps.NICInitAsicFromEEPROM(__pAd);	\
} while (0)

#define RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(__pAd)								\
do {	\
		if (__pAd->chipOps.InitTemperCompensation != NULL)					\
			__pAd->chipOps.InitTemperCompensation(__pAd);	\
} while (0)

#define RTMP_CHIP_ASIC_SET_EDCCA(__pAd, __bOn)			\
do {	\
		if (__pAd->chipOps.ChipSetEDCCA != NULL)						\
			__pAd->chipOps.ChipSetEDCCA(__pAd, __bOn);	\
} while (0)

#define RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(__pAd)						\
do {	\
		if (__pAd->chipOps.TemperCompensation != NULL)					\
			__pAd->chipOps.TemperCompensation(__pAd);	\
} while (0)


#define RTMP_CHIP_UPDATE_BEACON(__pAd, Offset, Value, Unit)	\
do {	\
		if (__pAd->chipOps.BeaconUpdate != NULL)	\
			__pAd->chipOps.BeaconUpdate(__pAd, Offset, Value, Unit);	\
} while (0)

#ifdef CARRIER_DETECTION_SUPPORT
#define	RTMP_CHIP_CARRIER_PROGRAM(__pAd, threshold)	\
do {	\
		if(__pAd->chipOps.ToneRadarProgram != NULL)	\
			__pAd->chipOps.ToneRadarProgram(__pAd, threshold);	\
} while (0)
#endif /* CARRIER_DETECTION_SUPPORT */

#define RTMP_CHIP_CCK_MRC_STATUS_CTRL(__pAd)	\
do {	\
		if(__pAd->chipOps.CckMrcStatusCtrl != NULL)	\
			__pAd->chipOps.CckMrcStatusCtrl(__pAd);	\
} while (0)

#define RTMP_CHIP_RADAR_GLRT_COMPENSATE(__pAd) \
do {	\
		if(__pAd->chipOps.RadarGLRTCompensate != NULL)	\
			__pAd->chipOps.RadarGLRTCompensate(__pAd);	\
} while (0)


#define CHIP_CALIBRATION(__pAd, __CalibrationID, param) \
do {	\
	ANDES_CALIBRATION_PARAM calibration_param; \
	calibration_param.generic = param; \
	if(__pAd->chipOps.Calibration != NULL) \
		__pAd->chipOps.Calibration(__pAd, __CalibrationID, &calibration_param); \
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

#ifdef CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT
#define BBP_RANDOM_WRITE(_pAd, _RegPair, _Num)	\
do {	\
		if (_pAd->chipOps.BBPRandomWrite != NULL)	\
			_pAd->chipOps.BBPRandomWrite(_pAd, _RegPair, _Num);	\
} while (0)
#endif /* CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT */

#define RTMP_SECOND_CCA_DETECTION(__pAd) \
do {	\
	if (__pAd->chipOps.SecondCCADetection != NULL)	\
	{	\
		__pAd->chipOps.SecondCCADetection(__pAd);	\
	}	\
} while (0)

#define SC_RANDOM_WRITE(_ad, _table, _num, _flags)	\
do {	\
		if (_ad->chipOps.sc_random_write != NULL)	\
			_ad->chipOps.sc_random_write(_ad, _table, _num, _flags);	\
} while (0)

#define SC_RF_RANDOM_WRITE(_ad, _table, _num, _flags)	\
do {	\
		if (_ad->chipOps.sc_rf_random_write != NULL)	\
			_ad->chipOps.sc_rf_random_write(_ad, _table, _num, _flags);	\
} while (0)

#define DISABLE_TX_RX(_pAd, _Level)	\
do {	\
	if (_pAd->chipOps.DisableTxRx != NULL)	\
		_pAd->chipOps.DisableTxRx(_pAd, _Level);	\
} while (0)

#define ASIC_RADIO_ON(_pAd, _Stage)	\
do {	\
	if (_pAd->chipOps.AsicRadioOn != NULL)	\
		_pAd->chipOps.AsicRadioOn(_pAd, _Stage);	\
} while (0)

#define ASIC_RADIO_OFF(_pAd, _Stage)	\
do {	\
	if (_pAd->chipOps.AsicRadioOff != NULL)	\
		_pAd->chipOps.AsicRadioOff(_pAd, _Stage);	\
} while (0)

#ifdef MICROWAVE_OVEN_SUPPORT
#define ASIC_MEASURE_FALSE_CCA(_pAd)	\
do {	\
	if (_pAd->chipOps.AsicMeasureFalseCCA != NULL)	\
		_pAd->chipOps.AsicMeasureFalseCCA(_pAd);	\
} while (0)

#define ASIC_MITIGATE_MICROWAVE(_pAd)	\
do {	\
	if (_pAd->chipOps.AsicMitigateMicrowave != NULL)	\
		_pAd->chipOps.AsicMitigateMicrowave(_pAd);	\
} while (0)
#endif /* MICROWAVE_OVEN_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
#define ASIC_WOW_ENABLE(_pAd, _pStaCfg)	\
do {	\
	if (_pAd->chipOps.AsicWOWEnable != NULL)	\
		_pAd->chipOps.AsicWOWEnable(_pAd, _pStaCfg);	\
} while (0)

#define ASIC_WOW_DISABLE(_pAd, _pStaCfg)	\
do {	\
	if (_pAd->chipOps.AsicWOWDisable != NULL)	\
		_pAd->chipOps.AsicWOWDisable(_pAd, _pStaCfg);	\
} while(0)

#define ASIC_WOW_INIT(_pAd) \
do {	\
	if (_pAd->chipOps.AsicWOWInit != NULL)	\
		_pAd->chipOps.AsicWOWInit(_pAd);	\
} while(0)

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#define MCU_CTRL_INIT(_pAd)	\
do {	\
	if (_pAd->chipOps.MCUCtrlInit != NULL)	\
		_pAd->chipOps.MCUCtrlInit(_pAd);	\
} while (0)

#define MCU_CTRL_EXIT(_pAd)	\
do {	\
	if (_pAd->chipOps.MCUCtrlExit != NULL)	\
		_pAd->chipOps.MCUCtrlExit(_pAd);	\
} while (0)

#define USB_CFG_READ(_ad, _pvalue)	\
do {	\
	if (_ad->chipOps.usb_cfg_read != NULL)	\
		_ad->chipOps.usb_cfg_read(_ad, _pvalue);	\
	else {\
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s(): usb_cfg_read not inited!\n", __FUNCTION__));\
	}\
} while (0)

#define USB_CFG_WRITE(_ad, _value)	\
do {	\
	if (_ad->chipOps.usb_cfg_write != NULL)	\
		_ad->chipOps.usb_cfg_write(_ad, _value);	\
	else {\
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s(): usb_cfg_write not inited!\n", __FUNCTION__));\
	}\
} while (0)

#ifdef SMART_CARRIER_SENSE_SUPPORT
#define RTMP_CHIP_ASIC_SET_SCS(__pAd, __BandIdx, __value)			\
do {	\
		if (__pAd->chipOps.ChipSetSCS != NULL)						\
			__pAd->chipOps.ChipSetSCS(__pAd,__BandIdx, __value);	\
} while (0)
#endif /* SMART_CARRIER_SENSE_SUPPORT */

VOID AsicInitTxRxRing(struct _RTMP_ADAPTER *pAd);

int RtmpChipOpsHook(VOID *pCB);
VOID RtmpChipBcnInit(struct _RTMP_ADAPTER *pAd);
VOID RtmpChipBcnSpecInit(struct _RTMP_ADAPTER *pAd);
#ifdef RLT_MAC
VOID rlt_bcn_buf_init(struct _RTMP_ADAPTER *pAd);
#endif /* RLT_MAC */
#ifdef MT_MAC
VOID mt_bcn_buf_init(struct _RTMP_ADAPTER *pAd);
VOID mt_chip_bcn_parameter_init(struct _RTMP_ADAPTER *pAd);
#endif /* MT_MAC */

UINT8 NICGetBandSupported(struct _RTMP_ADAPTER *pAd);

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

INT tx_pwr_comp_init(struct _RTMP_ADAPTER *pAd);


VOID NetDevNickNameInit(IN struct _RTMP_ADAPTER *pAd);


#ifdef ANT_DIVERSITY_SUPPORT
VOID HWAntennaDiversityEnable(struct _RTMP_ADAPTER *pAd);
#endif /* ANT_DIVERSITY_SUPPORT */

#ifdef GREENAP_SUPPORT
struct greenap_ctrl;
struct greenap_on_off_ctrl;
VOID greenap_init(
    struct greenap_ctrl *greenap);
VOID greenap_exit(
    struct _RTMP_ADAPTER *pAd, 
    struct wifi_dev *wdev, 
    struct greenap_ctrl *greenap);
VOID greenap_show(
    struct _RTMP_ADAPTER *pAd, 
    struct greenap_ctrl *greenap);
VOID greenap_check_when_if_down_up(
    struct _RTMP_ADAPTER *pA, 
    struct greenap_ctrl *greenap);
VOID greenap_check_peer_connection_status(
    struct _RTMP_ADAPTER *pAd, 
    UINT8 band_idx, 
    BOOLEAN previous_greenap_active,
    BOOLEAN greenap_allow);
VOID greenap_check_peer_connection_at_link_up_down(
    struct _RTMP_ADAPTER *pAd,
    struct wifi_dev *wdev,
    struct greenap_ctrl *greenap);
BOOLEAN greenap_get_suspend_status(
    struct greenap_ctrl *greenap);
VOID greenap_suspend(
    struct _RTMP_ADAPTER *pAd, 
    struct greenap_ctrl *greenap,
    UINT32 reason);
VOID greenap_resume(
    struct _RTMP_ADAPTER *pAd, 
    struct greenap_ctrl *greenap,
    UINT32 reason);
VOID greenap_check_allow_status(
    struct _RTMP_ADAPTER *pAd, 
    struct greenap_ctrl *greenap);
BOOLEAN greenap_get_allow_status(
    struct greenap_ctrl *greenap);
VOID greenap_set_capability(
    struct greenap_ctrl *greenap, 
    BOOLEAN greenap_cap);
BOOLEAN greenap_get_capability(
    struct greenap_ctrl *greenap);
VOID greenap_proc(
    struct _RTMP_ADAPTER *pAd, 
    struct greenap_ctrl *greenap, 
    BOOLEAN greenap_cap_on);

VOID enable_greenap(
    struct _RTMP_ADAPTER *pAd, 
    struct greenap_on_off_ctrl *greenap_on_off);
VOID disable_greenap(
    struct _RTMP_ADAPTER *pAd, 
    struct greenap_on_off_ctrl *greenap_on_off);
VOID EnableAPMIMOPSv2(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
VOID DisableAPMIMOPSv2(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
VOID EnableAPMIMOPSv1(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
VOID DisableAPMIMOPSv1(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
#endif /* GREENAP_SUPPORT */

#ifdef RTMP_MAC
VOID RTxx_default_Init(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_MAC */


/* global variable */
extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];
extern UCHAR NUM_OF_3020_CHNL;
extern FREQUENCY_ITEM *FreqItems3020;
extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

INT AsicGetMacVersion(struct _RTMP_ADAPTER *pAd);

INT WaitForAsicReady(struct _RTMP_ADAPTER *pAd);

INT AsicWaitMacTxRxIdle(struct _RTMP_ADAPTER *pAd);

#define ASIC_MAC_TX			1
#define ASIC_MAC_RX			2
#define ASIC_MAC_TXRX		3
#define ASIC_MAC_TXRX_RXV	4
#define ASIC_MAC_RXV		5
#define ASIC_MAC_RX_RXV		6

INT AsicSetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 txrx, BOOLEAN enable);

enum {
	PDMA_TX,
	PDMA_RX,
	PDMA_TX_RX,
};

INT AsicSetWPDMA(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable);
BOOLEAN AsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us);
INT AsicSetMacWD(struct _RTMP_ADAPTER *pAd);

INT AsicReadAggCnt(struct _RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len);

INT AsicTOPInit(struct _RTMP_ADAPTER *pAd);

#ifdef MT_MAC
INT rt28xx_read16(struct _RTMP_ADAPTER *pAd, USHORT , USHORT *);
#endif /* MT_MAC */

INT StopDmaTx(struct _RTMP_ADAPTER *pAd, UCHAR Level);
INT StopDmaRx(struct _RTMP_ADAPTER *pAd, UCHAR Level);


#endif /* __RTMP_CHIP_H__ */

