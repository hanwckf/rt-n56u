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
	apcli_ctrl.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-06-23      modified for rt61-APClinent
*/
#ifdef APCLI_SUPPORT

#include "rt_config.h"


static VOID ApCliCtrlJoinReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlJoinReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlProbeRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuthRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuth2RspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuthReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuth2ReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlDeAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAssocReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlDisconnectReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlPeerDeAssocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlDeAssocAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlDeAuthAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliWpaMicFailureReportFrame(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

#ifdef APCLI_CERT_SUPPORT
static VOID ApCliCtrlScanDoneAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
#endif /* APCLI_CERT_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
static VOID ApCliCtrlTrialConnectAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliTrialConnectTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliTrialPhase2TimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliTrialConnectRetryTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);
static VOID ApCliTrialConnectTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

static VOID ApCliTrialConnectPhase2Timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

static VOID ApCliTrialConnectRetryTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(ApCliTrialConnectTimeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectTimeout);
DECLARE_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout);
DECLARE_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout);
#endif /* APCLI_CONNECTION_TRIAL */
/*
    ==========================================================================
    Description:
        The apcli ctrl state machine, 
    Parameters:
        Sm - pointer to the state machine
    Note:
        the state machine looks like the following
    ==========================================================================
 */
VOID ApCliCtrlStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR j;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
	PAPCLI_STRUCT	pApCliEntry;
#endif /* APCLI_CONNECTION_TRIAL */

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_CTRL_STATE, APCLI_MAX_CTRL_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_CTRL_DISCONNECTED,
		APCLI_CTRL_MACHINE_BASE);

	/* disconnected state */
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_JOIN_REQ, (STATE_MACHINE_FUNC)ApCliCtrlJoinReqAction);

	/* probe state */
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_PROBE_RSP, (STATE_MACHINE_FUNC)ApCliCtrlProbeRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_JOIN_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlJoinReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);

	/* auth state */
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_AUTH_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAuthRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_AUTH_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAuthReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
 	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);

	/* auth2 state */
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_AUTH_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAuth2RspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_AUTH_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAuth2ReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
 	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);

	/* assoc state */
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_ASSOC_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAssocRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_ASSOC_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAssocReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDeAssocAction);
 	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);

	/* deassoc state */
	StateMachineSetAction(Sm, APCLI_CTRL_DEASSOC, APCLI_CTRL_DEASSOC_RSP, (STATE_MACHINE_FUNC)ApCliCtrlDeAssocRspAction);

	/* connected state */
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDeAuthAction);
 	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_MT2_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliCtrlProbeRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_MIC_FAILURE_REPORT_FRAME, (STATE_MACHINE_FUNC)ApCliWpaMicFailureReportFrame);
#ifdef APCLI_CERT_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_SCAN_DONE, (STATE_MACHINE_FUNC)ApCliCtrlScanDoneAction);
#endif /* APCLI_CERT_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_TRIAL_CONNECT, (STATE_MACHINE_FUNC)ApCliCtrlTrialConnectAction);
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_TRIAL_CONNECT, (STATE_MACHINE_FUNC)ApCliCtrlTrialConnectAction);
	StateMachineSetAction(Sm, APCLI_CTRL_TRIAL_TRIGGERED, APCLI_CTRL_JOIN_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlTrialConnectAction);//for retry
	/* Trial Connect Timer Timeout handling */ 
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);

	/* Trial Phase2 Timer Timeout handling */ 
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);

	/* Trial Retry Timer Timeout handling */ 
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
#endif /* APCLI_CONNECTION_TRIAL */

	for (i = 0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].CtrlCurrState = APCLI_CTRL_DISCONNECTED;

#ifdef MAC_REPEATER_SUPPORT
		for (j = 0; j < MAX_EXT_MAC_ADDR_SIZE; j++)
		{
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].pAd = pAd;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchApCliIdx = i;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchLinkIdx = j;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].AuthCurrState = APCLI_CTRL_DISCONNECTED;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].CliConnectState = 0;
		}
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
		pApCliEntry = &pAd->ApCfg.ApCliTab[i];
		/* timer init */

		RTMPInitTimer(pAd,
			&pApCliEntry->TrialConnectTimer,
			GET_TIMER_FUNCTION(ApCliTrialConnectTimeout),
			(PVOID)pApCliEntry,
			FALSE);

		RTMPInitTimer(pAd,
			&pApCliEntry->TrialConnectPhase2Timer,
			GET_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout),
			(PVOID)pApCliEntry,
			FALSE);

		RTMPInitTimer(pAd,
			&pApCliEntry->TrialConnectRetryTimer,
			GET_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout),
			pApCliEntry,
			FALSE);
#endif /* APCLI_CONNECTION_TRIAL */
	}

	return;
}
#ifdef APCLI_CONNECTION_TRIAL
static VOID ApCliTrialConnectTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;

	UCHAR ifIndex = pApCliEntry->ifIndex;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, 0, NULL, ifIndex);

	RTMP_MLME_HANDLER(pAd);

	return;
}

static VOID ApCliTrialConnectPhase2Timeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;

	UCHAR ifIndex = pApCliEntry->ifIndex;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, 0, NULL, ifIndex);

	RTMP_MLME_HANDLER(pAd);

	return;

}

static VOID ApCliTrialConnectRetryTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;

	UCHAR ifIndex = pApCliEntry->ifIndex;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, 0, NULL, ifIndex);

	RTMP_MLME_HANDLER(pAd);

	return;
}

static VOID ApCliTrialConnectTimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PAPCLI_STRUCT	pApCliEntry;
	UCHAR	ifIndex;
	PULONG	pCurrState;


	ifIndex = pElem->Priv;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pCurrState = &pApCliEntry->CtrlCurrState;

	/* go back to original channel width */
	if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
		bbp_set_bw(pAd, pAd->hw_cfg.bbp_bw);
	AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, TRUE);
	AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);//jump back to origin channel, regenerate beacon.
	if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
	}

	if (*pCurrState == APCLI_CTRL_ASSOC) {
		//trialConnectTimeout, and currect status is ASSOC,
		//it means we got Auth Resp from new root AP already,
		//we shall serve the origin channel traffic first,
		//and jump back to trial channel to issue Assoc Req later, 
		//and finish four way-handshake if need.
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ApCliTrialConnectTimeout APCLI_CTRL_ASSOC set TrialConnectPhase2Timer\n", __func__));
		RTMPSetTimer(&(pApCliEntry->TrialConnectPhase2Timer),TRIAL_TIMEOUT);
	}
	else {
		//RTMPCancelTimer(&(pApCliEntry->ApCliMlmeAux.ProbeTimer), &Cancelled);
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt >= 10) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu, \n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh=0;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);//cleanup CfgSsid.
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;//cleanup retry count
			pApCliEntry->Enable = FALSE;
		}
		else
			*pCurrState = APCLI_CTRL_DISCONNECTED;//Disconnected State will bring the next probe req, auth req.
	}

	return;
}

static VOID ApCliTrialPhase2TimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PAPCLI_STRUCT pApCliEntry;
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	UCHAR ifIndex;
	struct wifi_dev *wdev;

	ifIndex = pElem->Priv;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s,\n \
			Jump back to trial channel:%d\n \
			to issue Assoc Req to new root AP\n",
			__func__, pApCliEntry->TrialCh));

	if (pApCliEntry->TrialCh != pAd->CommonCfg.CentralChannel) {
		/* Let BBP register at 20MHz */
		pAd->hw_cfg.bbp_bw = pAd->CommonCfg.BBPCurrentBW;
		if (pAd->CommonCfg.BBPCurrentBW != BW_20)
                	bbp_set_bw(pAd, BW_20);
		AsicDisableSync(pAd);//disable beacon
		AsicSwitchChannel(pAd, pApCliEntry->TrialCh, TRUE);
		//andes_switch_channel(pAd, pApCliEntry->TrialCh, 0, 0, 0x202, 0);//woody
	}
	ApCliLinkDown(pAd, ifIndex);
	if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
		RTMPSetTimer(&(pApCliEntry->TrialConnectRetryTimer),800);
	else
		RTMPSetTimer(&(pApCliEntry->TrialConnectRetryTimer), TRIAL_TIMEOUT);

	AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[pApCliEntry->ifIndex].MlmeAux.Bssid, pAd->ApCfg.ApCliTab[pApCliEntry->ifIndex].MlmeAux.CapabilityInfo,
				ASSOC_TIMEOUT, 5);

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
				sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, pApCliEntry->ifIndex);
	RTMP_MLME_HANDLER(pAd);

	return;
}

static VOID ApCliTrialConnectRetryTimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PAPCLI_STRUCT pApCliEntry;
	PULONG pCurrState;
	UCHAR ifIndex;

	PMAC_TABLE_ENTRY pMacEntry;
	STA_TR_ENTRY *tr_entry;
	//PMAC_TABLE_ENTRY pOldRootAp = &pApCliEntry->oldRootAP;

	ifIndex = pElem->Priv;
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pMacEntry = MacTableLookup(pAd,pApCliEntry->MlmeAux.Bssid);

	if (pMacEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCli_SYNC - %s, no CfgApCliBssid in mactable!\n", __func__));
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt >= 10) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu, \n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh=0;
			ApCliLinkDown(pAd, ifIndex);
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);//cleanup CfgSsid.
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;//cleanup retry count
			pApCliEntry->Enable = FALSE;
		}

		/* Restore original channel width */
		if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
			bbp_set_bw(pAd, pAd->hw_cfg.bbp_bw);
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, TRUE);
		AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);//jump back to origin channel, regenerate beacon.
		return;
	}

	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) && (*pCurrState == APCLI_CTRL_CONNECTED))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s, new rootAP connected!!\n", __func__));
		/* connected to new ap ok, change common channel to new channel */

		//AsicSetApCliBssid(pAd, pApCliEntry->ApCliMlmeAux.Bssid, 1);

		//MacTableDeleteEntry(pAd, pApCliEntry->MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s, jump back to origin channel to wait for User's operation!\n", __func__));
		if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
			bbp_set_bw(pAd, pAd->hw_cfg.bbp_bw);
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, TRUE);
		AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);//jump back to origin channel, regenerate beacon.
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);//cleanup CfgSsid.
		pApCliEntry->CfgSsidLen = 0;
		pApCliEntry->NewRootApRetryCnt = 0;//cleanup retry count
		pApCliEntry->Enable = FALSE;

	//	sprintf(tempBuf, "%d", pApCliEntry->TrialCh);
	//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Follow new rootAP Switch to channel :%s\n", tempBuf));
	//	Set_Channel_Proc(pAd, tempBuf);//APStartUp will regenerate beacon.
		pApCliEntry->TrialCh=0;
	}
	else 
	{
		/* 
		   Apcli does not connect to new root ap successfully yet,
		   jump back to origin channel to serve old rootap traffic.
		   re-issue assoc_req to go later.
		*/
		//pApCliEntry->MacTabWCID = pOldRootAp->Aid;
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt < 10)
			RTMPSetTimer(&(pApCliEntry->TrialConnectPhase2Timer), TRIAL_TIMEOUT);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu, \n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh=0;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);//cleanup CfgSsid.
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;//cleanup retry count
			pApCliEntry->Enable = FALSE;
			ApCliLinkDown(pAd, ifIndex);
		}

		if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
			bbp_set_bw(pAd, pAd->hw_cfg.bbp_bw);
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, TRUE);
		AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);//jump back to origin channel, regenerate beacon.
	}
	return;
}
#endif /* APCLI_CONNECTION_TRIAL */


