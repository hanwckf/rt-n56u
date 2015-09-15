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
    rtmp.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    2002-08-01    created
    James Tan   2002-09-06    modified (Revise NTCRegTable)
    John Chang  2004-09-06    modified for RT2600     
*/
#ifndef __RTMP_H__
#define __RTMP_H__

#include "link_list.h"
#include "spectrum_def.h"

#include "rtmp_dot11.h"
#include "wpa_cmm.h"

#ifdef CONFIG_AP_SUPPORT
#include "ap_autoChSel_cmm.h"
#endif // CONFIG_AP_SUPPORT //

#include "wsc.h"

#ifdef MAT_SUPPORT
#include "mat.h"
#endif // MAT_SUPPORT //


#ifdef WAPI_SUPPORT
#include "wapi_def.h"
#endif // WAPI_SUPPORT //

#include "rtmp_chip.h"





#ifdef CLIENT_WDS
#include "client_wds_cmm.h"
#endif // CLIENT_WDS //




#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#include	"frq_cal.h"
#endif // RTMP_FREQ_CALIBRATION_SUPPORT //

typedef struct _RTMP_ADAPTER		RTMP_ADAPTER;
typedef struct _RTMP_ADAPTER		*PRTMP_ADAPTER;

typedef struct _RTMP_CHIP_OP_ RTMP_CHIP_OP;
typedef struct _RTMP_CHIP_CAP_ RTMP_CHIP_CAP;


//#define DBG		1

//#define DBG_DIAGNOSE		1


//+++Add by shiang for merge MiniportMMRequest() and MiniportDataMMRequest() into one function
#define MAX_DATAMM_RETRY	3
#define MGMT_USE_QUEUE_FLAG	0x80
//---Add by shiang for merge MiniportMMRequest() and MiniportDataMMRequest() into one function
//
// The number of channels for per-channel Tx power offset
//
#define NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET      14
#define	MAXSEQ		(0xFFF)

#define MAX_MCS_SET 16 /* From MCS 0 ~ MCS 15 */

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)	if(_pAd->OpMode == OPMODE_AP)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)	if(_pAd->OpMode == OPMODE_STA)
#else
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)
#endif

#define MAX_TXPOWER_ARRAY_SIZE	5

extern  unsigned char   CISCO_OUI[];
extern  UCHAR	BaSizeArray[4];

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern ULONG BIT32[32];
extern UCHAR BIT8[8];
extern char* CipherName[];
extern UCHAR	 RxwiMCSToOfdmRate[12];
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR TPID[];
extern UCHAR APPLE_TALK[2];
extern UCHAR	 OfdmRateToRxwiMCS[];
extern UCHAR MapUserPriorityToAccessCategory[8];

extern USHORT RateUpPER[];
extern USHORT RateDownPER[];
extern UCHAR  Phy11BNextRateDownward[];
extern UCHAR  Phy11BNextRateUpward[];
extern UCHAR  Phy11BGNextRateDownward[];
extern UCHAR  Phy11BGNextRateUpward[];
extern UCHAR  Phy11ANextRateDownward[];
extern UCHAR  Phy11ANextRateUpward[];
extern unsigned char  RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR  CipherSuiteWpaNoneTkip[];
extern UCHAR  CipherSuiteWpaNoneTkipLen;

extern UCHAR  CipherSuiteWpaNoneAes[];
extern UCHAR  CipherSuiteWpaNoneAesLen;

extern UCHAR  SsidIe;
extern UCHAR  SupRateIe;
extern UCHAR  ExtRateIe;

#ifdef DOT11_N_SUPPORT
extern UCHAR  HtCapIe;
extern UCHAR  AddHtInfoIe;
extern UCHAR  NewExtChanIe;
extern UCHAR  BssCoexistIe;
extern UCHAR  ExtHtCapIe;
#endif // DOT11_N_SUPPORT //
extern UCHAR  ExtCapIe;

extern UCHAR  ErpIe;
extern UCHAR  DsIe;
extern UCHAR  TimIe;
extern UCHAR  WpaIe;
extern UCHAR  Wpa2Ie;
extern UCHAR  IbssIe;
extern UCHAR  WapiIe;

extern UCHAR  WPA_OUI[];
extern UCHAR  RSN_OUI[];
extern UCHAR  WAPI_OUI[];
extern UCHAR  WME_INFO_ELEM[];
extern UCHAR  WME_PARM_ELEM[];
extern UCHAR  RALINK_OUI[];
extern UCHAR  PowerConstraintIE[];


extern UCHAR  RateSwitchTable[];
extern UCHAR  RateSwitchTable11B[];
extern UCHAR  RateSwitchTable11G[];
extern UCHAR  RateSwitchTable11BG[];

#ifdef DOT11_N_SUPPORT
extern UCHAR  RateSwitchTable11BGN1S[];
extern UCHAR  RateSwitchTable11BGN2S[];
extern UCHAR  RateSwitchTable11BGN2SForABand[];
extern UCHAR  RateSwitchTable11N1S[];
extern UCHAR  RateSwitchTable11N2S[];
extern UCHAR  RateSwitchTable11N3S[];
extern UCHAR  RateSwitchTable11N3SReplacement[];
extern UCHAR  RateSwitchTable11N2SForABand[];
extern UCHAR  RateSwitchTable11BGN3S[];
extern UCHAR  RateSwitchTable11BGN3SForABand[];

#endif // DOT11_N_SUPPORT //


#ifdef RALINK_ATE
typedef	struct _ATE_INFO {
	UCHAR	Mode;
	BOOLEAN	PassiveMode;
	CHAR	TxPower0;
	CHAR	TxPower1;
	CHAR    TxAntennaSel;
	CHAR    RxAntennaSel;
	TXWI_STRUC  TxWI; 	  // TXWI
	USHORT	QID;
	UCHAR	Addr1[MAC_ADDR_LEN];
	UCHAR	Addr2[MAC_ADDR_LEN];
	UCHAR	Addr3[MAC_ADDR_LEN];
	UCHAR	Channel;
	UCHAR	Payload; // Payload pattern
	UINT32	TxLength;
	UINT32	TxCount;
	UINT32	TxDoneCount; // Tx DMA Done
	UINT32	RFFreqOffset;
#ifdef RT53xx
	UINT32   PreRFXCodeValue;
#endif // RT53xx //
	UINT32	IPG;
	BOOLEAN	bRxFER;		// Show Rx Frame Error Rate
	BOOLEAN	bQATxStart; // Have compiled QA in and use it to ATE tx.
	BOOLEAN	bQARxStart;	// Have compiled QA in and use it to ATE rx.
	BOOLEAN	bAutoTxAlc;	// Set Auto Tx Alc
#ifdef RTMP_MAC_PCI
	BOOLEAN	bFWLoading;	// Reload firmware when ATE is done.
#endif // RTMP_MAC_PCI //
	UINT32	RxTotalCnt;
	UINT32	RxCntPerSec;
#ifdef IQ_CAL_SUPPORT
	BOOLEAN bIQCompEnable; /* Enable I/Q compensation */
#endif /* IQ_CAL_SUPPORT */
	CHAR	LastSNR0;             // last received SNR
	CHAR    LastSNR1;             // last received SNR for 2nd  antenna
	CHAR    LastRssi0;            // last received RSSI
	CHAR    LastRssi1;            // last received RSSI for 2nd  antenna
	CHAR    LastRssi2;            // last received RSSI for 3rd  antenna
	CHAR    AvgRssi0;             // last 8 frames' average RSSI
	CHAR    AvgRssi1;             // last 8 frames' average RSSI
	CHAR    AvgRssi2;             // last 8 frames' average RSSI
	SHORT   AvgRssi0X8;           // sum of last 8 frames' RSSI
	SHORT   AvgRssi1X8;           // sum of last 8 frames' RSSI
	SHORT   AvgRssi2X8;           // sum of last 8 frames' RSSI

	UINT32	NumOfAvgRssiSample;


#ifdef RALINK_QA
	// Tx frame
	USHORT		HLen; // Header Length
	USHORT		PLen; // Pattern Length
	UCHAR 		Header[32]; // Header buffer
	UCHAR		Pattern[32]; // Pattern buffer
	USHORT		DLen; // Data Length
	USHORT		seq;
	UINT32		CID;
	RTMP_OS_PID	AtePid;
	// counters
	UINT32		U2M;
	UINT32		OtherData;
	UINT32		Beacon;
	UINT32		OtherCount;
	UINT32		TxAc0;
	UINT32		TxAc1;
	UINT32		TxAc2;
	UINT32		TxAc3;
	UINT32		TxHCCA;
	UINT32		TxMgmt;
	UINT32		RSSI0;
	UINT32		RSSI1;
	UINT32		RSSI2;
	UINT32		SNR0;
	UINT32		SNR1;
	// TxStatus : 0 --> task is idle, 1 --> task is running
	UCHAR		TxStatus;
#endif // RALINK_QA //
}	ATE_INFO, *PATE_INFO;

#ifdef RALINK_QA
struct ate_racfghdr {
 	UINT32		magic_no;
	USHORT		command_type;
	USHORT		command_id;
	USHORT		length;
	USHORT		sequence;
	USHORT		status;
	UCHAR		data[2046];
}  __attribute__((packed));
#endif // RALINK_QA //
#endif // RALINK_ATE //


typedef struct 	_RSSI_SAMPLE {
	CHAR			LastRssi0;             // last received RSSI
	CHAR			LastRssi1;             // last received RSSI
	CHAR			LastRssi2;             // last received RSSI
	CHAR			AvgRssi0;
	CHAR			AvgRssi1;
	CHAR			AvgRssi2;
	SHORT			AvgRssi0X8;
	SHORT			AvgRssi1X8;
	SHORT			AvgRssi2X8;
	CHAR			LastSnr0;             
	CHAR			LastSnr1;             
	CHAR			LastSnr2;
	CHAR			AvgSnr0;
	CHAR			AvgSnr1;
	CHAR			AvgSnr2;
	SHORT			AvgSnr0X8;
	SHORT			AvgSnr1X8;
	SHORT			AvgSnr2X8;
	CHAR			LastNoiseLevel0;             
	CHAR			LastNoiseLevel1;             
	CHAR			LastNoiseLevel2;  	
} RSSI_SAMPLE;

//
//  Queue structure and macros
//
#define InitializeQueueHeader(QueueHeader)              \
{                                                       \
	(QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
	(QueueHeader)->Number = 0;                          \
}

#define RemoveHeadQueue(QueueHeader)                \
(QueueHeader)->Head;                                \
{                                                   \
	PQUEUE_ENTRY pNext;                             \
	if ((QueueHeader)->Head != NULL)				\
	{												\
		pNext = (QueueHeader)->Head->Next;          \
		(QueueHeader)->Head->Next = NULL;		\
		(QueueHeader)->Head = pNext;                \
		if (pNext == NULL)                          \
			(QueueHeader)->Tail = NULL;             \
		(QueueHeader)->Number--;                    \
	}												\
}

#define InsertHeadQueue(QueueHeader, QueueEntry)            \
{                                                           \
		((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
		if ((QueueHeader)->Tail == NULL)                        \
			(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
		(QueueHeader)->Number++;                                \
}

#define InsertTailQueue(QueueHeader, QueueEntry)				\
{                                                               \
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
	if ((QueueHeader)->Tail)                                    \
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
	else                                                        \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
	(QueueHeader)->Number++;                                    \
}

#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)			\
{																		\
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;							\
	if ((QueueHeader)->Tail)											\
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);			\
	else																\
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);				\
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);					\
	(QueueHeader)->Number++;											\
}

#ifdef WMM_ACM_SUPPORT

#undef InsertTailQueueAc


/* we need to check if the packet can be put to the queue based on ACM */
#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)				\
{																			\
	PQUEUE_HEADER __QueueHeaderNew;											\
	ACM_FUNC_STATUS __Status = ACM_RTN_NO_ACM;								\
	UINT32 __QueueType;														\
	BOOLEAN __FlgIsPktFreed = FALSE;										\
																			\
	ACMR_PKT_MARK_MIN_PHY_MODE(QUEUE_ENTRY_TO_PACKET(QueueEntry), ACMR_PHY_NONE);\
	RTMP_SET_PACKET_TX_TIME(QUEUE_ENTRY_TO_PACKET(QueueEntry), 0);			\
		if (ACMP_IsNeedToDoAcm(pAd, pEntry, RTMP_GET_PACKET_UP(				\
								QUEUE_ENTRY_TO_PACKET(QueueEntry))) == TRUE)\
		{																	\
			ACMR_PKT_QOS_TYPE_SET(QUEUE_ENTRY_TO_PACKET(QueueEntry), 0);	\
			__Status = ACMP_DataPacketQueue(pAd, pEntry, 					\
					QUEUE_ENTRY_TO_PACKET(QueueEntry), 0, &__QueueType);	\
																			\
			if ((__Status == ACM_RTN_FAIL) ||									\
				((__Status == ACM_RTN_OK) &&									\
				(pAd->TxSwQueue[__QueueType].Number >= MAX_PACKETS_IN_QUEUE)))	\
			{																	\
				__FlgIsPktFreed = TRUE;										\
																			\
				if ((__Status == ACM_RTN_OK) &&								\
					(__QueueType == ACMR_QID_AC_BE)	&&						\
					(ACMP_BE_QueueFullHandle(pAd,							\
											pEntry,							\
						QUEUE_ENTRY_TO_PACKET(QueueEntry)) == ACM_RTN_OK))	\
				{															\
					__FlgIsPktFreed = FALSE;								\
				}															\
																			\
				if (__FlgIsPktFreed == TRUE)								\
				{															\
					RELEASE_NDIS_PACKET(pAd,								\
										QUEUE_ENTRY_TO_PACKET(QueueEntry),	\
										NDIS_STATUS_FAILURE);				\
				}															\
			}																\
		}																	\
	if (__FlgIsPktFreed == FALSE)											\
	{																		\
		if (__Status == ACM_RTN_NO_ACM)										\
			__QueueHeaderNew = QueueHeader;									\
		else																\
			__QueueHeaderNew = &pAd->TxSwQueue[__QueueType];				\
		((PQUEUE_ENTRY)QueueEntry)->Next = NULL;							\
		if ((__QueueHeaderNew)->Tail)										\
			(__QueueHeaderNew)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);	\
		else																\
			(__QueueHeaderNew)->Head = (PQUEUE_ENTRY)(QueueEntry);			\
		(__QueueHeaderNew)->Tail = (PQUEUE_ENTRY)(QueueEntry);				\
		(__QueueHeaderNew)->Number++;										\
	}																		\
}
#endif // WMM_ACM_SUPPORT //


//
//  Macros for flag and ref count operations
//
#define RTMP_SET_FLAG(_M, _F)       ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)     ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)        ((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)      (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)     (((_M)->Flags & (_F)) == (_F))
// Macro for power save flag.
#define RTMP_SET_PSFLAG(_M, _F)       ((_M)->PSFlags |= (_F))
#define RTMP_CLEAR_PSFLAG(_M, _F)     ((_M)->PSFlags &= ~(_F))
#define RTMP_CLEAR_PSFLAGS(_M)        ((_M)->PSFlags = 0)
#define RTMP_TEST_PSFLAG(_M, _F)      (((_M)->PSFlags & (_F)) != 0)
#define RTMP_TEST_PSFLAGS(_M, _F)     (((_M)->PSFlags & (_F)) == (_F))

#define OPSTATUS_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)      ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)    ((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F)     (((_pEntry)->ClientStatusFlags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pAd, _F)    ((_pAd)->CommonCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pAd, _F)  ((_pAd)->CommonCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pAd, _F)   (((_pAd)->CommonCfg.PacketFilter & (_F)) != 0)

#define RTMP_SET_MORE_FLAG(_M, _F)       ((_M)->MoreFlags |= (_F))
#define RTMP_TEST_MORE_FLAG(_M, _F)      (((_M)->MoreFlags & (_F)) != 0)
#define RTMP_CLEAR_MORE_FLAG(_M, _F)     ((_M)->MoreFlags &= ~(_F))


#define CKIP_KP_ON(_p)				((((_p)->StaCfg.CkipFlag) & 0x10) && ((_p)->StaCfg.bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg.CkipFlag) & 0x08) && ((_p)->StaCfg.bCkipCmicOn == TRUE))


#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}


#ifdef DOT11_N_SUPPORT
// StaActive.SupportedHtPhy.MCSSet is copied from AP beacon.  Don't need to update here.
#define COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
	_pAd->StaActive.SupportedHtPhy.ChannelWidth = _pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;      \
	_pAd->StaActive.SupportedHtPhy.MimoPs = _pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs;      \
	_pAd->StaActive.SupportedHtPhy.GF = _pAd->MlmeAux.HtCapability.HtCapInfo.GF;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor20 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor40 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40;      \
	_pAd->StaActive.SupportedHtPhy.TxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.RxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.ExtChanOffset = _pAd->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset;      \
	_pAd->StaActive.SupportedHtPhy.RecomWidth = _pAd->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth;      \
	_pAd->StaActive.SupportedHtPhy.OperaionMode = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;      \
	_pAd->StaActive.SupportedHtPhy.NonGfPresent = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent;      \
	NdisMoveMemory((_pAd)->MacTab.Content[BSSID_WCID].HTCapability.MCSSet, (_pAd)->StaActive.SupportedPhyInfo.MCSSet, sizeof(UCHAR) * 16);\
}

#define COPY_AP_HTSETTINGS_FROM_BEACON(_pAd, _pHtCapability)                                 \
{                                                                                       \
	_pAd->MacTab.Content[BSSID_WCID].AMsduSize = (UCHAR)(_pHtCapability->HtCapInfo.AMsduSize);	\
	_pAd->MacTab.Content[BSSID_WCID].MmpsMode= (UCHAR)(_pHtCapability->HtCapInfo.MimoPs);	\
	_pAd->MacTab.Content[BSSID_WCID].MaxRAmpduFactor = (UCHAR)(_pHtCapability->HtCapParm.MaxRAmpduFactor);	\
}
#endif // DOT11_N_SUPPORT //

//
// MACRO for 32-bit PCI register read / write
//
// Usage : RTMP_IO_READ32(
//              PRTMP_ADAPTER pAd,
//              ULONG Register_Offset,
//              PULONG  pValue)
//
//         RTMP_IO_WRITE32(
//              PRTMP_ADAPTER pAd,
//              ULONG Register_Offset,
//              ULONG Value)
//


//
// Common fragment list structure -  Identical to the scatter gather frag list structure
//
//#define RTMP_SCATTER_GATHER_ELEMENT         SCATTER_GATHER_ELEMENT 
//#define PRTMP_SCATTER_GATHER_ELEMENT        PSCATTER_GATHER_ELEMENT 
#define NIC_MAX_PHYS_BUF_COUNT              8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
    PVOID		Address;
    ULONG		Length;
    PULONG		Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;


typedef struct _RTMP_SCATTER_GATHER_LIST {
    ULONG  NumberOfElements;
    PULONG Reserved;
    RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;


//
//  Some utility macros
//
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

#define GET_LNA_GAIN(_pAd)	((_pAd->LatchRfRegs.Channel <= 14) ? (_pAd->BLNAGain) : ((_pAd->LatchRfRegs.Channel <= 64) ? (_pAd->ALNAGain0) : ((_pAd->LatchRfRegs.Channel <= 128) ? (_pAd->ALNAGain1) : (_pAd->ALNAGain2))))

#define INC_COUNTER64(Val)          (Val.QuadPart++)

#define INFRA_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_ON))
#define ADHOC_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))
#define MONITOR_ON(_p)              (((_p)->StaCfg.BssType) == BSS_MONITOR)
#define IDLE_ON(_p)                 (!INFRA_ON(_p) && !ADHOC_ON(_p))

// Check LEAP & CCKM flags
#define LEAP_ON(_p)                 (((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP)
#define LEAP_CCKM_ON(_p)            ((((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP) && ((_p)->StaCfg.LeapAuthInfo.CCKM == TRUE))

// if orginal Ethernet frame contains no LLC/SNAP, then an extra LLC/SNAP encap is required 
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(_pBufVA, _pExtraLlcSnapEncap)		\
{																\
	if (((*(_pBufVA + 12) << 8) + *(_pBufVA + 13)) > 1500)		\
	{															\
		_pExtraLlcSnapEncap = SNAP_802_1H;						\
		if (NdisEqualMemory(IPX, _pBufVA + 12, 2) || 			\
			NdisEqualMemory(APPLE_TALK, _pBufVA + 12, 2))		\
		{														\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		}														\
	}															\
	else														\
	{															\
		_pExtraLlcSnapEncap = NULL;								\
	}															\
}

// New Define for new Tx Path.
#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(_pBufVA, _pExtraLlcSnapEncap)	\
{																\
	if (((*(_pBufVA) << 8) + *(_pBufVA + 1)) > 1500)			\
	{															\
		_pExtraLlcSnapEncap = SNAP_802_1H;						\
		if (NdisEqualMemory(IPX, _pBufVA, 2) || 				\
			NdisEqualMemory(APPLE_TALK, _pBufVA, 2))			\
		{														\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		}														\
	}															\
	else														\
	{															\
		_pExtraLlcSnapEncap = NULL;								\
	}															\
}


#define MAKE_802_3_HEADER(_p, _pMac1, _pMac2, _pType)                   \
{                                                                       \
    NdisMoveMemory(_p, _pMac1, MAC_ADDR_LEN);                           \
    NdisMoveMemory((_p + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);          \
    NdisMoveMemory((_p + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE); \
}

// if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel), keep it that way.
// else if the received frame is LLC/SNAP-encaped IPX or APPLETALK, preserve the LLC/SNAP field 
// else remove the LLC/SNAP field from the result Ethernet frame
// Patch for WHQL only, which did not turn on Netbios but use IPX within its payload
// Note:
//     _pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO
//     _pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed
#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)      \
{                                                                       \
    char LLC_Len[2];                                                    \
                                                                        \
    _pRemovedLLCSNAP = NULL;                                            \
    if (NdisEqualMemory(SNAP_802_1H, _pData, 6)  ||                     \
        NdisEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6))                 \
    {                                                                   \
        PUCHAR pProto = _pData + 6;                                     \
                                                                        \
        if ((NdisEqualMemory(IPX, pProto, 2) || NdisEqualMemory(APPLE_TALK, pProto, 2)) &&  \
            NdisEqualMemory(SNAP_802_1H, _pData, 6))                    \
        {                                                               \
            LLC_Len[0] = (UCHAR)(_DataSize >> 8);                       \
            LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));                \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);          \
        }                                                               \
        else                                                            \
        {                                                               \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);           \
            _pRemovedLLCSNAP = _pData;                                  \
            _DataSize -= LENGTH_802_1_H;                                \
            _pData += LENGTH_802_1_H;                                   \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        LLC_Len[0] = (UCHAR)(_DataSize >> 8);                           \
        LLC_Len[1] = (UCHAR)(_DataSize & (256 - 1));                    \
        MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);              \
    }                                                                   \
}


// Enqueue this frame to MLME engine
// We need to enqueue the whole frame because MLME need to pass data type
// information from 802.11 header
#ifdef RTMP_MAC_PCI
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _MinSNR)        \
{                                                                                       \
    UINT32 High32TSF, Low32TSF;                                                          \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW1, &High32TSF);                                       \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW0, &Low32TSF);                                        \
    MlmeEnqueueForRecv(_pAd, Wcid, High32TSF, Low32TSF, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2,_FrameSize, _pFrame, (UCHAR)_MinSNR);   \
}
#endif // RTMP_MAC_PCI //

#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)    ((len1==len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))

//
// Check if it is Japan W53(ch52,56,60,64) channel.
//
#define JapanChannelCheck(channel)  ((channel == 52) || (channel == 56) || (channel == 60) || (channel == 64))




//
//  Data buffer for DMA operation, the buffer must be contiguous physical memory
//  Both DMA to / from CPU use the same structure.
//
typedef struct  _RTMP_DMABUF
{
	ULONG                   AllocSize;
	PVOID                   AllocVa;            // TxBuf virtual address
	NDIS_PHYSICAL_ADDRESS   AllocPa;            // TxBuf physical address
} RTMP_DMABUF, *PRTMP_DMABUF;


//
// Control block (Descriptor) for all ring descriptor DMA operation, buffer must be 
// contiguous physical memory. NDIS_PACKET stored the binding Rx packet descriptor
// which won't be released, driver has to wait until upper layer return the packet
// before giveing up this rx ring descriptor to ASIC. NDIS_BUFFER is assocaited pair
// to describe the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor
// which driver should ACK upper layer when the tx is physically done or failed.
//
typedef struct _RTMP_DMACB
{
	ULONG                   AllocSize;          // Control block size
	PVOID                   AllocVa;            // Control block virtual address
	NDIS_PHYSICAL_ADDRESS   AllocPa;            // Control block physical address
	PNDIS_PACKET pNdisPacket;
	PNDIS_PACKET pNextNdisPacket;

	RTMP_DMABUF             DmaBuf;             // Associated DMA buffer structure
} RTMP_DMACB, *PRTMP_DMACB;


typedef struct _RTMP_TX_RING
{
	RTMP_DMACB  Cell[TX_RING_SIZE];
	UINT32		TxCpuIdx;	
	UINT32		TxDmaIdx;   
	UINT32		TxSwFreeIdx; 	// software next free tx index
} RTMP_TX_RING, *PRTMP_TX_RING;

typedef struct _RTMP_RX_RING
{
	RTMP_DMACB  Cell[RX_RING_SIZE];
	UINT32		RxCpuIdx;
	UINT32		RxDmaIdx;
	INT32		RxSwReadIdx; 	// software next read index
} RTMP_RX_RING, *PRTMP_RX_RING;

typedef struct _RTMP_MGMT_RING
{
	RTMP_DMACB  Cell[MGMT_RING_SIZE];
	UINT32		TxCpuIdx;
	UINT32		TxDmaIdx;
	UINT32		TxSwFreeIdx; // software next free tx index
} RTMP_MGMT_RING, *PRTMP_MGMT_RING;


//
//  Statistic counter structure
//
typedef struct _COUNTER_802_3
{
	// General Stats
	ULONG       GoodTransmits;
	ULONG       GoodReceives;
	ULONG       TxErrors;
	ULONG       RxErrors;
	ULONG       RxNoBuffer;

	// Ethernet Stats
	ULONG       RcvAlignmentErrors;
	ULONG       OneCollision;
	ULONG       MoreCollisions;

} COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG           Length;
//	LARGE_INTEGER   LastTransmittedFragmentCount;
	LARGE_INTEGER   TransmittedFragmentCount;
	LARGE_INTEGER   MulticastTransmittedFrameCount;
	LARGE_INTEGER   FailedCount;
	LARGE_INTEGER   RetryCount;
	LARGE_INTEGER   MultipleRetryCount;
	LARGE_INTEGER   RTSSuccessCount;
	LARGE_INTEGER   RTSFailureCount;
	LARGE_INTEGER   ACKFailureCount;
	LARGE_INTEGER   FrameDuplicateCount;
	LARGE_INTEGER   ReceivedFragmentCount;
	LARGE_INTEGER   MulticastReceivedFrameCount;
	LARGE_INTEGER   FCSErrorCount;
} COUNTER_802_11, *PCOUNTER_802_11;

typedef struct _COUNTER_RALINK {

	UINT32			OneSecStart; /* for one sec count clear use */
	UINT32          OneSecBeaconSentCnt;
	UINT32          OneSecFalseCCACnt;      // CCA error count, for debug purpose, might move to global counter
	UINT32          OneSecRxFcsErrCnt;      // CRC error
	UINT32          OneSecRxOkCnt;          // RX without error
	UINT32          OneSecTxFailCount;
	UINT32          OneSecTxNoRetryOkCount;
	UINT32          OneSecTxRetryOkCount;
	UINT32          OneSecRxOkDataCnt;      // unicast-to-me DATA frame count
	UINT32          OneSecTransmittedByteCount;   // both successful and failure, used to calculate TX throughput

	ULONG           OneSecOsTxCount[NUM_OF_TX_RING];
	ULONG           OneSecDmaDoneCount[NUM_OF_TX_RING];
	UINT32          OneSecTxDoneCount;
	ULONG           OneSecRxCount;
	UINT32          OneSecReceivedByteCount;
	UINT32          OneSecTxAggregationCount;
	UINT32          OneSecRxAggregationCount;
	UINT32			OneSecEnd; /* for one sec count clear use */

	ULONG           TransmittedByteCount;   // both successful and failure, used to calculate TX throughput
	ULONG           ReceivedByteCount;      // both CRC okay and CRC error, used to calculate RX throughput
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
	ULONG		LastReceivedByteCount;
#endif // defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) //
	ULONG           BadCQIAutoRecoveryCount;
	ULONG           PoorCQIRoamingCount;
	ULONG           MgmtRingFullCount;
	ULONG           RxCountSinceLastNULL;
	ULONG           RxCount;
	ULONG           KickTxCount;
	LARGE_INTEGER   RealFcsErrCount;
	ULONG           PendingNdisPacketCount;

//	ULONG           OneSecOsTxCount[NUM_OF_TX_RING];
//	ULONG           OneSecDmaDoneCount[NUM_OF_TX_RING];
//	UINT32          OneSecTxDoneCount;
//	ULONG           OneSecRxCount;
//	UINT32          OneSecTxAggregationCount;
//	UINT32          OneSecRxAggregationCount;
//	UINT32          OneSecReceivedByteCount;
//	UINT32   		OneSecFrameDuplicateCount;

//	UINT32          OneSecTransmittedByteCount;   // both successful and failure, used to calculate TX throughput
//	UINT32          OneSecTxNoRetryOkCount;
//	UINT32          OneSecTxRetryOkCount;
//	UINT32          OneSecTxFailCount;
//	UINT32          OneSecFalseCCACnt;      // CCA error count, for debug purpose, might move to global counter
//	UINT32          OneSecRxOkCnt;          // RX without error
//	UINT32          OneSecRxOkDataCnt;      // unicast-to-me DATA frame count
//	UINT32          OneSecRxFcsErrCnt;      // CRC error
//	UINT32          OneSecBeaconSentCnt;

	UINT32          LastOneSecTotalTxCount; // OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount
	UINT32          LastOneSecRxOkDataCnt;  // OneSecRxOkDataCnt
	ULONG		DuplicateRcv;
	ULONG		TxAggCount;
	ULONG		TxNonAggCount;
	ULONG		TxAgg1MPDUCount;
	ULONG		TxAgg2MPDUCount;
	ULONG		TxAgg3MPDUCount;
	ULONG		TxAgg4MPDUCount;
	ULONG		TxAgg5MPDUCount;
	ULONG		TxAgg6MPDUCount;
	ULONG		TxAgg7MPDUCount;
	ULONG		TxAgg8MPDUCount;
	ULONG		TxAgg9MPDUCount;
	ULONG		TxAgg10MPDUCount;
	ULONG		TxAgg11MPDUCount;
	ULONG		TxAgg12MPDUCount;
	ULONG		TxAgg13MPDUCount;
	ULONG		TxAgg14MPDUCount;
	ULONG		TxAgg15MPDUCount;
	ULONG		TxAgg16MPDUCount;

	LARGE_INTEGER       TransmittedOctetsInAMSDU; 
	LARGE_INTEGER       TransmittedAMSDUCount; 
	LARGE_INTEGER       ReceivedOctesInAMSDUCount; 
	LARGE_INTEGER       ReceivedAMSDUCount; 
	LARGE_INTEGER       TransmittedAMPDUCount;
	LARGE_INTEGER       TransmittedMPDUsInAMPDUCount;
	LARGE_INTEGER       TransmittedOctetsInAMPDUCount; 		
	LARGE_INTEGER       MPDUInReceivedAMPDUCount;
} COUNTER_RALINK, *PCOUNTER_RALINK;


