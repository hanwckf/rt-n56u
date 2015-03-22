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
	Peer to peer is also called Wifi Direct. P2P is a Task Group of WFA.

	Revision History:
	Who              When               What
	--------    ----------    ----------------------------------------------
	Jan Lee         2009-10-05    created for Peer-to-Peer(Wifi Direct)
*/
#include "rt_config.h"

/* Vendor Specific OUI for P2P defined by WFA. */
UCHAR	P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9}; /* spec. 1.14 OUI */
extern UCHAR ZERO_MAC_ADDR[];
extern UCHAR	STA_Wsc_Pri_Dev_Type[];

UCHAR	WILDP2PSSID[7] = {'D', 'I', 'R', 'E', 'C', 'T','-'};
UCHAR	WILDP2PSSIDLEN = 7;
UCHAR	WIFIDIRECT_OUI[] = {0x50, 0x6f, 0x9a, 0x09}; /* spec. 1.14 OUI */
/* UCHAR	DEFAULTWPAPSKEY[8] = {0x33, 0x35, 0x33, 0x34, 0x33, 0x36, 0x31, 0x34};*/
/* Like to key in 12345678 */
UCHAR	DEFAULTWPAPSKEY[8] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};

extern INT Set_P2p_OpMode_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

extern INT Set_AP_WscSsid_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN	PSTRING arg);

extern INT Set_AP_WscConfMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

extern INT Set_AP_WscConfStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

extern INT Set_AP_WscMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

extern INT Set_AP_WscGetConf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

extern INT Set_AP_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

extern INT Set_P2pCli_WscSsid_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

extern INT Set_P2pCli_Ssid_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

extern INT Set_P2pCli_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

/*	
	==========================================================================
	Description: 
		Called once when the card is being initialized.
		
	Parameters: 

	Note:

	==========================================================================
 */

#include "rt_config.h"
#include "p2p.h"

VOID	P2pCfgInit(

	IN PRTMP_ADAPTER pAd) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pP2PCtrl->P2p_OpMode = P2P_CONCURRENT;

	pP2PCtrl->bKeepSlient = FALSE;
	pP2PCtrl->NoAIndex = MAX_P2P_GROUP_SIZE;
	pP2PCtrl->ListenChannel = 1;
	pP2PCtrl->GroupChannel = 1;
	pP2PCtrl->GroupOpChannel = 1;
	P2pSetListenIntBias(pAd, 3);
	pP2PCtrl->DeviceNameLen = 10;
	pP2PCtrl->DeviceName[0] = 'R';
	pP2PCtrl->DeviceName[1] = 'a';
	pP2PCtrl->DeviceName[2] = 'l';
	pP2PCtrl->DeviceName[3] = 'i';
	pP2PCtrl->DeviceName[4] = 'n';
	pP2PCtrl->DeviceName[5] = 'k';
	pP2PCtrl->DeviceName[6] = '-';
	pP2PCtrl->DeviceName[7] = 'P';
	pP2PCtrl->DeviceName[8] = 0x32;
	pP2PCtrl->DeviceName[9] = 'P';

	/*pP2PCtrl->P2PDiscoProvState = P2P_DISABLE; */
	pP2PCtrl->P2PConnectState = P2P_CONNECT_IDLE;
	/* Set Dpid to "not specified". it means, GUI doesn't set for connection yet. */
	pP2PCtrl->Dpid = DEV_PASS_ID_NOSPEC;
	pP2PCtrl->P2pManagedParm.APP2pManageability = 0xff;
	pP2PCtrl->P2pManagedParm.ICSStatus = ICS_STATUS_DISABLED; 
	P2pGroupTabInit(pAd);
	P2pCrednTabClean(pAd);
	P2pScanChannelDefault(pAd);
	RTMPZeroMemory(pAd->P2pCfg.SSID, MAX_LEN_OF_SSID);
	RTMPMoveMemory(pAd->P2pCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
	/*RTMPMoveMemory(pAd->P2pCfg.Bssid, pAd->P2pCfg.CurrentAddress, MAC_ADDR_LEN); */
	pP2PCtrl->SSIDLen = WILDP2PSSIDLEN;
	pP2PCtrl->GONoASchedule.bValid = FALSE;
	pP2PCtrl->GONoASchedule.bInAwake = TRUE;
	pP2PCtrl->GONoASchedule.bWMMPSInAbsent = FALSE; /* Set to FALSE if changes state to Awake */
	pP2PCtrl->GONoASchedule.Token = 0;
	pP2PCtrl->GoIntentIdx = 0;
	pP2PCtrl->Rule = P2P_IS_DEVICE;
	pP2PCtrl->WscMode = WSC_PIN_MODE;
	pP2PCtrl->DefaultConfigMethod = P2P_REG_CM_DISPLAY;
	pP2PCtrl->bExtListen = FALSE;
	pP2PCtrl->bIntraBss = FALSE;
	pP2PCtrl->ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
	pP2PCtrl->ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
	pP2PCtrl->GONoASchedule.Count = 0;
	pP2PCtrl->GONoASchedule.Duration = 0;
	pP2PCtrl->GONoASchedule.Interval = 0;
	pP2PCtrl->DevDiscPeriod = P2P_SCAN_PERIOD;
	/* P2P WSC_IR default value */
	pP2PCtrl->DevInfo.Version = WSC_VERSION;
	pP2PCtrl->bConfiguredAP = TRUE;
	pP2PCtrl->DevInfo.RfBand |= WSC_RFBAND_24GHZ;			/* 2.4G */
	NdisMoveMemory(&pP2PCtrl->DevInfo.PriDeviceType, &STA_Wsc_Pri_Dev_Type[0], 8);
	WscGenerateUUID(pAd, &pP2PCtrl->Wsc_Uuid_E[0], &pP2PCtrl->Wsc_Uuid_Str[0], 0, FALSE);
	NdisMoveMemory(&pP2PCtrl->DevInfo.Uuid[0], &pP2PCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
	

	pP2PCtrl->bSigmaEnabled = FALSE;
	pP2PCtrl->bP2pCliPmEnable = FALSE;
	pP2PCtrl->bLowRateQoSNULL = FALSE;
	pP2PCtrl->bP2pCliReConnect = FALSE;
	pP2PCtrl->bStopAuthRsp = TRUE;
	pP2PCtrl->bP2pReSendTimerRunning = FALSE;
	pP2PCtrl->DevDiscPeriod = P2P_SCAN_PERIOD;
	pP2PCtrl->bPeriodicListen = TRUE;
	pP2PCtrl->bConfirmByUI = FALSE;

	pP2PCtrl->bProvAutoRsp = TRUE;
	pP2PCtrl->P2pProvIndex = 0xFF;
	pP2PCtrl->P2pProvUserNotify = 0;
	pP2PCtrl->pGoNegoRspOutBuffer = NULL;
	pP2PCtrl->bSentProbeRSP = FALSE;
	/*
		Bit[31] : Software Based NoA implementation
		Bit[23] : Opps, not use.
		Bit[22] : Service Discovey, not use.
		Bit[21] : Extent Listen
		Bit[20] : Client Discovery
		Bit[16:19] : OpChannel, not use
		Bit[15] : IntraBss
		Bit[12:14] : Config Method, not use.
		Bit[8:11] : Default Channel, not use.
		Bit[4:7] : GO Intent, not use.
		Bit[3] : Invite
		Bit[2] : Persistent, not use.
		Bit[1] : Managed
		Bit[0] : Enable, not use.
	 */
	pP2PCtrl->P2pControl.word = 0x80108008;


	DBGPRINT(RT_DEBUG_ERROR, ("%s:: \n", __FUNCTION__));
}

/*	
	==========================================================================
	Description: 
		Periodic Routine for P2P. 
		
	Parameters: 
		 
	Note:
		 
	==========================================================================
 */
VOID P2pPeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	ULONG		BssIdx = BSS_NOT_FOUND;

	if ((pP2PCtrl->P2pCounter.bListen) && (pP2PCtrl->P2pCounter.ListenInterval > 0))
		pP2PCtrl->P2pCounter.ListenInterval--; /* update Listen interval */

	if (pP2PCtrl->P2pCounter.NextScanRound > 0)
		pP2PCtrl->P2pCounter.NextScanRound--; /* update Next Scan Round */

	if (pP2PCtrl->P2pCounter.CounterAftrScanButton > 0)
		pP2PCtrl->P2pCounter.CounterAftrScanButton--; /* update Device Discovery period */

	if (pP2PCtrl->P2pCounter.CounterAftrSetEvent != 0xffffffff)
		pP2PCtrl->P2pCounter.CounterAftrSetEvent++;

	if (pP2PCtrl->P2pCounter.ClientConnectedCounter > 0)
		pP2PCtrl->P2pCounter.ClientConnectedCounter--;

	if (pP2PCtrl->P2pCounter.ManageAPEnReconCounter > 0)
		pP2PCtrl->P2pCounter.ManageAPEnReconCounter--;

	DBGPRINT(RT_DEBUG_INFO, ("%s : Counter100ms[%ld] CounterAftScanButton[%ld] ClientConnCnt[%ld] MngAPEnReconnCnt[%ld] DisableRetryGrpFormCnt[%ld] NextScanRound[%ld]\n", 
		__FUNCTION__, pP2PCtrl->P2pCounter.Counter100ms, pP2PCtrl->P2pCounter.CounterAftrScanButton, pP2PCtrl->P2pCounter.ClientConnectedCounter,
		pP2PCtrl->P2pCounter.CounterAftrSetEvent, pP2PCtrl->P2pCounter.DisableRetryGrpFormCounter, pP2PCtrl->P2pCounter.NextScanRound));

 	/* Scan period expired. Return to listen state. Only do once. So check value equal. */
	if ((pP2PCtrl->P2pCounter.bStartScan == TRUE) && (pAd->P2pCfg.P2pCounter.CounterAftrScanButton == 0))
		P2PDevDiscTimerExec(pAd, 0);
	/* Check whether to start a  P2P scan /search process. */
	if ((pP2PCtrl->P2pCounter.bListen) && (pP2PCtrl->P2pCounter.ListenInterval == 0))
		P2PListenTimerExec(pAd, 0);
	if ((pP2PCtrl->P2pCounter.bNextScan == TRUE) && (pP2PCtrl->P2pCounter.NextScanRound == 0))
		P2PNextScanTimerExec(pAd, 0);

	if (pAd->P2pCfg.P2pCounter.UserAccept > 0)
	{
		pAd->P2pCfg.P2pCounter.UserAccept--;
	}

	if ((pP2PCtrl->bProvAutoRsp == FALSE) && (pP2PCtrl->P2pProvIndex != P2P_NOT_FOUND) )
	{
		if (pP2PCtrl->P2pCounter.UserAccept == 0)
		{
			PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	
			pP2PCtrl->P2pProvUserNotify = 0;
			pP2PCtrl->P2pProvIndex = P2P_NOT_FOUND;
		}
	}

		/* ====================================> */
		/*	P2P connect state maintain */
		if (((pP2PCtrl->P2PConnectState == P2P_DO_GO_SCAN_BEGIN) 
			|| (pP2PCtrl->P2PConnectState == P2P_DO_GO_NEG_DONE_CLIENT)
			|| (pP2PCtrl->P2PConnectState == P2P_DO_GO_SCAN_OP_BEGIN))
			&& (pP2PCtrl->P2pCounter.Counter100ms == pP2PCtrl->P2pCounter.NextScanRound))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("P2P P2pPeriodicExec Scan Begin NextScanRound = %ld \n", pP2PCtrl->P2pCounter.NextScanRound));
			MlmeEnqueue(pAd, 
				MLME_CNTL_STATE_MACHINE, 
				OID_802_11_BSSID_LIST_SCAN, 
				0,
				"",
				0);

			MlmeHandler(pAd);
			pP2PCtrl->P2pCounter.Counter100ms = 0;
			pP2PCtrl->P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_WPS_BASE) + 4;
		}
		else if ((pAd->P2pCfg.P2PConnectState == P2P_DO_GO_SCAN_DONE) 
			|| (pAd->P2pCfg.P2PConnectState == P2P_DO_GO_SCAN_OP_DONE))
		{
			/* if(pP2PCtrl->PortSubtype != PORTSUBTYPE_P2PGO) */
			if (!P2P_GO_ON(pAd))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("P2PConnectState[%s] : Do BssTableSearch\n", decodeP2PState(pP2PCtrl->P2PConnectState)));
				/* BssIdx = BssTableSearch(pAd, &pAd->ScanTab, pAd->P2pCfg.Bssid, pAd->StaCfg.WscControl.WscAPChannel); */
				BssIdx = BssTableSearch(&pAd->ScanTab, pAd->P2pCfg.Bssid, pAd->MlmeAux.Channel);
			}
			/* Since can't find the target AP in the list.
			    Go back to scan state again to scan the target AP. */
			if (BssIdx == BSS_NOT_FOUND) 
			{
				pP2PCtrl->P2PConnectState = P2P_DO_GO_SCAN_BEGIN;
				pP2PCtrl->P2pCounter.Counter100ms = 0;
				pP2PCtrl->P2pCounter.NextScanRound = 10;	/* start scan after 1 s */
				DBGPRINT(RT_DEBUG_ERROR, ("CNTL - Nr= %d. Channel = %d. BSSID not found.  %02x:%02x:%02x:%02x:%02x:%02x.\n", pAd->ScanTab.BssNr, pAd->MlmeAux.Channel /*pAd->StaCfg.WscControl.WscAPChannel*/, PRINT_MAC(pP2PCtrl->Bssid)));
				DBGPRINT(RT_DEBUG_ERROR, ("CNTL - BSSID not found.	Goback to %s  \n", decodeP2PState(pP2PCtrl->P2PConnectState)));
				return;
			}
			else if (pAd->ScanTab.BssEntry[BssIdx].SsidLen < 9)
			{
				pP2PCtrl->P2PConnectState = P2P_DO_GO_SCAN_BEGIN;
				pP2PCtrl->P2pCounter.Counter100ms = 0;
				pP2PCtrl->P2pCounter.NextScanRound = 5;	/* start scan after 500ms */
				DBGPRINT(RT_DEBUG_ERROR, ("CNTL -Nr= %d. Channel = %d.	SSID is  %c%c%c%c%c%c \n", pAd->ScanTab.BssNr, pAd->MlmeAux.Channel /*pAd->StaCfg.WscControl.WscAPChannel*/, pAd->P2pCfg.SSID[0], pAd->P2pCfg.Bssid[1],pAd->P2pCfg.SSID[2],pAd->P2pCfg.SSID[3],pAd->P2pCfg.SSID[4],pAd->P2pCfg.SSID[5]));
				DBGPRINT(RT_DEBUG_ERROR, ("CNTL -	Goback to %s  \n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
				return;
			}
	
			/* Now copy the scanned SSID to my CommonCfg.Ssid */
 			RTMPMoveMemory(pP2PCtrl->PortCfg.Ssid, pAd->ScanTab.BssEntry[BssIdx].Ssid,32);
			pP2PCtrl->PortCfg.SsidLen = pAd->ScanTab.BssEntry[BssIdx].SsidLen;
 			DBGPRINT(RT_DEBUG_ERROR, ("P2P P2pPeriodicExec P2P_DO_GO_SCAN_DONE. Find BssIdx = %ld\n", BssIdx));
			DBGPRINT(RT_DEBUG_ERROR, ("Change P2PConnectState[%s -> %s]\n", decodeP2PState(pP2PCtrl->P2PConnectState), decodeP2PState(P2P_DO_WPS_ENROLLEE)));
			pP2PCtrl->P2PConnectState = P2P_DO_WPS_ENROLLEE;
		}
		/* <<==================================== */
		P2pGroupMaintain(pAd);
			
		/* P2P_ANY_IN_FORMATION_AS_GO means I am AutoGO. AutoGo also need to do scan. So don't return here. */
		if (IS_P2P_CONNECTING(pAd) && (pP2PCtrl->P2PConnectState != P2P_ANY_IN_FORMATION_AS_GO))
		{
			return;
		}

		/* <<==================================== */
		
		/* Maintain listen state when in Concurrent mode. STA+P2P
		    check if need to resume NoA Schedule. */
}

BOOLEAN P2pResetNoATimer(
	IN PRTMP_ADAPTER pAd,
	IN	ULONG	DiffTimeInus)
{
	ULONG	GPDiff;
	/*ULONG	Value;*/
	BOOLEAN	brc = FALSE;

	/*
		Software based timer means don't use GP interrupt to get precise timer calculation. 
		So need to check time offset caused by software timer.
	 */
	if (IS_SW_NOA_TIMER(pAd))
	{
		GPDiff = (DiffTimeInus>>10) & 0xffff;
		if (GPDiff > 0)
		{
			GPDiff++;
			RTMPSetTimer(&pAd->P2pCfg.P2pSwNoATimer, GPDiff);
			/* Increase timer tick counter. */
			pAd->P2pCfg.GONoASchedule.SwTimerTickCounter++;
			brc = TRUE;
			/* Will go to awake later. Set a pre-enter-absence timer that the time out is smaller the GPDiff. */
			if (pAd->P2pCfg.GONoASchedule.bInAwake == FALSE)
			{
				if (GPDiff > 10)
				{
					RTMPSetTimer(&pAd->P2pCfg.P2pPreAbsenTimer, (GPDiff - 10));
				}
			}
		}
	}
	else
	{
		brc = P2pSetGP(pAd, DiffTimeInus);
	}
	return brc;

}


/*	
	==========================================================================
	Description: 
		Start P2P Search State
		
	Parameters: 
		 
	Note:
		 
	==========================================================================
 */
VOID P2pScan(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		/* Stop Scan and resume */
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
		pAd->MlmeAux.Channel = 0;
		ScanNextChannel(pAd, OPMODE_STA);
	}

	P2pGotoIdle(pAd);
	P2pGroupTabInit(pAd);
	P2PInitDevDiscTimer(pAd, 0);
	P2PInitNextScanTimer(pAd, 0);	
	/* Set P2P Device Discovery Timer */
	P2PSetDevDiscTimer(pAd, 0);
	P2PSetNextScanTimer(pAd, 10);
}

/*	
	==========================================================================
	Description: 
		Stop connect command to connect with current MAC becuase the connect process already bagan.
		
	Parameters: 
		 
	Note:
		 
	==========================================================================
 */
VOID P2pStopConnectThis(
	IN PRTMP_ADAPTER pAd)
{
	RTMPZeroMemory(&pAd->P2pCfg.ConnectingMAC[0], MAC_ADDR_LEN);
}

/*	
	==========================================================================
	Description: 
		Set parameter stop P2P Search State when P2P has started Group Formation.
		
	Parameters: 
		 
	Note:
		 
	==========================================================================
 */
VOID P2pStopScan(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_INFO, ("<---- P2P - P2pStopScan @channel = %d.\n", pAd->MlmeAux.Channel));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		/* Stop Scan and resume */
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
		pAd->MlmeAux.Channel = 0;
		ScanNextChannel(pAd, OPMODE_STA);
	}

	/* Set scan channel to Last one to stop Scan Phase. Because SCannextchannel will use channel to judge if it should stop scan. */
	P2PInitDevDiscTimer(pAd, 0);
	P2PInitListenTimer(pAd, 0);
	P2PInitNextScanTimer(pAd, 0);
	P2pGotoIdle(pAd);
	/* update P2P Ctrl State Machine status. */
	MlmeEnqueue(pAd, P2P_CTRL_STATE_MACHINE, P2P_CTRL_DISC_CANL_EVT, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);

	DBGPRINT(RT_DEBUG_INFO, ("----> P2P - P2pStopScan @channel = %d.\n", pAd->MlmeAux.Channel));
}

/*	
	==========================================================================
	Description: 
		 Goto Idle state. Update necessary parameters.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pGotoIdle(
	IN PRTMP_ADAPTER pAd) 
{
	/* pAd->P2pCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY; */
	pAd->P2pCfg.P2pCounter.Counter100ms = 0;
	/* Set a randon period to start next Listen State. */
	/* pAd->P2pCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + P2P_RANDOM_BIAS; */

	pAd->P2pCfg.CtrlCurrentState = P2P_CTRL_IDLE;
	pAd->P2pCfg.DiscCurrentState = P2P_DISC_IDLE;
	pAd->P2pCfg.GoFormCurrentState = P2P_GO_FORM_IDLE;
}


/*	
	==========================================================================
	Description: 
		 Goto Scan/Search state. Update necessary parameters.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pGotoScan(
	IN PRTMP_ADAPTER pAd) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	/* Reset 100ms Counter to zero. */
	pAd->P2pCfg.P2pCounter.Counter100ms = 0;
	/* Set a short time to start next search State for the 1st time. */
	pAd->P2pCfg.P2pCounter.NextScanRound = 5;

	DBGPRINT(RT_DEBUG_ERROR, ("P2pGotoScan!  Set pP2PCtrl->NextScanRound = %ld Here!!!\n", pP2PCtrl->P2pCounter.NextScanRound));
}

/*	
	==========================================================================
	Description: 
		When I am GO, start a P2P NoA schedule.
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2pGOStartNoA(
	IN PRTMP_ADAPTER pAd)
{
	ULONG	Value;
	ULONG	TimeTillTbtt;
	ULONG	temp;
	
	pAd->P2pCfg.GONoASchedule.Token++;
	pAd->P2pCfg.GONoASchedule.bValid = TRUE;
			/* Start Time */
	RTMP_IO_READ32(pAd, TSF_TIMER_DW1, &pAd->P2pCfg.GONoASchedule.TsfHighByte);
	DBGPRINT(RT_DEBUG_TRACE,("P2pGOStartNoA parameter.!!!!HighByte = %lx \n", pAd->P2pCfg.GONoASchedule.TsfHighByte));
	RTMP_IO_READ32(pAd, TBTT_TIMER, &TimeTillTbtt);
	TimeTillTbtt = TimeTillTbtt&0x1ffff;
	DBGPRINT(RT_DEBUG_TRACE,("   .!!!!TimeTillTbtt =  %ld  \n", TimeTillTbtt));
	
	RTMP_IO_READ32(pAd, TSF_TIMER_DW0, &Value);
	DBGPRINT(RT_DEBUG_TRACE,("   .!!!!Current Tsf LSB = = %ld \n",  Value));
	temp = TimeTillTbtt*64+Value;
	DBGPRINT(RT_DEBUG_TRACE,("   .!!!!Tsf LSB + TimeTillTbtt= %ld \n", temp));
	/* Wait five beacon 0x7d00 for 5 beacon interval.  0x6400 is set to 25%*beacon interval */
	pAd->P2pCfg.GONoASchedule.StartTime = Value + TimeTillTbtt*64 + 512000 + 25600;
	pAd->P2pCfg.GONoASchedule.NextTargetTimePoint = Value + TimeTillTbtt*64 + 512000 + 25600 + pAd->P2pCfg.GONoASchedule.Duration;
	pAd->P2pCfg.GONoASchedule.ThreToWrapAround = pAd->P2pCfg.GONoASchedule.StartTime + 0x7fffffff;
	P2pSetGP(pAd, (TimeTillTbtt*64 + 512000 + 25600));
	temp = Value + TimeTillTbtt*64 + 0x7D000 + 0x6400;
	DBGPRINT(RT_DEBUG_TRACE,("   .!!!!Expect Starttime= %ld. ThreToWrapAround = %ld. \n", temp, pAd->P2pCfg.GONoASchedule.ThreToWrapAround));
	temp = temp - Value;
	DBGPRINT(RT_DEBUG_TRACE,("   .!!!!more = %ld  to start time \n", temp));
	pAd->P2pCfg.GONoASchedule.bInAwake = TRUE;

}

VOID	 P2pStopNoA(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY	pMacClient)
{
	ULONG	Value;
	BOOLEAN	Cancelled;
	
	DBGPRINT(RT_DEBUG_TRACE,("P2pStopNoA.!!!! \n"));
	
	RTMPCancelTimer(&pAd->P2pCfg.P2pPreAbsenTimer, &Cancelled);
	pAd->P2pCfg.bKeepSlient = FALSE;
	pAd->P2pCfg.bPreKeepSlient = FALSE;
	if (pMacClient != NULL)
	{
		pMacClient->P2pInfo.NoADesc[0].Count = 0xf3;
		pMacClient->P2pInfo.NoADesc[0].bValid = FALSE;
		pMacClient->P2pInfo.NoADesc[0].bInAwake = TRUE;
		/*
			Try set Token to a value that has smallest chane the same as the Next Token GO will use.
			So decrease 1
		 */
		pMacClient->P2pInfo.NoADesc[0].Token--;
	}
	RTMPCancelTimer(&pAd->P2pCfg.P2pSwNoATimer, &Cancelled);
	pAd->P2pCfg.GONoASchedule.bValid = FALSE;
	pAd->P2pCfg.GONoASchedule.bInAwake = TRUE;
	pAd->P2pCfg.GONoASchedule.bWMMPSInAbsent = FALSE; /* Set to FALSE if changes state to Awake */
	/* If need not resume NoA. Can reset all parameters. */
	if (pAd->P2pCfg.GONoASchedule.bNeedResumeNoA == FALSE)
	{
		pAd->P2pCfg.GONoASchedule.Count = 1;
		pAd->P2pCfg.GONoASchedule.Duration = 0xc800;
		pAd->P2pCfg.GONoASchedule.Interval = 0x19000;
	}

	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &= (0xfffffffd);
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	pAd->P2pCfg.GONoASchedule.SwTimerTickCounter = 0;

	/* Set to false again. */
	pAd->P2pCfg.bPreKeepSlient = FALSE;

}

VOID	 P2pStartOpPS(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->P2pCfg.GONoASchedule.bValid == TRUE)
		P2pStopNoA(pAd, NULL);
	
	DBGPRINT(RT_DEBUG_TRACE,("P2P : !! P2pStartOpPS \n"));
	pAd->P2pCfg.CTWindows = 0x8a;
	/* Wait next beacon period to really start queue packet. */
	pAd->P2pCfg.bKeepSlient = FALSE;

}

VOID	 P2pStopOpPS(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->P2pCfg.GONoASchedule.bValid == FALSE)
		pAd->P2pCfg.bKeepSlient = FALSE;

	if (P2P_TEST_BIT(pAd->P2pCfg.CTWindows, P2P_OPPS_BIT))
		pAd->P2pCfg.bFirstTimeCancelOpps = TRUE;
	pAd->P2pCfg.CTWindows = 0;
}

