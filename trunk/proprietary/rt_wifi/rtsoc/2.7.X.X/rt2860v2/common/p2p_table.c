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
	p2p.c

	Abstract:
	Peer to peer is also called Wifi Direct. This function handles P2P table management. also include persistent table that saves credential.

	Revision History:
	Who              When               What
	--------    ----------    ----------------------------------------------
	Jan Lee         2010-05-21    created for Peer-to-Peer Action frame(Wifi Direct)
*/
#include "rt_config.h"

extern UCHAR	ZeroSsid[];

/*	
	==========================================================================
	Description: 
		GUI needs to update whole Persistent table to driver after reload of driver. 
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2pSetPerstTable(
	IN PRTMP_ADAPTER pAd, 
	IN PVOID pInformationBuffer) 
{
	POID_P2P_PERSISTENT_TABLE		pP2pPerstTab;
	UCHAR		i;
	
	pP2pPerstTab = (POID_P2P_PERSISTENT_TABLE)pInformationBuffer;
	DBGPRINT(RT_DEBUG_TRACE,("P2pSetPerstTable   Num = %d \n", pP2pPerstTab->PerstNumber));
	pAd->P2pTable.PerstNumber = pP2pPerstTab->PerstNumber;
	
	if (pAd->P2pTable.PerstNumber == 0)
	{
		for (i = 0; i < MAX_P2P_TABLE_SIZE;i++)
		{
			pAd->P2pTable.PerstEntry[i].bValid = FALSE;
		}
	}
	
	for (i = 0; i < 1; i++)
	{
		RTMPMoveMemory(&pAd->P2pTable.PerstEntry[i], &pP2pPerstTab->PerstEntry[i], sizeof(RT_P2P_PERSISTENT_ENTRY));
		P2PPrintP2PPerstEntry(pAd, i );
	}

}

/*	
	==========================================================================
	Description: 
		This is a periodic routine that check P2P Group Table's Status. One importatn task is to check if some frame
		that need transmission result is success or retry fail.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pGroupMaintain(
	IN PRTMP_ADAPTER pAd)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	UCHAR		i;
	PRT_P2P_CLIENT_ENTRY		pP2pEntry;
	ULONG		Data;
	/*UCHAR		Value;*/
	/*BCN_TIME_CFG_STRUC csr;*/
	BOOLEAN		bAllPsm = TRUE;

	if (pP2PCtrl->GONoASchedule.bValid == TRUE)
	{
		/* Disable OppPS when NoA is ON. */
		P2pStopOpPS(pAd);
		RTMP_IO_READ32(pAd, TSF_TIMER_DW1, &Data);
		if (Data != pP2PCtrl->GONoASchedule.TsfHighByte)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("P2pGroupMaintain. Tsf MSB changed to %ld from %ld.  restart NoA . \n",Data, pP2PCtrl->GONoASchedule.TsfHighByte ));
			/* I want to resume the NoA */
			pP2PCtrl->GONoASchedule.bNeedResumeNoA = TRUE;
			P2pStopNoA(pAd, NULL);
			/* Ok. Now resume it. */
			pP2PCtrl->GONoASchedule.bNeedResumeNoA = FALSE;
			P2pGOStartNoA(pAd);
		}

	}
	else if ((P2P_GO_ON(pAd)) && (pP2PCtrl->GONoASchedule.bValid == FALSE)
		&& (IS_OPPS_ON(pAd)))
	{
		/* Since NoA is OFF, consider to enable OppPS. */
		for (i = 0; i < MAX_LEN_OF_MAC_TABLE;i++)
		{
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry)
				&& (pEntry->PsMode == PWR_ACTIVE))
			{
				bAllPsm = FALSE;
				break;
			}
		}
		if ((bAllPsm == TRUE) && (pAd->MacTab.Size > 0))
		{
			/* bit 7 is OppPS bit. set 1 to enable. bit [0:6] is in unit TU. */
			if (IS_OPPS_ON(pAd))
			{
				P2pStartOpPS(pAd);
				pP2PCtrl->CTWindows = 0x8a;
			}
			/* case 2 to turn on CTWindows.  Not decide the case 2 rule yet. 2010-June */
			else if (0)
			{
				pP2PCtrl->CTWindows = 0x8a;
			}
		}
		else if (P2P_TEST_BIT(pAd->P2pCfg.CTWindows, P2P_OPPS_BIT))
		{
			P2pStopOpPS(pAd);
		}
	}

	if (pP2PCtrl->p2pidxForServiceCbReq < MAX_P2P_GROUP_SIZE)
	{
		if (pAd->P2pTable.Client[pAd->P2pCfg.p2pidxForServiceCbReq].ConfigTimeOut > 0)
			pAd->P2pTable.Client[pAd->P2pCfg.p2pidxForServiceCbReq].ConfigTimeOut--;
		if (pAd->P2pTable.Client[pAd->P2pCfg.p2pidxForServiceCbReq].P2pClientState == P2PSTATE_SERVICE_COMEBACK_COMMAND
			&& (pAd->P2pTable.Client[pAd->P2pCfg.p2pidxForServiceCbReq].ConfigTimeOut == 0))
		{
			/*P2pSendComebackReq(pAd, pAd->P2pCfg.p2pidxForServiceCbReq, pAd->P2pTable.Client[pAd->P2pCfg.p2pidxForServiceCbReq].addr); */
			pP2PCtrl->p2pidxForServiceCbReq = MAX_P2P_GROUP_SIZE;
		}
	}

	if (IS_PERSISTENT_ON(pAd) 
		&& (!P2P_GO_ON(pAd))
		&& (!P2P_CLI_ON(pAd)))
	{
		for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
		{		
			pP2pEntry = &pAd->P2pTable.Client[i];
			/* Add some delay to connect to Persistent GO. Because some GO like broadcom need configuration time to start GO. */
			if ((pP2pEntry->P2pClientState == P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME))
			{
				if (pP2pEntry->ConfigTimeOut > 0)
					pP2pEntry->ConfigTimeOut--;
				if (pP2pEntry->ConfigTimeOut == 0)
				{
					pP2PCtrl->P2PConnectState = P2P_DO_WPS_ENROLLEE;
					pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
					P2pWpsDone(pAd, pP2pEntry->addr);
				}
			}
		}
	}

	if (P2P_GO_ON(pAd))
	{
		for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
		{		
			pP2pEntry = &pAd->P2pTable.Client[i];
			if (pP2pEntry->P2pClientState == P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS)
			{
				ULONG		TotalFrameLen;
				DBGPRINT(RT_DEBUG_TRACE,("P2P  P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS \n"));

				P2PSendDevDisRsp(pAd, P2PSTATUS_SUCCESS, pAd->P2pCfg.LatestP2pPublicFrame.Token, pAd->P2pCfg.LatestP2pPublicFrame.p80211Header.Addr2, &TotalFrameLen);
				pP2pEntry->P2pClientState = P2PSTATE_CLIENT_OPERATING;
			}
			else if ((pP2pEntry->P2pClientState == P2PSTATE_PROVISION_COMMAND) || (pP2pEntry->P2pClientState == P2PSTATE_INVITE_COMMAND))
			{
				if (pP2pEntry->StateCount > 0)
				{
					/*DBGPRINT(RT_DEBUG_ERROR, ("pEntry[%d] StateCount = %d\n", i, pP2pEntry->StateCount)); */
					pP2pEntry->StateCount--;
				}
			
				if ((pP2pEntry->StateCount == 0) && (pP2pEntry->bValid))
				{
					if (pP2pEntry->ReTransmitCnt >= 20)
					{
						DBGPRINT(RT_DEBUG_ERROR, ("%s:: ReTransmit Probe Req. limit! stop connect this p2p device!\n",
								__FUNCTION__));
						P2pLinkDown(pAd, P2P_DISCONNECTED);
						pP2pEntry->ReTransmitCnt = 0;
					}
					pP2pEntry->ReTransmitCnt++;
					DBGPRINT(RT_DEBUG_TRACE, ("P2P Table : idx=%d Send Probe Req. \n", i));
					P2pSendProbeReq(pAd, pP2pEntry->ListenChannel);
				}
			}
		}
			}
	/* time out case. */
	else if ((pP2PCtrl->P2PConnectState ==  P2P_DO_GO_SCAN_BEGIN)
		&& (pP2PCtrl->P2pCounter.GoScanBeginCounter100ms > 1200 /*GOSCANBEGINCOUNTER_MAX*/))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("P2P_DO_GO_SCAN_BEGIN Timeout. BAck to idle. \n"));
		pP2PCtrl->P2PConnectState = P2P_CONNECT_IDLE;
		}
	else if ((pAd->flg_p2p_OpStatusFlags == 0) &&
			(!MAC_ADDR_EQUAL(&ZERO_MAC_ADDR, &pP2PCtrl->ConnectingMAC)))
	{
		for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
		{		
			pP2pEntry = &pAd->P2pTable.Client[i];
			if (pP2pEntry->P2pClientState == P2PSTATE_NONE)
				continue;
			if ((pP2pEntry->P2pClientState >= P2PSTATE_SENT_GO_NEG_REQ) && 
				(pP2pEntry->P2pClientState <= P2PSTATE_WAIT_GO_COMFIRM_ACK))
			{
				if (pP2pEntry->StateCount > 0)
				{
					/*DBGPRINT(RT_DEBUG_ERROR, ("pEntry[%d] StateCount = %d\n", i, pP2pEntry->StateCount)); */
					pP2pEntry->StateCount--;
				}
				if ((pP2pEntry->StateCount == 0) && ((pP2pEntry->bValid)))
				{
					if (pP2pEntry->ReTransmitCnt >= 20)
					{
						DBGPRINT(RT_DEBUG_ERROR, ("%s:: [%s] ReTransmit Probe Req. limit! stop connect this p2p device!\n",
								__FUNCTION__, decodeP2PClientState(pP2pEntry->P2pClientState)));
						P2pStopConnectThis(pAd);
						P2P_WSC_CONF_MTHD_DEFAULT(pAd);
						pP2pEntry->ReTransmitCnt = 0;
					}
					/*pP2pEntry->P2pClientState = P2PSTATE_CONNECT_COMMAND; */
					/*pP2PCtrl->P2PConnectState = P2P_CONNECT_IDLE; */
					/*pP2PCtrl->P2pCounter.Counter100ms = 0; */
					pP2pEntry->StateCount = 50;
					pP2pEntry->ReTransmitCnt++;
					DBGPRINT(RT_DEBUG_ERROR, ("P2P Table : idx=%d Go Nego Req Retry. \n", i));
					P2pSendProbeReq(pAd, pP2pEntry->ListenChannel);
				}

			}
			else if (pP2pEntry->P2pClientState == P2PSTATE_GO_COMFIRM_ACK_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("P2P Table : idx=%d Get Confirm Ask Success.  p2pState = %d.\n", i, pP2PCtrl->P2PConnectState));
				/* Don't leet ClientState keep in P2PSTATE_GO_COMFIRM_ACK_SUCCESS, */
				/* Or will keep calling P2pGoNegoDone(). */
				/* ClientState will be updated when GO receiving AuthReq. */
				pP2pEntry->P2pClientState = P2PSTATE_GOT_GO_COMFIRM;
				P2pGoNegoDone(pAd, pP2pEntry);
			}
			else if (pP2pEntry->P2pClientState== P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS)
			{
				/* Only when I am GO . I need to check the response ack success or not. */
				/* doesn't check rule.  start GO right away. */
				pP2pEntry->P2pClientState = P2PSTATE_CLIENT_WPS;
				P2pStartAutoGo(pAd);
				DBGPRINT(RT_DEBUG_TRACE,("P2P Table : idx=%d Get Invite Rsp Ask Success.  p2pState = %d.\n", i, pP2PCtrl->P2PConnectState));

				pAd->StaCfg.WscControl.WscState = WSC_STATE_OFF;
				/* this is not Auto GO by command from GUI. So set the intent index to != 16 */
				pAd->P2pCfg.GoIntentIdx = 15;
			}
			else if ((pP2pEntry->P2pClientState == P2PSTATE_CONNECT_COMMAND) || (pP2pEntry->P2pClientState == P2PSTATE_PROVISION_COMMAND) || (pP2pEntry->P2pClientState == P2PSTATE_INVITE_COMMAND))
			{
				if (pP2pEntry->StateCount > 0)
				{
					DBGPRINT(RT_DEBUG_INFO, ("pEntry[%d] State = %s.  StateCount = %d\n", i, decodeP2PClientState(pP2pEntry->P2pClientState), pP2pEntry->StateCount));
					pP2pEntry->StateCount--;
				}

				if ((pP2pEntry->StateCount == 0) && (pP2pEntry->bValid))
				{
					if (pP2pEntry->ReTransmitCnt >= 20)
					{
						DBGPRINT(RT_DEBUG_ERROR, ("%s:: [%s] ReTransmit Probe Req. limit! stop connect this p2p device!\n",
								__FUNCTION__, decodeP2PClientState(pP2pEntry->P2pClientState)));
						P2pStopConnectThis(pAd);
						P2P_WSC_CONF_MTHD_DEFAULT(pAd);
						pP2pEntry->ReTransmitCnt = 0;
					}

					pP2pEntry->StateCount = 3;
					pP2pEntry->ReTransmitCnt++;
					DBGPRINT(RT_DEBUG_TRACE, ("P2P Table : idx=%d Send Probe Req. \n", i));
					P2pSendProbeReq(pAd, pP2pEntry->ListenChannel);
				}

			}
			else if ((pP2pEntry->P2pClientState == P2PSTATE_DISCOVERY_GO) && (MAC_ADDR_EQUAL(pP2PCtrl->ConnectingMAC, pP2pEntry->addr)))
			{
				if (pP2pEntry->StateCount > 0)
				{
					DBGPRINT(RT_DEBUG_INFO, ("pEntry[%d] State = %s.  StateCount = %d\n", i, decodeP2PClientState(pP2pEntry->P2pClientState), pP2pEntry->StateCount));
					pP2pEntry->StateCount--;
				}

				if ((pP2pEntry->StateCount == 0) && (pP2pEntry->bValid))
				{
					if (pP2pEntry->ReTransmitCnt >= 20)
					{
						DBGPRINT(RT_DEBUG_ERROR, ("%s:: [%s] ReTransmit Probe Req. limit! stop connect this p2p device!\n",
								__FUNCTION__, decodeP2PClientState(pP2pEntry->P2pClientState)));
						P2pStopConnectThis(pAd);
						P2P_WSC_CONF_MTHD_DEFAULT(pAd);
						pP2pEntry->ReTransmitCnt = 0;
					}

					pP2pEntry->P2pClientState = P2PSTATE_PROVISION_COMMAND;
					pP2pEntry->StateCount =0;
					pP2pEntry->ReTransmitCnt++;
					DBGPRINT(RT_DEBUG_TRACE, ("P2P Table : idx=%d Send Probe Req. \n", i));
					P2pSendProbeReq(pAd, pP2pEntry->ListenChannel);
				}
			}
		}
	}
}