typedef struct _COUNTER_DRS {
	// to record the each TX rate's quality. 0 is best, the bigger the worse.
	USHORT          TxQuality[MAX_STEP_OF_TX_RATE_SWITCH];
	UCHAR           PER[MAX_STEP_OF_TX_RATE_SWITCH];
	UCHAR           TxRateUpPenalty;      // extra # of second penalty due to last unstable condition
	ULONG           CurrTxRateStableTime; // # of second in current TX rate
	//BOOLEAN         fNoisyEnvironment;
	BOOLEAN         fLastSecAccordingRSSI;
	UCHAR           LastSecTxRateChangeAction; // 0: no change, 1:rate UP, 2:rate down
	UCHAR			LastTimeTxRateChangeAction; //Keep last time value of LastSecTxRateChangeAction
	ULONG			LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;


/***************************************************************************
  *	security key related data structure
  **************************************************************************/

// structure to define WPA Group Key Rekey Interval
typedef struct GNU_PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;          // mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based
	ULONG ReKeyInterval;        // time-based: seconds, packet-based: kilo-packets
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;



typedef struct {
	UCHAR        Addr[MAC_ADDR_LEN];
	UCHAR        ErrorCode[2];  //00 01-Invalid authentication type
							//00 02-Authentication timeout
							//00 03-Challenge from AP failed
							//00 04-Challenge to AP failed
	BOOLEAN      Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UCHAR               RogueApNr;
	ROGUEAP_ENTRY       RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

/*
  *	Fragment Frame structure
  */
typedef struct  _FRAGMENT_FRAME {
	PNDIS_PACKET    pFragPacket;
	ULONG       RxSize;
	USHORT      Sequence;
	USHORT      LastFrag;
	ULONG       Flags;          // Some extra frame information. bit 0: LLC presented
} FRAGMENT_FRAME, *PFRAGMENT_FRAME;


//
// Packet information for NdisQueryPacket
//
typedef struct  _PACKET_INFO    {
	UINT            PhysicalBufferCount;    // Physical breaks of buffer descripor chained
	UINT            BufferCount ;           // Number of Buffer descriptor chained
	UINT            TotalPacketLength ;     // Self explained
	PNDIS_BUFFER    pFirstBuffer;           // Pointer to first buffer descriptor
} PACKET_INFO, *PPACKET_INFO;

//
// Tkip Key structure which RC4 key & MIC calculation
//
typedef struct  _TKIP_KEY_INFO  {
	UINT        nBytesInM;  // # bytes in M for MICKEY
	ULONG       IV16;
	ULONG       IV32;   
	ULONG       K0;         // for MICKEY Low
	ULONG       K1;         // for MICKEY Hig
	ULONG       L;          // Current state for MICKEY
	ULONG       R;          // Current state for MICKEY
	ULONG       M;          // Message accumulator for MICKEY
	UCHAR       RC4KEY[16];
	UCHAR       MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;


//
// Private / Misc data, counters for driver internal use
//
typedef struct  __PRIVATE_STRUC {
	UINT       SystemResetCnt;         // System reset counter
	UINT       TxRingFullCnt;          // Tx ring full occurrance number
	UINT       PhyRxErrCnt;            // PHY Rx error count, for debug purpose, might move to global counter
	// Variables for WEP encryption / decryption in rtmp_wep.c
	// Tkip stuff
	TKIP_KEY_INFO   Tx;
	TKIP_KEY_INFO   Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;


/***************************************************************************
  *	Channel and BBP related data structures
  **************************************************************************/
// structure to tune BBP R66 (BBP TUNING)
typedef struct _BBP_R66_TUNING {
	BOOLEAN     bEnable;
	USHORT      FalseCcaLowerThreshold;  // default 100
	USHORT      FalseCcaUpperThreshold;  // default 512
	UCHAR       R66Delta;
	UCHAR       R66CurrentValue;
	BOOLEAN		R66LowerUpperSelect; //Before LinkUp, Used LowerBound or UpperBound as R66 value.
} BBP_R66_TUNING, *PBBP_R66_TUNING;


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#define EFFECTED_CH_SECONDARY 0x1
#define EFFECTED_CH_PRIMARY	0x2
#define EFFECTED_CH_LEGACY		0x4
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //


// structure to store channel TX power
typedef struct _CHANNEL_TX_POWER {
	USHORT     RemainingTimeForUse;		//unit: sec
	UCHAR      Channel;
#ifdef DOT11N_DRAFT3
	BOOLEAN       bEffectedChannel;	// For BW 40 operating in 2.4GHz , the "effected channel" is the channel that is covered in 40Mhz.
#endif // DOT11N_DRAFT3 //
	CHAR       Power;
	CHAR       Power2;
	UCHAR      MaxTxPwr;
	UCHAR      DfsReq;
	UCHAR	   RegulatoryDomain;

/*
	Channel property:
 
	CHANNEL_DISABLED: The channel is disabled.
	CHANNEL_PASSIVE_SCAN: Only passive scanning is allowed.
	CHANNEL_NO_IBSS: IBSS is not allowed.
	CHANNEL_RADAR: Radar detection is required.
	CHANNEL_NO_FAT_ABOVE: Extension channel above this channel is not allowed.
	CHANNEL_NO_FAT_BELOW: Extension channel below this channel is not allowed.
 */
#define CHANNEL_DISABLED		0x01 /* no use */
#define CHANNEL_PASSIVE_SCAN	0x02
#define CHANNEL_NO_IBSS			0x04
#define CHANNEL_RADAR			0x08
#define CHANNEL_NO_FAT_ABOVE	0x10
#define CHANNEL_NO_FAT_BELOW	0x20

	UCHAR	Flags;

#ifdef RT30xx
	UCHAR		Tx0FinePowerCtrl; // Tx0 fine power control in 0.1dB step
	UCHAR		Tx1FinePowerCtrl; // Tx1 fine power control in 0.1dB step
	UCHAR		Tx2FinePowerCtrl; // Tx2 fine power control in 0.1dB step
#endif // RT30xx //
} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;


typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR     EvaluatePeriod;		 // 0:not evalute status, 1: evaluate status, 2: switching status
	UCHAR     EvaluateStableCnt;
	UCHAR     Pair1PrimaryRxAnt;     // 0:Ant-E1, 1:Ant-E2
	UCHAR     Pair1SecondaryRxAnt;   // 0:Ant-E1, 1:Ant-E2
	SHORT     Pair1LastAvgRssi;      // 
	SHORT     Pair2LastAvgRssi;      // 
	ULONG     RcvPktNumWhenEvaluate;
	BOOLEAN   FirstPktArrivedWhenEvaluate;
#ifdef CONFIG_AP_SUPPORT
	LONG     Pair1AvgRssiGroup1[2];
	LONG     Pair1AvgRssiGroup2[2];
	ULONG     RcvPktNum[2];
#endif // CONFIG_AP_SUPPORT //
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;


/***************************************************************************
  *	structure for radar detection and channel switch
  **************************************************************************/
typedef struct _RADAR_DETECT_STRUCT {
    //BOOLEAN		IEEE80211H;			// 0: disable, 1: enable IEEE802.11h
	UCHAR		CSCount;			//Channel switch counter
	UCHAR		CSPeriod;			//Channel switch period (beacon count)
	UCHAR		RDCount;			//Radar detection counter
	UCHAR		RDMode;				//Radar Detection mode
	UCHAR		RDDurRegion;		//Radar detection duration region
	UCHAR		BBPR16;
	UCHAR		BBPR17;
	UCHAR		BBPR18;
	UCHAR		BBPR21;
	UCHAR		BBPR22;
	UCHAR		BBPR64;
	ULONG		InServiceMonitorCount; // unit: sec
	UINT8		DfsSessionTime;
	BOOLEAN		bFastDfs;
	USHORT		ChMovingTime;
	UINT8		LongPulseRadarTh;
#if defined(RTMP_RBUS_SUPPORT)||defined(DFS_INTERRUPT_SUPPORT)
	CHAR		AvgRssiReq;
	ULONG		DfsLowerLimit;
	ULONG		DfsUpperLimit;
	UINT8		FixDfsLimit;
	ULONG		upperlimit;
	ULONG		lowerlimit;
#endif // defined(RTMP_RBUS_SUPPORT)||defined(DFS_INTERRUPT_SUPPORT) //
} RADAR_DETECT_STRUCT, *PRADAR_DETECT_STRUCT;


#ifdef DFS_SUPPORT
typedef enum _DFS_VERSION
{
	SOFTWARE_DFS=0,
	HARDWARE_DFS_V1,
	HARDWARE_DFS_V2
}  DFS_VERSION;
#endif // DFS_SUPPORT //


#ifdef CARRIER_DETECTION_SUPPORT
typedef enum CD_STATE_n
{
	CD_NORMAL,
	CD_SILENCE,
	CD_MAX_STATE
} CD_STATE;


typedef enum _TONE_RADAR_VERSION
{
	DISABLE_TONE_RADAR=0,
	TONE_RADAR_V1,
	TONE_RADAR_V2
} TONE_RADAR_VERSION;

#ifdef TONE_RADAR_DETECT_SUPPORT
#define CARRIER_DETECT_RECHECK_TIME			3
#define CARRIER_DETECT_CRITIRIA				7000
#define CARRIER_DETECT_STOP_RATIO				2
#define CARRIER_DETECT_STOP_RECHECK_TIME		4
#define CARRIER_DETECT_CRITIRIA_A				230
#define CARRIER_DETECT_DELTA					7
#define CARRIER_DETECT_DIV_FLAG				0
#define CARRIER_DETECT_THRESHOLD			0x0fffffff
#endif // TONE_RADAR_DETECT_SUPPORT //

typedef struct CARRIER_DETECTION_s
{
	BOOLEAN					Enable;
	UINT8					CDSessionTime;
	UINT8					CDPeriod;
	CD_STATE				CD_State;
#ifdef TONE_RADAR_DETECT_SUPPORT
	UINT8					delta;
	UINT8					div_flag;
	UINT32					threshold;
	UINT8					recheck;
	UINT8					recheck1;
	UINT32					TimeStamp;
	UINT32					criteria;
	UINT32					CarrierDebug;
	ULONG					idle_time;
	ULONG					busy_time;
	ULONG					Debug;
	ULONG					OneSecIntCount;
#ifdef  TONE_RADAR_DETECT_V2
	UCHAR Symmetric_Round;
	UCHAR VGA_Mask;
	UCHAR Packet_End_Mask;
	UCHAR Rx_PE_Mask;
#endif // TONE_RADAR_DETECT_V2 //
#endif // TONE_RADAR_DETECT_SUPPORT //
}CARRIER_DETECTION_STRUCT, *PCARRIER_DETECTION_STRUCT;
#endif // CARRIER_DETECTION_SUPPORT //


#ifdef DFS_HARDWARE_SUPPORT
typedef struct _NewDFSDebug
{
	UCHAR channel;
	ULONG wait_time;
	UCHAR delta_delay_range;
	UCHAR delta_delay_step;
	UCHAR EL_range;
	UCHAR EL_step;
	UCHAR EH_range;
	UCHAR EH_step;
	UCHAR WL_range;
	UCHAR WL_step;
	UCHAR WH_range;
	UCHAR WH_step;
	ULONG T_expected;
	ULONG T_margin;
	UCHAR start;
	ULONG count;
	ULONG idx;

}NewDFSDebug, *pNewDFSDebug;

#define NEW_DFS_FCC_5_ENT_NUM		5
#define NEW_DFS_DBG_PORT_ENT_NUM_POWER	8
#define NEW_DFS_DBG_PORT_ENT_NUM		(1 << NEW_DFS_DBG_PORT_ENT_NUM_POWER)  // CE Debug Port entry number, 256
#define NEW_DFS_DBG_PORT_MASK	(NEW_DFS_DBG_PORT_ENT_NUM - 1) // 0xff

// Matched Period definition
#define NEW_DFS_MPERIOD_ENT_NUM_POWER		8
#define NEW_DFS_MPERIOD_ENT_NUM		(1 << NEW_DFS_MPERIOD_ENT_NUM_POWER)	 // CE Period Table entry number, 512
#define NEW_DFS_MAX_CHANNEL	4

typedef struct _NewDFSDebugPort{
	ULONG counter;
	ULONG timestamp;
	USHORT width;
	USHORT start_idx; // start index to period table
	USHORT end_idx; // end index to period table
}NewDFSDebugPort, *pNewDFSDebugPort;

// Matched Period Table
typedef struct _NewDFSMPeriod{
	USHORT	idx;
	USHORT width;
	USHORT	idx2;
	USHORT width2;
	ULONG period;
}NewDFSMPeriod, *pNewDFSMPeriod;

typedef struct _NewDFSParam{
	BOOLEAN	valid;
	UCHAR 	mode;
	USHORT 	avgLen;
	USHORT 	ELow;
	USHORT 	EHigh;
	USHORT 	WLow;
	USHORT 	WHigh;
	UCHAR 	EpsilonW;
	ULONG 	TLow;
	ULONG 	THigh;
	UCHAR 	EpsilonT;
#ifdef DFS_2_SUPPORT
	ULONG BLow;
	ULONG BHigh;
#endif //DFS_2_SUPPORT
}NewDFSParam, *pNewDFSParam;
#endif // DFS_HARDWARE_SUPPORT //


typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

/***************************************************************************
  *	structure for MLME state machine
  **************************************************************************/
typedef struct _MLME_STRUCT {
	STATE_MACHINE_FUNC      ActFunc[ACT_FUNC_SIZE];
	// Action 
	STATE_MACHINE           ActMachine;

#ifdef WSC_INCLUDED
	STATE_MACHINE			WscMachine;
	STATE_MACHINE_FUNC		WscFunc[WSC_FUNC_SIZE];
#endif // WSC_INCLUDED //

#ifdef QOS_DLS_SUPPORT
	STATE_MACHINE			DlsMachine;
	STATE_MACHINE_FUNC      DlsFunc[DLS_FUNC_SIZE];
#endif // QOS_DLS_SUPPORT //


#ifdef CONFIG_AP_SUPPORT
	// AP state machines
	STATE_MACHINE           ApAssocMachine;
	STATE_MACHINE           ApAuthMachine;
	STATE_MACHINE           ApSyncMachine;
	STATE_MACHINE_FUNC      ApAssocFunc[AP_ASSOC_FUNC_SIZE];
//	STATE_MACHINE_FUNC		ApDlsFunc[DLS_FUNC_SIZE];
	STATE_MACHINE_FUNC      ApAuthFunc[AP_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC      ApSyncFunc[AP_SYNC_FUNC_SIZE];
#ifdef APCLI_SUPPORT
	STATE_MACHINE          ApCliAuthMachine;
	STATE_MACHINE          ApCliAssocMachine;
	STATE_MACHINE          ApCliCtrlMachine;
	STATE_MACHINE          ApCliSyncMachine;
	STATE_MACHINE          ApCliWpaPskMachine;

	STATE_MACHINE_FUNC     ApCliAuthFunc[APCLI_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC     ApCliAssocFunc[APCLI_ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC     ApCliCtrlFunc[APCLI_CTRL_FUNC_SIZE];
	STATE_MACHINE_FUNC     ApCliSyncFunc[APCLI_SYNC_FUNC_SIZE];
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //
	
	// common WPA state machine
	STATE_MACHINE           WpaMachine;
	STATE_MACHINE_FUNC      WpaFunc[WPA_FUNC_SIZE];

	

	ULONG                   ChannelQuality;  // 0..100, Channel Quality Indication for Roaming
	ULONG                   Now32;           // latch the value of NdisGetSystemUpTime()
	ULONG                   LastSendNULLpsmTime;

	BOOLEAN                 bRunning;
	NDIS_SPIN_LOCK          TaskLock;
	MLME_QUEUE              Queue;

	UINT                    ShiftReg;

	RALINK_TIMER_STRUCT     PeriodicTimer;
	RALINK_TIMER_STRUCT     APSDPeriodicTimer;
	RALINK_TIMER_STRUCT     LinkDownTimer;
	RALINK_TIMER_STRUCT     LinkUpTimer;
#ifdef RTMP_MAC_PCI
    UCHAR                   bPsPollTimerRunning;
    RALINK_TIMER_STRUCT     PsPollTimer;    
	RALINK_TIMER_STRUCT     RadioOnOffTimer;
#endif // RTMP_MAC_PCI //
	ULONG                   PeriodicRound;
	ULONG                   GPIORound;
	ULONG                   OneSecPeriodicRound;

	UCHAR					RealRxPath;
	BOOLEAN					bLowThroughput;
	BOOLEAN					bEnableAutoAntennaCheck;
	RALINK_TIMER_STRUCT		RxAntEvalTimer;

#ifdef RT30xx
	UCHAR CaliBW40RfR24;
	UCHAR CaliBW20RfR24;
	UCHAR CaliBW20RfR31;
	UCHAR CaliBW40RfR31;
#endif // RT30xx //
	
} MLME_STRUCT, *PMLME_STRUCT;


#ifdef DOT11_N_SUPPORT
/***************************************************************************
  *	802.11 N related data structures
  **************************************************************************/
struct reordering_mpdu
{
	struct reordering_mpdu	*next;
	PNDIS_PACKET			pPacket;		/* coverted to 802.3 frame */
	int						Sequence;		/* sequence number of MPDU */
	BOOLEAN					bAMSDU;
};

struct reordering_list
{
	struct reordering_mpdu *next;
	int 	qlen;
};

struct reordering_mpdu_pool
{
	PVOID					mem;
	NDIS_SPIN_LOCK			lock;
	struct reordering_list 	freelist;
};

typedef enum _REC_BLOCKACK_STATUS
{	
    Recipient_NONE=0,
	Recipient_USED,
	Recipient_HandleRes,
    Recipient_Accept
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS
{
    Originator_NONE=0,
	Originator_USED,
    Originator_WaitRes,
    Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct _BA_ORI_ENTRY{
	UCHAR   Wcid;
	UCHAR   TID;
	UCHAR   BAWinSize;
	UCHAR   Token;
// Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header.
	USHORT	Sequence;
	USHORT	TimeOutValue;
	ORI_BLOCKACK_STATUS  ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
	PVOID	pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

typedef struct _BA_REC_ENTRY {
	UCHAR   Wcid;
	UCHAR   TID;
	UCHAR   BAWinSize;	// 7.3.1.14. each buffer is capable of holding a max AMSDU or MSDU.
	//UCHAR	NumOfRxPkt;
	//UCHAR    Curindidx; // the head in the RX reordering buffer
	USHORT		LastIndSeq;
//	USHORT		LastIndSeqAtTimer;
	USHORT		TimeOutValue;
	RALINK_TIMER_STRUCT RECBATimer;
	ULONG		LastIndSeqAtTimer;
	ULONG		nDropPacket;
	ULONG		rcvSeq;
	REC_BLOCKACK_STATUS  REC_BA_Status;
//	UCHAR	RxBufIdxUsed;
	// corresponding virtual address for RX reordering packet storage.
	//RTMP_REORDERDMABUF MAP_RXBuf[MAX_RX_REORDERBUF]; 
	NDIS_SPIN_LOCK          RxReRingLock;                 // Rx Ring spinlock
//	struct _BA_REC_ENTRY *pNext;
	PVOID	pAdapter;
	struct reordering_list	list;
} BA_REC_ENTRY, *PBA_REC_ENTRY;


typedef struct {
	ULONG		numAsRecipient;		// I am recipient of numAsRecipient clients. These client are in the BARecEntry[]
	ULONG		numAsOriginator;	// I am originator of 	numAsOriginator clients. These clients are in the BAOriEntry[]
	ULONG  		numDoneOriginator; 	// count Done Originator sessions
	BA_ORI_ENTRY       BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	BA_REC_ENTRY       BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
} BA_TABLE, *PBA_TABLE;

//For QureyBATableOID use;
typedef struct  GNU_PACKED _OID_BA_REC_ENTRY{
	UCHAR   MACAddr[MAC_ADDR_LEN];
	UCHAR   BaBitmap;   // if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize
	UCHAR   rsv; 
	UCHAR   BufSize[8];
	REC_BLOCKACK_STATUS	REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

//For QureyBATableOID use;
typedef struct  GNU_PACKED _OID_BA_ORI_ENTRY{
	UCHAR   MACAddr[MAC_ADDR_LEN];
	UCHAR   BaBitmap;  // if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status
	UCHAR   rsv; 
	UCHAR   BufSize[8];
	ORI_BLOCKACK_STATUS  ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE{
	OID_BA_ORI_ENTRY       BAOriEntry[32];
	OID_BA_REC_ENTRY       BARecEntry[32];
	UCHAR   OriNum;// Number of below BAOriEntry
	UCHAR   RecNum;// Number of below BARecEntry
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef	union	_BACAP_STRUC	{
#ifdef RT_BIG_ENDIAN
	struct	{
		UINT32     :4;
		UINT32     b2040CoexistScanSup:1;		//As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz.
		UINT32     bHtAdhoc:1;			// adhoc can use ht rate.
		UINT32     MMPSmode:2;	// MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable
		UINT32     AmsduSize:1;	// 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]	= { 3839, 7935};
		UINT32     AmsduEnable:1;	//Enable AMSDU transmisstion
		UINT32		MpduDensity:3;	
		UINT32		Policy:2;	// 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use
		UINT32		AutoBA:1;	// automatically BA  	
		UINT32		TxBAWinLimit:8;
		UINT32		RxBAWinLimit:8;
	}	field;
#else
	struct	{
		UINT32		RxBAWinLimit:8;
		UINT32		TxBAWinLimit:8;
		UINT32		AutoBA:1;	// automatically BA  	
		UINT32		Policy:2;	// 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use
		UINT32		MpduDensity:3;	
		UINT32       	AmsduEnable:1;	//Enable AMSDU transmisstion
		UINT32       	AmsduSize:1;	// 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]	= { 3839, 7935};
		UINT32       	MMPSmode:2;	// MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable
		UINT32       	bHtAdhoc:1;			// adhoc can use ht rate.
		UINT32       	b2040CoexistScanSup:1;		//As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz.
		UINT32       	:4;	
	}	field;
#endif
	UINT32			word;
} BACAP_STRUC, *PBACAP_STRUC;


typedef struct {
	BOOLEAN		IsRecipient;
	UCHAR   MACAddr[MAC_ADDR_LEN];
	UCHAR   TID;
	UCHAR   nMSDU;
	USHORT   TimeOut;
	BOOLEAN bAllTid;  // If True, delete all TID for BA sessions with this MACaddr.
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;


#ifdef DOT11N_DRAFT3
typedef enum _BSS2040COEXIST_FLAG{
	BSS_2040_COEXIST_DISABLE = 0,
	BSS_2040_COEXIST_TIMER_FIRED  = 1,
	BSS_2040_COEXIST_INFO_SYNC = 2,
	BSS_2040_COEXIST_INFO_NOTIFY = 4,
}BSS2040COEXIST_FLAG;

typedef struct _BssCoexChRange_{
	UCHAR primaryCh;
	UCHAR secondaryCh;
	UCHAR effectChStart;
	UCHAR effectChEnd;
}BSS_COEX_CH_RANGE;
#endif // DOT11N_DRAFT3 //

#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#endif // DOT11_N_SUPPORT //


//This structure is for all 802.11n card InterOptibilityTest action. Reset all Num every n second.  (Details see MLMEPeriodic)
typedef	struct	_IOT_STRUC	{
	BOOLEAN			bRTSLongProtOn;
} IOT_STRUC, *PIOT_STRUC;


// This is the registry setting for 802.11n transmit setting.  Used in advanced page.
typedef union _REG_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
 struct {
         UINT32  rsv:13;
		 UINT32  EXTCHA:2;
		 UINT32  HTMODE:1;
		 UINT32  TRANSNO:2; 
		 UINT32  STBC:1; //SPACE 
		 UINT32  ShortGI:1;
		 UINT32  BW:1; //channel bandwidth 20MHz or 40 MHz
		 UINT32  TxBF:1; // 3*3
		 UINT32  ITxBfEn:1;
		 UINT32  rsv0:9;
		 //UINT32  MCS:7;                 // MCS
         //UINT32  PhyMode:4;             
    } field;
#else
 struct {
         //UINT32  PhyMode:4;                
         //UINT32  MCS:7;                 // MCS
		 UINT32  rsv0:9;
		 UINT32  ITxBfEn:1;
		 UINT32  TxBF:1;
         UINT32  BW:1; //channel bandwidth 20MHz or 40 MHz
         UINT32  ShortGI:1;
         UINT32  STBC:1; //SPACE 
         UINT32  TRANSNO:2; 
         UINT32  HTMODE:1;
         UINT32  EXTCHA:2;
         UINT32  rsv:13;
    } field;
#endif
 UINT32   word;
} REG_TRANSMIT_SETTING, *PREG_TRANSMIT_SETTING;


typedef union  _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct	{
			USHORT		rsv:3;	 
			USHORT		FixedTxMode:2;			// If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode.
			USHORT		PhyMode:4;	
			USHORT   	MCS:7;                 // MCS
	}	field;
#else
	struct	{
			USHORT   	MCS:7;                 	// MCS
			USHORT		PhyMode:4;				
			USHORT	 	FixedTxMode:2;			// If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode.
			USHORT		rsv:3;	 	 
	}	field;
#endif
	USHORT		word;
 } DESIRED_TRANSMIT_SETTING, *PDESIRED_TRANSMIT_SETTING;




/***************************************************************************
  *	Multiple SSID related data structures
  **************************************************************************/
#define WLAN_MAX_NUM_OF_TIM			((MAX_LEN_OF_MAC_TABLE >> 3) + 1) /* /8 + 1 */
#define WLAN_CT_TIM_BCMC_OFFSET		0 /* unit: 32B */

/* clear bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_CLEAR(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~BIT8[0];

/* set bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_SET(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= BIT8[0];

/* clear a station PS TIM bit */
#define WLAN_MR_TIM_BIT_CLEAR(ad_p, apidx, wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] &= (~BIT8[bit_offset]); }

/* set a station PS TIM bit */
#define WLAN_MR_TIM_BIT_SET(ad_p, apidx, wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] |= BIT8[bit_offset]; }


#ifdef CONFIG_AP_SUPPORT
typedef struct _MULTISSID_STRUCT {
	UCHAR								Bssid[MAC_ADDR_LEN];
    UCHAR                               SsidLen;
    CHAR                                Ssid[MAX_LEN_OF_SSID];
    USHORT                              CapabilityInfo;
    
	UCHAR								MaxStaNum;		// Limit the STA connection number per BSS
	UCHAR								StaCount;
    
    PNET_DEV                   			MSSIDDev;

	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;
	NDIS_802_11_WEP_STATUS              WepStatus;
	NDIS_802_11_WEP_STATUS				GroupKeyWepStatus;
	WPA_MIX_PAIR_CIPHER					WpaMixPairCipher;

	ULONG								TxCount;
	ULONG								RxCount;
	ULONG								ReceivedByteCount;
	ULONG								TransmittedByteCount;
	ULONG								RxErrorCount;
	ULONG								RxDropCount;

	HTTRANSMIT_SETTING					HTPhyMode, MaxHTPhyMode, MinHTPhyMode;// For transmit phy setting in TXWI.	
	RT_HT_PHY_INFO						DesiredHtPhyInfo;
	DESIRED_TRANSMIT_SETTING        	DesiredTransmitSetting; // Desired transmit setting. this is for reading registry setting only. not useful.
	BOOLEAN								bAutoTxRateSwitch;

#ifdef STATS_COUNT_SUPPORT
	ULONG								TxErrorCount;
	ULONG								TxDropCount;
	ULONG								ucPktsTx ;
	ULONG								ucPktsRx ;
	ULONG								mcPktsTx;
	ULONG								mcPktsRx;
	ULONG								bcPktsTx;
	ULONG								bcPktsRx;
#endif // STATS_COUNT_SUPPORT //

	//CIPHER_KEY                          SharedKey[SHARE_KEY_NUM]; // ref pAd->SharedKey[BSS][4]
	UCHAR                               DefaultKeyId;

	UCHAR								TxRate;       // RATE_1, RATE_2, RATE_5_5, RATE_11, ...
	UCHAR     							DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];// OID_802_11_DESIRED_RATES
	UCHAR								DesiredRatesIndex;
	UCHAR     							MaxTxRate;            // RATE_1, RATE_2, RATE_5_5, RATE_11

//	ULONG           					TimBitmap;      // bit0 for broadcast, 1 for AID1, 2 for AID2, ...so on
//    ULONG           					TimBitmap2;     // b0 for AID32, b1 for AID33, ... and so on
	UCHAR								TimBitmaps[WLAN_MAX_NUM_OF_TIM];

    // WPA
    UCHAR                               GMK[32];
    UCHAR                               PMK[32];
	UCHAR								GTK[32];
#ifdef DOT1X_SUPPORT	
    BOOLEAN                             IEEE8021X;
    BOOLEAN                             PreAuth;
#endif // DOT1X_SUPPORT //	
    UCHAR                               GNonce[32];
    UCHAR                               PortSecured;
    NDIS_802_11_PRIVACY_FILTER          PrivacyFilter;
    UCHAR                               BANClass3Data;
    ULONG                               IsolateInterStaTraffic;

    UCHAR                               RSNIE_Len[2];
    UCHAR                               RSN_IE[2][MAX_LEN_OF_RSNIE];

	// for Group Rekey 
	RT_WPA_REKEY    					WPAREKEY;
	ULONG           					REKEYCOUNTER;
	RALINK_TIMER_STRUCT     			REKEYTimer;
	UCHAR           					REKEYTimerRunning;
	UINT8								RekeyCountDown;
	
#ifdef WAPI_SUPPORT
	UCHAR								WAPIPassPhrase[64];		// WAPI PSK pass phrase
	UINT								WAPIPassPhraseLen;		// the length of WAPI PSK pass phrase
	UINT								WapiPskType;			// 0 - Hex, 1 - ASCII 								
	UCHAR								WAPI_BK[16];			// WAPI base key
	
	UCHAR								NMK[LEN_WAPI_NMK];
	UCHAR								key_announce_flag[LEN_WAPI_TSC];	
	BOOLEAN								sw_wpi_encrypt;			// WPI data encrypt by SW
#endif // WAPI_SUPPORT //

    UCHAR                   			TimIELocationInBeacon;
    UCHAR                   			CapabilityInfoLocationInBeacon;

	// For PMK Cache using
	ULONG								PMKCachePeriod;			// unit : jiffies	
    NDIS_AP_802_11_PMKID				PMKIDCache;

    // outgoing BEACON frame buffer and corresponding TXWI
	// PTXWI_STRUC                           BeaconTxWI; // 
    CHAR                                BeaconBuf[MAX_BEACON_SIZE]; // NOTE: BeaconBuf should be 4-byte aligned

    BOOLEAN                             bHideSsid;
	UINT16								StationKeepAliveTime; // unit: second

	// VLAN related
    USHORT                              VLAN_VID;
    USHORT                              VLAN_Priority;

    RT_802_11_ACL						AccessControlList;

	// EDCA Qos
    BOOLEAN								bWmmCapable;	// 0:disable WMM, 1:enable WMM
    BOOLEAN								bDLSCapable;	// 0:disable DLS, 1:enable DLS

	/*
		Why need the parameter: 2009/09/22

		1. iwpriv ra0 set WmmCapable=0
		2. iwpriv ra0 set WirelessMode=9
		3. iwpriv ra0 set WirelessMode=0
		4. iwpriv ra0 set SSID=SampleAP

		After the 4 commands, WMM is still enabled.
		So we need the parameter to recover WMM Capable flag.

		No the problem in station mode.
	*/
	BOOLEAN								bWmmCapableOrg;	// origin Wmm Capable in non-11n mode

#ifdef QOS_DLS_SUPPORT
	UCHAR           					DlsPTK[64];		// Due to windows dirver count on meetinghouse to handle 4-way shake
#endif // QOS_DLS_SUPPORT //
	
#ifdef DOT1X_SUPPORT	
	// For 802.1x daemon setting per BSS
	UINT8								radius_srv_num;
	RADIUS_SRV_INFO						radius_srv_info[MAX_RADIUS_SRV_NUM];
	UINT8								NasId[IFNAMSIZ];
	UINT8								NasIdLen;
#endif // DOT1X_SUPPORT //

#ifdef RTL865X_SOC
	unsigned int						mylinkid;
#endif 

#ifdef CONFIG_AP_SUPPORT
	WSC_LV_INFO            WscIEBeacon;
   	WSC_LV_INFO            WscIEProbeResp;
#ifdef WSC_AP_SUPPORT	
	WSC_CTRL	           WscControl;
	WSC_SECURITY_MODE		WscSecurityMode; 
#endif // WSC_AP_SUPPORT //

#endif // CONFIG_AP_SUPPORT //

#ifdef IDS_SUPPORT
	UINT32					RcvdConflictSsidCount;					
	UINT32					RcvdSpoofedAssocRespCount;	
	UINT32					RcvdSpoofedReassocRespCount;	
	UINT32					RcvdSpoofedProbeRespCount;
	UINT32					RcvdSpoofedBeaconCount;
	UINT32					RcvdSpoofedDisassocCount;	
	UINT32					RcvdSpoofedAuthCount;
	UINT32					RcvdSpoofedDeauthCount;	
	UINT32					RcvdSpoofedUnknownMgmtCount;	
	UINT32					RcvdReplayAttackCount;

	CHAR					RssiOfRcvdConflictSsid;					
	CHAR					RssiOfRcvdSpoofedAssocResp;	
	CHAR					RssiOfRcvdSpoofedReassocResp;	
	CHAR					RssiOfRcvdSpoofedProbeResp;
	CHAR					RssiOfRcvdSpoofedBeacon;
	CHAR					RssiOfRcvdSpoofedDisassoc;	
	CHAR					RssiOfRcvdSpoofedAuth;
	CHAR					RssiOfRcvdSpoofedDeauth;	
	CHAR					RssiOfRcvdSpoofedUnknownMgmt;	
	CHAR					RssiOfRcvdReplayAttack;		
#endif // IDS_SUPPORT //

	/* used in if beacon send or stop */
	BOOLEAN					bBcnSntReq;
	UCHAR					BcnBufIdx;





	UCHAR       			PhyMode;

} MULTISSID_STRUCT, *PMULTISSID_STRUCT;
#endif // CONFIG_AP_SUPPORT //

// configuration common to OPMODE_AP as well as OPMODE_STA
typedef struct _COMMON_CONFIG {

	BOOLEAN		bCountryFlag;
	UCHAR		CountryCode[3];
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR		Geography;
#endif // EXT_BUILD_CHANNEL_LIST //
	UCHAR       CountryRegion;      // Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel
	UCHAR       CountryRegionForABand;	// Enum of country region for A band
	UCHAR       PhyMode;            // PHY_11A, PHY_11B, PHY_11BG_MIXED, PHY_ABG_MIXED
	UCHAR       DesiredPhyMode;            // PHY_11A, PHY_11B, PHY_11BG_MIXED, PHY_ABG_MIXED	
	USHORT      Dsifs;              // in units of usec
	ULONG       PacketFilter;       // Packet filter for receiving
	UINT8		RegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS];

	CHAR        Ssid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
	UCHAR       SsidLen;               // the actual ssid length in used
	UCHAR       LastSsidLen;               // the actual ssid length in used
	CHAR        LastSsid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
	UCHAR		LastBssid[MAC_ADDR_LEN];

	UCHAR       Bssid[MAC_ADDR_LEN];
	USHORT      BeaconPeriod;
	UCHAR       Channel;            
	UCHAR       CentralChannel;    	// Central Channel when using 40MHz is indicating. not real channel.        

	UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR       SupRateLen;
	UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR       ExtRateLen;
	UCHAR       DesireRate[MAX_LEN_OF_SUPPORTED_RATES];      // OID_802_11_DESIRED_RATES
	UCHAR       MaxDesiredRate;
	UCHAR       ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

	ULONG       BasicRateBitmap;        // backup basic ratebitmap
	ULONG       BasicRateBitmapOld;     // backup basic ratebitmap

	BOOLEAN		bAPSDCapable;
	BOOLEAN		bInServicePeriod;
	BOOLEAN		bAPSDAC_BE;
	BOOLEAN		bAPSDAC_BK;
	BOOLEAN		bAPSDAC_VI;
	BOOLEAN		bAPSDAC_VO;

	/* because TSPEC can modify the APSD flag, we need to keep the APSD flag
		requested in association stage from the station;
		we need to recover the APSD flag after the TSPEC is deleted. */
	BOOLEAN		bACMAPSDBackup[4]; /* for delivery-enabled & trigger-enabled both */
	BOOLEAN		bACMAPSDTr[4]; /* no use */

	BOOLEAN		bNeedSendTriggerFrame;
	BOOLEAN		bAPSDForcePowerSave;	// Force power save mode, should only use in APSD-STAUT
	ULONG		TriggerTimerCount;
	UCHAR		MaxSPLength;
	UCHAR		BBPCurrentBW;	// BW_10, 	BW_20, BW_40
	// move to MULTISSID_STRUCT for MBSS 
	//HTTRANSMIT_SETTING	HTPhyMode, MaxHTPhyMode, MinHTPhyMode;// For transmit phy setting in TXWI.	
	REG_TRANSMIT_SETTING        RegTransmitSetting; //registry transmit setting. this is for reading registry setting only. not useful.
	//UCHAR       FixedTxMode;              // Fixed Tx Mode (CCK, OFDM), for HT fixed tx mode (GF, MIX) , refer to RegTransmitSetting.field.HTMode
	UCHAR       TxRate;                 // Same value to fill in TXD. TxRate is 6-bit 
	UCHAR       MaxTxRate;              // RATE_1, RATE_2, RATE_5_5, RATE_11
	UCHAR       TxRateIndex;            // Tx rate index in RateSwitchTable
	UCHAR       TxRateTableSize;        // Valid Tx rate table size in RateSwitchTable
	//BOOLEAN		bAutoTxRateSwitch;
	UCHAR       MinTxRate;              // RATE_1, RATE_2, RATE_5_5, RATE_11
	UCHAR       RtsRate;                // RATE_xxx
	HTTRANSMIT_SETTING	MlmeTransmit;   // MGMT frame PHY rate setting when operatin at Ht rate.
	UCHAR       MlmeRate;               // RATE_xxx, used to send MLME frames
	UCHAR       BasicMlmeRate;          // Default Rate for sending MLME frames

	USHORT      RtsThreshold;           // in unit of BYTE
	USHORT      FragmentThreshold;      // in unit of BYTE

	UCHAR       TxPower;                // in unit of mW
	ULONG       TxPowerPercentage;      // 0~100 %
	ULONG       TxPowerDefault;         // keep for TxPowerPercentage
	UINT8		PwrConstraint;

#ifdef DOT11_N_SUPPORT
	BACAP_STRUC        BACapability; //   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0
	BACAP_STRUC        REGBACapability; //   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0
#endif // DOT11_N_SUPPORT //
	IOT_STRUC		IOTestParm;	// 802.11n InterOpbility Test Parameter;
	ULONG       TxPreamble;             // Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto
	BOOLEAN     bUseZeroToDisableFragment;     // Microsoft use 0 as disable 
	ULONG       UseBGProtection;        // 0: auto, 1: always use, 2: always not use
	BOOLEAN     bUseShortSlotTime;      // 0: disable, 1 - use short slot (9us)
	BOOLEAN     bEnableTxBurst;         // 1: enble TX PACKET BURST (when BA is established or AP is not a legacy WMM AP), 0: disable TX PACKET BURST
	BOOLEAN     bAggregationCapable;      // 1: enable TX aggregation when the peer supports it
	BOOLEAN     bPiggyBackCapable;		// 1: enable TX piggy-back according MAC's version
	BOOLEAN     bIEEE80211H;			// 1: enable IEEE802.11h spec.
	ULONG		DisableOLBCDetect;		// 0: enable OLBC detect; 1 disable OLBC detect 

#ifdef DOT11_N_SUPPORT
	BOOLEAN				bRdg;
#endif // DOT11_N_SUPPORT //
	BOOLEAN             bWmmCapable;        // 0:disable WMM, 1:enable WMM
	QOS_CAPABILITY_PARM APQosCapability;    // QOS capability of the current associated AP
	EDCA_PARM           APEdcaParm;         // EDCA parameters of the current associated AP
	QBSS_LOAD_PARM      APQbssLoad;         // QBSS load of the current associated AP
	UCHAR               AckPolicy[4];       // ACK policy of the specified AC. see ACK_xxx
	// a bitmap of BOOLEAN flags. each bit represent an operation status of a particular 
	// BOOLEAN control, either ON or OFF. These flags should always be accessed via
	// OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros.
	// see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition
	ULONG               OpStatusFlags;

	BOOLEAN				NdisRadioStateOff; //For HCT 12.0, set this flag to TRUE instead of called MlmeRadioOff.       
	ABGBAND_STATE		BandState;		// For setting BBP used on B/G or A mode.
#ifdef ANT_DIVERSITY_SUPPORT
	UCHAR				bRxAntDiversity; // 0:disable, 1:enable Software Rx Antenna Diversity.
#endif // ANT_DIVERSITY_SUPPORT //

	// IEEE802.11H--DFS.
	RADAR_DETECT_STRUCT	RadarDetect;
	BOOLEAN		bDFSIndoor;
#ifdef CARRIER_DETECTION_SUPPORT
	CARRIER_DETECTION_STRUCT		CarrierDetect;
	UCHAR carrier_func;
#endif // CARRIER_DETECTION_SUPPORT //

#ifdef DOT11_N_SUPPORT
	// HT
	//UCHAR			BASize;		// USer desired BAWindowSize. Should not exceed our max capability
	//RT_HT_CAPABILITY	SupportedHtPhy;
	RT_HT_CAPABILITY	DesiredHtPhy;
	HT_CAPABILITY_IE		HtCapability;	
	ADD_HT_INFO_IE		AddHTInfo;	// Useful as AP.
	//This IE is used with channel switch announcement element when changing to a new 40MHz.
	//This IE is included in channel switch ammouncement frames 7.4.1.5, beacons, probe Rsp.
	NEW_EXT_CHAN_IE	NewExtChanOffset;	//7.3.2.20A, 1 if extension channel is above the control channel, 3 if below, 0 if not present

	EXT_CAP_INFO_ELEMENT	ExtCapIE;	// this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init.
	
#ifdef DOT11N_DRAFT3
	BOOLEAN					bBssCoexEnable;
	/* 
		Following two paramters now only used for the initial scan operation. the AP only do 
			bandwidth fallback when BssCoexApCnt > BssCoexApCntThr
		By default, the "BssCoexApCntThr" is set as 0 in "UserCfgInit()".
	*/
	UCHAR					BssCoexApCntThr;	
	UCHAR					BssCoexApCnt;
	
	UCHAR					Bss2040CoexistFlag;	// bit 0: bBssCoexistTimerRunning, bit 1: NeedSyncAddHtInfo.
	RALINK_TIMER_STRUCT	Bss2040CoexistTimer;
	UCHAR					Bss2040NeedFallBack; 

	//This IE is used for 20/40 BSS Coexistence.
	BSS_2040_COEXIST_IE		BSS2040CoexistInfo;
	// ====== 11n D3.0 =======================>
	USHORT					Dot11OBssScanPassiveDwell;				// Unit : TU. 5~1000
	USHORT					Dot11OBssScanActiveDwell;				// Unit : TU. 10~1000
	USHORT					Dot11BssWidthTriggerScanInt;			// Unit : Second
	USHORT					Dot11OBssScanPassiveTotalPerChannel;	// Unit : TU. 200~10000
	USHORT					Dot11OBssScanActiveTotalPerChannel;	// Unit : TU. 20~10000
	USHORT					Dot11BssWidthChanTranDelayFactor;
	USHORT					Dot11OBssScanActivityThre;				// Unit : percentage
	
	ULONG					Dot11BssWidthChanTranDelay;			// multiple of (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor)
	ULONG					CountDownCtr;	// CountDown Counter from (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor)
	
	BSS_2040_COEXIST_IE		LastBSSCoexist2040;
	BSS_2040_COEXIST_IE		BSSCoexist2040;
	TRIGGER_EVENT_TAB		TriggerEventTab;
	UCHAR					ChannelListIdx;
	// <====== 11n D3.0 =======================
	BOOLEAN					bOverlapScanning;
	BOOLEAN					bBssCoexNotify;
#endif // DOT11N_DRAFT3 //

    BOOLEAN                 bHTProtect;
    BOOLEAN                 bMIMOPSEnable;
    BOOLEAN					bBADecline;
	BOOLEAN					bDisableReordering;
#ifdef DOT11N_DRAFT3
	BOOLEAN					bForty_Mhz_Intolerant;
#endif // DOT11N_DRAFT3 //
	BOOLEAN					bExtChannelSwitchAnnouncement;
	BOOLEAN					bRcvBSSWidthTriggerEvents;
	ULONG					LastRcvBSSWidthTriggerEventsTime;

	UCHAR					TxBASize;	
#endif // DOT11_N_SUPPORT //

#ifdef SYSTEM_LOG_SUPPORT
	// Enable wireless event
	BOOLEAN				bWirelessEvent;
#endif // SYSTEM_LOG_SUPPORT //

	BOOLEAN				bWiFiTest;				// Enable this parameter for WiFi test		

	// Tx & Rx Stream number selection
	UCHAR				TxStream;
	UCHAR				RxStream;

	// transmit phy mode, trasmit rate for Multicast.
#ifdef MCAST_RATE_SPECIFIC
	UCHAR				McastTransmitMcs;
	UCHAR				McastTransmitPhyMode;
#endif // MCAST_RATE_SPECIFIC //

	BOOLEAN     		bHardwareRadio;     // Hardware controlled Radio enabled


#ifdef WSC_INCLUDED
	/* WSC hardware push button function 0811 */
	UINT8					WscHdrPshBtnCheckCount;
#endif // WSC_INCLUDED //

	NDIS_SPIN_LOCK			MeasureReqTabLock;
	PMEASURE_REQ_TAB		pMeasureReqTab;

	NDIS_SPIN_LOCK			TpcReqTabLock;
	PTPC_REQ_TAB			pTpcReqTab;

	// transmit phy mode, trasmit rate for Multicast.
#ifdef MCAST_RATE_SPECIFIC
	HTTRANSMIT_SETTING		MCastPhyMode;
#endif // MCAST_RATE_SPECIFIC //

#ifdef SINGLE_SKU
	UINT16					DefineMaxTxPwr;
	BOOLEAN bSKUMode;
	UINT16	AntGain;
	UINT16	BandedgeDelta;
#endif // SINGLE_SKU //

#ifdef WAPI_SUPPORT
	COMMON_WAPI_INFO	comm_wapi_info;

	// rekey related parameter
	// USK update parameter
	UINT8				wapi_usk_rekey_method;			// 0:disable , 1:time, 2:packet
	UINT32				wapi_usk_rekey_threshold;		// the rekey threshold

	// MSK update parameter
	UINT8				wapi_msk_rekey_method;			// 0:disable , 1:time, 2:packet
	UINT32				wapi_msk_rekey_threshold;		// the rekey threshold

	UINT32           	wapi_msk_rekey_cnt;	
	RALINK_TIMER_STRUCT WapiMskRekeyTimer;
	UCHAR           	WapiMskRekeyTimerRunning;
#endif // WAPI_SUPPORT //

#ifdef DFS_SUPPORT
#if defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)

	ULONG CID;
	ULONG CN;

	// Radar and carrier detection in 2880-SW-MCU
	UCHAR McuRadarCmd; // RADAR_DETECTION==1 or CARRIER_DETECTION==2, use bit map
	UCHAR McuRadarEvent; // signal between Driver and 2880-SW-MCU
	UCHAR McuRadarProtection;
	UCHAR McuRadarState; // WAIT_CTS_BEING_SENT / DO_DETECTION / FREE_FOR_TX
	UCHAR McuCarrierState;

	// old value
	ULONG OldRtsRetryLimit;
	UCHAR _R65;
	UCHAR _R66;
	UCHAR _R69;
	UCHAR _R70;
	UCHAR _R73;
	UCHAR R65;
	UCHAR R66;
	UCHAR DFS_R66;
#endif // defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //
	
#ifdef DFS_HARDWARE_SUPPORT
	ULONG MCURadarRegion;
	UCHAR DeltaDelay;
	UCHAR RadarReEnable;
	USHORT PollTime;
	USHORT ChEnable;
	INT   DfsRssiHigh;
	INT   DfsRssiLow;
	INT   DfsRssiHighFromCfg;
	INT   DfsRssiLowFromCfg;
	BOOLEAN  DfsRssiHighCfgValid;
	BOOLEAN  DfsRssiLowCfgValid;
	NewDFSDebug DfsDebug;
	NewDFSDebugPort FCC_5[NEW_DFS_FCC_5_ENT_NUM];
	UCHAR	fcc_5_idx;
	UCHAR	fcc_5_last_idx;
	USHORT	fcc_5_threshold; // to check the width of long pulse radar
	ULONG	fcc_5_counter;
	UCHAR	use_tasklet;
	UCHAR		DFSParamFromConfig;
	NewDFSParam	NewDFSTableEntry[16];
#ifdef DFS_DEBUG
	// Roger debug
	UCHAR DebugPort[384];
	UCHAR DebugPortPrint; // 0 = stop, 1 = log req, 2 = loging, 3 = log done
	ULONG TotalEntries[4];
		
	ULONG T_Matched_2;
	ULONG T_Matched_3;
	ULONG T_Matched_4;
	ULONG T_Matched_5;
	UCHAR BBP127Repeat;
	
	ULONG CounterStored[5];
	ULONG CounterStored2[5];
	ULONG CounterStored3;
	NewDFSDebugPort CE_DebugCh0[NEW_DFS_DBG_PORT_ENT_NUM];
	NewDFSMPeriod CE_TCh0[NEW_DFS_MPERIOD_ENT_NUM];
#endif

	// CE Staggered radar / weather radar
	NewDFSDebugPort DFS_W[NEW_DFS_MAX_CHANNEL][NEW_DFS_DBG_PORT_ENT_NUM];
	USHORT	dfs_w_idx[NEW_DFS_MAX_CHANNEL];
	USHORT	dfs_w_last_idx[NEW_DFS_MAX_CHANNEL];
	ULONG	dfs_w_counter;

	NewDFSMPeriod DFS_T[NEW_DFS_MAX_CHANNEL][NEW_DFS_MPERIOD_ENT_NUM]; // period table
	USHORT	dfs_t_idx[NEW_DFS_MAX_CHANNEL];
	
	USHORT	dfs_width_diff_ch1_Shift;
	USHORT	dfs_width_diff_ch2_Shift;
	USHORT 	dfs_width_diff_Shift;
	USHORT	dfs_period_err;
	ULONG	dfs_max_period; // Max possible Period
	USHORT	dfs_width_diff;
	USHORT	dfs_width_ch0_err_L;
	USHORT	dfs_width_ch0_err_H;
	UCHAR	dfs_check_loop;
	UCHAR	dfs_declare_thres;
	UCHAR	radarDeclared;
	UCHAR	Ch0Overflow;
	UCHAR	ce_staggered_check;
	UCHAR	hw_dfs_disabled;

	UINT					sw_idx[NEW_DFS_MAX_CHANNEL];
	UINT					hw_idx[NEW_DFS_MAX_CHANNEL];
	UINT					pr_idx[NEW_DFS_MAX_CHANNEL];
	
	// Hardware detection re-check
	ULONG	re_check_jiffies[NEW_DFS_MAX_CHANNEL];
	ULONG	re_check_Width[NEW_DFS_MAX_CHANNEL];
	ULONG	re_check_Period[NEW_DFS_MAX_CHANNEL];
		
	// false detection filter
	UCHAR	BUM_time; // Bandwidth Usage monitor time
	ULONG	idle_time;
	ULONG	busy_time;
	int		rssi;
	UCHAR	ch_busy;
	UCHAR  ch_busy_idle_ratio;
	UCHAR  BusyIdleFromCfg;
	BOOLEAN  BusyIdleCfgValid;
	
	// Dfs: Radar event expiration parameter
	ULONG  RadarEventExpire[4];
	
#define CH_BUSY_NEGATIVE_MASK 0xe0000000  // this mask is used when use shift operation instead of division on negative integer.
#define CH_BUSY_SAMPLE_POWER 3
#define CH_BUSY_SAMPLE (1 << CH_BUSY_SAMPLE_POWER)
#define CH_BUSY_MASK  (CH_BUSY_SAMPLE - 1)
	UCHAR  print_ch_busy_sta;
	ULONG  ch_busy_sta[CH_BUSY_SAMPLE];
	ULONG  ch_idle_sta[CH_BUSY_SAMPLE];
	UCHAR  ch_busy_sta_index;
	int  ch_busy_sum;
	int  ch_idle_sum;
	
#define MAX_FDF_NUMBER 5 // max false-detection-filter number
	UCHAR	fdf_num;
	USHORT	ch_busy_threshold[MAX_FDF_NUMBER];
	int		rssi_threshold[MAX_FDF_NUMBER];

	// Support after dfs_func >= 2
	UCHAR Symmetric_Round;
	UCHAR VGA_Mask;
	UCHAR Packet_End_Mask;
	UCHAR Rx_PE_Mask;
	UCHAR SymRoundFromCfg;
	UCHAR SymRoundCfgValid;
#endif // DFS_HARDWARE_SUPPORT //


	// DFS patch for long pulse
	UCHAR W56_debug;
	ULONG W56_idx;
	ULONG W56_1s;
	ULONG W56_4s[4];
	ULONG W56_total;
	ULONG W56_hw_1;
	ULONG W56_hw_2;
	ULONG W56_hw_sum;
	UCHAR RadarElectNum;

	//UCHAR BlockCh[MAX_NUM_OF_CHANNELS];
	//USHORT ChRemainTime[MAX_NUM_OF_CHANNELS];
	//UCHAR BlockChNum;
	
	UCHAR McuRadarDebug;

	// Radar detection
	UCHAR McuRadarTick;
	UCHAR McuRadarPeriod;
	UCHAR McuRadarDetectPeriod;
	UCHAR McuRadarCtsProtect;
	UCHAR McuRadarDetectCount;
	ULONG RadarTimeStampHigh;
	ULONG RadarTimeStampLow;
	//BOOLEAN		bDFSIndoor;

#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
	// Carrier detection
	USHORT McuCarrierTick;
	USHORT McuCarrierDetectCount;
	USHORT McuCarrierPeriod;
	UCHAR  McuCarrierDetectPeriod;
	UCHAR  McuCarrierCtsProtect;
#endif // CARRIER_DETECTION_SUPPORT //


#ifdef DFS_SUPPORT
	DFS_VERSION dfs_func;
#ifdef DFS_HARDWARE_SUPPORT
	UCHAR	DFSAPRestart;
	RALINK_TIMER_STRUCT	DFSWatchDogTimer;
	BOOLEAN					DFSWatchDogIsRunning;
	RALINK_TIMER_STRUCT	CSWatchDogTimer;
#endif // DFS_HARDWARE_SUPPORT //
#endif // DFS_SUPPORT //
#if defined(RT305x)||defined(RT30xx)
	// request by Gary, for High Power issue
	UCHAR	HighPowerPatchDisabled;
#endif // defined(RT305x)|| defined(RT30xx) //


	BOOLEAN		HT_DisallowTKIP;		/* Restrict the encryption type in 11n HT mode */

	BOOLEAN		HT_Disable; /* 1: disable HT function; 0: enable HT function */

//#ifdef RTMP_RBUS_SUPPORT

	ULONG DebugFlags;			// Temporary debug flags
//#endif // RTMP_RBUS_SUPPORT //

#ifdef RTMP_MAC_PCI
	BOOLEAN bPCIeBus; // The adapter runs over PCIe bus
#endif // RTMP_MAC_PCI //

#ifdef WSC_INCLUDED
	BOOLEAN					WscPBCOverlap;
	WSC_STA_PBC_PROBE_INFO	WscStaPbcProbeInfo;
#endif // WSC_INCLUDED //


#if defined(RT5390) || defined(RT5370)
	//
	// Temperature compensation, 0: Disabled, 1: Method 1, 2: Method 2
	//
	ULONG TempComp;
#endif // defined(RT5390) || defined(RT5370)//
} COMMON_CONFIG, *PCOMMON_CONFIG;

//#ifdef RTMP_RBUS_SUPPORT
// DebugFlag definitions
#define DBF_LESS_AGG_UP			0x0001	// Less aggressive traing up
#define DBF_CHECK_UP_PER		0x0002	// Don't train up if PER too high
#define DBF_NO_TXBF_3SS			0x0004	// Disable TXBF for 3SS
#define DBF_LOW_RA_THRESHOLDS	0x0008	// Use low RA thresholds
#define DBF_UNUSED0				0x0010	// unused
#define DBF_INIT_MCS_MARGIN		0x0020	// Use 6 dB margin when selecting initial MCS
#define DBF_INIT_MCS_DIS1		0x0040	// Disable highest MCSs when selecting initial MCS
#define DBF_INIT_MCS_DIS2		0x0080	// Disable 2nd highest MCSs when selecting initial MCS
#define DBF_FORCE_SGI			0x0100	// Force Short GI
#define DBF_UNUSED0200				0x0200	// unused
#define DBF_UNUSED0400				0x0400	// unused
#define DBF_UNUSED0800				0x0800	// unused
#define DBF_FORCE_AC_VI			0x1000	// Force AC_VI category
#define DBF_UNUSED2000				0x2000	// unused
#define DBF_UNUSED4000				0x4000	// unused
#define DBF_UNUSED8000				0x8000	// unused
//#endif // RTMP_RBUS_SUPPORT //



#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP related data structures
  **************************************************************************/
// AUTH-RSP State Machine Aux data structure
typedef struct _AP_MLME_AUX {
	UCHAR               Addr[MAC_ADDR_LEN];
	USHORT              Alg;
	CHAR                Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;

typedef enum _MAC_ENTRY_OP_MODE_{
	ENTRY_OP_MODE_ERROR = 0x00,
	ENTRY_OP_MODE_CLI = 0x01,		// Sta mode, set this TRUE after Linkup,too.
	ENTRY_OP_MODE_WDS = 0x02,	// This is WDS Entry. only for AP mode.
	ENTRY_OP_MODE_APCLI = 0x04,	// This is a AP-Client entry, only for AP mode which enable AP-Client functions.
	ENTRY_OP_MODE_MESH = 0x08,	// Peer conect with us via mesh.
	ENTRY_OP_MODE_DLS = 0x10,		// This is DLS Entry. only for STA mode.
	ENTRY_OP_MODE_MAX = 0x20
}MAC_ENTRY_OP_MODE;
#endif // CONFIG_AP_SUPPORT //


typedef struct _MAC_TABLE_ENTRY {
	/*
		0:Invalid,
		Bit 0: AsCli, Bit 1: AsWds, Bit 2: AsAPCLI,
		Bit 3: AsMesh, Bit 4: AsDls, Bit 5: AsTDls
	*/
	UINT32		EntryType;
	BOOLEAN		isCached;
	BOOLEAN		bIAmBadAtheros;	// Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection.

	/* WPA/WPA2 4-way database */
	UCHAR         	EnqueueEapolStartTimerRunning;  // Enqueue EAPoL-Start for triggering EAP SM
	RALINK_TIMER_STRUCT				EnqueueStartForPSKTimer;	    // A timer which enqueue EAPoL-Start for triggering PSK SM
    
	//jan for wpa
	// record which entry revoke MIC Failure , if it leaves the BSS itself, AP won't update aMICFailTime MIB
	UCHAR           CMTimerRunning;
	UCHAR           apidx;			// MBSS number 
	UCHAR           RSNIE_Len;
	UCHAR           RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR           ANonce[LEN_KEY_DESC_NONCE];
	UCHAR           SNonce[LEN_KEY_DESC_NONCE];
	UCHAR           R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR           PTK[64];
	UCHAR           ReTryCounter;   
	RALINK_TIMER_STRUCT                 RetryTimer;
	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;   // This should match to whatever microsoft defined
	NDIS_802_11_WEP_STATUS              WepStatus;
	NDIS_802_11_WEP_STATUS              GroupKeyWepStatus;
	UINT8    		WpaState;
	UINT8       	GTKState;
	USHORT          PortSecured;
	NDIS_802_11_PRIVACY_FILTER  PrivacyFilter;      // PrivacyFilter enum for 802.1X
	CIPHER_KEY      PairwiseKey;
	PVOID           pAd;
	INT				PMKID_CacheIdx;
	UCHAR			PMKID[LEN_PMKID];
	UCHAR			NegotiatedAKM[LEN_OUI_SUITE];	// It indicate the negotiated AKM suite

#ifdef WAPI_SUPPORT
	UCHAR			usk_id;					// unicast key index for WPI
#endif // WAPI_SUPPORT //

	UCHAR           Addr[MAC_ADDR_LEN];
	UCHAR           PsMode;
	SST             Sst;
	AUTH_STATE      AuthState; // for SHARED KEY authentication state machine used only
	BOOLEAN			IsReassocSta;	// Indicate whether this is a reassociation procedure
	USHORT          Aid;
	USHORT          CapabilityInfo;
	UCHAR           LastRssi;
	ULONG           NoDataIdleCount;
	UINT16			StationKeepAliveCount; // unit: second
	ULONG           PsQIdleCount;
	QUEUE_HEADER    PsQueue;

	UINT32			StaConnectTime;		// the live time of this station since associated with AP 
	UINT32			StaIdleTimeout;		/* idle timeout per entry */

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
	/* these UAPSD states are used on the fly */
	/* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	BOOLEAN			bAPSDCapablePerAC[4]; /* for trigger-enabled */
	BOOLEAN			bAPSDDeliverEnabledPerAC[4]; /* for delivery-enabled */

#ifdef WMM_ACM_SUPPORT
	/* because TSPEC can modify the UAPSD flag, we need to keep the APSD flag
		requested in (re)association stage from the station;
		we need to recover the APSD flag after the TSPEC is deleted. */
	BOOLEAN			bACMAPSDBackup[4];
	BOOLEAN			bACMAPSDBackupDeliverEnabled[4];
#endif // WMM_ACM_SUPPORT //

    UCHAR			MaxSPLength;

    BOOLEAN         bAPSDAllAC; /* 1: all AC are delivery-enabled U-APSD */

    QUEUE_HEADER    UAPSDQueue[WMM_NUM_OF_AC]; /* queue for each U-APSD */
    USHORT          UAPSDQIdleCount;           /* U-APSD queue timeout */

    PQUEUE_ENTRY    pUAPSDEOSPFrame;    /* the last U-APSD frame */
    USHORT          UAPSDTxNum;         /* total U-APSD frame number */
    BOOLEAN         bAPSDFlagEOSPOK;    /* 1: EOSP frame is tx by ASIC */
    BOOLEAN         bAPSDFlagSPStart;   /* 1: SP is started */

	/* need to use unsigned long, because time parameters in OS is defined as
		unsigned long */
	unsigned long	UAPSDTimeStampLast;	/* unit: 1000000/OS_HZ */
	BOOLEAN			bAPSDFlagSpRoughUse;/* 1: use rough SP (default: accurate) */

	/* we will set the flag when PS-poll frame is received and
		clear it when statistics handle.
		if the flag is set when PS-poll frame is received then calling
		statistics handler to clear it. */
	BOOLEAN			bAPSDFlagLegacySent;/* 1: Legacy PS sent but
											yet statistics handle */

#endif // UAPSD_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#ifdef DOT11_N_SUPPORT
	BOOLEAN			bSendBAR;
	USHORT			NoBADataCountDown;

	UINT32   		CachedBuf[16];		// UINT (4 bytes) for alignment

#endif // DOT11_N_SUPPORT //

	UINT			FIFOCount;
	UINT			DebugFIFOCount;
	UINT			DebugTxCount;
    BOOLEAN			bDlsInit;

//====================================================
//WDS entry needs these
// if ValidAsWDS==TRUE, MatchWDSTabIdx is the index in WdsTab.MacTab
	UINT			MatchWDSTabIdx;
	UCHAR           MaxSupportedRate;
	UCHAR           CurrTxRate;
	UCHAR           CurrTxRateIndex;

//#ifdef RTMP_RBUS_SUPPORT
#ifdef NEW_RATE_ADAPT_SUPPORT
	UCHAR			lastRateIdx;
	UCHAR			fewPktsCnt;
	BOOLEAN			perThrdAdj;
#endif // NEW_RATE_ADAPT_SUPPORT //
	UCHAR			mcsGroup;//the mcs group to be tried
	UCHAR			lastLegalMfb;//last legal mfb which is used to set rate
	BOOLEAN			isMfbChanged;//purpose: true when mfb has changed but the new mfb is not adopted for Tx
	PRTMP_TX_RATE_SWITCH	LegalMfbRS;
	BOOLEAN			fLastChangeAccordingMfb;
	NDIS_SPIN_LOCK	fLastChangeAccordingMfbLock;
//Tx MRQ
	BOOLEAN			toTxMrq;
	UCHAR			msiToTx, mrqCnt;//mrqCnt is used to count down the inverted-BF mrq to be sent
//Rx mfb
	UCHAR			pendingMfsi;
//Tx MFB
	BOOLEAN			toTxMfb;
	UCHAR			mfbToTx;
//	UCHAR 		legalMfbOld;
	UCHAR		bfState;
	UCHAR		bestMethod;
	UCHAR		mfb0, mfb1; 
//	BOOLEAN		isTxBFOld;
	BOOLEAN		toTxSndg;
	UCHAR		sndgMcs;
	UCHAR		sndgBW;
	UCHAR		sndgRateIdx;
	BOOLEAN		ndpSndg;
	UCHAR		sndg0Mcs, bf0Mcs, sndg0RateIdx, bf0RateIdx;
	INT			sndg0Snr0, sndg0Snr1, sndg0Snr2;
	UCHAR		sndg1Mcs, bf1Mcs, sndg1RateIdx, bf1RateIdx;
	INT			sndg1Snr0, sndg1Snr1, sndg1Snr2;
	UCHAR		noSndgCnt;
	UCHAR		eTxBfEnCond, noSndgCntThrd, ndpSndgStreams;
	BOOLEAN		useNewRateAdapt;
//#endif // RTMP_RBUS_SUPPORT //

	// to record the each TX rate's quality. 0 is best, the bigger the worse.
	USHORT          TxQuality[MAX_STEP_OF_TX_RATE_SWITCH];
//	USHORT          OneSecTxOkCount;
	UINT32			OneSecTxNoRetryOkCount;
	UINT32          OneSecTxRetryOkCount;
	UINT32          OneSecTxFailCount;
	UINT32			ContinueTxFailCnt;
	UINT32          CurrTxRateStableTime; // # of second in current TX rate
	UCHAR           TxRateUpPenalty;      // extra # of second penalty due to last unstable condition
#ifdef WDS_SUPPORT
	BOOLEAN		LockEntryTx; // TRUE = block to WDS Entry traffic, FALSE = not. 
#endif // WDS_SUPPORT //
	ULONG		TimeStamp_toTxRing;

//====================================================

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT			MatchAPCLITabIdx; // indicate the index in ApCfg.ApCliTab. 
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //



	BOOLEAN			fLastSecAccordingRSSI;
	UCHAR           LastSecTxRateChangeAction; // 0: no change, 1:rate UP, 2:rate down
	CHAR			LastTimeTxRateChangeAction; //Keep last time value of LastSecTxRateChangeAction	
	ULONG			LastTxOkCount;
	UCHAR           PER[MAX_STEP_OF_TX_RATE_SWITCH];

	// a bitmap of BOOLEAN flags. each bit represent an operation status of a particular 
	// BOOLEAN control, either ON or OFF. These flags should always be accessed via
	// CLIENT_STATUS_TEST_FLAG(), CLIENT_STATUS_SET_FLAG(), CLIENT_STATUS_CLEAR_FLAG() macros.
	// see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition. fCLIENT_STATUS_AMSDU_INUSED
	ULONG           ClientStatusFlags;

	HTTRANSMIT_SETTING	HTPhyMode, MaxHTPhyMode, MinHTPhyMode;// For transmit phy setting in TXWI.
	
#ifdef DOT11_N_SUPPORT
	// HT EWC MIMO-N used parameters
	USHORT		RXBAbitmap;	// fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format
	USHORT		TXBAbitmap;	// This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI 
	USHORT		TXAutoBAbitmap;
	USHORT		BADeclineBitmap;
	USHORT		BARecWcidArray[NUM_OF_TID];	// The mapping wcid of recipient session. if RXBAbitmap bit is masked
	USHORT		BAOriWcidArray[NUM_OF_TID]; // The mapping wcid of originator session. if TXBAbitmap bit is masked
	USHORT		BAOriSequence[NUM_OF_TID]; // The mapping wcid of originator session. if TXBAbitmap bit is masked

	// 802.11n features.
	UCHAR		MpduDensity;
	UCHAR		MaxRAmpduFactor;
	UCHAR		AMsduSize;
	UCHAR		MmpsMode;	// MIMO power save more. 

	HT_CAPABILITY_IE		HTCapability;

#ifdef DOT11N_DRAFT3
	UCHAR		BSS2040CoexistenceMgmtSupport;
	BOOLEAN		bForty_Mhz_Intolerant;
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //


	BOOLEAN		bAutoTxRateSwitch;

	UCHAR       RateLen;
	struct _MAC_TABLE_ENTRY *pNext;
    USHORT      TxSeq[NUM_OF_TID];
	USHORT		NonQosDataSeq;

	RSSI_SAMPLE	RssiSample;
	UINT32 LastRxRate;
	

    BOOLEAN                 bWscCapable;
    UCHAR                   Receive_EapolStart_EapRspId;

	UINT32			TXMCSExpected[MAX_MCS_SET];
	UINT32			TXMCSSuccessful[MAX_MCS_SET];
	UINT32			TXMCSFailed[MAX_MCS_SET];
	UINT32			TXMCSAutoFallBack[MAX_MCS_SET][MAX_MCS_SET];


#ifdef WAPI_SUPPORT
	BOOLEAN                 WapiUskRekeyTimerRunning; 
    RALINK_TIMER_STRUCT     WapiUskRekeyTimer;
	UINT32           		wapi_usk_rekey_cnt;
#endif // WAPI_SUPPORT //

#ifdef WMM_ACM_SUPPORT
/* reference to acm_extr.h */
/* 8 pointers for IN TSPEC and 8 pointers for OUT TSEPC */
#define WMM_STA_ACM_INFO_SIZE	(sizeof(UINT8 *)*16)
	UCHAR			ACM_Info[WMM_STA_ACM_INFO_SIZE];

	UCHAR			ACM_NumOfTspecIn;
	UCHAR			ACM_NumOfTspecOut;

	UCHAR			ACM_NumOfOutTspecInAc[4];
	UCHAR			ACM_NumOfInTspecInAc[4];
#endif // WMM_ACM_SUPPORT //

	ULONG AssocDeadLine;




	ULONG ChannelQuality;  // 0..100, Channel Quality Indication for Roaming




#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif // CONFIG_AP_SUPPORT //
#ifdef VENDOR_FEATURE1_SUPPORT
	// total 128B, use UINT32 to avoid alignment problem
	UINT32			HeaderBuf[32];	// (total 128B) TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP

	UCHAR			HdrPadLen;		// recording Header Padding Length;
	UCHAR			MpduHeaderLen;
	UINT16			Protocol;
#endif // VENDOR_FEATURE1_SUPPORT //

#ifdef AGS_SUPPORT
	AGS_CONTROL AGSCtrl; // AGS control
#endif // AGS_SUPPORT //


#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG TxBytes;
	ULONG RxBytes;
#endif // CONFIG_AP_SUPPORT //

} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

typedef struct _MAC_TABLE {
	USHORT			Size;
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];
	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	QUEUE_HEADER    McastPsQueue;
	ULONG           PsQIdleCount;
	BOOLEAN         fAnyStationInPsm;   
	BOOLEAN         fAnyStationBadAtheros;	// Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip. 
	BOOLEAN			fAnyTxOPForceDisable;	// Check if it is necessary to disable BE TxOP
	BOOLEAN			fAllStationAsRalink; 	// Check if all stations are ralink-chipset 
#ifdef DOT11_N_SUPPORT	
	BOOLEAN         fAnyStationIsLegacy;	// Check if I use legacy rate to transmit to my BSS Station/
	BOOLEAN         fAnyStationNonGF;		// Check if any Station can't support GF.
	BOOLEAN         fAnyStation20Only;		// Check if any Station can't support GF.
	BOOLEAN			fAnyStationMIMOPSDynamic; // Check if any Station is MIMO Dynamic
	BOOLEAN         fAnyBASession;   // Check if there is BA session.  Force turn on RTS/CTS 
#ifdef DOT11N_DRAFT3
	BOOLEAN		fAnyStaFortyIntolerant;		// Check if still has any station set the Intolerant bit on!
#endif // DOT11N_DRAFT3 //

//2008/10/28: KH add to support Antenna power-saving of AP<--
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN         fAnyStationIsHT;   // Check if there is 11n STA.  Force turn off AP MIMO PS 
#endif // CONFIG_AP_SUPPORT //
//2008/10/28: KH add to support Antenna power-saving of AP-->
#endif // DOT11_N_SUPPORT //
#ifdef WAPI_SUPPORT
	BOOLEAN			fAnyWapiStation;
#endif // WAPI_SUPPORT //
} MAC_TABLE, *PMAC_TABLE;



#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP WDS related data structures
  **************************************************************************/
#ifdef WDS_SUPPORT
typedef struct _WDS_COUNTER {
	LARGE_INTEGER   ReceivedFragmentCount;
	LARGE_INTEGER   TransmittedFragmentCount;
	ULONG           ReceivedByteCount;
	ULONG           TransmittedByteCount; 
	ULONG       RxErrors;
	ULONG       TxErrors;
	LARGE_INTEGER   MulticastReceivedFrameCount;
	ULONG       OneCollision;
	ULONG       MoreCollisions;
	ULONG       RxNoBuffer;
	ULONG       RcvAlignmentErrors;
} WDS_COUNTER, *PWDS_COUNTER;

typedef struct _WDS_ENTRY {
	BOOLEAN         Valid;
	UCHAR           Addr[MAC_ADDR_LEN];
	ULONG           NoDataIdleCount;
	struct _WDS_ENTRY *pNext;
} WDS_ENTRY, *PWDS_ENTRY;

typedef struct  _WDS_TABLE_ENTRY {
	USHORT			Size;
	UCHAR           WdsAddr[MAC_ADDR_LEN];
	WDS_ENTRY       *Hash[HASH_TABLE_SIZE];
	WDS_ENTRY       Content[MAX_LEN_OF_MAC_TABLE];
	UCHAR           MaxSupportedRate;
	UCHAR           CurrTxRate;
	USHORT          TxQuality[MAX_LEN_OF_SUPPORTED_RATES];
	USHORT          OneSecTxOkCount;
	USHORT          OneSecTxRetryOkCount;
	USHORT          OneSecTxFailCount;
	ULONG           CurrTxRateStableTime; // # of second in current TX rate
	UCHAR           TxRateUpPenalty;      // extra # of second penalty due to last unstable condition
} WDS_TABLE_ENTRY, *PWDS_TABLE_ENTRY;

typedef struct _RT_802_11_WDS_ENTRY {
	PNET_DEV			dev;
	UCHAR				Valid;
	UCHAR				PhyMode;
	UCHAR				PeerWdsAddr[MAC_ADDR_LEN];
	UCHAR				MacTabMatchWCID;	// ASIC
	NDIS_802_11_WEP_STATUS  WepStatus;
	UCHAR					KeyIdx;
	CIPHER_KEY          	WdsKey;
	HTTRANSMIT_SETTING				HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	RT_HT_PHY_INFO					DesiredHtPhyInfo;
	BOOLEAN							bAutoTxRateSwitch;
	DESIRED_TRANSMIT_SETTING       	DesiredTransmitSetting; // Desired transmit setting.
	WDS_COUNTER WdsCounter;
} RT_802_11_WDS_ENTRY, *PRT_802_11_WDS_ENTRY;

typedef struct _WDS_TABLE {
	UCHAR               Mode;
	ULONG               Size;
	RT_802_11_WDS_ENTRY	WdsEntry[MAX_WDS_ENTRY];
} WDS_TABLE, *PWDS_TABLE;
#endif // WDS_SUPPORT //

/***************************************************************************
  *	AP APCLI related data structures
  **************************************************************************/
typedef struct _APCLI_STRUCT {
	PNET_DEV				dev;
#ifdef RTL865X_SOC
	unsigned int            mylinkid;
#endif
	BOOLEAN                 Enable;	// Set it as 1 if the apcli interface was configured to "1"  or by iwpriv cmd "ApCliEnable"
	BOOLEAN                 Valid;	// Set it as 1 if the apcli interface associated success to remote AP.
	UCHAR					MacTabWCID;	//WCID value, which point to the entry of ASIC Mac table.
	UCHAR                   SsidLen;
	CHAR                    Ssid[MAX_LEN_OF_SSID];

	UCHAR                   CfgSsidLen;
	CHAR                    CfgSsid[MAX_LEN_OF_SSID];
	UCHAR                   CfgApCliBssid[ETH_LENGTH_OF_ADDRESS];
	UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];

	ULONG                   ApCliRcvBeaconTime;
	ULONG			ApCliLinkUpTime;

	ULONG                   CtrlCurrState;
	ULONG                   SyncCurrState;
	ULONG                   AuthCurrState;
	ULONG                   AssocCurrState;
	ULONG					WpaPskCurrState;

	USHORT                  AuthReqCnt;
	USHORT                  AssocReqCnt;

	ULONG                   ClientStatusFlags;
	UCHAR                   MpduDensity;

	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;   // This should match to whatever microsoft defined
	NDIS_802_11_WEP_STATUS              WepStatus;

	// Add to support different cipher suite for WPA2/WPA mode
	NDIS_802_11_ENCRYPTION_STATUS		GroupCipher;		// Multicast cipher suite
	NDIS_802_11_ENCRYPTION_STATUS		PairCipher;			// Unicast cipher suite
	BOOLEAN								bMixCipher;			// Indicate current Pair & Group use different cipher suites
	USHORT								RsnCapability;
	
	UCHAR		PSK[100];				// reserve PSK key material
	UCHAR       PSKLen;
	UCHAR       PMK[32];                // WPA PSK mode PMK
	//UCHAR       PTK[64];                // WPA PSK mode PTK
	UCHAR		GTK[32];				// GTK from authenticator

	//CIPHER_KEY		PairwiseKey;
	CIPHER_KEY      SharedKey[SHARE_KEY_NUM];
	UCHAR           DefaultKeyId;

	// WPA 802.1x port control, WPA_802_1X_PORT_SECURED, WPA_802_1X_PORT_NOT_SECURED
	//UCHAR       PortSecured;
    
	// store RSN_IE built by driver
	UCHAR		RSN_IE[MAX_LEN_OF_RSNIE];  // The content saved here should be convert to little-endian format.
	UCHAR		RSNIE_Len; 

	// For WPA countermeasures
	ULONG       LastMicErrorTime;   // record last MIC error time
	//ULONG       MicErrCnt;          // Should be 0, 1, 2, then reset to zero (after disassoiciation).
	BOOLEAN                 bBlockAssoc; // Block associate attempt for 60 seconds after counter measure occurred.

	// For WPA-PSK supplicant state
	//WPA_STATE   	WpaState;           // Default is SS_NOTUSE
	//UCHAR       	ReplayCounter[8];
	//UCHAR       	ANonce[32];         // ANonce for WPA-PSK from authenticator
	UCHAR       	SNonce[32];         // SNonce for WPA-PSK
	UCHAR			GNonce[32];			// GNonce for WPA-PSK from authenticator
	
#ifdef WSC_AP_SUPPORT
	WSC_CTRL	           WscControl;
#endif // WSC_AP_SUPPORT //	

	HTTRANSMIT_SETTING				HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	RT_HT_PHY_INFO					DesiredHtPhyInfo;
	BOOLEAN							bAutoTxRateSwitch;
	DESIRED_TRANSMIT_SETTING       	DesiredTransmitSetting; // Desired transmit setting.
	UCHAR							RxMcsSet[16];
} APCLI_STRUCT, *PAPCLI_STRUCT;


typedef struct _AP_ADMIN_CONFIG {
	USHORT          CapabilityInfo;
	// Multiple SSID
    UCHAR               BssidNum;
	UCHAR				MacMask;	
    MULTISSID_STRUCT    MBSSID[MAX_MBSSID_NUM_CONS];
	ULONG				IsolateInterStaTrafficBTNBSSID;

#ifdef HOSTAPD_SUPPORT
	BOOLEAN Hostapd;
	BOOLEAN HostapdWPS;
#endif //HOSTAPD_SUPPORT//


#ifdef APCLI_SUPPORT
	UCHAR				ApCliInfRunned;				// Number of  ApClient interface which was running. value from 0 to MAX_APCLI_INTERFACE
	APCLI_STRUCT		ApCliTab[MAX_APCLI_NUM];		//AP-client 
#endif // APCLI_SUPPORT //

	// for wpa
	RALINK_TIMER_STRUCT         CounterMeasureTimer;

	UCHAR           CMTimerRunning;
	UCHAR           BANClass3Data;
	LARGE_INTEGER   aMICFailTime;
	LARGE_INTEGER   PrevaMICFailTime;
	ULONG           MICFailureCounter;

	RSSI_SAMPLE		RssiSample;
	ULONG           NumOfAvgRssiSample;

	BOOLEAN         bAutoChannelAtBootup;  // 0: disable, 1: enable
	ChannelSel_Alg  AutoChannelAlg;        // Alg for selecting Channel
	BOOLEAN         bAvoidDfsChannel; 	 	// 0: disable, 1: enable
	BOOLEAN         bIsolateInterStaTraffic;
	BOOLEAN         bHideSsid;

	// temporary latch for Auto channel selection
	ULONG           ApCnt;  // max RSSI during Auto Channel Selection period
	UCHAR           AutoChannel_Channel;  // channel number during Auto Channel Selection
	UCHAR           current_channel_index;  // current index of channel list 
	UCHAR 		AutoChannelSkipListNum; 		// number of rejected channel list
	UCHAR		AutoChannelSkipList[10];
	UCHAR           DtimCount;      // 0.. DtimPeriod-1
	UCHAR           DtimPeriod;     // default = 3
	UCHAR           ErpIeContent;
	ULONG           LastOLBCDetectTime;
	ULONG			LastNoneHTOLBCDetectTime;
	ULONG       	LastScanTime;       // Record last scan time for issue BSSID_SCAN_LIST

	UCHAR       	LastSNR0;             // last received BEACON's SNR
	UCHAR       	LastSNR1;            // last received BEACON's SNR for 2nd  antenna

#ifdef DOT1X_SUPPORT
	// dot1x related parameter
	UINT32			own_ip_addr;
	UINT32			retry_interval;
	UINT32			session_timeout_interval;
	UINT32			quiet_interval;
	UCHAR 			EAPifname[MAX_MBSSID_NUM_CONS][IFNAMSIZ];		// indicate as the binding interface for EAP negotiation.
	UCHAR			EAPifname_len[MAX_MBSSID_NUM_CONS];
	UCHAR 			PreAuthifname[MAX_MBSSID_NUM_CONS][IFNAMSIZ];	// indicate as the binding interface for WPA2 Pre-authentication.
	UCHAR			PreAuthifname_len[MAX_MBSSID_NUM_CONS];
#endif // DOT1X_SUPPORT //

	// EDCA parameters to be announced to its local BSS
	EDCA_PARM       BssEdcaParm;

	RALINK_TIMER_STRUCT		ApQuickResponeForRateUpTimer;
	BOOLEAN					ApQuickResponeForRateUpTimerRunning;

#ifdef IDS_SUPPORT
	// intrusion detection parameter	
	BOOLEAN					IdsEnable;
	UINT32					AuthFloodThreshold;		// Authentication frame flood threshold
	UINT32					AssocReqFloodThreshold;		// Association request frame flood threshold
	UINT32					ReassocReqFloodThreshold;	// Re-association request frame flood threshold
	UINT32					ProbeReqFloodThreshold;		// Probe request frame flood threshold
	UINT32					DisassocFloodThreshold;		// Disassociation frame flood threshold
	UINT32					DeauthFloodThreshold;		// Deauthentication frame flood threshold
	UINT32					EapReqFloodThreshold;		// EAP request frame flood threshold

	UINT32					RcvdAuthCount;	
	UINT32					RcvdAssocReqCount;		
	UINT32					RcvdReassocReqCount;	
	UINT32					RcvdProbeReqCount;		
	UINT32					RcvdDisassocCount;		
	UINT32					RcvdDeauthCount;		
	UINT32					RcvdEapReqCount;	

	RALINK_TIMER_STRUCT		IDSTimer;
	BOOLEAN					IDSTimerRunning;
#endif // IDS_SUPPORT //

	/* Indicate the maximum idle timeout */
	UINT32					StaIdleTimeout;

	ULONG					EntryLifeCheck;

#ifdef IGMP_SNOOP_SUPPORT
	BOOLEAN					IgmpSnoopEnable;	// 0: disable, 1: enable.
#endif // IGMP_SNOOP_SUPPORT //


#ifdef CLIENT_WDS
	NDIS_SPIN_LOCK CliWdsTabLock;
	PCLIWDS_PROXY_ENTRY pCliWdsEntryPool;
	LIST_HEADER CliWdsEntryFreeList;
	LIST_HEADER	CliWdsProxyTab[CLIWDS_HASH_TAB_SIZE];
#endif // CLIENT_WDS //
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	UCHAR 				GreenAPLevel;
	BOOLEAN					bGreenAPEnable;
	BOOLEAN					bGreenAPActive;
#endif // GREENAP_SUPPORT //

#endif // DOT11_N_SUPPORT //

	UCHAR	EntryClientCount;
} AP_ADMIN_CONFIG, *PAP_ADMIN_CONFIG;

#ifdef IGMP_SNOOP_SUPPORT
typedef enum _IGMP_GROUP_TYPE
{
	MODE_IS_INCLUDE = 1,
	MODE_IS_EXCLUDE,
	CHANGE_TO_INCLUDE_MODE,
	CHANGE_TO_EXCLUDE_MODE,
	ALLOW_NEW_SOURCES,
	BLOCK_OLD_SOURCES
} IgmpGroupType;

typedef enum _MULTICAST_FILTER_ENTRY_TYPE
{
	MCAT_FILTER_STATIC = 0,
	MCAT_FILTER_DYNAMIC,
} MulticastFilterEntryType;

typedef struct _MEMBER_ENTRY
{
	struct _MEMBER_ENTRY *pNext;
	UCHAR Addr[MAC_ADDR_LEN];
//	USHORT Aid;
} MEMBER_ENTRY, *PMEMBER_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE_ENTRY
{
	BOOLEAN Valid;
	MulticastFilterEntryType type; // 0: static, 1: dynamic.
	UINT  lastTime;
	PNET_DEV net_dev;
	UCHAR Addr[MAC_ADDR_LEN];
	LIST_HEADER MemberList;
	struct _MULTICAST_FILTER_TABLE_ENTRY *pNext;
} MULTICAST_FILTER_TABLE_ENTRY, *PMULTICAST_FILTER_TABLE_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE
{
	UCHAR Size;
	PMULTICAST_FILTER_TABLE_ENTRY Hash[MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE];
	MULTICAST_FILTER_TABLE_ENTRY Content[MAX_LEN_OF_MULTICAST_FILTER_TABLE];
	NDIS_SPIN_LOCK MulticastFilterTabLock;
	NDIS_SPIN_LOCK FreeMemberPoolTabLock;
	MEMBER_ENTRY freeMemberPool[FREE_MEMBER_POOL_SIZE];
	LIST_HEADER freeEntryList;
} MULTICAST_FILTER_TABLE, *PMULTICAST_FILTER_TABLE;
#endif // IGMP_SNOOP_SUPPORT //




#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
typedef enum _RT_GREEN_AP_LEVEL
{
	GREENAP_11BGN_STAS=0,
	GREENAP_ONLY_11BG_STAS,
	GREENAP_WITHOUT_ANY_STAS_CONNECT
} RT_GREEN_AP_LEVEL;
#endif // DOT11_N_SUPPORT //
#endif // GREENAP_SUPPORT //

// ----------- end of AP ----------------------------
#endif // CONFIG_AP_SUPPORT //

#ifdef BLOCK_NET_IF
typedef struct _BLOCK_QUEUE_ENTRY
{
	BOOLEAN SwTxQueueBlockFlag;
	LIST_HEADER NetIfList;
} BLOCK_QUEUE_ENTRY, *PBLOCK_QUEUE_ENTRY;
#endif // BLOCK_NET_IF //


#ifdef SPECIFIC_BCN_BUF_SUPPORT
typedef struct _BCN_PARAM_STRUCT
{
	BOOLEAN					bHighShareMemSupport;
	UINT32					DYN_HW_BEACON_BASE0;
	UINT32					DYN_HW_BEACON_MAX_SIZE;
	UINT8					DYN_HW_BEACON_MAX_COUNT;
	UINT16					DYN_HW_RESERVED_WCID;
	UINT8					DYN_MAX_MBSSID_NUM;
}BCN_PARAM_STRUCT, *PBCN_PARAM_STRUCT;
#endif // SPECIFIC_BCN_BUF_SUPPORT //

struct wificonf
{
	BOOLEAN	bShortGI;
	BOOLEAN bGreenField;
};


typedef struct _RTMP_DEV_INFO_
{
	UCHAR			chipName[16];
	RTMP_INF_TYPE	infType;
}RTMP_DEV_INFO;


#ifdef DBG_DIAGNOSE
#define DIAGNOSE_TIME	10   // 10 sec
typedef struct _RtmpDiagStrcut_
{	// Diagnosis Related element
	unsigned char		inited;
	unsigned char 	qIdx;
	unsigned char 	ArrayStartIdx;
	unsigned char		ArrayCurIdx;
	// Tx Related Count
	USHORT			TxDataCnt[DIAGNOSE_TIME];
	USHORT			TxFailCnt[DIAGNOSE_TIME];				
//	USHORT			TxDescCnt[DIAGNOSE_TIME][16];		// TxDesc queue length in scale of 0~14, >=15
	USHORT			TxDescCnt[DIAGNOSE_TIME][24]; // 3*3	// TxDesc queue length in scale of 0~14, >=15
//	USHORT			TxMcsCnt[DIAGNOSE_TIME][16];			// TxDate MCS Count in range from 0 to 15, step in 1.
	USHORT			TxMcsCnt[DIAGNOSE_TIME][MAX_MCS_SET]; // 3*3
	USHORT			TxSWQueCnt[DIAGNOSE_TIME][9];		// TxSwQueue length in scale of 0, 1, 2, 3, 4, 5, 6, 7, >=8

	USHORT			TxAggCnt[DIAGNOSE_TIME];
	USHORT			TxNonAggCnt[DIAGNOSE_TIME];
//	USHORT			TxAMPDUCnt[DIAGNOSE_TIME][16];		// 10 sec, TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1.
	USHORT			TxAMPDUCnt[DIAGNOSE_TIME][MAX_MCS_SET]; // 3*3 // 10 sec, TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1.
	USHORT			TxRalinkCnt[DIAGNOSE_TIME];			// TxRalink Aggregation Count in 1 sec scale.
	USHORT			TxAMSDUCnt[DIAGNOSE_TIME];			// TxAMSUD Aggregation Count in 1 sec scale.

	// Rx Related Count
	USHORT			RxDataCnt[DIAGNOSE_TIME];			// Rx Total Data count.
	USHORT			RxCrcErrCnt[DIAGNOSE_TIME];
//	USHORT			RxMcsCnt[DIAGNOSE_TIME][16];		// Rx MCS Count in range from 0 to 15, step in 1.
	USHORT			RxMcsCnt[DIAGNOSE_TIME][MAX_MCS_SET]; // 3*3
}RtmpDiagStruct;
#endif // DBG_DIAGNOSE //


struct _RTMP_CHIP_OP_
{
	/*  Calibration access related callback functions */
	int (*eeinit)(RTMP_ADAPTER *pAd);										/* int (*eeinit)(RTMP_ADAPTER *pAd); */
	int (*eeread)(RTMP_ADAPTER *pAd, USHORT offset, PUSHORT pValue);				/* int (*eeread)(RTMP_ADAPTER *pAd, int offset, PUSHORT pValue); */
	int (*eewrite)(RTMP_ADAPTER *pAd, USHORT offset, USHORT value);				/* int (*eewrite)(RTMP_ADAPTER *pAd, int offset, USHORT value); */

	/* MCU related callback functions */
	int (*loadFirmware)(RTMP_ADAPTER *pAd);								/* int (*loadFirmware)(RTMP_ADAPTER *pAd); */
	int (*eraseFirmware)(RTMP_ADAPTER *pAd);								/* int (*eraseFirmware)(RTMP_ADAPTER *pAd); */
	int (*sendCommandToMcu)(RTMP_ADAPTER *pAd, BOOLEAN bNeedSpinLock, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1, BOOLEAN bNeedMcuCmdLock);	/* int (*sendCommandToMcu)(RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1); */

	/* RF access related callback functions */
	REG_PAIR *pRFRegTable;
	REG_PAIR *pBBPRegTable;
	UCHAR	bbpRegTbSize;
	void (*AsicRfInit)(RTMP_ADAPTER *pAd);
	void (*AsicRfTurnOn)(RTMP_ADAPTER *pAd);
	void (*AsicRfTurnOff)(RTMP_ADAPTER *pAd);
	void (*AsicReverseRfFromSleepMode)(RTMP_ADAPTER *pAd);
	void (*AsicHaltAction)(RTMP_ADAPTER *pAd);

	void (*SetRxAnt)(RTMP_ADAPTER	*pAd, UCHAR	Ant);
    void (*HwAntEnable)(RTMP_ADAPTER *pAd);

};

struct _RTMP_CHIP_CAP_ {
	BOOLEAN FlgIsHwAntennaDiversitySup;
#ifdef TXRX_SW_ANTDIV_SUPPORT
	BOOLEAN bTxRxSwAntDiv;
#endif /* TXRX_SW_ANTDIV_SUPPORT */

};
#ifdef RTMP_INTERNAL_TX_ALC
//
// The Tx power control using the internal ALC
//
typedef struct _TX_POWER_CONTROL
{
	BOOLEAN bInternalTxALC; // Internal Tx ALC
#ifdef RT53xx
	BOOLEAN bExtendedTssiMode; // The extended TSSI mode (each channel has different Tx power if needed)
	CHAR PerChTxPwrOffset[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1]; // Per-channel Tx power offset
#if defined(RT5390) || defined(RT5370)
	//
	// RT5392 temperature compensation
	//
	// lookup table, 33 elements (-7, -6....0, 1, 2...25)
	INT LookupTable[33];

	//Reference temperature, from EEPROM
	INT RefTempG;

	// Index offset, -7....25.
	INT LookupTableIndex;
	
	CHAR idxTxPowerTable2; // The index of the Tx power table
#endif // defined(RT5390) || defined(RT5370) //
#endif // RT53xx //
	CHAR idxTxPowerTable; // The index of the Tx power table

	CHAR	RF_TX0_ALC; // 3390: RF R12[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC
	CHAR	MAC_PowerDelta; // Tx power control over MAC 0x1314~0x1324
} TX_POWER_CONTROL, *PTX_POWER_CONTROL;
#endif // RTMP_INTERNAL_TX_ALC //

//
// The entry of transmit power control over MAC
//
typedef struct _TX_POWER_CONTROL_OVER_MAC_ENTRY
{
	USHORT	MACRegisterOffset; // MAC register offset
	ULONG	RegisterValue; // Register value
} TX_POWER_CONTROL_OVER_MAC_ENTRY, *PTX_POWER_CONTROL_OVER_MAC_ENTRY;

//
// The maximum registers of transmit power control
//
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS 5



//
// The configuration of the transmit power control over MAC
//
typedef struct _CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC
{
	UCHAR								NumOfEntries; // Number of entries
	TX_POWER_CONTROL_OVER_MAC_ENTRY	TxPwrCtrlOverMAC[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
} CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC, *PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC;

//
// The extension of the transmit power control over MAC
//
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC
{
	struct
	{
		ULONG	TxPwrCfg0;		 // MAC 0x1314
		ULONG	TxPwrCfg0Ext;	 // MAC 0x1390
		ULONG	TxPwrCfg1;		 // MAC 0x1318
		ULONG	TxPwrCfg1Ext;	 // MAC 0x1394
		ULONG	TxPwrCfg2;		 // MAC 0x131C
		ULONG	TxPwrCfg2Ext;	 // MAC 0x1398
		ULONG	TxPwrCfg3;		 // MAC 0x1320
		ULONG	TxPwrCfg3Ext;	 // MAC 0x139C
		ULONG	TxPwrCfg4;		 // MAC 0x1324
		ULONG	TxPwrCfg4Ext;	 // MAC 0x13A0
		ULONG	TxPwrCfg5;		 // MAC 0x1384
		ULONG	TxPwrCfg6;		 // MAC 0x1388
		ULONG	TxPwrCfg7;		 // MAC 0x13D4
		ULONG	TxPwrCfg8;		 // MAC 0x13D8
		ULONG	TxPwrCfg9;		 // MAC 0x13DC
	} BW20Over2Dot4G;

	struct
	{
		ULONG	TxPwrCfg0;		 // MAC 0x1314
		ULONG	TxPwrCfg0Ext;	 // MAC 0x1390
		ULONG	TxPwrCfg1;		 // MAC 0x1318
		ULONG	TxPwrCfg1Ext;	 // MAC 0x1394
		ULONG	TxPwrCfg2;		 // MAC 0x131C
		ULONG	TxPwrCfg2Ext;	 // MAC 0x1398
		ULONG	TxPwrCfg3;		 // MAC 0x1320
		ULONG	TxPwrCfg3Ext;	 // MAC 0x139C
		ULONG	TxPwrCfg4;		 // MAC 0x1324
		ULONG	TxPwrCfg4Ext;	 // MAC 0x13A0
		ULONG	TxPwrCfg5;		 // MAC 0x1384
		ULONG	TxPwrCfg6;		 // MAC 0x1388
		ULONG	TxPwrCfg7;		 // MAC 0x13D4
		ULONG	TxPwrCfg8;		 // MAC 0x13D8
		ULONG	TxPwrCfg9;		 // MAC 0x13DC
	} BW40Over2Dot4G;

	struct
	{
		ULONG	TxPwrCfg0;		 // MAC 0x1314
		ULONG	TxPwrCfg0Ext;	 // MAC 0x1390
		ULONG	TxPwrCfg1;		 // MAC 0x1318
		ULONG	TxPwrCfg1Ext;	 // MAC 0x1394
		ULONG	TxPwrCfg2;		 // MAC 0x131C
		ULONG	TxPwrCfg2Ext;	 // MAC 0x1398
		ULONG	TxPwrCfg3;		 // MAC 0x1320
		ULONG	TxPwrCfg3Ext;	 // MAC 0x139C
		ULONG	TxPwrCfg4;		 // MAC 0x1324
		ULONG	TxPwrCfg4Ext;	 // MAC 0x13A0
		ULONG	TxPwrCfg5;		 // MAC 0x1384
		ULONG	TxPwrCfg6;		 // MAC 0x1388
		ULONG	TxPwrCfg7;		 // MAC 0x13D4
		ULONG	TxPwrCfg8;		 // MAC 0x13D8
		ULONG	TxPwrCfg9;		 // MAC 0x13DC
	} BW20Over5G;

	struct
	{
		ULONG	TxPwrCfg0;		 // MAC 0x1314
		ULONG	TxPwrCfg0Ext;	 // MAC 0x1390
		ULONG	TxPwrCfg1;		 // MAC 0x1318
		ULONG	TxPwrCfg1Ext;	 // MAC 0x1394
		ULONG	TxPwrCfg2;		 // MAC 0x131C
		ULONG	TxPwrCfg2Ext;	 // MAC 0x1398
		ULONG	TxPwrCfg3;		 // MAC 0x1320
		ULONG	TxPwrCfg3Ext;	 // MAC 0x139C
		ULONG	TxPwrCfg4;		 // MAC 0x1324
		ULONG	TxPwrCfg4Ext;	 // MAC 0x13A0
		ULONG	TxPwrCfg5;		 // MAC 0x1384
		ULONG	TxPwrCfg6;		 // MAC 0x1388
		ULONG	TxPwrCfg7;		 // MAC 0x13D4
		ULONG	TxPwrCfg8;		 // MAC 0x13D8
		ULONG	TxPwrCfg9;		 // MAC 0x13DC
	} BW40Over5G;
} TX_POWER_CONTROL_EXT_OVER_MAC, *PTX_POWER_CONTROL_EXT_OVER_MAC;

typedef struct _BBP_RESET_CTL
{
	#define 		BBP_RECORD_NUM	47
	REG_PAIR 	BBPRegDB[BBP_RECORD_NUM];
	BOOLEAN		AsicCheckEn;
} BBP_RESET_CTL, *PBBP_RESET_CTL;

//
//  The miniport adapter structure
//
struct _RTMP_ADAPTER
{
	PVOID					OS_Cookie;	// save specific structure relative to OS
	PNET_DEV				net_dev;
	ULONG					VirtualIfCnt;

	RTMP_CHIP_OP			chipOps;
	RTMP_CHIP_CAP			chipCap;
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
	USHORT					SameRxByteCount;
	UCHAR					BbpResetCount;	
	UCHAR					BbpResetFlag;
	UCHAR					BbpResetFlagCount;
	UCHAR					BbpResetFlagCountVale;
	BBP_RESET_CTL 			BBPResetCtl;
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */

#ifdef HOSTAPD_SUPPORT
	UINT32               IoctlIF;
#endif//HOSTAPD_SUPPORT//
#ifdef INF_PPA_SUPPORT
	UINT32  g_if_id;
	BOOLEAN	PPAEnable;
	PPA_DIRECTPATH_CB       *pDirectpathCb;
#endif // INF_PPA_SUPPORT //
	
#ifdef RTMP_MAC_PCI
/*****************************************************************************************/
/*      PCI related parameters                                                           								  */
/*****************************************************************************************/
	PUCHAR                  CSRBaseAddress;     // PCI MMIO Base Address, all access will use
	unsigned int			irq_num;
	USHORT                  	DeviceID;           // Read from PCI config
	ULONG				AccessBBPFailCount;
   	BOOLEAN					bPCIclkOff;						// flag that indicate if the PICE power status in Configuration SPace..
	BOOLEAN					bPCIclkOffDisableTx;			// 

	BOOLEAN					brt30xxBanMcuCmd;	//when = 0xff means all commands are ok to set .
	BOOLEAN					b3090ESpecialChip;	//3090E special chip that write EEPROM 0x24=0x9280.
	//ULONG					CheckDmaBusyCount;  // Check Interrupt Status Register Count.

	UINT					int_enable_reg;
	UINT					int_disable_mask;
	UINT					int_pending;

	RTMP_DMABUF             TxBufSpace[NUM_OF_TX_RING]; // Shared memory of all 1st pre-allocated TxBuf associated with each TXD
	RTMP_DMABUF             RxDescRing;                 // Shared memory for RX descriptors
	RTMP_DMABUF             TxDescRing[NUM_OF_TX_RING]; 	// Shared memory for Tx descriptors
	RTMP_TX_RING            TxRing[NUM_OF_TX_RING];     	// AC0~4 + HCCA
#endif // RTMP_MAC_PCI //


	NDIS_SPIN_LOCK		irq_lock;
	UCHAR				irq_disabled;

	//======Cmd Thread in PCI/RBUS/USB
	CmdQ					CmdQ;
	NDIS_SPIN_LOCK			CmdQLock;				// CmdQLock spinlock
	RTMP_OS_TASK			cmdQTask;


/*****************************************************************************************/
/*      RBUS related parameters                                                           								  */
/*****************************************************************************************/

/*****************************************************************************************/
/*      Both PCI/USB related parameters                                                  							  */
/*****************************************************************************************/
	//RTMP_DEV_INFO			chipInfo;
	RTMP_INF_TYPE			infType;

/*****************************************************************************************/
/*      Driver Mgmt related parameters                                                  							  */
/*****************************************************************************************/
	RTMP_OS_TASK			mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
	// If you want use timer task to handle the timer related jobs, enable this.
	RTMP_TIMER_TASK_QUEUE	TimerQ;
	NDIS_SPIN_LOCK			TimerQLock;
	RTMP_OS_TASK			timerTask;
#endif // RTMP_TIMER_TASK_SUPPORT //


/*****************************************************************************************/
/*      Tx related parameters                                                           */
/*****************************************************************************************/
	BOOLEAN                 DeQueueRunning[NUM_OF_TX_RING];  // for ensuring RTUSBDeQueuePacket get call once
	NDIS_SPIN_LOCK          DeQueueLock[NUM_OF_TX_RING];
		

	// resource for software backlog queues
	QUEUE_HEADER            TxSwQueue[NUM_OF_TX_RING];  // 4 AC + 1 HCCA
	NDIS_SPIN_LOCK          TxSwQueueLock[NUM_OF_TX_RING];	// TxSwQueue spinlock
	
	RTMP_DMABUF             MgmtDescRing;               	// Shared memory for MGMT descriptors
	RTMP_MGMT_RING          MgmtRing;
	NDIS_SPIN_LOCK          MgmtRingLock;               	// Prio Ring spinlock

	UCHAR					LastMCUCmd;

/*****************************************************************************************/
/*      Rx related parameters                                                           */
/*****************************************************************************************/

#ifdef RTMP_MAC_PCI
	RTMP_RX_RING            RxRing;
	NDIS_SPIN_LOCK          RxRingLock;                 // Rx Ring spinlock
#if defined(RT3090) || defined(RT3592) || defined(RT3390) || defined(RT3593) || defined(RT5390)
	NDIS_SPIN_LOCK          McuCmdLock;              //MCU Command Queue spinlock
#endif // defined(RT3090) || defined(RT3592) || defined(RT3390) || defined(RT3593) //
#endif // RTMP_MAC_PCI //
	


/*****************************************************************************************/
/*      ASIC related parameters                                                          */
/*****************************************************************************************/
	UINT32               	MACVersion;      	// MAC version. Record rt2860C(0x28600100) or rt2860D (0x28600101)..
	UINT16			ChipId;                /* Chip version. Read from EEPROM 0x00 to identify RT5390H */

	// ---------------------------
	// E2PROM
	// ---------------------------
	ULONG				EepromVersion;          // byte 0: version, byte 1: revision, byte 2~3: unused
	ULONG				FirmwareVersion;        // byte 0: Minor version, byte 1: Major version, otherwise unused.
	USHORT				EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR				EEPROMAddressNum;       // 93c46=6  93c66=8
	BOOLEAN				EepromAccess;
	UCHAR				EFuseTag;


	// ---------------------------
	// BBP Control
	// ---------------------------
#if defined(RTMP_RBUS_SUPPORT) || defined(RT3593) || defined(RT53xx) || defined(RT33xx)
	UCHAR                   BbpWriteLatch[256];     // record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID
#else
	UCHAR                   BbpWriteLatch[140];     // record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID
#endif // defined(RTMP_RBUS_SUPPORT) || defined(RT3593) //
	CHAR					BbpRssiToDbmDelta;		// change from UCHAR to CHAR for high power
	BBP_R66_TUNING          BbpTuning;

	// ----------------------------
	// RFIC control
	// ----------------------------
	UCHAR                   RfIcType;       // RFIC_xxx
	ULONG                   RfFreqOffset;   // Frequency offset for channel switching
	
#ifdef RTMP_MAC_PCI
	UCHAR                   ShowRf;   // Show RF register write for 2880
#endif // RTMP_MAC_PCI //

	RTMP_RF_REGS            LatchRfRegs;    // latch th latest RF programming value since RF IC doesn't support READ

	EEPROM_ANTENNA_STRUC    Antenna;                            // Since ANtenna definition is different for a & g. We need to save it for future reference.
	EEPROM_NIC_CONFIG2_STRUC    NicConfig2;

	// This soft Rx Antenna Diversity mechanism is used only when user set 
	// RX Antenna = DIVERSITY ON
	SOFT_RX_ANT_DIVERSITY   RxAnt;

	CHANNEL_TX_POWER        TxPower[MAX_NUM_OF_CHANNELS];       // Store Tx power value for all channels.
	CHANNEL_TX_POWER        ChannelList[MAX_NUM_OF_CHANNELS];   // list all supported channels for site survey
	//CHANNEL_11J_TX_POWER    TxPower11J[MAX_NUM_OF_11JCHANNELS];       // 802.11j channel and bw
	
	UCHAR                   ChannelListNum;                     // number of channel in ChannelList[]
	UCHAR					Bbp94;
	BOOLEAN					BbpForCCK;
	ULONG		Tx20MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG		Tx20MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG		Tx40MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG		Tx40MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];



	BOOLEAN     bAutoTxAgcA;                // Enable driver auto Tx Agc control
	UCHAR	    TssiRefA;					// Store Tssi reference value as 25 temperature.	
	UCHAR	    TssiPlusBoundaryA[5];		// Tssi boundary for increase Tx power to compensate.
	UCHAR	    TssiMinusBoundaryA[5];		// Tssi boundary for decrease Tx power to compensate.
	UCHAR	    TxAgcStepA;					// Store Tx TSSI delta increment / decrement value
	CHAR		TxAgcCompensateA;			// Store the compensation (TxAgcStep * (idx-1))
	
	BOOLEAN     bAutoTxAgcG;                // Enable driver auto Tx Agc control
	UCHAR	    TssiRefG;					// Store Tssi reference value as 25 temperature.	
	UCHAR	    TssiPlusBoundaryG[5];		// Tssi boundary for increase Tx power to compensate.
	UCHAR	    TssiMinusBoundaryG[5];		// Tssi boundary for decrease Tx power to compensate.
	UCHAR	    TxAgcStepG;					// Store Tx TSSI delta increment / decrement value
	CHAR		TxAgcCompensateG;			// Store the compensation (TxAgcStep * (idx-1))
#ifdef RTMP_INTERNAL_TX_ALC
	TX_POWER_CONTROL TxPowerCtrl; // The Tx power control using the internal ALC
#endif // RTMP_INTERNAL_TX_ALC //


	signed char		BGRssiOffset0;				// Store B/G RSSI#0 Offset value on EEPROM 0x46h
	signed char		BGRssiOffset1;				// Store B/G RSSI#1 Offset value 
	signed char		BGRssiOffset2;				// Store B/G RSSI#2 Offset value 
	
	signed char		ARssiOffset0;				// Store A RSSI#0 Offset value on EEPROM 0x4Ah
	signed char		ARssiOffset1;				// Store A RSSI#1 Offset value 
	signed char		ARssiOffset2;				// Store A RSSI#2 Offset value 
	
	CHAR		BLNAGain;					// Store B/G external LNA#0 value on EEPROM 0x44h
	CHAR		ALNAGain0;					// Store A external LNA#0 value for ch36~64
	CHAR		ALNAGain1;					// Store A external LNA#1 value for ch100~128
	CHAR		ALNAGain2;					// Store A external LNA#2 value for ch132~165
#ifdef RT30xx
	// for 3572	
	UCHAR		Bbp25;	
	UCHAR		Bbp26;
	
	UCHAR		TxMixerGain24G;				// Tx mixer gain value from EEPROM to improve Tx EVM / Tx DAC, 2.4G
	UCHAR		TxMixerGain5G;	
#endif // RT30xx //
	// ----------------------------
	// LED control
	// ----------------------------
	MCU_LEDCS_STRUC		LedCntl;
	USHORT				Led1;	// read from EEPROM 0x3c
	USHORT				Led2;	// EEPROM 0x3e
	USHORT				Led3;	// EEPROM 0x40
	UCHAR				LedIndicatorStrength;
	UCHAR				RssiSingalstrengthOffet;
	BOOLEAN				bLedOnScanning;
	UCHAR				LedStatus;

	// ----------------------------
	// MAC control
	// ----------------------------
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	//UCHAR				ShrMSel;
	NDIS_SPIN_LOCK			ShrMemLock;	
#endif // SPECIFIC_BCN_BUF_SUPPORT //

/*****************************************************************************************/
/*      802.11 related parameters                                                        */
/*****************************************************************************************/
	// outgoing BEACON frame buffer and corresponding TXD 
	TXWI_STRUC              	BeaconTxWI;
	PUCHAR						BeaconBuf;
	USHORT						BeaconOffset[HW_BEACON_MAX_COUNT_CONS];

	// pre-build PS-POLL and NULL frame upon link up. for efficiency purpose.
	HEADER_802_11           	NullFrame;




//=========AP===========
#ifdef CONFIG_AP_SUPPORT
	// -----------------------------------------------
	// AP specific configuration & operation status
	// used only when pAd->OpMode == OPMODE_AP
	// -----------------------------------------------
	AP_ADMIN_CONFIG         ApCfg;             // user configuration when in AP mode
	AP_MLME_AUX             ApMlmeAux;	



#ifdef UAPSD_AP_SUPPORT
	NDIS_SPIN_LOCK          UAPSDEOSPLock; /* EOSP frame access lock use */
	BOOLEAN					bAPSDFlagSPSuspend;	/* 1: SP is suspended; 0: SP is not */
#endif // UAPSD_SUPPORT //

#ifdef WDS_SUPPORT
	WDS_TABLE               WdsTab;            // WDS table when working as an AP
	NDIS_SPIN_LOCK          WdsTabLock;
#endif // WDS_SUPPORT //

#ifdef MBSS_SUPPORT
	BOOLEAN					FlgMbssInit;
#endif // MBSS_SUPPORT //

#ifdef WDS_SUPPORT
	BOOLEAN					flg_wds_init;
#endif // WDS_SUPPORT //

#ifdef APCLI_SUPPORT
	BOOLEAN					flg_apcli_init;
#endif // APCLI_SUPPORT //

	PBSSINFO				pBssInfoTab;
	PCHANNELINFO			pChannelInfo;


#endif // CONFIG_AP_SUPPORT //


//=======STA===========

//=======Common===========
	// OP mode: either AP or STA
	UCHAR                   OpMode;                     // OPMODE_STA, OPMODE_AP
	
	NDIS_MEDIA_STATE        IndicateMediaState;			// Base on Indication state, default is NdisMediaStateDisConnected

#ifdef WSC_INCLUDED
	RTMP_OS_TASK	wscTask;
	UCHAR			WriteWscCfgToDatFile;
	BOOLEAN              WriteWscCfgToAr9DatFile;
	NDIS_SPIN_LOCK WscElmeLock;
	MLME_QUEUE_ELEM *pWscElme;

	/* WSC hardware push button function 0811 */
	BOOLEAN					WscHdrPshBtnFlag; /* 1: support, read from EEPROM */
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN			bWscDriverAutoUpdateCfg;
#endif // CONFIG_AP_SUPPORT //
#endif // WSC_INCLUDED //


	/* MAT related parameters */
#ifdef MAT_SUPPORT
	MAT_STRUCT		MatCfg;
#endif // MAT_SUPPORT //
	

	// configuration: read from Registry & E2PROM
	BOOLEAN                 bLocalAdminMAC;             // Use user changed MAC
	UCHAR                   PermanentAddress[MAC_ADDR_LEN];    // Factory default MAC address
	UCHAR                   CurrentAddress[MAC_ADDR_LEN];      // User changed MAC address

	// ------------------------------------------------------
	// common configuration to both OPMODE_STA and OPMODE_AP
	// ------------------------------------------------------
	COMMON_CONFIG           CommonCfg;
	MLME_STRUCT             Mlme;

	// AP needs those vaiables for site survey feature.
	MLME_AUX                MlmeAux;           // temporary settings used during MLME state machine
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	BSS_TABLE               ScanTab;           // store the latest SCAN result
#endif

	//About MacTab, the sta driver will use #0 and #1 for multicast and AP.
	MAC_TABLE                 MacTab;     // ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table.
	NDIS_SPIN_LOCK          MacTabLock;

#ifdef DOT11_N_SUPPORT
	BA_TABLE			BATable;
	NDIS_SPIN_LOCK          BATabLock;
	RALINK_TIMER_STRUCT RECBATimer;
#endif // DOT11_N_SUPPORT //
	
	// encryption/decryption KEY tables
	CIPHER_KEY              SharedKey[MAX_MBSSID_NUM_CONS][4]; // STA always use SharedKey[BSS0][0..3]

	// RX re-assembly buffer for fragmentation
	FRAGMENT_FRAME          FragFrame;                  // Frame storage for fragment frame

	// various Counters 
	COUNTER_802_3           Counters8023;               // 802.3 counters
	COUNTER_802_11          WlanCounters;               // 802.11 MIB counters
	COUNTER_RALINK          RalinkCounters;             // Ralink propriety counters
	COUNTER_DRS             DrsCounters;                // counters for Dynamic TX Rate Switching
	PRIVATE_STRUC           PrivateInfo;                // Private information & counters

	// flags, see fRTMP_ADAPTER_xxx flags
	ULONG                   Flags;                      // Represent current device status
	ULONG                   PSFlags;                    // Power Save operation flag.
	ULONG                   MoreFlags;                  // Represent specific requirement

	// current TX sequence #
	USHORT                  Sequence;

	// Control disconnect / connect event generation
	//+++Didn't used anymore
	ULONG                   LinkDownTime;
	//---
	ULONG                   LastRxRate;
	ULONG                   LastTxRate;
	//+++Used only for Station
	BOOLEAN                 bConfigChanged;         // Config Change flag for the same SSID setting
	//---
	
	ULONG                   ExtraInfo;              // Extra information for displaying status
	ULONG                   SystemErrorBitmap;      // b0: E2PROM version error

	//+++Didn't used anymore
	ULONG                   MacIcVersion;           // MAC/BBP serial interface issue solved after ver.D
	//---

#ifdef SYSTEM_LOG_SUPPORT
	// ---------------------------
	// System event log
	// ---------------------------
	RT_802_11_EVENT_TABLE   EventTab;
#endif // SYSTEM_LOG_SUPPORT //


	BOOLEAN		HTCEnable;
	
	/*****************************************************************************************/
	/*      Statistic related parameters                                                     */
	/*****************************************************************************************/

	BOOLEAN						bUpdateBcnCntDone;		

	ULONG						macwd;	
	// ----------------------------
	// DEBUG paramerts
	// ----------------------------
	//ULONG		DebugSetting[4];
	BOOLEAN		bPromiscuous;

	// ----------------------------
	// rt2860c emulation-use Parameters 
	// ----------------------------
	//ULONG		rtsaccu[30];
	//ULONG		ctsaccu[30];
	//ULONG		cfendaccu[30];
	//ULONG		bacontent[16];
	//ULONG		rxint[RX_RING_SIZE+1];
	//UCHAR		rcvba[60];
	BOOLEAN		bLinkAdapt;
	BOOLEAN		bForcePrintTX;
	BOOLEAN		bForcePrintRX;
	//BOOLEAN		bDisablescanning;		//defined in RT2870 USB
	BOOLEAN		bStaFifoTest;
	BOOLEAN		bProtectionTest;
	BOOLEAN		bHCCATest;
	BOOLEAN		bGenOneHCCA;
	BOOLEAN		bBroadComHT;
	//+++Following add from RT2870 USB.
	ULONG		BulkOutReq;
	ULONG		BulkOutComplete;
	ULONG		BulkOutCompleteOther;
	ULONG		BulkOutCompleteCancel;	// seems not use now?
	ULONG		BulkInReq;
	ULONG		BulkInComplete;
	ULONG		BulkInCompleteFail;
	//---

    struct wificonf			WIFItestbed;

#ifdef RALINK_ATE
	ATE_INFO				ate;
	UCHAR		TssiGain;
#endif // RALINK_ATE //

#ifdef DOT11_N_SUPPORT
	struct reordering_mpdu_pool mpdu_blk_pool;
#endif // DOT11_N_SUPPORT //

	/* statistics count */
#ifdef LINUX
#if WIRELESS_EXT >= 12
    struct iw_statistics    iw_stats;
#endif

	struct net_device_stats	stats;
#endif // LINUX //

#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY		blockQueueTab[NUM_OF_TX_RING];
#endif // BLOCK_NET_IF //

#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
	PMULTICAST_FILTER_TABLE	pMulticastFilterTable;
	UCHAR					IgmpGroupTxRate;
#endif // IGMP_SNOOP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#ifdef MULTIPLE_CARD_SUPPORT
	INT32					MC_RowID;
	STRING					MC_FileName[256];
#endif // MULTIPLE_CARD_SUPPORT //

	ULONG					TbttTickCount; /* beacon timestamp work-around */
#ifdef PCI_MSI_SUPPORT
	BOOLEAN					HaveMsi;
#endif // PCI_MSI_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT     PeriodicTimer;

#ifdef AP_QLOAD_SUPPORT
	UINT8					FlgQloadEnable; /* 1: any BSS WMM is enabled */
	ULONG					QloadUpTimeLast; /* last up time */
	UINT8					QloadChanUtil; /* last QBSS Load, unit: us */
	UINT32					QloadChanUtilTotal; /* current QBSS Load Total */
	UINT8					QloadChanUtilBeaconCnt; /* 1~100, default: 50 */
	UINT8					QloadChanUtilBeaconInt; /* 1~100, default: 50 */
	UINT32					QloadLatestChannelBusyTimePri;
	UINT32					QloadLatestChannelBusyTimeSec;

	/*
		ex: For 100ms beacon interval,
		if the busy time in last TBTT is smaller than 5ms, QloadBusyCount[0] ++;
		if the busy time in last TBTT is between 5 and 10ms, QloadBusyCount[1] ++;
		......
		if the busy time in last TBTT is larger than 95ms, QloadBusyCount[19] ++;

		Command: "iwpriv ra0 qload show".
	*/

/* provide busy time statistics for every TBTT */
#define QLOAD_FUNC_BUSY_TIME_STATS

/* provide busy time alarm mechanism */
/* use the function to avoid to locate in some noise environments */
#define QLOAD_FUNC_BUSY_TIME_ALARM

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
#define QLOAD_BUSY_INTERVALS	20	/* partition TBTT to QLOAD_BUSY_INTERVALS */
	/* for primary channel & secondary channel */
	UINT32					QloadBusyCountPri[QLOAD_BUSY_INTERVALS];
	UINT32					QloadBusyCountSec[QLOAD_BUSY_INTERVALS];
#endif // QLOAD_FUNC_BUSY_TIME_STATS //

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
#define QLOAD_DOES_ALARM_OCCUR(pAd)	(pAd->FlgQloadAlarmIsSuspended == TRUE)
#define QLOAD_ALARM_EVER_OCCUR(pAd) (pAd->QloadAlarmNumber > 0)
	BOOLEAN					FlgQloadAlarmIsSuspended; /* 1: suspend */

	UINT8					QloadAlarmBusyTimeThreshold; /* unit: 1/100 */
	UINT8					QloadAlarmBusyNumThreshold; /* unit: 1 */
	UINT8					QloadAlarmBusyNum;
	UINT8					QloadAlarmDuration; /* unit: TBTT */

	UINT32					QloadAlarmNumber; /* total alarm times */
	BOOLEAN					FlgQloadAlarm; /* 1: alarm occurs */

	/* speed up use */
	UINT32					QloadTimePeriodLast;
	UINT32					QloadBusyTimeThreshold;
#else

#define QLOAD_DOES_ALARM_OCCUR(pAd)	0
#endif // QLOAD_FUNC_BUSY_TIME_ALARM //

#endif // AP_QLOAD_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

	/* for detect_wmm_traffic() BE TXOP use */
	ULONG					OneSecondnonBEpackets;		// record non BE packets per second 
	UCHAR					is_on;

	/* for detect_wmm_traffic() BE/BK TXOP use */
#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND		(1000000/TIME_BASE)
	UCHAR					flg_be_adjust;
	ULONG					be_adjust_last_time;


#ifdef WSC_INCLUDED
    // for multiple card
    UCHAR	                *pHmacData;
 //   UCHAR                   Wsc_Uuid_E[UUID_LEN_HEX];  // now these two parameters move to structure "WSC_CTRL"
 //   UCHAR                   Wsc_Uuid_Str[UUID_LEN_STR];
#endif // WSC_INCLUDED //

#ifdef IKANOS_VX_1X0
	struct IKANOS_TX_INFO	IkanosTxInfo;
	struct IKANOS_TX_INFO	IkanosRxInfo[MAX_MBSSID_NUM + MAX_WDS_ENTRY + MAX_APCLI_NUM + MAX_MESH_NUM];
#endif // IKANOS_VX_1X0 //


#ifdef DBG_DIAGNOSE
	RtmpDiagStruct	DiagStruct;
#endif // DBG_DIAGNOSE //

#ifdef WMM_ACM_SUPPORT
	UINT8					*pACM_Ctrl_BK;
	UINT16					AcmAvalCap; /* unit: 32us */

	NDIS_SPIN_LOCK			AcmTspecSemLock;
	NDIS_SPIN_LOCK			AcmTspecIrqLock;
#endif // WMM_ACM_SUPPORT //

	UINT8					FlgCtsEnabled;
	UINT8					PM_FlgSuspend;

#ifdef RT30xx
#ifdef RTMP_EFUSE_SUPPORT
	BOOLEAN		bUseEfuse;
	BOOLEAN		bEEPROMFile;
	BOOLEAN		bFroceEEPROMBuffer;
	UCHAR		EEPROMImage[1024];
#endif // RTMP_EFUSE_SUPPORT //
#endif // RT30xx //



	EXT_CAP_INFO_ELEMENT ExtCapInfo;


#ifdef VENDOR_FEATURE1_SUPPORT
	UCHAR	FifoUpdateDone, FifoUpdateRx;
#endif // VENDOR_FEATURE1_SUPPORT //

#ifdef VENDOR_FEATURE2_SUPPORT

#define MEM_DBG_PKT_ALLOC_INC(_pAd)	_pAd->NumOfPktAlloc ++
#define MEM_DBG_PKT_FREE_INC(_pAd)	_pAd->NumOfPktFree ++
	UINT32	NumOfPktAlloc, NumOfPktFree; // used in memory debug
#else

#define MEM_DBG_PKT_ALLOC_INC(_pAd)
#define MEM_DBG_PKT_FREE_INC(_pAd)
#endif // VENDOR_FEATURE2_SUPPORT //

#define RFIC_24GHZ		0x01
#define RFIC_5GHZ		0x02
	UINT8	RFICType;

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	VOID *pCfgDev;
	VOID *pCfg80211_CB;

	BOOLEAN FlgCfg80211Scanning;
	BOOLEAN FlgCfg80211Connecting;
	UCHAR Cfg80211_Alpha2[2];

	/* function pointers used in driver.ko */
	VOID (*CFG80211_Register)(
		IN VOID 				*pAd,
		IN struct device		*pDev,
		IN struct net_device	*pNetDev);

	CFG80211_FUNC_OPS;

#endif // RT_CFG80211_SUPPORT //
#endif // LINUX //

#ifdef OS_ABL_SUPPORT
#ifdef MAT_SUPPORT
	/* used in OS_ABL */
	BOOLEAN (*MATPktRxNeedConvert)(
		IN PRTMP_ADAPTER	pAd, 
		IN PNET_DEV			net_dev);
	PUCHAR (*MATEngineRxHandle)(
		IN PRTMP_ADAPTER	pAd,
		IN PNDIS_PACKET		pPkt,
		IN UINT				infIdx);
#endif // MAT_SUPPORT //
#endif // OS_ABL_SUPPORT //
	struct {
	INT		IeLen;
	UCHAR	*pIe;	
	} ProbeRespIE[MAX_LEN_OF_BSS_TABLE];

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	// The RT53xx driver should backward compatible, but the beacon buffer base address are different.
	BCN_PARAM_STRUCT		BcnCB;
#endif // SPECIFIC_BCN_BUF_SUPPORT //
//#ifdef CONFIG_STA_SUPPORT
//#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
	UCHAR PreRFXCodeValue;	// latch for RF (Freq Cal) value
//#endif // RTMP_FREQ_CALIBRATION_SUPPORT //					
//#endif // CONFIG_STA_SUPPORT //	
/* purpose: Cancel all timers when module is removed */
	LIST_HEADER				RscTimerCreateList; /* timers list */
};

#ifdef RTMP_INTERNAL_TX_ALC
// The Tx power tuning entry
typedef struct  _TX_POWER_TUNING_ENTRY_STRUCT
{
	CHAR	RF_TX0_ALC; // 3390: RF R12[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC
	CHAR	MAC_PowerDelta; // Tx power control over MAC 0x1314~0x1324
}   TX_POWER_TUNING_ENTRY_STRUCT, *PTX_POWER_TUNING_ENTRY_STRUCT;

// The offset of the Tx power tuning entry (zero-based array)
#define TX_POWER_TUNING_ENTRY_OFFSET		30

// The lower-bound of the Tx power tuning entry
#define LOWERBOUND_TX_POWER_TUNING_ENTRY	-30

// The upper-bound of the Tx power tuning entry
#define UPPERBOUND_TX_POWER_TUNING_ENTRY	45 // zero-based array

#ifdef RT53xx
//
// The offset of the Tx power tuning entry (zero-based array) for 5390
//
#define TX_POWER_TUNING_ENTRY_OFFSET_OVER_5390		30

//
// The lower-bound of the Tx power tuning entry for 5390
//
#define LOWERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390	-30

//
// The upper-bound of the Tx power tuning entry for 5390
//
#define UPPERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390	69 // zero-based array

//
// Temperature compensation lookup table
//
#define TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET 7

//
// The lower/upper power delta index for the TSSI rate table
//
#define LOWER_POWER_DELTA_INDEX	0
#define UPPER_POWER_DELTA_INDEX		24

//
// The offset of the TSSI rate table
//
#define TSSI_RATIO_TABLE_OFFSET	12

//
// Get the power delta bound
//
#define GET_TSSI_RATE_TABLE_INDEX(x) (((x) > UPPER_POWER_DELTA_INDEX) ? (UPPER_POWER_DELTA_INDEX) : (((x) < LOWER_POWER_DELTA_INDEX) ? (LOWER_POWER_DELTA_INDEX) : ((x))))

//
// 802.11b CCK TSSI information
//
typedef union _CCK_TSSI_INFO
{
#ifdef RT_BIG_ENDIAN
	struct
	{
		UCHAR	Reserved:1;
		UCHAR	ShortPreamble:1;
		UCHAR	Rate:2;
		UCHAR	Tx40MSel:2;
		UCHAR	TxType:2;
	} field;
#else
	struct
	{
		UCHAR	TxType:2;
		UCHAR	Tx40MSel:2;
		UCHAR	Rate:2;
		UCHAR	ShortPreamble:1;
		UCHAR	Reserved:1;
	} field;
#endif /* RT_BIG_ENDIAN */

	UCHAR	value;
} CCK_TSSI_INFO, *PCCK_TSSI_INFO;

//
// 802.11a/g OFDM TSSI information
//
typedef union _OFDM_TSSI_INFO
{
#ifdef RT_BIG_ENDIAN
	struct
	{
		UCHAR	Rate:4;
		UCHAR	Tx40MSel:2;
		UCHAR	TxType:2;
	} field;
#else
	struct
	{
		UCHAR	TxType:2;
		UCHAR	Tx40MSel:2;
		UCHAR	Rate:4;
	} field;
#endif /* RT_BIG_ENDIAN */

	UCHAR	value;
} OFDM_TSSI_INFO, *POFDM_TSSI_INFO;

//
// 802.11n HT TSSI information
//
typedef union _HT_TSSI_INFO
{
	union
	{
#ifdef RT_BIG_ENDIAN
		struct
		{
			UCHAR	SGI:1;
			UCHAR	STBC:2;
			UCHAR	Aggregation:1;
			UCHAR	Tx40MSel:2;
			UCHAR	TxType:2;
		} field;
#else	
		struct
		{
			UCHAR	TxType:2;
			UCHAR	Tx40MSel:2;
			UCHAR	Aggregation:1;
			UCHAR	STBC:2;
			UCHAR	SGI:1;
		} field;
#endif /* RT_BIG_ENDIAN */

		UCHAR	value;
	} PartA;

	union
	{
#ifdef RT_BIG_ENDIAN
		struct
		{
			UCHAR	BW:1;
			UCHAR	MCS:7;
		} field;
#else	
		struct
		{
			UCHAR	MCS:7;
			UCHAR	BW:1;
		} field;
#endif /* RT_BIG_ENDIAN */

		UCHAR	value;
	} PartB;
} HT_TSSI_INFO, *PHT_TSSI_INFO;

#endif // RT53xx //
#endif // RTMP_INTERNAL_TX_ALC //

#ifdef TONE_RADAR_DETECT_SUPPORT
#define DELAYINTMASK		0x0013fffb
#define INTMASK				0x0013fffb
#define IndMask				0x0013fffc
#define RadarInt			0x00100000
#else
#define DELAYINTMASK		0x0003fffb
#define INTMASK				0x0003fffb
#define IndMask				0x0003fffc
#endif // TONE_RADAR_DETECT_SUPPORT //

#define RxINT				0x00000005	// Delayed Rx or indivi rx
#define TxDataInt			0x000000fa	// Delayed Tx or indivi tx
#define TxMgmtInt			0x00000102	// Delayed Tx or indivi tx 
#define TxCoherent			0x00020000	// tx coherent 
#define RxCoherent			0x00010000	// rx coherent
#define McuCommand			0x00000200	// mcu
#define PreTBTTInt			0x00001000	// Pre-TBTT interrupt
#define TBTTInt				0x00000800		// TBTT interrupt
#define GPTimeOutInt			0x00008000		// GPtimeout interrupt
#define AutoWakeupInt		0x00004000		// AutoWakeupInt interrupt
#define FifoStaFullInt			0x00002000	//  fifo statistics full interrupt


/***************************************************************************
  *	Rx Path software control block related data structures
  **************************************************************************/
typedef struct _RX_BLK_
{
//	RXD_STRUC		RxD; // sample
	RT28XX_RXD_STRUC	RxD;
	PRXWI_STRUC			pRxWI;
	PHEADER_802_11		pHeader;
	PNDIS_PACKET		pRxPacket;
	UCHAR				*pData;
	USHORT				DataSize;
	USHORT				Flags;
	UCHAR				UserPriority;	// for calculate TKIP MIC using
} RX_BLK;


#define RX_BLK_SET_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags |= _flag)
#define RX_BLK_TEST_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags & _flag)
#define RX_BLK_CLEAR_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags &= ~(_flag))


#define fRX_WDS			0x0001
#define fRX_AMSDU		0x0002
#define fRX_ARALINK		0x0004
#define fRX_HTC			0x0008
#define fRX_PAD			0x0010
#define fRX_AMPDU		0x0020
#define fRX_QOS			0x0040
#define fRX_INFRA		0x0080
#define fRX_EAP			0x0100
#define fRX_MESH		0x0200
#define fRX_APCLI		0x0400
#define fRX_DLS			0x0800
#define fRX_WPI			0x1000

#define LENGTH_AMSDU_SUBFRAMEHEAD	14
#define LENGTH_ARALINK_SUBFRAMEHEAD	14
#define LENGTH_ARALINK_HEADER_FIELD	 2


/***************************************************************************
  *	Tx Path software control block related data structures
  **************************************************************************/
#define TX_UNKOWN_FRAME		0x00
#define TX_MCAST_FRAME			0x01
#define TX_LEGACY_FRAME		0x02
#define TX_AMPDU_FRAME		0x04
#define TX_AMSDU_FRAME		0x08
#define TX_RALINK_FRAME		0x10
#define TX_FRAG_FRAME			0x20


//	Currently the sizeof(TX_BLK) is 148 bytes.
typedef struct _TX_BLK_
{
	UCHAR				QueIdx;
	UCHAR				TxFrameType;				// Indicate the Transmission type of the all frames in one batch
	UCHAR				TotalFrameNum;				// Total frame number want to send-out in one batch
	USHORT				TotalFragNum;				// Total frame fragments required in one batch
	USHORT				TotalFrameLen;				// Total length of all frames want to send-out in one batch

	QUEUE_HEADER		TxPacketList;
	MAC_TABLE_ENTRY		*pMacEntry;					// NULL: packet with 802.11 RA field is multicast/broadcast address
	HTTRANSMIT_SETTING	*pTransmit;
	
	// Following structure used for the characteristics of a specific packet.
	PNDIS_PACKET		pPacket;
	PUCHAR				pSrcBufHeader;				// Reference to the head of sk_buff->data
	PUCHAR				pSrcBufData;				// Reference to the sk_buff->data, will changed depends on hanlding progresss
	UINT				SrcBufLen;					// Length of packet payload which not including Layer 2 header

	PUCHAR				pExtraLlcSnapEncap;			// NULL means no extra LLC/SNAP is required
#ifndef VENDOR_FEATURE1_SUPPORT
	/*
		Note: Can not insert any other new parameters
		between pExtraLlcSnapEncap & HeaderBuf; Or
		the start address of HeaderBuf will not be aligned by 4.

		But we can not change HeaderBuf[128] to HeaderBuf[32] because
		many codes use HeaderBuf[index].
	*/
	UCHAR				HeaderBuf[128];				// TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP
#else
	UINT32				HeaderBuffer[32];			// total 128B, use UINT32 to avoid alignment problem
	UCHAR				*HeaderBuf;
#endif // VENDOR_FEATURE1_SUPPORT //
	UCHAR				MpduHeaderLen;				// 802.11 header length NOT including the padding
	UCHAR				HdrPadLen;					// recording Header Padding Length;
	UCHAR				apidx;						// The interface associated to this packet 
	UCHAR				Wcid;						// The MAC entry associated to this packet
	UCHAR				UserPriority;				// priority class of packet
	UCHAR				FrameGap;					// what kind of IFS this packet use
	UCHAR				MpduReqNum;					// number of fragments of this frame
	UCHAR				TxRate;						// TODO: Obsoleted? Should change to MCS?
	UCHAR				CipherAlg;					// cipher alogrithm
	PCIPHER_KEY			pKey;
	UCHAR				KeyIdx;						// Indicate the transmit key index

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT				ApCliIfidx;
	PAPCLI_STRUCT		pApCliEntry;
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

	UINT32				Flags;						//See following definitions for detail.

	//YOU SHOULD NOT TOUCH IT! Following parameters are used for hardware-depended layer.
	ULONG				Priv;						// Hardware specific value saved in here.

#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif // CONFIG_AP_SUPPORT //

} TX_BLK, *PTX_BLK;


#define fTX_bRtsRequired			0x0001	// Indicate if need send RTS frame for protection. Not used in RT2860/RT2870.
#define fTX_bAckRequired			0x0002	// the packet need ack response
#define fTX_bPiggyBack			0x0004	// Legacy device use Piggback or not
#define fTX_bHTRate				0x0008	// allow to use HT rate
#define fTX_bForceNonQoS		0x0010	// force to transmit frame without WMM-QoS in HT mode 
#define fTX_bAllowFrag			0x0020	// allow to fragment the packet, A-MPDU, A-MSDU, A-Ralink is not allowed to fragment
#define fTX_bMoreData			0x0040	// there are more data packets in PowerSave Queue
#define fTX_bWMM				0x0080	// QOS Data
#define fTX_bClearEAPFrame		0x0100

#define	fTX_bSwEncrypt			0x0400	// this packet need to be encrypted by software before TX
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#define fTX_bApCliPacket			0x0200
#endif // APCLI_SUPPORT //

#ifdef UAPSD_AP_SUPPORT
#define	fTX_bWMM_UAPSD_EOSP	0x0800	// Used when UAPSD_SUPPORT
#endif // UAPSD_AP_SUPPORT //

#ifdef WDS_SUPPORT
#define fTX_bWDSEntry			0x1000	// Used when WDS_SUPPORT
#endif // WDS_SUPPORT //
#endif // CONFIG_AP_SUPPORT //



#ifdef WAPI_SUPPORT
#define	fTX_bWPIDataFrame		0x8000	// indicate this packet is an WPI data frame, it need to be encrypted by software 
#endif // WAPI_SUPPORT //

#ifdef CLIENT_WDS
#define fTX_bClientWDSFrame		0x10000
#endif // CLIENT_WDS //


#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))
	



#ifdef RT_BIG_ENDIAN
/***************************************************************************
  *	Endian conversion related functions
  **************************************************************************/
/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd 	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
static inline VOID	RTMPWIEndianChange(
	IN	PUCHAR			pData,
	IN	ULONG			DescriptorType)
{
	int size;
	int i;
	
	size = ((DescriptorType == TYPE_TXWI) ? TXWI_SIZE : RXWI_SIZE);
	
	if(DescriptorType == TYPE_TXWI)
	{
		*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		// Byte 0~3
		*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData+4)));	// Byte 4~7
	} 
	else
	{
		for(i=0; i < size/4 ; i++)
			*(((UINT32 *)pData) +i) = SWAP32(*(((UINT32 *)pData)+i));
	}
}


#ifdef RTMP_MAC_PCI
static inline VOID	WriteBackToDescriptor(
	IN  PUCHAR			Dest,
 	IN	PUCHAR			Src,
    IN  BOOLEAN			DoEncrypt,
	IN  ULONG           DescriptorType)
{
	UINT32 *p1, *p2;

	p1 = ((UINT32 *)Dest);
	p2 = ((UINT32 *)Src);
	
	*p1 = *p2;
	*(p1+2) = *(p2+2);
	*(p1+3) = *(p2+3);
	*(p1+1) = *(p2+1); // Word 1; this must be written back last
}
#endif // RTMP_MAC_PCI //


/*
	========================================================================

	Routine Description:
		Endian conversion of Tx/Rx descriptor .

	Arguments:
		pAd 	Pointer to our adapter
		pData			Pointer to Tx/Rx descriptor
		DescriptorType	Direction of the frame

	Return Value:
		None

	Note:
		Call this function when read or update descriptor
	========================================================================
*/
#ifdef RTMP_MAC_PCI
static inline VOID	RTMPDescriptorEndianChange(
	IN	PUCHAR			pData,
	IN	ULONG			DescriptorType)
{
	*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		// Byte 0~3
	*((UINT32 *)(pData + 8)) = SWAP32(*((UINT32 *)(pData+8)));	// Byte 8~11
	*((UINT32 *)(pData +12)) = SWAP32(*((UINT32 *)(pData + 12)));	// Byte 12~15
	*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData + 4)));				// Byte 4~7, this must be swapped last
}
#endif // RTMP_MAC_PCI //

/*
	========================================================================

	Routine Description:
		Endian conversion of all kinds of 802.11 frames .

	Arguments:
		pAd 	Pointer to our adapter
		pData			Pointer to the 802.11 frame structure
		Dir 			Direction of the frame
		FromRxDoneInt	Caller is from RxDone interrupt

	Return Value:
		None

	Note:
		Call this function when read or update buffer data
	========================================================================
*/
static inline VOID	RTMPFrameEndianChange(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			pData, 
	IN	ULONG			Dir,
	IN	BOOLEAN 		FromRxDoneInt)
{
	PHEADER_802_11 pFrame;
	PUCHAR	pMacHdr;

	// swab 16 bit fields - Frame Control field
	if(Dir == DIR_READ)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}

	pFrame = (PHEADER_802_11) pData;
	pMacHdr = (PUCHAR) pFrame;

	// swab 16 bit fields - Duration/ID field
	*(USHORT *)(pMacHdr + 2) = SWAP16(*(USHORT *)(pMacHdr + 2));

	if (pFrame->FC.Type != BTYPE_CNTL)
	{
		// swab 16 bit fields - Sequence Control field
		*(USHORT *)(pMacHdr + 22) = SWAP16(*(USHORT *)(pMacHdr + 22));
	}

	if(pFrame->FC.Type == BTYPE_MGMT)
	{
		switch(pFrame->FC.SubType)
		{
			case SUBTYPE_ASSOC_REQ:
			case SUBTYPE_REASSOC_REQ:
				// swab 16 bit fields - CapabilityInfo field
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				// swab 16 bit fields - Listen Interval field
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_ASSOC_RSP:
			case SUBTYPE_REASSOC_RSP:
				// swab 16 bit fields - CapabilityInfo field
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				// swab 16 bit fields - Status Code field
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				// swab 16 bit fields - AID field
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_AUTH:
				 /* When the WEP bit is on, don't do the conversion here.
					This is only shared WEP can hit this condition. 
					For AP, it shall do conversion after decryption. 
					For STA, it shall do conversion before encryption. */
				if (pFrame->FC.Wep == 1)
					break;
				else
				{
					// swab 16 bit fields - Auth Alg No. field
					pMacHdr += sizeof(HEADER_802_11);
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					// swab 16 bit fields - Auth Seq No. field
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					// swab 16 bit fields - Status Code field
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				}
				break;

			case SUBTYPE_BEACON:
			case SUBTYPE_PROBE_RSP:
				// swab 16 bit fields - BeaconInterval field
				pMacHdr += (sizeof(HEADER_802_11) + TIMESTAMP_LEN);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				// swab 16 bit fields - CapabilityInfo field
				pMacHdr += sizeof(USHORT);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_DEAUTH:
			case SUBTYPE_DISASSOC:
				/* If the PMF is negotiated, those frames shall be encrypted */
				if(!FromRxDoneInt && pFrame->FC.Wep == 1)
					break;
				else
				{
					// swab 16 bit fields - Reason code field
					pMacHdr += sizeof(HEADER_802_11);
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				}
				break;
		}
	}
	else if( pFrame->FC.Type == BTYPE_DATA )
	{
	}
	else if(pFrame->FC.Type == BTYPE_CNTL)
	{
		switch(pFrame->FC.SubType)
		{
			case SUBTYPE_BLOCK_ACK_REQ:
				{
					PFRAME_BA_REQ pBAReq = (PFRAME_BA_REQ)pFrame;
					*(USHORT *)(&pBAReq->BARControl) = SWAP16(*(USHORT *)(&pBAReq->BARControl));
					pBAReq->BAStartingSeq.word = SWAP16(pBAReq->BAStartingSeq.word);
				}
				break;
			case SUBTYPE_BLOCK_ACK:
				// For Block Ack packet, the HT_CONTROL field is in the same offset with Addr3 
				*(UINT32 *)(&pFrame->Addr3[0]) = SWAP32(*(UINT32 *)(&pFrame->Addr3[0]));
				break;

			case SUBTYPE_ACK:
				//For ACK packet, the HT_CONTROL field is in the same offset with Addr2
				*(UINT32 *)(&pFrame->Addr2[0])=	SWAP32(*(UINT32 *)(&pFrame->Addr2[0]));
				break;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("Invalid Frame Type!!!\n"));
	}

	// swab 16 bit fields - Frame Control
	if(Dir == DIR_WRITE)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}
}
#endif // RT_BIG_ENDIAN //


/***************************************************************************
  *	Other static inline function definitions
  **************************************************************************/
static inline VOID ConvertMulticastIP2MAC(
	IN PUCHAR pIpAddr,
	IN PUCHAR *ppMacAddr, 
	IN UINT16 ProtoType)
{
	if (pIpAddr == NULL)
		return;

	if (ppMacAddr == NULL || *ppMacAddr == NULL)
		return;

	switch (ProtoType)
	{
		case ETH_P_IPV6:
//			memset(*ppMacAddr, 0, ETH_LENGTH_OF_ADDRESS);
			*(*ppMacAddr) = 0x33;
			*(*ppMacAddr + 1) = 0x33;
			*(*ppMacAddr + 2) = pIpAddr[12];
			*(*ppMacAddr + 3) = pIpAddr[13];
			*(*ppMacAddr + 4) = pIpAddr[14];
			*(*ppMacAddr + 5) = pIpAddr[15];
			break;

		case ETH_P_IP:
		default:
//			memset(*ppMacAddr, 0, ETH_LENGTH_OF_ADDRESS);
			*(*ppMacAddr) = 0x01;
			*(*ppMacAddr + 1) = 0x00;
			*(*ppMacAddr + 2) = 0x5e;
			*(*ppMacAddr + 3) = pIpAddr[1] & 0x7f;
			*(*ppMacAddr + 4) = pIpAddr[2];
			*(*ppMacAddr + 5) = pIpAddr[3];
			break;
	}

	return;
}


char *GetPhyMode(int Mode);
char* GetBW(int BW);



BOOLEAN RTMPCheckForHang(
	IN  NDIS_HANDLE MiniportAdapterContext);

//
//  Private routines in rtmp_init.c
//
NDIS_STATUS RTMPAllocAdapterBlock(
	IN PVOID			handle,
	OUT PRTMP_ADAPTER   *ppAdapter);

NDIS_STATUS RTMPAllocTxRxRingMemory(
	IN  PRTMP_ADAPTER   pAd);

#ifdef RESOURCE_PRE_ALLOC
NDIS_STATUS RTMPInitTxRxRingMemory(
	IN RTMP_ADAPTER *pAd);
#endif // RESOURCE_PRE_ALLOC //

NDIS_STATUS	RTMPReadParametersHook(
	IN	PRTMP_ADAPTER pAd);

NDIS_STATUS	RTMPSetProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING		pBuffer);

