/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


	Module Name:
	apcli_auth.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-6-23		modified for rt61-APClinent
*/

#ifdef P2P_SUPPORT

#include "rt_config.h"

static VOID ApCliAuthTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

static VOID ApCliMlmeAuthReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerAuthRspAtSeq2Action(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerAuthRspAtSeq4Action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerDeauthAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliAuthTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenAuth(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliMlmeDeauthReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

DECLARE_TIMER_FUNCTION(ApCliAuthTimeout);
BUILD_TIMER_FUNCTION(ApCliAuthTimeout);

/*
	==========================================================================
	Description:
		authenticate state machine init, including state transition and timer init
	Parameters:
		Sm - pointer to the auth state machine
	Note:
		The state machine looks like this
	==========================================================================
 */

VOID ApCliAuthStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_AUTH_STATE, APCLI_MAX_AUTH_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_AUTH_REQ_IDLE,
		APCLI_AUTH_MACHINE_BASE);

	/* the first column */
	StateMachineSetAction(Sm, APCLI_AUTH_REQ_IDLE, APCLI_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeAuthReqAction);
	StateMachineSetAction(Sm, APCLI_AUTH_REQ_IDLE, APCLI_MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)ApCliPeerDeauthAction);
	StateMachineSetAction(Sm, APCLI_AUTH_REQ_IDLE, APCLI_MT2_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDeauthReqAction);

	/* the second column */
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAuth);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)ApCliPeerAuthRspAtSeq2Action);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)ApCliPeerDeauthAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAuthTimeoutAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDeauthReqAction);

	/* the third column */
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAuth);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)ApCliPeerAuthRspAtSeq4Action);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)ApCliPeerDeauthAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAuthTimeoutAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDeauthReqAction);

	/* timer init */
	RTMPInitTimer(pAd, &pAd->ApCliMlmeAux.ApCliAuthTimer, GET_TIMER_FUNCTION(ApCliAuthTimeout), pAd, FALSE);

	for (i=0; i < MAX_APCLI_NUM; i++)
		pAd->ApCfg.ApCliTab[i].AuthCurrState = APCLI_AUTH_REQ_IDLE;

	return;
}

/*
	==========================================================================
	Description:
		function to be executed at timer thread when auth timer expires
	==========================================================================
 */
static VOID ApCliAuthTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - AuthTimeout\n"));

	MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_AUTH_TIMEOUT, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliMlmeAuthReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN             Cancelled;
	NDIS_STATUS         NState;
	UCHAR               Addr[MAC_ADDR_LEN];
	USHORT              Alg, Seq, Status;
	ULONG               Timeout;
	HEADER_802_11       AuthHdr; 
	PUCHAR              pOutBuffer = NULL;
	ULONG               FrameLen = 0;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	/* Block all authentication request durning WPA block period */
	if (pAd->ApCfg.ApCliTab[ifIndex].bBlockAssoc == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Block Auth request durning WPA block period!\n"));
		*pCurrState = APCLI_AUTH_REQ_IDLE;
		ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}
	else if(MlmeAuthReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr, &Timeout, &Alg))
	{
		/* reset timer */
		RTMPCancelTimer(&pAd->ApCliMlmeAux.ApCliAuthTimer, &Cancelled);

		pAd->ApCliMlmeAux.Alg  = Alg;

		Seq = 1;
		Status = MLME_SUCCESS;

		/* allocate and send out AuthReq frame */
		NState = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
		if(NState != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - MlmeAuthReqAction() allocate memory failed\n"));
			*pCurrState = APCLI_AUTH_REQ_IDLE;

			ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			return;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Send AUTH request seq#1 (Alg=%d)...\n", Alg));
		ApCliMgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr, pAd->ApCliMlmeAux.Bssid, ifIndex);

		MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
						  sizeof(HEADER_802_11),&AuthHdr, 
						  2,                    &Alg, 
						  2,                    &Seq, 
						  2,                    &Status, 
						  END_OF_ARGS);

		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

		RTMPSetTimer(&pAd->ApCliMlmeAux.ApCliAuthTimer, AUTH_TIMEOUT);

		*pCurrState = APCLI_AUTH_WAIT_SEQ2;
	} else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("APCLI AUTH - MlmeAuthReqAction() sanity check failed. BUG!!!!!\n"));
		*pCurrState = APCLI_AUTH_REQ_IDLE;
	}

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliPeerAuthRspAtSeq2Action(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	BOOLEAN         Cancelled;
	UCHAR           Addr2[MAC_ADDR_LEN];
	USHORT          Seq, Status, Alg;
	USHORT          RemoteStatus;
	UCHAR			iv_hdr[LEN_WEP_IV_HDR];
/*	UCHAR           ChlgText[CIPHER_TEXT_LEN]; */
	UCHAR           *ChlgText = NULL;
	UCHAR           CyperChlgText[CIPHER_TEXT_LEN + 8 + 8];
	ULONG			c_len = 0;	
	HEADER_802_11   AuthHdr;
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer = NULL;
	ULONG           FrameLen = 0;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	UCHAR		  	ChallengeIe = IE_CHALLENGE_TEXT;
	UCHAR		  	len_challengeText = CIPHER_TEXT_LEN;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ChlgText, CIPHER_TEXT_LEN);
	if (ChlgText == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	if(PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, (CHAR *) ChlgText))
	{
		if(MAC_ADDR_EQUAL(pAd->ApCliMlmeAux.Bssid, Addr2) && Seq == 2)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Receive AUTH_RSP seq#2 to me (Alg=%d, Status=%d)\n", Alg, Status));
			RTMPCancelTimer(&pAd->ApCliMlmeAux.ApCliAuthTimer, &Cancelled);

			if(Status == MLME_SUCCESS)
			{
				if(pAd->ApCliMlmeAux.Alg == Ndis802_11AuthModeOpen)
				{
					*pCurrState = APCLI_AUTH_REQ_IDLE;

					ApCliCtrlMsg.Status= MLME_SUCCESS;
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
						sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
				} 
				else
				{
					PCIPHER_KEY  pKey;	
					UINT	default_key = pAd->ApCfg.ApCliTab[ifIndex].DefaultKeyId;

					pKey = &pAd->ApCfg.ApCliTab[ifIndex].SharedKey[default_key];

					/* 2. shared key, need to be challenged */
					Seq++;
					RemoteStatus = MLME_SUCCESS;
					/* allocate and send out AuthRsp frame	 */				
					NState = MlmeAllocateMemory(pAd, &pOutBuffer); 					
					if(NState != NDIS_STATUS_SUCCESS)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("AUTH - ApCliPeerAuthRspAtSeq2Action allocate memory fail\n"));
						*pCurrState = APCLI_AUTH_REQ_IDLE;

						ApCliCtrlMsg.Status= MLME_FAIL_NO_RESOURCE;
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
						goto LabelOK;
					}

					DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send AUTH request seq#3...\n"));
					ApCliMgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr2, pAd->ApCliMlmeAux.Bssid, ifIndex);
					AuthHdr.FC.Wep = 1;
					
					/* Encrypt challenge text & auth information */
					/* TSC increment */ 
					INC_TX_TSC(pKey->TxTsc, LEN_WEP_TSC);

					/* Construct the 4-bytes WEP IV header */
					RTMPConstructWEPIVHdr(default_key, pKey->TxTsc, iv_hdr);
									 
					Alg = cpu2le16(*(USHORT *)&Alg);
					Seq = cpu2le16(*(USHORT *)&Seq);
					RemoteStatus= cpu2le16(*(USHORT *)&RemoteStatus);                    				

					/* Construct message text */
					MakeOutgoingFrame(CyperChlgText,        &c_len, 
							          2,                    &Alg, 
							          2,                    &Seq,
							          2,                    &RemoteStatus,  
							          1,					&ChallengeIe, 
							          1,					&len_challengeText,
							          len_challengeText,	ChlgText,
							          END_OF_ARGS);

					if (RTMPSoftEncryptWEP(pAd, 
										   iv_hdr, 
										   pKey,
										   CyperChlgText, 
										   c_len) == FALSE)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("AUTH - ApCliPeerAuthRspAtSeq2Action allocate memory fail\n"));
						*pCurrState = APCLI_AUTH_REQ_IDLE;

						ApCliCtrlMsg.Status= MLME_FAIL_NO_RESOURCE;
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
									sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
						goto LabelOK;
					}

					/* Update the total length for 4-bytes ICV */
					c_len += LEN_ICV;

					MakeOutgoingFrame(pOutBuffer,               &FrameLen, 
									  sizeof(HEADER_802_11),    &AuthHdr,  
							          LEN_WEP_IV_HDR,			iv_hdr,								          
							          c_len,     				CyperChlgText, 
									  END_OF_ARGS);

					MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

					RTMPSetTimer(&pAd->ApCliMlmeAux.ApCliAuthTimer, AUTH_TIMEOUT);
					*pCurrState = APCLI_AUTH_WAIT_SEQ4;
				}
			} 
			else
			{
				*pCurrState = APCLI_AUTH_REQ_IDLE;

				ApCliCtrlMsg.Status= Status;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}
		}
	} else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - PeerAuthSanity() sanity check fail\n"));
	}

