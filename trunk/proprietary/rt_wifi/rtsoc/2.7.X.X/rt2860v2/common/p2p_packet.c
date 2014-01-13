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
	p2p_packet.c

	Abstract:
	Peer to peer is also called Wifi Direct. P2P is a Task Group of WFA. this file contains all functions that handles p2p packets 

	Revision History:
	Who              When               What
	--------    ----------    ----------------------------------------------
	Jan Lee         2010-03-08    created for Peer-to-Peer(Wifi Direct)
*/
#include "rt_config.h"

extern UCHAR	OutMsgBuf[];		/* buffer to create message contents */
extern UCHAR	WILDP2PSSID[];
extern UCHAR	WILDP2PSSIDLEN;
extern UCHAR	WIFIDIRECT_OUI[];
extern UCHAR	P2POUIBYTE[];
extern UCHAR ZERO_MAC_ADDR[];
extern UCHAR WPS_OUI[];
extern UCHAR	STA_Wsc_Pri_Dev_Type[];
extern UCHAR	AP_Wsc_Pri_Dev_Type[];

extern INT Set_P2p_OpMode_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

extern INT Set_P2pCli_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);


/*	
	==========================================================================
	Description: 
		Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
		802.11u. 7.4.7.10
		
	Parameters: 
	Note:

	==========================================================================
 */
VOID PeerGASIntialReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	/*UCHAR	Action = Elem->Msg[LENGTH_802_11+1]; */
	/*PUCHAR	pAdProtocolElem; */
	/*PUCHAR	pQueryReq; */
	
	DBGPRINT(RT_DEBUG_TRACE,("P2pPeer  GASIntialReqAction = %ld, \n", Elem->MsgLen));
}

/*	
	==========================================================================
	Description: 
		Publiac action frame. But with ACtion is GAS_INITIAL_RSP (12).
		802.11u. 7.4.7.11
		
	Parameters: 
	Note:

	==========================================================================
 */
VOID PeerGASIntialRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	/*UCHAR	Action = Elem->Msg[LENGTH_802_11+1]; */
	/*UCHAR	StatusCode; */
	/*USHORT	ComeBackDelay; */
	/*PUCHAR	pAdProtocolElem; */
	/*PUCHAR	pQueryRsp; */
	ULONG	i;
	PUCHAR	pDest;
	
	DBGPRINT(RT_DEBUG_TRACE,("P2pPeer  PeerGASIntialRspAction = %ld, \n", Elem->MsgLen));
	pDest = &Elem->Msg[0];
	for (i = 0; i <Elem->MsgLen; )
	{
		DBGPRINT(RT_DEBUG_TRACE,(": %x %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2), 
		*(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8), *(pDest+i+9)));
		i = i + 10;
	}
}

/*	
	==========================================================================

	Description: 
		Receive Public Action frame that set to  verdor specific and with OUI type = WFA P2P
	==========================================================================
 */
VOID P2PPublicAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	DBGPRINT(RT_DEBUG_ERROR, ("!!!!! Should not in here !!!!!\n"));
}

/*	
	==========================================================================
	Description: 
		Make a P2P Fake NoA Attribute to trigger myself to restart NoA. The Start time is changed. Duration and Interval and Count
		is the same as GO's beacon
		
	Parameters: 
		 StartTime : A new Start time.
		 pOutBuffer : pointer to buffer that should put data to.
	Note:
		 
	==========================================================================
 */
VOID P2PMakeFakeNoATlv(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	 StartTime,
	IN PUCHAR		pOutBuffer)
{
	PUCHAR		pDest;
		
	pDest = pOutBuffer;

	*(pDest) = SUBID_P2P_NOA;
	/* Length is 13*n + 2 = 15 when n = 1 */
	*(pDest+1) = 15;
	/* Lenght 2nd byte */
	*(pDest+2) = 0;
	/* Index. */
	*(pDest+3) = pAd->P2pCfg.GONoASchedule.Token;
	/* CT Windows and OppPS parm. Don't turn on both. So Set CTWindows = 0 */
	*(pDest+4) = 0;
	/* Count.  Test Plan set to 255. */
	*(pDest+5) = pAd->P2pCfg.GONoASchedule.Count;
	/* Duration */
	RTMPMoveMemory((pDest+6), &pAd->P2pCfg.GONoASchedule.Duration, 4);
	/* Interval */
	RTMPMoveMemory((pDest+10), &pAd->P2pCfg.GONoASchedule.Interval, 4);
	RTMPMoveMemory((pDest+14), &StartTime, 4);

}

/*	
	==========================================================================
	Description: 
		Insert P2P subelement P2P Group Info format in Probe Response. it contains device information of 
		P2P Clients that are members of my P2P group.  
		
	Parameters: 
		 pInBuffer : pointer to data that contains data to put in
		 pOutBuffer : pointer to buffer that should put data to.
	Note:
		 
	==========================================================================
 */