/*	
	==========================================================================
	Description: 
		Copy P2P Table's information to Mac Table when the P2P Device is in my group.
		
	Parameters: 
	Note:
	==========================================================================
 */
VOID P2pCopyMacTabtoP2PTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		P2pindex,
	IN UCHAR		Macindex)
{
	MAC_TABLE_ENTRY  *pEntry;
	RT_P2P_CLIENT_ENTRY	*pP2pEntry;
	
	if ((P2pindex >= MAX_P2P_GROUP_SIZE) || (Macindex >= MAX_LEN_OF_MAC_TABLE))
		return;

	pEntry = &pAd->MacTab.Content[Macindex];
	pP2pEntry = &pAd->P2pTable.Client[P2pindex];
	pP2pEntry->CTWindow= pEntry->P2pInfo.CTWindow;
	pP2pEntry->P2pClientState = pEntry->P2pInfo.P2pClientState;
	pP2pEntry->P2pFlag = pEntry->P2pInfo.P2pFlag;
	pP2pEntry->NoAToken = pEntry->P2pInfo.NoAToken;
	pP2pEntry->GeneralToken = pEntry->P2pInfo.GeneralToken;

	pP2pEntry->DevCapability = pEntry->P2pInfo.DevCapability;
	pP2pEntry->GroupCapability = pEntry->P2pInfo.GroupCapability;
	pP2pEntry->NumSecondaryType = pEntry->P2pInfo.NumSecondaryType;	
	pP2pEntry->DeviceNameLen = pEntry->P2pInfo.DeviceNameLen;
	pP2pEntry->ConfigMethod = pEntry->P2pInfo.ConfigMethod;
	
	RTMPMoveMemory(pP2pEntry->addr, pEntry->P2pInfo.DevAddr, MAC_ADDR_LEN);
	RTMPMoveMemory(pP2pEntry->InterfaceAddr, pEntry->P2pInfo.InterfaceAddr, MAC_ADDR_LEN);
	/* Save the bssid with interface address. */
	RTMPMoveMemory(pP2pEntry->bssid, pEntry->P2pInfo.InterfaceAddr, MAC_ADDR_LEN);
	RTMPMoveMemory(pP2pEntry->PrimaryDevType, pEntry->P2pInfo.PrimaryDevType, P2P_DEVICE_TYPE_LEN);
	RTMPMoveMemory(pP2pEntry->DeviceName, pEntry->P2pInfo.DeviceName, 32);
	RTMPMoveMemory(pP2pEntry->SecondaryDevType, pEntry->P2pInfo.SecondaryDevType, P2P_DEVICE_TYPE_LEN);

}

