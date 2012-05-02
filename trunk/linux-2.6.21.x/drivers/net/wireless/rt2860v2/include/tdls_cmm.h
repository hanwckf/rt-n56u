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
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#ifndef __TDLS_CMM_H
#define __TDLS_CMM_H

#define TDLS_MAX_BLACL_LIST_SZIE	64
#define	MAX_NUM_OF_TDLS_ENTRY        4
#define TDLS_ENTRY_AGEOUT_TIME		30		// unit: sec
#define TDLS_MAX_SEARCH_TABLE_SZIE	64

#define TDLS_SEARCH_ENTRY_AGEOUT 5000  /* seconds */
#define TDLS_SEARCH_ENTRY_AGEOUT_LIMIT	600000  /* seconds */
#define TDLS_SEARCH_POOL_SIZE 64
#define TDLS_SEARCH_HASH_TAB_SIZE 32  /* the legth of hash table must be power of 2. */

#define TDLS_ENTRY_POOL_SIZE 8
#define TDLS_ENTRY_HASH_TAB_SIZE 4  /* the legth of hash table must be power of 2. */

typedef struct _TDLS_SEARCH_ENTRY {
	struct _TDLS_SEARCH_ENTRY * pNext;
	ULONG	LastRefTime;
	UCHAR	RetryCount;
	UCHAR	InitRefTime;
	UCHAR	Addr[MAC_ADDR_LEN];
} TDLS_SEARCH_ENTRY, *PTDLS_SEARCH_ENTRY;

// TDLS Settings for each link entry
typedef struct _RT_802_11_TDLS {
	USHORT			TimeOut;		// unit: second , set by UI
	USHORT			CountDownTimer;	// unit: second , used by driver only
	UCHAR			MacAddr[MAC_ADDR_LEN];		// set by UI
	UCHAR			Status;			// 0: none , 1: wait result, 2: wait add , 3: connected
	BOOLEAN			Valid;			// 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link
	// The above parameters are the same as RT_802_11_DLS_UI

	UCHAR			Token;			// Dialog token
	RALINK_TIMER_STRUCT	Timer;			// Use to time out while handshake
	BOOLEAN			bInitiator;		// TRUE: I am TDLS Initiator STA, FALSE: I am TDLS Responder STA
	UCHAR			MacTabMatchWCID;
	PVOID			pAd;
	USHORT			CapabilityInfo;

	// Copy supported rate from desired Initiator. We are trying to match
	// Initiator's supported and extended rate settings.
	UCHAR			SupRateLen;
	UCHAR			ExtRateLen;
	UCHAR			SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			ExtRate[MAX_LEN_OF_SUPPORTED_RATES];

	// For TPK handshake
	UCHAR			ANonce[32];	// Generated in Message 1, random variable
	UCHAR			SNonce[32];	// Generated in Message 2, random variable
	ULONG			KeyLifetime;	//  Use type= 'Key Lifetime Interval' unit: Seconds, min lifetime = 300 seconds
	UCHAR			TPK[LEN_PMK];	// TPK-KCK(16 bytes) for MIC + TPK-TP (16 bytes) for data
	UCHAR			TPKName[LEN_PMK_NAME];
	
	// For QOS
	BOOLEAN			bWmmCapable;	// WMM capable of the peer TDLS
	UCHAR			QosCapability; // QOS capability of the current connecting Initiator
	EDCA_PARM		EdcaParm;		// EDCA parameters of the Initiator

	// Features
	UCHAR					HtCapabilityLen;
	HT_CAPABILITY_IE		HtCapability;
	UCHAR					TdlsExtCapLen;
	EXT_CAP_INFO_ELEMENT	TdlsExtCap;
} RT_802_11_TDLS, *PRT_802_11_TDLS;
#endif /* __TDLS_CMM_H */
#endif // DOT11Z_TDLS_SUPPORT //

