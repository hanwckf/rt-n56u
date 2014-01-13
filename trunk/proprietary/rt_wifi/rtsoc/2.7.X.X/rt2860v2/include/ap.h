/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    08-01-2002    created
    James Tan   09-06-2002    modified (Revise NTCRegTable)
    John Chang  12-22-2004    modified for RT2561/2661. merge with STA driver
*/
#ifndef __AP_H__
#define __AP_H__


/* ============================================================= */
/*      Common definition */
/* ============================================================= */
#define MBSS_VLAN_INFO_GET(												\
	__pAd, __VLAN_VID, __VLAN_Priority, __FromWhichBSSID) 				\
{																		\
	if ((__FromWhichBSSID < __pAd->ApCfg.BssidNum) &&					\
		(__FromWhichBSSID < HW_BEACON_MAX_NUM) &&						\
		(__pAd->ApCfg.MBSSID[__FromWhichBSSID].VLAN_VID != 0))			\
	{																	\
		__VLAN_VID = __pAd->ApCfg.MBSSID[__FromWhichBSSID].VLAN_VID;	\
		__VLAN_Priority = __pAd->ApCfg.MBSSID[__FromWhichBSSID].VLAN_Priority; \
	}																	\
}

/* ============================================================= */
/*      Function Prototypes */
/* ============================================================= */

/* ap_data.c */

BOOLEAN APBridgeToWirelessSta(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pHeader,
    IN  UINT            HdrLen,
    IN  PUCHAR          pData,
    IN  UINT            DataLen,
    IN  ULONG           fromwdsidx);

VOID RTMP_BASetup(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN UINT8 UserPriority);

VOID	APSendPackets(
	IN	NDIS_HANDLE		MiniportAdapterContext,
	IN	PPNDIS_PACKET	ppPacketArray,
	IN	UINT			NumberOfPackets);

NDIS_STATUS APSendPacket(
    IN  PRTMP_ADAPTER   pAd,
    IN  PNDIS_PACKET    pPacket);

NDIS_STATUS APInsertPsQueue(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR QueIdx);

NDIS_STATUS APHardTransmit(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			QueIdx);

VOID APRxEAPOLFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

NDIS_STATUS APCheckRxError(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRT28XX_RXD_STRUC		pRxD,
	IN	UCHAR			Wcid);

BOOLEAN APCheckClass2Class3Error(
    IN  PRTMP_ADAPTER   pAd,
	IN ULONG Wcid, 
	IN  PHEADER_802_11  pHeader);

VOID APHandleRxPsPoll(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pAddr,
	IN	USHORT			Aid,
    IN	BOOLEAN			isActive);

VOID    RTMPDescriptorEndianChange(
    IN  PUCHAR          pData,
    IN  ULONG           DescriptorType);
    
VOID    RTMPFrameEndianChange(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pData,
    IN  ULONG           Dir,
    IN  BOOLEAN         FromRxDoneInt);

/* ap_assoc.c */