/*	
	==========================================================================
	Description: 
		Copy P2P Table's information to Mac Table when the P2P Device is in my group.
		
	Parameters: 
	Note:
	==========================================================================
 */
VOID P2pCopyP2PTabtoMacTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		P2pindex,
	IN UCHAR		Macindex)
{
	MAC_TABLE_ENTRY  *pEntry;
	RT_P2P_CLIENT_ENTRY	*pP2pEntry;

	if ((P2pindex >= MAX_P2P_GROUP_SIZE) || (Macindex >= MAX_LEN_OF_MAC_TABLE))
		return;

	pEntry = &pAd->MacTab.Content[Macindex];
	pP2pEntry = &pAd->P2pTable.Client[P2pindex];
	pEntry->P2pInfo.CTWindow = pP2pEntry->CTWindow;
	pEntry->P2pInfo.P2pClientState = pP2pEntry->P2pClientState;
	pEntry->P2pInfo.P2pFlag = pP2pEntry->P2pFlag;
	pEntry->P2pInfo.NoAToken = pP2pEntry->NoAToken;
	pEntry->P2pInfo.GeneralToken = pP2pEntry->GeneralToken;
	DBGPRINT(RT_DEBUG_TRACE, ("MacTab Add = %s. \n", decodeP2PClientState(pEntry->P2pInfo.P2pClientState)));
	pEntry->P2pInfo.ConfigMethod = pP2pEntry->ConfigMethod;

	pEntry->P2pInfo.DevCapability = pP2pEntry->DevCapability;
	pEntry->P2pInfo.GroupCapability = pP2pEntry->GroupCapability;
	pEntry->P2pInfo.NumSecondaryType = pP2pEntry->NumSecondaryType;	
	pEntry->P2pInfo.DeviceNameLen = pP2pEntry->DeviceNameLen;
	
	RTMPMoveMemory(pEntry->P2pInfo.DevAddr, pP2pEntry->addr, MAC_ADDR_LEN);
	RTMPMoveMemory(pEntry->P2pInfo.InterfaceAddr, pP2pEntry->InterfaceAddr, MAC_ADDR_LEN);
	DBGPRINT(RT_DEBUG_TRACE, ("MacTab InterfaceAddr = %x %x %x %x %x %x . \n", PRINT_MAC(pP2pEntry->InterfaceAddr)));
	DBGPRINT(RT_DEBUG_TRACE, ("MacTab DevAddr = %x %x %x  %x %x %x. \n", PRINT_MAC(pP2pEntry->addr)));
	DBGPRINT(RT_DEBUG_TRACE, ("MacTab  DeviceNameLen = %ld . \n", pEntry->P2pInfo.DeviceNameLen));
	RTMPMoveMemory(pEntry->P2pInfo.PrimaryDevType, pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
	RTMPMoveMemory(pEntry->P2pInfo.DeviceName, pP2pEntry->DeviceName, 32);
	RTMPMoveMemory(pEntry->P2pInfo.SecondaryDevType, pP2pEntry->SecondaryDevType, P2P_DEVICE_TYPE_LEN);
	P2PPrintMac(pAd, Macindex);
}


/*	
	==========================================================================
	Description: 
		Init P2P Group Table.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pGroupTabInit(
	IN PRTMP_ADAPTER pAd) 
{


	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	UCHAR		i;
	
	DBGPRINT(RT_DEBUG_ERROR, ("P2pGroupTabInit .  \n"));
	for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
	{		
		Tab->Client[i].P2pClientState = P2PSTATE_NONE;
		Tab->Client[i].Rule = P2P_IS_CLIENT;
		Tab->Client[i].DevCapability = 0;
		Tab->Client[i].GroupCapability = 0;
		RTMPZeroMemory(Tab->Client[i].addr, MAC_ADDR_LEN);
		RTMPZeroMemory(Tab->Client[i].bssid, MAC_ADDR_LEN);
		RTMPZeroMemory(Tab->Client[i].InterfaceAddr, MAC_ADDR_LEN);
		RTMPZeroMemory(Tab->Client[i].Ssid, MAX_LEN_OF_SSID);
		RTMPZeroMemory(Tab->Client[i].PrimaryDevType, P2P_DEVICE_TYPE_LEN);
		RTMPZeroMemory(Tab->Client[i].SecondaryDevType, P2P_DEVICE_TYPE_LEN);
		RTMPZeroMemory(Tab->Client[i].DeviceName, P2P_DEVICE_NAME_LEN);
		Tab->Client[i].DeviceNameLen = 0;
		Tab->Client[i].SsidLen = 0;
		Tab->Client[i].GoIntent = 0;
		Tab->Client[i].OpChannel = 0;
		Tab->Client[i].ListenChannel = 0;
		Tab->Client[i].ConfigMethod = 0;
		Tab->Client[i].P2pClientState = P2PSTATE_NONE;
		Tab->Client[i].StateCount = 0;
		Tab->Client[i].bValid = FALSE;
		Tab->Client[i].ReTransmitCnt = 0;

	}
	Tab->ClientNumber = 0;
	return;
}

/*	
	==========================================================================
	Description: 
		Clean P2P Group Table. If necessary, Send Disassociation
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pGroupTabDisconnect(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bSendDeAuth) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PRT_P2P_CLIENT_ENTRY		pP2pEntry;
	UCHAR		i;
	/*ULONG		BytesRead, BytesNeeded; */
	/*MLME_QUEUE_ELEM	MsgElem;*/
	MLME_QUEUE_ELEM	*pMsgElem = NULL;
	/*MLME_DISASSOC_REQ_STRUCT DisReq;*/
	MLME_DEAUTH_REQ_STRUCT      DeAuthReq;

	DBGPRINT(RT_DEBUG_ERROR, ("P2pGroupTab  Disconnect All==> \n"));
	/*pMsgElem = &MsgElem;*/
	if (bSendDeAuth && P2P_CLI_ON(pAd))
	{
		os_alloc_mem(pAd, (UCHAR **)&pMsgElem, sizeof(MLME_QUEUE_ELEM));

		COPY_MAC_ADDR(DeAuthReq.Addr, pP2PCtrl->PortCfg.Bssid);
		DeAuthReq.Reason = REASON_DEAUTH_STA_LEAVING;
		pMsgElem->MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
		NdisMoveMemory(pMsgElem->Msg, &DeAuthReq, sizeof(MLME_DEAUTH_REQ_STRUCT));
		MlmeDeauthReqAction(pAd, pMsgElem);
		os_free_mem(NULL, pMsgElem);

		return;		
	}
	for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
	{		
		pP2pEntry = &pAd->P2pTable.Client[i];
		if (pP2pEntry->P2pClientState >= P2PSTATE_CLIENT_OPERATING)
		{
	
			/* this Client is operating, need to send Disassociate frame disconnect. */
			/*Send dis_assoc & de_auth */
			/*
			COPY_MAC_ADDR(&DisReq.Addr, pP2pEntry->addr);
			DisReq.Reason =  REASON_DISASSOC_STA_LEAVING;
			pMsgElem->Machine = ASSOC_STATE_MACHINE;
			pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
			pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			pMsgElem->PortNum = pAd->PortList[pAd->P2pCfg.PortNumber]->PortNumber;
			NdisMoveMemory(pMsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			MlmeDisassocReqAction(pAd, pMsgElem);
			*/
			if (bSendDeAuth)
			{
			os_alloc_mem(pAd, (UCHAR **)&pMsgElem, sizeof(MLME_QUEUE_ELEM));
			
			COPY_MAC_ADDR(DeAuthReq.Addr, pP2pEntry->addr);
			DeAuthReq.Reason = REASON_DEAUTH_STA_LEAVING;
			pMsgElem->MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
			NdisMoveMemory(pMsgElem->Msg, &DeAuthReq, sizeof(MLME_DEAUTH_REQ_STRUCT));
			MlmeDeauthReqAction(pAd, pMsgElem);
			os_free_mem(NULL, pMsgElem);
			}
			/* Delete this peer from table. */
			pP2pEntry->P2pClientState = P2PSTATE_NONE;
			RTMPZeroMemory(pP2pEntry->addr, MAC_ADDR_LEN);
			RTMPZeroMemory(pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
			RTMPZeroMemory(pP2pEntry->SecondaryDevType, P2P_DEVICE_TYPE_LEN);
			RTMPZeroMemory(pP2pEntry->DeviceName, 32);
			pP2pEntry->NumSecondaryType = 0;
			pP2pEntry->DeviceNameLen = 0;

			RTMPZeroMemory(pP2pEntry->Ssid, 32);
			pP2pEntry->SsidLen = 0;
			pP2pEntry->GoIntent = 0;
			if (pAd->P2pTable.ClientNumber > 0)
				pAd->P2pTable.ClientNumber--;
		}
	}

	return;

}

/*	
	==========================================================================
	Description: 
		insert a peer to P2P Group Table. Because this Peer contains P2P IE and P2P Wildwork SSID to indicate that it support P2P
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
UCHAR P2pGroupTabInsert(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr,
	IN P2P_CLIENT_STATE	State,
	IN CHAR Ssid[], 
	IN UCHAR SsidLen,
	IN UCHAR DevCap,
	IN UCHAR GrpCap)
{
	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	UCHAR		i;
	
	RTMP_SEM_LOCK(&pAd->P2pTableSemLock);
	
	if (NdisEqualMemory(ZeroSsid, Addr, 6))
	{
		DBGPRINT(RT_DEBUG_ERROR,("P2pGroupTabInsert . Addr all zero Error. \n"));
		RTMP_SEM_UNLOCK(&pAd->P2pTableSemLock);
		return P2P_NOT_FOUND;
	}
	if ((Addr[0] & 0x1) == 0x1)
	{
		DBGPRINT(RT_DEBUG_ERROR,("P2pGroupTabInsert . Insert mcast Addr Error. \n"));
		RTMP_SEM_UNLOCK(&pAd->P2pTableSemLock);
		return P2P_NOT_FOUND;
	}
	
	for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
	{		
		/* This peer already exist, so only update state. */
		if ((Tab->Client[i].P2pClientState != P2PSTATE_NONE) 
			&& (RTMPEqualMemory(Tab->Client[i].addr, Addr, MAC_ADDR_LEN)))
		{
			if (State != P2PSTATE_NONE)
			Tab->Client[i].P2pClientState = State;
			if ((SsidLen > 0) && (Ssid != NULL))
				RTMPMoveMemory(Tab->Client[i].Ssid, Ssid, 32);
			Tab->Client[i].SsidLen = SsidLen;

			RTMP_SEM_UNLOCK(&pAd->P2pTableSemLock);
			return i;
		}
		else if (Tab->Client[i].P2pClientState == P2PSTATE_NONE)
		{
			Tab->ClientNumber++;
			RTMPMoveMemory(Tab->Client[i].addr, Addr, 6);
			
			DBGPRINT(RT_DEBUG_ERROR, ("    P2pGroupTabInsert[%d] . Arrd[%02x:%02x:%02x:%02x:%02x:%02x] Update State = %s \n", i, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5], decodeP2PClientState(State)));
			Tab->Client[i].P2pClientState = State;
			if ((SsidLen > 0) && (Ssid != NULL))
				RTMPMoveMemory(Tab->Client[i].Ssid, Ssid, 32);
			Tab->Client[i].SsidLen = SsidLen;
			pAd->P2pTable.Client[i].Dbm = 0;
			pAd->P2pTable.Client[i].GoIntent = 0;
			pAd->P2pTable.Client[i].MyGOIndex = 0xff;
			pAd->P2pTable.Client[i].Peerip = 0;
			pAd->P2pTable.Client[i].ConfigTimeOut = 0;
			pAd->P2pTable.Client[i].OpChannel = 0;
			pAd->P2pTable.Client[i].ListenChannel = 0;
			pAd->P2pTable.Client[i].GeneralToken = RandomByte(pAd);
			pAd->P2pTable.Client[i].DevCapability = DevCap;
			pAd->P2pTable.Client[i].GroupCapability = GrpCap;
			pAd->P2pTable.Client[i].ReTransmitCnt = 0;

			if ((pAd->P2pTable.Client[i].GeneralToken == 0)
				 || (pAd->P2pTable.Client[i].GeneralToken > 245))
				 pAd->P2pTable.Client[i].GeneralToken = 6;
			pAd->P2pTable.Client[i].Dpid = DEV_PASS_ID_NOSPEC;
			pAd->P2pTable.Client[i].P2pFlag = 0;
			if (State == P2PSTATE_DISCOVERY_GO)
				pAd->P2pTable.Client[i].Rule = P2P_IS_GO;
			else
				pAd->P2pTable.Client[i].Rule = P2P_IS_CLIENT;

			RTMP_SEM_UNLOCK(&pAd->P2pTableSemLock);
			return i;
		}
	}

	RTMP_SEM_UNLOCK(&pAd->P2pTableSemLock);
	return P2P_NOT_FOUND;

}