/*	
	==========================================================================
	Description: 
		When I am P2P Client , Handle NoA Attribute.
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2pPreAbsenTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	pAd->P2pCfg.bPreKeepSlient = TRUE;
}

/*	
	==========================================================================
	Description: 
		When I am P2P Client , Handle NoA Attribute.
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2pSwNoATimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	P2pGPTimeOutHandle(pAd);
}

/*	
	==========================================================================
	Description: 
		When I am P2P GO / Client , Handle P2P session execute WSC in 30secs.
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
VOID P2pWscTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	BOOLEAN		Cancelled;
	PRT_P2P_CONFIG 	pP2pCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::  P2P Execute WSC has expire %dsecs, Stop it!\n", __FUNCTION__, (P2P_WSC_TIMER/1000)));
	RTMPCancelTimer(&pP2pCtrl->P2pWscTimer, &Cancelled);

	if (P2P_CLI_ON(pAd))
	{
#ifdef RTMP_MAC_PCI
		P2pLinkDown(pAd, P2P_DISCONNECTED);
#endif /* RTMP_MAC_PCI */
	}
	else if (P2P_GO_ON(pAd))
	{
		UINT32 i, p2pEntryCnt=0;
		MAC_TABLE_ENTRY *pEntry;
		PWSC_CTRL			pWscControl = &pAd->ApCfg.MBSSID[0].WscControl;

		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			pEntry = &pAd->MacTab.Content[i];
			if (IS_P2P_GO_ENTRY(pEntry) && (pEntry->WpaState == AS_PTKINITDONE))
				p2pEntryCnt++;
		}
		DBGPRINT(RT_DEBUG_ERROR, ("P2pWscTimeOut - Total= %d. p2pEntry = %d.\n", pAd->MacTab.Size, p2pEntryCnt));

		if ((p2pEntryCnt == 0) && (pAd->flg_p2p_OpStatusFlags == P2P_GO_UP))
#ifdef RTMP_MAC_PCI
			P2pLinkDown(pAd, P2P_DISCONNECTED);
#endif /* RTMP_MAC_PCI */
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s::  not P2P GO / CLI on !!\n", __FUNCTION__));
}

VOID P2pReSendTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	PRT_P2P_CLIENT_ENTRY pP2pEntry = NULL;
	BOOLEAN			Cancelled;
	PRT_P2P_CONFIG	pP2pCtrl = &pAd->P2pCfg;
	UCHAR			BBPValue = 0;
	UCHAR			p2pindex = P2P_NOT_FOUND;
	ULONG			FrameLen;
	USHORT			ConfigMthd;


	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Send to %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(pP2pCtrl->ConnectingMAC)));
	pP2pCtrl->bP2pReSendTimerRunning = FALSE;
	RTMPCancelTimer(&pP2pCtrl->P2pReSendTimer, &Cancelled);

	p2pindex = P2pGroupTabSearch(pAd, pP2pCtrl->ConnectingMAC);
	if (p2pindex < MAX_P2P_GROUP_SIZE)
	{
		pP2pEntry = &pAd->P2pTable.Client[p2pindex];
		pP2pEntry->ReTransmitCnt++;

		if (pP2pEntry->ReTransmitCnt >= 10)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ReTransmitCnt limit! stop connect this p2p device!\n"));
			P2pStopConnectThis(pAd);
		}

		pP2pEntry->GeneralToken++;
		switch(pP2pEntry->P2pClientState)
		{
			case P2PSTATE_SENT_PROVISION_REQ:
				if (pP2pCtrl->ConfigMethod == WSC_CONFMET_DISPLAY)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Re-Send PROVISION REQ. KEYPAD\n"));
					ConfigMthd = WSC_CONFMET_KEYPAD;
				}
				else if (pP2pCtrl->ConfigMethod == WSC_CONFMET_KEYPAD)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Re-Send PROVISION REQ. DISPLAY\n"));
					ConfigMthd = WSC_CONFMET_DISPLAY;
				}
				else if (pP2pCtrl->ConfigMethod == WSC_CONFMET_PBC)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Re-Send PROVISION REQ. PBC\n"));
					ConfigMthd = WSC_CONFMET_PBC;
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Re-Send PROVISION REQ. wrong Config Method(%d)\n", 
							pP2pCtrl->ConfigMethod));
					return;
				}
				P2PSendProvisionReq(pAd, ConfigMthd, pP2pEntry->GeneralToken, pP2pEntry->addr, &FrameLen);
				break;
			case P2PSTATE_SENT_GO_NEG_REQ:
				DBGPRINT(RT_DEBUG_ERROR, ("Re-Send GO NEGO REQ.\n"));
				P2pStartGroupForm(pAd, pP2pCtrl->ConnectingMAC, p2pindex);
				break;
			case P2PSTATE_SENT_PROVISION_RSP:
				if ( pP2pCtrl->bProvAutoRsp == TRUE )
					break;

				if ( pP2pCtrl->P2pProvUserNotify == 1 )
				{
					if ( pP2pEntry->ReTransmitCnt < 9 )
					{	
						P2PSendProvisionRsp(pAd, pP2pCtrl->P2pProvConfigMethod, pP2pCtrl->P2pProvToken, pP2pCtrl->ConnectingMAC, &FrameLen);
					}
					else
					{
						pP2pCtrl->P2pProvUserNotify = 0;
						pP2pCtrl->P2pProvIndex = P2P_NOT_FOUND;
						pP2pCtrl->P2pProvUserNotify = FALSE;
						pAd->P2pCfg.P2pCounter.UserAccept = 0;
					}
				}
				else
				{
					if ( pP2pEntry->ReTransmitCnt < 2 )
						P2PSendProvisionRsp(pAd, 0, pP2pCtrl->P2pProvToken, pP2pCtrl->ConnectingMAC, &FrameLen);
					else
						pP2pCtrl->P2pProvUserNotify = 0;
				}
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, ("ReSendTimeout execute with unknown state - %s\n", decodeP2PClientState(pP2pEntry->P2pClientState)));
		}
	}
}

VOID P2pCliReConnectTimeOut(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	PRT_P2P_CLIENT_ENTRY pP2pEntry = NULL;
	BOOLEAN			Cancelled;
	PRT_P2P_CONFIG	pP2pCtrl = &pAd->P2pCfg;
	UCHAR			BBPValue = 0;
	UCHAR			p2pindex = P2P_NOT_FOUND;
	ULONG			FrameLen;
	USHORT			ConfigMthd;


	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Send to %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(pP2pCtrl->ConnectingMAC)));
	pP2pCtrl->bP2pCliReConnectTimerRunning = FALSE;
	RTMPCancelTimer(&pP2pCtrl->P2pCliReConnectTimer, &Cancelled);

	/* Tear Down the P2P Connection */
	P2pLinkDown(pAd, P2P_DISCONNECTED);
}

/*	
	==========================================================================
	Description: 
		When I am P2P Client , Handle NoA Attribute.
		
	Parameters: 
		S - pointer to the association state machine
	Note:
		The state machine looks like the following as name implies its function
	==========================================================================
 */
BOOLEAN P2pHandleNoAAttri(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY	pMacClient,
	IN PUCHAR pData) 
{
	PP2P_NOA_DESC	pNoADesc;
	ULONG		Value, GPDiff;
	UCHAR		index;
	ULONG		NoALen;
	ULONG		StartTime;
	
	if (pMacClient == NULL)
		return FALSE;

	if ((*pData == SUBID_P2P_NOA))
	{
		NoALen = *(pData+1);
		if (NoALen == 2)
		{
			pMacClient->P2pInfo.CTWindow = *(pData+4); 
			if (pMacClient->P2pInfo.NoADesc[0].bValid == TRUE)
				P2pStopNoA(pAd, pMacClient);
			/*
				Copy my GO's CTWindow to P2Pcfg.CTWindow parameters, 
				Then As Client, I don't need to search for Client when I want to use CTWindow Value.
			 */
			pAd->P2pCfg.CTWindows = *(pData+4); 
			return TRUE;
		}
			
		index = *(pData+3);
		pMacClient->P2pInfo.CTWindow = *(pData+4);
		/* 
			Copy GO's CTWindow to P2Pcfg.CTWindow parameters, 
			Then As Client, I don't need to search for Client when I want to use CTWindow Value.
		 */
		pAd->P2pCfg.CTWindows = *(pData+4); 
		pNoADesc = (PP2P_NOA_DESC)(pData+5);
		pMacClient->P2pInfo.NoADesc[0].Count = pNoADesc->Count;
		pMacClient->P2pInfo.NoADesc[0].Duration = *(PUINT32)&pNoADesc->Duration[0];
		pMacClient->P2pInfo.NoADesc[0].Interval = *(PUINT32)&pNoADesc->Interval[0];
		pMacClient->P2pInfo.NoADesc[0].StartTime = *(PUINT32)&pNoADesc->StartTime[0];
		StartTime = *(PUINT32)&pNoADesc->StartTime[0];
	
		if (pMacClient->P2pInfo.NoADesc[0].Token == index)
		{
			/* The same NoA. Doesn't need to set this NoA again. */
			return FALSE;
		}
		
		DBGPRINT(RT_DEBUG_TRACE,("P2P : !!!NEW NOA Here =[%d, %d] Count = %d. Duration =  %ld \n", pMacClient->P2pInfo.NoADesc[0].Token, index, pNoADesc->Count, pMacClient->P2pInfo.NoADesc[0].Duration));
		DBGPRINT(RT_DEBUG_TRACE,("P2P : !!!NEW NOA Here =  CTWindow =  %x \n", pMacClient->P2pInfo.CTWindow));
		pMacClient->P2pInfo.NoADesc[0].Token = index;
		/*RTMP_IO_FORCE_READ32(pAd, TSF_TIMER_DW0, &Value); */
		Value = pAd->P2pCfg.GONoASchedule.LastBeaconTimeStamp;
		DBGPRINT(RT_DEBUG_TRACE,("Interval = %ld. StartTime = %ld. TSF timer Register = %ld\n", pMacClient->P2pInfo.NoADesc[0].Interval, pMacClient->P2pInfo.NoADesc[0].StartTime, Value));
		if ((pMacClient->P2pInfo.NoADesc[0].Duration <= 0x40) || (pMacClient->P2pInfo.NoADesc[0].Interval <= 0x40))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("!!!!!Interval or Duration too small. ignore.  = %lx return 1\n", Value));
			return FALSE;
		}
		else if ((pMacClient->P2pInfo.NoADesc[0].Duration >= pMacClient->P2pInfo.NoADesc[0].Interval)
			&& (pMacClient->P2pInfo.NoADesc[0].Count > 1))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("!!!!!Duration > Inveral.  return 2\n"));
			return FALSE;
		}
		
		/* if Start time point is in the future. */
		pAd->P2pCfg.GONoASchedule.CurrentTargetTimePoint = pMacClient->P2pInfo.NoADesc[0].StartTime;
		if (Value < StartTime)
		{
			GPDiff = pMacClient->P2pInfo.NoADesc[0].StartTime - Value;
			pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint = pMacClient->P2pInfo.NoADesc[0].StartTime + pMacClient->P2pInfo.NoADesc[0].Duration;
			pAd->P2pCfg.GONoASchedule.OngoingAwakeTime = pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint;
			pAd->P2pCfg.GONoASchedule.NextTimePointForWMMPSCounting = pMacClient->P2pInfo.NoADesc[0].StartTime;
			DBGPRINT(RT_DEBUG_TRACE,("!!!!! GPDiff = %ld = 0x%lx. NextTargetTimePoint = %ld\n", GPDiff, GPDiff, pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint));
			/* try to set General Timer. */
			pAd->P2pCfg.GONoASchedule.LastBeaconTimeStamp += GPDiff;
			if (P2pResetNoATimer(pAd, GPDiff))
			{
				DBGPRINT(RT_DEBUG_TRACE,("!!!!!Start NoA 1  GPDiff = %ld \n", GPDiff));
				pMacClient->P2pInfo.NoADesc[0].bValid = TRUE;
				pMacClient->P2pInfo.NoADesc[0].bInAwake = TRUE;
				pMacClient->P2pInfo.NoADesc[0].Token = index;
				return TRUE;
			}
		}
		/* else if Start time point is in the past. */
		else if (Value >= StartTime)
		{
			do
			{
				StartTime += pMacClient->P2pInfo.NoADesc[0].Interval;
				if ((StartTime > Value) && ((StartTime-Value) > 0x80))
				{
					GPDiff = StartTime - Value;
					pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint = StartTime /*+ pMacClient->P2pInfo.NoADesc[0].Interval*/ - pMacClient->P2pInfo.NoADesc[0].Duration;
					pAd->P2pCfg.GONoASchedule.OngoingAwakeTime = pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint;
					pAd->P2pCfg.GONoASchedule.LastBeaconTimeStamp += GPDiff;
					if (P2pResetNoATimer(pAd, GPDiff))
					{
						DBGPRINT(RT_DEBUG_TRACE,("!!!!!Start NoA 2  GPDiff = %ld\n", GPDiff));
						pMacClient->P2pInfo.NoADesc[0].bValid = TRUE;
						pMacClient->P2pInfo.NoADesc[0].bInAwake = TRUE;
						pMacClient->P2pInfo.NoADesc[0].Token = index;
						return TRUE;
					}
				}
			}while(TRUE);
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Start time in before ..!!Check \n"));
		}
	}
	return FALSE;

}

BOOLEAN 	P2pSetGP(
	IN PRTMP_ADAPTER pAd,
	IN	ULONG	DiffTimeInus)
{
	ULONG	GPDiff;
	ULONG	Value;
	
	GPDiff = (DiffTimeInus/64) & 0xffff;
	if (GPDiff > 0)
	{
		GPDiff = GPDiff<<16;
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, GPDiff);
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
		Value |= 0x2;
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
		return TRUE;
	}
	return FALSE;
}

BOOLEAN	P2pAdjustSwNoATimer(
	IN PRTMP_ADAPTER pAd,
	IN ULONG		CurrentTimeStamp, 
	IN ULONG		NextTimePoint) 
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	ULONG	AwakeDuration, NewStartTime;
	UCHAR		FakeNoAAttribute[32];
	
	RTMPZeroMemory(FakeNoAAttribute, 32);
	AwakeDuration = pP2PCtrl->GONoASchedule.Interval - pP2PCtrl->GONoASchedule.Duration;
	if (CurrentTimeStamp < pP2PCtrl->GONoASchedule.CurrentTargetTimePoint)
	{
		/* If offset is more than 1/4 of duration. */
		if ((pP2PCtrl->GONoASchedule.OngoingAwakeTime) >= (AwakeDuration>> 2))
		{
			DBGPRINT(RT_DEBUG_TRACE,("P2pAdjustSwNoATimer HERE HERE!!!! \n"));
			DBGPRINT(RT_DEBUG_TRACE,("OngoingAwakeTime = %ld. CurrentTimeStamp = %ld.!!!! \n", pP2PCtrl->GONoASchedule.OngoingAwakeTime, CurrentTimeStamp));
			P2pStopNoA(pAd, &pAd->MacTab.Content[pP2PCtrl->MyGOwcid]);
			FakeNoAAttribute[0] = SUBID_P2P_NOA;
			NewStartTime = pP2PCtrl->GONoASchedule.StartTime + (pP2PCtrl->GONoASchedule.SwTimerTickCounter - 1)*(pP2PCtrl->GONoASchedule.Interval);
			P2PMakeFakeNoATlv(pAd, NewStartTime, &FakeNoAAttribute[0]);
			pAd->MacTab.Content[pP2PCtrl->MyGOwcid].P2pInfo.NoADesc[0].Token--;
			P2pHandleNoAAttri(pAd, &pAd->MacTab.Content[pP2PCtrl->MyGOwcid], &FakeNoAAttribute[0]);
		}
		/* Update expected next Current Target Time Point with NextTimePoint */
		pP2PCtrl->GONoASchedule.CurrentTargetTimePoint = NextTimePoint;
		/* Can immediately dequeue packet because peer already in awake period. */
		return TRUE;
	}
	else
	{
		/* Update expected next Current Target Time Point with NextTimePoint */
		pP2PCtrl->GONoASchedule.CurrentTargetTimePoint = NextTimePoint;
		return FALSE;	
	}
}

VOID P2pGPTimeOutHandle(
	IN PRTMP_ADAPTER pAd) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	MAC_TABLE_ENTRY *pEntry;
	ULONG		MacValue;
	ULONG		Value;
	ULONG		GPDiff;
	ULONG		NextDiff;
	ULONG			 SavedNextTargetTimePoint;

	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &= 0xfffffffd;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	/* GO operating or Autonomous GO */
	if (P2P_GO_ON(pAd))
	{
		if (pP2PCtrl->GONoASchedule.bValid == TRUE)
		{
			if ((pP2PCtrl->GONoASchedule.Count > 0) && (pP2PCtrl->GONoASchedule.Count < 255))
			{
				/*
					Sometimes go to awake, sometime go to silence. Two state counts One count down.
					so only minus Count when I change from Sleep to Awake
				 */
				if (pP2PCtrl->GONoASchedule.bInAwake == FALSE)
					pP2PCtrl->GONoASchedule.Count--;
			}
			if (pP2PCtrl->GONoASchedule.Count == 0)
			{
				P2pStopNoA(pAd, NULL);
				DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle.!!StopGP.	return.1 \n"));
				return;
			}
				
			if (pP2PCtrl->GONoASchedule.bInAwake == TRUE)
				NextDiff = pP2PCtrl->GONoASchedule.Duration;
			else
				NextDiff = pP2PCtrl->GONoASchedule.Interval - pP2PCtrl->GONoASchedule.Duration;
					
			/* Prepare next time. */
			/*RTMP_IO_FORCE_READ32(pAd, TSF_TIMER_DW0, &Value); */
			Value = pAd->P2pCfg.GONoASchedule.LastBeaconTimeStamp;

			/* Check whether we should to renew the NoA because at least 2^31 us should update once according to spec. */
			if (pP2PCtrl->GONoASchedule.ThreToWrapAround > pP2PCtrl->GONoASchedule.StartTime)
			{
				if (Value > pP2PCtrl->GONoASchedule.ThreToWrapAround)
				{
					pP2PCtrl->GONoASchedule.bNeedResumeNoA = TRUE;
					P2pStopNoA(pAd, NULL);
					DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle.!!StopGP.	return.3. will resume.\n"));
					return;
				}
			}
			else
			{
				if ((Value > pP2PCtrl->GONoASchedule.ThreToWrapAround) && (Value < pP2PCtrl->GONoASchedule.StartTime))
				{
					pP2PCtrl->GONoASchedule.bNeedResumeNoA = TRUE;
					P2pStopNoA(pAd, NULL);
					DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle.!!StopGP.	return.4. will resume. \n"));
					return;
				}
			}

			SavedNextTargetTimePoint = pP2PCtrl->GONoASchedule.NextTargetTimePoint;
			if (Value <= pP2PCtrl->GONoASchedule.NextTargetTimePoint)
			{
				GPDiff = pP2PCtrl->GONoASchedule.NextTargetTimePoint - Value;
				pP2PCtrl->GONoASchedule.NextTimePointForWMMPSCounting = pP2PCtrl->GONoASchedule.NextTargetTimePoint;
				pP2PCtrl->GONoASchedule.NextTargetTimePoint += NextDiff;
				P2pResetNoATimer(pAd, GPDiff);
				DBGPRINT(RT_DEBUG_INFO,(" NextTargetTimePoint = %ld. \n", pAd->P2pCfg.GONoASchedule.NextTargetTimePoint));
				DBGPRINT(RT_DEBUG_INFO,("  Value = %ld.	GPDiff = %ld.\n", Value, GPDiff));
			}
			else
			{
				/* driver restart NoA due to our GP timer's delay. */
				if (pP2PCtrl->GONoASchedule.Count == 255)
				{
					pP2PCtrl->GONoASchedule.bNeedResumeNoA = TRUE;
					DBGPRINT(RT_DEBUG_TRACE,(" Prepare resume NoA. \n"));
				}
				P2pStopNoA(pAd, NULL);
				DBGPRINT(RT_DEBUG_TRACE,("3 NextTargetTimePoint = %ld. \n", pP2PCtrl->GONoASchedule.NextTargetTimePoint));
				DBGPRINT(RT_DEBUG_TRACE,(" 3 Value = %ld= 0x%lx NextDiff = %ld.\n", Value, Value, NextDiff));
				DBGPRINT(RT_DEBUG_TRACE,("3 P2pGPTimeOutHandle.!!StopGP.  return.2 \n"));
				return;
			}

			if (pP2PCtrl->GONoASchedule.bInAwake == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE,(" ----------------------->>> NoA  Go to SLEEP \n"));		
				pP2PCtrl->GONoASchedule.bWMMPSInAbsent = TRUE; /* Set to FALSE if changes state to absent */
				pP2PCtrl->bKeepSlient = TRUE;
				pP2PCtrl->bPreKeepSlient = TRUE;
				pP2PCtrl->GONoASchedule.bInAwake = FALSE; 
				/* roughly check that if duration > 100ms, we should call Pause beacon. */
				if (pP2PCtrl->GONoASchedule.Duration >= 0x19000)
				{
					P2pPauseBssSync(pAd);
				}
	

			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE,(" NoA	Wake UP ----------------------->>> \n"));
				pP2PCtrl->GONoASchedule.bInAwake = TRUE;
				pP2PCtrl->GONoASchedule.bWMMPSInAbsent = FALSE; /* Set to FALSE if changes state to Awake */
				pP2PCtrl->bKeepSlient = FALSE;	
				pP2PCtrl->bPreKeepSlient = FALSE;
				if (pP2PCtrl->GONoASchedule.Duration >= 0x19000)
				{
					P2pResumeBssSync(pAd);
				}
				if (IS_SW_NOA_TIMER(pAd)
					&& (pP2PCtrl->GONoASchedule.Count > 100))
				{
					if (TRUE == P2pAdjustSwNoATimer(pAd, Value, SavedNextTargetTimePoint))
					{
						/*DBGPRINT(RT_DEBUG_TRACE,("SwBasedNoA : Dequeue here. %d\n", pAd->TxSwNoAMgmtQueue.Number));*/
						/*RTMPDeQueueNoAMgmtPacket(pAd);*/
						RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
					}
				}
				else
				{
					/*RTMPDeQueueNoAMgmtPacket(pAd);*/
					RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
				}
			}

		}

	}
	else if (P2P_CLI_ON(pAd))
	{
		if (pP2PCtrl->NoAIndex >= MAX_LEN_OF_MAC_TABLE)
			return;
	
		if (pP2PCtrl->NoAIndex != pP2PCtrl->MyGOwcid)
			DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle. !bug, please check driver %d. \n", pP2PCtrl->NoAIndex));
				
		pEntry = &pAd->MacTab.Content[pP2PCtrl->NoAIndex];
		if (pEntry->P2pInfo.NoADesc[0].bValid == TRUE)
		{
			if ((pEntry->P2pInfo.NoADesc[0].Count > 0) && (pEntry->P2pInfo.NoADesc[0].Count < 255))
			{
				/*
					Sometimes go to awake, sometime go to silence. Two state counts One count down.
					so only minus Count when I change from Sleep to Awake
				 */
				if (pEntry->P2pInfo.NoADesc[0].bInAwake == FALSE)
					pEntry->P2pInfo.NoADesc[0].Count--;
			}
			if (pEntry->P2pInfo.NoADesc[0].Count == 0)
			{
				P2pStopNoA(pAd, pEntry);
				DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle. Count down to zero!!StopGP.  return.1 \n"));
				return;
			}
	
			/* To enter absence period, stop transmission a little bit earlier to leave HW to clean the queue. */
			if (pEntry->P2pInfo.NoADesc[0].bInAwake == FALSE)
				NextDiff = pEntry->P2pInfo.NoADesc[0].Duration - 0x200;
			else
				NextDiff = pEntry->P2pInfo.NoADesc[0].Interval - pEntry->P2pInfo.NoADesc[0].Duration + 0x200;

			/* Prepare next time. */
			MacValue = 0x333;
			/*RTMP_IO_READ32(pAd, TSF_TIMER_DW0, &MacValue); */
			MacValue = pAd->P2pCfg.GONoASchedule.LastBeaconTimeStamp;
			DBGPRINT(RT_DEBUG_INFO,("2 Tsf	Timer  = %ld= 0x%lx	 NextTargetTimePoint = %ld.\n", MacValue,  MacValue,pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint));
			SavedNextTargetTimePoint = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint;
			if (MacValue <= pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint)
			{
				GPDiff = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint - MacValue;
				pAd->P2pCfg.GONoASchedule.NextTimePointForWMMPSCounting = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint;
				pEntry->P2pInfo.NoADesc[0].NextTimePointForWMMPSCounting = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint;
				pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint += NextDiff;
				P2pResetNoATimer(pAd, GPDiff);
				DBGPRINT(RT_DEBUG_INFO,("3	Continue next NOA NextTargetTimePoint = %lx. \n", pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint));
				DBGPRINT(RT_DEBUG_INFO,("3	Value = %lx.  NextDiff = %lx.\n", MacValue, NextDiff));
			}
			else
			{
				P2pStopNoA(pAd, pEntry);
				DBGPRINT(RT_DEBUG_TRACE,("4  NOA NextTargetTimePoint = %ld. \n", pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint));
				DBGPRINT(RT_DEBUG_TRACE,("4  Value = %ld = 0x%lx.  NextDiff = %ld.\n", MacValue,  MacValue, NextDiff));
				return;
			}
					
			if (pEntry->P2pInfo.NoADesc[0].bInAwake == TRUE)
			{
				pEntry->P2pInfo.NoADesc[0].bInAwake = FALSE;
				pP2PCtrl->bKeepSlient = TRUE;
				pP2PCtrl->bPreKeepSlient = TRUE;
				DBGPRINT(RT_DEBUG_TRACE,("Enter Absence now ======> %d\n", pP2PCtrl->bKeepSlient));
			}
			else
			{
				pEntry->P2pInfo.NoADesc[0].bInAwake = TRUE;
				pP2PCtrl->bKeepSlient = FALSE;
				pP2PCtrl->bPreKeepSlient = FALSE;
				if (IS_SW_NOA_TIMER(pAd)
					&& (pP2PCtrl->GONoASchedule.Count > 100))
				{
					if (TRUE == P2pAdjustSwNoATimer(pAd, Value, SavedNextTargetTimePoint))
					{
						/*DBGPRINT(RT_DEBUG_TRACE,("SwBasedNoA : Dequeue here. %d\n", pAd->TxSwNoAMgmtQueue.Number));*/
						/*RTMPDeQueueNoAMgmtPacket(pAd);*/
						RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
					}
				}
				else
				{
					/*RTMPDeQueueNoAMgmtPacket(pAd);*/
					RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
				}
				DBGPRINT(RT_DEBUG_TRACE,("Enter Awake now ======= %d\n", pAd->P2pCfg.bKeepSlient));
	
			}
	
		}
					
	}

}

