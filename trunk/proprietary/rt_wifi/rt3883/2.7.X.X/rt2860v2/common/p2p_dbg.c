#ifdef P2P_SUPPORT


#include "rt_config.h"

extern UCHAR	WILDP2PSSID[];
extern UCHAR	WILDP2PSSIDLEN;

extern INT Set_AP_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_P2P_Print_Cfg(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	if (P2P_INF_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("P2P Device Config :\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("=====================================================================\n"));
		DBGPRINT(RT_DEBUG_ERROR,("Device Name[%ld] = %s.\n", pP2PCtrl->DeviceNameLen, pP2PCtrl->DeviceName));
		DBGPRINT(RT_DEBUG_ERROR,("Device Addr = %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pP2PCtrl->CurrentAddress)));
		DBGPRINT(RT_DEBUG_ERROR, ("OpChannel = %d.        Listen Channel = %d. \n", pP2PCtrl->GroupChannel, pP2PCtrl->ListenChannel));
		DBGPRINT(RT_DEBUG_ERROR, ("My Go Intent = %d.\n", pP2PCtrl->GoIntentIdx));
		/*DBGPRINT(RT_DEBUG_ERROR, ("WscMode = %s.        ConfigMethod = %s.\n", (pP2PCtrl->WscMode == 1) ? "PIN" : "PBC", decodeConfigMethod(pP2PCtrl->ConfigMethod))); */
		if (pP2PCtrl->WscMode == 1)
			DBGPRINT(RT_DEBUG_ERROR, ("WscMode = PIN.\n"));
		else if (pP2PCtrl->WscMode == 2)
			DBGPRINT(RT_DEBUG_ERROR, ("WscMode = PBC.\n"));
		else
			DBGPRINT(RT_DEBUG_ERROR, ("WscMode = ***Unknown***.\n"));

		DBGPRINT(RT_DEBUG_ERROR, ("WscConfigMethod = %s.\n", decodeConfigMethod(pP2PCtrl->ConfigMethod)));
		DBGPRINT(RT_DEBUG_ERROR, ("WscDpid = %s.\n", decodeDpid(pP2PCtrl->Dpid)));
		if (pP2PCtrl->ConfigMethod == WSC_CONFMET_DISPLAY)
			DBGPRINT(RT_DEBUG_ERROR, ("My Self PIN Code = %08u\n", pAd->ApCfg.ApCliTab[0].WscControl.WscEnrolleePinCode));
		else if (pP2PCtrl->ConfigMethod == WSC_CONFMET_KEYPAD)
		{
			UINT PinCode = simple_strtol(pP2PCtrl->PinCode, 0, 10);
			DBGPRINT(RT_DEBUG_ERROR, ("Peer PIN Code = %08u\n", PinCode));
		}

		DBGPRINT(RT_DEBUG_ERROR, ("SSID[%d] = %s.\n", pP2PCtrl->SSIDLen, pP2PCtrl->SSID));
		DBGPRINT(RT_DEBUG_ERROR, ("NoA_Count = %d.        NoA_Duration = %ld.        NoA_Interval = %ld.        StartTime = %ld.\n", 
									pP2PCtrl->GONoASchedule.Count, pP2PCtrl->GONoASchedule.Duration, pP2PCtrl->GONoASchedule.Interval, pP2PCtrl->GONoASchedule.StartTime));
		DBGPRINT(RT_DEBUG_INFO, ("ExtListenPeriod = %d.        ExtListenInterval = %d.\n", pP2PCtrl->ExtListenPeriod, pP2PCtrl->ExtListenInterval));
		DBGPRINT(RT_DEBUG_INFO, ("Intra-Bss = %d.        \n", pP2PCtrl->bIntraBss));
		DBGPRINT(RT_DEBUG_ERROR, ("ConenctMAC = %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pP2PCtrl->ConnectingMAC)));
		DBGPRINT(RT_DEBUG_ERROR, ("p2pControl = %u8.    Persistent = %s. Invite = %s.    ClientDiscovery = %s.    IntraBss = %s.    ExtListen = %s.\n", pP2PCtrl->P2pControl.word, (IS_PERSISTENT_ON(pAd))? "ON" : "OFF", (IS_INVITE_ON(pAd))? "ON" : "OFF",
					(IS_CLIENT_DISCOVERY_ON(pAd))? "ON" : "OFF", (IS_INTRA_BSS_ON(pAd))? "ON" : "OFF", (IS_EXT_LISTEN_ON(pAd))? "ON" : "OFF"));
		DBGPRINT(RT_DEBUG_ERROR, ("                         Opps = %s.    SwNoATimer = %s.\n", (IS_OPPS_ON(pAd))? "ON" : "OFF", (IS_SW_NOA_TIMER(pAd))? "ON" : "OFF"));

	}

	return TRUE;
}

INT Set_P2P_Enable(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	UINT32 enable = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	enable = (UCHAR) simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Enable = %d.\n", __FUNCTION__, enable));
	return TRUE;
}

INT Set_P2P_Listen_Channel(
		IN	PRTMP_ADAPTER	pAd, 
		IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	UINT32 channel;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	channel = (UCHAR) simple_strtol(arg, 0, 10);
	/* check if this channel is valid */
	if (ChannelSanity(pAd, channel) == TRUE)
	{
		pAd->P2pCfg.ListenChannel = channel;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Listen Channel out of range, using default.\n"));
		pAd->P2pCfg.ListenChannel = 1;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Listen Channel = %d.\n", __FUNCTION__, pAd->P2pCfg.ListenChannel));
	return TRUE;
}

INT Set_P2P_Operation_Channel(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	UINT32 channel;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	channel = (UCHAR) simple_strtol(arg, 0, 10);
	/* check if this channel is valid */
	if (ChannelSanity(pAd, channel) == TRUE)
	{
		pAd->P2pCfg.GroupChannel = channel;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Opertation Channel out of range, using default.\n"));
		pAd->P2pCfg.GroupChannel = 1;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Op Channel = %d.\n", __FUNCTION__, pAd->P2pCfg.GroupChannel));
	return TRUE;
}


INT Set_P2P_GO_Intent(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	UINT32	intent;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	intent = simple_strtol(arg, 0, 10);
	if (intent <= 15)
		pAd->P2pCfg.GoIntentIdx = intent;		
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("GO Intent out of range 0 ~ 15, using default.\n"));
		pAd->P2pCfg.GoIntentIdx = 0;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: GO Intent = %d.\n", __FUNCTION__, pAd->P2pCfg.GoIntentIdx));
	return TRUE;
}


INT Set_P2P_Device_Name(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	if (strlen(arg) <= 32)
	{
		pAd->P2pCfg.DeviceNameLen = (UCHAR) strlen(arg);
		NdisZeroMemory(pAd->P2pCfg.DeviceName, 32);
		NdisMoveMemory(pAd->P2pCfg.DeviceName, arg, pAd->P2pCfg.DeviceNameLen);
		DBGPRINT(RT_DEBUG_ERROR, ("Set P2P Device Name - %s", pAd->P2pCfg.DeviceName));
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: Device Name = %s.\n", __FUNCTION__, pAd->P2pCfg.DeviceName));
	return TRUE;
}

INT Set_P2P_WSC_Mode(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	int wscMode;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	wscMode = simple_strtol(arg, 0, 10);
	if (wscMode <= 2 && wscMode >= 1)
		pAd->P2pCfg.WscMode= wscMode;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("WscMode is invalid, using default and accept wsc method from peer's provision request action frame.\n"));
		pAd->P2pCfg.WscMode = WSC_PIN_MODE; /* PIN */
		pAd->P2pCfg.Dpid = DEV_PASS_ID_NOSPEC;
		pAd->P2pCfg.ConfigMethod = (WSC_CONFMET_PBC | WSC_CONFMET_KEYPAD | WSC_CONFMET_DISPLAY);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: WscMode = %s.\n", __FUNCTION__, (wscMode == 1) ? "PIN" : "PBC"));
	return TRUE;
}

INT Set_P2P_WSC_ConfMethod(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	int method;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	method = simple_strtol(arg, 0, 10);

	if (pAd->P2pCfg.WscMode == WSC_PIN_MODE)
	{
		if (method == 1)
		{
			/* Display PIN */
			pAd->P2pCfg.Dpid = DEV_PASS_ID_REG;
			pAd->P2pCfg.ConfigMethod =  WSC_CONFMET_DISPLAY;
			DBGPRINT(RT_DEBUG_TRACE, ("    *************************************************\n"));
			DBGPRINT(RT_DEBUG_TRACE, ("    *                                               *\n"));
			DBGPRINT(RT_DEBUG_TRACE, ("    *       PIN Code = %08u                     *\n", pAd->ApCfg.ApCliTab[0].WscControl.WscEnrolleePinCode));
			DBGPRINT(RT_DEBUG_TRACE, ("    *                                               *\n"));
			DBGPRINT(RT_DEBUG_TRACE, ("    *************************************************\n"));

		}
		else if (method == 2)
		{
			/* Enter PIN */
			pAd->P2pCfg.Dpid = DEV_PASS_ID_USER;
			pAd->P2pCfg.ConfigMethod =  WSC_CONFMET_KEYPAD;
		}
	}
	else if (pAd->P2pCfg.WscMode == WSC_PBC_MODE)
	{
		if (method == 3)
		{
			pAd->P2pCfg.Dpid = DEV_PASS_ID_PBC;
			pAd->P2pCfg.ConfigMethod = WSC_CONFMET_PBC;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Config Method = %s.\n", __FUNCTION__, decodeConfigMethod(pAd->P2pCfg.ConfigMethod)));
	return TRUE;
}

INT Set_P2P_NoA_Count(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	pAd->P2pCfg.GONoASchedule.Count = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: NoA Count = %d\n", __FUNCTION__, pAd->P2pCfg.GONoASchedule.Count));
	return TRUE;
}

INT Set_P2P_NoA_Duration(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	pAd->P2pCfg.GONoASchedule.Duration = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: NoA Duration = %ld\n", __FUNCTION__, pAd->P2pCfg.GONoASchedule.Duration));
	return TRUE;
}

INT Set_P2P_NoA_Interval(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	pAd->P2pCfg.GONoASchedule.Interval = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: NoA Interval = %ld\n", __FUNCTION__, pAd->P2pCfg.GONoASchedule.Interval));
	return TRUE;
}

INT Set_P2P_Extend_Listen(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	UCHAR ExtListen;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	ExtListen = simple_strtol(arg, 0, 10);
	if (ExtListen)
		pAd->P2pCfg.bExtListen = TRUE;
	else
		pAd->P2pCfg.bExtListen = FALSE;
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Exted Listen = %d\n", __FUNCTION__, pAd->P2pCfg.bExtListen));
	return TRUE;
}

INT Set_P2P_Extend_Listen_Periodic(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	pAd->P2pCfg.ExtListenPeriod = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Extend Listen Interval = %d\n", __FUNCTION__, pAd->P2pCfg.ExtListenPeriod));
	return TRUE;
}

INT Set_P2P_Extend_Listen_Interval(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	pAd->P2pCfg.ExtListenInterval = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Extend Listen Interval = %d\n", __FUNCTION__, pAd->P2pCfg.ExtListenInterval));
	return TRUE;
}

INT Set_P2P_Intra_Bss(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	UCHAR IntraBss;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	IntraBss = simple_strtol(arg, 0, 10);
	if (IntraBss)
		pAd->P2pCfg.bIntraBss = TRUE;
	else
		pAd->P2pCfg.bIntraBss = FALSE;
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: IntraBss = %d\n", __FUNCTION__, pAd->P2pCfg.bIntraBss));
	return TRUE;
}

INT Set_P2P_Scan(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	POS_COOKIE			pObj;
	int bScan;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	bScan = simple_strtol(arg, 0, 10);
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;


	if (bScan)
	{
		pAd->StaCfg.bAutoReconnect = FALSE;
		pP2PCtrl->bSentProbeRSP = TRUE;
		P2pGroupTabInit(pAd);
		P2pScan(pAd);
	}
	else
	{
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
		pAd->StaCfg.bAutoReconnect = FALSE;
#else
		pAd->StaCfg.bAutoReconnect = TRUE;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
		pP2PCtrl->bSentProbeRSP = FALSE;
		P2pStopScan(pAd);
	}
	return TRUE;
}

INT Set_P2P_Print_GroupTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	int i, j;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	RTMP_SEM_LOCK(&pAd->P2pTableSemLock);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2pTable ClientNum = %d\n", __FUNCTION__, pAd->P2pTable.ClientNumber));
	for (i=0; i < pAd->P2pTable.ClientNumber; i++)
	{
		PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAd->P2pTable.Client[i];
		DBGPRINT(RT_DEBUG_ERROR, ("Table.Client[%d]: DeviceName[%d][%s]\n", i, pP2pEntry->DeviceNameLen, pP2pEntry->DeviceName));
		DBGPRINT(RT_DEBUG_ERROR, ("                         Addr[%02x:%02x:%02x:%02x:%02x:%02x]\n", PRINT_MAC(pP2pEntry->addr)));
		DBGPRINT(RT_DEBUG_ERROR, ("                         BSSID[%02x:%02x:%02x:%02x:%02x:%02x]\n", PRINT_MAC(pP2pEntry->bssid)));
		DBGPRINT(RT_DEBUG_ERROR, ("                         InterfaceAddr[%02x:%02x:%02x:%02x:%02x:%02x]\n", PRINT_MAC(pP2pEntry->InterfaceAddr)));
		DBGPRINT(RT_DEBUG_ERROR, ("                         SSID["));
		for (j=0; j<pP2pEntry->SsidLen;j++)
			DBGPRINT(RT_DEBUG_ERROR, ("%c ", pP2pEntry->Ssid[j]));
		DBGPRINT(RT_DEBUG_ERROR, ("]\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("                         OpChannel = %d.        ListenChannel = %d.\n", pP2pEntry->OpChannel, pP2pEntry->ListenChannel));
		DBGPRINT(RT_DEBUG_ERROR, ("                         P2pClientState = %s.        MyGOIndex = %d.\n", decodeP2PClientState(pP2pEntry->P2pClientState), pP2pEntry->MyGOIndex));
		DBGPRINT(RT_DEBUG_ERROR, ("                         Dpid = %s.        Rule = %s.\n", decodeDpid(pP2pEntry->Dpid), decodeMyRule(pP2pEntry->Rule)));

		if (pP2pEntry->WscMode == 1)
			DBGPRINT(RT_DEBUG_ERROR, ("                         WscMode = PIN.        PIN = %02x %02x %02x %02x %02x %02x %02x %02x.\n", 
					pP2pEntry->PIN[0], pP2pEntry->PIN[1], pP2pEntry->PIN[2], pP2pEntry->PIN[3], 
					pP2pEntry->PIN[4], pP2pEntry->PIN[5], pP2pEntry->PIN[6], pP2pEntry->PIN[7]));
		else if (pAd->P2pTable.Client[i].WscMode == 2)
			DBGPRINT(RT_DEBUG_ERROR, ("                         WscMode = PBC.\n"));
		else
			DBGPRINT(RT_DEBUG_ERROR, ("                         WscMode = ***Unknown***.\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("                         CfgMethod = %s.        GoIntent = %d.\n", decodeConfigMethod(pAd->P2pTable.Client[i].ConfigMethod), pAd->P2pTable.Client[i].GoIntent));
		decodeDeviceCap(pP2pEntry->DevCapability);
		decodeGroupCap(pP2pEntry->GroupCapability);
		DBGPRINT(RT_DEBUG_ERROR, ("                         Rssi = %d.\n", pP2pEntry->Rssi));
		DBGPRINT(RT_DEBUG_ERROR, ("\n"));
    }

	RTMP_SEM_UNLOCK(&pAd->P2pTableSemLock);
	return TRUE;
}

INT Set_P2P_Print_PersistentTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	int i;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2pTable ClientNum = %d\n", __FUNCTION__, pAd->P2pTable.PerstNumber));
	for (i=0; i < pAd->P2pTable.PerstNumber; i++)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Table.PerstEntry[%d]: Rule = %s\n", i, decodeMyRule(pAd->P2pTable.PerstEntry[i].MyRule)));
		DBGPRINT(RT_DEBUG_ERROR, ("                         DevAddr = %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pAd->P2pTable.PerstEntry[i].Addr)));
		DBGPRINT(RT_DEBUG_ERROR, ("                         SSID[%d] = %s.\n", pAd->P2pTable.PerstEntry[i].Profile.SSID.SsidLength, pAd->P2pTable.PerstEntry[i].Profile.SSID.Ssid));
		DBGPRINT(RT_DEBUG_ERROR, ("                         MACAddr = %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pAd->P2pTable.PerstEntry[i].Profile.MacAddr)));
		DBGPRINT(RT_DEBUG_ERROR, ("                         Key[%d] = %02x %02x %02x %02x  %02x %02x %02x %02x.\n", pAd->P2pTable.PerstEntry[i].Profile.KeyIndex, pAd->P2pTable.PerstEntry[i].Profile.Key[0], pAd->P2pTable.PerstEntry[i].Profile.Key[1],
								pAd->P2pTable.PerstEntry[i].Profile.Key[2], pAd->P2pTable.PerstEntry[i].Profile.Key[3], pAd->P2pTable.PerstEntry[i].Profile.Key[4], pAd->P2pTable.PerstEntry[i].Profile.Key[5],
								pAd->P2pTable.PerstEntry[i].Profile.Key[6], pAd->P2pTable.PerstEntry[i].Profile.Key[7]));
		DBGPRINT(RT_DEBUG_ERROR, ("                                  %02x %02x %02x %02x  %02x %02x %02x %02x.\n", pAd->P2pTable.PerstEntry[i].Profile.Key[8], pAd->P2pTable.PerstEntry[i].Profile.Key[9],
								pAd->P2pTable.PerstEntry[i].Profile.Key[10], pAd->P2pTable.PerstEntry[i].Profile.Key[11], pAd->P2pTable.PerstEntry[i].Profile.Key[12], pAd->P2pTable.PerstEntry[i].Profile.Key[13],
								pAd->P2pTable.PerstEntry[i].Profile.Key[14], pAd->P2pTable.PerstEntry[i].Profile.Key[15]));
	}
	return TRUE;
}

INT Set_P2P_Provision_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	UCHAR p2pindex;
	PUCHAR	pAddr;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	p2pindex = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_ERROR, ("%s: TabIdx[%d]\n", __FUNCTION__, p2pindex));
	if (p2pindex < pAd->P2pTable.ClientNumber)
	{
/*		P2PPrintP2PEntry(pAd, P2pTabIdx); */
		pAddr = &pAd->P2pTable.Client[p2pindex].addr[0];
/*		pAd->P2pTable.Client[P2pTabIdx].StateCount = 10; */
/*		pAd->P2pTable.Client[P2pTabIdx].bValid = TRUE; */
/*		P2pProvision(pAd, pAddr); */
		P2pConnectPrepare(pAd, pAddr, P2PSTATE_PROVISION_COMMAND);
    }
    else
        DBGPRINT(RT_DEBUG_ERROR, ("Table Idx out of range!\n"));

	return TRUE;

}

INT Set_P2P_Invite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	UCHAR		p2pindex;
	PUCHAR	pAddr;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	p2pindex = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: TabIdx[%d]\n", __FUNCTION__, p2pindex));

	if (p2pindex < pAd->P2pTable.ClientNumber)
	{
		pAddr = &pAd->P2pTable.Client[p2pindex].addr[0];
		P2pConnectPrepare(pAd, pAddr, P2PSTATE_INVITE_COMMAND);
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("Table Idx out of range!\n"));

	return TRUE;
}

INT Set_P2P_Device_Discoverability_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	UCHAR		p2pindex;
	UCHAR		MyGOIdx;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	p2pindex = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: TabIdx[%d]\n", __FUNCTION__, p2pindex));
	P2PPrintP2PEntry(pAd, p2pindex);
	if (p2pindex < MAX_P2P_GROUP_SIZE)
	{
		MyGOIdx = pAd->P2pTable.Client[p2pindex].MyGOIndex;
		if (MyGOIdx != P2P_NOT_FOUND)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Start P2P Device Discoverability = %02x:%02x:%02x:%02x:%02x:%02x.	\n",  PRINT_MAC(pAd->P2pTable.Client[p2pindex].addr)));
			pAd->P2pTable.Client[p2pindex].GeneralToken++;
				pAd->P2pTable.Client[MyGOIdx].P2pClientState = P2PSTATE_GO_DISCO_COMMAND;
				pAd->P2pTable.Client[p2pindex].P2pClientState = P2PSTATE_CLIENT_DISCO_COMMAND;
			P2pSetListenIntBias(pAd, 12);
		}
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("Table Idx out of range!\n"));

	return TRUE;
}

INT Set_P2P_Connect_GoIndex_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	UCHAR		p2pindex;
	PUCHAR	pAddr;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	p2pindex = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: TabIdx[%d]\n", __FUNCTION__, p2pindex));


	if (p2pindex < pAd->P2pTable.ClientNumber)
	{
		/*
		P2PPrintP2PEntry(pAd, P2pTabIdx);
		pAd->P2pCfg.ConnectingIndex = 0;
		if (pAd->P2pTable.Client[P2pTabIdx].P2pClientState == P2PSTATE_DISCOVERY)
			pAd->P2pTable.Client[P2pTabIdx].P2pClientState = P2PSTATE_CONNECT_COMMAND;
		COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pAd->P2pTable.Client[P2pTabIdx].addr);
		pAd->P2pTable.Client[P2pTabIdx].StateCount = 10;
		pAd->P2pTable.Client[P2pTabIdx].bValid = TRUE;
		P2pConnect(pAd);
		 */
		pAddr = &pAd->P2pTable.Client[p2pindex].addr[0];
		P2pConnectPrepare(pAd, pAddr, P2PSTATE_CONNECT_COMMAND);
    }
    else
        DBGPRINT(RT_DEBUG_ERROR, ("Table Idx out of range!\n"));

	return TRUE;
}


INT Set_P2P_Connect_Dev_Addr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	UCHAR		p2pindex = P2P_NOT_FOUND;
	PUCHAR	pAddr;
	UCHAR ConnAddr[6];
	UINT32	i;
	extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
	extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::  Connect to DevAddr[%s]\n", __FUNCTION__, arg));


	/*
		If the input is the zero mac address, it means use our default(from EEPROM) MAC address as out-going 
		   MAC address.
		If the input is the broadcast MAC address, it means use the source MAC of first packet forwarded by
		   our device as the out-going MAC address.
		If the input is any other specific valid MAC address, use it as the out-going MAC address.
	*/

	NdisMoveMemory(&ConnAddr[0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN);
	if (rtstrmactohex(arg, (PSTRING) &ConnAddr[0]) == FALSE)
		return FALSE;

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: DevMac = %02x:%02x:%02x:%02x:%02x:%02x\n",	__FUNCTION__, PRINT_MAC(ConnAddr)));
	
	if (MAC_ADDR_EQUAL(ConnAddr, ZERO_MAC_ADDR))
	{
		P2pLinkDown(pAd, P2P_CONNECT_FAIL);
		return TRUE;
	}

	for (i=0; i < pAd->P2pTable.ClientNumber; i++)
	{
		if (MAC_ADDR_EQUAL(pAd->P2pTable.Client[i].addr, ConnAddr) ||
			MAC_ADDR_EQUAL(pAd->P2pTable.Client[i].bssid, ConnAddr) ||
			MAC_ADDR_EQUAL(pAd->P2pTable.Client[i].InterfaceAddr, ConnAddr))
		{
			p2pindex = i;
			break;
		}
	}

	if ((p2pindex < pAd->P2pTable.ClientNumber) && (p2pindex != P2P_NOT_FOUND))
	{
		pAddr = &pAd->P2pTable.Client[p2pindex].addr[0];
		P2pConnectPrepare(pAd, pAddr, P2PSTATE_CONNECT_COMMAND);
    }
    else
        DBGPRINT(RT_DEBUG_ERROR, ("Table Idx out of range!\n"));

	return TRUE;
}

INT Set_P2P_Provision_Dev_Addr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	UCHAR		p2pindex = P2P_NOT_FOUND;
	PUCHAR	pAddr;
	UCHAR ConnAddr[6];
	UINT32	i;
	UINT16 retry_cnt = 0;
	extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
	extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	DBGPRINT(RT_DEBUG_ERROR, ("%s::  Connect to DevAddr[%s]\n", __FUNCTION__, arg));


	/*
		If the input is the zero mac address, it means use our default(from EEPROM) MAC address as out-going 
		   MAC address.
		If the input is the broadcast MAC address, it means use the source MAC of first packet forwarded by
		   our device as the out-going MAC address.
		If the input is any other specific valid MAC address, use it as the out-going MAC address.
	*/

	if( pAd->P2pTable.ClientNumber == 0)
	{
		DBGPRINT( RT_DEBUG_ERROR, ("P2P Table is Empty! Scan First!\n"));
		P2pScan(pAd);
		OS_WAIT(2000);
	}

	NdisMoveMemory(&ConnAddr[0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN);
	if (rtstrmactohex(arg, (PSTRING) &ConnAddr[0]) == FALSE)
		return FALSE;

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: DevMac = %02x:%02x:%02x:%02x:%02x:%02x\n",	__FUNCTION__, PRINT_MAC(ConnAddr)));
	
	if (MAC_ADDR_EQUAL(ConnAddr, ZERO_MAC_ADDR))
	{
		P2pLinkDown(pAd, P2P_CONNECT_FAIL);
		return TRUE;
	}

retry:
	
	for (i=0; i < pAd->P2pTable.ClientNumber; i++)
	{
		if (MAC_ADDR_EQUAL(pAd->P2pTable.Client[i].addr, ConnAddr) ||
			MAC_ADDR_EQUAL(pAd->P2pTable.Client[i].bssid, ConnAddr) ||
			MAC_ADDR_EQUAL(pAd->P2pTable.Client[i].InterfaceAddr, ConnAddr))
		{
			p2pindex = i;
			break;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE,("pAd->P2pTable.ClientNumber = %d, p2pindex = %d\n", pAd->P2pTable.ClientNumber, p2pindex));
	if ((p2pindex < pAd->P2pTable.ClientNumber) && (p2pindex != P2P_NOT_FOUND))
	{
		pAddr = &pAd->P2pTable.Client[p2pindex].addr[0];
			P2pConnectPrepare(pAd, pAddr, P2PSTATE_PROVISION_COMMAND);
	}
	else
	{
		retry_cnt ++;
		if ( retry_cnt < 5 )
		{
			OS_WAIT(2000);
			goto retry;
		}
		DBGPRINT(RT_DEBUG_ERROR, ("Table Idx out of range!\n"));
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
		P2pSendWirelessEvent(pAd, RT_P2P_CONNECT_FAIL, NULL, NULL);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
	}

	return TRUE;
}

INT Set_P2P_State_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	DBGPRINT(RT_DEBUG_ERROR, ("P2P Current State\n"));	
	DBGPRINT(RT_DEBUG_ERROR, ("=====================================================================\n"));
	DBGPRINT(RT_DEBUG_ERROR, ("My Rule = %s\n", decodeMyRule(pP2PCtrl->Rule)));
	DBGPRINT(RT_DEBUG_ERROR, ("p2p_OpStatus = %04x\n", pAd->flg_p2p_OpStatusFlags));
	DBGPRINT(RT_DEBUG_ERROR, ("CTRL Machine State = %s.\n", decodeCtrlState(pP2PCtrl->CtrlCurrentState)));
	DBGPRINT(RT_DEBUG_ERROR, ("DISC Machine State = %s.\n", decodeDiscoveryState(pP2PCtrl->DiscCurrentState)));
	DBGPRINT(RT_DEBUG_ERROR, ("GO_FORM Machine State = %s.\n", decodeGroupFormationState(pP2PCtrl->GoFormCurrentState)));
	DBGPRINT(RT_DEBUG_ERROR, ("AutoReconn = %d\n", pAd->StaCfg.bAutoReconnect));
	/*DBGPRINT(RT_DEBUG_ERROR, ("P2PDiscoProvState = %s\n", decodeP2PState(pP2PCtrl->P2PDiscoProvState))); */
	DBGPRINT(RT_DEBUG_ERROR, ("P2PConnectState = %s\n", decodeP2PState(pP2PCtrl->P2PConnectState)));

	return TRUE;
}

INT Set_P2P_Reset_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	/*UINT32 DiscPerd = 0;*/
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	if (P2P_CLI_ON(pAd))
		P2P_CliStop(pAd);
	else if (P2P_GO_ON(pAd))
	{
		P2P_GoStop(pAd);
		if (INFRA_ON(pAd))
			AsicEnableBssSync(pAd);
	}
	P2PCfgInit(pAd);
	P2pGroupTabInit(pAd);
	pP2PCtrl->Rule = P2P_IS_DEVICE;
	pAd->flg_p2p_OpStatusFlags = P2P_DISABLE;
	pP2PCtrl->ConfigMethod = 0x188;
	pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
	pP2PCtrl->GoFormCurrentState = P2P_GO_FORM_IDLE;
	pP2PCtrl->DiscCurrentState = P2P_DISC_IDLE;
	pP2PCtrl->CtrlCurrentState = P2P_CTRL_IDLE;
	NdisZeroMemory(&pP2PCtrl->P2pCounter, sizeof(P2P_COUNTER_STRUCT));	
	P2pSetListenIntBias(pAd, 3);
	RTMPZeroMemory(pAd->P2pCfg.SSID, MAX_LEN_OF_SSID);
	RTMPMoveMemory(pAd->P2pCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
	pP2PCtrl->SSIDLen = WILDP2PSSIDLEN;
	/* Set Dpid to "not specified". it means, GUI doesn't set for connection yet. */
	pP2PCtrl->Dpid = DEV_PASS_ID_NOSPEC;
	RTMPZeroMemory(pAd->P2pCfg.ConnectingMAC, MAC_ADDR_LEN);
	return TRUE;
}

INT Set_P2P_Default_Config_Method_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	UINT32 ConfigMethod = 0;
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	ConfigMethod = simple_strtol(arg, 0, 10);
	if ((ConfigMethod >= 1) && (ConfigMethod <= 3))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Default Config Method is %d.\n", __FUNCTION__, ConfigMethod));
		DBGPRINT(RT_DEBUG_ERROR, ("    1: DISPLAY.    2: KEYPAD.    3. PBC\n"));
		pP2PCtrl->DefaultConfigMethod = ConfigMethod;
	}
	return TRUE;
}

INT Set_P2P_Link_Down_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	POS_COOKIE			pObj;


	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	P2pLinkDown(pAd, P2P_DISCONNECTED);
	return TRUE;
}

INT Set_P2P_Sigma_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	UINT32 SigmaEnabled = 0;
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	SigmaEnabled = simple_strtol(arg, 0, 10);
	pP2PCtrl->bSigmaEnabled = ((SigmaEnabled == 0) ? FALSE : TRUE);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Sigma Enable = %d.\n", __FUNCTION__, pP2PCtrl->bSigmaEnabled));

	return TRUE;
}

INT Set_P2P_QoS_NULL_Legacy_Rate_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	UINT32 QoSNull = 0;
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	QoSNull = simple_strtol(arg, 0, 10);
	pP2PCtrl->bLowRateQoSNULL = ((QoSNull == 0) ? FALSE : TRUE);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P QoS NULL using Legacy Rate Enable = %d.\n", __FUNCTION__, pP2PCtrl->bLowRateQoSNULL));

	return TRUE;
}

INT Set_P2P_CLIENT_PM_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	UINT32 P2pClientPm = 0;
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	P2pClientPm = simple_strtol(arg, 0, 10);
	pP2PCtrl->bP2pCliPmEnable= ((P2pClientPm == 0) ? FALSE : TRUE);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P CLIENT PM Enable = %d.\n", __FUNCTION__, pP2PCtrl->bP2pCliPmEnable));

	return TRUE;
}

INT Set_P2P_Enter_WSC_PIN_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;


	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	if ((strlen(arg) != 4) && (strlen(arg) != 8))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Reject this PIN (%s.)\n", __FUNCTION__, arg));
		return 0;
	}

	NdisZeroMemory(&pP2PCtrl->PinCode[0], sizeof(pP2PCtrl->PinCode));
	RTMPMoveMemory(&pP2PCtrl->PinCode[0], arg, strlen(arg));
	Set_AP_WscPinCode_Proc(pAd, arg);
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Enter Peer PIN Code = %s.\n", __FUNCTION__, pP2PCtrl->PinCode));

	return TRUE;
}