INT RTMPGetKeyParameter(
    IN PSTRING key,
    OUT PSTRING dest,
    IN INT destsize,
    IN PSTRING buffer,
    IN BOOLEAN bTrimSpace);

#ifdef WSC_INCLUDED
VOID rtmp_read_wsc_user_parms_from_file(
	IN	PRTMP_ADAPTER pAd, 
	IN 	char *tmpbuf, 
	IN 	char *buffer);
#endif//WSC_INCLUDED//

VOID RTMPFreeAdapter(
	IN  PRTMP_ADAPTER   pAd);

#ifdef RTMP_RF_RW_SUPPORT
VOID NICInitRFRegisters(
	IN PRTMP_ADAPTER pAd);

VOID RtmpChipOpsRFHook(
	IN RTMP_ADAPTER *pAd);
	
VOID RtmpChipOpsHook(
	IN RTMP_ADAPTER *pAd);

NDIS_STATUS	RT30xxWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT30xxReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);
#endif // RTMP_RF_RW_SUPPORT //

VOID NICReadEEPROMParameters(
	IN  PRTMP_ADAPTER       pAd,
	IN	PSTRING				mac_addr);

VOID NICInitAsicFromEEPROM(
	IN  PRTMP_ADAPTER       pAd);


NDIS_STATUS NICInitializeAdapter(
	IN  PRTMP_ADAPTER   pAd,
	IN   BOOLEAN    bHardReset);