/*	
	==========================================================================
	Description: 

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID P2pPauseBssSync(
	IN PRTMP_ADAPTER pAd)
{
	/*BCN_TIME_CFG_STRUC csr;*/
	
	DBGPRINT(RT_DEBUG_TRACE, ("--->P2pPauseBssSync  %s\n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
	AsicDisableSync(pAd);
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID P2pResumeBssSync(
	IN PRTMP_ADAPTER pAd)
{
	/*BCN_TIME_CFG_STRUC csr;*/
	
	DBGPRINT(RT_DEBUG_TRACE, ("--->P2pResumeBssSync  %s\n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
	AsicEnableP2PGoSync(pAd);
}

/*	
	==========================================================================
	Description: 
		OppPS CTWindows timer. 
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PCTWindowTimer(
	IN PVOID	SystemSpecific1, 
	IN PVOID	FunctionContext, 
	IN PVOID	SystemSpecific2, 
	IN PVOID	SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	PRT_P2P_CONFIG pP2pCtrl = &pAd->P2pCfg;

	if (P2P_TEST_BIT(pP2pCtrl->CTWindows, P2P_OPPS_BIT))
		pP2pCtrl->bKeepSlient = TRUE;
}

/*	
	==========================================================================
	Description: 
		Before reinvoke a persistent group, copy persistent parameter to pAd->P2pCfg.. 
		
	Parameters: 
		Perstindex : the index for entry in Persistent Table.
	Note:
		 
	==========================================================================
 */
VOID P2pCopyPerstParmToCfg(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		Perstindex)
{

}

/*	
	==========================================================================
	Description: 
		Get Random SSID "DIRECT-xx" 
		
	Parameters: 
		pSSID : output SSID
		pSSIDLen : Length of pSSID
	Note:
		 
	==========================================================================
 */
VOID P2pGetRandomSSID(
	IN PRTMP_ADAPTER pAd,
	OUT PSTRING pSSID,
	OUT PUCHAR pSSIDLen)
{
	UCHAR tmp[52];
	UCHAR i;
	//gen a-z , A-Z tmp array!
	for(i=0 ; i < 26 ; i++)
	{
		tmp[i]='a'+i;
		tmp[i+26]='A'+i;
	}
	
	NdisMoveMemory(pSSID, "DIRECT-", 7);
	pSSID[7] = tmp[RandomByte(pAd)%52];
	pSSID[8] = tmp[RandomByte(pAd)%52];
	NdisMoveMemory((pSSID + 9), pAd->P2pCfg.DeviceName, pAd->P2pCfg.DeviceNameLen);
	(*pSSIDLen) = 9 + pAd->P2pCfg.DeviceNameLen;
}

/*	
	==========================================================================
	Description: 
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pSetListenIntBias(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		Bias)
{
	pAd->P2pCfg.P2pCounter.ListenIntervalBias = Bias;
	if (INFRA_ON(pAd))
		pAd->P2pCfg.P2pCounter.ListenIntervalBias = 1;
}


/*	
	==========================================================================
	Description: 
		The routine that decide my Rule as GO or Client? 
		And then do necessary setting : update channel, Bssid, SSID etc.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pSetRule(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		Index,
	IN PUCHAR		PeerBssid,
	IN UCHAR		PeerGOIntentAttri,
	IN UCHAR		Channel)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAd->P2pTable.Client[Index];
	UCHAR		RealIntent;
	USHORT		PeerWscMethod;

	DBGPRINT(RT_DEBUG_ERROR, (" P2pSetRule - Channel = %d. GoIntent = %x\n", Channel, PeerGOIntentAttri));
	RealIntent = PeerGOIntentAttri>>1;
	if (RealIntent > pP2PCtrl->GoIntentIdx)
	{
		pP2pEntry->Rule = P2P_IS_GO;
		pP2pEntry->GoIntent = RealIntent;
		/* Use peer addr as bssid */
		RTMPMoveMemory(pP2PCtrl->PortCfg.Bssid, PeerBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pP2PCtrl->Bssid, PeerBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pP2PCtrl->SSID, pP2pEntry->Ssid, 32);
		pP2PCtrl->SSIDLen = pP2pEntry->SsidLen;
		pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
		
		/*
			The Operating Channel attribute may be present in the P2P IE.
			If Channel is 0, use self configuration.
		*/
		if (Channel != 0)
		pP2PCtrl->GroupOpChannel = Channel;
		else			
			pP2PCtrl->GroupOpChannel = pP2PCtrl->GroupChannel;
		
		/* Update My WPS Mode. */
		P2P_SetWscRule(pAd, Index, &PeerWscMethod);
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
	}
	else if (RealIntent < pP2PCtrl->GoIntentIdx)
	{
		pP2pEntry->Rule = P2P_IS_CLIENT;
		pP2pEntry->GoIntent = RealIntent;
		/* Use my addr as bssid */
		RTMPMoveMemory(pP2PCtrl->Bssid, pP2PCtrl->CurrentAddress, MAC_ADDR_LEN);
		RTMPMoveMemory(pP2PCtrl->PortCfg.Bssid, pP2PCtrl->CurrentAddress, MAC_ADDR_LEN);
		pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_GO;
		pP2PCtrl->GroupOpChannel = pP2PCtrl->GroupChannel;

		/* Update My WPS Config Method. */
		P2P_SetWscRule(pAd, Index, &PeerWscMethod);
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - MyIntent = %d, PeerIntent = %d\n", pP2PCtrl->GoIntentIdx, RealIntent));
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
	}
	else if (((PeerGOIntentAttri&1) == 1) && (RealIntent == pP2PCtrl->GoIntentIdx))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("P2pSetRule Peer Tie Breaker bit is On.  %x \n", RealIntent));
		pP2pEntry->Rule = P2P_IS_GO;
		pP2pEntry->GoIntent = RealIntent;
		/* Use peer addr as bssid */
		RTMPMoveMemory(pP2PCtrl->PortCfg.Bssid, PeerBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pP2PCtrl->Bssid, PeerBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pP2PCtrl->SSID, pP2pEntry->Ssid, 32);
		pP2PCtrl->SSIDLen = pP2pEntry->SsidLen;
		pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
		/*
			The Operating Channel attribute may be present in the P2P IE.
			If Channel is 0, use self configuration.
		*/
		if (Channel != 0)
		pP2PCtrl->GroupOpChannel = Channel;
		else			
			pP2PCtrl->GroupOpChannel = pP2PCtrl->GroupChannel;
		
		/* Update My WPS Config Method. */
		P2P_SetWscRule(pAd, Index, &PeerWscMethod);
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
	}
	else if (((PeerGOIntentAttri & 0x1) == 0) && (RealIntent == pP2PCtrl->GoIntentIdx))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("P2pSetRule Peer Tie Breaker bit is Off.  %x \n", RealIntent));
		pP2pEntry->Rule = P2P_IS_CLIENT;
		pP2pEntry->GoIntent = RealIntent;
		/* Use my addr as bssid */
		RTMPMoveMemory(pP2PCtrl->Bssid, pP2PCtrl->CurrentAddress, MAC_ADDR_LEN);
		RTMPMoveMemory(pP2PCtrl->PortCfg.Bssid, pP2PCtrl->CurrentAddress, MAC_ADDR_LEN);
		pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_GO;
		pP2PCtrl->GroupOpChannel = pP2PCtrl->GroupChannel;

		/* Update My WPS Config Method. */
		P2P_SetWscRule(pAd, Index, &PeerWscMethod);
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
	}

	DBGPRINT(RT_DEBUG_ERROR, ("P2pSetRule pPort Bssid = %02x %02x %02x %02x %02x %02x  \n",  pP2PCtrl->PortCfg.Bssid[0], pP2PCtrl->PortCfg.Bssid[1],pP2PCtrl->PortCfg.Bssid[2],pP2PCtrl->PortCfg.Bssid[3],pP2PCtrl->PortCfg.Bssid[4],pP2PCtrl->PortCfg.Bssid[5]));
	DBGPRINT(RT_DEBUG_ERROR, ("GroupOpChannel =  %d  \n",pP2PCtrl->GroupOpChannel));
	DBGPRINT(RT_DEBUG_ERROR, (" P2pSetRule - pP2pEntry->Rule = %s. \n", decodeMyRule(pP2pEntry->Rule)));

}

/*	
	==========================================================================
	Description: 
		Start Provisionint process.  Send out Provision Discovery Request frame.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pProvision(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr)
{
	UCHAR		p2pindex, Channel;
	
	DBGPRINT(RT_DEBUG_ERROR, ("P2pProvision = %02x:%02x:%02x:%02x:%02x:%02x.  \n",  PRINT_MAC(Addr)));
	p2pindex = P2pGroupTabSearch(pAd, Addr);
	if (p2pindex < MAX_P2P_GROUP_SIZE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Start P2pProvision = %02x:%02x:%02x:%02x:%02x:%02x.  \n",  PRINT_MAC(Addr)));
		pAd->P2pTable.Client[p2pindex].GeneralToken++;
		pAd->P2pTable.Client[p2pindex].P2pClientState = P2PSTATE_PROVISION_COMMAND;

		/* Stop Scan and switch to peer's Listen Channel. */
		P2pStopScan(pAd);
		Channel = pAd->P2pTable.Client[p2pindex].ListenChannel;
		AsicSwitchChannel(pAd, Channel, FALSE);
		AsicLockChannel(pAd, Channel);
		pAd->P2pCfg.DiscCurrentState = P2P_DISC_SEARCH;
		COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pAd->P2pTable.Client[p2pindex].addr);
		return TRUE;
	}
	return FALSE;

}


/*	
	==========================================================================
	Description: 
		Connect to Addr. Include all "Connect" cenario. GO_nego, invite, etc. Addr might be P2P GO, or a P2P Device.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pConnect(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR	Channel, p2pIdx;

	/* Stop Scan and switch to peer's Listen Channel. */
	P2pStopScan(pAd);
	p2pIdx = P2pGroupTabSearch(pAd, pAd->P2pCfg.ConnectingMAC);
	if (p2pIdx >= MAX_P2P_GROUP_SIZE)
		return FALSE;

	Channel = pAd->P2pTable.Client[p2pIdx].ListenChannel;
	if (Channel == 0)
		Channel = 1;
	AsicSwitchChannel(pAd, Channel, FALSE);
	AsicLockChannel(pAd, Channel);
	pAd->P2pCfg.DiscCurrentState = P2P_DISC_SEARCH;

	DBGPRINT(RT_DEBUG_ERROR, ("P2pConnect =  \n"));
	DBGPRINT(RT_DEBUG_ERROR, ("Addr = %02x:%02x:%02x:%02x:%02x:%02x.  \n",  PRINT_MAC(pAd->P2pCfg.ConnectingMAC)));
	DBGPRINT(RT_DEBUG_ERROR, ("ListenChannel =  %d\n", Channel));

	return TRUE;
}

VOID P2pConnectPrepare(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	Addr,
	IN UINT32 ConnType)
{
	UCHAR	Channel, p2pindex;

	p2pindex = P2pGroupTabSearch(pAd, Addr);
	if (p2pindex < MAX_P2P_GROUP_SIZE)
	{
		pAd->P2pCfg.bConfirmByUI = TRUE;
		/* Stop Scan and switch to peer's Listen Channel. */
		P2pStopScan(pAd);
		OS_WAIT(200);

		pAd->P2pCfg.bPeriodicListen = FALSE;

		if ((pAd->P2pTable.Client[p2pindex].P2pClientState == P2PSTATE_DISCOVERY_GO))
			Channel = pAd->P2pTable.Client[p2pindex].OpChannel;
		else
			Channel = pAd->P2pTable.Client[p2pindex].ListenChannel;

		if (INFRA_ON(pAd) || P2P_GO_ON(pAd))
		{
			if (pAd->CommonCfg.BBPCurrentBW == BW_40)
			{
				UCHAR BBPValue = 0;

				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
				BBPValue &= (~0x18);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			}
		}

		AsicSwitchChannel(pAd, Channel, FALSE);
		AsicLockChannel(pAd, Channel);

		/* Copy pEntry MAC Address in ConnectinfMAC. */
		pAd->P2pCfg.ConnectingIndex = 0;
		COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pAd->P2pTable.Client[p2pindex].addr);

		/* Set Connecting Timer for Periodic Timer use. */
		pAd->P2pTable.Client[p2pindex].StateCount = 0;
		pAd->P2pTable.Client[p2pindex].bValid = TRUE;

		/* Set pEntry Client Status. */
		if ((pAd->P2pTable.Client[p2pindex].P2pClientState == P2PSTATE_DISCOVERY_GO))
		{
			COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pAd->P2pTable.Client[p2pindex].bssid);
			pAd->P2pTable.Client[p2pindex].P2pClientState = P2PSTATE_PROVISION_COMMAND;
		}
		else
			pAd->P2pTable.Client[p2pindex].P2pClientState = ConnType;
		
		DBGPRINT(RT_DEBUG_ERROR, ("P2P Connect Command Type =  %s\n", decodeP2PClientState(ConnType)));
		P2PPrintP2PEntry(pAd, p2pindex);
	
		/* Change Discovery State Machine State. */
		pAd->P2pCfg.DiscCurrentState = P2P_DISC_SEARCH;
		pAd->P2pCfg.GoFormCurrentState = P2P_GO_FORM_IDLE;
	}
}

/*	
	==========================================================================
	Description: 
		Ready to connect to Addr. Include all "Connect" cenario. GO_nego, invite, etc. Addr might be P2P GO, or a P2P Device.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pConnectAfterScan(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN	bBeacon,
	IN UCHAR		idx)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	BOOLEAN	brc = TRUE;
	UCHAR		index;/*, i; */
	UCHAR		GrpIndex = idx;
	BOOLEAN		bAction = FALSE;
	
	DBGPRINT(RT_DEBUG_ERROR, (" P2pConnectAfterScan   %d. %s \n", pAd->P2pTable.Client[idx].P2pClientState, decodeP2PClientState(pAd->P2pTable.Client[idx].P2pClientState)));
	DBGPRINT(RT_DEBUG_ERROR, (" GroupOpChannel  = %d  \n", pP2PCtrl->GroupOpChannel));

	if (GrpIndex >= MAX_P2P_GROUP_SIZE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectAfterScan Check - Reach Device Limit. return.  \n"));
		return FALSE;
	}
	
	/*
		peer is a P2P GO.
		Peer is a P2P Device. Need to have Go Nego procedure first.
		Or can reinvoke a persistent
	*/
	if (pAd->P2pTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY)
	{
		/* Decide connect method when I am in Connect_Idle state. This means I am a P2P Device. */
		if (P2P_CLI_ON(pAd) || (P2P_GO_ON(pAd)))
		{
			/* Invite Case 1 */
			pAd->P2pTable.Client[GrpIndex].P2pClientState = P2PSTATE_INVITE_COMMAND;
			DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan -  Use Invite %d.\n", GrpIndex));
			bAction = TRUE;
		}
		else if (IS_P2P_CONNECT_IDLE(pAd))
		{
			index = P2pPerstTabSearch(pAd, pAd->P2pTable.Client[GrpIndex].addr, 
										pAd->P2pTable.Client[GrpIndex].bssid, 
										pAd->P2pTable.Client[GrpIndex].InterfaceAddr);
			DBGPRINT(RT_DEBUG_ERROR, (" P2pConnectAfterScan - Perst index %d.  \n", index));
			DBGPRINT(RT_DEBUG_ERROR, (" P2pConnectAfterScan - EnablePresistent %d.  \n", 
					pP2PCtrl->P2pControl.field.EnablePresistent));
			if ((index < MAX_P2P_TABLE_SIZE) && IS_PERSISTENT_ON(pAd))
			{
				/* Invite Case 3: */
				pAd->P2pTable.Client[GrpIndex].P2pClientState = P2PSTATE_INVITE_COMMAND;
				DBGPRINT(RT_DEBUG_ERROR, (" P2pConnectAfterScan -  Use reinvoke Invite %d.  \n", index));
				bAction = TRUE;
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan -  Start From Group Forming   \n"));

				/* Start Scan and Then go to Group Forming process. */
				pAd->P2pTable.Client[idx].P2pClientState = P2PSTATE_CONNECT_COMMAND;
				bAction = TRUE;
			}
		}
		if (bAction == TRUE)
		{
			/* Now only support connect to ONE. So set ConnectingIndex to MAX_P2P_GROUP_SIZE to stop connect further MAC. */
			P2pConnectAction(pAd, bBeacon, idx);
		}
	}
	else if ((pAd->P2pTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY_GO))
	{
		DBGPRINT(RT_DEBUG_TRACE, (" case 1  = peer is go  \n"));
		P2pStopConnectThis(pAd);
		brc = P2pConnectP2pGo(pAd, GrpIndex);
	}
	/* peer is a P2P client in a P2P Group. */
	else if ((pAd->P2pTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY_CLIENT))
	{
		DBGPRINT(RT_DEBUG_TRACE, (" case 2  = peer is client  \n"));
		P2pStopConnectThis(pAd);
		P2pConnectP2pClient(pAd,GrpIndex);
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan - Peer state %s \n", 
				decodeP2PClientState(pAd->P2pTable.Client[GrpIndex].P2pClientState)));

	return brc;
}

/*	
	==========================================================================
	Description: 
		Prepare to connect to Connecting MAC. ConnectingMAC might contain several MAC address that I can connect to One after one.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pConnectAction(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN	bBeacon,
	IN UCHAR		index)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	BOOLEAN		bresult = FALSE;
	UCHAR		perstindex;

	DBGPRINT(RT_DEBUG_ERROR, (" P2pConnectAction   %d. %s \n", pAd->P2pTable.Client[index].P2pClientState, decodeP2PClientState(pAd->P2pTable.Client[index].P2pClientState)));

	if (bBeacon == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2pConnectAction   from bBeacon \n"));
		/* Check If ever have a command to connect to this peer. */
		if (pAd->P2pTable.Client[index].P2pClientState == P2PSTATE_CONNECT_COMMAND
			||(pAd->P2pTable.Client[index].P2pClientState == P2PSTATE_INVITE_COMMAND))
		{
			pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
			P2pGoNegoDone(pAd, &pAd->P2pTable.Client[index]);
		}
		else if (pAd->P2pTable.Client[index].P2pClientState == P2PSTATE_GO_DISCO_COMMAND)
		{
			bresult = P2pClientDiscovery(pAd, pAd->P2pTable.Client[index].addr, index);
		}
	}
	else
	{		
		DBGPRINT(RT_DEBUG_ERROR, (" P2pConnectAction  not from bBeacon \n"));
		if ((pAd->P2pTable.Client[index].P2pClientState == P2PSTATE_CONNECT_COMMAND)
			|| (pAd->P2pTable.Client[index].P2pClientState == P2PSTATE_INVITE_COMMAND))
		{
			/* The check sequence must be the same as in where we set P2PSTATE_INVITE_COMMAND in P2PConnect() */
			if ((P2P_GO_ON(pAd)) || (P2P_CLI_ON(pAd)))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("  case 1 \n"));
				/* Invite Case 1 : I am Auto GO to invite a P2P Device or when I am P2P Client */
				bresult = P2pInvite(pAd, pAd->P2pTable.Client[index].addr, MAX_P2P_TABLE_SIZE, index);
			}
			else if (pAd->P2pTable.Client[index].Rule == P2P_IS_GO)
			{

				DBGPRINT(RT_DEBUG_TRACE, ("  case 2 \n"));
				/* directly associate to  GO. */
				pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
				P2pGoNegoDone(pAd, &pAd->P2pTable.Client[index]);
			}
			else if (IS_P2P_CONNECT_IDLE(pAd))
			{
				UCHAR Channel;
				P2P_CMD_STRUCT	P2pCmd;
				UCHAR ClientState = pAd->P2pTable.Client[index].P2pClientState;
				DBGPRINT(RT_DEBUG_ERROR, ("  case 3 \n"));
				/* since I am idle,  */
				perstindex = P2pPerstTabSearch(pAd, pAd->P2pTable.Client[index].addr, 
												pAd->P2pTable.Client[index].bssid, 
												pAd->P2pTable.Client[index].InterfaceAddr);
				if ((perstindex < MAX_P2P_TABLE_SIZE) && (IS_PERSISTENT_ON(pAd)))
				{
					/*
						I have credential, my persistent is enabled, peer 's persistent is enabled.  
						So use Reinvoke method to start P2P group.
						Stop Scan and switch to peer's Listen Channel.
					*/
					P2pStopScan(pAd);		
					Channel = pAd->P2pTable.Client[index].ListenChannel;
					AsicSwitchChannel(pAd, Channel, FALSE);
					AsicLockChannel(pAd, Channel);

					ClientState = P2PSTATE_INVITE_COMMAND;
					pAd->P2pCfg.P2PConnectState = P2P_CONNECT_IDLE;
				}
				else
				{
					ClientState = P2PSTATE_CONNECT_COMMAND;		
				}
				COPY_MAC_ADDR(&P2pCmd.Addr[0], pAd->P2pTable.Client[index].addr);
				P2pCmd.Idx = index;			
				MlmeEnqueue(pAd, P2P_GO_FORM_STATE_MACHINE, P2P_START_COMMUNICATE_CMD_EVT, sizeof(P2P_CMD_STRUCT), &P2pCmd, ClientState);
				RTMP_MLME_HANDLER(pAd);
			}
			else
				DBGPRINT(RT_DEBUG_TRACE, ("  case 4 ?????? \n"));
			if (bresult == TRUE)
			{
				/* do nothing */
			}
		}
		
	}
	pP2PCtrl->P2pCounter.GoScanBeginCounter100ms = 0;
}


/*	
	==========================================================================
	Description: 
		Connect to Addr. Addr is already in a P2P Group.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pConnectP2pClient(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR		GrpIndex)
{
	BOOLEAN		bGoToScan = FALSE;
	UCHAR		GoP2pIndex;
	
	DBGPRINT(RT_DEBUG_TRACE, ("P2pConnect to P2pClient====>.  \n"));
	
	/* Decide connect method when I am in COnnect_Idle state. This means I am a P2P Device. */
	if (IS_P2P_CONNECT_IDLE(pAd))
	{
		GoP2pIndex = pAd->P2pTable.Client[GrpIndex].MyGOIndex;
		if ((pAd->P2pTable.Client[GrpIndex].DevCapability & DEVCAP_CLIENT_DISCOVER) == 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("P2P Client Not support Discoverability  %x \n", pAd->P2pTable.Client[GrpIndex].DevCapability));
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("P2P Client CAN Support Discoverability  %x \n", pAd->P2pTable.Client[GrpIndex].DevCapability));

		DBGPRINT(RT_DEBUG_TRACE, ("P2P Client DevCapability  %x. GoP2pIndex = %d. \n", pAd->P2pTable.Client[GrpIndex].DevCapability, GoP2pIndex));
		if ((GoP2pIndex < MAX_P2P_GROUP_SIZE) && (pAd->P2pTable.Client[GoP2pIndex].P2pClientState == P2PSTATE_DISCOVERY_GO))
		{
			pAd->P2pTable.Client[GrpIndex].P2pClientState = P2PSTATE_CLIENT_DISCO_COMMAND;
			pAd->P2pTable.Client[GoP2pIndex].P2pClientState = P2PSTATE_GO_DISCO_COMMAND;
			DBGPRINT(RT_DEBUG_TRACE,("P2P  GrpIndex  %d . GoP2pIndex = %d\n", GrpIndex, GoP2pIndex));
			bGoToScan = TRUE;	
		}
		DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectIdle - peer device's state %s \n", decodeP2PClientState(pAd->P2pTable.Client[GrpIndex].P2pClientState)));
	}

	if (bGoToScan == TRUE)
	{
		P2pSetListenIntBias(pAd, 12);
		pAd->P2pCfg.P2pCounter.CounterAftrScanButton = 0;
		P2pGotoScan(pAd);
	}

}

/*	
	==========================================================================
	Description: 
		Connect to Addr. Addr is already in a P2P Group.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pConnectP2pGo(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR		idx)
{
	BOOLEAN	brc = FALSE;
	
	DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectP2pGo.  %s\n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
	DBGPRINT(RT_DEBUG_TRACE, ("pAd->P2pCfg.Dpid = %d \n", pAd->P2pCfg.Dpid));
	/* Decide connect method when I am in COnnect_Idle state. This means I am a P2P Device. */
	if (P2P_GO_ON(pAd))
		return brc;

	if (pAd->P2pCfg.P2PConnectState == P2P_CONNECT_IDLE)
	{
		pAd->P2pTable.Client[idx].P2pClientState = P2PSTATE_PROVISION_COMMAND;
		DBGPRINT(RT_DEBUG_TRACE,("P2p : Use Provision first before connecting to GO with Bssid   %x.  %x. %x.  %x. %x.  %x. \n", pAd->P2pTable.Client[idx].bssid[0], pAd->P2pTable.Client[idx].bssid[1],pAd->P2pTable.Client[idx].bssid[2],pAd->P2pTable.Client[idx].bssid[3],pAd->P2pTable.Client[idx].bssid[4],pAd->P2pTable.Client[idx].bssid[5]));
		DBGPRINT(RT_DEBUG_TRACE,("P2p : its GroupCapability= %x.  DevCapability= %x. \n", pAd->P2pTable.Client[idx].GroupCapability, pAd->P2pTable.Client[idx].DevCapability));
		brc = TRUE;
	}
	else if (P2P_CLI_ON(pAd))
	{
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("invalid P2pConnectP2pGo command when %s \n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));

	return brc;

}

/*	
	==========================================================================
	Description: 
		Try to connect to a P2P Client.  So Use Client discovery first. CLient Discvoery frame is 
		forwarded by the help of GO.  
		
	Parameters: 
		P2pTabIdx is GO's index in P2P table.
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pClientDiscovery(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR 		GoP2pTabIdx)
{
	UCHAR		ClientP2PIndex = P2P_NOT_FOUND;
	UCHAR		i;
	ULONG		TotalFrameLen;
	
	DBGPRINT(RT_DEBUG_ERROR, ("P2pClientDiscovery  %d\n", GoP2pTabIdx));
	if (GoP2pTabIdx >= MAX_P2P_GROUP_SIZE)
		return FALSE;

	/* Search what is the P2P client that I was about to ask it's existence. */
	for (i = 0; i < MAX_P2P_GROUP_SIZE;i++)
	{
		if (pAd->P2pTable.Client[i].P2pClientState == P2PSTATE_CLIENT_DISCO_COMMAND)
			ClientP2PIndex = i;
	}
	DBGPRINT(RT_DEBUG_ERROR, ("P2pClientDiscovery.  ClientP2PIndex= %d\n", ClientP2PIndex));
	if (ClientP2PIndex == P2P_NOT_FOUND)
		return FALSE;

	P2PSendDevDisReq(pAd, pAd->P2pTable.Client[GoP2pTabIdx].addr, pAd->P2pTable.Client[GoP2pTabIdx].addr, pAd->P2pTable.Client[ClientP2PIndex].addr, &TotalFrameLen);
	if (TotalFrameLen > 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("P2pClientDiscovery. ClientP2PIndex = %d\n", ClientP2PIndex));
		return TRUE;
	}
	else	
		return FALSE;

}

