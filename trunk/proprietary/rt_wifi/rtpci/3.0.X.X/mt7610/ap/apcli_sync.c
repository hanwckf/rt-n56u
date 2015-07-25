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
	sta_sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-06-23      modified for rt61-APClinent
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

static VOID ApCliProbeTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

static VOID ApCliMlmeProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerProbeRspAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliProbeTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliEnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR SsidLen,
	OUT PCHAR Ssid,
	IN USHORT ifIndex);

DECLARE_TIMER_FUNCTION(ApCliProbeTimeout);
BUILD_TIMER_FUNCTION(ApCliProbeTimeout);

/*
    ==========================================================================
    Description:
        The sync state machine, 
    Parameters:
        Sm - pointer to the state machine
    Note:
        the state machine looks like the following
    ==========================================================================
 */
VOID ApCliSyncStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
	PAPCLI_STRUCT	pApCliEntry;

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_SYNC_STATE, APCLI_MAX_SYNC_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_SYNC_IDLE,
		APCLI_SYNC_MACHINE_BASE);

	/* column 1 */
	StateMachineSetAction(Sm, APCLI_SYNC_IDLE, APCLI_MT2_MLME_PROBE_REQ, (STATE_MACHINE_FUNC)ApCliMlmeProbeReqAction);

	/*column 2 */
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_MLME_PROBE_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenJoin);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)ApCliPeerProbeRspAtJoinAction);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PROBE_TIMEOUT, (STATE_MACHINE_FUNC)ApCliProbeTimeoutAtJoinAction);


	for (i = 0; i < MAX_APCLI_NUM; i++)
	{
		/* timer init */
		pApCliEntry = &pAd->ApCfg.ApCliTab[i];
		RTMPInitTimer(pAd, &pApCliEntry->ApCliMlmeAux.ProbeTimer, GET_TIMER_FUNCTION(ApCliProbeTimeout), (PVOID)pApCliEntry, FALSE);

		pApCliEntry->SyncCurrState = APCLI_SYNC_IDLE;
	}

	return;
}

/* 
    ==========================================================================
    Description:
        Becaon timeout handler, executed in timer thread
    ==========================================================================
 */
static VOID ApCliProbeTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli_SYNC - ProbeReqTimeout\n"));

#ifndef APCLI_CONNECTION_TRIAL
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_PROBE_TIMEOUT, 0, NULL, 0);
#else
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_PROBE_TIMEOUT, 0, NULL, pApCliEntry->ifIndex);
#endif /* APCLI_CONNECTION_TRIAL */
	RTMP_MLME_HANDLER(pAd);

	return;
}

/* 
    ==========================================================================
    Description:
        MLME PROBE req state machine procedure
    ==========================================================================
 */
static VOID ApCliMlmeProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
	APCLI_MLME_JOIN_REQ_STRUCT *Info = (APCLI_MLME_JOIN_REQ_STRUCT *)(Elem->Msg);
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
	PAPCLI_STRUCT pApCliEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli SYNC - ApCliMlmeProbeReqAction(Ssid %s), ifIndex = %d\n", Info->Ssid, ifIndex));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* reset all the timers */
	RTMPCancelTimer(&(pApCliEntry->ApCliMlmeAux.ProbeTimer), &Cancelled);

	pApCliEntry->ApCliMlmeAux.Rssi = -9999;
#ifdef APCLI_CONNECTION_TRIAL
	if (pApCliEntry->TrialCh ==0)
#endif /* APCLI_CONNECTION_TRIAL */
	pApCliEntry->ApCliMlmeAux.Channel = pAd->CommonCfg.Channel;
#ifdef APCLI_CONNECTION_TRIAL
	else
		pApCliEntry->ApCliMlmeAux.Channel = pApCliEntry->TrialCh;
#endif /* APCLI_CONNECTION_TRIAL */
	pApCliEntry->ApCliMlmeAux.SupRateLen = pAd->CommonCfg.SupRateLen;
	NdisMoveMemory(pApCliEntry->ApCliMlmeAux.SupRate, pAd->CommonCfg.SupRate, pAd->CommonCfg.SupRateLen);

	/* Prepare the default value for extended rate */
	pApCliEntry->ApCliMlmeAux.ExtRateLen = pAd->CommonCfg.ExtRateLen;
	NdisMoveMemory(pApCliEntry->ApCliMlmeAux.ExtRate, pAd->CommonCfg.ExtRate, pAd->CommonCfg.ExtRateLen);

	RTMPSetTimer(&(pApCliEntry->ApCliMlmeAux.ProbeTimer), PROBE_TIMEOUT);

