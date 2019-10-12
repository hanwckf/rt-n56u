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
#endif /* CONFIG_AP_SUPPORT */

#include "wsc.h"
#ifdef MAT_SUPPORT
#include "mat.h"
#endif /* MAT_SUPPORT */


#ifdef WAPI_SUPPORT
#include "wapi_def.h"
#endif /* WAPI_SUPPORT */

#include "rtmp_chip.h"




#ifdef DOT11Z_TDLS_SUPPORT
#include "tdls_cmm.h"
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef CLIENT_WDS
#include "client_wds_cmm.h"
#endif /* CLIENT_WDS */



#ifdef P2P_SUPPORT
#include "p2p_cmm.h"
#endif /* P2P_SUPPORT */

#include "drs_extr.h"

struct _RTMP_TX_RATE_SWITCH;

typedef struct _RTMP_ADAPTER RTMP_ADAPTER;
typedef struct _RTMP_ADAPTER *PRTMP_ADAPTER;

typedef struct _RTMP_CHIP_OP_ RTMP_CHIP_OP;
typedef struct _RTMP_CHIP_CAP_ RTMP_CHIP_CAP;

typedef struct _UAPSD_INFO {
	BOOLEAN bAPSDCapable;
} UAPSD_INFO;

#include "radar.h"

#ifdef CARRIER_DETECTION_SUPPORT
#include "cs.h"
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DFS_SUPPORT
#include "dfs.h"
#endif /* DFS_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
#include "rt_led.h"
#endif /* LED_CONTROL_SUPPORT */


#ifdef RALINK_ATE
#include "rt_ate.h"
#endif /* RALINK_ATE */

/*#define DBG		1 */

/*#define DBG_DIAGNOSE		1 */


/*+++Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
#define MAX_DATAMM_RETRY	3
#define MGMT_USE_QUEUE_FLAG	0x80
/*---Used for merge MiniportMMRequest() and MiniportDataMMRequest() into one function */
/* The number of channels for per-channel Tx power offset */


#define	MAXSEQ		(0xFFF)

#ifdef DOT11N_SS3_SUPPORT
#define MAX_MCS_SET 24		/* From MCS 0 ~ MCS 23 */
#else
#define MAX_MCS_SET 16		/* From MCS 0 ~ MCS 15 */
#endif /* DOT11N_SS3_SUPPORT */


#define MAX_TXPOWER_ARRAY_SIZE	5
#ifdef RT2883
#undef MAX_TXPOWER_ARRAY_SIZE
#define MAX_TXPOWER_ARRAY_SIZE	7
#endif /* RT2883 */
#ifdef RT3883
#undef MAX_TXPOWER_ARRAY_SIZE
#define MAX_TXPOWER_ARRAY_SIZE	10
#endif /* RT3883 */

extern unsigned char CISCO_OUI[];
extern UCHAR BaSizeArray[4];

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern ULONG BIT32[32];
extern char *CipherName[];
extern UCHAR RxwiMCSToOfdmRate[12];
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR TPID[];
extern UCHAR APPLE_TALK[2];
#ifdef DOT11Z_TDLS_SUPPORT
extern UCHAR TDLS_LLC_SNAP_WITH_CATEGORY[10];
extern UCHAR TDLS_ETHERTYPE[2];
#endif /* DOT11Z_TDLS_SUPPORT */
extern UCHAR OfdmRateToRxwiMCS[];
extern UCHAR MapUserPriorityToAccessCategory[8];

extern USHORT RateUpPER[];
extern USHORT RateDownPER[];
extern UCHAR Phy11BNextRateDownward[];
extern UCHAR Phy11BNextRateUpward[];
extern UCHAR Phy11BGNextRateDownward[];
extern UCHAR Phy11BGNextRateUpward[];
extern UCHAR Phy11ANextRateDownward[];
extern UCHAR Phy11ANextRateUpward[];
extern unsigned char RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR CipherSuiteWpaNoneTkip[];
extern UCHAR CipherSuiteWpaNoneTkipLen;

extern UCHAR CipherSuiteWpaNoneAes[];
extern UCHAR CipherSuiteWpaNoneAesLen;

extern UCHAR SsidIe;
extern UCHAR SupRateIe;
extern UCHAR ExtRateIe;

#ifdef DOT11_N_SUPPORT
extern UCHAR HtCapIe;
extern UCHAR AddHtInfoIe;
extern UCHAR NewExtChanIe;
extern UCHAR BssCoexistIe;
extern UCHAR ExtHtCapIe;
#ifdef CONFIG_STA_SUPPORT
extern UCHAR PRE_N_HT_OUI[];
#endif /* CONFIG_STA_SUPPORT */
#endif /* DOT11_N_SUPPORT */
extern UCHAR ExtCapIe;

extern UCHAR ErpIe;
extern UCHAR DsIe;
extern UCHAR TimIe;
extern UCHAR WpaIe;
extern UCHAR Wpa2Ie;
extern UCHAR IbssIe;
extern UCHAR WapiIe;

extern UCHAR WPA_OUI[];
extern UCHAR RSN_OUI[];
extern UCHAR WAPI_OUI[];
extern UCHAR WME_INFO_ELEM[];
extern UCHAR WME_PARM_ELEM[];
extern UCHAR RALINK_OUI[];
extern UCHAR PowerConstraintIE[];

typedef	struct {
	LONG	i:8;
	LONG	q:8;
}	COMPLEX_VALUE;	/* Signed 8-bit complex values */

typedef union _CAPTURE_MODE_PACKET_BUFFER {
	struct 
	{
		UINT32       BYTE0:8;     
		UINT32       BYTE1:8;
		UINT32       BYTE2:8;
		UINT32       BYTE3:8;
	} field;
	UINT32                   Value;
}CAPTURE_MODE_PACKET_BUFFER, *PCAPTURE_MODE_PACKET_BUFFER;

typedef struct _RSSI_SAMPLE {
	CHAR LastRssi0;		/* last received RSSI */
	CHAR LastRssi1;		/* last received RSSI */
	CHAR LastRssi2;		/* last received RSSI */
	CHAR AvgRssi0;
	CHAR AvgRssi1;
	CHAR AvgRssi2;
	SHORT AvgRssi0X8;
	SHORT AvgRssi1X8;
	SHORT AvgRssi2X8;
	CHAR LastSnr0;
	CHAR LastSnr1;
	CHAR LastSnr2;
	CHAR AvgSnr0;
	CHAR AvgSnr1;
	CHAR AvgSnr2;
	SHORT AvgSnr0X8;
	SHORT AvgSnr1X8;
	SHORT AvgSnr2X8;
	CHAR LastNoiseLevel0;
	CHAR LastNoiseLevel1;
	CHAR LastNoiseLevel2;
} RSSI_SAMPLE;

/* */
/*  Queue structure and macros */
/* */
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


/* */
/*  Macros for flag and ref count operations */
/* */
#define RTMP_SET_FLAG(_M, _F)       ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)     ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)        ((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)      (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)     (((_M)->Flags & (_F)) == (_F))
/* Macro for power save flag. */
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

#ifdef CONFIG_STA_SUPPORT
#define STA_NO_SECURITY_ON(_p)          (_p->StaCfg.WepStatus == Ndis802_11EncryptionDisabled)
#define STA_WEP_ON(_p)                  (_p->StaCfg.WepStatus == Ndis802_11Encryption1Enabled)
#define STA_TKIP_ON(_p)                 (_p->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
#define STA_AES_ON(_p)                  (_p->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)

#define STA_TGN_WIFI_ON(_p)             (_p->StaCfg.bTGnWifiTest == TRUE)
#endif /* CONFIG_STA_SUPPORT */

#define CKIP_KP_ON(_p)				((((_p)->StaCfg.CkipFlag) & 0x10) && ((_p)->StaCfg.bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg.CkipFlag) & 0x08) && ((_p)->StaCfg.bCkipCmicOn == TRUE))

#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}

#ifdef USB_BULK_BUF_ALIGMENT
#define CUR_WRITE_IDX_INC(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}
#endif /* USB_BULK_BUF_ALIGMENT */

#ifdef DOT11_N_SUPPORT
/* StaActive.SupportedHtPhy.MCSSet is copied from AP beacon.  Don't need to update here. */
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
#endif /* DOT11_N_SUPPORT */

/* */
/* MACRO for 32-bit PCI register read / write */
/* */
/* Usage : RTMP_IO_READ32( */
/*              PRTMP_ADAPTER pAd, */
/*              ULONG Register_Offset, */
/*              PULONG  pValue) */
/* */
/*         RTMP_IO_WRITE32( */
/*              PRTMP_ADAPTER pAd, */
/*              ULONG Register_Offset, */
/*              ULONG Value) */
/* */

/* */
/* Common fragment list structure -  Identical to the scatter gather frag list structure */
/* */
/*#define RTMP_SCATTER_GATHER_ELEMENT         SCATTER_GATHER_ELEMENT */
/*#define PRTMP_SCATTER_GATHER_ELEMENT        PSCATTER_GATHER_ELEMENT */
#define NIC_MAX_PHYS_BUF_COUNT              8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
	PVOID Address;
	ULONG Length;
	PULONG Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;

typedef struct _RTMP_SCATTER_GATHER_LIST {
	ULONG NumberOfElements;
	PULONG Reserved;
	RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;

/* */
/*  Some utility macros */
/* */
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

/* Check LEAP & CCKM flags */
#define LEAP_ON(_p)                 (((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP)
#define LEAP_CCKM_ON(_p)            ((((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP) && ((_p)->StaCfg.LeapAuthInfo.CCKM == TRUE))

/* if orginal Ethernet frame contains no LLC/SNAP, then an extra LLC/SNAP encap is required */
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

/* New Define for new Tx Path. */
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

/* if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel), keep it that way. */
/* else if the received frame is LLC/SNAP-encaped IPX or APPLETALK, preserve the LLC/SNAP field */
/* else remove the LLC/SNAP field from the result Ethernet frame */
/* Patch for WHQL only, which did not turn on Netbios but use IPX within its payload */
/* Note: */
/*     _pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO */
/*     _pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed */
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

/* Enqueue this frame to MLME engine */
/* We need to enqueue the whole frame because MLME need to pass data type */
/* information from 802.11 header */
#ifdef RTMP_MAC_PCI
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _MinSNR, _OpMode)        \
{                                                                                       \
    UINT32 High32TSF, Low32TSF;                                                          \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW1, &High32TSF);                                       \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW0, &Low32TSF);                                        \
    MlmeEnqueueForRecv(_pAd, Wcid, High32TSF, Low32TSF, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2,_FrameSize, _pFrame, (UCHAR)_MinSNR, _OpMode);   \
}
#endif /* RTMP_MAC_PCI */

#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)    ((len1==len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))

/* */
/* Check if it is Japan W53(ch52,56,60,64) channel. */
/* */
#define JapanChannelCheck(channel)  ((channel == 52) || (channel == 56) || (channel == 60) || (channel == 64))

#ifdef CONFIG_STA_SUPPORT
#define STA_EXTRA_SETTING(_pAd)

#define STA_PORT_SECURED(_pAd) \
{ \
	BOOLEAN	Cancelled; \
	(_pAd)->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED; \
	RTMP_IndicateMediaState(_pAd, NdisMediaStateConnected); \
	NdisAcquireSpinLock(&((_pAd)->MacTabLock)); \
	(_pAd)->MacTab.Content[BSSID_WCID].PortSecured = (_pAd)->StaCfg.PortSecured; \
	(_pAd)->MacTab.Content[BSSID_WCID].PrivacyFilter = Ndis802_11PrivFilterAcceptAll;\
	NdisReleaseSpinLock(&(_pAd)->MacTabLock); \
	RTMPCancelTimer(&((_pAd)->Mlme.LinkDownTimer), &Cancelled);\
	STA_EXTRA_SETTING(_pAd); \
}
#endif /* CONFIG_STA_SUPPORT */

/* */
/*  Data buffer for DMA operation, the buffer must be contiguous physical memory */
/*  Both DMA to / from CPU use the same structure. */
/* */
typedef struct _RTMP_DMABUF {
	ULONG AllocSize;
	PVOID AllocVa;		/* TxBuf virtual address */
	NDIS_PHYSICAL_ADDRESS AllocPa;	/* TxBuf physical address */
} RTMP_DMABUF, *PRTMP_DMABUF;

/* */
/* Control block (Descriptor) for all ring descriptor DMA operation, buffer must be */
/* contiguous physical memory. NDIS_PACKET stored the binding Rx packet descriptor */
/* which won't be released, driver has to wait until upper layer return the packet */
/* before giveing up this rx ring descriptor to ASIC. NDIS_BUFFER is assocaited pair */
/* to describe the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor */
/* which driver should ACK upper layer when the tx is physically done or failed. */
/* */
typedef struct _RTMP_DMACB {
	ULONG AllocSize;	/* Control block size */
	PVOID AllocVa;		/* Control block virtual address */
	NDIS_PHYSICAL_ADDRESS AllocPa;	/* Control block physical address */
	PNDIS_PACKET pNdisPacket;
	PNDIS_PACKET pNextNdisPacket;

	RTMP_DMABUF DmaBuf;	/* Associated DMA buffer structure */
#ifdef CACHE_LINE_32B
	RXD_STRUC LastBDInfo;
#endif /* CACHE_LINE_32B */
} RTMP_DMACB, *PRTMP_DMACB;

typedef struct _RTMP_TX_RING {
	RTMP_DMACB Cell[TX_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
} RTMP_TX_RING, *PRTMP_TX_RING;

typedef struct _RTMP_RX_RING {
	RTMP_DMACB Cell[RX_RING_SIZE];
	UINT32 RxCpuIdx;
	UINT32 RxDmaIdx;
	INT32 RxSwReadIdx;	/* software next read index */
} RTMP_RX_RING, *PRTMP_RX_RING;

typedef struct _RTMP_MGMT_RING {
	RTMP_DMACB Cell[MGMT_RING_SIZE];
	UINT32 TxCpuIdx;
	UINT32 TxDmaIdx;
	UINT32 TxSwFreeIdx;	/* software next free tx index */
} RTMP_MGMT_RING, *PRTMP_MGMT_RING;


/* */
/*  Statistic counter structure */
/* */
typedef struct _COUNTER_802_3 {
	/* General Stats */
	ULONG GoodTransmits;
	ULONG GoodReceives;
	ULONG TxErrors;
	ULONG RxErrors;
	ULONG RxNoBuffer;

	/* Ethernet Stats */
	ULONG RcvAlignmentErrors;
	ULONG OneCollision;
	ULONG MoreCollisions;

} COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG Length;
/*	LARGE_INTEGER   LastTransmittedFragmentCount; */
	LARGE_INTEGER TransmittedFragmentCount;
	LARGE_INTEGER MulticastTransmittedFrameCount;
	LARGE_INTEGER FailedCount;
	LARGE_INTEGER RetryCount;
	LARGE_INTEGER MultipleRetryCount;
	LARGE_INTEGER RTSSuccessCount;
	LARGE_INTEGER RTSFailureCount;
	LARGE_INTEGER ACKFailureCount;
	LARGE_INTEGER FrameDuplicateCount;
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER MulticastReceivedFrameCount;
	LARGE_INTEGER FCSErrorCount;
	LARGE_INTEGER TransmittedFrameCount;
	LARGE_INTEGER WEPUndecryptableCount;
	LARGE_INTEGER TransmitCountFrmOs;
} COUNTER_802_11, *PCOUNTER_802_11;



typedef struct _COUNTER_RALINK {
	UINT32 OneSecStart;	/* for one sec count clear use */
	UINT32 OneSecBeaconSentCnt;
	UINT32 OneSecFalseCCACnt;	/* CCA error count, for debug purpose, might move to global counter */
	UINT32 OneSecRxFcsErrCnt;	/* CRC error */
	UINT32 OneSecRxOkCnt;	/* RX without error */
	UINT32 OneSecTxFailCount;
	UINT32 OneSecTxNoRetryOkCount;
	UINT32 OneSecTxRetryOkCount;
	UINT32 OneSecRxOkDataCnt;	/* unicast-to-me DATA frame count */
	UINT32 OneSecTransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */

	ULONG OneSecOsTxCount[NUM_OF_TX_RING];
	ULONG OneSecDmaDoneCount[NUM_OF_TX_RING];
	UINT32 OneSecTxDoneCount;
	ULONG OneSecRxCount;
	UINT32 OneSecReceivedByteCount;
	UINT32 OneSecTxAggregationCount;
	UINT32 OneSecRxAggregationCount;
	UINT32 OneSecEnd;	/* for one sec count clear use */

#ifdef MICROWAVE_OVEN_SUPPORT
	UINT16 FalseCCACnt_100MS[MLME_TASK_EXEC_MULTIPLE]; /* one handred millisecond false CCA Count */
	UINT16 PLCPErrCnt_100MS[MLME_TASK_EXEC_MULTIPLE]; /* one handred millisecond PLCP Error Count */
#endif /* MICROWAVE_OVEN_SUPPORT */

	ULONG TransmittedByteCount;	/* both successful and failure, used to calculate TX throughput */
	ULONG ReceivedByteCount;	/* both CRC okay and CRC error, used to calculate RX throughput */
	ULONG BadCQIAutoRecoveryCount;
	ULONG PoorCQIRoamingCount;
	ULONG MgmtRingFullCount;
	ULONG RxCountSinceLastNULL;
	ULONG RxCount;
	ULONG KickTxCount;
	LARGE_INTEGER RealFcsErrCount;
	ULONG PendingNdisPacketCount;
	ULONG FalseCCACnt;                    /* CCA error count */
	UINT32 OneSecFalseCCACnt1;

	UINT32 LastOneSecTotalTxCount;	/* OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount */
	UINT32 LastOneSecRxOkDataCnt;	/* OneSecRxOkDataCnt */
	ULONG DuplicateRcv;
	ULONG TxAggCount;
	ULONG TxNonAggCount;
	ULONG TxAgg1MPDUCount;
	ULONG TxAgg2MPDUCount;
	ULONG TxAgg3MPDUCount;
	ULONG TxAgg4MPDUCount;
	ULONG TxAgg5MPDUCount;
	ULONG TxAgg6MPDUCount;
	ULONG TxAgg7MPDUCount;
	ULONG TxAgg8MPDUCount;
	ULONG TxAgg9MPDUCount;
	ULONG TxAgg10MPDUCount;
	ULONG TxAgg11MPDUCount;
	ULONG TxAgg12MPDUCount;
	ULONG TxAgg13MPDUCount;
	ULONG TxAgg14MPDUCount;
	ULONG TxAgg15MPDUCount;
	ULONG TxAgg16MPDUCount;

	LARGE_INTEGER TransmittedOctetsInAMSDU;
	LARGE_INTEGER TransmittedAMSDUCount;
	LARGE_INTEGER ReceivedOctesInAMSDUCount;
	LARGE_INTEGER ReceivedAMSDUCount;
	LARGE_INTEGER TransmittedAMPDUCount;
	LARGE_INTEGER TransmittedMPDUsInAMPDUCount;
	LARGE_INTEGER TransmittedOctetsInAMPDUCount;
	LARGE_INTEGER MPDUInReceivedAMPDUCount;
} COUNTER_RALINK, *PCOUNTER_RALINK;