/*	
	==========================================================================
	Description: 
		Delete a peer in P2P Group Table.  
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
UCHAR P2pGroupTabDelete(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR    p2pindex, 
	IN PUCHAR    Addr) 
{
	UCHAR	index = 0xff;
	PRT_P2P_CLIENT_ENTRY		pP2pEntry;

	RTMP_SEM_LOCK(&pAd->P2pTableSemLock);

	if ((p2pindex >= MAX_P2P_GROUP_SIZE) && (Addr != NULL))
		index = P2pGroupTabSearch(pAd, Addr);
	else
		index = p2pindex;
	
	DBGPRINT(RT_DEBUG_TRACE,("P2pGroupTabDelete . index = %d. \n", index));
	if (index < MAX_P2P_GROUP_SIZE)
	{
		pP2pEntry = &pAd->P2pTable.Client[index];
		/* Before connected, there is WPS provisioning process. */
		/* So maybe receive disassoc frame. but we can't delete p2p client entry . */
		/* So need to check P2pClientState is connected, then we can delete the entry. */
		DBGPRINT(RT_DEBUG_ERROR, ("P2pGroupTabDelete  index %d.  search addr[3~5] is %x %x %x\n", index, Addr[3],Addr[4],Addr[5]));
		
		RTMPZeroMemory(pP2pEntry->addr, MAC_ADDR_LEN);
		RTMPZeroMemory(pP2pEntry->bssid, MAC_ADDR_LEN);
		RTMPZeroMemory(pP2pEntry->InterfaceAddr, MAC_ADDR_LEN);
		/* Assign a strange address first. */
		pP2pEntry->addr[3] = 0x55;
		pP2pEntry->bssid[3] = 0x55;
		pP2pEntry->InterfaceAddr[3] = 0x55;
		RTMPZeroMemory(pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
		RTMPZeroMemory(pP2pEntry->SecondaryDevType, P2P_DEVICE_TYPE_LEN);
		RTMPZeroMemory(pP2pEntry->DeviceName, P2P_DEVICE_NAME_LEN);
		pP2pEntry->NumSecondaryType = 0;
		pP2pEntry->DeviceNameLen = 0;
		pP2pEntry->ConfigMethod = 0;
		pP2pEntry->OpChannel = 0;
		pP2pEntry->ListenChannel = 0;
		pP2pEntry->Dpid = DEV_PASS_ID_NOSPEC;
		pP2pEntry->MyGOIndex = 0xff;
		pP2pEntry->Peerip = 0;
		pP2pEntry->ConfigTimeOut = 0;
		pP2pEntry->Rule = P2P_IS_CLIENT;

		RTMPZeroMemory(pP2pEntry->Ssid, MAX_LEN_OF_SSID);
		pP2pEntry->SsidLen = 0;
		pP2pEntry->GoIntent = 0;
		if ((pAd->P2pTable.ClientNumber > 0) && (pP2pEntry->P2pClientState != P2PSTATE_NONE))
			pAd->P2pTable.ClientNumber--;
		
		pP2pEntry->P2pClientState = P2PSTATE_NONE;
	}

	RTMP_SEM_UNLOCK(&pAd->P2pTableSemLock);
	
	return index;

}

