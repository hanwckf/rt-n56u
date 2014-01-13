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
	rt35xx.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT35XX_H__
#define __RT35XX_H__

#ifdef RT35xx

struct _RTMP_ADAPTER;

#ifndef RTMP_RF_RW_SUPPORT
#error "For RT3062/3562/3572/3592, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#include "chip/rt30xx.h"

extern REG_PAIR   RF3572_RFRegTable[];

/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */
#define NIC3062_PCI_DEVICE_ID	0x3062		/* 2T/2R miniCard */
#define NIC3562_PCI_DEVICE_ID	0x3562		/* 2T/2R miniCard */
#define NIC3060_PCI_DEVICE_ID	0x3060		/* 1T/1R miniCard */

#define EDIMAX_PCI_VENDOR_ID	0x1432

/* use CHIPSET = 3562 compile */
#define NIC3592_PCIe_DEVICE_ID	0x3592		/* 2T/2R miniCard */

VOID RT35xx_Init(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT35xxMacRegisters(
	IN	struct _RTMP_ADAPTER	*pAd);

VOID RT35xx_ChipSwitchChannel(
	IN struct _RTMP_ADAPTER		*pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan);

VOID RT35xx_RxSensitivityTuning(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT3572RFRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT3572ReverseRFSleepModeSetup(
	IN struct _RTMP_ADAPTER		*pAd,
	IN BOOLEAN					FlgIsInitState);

VOID RT35xx_NICInitAsicFromEEPROM(
	IN struct _RTMP_ADAPTER		*pAd);

UCHAR RT35xx_ChipAGCAdjust(
	IN	struct _RTMP_ADAPTER	*pAd,
	IN  CHAR					Rssi,
	IN  UCHAR					R66);

VOID RT35xx_ChipBBPAdjust(
	IN	struct _RTMP_ADAPTER	*pAd);

#endif /* RT35xx */
#endif /*__RT35XX_H__ */

