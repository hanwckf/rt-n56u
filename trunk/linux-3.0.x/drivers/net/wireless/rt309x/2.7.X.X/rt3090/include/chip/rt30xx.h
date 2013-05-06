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
	rt30xx.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT30XX_H__
#define __RT30XX_H__

#ifdef RT30xx

struct _RTMP_ADAPTER;

#include "rtmp_type.h"

extern REG_PAIR RT3020_RFRegTable[];
extern UCHAR NUM_RF_3020_REG_PARMS;

VOID RT30xx_Init(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT30xx_ChipSwitchChannel(
	IN struct _RTMP_ADAPTER		*pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan);

VOID RT30xx_ChipBBPAdjust(
	IN struct _RTMP_ADAPTER	*pAd);

VOID RT30xx_ChipAGCInit(
	IN struct _RTMP_ADAPTER		*pAd,
	IN UCHAR					BandWidth);

#ifdef MICROWAVE_OVEN_SUPPORT
VOID RT30xx_AsicMitigateMicrowave(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT30xx_AsicMeasureFalseCCA(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
VOID RtmpEfuseSupportCheck(
	IN struct _RTMP_ADAPTER 	*pAd);
#endif /* RTMP_EFUSE_SUPPORT */
#endif /* RT30xx */

#endif /*__RT30XX_H__ */

