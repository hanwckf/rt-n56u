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
	rt5390.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT53xx_H__
#define __RT53xx_H__

#ifdef RT53xx


#ifndef RTMP_RF_RW_SUPPORT
#error "For RT5390, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#ifndef RT30xx
#error "For RT5390, you should define the compile flag -DRT30xx"
#endif
#ifdef CARRIER_DETECTION_SUPPORT
#define TONE_RADAR_DETECT_SUPPORT
#define TONE_RADAR_DETECT_V2
#endif // CARRIER_DETECTION_SUPPORT //

extern REG_PAIR RF5390RegTable[];
extern UCHAR NUM_RF_5390_REG_PARMS;

#define BBP_REG_BF			BBP_R163 // TxBf control

#ifdef RTMP_FLASH_SUPPORT
#undef EEPROM_DEFAULT_FILE_PATH
#define EEPROM_DEFAULT_FILE_PATH			"/etc_ro/Wireless/RT5392_PCIe_2T2R_ALC_V1_3.bin"

#if defined (RT_IFNAME_1ST)
#if defined (CONFIG_RT_FIRST_IF_RF_OFFSET)
 #define RF_OFFSET					CONFIG_RT_FIRST_IF_RF_OFFSET
#else
 #define RF_OFFSET					0x40000
#endif
#else /* !RT_IFNAME_1ST */
#if defined (CONFIG_RT_SECOND_IF_RF_OFFSET)
 #define RF_OFFSET					CONFIG_RT_SECOND_IF_RF_OFFSET
#else
 #define RF_OFFSET					0x48000
#endif
#endif /* RT_IFNAME_1ST */

extern void RtmpFlashWrite(UCHAR * p, ULONG a, ULONG b);
extern void RtmpFlashRead(UCHAR * p, ULONG a, ULONG b);
#endif // RTMP_FLASH_SUPPORT //
//
// Device ID & Vendor ID, these values should match EEPROM value
//
#define NIC5390_PCIe_DEVICE_ID	0x5390
#define NIC539F_PCIe_DEVICE_ID  0x539F
#define NIC5392_PCIe_DEVICE_ID  0x5392
#define NIC5362_PCI_DEVICE_ID	0x5362
#define NIC5360_PCI_DEVICE_ID   0x5360
#endif // RT53xx //

#endif //__RT53xx_H__ //

