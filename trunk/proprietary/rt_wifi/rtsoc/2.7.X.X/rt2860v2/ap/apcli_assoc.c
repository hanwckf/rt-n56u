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
	apcli_assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-6-23		modified for rt61-APClinent
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

#ifdef MAC_REPEATER_SUPPORT
static VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);
#endif /* MAC_REPEATER_SUPPORT */

static VOID ApCliMlmeAssocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliMlmeDisassocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerDisassocAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliAssocTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenAssoc(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenDisassociate(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pAddr2, 
	IN USHORT CapabilityInfo, 
	IN USHORT IfIndex, 
	IN UCHAR SupRate[], 
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen, 
	IN ADD_HT_INFO_IE *pAddHtInfo);

DECLARE_TIMER_FUNCTION(ApCliAssocTimeout);
BUILD_TIMER_FUNCTION(ApCliAssocTimeout);
#ifdef MAC_REPEATER_SUPPORT
DECLARE_TIMER_FUNCTION(ApCliAssocTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAssocTimeoutExt);
#endif /* MAC_REPEATER_SUPPORT */

/*  
    ==========================================================================
    Description: 
        association state machine init, including state transition and timer init
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following 
    ==========================================================================
 */
VOID ApCliAssocStateMachineInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN  STATE_MACHINE *S, 
	OUT STATE_MACHINE_FUNC Trans[]) 
{
	UCHAR i;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR j;
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	StateMachineInit(S, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_ASSOC_STATE, APCLI_MAX_ASSOC_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_ASSOC_IDLE,
		APCLI_ASSOC_MACHINE_BASE);

	/* first column */
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeAssocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDisassocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);
   
	/* second column */
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAssoc);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenDisassociate);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_ASSOC_RSP, (STATE_MACHINE_FUNC)ApCliPeerAssocRspAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_ASSOC_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAssocTimeoutAction);

	for (i=0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].AssocCurrState = APCLI_ASSOC_IDLE;

		/* timer init */
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ApCliAssocTimer,
							GET_TIMER_FUNCTION(ApCliAssocTimeout), pAd, FALSE);

		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.WpaDisassocAndBlockAssocTimer, 
							GET_TIMER_FUNCTION(ApCliWpaDisassocApAndBlockAssoc), pAd, FALSE);

#ifdef MAC_REPEATER_SUPPORT
		for (j = 0; j < MAX_EXT_MAC_ADDR_SIZE; j++)
		{
			pReptCliEntry = &pAd->ApCfg.ApCliTab[i].RepeaterCli[j];

			pReptCliEntry->pAd = pAd;
			pReptCliEntry->MatchApCliIdx = i;
			pReptCliEntry->MatchLinkIdx = j;
			pReptCliEntry->AssocCurrState = APCLI_ASSOC_IDLE;

			/* timer init */
			RTMPInitTimer(pAd, &pReptCliEntry->ApCliAssocTimer, GET_TIMER_FUNCTION(ApCliAssocTimeoutExt), pReptCliEntry, FALSE);
		}
#endif /* MAC_REPEATER_SUPPORT */
	}

	return;
}

/*
    ==========================================================================
    Description:
        Association timeout procedure. After association timeout, this function 
        will be called and it will put a message into the MLME queue
    Parameters:
        Standard timer parameters
    ==========================================================================
 */
static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT \n"));

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_ASSOC_TIMEOUT, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);

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
static VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	USHORT ifIndex = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("Repeater Cli ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT\n"));

	pAd = pRepeaterCliEntry->pAd;
	ifIndex = (64 + (16*pRepeaterCliEntry->MatchApCliIdx) + pRepeaterCliEntry->MatchLinkIdx);

	DBGPRINT(RT_DEBUG_ERROR, (" (%s) ifIndex = %d, CliIdx = %d !!!\n",
					__FUNCTION__, pRepeaterCliEntry->MatchApCliIdx, pRepeaterCliEntry->MatchLinkIdx));

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_ASSOC_TIMEOUT, 0, NULL, ifIndex);
	RTMP_MLME_HANDLER(pAd);

	return;
}
#endif /* MAC_REPEATER_SUPPORT */

