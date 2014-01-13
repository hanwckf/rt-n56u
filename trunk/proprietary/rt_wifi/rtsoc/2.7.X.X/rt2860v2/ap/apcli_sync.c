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
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ProbeTimer, GET_TIMER_FUNCTION(ApCliProbeTimeout), pAd, FALSE);

		pAd->ApCfg.ApCliTab[i].SyncCurrState = APCLI_SYNC_IDLE;
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
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli_SYNC - ProbeReqTimeout\n"));

	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_PROBE_TIMEOUT, 0, NULL, 0);
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

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli SYNC - ApCliMlmeProbeReqAction(Ssid %s)\n", Info->Ssid));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* reset all the timers */
	RTMPCancelTimer(&(pApCliEntry->ApCliMlmeAux.ProbeTimer), &Cancelled);

	pApCliEntry->ApCliMlmeAux.Rssi = -9999;
	pApCliEntry->ApCliMlmeAux.Channel = pAd->CommonCfg.Channel;
	pApCliEntry->ApCliMlmeAux.SupRateLen = pAd->CommonCfg.SupRateLen;
	NdisMoveMemory(pApCliEntry->ApCliMlmeAux.SupRate, pAd->CommonCfg.SupRate, pAd->CommonCfg.SupRateLen);

	/* Prepare the default value for extended rate */
	pApCliEntry->ApCliMlmeAux.ExtRateLen = pAd->CommonCfg.ExtRateLen;
	NdisMoveMemory(pApCliEntry->ApCliMlmeAux.ExtRate, pAd->CommonCfg.ExtRate, pAd->CommonCfg.ExtRateLen);

	RTMPSetTimer(&(pApCliEntry->ApCliMlmeAux.ProbeTimer), PROBE_TIMEOUT);

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
	UCHAR Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	UCHAR /* Ssid[MAX_LEN_OF_SSID], */ SsidLen=0, BssType, Channel=0, MessageToMe, 
		DtimCount, DtimPeriod, BcastFlag; 
	UCHAR *Ssid = NULL;
	LARGE_INTEGER TimeStamp;
	USHORT BeaconPeriod, AtimWin, CapabilityInfo;
/*	UINT FrameLen = 0; */
	CF_PARM Cf;
	UCHAR Erp;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen, ExtRateLen;
	UCHAR CkipFlag;
	USHORT LenVIE;
	UCHAR AironetCellPowerLimit;
	EDCA_PARM EdcaParm;
	QBSS_LOAD_PARM QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
/*	UCHAR VarIE[MAX_VIE_LEN];		// Total VIE length = MAX_VIE_LEN - -5 */
	UCHAR *VarIE = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	ULONG RalinkIe;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
/*	HT_CAPABILITY_IE HtCapability; */
/*	ADD_HT_INFO_IE AddHtInfo;	// AP might use this additional ht info IE */
	HT_CAPABILITY_IE *pHtCapability = NULL;
	ADD_HT_INFO_IE *pAddHtInfo = NULL;	/* AP might use this additional ht info IE */
	UCHAR HtCapabilityLen;
	UCHAR AddHtInfoLen;
	UCHAR NewChannel;
	UCHAR NewExtChannelOffset = 0xff;
	PAPCLI_STRUCT pApCliEntry = NULL;
#ifdef DOT11_N_SUPPORT
        UCHAR CentralChannel;
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	UCHAR	pPreNHtCapabilityLen = 0;
#endif /* CONFIG_STA_SUPPORT */
	EXT_CAP_INFO_ELEMENT	ExtCapInfo;

	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	os_alloc_mem(NULL, (UCHAR **)&pHtCapability, sizeof(HT_CAPABILITY_IE));
	if (pHtCapability == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	os_alloc_mem(NULL, (UCHAR **)&pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	if (pAddHtInfo == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	os_alloc_mem(NULL, (UCHAR **)&Ssid, MAX_LEN_OF_SSID);
	if (Ssid == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}

	/* Init Variable IE structure */
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	RTMPZeroMemory(&QosCapability, sizeof(QosCapability));
	RTMPZeroMemory(&EdcaParm, sizeof(EdcaParm));

	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Elem->Channel,
								Addr2, 
								Bssid, 
								(PCHAR) Ssid, 
								&SsidLen, 
								&BssType, 
								&BeaconPeriod, 
								&Channel, 
								&NewChannel,
								&TimeStamp, 
								&Cf, 
								&AtimWin, 
								&CapabilityInfo, 
								&Erp,
								&DtimCount, 
								&DtimPeriod, 
								&BcastFlag, 
								&MessageToMe, 
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
#ifdef CONFIG_STA_SUPPORT
								&pPreNHtCapabilityLen,
#endif /* CONFIG_STA_SUPPORT */
								pHtCapability,
								&ExtCapInfo,
								&AddHtInfoLen,
								pAddHtInfo,
								&NewExtChannelOffset,
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
			bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, Bssid);
		else
			bssidEmptyFlag = TRUE;

		/* Check the Probe-Rsp's Ssid. */
		if(pApCliEntry->CfgSsidLen != 0)
			ssidEqualFlag = SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, Ssid, SsidLen);
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

			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - receive desired PROBE_RSP at JoinWaitProbeRsp... Channel = %d\n", Channel));

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

			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.Ssid, Ssid, SsidLen);
			pApCliEntry->ApCliMlmeAux.SsidLen = SsidLen;

			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.Bssid, Bssid, MAC_ADDR_LEN);			
			pApCliEntry->ApCliMlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
			pApCliEntry->ApCliMlmeAux.BssType = BssType;
			pApCliEntry->ApCliMlmeAux.BeaconPeriod = BeaconPeriod;
			pApCliEntry->ApCliMlmeAux.Channel = Channel;
			pApCliEntry->ApCliMlmeAux.AtimWin = AtimWin;
			pApCliEntry->ApCliMlmeAux.CfpPeriod = Cf.CfpPeriod;
			pApCliEntry->ApCliMlmeAux.CfpMaxDuration = Cf.CfpMaxDuration;
			pApCliEntry->ApCliMlmeAux.APRalinkIe = RalinkIe;

			/* Copy AP's supported rate to ApCliMlmeAux for creating assoication request */
			/* Also filter out not supported rate */
			pApCliEntry->ApCliMlmeAux.SupRateLen = SupRateLen;
			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.SupRate, SupRate, SupRateLen);
			RTMPCheckRates(pAd, pApCliEntry->ApCliMlmeAux.SupRate, &(pApCliEntry->ApCliMlmeAux.SupRateLen));
			pApCliEntry->ApCliMlmeAux.ExtRateLen = ExtRateLen;
			NdisMoveMemory(pApCliEntry->ApCliMlmeAux.ExtRate, ExtRate, ExtRateLen);
			RTMPCheckRates(pAd, pApCliEntry->ApCliMlmeAux.ExtRate, &pApCliEntry->ApCliMlmeAux.ExtRateLen);