NDIS_STATUS NICInitializeAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN		bHardReset);


VOID RTMPRingCleanUp(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           RingType);

VOID UserCfgExit(	
	IN  RTMP_ADAPTER *pAd);

VOID UserCfgInit(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS NICLoadFirmware(
	IN  PRTMP_ADAPTER   pAd);

VOID NICEraseFirmware(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS NICLoadRateSwitchingParams(
	IN PRTMP_ADAPTER pAd);

VOID NICUpdateFifoStaCounters(
	IN PRTMP_ADAPTER pAd);

VOID NICUpdateRawCounters(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPZeroMemory(
	IN  PVOID   pSrc,
	IN  ULONG   Length);

ULONG RTMPCompareMemory(
	IN  PVOID   pSrc1,
	IN  PVOID   pSrc2,
	IN  ULONG   Length);

VOID RTMPMoveMemory(
	OUT PVOID   pDest,
	IN  PVOID   pSrc,
	IN  ULONG   Length);

VOID AtoH(
	PSTRING	src,
	PUCHAR dest,
	int		destlen);

UCHAR BtoH(
	char ch);

VOID RTMPInitTimer(
	IN  PRTMP_ADAPTER           pAd,
	IN  PRALINK_TIMER_STRUCT    pTimer,
	IN  PVOID                   pTimerFunc,
	IN	PVOID					pData,
	IN  BOOLEAN                 Repeat);

VOID RTMPSetTimer(
	IN  PRALINK_TIMER_STRUCT    pTimer,
	IN  ULONG                   Value);


VOID RTMPModTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value);

VOID RTMPCancelTimer(
	IN  PRALINK_TIMER_STRUCT    pTimer,
	OUT BOOLEAN                 *pCancelled);

VOID RTMPReleaseTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	OUT	BOOLEAN					*pCancelled);

VOID RTMP_TimerListRelease(
	IN	PRTMP_ADAPTER			pAd);

VOID RTMP_OS_Release_Timer(
	IN	NDIS_MINIPORT_TIMER	*pTimerOrg);

#ifdef LED_CONTROL_SUPPORT
VOID RTMPSetLED(
	IN PRTMP_ADAPTER 	pAd, 
	IN UCHAR			Status);

VOID RTMPSetSignalLED(
	IN PRTMP_ADAPTER 	pAd, 
	IN NDIS_802_11_RSSI Dbm);
#endif // LED_CONTROL_SUPPORT //

#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
VOID LEDConnectionStart(
	IN PRTMP_ADAPTER pAd);

VOID LEDConnectionCompletion(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN bSuccess);
#endif // WSC_LED_SUPPORT //
#endif // WSC_STA_SUPPORT //

VOID RTMPEnableRxTx(
	IN PRTMP_ADAPTER	pAd);

VOID AntCfgInit(
IN  PRTMP_ADAPTER   pAd);
//
// prototype in action.c
//
VOID ActionStateMachineInit(
    IN	PRTMP_ADAPTER	pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeADDBAAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDELBAAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDLSAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeInvalidAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeQOSAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

#ifdef DOT11_N_SUPPORT
VOID PeerAddBAReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAddBARspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerDelBAAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerBAAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);
#endif // DOT11_N_SUPPORT //

VOID SendPSMPAction(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Wcid,
	IN UCHAR			Psmp);
				   
#ifdef CONFIG_AP_SUPPORT				   
VOID SendBeaconRequest(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				Wcid);
#endif // CONFIG_AP_SUPPORT //

VOID PeerRMAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPublicAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);