/* 
    ==========================================================================
    Description:
        APCLI MLME JOIN req state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlJoinReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_MLME_JOIN_REQ_STRUCT JoinReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#ifdef WSC_AP_SUPPORT
	PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.ApCliTab[ifIndex].WscControl;
#endif /* WSC_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Start Probe Req.\n", __FUNCTION__));
	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (ApScanRunning(pAd) == TRUE)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
	{
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
	}

#ifdef WSC_AP_SUPPORT
    if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
		(pWpsCtrl->bWscTrigger == TRUE))
    {
    	ULONG bss_idx = 0;
        NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
        JoinReq.SsidLen = pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid, JoinReq.SsidLen);
		if (pWpsCtrl->WscMode == 1) /* PIN */
		{
			bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, (PUCHAR)(JoinReq.Ssid), JoinReq.SsidLen);
            if(pWpsCtrl->WscPINSsidIsSet)
                pWpsCtrl->WscPINBssCount = 0;
			if ((bss_idx == BSS_NOT_FOUND) || (pWpsCtrl->TryNextWpsPINAp == TRUE) )
			{
				if(pWpsCtrl->WscPINBssCount > 0)
				{
					pWpsCtrl->WscPINBssCount--;
                    NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid, MAX_LEN_OF_SSID);
					pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.SsidLength = pWpsCtrl->ApUuidBssid[pWpsCtrl->WscPINBssCount].SsidLen;
					NdisMoveMemory(pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid,pWpsCtrl->ApUuidBssid[pWpsCtrl->WscPINBssCount].Ssid, pWpsCtrl->ApUuidBssid[pWpsCtrl->WscPINBssCount].SsidLen);
					pWpsCtrl->TryNextWpsPINAp = FALSE;
                    pWpsCtrl->WscState = WSC_STATE_START;
                }
				else
				{
                        
                    if((pWpsCtrl->WscStatus != STATUS_WSC_PIN_NO_AP) && (pWpsCtrl->WscStatus != STATUS_WSC_SCAN_AP)) 
                    {       
						pAd->ScanTab.BssNr = 0;
                        pWpsCtrl->WscState = WSC_STATE_START;
                        pWpsCtrl->WscStatus = STATUS_WSC_PIN_NO_AP;
#ifdef APCLI_SUPPORT
                        RTMPSetTimer(&pWpsCtrl->WscPINTimer, 1000);
	          			pWpsCtrl->WscPINTimerRunning = TRUE;
#endif /* APCLI_SUPPORT */
                    } 
				    if(pWpsCtrl->WscPINSsidIsSet)
                        pWpsCtrl->TryNextWpsPINAp = FALSE;    
				}
				return;
			}
			else if(JoinReq.SsidLen)
			{
				INT old_conf_mode = pWpsCtrl->WscConfMode;
				ADD_HTINFO	RootApHtInfo, ApHtInfo;
				UCHAR channel = pAd->CommonCfg.Channel;
			        UCHAR	RootApChannel = pAd->ScanTab.BssEntry[bss_idx].Channel;
				UCHAR RootApCentralChannel = pAd->ScanTab.BssEntry[bss_idx].CentralChannel;
				ApHtInfo = pAd->CommonCfg.AddHTInfo.AddHtInfo;
				RootApHtInfo = pAd->ScanTab.BssEntry[bss_idx].AddHtInfo.AddHtInfo;

                
				if (((RootApCentralChannel != RootApChannel) &&
					 (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
					 (ApHtInfo.ExtChanOffset != RootApHtInfo.ExtChanOffset)) || (RootApChannel != channel))
				{
					RTMP_STRING ChStr[5] = {0};
					if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
					{
						if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE)
							Set_HtExtcha_Proc(pAd, "1");
						else
							Set_HtExtcha_Proc(pAd, "0");
					}
					snprintf(ChStr, sizeof(ChStr), "%d", pAd->ScanTab.BssEntry[bss_idx].Channel);
					Set_Channel_Proc(pAd, ChStr);
					/*
						ApStop will call WscStop, we need to reset WscConfMode, WscMode & bWscTrigger here.
					*/
		   			pWpsCtrl->WscMode = 1;
					pWpsCtrl->WscConfMode = old_conf_mode;
					pWpsCtrl->bWscTrigger = TRUE;
                    return;
                }
					pWpsCtrl->WscState = WSC_STATE_START;
					pWpsCtrl->WscStatus = STATUS_WSC_START_ASSOC;

                 NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid , MAX_LEN_OF_SSID);    
                 pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.SsidLength;
                 NdisMoveMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid, pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen);
                pWpsCtrl->TryNextWpsPINAp = TRUE; // if WPS PIN connection fails with the current AP then try next AP.
			}
            else 
                pWpsCtrl->TryNextWpsPINAp = TRUE;
		}
    }
    else