#ifdef DOT11_N_SUPPORT
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].RxMcsSet,sizeof(pAd->ApCfg.ApCliTab[ifIndex].RxMcsSet));
			/* filter out un-supported ht rates */
			if ((HtCapabilityLen > 0) && 
				(pApCliEntry->DesiredHtPhyInfo.bHtEnable) &&
				(pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
			{
				RTMPZeroMemory(&(pApCliEntry->ApCliMlmeAux.HtCapability), SIZE_HT_CAP_IE);
				pApCliEntry->ApCliMlmeAux.NewExtChannelOffset = NewExtChannelOffset;
				pApCliEntry->ApCliMlmeAux.HtCapabilityLen = HtCapabilityLen;
				ApCliCheckHt(pAd, ifIndex, pHtCapability, pAddHtInfo);

				if (AddHtInfoLen > 0)
				{
					CentralChannel = pAddHtInfo->ControlChan;
		 			/* Check again the Bandwidth capability of this AP. */
		 			if ((pAddHtInfo->ControlChan > 2)&& (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
		 			{
		 				CentralChannel = pAddHtInfo->ControlChan - 2;
		 			}
		 			else if ((pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
		 			{
		 				CentralChannel = pAddHtInfo->ControlChan + 2;
		 			}
		 			DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction HT===>Central Channel = %d, Control Channel = %d,  .\n", CentralChannel, pAddHtInfo->ControlChan));

				}
				
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
				RTMPZeroMemory(&(pApCliEntry->ApCliMlmeAux.HtCapability), SIZE_HT_CAP_IE);
				RTMPZeroMemory(&(pApCliEntry->ApCliMlmeAux.AddHtInfo), SIZE_ADD_HT_INFO_IE);
				pApCliEntry->ApCliMlmeAux.HtCapabilityLen = 0;
			}
			ApCliUpdateMlmeRate(pAd, ifIndex);

#ifdef DOT11_N_SUPPORT
			/* copy QOS related information */
			if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			{
				NdisMoveMemory(&(pApCliEntry->ApCliMlmeAux.APEdcaParm), &EdcaParm, sizeof(EDCA_PARM));
				NdisMoveMemory(&(pApCliEntry->ApCliMlmeAux.APQbssLoad), &QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&(pApCliEntry->ApCliMlmeAux.APQosCapability), &QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
				NdisZeroMemory(&(pApCliEntry->ApCliMlmeAux.APEdcaParm), sizeof(EDCA_PARM));
				NdisZeroMemory(&(pApCliEntry->ApCliMlmeAux.APQbssLoad), sizeof(QBSS_LOAD_PARM));
				NdisZeroMemory(&(pApCliEntry->ApCliMlmeAux.APQosCapability), sizeof(QOS_CAPABILITY_PARM));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("APCLI SYNC - after JOIN, SupRateLen=%d, ExtRateLen=%d\n", 
				pApCliEntry->ApCliMlmeAux.SupRateLen, pApCliEntry->ApCliMlmeAux.ExtRateLen));

			if (AironetCellPowerLimit != 0xFF)
			{
				/*We need to change our TxPower for CCX 2.0 AP Control of Client Transmit Power */
				ChangeToCellPowerLimit(pAd, AironetCellPowerLimit);
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
				RootApHtInfo = pAddHtInfo->AddHtInfo;
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
			}
		}
		/* not to me BEACON, ignored */
	}
	/* sanity check fail, ignore this frame */

LabelErr:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (pHtCapability != NULL)
		os_free_mem(NULL, pHtCapability);
	if (pAddHtInfo != NULL)
		os_free_mem(NULL, pAddHtInfo);
	if (Ssid != NULL)
		os_free_mem(NULL, Ssid);
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

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_SYNC - ProbeTimeoutAtJoinAction\n"));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	*pCurrState = SYNC_IDLE;

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

		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}

	return;
}

#endif /* APCLI_SUPPORT */