#ifdef CONFIG_AP_SUPPORT
VOID ApPublicAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
#endif // CONFIG_AP_SUPPORT //

VOID PeerBSSTranAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

#ifdef DOT11_N_SUPPORT
VOID PeerHTAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
#endif // DOT11_N_SUPPORT //

VOID PeerQOSAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

#ifdef QOS_DLS_SUPPORT
VOID PeerDLSAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);
#endif // QOS_DLS_SUPPORT //


#ifdef DOT11_N_SUPPORT
VOID RECBATimerTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID ORIBATimerTimeout(
	IN	PRTMP_ADAPTER	pAd);

VOID SendRefreshBAR(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);

#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(
	IN PRTMP_ADAPTER pAd);

VOID SendBSS2040CoexistMgmtAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	Wcid,
	IN	UCHAR	apidx,
	IN	UCHAR	InfoReq);

VOID SendNotifyBWActionFrame(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR  Wcid,
	IN UCHAR apidx);
	
BOOLEAN ChannelSwitchSanityCheck(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN    UCHAR  NewChannel,
	IN    UCHAR  Secondary);

VOID ChannelSwitchAction(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN    UCHAR  Channel,
	IN    UCHAR  Secondary);

ULONG BuildIntolerantChannelRep(
	IN	PRTMP_ADAPTER	pAd,
	IN    PUCHAR  pDest); 