#endif /* WSC_AP_SUPPORT */
	if (pApCliEntry->CfgSsidLen != 0)
	{
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) 
		ULONG bss_idx = BSS_NOT_FOUND;
		bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, (PCHAR)pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);
		if (bss_idx == BSS_NOT_FOUND)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::  can't find SSID[%s] in ScanTab.\n", __FUNCTION__, pApCliEntry->CfgSsid));
			*pCurrState = APCLI_CTRL_PROBE;
                                     
			CFG80211_checkScanTable(pAd);
            RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, JoinReq.Bssid, NULL, 0, NULL, 0, 0);

			return;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::  find SSID[%ld][%s] channel[%d-%d] in ScanTab.\n", __FUNCTION__, 
						bss_idx, pApCliEntry->CfgSsid, pAd->ScanTab.BssEntry[bss_idx].Channel, 
						pAd->ScanTab.BssEntry[bss_idx].CentralChannel));

//TODO	
		/* BssSearch Table has found the pEntry, send Prob Req. directly */
//		if (pAd->CommonCfg.Channel != pAd->ScanTab.BssEntry[bss_idx].Channel)
		{
			pApCliEntry->MlmeAux.Channel = pAd->ScanTab.BssEntry[bss_idx].Channel;
				pApCliEntry->wdev.CentralChannel = pApCliEntry->MlmeAux.Channel ;
				//should be check and update in in asso to check ==> ApCliCheckHt()
				pApCliEntry->wdev.channel = pApCliEntry->wdev.CentralChannel;
				pApCliEntry->wdev.bw = HT_BW_20;
				pApCliEntry->wdev.extcha = EXTCHA_NONE;				
			AsicSwitchChannel(pAd, pApCliEntry->MlmeAux.Channel, FALSE);
			AsicLockChannel(pAd, pApCliEntry->MlmeAux.Channel);

		}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

		JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
		NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
	}

	if (JoinReq.SsidLen <= MAX_LEN_OF_SSID)
	{
		char SSID[MAX_LEN_OF_SSID + 1] = {0};

		snprintf(SSID, JoinReq.SsidLen + 1, "%s", JoinReq.Ssid);
		printk(KERN_INFO "AP-Client probe: SSID=%s, BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
			SSID, PRINT_MAC(JoinReq.Bssid));
	}

	*pCurrState = APCLI_CTRL_PROBE;

	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
		sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME JOIN req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlJoinReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_MLME_JOIN_REQ_STRUCT JoinReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Probe Req Timeout.\n", __FUNCTION__));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (ApScanRunning(pAd) == TRUE)
	{
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		return;
	}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pApCliEntry->ProbeReqCnt++;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Probe Req Timeout. ProbeReqCnt=%d\n",
				__FUNCTION__, pApCliEntry->ProbeReqCnt));

	if (pApCliEntry->ProbeReqCnt > APCLI_MAX_PROBE_RETRY_NUM)
	{
		/*
			if exceed the APCLI_MAX_PROBE_RETRY_NUM,
			switch to try next candidate AP.
		*/
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pApCliEntry->ProbeReqCnt = 0;
		
		if (pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
			ApCliSwitchCandidateAP(pAd, ifIndex);
		
		if ((pAd->ApCfg.ApCliAutoConnectRunning == FALSE) && (pApCliEntry->AutoConnectFlag == TRUE))
			ApCliAutoConnectStart(pAd, ifIndex);
		
		return;
	}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_PROBE;

	/* retry Probe Req. */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Retry Probe Req.\n", __FUNCTION__));

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
	{
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
	}

#ifdef WSC_AP_SUPPORT
    if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
    {
        NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
        JoinReq.SsidLen = pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid, JoinReq.SsidLen);
    }
    else
#endif /* WSC_AP_SUPPORT */
	if (pApCliEntry->CfgSsidLen != 0)
	{
		JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
		NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
	}

	if (JoinReq.SsidLen <= MAX_LEN_OF_SSID)
	{
		char SSID[MAX_LEN_OF_SSID + 1] = {0};

		snprintf(SSID, JoinReq.SsidLen + 1, "%s", JoinReq.Ssid);
		printk(KERN_INFO "AP-Client probe: SSID=%s, BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
			SSID, PRINT_MAC(JoinReq.Bssid));
	}

	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
		sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME Probe Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlProbeRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	PAPCLI_STRUCT pApCliEntry;
	MLME_AUTH_REQ_STRUCT AuthReq;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
	}

	if (Info->CliIdx != 0xFF)
		CliIdx = Info->CliIdx;