LabelOK:
	if (ChlgText != NULL)
		os_free_mem(NULL, ChlgText);
	if (pOutBuffer != NULL)
		MlmeFreeMemory(pAd, pOutBuffer);
	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliPeerAuthRspAtSeq4Action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN     Cancelled;
	UCHAR       Addr2[MAC_ADDR_LEN];
	USHORT      Alg, Seq, Status;
	CHAR        ChlgText[CIPHER_TEXT_LEN];
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if(PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, ChlgText))
	{
		if(MAC_ADDR_EQUAL(pAd->ApCliMlmeAux.Bssid, Addr2) && Seq == 4)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Receive AUTH_RSP seq#4 to me\n"));
			RTMPCancelTimer(&pAd->ApCliMlmeAux.ApCliAuthTimer, &Cancelled);

			ApCliCtrlMsg.Status = MLME_SUCCESS;

			if(Status != MLME_SUCCESS)
			{
				ApCliCtrlMsg.Status = Status;
			}

			*pCurrState = APCLI_AUTH_REQ_IDLE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		}
	} else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI - PeerAuthRspAtSeq4Action() sanity check fail\n"));
	}

	return;
}

/*
    ==========================================================================
    Description:
    ==========================================================================
*/
static VOID ApCliPeerDeauthAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR       Addr1[MAC_ADDR_LEN];
	UCHAR       Addr2[MAC_ADDR_LEN];
	UCHAR       Addr3[MAC_ADDR_LEN];
	USHORT      Reason;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, Addr3, &Reason))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH_RSP - receive DE-AUTH from our AP\n"));
		*pCurrState = APCLI_AUTH_REQ_IDLE;

		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PEER_DISCONNECT_REQ, 0, NULL, ifIndex);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH_RSP - ApCliPeerDeauthAction() sanity check fail\n"));
	}

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliAuthTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - AuthTimeoutAction\n"));

	*pCurrState = APCLI_AUTH_REQ_IDLE;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_REQ_TIMEOUT, 0, NULL, ifIndex);

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliInvalidStateWhenAuth(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - InvalidStateWhenAuth (state=%ld), reset AUTH state machine\n",
		pAd->Mlme.ApCliAuthMachine.CurrState));

	*pCurrState= APCLI_AUTH_REQ_IDLE;

	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliMlmeDeauthReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_DEAUTH_REQ_STRUCT pDeauthReq;
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;


	DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - ApCliMlmeAuthReqAction (state=%ld), reset AUTH state machine\n",
		pAd->Mlme.ApCliAuthMachine.CurrState));

	pDeauthReq = (PMLME_DEAUTH_REQ_STRUCT)(Elem->Msg);

	*pCurrState= APCLI_AUTH_REQ_IDLE;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;
	
	DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send DE-AUTH request (Reason=%d)...\n", pDeauthReq->Reason));

	ApCliMgtMacHeaderInit(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pDeauthReq->Addr, pDeauthReq->Addr, ifIndex);
	MakeOutgoingFrame(pOutBuffer,           &FrameLen,
		sizeof(HEADER_802_11),&DeauthHdr,
		2,                    &pDeauthReq->Reason,
		END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	return;
}

#endif /* P2P_SUPPORT */

