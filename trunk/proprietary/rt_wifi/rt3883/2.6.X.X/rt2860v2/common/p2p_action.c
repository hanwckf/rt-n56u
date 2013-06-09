/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	p2p.c

	Abstract:
	Peer to peer is also called Wifi Direct. P2P is a Task Group of WFA.

	Revision History:
	Who              When               What
	--------    ----------    ----------------------------------------------
	Jan Lee         2010-05-21    created for Peer-to-Peer Action frame(Wifi Direct)
*/
#include "rt_config.h"
extern UCHAR	P2POUIBYTE[];

/*	
	==========================================================================
	Description: 
		P2P state machine init. P2P state machine starts to function after P2P group is formed. the main task is support power save
		mechanism.
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2PStateMachineInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN	STATE_MACHINE *Sm, 
	OUT STATE_MACHINE_FUNC Trans[]) 
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, (ULONG)MAX_P2P_STATE, 
		(ULONG)MAX_P2P_MSG, (STATE_MACHINE_FUNC)Drop, P2P_IDLE_STATE, P2P_IDLE_STATE);

	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_NOA, (STATE_MACHINE_FUNC)MlmeP2pNoaAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_PRESENCE_REQ, (STATE_MACHINE_FUNC)MlmeP2pPresReqAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_PRESENCE_RSP, (STATE_MACHINE_FUNC)MlmeP2pPresRspAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_GO_DIS_REQ, (STATE_MACHINE_FUNC)MlmeP2pGoDiscoverAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_INT_REQ, (STATE_MACHINE_FUNC)MlmeGASIntialReqAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_INT_RSP, (STATE_MACHINE_FUNC)MlmeGASIntialRspAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_CB_REQ, (STATE_MACHINE_FUNC)MlmeGASComebackReqAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_CB_RSP, (STATE_MACHINE_FUNC)MlmeGASComebackRspAction);

	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_PEER_P2P_NOA, (STATE_MACHINE_FUNC)PeerP2pNoaAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_PEER_P2P_PRESENCE_REQ, (STATE_MACHINE_FUNC)PeerP2pPresReqAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_PEER_P2P_PRESENCE_RSP, (STATE_MACHINE_FUNC)PeerP2pPresRspAction);
	StateMachineSetAction(Sm, P2P_IDLE_STATE, MT2_PEER_P2P_GO_DIS_REQ, (STATE_MACHINE_FUNC)PeerP2pGoDiscoverAction);

	/* init all P2P ctrl state. */
	pAd->P2pCfg.ActionState = P2P_IDLE_STATE;

}

/*	
	==========================================================================
	Description: 
		Support WiFi Direct Certification test for P2P Client to send Presence Request Test Case..
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2pSendServiceReqCmd(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR	 p2pindex)
{

}

/*	
	==========================================================================
	Description: 
		Support WiFi Direct Certification test for P2P Client to send Presence Request Test Case..
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2pSendPresenceReqCmd(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR	 p2pindex) 
{

}

/*	
	==========================================================================
	Description: 
		P2P Action frame differs only in InBuffer. Others are all common to all ACtion Subtype
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID MlmeP2pCommonAction(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR		OUISubType,
	IN UCHAR		Token,
	IN PUCHAR		pInBuffer,
	IN UCHAR		InBufferLen,
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	PMLME_P2P_ACTION_STRUCT 	  pGoReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
	PUCHAR		   pOutBuffer = NULL;
	NDIS_STATUS 	NStatus;
	ULONG		FrameLen = 0;
	FRAME_P2P_ACTION		Frame;
	ULONG		TmpLen;
	UCHAR		i;
	PUCHAR		pDest;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
	{
		return;
	}
	DBGPRINT(RT_DEBUG_ERROR, (" TO= %x %x %x %x %x %x  \n",  PRINT_MAC(pGoReq->Addr)));
	DBGPRINT(RT_DEBUG_ERROR, (" Bssid= %x %x %x %x %x %x  \n",	PRINT_MAC(pP2PCtrl->CurrentAddress)));

	ActHeaderInit(pAd, &Frame.Hdr, pGoReq->Addr, pP2PCtrl->CurrentAddress, pP2PCtrl->CurrentAddress);
	Frame.Category = MT2_ACT_VENDOR; /* 0x7F */
	RTMPMoveMemory(&Frame.OUI[0], P2POUIBYTE, 4);
	Frame.OUISubType = OUISubType;
	Frame.Token = Token;
	/* No Element */
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
						sizeof(FRAME_P2P_ACTION),	&Frame,
						END_OF_ARGS);

	if ((InBufferLen > 0) && (pInBuffer != NULL))
	{
		MakeOutgoingFrame(pOutBuffer + FrameLen,				&TmpLen,
							InBufferLen,	pInBuffer,
							END_OF_ARGS);
		FrameLen += TmpLen;
	}

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
	pDest = (PUCHAR)&Frame;
	for (i = 0; i <FrameLen; )
	{
		DBGPRINT(RT_DEBUG_ERROR,(": %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2), 
		*(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8)));
		i = i + 9;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("Common P2P ACT request.	 FrameLen = %ld.  \n", FrameLen));
}