VOID Update2040CoexistFrameAndNotify(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);
	
VOID Send2040CoexistAction(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID UpdateBssScanParm(
	IN PRTMP_ADAPTER pAd,
	IN OVERLAP_BSS_SCAN_IE APBssScan);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //
	
VOID ActHeaderInit(
    IN	PRTMP_ADAPTER	pAd, 
    IN OUT PHEADER_802_11 pHdr80211, 
    IN PUCHAR Addr1, 
    IN PUCHAR Addr2,
    IN PUCHAR Addr3);

VOID BarHeaderInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN OUT PFRAME_BAR pCntlBar, 
	IN PUCHAR pDA,
	IN PUCHAR pSA);

VOID InsertActField(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode);

BOOLEAN QosBADataParse(
	IN PRTMP_ADAPTER	pAd, 
	IN BOOLEAN bAMSDU,
	IN PUCHAR p8023Header,
	IN UCHAR	WCID,
	IN UCHAR	TID,
	IN USHORT Sequence,
	IN UCHAR DataOffset, 
	IN USHORT Datasize,
	IN UINT   CurRxIndex);

#ifdef DOT11_N_SUPPORT
BOOLEAN CntlEnqueueForRecv(
    IN	PRTMP_ADAPTER	pAd, 
	IN ULONG Wcid, 
    IN ULONG MsgLen, 
	IN PFRAME_BA_REQ pMsg);

VOID BaAutoManSwitch(
	IN	PRTMP_ADAPTER	pAd);
#endif // DOT11_N_SUPPORT //

VOID HTIOTCheck(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR     BatRecIdx);

//
// Private routines in rtmp_data.c
//
BOOLEAN RTMPHandleTxRingDmaDoneInterrupt(
	IN  PRTMP_ADAPTER   pAd,
	IN  INT_SOURCE_CSR_STRUC TxRingBitmap);

VOID RTMPHandleMgmtRingDmaDoneInterrupt(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPHandleTBTTInterrupt(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPHandlePreTBTTInterrupt(
	IN  PRTMP_ADAPTER   pAd);

void RTMPHandleTwakeupInterrupt(
	IN PRTMP_ADAPTER pAd);

VOID	RTMPHandleRxCoherentInterrupt(
	IN	PRTMP_ADAPTER	pAd);

#ifdef CONFIG_AP_SUPPORT
VOID RTMPHandleMcuInterrupt(
	IN PRTMP_ADAPTER pAd);
#endif // CONFIG_AP_SUPPORT //

NDIS_STATUS Sniff2BytesFromNdisBuffer(
	IN  PNDIS_BUFFER    pFirstBuffer,
	IN  UCHAR           DesiredOffset,
	OUT PUCHAR          pByte0,
	OUT PUCHAR          pByte1);

NDIS_STATUS STASendPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    pPacket);

VOID STASendPackets(
	IN  NDIS_HANDLE     MiniportAdapterContext,
	IN  PPNDIS_PACKET   ppPacketArray,
	IN  UINT            NumberOfPackets);

VOID RTMPDeQueuePacket(
	IN  PRTMP_ADAPTER   pAd,
   	IN	BOOLEAN			bIntContext,
	IN  UCHAR			QueIdx,
	IN	UCHAR			Max_Tx_Packets);

NDIS_STATUS	RTMPHardTransmit(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET		pPacket,
	IN  UCHAR			QueIdx,
	OUT	PULONG			pFreeTXDLeft);

NDIS_STATUS	STAHardTransmit(
	IN PRTMP_ADAPTER	pAd,
	IN TX_BLK			*pTxBlk,
	IN  UCHAR			QueIdx);

VOID STARxEAPOLFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

NDIS_STATUS RTMPFreeTXDRequest(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           RingType,
	IN  UCHAR           NumberRequired,
	IN 	PUCHAR          FreeNumberIs);

NDIS_STATUS MlmeHardTransmit(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

#ifdef RTMP_MAC_PCI
NDIS_STATUS MlmeHardTransmitTxRing(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

NDIS_STATUS MlmeDataHardTransmit(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	QueIdx,
	IN	PNDIS_PACKET	pPacket);

VOID RTMPWriteTxDescriptor(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXD_STRUC		pTxD,
	IN	BOOLEAN			bWIV,
	IN	UCHAR			QSEL);
#endif // RTMP_MAC_PCI //

USHORT  RTMPCalcDuration(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Rate,
	IN  ULONG           Size);

VOID RTMPWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC		pTxWI,	
	IN  BOOLEAN    		FRAG,	
	IN  BOOLEAN    		CFACK,
	IN  BOOLEAN    		InsTimestamp,
	IN	BOOLEAN			AMPDU,
	IN	BOOLEAN			Ack,
	IN	BOOLEAN			NSeq,		// HW new a sequence.
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN  UCHAR      		PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,	
	IN	BOOLEAN			CfAck,	
	IN	HTTRANSMIT_SETTING	*pTransmit);


VOID RTMPWriteTxWI_Data(
	IN	PRTMP_ADAPTER		pAd,
	IN	OUT PTXWI_STRUC		pTxWI,
	IN	TX_BLK				*pTxBlk);

	
VOID RTMPWriteTxWI_Cache(
	IN	PRTMP_ADAPTER		pAd,
	IN	OUT PTXWI_STRUC		pTxWI,
	IN	TX_BLK				*pTxBlk);

VOID RTMPSuspendMsduTransmission(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPResumeMsduTransmission(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS MiniportMMRequest(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			QueIdx,
	IN	PUCHAR			pData,
	IN  UINT            Length);

VOID RTMPSendNullFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           TxRate,
	IN	BOOLEAN			bQosNull);


NDIS_STATUS RTMPAllocateNdisPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    *pPacket,
	IN  PUCHAR          pHeader,
	IN  UINT            HeaderLen,
	IN  PUCHAR          pData,
	IN  UINT            DataLen);

VOID RTMPFreeNdisPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    pPacket);

BOOLEAN RTMPFreeTXDUponTxDmaDone(
	IN PRTMP_ADAPTER    pAd, 
	IN UCHAR            QueIdx);

BOOLEAN RTMPCheckEtherType(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket,
	IN	PMAC_TABLE_ENTRY pMacEntry,
	OUT PUCHAR pUserPriority,
	OUT PUCHAR pQueIdx);


VOID RTMPCckBbpTuning(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UINT			TxRate);
//
// MLME routines
//

// Asic/RF/BBP related functions

VOID AsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd);

VOID 	AsicUpdateProtect(
	IN		PRTMP_ADAPTER	pAd,
	IN 		USHORT			OperaionMode,
	IN 		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist);

VOID AsicBBPAdjust(
	IN RTMP_ADAPTER *pAd);

VOID AsicSwitchChannel(
	IN  PRTMP_ADAPTER   pAd, 
	IN	UCHAR			Channel,
	IN	BOOLEAN			bScan);

VOID AsicLockChannel(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR Channel) ;

VOID AsicAntennaSelect(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Channel);


VOID AsicResetBBPAgent(
	IN PRTMP_ADAPTER pAd);


VOID AsicSetBssid(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PUCHAR pBssid);

VOID AsicSetMcastWC(
	IN PRTMP_ADAPTER pAd);


VOID AsicDelWcidTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	Wcid);

#ifdef DOT11_N_SUPPORT
VOID AsicEnableRDG(
	IN PRTMP_ADAPTER pAd);

VOID AsicDisableRDG(
	IN PRTMP_ADAPTER pAd);
#endif // DOT11_N_SUPPORT //

VOID AsicDisableSync(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicEnableBssSync(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicEnableIbssSync(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicSetEdcaParm(
	IN PRTMP_ADAPTER pAd,
	IN PEDCA_PARM    pEdcaParm);

VOID AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime);

//
// Update the Tx chain address
//
// Parameters
//	pAd: The adapter data structure
//	pMacAddress: The MAC address of the peer STA
//
// Return Value:
//	None
//
VOID AiscUpdateTxChainAddress(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pMacAddress);

VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         BssIndex,
	IN UCHAR         KeyIdx,
	IN PCIPHER_KEY	 pCipherKey);

VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         BssIndex,
	IN UCHAR         KeyIdx);

VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV);

VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN PUCHAR        pAddr);

VOID	AsicUpdateWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8				Wcid,
	IN	UINT8				KeyTabFlag);

VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey);

VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER  pAd,
	IN UCHAR		 Wcid);

BOOLEAN AsicSendCommandToMcu(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN		 bNeedSpinLock,
	IN UCHAR         Command,
	IN UCHAR         Token,
	IN UCHAR         Arg0,
	IN UCHAR         Arg1);

BOOLEAN AsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN		FlgIsNeedLocked,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		bNeedMcuCmdLock);


#ifdef RTMP_MAC_PCI
BOOLEAN AsicCheckCommanOk(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command);
#endif // RTMP_MAC_PCI //

#ifdef WAPI_SUPPORT
VOID AsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high);
#endif // WAPI_SUPPORT //


VOID MacAddrRandomBssid(
	IN  PRTMP_ADAPTER   pAd, 
	OUT PUCHAR pAddr);

VOID MgtMacHeaderInit(
	IN  PRTMP_ADAPTER     pAd, 
	IN OUT PHEADER_802_11 pHdr80211, 
	IN UCHAR SubType, 
	IN UCHAR ToDs, 
	IN PUCHAR pDA, 
	IN PUCHAR pBssid);

VOID MlmeRadioOff(
	IN PRTMP_ADAPTER pAd);

VOID MlmeRadioOn(
	IN PRTMP_ADAPTER pAd);


VOID BssTableInit(
	IN BSS_TABLE *Tab);

#ifdef DOT11_N_SUPPORT
VOID BATableInit(
	IN PRTMP_ADAPTER pAd,
    IN BA_TABLE *Tab);

VOID BATableExit(	
	IN RTMP_ADAPTER *pAd);
#endif // DOT11_N_SUPPORT //

ULONG BssTableSearch(
	IN BSS_TABLE *Tab, 
	IN PUCHAR pBssid,
	IN UCHAR Channel);

ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab, 
	IN PUCHAR    pBssid,
	IN PUCHAR    pSsid,
	IN UCHAR     SsidLen,
	IN UCHAR     Channel);

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab, 
	IN PUCHAR    Bssid,
	IN PUCHAR    pSsid,
	IN UCHAR     SsidLen,
	IN UCHAR     Channel);

ULONG BssSsidTableSearchBySSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen);

VOID BssTableDeleteEntry(
	IN OUT  PBSS_TABLE pTab, 
	IN      PUCHAR pBssid,
	IN      UCHAR Channel);

VOID  BssEntrySet(
	IN  PRTMP_ADAPTER   pAd, 
	OUT PBSS_ENTRY pBss, 
	IN PUCHAR pBssid, 
	IN CHAR Ssid[], 
	IN UCHAR SsidLen, 
	IN UCHAR BssType, 
	IN USHORT BeaconPeriod,
	IN PCF_PARM CfParm, 
	IN USHORT AtimWin, 
	IN USHORT CapabilityInfo, 
	IN UCHAR SupRate[], 
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[], 
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	// AP might use this additional ht info IE 
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR Channel,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

ULONG  BssTableSetEntry(
	IN  PRTMP_ADAPTER   pAd, 
	OUT PBSS_TABLE pTab, 
	IN PUCHAR pBssid, 
	IN CHAR Ssid[], 
	IN UCHAR SsidLen, 
	IN UCHAR BssType, 
	IN USHORT BeaconPeriod, 
	IN CF_PARM *CfParm, 
	IN USHORT AtimWin, 
	IN USHORT CapabilityInfo, 
	IN UCHAR SupRate[], 
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	// AP might use this additional ht info IE 
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR Channel,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

#ifdef DOT11_N_SUPPORT
VOID BATableInsertEntry(
    IN	PRTMP_ADAPTER	pAd, 
	IN USHORT Aid,    
    IN USHORT		TimeOutValue,
	IN USHORT		StartingSeq,
    IN UCHAR TID, 
	IN UCHAR BAWinSize, 
	IN UCHAR OriginatorStatus, 
    IN BOOLEAN IsRecipient);

#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);


VOID  TriEventInit(
	IN	PRTMP_ADAPTER	pAd);

INT TriEventTableSetEntry(
	IN	PRTMP_ADAPTER	pAd, 
	OUT TRIGGER_EVENT_TAB *Tab, 
	IN PUCHAR pBssid, 
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			RegClass,
	IN UCHAR ChannelNo);

#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

VOID BssTableSsidSort(
	IN  PRTMP_ADAPTER   pAd, 
	OUT BSS_TABLE *OutTab, 
	IN  CHAR Ssid[], 
	IN  UCHAR SsidLen);

VOID  BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab);

VOID BssCipherParse(
	IN OUT  PBSS_ENTRY  pBss);

NDIS_STATUS  MlmeQueueInit(
	IN MLME_QUEUE *Queue);

VOID  MlmeQueueDestroy(
	IN MLME_QUEUE *Queue);

BOOLEAN MlmeEnqueue(
	IN PRTMP_ADAPTER pAd, 
	IN ULONG Machine, 
	IN ULONG MsgType, 
	IN ULONG MsgLen, 
	IN VOID *Msg,
	IN ULONG Priv);

BOOLEAN MlmeEnqueueForRecv(
	IN  PRTMP_ADAPTER   pAd, 
	IN ULONG Wcid, 
	IN ULONG TimeStampHigh, 
	IN ULONG TimeStampLow, 
	IN UCHAR Rssi0, 
	IN UCHAR Rssi1, 
	IN UCHAR Rssi2, 
	IN ULONG MsgLen, 
	IN PVOID Msg,
	IN UCHAR Signal);

#ifdef WSC_INCLUDED
BOOLEAN MlmeEnqueueForWsc(
	IN PRTMP_ADAPTER	pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg);
#endif // WSC_INCLUDED //

BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue, 
	OUT MLME_QUEUE_ELEM **Elem);

VOID    MlmeRestartStateMachine(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN  MlmeQueueEmpty(
	IN MLME_QUEUE *Queue);

BOOLEAN  MlmeQueueFull(
	IN MLME_QUEUE *Queue);

BOOLEAN  MsgTypeSubst(
	IN PRTMP_ADAPTER pAd, 
	IN PFRAME_802_11 pFrame, 
	OUT INT *Machine, 
	OUT INT *MsgType);

VOID StateMachineInit(
	IN STATE_MACHINE *Sm, 
	IN STATE_MACHINE_FUNC Trans[], 
	IN ULONG StNr, 
	IN ULONG MsgNr, 
	IN STATE_MACHINE_FUNC DefFunc, 
	IN ULONG InitState, 
	IN ULONG Base);

VOID StateMachineSetAction(
	IN STATE_MACHINE *S, 
	IN ULONG St, 
	ULONG Msg, 
	IN STATE_MACHINE_FUNC F);

VOID StateMachinePerformAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN STATE_MACHINE *S, 
	IN MLME_QUEUE_ELEM *Elem,
	IN ULONG CurrState);

VOID Drop(
	IN  PRTMP_ADAPTER   pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID AssocStateMachineInit(
	IN  PRTMP_ADAPTER   pAd, 
	IN  STATE_MACHINE *Sm, 
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ReassocTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID AssocTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID DisassocTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

//----------------------------------------------
VOID MlmeDisassocReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeAssocReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeReassocReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeDisassocReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAssocRspAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerReassocRspAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerDisassocAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID DisassocTimeoutAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID AssocTimeoutAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID  ReassocTimeoutAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID  Cls3errAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PUCHAR pAddr);

VOID  InvalidStateWhenAssoc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID  InvalidStateWhenReassoc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenDisassociate(
	IN  PRTMP_ADAPTER pAd, 
	IN  MLME_QUEUE_ELEM *Elem);


VOID  ComposePsPoll(
	IN  PRTMP_ADAPTER   pAd);

VOID  ComposeNullFrame(
	IN  PRTMP_ADAPTER pAd);

VOID  AssocPostProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PUCHAR pAddr2, 
	IN  USHORT CapabilityInfo, 
	IN  USHORT Aid, 
	IN  UCHAR SupRate[], 
	IN  UCHAR SupRateLen,
	IN  UCHAR ExtRate[],
	IN  UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE		*pHtCapability,
	IN  UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE		*pAddHtInfo);

