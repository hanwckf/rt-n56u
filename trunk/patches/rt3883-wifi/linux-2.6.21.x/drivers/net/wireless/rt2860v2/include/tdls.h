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

#ifndef __TDLS_H
#define __TDLS_H

#include "dot11z_tdls.h"
#include "dot11r_ft.h"

#define LENGTH_TDLS_H				25
#define TDLS_TIMEOUT				1200	// unit: msec

// TDLS State
typedef enum _TDLS_STATE {
	TDLS_MODE_NONE,				// Init state
	TDLS_MODE_WAIT_RESPONSE,		// Wait a response from the Responder
	TDLS_MODE_WAIT_CONFIRM,			// Wait an confirm from the Initiator
	TDLS_MODE_CONNECTED,		// Tunneled Direct Link estabilished
//	TDLS_MODE_SWITCH_CHANNEL,
//	TDLS_MODE_PSM,
//	TDLS_MODE_UAPSD
} TDLS_STATE;

typedef struct _MLME_TDLS_REQ_STRUCT {
	PRT_802_11_TDLS	pTDLS;
	USHORT			Reason;
	UCHAR			Action;
	BOOLEAN			IsViaAP;
} MLME_TDLS_REQ_STRUCT, *PMLME_TDLS_REQ_STRUCT;

VOID
TDLS_Table_Init(
	IN	PRTMP_ADAPTER	pAd);

VOID
TDLS_SearchTabInit(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_Table_Destory(
	IN	PRTMP_ADAPTER	pAd);

VOID
TDLS_SearchTabDestory(
	IN PRTMP_ADAPTER pAd);

PTDLS_SEARCH_ENTRY
TDLS_SearchEntyAlloc(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_SearchEntyFree(
	IN PRTMP_ADAPTER pAd,
	IN PTDLS_SEARCH_ENTRY pTdlsSearchEntry);

BOOLEAN
TLDS_SearchEntryLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac);

BOOLEAN
TDLS_SearchEntryUpdate(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac);

BOOLEAN
TDLS_SearchEntryDelete(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac);

VOID
TDLS_SearchTabReset(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_SearchTabMaintain(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_StateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID
TDLS_CntlOidTDLSRequestProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

INT
TDLS_SearchLinkId(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR	pAddr);

VOID
TDLS_MlmeParmFill(
	IN PRTMP_ADAPTER pAd, 
	IN OUT MLME_TDLS_REQ_STRUCT *pTdlsReq,
	IN PRT_802_11_TDLS pTdls,
	IN USHORT Reason,
	IN BOOLEAN IsViaAP);

INT	Set_TdlsCapableProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

INT	Set_TdlsSearchResetProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_TdlsSetupProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

INT	Set_TdlsTearDownProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

NDIS_STATUS
TDLS_SetupRequestAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS);

NDIS_STATUS
TDLS_SetupResponseAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

NDIS_STATUS
TDLS_SetupConfirmAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

VOID
TDLS_InitPeerEntryRateCapability(
	IN	PRTMP_ADAPTER pAd,
	IN	MAC_TABLE_ENTRY *pEntry,
	IN USHORT *pCapabilityInfo,
	IN UCHAR SupportRateLens,
	IN UCHAR *pSupportRates,
	IN UCHAR HtCapabilityLen,
	IN HT_CAPABILITY_IE *pHtCapability);

VOID
TDLS_BuildSetupRequest(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS);

VOID
TDLS_BuildSetupResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

VOID
TDLS_BuildSetupConfirm(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

VOID
TDLS_BuildTeardown(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS,
	IN	UINT16	ReasonCode);

VOID
TDLS_InsertActField(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	Category,
	IN	UINT8	ActCode);

VOID
TDLS_InsertStatusCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	StatusCode);

VOID
TDLS_InsertReasonCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	ReasonCode);

VOID
TDLS_InsertDialogToken(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	DialogToken);

VOID
TDLS_InsertLinkIdentifierIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	PUCHAR	pInitAddr,
	IN	PUCHAR	pPeerAddr);

VOID
TDLS_InsertCapIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertSSIDIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertSupportRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertExtRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertQosCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertEDCAParameterSetIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS);

#ifdef DOT11_N_SUPPORT
VOID
TDLS_InsertHtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

#ifdef DOT11N_DRAFT3
VOID
TDLS_InsertBSSCoexistenceIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

VOID
TDLS_InsertExtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertFTIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Length,
	IN FT_MIC_CTR_FIELD MICCtr,
	IN PUINT8 pMic,
	IN PUINT8 pANonce,
	IN PUINT8 pSNonce);

VOID
TDLS_InsertTimeoutIntervalIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_TIMEOUT_INTERVAL_TYPE Type,
	IN UINT32 TimeOutValue);

VOID TDLS_MlmeTdlsReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_PeerTdlsReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TDLS_PeerTdlsReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TDLS_PeerTdlsRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TDLS_PeerTdlsConfAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TDLS_MlmeTdlsTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TDLS_PeerTdlsTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

BOOLEAN
TDLS_MsgTypeSubst(
	IN	UCHAR	TDLSActionType,
	OUT	INT		*MsgType);

BOOLEAN TDLS_CheckTDLSframe(
    IN PRTMP_ADAPTER    pAd,
    IN PUCHAR           pData,
    IN ULONG            DataByteCount);

VOID
TDLS_LinkTearDown(
	IN PRTMP_ADAPTER	pAd);

BOOLEAN
PeerTdlsSetupReqSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pExtCap,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[]);

BOOLEAN
PeerTdlsSetupRspSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pExtCap,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[]);

BOOLEAN
PeerTdlsSetupConfSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT EDCA_PARM	*pEdcaParm,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[]);

BOOLEAN
PeerTdlsTearDownSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
	OUT UCHAR	*pSA,
	OUT	BOOLEAN *pIsInitator,
    OUT USHORT *pReasonCode,
	OUT UCHAR	*pFTLen,    
	OUT UCHAR	FTIe[]);

VOID
TDLS_TimeoutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID
TDLS_FTDeriveTPK(
	IN	PUCHAR 	mac_i,
	IN	PUCHAR 	mac_r,
	IN	PUCHAR 	a_nonce,
	IN	PUCHAR 	s_nonce,
	IN	PUCHAR 	bssid,
	IN	UINT	key_len,
 	OUT	PUCHAR	tpk,
	OUT	PUCHAR	tpk_name);

USHORT
TDLS_TPKMsg1Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen);

USHORT
TDLS_TPKMsg2Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen,
	OUT	PUCHAR				pTPK,
	OUT PUCHAR				pTPKName);

USHORT
TDLS_TPKMsg3Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen);

NDIS_STATUS
TDLS_TearDownAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UINT16	ReasonCode,
	IN BOOLEAN	bDirect);

BOOLEAN
MlmeTdlsReqSanity(
	IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PRT_802_11_TDLS *pTDLS,
    OUT PUINT16 pReason,
    OUT BOOLEAN *pIsViaAP);

VOID
TDLS_SendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN 		bQosNull);

VOID
TDLS_LinkMaintenance(
	IN PRTMP_ADAPTER pAd);

INT
Set_TdlsEntryInfo_Display_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR arg);

VOID
TDLS_DiscoveryPacketLearnAndCheck(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR			pAddr);

VOID
TDLS_InitChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	IN HT_CAPABILITY_IE *pHtCapability);
#endif /* __TDLS_H */

#endif // DOT11Z_TDLS_SUPPORT //
