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
	rt2883.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT2883_H__
#define __RT2883_H__

#ifndef RTMP_RBUS_SUPPORT
#error "For RT2880, you should define the compile flag -DRTMP_RBUS_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT2880, you should define the compile flag -DRTMP_MAC_PCI"
#endif

struct _RTMP_ADAPTER;

#define RTMP_MAC_CSR_ADDR 0xB0180000
#define RTMP_FLASH_BASE_ADDR	0xbfc00000

#define BBP_REG_BF		BBP_R105
#define BBP_REG_SNR0	BBP_R189
#define BBP_REG_SNR1	BBP_R190
#define BBP_REG_SNR2	BBP_R191


extern REG_PAIR   RT2883_BBPRegTable[];
extern UCHAR RT2883_NUM_BBP_REG_PARMS;


/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */


/* ========================= 
	Function definition
   ========================= */
VOID NICInitRT2883MacRegisters(
	IN	struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT2883BbpRegisters(
	IN	struct _RTMP_ADAPTER 		*pAd);

#endif /*__RT2880_H__ */

