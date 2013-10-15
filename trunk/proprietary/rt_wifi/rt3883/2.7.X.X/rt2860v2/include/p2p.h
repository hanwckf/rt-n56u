/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2005, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attempt
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	p2p.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------

*/

#ifndef __P2P_H__
#define __P2P_H__

#include "p2p_cmm.h"
#include "rtmp.h"

/*
 *  New is here
 */

/* Macro */
#define	P2P_DISABLE		0x00000000
#define P2P_GO_UP		0x00000001
#define P2P_CLI_UP		0x00000002
#define P2P_FIXED_MODE	0x80000000

#define P2P_GO_ON(_pAd) \
	(((_pAd)->flg_p2p_init) \
		&& (((_pAd)->flg_p2p_OpStatusFlags & P2P_GO_UP) == P2P_GO_UP))

#define P2P_CLI_ON(_pAd) \
	(((_pAd)->flg_p2p_init) \
		&& ((_pAd)->flg_p2p_OpStatusFlags == P2P_CLI_UP))

/* ASCII to Integer */

/*	atoi & atoh */

/* event */

/* general */




typedef struct _RTMP_OID_SET_P2P_CONFIG 
{
	UCHAR	ConfigMode;	/* Disable, activate p2p, or activate WPSE, or delete p2p profile. */
	ULONG	WscMode;	/* Method : PIN or PBC or SMPBC */
	UCHAR	PinCode[8];
	UCHAR	DeviceName[32];
	ULONG	DeviceNameLen;
	UCHAR	SSID[32];
	UCHAR	SSIDLen;
	UCHAR	P2PGroupMode; /* temporary or persistent. See definition. */
	UCHAR	GoIntentIdx;	/* Value = 0~15. Intent to be a GO in P2P */
	UCHAR	ConnectingMAC[MAX_P2P_GROUP_SIZE][6];  /* Specify MAC address want to connect. Set to all 0xff or all 0x0 if not specified. */
	UCHAR	ConnectingDeviceName[MAX_P2P_GROUP_SIZE][32];  /* Specify the Device Name that want to connect. Set to all 0xff or all 0x0 if not specified. */
	UCHAR	ListenChannel;
	UCHAR	OperatinChannel;
} RT_OID_SET_P2P_STRUCT, *PRT_OID_SET_P2P_STRUCT;

typedef struct _RALINKIP_IE
{
	UCHAR	ElementID;		/* 0xDD */
	UCHAR	Length;			/* limited by 256 */
	UCHAR	OUI[3];			/* should be SSIDL_OUI {00-0c-43-} */
	UCHAR	OUIMode;			/* See definition RALINKOUIMODE_xxxx */
	UCHAR	Octet[1];		/* Set bit position according to Aid.  (Like DTIM) */
} RALINKIP_IE, *PRALINKIP_IE;

typedef struct _RALINKMBRIP_ELEM
{
	UCHAR	Addr[MAC_ADDR_LEN];		  
	ULONG	Memberip;			 
} RALINKMBRIP_ELEM, *PRALINKMBRIP_ELEM;

/* global function prototype for non-p2p modules */