VOID AuthStateMachineInit(
	IN  PRTMP_ADAPTER   pAd, 
	IN PSTATE_MACHINE sm, 
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AuthTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID MlmeAuthReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq2Action(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq4Action(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID AuthTimeoutAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID Cls2errAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PUCHAR pAddr);

VOID MlmeDeauthReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenAuth(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

//=============================================

VOID AuthRspStateMachineInit(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PSTATE_MACHINE Sm, 
	IN  STATE_MACHINE_FUNC Trans[]);

VOID PeerDeauthAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAuthSimpleRspGenAndSend(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PHEADER_802_11  pHdr80211, 
	IN  USHORT Alg, 
	IN  USHORT Seq, 
	IN  USHORT Reason, 
	IN  USHORT Status);

//
// Private routines in dls.c
//
#ifdef CONFIG_AP_SUPPORT
#ifdef QOS_DLS_SUPPORT
VOID APDLSStateMachineInit(
    IN	PRTMP_ADAPTER		pAd, 
    IN  STATE_MACHINE		*S, 
    OUT STATE_MACHINE_FUNC	Trans[]);

VOID APPeerDlsReqAction(
    IN PRTMP_ADAPTER	pAd, 
    IN MLME_QUEUE_ELEM	*Elem);

VOID APPeerDlsRspAction(
    IN PRTMP_ADAPTER	pAd, 
    IN MLME_QUEUE_ELEM	*Elem);

VOID APPeerDlsTearDownAction(
    IN PRTMP_ADAPTER	pAd, 
    IN MLME_QUEUE_ELEM	*Elem);
#endif // QOS_DLS_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#ifdef QOS_DLS_SUPPORT
BOOLEAN PeerDlsReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pDlsTimeout,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability);

BOOLEAN PeerDlsRspSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pStatus,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability);

BOOLEAN PeerDlsTearDownSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pReason);
#endif // QOS_DLS_SUPPORT //

BOOLEAN PeerProbeReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *SsidLen,
    OUT BOOLEAN *bRequestRssi);

//========================================

VOID SyncStateMachineInit(
	IN  PRTMP_ADAPTER   pAd, 
	IN  STATE_MACHINE *Sm, 
	OUT STATE_MACHINE_FUNC Trans[]);

VOID BeaconTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID ScanTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID MlmeScanReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenScan(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenJoin(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenStart(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID EnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd);

BOOLEAN ScanRunning(
		IN PRTMP_ADAPTER pAd);
//=========================================

VOID MlmeCntlInit(
	IN  PRTMP_ADAPTER   pAd, 
	IN  STATE_MACHINE *S, 
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeCntlMachinePerformAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  STATE_MACHINE *S, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlIdleProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidScanProc(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidSsidProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlOidRTBssidProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlMlmeRoamingProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlWaitDisassocProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitJoinProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitReassocProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitStartProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc2(
	IN  PRTMP_ADAPTER pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAssocProc(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

#ifdef QOS_DLS_SUPPORT
VOID CntlOidDLSSetupProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif // QOS_DLS_SUPPORT //


VOID LinkUp(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR BssType);

VOID LinkDown(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN         IsReqFromAP);

VOID IterateOnBssTab(
	IN  PRTMP_ADAPTER   pAd);

VOID IterateOnBssTab2(
	IN  PRTMP_ADAPTER   pAd);;

VOID JoinParmFill(
	IN  PRTMP_ADAPTER   pAd, 
	IN  OUT MLME_JOIN_REQ_STRUCT *JoinReq, 
	IN  ULONG BssIdx);

VOID AssocParmFill(
	IN  PRTMP_ADAPTER   pAd, 
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq, 
	IN  PUCHAR pAddr, 
	IN  USHORT CapabilityInfo, 
	IN  ULONG Timeout, 
	IN  USHORT ListenIntv);

VOID ScanParmFill(
	IN  PRTMP_ADAPTER   pAd, 
	IN  OUT MLME_SCAN_REQ_STRUCT *ScanReq, 
	IN  STRING Ssid[], 
	IN  UCHAR SsidLen, 
	IN  UCHAR BssType, 
	IN  UCHAR ScanType); 

VOID DisassocParmFill(
	IN  PRTMP_ADAPTER   pAd, 
	IN  OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq, 
	IN  PUCHAR pAddr, 
	IN  USHORT Reason);

VOID StartParmFill(
	IN  PRTMP_ADAPTER   pAd, 
	IN  OUT MLME_START_REQ_STRUCT *StartReq, 
	IN  CHAR Ssid[], 
	IN  UCHAR SsidLen);

VOID AuthParmFill(
	IN  PRTMP_ADAPTER   pAd, 
	IN  OUT MLME_AUTH_REQ_STRUCT *AuthReq, 
	IN  PUCHAR pAddr, 
	IN  USHORT Alg);

VOID EnqueuePsPoll(
	IN  PRTMP_ADAPTER   pAd);

VOID EnqueueBeaconFrame(
	IN  PRTMP_ADAPTER   pAd); 

VOID MlmeJoinReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeScanReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeStartReqAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID ScanTimeoutAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID BeaconTimeoutAtJoinAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtScanAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtJoinAction(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerProbeReqAction(
	IN  PRTMP_ADAPTER pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID ScanNextChannel(
	IN  PRTMP_ADAPTER   pAd);

ULONG MakeIbssBeacon(
	IN  PRTMP_ADAPTER   pAd);


BOOLEAN MlmeScanReqSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	OUT UCHAR *BssType, 
	OUT CHAR ssid[], 
	OUT UCHAR *SsidLen, 
	OUT UCHAR *ScanType);

BOOLEAN PeerBeaconAndProbeRspSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	IN  UCHAR MsgChannel,
	OUT PUCHAR pAddr2, 
	OUT PUCHAR pBssid, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT UCHAR *pBssType, 
	OUT USHORT *pBeaconPeriod, 
	OUT UCHAR *pChannel, 
	OUT UCHAR *pNewChannel, 
	OUT LARGE_INTEGER *pTimestamp, 
	OUT CF_PARM *pCfParm, 
	OUT USHORT *pAtimWin, 
	OUT USHORT *pCapabilityInfo, 
	OUT UCHAR *pErp,
	OUT UCHAR *pDtimCount, 
	OUT UCHAR *pDtimPeriod, 
	OUT UCHAR *pBcastFlag, 
	OUT UCHAR *pMessageToMe, 
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT	UCHAR *pCkipFlag,
	OUT	UCHAR *pAironetCellPowerLimit,
	OUT PEDCA_PARM       pEdcaParm,
	OUT PQBSS_LOAD_PARM  pQbssLoad,
	OUT PQOS_CAPABILITY_PARM pQosCapability,
	OUT ULONG *pRalinkIe,
	OUT UCHAR		 *pHtCapabilityLen,
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT EXT_CAP_INFO_ELEMENT *pExtCapInfo,
	OUT UCHAR		 *AddHtInfoLen,
	OUT ADD_HT_INFO_IE *AddHtInfo,
	OUT UCHAR *NewExtChannel,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE);


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
BOOLEAN PeerBeaconAndProbeRspSanity2(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	IN OVERLAP_BSS_SCAN_IE *BssScan,
	OUT UCHAR 	*RegClass);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

BOOLEAN PeerAddBAReqActionSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen,
	OUT PUCHAR pAddr2);

BOOLEAN PeerAddBARspActionSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen);

BOOLEAN PeerDelBAActionSanity(
    IN PRTMP_ADAPTER pAd, 
    IN UCHAR Wcid, 
    IN VOID *pMsg, 
    IN ULONG MsgLen);

BOOLEAN MlmeAssocReqSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	OUT PUCHAR pApAddr, 
	OUT USHORT *CapabilityInfo, 
	OUT ULONG *Timeout, 
	OUT USHORT *ListenIntv);

BOOLEAN MlmeAuthReqSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	OUT PUCHAR pAddr, 
	OUT ULONG *Timeout, 
	OUT USHORT *Alg);

BOOLEAN MlmeStartReqSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	OUT CHAR Ssid[], 
	OUT UCHAR *Ssidlen);

BOOLEAN PeerAuthSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	OUT PUCHAR pAddr, 
	OUT USHORT *Alg, 
	OUT USHORT *Seq, 
	OUT USHORT *Status, 
	OUT CHAR ChlgText[]);

BOOLEAN PeerAssocRspSanity(
	IN  PRTMP_ADAPTER   pAd, 
    IN VOID *pMsg, 
	IN  ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT USHORT *pCapabilityInfo, 
	OUT USHORT *pStatus, 
	OUT USHORT *pAid, 
	OUT UCHAR SupRate[], 
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE		*pHtCapability,
    OUT ADD_HT_INFO_IE		*pAddHtInfo,	// AP might use this additional ht info IE 
    OUT UCHAR			*pHtCapabilityLen,
    OUT UCHAR			*pAddHtInfoLen,
    OUT UCHAR			*pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT EXT_CAP_INFO_ELEMENT *pExtCapInfo,
	OUT UCHAR *pCkipFlag);

BOOLEAN PeerDisassocSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT USHORT *Reason);

BOOLEAN PeerDeauthSanity(
	IN  PRTMP_ADAPTER   pAd, 
	IN  VOID *Msg, 
	IN  ULONG MsgLen, 
	OUT PUCHAR pAddr1,
	OUT PUCHAR pAddr2, 
	OUT PUCHAR pAddr3, 
	OUT USHORT *Reason);

BOOLEAN GetTimBit(
	IN  CHAR *Ptr, 
	IN  USHORT Aid, 
	OUT UCHAR *TimLen, 
	OUT UCHAR *BcastFlag, 
	OUT UCHAR *DtimCount, 
	OUT UCHAR *DtimPeriod, 
	OUT UCHAR *MessageToMe);

UCHAR ChannelSanity(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR channel);

NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(
	IN PBSS_ENTRY pBss);

BOOLEAN MlmeDelBAReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen);

BOOLEAN MlmeAddBAReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2);

ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer, 
	OUT ULONG *Length, ...);

UCHAR RandomByte(
	IN  PRTMP_ADAPTER   pAd);

UCHAR RandomByte2(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicUpdateAutoFallBackTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pTxRate);
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
	
VOID AsicCheckForHwRecovery(
	IN PRTMP_ADAPTER	pAd) ;
#endif // defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) //
VOID  MlmePeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID LinkDownExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID LinkUpExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID STAMlmePeriodicExec(
	PRTMP_ADAPTER pAd);

VOID MlmeAutoScan(
	IN PRTMP_ADAPTER pAd);

VOID MlmeAutoReconnectLastSSID(
	IN PRTMP_ADAPTER pAd);

BOOLEAN MlmeValidateSSID(
	IN PUCHAR pSsid,
	IN UCHAR  SsidLen); 

VOID MlmeCheckForRoaming(
	IN PRTMP_ADAPTER pAd,
	IN ULONG    Now32);

BOOLEAN MlmeCheckForFastRoaming(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeDynamicTxRateSwitching(
	IN PRTMP_ADAPTER pAd);

#ifdef NEW_RATE_ADAPT_SUPPORT
VOID MlmeDynamicTxRateSwitchingAdapt(
	IN PRTMP_ADAPTER pAd,
	IN ULONG i);
#endif // NEW_RATE_ADAPT_SUPPORT //

VOID MlmeSelectRateSwitchTable11N3SReplacement(
	IN PUCHAR	*ppTable);

#ifdef AGS_SUPPORT
INT Show_AGS_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  PSTRING			arg);

//
// The dynamic Tx rate switching for AGS (Adaptive Group Switching)
//
// Parameters
//	pAd: The adapter data structure
//	pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//	pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//	TableSize: The size, in bytes, of the specified Tx rate switching table
//	pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//
// Return Value:
//	None
//
VOID MlmeDynamicTxRateSwitchingAGS(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY pEntry, 
	IN PUCHAR pTable, 
	IN UCHAR TableSize, 
	IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
	IN UCHAR InitTxRateIdx);

//
// Auto Tx rate faster train up/down for AGS (Adaptive Group Switching)
//
// Parameters
//	pAd: The adapter data structure
//	pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//	pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//	TableSize: The size, in bytes, of the specified Tx rate switching table
//	pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//
// Return Value:
//	None
//
VOID StaQuickResponeForRateUpExecAGS(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY pEntry, 
	IN PUCHAR pTable, 
	IN UCHAR TableSize, 
	IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
	IN UCHAR InitTxRateIdx);
#endif // AGS_SUPPORT //

VOID MlmeSetTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PRTMP_TX_RATE_SWITCH	pTxRate);

VOID MlmeSelectTxRateTable(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PUCHAR				*ppTable,
	IN PUCHAR				pTableSize,
	IN PUCHAR				pInitTxRateIdx);

VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now);

VOID MlmeCheckPsmChange(
	IN PRTMP_ADAPTER pAd,
	IN ULONG    Now32);

VOID MlmeSetPsmBit(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT psm);

VOID MlmeSetTxPreamble(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TxPreamble);

VOID UpdateBasicRateBitmap(
	IN	PRTMP_ADAPTER	pAd);

VOID MlmeUpdateTxRates(
	IN PRTMP_ADAPTER 	pAd,
	IN 	BOOLEAN		 	bLinkUp,
	IN	UCHAR			apidx);

#ifdef DOT11_N_SUPPORT
VOID MlmeUpdateHtTxRates(
	IN PRTMP_ADAPTER 		pAd,
	IN	UCHAR				apidx);
#endif // DOT11_N_SUPPORT //

VOID    RTMPCheckRates(
	IN      PRTMP_ADAPTER   pAd,
	IN OUT  UCHAR           SupRate[],
	IN OUT  UCHAR           *SupRateLen);


BOOLEAN 	RTMPCheckHt(
	IN		PRTMP_ADAPTER	pAd,
	IN		UCHAR	Wcid,
	IN OUT	HT_CAPABILITY_IE			*pHtCapability,
	IN OUT	ADD_HT_INFO_IE			*pAddHtInfo);

VOID StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

#ifdef NEW_RATE_ADAPT_SUPPORT
VOID StaQuickResponeForRateUpExecAdapt(
	IN PRTMP_ADAPTER	pAd,
	IN ULONG i);
#endif // NEW_RATE_ADAPT_SUPPORT //

VOID RTMPUpdateMlmeRate(
	IN PRTMP_ADAPTER	pAd);

CHAR RTMPMaxRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

CHAR RTMPMinSnr(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Snr0,
	IN CHAR				Snr1);

CHAR RTMPMinNoiseLevel(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				NoiseLevel0,
	IN CHAR				NoiseLevel1,
	IN CHAR				NoiseLevel2);

VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);

#ifdef RT30xx

VOID RTMPFilterCalibration(
	IN PRTMP_ADAPTER pAd);

#ifdef RTMP_EFUSE_SUPPORT
//2008/09/11:KH add to support efuse<--
INT set_eFuseGetFreeBlockCount_Proc(  
   	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT set_eFusedump_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT set_eFuseLoadFromBin_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

VOID eFusePhysicalReadRegisters( 
	IN	PRTMP_ADAPTER	pAd, 
	IN	USHORT Offset, 
	IN	USHORT Length, 
	OUT	USHORT* pData);

int RtmpEfuseSupportCheck(
	IN RTMP_ADAPTER *pAd);

#ifdef RALINK_ATE
INT set_eFuseBufferModeWriteBack_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif // RALINK_ATE //

INT eFuseLoadEEPROM(
	IN PRTMP_ADAPTER pAd);

INT eFuseWriteEeeppromBuf(
	IN PRTMP_ADAPTER pAd);

VOID eFuseGetFreeBlockCount(IN PRTMP_ADAPTER pAd, 
	PUINT EfuseFreeBlock);

INT eFuse_init(
	IN PRTMP_ADAPTER pAd);

NTSTATUS eFuseRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUSHORT			pData,
	IN	USHORT			Length);

NTSTATUS eFuseWrite(  
   	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUSHORT			pData,
	IN	USHORT			length);
//2008/09/11:KH add to support efuse-->
#endif // RTMP_EFUSE_SUPPORT //

// add by johnli, RF power sequence setup
VOID RT30xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd);

VOID RT30xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd);

VOID RT30xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd);
// end johnli


#ifdef RT3090
VOID NICInitRT3090RFRegisters(
	IN RTMP_ADAPTER *pAd);
#endif // RT3090 //

VOID RT30xxHaltAction(
	IN PRTMP_ADAPTER 	pAd);

VOID RT30xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);

VOID PostBBPInitialization(
	IN PRTMP_ADAPTER	pAd);
#endif // RT30xx //
#ifdef RT33xx
VOID RT33xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd);

VOID RT33xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd);

VOID RT33xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd);


#ifdef RT3390
VOID NICInitRT3390RFRegisters(
	IN RTMP_ADAPTER *pAd);
#endif // RT3090 //

VOID RT33xxHaltAction(
	IN PRTMP_ADAPTER 	pAd);

VOID RT33xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);

#endif // RT33xx //



#ifdef RT53xx
VOID NICInitRT5390RFRegisters(
	IN PRTMP_ADAPTER pAd);
VOID RT5390_PostBBPInitialization(
	IN PRTMP_ADAPTER pAd);
NTSTATUS	RT5392WriteBBPR66(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Value);
VOID RT5390SetRxAnt(
    IN PRTMP_ADAPTER    pAd,
    IN UCHAR            Ant);
#endif // RT53xx //


VOID AsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd);

VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

UCHAR RTMPStaFixedTxMode(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

VOID RTMPUpdateLegacyTxSetting(
		UCHAR				fixed_tx_mode,
		PMAC_TABLE_ENTRY	pEntry);

BOOLEAN RTMPAutoRateSwitchCheck(
	IN PRTMP_ADAPTER    pAd);

NDIS_STATUS MlmeInit(
	IN  PRTMP_ADAPTER   pAd);

#ifdef RTMP_FREQ_CALIBRATION_SUPPORT
#ifdef RT53xx
	void RFMultiStepXoCode(
	IN	PRTMP_ADAPTER pAd,
	IN	UCHAR	rfRegID,
	IN	UCHAR	rfRegValue,
	IN	UCHAR	rfRegValuePre);
#endif // RT53xx//
#endif // RTMP_FREQ_CALIBRATION_SUPPORT //

#ifdef RTMP_INTERNAL_TX_ALC
//
// Initialize the desired TSSI table
//
// Parameters
//	pAd: The adapter data structure
//
// Return Value:
//	None
//
VOID InitDesiredTSSITable(
	IN PRTMP_ADAPTER pAd);

#if defined(RT5390)  || defined(RT5370)
VOID InitLookupTable(
	IN PRTMP_ADAPTER pAd);
#endif // defined(RT5390)  || defined(RT5370) //
//
// Rounding to integer
// e.g., +16.9 ~= 17 and -16.9 ~= -17
//
// Parameters
//	pAd: The adapter data structure
//	Integer: Integer part
//	Fraction: Fraction part
//	DenominatorOfTssiRatio: The denominator of the TSSI ratio
//
// Return Value:
//	Rounding result
//
LONG Rounding(
	IN PRTMP_ADAPTER pAd, 
	IN LONG Integer, 
	IN LONG Fraction, 
	IN LONG DenominatorOfTssiRatio);

//
// Get the desired TSSI based on the latest packet
//
// Parameters
//	pAd: The adapter data structure
//	pDesiredTssi: The desired TSSI
//	pCurrentTssi: The current TSSI
//
// Return Value:
//	Success or failure
//
BOOLEAN GetDesiredTssiAndCurrentTssi(
	IN PRTMP_ADAPTER pAd, 
	IN OUT PCHAR pDesiredTssi, 
	IN OUT PCHAR pCurrentTssi);

//
// Get the desired TSSI based on the latest packet
//
// Parameters
//	pAd: The adapter data structure
//
// Return Value:
//	The desired TSSI
//
UCHAR GetDesiredTSSI(
	IN PRTMP_ADAPTER pAd);
#endif // RTMP_INTERNAL_TX_ALC //

VOID MlmeHandler(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeHalt(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeResetRalinkCounters(
	IN  PRTMP_ADAPTER   pAd);

VOID BuildChannelList(
	IN PRTMP_ADAPTER pAd);

UCHAR FirstChannel(
	IN  PRTMP_ADAPTER   pAd);

UCHAR NextChannel(
	IN  PRTMP_ADAPTER   pAd, 
	IN  UCHAR channel);

VOID ChangeToCellPowerLimit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         AironetCellPowerLimit);

//
// Prototypes of function definition in cmm_tkip.c
//
VOID    RTMPInitMICEngine(
	IN  PRTMP_ADAPTER   pAd,    
	IN  PUCHAR          pKey,
	IN  PUCHAR          pDA,
	IN  PUCHAR          pSA,
	IN  UCHAR           UserPriority,
	IN  PUCHAR          pMICKey);

BOOLEAN RTMPTkipCompareMICValue(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pSrc,
	IN  PUCHAR          pDA,
	IN  PUCHAR          pSA,
	IN  PUCHAR          pMICKey,
	IN	UCHAR			UserPriority,
	IN  UINT            Len);

VOID    RTMPCalculateMICValue(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    pPacket,
	IN  PUCHAR          pEncap,
	IN  PCIPHER_KEY     pKey,
	IN	UCHAR			apidx);

VOID    RTMPTkipAppendByte( 
	IN  PTKIP_KEY_INFO  pTkip,  
	IN  UCHAR           uChar);

VOID    RTMPTkipAppend( 
	IN  PTKIP_KEY_INFO  pTkip,  
	IN  PUCHAR          pSrc,
	IN  UINT            nBytes);

VOID    RTMPTkipGetMIC( 
	IN  PTKIP_KEY_INFO  pTkip);

//
// Prototypes of function definition in cmm_cfg.c
//
INT RT_CfgSetCountryRegion(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg,
	IN INT				band);

INT RT_CfgSetWirelessMode(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
INT RT_CfgSetMbssWirelessMode(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

BOOLEAN RT_CfgMbssWirelessModeSameBand(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			WirelessModeNew);

UCHAR RT_CfgMbssWirelessModeMaxGet(
	IN	PRTMP_ADAPTER	pAd);
#endif // MBSS_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

INT RT_CfgSetShortSlot(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	RT_CfgSetWepKey(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx);

INT RT_CfgSetWPAPSKKey(
	IN RTMP_ADAPTER	*pAd, 
	IN PSTRING		keyString,
	IN UCHAR		*pHashStr,
	IN INT			hashStrLen,
	OUT PUCHAR		pPMKBuf);

INT	RT_CfgSetFixedTxPhyMode(
	IN	PSTRING			arg);

INT	RT_CfgSetMacAddress(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PSTRING			arg);

INT	RT_CfgSetTxMCSProc(
	IN	PSTRING			arg,
	OUT	BOOLEAN			*pAutoRate);

INT	RT_CfgSetAutoFallBack(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PSTRING			arg);

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING		pPinCodeStr,
	OUT PWSC_CTRL   pWscControl);
#endif // WSC_INCLUDED //


//
// Prototypes of function definition in cmm_info.c
//
NDIS_STATUS RTMPWPARemoveKeyProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  PVOID           pBuf);

VOID    RTMPWPARemoveAllKeys(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN RTMPCheckStrPrintAble(
    IN  CHAR *pInPutStr, 
    IN  UCHAR strLen);
    
VOID    RTMPSetPhyMode(
	IN  PRTMP_ADAPTER   pAd,
	IN  ULONG phymode);

VOID	RTMPUpdateHTIE(
	IN	RT_HT_CAPABILITY	*pRtHt,
	IN		UCHAR				*pMcsSet,
	OUT		HT_CAPABILITY_IE *pHtCapability,
	OUT		ADD_HT_INFO_IE		*pAddHtInfo);

VOID	RTMPAddWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN 	MAC_TABLE_ENTRY *pEntry);

PSTRING GetEncryptType(
	CHAR enc);

PSTRING GetAuthMode(
	CHAR auth);

#ifdef DOT11_N_SUPPORT
VOID	RTMPSetHT(
	IN	PRTMP_ADAPTER	pAd,
	IN	OID_SET_HT_PHYMODE *pHTPhyMode);

VOID	RTMPSetIndividualHT(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				apidx);
#endif // DOT11_N_SUPPORT //

#ifdef SYSTEM_LOG_SUPPORT
VOID RTMPSendWirelessEvent(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Event_flag,
	IN	PUCHAR 			pAddr,
	IN  UCHAR			BssIdx,
	IN	CHAR			Rssi);
#else
#define RTMPSendWirelessEvent(_pAd, _Event_flag, _pAddr, _BssIdx, _Rssi)
#endif // SYSTEM_LOG_SUPPORT //
	
CHAR    ConvertToRssi(
	IN PRTMP_ADAPTER  pAd,
	IN CHAR				Rssi,
	IN UCHAR    RssiNumber);

CHAR    ConvertToSnr(
	IN PRTMP_ADAPTER  	pAd,
	IN UCHAR			Snr);

#ifdef DOT11N_DRAFT3
VOID BuildEffectedChannelList(
	IN PRTMP_ADAPTER pAd);


VOID DeleteEffectedChannelList(
	IN PRTMP_ADAPTER pAd);

VOID CntlChannelWidth(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			PrimaryChannel,
	IN UCHAR			CentralChannel,	
	IN UCHAR			ChannelWidth,
	IN UCHAR			SecondaryChannelOffset);

#endif // DOT11N_DRAFT3 //


VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd);

#ifdef ANT_DIVERSITY_SUPPORT
VOID	APAsicAntennaAvg(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	              AntSelect,
	IN	SHORT	              *RssiAvg);
VOID HWAntennaDiversityEnable(
	IN PRTMP_ADAPTER		pAd);
#endif // ANT_DIVERSITY_SUPPORT //

VOID APAsicRxAntEvalTimeout(
	IN PRTMP_ADAPTER	pAd);


// 
// function prototype in ap_wpa.c
//
VOID RTMPGetTxTscFromAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pTxTsc);

MAC_TABLE_ENTRY *PACInquiry(
	IN  PRTMP_ADAPTER   pAd, 
	IN  ULONG           Wcid);

UINT	APValidateRSNIE(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR			pRsnIe,
	IN UCHAR			rsnie_len);

VOID HandleCounterMeasure(
	IN PRTMP_ADAPTER pAd, 
	IN MAC_TABLE_ENTRY  *pEntry);

VOID WPAStart4WayHS(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	ULONG			TimeInterval);

VOID WPAStart2WayGroupHS(
	IN  PRTMP_ADAPTER   pAd, 
	IN  MAC_TABLE_ENTRY *pEntry);

VOID PeerPairMsg1Action(
	IN PRTMP_ADAPTER pAd, 
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg2Action(
	IN PRTMP_ADAPTER pAd, 
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg3Action(
	IN PRTMP_ADAPTER pAd, 
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg4Action(
	IN PRTMP_ADAPTER pAd, 
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerGroupMsg1Action(
	IN  PRTMP_ADAPTER    pAd, 
	IN  PMAC_TABLE_ENTRY pEntry,
    IN  MLME_QUEUE_ELEM  *Elem);

VOID PeerGroupMsg2Action(
	IN  PRTMP_ADAPTER    pAd, 
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  VOID             *Msg,
	IN  UINT             MsgLen);

VOID CMTimerExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID WPARetryExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);


VOID EnqueueStartForPSKExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3); 

VOID RTMPHandleSTAKey(
    IN PRTMP_ADAPTER    pAdapter, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID MlmeDeAuthAction(
	IN  PRTMP_ADAPTER    pAd, 
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  USHORT           Reason,
	IN  BOOLEAN          bDataFrameFirst);

VOID GREKEYPeriodicExec(
	IN  PVOID   SystemSpecific1, 
	IN  PVOID   FunctionContext, 
	IN  PVOID   SystemSpecific2, 
	IN  PVOID   SystemSpecific3);

VOID AES_128_CMAC(
	IN	PUCHAR	key,
	IN	PUCHAR	input,
	IN	INT		len,
	OUT	PUCHAR	mac);

#ifdef DOT1X_SUPPORT
VOID    WpaSend(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          pPacket,
    IN  ULONG           Len);

VOID RTMPAddPMKIDCache(
	IN  PRTMP_ADAPTER   		pAd,
	IN	INT						apidx,
	IN	PUCHAR				pAddr,
	IN	UCHAR					*PMKID,
	IN	UCHAR					*PMK);

INT RTMPSearchPMKIDCache(
	IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx,
	IN	PUCHAR		pAddr);

VOID RTMPDeletePMKIDCache(
	IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx,
	IN  INT				idx);

VOID RTMPMaintainPMKIDCache(
	IN  PRTMP_ADAPTER   pAd);
#else
#define RTMPMaintainPMKIDCache(_pAd)
#endif // DOT1X_SUPPORT //

/* timeout -- ms */
VOID RTMP_SetPeriodicTimer(
	IN	NDIS_MINIPORT_TIMER *pTimer, 
	IN	unsigned long timeout);

VOID RTMP_OS_Init_Timer(
	IN	PRTMP_ADAPTER pAd, 
	IN	NDIS_MINIPORT_TIMER *pTimer, 
	IN	TIMER_FUNCTION function,
	IN	PVOID data);

VOID RTMP_OS_Add_Timer(
	IN	NDIS_MINIPORT_TIMER	*pTimer,
	IN	unsigned long timeout);

VOID RTMP_OS_Mod_Timer(
	IN	NDIS_MINIPORT_TIMER	*pTimer,
	IN	unsigned long timeout);


VOID RTMP_OS_Del_Timer(
	IN	NDIS_MINIPORT_TIMER	*pTimer,
	OUT	BOOLEAN				 *pCancelled);

VOID RTMPusecDelay(
	IN	ULONG	usec);

NDIS_STATUS os_alloc_mem(
	IN	RTMP_ADAPTER *pAd,
	OUT	UCHAR **mem,
	IN	ULONG  size);

NDIS_STATUS os_free_mem(
	IN	PRTMP_ADAPTER pAd,
	IN	PVOID mem);


#ifdef RESOURCE_PRE_ALLOC
VOID RTMPResetTxRxRingMemory(
	IN  RTMP_ADAPTER   *pAd);
#endif // RESOURCE_PRE_ALLOC //


VOID RTMPFreeTxRxRingMemory(
    IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS AdapterBlockAllocateMemory(
	IN PVOID	handle,
	OUT	PVOID	*ppAd);

void RTMP_AllocateTxDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT	Index,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_AllocateFirstTxBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT	Index,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_FreeFirstTxBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	IN	PVOID	VirtualAddress,
	IN	NDIS_PHYSICAL_ADDRESS PhysicalAddress);
	
void RTMP_AllocateMgmtDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_AllocateRxDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_FreeDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	PVOID	VirtualAddress,
	IN	NDIS_PHYSICAL_ADDRESS PhysicalAddress);

PNDIS_PACKET RtmpOSNetPktAlloc(
	IN RTMP_ADAPTER *pAd, 
	IN int size);
	
PNDIS_PACKET RTMP_AllocateRxPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

PNDIS_PACKET RTMP_AllocateFragPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length);

void RTMP_QueryPacketInfo(
	IN  PNDIS_PACKET pPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen);

BOOLEAN RTMP_FillTxBlkInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk);


PRTMP_SCATTER_GATHER_LIST
rt_get_sg_list_from_packet(PNDIS_PACKET pPacket, RTMP_SCATTER_GATHER_LIST *sg);


 void announce_802_3_packet(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket);

#ifdef DOT11_N_SUPPORT
UINT BA_Reorder_AMSDU_Annnounce(
	IN	PRTMP_ADAPTER	pAd, 	
	IN	PNDIS_PACKET	pPacket);
#endif // DOT11_N_SUPPORT //

PNET_DEV get_netdev_from_bssid(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			FromWhichBSSID);


PNDIS_PACKET duplicate_pkt(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID);


PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pOldPkt);
	
PNDIS_PACKET duplicate_pkt_with_VLAN(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID,
	IN	UCHAR			*TPID);

BOOLEAN RTMPL2FrameTxAction(
		IN  PRTMP_ADAPTER		pAd,
		IN	UCHAR				apidx,
		IN	PUCHAR				pData,
		IN	UINT32				data_len);

PNDIS_PACKET ExpandPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UINT32			ext_head_len,
	IN	UINT32			ext_tail_len);

UCHAR VLAN_8023_Header_Copy(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			pHeader802_3,
	IN	UINT            HdrLen,
	OUT PUCHAR			pData,
	IN	UCHAR			FromWhichBSSID,
	IN	UCHAR			*TPID);

#ifdef DOT11_N_SUPPORT
void ba_flush_reordering_timeout_mpdus(
	IN PRTMP_ADAPTER	pAd, 
	IN PBA_REC_ENTRY	pBAEntry,
	IN ULONG			Now32);


VOID BAOriSessionSetUp(
			IN PRTMP_ADAPTER    pAd, 
			IN MAC_TABLE_ENTRY	*pEntry,
			IN UCHAR			TID,
			IN USHORT			TimeOut,
			IN ULONG			DelayTime,
			IN BOOLEAN		isForced);

VOID BASessionTearDownALL(
	IN OUT	PRTMP_ADAPTER pAd, 
	IN		UCHAR Wcid);

VOID BAOriSessionTearDown(
	IN OUT	PRTMP_ADAPTER	pAd, 
	IN		UCHAR			Wcid,
	IN		UCHAR			TID,
	IN		BOOLEAN			bPassive,
	IN		BOOLEAN			bForceSend);

VOID BARecSessionTearDown(
	IN OUT	PRTMP_ADAPTER	pAd, 
	IN		UCHAR			Wcid,
	IN		UCHAR			TID,
	IN		BOOLEAN			bPassive);
#endif // DOT11_N_SUPPORT //

BOOLEAN ba_reordering_resource_init(PRTMP_ADAPTER pAd, int num);
void ba_reordering_resource_release(PRTMP_ADAPTER pAd);

INT ComputeChecksum(
	IN UINT PIN);

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
#ifdef CONFIG_AP_SUPPORT
    IN  BOOLEAN         bFromApcli,
#endif // CONFIG_AP_SUPPORT //
	IN	UCHAR	apidx);

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_WscVendorPinCode_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT	Set_WscResetPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef WSC_AP_SUPPORT
VOID RTMPIoctlSetWSCOOB(IN PRTMP_ADAPTER pAd);
#endif

#ifdef WSC_STA_SUPPORT
VOID CntlWscIterate(
	IN PRTMP_ADAPTER pAd);

USHORT WscGetAuthTypeFromStr(
    IN  PSTRING          arg);

USHORT WscGetEncrypTypeFromStr(
    IN  PSTRING          arg);
#endif // WSC_STA_SUPPORT //
//
// prototype in wsc.c
//
BOOLEAN	WscMsgTypeSubst(
	IN	UCHAR	EAPType,
	IN	UCHAR	EAPCode,
	OUT	INT	    *MsgType);

VOID    WscStateMachineInit(
	IN	PRTMP_ADAPTER		pAd, 
	IN	STATE_MACHINE		*S, 
	OUT STATE_MACHINE_FUNC Trans[]);