/*	
	==========================================================================
	Description: 
		Invite can be used for reinvoke persistent entry or just to connect to an P2P client. So 
		Has 2 index for those choices. Only one is used.  
		invitation procedure is  an optional procedure used for the following :
		1. A P2P group owner inviting a P2P Device to become a P2P Client in its P2P Group.
		2. A P2P client invitin another P2P Device to join the P2P Group of which the P2P Client is a member 
		because it wished to use some service of the P2P Device.
		3. Requesting to invoke a Persistent P2P Group for which both P2P Devices have previouslt been provisioned.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pInviteAsRule(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR		MyRule,
	IN UCHAR 		P2pTabIdx)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	RT_P2P_CLIENT_ENTRY	*pClient;
	ULONG		FrameLen;

	/* Reflash this flag on the next Invitation or DPID setting. */
	DBGPRINT(RT_DEBUG_TRACE, ("P2pInviteAsRule MyRule = %d. P2pTabIdx= %d \n",  MyRule, P2pTabIdx));
	pClient = &pAd->P2pTable.Client[P2pTabIdx];
	P2PMakeInviteReq(pAd, MyRule, 0, pClient->addr, pP2PCtrl->CurrentAddress, &FrameLen);

	DBGPRINT(RT_DEBUG_TRACE, ("P2pInviteAsRule FrameLen = %ld.  \n", FrameLen));
	if (FrameLen > 0)
		return TRUE;

	return FALSE;

}

/*	
	==========================================================================
	Description: 
		Invite can be used for reinvoke persistent entry or just to connect to an P2P client. So 
		Has 2 index for those choices. Only one is used.  
		invitation procedure is  an optional procedure used for the following :
		1. A P2P group owner inviting a P2P Device to become a P2P Client in its P2P Group.
		2. A P2P client invitin another P2P Device to join the P2P Group of which the P2P Client is a member 
		because it wished to use some service of the P2P Device.
		3. Requesting to invoke a Persistent P2P Group for which both P2P Devices have previouslt been provisioned.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pInvite(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR		PersistentTabIdx, 
	IN UCHAR 		P2pTabIdx)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	RT_P2P_PERSISTENT_ENTRY *pEntry;
	RT_P2P_CLIENT_ENTRY	*pClient;
/*	UCHAR		index; */
	ULONG		FrameLen;
	BOOLEAN		brc = FALSE;
	
	/* Get one index that is valid to use during invitation procedure. */
	if (PersistentTabIdx < MAX_P2P_TABLE_SIZE)
	{
		/* those who can save to persistent table must also support persistent . So can use invite procedure now. */
		pEntry = &pAd->P2pTable.PerstEntry[PersistentTabIdx];
		if (P2pTabIdx < MAX_P2P_GROUP_SIZE)
		{
			pAd->P2pCfg.PhraseKeyLen = (UCHAR)pEntry->Profile.KeyLength;
			RTMPMoveMemory(pAd->P2pCfg.PhraseKey, pEntry->Profile.Key, pEntry->Profile.KeyLength);
			pClient = &pAd->P2pTable.Client[P2pTabIdx];
			RTMPMoveMemory(pAd->P2pCfg.SSID, pEntry->Profile.SSID.Ssid, 32);
			pP2PCtrl->SSIDLen = (UCHAR)pEntry->Profile.SSID.SsidLength;
			RTMPMoveMemory(pP2PCtrl->SSID, pEntry->Profile.SSID.Ssid, pP2PCtrl->SSIDLen);

			if (pEntry->MyRule == P2P_IS_CLIENT)
			{
				pClient->Rule = P2P_IS_GO;
				pClient->P2pClientState = P2PSTATE_GO_WPS;
				RTMPMoveMemory(pAd->P2pCfg.Bssid, pEntry->Addr, MAC_ADDR_LEN);
			}
			else
			{
				pClient->Rule = P2P_IS_CLIENT;
				pClient->P2pClientState = P2PSTATE_CLIENT_WPS;
				RTMPMoveMemory(pAd->P2pCfg.Bssid, pAd->CurrentAddress, MAC_ADDR_LEN);
			}
			P2PMakeInviteReq(pAd, pEntry->MyRule, P2P_INVITE_FLAG_REINVOKE, pEntry->Addr, pP2PCtrl->CurrentAddress, &FrameLen);
			if (FrameLen > 0)
			{
				brc = TRUE;
				pClient->P2pClientState = P2PSTATE_SENT_INVITE_REQ;
			}
		}
	}
	else if (P2pTabIdx < MAX_P2P_GROUP_SIZE)
	{
		if ((pAd->P2pTable.Client[P2pTabIdx].DevCapability & DEVCAP_INVITE) == DEVCAP_INVITE)
		{
			/*if (pP2PCtrl->PortSubtype == PORTSUBTYPE_P2PGO) */
			if (P2P_GO_ON(pAd))
			{
				brc = P2pInviteAsRule(pAd, P2P_IS_GO, P2pTabIdx);
			}
			else if (P2P_CLI_ON(pAd))
				brc = P2pInviteAsRule(pAd, P2P_IS_CLIENT, P2pTabIdx);
				
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("Peer doesn't support Invite. DevCapability = %x \n", pAd->P2pTable.Client[P2pTabIdx].DevCapability));
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("invalid P2pInvite command. %d %d \n",  PersistentTabIdx, P2pTabIdx));

	return  brc;

}



/*	
	==========================================================================
	Description: 
		Start Group Formation Process.   will send out Go Negociation Request frame.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pStartGroupForm(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN UCHAR		idx)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	NDIS_STATUS   NStatus;
	PUCHAR        pOutBuffer = NULL;
	ULONG         FrameLen = 0;
	
	/* Insert this candidate to p2p client table. */
	if (idx < MAX_P2P_GROUP_SIZE)
	{
		if ((pAd->P2pTable.Client[idx].P2pClientState == P2PSTATE_CONNECT_COMMAND)|| (pAd->P2pTable.Client[idx].P2pClientState == P2PSTATE_DISCOVERY))
		{
			/* Reset Scan Counter to Zero . So Won't do Scan too sooon because group forming is started. */
			pAd->P2pCfg.P2pCounter.Counter100ms = 0;
			if (IS_EXT_LISTEN_ON(pAd))
			{
				pAd->P2pCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
				pAd->P2pCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
			}

			{
				pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
				DBGPRINT(RT_DEBUG_ERROR, ("%s::  Start GroupForm .  \n", __FUNCTION__));
				/* allocate and send out ProbeRsp frame */
				NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /* Get an unused nonpaged memory */
				if (NStatus != NDIS_STATUS_SUCCESS)
					return FALSE;
				pAd->P2pTable.Client[idx].P2pClientState = P2PSTATE_SENT_GO_NEG_REQ;
				pAd->P2pTable.Client[idx].StateCount = 0;
				pAd->P2pTable.Client[idx].bValid = FALSE;


				/* Set as Client temporarily. Later when I get the GO Rsp, will update this based on correct Intent. */
				P2PMakeGoNegoReq(pAd, idx, Addr, pOutBuffer, &FrameLen);
				pP2PCtrl->bP2pReSendTimerRunning = TRUE;
				RTMPSetTimer(&pP2PCtrl->P2pReSendTimer, P2P_TRANSMIT_TIMER);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pAd, pOutBuffer);

				DBGPRINT(RT_DEBUG_ERROR, (" P2P - Make GO Negociation Req FrameLen  = %ld. \n", FrameLen));	
			}	
			/* Once dwell in a listen and start group formation process, should stop scan right away. */
			return TRUE;
		}
	}

	return FALSE;

}

/*	
	==========================================================================
	Description: 
		Check if the addr is in my MAC wish list if any.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN P2PDeviceMatch(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	Addr,
	IN PUCHAR	DeviceName,
	IN ULONG		DeviceNameLen)
{
	UCHAR	i;
	UCHAR	*ptr;
	
	ptr = &pAd->P2pCfg.ConnectingMAC[0];
	return TRUE;
	
	if (NdisEqualMemory(Addr, ZERO_MAC_ADDR, 6))
		return FALSE;

	/* If didn't assign MAC connecting wish list, allow all and return TRUE. */
	if (NdisEqualMemory(pAd->P2pCfg.ConnectingMAC, ZERO_MAC_ADDR, 6))
		return TRUE;

	for (i = 0;i<MAX_P2P_GROUP_SIZE;i++)
	{
		if (NdisEqualMemory(Addr, ptr, 6))
			return TRUE;
		
		ptr += MAC_ADDR_LEN;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("P2p : P2PDevice Not Match from %02x:%02x:%02x:%02x:%02x:%02x.. = %s \n", PRINT_MAC(Addr), decodeP2PState(pAd->P2pCfg.P2PConnectState)));
	return FALSE;

}

/*	
	==========================================================================
	Description: 
		Set WPS related parameters
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pSetWps(
	IN	PRTMP_ADAPTER	pAd,
	IN PRT_P2P_CLIENT_ENTRY pP2pEntry)
{ 
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	POS_COOKIE          pObj = (POS_COOKIE) pAd->OS_Cookie;
	/*BOOLEAN 	Cancelled;*/
	BOOLEAN bChangeInitBW = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: ==> P2P Connect State = %d.     %s.\n", 
				__FUNCTION__, pP2PCtrl->P2PConnectState, decodeP2PState(pP2PCtrl->P2PConnectState)));	

	P2pStopConnectThis(pAd); /* clean Connecting MAC Address. */
	P2pStopScan(pAd);
	pObj->ioctl_if_type = INT_P2P;
	pP2PCtrl->bConfirmByUI = FALSE;
	if ((pP2PCtrl->Rule == P2P_IS_GO) && (P2P_GO_ON(pAd)))
	{
		PWSC_CTRL			pWscControl = &pAd->ApCfg.MBSSID[0].WscControl;

		/* Set Channel. */
		DBGPRINT(RT_DEBUG_ERROR, ("(%d, %d)\n", pAd->LatchRfRegs.Channel != pP2PCtrl->GroupChannel));
		if (pAd->LatchRfRegs.Channel != pP2PCtrl->GroupChannel)
			bChangeInitBW = TRUE;
		else
		{
			pApCliEntry = &pAd->ApCfg.ApCliTab[BSS0];

			if (!INFRA_ON(pAd)&& !P2P_GO_ON(pAd) && (pApCliEntry->Valid == FALSE))
				bChangeInitBW = TRUE;
			else
			{
				if (INFRA_ON(pAd) || P2P_GO_ON(pAd))
				{
					if ((pAd->CommonCfg.BBPCurrentBW == BW_40) && (pAd->LatchRfRegs.Channel != pAd->CommonCfg.CentralChannel))
						bChangeInitBW = TRUE;
				}
			}
		}

		if (bChangeInitBW == TRUE)
		{			
			UINT32	Data = 0, macStatus;
			UINT32 MTxCycle, MRxCycle;
			UCHAR BBPValue = 0;

			/* Disable MAC Tx/Rx */
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
			Data &= (~0x0C);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);

			/* Check MAC Tx/Rx idle */
			for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
			{
				RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
				if (macStatus & 0x3)
					RTMPusecDelay(50);
				else
					break;
			}

			if (!INFRA_ON(pAd))
			{
				pAd->CommonCfg.Channel = pP2PCtrl->GroupOpChannel;
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED
					&& pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
					N_SetCenCh(pAd);
				AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
				AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
			}
			else
			{
				if ((pAd->CommonCfg.Channel != pAd->CommonCfg.CentralChannel) && (pAd->CommonCfg.BBPCurrentBW == BW_40))
				{
					AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
				}
				else
				{
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
				}
			}			
				
			if (pAd->CommonCfg.BBPCurrentBW == BW_40)
			{
				UCHAR BBPValue = 0;
				
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
				BBPValue &= (~0x18);
				BBPValue |= 0x10;
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			}	

			/* Enable MAC Tx/Rx */
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
			Data |= 0x0C;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
		}
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Rule = %s. OpChannel = %d. CentralChannel = %d\n", 
					__FUNCTION__, decodeMyRule(pP2PCtrl->Rule), pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel));
		DBGPRINT(RT_DEBUG_ERROR, ("      SSID[%d] = %s.    BSSID = %02x:%02x:%02x:%02x:%02x:%02x.\n", 
					pP2PCtrl->SSIDLen, pP2PCtrl->SSID, PRINT_MAC(pP2PCtrl->CurrentAddress)));
		pP2PCtrl->P2pCapability[1] |= GRPCAP_OWNER;
		/* P2P GO up. */
		if (pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("P2P_FIXED_MODE && WSC_SCSTATE_CONFIGURED\n"));
			AsicBBPAdjust(pAd);
		}
		else
		{
		Set_P2p_OpMode_Proc(pAd, "1");
		}

		/* GO WPS trigger. */
		if (pP2PCtrl->WscMode == WSC_PIN_MODE)
		{
			if (pP2PCtrl->ConfigMethod == WSC_CONFMET_KEYPAD)
			{
				Set_AP_WscConfMode_Proc(pAd, "5"); /* Registrar / Enrollee  */
				Set_AP_WscMode_Proc(pAd, "1"); /* PIN  */
				/* note 2011-10-12 : only AutoGO need WscPinCode command */
				if ((!(pAd->flg_p2p_OpStatusFlags & P2P_FIXED_MODE)) ||
					(pP2PCtrl->bSigmaEnabled == TRUE))
				{
				Set_AP_WscPinCode_Proc(pAd, pP2PCtrl->PinCode);
				OS_WAIT(700);
				}
				Set_AP_WscGetConf_Proc(pAd, "1");
			}
			else if (pP2PCtrl->ConfigMethod == WSC_CONFMET_DISPLAY)
			{
				Set_AP_WscConfMode_Proc(pAd, "5"); /* Registrar / Enrollee */
			Set_AP_WscMode_Proc(pAd, "1"); /* PIN */
			OS_WAIT(700);
			Set_AP_WscGetConf_Proc(pAd, "1");
		}
		}
		else if (pP2PCtrl->WscMode == WSC_PBC_MODE)
		{
			Set_AP_WscConfMode_Proc(pAd, "5"); /* Registrar / Enrollee */
			Set_AP_WscMode_Proc(pAd, "2"); /* PBC */
			OS_WAIT(700);
			Set_AP_WscGetConf_Proc(pAd, "1");
		}
		RTMPSetTimer(&pAd->P2pCfg.P2pWscTimer, P2P_WSC_TIMER);

	}
	else if ((pP2PCtrl->Rule == P2P_IS_CLIENT) && (P2P_CLI_ON(pAd)))
	{
		/* Set Channel. */
		pAd->CommonCfg.Channel = pP2PCtrl->GroupOpChannel;
		/*N_SetCenCh(pAd); */
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Rule = %s. OpChannel = %d. CentralChannel = %d\n", 
					__FUNCTION__, decodeMyRule(pP2PCtrl->Rule), pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel));
		DBGPRINT(RT_DEBUG_ERROR, (" 	 SSID[%d] = %s.    BSSID = %02x:%02x:%02x:%02x:%02x:%02x.\n", 
					pP2PCtrl->SSIDLen, pP2PCtrl->SSID, PRINT_MAC(pP2PCtrl->Bssid)));

		/* P2P CLIENT up. */
		Set_P2p_OpMode_Proc(pAd, "2");
		/* P2P AP-Client Enable. */
		Set_P2pCli_Enable_Proc(pAd, "1");

		/* CLIENT WPS trigger. */
		if (pP2PCtrl->WscMode == WSC_PIN_MODE)
		{
			if (pP2PCtrl->ConfigMethod == WSC_CONFMET_KEYPAD)
			{
				Set_AP_WscConfMode_Proc(pAd, "1"); /* Enrollee */
				Set_AP_WscMode_Proc(pAd, "1"); /* PIN */
				Set_P2pCli_WscSsid_Proc(pAd, &pP2PCtrl->SSID[0]);
				/* Need Enter PIN Code and trigger WPS. */
				Set_AP_WscPinCode_Proc(pAd, pP2PCtrl->PinCode);
				Set_AP_WscGetConf_Proc(pAd, "1");
			}
			else if (pP2PCtrl->ConfigMethod == WSC_CONFMET_DISPLAY)
			{
			Set_AP_WscConfMode_Proc(pAd, "1"); /* Enrollee */
			Set_AP_WscMode_Proc(pAd, "1"); /* PIN */
			Set_P2pCli_Ssid_Proc(pAd, &pP2PCtrl->SSID[0]);
			Set_AP_WscGetConf_Proc(pAd, "1");
		}
			/*COPY_MAC_ADDR(pAd->ApCfg.ApCliTab[0].CfgApCliBssid, pP2PCtrl->Bssid);*/
		}
		else if (pP2PCtrl->WscMode == WSC_PBC_MODE)
		{
			Set_AP_WscConfMode_Proc(pAd, "1"); /* Enrollee */
			Set_AP_WscMode_Proc(pAd, "2"); /* PBC */
			Set_P2pCli_WscSsid_Proc(pAd, &pP2PCtrl->SSID[0]);
			Set_AP_WscGetConf_Proc(pAd, "1");
		}

		RTMPSetTimer(&pAd->P2pCfg.P2pWscTimer, P2P_WSC_TIMER);
	}
}

/*	
	==========================================================================
	Description: 
		Call this function after a successful Go negociation. will do coreessponding action as  GO or Client.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pGoNegoDone(
	IN PRTMP_ADAPTER pAd,
	IN PRT_P2P_CLIENT_ENTRY pP2pEntry) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;


	if (pP2PCtrl->P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO)
	{
		/* Set MyRule in P2P GroupFormat */
		pP2PCtrl->Rule = P2P_IS_GO;
		pAd->flg_p2p_OpStatusFlags |= P2P_GO_UP;
		P2pSetWps(pAd, pP2pEntry);
	}
	else if (pP2PCtrl->P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("pAd->P2pCfg.Dpid = %s \n", decodeDpid(pP2PCtrl->Dpid)));
		/* Set MyRule in P2P GroupFormat */
		pP2PCtrl->Rule = P2P_IS_CLIENT;
		pAd->flg_p2p_OpStatusFlags = P2P_CLI_UP;

		/* If WPS not triggered, don't so WPS AP now. Wait until P2PPRofile called */
		if (pP2PCtrl->Dpid != DEV_PASS_ID_NOSPEC)
		{
			/* Update Per client state */
			pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
			/* Update Global State. Prepare to scan for GO beacon. So as to connect using WPS. */
			pP2PCtrl->P2PConnectState = P2P_DO_GO_SCAN_BEGIN;
			pP2PCtrl->P2pCounter.Counter100ms = 0;
			/* When Counter100ms reaches NextScanRound in P2P_DO_GO_NEG_DONE_CLIENT State, */
			/* Will start a site survey again. Because P2P GO start beacon after Go Negociation. */
			pP2PCtrl->P2pCounter.NextScanRound = 20 + pP2pEntry->ConfigTimeOut * 10;
			/*pAd->StaCfg.WscControl.WscAPChannel = pP2PCtrl->GroupOpChannel; */

			DBGPRINT(RT_DEBUG_ERROR, ("P2pGoNegoDone. NextScanRound= %ld\n", pP2PCtrl->P2pCounter.NextScanRound));
			DBGPRINT(RT_DEBUG_ERROR, ("2 WscMode = %lx     \n", pP2PCtrl->WscMode));
			/* Start WPS. */
			P2pSetWps(pAd, pP2pEntry);

		}
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("P2pGoNegoDone-  invalid p2pstate = %s\n", decodeP2PState(pP2PCtrl->P2PConnectState)));
		

}

/*	
	==========================================================================
	Description: 
		Call this function after a successful WPS provisioning.  do coreessponding action as  GO or Client.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pWpsDone(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		pAddr)
{

}

VOID P2pLinkDown(
	IN PRTMP_ADAPTER pAd,
	IN INT32 type)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT if_type = pObj->ioctl_if_type;
	UCHAR           BBPValue = 0;


	DBGPRINT(RT_DEBUG_ERROR, ("%s:: ==> P2P Connect State = %d. 	%s.\n", __FUNCTION__, pP2PCtrl->P2PConnectState, decodeP2PState(pP2PCtrl->P2PConnectState)));	

	P2pStopConnectThis(pAd); /* clean Connecting MAC Address. */

 	pP2PCtrl->P2PConnectState = P2P_CONNECT_IDLE;
/*
	pP2PCtrl->CtrlCurrentState = P2P_CTRL_IDLE;
	pP2PCtrl->DiscCurrentState = P2P_DISC_LISTEN;
	pP2PCtrl->GoFormCurrentState = P2P_GO_FORM_IDLE;
*/
 	P2pStopScan(pAd);
	P2pGroupTabInit(pAd);
	pP2PCtrl->GoFormCurrentState = P2P_GO_FORM_IDLE;

	/* Restore P2P WSC Mode / Config Method */
	pP2PCtrl->WscMode = WSC_PIN_MODE; /* PIN */
	pP2PCtrl->ConfigMethod = 0x188;
	pP2PCtrl->Dpid = DEV_PASS_ID_NOSPEC;
	/*MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_CANL_CMD_EVT, 0, NULL, 0);*/

	pObj->ioctl_if_type = INT_P2P;

	if (P2P_CLI_ON(pAd))
		Set_P2pCli_Enable_Proc(pAd, "0");
	if (P2P_GO_ON(pAd))
	{
		PWSC_CTRL			pWscControl = &pAd->ApCfg.MBSSID[0].WscControl;
		pWscControl->WscState = WSC_STATE_OFF;
	}

	Set_P2p_OpMode_Proc(pAd, "0");
	pObj->ioctl_if_type = if_type;
	pAd->ApCfg.ApCliTab[0].WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
	pAd->ApCfg.MBSSID[0].WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;

	P2pGetRandomSSID(pAd, pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, &(pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen));
	pAd->P2pCfg.SSIDLen = pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen;
	NdisMoveMemory(pAd->P2pCfg.SSID, pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, pAd->P2pCfg.SSIDLen);
	
	/* Set Channel. */
	if ((INFRA_ON(pAd)) && (pAd->StaActive.SupportedHtPhy.ChannelWidth == BW_40))
	{
	}
	else
	{
		BOOLEAN bDisableMAC = FALSE;
		UINT32	Data = 0, macStatus;
		UINT32 MTxCycle;

		if (pAd->CommonCfg.BBPCurrentBW = BW_40)
			bDisableMAC = TRUE;

		if (bDisableMAC)
		{
			/* Disable MAC Tx/Rx */
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
			Data &= (~0x0C);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);

			/* Check MAC Tx/Rx idle */
			for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
			{
				RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
				if (macStatus & 0x3)
					RTMPusecDelay(50);
				else
					break;
			}
		}

		OS_WAIT(500);
		pAd->CommonCfg.BBPCurrentBW = BW_20;
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
		BBPValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

		if (bDisableMAC)
		{
			/* Enable MAC Tx/Rx */
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
			Data |= 0x0C;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
		}

		if (!INFRA_ON(pAd))
		{
			pAd->CommonCfg.Channel = pP2PCtrl->ListenChannel;
			pAd->MlmeAux.AutoReconnectSsidLen = pAd->CommonCfg.SsidLen;
			NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
		}
		
		if (pAd->CommonCfg.BBPCurrentBW == BW_20)
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
	}

	if (IS_PERSISTENT_ON(pAd) && pP2PCtrl->bP2pCliReConnect)
	{
		COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pAd->P2pTable.PerstEntry[0].Addr);
		DBGPRINT(RT_DEBUG_ERROR, ("P2P CLIENT Re-Connect to %02x:%02x:%02x:%02x:%02x:%02x\n", 
					PRINT_MAC(pAd->P2pCfg.ConnectingMAC)));
	}
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
	if (type == P2P_CONNECT_FAIL)
		type = RT_P2P_CONNECT_FAIL;
	else if (type == P2P_DISCONNECTED)
		type = RT_P2P_DISCONNECTED;
	P2pSendWirelessEvent(pAd, type, NULL, NULL);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
}

/*	
	==========================================================================
	Description: 
		Start autonomous GO function/
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pStartAutoGo(
	IN PRTMP_ADAPTER pAd) 
{

}

VOID P2pEnable(
	IN PRTMP_ADAPTER pAd)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	RTMPMoveMemory(pP2PCtrl->SSID, WILDP2PSSID, WILDP2PSSIDLEN);

	pP2PCtrl->P2pCapability[0] =	0;
	pP2PCtrl->P2pCapability[1] =	0;

	if (IS_PERSISTENT_ON(pAd))
	{
		pAd->P2pCfg.P2pCapability[1] |= GRPCAP_PERSISTENT;
		pAd->P2pCfg.P2pCapability[1] |= GRPCAP_PERSISTENT_RECONNECT;
		pAd->P2pCfg.P2pCapability[0] |= DEVCAP_INVITE;
		pAd->P2pCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
		pAd->P2pCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
	}
	else
	{
		/* When Persistent is disabled, I won't support extended listening, so set to Zero. */
		pAd->P2pCfg.ExtListenInterval = 0;
		pAd->P2pCfg.ExtListenPeriod = 0;
	}

	/* P2P Device Capability */
	pP2PCtrl->P2pCapability[0] |= DEVCAP_INVITE;

	if (pP2PCtrl->P2p_OpMode == P2P_CONCURRENT)
	{
		pP2PCtrl->P2pCapability[1] |= GRPCAP_CROSS_CONNECT;
		pP2PCtrl->P2pCapability[0] |= DEVCAP_CLIENT_CONCURRENT;
	}


	if (IS_INTRA_BSS_ON(pAd))
	{
		pP2PCtrl->P2pCapability[1] |= GRPCAP_INTRA_BSS;
	}

	{
		pP2PCtrl->ConfigMethod = 0x188;
		pAd->StaCfg.WscControl.RegData.SelfInfo.ConfigMethods = 0x188;
		pAd->StaCfg.WscControl.RegData.SelfInfo.DevPwdId = 0x188;
	}

	pP2PCtrl->ConfigTimeout[0] = 4;
	pP2PCtrl->ConfigTimeout[1] = 30;

	/* Construct AP RSN_IE.  */
	RTMPMakeRSNIE(pAd, Ndis802_11AuthModeWPA2PSK, Ndis802_11Encryption3Enabled, 0 /*apidx*/);

	P2pStopNoA(pAd, NULL);
	pAd->P2pCfg.CTWindows = 0;
	P2pScanChannelDefault(pAd);
	pP2PCtrl->P2PConnectState = P2P_CONNECT_IDLE;
	P2pGotoIdle(pAd);

	/*AsicSetBssid(pAd, pP2PCtrl->CurrentAddress);  */
	{
		ULONG	Addr4;
		UINT32	regValue;
		PUCHAR pP2PBssid = &pAd->CurrentAddress[0];

		Addr4 = (ULONG)(pP2PBssid[0])		 | 
				(ULONG)(pP2PBssid[1] << 8)	| 
				(ULONG)(pP2PBssid[2] << 16) |
				(ULONG)(pP2PBssid[3] << 24);
		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

		Addr4 = 0;

		/* always one BSSID in STA mode */
		Addr4 = (ULONG)(pP2PBssid[4]) | (ULONG)(pP2PBssid[5] << 8);

		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);

		RTMP_IO_READ32(pAd, MAC_BSSID_DW1, &regValue);
		regValue &= 0x0000FFFF;