#endif /* MAC_REPEATER_SUPPORT */

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		pCurrState = &pApCliEntry->RepeaterCli[CliIdx].CtrlCurrState;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pApCliEntry->CtrlCurrState;

	if (Status == MLME_SUCCESS)
	{
		if (pApCliEntry->SsidLen <= MAX_LEN_OF_SSID)
		{
			char SSID[MAX_LEN_OF_SSID + 1] = {0};

			snprintf(SSID, pApCliEntry->SsidLen + 1, "%s", pApCliEntry->Ssid);
			printk(KERN_INFO "AP-Client probe response: SSID=%s, BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
				SSID, PRINT_MAC(pApCliEntry->MlmeAux.Bssid));
		}

#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.Channel < 14)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
		{
			ADD_HTINFO RootApHtInfo = pApCliEntry->MlmeAux.AddHtInfo.AddHtInfo;

			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40 &&
			    RootApHtInfo.ExtChanOffset != pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset &&
			    RootApHtInfo.RecomWidth)
			{
				if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE) {
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
					pApCliEntry->MlmeAux.CentralChannel = pApCliEntry->MlmeAux.Channel + 2;
				} else {
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_BELOW;
					pApCliEntry->MlmeAux.CentralChannel = pApCliEntry->MlmeAux.Channel - 2;
				}

				pAd->CommonCfg.Channel = pApCliEntry->MlmeAux.Channel;
				pAd->CommonCfg.CentralChannel = pApCliEntry->MlmeAux.CentralChannel;
#ifdef DOT11N_DRAFT3
				pAd->CommonCfg.Bss2040NeedFallBack = 0;
				pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 0;
#endif /* DOT11N_DRAFT3 */
				SetCommonHT(pAd);
				
				pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;
				AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, FALSE);
				AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ch(%d), cent_ch(%d), extcha(%d), bbp bw(%d)\n",
							__FUNCTION__,
							pAd->CommonCfg.Channel,
							pAd->CommonCfg.CentralChannel,
							pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
							pAd->CommonCfg.BBPCurrentBW));
#ifdef DOT11N_DRAFT3
				pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
#endif /* DOT11N_DRAFT3 */
			}
		}
#endif /* DOT11_N_SUPPORT */

		*pCurrState = APCLI_CTRL_AUTH;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->AuthReqCnt = 0;

		COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);

		/* start Authentication Req. */
		/* If AuthMode is Auto, try shared key first */
		if ((wdev->AuthMode == Ndis802_11AuthModeShared) ||
			(wdev->AuthMode == Ndis802_11AuthModeAutoSwitch))
			AuthReq.Alg = Ndis802_11AuthModeShared;
		else
			AuthReq.Alg = Ndis802_11AuthModeOpen;

		AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
		AuthReq.BssIdx = ifIndex;
		AuthReq.CliIdx = CliIdx;
		if (CliIdx != 0xFF)
		{
			ifIndex = (64 + 16*ifIndex + CliIdx);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Repeater Cli Trigger Auth Req ifIndex = %d, CliIdx = %d !!!\n",
							__FUNCTION__, ifIndex, CliIdx));
		}
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
			sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Probe respond fail.\n", __FUNCTION__));
		*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd, ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME AUTH Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuthRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	MLME_AUTH_REQ_STRUCT AuthReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
	}
#endif /* MAC_REPEATER_SUPPORT */

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Repeater Cli Receive ifIndex = %d, CliIdx = %d !!!\n",
						__FUNCTION__, ifIndex, CliIdx));

		pCurrState = &pApCliEntry->RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pApCliEntry->CtrlCurrState;

	if(Status == MLME_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Auth Rsp Success.\n", __FUNCTION__));
		*pCurrState = APCLI_CTRL_ASSOC;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pApCliEntry->RepeaterCli[CliIdx].AssocReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->AssocReqCnt = 0;

#ifdef APCLI_CONNECTION_TRIAL
		//if connection trying, wait until trialtimeout and enqueue Assoc REQ then.
		//TrialCh == 0 means trial has not been triggered.
		if (pApCliEntry->TrialCh == 0) {
#endif /* APCLI_CONNECTION_TRIAL */
		AssocParmFill(pAd, &AssocReq, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.CapabilityInfo,
			ASSOC_TIMEOUT, 5);
#ifdef APCLI_CONNECTION_TRIAL
		}
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
		//if connection trying, wait until trialtimeout and enqueue Assoc REQ then.
		//TrialCh == 0 means trial has not been triggered.
		if (pApCliEntry->TrialCh == 0) {
#endif /* APCLI_CONNECTION_TRIAL */
		MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
			sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);
#ifdef APCLI_CONNECTION_TRIAL
		}