#ifdef APCLI_CONNECTION_TRIAL
	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, MAX_LEN_OF_SSID);
	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, MAC_ADDR_LEN);
	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Ssid, pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen);
#endif /* APCLI_CONNECTION_TRIAL */

	ApCliEnqueueProbeRequest(pAd, Info->SsidLen, (PCHAR) Info->Ssid, ifIndex);

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli SYNC - Start Probe the SSID %s on channel =%d\n", pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->ApCliMlmeAux.Channel));

	*pCurrState = APCLI_JOIN_WAIT_PROBE_RSP;

	return;
}

/* 
    ==========================================================================
    Description:
        When waiting joining the (I)BSS, beacon received from external
    ==========================================================================
 */
static VOID ApCliPeerProbeRspAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT LenVIE;
	UCHAR *VarIE = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	PAPCLI_STRUCT pApCliEntry = NULL;
#ifdef DOT11_N_SUPPORT
	UCHAR CentralChannel;
#endif /* DOT11_N_SUPPORT */
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BCN_IE_LIST *ie_list = NULL;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;

	/* Init Variable IE structure */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate ie_list fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));




	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Elem->Channel,
								ie_list,
								&LenVIE,
								pVIE))
	{
		/*
			BEACON from desired BSS/IBSS found. We should be able to decide most
			BSS parameters here.
			Q. But what happen if this JOIN doesn't conclude a successful ASSOCIATEION?
				Do we need to receover back all parameters belonging to previous BSS?
			A. Should be not. There's no back-door recover to previous AP. It still need
				a new JOIN-AUTH-ASSOC sequence.
		*/
		INT ssidEqualFlag = FALSE;
		INT ssidEmptyFlag = FALSE;
		INT bssidEqualFlag = FALSE;
		INT bssidEmptyFlag = FALSE;
		INT matchFlag = FALSE;

		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

		/* Check the Probe-Rsp's Bssid. */
		if(!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
			bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ie_list->Bssid);
		else
			bssidEmptyFlag = TRUE;

		/* Check the Probe-Rsp's Ssid. */
		if(pApCliEntry->CfgSsidLen != 0)
			ssidEqualFlag = SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, ie_list->Ssid, ie_list->SsidLen);
		else
			ssidEmptyFlag = TRUE;


		/* bssid and ssid, Both match. */
		if (bssidEqualFlag && ssidEqualFlag)
			matchFlag = TRUE;

		/* ssid match but bssid doesn't be indicate. */
		else if(ssidEqualFlag && bssidEmptyFlag)
			matchFlag = TRUE;

		/* user doesn't indicate any bssid or ssid. AP-Clinet will auto pick a AP to join by most strong siganl strength. */
		else if (bssidEmptyFlag && ssidEmptyFlag)
			matchFlag = TRUE;


		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - bssidEqualFlag=%d, ssidEqualFlag=%d, matchFlag=%d\n", bssidEqualFlag, ssidEqualFlag, matchFlag));
		if (matchFlag)
		{
			/* Validate RSN IE if necessary, then copy store this information */
			if ((LenVIE > 0) 
#ifdef WSC_AP_SUPPORT
                && ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode == WSC_DISABLE) || 
                	(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
                )
			{
				if (ApCliValidateRSNIE(pAd, (PEID_STRUCT)pVIE, LenVIE, ifIndex))
				{
					pApCliEntry->ApCliMlmeAux.VarIELen = LenVIE;
					NdisMoveMemory(pApCliEntry->ApCliMlmeAux.VarIEs, pVIE, pApCliEntry->ApCliMlmeAux.VarIELen);
				}
				else
				{
					/* ignore this response */
					pApCliEntry->ApCliMlmeAux.VarIELen = 0;
					DBGPRINT(RT_DEBUG_ERROR, ("ERROR: The RSN IE of this received Probe-resp is dis-match !!!!!!!!!! \n"));
					goto LabelErr;
				}
			}
			else
			{
				if (pApCliEntry->AuthMode >= Ndis802_11AuthModeWPA
#ifdef WSC_AP_SUPPORT
                    && ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode == WSC_DISABLE) || 
                		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
                    )
				{
					/* ignore this response */
					DBGPRINT(RT_DEBUG_ERROR, ("ERROR: The received Probe-resp has empty RSN IE !!!!!!!!!! \n"));
					goto LabelErr;
				}	
				
				pApCliEntry->ApCliMlmeAux.VarIELen = 0;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - receive desired PROBE_RSP at JoinWaitProbeRsp... Channel = %d\n", ie_list->Channel));

			/* if the Bssid doesn't be indicated then you need to decide which AP to connect by most strong Rssi signal strength. */
			if (bssidEqualFlag == FALSE)
			{
				/* caculate real rssi value. */
				CHAR Rssi0 = ConvertToRssi(pAd, Elem->Rssi0, RSSI_0);
				CHAR Rssi1 = ConvertToRssi(pAd, Elem->Rssi1, RSSI_1);
				CHAR Rssi2 = ConvertToRssi(pAd, Elem->Rssi2, RSSI_2);
				LONG RealRssi = (LONG)(RTMPMaxRssi(pAd, Rssi0, Rssi1, Rssi2));

				DBGPRINT(RT_DEBUG_TRACE, ("SYNC - previous Rssi = %ld current Rssi=%ld\n", pApCliEntry->ApCliMlmeAux.Rssi, (LONG)RealRssi));
				if (pApCliEntry->ApCliMlmeAux.Rssi > (LONG)RealRssi)
					goto LabelErr;
				else
					pApCliEntry->ApCliMlmeAux.Rssi = RealRssi;
			} else
			{
				BOOLEAN Cancelled;
				RTMPCancelTimer(&pApCliEntry->ApCliMlmeAux.ProbeTimer, &Cancelled);
			}

			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.Ssid, ie_list->Ssid, ie_list->SsidLen);
			pApCliEntry->ApCliMlmeAux.SsidLen = ie_list->SsidLen;

			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.Bssid, ie_list->Bssid, MAC_ADDR_LEN);			
			pApCliEntry->ApCliMlmeAux.CapabilityInfo = ie_list->CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
			pApCliEntry->ApCliMlmeAux.BssType = ie_list->BssType;
			pApCliEntry->ApCliMlmeAux.BeaconPeriod = ie_list->BeaconPeriod;
			pApCliEntry->ApCliMlmeAux.Channel = ie_list->Channel;
			pApCliEntry->ApCliMlmeAux.AtimWin = ie_list->AtimWin;
			pApCliEntry->ApCliMlmeAux.CfpPeriod = ie_list->CfParm.CfpPeriod;
			pApCliEntry->ApCliMlmeAux.CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			pApCliEntry->ApCliMlmeAux.APRalinkIe = ie_list->RalinkIe;

			/* Copy AP's supported rate to ApCliMlmeAux for creating assoication request */
			/* Also filter out not supported rate */
			pApCliEntry->ApCliMlmeAux.SupRateLen = ie_list->SupRateLen;
			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.SupRate, ie_list->SupRate, ie_list->SupRateLen);
			RTMPCheckRates(pAd, pApCliEntry->ApCliMlmeAux.SupRate, &pApCliEntry->ApCliMlmeAux.SupRateLen);
			pApCliEntry->ApCliMlmeAux.ExtRateLen = ie_list->ExtRateLen;
			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.ExtRate, ie_list->ExtRate, ie_list->ExtRateLen);
			RTMPCheckRates(pAd, pApCliEntry->ApCliMlmeAux.ExtRate, &pApCliEntry->ApCliMlmeAux.ExtRateLen);

#ifdef DOT11_N_SUPPORT
			NdisZeroMemory(pApCliEntry->RxMcsSet,sizeof(pApCliEntry->RxMcsSet));
			/* filter out un-supported ht rates */
			if ((ie_list->HtCapabilityLen > 0) && 
				(pApCliEntry->DesiredHtPhyInfo.bHtEnable) &&
				WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			{
				RTMPZeroMemory(&pApCliEntry->ApCliMlmeAux.HtCapability, SIZE_HT_CAP_IE);
				pApCliEntry->ApCliMlmeAux.NewExtChannelOffset = ie_list->NewExtChannelOffset;
				pApCliEntry->ApCliMlmeAux.HtCapabilityLen = ie_list->HtCapabilityLen;
				ApCliCheckHt(pAd, ifIndex, &ie_list->HtCapability, &ie_list->AddHtInfo);

				if (ie_list->AddHtInfoLen > 0)
				{
					CentralChannel = ie_list->AddHtInfo.ControlChan;
		 			/* Check again the Bandwidth capability of this AP. */
					CentralChannel = get_cent_ch_by_htinfo(pAd, &ie_list->AddHtInfo,
														&ie_list->HtCapability);
		 			DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction HT===>Central Channel = %d, Control Channel = %d,  .\n", CentralChannel, ie_list->AddHtInfo.ControlChan));

				}
				
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
				RTMPZeroMemory(&pApCliEntry->ApCliMlmeAux.HtCapability, SIZE_HT_CAP_IE);
				RTMPZeroMemory(&pApCliEntry->ApCliMlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
				pApCliEntry->ApCliMlmeAux.HtCapabilityLen = 0;
			}

#ifdef DOT11_VHT_AC
			if (ie_list->vht_op_len)
			{
				/*
					To save the VHT Channel Width of AP.
				*/
				RTMPZeroMemory(&pApCliEntry->ApCliMlmeAux.vht_op, sizeof(VHT_OP_IE));
				NdisCopyMemory(&pApCliEntry->ApCliMlmeAux.vht_op, &(ie_list->vht_op_ie), ie_list->vht_op_len);
			}
#endif /* DOT11_VHT_AC */

			ApCliUpdateMlmeRate(pAd, ifIndex);

#ifdef DOT11_N_SUPPORT
			/* copy QOS related information */
			if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			{
				NdisMoveMemory(&pApCliEntry->ApCliMlmeAux.APEdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
				NdisMoveMemory(&pApCliEntry->ApCliMlmeAux.APQbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pApCliEntry->ApCliMlmeAux.APQosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
				NdisZeroMemory(&pApCliEntry->ApCliMlmeAux.APEdcaParm, sizeof(EDCA_PARM));
				NdisZeroMemory(&pApCliEntry->ApCliMlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisZeroMemory(&pApCliEntry->ApCliMlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("APCLI SYNC - after JOIN, SupRateLen=%d, ExtRateLen=%d\n", 
				pApCliEntry->ApCliMlmeAux.SupRateLen, pApCliEntry->ApCliMlmeAux.ExtRateLen));

			if (ie_list->AironetCellPowerLimit != 0xFF)
			{
				/*We need to change our TxPower for CCX 2.0 AP Control of Client Transmit Power */
				ChangeToCellPowerLimit(pAd, ie_list->AironetCellPowerLimit);
			}
			else  /*Used the default TX Power Percentage. */
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;

#ifdef WSC_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
			if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
                (pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
			{
				ADD_HTINFO	RootApHtInfo, ApHtInfo;
				ApHtInfo = pAd->CommonCfg.AddHTInfo.AddHtInfo;
				RootApHtInfo = ie_list->AddHtInfo.AddHtInfo;
				if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
					(RootApHtInfo.RecomWidth) &&
					(RootApHtInfo.ExtChanOffset != ApHtInfo.ExtChanOffset))
				{
					/*STRING	ChStr[5] = {0}; */
					
					if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE)
						Set_HtExtcha_Proc(pAd, "1");
					else
						Set_HtExtcha_Proc(pAd, "0");

					goto LabelErr;
				}				
			}
#endif /* DOT11_N_SUPPORT */
#endif /* WSC_AP_SUPPORT */
			if(bssidEqualFlag == TRUE)
			{
				*pCurrState = APCLI_SYNC_IDLE;

				ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
				ApCliCtrlMsg.BssIdx = ifIndex;
				ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
				MlmeHandler(pAd);
			}
		}
		/* not to me BEACON, ignored */
	}
	/* sanity check fail, ignore this frame */

LabelErr:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);

	return;
}

static VOID ApCliProbeTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
	PAPCLI_STRUCT pApCliEntry = NULL;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCurrCtrlState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_SYNC - ProbeTimeoutAtJoinAction, ifIndex = %d, pCurrCtrlState = %ld, pCurrSyncState = %ld\n", ifIndex, *pCurrCtrlState, *pCurrState));
#endif

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pCurrState = &pApCliEntry->SyncCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_SYNC - ProbeTimeoutAtJoinAction\n"));
	*pCurrState = SYNC_IDLE;
#ifdef APCLI_CONNECTION_TRIAL
	if (ifIndex == 1)
		*pCurrCtrlState = APCLI_CTRL_DISCONNECTED;
#endif /* APCLI_CONNECTION_TRIAL */

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_SYNC - ApCliMlmeAux.Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pApCliEntry->ApCliMlmeAux.Bssid[0], 
		pApCliEntry->ApCliMlmeAux.Bssid[1], 
		pApCliEntry->ApCliMlmeAux.Bssid[2], 
		pApCliEntry->ApCliMlmeAux.Bssid[3], 
		pApCliEntry->ApCliMlmeAux.Bssid[4], 
		pApCliEntry->ApCliMlmeAux.Bssid[5]));

	if(!MAC_ADDR_EQUAL(pApCliEntry->ApCliMlmeAux.Bssid, ZERO_MAC_ADDR))
	{
		ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	} else
	{
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
	}

	return;
}