#ifndef NEW_MBSSID_MODE
		if ((pAd->CurrentAddress[5] % 2 != 0)
#ifdef P2P_ODD_MAC_ADJUST
				&& FALSE
#endif /* P2P_ODD_MAC_ADJUST */
			)
			DBGPRINT(RT_DEBUG_ERROR, ("The 2-BSSID mode is enabled, the BSSID byte5 MUST be the multiple of 2\n"));
#endif
		
		regValue |= (1 << 16);
		/*	set as 0/1 bit-21 of MAC_BSSID_DW1(offset: 0x1014) 
			to disable/enable the new MAC address assignment.  */
		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
		regValue |= (1 << 21);
		RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, regValue);
	}

	DBGPRINT(RT_DEBUG_ERROR, ("!4 OpStatusFlags = %lx. CentralChannel = %d. Channel	= %d. !!! \n", pAd->CommonCfg.OpStatusFlags,pAd->CommonCfg.CentralChannel,pAd->CommonCfg.Channel));

}

/*	
	==========================================================================
	Description: 
		Modify P2P's listen channel when in Concurrent Operation to get better performance. 
		Can be called from LinkUp and LinkDown;
		
	Parameters: 
		 
	Note:
		 
	==========================================================================
 */
VOID P2pScanChannelDefault(
	IN PRTMP_ADAPTER pAd)
{
	/* Default listen channel is based on spec, using Social channels 1, 6, 11. */
	pAd->P2pCfg.P2pProprietary.ListenChanel[0] = 1;
	pAd->P2pCfg.P2pProprietary.ListenChanel[1] = 6;
	pAd->P2pCfg.P2pProprietary.ListenChanel[2] = 11;
	pAd->P2pCfg.P2pProprietary.ListenChanelIndex = 0;
	pAd->P2pCfg.P2pProprietary.ListenChanelCount = 3;

	DBGPRINT(RT_DEBUG_ERROR, ("%s <=== count = %d, Channels are %d, %d,%d separately   \n", 
		__FUNCTION__, pAd->P2pCfg.P2pProprietary.ListenChanelCount, pAd->P2pCfg.P2pProprietary.ListenChanel[0], pAd->P2pCfg.P2pProprietary.ListenChanel[1], pAd->P2pCfg.P2pProprietary.ListenChanel[2]));
}

/*
	========================================================================
	
	Routine Description:
		If.

	Arguments:
		    - NIC Adapter pointer
		
	Return Value:
		FALSE - None of channel in ChannelList Match any channel in pAd->ChannelList[] array

	IRQL = DISPATCH_LEVEL
	
	Note:
	========================================================================

*/
BOOLEAN P2pCheckChannelList(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pChannelList)
{
	UCHAR		i, k;
	UCHAR		NumOfDismatch = 0;
	
	/* Check if new (control) channel is in our channellist which we currently agree to operate in. */
	for (k = 0;k < MAX_NUM_OF_CHANNELS;k++)
	{
		if (*(pChannelList + k) == 0)
			break;
		
		for (i = 0;i < pAd->ChannelListNum;i++)
		{
			if (pAd->ChannelList[i].Channel == *(pChannelList + k))
			{
				break;
			}
		}
		if ( i == pAd->ChannelListNum)
			NumOfDismatch++;			
	}

	if ((NumOfDismatch == k) && (k != 0))
	{
		DBGPRINT(RT_DEBUG_TRACE, (" P2P - no common channel = %d...\n", *pChannelList));
		return FALSE;
	}

	return TRUE;

}


/*	
	==========================================================================
	Description: 
		The routine check whether we need to check this frame's transmission result. Success ? or Retry fail?.
		If found retry fail, we may need to resend this frame again. 
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pAckRequiredCheck(
	IN PRTMP_ADAPTER pAd,
	IN PP2P_PUBLIC_FRAME	pFrame,
	OUT 		UCHAR	*TempPid)
{
		UCHAR				EAPHEAD[8] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
		PUCHAR		pDest;
		PFRAME_P2P_ACTION		pP2pActFrame;
		PUCHAR pP2pOUI = (PUCHAR) pFrame;

		pP2pOUI += (sizeof(HEADER_802_11) + 2);
	
		pP2pActFrame = (PFRAME_P2P_ACTION)pFrame;
		/* check Go Neg Confirm Frame. */
		if ((pFrame->Subtype == GO_NEGOCIATION_CONFIRM) && 
			(NdisEqualMemory(pP2pOUI, P2POUIBYTE, 4)) && 
			(pFrame->Action == ACTION_WIFI_DIRECT) && 
			(pFrame->Category == CATEGORY_PUBLIC))
		{
			DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a Confirm Frame. Pid = RequireACK !!!\n"));
			*TempPid = 0x7; /* PID_REQUIRE_ACK */
			return;
		}
	
		/* check Intivation Response. */
		if ((pFrame->Subtype == P2P_INVITE_RSP) && 
			(NdisEqualMemory(pP2pOUI, P2POUIBYTE, 4)) && 
			(pFrame->Action == ACTION_WIFI_DIRECT) && 
			(pFrame->Category == CATEGORY_PUBLIC))
		{
			DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a Invite Rsp Frame. Pid = RequireACK !!!\n"));
			*TempPid = 0x7; /* PID_REQUIRE_ACK */
			return;
		}
	
		if ((pP2pActFrame->Category == 0x7F/*MT2_ACT_VENDOR*/) 
			&& (pP2pActFrame->OUISubType == P2PACT_GO_DISCOVER_REQ) 
			&& (pP2pActFrame->OUIType == 0x9))	
		{
			DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a GO Discoverbility. Pid = RequireACK !!!\n"));
			*TempPid = 0x7; /* PID_REQUIRE_ACK */
			return;
		}
		/* As Go, need to check EAP fail's ACK */
		if ((IS_P2P_REGISTRA(pAd)) && (pFrame->p80211Header.FC.Type == BTYPE_DATA))
		{
			pDest = &pFrame->Category;
			if (NdisEqualMemory(EAPHEAD, pDest, 8))
			{
				pDest += 8;
				if ((*pDest == EAPOL_VER) && (*(pDest+1) == EAPPacket) && (*(pDest + 3) == EAP_CODE_FAIL))
				{
					DBGPRINT(RT_DEBUG_TRACE,("P2pAckRequiredCheck --> This is a EAP FailFrame.	Pid = RequireACK!!!\n"));
					*TempPid = 0x7; /* PID_REQUIRE_ACK */
				}
			}
	
		}

}

BOOLEAN IsP2pFirstMacSmaller(
	IN PUCHAR		Firststaddr,
	IN PUCHAR		SecondAddr)
{
	UCHAR		i;
	for (i=0 ; i< 6;i++)
	{
		if (*(Firststaddr+i) > *(SecondAddr+i))
			return FALSE;
	}

	return TRUE;
}

/* 
	==========================================================================
	Description:
		Initial Listen interval for Listen Timer
	Return:
		VOID
	NOTE:
	==========================================================================
*/
VOID	P2PInitListenTimer(
	IN PRTMP_ADAPTER	pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_TRACE, ("<---- %s\n", __FUNCTION__));
	pP2PCtrl->P2pCounter.bListen = FALSE;
	pP2PCtrl->P2pCounter.ListenInterval = 0;
}

/* 
	==========================================================================
	Description:
		Set Listen interval for Listen Timer
	Return:
		VOID
	NOTE:
	==========================================================================
*/
VOID	P2PSetListenTimer(
	IN PRTMP_ADAPTER	pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	if ((pP2PCtrl->P2pCounter.bStartScan == TRUE) && (pP2PCtrl->P2pCounter.bListen == FALSE))
	{
		/* update Discovery State Machine state. */
		if (pP2PCtrl->P2pCounter.bStartScan)
		MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_LISTEN_CMD_EVT, 0, NULL, 0);
	}
}

/* 
	==========================================================================
	Description:
		Listen State Timer Handler 
	Return:
		VOID
	NOTE:
	==========================================================================
*/
VOID	P2PListenTimerExec(
	IN PRTMP_ADAPTER	pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	pP2PCtrl->P2pCounter.bListen = FALSE;

	if (pP2PCtrl->P2pCounter.bStartScan == TRUE)
	{
	/* Listen interval has expired, go to Search state. */
	MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_SEARCH_CMD_EVT, 0, NULL, 0);
}
	else
	{
		/* Listen interval has expired, go to IDLE state. */
		MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_CANL_CMD_EVT, 0, NULL, 0);

		NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
		pAd->MlmeAux.AutoReconnectSsidLen = (UCHAR)pAd->CommonCfg.SsidLen;
	}
}

/* 
	==========================================================================
	Description:
		Initial Next Scan Round for Scan Timer
	Return:
		VOID
	NOTE:
	==========================================================================
*/
VOID	P2PInitNextScanTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_TRACE, ("<---- %s\n", __FUNCTION__));
	pP2PCtrl->P2pCounter.bNextScan = FALSE;
	pP2PCtrl->P2pCounter.NextScanRound = 0xffffffff;
}

/* 
	==========================================================================
	Description:
		Set Next Scan Round for Scan Timer
	Return:
		VOID
	NOTE:
	==========================================================================
*/
VOID	P2PSetNextScanTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	if ((pP2PCtrl->P2pCounter.bStartScan == TRUE) && (pP2PCtrl->P2pCounter.bNextScan == FALSE))
	{
		pP2PCtrl->P2pCounter.bNextScan = TRUE;
		if (value)
			pP2PCtrl->P2pCounter.NextScanRound = value;
		else
			pP2PCtrl->P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + P2P_RANDOM_BIAS;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("%s: NextScanRound - %ld\n", __FUNCTION__, pP2PCtrl->P2pCounter.NextScanRound));
}

/* 
	==========================================================================
	Description:
		Next Scan Round Timer Handler 
	Return:
		VOID
	NOTE:
	==========================================================================
*/
VOID	P2PNextScanTimerExec(
	IN PRTMP_ADAPTER pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	pP2PCtrl->P2pCounter.bNextScan = FALSE;
	MlmeEnqueue(pAd, P2P_CTRL_STATE_MACHINE, P2P_CTRL_DISC_EVT, 0, NULL, 0);
}

VOID	P2PInitDevDiscTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	pP2PCtrl->P2pCounter.bStartScan = FALSE;
	pP2PCtrl->P2pCounter.CounterAftrScanButton = 0xffffffff;
}

VOID	P2PSetDevDiscTimer(
	IN PRTMP_ADAPTER pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	if (pP2PCtrl->P2pCounter.bStartScan == FALSE)
	{
		pP2PCtrl->P2pCounter.bStartScan = TRUE;
		if (P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
			pP2PCtrl->P2pCounter.CounterAftrScanButton = 200;
		pP2PCtrl->P2pCounter.CounterAftrScanButton = pP2PCtrl->DevDiscPeriod;
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: CounterAftrScanButton = %lu.\n", __FUNCTION__, pP2PCtrl->P2pCounter.CounterAftrScanButton));
	}
}

VOID	P2PDevDiscTimerExec(
	IN PRTMP_ADAPTER pAd,
	UINT32	value)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	pP2PCtrl->P2pCounter.bStartScan = FALSE;
	MlmeEnqueue(pAd, P2P_CTRL_STATE_MACHINE, P2P_CTRL_DISC_DONE_EVT, 0, NULL, 1);
}

