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
	p2p_ctrl.c

	Abstract:
	Peer to peer is also called Wifi Direct. P2P is a Task Group of WFA.

	Revision History:
	Who              When               What
	--------    ----------    ----------------------------------------------

*/
#include "rt_config.h"


/* Group Formation Action */

VOID P2pPeerGoNegoReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pPeerGoNegoRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pPeerGoNegoConfirmAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);


VOID P2pPeerProvisionReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pPeerProvisionRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);


VOID P2pPeerDeviceDiscRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pPeerInvitesReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pPeerInvitesRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pStartCommunicateAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pSendProvisionCmd(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pSendInviteCmd(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pSendDevDiscCmd(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pSendServDiscCmd(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pSendStartGroupFormCmd(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pPeerDevDiscoverReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID P2pSendPassedAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID InvalidP2PGoNegoState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static p2p_cmd_handler rt_p2p_handler[] =
{
	(p2p_cmd_handler) NULL,									/* P2PSTATE_NONE, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_DISCOVERY, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_DISCOVERY_GO, */
	(p2p_cmd_handler) NULL, 									/* P2PSTATE_DISCOVERY_CLIENT, */
	(p2p_cmd_handler) NULL, 									/* P2PSTATE_DISCOVERY_UNKNOWN, */
	(p2p_cmd_handler) NULL, 									/* P2PSTATE_CLIENT_DISCO_COMMAND, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_WAIT_GO_DISCO_ACK, */
	(p2p_cmd_handler) NULL, 									/* P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS, */
	(p2p_cmd_handler) P2pSendDevDiscCmd, 					/* P2PSTATE_GO_DISCO_COMMAND, */
	(p2p_cmd_handler) P2pSendInviteCmd,						/* P2PSTATE_INVITE_COMMAND, */
	(p2p_cmd_handler) P2pSendStartGroupFormCmd,				/* P2PSTATE_CONNECT_COMMAND, */
	(p2p_cmd_handler) P2pSendProvisionCmd,					/* P2PSTATE_PROVISION_COMMAND, */
	(p2p_cmd_handler) P2pSendServDiscCmd,					/* P2PSTATE_SERVICE_DISCO_COMMAND, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_SERVICE_COMEBACK_COMMAND, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_SENT_INVITE_REQ, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_SENT_PROVISION_REQ, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_SENT_PROVISION_RSP, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS, */		
	(p2p_cmd_handler) P2pSendStartGroupFormCmd,				/* P2PSTATE_SENT_GO_NEG_REQ, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GOT_GO_RSP_INFO_UNAVAI, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_WAIT_GO_COMFIRM, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_WAIT_GO_COMFIRM_ACK, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GOT_GO_COMFIRM, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_COMFIRM_ACK_SUCCESS, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_DONE, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_WPS, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_AUTH, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_ASSOC, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_WPS, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_WPS_DONE, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_AUTH, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_ASSOC, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_OPERATING, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_ABSENCE, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_SCAN, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_CLIENT_FIND, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_OPERATING, */		
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_ABSENCE, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_SCAN, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_GO_FIND, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_NONP2P_PSK, */
	(p2p_cmd_handler) NULL,									/* P2PSTATE_NONP2P_WPS, */
};

/*
	==========================================================================
	Description:
		The mesh control state machine, 
	Parameters:
		Sm - pointer to the state machine
	Note:
		the state machine looks like the following
	==========================================================================
 */
VOID P2PGoFormationStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, (ULONG)P2P_GO_FORM_MAX_STATES,
		(ULONG)P2P_GO_NEGO_MAX_EVENTS, (STATE_MACHINE_FUNC)Drop, P2P_GO_FORM_IDLE, P2P_GO_FORM_IDLE);

	/* P2P_GO_FORM_IDLE state */
	/*StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_GO_NEGO_REQ_CMD_EVT, (STATE_MACHINE_FUNC)P2PStartGroupFormsAction); */
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_PEER_GO_NEGO_REQ_EVT, (STATE_MACHINE_FUNC)P2pPeerGoNegoReqAction);
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_PEER_GO_NEGO_RSP_EVT, (STATE_MACHINE_FUNC)P2pPeerGoNegoRspAction); 
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_PEER_GO_NEGO_PROV_REQ_EVT, (STATE_MACHINE_FUNC)P2pPeerProvisionReqAction);
	/*StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_GO_NEGO_PROV_REQ_CMD_EVT, (STATE_MACHINE_FUNC)P2pProvisionReqAction); */
	/*StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_DEV_DISC_REQ_CMD_EVT, (STATE_MACHINE_FUNC)P2pDeviceDiscReqAction); */
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_PEER_INVITE_REQ_EVT, (STATE_MACHINE_FUNC)P2pPeerInvitesReqAction);
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_START_COMMUNICATE_CMD_EVT, (STATE_MACHINE_FUNC)P2pStartCommunicateAction);
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_PEER_DEV_DISC_REQ_EVT, (STATE_MACHINE_FUNC)P2pPeerDevDiscoverReqAction);
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_SEND_PASSED_CMD_EVT, (STATE_MACHINE_FUNC)P2pSendPassedAction);
	StateMachineSetAction(Sm, P2P_GO_FORM_IDLE, P2P_PEER_GO_NEGO_PROV_RSP_EVT, (STATE_MACHINE_FUNC)P2pPeerProvisionRspAction);

	/* P2P_WAIT_GO_FORM_RSP state */
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_RSP, P2P_PEER_GO_NEGO_RSP_EVT, (STATE_MACHINE_FUNC)P2pPeerGoNegoRspAction);
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_RSP, P2P_START_COMMUNICATE_CMD_EVT, (STATE_MACHINE_FUNC)InvalidP2PGoNegoState);

	/* P2P_WAIT_GO_FORM_CONF state */
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_CONF, P2P_PEER_GO_NEGO_CONFIRM_EVT, (STATE_MACHINE_FUNC)P2pPeerGoNegoConfirmAction);
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_CONF, P2P_SEND_PASSED_CMD_EVT, (STATE_MACHINE_FUNC)P2pSendPassedAction);
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_CONF, P2P_PEER_GO_NEGO_REQ_EVT, (STATE_MACHINE_FUNC)P2pPeerGoNegoReqAction); // carella : 2011-06-13.

	/* P2P_GO_FORM_PROV state */
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_PROV_RSP, P2P_PEER_GO_NEGO_PROV_RSP_EVT, (STATE_MACHINE_FUNC)P2pPeerProvisionRspAction);

	/* P2P_GO_FORM_DEV_DISC state */
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_DEV_DISC_RSP, P2P_PEER_DEV_DISC_RSP_EVT, (STATE_MACHINE_FUNC)P2pPeerDeviceDiscRspAction);

	/* P2P_WAIT_GO_FORM_INVITE_RSP state */
	StateMachineSetAction(Sm, P2P_WAIT_GO_FORM_INVITE_RSP, P2P_PEER_INVITE_RSP_EVT, (STATE_MACHINE_FUNC)P2pPeerInvitesRspAction);

	/* init all P2P ctrl state. */
	pAd->P2pCfg.GoFormCurrentState = P2P_GO_FORM_IDLE;

	return;
}
	