typedef struct _COUNTER_DRS {
	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT TxQuality[MAX_TX_RATE_INDEX+1];
	UCHAR PER[MAX_TX_RATE_INDEX+1];
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
	ULONG CurrTxRateStableTime;	/* # of second in current TX rate */
	/*BOOLEAN         fNoisyEnvironment; */
	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	UCHAR LastTimeTxRateChangeAction;	/*Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;


#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
typedef
    struct {
	ULONG TxSuccessCount;
	ULONG TxRetryCount;
	ULONG TxFailCount;
	ULONG ETxSuccessCount;
	ULONG ETxRetryCount;
	ULONG ETxFailCount;
	ULONG ITxSuccessCount;
	ULONG ITxRetryCount;
	ULONG ITxFailCount;
} COUNTER_TXBF;
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */


#ifdef STREAM_MODE_SUPPORT
typedef struct _STREAM_MODE_ENTRY_{
#define STREAM_MODE_STATIC		1
	USHORT flag;
	UCHAR macAddr[MAC_ADDR_LEN];
}STREAM_MODE_ENTRY;
#endif /* STREAM_MODE_SUPPORT */

#if defined(MICROWAVE_OVEN_SUPPORT) || defined(DYNAMIC_VGA_SUPPORT)
/* for Microwave oven */
typedef struct _MO_CFG_STRUCT {
	BOOLEAN		bEnable;
	BOOLEAN		bDyncVGAEnable;
	UINT8  		nPeriod_Cnt; 	/* measurement period 100ms, mitigate the interference period 900 ms */
	UINT16 		nFalseCCACnt;	
	UINT16		nFalseCCATh;	/* default is 100 */
	UINT16		nLowFalseCCATh;
	UINT32		Stored_BBP_R65;
	UCHAR		Stored_BBP_R66;
	UCHAR		Stored_RF_B5_R6;
	UCHAR		Stored_RF_B5_R7;
#ifdef RT6352
	UCHAR		Stored_RF_B7_R6;
	UCHAR		Stored_RF_B7_R7;
#endif /* RT6352 */
} MO_CFG_STRUCT, *PMO_CFG_STRUCT;
#endif /* defined(MICROWAVE_OVEN_SUPPORT) || defined(DYNAMIC_VGA_SUPPORT) */

/***************************************************************************
  *	security key related data structure
  **************************************************************************/

/* structure to define WPA Group Key Rekey Interval */
typedef struct GNU_PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;	/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG ReKeyInterval;	/* time-based: seconds, packet-based: kilo-packets */
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;



typedef struct {
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR ErrorCode[2];	/*00 01-Invalid authentication type */
	/*00 02-Authentication timeout */
	/*00 03-Challenge from AP failed */
	/*00 04-Challenge to AP failed */
	BOOLEAN Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UCHAR RogueApNr;
	ROGUEAP_ENTRY RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

/*
  *	Fragment Frame structure
  */
typedef struct _FRAGMENT_FRAME {
	PNDIS_PACKET pFragPacket;
	ULONG RxSize;
	USHORT Sequence;
	USHORT LastFrag;
	ULONG Flags;		/* Some extra frame information. bit 0: LLC presented */
} FRAGMENT_FRAME, *PFRAGMENT_FRAME;


/* */
/* Tkip Key structure which RC4 key & MIC calculation */
/* */
typedef struct _TKIP_KEY_INFO {
	UINT nBytesInM;		/* # bytes in M for MICKEY */
	ULONG IV16;
	ULONG IV32;
	ULONG K0;		/* for MICKEY Low */
	ULONG K1;		/* for MICKEY Hig */
	ULONG L;		/* Current state for MICKEY */
	ULONG R;		/* Current state for MICKEY */
	ULONG M;		/* Message accumulator for MICKEY */
	UCHAR RC4KEY[16];
	UCHAR MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;


/* */
/* Private / Misc data, counters for driver internal use */
/* */
typedef struct __PRIVATE_STRUC {
	UINT SystemResetCnt;	/* System reset counter */
	UINT TxRingFullCnt;	/* Tx ring full occurrance number */
	UINT PhyRxErrCnt;	/* PHY Rx error count, for debug purpose, might move to global counter */
	/* Variables for WEP encryption / decryption in rtmp_wep.c */
	/* Tkip stuff */
	TKIP_KEY_INFO Tx;
	TKIP_KEY_INFO Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;


/***************************************************************************
  *	Channel and BBP related data structures
  **************************************************************************/
/* structure to tune BBP R66 (BBP TUNING) */
typedef struct _BBP_R66_TUNING {
	BOOLEAN bEnable;
	USHORT FalseCcaLowerThreshold;	/* default 100 */
	USHORT FalseCcaUpperThreshold;	/* default 512 */
	UCHAR R66Delta;
	UCHAR R66CurrentValue;
	BOOLEAN R66LowerUpperSelect;	/*Before LinkUp, Used LowerBound or UpperBound as R66 value. */
} BBP_R66_TUNING, *PBBP_R66_TUNING;


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#define EFFECTED_CH_SECONDARY 0x1
#define EFFECTED_CH_PRIMARY	0x2
#define EFFECTED_CH_LEGACY		0x4
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/* structure to store channel TX power */
typedef struct _CHANNEL_TX_POWER {
	USHORT RemainingTimeForUse;	/*unit: sec */
	UCHAR Channel;
#ifdef DOT11N_DRAFT3
	BOOLEAN bEffectedChannel;	/* For BW 40 operating in 2.4GHz , the "effected channel" is the channel that is covered in 40Mhz. */
#endif /* DOT11N_DRAFT3 */
	CHAR Power;
	CHAR Power2;
#ifdef DOT11N_SS3_SUPPORT
	CHAR Power3;
#endif /* DOT11N_SS3_SUPPORT */
	UCHAR MaxTxPwr;
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;

/*
	Channel property:
 
	CHANNEL_DISABLED: The channel is disabled.
	CHANNEL_PASSIVE_SCAN: Only passive scanning is allowed.
	CHANNEL_NO_IBSS: IBSS is not allowed.
	CHANNEL_RADAR: Radar detection is required.
	CHANNEL_NO_FAT_ABOVE: Extension channel above this channel is not allowed.
	CHANNEL_NO_FAT_BELOW: Extension channel below this channel is not allowed.
	CHANNEL_40M_CAP: 40 BW channel group
 */
#define CHANNEL_DEFAULT_PROP	0x00
#define CHANNEL_DISABLED		0x01	/* no use */
#define CHANNEL_PASSIVE_SCAN	0x02
#define CHANNEL_NO_IBSS			0x04
#define CHANNEL_RADAR			0x08
#define CHANNEL_NO_FAT_ABOVE	0x10
#define CHANNEL_NO_FAT_BELOW	0x20
#define CHANNEL_40M_CAP			0x40

	UCHAR Flags;

} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;

/* Channel list subset */
typedef struct _CHANNEL_LIST_SUB {
	UCHAR	Channel;
	UCHAR	IdxMap; /* Index mapping to original channel list */
} CHANNEL_LIST_SUB, *PCHANNEL_LIST_SUB;


typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR EvaluatePeriod;	/* 0:not evalute status, 1: evaluate status, 2: switching status */
	UCHAR EvaluateStableCnt;
	UCHAR Pair1PrimaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
	UCHAR Pair1SecondaryRxAnt;	/* 0:Ant-E1, 1:Ant-E2 */
#ifdef CONFIG_STA_SUPPORT
	SHORT Pair1AvgRssi[2];	/* AvgRssi[0]:E1, AvgRssi[1]:E2 */
	SHORT Pair2AvgRssi[2];	/* AvgRssi[0]:E3, AvgRssi[1]:E4 */
#endif /* CONFIG_STA_SUPPORT */
	SHORT Pair1LastAvgRssi;	/* */
	SHORT Pair2LastAvgRssi;	/* */
	ULONG RcvPktNumWhenEvaluate;
	BOOLEAN FirstPktArrivedWhenEvaluate;
#ifdef CONFIG_AP_SUPPORT
	LONG Pair1AvgRssiGroup1[2];
	LONG Pair1AvgRssiGroup2[2];
	ULONG RcvPktNum[2];
#endif /* CONFIG_AP_SUPPORT */
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
/* Power save method control */
typedef union _PS_CONTROL {
	struct {
		ULONG EnablePSinIdle:1;	/* Enable radio off when not connect to AP. radio on only when sitesurvey, */
		ULONG EnableNewPS:1;	/* Enable new  Chip power save fucntion . New method can only be applied in chip version after 2872. and PCIe. */
		ULONG rt30xxPowerMode:2;	/* Power Level Mode for rt30xx chip */
		ULONG rt30xxFollowHostASPM:1;	/* Card Follows Host's setting for rt30xx chip. */
		ULONG rt30xxForceASPMTest:1;	/* Force enable L1 for rt30xx chip. This has higher priority than rt30xxFollowHostASPM Mode. */
/*		ULONG		rsv:26;			// Radio Measurement Enable */
		ULONG AMDNewPSOn:1;	/* Enable for AMD L1 (toggle) */
		ULONG LedMode:2;	/* 0: Blink normal.  1: Slow blink not normal. */
		ULONG rt30xxForceL0:1;	/* Force only use L0 for rt30xx */

		/* PCIe config space [Completion TimeOut Disable], compatible issue with Intel HM55 */
		ULONG CTO:1;	/* 0: default, update the CTO bit to disable; 1: Keep BIOS config value */
		ULONG PM4PCIeCLKOn:1;	/* 0: default, turn off PCIE CLk at PM4; 1: FW MCU cmd arg1 as "0x5a" which will not turn off PCIE CLK */

		ULONG rsv:20;	/* Rsvd */
	} field;
	ULONG word;
} PS_CONTROL, *PPS_CONTROL;
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */
/***************************************************************************
  *	structure for MLME state machine
  **************************************************************************/
typedef struct _MLME_STRUCT {
#ifdef CONFIG_STA_SUPPORT
	/* STA state machines */
	STATE_MACHINE CntlMachine;
	STATE_MACHINE AssocMachine;
	STATE_MACHINE AuthMachine;
	STATE_MACHINE AuthRspMachine;
	STATE_MACHINE SyncMachine;
	STATE_MACHINE WpaPskMachine;
	STATE_MACHINE LeapMachine;
	STATE_MACHINE_FUNC AssocFunc[ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC AuthFunc[AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC AuthRspFunc[AUTH_RSP_FUNC_SIZE];
	STATE_MACHINE_FUNC SyncFunc[SYNC_FUNC_SIZE];

#endif /* CONFIG_STA_SUPPORT */
	STATE_MACHINE_FUNC ActFunc[ACT_FUNC_SIZE];
	/* Action */
	STATE_MACHINE ActMachine;

#ifdef WSC_INCLUDED
	STATE_MACHINE WscMachine;
	STATE_MACHINE_FUNC WscFunc[WSC_FUNC_SIZE];

#ifdef IWSC_SUPPORT
	STATE_MACHINE			IWscMachine;
	STATE_MACHINE_FUNC		IWscFunc[IWSC_FUNC_SIZE];
#endif /* IWSC_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef QOS_DLS_SUPPORT
	STATE_MACHINE DlsMachine;
	STATE_MACHINE_FUNC DlsFunc[DLS_FUNC_SIZE];
#endif /* QOS_DLS_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
	STATE_MACHINE TdlsMachine;
	STATE_MACHINE_FUNC TdlsFunc[TDLS_FUNC_SIZE];
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/* AP state machines */
	STATE_MACHINE ApAssocMachine;
	STATE_MACHINE ApAuthMachine;
	STATE_MACHINE ApSyncMachine;
	STATE_MACHINE_FUNC ApAssocFunc[AP_ASSOC_FUNC_SIZE];
/*	STATE_MACHINE_FUNC		ApDlsFunc[DLS_FUNC_SIZE]; */
	STATE_MACHINE_FUNC ApAuthFunc[AP_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC ApSyncFunc[AP_SYNC_FUNC_SIZE];
#ifdef APCLI_SUPPORT
	STATE_MACHINE ApCliAuthMachine;
	STATE_MACHINE ApCliAssocMachine;
	STATE_MACHINE ApCliCtrlMachine;
	STATE_MACHINE ApCliSyncMachine;
	STATE_MACHINE ApCliWpaPskMachine;

	STATE_MACHINE_FUNC ApCliAuthFunc[APCLI_AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliAssocFunc[APCLI_ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliCtrlFunc[APCLI_CTRL_FUNC_SIZE];
	STATE_MACHINE_FUNC ApCliSyncFunc[APCLI_SYNC_FUNC_SIZE];
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/* common WPA state machine */
	STATE_MACHINE WpaMachine;
	STATE_MACHINE_FUNC WpaFunc[WPA_FUNC_SIZE];


	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */
	ULONG Now32;		/* latch the value of NdisGetSystemUpTime() */
	ULONG LastSendNULLpsmTime;

	BOOLEAN bRunning;
	NDIS_SPIN_LOCK TaskLock;
	MLME_QUEUE Queue;

	UINT ShiftReg;

	RALINK_TIMER_STRUCT PeriodicTimer;
	RALINK_TIMER_STRUCT APSDPeriodicTimer;
	RALINK_TIMER_STRUCT LinkDownTimer;
	RALINK_TIMER_STRUCT LinkUpTimer;
#ifdef RTMP_MAC_PCI
	UCHAR bPsPollTimerRunning;
	RALINK_TIMER_STRUCT PsPollTimer;
	RALINK_TIMER_STRUCT RadioOnOffTimer;
#endif /* RTMP_MAC_PCI */
	ULONG PeriodicRound;
	ULONG GPIORound;
	ULONG OneSecPeriodicRound;

	UCHAR RealRxPath;
	BOOLEAN bLowThroughput;
	BOOLEAN bEnableAutoAntennaCheck;
	RALINK_TIMER_STRUCT RxAntEvalTimer;


} MLME_STRUCT, *PMLME_STRUCT;

#ifdef DOT11_N_SUPPORT
/***************************************************************************
  *	802.11 N related data structures
  **************************************************************************/
struct reordering_mpdu {
	struct reordering_mpdu *next;
	PNDIS_PACKET pPacket;	/* coverted to 802.3 frame */
	int Sequence;		/* sequence number of MPDU */
	BOOLEAN bAMSDU;
	UCHAR					OpMode;
};

struct reordering_list {
	struct reordering_mpdu *next;
	int qlen;
};

struct reordering_mpdu_pool {
	PVOID mem;
	NDIS_SPIN_LOCK lock;
	struct reordering_list freelist;
};

typedef enum _REC_BLOCKACK_STATUS {
	Recipient_NONE = 0,
	Recipient_USED,
	Recipient_HandleRes,
	Recipient_Accept
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS {
	Originator_NONE = 0,
	Originator_USED,
	Originator_WaitRes,
	Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct _BA_ORI_ENTRY {
	UCHAR Wcid;
	UCHAR TID;
	UCHAR BAWinSize;
	UCHAR Token;
/* Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header. */
	USHORT Sequence;
	USHORT TimeOutValue;
	ORI_BLOCKACK_STATUS ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
	PVOID pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

typedef struct _BA_REC_ENTRY {
	UCHAR Wcid;
	UCHAR TID;
	UCHAR BAWinSize;	/* 7.3.1.14. each buffer is capable of holding a max AMSDU or MSDU. */
	/*UCHAR NumOfRxPkt; */
	/*UCHAR    Curindidx; // the head in the RX reordering buffer */
	USHORT LastIndSeq;
/*	USHORT		LastIndSeqAtTimer; */
	USHORT TimeOutValue;
	RALINK_TIMER_STRUCT RECBATimer;
	ULONG LastIndSeqAtTimer;
	ULONG nDropPacket;
	ULONG rcvSeq;
	REC_BLOCKACK_STATUS REC_BA_Status;
/*	UCHAR	RxBufIdxUsed; */
	/* corresponding virtual address for RX reordering packet storage. */
	/*RTMP_REORDERDMABUF MAP_RXBuf[MAX_RX_REORDERBUF]; */
	NDIS_SPIN_LOCK RxReRingLock;	/* Rx Ring spinlock */
/*	struct _BA_REC_ENTRY *pNext; */
	PVOID pAdapter;
	struct reordering_list list;
} BA_REC_ENTRY, *PBA_REC_ENTRY;


typedef struct {
	ULONG numAsRecipient;	/* I am recipient of numAsRecipient clients. These client are in the BARecEntry[] */
	ULONG numAsOriginator;	/* I am originator of   numAsOriginator clients. These clients are in the BAOriEntry[] */
	ULONG numDoneOriginator;	/* count Done Originator sessions */
	BA_ORI_ENTRY BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	BA_REC_ENTRY BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
} BA_TABLE, *PBA_TABLE;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_REC_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize */
	UCHAR rsv;
	UCHAR BufSize[8];
	REC_BLOCKACK_STATUS REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

/*For QureyBATableOID use; */
typedef struct GNU_PACKED _OID_BA_ORI_ENTRY {
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR BaBitmap;		/* if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status */
	UCHAR rsv;
	UCHAR BufSize[8];
	ORI_BLOCKACK_STATUS ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE {
	OID_BA_ORI_ENTRY BAOriEntry[32];
	OID_BA_REC_ENTRY BARecEntry[32];
	UCHAR OriNum;		/* Number of below BAOriEntry */
	UCHAR RecNum;		/* Number of below BARecEntry */
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef union _BACAP_STRUC {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32:4;
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
		UINT32 MpduDensity:3;
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 TxBAWinLimit:8;
		UINT32 RxBAWinLimit:8;
	} field;
#else
	struct {
		UINT32 RxBAWinLimit:8;
		UINT32 TxBAWinLimit:8;
		UINT32 AutoBA:1;	/* automatically BA */
		UINT32 Policy:2;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use */
		UINT32 MpduDensity:3;
		UINT32 AmsduEnable:1;	/*Enable AMSDU transmisstion */
		UINT32 AmsduSize:1;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
		UINT32 MMPSmode:2;	/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
		UINT32 bHtAdhoc:1;	/* adhoc can use ht rate. */
		UINT32 b2040CoexistScanSup:1;	/*As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz. */
		 UINT32:4;
	} field;
#endif
	UINT32 word;
} BACAP_STRUC, *PBACAP_STRUC;

typedef struct {
	BOOLEAN IsRecipient;
	UCHAR MACAddr[MAC_ADDR_LEN];
	UCHAR TID;
	UCHAR nMSDU;
	USHORT TimeOut;
	BOOLEAN bAllTid;	/* If True, delete all TID for BA sessions with this MACaddr. */
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

#ifdef DOT11N_DRAFT3
typedef enum _BSS2040COEXIST_FLAG {
	BSS_2040_COEXIST_DISABLE = 0,
	BSS_2040_COEXIST_TIMER_FIRED = 1,
	BSS_2040_COEXIST_INFO_SYNC = 2,
	BSS_2040_COEXIST_INFO_NOTIFY = 4,
} BSS2040COEXIST_FLAG;

typedef struct _BssCoexChRange_ {
	UCHAR primaryCh;
	UCHAR secondaryCh;
	UCHAR effectChStart;
	UCHAR effectChEnd;
} BSS_COEX_CH_RANGE;
#endif /* DOT11N_DRAFT3 */

#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#endif /* DOT11_N_SUPPORT */

/*This structure is for all 802.11n card InterOptibilityTest action. Reset all Num every n second.  (Details see MLMEPeriodic) */
typedef struct _IOT_STRUC {
	BOOLEAN bRTSLongProtOn;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bLastAtheros;
	BOOLEAN bCurrentAtheros;
	BOOLEAN bNowAtherosBurstOn;
	BOOLEAN bNextDisableRxBA;
	BOOLEAN bToggle;
#endif /* CONFIG_STA_SUPPORT */
} IOT_STRUC, *PIOT_STRUC;

/* This is the registry setting for 802.11n transmit setting.  Used in advanced page. */
typedef union _REG_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rsv:13;
		UINT32 EXTCHA:2;
		UINT32 HTMODE:1;
		UINT32 TRANSNO:2;
		UINT32 STBC:1;	/*SPACE */
		UINT32 ShortGI:1;
		UINT32 BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		UINT32 TxBF:1;	/* 3*3 */
		UINT32 ITxBfEn:1;
		UINT32 rsv0:9;
		/*UINT32  MCS:7;                 // MCS */
		/*UINT32  PhyMode:4; */
	} field;
#else
	struct {
		/*UINT32  PhyMode:4; */
		/*UINT32  MCS:7;                 // MCS */
		UINT32 rsv0:9;
		UINT32 ITxBfEn:1;
		UINT32 TxBF:1;
		UINT32 BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		UINT32 ShortGI:1;
		UINT32 STBC:1;	/*SPACE */
		UINT32 TRANSNO:2;
		UINT32 HTMODE:1;
		UINT32 EXTCHA:2;
		UINT32 rsv:13;
	} field;
#endif
	UINT32 word;
} REG_TRANSMIT_SETTING, *PREG_TRANSMIT_SETTING;


typedef union _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT rsv:3;
		USHORT FixedTxMode:2;	/* If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode. */
		USHORT PhyMode:4;
		USHORT MCS:7;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:7;	/* MCS */
		USHORT PhyMode:4;
		USHORT FixedTxMode:2;	/* If MCS isn't AUTO, fix rate in CCK, OFDM or HT mode. */
		USHORT rsv:3;
	} field;
#endif
	USHORT word;
 } DESIRED_TRANSMIT_SETTING, *PDESIRED_TRANSMIT_SETTING;



/***************************************************************************
  *	Multiple SSID related data structures
  **************************************************************************/
#define WLAN_MAX_NUM_OF_TIM			((MAX_LEN_OF_MAC_TABLE >> 3) + 1)	/* /8 + 1 */
#define WLAN_CT_TIM_BCMC_OFFSET		0	/* unit: 32B */

/* clear bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_CLEAR(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~NUM_BIT8[0];

/* set bcmc TIM bit */
#define WLAN_MR_TIM_BCMC_SET(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= NUM_BIT8[0];

/* clear a station PS TIM bit */
#define WLAN_MR_TIM_BIT_CLEAR(ad_p, apidx, wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] &= (~NUM_BIT8[bit_offset]); }

/* set a station PS TIM bit */
#define WLAN_MR_TIM_BIT_SET(ad_p, apidx, wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] |= NUM_BIT8[bit_offset]; }


#ifdef CONFIG_AP_SUPPORT
typedef struct _MULTISSID_STRUCT {

#ifdef HOSTAPD_SUPPORT
	BOOLEAN Hostapd;
	BOOLEAN HostapdWPS;
#endif

	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	USHORT CapabilityInfo;

	UCHAR MaxStaNum;	/* Limit the STA connection number per BSS */
	UCHAR StaCount;

	PNET_DEV MSSIDDev;

	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	WPA_MIX_PAIR_CIPHER WpaMixPairCipher;

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */
	RT_HT_PHY_INFO DesiredHtPhyInfo;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;	/* Desired transmit setting. this is for reading registry setting only. not useful. */
	BOOLEAN bAutoTxRateSwitch;

	LARGE_INTEGER ReceivedByteCount;
	LARGE_INTEGER TransmittedByteCount;
	ULONG TxCount;
	ULONG RxCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;

	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;

	/*CIPHER_KEY                          SharedKey[SHARE_KEY_NUM]; // ref pAd->SharedKey[BSS][4] */
	UCHAR DefaultKeyId;

	UCHAR TxRate;		/* RATE_1, RATE_2, RATE_5_5, RATE_11, ... */
	UCHAR DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR DesiredRatesIndex;
	UCHAR MaxTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */

	UCHAR TimBitmaps[WLAN_MAX_NUM_OF_TIM];

	/* WPA */
	UCHAR GMK[32];
	UCHAR PMK[32];
	UCHAR GTK[32];
#ifdef DOT1X_SUPPORT
	BOOLEAN IEEE8021X;
	BOOLEAN PreAuth;
#endif				/* DOT1X_SUPPORT */
	UCHAR GNonce[32];
	UCHAR PortSecured;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;
	UCHAR BANClass3Data;
	BOOLEAN IsolateInterStaTraffic;
	BOOLEAN IsolateInterStaMBCast;

	UCHAR RSNIE_Len[2];
	UCHAR RSN_IE[2][MAX_LEN_OF_RSNIE];

	/* for Group Rekey */
	RT_WPA_REKEY WPAREKEY;
	ULONG REKEYCOUNTER;
	RALINK_TIMER_STRUCT REKEYTimer;
	UCHAR REKEYTimerRunning;
	UINT8 RekeyCountDown;

#ifdef WAPI_SUPPORT
	UCHAR WAPIPassPhrase[64];	/* WAPI PSK pass phrase */
	UINT WAPIPassPhraseLen;	/* the length of WAPI PSK pass phrase */
	UINT WapiPskType;	/* 0 - Hex, 1 - ASCII */
	UCHAR WAPI_BK[16];	/* WAPI base key */

	UCHAR NMK[LEN_WAPI_NMK];
	UCHAR key_announce_flag[LEN_WAPI_TSC];
	BOOLEAN sw_wpi_encrypt;	/* WPI data encrypt by SW */
#endif /* WAPI_SUPPORT */

	UCHAR TimIELocationInBeacon;
	UCHAR CapabilityInfoLocationInBeacon;

	/* For PMK Cache using */
	ULONG PMKCachePeriod;	/* unit : jiffies */
	NDIS_AP_802_11_PMKID PMKIDCache;

	/* outgoing BEACON frame buffer and corresponding TXWI */
	/* PTXWI_STRUC                           BeaconTxWI; */
	CHAR BeaconBuf[MAX_BEACON_SIZE];	/* NOTE: BeaconBuf should be 4-byte aligned */

	BOOLEAN bHideSsid;
	UINT16 StationKeepAliveTime;	/* unit: second */

	/* VLAN related */
	BOOLEAN								bVLAN_Tag;
	USHORT VLAN_VID;
	USHORT VLAN_Priority;

	RT_802_11_ACL AccessControlList;

	/* EDCA Qos */
	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */

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
	BOOLEAN bWmmCapableOrg;	/* origin Wmm Capable in non-11n mode */

#ifdef QOS_DLS_SUPPORT
	UCHAR DlsPTK[64];	/* Due to windows dirver count on meetinghouse to handle 4-way shake */
#endif /* QOS_DLS_SUPPORT */

#ifdef DOT1X_SUPPORT
	/* For 802.1x daemon setting per BSS */
	UINT8 radius_srv_num;
	RADIUS_SRV_INFO radius_srv_info[MAX_RADIUS_SRV_NUM];
	UINT8 NasId[IFNAMSIZ];
	UINT8 NasIdLen;
#endif /* DOT1X_SUPPORT */

#ifdef RTL865X_SOC
	unsigned int mylinkid;
#endif

#ifdef CONFIG_AP_SUPPORT
	WSC_LV_INFO WscIEBeacon;
	WSC_LV_INFO WscIEProbeResp;
#ifdef WSC_AP_SUPPORT
	WSC_CTRL WscControl;
	WSC_SECURITY_MODE WscSecurityMode;
#endif /* WSC_AP_SUPPORT */

#endif				/* CONFIG_AP_SUPPORT */

#ifdef IDS_SUPPORT
	UINT32 RcvdConflictSsidCount;
	UINT32 RcvdSpoofedAssocRespCount;
	UINT32 RcvdSpoofedReassocRespCount;
	UINT32 RcvdSpoofedProbeRespCount;
	UINT32 RcvdSpoofedBeaconCount;
	UINT32 RcvdSpoofedDisassocCount;
	UINT32 RcvdSpoofedAuthCount;
	UINT32 RcvdSpoofedDeauthCount;
	UINT32 RcvdSpoofedUnknownMgmtCount;
	UINT32 RcvdReplayAttackCount;

	CHAR RssiOfRcvdConflictSsid;
	CHAR RssiOfRcvdSpoofedAssocResp;
	CHAR RssiOfRcvdSpoofedReassocResp;
	CHAR RssiOfRcvdSpoofedProbeResp;
	CHAR RssiOfRcvdSpoofedBeacon;
	CHAR RssiOfRcvdSpoofedDisassoc;
	CHAR RssiOfRcvdSpoofedAuth;
	CHAR RssiOfRcvdSpoofedDeauth;
	CHAR RssiOfRcvdSpoofedUnknownMgmt;
	CHAR RssiOfRcvdReplayAttack;
#endif /* IDS_SUPPORT */

	/* used in if beacon send or stop */
	BOOLEAN bBcnSntReq;
	UCHAR BcnBufIdx;





	UCHAR PhyMode;


	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;

#ifdef SPECIFIC_TX_POWER_SUPPORT
	CHAR TxPwrAdj;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

} MULTISSID_STRUCT, *PMULTISSID_STRUCT;
#endif /* CONFIG_AP_SUPPORT */

/* configuration common to OPMODE_AP as well as OPMODE_STA */
typedef struct _COMMON_CONFIG {

	BOOLEAN bCountryFlag;
	UCHAR CountryCode[3];
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR Geography;
	UCHAR DfsType;
	PUCHAR pChDesp;
#endif				/* EXT_BUILD_CHANNEL_LIST */
	UCHAR CountryRegion;	/* Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel */
	UCHAR CountryRegionForABand;	/* Enum of country region for A band */
	UCHAR PhyMode;		/* PHY_11A, PHY_11B, PHY_11BG_MIXED, PHY_ABG_MIXED */
	UCHAR DesiredPhyMode;	/* PHY_11A, PHY_11B, PHY_11BG_MIXED, PHY_ABG_MIXED */
	UCHAR SavedPhyMode;
	USHORT Dsifs;		/* in units of usec */
	ULONG PacketFilter;	/* Packet filter for receiving */
	UINT8 RegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS];

	CHAR Ssid[MAX_LEN_OF_SSID];	/* NOT NULL-terminated */
	UCHAR SsidLen;		/* the actual ssid length in used */
	UCHAR LastSsidLen;	/* the actual ssid length in used */
	CHAR LastSsid[MAX_LEN_OF_SSID];	/* NOT NULL-terminated */
	UCHAR LastBssid[MAC_ADDR_LEN];

	UCHAR Bssid[MAC_ADDR_LEN];
	USHORT BeaconPeriod;
	UCHAR Channel;
	UCHAR CentralChannel;	/* Central Channel when using 40MHz is indicating. not real channel. */
#ifdef SINGLE_SKU_V2
	UCHAR SkuChannel;
#endif /* SINGLE_SKU_V2 */

	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen;
	UCHAR DesireRate[MAX_LEN_OF_SUPPORTED_RATES];	/* OID_802_11_DESIRED_RATES */
	UCHAR MaxDesiredRate;
	UCHAR ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

	ULONG BasicRateBitmap;	/* backup basic ratebitmap */
	ULONG BasicRateBitmapOld;	/* backup basic ratebitmap */

	BOOLEAN bInServicePeriod;


	BOOLEAN bAPSDAC_BE;
	BOOLEAN bAPSDAC_BK;
	BOOLEAN bAPSDAC_VI;
	BOOLEAN bAPSDAC_VO;

#ifdef DOT11Z_TDLS_SUPPORT
	BOOLEAN	 TDLS_bAPSDAC_BE;
	BOOLEAN	 TDLS_bAPSDAC_BK;
	BOOLEAN	 TDLS_bAPSDAC_VI;
	BOOLEAN	 TDLS_bAPSDAC_VO;
	UCHAR TDLS_MaxSPLength;
#endif /* DOT11Z_TDLS_SUPPORT */

	/* because TSPEC can modify the APSD flag, we need to keep the APSD flag
	   requested in association stage from the station;
	   we need to recover the APSD flag after the TSPEC is deleted. */
	BOOLEAN bACMAPSDBackup[4];	/* for delivery-enabled & trigger-enabled both */
	BOOLEAN bACMAPSDTr[4];	/* no use */
	UCHAR MaxSPLength;

	BOOLEAN bNeedSendTriggerFrame;
	BOOLEAN bAPSDForcePowerSave;	/* Force power save mode, should only use in APSD-STAUT */
	ULONG TriggerTimerCount;
	UCHAR BBPCurrentBW;	/* BW_10,       BW_20, BW_40 */
	/* move to MULTISSID_STRUCT for MBSS */
	/*HTTRANSMIT_SETTING    HTPhyMode, MaxHTPhyMode, MinHTPhyMode;// For transmit phy setting in TXWI. */
	REG_TRANSMIT_SETTING RegTransmitSetting;	/*registry transmit setting. this is for reading registry setting only. not useful. */
	/*UCHAR       FixedTxMode;              // Fixed Tx Mode (CCK, OFDM), for HT fixed tx mode (GF, MIX) , refer to RegTransmitSetting.field.HTMode */
	UCHAR TxRate;		/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR MaxTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR TxRateIndex;	/* Tx rate index in RateSwitchTable */
	UCHAR TxRateTableSize;	/* Valid Tx rate table size in RateSwitchTable */
	/*BOOLEAN               bAutoTxRateSwitch; */
	UCHAR MinTxRate;	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR RtsRate;		/* RATE_xxx */
	HTTRANSMIT_SETTING MlmeTransmit;	/* MGMT frame PHY rate setting when operatin at Ht rate. */
	UCHAR MlmeRate;		/* RATE_xxx, used to send MLME frames */
	UCHAR BasicMlmeRate;	/* Default Rate for sending MLME frames */

	USHORT RtsThreshold;	/* in unit of BYTE */
	USHORT FragmentThreshold;	/* in unit of BYTE */

	UCHAR TxPower;		/* in unit of mW */
	ULONG TxPowerPercentage;	/* 0~100 % */
	ULONG TxPowerDefault;	/* keep for TxPowerPercentage */
	UINT8 PwrConstraint;

#ifdef DOT11_N_SUPPORT
	BACAP_STRUC BACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
	BACAP_STRUC REGBACapability;	/*   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0 */
#endif				/* DOT11_N_SUPPORT */
	IOT_STRUC IOTestParm;	/* 802.11n InterOpbility Test Parameter; */
	ULONG TxPreamble;	/* Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto */
	BOOLEAN bUseZeroToDisableFragment;	/* Microsoft use 0 as disable */
	ULONG UseBGProtection;	/* 0: auto, 1: always use, 2: always not use */
	BOOLEAN bUseShortSlotTime;	/* 0: disable, 1 - use short slot (9us) */
	BOOLEAN bEnableTxBurst;	/* 1: enble TX PACKET BURST (when BA is established or AP is not a legacy WMM AP), 0: disable TX PACKET BURST */
	BOOLEAN bAggregationCapable;	/* 1: enable TX aggregation when the peer supports it */
	BOOLEAN bPiggyBackCapable;	/* 1: enable TX piggy-back according MAC's version */
	BOOLEAN bIEEE80211H;	/* 1: enable IEEE802.11h spec. */
	UCHAR RDDurRegion; /* Region of radar detection */
	ULONG DisableOLBCDetect;	/* 0: enable OLBC detect; 1 disable OLBC detect */

#ifdef DOT11_N_SUPPORT
	BOOLEAN bRdg;
#endif				/* DOT11_N_SUPPORT */
	BOOLEAN bWmmCapable;	/* 0:disable WMM, 1:enable WMM */
	QOS_CAPABILITY_PARM APQosCapability;	/* QOS capability of the current associated AP */
	EDCA_PARM APEdcaParm;	/* EDCA parameters of the current associated AP */
#ifdef MULTI_CLIENT_SUPPORT
	BOOLEAN	bWmm;               /* have BSS enable/disable WMM */
	UCHAR APCwmin;            /* record ap cwmin */
	UCHAR APCwmax;            /* record ap cwmax */
	UCHAR BSSCwmin;           /* record BSS cwmin */
#endif /* MULTI_CLIENT_SUPPORT */
	QBSS_LOAD_PARM APQbssLoad;	/* QBSS load of the current associated AP */
	UCHAR AckPolicy[4];	/* ACK policy of the specified AC. see ACK_xxx */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN bDLSCapable;	/* 0:disable DLS, 1:enable DLS */
#endif				/* CONFIG_STA_SUPPORT */
	/* a bitmap of BOOLEAN flags. each bit represent an operation status of a particular */
	/* BOOLEAN control, either ON or OFF. These flags should always be accessed via */
	/* OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros. */
	/* see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition */
	ULONG OpStatusFlags;

	BOOLEAN NdisRadioStateOff;	/*For HCT 12.0, set this flag to TRUE instead of called MlmeRadioOff. */
	ABGBAND_STATE BandState;	/* For setting BBP used on B/G or A mode. */

#ifdef DFS_SUPPORT
	/* IEEE802.11H--DFS. */
	RADAR_DETECT_STRUCT RadarDetect;
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	CARRIER_DETECTION_STRUCT CarrierDetect;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* HT */
	/*UCHAR                 BASize;         // USer desired BAWindowSize. Should not exceed our max capability */
	/*RT_HT_CAPABILITY      SupportedHtPhy; */
	RT_HT_CAPABILITY DesiredHtPhy;
	HT_CAPABILITY_IE HtCapability;
	ADD_HT_INFO_IE AddHTInfo;	/* Useful as AP. */
	/*This IE is used with channel switch announcement element when changing to a new 40MHz. */
	/*This IE is included in channel switch ammouncement frames 7.4.1.5, beacons, probe Rsp. */
	NEW_EXT_CHAN_IE NewExtChanOffset;	/*7.3.2.20A, 1 if extension channel is above the control channel, 3 if below, 0 if not present */

	EXT_CAP_INFO_ELEMENT ExtCapIE;	/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */

#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable;
	/* 
	   Following two paramters now only used for the initial scan operation. the AP only do 
	   bandwidth fallback when BssCoexApCnt > BssCoexApCntThr
	   By default, the "BssCoexApCntThr" is set as 0 in "UserCfgInit()".
	 */
	UCHAR BssCoexApCntThr;
	UCHAR BssCoexApCnt;

	UCHAR Bss2040CoexistFlag;	/* bit 0: bBssCoexistTimerRunning, bit 1: NeedSyncAddHtInfo. */
	RALINK_TIMER_STRUCT Bss2040CoexistTimer;
	UCHAR					Bss2040NeedFallBack; 	/* 1: Need Fall back to 20MHz */

	/*This IE is used for 20/40 BSS Coexistence. */
	BSS_2040_COEXIST_IE BSS2040CoexistInfo;

	USHORT Dot11OBssScanPassiveDwell;	/* Unit : TU. 5~1000 */
	USHORT Dot11OBssScanActiveDwell;	/* Unit : TU. 10~1000 */
	USHORT Dot11BssWidthTriggerScanInt;	/* Unit : Second */
	USHORT Dot11OBssScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000 */
	USHORT Dot11OBssScanActiveTotalPerChannel;	/* Unit : TU. 20~10000 */
	USHORT Dot11BssWidthChanTranDelayFactor;
	USHORT Dot11OBssScanActivityThre;	/* Unit : percentage */

	ULONG Dot11BssWidthChanTranDelay;	/* multiple of (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */
	ULONG CountDownCtr;	/* CountDown Counter from (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor) */

	BSS_2040_COEXIST_IE LastBSSCoexist2040;
	BSS_2040_COEXIST_IE BSSCoexist2040;
	TRIGGER_EVENT_TAB TriggerEventTab;
	UCHAR ChannelListIdx;

	BOOLEAN bOverlapScanning;
	BOOLEAN bBssCoexNotify;
#endif /* DOT11N_DRAFT3 */

	BOOLEAN bHTProtect;
	BOOLEAN bMIMOPSEnable;
	BOOLEAN bBADecline;
	BOOLEAN bDisableReordering;
#ifdef DOT11N_DRAFT3
	BOOLEAN bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */
	BOOLEAN bExtChannelSwitchAnnouncement;
	BOOLEAN bRcvBSSWidthTriggerEvents;
	ULONG LastRcvBSSWidthTriggerEventsTime;

	UCHAR TxBASize;

	BOOLEAN bRalinkBurstMode;
	UINT32 RestoreBurstMode;
#endif /* DOT11_N_SUPPORT */

#ifdef SYSTEM_LOG_SUPPORT
	/* Enable wireless event */
	BOOLEAN bWirelessEvent;
#endif /* SYSTEM_LOG_SUPPORT */

	BOOLEAN bWiFiTest;	/* Enable this parameter for WiFi test */

	/* Tx & Rx Stream number selection */
	UCHAR TxStream;
	UCHAR RxStream;

	/* transmit phy mode, trasmit rate for Multicast. */
#ifdef MCAST_RATE_SPECIFIC
	UCHAR McastTransmitMcs;
	UCHAR McastTransmitPhyMode;
#endif /* MCAST_RATE_SPECIFIC */

	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */


#ifdef WSC_INCLUDED
	/* WSC hardware push button function 0811 */
	UINT8 WscHdrPshBtnCheckCount;
#endif				/* WSC_INCLUDED */


	NDIS_SPIN_LOCK MeasureReqTabLock;
	PMEASURE_REQ_TAB pMeasureReqTab;

	NDIS_SPIN_LOCK TpcReqTabLock;
	PTPC_REQ_TAB pTpcReqTab;

	/* transmit phy mode, trasmit rate for Multicast. */
#ifdef MCAST_RATE_SPECIFIC
	HTTRANSMIT_SETTING MCastPhyMode;
#endif /* MCAST_RATE_SPECIFIC */

#ifdef SINGLE_SKU
	UINT16 DefineMaxTxPwr;
	BOOLEAN bSKUMode;
	UINT16 AntGain;
	UINT16 BandedgeDelta;
	UINT16 ModuleTxpower;
#endif /* SINGLE_SKU */

#ifdef WAPI_SUPPORT
	COMMON_WAPI_INFO comm_wapi_info;

	/* rekey related parameter */
	/* USK update parameter */
	UINT8 wapi_usk_rekey_method;	/* 0:disable , 1:time, 2:packet */
	UINT32 wapi_usk_rekey_threshold;	/* the rekey threshold */

	/* MSK update parameter */
	UINT8 wapi_msk_rekey_method;	/* 0:disable , 1:time, 2:packet */
	UINT32 wapi_msk_rekey_threshold;	/* the rekey threshold */

	UINT32 wapi_msk_rekey_cnt;
	RALINK_TIMER_STRUCT WapiMskRekeyTimer;
	UCHAR WapiMskRekeyTimerRunning;
#endif /* WAPI_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT
	ULONG CID;
	ULONG CN;
#ifdef RT6352
	UCHAR PKG_ID;
	UCHAR Chip_VerID;
	UCHAR Chip_E_Number;
#endif /* RT6352 */
#endif /* RTMP_RBUS_SUPPORT */

#if defined(RT305x)||defined(RT30xx)
	/* request by Gary, for High Power issue */
	UCHAR HighPowerPatchDisabled;
#endif /* defined(RT305x)|| defined(RT30xx) */

	BOOLEAN HT_DisallowTKIP;	/* Restrict the encryption type in 11n HT mode */

	BOOLEAN HT_Disable;	/* 1: disable HT function; 0: enable HT function */

#if defined (RT2883) || defined (RT3883)
	ULONG PhyRateLimit;	/* PHY Rate limit in Mbps */
	INT		FixedRate;	/* Fix the rate during Rate Adaptation. FixedRate is ItemNo index into RateSwitch Table. -1 => disabled. */
	BOOLEAN FineAGC;	/* Fine AGC enabled */
#endif /* defined (RT2883) || defined (RT3883) */

#ifdef PRE_ANT_SWITCH
	BOOLEAN PreAntSwitch;	/* Preamble Antenna Switch */
	SHORT PreAntSwitchRSSI;	/* Preamble Antenna Switch RSSI threshold */
	SHORT PreAntSwitchTimeout; /* Preamble Antenna Switch timeout in seconds */
#endif /* PRE_ANT_SWITCH */

#ifdef CFO_TRACK
	SHORT	CFOTrack;	/* CFO Tracking. 0=>use default, 1=>track, 2-7=> track 8-n times, 8=>done tracking */
#endif /* CFO_TRACK */

#ifdef NEW_RATE_ADAPT_SUPPORT
	USHORT	lowTrafficThrd;		/* Threshold for reverting to default MCS when traffic is low */
	BOOLEAN	TrainUpRule;		/* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
	SHORT	TrainUpRuleRSSI;	/* If TrainUpRule=2 then use Hybrid rule when RSSI < TrainUpRuleRSSI */
	USHORT	TrainUpLowThrd;		/* QuickDRS Hybrid train up low threshold */
	USHORT	TrainUpHighThrd;	/* QuickDRS Hybrid train up high threshold */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
#define		STREAM_MODE_STA_NUM		4

	UCHAR	StreamMode; /* 0=disabled, 1=enable for 1SS, 2=enable for 2SS, 3=enable for 1,2SS */
	UCHAR	StreamModeMac[STREAM_MODE_STA_NUM][MAC_ADDR_LEN];
	UINT16	StreamModeMCS;	/* Bit map for enabling Stream Mode based on MCS */
#endif /* STREAM_MODE_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ITxBfTimeout;
	ULONG ETxBfTimeout;
	ULONG	ETxBfEnCond;		/* Enable sending of sounding and beamforming */
	BOOLEAN	ETxBfNoncompress;	/* Force non-compressed Sounding Response */
	BOOLEAN	ETxBfIncapable;		/* Report Incapable of BF in TX BF Capabilities */
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
	ULONG DebugFlags;	/* Temporary debug flags */
#endif /* DBG_CTRL_SUPPORT */

#ifdef RTMP_MAC_PCI
	BOOLEAN bPCIeBus;	/* The adapter runs over PCIe bus */
#endif /* RTMP_MAC_PCI */

#ifdef WSC_INCLUDED
	BOOLEAN WscPBCOverlap;
	WSC_STA_PBC_PROBE_INFO WscStaPbcProbeInfo;
#endif /* WSC_INCLUDED */


#if defined(MICROWAVE_OVEN_SUPPORT) || defined(DYNAMIC_VGA_SUPPORT)
	MO_CFG_STRUCT MO_Cfg;	/* data structure for mitigating microwave interference */
#endif /* defined(MICROWAVE_OVEN_SUPPORT) || defined(DYNAMIC_VGA_SUPPORT) */
	BOOLEAN bEnTemperatureTrack;

} COMMON_CONFIG, *PCOMMON_CONFIG;

#ifdef DBG_CTRL_SUPPORT
/* DebugFlag definitions */
#define DBF_NO_BF_AWARE_RA		0x0001	/* Revert to older Rate Adaptation that is not BF aware */
#define DBF_SHOW_BF_STATS		0x0002	/* Display BF statistics in AP "iwpriv stat" display */
#define DBF_NO_TXBF_3SS			0x0004	/* Disable TXBF for MCS > 20 */
#define DBF_UNUSED0008			0x0008	/* Unused */
#define DBF_DBQ_RA_LOG			0x0010	/* Log RA information in DBQ */
#define DBF_INIT_MCS_MARGIN		0x0020	/* Use 6 dB margin when selecting initial MCS */
#define DBF_INIT_MCS_DIS1		0x0040	/* Disable highest MCSs when selecting initial MCS */
#define DBF_FORCE_QUICK_DRS		0x0080	/* Force Quick DRS even if rate didn't change */
#define DBF_FORCE_SGI			0x0100	/* Force Short GI */
#define DBF_DBQ_NO_BCN			0x0200	/* Disable logging of RX Beacon frames */
#define DBF_LOG_VCO_CAL			0x0400	/* Log VCO cal */
#define DBF_DISABLE_CAL			0x0800	/* Disable Divider Calibration at channel change */
#ifdef INCLUDE_DEBUG_QUEUE
#define DBF_DBQ_TXFIFO			0x1000	/* Enable logging of TX information from FIFO */
#define DBF_DBQ_TXFRAME			0x2000	/* Enable logging of Frames queued for TX */
#define DBF_DBQ_RXWI_FULL		0x4000	/* Enable logging of full RXWI */
#define DBF_DBQ_RXWI			0x8000	/* Enable logging of partial RXWI */
#endif /* INCLUDE_DEBUG_QUEUE */

#define DBF_SHOW_RA_LOG			0x010000	/* Display concise Rate Adaptation information */
#define DBF_SHOW_ZERO_RA_LOG	0x020000	/* Include RA Log entries when TxCount is 0 */
#define DBF_FORCE_20MHZ			0x040000	/* Force 20 MHz TX */
#define DBF_FORCE_40MHZ 		0x080000	/* Force 40 MHz Tx */
#define DBF_DISABLE_CCK			0x100000	/* Disable CCK */
#define DBF_UNUSED200000		0x200000	/* Unused */
#define DBF_ENABLE_HT_DUP		0x400000	/* Allow HT Duplicate mode in TX rate table */
#define DBF_ENABLE_CCK_5G		0x800000	/* Enable CCK rates in 5G band */
#define DBF_UNUSED0100000		0x0100000	/* Unused */
#define DBF_ENABLE_20MHZ_MCS8	0x02000000	/* Substitute 20MHz MCS8 for 40MHz MCS8 */
#define DBF_DISABLE_20MHZ_MCS0	0x04000000	/* Disable substitution of 20MHz MCS0 for 40MHz MCS32 */
#define DBF_DISABLE_20MHZ_MCS1	0x08000000	/* Disable substitution of 20MHz MCS1 for 40MHz MCS0 */
#endif /* DBG_CTRL_SUPPORT */

#ifdef CONFIG_STA_SUPPORT

#ifdef CREDENTIAL_STORE
typedef struct _STA_CONNECT_INFO {
	BOOLEAN Changeable;
	BOOLEAN IEEE8021X;
	CHAR Ssid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
	UCHAR SsidLen; // the actual ssid length in used
	NDIS_802_11_AUTHENTICATION_MODE AuthMode; // This should match to whatever microsoft defined
	NDIS_802_11_WEP_STATUS WepStatus;
	UCHAR DefaultKeyId;
	UCHAR PMK[LEN_PMK]; // WPA PSK mode PMK
	UCHAR WpaPassPhrase[64]; // WPA PSK pass phrase
	UINT WpaPassPhraseLen; // the length of WPA PSK pass phrase
	UINT8 WpaState;
	CIPHER_KEY SharedKey[1][4]; // STA always use SharedKey[BSS0][0..3]
	NDIS_SPIN_LOCK Lock;
} STA_CONNECT_INFO, *P_STA_CONNECT_INFO;
#endif /* CREDENTIAL_STORE */

#ifdef DOT11Z_TDLS_SUPPORT
typedef struct _TDLS_STRUCT
{
	BOOLEAN bTDLSCapable;	/* 0:disable TDLS, 1:enable TDLS */
	BOOLEAN	TdlsChSwitchSupp;
	BOOLEAN	TdlsPsmSupp;
	UINT8 TdlsDialogToken;
	UINT32 TdlsKeyLifeTime;
	UINT8 TdlsLinkSize;		/* record how much links establish already. */
	RT_802_11_TDLS TDLSEntry[MAX_NUM_OF_TDLS_ENTRY];
	NDIS_SPIN_LOCK TDLSEntryLock;
	NDIS_SPIN_LOCK TDLSUapsdLock;
#ifdef TDLS_AUTOLINK_SUPPORT
	BOOLEAN	TdlsAutoLink;
	LIST_HEADER TdlsDiscovPeerList;
	NDIS_SPIN_LOCK TdlsDiscovPeerListSemLock;
	LIST_HEADER TdlsBlackList;
	NDIS_SPIN_LOCK TdlsBlackListSemLock;

	CHAR TdlsAutoSetupRssiThreshold;
	CHAR TdlsAutoTeardownRssiThreshold;
	USHORT TdlsRssiMeasurementPeriod;
	USHORT TdlsDisabledPeriodByTeardown;
	USHORT TdlsAutoDiscoveryPeriod;
#endif /* TDLS_AUTOLINK_SUPPORT */

	/* Channel Switch */
	UINT16 TdlsSwitchTime;
	UINT16 TdlsSwitchTimeout;
	BOOLEAN TdlsForcePowerSaveWithAP;
	BOOLEAN bTdlsNoticeAPPowerSave;

	RALINK_TIMER_STRUCT TdlsPeriodGoBackBaseChTimer;
	RALINK_TIMER_STRUCT TdlsPeriodGoOffChTimer;
	RALINK_TIMER_STRUCT TdlsResponderGoBackBaseChTimer;
	RALINK_TIMER_STRUCT TdlsDisableChannelSwitchTimer;

	BOOLEAN bDoingPeriodChannelSwitch;
	BOOLEAN bChannelSwitchUnSolRsp;
	BOOLEAN bChannelSwitchInitiator;
	BOOLEAN bTdlsCurrentDoingChannelSwitchWaitSuccess;
	UCHAR TdlsTargetOffChannel;
	UCHAR TdlsTargetOffChannelBW;
	UCHAR TdlsCurrentChannel;
	UCHAR TdlsCurrentChannelBW;
	UCHAR TdlsCurrentOperateChannel;
	UCHAR TdlsCurrentOperateChannelBW;

	UINT16 TdlsOffChannelDelay;
	UCHAR TdlsChannelSwitchPairCount;
	UCHAR TdlsChannelSwitchRetryCount;
	ULONG TdlsGoBackStartTime;
	ULONG TdlsGlobalSwitchTime;
	ULONG TdlsGlobalSwitchTimeOut;
	UCHAR TdlsSendNullFrameCount;
	BOOLEAN TdlsChannelSwitchHWTimerRunning;
	UCHAR TdlsChannelSwitchHWTimerCount;
	UCHAR TdlsDtimCount;

	/* record old power save mode */
#define TDLS_POWER_SAVE_ACTIVE_COUNT_DOWN_NUM		(5*1000/MLME_TASK_EXEC_INTV)
	BOOLEAN TdlsFlgIsKeepingActiveCountDown; /* keep active until 0 */
	UINT8 TdlsPowerSaveActiveCountDown;
}TDLS_STRUCT, *PTDLS_STRUCT;
#endif // DOT11Z_TDLS_SUPPORT //

/* Modified by Wu Xi-Kun 4/21/2006 */
/* STA configuration and status */
typedef struct _STA_ADMIN_CONFIG {
	/*
		GROUP 1 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, but not necessary fully equal to the final
		settings in ACTIVE BSS after negotiation/compromize with the BSS holder (either
		AP or IBSS holder).
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	UCHAR BssType;		/* BSS_INFRA or BSS_ADHOC */

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
#define MONITOR_FLAG_11N_SNIFFER		0x01
	UCHAR BssMonitorFlag;	/* Specific flag for monitor */
#endif				/* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

	USHORT AtimWin;		/* used when starting a new IBSS */

	/*
		GROUP 2 -
		User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
		the user intended configuration, and should be always applied to the final
		settings in ACTIVE BSS without compromising with the BSS holder.
		Once initialized, user configuration can only be changed via OID_xxx
	*/
	UCHAR RssiTrigger;
	UCHAR RssiTriggerMode;	/* RSSI_TRIGGERED_UPON_BELOW_THRESHOLD or RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD */
	USHORT DefaultListenCount;	/* default listen count; */
	ULONG WindowsPowerMode;	/* Power mode for AC power */
	ULONG WindowsBatteryPowerMode;	/* Power mode for battery if exists */
	BOOLEAN bWindowsACCAMEnable;	/* Enable CAM power mode when AC on */
	BOOLEAN bAutoReconnect;	/* Set to TRUE when setting OID_802_11_SSID with no matching BSSID */
	ULONG WindowsPowerProfile;	/* Windows power profile, for NDIS5.1 PnP */

	BOOLEAN	 FlgPsmCanNotSleep; /* TRUE: can not switch ASIC to sleep */
	/* MIB:ieee802dot11.dot11smt(1).dot11StationConfigTable(1) */
	USHORT Psm;		/* power management mode   (PWR_ACTIVE|PWR_SAVE) */
	USHORT DisassocReason;
	UCHAR DisassocSta[MAC_ADDR_LEN];
	USHORT DeauthReason;
	UCHAR DeauthSta[MAC_ADDR_LEN];
	USHORT AuthFailReason;
	UCHAR AuthFailSta[MAC_ADDR_LEN];

	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;

	/* Add to support different cipher suite for WPA2/WPA mode */
	NDIS_802_11_ENCRYPTION_STATUS GroupCipher;	/* Multicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS PairCipher;	/* Unicast cipher suite */
	BOOLEAN bMixCipher;	/* Indicate current Pair & Group use different cipher suites */
	USHORT RsnCapability;

	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;

	UCHAR WpaPassPhrase[64];	/* WPA PSK pass phrase */
	UINT WpaPassPhraseLen;	/* the length of WPA PSK pass phrase */
	UCHAR PMK[LEN_PMK];	/* WPA PSK mode PMK */
	UCHAR PTK[LEN_PTK];	/* WPA PSK mode PTK */
	UCHAR GMK[LEN_GMK];	/* WPA PSK mode GMK */
	UCHAR GTK[MAX_LEN_GTK];	/* GTK from authenticator */
	UCHAR GNonce[32];	/* GNonce for WPA2PSK from authenticator */
	CIPHER_KEY TxGTK;
	BSSID_INFO SavedPMK[PMKID_NO];
	UINT SavedPMKNum;	/* Saved PMKID number */

	UCHAR DefaultKeyId;

#ifdef WAPI_SUPPORT
	UCHAR WAPIPassPhrase[64];	/* WAPI PSK pass phrase */
	UINT WAPIPassPhraseLen;	/* the length of WAPI PSK pass phrase */
	UINT WapiPskType;	/* 0 - Hex, 1 - ASCII */
	UCHAR WAPI_BK[16];	/* WAPI base key */

	UCHAR NMK[16];		/* WAPI notify master key */
#endif				/* WAPI_SUPPORT */

	/* WPA 802.1x port control, WPA_802_1X_PORT_SECURED, WPA_802_1X_PORT_NOT_SECURED */
	UCHAR PortSecured;

	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG MicErrCnt;	/* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */
	/* For WPA-PSK supplicant state */
	UINT8 WpaState;		/* Default is SS_NOTUSE and handled by microsoft 802.1x */
	UCHAR ReplayCounter[8];
	UCHAR ANonce[32];	/* ANonce for WPA-PSK from aurhenticator */
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */

	UCHAR LastSNR0;		/* last received BEACON's SNR */
	UCHAR LastSNR1;		/* last received BEACON's SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;		/* last received BEACON's SNR for 3nd  antenna */
	INT32 BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
#endif /* DOT11N_SS3_SUPPORT */
	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	ULONG LastBeaconRxTime;	/* OS's timestamp of the last BEACON RX time */
	ULONG Last11bBeaconRxTime;	/* OS's timestamp of the last 11B BEACON RX time */
	ULONG Last11gBeaconRxTime;	/* OS's timestamp of the last 11G BEACON RX time */
	ULONG Last20NBeaconRxTime;	/* OS's timestamp of the last 20MHz N BEACON RX time */

	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */
	BOOLEAN bNotFirstScan;	/* Sam add for ADHOC flag to do first scan when do initialization */
	BOOLEAN bSwRadio;	/* Software controlled Radio On/Off, TRUE: On */
	BOOLEAN bHwRadio;	/* Hardware controlled Radio On/Off, TRUE: On */
	BOOLEAN bRadio;		/* Radio state, And of Sw & Hw radio state */
	BOOLEAN bHardwareRadio;	/* Hardware controlled Radio enabled */
	BOOLEAN bShowHiddenSSID;	/* Show all known SSID in SSID list get operation */

	/* New for WPA, windows want us to to keep association information and */
	/* Fixed IEs from last association response */
	NDIS_802_11_ASSOCIATION_INFORMATION AssocInfo;
	USHORT ReqVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ReqVarIEs[MAX_VIE_LEN];	/* The content saved here should be little-endian format. */
	USHORT ResVarIELen;	/* Length of next VIE include EID & Length */
	UCHAR ResVarIEs[MAX_VIE_LEN];

	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be little-endian format. */

	ULONG CLBusyBytes;	/* Save the total bytes received durning channel load scan time */
	USHORT RPIDensity[8];	/* Array for RPI density collection */

	UCHAR RMReqCnt;		/* Number of measurement request saved. */
	UCHAR CurrentRMReqIdx;	/* Number of measurement request saved. */
	BOOLEAN ParallelReq;	/* Parallel measurement, only one request performed, */
	/* It must be the same channel with maximum duration */
	USHORT ParallelDuration;	/* Maximum duration for parallel measurement */
	UCHAR ParallelChannel;	/* Only one channel with parallel measurement */
	USHORT IAPPToken;	/* IAPP dialog token */
	/* Hack for channel load and noise histogram parameters */
	UCHAR NHFactor;		/* Parameter for Noise histogram */
	UCHAR CLFactor;		/* Parameter for channel load */

	RALINK_TIMER_STRUCT StaQuickResponeForRateUpTimer;
	BOOLEAN StaQuickResponeForRateUpTimerRunning;

	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */

#ifdef QOS_DLS_SUPPORT
	RT_802_11_DLS DLSEntry[MAX_NUM_OF_DLS_ENTRY];
	UCHAR DlsReplayCounter[8];
#endif				/* QOS_DLS_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
	TDLS_STRUCT TdlsInfo;
#endif				/* DOT11Z_TDLS_SUPPORT */

	RALINK_TIMER_STRUCT WpaDisassocAndBlockAssocTimer;
	/* Fast Roaming */
	BOOLEAN bAutoRoaming;	/* 0:disable auto roaming by RSSI, 1:enable auto roaming by RSSI */
	CHAR dBmToRoam;		/* the condition to roam when receiving Rssi less than this value. It's negative value. */

#ifdef WPA_SUPPLICANT_SUPPORT
	BOOLEAN IEEE8021X;
	BOOLEAN IEEE8021x_required_keys;
	CIPHER_KEY DesireSharedKey[4];	/* Record user desired WEP keys */
	UCHAR DesireSharedKeyId;

	/* 0x00: driver ignores wpa_supplicant */
	/* 0x01: wpa_supplicant initiates scanning and AP selection */
	/* 0x02: driver takes care of scanning, AP selection, and IEEE 802.11 association parameters */
	/* 0x80: wpa_supplicant trigger driver to do WPS */
	UCHAR WpaSupplicantUP;
	UCHAR WpaSupplicantScanCount;
	BOOLEAN bRSN_IE_FromWpaSupplicant;
	BOOLEAN bLostAp;
	UCHAR *pWpsProbeReqIe;
	UINT WpsProbeReqIeLen;
	UCHAR *pWpaAssocIe;
	UINT WpaAssocIeLen;
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_STA_SUPPORT
	WSC_LV_INFO WpsIEBeacon;
	WSC_LV_INFO WpsIEProbeResp;
	WSC_CTRL WscControl;
#ifdef IWSC_SUPPORT
	IWSC_INFO			IWscInfo;
#endif /* IWSC_SUPPORT */
#endif /* WSC_STA_SUPPORT */
	CHAR dev_name[16];
	USHORT OriDevType;

	BOOLEAN bTGnWifiTest;
	BOOLEAN bSkipAutoScanConn;

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;
	RT_HT_PHY_INFO DesiredHtPhyInfo;
	BOOLEAN bAutoTxRateSwitch;

#ifdef RTMP_MAC_PCI
	UCHAR BBPR3;

	/* PS Control has 2 meanings for advanced power save function. */
	/* 1. EnablePSinIdle : When no connection, always radio off except need to do site survey. */
	/* 2. EnableNewPS  : will save more current in sleep or radio off mode. */
	PS_CONTROL PSControl;
#endif				/* RTMP_MAC_PCI */

#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR IEEE80211dClientMode;
	UCHAR StaOriCountryCode[3];
	UCHAR StaOriGeography;
#endif				/* EXT_BUILD_CHANNEL_LIST */





	BOOLEAN bAutoConnectByBssid;
	ULONG BeaconLostTime;	/* seconds */
	BOOLEAN bForceTxBurst;	/* 1: force enble TX PACKET BURST, 0: disable */
#ifdef XLINK_SUPPORT
	BOOLEAN PSPXlink;	/* 0: Disable. 1: Enable */
#endif /* XLINK_SUPPORT */
	BOOLEAN bAutoConnectIfNoSSID;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	UCHAR RegClass;		/*IE_SUPP_REG_CLASS: 2009 PF#3: For 20/40 Intolerant Channel Report */
#endif /* DOT11N_DRAFT3 */
	BOOLEAN bAdhocN;
#endif /* DOT11_N_SUPPORT */
	BOOLEAN bAdhocCreator;	/*TRUE indicates divice is Creator. */


	/*
	   Enhancement Scanning Mechanism
	   To decrease the possibility of ping loss
	 */
	BOOLEAN bImprovedScan;
	BOOLEAN BssNr;
	UCHAR ScanChannelCnt;	/* 0 at the beginning of scan, stop at 7 */
	UCHAR LastScanChannel;
	/************************************/

	BOOLEAN bFastConnect;

/*connectinfo  for tmp store connect info from UI*/
	BOOLEAN Connectinfoflag;
	UCHAR   ConnectinfoBssid[MAC_ADDR_LEN];
	UCHAR   ConnectinfoChannel;
	UCHAR   ConnectinfoSsidLen;
	CHAR    ConnectinfoSsid[MAX_LEN_OF_SSID];
	UCHAR ConnectinfoBssType;
	



	/* UAPSD information: such as enable or disable, do not remove */
	UAPSD_INFO UapsdInfo;

} STA_ADMIN_CONFIG, *PSTA_ADMIN_CONFIG;

/* This data structure keep the current active BSS/IBSS's configuration that this STA */
/* had agreed upon joining the network. Which means these parameters are usually decided */
/* by the BSS/IBSS creator instead of user configuration. Data in this data structurre */
/* is valid only when either ADHOC_ON(pAd) or INFRA_ON(pAd) is TRUE. */
/* Normally, after SCAN or failed roaming attempts, we need to recover back to */
/* the current active settings. */
typedef struct _STA_ACTIVE_CONFIG {
	USHORT Aid;
	USHORT AtimWin;		/* in kusec; IBSS parameter set element */
	USHORT CapabilityInfo;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	USHORT CfpMaxDuration;
	USHORT CfpPeriod;

	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRateLen;
	/* Copy supported ht from desired AP's beacon. We are trying to match */
	RT_HT_PHY_INFO SupportedPhyInfo;
	RT_HT_CAPABILITY SupportedHtPhy;
} STA_ACTIVE_CONFIG, *PSTA_ACTIVE_CONFIG;



#endif /* CONFIG_STA_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
/* Video Mode related configuration */
typedef struct _AP_VIDEO_CONFIG {
	BOOLEAN Enable;
	BOOLEAN ClassifierEnable;
	BOOLEAN HighTxMode;
	UCHAR TxPwr;
	BOOLEAN VideoMCSEnable;
	UCHAR VideoMCS;
	UCHAR TxBASize;
	BOOLEAN TxLifeTimeMode;	/* 1: Packet Life Time mode, 0: Retry Limit mode */
	UCHAR TxLifeTime;
	UINT16 TxRetryLimit;
} AP_VIDEO_STRUCT, *PAP_VIDEO_STRUCT;

#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP related data structures
  **************************************************************************/
/* AUTH-RSP State Machine Aux data structure */
typedef struct _AP_MLME_AUX {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT Alg;
	CHAR Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;

typedef enum _MAC_ENTRY_OP_MODE_ {
	ENTRY_OP_MODE_ERROR = 0x00,
	ENTRY_OP_MODE_CLI = 0x01,	/* Sta mode, set this TRUE after Linkup,too. */
	ENTRY_OP_MODE_WDS = 0x02,	/* This is WDS Entry. only for AP mode. */
	ENTRY_OP_MODE_APCLI = 0x04,	/* This is a AP-Client entry, only for AP mode which enable AP-Client functions. */
	ENTRY_OP_MODE_MESH = 0x08,	/* Peer conect with us via mesh. */
	ENTRY_OP_MODE_DLS = 0x10,	/* This is DLS Entry. only for STA mode. */
	ENTRY_OP_MODE_MAX = 0x20
} MAC_ENTRY_OP_MODE;
#endif /* CONFIG_AP_SUPPORT */

/* Values of LastSecTxRateChangeAction */
#define RATE_NO_CHANGE	0		/* No change in rate */
#define RATE_UP			1		/* Trying higher rate or same rate with different BF */
#define RATE_DOWN		2		/* Trying lower rate */

#ifdef ADHOC_WPA2PSK_SUPPORT
typedef struct _FOUR_WAY_HANDSHAKE_PROFILE {
	UCHAR ANonce[LEN_KEY_DESC_NONCE];
	UCHAR SNonce[LEN_KEY_DESC_NONCE];
	UCHAR ReplayCounter[LEN_KEY_DESC_REPLAY];
	UCHAR PTK[64];
	UINT8 WpaState;
	UCHAR MsgType;
	RALINK_TIMER_STRUCT MsgRetryTimer;
	UCHAR MsgRetryCounter;
} FOUR_WAY_HANDSHAKE_PROFILE, *PFOUR_WAY_HANDSHAKE_PROFILE;
#endif /* ADHOC_WPA2PSK_SUPPORT */

typedef struct _MAC_TABLE_ENTRY {
	/*
	   0:Invalid,
	   Bit 0: AsCli, Bit 1: AsWds, Bit 2: AsAPCLI,
	   Bit 3: AsMesh, Bit 4: AsDls, Bit 5: AsTDls
	 */
	UINT32 EntryType;
#ifdef P2P_SUPPORT
	UINT32 P2PEntryType; /* 0:Invalid, 1:P2P_GO, 2:P2P_CLI */
#endif /* P2P_SUPPORT */
	BOOLEAN isCached;
	BOOLEAN bIAmBadAtheros;	/* Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection. */

	/* WPA/WPA2 4-way database */
	UCHAR EnqueueEapolStartTimerRunning;	/* Enqueue EAPoL-Start for triggering EAP SM */
	RALINK_TIMER_STRUCT EnqueueStartForPSKTimer;	/* A timer which enqueue EAPoL-Start for triggering PSK SM */
#ifdef ADHOC_WPA2PSK_SUPPORT
	FOUR_WAY_HANDSHAKE_PROFILE WPA_Supplicant;
	FOUR_WAY_HANDSHAKE_PROFILE WPA_Authenticator;
	CIPHER_KEY RxGTK;
	BOOLEAN bPeerHigherMAC;
#ifdef IWSC_SUPPORT
	BOOLEAN bUpdateInfoFromPeerBeacon;
#endif /* IWSC_SUPPORT */
#endif				/* ADHOC_WPA2PSK_SUPPORT */

#ifdef DROP_MASK_SUPPORT
	RALINK_TIMER_STRUCT	dropmask_timer;
#endif /* DROP_MASK_SUPPORT */

	/*jan for wpa */
	/* record which entry revoke MIC Failure , if it leaves the BSS itself, AP won't update aMICFailTime MIB */
	UCHAR CMTimerRunning;
	UCHAR apidx;		/* MBSS number */
	UCHAR RSNIE_Len;
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR ANonce[LEN_KEY_DESC_NONCE];
	UCHAR SNonce[LEN_KEY_DESC_NONCE];
	UCHAR R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR PTK[64];
	UCHAR ReTryCounter;
	BOOLEAN AllowInsPTK;
	UCHAR LastGroupKeyId;
	UCHAR LastGTK[MAX_LEN_GTK];
	RALINK_TIMER_STRUCT RetryTimer;
	RALINK_TIMER_STRUCT Start2WayGroupHSTimer;
#ifdef TXBF_SUPPORT
	RALINK_TIMER_STRUCT eTxBfProbeTimer;
#endif /* TXBF_SUPPORT */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_802_11_WEP_STATUS GroupKeyWepStatus;
	UINT8 WpaState;
	UINT8 GTKState;
	USHORT PortSecured;
	NDIS_802_11_PRIVACY_FILTER PrivacyFilter;	/* PrivacyFilter enum for 802.1X */
	CIPHER_KEY PairwiseKey;
	PVOID pAd;
	INT PMKID_CacheIdx;
	UCHAR PMKID[LEN_PMKID];
	UCHAR NegotiatedAKM[LEN_OUI_SUITE];	/* It indicate the negotiated AKM suite */

#ifdef WAPI_SUPPORT
	UCHAR usk_id;		/* unicast key index for WPI */
#endif				/* WAPI_SUPPORT */

	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR			HdrAddr1[MAC_ADDR_LEN];
	UCHAR			HdrAddr2[MAC_ADDR_LEN];
	UCHAR			HdrAddr3[MAC_ADDR_LEN];
	UCHAR PsMode;
	UCHAR FlgPsModeIsWakeForAWhile; /* wake up for a while until a condition */
	UCHAR VirtualTimeout; /* peer power save virtual timeout */
	SST Sst;
	AUTH_STATE AuthState;	/* for SHARED KEY authentication state machine used only */
	BOOLEAN IsReassocSta;	/* Indicate whether this is a reassociation procedure */
	USHORT Aid;
	USHORT CapabilityInfo;
	UCHAR LastRssi;
	ULONG NoDataIdleCount;
	UINT16 StationKeepAliveCount;	/* unit: second */
	ULONG PsQIdleCount;
	QUEUE_HEADER PsQueue;

	UINT32 StaConnectTime;	/* the live time of this station since associated with AP */
	UINT32 StaIdleTimeout;	/* idle timeout per entry */

#ifdef UAPSD_SUPPORT
	/* these UAPSD states are used on the fly */
	/* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	BOOLEAN bAPSDCapablePerAC[4];	/* for trigger-enabled */
	BOOLEAN bAPSDDeliverEnabledPerAC[4];	/* for delivery-enabled */


	UCHAR MaxSPLength;

	BOOLEAN bAPSDAllAC;	/* 1: all AC are delivery-enabled U-APSD */

	QUEUE_HEADER UAPSDQueue[WMM_NUM_OF_AC];	/* queue for each U-APSD */
	USHORT UAPSDQIdleCount;	/* U-APSD queue timeout */

	PQUEUE_ENTRY pUAPSDEOSPFrame;	/* the last U-APSD frame */
	USHORT UAPSDTxNum;	/* total U-APSD frame number */
	BOOLEAN bAPSDFlagEOSPOK;	/* 1: EOSP frame is tx by ASIC */
	BOOLEAN bAPSDFlagSPStart;	/* 1: SP is started */

	/* need to use unsigned long, because time parameters in OS is defined as
	   unsigned long */
	unsigned long UAPSDTimeStampLast;	/* unit: 1000000/OS_HZ */
	BOOLEAN bAPSDFlagSpRoughUse;	/* 1: use rough SP (default: accurate) */

	/* we will set the flag when PS-poll frame is received and
	   clear it when statistics handle.
	   if the flag is set when PS-poll frame is received then calling
	   statistics handler to clear it. */
	BOOLEAN bAPSDFlagLegacySent;	/* 1: Legacy PS sent but
					   yet statistics handle */

#endif /* UAPSD_SUPPORT */

#ifdef DOT11_N_SUPPORT
	BOOLEAN bSendBAR;
	USHORT NoBADataCountDown;

	UINT32 CachedBuf[16];	/* UINT (4 bytes) for alignment */

#ifdef TXBF_SUPPORT
	COUNTER_TXBF TxBFCounters;		/* TxBF Statistics */
	UINT LastETxCount;		/* Used to compute %BF statistics */
	UINT LastITxCount;
	UINT LastTxCount;
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	UINT32 StreamModeMACReg;	/* MAC reg used to control stream mode for this client. 0=>No stream mode */
#endif // STREAM_MODE_SUPPORT //

	UINT FIFOCount;
	UINT DebugFIFOCount;
	UINT DebugTxCount;
	BOOLEAN bDlsInit;

/*==================================================== */
/*WDS entry needs these */
/* if ValidAsWDS==TRUE, MatchWDSTabIdx is the index in WdsTab.MacTab */
	UINT MatchWDSTabIdx;
	UCHAR MaxSupportedRate;
	UCHAR CurrTxRate;
	UCHAR CurrTxRateIndex;
	UCHAR lastRateIdx;
	UCHAR *pTable;	/* Pointer to this entry's Tx Rate Table */

#ifdef NEW_RATE_ADAPT_SUPPORT
	UCHAR lowTrafficCount;
	UCHAR fewPktsCnt;
	BOOLEAN perThrdAdj;
	UCHAR mcsGroup;/*the mcs group to be tried */
#endif /* NEW_RATE_ADAPT_SUPPORT */
	enum RATE_ADAPT_ALG rateAlg;

#ifdef PEER_DELBA_TX_ADAPT
	BOOLEAN bPeerDelBaTxAdaptEn;
	RALINK_TIMER_STRUCT DelBA_Tx_AdaptTimer;
#endif /* PEER_DELBA_TX_ADAPT */

#ifdef MFB_SUPPORT
	UCHAR lastLegalMfb;	/*last legal mfb which is used to set rate */
	BOOLEAN isMfbChanged;	/*purpose: true when mfb has changed but the new mfb is not adopted for Tx */
	struct _RTMP_TX_RATE_SWITCH *LegalMfbRS;
	BOOLEAN fLastChangeAccordingMfb;
	NDIS_SPIN_LOCK fLastChangeAccordingMfbLock;
/*Tx MRQ */
	BOOLEAN toTxMrq;
	UCHAR msiToTx, mrqCnt;	/*mrqCnt is used to count down the inverted-BF mrq to be sent */
/*Rx mfb */
	UCHAR pendingMfsi;
/*Tx MFB */
	BOOLEAN toTxMfb;
	UCHAR	mfbToTx;
	UCHAR	mfb0, mfb1;
#endif	/* MFB_SUPPORT */
#ifdef TXBF_SUPPORT
	UCHAR			TxSndgType;
	NDIS_SPIN_LOCK	TxSndgLock;

/* ETxBF */
	UCHAR		bfState;
	UCHAR		sndgMcs;
	UCHAR		sndgBW;
	INT			sndg0Snr0, sndg0Snr1, sndg0Snr2;
	UCHAR		sndg0Mcs;
#ifdef ETXBF_EN_COND3_SUPPORT
	UCHAR		bestMethod;
	UCHAR		sndgRateIdx;
	UCHAR		bf0Mcs, sndg0RateIdx, bf0RateIdx;
	UCHAR		sndg1Mcs, bf1Mcs, sndg1RateIdx, bf1RateIdx;
	INT			sndg1Snr0, sndg1Snr1, sndg1Snr2;
#endif /* ETXBF_EN_COND3_SUPPORT */
	UCHAR		noSndgCnt;
	UCHAR		eTxBfEnCond;
	UCHAR		noSndgCntThrd, ndpSndgStreams;
	UCHAR		iTxBfEn;

	BOOLEAN		phyETxBf;			/* True=>Set ETxBF bit in PHY rate */
	BOOLEAN		phyITxBf;			/* True=>Set ITxBF bit in PHY rate */
	UCHAR		lastNonBfRate;		/* Last good non-BF rate */
	BOOLEAN		lastRatePhyTxBf;	/* For Quick Check. True if last rate was BF */
	USHORT      BfTxQuality[MAX_TX_RATE_INDEX + 1];	// Beamformed TX Quality
#endif /* TXBF_SUPPORT */

	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	USHORT      TxQuality[MAX_TX_RATE_INDEX + 1];
	UINT32		OneSecTxNoRetryOkCount;
	UINT32      OneSecTxRetryOkCount;
	UINT32      OneSecTxFailCount;
	UINT32      OneSecRxLGICount;		// unicast-to-me Long GI count
	UINT32      OneSecRxSGICount;      	// unicast-to-me Short GI count

#ifdef FIFO_EXT_SUPPORT
	UINT32		fifoTxSucCnt;
	UINT32		fifoTxRtyCnt;
#endif /* FIFO_EXT_SUPPORT */


	BOOLEAN fLastSecAccordingRSSI;
	UCHAR LastSecTxRateChangeAction;	/* 0: no change, 1:rate UP, 2:rate down */
	CHAR LastTimeTxRateChangeAction;	/*Keep last time value of LastSecTxRateChangeAction */
	ULONG LastTxOkCount; /* TxSuccess count in last Rate Adaptation interval */
	UCHAR LastTxPER;	/* Tx PER in last Rate Adaptation interval */
	UCHAR PER[MAX_TX_RATE_INDEX + 1];

	UINT32 ContinueTxFailCnt;
	UINT32 CurrTxRateStableTime;	/* # of second in current TX rate */
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
#ifdef WDS_SUPPORT
	BOOLEAN LockEntryTx;	/* TRUE = block to WDS Entry traffic, FALSE = not. */
#endif				/* WDS_SUPPORT */
	ULONG TimeStamp_toTxRing;

/*==================================================== */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT MatchAPCLITabIdx;	/* indicate the index in ApCfg.ApCliTab. */
#endif				/* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bReptCli;
	BOOLEAN bReptEthCli;
	BOOLEAN bReptBridgeCli;
	UCHAR MatchReptCliIdx;
	UCHAR ReptCliAddr[MAC_ADDR_LEN];
	ULONG ReptCliIdleCount;
#endif /* MAC_REPEATER_SUPPORT */
#endif				/* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
	UINT MatchDlsEntryIdx;	/* indicate the index in pAd->StaCfg.DLSEntry */
#endif				/* QOS_DLS_SUPPORT */
#endif				/* CONFIG_STA_SUPPORT */

	/* a bitmap of BOOLEAN flags. each bit represent an operation status of a particular */
	/* BOOLEAN control, either ON or OFF. These flags should always be accessed via */
	/* CLIENT_STATUS_TEST_FLAG(), CLIENT_STATUS_SET_FLAG(), CLIENT_STATUS_CLEAR_FLAG() macros. */
	/* see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition. fCLIENT_STATUS_AMSDU_INUSED */
	ULONG ClientStatusFlags;

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;	/* For transmit phy setting in TXWI. */

#ifdef DOT11_N_SUPPORT
	/* HT EWC MIMO-N used parameters */
	USHORT RXBAbitmap;	/* fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format */
	USHORT TXBAbitmap;	/* This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI */
	USHORT TXAutoBAbitmap;
	USHORT BADeclineBitmap;
	USHORT BARecWcidArray[NUM_OF_TID];	/* The mapping wcid of recipient session. if RXBAbitmap bit is masked */
	USHORT BAOriWcidArray[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */
	USHORT BAOriSequence[NUM_OF_TID];	/* The mapping wcid of originator session. if TXBAbitmap bit is masked */

	/* 802.11n features. */
	UCHAR MpduDensity;
	UCHAR MaxRAmpduFactor;
	UCHAR AMsduSize;
	UCHAR MmpsMode;		/* MIMO power save more. */

	HT_CAPABILITY_IE HTCapability;

#ifdef DOT11N_DRAFT3
	UCHAR BSS2040CoexistenceMgmtSupport;
	BOOLEAN bForty_Mhz_Intolerant;
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


	BOOLEAN bAutoTxRateSwitch;

	UCHAR RateLen;
	struct _MAC_TABLE_ENTRY *pNext;
	USHORT TxSeq[NUM_OF_TID];
	USHORT NonQosDataSeq;
	INT TxBarSeq[NUM_OF_TID];

	RSSI_SAMPLE RssiSample;
	UINT32 LastRxRate;
	SHORT freqOffset;		/* Last RXWI FOFFSET */
	SHORT freqOffsetValid;	/* Set when freqOffset field has been updated */

#if defined (RT2883) || defined (RT3883)
	INT32 BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
#endif /* defined (RT2883) || defined (RT3883) */

	BOOLEAN bWscCapable;
	UCHAR Receive_EapolStart_EapRspId;

	UINT32 TXMCSExpected[MAX_MCS_SET];
	UINT32 TXMCSSuccessful[MAX_MCS_SET];
	UINT32 TXMCSFailed[MAX_MCS_SET];
	UINT32 TXMCSAutoFallBack[MAX_MCS_SET][MAX_MCS_SET];

#ifdef CONFIG_STA_SUPPORT
	ULONG LastBeaconRxTime;
#endif				/* CONFIG_STA_SUPPORT */

#ifdef WAPI_SUPPORT
	BOOLEAN WapiUskRekeyTimerRunning;
	RALINK_TIMER_STRUCT WapiUskRekeyTimer;
	UINT32 wapi_usk_rekey_cnt;
#endif				/* WAPI_SUPPORT */


	ULONG AssocDeadLine;




	ULONG ChannelQuality;	/* 0..100, Channel Quality Indication for Roaming */


#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif				/* CONFIG_AP_SUPPORT */
#ifdef VENDOR_FEATURE1_SUPPORT
	/* total 128B, use UINT32 to avoid alignment problem */
	UINT32 HeaderBuf[32];	/* (total 128B) TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */

	UCHAR HdrPadLen;	/* recording Header Padding Length; */
	UCHAR MpduHeaderLen;
	UINT16 Protocol;
#endif				/* VENDOR_FEATURE1_SUPPORT */

#ifdef AGS_SUPPORT
	AGS_CONTROL AGSCtrl;	/* AGS control */
#endif				/* AGS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	LARGE_INTEGER TxPackets;
	LARGE_INTEGER RxPackets;
	ULONG TxBytes;
	ULONG RxBytes;
#endif				/* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT
	P2P_ENTRY_PARM		P2pInfo;
	BOOLEAN bP2pClient;
#endif /* P2P_SUPPORT */
#ifdef IWSC_SUPPORT
	BOOLEAN			bIWscSmpbcAccept;
#endif // IWSC_SUPPORT //

UCHAR	SupportRateMode; /* 1: CCK 2:OFDM 4: HT */
BOOLEAN SupportCCKMCS[MAX_LEN_OF_CCK_RATES];
BOOLEAN SupportOFDMMCS[MAX_LEN_OF_OFDM_RATES];
BOOLEAN SupportHTMCS[MAX_LEN_OF_HT_RATES];

#ifdef DOT11Z_TDLS_SUPPORT
	UINT16			TdlsTxFailCount;
	UINT32			TdlsKeyLifeTimeCount;
	UCHAR			MatchTdlsEntryIdx; // indicate the index in pAd->StaCfg.DLSEntry
#endif // DOT11Z_TDLS_SUPPORT //
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

typedef struct _MAC_TABLE {
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];
#ifdef MAC_REPEATER_SUPPORT
	MAC_TABLE_ENTRY Content[MAX_MAC_TABLE_SIZE_WITH_REPEATER];
#else
	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
#endif /* MAC_REPEATER_SUPPORT */
	USHORT Size;
	QUEUE_HEADER McastPsQueue;
	ULONG PsQIdleCount;
	BOOLEAN fAnyStationInPsm;
	BOOLEAN fAnyStationBadAtheros;	/* Check if any Station is atheros 802.11n Chip.  We need to use RTS/CTS with Atheros 802,.11n chip. */
	BOOLEAN fAnyTxOPForceDisable;	/* Check if it is necessary to disable BE TxOP */
	BOOLEAN fAllStationAsRalink;	/* Check if all stations are ralink-chipset */
#ifdef DOT11_N_SUPPORT
	BOOLEAN fAnyStationIsLegacy;	/* Check if I use legacy rate to transmit to my BSS Station/ */
	BOOLEAN fAnyStationNonGF;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStation20Only;	/* Check if any Station can't support GF. */
	BOOLEAN fAnyStationMIMOPSDynamic;	/* Check if any Station is MIMO Dynamic */
	BOOLEAN fAnyBASession;	/* Check if there is BA session.  Force turn on RTS/CTS */
#ifdef DOT11N_DRAFT3
	BOOLEAN fAnyStaFortyIntolerant;	/* Check if still has any station set the Intolerant bit on! */
#endif /* DOT11N_DRAFT3 */
	BOOLEAN fAllStationGainGoodMCS; /* Check if all stations more than MCS threshold */

#ifdef CONFIG_AP_SUPPORT
	BOOLEAN fAnyStationIsHT;	/* Check if there is 11n STA.  Force turn off AP MIMO PS */
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	USHORT MsduLifeTime; /* life time for PS packet */

#ifdef WAPI_SUPPORT
	BOOLEAN fAnyWapiStation;
#endif /* WAPI_SUPPORT */

#ifdef PEER_DELBA_TX_ADAPT
	UCHAR DelBA_Client_Count;
#endif /* PEER_DELBA_TX_ADAPT */
} MAC_TABLE, *PMAC_TABLE;



#ifdef CONFIG_AP_SUPPORT
/***************************************************************************
  *	AP WDS related data structures
  **************************************************************************/
#ifdef WDS_SUPPORT
typedef struct _WDS_COUNTER {
	LARGE_INTEGER ReceivedByteCount;
	LARGE_INTEGER TransmittedByteCount;
	ULONG ReceivedFragmentCount;
	ULONG TransmittedFragmentCount;
	ULONG RxErrors;
	ULONG MulticastReceivedFrameCount;
} WDS_COUNTER, *PWDS_COUNTER;

typedef struct _WDS_ENTRY {
	BOOLEAN Valid;
	UCHAR Addr[MAC_ADDR_LEN];
	ULONG NoDataIdleCount;
	struct _WDS_ENTRY *pNext;
} WDS_ENTRY, *PWDS_ENTRY;

typedef struct _WDS_TABLE_ENTRY {
	USHORT Size;
	UCHAR WdsAddr[MAC_ADDR_LEN];
	WDS_ENTRY *Hash[HASH_TABLE_SIZE];
	WDS_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	UCHAR MaxSupportedRate;
	UCHAR CurrTxRate;
	USHORT TxQuality[MAX_LEN_OF_SUPPORTED_RATES];
	USHORT OneSecTxOkCount;
	USHORT OneSecTxRetryOkCount;
	USHORT OneSecTxFailCount;
	ULONG CurrTxRateStableTime;	/* # of second in current TX rate */
	UCHAR TxRateUpPenalty;	/* extra # of second penalty due to last unstable condition */
} WDS_TABLE_ENTRY, *PWDS_TABLE_ENTRY;

typedef struct _RT_802_11_WDS_ENTRY {
	PNET_DEV dev;
	UCHAR Valid;
	UCHAR PhyMode;
	UCHAR PeerWdsAddr[MAC_ADDR_LEN];
	UCHAR MacTabMatchWCID;	/* ASIC */
	NDIS_802_11_WEP_STATUS WepStatus;
	UCHAR KeyIdx;
	CIPHER_KEY WdsKey;
	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	RT_HT_PHY_INFO DesiredHtPhyInfo;
	BOOLEAN bAutoTxRateSwitch;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;	/* Desired transmit setting. */
	WDS_COUNTER WdsCounter;

#ifdef WDS_VLAN_SUPPORT
	/* VLAN */
    USHORT				VLAN_VID;
    USHORT				VLAN_Priority;
#endif /* WDS_VLAN_SUPPORT */
} RT_802_11_WDS_ENTRY, *PRT_802_11_WDS_ENTRY;

typedef struct _WDS_TABLE {
	UCHAR Mode;
	ULONG Size;
	RT_802_11_WDS_ENTRY WdsEntry[MAX_WDS_ENTRY];
} WDS_TABLE, *PWDS_TABLE;
#endif /* WDS_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
typedef struct _REPEATER_CLIENT_ENTRY {
	//BOOLEAN bValid;
	BOOLEAN CliEnable;
	BOOLEAN CliValid;
	BOOLEAN bEthCli;
	UCHAR MatchApCliIdx;
	UCHAR MatchLinkIdx;
	UCHAR MacTabWCID;
	UCHAR CliConnectState; /* 0: disconnect 1: connecting 2: connected */

	ULONG CtrlCurrState;
	ULONG SyncCurrState;
	ULONG AuthCurrState;
	ULONG AssocCurrState;

	RALINK_TIMER_STRUCT ApCliAssocTimer, ApCliAuthTimer;
	RALINK_TIMER_STRUCT ReptCliResetEntryTimer;

	USHORT AuthReqCnt;
	USHORT AssocReqCnt;
	ULONG CliTriggerTime;

	/* For WPA countermeasures */
	ULONG LastMicErrorTime; /* record last MIC error time */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */

	UCHAR OriginalAddress[ETH_LENGTH_OF_ADDRESS];
	UCHAR CurrentAddress[ETH_LENGTH_OF_ADDRESS];
	PVOID pAd;
	struct _REPEATER_CLIENT_ENTRY *pNext;
} REPEATER_CLIENT_ENTRY, *PREPEATER_CLIENT_ENTRY;

typedef struct _REPEATER_CLIENT_ENTRY_MAP {
	PREPEATER_CLIENT_ENTRY pReptCliEntry;
	struct _REPEATER_CLIENT_ENTRY_MAP *pNext;
} REPEATER_CLIENT_ENTRY_MAP, *PREPEATER_CLIENT_ENTRY_MAP;

typedef struct _INVAILD_TRIGGER_MAC_ENTRY {
	UCHAR MacAddr[MAC_ADDR_LEN];
	UCHAR entry_idx;
	BOOLEAN bInsert;
	struct _INVAILD_TRIGGER_MAC_ENTRY *pNext;
} INVAILD_TRIGGER_MAC_ENTRY, *PINVAILD_TRIGGER_MAC_ENTRY;

typedef struct _REPEATER_CTRL_STRUCT {
	INVAILD_TRIGGER_MAC_ENTRY RepeaterInvaildEntry[32];
	INVAILD_TRIGGER_MAC_ENTRY *ReptInvaildHash[HASH_TABLE_SIZE];
	UCHAR ReptInVaildMacSize;
} REPEATER_CTRL_STRUCT, *PREPEATER_CTRL_STRUCT;
#endif /* MAC_REPEATER_SUPPORT */

/***************************************************************************
  *	AP APCLI related data structures
  **************************************************************************/
typedef struct _APCLI_COUNTER {
	LARGE_INTEGER ReceivedByteCount;
	LARGE_INTEGER TransmittedByteCount;
	ULONG ReceivedFragmentCount;
	ULONG TransmittedFragmentCount;
	ULONG RxErrors;
	ULONG MulticastReceivedFrameCount;
} APCLI_COUNTER, *PAPCLI_COUNTER;

typedef struct _APCLI_STRUCT {
	PNET_DEV dev;
#ifdef RTL865X_SOC
	unsigned int mylinkid;
#endif
	BOOLEAN Enable;		/* Set it as 1 if the apcli interface was configured to "1"  or by iwpriv cmd "ApCliEnable" */
	BOOLEAN Valid;		/* Set it as 1 if the apcli interface associated success to remote AP. */

	MLME_AUX ApCliMlmeAux;			/* temporary settings used during MLME state machine */

	UCHAR MacTabWCID;	/*WCID value, which point to the entry of ASIC Mac table. */
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];

	UCHAR CfgSsidLen;
	CHAR CfgSsid[MAX_LEN_OF_SSID];
	UCHAR CfgApCliBssid[ETH_LENGTH_OF_ADDRESS];
	UCHAR CurrentAddress[ETH_LENGTH_OF_ADDRESS];

	ULONG ApCliRcvBeaconTime;
	ULONG ApCliLinkUpTime;
	USHORT ApCliBeaconPeriod;

	ULONG CtrlCurrState;
	ULONG SyncCurrState;
	ULONG AuthCurrState;
	ULONG AssocCurrState;
	ULONG WpaPskCurrState;

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT	ProbeReqCnt;
	BOOLEAN AutoConnectFlag;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	USHORT AuthReqCnt;
	USHORT AssocReqCnt;

	ULONG ClientStatusFlags;
	UCHAR MpduDensity;

	NDIS_802_11_AUTHENTICATION_MODE AuthMode;	/* This should match to whatever microsoft defined */
	NDIS_802_11_WEP_STATUS WepStatus;

	/* Add to support different cipher suite for WPA2/WPA mode */
	NDIS_802_11_ENCRYPTION_STATUS GroupCipher;	/* Multicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS PairCipher;	/* Unicast cipher suite */
	BOOLEAN bMixCipher;	/* Indicate current Pair & Group use different cipher suites */
	USHORT RsnCapability;

	UCHAR PSK[100];		/* reserve PSK key material */
	UCHAR PSKLen;
	UCHAR PMK[32];		/* WPA PSK mode PMK */
	UCHAR PTK[64];                /* WPA PSK mode PTK */
	UCHAR GTK[32];		/* GTK from authenticator */

	/*CIPHER_KEY            PairwiseKey; */
	CIPHER_KEY SharedKey[SHARE_KEY_NUM];
	UCHAR DefaultKeyId;

	/* WPA 802.1x port control, WPA_802_1X_PORT_SECURED, WPA_802_1X_PORT_NOT_SECURED */
	/*UCHAR       PortSecured; */

	/* store RSN_IE built by driver */
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];	/* The content saved here should be convert to little-endian format. */
	UCHAR RSNIE_Len;

	/* For WPA countermeasures */
	ULONG LastMicErrorTime;	/* record last MIC error time */
	ULONG       MicErrCnt;          /* Should be 0, 1, 2, then reset to zero (after disassoiciation). */
	BOOLEAN bBlockAssoc;	/* Block associate attempt for 60 seconds after counter measure occurred. */

	/* For WPA-PSK supplicant state */
	/*WPA_STATE     WpaState;           // Default is SS_NOTUSE */
	UCHAR         ReplayCounter[8];
	/*UCHAR         ANonce[32];         // ANonce for WPA-PSK from authenticator */
	UCHAR SNonce[32];	/* SNonce for WPA-PSK */
	UCHAR GNonce[32];	/* GNonce for WPA-PSK from authenticator */

#ifdef WSC_AP_SUPPORT
	WSC_CTRL WscControl;
#endif /* WSC_AP_SUPPORT */

	HTTRANSMIT_SETTING HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	RT_HT_PHY_INFO DesiredHtPhyInfo;
	BOOLEAN bAutoTxRateSwitch;
	DESIRED_TRANSMIT_SETTING DesiredTransmitSetting;	/* Desired transmit setting. */
	UCHAR RxMcsSet[16];
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
    	BOOLEAN             IEEE8021X;
  	BOOLEAN             IEEE8021x_required_keys;
    	CIPHER_KEY	        DesireSharedKey[4];	// Record user desired WEP keys	
    	UCHAR               DesireSharedKeyId;
    	UCHAR               WpaSupplicantUP;
	UCHAR				WpaSupplicantScanCount;
	BOOLEAN				bRSN_IE_FromWpaSupplicant;
	BOOLEAN				bLostAp;
	UCHAR				*pWpsProbeReqIe;
	UINT				WpsProbeReqIeLen;
	UCHAR				*pWpaAssocIe;
	UINT				WpaAssocIeLen;
	BOOLEAN			    bScanReqIsFromWebUI;
	BSSID_INFO	SavedPMK[PMKID_NO];
	UINT		SavedPMKNum;			// Saved PMKID number
	BOOLEAN		bConfigChanged;
	NDIS_802_11_ASSOCIATION_INFORMATION     AssocInfo;
	USHORT       ReqVarIELen;                // Length of next VIE include EID & Length
	UCHAR       ReqVarIEs[MAX_VIE_LEN];		// The content saved here should be little-endian format.
	USHORT       ResVarIELen;                // Length of next VIE include EID & Length
	UCHAR       ResVarIEs[MAX_VIE_LEN];
	UCHAR       LastSsidLen;               // the actual ssid length in used
	CHAR        LastSsid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
	UCHAR		LastBssid[MAC_ADDR_LEN];
#endif/*APCLI_WPA_SUPPLICANT_SUPPORT*/

#ifdef P2P_SUPPORT
	BOOLEAN 					bP2pClient;
#endif /* P2P_SUPPORT */

	PSPOLL_FRAME PsPollFrame;
	HEADER_802_11 NullFrame;

	UAPSD_INFO	UapsdInfo;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY RepeaterCli[MAX_EXT_MAC_ADDR_SIZE];
	REPEATER_CLIENT_ENTRY_MAP RepeaterCliMap[MAX_EXT_MAC_ADDR_SIZE];
#endif /* MAC_REPEATER_SUPPORT */
	UCHAR LinkIdx;
	APCLI_COUNTER ApCliCounter;
	PVOID pAd;
} APCLI_STRUCT, *PAPCLI_STRUCT;

typedef struct _AP_ADMIN_CONFIG {
	USHORT CapabilityInfo;
	/* Multiple SSID */
	UCHAR BssidNum;
	UCHAR MacMask;
	MULTISSID_STRUCT MBSSID[HW_BEACON_MAX_NUM];
	BOOLEAN IsolateInterStaTrafficBTNBSSID;

#ifdef APCLI_SUPPORT
	UCHAR ApCliInfRunned;	/* Number of  ApClient interface which was running. value from 0 to MAX_APCLI_INTERFACE */
	BOOLEAN FlgApCliIsUapsdInfoUpdated;
	APCLI_STRUCT ApCliTab[MAX_APCLI_NUM];	/*AP-client */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	BOOLEAN		ApCliAutoConnectRunning;
	BOOLEAN		ApCliAutoConnectChannelSwitching;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif	/* APCLI_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	NDIS_SPIN_LOCK ReptCliEntryLock;
	REPEATER_CLIENT_ENTRY *ReptCliHash[HASH_TABLE_SIZE];
	REPEATER_CLIENT_ENTRY_MAP *ReptMapHash[HASH_TABLE_SIZE];
	UCHAR BridgeAddress[ETH_LENGTH_OF_ADDRESS];
	REPEATER_CTRL_STRUCT ReptControl;
#endif /* MAC_REPEATER_SUPPORT */

	/* for wpa */
	RALINK_TIMER_STRUCT CounterMeasureTimer;

	UCHAR CMTimerRunning;
	UCHAR BANClass3Data;
	LARGE_INTEGER aMICFailTime;
	LARGE_INTEGER PrevaMICFailTime;
	ULONG MICFailureCounter;

	RSSI_SAMPLE RssiSample;
	ULONG NumOfAvgRssiSample;

	BOOLEAN bAutoChannelAtBootup;	/* 0: disable, 1: enable */
	ChannelSel_Alg AutoChannelAlg;	/* Alg for selecting Channel */
#ifdef AP_SCAN_SUPPORT
	UINT32  ACSCheckTime;           /* Periodic timer to trigger Auto Channel Selection (unit: second) */
	UINT32  ACSCheckCount;          /* if  ACSCheckCount > ACSCheckTime, then do ACS check */
	UINT32	ScanChannelCnt;
	UINT32	LastScanChannel;
	BOOLEAN	bImprovedScan;
#endif /* AP_SCAN_SUPPORT */
	BOOLEAN bAvoidDfsChannel;	/* 0: disable, 1: enable */
	BOOLEAN bHideSsid;

	/* temporary latch for Auto channel selection */
	ULONG ApCnt;		/* max RSSI during Auto Channel Selection period */
	UCHAR AutoChannel_Channel;	/* channel number during Auto Channel Selection */
	UCHAR current_channel_index;	/* current index of channel list */
	UCHAR AutoChannelSkipListNum;	/* number of rejected channel list */
	UCHAR AutoChannelSkipList[10];
	UCHAR DtimCount;	/* 0.. DtimPeriod-1 */
	UCHAR DtimPeriod;	/* default = 3 */
	UCHAR ErpIeContent;
	ULONG LastOLBCDetectTime;
	ULONG LastNoneHTOLBCDetectTime;
	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */

	UCHAR LastSNR0;		/* last received BEACON's SNR */
	UCHAR LastSNR1;		/* last received BEACON's SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	UCHAR LastSNR2;		/* last received BEACON's SNR for 2nd  antenna */
#endif				/* DOT11N_SS3_SUPPORT */

#ifdef DOT1X_SUPPORT
	/* dot1x related parameter */
	UINT32 own_ip_addr;
	UINT32 retry_interval;
	UINT32 session_timeout_interval;
	UINT32 quiet_interval;
	UCHAR EAPifname[HW_BEACON_MAX_NUM][IFNAMSIZ];	/* indicate as the binding interface for EAP negotiation. */
	UCHAR EAPifname_len[HW_BEACON_MAX_NUM];
	UCHAR PreAuthifname[HW_BEACON_MAX_NUM][IFNAMSIZ];	/* indicate as the binding interface for WPA2 Pre-authentication. */
	UCHAR PreAuthifname_len[HW_BEACON_MAX_NUM];
#endif				/* DOT1X_SUPPORT */

	/* EDCA parameters to be announced to its local BSS */
	EDCA_PARM BssEdcaParm;

	RALINK_TIMER_STRUCT ApQuickResponeForRateUpTimer;
	BOOLEAN ApQuickResponeForRateUpTimerRunning;

#ifdef IDS_SUPPORT
	/* intrusion detection parameter */
	BOOLEAN IdsEnable;
	UINT32 AuthFloodThreshold;	/* Authentication frame flood threshold */
	UINT32 AssocReqFloodThreshold;	/* Association request frame flood threshold */
	UINT32 ReassocReqFloodThreshold;	/* Re-association request frame flood threshold */
	UINT32 ProbeReqFloodThreshold;	/* Probe request frame flood threshold */
	UINT32 DisassocFloodThreshold;	/* Disassociation frame flood threshold */
	UINT32 DeauthFloodThreshold;	/* Deauthentication frame flood threshold */
	UINT32 EapReqFloodThreshold;	/* EAP request frame flood threshold */

	UINT32 RcvdAuthCount;
	UINT32 RcvdAssocReqCount;
	UINT32 RcvdReassocReqCount;
	UINT32 RcvdProbeReqCount;
	UINT32 RcvdDisassocCount;
	UINT32 RcvdDeauthCount;
	UINT32 RcvdEapReqCount;

	RALINK_TIMER_STRUCT IDSTimer;
	BOOLEAN IDSTimerRunning;
#endif				/* IDS_SUPPORT */

	/* Indicate the maximum idle timeout */
	UINT32 StaIdleTimeout;

	ULONG EntryLifeCheck;

#ifdef IGMP_SNOOP_SUPPORT
	BOOLEAN IgmpSnoopEnable;	/* 0: disable, 1: enable. */
#endif				/* IGMP_SNOOP_SUPPORT */


#ifdef CLIENT_WDS
	NDIS_SPIN_LOCK CliWdsTabLock;
	PCLIWDS_PROXY_ENTRY pCliWdsEntryPool;
	LIST_HEADER CliWdsEntryFreeList;
	LIST_HEADER CliWdsProxyTab[CLIWDS_HASH_TAB_SIZE];
#endif /* CLIENT_WDS */

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	UCHAR GreenAPLevel;
	BOOLEAN bGreenAPEnable;
	BOOLEAN bGreenAPActive;
#endif /* GREENAP_SUPPORT */

	ULONG	MAX_PSDU_LEN;	/* Maximum PSDU length */
#endif /* DOT11_N_SUPPORT */

	UCHAR EntryClientCount;
#ifdef MULTI_CLIENT_SUPPORT
	UCHAR ChangeTxOpClient;
#endif /* MULTI_CLIENT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	BOOLEAN bMACRepeaterEn;
	UCHAR MACRepeaterOuiMode;
	UINT8 EthApCliIdx;
	UCHAR RepeaterCliSize;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef BAND_STEERING
	/* 
		This is used to let user config band steering on/off by profile.
		0: OFF / 1: ON / 2: Auto ONOFF
	*/
	BOOLEAN BandSteering;
	BND_STRG_CLI_TABLE BndStrgTable;
#endif /* BAND_STEERING */
} AP_ADMIN_CONFIG, *PAP_ADMIN_CONFIG;

#ifdef IGMP_SNOOP_SUPPORT
typedef enum _IGMP_GROUP_TYPE {
	MODE_IS_INCLUDE = 1,
	MODE_IS_EXCLUDE,
	CHANGE_TO_INCLUDE_MODE,
	CHANGE_TO_EXCLUDE_MODE,
	ALLOW_NEW_SOURCES,
	BLOCK_OLD_SOURCES
} IgmpGroupType;

typedef enum _MULTICAST_FILTER_ENTRY_TYPE {
	MCAT_FILTER_STATIC = 0,
	MCAT_FILTER_DYNAMIC,
} MulticastFilterEntryType;

typedef struct _MEMBER_ENTRY {
	struct _MEMBER_ENTRY *pNext;
	UCHAR Addr[MAC_ADDR_LEN];
/*	USHORT Aid; */
} MEMBER_ENTRY, *PMEMBER_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE_ENTRY {
	BOOLEAN Valid;
	MulticastFilterEntryType type;	/* 0: static, 1: dynamic. */
	UINT lastTime;
	PNET_DEV net_dev;
	UCHAR Addr[MAC_ADDR_LEN];
	LIST_HEADER MemberList;
	struct _MULTICAST_FILTER_TABLE_ENTRY *pNext;
} MULTICAST_FILTER_TABLE_ENTRY, *PMULTICAST_FILTER_TABLE_ENTRY;

typedef struct _MULTICAST_FILTER_TABLE {
	UCHAR Size;
	PMULTICAST_FILTER_TABLE_ENTRY
	  Hash[MAX_LEN_OF_MULTICAST_FILTER_HASH_TABLE];
	MULTICAST_FILTER_TABLE_ENTRY Content[MAX_LEN_OF_MULTICAST_FILTER_TABLE];
	NDIS_SPIN_LOCK MulticastFilterTabLock;
	NDIS_SPIN_LOCK FreeMemberPoolTabLock;
	MEMBER_ENTRY freeMemberPool[FREE_MEMBER_POOL_SIZE];
	LIST_HEADER freeEntryList;
} MULTICAST_FILTER_TABLE, *PMULTICAST_FILTER_TABLE;
#endif /* IGMP_SNOOP_SUPPORT */


#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
typedef enum _RT_GREEN_AP_LEVEL {
	GREENAP_11BGN_STAS = 0,
	GREENAP_ONLY_11BG_STAS,
	GREENAP_WITHOUT_ANY_STAS_CONNECT
} RT_GREEN_AP_LEVEL;
#endif /* DOT11_N_SUPPORT */
#endif /* GREENAP_SUPPORT */

/* ----------- end of AP ---------------------------- */
#endif /* CONFIG_AP_SUPPORT */

#ifdef BLOCK_NET_IF
typedef struct _BLOCK_QUEUE_ENTRY {
	BOOLEAN SwTxQueueBlockFlag;
	LIST_HEADER NetIfList;
} BLOCK_QUEUE_ENTRY, *PBLOCK_QUEUE_ENTRY;
#endif /* BLOCK_NET_IF */


struct wificonf {
	BOOLEAN bShortGI;
	BOOLEAN bGreenField;
};

typedef struct _RTMP_DEV_INFO_ {
	UCHAR chipName[16];
	RTMP_INF_TYPE infType;
} RTMP_DEV_INFO;

#ifdef DBG_DIAGNOSE
#define DIAGNOSE_TIME	10	/* 10 sec */
typedef struct _RtmpDiagStrcut_ {	/* Diagnosis Related element */
	unsigned char inited;
	unsigned char qIdx;
	unsigned char ArrayStartIdx;
	unsigned char ArrayCurIdx;
	/* Tx Related Count */
	USHORT TxDataCnt[DIAGNOSE_TIME];
	USHORT TxFailCnt[DIAGNOSE_TIME];
/*	USHORT TxDescCnt[DIAGNOSE_TIME][16];		// TxDesc queue length in scale of 0~14, >=15 */
	USHORT TxDescCnt[DIAGNOSE_TIME][24];	/* 3*3    // TxDesc queue length in scale of 0~14, >=15 */
/*	USHORT TxMcsCnt[DIAGNOSE_TIME][16];			// TxDate MCS Count in range from 0 to 15, step in 1. */
	USHORT TxMcsCnt[DIAGNOSE_TIME][MAX_MCS_SET];	/* 3*3 */
	USHORT TxSGICnt[DIAGNOSE_TIME][MAX_MCS_SET];
	USHORT TxSWQueCnt[DIAGNOSE_TIME][9];	/* TxSwQueue length in scale of 0, 1, 2, 3, 4, 5, 6, 7, >=8 */

	USHORT TxAggCnt[DIAGNOSE_TIME];
	USHORT TxNonAggCnt[DIAGNOSE_TIME];
/*	USHORT TxAMPDUCnt[DIAGNOSE_TIME][16];		// 10 sec, TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1. */
	USHORT TxAMPDUCnt[DIAGNOSE_TIME][MAX_MCS_SET];	/* 3*3 // 10 sec, TxDMA APMDU Aggregation count in range from 0 to 15, in setp of 1. */
	USHORT TxRalinkCnt[DIAGNOSE_TIME];	/* TxRalink Aggregation Count in 1 sec scale. */
	USHORT TxAMSDUCnt[DIAGNOSE_TIME];	/* TxAMSUD Aggregation Count in 1 sec scale. */

	/* Rx Related Count */
	USHORT RxDataCnt[DIAGNOSE_TIME];	/* Rx Total Data count. */
	USHORT RxCrcErrCnt[DIAGNOSE_TIME];
/*	USHORT			RxMcsCnt[DIAGNOSE_TIME][16];		// Rx MCS Count in range from 0 to 15, step in 1. */
	USHORT RxMcsCnt[DIAGNOSE_TIME][MAX_MCS_SET];	/* 3*3 */
	USHORT RxSGICnt[DIAGNOSE_TIME][MAX_MCS_SET];
} RtmpDiagStruct;
#endif /* DBG_DIAGNOSE */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 
/*
	The number of channels for per-channel Tx power offset
*/
#define NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET	14

/* The Tx power control using the internal ALC */
typedef struct _TX_POWER_CONTROL {
	BOOLEAN bInternalTxALC; /* Internal Tx ALC */
	BOOLEAN bExtendedTssiMode; /* The extended TSSI mode (each channel has different Tx power if needed) */
	CHAR PerChTxPwrOffset[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1]; /* Per-channel Tx power offset */
	CHAR idxTxPowerTable; /* The index of the Tx power table for ant0 */
	CHAR idxTxPowerTable2; /* The index of the Tx power table for ant1 */
	CHAR RF_TX_ALC; /* 3390: RF R12[4:0]: Tx0 ALC, 3352: RF R47[4:0]: Tx0 ALC, 5390: RF R49[5:0]: Tx0 ALC */
	CHAR MAC_PowerDelta; /* Tx power control over MAC 0x1314~0x1324 */
	CHAR MAC_PowerDelta2; /* Tx power control for Tx1 */
	CHAR TotalDeltaPower2; /* Tx power control for Tx1 */
#ifdef RTMP_TEMPERATURE_COMPENSATION
	INT LookupTable[IEEE80211_BAND_NUMS][33];
	INT RefTemp[IEEE80211_BAND_NUMS];
	UCHAR TssiGain[IEEE80211_BAND_NUMS];
	/* Index offset, -7....25. */
	INT LookupTableIndex;
#endif /* RTMP_TEMPERATURE_COMPENSATION */
} TX_POWER_CONTROL, *PTX_POWER_CONTROL;
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

/* */
/* The entry of transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_OVER_MAC_ENTRY {
	USHORT MACRegisterOffset;	/* MAC register offset */
	ULONG RegisterValue;	/* Register value */
} TX_POWER_CONTROL_OVER_MAC_ENTRY, *PTX_POWER_CONTROL_OVER_MAC_ENTRY;

/* */
/* The maximum registers of transmit power control */
/* */
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS 5

#ifdef RT2883
#undef MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS	12
#endif /* RT2883 */

#ifdef RT3883
#undef MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS
#define MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS	15
#endif /* RT3883 */

/* */
/* The configuration of the transmit power control over MAC */
/* */
typedef struct _CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC {
	UCHAR NumOfEntries;	/* Number of entries */
	TX_POWER_CONTROL_OVER_MAC_ENTRY TxPwrCtrlOverMAC[MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS];
} CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC, *PCONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC;

/* */
/* The extension of the transmit power control over MAC */
/* */
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC {
	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over2Dot4G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW20Over5G;

	struct {
		ULONG TxPwrCfg0;	/* MAC 0x1314 */
		ULONG TxPwrCfg0Ext;	/* MAC 0x1390 */
		ULONG TxPwrCfg1;	/* MAC 0x1318 */
		ULONG TxPwrCfg1Ext;	/* MAC 0x1394 */
		ULONG TxPwrCfg2;	/* MAC 0x131C */
		ULONG TxPwrCfg2Ext;	/* MAC 0x1398 */
		ULONG TxPwrCfg3;	/* MAC 0x1320 */
		ULONG TxPwrCfg3Ext;	/* MAC 0x139C */
		ULONG TxPwrCfg4;	/* MAC 0x1324 */
		ULONG TxPwrCfg4Ext;	/* MAC 0x13A0 */
		ULONG TxPwrCfg5;	/* MAC 0x1384 */
		ULONG TxPwrCfg6;	/* MAC 0x1388 */
		ULONG TxPwrCfg7;	/* MAC 0x13D4 */
		ULONG TxPwrCfg8;	/* MAC 0x13D8 */
		ULONG TxPwrCfg9;	/* MAC 0x13DC */
	} BW40Over5G;
} TX_POWER_CONTROL_EXT_OVER_MAC, *PTX_POWER_CONTROL_EXT_OVER_MAC;

/* For Wake on Wireless LAN */
#ifdef WOW_SUPPORT
#endif /* WOW_SUPPORT */

/*
	Packet drop reason code
*/
typedef enum{
	PKT_ATE_ON = 1 << 8,
	PKT_RADAR_ON = 2 << 8,
	PKT_RRM_QUIET = 3 << 8,
	PKT_TX_STOP = 4 <<8,
	PKT_TX_JAM = 5 << 8,

	PKT_NETDEV_DOWN = 6 < 8,
	PKT_NETDEV_NO_MATCH = 7 << 8,
	PKT_NOT_ALLOW_SEND = 8 << 8,

	PKT_INVALID_DST = 9<< 8,
	PKT_INVALID_SRC = 10 << 8,
	PKT_INVALID_PKT_DATA = 11 << 8,
	PKT_INVALID_PKT_LEN = 12 << 8,
	PKT_INVALID_ETH_TYPE = 13 << 8,
	PKT_INVALID_TXBLK_INFO = 14 << 8,
	PKT_INVALID_SW_ENCRYPT = 15 << 8,
	PKT_INVALID_PKT_TYPE = 16 << 8,
	PKT_INVALID_PKT_MIC = 17 << 8,
	
	PKT_PORT_NOT_SECURE = 18 << 8,
	PKT_TSPEC_NO_MATCH  = 19 << 8,
	PKT_NO_ASSOCED_STA = 20 << 8,
	PKT_INVALID_MAC_ENTRY = 21 << 8,
	
	PKT_TX_QUE_FULL = 22 << 8,
	PKT_TX_QUE_ADJUST = 23<<8,
		
	PKT_PS_QUE_TIMEOUT = 24 <<8,
	PKT_PS_QUE_CLEAN = 25 << 8,
	PKT_MCAST_PS_QUE_FULL = 26 << 8,
	PKT_UCAST_PS_QUE_FULL = 27 << 8,
	
	PKT_RX_EAPOL_SANITY_FAIL = 28 <<8,
	PKT_RX_NOT_TO_KERNEL = 29 << 8,
	PKT_RX_MESH_SIG_FAIL = 30 << 8,
	PKT_APCLI_FAIL = 31 << 8,
	PKT_ZERO_DATA = 32 <<8,
	PKT_SW_DECRYPT_FAIL = 33 << 8,
	PKT_TX_SW_ENC_FAIL = 34 << 8,

	PKT_ACM_FAIL = 35 << 8,
	PKT_IGMP_GRP_FAIL = 36 << 8,
	PKT_MGMT_FAIL = 37 << 8,
	PKT_AMPDU_OUT_ORDER = 38 << 8,
	PKT_UAPSD_EOSP = 39 << 8,
	PKT_UAPSD_Q_FULL = 40 << 8,

	PKT_DRO_REASON_MAX = 41,
}PKT_DROP_REASON;

/* Packet drop Direction code */
typedef enum{
	PKT_TX = 0,
	PKT_RX = 1 << 31,
}PKT_DROP_DIECTION;




typedef struct _BBP_RESET_CTL
{
#define BBP_RECORD_NUM	47
	REG_PAIR BBPRegDB[BBP_RECORD_NUM];
	BOOLEAN	AsicCheckEn;
} BBP_RESET_CTL, *PBBP_RESET_CTL;

/* */
/*  The miniport adapter structure */
/* */
struct _RTMP_ADAPTER {
	PVOID OS_Cookie;	/* save specific structure relative to OS */
	PNET_DEV net_dev;
	ULONG VirtualIfCnt;

	RTMP_CHIP_OP chipOps;
	RTMP_CHIP_CAP chipCap;

#ifdef CONFIG_STA_SUPPORT
	USHORT ThisTbttNumToNextWakeUp;
#endif /* CONFIG_STA_SUPPORT */

#ifdef HOSTAPD_SUPPORT
	UINT32 IoctlIF;
#endif /* HOSTAPD_SUPPORT */
#ifdef INF_PPA_SUPPORT
	UINT32 g_if_id;
	BOOLEAN PPAEnable;
	PPA_DIRECTPATH_CB *pDirectpathCb;
#endif /* INF_PPA_SUPPORT */

#ifdef RTMP_MAC_PCI
/*****************************************************************************************/
/*      PCI related parameters                                                           								  */
/*****************************************************************************************/
	PUCHAR CSRBaseAddress;	/* PCI MMIO Base Address, all access will use */
	unsigned int irq_num;
#ifdef CONFIG_STA_SUPPORT
	USHORT LnkCtrlBitMask;
	USHORT RLnkCtrlConfiguration;
	USHORT RLnkCtrlOffset;
	USHORT HostLnkCtrlConfiguration;
	USHORT HostLnkCtrlOffset;
	USHORT PCIePowerSaveLevel;
	ULONG Rt3xxHostLinkCtrl;	/* USed for 3090F chip */
	ULONG Rt3xxRalinkLinkCtrl;	/* USed for 3090F chip */
	ULONG				HostVendor;
#endif /* CONFIG_STA_SUPPORT */
	USHORT DeviceID;	/* Read from PCI config */
	ULONG AccessBBPFailCount;
	BOOLEAN bPCIclkOff;	/* flag that indicate if the PICE power status in Configuration SPace.. */
	BOOLEAN bPCIclkOffDisableTx;	/* */

	BOOLEAN brt30xxBanMcuCmd;	/*when = 0xff means all commands are ok to set . */
	BOOLEAN b3090ESpecialChip;	/*3090E special chip that write EEPROM 0x24=0x9280. */
	/*ULONG                                 CheckDmaBusyCount;  // Check Interrupt Status Register Count. */

	UINT int_enable_reg;
	UINT int_disable_mask;
	UINT int_pending;

	RTMP_DMABUF TxBufSpace[NUM_OF_TX_RING];	/* Shared memory of all 1st pre-allocated TxBuf associated with each TXD */
	RTMP_DMABUF RxDescRing;	/* Shared memory for RX descriptors */
	RTMP_DMABUF TxDescRing[NUM_OF_TX_RING];	/* Shared memory for Tx descriptors */
	RTMP_TX_RING TxRing[NUM_OF_TX_RING];	/* AC0~4 + HCCA */

	NDIS_SPIN_LOCK LockInterrupt;
#endif /* RTMP_MAC_PCI */

	NDIS_SPIN_LOCK irq_lock;

	/*======Cmd Thread in PCI/RBUS/USB */
	CmdQ CmdQ;
	NDIS_SPIN_LOCK CmdQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK cmdQTask;


/*****************************************************************************************/
/*      RBUS related parameters                                                           								  */
/*****************************************************************************************/

/*****************************************************************************************/
/*      Both PCI/USB related parameters                                                  							  */
/*****************************************************************************************/
	/*RTMP_DEV_INFO                 chipInfo; */
	RTMP_INF_TYPE infType;

/*****************************************************************************************/
/*      Driver Mgmt related parameters                                                  							  */
/*****************************************************************************************/
	RTMP_OS_TASK mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
	/* If you want use timer task to handle the timer related jobs, enable this. */
	RTMP_TIMER_TASK_QUEUE TimerQ;
	NDIS_SPIN_LOCK TimerQLock;
	RTMP_OS_TASK timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */

/*****************************************************************************************/
/*      Tx related parameters                                                           */
/*****************************************************************************************/
	BOOLEAN DeQueueRunning[NUM_OF_TX_RING];	/* for ensuring RTUSBDeQueuePacket get call once */
	NDIS_SPIN_LOCK DeQueueLock[NUM_OF_TX_RING];


	/* resource for software backlog queues */
	QUEUE_HEADER TxSwQueue[NUM_OF_TX_RING];	/* 4 AC + 1 HCCA */
	NDIS_SPIN_LOCK TxSwQueueLock[NUM_OF_TX_RING];	/* TxSwQueue spinlock */

	/* Maximum allowed tx software Queue length */
	UINT32					TxSwQMaxLen;

	RTMP_DMABUF MgmtDescRing;	/* Shared memory for MGMT descriptors */
	RTMP_MGMT_RING MgmtRing;
	NDIS_SPIN_LOCK MgmtRingLock;	/* Prio Ring spinlock */

	UCHAR LastMCUCmd;

/*****************************************************************************************/
/*      Rx related parameters                                                           */
/*****************************************************************************************/

#ifdef RTMP_MAC_PCI
	RTMP_RX_RING RxRing;
	NDIS_SPIN_LOCK RxRingLock;	/* Rx Ring spinlock */
#endif /* RTMP_MAC_PCI */


/*****************************************************************************************/
/*      ASIC related parameters                                                          */
/*****************************************************************************************/
	UINT32 MACVersion;	/* MAC version. Record rt2860C(0x28600100) or rt2860D (0x28600101).. */

	/* --------------------------- */
	/* E2PROM */
	/* --------------------------- */
	ULONG EepromVersion;	/* byte 0: version, byte 1: revision, byte 2~3: unused */
	ULONG FirmwareVersion;	/* byte 0: Minor version, byte 1: Major version, otherwise unused. */
	USHORT EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
#ifdef TXBF_SUPPORT
	USHORT EEPROMITxBFCalParams[6];
#endif /* TXBF_SUPPORT */
	UCHAR EEPROMAddressNum;	/* 93c46=6  93c66=8 */
	BOOLEAN EepromAccess;
	UCHAR EFuseTag;

	/* --------------------------- */
	/* BBP Control */
	/* --------------------------- */
	UCHAR BbpWriteLatch[MAX_BBP_ID + 1];	/* record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID */
	CHAR BbpRssiToDbmDelta;	/* change from UCHAR to CHAR for high power */
	BBP_R66_TUNING BbpTuning;

	/* ---------------------------- */
	/* RFIC control */
	/* ---------------------------- */
	UCHAR RfIcType;		/* RFIC_xxx */
	ULONG RfFreqOffset;	/* Frequency offset for channel switching */

#ifdef RTMP_RBUS_SUPPORT
	UCHAR RfFreqDelta;	/* Frequency Delta */
#endif /* RTMP_RBUS_SUPPORT */
#ifdef RTMP_MAC_PCI
	UCHAR ShowRf;		/* Show RF register write for 2880 */
#endif /* RTMP_MAC_PCI */

	RTMP_RF_REGS LatchRfRegs;	/* latch th latest RF programming value since RF IC doesn't support READ */

	EEPROM_ANTENNA_STRUC Antenna;	/* Since ANtenna definition is different for a & g. We need to save it for future reference. */
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	/* This soft Rx Antenna Diversity mechanism is used only when user set */
	/* RX Antenna = DIVERSITY ON */
	SOFT_RX_ANT_DIVERSITY RxAnt;

	CHANNEL_TX_POWER TxPower[MAX_NUM_OF_CHANNELS];	/* Store Tx power value for all channels. */
	CHANNEL_TX_POWER ChannelList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for site survey */

#ifdef RT3883
	BOOLEAN bTxPwrRangeExt;
#endif /* RT3883 */


	UCHAR ChannelListNum;	/* number of channel in ChannelList[] */
	UCHAR Bbp94;
	BOOLEAN BbpForCCK;
	ULONG Tx20MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx20MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgABand[MAX_TXPOWER_ARRAY_SIZE];
	ULONG Tx40MPwrCfgGBand[MAX_TXPOWER_ARRAY_SIZE];


	BOOLEAN bAutoTxAgcA;	/* Enable driver auto Tx Agc control */
	UCHAR TssiRefA;		/* Store Tssi reference value as 25 temperature. */
	UCHAR TssiPlusBoundaryA[5];	/* Tssi boundary for increase Tx power to compensate. */
	UCHAR TssiMinusBoundaryA[5];	/* Tssi boundary for decrease Tx power to compensate. */
	UCHAR TxAgcStepA;	/* Store Tx TSSI delta increment / decrement value */
	CHAR TxAgcCompensateA;	/* Store the compensation (TxAgcStep * (idx-1)) */

	BOOLEAN bAutoTxAgcG;	/* Enable driver auto Tx Agc control */
#ifdef RT6352
	CHAR TssiRefG;		/* Store Tssi reference value as 25 degrees. */
	CHAR TssiPlusBoundaryG[8];		/* Tssi boundary for increase Tx power to compensate. */
	CHAR TssiMinusBoundaryG[8];		/* Tssi boundary for decrease Tx power to compensate. */
	CHAR TssiCalibratedOffset;		/* reference temperature(e2p[D1h]) */  
	CHAR mp_delta_pwr;				/* calculated by MP ATE temperature(e2p[77h]) */
#else
	UCHAR TssiRefG;		/* Store Tssi reference value as 25 temperature. */
	UCHAR TssiPlusBoundaryG[5];	/* Tssi boundary for increase Tx power to compensate. */
	UCHAR TssiMinusBoundaryG[5];	/* Tssi boundary for decrease Tx power to compensate. */
#endif /* RT6352 */
	UCHAR TxAgcStepG;	/* Store Tx TSSI delta increment / decrement value */
	CHAR TxAgcCompensateG;	/* Store the compensation (TxAgcStep * (idx-1)) */
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 
	TX_POWER_CONTROL TxPowerCtrl;	/* The Tx power control using the internal ALC */
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

	UCHAR rf_pa_mode_over_cck[4];
	UCHAR rf_pa_mode_over_ofdm[8];
	UCHAR rf_pa_mode_over_ht[16];


#ifdef THERMAL_PROTECT_SUPPORT
	BOOLEAN force_one_tx_stream;
	INT32 last_thermal_pro_temp;
	INT32 thermal_pro_criteria;
#endif /* THERMAL_PROTECT_SUPPORT */

	signed char BGRssiOffset0;	/* Store B/G RSSI#0 Offset value on EEPROM 0x46h */
	signed char BGRssiOffset1;	/* Store B/G RSSI#1 Offset value */
	signed char BGRssiOffset2;	/* Store B/G RSSI#2 Offset value */

	signed char ARssiOffset0;	/* Store A RSSI#0 Offset value on EEPROM 0x4Ah */
	signed char ARssiOffset1;	/* Store A RSSI#1 Offset value */
	signed char ARssiOffset2;	/* Store A RSSI#2 Offset value */

	CHAR BLNAGain;		/* Store B/G external LNA#0 value on EEPROM 0x44h */
	CHAR ALNAGain0;		/* Store A external LNA#0 value for ch36~64 */
	CHAR ALNAGain1;		/* Store A external LNA#1 value for ch100~128 */
	CHAR ALNAGain2;		/* Store A external LNA#2 value for ch132~165 */


#ifdef LED_CONTROL_SUPPORT
	/* LED control */
	LED_CONTROL LedCntl;
#endif /* LED_CONTROL_SUPPORT */

	/* ---------------------------- */
	/* MAC control */
	/* ---------------------------- */
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	UCHAR ShrMSel;
	NDIS_SPIN_LOCK ShrMemLock;
#endif /* SPECIFIC_BCN_BUF_SUPPORT */

#ifdef DROP_MASK_SUPPORT
	NDIS_SPIN_LOCK drop_mask_lock;
#endif /* DROP_MASK_SUPPORT */

/*****************************************************************************************/
/*      802.11 related parameters                                                        */
/*****************************************************************************************/
	/* outgoing BEACON frame buffer and corresponding TXD */
	TXWI_STRUC BeaconTxWI;
	PUCHAR BeaconBuf;
	USHORT BeaconOffset[HW_BEACON_MAX_NUM];

	/* pre-build PS-POLL and NULL frame upon link up. for efficiency purpose. */
#ifdef CONFIG_STA_SUPPORT
	PSPOLL_FRAME PsPollFrame;
#endif /* CONFIG_STA_SUPPORT */
	HEADER_802_11 NullFrame;



#ifdef UAPSD_SUPPORT
	NDIS_SPIN_LOCK UAPSDEOSPLock;	/* EOSP frame access lock use */
	BOOLEAN bAPSDFlagSPSuspend;	/* 1: SP is suspended; 0: SP is not */
#endif /* UAPSD_SUPPORT */

/*=========AP=========== */
#ifdef CONFIG_AP_SUPPORT
	/* ----------------------------------------------- */
	/* AP specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_AP */
	/* ----------------------------------------------- */
	AP_ADMIN_CONFIG ApCfg;	/* user configuration when in AP mode */
	AP_MLME_AUX ApMlmeAux;
#ifdef APCLI_SUPPORT
		//MLME_AUX				ApCliMlmeAux;			/* temporary settings used during MLME state machine */
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
	WDS_TABLE WdsTab;	/* WDS table when working as an AP */
	NDIS_SPIN_LOCK WdsTabLock;
#endif /* WDS_SUPPORT */

#ifdef MBSS_SUPPORT
	BOOLEAN FlgMbssInit;
#endif /* MBSS_SUPPORT */

#ifdef WDS_SUPPORT
	BOOLEAN flg_wds_init;
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
	BOOLEAN flg_apcli_init;
#endif /* APCLI_SUPPORT */

	PBSSINFO pBssInfoTab;
	PCHANNELINFO pChannelInfo;

#endif /* CONFIG_AP_SUPPORT */

/*=======STA=========== */
#ifdef CONFIG_STA_SUPPORT
	/* ----------------------------------------------- */
	/* STA specific configuration & operation status */
	/* used only when pAd->OpMode == OPMODE_STA */
	/* ----------------------------------------------- */
	STA_ADMIN_CONFIG StaCfg;	/* user desired settings */
	STA_ACTIVE_CONFIG StaActive;	/* valid only when ADHOC_ON(pAd) || INFRA_ON(pAd) */
	CHAR nickname[IW_ESSID_MAX_SIZE + 1];	/* nickname, only used in the iwconfig i/f */
	NDIS_MEDIA_STATE PreMediaState;
#endif /* CONFIG_STA_SUPPORT */

/*=======Common=========== */
	/* OP mode: either AP or STA */
	UCHAR OpMode;		/* OPMODE_STA, OPMODE_AP */

	enum RATE_ADAPT_ALG rateAlg;		/* Rate adaptation algorithm */
	
	NDIS_MEDIA_STATE IndicateMediaState;	/* Base on Indication state, default is NdisMediaStateDisConnected */

#ifdef PROFILE_STORE
	RTMP_OS_TASK 	WriteDatTask;
	BOOLEAN			bWriteDat;
#endif /* PROFILE_STORE */

#ifdef CREDENTIAL_STORE	
	STA_CONNECT_INFO StaCtIf;
#endif /* CREDENTIAL_STORE */

#ifdef WSC_INCLUDED
	RTMP_OS_TASK wscTask;
	UCHAR WriteWscCfgToDatFile;
	BOOLEAN WriteWscCfgToAr9DatFile;
	NDIS_SPIN_LOCK WscElmeLock;
	MLME_QUEUE_ELEM *pWscElme;

	/* WSC hardware push button function 0811 */
	BOOLEAN WscHdrPshBtnFlag;	/* 1: support, read from EEPROM */
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN bWscDriverAutoUpdateCfg;
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef RTMP_RBUS_SUPPORT
#ifdef RT3XXX_ANTENNA_DIVERSITY_SUPPORT
	RTMP_OS_TASK ADTask;
#endif /* RT3XXX_ANTENNA_DIVERSITY_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

	/* MAT related parameters */
#ifdef MAT_SUPPORT
	MAT_STRUCT MatCfg;
#endif /* MAT_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
	AP_VIDEO_STRUCT VideoTurbine;
#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

	/* 
		Frequency setting for rate adaptation 
			@ra_interval: 		for baseline time interval
			@ra_fast_interval:	for quick response time interval
	*/
	UINT32			ra_interval;
	UINT32			ra_fast_interval;

	/* configuration: read from Registry & E2PROM */
	BOOLEAN bLocalAdminMAC;	/* Use user changed MAC */
	UCHAR PermanentAddress[MAC_ADDR_LEN];	/* Factory default MAC address */
	UCHAR CurrentAddress[MAC_ADDR_LEN];	/* User changed MAC address */

	/* ------------------------------------------------------ */
	/* common configuration to both OPMODE_STA and OPMODE_AP */
	/* ------------------------------------------------------ */
	COMMON_CONFIG CommonCfg;
	MLME_STRUCT Mlme;

	/* AP needs those vaiables for site survey feature. */
	MLME_AUX MlmeAux;	/* temporary settings used during MLME state machine */
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	BSS_TABLE ScanTab;	/* store the latest SCAN result */
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */

	/*About MacTab, the sta driver will use #0 and #1 for multicast and AP. */
	MAC_TABLE MacTab;	/* ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table. */
	NDIS_SPIN_LOCK MacTabLock;

#ifdef DOT11_N_SUPPORT
	BA_TABLE BATable;
	NDIS_SPIN_LOCK BATabLock;
	RALINK_TIMER_STRUCT RECBATimer;
#endif /* DOT11_N_SUPPORT */

	/* DOT11_H */
	DOT11_H Dot11_H;

	/* encryption/decryption KEY tables */
	CIPHER_KEY SharedKey[HW_BEACON_MAX_NUM + MAX_P2P_NUM][4];	/* STA always use SharedKey[BSS0][0..3] */

	/* RX re-assembly buffer for fragmentation */
	FRAGMENT_FRAME FragFrame;	/* Frame storage for fragment frame */

	/* various Counters */
	COUNTER_802_3 Counters8023;	/* 802.3 counters */
	COUNTER_802_11 WlanCounters;	/* 802.11 MIB counters */
	COUNTER_RALINK RalinkCounters;	/* Ralink propriety counters */
	/* COUNTER_DRS DrsCounters;	*/ /* counters for Dynamic TX Rate Switching */
	PRIVATE_STRUC PrivateInfo;	/* Private information & counters */

	/* flags, see fRTMP_ADAPTER_xxx flags */
	ULONG Flags;		/* Represent current device status */
	ULONG PSFlags;		/* Power Save operation flag. */
	ULONG MoreFlags;	/* Represent specific requirement */

	/* current TX sequence # */
	USHORT Sequence;

	/* Control disconnect / connect event generation */
	/*+++Didn't used anymore */
	ULONG LinkDownTime;
	/*--- */
	ULONG LastRxRate;
	ULONG LastTxRate;
	/*+++Used only for Station */
	BOOLEAN bConfigChanged;	/* Config Change flag for the same SSID setting */
	/*--- */

	ULONG ExtraInfo;	/* Extra information for displaying status */
	ULONG SystemErrorBitmap;	/* b0: E2PROM version error */

	/*+++Didn't used anymore */
	ULONG MacIcVersion;	/* MAC/BBP serial interface issue solved after ver.D */
	/*--- */

#ifdef SYSTEM_LOG_SUPPORT
	/* --------------------------- */
	/* System event log */
	/* --------------------------- */
	RT_802_11_EVENT_TABLE EventTab;
#endif /* SYSTEM_LOG_SUPPORT */

	BOOLEAN HTCEnable;

	/*****************************************************************************************/
	/*      Statistic related parameters                                                     */
	/*****************************************************************************************/

	BOOLEAN bUpdateBcnCntDone;

	ULONG macwd;
	BOOLEAN bEnableMacWD;
	/* ---------------------------- */
	/* DEBUG paramerts */
	/* ---------------------------- */

	/* ---------------------------- */
	/* rt2860c emulation-use Parameters */
	/* ---------------------------- */
	/*ULONG         rtsaccu[30]; */
	/*ULONG         ctsaccu[30]; */
	/*ULONG         cfendaccu[30]; */
	/*ULONG         bacontent[16]; */
	/*ULONG         rxint[RX_RING_SIZE+1]; */
	/*UCHAR         rcvba[60]; */
	BOOLEAN bLinkAdapt;
	BOOLEAN bForcePrintTX;
	BOOLEAN bForcePrintRX;
	/*BOOLEAN               bDisablescanning;               //defined in RT2870 USB */
	BOOLEAN bStaFifoTest;
	BOOLEAN bProtectionTest;
	BOOLEAN bHCCATest;
	BOOLEAN bGenOneHCCA;
	BOOLEAN bBroadComHT;
	/*+++Following add from RT2870 USB. */
	ULONG BulkOutReq;
	ULONG BulkOutComplete;
	ULONG BulkOutCompleteOther;
	ULONG BulkOutCompleteCancel;	/* seems not use now? */
	ULONG BulkInReq;
	ULONG BulkInComplete;
	ULONG BulkInCompleteFail;
	/*--- */

	struct wificonf WIFItestbed;

	UCHAR		TssiGain;
#ifdef RALINK_ATE
	ATE_INFO ate;
#endif /* RALINK_ATE */

#ifdef DOT11_N_SUPPORT
	struct reordering_mpdu_pool mpdu_blk_pool;
#endif /* DOT11_N_SUPPORT */

	/* statistics count */

	VOID *iw_stats;

#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY blockQueueTab[NUM_OF_TX_RING];
#endif /* BLOCK_NET_IF */

#ifdef CONFIG_AP_SUPPORT
#ifdef IGMP_SNOOP_SUPPORT
	PMULTICAST_FILTER_TABLE pMulticastFilterTable;
	UCHAR IgmpGroupTxRate;
#endif /* IGMP_SNOOP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef MULTIPLE_CARD_SUPPORT
	INT32 MC_RowID;
	STRING MC_FileName[256];
	UINT32 E2P_OFFSET_IN_FLASH[MAX_NUM_OF_MULTIPLE_CARD];
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef RTMP_FLASH_SUPPORT
	UCHAR *eebuf;
#endif /* RTMP_FLASH_SUPPORT */

	ULONG TbttTickCount;	/* beacon timestamp work-around */
#ifdef PCI_MSI_SUPPORT
	BOOLEAN HaveMsi;
#endif /* PCI_MSI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT PeriodicTimer;

#ifdef AP_QLOAD_SUPPORT
	UINT8 FlgQloadEnable;	/* 1: any BSS WMM is enabled */
	ULONG QloadUpTimeLast;	/* last up time */
	UINT8 QloadChanUtil;	/* last QBSS Load, unit: us */
	UINT32 QloadChanUtilTotal;	/* current QBSS Load Total */
	UINT8 QloadChanUtilBeaconCnt;	/* 1~100, default: 50 */
	UINT8 QloadChanUtilBeaconInt;	/* 1~100, default: 50 */
	UINT32 QloadLatestChannelBusyTimePri;
	UINT32 QloadLatestChannelBusyTimeSec;

	/*
	   ex: For 100ms beacon interval,
	   if the busy time in last TBTT is smaller than 5ms, QloadBusyCount[0] ++;
	   if the busy time in last TBTT is between 5 and 10ms, QloadBusyCount[1] ++;
	   ......
	   if the busy time in last TBTT is larger than 95ms, QloadBusyCount[19] ++;

	   Command: "iwpriv ra0 qload show".
	 */

/* provide busy time alarm mechanism */
/* use the function to avoid to locate in some noise environments */
#define QLOAD_FUNC_BUSY_TIME_ALARM

#ifdef QLOAD_FUNC_BUSY_TIME_STATS
#define QLOAD_BUSY_INTERVALS	20	/* partition TBTT to QLOAD_BUSY_INTERVALS */
	/* for primary channel & secondary channel */
	UINT32 QloadBusyCountPri[QLOAD_BUSY_INTERVALS];
	UINT32 QloadBusyCountSec[QLOAD_BUSY_INTERVALS];
#endif /* QLOAD_FUNC_BUSY_TIME_STATS */

#ifdef QLOAD_FUNC_BUSY_TIME_ALARM
#define QLOAD_DOES_ALARM_OCCUR(pAd)	(pAd->FlgQloadAlarmIsSuspended == TRUE)
#define QLOAD_ALARM_EVER_OCCUR(pAd) (pAd->QloadAlarmNumber > 0)
	BOOLEAN FlgQloadAlarmIsSuspended;	/* 1: suspend */

	UINT8 QloadAlarmBusyTimeThreshold;	/* unit: 1/100 */
	UINT8 QloadAlarmBusyNumThreshold;	/* unit: 1 */
	UINT8 QloadAlarmBusyNum;
	UINT8 QloadAlarmDuration;	/* unit: TBTT */

	UINT32 QloadAlarmNumber;	/* total alarm times */
	BOOLEAN FlgQloadAlarm;	/* 1: alarm occurs */

	/* speed up use */
	UINT32 QloadTimePeriodLast;
	UINT32 QloadBusyTimeThreshold;
#else

#define QLOAD_DOES_ALARM_OCCUR(pAd)	0
#endif /* QLOAD_FUNC_BUSY_TIME_ALARM */

#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/* for detect_wmm_traffic() BE TXOP use */
	ULONG OneSecondnonBEpackets;	/* record non BE packets per second */
	UCHAR is_on;

	/* for detect_wmm_traffic() BE/BK TXOP use */
#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND		(1000000/TIME_BASE)
	UCHAR flg_be_adjust;
	ULONG be_adjust_last_time;


#ifdef WSC_INCLUDED
	/* for multiple card */
	UCHAR *pHmacData;

#ifdef CON_WPS
#define CON_WPS_STATUS_DISABLED      0x00
#define CON_WPS_STATUS_AP_RUNNING    0x01
#define CON_WPS_STATUS_APCLI_RUNNING 0x02

	INT conWscStatus;            /* 0x0 Disabled, 0x01 ApRunning, 0x02 ApCliRunning */ 
#endif /* CON_WPS */

	/*   UCHAR                   Wsc_Uuid_E[UUID_LEN_HEX];  // now these two parameters move to structure "WSC_CTRL" */
	/*   UCHAR                   Wsc_Uuid_Str[UUID_LEN_STR]; */
#endif /* WSC_INCLUDED */

#ifdef IKANOS_VX_1X0
	struct IKANOS_TX_INFO IkanosTxInfo;
	struct IKANOS_TX_INFO IkanosRxInfo[HW_BEACON_MAX_NUM + MAX_WDS_ENTRY +
					   MAX_APCLI_NUM + MAX_MESH_NUM];
#endif /* IKANOS_VX_1X0 */


#ifdef DBG_DIAGNOSE
	RtmpDiagStruct DiagStruct;
#endif /* DBG_DIAGNOSE */


	UINT8 FlgCtsEnabled;
	UINT8 PM_FlgSuspend;


#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

	EXT_CAP_INFO_ELEMENT ExtCapInfo;


#ifdef VENDOR_FEATURE1_SUPPORT
	UCHAR FifoUpdateDone, FifoUpdateRx;
#endif /* VENDOR_FEATURE1_SUPPORT */

	UINT8 RFICType;

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	VOID *pCfgDev;
	VOID *pCfg80211_CB;

	BOOLEAN FlgCfg80211Scanning;
	BOOLEAN FlgCfg80211Connecting;
	UCHAR Cfg80211_Alpha2[2];
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef OS_ABL_SUPPORT
#ifdef MAT_SUPPORT
	/* used in OS_ABL */
	BOOLEAN(*MATPktRxNeedConvert) (
		IN PRTMP_ADAPTER        pAd,
		IN PNET_DEV                     net_dev);

	PUCHAR (*MATEngineRxHandle)(
		IN PRTMP_ADAPTER	pAd,
		IN PNDIS_PACKET		pPkt,
		IN UINT				infIdx);
#endif /* MAT_SUPPORT */
#endif /* OS_ABL_SUPPORT */

	UINT32 ContinueMemAllocFailCount;

	struct {
		INT IeLen;
		UCHAR *pIe;
	} ProbeRespIE[MAX_LEN_OF_BSS_TABLE];

	/* purpose: We free all kernel resources when module is removed */
	LIST_HEADER RscTimerMemList;	/* resource timers memory */
	LIST_HEADER RscTaskMemList;	/* resource tasks memory */
	LIST_HEADER RscLockMemList;	/* resource locks memory */
	LIST_HEADER RscTaskletMemList;	/* resource tasklets memory */
	LIST_HEADER RscSemMemList;	/* resource semaphore memory */
	LIST_HEADER RscAtomicMemList;	/* resource atomic memory */

	/* purpose: Cancel all timers when module is removed */
	LIST_HEADER RscTimerCreateList;	/* timers list */

#ifdef OS_ABL_SUPPORT
#ifdef RTMP_PCI_SUPPORT
	RTMP_PCI_CONFIG PciConfig;
#endif /* RTMP_PCI_SUPPORT */
#endif /* OS_ABL_SUPPORT */

#ifdef P2P_SUPPORT
	RT_P2P_CONFIG			P2pCfg;
	NDIS_SPIN_LOCK			P2pTableSemLock;
	RT_P2P_TABLE			P2pTable;
	ULONG					GOBeaconBufNoALen;
	CHAR					GoBeaconBuf[512]; /* NOTE: BeaconBuf should be 4-byte aligned */
	ULONG					BeaconBufLen;
	ULONG					GoBeaconBufLen;
	BOOLEAN 				bIsClearScanTab;   /* TURE, we need to force Scan */
	BOOLEAN 				flg_p2p_init;
	ULONG					flg_p2p_OpStatusFlags;
	UCHAR					P2PChannel;
#ifdef DOT11_N_SUPPORT	
	UINT8					P2PExtChOffset;
#endif /* DOT11_N_SUPPORT */
	UCHAR					P2PCurrentAddress[MAC_ADDR_LEN];	  /* User changed MAC address */
	PNET_DEV				p2p_dev;
#endif /* P2P_SUPPORT */

#ifdef RT3883
	UINT32 FlgCWC;
#endif /* RT3883 */



#ifdef WOW_SUPPORT
#endif /* WOW_SUPPORT */
#ifdef APCLI_CERT_SUPPORT
	BOOLEAN bApCliCertTest;	
#endif /* APCLI_CERT_SUPPORT */
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
	BOOLEAN bUseMultiMacAddrExt;
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */

#ifdef MCS_LUT_SUPPORT
	BOOLEAN bUseHwTxLURate;
#endif /* MCS_LUT_SUPPORT */

#ifdef RT6352
	UCHAR RfBank;
	UCHAR VGA_Gain0_idx;
	UCHAR VGA_Gain1_idx;
	BOOLEAN bCalibrationDone;
	CHAR tx_bw_cal[2];	// bw cal value for RF_R32, 0:20M, 1:40M
	CHAR rx_bw_cal[2];	// bw cal value for RF_R32, 0:20M, 1:40M
	UCHAR Tx0_DPD_ALC_tag0;
	UCHAR Tx0_DPD_ALC_tag1;
	UCHAR Tx1_DPD_ALC_tag0;
	UCHAR Tx1_DPD_ALC_tag1;

	UCHAR Tx0_DPD_ALC_tag0_flag;
	UCHAR Tx0_DPD_ALC_tag1_flag;
	UCHAR Tx1_DPD_ALC_tag0_flag;
	UCHAR Tx1_DPD_ALC_tag1_flag;
	INT32 DoDPDCurrTemperature;
	BOOLEAN DoDPDRunning;
	INT32 DoCalibrationTemperature;
	BOOLEAN bDoReCalibration;
	UINT16 E2p_D0_Value;
	INT32 CurrTemperature;
	UCHAR CurrTemperatureMode;
	CHAR TemperatureRef25C;
	BOOLEAN bRef25CVaild;
	BOOLEAN bLowTemperatureTrigger;
	CHAR BW_Power_Delta;
#ifdef RTMP_TEMPERATURE_COMPENSATION
	CHAR DeltaPwrBeforeTempComp;
#endif /* RTMP_TEMPERATURE_COMPENSATION */
	BOOLEAN bExtPA;
#endif /* RT6352 */

#ifdef SINGLE_SKU_V2
	DL_LIST SingleSkuPwrList;
	CHAR SingleSkuRatePwrDiff[18];
	BOOLEAN bOpenFileSuccess;
#endif /* SINGLE_SKU_V2 */
};

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 
/* The offset of the Tx power tuning entry (zero-based array) */
#define TX_POWER_TUNING_ENTRY_OFFSET			30

/* The lower-bound of the Tx power tuning entry */
#define LOWERBOUND_TX_POWER_TUNING_ENTRY		-30

/* The upper-bound of the Tx power tuning entry in G band */
#define UPPERBOUND_TX_POWER_TUNING_ENTRY(__pAd)		((__pAd)->chipCap.TxAlcTxPowerUpperBound_2G)

#ifdef A_BAND_SUPPORT
/* The upper-bound of the Tx power tuning entry in A band */
#define UPPERBOUND_TX_POWER_TUNING_ENTRY_5G(__pAd)		((__pAd)->chipCap.TxAlcTxPowerUpperBound_5G)
#endif /* A_BAND_SUPPORT */

/* Temperature compensation lookup table */

#define TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET	7

/* The lower/upper power delta index for the TSSI rate table */

#define LOWER_POWER_DELTA_INDEX		0
#define UPPER_POWER_DELTA_INDEX		24

/* The offset of the TSSI rate table */

#define TSSI_RATIO_TABLE_OFFSET	12


/* Get the power delta bound */

#define GET_TSSI_RATE_TABLE_INDEX(x) (((x) > UPPER_POWER_DELTA_INDEX) ? (UPPER_POWER_DELTA_INDEX) : (((x) < LOWER_POWER_DELTA_INDEX) ? (LOWER_POWER_DELTA_INDEX) : ((x))))

/* 802.11b CCK TSSI information */

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


/* 802.11a/g OFDM TSSI information */

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


/* 802.11n HT TSSI information */

typedef struct _HT_TSSI_INFO
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

typedef struct _TSSI_INFO_{
	UCHAR tssi_info_0;
	union {
		CCK_TSSI_INFO cck_tssi_info;
		OFDM_TSSI_INFO ofdm_tssi_info;
		HT_TSSI_INFO ht_tssi_info_1;
		UCHAR byte;
	}tssi_info_1;
	HT_TSSI_INFO ht_tssi_info_2;
}TSSI_INFO;

#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

#ifdef CARRIER_DETECTION_SUPPORT
#define DELAYINTMASK		0x0013fffb
#define INTMASK				0x0013fffb
#define IndMask				0x0013fffc
#define RadarInt			0x00100000
#else
#define DELAYINTMASK		0x0003fffb
#define INTMASK				0x0003fffb
#define IndMask				0x0003fffc
#endif /* CARRIER_DETECTION_SUPPORT */

#define RxINT				0x00000005	/* Delayed Rx or indivi rx */
#define TxDataInt			0x000000fa	/* Delayed Tx or indivi tx */
#define TxMgmtInt			0x00000102	/* Delayed Tx or indivi tx */
#define TxCoherent			0x00020000	/* tx coherent */
#define RxCoherent			0x00010000	/* rx coherent */
#define TxRxCoherent		0x00000400	/* tx rx coherent */
#define McuCommand			0x00000200	/* mcu */
#define PreTBTTInt			0x00001000	/* Pre-TBTT interrupt */
#define TBTTInt				0x00000800		/* TBTT interrupt */
#define GPTimeOutInt			0x00008000		/* GPtimeout interrupt */
#define AutoWakeupInt		0x00004000		/* AutoWakeupInt interrupt */
#define FifoStaFullInt			0x00002000	/*  fifo statistics full interrupt */


/***************************************************************************
  *	Rx Path software control block related data structures
  **************************************************************************/
typedef struct _RX_BLK_
{
/*	RXD_STRUC		RxD; // sample */
	RT28XX_RXD_STRUC	RxD;
	PRXWI_STRUC			pRxWI;
	PHEADER_802_11		pHeader;
	PNDIS_PACKET		pRxPacket;
	UCHAR				*pData;
	USHORT				DataSize;
	USHORT				Flags;
	UCHAR				UserPriority;	/* for calculate TKIP MIC using */
	UCHAR				OpMode;	/* 0:OPMODE_STA 1:OPMODE_AP */
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
#define fRX_P2PGO		0x2000
#define fRX_P2PCLI		0x4000

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


/*	Currently the sizeof(TX_BLK) is 148 bytes. */
typedef struct _TX_BLK_
{
	UCHAR				QueIdx;
	UCHAR				TxFrameType;				/* Indicate the Transmission type of the all frames in one batch */
	UCHAR				TotalFrameNum;				/* Total frame number want to send-out in one batch */
	USHORT				TotalFragNum;				/* Total frame fragments required in one batch */
	USHORT				TotalFrameLen;				/* Total length of all frames want to send-out in one batch */

	QUEUE_HEADER		TxPacketList;
	MAC_TABLE_ENTRY		*pMacEntry;					/* NULL: packet with 802.11 RA field is multicast/broadcast address */
	HTTRANSMIT_SETTING	*pTransmit;
	
	/* Following structure used for the characteristics of a specific packet. */
	PNDIS_PACKET		pPacket;
	PUCHAR				pSrcBufHeader;				/* Reference to the head of sk_buff->data */
	PUCHAR				pSrcBufData;				/* Reference to the sk_buff->data, will changed depends on hanlding progresss */
	UINT				SrcBufLen;					/* Length of packet payload which not including Layer 2 header */

	PUCHAR				pExtraLlcSnapEncap;			/* NULL means no extra LLC/SNAP is required */
#ifndef VENDOR_FEATURE1_SUPPORT
	/*
		Note: Can not insert any other new parameters
		between pExtraLlcSnapEncap & HeaderBuf; Or
		the start address of HeaderBuf will not be aligned by 4.

		But we can not change HeaderBuf[128] to HeaderBuf[32] because
		many codes use HeaderBuf[index].
	*/
	UCHAR				HeaderBuf[128];				/* TempBuffer for TX_INFO + TX_WI + 802.11 Header + padding + AMSDU SubHeader + LLC/SNAP */
#else
	UINT32				HeaderBuffer[32];			/* total 128B, use UINT32 to avoid alignment problem */
	UCHAR				*HeaderBuf;
#endif /* VENDOR_FEATURE1_SUPPORT */
	UCHAR				MpduHeaderLen;				/* 802.11 header length NOT including the padding */
	UCHAR				HdrPadLen;					/* recording Header Padding Length; */
	UCHAR				apidx;						/* The interface associated to this packet */
	UCHAR				Wcid;						/* The MAC entry associated to this packet */
	UCHAR				UserPriority;				/* priority class of packet */
	UCHAR				FrameGap;					/* what kind of IFS this packet use */
	UCHAR				MpduReqNum;					/* number of fragments of this frame */
	UCHAR				TxRate;						/* TODO: Obsoleted? Should change to MCS? */
	UCHAR				CipherAlg;					/* cipher alogrithm */
	PCIPHER_KEY			pKey;
	UCHAR				KeyIdx;						/* Indicate the transmit key index */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	UINT				ApCliIfidx;
	PAPCLI_STRUCT		pApCliEntry;
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	UINT32				Flags;						/*See following definitions for detail. */

	/*YOU SHOULD NOT TOUCH IT! Following parameters are used for hardware-depended layer. */
	ULONG				Priv;						/* Hardware specific value saved in here. */

#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif /* CONFIG_AP_SUPPORT */

#ifdef TXBF_SUPPORT
	UCHAR				TxSndgPkt; /* 1: sounding 2: NDP sounding */
	UCHAR				TxNDPSndgBW;
	UCHAR				TxNDPSndgMcs;
#endif /* TXBF_SUPPORT */

	UCHAR				OpMode;
} TX_BLK, *PTX_BLK;


#define fTX_bRtsRequired			0x0001	/* Indicate if need send RTS frame for protection. Not used in RT2860/RT2870. */
#define fTX_bAckRequired			0x0002	/* the packet need ack response */
#define fTX_bPiggyBack			0x0004	/* Legacy device use Piggback or not */
#define fTX_bHTRate				0x0008	/* allow to use HT rate */
#define fTX_bForceNonQoS		0x0010	/* force to transmit frame without WMM-QoS in HT mode */
#define fTX_bAllowFrag			0x0020	/* allow to fragment the packet, A-MPDU, A-MSDU, A-Ralink is not allowed to fragment */
#define fTX_bMoreData			0x0040	/* there are more data packets in PowerSave Queue */
#define fTX_bWMM				0x0080	/* QOS Data */
#define fTX_bClearEAPFrame		0x0100

#define	fTX_bSwEncrypt			0x0400	/* this packet need to be encrypted by software before TX */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#define fTX_bApCliPacket			0x0200
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
#define fTX_bWDSEntry			0x1000	/* Used when WDS_SUPPORT */
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef UAPSD_SUPPORT
#define	fTX_bWMM_UAPSD_EOSP	0x0800	/* Used when UAPSD_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */


#ifdef WAPI_SUPPORT
#define	fTX_bWPIDataFrame		0x8000	/* indicate this packet is an WPI data frame, it need to be encrypted by software */
#endif /* WAPI_SUPPORT */

#ifdef CLIENT_WDS
#define fTX_bClientWDSFrame		0x10000
#endif /* CLIENT_WDS */

#ifdef DOT11Z_TDLS_SUPPORT
#define fTX_bTdlsEntry				0x20000	/* Used when DOT11Z_TDLS_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */

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
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pData,
	IN	ULONG			DescriptorType)
{
	int size;
	int i;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 RXWISize = pAd->chipCap.RXWISize;
	
	size = ((DescriptorType == TYPE_TXWI) ? TXWISize : RXWISize);
	
	if(DescriptorType == TYPE_TXWI)
	{
		*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		/* Byte 0~3 */
		*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData+4)));	/* Byte 4~7 */
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
	*(p1+1) = *(p2+1); /* Word 1; this must be written back last */
}
#endif /* RTMP_MAC_PCI */


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
	*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		/* Byte 0~3 */
	*((UINT32 *)(pData + 8)) = SWAP32(*((UINT32 *)(pData+8)));	/* Byte 8~11 */
	*((UINT32 *)(pData +12)) = SWAP32(*((UINT32 *)(pData + 12)));	/* Byte 12~15 */
	*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData + 4)));				/* Byte 4~7, this must be swapped last */
}
#endif /* RTMP_MAC_PCI */

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

	/* swab 16 bit fields - Frame Control field */
	if(Dir == DIR_READ)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}

	pFrame = (PHEADER_802_11) pData;
	pMacHdr = (PUCHAR) pFrame;

	/* swab 16 bit fields - Duration/ID field */
	*(USHORT *)(pMacHdr + 2) = SWAP16(*(USHORT *)(pMacHdr + 2));

	if (pFrame->FC.Type != BTYPE_CNTL)
	{
		/* swab 16 bit fields - Sequence Control field */
		*(USHORT *)(pMacHdr + 22) = SWAP16(*(USHORT *)(pMacHdr + 22));
	}

	if(pFrame->FC.Type == BTYPE_MGMT)
	{
		switch(pFrame->FC.SubType)
		{
			case SUBTYPE_ASSOC_REQ:
			case SUBTYPE_REASSOC_REQ:
				/* swab 16 bit fields - CapabilityInfo field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - Listen Interval field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_ASSOC_RSP:
			case SUBTYPE_REASSOC_RSP:
				/* swab 16 bit fields - CapabilityInfo field */
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - Status Code field */
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - AID field */
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
					/* swab 16 bit fields - Auth Alg No. field */
					pMacHdr += sizeof(HEADER_802_11);
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					/* swab 16 bit fields - Auth Seq No. field */
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					/* swab 16 bit fields - Status Code field */
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				}
				break;

			case SUBTYPE_BEACON:
			case SUBTYPE_PROBE_RSP:
				/* swab 16 bit fields - BeaconInterval field */
				pMacHdr += (sizeof(HEADER_802_11) + TIMESTAMP_LEN);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				/* swab 16 bit fields - CapabilityInfo field */
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
					/* swab 16 bit fields - Reason code field */
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
				/* For Block Ack packet, the HT_CONTROL field is in the same offset with Addr3 */
				*(UINT32 *)(&pFrame->Addr3[0]) = SWAP32(*(UINT32 *)(&pFrame->Addr3[0]));
				break;

			case SUBTYPE_ACK:
				/*For ACK packet, the HT_CONTROL field is in the same offset with Addr2 */
				*(UINT32 *)(&pFrame->Addr2[0])=	SWAP32(*(UINT32 *)(&pFrame->Addr2[0]));
				break;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("Invalid Frame Type!!!\n"));
	}

	/* swab 16 bit fields - Frame Control */
	if(Dir == DIR_WRITE)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}
}
#endif /* RT_BIG_ENDIAN */


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
/*			memset(*ppMacAddr, 0, ETH_LENGTH_OF_ADDRESS); */
			*(*ppMacAddr) = 0x33;
			*(*ppMacAddr + 1) = 0x33;
			*(*ppMacAddr + 2) = pIpAddr[12];
			*(*ppMacAddr + 3) = pIpAddr[13];
			*(*ppMacAddr + 4) = pIpAddr[14];
			*(*ppMacAddr + 5) = pIpAddr[15];
			break;

		case ETH_P_IP:
		default:
