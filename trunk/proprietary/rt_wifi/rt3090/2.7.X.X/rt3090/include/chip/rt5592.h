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
	rt5592.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT5592_H__
#define __RT5592_H__

#include "chip/rt30xx.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "For RT5592, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#ifndef RT30xx
#error "For RT5592, you should define the compile flag -DRT30xx"
#endif

#define BBP_REG_BF			BBP_R163 // TxBf control
#define BBP_REG_BF			BBP_R163 // TxBf control



#ifdef RT5592EP_SUPPORT
#define RT5592_TYPE_EP 1
#endif /* RT5592EP_SUPPORT */

/*
 * If MAC 0x5E8 bit[31] = 0, Xtal is 20M
 * If MAC 0x5E8 bit[31] = 1, Xtal is 40M
 */ 
enum XTAL{
	XTAL20M,
	XTAL40M
};

/* 
 * Frequency plan item  for RT5592 
 * N: R9[4], R8[7:0]
 * K: R9[3:0]
 * mod: R9[7], R11[3:2] (eg. mod=8 => 0x0, mod=10 => 0x2)
 * R: R11[1:0] (eg. R=1 => 0x0, R=3 => 0x2)
 */
typedef struct _RT5592_FREQUENCY_ITEM {
	UCHAR Channel;
	UINT16 N;
	UCHAR K;
	UCHAR mod;
	UCHAR R;
} RT5592_FREQUENCY_ITEM, *PRT5592_FREQUENCY_ITEM;

/* Frequency plan table */
typedef struct _RT5592_FREQUENCY_PLAN {
	const struct _RT5592_FREQUENCY_ITEM *pFrequencyPlan;
	UCHAR totalFreqItem;
} RT5592_FREQUENCY_PLAN, *PRT5592_FREQUENCY_PLAN;

VOID RT5592_Init(
	IN struct _RTMP_ADAPTER			*pAd);

#endif /* __RT5592_H__ */
