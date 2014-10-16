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
	rt3090.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT3090_H__
#define __RT3090_H__

#ifdef RT3090

#ifndef RTMP_PCI_SUPPORT
#error "For RT3090, you should define the compile flag -DRTMP_PCI_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT3090, you should define the compile flag -DRTMP_MAC_PCI"
#endif

#ifndef RTMP_RF_RW_SUPPORT
#error "For RT3090, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#ifndef RT30xx
#error "For RT3090, you should define the compile flag -DRT30xx"
#endif

#ifdef CARRIER_DETECTION_SUPPORT
#define TONE_RADAR_DETECT_SUPPORT
#define TONE_RADAR_DETECT_V1
#define TONE_RADAR_DETECT_V2
#endif // CARRIER_DETECTION_SUPPORT //

#include "chip/mac_pci.h"
#include "chip/rt30xx.h"
#ifdef RTMP_FLASH_SUPPORT
#define EEPROM_DEFAULT_FILE_PATH			"/etc_ro/Wireless/RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"

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
#define NIC3090_PCIe_DEVICE_ID  0x3090		// 1T/1R miniCard
#define NIC3091_PCIe_DEVICE_ID  0x3091		// 1T/2R miniCard
#define NIC3092_PCIe_DEVICE_ID  0x3092		// 2T/2R miniCard

#endif // RT3090 //

#endif //__RT3090_H__ //