/*			memset(*ppMacAddr, 0, ETH_LENGTH_OF_ADDRESS); */
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

/* */
/*  Private routines in rtmp_init.c */
/* */

NDIS_STATUS RTMPAllocTxRxRingMemory(
	IN  PRTMP_ADAPTER   pAd);

#ifdef RESOURCE_PRE_ALLOC
NDIS_STATUS RTMPInitTxRxRingMemory(
	IN RTMP_ADAPTER *pAd);
#endif /* RESOURCE_PRE_ALLOC */

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
#endif/*WSC_INCLUDED*/


#ifdef RTMP_RF_RW_SUPPORT
VOID NICInitRFRegisters(
	IN PRTMP_ADAPTER pAd);

VOID RTMP_ReadRF(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RegID,
	OUT	PUCHAR			pValue1,
	OUT PUCHAR			pValue2,
	IN	UCHAR			BitMask);

VOID RTMP_WriteRF(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RegID,
	IN	UCHAR			Value,
	IN	UCHAR			BitMask);

NDIS_STATUS	RT30xxWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT30xxReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

NDIS_STATUS	RT635xWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT635xReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			bank,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);

#endif /* RTMP_RF_RW_SUPPORT */

NDIS_STATUS NICInitBBP(
	IN RTMP_ADAPTER *pAd);

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

