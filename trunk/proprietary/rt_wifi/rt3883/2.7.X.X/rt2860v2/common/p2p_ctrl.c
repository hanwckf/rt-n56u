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

static VOID P2PCtrlDiscoveryAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PCtrlDiscoveryCancelAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PCtrlDiscoveryDoneAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PCtrlGroupFormationAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PCtrlGroupFormationDoneAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

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
	
#ifdef RELEASE_EXCLUDE
/*
							Scan command			P2P Periodic				P2P Scan							CntlOidScanProc 			MlmeCntlMachinePerformAction						  P2P Periodic
	P2P_ENABLE_LISTEN_ONLY ---------------> P2P_IDLE -----------> P2P_SEARCH ---------> P2P_SEARCH_COMMAND ---------------> P2P_SEARCH ---------------------------> P2P_SEARCH_COMPLETE -----------> P2P_IDLE
	MeshCtrlJoinAction : 
		. Init Bss Table
		. Init Mesh Channel
		. enqueue SYNC_STATE_MACHINE with MT2_MLME_SCAN_REQ
	ScanNextChannel :
		. scan done, enequeue MESH_CTRL_STATE_MACHINE with APMT2_MLME_SCAN_FINISH

	P2PCtrlJoinAction
		. Init P2P Table
		. Init random timer
		. enqueeue SYNC_STATE_MACHINE with MT2_MLME_SCAN_REQ
	ScanNextChannel :
		. scan done, enequeue P2P_CTRL_STATE_MACHINE with P2P_SCAN_FINISH
	P2PCtrlFinishDiscoveryAction :
		. Init ScanNextRound
		. change in P2P_IDLE
*/
	
/*

	P2P_CTRL_IDLE : do not scan / search / listen.
					p2p interface up then in IDLE state, or after change as P2P Client / P2P GO in IDLE state.
	P2P_CTRL_DISCOVERY : do scan / search / listen in an period.
					Use P2PDiscMachine do such things and after times up, P2PDiscMachine and P2PCtrlMachine in IDLE state.
					If we have Device Name / Device Address and WSC config method, P2PPeriodicExec should MlmeEnqueue
					P2P_CTRL_GROUP_FORMATION in P2PCtrlMachine.
	P2P_CTRL_GROUP_FORMATION : do Group Negotiation and Provision.
					Use P2PGoFormMachine do such things in ConfigTimeout periodic.
	P2P_CTRL_DONE : Group Formation has finish , interface will up as AP or STA code.
					After AsicKey write done, can change to IDLE state.

*/
#endif /* RELEASE_EXCLUDE */
	 
VOID P2PCtrlStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, (ULONG)P2P_CTRL_MAX_STATES,
		(ULONG)P2P_CTRL_MAX_EVENTS, (STATE_MACHINE_FUNC)Drop, P2P_CTRL_IDLE, P2P_CTRL_IDLE);

	/* P2P_CTRL_IDLE state */
	StateMachineSetAction(Sm, P2P_CTRL_IDLE, P2P_CTRL_DISC_EVT, (STATE_MACHINE_FUNC)P2PCtrlDiscoveryAction);
	StateMachineSetAction(Sm, P2P_CTRL_IDLE, P2P_CTRL_GO_NEGO_EVT, (STATE_MACHINE_FUNC)P2PCtrlGroupFormationAction);

	/* P2P_CTRL_DISCOVERY state */
	StateMachineSetAction(Sm, P2P_CTRL_DISCOVERY, P2P_CTRL_DISC_EVT, (STATE_MACHINE_FUNC)P2PCtrlDiscoveryAction);
	StateMachineSetAction(Sm, P2P_CTRL_DISCOVERY, P2P_CTRL_DISC_CANL_EVT, (STATE_MACHINE_FUNC)P2PCtrlDiscoveryCancelAction);
	StateMachineSetAction(Sm, P2P_CTRL_DISCOVERY, P2P_CTRL_DISC_DONE_EVT, (STATE_MACHINE_FUNC)P2PCtrlDiscoveryDoneAction);
	StateMachineSetAction(Sm, P2P_CTRL_DISCOVERY, P2P_CTRL_GO_NEGO_EVT, (STATE_MACHINE_FUNC)P2PCtrlGroupFormationAction);

	/* P2P_CTRL_GROUP_FORMATION state */
	StateMachineSetAction(Sm, P2P_CTRL_GROUP_FORMATION, P2P_CTRL_GO_NEGO_EVT, (STATE_MACHINE_FUNC)P2PCtrlGroupFormationAction);
	StateMachineSetAction(Sm, P2P_CTRL_GROUP_FORMATION, P2P_CTRL_GO_NEGO_CANL_EVT, (STATE_MACHINE_FUNC)P2PCtrlGroupFormationDoneAction);
	StateMachineSetAction(Sm, P2P_CTRL_GROUP_FORMATION, P2P_CTRL_DISC_CANL_EVT, (STATE_MACHINE_FUNC)P2PCtrlDiscoveryDoneAction);
	StateMachineSetAction(Sm, P2P_CTRL_GROUP_FORMATION, P2P_CTRL_GO_NEGO_DONE_EVT, (STATE_MACHINE_FUNC)P2PCtrlGroupFormationDoneAction);
	StateMachineSetAction(Sm, P2P_CTRL_GROUP_FORMATION, P2P_CTRL_DISC_EVT, (STATE_MACHINE_FUNC)P2PCtrlDiscoveryAction);

	/* P2P_CTRL_DONE state */
	/*StateMachineSetAction(Sm, P2P_CTRL_DONE, Event, (STATE_MACHINE_FUNC)Action); */

	/* init all P2P ctrl state. */
	pAd->P2pCfg.CtrlCurrentState = P2P_CTRL_IDLE;

	return;
}

/* Ctrl Action */
static VOID P2PCtrlDiscoveryAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_CTRL_STATE *pCurrState = &(pAd->P2pCfg.CtrlCurrentState);

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: \n", __FUNCTION__));

	if (P2P_GO_ON(pAd))
		MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_LISTEN_CMD_EVT, 0, NULL, 0);
	else
	MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_SCAN_CMD_EVT, 0, NULL, 0);

	*pCurrState = P2P_CTRL_DISCOVERY;
}

static VOID P2PCtrlDiscoveryCancelAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_CTRL_STATE *pCurrState = &(pAd->P2pCfg.CtrlCurrentState);

	DBGPRINT(RT_DEBUG_TRACE, ("%s::\n", __FUNCTION__));
	/* update Discovery State Machine state. */
	MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_CANL_CMD_EVT, 0, NULL, 0);
	MlmeHandler(pAd);

	*pCurrState = P2P_CTRL_IDLE;
}

static VOID P2PCtrlDiscoveryDoneAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	/*PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;*/
	P2P_CTRL_STATE *pCurrState = &(pAd->P2pCfg.CtrlCurrentState);

	DBGPRINT(RT_DEBUG_TRACE, ("%s::  P2P Device Discovery Finished, found total %d p2p devices.\n", __FUNCTION__, pAd->P2pTable.ClientNumber));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		BOOLEAN bCancelled;
		/* Stop Scan and resume */
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &bCancelled);
		pAd->MlmeAux.Channel = 0;
		ScanNextChannel(pAd, OPMODE_STA);
	}

	P2PInitDevDiscTimer(pAd, 0);
	P2PInitListenTimer(pAd, 0);
	P2PInitNextScanTimer(pAd, 0);
	/* update Discovery State Machine state. */
	MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_CANL_CMD_EVT, 0, NULL, 0);
	*pCurrState = P2P_CTRL_IDLE;
}

static VOID P2PCtrlGroupFormationAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_CTRL_STATE *pCurrState = &(pAd->P2pCfg.CtrlCurrentState);

	DBGPRINT(RT_DEBUG_TRACE, ("%s::\n", __FUNCTION__));
	*pCurrState = P2P_CTRL_GROUP_FORMATION;
}

static VOID P2PCtrlGroupFormationDoneAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	/*PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;*/
	P2P_CTRL_STATE *pCurrState = &(pAd->P2pCfg.CtrlCurrentState);

	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	/*pAd->StaCfg.bAutoReconnect = TRUE;*/
	DBGPRINT(RT_DEBUG_ERROR, ("auto re-conect to GO[%s]\n", pAd->MlmeAux.Ssid));

	*pCurrState = P2P_CTRL_DONE;
}


