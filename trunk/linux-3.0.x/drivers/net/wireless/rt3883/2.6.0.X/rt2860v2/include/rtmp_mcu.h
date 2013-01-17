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
	rtmp_mcu.h

	Abstract:
	Miniport header file for mcu related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __RTMP_MCU_H__
#define __RTMP_MCU_H__

INT RtmpAsicEraseFirmware(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS RtmpAsicLoadFirmware(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS isMCUnotReady(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS isMCUNeedToLoadFIrmware(
	IN PRTMP_ADAPTER pAd);

INT RtmpAsicSendCommandToMcu(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Command,
	IN UCHAR			Token,
	IN UCHAR			Arg0,
	IN UCHAR			Arg1,
	IN BOOLEAN			FlgIsNeedLocked);

#endif /* __RTMP_MCU_H__ */