#ifdef RTMP_RBUS_SUPPORT
VOID	NICResetFromError(
	IN	PRTMP_ADAPTER	pAd);
#endif /* RTMP_RBUS_SUPPORT */

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

#ifdef CONFIG_AP_SUPPORT
VOID ClearTxRingClientAck(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry);
#endif /* CONFIG_AP_SUPPORT */

#ifdef FIFO_EXT_SUPPORT
BOOLEAN NicGetMacFifoTxCnt(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry);

VOID AsicFifoExtSet(
	IN RTMP_ADAPTER *pAd);

VOID AsicFifoExtEntryClean(
	IN RTMP_ADAPTER * pAd, 
	IN MAC_TABLE_ENTRY *pEntry);
#endif /* FIFO_EXT_SUPPORT */

VOID NicResetRawCounters(RTMP_ADAPTER *pAd);

VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC *pStaTxCnt0,
	IN TX_STA_CNT1_STRUC *pStaTxCnt1);
	
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

VOID	RTMP_TimerListAdd(
	IN	PRTMP_ADAPTER			pAd,
	IN	VOID					*pRsc);

VOID	RTMP_TimerListRelease(
	IN	PRTMP_ADAPTER			pAd);

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

VOID	RTMPReleaseTimer(
	IN  PRALINK_TIMER_STRUCT    pTimer,
	OUT BOOLEAN                 *pCancelled);

