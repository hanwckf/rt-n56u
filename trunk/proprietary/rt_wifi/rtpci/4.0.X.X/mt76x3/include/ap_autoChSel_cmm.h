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

    Abstract:


 */


#ifndef __AUTOCHSELECT_CMM_H__
#define __AUTOCHSELECT_CMM_H__

#define RSSI_TO_DBM_OFFSET 120 /* RSSI-115 = dBm */
#ifdef SUPPORT_ACS_ALL_CHANNEL_RANK
#define ACS_DIRTINESS_RSSI_STRONG   (-50) /* dBm */
#define ACS_DIRTINESS_RSSI_WEAK     (-80) /* dBm */
#define ACS_FALSECCA_THRESHOLD      (100)
#endif


typedef struct {
	ULONG dirtyness[MAX_NUM_OF_CHANNELS+1];
	ULONG ApCnt[MAX_NUM_OF_CHANNELS+1];
	UINT32 FalseCCA[MAX_NUM_OF_CHANNELS+1];
	BOOLEAN SkipList[MAX_NUM_OF_CHANNELS+1];
#ifdef CUSTOMER_DCC_FEATURE
	UINT32	ChannelNo;
	BOOLEAN GetChannelInfo;
#endif
	UINT32 chanbusytime[MAX_NUM_OF_CHANNELS+1]; /* QLOAD ALARM */
	BOOLEAN IsABand;
} CHANNELINFO, *PCHANNELINFO;

typedef struct {
	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR Channel;
	UCHAR ExtChOffset;
	CHAR Rssi;
} BSSENTRY, *PBSSENTRY;

typedef struct {
	UCHAR BssNr;
	BSSENTRY BssEntry[MAX_LEN_OF_BSS_TABLE];	
} BSSINFO, *PBSSINFO;

#ifdef SUPPORT_ACS_ALL_CHANNEL_RANK
typedef struct {
    UCHAR ch;
    ULONG dirtyness;
    ULONG falseCCA;
} ACS_SORT_ENTRY;
#endif

typedef enum ChannelSelAlg
{
	ChannelAlgRandom, /*use by Dfs */
	ChannelAlgApCnt,
	ChannelAlgCCA,
	ChannelAlgBusyTime,
	ChannelAlgCombined
} ChannelSel_Alg;

#endif /* __AUTOCHSELECT_CMM_H__ */