ULONG InsertP2PGroupInfoTlv(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		pOutBuffer)
{
	PUCHAR	pDest, pContent;
	UCHAR	Length;
	UCHAR	i;
	UCHAR	NoOfClient = 0;
	P2P_CLIENT_INFO_DESC		ClientInfo;
	RT_P2P_CLIENT_ENTRY	*pClient;
	UCHAR		ZeroType[P2P_DEVICE_TYPE_LEN];
	UCHAR		DevCapability;
	USHORT config_method = 0;
	
	pDest = pOutBuffer;
	RTMPZeroMemory(pDest, 255);
	RTMPZeroMemory(ZeroType, P2P_DEVICE_TYPE_LEN);
	*pDest = SUBID_P2P_GROUP_INFO;
	*(pDest + 2) = 0; 		/* Set length to 0 first.  Need to update to real length in the end of this function. */
	pContent = pDest + 3;	/* pContent points to payload. */
	Length = 0;

	for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
	{
		pClient = &pAd->P2pTable.Client[i];

		/* if (IS_P2P_PEER_CLIENT_OP(pClient)) */
		if (pClient->P2pClientState == P2PSTATE_CLIENT_WPS_DONE)
		{
			NoOfClient++;
			RTMPZeroMemory(&ClientInfo, sizeof(ClientInfo));
			RTMPMoveMemory(ClientInfo.DevAddr, pClient->addr, MAC_ADDR_LEN);
			DBGPRINT(RT_DEBUG_INFO, ("  -- InsertP2PGroupInfoTlv( ) (Mac  %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pClient->addr)));
			RTMPMoveMemory(ClientInfo.InterfaceAddr, pClient->InterfaceAddr, MAC_ADDR_LEN);
			DevCapability = pClient->DevCapability;
			if (P2P_TEST_FLAG(pClient, P2PFLAG_DEVICE_DISCOVERABLE))
				DevCapability |= DEVCAP_CLIENT_DISCOVER;
			
			ClientInfo.Capability = DevCapability;
			config_method = cpu2be16(pClient->ConfigMethod);
			RTMPMoveMemory(ClientInfo.ConfigMethod, &config_method, 2);
			RTMPMoveMemory(ClientInfo.PrimaryDevType, pClient->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
			ClientInfo.NumSecondaryType = pClient->NumSecondaryType;
			/* JANTEMP   set to zero first. will add soon. */
			ClientInfo.NumSecondaryType = 0;
			RTMPMoveMemory(ClientInfo.PrimaryDevType, pClient->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
			ClientInfo.Length = SIZE_OF_FIXED_CLIENT_INFO_DESC - 1 + pClient->DeviceNameLen + 4;
			RTMPMoveMemory(pContent, &ClientInfo, SIZE_OF_FIXED_CLIENT_INFO_DESC);
			pContent += SIZE_OF_FIXED_CLIENT_INFO_DESC;
			/* Length is accumulated length for this attirbute. */
			Length += SIZE_OF_FIXED_CLIENT_INFO_DESC;
			/* Insert WPS Device Name TLV */
			*((PUSHORT) pContent) = cpu2be16(WSC_ID_DEVICE_NAME);
			*((PUSHORT) (pContent + 2)) = cpu2be16(pClient->DeviceNameLen);	
			RTMPMoveMemory(pContent + 4, pClient->DeviceName, pClient->DeviceNameLen);
			Length += (UCHAR)pClient->DeviceNameLen + 4;
			pContent += (pClient->DeviceNameLen + 4);
			/* Assign this client info descriptor's length. Length is accumulated length. so can't use Length. */
			/*if (!RTMPEqualMemory(pClient->SecondaryDevType, &ZeroType, P2P_DEVICE_TYPE_LEN))
			{
				// JANTEMP. force to support add only one secondary device type first. will improve later.
				*pContent = 1;
				RTMPMoveMemory(pContent+1, pClient->SecondaryDevType, P2P_DEVICE_TYPE_LEN);
				Length += (1+P2P_DEVICE_TYPE_LEN);
				ClientInfo.Length += (1+P2P_DEVICE_TYPE_LEN);
			}*/
			DBGPRINT(RT_DEBUG_INFO, (" ----- InsertP2PGroupInfoTlv(%d) (Total Len now = %d) DevNameLen = %ld.\n", i, Length, pClient->DeviceNameLen));
		}
	}

	/* Because the length field doesn't count itself. So when update Attribute length, need to add number of client descriptor. */
	//Length += NoOfClient;

	*(pDest + 1) = Length;		/* Set to real length */

	return (Length + 3);

}


/*	
	==========================================================================
	Description: 
		Used to insert P2P subelement TLV format.
		
	Parameters: 
		 pInBuffer : pointer to data that contains data to put in
		 pOutBuffer : pointer to buffer that should put data to.
	Note:
		 
	==========================================================================
 */
ULONG InsertP2PSubelmtTlv(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR			SubId,
	IN PUCHAR		pInBuffer,
	IN PUCHAR		pOutBuffer)
{
		PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
		PUCHAR	pDest;
		ULONG	Length;

		pDest = pOutBuffer;
		RTMPZeroMemory(pDest, 255);
		*pDest = SubId;
		pDest += 1;
		Length = 0;
		switch(SubId)
		{
			case SUBID_P2P_INVITE_FLAG:
				*pDest = 1;
				*(pDest + 1) = 0;
				*(pDest + 2) = *pInBuffer;
				
				Length = 4;
				break;
			case SUBID_P2P_STATUS:	/* 0 */
				*pDest = 1;
				*(pDest + 1) = 0;
				*(pDest + 2) = *pInBuffer;
				
				Length = 4;
				break;
			case SUBID_P2P_MINOR_REASON:	/* 0 */
				break;
			case SUBID_P2P_CAP: 	/* 2 */
				*pDest = 2;
				*(pDest + 1) = 0;
				RTMPMoveMemory(pDest + 2, pInBuffer, 2);
				Length = 5;
				break;
			case SUBID_P2P_DEVICE_ID:	/* 3 this is Device Address! */
			case SUBID_P2P_GROUP_BSSID: /* 6 group Bssid */
			case SUBID_P2P_INTERFACE_ADDR:	/* 9 */
				*pDest = 6;
				*(pDest + 1) = 0;
				RTMPMoveMemory(pDest + 2, pInBuffer, MAC_ADDR_LEN);
				/* DEvice Type */
				/*RTMPMoveMemory(pDest + 7, pAd->CurrentAddress, 8); */

				Length = 9;
				break;
			case SUBID_P2P_OWNER_INTENT:	/* 4 */
				*pDest = 1;
				*(pDest + 1) = 0;
				*(pDest + 2) = *pInBuffer;
				Length = 4;
				break;
	
			case SUBID_P2P_CONFIG_TIMEOUT:	/* 5 */
				*pDest = 2;
				*(pDest + 1) = 0;
				RTMPMoveMemory(pDest + 2, pInBuffer, 2);
				Length = 5;
				break;
			case SUBID_P2P_CHANNEL_LIST:	/* 11 */
				*(pDest + 1) = 0;
				/*country string . Two ASCII +	one more byte */
				*(pDest + 2) = 0x55;
				*(pDest + 3) = 0x53;
				*(pDest + 4) = 0x04;
				InsertP2pChannelList(pAd, *pInBuffer, &Length, (pDest + 5));
				*pDest = Length + 3;
				Length += 6;
				break;
			case SUBID_P2P_OP_CHANNEL:
			case SUBID_P2P_LISTEN_CHANNEL:	/* 6 */
				*pDest = 5;
				*(pDest + 1) = 0;
				/*Annex J. 802.11ERVmb_D3/0.pdf */
				*(pDest + 2) = 0x55;
				*(pDest + 3) = 0x53;
				*(pDest + 4) = 0x04;
				*(pDest + 5) = ChannelToClass(*pInBuffer, 1/*COUNTRY_USA*/);
				*(pDest + 6) = *pInBuffer;
				Length = 8;
				break;
			case SUBID_P2P_EXT_LISTEN_TIMING:	/* 8 */
				*pDest = 4;
				*(pDest + 1) = 0;
				*((PUSHORT) (pDest + 2)) = (P2P_EXT_LISTEN_PERIOD);
				*((PUSHORT) (pDest + 4)) = (P2P_EXT_LISTEN_INTERVAL);
				Length = 7;
				break;
			case SUBID_P2P_MANAGEABILITY:	/* 10 */
				break;	
			case SUBID_P2P_NOA: /* 12 */
				break;
			case SUBID_P2P_DEVICE_INFO: /* 13 */
				*pDest = 17 + pP2PCtrl->DeviceNameLen + 4 + (8*pP2PCtrl->DevInfo.SecDevTypList[0]);
				*(pDest + 1) = 0;

				RTMPMoveMemory(pDest + 2, pInBuffer, MAC_ADDR_LEN);
				/* DEvice Type */
				*((PUSHORT) (pDest + 8)) = cpu2be16(pP2PCtrl->ConfigMethod);
				RTMPMoveMemory(pDest + 10, &pP2PCtrl->DevInfo.PriDeviceType[0], 8);

				{
					*(pDest + 18) = pP2PCtrl->DevInfo.SecDevTypList[0];
					NdisMoveMemory((pDest + 19), &pP2PCtrl->DevInfo.SecDevTypList[1], (8*(pP2PCtrl->DevInfo.SecDevTypList[0])));
					pDest += (19 + (8*pP2PCtrl->DevInfo.SecDevTypList[0]));
				}

				*((PUSHORT) pDest) = cpu2be16(WSC_ID_DEVICE_NAME);
				*((PUSHORT) (pDest + 2)) = cpu2be16(pP2PCtrl->DeviceNameLen);
				RTMPMoveMemory(pDest + 4, &pP2PCtrl->DeviceName[0], pP2PCtrl->DeviceNameLen);

				Length = 20+ (8*(pP2PCtrl->DevInfo.SecDevTypList[0])) + pP2PCtrl->DeviceNameLen + 4;
				break;
			case SUBID_P2P_GROUP_INFO:	/* 14 */
				break;
			case SUBID_P2P_GROUP_ID:	/* 15 */
				RTMPMoveMemory(pDest, &pInBuffer[0], (pInBuffer[0] + 2));
				Length = 1 + (pInBuffer[0] + 2);
				DBGPRINT(RT_DEBUG_ERROR, (" -----Insert SUBID_P2P_GROUP_ID (Len = %ld) \n", Length));
				break;
			case SUBID_P2P_INTERFACE:	/* 16 */
				break;
			default:
				*pDest = 0;
				Length = 0;
				break;
		}
		
		return Length;
}

/*	
	==========================================================================
	Description: 
		Used to insert P2P Channel list attribute.
		
	Parameters: 
		ChannelListLen : output for total length.
		 pDest : pointer to buffer that should put data to.
	Note:
		 
	==========================================================================
 */
VOID InsertP2pChannelList(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR	 OpChannel, 
	OUT ULONG	 *ChannelListLen, 
	OUT PUCHAR 	pDest)
{
	UCHAR		i, pos;
	UCHAR		LastRegClass = 0xff, CurRegClass;
	PUCHAR		pLastLenPos;
	UCHAR		LastNum = 0;

	pLastLenPos = pDest + 1;
	pos = 2;
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		CurRegClass = ChannelToClass(pAd->ChannelList[i].Channel, 1);
		/* 0. Decide current regulatory class. 11y */
		/* Insert RegClass if necessary */
		if (LastRegClass == 0xff)
		{
			/* case 0 : initilize */
			LastRegClass = CurRegClass;
			*(pDest) = CurRegClass;
		}
		else if ((CurRegClass != LastRegClass) && (LastRegClass != 0xff))
		{
			/* case 1 : change regulatory class */
			*(pDest + pos) = CurRegClass;
			*pLastLenPos = LastNum;
			
			LastNum = 0;
			LastRegClass = CurRegClass;
			pLastLenPos = pDest + pos + 1;
			pos = pos + 2;
		}
			
		/* Insert  */
		*(pDest + pos) = pAd->ChannelList[i].Channel;
		LastNum++;
		pos++;

		/* Last item. Update Num. */
		if (i == pAd->ChannelListNum -1)
		{
			*pLastLenPos = LastNum;
		}
	}

	*ChannelListLen = pos;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Channel List ==> output length is %ld \n", __FUNCTION__, *ChannelListLen));
	for (i = 0; i <*ChannelListLen; )
	{
		DBGPRINT(RT_DEBUG_TRACE, (": %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2), 
		*(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8)));
		i = i + 9;
	}

}

/*	
	==========================================================================
	Description: 
		Parse P2P subelement content to fill into the correct output buffer
		
	Parameters: 
		return : TRUE if the pSearchAddr is in GroupInfoAttribute.
	Note:
		 
	==========================================================================
 */
BOOLEAN P2pParseGroupInfoAttribute(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR P2pindex, 
	IN VOID *Msg, 
	IN ULONG MsgLen) 
{
	PEID_STRUCT   pEid;
	ULONG				Length = 0;
	LONG		GroupInfoLen;
	UCHAR		i;
	UCHAR		idx;
	PUCHAR		pData;
	UCHAR		ThisDescLen;
	BOOLEAN		brc = TRUE;
	P2P_CLIENT_INFO_DESC	*pClient;
	USHORT 			WscType, WscLen;
	ULONG			LeftLength;
	PP2PEID_STRUCT	pP2pEid;
	ULONG			AttriLen;
	UCHAR			SmallerP2Pidx;
	BOOLEAN	bSendP2pEvent = FALSE;

	pEid = (PEID_STRUCT) Msg;

	Length = 0;
	LeftLength = MsgLen; 
	pEid = (PEID_STRUCT)Msg;
	while ((ULONG)(pEid->Len + 2) <= LeftLength)
	{
		/* might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE. */
		/* To check Octet[1] is because Ralink add one byte itself for P2P IE. So the IE content shift one byte afterward. */
		if (RTMPEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
		{
			/* To check Octet[5] for first P2PEid */
			/* is because Ralink add one byte itself for P2P IE. So the IE content shift one byte afterward. */
			pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
			AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			Length = 0;
			while ((Length + 3 + AttriLen) <= pEid->Len)    
			{
				switch(pP2pEid->Eid)
				{
					case SUBID_P2P_GROUP_INFO:
						GroupInfoLen = AttriLen;
						pData = &pP2pEid->Octet[0];
						DBGPRINT(RT_DEBUG_INFO, ("SUBID_P2P_GROUP_INFO - Go index=%d\n",  P2pindex));
						while(GroupInfoLen > 0)
						{	
							pClient = (P2P_CLIENT_INFO_DESC*)pData;
							ThisDescLen = pClient->Length;
							if ((ThisDescLen < 23) || (ThisDescLen > GroupInfoLen))
							{
								DBGPRINT(RT_DEBUG_ERROR, ("Error parsing P2P IE group info attribute. This Client -%d/%ld\n", ThisDescLen, GroupInfoLen));
								break;
							}

							idx = P2pGroupTabSearch(pAd, pClient->DevAddr);
							if ((idx < MAX_P2P_GROUP_SIZE) 
								&& (pAd->P2pTable.Client[idx].P2pClientState != P2PSTATE_DISCOVERY_CLIENT)
								&& (pAd->P2pTable.Client[idx].P2pClientState != P2PSTATE_CLIENT_DISCO_COMMAND))
							{
								/* P2P topology changed. Reset the P2P Table and rescan. */
								if (idx > P2pindex)
									SmallerP2Pidx = P2pindex + 1;	/* don't delete GO */
								else
									SmallerP2Pidx = idx;		/* this method also delete GO. :(	  */
								DBGPRINT(RT_DEBUG_TRACE, ("!P2P topology changed[P2pClientState = %d] when parsing P2P IE group info attribute. Delete from index : %d\n", pAd->P2pTable.Client[idx].P2pClientState, SmallerP2Pidx ));
								/*P2PPrintP2PEntry(pAd, idx); */
								for (i = (SmallerP2Pidx); i < MAX_P2P_GROUP_SIZE;i++)
							{
								
									if ((pAd->P2pTable.Client[i].P2pClientState == P2PSTATE_PROVISION_COMMAND)
										|| (pAd->P2pTable.Client[i].P2pClientState == P2PSTATE_CONNECT_COMMAND)
										|| (pAd->P2pTable.Client[i].P2pClientState > P2PSTATE_DISCOVERY_CLIENT))
									{
										DBGPRINT(RT_DEBUG_ERROR, ("!break right away because we have another connect command to continue. update topology is not so important to do right now.\n" ));
										break;
									}
									else
									{
										P2pGroupTabDelete(pAd, i, pAd->P2pTable.Client[i].addr);
									}
									/* Don't update this now. */
									if (pAd->P2pTable.ClientNumber == SmallerP2Pidx)
										return FALSE;
								}
								/*  because we check idx in following code,  */
								/* So need to Search again after topology changed */
								idx = P2pGroupTabSearch(pAd, pClient->DevAddr);
								
							}

							if ((idx == P2P_NOT_FOUND) && ( pAd->P2pTable.ClientNumber < (MAX_P2P_GROUP_SIZE - 1))
								&& (!RTMPEqualMemory(pClient->DevAddr, pAd->P2pCfg.CurrentAddress, MAC_ADDR_LEN)))
							{
								idx = P2pGroupTabInsert(pAd, pClient->DevAddr, P2PSTATE_DISCOVERY_CLIENT, NULL, 0, 0, 0);
								
								DBGPRINT(RT_DEBUG_TRACE, ("Insert a P2P Client to P2P table [%d]  .\n", idx));
								/* Insert a P2P Client followi ng the GO that we received. Need to delete ALL following client that is already in P2P table. */
								/* Because the number of client that is in the p2p group might be changed. */
								bSendP2pEvent = TRUE;
							}

							if (idx < MAX_P2P_GROUP_SIZE)
							{
								pAd->P2pTable.Client[idx].DevCapability = pClient->Capability;
								/* Don't update state when it's in P2PSTATE_CLIENT_DISCO_COMMAND */
								if (pAd->P2pTable.Client[idx].P2pClientState != P2PSTATE_CLIENT_DISCO_COMMAND)
									pAd->P2pTable.Client[idx].P2pClientState = P2PSTATE_DISCOVERY_CLIENT;
								pAd->P2pTable.Client[idx].ConfigMethod = *((PUSHORT) pClient->ConfigMethod);
								pAd->P2pTable.Client[idx].ConfigMethod = be2cpu16(pAd->P2pTable.Client[idx].ConfigMethod);
								RTMPMoveMemory(pAd->P2pTable.Client[idx].PrimaryDevType, pClient->PrimaryDevType, P2P_DEVICE_TYPE_LEN);
								pAd->P2pTable.Client[idx].NumSecondaryType = pClient->NumSecondaryType;
								WscType = cpu2be16(*((PUSHORT) &pClient->Octet[pClient->NumSecondaryType*P2P_DEVICE_TYPE_LEN]));
								WscLen  = cpu2be16(*((PUSHORT) (&pClient->Octet[2 + pClient->NumSecondaryType*P2P_DEVICE_TYPE_LEN])));
								RTMPMoveMemory(&pAd->P2pTable.Client[idx].DeviceName[0], &pClient->Octet[4+(pClient->NumSecondaryType*P2P_DEVICE_TYPE_LEN)], 32);
								if (WscLen <= 32)
									pAd->P2pTable.Client[idx].DeviceNameLen = WscLen;
								pAd->P2pTable.Client[idx].MyGOIndex = P2pindex;
								pAd->P2pTable.Client[idx].DeviceName[pAd->P2pTable.Client[idx].DeviceNameLen] = 0x0;
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
								if (bSendP2pEvent)
									P2pSendWirelessEvent(pAd, RT_P2P_DEVICE_FIND, &pAd->P2pTable.Client[idx], NULL);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
								if (pAd->P2pCfg.ConnectingIndex < MAX_P2P_GROUP_SIZE)
								{
									if (RTMPEqualMemory(pAd->P2pTable.Client[idx].addr, &pAd->P2pCfg.ConnectingMAC[0], MAC_ADDR_LEN))
										P2pConnectAfterScan(pAd, FALSE, idx);
								}
							}
							GroupInfoLen -= (ThisDescLen + 1);
							pData += (ThisDescLen + 1);
						};
						break;
					default:
						break;
						
				}
				Length = Length + 3 + AttriLen;  /* Eid[1] + Len[1]+ content[Len] */
				pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
				AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			}
			/* We already get what we want. so break. */
			break;
		}
		LeftLength = LeftLength - pEid->Len - 2;
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	}
	return brc;

}

/*	
	==========================================================================
	Description: 
		Parse P2P NoA subelement content to make appropriate action for NoA schedule.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pParseNoASubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	IN UCHAR  wcidindex,
	IN UINT32 Sequence)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	ULONG				Length = 0;
	PP2PEID_STRUCT	pP2pEid;
	ULONG			AttriLen;
	ULONG			LeftLength;
	PEID_STRUCT 	pEid;
	BOOLEAN 		brc;
	BOOLEAN 		bNoAAttriExist = FALSE;
	PUCHAR pPtrEid = NULL;
		
	/* Intel sends multiple P2P IE... So I can't give each input a default value.. */
	if (MsgLen == 0)
		return;
	
	LeftLength = MsgLen; 
	pEid = (PEID_STRUCT)Msg;
	while ((ULONG)(pEid->Len + 2) <= LeftLength)
	{
		/* might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE. */
		if (RTMPEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
		{
			/* Get Request content capability */
			pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
			pPtrEid = (PUCHAR) pP2pEid;
			AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			Length = 0;
			while ((Length + 3 + AttriLen) <= pEid->Len)	
			{
				switch(pP2pEid->Eid)
				{
					case SUBID_P2P_NOA:
						{
							PUCHAR pData = &pEid->Octet[0];
							DBGPRINT(RT_DEBUG_INFO,("Get NoA Attr: %x %x %x %x %x %x %x %x %x \n", *(pData+0), *(pData+1), *(pData+2), 
													*(pData+3), *(pData+4), *(pData+5), *(pData+6), *(pData+7), *(pData+8)));
							bNoAAttriExist = TRUE;
							brc = P2pHandleNoAAttri(pAd, &pAd->MacTab.Content[wcidindex], pPtrEid);
							/* Got a NoA Attribute from this p2pindex. In fact, This should be GO. */
							if (brc == TRUE)
								pP2PCtrl->NoAIndex = wcidindex;
						}
						break;
					default:
						break;
						
				}
				Length = Length + 3 + AttriLen;  /* Eid[1] + Len[1]+ content[Len] */
				pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);
				pPtrEid = (PUCHAR) pP2pEid;
				AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			}	
		}
		LeftLength = LeftLength - pEid->Len - 2;
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}

	if (bNoAAttriExist == FALSE)
	{
		if (P2P_TEST_BIT(pAd->P2pCfg.CTWindows, P2P_OPPS_BIT))
		{
			DBGPRINT(RT_DEBUG_TRACE,("Beacon and no NoA Attribute! \n"));
			P2pStopOpPS(pAd);
		}
		if ((pAd->MacTab.Content[wcidindex].P2pInfo.NoADesc[0].bValid == TRUE))
		{
			DBGPRINT(RT_DEBUG_TRACE,("Beacon and no NoA Attribute!Stop active NoA [%d]\n", Sequence));
			P2pStopNoA(pAd, &pAd->MacTab.Content[wcidindex]);
		}
	}

}


/*	
	==========================================================================
	Description: 
		Parse P2P NoA subelement content to make appropriate action for NoA schedule.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pParseExtListenSubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT USHORT *ExtListenPeriod,
	OUT USHORT *ExtListenInterval) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	ULONG				Length = 0;
	PP2PEID_STRUCT	pP2pEid;
	ULONG			AttriLen;
	/*UCHAR			offset; */
	ULONG			LeftLength;
	PEID_STRUCT		pEid;
	USHORT			ExtTime;
	/*BOOLEAN			brc; */
	/*PUCHAR			pData; */
	
	/* Intel sends multiple P2P IE... So I can't give each input a default value.. */
	if (MsgLen == 0)
		return;

	LeftLength = MsgLen; 
	pEid = (PEID_STRUCT)Msg;
	while ((ULONG)(pEid->Len + 2) <= LeftLength)
	{
		/* might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE. */
		if (RTMPEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
		{
			/* Get Request content capability */
			pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
			AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			Length = 0;
			while ((Length + 3 + AttriLen) <= pEid->Len)    
			{
				switch(pP2pEid->Eid)
				{
					case SUBID_P2P_EXT_LISTEN_TIMING:
						if (AttriLen == 4)
						{
							ExtTime = *(PUSHORT)&pP2pEid->Octet[0];
							DBGPRINT(RT_DEBUG_TRACE, ("  - (Ext Listen Period %x = %d)  \n", ExtTime, ExtTime));
							if (ExtTime > 0)
								pP2PCtrl->ExtListenPeriod = ExtTime;
							ExtTime = *(PUSHORT)&pP2pEid->Octet[2];
							DBGPRINT(RT_DEBUG_TRACE, ("  - (Ext Listen Interval  %x = %d)  \n", ExtTime, ExtTime));
							if (ExtTime > 0)
								pP2PCtrl->ExtListenInterval = ExtTime;
						}
						break;
					default:
						break;
						
				}
				Length = Length + 3 + AttriLen;  /* Eid[1] + Len[1]+ content[Len] */
				pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
				AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			}

		}
		LeftLength = LeftLength - pEid->Len - 2;
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	}

}

/*	
	==========================================================================
	Description: 
		Parse P2P subelement content to fill into the correct output buffer
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pParseManageSubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT UCHAR *pChannel,
	OUT UCHAR *pNumOfP2pOtherAttribute,
	OUT UCHAR *pTotalNumOfP2pAttribute,
	OUT UCHAR *pMamageablity,
	OUT UCHAR *pMinorReason)
{
	PP2PEID_STRUCT	pP2pEid;
	PEID_STRUCT		pEid;
	ULONG			Length;
	ULONG			AttriLen;
	ULONG			LeftLength;

	DBGPRINT(RT_DEBUG_TRACE, ("P2pParseManageSubElmt  MsgLen = %ld.  \n", MsgLen));
	if (pNumOfP2pOtherAttribute != NULL)
		*pNumOfP2pOtherAttribute = 0;
	
	if (pTotalNumOfP2pAttribute != NULL)
		*pTotalNumOfP2pAttribute = 0;

	LeftLength = MsgLen; 
	pEid = (PEID_STRUCT)Msg;
	while ((ULONG)(pEid->Len + 2) <= LeftLength)
	{
		if (pEid->Eid == IE_CHANNEL_USAGE)
		{
			*pChannel = pEid->Octet[2];
			DBGPRINT(RT_DEBUG_TRACE, ("IE_CHANNEL_USAGE  = %x %x %x %x [ch=]%x.  \n", pEid->Eid, pEid->Len, pEid->Octet[0], pEid->Octet[1], pEid->Octet[2]));
		}
		/* might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE. */
		if (RTMPEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
		{
			/* Get Request content capability */
			pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
			AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			Length = 0;
			while ((Length + 3 + AttriLen) <= pEid->Len)    
			{
				switch(pP2pEid->Eid)
				{
					case SUBID_P2P_MINOR_REASON:
						DBGPRINT(RT_DEBUG_TRACE, ("SYNC -  Has P2P SUBID_P2P_MINOR_REASON IE Minor Reason = %d.\n", pP2pEid->Octet[0]));
						if (pTotalNumOfP2pAttribute != NULL)
							*pTotalNumOfP2pAttribute = *pTotalNumOfP2pAttribute+1;
						if (pNumOfP2pOtherAttribute != NULL)
							*pNumOfP2pOtherAttribute = *pNumOfP2pOtherAttribute+1;
						if (pMinorReason != NULL)
							*pMinorReason = pP2pEid->Octet[0];
						break;
					case SUBID_P2P_MANAGEABILITY:
						if (pTotalNumOfP2pAttribute != NULL)
						{
							*pTotalNumOfP2pAttribute = *pTotalNumOfP2pAttribute+1;
						DBGPRINT(RT_DEBUG_TRACE, ("SYNC -Ap Has P2P Manageability IE . Total P2P IE count is %d \n", *pTotalNumOfP2pAttribute));
						}
						if (pMamageablity != NULL)
							*pMamageablity = pP2pEid->Octet[0];
						break;
					default:
							if (pTotalNumOfP2pAttribute != NULL)
								*pTotalNumOfP2pAttribute = *pTotalNumOfP2pAttribute+1;
							if (pNumOfP2pOtherAttribute != NULL)
								*pNumOfP2pOtherAttribute = *pNumOfP2pOtherAttribute+1;
						break;
						
				}
				Length = Length + 3 + AttriLen;  /* Eid[1] + Len[1]+ content[Len] */
				pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
				AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;
			}

		}
		LeftLength = LeftLength - pEid->Len - 2;
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	}

}

/*	
	==========================================================================
	Description: 
		Parse P2P subelement content to fill into the correct output buffer. May contain multiple P2P IE and WPS IE.
		So need while loop to find all IE.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pParseSubElmt(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	IN BOOLEAN  bBeacon, 
	OUT USHORT *pDpid,
	OUT UCHAR *pGroupCap,
	OUT UCHAR *pDeviceCap,
	OUT UCHAR *pDeviceName,
	OUT UCHAR *pDeviceNameLen,
	OUT UCHAR *pDevAddr,
	OUT UCHAR *pInterFAddr,
	OUT UCHAR *pBssidAddr,
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pSsid,
	OUT USHORT *pConfigMethod,
	OUT USHORT *pWpsConfigMethod,
	OUT UCHAR *pDevType,
	OUT UCHAR *pListenChannel,
	OUT UCHAR *pOpChannel,
	OUT UCHAR *pChannelList,
	OUT UCHAR *pIntent,
	OUT UCHAR *pStatusCode,
	OUT UCHAR *pInviteFlag) 
{
	ULONG				Length = 0;
	PP2PEID_STRUCT	pP2pEid;
	ULONG			AttriLen;
	USHORT 			WscType, WscLen;
	UCHAR			offset;
	PEID_STRUCT		pEid;
	UCHAR			ChannelListLeft = MAX_NUM_OF_CHANNELS;
	/*USHORT			ExtTime; */
	ULONG			AccuP2PIELen;
	ULONG			AccuIeLen = 0;
	
	/* Intel sends multiple P2P IE... So I can't give each input a default value.. */
	if (MsgLen == 0)
		return;

	pEid = (PEID_STRUCT)Msg;
	AccuIeLen = pEid->Len + 2; /* 2: Tag (1 octet) + Len (1 octet) */
	if (RTMPEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
	{
		/*
			In this case, length is 2 bytes.
		*/
		AccuIeLen = pEid->Len + pEid->Octet[0]*256 + 3; /* 3: Tag (1 octet) + Len (2 octet) */
	}

	while ((ULONG)(AccuIeLen) <= MsgLen)
	{
		if (RTMPEqualMemory(&pEid->Octet[0], WPS_OUI, 4))
		{
			/*PUCHAR pWscEid = &pEid->Eid;*/
			if (bBeacon == TRUE)
				P2PParseWPSIE(&pEid->Octet, (pEid->Len + 2), pDpid, pWpsConfigMethod, pDeviceName, pDeviceNameLen);
			else
				P2PParseWPSIE(&pEid->Octet, (pEid->Len + 2), pDpid, pWpsConfigMethod, NULL, NULL);
		}
		/* might contains P2P IE and WPS IE.  So use if else if enough for locate  P2P IE. */
		else if ((RTMPEqualMemory(&pEid->Octet[0], WIFIDIRECT_OUI, 4))
			||(RTMPEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4)))
		{
			/* Get Request content capability */
			if (RTMPEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
			{
				/*
					In this case, length is 2 bytes.
				*/
				pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[5];
				AccuP2PIELen = pEid->Len + pEid->Octet[0]*256;
			}
			else
			{
				pP2pEid = (PP2PEID_STRUCT) &pEid->Octet[4];
				AccuP2PIELen = pEid->Len;
			}
			/*
				The value of AccuP2PIELen shall reduce the length of OUI (4).
			*/
			AccuP2PIELen -= 4; 

			AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] *8;

			Length = 0;

				while ((Length + 3 + AttriLen) <= AccuP2PIELen)    
				{
					switch(pP2pEid->Eid)
					{						
						case SUBID_P2P_EXT_LISTEN_TIMING:
							break;
						case SUBID_P2P_INVITE_FLAG:
							if (pInviteFlag != NULL)
								*pInviteFlag = pP2pEid->Octet[0];
							break;
						case SUBID_P2P_MANAGEABILITY:
							break;
						case SUBID_P2P_CAP:
							*pGroupCap = pP2pEid->Octet[1]; 
							*pDeviceCap = pP2pEid->Octet[0]; 
							break;
						case SUBID_P2P_OWNER_INTENT:
							if (pIntent != NULL)
								*pIntent = pP2pEid->Octet[0];
							break;
						case SUBID_P2P_CHANNEL_LIST:
							DBGPRINT(RT_DEBUG_INFO, ("SUBID_P2P_CHANNEL_LIST - ( Len %d %d)= %x %x %x %x %x %x..\n",  pP2pEid->Len[0], pP2pEid->Len[1], pP2pEid->Octet[0],pP2pEid->Octet[1],pP2pEid->Octet[2],pP2pEid->Octet[3],pP2pEid->Octet[4],pP2pEid->Octet[5]));
							if ((pChannelList != NULL) && (ChannelListLeft >= pP2pEid->Octet[4]))
							{
								ChannelListLeft -= pP2pEid->Octet[4];
								RTMPMoveMemory(pChannelList, &pP2pEid->Octet[5], pP2pEid->Octet[4]);
								pChannelList += pP2pEid->Octet[4];
							}
							break;
						case SUBID_P2P_OP_CHANNEL:
							if (pOpChannel != NULL)
								*pOpChannel = pP2pEid->Octet[4];	/* Octet[1] is regulatory  */
							break;
						case SUBID_P2P_LISTEN_CHANNEL:
							if (pListenChannel != NULL)
								*pListenChannel = pP2pEid->Octet[4];	/* Octet[1] is regulatory  */
							break;
						case SUBID_P2P_GROUP_BSSID:	/* 6 group Bssid */
							if (pBssidAddr != NULL)
								RTMPMoveMemory(pBssidAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
							break;
						case SUBID_P2P_INTERFACE_ADDR:	/* 9 */
							if (pInterFAddr != NULL)
							{
								RTMPMoveMemory(pInterFAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
							}
							break;
						case SUBID_P2P_DEVICE_ID:
							/* Beacon has this field. */
							if (pDevAddr != NULL)
								RTMPMoveMemory(pDevAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
							break;
						case SUBID_P2P_GROUP_ID:
							if ((pSsid != NULL) && (pP2pEid->Len[0]) > 6 && (pP2pEid->Len[0] <= 38))
							{
								RTMPMoveMemory(pSsid, &pP2pEid->Octet[6], (pP2pEid->Len[0] - 6));
								if (pSsidLen != NULL)
								{
									*pSsidLen = pP2pEid->Len[0] - 6;
								DBGPRINT(RT_DEBUG_INFO, (" SUBID_P2P_GROUP_ID - SSID ( Len %d)= %c %c %c %c %c %c %c %c %c.. \n",  *pSsidLen, pSsid[0], pSsid[1],pSsid[2],pSsid[3],pSsid[4],pSsid[5],pSsid[6],pSsid[7],pSsid[8]));
							}
							}
							break;
						case SUBID_P2P_DEVICE_INFO:
							if (pDevAddr != NULL)
								RTMPMoveMemory(pDevAddr, &pP2pEid->Octet[0], MAC_ADDR_LEN);
							if (pConfigMethod != NULL)
								*pConfigMethod = be2cpu16(*(PUSHORT)&pP2pEid->Octet[6]);
							if (pDevType != NULL)
								RTMPMoveMemory(pDevType, &pP2pEid->Octet[8], 8);
							/* Count the DeviceName offset. */
							offset = 17 + pP2pEid->Octet[16] * 8;
							WscType = cpu2be16(*((PUSHORT) &pP2pEid->Octet[offset]));
							WscLen  = cpu2be16(*((PUSHORT) (&pP2pEid->Octet[offset+2])));
							if ((WscType == WSC_ID_DEVICE_NAME) && (WscLen <= 32))
							{
								if ((pDeviceName != NULL))
								{
									*pDeviceNameLen = (UCHAR)WscLen;
									RTMPMoveMemory(pDeviceName, &pP2pEid->Octet[21 + pP2pEid->Octet[16]*8], WscLen);
									DBGPRINT(RT_DEBUG_INFO, ("SUBID_P2P_DEVICE_INFO Device Name= %c %c %c %c %c %c \n",  pDeviceName[0], pDeviceName[1],pDeviceName[2],pDeviceName[3],pDeviceName[4],pDeviceName[5]));
								}
							}

							break;
						case SUBID_P2P_STATUS:
							if (pStatusCode != NULL)
							{
								*pStatusCode = pP2pEid->Octet[0];
								DBGPRINT(RT_DEBUG_INFO, (" SUBID_P2P_STATUS    Eid = %x \n", *pStatusCode));
							}
							
							break;
						case SUBID_P2P_GROUP_INFO:
							
							break;
						case SUBID_P2P_NOA:
							break;
						case SUBID_P2P_CONFIG_TIMEOUT:
							break;

						default:
							DBGPRINT(RT_DEBUG_ERROR, (" SUBID_P2P_ unknown  Eid = %x \n", pP2pEid->Eid));
							break;
							
					}
					Length = Length + 3 + AttriLen;  /* Eid[1] + Len[1]+ content[Len] */
					if (Length >= AccuP2PIELen)
						break;

					pP2pEid = (PP2PEID_STRUCT)((UCHAR*)pP2pEid + 3 + AttriLen);        
					AttriLen = pP2pEid->Len[0] + pP2pEid->Len[1] * 256;
				}

		}
		/* already reach the last IE. Stop finding next Eid. */
		if (AccuIeLen >= MsgLen)
			break;

		/* Forward buffer to next pEid */
		if (RTMPEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
		{
			pEid = (PEID_STRUCT)((UCHAR*)pEid + (pEid->Len + pEid->Octet[0]*256 + 3));    
			/* We already accumul ate all P2P IE. don't need to search next P2P IE */
			break;
		}
		else
		{
			pEid = (PEID_STRUCT)((UCHAR*)pEid + pEid->Len + 2);    
		}
		
		/* Since we get the next pEid,  */
		/* Predict the accumulated IeLen after adding the next pEid's length.	 */
		/* The accumulated IeLen is for checking length. */
		if (RTMPEqualMemory(&pEid->Octet[1], WIFIDIRECT_OUI, 4))
		{
			AccuIeLen += (pEid->Len + pEid->Octet[0]*256 + 3);
		}
		else
		{
			AccuIeLen += (pEid->Len + 2);
		}

	}

}

/*	
	==========================================================================
	Description: 
		The routine that is called when receiving Go Negociation Confirm packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceGoNegoConfirmAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
	UCHAR		DevType[8] = {0}, DevAddr[6] = {0}, IfAddr[6] = {0};
	UCHAR 		Channel, OpChannel = 0, Intent, index, Ssid[32];
	UCHAR		GroupCap, DeviceCap, DeviceNameLen;
 	/*ULONG		FrameLen; */
 	/*ULONG		TempLen; */
	UCHAR		StatusCode, SsidLen = 0;
	/*PEID_STRUCT	pEid; */
	/*ULONG		LeftLength; */
	USHORT		Dpid, ConfigMethod;
	UCHAR       AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	
	DBGPRINT(RT_DEBUG_ERROR, (" P2P - Recieve Confirm Confirm Confirm. -> From %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pFrame->p80211Header.Addr2)));

	/* Get Request content capability */
	P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen + 7 - sizeof(P2P_PUBLIC_FRAME)), 
		FALSE, &Dpid, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, DevAddr, IfAddr, NULL, &SsidLen, (PUCHAR)&Ssid, &ConfigMethod, NULL, DevType, &Channel, &OpChannel, NULL, &Intent, &StatusCode, NULL);
	/* confirm doesn't attach device addr in subelement. So copy from SA. */
	RTMPMoveMemory(DevAddr, pFrame->p80211Header.Addr2, MAC_ADDR_LEN);
	DBGPRINT(RT_DEBUG_ERROR, (" DevAddr = %02x:%02x:%02x:%02x:%02x:%02x.  \n",  PRINT_MAC(DevAddr)));
	DBGPRINT(RT_DEBUG_ERROR, (" OpChannel = %d.   \n",  OpChannel));
	
	/* Check StatusCode. */
	if (StatusCode != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - Ignore Go Negociation Confirm Status Code not Success. \n"));
		return;
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - Recieve Confirm Confirm Confirm Success. \n"));


 	/* Check Peer is in the valid state to receive Go Negociation Response. */
	index = P2pGroupTabSearch(pAd, DevAddr);
	if (index == P2P_NOT_FOUND)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - Ignore Go Negociation Confirm from Unknown device. \n"));
		P2PPrintP2PEntry(pAd, 0);
		return;
	}
	else 
	{
		/* Existing peer stay in another state. Doesn't need respond the Go Negociation Request. */
		if ((pP2PCtrl->P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) && (pAd->P2pTable.Client[index].P2pClientState != P2PSTATE_WAIT_GO_COMFIRM))
		{
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - Existing peer stay in another state. = %d. return.\n", pAd->P2pTable.Client[index].P2pClientState));
			return;
		}
	}

	if (index < MAX_P2P_GROUP_SIZE)
	{
		PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAd->P2pTable.Client[index];
		if (!NdisEqualMemory(AllZero, IfAddr, MAC_ADDR_LEN))
			RTMPMoveMemory(pP2pEntry->InterfaceAddr, IfAddr, MAC_ADDR_LEN);
		DBGPRINT(RT_DEBUG_ERROR, (" pP2pEntry[%d]->rule = %s. GoIntent = %d. My intent is %d. \n",  index, decodeMyRule(pP2pEntry->Rule), pP2pEntry->GoIntent, pP2PCtrl->GoIntentIdx));
		/*DBGPRINT(RT_DEBUG_ERROR, ("---->P2P pP2PCtrl->SsidLength = %d. [%x %x %x..]\n", pP2PCtrl->SSIDLen, 
								pP2PCtrl->SSID[0], pP2PCtrl->SSID[1], pP2PCtrl->SSID[2]));*/
		if (SsidLen > 0)
		{
			RTMPMoveMemory(pP2pEntry->Ssid, Ssid, 32);
			pP2pEntry->SsidLen = SsidLen;
		}

		if (pP2PCtrl->P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT)
		{	
			int i;
			/* I become Client */
			pP2pEntry->Rule = P2P_IS_GO;
			pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
			pP2PCtrl->GroupChannel = OpChannel;
			pP2PCtrl->GroupOpChannel = OpChannel;
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become ENROLLEE!GOGO Go's SsidLen = %d.!!\n", pP2pEntry->SsidLen));
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become ENROLLEE!GOGO Go's Ssid[%d] = ", pP2pEntry->SsidLen));	
			for (i=0; i<pP2pEntry->SsidLen; i++)
				DBGPRINT(RT_DEBUG_ERROR, ("%c ", pP2pEntry->Ssid[i]));
			DBGPRINT(RT_DEBUG_ERROR, ("\n"));
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become ENROLLEE!!GOGO Go's Bssid = %02x:%02x:%02x:%02x:%02x:%02x!!\n", PRINT_MAC(pP2PCtrl->Bssid)));
			RTMPZeroMemory(pP2PCtrl->SSID, 32);
			RTMPMoveMemory(pP2PCtrl->SSID, pP2pEntry->Ssid, pP2pEntry->SsidLen);
			pP2PCtrl->SSIDLen = pP2pEntry->SsidLen;
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become ENROLLEE!!GOGO Go's SSID[%d] = %s!!\n", pP2PCtrl->SSIDLen, pP2PCtrl->SSID));
			P2pGoNegoDone(pAd, pP2pEntry);
		}
		else
		{
			/* I become GO */
			pP2pEntry->Rule = P2P_IS_CLIENT;
			pP2PCtrl->GroupOpChannel = pP2PCtrl->GroupChannel;
			pP2pEntry->P2pClientState = P2PSTATE_GOT_GO_COMFIRM;
			P2pGoNegoDone(pAd,  pP2pEntry);
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
		}

	}

}
/*	
	==========================================================================
	Description: 
		The routine that is called when receiving Go Negociation Response packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceGoNegoRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)&Elem->Msg[0];
	UCHAR		DevType[8], Ssid[32], DevAddr[6], IfAddr[6] = {0}, Channel = 0, OpChannel = 0, Intent, index;
	UCHAR		GroupCap, DeviceCap, DeviceName[32];
	UCHAR		SsidLen = 0, DeviceNameLen = 0;
	PRT_P2P_CLIENT_ENTRY pP2pEntry;
	UCHAR		StatusCode = 8;
	UCHAR		TempIntent;
	USHORT		Dpid, ConfigMethod;
	BOOLEAN		Cancelled;
	UCHAR       AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	
	DBGPRINT(RT_DEBUG_ERROR, (" P2P -  GOGOGO Recieve Response.Response MsgLen = %ld.\n", Elem->MsgLen));
	if (pP2PCtrl->bP2pReSendTimerRunning)
	{
		pP2PCtrl->bP2pReSendTimerRunning = FALSE;
		RTMPCancelTimer(&pP2PCtrl->P2pReSendTimer, &Cancelled);
	}
	/* Get Request content capability */
	P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8),  
		FALSE, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, 
		DevAddr, IfAddr, NULL, &SsidLen, Ssid, &ConfigMethod, NULL, DevType, &Channel, &OpChannel, NULL, &TempIntent, &StatusCode, NULL);

	Intent = TempIntent >>1;
	DBGPRINT(RT_DEBUG_ERROR, (" Dev Addr = %02x:%02x:%02x:%02x:%02x:%02x. Intent = %d. My Intent = %d. \n",  PRINT_MAC(DevAddr), Intent, pP2PCtrl->GoIntentIdx));
	DBGPRINT(RT_DEBUG_ERROR, (" interface addr = %02x:%02x:%02x:%02x:%02x:%02x  \n",  PRINT_MAC(IfAddr)));

	DBGPRINT(RT_DEBUG_ERROR, (" P2P -StatusCode = %d. PeerIntent = %d . OpChannel = %d. My Intent = %d.\n", StatusCode, Intent, OpChannel, pP2PCtrl->GoIntentIdx));
	
	/* Check StatusCode. */
	if (StatusCode == P2PSTATUS_BOTH_INTENT15)
	{
		pP2PCtrl->P2pCounter.DisableRetryGrpFormCounter = 1200;
		DBGPRINT(RT_DEBUG_TRACE, (" P2P - Receive Status Code that both Go Intent Value is 15.  = %x \n", StatusCode));
		P2pLinkDown(pAd, P2P_CONNECT_FAIL);
		return;
	}
	else if ((StatusCode == P2PSTATUS_REJECT_BY_USER) || (StatusCode == P2PSTATUS_PASSED))
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P -Go Negociation Response Status Code PASSED or being rejected  = %d \n", StatusCode));
		index = P2pGroupTabSearch(pAd, DevAddr);
		if (index < MAX_P2P_GROUP_SIZE)
		{
			pAd->P2pTable.Client[index].ReTransmitCnt = 0;
			/* since peer return that info unknown, then stop resend GO Negociation Req. */
			/* doesn't need to prepare for retry GO Nego REQ. */
			/*pAd->P2pTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY; */
			/* Test Plan 5.1.22 */
			P2pStopConnectThis(pAd);
			/* Reset Peer State as Default. */
			pAd->P2pTable.Client[index].P2pClientState = P2PSTATE_DISCOVERY_CLIENT;
			/* Turn Back to My Listen Channel. */
			P2pStopScan(pAd);
			/* Back to LISTEN State. */
			MlmeEnqueue(pAd, P2P_DISC_STATE_MACHINE, P2P_DISC_CANL_CMD_EVT, 0, NULL, 0);
			DBGPRINT(RT_DEBUG_ERROR, (" P2P -  = %s \n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
		}

		Channel = pAd->P2pCfg.ListenChannel;
		AsicSwitchChannel(pAd, Channel, FALSE);
		AsicLockChannel(pAd, Channel);

		return;
	}
	else if (StatusCode != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - Ignore Go Negociation Response Status Code not Success.  = %d \n", StatusCode));
		return;
	}

 	/* Check Peer is in the valid state to receive Go Negociation Response. */
	index = P2pGroupTabSearch(pAd, DevAddr);
	if (index == P2P_NOT_FOUND)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - Ignore Go Negociation Response from Unknown device. \n"));
		return;
	}
	else 
	{

	}

	if (index < MAX_P2P_GROUP_SIZE)
	{
		pP2pEntry = &pAd->P2pTable.Client[index];
		pP2pEntry->ConfigMethod = ConfigMethod;
		/* Check peer capability */
		pP2pEntry->ListenChannel = Channel;
		if (OpChannel != 0)
		pP2pEntry->OpChannel = OpChannel;
		pP2pEntry->GoIntent = Intent;
		pP2pEntry->StateCount = 0;
		pP2pEntry->bValid = FALSE;
		pP2pEntry->Dpid = Dpid;
		if (!NdisEqualMemory(AllZero, IfAddr, MAC_ADDR_LEN))
		RTMPMoveMemory(pP2pEntry->InterfaceAddr, IfAddr, MAC_ADDR_LEN);

		if (SsidLen > 0)
		{
			RTMPMoveMemory(pP2pEntry->Ssid, Ssid, SsidLen);
			pP2pEntry->SsidLen = SsidLen;
		}
		P2PPrintP2PEntry(pAd, index);
		/* If this peer is provistioned, dpid should follows spec's assignment on page 33 */
		if (P2P_TEST_FLAG(pP2pEntry, P2PFLAG_PROVISIONED))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("P2P provisioned -dpid= %x. ConfigMethod = %s.\n", Dpid, decodeConfigMethod(pP2pEntry->ConfigMethod)));
			switch(pP2pEntry->ConfigMethod)
			{
				case WSC_CONFMET_DISPLAY:
					if ((Dpid != DEV_PASS_ID_REG) && (Dpid != DEV_PASS_ID_PIN))
					{
						DBGPRINT(RT_DEBUG_ERROR, (" P2P -1 Ignore Go Negociation Response with wrong dpid \n"));
						return;
					}
					break;
				case WSC_CONFMET_KEYPAD:
					if (Dpid != DEV_PASS_ID_USER)
					{
						DBGPRINT(RT_DEBUG_ERROR, (" P2P -3 Ignore Go Negociation Response with wrong dpid \n"));
						return;
					}
					break;
				default:
					break;
			}
		}
		P2pSetRule(pAd, index, IfAddr, TempIntent, OpChannel);
		
		if ((pP2PCtrl->P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) && (pAd->P2pTable.Client[index].P2pClientState != P2PSTATE_SENT_GO_NEG_REQ))
		{
			DBGPRINT(RT_DEBUG_ERROR, (" P2P -  Existing peer stay in another state. = %d. \n", pAd->P2pTable.Client[index].P2pClientState));
			return;
		}
		P2PSendGoNegoConfirm(pAd, pFrame->Token, index, pP2pEntry->addr);
	}

}