INT Set_P2P_Persistent_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	UINT32 PersistentEnabled = 0;
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	PersistentEnabled = simple_strtol(arg, 0, 10);
	pP2PCtrl->P2pControl.field.EnablePresistent = PersistentEnabled;
	if ( PersistentEnabled == TRUE )
	{ 
		pP2PCtrl->P2pCapability[1] |= GRPCAP_PERSISTENT;
		pP2PCtrl->P2pCapability[1] |= GRPCAP_PERSISTENT_RECONNECT;
		pP2PCtrl->ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
		pP2PCtrl->ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
	}
	else
	{
		pP2PCtrl->P2pCapability[1] &= 0xDD; /* Set GRPCAP_PERSISTENT and GRPCAP_PERSISTENT_RECONNECT bits to 0 */
		pP2PCtrl->ExtListenInterval = 0;
		pP2PCtrl->ExtListenPeriod = 0;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Persistent Enable = %d.\n", __FUNCTION__, pP2PCtrl->P2pControl.field.EnablePresistent));

	return TRUE;
}

INT Set_P2P_Dev_Discoverability_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)

{
	UINT32 DevDiscoverEnabled = 0;
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	DevDiscoverEnabled = simple_strtol(arg, 0, 10);
	pP2PCtrl->P2pControl.field.ClientDiscovery = DevDiscoverEnabled;
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Device Discoverability Enable = %d.\n", __FUNCTION__, pP2PCtrl->P2pControl.field.ClientDiscovery));

	return TRUE;
}