INT Set_P2P_Enable(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Listen_Channel(
		IN	PRTMP_ADAPTER	pAd, 
		IN	PSTRING 		arg);

INT Set_P2P_Operation_Channel(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_GO_Intent(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_Device_Name(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_WSC_Mode(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT  Set_P2P_WSC_ConfMethod(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_NoA_Count(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_NoA_Duration(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_NoA_Interval(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_Extend_Listen(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_Extend_Listen_Interval(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_Extend_Listen_Periodic(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_Intra_Bss(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_Print_Cfg(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Scan(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg);

INT Set_P2P_Print_GroupTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Print_PersistentTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Provision_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Invite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Device_Discoverability_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_P2P_Connect_GoIndex_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Connect_Dev_Addr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Provision_Dev_Addr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_P2P_State_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Reset_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Link_Down_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Sigma_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_QoS_NULL_Legacy_Rate_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_CLIENT_PM_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Persistent_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Enter_WSC_PIN_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Dev_Discoverability_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Default_Config_Method_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_DelDevByAddr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_DevDiscPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_PriDeviceType_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_SecDevTypeList_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Cancel_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

VOID P2PStateMachineInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN	STATE_MACHINE *S, 
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeGASIntialReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeGASIntialRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeGASComebackReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeGASComebackRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeP2pNoaAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeP2pPresRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeP2pGoDiscoverAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID MlmeP2pPresReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerP2pNoaAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerP2pPresRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerP2pGoDiscoverAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerP2pPresReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pSendServiceReqCmd(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR 	p2pindex);

VOID P2pSendPresenceReqCmd(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR	 p2pindex);

VOID P2pGOStartNoA(
	IN PRTMP_ADAPTER pAd);

VOID P2pStopNoA(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY	pMacClient);

VOID P2pStartOpPS(
	IN PRTMP_ADAPTER pAd);

VOID P2pStopOpPS(
	IN PRTMP_ADAPTER pAd);

VOID P2pPreAbsenTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID P2pSwNoATimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID P2pWscTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID P2pReSendTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID P2pCliReConnectTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

BOOLEAN P2pHandleNoAAttri(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY	pMacClient,
	IN PUCHAR pData);

BOOLEAN P2pResetNoATimer(
	IN PRTMP_ADAPTER pAd,
	IN	ULONG	DiffTimeInus);

BOOLEAN 	P2pSetGP(
	IN PRTMP_ADAPTER pAd,
	IN	ULONG	DiffTimeInus);

VOID		P2pGPTimeOutHandle(
	IN PRTMP_ADAPTER pAd);

VOID PeerGASIntialReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerGASIntialRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2PPublicAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pCopyPerstParmToCfg(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		Perstindex);

VOID P2pGetRandomSSID(
	IN PRTMP_ADAPTER pAd,
	OUT PSTRING pSSID,
	OUT PUCHAR pSSIDLen);

INT	P2P_GoSetCommonHT(	
	IN	PRTMP_ADAPTER	pAd);

VOID P2pSetPerstTable(
	IN PRTMP_ADAPTER pAd, 
	IN PVOID pInformationBuffer);

VOID P2pEnable(
	IN PRTMP_ADAPTER pAd);

VOID P2pCfgInit(
	IN PRTMP_ADAPTER pAd);

VOID P2pLinkDown(
	IN PRTMP_ADAPTER pAd,
	IN INT32 type);

VOID P2pScanChannelDefault(
	IN PRTMP_ADAPTER pAd);

VOID P2pPeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID P2pStopConnectThis(
	IN PRTMP_ADAPTER pAd);

VOID P2pScan(
	IN PRTMP_ADAPTER pAd);

VOID P2pStopScan(
	IN PRTMP_ADAPTER pAd);

VOID P2pCheckInviteReq(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN		bIAmGO,
	IN UCHAR		index,
	IN PUCHAR	ChannelList,
	IN PUCHAR	BssidAddr,
	IN UCHAR		OpChannel,
	IN PUCHAR	Ssid,
	IN UCHAR	SsidLen,
	IN UCHAR	*pRspStatus);

VOID P2pCheckInviteReqFromExisting(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	ChannelList,
	IN PUCHAR	BssidAddr,
	IN UCHAR		OpChannel,
	IN PUCHAR	Ssid,
	IN UCHAR		SsidLen,
	IN PUCHAR	pRspStatus);


BOOLEAN P2pCheckChannelList(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pChannelList);

VOID P2PMakeFakeNoATlv(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	 StartTime,
	IN PUCHAR		pOutBuffer);

ULONG InsertP2PGroupInfoTlv(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		pOutBuffer);

ULONG InsertP2PSubelmtTlv(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR			SubId,
	IN PUCHAR		pInBuffer,
	IN PUCHAR		pOutBuffer);

VOID InsertP2pChannelList(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR	 OpChannel, 
	OUT ULONG	 *ChannelListLen, 
	OUT PUCHAR 	pDest);

BOOLEAN P2pParseGroupInfoAttribute(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR P2pindex, 
	IN VOID *Msg, 
	IN ULONG MsgLen);

VOID P2pParseNoASubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	IN UCHAR  wcidindex,
	IN UINT32 Sequence);

VOID P2pParseExtListenSubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT USHORT *ExtListenPeriod,
	OUT USHORT *ExtListenInterval);

VOID P2pParseManageSubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT UCHAR *pChannel,
	OUT UCHAR *pNumOfP2pOtherAttribute,
	OUT UCHAR *pTotalNumOfP2pAttribute,
	OUT UCHAR *pMamageablity,
	OUT UCHAR *pMinorReason);

VOID P2pParseSubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	IN BOOLEAN  bBeacon, 
	OUT USHORT *pDpid,
	OUT UCHAR *pGroupCap,
	OUT UCHAR *pDeviceCap,
	OUT UCHAR *pDeviceName,
	OUT UCHAR *pDeviceNameLen,
	OUT UCHAR *pDevAddr,
	OUT UCHAR *pInterFAddr,
	OUT UCHAR *pBssidAddr,
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pSsid,
	OUT USHORT *pConfigMethod,
	OUT USHORT *pWpsConfigMethod,
	OUT UCHAR *pDevType,
	OUT UCHAR *pListenChannel,
	OUT UCHAR *pOpChannel,
	OUT UCHAR *pChannelList,
	OUT UCHAR *pIntent,
	OUT UCHAR *pStatusCode,
	OUT UCHAR *pInviteFlag);

VOID P2pReceGoNegoConfirmAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem); 

VOID P2pReceGoNegoRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem); 

VOID P2pSendProbeReq(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel);

VOID P2pReceGoNegoReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem); 

VOID P2pReceDevDisReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceDevDisRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceInviteRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceInviteReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceProvisionReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceProvisionRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);


VOID P2PMakeGoNegoConfirm(
	IN PRTMP_ADAPTER pAd,
 	IN PUCHAR		Addr1,
	IN UCHAR			Token,
	IN PUCHAR		pOutBuffer,
	OUT PULONG		pTotalFrameLen);

VOID P2PSendGoNegoConfirm(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR			Token,
	IN UCHAR			idx,
	IN PUCHAR		Addr1);

VOID P2PSendDevDisReq(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		Addr1,
	IN PUCHAR		Bssid,
	IN PUCHAR		ClientAddr1,
	OUT PULONG		pTotalFrameLen);

VOID P2PSendDevDisRsp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR 		RspStatus,
	IN UCHAR 			Token,
	IN PUCHAR		Addr1,
	OUT PULONG		pTotalFrameLen);

VOID P2PSendProvisionReq(
	IN PRTMP_ADAPTER pAd,
	IN USHORT 		ConfigMethod,
	IN UCHAR 			Token,
	IN PUCHAR		Addr1,
	OUT PULONG		pTotalFrameLen);

VOID P2PSendProvisionRsp(
	IN PRTMP_ADAPTER pAd,
	IN USHORT 		ConfigMethod,
	IN UCHAR 			Token,
	IN PUCHAR		Addr1,
	OUT PULONG		pTotalFrameLen);

VOID P2PMakeGoNegoRsp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		Addr1,
	IN USHORT			ReceDpid,
	IN UCHAR			Token,
	IN UCHAR			TempIntent,
	IN UCHAR			Channel,
	IN UCHAR			Status,
	IN PUCHAR		pOutBuffer,
	OUT PULONG		pTotalFrameLen);

VOID P2PMakeGoNegoReq(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR			index,
	IN PUCHAR		Addr1,
	IN PUCHAR		pOutBuffer,
	OUT PULONG		pTotalFrameLen);

VOID P2PMakeInviteReq(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR 			MyRule,
	IN UCHAR 			InviteFlag,
	IN PUCHAR		Addr1,
	IN PUCHAR		Bssid,
	OUT PULONG		pTotalFrameLen);

VOID P2PMakeInviteRsp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR 		MyRule,
	IN UCHAR 		Token,
	IN PUCHAR		Addr1,
	IN PUCHAR		Bssid,
	IN PUCHAR		OpChannel,
	IN PUCHAR		Status,
	OUT PULONG		pTotalFrameLen);

VOID P2pAckRequiredCheck(
	IN PRTMP_ADAPTER pAd,
	IN PP2P_PUBLIC_FRAME	pFrame,
	OUT 		UCHAR	*TempPid);

BOOLEAN IsP2pFirstMacSmaller(
	IN PUCHAR		Firststaddr,
	IN PUCHAR		SecondAddr);

VOID P2pSetListenIntBias(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		Bias);

VOID P2pSetRule(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		Index,
	IN PUCHAR		PeerBssid,
	IN UCHAR		PeerGOIntent,
	IN UCHAR		Channel);

VOID P2pGroupMaintain(
	IN PRTMP_ADAPTER pAd);

VOID P2pCopyP2PTabtoMacTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		P2pindex,
	IN UCHAR		Macindex);

VOID P2pGroupTabInit(
	IN PRTMP_ADAPTER pAd);

VOID P2pGroupTabDisconnect(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bSendDeAuth);

UCHAR P2pGroupTabInsert(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr,
	IN P2P_CLIENT_STATE	State,
	IN CHAR Ssid[], 
	IN UCHAR SsidLen,
	IN UCHAR DevCap,
	IN UCHAR GrpCap);

UCHAR P2pGroupTabDelete(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR    p2pindex, 
	IN PUCHAR    Addr);

UCHAR P2pGroupTabSearch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr); 

VOID P2pPerstTabClean(
	IN PRTMP_ADAPTER pAd);

UCHAR P2pPerstTabInsert(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pAddr,
	IN PWSC_CREDENTIAL pProfile);

UCHAR P2pPerstTabDelete(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	*pMacList);

UCHAR P2pPerstTabSearch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr,
	IN PUCHAR    Bssid,
	IN PUCHAR    InfAddr);