/*	
	==========================================================================
	Description: 
		The routine that is called when receiving Go Negociation Request packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceGoNegoReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
		PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
		PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
		UCHAR		DevType[8] = {0}, DevAddr[6] = {0}, BssidAddr[6] = {0};
		UCHAR		Channel, OpChannel, Intent, index, StatusCode;
		UCHAR		GroupCap, DeviceCap, DeviceName[32], DeviceNameLen;
		ULONG		FrameLen;
		ULONG		TempLen;
		UCHAR		SsidLen = 0;
		RT_P2P_CLIENT_ENTRY *pP2pEntry;
		NDIS_STATUS   NStatus;
		PUCHAR			pOutBuffer = NULL;
		PUCHAR		pDest;
		UCHAR		RspStatus = P2PSTATUS_SUCCESS;
		UCHAR		TempIntent;
		/*PUCHAR		pP2pIE; */
		USHORT		Dpid, SentDpid, ConfigMethod;
		BOOLEAN		bSendEvent = FALSE;
		UCHAR       AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
		BOOLEAN		Cancelled;
	
		DBGPRINT(RT_DEBUG_ERROR, ("GOGOGOGO P2P - receive Go Neg Request. MsgLen = %ld \n", Elem->MsgLen));

		RTMPZeroMemory(&DeviceName[0], 32);
		pP2pEntry = NULL;
		if (IS_P2P_DEVICE_DISCOVERING(pAd))
		P2pStopScan(pAd);
		/* Get Request content capability */
		P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8),
			FALSE, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, 
			BssidAddr, NULL, &SsidLen, NULL, &ConfigMethod, NULL, DevType, &Channel, &OpChannel, NULL, &TempIntent, &StatusCode, NULL);

		Intent = TempIntent >>1;
		DBGPRINT(RT_DEBUG_TRACE, ("    DevAddr in P2P IE = %02x %02x %02x %02x %02x %02x  \n",  PRINT_MAC(DevAddr)));
		DBGPRINT(RT_DEBUG_TRACE, ("    Addr2 = %02x %02x %02x %02x %02x %02x  \n",  PRINT_MAC(pFrame->p80211Header.Addr2)));
		DBGPRINT(RT_DEBUG_TRACE, ("    its bssid = %02x %02x %02x %02x %02x %02x  \n",  PRINT_MAC(BssidAddr)));
		DBGPRINT(RT_DEBUG_TRACE, ("    Dpid = %x ,%s \n", Dpid, decodeDpid(Dpid)));
	
		index = P2pGroupTabSearch(pAd, DevAddr);

		if ((index != P2P_NOT_FOUND) && (pP2PCtrl->bP2pReSendTimerRunning))
		{
			pP2PCtrl->bP2pReSendTimerRunning = FALSE;
			pAd->P2pTable.Client[index].ReTransmitCnt = 0;
			RTMPCancelTimer(&pP2PCtrl->P2pReSendTimer, &Cancelled);
		}
 
		DBGPRINT(RT_DEBUG_ERROR, ("P2P -Peer[%d] Intent = %x . OpChannel = %x. My Intent = %x. TempIntent = %x\n", index, Intent, Channel, pP2PCtrl->GoIntentIdx, TempIntent));
		if (index == P2P_NOT_FOUND)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("P2P -1  insert\n"));
			index = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY, NULL, 0, DeviceCap, GroupCap);
			if (index < MAX_P2P_GROUP_SIZE)
			{
				pP2PCtrl->PopUpIndex = index;
				RTMPMoveMemory(pAd->P2pTable.Client[index].DeviceName, DeviceName, 32);
				DBGPRINT(RT_DEBUG_ERROR, ("From a unknown peer. Pop up setting windows. %d\n", pP2PCtrl->PopUpIndex));
				pAd->P2pTable.Client[index].DeviceNameLen = DeviceNameLen;
				pAd->P2pTable.Client[index].DeviceName[pAd->P2pTable.Client[index].DeviceNameLen] = 0x0;

				pP2pEntry = &pAd->P2pTable.Client[index];
				pP2pEntry->ConfigMethod = ConfigMethod;
			}
			bSendEvent = TRUE;
		}
		
		if (index < MAX_P2P_GROUP_SIZE)
		{
			pP2pEntry = &pAd->P2pTable.Client[index];
			pP2pEntry->ConfigMethod = ConfigMethod;
			if (!NdisEqualMemory(AllZero, BssidAddr, MAC_ADDR_LEN))
				RTMPMoveMemory(pP2pEntry->InterfaceAddr, BssidAddr, MAC_ADDR_LEN);
			P2PPrintP2PEntry(pAd, index);

			DBGPRINT(RT_DEBUG_TRACE, ("My P2P Dpid = %x.    State = %s. \n", pP2PCtrl->Dpid, decodeP2PClientState(pP2pEntry->P2pClientState)));
			if (IS_P2P_PEER_CLIENT_OP(pP2pEntry) || IS_P2P_PEER_WPAPSK(pP2pEntry) || IS_P2P_PEER_PROVISIONING(pP2pEntry))
			{
				/* Receive Go Neg Req when this peer's state is operating or doing provisioning. Delete this peer. */
				DBGPRINT(RT_DEBUG_TRACE, (" P2P - Existing peer  state is %d. %s,.	Delete it.\n", pP2pEntry->P2pClientState, decodeP2PClientState(pP2pEntry->P2pClientState)));
	
				return;
			}
			if (pP2pEntry->P2pClientState > P2PSTATE_GO_DONE)
			{
				DBGPRINT(RT_DEBUG_TRACE, (" P2P - Existing peer  state is %d . %s,. return.\n", pP2pEntry->P2pClientState, decodeP2PClientState(pP2pEntry->P2pClientState)));
				return;
			}
			else if (pP2pEntry->P2pClientState == P2PSTATE_SENT_GO_NEG_REQ)
			{
				if (IsP2pFirstMacSmaller(pP2pEntry->addr, pP2PCtrl->CurrentAddress))
				{
					DBGPRINT(RT_DEBUG_TRACE, (" P2P - Dual GO Req. Existing peer  state is %s. \n", decodeP2PClientState(pP2pEntry->P2pClientState)));
					return;
				}
			}
			if (P2P_GO_ON(pAd))
			{
				/* I am GO . Don't need go through GO NEgo process. So return fail. */
				RspStatus = P2PSTATUS_INVALID_PARM;
			}
			else if (pP2PCtrl->Dpid == DEV_PASS_ID_NOSPEC)
			{
				/* Test Plan 5.1.21 */
				RspStatus = P2PSTATUS_PASSED;
			}
		}
		else
			return;
				
		/* Ban Request from the same GoIntent index. */
		if ((Intent == pP2PCtrl->GoIntentIdx) && (Intent == 15))
		{
			RspStatus = P2PSTATUS_BOTH_INTENT15;
			DBGPRINT(RT_DEBUG_ERROR, (" P2pReceGoNegoReqAction -Receive a peer that has the same Go Intent index as mine. \n"));
		}	
			
		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
		if (NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR, (" MlmeAllocateMemory	fail -  . \n"));
			return;
		}
	
		pDest = pOutBuffer;
		if (RspStatus == P2PSTATUS_SUCCESS)
		{
			pP2pEntry->P2pClientState = P2PSTATE_WAIT_GO_COMFIRM;
			pP2pEntry->StateCount = 0;
			pP2pEntry->bValid = FALSE;
			pP2pEntry->Dpid = Dpid;
			P2pSetRule(pAd, index, BssidAddr, TempIntent, OpChannel);
			/* If I am Go, Use my channel to set in the Go Nego Rsp. */
			if (pP2pEntry->Rule == P2P_IS_CLIENT)
				OpChannel = pP2PCtrl->GroupChannel;
			/* Change beacon content */
			pP2PCtrl->P2pCapability[1] |= (GRPCAP_GROUP_FORMING);
			P2pUpdateBssBeacon(pAd, pP2PCtrl->P2pCapability, NULL);
		}
				
		/*SentDpid = DEV_PASS_ID_PIN; */
		if (P2P_TEST_FLAG(pP2pEntry, P2PFLAG_PROVISIONED))
		{
			DBGPRINT(RT_DEBUG_ERROR, (" P2P -	Go Negociation Responce to provisioned	\n"));
			if (pP2PCtrl->ConfigMethod == WSC_CONFMET_DISPLAY)
				SentDpid = DEV_PASS_ID_REG;
			else if (pP2PCtrl->ConfigMethod == WSC_CONFMET_LABEL)
				SentDpid = DEV_PASS_ID_PIN;
			else if  (pP2PCtrl->ConfigMethod == WSC_CONFMET_KEYPAD)
			{
				if (pP2PCtrl->bSigmaEnabled == FALSE)
					RspStatus = P2PSTATUS_PASSED;
				SentDpid = DEV_PASS_ID_USER;
			}
			else if (pP2PCtrl->ConfigMethod == WSC_CONFMET_PBC)
				SentDpid = DEV_PASS_ID_PBC;
		}
		else
		{
			if (Dpid == DEV_PASS_ID_PIN)
				SentDpid = DEV_PASS_ID_USER;
			else if (Dpid == DEV_PASS_ID_USER)
				SentDpid = DEV_PASS_ID_PIN;
			else if  (Dpid == DEV_PASS_ID_REG)
			{
				if (pP2PCtrl->bSigmaEnabled == FALSE)
					RspStatus = P2PSTATUS_PASSED;
				SentDpid = DEV_PASS_ID_USER;
			}
			else if (Dpid == DEV_PASS_ID_PBC)
				SentDpid = DEV_PASS_ID_PBC;
			else
				DBGPRINT(RT_DEBUG_ERROR, ("Peer Go Nego Req DPID = %x\n", Dpid));
		}

		/* Save  peer capability */
		pP2pEntry->ListenChannel = Channel;
		pP2pEntry->OpChannel = OpChannel;
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
		if (bSendEvent)
			P2pSendWirelessEvent(pAd, RT_P2P_DEVICE_FIND, &pAd->P2pTable.Client[index], pFrame->p80211Header.Addr2);
		P2pSendWirelessEvent(pAd, RT_P2P_RECV_GO_NEGO_REQ, &pAd->P2pTable.Client[index], pFrame->p80211Header.Addr2);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
		/*P2P_SetWscRule(pAd, SentDpid); */
		SentDpid = pP2PCtrl->Dpid;
		DBGPRINT(RT_DEBUG_TRACE, (" P2P - ReceDpid = %x. SentDpid %x \n", (Dpid), (SentDpid)));
		DBGPRINT(RT_DEBUG_TRACE, (" P2P - ReceDpid = %s. SentDpid %s \n", decodeDpid(Dpid), decodeDpid(SentDpid)));
		P2PMakeGoNegoRsp(pAd, DevAddr, SentDpid, pFrame->Token, TempIntent, OpChannel, RspStatus, pOutBuffer, &TempLen);
		FrameLen = TempLen;
		pDest += TempLen;
		/* : copy req to rsp first. */
	
		DBGPRINT(RT_DEBUG_TRACE, (" P2P - Make Go Negociation Responce Length = %ld.RspStatus = %d \n", FrameLen, RspStatus));
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);


		if (RspStatus == P2PSTATUS_PASSED)
		{
			P2P_CMD_STRUCT	P2pCmd;

			pP2pEntry->Dpid = Dpid;
			P2pSetRule(pAd, index, BssidAddr, TempIntent, OpChannel);
			/*UCHAR ClientState = pAd->P2pTable.Client[index].P2pClientState;*/
			COPY_MAC_ADDR(&P2pCmd.Addr[0], pAd->P2pTable.Client[index].addr);
			P2pCmd.Idx = index;
			if (pP2PCtrl->ConfigMethod != WSC_CONFMET_KEYPAD)
			MlmeEnqueue(pAd, P2P_GO_FORM_STATE_MACHINE, P2P_SEND_PASSED_CMD_EVT, sizeof(P2P_CMD_STRUCT), &P2pCmd, 0);

		}

		/*P2PPrintP2PEntry(pAd, index); */
		/* else ignore this Go Negociation Request because P2P table reach maximum. */

}

/*	
	==========================================================================
	Description: 
		The routine that is called when receiving Client Discovery Request Public Action Frame packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
UCHAR DevDiscCnt = 0;
VOID P2pReceDevDisReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
	UCHAR		devAddr[6], BssidAddr[6], OpChannel, StatusCode;
	UCHAR		GroupCap, DeviceCap, DeviceNameLen;
	UCHAR		SsidLen;
	UCHAR		p2pClientIndex;
	RT_P2P_CLIENT_ENTRY 	*pEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	ULONG			TotalFrameLen;

	DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2pReceDevDisReqAction = %ld\n", Elem->MsgLen));

	/* stop scan and lock at Listen Channel. */
	if ((!P2P_GO_ON(pAd)) && (!P2P_CLI_ON(pAd)))
	{
		P2pStopScan(pAd);
		AsicSwitchChannel(pAd, pAd->P2pCfg.ListenChannel, FALSE);
		AsicLockChannel(pAd, pAd->P2pCfg.ListenChannel);
	}

	/* Get Request content capability */
	P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
		FALSE, NULL, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, devAddr, NULL, 
		BssidAddr, &SsidLen, NULL, NULL, NULL, NULL, NULL, &OpChannel, NULL, NULL, &StatusCode, NULL);
	/* Use TX to decide who is sending. */
	p2pClientIndex = P2pGroupTabSearch(pAd, devAddr);
	if (p2pClientIndex < MAX_P2P_GROUP_SIZE)
	{
		pEntry = &pAd->P2pTable.Client[p2pClientIndex];
		pMacEntry = MacTableLookup(pAd, pEntry->InterfaceAddr);
	}
	DBGPRINT(RT_DEBUG_ERROR, ("P2P - ask for p2p client[%d] = %02x:%02x:%02x:%02x:%02x:%02x.\n", p2pClientIndex, PRINT_MAC(devAddr)));

	if (((pMacEntry != NULL) && (pEntry != NULL)
		&& (pEntry->P2pClientState == P2PSTATE_CLIENT_WPS_DONE)))
	{
		MLME_P2P_ACTION_STRUCT	P2PActReq;	


		NdisZeroMemory(&P2PActReq, sizeof(P2PActReq));
		DBGPRINT(RT_DEBUG_TRACE, ("to interface Addr = %02x:%02x:%02x:%02x:%02x:%02x.\n",  
					PRINT_MAC(pEntry->InterfaceAddr)));
		COPY_MAC_ADDR(P2PActReq.Addr, pEntry->InterfaceAddr);
		P2PActReq.TabIndex = p2pClientIndex;
		MlmeEnqueue(pAd, P2P_ACTION_STATE_MACHINE, MT2_MLME_P2P_GO_DIS_REQ, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq, 0);
		MlmeHandler(pAd);
		DevDiscCnt++;

		/* Device Discvoery  Response is delayed until I get the GO Discovery Request Frame's Ack. */
		/* Assume always success.  Send back response. */
		RTMPMoveMemory(&pAd->P2pCfg.LatestP2pPublicFrame, pFrame, sizeof(P2P_SAVED_PUBLIC_FRAME));
		OS_WAIT(500);
		P2PSendDevDisRsp(pAd, P2PSTATUS_SUCCESS, pAd->P2pCfg.LatestP2pPublicFrame.Token, pAd->P2pCfg.LatestP2pPublicFrame.p80211Header.Addr2, &TotalFrameLen);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s:: P2P -can't find p2p client .Status fail", __FUNCTION__));
		P2PSendDevDisRsp(pAd, P2PSTATUS_IMCOMPA_PARM, pFrame->Token, pFrame->p80211Header.Addr2, &TotalFrameLen);
	}
}


/*	
	==========================================================================
	Description: 
		The routine that is called when receiving Client Discovery Response Public Action Frame packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceDevDisRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
	UCHAR		DevType[8], DevAddr[6], BssidAddr[6], Channel, OpChannel, index, StatusCode;
	UCHAR		GroupCap, DeviceCap, DeviceNameLen, TempIntent;
	UCHAR		SsidLen = 0;
	RT_P2P_CLIENT_ENTRY *pP2pEntry;
	USHORT		Dpid, ConfigMethod;
	UCHAR		ClientP2PIndex = P2P_NOT_FOUND, i;


	DBGPRINT(RT_DEBUG_ERROR, ("GOGO  P2P - P2pReceDevDisRspAction MsgLen = %ld ", Elem->MsgLen));

	pP2pEntry = NULL;

	index = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
	if (index == P2P_NOT_FOUND)
		return;

	pP2pEntry = &pAd->P2pTable.Client[index];

	DBGPRINT(RT_DEBUG_ERROR, ("P2pReceDevDisRspAction %s.\n ", decodeP2PClientState(pP2pEntry->P2pClientState)));
	if (pP2pEntry->P2pClientState != P2PSTATE_GO_DISCO_COMMAND)
		return;
	/* Change State back to P2PSTATE_DISCOVERY_GO */
	/* Because P2PSTATE_GO_DISCO_COMMAND is from P2PSTATE_DISCOVERY_GO */
	pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY_GO;
	/* Get Request content capability */
	P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8),
		FALSE, &Dpid, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, DevAddr, 
		BssidAddr, NULL, &SsidLen, NULL, &ConfigMethod, NULL, DevType, &Channel, &OpChannel, NULL, &TempIntent, &StatusCode, NULL);

	if (StatusCode == P2PSTATUS_SUCCESS)
	{
		for (i = index; i < MAX_P2P_GROUP_SIZE;i++)
		{
			if (pAd->P2pTable.Client[i].P2pClientState == P2PSTATE_CLIENT_DISCO_COMMAND)
				ClientP2PIndex = i;
		}
		if (ClientP2PIndex == P2P_NOT_FOUND)
			return;

		pAd->P2pTable.Client[ClientP2PIndex].P2pClientState = P2PSTATE_CONNECT_COMMAND;
		P2pStartGroupForm(pAd, pAd->P2pTable.Client[ClientP2PIndex].addr, ClientP2PIndex);
	}

}

/*	
	==========================================================================
	Description: 
		The routine that is called when receiving Invitation Request Action Frame packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceInviteReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
	UCHAR		DevType[8], Ssid[32], DevAddr[6], BssidAddr[6], InfAddr[6], OpChannel, index, StatusCode;
	UCHAR		GroupCap, DeviceCap, DeviceNameLen;
 	ULONG		FrameLen;
	UCHAR		ChannelList[MAX_NUM_OF_CHANNELS];
	UCHAR		SsidLen, p2pindex;
	/*RT_P2P_CLIENT_ENTRY *pP2pEntry;*/
	UCHAR		RspStatus = P2PSTATUS_INVALID_PARM;
	UCHAR		MyRule = P2P_IS_GO;
	USHORT		ConfigMethod;
	UCHAR		InviteFlag;
	BOOLEAN		bReinvoke = FALSE;

	RTMPZeroMemory(ChannelList, MAX_NUM_OF_CHANNELS);
	RTMPZeroMemory(Ssid, MAX_LEN_OF_SSID);
	/* Get Request content capability */
	P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
		FALSE, NULL, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, DevAddr, InfAddr, 
		BssidAddr, &SsidLen, Ssid, &ConfigMethod, NULL, DevType, NULL, &OpChannel, ChannelList, NULL, &StatusCode, &InviteFlag);
	DBGPRINT(RT_DEBUG_ERROR, ("P2pReceInviteReqAction ==>InviteFlag = %d.  %s \n", 
				InviteFlag, decodeP2PState(pP2PCtrl->P2PConnectState)));

	/* Use TX to decide who is sending. */
	RTMPMoveMemory(DevAddr, pFrame->p80211Header.Addr2, MAC_ADDR_LEN);

	DBGPRINT(RT_DEBUG_ERROR, ("OpChannel = %d.    \n",  OpChannel));
	DBGPRINT(RT_DEBUG_ERROR, ("BssidAddr = %02x:%02x:%02x:%02x:%02x:%02x  \n",  PRINT_MAC(BssidAddr)));
	DBGPRINT(RT_DEBUG_ERROR, ("DevAddr = %02x:%02x:%02x:%02x:%02x:%02x  \n",  PRINT_MAC(DevAddr)));
	DBGPRINT(RT_DEBUG_ERROR, ("Ssid[%d] = %s.  \n",  SsidLen, Ssid));
	p2pindex = P2pGroupTabSearch(pAd, DevAddr);
	if (p2pindex >= MAX_P2P_GROUP_SIZE)
	{
		p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY_GO, Ssid, SsidLen, DeviceCap, GroupCap);
	}
	index = P2pPerstTabSearch(pAd, DevAddr, BssidAddr, InfAddr);
	DBGPRINT(RT_DEBUG_ERROR, ("perst index = %d. p2pindex = %d. \n", index, p2pindex));
	DBGPRINT(RT_DEBUG_ERROR, ("My Dpid = %d.    Dpid = %s.\n", pP2PCtrl->Dpid, decodeDpid(pP2PCtrl->Dpid)));
	P2PPrintP2PEntry(pAd, p2pindex);

	if (p2pindex == P2P_NOT_FOUND)
		return;

	if ((index < MAX_P2P_TABLE_SIZE) && ((InviteFlag & P2P_INVITE_FLAG_REINVOKE) == P2P_INVITE_FLAG_REINVOKE))
		bReinvoke = TRUE;

	/* case 1: Reinvoke case: */
	/* If I have credential and both enable persistent.  */
	if (bReinvoke == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Invite: reinvoke.  \n"));
		P2pCheckInviteReq(pAd, (pAd->P2pTable.PerstEntry[index].MyRule == P2P_IS_GO), index, ChannelList, BssidAddr, OpChannel, Ssid, SsidLen, &RspStatus);
		if (RspStatus == P2PSTATUS_SUCCESS)
		{
			/* Find peer,. So set Status = success. */
			if (pAd->P2pTable.PerstEntry[index].MyRule == P2P_IS_GO)
			{
				OpChannel = pP2PCtrl->GroupChannel;
				pP2PCtrl->GroupOpChannel = OpChannel;
				/* pAd->StaCfg.WscControl.WscAPChannel = OpChannel; */
				DBGPRINT(RT_DEBUG_ERROR, ("Invite: Decide to use OpChannel = %d for group \n", OpChannel));
				p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_CLIENT_WPS, Ssid, SsidLen, DeviceCap, GroupCap);
				if (p2pindex < MAX_P2P_GROUP_SIZE)
				{
					pAd->P2pTable.Client[p2pindex].P2pClientState = P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK;
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Invite: reach my limit. Send back RspStatus = %d.  \n", RspStatus));
					RspStatus = P2PSTATUS_LIMIT;
				}
			}
			else
			{
				MyRule = P2P_IS_CLIENT;
				pP2PCtrl->GroupChannel = OpChannel;
				pP2PCtrl->GroupOpChannel = OpChannel;
				/*P2pCopyPerstParmToCfg(pAd, index); */
				DBGPRINT(RT_DEBUG_ERROR, ("Invite: Reinvoke as client for group \n"));
				pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
				/* Update the Client state and SSID. */
				P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_GO_WPS, Ssid, SsidLen, DeviceCap, GroupCap);
				/*P2pGoNegoDone(pAd, pAd->PortList[pAd->P2pCfg.PortNumber], &pAd->P2pTable.Client[index]); */
				pP2PCtrl->P2PConnectState = P2P_DO_WPS_ENROLLEE;
				/*P2pWpsDone(pAd, DevAddr); */
				COPY_MAC_ADDR(pAd->ApCfg.ApCliTab[0].CfgApCliBssid, pP2PCtrl->Bssid);
			}
		}
	}
	/* case 2: abNormal invitation cases:...
	 */
	else if ((InviteFlag & P2P_INVITE_FLAG_REINVOKE) == P2P_INVITE_FLAG_REINVOKE)
	{
		/* Peer ask me to reinvoke. But I can't find my record. Send back error code. */
		RspStatus = P2PSTATUS_UNKNOWN_GROUP;
		/*
			In this case, we need to do P2P GO formation with that P2P peer again.
			Therefore we need to update P2pClientState and Rule here for preventing wrong action in P2pReceProvisionReqAction.
		*/
		pAd->P2pTable.Client[p2pindex].Rule = P2P_IS_DEVICE;
		pAd->P2pTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
		DBGPRINT(RT_DEBUG_ERROR, ("Invite:  Can't find Credential.    \n"));
	}
	/* case 3: Normal invitation cases:
	 */  
	else
	{
		if (RTMPEqualMemory(BssidAddr, ZERO_MAC_ADDR, MAC_ADDR_LEN))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Invite Req carry allzero Bssid, \n"));
		}
		else if (IS_P2P_CONNECT_IDLE(pAd) || (pAd->P2pCfg.P2PConnectState == P2P_INVITE))
		{
			P2pCheckInviteReqFromExisting(pAd, ChannelList, DevAddr, OpChannel, Ssid, SsidLen, &RspStatus);
			DBGPRINT(RT_DEBUG_ERROR, ("Invite: Got invitation from P2P Client . RspStatus = %d \n", RspStatus));
			if (RspStatus == P2PSTATUS_SUCCESS)
			{
				RTMPZeroMemory(pP2PCtrl->SSID, MAX_LEN_OF_SSID);
				RTMPMoveMemory(pP2PCtrl->SSID, Ssid, SsidLen);
				RTMPZeroMemory(pAd->P2pTable.Client[p2pindex].Ssid, MAX_LEN_OF_SSID);
				RTMPMoveMemory(pAd->P2pTable.Client[p2pindex].Ssid, Ssid, SsidLen);
				pAd->P2pTable.Client[p2pindex].SsidLen = SsidLen;
				COPY_MAC_ADDR(pP2PCtrl->Bssid, BssidAddr);
				COPY_MAC_ADDR(pAd->P2pTable.Client[p2pindex].bssid, BssidAddr);
				COPY_MAC_ADDR(pAd->P2pTable.Client[p2pindex].InterfaceAddr, BssidAddr);
				pP2PCtrl->SSIDLen = SsidLen;
				pP2PCtrl->GroupOpChannel = OpChannel;
				pAd->P2pTable.Client[p2pindex].OpChannel = OpChannel;
				pAd->P2pTable.Client[p2pindex].ConfigMethod = ConfigMethod;
				MyRule = P2P_IS_CLIENT;
			}
		}
	}


#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
	P2pSendWirelessEvent(pAd, RT_P2P_RECV_INVITE_REQ, &pAd->P2pTable.Client[p2pindex], pFrame->p80211Header.Addr2);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
	DBGPRINT(RT_DEBUG_ERROR, ("P2p Send Invite Rsp RspStatus = %d.  \n", RspStatus));
	P2PMakeInviteRsp(pAd, MyRule, pFrame->Token, pFrame->p80211Header.Addr2, pP2PCtrl->CurrentAddress, &OpChannel, &RspStatus, &FrameLen);

	/* Start Provision */
	if (((bReinvoke == FALSE) && (MyRule == P2P_IS_CLIENT)) && 
		((RspStatus == P2PSTATUS_SUCCESS) || ((RspStatus == P2PSTATUS_UNKNOWN_GROUP))))
	{
		P2pConnectPrepare(pAd, DevAddr, P2PSTATE_PROVISION_COMMAND);
	}
	else if ((bReinvoke == TRUE) && (RspStatus == P2PSTATUS_SUCCESS))
	{
		PWSC_CTRL           pWscControl;
		if (MyRule == P2P_IS_CLIENT)
		{
			POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
			pObj->ioctl_if_type = INT_P2P;
			pWscControl = &pAd->ApCfg.ApCliTab[0].WscControl;


			/* P2P CLIENT up. */
			Set_P2p_OpMode_Proc(pAd, "2");

			OS_WAIT(500);

			pObj->ioctl_if_type = INT_APCLI;
			NdisMoveMemory(pP2PCtrl->Bssid, pAd->P2pTable.PerstEntry[index].Addr, MAC_ADDR_LEN);
			WscWriteConfToApCliCfg(pAd,
							pWscControl,
							&pAd->P2pTable.PerstEntry[index].Profile,
							TRUE);
			pObj->ioctl_if_type = INT_P2P;

			if ((pAd->CommonCfg.Channel != OpChannel) && (!INFRA_ON(pAd)))
			{
				pAd->CommonCfg.Channel = OpChannel;
				AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
				AsicLockChannel(pAd, pAd->CommonCfg.Channel);
			}
			/* P2P AP-Client Enable. */
			Set_P2pCli_Enable_Proc(pAd, "1");
		}
		else
		{
			if (!P2P_GO_ON(pAd) && !P2P_CLI_ON(pAd))
			{
				pWscControl = &pAd->ApCfg.MBSSID[0].WscControl;

				WscWriteConfToPortCfg(pAd,
								  pWscControl,
								  &pAd->P2pTable.PerstEntry[index].Profile,
								  TRUE);
				P2P_GoStop(pAd);
				P2P_GoStartUp(pAd, MAIN_MBSSID);
			}
		}
	}
}

/*	
	==========================================================================
	Description: 
		The routine that is called when receiving Invitation Response Action Frame packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceInviteRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
	UCHAR		DevType[8], devAddr[6], BssidAddr[6] = {0}, OpChannel, index, StatusCode;
	UCHAR		GroupCap, DeviceCap, DeviceNameLen;
	USHORT		ConfigMethod;
	UCHAR		SsidLen, ChannelList[MAX_NUM_OF_CHANNELS];
	PRT_P2P_CLIENT_ENTRY		pP2pEntry = NULL;
	PWSC_CTRL           pWscControl;

	RTMPZeroMemory(ChannelList, MAX_NUM_OF_CHANNELS);
	RTMPZeroMemory(BssidAddr, MAC_ADDR_LEN);
	/* Get Request content capability */
	P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
		FALSE, NULL, &GroupCap, &DeviceCap, NULL, &DeviceNameLen, devAddr, NULL, 
		BssidAddr, &SsidLen, NULL, &ConfigMethod, NULL, DevType, NULL, &OpChannel, ChannelList, NULL, &StatusCode, NULL);

	DBGPRINT(RT_DEBUG_TRACE, ("P2pReceInviteRspAction ==> \n"));
	DBGPRINT(RT_DEBUG_TRACE, ("TA = %02x:%02x:%02x:%02x:%02x:%02x  \n",  
				PRINT_MAC(pFrame->p80211Header.Addr2)));
	
	if ((StatusCode == P2PSTATUS_SUCCESS) || (StatusCode == P2PSTATUS_PASSED))
	{
		index = P2pPerstTabSearch(pAd, pFrame->p80211Header.Addr2, NULL, NULL);
		if ((index < MAX_P2P_TABLE_SIZE)
			&& (IS_PERSISTENT_ON(pAd)))
		{
			/* this is a persistent connection. */
			pP2PCtrl->ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
			pP2PCtrl->ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
			/* Find peer,. So set Status = success. */
			if (pAd->P2pTable.PerstEntry[index].MyRule == P2P_IS_GO)
			{
				OpChannel = pP2PCtrl->GroupChannel;
				pP2PCtrl->GroupOpChannel = OpChannel;
				P2pCopyPerstParmToCfg(pAd, index);
				DBGPRINT(RT_DEBUG_TRACE, ("Decide to use OpChannel = %d for group \n", OpChannel));
				P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_CLIENT_WPS, NULL, SsidLen, 0, 0);
				pP2pEntry = &pAd->P2pTable.Client[index];


				pWscControl = &pAd->ApCfg.MBSSID[0].WscControl;
				
				WscWriteConfToPortCfg(pAd,
								  pWscControl,
								  &pAd->P2pTable.PerstEntry[index].Profile,
								  TRUE);
				P2P_GoStop(pAd);
				P2P_GoStartUp(pAd, MAIN_MBSSID);
			}
			else
			{
				POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
				pObj->ioctl_if_type = INT_P2P;
				pWscControl = &pAd->ApCfg.ApCliTab[0].WscControl;

				pP2PCtrl->GroupChannel = OpChannel;
				pP2PCtrl->GroupOpChannel = OpChannel;

				DBGPRINT(RT_DEBUG_ERROR, ("Reinvoke as client for group. OpChannel= %d\n", OpChannel));
				pP2PCtrl->P2PConnectState = P2P_DO_WPS_ENROLLEE;
				/* Add some delay to connect GO */
				P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME, NULL, SsidLen, 0, 0);

				/* P2P CLIENT up. */
				Set_P2p_OpMode_Proc(pAd, "2");

				OS_WAIT(500);

				pObj->ioctl_if_type = INT_APCLI;
				WscWriteConfToApCliCfg(pAd,
								pWscControl,
								&pAd->P2pTable.PerstEntry[index].Profile,
								TRUE);
				pObj->ioctl_if_type = INT_P2P;

				/* P2P AP-Client Enable. */
				Set_P2pCli_Enable_Proc(pAd, "1");

			}
		}
		else
		{
			index = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
			if (index < MAX_P2P_GROUP_SIZE)
			{
				if (P2P_GO_ON(pAd))
				{
					pP2pEntry = &pAd->P2pTable.Client[index];
					pP2pEntry->P2pClientState = P2PSTATE_GOT_GO_COMFIRM;
					P2pSetWps(pAd, pP2pEntry);
					DBGPRINT(RT_DEBUG_ERROR, ("P2pReceInviteRspAction Success.  I am GO. \n"));
				}
				else if (P2P_CLI_ON(pAd))
					DBGPRINT(RT_DEBUG_ERROR, ("P2pReceInviteRspAction Success.  I am client. \n"));
					
			}
		}
	}
}

/*	
	==========================================================================
	Description: 
		The routine that is called when receiving provistion request Action Frame packet.
		the Element field contains a single WPS IE.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceProvisionReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PRT_P2P_CLIENT_ENTRY		pP2pEntry = NULL;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
	USHORT		WpsConfigMethod;
	ULONG		FrameLen;
	UCHAR		p2pindex;
	UCHAR		GroupCap, DeviceCap, DevAddr[MAC_ADDR_LEN], BssidAddr[MAC_ADDR_LEN], OpChannel;
	UCHAR		Ssid[32], SsidLen;
	UCHAR		DeviceName[32], DeviceNameLen;
	UCHAR		DevType[8];
	USHORT		PeerWscMethod;
	BOOLEAN		bSendEvent = FALSE;

	DBGPRINT(RT_DEBUG_ERROR, ("P2pRece Provision ReqAction ==> \n"));
	if (IS_P2P_DEVICE_DISCOVERING(pAd))
		P2pStopScan(pAd);
	RTMPZeroMemory(&DeviceName, 32);
	P2pParseSubElmt(pAd, &pFrame->ElementID, (Elem->MsgLen - LENGTH_802_11 - 8), 
		FALSE, NULL, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, 
		BssidAddr, &SsidLen, Ssid, NULL, &WpsConfigMethod, DevType, NULL, &OpChannel, NULL, NULL, NULL, NULL);

	DBGPRINT(RT_DEBUG_ERROR, ("P2pRece Provision ReqAction ==>ConfigMethod = %x %s\n", WpsConfigMethod, decodeConfigMethod(WpsConfigMethod)));

	/*
		Patch for other vendor that might send multiple config... IN spec, the config method in Provision
		Discovery req should carry config method that indicate SINGLE method.
	*/
	if (WpsConfigMethod == WSC_CONFMET_PBC
		&& (pP2PCtrl->Dpid == DEV_PASS_ID_PBC))
	{	
		/* Correct. Keep going. */
	}
	else if (WpsConfigMethod == WSC_CONFMET_DISPLAY
		&& (pP2PCtrl->Dpid == DEV_PASS_ID_REG))
	{
		/* Correct. Keep going. */
	}
	else if (WpsConfigMethod == WSC_CONFMET_KEYPAD
		&& (pP2PCtrl->Dpid == DEV_PASS_ID_USER))
	{
		/* Correct. Keep going. */
	}
	else if (WpsConfigMethod == WSC_CONFMET_LABEL
		&& (pP2PCtrl->Dpid == DEV_PASS_ID_USER))
	{
		/* Correct. Keep going. */
	}
	else
	{
		if (WpsConfigMethod == WSC_CONFMET_PBC)
		{
			pP2PCtrl->WscMode = WSC_PBC_MODE;
			pP2PCtrl->Dpid = DEV_PASS_ID_PBC;
			pP2PCtrl->ConfigMethod = WSC_CONFMET_PBC;
		}
		else if (WpsConfigMethod == WSC_CONFMET_DISPLAY)
		{
			pP2PCtrl->WscMode = WSC_PIN_MODE;
			pP2PCtrl->Dpid = DEV_PASS_ID_REG;
			pP2PCtrl->ConfigMethod = WSC_CONFMET_DISPLAY;
		}
		else if (WpsConfigMethod == WSC_CONFMET_KEYPAD)
		{
			pP2PCtrl->WscMode = WSC_PIN_MODE;
			pP2PCtrl->Dpid = DEV_PASS_ID_USER;
			pP2PCtrl->ConfigMethod = WSC_CONFMET_KEYPAD;
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("P2pRece Provision ReqAction ==> Ignore this provision request here\n"));
			return;
		}
	}

	/* I only support those 4 method. */
	if ( (WpsConfigMethod == WSC_CONFMET_DISPLAY)
		|| (WpsConfigMethod == WSC_CONFMET_PBC) 
		|| (WpsConfigMethod == WSC_CONFMET_KEYPAD)
		|| (WpsConfigMethod == 0x188))
	{
		p2pindex = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
		if (p2pindex >= MAX_P2P_GROUP_SIZE)
		{
			if ((GroupCap & GRPCAP_OWNER) == GRPCAP_OWNER)
			{
				p2pindex = P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_DISCOVERY_GO, NULL, 0, DeviceCap, GroupCap);
				RTMPZeroMemory(pP2PCtrl->SSID, MAX_LEN_OF_SSID);
				RTMPMoveMemory(pP2PCtrl->SSID, Ssid, SsidLen);
				RTMPZeroMemory(pAd->P2pTable.Client[p2pindex].Ssid, MAX_LEN_OF_SSID);
				RTMPMoveMemory(pAd->P2pTable.Client[p2pindex].Ssid, Ssid, SsidLen);
				COPY_MAC_ADDR(pP2PCtrl->Bssid, BssidAddr);
				COPY_MAC_ADDR(pAd->P2pTable.Client[p2pindex].bssid, DevAddr);
				COPY_MAC_ADDR(pAd->P2pTable.Client[p2pindex].InterfaceAddr, DevAddr);
				pAd->P2pTable.Client[p2pindex].SsidLen = SsidLen;
				pP2PCtrl->SSIDLen = SsidLen;
			}
			else
			p2pindex = P2pGroupTabInsert(pAd, pFrame->p80211Header.Addr2, P2PSTATE_DISCOVERY, NULL, 0, DeviceCap, GroupCap);
			bSendEvent = TRUE;
		}
		if (p2pindex < MAX_P2P_GROUP_SIZE)
		{
			pP2pEntry = &pAd->P2pTable.Client[p2pindex];
			if (DeviceNameLen != 0)
			{
				RTMPMoveMemory(&pAd->P2pTable.Client[p2pindex].DeviceName[0], DeviceName, 32);
				pAd->P2pTable.Client[p2pindex].DeviceNameLen = DeviceNameLen;
				pAd->P2pTable.Client[p2pindex].DeviceName[pAd->P2pTable.Client[p2pindex].DeviceNameLen] = 0x0;
			}

			DBGPRINT(RT_DEBUG_ERROR, ("Update My Config Method to  %s (%d)\n", decodeConfigMethod(pP2PCtrl->ConfigMethod ), pP2PCtrl->ConfigMethod));			
			if (WpsConfigMethod == WSC_CONFMET_KEYPAD)
			{
				pP2pEntry->ConfigMethod = WSC_CONFMET_DISPLAY;
				pP2pEntry->Dpid = DEV_PASS_ID_REG;
			}
			else if (WpsConfigMethod == WSC_CONFMET_PBC)
			{
				pP2pEntry->ConfigMethod = WSC_CONFMET_PBC;
				pP2pEntry->Dpid = DEV_PASS_ID_PBC;
			}
			else
			{
				pP2pEntry->ConfigMethod = WSC_CONFMET_KEYPAD;
				pP2pEntry->Dpid = DEV_PASS_ID_USER;
			}

			if ( pP2PCtrl->bProvAutoRsp == TRUE)
			{
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
				P2pSendWirelessEvent(pAd, RT_P2P_RECV_PROV_REQ, &pAd->P2pTable.Client[p2pindex], pFrame->p80211Header.Addr2);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
				/* Update My WPS Configuration. */
				P2P_SetWscRule(pAd, p2pindex, &PeerWscMethod);
				/* Update Sigma.ConfigMethod after finished provision procedure. */
				P2P_SET_FLAG(&pAd->P2pTable.Client[p2pindex], P2PFLAG_PROVISIONED);
				P2PSendProvisionRsp(pAd, WpsConfigMethod, pFrame->Token, pFrame->p80211Header.Addr2, &FrameLen);
				if ((P2P_GO_ON(pAd))
					)
					P2pSetWps(pAd, pP2pEntry);
				else if ((pP2pEntry->Rule == P2P_IS_GO)

					)
				{
					pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
					pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
					RTMPMoveMemory(pP2PCtrl->Bssid, pAd->P2pTable.Client[p2pindex].bssid, MAC_ADDR_LEN);
					RTMPMoveMemory(pAd->P2pCfg.Bssid, pP2pEntry->bssid, MAC_ADDR_LEN);
					RTMPZeroMemory(pP2PCtrl->SSID, 32);
					RTMPMoveMemory(pP2PCtrl->SSID, pP2pEntry->Ssid, pP2pEntry->SsidLen);
					pP2PCtrl->SSIDLen = pP2pEntry->SsidLen;
					pP2PCtrl->GroupOpChannel = pP2pEntry->OpChannel;
					P2pGoNegoDone(pAd, pP2pEntry);
				}
				else
				{
					pP2pEntry->P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
					COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pP2pEntry->addr);
				}
			
			DBGPRINT(RT_DEBUG_ERROR, ("Accept Provision Req ==>ConfigMethod = %s \n", decodeConfigMethod(WpsConfigMethod)));
			}
			else
			{
				if ( pP2PCtrl->P2pProvIndex== P2P_NOT_FOUND )
				{
					pP2PCtrl->P2pProvConfigMethod = WpsConfigMethod;
					pP2PCtrl->P2pProvToken = pFrame->Token;
					RTMPMoveMemory(pP2PCtrl->ConnectingMAC, pFrame->p80211Header.Addr2, MAC_ADDR_LEN);
					pP2PCtrl->P2pCounter.UserAccept = 300;
					pP2PCtrl->P2pProvIndex = p2pindex;
					pP2PCtrl->P2pProvUserNotify = FALSE;
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
					P2pSendWirelessEvent(pAd, RT_P2P_RECV_PROV_REQ, &pAd->P2pTable.Client[p2pindex], pFrame->p80211Header.Addr2);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
				}
				else
				{
					pP2pEntry->P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
					COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pP2pEntry->addr);
				}
			}
		}
	}
	else
		/* send null to indicate failure */
		P2PSendProvisionRsp(pAd, 0, pFrame->Token, pFrame->p80211Header.Addr2, &FrameLen);
}

/*	
	==========================================================================
	Description: 
		The routine that is called when receiving provistion request Action Frame packet.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceProvisionRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PRT_P2P_CLIENT_ENTRY		pP2pEntry;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)Elem->Msg;
	USHORT		ConfigMethod = 0, PeerWscMethod;
	/*ULONG		FrameLen; */
	UCHAR		p2pindex = 0xff;
	/*PEID_STRUCT         pEid;*/
	PUCHAR Ptr = (PUCHAR) pFrame;
	PUCHAR pAddr = NULL;
	BOOLEAN 	Cancelled;


	p2pindex = P2pGroupTabSearch(pAd, pFrame->p80211Header.Addr2);
	if (p2pindex >= MAX_P2P_GROUP_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("P2pReceProvisionRspAction from unknown device ==> \n"));
		return;
	}

	if (pP2PCtrl->bP2pReSendTimerRunning)
	{
		pP2PCtrl->bP2pReSendTimerRunning = FALSE;
		pAd->P2pTable.Client[p2pindex].ReTransmitCnt = 0;
		RTMPCancelTimer(&pP2PCtrl->P2pReSendTimer, &Cancelled);
	}
	if (pAd->P2pCfg.GoFormCurrentState == P2P_WAIT_GO_FORM_PROV_RSP)
		pAd->P2pCfg.GoFormCurrentState = P2P_GO_FORM_IDLE;
	P2pStopConnectThis(pAd);
	pP2pEntry = &pAd->P2pTable.Client[p2pindex];
	pP2pEntry->bValid = FALSE;
	DBGPRINT(RT_DEBUG_ERROR, ("P2pReceProvisionRspAction from P2P_Tab[%d] = %02x:%02x:%02x:%02x:%02x:%02x ==> \n", p2pindex, PRINT_MAC(pP2pEntry->addr)));
	/* point to OUI2 */
	Ptr += (sizeof(HEADER_802_11) + 2 + 4 + 4); /* Category + Action + OUI + OUIType + SubType + Token + ElementID + Length */
	if (RTMPEqualMemory(Ptr, WPS_OUI, 4))
	{
		/* point to Element ID */
		/*Ptr -= 2;*/
		P2PParseWPSIE(Ptr, (pFrame->Length +2), NULL, &ConfigMethod, NULL, NULL);
	}

	DBGPRINT(RT_DEBUG_ERROR, ("P2pRece Provision RspAction ==>ConfigMethod = %04x. %s.\n", ConfigMethod, decodeConfigMethod(ConfigMethod)));
	DBGPRINT(RT_DEBUG_ERROR, ("pAd->P2pCfg.Dpid = %d. %s.\n", pAd->P2pCfg.Dpid, decodeDpid(pAd->P2pCfg.Dpid)));
	/* Set Event to GUI to display PIN digit. */
	if (ConfigMethod == WSC_CONFMET_KEYPAD)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("P2pKeSetEvent 13. Show PIN now\n"));
	}

	if (pP2pEntry->P2pClientState == P2PSTATE_SENT_PROVISION_REQ)
	{
		if (pP2pEntry->Rule == P2P_IS_GO)
			pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY_GO;
		else
			pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY;
	}

	/* p.s spec said null indicates failure. So check ConfigMethod to decide if should go on following actions. */
	if ((ConfigMethod == WSC_CONFMET_DISPLAY)
		|| (ConfigMethod == WSC_CONFMET_KEYPAD)
		|| (ConfigMethod == WSC_CONFMET_PBC))
	{
		BOOLEAN bAutoGroupFormation = FALSE;
		P2P_SET_FLAG(&pAd->P2pTable.Client[p2pindex], P2PFLAG_PROVISIONED);
		if (pP2pEntry->ConfigMethod == 0)
			bAutoGroupFormation = TRUE;
		pP2pEntry->ConfigMethod = ConfigMethod;
		P2P_SetWscRule(pAd, p2pindex, &PeerWscMethod);


		/* Peer use Label or Display. If peer is alreayd GO, I alreayd have WPS information to connect */
		/* So doesn't need to pop up a setting windows. */
		if ((ConfigMethod == WSC_CONFMET_DISPLAY)
			&& (pP2pEntry->Rule != P2P_IS_GO))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Need setting first. Pop up setting windows. %d\n", pAd->P2pCfg.PopUpIndex));
			pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY;
		}

		DBGPRINT(RT_DEBUG_ERROR, ("Peer %d ConfigMethod = %s \n", p2pindex, decodeConfigMethod(ConfigMethod)));
		if (pP2pEntry->Rule == P2P_IS_GO)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Decide to Join P2p group? when I am %s \n", decodeP2PState(pAd->P2pCfg.P2PConnectState)));
			if (pAd->P2pCfg.P2PConnectState == P2P_CONNECT_IDLE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("P2p :connecting to GO with Bssid   %02x:%02x:%02x:%02x:%02x:%02x. \n", PRINT_MAC(pAd->P2pTable.Client[p2pindex].bssid)));
				DBGPRINT(RT_DEBUG_TRACE, ("P2p : its GroupCapability= %x.  DevCapability= %x. \n", pAd->P2pTable.Client[p2pindex].GroupCapability, pAd->P2pTable.Client[p2pindex].DevCapability));
				pP2PCtrl->P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
				pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
				RTMPMoveMemory(pP2PCtrl->Bssid, pAd->P2pTable.Client[p2pindex].bssid, MAC_ADDR_LEN);
				RTMPMoveMemory(pAd->P2pCfg.Bssid, pP2pEntry->bssid, MAC_ADDR_LEN);
				RTMPZeroMemory(pP2PCtrl->SSID, 32);
				RTMPMoveMemory(pP2PCtrl->SSID, pP2pEntry->Ssid, pP2pEntry->SsidLen);
				pP2PCtrl->SSIDLen = pP2pEntry->SsidLen;
				pP2PCtrl->GroupOpChannel = pP2pEntry->OpChannel;
				P2pGoNegoDone(pAd, pP2pEntry);
			}
		}
		else
		{
			if (P2P_GO_ON(pAd))
			{
				/* Set MyRule in P2P GroupFormat */
				P2pSetWps(pAd, pP2pEntry);
				DBGPRINT(RT_DEBUG_ERROR, (" P2P - I am Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
			}
			else
			{
				if ((pP2PCtrl->ConfigMethod == WSC_CONFMET_KEYPAD) || 
					((pP2PCtrl->ConfigMethod == WSC_CONFMET_DISPLAY) && bAutoGroupFormation == FALSE))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s:: recv Provision Rsp. Condif Method = %04x.\n", __FUNCTION__, ConfigMethod));
				}
				else
				{
					/* After Provision Success, start connect command */
					pAddr = &pAd->P2pTable.Client[p2pindex].addr;
					P2pConnectPrepare(pAd, pAddr, P2PSTATE_CONNECT_COMMAND);
				}
			}
		}
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
		P2pSendWirelessEvent(pAd, RT_P2P_RECV_PROV_RSP, &pAd->P2pTable.Client[p2pindex], pFrame->p80211Header.Addr2);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */

	}
	else
	{
		pP2pEntry->ConfigMethod = 0;
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
		P2pSendWirelessEvent(pAd, RT_P2P_RECV_PROV_RSP, &pAd->P2pTable.Client[p2pindex], pFrame->p80211Header.Addr2);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
	}
}


/*	
	==========================================================================
	Description: 
		The routine send additional Probe Request when being in P2P Search State..
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pSendProbeReq(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	PUCHAR		pOutBuffer;
	ULONG		FrameLen;
	NDIS_STATUS   NStatus;
	MLME_QUEUE_ELEM 		*Elem;
	
	os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
	if (Elem)
	{
		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
		if (NStatus != NDIS_STATUS_SUCCESS)
		{
			os_free_mem(NULL, Elem);
			return;
		}

		P2PMakeProbe(pAd, Elem, Channel, SUBTYPE_PROBE_REQ, pOutBuffer, &FrameLen);
		if (FrameLen > 0)
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}
	os_free_mem(NULL, Elem);
}

/*	
	==========================================================================
	Description: 
		The routine send Go Negociation Confirm packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PSendGoNegoConfirm(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR			Token,
	IN UCHAR			idx,
	IN PUCHAR		Addr1)
{
		PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
		ULONG		FrameLen;
		ULONG		TempLen;
		PRT_P2P_CLIENT_ENTRY pP2pEntry;
		NDIS_STATUS   NStatus;
		PUCHAR			pOutBuffer = NULL, pDest;
		UCHAR		Channel, StatusCode;
		PP2P_PUBLIC_FRAME	pFrame;
		ULONG		P2pLen = 0;
		
		
		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
		if (NStatus != NDIS_STATUS_SUCCESS)
			return;
	
		pP2pEntry = &pAd->P2pTable.Client[idx];
		pDest = pOutBuffer;
		pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
		P2PMakeGoNegoConfirm(pAd, Addr1, Token, pDest, &TempLen);
		FrameLen = TempLen;
		pDest += TempLen;
		P2pLen = 4;
		StatusCode = 0;
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_STATUS, &StatusCode, pDest);
		pDest += TempLen;
		FrameLen += TempLen;
		P2pLen +=TempLen;
		/* :  .
		 */
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CAP, pP2PCtrl->P2pCapability, pDest);
		FrameLen += TempLen;
		pDest += TempLen;
		P2pLen +=TempLen;
		/* If I am Go, Use my channel to set in the Go Nego Rsp. */
		if (pP2pEntry->Rule == P2P_IS_CLIENT)
		{
			if (INFRA_ON(pAd))
				Channel = pAd->CommonCfg.Channel;
			else
				Channel = pP2PCtrl->GroupChannel;
			DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P - I become Internal REGISTRA!!	  REGISTRA. !! GOGOGO\n", __FUNCTION__));
			TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &Channel, pDest);
			FrameLen += TempLen;
			pDest += TempLen;
			P2pLen +=TempLen;
			TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CHANNEL_LIST, &Channel, pDest);
			FrameLen += TempLen;
			pDest += TempLen;
			P2pLen +=TempLen;
			/* Shall include Group ID if I am GO */
			/*NdisMoveMemory(pP2PCtrl->SSID, "DIRECT-qq", 9); */
			/*pP2PCtrl->SSIDLen = 9; */
			RTMPMoveMemory(pP2PCtrl->SSID, &pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen);
			pP2PCtrl->SSIDLen = pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen;
			{
				UCHAR tmpGroupID[40]; // 2 (Length) + 6 (MAC Address ) + 32 (SSID)
				tmpGroupID[0] = 6 + pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen;
				tmpGroupID[1] = 0;
				RTMPMoveMemory(&tmpGroupID[2], pP2PCtrl->CurrentAddress, 6);
				RTMPMoveMemory(&tmpGroupID[8], &pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen);
				TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_GROUP_ID, &tmpGroupID, pDest);
			}
			FrameLen += TempLen;
			pDest += TempLen;
			P2pLen +=TempLen;
			
			pP2PCtrl->Rule = P2P_IS_GO;
			pAd->flg_p2p_OpStatusFlags = P2P_GO_UP;			
		}
		else
		{
			/* update GO's SSID */
			RTMPZeroMemory(pP2PCtrl->SSID, 32);
                        RTMPMoveMemory(pP2PCtrl->SSID, pP2pEntry->Ssid, pP2pEntry->SsidLen);
                        pP2PCtrl->SSIDLen = pP2pEntry->SsidLen;
			Channel = pP2pEntry->OpChannel;
			TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &Channel, pDest);
			FrameLen += TempLen;
			pDest += TempLen;
			P2pLen +=TempLen;
			TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CHANNEL_LIST, &Channel, pDest);
			FrameLen += TempLen;
			pDest += TempLen;
			P2pLen +=TempLen;
			DBGPRINT(RT_DEBUG_ERROR, ("%s:: P2P - I become ENROLLEE!!	ENROLLEE.!!\n", __FUNCTION__));
/*
			*pDest = SUBID_P2P_CHANNEL_LIST;
			pDest++;
			*pDest = 6;
			*(pDest + 1) = 0;
			//country string . Two ASCII +	one more byte
			*(pDest + 2) = 0x55;
			*(pDest + 3) = 0x53;
			*(pDest + 4) = 0x20;
			*(pDest + 5) = 3;
			if (Channel  <= 11)
			{
				*(pDest + 5) = 12;
			}
			else if (Channel  <= 48)
			{
				*(pDest + 5) = 1;
			}			
			else if (Channel  <= 64)
			{
				*(pDest + 5) = 2;
			}
			else if (Channel  <= 140)
			{
				*(pDest + 5) = 4;
			}
			else if (Channel  == 165)
			{
				*(pDest + 5) = 5;
			}
			*(pDest + 6) = 1;
			*(pDest + 7) = Channel;
			P2pLen += 9;
			FrameLen += 9;
			pDest += 9;
	
			// Insert Channel List Attribute.
			*pDest = SUBID_P2P_CHANNEL_LIST;
			pDest++;
			*pDest = 6;
			*(pDest + 1) = 0;
*/	
			pP2PCtrl->Rule = P2P_IS_CLIENT;
			pAd->flg_p2p_OpStatusFlags = P2P_CLI_UP;
		}
		DBGPRINT(RT_DEBUG_INFO, ("%s:: P2P - Opchannel is %d \n", __FUNCTION__, Channel));
	
		pFrame->Length = (UCHAR)P2pLen;
		pP2pEntry->P2pClientState = P2PSTATE_WAIT_GO_COMFIRM_ACK;
		/* PFP2P */
		pP2pEntry->StateCount = 0;
		pP2pEntry->bValid = FALSE;
		/*P2pSetWps(pAd, pPort); */
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

		/*
			Wait here for waiting this GO Nego Confirm is sent out;
			otherwise this GO Nego Confirm will be sent out in operation channel.
			P2pSetWps will do AsicSwitchChannel to operation channel.
		*/
		OS_WAIT(50);
		/* Start WPS. */
		P2pSetWps(pAd, pP2pEntry);

}

/*	
	==========================================================================
	Description: 
		The routine make  Device Discovery Request Action Frame Packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PSendDevDisReq(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		Addr1,
	IN PUCHAR		Bssid,
	IN PUCHAR		ClientAddr1,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame;
	ULONG	P2pIeLen = 0;
	PUCHAR			pDest;
	ULONG	Length;
	PUCHAR        	pOutBuffer = NULL;
	NDIS_STATUS   NStatus;
	UCHAR		i;
	PUCHAR Ptr = NULL;

	*pTotalFrameLen = 0;
	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	pDest = pOutBuffer;
	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	Ptr = (PUCHAR) pFrame;
	ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Bssid);
	DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevDisReq  TO %02x:%02x:%02x:%02x:%02x:%02x. \n", PRINT_MAC(Addr1)));
	DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevDisReq  Bssid %02x:%02x:%02x:%02x:%02x:%02x. \n", PRINT_MAC(Bssid)));
	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = P2P_DEV_DIS_REQ;

	pP2PCtrl->Token++;
	pFrame->Token = pP2PCtrl->Token;
	
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	P2pIeLen = 4;
	*pTotalFrameLen = 38;
	
	pDest = &pFrame->Octet[0];
	/* attach subelementID = 3. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_DEVICE_ID, ClientAddr1, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;

	pFrame->Length = (UCHAR)P2pIeLen;
	pDest = (PUCHAR)pOutBuffer;
	for (i = 0; i <*pTotalFrameLen; )
	{
		DBGPRINT(RT_DEBUG_TRACE,(": %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2), 
		*(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8)));
		i = i + 9;
	}
	MiniportMMRequest(pAd, 0, pOutBuffer, *pTotalFrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("P2P - P2PSendDevice Discovery Req  . TO %02x:%02x:%02x:%02x:%02x:%02x.  *pTotalFrameLen = %ld. \n", PRINT_MAC(Addr1), *pTotalFrameLen));

}

/*	
	==========================================================================
	Description: 
		The routine make  Device Discovery Response Action Frame Packet .
		Send by GO. So Bssid is my GO addr
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PSendDevDisRsp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR 		RspStatus,
	IN UCHAR 			Token,
	IN PUCHAR		Addr1,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame;
	ULONG	P2pIeLen = 0;
	PUCHAR			pDest;
	ULONG	Length;
	PUCHAR        	pOutBuffer = NULL;
	NDIS_STATUS   NStatus;
	PUCHAR Ptr = NULL;

	*pTotalFrameLen = 0;
	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	pDest = pOutBuffer;
	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	Ptr = (PUCHAR) pFrame;
	ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, pP2PCtrl->CurrentAddress);
	DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2PSendDevDisRsp TO %02x:%02x:%02x:%02x:%02x:%02x. \n", PRINT_MAC(Addr1)));
	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = P2P_DEV_DIS_RSP;

	pFrame->Token = Token;
	
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	P2pIeLen = 4;
	*pTotalFrameLen = 38;
	
	pDest = &pFrame->Octet[0];
	/* attach subelementID = 3. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_STATUS, &RspStatus, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;

	pFrame->Length = (UCHAR)P2pIeLen;
	MiniportMMRequest(pAd, 0, pOutBuffer, *pTotalFrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2PSendDevice Discovery Response. TO %02x:%02x:%02x:%02x:%02x:%02x.   \n", PRINT_MAC(Addr1)));

}

/*	
	==========================================================================
	Description: 
		The routine make Invitiation Response Action Frame Packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PSendProvisionReq(
	IN PRTMP_ADAPTER pAd,
	IN USHORT 		ConfigMethod,
	IN UCHAR 			Token,
	IN PUCHAR		Addr1,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame;
	ULONG	WpsIeLen = 0;
	PUCHAR			pDest;
	PUCHAR			pP2PIeLenDest;
	ULONG	Length;
	PUCHAR        	pOutBuffer = NULL;
	NDIS_STATUS   NStatus;
	PUCHAR Ptr = NULL;

	UCHAR index;
	PRT_P2P_CLIENT_ENTRY pP2pEntry;
	*pTotalFrameLen = 0;
	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2PMakeProvisionReq  . TO %02x:%02x:%02x:%02x:%02x:%02x.  ConfigMethod = %s \n", PRINT_MAC(Addr1), decodeConfigMethod(ConfigMethod)));
	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	Ptr = (PUCHAR) pFrame;
	ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Addr1);
	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = P2P_PROVISION_REQ;
	pFrame->Token = Token;
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->OUI2), WPS_OUI, 4);*/
	RTMPMoveMemory(Ptr, WPS_OUI, 4);
	WpsIeLen = 4;
	*pTotalFrameLen = 38;
	pDest = pFrame->Octet;
	/* attach config method . */
	*((PUSHORT) pDest) = cpu2be16(WSC_ID_CONFIG_METHODS);
	*((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
	*((PUSHORT) (pDest + 4)) = cpu2be16(ConfigMethod);		/* Label, Display, PBC */
	pDest += 6;
	WpsIeLen   += 6;
	*pTotalFrameLen += 6;
	pFrame->Length = (UCHAR)WpsIeLen;

	/* Insert P2P IE===> */
	*pDest = IE_VENDOR_SPECIFIC;
	pP2PIeLenDest = (pDest+1);
	RTMPMoveMemory(pDest+2, P2POUIBYTE, 4);
	WpsIeLen = 4;
	pDest += 6;
	*pTotalFrameLen += 6;

	/* attach capability	 */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CAP,  pP2PCtrl->P2pCapability, pDest);
	pDest += Length;
	WpsIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 13. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_DEVICE_INFO,  pP2PCtrl->CurrentAddress, pDest);
	pDest += Length;
	WpsIeLen += Length;
	*pP2PIeLenDest = (UCHAR)WpsIeLen;
	*pTotalFrameLen += Length;
	/* attach subelementID = 15 */
	index = P2pGroupTabSearch(pAd, Addr1);
	if (index != P2P_NOT_FOUND)
	{
		UCHAR tmpGroupID[40]; // 2 (Length) + 6 (MAC Address ) + 32 (SSID)	
		pP2pEntry = &pAd->P2pTable.Client[index];
		DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2PMakeProvisionReq   MyRule=%s, PeerRule=%d \n", decodeMyRule(pP2PCtrl->Rule), pP2pEntry->Rule));
		if (pP2pEntry->Rule == P2P_IS_GO) // peer is GO
		{
            tmpGroupID[0] = 6 + pP2pEntry->SsidLen;
            tmpGroupID[1] = 0;
            RTMPMoveMemory(&tmpGroupID[2], pP2pEntry->bssid, 6);
            RTMPMoveMemory(&tmpGroupID[8], &pP2pEntry->Ssid[0], pP2pEntry->SsidLen);
			Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_GROUP_ID, &tmpGroupID, pDest);
		}else if (P2P_GO_ON(pAd)) //I'm GO
		{
			tmpGroupID[0] = 6 + pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen;
			tmpGroupID[1] = 0;
			RTMPMoveMemory(&tmpGroupID[2], pP2PCtrl->CurrentAddress, 6);
			RTMPMoveMemory(&tmpGroupID[8], &pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen);
			Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_GROUP_ID, &tmpGroupID, pDest);		
		}
		else
		{
			Length =0;
		}
	}
	else
	{
		Length =0;	
	}
	pDest += Length;
	WpsIeLen += Length;
	*pP2PIeLenDest = (UCHAR)WpsIeLen;
	*pTotalFrameLen += Length;
	
	pP2PCtrl->bP2pReSendTimerRunning = TRUE;
	RTMPSetTimer(&pP2PCtrl->P2pReSendTimer, P2P_TRANSMIT_TIMER);
	MiniportMMRequest(pAd, 0, pOutBuffer, *pTotalFrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

}
/*	
	==========================================================================
	Description: 
		The routine make Invitiation Response Action Frame Packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PSendProvisionRsp(
	IN PRTMP_ADAPTER pAd,
	IN USHORT 		ConfigMethod,
	IN UCHAR 			Token,
	IN PUCHAR		Addr1,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame;
	UCHAR	WpsIeLen = 0;
	PUCHAR			pDest;
	//ULONG	Length;
	PUCHAR        	pOutBuffer = NULL;
	NDIS_STATUS   NStatus;
	PUCHAR Ptr = NULL;

	*pTotalFrameLen = 0;
	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2PMakeProvisionRsp  . TO %02x:%02x:%02x:%02x:%02x:%02x.  ConfigMethod = %x(%s) \n", PRINT_MAC(Addr1), ConfigMethod, decodeConfigMethod(ConfigMethod)));
	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	Ptr = (PUCHAR) pFrame;
	ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Addr1);
	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4); */
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = P2P_PROVISION_RSP;
	pFrame->Token = Token;
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->OUI2), WPS_OUI, 4); */
	RTMPMoveMemory(Ptr, WPS_OUI, 4);
	WpsIeLen = 4;
	*pTotalFrameLen = 38;
	pDest = pFrame->Octet;
	if (ConfigMethod > 0)
	{
		/* attach wsc version */
		*((PUSHORT) pDest) = cpu2be16(WSC_ID_VERSION);
		*((PUSHORT) (pDest + 2)) = cpu2be16(0x0001);
		*(pDest + 4) = WSC_VERSION;	/* TODO: WPS2.0 ?? */
		pDest += 5;
		WpsIeLen   += 5;
		*pTotalFrameLen += 5;
		
		/* attach config method . */
		*((PUSHORT) pDest) = cpu2be16(WSC_ID_CONFIG_METHODS);
		*((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
		*((PUSHORT) (pDest + 4)) = cpu2be16(ConfigMethod);		/* Label, Display, PBC */
		pDest += 6;
		WpsIeLen   += 6;
		*pTotalFrameLen += 6;
	}
	pFrame->Length = WpsIeLen;
	pP2PCtrl->bP2pReSendTimerRunning = TRUE;
	RTMPSetTimer(&pP2PCtrl->P2pReSendTimer, /*P2P_TRANSMIT_TIMER*/ 1000);
	MiniportMMRequest(pAd, 0, pOutBuffer, *pTotalFrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

}

/*	
	==========================================================================
	Description: 
		The routine prepares Go Negociation Confirm packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PMakeGoNegoConfirm(
	IN PRTMP_ADAPTER pAd,
 	IN PUCHAR		Addr1,
	IN UCHAR			Token,
	IN PUCHAR		pOutBuffer,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	/*PUCHAR			pDest; */
	/*ULONG	Length; */
	PUCHAR Ptr = (PUCHAR) pFrame;

	ActHeaderInit(pAd, (PHEADER_802_11)pOutBuffer, Addr1, pP2PCtrl->CurrentAddress, pP2PCtrl->PortCfg.Bssid);

	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4); */
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = GO_NEGOCIATION_CONFIRM;
	pFrame->Token = Token;
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->Octet), P2POUIBYTE, 4); */
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);

	*pTotalFrameLen = 38;

}


/*	
	==========================================================================
	Description: 
		The routine make Go Negociation Response packet .
		
	Parameters:
		TempIntent is directly from Go Req. Still need to parse this.
	Note:
		 
	==========================================================================
 */
VOID P2PMakeGoNegoRsp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		Addr1,
	IN USHORT			SentDpid,
	IN UCHAR			Token,
	IN UCHAR			TempIntent,
	IN UCHAR			Channel,
	IN UCHAR			Status,
	IN PUCHAR		pOutBuffer,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	PUCHAR			pDest;
	ULONG	Length;
	/*UCHAR	P2PCap[2]; */
	ULONG	P2pIeLen = 0;
	UCHAR	RealIntent;
	UCHAR			WscIEFixed[] = {0xdd, 0x0f, 0x00, 0x50, 0xf2, 0x04};	/* length will modify later */
	PUCHAR Ptr = (PUCHAR) pFrame;

	ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Addr1);

	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* pointer to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr +=4;
	/* OUISubtype */
	pFrame->Subtype = GO_NEGOCIATION_RSP;
	pFrame->Token = Token;
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->Octet), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	P2pIeLen = 4;
	*pTotalFrameLen = sizeof(P2P_PUBLIC_FRAME) - 1;
	pDest = pFrame->Octet;
	
	/* Start attach subelement. */
	/* attach subelementID= 0. */
	DBGPRINT(RT_DEBUG_ERROR, ("%s: P2P - Status %d \n", __FUNCTION__, Status));
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_STATUS, &Status, pDest);
	pDest += Length;
 	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 2. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CAP,  pP2PCtrl->P2pCapability, pDest);
	pDest += Length;
 	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	
	/* attach subelementID= 4. */
	RealIntent = TempIntent>>1;
	RealIntent = (pP2PCtrl->GoIntentIdx << 1) + ((TempIntent & 0x1) == 1 ? 0 : 1);

	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OWNER_INTENT, &RealIntent, pDest);
	pDest += Length;
 	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 4. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CONFIG_TIMEOUT, &pP2PCtrl->ConfigTimeout[0], pDest);
	pDest += Length;
 	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 17. */
	if (INFRA_ON(pAd))
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &pAd->CommonCfg.Channel, pDest);
	else
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &pP2PCtrl->GroupChannel, pDest);
	pDest += Length;
 	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/*  attach subelementID= 9. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_INTERFACE_ADDR, pP2PCtrl->CurrentAddress, pDest);
	pDest += Length;
 	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/*  attach channel List= 11. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CHANNEL_LIST, &pP2PCtrl->GroupChannel, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/*  attach subelementID= 13. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_DEVICE_INFO, pP2PCtrl->CurrentAddress, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;

	/*  attach subelementID= 15. */
	if (pP2PCtrl->P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO)
	{
		UCHAR tmpGroupID[40]; // 2 (Length) + 6 (MAC Address ) + 32 (SSID)
		tmpGroupID[0] = 6 + pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen;
		tmpGroupID[1] = 0;
		RTMPMoveMemory(&tmpGroupID[2], pP2PCtrl->CurrentAddress, 6);
		RTMPMoveMemory(&tmpGroupID[8], &pAd->ApCfg.MBSSID[MAIN_MBSSID].Ssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].SsidLen);
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_GROUP_ID, &tmpGroupID, pDest);
		pDest += Length;
		P2pIeLen += Length;
		*pTotalFrameLen += Length;
	}

	pFrame->Length = (UCHAR)P2pIeLen;
	/* 0. WSC fixed IE */
	RTMPMoveMemory(pDest, &WscIEFixed[0], 6);
	pDest += 6;
	*pTotalFrameLen += 6;
	/* 1. Version */
	*((PUSHORT) pDest) = cpu2be16(WSC_ID_VERSION);
	*((PUSHORT) (pDest + 2)) = cpu2be16(0x0001);
	*(pDest + 4) = WSC_VERSION;
	pDest += 5;
	*pTotalFrameLen   += 5;
	/* 9. Device password ID. According to Table.1 in P2P Spec. */

	*((PUSHORT) pDest) = cpu2be16(WSC_ID_DEVICE_PWD_ID);
	*((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
	*((PUSHORT) (pDest + 4)) = cpu2be16(SentDpid);
	pDest += 6;
	*pTotalFrameLen   += 6;

}


/*	
	==========================================================================
	Description: 
		The routine make Go Negociation Request packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PMakeGoNegoReq(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR			index,
	IN PUCHAR		Addr1,
	IN PUCHAR		pOutBuffer,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	PUCHAR			pDest;
	ULONG	Length;
	ULONG	P2pIeLen = 0;
	UCHAR	FilledIntent;
	UCHAR	RandomB;
	UCHAR			WscIEFixed[] = {0xdd, 0x0f, 0x00, 0x50, 0xf2, 0x04};	/* length will modify later */
	PUCHAR		pWscIeLen;
	/*UCHAR		ChannelList; */
	USHORT		SentDpid;
	PUCHAR Ptr = (PUCHAR) pFrame;

	if (index >= MAX_P2P_GROUP_SIZE)
	{
		return;
	}
	*pTotalFrameLen = 0;
	RTMPMoveMemory(pP2PCtrl->PortCfg.Bssid, Addr1, 6);
	ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Addr1);

	DBGPRINT(RT_DEBUG_ERROR, (" %s - P2PMakeGoNegoReq  TO %02x:%02x:%02x:%02x:%02x:%02x. \n", __FUNCTION__, Addr1[0], Addr1[1], Addr1[2],Addr1[3],Addr1[4],Addr1[5]));

	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = GO_NEGOCIATION_REQ;
	pFrame->Token = 1;
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	P2pIeLen = 4;
	*pTotalFrameLen = 38;
	pDest = pFrame->Octet;
	
	/* Start attach subelement. */
	/* attach subelementID= 2. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CAP, pP2PCtrl->P2pCapability, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/*  attach subelementID= 4. */
	FilledIntent = pP2PCtrl->GoIntentIdx<<1;
	RandomB = RandomByte(pAd);
	/* If Intent is not 15, set tie breaker bit to 1 randomly. */
	if (((RandomB %2) == 0) && (pP2PCtrl->GoIntentIdx != 15))
		FilledIntent |= 1;
	
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OWNER_INTENT, &FilledIntent, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/*  attach subelementID= 7. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CONFIG_TIMEOUT, &pP2PCtrl->ConfigTimeout[0], pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_LISTEN_CHANNEL, &pP2PCtrl->ListenChannel, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	
	/*  attach subelementID= 9. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_INTERFACE_ADDR, pP2PCtrl->CurrentAddress, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/*  attach channel List= 11. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CHANNEL_LIST, &pP2PCtrl->GroupChannel, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/*  attach subelementID= 13. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_DEVICE_INFO, pP2PCtrl->CurrentAddress, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;

	/*  attach subelementID= 17. */
	if (INFRA_ON(pAd))
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &pAd->CommonCfg.Channel, pDest);
	else
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &pP2PCtrl->GroupChannel, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;

	if (IS_P2P_SUPPORT_EXT_LISTEN(pAd))
	{
		/*  attach subelementID= 17. */
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_EXT_LISTEN_TIMING, NULL, pDest);
		pDest += Length;
		P2pIeLen += Length;
		*pTotalFrameLen += Length;
	}

	pFrame->Length = (UCHAR)P2pIeLen;

	/* 0. WSC fixed IE */

	RTMPMoveMemory(pDest, &WscIEFixed[0], 6);
	pWscIeLen = pDest + 1;
	pDest += 6;
	*pTotalFrameLen += 6;
	*pWscIeLen = 4;
	/* 1. Version */
	*((PUSHORT) pDest) = cpu2be16(WSC_ID_VERSION);
	*((PUSHORT) (pDest + 2)) = cpu2be16(0x0001);
	*(pDest + 4) = WSC_VERSION;
	pDest += 5;
	*pTotalFrameLen   += 5;
	*pWscIeLen += 5;
		/* 9. Device password ID */
	*((PUSHORT) pDest) = cpu2be16(WSC_ID_DEVICE_PWD_ID);
	*((PUSHORT) (pDest + 2)) = cpu2be16(0x0002);
	SentDpid = pP2PCtrl->Dpid;
	if (P2P_TEST_FLAG(&pAd->P2pTable.Client[index], P2PFLAG_PROVISIONED))
	{
		switch(pAd->P2pTable.Client[index].ConfigMethod)
		{
			case WSC_CONFMET_DISPLAY:
				SentDpid = DEV_PASS_ID_USER;
				break;
			case WSC_CONFMET_PBC:
				SentDpid = DEV_PASS_ID_PBC;
				break;
			case WSC_CONFMET_KEYPAD:
				SentDpid = DEV_PASS_ID_REG;
			default :
				SentDpid = DEV_PASS_ID_PIN;
				break;
		}
	}
	*((PUSHORT) (pDest + 4)) = cpu2be16(SentDpid);
	pDest += 6;
	*pTotalFrameLen   += 6;
	*pWscIeLen += 6;

}

/*	
	==========================================================================
	Description: 
		The routine make Invitiation Request Action Frame Packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PMakeInviteReq(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR 		MyRule,
	IN UCHAR 		InviteFlag,
	IN PUCHAR		Addr1,
	IN PUCHAR		Bssid,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame;
	ULONG	P2pIeLen = 0;
	PUCHAR			pDest;
	ULONG	Length;
	PUCHAR        	pOutBuffer = NULL;
	NDIS_STATUS   NStatus;
	PUCHAR Ptr = NULL;

	*pTotalFrameLen = 0;
	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	Ptr = (PUCHAR) pFrame;
	ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Bssid);
	DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2PMakeInviteReq  TO %02x:%02x:%02x:%02x:%02x:%02x. \n", PRINT_MAC(Addr1)));
	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = P2P_INVITE_REQ;

	pP2PCtrl->Token++;
	pFrame->Token = pP2PCtrl->Token;
	
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	P2pIeLen = 4;
	*pTotalFrameLen = 38;
	pDest = pFrame->Octet;
	/* attach subelementID= 5. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CONFIG_TIMEOUT, &pP2PCtrl->ConfigTimeout[0], pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 11. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_INVITE_FLAG, &InviteFlag, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 17. */
	if (MyRule == P2P_IS_GO)
	{
		if (INFRA_ON(pAd))
			Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &pAd->CommonCfg.Channel, pDest);
		else
			Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &pP2PCtrl->GroupOpChannel, pDest);
		pDest += Length;
		P2pIeLen += Length;
		*pTotalFrameLen += Length;
	}
	/* attach subelementID= 7. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_GROUP_BSSID, &pP2PCtrl->CurrentAddress[0], pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 11. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CHANNEL_LIST, &pP2PCtrl->GroupOpChannel, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 11. */
	{
		UCHAR tmpGroupID[40]; // 2 (Length) + 6 (MAC Address ) + 32 (SSID)
		tmpGroupID[0] = 6 + pP2PCtrl->SSIDLen;
		tmpGroupID[1] = 0;
		RTMPMoveMemory(&tmpGroupID[2], pP2PCtrl->CurrentAddress, 6);
		RTMPMoveMemory(&tmpGroupID[8], &pP2PCtrl->SSID[0], pP2PCtrl->SSIDLen);
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_GROUP_ID, &tmpGroupID, pDest);
	}
			
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 11. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_DEVICE_INFO, pP2PCtrl->CurrentAddress, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;


	pFrame->Length = (UCHAR)P2pIeLen;
	MiniportMMRequest(pAd, 0, pOutBuffer, *pTotalFrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

}

/*	
	==========================================================================
	Description: 
		The routine make Invitiation Response Action Frame Packet .
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PMakeInviteRsp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR 		MyRule,
	IN UCHAR 		Token,
	IN PUCHAR		Addr1,
	IN PUCHAR		Bssid,
	IN PUCHAR		OpChannel,
	IN PUCHAR		Status,
	OUT PULONG		pTotalFrameLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PP2P_PUBLIC_FRAME	pFrame;
	ULONG	P2pIeLen = 0;
	PUCHAR			pDest;
	ULONG	Length;
	PUCHAR        	pOutBuffer = NULL;
	NDIS_STATUS   NStatus;
	PUCHAR Ptr = NULL;

	*pTotalFrameLen = 0;
	/* allocate and send out ProbeRsp frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	pFrame = (PP2P_PUBLIC_FRAME)pOutBuffer;
	Ptr = (PUCHAR) pFrame;
	if (MyRule == P2P_IS_CLIENT)
		ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Addr1);
	else
		ActHeaderInit(pAd, &pFrame->p80211Header, Addr1, pP2PCtrl->CurrentAddress, Bssid);
	DBGPRINT(RT_DEBUG_ERROR, ("P2P - P2PMakeInviteRsp MyRule = %s. TO %02x:%02x:%02x:%02x:%02x:%02x. \n", decodeMyRule(MyRule), PRINT_MAC(Addr1)));
	DBGPRINT(RT_DEBUG_ERROR, ("P2P - Bssid %02x:%02x:%02x:%02x:%02x:%02x. \n", PRINT_MAC(Bssid)));
	pFrame->Category = CATEGORY_PUBLIC;
	pFrame->Action = ACTION_WIFI_DIRECT;
	/* point to OUI */
	Ptr += (sizeof(HEADER_802_11) + 2);
	/*RTMPMoveMemory((pFrame->OUI), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	Ptr += 4;
	/* OUISubtype */
	pFrame->Subtype = P2P_INVITE_RSP;
	pFrame->Token = Token;
	pFrame->ElementID = IE_VENDOR_SPECIFIC;
	/* point to OUI2 */
	Ptr += 4;
	/*RTMPMoveMemory((pFrame->OUI2), P2POUIBYTE, 4);*/
	RTMPMoveMemory(Ptr, P2POUIBYTE, 4);
	P2pIeLen = 4;
	*pTotalFrameLen = 38;
	pDest = pFrame->Octet;
	/* attach subelementID= 0. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_STATUS, Status, pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 5. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CONFIG_TIMEOUT, &pP2PCtrl->ConfigTimeout[0], pDest);
	pDest += Length;
	P2pIeLen += Length;
	*pTotalFrameLen += Length;
	/* attach subelementID= 17. */
	if ((MyRule == P2P_IS_GO) && (*Status == P2PSTATUS_SUCCESS))
	{
		DBGPRINT(RT_DEBUG_TRACE, (" P2P - OpChannel =  %d \n", pP2PCtrl->GroupChannel));
		if (INFRA_ON(pAd))
			Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, &pAd->CommonCfg.Channel, pDest);
		else
			Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, OpChannel, pDest);
		pDest += Length;
		P2pIeLen += Length;
		*pTotalFrameLen += Length;
	}
	/* attach subelementID= 7. */
	if ((MyRule == P2P_IS_GO) && (*Status == P2PSTATUS_SUCCESS))
	{
		Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_GROUP_BSSID, Bssid, pDest);
		pDest += Length;
		P2pIeLen += Length;
		*pTotalFrameLen += Length;
	}
	/* attach subelementID= 7. */
	Length = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CHANNEL_LIST, OpChannel, pDest);
	pDest += Length;
	P2pIeLen += Length;
	/* attach subelementID= 17. */
	*pTotalFrameLen += Length;
	pFrame->Length = (UCHAR)P2pIeLen;

	MiniportMMRequest(pAd, 0, pOutBuffer, *pTotalFrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

}

	
/*	
	==========================================================================
	Description: 
		Sanity check of the Probe Request frame when operating as a P2P Device. 
		Can be called from both as AP's state machine or as STA( that is doing P2P search)'s state machine
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN PeerP2pProbeReqSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement, 
	OUT ULONG *WpsIELen, 
	OUT PUCHAR pWpsIE) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PFRAME_802_11		pFrame;
	PEID_STRUCT 			pEid;
	ULONG				Length = 0;
	BOOLEAN 			brc = FALSE;
	UCHAR			RateLen;
		
	pFrame = (PFRAME_802_11)Msg;
	Length += LENGTH_802_11;
	
	*P2PSubelementLen = 0;
	*WpsIELen = 0;
	*pSsidLen = 0;
	*Peerip = 0;
	RateLen = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

	pEid = (PEID_STRUCT) pFrame->Octet;

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen)	  
	{
		switch(pEid->Eid)
		{
			case IE_EXT_SUPP_RATES:
				/* concatenate all extended rates to Rates[] and RateLen */
				RateLen = (RateLen) + pEid->Len;
				break;
			case IE_SUPP_RATES:
				RateLen = (RateLen) + pEid->Len;
				break;
			case IE_SSID:
				if(pEid->Len <= MAX_LEN_OF_SSID)
				{
					RTMPMoveMemory(Ssid, pEid->Octet, pEid->Len);
					*pSsidLen = pEid->Len;
				}
				break;
			case IE_VENDOR_SPECIFIC:
				/* Check the OUI version, filter out non-standard usage */
				if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len > 4))
				{
					RTMPMoveMemory(pWpsIE, &pEid->Eid, pEid->Len +2);
					*WpsIELen = pEid->Len + 2;
				}
				if (NdisEqualMemory(pEid->Octet, P2POUIBYTE, 4) && (pEid->Len > 4))
				{
					if (*P2PSubelementLen == 0)
					{
						RTMPMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len+2);
						*P2PSubelementLen = pEid->Len+2;
						brc = TRUE;
					}
					else if (*P2PSubelementLen > 0)
					{
						RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
						*P2PSubelementLen += (pEid->Len+2);
						brc = TRUE;
					}

					DBGPRINT(RT_DEBUG_INFO, (" !FIND!!!!!!===>P2P - PeerP2pProbeReq P2P IE Len = %ld.   %s\n", *P2PSubelementLen, decodeP2PState(pP2PCtrl->P2PConnectState)));
				}
				break;
		}
		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len] */
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	
	}

	/* Doesn't parse probe request that only support 11b. So return FALSE. */
	if (RateLen <= 4)
	{
		brc = FALSE;
		DBGPRINT(RT_DEBUG_INFO, ("Ignore Probe Request that is 11b-only.\n"));
	}
	/* Check P2P's Probe Response validatity. */
	if (*P2PSubelementLen <= 4)
	{
		brc = FALSE;
		DBGPRINT(RT_DEBUG_INFO, ("Ignore Probe Request that doesn't have P2P IE.\n"));
	}

	return brc;

}

/*	
	==========================================================================
	Description: 
		Sanity check of the Probe Response frame when operating as a P2P Device. 
		Only called from STA's state machine that is in scanning.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN PeerP2pBeaconSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement) 
{
	PFRAME_802_11		pFrame;
	PEID_STRUCT         pEid;
	ULONG				Length = 0;
	BOOLEAN				brc = FALSE;
	PUCHAR				Ptr;
	BOOLEAN				bFirstP2pOUI = TRUE;
	
	pFrame = (PFRAME_802_11)Msg;
	Length += LENGTH_802_11;

	*P2PSubelementLen = 0;
	*pSsidLen = 0;
	*Peerip = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

	Ptr = pFrame->Octet;

	/* get timestamp from payload and advance the pointer */
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;

	/* get beacon interval from payload and advance the pointer */
	Ptr += 2;
	Length += 2;

	/* get capability info from payload and advance the pointer */
	Ptr += 2;
	Length += 2;

	pEid = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen)    
	{
		switch(pEid->Eid)
		{			
			case IE_SSID:
				if(pEid->Len <= MAX_LEN_OF_SSID)
				{
					RTMPMoveMemory(Ssid, pEid->Octet, pEid->Len);
					*pSsidLen = pEid->Len;
				}
				break;
			case IE_VENDOR_SPECIFIC:
				/* Check the OUI version, filter out non-standard usage */
				if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len >= 4))
				{
					if (*P2PSubelementLen == 0)
					{
						RTMPMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
						*P2PSubelementLen = pEid->Len +2;
					}
					else if (*P2PSubelementLen > 0)
					{
						RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
						*P2PSubelementLen += (pEid->Len+2);
					}
				}
				else if (NdisEqualMemory(pEid->Octet, P2POUIBYTE, 4) && (pEid->Len >= 4))
				{
					/*
						If this is the first P2P OUI. Then also append P2P OUI. 
						Beacon 's P2P attribute doesn't exceed 256 bytes. So not use acumulcated form.
					 */
					if (bFirstP2pOUI == TRUE)
					{
						if (*P2PSubelementLen == 0)
						{
							RTMPMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
							*P2PSubelementLen = (pEid->Len +2);
							brc = TRUE;
						}
						else if (*P2PSubelementLen > 0)
						{
							RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
							*P2PSubelementLen += (pEid->Len+2);
							brc = TRUE;
						}
						bFirstP2pOUI = FALSE;
					}
					else
					{
						/*
							If this is not the first P2P OUI. Then don't append P2P OUI.
							because our parse function doesn't need so many P2P OUI.
						 */
						if ((*P2PSubelementLen > 0) && (pEid->Len > 4))
						{
							RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
							*P2PSubelementLen += (pEid->Len+2);
							brc = TRUE;
						}
					}
				}
				break;
		}
		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len] */
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	
	}
	return brc;
}

/*	
	==========================================================================
	Description: 
		Sanity check of the Probe Response frame when operating as a P2P Device. 
		Only called from STA's state machine that is in scanning.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
BOOLEAN PeerP2pProbeRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement) 
{
	PFRAME_802_11		pFrame;
	PEID_STRUCT         pEid;
	ULONG				Length = 0;
	BOOLEAN				brc = FALSE;
	PUCHAR				Ptr;
	BOOLEAN				bFirstP2pOUI = TRUE;
	BOOLEAN				bLastIsP2pOUI = FALSE;
	PUCHAR				pP2PIeConLen = NULL;	/* pointer to 2 bytes to indicate Contenated length of all P2P IE */
	ULONG				P2PIeConLen = 0; 	/*  Contenated length of all P2P IE */
	ULONG			idx;

	pFrame = (PFRAME_802_11)Msg;
	Length = LENGTH_802_11;

	*P2PSubelementLen = 0;
	*pSsidLen = 0;
	*Peerip = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	
	Ptr = pFrame->Octet;

	/* get timestamp from payload and advance the pointer */
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;

	/* get beacon interval from payload and advance the pointer */
	Ptr += 2;
	Length += 2;

	/* get capability info from payload and advance the pointer */
	Ptr += 2;
	Length += 2;

	pEid = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen)    
	{
		switch(pEid->Eid)
		{			
			case IE_SSID:
				bLastIsP2pOUI = FALSE;
				if(pEid->Len <= MAX_LEN_OF_SSID)
				{
					RTMPMoveMemory(Ssid, pEid->Octet, pEid->Len);
					*pSsidLen = pEid->Len;
				}
				break;
			case IE_VENDOR_SPECIFIC:
				bLastIsP2pOUI = FALSE;
				/* Check the OUI version, filter out non-standard usage */
				if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len >= 4))
				{
					if (*P2PSubelementLen == 0)
					{
						RTMPMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
						*P2PSubelementLen = pEid->Len +2;
					}
					else if (*P2PSubelementLen > 0)
					{
						RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
						*P2PSubelementLen += (pEid->Len+2);
					}
				}
				else if (NdisEqualMemory(pEid->Octet, P2POUIBYTE, 4) && (pEid->Len >= 4))
				{
					brc = TRUE;
					bLastIsP2pOUI = TRUE;
					/* If this is the first P2P OUI. Then also append P2P OUI. */
					if (bFirstP2pOUI == TRUE)
					{
						/* Althought this is first P2P IE. */
						/* still need to Check *P2PSubelementLen, because *P2PSubelementLen also includes WPS IE. */
						if (*P2PSubelementLen == 0)
						{
							RTMPMoveMemory(pP2pSubelement, &pEid->Eid, 2);
							*(pP2pSubelement + 2) = 0;
							/* Make one more byte for P2P accumulated length. */
							RTMPMoveMemory(pP2pSubelement + 3, &pEid->Octet[0], pEid->Len);
							pP2PIeConLen = pP2pSubelement + *P2PSubelementLen + 1;
							*P2PSubelementLen = (pEid->Len + 3);
							P2PIeConLen = pEid->Len;	/* Real P2P IE length is Len. */
							DBGPRINT(RT_DEBUG_INFO, ("SYNC -1-1 P2PIeConLen  = %ld\n", P2PIeConLen));
						}
						else if (*P2PSubelementLen > 0)
						{
							RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, 2);
							*(pP2pSubelement + *P2PSubelementLen + 2) = 0;
							/* Make one more byte for P2P accumulated length. */
							RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen + 3, &pEid->Octet[0], pEid->Len);
							pP2PIeConLen = pP2pSubelement + *P2PSubelementLen + 1;
							*P2PSubelementLen += (pEid->Len+3);
							/* bFirstP2pOUI is TURE. So use =   */
							P2PIeConLen = pEid->Len;
							DBGPRINT(RT_DEBUG_INFO, (" -1-2 P2PIeConLen  = %ld\n", P2PIeConLen));
						}
						bFirstP2pOUI = FALSE;
					}
					else if (bLastIsP2pOUI == TRUE)
					{
						/* If this is not the first P2P OUI. Then don't append P2P OUI. */
						/* because our parse function doesn't need so many P2P OUI. */
						if ((*P2PSubelementLen > 0) && (pEid->Len > 4))
						{
							RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Octet[4], pEid->Len-4);
							*P2PSubelementLen += (pEid->Len-4);
							P2PIeConLen += (pEid->Len - 4);
						}
					}
				}
				break;
			default : 
				bLastIsP2pOUI = FALSE;
				break;

		}
		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len] */
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	
	}

	if ((P2PIeConLen != 0) && (pP2PIeConLen != NULL))
	{
		*pP2PIeConLen = (UCHAR)(P2PIeConLen%256);
		*(pP2PIeConLen+1) = (UCHAR)(P2PIeConLen/256);
		DBGPRINT(RT_DEBUG_INFO, ("  - 3 P2PIeConLen  = %ld. /256 = %ld. *P2PSubelementLen = %ld \n", P2PIeConLen, (P2PIeConLen/256), *P2PSubelementLen));
		DBGPRINT(RT_DEBUG_INFO, ("  -  %x %x \n", *pP2PIeConLen,  *(pP2PIeConLen+1) ));
		for (idx = 0; idx < (*P2PSubelementLen);)
		{
			DBGPRINT_RAW(RT_DEBUG_INFO, ("%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-", 
				*(pP2pSubelement+idx), *(pP2pSubelement+idx+1), *(pP2pSubelement+idx+2), *(pP2pSubelement+idx+3)
				,*(pP2pSubelement+idx+4) ,*(pP2pSubelement+idx+5) ,*(pP2pSubelement+idx+6),*(pP2pSubelement+idx+7)
				,*(pP2pSubelement+idx+8),*(pP2pSubelement+idx+9),*(pP2pSubelement+idx+10),*(pP2pSubelement+idx+11)));
			
			 idx = idx + 12;
		}
		
	}
	return brc;
}

VOID P2pPeerBeaconAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem,
	IN PUCHAR		Bssid) 
{
	/*PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;*/
	UCHAR		P2pManageability = 0xff;
	UCHAR		NumOfOtherP2pAttri = 1;
	UCHAR		Channel = 1;
	UCHAR		TotalNumOfP2pAttribute = 1;

	if (Elem->MsgLen <= (LENGTH_802_11 + 12))
		return;
	
	P2pParseManageSubElmt(pAd, 
						&Elem->Msg[LENGTH_802_11 + 12], 
						(Elem->MsgLen - LENGTH_802_11 - 12), 
		&Channel,
		&NumOfOtherP2pAttri,
		&TotalNumOfP2pAttribute,
						&P2pManageability,
						NULL);
	
	DBGPRINT(RT_DEBUG_INFO, ("P2pParseManageSubElmt  TotalNumOfP2pAttribute = %d.  \n", TotalNumOfP2pAttribute));
	/* If this AP carries Managed Attribute. Update to our ManageAPBSsid */
	if (P2pManageability != 0xff)
	{
		DBGPRINT(RT_DEBUG_INFO, ("SYNC - Receive desired BEACON with valid P2pManageability= %x\n", P2pManageability));
		DBGPRINT(RT_DEBUG_INFO, ("SYNC -NumOfOtherP2pAttri %d . Channel = %d. \n", NumOfOtherP2pAttri, Channel));
		RTMPMoveMemory(pAd->P2pCfg.P2pManagedParm.ManageAPBSsid, Bssid, MAC_ADDR_LEN);

		/* Reset Minor Reason when connecting to Managed AP. */
		pAd->P2pCfg.P2pManagedParm.APP2pMinorReason = 0;
		pAd->P2pCfg.P2pManagedParm.APUsageChannel = Channel ;
		/* If this is the latest managed AP that I connected to. but this AP turn off the managed function. */
	/* I should clear my record. */
	
	/* If this is a manged AP. update to the ManageAPBSsid. */
		if ((P2P_TEST_BIT(P2pManageability, P2PMANAGED_ENABLE_BIT)) && (NumOfOtherP2pAttri == 0))
		{
			DBGPRINT(RT_DEBUG_INFO, ("SYNC -I am connecting to a Managed AP. Save this Bssid. %x %x %x \n", Bssid[3],Bssid[4],Bssid[5]));
		}
	
		pAd->P2pCfg.P2pManagedParm.APP2pManageability = P2pManageability;
	}
	pAd->P2pCfg.P2pManagedParm.TotalNumOfP2pAttribute = TotalNumOfP2pAttribute;

	DBGPRINT(RT_DEBUG_INFO, ("SYNC - BEACON P2pManageability= %x. TotalNumOfP2pAttribute = %d. OtherP2pAttri = %d\n", P2pManageability, TotalNumOfP2pAttribute, NumOfOtherP2pAttri));

}

/*	
	==========================================================================
	Description: 
		Processing the Probe Response frame when operating as a P2P Device. 
		Only called from STA's state machine that is in scanning.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID PeerP2pBeaconProbeRspAtScan(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
/*	IN USHORT CapabilityInfo, */
/*	IN UCHAR	WorkingChannel) */
{
		PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
		PRT_P2P_CLIENT_ENTRY pP2pEntry = NULL;
		BOOLEAN 	bBeacon = FALSE;
		BOOLEAN 	bresult;
		BOOLEAN		bChangeState = FALSE;
		ULONG	P2PSubelementLen;
		PUCHAR	P2pSubelement = NULL;
		UCHAR	index;/*, perstindex; */
		UCHAR	Addr2[6], SsidLen;
		UCHAR	Ssid[32];
		ULONG		Peerip;
		UCHAR		DevType[8], DevAddr[6], Channel = 0, OpChannel = 0, Intent;
		UCHAR		GroupCap, DeviceCap, StatusCode;
		USHORT		Dpid;
		PHEADER_802_11	pHeader;
		USHORT		ConfigMethod = 0xffff;
		UCHAR		DeviceNameLen = 0, DeviceName[32];
		UCHAR		SavedP2PTableNum;
		P2P_CMD_STRUCT	P2pCmd;
		BOOLEAN bSendP2pEvent = FALSE;
		
		/* If there is already BSS or IBSS and only one port, no need to parse Probe response for P2P discovery feature */
		/* INFRA on includes case that I am P2P client. in this case, doesn't need to parse P2P IE in Probe Response either?? */
	/*	if ((INFRA_ON(pAd) || ADHOC_ON(pAd)) && (pAd->OpMode == OPMODE_STA))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("1 PeerP2pBeaconProbeRspAtScan:: %s \n", decodeP2PState(pP2PCtrl->P2PDiscoProvState)));
			return;
		}
	*/

		/* Init P2pSubelement */
		os_alloc_mem(pAd, &P2pSubelement, MAX_VIE_LEN);

		if (P2pSubelement == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("PeerP2pBeaconProbeRspAtScan::Allocate memory size(=1024) failed\n"));
			goto CleanUp;
		}
		RTMPZeroMemory(&DeviceName[0], 32);

		/* P2P Device's probe response doesn't set ess bit on.  */
		pHeader = (PHEADER_802_11)&Elem->Msg[0];
		/* In P2P spec, the P2P IE in probe response and beacon are different.  */
		/* So set a boolean to check if this is a beacon frame. */
		if (pHeader->FC.SubType == SUBTYPE_BEACON)
			bBeacon = TRUE;

		/* Intel put P2P IE into two separate IE */
		/* Sanity check */
		if (PeerP2pProbeRspSanity(pAd, 
									Elem->Msg, 
									Elem->MsgLen, 
									Addr2, 
									Ssid, 
									&SsidLen, 
									&Peerip,
									&P2PSubelementLen,
									P2pSubelement))
		{
			CHAR Rssi;
			/* Check P2P's Probe Response validatity. */
			if (!(NdisEqualMemory(Ssid, &WILDP2PSSID[0], WILDP2PSSIDLEN)))
			{
				DBGPRINT(RT_DEBUG_INFO, ("PeerP2pBeaconProbeRspAtScan: Len = %ld.: \n", P2PSubelementLen));
				DBGPRINT(RT_DEBUG_INFO, ("= %02x:%02x:%02x:%02x:%02x:%02x	\n",  PRINT_MAC(Addr2)));
				DBGPRINT(RT_DEBUG_INFO, ("= %c %c %c %c %c %c%c \n",  Ssid[0], Ssid[1],Ssid[2],Ssid[3],Ssid[4],Ssid[5],Ssid[6]));
				goto CleanUp;
			}
			else
				DBGPRINT(RT_DEBUG_INFO, ("!! %s = %02x:%02x:%02x:%02x:%02x:%02x!!\n",  __FUNCTION__, PRINT_MAC(Addr2)));
			Rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));

			/* Step 1:	Parse P2P attribute */
			/* If this peer is provisioned, don't update Config */
			if (bBeacon == FALSE)
			{
				P2pParseSubElmt(pAd, (PVOID)P2pSubelement, P2PSubelementLen, 
					bBeacon, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, NULL, NULL, NULL, &ConfigMethod, NULL, DevType, &Channel, &OpChannel, NULL, &Intent, &StatusCode, NULL);
			}
			else
				P2pParseSubElmt(pAd, (PVOID)P2pSubelement, P2PSubelementLen, 
					bBeacon, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, NULL, NULL, NULL, NULL, NULL, DevType, &Channel, &OpChannel, NULL, &Intent, &StatusCode, NULL);

			/* Step 3:	 Insert. */
			SavedP2PTableNum = pAd->P2pTable.ClientNumber;
			index = P2pGroupTabSearch(pAd, DevAddr);
			if (index == P2P_NOT_FOUND)
			{
				/* Because only Probe Response carrys group info,  */
				/* If we use Probe Response to add to my P2P table structure. If this is beacon  */
				/* and in social channels. return. */
				if (bBeacon == TRUE)
					goto CleanUp;

				DBGPRINT(RT_DEBUG_ERROR, ("2 New From Addr2 = %02x:%02x:%02x:%02x:%02x:%02x. bBeacon = %d . GroupCap = %x \n",	PRINT_MAC(Addr2), bBeacon, GroupCap));
				DBGPRINT(RT_DEBUG_ERROR, ("2 New From DevAddr = %02x:%02x:%02x:%02x:%02x:%02x. bBeacon = %d . GroupCap = %x \n",  PRINT_MAC(DevAddr), bBeacon, GroupCap));
				if ((bBeacon == TRUE) || ((GroupCap&GRPCAP_OWNER) == GRPCAP_OWNER))
				{
					index = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY_GO, Ssid, SsidLen, DeviceCap, GroupCap);

				}
				/* P2P client can't send probe response. So the probe response must be from P2P device. */
				else
				{
					index = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_DISCOVERY, Ssid, SsidLen, DeviceCap, GroupCap);
				}
				bSendP2pEvent = TRUE;
			}

			/* No matter the index is from existing table, or newly added, always update Ssid Information If this peer is not connected. */
			/* Step 4:	 Update table. */
			if (index < MAX_P2P_GROUP_SIZE)
			{
				pP2pEntry = &pAd->P2pTable.Client[index];
				DBGPRINT(RT_DEBUG_INFO, ("%s:: got ProbeResp.index = %d. statue = %s\n", __FUNCTION__, index, decodeP2PClientState(pAd->P2pTable.Client[index].P2pClientState)));
				pP2pEntry->DevCapability = DeviceCap;
				pP2pEntry->GroupCapability = GroupCap;
				pP2pEntry->Rssi = Rssi;


				/* Update Rule according to latest Probe Response or Beacon that I received. */
				if ((bBeacon == TRUE) && 
					((pP2pEntry->P2pClientState == P2PSTATE_DISCOVERY_CLIENT) || (pP2pEntry->P2pClientState == P2PSTATE_DISCOVERY)))
				{
						/* If total P2P device number > 10, don't want keep update topology if topology change. */
						if ((pAd->P2pTable.ClientNumber < 10)
							|| (((pAd->P2pTable.ClientNumber%4) == 3)&&(pAd->P2pTable.ClientNumber >= 10)))
						bChangeState = TRUE;
						pP2pEntry->Rule = P2P_IS_GO;
						pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY_GO;
						DBGPRINT(RT_DEBUG_ERROR, ("3 From DevAddr = %02x:%02x:%02x:%02x:%02x:%02x.	Change rule to GO GroupCap = %x \n",  PRINT_MAC(DevAddr), GroupCap));
				}
				else if ((pP2pEntry->P2pClientState == P2PSTATE_DISCOVERY_GO)
					&& ((GroupCap&GRPCAP_OWNER) != GRPCAP_OWNER))
				{
					if ((pAd->P2pTable.ClientNumber < 10)
						|| (((pAd->P2pTable.ClientNumber % 4) == 3) && (pAd->P2pTable.ClientNumber >= 10)))
					bChangeState = TRUE;
					pP2pEntry->Rule = P2P_IS_CLIENT;
					pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY;
					DBGPRINT(RT_DEBUG_ERROR, ("4 From DevAddr = %02x:%02x:%02x:%02x:%02x:%02x.	Change rule to Device. GroupCap = %x \n",  PRINT_MAC(DevAddr), GroupCap));
				}
				

				
				/* If peer is provisioned, don't update. We treat him only support this provisioned configmethod. */
				if (!P2P_TEST_FLAG(pP2pEntry, P2PFLAG_PROVISIONED) && (ConfigMethod != 0xffff))
					pP2pEntry->ConfigMethod = ConfigMethod;

				/* Always Update Device Name */
				if (DeviceNameLen != 0)
				{
					RTMPMoveMemory(&pP2pEntry->DeviceName[0], DeviceName, 32);
					pP2pEntry->DeviceNameLen = DeviceNameLen;
					pP2pEntry->DeviceName[pP2pEntry->DeviceNameLen] = 0x0;
				}

				/* If this is beacon. It must use correct opchannel, ssid, interface addr, and SSID. So update. */
				/* Step 4 - 1:	 Update table according to this is beacon or probe response. Beacon and probe response carries */
				/* different attribute.  */
				if (bBeacon == TRUE)
				{
					pP2pEntry->Rule = P2P_IS_GO;
					RTMPMoveMemory(pP2pEntry->Ssid, Ssid, 32);
					pP2pEntry->SsidLen = SsidLen;
					RTMPMoveMemory(pP2pEntry->bssid, Addr2, MAC_ADDR_LEN);
					RTMPMoveMemory(pP2pEntry->InterfaceAddr, Addr2, MAC_ADDR_LEN);
					pP2pEntry->OpChannel = Elem->Priv; /* WorkingChannel; */
					/* The peer is GO. So set its state to GO_WPS directly. No need for Group forming procedure. */
					/* Search this beacon's group info, and insert its client to my p2p table too. */
					if ((GroupCap & GRPCAP_OWNER) != GRPCAP_OWNER)
					{
						/* Print for debug. */
						DBGPRINT(RT_DEBUG_ERROR, ("P2p : One P2P device[%d] send out Beacon. But group owner %x bit not set ? \n", index, GroupCap));
					}
				}
				/* Make it more readable to use "else if". Group info only appears in Probe Response. */
				else if (bBeacon == FALSE)
				{
					/* If this is GO. */			
					if ((GroupCap & GRPCAP_OWNER) == GRPCAP_OWNER)
					{
						/* The peer is GO. So set its state to GO_WPS directly. No need for Group forming procedure. */
						pP2pEntry->Rule = P2P_IS_GO;
					}
					else
					{
						/* Not GO anymore, update State. */
						if (IS_P2P_PEER_DISCOVERY(pP2pEntry))
							pP2pEntry->P2pClientState = P2PSTATE_DISCOVERY;
						pP2pEntry->Rule = P2P_IS_CLIENT;
					}
					/* P2P PrimaryDevType only appears in Probe Response. not in beacon. */
					RTMPMoveMemory(pP2pEntry->PrimaryDevType, DevType, 8);
					/* Can I just set this channel that I got Probe response as listent channel?? */
					if (Elem->Priv)
						pP2pEntry->ListenChannel = Elem->Priv;
					bresult = P2pParseGroupInfoAttribute(pAd, index, P2pSubelement, P2PSubelementLen);
					if (bresult == FALSE)
						goto CleanUp;
				}

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
				if (bSendP2pEvent)
					P2pSendWirelessEvent(pAd, RT_P2P_DEVICE_FIND, pP2pEntry, Addr2);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
				/* Step 5: Take Some action when the peer is  */
				/* Decide to connect? or Provision ? or Service discovery ? */
				if ((pP2pEntry->P2pClientState <= P2PSTATE_GO_DONE))
				{
					if ((DeviceCap & DEVCAP_INVITE) == DEVCAP_INVITE)
						P2P_SET_FLAG(pP2pEntry, P2PFLAG_INVITE_ENABLED);

					if (pP2pEntry->P2pClientState != P2PSTATE_DISCOVERY)
					{
						UCHAR ClientState = pP2pEntry->P2pClientState;
						COPY_MAC_ADDR(&P2pCmd.Addr[0], pP2pEntry->addr);
						P2pCmd.Idx = index;
						pP2pEntry->ReTransmitCnt = 0;
						/*P2pCmd.ConfigMethod = pAd->P2pTable.Client[index].ConfigMethod; */
						DBGPRINT(RT_DEBUG_INFO, ("P2p : DevAddr[%02x:%02x:%02x:%02x:%02x:%02x] State = %s.\n", 
								PRINT_MAC(DevAddr), decodeP2PClientState(pP2pEntry->P2pClientState)));
						DBGPRINT(RT_DEBUG_INFO, ("P2p : GrpCap=%x. DevCap=%x. ConfigMethod= %x.\n", 
								GroupCap, DeviceCap, pP2pEntry->ConfigMethod));

						MlmeEnqueue(pAd, P2P_GO_FORM_STATE_MACHINE, P2P_START_COMMUNICATE_CMD_EVT, sizeof(P2P_CMD_STRUCT), &P2pCmd, ClientState);
						RTMP_MLME_HANDLER(pAd);
						goto CleanUp;
					}
					if (pAd->P2pCfg.ConnectingIndex < MAX_P2P_GROUP_SIZE)
					{
						if (MAC_ADDR_EQUAL(pAd->P2pTable.Client[index].addr, &pAd->P2pCfg.ConnectingMAC[0]))
						{
							DBGPRINT(RT_DEBUG_INFO, ("    From : %02x:%02x:%02x:%02x:%02x:%02x, state = %s\n", PRINT_MAC(Addr2), decodeP2PClientState(pP2pEntry->P2pClientState)));
							DBGPRINT(RT_DEBUG_INFO, ("    P2PTab[%d] Addr : %02x:%02x:%02x:%02x:%02x:%02x\n", index, PRINT_MAC(pP2pEntry->addr)));
							DBGPRINT(RT_DEBUG_INFO, ("    ConenctIdx = %d, Addr : %02x:%02x:%02x:%02x:%02x:%02x\n", pAd->P2pCfg.ConnectingIndex, PRINT_MAC(pAd->P2pCfg.ConnectingMAC)));

							P2pConnectAfterScan(pAd, bBeacon, index);
						}
					}
				}
			}
		}