VOID P2P_SetWscRule(
	IN PRTMP_ADAPTER pAd,
	UCHAR	index,
	PUSHORT PeerWscMethod)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAd->P2pTable.Client[index];
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Peer Dpid = %s.    My Dpid = %s.\n", __FUNCTION__, decodeDpid(pP2pEntry->Dpid), decodeDpid(pP2PCtrl->Dpid)));
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Peer Config Method = %s.    My Config Method = %s.\n", __FUNCTION__, decodeConfigMethod(pP2pEntry->ConfigMethod), decodeConfigMethod(pP2PCtrl->ConfigMethod)));

	if (pP2PCtrl->Dpid == DEV_PASS_ID_NOSPEC)
	{
		if ((pP2pEntry->ConfigMethod == WSC_CONFMET_KEYPAD) || (pP2pEntry->Dpid == DEV_PASS_ID_USER))
		{
			pP2PCtrl->WscMode = WSC_PIN_MODE;
			pP2PCtrl->ConfigMethod = WSC_CONFMET_DISPLAY;
			pP2PCtrl->Dpid = DEV_PASS_ID_REG;
			*PeerWscMethod = WSC_CONFMET_KEYPAD;
		}
		else if ((pP2pEntry->ConfigMethod == WSC_CONFMET_DISPLAY) || (pP2pEntry->Dpid == DEV_PASS_ID_REG))
		{
			pP2PCtrl->WscMode = WSC_PIN_MODE;
			pP2PCtrl->ConfigMethod = WSC_CONFMET_KEYPAD;
			pP2PCtrl->Dpid = DEV_PASS_ID_USER;
			*PeerWscMethod = WSC_CONFMET_DISPLAY;
		}
		else if ((pP2pEntry->ConfigMethod == WSC_CONFMET_PBC) || (pP2pEntry->Dpid == DEV_PASS_ID_PBC))
		{
			pP2PCtrl->WscMode = WSC_PBC_MODE;
			pP2PCtrl->ConfigMethod = WSC_CONFMET_PBC;
			pP2PCtrl->Dpid = DEV_PASS_ID_PBC;
			*PeerWscMethod = WSC_CONFMET_PBC;
		}
		else
		{
			if ((pP2pEntry->Rule == P2P_IS_CLIENT) && (pP2PCtrl->Rule == P2P_IS_GO))
			{
				/* I'm Registrar and Enter PIN. */
				pP2PCtrl->WscMode = WSC_PIN_MODE;
				pP2PCtrl->ConfigMethod = WSC_CONFMET_KEYPAD;
				pP2PCtrl->Dpid = DEV_PASS_ID_USER;
			}
			else
			{
				/* I'm Enrollee and Use Default Config Method. */
				if (pP2PCtrl->DefaultConfigMethod == P2P_REG_CM_DISPLAY)
				{
					pP2PCtrl->WscMode = WSC_PIN_MODE;
					pP2PCtrl->ConfigMethod = WSC_CONFMET_DISPLAY;
					pP2PCtrl->Dpid = DEV_PASS_ID_REG;
					*PeerWscMethod = WSC_CONFMET_KEYPAD;
				}
				else if (pP2PCtrl->DefaultConfigMethod == P2P_REG_CM_KEYPAD)
				{
					pP2PCtrl->WscMode = WSC_PIN_MODE;
					pP2PCtrl->ConfigMethod = WSC_CONFMET_KEYPAD;
					pP2PCtrl->Dpid = DEV_PASS_ID_USER;
					*PeerWscMethod = WSC_CONFMET_DISPLAY;
				}
				else if (pP2PCtrl->DefaultConfigMethod == P2P_REG_CM_PBC)
				{
					pP2PCtrl->WscMode = WSC_PBC_MODE;
					pP2PCtrl->ConfigMethod = WSC_CONFMET_PBC;
					pP2PCtrl->Dpid = DEV_PASS_ID_PBC;
					*PeerWscMethod = WSC_CONFMET_PBC;
				}
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("My P2P Wsc Configuration:\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("WscMode = %d.    WscConfigMethod = %s.    WscDpid = %s.\n", pP2PCtrl->WscMode, decodeConfigMethod(pP2PCtrl->ConfigMethod), decodeDpid(pP2PCtrl->Dpid)));
	if (pP2PCtrl->ConfigMethod == WSC_CONFMET_DISPLAY)
	{
		/*
			Although driver uses different structure for P2P GO and P2P CLI but PIN shall be the same.
		*/
		pAd->ApCfg.MBSSID[0].WscControl.WscEnrolleePinCode = pAd->ApCfg.ApCliTab[0].WscControl.WscEnrolleePinCode;
		pAd->ApCfg.MBSSID[0].WscControl.WscEnrolleePinCodeLen = pAd->ApCfg.ApCliTab[0].WscControl.WscEnrolleePinCodeLen;
		
		/* Woody 1: Show PIN Code */
		DBGPRINT(RT_DEBUG_TRACE, ("    *************************************************\n"));
		DBGPRINT(RT_DEBUG_TRACE, ("    *                                               *\n"));
		DBGPRINT(RT_DEBUG_TRACE, ("    *       PIN Code = %08u                     *\n", pAd->ApCfg.ApCliTab[0].WscControl.WscEnrolleePinCode));
		DBGPRINT(RT_DEBUG_TRACE, ("    *                                               *\n"));
		DBGPRINT(RT_DEBUG_TRACE, ("    *************************************************\n"));
		{
/*
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_SHOWPIN, -1, pP2pEntry->addr,
						NULL, 0, 0);
*/
	}
	}
	else if (pP2PCtrl->ConfigMethod == WSC_CONFMET_KEYPAD)
	{
		/* Woody 2: Enter PIN Code */
		{
/*
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_PIN, -1, pP2pEntry->addr,
						NULL, 0, 0);
*/
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Peer P2P Wsc Configuration:\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("WscMode = %d.    WscConfigMethod = %s.    WscDpid = %s.\n", pP2pEntry->WscMode, decodeConfigMethod(pP2pEntry->ConfigMethod), decodeDpid(pP2pEntry->Dpid)));	

}

/* 
	==========================================================================
	Description:
		decode Dpid 
	Return:
		State string
	NOTE:
	==========================================================================
*/
 PSTRING decodeDpid (USHORT dpid)
{
	PSTRING retval = "                                 ";
	switch (dpid) 
	{
		case DEV_PASS_ID_PIN:				retval = "DEV_PASS_ID_PIN    ";					break;
		case DEV_PASS_ID_USER:				retval = "DEV_PASS_ID_USER_SPECIFIED";			break;
		case DEV_PASS_ID_REG:				retval = "DEV_PASS_ID_REGISTRA_SPECIFIED";		break;
		case DEV_PASS_ID_PBC:				retval = "DEV_PASS_ID_PBC";					break;
		default:								retval = "***UNKNOWN dpid***";
	}
	return(retval);

}
/* 
	==========================================================================
	Description:
		decode ConfigMethod
	Return:
		State string
	NOTE:
	==========================================================================
*/
PSTRING ddecodeConfigMethod (USHORT ConfigMethos)
{
	PSTRING retval = "                                 ";
	ULONG		mm[4], i;
	mm[0] = 0;
	mm[1] = 0;
	mm[2] = 0;
	mm[3] = 0;
	if ((ConfigMethos & WSC_CONFMET_LABEL) != 0)
		mm[0] = 1;
	if ((ConfigMethos & WSC_CONFMET_DISPLAY) != 0)
		mm[1] = 1;
	if ((ConfigMethos & WSC_CONFMET_KEYPAD) != 0)
		mm[2] = 1;
	if ((ConfigMethos & WSC_CONFMET_PBC) != 0)
		mm[3] = 1;
	for ( i = 0;i < 4;i++)
	{
	}
	return (retval);
}

PSTRING decodeMyRule (USHORT Rule)
{
	PSTRING retval = "                                                  ";
	switch (Rule) 
	{
		case P2P_IS_CLIENT:						retval = "I am P2P Client";				break;
		case P2P_IS_GO:							retval = "I am P2P GO";					break;
		case P2P_IS_DEVICE:						retval = "I am P2P Device";				break;
		case P2P_IS_CLIENT_IN_GROUP:				retval = "I am P2P Client in Group";		break;
		default:									retval = "***  Unknown Rule ***";
	}
	return(retval);
}

PSTRING decodeConfigMethod (USHORT ConfigMethos)
{
	PSTRING retval = "                                                  ";
	switch (ConfigMethos) 
	{
		case WSC_CONFMET_LABEL:				retval = "LABEL  LABEL ";		break;
		case WSC_CONFMET_DISPLAY:				retval = "DISPLAY  DISPLAY";	break;
		case WSC_CONFMET_KEYPAD:				retval = "KEYPAD  KEYPAD";		break;
		case WSC_CONFMET_PBC:					retval = "PBC   PBC";			break;
		case WSC_CONFMET_DISPLAY | WSC_CONFMET_KEYPAD:		retval = "DISPLAY  KEYPAD";		break;
		case WSC_CONFMET_DISPLAY | WSC_CONFMET_PBC:		retval = "DISPLAY  PBC";		break;
		case WSC_CONFMET_KEYPAD | WSC_CONFMET_PBC:		retval = "KEYPAD  PBC";			break;
		case WSC_CONFMET_DISPLAY | WSC_CONFMET_KEYPAD | WSC_CONFMET_PBC:	retval = "DISPLAY  KEYPAD  PBC";	break;
		default:									retval = "***Multiple ConfigMethod***";
	}
	return(retval);
}


/* 
	==========================================================================
	Description:
		decode P2P state
	Return:
		State string
	NOTE:
	==========================================================================
*/
 PSTRING decodeP2PState (UCHAR P2pState)
{
	PSTRING retval = "                                                                       ";
	switch (P2pState) {
		case P2P_CONNECT_IDLE:					retval = "P2P_CONNECT_IDLE";				break;
		case P2P_ANY_IN_FORMATION_AS_CLIENT:	retval = "GroupForming as Client";			break;
		case P2P_ANY_IN_FORMATION_AS_GO:		retval = "P2P_ANY_IN_FORMATION_AS_GO";	break;
		case P2P_DO_GO_NEG_DONE_CLIENT:		retval = "P2P_DO_GO_NEG_DONE_CLIENT";		break;
		case P2P_DO_GO_SCAN_BEGIN:				retval = "P2P_DO_GO_SCAN_BEGIN";			break;
		case P2P_DO_GO_SCAN_DONE:				retval = "P2P_DO_GO_SCAN_DONE";			break;
		case P2P_WPS_REGISTRA:					retval = "P2P_WPS_REGISTRA";				break;
		case P2P_DO_WPS_ENROLLEE:				retval = "P2P_DO_WPS_ENROLLEE";			break;
		default:									retval = "***UNKNOWN state***";
	}
	return(retval);
}

/* 
	==========================================================================
	Description:
		decode P2P client state
	Return:
		State string
	NOTE:
	==========================================================================
*/
 PSTRING decodeP2PClientState (P2P_CLIENT_STATE P2pClientState)
{
    PSTRING retval = "                                                                        ";
    switch (P2pClientState) {
		case P2PSTATE_NONE:							retval = "P2PSTATE_NONE";							break;
		case P2PSTATE_DISCOVERY:					retval = "P2PSTATE_DISCOVERY";					break;
		case P2PSTATE_DISCOVERY_CLIENT:				retval = "P2PSTATE_DISCOVERY_CLIENT";				break;
		case P2PSTATE_DISCOVERY_GO:				retval = "P2PSTATE_DISCOVERY_GO";					break;
		case P2PSTATE_CLIENT_DISCO_COMMAND:		retval = "P2PSTATE_CLIENT_DISCO_COMMAND";		break;
		case P2PSTATE_GO_DISCO_COMMAND:	             retval = "P2PSTATE_GO_DISCO_COMMAND";			break;
		case P2PSTATE_PROVISION_COMMAND:	             retval = "P2PSTATE_PROVISION_COMMAND";			break;
		case P2PSTATE_SERVICE_DISCO_COMMAND:		retval = "P2PSTATE_SERVICE_DISCO_COMMAND";		break;
		case P2PSTATE_INVITE_COMMAND:				retval = "P2PSTATE_INVITE_COMMAND";				break;
		case P2PSTATE_CONNECT_COMMAND:			retval = "P2PSTATE_CONNECT_COMMAND";			break;
		case P2PSTATE_SENT_INVITE_REQ:				retval = "P2PSTATE_SENT_INVITE_REQ";				break;
		case P2PSTATE_SENT_GO_NEG_REQ:				retval = "P2PSTATE_SENT_GO_NEG_REQ";				break;
		case P2PSTATE_WAIT_GO_COMFIRM:				retval = "P2PSTATE_WAIT_GO_COMFIRM";				break;
		case P2PSTATE_WAIT_GO_COMFIRM_ACK:		retval = "P2PSTATE_WAIT_GO_COMFIRM_ACK";			break;
		case P2PSTATE_GO_COMFIRM_ACK_SUCCESS:		retval = "P2PSTATE_GO_COMFIRM_ACK_SUCCESS";		break;
		case P2PSTATE_WAIT_GO_DISCO_ACK:			retval = "P2PSTATE_WAIT_GO_DISCO_ACK";			break;
		case P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS:	retval = "P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS";	break;
		case P2PSTATE_GO_WPS:						retval = "P2PSTATE_GO_WPS";						break;
		case P2PSTATE_GO_AUTH:						retval = "P2PSTATE_GO_AUTH";						break;
		case P2PSTATE_GO_ASSOC:					retval = "P2PSTATE_GO_ASSOC";						break;
		case P2PSTATE_CLIENT_WPS:					retval = "P2PSTATE_CLIENT_WPS";					break;
		case P2PSTATE_CLIENT_WPS_DONE:				retval = "P2PSTATE_CLIENT_WPS_DONE";				break;
		case P2PSTATE_CLIENT_AUTH:					retval = "P2PSTATE_CLIENT_AUTH";					break;
		case P2PSTATE_CLIENT_ASSOC:					retval = "P2PSTATE_CLIENT_ASSOC";					break;
		case P2PSTATE_CLIENT_OPERATING:				retval = "P2PSTATE_CLIENT_OPERATING";				break;
		case P2PSTATE_CLIENT_ABSENCE:				retval = "P2PSTATE_CLIENT_ABSENCE";				break;
		case P2PSTATE_CLIENT_SCAN:					retval = "P2PSTATE_CLIENT_SCAN";					break;
		case P2PSTATE_CLIENT_FIND:					retval = "P2PSTATE_CLIENT_FIND";					break;
		case P2PSTATE_GO_OPERATING:					retval = "P2PSTATE_GO_OPERATING";					break;
		case P2PSTATE_GO_ABSENCE:					retval = "P2PSTATE_GO_ABSENCE";					break;
		case P2PSTATE_GO_SCAN:						retval = "P2PSTATE_GO_SCAN";						break;
		case P2PSTATE_GO_FIND:						retval = "P2PSTATE_GO_FIND";						break;
		case P2PSTATE_NONP2P_WPS:					retval = "P2PSTATE_NONP2P_WPS";					break;
		case P2PSTATE_NONP2P_PSK:					retval = "P2PSTATE_NONP2P_PSK";					break;
		default:										retval = "***UNKNOWN state***";
    }
    return(retval);

}

PSTRING decodeCtrlState (UCHAR State)
{
	PSTRING retval = "                                                  ";
	switch (State) 
	{
		case P2P_CTRL_IDLE:							retval = "P2P_CTRL_IDLE";					break;
		case P2P_CTRL_DISCOVERY:					retval = "P2P_CTRL_DISCOVERY";				break;
		case P2P_CTRL_GROUP_FORMATION:				retval = "P2P_CTRL_GROUP_FORMATION";		break;
		case P2P_CTRL_DONE:							retval = "P2P_CTRL_DONE";					break;
		default:										retval = "***Unknown CTRL State***";
	}
	return(retval);
}

PSTRING decodeDiscoveryState (UCHAR State)
{
	PSTRING retval = "                                                  ";
	switch (State) 
	{
		case P2P_DISC_IDLE:							retval = "P2P_DISC_IDLE";					break;
		case P2P_DISC_SCAN:							retval = "P2P_DISC_SCAN";					break;
		case P2P_DISC_LISTEN:						retval = "P2P_DISC_LISTEN";				break;
		case P2P_DISC_SEARCH:						retval = "P2P_DISC_SEARCH";				break;
		default:										retval = "***Unknown Device Discovery State***";
	}
	return(retval);
}

PSTRING decodeGroupFormationState (UCHAR State)
{
	PSTRING retval = "                                                  ";
	switch (State) 
	{
		case P2P_GO_FORM_IDLE:						retval = "P2P_GO_FORM_IDLE";				break;
		case P2P_WAIT_GO_FORM_RSP:					retval = "P2P_WAIT_GO_FORM_RSP";			break;
		case P2P_WAIT_GO_FORM_CONF:				retval = "P2P_WAIT_GO_FORM_CONF";		break;
		case P2P_GO_FORM_DONE:						retval = "P2P_GO_FORM_DONE";				break;
		case P2P_GO_FORM_PROV:						retval = "P2P_GO_FORM_PROV";				break;
		case P2P_GO_FORM_INVITE:					retval = "P2P_GO_FORM_INVITE";				break;
		case P2P_WAIT_GO_FORM_INVITE_RSP:			retval = "P2P_WAIT_GO_FORM_INVITE_RSP";	break;
		default:										retval = "***Unknown Group Formation State***";
	}
	return(retval);
}

VOID decodeDeviceCap (UCHAR State)
{
	DBGPRINT(RT_DEBUG_TRACE, ("                         DeviceCapability = %x.\n", State));
	if (State & DEVCAP_SD)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [0] bServiceDiscovery\n"));
	if (State & DEVCAP_CLIENT_DISCOVER)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [1] bP2PClientDiscoverability\n"));
	if (State & DEVCAP_CLIENT_CONCURRENT)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [2] bConcurrentOperation\n"));
	if (State & DEVCAP_INFRA_MANAGED)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [3] bP2PInfrastructureManaged\n"));
	if (State & DEVCAP_DEVICE_LIMIT)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [4] bP2PDeviceLimit\n"));
	if (State & DEVCAP_INVITE)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [5] bP2PInvitationProcedure\n"));
}

VOID decodeGroupCap (UCHAR State)
{
	DBGPRINT(RT_DEBUG_TRACE, ("                         GroupCapability = %x.\n", State));
	if (State & GRPCAP_OWNER)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [0] bP2PGroupOwner\n"));
	if (State & GRPCAP_PERSISTENT)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [1] bPersistentP2PGroup\n"));
	if (State & GRPCAP_LIMIT)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [2] bP2PGroupLimit\n"));
	if (State & GRPCAP_INTRA_BSS)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [3] bIntra-BssDistribution\n"));
	if (State & GRPCAP_CROSS_CONNECT)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [4] bCrossConnection\n"));
	if (State & GRPCAP_PERSISTENT_RECONNECT)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [5] bPersistentReconnect\n"));
	if (State & GRPCAP_GROUP_FORMING)
		DBGPRINT(RT_DEBUG_TRACE, ("                             [6] bGroupFormation\n"));
}

/* 
	==========================================================================
	Description:
		debug print function for P2P.
	Return:

	NOTE:
		 
	==========================================================================
*/
VOID P2PPrintMac(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR	macindex)
{
	MAC_TABLE_ENTRY *pEntry;
	 
	DBGPRINT(RT_DEBUG_TRACE, (" ====================================================>P2PPrintMac i = %d,   \n", macindex));
	pEntry = &pAd->MacTab.Content[macindex];
	DBGPRINT(RT_DEBUG_TRACE, ("!! ,  HTMode = %x, WepStatus = %d, AuthMode = %d. \n", pEntry->HTPhyMode.word, pEntry->WepStatus, pEntry->AuthMode));

	DBGPRINT(RT_DEBUG_TRACE, ("ValidAsCLI = %d. ClientStatusFlags = %lx, \n", IS_ENTRY_CLIENT(pEntry), pEntry->ClientStatusFlags));
#ifdef RELEASE_EXLCUDE
		/* MAC_TABLE_ENTRY don't have P2pInfo, skip this. */
#endif /* RELEASE_EXCLUDE */
	DBGPRINT(RT_DEBUG_TRACE, ("devaddr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pEntry->Addr)));
#ifdef RELEASE_EXLCUDE
	/* MAC_TABLE_ENTRY don't have P2pInfo, skip this. */
#endif /* RELEASE_EXCLUDE */
}

/* 
	==========================================================================
	Description:
		debug print function for P2P Table entry.
	Return:

	NOTE:
		 
	==========================================================================
*/
VOID P2PPrintP2PEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		p2pindex)
{
	RT_P2P_CLIENT_ENTRY *pClient;
	int j;

	if (p2pindex >= MAX_P2P_GROUP_SIZE)
		return;
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: p2p Entry idx = %d\n", __FUNCTION__, p2pindex));
	DBGPRINT(RT_DEBUG_TRACE, ("====================================================>\n"));
	pClient = &pAd->P2pTable.Client[p2pindex];
	DBGPRINT(RT_DEBUG_TRACE, ("P2pClientState = %s  \n", decodeP2PClientState(pClient->P2pClientState)));
	DBGPRINT(RT_DEBUG_TRACE, ("ConfigMethod = %x.    %s \n", pClient->ConfigMethod, decodeConfigMethod(pClient->ConfigMethod)));
	DBGPRINT(RT_DEBUG_TRACE, ("Opchannel = %d.    Listenchannel = %d.\n", pClient->OpChannel, pClient->ListenChannel));
	DBGPRINT(RT_DEBUG_TRACE, ("MyGOIndex = %d. rule = %s.\n", pClient->MyGOIndex, decodeMyRule(pClient->Rule)));
	DBGPRINT(RT_DEBUG_TRACE, ("addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pClient->addr)));
	DBGPRINT(RT_DEBUG_TRACE, ("bssid =   %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pClient->bssid)));
	DBGPRINT(RT_DEBUG_TRACE, ("interface addr =   %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pClient->InterfaceAddr)));
	DBGPRINT(RT_DEBUG_TRACE, ("DeviceType = %02x %02x %02x %02x %02x %02x %02x %02x\n", pClient->PrimaryDevType[0], pClient->PrimaryDevType[1], pClient->PrimaryDevType[2],pClient->PrimaryDevType[3],pClient->PrimaryDevType[4],pClient->PrimaryDevType[5],pClient->PrimaryDevType[6],pClient->PrimaryDevType[7]));
	DBGPRINT(RT_DEBUG_TRACE, ("NumSecondaryType = %x.    RegClass = %x.    ConfigTimeOut = %x. \n", pClient->NumSecondaryType, pClient->RegClass, pClient->ConfigTimeOut));
	DBGPRINT(RT_DEBUG_TRACE, ("DeviceName[%ld] = %s\n", pClient->DeviceNameLen, pAd->P2pTable.Client[p2pindex].DeviceName));
	decodeDeviceCap(pClient->DevCapability);
	decodeGroupCap(pClient->GroupCapability);
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("<====================================================\n\n"));
}

/* 
	==========================================================================
	Description:
		debug print function for P2P Persistent Table entry.
	Return:

	NOTE:
		 
	==========================================================================
*/
VOID P2PPrintP2PPerstEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		p2pindex)
{
	RT_P2P_PERSISTENT_ENTRY *pClient;

	if (p2pindex >= MAX_P2P_TABLE_SIZE)
		return;
	
	DBGPRINT(RT_DEBUG_TRACE, (" P2PPrintP2PPerstEntry i = %d,   \n", p2pindex));
	pClient = &pAd->P2pTable.PerstEntry[p2pindex];
	DBGPRINT(RT_DEBUG_TRACE, ("addr = %02x:%02x:%02x:%02x:%02x:%02x\n",  PRINT_MAC(pClient->Addr)));
	DBGPRINT(RT_DEBUG_TRACE, ("Key = %x %x %x %x %x %x %x %x\n", pClient->Profile.Key[0], pClient->Profile.Key[1], pClient->Profile.Key[2],pClient->Profile.Key[3],pClient->Profile.Key[4],pClient->Profile.Key[5],pClient->Profile.Key[6],pClient->Profile.Key[7]));
	DBGPRINT(RT_DEBUG_TRACE, ("MacAddr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pClient->Profile.MacAddr)));
	DBGPRINT(RT_DEBUG_TRACE, ("Ssid %d = DIRECT-%c%c%c%c%c%c\n", pClient->Profile.SSID.SsidLength,pClient->Profile.SSID.Ssid[7], pClient->Profile.SSID.Ssid[8], pClient->Profile.SSID.Ssid[9],pClient->Profile.SSID.Ssid[10],pClient->Profile.SSID.Ssid[11],pClient->Profile.SSID.Ssid[12]));
	DBGPRINT(RT_DEBUG_TRACE, ("MyRule = %x, bValid = %x,    \n", pClient->MyRule, pClient->bValid));

}

/* 
	==========================================================================
	Description:
		debug print function for P2P.
	Return:

	NOTE:
		 
	==========================================================================
*/

VOID P2P_GoStartUp(
	IN PRTMP_ADAPTER 	pAd,
	IN INT         	bssidx)
{
	UINT32 rx_filter_flag;
	INT idx;
	ULONG		offset;
#ifdef INF_AMAZON_SE
	ULONG i;
#endif /* INF_AMAZON_SE */
	BOOLEAN		bWmmCapable = FALSE;
	BOOLEAN		TxPreamble, SpectrumMgmt = FALSE;
	UCHAR		BBPR1 = 0, BBPR3 = 0;
	UINT32		Value = 0;
#ifdef DOT1X_SUPPORT
/*	BOOLEAN		bDot1xReload = FALSE; */
#endif /* DOT1X_SUPPORT */
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("===> P2P_GoStartUp\n"));

	/*RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled); */
	/*pAd->MlmeAux.Channel = 0; */
	/*ScanNextChannel(pAd, OPMODE_STA); */


	pAd->P2pCfg.P2pCapability[1] |= GRPCAP_OWNER;
	pAd->StaCfg.bAutoReconnect = FALSE;
	pAd->ApCfg.MBSSID[0].UapsdInfo.bAPSDCapable = TRUE;
	pAd->P2pCfg.bSentProbeRSP = TRUE;
	/*pAd->P2pCfg.DiscCurrentState = P2P_DISC_LISTEN;*/

	/* Stop Scan. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
	RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
	pAd->MlmeAux.Channel = 0;
	ScanNextChannel(pAd, OPMODE_STA);
	}

	rx_filter_flag = APNORMAL;
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, rx_filter_flag);     /* enable RX of DMA block */

	pAd->ApCfg.BssidNum = 1;
	pAd->MacTab.MsduLifeTime = 20; /* default 5 seconds */
	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = TRUE;

	/*OPSTATUS_SET_FLAG(pAd, fOP_STATUS_P2P_GO); */
	pAd->flg_p2p_OpStatusFlags |= P2P_GO_UP;
	pAd->P2pCfg.GroupOpChannel = pAd->P2pCfg.GroupChannel;
	if (!INFRA_ON(pAd))
		pAd->CommonCfg.Channel = pAd->P2pCfg.GroupChannel;
	pAd->P2pCfg.Rule = P2P_IS_GO;

#ifdef INF_AMAZON_SE
	for (i = 0; i < NUM_OF_TX_RING; i++)
	{
		pAd->BulkOutDataSizeLimit[i]=24576;
	}
#endif /* INF_AMAZON_SE  */
		
	AsicDisableSync(pAd);


	if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
	{
		if (pAd->CommonCfg.Channel > 14)
			pAd->ApCfg.MBSSID[MAIN_MBSSID].PhyMode = PHY_11AN_MIXED;
		else
			pAd->ApCfg.MBSSID[MAIN_MBSSID].PhyMode = PHY_11BGN_MIXED;
	}
	else
	{
		if (pAd->CommonCfg.Channel > 14)
			pAd->ApCfg.MBSSID[MAIN_MBSSID].PhyMode = PHY_11A;
		else
			pAd->ApCfg.MBSSID[MAIN_MBSSID].PhyMode = PHY_11BG_MIXED;
	}

	TxPreamble = (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1);

	{
		PMULTISSID_STRUCT	pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

		if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID))
		{
			NdisMoveMemory(pMbss->Ssid, "P2P_Linux_AP", 12);
			pMbss->SsidLen = 12;			
		}

		if (pMbss->bWmmCapable)
		{
        	bWmmCapable = TRUE;
		}
		
		/* GO always use WPA2PSK / AES */
		pMbss->AuthMode = Ndis802_11AuthModeWPA2PSK;
 		pMbss->WepStatus = Ndis802_11Encryption3Enabled;
		pMbss->WscSecurityMode = WPA2PSKAES;
		pMbss->GroupKeyWepStatus = pMbss->WepStatus;
		pMbss->CapabilityInfo =
			CAP_GENERATE(1, 0, (pMbss->WepStatus != Ndis802_11EncryptionDisabled), TxPreamble, pAd->CommonCfg.bUseShortSlotTime, SpectrumMgmt);

#ifdef UAPSD_SUPPORT
		if (pAd->ApCfg.MBSSID[0].UapsdInfo.bAPSDCapable == TRUE)
		{
			/* QAPs set the APSD subfield to 1 within the Capability Information
			   field when the MIB attribute dot11APSDOptionImplemented is true
			   and set it to 0 otherwise. STAs always set this subfield to 0. */
            pMbss->CapabilityInfo |= 0x0800;
        } /* End of if */
#endif /* UAPSD_SUPPORT */

		/* decide the mixed WPA cipher combination */
		if (pMbss->WepStatus == Ndis802_11Encryption4Enabled)
		{
			switch ((UCHAR)pMbss->AuthMode)
			{
				/* WPA mode */
				case Ndis802_11AuthModeWPA:
				case Ndis802_11AuthModeWPAPSK:
					pMbss->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
					break;	

				/* WPA2 mode */
				case Ndis802_11AuthModeWPA2:
				case Ndis802_11AuthModeWPA2PSK:
					pMbss->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
					break;

				/* WPA and WPA2 both mode */
				case Ndis802_11AuthModeWPA1WPA2:
				case Ndis802_11AuthModeWPA1PSKWPA2PSK:	

					/* In WPA-WPA2 and TKIP-AES mixed mode, it shall use the maximum  */
					/* cipher capability unless users assign the desired setting. */
					if (pMbss->WpaMixPairCipher == MIX_CIPHER_NOTUSE || 
						pMbss->WpaMixPairCipher == WPA_TKIPAES_WPA2_NONE || 
						pMbss->WpaMixPairCipher == WPA_NONE_WPA2_TKIPAES)
						pMbss->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;										
					break;				
			}
											
		}
		else
		pMbss->WpaMixPairCipher = MIX_CIPHER_NOTUSE;

		/* Generate the corresponding RSNIE */
		RTMPMakeRSNIE(pAd, pMbss->AuthMode, pMbss->WepStatus, MAIN_MBSSID + MIN_NET_DEVICE_FOR_P2P_GO);

#ifdef WSC_V2_SUPPORT
		if (pMbss->WscControl.WscV2Info.bEnableWpsV2)
		{
			/*
				WPS V2 doesn't support WEP and WPA/WPAPSK-TKIP.
			*/
			if ((pMbss->WepStatus == Ndis802_11WEPEnabled) || (pMbss->WepStatus == Ndis802_11Encryption2Enabled))
				WscOnOff(pAd, MAIN_MBSSID | MIN_NET_DEVICE_FOR_P2P_GO, TRUE);
			else
				WscOnOff(pAd, MAIN_MBSSID | MIN_NET_DEVICE_FOR_P2P_GO, FALSE);
		}
#endif /* WSC_V2_SUPPORT */
	}

	if (INFRA_ON(pAd) && (pAd->StaActive.SupportedHtPhy.ChannelWidth == BW_40) && 
		(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA != pAd->StaActive.SupportedHtPhy.ExtChanOffset))
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = pAd->StaActive.SupportedHtPhy.ExtChanOffset;

	N_ChannelCheck(pAd);

#ifdef DOT11_N_SUPPORT
	RTMPSetPhyMode(pAd,  pAd->CommonCfg.PhyMode);
	P2P_GoSetCommonHT(pAd);	
#endif /* DOT11_N_SUPPORT */

#if defined(RT2883) || defined(RT3883)
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	{
		/* do nothing for DAC */
	}
	else
#endif /* defined(RT2883) || defined(RT3883) */
	{
#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->Antenna.field.TxPath == 2))
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
			BBPR1 &= (~0x18);
			BBPR1 |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
			BBPR1 &= (~0x18);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);
		}
	}

	/* Receiver Antenna selection, write to BBP R3(bit4:3) */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= (~0x18);
	if(pAd->Antenna.field.RxPath == 3)
	{
		BBPR3 |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		BBPR3 |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		BBPR3 |= (0x0);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

	if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
	if ((pAd->CommonCfg.PhyMode > PHY_11G) || bWmmCapable)
	{
		/* EDCA parameters used for AP's own transmission */
			pAd->CommonCfg.APEdcaParm.bValid = TRUE;
			pAd->CommonCfg.APEdcaParm.Aifsn[0] = 3;
			pAd->CommonCfg.APEdcaParm.Aifsn[1] = 7;
			pAd->CommonCfg.APEdcaParm.Aifsn[2] = 1;
			pAd->CommonCfg.APEdcaParm.Aifsn[3] = 1;

			pAd->CommonCfg.APEdcaParm.Cwmin[0] = 4;
			pAd->CommonCfg.APEdcaParm.Cwmin[1] = 4;
			pAd->CommonCfg.APEdcaParm.Cwmin[2] = 3;
			pAd->CommonCfg.APEdcaParm.Cwmin[3] = 2;

			pAd->CommonCfg.APEdcaParm.Cwmax[0] = 6;
			pAd->CommonCfg.APEdcaParm.Cwmax[1] = 10;
			pAd->CommonCfg.APEdcaParm.Cwmax[2] = 4;
			pAd->CommonCfg.APEdcaParm.Cwmax[3] = 3;

			pAd->CommonCfg.APEdcaParm.Txop[0]  = 0;
			pAd->CommonCfg.APEdcaParm.Txop[1]  = 0;
			pAd->CommonCfg.APEdcaParm.Txop[2]  = 94;	/*96; */
			pAd->CommonCfg.APEdcaParm.Txop[3]  = 47;	/*48; */
		AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);

		/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
			pAd->ApCfg.BssEdcaParm.bValid = TRUE;
			pAd->ApCfg.BssEdcaParm.Aifsn[0] = 3;
			pAd->ApCfg.BssEdcaParm.Aifsn[1] = 7;
			pAd->ApCfg.BssEdcaParm.Aifsn[2] = 2;
			pAd->ApCfg.BssEdcaParm.Aifsn[3] = 2;

			pAd->ApCfg.BssEdcaParm.Cwmin[0] = 4;
			pAd->ApCfg.BssEdcaParm.Cwmin[1] = 4;
			pAd->ApCfg.BssEdcaParm.Cwmin[2] = 3;
			pAd->ApCfg.BssEdcaParm.Cwmin[3] = 2;

			pAd->ApCfg.BssEdcaParm.Cwmax[0] = 10;
			pAd->ApCfg.BssEdcaParm.Cwmax[1] = 10;
			pAd->ApCfg.BssEdcaParm.Cwmax[2] = 4;
			pAd->ApCfg.BssEdcaParm.Cwmax[3] = 3;

			pAd->ApCfg.BssEdcaParm.Txop[0]  = 0;
			pAd->ApCfg.BssEdcaParm.Txop[1]  = 0;
			pAd->ApCfg.BssEdcaParm.Txop[2]  = 94;	/*96; */
			pAd->ApCfg.BssEdcaParm.Txop[3]  = 47;	/*48; */
		}
	else
		AsicSetEdcaParm(pAd, NULL);
	}

#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.PhyMode < PHY_11ABGN_MIXED)
	{
		/* Patch UI */
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
	}

	/* init */
	if (pAd->CommonCfg.bRdg)
	{	
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		AsicEnableRDG(pAd);
	}
	else	
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		AsicDisableRDG(pAd);
	}	
#endif /* DOT11_N_SUPPORT */

	AsicSetBssid(pAd, pAd->CurrentAddress); 
	AsicSetMcastWC(pAd);

	/* In AP mode,  First WCID Table in ASIC will never be used. To prevent it's 0xff-ff-ff-ff-ff-ff, Write 0 here. */
	/* p.s ASIC use all 0xff as termination of WCID table search. */
	RTMP_IO_WRITE32(pAd, MAC_WCID_BASE, 0x00);
	RTMP_IO_WRITE32(pAd, MAC_WCID_BASE+4, 0x0);

	/* reset WCID table */
	for (idx=2; idx<255; idx++)
	{
		offset = MAC_WCID_BASE + (idx * HW_WCID_ENTRY_SIZE);	
		RTMP_IO_WRITE32(pAd, offset, 0x0);
		RTMP_IO_WRITE32(pAd, offset+4, 0x0);
	}

	pAd->MacTab.Content[0].Addr[0] = 0x01;
	pAd->MacTab.Content[0].HTPhyMode.field.MODE = MODE_OFDM;
	pAd->MacTab.Content[0].HTPhyMode.field.MCS = 3;
	/*pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;*/
	pAd->P2PChannel = pAd->CommonCfg.Channel;

	AsicBBPAdjust(pAd);

	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
	if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED
		&& pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
	N_SetCenCh(pAd);	
	AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);	
	MlmeUpdateTxRates(pAd, FALSE, MIN_NET_DEVICE_FOR_P2P_GO);
#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.PhyMode > PHY_11G)
		MlmeUpdateHtTxRates(pAd, MIN_NET_DEVICE_FOR_P2P_GO);
#endif /* DOT11_N_SUPPORT */

	/* Disable Protection first. */
	if (!INFRA_ON(pAd))
		AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);

	APUpdateCapabilityAndErpIe(pAd);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_N_SUPPORT
#endif /* DOT11_N_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	/* Set LED */
	RTMPSetLED(pAd, LED_LINK_UP);
#endif /* LED_CONTROL_SUPPORT */

	/* Initialize security variable per entry, 
		1. 	pairwise key table, re-set all WCID entry as NO-security mode.
		2.	access control port status
	*/
	for (idx=2; idx<MAX_LEN_OF_MAC_TABLE; idx++)
	{
		pAd->MacTab.Content[idx].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		AsicRemovePairwiseKeyEntry(pAd, (UCHAR)idx);
	}

	{
		USHORT		Wcid = 0;	
		PMULTISSID_STRUCT	pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

		pMbss->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

		if (IS_WPA_CAPABILITY(pMbss->AuthMode))
		{   
			pMbss->DefaultKeyId = 1;
		}

		/* Get a specific WCID to record this MBSS key attribute */
		GET_GroupKey_WCID(pAd, Wcid, MAIN_MBSSID);

		/* When WEP, TKIP or AES is enabled, set group key info to Asic */
		if (pMbss->WepStatus == Ndis802_11WEPEnabled)
		{
    			UCHAR	CipherAlg;
			UCHAR	idx_len;

			for (idx=0; idx < SHARE_KEY_NUM; idx++)
			{
				CipherAlg = pAd->SharedKey[MAIN_MBSSID + 1][idx].CipherAlg;

				if (pAd->SharedKey[MAIN_MBSSID + 1][idx].KeyLen > 0)
				{
					/* Set key material to Asic */
    				RTMP_ASIC_SHARED_KEY_TABLE(pAd,
    											MAIN_MBSSID + 1,
    											idx,
    											&pAd->SharedKey[MAIN_MBSSID + 1][idx]);	
		
					if (idx == pMbss->DefaultKeyId)
					{
						/* Generate 3-bytes IV randomly for software encryption using */						
				    	for(idx_len = 0; idx_len < LEN_WEP_TSC; idx_len++)
							pAd->SharedKey[MAIN_MBSSID + 1][idx].TxTsc[idx_len] = RandomByte(pAd);   
											
						/* Update WCID attribute table and IVEIV table */
						RTMPSetWcidSecurityInfo(pAd, 
												MAIN_MBSSID + 1, 
												idx, 												
												CipherAlg,
												Wcid, 
												SHAREDKEYTABLE);					
					}
				}
			}
    	}
		else if ((pMbss->WepStatus == Ndis802_11Encryption2Enabled) ||
				 (pMbss->WepStatus == Ndis802_11Encryption3Enabled) ||
				 (pMbss->WepStatus == Ndis802_11Encryption4Enabled))
		{
			/* Generate GMK and GNonce randomly per MBSS */
			GenRandom(pAd, pMbss->Bssid, pMbss->GMK);
			GenRandom(pAd, pMbss->Bssid, pMbss->GNonce);		

			/* Derive GTK per BSSID */
			WpaDeriveGTK(pMbss->GMK, 
						(UCHAR*)pMbss->GNonce, 
						pMbss->Bssid, 
						pMbss->GTK, 
						LEN_TKIP_GTK);

			/* Install Shared key */
			WPAInstallSharedKey(pAd, 
								pMbss->GroupKeyWepStatus, 
								MAIN_MBSSID + 1, 
								pMbss->DefaultKeyId, 
								Wcid,
								TRUE,
								pMbss->GTK,
								LEN_TKIP_GTK);

		}
#ifdef WAPI_SUPPORT
		else if (pMbss->WepStatus == Ndis802_11EncryptionSMS4Enabled)
		{	
			INT	cnt;
		
			/* Initial the related variables */
			pMbss->DefaultKeyId = 0;
			NdisMoveMemory(pMbss->key_announce_flag, AE_BCAST_PN, LEN_WAPI_TSC);
			if (IS_HW_WAPI_SUPPORT(pAd))
				pMbss->sw_wpi_encrypt = FALSE;					
			else
				pMbss->sw_wpi_encrypt = TRUE;

			/* Generate NMK randomly */
			for (cnt = 0; cnt < LEN_WAPI_NMK; cnt++)
				pMbss->NMK[cnt] = RandomByte(pAd);
			
			/* Count GTK for this BSSID */
			RTMPDeriveWapiGTK(pMbss->NMK, pMbss->GTK);

			/* Install Shared key */
			WAPIInstallSharedKey(pAd, 
								 pMbss->GroupKeyWepStatus, 
								 MAIN_MBSSID + 1, 
								 pMbss->DefaultKeyId, 
								 Wcid,
								 pMbss->GTK);
			
		}
#endif /* WAPI_SUPPORT */

#ifdef DOT1X_SUPPORT
		/* Send singal to daemon to indicate driver had restarted */
		if ((pMbss->AuthMode == Ndis802_11AuthModeWPA) || (pMbss->AuthMode == Ndis802_11AuthModeWPA2)
        		|| (pMbss->AuthMode == Ndis802_11AuthModeWPA1WPA2) || (pMbss->IEEE8021X == TRUE))
		{
			;/*bDot1xReload = TRUE; */
    	}
#endif /* DOT1X_SUPPORT */

		DBGPRINT(RT_DEBUG_TRACE, ("### BSS(%d) AuthMode(%d)=%s, WepStatus(%d)=%s , AccessControlList.Policy=%ld\n", MAIN_MBSSID + 1, pMbss->AuthMode, GetAuthMode(pMbss->AuthMode), 
																  pMbss->WepStatus, GetEncryptType(pMbss->WepStatus), pMbss->AccessControlList.Policy));

#ifdef DOT1X_SUPPORT
	/* Send internal command to DOT1X daemon for reloading configuration */
/*
	if (bDot1xReload)
		DOT1X_InternalCmdAction(pAd, NULL, DOT1X_RELOAD_CONFIG);
*/
#endif /* DOT1X_SUPPORT */

	/* Disable Protection first. */
	/*AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE); */
#ifdef PIGGYBACK_SUPPORT
	RTMPSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */

	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);

#if defined(WSC_AP_SUPPORT) || defined(WSC_STA_SUPPORT)
		{
			PWSC_CTRL pWscControl;
			UCHAR zeros16[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			
			pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl;
			DBGPRINT(RT_DEBUG_TRACE, ("Generate UUID for apidx(%d)\n", MAIN_MBSSID));
			if (NdisEqualMemory(&pWscControl->Wsc_Uuid_E[0], zeros16, UUID_LEN_HEX))
				WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], MIN_NET_DEVICE_FOR_P2P_GO, FALSE);
			WscInit(pAd, FALSE, MIN_NET_DEVICE_FOR_P2P_GO);
		}
#endif /* defined(WSC_AP_SUPPORT) || defined(WSC_STA_SUPPORT) */
	}

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);


	/*pAd->Dot11_H.RDMode = RD_NORMAL_MODE;*/
	/*AsicEnableP2PGoSync(pAd); */

	/* start sending BEACON out */
	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);

	/*AsicEnableP2PGoSync(pAd); */

	{
		pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
		AsicEnableP2PGoSync(pAd);

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef A_BAND_SUPPORT
		if (pAd->CommonCfg.Channel > 14)
		{
			if ((pAd->CommonCfg.CarrierDetect.Enable == 0)
				&& ((pAd->CommonCfg.RDDurRegion == JAP)
					|| (pAd->CommonCfg.RDDurRegion == JAP_W53)
					|| (pAd->CommonCfg.RDDurRegion == JAP_W56)))
			{
				pAd->CommonCfg.CarrierDetect.Enable = 1;
			}
		}
		else
#endif /* A_BAND_SUPPORT */		
		{
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
			{
				if ((pAd->CommonCfg.CarrierDetect.Enable == 0)
						&& ((pAd->CommonCfg.RDDurRegion == JAP)
							|| (pAd->CommonCfg.RDDurRegion == JAP_W53)
							|| (pAd->CommonCfg.RDDurRegion == JAP_W56)))
				{
					pAd->CommonCfg.CarrierDetect.Enable = 1;
				}
			}
		}

		if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		{
			/* trun on Carrier-Detection.*/
			CarrierDetectionStart(pAd);
		}
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef WAPI_SUPPORT
	RTMPStartWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */

	/* Pre-tbtt interrupt setting. */
	RTMP_IO_READ32(pAd, INT_TIMER_CFG, &Value);
	Value &= 0xffff0000;
	Value |= 6 << 4; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable. */
	RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, Value);
	/* Enable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value |=0x1;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);

	/* Set group re-key timer if necessary.  
	   It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS" */
	WPA_APSetGroupRekeyAction(pAd);

#ifdef WDS_SUPPORT
	/* Prepare WEP key */
	WdsPrepareWepKeyFromMainBss(pAd);

	/* Add wds key infomation to ASIC */
	AsicUpdateWdsRxWCIDTable(pAd);
#endif /* WDS_SUPPORT */

#ifdef IDS_SUPPORT
	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable)
	{
#ifdef SYSTEM_LOG_SUPPORT	
		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			DBGPRINT(RT_DEBUG_WARN, ("!!! WARNING !!! The WirelessEvent parameter doesn't be enabled \n"));
#endif /* SYSTEM_LOG_SUPPORT */
		
		RTMPIdsStart(pAd);
	}
#endif /* IDS_SUPPORT */





	/* Start respons Auth Req. */
	pAd->P2pCfg.bStopAuthRsp = FALSE;
	DBGPRINT(RT_DEBUG_TRACE, ("<=== P2P_GoStartUp\n"));
}

VOID P2P_GoStop(
	IN PRTMP_ADAPTER pAd) 
{
	BOOLEAN     Cancelled;
	UINT32		Value;
	INT			apidx;
	
	DBGPRINT(RT_DEBUG_TRACE, ("===> P2P_GoStop\n"));

	pAd->P2pCfg.P2pCapability[1] &= ~(GRPCAP_OWNER);

	pAd->flg_p2p_OpStatusFlags &= P2P_FIXED_MODE;

	pAd->P2pCfg.Rule = P2P_IS_DEVICE;

	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = FALSE;
	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);

#ifdef DFS_SUPPORT
	NewRadarDetectionStop(pAd);
#endif /* DFS_SUPPORT */


#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

	P2PMacTableReset(pAd);

	/* Disable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &=0xe;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);

	if (!INFRA_ON(pAd))
	{
		/* Disable piggyback */
		RTMPSetPiggyBack(pAd, FALSE);

   		AsicUpdateProtect(pAd, 0,  (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);

	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		/*RTMP_ASIC_INTERRUPT_DISABLE(pAd); */
		AsicDisableSync(pAd);

#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif /* LED_CONTROL_SUPPORT */
	}



	for (apidx = 0; apidx < MAX_MBSSID_NUM(pAd); apidx++)
	{
		if (pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning == TRUE)
		{
			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].REKEYTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning = FALSE;
		}
	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
	}
	
#ifdef WAPI_SUPPORT
	RTMPCancelWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */
	
	/*
	 * Cancel the Timer, to make sure the timer was not queued.
	 */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */



	
	DBGPRINT(RT_DEBUG_TRACE, ("<=== P2P_GoStop\n"));
}

VOID P2P_CliStartUp(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR ifIndex;
	PAPCLI_STRUCT pApCliEntry;

	/* Reset is in progress, stop immediately */
	if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
		 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	/* sanity check whether the interface is initialized. */
	if (pAd->flg_apcli_init != TRUE)
		return;

	pAd->P2pCfg.P2pCapability[1] &= ~(GRPCAP_OWNER);
	pAd->flg_p2p_OpStatusFlags = P2P_CLI_UP;
	pAd->P2pCfg.Rule = P2P_IS_CLIENT;
	pAd->StaCfg.bAutoReconnect = FALSE;
	AsicSetBssid(pAd, pAd->CurrentAddress); 
	AsicSetMcastWC(pAd);

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

		if (pAd->CommonCfg.Channel != 0)
			pAd->P2PChannel = pAd->CommonCfg.Channel;
		else
			pAd->P2PChannel  = FirstChannel(pAd);
		
		if (APCLI_IF_UP_CHECK(pAd, ifIndex) 
			&& (pApCliEntry->Enable == TRUE)
			&& (pApCliEntry->Valid == FALSE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startup.\n", __FUNCTION__, ifIndex));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex);
		}
	}

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	return;
}

VOID P2P_CliStop(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR ifIndex;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===>\n", __FUNCTION__));

	/*OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_P2P_CLI); */
	pAd->flg_p2p_OpStatusFlags = P2P_DISABLE;
	pAd->P2pCfg.Rule = P2P_IS_DEVICE;
	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		/* send disconnect-req to sta State Machine. */
		if (pAd->ApCfg.ApCliTab[ifIndex].Enable)
		{
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
			RTMP_MLME_HANDLER(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));
		}
	}
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);


}

/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/
VOID P2PMacTableMaintenance(
	IN PRTMP_ADAPTER pAd)
{
	int         i, FirstWcid;
#ifdef DOT11_N_SUPPORT
	ULONG MinimumAMPDUSize = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor; /*Default set minimum AMPDU Size to 2, i.e. 32K */
	BOOLEAN	bRdgActive;
#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags;
#endif /* RTMP_MAC_PCI */
	UINT	fAnyStationPortSecured[MAX_MBSSID_NUM(pAd)];
 	UINT 	bss_index;
	MAC_TABLE *pMacTable;

	FirstWcid = 2;

	for (bss_index = BSS0; bss_index < MAX_MBSSID_NUM(pAd); bss_index++)
		fAnyStationPortSecured[bss_index] = 0;

	pMacTable = &pAd->MacTab;
	pMacTable->fAnyStationInPsm = FALSE;
	pMacTable->fAnyStationBadAtheros = FALSE;
	pMacTable->fAnyTxOPForceDisable = FALSE;
	pMacTable->fAllStationAsRalink = TRUE;
#ifdef DOT11_N_SUPPORT
	pMacTable->fAnyStationNonGF = FALSE;
	pMacTable->fAnyStation20Only = FALSE;
	pMacTable->fAnyStationIsLegacy = FALSE;
	pMacTable->fAnyStationMIMOPSDynamic = FALSE;
#ifdef GREENAP_SUPPORT
	//Support Green AP
	pMacTable->fAnyStationIsHT=FALSE;
#endif /* GREENAP_SUPPORT */

#ifdef DOT11N_DRAFT3
	pMacTable->fAnyStaFortyIntolerant = FALSE;
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#ifdef WAPI_SUPPORT
	pMacTable->fAnyWapiStation = FALSE;
#endif /* WAPI_SUPPORT */

	for (i = FirstWcid; i < MAX_LEN_OF_MAC_TABLE; i++) 
	{
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[i];

		BOOLEAN bDisconnectSta = FALSE;

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->NoDataIdleCount == 0)
			pEntry->StationKeepAliveCount = 0;

		pEntry->NoDataIdleCount ++;  
		pEntry->StaConnectTime ++;

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x fail to complete ASSOC in %d sec\n",
					pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
					pEntry->Addr[4],pEntry->Addr[5],MAC_TABLE_ASSOC_TIMEOUT));
			MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
			continue;
		}

		/* 1. check if there's any associated STA in power-save mode. this affects outgoing */
		/*    MCAST/BCAST frames should be stored in PSQ till DtimCount=0 */
		if (pEntry->PsMode == PWR_SAVE)
			pMacTable->fAnyStationInPsm = TRUE;

#ifdef DOT11_N_SUPPORT
		if (pEntry->MmpsMode == MMPS_DYNAMIC)
		{
			pMacTable->fAnyStationMIMOPSDynamic = TRUE;
		}

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = TRUE;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD)
			pMacTable->fAnyStationNonGF = TRUE;

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
		{
			pMacTable->fAnyStationIsLegacy = TRUE;
		}
#ifdef GREENAP_SUPPORT
		else
		{
			pMacTable->fAnyStationIsHT=TRUE;
		}
#endif /* GREENAP_SUPPORT */

#ifdef DOT11N_DRAFT3
		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = TRUE;
#endif /* DOT11N_DRAFT3 */

		/* Get minimum AMPDU size from STA	 */
		if (MinimumAMPDUSize > pEntry->MaxRAmpduFactor) 
		{
			MinimumAMPDUSize = pEntry->MaxRAmpduFactor;						
		}
#endif /* DOT11_N_SUPPORT */
		
		if (pEntry->bIAmBadAtheros)
		{
			pMacTable->fAnyStationBadAtheros = TRUE;

			if (!INFRA_ON(pAd))
			{
#ifdef DOT11_N_SUPPORT
				if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
					AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, pMacTable->fAnyStationNonGF);
#endif /* DOT11_N_SUPPORT */

#ifndef DOT11N_SS3_SUPPORT
				if (pEntry->WepStatus != Ndis802_11EncryptionDisabled)
				{
					pMacTable->fAnyTxOPForceDisable = TRUE;
				}
#endif /* DOT11N_SS3_SUPPORT */
			}
		}

		/* detect the station alive status */
		/* detect the station alive status */
		if ((pAd->ApCfg.MBSSID[pEntry->apidx].StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pAd->ApCfg.MBSSID[pEntry->apidx].StationKeepAliveTime))
		{
			MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];

			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

		        ......
		        sam> tx ok!
		        sam> count = 1!	 ==> 1 second after the Null Frame is acked
		        sam> count = 2!	 ==> 2 second after the Null Frame is acked
		        sam> count = 3!
		        sam> count = 4!
		        sam> count = 5!
		        sam> count = 6!
		        sam> count = 7!
		        sam> count = 8!
		        sam> count = 9!
		        sam> count = 10!
		        sam> count = 11!
		        sam> count = 12!
		        sam> count = 13!
		        sam> count = 14!
		        sam> count = 15! ==> 15 second after the Null Frame is acked
		        sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
		        sam> count = 1!  ==> 1 second after the Null Frame is acked
		        sam> count = 2!
		        sam> count = 3!
		        sam> count = 4!
		        ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we dont support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/

			if (pEntry->StationKeepAliveCount++ == 0)
			{
#ifdef P2P_SUPPORT
				/* Modify for P2P test plan 6.1.11 /6.1.12, enqueue null frame will influence the test item */
				if (pAd->P2pCfg.bSigmaEnabled == FALSE)
				{
#endif /* P2P_SUPPORT */
					if (pEntry->PsMode == PWR_SAVE)
					{
						/* use TIM bit to detect the PS station */
						WLAN_MR_TIM_BIT_SET(pAd, pEntry->apidx, pEntry->Aid);
					}
					else
					{
						/* use Null or QoS Null to detect the ACTIVE station */
						BOOLEAN bQosNull = FALSE;
	
						if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
							bQosNull = TRUE;

		            			ApEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
	    	                           	pEntry->Aid, pEntry->apidx, bQosNull, TRUE, 0);
					}
#ifdef P2P_SUPPORT
				}
#endif /* P2P_SUPPORT */				
			}
			else
			{
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime)
					pEntry->StationKeepAliveCount = 0;
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */
		if (pEntry->NoDataIdleCount >= pEntry->StaIdleTimeout)
		{
			bDisconnectSta = TRUE;
			DBGPRINT(RT_DEBUG_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
					pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
					pEntry->Addr[4],pEntry->Addr[5],pEntry->StaIdleTimeout));
			ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
		}
		else if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
		{
			/*
				AP have no way to know that the PwrSaving STA is leaving or not.
				So do not disconnect for PwrSaving STA.
			*/
			if (pEntry->PsMode != PWR_SAVE)
			{
				bDisconnectSta = TRUE;
				DBGPRINT(RT_DEBUG_WARN, ("STA-%02x:%02x:%02x:%02x:%02x:%02x had left\n",
					pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
					pEntry->Addr[4],pEntry->Addr[5]));			
			}
		}

		if (bDisconnectSta)
		{
			/* send wireless event - for ageout  */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0); 

			if (pEntry->Sst == SST_ASSOC)
			{
				PUCHAR      pOutBuffer = NULL;
				NDIS_STATUS NStatus;
				ULONG       FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				USHORT      Reason;

				/*  send out a DISASSOC request frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS) 
				{
					DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					/*NdisReleaseSpinLock(&pAd->MacTabLock); */
					continue;
				}
				Reason = REASON_DEAUTH_STA_LEAVING;
				DBGPRINT(RT_DEBUG_WARN, ("Send DEAUTH - Reason = %d frame  TO %02x:%02x:%02x:%02x:%02x:%02x. \n",Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
#ifdef P2P_SUPPORT
									pAd->ApCfg.MBSSID[pEntry->apidx].Bssid,
#endif /* P2P_SUPPORT */
									pAd->ApCfg.MBSSID[pEntry->apidx].Bssid);				
		    	MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
		    	                  sizeof(HEADER_802_11), &DeAuthHdr, 
		    	                  2,                     &Reason, 
		    	                  END_OF_ARGS);				
		    	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		    	MlmeFreeMemory(pAd, pOutBuffer);
			}

			MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
			continue;
		}

		/* 3. garbage collect the PsQueue if the STA has being idle for a while */
		if (pEntry->PsQueue.Head)
		{
			pEntry->PsQIdleCount ++;  
			if (pEntry->PsQIdleCount > 2) 
			{
				NdisAcquireSpinLock(&pAd->irq_lock);
				APCleanupPsQueue(pAd, &pEntry->PsQueue);
				NdisReleaseSpinLock(&pAd->irq_lock);
				pEntry->PsQIdleCount = 0;
				WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->apidx, pEntry->Aid);
			}
		}
		else
			pEntry->PsQIdleCount = 0;
	
#ifdef UAPSD_SUPPORT
        UAPSD_QueueMaintenance(pAd, pEntry);
#endif /* UAPSD_SUPPORT */

		/* check if this STA is Ralink-chipset  */
		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
			pMacTable->fAllStationAsRalink = FALSE;

		/* Check if the port is secured */
		if (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
			fAnyStationPortSecured[pEntry->apidx]++;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if ((pEntry->BSS2040CoexistenceMgmtSupport) 
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		)
		{
			SendNotifyBWActionFrame(pAd, pEntry->Aid, pEntry->apidx);
	}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef WAPI_SUPPORT
		if (pEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled)
			pMacTable->fAnyWapiStation = TRUE;
#endif /* WAPI_SUPPORT */

	}

	/* Update the state of port per MBSS */
	for (bss_index = BSS0; bss_index < MAX_MBSSID_NUM(pAd); bss_index++)
	{
		if (fAnyStationPortSecured[bss_index] > 0)
		{
			pAd->ApCfg.MBSSID[bss_index].PortSecured = WPA_802_1X_PORT_SECURED;
		}
		else
			pAd->ApCfg.MBSSID[bss_index].PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);
#endif /* DOT11N_DRAFT3 */

	/* If all associated STAs are Ralink-chipset, AP shall enable RDG. */
	if (pAd->CommonCfg.bRdg && pMacTable->fAllStationAsRalink)
	{
		bRdgActive = TRUE;
	}
	else
	{
		bRdgActive = FALSE;
	}
#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	if (pAd->CommonCfg.PhyMode>=PHY_11ABGN_MIXED)
	{
		if(pAd->MacTab.fAnyStationIsHT==FALSE
			&& pAd->ApCfg.bGreenAPEnable == TRUE)
		{
			if (pAd->ApCfg.GreenAPLevel!=GREENAP_ONLY_11BG_STAS)
			{
				RTMP_CHIP_ENABLE_AP_MIMOPS(pAd);
				pAd->ApCfg.GreenAPLevel=GREENAP_ONLY_11BG_STAS;
			}
		}
		else
		{
			if (pAd->ApCfg.GreenAPLevel!=GREENAP_11BGN_STAS)
			{
				RTMP_CHIP_DISABLE_AP_MIMOPS(pAd);
				pAd->ApCfg.GreenAPLevel=GREENAP_11BGN_STAS;
			}
		}
	}
#endif /* GREENAP_SUPPORT */

	if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
	{
		if (bRdgActive)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
			AsicEnableRDG(pAd);
		}
		else
		{
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
			AsicDisableRDG(pAd);
		}
	}
#endif /* DOT11_N_SUPPORT */


	if (!INFRA_ON(pAd))
	{
		if ((pMacTable->fAnyStationBadAtheros == FALSE) && (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == TRUE))
		{
			AsicUpdateProtect(pAd, pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, pMacTable->fAnyStationNonGF);
		}
	}
#endif /* DOT11_N_SUPPORT */

#ifdef RTMP_MAC_PCI
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	/* 
	 *	4. garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
	 *    stale in queue. Since MCAST/BCAST frames always been sent out whenever 
	 *    DtimCount==0, the only case to let them stale is surprise removal of the NIC,
	 *    so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	 */
	if (pMacTable->McastPsQueue.Head)
	{
		UINT bss_index;

		pMacTable->PsQIdleCount ++;
		if (pMacTable->PsQIdleCount > 1)
		{

			/*NdisAcquireSpinLock(&pAd->MacTabLock); */
			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			/*NdisReleaseSpinLock(&pAd->MacTabLock); */
			pMacTable->PsQIdleCount = 0;

		        /* sanity check */
			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
		        /* End of if */
	        
		        /* clear MCAST/BCAST backlog bit for all BSS */
			for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		        /* End of for */
		}
	}
	else
		pMacTable->PsQIdleCount = 0;
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
}

VOID AsicEnableP2PGoSync(
	IN PRTMP_ADAPTER pAd)
{
	BCN_TIME_CFG_STRUC csr;

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);

	
	/* start sending BEACON */
	csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; /* ASIC register in units of 1/16 TU */
	csr.field.bTsfTicking = 1;
	csr.field.TsfSyncMode = 3; /* sync TSF in IBSS mode */
	csr.field.bTBTTEnable = 1;
	csr.field.bBeaconGen = 1;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}

VOID MgtMacP2PHeaderInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN OUT PHEADER_802_11 pHdr80211, 
	IN UCHAR SubType, 
	IN UCHAR ToDs, 
	IN PUCHAR pDA, 
	IN PUCHAR pBssid) 
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	
	pHdr80211->FC.Type = BTYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
	COPY_MAC_ADDR(pHdr80211->Addr2, pBssid);
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}

/*
	==========================================================================
	Description:
		This routine reset the entire MAC table. All packets pending in
		the power-saving queues are freed here.
	==========================================================================
 */
VOID P2PMacTableReset(
	IN  PRTMP_ADAPTER  pAd)
{
	int         i, FirstWcid;
	BOOLEAN     Cancelled;    
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags;
#endif /* RTMP_MAC_PCI */
	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT      Reason;
    UCHAR       apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("MacTableReset\n"));
	/*NdisAcquireSpinLock(&pAd->MacTabLock); */


	FirstWcid = 2;

	for (i = FirstWcid; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry))
		{
	   		/* Delete a entry via WCID */

			/*MacTableDeleteEntry(pAd, i, pAd->MacTab.Content[i].Addr); */
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);

#ifdef CONFIG_STA_SUPPORT
#ifdef ADHOC_WPA2PSK_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
    			RTMPCancelTimer(&pEntry->WPA_Authenticator.MsgRetryTimer, &Cancelled);
            }
#endif /* ADHOC_WPA2PSK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

            pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;

#ifdef CONFIG_AP_SUPPORT
			if ((pAd->OpMode == OPMODE_AP) || IS_P2P_GO_ENTRY(pEntry))
			{
				/* Before reset MacTable, send disassociation packet to client. */
				if (pEntry->Sst == SST_ASSOC)
				{
					/*  send out a De-authentication request frame */
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
					if (NStatus != NDIS_STATUS_SUCCESS)
					{
						DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock); */
						return;
					}
					
					Reason = REASON_NO_LONGER_VALID;
					DBGPRINT(RT_DEBUG_WARN, ("Send DEAUTH - Reason = %d frame tO %02x:%02x:%02x:%02x:%02x:%02x \n",
												Reason, PRINT_MAC(pAd->MacTab.Content[i].Addr)));

					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->HdrAddr1,
#ifdef P2P_SUPPORT
										pEntry->HdrAddr2,
#endif /* P2P_SUPPORT */
										pEntry->HdrAddr3);
			    	MakeOutgoingFrame(pOutBuffer,            &FrameLen,
			    	                  sizeof(HEADER_802_11), &DeAuthHdr,
			    	                  2,                     &Reason,
			    	                  END_OF_ARGS);

			    	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			    	MlmeFreeMemory(pAd, pOutBuffer);
					RTMPusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */

			/* Delete a entry via WCID */
			MacTableDeleteEntry(pAd, i, pEntry->Addr);
		}
		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
	}

#ifdef CONFIG_AP_SUPPORT
	{
		for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
#ifdef WSC_AP_SUPPORT
			PWSC_CTRL   pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
			BOOLEAN Cancelled;
			
	    	RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
	    	pWscControl->EapolTimerRunning = FALSE;
			NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
	    	pWscControl->EapMsgRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
			pAd->ApCfg.MBSSID[apidx].StaCount = 0; 
		}
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
		DBGPRINT(RT_DEBUG_TRACE, ("McastPsQueue.Number %ld...\n",pAd->MacTab.McastPsQueue.Number));
		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);
		DBGPRINT(RT_DEBUG_TRACE, ("2McastPsQueue.Number %ld...\n",pAd->MacTab.McastPsQueue.Number));

		/*NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE)); */
		InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}

VOID P2PChannelInit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		ifIndex)
{
	if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
#ifdef DOT11_N_SUPPORT
		ADD_HTINFO	RootApHtInfo, HtInfo;

		HtInfo = pAd->CommonCfg.AddHTInfo.AddHtInfo;
		RootApHtInfo = pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.AddHtInfo.AddHtInfo;

		if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
			(RootApHtInfo.RecomWidth) &&
			(RootApHtInfo.ExtChanOffset != HtInfo.ExtChanOffset))
		{			
			if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE)
				Set_HtExtcha_Proc(pAd, "1");
			else
				Set_HtExtcha_Proc(pAd, "0");
		}

		N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */
		{
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.Channel);
		}
	}
}

VOID	P2PCfgInit(
	IN	PRTMP_ADAPTER pAd)
{
	UCHAR apcliIdx, apidx = MAIN_MBSSID;

	pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeOpen;
	pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11EncryptionDisabled;
	pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus = Ndis802_11EncryptionDisabled;
	pAd->ApCfg.MBSSID[apidx].DefaultKeyId = 0;
	pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = MIX_CIPHER_NOTUSE;

#ifdef DOT1X_SUPPORT
	pAd->ApCfg.MBSSID[apidx].IEEE8021X = FALSE;
	pAd->ApCfg.MBSSID[apidx].PreAuth = FALSE;

	/* PMK cache setting */
	pAd->ApCfg.MBSSID[apidx].PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute) */
	NdisZeroMemory(&pAd->ApCfg.MBSSID[apidx].PMKIDCache, sizeof(NDIS_AP_802_11_PMKID));

	/* dot1x related per BSS */
	pAd->ApCfg.MBSSID[apidx].radius_srv_num = 0;
	pAd->ApCfg.MBSSID[apidx].NasIdLen = 0;