VOID P2pCrednTabClean(
	IN PRTMP_ADAPTER pAd);

VOID P2pCrednTabInsert(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pAddr,
	IN WSC_CREDENTIAL	*pProfile);

VOID P2pCrednTabDelete(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	*pAddr);

BOOLEAN P2pCrednEntrySearch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr,
	IN PUCHAR	ResultIndex);

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
VOID P2pSendWirelessEvent(
    IN PRTMP_ADAPTER pAd,
    IN INT MsgType,
    IN PRT_P2P_CLIENT_ENTRY pP2pEntry,
    IN PUCHAR Addr);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */


BOOLEAN P2pStartGroupForm(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR		idx);

BOOLEAN P2pProvision(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr);

BOOLEAN P2pConnect(
	IN PRTMP_ADAPTER pAd); 

VOID P2pConnectPrepare(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	Addr,
	IN UINT32 ConnType); 

BOOLEAN P2pConnectAfterScan(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN	bBeacon,
	IN UCHAR		idx);

VOID P2pConnectAction(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN	bBeacon,
	IN UCHAR		index);

VOID P2pConnectP2pClient(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR		GrpIndex);

BOOLEAN P2pConnectP2pGo(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR		idx);

BOOLEAN P2pClientDiscovery(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR 		GoP2pTabIdx);