/*	
	==========================================================================
	Description: 
		Search a peer in P2P Group Table by the same MAc Addr..  
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
UCHAR P2pGroupTabSearch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr) 
{
	UCHAR	i;
	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	UCHAR	index = P2P_NOT_FOUND;
	UCHAR           Allff[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	UCHAR           AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	/* find addr can't be 1. multicast, 2. all zero, 3. all 0xff */
	if ((Addr[0] & 0x1) == 0x1)
		return index;
	
	if (NdisEqualMemory(Allff, Addr, MAC_ADDR_LEN))
		return index;

	if (NdisEqualMemory(AllZero, Addr, MAC_ADDR_LEN))
		return index;

	if (NdisEqualMemory(ZeroSsid, Addr, MAC_ADDR_LEN))
		return index;

	for (i = 0; i < MAX_P2P_GROUP_SIZE; i++)
	{		
		/* If addr format is all zero or all 0xff or multicast, return before. So doesn't need to  */
		/* check invalid match here. */
		if (((NdisEqualMemory(Tab->Client[i].bssid, Addr, 6)) 
			|| (NdisEqualMemory(Tab->Client[i].InterfaceAddr, Addr, 6))
			|| (NdisEqualMemory(Tab->Client[i].addr, Addr, 6)))
			&& (Tab->Client[i].P2pClientState > P2PSTATE_NONE))
			index = i;
	}

	return index;
}