/* 
    ==========================================================================
    Description:
    ==========================================================================
 */
static VOID ApCliInvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;

	*pCurrState = APCLI_SYNC_IDLE;
	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_AYNC - ApCliInvalidStateWhenJoin(state=%ld). Reset SYNC machine\n", *pCurrState));

	return;
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliEnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR SsidLen,
	OUT PCHAR Ssid,
	IN USHORT ifIndex)
{
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer;
	ULONG           FrameLen = 0;
	HEADER_802_11   Hdr80211;
	UCHAR           SsidIe    = IE_SSID;
	UCHAR           SupRateIe = IE_SUPP_RATES;
	UCHAR ssidLen;
	CHAR ssid[MAX_LEN_OF_SSID];
	PAPCLI_STRUCT pApCliEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("force out a ProbeRequest ...\n"));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	
	NState = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
	if(NState != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("EnqueueProbeRequest() allocate memory fail\n"));
		return;
	} else
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		if(MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, ZERO_MAC_ADDR))
			ApCliMgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0,
				BROADCAST_ADDR, BROADCAST_ADDR, ifIndex);
		else
			ApCliMgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0,
				pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, ifIndex);

		ssidLen = SsidLen;
		NdisZeroMemory(ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(ssid, Ssid, ssidLen);

		/* this ProbeRequest explicitly specify SSID to reduce unwanted ProbeResponse */
		MakeOutgoingFrame(pOutBuffer,		&FrameLen,
			sizeof(HEADER_802_11),			&Hdr80211,
			1,								&SsidIe,
			1,								&ssidLen,
			ssidLen,						ssid,
			1,								&SupRateIe,
			1,								&(pApCliEntry->ApCliMlmeAux.SupRateLen),
			pApCliEntry->ApCliMlmeAux.SupRateLen,		pApCliEntry->ApCliMlmeAux.SupRate,
			END_OF_ARGS);

		/* Add the extended rate IE */
		if (pApCliEntry->ApCliMlmeAux.ExtRateLen != 0)
		{
			ULONG            tmp;
		
			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
				1,                        &ExtRateIe,
				1,                        &(pApCliEntry->ApCliMlmeAux.ExtRateLen),
				pApCliEntry->ApCliMlmeAux.ExtRateLen,  pApCliEntry->ApCliMlmeAux.ExtRate,                           
				END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
			(pAd->CommonCfg.Channel > 14))
		{
			/*
				We don't know VHT_BW of AP in this stage.
				Use own AP's VHT MAX MCS CAp in probe request.
			*/
			build_vht_cap_ie(pAd, (UCHAR *)&pApCliEntry->ApCliMlmeAux.vht_cap, pAd->CommonCfg.vht_max_mcs_cap);
			pApCliEntry->ApCliMlmeAux.vht_cap_len = sizeof(VHT_CAP_IE);
			FrameLen += build_vht_ies(pAd, (UCHAR *)(pOutBuffer + FrameLen), SUBTYPE_PROBE_REQ, pAd->CommonCfg.vht_max_mcs_cap);
		}
#endif /* DOT11_VHT_AC */

		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}

	return;
}

#endif /* APCLI_SUPPORT */