/* Group Formation Acrtion */
	
VOID P2pPeerGoNegoReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	if (pAd->P2pCfg.GoFormCurrentState != P2P_GO_FORM_IDLE && pAd->P2pCfg.GoFormCurrentState != P2P_WAIT_GO_FORM_CONF )
		return;
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	if ( pP2PCtrl->bProvAutoRsp == FALSE )
	{
		BOOLEAN 	Cancelled;

		if (pP2PCtrl->bP2pReSendTimerRunning)
		{
			pP2PCtrl->bP2pReSendTimerRunning = FALSE;
			pAd->P2pTable.Client[pP2PCtrl->P2pProvIndex].ReTransmitCnt = 0;
			RTMPCancelTimer(&pP2PCtrl->P2pReSendTimer, &Cancelled);
		}
		
		pP2PCtrl->P2pProvIndex = P2P_NOT_FOUND;
		pP2PCtrl->P2pProvUserNotify = FALSE;
		pAd->P2pCfg.P2pCounter.UserAccept = 0;
	}

	/*
		Skip auto scan conn in STAMlmePeriodicExec
	*/
	pAd->StaCfg.bSkipAutoScanConn = TRUE;
	P2pReceGoNegoReqAction(pAd, Elem);
	
	*pCurrState = P2P_WAIT_GO_FORM_CONF;
}

VOID P2pPeerGoNegoRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	if (pAd->P2pCfg.GoFormCurrentState != P2P_GO_FORM_IDLE && pAd->P2pCfg.GoFormCurrentState != P2P_WAIT_GO_FORM_RSP )
		return;
	
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	*pCurrState = P2P_GO_FORM_IDLE;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	P2pReceGoNegoRspAction(pAd, Elem);

}

VOID P2pPeerGoNegoConfirmAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	if (pAd->P2pCfg.GoFormCurrentState != P2P_WAIT_GO_FORM_CONF)
		return;

	P2pReceGoNegoConfirmAction(pAd, Elem);
	/*
		DO NOT skip auto scan conn in STAMlmePeriodicExec
	*/
	pAd->StaCfg.bSkipAutoScanConn = FALSE;
	*pCurrState = P2P_GO_FORM_IDLE;
}


VOID P2pPeerProvisionReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	if (pAd->P2pCfg.GoFormCurrentState != P2P_GO_FORM_IDLE)
		return;

	/*
		Skip auto scan conn in STAMlmePeriodicExec
	*/
	pAd->StaCfg.bSkipAutoScanConn = TRUE;
	
	P2pReceProvisionReqAction(pAd, Elem);

	*pCurrState = P2P_GO_FORM_IDLE;
}

VOID P2pPeerProvisionRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	if (pAd->P2pCfg.GoFormCurrentState != P2P_WAIT_GO_FORM_PROV_RSP && pAd->P2pCfg.GoFormCurrentState != P2P_GO_FORM_IDLE)

		return;

	P2pReceProvisionRspAction(pAd, Elem);
	MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_CANL_CMD_EVT, 0, NULL, 0);

	/*
		DO NOT skip auto scan conn in STAMlmePeriodicExec
	*/
	pAd->StaCfg.bSkipAutoScanConn = FALSE;
	*pCurrState = P2P_GO_FORM_IDLE;
}


VOID P2pPeerDeviceDiscRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{	
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	if (pAd->P2pCfg.GoFormCurrentState != P2P_WAIT_GO_FORM_DEV_DISC_RSP)
		return;

	P2pReceDevDisRspAction(pAd, Elem);

	*pCurrState = P2P_GO_FORM_IDLE;
}

VOID P2pPeerInvitesReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	if (pAd->P2pCfg.GoFormCurrentState != P2P_GO_FORM_IDLE )

		return;	

	/*
		Skip auto scan conn in STAMlmePeriodicExec
	*/
	pAd->StaCfg.bSkipAutoScanConn = TRUE;
	P2pReceInviteReqAction(pAd, Elem);

	*pCurrState = P2P_GO_FORM_IDLE;
}

VOID P2pPeerInvitesRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	if (pAd->P2pCfg.GoFormCurrentState != P2P_WAIT_GO_FORM_INVITE_RSP)
		return;	

	P2pReceInviteRspAction(pAd, Elem);
	MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_CANL_CMD_EVT, 0, NULL, 0);

	/*
		DO NOT skip auto scan conn in STAMlmePeriodicExec
	*/
	pAd->StaCfg.bSkipAutoScanConn = FALSE;
	*pCurrState = P2P_GO_FORM_IDLE;
}

VOID P2pPeerDevDiscoverReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);

	if (pAd->P2pCfg.GoFormCurrentState != P2P_GO_FORM_IDLE)
		return;	

	P2pReceDevDisReqAction(pAd, Elem);

	*pCurrState = P2P_GO_FORM_IDLE;
}

VOID InvalidP2PGoNegoState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	*pCurrState = P2P_GO_FORM_IDLE;
}

VOID P2pStartCommunicateAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	p2p_cmd_handler p2p_handler;

	p2p_handler = rt_p2p_handler[Elem->Priv];
	if (p2p_handler)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: CMD_ID = %ld\n", __FUNCTION__, Elem->Priv));
		p2p_handler(pAd, Elem);
	}
}

VOID P2pSendProvisionCmd(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	PP2P_CMD_STRUCT pP2pCmd = (PP2P_CMD_STRUCT)Elem->Msg;
	UCHAR p2pIdx;
	ULONG	FrameLen;
	USHORT PeerWscMethod;
	UCHAR Addr[6] = {0};
	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	DBGPRINT(RT_DEBUG_ERROR, ("Addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pP2pCmd->Addr)));
	p2pIdx = P2pGroupTabSearch(pAd, pP2pCmd->Addr);
	P2PPrintP2PEntry(pAd, p2pIdx);

	if (p2pIdx == P2P_NOT_FOUND)
		return;

	if (pAd->P2pTable.Client[p2pIdx].Rule == P2P_IS_GO)
		RTMPMoveMemory(&Addr, pAd->P2pTable.Client[p2pIdx].bssid, sizeof(Addr));
	else
		RTMPMoveMemory(&Addr, pAd->P2pTable.Client[p2pIdx].addr, sizeof(Addr));
	
	if (pAd->P2pCfg.Dpid != DEV_PASS_ID_NOSPEC)
	{
		if ((pAd->P2pCfg.Dpid == DEV_PASS_ID_USER) && ((pAd->P2pTable.Client[p2pIdx].ConfigMethod & WSC_CONFMET_DISPLAY) != 0))
		{
			pAd->P2pTable.Client[p2pIdx].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
			P2PSendProvisionReq(pAd, WSC_CONFMET_DISPLAY, pAd->P2pTable.Client[p2pIdx].GeneralToken, pAd->P2pTable.Client[p2pIdx].addr, &FrameLen);
			DBGPRINT(RT_DEBUG_ERROR, ("Request : CONFIG_METHOD_DISPLAY	\n"));
		}
		else if ((pAd->P2pCfg.Dpid == DEV_PASS_ID_PBC) && ((pAd->P2pTable.Client[p2pIdx].ConfigMethod & WSC_CONFMET_PBC) != 0))
		{
			pAd->P2pTable.Client[p2pIdx].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
			P2PSendProvisionReq(pAd, WSC_CONFMET_PBC, pAd->P2pTable.Client[p2pIdx].GeneralToken, pAd->P2pTable.Client[p2pIdx].addr, &FrameLen);
			DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_PUSHBUTTON  \n"));
		}
		else if ((pAd->P2pCfg.Dpid == DEV_PASS_ID_REG) && ((pAd->P2pTable.Client[p2pIdx].ConfigMethod&WSC_CONFMET_KEYPAD) != 0))
		{
			pAd->P2pTable.Client[p2pIdx].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
			P2PSendProvisionReq(pAd, WSC_CONFMET_KEYPAD, pAd->P2pTable.Client[p2pIdx].GeneralToken, pAd->P2pTable.Client[p2pIdx].addr, &FrameLen);
			DBGPRINT(RT_DEBUG_ERROR, ("Request : CONFIG_METHOD_KEYPAD  \n"));
		}
		else
		{
			pAd->P2pTable.Client[p2pIdx].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
			if (pAd->P2pCfg.ConfigMethod == WSC_CONFMET_DISPLAY)
				P2PSendProvisionReq(pAd, WSC_CONFMET_KEYPAD, pAd->P2pTable.Client[p2pIdx].GeneralToken, Addr/*pAd->P2pTable.Client[p2pIdx].addr*/, &FrameLen);
			else if (pAd->P2pCfg.ConfigMethod == WSC_CONFMET_KEYPAD)
				P2PSendProvisionReq(pAd, WSC_CONFMET_DISPLAY, pAd->P2pTable.Client[p2pIdx].GeneralToken, Addr/*pAd->P2pTable.Client[p2pIdx].addr*/, &FrameLen);
			else
				P2PSendProvisionReq(pAd, WSC_CONFMET_PBC, pAd->P2pTable.Client[p2pIdx].GeneralToken, Addr/*pAd->P2pTable.Client[p2pIdx].addr*/, &FrameLen);
		}
	}
	else
	{
		P2P_SetWscRule(pAd, pP2pCmd->Idx, &PeerWscMethod);
		pAd->P2pTable.Client[p2pIdx].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
		P2PSendProvisionReq(pAd, PeerWscMethod, pAd->P2pTable.Client[p2pIdx].GeneralToken, pAd->P2pTable.Client[p2pIdx].addr, &FrameLen);
		DBGPRINT(RT_DEBUG_ERROR, ("Request : CONFIG_METHOD_KEYPAD  \n"));
	}

	*pCurrState = P2P_WAIT_GO_FORM_PROV_RSP;
}


VOID P2pSendInviteCmd(
		IN PRTMP_ADAPTER pAd,
		IN MLME_QUEUE_ELEM *Elem)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	PP2P_CMD_STRUCT pP2pCmd = (PP2P_CMD_STRUCT)Elem->Msg;
	UCHAR perstindex, p2pIndex;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	DBGPRINT(RT_DEBUG_ERROR, ("Addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pP2pCmd->Addr)));

	p2pIndex = pP2pCmd->Idx;
	if (pP2PCtrl->Rule == P2P_IS_GO)
	{
		/* Invite Case 1 : I am Auto GO to invite a P2P Device or when I am P2P Client */
		P2pInvite(pAd, pP2pCmd->Addr, MAX_P2P_TABLE_SIZE, p2pIndex);
	}
	else if (IS_P2P_CONNECT_IDLE(pAd))
	{
		/* since I am idle,  */
		perstindex = P2pPerstTabSearch(pAd, pAd->P2pTable.Client[p2pIndex].addr, pAd->P2pTable.Client[p2pIndex].bssid, pAd->P2pTable.Client[p2pIndex].InterfaceAddr);
		if ((perstindex < MAX_P2P_TABLE_SIZE) && (IS_PERSISTENT_ON(pAd)))
		{
			/*
				I have credential, my persistent is enabled, peer 's persistent is enabled.	
				So use Reinvoke method to start P2P group.
			 */
			P2pInvite(pAd, pAd->P2pTable.Client[p2pIndex].addr, perstindex, p2pIndex);
			pAd->P2pCfg.P2PConnectState = P2P_INVITE;
		}
	}

	*pCurrState = P2P_WAIT_GO_FORM_INVITE_RSP;
}

VOID P2pSendDevDiscCmd(
		IN PRTMP_ADAPTER pAd,
		IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	PP2P_CMD_STRUCT pP2pCmd = (PP2P_CMD_STRUCT)Elem->Msg;
	UCHAR p2pIdx;
	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	DBGPRINT(RT_DEBUG_ERROR, ("Addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pP2pCmd->Addr)));
	p2pIdx = P2pGroupTabSearch(pAd, pP2pCmd->Addr);
	P2PPrintP2PEntry(pAd, p2pIdx);

	if (p2pIdx == P2P_NOT_FOUND)
		return;

	P2pClientDiscovery(pAd, pAd->P2pTable.Client[p2pIdx].addr, p2pIdx);
	*pCurrState = P2P_WAIT_GO_FORM_DEV_DISC_RSP;
}

VOID P2pSendServDiscCmd(
		IN PRTMP_ADAPTER pAd,
		IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	PP2P_CMD_STRUCT pP2pCmd = (PP2P_CMD_STRUCT)Elem->Msg;
	UCHAR p2pIdx;
	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	DBGPRINT(RT_DEBUG_ERROR, ("Addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pP2pCmd->Addr)));
	p2pIdx = P2pGroupTabSearch(pAd, pP2pCmd->Addr);
	P2PPrintP2PEntry(pAd, p2pIdx);

	if (p2pIdx == P2P_NOT_FOUND)
		return;

	P2pSendServiceReqCmd(pAd, pAd->P2pTable.Client[p2pIdx].addr, p2pIdx);
	pAd->P2pTable.Client[p2pIdx].P2pClientState = P2PSTATE_DISCOVERY;
	*pCurrState = P2P_WAIT_GO_FORM_SRV_DISC_RSP;
}

VOID P2pSendStartGroupFormCmd(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	PP2P_CMD_STRUCT pP2pCmd = (PP2P_CMD_STRUCT)Elem->Msg;
	USHORT PeerWscMethod;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	DBGPRINT(RT_DEBUG_ERROR, ("Addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pP2pCmd->Addr)));
	P2PPrintP2PEntry(pAd, pP2pCmd->Idx);
	*pCurrState = P2P_WAIT_GO_FORM_RSP;
	if (pP2PCtrl->Dpid == DEV_PASS_ID_NOSPEC)
		P2P_SetWscRule(pAd, pP2pCmd->Idx, &PeerWscMethod);
	P2pStartGroupForm(pAd, pAd->P2pTable.Client[pP2pCmd->Idx].addr, pP2pCmd->Idx);
}

VOID P2pSendPassedAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_GO_FORM_STATE *pCurrState = &(pAd->P2pCfg.GoFormCurrentState);
	PP2P_CMD_STRUCT pP2pCmd = (PP2P_CMD_STRUCT)Elem->Msg;
	UCHAR index = pP2pCmd->Idx;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	DBGPRINT(RT_DEBUG_ERROR, ("Addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pP2pCmd->Addr)));

	P2pConnectPrepare(pAd, pP2pCmd->Addr, P2PSTATE_CONNECT_COMMAND);
	*pCurrState = P2P_GO_FORM_IDLE;
}

