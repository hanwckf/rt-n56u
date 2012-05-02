/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#include "rt_config.h"
#include "dot11r_ft.h"

/*
    ==========================================================================
    Description:
        dls state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the dls state machine
    Note:
        The state machine looks like this
        
                            DLS_IDLE
    MT2_MLME_DLS_REQUEST   MlmeDlsReqAction
    MT2_PEER_DLS_REQUEST   PeerDlsReqAction
    MT2_PEER_DLS_RESPONSE  PeerDlsRspAction
    MT2_MLME_DLS_TEARDOWN  MlmeTearDownAction
    MT2_PEER_DLS_TEARDOWN  PeerTearDownAction
        
	IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID TDLS_StateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
	UCHAR	i;
	
    StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, MAX_TDLS_STATE, MAX_TDLS_MSG, (STATE_MACHINE_FUNC)Drop, TDLS_IDLE, TDLS_MACHINE_BASE);
     
    // the first column
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_SETUP_REQ, (STATE_MACHINE_FUNC)TDLS_MlmeTdlsReqAction);
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_SETUP_REQ, (STATE_MACHINE_FUNC)TDLS_PeerTdlsReqAction);
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_SETUP_RSP, (STATE_MACHINE_FUNC)TDLS_PeerTdlsRspAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_SETUP_CONF,(STATE_MACHINE_FUNC)TDLS_PeerTdlsConfAction);
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_TEAR_DOWN, (STATE_MACHINE_FUNC)TDLS_MlmeTdlsTearDownAction);
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_TEAR_DOWN, (STATE_MACHINE_FUNC)TDLS_PeerTdlsTearDownAction);

	for (i = 0; i < MAX_NUMBER_OF_DLS_ENTRY; i++)
	{
		pAd->StaCfg.TDLSEntry[i].pAd = pAd;
		RTMPInitTimer(pAd, &pAd->StaCfg.TDLSEntry[i].Timer, GET_TIMER_FUNCTION(TDLS_TimeoutAction),  &pAd->StaCfg.TDLSEntry[i], FALSE);
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_TimeoutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRT_802_11_TDLS			pTDLS = (PRT_802_11_TDLS)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("TdlsTimeout - Failed to wait for the response, terminate the setup procedure (%02x:%02x:%02x:%02x:%02x:%02x)\n",
		pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2], pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5]));

	if ((pTDLS) && (pTDLS->Valid) && (pTDLS->Status < TDLS_MODE_CONNECTED))
	{
		pTDLS->Valid	= FALSE;
		pTDLS->Status	= TDLS_MODE_NONE;
	}
		
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeTdlsReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS	pTDLS = NULL;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UINT16			reason;
	BOOLEAN			IsViaAP;

	if (!MlmeTdlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, &pTDLS, &reason, &IsViaAP))
		return;

	DBGPRINT(RT_DEBUG_TRACE,("TDLS - MlmeTdlsSetupReqAction() \n"));

	if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		if ((pAd->StaCfg.WepStatus != Ndis802_11Encryption2Enabled) && (pAd->StaCfg.WepStatus != Ndis802_11Encryption3Enabled))
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - MlmeTdlsSetupReqAction() didn't support NONE- TKIP/AES cipher suite \n"));
			return;
		}	
	}

	/* Build TDLS Setup Request Frame */
	NStatus = TDLS_SetupRequestAction(pAd, pTDLS);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - MlmeTdlsSetupReqAction() Build Setup Request Fail !!!\n"));
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerTdlsReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	USHORT			StatusCode = MLME_SUCCESS;
	SHORT			idx;
	ULONG			Timeout = TDLS_TIMEOUT;
	BOOLEAN			TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;
	RT_802_11_TDLS	TmpTDLS;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	USHORT			CapabilityInfo;
	UCHAR			SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			SupRateLen, ExtRateLen, HtCapLen, ExtCapLen, RsnLen, FTLen, TILen;
	UCHAR			RsnIe[64], FTIe[128], TIIe[7];
	UCHAR			QosCapability;
	HT_CAPABILITY_IE		HtCap;
	EXT_CAP_INFO_ELEMENT	ExtCap;
	BOOLEAN			bDiscard = FALSE;
	BOOLEAN			bWmmCapable;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_PeerTdlsReqAction\n"));

	// Not TDLS Capable, ignore it
	if (!pAd->StaCfg.bTDLSCapable)
		return;
	
	// Not BSS mode, ignore it
	if (!INFRA_ON(pAd))
		return;
		
	// Init all kinds of fields within the packet
	NdisZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
	NdisZeroMemory(&HtCap, sizeof(HtCap));
	NdisZeroMemory(&ExtCap, sizeof(ExtCap));
	NdisZeroMemory(RsnIe, sizeof(RsnIe));
	NdisZeroMemory(FTIe, sizeof(FTIe));
	NdisZeroMemory(TIIe, sizeof(TIIe));


	hex_dump("TDLS setup request receive pack", Elem->Msg, Elem->MsgLen);

	if (!PeerTdlsSetupReqSanity(
							pAd, 
							Elem->Msg, 
							Elem->MsgLen,
							&Token,
							PeerAddr,
							&CapabilityInfo,
							&SupRateLen,	
							&SupRate[0],
							&ExtRateLen,
							&ExtRate[0],
							&bWmmCapable,
							&QosCapability,
							&HtCapLen,
							&HtCap,
							&ExtCapLen,
							&ExtCap,
							&RsnLen,	
							RsnIe,
							&FTLen,
							FTIe,
							&TILen,
							TIIe))
	{
		StatusCode = MLME_REQUEST_DECLINED;
	}

	DBGPRINT(RT_DEBUG_TRACE,("TDLS - PeerTdlsSetupReqAction() received a request from %02x:%02x:%02x:%02x:%02x:%02x\n", PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));

	/* Find table to update parameters. */
	if (StatusCode == MLME_SUCCESS)
	{
		for (idx = MAX_NUM_OF_TDLS_ENTRY - 1; idx >= 0; idx--)
		{
			pTDLS = &pAd->StaCfg.TDLSEntry[idx];

			if (pTDLS->Valid && MAC_ADDR_EQUAL(PeerAddr, pTDLS->MacAddr))
			{
				if (pTDLS->Status == TDLS_MODE_WAIT_RESPONSE)
				{
					if (RTMPCompareMemory(PeerAddr, pAd->CurrentAddress, MAC_ADDR_LEN) == 2)
					{
						/*
							11.20.2 TDLS Link Establishment

							4.	The TDLS setup request frame is received after sending a TDLS Setup Request frame and before
								receiving the corresponding TDLS Setup Response frame, and the source address of the received
								TDLS Setup Request frame is lower than its own MAC address. In this case, the TDLS responder
								STA shall terminate the TDLS setup it initiated. The TDLS responder STA shall send a response
								frame.
						*/
						RTMPCancelTimer(&(pTDLS->Timer), &TimerCancelled);
						pTDLS->Valid = FALSE;
						NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
						pTDLS->Status = TDLS_MODE_NONE;
						//pTDLS = pTmpTDLS;
						DBGPRINT(RT_DEBUG_TRACE,("TDLS - PeerTdlsSetupReqAction() find the same entry \n"));
					}
					else
					{
						/*
							11.20.2 TDLS Link Establishment

							3.	The TDLS setup request is received after sending a TDLS Setup Request frame and before
								receiving the corresponding TDLS Setup Response frame, and the source address of the received
								TDLS Setup Request frame is higher than its own MAC address, in which case the TDLS
								responder STA shall silently discard the message and the TDLS responder STA shall send no
								TDLS Setup Response frame.
						*/
						bDiscard = TRUE;
						DBGPRINT(RT_DEBUG_TRACE,("TDLS - PeerTdlsSetupReqAction() find the same entry and discard it\n"));
					}
				}

				break;
			}
		}

		if (bDiscard == TRUE)
			return;

		/* Can not find in table, create a new one */
		if (idx < 0)
		{
			for (idx = (MAX_NUM_OF_TDLS_ENTRY - 1); idx >= 0; idx--)
			{
				pTDLS = &pAd->StaCfg.TDLSEntry[idx];

				if (!pTDLS->Valid)
				{
					RTMPCancelTimer(&(pTDLS->Timer), &TimerCancelled);
					pTDLS->Valid = TRUE;
					NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
					DBGPRINT(RT_DEBUG_TRACE,("TDLS - PeerTdlsSetupReqAction() create a new entry \n"));
					
					break;
				}
			}
		}


		if (idx < 0)
		{
			// Table full !!!!!
			StatusCode = MLME_REQUEST_DECLINED;
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupReqAction() TDLSEntry table full(only can support %d TDLS session) \n", MAX_NUM_OF_TDLS_ENTRY));
		}
		else if (pTDLS)
		{	
			//
			// Process TPK Handshake Message 1 here!
			//
			if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
				((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
			{
				USHORT Result;

				// RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
				Result = TDLS_TPKMsg1Process(pAd, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen);
				if (Result != MLME_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_TPKMsg1Process() Failed, reason=%d \n", Result));
					if (Result == MLME_REQUEST_DECLINED)	// if mic error , ignore
					{
						return;
					}
					else
					{
						StatusCode = Result;
						goto send_out;
					}
				}

				// Copy SNonce, Key lifetime
				pTDLS->KeyLifetime = le2cpu32(*((PULONG) (TIIe + 3)));
				NdisMoveMemory(pTDLS->SNonce, &FTIe[52], 32);
			}


			//
			// Update temporarliy settings. Try to match Initiator's capabilities
			//
			pTDLS->Token = Token;
			// I am Responder.And peer are Initiator
			pTDLS->bInitiator = TRUE;
			pTDLS->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;

			// Copy Initiator's supported rate and filter out not supported rate
			pTDLS->SupRateLen = SupRateLen;
			NdisMoveMemory(pTDLS->SupRate, SupRate, SupRateLen);
			RTMPCheckRates(pAd, pTDLS->SupRate, &pTDLS->SupRateLen);
			pTDLS->ExtRateLen = ExtRateLen;
			NdisMoveMemory(pTDLS->ExtRate, ExtRate, ExtRateLen);
			RTMPCheckRates(pAd, pTDLS->ExtRate, &pTDLS->ExtRateLen);

			// Filter out un-supported ht rates
			if ((HtCapLen > 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
			{
				// HtCapability carries Responder's capability, so not copy from Initiator here.
				RTMPZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
				pTDLS->HtCapabilityLen = SIZE_HT_CAP_IE;
				NdisMoveMemory(&pTDLS->HtCapability, &HtCap, SIZE_HT_CAP_IE);
			}
			else
			{
				pTDLS->HtCapabilityLen = 0;
				RTMPZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
			}

			// Copy extended capability
			NdisMoveMemory(&pTDLS->TdlsExtCap, &ExtCap, sizeof(EXT_CAP_INFO_ELEMENT));

			// Copy QOS related information
			pTDLS->QosCapability = QosCapability;
			pTDLS->bWmmCapable = bWmmCapable;
		}
	}

send_out:

	if (StatusCode != MLME_SUCCESS)
	{
		NdisZeroMemory(&TmpTDLS, sizeof(RT_802_11_TDLS));
		pTDLS = &TmpTDLS;
		NdisMoveMemory(pTDLS->MacAddr, PeerAddr, MAC_ADDR_LEN);
		pTDLS->Token = Token;
	}

	TDLS_SetupResponseAction(pAd, pTDLS, RsnLen, RsnIe, FTLen, FTIe,  TILen, TIIe, StatusCode);

	if (StatusCode == MLME_SUCCESS)
	{
		/*  Set sendout timer */
		RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
		RTMPSetTimer(&pTDLS->Timer, Timeout);

		/* State Change */
		pTDLS->Status = TDLS_MODE_WAIT_CONFIRM;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_PeerTdlsReqAction\n"));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerTdlsRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	USHORT			StatusCode = MLME_SUCCESS, LocalStatusCode = MLME_SUCCESS;
	BOOLEAN			TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	USHORT			CapabilityInfo;
	UCHAR			SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			SupRateLen, ExtRateLen, HtCapLen, ExtCapLen, RsnLen, FTLen, TILen;
	UCHAR			RsnIe[64], FTIe[128], TIIe[7];
	UCHAR			QosCapability;
	BOOLEAN			bWmmCapable;
	HT_CAPABILITY_IE		HtCap;
	EXT_CAP_INFO_ELEMENT	ExtCap;
	INT						LinkId = 0xff;
	UCHAR					TPK[LEN_PMK], TPKName[LEN_PMK_NAME];
	PMAC_TABLE_ENTRY		pMacEntry = NULL;
	
	// Not TDLS Capable, ignore it
	if (!pAd->StaCfg.bTDLSCapable)
		return;
	
	// Not BSS mode, ignore it
	if (!INFRA_ON(pAd))
		return;

	hex_dump("TDLS setup response receive pack", Elem->Msg, Elem->MsgLen);

	// Init all kinds of fields within the packet
	NdisZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
	NdisZeroMemory(&HtCap, sizeof(HtCap));
	NdisZeroMemory(&ExtCap, sizeof(ExtCap));
	NdisZeroMemory(RsnIe, sizeof(RsnIe));
	NdisZeroMemory(FTIe, sizeof(FTIe));
	NdisZeroMemory(TIIe, sizeof(TIIe));

	if (!PeerTdlsSetupRspSanity(
							pAd, 
							Elem->Msg, 
							Elem->MsgLen,
							&Token,
							PeerAddr,
							&CapabilityInfo,
							&SupRateLen,	
							SupRate,
							&ExtRateLen,
							ExtRate,
							&bWmmCapable,
							&QosCapability,
							&HtCapLen,
							&HtCap,
							&ExtCapLen,
							&ExtCap,
							&StatusCode,
							&RsnLen,	
							RsnIe,
							&FTLen,
							FTIe,
							&TILen,
							TIIe))
	{
		LocalStatusCode = MLME_REQUEST_DECLINED;
	}


	DBGPRINT(RT_DEBUG_TRACE,("TDLS - PeerTdlsSetupRspAction() received a response from %02x:%02x:%02x:%02x:%02x:%02x with StatusCode=%d\n", PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5], StatusCode));

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		return;

	// Point to the current Link ID
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[LinkId];
	// Cancel the timer since the received packet to me.
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

	// Received a error code from the Peer TDLS.
	// Let's terminate the setup procedure right now.
	if (StatusCode != MLME_SUCCESS)
	{
		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid	= FALSE;
		
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupRspAction() received a failed StatusCode, terminate the setup procedure \n"));

		return;
	}
	else
		StatusCode = LocalStatusCode;
	
	// 
	// Validate the content on Setup Response Frame
	//
	while (StatusCode == MLME_SUCCESS)
	{		
		// Invalid TDLS State
		if (pTDLS->Status != TDLS_MODE_WAIT_RESPONSE)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupRspAction() Not in TDLS_MODE_WAIT_RESPONSE STATE\n"));
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}

		// Is the same Dialog Token?
		if (pTDLS->Token != Token)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupRspAction() Not match with Dialig Token my token = %d, peer token = %d\n", pTDLS->Token, Token));
			
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}
		
		// Process TPK Handshake Message 2 here!
		if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
			((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
		{	
			USHORT Result;

			// RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
			Result = TDLS_TPKMsg2Process(pAd, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen, TPK, TPKName);
			if (Result != MLME_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_TPKMsg2Process() Failed, reason=%d \n", Result));
				if (Result == MLME_REQUEST_DECLINED)	// if mic error , ignore
				{
					return;
				}
				else
				{
					StatusCode = Result;
					goto send_out;
				}
			}
			// Copy ANonce, Key lifetime, TPK, TPK Name
			pTDLS->KeyLifetime =  le2cpu32(*((PULONG) (TIIe + 3)));
			NdisMoveMemory(pTDLS->ANonce, &FTIe[20], 32);
				NdisMoveMemory(pTDLS->TPK, TPK, LEN_PMK);
			NdisMoveMemory(pTDLS->TPKName, TPKName, LEN_PMK_NAME);
		}

		// Update parameters
		if (StatusCode == MLME_SUCCESS)
		{
			// I am Initiator. And peer are Responder
			pTDLS->bInitiator = FALSE;
			// Capabilities
			pTDLS->CapabilityInfo = CapabilityInfo;

			pTDLS->SupRateLen = SupRateLen;
			pTDLS->ExtRateLen = ExtRateLen;
			
			// Copy ht capabilities from the Peer TDLS
			if ((HtCapLen > 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
			{
				NdisMoveMemory(&pTDLS->HtCapability, &HtCap, HtCapLen);
				pTDLS->HtCapabilityLen = HtCapLen;				
			}
			else
			{
				pTDLS->HtCapabilityLen = 0;
				RTMPZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
			}

			// Copy extended capability
			NdisMoveMemory(&pTDLS->TdlsExtCap, &ExtCap, sizeof(EXT_CAP_INFO_ELEMENT));

			// Copy QOS related information
			pTDLS->QosCapability = QosCapability;
			pTDLS->bWmmCapable = bWmmCapable;
			
			// Copy TPK related information
			if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
				((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
			{
				// SNonce, Key lifetime
			}

		}

		
		break;
	}

	//
	// Insert into mac table
	//
	if (StatusCode == MLME_SUCCESS)
	{
		// allocate one MAC entry
		pMacEntry = MacTableLookup(pAd, pTDLS->MacAddr);

		if (pMacEntry && IS_ENTRY_TDLS(pMacEntry))
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - MacTable Entry exist !!!\n"));
		else
			pMacEntry = MacTableInsertEntry(pAd, pTDLS->MacAddr, BSS0 + MIN_NET_DEVICE_FOR_TDLS, TRUE);

		if (pMacEntry)
		{
			pTDLS->MacTabMatchWCID = pMacEntry->Aid;
			pMacEntry->AuthMode = pAd->StaCfg.AuthMode;
			pMacEntry->WepStatus = pAd->StaCfg.WepStatus;
			pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
			pMacEntry->Sst = SST_ASSOC;

			DBGPRINT(RT_DEBUG_TRACE, ("MacTableInsertTDlsEntry - allocate entry #%d, Total= %d\n",pMacEntry->Aid, pAd->MacTab.Size));

			// Set WMM capability
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable))
			{
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
				DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  WMM Capable\n"));
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}

			TDLS_InitPeerEntryRateCapability(pAd,
											pMacEntry,
											&CapabilityInfo,
											SupRateLen,
											SupRate,
											HtCapLen,
											&HtCap);

			//
			// Install Peer Key if RSNA Enabled
			//
			if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
				((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
			{	
				// Write to ASIC on-chip table.
				if ( pMacEntry->Aid > 1)
				{
					CIPHER_KEY		PairwiseKey;

					if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
						PairwiseKey.CipherAlg = CIPHER_TKIP;
					else if (pAd->StaCfg.PairCipher == Ndis802_11Encryption3Enabled)
						PairwiseKey.CipherAlg = CIPHER_AES;

					// Set Peer Key
					PairwiseKey.KeyLen = LEN_TK;
					NdisMoveMemory(PairwiseKey.Key, &pTDLS->TPK[16], LEN_TK);

					RTMP_ASIC_PAIRWISE_KEY_TABLE(pAd,
											pMacEntry->Aid,
											&PairwiseKey);															

					RTMP_SET_WCID_SEC_INFO(pAd, 
										BSS0,
										0, 
										PairwiseKey.CipherAlg, 
										pMacEntry->Aid,
										PAIRWISEKEYTABLE);

					NdisMoveMemory(&pMacEntry->PairwiseKey, &PairwiseKey, sizeof(CIPHER_KEY));

					pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}	
				
			}
		}
		else
		{
			StatusCode = MLME_REQUEST_DECLINED;
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupRspAction() MacTableInsertEntry failed\n"));
		}
	}
	else
		StatusCode = MLME_REQUEST_DECLINED;

send_out:	

	if (StatusCode == MLME_SUCCESS)
	{
		TDLS_SetupConfirmAction(pAd, pTDLS, RsnLen, RsnIe, FTLen, FTIe, TILen, TIIe, StatusCode);
		pTDLS->Status = TDLS_MODE_CONNECTED;
		TDLS_SearchEntryDelete(pAd, pTDLS->MacAddr);
	}
	else
	{
		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid	= FALSE;
	}
	
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerTdlsConfAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	USHORT			StatusCode = MLME_SUCCESS, LocalStatusCode = MLME_SUCCESS;
	BOOLEAN			TimerCancelled;	
	PRT_802_11_TDLS	pTDLS = NULL;
	UCHAR			Token;
	UCHAR			PeerAddr[MAC_ADDR_LEN];
	USHORT			CapabilityInfo;
	EDCA_PARM		EdcaParm;
	INT				LinkId = 0xff;
	UCHAR			RsnLen, FTLen, TILen;
	UCHAR			RsnIe[64], FTIe[128], TIIe[7];
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	
	// Not TDLS Capable, ignore it
	if (!pAd->StaCfg.bTDLSCapable)
		return;
	
	// Not BSS mode, ignore it
	if (!INFRA_ON(pAd))
		return;

	hex_dump("TDLS setup confirm receive pack", Elem->Msg, Elem->MsgLen);

	// Init all kinds of fields within the packet
	NdisZeroMemory(&EdcaParm, sizeof(EdcaParm));
	NdisZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
	NdisZeroMemory(RsnIe, sizeof(RsnIe));
	NdisZeroMemory(FTIe, sizeof(FTIe));
	NdisZeroMemory(TIIe, sizeof(TIIe));


	if (!PeerTdlsSetupConfSanity(
							pAd, 
							Elem->Msg, 
							Elem->MsgLen,
							&Token,
							PeerAddr,
							&CapabilityInfo,
							&EdcaParm,
							&StatusCode,
							&RsnLen,	
							RsnIe,
							&FTLen,
							FTIe,
							&TILen,
							TIIe))
	{
		LocalStatusCode = MLME_REQUEST_DECLINED;
	}


	DBGPRINT(RT_DEBUG_TRACE,("TDLS - PeerTdlsSetupConfAction() received a confirm from %02x:%02x:%02x:%02x:%02x:%02x with StatusCode=%d\n", PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5], StatusCode));

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		return;

	// Point to the current Link ID
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[LinkId];
	// Cancel the timer since the received packet to me.
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);



	// Received a error code from the Peer TDLS.
	// Let's terminate the setup procedure right now.
	if (StatusCode != MLME_SUCCESS)
	{
		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid	= FALSE;
		
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupConfAction() received a failed StatusCode, terminate the setup procedure \n"));

		return;
	}
	else
		StatusCode = LocalStatusCode;

	// 
	// Validate the content on Setup Confirm Frame
	//
	while (StatusCode == MLME_SUCCESS)
	{		
		// Invalid TDLS State
		if (pTDLS->Status != TDLS_MODE_WAIT_CONFIRM)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupConfAction() Not in TDLS_MODE_WAIT_CONFIRM STATE\n"));		
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}

		// Is the same Dialog Token?
		if (pTDLS->Token != Token)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupConfAction() Not match with Dialig Token \n"));
			StatusCode =  MLME_REQUEST_DECLINED;
			break;
		}
		
		// Process TPK Handshake Message 3 here!
		if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
		{		
			USHORT Result;

			// RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
			Result = TDLS_TPKMsg3Process(pAd, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen);
			if (Result != MLME_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_ERROR,("TDLS - TPKMsg3Process() Failed, reason=%d \n", Result));
				StatusCode = Result;
				break;
			}
		}

		// Update parameters
		if (StatusCode == MLME_SUCCESS)
		{
			// I am Responder.And peer are Initiator
			pTDLS->bInitiator = TRUE;
			
			// Copy EDCA Parameters
			//if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable))
				//NdisMoveMemory(&pTDLS->EdcaParm, &EdcaParm, sizeof(QBSS_STA_EDCA_PARM));	
			//else
				//NdisZeroMemory(&pTDLS->EdcaParm, sizeof(EDCA_PARM));

			// Copy TPK related information
			if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
				((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
			{
				// SNonce, Key lifetime
			}

		}

		break;
		
	}

	//
	// Insert into mac table
	//
	if (StatusCode == MLME_SUCCESS)
	{
		// allocate one MAC entry
		pMacEntry = MacTableLookup(pAd, pTDLS->MacAddr);
		if (pMacEntry && IS_ENTRY_TDLS(pMacEntry))
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - MacTable Entry exist !!!\n"));
		else
			pMacEntry = MacTableInsertEntry(pAd, pTDLS->MacAddr, BSS0 + MIN_NET_DEVICE_FOR_TDLS, TRUE);

		if (pMacEntry)
		{
			pTDLS->MacTabMatchWCID = pMacEntry->Aid;
			pMacEntry->AuthMode = pAd->StaCfg.AuthMode;
			pMacEntry->WepStatus = pAd->StaCfg.WepStatus;
			pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
			pMacEntry->Sst = SST_ASSOC;

			DBGPRINT(RT_DEBUG_TRACE, ("MacTableInsertTDlsEntry - allocate entry #%d, Total= %d\n",pMacEntry->Aid, pAd->MacTab.Size));

			// Set WMM capability
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable))
			{
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
				DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  WMM Capable\n"));
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}

			TDLS_InitPeerEntryRateCapability(pAd,
											pMacEntry,
											&pTDLS->CapabilityInfo,
											pTDLS->SupRateLen,
											pTDLS->SupRate,
											pTDLS->HtCapabilityLen,
											&pTDLS->HtCapability);

			//
			// Install Peer Key if RSNA Enabled
			//
			if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
				((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
			{	
				// Write to ASIC on-chip table.
				if ( pMacEntry->Aid > 1)
				{
					CIPHER_KEY		PairwiseKey;

					if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
						PairwiseKey.CipherAlg = CIPHER_TKIP;
					else if (pAd->StaCfg.PairCipher == Ndis802_11Encryption3Enabled)
						PairwiseKey.CipherAlg = CIPHER_AES;

					// Set Peer Key
					PairwiseKey.KeyLen = LEN_TK;
					NdisMoveMemory(PairwiseKey.Key, &pTDLS->TPK[16], LEN_TK);

					RTMP_ASIC_PAIRWISE_KEY_TABLE(pAd,
											pMacEntry->Aid,
											&PairwiseKey);															

					RTMP_SET_WCID_SEC_INFO(pAd, 
										BSS0,
										0, 
										PairwiseKey.CipherAlg, 
										pMacEntry->Aid,
										PAIRWISEKEYTABLE);

					NdisMoveMemory(&pMacEntry->PairwiseKey, &PairwiseKey, sizeof(CIPHER_KEY));

					pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}	
				
			}

			pTDLS->Status = TDLS_MODE_CONNECTED;
			TDLS_SearchEntryDelete(pAd, pTDLS->MacAddr);
		}
		else
		{
			StatusCode = MLME_REQUEST_DECLINED;
			pTDLS->Status = TDLS_MODE_NONE;
			pTDLS->Valid = FALSE;
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsSetupRspAction() MacTableInsertEntry failed\n"));
		}
	}
	else
	{
		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid = FALSE;
		StatusCode = MLME_REQUEST_DECLINED;
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeTdlsTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	PRT_802_11_TDLS	pTDLS = NULL;
	UINT16			ReasonCode;
	BOOLEAN			IsViaAP = FALSE;

	DBGPRINT(RT_DEBUG_TRACE,("TDLS - TDLS_MlmeTdlsTearDownAction() \n"));

	if (!MlmeTdlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, &pTDLS, &ReasonCode, &IsViaAP))
		return;

	/* Build TDLS Setup Request Frame */

	NStatus = TDLS_TearDownAction(pAd, pTDLS, ReasonCode,TRUE);

	if (NStatus	!= NDIS_STATUS_SUCCESS)	
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeTdlsTearDownAction() Build Setup Request Fail !!!\n"));
	}
	else
	{
		if (!VALID_WCID(pTDLS->MacTabMatchWCID))
			return;

		MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, pTDLS->MacAddr);
	}

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerTdlsTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR				SA[MAC_ADDR_LEN];
	USHORT				ReasonCode;
	PRT_802_11_TDLS		pTDLS = NULL;
	INT					LinkId = 0xff;
	BOOLEAN				IsInitator;
	//UINT				i;
	BOOLEAN				TimerCancelled;
	UCHAR				FTLen;
	UCHAR				FTIe[128];
	UCHAR	TdlsZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	// Not TDLS Capable, ignore it
	if (!pAd->StaCfg.bTDLSCapable)
		return;

	if (!INFRA_ON(pAd))
		return;

	// Init FTIe
	NdisZeroMemory(FTIe, sizeof(FTIe));
	
	if (!PeerTdlsTearDownSanity(pAd, Elem->Msg, Elem->MsgLen, SA, &IsInitator, &ReasonCode, &FTLen, FTIe))
		return;

	DBGPRINT(RT_DEBUG_TRACE,("TDLS - PeerTdlsTearDownAction() from %02x:%02x:%02x:%02x:%02x:%02x with ReasonCode=%d\n", SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], ReasonCode));

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, SA);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		return;
	
	// Point to the current Link ID
	pTDLS = &pAd->StaCfg.TDLSEntry[LinkId];

	// Cancel the timer since the received packet to me.
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

	// Drop mismatched identifier.
	if (pTDLS->bInitiator != IsInitator)
		return;

	// Process FTIE here!
	if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
		((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
	{	
		FT_FTIE		*ft = NULL;
		UCHAR		oldMic[16];
		UCHAR		LinkIdentifier[20];
		UCHAR		content[256];
		ULONG		c_len = 0;
		ULONG		tmp_len = 0;
		UCHAR		seq = 4;
		UCHAR		mic[16];
		UINT	tmp_aes_len = 0;
		
		// FTIE (7.3.2.48)
		// It's the same as that included in TPK Handshake Message3 with the exception of MIC field.

		// Validate FTIE and its MIC
		//

		// point to the element of IE
		ft = (PFT_FTIE)(FTIe + 2); 
		
		if ((FTLen != (sizeof(FT_FTIE) + 2)) || RTMPEqualMemory(&ft->MICCtr, TdlsZeroSsid, 2) == 0 || 
			(RTMPEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0) || (RTMPEqualMemory(ft->ANonce, pTDLS->ANonce, 32) == 0))
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsTearDownAction() Invalid FTIE, drop the teardown frame\n"));
			return;
		}
		
		// backup MIC from the peer TDLS
		NdisMoveMemory(oldMic, ft->MIC, 16);

		
		// set MIC field to zero before MIC calculation
		NdisZeroMemory(ft->MIC, 16);

		// Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
		NdisZeroMemory(LinkIdentifier, 20);
		LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
		LinkIdentifier[1] = 18;
		NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
		if (pTDLS->bInitiator)
		{
			NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
			NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);
		}
		else
		{
			NdisMoveMemory(&LinkIdentifier[8], pAd->CurrentAddress, 6);
			NdisMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);
		}

		////////////////////////////////////////////////////////////////////////
		// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
		// 1. Link Identifier (20 bytes)
		// 2. Reason Code (2 bytes)
		// 3. Dialog token (1 byte)
		// 4. Transaction Sequence = 4 (1 byte)
		// 5. FTIE with the MIC field of FTIE set to zero (84 bytes)	
		
		/* concatenate Link Identifier, Reason Code, Dialog token, Transaction Sequence */
		MakeOutgoingFrame(content,            		&tmp_len,
						sizeof(LinkIdentifier),		LinkIdentifier,	
						2,							&ReasonCode,
						1,							&pTDLS->Token,
						1,							&seq,
						END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate FTIE */
		MakeOutgoingFrame(content + c_len,		&tmp_len,
						FTIe[1] + 2,			FTIe,  
						END_OF_ARGS);
		c_len += tmp_len;					
		
		/* Calculate MIC */
		NdisZeroMemory(mic, sizeof(mic));
		//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

		/* Compute AES-128-CMAC over the concatenation */
		tmp_aes_len = AES_KEY128_LENGTH;
    	AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);


		////////////////////////////////////////////////////////////////////////

		if (RTMPEqualMemory(oldMic, mic, 16) == 0)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - PeerTdlsTearDownAction() MIC Error, drop the teardown frame\n"));
			return;
		}

	}
	

	// clear tdls table entry
	if (pTDLS->Valid && MAC_ADDR_EQUAL(SA, pTDLS->MacAddr))
	{
		pTDLS->Status = TDLS_MODE_NONE;
		pTDLS->Valid = FALSE;
		RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

		MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, SA);
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsSetupReqSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pTdlsExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pTdlsExtCap,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	UCHAR			Sanity;
	ULONG			Length = 0;

	// Init output parameters
	*pSupRateLen = 0;
	*pExtRateLen = 0;
	*pCapabilityInfo = 0;
	*pHtCapLen = 0;
	*pTdlsExtCapLen = 0;
	*pbWmmCapable = FALSE;
	*pQosCapability = 0; // default: no IE_QOS_CAPABILITY found
	*pRsnLen = 0;
	*pFTLen = 0;
	*pTILen = 0;
	
	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + 3)) 
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (action header) \n"));
		return FALSE;	
	}

	// Offset to Dialog Token
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 3);

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (dialog token) \n"));
		return FALSE;
	}	
	*pToken = *Ptr;
	
	// Offset to Link Identifier
	Ptr += 1;
	RemainLen -= 1;

	// Get BSSID, SA and DA from payload and advance the pointer
	if (RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_IDENTIFIER || Ptr[1] != 18)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (link identifier) \n"));
		return FALSE;
	}
	if (!MAC_ADDR_EQUAL(Ptr+2, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> It's not my BSSID\n"));
		return FALSE;
	}	
	else if (!MAC_ADDR_EQUAL(Ptr+14, pAd->CurrentAddress))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> It's not my MAC address\n"));
		return FALSE;
	}	

	NdisMoveMemory(pSA, Ptr+8, MAC_ADDR_LEN);

	// Offset to Capability
	Ptr += 20;
	RemainLen -= 20;

	// Get capability info from payload and advance the pointer
	if (RemainLen < 2) 
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity --> Invaild packet length - (capability) \n"));
		return FALSE;
	}	
	NdisMoveMemory((PUCHAR)pCapabilityInfo, Ptr, 2);

	// Offset to other elements
	Ptr += 2;
	RemainLen -= 2;

	// Add for 2 necessary EID field check
	Sanity = 0;
	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_SSID:
				if(pEid->Len == pAd->CommonCfg.SsidLen && NdisEqualMemory(pEid->Octet, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen))
					Sanity |= 0x1;
				else
					return FALSE;
				break;
				
			case IE_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					Sanity |= 0x2;
					NdisMoveMemory(SupRate, pEid->Octet, pEid->Len);
					*pSupRateLen = pEid->Len;
				}
				else
					return FALSE;

				break;
			case IE_EXT_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
					*pExtRateLen = pEid->Len;
				}
				break;

			case IE_POWER_CAPABILITY:
				break;
				
			case IE_QOS_CAPABILITY:
				if (pEid->Len == 1)
				{
					*pbWmmCapable = TRUE;
					*pQosCapability = *(pEid->Octet);
				}
				
				break;

			case IE_HT_CAP:
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
				{
					if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
					{
						NdisMoveMemory(pHtCap, pEid->Octet, sizeof(HT_CAPABILITY_IE));
						*pHtCapLen = SIZE_HT_CAP_IE;	// Nnow we only support 26 bytes.
					}
				}
				break;
				
			case IE_2040_BSS_COEXIST:
				break;

			case IE_EXT_CAPABILITY:
				if (pEid->Len >= sizeof(EXT_CAP_INFO_ELEMENT))
				{
					NdisMoveMemory(pTdlsExtCap, pEid->Octet, 1);
					*pTdlsExtCapLen = pEid->Len;
				}
				break;

			case IE_RSN:
				if ((pEid->Len + 2) < 64)
				{
					NdisMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
					*pRsnLen = pEid->Len + 2;
				}
				break;

			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}	
				break;
				
			case IE_FT_TIMEOUT_INTERVAL:
				if ((pEid->Len + 2) == 7)
				{
					NdisMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
					*pTILen = pEid->Len + 2;
				}
				break;
				
			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}


	if ((Sanity&0x7) != 0x3)
	{
		DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity - missing field, Sanity=0x%02x\n", Sanity));
		return FALSE;
	}
	else
	{
		// Process in succeed
	    return TRUE;
	}

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsSetupRspSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pTdlsExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pTdlsExtCap,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	UCHAR			Sanity;
	ULONG			Length = 0;	

	// Init output parameters
	*pSupRateLen = 0;
	*pExtRateLen = 0;
	*pCapabilityInfo = 0;
	*pHtCapLen = 0;
	*pTdlsExtCapLen = 0;
	*pbWmmCapable = FALSE;
	*pQosCapability= 0; // default: no IE_QOS_CAPABILITY found
	*pStatusCode = MLME_SUCCESS;
	*pRsnLen = 0;
	*pFTLen = 0;
	*pTILen = 0;

	
	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + 3))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (action header) \n"));
		return FALSE;	
	}
	// Offset to Status Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	
	// Get the value of Status Code from payload and advance the pointer
	if (RemainLen < 2)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (status code) \n"));
		return FALSE;
	}	
	*pStatusCode = (*((PUSHORT) Ptr));

	// Offset to Dialog Token
	Ptr	+= 2;
	RemainLen -= 2;

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (dialog token) \n"));
		return FALSE;
	}	
	*pToken = *Ptr;

	if (*pStatusCode != MLME_SUCCESS)
		return TRUE;	// in the end of Setup Response frame
		
	// Offset to Link Identifier
	Ptr += 1;
	RemainLen -= 1;

	// Get BSSID, SA and DA from payload and advance the pointer
	if (RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_IDENTIFIER || Ptr[1] != 18) 
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> Invaild packet length - (link identifier) \n"));
		return FALSE;
	}
	if (!MAC_ADDR_EQUAL(Ptr+2, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> It's not my BSSID\n"));
		return FALSE;
	}	
	else if (!MAC_ADDR_EQUAL(Ptr+8, pAd->CurrentAddress))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity --> It's not my MAC address\n"));
		return FALSE;
	}	

	NdisMoveMemory(pSA, Ptr+14, MAC_ADDR_LEN);

	// Offset to Capability
	Ptr += 20;
	RemainLen -= 20;

	// Get capability info from payload and advance the pointer
	if (RemainLen < 2) 
		return FALSE;
	NdisMoveMemory(pCapabilityInfo, Ptr, 2);

	// Offset to other elements
	Ptr += 2;
	RemainLen -= 2;


	// Add for 2 necessary EID field check
	Sanity = 0;
	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_SSID:
				if (pEid->Len == pAd->CommonCfg.SsidLen && NdisEqualMemory(pEid->Octet, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen))
					Sanity |= 0x1;
				else
					return FALSE;
				break;
				
			case IE_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					Sanity |= 0x2;
					NdisMoveMemory(SupRate, pEid->Octet, pEid->Len);
					*pSupRateLen = pEid->Len;
				}
				else
					return FALSE;

				break;
			case IE_EXT_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
					*pExtRateLen = pEid->Len;
				}
				break;

			case IE_POWER_CAPABILITY:
				break;
				
			case IE_QOS_CAPABILITY:
				if (pEid->Len ==  1)
				{
					*pQosCapability = *(pEid->Octet);
					*pbWmmCapable = TRUE;
				}
				break;

			case IE_HT_CAP:
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
				{
					if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
					{
						NdisMoveMemory(pHtCap, pEid->Octet, sizeof(HT_CAPABILITY_IE));
						*pHtCapLen = SIZE_HT_CAP_IE;	// Nnow we only support 26 bytes.
					}
				}
				
				break;
				
			case IE_2040_BSS_COEXIST:
				break;

			case IE_EXT_CAPABILITY:
				if (pEid->Len == 1)
				{
					*pTdlsExtCapLen = pEid->Len;
					NdisMoveMemory(pTdlsExtCap, pEid->Octet, sizeof(EXT_CAP_INFO_ELEMENT));
				}
				break;

			case IE_RSN:
				if ((pEid->Len + 2) < 64)
				{
					NdisMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
					*pRsnLen = pEid->Len + 2;
				}
				break;

			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}	
				break;
				
			case IE_FT_TIMEOUT_INTERVAL:
				if ((pEid->Len + 2) == 7)
				{
					NdisMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
					*pTILen = pEid->Len + 2;
				}
				break;
				
			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}


	if ((Sanity&0x7) != 0x3)
	{
		DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity - missing field, Sanity=0x%02x\n", Sanity));
		return FALSE;
	}
	else
	{
		// Process in succeed
		*pStatusCode = MLME_SUCCESS;
	    return TRUE;
	}

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsSetupConfSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT EDCA_PARM	*pEdcaParm,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	ULONG			Length = 0;	

	// Init output parameters
	*pCapabilityInfo = 0;
	*pStatusCode = MLME_REQUEST_DECLINED;
	 //pEdcaParm = 0;      // default: no IE_EDCA_PARAMETER found
	*pRsnLen = 0;
	*pFTLen = 0;
	*pTILen = 0;
	
	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + 3))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (action header) \n"));
		return FALSE;	
	}
	// Offset to Status Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	
	// Get the value of Status Code from payload and advance the pointer
	if (RemainLen < 2)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (status code) \n"));
		return FALSE;
	}	
	*pStatusCode = (*((PUSHORT) Ptr));


	// Offset to Dialog Token
	Ptr	+= 2;
	RemainLen -= 2;

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (dialog token) \n"));
		return FALSE;
	}

	*pToken = *Ptr;

	if (*pStatusCode != MLME_SUCCESS)
		return TRUE;	// end of Setup Response frame
		
	// Offset to Link Identifier
	Ptr += 1;
	RemainLen -= 1;

	// Get BSSID, SA and DA from payload and advance the pointer
	if (RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_IDENTIFIER || Ptr[1] != 18)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> Invaild packet length - (link identifier) \n"));
		return FALSE;
	}

	if (!MAC_ADDR_EQUAL(Ptr+2, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> It's not my BSSID\n"));
		return FALSE;
	}	
	else if (!MAC_ADDR_EQUAL(Ptr+14, pAd->CurrentAddress))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity --> It's not my MAC address\n"));
		return FALSE;
	}	

	NdisMoveMemory(pSA, Ptr+8, MAC_ADDR_LEN);

	// Offset to other elements
	Ptr += 20;
	RemainLen -= 20;

	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_EDCA_PARAMETER:
				//if (pEid->Len == 18)
					//NdisMoveMemory(pEdcaParm, pEid->Octet, sizeof(QBSS_STA_EDCA_PARM));
				break;

			case IE_VENDOR_SPECIFIC:
				// handle WME PARAMTER ELEMENT
				if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
				{
					PUCHAR ptr;
					int i;

					// parsing EDCA parameters
					pEdcaParm->bValid		   = TRUE;
					pEdcaParm->bQAck		   = FALSE; // pEid->Octet[0] & 0x10;
					pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
					pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
					//pEdcaParm->bMoreDataAck	 = FALSE; // pEid->Octet[0] & 0x80;
					pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
					pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
					ptr = &pEid->Octet[8];
					for (i=0; i<4; i++)
					{
						UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
						pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);	 // b5 is ACM
						pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;				 // b0~3 is AIFSN
						pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;			 // b0~4 is Cwmin
						pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;				 // b5~8 is Cwmax
						pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
						ptr += 4; // point to next AC
					}
				}
				break;

			case IE_RSN:
				if ((pEid->Len + 2) < 64)
				{
					NdisMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
					*pRsnLen = pEid->Len + 2;
				}
				break;
				
			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}	
				break;
				
			case IE_FT_TIMEOUT_INTERVAL:
				if ((pEid->Len + 2) == 7)
				{
					NdisMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
					*pTILen = pEid->Len + 2;
				}
				break;
				
			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}


	// Process in succeed
	*pStatusCode = MLME_SUCCESS;
    return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsTearDownSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
	OUT UCHAR	*pSA,
	OUT	BOOLEAN *pIsInitator,
    OUT USHORT *pReasonCode,
	OUT UCHAR	*pFTLen,    
	OUT UCHAR	FTIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	//ULONG			Length = 0;	

	// Init output parameters
	*pReasonCode = 0;
	*pFTLen = 0 ;

	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + 3))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> Invaild packet length - (cation header) \n"));
		return FALSE;	
	}
	// Offset to Reason Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	
	// Get the value of Reason Code from payload and advance the pointer
	if (RemainLen < 2) 
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> Invaild packet length - (reason code) \n"));
		return FALSE;	
	}
	*pReasonCode = (*((PUSHORT) Ptr));


	// Offset to Link Identifier
	Ptr += 2;
	RemainLen -= 2;

	// Get BSSID, SA and DA from payload and advance the pointer
	if (RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_IDENTIFIER || Ptr[1] != 18) 
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> Invaild packet length - (link identifier) \n"));
		return FALSE;	
	}
	// It's not my BSSID
	if (!MAC_ADDR_EQUAL(Ptr+2, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity --> It's not my BSSID\n"));
		return FALSE;
	}	

	// Check if my MAC address and then find out SA
	if (!MAC_ADDR_EQUAL(pAd->CurrentAddress, Ptr+8))
	{
		if (!MAC_ADDR_EQUAL(pAd->CurrentAddress, Ptr+14))
			return FALSE;
		else
		{
			*pIsInitator = TRUE;	// peer are Initator.
			NdisMoveMemory(pSA, Ptr+8, MAC_ADDR_LEN);
		}
	}
	else
	{
		*pIsInitator = FALSE;	// peer are not Initator.
		NdisMoveMemory(pSA, Ptr+14, MAC_ADDR_LEN);
	}
	
	
	// Offset to other elements
	Ptr += 20;
	RemainLen -= 20;

	pEid = (PEID_STRUCT) Ptr;

	// TPK Handshake if RSNA Enabled
	if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		if(RemainLen < ((UCHAR)(pEid->Len + 2)) || (pEid->Eid != IE_FT_FTIE))
			return FALSE;

		if ((pEid->Len) == sizeof(FT_FTIE))
		{
			NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
			*pFTLen = pEid->Len + 2;
		}		
	}

    return TRUE;
}
#endif // DOT11Z_TDLS_SUPPORT //