/*	
	==========================================================================
	Description: 
		Clean  in P2P Persistent Table.  
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pPerstTabClean(
	IN PRTMP_ADAPTER pAd) 
{
	PRT_P2P_PERSISTENT_ENTRY		pP2pPerstEntry;
	UCHAR		i;
	
	DBGPRINT(RT_DEBUG_TRACE,("P2pPerstTabClean .  \n"));

	for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
	{		
		pP2pPerstEntry = &pAd->P2pTable.PerstEntry[i];
		RTMPZeroMemory(pP2pPerstEntry, sizeof(RT_P2P_PERSISTENT_ENTRY));
		pP2pPerstEntry->bValid = FALSE;
	}
	pAd->P2pTable.PerstNumber = 0;
	return;

}

/*	
	==========================================================================
	Description: 
		Insert a peer into P2P Persistent Table.  
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
UCHAR P2pPerstTabInsert(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pAddr,
	IN PWSC_CREDENTIAL pProfile) 
{
	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	UCHAR		i, j;
	UCHAR		index = 0;
	WSC_CREDENTIAL	*pPerstProfile;
	
	index = P2pPerstTabSearch(pAd, pAddr, NULL, NULL);
	/* Doesn't have this entry. Add a new one. */
	if (index == P2P_NOT_FOUND)
	{
		for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
		{		
			if (Tab->PerstEntry[i].bValid == FALSE)
			{
				Tab->PerstEntry[i].bValid = TRUE;
				Tab->PerstNumber++;
				if (P2P_GO_ON(pAd))
					Tab->PerstEntry[i].MyRule = P2P_IS_GO;
				else
					Tab->PerstEntry[i].MyRule = P2P_IS_CLIENT;
				
				RTMPMoveMemory(Tab->PerstEntry[i].Addr, pAddr, MAC_ADDR_LEN);
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::Registra MacAddr = %x %x %x %x %x %x \n",Tab->PerstEntry[i].Addr[0], Tab->PerstEntry[i].Addr[1], Tab->PerstEntry[i].Addr[2],Tab->PerstEntry[i].Addr[3],Tab->PerstEntry[i].Addr[4],Tab->PerstEntry[i].Addr[5]));
				RTMPMoveMemory(&Tab->PerstEntry[i].Profile, pProfile, sizeof(WSC_CREDENTIAL));
				pPerstProfile = &Tab->PerstEntry[i].Profile;
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::SsidLen = %d\n",pPerstProfile->SSID.SsidLength));
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::Ssid = %s.\n", pPerstProfile->SSID.Ssid));
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::MacAddr = %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pPerstProfile->MacAddr)));
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::AuthType = 0x%x. EncrType = %d\n",pPerstProfile->AuthType,pPerstProfile->EncrType));
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::KeyIndex = %d\n",pPerstProfile->KeyIndex));
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::KeyLength = %d\n",pPerstProfile->KeyLength));
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::Key ==>\n"));
				for (j=0;j<16;)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("  0x%x 0x%x 0x%x 0x%x\n",
						pPerstProfile->Key[j], pPerstProfile->Key[j+1], pPerstProfile->Key[j+2],pPerstProfile->Key[j+3]));
					j = j+4;
				}
				DBGPRINT(RT_DEBUG_ERROR, ("Perst::<===Key =\n"));
				/*DBGPRINT(RT_DEBUG_ERROR, ("Perst::MacAddr = %x %x %x %x %x %x\n",pProfile->MacAddr[0], pProfile->MacAddr[1], pProfile->MacAddr[2],pProfile->MacAddr[3],pProfile->MacAddr[4],pProfile->MacAddr[5]));*/
				DBGPRINT(RT_DEBUG_ERROR, (" P2P -P2pPerstTabInsert to index = %x. Rule = %s.\n", i, decodeMyRule(Tab->PerstEntry[i].MyRule)));
				return i;
			}
		}
	}
	else if (index < MAX_P2P_TABLE_SIZE)
	{

		i = index;
		Tab->PerstEntry[i].bValid = TRUE;
		if (P2P_GO_ON(pAd))
			Tab->PerstEntry[i].MyRule = P2P_IS_GO;
		else
			Tab->PerstEntry[i].MyRule = P2P_IS_CLIENT;
		RTMPMoveMemory(Tab->PerstEntry[i].Addr, pAddr, MAC_ADDR_LEN);
		RTMPMoveMemory(&Tab->PerstEntry[i].Profile, pProfile, sizeof(WSC_CREDENTIAL));
		pPerstProfile = &Tab->PerstEntry[i].Profile;
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::SsidLen = %d\n",pPerstProfile->SSID.SsidLength));
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::Ssid = %c%c%c%c%c%c%c \n",
			pPerstProfile->SSID.Ssid[0],
			pPerstProfile->SSID.Ssid[1],
			pPerstProfile->SSID.Ssid[2],
			pPerstProfile->SSID.Ssid[3],
			pPerstProfile->SSID.Ssid[4],
			pPerstProfile->SSID.Ssid[5],
			pPerstProfile->SSID.Ssid[6]));
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::AuthType = 0x%x. EncrType = %d\n",pPerstProfile->AuthType,pPerstProfile->EncrType));
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::KeyIndex = %d\n",pPerstProfile->KeyIndex));
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::KeyLength = %d\n",pPerstProfile->KeyLength));
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::Key ==>\n"));
		for (j=0;j<16;)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("  0x%x  0x%x 0x%x 0x%x\n",
				pPerstProfile->Key[j], pPerstProfile->Key[j+1], pPerstProfile->Key[j+2],pPerstProfile->Key[j+3]));
			j = j+4;
		}
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::<===Key =\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("Perst::MacAddr = %x %x %x %x %x %x\n",
			pPerstProfile->MacAddr[0], 
			pPerstProfile->MacAddr[1], 
			pPerstProfile->MacAddr[2],
			pPerstProfile->MacAddr[3],
			pPerstProfile->MacAddr[4],
			pPerstProfile->MacAddr[5]));
		DBGPRINT(RT_DEBUG_ERROR, (" P2P -P2pPerstTabInsert update to index = %x.\n", i));
		return i;
	}
	DBGPRINT(RT_DEBUG_ERROR, ("P2P -P2pPerstTabInsert . PerstNumber = %d.\n", Tab->PerstNumber));
	return index;

}