BOOLEAN P2pInviteAsRule(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR		MyRule,
	IN UCHAR 		P2pTabIdx);

BOOLEAN P2pInvite(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR		PersistentTabIdx, 
	IN UCHAR 		P2pTabIdx);

BOOLEAN PeerP2pProbeReqSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
/*	OUT RALINKIP_IE *RequestIp, */
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement, 
/*	OUT USHORT *WSCInfoAtBeaconsLen, */
/*	OUT PUCHAR WSCInfoAtBeacons, */
/*	OUT USHORT *WSCInfoAtProbeRspLen, */
/*	OUT PUCHAR WSCInfoAtProbeRsp, */
	OUT ULONG *WpsIELen, 
	OUT PUCHAR pWpsIE);

BOOLEAN PeerP2pBeaconSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement);

BOOLEAN PeerP2pProbeRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement);

VOID P2pPeerBeaconAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem,
	IN PUCHAR		Bssid);

VOID PeerP2pBeaconProbeRspAtScan(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
/*	IN USHORT CapabilityInfo, */
/*	IN UCHAR	WorkingChannel); */
/*	IN USHORT WSCInfoAtBeaconsLen,
	IN PUCHAR WSCInfoAtBeacons,
	IN USHORT WSCInfoAtProbeRspLen,
	IN PUCHAR WSCInfoAtProbeRsp);*/

VOID PeerP2pBeacon(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	pAddr2,
	IN MLME_QUEUE_ELEM *Elem,
	IN LARGE_INTEGER   TimeStamp);

BOOLEAN PeerBeaconParseRalinkIE( 
		IN PRTMP_ADAPTER pAd, 
		IN VOID *Msg, 
		IN ULONG MsgLen, 
		OUT RALINKIP_IE				*pRalinkIE,
		OUT RALINKMBRIP_ELEM				*pMemberip,
		OUT ULONG *pPeerip);

VOID PeerP2pProbeReq(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
/*	IN BOOLEAN	bSendRsp);  */

VOID P2PParseWPSIE(
	IN PUCHAR	pWpsData,
	IN USHORT		WpsLen,
	OUT PUSHORT	Dpid,
	OUT PUSHORT	ConfigMethod,
	OUT PUCHAR	DeviceName,
	OUT UCHAR	*DeviceNameLen);

BOOLEAN P2PDeviceMatch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN PUCHAR	DeviceName,
	IN ULONG		DeviceNameLen);

VOID P2PMakeProbe(
	IN PRTMP_ADAPTER pAd,  
	IN MLME_QUEUE_ELEM *Elem, 
	IN UCHAR		DsChannel,
	IN USHORT	SubType,
	OUT PUCHAR pDest,
	OUT	ULONG *pFrameLen);

VOID P2pMakeP2pIE(
	IN	PRTMP_ADAPTER	pAd,
	IN 	UCHAR			PacketType,
	OUT	PUCHAR			pOutBuf,
	OUT	PULONG			pIeLen);

VOID P2pSetWps(
	IN	PRTMP_ADAPTER	pAd,
	IN PRT_P2P_CLIENT_ENTRY pP2pEntry); 

VOID P2pGotoIdle(
	IN PRTMP_ADAPTER pAd);

VOID P2pGotoScan(
	IN PRTMP_ADAPTER pAd);

VOID P2pGoNegoDone(
	IN PRTMP_ADAPTER pAd,
	IN PRT_P2P_CLIENT_ENTRY pP2pEntry);

VOID P2pWpsDone(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		pAddr);

VOID P2pCopyMacTabtoP2PTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		P2pindex,
	IN UCHAR		Macindex);

VOID P2pReceiveEapNack(
	IN PRTMP_ADAPTER pAd,
	IN	PMLME_QUEUE_ELEM	pElem);

VOID P2pMakeProbeRspWSCIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR			pOutBuf,
	OUT	PULONG			pIeLen);

VOID P2pMakeProbeReqIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen);

VOID P2pStartAutoGo(
	IN PRTMP_ADAPTER pAd);

VOID P2pStartGo(
	IN PRTMP_ADAPTER pAd);

VOID P2pPauseBssSync(
	IN PRTMP_ADAPTER pAd);

VOID P2pResumeBssSync(
	IN PRTMP_ADAPTER pAd);

VOID P2PCTWindowTimer(
	IN PVOID	SystemSpecific1, 
	IN PVOID	FunctionContext, 
	IN PVOID	SystemSpecific2, 
	IN PVOID	SystemSpecific3);

VOID GoPeerDisassocReq(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	Addr2);

ULONG P2pUpdateGroupBeacon(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	StartPosition);

ULONG P2pUpdateNoABeacon(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
/*	IN ULONG	StartPosition); */
	IN PUCHAR	pDest);

ULONG P2pUpdateNoAProbeRsp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pInbuffer);

VOID P2pUpdateBssBeacon(
	IN PRTMP_ADAPTER pAd,
	IN  PUCHAR	 pCapability,
	IN  PUCHAR	pIpReq);

VOID P2pMakeBssBeacon(
	IN PRTMP_ADAPTER pAd);

VOID	P2PInitListenTimer(
	IN PRTMP_ADAPTER	pAd,
	UINT32	value);

VOID	P2PSetListenTimer(
	IN PRTMP_ADAPTER	pAd,
	UINT32	value);
	
VOID	P2PListenTimerExec(
	IN PRTMP_ADAPTER	pAd,
	UINT32	value);

VOID	P2PInitNextScanTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value);

VOID	P2PSetNextScanTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value);

VOID	P2PNextScanTimerExec(
	IN PRTMP_ADAPTER pAd,
	UINT32	value);

VOID	P2PInitDevDiscTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value);

VOID	P2PSetDevDiscTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value);

VOID	P2PDevDiscTimerExec(
	IN PRTMP_ADAPTER pAd,
	UINT32	value);

VOID P2P_SetWscRule(
	IN PRTMP_ADAPTER pAd,
	UCHAR	index,
	PUSHORT PeerWscMethod);

VOID GOUpdateBeaconFrame(
	IN PRTMP_ADAPTER pAd);

UCHAR ChannelToClass(
	IN UCHAR		Channel,
	IN UCHAR		Country);