INT Set_P2P_DelDevByAddr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj;
	UCHAR DevAddr[6] = {0};
	PMAC_TABLE_ENTRY pEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	if (rtstrmactohex(arg, (PSTRING) &DevAddr[0]) == FALSE)
		return FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: DevMac = %02x:%02x:%02x:%02x:%02x:%02x\n",	__FUNCTION__, PRINT_MAC(DevAddr)));

	pEntry = MacTableLookup(pAd, DevAddr);

	if (pEntry)
	{
		PUCHAR		pOutBuffer = NULL;		
		ULONG		FrameLen = 0;
		USHORT 		Reason = REASON_NO_LONGER_VALID;
		HEADER_802_11 DeAuthHdr;
		
		MlmeAllocateMemory(pAd, &pOutBuffer);

		if (pOutBuffer)
		{
			DBGPRINT(RT_DEBUG_WARN, ("Send DEAUTH - Reason = %d frame tO %02x:%02x:%02x:%02x:%02x:%02x \n",
										Reason, PRINT_MAC(DevAddr)));

			MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, 
								pEntry->HdrAddr1,
								pEntry->HdrAddr2,
								pEntry->HdrAddr3);
	    	MakeOutgoingFrame(pOutBuffer,            &FrameLen,
	    	                  sizeof(HEADER_802_11), &DeAuthHdr,
	    	                  2,                     &Reason,
	    	                  END_OF_ARGS);

	    	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	    	MlmeFreeMemory(pAd, pOutBuffer);
			MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
			P2pGroupTabDelete(pAd, P2P_NOT_FOUND, DevAddr);
		}
	}
	
	return TRUE;

}

INT Set_P2P_DevDiscPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE			pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return 0;

	pP2PCtrl->DevDiscPeriod = (UINT32)simple_strtol(arg, 0, 10);
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P Device Discovery Period = %lu.\n", __FUNCTION__, pP2PCtrl->DevDiscPeriod));
	return TRUE;
}

INT Set_P2P_PriDeviceType_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE		pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	if (strlen(arg) != 16)
		return FALSE;

	AtoH(arg, pP2PCtrl->DevInfo.PriDeviceType, 8);
	
	hex_dump("Set_P2P_PriDeviceType_Proc:: PriDeviceType", pP2PCtrl->DevInfo.PriDeviceType, 8);
	return TRUE;
}

/*
	Usage Example:
	Reference P2P Spec. v1.1 Table 29 , Device Info attribute format.
	1). 01 = length , 0007 == > Category ID (Display), 0050F204 ==>WFA OUI, 0001 ==> Sub Category ID
	iwpriv p2p0 set p2p2ndDevTypeList=0100070050F2040001
	
	2). 02 = length , 0007 == > Category ID (Display), 0050F204 ==>WFA OUI, 0001 ==> Sub Category ID ;  0007 == > Category ID (Display), 0050F204 ==>WFA OUI, 0002 ==> Sub Category ID ;
	iwpriv p2p0 set p2p2ndDevTypeList=0200070050F204000100070050F2040002

	3). clean up 2nd Device Type List 
	iwpriv p2p0 set p2p2ndDevTypeList=	
*/