/*	
	==========================================================================
	Description: 
		Delete an entry  in P2P Persistent Table.  
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
UCHAR P2pPerstTabDelete(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	*pMacList) 
{
	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	UCHAR		i;
	
	for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
	{		
		if (Tab->PerstEntry[i].bValid == TRUE)
		{
			if (NdisEqualMemory(Tab->PerstEntry[i].Addr, pMacList, 6))
			{
				RTMPZeroMemory(&Tab->PerstEntry[i], sizeof(RT_P2P_PERSISTENT_ENTRY));
				Tab->PerstEntry[i].bValid = FALSE;

				if (Tab->PerstNumber > 0)
				{
					Tab->PerstNumber--;
				}
				else
					DBGPRINT(RT_DEBUG_ERROR, (" P2P - Persistent table count error. \n"));

				DBGPRINT(RT_DEBUG_ERROR, (" P2P - Delete a Persistent Entry .Table Number = %d. \n",  Tab->PerstNumber));
			}
				
			return i;
		}
	}
	return 0xff;

}

/*	
	==========================================================================
	Description: 
		SEarch an entry  in P2P Persistent Table.  All parameters must the same.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
UCHAR P2pPerstTabSearch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr,
	IN PUCHAR    Bssid,
	IN PUCHAR    InfAddr) 
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	UCHAR	i;
	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	UCHAR	index = P2P_NOT_FOUND;
	UCHAR           Allff[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	UCHAR           AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	if (Addr && ((Addr[0] & 0x1) == 0x1))
		return index;
	
	if (Addr && NdisEqualMemory(Allff, Addr, MAC_ADDR_LEN))
		return index;

	if (Addr && NdisEqualMemory(AllZero, Addr, MAC_ADDR_LEN))
		return index;

	for (i = 0; i < MAX_P2P_TABLE_SIZE; i++)
	{		
		if ((Tab->PerstEntry[i].bValid == TRUE) &&
			((Addr && NdisEqualMemory(&Tab->PerstEntry[i].Addr, Addr, MAC_ADDR_LEN)) ||
			 (Bssid && NdisEqualMemory(&Tab->PerstEntry[i].Addr, Bssid, MAC_ADDR_LEN)) ||
			 (InfAddr && NdisEqualMemory(&Tab->PerstEntry[i].Addr, InfAddr, MAC_ADDR_LEN)))
			)
		{
			DBGPRINT(RT_DEBUG_TRACE, (" P2P - i = %d. \n", i));
			index = i;
		}
	}
	
	return index;
}

/*	
	==========================================================================
	Description: 
		Clean  in P2P Persistent Table.  
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pCrednTabClean(
	IN PRTMP_ADAPTER pAd) 
{

	UCHAR		i;
	RT_GO_CREDENTIAL_ENTRY	*pCrednEntry;
	
	DBGPRINT(RT_DEBUG_TRACE,("P2pCrednTabClean .  \n"));

	for (i = 0; i < 2/*MAX_P2P_SAVECREDN_SIZE*/; i++)
	{		
		pCrednEntry = &pAd->P2pTable.TempCredential[i];
		RTMPZeroMemory(pCrednEntry, sizeof(RT_GO_CREDENTIAL_ENTRY));
		pCrednEntry->bValid = FALSE;
	}
	return;
}
/*	
	==========================================================================
	Description: 
		Insert a peer into P2P Persistent Table.  
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pCrednTabInsert(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pAddr,
	IN WSC_CREDENTIAL	*pProfile) 
{

	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	UCHAR		i = 0;
	UCHAR		index;
	BOOLEAN			bExist;

	bExist = P2pCrednEntrySearch(pAd, pAddr, &index);
	DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednTabInsert \n"));
	if (bExist == FALSE)
	{
		i = (RandomByte(pAd))%2;
		if (i >= 2/*MAX_P2P_SAVECREDN_SIZE*/)
			i = 0;

		for ( i = 0; i < 2/*MAX_P2P_SAVECREDN_SIZE*/;i++)
		{
				DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednTabInsert NEW\n"));
				Tab->TempCredential[i].bValid = TRUE;
				RTMPMoveMemory(&Tab->TempCredential[i].Profile, pProfile, sizeof(WSC_CREDENTIAL));
				RTMPMoveMemory(&Tab->TempCredential[i].InterAddr, pAddr, MAC_ADDR_LEN);
		}
	}
	else if (index < 2/*MAX_P2P_SAVECREDN_SIZE*/)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednTabInsert Update Existing\n"));
		Tab->TempCredential[index].bValid = TRUE;
		RTMPMoveMemory(&Tab->TempCredential[index].Profile, pProfile, sizeof(WSC_CREDENTIAL));
		RTMPMoveMemory(&Tab->TempCredential[index].InterAddr, pAddr, MAC_ADDR_LEN);
	}
}