VOID APAssocStateMachineInit(
    IN  PRTMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID  APPeerAssocReqAction(
    IN  PRTMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem);

VOID  APPeerReassocReqAction(
    IN  PRTMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem);

VOID  APPeerDisassocReqAction(
    IN  PRTMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem);

VOID MbssKickOutStas(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN USHORT Reason);

VOID APMlmeKickOutSta(
    IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pStaAddr, 
	IN UCHAR Wcid,
	IN USHORT Reason);

    
VOID APMlmeDisassocReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID  APCls3errAction(
    IN  PRTMP_ADAPTER   pAd, 
	IN 	ULONG Wcid,
    IN	PHEADER_802_11	pHeader);


USHORT APBuildAssociation(
    IN PRTMP_ADAPTER pAd,
    IN MAC_TABLE_ENTRY *pEntry,
    IN USHORT CapabilityInfo,
    IN UCHAR  MaxSupportedRateIn500Kbps,
    IN UCHAR  *RSN,
    IN UCHAR  *pRSNLen, 
    IN BOOLEAN bWmmCapable,
    IN ULONG  RalinkIe,
    IN EXT_CAP_INFO_ELEMENT ExtCapInfo,
	IN HT_CAPABILITY_IE		*pHtCapability,
	OUT UCHAR		 *pHtCapabilityLen,
    OUT USHORT *pAid);

/*
VOID	RTMPAddClientSec(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	BssIdx,
	IN UCHAR		 KeyIdx,
	IN UCHAR		 CipherAlg,
	IN PUCHAR		 pKey,
	IN PUCHAR		 pTxMic,
	IN PUCHAR		 pRxMic,
	IN MAC_TABLE_ENTRY *pEntry);
*/

/* ap_auth.c */

void APAuthStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID APCls2errAction(
    IN PRTMP_ADAPTER pAd, 
	IN 	ULONG Wcid,
    IN	PHEADER_802_11	pHeader);

/* ap_connect.c */

#ifdef CONFIG_AP_SUPPORT
BOOLEAN BeaconTransmitRequired(
	IN PRTMP_ADAPTER	pAd,
	IN INT				apidx,
	IN MULTISSID_STRUCT *pMbss);
#endif /* CONFIG_AP_SUPPORT */

VOID APMakeBssBeacon(
    IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx);

VOID  APUpdateBeaconFrame(
    IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx);

VOID APMakeAllBssBeacon(
    IN  PRTMP_ADAPTER   pAd);

VOID  APUpdateAllBeaconFrame(
    IN  PRTMP_ADAPTER   pAd);


/* ap_sync.c */

VOID APSyncStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID APScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APInvalidStateWhenScan(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APScanTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APPeerProbeReqAction(
    IN  PRTMP_ADAPTER pAd, 
    IN  MLME_QUEUE_ELEM *Elem);

VOID APPeerBeaconAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID APMlmeScanReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APPeerBeaconAtScanAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APScanCnclAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID ApSiteSurvey(
	IN	PRTMP_ADAPTER  		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel);

VOID SupportRate(
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN PUCHAR ExtRate,
	IN UCHAR ExtRateLen,
	OUT PUCHAR *Rates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate);


BOOLEAN ApScanRunning(
	IN PRTMP_ADAPTER pAd);
	
#ifdef DOT11N_DRAFT3
VOID APOverlappingBSSScan(
	IN RTMP_ADAPTER *pAd);

INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN BSS_COEX_CH_RANGE *pCoexChRange);

#endif /* DOT11N_DRAFT3 */

/* ap_wpa.c */
VOID WpaStateMachineInit(
    IN  PRTMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

/* ap_mlme.c */
VOID APMlmePeriodicExec(
    IN  PRTMP_ADAPTER   pAd);

BOOLEAN APMsgTypeSubst(
    IN PRTMP_ADAPTER pAd,
    IN PFRAME_802_11 pFrame, 
    OUT INT *Machine, 
    OUT INT *MsgType);

VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);


VOID RTMPSetPiggyBack(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN			bPiggyBack);

VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd);

VOID APAsicRxAntEvalTimeout(
	IN PRTMP_ADAPTER	pAd);

/* ap.c */
NDIS_STATUS APInitialize(
    IN  PRTMP_ADAPTER   pAd);

VOID APShutdown(
    IN PRTMP_ADAPTER    pAd);

VOID APStartUp(
    IN  PRTMP_ADAPTER   pAd);

VOID APStop(
    IN  PRTMP_ADAPTER   pAd);

VOID APCleanupPsQueue(
    IN  PRTMP_ADAPTER   pAd,
    IN  PQUEUE_HEADER   pQueue);

VOID MacTableReset(
    IN  PRTMP_ADAPTER   pAd);

