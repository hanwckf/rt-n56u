#ifndef __MWDS_H__
#define __MWDS_H__
/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
	mwds.h
 
    Abstract:
    This is MWDS feature used to process those 4-addr of connected APClient or STA.
    
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef MWDS
#include "rtmp_def.h"

enum MWDS_ENTRY {
	MWDS_ENTRY_NONE = 0,
	MWDS_ENTRY_AP,
	MWDS_ENTRY_CLIENT
};

#define IS_ENTRY_MWDS(_x)			((_x)->MWDSEntry != MWDS_ENTRY_NONE)
#define IS_MWDS_OPMODE_APCLI(_x)	((_x)->MWDSEntry == MWDS_ENTRY_AP)
#define IS_MWDS_OPMODE_AP(_x)	 	((_x)->MWDSEntry == MWDS_ENTRY_CLIENT)
#define SET_MWDS_OPMODE_NONE(_x)	((_x)->MWDSEntry = MWDS_ENTRY_NONE)
#define SET_MWDS_OPMODE_APCLI(_x)	((_x)->MWDSEntry = MWDS_ENTRY_AP) 	   /* We operate as MWDS APCLI*/
#define SET_MWDS_OPMODE_AP(_x)		((_x)->MWDSEntry = MWDS_ENTRY_CLIENT)  /* We operate as MWDS AP*/
#define MWDS_SUPPORT(B0)            ((B0)&0x80)
#define IS_MEDIATEK_CLI_ENTRY(B0)            ((B0)&0x10)

typedef struct _MWDS_CONNECT_ENTRY {
	DL_LIST List;
	UCHAR Valid;
	UCHAR wcid;
} MWDS_CONNECT_ENTRY, *PMWDS_CONNECT_ENTRY;

VOID MWDSConnEntryListInit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID MWDSConnEntryListClear(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

BOOLEAN MWDSConnEntryLookupByWCID(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN UCHAR wcid);

BOOLEAN MWDSConnEntryLookupByAddr(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN PUCHAR pMac);

VOID MWDSConnEntryUpdate(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN UCHAR wcid);

VOID MWDSConnEntryDelete(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN UCHAR wcid);

INT MWDSGetConnEntryCount(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

BOOLEAN ISMWDSValid(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR ifIndex);

VOID MWDSProxyEntryDelete(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN PUCHAR pMac);

BOOLEAN MWDSProxyLookup(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN PUCHAR pMac,
	IN BOOLEAN bUpdateAliveTime,
	OUT UCHAR *pWcid);

VOID MWDSProxyTabUpdate(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN UCHAR wcid,
	IN PUCHAR pMac,
	IN UINT32 ARPSenderIP);

VOID MWDSProxyTabMaintain(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

void MWDSSendClonePacket(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN PNDIS_PACKET pPacket,
	IN PUCHAR pExcludeMac);

VOID MWDSAPPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry);

VOID MWDSAPPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry);

#ifdef APCLI_SUPPORT
VOID MWDSAPCliPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN MAC_TABLE_ENTRY *pEntry);

VOID MWDSAPCliPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN MAC_TABLE_ENTRY *pEntry);
#endif /* APCLI_SUPPORT */
INT MWDSEnable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevOpen);

INT MWDSDisable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevClose);

INT MWDSAPUP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

INT MWDSAPDown(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);


INT Set_Ap_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_ApCli_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_Ap_MWDS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_ApCli_MWDS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

INT Set_APProxy_Status_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg);

VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer);

#endif /* MWDS */
#endif /* __MWDS_H__*/