CleanUp:
		if (P2pSubelement)
			os_free_mem(pAd, P2pSubelement);

}

/*	
	==========================================================================
	Description: 
		Processing the Beacon frame when operating as a P2P client. 
		Only called from STA's state machine that is in idle.
		this function can support NoA and show Ralink IP.
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID PeerP2pBeacon(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	pAddr2,
	IN MLME_QUEUE_ELEM *Elem,
	IN LARGE_INTEGER   TimeStamp) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	UCHAR	Addr2[6], SsidLen;
	UCHAR	Ssid[32];
	ULONG		Peerip;
	ULONG	P2PSubelementLen;
	PUCHAR	P2pSubelement = NULL;
	PFRAME_802_11		pFrame;
	/*UCHAR	bitmap;*/
	PMAC_TABLE_ENTRY pMacEntry = NULL;

	pFrame = (PFRAME_802_11)Elem->Msg;
	/* Only check beacon . */
	if (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP)
		return;

	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

	pMacEntry = &pAd->MacTab.Content[Elem->Wcid];
	if (!(IS_P2P_CLI_ENTRY(pMacEntry) && IS_ENTRY_APCLI(pMacEntry)))
	{
		DBGPRINT(RT_DEBUG_INFO, ("1 PeerP2pBeaconProbeRspAtScan failed :  wcid = %d. not ValidAsP2P. Bug!please check. \n", Elem->Wcid));
		DBGPRINT(RT_DEBUG_INFO, ("pMacEntry Addr = %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pMacEntry->Addr)));
		return;
	}

	if ((pMacEntry->WpaState != AS_PTKINITDONE))
		return;

	/* Init P2pSubelement */
	os_alloc_mem(pAd, &P2pSubelement, MAX_VIE_LEN);
	if (P2pSubelement == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("3 PeerP2pBeaconProbeRspAtScan::1Allocate memory size(=1024) failed\n"));
		goto CleanUp;
	}
	
	if (PeerP2pBeaconSanity(pAd, 
									Elem->Msg, 
									Elem->MsgLen, 
									Addr2, 
									Ssid, 
									&SsidLen, 
									&Peerip,
									&P2PSubelementLen,
									P2pSubelement))
	{
		/* Parse the power managemenr parameters in here. */
		pP2PCtrl->GONoASchedule.LastBeaconTimeStamp = TimeStamp.u.LowPart;
		P2pParseNoASubElmt(pAd, P2pSubelement, P2PSubelementLen, Elem->Wcid, pFrame->Hdr.Sequence);
		/* Since we get beacon, check if GO enable and OppPS. */
		if (P2P_TEST_BIT(pAd->P2pCfg.CTWindows, P2P_OPPS_BIT))
		{
			pAd->P2pCfg.bKeepSlient = FALSE;
			/* TO DO : sync with windows if necessary */
			/*RTMPDeQueueNoAMgmtPacket(pAd);*/
			RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS); 	/* Dequeue outgoing frames from TxSwQueue0..3 queue and process it */
			if (((pAd->P2pCfg.CTWindows&0x7f) > 0) && ((pAd->P2pCfg.CTWindows&0x7f) < 80))
			{
				DBGPRINT(RT_DEBUG_INFO, ("%s::  set P2P CTWindows timer.\n", __FUNCTION__));
				RTMPSetTimer(&pAd->P2pCfg.P2pCTWindowTimer, (pAd->P2pCfg.CTWindows&0x7f));
			}
		}
	}
	
CleanUp:
	if (P2pSubelement)
		os_free_mem(NULL, P2pSubelement);

}

/*	
	==========================================================================
	Description: 
		Processing the Beacon frame when operating as a P2P client. 
		Only called from STA's state machine that is in idle.
		
	Parameters:
		*pPeerip : Go's ip.
		*pMemberip : Other member's ip that is in the same P2P Group.
	Note:
		 
	==========================================================================
 */
BOOLEAN PeerBeaconParseRalinkIE( 
		IN PRTMP_ADAPTER pAd, 
		IN VOID *Msg, 
		IN ULONG MsgLen, 
		OUT RALINKIP_IE				*pRalinkIE,
		OUT RALINKMBRIP_ELEM				*pMemberip,
		OUT ULONG *pPeerip)
{
		PFRAME_802_11		pFrame;
		PEID_STRUCT 		pEid;
		ULONG				Length = 0;
		BOOLEAN 			brc = FALSE;
		PUCHAR				Ptr;
	
		pFrame = (PFRAME_802_11)Msg;
		Length += LENGTH_802_11;
	
		*pPeerip = 0;
		NdisZeroMemory(pRalinkIE, sizeof(RALINKIP_IE));
		NdisZeroMemory(pMemberip, sizeof(RALINKMBRIP_ELEM));
		
		Ptr = pFrame->Octet;
	
		/* get timestamp from payload and advance the pointer */
		Ptr += TIMESTAMP_LEN;
		Length += TIMESTAMP_LEN;
	
		/* get beacon interval from payload and advance the pointer */
		Ptr += 2;
		Length += 2;
	
		/* get capability info from payload and advance the pointer */
		Ptr += 2;
		Length += 2;
	
		pEid = (PEID_STRUCT) Ptr;

		/* get variable fields from payload and advance the pointer */
		while ((Length + 2 + pEid->Len) <= MsgLen)	  
		{
			switch(pEid->Eid)
			{	
				case IE_VENDOR_SPECIFIC:
					if (NdisEqualMemory(pEid->Octet, RALINK_OUI, 3) && (pEid->Len == 5))
					{
						RTMPMoveMemory(pRalinkIE, &pEid->Eid, sizeof(RALINKIP_IE));
						brc = TRUE;
					}
					else if (NdisEqualMemory(pEid->Octet, RALINK_OUI, 3) && (pEid->Len == 8))
					{
						if (pEid->Octet[3] == RALINKOUIMODE_IPRSP)
						{
							*pPeerip = *(PULONG)&pEid->Octet[4];
							brc = TRUE;
						}
					}
					else if (NdisEqualMemory(pEid->Octet, RALINK_OUI, 3))
					{
						/* Beacon broadcast's other peer's ip in Probe rsp. */
						if (pEid->Octet[3] == RALINKOUIMODE_MBRIPRSP)
						{
							RTMPMoveMemory(pMemberip, &pEid->Octet[4], sizeof(RALINKMBRIP_ELEM));
							brc = TRUE;
						}
					}
					break;
				default:
					break;
			}
	
			Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len] */
			pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
		}
	
	
	return brc;

}

/*	
	==========================================================================
	Description: 
		Processing Probe Request frame when operating as a P2P Device. 
		Can be called from both as AP's state machine or as STA( that is doing P2P search)'s state machine
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID PeerP2pProbeReq(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
/*	IN BOOLEAN	bSendRsp)  */
{
		PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
		ULONG			P2PSubelementLen, WpsIELen;
		UCHAR			*P2pSubelement;
		UCHAR			*WpsIE;
		NDIS_STATUS   NStatus;
		PUCHAR		  pOutBuffer = NULL;
		ULONG		  FrameLen = 0;
		UCHAR		   Addr2[MAC_ADDR_LEN];
		UCHAR			Ssid[MAX_LEN_OF_SSID], SsidLen;
		UCHAR		index, perstindex;
		/*BOOLEAN 	bNewlyAdd = FALSE; */
		ULONG		Peerip;
		/*RALINKIP_IE 	RalinkIp;*/
		BOOLEAN 	bresult = FALSE;
		UCHAR		GroupCap, DeviceCap, DevAddr[MAC_ADDR_LEN], StatusCode;
		BOOLEAN		bSendRsp = TRUE;
		UCHAR		Channel = Elem->Priv;	

		/* When I am a P2P Client , can't send probe response. */
		if (P2P_CLI_ON(pAd))
			return;

		if ( pP2PCtrl->bSentProbeRSP != TRUE )
			return;

		/* When I am not in listen channel , can't send probe response. */
		if (Channel != pAd->P2pCfg.ListenChannel)
		{
			bSendRsp = FALSE;
			if (P2P_GO_ON(pAd) && (Channel == pAd->P2pCfg.GroupOpChannel))
				bSendRsp = TRUE;			
		}

		P2pSubelement = NULL;
		WpsIE = NULL;

		os_alloc_mem(pAd, &P2pSubelement, MAX_VIE_LEN);
		os_alloc_mem(pAd, &WpsIE, MAX_VIE_LEN);

		if ((P2pSubelement == NULL) || (WpsIE == NULL))
			goto CleanUp;

		if (PeerP2pProbeReqSanity(pAd, 
									Elem->Msg, 
									Elem->MsgLen, 
									Addr2, 
									Ssid, 
									&SsidLen, 
									&Peerip,
									&P2PSubelementLen,
									P2pSubelement,
									&WpsIELen,
									WpsIE))
		{
			if (FALSE == P2PDeviceMatch(pAd, Addr2, NULL, 0))
				goto CleanUp;
			
			DBGPRINT(RT_DEBUG_INFO, ("P2P Peer Probe Req from %02x %02x %02x %02x %02x %02x	\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
			/*if ((RalinkIp.OUIMode == RALINKOUIMODE_IPREQ) 
				&& (pAd->P2pCfg.P2pEventQueue.bGotMyip == TRUE))
			{
				pP2PCtrl->P2pEventQueue.bSendMyip = TRUE;
				DBGPRINT(RT_DEBUG_INFO,("Set bSendMyip = TRUE for this probe request.  \n"));
			}
			if (Peerip != 0)
			{
				DBGPRINT(RT_DEBUG_INFO,("Client[%d] ip = %x. \n", index, Peerip));
				if (index < MAX_P2P_GROUP_SIZE)
				{
					if (pAd->P2pTable.Client[index].Peerip != Peerip)
					{
						pAd->P2pTable.Client[index].Peerip = Peerip;
						pAd->P2pCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
	
						DBGPRINT(RT_DEBUG_INFO,("Update Client[%d] ip = %x. \n", index, Peerip));
						P2pUpdateBssBeacon(pAd, NULL, &index);
					}
				}
			}*/
			index = P2pGroupTabSearch(pAd, Addr2);
	
				if (index < MAX_P2P_GROUP_SIZE)
				{
				DBGPRINT(RT_DEBUG_INFO, ("%s / %s\n", 
					decodeP2PClientState(pAd->P2pTable.Client[index].P2pClientState), decodeP2PState(pAd->P2pCfg.P2PConnectState)));
				if ((pAd->P2pTable.Client[index].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN) || 
					(pAd->P2pCfg.P2PConnectState != P2P_CONNECT_IDLE))
					bSendRsp = TRUE;
				}

			DBGPRINT(RT_DEBUG_TRACE, ("(%d) P2P Peer Probe Req from %02x %02x %02x %02x %02x %02x	\n", 
								bSendRsp, Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));	
	
			if (bSendRsp == TRUE)
			{				
				/* allocate and send out ProbeRsp frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
				if (NStatus != NDIS_STATUS_SUCCESS)
					goto CleanUp;
	
	
				P2PMakeProbe(pAd, Elem, Channel, SUBTYPE_PROBE_RSP, pOutBuffer, &FrameLen);


				if (FrameLen > 0)
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

				
				DBGPRINT(RT_DEBUG_INFO, ("Got P2P Peer %d Probe Req . Send probe response back  len=%ld \n", index, FrameLen));
				MlmeFreeMemory(pAd, pOutBuffer);
			}
			/* Check already in table ? */
			/* If currently there is no activated P2P profile, we can still check if there is matching peer that is */
			/* in Persistent table send probe response.  If there is one, maybe we can try to connect to it. */
	
			if (index < MAX_P2P_GROUP_SIZE)
			{
				P2pParseSubElmt(pAd, P2pSubelement, P2PSubelementLen, 
					FALSE, NULL, &GroupCap, &DeviceCap, NULL, NULL, DevAddr, NULL, NULL, &SsidLen, Ssid, NULL, NULL, pAd->P2pTable.Client[index].PrimaryDevType, &pAd->P2pTable.Client[index].ListenChannel, &pAd->P2pTable.Client[index].OpChannel, NULL, &pAd->P2pTable.Client[index].GoIntent, &StatusCode, NULL);
				if (pAd->P2pTable.Client[index].P2pClientState == P2PSTATE_CONNECT_COMMAND)
				{
					P2P_CMD_STRUCT	P2pCmd;
					UCHAR ClientState = pAd->P2pTable.Client[index].P2pClientState;
					DBGPRINT(RT_DEBUG_ERROR, ("<< %s: Start Group Form!\n", __FUNCTION__));
					COPY_MAC_ADDR(&P2pCmd.Addr[0], pAd->P2pTable.Client[index].addr);
					P2pCmd.Idx = index;
					/*P2pStartGroupForm(pAd, pAd->P2pTable.Client[index].addr, index); */
				}
				else if (pAd->P2pTable.Client[index].P2pClientState == P2PSTATE_INVITE_COMMAND)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("<< %s: Invite!\n", __FUNCTION__));

					if (IS_P2P_CONNECT_IDLE(pAd))
					{
						perstindex = P2pPerstTabSearch(pAd, pAd->P2pTable.Client[index].addr, pAd->P2pTable.Client[index].bssid, pAd->P2pTable.Client[index].InterfaceAddr);
						bresult = P2pInvite(pAd, pAd->P2pTable.Client[index].addr, perstindex, index);
					}
					else if (P2P_GO_ON(pAd))
					{
						/* Invite Case 1 */
						bresult = P2pInvite(pAd, pAd->P2pTable.Client[index].addr, MAX_P2P_TABLE_SIZE, index);
	
					}
					else if (P2P_CLI_ON(pAd))
					{
						/* Invite Case 1 */
						bresult = P2pInvite(pAd, pAd->P2pTable.Client[index].addr, MAX_P2P_TABLE_SIZE, index);
					}
	
					if (bresult == TRUE)
					{
						P2pStopScan(pAd);
						pP2PCtrl->P2PConnectState = P2P_INVITE;
					}
				}

			}
			
		}
	CleanUp:
	
		if (P2pSubelement)
			os_free_mem(NULL, P2pSubelement);
		if (WpsIE)
			os_free_mem(NULL, WpsIE);

}

/*	
	==========================================================================
	Description: 
		Call this function when receiving WPS EAP Nack frame. Most of time is because incorrect PIN.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pReceiveEapNack(
	IN PRTMP_ADAPTER pAd,
	IN	PMLME_QUEUE_ELEM	pElem)
{
}


VOID P2pMakeProbeRspWSCIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR			pOutBuf,
	OUT	PULONG			pIeLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	UCHAR			WscIEFixed[] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x04};	/* length will modify later */

	
	ULONG			Len;
	PUCHAR			pData;
	PWSC_DEV_INFO	pDevInfo;	


	/* Role play, Enrollee or Registrar */
	pDevInfo = &pP2PCtrl->DevInfo;

	pData = pOutBuf;
	Len = 0;
	*pIeLen = 0;

	/* 0. WSC fixed IE */
	RTMPMoveMemory(pData, &WscIEFixed[0], 6);
	pData += 6;
	Len += 6;
			
	/* 1. Version */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_VERSION);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
	*(pData + 4) = pDevInfo->Version;
	pData += 5;
	Len   += 5;
	/* 2. Wi-Fi Protected Setup State */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_SC_STATE);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
	*(pData + 4) = (pAd->P2pCfg.bConfiguredAP ? WSC_SCSTATE_CONFIGURED : WSC_SCSTATE_UNCONFIGURED);
	pData += 5;
	Len   += 5;



	/* 3. Response Type WSC_ID_RESP_TYPE */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_RESP_TYPE);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
	if (P2P_GO_ON(pAd))
		*(pData + 4) = WSC_MSGTYPE_AP_WLAN_MGR;
	else
		*(pData + 4) = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;
	pData += 5;
	Len   += 5;

	/* 4. UUID-E */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_UUID_E);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0010);	
	NdisMoveMemory((pData + 4), pDevInfo->Uuid, 16);
	pData += 20;
	Len   += 20;

	// 5. Manufacture
	*((PUSHORT) pData) = cpu2be16(WSC_ID_MANUFACTURER);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
	*(pData + 4) = 0x20;
	pData += 5;
	Len   +=  5;

	/* We don't support full length manufacture, model name, model name and model serial  */
	/* because it shall overflow. (> 255 bytes) */
	/* 6. Model Name */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_MODEL_NAME);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
	*(pData + 4) = 0x20;
	pData += 5;
	Len   += 5;
		
	/* 7. Model Number */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_MODEL_NUMBER);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001); 
	*(pData + 4) = 0x20;
	pData += 5;
	Len   += 5;
		
	/* 8. Serial Number */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_SERIAL_NUM);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001); 
	*(pData + 4) = 0x20;
	pData += 5;
	Len   += 5;

	/* 9. Primary device type */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_PRIM_DEV_TYPE);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0008);	
	NdisMoveMemory((pData + 4), pDevInfo->PriDeviceType, 8);
	pData += 12;
	Len   += 12;

	/* 10. Primary device name */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_DEVICE_NAME);
	*((PUSHORT) (pData + 2)) = cpu2be16(pAd->P2pCfg.DeviceNameLen); 
	NdisMoveMemory((pData + 4), pAd->P2pCfg.DeviceName, pAd->P2pCfg.DeviceNameLen);
	pData += pAd->P2pCfg.DeviceNameLen + 4;
	Len   += pAd->P2pCfg.DeviceNameLen + 4;

	/* 11. Config Method */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_CONFIG_METHODS);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
	*((PUSHORT) (pData + 4)) = cpu2be16(0x0188);

	pData += 6;
	Len   += 6;

	/* The WPS IE shall contain the attributes required for an AP/Registrar as described in ¡±7.2.5 of [6]
	 */
	{
		/* 12. RF band, shall change based on current channel */
		*((PUSHORT) pData) = cpu2be16(WSC_ID_RF_BAND);
		*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
		*(pData + 4) = pDevInfo->RfBand;
		pData += 5;
		Len   += 5;

		/* 13. DPID shall be a required attribute if Credentials are available and ready for immediate use. */
		if (pAd->P2pCfg.Dpid  != DEV_PASS_ID_NOSPEC)
		{
			/* Device Password ID */
			*((PUSHORT) pData) = cpu2be16(WSC_ID_DEVICE_PWD_ID);
			*((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
			*((PUSHORT) (pData + 4)) = cpu2be16(pAd->P2pCfg.Dpid);
			pData += 6;
			Len   += 6;
		}

		/* When PBC has triggered, done or connected with timeout, */
		/* we must change the value of SelReg. */
#ifdef CONFIG_AP_SUPPORT
		if ((pAd->ApCfg.MBSSID[BSS0].WscControl.WscSelReg) && (pAd->P2pCfg.Dpid == DEV_PASS_ID_PBC))
		{
			if (P2P_GO_ON(pAd))
			{
				/* 14. Selected Registrar */
				*((PUSHORT) pData) = cpu2be16(WSC_ID_SEL_REGISTRAR);
				*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
				*(pData + 4) = pAd->ApCfg.MBSSID[BSS0].WscControl.WscSelReg; /* AP */
				pData += 5;
				Len   += 5;

				/* Device Password ID (append it above) */				
				/* 15. Selected Registrar Config Methods */
				*((PUSHORT) pData) = cpu2be16(WSC_ID_SEL_REG_CFG_METHODS);
				*((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
				*((PUSHORT) (pData + 4)) = cpu2be16(0x18c); /* Support All : PBC, Keypad, Label, Display */
				pData += 6;
				Len   += 6;
				
				DBGPRINT(RT_DEBUG_INFO, ("P2pMakeProbeRspWSCIE: SelReg=1 \n"));
			}
		}	
#endif /* CONFIG_AP_SUPPORT */
	}

	/* update the total length in vendor specific IE */
	*(pOutBuf+1) = Len - 2;

	/* fill in output buffer */
	*pIeLen = Len;
}

/*
	========================================================================
	
	Routine Description:
		Make WSC IE for the ProbeReq frame for P2P Spec requirement

	Arguments:
		pAdapter    - NIC Adapter pointer
		pOutBuf		- all of WSC IE field 
		pIeLen		- length
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		None
		
	========================================================================
*/
VOID P2pMakeProbeReqIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pIeLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	UCHAR			WscIEFixed[] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x04};	/* length will modify later */
	UCHAR			Len;
	PUCHAR			pData;
	PWSC_REG_DATA	pReg;
	PWSC_DEV_INFO	pDevInfo;
	UCHAR			OutMsgBuf[512];


	pReg = (PWSC_REG_DATA) &pAd->StaCfg.WscControl.RegData;

	/* Role play, Enrollee or Registrar */
	pDevInfo = (PWSC_DEV_INFO) &pReg->SelfInfo;

	pData = (PUCHAR) &OutMsgBuf[0];
	Len = 0;
	*pIeLen = 0;
	
	/* 0. WSC fixed IE */
	RTMPMoveMemory(pData, &WscIEFixed[0], 6);
	pData += 6;
	Len += 6;
				
	/* 1. Version */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_VERSION);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
	*(pData + 4) = pDevInfo->Version;
	pData += 5;
	Len   += 5;

	/* 2. Request Type */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_REQ_TYPE);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0001);
	*(pData + 4) = ((pAd->StaCfg.WscControl.WscConfMode == WSC_REGISTRAR) ? WSC_MSGTYPE_AP_WLAN_MGR : WSC_MSGTYPE_ENROLLEE_OPEN_8021X );
	pData += 5;
	Len   += 5;

	/* 3. Config method */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_CONFIG_METHODS);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
	*((PUSHORT) (pData + 4)) = cpu2be16(0x188);/* Keypad, Display, PBC */
	pData += 6;
	Len   += 6;

	/* 4. UUID-(E or R) */
	*((PUSHORT) pData) = ((pAd->StaCfg.WscControl.WscConfMode == WSC_REGISTRAR) ? cpu2be16(WSC_ID_UUID_R) : cpu2be16(WSC_ID_UUID_E));
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0010);	
	NdisMoveMemory((pData + 4), pDevInfo->Uuid, 16);
	pData += 20;
	Len   += 20;

	/* 5. Primary device type */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_PRIM_DEV_TYPE);
	*((PUSHORT) (pData + 2)) = cpu2be16(0x0008);	
	NdisMoveMemory((pData + 4), pP2PCtrl->DevInfo.PriDeviceType, 8);
	pData += 12;
	Len   += 12;

	/* 6. Primary device name */
	*((PUSHORT) pData) = cpu2be16(WSC_ID_DEVICE_NAME);
	*((PUSHORT) (pData + 2)) = cpu2be16(pP2PCtrl->DeviceNameLen);
	NdisMoveMemory((pData + 4), pP2PCtrl->DeviceName, pP2PCtrl->DeviceNameLen);
	pData += (pP2PCtrl->DeviceNameLen + 4);
	Len   += (pP2PCtrl->DeviceNameLen + 4);				

	/* 13. DPID shall be a required attribute if Credentials are available and ready for immediate use. */
	{
		USHORT Dpid = 0x188;
		/* Device Password ID */
		*((PUSHORT) pData) = cpu2be16(WSC_ID_DEVICE_PWD_ID);
		*((PUSHORT) (pData + 2)) = cpu2be16(0x0002);
		*((PUSHORT) (pData + 4)) = cpu2be16(Dpid);
		pData += 6;
		Len   += 6;
	}

	/* update the total length in vendor specific IE */
	OutMsgBuf[1] = Len - 2;

	/* fill in output buffer */
	*pIeLen = Len;
	NdisMoveMemory(pOutBuf, &OutMsgBuf[0], *pIeLen);


}

/*	
	==========================================================================
	Description: 
		Prepare Probe reqeust or response frame when opeartin as P2P DEvice.
		
	Parameters: 
		pDest : buffer to put frame content.
		pFrameLen : frame length.
	Note:
		 
	==========================================================================
 */
VOID P2PMakeProbe(
	IN PRTMP_ADAPTER pAd,  
	IN MLME_QUEUE_ELEM *Elem, 
	IN UCHAR		DsChannel,
	IN USHORT	SubType,
	OUT PUCHAR pDest,
	OUT	ULONG *pFrameLen) 
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	PUCHAR		pOutBuffer = pDest;
	UCHAR		Addr2[6];
	PHEADER_802_11	pHeader;
	HEADER_802_11 ProbeRspHdr;
	UCHAR   RSNIe=IE_WPA2;/*, RSNIe2=IE_WPA2, RSN_Len=22; */
	UCHAR       tmpSupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR       tmpExtRateLen;
	LARGE_INTEGER FakeTimestamp;
	UCHAR		DsLen = 1, SsidLen = 0;
	ULONG		TmpLen;
	ULONG		FrameLen = 0;
	/*UCHAR   	ErpIeLen = 1; */
	/*UCHAR			P2pIEFixed[6] = {0xdd, 0x08, 0x00, 0x50, 0xf2, 0x09}; */	/* length will modify later */
	USHORT		CapabilityInfo;
	UCHAR		SupRateLen;
	PUCHAR	ptr;
 
	pHeader = (PHEADER_802_11) &Elem->Msg[0];
	RTMPMoveMemory(Addr2, pHeader->Addr2, 6);

	if (SubType== SUBTYPE_PROBE_RSP)
	{
		DBGPRINT(RT_DEBUG_INFO, ("SYNC - P2PMakeProbeRsp. Addr2 = %02x:%02x:%02x:%02x:%02x:%02x...\n", PRINT_MAC(Addr2)));
		MgtMacHeaderInit(pAd,&ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, Addr2,
#ifdef P2P_SUPPORT
							pP2PCtrl->CurrentAddress,
#endif /* P2P_SUPPORT */
							pP2PCtrl->CurrentAddress);
	}
	else
		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
#ifdef P2P_SUPPORT
							pP2PCtrl->CurrentAddress,