/*
    ==========================================================================
    Description:
        mlme assoc req handling procedure
    Parameters:
        Adapter - Adapter pointer
        Elem - MLME Queue Element
    Pre:
        the station has been authenticated and the following information is stored in the config
            -# SSID
            -# supported rates and their length
    Post  :
        -# An association request frame is generated and sent to the air
        -# Association timer starts
        -# Association state -> ASSOC_WAIT_RSP
        
    ==========================================================================
 */
static VOID ApCliMlmeAssocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	NDIS_STATUS		 NStatus;
	BOOLEAN          Cancelled;
	UCHAR            ApAddr[6];
	HEADER_802_11    AssocHdr;
	UCHAR            WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	USHORT           ListenIntv;
	ULONG            Timeout;
	USHORT           CapabilityInfo;
	PUCHAR           pOutBuffer = NULL;
	ULONG            FrameLen = 0;
	ULONG            tmp;
	UCHAR            SsidIe    = IE_SSID;
	UCHAR            SupRateIe = IE_SUPP_RATES;
	UCHAR            ExtRateIe = IE_EXT_SUPP_RATES;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	PAPCLI_STRUCT pApCliEntry = NULL;
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
	USHORT			VarIesOffset = 0;
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */
	UCHAR RSNIe = IE_WPA;
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* Block all authentication request durning WPA block period */
	if (pApCliEntry->bBlockAssoc == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Block Auth request durning WPA block period!\n"));
		*pCurrState = APCLI_ASSOC_IDLE;
		ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}
	else if(MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv))
	{
		//RTMPCancelTimer(&pAd->ApCliMlmeAux.ApCliAssocTimer, &Cancelled);
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].ApCliAssocTimer, &Cancelled);
		else