#endif /* DOT1X_SUPPORT */

	/* VLAN related */
	pAd->ApCfg.MBSSID[apidx].VLAN_VID = 0;

	/* Default MCS as AUTO */
	pAd->ApCfg.MBSSID[apidx].bAutoTxRateSwitch = TRUE;
	pAd->ApCfg.MBSSID[apidx].DesiredTransmitSetting.field.MCS = MCS_AUTO;

	/* Default is zero. It means no limit. */
	pAd->ApCfg.MBSSID[apidx].MaxStaNum = 0;
	pAd->ApCfg.MBSSID[apidx].StaCount = 0;
			
#ifdef P2P_SUPPORT
#ifdef APCLI_SUPPORT
	for(apcliIdx = 0; apcliIdx < MAX_APCLI_NUM; apcliIdx++)
	{
		pAd->ApCfg.ApCliTab[apcliIdx].AuthMode = Ndis802_11AuthModeOpen;
		pAd->ApCfg.ApCliTab[apcliIdx].WepStatus = Ndis802_11WEPDisabled;
	}
#endif /* APCLI_SUPPORT */
#endif /* P2P_SUPPORT */

	for(apcliIdx = 0; apcliIdx < WLAN_MAX_NUM_OF_TIM; apcliIdx++)
		pAd->ApCfg.MBSSID[apidx].TimBitmaps[apcliIdx] = 0;

	P2pGotoIdle(pAd);
}

VOID P2PInitChannelRelatedValue(
	IN PRTMP_ADAPTER pAd)
{
}

/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID P2PUpdateMlmeRate(
	IN PRTMP_ADAPTER pAd,
	USHORT ifIndex)
{
	UCHAR	MinimumRate;
	UCHAR	ProperMlmeRate; /*= RATE_54; */
	UCHAR	i, j, RateIdx = 12; /* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
	BOOLEAN	bMatch = FALSE;

	switch (pAd->CommonCfg.PhyMode) 
	{
		case PHY_11B:
			ProperMlmeRate = RATE_11;
			MinimumRate = RATE_1;
			break;
		case PHY_11BG_MIXED:
#ifdef DOT11_N_SUPPORT
		case PHY_11ABGN_MIXED:
		case PHY_11BGN_MIXED:
#endif /* DOT11_N_SUPPORT */
			if ((pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.SupRateLen == 4) &&
				(pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.ExtRateLen == 0))
				/* B only AP */
				ProperMlmeRate = RATE_11;
			else
				ProperMlmeRate = RATE_24;
			
			if (pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Channel <= 14)
			MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		case PHY_11A:
#ifdef DOT11_N_SUPPORT
		case PHY_11N_2_4G:	/* rt2860 need to check mlmerate for 802.11n */
		case PHY_11GN_MIXED:
		case PHY_11AGN_MIXED:
		case PHY_11AN_MIXED:
		case PHY_11N_5G:	
#endif /* DOT11_N_SUPPORT */
			ProperMlmeRate = RATE_24;
			MinimumRate = RATE_6;
			break;
		case PHY_11ABG_MIXED:
			ProperMlmeRate = RATE_24;
			if (pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.Channel <= 14)
			   MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		default: /* error */
			ProperMlmeRate = RATE_1;
			MinimumRate = RATE_1;
			break;
	}

	for (i = 0; i < pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.SupRateLen; i++)
	{
		for (j = 0; j < RateIdx; j++)
		{
			if ((pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
			{
				if (j == ProperMlmeRate)
				{
					bMatch = TRUE;
					break;
				}
			}			
		}

		if (bMatch)
			break;
	}

	if (bMatch == FALSE)
	{
		for (i = 0; i < pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.ExtRateLen; i++)
		{
			for (j = 0; j < RateIdx; j++)
			{
				if ((pAd->ApCfg.ApCliTab[ifIndex].ApCliMlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j])
				{
					if (j == ProperMlmeRate)
					{
						bMatch = TRUE;
						break;
					}
				}			
			}
		
			if (bMatch)
			break;		
		}
	}

	if (bMatch == FALSE)
	{
		ProperMlmeRate = MinimumRate;
	}

	if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{	
		pAd->CommonCfg.MlmeRate = MinimumRate;
		pAd->CommonCfg.RtsRate = ProperMlmeRate;
		if (pAd->CommonCfg.MlmeRate >= RATE_6)
		{
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
			pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}
		else
		{
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
			pAd->CommonCfg.MlmeTransmit.field.MCS = pAd->CommonCfg.MlmeRate;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_CCK;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = pAd->CommonCfg.MlmeRate;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPUpdateMlmeRate ==>   MlmeTransmit = 0x%x  \n" , pAd->CommonCfg.MlmeTransmit.word));
}


/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID P2PAPShutdown(
	IN PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("---> P2PAPShutdown\n"));

#ifdef RTMP_MAC_PCI
		APStop(pAd);
#endif /* RTMP_MAC_PCI */

	MlmeRadioOff(pAd);

#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(&pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

	NdisFreeSpinLock(&pAd->MacTabLock);

	DBGPRINT(RT_DEBUG_TRACE, ("<--- P2PAPShutdown\n"));
}

VOID P2PUserCfgInit(
	IN	PRTMP_ADAPTER pAd)
{
	UINT i, j;

	/* Set MBSS Default Configurations */
	pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
	for(j = BSS0; j < pAd->ApCfg.BssidNum; j++)
	{
		pAd->ApCfg.MBSSID[j].AuthMode = Ndis802_11AuthModeOpen;
		pAd->ApCfg.MBSSID[j].WepStatus = Ndis802_11EncryptionDisabled;
		pAd->ApCfg.MBSSID[j].GroupKeyWepStatus = Ndis802_11EncryptionDisabled;
		pAd->ApCfg.MBSSID[j].DefaultKeyId = 0;
		pAd->ApCfg.MBSSID[j].WpaMixPairCipher = MIX_CIPHER_NOTUSE;

#ifdef DOT1X_SUPPORT
		pAd->ApCfg.MBSSID[j].IEEE8021X = FALSE;
		pAd->ApCfg.MBSSID[j].PreAuth = FALSE;

		/* PMK cache setting */
		pAd->ApCfg.MBSSID[j].PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute) */
		NdisZeroMemory(&pAd->ApCfg.MBSSID[j].PMKIDCache, sizeof(NDIS_AP_802_11_PMKID));

		/* dot1x related per BSS */
		pAd->ApCfg.MBSSID[j].radius_srv_num = 0;
		pAd->ApCfg.MBSSID[j].NasIdLen = 0;
#endif /* DOT1X_SUPPORT */

		/* VLAN related */
		pAd->ApCfg.MBSSID[j].VLAN_VID = 0;

		/* Default MCS as AUTO */
		pAd->ApCfg.MBSSID[j].bAutoTxRateSwitch = TRUE;
		pAd->ApCfg.MBSSID[j].DesiredTransmitSetting.field.MCS = MCS_AUTO;

		/* Default is zero. It means no limit. */
		pAd->ApCfg.MBSSID[j].MaxStaNum = 0;
		pAd->ApCfg.MBSSID[j].StaCount = 0;
	
#ifdef WSC_AP_SUPPORT
		{
			PWSC_CTRL pWscControl;
			INT idx;
#ifdef WSC_V2_SUPPORT
			PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
			/*
				WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize 
				and MLME Initialize is called before UserCfgInit.
			*/
			pWscControl = &pAd->ApCfg.MBSSID[j].WscControl;
			NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
			NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
			pWscControl->WscMode = 1;
			pWscControl->WscConfStatus = 1;
			pWscControl->WscConfigMethods= 0x0184;
			pWscControl->RegData.ReComputePke = 1;
			pWscControl->lastId = 1;
			/*pWscControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | j); */
			pWscControl->pAd = pAd;
			pWscControl->WscRejectSamePinFromEnrollee = FALSE;
			pAd->CommonCfg.WscPBCOverlap = FALSE;
			pWscControl->WscConfMode = 0;
			pWscControl->WscStatus = 0;
			pWscControl->WscState = 0;
			pWscControl->WscPinCode = 0;
			pWscControl->WscLastPinFromEnrollee = 0;
			pWscControl->WscEnrollee4digitPinCode = FALSE;
			pWscControl->WscEnrolleePinCode = 0;
			pWscControl->WscSelReg = 0;
			pWscControl->WscUseUPnP = 0;
			pWscControl->bWCNTest = FALSE;
			pWscControl->WscKeyASCII = 0; /* default, 0 (64 Hex) */

			/*
				Enrollee 192 random bytes for DH key generation
			*/
			for (idx = 0; idx < 192; idx++)
				pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

			/* Enrollee Nonce, first generate and save to Wsc Control Block */
			for (idx = 0; idx < 16; idx++)
			{
				pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);
			}
			NdisZeroMemory(&pWscControl->WscDefaultSsid, sizeof(NDIS_802_11_SSID));
			NdisZeroMemory(&pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
			NdisZeroMemory(&pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
			pWscControl->bCheckMultiByte = FALSE;

#ifdef WSC_V2_SUPPORT
			pWscV2Info = &pWscControl->WscV2Info;
			pWscV2Info->bWpsEnable = TRUE;
			pWscV2Info->ExtraTlv.TlvLen = 0;
			pWscV2Info->ExtraTlv.TlvTag = 0;
			pWscV2Info->ExtraTlv.pTlvData = NULL;
			pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
			pWscV2Info->bEnableWpsV2 = TRUE;
#endif /* WSC_V2_SUPPORT */
		}
#endif /* WSC_AP_SUPPORT */


		for(i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
			pAd->ApCfg.MBSSID[j].TimBitmaps[i] = 0;
	}

	pAd->ApCfg.DtimCount  = 0;
	pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;

	pAd->ApCfg.ErpIeContent = 0;

	pAd->ApCfg.StaIdleTimeout = MAC_TABLE_AGEOUT_TIME;

#ifdef IDS_SUPPORT
	/* Default disable IDS threshold and reset all IDS counters */
	pAd->ApCfg.IdsEnable = FALSE;
	pAd->ApCfg.AuthFloodThreshold = 0;
	pAd->ApCfg.AssocReqFloodThreshold = 0;
	pAd->ApCfg.ReassocReqFloodThreshold = 0;
	pAd->ApCfg.ProbeReqFloodThreshold = 0;
	pAd->ApCfg.DisassocFloodThreshold = 0;
	pAd->ApCfg.DeauthFloodThreshold = 0;
	pAd->ApCfg.EapReqFloodThreshold = 0;
	RTMPClearAllIdsCounter(pAd);
#endif /* IDS_SUPPORT */

#ifdef WSC_INCLUDED
	pAd->WriteWscCfgToDatFile = 0xFF;
	pAd->WriteWscCfgToAr9DatFile = FALSE;
#ifdef CONFIG_AP_SUPPORT
	pAd->bWscDriverAutoUpdateCfg = TRUE;
#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
		/* Update PortCfg by WebUI */
		pAd->bWscDriverAutoUpdateCfg = FALSE;
#endif
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

	for(j = 0; j < MAX_APCLI_NUM; j++) 
	{
		pAd->ApCfg.ApCliTab[j].AuthMode = Ndis802_11AuthModeOpen;
		pAd->ApCfg.ApCliTab[j].WepStatus = Ndis802_11WEPDisabled;
		pAd->ApCfg.ApCliTab[j].bAutoTxRateSwitch = TRUE;
		pAd->ApCfg.ApCliTab[j].DesiredTransmitSetting.field.MCS = MCS_AUTO;
	}

	{
		PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

		pP2PCtrl->P2p_OpMode = P2P_CONCURRENT;

		pP2PCtrl->bKeepSlient = FALSE;
		pP2PCtrl->NoAIndex = MAX_P2P_GROUP_SIZE;
/*		pP2PCtrl->PortNumber = 0; */
		pP2PCtrl->ListenChannel = 1;
		pP2PCtrl->GroupChannel = 11;
		pP2PCtrl->GroupOpChannel = 11;
		P2pSetListenIntBias(pAd, 3);
		pP2PCtrl->DeviceNameLen = 10;
		pP2PCtrl->DeviceName[0] = 'R';
		pP2PCtrl->DeviceName[1] = 'a';
		pP2PCtrl->DeviceName[2] = 'l';
		pP2PCtrl->DeviceName[3] = 'i';
		pP2PCtrl->DeviceName[4] = 'n';
		pP2PCtrl->DeviceName[5] = 'k';
		pP2PCtrl->DeviceName[6] = '-';
		pP2PCtrl->DeviceName[7] = 'P';
		pP2PCtrl->DeviceName[8] = 0x32;
		pP2PCtrl->DeviceName[9] = 'P';

/*		pP2PCtrl->P2PDiscoProvState = P2P_DISABLE; */
		pP2PCtrl->P2PConnectState = P2P_CONNECT_IDLE;
		/* Set Dpid to "not specified". it means, GUI doesn't set for connection yet. */
		pP2PCtrl->Dpid = DEV_PASS_ID_NOSPEC;
		pP2PCtrl->P2pManagedParm.APP2pManageability = 0xff;
		pP2PCtrl->P2pManagedParm.ICSStatus = ICS_STATUS_DISABLED; 
		P2pGroupTabInit(pAd);
		P2pCrednTabClean(pAd);
		P2pScanChannelDefault(pAd);
		RTMPMoveMemory(pAd->P2pCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
/*		RTMPMoveMemory(pAd->P2pCfg.Bssid, pAd->P2pCfg.CurrentAddress, MAC_ADDR_LEN); */
		pP2PCtrl->SSIDLen = WILDP2PSSIDLEN;
		pP2PCtrl->GONoASchedule.bValid = FALSE;
		pP2PCtrl->GONoASchedule.bInAwake = TRUE;
		pP2PCtrl->GONoASchedule.bWMMPSInAbsent = FALSE; /* Set to FALSE if changes state to Awake */
		pP2PCtrl->GONoASchedule.Token = 0;
		pP2PCtrl->GoIntentIdx = 0;
		pP2PCtrl->Rule = P2P_IS_DEVICE;
		pP2PCtrl->WscMode = WSC_PIN_MODE;
		pP2PCtrl->DefaultConfigMethod = P2P_REG_CM_DISPLAY;
		pP2PCtrl->bExtListen = FALSE;
		pP2PCtrl->bIntraBss = FALSE;
		pP2PCtrl->ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
		pP2PCtrl->ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
		pP2PCtrl->GONoASchedule.Count = 0;
		pP2PCtrl->GONoASchedule.Duration = 0;
		pP2PCtrl->GONoASchedule.Interval = 0;
	}
}

VOID P2pInit(
	IN PRTMP_ADAPTER 				pAd,
	IN RTMP_OS_NETDEV_OP_HOOK		*pNetDevOps)
{
	PNET_DEV	new_dev_p;
	APCLI_STRUCT	*pApCliEntry;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_p2p_init != FALSE)
		return;
	/* End of if */

	DBGPRINT(RT_DEBUG_TRACE, ("%s --->\n", __FUNCTION__));

	/* init */
	pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = NULL;
	pAd->ApCfg.ApCliTab[MAIN_MBSSID].dev = NULL;
	pApCliEntry = &pAd->ApCfg.ApCliTab[MAIN_MBSSID];

    /* create virtual network interface */
	{
		UINT32 MC_RowID = 0, IoctlIF = 0;
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_P2P, MAIN_MBSSID, sizeof(PRTMP_ADAPTER), INF_P2P_DEV_NAME);

		if (new_dev_p == NULL)
		{
			/* allocation fail, exit */
			DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (MBSS)...\n"));
			return;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Register P2P IF (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
		}

		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);


		pNetDevOps->priv_flags = INT_P2P;			/* We are virtual interface */
		pNetDevOps->needProtcted = TRUE;

		/* Init MAC address of virtual network interface */
		COPY_MAC_ADDR(pAd->P2PCurrentAddress, pAd->CurrentAddress);
		/* 	
			Refer to HW definition - 
						Bit1 of MAC address Byte0 is local administration bit 
						and should be set to 1 in extended multiple BSSIDs'
						Bit3~ of MAC address Byte0 is extended multiple BSSID index.
		*/
		if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
		pAd->P2PCurrentAddress[0] += 2; 
		else
		{
#ifdef P2P_ODD_MAC_ADJUST
			if (pAd->P2PCurrentAddress[5] & 0x01 == 0x01)
				pAd->P2PCurrentAddress[5] -= 1;
			else
#endif /* P2P_ODD_MAC_ADJUST */
		pAd->P2PCurrentAddress[5] += FIRST_MBSSID;
		}
		NdisMoveMemory(&pNetDevOps->devAddr[0], &pAd->P2PCurrentAddress[0], MAC_ADDR_LEN);
		
		/* backup our virtual network interface */
		pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev = new_dev_p;
		COPY_MAC_ADDR(pAd->ApCfg.MBSSID[MAIN_MBSSID].Bssid, pAd->P2PCurrentAddress);

		pApCliEntry->dev = new_dev_p;
		COPY_MAC_ADDR(pApCliEntry->CurrentAddress, pAd->P2PCurrentAddress);

		COPY_MAC_ADDR(pAd->P2pCfg.CurrentAddress, pAd->P2PCurrentAddress);
		pAd->p2p_dev = new_dev_p;
		
		/* register this device to OS */
		RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	}

	pAd->CommonCfg.BeaconPeriod = 100;
	pAd->ApCfg.DtimPeriod = 1;
	pAd->CommonCfg.DisableOLBCDetect = 0;

	P2pGetRandomSSID(pAd, pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, &(pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen));
	pAd->ApCfg.MBSSID[MAIN_MBSSID].AuthMode = Ndis802_11AuthModeOpen;
	pAd->ApCfg.MBSSID[MAIN_MBSSID].WepStatus = Ndis802_11EncryptionDisabled;
	pAd->ApCfg.MBSSID[MAIN_MBSSID].DesiredTransmitSetting.field.MCS = pAd->StaCfg.DesiredTransmitSetting.field.MCS;

	if ((pAd->CommonCfg.bWmmCapable) || (pAd->CommonCfg.PhyMode > PHY_11G))
		pAd->ApCfg.MBSSID[MAIN_MBSSID].bWmmCapable = TRUE;

	/*OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_P2P_GO); */
	/*OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_P2P_CLI); */

	pAd->flg_p2p_init = TRUE;
	pAd->ApCfg.ApCliTab[MAIN_MBSSID].AuthMode = Ndis802_11AuthModeOpen;
	pAd->ApCfg.ApCliTab[MAIN_MBSSID].WepStatus = Ndis802_11WEPDisabled;
	pAd->ApCfg.ApCliTab[MAIN_MBSSID].DesiredTransmitSetting.field.MCS = pAd->StaCfg.DesiredTransmitSetting.field.MCS;
	RTMPSetIndividualHT(pAd, MIN_NET_DEVICE_FOR_APCLI);
	pAd->flg_apcli_init = TRUE;

	pAd->flg_p2p_OpStatusFlags = P2P_DISABLE;

}

INT P2P_OpenPre(
	IN	PNET_DEV					pDev)
{
	PRTMP_ADAPTER pAd;


	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	if (ADHOC_ON(pAd))
		return -1;

	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = FALSE;
	return 0;
}

INT P2P_OpenPost(
	IN	PNET_DEV					pDev)
{
	PRTMP_ADAPTER pAd;
	PMULTISSID_STRUCT	pMbss;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);

	if (ADHOC_ON(pAd))
		return -1;
	
	pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	/* re-copy the MAC to virtual interface to avoid these MAC = all zero, 
		when re-open the ra0,
		i.e. ifconfig ra0 down, ifconfig ra0 up, ifconfig ra0 down, ifconfig up ... */
	
	COPY_MAC_ADDR(pMbss->Bssid, pAd->CurrentAddress);
	
	/*	
		Refer to HW definition - 
					Bit1 of MAC address Byte0 is local administration bit 
					and should be set to 1 in extended multiple BSSIDs'
					Bit3~ of MAC address Byte0 is extended multiple BSSID index.
	*/
	if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
	pMbss->Bssid[MAIN_MBSSID] += 2; 	
	else
	{
#ifdef P2P_ODD_MAC_ADJUST
		if ((pMbss->Bssid[5] & 0x01) == 0x01)
			pMbss->Bssid[5] -= 1;
		else
#endif /* P2P_ODD_MAC_ADJUST */
	pMbss->Bssid[5] += FIRST_MBSSID;
	}
	if (pMbss->MSSIDDev != NULL)
	{
		NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(pMbss->MSSIDDev), 
							pMbss->Bssid,
							MAC_ADDR_LEN);
	}

	P2PCfgInit(pAd);
	/*P2P_GoStartUp(pAd, MAIN_MBSSID); */
	/*P2P_CliStartUp(pAd); */
	
	/* P2P Enable */
	P2pEnable(pAd);

	NdisAllocateSpinLock(pAd, &pAd->P2pTableSemLock);

	return 0;
}

INT P2P_Close(
	IN	PNET_DEV					pDev)
{
	PRTMP_ADAPTER pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);

	if (P2P_CLI_ON(pAd))
		P2P_CliStop(pAd);
	else if (P2P_GO_ON(pAd))
		P2P_GoStop(pAd);

	P2pGroupTabDisconnect(pAd, FALSE);
	{
		UINT32 Value;
		/* Disable pre-tbtt interrupt */
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
		Value &=0xe;
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	}

	/*update beacon Sync */
	/*if rausb0 is up => stop beacon */
	/*if rausb0 is down => we will call AsicDisableSync() in usb_rtusb_close_device() */
	if (INFRA_ON(pAd))
		AsicEnableBssSync(pAd);
	else if (ADHOC_ON(pAd))
		AsicEnableIbssSync(pAd);
	else
		AsicDisableSync(pAd);

	NdisFreeSpinLock(&pAd->P2pTableSemLock);
	return 0;
}

VOID P2P_Remove(
	IN PRTMP_ADAPTER				pAd)
{
	MULTISSID_STRUCT *pMbss;


	if (pAd->p2p_dev)
	{
		RtmpOSNetDevDetach(pAd->p2p_dev);
		RtmpOSNetDevFree(pAd->p2p_dev);

		/* clear it as NULL to prevent latter access error */
		pAd->p2p_dev = NULL;
		pAd->flg_p2p_init = FALSE;

		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		pMbss->MSSIDDev = NULL;

		pAd->ApCfg.ApCliTab[MAIN_MBSSID].dev = NULL;
		pAd->flg_apcli_init = FALSE;
	}

	
} /* End of P2P_Remove */

int P2P_PacketSend(
	IN	PNDIS_PACKET				pPktSrc, 
	IN	PNET_DEV					pDev,
	IN	RTMP_NET_PACKET_TRANSMIT	Func)
{
	PRTMP_ADAPTER pAd;
	PAPCLI_STRUCT pApCli;
	

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);
	
#ifdef RALINK_ATE
		if (ATE_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
			return 0;
		}
#endif /* RALINK_ATE */
		if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))			||
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
		{
			/* wlan is scanning/disabled/reset */
			RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
			/*printk("1. P2P_VirtualIF_PacketSend @@@@@\n"); */
			return 0;
		}
	
		if(!(RTMP_OS_NETDEV_STATE_RUNNING(pDev)))
		{
			/* the interface is down */
			RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
			/*printk("2. P2P_VirtualIF_PacketSend @@@@@\n"); */
			return 0;
		}
	
		if (P2P_CLI_ON(pAd))
		{
			pApCli = (PAPCLI_STRUCT)&pAd->ApCfg.ApCliTab;
	
			if (pApCli[MAIN_MBSSID].Valid != TRUE)
			{
				RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
				/*printk("3. P2P_VirtualIF_PacketSend @@@@@\n"); */
				return 0;
			}
	
			/* find the device in our ApCli list */
			if (pApCli[MAIN_MBSSID].dev == pDev)
			{
				/* ya! find it */
				pAd->RalinkCounters.PendingNdisPacketCount ++;
				/*NdisZeroMemory((PUCHAR)&(RTPKT_TO_OSPKT(pPktSrc))->cb[CB_OFF], 15);*/
				NdisZeroMemory((PUCHAR)(GET_OS_PKT_CB(pPktSrc) + CB_OFF), 15);
				RTMP_SET_PACKET_MOREDATA(pPktSrc, FALSE);
				RTMP_SET_PACKET_NET_DEVICE_APCLI(pPktSrc, MAIN_MBSSID);
				SET_OS_PKT_NETDEV(pPktSrc, pAd->net_dev);
				RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
	
	
				/* transmit the packet */
				/*return rt28xx_packet_xmit(RTPKT_TO_OSPKT(skb_p)); */
				return Func(RTPKT_TO_OSPKT(pPktSrc));
			}
		}
		else
		{
			/*printk("4. P2P_VirtualIF_PacketSend @@@@@\n"); */
			/* find the device in our p2p list */
			if (pAd->ApCfg.MBSSID[MAIN_MBSSID].MSSIDDev == pDev)
			{
				/* ya! find it */
				pAd->RalinkCounters.PendingNdisPacketCount ++;
				/*NdisZeroMemory((PUCHAR)&(RTPKT_TO_OSPKT(pPktSrc))->cb[CB_OFF], 15);*/
				NdisZeroMemory((PUCHAR)(GET_OS_PKT_CB(pPktSrc) + CB_OFF), 15);
				RTMP_SET_PACKET_MOREDATA(pPktSrc, FALSE);
				RTMP_SET_PACKET_NET_DEVICE_P2P(pPktSrc, MAIN_MBSSID);
				SET_OS_PKT_NETDEV(pPktSrc, pAd->net_dev);
				RTMP_SET_PACKET_OPMODE(pPktSrc, OPMODE_AP);
	
	
				/* transmit the packet */
				/*return rt28xx_packet_xmit(RTPKT_TO_OSPKT(skb_p));*/
				return Func(RTPKT_TO_OSPKT(pPktSrc));
			}
		}
	
		RELEASE_NDIS_PACKET(pAd, pPktSrc, NDIS_STATUS_FAILURE);
	
		return 0;
}

INT	P2P_GoSetCommonHT(
	IN	PRTMP_ADAPTER	pAd)
{
	OID_SET_HT_PHYMODE	SetHT;
	
	if (pAd->CommonCfg.PhyMode < PHY_11ABGN_MIXED)
	{
		/* Clear previous HT information */
		RTMPDisableDesiredHtInfo(pAd);
		return FALSE;
	}

	SetHT.PhyMode = (RT_802_11_PHY_MODE)pAd->CommonCfg.PhyMode;
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.ExtOffset = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
	SetHT.MCS = MCS_AUTO;
	SetHT.BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
	SetHT.STBC = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
	SetHT.SHORTGI = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI; 

	if (INFRA_ON(pAd))
	{
		if (pAd->StaActive.SupportedHtPhy.ChannelWidth == BW_40)
			SetHT.ExtOffset = (UCHAR)pAd->StaActive.SupportedHtPhy.ExtChanOffset;
	}

	RTMPSetHT(pAd, &SetHT);

	return TRUE;
}