/*	
	==========================================================================
	Description: 
		Send Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
		802.11u. 7.4.7.10
		
	Parameters: 
	Note:

	==========================================================================
 */
VOID MlmeGASIntialReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{

}

/*	
	==========================================================================
	Description: 
		Send Publiac action frame. But with ACtion is GAS_INITIAL_RSP (12).
		802.11u. 7.4.7.10
		
	Parameters: 
	Note:

	==========================================================================
 */
VOID MlmeGASIntialRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{

}

/*	
	==========================================================================
	Description: 
		Send Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
		802.11u. 7.4.7.10
		
	Parameters: 
	Note:

	==========================================================================
 */
VOID MlmeGASComebackReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
}

/*	
	==========================================================================
	Description: 
		Send Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
		802.11u. 7.4.7.10
		
	Parameters: 
	Note:

	==========================================================================
 */
VOID MlmeGASComebackRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
}

VOID MlmeP2pNoaAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{

}

VOID MlmeP2pPresReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
}

VOID MlmeP2pPresRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR		NoAAttribute[36];
	UCHAR		p2pindex;
	PMLME_P2P_ACTION_STRUCT 	  pGoReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
	UCHAR			P2pIEFixed[6] = {0xdd, 0x04, 0x00, 0x50, 0xf2, 0x09};	
	UCHAR			Index = 0;

	RTMPMoveMemory(&P2pIEFixed[2], P2POUIBYTE, 4);
	p2pindex = pGoReq->TabIndex;
	/* If AP's index exists, it also means I am connected.	Similar to sanity check. */
	if (p2pindex < MAX_LEN_OF_MAC_TABLE)
	{
		/* Add the header of P2P IE */
		RTMPMoveMemory(&NoAAttribute[0], &P2pIEFixed, 6);
		Index += 6;

		NoAAttribute[Index] = SUBID_P2P_STATUS;
		NoAAttribute[Index+1] = 1;
		NoAAttribute[Index+2] = 0;

		/* Count Field is also called Type. type = 1 means preferred. type = 2 means max limit. */
		if ((pAd->MacTab.Content[p2pindex].P2pInfo.NoADesc[0].Count > 2) || (pAd->MacTab.Content[p2pindex].P2pInfo.NoADesc[0].Count == 0))
		{
			NoAAttribute[Index+3] = P2PSTATUS_INVALID_PARM; /* index */
			MlmeP2pCommonAction(pAd, P2PACT_PERSENCE_RSP, pAd->MacTab.Content[p2pindex].P2pInfo.NoAToken, &NoAAttribute[0], 3+6, Elem);
			DBGPRINT(RT_DEBUG_ERROR, ("P2P- Presence Response sent with error. \n"));
		}
		else
		{
			/* update NoA to Go  */
			pAd->P2pCfg.GONoASchedule.Duration = pAd->MacTab.Content[p2pindex].P2pInfo.NoADesc[0].Duration;
			pAd->P2pCfg.GONoASchedule.Interval = pAd->MacTab.Content[p2pindex].P2pInfo.NoADesc[0].Interval;
			pAd->P2pCfg.GONoASchedule.Count = 255;
		
			NoAAttribute[Index+3] = P2PSTATUS_SUCCESS;	/* index */
			NoAAttribute[Index+4] = SUBID_P2P_NOA;
			NoAAttribute[Index+5] = 15;
			NoAAttribute[Index+6] = 0;
			NoAAttribute[Index+7] = pAd->MacTab.Content[p2pindex].P2pInfo.NoAToken; /* NoAToken should save the token from client's presence request. */
			NoAAttribute[Index+8] = pAd->P2pCfg.CTWindows;	/* CTWindows */
			NoAAttribute[Index+9] = pAd->P2pCfg.GONoASchedule.Count;	/* Count */
			/* TO DO : sync with windows if necessary */
			/*P2pGOStartNoA(pAd); */
			/* Duration */
			RTMPMoveMemory(&NoAAttribute[Index+10], &pAd->P2pCfg.GONoASchedule.Duration, 4);
			/* Interval */
			RTMPMoveMemory(&NoAAttribute[Index+14], &pAd->P2pCfg.GONoASchedule.Interval, 4);
			RTMPMoveMemory(&NoAAttribute[Index+18], &pAd->P2pCfg.GONoASchedule.StartTime, 4);
			/* Update IE length */
			NoAAttribute[1] += 22; 
			MlmeP2pCommonAction(pAd, P2PACT_PERSENCE_RSP, pAd->MacTab.Content[p2pindex].P2pInfo.NoAToken, &NoAAttribute[0], 22+6, Elem);
			DBGPRINT(RT_DEBUG_TRACE, ("P2P- Presence Response sent. \n"));

			/* Trigger to update GO's beacon */
			pAd->P2pCfg.GONoASchedule.bValid = TRUE;
			DBGPRINT(RT_DEBUG_ERROR, ("MlmeP2pPresRspAction: Update NoA Schedual on GO!\n"));
		}

	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: index = %d out of range.\n", __FUNCTION__, p2pindex));

}

VOID MlmeP2pGoDiscoverAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PMLME_P2P_ACTION_STRUCT       pGoReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
	DBGPRINT(RT_DEBUG_TRACE, ("P2P- GO DISCOVERY request. \n"));
	MlmeP2pCommonAction(pAd, P2PACT_GO_DISCOVER_REQ, 0, NULL, 0, Elem);
	
	if (pGoReq->TabIndex < MAX_P2P_GROUP_SIZE)
	{
		/* when used in MlmeP2pGoDiscoverAction, WcidIndex */
		pAd->P2pTable.Client[pGoReq->TabIndex].P2pClientState = P2PSTATE_WAIT_GO_DISCO_ACK;
		DBGPRINT(RT_DEBUG_TRACE, ("P2P- Client State %s  \n", decodeP2PClientState(pAd->P2pTable.Client[pGoReq->TabIndex].P2pClientState)));
	}

}

VOID PeerP2pNoaAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PP2P_ACTION_FRAME	pFrame = (PP2P_ACTION_FRAME)Elem->Msg;
	PMAC_TABLE_ENTRY		pClient;
	
	DBGPRINT(RT_DEBUG_TRACE,("PeerP2pNoaAction  %s. \n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
	DBGPRINT(RT_DEBUG_TRACE,("Category = %d. Subtype = %d. Token = %d.\n", pFrame->Category, pFrame->Subtype, pFrame->Token));
	if (!P2P_CLI_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_TRACE,("PeerP2pNoaAction return %s. \n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
		return;
	}
	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		DBGPRINT(RT_DEBUG_TRACE,("PeerP2pNoaAction Elem->Wcid %d.  return.\n",  Elem->Wcid));
		return;
	}

	pClient = &pAd->MacTab.Content[Elem->Wcid];
	pAd->P2pCfg.NoAIndex = Elem->Wcid;
	DBGPRINT(RT_DEBUG_TRACE,("PeerP2pNoaAction Current  NoAToken = %d. \n",  pClient->P2pInfo.NoAToken));
	if (pFrame->Token != pClient->P2pInfo.NoAToken)
	{
		if ((RTMPEqualMemory(&pFrame->Octet[0], P2POUIBYTE, 4)) && (pFrame->Octet[4] == SUBID_P2P_NOA))
		{
			DBGPRINT(RT_DEBUG_TRACE,("PeerP2pNoaAction  CTWindow = %d. \n", pFrame->Octet[8]));
			pClient->P2pInfo.CTWindow = pFrame->Octet[8]; 
			P2pHandleNoAAttri(pAd, pClient, &pFrame->Octet[0]);
		}
	}
	
}

VOID PeerP2pPresReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	MLME_P2P_ACTION_STRUCT	P2PActReq;	
	MAC_TABLE_ENTRY		*pEntry;
	PFRAME_P2P_ACTION		pFrame;
	PP2P_NOA_DESC	pNoADesc;	
	
	pFrame = (PFRAME_P2P_ACTION)Elem->Msg;

	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("PeerP2pPresReqAction. unknown Elem->Wcid  = %d \n", Elem->Wcid ));
	}
		
	DBGPRINT(RT_DEBUG_ERROR, ("PeerP2pPresReqAction. Send back to Elem->Wcid  = %d \n", Elem->Wcid ));
		
	pEntry = &pAd->MacTab.Content[Elem->Wcid];

	pNoADesc = (PP2P_NOA_DESC)(&Elem->Msg[11 + sizeof(FRAME_P2P_ACTION)]);
	pEntry->P2pInfo.NoADesc[0].Count = pNoADesc->Count;
	pEntry->P2pInfo.NoADesc[0].Duration = *(PUINT32)&pNoADesc->Duration[0];
	pEntry->P2pInfo.NoADesc[0].Interval = *(PUINT32)&pNoADesc->Interval[0];
	pEntry->P2pInfo.NoADesc[0].StartTime = *(PUINT32)&pNoADesc->StartTime[0];
	DBGPRINT(RT_DEBUG_ERROR,(" pP2pEntry->NoADesc[0].Count = %d, \n", pEntry->P2pInfo.NoADesc[0].Count));
	DBGPRINT(RT_DEBUG_ERROR,(" pP2pEntry->NoADesc[0].Duration = %ld, \n", pEntry->P2pInfo.NoADesc[0].Duration));
	DBGPRINT(RT_DEBUG_ERROR,(" pP2pEntry->NoADesc[0].Interval = %ld, \n", pEntry->P2pInfo.NoADesc[0].Interval));
	DBGPRINT(RT_DEBUG_ERROR,(" pP2pEntry->NoADesc[0].StartTime = %ld, \n", pEntry->P2pInfo.NoADesc[0].StartTime));
	DBGPRINT(RT_DEBUG_ERROR,("pFrame->Token  = %d \n", pFrame->Token));

	pEntry->P2pInfo.NoAToken = pFrame->Token;
/*	pP2pEntry->NoADesc[0].Duration = Elem->Msg; */
	NdisZeroMemory(&P2PActReq, sizeof(P2PActReq));
	COPY_MAC_ADDR(P2PActReq.Addr, pEntry->Addr);
	P2PActReq.TabIndex = Elem->Wcid;
	MlmeEnqueue(pAd, P2P_ACTION_STATE_MACHINE, MT2_MLME_P2P_PRESENCE_RSP, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq, 0);
	MlmeHandler(pAd);

}


VOID PeerP2pPresRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	DBGPRINT(RT_DEBUG_TRACE, ("PeerP2pPresRspAction.\n"));
}

VOID PeerP2pGoDiscoverAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR		i;
	PUCHAR		pDest;

	DBGPRINT(RT_DEBUG_TRACE,("PeerP2pGoDiscoverAction.\n"));
	DBGPRINT(RT_DEBUG_TRACE,("bKeepSlient = %d.\n", pAd->P2pCfg.bKeepSlient));
	pDest = &Elem->Msg[0];
	for (i = 0; i <Elem->MsgLen; )
	{
		DBGPRINT(RT_DEBUG_TRACE,(": %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2), 
		*(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8)));
		i = i + 9;
	}
}