#endif /* APCLI_CONNECTION_TRIAL */
	} 
	else
	{
		if (pApCliEntry->wdev.AuthMode == Ndis802_11AuthModeAutoSwitch)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Auth Rsp Failure.\n", __FUNCTION__));

			*pCurrState = APCLI_CTRL_AUTH_2;

			/* start Second Authentication Req. */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Start Second Auth Rep.\n", __FUNCTION__));
			COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);
			AuthReq.Alg = Ndis802_11AuthModeOpen;
			AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
			AuthReq.BssIdx = ifIndex;
			AuthReq.CliIdx = CliIdx;
			ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
			sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
		}
		else
		{
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt = 0;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
				NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
				pApCliEntry->AuthReqCnt = 0;
			}
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				)
				ApCliSwitchCandidateAP(pAd, ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		}
	}

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME AUTH2 Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuth2RspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
	}
#endif /* MAC_REPEATER_SUPPORT */

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		pCurrState = &pApCliEntry->RepeaterCli[CliIdx].CtrlCurrState;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pApCliEntry->CtrlCurrState;
	if(Status == MLME_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Auth2 Rsp Success.\n", __FUNCTION__));
		*pCurrState = APCLI_CTRL_ASSOC;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pApCliEntry->RepeaterCli[CliIdx].AssocReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->AssocReqCnt = 0;

		AssocParmFill(pAd, &AssocReq, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.CapabilityInfo,
			ASSOC_TIMEOUT, 5);

#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ, 
			sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);
	} 
	else
	{
		printk(KERN_WARNING "AP-Client: authentication failed!\n");

		*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd, ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME Auth Req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuthReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_AUTH_REQ_STRUCT AuthReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Auth Req Timeout.\n", __FUNCTION__));

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt++;

		if (pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt = 0;
			return;
		}
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->AuthReqCnt++;

		if (pApCliEntry->AuthReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
			NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pApCliEntry->AuthReqCnt = 0;
		
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd, ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			return;
	}
	}

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_AUTH;

	/* retry Authentication. */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Retry Auth Req.\n", __FUNCTION__));
	COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);
	AuthReq.Alg = pApCliEntry->MlmeAux.Alg; /*Ndis802_11AuthModeOpen; */
	AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
	AuthReq.BssIdx = ifIndex;
	AuthReq.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
		sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME Auth2 Req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuth2ReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME ASSOC RSP state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Repeater Cli Receive Assoc Rsp ifIndex = %d, CliIdx = %d.\n",
					__FUNCTION__, ifIndex, CliIdx));
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if(Status == MLME_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) apCliIf = %d, Receive Assoc Rsp Success.\n", __FUNCTION__, ifIndex));

#ifdef WPA_SUPPLICANT_SUPPORT
			    if (pAd->ApCfg.ApCliTab[ifIndex].wpa_supplicant_info.WpaSupplicantUP)
			    {
			        ApcliSendAssocIEsToWpaSupplicant(pAd,ifIndex);
				RtmpOSWrielessEventSend(pAd->net_dev,
								RT_WLAN_EVENT_CUSTOM,
								RT_ASSOC_EVENT_FLAG,
								NULL, NULL, 0);
			    }
#endif /* WPA_SUPPLICANT_SUPPORT */                    

#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		if (ApCliLinkUp(pAd, ifIndex))
		{
			*pCurrState = APCLI_CTRL_CONNECTED;

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
			CFG80211_checkScanTable(pAd);
			RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pApCliEntry->MlmeAux.Bssid,
				pApCliEntry->ReqVarIEs, pApCliEntry->ReqVarIELen,	
				pApCliEntry->ResVarIEs, pApCliEntry->ResVarIELen, TRUE);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
			
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) apCliIf = %d, Insert Remote AP to MacTable failed.\n", __FUNCTION__,  ifIndex));
			/* Reset the apcli interface as disconnected and Invalid. */
			*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				ifIndex = ((ifIndex - 64) / 16);
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->Valid = FALSE;
			
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				)
				ApCliSwitchCandidateAP(pAd, ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
			CFG80211_checkScanTable(pAd);
			RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pApCliEntry->MlmeAux.Bssid,
				NULL, 0, NULL, 0, 0);			
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) apCliIf = %d, Receive Assoc Rsp Failure.\n", __FUNCTION__,  ifIndex));

		*pCurrState = APCLI_CTRL_DISCONNECTED;

		/* set the apcli interface be valid. */
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
		{
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->Valid = FALSE;
		
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd, ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		CFG80211_checkScanTable(pAd);					
		RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pApCliEntry->MlmeAux.Bssid,
			NULL, 0, NULL, 0, 0);	
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

	}

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME DeASSOC RSP state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlDeAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if (Status == MLME_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Receive DeAssoc Rsp Success.\n", __FUNCTION__));
	} else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Receive DeAssoc Rsp Failure.\n", __FUNCTION__));
	}

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	if (bValid)
		ApCliLinkDown(pAd, ifIndex);
	
	*pCurrState = APCLI_CTRL_DISCONNECTED;

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME Assoc Req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAssocReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Assoc Req Timeout.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* give up to retry authentication req after retry it 5 times. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx !=0xFF)
	{
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt++;
		if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt = 0;
			return;
		}
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->AssocReqCnt++;
		if (pApCliEntry->AssocReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pApCliEntry->AssocReqCnt = 0;
		
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				)
				ApCliSwitchCandidateAP(pAd, ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			return;
		}
	}

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_ASSOC;

	/* retry Association Req. */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Retry Association Req.\n", __FUNCTION__));
	AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.CapabilityInfo,
		ASSOC_TIMEOUT, 5);

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ, 
		sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME Disconnect Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlDisconnectReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) MLME Request disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) 2. Before do ApCliLinkDown.\n", __FUNCTION__));
	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);			
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
	pApCliEntry->Valid = FALSE;

	/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	*pCurrState = APCLI_CTRL_DISCONNECTED;

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME Peer DeAssoc Req state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlPeerDeAssocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
		UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	printk(KERN_INFO "AP-Client: disconnected by peer\n");

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
	{
		UCHAR index;
		BOOLEAN Cancelled;

		for(index = 0; index < MAX_EXT_MAC_ADDR_SIZE; index++)
		{
			if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].CliEnable)
			{
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].ApCliAuthTimer, &Cancelled);
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].ApCliAssocTimer, &Cancelled);
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].CliValid)
					ApCliLinkDown(pAd, (64 + MAX_EXT_MAC_ADDR_SIZE*ifIndex + index));
				RTMPRemoveRepeaterEntry(pAd, ifIndex, index);
			}
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
	bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		)
	{
		STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];
		if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			ApCliSwitchCandidateAP(pAd, ifIndex);
	}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */	

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);		
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
	pApCliEntry->Valid = FALSE;

	/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	*pCurrState = APCLI_CTRL_DISCONNECTED;

	return;
}

/* 
    ==========================================================================
    Description:
        APCLI MLME Disconnect Req state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlDeAssocAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	MLME_DISASSOC_REQ_STRUCT DisassocReq;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) MLME Request Disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	DisassocParmFill(pAd, &DisassocReq, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_DISASSOC_REQ,
		sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, ifIndex);

	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);			
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
	pApCliEntry->Valid = FALSE;

	/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	*pCurrState = APCLI_CTRL_DEASSOC;

	return;
}


/* 
    ==========================================================================
    Description:
        APCLI MLME Disconnect Req state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlDeAuthAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	MLME_DEAUTH_REQ_STRUCT	DeAuthFrame;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) MLME Request Disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
		ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* Fill in the related information */
	DeAuthFrame.Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
	COPY_MAC_ADDR(DeAuthFrame.Addr, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	
	MlmeEnqueue(pAd, 
				  APCLI_AUTH_STATE_MACHINE, 
				  APCLI_MT2_MLME_DEAUTH_REQ, 
				  sizeof(MLME_DEAUTH_REQ_STRUCT),
				  &DeAuthFrame, 
				  ifIndex);

	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		//RTMPDelRepeaterCliAsicEntry(pAd, CliIdx);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);		
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;

		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	*pCurrState = APCLI_CTRL_DISCONNECTED;

	return;
}

VOID ApCliWpaMicFailureReportFrame(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PUCHAR              pOutBuffer = NULL;
	UCHAR               Header802_3[14];
	ULONG               FrameLen = 0;
	UCHAR		*mpool;
	PEAPOL_PACKET       pPacket;
	UCHAR               Mic[16];
	BOOLEAN             bUnicast;
	UCHAR			Wcid;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	USHORT ifIndex = (USHORT)(Elem->Priv);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliWpaMicFailureReportFrame ----->\n"));

	if (ifIndex >= MAX_APCLI_NUM)
		return;
										
	bUnicast = (Elem->Msg[0] == 1 ? TRUE:FALSE);
	pAd->Sequence = ((pAd->Sequence) + 1) & (MAX_SEQ_NUMBER);
	/* init 802.3 header and Fill Packet */
	pMacEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[ifIndex].MacTabWCID];
	if (!IS_ENTRY_APCLI(pMacEntry))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : !IS_ENTRY_APCLI(pMacEntry)\n", __FUNCTION__));
		return;
	}
	Wcid =  pAd->ApCfg.ApCliTab[ifIndex].MacTabWCID;
	MAKE_802_3_HEADER(Header802_3, pAd->MacTab.Content[Wcid].Addr, pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr, EAPOL);
					
	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
		return;
	}

	pPacket = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pPacket, TX_EAPOL_BUFFER);
								
	pPacket->ProVer	= EAPOL_VER;
	pPacket->ProType	= EAPOLKey;
											
	pPacket->KeyDesc.Type = WPA1_KEY_DESC;

    	/* Request field presented */
	pPacket->KeyDesc.KeyInfo.Request = 1;
			        
    	if(pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus  == Ndis802_11Encryption3Enabled)
	{
		pPacket->KeyDesc.KeyInfo.KeyDescVer = 2;
	} 
	else	  /* TKIP */
	{
		pPacket->KeyDesc.KeyInfo.KeyDescVer = 1;
	}

	pPacket->KeyDesc.KeyInfo.KeyType = (bUnicast ? PAIRWISEKEY : GROUPKEY);

    	/* KeyMic field presented */
    	pPacket->KeyDesc.KeyInfo.KeyMic  = 1;

	/* Error field presented */
	pPacket->KeyDesc.KeyInfo.Error  = 1;
										    
	/* Update packet length after decide Key data payload */
	SET_UINT16_TO_ARRARY(pPacket->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG)

	/* Key Replay Count */
	NdisMoveMemory(pPacket->KeyDesc.ReplayCounter, pAd->ApCfg.ApCliTab[ifIndex].ReplayCounter, LEN_KEY_DESC_REPLAY);
   	inc_byte_array(pAd->ApCfg.ApCliTab[ifIndex].ReplayCounter, 8);

	/* Convert to little-endian format. */
	*((USHORT *)&pPacket->KeyDesc.KeyInfo) = cpu2le16(*((USHORT *)&pPacket->KeyDesc.KeyInfo));


	MlmeAllocateMemory(pAd, (PUCHAR *)&pOutBuffer);  /* allocate memory */
	if(pOutBuffer == NULL)
	{
		os_free_mem(NULL, mpool);
		return;
	}
											    
	/*
   	   Prepare EAPOL frame for MIC calculation
   	   Be careful, only EAPOL frame is counted for MIC calculation
   	*/
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
        	CONV_ARRARY_TO_UINT16(pPacket->Body_Len) + 4,   pPacket,
		END_OF_ARGS);

	/* Prepare and Fill MIC value */
	NdisZeroMemory(Mic, sizeof(Mic));
	if(pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus  == Ndis802_11Encryption3Enabled)
	{	/* AES */
       		UCHAR digest[20] = {0};
		RT_HMAC_SHA1(pAd->ApCfg.ApCliTab[ifIndex].PTK, LEN_PTK_KCK, pOutBuffer, FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
	} 
	else
	{	/* TKIP */
		RT_HMAC_MD5(pAd->ApCfg.ApCliTab[ifIndex].PTK, LEN_PTK_KCK, pOutBuffer, FrameLen, Mic, MD5_DIGEST_SIZE);
	}
	NdisMoveMemory(pPacket->KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);

	/* copy frame to Tx ring and send MIC failure report frame to authenticator */
	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[Wcid],
		  	Header802_3, LENGTH_802_3, 
			(PUCHAR)pPacket, 
			CONV_ARRARY_TO_UINT16(pPacket->Body_Len) + 4, FALSE);

	MlmeFreeMemory(pAd, (PUCHAR)pOutBuffer);

	os_free_mem(NULL, mpool);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliWpaMicFailureReportFrame <-----\n"));
}

