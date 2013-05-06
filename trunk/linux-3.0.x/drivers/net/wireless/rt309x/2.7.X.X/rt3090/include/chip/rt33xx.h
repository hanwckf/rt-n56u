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

#ifndef __RT33XX_H__
#define __RT33XX_H__

#ifdef RT33xx

#include "rtmp_type.h"

#ifdef RT3390
extern REG_PAIR RT3390_RFRegTable[];
extern UCHAR RT3390_NUM_RF_REG_PARMS;
#define BW20RFR24	0x48
#define BW40RFR24	0X68
#define BW20RFR31	0x4F
#define BW40RFR31	0X6F
#endif /* RT3390 */


VOID RT33xx_Init(
        IN struct _RTMP_ADAPTER         *pAd);

VOID RT33xx_ChipSwitchChannel(
	IN struct _RTMP_ADAPTER 	*pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan);

#ifdef RTMP_INTERNAL_TX_ALC
VOID RT33xx_InitDesiredTSSITable(
	IN struct _RTMP_ADAPTER		*pAd);

UCHAR RT33xx_GetDesiredTSSI(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT33xx_AsicTxAlcGetAutoAgcOffset(
	IN struct _RTMP_ADAPTER		*pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1);
#endif /* RTMP_INTERNAL_TX_ALC */

#endif /* RT33xx */

#endif /*__RT33XX_H__ */