#endif /* MAC_REPEATER_SUPPORT */
		RTMPCancelTimer(&pApCliEntry->ApCliMlmeAux.ApCliAssocTimer, &Cancelled);

		/* allocate and send out AssocRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
		if (NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("APCLI_ASSOC - ApCliMlmeAssocReqAction() allocate memory failed \n"));
			*pCurrState = APCLI_ASSOC_IDLE;

			ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

			return;
		}

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
		pApCliEntry->AssocInfo.Length = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION); 
		pApCliEntry->AssocInfo.AvailableRequestFixedIEs =
			NDIS_802_11_AI_REQFI_CAPABILITIES | NDIS_802_11_AI_REQFI_LISTENINTERVAL;
		pApCliEntry->AssocInfo.RequestFixedIEs.Capabilities = CapabilityInfo;
		pApCliEntry->AssocInfo.RequestFixedIEs.ListenInterval = ListenIntv;		
		pApCliEntry->AssocInfo.OffsetRequestIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
		
		NdisZeroMemory(pApCliEntry->ReqVarIEs, MAX_VIE_LEN);
		/*First add SSID*/
		VarIesOffset = 0;
		NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, &SsidIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SsidLen, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
		VarIesOffset += pAd->MlmeAux.SsidLen;

		/*Second add Supported rates*/
		NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, &SupRateIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SupRateLen, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen);
		VarIesOffset += pAd->MlmeAux.SupRateLen;
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */
	

		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Send ASSOC request...\n"));
		ApCliMgtMacHeaderInit(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr, ApAddr, ifIndex);

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			COPY_MAC_ADDR(AssocHdr.Addr2, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
#endif /* MAC_REPEATER_SUPPORT */

		/* Build basic frame first */
		MakeOutgoingFrame(pOutBuffer,                          &FrameLen,
						sizeof(HEADER_802_11),                 &AssocHdr,
						2,                                     &CapabilityInfo,
						2,                                     &ListenIntv,
						1,                                     &SsidIe,
						1,                                     &pApCliEntry->ApCliMlmeAux.SsidLen, 
						pApCliEntry->ApCliMlmeAux.SsidLen,     pApCliEntry->ApCliMlmeAux.Ssid,
						1,                                     &SupRateIe,
						1,                                     &pApCliEntry->ApCliMlmeAux.SupRateLen,
						pApCliEntry->ApCliMlmeAux.SupRateLen,  pApCliEntry->ApCliMlmeAux.SupRate,
						END_OF_ARGS);

		if(pApCliEntry->ApCliMlmeAux.ExtRateLen != 0)
		{
			MakeOutgoingFrame(pOutBuffer + FrameLen,               &tmp,
							1,                                     &ExtRateIe,
							1,                                     &pApCliEntry->ApCliMlmeAux.ExtRateLen,
							pApCliEntry->ApCliMlmeAux.ExtRateLen,  pApCliEntry->ApCliMlmeAux.ExtRate,                           
							END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef DOT11_N_SUPPORT
		/* HT */
		if ((pApCliEntry->ApCliMlmeAux.HtCapabilityLen > 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
		{
			ULONG TmpLen;
			HT_CAPABILITY_IE HtCapabilityTmp;

			NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			NdisMoveMemory(&HtCapabilityTmp, &pApCliEntry->ApCliMlmeAux.HtCapability, pApCliEntry->ApCliMlmeAux.HtCapabilityLen);
#ifdef DOT11N_SS3_SUPPORT
			HtCapabilityTmp.MCSSet[2] = (pApCliEntry->ApCliMlmeAux.HtCapability.MCSSet[2] & pApCliEntry->RxMcsSet[2]);
#endif /* DOT11N_SS3_SUPPORT */

#ifdef RT_BIG_ENDIAN
        		*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
        		*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* RT_BIG_ENDINA */
        	MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
        					1,                           &HtCapIe,
        					1,                           &pApCliEntry->ApCliMlmeAux.HtCapabilityLen,
        					pApCliEntry->ApCliMlmeAux.HtCapabilityLen, &HtCapabilityTmp, 
        					END_OF_ARGS);

			FrameLen += TmpLen;
		}

#ifdef DOT11N_DRAFT3
#ifdef APCLI_CERT_SUPPORT
		if (pAd->bApCliCertTest == TRUE)
		{
			ULONG TmpLen;
			EXT_CAP_INFO_ELEMENT extCapInfo;
			UCHAR extInfoLen;

			extInfoLen = sizeof (EXT_CAP_INFO_ELEMENT);
			NdisZeroMemory(&extCapInfo, extInfoLen);


			if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
			    (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			    && (pAd->CommonCfg.Channel <= 14)
			    ) 
			{
				extCapInfo.BssCoexistMgmtSupport = 1;
				DBGPRINT(RT_DEBUG_TRACE, ("%s: BssCoexistMgmtSupport = 1\n", __FUNCTION__));
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
					  1, &ExtCapIe,
					  1, &extInfoLen,
								extInfoLen,			&extCapInfo,
								END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif /* APCLI_CERT_SUPPORT */
#endif /* DOT11N_DRAFT3 */		
#endif /* DOT11_N_SUPPORT */

#ifdef AGGREGATION_SUPPORT
		/*
			add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION
			Case I: (Aggregation + Piggy-Back)
				1. user enable aggregation, AND
				2. Mac support piggy-back
				3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON
			Case II: (Aggregation)
				1. user enable aggregation, AND
				2. AP annouces it's AGGREGATION-capable in BEACON
		*/
		if (pAd->CommonCfg.bAggregationCapable)
		{
#ifdef PIGGYBACK_SUPPORT
			if ((pAd->CommonCfg.bPiggyBackCapable) && ((pApCliEntry->ApCliMlmeAux.APRalinkIe & 0x00000003) == 3))
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00}; 
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			} else
#endif /* PIGGYBACK_SUPPORT */
			if (pApCliEntry->ApCliMlmeAux.APRalinkIe & 0x00000001)
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00}; 
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
		else
		{
			ULONG TmpLen;
			UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00}; 
			MakeOutgoingFrame(pOutBuffer+FrameLen,		 &TmpLen,
							  9,						 RalinkIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif  /* AGGREGATION_SUPPORT */

		if (pApCliEntry->ApCliMlmeAux.APEdcaParm.bValid)
		{
			if (pApCliEntry->UapsdInfo.bAPSDCapable &&
				pApCliEntry->ApCliMlmeAux.APEdcaParm.bAPSDCapable)
			{
				QBSS_STA_INFO_PARM QosInfo;

				NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
				QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
				QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
				QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
				QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
				QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
				WmeIe[8] |= *(PUCHAR)&QosInfo;
			}
			else
			{
                /* The Parameter Set Count is set to ¡§0¡¨ in the association request frames */
                /* WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f); */
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  9,                        &WmeIe[0],
							  END_OF_ARGS);
			FrameLen += tmp;
		}
		/* Append RSN_IE when WPAPSK OR WPA2PSK, */
		if (((pApCliEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || 
            (pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
            || (pApCliEntry->AuthMode >= Ndis802_11AuthModeWPA)
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_AP_SUPPORT
                        && ((pApCliEntry->WscControl.WscConfMode == WSC_DISABLE) ||
                        ((pApCliEntry->WscControl.WscConfMode != WSC_DISABLE) &&
                         !(pApCliEntry->WscControl.bWscTrigger)))
#endif /* WSC_AP_SUPPORT */
            )
		{
			RSNIe = IE_WPA;
			
			if ((pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
				||(pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2)
#endif/*APCLI_WPA_SUPPLICANT_SUPPORT*/
				)
				RSNIe = IE_WPA2;


#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
			if (pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2)
			{
			INT idx;
                BOOLEAN FoundPMK = FALSE;
			/* Search chched PMKID, append it if existed */
				for (idx = 0; idx < PMKID_NO; idx++)
				{
					if (NdisEqualMemory(ApAddr, &pApCliEntry->SavedPMK[idx].BSSID, 6))
					{
						FoundPMK = TRUE;
						break;
					}
				}

				/*
					When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP,
					AP would not do PMK cache with STA after STA re-connect to AP again.
					In this case, driver doesn't need to send PMKID to AP and WpaSupplicant.
				*/
				if ((pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2) &&
					(NdisEqualMemory(pAd->MlmeAux.Bssid, pAd->CommonCfg.LastBssid, MAC_ADDR_LEN)))
				{
					FoundPMK = FALSE;
				}

				if (FoundPMK)
				{
					// Set PMK number
					*(PUSHORT) &pApCliEntry->RSN_IE[pApCliEntry->RSNIE_Len] = 1;
					NdisMoveMemory(&pApCliEntry->RSN_IE[pApCliEntry->RSNIE_Len + 2], &pApCliEntry->SavedPMK[idx].PMKID, 16);
                    pApCliEntry->RSNIE_Len += 18;
				}
			}

#ifdef SIOCSIWGENIE
			if ((pApCliEntry->WpaSupplicantUP & WPA_SUPPLICANT_ENABLE) &&
				(pApCliEntry->bRSN_IE_FromWpaSupplicant == TRUE))			
			{
				;
			}
			else
#endif
#endif /*APCLI_WPA_SUPPLICANT_SUPPORT*/

			MakeOutgoingFrame(pOutBuffer + FrameLen,		&tmp,
			              	1,								&RSNIe,
	                        1,								&pApCliEntry->RSNIE_Len,
	                        pApCliEntry->RSNIE_Len,			pApCliEntry->RSN_IE,
	                        END_OF_ARGS);
			
			FrameLen += tmp;	
		}	


#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
#ifdef SIOCSIWGENIE
			if (((pApCliEntry->WpaSupplicantUP & 0x7F) != WPA_SUPPLICANT_ENABLE) ||
				(pApCliEntry->bRSN_IE_FromWpaSupplicant == FALSE))
#endif
			{
				// Append Variable IE
				NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, &RSNIe, 1);
				VarIesOffset += 1;
				NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, &pApCliEntry->RSNIE_Len, 1);
				VarIesOffset += 1;

				NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, pApCliEntry->RSN_IE, pApCliEntry->RSNIE_Len);
				VarIesOffset += pAd->ApCfg.ApCliTab[ifIndex].RSNIE_Len;

				// Set Variable IEs Length
				pApCliEntry->ReqVarIELen = VarIesOffset;
			}		

#ifdef SIOCSIWGENIE
		if ((pApCliEntry->WpaSupplicantUP & WPA_SUPPLICANT_ENABLE) &&
			(pApCliEntry->bRSN_IE_FromWpaSupplicant == TRUE))			
		{
			ULONG TmpWpaAssocIeLen = 0;
			MakeOutgoingFrame(pOutBuffer + FrameLen,		&TmpWpaAssocIeLen,
	                        pApCliEntry->WpaAssocIeLen,		pApCliEntry->pWpaAssocIe,
	                        END_OF_ARGS);

			FrameLen += TmpWpaAssocIeLen;

			NdisMoveMemory(pApCliEntry->ReqVarIEs + VarIesOffset, pApCliEntry->pWpaAssocIe, pApCliEntry->WpaAssocIeLen);
			VarIesOffset += pApCliEntry->WpaAssocIeLen;

			// Set Variable IEs Length
			pApCliEntry->ReqVarIELen = VarIesOffset;
		}
#endif
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_AP_SUPPORT
		/* Add WSC IE if we are connecting to WSC AP */
		if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
		    (pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger)) 
                {
			UCHAR *pWscBuf = NULL, WscIeLen = 0;
			ULONG WscTmpLen = 0;

			os_alloc_mem(pAd, (UCHAR **) & pWscBuf, 512);
/*			if( (pWscBuf = kmalloc(512, GFP_ATOMIC)) != NULL) */
			if (pWscBuf != NULL) {
				NdisZeroMemory(pWscBuf, 512);
				WscBuildAssocReqIE(&pAd->ApCfg.ApCliTab[ifIndex].WscControl, pWscBuf, &WscIeLen);

				MakeOutgoingFrame(pOutBuffer + FrameLen,
						  &WscTmpLen, WscIeLen, pWscBuf,
						  END_OF_ARGS);

				FrameLen += WscTmpLen;
/*				kfree(pWscBuf); */
				os_free_mem(NULL, pWscBuf);
			} else
				DBGPRINT(RT_DEBUG_WARN,
					 ("%s:: WscBuf Allocate failed!\n",
					  __FUNCTION__));
		}
#endif /* WSC_AP_SUPPORT */

		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			RTMPSetTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].ApCliAssocTimer, Timeout);
		else
#endif /* MAC_REPEATER_SUPPORT */
		RTMPSetTimer(&pApCliEntry->ApCliMlmeAux.ApCliAssocTimer, Timeout);
		*pCurrState = APCLI_ASSOC_WAIT_RSP;
	} 
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeAssocReqAction() sanity check failed. BUG!!!!!! \n"));
		*pCurrState = APCLI_ASSOC_IDLE;

		ApCliCtrlMsg.Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}

	return;
}

/*
    ==========================================================================
    Description:
        Upper layer issues disassoc request
    Parameters:
        Elem -
    ==========================================================================
 */
static VOID ApCliMlmeDisassocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PMLME_DISASSOC_REQ_STRUCT pDisassocReq;
	HEADER_802_11         DisassocHdr;
	PUCHAR                 pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	/* skip sanity check */
	pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);

	/* allocate and send out DeassocReq frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeDisassocReqAction() allocate memory failed\n"));
		*pCurrState = APCLI_ASSOC_IDLE;

		ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = CliIdx;
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Send DISASSOC request [BSSID::%02x:%02x:%02x:%02x:%02x:%02x] \n", 
				pDisassocReq->Addr[0], pDisassocReq->Addr[1], pDisassocReq->Addr[2],
				pDisassocReq->Addr[3], pDisassocReq->Addr[4], pDisassocReq->Addr[5]));
	ApCliMgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr, pDisassocReq->Addr, ifIndex);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
	}
#endif /* MAC_REPEATER_SUPPORT */

	MakeOutgoingFrame(pOutBuffer,				&FrameLen, 
						sizeof(HEADER_802_11),	&DisassocHdr, 
						2,						&pDisassocReq->Reason, 
						END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

    *pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_SUCCESS;

#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
	if (pAd->ApCfg.ApCliTab[ifIndex].WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) 
	{
		/*send disassociate event to wpa_supplicant*/
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
	}
	        RtmpOSWrielessEventSend(pAd->net_dev, SIOCGIWAP, -1, NULL, NULL, 0);     
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, NULL, BSS0, 0); 
#endif /*APCLI_WPA_SUPPLICANT_SUPPORT*/ 
	return;
}


/*
    ==========================================================================
    Description:
        peer sends assoc rsp back
    Parameters:
        Elme - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	BOOLEAN				Cancelled;
	USHORT				CapabilityInfo, Status, Aid;
	UCHAR				SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	UCHAR				ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	UCHAR				Addr2[MAC_ADDR_LEN];
	EDCA_PARM			EdcaParm;
	UCHAR				CkipFlag;
	APCLI_CTRL_MSG_STRUCT	ApCliCtrlMsg;
	HT_CAPABILITY_IE	HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	/* AP might use this additional ht info IE */
	UCHAR				HtCapabilityLen;
	UCHAR				AddHtInfoLen;
	UCHAR				NewExtChannelOffset = 0xff;
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	if (ApCliPeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &CapabilityInfo, &Status, &Aid, SupRate, &SupRateLen, ExtRate, &ExtRateLen, 
		&HtCapability, &AddHtInfo, &HtCapabilityLen,&AddHtInfoLen,&NewExtChannelOffset, &EdcaParm, &CkipFlag))
	{
		/* The frame is for me ? */
		if(MAC_ADDR_EQUAL(Addr2, pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - receive ASSOC_RSP to me (status=%d)\n", Status));
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].ApCliAssocTimer, &Cancelled);
			else
#endif /* MAC_REPEATER_SUPPORT */
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.ApCliAssocTimer, &Cancelled);
			if(Status == MLME_SUCCESS) 
			{
				/* go to procedure listed on page 376 */
#ifdef MAC_REPEATER_SUPPORT
				if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				{
					ApCliAssocPostProc(pAd, Addr2, CapabilityInfo, ifIndex, SupRate, SupRateLen,
										ExtRate, ExtRateLen, &EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo);  	
				}

				ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
				ApCliCtrlMsg.BssIdx = ifIndex;
				ApCliCtrlMsg.CliIdx = CliIdx;
				ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}
			else
			{
				ApCliCtrlMsg.Status = Status;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}

			*pCurrState = APCLI_ASSOC_IDLE;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliPeerAssocRspAction() sanity check fail\n"));
	}

	return;
}

/*
    ==========================================================================
    Description:
        left part of IEEE 802.11/1999 p.374 
    Parameters:
        Elem - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerDisassocAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR         Addr2[MAC_ADDR_LEN];
	USHORT        Reason;
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	if(PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason))
	{
		if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Bssid, Addr2))
		{
			*pCurrState = APCLI_ASSOC_IDLE;

#ifdef MAC_REPEATER_SUPPORT
			ifIndex = (USHORT)(Elem->Priv);

			if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && (ifIndex >= 64))
			{
				ifIndex = ((ifIndex - 64) / 16);
				RTMPRemoveRepeaterDisconnectEntry(pAd, ifIndex, CliIdx);
				RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PEER_DISCONNECT_REQ, 0, NULL, ifIndex);
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliPeerDisassocAction() sanity check fail\n"));
    }
	
	return;
}

/*
    ==========================================================================
    Description:
        what the state machine will do after assoc timeout
    ==========================================================================
 */
static VOID ApCliAssocTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliAssocTimeoutAction\n"));

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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	*pCurrState = APCLI_ASSOC_IDLE;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_REQ_TIMEOUT, 0, NULL, ifIndex);

	return;
}

static VOID ApCliInvalidStateWhenAssoc(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliInvalidStateWhenAssoc(state=%ld), reset APCLI_ASSOC state machine\n", *pCurrState));
	*pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

static VOID ApCliInvalidStateWhenDisassociate(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - InvalidStateWhenApCliDisassoc(state=%ld), reset APCLI_ASSOC state machine\n", *pCurrState));
	*pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;

#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

/*
    ==========================================================================
    Description:
        procedures on IEEE 802.11/1999 p.376 
    Parametrs:
    ==========================================================================
 */
static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pAddr2, 
	IN USHORT CapabilityInfo, 
	IN USHORT IfIndex, 
	IN UCHAR SupRate[], 
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen, 
	IN ADD_HT_INFO_IE *pAddHtInfo)
{
	PAPCLI_STRUCT pApCliEntry = NULL;

	if (IfIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];

	pApCliEntry->ApCliMlmeAux.BssType = BSS_INFRA;	
	pApCliEntry->ApCliMlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	NdisMoveMemory(&pApCliEntry->ApCliMlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));

	/* filter out un-supported rates */
	pApCliEntry->ApCliMlmeAux.SupRateLen = SupRateLen;
	NdisMoveMemory(pApCliEntry->ApCliMlmeAux.SupRate, SupRate, SupRateLen);
    RTMPCheckRates(pAd, pApCliEntry->ApCliMlmeAux.SupRate, &(pApCliEntry->ApCliMlmeAux.SupRateLen));

	/* filter out un-supported rates */
	pApCliEntry->ApCliMlmeAux.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pApCliEntry->ApCliMlmeAux.ExtRate, ExtRate, ExtRateLen);
    RTMPCheckRates(pAd, pApCliEntry->ApCliMlmeAux.ExtRate, &(pApCliEntry->ApCliMlmeAux.ExtRateLen));

	DBGPRINT(RT_DEBUG_TRACE, (HtCapabilityLen ? "%s===> 11n HT STA\n" : "%s===> legacy STA\n", __FUNCTION__));

#ifdef DOT11_N_SUPPORT
	if (HtCapabilityLen > 0 && (pAd->CommonCfg.PhyMode > PHY_11ABGN_MIXED))
	{
		ApCliCheckHt(pAd, IfIndex, pHtCapability, pAddHtInfo);
	}
#endif /* DOT11_N_SUPPORT */

}

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
VOID    ApcliSendAssocIEsToWpaSupplicant( 
    IN  PRTMP_ADAPTER pAd,
    IN UINT ifIndex)
{
	STRING custom[IW_CUSTOM_MAX] = {0};

	if ((pAd->ApCfg.ApCliTab[ifIndex].ReqVarIELen + 17) <= IW_CUSTOM_MAX)
	{
		sprintf(custom, "ASSOCINFO_ReqIEs=");
		NdisMoveMemory(custom+17, pAd->ApCfg.ApCliTab[ifIndex].ReqVarIEs, pAd->ApCfg.ApCliTab[ifIndex].ReqVarIELen);
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_REQIE_EVENT_FLAG, NULL, (PUCHAR)custom, pAd->ApCfg.ApCliTab[ifIndex].ReqVarIELen + 17);
		
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_ASSOCINFO_EVENT_FLAG, NULL, NULL, 0);
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("pAd->ApCfg.ApCliTab[%d].ReqVarIELen + 17 > MAX_CUSTOM_LEN\n",ifIndex));

	return;
}
#endif /*APCLI_WPA_SUPPLICANT_SUPPORT */
#endif /* APCLI_SUPPORT */