MAC_TABLE_ENTRY *MacTableInsertEntry(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PUCHAR          pAddr,
	IN	UCHAR			apidx,
	IN	UCHAR           OpMode,
	IN BOOLEAN	CleanAll); 

BOOLEAN MacTableDeleteEntry(
    IN  PRTMP_ADAPTER   pAd, 
	IN USHORT wcid,
    IN  PUCHAR          pAddr);

MAC_TABLE_ENTRY *MacTableLookup(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PUCHAR          pAddr);

VOID MacTableMaintenance(
    IN PRTMP_ADAPTER pAd);

UINT32 MacTableAssocStaNumGet(
	IN PRTMP_ADAPTER pAd);

MAC_TABLE_ENTRY *APSsPsInquiry(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PUCHAR          pAddr, 
    OUT SST             *Sst, 
    OUT USHORT          *Aid,
    OUT UCHAR           *PsMode,
    OUT UCHAR           *Rate); 

BOOLEAN APPsIndicate(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PUCHAR          pAddr, 
	IN ULONG Wcid, 
    IN  UCHAR           Psm);

#ifdef SYSTEM_LOG_SUPPORT
VOID ApLogEvent(
    IN PRTMP_ADAPTER    pAd,
    IN PUCHAR           pAddr,
    IN USHORT           Event);
#else
#define ApLogEvent(_pAd, _pAddr, _Event)
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID APUpdateOperationMode(
    IN PRTMP_ADAPTER pAd);
#endif /* DOT11_N_SUPPORT */

VOID APUpdateCapabilityAndErpIe(
	IN PRTMP_ADAPTER pAd);

BOOLEAN ApCheckAccessControlList(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         Apidx);

VOID ApUpdateAccessControlList(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR         Apidx);

VOID ApEnqueueNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         TxRate,
	IN UCHAR         PID,
	IN UCHAR         apidx,
    IN BOOLEAN       bQosNull,
    IN BOOLEAN       bEOSP,
    IN UCHAR         OldUP);

/* ap_sanity.c */


BOOLEAN PeerAssocReqCmmSanity(
    IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN isRessoc,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pListenInterval, 
    OUT PUCHAR pApAddr,
    OUT UCHAR *pSsidLen,
    OUT char *Ssid,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *RSN,
    OUT UCHAR *pRSNLen,
    OUT BOOLEAN *pbWmmCapable,
#ifdef WSC_AP_SUPPORT
    OUT BOOLEAN *pWscCapable,
#endif /* WSC_AP_SUPPORT */    
    OUT ULONG  *pRalinkIe,
    OUT EXT_CAP_INFO_ELEMENT	*pExtCapInfo,
#ifdef P2P_SUPPORT
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement, 
#endif /* P2P_SUPPORT */
    OUT UCHAR		 *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability);


BOOLEAN PeerDisassocReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT	UINT16	*SeqNum,
    OUT USHORT *Reason);

BOOLEAN PeerDeauthReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
   	OUT	UINT16	*SeqNum,    
    OUT USHORT *Reason);

BOOLEAN APPeerAuthSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
	OUT PUCHAR pAddr1, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *Alg, 
    OUT USHORT *Seq, 
    OUT USHORT *Status, 
    OUT CHAR *ChlgText
	);


#ifdef DOT1X_SUPPORT
/* ap_cfg.h */
INT	Set_OwnIPAddr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_EAPIfName_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_PreAuthIfName_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

/* Define in ap.c */
BOOLEAN DOT1X_InternalCmdAction(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN	UINT8			cmd);

BOOLEAN DOT1X_EapTriggerAction(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry);
#endif /* DOT1X_SUPPORT */
#endif  /* __AP_H__ */

VOID AP_E2PROM_IOCTL_PostCtrl(
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	PSTRING					msg);

VOID IAPP_L2_UpdatePostCtrl(
	IN PRTMP_ADAPTER	pAd,
    IN UINT8 *mac_p,
    IN INT  bssid);
