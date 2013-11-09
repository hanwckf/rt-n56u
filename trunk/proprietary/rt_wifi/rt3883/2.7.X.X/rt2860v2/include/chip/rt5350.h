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
	rt5350.h
 
    Abstract:
	2*2 Wireless Chip SoC

    Revision History:
    Who			When			What
    ---------	----------		----------------------------------------------
	SampleLin	20100908		Initial version
 */

#ifndef __RT5350_H__
#define __RT5350_H__

#ifdef RT5350

struct _RTMP_ADAPTER;

#undef EEPROM_TSSI_REF_OFFSET

#define EEPROM_TSSI_REF_OFFSET		        0x6E
#define EEPROM_TSSI_DELTA_CH1_CH2			0x6F
#define EEPROM_TSSI_DELTA_CH3_CH4			0x70
#define EEPROM_TSSI_DELTA_CH5_CH6			0x71
#define EEPROM_TSSI_DELTA_CH7_CH8			0x72
#define EEPROM_TSSI_DELTA_CH9_CH10			0x73
#define EEPROM_TSSI_DELTA_CH11_CH12			0x74
#define EEPROM_TSSI_DELTA_CH13_CH14			0x75

#define EEPROM_TSSI_GAIN_ATTENUATION		0x76

#define EEPROM_RSSI_GAIN			0x120

#ifdef RTMP_INTERNAL_TX_ALC
//TSSI delta info is signed number with 4bits
typedef struct
{
    char delta:4;
} TssiDeltaInfo;
#endif /* RTMP_INTERNAL_TX_ALC */

extern REG_PAIR RT5350_RFRegTable[];
extern UCHAR RT5350_NUM_RF_REG_PARMS;
extern REG_PAIR RT5350_BBPRegTable[];
extern UCHAR RT5350_NUM_BBP_REG_PARMS;
extern RTMP_REG_PAIR RT5350_MACRegTable[];
extern UCHAR RT5350_NUM_MAC_REG_PARMS;

VOID RT5350_Init(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT5350MacRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT5350BbpRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID NICInitRT5350RFRegisters(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT5350_RxSensitivityTuning(
	IN struct _RTMP_ADAPTER		*pAd);

UCHAR RT5350_ChipAGCAdjust(
	IN struct _RTMP_ADAPTER		*pAd,
	IN CHAR						Rssi,
	IN UCHAR					OrigR66Value);

VOID RT5350_ChipBBPAdjust(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT5350_ChipSwitchChannel(
	IN struct _RTMP_ADAPTER 	*pAd,
	IN UCHAR					Channel,
	IN BOOLEAN					bScan);

VOID RT5350_AsicInitDesiredTSSITable(
	IN struct _RTMP_ADAPTER		*pAd);

VOID *RT5350_AsicTxAlcTxPwrAdjOverRF(
	IN struct _RTMP_ADAPTER		*pAd,
	IN VOID						*pTxPowerTuningEntrySrc);

VOID RT5350_RTMPSetAGCInitValue(
	IN struct _RTMP_ADAPTER		*pAd,
	IN UCHAR					BandWidth);

VOID RT5350_AsicTxAlcGetAutoAgcOffset(
	IN struct _RTMP_ADAPTER		*pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1);

VOID RT5350_InitDesiredTSSITable(
	IN struct _RTMP_ADAPTER		*pAd);

VOID RT5350_AsicExtraPowerOverMAC(
	IN struct _RTMP_ADAPTER		*pAd);

#endif // RT5350 //
#endif //__RT5350_H__ //

/* End of rt5350.h */