VOID    WscEAPOLStartAction(
    IN  PRTMP_ADAPTER    pAd, 
    IN  MLME_QUEUE_ELEM  *Elem);

VOID    WscEAPAction(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	MLME_QUEUE_ELEM *Elem);

VOID    WscEapEnrolleeAction(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR	        MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL       pWscControl);

#ifdef CONFIG_AP_SUPPORT
VOID    WscEapApProxyAction(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR	        MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL       pWscControl);
#endif // CONFIG_AP_SUPPORT //

VOID    WscEapRegistrarAction(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	MLME_QUEUE_ELEM	*Elem,
	IN  UCHAR	        MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL       pWscControl);
	
    
VOID    WscEAPOLTimeOutAction(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

VOID    Wsc2MinsTimeOutAction(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

UCHAR	WscRxMsgType(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PMLME_QUEUE_ELEM	pElem);

VOID	WscInitRegistrarPair(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN  UCHAR				apidx);

VOID    WscInitEntryFunc(
    IN  PMAC_TABLE_ENTRY pEntry);

VOID	WscSendEapReqId(
	IN	PRTMP_ADAPTER		pAd,
	IN	PMAC_TABLE_ENTRY	pEntry);

VOID    WscSendEapolStart(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pBssid);

VOID	WscSendEapRspId(
	IN	PRTMP_ADAPTER		pAd,
	IN  PMAC_TABLE_ENTRY    pEntry,
	IN  PWSC_CTRL           pWscControl);

VOID	WscMacHeaderInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN OUT	PHEADER_802_11	Hdr, 
	IN	PUCHAR 			pAddr1,
	IN  PUCHAR          pBSSID,
	IN  BOOLEAN         bFromApCli);

VOID	WscSendMessage(
	IN	PRTMP_ADAPTER		pAdapter, 
	IN  UCHAR               OpCode,
	IN  PUCHAR				pData,
	IN  INT					Len,
	IN  PWSC_CTRL           pWscControl,
    IN  UCHAR               OpMode,	// 0: AP Mode, 1: AP Client Mode, 2: STA Mode
    IN  UCHAR               EapType);

VOID	WscSendEapReqAck(
	IN	PRTMP_ADAPTER		pAdapter, 
	IN	PMAC_TABLE_ENTRY	pEntry);
	
VOID	WscSendEapReqDone(
	IN	PRTMP_ADAPTER		pAdapter, 
	IN	PMLME_QUEUE_ELEM	pElem);

VOID	WscSendEapFail(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN  BOOLEAN				bSendDeAuth);

VOID WscM2DTimeOutAction(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

VOID WscUPnPM2DTimeOutAction(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

VOID WscUPnPMsgTimeOutAction(
	IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

int WscSendUPnPConfReqMsg(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apIdx,
	IN PUCHAR ssidStr,
	IN PUCHAR macAddr,
	IN INT	  Status,
	IN UINT   eventID);

	
int WscSendUPnPMessage(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				apIdx,
	IN	USHORT				msgType,
	IN	USHORT				msgSubType,
	IN	PUCHAR				pData,
	IN	INT					dataLen,
	IN	UINT				eventID,
	IN	UINT				toIPAddr,
	IN	PUCHAR				pMACAddr);

VOID WscUPnPErrHandle(
	IN PRTMP_ADAPTER 	pAd,
	IN PWSC_CTRL		pWscControl,
	IN UINT 			eventID);

VOID    WscBuildBeaconIE(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	UCHAR b_configured,
	IN	BOOLEAN b_selRegistrar,
	IN	USHORT devPwdId,
	IN	USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  INT   AuthorizedMACsLen);

VOID    WscBuildProbeRespIE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	UCHAR respType,
	IN	UCHAR scState,
	IN	BOOLEAN b_selRegistrar,
	IN	USHORT devPwdId,
	IN	USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  INT   AuthorizedMACsLen);

#ifdef CONFIG_AP_SUPPORT
VOID WscBuildAssocRespIE(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR 			ApIdx,
	IN  UCHAR			Reason,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

VOID	WscSelectedRegistrar(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR	RegInfo,
	IN	UINT	length,
	IN  UCHAR 	apidx);

VOID    WscInformFromWPA(
    IN  PMAC_TABLE_ENTRY    pEntry);
#endif // CONFIG_AP_SUPPORT //


VOID    WscProfileRetryTimeout(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);

VOID    WscPBCTimeOutAction(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

VOID    WscScanTimeOutAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);


INT WscGenerateUUID(
	RTMP_ADAPTER	*pAd, 
	UCHAR 			*uuidHexStr, 
	UCHAR 			*uuidAscStr, 
	int 			apIdx,
	BOOLEAN			bUseCurrentTime);
	
VOID WscStop(
	IN	PRTMP_ADAPTER	pAd,
#ifdef CONFIG_AP_SUPPORT
    IN  BOOLEAN         bFromApcli,
#endif // CONFIG_AP_SUPPORT //
	IN  PWSC_CTRL       pWscControl);

VOID WscInit(
	IN	PRTMP_ADAPTER	pAd,
    IN  BOOLEAN         bFromApcli,	
	IN  UCHAR       	BssIndex);

BOOLEAN
ValidateChecksum(
	IN UINT PIN);

UINT WscRandomGeneratePinCode(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	apidx);

int BuildMessageM1(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM2(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM2D(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM3(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM4(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM5(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM6(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM7(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageM8(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageDONE(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageACK(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int BuildMessageNACK(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	OUT	VOID *pbuf);

int ProcessMessageM1(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM2(
	IN	PRTMP_ADAPTER		pAdapter,
	IN	VOID *precv,
	IN	INT Length,
	IN  UCHAR			apidx,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM2D(
	IN	PRTMP_ADAPTER		pAdapter,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM3(
	IN	PRTMP_ADAPTER		pAdapter,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM4(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM5(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM6(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM7(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL           pWscControl,
	IN	VOID *precv,
	IN	INT Length,
	OUT	PWSC_REG_DATA pReg);

int ProcessMessageM8(
	IN	PRTMP_ADAPTER		pAdapter,
	IN	VOID *precv,
	IN	INT Length,
	IN  PWSC_CTRL       pWscControl);

USHORT  WscGetAuthType(
    IN  NDIS_802_11_AUTHENTICATION_MODE authType);

USHORT  WscGetEncryType(
    IN  NDIS_802_11_WEP_STATUS encryType);

NDIS_STATUS WscThreadInit(
	IN RTMP_ADAPTER *pAd);

BOOLEAN WscThreadExit(
	IN RTMP_ADAPTER *pAd);

int     AppendWSCTLV(
    IN  USHORT index, 
    OUT UCHAR * obuf, 
    IN  UCHAR * ibuf, 
    IN  USHORT varlen);

VOID    WscGetRegDataPIN(
    IN  PRTMP_ADAPTER   pAd,
    IN  UINT            PinCode,
    IN  PWSC_CTRL       pWscControl);

VOID    WscPushPBCAction(
	IN	PRTMP_ADAPTER	pAd);

VOID    WscScanExec(
	IN	PRTMP_ADAPTER	pAd);

BOOLEAN WscPBCExec(
	IN	PRTMP_ADAPTER	pAd,
	IN  BOOLEAN			bFromM2);

VOID    WscPBCBssTableSort(
	IN	PRTMP_ADAPTER	pAd);

VOID	WscCreateProfileFromCfg(
	IN	PRTMP_ADAPTER		pAd,
	IN  UCHAR               OpMode,         // 0: AP Mode, 1: AP Client Mode, 2: STA Mode
	IN  PWSC_CTRL           pWscControl,
	OUT PWSC_PROFILE        pWscProfile); 

void    WscWriteConfToPortCfg(
    IN  PRTMP_ADAPTER   pAd,
    IN  PWSC_CTRL       pWscControl,
    IN  BOOLEAN         bEnrollee);

VOID   WpsSmProcess(
    IN PRTMP_ADAPTER        pAd,
    IN MLME_QUEUE_ELEM 	   *Elem);

VOID WscPBCSessionOverlapCheck(
	IN	PRTMP_ADAPTER	pAd);

VOID WscPBC_DPID_FromSTA(
	IN	PRTMP_ADAPTER		pAd,
	IN	PUCHAR				pMacAddr);

#ifdef CONFIG_AP_SUPPORT
INT	WscGetConfWithoutTrigger(
	IN	PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL       pWscControl,
	IN  BOOLEAN         bFromUPnP);

BOOLEAN	WscReadProfileFromUfdFile(
	IN	PRTMP_ADAPTER		pAd,
	IN  UCHAR               ApIdx,
	IN  PSTRING				pUfdFileName);

BOOLEAN	WscWriteProfileToUfdFile(
	IN	PRTMP_ADAPTER		pAd,
	IN  UCHAR               ApIdx,
	IN  PSTRING				pUfdFileName);
#endif // CONFIG_AP_SUPPORT //

VOID WscCheckWpsIeFromWpsPeer(
    IN  PRTMP_ADAPTER 	pAd, 
    IN  PEID_STRUCT		pEid,
    OUT PUSHORT			pDPIDFromAP);


/* WSC hardware push button function 0811 */
VOID WSC_HDR_BTN_Init(
	IN	PRTMP_ADAPTER	pAd);

VOID WSC_HDR_BTN_Stop(
	IN	PRTMP_ADAPTER	pAd);
 
VOID WSC_HDR_BTN_CheckHandler(
	IN	PRTMP_ADAPTER	pAd);
#ifdef WSC_LED_SUPPORT
BOOLEAN WscSupportWPSLEDMode(
	IN PRTMP_ADAPTER pAdapter);

BOOLEAN WscSupportWPSLEDMode10(
	IN PRTMP_ADAPTER pAdapter);

BOOLEAN WscAPHasSecuritySetting(
	IN PRTMP_ADAPTER pAdapter,
	IN PWSC_CTRL     pWscControl);

VOID WscLEDTimer(
	IN PVOID	SystemSpecific1, 
	IN PVOID	FunctionContext, 
	IN PVOID	SystemSpecific2, 
	IN PVOID	SystemSpecific3);

VOID WscSkipTurnOffLEDTimer(
	IN PVOID	SystemSpecific1, 
	IN PVOID	FunctionContext, 
	IN PVOID	SystemSpecific2, 
	IN PVOID	SystemSpecific3);
#endif // WSC_LED_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
VOID WscUpdatePortCfgTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WscSetupLockTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WscPinAttackCountCheckTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif // CONFIG_AP_SUPPORT //

#ifdef WSC_V2_SUPPORT
#ifdef CONFIG_AP_SUPPORT
VOID 	WscOnOff(
	IN  PRTMP_ADAPTER	pAd,
	IN  INT				ap_idx,
	IN  BOOLEAN			bOff);
#endif // CONFIG_AP_SUPPORT //

BOOLEAN	WscGenV2Msg(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	UCHAR			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen);

BOOLEAN	WscParseV2Msg(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	USHORT			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen);
#endif // WSC_V2_SUPPORT //

PWSC_PEER_ENTRY	WscFindPeerEntry(
	IN  PRTMP_ADAPTER	pAd,
	IN  UCHAR			ApIdx,
	IN	PUCHAR			pMacAddr);

VOID	WscClearPeerList(
	IN  PRTMP_ADAPTER	pAd,
	IN  UCHAR			ApIdx);

VOID	WscShowPeerList(
	IN  PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

VOID	WscMaintainPeerList(
	IN  PRTMP_ADAPTER	pAd,
	IN  UCHAR			ApIdx);

VOID	WscAssignEntryMAC(
	IN  PRTMP_ADAPTER	pAd,
	IN	UCHAR			ap_idx);

VOID	WscInsertPeerEntryByMAC(
	PLIST_HEADER		pWscEnList,
	IN	PUCHAR			pMacAddr);

BOOLEAN WscGetDataFromPeerByTag(
    IN  PRTMP_ADAPTER 	pAd, 
    IN  PEID_STRUCT		pEid,
    IN  USHORT			WscTag,
    OUT PUCHAR			pWscBuf,
    OUT PUSHORT			pWscBufLen);

#endif // WSC_INCLUDED //



BOOLEAN rtstrmactohex(
	IN PSTRING s1,
	IN PSTRING s2);

BOOLEAN rtstrcasecmp(
	IN PSTRING s1,
	IN PSTRING s2);

PSTRING rtstrstruncasecmp(
	IN PSTRING s1,
	IN PSTRING s2);

PSTRING rtstrstr(
	IN	const PSTRING s1,
	IN	const PSTRING s2);

PSTRING rstrtok(
	IN PSTRING s,
	IN const PSTRING ct);
	
int rtinet_aton(
	const PSTRING cp, 
	unsigned int *addr);
	
////////// common ioctl functions //////////
INT Set_DriverVersion_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_MBSS_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
INT Set_BbpResetFlagCountVale(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */

INT	Set_ShortSlot_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_BGProtection_Proc(
	IN  PRTMP_ADAPTER		pAd, 
	IN  PSTRING			arg);

INT Set_TxPreamble_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

INT Set_RTSThreshold_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

INT Set_FragThreshold_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

INT Set_TxBurst_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

#ifdef RTMP_MAC_PCI
INT Set_ShowRF_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);
#endif // RTMP_MAC_PCI //

#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);
#endif // AGGREGATION_SUPPORT //

#ifdef INF_PPA_SUPPORT
INT	Set_INF_AMAZON_SE_PPA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg);

#endif // INF_PPA_SUPPORT //

INT	Set_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef DBG
INT	Set_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif


INT	Show_DescInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ResetStatCounter_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_BADecline_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_BAOriTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_BARecTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtStbc_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtHtc_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtLinkAdapt_Proc(																																																																																																																																																																																																																																									
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtAmsdu_Proc(																																																																																																																																																																																																																																																																																																																			
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
	
INT	Set_HtAutoBa_Proc(																																																																																																																																																																																																																																																																																																																			
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
					
INT	Set_HtProtect_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtMimoPs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef DOT11N_DRAFT3
INT Set_HT_BssCoex_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam);

INT Set_HT_BssCoexApCntThr_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam);
#endif // DOT11N_DRAFT3 //


#ifdef CONFIG_AP_SUPPORT
INT	Set_HtTxStream_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtRxStream_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
INT	Set_GreenAP_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // GREENAP_SUPPORT //
#endif // DOT11_N_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

INT	Set_ForceShortGI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ForceGF_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	SetCommonHT(
	IN	PRTMP_ADAPTER	pAd);

INT	Set_SendPSMPAction_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtMIMOPSmode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);


INT	Set_HtTxBASize_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtDisallowTKIP_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#endif // DOT11_N_SUPPORT //

#ifdef APCLI_SUPPORT
INT RTMPIoctlConnStatus(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

#endif //APCLI_SUPPORT//





#ifdef CONFIG_AP_SUPPORT
VOID detect_wmm_traffic(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR			UserPriority,
	IN	UCHAR			FlgIsOutput);

VOID dynamic_tune_be_tx_op(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG			nonBEpackets);
#endif // CONFIG_AP_SUPPORT //


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID Handle_BSS_Width_Trigger_Events(
	IN PRTMP_ADAPTER pAd);
#endif // DOT11N_DRAFT3 //

void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE);
#endif // DOT11_N_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
void SendSignalToDaemon(
	IN INT              sig,
	RTMP_OS_PID pid,
	unsigned long pid_no);
#endif // CONFIG_AP_SUPPORT //

BOOLEAN APRxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd);

BOOLEAN STARxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd,
	IN	BOOLEAN			argc);

#ifdef DOT11_N_SUPPORT
// AMPDU packet indication
VOID Indicate_AMPDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

// AMSDU packet indication
VOID Indicate_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);
#endif // DOT11_N_SUPPORT //

// Normal legacy Rx packet indication
VOID Indicate_Legacy_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID Indicate_EAPOL_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

void  update_os_packet_info(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

void wlan_802_11_to_802_3_packet(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk,
	IN	PUCHAR			pHeader802_3,
	IN  UCHAR			FromWhichBSSID,
	IN	UCHAR			*TPID);

UINT deaggregate_AMSDU_announce(
	IN	PRTMP_ADAPTER	pAd,
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize);


#ifdef CONFIG_AP_SUPPORT
// remove LLC and get 802_3 Header
#define  RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)	\
{																						\
	PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;                                 						\
																				\
																				\
	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH)) 		\
	{                                                                           											\
		_pDA = _pRxBlk->pHeader->Addr3;                                         							\
		_pSA = (PUCHAR)_pRxBlk->pHeader + sizeof(HEADER_802_11);                					\
	}                                                                           											\
	else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_APCLI))										\
	{																					\
		_pDA = _pRxBlk->pHeader->Addr1; 													\
		_pSA = _pRxBlk->pHeader->Addr3;													\
	}																					\
	else                                                                        										\
	{                                                                           											\
		_pDA = _pRxBlk->pHeader->Addr3;                                         							\
		_pSA = _pRxBlk->pHeader->Addr2;                                         							\
	}                                                                           											\
																				\
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData, 						\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);                                   						\
}
#endif // CONFIG_AP_SUPPORT //



BOOLEAN APFowardWirelessStaToWirelessSta(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	ULONG			FromWhichBSSID);

VOID Announce_or_Forward_802_3_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID);

VOID Sta_Announce_or_Forward_802_3_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID);

#ifdef CONFIG_AP_SUPPORT
#define AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
			Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
#endif // CONFIG_AP_SUPPORT //



PNDIS_PACKET DuplicatePacket(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID);


PNDIS_PACKET ClonePacket(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket,	
	IN	PUCHAR			pData,
	IN	ULONG			DataSize);


// Normal, AMPDU or AMSDU
VOID CmmRxnonRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID CmmRxRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID Update_Rssi_Sample(
	IN PRTMP_ADAPTER	pAd,
	IN RSSI_SAMPLE		*pRssi,
	IN PRXWI_STRUC		pRxWI);

PNDIS_PACKET GetPacketFromRxRing(
	IN		PRTMP_ADAPTER	pAd,
	OUT		PRT28XX_RXD_STRUC		pSaveRxD,
	OUT		BOOLEAN			*pbReschedule,
	IN OUT	UINT32			*pRxPending);

PNDIS_PACKET RTMPDeFragmentDataFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk);

////////////////////////////////////////

#if defined (AP_SCAN_SUPPORT) || defined (CONFIG_STA_SUPPORT)
VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	struct iwreq	*wrq);
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
INT Set_ApCli_AuthMode_Proc(
    IN PRTMP_ADAPTER pAd, 
    IN PSTRING arg);

INT Set_ApCli_EncrypType_Proc(
    IN PRTMP_ADAPTER pAd, 
    IN PSTRING arg);
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#ifdef MAT_SUPPORT

VOID getIPv6MacTbInfo(MAT_STRUCT *, char *);

VOID getIPMacTbInfo(
	IN MAT_STRUCT *pMatCfg, 
	IN char *pOutBuf);

NDIS_STATUS MATEngineInit(
	IN RTMP_ADAPTER *pAd);

NDIS_STATUS MATEngineExit(
	IN RTMP_ADAPTER *pAd);

PUCHAR MATEngineRxHandle(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET		pPkt,
	IN UINT				infIdx);


PUCHAR MATEngineTxHandle(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET     pPkt,
	IN UINT				infIdx);

BOOLEAN MATPktRxNeedConvert(
	IN PRTMP_ADAPTER	pAd, 
	IN PNET_DEV			net_dev);

#endif // MAT_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
typedef struct CountryCodeToCountryRegion {
	USHORT		CountryNum;
	UCHAR		IsoName[3];
	//UCHAR		CountryName[40];
	PSTRING		pCountryName;
	BOOLEAN		SupportABand;
	//ULONG		RegDomainNum11A;
	UCHAR		RegDomainNum11A;
	BOOLEAN  	SupportGBand;
	//ULONG		RegDomainNum11G;
	UCHAR		RegDomainNum11G;
} COUNTRY_CODE_TO_COUNTRY_REGION;
#endif // CONFIG_AP_SUPPORT //

#ifdef SNMP_SUPPORT
//for snmp , kathy
typedef struct _DefaultKeyIdxValue
{
	UCHAR	KeyIdx;
	UCHAR	Value[16];
} DefaultKeyIdxValue, *PDefaultKeyIdxValue;
#endif



INT	Set_FixedTxMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // CONFIG_APSTA_MIXED_SUPPORT //

INT Set_LongRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Set_ShortRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Set_AutoFallBack_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);



VOID RT28XXDMADisable(
	IN RTMP_ADAPTER 		*pAd);

VOID RT28XXDMAEnable(
	IN RTMP_ADAPTER 		*pAd);
	
VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER * pAd, 
	IN INT apidx,
	IN ULONG BeaconLen,
	IN ULONG UpdatePos);

void CfgInitHook(PRTMP_ADAPTER pAd);

int rt28xx_init(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING pDefaultMac, 
	IN PSTRING pHostName);

NDIS_STATUS RtmpNetTaskInit(
	IN RTMP_ADAPTER *pAd);

VOID RtmpNetTaskExit(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS RtmpMgmtTaskInit(
	IN RTMP_ADAPTER *pAd);
	
VOID RtmpMgmtTaskExit(
	IN RTMP_ADAPTER *pAd);

#ifdef WORKQUEUE_BH
void tbtt_workq(struct work_struct *work);
#else
void tbtt_tasklet(unsigned long data);
#endif // WORKQUEUE_BH //


PNET_DEV RtmpPhyNetDevInit(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_OS_NETDEV_OP_HOOK *pNetHook);

BOOLEAN RtmpPhyNetDevExit(
	IN RTMP_ADAPTER *pAd, 
	IN PNET_DEV net_dev);
	
INT RtmpRaDevCtrlInit(
	IN RTMP_ADAPTER *pAd, 
	IN RTMP_INF_TYPE infType);

BOOLEAN RtmpRaDevCtrlExit(
	IN RTMP_ADAPTER *pAd);

#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
VOID NICRestoreBBPValue(
	IN PRTMP_ADAPTER 	pAd,
	IN REG_PAIR 			*RegPair);
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */
	
#ifdef RTMP_MAC_PCI
//
// Function Prototype in cmm_data_pci.c
//
USHORT RtmpPCI_WriteSingleTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber);
	
USHORT RtmpPCI_WriteMultiTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			frameNum,
	OUT	USHORT			*FreeNumber);

USHORT	RtmpPCI_WriteFragTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			fragNum,
	OUT	USHORT			*FreeNumber);
	
VOID RtmpPCI_FinalWriteTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	USHORT			totalMPDUSize,
	IN	USHORT			FirstTxIdx);

int RtmpPCIMgmtKickOut(
	IN RTMP_ADAPTER 	*pAd, 
	IN UCHAR 			QueIdx,
	IN PNDIS_PACKET		pPacket,
	IN PUCHAR			pSrcBufVA,
	IN UINT 			SrcBufLen);


NDIS_STATUS RTMPCheckRxError(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PHEADER_802_11  pHeader,
	IN	PRXWI_STRUC	pRxWI,
	IN  PRT28XX_RXD_STRUC      pRxD);

BOOLEAN RT28xxPciAsicRadioOff(
	IN PRTMP_ADAPTER    pAd,
	IN UCHAR            Level, 
	IN USHORT           TbttNumToNextWakeUp);

BOOLEAN RT28xxPciAsicRadioOn(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR     Level);


VOID RT28xxPciMlmeRadioOn(
	IN PRTMP_ADAPTER pAd);

VOID RT28xxPciMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd);
#endif // RTMP_MAC_PCI //

VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER    pAd, 
	IN	UCHAR           Channel);



#ifdef RTMP_TIMER_TASK_SUPPORT
INT RtmpTimerQThread(
	IN ULONG Context);

RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(
	IN RTMP_ADAPTER *pAd, 
	IN RALINK_TIMER_STRUCT *pTimer);

BOOLEAN RtmpTimerQRemove(
	IN RTMP_ADAPTER *pAd, 
	IN RALINK_TIMER_STRUCT *pTimer);

void RtmpTimerQExit(
	IN RTMP_ADAPTER *pAd);

void RtmpTimerQInit(
	IN RTMP_ADAPTER *pAd);
#endif // RTMP_TIMER_TASK_SUPPORT //



////////////////////////////////////////

#ifdef AP_QLOAD_SUPPORT
VOID QBSS_LoadInit(
 	IN		RTMP_ADAPTER	*pAd);

VOID QBSS_LoadAlarmReset(
 	IN		RTMP_ADAPTER	*pAd);

VOID QBSS_LoadAlarmResume(
 	IN		RTMP_ADAPTER	*pAd);

UINT32 QBSS_LoadBusyTimeGet(
 	IN		RTMP_ADAPTER	*pAd);

BOOLEAN QBSS_LoadIsAlarmIssued(
 	IN		RTMP_ADAPTER	*pAd);

BOOLEAN QBSS_LoadIsBusyTimeAccepted(
 	IN		RTMP_ADAPTER	*pAd,
	IN		UINT32			BusyTime);

UINT32 QBSS_LoadElementAppend(
 	IN		RTMP_ADAPTER	*pAd,
	OUT		UINT8			*buf_p);

UINT32 QBSS_LoadElementParse(
 	IN		RTMP_ADAPTER	*pAd,
	IN		UINT8			*pElement,
	OUT		UINT16			*pStationCount,
	OUT		UINT8			*pChanUtil,
	OUT		UINT16			*pAvalAdmCap);

VOID QBSS_LoadUpdate(
 	IN		RTMP_ADAPTER	*pAd,
	IN		ULONG			UpTime);

VOID QBSS_LoadStatusClear(
 	IN		RTMP_ADAPTER	*pAd);

INT	Show_QoSLoad_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif // AP_QLOAD_SUPPORT //

///////////////////////////////////////
INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			pName,
	IN	PSTRING			pBuf);

PSTRING RTMPGetRalinkAuthModeStr(
    IN  NDIS_802_11_AUTHENTICATION_MODE authMode);

PSTRING RTMPGetRalinkEncryModeStr(
    IN  USHORT encryMode);
//////////////////////////////////////


void RTMP_IndicateMediaState(	
	IN	PRTMP_ADAPTER		pAd,
	IN  NDIS_MEDIA_STATE	media_state);

VOID ReSyncBeaconTime(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BandWidth);


int rt28xx_close(IN PNET_DEV dev);
int rt28xx_open(IN PNET_DEV dev);


#define VIRTUAL_IF_INC(__pAd) ((__pAd)->VirtualIfCnt++)
#define VIRTUAL_IF_DEC(__pAd) ((__pAd)->VirtualIfCnt--)
#define VIRTUAL_IF_NUM(__pAd) ((__pAd)->VirtualIfCnt)



#ifdef LINUX
__inline INT VIRTUAL_IF_UP(PRTMP_ADAPTER pAd)
{
	if (VIRTUAL_IF_NUM(pAd) == 0)
	{
		if (rt28xx_open(pAd->net_dev) != 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_open return fail!\n"));
			return -1;
		}
	}
	else
	{
#ifdef CONFIG_AP_SUPPORT
		extern VOID APMakeAllBssBeacon(IN PRTMP_ADAPTER pAd);
		extern VOID  APUpdateAllBeaconFrame(IN PRTMP_ADAPTER pAd);
		APMakeAllBssBeacon(pAd);
		APUpdateAllBeaconFrame(pAd);
#endif // CONFIG_AP_SUPPORT //
	}
	VIRTUAL_IF_INC(pAd);
	return 0;
}

__inline VOID VIRTUAL_IF_DOWN(PRTMP_ADAPTER pAd)
{
	VIRTUAL_IF_DEC(pAd);
	if (VIRTUAL_IF_NUM(pAd) == 0)
		rt28xx_close(pAd->net_dev);
	return;
}
#endif // LINUX //




#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(
	IN  PRTMP_ADAPTER   pAd,
	IN	PTX_BLK			pTxBlk);

VOID RTMPUpdateSwCacheCipherInfo(	
	IN  PRTMP_ADAPTER   pAd,
	IN	PTX_BLK			pTxBlk,
	IN	PUCHAR			pHdr);
#endif // SOFT_ENCRYPT //


/*
	OS Related funciton prototype definitions.
	TODO: Maybe we need to move these function prototypes to other proper place.
*/
int RtmpOSWrielessEventSend(
	IN RTMP_ADAPTER *pAd,
	IN UINT32		eventType,
	IN INT			flags,
	IN PUCHAR		pSrcMac,
	IN PUCHAR		pData,
	IN UINT32		dataLen);

int RtmpOSNetDevAddrSet(
	IN PNET_DEV pNetDev,
	IN PUCHAR	pMacAddr);
	
int RtmpOSNetDevAttach(
	IN PNET_DEV pNetDev, 
	IN RTMP_OS_NETDEV_OP_HOOK *pDevOpHook);

void RtmpOSNetDevClose(
	IN PNET_DEV pNetDev);

void RtmpOSNetDevDetach(
	IN PNET_DEV pNetDev);

INT RtmpOSNetDevAlloc(
	IN PNET_DEV *pNewNetDev,
	IN UINT32	privDataSize);

#ifdef LINUX
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
INT RtmpOSNetDevOpsAlloc(
	IN PVOID *pNetDevOps);
#endif
#endif // LINUX //

void RtmpOSNetDevFree(
	IN PNET_DEV pNetDev);
	
PNET_DEV RtmpOSNetDevGetByName(
	IN PNET_DEV pNetDev, 
	IN PSTRING	pDevName);

void RtmpOSNetDeviceRefPut(
	IN PNET_DEV pNetDev);

PNET_DEV RtmpOSNetDevCreate(
	IN RTMP_ADAPTER *pAd,
	IN INT 			devType, 
	IN INT			devNum,
	IN INT			privMemSize,
	IN PSTRING		pNamePrefix);

/*
	Task operation related function prototypes
*/
void RtmpOSTaskCustomize(
	IN RTMP_OS_TASK *pTask);

INT RtmpOSTaskNotifyToExit(
	IN RTMP_OS_TASK *pTask);

NDIS_STATUS RtmpOSTaskKill(
	IN RTMP_OS_TASK *pTask);

NDIS_STATUS RtmpOSTaskInit(
	IN RTMP_OS_TASK *pTask,
	PSTRING			 pTaskName,
	VOID			 *pPriv);
	
NDIS_STATUS RtmpOSTaskAttach(
	IN RTMP_OS_TASK *pTask,
	IN RTMP_OS_TASK_CALLBACK taskEntryPoint, 
	IN ULONG arg);


/*
	File operation related function prototypes
*/
RTMP_OS_FD RtmpOSFileOpen(
	IN char *pPath,  
	IN int flag, 
	IN int mode);

int RtmpOSFileClose(
	IN RTMP_OS_FD osfd);

void RtmpOSFileSeek(
	IN RTMP_OS_FD osfd, 
	IN int offset);

int RtmpOSFileRead(
	IN RTMP_OS_FD osfd, 
	IN char *pDataPtr, 
	IN int readLen);

int RtmpOSFileWrite(
	IN RTMP_OS_FD osfd, 
	IN char *pDataPtr, 
	IN int writeLen);

void RtmpOSFSInfoChange(
	IN RTMP_OS_FS_INFO *pOSFSInfo, 
	IN BOOLEAN bSet);


VOID	RTInitializeCmdQ(
	IN	PCmdQ	cmdq);

INT RTPCICmdThread(
	IN VOID *Context);

VOID CMDHandler(
    IN PRTMP_ADAPTER pAd);

VOID	RTThreadDequeueCmd(
	IN	PCmdQ		cmdq,
	OUT	PCmdQElmt	*pcmdqelmt);

NDIS_STATUS RTEnqueueInternalCmd(
	IN PRTMP_ADAPTER	pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength);

#ifdef HOSTAPD_SUPPORT
VOID ieee80211_notify_michael_failure(
	IN	PRTMP_ADAPTER    pAd,
	IN	PHEADER_802_11   pHeader,
	IN	UINT            keyix,
	IN	INT              report);

const CHAR* ether_sprintf(const UINT8 *mac);
#endif//HOSTAPD_SUPPORT//

#ifdef VENDOR_FEATURE3_SUPPORT
VOID RTMP_IO_WRITE32(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT32 Value);
#endif // VENDOR_FEATURE3_SUPPORT //

VOID RTMP_BBP_IO_READ8_BY_REG_ID(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT8 *pValue);

VOID RTMP_BBP_IO_READ8(
	PRTMP_ADAPTER pAd,
	UCHAR Offset,
	UINT8 *pValue,
	BOOLEAN FlgValidMCR);

VOID RTMP_BBP_IO_WRITE8_BY_REG_ID(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT8 Value);

VOID RTMP_BBP_IO_WRITE8(
	PRTMP_ADAPTER pAd,
	UCHAR Offset,
	UINT8 Value,
	BOOLEAN FlgValidMCR);

BOOLEAN CHAN_PropertyCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UINT32			ChanNum,
	IN UCHAR			Property);


INT     Set_Antenna_Proc(
        IN      PRTMP_ADAPTER   pAd,
        IN      PSTRING                 arg);

void  getRate(
    IN HTTRANSMIT_SETTING HTSetting, 
    OUT ULONG* fLastTxRxRate);


#if defined(RT3350) || defined(RT33xx)
VOID RTMP_TxEvmCalibration(
	IN PRTMP_ADAPTER pAd);
#endif // defined(RT3350) || defined(RT33xx) //

#ifdef IQ_CAL_SUPPORT
VOID GetIQCalibration(
	IN struct _RTMP_ADAPTER *pAd);

UCHAR IQCal(
	IN enum IQ_CAL_CHANNEL_INDEX 	ChannelIndex,
	IN enum IQ_CAL_TXRX_CHAIN 		TxRxChain,
	IN enum IQ_CAL_TYPE 				IQCalType);

VOID IQCalibration(
	IN struct _RTMP_ADAPTER		*pAd,
	IN UCHAR 					Channel);

VOID IQCalibrationViaBBPAccessSpace(
	IN PRTMP_ADAPTER 			pAd,
	IN UCHAR 					Channel);
#endif /* IQ_CAL_SUPPORT */
#endif  // __RTMP_H__