/*	
	==========================================================================
	Description: 
		SEarch an entry  in P2P Persistent Table.  All parameters must the same.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pCrednEntrySearch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR    Addr,
	IN PUCHAR	ResultIndex) 
{

	UCHAR	i;
	PRT_P2P_TABLE	Tab = &pAd->P2pTable;
	BOOLEAN		bFind = FALSE;
	UCHAR           Allff[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	UCHAR           AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	if ((Addr[0] & 0x1) == 0x1)
		return bFind;
	
	if (NdisEqualMemory(Allff, Addr, MAC_ADDR_LEN))
		return bFind;

	if (NdisEqualMemory(AllZero, Addr, MAC_ADDR_LEN))
		return bFind;

	DBGPRINT(RT_DEBUG_ERROR, (" P2P - P2pCrednEntrySearch \n"));
	for (i = 0; i < 2/*MAX_P2P_SAVECREDN_SIZE*/; i++)
	{		
		if ((Tab->TempCredential[i].bValid == TRUE) 
			&& NdisEqualMemory(&Tab->TempCredential[i].InterAddr, Addr, MAC_ADDR_LEN))
		{
			DBGPRINT(RT_DEBUG_TRACE, (" Find Credential Entry - i = %d. \n", i));
			bFind = TRUE;
			*ResultIndex = i;
		}
	}
	
	return bFind;
}

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
VOID P2pSendWirelessEvent(
	IN 	PRTMP_ADAPTER pAd,
	IN	INT MsgType,
	IN	PRT_P2P_CLIENT_ENTRY pP2pEntry,
	IN	PUCHAR Addr)
{
	RT_P2P_DEV_FOUND	p2p_dev_info;
	RT_P2P_GROUP_INFO	p2p_group_info;
	RT_P2P_PROV_DISC_RESP p2p_prov_disc_rsp;
	RT_P2P_GO_NEG_REQ_RX p2p_go_neg_req_rx;
	UCHAR p2pIdx = P2P_NOT_FOUND;
	UINT32 rtsp_port = 0;


	if (pAd->P2pCfg.bSigmaEnabled == TRUE)
		return;

	switch (MsgType)
	{
		case RT_P2P_DEVICE_FIND:
			RTMPZeroMemory(&p2p_dev_info, sizeof(RT_P2P_DEV_FOUND));
			if (Addr)
				RTMPMoveMemory(p2p_dev_info.addr, Addr, MAC_ADDR_LEN);
			RTMPMoveMemory(p2p_dev_info.dev_addr, pP2pEntry->addr, MAC_ADDR_LEN);
			RTMPMoveMemory(p2p_dev_info.dev_name, pP2pEntry->DeviceName, pP2pEntry->DeviceNameLen);
			RTMPMoveMemory(p2p_dev_info.pri_dev_type, pP2pEntry->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
			p2p_dev_info.config_methods = pP2pEntry->ConfigMethod;
			p2p_dev_info.dev_capab = pP2pEntry->DevCapability;
			p2p_dev_info.group_capab = pP2pEntry->DevCapability;
			p2p_dev_info.rssi = pP2pEntry->Rssi;
#ifdef WFD_SUPPORT
			p2p_dev_info.bWfdClient = pP2pEntry->bWfdClient;
			if (pP2pEntry->bWfdClient)
			{
				p2p_dev_info.wfd_devive_type = pP2pEntry->wfd_devive_type;
				p2p_dev_info.source_coupled = pP2pEntry->source_coupled;
				p2p_dev_info.session_avail = pP2pEntry->session_avail;
				p2p_dev_info.sink_coupled = pP2pEntry->sink_coupled;
				p2p_dev_info.wfd_service_discovery = pP2pEntry->wfd_service_discovery;
				p2p_dev_info.wfd_PC = pP2pEntry->wfd_PC;
				p2p_dev_info.wfd_CP = pP2pEntry->wfd_CP;
				p2p_dev_info.wfd_time_sync = pP2pEntry->wfd_time_sync;
				p2p_dev_info.rtsp_port = pP2pEntry->rtsp_port;
				p2p_dev_info.max_throughput = pP2pEntry->max_throughput;
				RTMPMoveMemory(p2p_dev_info.assoc_addr, pP2pEntry->assoc_addr, MAC_ADDR_LEN);
				p2p_dev_info.coupled_sink_status.CoupledStat = pP2pEntry->coupled_sink_status.CoupledStat;
			}
#endif /* WFD_SUPPORT */
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_DEVICE_FIND, NULL, &p2p_dev_info, sizeof(RT_P2P_DEV_FOUND));
			break;
		case RT_P2P_RECV_PROV_REQ:
			{
				PRT_P2P_CONFIG pP2pCtrl = &pAd->P2pCfg;
				RTMPZeroMemory(&p2p_dev_info, sizeof(RT_P2P_DEV_FOUND));
				printk("RT_P2P_RECV_PROV_REQ\n");
				if (Addr)
					RTMPMoveMemory(p2p_dev_info.addr, Addr, MAC_ADDR_LEN);
				RTMPMoveMemory(p2p_dev_info.dev_addr, pP2pEntry->addr, MAC_ADDR_LEN);
				RTMPMoveMemory(p2p_dev_info.dev_name, pP2pEntry->DeviceName, pP2pEntry->DeviceNameLen);
				p2p_dev_info.config_methods = pP2pCtrl->ConfigMethod;
				p2p_dev_info.dpid = pP2pCtrl->Dpid;
				RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_RECV_PROV_REQ, NULL, &p2p_dev_info, sizeof(RT_P2P_DEV_FOUND));
			}
			break;
		case RT_P2P_RECV_PROV_RSP:
			RTMPMoveMemory(p2p_prov_disc_rsp.peer, pP2pEntry->addr, MAC_ADDR_LEN);
			p2p_prov_disc_rsp.config_methods = pP2pEntry->ConfigMethod;
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_RECV_PROV_RSP, NULL, &p2p_prov_disc_rsp, sizeof(RT_P2P_PROV_DISC_RESP));
			break;
		case RT_P2P_RECV_GO_NEGO_REQ:
			{
				RTMPZeroMemory(&p2p_go_neg_req_rx, sizeof(RT_P2P_GO_NEG_REQ_RX));
				printk("RT_P2P_RECV_GO_NEGO_REQ\n");
				RTMPMoveMemory(p2p_go_neg_req_rx.src, pP2pEntry->addr, MAC_ADDR_LEN);
				p2p_go_neg_req_rx.dev_passwd_id = pP2pEntry->ConfigMethod;
				RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_RECV_GO_NEGO_REQ, NULL, &p2p_go_neg_req_rx, sizeof(RT_P2P_GO_NEG_REQ_RX));
			}
			break;
		case RT_P2P_GO_NEG_COMPLETED:
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_GO_NEG_COMPLETED, NULL, NULL, 0);
			break;

		case RT_P2P_WPS_COMPLETED:
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_WPS_COMPLETED, NULL, NULL, 0);
			break;

		case RT_P2P_CONNECTED:
			RTMPZeroMemory(&p2p_group_info, sizeof(RT_P2P_GROUP_INFO));
			if (P2P_GO_ON(pAd))
			{
				p2p_group_info.Rule = 1;
				RTMPMoveMemory(p2p_group_info.Bssid, pAd->P2PCurrentAddress, MAC_ADDR_LEN);
				RTMPMoveMemory(p2p_group_info.Ssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen);
				RTMPMoveMemory(p2p_group_info.peer, Addr, MAC_ADDR_LEN);
			}
			else
			{
				p2p_group_info.Rule = 2;
				RTMPMoveMemory(p2p_group_info.Bssid, pAd->P2pCfg.Bssid, MAC_ADDR_LEN);
				RTMPMoveMemory(p2p_group_info.Ssid, pAd->P2pCfg.SSID, pAd->P2pCfg.SSIDLen);
				RTMPMoveMemory(p2p_group_info.peer, Addr, MAC_ADDR_LEN);
			}
#ifdef WFD_SUPPORT
			p2pIdx = P2pGroupTabSearch(pAd, Addr);
			if (p2pIdx < MAX_P2P_GROUP_SIZE)
				p2p_group_info.rtsp_port = pAd->P2pTable.Client[p2pIdx].rtsp_port;
#endif /* WFD_SUPPORT */
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_CONNECTED, NULL, &p2p_group_info, sizeof(RT_P2P_GROUP_INFO));
			break;
		case RT_P2P_DISCONNECTED:
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_DISCONNECTED, NULL, NULL, 0);
			break;
		case RT_P2P_CONNECT_FAIL:
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_CONNECT_FAIL, NULL, NULL, 0);
			break;	
		case RT_P2P_LEGACY_CONNECTED:
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_LEGACY_CONNECTED, NULL, NULL, 0);
			break;	
		case RT_P2P_LEGACY_DISCONNECTED:
			RtmpOSWrielessEventSend(pAd->p2p_dev, RT_WLAN_EVENT_CUSTOM, RT_P2P_LEGACY_DISCONNECTED, NULL, NULL, 0);
			break;	
	}
}
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */


