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

#ifdef MAC_REPEATER_SUPPORT
static VOID ReptCliResetEntryTimeoutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);
#endif /* MAC_REPEATER_SUPPORT */

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

#ifdef MAC_REPEATER_SUPPORT
DECLARE_TIMER_FUNCTION(ReptCliResetEntryTimeoutAction);
BUILD_TIMER_FUNCTION(ReptCliResetEntryTimeoutAction);
#endif /* MAC_REPEATER_SUPPORT */

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
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

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
	for (i = 0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].CtrlCurrState = APCLI_CTRL_DISCONNECTED;

#ifdef MAC_REPEATER_SUPPORT
		for (j = 0; j < MAX_EXT_MAC_ADDR_SIZE; j++)
		{
			pReptCliEntry = &pAd->ApCfg.ApCliTab[i].RepeaterCli[j];

			pReptCliEntry->pAd = pAd;
			pReptCliEntry->MatchApCliIdx = i;
			pReptCliEntry->MatchLinkIdx = j;
			pReptCliEntry->CtrlCurrState = APCLI_CTRL_DISCONNECTED;
			pReptCliEntry->CliConnectState = 0;

			/* timer init */
			RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].RepeaterCli[j].ReptCliResetEntryTimer,
							GET_TIMER_FUNCTION(ReptCliResetEntryTimeoutAction), &pAd->ApCfg.ApCliTab[i].RepeaterCli[j], FALSE);
		}
#endif /* MAC_REPEATER_SUPPORT */
	}

	return;
}

#ifdef MAC_REPEATER_SUPPORT
/*
    ==========================================================================
    Description:
        Association timeout procedure. After association timeout, this function 
        will be called and it will put a message into the MLME queue
    Parameters:
        Standard timer parameters
    ==========================================================================
 */
static VOID ReptCliResetEntryTimeoutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	REPEATER_CLIENT_ENTRY *pPrevEntry, *pProbeEntry;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry, *pPrevMapEntry, *pProbeMapEntry;
	USHORT HashIdx;
	PRTMP_ADAPTER pAd = NULL;

	DBGPRINT(RT_DEBUG_ERROR, (" (%s) ifIndex = %d, CliIdx = %d  Reset The Entry!!!\n",
					__FUNCTION__, pEntry->MatchApCliIdx, pEntry->MatchLinkIdx));

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	pAd = pEntry->pAd;
	RTMPRemoveRepeaterAsicEntry(pAd, pEntry->MatchLinkIdx);

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->CurrentAddress);

	pPrevEntry = NULL;
	pProbeEntry = pAd->ApCfg.ReptCliHash[HashIdx];
	ASSERT(pProbeEntry);

	if (pProbeEntry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (pProbeEntry == pEntry)
			{
				if (pPrevEntry == NULL)
				{
					pAd->ApCfg.ReptCliHash[HashIdx] = pEntry->pNext;
				}
				else
				{
					pPrevEntry->pNext = pEntry->pNext;
				}
				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeEntry != NULL);

	pMapEntry = &pAd->ApCfg.ApCliTab[pEntry->MatchApCliIdx].RepeaterCliMap[pEntry->MatchLinkIdx];

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->OriginalAddress);

	pPrevMapEntry = NULL;
	pProbeMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];
	ASSERT(pProbeMapEntry);
	if (pProbeMapEntry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (pProbeMapEntry == pMapEntry)
			{
				if (pPrevMapEntry == NULL)
				{
					pAd->ApCfg.ReptMapHash[HashIdx] = pMapEntry->pNext;
				}
				else
				{
					pPrevMapEntry->pNext = pMapEntry->pNext;
				}
				break;
			}

			pPrevMapEntry = pProbeMapEntry;
			pProbeMapEntry = pProbeMapEntry->pNext;
		} while (pProbeMapEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeMapEntry != NULL);

	NdisZeroMemory(pEntry->OriginalAddress, MAC_ADDR_LEN);
	NdisZeroMemory(pEntry->CurrentAddress, MAC_ADDR_LEN);
	pEntry->CtrlCurrState = APCLI_CTRL_DISCONNECTED;
	pEntry->AuthCurrState = APCLI_AUTH_REQ_IDLE;
	pEntry->AssocCurrState = APCLI_ASSOC_IDLE;
	pEntry->CliConnectState = 0;
	pEntry->CliValid = FALSE;
	pEntry->CliEnable = FALSE;
	pEntry->bEthCli = FALSE;
	pEntry->MacTabWCID = 0xFF;
	pEntry->AuthReqCnt = 0;
	pEntry->AssocReqCnt = 0;
	pEntry->CliTriggerTime = 0;

	if (pAd->ApCfg.RepeaterCliSize > 0)
		pAd->ApCfg.RepeaterCliSize--;

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
}
#endif /* MAC_REPEATER_SUPPORT */

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

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Start Probe Req.\n", __FUNCTION__));
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
			if (bss_idx == BSS_NOT_FOUND)
			{
				ApSiteSurvey(pAd, NULL, SCAN_WSC_ACTIVE, FALSE);
				return;
			}
			else
			{
				INT old_conf_mode = pWpsCtrl->WscConfMode;
				ADD_HTINFO	RootApHtInfo, ApHtInfo;
				UCHAR channel = pAd->CommonCfg.Channel, RootApChannel = pAd->ScanTab.BssEntry[bss_idx].Channel;
				UCHAR RootApCentralChannel = pAd->ScanTab.BssEntry[bss_idx].CentralChannel;
				ApHtInfo = pAd->CommonCfg.AddHTInfo.AddHtInfo;
				RootApHtInfo = pAd->ScanTab.BssEntry[bss_idx].AddHtInfo.AddHtInfo;
				
				if ((RootApChannel != channel) ||
					((RootApCentralChannel != RootApChannel) &&
					 (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
					 (ApHtInfo.ExtChanOffset != RootApHtInfo.ExtChanOffset)))
				{
					STRING	ChStr[5] = {0};
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

					pWpsCtrl->WscState = WSC_STATE_START;
					pWpsCtrl->WscStatus = STATUS_WSC_START_ASSOC;
					pWpsCtrl->WscMode = 1;
					pWpsCtrl->WscConfMode = old_conf_mode;
					pWpsCtrl->bWscTrigger = TRUE;
					return;
				}				
			}
		}
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


	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe Req Timeout.\n", __FUNCTION__));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (ApScanRunning(pAd) == TRUE)
	{
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pApCliEntry->ProbeReqCnt++;
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe Req Timeout. ProbeReqCnt=%d\n",
				__FUNCTION__, pApCliEntry->ProbeReqCnt));

	if (pApCliEntry->ProbeReqCnt > APCLI_MAX_PROBE_RETRY_NUM)
	{
		/*
			if exceed the APCLI_MAX_PROBE_RETRY_NUM,
			switch to try next candidate AP.
		*/
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		NdisZeroMemory(pApCliEntry->ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pApCliEntry->ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
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
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Retry Probe Req.\n", __FUNCTION__));

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
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
	}

	if (Info->CliIdx != 0xFF)
		CliIdx = Info->CliIdx;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if (Status == MLME_SUCCESS)
	{
		if (pApCliEntry->SsidLen <= MAX_LEN_OF_SSID)
		{
			char SSID[MAX_LEN_OF_SSID + 1] = {0};

			snprintf(SSID, pApCliEntry->SsidLen + 1, "%s", pApCliEntry->Ssid);
			printk(KERN_INFO "AP-Client probe response: SSID=%s, BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
				SSID, PRINT_MAC(pApCliEntry->ApCliMlmeAux.Bssid));
		}

#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.Channel < 14)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
		{
			ADD_HTINFO RootApHtInfo = pApCliEntry->ApCliMlmeAux.AddHtInfo.AddHtInfo;

			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40 &&
			    RootApHtInfo.ExtChanOffset != pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset &&
			    RootApHtInfo.RecomWidth)
			{
				if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE) {
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
					pApCliEntry->ApCliMlmeAux.CentralChannel = pApCliEntry->ApCliMlmeAux.Channel + 2;
				} else {
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_BELOW;
					pApCliEntry->ApCliMlmeAux.CentralChannel = pApCliEntry->ApCliMlmeAux.Channel - 2;
				}

				pAd->CommonCfg.Channel = pApCliEntry->ApCliMlmeAux.Channel;
				pAd->CommonCfg.CentralChannel = pApCliEntry->ApCliMlmeAux.CentralChannel;
#ifdef DOT11N_DRAFT3
				pAd->CommonCfg.Bss2040NeedFallBack = 0;
				pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 0;
#endif /* DOT11N_DRAFT3 */
				SetCommonHT(pAd);
				AsicBBPAdjust(pAd);
				AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
				AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

				DBGPRINT(RT_DEBUG_OFF, ("%s(): ch(%d), cent_ch(%d), extcha(%d), bbp bw(%d)\n",
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
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->AuthReqCnt = 0;

		COPY_MAC_ADDR(AuthReq.Addr, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid);

		/* start Authentication Req. */
		/* If AuthMode is Auto, try shared key first */
		if ((pAd->ApCfg.ApCliTab[ifIndex].AuthMode == Ndis802_11AuthModeShared) ||
				(pAd->ApCfg.ApCliTab[ifIndex].AuthMode == Ndis802_11AuthModeAutoSwitch))
		{		
			AuthReq.Alg = Ndis802_11AuthModeShared;
		}
		else
		{
			AuthReq.Alg = Ndis802_11AuthModeOpen;
		}

		AuthReq.Timeout = AUTH_TIMEOUT;

#ifdef MAC_REPEATER_SUPPORT
		AuthReq.BssIdx = ifIndex;
		AuthReq.CliIdx = CliIdx;
		if (CliIdx != 0xFF)
		{
			ifIndex = (64 + 16*ifIndex + CliIdx);
			DBGPRINT(RT_DEBUG_ERROR, ("(%s) Repeater Cli Trigger Auth Req ifIndex = %d, CliIdx = %d !!!\n",
							__FUNCTION__, ifIndex, CliIdx));
		}
#endif /* MAC_REPEATER_SUPPORT */

		MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
			sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
	} 
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe respond fail.\n", __FUNCTION__));
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
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
	}
#endif /* MAC_REPEATER_SUPPORT */

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("(%s) Repeater Cli Receive ifIndex = %d, CliIdx = %d !!!\n",
						__FUNCTION__, ifIndex, CliIdx));

		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	if(Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth Rsp Success.\n", __FUNCTION__));

		*pCurrState = APCLI_CTRL_ASSOC;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->AssocReqCnt = 0;

		AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.CapabilityInfo,
			ASSOC_TIMEOUT, 5);

#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
			sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);
	} 
	else
	{
		if (pApCliEntry->AuthMode == Ndis802_11AuthModeAutoSwitch)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth Rsp Failure.\n", __FUNCTION__));

			*pCurrState = APCLI_CTRL_AUTH_2;

			/* start Second Authentication Req. */
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) Start Second Auth Rep.\n", __FUNCTION__));
			COPY_MAC_ADDR(AuthReq.Addr, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid);
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
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthReqCnt = 0;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
				NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
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
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
	}
#endif /* MAC_REPEATER_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if(Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth2 Rsp Success.\n", __FUNCTION__));
		*pCurrState = APCLI_CTRL_ASSOC;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->AssocReqCnt = 0;

		AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.CapabilityInfo,
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
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth Req Timeout.\n", __FUNCTION__));

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthReqCnt++;

		if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthReqCnt = 0;

			/* todo : need handle associate request time out retry 5 times */
			//RTMPSetTimer(&pApCliEntry->RepeaterCli[CliIdx].ReptCliResetEntryTimer, AUTH_TIMEOUT * 20);
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
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
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
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Retry Auth Req.\n", __FUNCTION__));
	COPY_MAC_ADDR(AuthReq.Addr, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid);
	AuthReq.Alg = pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Alg; /*Ndis802_11AuthModeOpen; */
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
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);

		DBGPRINT(RT_DEBUG_ERROR, ("(%s) Repeater Cli Receive Assoc Rsp ifIndex = %d, CliIdx = %d.\n",
					__FUNCTION__, ifIndex, CliIdx));
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if(Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) apCliIf = %d, Receive Assoc Rsp Success.\n", __FUNCTION__, ifIndex));

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
			    if (pAd->ApCfg.ApCliTab[ifIndex].WpaSupplicantUP)
			    {
			        ApcliSendAssocIEsToWpaSupplicant(pAd,ifIndex);
				RtmpOSWrielessEventSend(pAd->net_dev,
								RT_WLAN_EVENT_CUSTOM,
								RT_ASSOC_EVENT_FLAG,
								NULL, NULL, 0);
			    }
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */                    

#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		if (ApCliLinkUp(pAd, ifIndex))
		{
			*pCurrState = APCLI_CTRL_CONNECTED;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) apCliIf = %d, Insert Remote AP to MacTable failed.\n", __FUNCTION__,  ifIndex));
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
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) apCliIf = %d, Receive Assoc Rsp Failure.\n", __FUNCTION__,  ifIndex));

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
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if (Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Receive DeAssoc Rsp Success.\n", __FUNCTION__));
	} else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Receive DeAssoc Rsp Failure.\n", __FUNCTION__));
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

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Assoc Req Timeout.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
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

			/* todo : need handle associate request time out retry 5 times */
			//RTMPSetTimer(&pApCliEntry->RepeaterCli[CliIdx].ReptCliResetEntryTimer, ASSOC_TIMEOUT * 20);
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
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
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
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Retry Association Req.\n", __FUNCTION__));
	AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.CapabilityInfo,
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

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) MLME Request disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
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

		/* clear ApCliMlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Rssi = 0;
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
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
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

		if (pApCliEntry->Valid)
		{
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];

			if (pEntry && (pEntry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				for(index = 0; index < MAX_EXT_MAC_ADDR_SIZE; index++)
				{
					if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].CliEnable)
					{
						RTMPRemoveRepeaterDisconnectEntry(pAd, ifIndex, index);
						RTMPRemoveRepeaterEntry(pAd, ifIndex, index);
					}
				}
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
		PMAC_TABLE_ENTRY pMacEntry;
		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		if (pMacEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
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

		/* clear ApCliMlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Rssi = 0;
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

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) MLME Request Disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	DisassocParmFill(pAd, &DisassocReq, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		COPY_MAC_ADDR(DisassocReq.Addr2, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	}
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
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable = FALSE;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;

		/* clear ApCliMlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Rssi = 0;
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

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) MLME Request Disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* Fill in the related information */
	DeAuthFrame.Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
	COPY_MAC_ADDR(DeAuthFrame.Addr, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		COPY_MAC_ADDR(DeAuthFrame.Addr2, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	}
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
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable = FALSE;
		//RTMPDelRepeaterCliAsicEntry(pAd, CliIdx);
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;

		/* clear ApCliMlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Rssi = 0;
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
	UCHAR				*mpool;
	PEAPOL_PACKET       pPacket;
	UCHAR               Mic[16];
	BOOLEAN             bUnicast;
	UCHAR			Wcid;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	DBGPRINT(RT_DEBUG_TRACE, ("ApCliWpaMicFailureReportFrame ----->\n"));

	if (ifIndex >= MAX_APCLI_NUM)
		return;
	
	bUnicast = (Elem->Msg[0] == 1 ? TRUE:FALSE);
	pAd->Sequence = ((pAd->Sequence) + 1) & (MAX_SEQ_NUMBER);
	

	/* init 802.3 header and Fill Packet */
	pMacEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[ifIndex].MacTabWCID];
	if (!IS_ENTRY_APCLI(pMacEntry))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : !IS_ENTRY_APCLI(pMacEntry)\n", __FUNCTION__));
		return;
	}
	
	Wcid =  pAd->ApCfg.ApCliTab[ifIndex].MacTabWCID;
	MAKE_802_3_HEADER(Header802_3, pAd->MacTab.Content[Wcid].Addr, pAd->ApCfg.ApCliTab[ifIndex].CurrentAddress, EAPOL);
	
	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
		return;
	}

	pPacket = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pPacket, TX_EAPOL_BUFFER);
	
	pPacket->ProVer	= EAPOL_VER;
	pPacket->ProType	= EAPOLKey;
	
	pPacket->KeyDesc.Type = WPA1_KEY_DESC;

	/* Request field presented */
	pPacket->KeyDesc.KeyInfo.Request = 1;

	if(pAd->ApCfg.ApCliTab[ifIndex].WepStatus  == Ndis802_11Encryption3Enabled)
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
	if(pAd->ApCfg.ApCliTab[ifIndex].WepStatus  == Ndis802_11Encryption3Enabled)
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

	DBGPRINT(RT_DEBUG_TRACE, ("ApCliWpaMicFailureReportFrame <-----\n"));
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
	    	DBGPRINT(RT_DEBUG_TRACE, ("Update2040CoexistFrameAndNotify @%s  \n", __FUNCTION__));    
		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (IS_ENTRY_APCLI(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].apidx == ifIndex))
			{
				Update2040CoexistFrameAndNotify(pAd, i, TRUE);
			}
		}			
	}
#endif /* DOT11N_DRAFT3 */
}
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */

