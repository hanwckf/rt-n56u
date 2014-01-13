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
	rt305x.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RT305x_H__
#define __RT305x_H__

#ifdef RT305x

struct _RTMP_ADAPTER;

#ifndef RTMP_RBUS_SUPPORT
#error "For RT305x, you should define the compile flag -DRTMP_RBUS_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT305x, you should define the compile flag -DRTMP_MAC_PCI"
#endif

#ifndef RTMP_RF_RW_SUPPORT
#error "For RT305x, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#define RTMP_SYS_CTL_ADDR 0xB0000000
#define RTMP_INT_CTL_ADDR 0xB0000200
#define RTMP_PIO_CTL_ADDR 0xB0000600
#define RTMP_MAC_CSR_ADDR 0xB0180000
#define RTMP_FLASH_BASE_ADDR 0xbfc00000

/* System Control */
#define RTMP_SYS_GPIOMODE_OFFSET 0x0060

/* Programmable I/O */
#define RTMP_PIO2100_DATA_OFFSET 0x0020
#define RTMP_PIO2100_DIR_OFFSET 0x0024
#define RTMP_PIO2100_POL_OFFSET 0x0028

extern REG_PAIR   RT305x_RFRegTable[];

extern REG_PAIR   RT305x_BBPRegTable[];
extern UCHAR RT305x_NUM_BBP_REG_PARMS;

//
// Device ID & Vendor ID, these values should match EEPROM value
//


VOID RT305x_Init(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT305xMacRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT305xBbpRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT305x_ChipSwitchChannel(
	IN struct _RTMP_ADAPTER 	*pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan);

#ifdef RTMP_INTERNAL_TX_ALC
VOID RT3350_InitDesiredTSSITable(
	IN struct _RTMP_ADAPTER		*pAd);

UCHAR RT3350_GetDesiredTSSI(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT3350_AsicTxAlcGetAutoAgcOffset(
	IN struct _RTMP_ADAPTER		*pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1);
#endif /* RTMP_INTERNAL_TX_ALC */

VOID RT305x_ChipSpecInit(
	IN struct _RTMP_ADAPTER		*pAd);

#endif // RT305x //

#endif //__RT305x_H__ //