INT Set_P2P_SecDevTypeList_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE		pObj;
	UCHAR len=0;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	len = strlen(arg);

	if (len == 0) //reset to zero
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: reset 2nd Device Type List!\n", __FUNCTION__));
		pP2PCtrl->DevInfo.SecDevTypList[0]=0x00; /* length is zero, so no don't need following 8 bytes*/
	}
	else if((len >= (9*2))) /* 9 = 1 (length) + 8 (Category ID + WFA OUI + Sub-Category ID */
	{
		if ((len % (8*2))==2)
			
		{
			UCHAR tpylen=0;
			AtoH(arg, (PUCHAR)&tpylen, 1);
			AtoH(arg, pP2PCtrl->DevInfo.SecDevTypList, (tpylen*8)+1);
			hex_dump("Set_P2P_SecDevTypeList_Proc : p2p2ndDevTypList",pP2PCtrl->DevInfo.SecDevTypList, (tpylen*8)+1);
			
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: arg length incorrect\n", __FUNCTION__));	
			return FALSE;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: arg length is not enough!\n", __FUNCTION__));	
		return FALSE;
	}

	return TRUE;
}

INT Set_P2P_Cancel_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE		pObj;
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PWSC_CTRL pWscControl = NULL;
	INT val = (INT)simple_strtol(arg, 0, 10);

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	if (val == 1)
	{
		pP2PCtrl->P2PConnectState = P2P_CONNECT_IDLE;
		if (!MAC_ADDR_EQUAL(&ZERO_MAC_ADDR[0], &pP2PCtrl->ConnectingMAC[0]))
		{
			UCHAR p2pindex;
	
			p2pindex = P2pGroupTabSearch(pAd, pP2PCtrl->ConnectingMAC);
			if (p2pindex < MAX_P2P_GROUP_SIZE)
			{
				if (pAd->P2pTable.Client[p2pindex].Rule == P2P_IS_GO)
					pAd->P2pTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY_GO;
				else
					pAd->P2pTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
				pAd->P2pTable.Client[p2pindex].StateCount = 0;
			}
			NdisZeroMemory(pP2PCtrl->ConnectingMAC, MAC_ADDR_LEN);
		}

		if (P2P_GO_ON(pAd))
		{
			UINT32 i, p2pEntryCnt=0;
			MAC_TABLE_ENTRY	*pEntry;
			INT	 IsAPConfigured;
			
			pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl;
			IsAPConfigured = pWscControl->WscConfStatus;
			if (pWscControl->bWscTrigger)
			{
				WscStop(pAd, FALSE, pWscControl);
				pWscControl->WscPinCode = 0;
				WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, MAIN_MBSSID, NULL, 0, AP_MODE);
				WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, (MAIN_MBSSID | MIN_NET_DEVICE_FOR_P2P_GO), NULL, 0, AP_MODE);
				APUpdateBeaconFrame(pAd, pObj->ioctl_if);
			}

			for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
			{
				pEntry = &pAd->MacTab.Content[i];
				if (IS_P2P_GO_ENTRY(pEntry) && (pEntry->WpaState == AS_PTKINITDONE))
					p2pEntryCnt++;
			}
			DBGPRINT(RT_DEBUG_ERROR, ("%s:: Total= %d. p2pEntry = %d.\n", __FUNCTION__, pAd->MacTab.Size, p2pEntryCnt));
			if ((p2pEntryCnt == 0) && (pAd->flg_p2p_OpStatusFlags == P2P_GO_UP))
			{
			}
	    }
		else if (P2P_CLI_ON(pAd))
		{
			pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].WscControl;
			pWscControl->WscPinCode = 0;
			if (pWscControl->bWscTrigger)
				WscStop(pAd, TRUE, pWscControl);
			P2pLinkDown(pAd, P2P_DISCONNECTED);
		}
		P2pStopScan(pAd);
		pP2PCtrl->bPeriodicListen = TRUE;
		pP2PCtrl->bConfirmByUI = FALSE;

		if (INFRA_ON(pAd) || P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
		{
			if (pAd->CommonCfg.BBPCurrentBW == BW_40)
			{
				UCHAR BBPValue = 0;
				UINT32 Data = 0, macStatus;
				UINT32 MTxCycle;

				AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
				AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

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

				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
				BBPValue &= (~0x18);
				BBPValue |= 0x10;
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

				/* Enable MAC Tx/Rx */
				RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
				Data |= 0x0C;
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
			}
			else
			{
				AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
				AsicLockChannel(pAd, pAd->CommonCfg.Channel);
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Cancel P2P action\n", __FUNCTION__, val));
	return TRUE;
}

#endif /* P2P_SUPPORT */