VOID RTMPEnableRxTx(
	IN PRTMP_ADAPTER	pAd);

VOID AntCfgInit(
	IN  PRTMP_ADAPTER   pAd);

/* */
/* prototype in action.c */
/* */
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
#endif /* DOT11_N_SUPPORT */

VOID SendPSMPAction(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Wcid,
	IN UCHAR			Psmp);
				   
#ifdef CONFIG_AP_SUPPORT				   
VOID SendBeaconRequest(
	IN PRTMP_ADAPTER		pAd,
	IN UCHAR				Wcid);
#endif /* CONFIG_AP_SUPPORT */

VOID PeerRMAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPublicAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

#ifdef CONFIG_STA_SUPPORT
VOID StaPublicAction(
	IN PRTMP_ADAPTER pAd, 
	IN BSS_2040_COEXIST_IE *pBss2040CoexIE);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID ApPublicAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
#endif /* CONFIG_AP_SUPPORT */

VOID PeerBSSTranAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

#ifdef DOT11_N_SUPPORT
VOID PeerHTAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
#endif /* DOT11_N_SUPPORT */

VOID PeerQOSAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

#ifdef QOS_DLS_SUPPORT
VOID PeerDLSAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);
#endif /* QOS_DLS_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
VOID DlsParmFill(
	IN PRTMP_ADAPTER pAd, 
	IN OUT MLME_DLS_REQ_STRUCT *pDlsReq,
	IN PRT_802_11_DLS pDls,
	IN USHORT reason);
#endif /* QOS_DLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

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
#endif /* DOT11N_DRAFT3 */

VOID AsicEnableRalinkBurstMode(
	IN PRTMP_ADAPTER pAd);

VOID AsicDisableRalinkBurstMode(
	IN PRTMP_ADAPTER pAd);
#endif /* DOT11_N_SUPPORT */
	
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
#endif /* DOT11_N_SUPPORT */

VOID HTIOTCheck(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR     BatRecIdx);

/* */
/* Private routines in rtmp_data.c */
/* */
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
#endif /* CONFIG_AP_SUPPORT */


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
	IN  PNDIS_PACKET    pPacket,
	IN	BOOLEAN			FlgDataQForce,
	IN	BOOLEAN			FlgIsLocked);

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
#endif /* RTMP_MAC_PCI */

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
	IN	BOOLEAN			NSeq,		/* HW new a sequence. */
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
	IN	BOOLEAN			bQosNull,
	IN  USHORT			PwrMgmt);

#ifdef APCLI_SUPPORT
VOID	ApCliRTMPReportMicError(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PCIPHER_KEY 	pWpaKey,
	IN	INT			ifIndex);

VOID   ApCliWpaDisassocApAndBlockAssoc(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);
#endif/*APCLI_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
VOID RTMPReportMicError(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PCIPHER_KEY     pWpaKey);

VOID	WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER    pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID    WpaDisassocApAndBlockAssoc(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

VOID WpaStaPairwiseKeySetting(
	IN	PRTMP_ADAPTER	pAd);

VOID WpaStaGroupKeySetting(
	IN	PRTMP_ADAPTER	pAd);

VOID    WpaSendEapolStart(
	IN	PRTMP_ADAPTER	pAdapter,
	IN  PUCHAR          pBssid);

#endif /* CONFIG_STA_SUPPORT */



BOOLEAN RTMPFreeTXDUponTxDmaDone(
	IN PRTMP_ADAPTER    pAd, 
	IN UCHAR            QueIdx);

BOOLEAN RTMPCheckEtherType(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket,
	IN	PMAC_TABLE_ENTRY pMacEntry,
	IN	UCHAR			OpMode,
	OUT PUCHAR pUserPriority,
	OUT PUCHAR pQueIdx);


VOID RTMPCckBbpTuning(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UINT			TxRate);
/* */
/* MLME routines */
/* */

VOID InitRfPaModeTable(
	IN struct _RTMP_ADAPTER 	*pAd);
	
/* Asic/RF/BBP related functions */
VOID AsicGetTxPowerOffset(
	IN PRTMP_ADAPTER 			pAd,
	IN PULONG					TxPwr);

VOID AsicGetAutoAgcOffsetForExternalTxAlc(
	IN PRTMP_ADAPTER 		pAd,
	IN PCHAR 				pDeltaPwr,
	IN PCHAR 				pTotalDeltaPwr,
	IN PCHAR 				pAgcCompensate,
	IN PCHAR 				pDeltaPowerByBbpR1);
	
#ifdef RTMP_TEMPERATURE_COMPENSATION
VOID AsicGetAutoAgcOffsetForTemperatureSensor(
	IN PRTMP_ADAPTER 		pAd,
	IN PCHAR				pDeltaPwr,
	IN PCHAR				pTotalDeltaPwr,
	IN PCHAR				pAgcCompensate,
	IN PCHAR 				pDeltaPowerByBbpR1);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#ifdef SINGLE_SKU
VOID GetSingleSkuDeltaPower(
	IN 		PRTMP_ADAPTER 	pAd,
	IN 		PCHAR 			pTotalDeltaPower,
	INOUT 	PULONG			pSingleSKUTotalDeltaPwr,
	INOUT  	PUCHAR              	pSingleSKUBbpR1Offset);
#endif /* SINGLE_SKU*/

VOID AsicPercentageDeltaPower(
	IN 		PRTMP_ADAPTER 		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1);

VOID AsicCompensatePowerViaBBP(
	IN 		PRTMP_ADAPTER 		pAd,
	INOUT	PCHAR				pTotalDeltaPower);

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

#ifdef THERMAL_PROTECT_SUPPORT	
VOID thermal_protection(
	IN  PRTMP_ADAPTER   pAd);
#endif /* THERMAL_PROTECT_SUPPORT */

VOID AsicResetBBPAgent(
	IN PRTMP_ADAPTER pAd);

#ifdef CONFIG_STA_SUPPORT
VOID AsicSleepThenAutoWakeup(
	IN  PRTMP_ADAPTER   pAd, 
	IN  USHORT TbttNumToNextWakeUp);

VOID AsicForceSleep(
	IN PRTMP_ADAPTER pAd);

VOID AsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN    bFromTx);
#endif /* CONFIG_STA_SUPPORT */

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
#endif /* DOT11_N_SUPPORT */

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
	IN UCHAR         Command,
	IN UCHAR         Token,
	IN UCHAR         Arg0,
	IN UCHAR         Arg1,
	IN BOOLEAN in_atomic);

BOOLEAN AsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked);

#ifdef RTMP_RBUS_SUPPORT
int RtmpAsicSendCommandToSwMcu(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR Command, 
	IN UCHAR Token,
	IN UCHAR Arg0, 
	IN UCHAR Arg1,
	IN BOOLEAN FlgIsNeedLocked);
#endif /* RTMP_RBUS_SUPPORT */

#ifdef RTMP_MAC_PCI
BOOLEAN AsicCheckCommanOk(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command);
#endif /* RTMP_MAC_PCI */

#ifdef WAPI_SUPPORT
VOID AsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high);
#endif /* WAPI_SUPPORT */

#ifdef VCORECAL_SUPPORT
VOID AsicVCORecalibration(
	IN PRTMP_ADAPTER pAd);
#endif /* VCORECAL_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
UINT32 StreamModeRegVal(
	IN RTMP_ADAPTER *pAd);

VOID AsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled);

VOID RtmpStreamModeInit(
	IN RTMP_ADAPTER *pAd);

/*
	Update the Tx chain address
	Parameters
		pAd: The adapter data structure
		pMacAddress: The MAC address of the peer STA

	Return Value:
		None
*/
VOID AsicUpdateTxChainAddress(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pMacAddress);

/*
	Enable the stream mode
	Parameters:
		pAd: The adapter data structure

	Return Value:
		None
*/
VOID AsicEnableStreamMode(
	IN PRTMP_ADAPTER pAd);

/*
	Disable the stream mode
	Parameters:
		pAd: The adapter data structure
	Return Value:
		None
*/
VOID AsicDisableStreamMode(
	IN PRTMP_ADAPTER pAd);

INT Set_StreamMode_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

INT Set_StreamModeMac_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT Set_StreamModeMCS_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  PSTRING         arg);
#endif /* STREAM_MODE_SUPPORT */

#ifdef WOW_SUPPORT
#endif /* WOW_SUPPORT */

VOID MacAddrRandomBssid(
	IN  PRTMP_ADAPTER   pAd, 
	OUT PUCHAR pAddr);

VOID MgtMacHeaderInit(
	IN  PRTMP_ADAPTER     pAd, 
	IN OUT PHEADER_802_11 pHdr80211, 
	IN UCHAR SubType, 
	IN UCHAR ToDs, 
	IN PUCHAR pDA, 
#ifdef P2P_SUPPORT
	IN PUCHAR pSA,
#endif /* P2P_SUPPORT */
	IN PUCHAR pBssid);

VOID MgtMacHeaderInitExt(
    IN  PRTMP_ADAPTER   pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN PUCHAR pDA,
    IN PUCHAR pSA,
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
#endif /* DOT11_N_SUPPORT */

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
	IN ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
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
	IN ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
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

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

VOID BssTableSsidSort(
	IN  PRTMP_ADAPTER   pAd, 
	OUT BSS_TABLE *OutTab, 
	IN  CHAR Ssid[], 
	IN  UCHAR SsidLen);

VOID  BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder);

VOID BssCipherParse(
	IN OUT  PBSS_ENTRY  pBss);

NDIS_STATUS  MlmeQueueInit(
	IN PRTMP_ADAPTER pAd,
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
	IN UCHAR Signal,
	IN UCHAR OpMode);

#ifdef WSC_INCLUDED
BOOLEAN MlmeEnqueueForWsc(
	IN PRTMP_ADAPTER	pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg);
#endif /* WSC_INCLUDED */

BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue, 
	OUT MLME_QUEUE_ELEM **Elem);

VOID    MlmeRestartStateMachine(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN  MlmeQueueEmpty(
	IN MLME_QUEUE *Queue);

BOOLEAN  MlmeQueueFull(
	IN MLME_QUEUE *Queue,
	IN UCHAR SendId);

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

/*---------------------------------------------- */
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

/*============================================= */

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

/* */
/* Private routines in dls.c */
/* */
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
#endif /* QOS_DLS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
void DlsStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeDlsReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDlsReqAction(
    IN PRTMP_ADAPTER	pAd, 
    IN MLME_QUEUE_ELEM	*Elem);

VOID PeerDlsRspAction(
    IN PRTMP_ADAPTER	pAd, 
    IN MLME_QUEUE_ELEM	*Elem);

VOID MlmeDlsTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDlsTearDownAction(
    IN PRTMP_ADAPTER	pAd, 
    IN MLME_QUEUE_ELEM	*Elem);

VOID RTMPCheckDLSTimeOut(
	IN PRTMP_ADAPTER	pAd);

BOOLEAN RTMPRcvFrameDLSCheck(
	IN PRTMP_ADAPTER	pAd,
	IN PHEADER_802_11	pHeader,
	IN ULONG			Len,
	IN PRT28XX_RXD_STRUC	pRxD);

INT	RTMPCheckDLSFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pDA);

VOID RTMPSendDLSTearDownFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pDA);

NDIS_STATUS RTMPSendSTAKeyRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA);

NDIS_STATUS RTMPSendSTAKeyHandShake(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA);

VOID DlsTimeoutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

BOOLEAN MlmeDlsReqSanity(
	IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PRT_802_11_DLS *pDLS,
    OUT PUSHORT pReason);

INT Set_DlsEntryInfo_Display_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR arg);

MAC_TABLE_ENTRY *MacTableInsertDlsEntry(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PUCHAR	pAddr,
	IN  UINT	DlsEntryIdx);

BOOLEAN MacTableDeleteDlsEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid,
	IN PUCHAR pAddr);

MAC_TABLE_ENTRY *DlsEntryTableLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pAddr,
	IN BOOLEAN	bResetIdelCount);

MAC_TABLE_ENTRY *DlsEntryTableLookupByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	wcid,
	IN PUCHAR	pAddr,
	IN BOOLEAN	bResetIdelCount);

INT	Set_DlsAddEntry_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DlsTearDownEntry_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* QOS_DLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

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
#endif /* QOS_DLS_SUPPORT */

BOOLEAN PeerProbeReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *SsidLen,
    OUT BOOLEAN *bRequestRssi);

/*======================================== */

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
/*========================================= */

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
#endif /* QOS_DLS_SUPPORT */


VOID LinkUp(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR BssType);

VOID LinkDown(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN         IsReqFromAP);

VOID IterateOnBssTab(
	IN  PRTMP_ADAPTER   pAd);

VOID IterateOnBssTab2(
	IN  PRTMP_ADAPTER   pAd);

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

VOID MlmeForceJoinReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeForceScanReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

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
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR	OpMode);


ULONG MakeIbssBeacon(
	IN  PRTMP_ADAPTER   pAd);

#ifdef CONFIG_STA_SUPPORT
VOID InitChannelRelatedValue(
	IN  PRTMP_ADAPTER   pAd);

VOID AdjustChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR *pBwFallBack,
	IN USHORT ifIndex,
	IN BOOLEAN BandWidth,
	IN UCHAR PriCh,
	IN UCHAR ExtraCh);
#endif /* CONFIG_STA_SUPPORT */

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
#ifdef CONFIG_STA_SUPPORT
	OUT UCHAR		 *pPreNHtCapabilityLen,
#endif /* CONFIG_STA_SUPPORT */
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
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

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
        OUT HT_CAPABILITY_IE *pHtCapability,
        OUT ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
        OUT UCHAR *pHtCapabilityLen,
        OUT UCHAR *pAddHtInfoLen,
        OUT UCHAR *pNewExtChannelOffset,
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

#ifdef TXBF_SUPPORT
BOOLEAN MlmeTxBfAllowed(
	IN PRTMP_ADAPTER 		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN struct _RTMP_TX_RATE_SWITCH *pTxRate);
#endif /* TXBF_SUPPORT */

#ifdef AGS_SUPPORT
INT Show_AGS_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  PSTRING			arg);

/* */
/* The dynamic Tx rate switching for AGS (Adaptive Group Switching) */
/* */
/* Parameters */
/*	pAd: The adapter data structure */
/*	pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry */
/*	pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table */
/*	TableSize: The size, in bytes, of the specified Tx rate switching table */
/*	pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information */
/* */
/* Return Value: */
/*	None */
/* */
VOID MlmeDynamicTxRateSwitchingAGS(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY pEntry, 
	IN PUCHAR pTable, 
	IN UCHAR TableSize, 
	IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
	IN UCHAR InitTxRateIdx);

/* */
/* Auto Tx rate faster train up/down for AGS (Adaptive Group Switching) */
/* */
/* Parameters */
/*	pAd: The adapter data structure */
/*	pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry */
/*	pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table */
/*	TableSize: The size, in bytes, of the specified Tx rate switching table */
/*	pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information */
/* */
/* Return Value: */
/*	None */
/* */
VOID StaQuickResponeForRateUpExecAGS(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY pEntry, 
	IN PUCHAR pTable, 
	IN UCHAR TableSize, 
	IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
	IN UCHAR InitTxRateIdx);
#endif /* AGS_SUPPORT */

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
#endif /* DOT11_N_SUPPORT */

VOID    RTMPCheckRates(
	IN      PRTMP_ADAPTER   pAd,
	IN OUT  UCHAR           SupRate[],
	IN OUT  UCHAR           *SupRateLen);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN RTMPCheckChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		CentralChannel,
	IN UCHAR		Channel);
#endif /* CONFIG_STA_SUPPORT */

BOOLEAN 	RTMPCheckHt(
	IN		PRTMP_ADAPTER	pAd,
	IN		UCHAR	Wcid,
	IN OUT	HT_CAPABILITY_IE			*pHtCapability,
	IN OUT	ADD_HT_INFO_IE			*pAddHtInfo);

VOID RTMPUpdateMlmeRate(
	IN PRTMP_ADAPTER	pAd);

CHAR RTMPMaxRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

CHAR RTMPAvgRssi(
        IN PRTMP_ADAPTER	pAd,
        IN RSSI_SAMPLE		*pRssi);


CHAR RTMPMinSnr(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Snr0,
	IN CHAR				Snr1);

VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);



#ifdef RT305x
VOID NICInitRT305xRFRegisters(
	IN PRTMP_ADAPTER pAd);
#endif /* RT305x */

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

BOOLEAN RTMPAdjustFrequencyOffset(
	IN PRTMP_ADAPTER    pAd,
	INOUT PUCHAR pRefFreqOffset);

NDIS_STATUS MlmeInit(
	IN  PRTMP_ADAPTER   pAd);


#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

#ifdef RTMP_TEMPERATURE_COMPENSATION
VOID InitLookupTable(
	IN PRTMP_ADAPTER pAd);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

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

/* */
/* Prototypes of function definition in cmm_tkip.c */
/* */
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

/* */
/* Prototypes of function definition in cmm_cfg.c */
/* */
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
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

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
	IN INT			keyStringLen,
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
#endif /* WSC_INCLUDED */

INT	Set_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef RT5350
VOID RT5350SetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);

INT Set_Hw_Antenna_Div_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif /* RT5350 */

#ifdef RT6352
INT Set_RfBankSel_Proc(
    IN  PRTMP_ADAPTER pAd, 
    IN  PSTRING	arg);

#ifdef RTMP_TEMPERATURE_CALIBRATION
INT Set_TemperatureCAL_Proc(
    IN  PRTMP_ADAPTER pAd, 
    IN  PSTRING	arg);
#endif /* RTMP_TEMPERATURE_CALIBRATION */
#endif /* RT6352 */

/* */
/* Prototypes of function definition in cmm_info.c */
/* */
NDIS_STATUS RTMPWPARemoveKeyProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  PVOID           pBuf);

VOID    RTMPWPARemoveAllKeys(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN RTMPCheckStrPrintAble(
    IN  CHAR *pInPutStr, 
    IN  UCHAR strLen);

VOID RTMPSetDefaultChannel(
	IN	PRTMP_ADAPTER	pAd);
    
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
#endif /* DOT11_N_SUPPORT */

VOID RTMPDisableDesiredHtInfo(
	IN	PRTMP_ADAPTER	pAd);

#ifdef SYSTEM_LOG_SUPPORT
VOID RtmpDrvSendWirelessEvent(
	IN	VOID			*pAdSrc,
	IN	USHORT			Event_flag,
	IN	PUCHAR 			pAddr,
	IN  UCHAR			BssIdx,
	IN	CHAR			Rssi);
#else
#define RtmpDrvSendWirelessEvent(_pAd, _Event_flag, _pAddr, _BssIdx, _Rssi)
#endif /* SYSTEM_LOG_SUPPORT */
	
CHAR    ConvertToRssi(
	IN PRTMP_ADAPTER  pAd,
	IN CHAR				Rssi,
	IN UCHAR    RssiNumber);

CHAR    ConvertToSnr(
	IN PRTMP_ADAPTER  pAd,
	IN UCHAR				Snr);

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

#endif /* DOT11N_DRAFT3 */


VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd);


VOID APAsicRxAntEvalTimeout(
	IN PRTMP_ADAPTER	pAd);


/* */
/* function prototype in ap_wpa.c */
/* */
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

#ifdef TXBF_SUPPORT
VOID eTxBfProbeTimerExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* TXBF_SUPPORT */

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

VOID Start2WayGroupHSExec(
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
#endif /* DOT1X_SUPPORT */

/* timeout -- ms */

#ifdef RESOURCE_PRE_ALLOC
VOID RTMPResetTxRxRingMemory(
	IN  RTMP_ADAPTER   *pAd);
#endif /* RESOURCE_PRE_ALLOC */

VOID RTMPFreeTxRxRingMemory(
    IN  PRTMP_ADAPTER   pAd);


BOOLEAN RTMP_FillTxBlkInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk);




 void announce_802_3_packet(
	IN	VOID			*pAdSrc,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			OpMode);

#ifdef DOT11_N_SUPPORT
UINT BA_Reorder_AMSDU_Annnounce(
	IN	PRTMP_ADAPTER	pAd, 	
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			OpMode);
#endif /* DOT11_N_SUPPORT */

PNET_DEV get_netdev_from_bssid(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			FromWhichBSSID);



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
#endif /* DOT11_N_SUPPORT */

BOOLEAN ba_reordering_resource_init(PRTMP_ADAPTER pAd, int num);
void ba_reordering_resource_release(PRTMP_ADAPTER pAd);

INT ComputeChecksum(
	IN UINT PIN);

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
    IN  BOOLEAN         bFromApcli,
	IN	UCHAR	apidx);

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_WscVendorPinCode_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

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
#endif /* WSC_STA_SUPPORT */
/* */
/* prototype in wsc.c */
/* */
BOOLEAN	WscMsgTypeSubst(
	IN	UCHAR	EAPType,
	IN	UCHAR	EAPCode,
	OUT	INT	    *MsgType);

VOID    WscStateMachineInit(
	IN	PRTMP_ADAPTER		pAd, 
	IN	STATE_MACHINE		*S, 
	OUT STATE_MACHINE_FUNC Trans[]);

#ifdef IWSC_SUPPORT
void	IWSC_StateMachineInit(
    IN  PRTMP_ADAPTER pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID	IWSC_Init(
	IN  IN PRTMP_ADAPTER pAd);
#endif // IWSC_SUPPORT //

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
#endif /* CONFIG_AP_SUPPORT */

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

VOID	WscSendEapReqId(
	IN	PRTMP_ADAPTER		pAd,
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				CurOpMode);

VOID    WscSendEapolStart(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pBssid,
	IN  UCHAR			CurOpMode);

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
    IN  UCHAR               OpMode,	/* 0: AP Mode, 1: AP Client Mode, 2: STA Mode */
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
	IN UINT   eventID,
	IN UCHAR  CurOpMode);

	
int WscSendUPnPMessage(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				apIdx,
	IN	USHORT				msgType,
	IN	USHORT				msgSubType,
	IN	PUCHAR				pData,
	IN	INT					dataLen,
	IN	UINT				eventID,
	IN	UINT				toIPAddr,
	IN	PUCHAR				pMACAddr,
	IN  UCHAR				CurOpMode);

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
	IN  UCHAR  	AuthorizedMACsLen,
	IN  UCHAR	CurOpMode);

VOID    WscBuildProbeRespIE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	UCHAR respType,
	IN	UCHAR scState,
	IN	BOOLEAN b_selRegistrar,
	IN	USHORT devPwdId,
	IN	USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  INT   AuthorizedMACsLen,
	IN  UCHAR	CurOpMode);


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
#endif /* CONFIG_AP_SUPPORT */

VOID WscBuildProbeReqIE(
	IN PWSC_CTRL pWscControl,
	IN  UCHAR			CurOpMode,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

VOID WscBuildAssocReqIE(
	IN  PWSC_CTRL		pWscControl,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

#ifdef CONFIG_STA_SUPPORT
#ifdef IWSC_SUPPORT
VOID	IWSC_Stop(
	IN  PRTMP_ADAPTER 	pAd,
	IN  BOOLEAN			bSendNotification);

VOID	IWSC_T1TimerAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);

VOID	IWSC_T2TimerAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);

VOID	IWSC_EntryTimerAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);

VOID	IWSC_DevQueryAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);

BOOLEAN	IWSC_PeerEapolStart(
    IN  PRTMP_ADAPTER		pAd,
    IN  PMAC_TABLE_ENTRY 	pEntry);

VOID 	IWSC_AddSmpbcEnrollee(
	IN  PRTMP_ADAPTER	pAd,
	IN  PUCHAR			pPeerAddr);

BOOLEAN IWSC_IpContentForCredential(
	IN  PRTMP_ADAPTER	pAd);
#endif /* IWSC_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

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
#endif /* CONFIG_AP_SUPPORT */
	IN  PWSC_CTRL       pWscControl);

VOID WscInit(
	IN	PRTMP_ADAPTER	pAd,
    IN  BOOLEAN         bFromApcli,	
	IN  UCHAR       	BssIndex);

#ifdef CON_WPS
VOID WscConWpsStop(
        IN  PRTMP_ADAPTER   pAd,
        IN  BOOLEAN         bFromApCli,
	IN  PWSC_CTRL       pWscControl);
#endif /* CON_WPS */

BOOLEAN	ValidateChecksum(
	IN UINT PIN);

UINT	WscRandomGen4digitPinCode(
	IN	PRTMP_ADAPTER	pAd);

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
	IN  PWSC_CTRL		pWscControl,
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
	IN	PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL   	pWscControl);

VOID    WscScanExec(
	IN	PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL   	pWscControl);

BOOLEAN WscPBCExec(
	IN	PRTMP_ADAPTER	pAd,
	IN  BOOLEAN			bFromM2,
	IN  PWSC_CTRL       pWscControl);

VOID    WscPBCBssTableSort(
	IN	PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL       pWscControl);

VOID	WscGenRandomKey(
	IN  	PRTMP_ADAPTER	pAd,
	IN  	PWSC_CTRL       pWscControl,
	INOUT	PUCHAR			pKey,
	INOUT	PUSHORT			pKeyLen);

VOID	WscCreateProfileFromCfg(
	IN	PRTMP_ADAPTER		pAd,
	IN  UCHAR               OpMode,         /* 0: AP Mode, 1: AP Client Mode, 2: STA Mode */
	IN  PWSC_CTRL           pWscControl,
	OUT PWSC_PROFILE        pWscProfile); 

void    WscWriteConfToPortCfg(
    IN  PRTMP_ADAPTER   pAd,
    IN  PWSC_CTRL       pWscControl,
    IN  PWSC_CREDENTIAL pCredential,
    IN  BOOLEAN         bEnrollee);

#ifdef APCLI_SUPPORT
void    WscWriteConfToApCliCfg(
    IN  PRTMP_ADAPTER   pAd,
    IN  PWSC_CTRL       pWscControl,
    IN  PWSC_CREDENTIAL pCredential,
    IN  BOOLEAN         bEnrollee);
#endif /* APCLI_SUPPORT */

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
#endif /* CONFIG_AP_SUPPORT */

VOID WscCheckWpsIeFromWpsAP(
    IN  PRTMP_ADAPTER 	pAd, 
    IN  PEID_STRUCT		pEid,
    OUT PUSHORT			pDPIDFromAP);

#ifdef CONFIG_STA_SUPPORT
VOID PeerProbeRespAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

ULONG WscSearchWpsApBySSID(
	IN  PRTMP_ADAPTER 	pAd, 
	IN PUCHAR	 		pSsid,
	IN UCHAR	 		SsidLen,
	IN INT		 		WscMode);
#endif /* CONFIG_STA_SUPPORT */

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
#endif /* WSC_LED_SUPPORT */



#ifdef CONFIG_AP_SUPPORT
VOID WscUpdatePortCfgTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* CONFIG_AP_SUPPORT */

VOID	WscCheckPeerDPID(
	IN  PRTMP_ADAPTER	pAd,
	IN  PFRAME_802_11 	Fr,
	IN  PUCHAR			eid_data,
	IN  INT				eid_len);

VOID	WscClearPeerList(
	IN  PLIST_HEADER	pWscEnList);

PWSC_PEER_ENTRY	WscFindPeerEntry(
	IN  PLIST_HEADER	pWscEnList,
	IN	PUCHAR			pMacAddr);

VOID	WscDelListEntryByMAC(
	PLIST_HEADER		pWscEnList,
	IN  PUCHAR			pMacAddr);

VOID	WscInsertPeerEntryByMAC(
	IN  PLIST_HEADER	pWscEnList,
	IN	PUCHAR			pMacAddr);

#ifdef CONFIG_AP_SUPPORT
INT WscApShowPeerList(
	IN  PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
INT WscStaShowPeerList(
	IN  PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif /* CONFIG_STA_SUPPORT */

VOID	WscMaintainPeerList(
	IN  PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL		pWpsCtrl);

VOID	WscAssignEntryMAC(
	IN  PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL		pWpsCtrl);

#ifdef WSC_V2_SUPPORT
#ifdef CONFIG_AP_SUPPORT
VOID 	WscOnOff(
	IN  PRTMP_ADAPTER	pAd,
	IN  INT				ApIdx,
	IN  BOOLEAN			bOff);

VOID	WscAddEntryToAclList(
	IN  PRTMP_ADAPTER	pAd,
	IN	INT				ApIdx,
	IN  PUCHAR			pMacAddr);

VOID WscSetupLockTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID	WscCheckPinAttackCount(
	IN  PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL		pWscControl);
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN	WscGenV2Msg(
	IN  PWSC_CTRL		pWpsCtrl,
	IN  BOOLEAN			bSelRegistrar,
	IN	PUCHAR			pAuthorizedMACs,
	IN  INT   			AuthorizedMACsLen,
	OUT	UCHAR			**pOutBuf,
	OUT	INT				*pOutBufLen);

BOOLEAN	WscParseV2SubItem(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	USHORT			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen);

VOID	WscSendEapFragAck(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry);

VOID	WscSendEapFragData(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry);
#endif /* WSC_V2_SUPPORT */

BOOLEAN WscGetDataFromPeerByTag(
    IN  PRTMP_ADAPTER 	pAd, 
    IN  PUCHAR			pIeData,
    IN  INT				IeDataLen,
    IN  USHORT			WscTag,
    OUT PUCHAR			pWscBuf,
    OUT PUSHORT			pWscBufLen);

#endif /* WSC_INCLUDED */




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
	
/*//////// common ioctl functions ////////*/
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
#endif /* RTMP_MAC_PCI */

#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);
#endif /* AGGREGATION_SUPPORT */

#ifdef INF_PPA_SUPPORT
INT	Set_INF_AMAZON_SE_PPA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			arg);

INT ifx_ra_start_xmit (
	IN	struct net_device *rx_dev, 
	IN	struct net_device *tx_dev,
	IN	struct sk_buff *skb,
	IN	int len);
#endif /* INF_PPA_SUPPORT */

INT	Set_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ExtCountryCode_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PSTRING          arg);

INT Set_ExtDfsType_Proc(
    IN      PRTMP_ADAPTER   pAd,
    IN      PSTRING         arg);

INT Set_ChannelListAdd_Proc(
    IN      PRTMP_ADAPTER   pAd,
    IN      PSTRING         arg);

INT Set_ChannelListShow_Proc(
    IN      PRTMP_ADAPTER	pAd,
    IN      PSTRING			arg);
INT Set_ChannelListDel_Proc(
    IN      PRTMP_ADAPTER	pAd,
    IN      PSTRING			arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DBG
INT	Set_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif

#ifdef TXBF_SUPPORT
INT	Set_ReadITxBf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ReadETxBf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WriteITxBf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WriteETxBf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_StatITxBf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_StatETxBf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_TxBfTag_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ITxBfTimeout_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT Set_ETxBfTimeout_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT	Set_InvTxBfTag_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ITxBfCal_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ITxBfDivCal_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ITxBfLnaCal_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ETxBfEnCond_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ETxBfCodebook_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT Set_ETxBfCoefficient_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT Set_ETxBfGrouping_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT Set_ETxBfNoncompress_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

INT Set_ETxBfIncapable_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  PSTRING          arg);

INT	Set_NoSndgCntThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_NdpSndgStreams_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Trigger_Sounding_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ITxBfEn_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#endif /* TXBF_SUPPORT */

INT Set_RateAdaptInterval(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING arg);


#ifdef PRE_ANT_SWITCH
INT Set_PreAntSwitch_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

INT Set_PreAntSwitchRSSI_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  PSTRING         arg);

INT Set_PreAntSwitchTimeout_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  PSTRING         arg);
#endif /* PRE_ANT_SWITCH */


#if defined (RT2883) || defined (RT3883)
INT Set_PhyRateLimit_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

#ifdef DBG
INT Set_FixedRate_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);
#endif /* DBG */
#endif /* defined (RT2883) || defined (RT3883) */

#ifdef CFO_TRACK
INT Set_CFOTrack_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  PSTRING         arg);
#endif // CFO_TRACK //

#ifdef DBG_CTRL_SUPPORT
INT Set_DebugFlags_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);

#ifdef INCLUDE_DEBUG_QUEUE
INT Set_DebugQueue_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

void dbQueueEnqueue(
	IN UCHAR type,
	IN UCHAR *data);

void dbQueueEnqueueTxFrame(
	IN UCHAR *pTxWI,
	IN UCHAR *pHeader_802_11);

void dbQueueEnqueueRxFrame(
	IN UCHAR *pRxWI,
	IN UCHAR *pHeader_802_11,
	IN ULONG flags);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

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
#endif /* DOT11N_DRAFT3 */


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
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

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

void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk,
	IN  UCHAR			FromWhichBSSID);

INT	Set_HtMIMOPSmode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);


INT	Set_HtTxBASize_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HtDisallowTKIP_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_BurstMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* DOT11_N_SUPPORT */

#ifdef APCLI_SUPPORT
INT RTMPIoctlConnStatus(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

#endif /*APCLI_SUPPORT*/




#ifdef CONFIG_STA_SUPPORT
/*Dls ,	kathy */
VOID RTMPSendDLSTearDownFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA);

#ifdef DOT11_N_SUPPORT
/*Block ACK */
VOID QueryBATABLE(
	IN  PRTMP_ADAPTER pAd,
	OUT PQUERYBA_TABLE pBAT);
#endif /* DOT11_N_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT
INT	    WpaCheckEapCode(
	IN  PRTMP_ADAPTER   	pAd,
	IN  PUCHAR				pFrame,
	IN  USHORT				FrameLen,
	IN  USHORT				OffSet);

#endif /* WPA_SUPPLICANT_SUPPORT */


#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID detect_wmm_traffic(
	IN	PRTMP_ADAPTER	pAd, 
	IN	UCHAR			UserPriority,
	IN	UCHAR			FlgIsOutput);

VOID dynamic_tune_be_tx_op(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG			nonBEpackets);
#endif /* CONFIG_AP_SUPPORT */


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID Handle_BSS_Width_Trigger_Events(
	IN PRTMP_ADAPTER pAd);
#endif /* DOT11N_DRAFT3 */

void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE);

void assoc_ht_info_debugshow(
	IN	PRTMP_ADAPTER		pAd,
	IN	MAC_TABLE_ENTRY 	*pEntry,
	IN	UCHAR				HTCapability_Len,
	IN	HT_CAPABILITY_IE	*pHTCapability);
#endif /* DOT11_N_SUPPORT */


BOOLEAN APRxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd);

BOOLEAN STARxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd,
	IN	BOOLEAN			argc);

BOOLEAN RxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd);

#ifdef DROP_MASK_SUPPORT
VOID asic_set_drop_mask(
	RTMP_ADAPTER *ad,
	USHORT	wcid,
	BOOLEAN enable);

VOID asic_drop_mask_reset(
	RTMP_ADAPTER *ad);

VOID drop_mask_init_per_client(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry);

VOID drop_mask_release_per_client(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry);

VOID drop_mask_set_per_client(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry,
	BOOLEAN enable);

VOID drop_mask_timer_action(
	PVOID SystemSpecific1, 
	PVOID FunctionContext, 
	PVOID SystemSpecific2, 
	PVOID SystemSpecific3);
#endif /* DROP_MASK_SUPPORT */

#ifdef DOT11_N_SUPPORT
/* AMPDU packet indication */
VOID Indicate_AMPDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

/* AMSDU packet indication */
VOID Indicate_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID BaReOrderingBufferMaintain(
    IN PRTMP_ADAPTER pAd);
#endif /* DOT11_N_SUPPORT */

/* Normal legacy Rx packet indication */
VOID Indicate_Legacy_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID Indicate_EAPOL_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);


UINT deaggregate_AMSDU_announce(
	IN	PRTMP_ADAPTER	pAd,
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			OpMode);

#ifdef TXBF_SUPPORT
BOOLEAN clientSupportsETxBF(
	IN	PRTMP_ADAPTER	 pAd,
	IN	HT_BF_CAP *pTxBFCap);

void setETxBFCap(
	IN	PRTMP_ADAPTER	 pAd,
	OUT	HT_BF_CAP *pTxBFCap);

#ifdef ETXBF_EN_COND3_SUPPORT
VOID txSndgSameMcs(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN UCHAR smoothMfb);

VOID txSndgOtherGroup(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);

VOID txMrqInvTxBF(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);

VOID chooseBestMethod(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	UCHAR			mfb);

VOID rxBestSndg(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);
#endif // ETXBF_EN_COND3_SUPPORT //

VOID handleBfFb(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk);

VOID TxBFInit(
	IN PRTMP_ADAPTER 	pAd,
	IN MAC_TABLE_ENTRY	*pEntry,
	IN BOOLEAN			supportsETxBF);

VOID eTxBFProbing(
 	IN PRTMP_ADAPTER 	pAd,
	IN MAC_TABLE_ENTRY	*pEntry);

VOID Trigger_Sounding_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			SndgType,
	IN	UCHAR			SndgBW,
	IN	UCHAR			SndgMcs,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID rtmp_asic_set_bf(
	IN RTMP_ADAPTER *pAd);

BOOLEAN rtmp_chk_itxbf_calibration(
	IN RTMP_ADAPTER *pAd);

#endif /* TXBF_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/* remove LLC and get 802_3 Header */
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
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
/* remove LLC and get 802_3 Header */
#define  RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)	\
{																				\
	PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;                                 \
																				\
	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_WDS) || RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH)) \
	{                                                                           \
		_pDA = _pRxBlk->pHeader->Addr3;                                         \
		_pSA = (PUCHAR)_pRxBlk->pHeader + sizeof(HEADER_802_11);                \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_INFRA))                              	\
		{                                                                       \
			_pDA = _pRxBlk->pHeader->Addr1;                                     \
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_DLS))									\
			_pSA = _pRxBlk->pHeader->Addr2;										\
		else																	\
			_pSA = _pRxBlk->pHeader->Addr3;                                     \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			_pDA = _pRxBlk->pHeader->Addr1;                                     \
			_pSA = _pRxBlk->pHeader->Addr2;                                     \
		}                                                                       \
	}                                                                           \
																				\
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData, 				\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);                                   \
}
#endif /* CONFIG_STA_SUPPORT */


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
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#define ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
			Sta_Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
			/*announce_802_3_packet(_pAd, _pPacket); */
#endif /* CONFIG_STA_SUPPORT */




/* Normal, AMPDU or AMSDU */
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

/*////////////////////////////////////*/

#if defined (AP_SCAN_SUPPORT) || defined (CONFIG_STA_SUPPORT)
VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
INT Set_ApCli_AuthMode_Proc(
    IN PRTMP_ADAPTER pAd, 
    IN PSTRING arg);

INT Set_ApCli_EncrypType_Proc(
    IN PRTMP_ADAPTER pAd, 
    IN PSTRING arg);
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef MAT_SUPPORT

VOID getIPv6MacTbInfo(MAT_STRUCT *, char *, ULONG);

VOID getIPMacTbInfo(
	IN MAT_STRUCT *pMatCfg, 
	IN char *pOutBuf,
	IN ULONG BufLen);

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
	IN UINT				infIdx,
	IN UCHAR                  OpMode);

BOOLEAN MATPktRxNeedConvert(
	IN PRTMP_ADAPTER	pAd, 
	IN PNET_DEV			net_dev);

#endif /* MAT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
typedef struct CountryCodeToCountryRegion {
	USHORT		CountryNum;
	UCHAR		IsoName[3];
	/*UCHAR		CountryName[40]; */
	PSTRING		pCountryName;
	BOOLEAN		SupportABand;
	/*ULONG		RegDomainNum11A; */
	UCHAR		RegDomainNum11A;
	BOOLEAN  	SupportGBand;
	/*ULONG		RegDomainNum11G; */
	UCHAR		RegDomainNum11G;
} COUNTRY_CODE_TO_COUNTRY_REGION;
#endif /* CONFIG_AP_SUPPORT */

#ifdef SNMP_SUPPORT
/*for snmp , kathy */
typedef struct _DefaultKeyIdxValue
{
	UCHAR	KeyIdx;
	UCHAR	Value[16];
} DefaultKeyIdxValue, *PDefaultKeyIdxValue;
#endif


#ifdef CONFIG_STA_SUPPORT

/* The radio capture header precedes the 802.11 header. */
typedef struct GNU_PACKED _ieee80211_radiotap_header {
    UINT8	it_version;	/* Version 0. Only increases
				 * for drastic changes,
				 * introduction of compatible
				 * new fields does not count.
				 */
    UINT8	it_pad;
    UINT16     it_len;         /* length of the whole
				 * header in bytes, including
				 * it_version, it_pad,
				 * it_len, and data fields.
				 */
    UINT32   it_present;	/* A bitmap telling which
					 * fields are present. Set bit 31
					 * (0x80000000) to extend the
					 * bitmap by another 32 bits.
					 * Additional extensions are made
					 * by setting bit 31.
					 */
}ieee80211_radiotap_header ;

enum ieee80211_radiotap_type {
    IEEE80211_RADIOTAP_TSFT = 0,
    IEEE80211_RADIOTAP_FLAGS = 1,
    IEEE80211_RADIOTAP_RATE = 2,
    IEEE80211_RADIOTAP_CHANNEL = 3,
    IEEE80211_RADIOTAP_FHSS = 4,
    IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5,
    IEEE80211_RADIOTAP_DBM_ANTNOISE = 6,
    IEEE80211_RADIOTAP_LOCK_QUALITY = 7,
    IEEE80211_RADIOTAP_TX_ATTENUATION = 8,
    IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9,
    IEEE80211_RADIOTAP_DBM_TX_POWER = 10,
    IEEE80211_RADIOTAP_ANTENNA = 11,
    IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12,
    IEEE80211_RADIOTAP_DB_ANTNOISE = 13
};

#define WLAN_RADIOTAP_PRESENT (			\
	(1 << IEEE80211_RADIOTAP_TSFT)	|	\
	(1 << IEEE80211_RADIOTAP_FLAGS) |	\
	(1 << IEEE80211_RADIOTAP_RATE)  | 	\
	 0)

typedef struct _wlan_radiotap_header {
	ieee80211_radiotap_header wt_ihdr;
	INT64 wt_tsft;
	UINT8 wt_flags;	
	UINT8 wt_rate;
} wlan_radiotap_header;
/* Definition from madwifi */


void STA_MonPktSend(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk);

VOID    RTMPSetDesiredRates(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  LONG            Rates);

#ifdef XLINK_SUPPORT
INT Set_XlinkMode_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);
#endif /* XLINK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

INT	Set_FixedTxMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

INT Set_LongRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Set_ShortRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Set_AutoFallBack_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

#ifdef RT2883
VOID RTMPRT2883ReadTxPwrPerRate(
	IN	PRTMP_ADAPTER	pAd);

VOID RTMPRT2883ReadChannelPwr(
	IN	PRTMP_ADAPTER	pAd);
#endif /* RT2883 */

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
#endif /* WORKQUEUE_BH */


	
	
#ifdef RTMP_MAC_PCI
/* */
/* Function Prototype in cmm_data_pci.c */
/* */

VOID DBGPRINT_TX_RING(
	IN PRTMP_ADAPTER  pAd,
	IN UCHAR          QueIdx);

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

VOID RTMPInitPCIeDevice(
    IN	RT_CMD_PCIE_INIT	*pConfig,
    IN	VOID				*pAd);

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
VOID RTMPInitPCIeLinkCtrlValue(
	IN	PRTMP_ADAPTER	pAd);


VOID RTMPPCIeLinkCtrlValueRestore(
	IN	PRTMP_ADAPTER	pAd,
	IN   UCHAR		Level);

VOID RTMPPCIeLinkCtrlSetting(
	IN	PRTMP_ADAPTER	pAd,
	IN 	USHORT		Max);

VOID RTMPrt3xSetPCIePowerLinkCtrl(
	IN	PRTMP_ADAPTER	pAd);

VOID PsPollWakeExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID  RadioOnExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);
#endif /* PCIE_PS_SUPPORT */

VOID RT28xxPciStaAsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       bFromTx);

VOID RT28xxPciStaAsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TbttNumToNextWakeUp);

#endif /* CONFIG_STA_SUPPORT */

VOID RT28xxPciMlmeRadioOn(
	IN PRTMP_ADAPTER pAd);

VOID RT28xxPciMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd);

ra_dma_addr_t RtmpDrvPciMapSingle(
	IN	PRTMP_ADAPTER			pAd,
	IN	VOID					*ptr,
	IN	size_t					size,
	IN	int						sd_idx,
	IN	int						direction);
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
NDIS_STATUS RecoverConnectInfo(
	IN  RTMP_ADAPTER *pAd);

NDIS_STATUS StoreConnectInfo(
	IN  RTMP_ADAPTER *pAd);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER    pAd, 
	IN	UCHAR           Channel);

#ifdef RT305x
INT Set_HiPower_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

#if defined(RT3352) || defined(RT5350)
#ifdef RTMP_INTERNAL_TX_ALC
INT Set_TSSIMaxRange_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING			arg);
#endif /* RTMP_INTERNAL_TX_ALC */
#endif /* defined(RT3352) || defined(RT5350) */
#endif /* RT305x */


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
#endif /* RTMP_TIMER_TASK_SUPPORT */



/*////////////////////////////////////*/

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
#endif /* AP_QLOAD_SUPPORT */

/*///////////////////////////////////*/
INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			pName,
	IN	PSTRING			pBuf,
	IN	UINT32			MaxLen);

PSTRING RTMPGetRalinkAuthModeStr(
    IN  NDIS_802_11_AUTHENTICATION_MODE authMode);

PSTRING RTMPGetRalinkEncryModeStr(
    IN  USHORT encryMode);
/*//////////////////////////////////*/

#ifdef CONFIG_STA_SUPPORT
VOID AsicStaBbpTuning(
	IN PRTMP_ADAPTER pAd);

BOOLEAN StaAddMacTableEntry(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				MaxSupportedRateIn500Kbps,
	IN  HT_CAPABILITY_IE	*pHtCapability,
	IN  UCHAR				HtCapabilityLen,
	IN  ADD_HT_INFO_IE		*pAddHtInfo,
	IN  UCHAR				AddHtInfoLen,
	IN  USHORT        		CapabilityInfo);


BOOLEAN	AUTH_ReqSend(
	IN  PRTMP_ADAPTER 		pAd,
	IN  PMLME_QUEUE_ELEM	pElem,
	IN  PRALINK_TIMER_STRUCT pAuthTimer,
	IN  PSTRING				pSMName,
	IN  USHORT				SeqNo,
	IN  PUCHAR				pNewElement,
	IN  ULONG				ElementLen);
#endif /* CONFIG_STA_SUPPORT */ 


VOID ReSyncBeaconTime(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BandWidth);

#ifdef TXBF_SUPPORT
VOID handleHtcField(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk);
#endif /* TXBF_SUPPORT */

#ifdef MFB_SUPPORT
VOID MFB_PerPareMRQ(
	IN	PRTMP_ADAPTER	pAd,
	OUT	VOID* pBuf,
	IN	PMAC_TABLE_ENTRY pEntry);

VOID MFB_PerPareMFB(
	IN	PRTMP_ADAPTER	pAd,
	OUT	VOID* pBuf,
	IN	PMAC_TABLE_ENTRY pEntry);
#endif /* MFB_SUPPORT */

#define VIRTUAL_IF_INC(__pAd) ((__pAd)->VirtualIfCnt++)
#define VIRTUAL_IF_DEC(__pAd) ((__pAd)->VirtualIfCnt--)
#define VIRTUAL_IF_NUM(__pAd) ((__pAd)->VirtualIfCnt)





#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(
	IN  PRTMP_ADAPTER   pAd,
	IN	PTX_BLK			pTxBlk);

VOID RTMPUpdateSwCacheCipherInfo(	
	IN  PRTMP_ADAPTER   pAd,
	IN	PTX_BLK			pTxBlk,
	IN	PUCHAR			pHdr);
#endif /* SOFT_ENCRYPT */


/*
	OS Related funciton prototype definitions.
	TODO: Maybe we need to move these function prototypes to other proper place.
*/

VOID	RTInitializeCmdQ(
	IN	PCmdQ	cmdq);

INT RTPCICmdThread(
	IN ULONG Context);

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
#endif/*HOSTAPD_SUPPORT*/

#ifdef VENDOR_FEATURE3_SUPPORT
VOID RTMP_IO_WRITE32(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT32 Value);
#endif /* VENDOR_FEATURE3_SUPPORT */

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

NDIS_STATUS AsicBBPWriteWithRxChain(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR bbpId,
	IN CHAR bbpVal,
	IN RX_CHAIN_IDX rx_ch_idx);

NDIS_STATUS AsicBBPReadWithRxChain(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR bbpId, 
	IN CHAR *pBbpVal,
	IN RX_CHAIN_IDX rx_ch_idx);

INT AsicGetMacVersion(
	IN RTMP_ADAPTER *pAd);

INT WaitForAsicReady(
	IN RTMP_ADAPTER *pAd);

BOOLEAN CHAN_PropertyCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UINT32			ChanNum,
	IN UCHAR			Property);

#ifdef CONFIG_STA_SUPPORT

/* command */
INT Set_SSID_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

#ifdef WMM_SUPPORT
INT	Set_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif

INT Set_NetworkType_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AuthMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_EncrypType_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_DefaultKeyID_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_Key1_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_Key2_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_Key3_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_Key4_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_WPAPSK_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);


INT Set_PSMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

#ifdef WPA_SUPPLICANT_SUPPORT
INT Set_Wpa_Support(
    IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef DBG

VOID RTMPIoctlMAC(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);

VOID RTMPIoctlE2PROM(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* DBG */

#ifdef WSC_STA_SUPPORT
INT	Set_WscConfMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WscConfStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WscSsid_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_WscBssid_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WscMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WscUUIDE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_WscGetConf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_WscStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef WSC_V2_SUPPORT
INT Set_WscForceSetAP_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);
#endif /* WSC_V2_SUPPORT */
#endif /* WSC_STA_SUPPORT */

NDIS_STATUS RTMPWPANoneAddKeyProc(
    IN  PRTMP_ADAPTER   pAd,
    IN	PVOID			pBuf);
	
INT Set_FragTest_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);
	
#ifdef DOT11_N_SUPPORT	
INT Set_TGnWifiTest_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg);
#endif /* DOT11_N_SUPPORT */

INT Set_LongRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Set_ShortRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_Ieee80211dClientMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

INT	Show_Adhoc_MacTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			extra,
	IN	UINT32			size);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq);
#endif /* RTMP_RF_RW_SUPPORT */


INT Set_BeaconLostTime_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

INT Set_AutoRoaming_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

INT Set_SiteSurvey_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ForceTxBurst_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

VOID RTMPAddKey(
	IN	PRTMP_ADAPTER	    pAd, 
	IN	PNDIS_802_11_KEY    pKey);


VOID StaSiteSurvey(
	IN	PRTMP_ADAPTER  		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType);

VOID MaintainBssTable(
	IN  PRTMP_ADAPTER pAd,
	IN OUT	BSS_TABLE *Tab,
	IN  ULONG	MaxRxTimeDiff,
	IN  UCHAR	MaxSameRxTimeCount);
#endif /* CONFIG_STA_SUPPORT */

void  getRate(
    IN HTTRANSMIT_SETTING HTSetting, 
    OUT ULONG* fLastTxRxRate);


#ifdef APCLI_SUPPORT
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
VOID    ApcliSendAssocIEsToWpaSupplicant( 
    IN  PRTMP_ADAPTER pAd,
    IN UINT ifIndex);

INT	    ApcliWpaCheckEapCode(
	IN  PRTMP_ADAPTER   		pAd,
	IN  PUCHAR				pFrame,
	IN  USHORT				FrameLen,
	IN  USHORT				OffSet);

VOID    ApcliWpaSendEapolStart(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pBssid,
	IN  PMAC_TABLE_ENTRY pMacEntry,
	IN	PAPCLI_STRUCT pApCliEntry);
#endif/*APCLI_WPA_SUPPLICANT_SUPPORT*/

VOID	ApCliRTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN 		bQosNull,
	IN PMAC_TABLE_ENTRY pMacEntry);
#endif/*APCLI_SUPPORT*/


void RTMP_IndicateMediaState(	
	IN	PRTMP_ADAPTER		pAd,
	IN  NDIS_MEDIA_STATE	media_state);

#if defined(RT3350) || defined(RT33xx)
VOID RTMP_TxEvmCalibration(
	IN PRTMP_ADAPTER pAd);
#endif /* defined(RT3350) || defined(RT33xx) */

INT RTMPSetInformation(
    IN  PRTMP_ADAPTER pAd,
    IN  OUT RTMP_IOCTL_INPUT_STRUCT		*rq,
    IN  INT                 cmd);

INT RTMPQueryInformation(
    IN  PRTMP_ADAPTER pAd,
    IN  OUT RTMP_IOCTL_INPUT_STRUCT		*rq,
    IN  INT                 cmd);

VOID RTMPIoctlShow(
	IN	PRTMP_ADAPTER			pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT			*rq,
	IN	UINT32					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data);

INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data);

#ifdef CONFIG_AP_SUPPORT
INT RTMP_AP_IoctlPrepare(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pCB);
#endif /* CONFIG_AP_SUPPORT */

#ifdef P2P_SUPPORT
BOOLEAN APHandleRxDonePacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pRxPacket,
	IN	RX_BLK			*pRxCell);

BOOLEAN STAHandleRxDonePacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pRxPacket,
	IN	RX_BLK			*pRxCell);
#endif /* P2P_SUPPORT */

INT Set_VcoPeriod_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN PSTRING			arg);

INT	Set_RateAlg_Proc(
	IN	PRTMP_ADAPTER			pAd,
	IN	PSTRING					arg);

VOID RtmpUpdateFilterCoefficientControl(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Channel);

#ifdef MAC_APCLI_SUPPORT
VOID AsicSetApCliBssid(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pBssid,
	IN UCHAR index);
#endif /* MAC_APCLI_SUPPORT */

#ifdef SINGLE_SKU
INT Set_ModuleTxpower_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* SINGLE_SKU */

VOID RtmpEnqueueNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         TxRate,
	IN UCHAR         PID,
	IN UCHAR         apidx,
    IN BOOLEAN       bQosNull,
    IN BOOLEAN       bEOSP,
    IN UCHAR         OldUP);

VOID RtmpCleanupPsQueue(
    IN  PRTMP_ADAPTER   pAd,
    IN  PQUEUE_HEADER   pQueue);

NDIS_STATUS RtmpInsertPsQueue(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR QueIdx);

VOID RtmpHandleRxPsPoll(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pAddr,
	IN	USHORT			Aid,
    IN	BOOLEAN			isActive);

BOOLEAN RtmpPsIndicate(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PUCHAR          pAddr, 
	IN	ULONG			Wcid, 
    IN  UCHAR           Psm);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN RtmpPktPmBitCheck(
	IN PRTMP_ADAPTER	pAd);

VOID RtmpPsActiveExtendCheck(
	IN PRTMP_ADAPTER	pAd);

VOID RtmpPsModeChange(
	IN PRTMP_ADAPTER	pAd, 
	IN UINT32			PsMode);
#endif /* CONFIG_STA_SUPPORT */

#if defined(MICROWAVE_OVEN_SUPPORT) || defined(DYNAMIC_VGA_SUPPORT)
INT Set_MO_FalseCCATh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_MO_LowFalseCCATh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(
	IN PRTMP_ADAPTER pAd);

VOID AsicMitigateMicrowave(
	IN PRTMP_ADAPTER pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */
#endif /* defined(MICROWAVE_OVEN_SUPPORT) || defined(DYNAMIC_VGA_SUPPORT) */

#ifdef MAC_REPEATER_SUPPORT
REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr);

VOID RTMPInsertRepeaterAsicEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr);

VOID RTMPRemoveRepeaterAsicEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx);

VOID RTMPInsertRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
	IN PUCHAR pAddr);

VOID RTMPRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
	IN UCHAR CliIdx);

VOID RTMPRemoveRepeaterDisconnectEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apIdx,
	IN UCHAR CliIdx);

VOID RTMPRepeaterReconnectionCheck(
	IN PRTMP_ADAPTER pAd);

MAC_TABLE_ENTRY *RTMPInsertRepeaterMacEntry(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pAddr,
	IN  UCHAR apidx,
	IN  UCHAR cliIdx,
	IN BOOLEAN CleanAll);

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

VOID RTMPRepeaterInsertInvaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

BOOLEAN RTMPRepeaterRemoveInvaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR idx,
	IN PUCHAR pAddr);

INT	Show_Repeater_Cli_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef SINGLE_SKU_V2
NDIS_STATUS	RTMPSetSingleSKUParameters(
	IN RTMP_ADAPTER *pAd);

VOID InitSkuRateDiffTable(
	IN PRTMP_ADAPTER pAd);

UCHAR GetSkuChannelBasePwr(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel);

UCHAR GetSkuPerRatePwr(
	IN PRTMP_ADAPTER pAd,
	IN CHAR phymode,
	IN UCHAR channel,
	IN UCHAR bw,
	IN INT32 paValue);
#endif /* SINGLE_SKU_V2 */

#ifdef PEER_DELBA_TX_ADAPT
VOID Peer_DelBA_Tx_Adapt_Init(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);

VOID Peer_DelBA_Tx_Adapt_Enable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);

VOID Peer_DelBA_Tx_Adapt_Disable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);

VOID Peer_DelBA_Tx_AdaptTimeOut(
        IN PVOID SystemSpecific1,
        IN PVOID FunctionContext,
        IN PVOID SystemSpecific2,
        IN PVOID SystemSpecific3);   
#endif /* PEER_DELBA_TX_ADAPT */
#endif  /* __RTMP_H__ */