#endif /* P2P_SUPPORT */
							BROADCAST_ADDR);

	NdisMoveMemory(tmpSupRate,pAd->CommonCfg.SupRate,pAd->CommonCfg.SupRateLen);
	tmpExtRateLen = pAd->CommonCfg.ExtRateLen;

	/* P2P device's probe response need to set both ess and ibss bit to zero. */
	CapabilityInfo = CAP_GENERATE(0, 0, 1, 0, 0, 0);
	if (P2P_GO_ON(pAd))
		CapabilityInfo = CAP_GENERATE(1, 0, 1, 0, 0, 0);
		
	tmpSupRate[0]  = 0x8C;    /* 6 mbps, in units of 0.5 Mbps, basic rate */
	tmpSupRate[1]  = 0x12;    /* 9 mbps, in units of 0.5 Mbps */
	tmpSupRate[2]  = 0x98;    /* 12 mbps, in units of 0.5 Mbps, basic rate */
	tmpSupRate[3]  = 0x24;    /* 18 mbps, in units of 0.5 Mbps */
	tmpSupRate[4]  = 0xb0;    /* 24 mbps, in units of 0.5 Mbps, basic rate */
	tmpSupRate[5]  = 0x48;    /* 36 mbps, in units of 0.5 Mbps */
	tmpSupRate[6]  = 0x60;    /* 48 mbps, in units of 0.5 Mbps */
	tmpSupRate[7]  = 0x6c;    /* 54 mbps, in units of 0.5 Mbps */
	SupRateLen  = 8;
	tmpExtRateLen = pAd->CommonCfg.ExtRateLen;

	if (DsChannel == 0)
		DBGPRINT(RT_DEBUG_ERROR, ("SYNC - P2PMakeProbeRsp. DsChannel = is 0 !!!!!\n"));
		
	if (SubType == SUBTYPE_PROBE_RSP)
	{
		SsidLen = WILDP2PSSIDLEN;
		if (P2P_GO_ON(pAd))
			SsidLen = pP2PCtrl->SSIDLen; 
		
		MakeOutgoingFrame(pOutBuffer,                 &FrameLen, 
						  sizeof(HEADER_802_11),      &ProbeRspHdr, 
						  TIMESTAMP_LEN,              &FakeTimestamp,
						  2,                          &pAd->CommonCfg.BeaconPeriod,
						  2,                          &CapabilityInfo,
						  1,                          &SsidIe, 
						  1,                          &SsidLen, 
						  SsidLen,     pP2PCtrl->SSID,
						  1,                          &SupRateIe, 
						  1,                          &SupRateLen,
						  SupRateLen,  		tmpSupRate, 
						  END_OF_ARGS);
	}
	else
	{
		SsidLen = WILDP2PSSIDLEN;
		MakeOutgoingFrame(pOutBuffer,                 &FrameLen, 
						  sizeof(HEADER_802_11),      &ProbeRspHdr, 
						  1,                          &SsidIe, 
						  1,                          &SsidLen, 
						  SsidLen,     &WILDP2PSSID[0],
						  1,                          &SupRateIe, 
						  1,                          &SupRateLen,
						  SupRateLen,  		tmpSupRate, 
						  END_OF_ARGS);

	}
	
	if ((DsChannel > 0) && (DsChannel <= 14))
	{
		MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen, 
						  1,                        &DsIe,
						  1,                        &DsLen,
						  1,                        &DsChannel,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	
	/* Add this IE after I already become GO. */
	if (tmpExtRateLen && (P2P_GO_ON(pAd)))
	{
	}

	/* Msut append RSN_IE because P2P uses WPA2PSK.  */
	{
		MakeOutgoingFrame(pOutBuffer + FrameLen,			&TmpLen,
							1,								&RSNIe,
							1,								&pAd->StaCfg.RSNIE_Len,
							pAd->StaCfg.RSNIE_Len,			pAd->StaCfg.RSN_IE,
							END_OF_ARGS);
		FrameLen += TmpLen;
	}

/*			

		// add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back
		if (pAd->CommonCfg.bAggregationCapable)
		{
			if ((pAd->CommonCfg.bPiggyBackCapable))
			{
				UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x07, 0x00, 0x00, 0x00};
				MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
								  9,                         RalinkSpecificIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
			else
			{
				UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x05, 0x00, 0x00, 0x00}; 
				MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
								  9,                         RalinkSpecificIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
		else
		{
			UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00}; 
			MakeOutgoingFrame(pOutBuffer+FrameLen,		 &TmpLen,
							  9,						 RalinkSpecificIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

		if ((pP2PCtrl->P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY))
		{			
			UCHAR	HtLen, AddHtLen, NewExtLen;
			ADD_HT_INFO_IE		AddHTInfo;	// Useful as AP.

			HT_CAPABILITY_IE HtCapability;
			NdisMoveMemory(&HtCapability, &pAd->ApCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
			if (pAd->Antenna.field.RxPath > 1)
			{
				HtCapability.MCSSet[1] = 0xff;
			}
			else
			{
				HtCapability.MCSSet[1] = 0x00;
			}

			HtLen = sizeof(pAd->ApCfg.HtCapability);
			AddHtLen = sizeof(pAd->ApCfg.AddHTInfoIe);
			NewExtLen = 1;

			NdisMoveMemory(&AddHTInfo, &pAd->ApCfg.AddHTInfoIe, sizeof(ADD_HT_INFO_IE));

			//New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame
			MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
							  1,                                &HtCapIe,
							  1,                                &HtLen,
							 sizeof(HT_CAPABILITY_IE),          &HtCapability, 
							  1,                                &AddHtInfoIe,
							  1,                                &AddHtLen,
							 sizeof(ADD_HT_INFO_IE),          	&AddHTInfo, 
							  1,                                &NewExtChanIe,
							  1,                                &NewExtLen,
							 sizeof(NEW_EXT_CHAN_IE),          	&pAd->CommonCfg.NewExtChanOffset, 
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	*/	
	/* New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */

	P2pMakeProbeRspWSCIE(pAd, pOutBuffer + FrameLen, &TmpLen);
	FrameLen += TmpLen;

	ptr = pOutBuffer + FrameLen;
	P2pMakeP2pIE(pAd, (UCHAR)SubType, ptr, &TmpLen);
	FrameLen += TmpLen;
	*pFrameLen = FrameLen;
			
}

/*	
	==========================================================================
	Description: 
		Make P2P IE.
		
	Parameters: 
		reutrn IE lenght and buffer.
	Note:
		 
	==========================================================================
 */
VOID P2pMakeP2pIE(
	IN	PRTMP_ADAPTER	pAd,
	IN 	UCHAR			PacketType,
	OUT	PUCHAR			pOutBuf,
	OUT	PULONG			pIeLen)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	UCHAR			P2pIEFixed[6] = {0xdd, 0x0e, 0x00, 0x50, 0xf2, 0x09};	/* length will modify later */
	ULONG			Len;
	PUCHAR			pData;
	ULONG			TempLen;
	UCHAR			Status;
	/*UCHAR			Maganed; */
	UCHAR				P2pCapability[2];
	PUCHAR			pBuf;

	RTMPMoveMemory(&P2pIEFixed[2], P2POUIBYTE, 4);	
	pData = pOutBuf;
	Len = 0;
	*pIeLen = 0;
	/* 0. P2P fixed IE */
	RTMPMoveMemory(pData, &P2pIEFixed[0], 6);
	pData += 6;
	Len += 6;

	P2pCapability[0] = pAd->P2pCfg.P2pCapability[0];
	P2pCapability[1] = pAd->P2pCfg.P2pCapability[1];

	/* :  .
	 */
	if (PacketType == SUBTYPE_ASSOC_RSP)
	{
		Status = P2PSTATUS_SUCCESS;
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_STATUS, &Status, pData);
		Len += TempLen;
		pData += TempLen;

		if (IS_P2P_SUPPORT_EXT_LISTEN(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("P2pMakeP2pIE (PacketType = %d)  insert SUBID_P2P_EXT_LISTEN_TIMING  .\n", PacketType));
			TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_EXT_LISTEN_TIMING, NULL, pData);
			Len += TempLen;
			pData += TempLen;
		}

		/* this is managed infra STA connect to Infra AP. So Add P2P Interface. then this's all. return here. */
		DBGPRINT(RT_DEBUG_TRACE, ("<----- P2pMakeP2pIE For managed STA. (Len = %ld) \n", Len));
		*(pOutBuf+1) = (Len-2);
		*pIeLen = Len;
		return;
	}

	/* Conenct to Managed AP. */
	if ((pP2PCtrl->P2pManagedParm.TotalNumOfP2pAttribute > 0) &&
		(PacketType == SUBTYPE_ASSOC_REQ))
	{
		/* Always support Managed when connecting using Assoc Req. */
		P2pCapability[0] |= DEVCAP_INFRA_MANAGED;
		/* If the cross connect is enabled, check whether we need to turn off it because Managed AP Asks us to do so. */
		/* If already turned off, no need to check. */
		if ((!P2P_TEST_BIT(P2pCapability[1], GRPCAP_CROSS_CONNECT))
			&& RTMPEqualMemory(pP2PCtrl->Bssid, pAd->P2pCfg.P2pManagedParm.ManageAPBSsid, MAC_ADDR_LEN))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("pAd->P2pCfg.P2pManagedParm.APP2pManageability = %x \n",	pAd->P2pCfg.P2pManagedParm.APP2pManageability));
			/* If This is the 1st Managed AP that I just want to connect to, but now I don't turn off the cross connect bit yet. */
			/* I have to turn off this bit when connecting to this AP. */
			if ((pAd->P2pCfg.P2pManagedParm.APP2pManageability != 0xff)
				&& (P2P_TEST_BIT(pAd->P2pCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ENABLE_BIT))
				&& (!P2P_TEST_BIT(pAd->P2pCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ICS_ENABLE_BIT)))
			{
				P2pCapability[1] &= (~GRPCAP_CROSS_CONNECT);
				DBGPRINT(RT_DEBUG_TRACE, ("Turn off Corss Conenct bit in Assoc Req. %x \n",  (~GRPCAP_CROSS_CONNECT)));
			}
		}

		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CAP, P2pCapability, pData);
		Len += TempLen;
		pData += TempLen;
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_INTERFACE, pP2PCtrl->CurrentAddress, pData);
		Len += TempLen;
		pData += TempLen;
		/* this is managed infra STA connect to Infra AP. So Add P2P Interface. then this's all. return here. */
		DBGPRINT(RT_DEBUG_ERROR, ("<-----P2pMakeP2pIE For managed STA. (Len = %ld PacketType = %d.) \n", Len, PacketType));
		*(pOutBuf+1) = (Len-2);
		*pIeLen = Len;
		return;
	}

	if (IS_CLIENT_DISCOVERY_ON(pAd))
	{
		P2pCapability[0] |= DEVCAP_CLIENT_DISCOVER;
	}

	if (PacketType == SUBTYPE_PROBE_REQ)
	{
		/* Probe Request Group Capability bit is reserved. (TestPlan 4.1.1) */
		P2pCapability[1] = 0;
	}

	pBuf = &P2pCapability[0];
	TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_CAP, pBuf, pData);
	Len += TempLen;
	pData += TempLen;

	/*if ((PortSubtype == PORTSUBTYPE_P2PClient)  */
	if ((pP2PCtrl->Rule == P2P_IS_CLIENT) || (pP2PCtrl->Rule == P2P_IS_DEVICE)
		|| (PacketType == SUBTYPE_PROBE_REQ))
	{
		/* Doesn't need to specify who I am searching for. So delete */
	}
	else
	{
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_DEVICE_ID, pP2PCtrl->CurrentAddress, pData);
		Len += TempLen;
		pData += TempLen;
	}
	if (PacketType == SUBTYPE_PROBE_REQ)
	{
		if (INFRA_ON(pAd))
			pBuf = &pAd->CommonCfg.Channel;
		else
			pBuf = &pP2PCtrl->ListenChannel;
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_LISTEN_CHANNEL, pBuf, pData);
		Len += TempLen;
		pData += TempLen;

		/*if (pP2PCtrl->PortSubtype == PORTSUBTYPE_P2PGO) */
		if (pP2PCtrl->Rule == P2P_IS_GO)
		{
			if (INFRA_ON(pAd))
				pBuf = &pAd->CommonCfg.Channel;
			else
				pBuf = &pP2PCtrl->GroupChannel;
			TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_OP_CHANNEL, pBuf, pData);
			Len += TempLen;
			pData += TempLen;
		}

	}
	if ((PacketType == SUBTYPE_PROBE_RSP) || (PacketType == SUBTYPE_ASSOC_REQ) || (PacketType == SUBTYPE_REASSOC_REQ))
	{
		if (PacketType == SUBTYPE_ASSOC_REQ)
		{
		DBGPRINT(RT_DEBUG_INFO, ("	P2pMakeP2pIE (PacketType = %d)	insert SUBID_P2P_DEVICE_INFO. DeviceNameLen = %ld.\n", PacketType, pAd->P2pCfg.DeviceNameLen));
		}
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_DEVICE_INFO, pP2PCtrl->CurrentAddress, pData);
		Len += TempLen;
		pData += TempLen;
	}

	if (((PacketType == SUBTYPE_PROBE_RSP) || (PacketType == SUBTYPE_PROBE_REQ)  || (PacketType == SUBTYPE_ASSOC_RSP)) 
		&& (IS_EXT_LISTEN_ON(pAd)))
	{
		DBGPRINT(RT_DEBUG_INFO, ("P2pMakeP2pIE (PacketType = %d)  insert SUBID_P2P_EXT_LISTEN_TIMING  .\n", PacketType));
		TempLen = InsertP2PSubelmtTlv(pAd, SUBID_P2P_EXT_LISTEN_TIMING, NULL, pData);
		Len += TempLen;
		pData += TempLen;
	}

	if (P2P_GO_ON(pAd) && (PacketType == SUBTYPE_PROBE_RSP))
	{
		/* If I am GO, must insert Group Info in my probe response to Probe Request that has P2P IE. */
		TempLen = InsertP2PGroupInfoTlv(pAd, pData);
		Len += TempLen;
		pData += TempLen;
	}

	/* The NoA has its own P2P IE. So NoA Attribute lenght doesn't count here. */
	*(pOutBuf+1) = (Len-2);

	if (P2P_GO_ON(pAd) && (pAd->P2pCfg.GONoASchedule.bValid == TRUE))
	{
		TempLen = P2pUpdateNoAProbeRsp(pAd, pData);
		Len += TempLen;
		pData += TempLen;
	}
	else if (P2P_GO_ON(pAd) && (P2P_TEST_BIT(pAd->P2pCfg.CTWindows, P2P_OPPS_BIT)))
	{
		TempLen = P2pUpdateNoAProbeRsp(pAd, pData);
		Len += TempLen;
		pData += TempLen;
	}
 
	DBGPRINT(RT_DEBUG_INFO, ("<----- P2pMakeP2pIE (Len = %ld) \n", Len));
	*pIeLen = Len;
}

/*	
	==========================================================================
	Description: 
		Processing WSC IE and put to OUTPUT buffer.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2PParseWPSIE(
	IN PUCHAR	pWpsData,
	IN USHORT		WpsLen,
	OUT PUSHORT	Dpid,
	OUT PUSHORT	ConfigMethod,
	OUT PUCHAR	DeviceName,
	OUT UCHAR	*DeviceNameLen)
{
	USHORT				Length;
	PUCHAR				pData;
	USHORT				WscType, WscLen;

	if (pWpsData == NULL)
		return;
	
	if (DeviceNameLen !=NULL)
		*DeviceNameLen = 0;
	pData = pWpsData+4; /*pWpsData+6;*/
	Length = WpsLen - 6;

	if (pData == NULL)
		return;
	
	/* Start to process WSC IEs */
	while (Length > 4)
	{
		WscType = cpu2be16(*((PUSHORT) pData));
		WscLen  = cpu2be16(*((PUSHORT) (pData + 2)));
		pData  += 4;
		Length -= 4;

		/* Parse M1 WSC type and store to RegData structure */
		switch (WscType)
		{
			case WSC_ID_DEVICE_NAME:		/* 1 */
				if (DeviceName !=NULL)
				{
					RTMPMoveMemory(DeviceName, pData, 32);
					if (DeviceNameLen !=NULL)
						*DeviceNameLen = (UCHAR)WscLen;
					DBGPRINT(RT_DEBUG_INFO,("%s :  DeviceName = %c%c%c%c%c ...\n", __FUNCTION__, DeviceName[0], DeviceName[1], DeviceName[2],DeviceName[3],DeviceName[4]));
				}
				break;
			case WSC_ID_DEVICE_PWD_ID:		/* 1 */
				if (Dpid !=NULL)
				{
					*Dpid = be2cpu16(*((USHORT *) pData));
					/**Dpid = *((PUSHORT) pData); */
					DBGPRINT(RT_DEBUG_INFO,("%s :  Dpid = %s  .\n", __FUNCTION__, decodeDpid(*Dpid)));
				}
				break;
			case WSC_ID_CONFIG_METHODS:		/* 1 */
				if (ConfigMethod !=NULL)
				{
					*ConfigMethod = be2cpu16(*((PUSHORT) pData));
					DBGPRINT(RT_DEBUG_INFO,(" Config = %x  ..\n", *ConfigMethod));
				}
				break;
			default:
				/*DBGPRINT(RT_DEBUG_TRACE, ("WscRecvMessageM1 --> Unknown IE 0x%04x\n", WscType));	    */
				break;				
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}	

}
	
/*	
	==========================================================================
	Description: 
		Go PeerDisassocReq Action.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID GoPeerDisassocReq(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	Addr2)
{
	/* Set to a temporary state.  If thie device connect within 25 seconds. he may use WPS to connect. */
	P2pGroupTabDelete(pAd, P2P_NOT_FOUND, Addr2);

	/* Don't Stop GO immediately. Give some time for this client to reconnect with 13 seconds.  */
	/* when StopGo timer expired,  */
}

/*	
	==========================================================================
	Description: 
		Update P2P beacon frame for P2P IE Group Info Attribute.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
ULONG P2pUpdateGroupBeacon(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	StartPosition) 
{

	return 0;
}

/*	
	==========================================================================
	Description: 
		Update P2P beacon frame for P2P IE NoA Attribute. When I am GO.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
ULONG P2pUpdateNoABeacon(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
/*	IN ULONG	StartPosition) */
	IN PUCHAR	pDest)
{
	PRT_P2P_CONFIG	pP2PCtrl = &pAd->P2pCfg;
	/*PUCHAR		pDest;*/
	UCHAR			P2PIEFixed[] = {0xdd, 0x16, 0x00, 0x50, 0xf2, 0x09};	/* length will modify later */
	/*UCHAR		i;*/
	/*PUCHAR	ptr;*/

	RTMPMoveMemory(&P2PIEFixed[2], P2POUIBYTE, 4);
	DBGPRINT(RT_DEBUG_INFO, ("<---- %s\n", __FUNCTION__));
	if (pP2PCtrl->GONoASchedule.bValid == TRUE)
	{
		/*pDest = &pAd->BeaconBuf[StartPosition];*/
		/*PUCHAR        pDest = (PUCHAR)pAd->ApCfg.MBSSID[apidx].BeaconBuf;*/
		/* Always support attach one NoA.. So.. length is fixed to 0x16. :)   */
		RTMPMoveMemory(pDest, P2PIEFixed, 6);

		*(pDest+6) = SUBID_P2P_NOA;
		/* Length is 13*n + 2 = 15 when n = 1 */
		*(pDest+7) = 15;
		/* Lenght 2nd byte */
		*(pDest+8) = 0;
		/* Index. */
		*(pDest+9) = pP2PCtrl->GONoASchedule.Token;
		/* CT Windows and OppPS parm */
		*(pDest+10) = pP2PCtrl->CTWindows;
		/* Count.  Test Plan set to 255. */
		*(pDest+11) = pP2PCtrl->GONoASchedule.Count;
		/* Duration */
		RTMPMoveMemory((pDest+12), &pP2PCtrl->GONoASchedule.Duration, 4);
		/* Interval */
		RTMPMoveMemory((pDest+16), &pP2PCtrl->GONoASchedule.Interval, 4);
		RTMPMoveMemory((pDest+20), &pP2PCtrl->GONoASchedule.StartTime, 4);
		pAd->GOBeaconBufNoALen = 24; 
	}
	else
	{
	}
	DBGPRINT(RT_DEBUG_INFO, ("----> %s\n", __FUNCTION__));

	return pAd->GOBeaconBufNoALen;
}

/*	
	==========================================================================
	Description: 
		Update P2P beacon frame for P2P IE NoA Attribute. When I am GO.
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
ULONG P2pUpdateNoAProbeRsp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pInbuffer) 
{
	PUCHAR		pDest;
	UCHAR			P2PIEFixed[6] = {0xdd, 0x16, 0x00, 0x50, 0xf2, 0x09};	/* length will modify later */
	/*UCHAR		i; */
	/*PUCHAR	ptr; */
	
	RTMPMoveMemory(&P2PIEFixed[2], P2POUIBYTE, 4);
	
	if (pAd->P2pCfg.GONoASchedule.bValid == TRUE)
	{
		pDest = pInbuffer;
		/* Always support attach one NoA.. So.. length is fixed to 0x16. :)   */
		RTMPMoveMemory(pDest, P2PIEFixed, 6);

		*(pDest+6) = SUBID_P2P_NOA;
		/* Length is 13*n + 2 = 15 when n = 1 */
		*(pDest+7) = 15;
		/* Lenght 2nd byte */
		*(pDest+8) = 0;
		/* Index. */
		*(pDest+9) = pAd->P2pCfg.GONoASchedule.Token;
		/* CT Windows and OppPS parm. Don't turn on both. So Set CTWindows = 0 */
		*(pDest+10) = 0;
		/* Count.  Test Plan set to 255. */
		*(pDest+11) = pAd->P2pCfg.GONoASchedule.Count;
		/* Duration */
		RTMPMoveMemory((pDest+12), &pAd->P2pCfg.GONoASchedule.Duration, 4);
		/* Interval */
		RTMPMoveMemory((pDest+16), &pAd->P2pCfg.GONoASchedule.Interval, 4);
		RTMPMoveMemory((pDest+20), &pAd->P2pCfg.GONoASchedule.StartTime, 4);

		return 24;
	}
	else if (P2P_TEST_BIT(pAd->P2pCfg.CTWindows, P2P_OPPS_BIT))
	{
		P2PIEFixed[1] = 0x9;
		pDest = pInbuffer;
		RTMPMoveMemory(pDest, P2PIEFixed, 6);
		*(pDest+6) = SUBID_P2P_NOA;
		/* Length is 13*n + 2 = 15 when n = 1 */
		*(pDest+7) = 2;
		/* Lenght 2nd byte */
		*(pDest+8) = 0;
		/* Index. */
		*(pDest+9) = pAd->P2pCfg.GONoASchedule.Token;
		/* CT Windows and OppPS parm */
		*(pDest+10) = pAd->P2pCfg.CTWindows;
		return 11;
	}
	else
	{
		return 0;
	}
	

}

/*	
	==========================================================================
	Description: 
		Update P2P beacon frame and save to  BeaconBuf[].
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pUpdateBssBeacon(
	IN PRTMP_ADAPTER pAd,
	IN  PUCHAR	 pCapability,
	IN  PUCHAR	pIpReqP2ptabindex) 
{
}

VOID GOUpdateBeaconFrame(
	IN PRTMP_ADAPTER pAd)
{

}
/*	
	==========================================================================
	Description: 
		Make P2P beacon frame and save to  BeaconBuf[]
		
	Parameters: 
	Note:
		 
	==========================================================================
 */
VOID P2pMakeBssBeacon(
	IN PRTMP_ADAPTER pAd)
{
}

/*
	========================================================================
	
	Routine Description:
		Check REinvoke's invitation Request frame .

	Arguments:
		    - NIC Adapter pointer
		
	Return Value:
		FALSE - None of channel in ChannelList Match any channel in pAd->ChannelList[] array

	IRQL = DISPATCH_LEVEL
	
	Note:
	========================================================================

*/
VOID P2pCheckInviteReq(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN		bIAmGO,
	IN UCHAR		index,
	IN PUCHAR	ChannelList,
	IN PUCHAR	BssidAddr,
	IN UCHAR		OpChannel,
	IN PUCHAR	Ssid,
	IN UCHAR	SsidLen,
	IN UCHAR	*pRspStatus)
{
	UCHAR		i;
	*pRspStatus = P2PSTATUS_SUCCESS;
	/* Check if have Common Channels. */
	if (FALSE == P2pCheckChannelList(pAd, ChannelList))
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - no common channel = %d...\n", *ChannelList));
		*pRspStatus = P2PSTATUS_NO_CHANNEL;
		return;
	}
	/* Invite Req from a CLient doesn't includes group_bssid in the request. So doesn't need check. */
	/* Check Bssid is correct. */
	if (!RTMPEqualMemory(BssidAddr, pAd->P2pTable.PerstEntry[index].Addr, MAC_ADDR_LEN)
		&& (bIAmGO == FALSE))
	{
		*pRspStatus = P2PSTATUS_INVALID_PARM;
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - MAc addr invalid  .\n"));
		return;
	}

	/* invite Req from Owner include OpChannel. from Client, doesn't include. */
	if (bIAmGO == FALSE)
	{
		/* Check Oopchannel is correct. */
		for (i = 0;i < pAd->ChannelListNum;i++)
		{
			if (pAd->ChannelList[i].Channel == OpChannel)
			{
				break;
			}
		}
		if ( i == pAd->ChannelListNum)
		{
			*pRspStatus = P2PSTATUS_NO_CHANNEL;
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - 2 P2PSTATUS_NO_CHANNEL  .\n"));
			return;
		}
	}
	/* Check SSID is correct. */
	if ((SsidLen > 0) && (!RTMPEqualMemory(pAd->P2pTable.PerstEntry[index].Profile.SSID.Ssid, Ssid, SsidLen)))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Ssid1[%d] = %s.  \n",  SsidLen, Ssid));
		DBGPRINT(RT_DEBUG_ERROR, ("Ssid2[%d] = %s.  \n",  pAd->P2pTable.PerstEntry[index].Profile.SSID.SsidLength, pAd->P2pTable.PerstEntry[index].Profile.SSID.Ssid));
		*pRspStatus = P2PSTATUS_INVALID_PARM;
		return;
	}

}

/*
	========================================================================
	
	Routine Description:
		Check REinvoke's invitation Request frame .

	Arguments:
		    - NIC Adapter pointer
		
	Return Value:
		FALSE - None of channel in ChannelList Match any channel in pAd->ChannelList[] array

	IRQL = DISPATCH_LEVEL
	
	Note:
	========================================================================

*/
VOID P2pCheckInviteReqFromExisting(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	ChannelList,
	IN PUCHAR	BssidAddr,
	IN UCHAR		OpChannel,
	IN PUCHAR	Ssid,
	IN UCHAR	SsidLen,
	IN UCHAR	*pRspStatus)
{
	UCHAR		i;
	*pRspStatus = P2PSTATUS_SUCCESS;
	/* Check if have Common Channels. */
	if (FALSE == P2pCheckChannelList(pAd, ChannelList))
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - no common channel. Channel list is : %d...\n", ChannelList[0]));
		*pRspStatus = P2PSTATUS_NO_CHANNEL;
		return;
	}

	/* invite Req from Owner include OpChannel. from Client, doesn't include. */
	if (OpChannel != 0)
	{
		/* Check Oopchannel is correct. */
		for (i = 0;i < pAd->ChannelListNum;i++)
		{
			if (pAd->ChannelList[i].Channel == OpChannel)
			{
				break;
			}
		}
		if ( i == pAd->ChannelListNum)
		{
			*pRspStatus = P2PSTATUS_NO_CHANNEL;
			DBGPRINT(RT_DEBUG_ERROR, (" P2P - No channel = %d...\n", OpChannel));
			return;
		}
	}

	/* Check SSID is correct. */
	if ((SsidLen == 0))
	{
		DBGPRINT(RT_DEBUG_ERROR, (" P2P - Ssidlen Zero = %d...\n", SsidLen));
		*pRspStatus = P2PSTATUS_INVALID_PARM;
		return;
	}

}

UCHAR ChannelToClass(
	IN UCHAR		Channel,
	IN UCHAR		Country)
{
	UCHAR		ReturnClass = 1;
	if (Country == 1/*COUNTRY_USA*/)
	{
		/* 0. Decide current regulatory class. P802.11REVmb_D3.0.pdf. TableJ-4 */
		if (Channel <= 11)
		{
			ReturnClass = 81;
		}
		else if (Channel <= 48)
		{
			ReturnClass = 115;	/* Atheros suggest to use 1 2010-May */
		}			
		else if (Channel <= 64)
		{
			ReturnClass = 118;
		}
		else if (Channel  <= 140)
		{
			ReturnClass = 121;
		}
		else if ((Channel  == 165) || (Channel  == 169))
		{
			ReturnClass = 125;
		}
		else
		{
			/* 3  when channels are 149.153. 157. 161 */
			ReturnClass = 124;
		}
	}
	return ReturnClass;
}