#ifdef APCLI_CERT_SUPPORT
static VOID ApCliCtrlScanDoneAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
		
#ifdef DOT11N_DRAFT3
	USHORT ifIndex = (USHORT)(Elem->Priv);
	UCHAR i;
	/* AP sent a 2040Coexistence mgmt frame, then station perform a scan, and then send back the respone. */
	if ((pAd->CommonCfg.BSSCoexist2040.field.InfoReq == 1)
    	    	&& OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) {
    		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Update2040CoexistFrameAndNotify @%s  \n", __FUNCTION__));    
		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (IS_ENTRY_APCLI(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].func_tb_idx == ifIndex))
			{
				Update2040CoexistFrameAndNotify(pAd, i, TRUE);
			}
		}			
	}
#endif /* DOT11N_DRAFT3 */
}

#endif /* APCLI_CERT_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
/* 
    ==========================================================================
    Description:
        APCLI trigger JOIN req state machine procedure 
	for connect the another rootAP
    ==========================================================================
 */
static VOID ApCliCtrlTrialConnectAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_MLME_JOIN_REQ_STRUCT JoinReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	BOOLEAN	Cancelled;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Start Probe Req Trial.\n", __func__));
	if (ifIndex >= MAX_APCLI_NUM || ifIndex == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Index: %d error!!\n", __func__, ifIndex));
		return;
	}

	if (ApScanRunning(pAd) == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Ap Scanning.....\n", __func__));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if (!(pApCliEntry->TrialCh)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Didn't assign the RootAP channel\n", __func__));
		return;
	}
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) channel TrialConnectTimer\n", __func__));
		RTMPCancelTimer(&(pApCliEntry->TrialConnectTimer), &Cancelled);

		if (pApCliEntry->TrialCh != pAd->CommonCfg.CentralChannel) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Jump to CH:%d\n", __func__, pApCliEntry->TrialCh));
			/* Always use 20Mhz channel width for trial connection */
			pAd->hw_cfg.bbp_bw = pAd->CommonCfg.BBPCurrentBW;
			if (pAd->CommonCfg.BBPCurrentBW != BW_20)
				bbp_set_bw(pAd,BW_20);

			AsicDisableSync(pAd);//disable beacon
			AsicSwitchChannel(pAd, pApCliEntry->TrialCh, TRUE);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) set  TrialConnectTimer(%d ms)\n", __func__,TRIAL_TIMEOUT));
			RTMPSetTimer(&(pApCliEntry->TrialConnectTimer), TRIAL_TIMEOUT);
		}
	}

	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
	{
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
	}

	if (pApCliEntry->CfgSsidLen != 0)
	{
		JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
		NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, JoinReq.Ssid, JoinReq.Bssid[0], JoinReq.Bssid[1], JoinReq.Bssid[2],
		JoinReq.Bssid[3], JoinReq.Bssid[4], JoinReq.Bssid[5]));

	*pCurrState = APCLI_CTRL_PROBE;

	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
		sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);

	return;
}
#endif /* APCLI_CONNECTION_TRIAL */
#endif /* APCLI_SUPPORT */