VOID P2PCtrlStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID P2PDiscoveryStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID P2PGoFormationStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID P2PStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID P2PInitChannelRelatedValue(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS P2PAPInitialize(
	IN  PRTMP_ADAPTER   pAd);

VOID P2PAPShutdown(
	IN PRTMP_ADAPTER pAd);

VOID P2PUserCfgInit(
	IN	PRTMP_ADAPTER pAd);

PSTRING decodeDpid (USHORT dpid);
PSTRING decodeConfigMethod (USHORT ConfigMethos);
PSTRING decodeP2PState (UCHAR P2pState);
PSTRING decodeP2PClientState (P2P_CLIENT_STATE P2pClientState);
PSTRING decodeMyRule (USHORT Rule);
PSTRING decodeCtrlState (UCHAR State);
PSTRING decodeDiscoveryState (UCHAR State);
PSTRING decodeGroupFormationState (UCHAR State);
VOID decodeDeviceCap (UCHAR State);
VOID decodeGroupCap (UCHAR State);


VOID P2PPrintMac(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	macindex);

VOID P2PPrintP2PEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		p2pindex);

VOID P2PPrintP2PPerstEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		p2pindex);

VOID P2P_GoStartUp(
	IN PRTMP_ADAPTER 	pAd,
	IN INT         	bssidx);

VOID P2P_GoStop(
	IN PRTMP_ADAPTER pAd);

VOID P2P_CliStartUp(
	IN PRTMP_ADAPTER 	pAd);

VOID P2P_CliStop(
	IN PRTMP_ADAPTER 	pAd);

VOID AsicEnableP2PGoSync(
	IN PRTMP_ADAPTER pAd);

VOID MgtMacP2PHeaderInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN OUT PHEADER_802_11 pHdr80211, 
	IN UCHAR SubType, 
	IN UCHAR ToDs, 
	IN PUCHAR pDA, 
	IN PUCHAR pBssid);

VOID P2PMacTableReset(
	IN  PRTMP_ADAPTER  pAd);

VOID P2PMacTableMaintenance(
	IN PRTMP_ADAPTER pAd);

VOID P2PChannelInit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		ifIndex);

VOID	P2PCfgInit(
	IN	PRTMP_ADAPTER pAd);

VOID P2PUpdateMlmeRate(
	IN PRTMP_ADAPTER pAd,
	USHORT ifIndex);

VOID P2pInit(
	IN PRTMP_ADAPTER 				pAd,
	IN RTMP_OS_NETDEV_OP_HOOK		*pNetDevOps);

INT P2P_OpenPre(
	IN	PNET_DEV					pDev);

INT P2P_OpenPost(
	IN	PNET_DEV					pDev);

INT P2P_Close(
	IN	PNET_DEV					pDev);

VOID P2P_Remove(
	IN PRTMP_ADAPTER				pAd);


/* ******************************************************************** */
/* WiFi Direct control */

#define P2P_INC_CHA_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx)++;                              \
    if ((_idx) >= (_RingSize)) _idx=0;     \
}
/*  channel 1, 6, 11 is listen channel. */
#define P2P_LISTEN_CHA_SIZE		3


#define ICS_STATUS_ENABLED		1
#define ICS_STATUS_DISABLED		0



#define P2PMANAGED_ENABLE_BIT		0x1
#define P2PMANAGED_ICS_ENABLE_BIT		0x2
#define P2PMANAGED_COEXIST_OPT_BIT		0x4

#define P2P_OPPS_BIT		0x80

/*
 *  Macros for bit check
*/

#define P2P_TEST_BIT(_M, _F)      (((_M) & (_F)) != 0)



/* Packet Format. */
#define IE_P2P					0xdd

typedef struct {
	UCHAR		OUI[3];
	UCHAR		OUIType;
	UCHAR            Octet[1];
} P2P_IE, *PP2P_IE;

#define SIZE_OF_FIXED_CLIENT_INFO_DESC	25




typedef struct _OID_P2P_PERSISTENT_TABLE
{
	UCHAR  PerstNumber;  /* What persistent profile is set ? */
	RT_P2P_PERSISTENT_ENTRY  PerstEntry[MAX_P2P_TABLE_SIZE]; /* Save persistent profile for auto reconnect */
} OID_P2P_PERSISTENT_TABLE, *POID_P2P_PERSISTENT_TABLE;






typedef struct {
    UCHAR   Eid;
    UCHAR   Len[2];
    CHAR   Octet[1];
} P2PEID_STRUCT,*PP2PEID_STRUCT;


#endif /* __P2P_H__ */


