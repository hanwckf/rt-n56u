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


static VOID P2PDiscScanAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PDiscListenAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PDiscSearchAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PDiscScanCanlAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PDiscListenCanlAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID P2PDiscSearchCanlAction(
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
	
 #ifdef RELEAE_EXCLUDE
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
VOID P2PDiscoveryStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, (ULONG)P2P_DISC_MAX_STATES,
		(ULONG)P2P_DISC_MAX_EVENTS, (STATE_MACHINE_FUNC)Drop, P2P_DISC_IDLE, P2P_DISC_IDLE);

	/* P2P_DISC_IDLE state */
	StateMachineSetAction(Sm, P2P_DISC_IDLE, P2P_DISC_SCAN_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscScanAction);

	/* P2P_DISC_SCAN state */
	StateMachineSetAction(Sm, P2P_DISC_SCAN, P2P_DISC_LISTEN_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscListenAction);
	StateMachineSetAction(Sm, P2P_DISC_SCAN, P2P_DISC_CANL_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscScanCanlAction);
	StateMachineSetAction(Sm, P2P_DISC_SCAN, P2P_DISC_PEER_PROB_RSP, (STATE_MACHINE_FUNC)PeerP2pBeaconProbeRspAtScan);

	/* P2P_DISC_LISTEN state */
	StateMachineSetAction(Sm, P2P_DISC_LISTEN, P2P_DISC_SEARCH_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscSearchAction);
	StateMachineSetAction(Sm, P2P_DISC_LISTEN, P2P_DISC_CANL_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscListenCanlAction);
	StateMachineSetAction(Sm, P2P_DISC_LISTEN, P2P_DISC_PEER_PROB_REQ, (STATE_MACHINE_FUNC)PeerP2pProbeReq);
	StateMachineSetAction(Sm, P2P_DISC_LISTEN, P2P_DISC_SCAN_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscScanAction);

	/* P2P_DISC_SEARCH state */
	StateMachineSetAction(Sm, P2P_DISC_SEARCH, P2P_DISC_LISTEN_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscListenAction);
	StateMachineSetAction(Sm, P2P_DISC_SEARCH, P2P_DISC_CANL_CMD_EVT, (STATE_MACHINE_FUNC)P2PDiscSearchCanlAction);
	StateMachineSetAction(Sm, P2P_DISC_SEARCH, P2P_DISC_PEER_PROB_RSP, (STATE_MACHINE_FUNC)PeerP2pBeaconProbeRspAtScan);

	/* init Device Discovery Timer */
	P2PInitDevDiscTimer(pAd, 0);
	P2PInitNextScanTimer(pAd, 0);
	P2PInitListenTimer(pAd, 0);

	/* init all P2P ctrl state. */
	pAd->P2pCfg.CtrlCurrentState = P2P_DISC_IDLE;

	return;
}

/* Device Discovery Action */
static VOID P2PDiscScanAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_DISC_STATE *pCurrState = &(pAd->P2pCfg.DiscCurrentState);
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	MLME_SCAN_REQ_STRUCT ScanReq;
	BOOLEAN Cancelled, bSetAutoReconnect = FALSE;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::\n", __FUNCTION__));
	/* If I just finish group formation as GO. don't do scan . If I am auto GO, I should support P2P scan too. So check GoIntentIdx != 16. */
	if ((pP2PCtrl->P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) && (pP2PCtrl->GoIntentIdx <= 15))
	{
		pP2PCtrl->P2pCounter.Counter100ms = 0;
	}

	/* Stop Scan and resume */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		/*
			pAd->StaCfg.bAutoReconnect can be changed by IOCTL.
			If bAutoReconnect already is FALSE, we cannot enable bAutoReconnect in this function.
		*/
		if (pAd->StaCfg.bAutoReconnect)
		{
			pAd->StaCfg.bAutoReconnect = FALSE;
			bSetAutoReconnect = TRUE;
		}
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
		pAd->MlmeAux.Channel = 0;
		ScanNextChannel(pAd, OPMODE_STA);
	}

	/* Scan Type is SCAN_P2P for SYNC State Machine */
	ScanParmFill(pAd, &ScanReq, "", 0, BSS_ANY, SCAN_P2P);
	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, 
		sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);

	*pCurrState = P2P_DISC_SCAN;
	if (bSetAutoReconnect)
		pAd->StaCfg.bAutoReconnect = TRUE;
}	

static VOID P2PDiscScanCanlAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_DISC_STATE *pCurrState = &(pAd->P2pCfg.DiscCurrentState);
	/*PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;*/

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: The Device Discovery time has expired, stop!!\n", __FUNCTION__));
	/* *pCurrState = P2P_DISC_IDLE; */
	*pCurrState = P2P_DISC_LISTEN;
}

static VOID P2PDiscListenAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_DISC_STATE *pCurrState = &(pAd->P2pCfg.DiscCurrentState);
	/*PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;*/

	DBGPRINT(RT_DEBUG_INFO, ("%s::\n", __FUNCTION__));
	*pCurrState = P2P_DISC_LISTEN;
}

static VOID P2PDiscListenCanlAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_DISC_STATE *pCurrState = &(pAd->P2pCfg.DiscCurrentState);
	/*PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;*/

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: The Device Discovery time has expired, stop!!\n", __FUNCTION__));
	/* *pCurrState = P2P_DISC_IDLE; */
	*pCurrState = P2P_DISC_LISTEN;
}

static VOID P2PDiscSearchAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_DISC_STATE *pCurrState = &(pAd->P2pCfg.DiscCurrentState);
	MLME_SCAN_REQ_STRUCT ScanReq;

	DBGPRINT(RT_DEBUG_INFO, ("%s::\n", __FUNCTION__));
	ScanParmFill(pAd, &ScanReq, "", 0, BSS_ANY, SCAN_P2P_SEARCH);
	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, 
		sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);

	*pCurrState = P2P_DISC_SEARCH;
}	

static VOID P2PDiscSearchCanlAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	P2P_DISC_STATE *pCurrState = &(pAd->P2pCfg.DiscCurrentState);

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: The Device Discovery time has expired, stop!!\n", __FUNCTION__));
	/* *pCurrState = P2P_DISC_IDLE; */
	*pCurrState = P2P_DISC_LISTEN;
}
