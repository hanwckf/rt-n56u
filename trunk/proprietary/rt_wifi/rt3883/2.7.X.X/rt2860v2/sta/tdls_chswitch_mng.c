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

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsChannelSwitchReqSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen,
	OUT UCHAR *pPeerAddr,
	OUT	BOOLEAN *pIsInitator,
	OUT UCHAR *pTargetChannel,    
	OUT UCHAR *pRegulatoryClass,
	OUT UCHAR *pNewExtChannelOffset,
	OUT USHORT *pChSwitchTime,
	OUT USHORT *pChSwitchTimeOut,
	OUT UCHAR *pLinkIdentLen,
	OUT	TDLS_LINK_IDENT_ELEMENT *pLinkIdent)
{
	ULONG RemainLen = MsgLen;
	CHAR *Ptr =(CHAR *)Msg;
	PEID_STRUCT pEid;
	ULONG Length = 0;
	PHEADER_802_11 pHeader;
	BOOLEAN rv = TRUE;

	/* init value */
	*pNewExtChannelOffset = 0;

	/* Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable) */
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("PeerTdlsChannelSwitchReqSanity --> Invaild packet length - (ation header) \n"));
		return FALSE;	
	}

	pHeader = (PHEADER_802_11)Ptr;
	COPY_MAC_ADDR(pPeerAddr,  &pHeader->Addr2);	

	/* Offset to Target Channel */
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);

	/* Get the value of target channel from payload and advance the pointer */
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("PeerTdlsChannelSwitchReqSanity --> Invaild packet length - (target channel) \n"));
		return FALSE;
	}	

	*pTargetChannel = *Ptr;

	/* Offset to Regulatory Class */
	Ptr += 1;
	RemainLen -= 1;

	/* Get the value of regulatory class from payload and advance the pointer */
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchReqSanity --> Invaild packet length - (regulatory class) \n"));
		return FALSE;
	}
	
	*pRegulatoryClass = *Ptr;
	DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchReqSanity - Regulatory class = %d \n", *pRegulatoryClass));

	/* Offset to other elements */
	Ptr += 1;
	RemainLen -= 1;

	pEid = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_SECONDARY_CH_OFFSET:
				if (pEid->Len == 1)
				{
					*pNewExtChannelOffset = pEid->Octet[0];
				}
				else
				{
					rv = FALSE;
					DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchReqSanity - wrong IE_SECONDARY_CH_OFFSET. \n"));
				}
				break;

			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len == TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					NdisMoveMemory(pLinkIdent, &pEid->Octet[0], sizeof(TDLS_LINK_IDENT_ELEMENT));
					*pLinkIdentLen = TDLS_ELM_LEN_LINK_IDENTIFIER;
				}
				else
				{
					rv = FALSE;
					DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchReqSanity - wrong IE_TDLS_LINK_IDENTIFIER. \n"));
				}
				break;

			case IE_TDLS_CHANNEL_SWITCH_TIMING:
				if (pEid->Len == 4)
				{
					TDLS_CH_SWITCH_TIMING_ELEMENT ChSwitchTiming;

					NdisMoveMemory(&ChSwitchTiming, &pEid->Octet[0], sizeof(TDLS_CH_SWITCH_TIMING_ELEMENT));
					*pChSwitchTime = ChSwitchTiming.ChSwitchTime;
					*pChSwitchTimeOut = ChSwitchTiming.ChSwitchTimeOut;
				}
				else
				{
					rv = FALSE;
					DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchReqSanity - wrong IE_TDLS_CHANNEL_SWITCH_TIMING. \n"));
				}
				break;

			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchReqSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 
	}

    return rv;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsChannelSwitchRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pPeerAddr,
	OUT USHORT	*pStatusCode,
	OUT USHORT	*pChSwitchTime,
	OUT USHORT	*pChSwitchTimeOut,
	OUT UCHAR	*pLinkIdentLen,
	OUT	TDLS_LINK_IDENT_ELEMENT *pLinkIdent)
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	ULONG			Length = 0;
	PHEADER_802_11	pHeader;

	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H))
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchRspSanity --> Invaild packet length - (ation header) \n"));
		return FALSE;	
	}

	pHeader = (PHEADER_802_11)Ptr;
	COPY_MAC_ADDR(pPeerAddr,  &pHeader->Addr2);	

	// Offset to Status Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	
	// Get the value of Status Code from payload and advance the pointer
	if (RemainLen < 2)
	{
		DBGPRINT_RAW(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchRspSanity --> Invaild packet length - (status code) \n"));
		return FALSE;
	}	
	NdisMoveMemory(pStatusCode, Ptr, 2);

	// Offset to other elements
	Ptr += 2;
	RemainLen -= 2;

	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len == TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					NdisMoveMemory(pLinkIdent, &pEid->Octet[0], sizeof(TDLS_LINK_IDENT_ELEMENT));
					*pLinkIdentLen = TDLS_ELM_LEN_LINK_IDENTIFIER;
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchRspSanity - wrong IE_TDLS_LINK_IDENTIFIER. \n"));
				}
				break;

			case IE_TDLS_CHANNEL_SWITCH_TIMING:
				if (pEid->Len == 4)
				{
					TDLS_CH_SWITCH_TIMING_ELEMENT ChSwitchTiming;

					NdisMoveMemory(&ChSwitchTiming, &pEid->Octet[0], sizeof(TDLS_CH_SWITCH_TIMING_ELEMENT));
					*pChSwitchTime = ChSwitchTiming.ChSwitchTime;
					*pChSwitchTimeOut = ChSwitchTiming.ChSwitchTimeOut;
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchRspSanity - wrong IE_TDLS_CHANNEL_SWITCH_TIMING. \n"));
				}
				break;
			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsChannelSwitchRspSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 
	}

    return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildChannelSwitchRequest(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pPeerAddr,
	IN USHORT ChSwitchTime,
	IN USHORT ChSwitchTimeOut,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW)
{
	PRT_802_11_TDLS	pTDLS = NULL;
	INT LinkId = 0xff;

	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
							CATEGORY_TDLS, TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST);

	/* Target Channel */
	TDLS_InsertTargetChannel(pAd, (pFrameBuf + *pFrameLen), pFrameLen, TargetChannel);

	/* Regulatory Class */
	TDLS_InsertRegulatoryClass(pAd, (pFrameBuf + *pFrameLen), pFrameLen, TargetChannel, TargetChannelBW);

	/* Secondary Channel Offset */
	if(TargetChannelBW != EXTCHA_NONE)
	{
		if (TargetChannel > 14)
		{
			if ((TargetChannel == 36) || (TargetChannel == 44) || (TargetChannel == 52) ||
				(TargetChannel == 60) || (TargetChannel == 100) || (TargetChannel == 108) ||
				(TargetChannel == 116) || (TargetChannel == 124) || (TargetChannel == 132) ||
				(TargetChannel == 149) || (TargetChannel == 157))
			{
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, EXTCHA_ABOVE);
				pAd->StaCfg.TdlsCurrentChannelBW = EXTCHA_ABOVE;
				
			}
			else if ((TargetChannel == 40) || (TargetChannel == 48) || (TargetChannel == 56) |
					(TargetChannel == 64) || (TargetChannel == 104) || (TargetChannel == 112) ||
					(TargetChannel == 120) || (TargetChannel == 128) || (TargetChannel == 136) ||
					(TargetChannel == 153) || (TargetChannel == 161))
			{
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, EXTCHA_BELOW);
				pAd->StaCfg.TdlsCurrentChannelBW = EXTCHA_BELOW;
			}
		}
		else
		{
			do
			{
				UCHAR ExtCh;
				UCHAR Dir = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
				ExtCh = TDLS_GetExtCh(TargetChannel, Dir);
				if (TDLS_IsValidChannel(pAd, ExtCh))
				{
					TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, Dir);
					pAd->StaCfg.TdlsCurrentChannelBW = Dir;
					break;
				}

				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = TDLS_GetExtCh(TargetChannel, Dir);
				if (TDLS_IsValidChannel(pAd, ExtCh))
				{
					TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, Dir);
					pAd->StaCfg.TdlsCurrentChannelBW = Dir;
					break;
				}
			} while(FALSE);
		}
	}
	else
	{
		pAd->StaCfg.TdlsCurrentChannelBW = EXTCHA_NONE;
	}

	/* fill link identifier */
	LinkId = TDLS_SearchLinkId(pAd, pPeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_ChannelSwitchReqAction() can not find the LinkId!\n"));
		return NDIS_STATUS_FAILURE;
	}
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pPeerAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pPeerAddr);

	/* Channel Switch Timing */
	TDLS_InsertChannelSwitchTimingIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ChSwitchTime, ChSwitchTimeOut);
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildChannelSwitchResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	ReasonCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE);

	/* fill reason code */
	TDLS_InsertReasonCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ReasonCode);

	/* fill link identifier */
	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->MacAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	/* Channel Switch Timing */
	TDLS_InsertChannelSwitchTimingIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ChSwitchTime, ChSwitchTimeOut);
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_ChannelSwitchReqAction(
	IN PRTMP_ADAPTER	pAd,
	IN PMLME_TDLS_CH_SWITCH_STRUCT	pChSwitchReq)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	MAC_TABLE_ENTRY       *pEntry = NULL;
	UINT16	SwitchTime = pAd->StaCfg.TdlsInfo.TdlsSwitchTime; //micro seconds
	UINT16	SwitchTimeout = pAd->StaCfg.TdlsInfo.TdlsSwitchTimeout; // micro seconds
	INT		LinkId = 0xff;
	PRT_802_11_TDLS	pTDLS = NULL;

	DBGPRINT(RT_DEBUG_WARN, ("====> TDLS_ChannelSwitchReqAction\n"));

	MAKE_802_3_HEADER(Header802_3, pChSwitchReq->PeerMacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,			&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildChannelSwitchRequest(pAd, pOutBuffer, &FrameLen, pChSwitchReq->PeerMacAddr,SwitchTime,
									SwitchTimeout, pChSwitchReq->TargetChannel, pChSwitchReq->TargetChannelBW);

	pEntry = MacTableLookup(pAd, pChSwitchReq->PeerMacAddr);

	if (pEntry && IS_ENTRY_TDLS(pEntry))
	{
		pTDLS->ChannelSwitchCurrentState = TDLS_CHANNEL_SWITCH_WAIT_RSP;
		if (pChSwitchReq->TargetChannel != pAd->CommonCfg.Channel)
			RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE, RTMP_TDLS_SPECIFIC_EDCA);
		else
			RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE, RTMP_TDLS_SPECIFIC_HCCA);
		pAd->StaCfg.TdlsCurrentChannel = pChSwitchReq->TargetChannel;
		pAd->StaCfg.TdlsCurrentChannelBW = pChSwitchReq->TargetChannelBW;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can't find TDLS entry on mac TABLE !!!!\n"));
	}

	hex_dump("TDLS switch channel request send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_WARN, ("<==== TDLS_ChannelSwitchReqAction\n"));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_ChannelSwitchRspAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	StatusCode,
	IN	UCHAR	FrameType)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_WARN, ("TDLS ===> TDLS_ChannelSwitchRspAction\n"));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus	!= NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - TDLS_ChannelSwitchRspAction() allocate memory failed \n"));
		return NStatus;
	}

	MakeOutgoingFrame(pOutBuffer,			&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildChannelSwitchResponse(pAd, pOutBuffer, &FrameLen, pTDLS, ChSwitchTime, ChSwitchTimeOut, StatusCode);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[pTDLS->MacTabMatchWCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE, FrameType);

	hex_dump("TDLS send channel switch response pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_WARN, ("TDLS <=== TDLS_ChannelSwitchRspAction\n"));

	return NStatus;
}


/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeChannelSwitchAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_TDLS_CH_SWITCH_STRUCT pChSwReq = NULL; 
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	INT	LinkId = 0xff;

	DBGPRINT(RT_DEBUG_WARN,("TDLS ===> TDLS_MlmeChannelSwitchAction() \n"));

	pChSwReq = (PMLME_TDLS_CH_SWITCH_STRUCT)Elem->Msg;

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE)
	{
		DBGPRINT(RT_DEBUG_OFF,("%s(%d): AP Prohibite TDLS Channel Switch !!!\n", __FUNCTION__, __LINE__));
		return;
	}

	if (INFRA_ON(pAd))
	{
		// Drop not within my TDLS Table that created before !
		LinkId = TDLS_SearchLinkId(pAd, pChSwReq->PeerMacAddr);

		if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeChannelSwitchAction() can not find the LinkId!\n"));
			return;
		}

		pAd->StaCfg.TdlsForcePowerSaveWithAP = TRUE;

		if (pAd->StaCfg.bTdlsNoticeAPPowerSave == FALSE)
		{
			pAd->StaCfg.TdlsSendNullFrameCount = 0;
			pAd->StaCfg.bTdlsNoticeAPPowerSave = TRUE;
			RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, TRUE);
		}
		else
		{
			pAd->StaCfg.TdlsSendNullFrameCount++;
			if (pAd->StaCfg.TdlsSendNullFrameCount >= 200)
				pAd->StaCfg.bTdlsNoticeAPPowerSave = FALSE;
		}

		DBGPRINT(RT_DEBUG_ERROR, ("103. %ld !!!\n", (jiffies * 1000) / OS_HZ));

		/* Build TDLS channel switch Request Frame */
		NStatus = TDLS_ChannelSwitchReqAction(pAd, pChSwReq);
		if (NStatus	!= NDIS_STATUS_SUCCESS)	
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeChannelSwitchAction() Build Channel Switch Request Fail !!!\n"));
		}
		else
		{
			pAd->StaCfg.TdlsChannelSwitchPairCount++;
			DBGPRINT(RT_DEBUG_WARN,("TDLS <=== TDLS_MlmeChannelSwitchAction() \n"));
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_WARN,("TDLS <=== TDLS_MlmeChannelSwitchAction() TDLS only support infra mode !!!\n"));
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
TDLS_MlmeChannelSwitchRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_TDLS_CH_SWITCH_STRUCT pMlmeChSwitchRsp = NULL; 
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	PRT_802_11_TDLS	pTdls = NULL;
	INT LinkId = 0xff;

	DBGPRINT(RT_DEBUG_WARN,("TDLS ===> TDLS_MlmeChannelSwitchRspAction() \n"));

	pMlmeChSwitchRsp = (PMLME_TDLS_CH_SWITCH_STRUCT)Elem->Msg;

	if (INFRA_ON(pAd))
	{
		// Drop not within my TDLS Table that created before !
		LinkId = TDLS_SearchLinkId(pAd, pMlmeChSwitchRsp->PeerMacAddr);

		if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		{
			DBGPRINT(RT_DEBUG_OFF,("TDLS - TDLS_MlmeChannelSwitchRspAction() can not find the LinkId!\n"));
			return;
		}

		/* Point to the current Link ID */
		pTdls = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

		/* Build TDLS channel switch Request Frame */
		NStatus = TDLS_ChannelSwitchRspAction(pAd, pTdls, pTdls->ChSwitchTime, pTdls->ChSwitchTimeout, 0, (RTMP_TDLS_SPECIFIC_CS_RSP_NOACK + RTMP_TDLS_SPECIFIC_HCCA));

		if (NStatus != NDIS_STATUS_SUCCESS)	
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeChannelSwitchRspAction() Build Channel Switch Response Fail !!!\n"));
		}
		else
		{
			RTMPusecDelay(300);
			NdisGetSystemUpTime(&pAd->StaCfg.TdlsGoBackStartTime);

			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
			if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
			else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_BELOW);
			else
				TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_NONE);
			TDLS_EnablePktChannel(pAd, TDLS_FIFO_ALL);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);

			DBGPRINT(RT_DEBUG_WARN,("TDLS <=== TDLS_MlmeChannelSwitchRspAction() \n"));
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS - TDLS_MlmeChannelSwitchRspAction() TDLS only support infra mode !!!\n"));
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
TDLS_PeerChannelSwitchReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS		pTDLS = NULL;
	INT					LinkId = 0xff;
	UCHAR				PeerAddr[MAC_ADDR_LEN];
	BOOLEAN				IsInitator;
	//BOOLEAN				TimerCancelled;
	UCHAR				TargetChannel;
	UCHAR				RegulatoryClass;
	UCHAR				NewExtChannelOffset = 0xff;
	UCHAR				LinkIdentLen;
	USHORT				PeerChSwitchTime;
	USHORT				PeerChSwitchTimeOut;
	TDLS_LINK_IDENT_ELEMENT	LinkIdent;
	NDIS_STATUS			NStatus = NDIS_STATUS_SUCCESS;
	USHORT				StatusCode = MLME_SUCCESS;
	UINT16				SwitchTime = pAd->StaCfg.TdlsInfo.TdlsSwitchTime; //micro seconds
	UINT16				SwitchTimeout = pAd->StaCfg.TdlsInfo.TdlsSwitchTimeout; // micro seconds

	DBGPRINT(RT_DEBUG_WARN,("TDLS ===> TDLS_PeerChannelSwitchReqAction() \n"));

	// Not TDLS Capable, ignore it
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	if (!INFRA_ON(pAd))
		return;

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE)
	{
		DBGPRINT(RT_DEBUG_OFF,("%s(%d): AP Prohibite TDLS Channel Switch !!!\n", __FUNCTION__, __LINE__));
		return;
	}

	tdls_hex_dump("TDLS peer channel switch request receive pack", Elem->Msg, Elem->MsgLen);
	
	if (!PeerTdlsChannelSwitchReqSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								PeerAddr,
								&IsInitator,
								&TargetChannel,
								&RegulatoryClass,
								&NewExtChannelOffset,
								&PeerChSwitchTime,
								&PeerChSwitchTimeOut,
								&LinkIdentLen,
								&LinkIdent))
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s(%d):  from %02x:%02x:%02x:%02x:%02x:%02x Sanity Check Fail !!!\n",
									__FUNCTION__,__LINE__, PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));
		return;
	}

	DBGPRINT(RT_DEBUG_WARN,("%s(%d):  from %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
								__FUNCTION__,__LINE__, PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));

	DBGPRINT(RT_DEBUG_ERROR, ("300. %ld !!!\n", (jiffies * 1000) / OS_HZ));

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s(%d):  can not find from %02x:%02x:%02x:%02x:%02x:%02x on TDLS entry !!!\n",
									__FUNCTION__,__LINE__, PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));
		return;
	}

	if (pAd->StaCfg.bChannelSwitchInitiator == FALSE)
	{
		pAd->StaCfg.TdlsForcePowerSaveWithAP = TRUE;

		if (pAd->StaCfg.bTdlsNoticeAPPowerSave == FALSE)
		{
			pAd->StaCfg.TdlsSendNullFrameCount = 0;
			pAd->StaCfg.bTdlsNoticeAPPowerSave = TRUE;
			RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, TRUE);
		}
		else
		{
			pAd->StaCfg.TdlsSendNullFrameCount++;
			if (pAd->StaCfg.TdlsSendNullFrameCount >= 200)
				pAd->StaCfg.bTdlsNoticeAPPowerSave = FALSE;
		}
	}



	// Point to the current Link ID
	pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	if (SwitchTime >= PeerChSwitchTime)
		PeerChSwitchTime = SwitchTime;

	if (SwitchTimeout >= PeerChSwitchTimeOut)
		PeerChSwitchTimeOut = SwitchTimeout;

	if (RtmpPktPmBitCheck(pAd))
	{
		RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
		TDLS_SendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, 0);
	}

	{
		UINT32 macCfg, TxCount;
		UINT32 MTxCycle;

		RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);

		if  (TargetChannel != pAd->CommonCfg.Channel)
			NStatus = TDLS_ChannelSwitchRspAction(pAd, pTDLS, PeerChSwitchTime, PeerChSwitchTimeOut, StatusCode, RTMP_TDLS_SPECIFIC_CS_RSP_WAIT_ACK);
		else
			NStatus = TDLS_ChannelSwitchRspAction(pAd, pTDLS, PeerChSwitchTime, PeerChSwitchTimeOut, StatusCode, (RTMP_TDLS_SPECIFIC_CS_RSP_WAIT_ACK + RTMP_TDLS_SPECIFIC_HCCA));
			

		for (MTxCycle = 0; MTxCycle < 500; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);
			TxCount = macCfg & 0x0000ffff;
			if (TxCount > 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MTxCycle = %d, %ld !!!\n", MTxCycle, (jiffies * 1000) / OS_HZ));
				break;
			}
			else
				RTMPusecDelay(50);
		}

		if (MTxCycle >= 500)
		{
			NStatus = NDIS_STATUS_FAILURE;
			DBGPRINT(RT_DEBUG_OFF,("TDLS Transmit Channel Switch Response Fail !!!\n"));
		}
	}

	if (NStatus == NDIS_STATUS_SUCCESS)
	{

		{
			if  (TargetChannel != pAd->CommonCfg.Channel)
			{
				BOOLEAN TimerCancelled;
				//ULONG Now, temp1, temp2, temp3;

				pAd->StaCfg.TdlsCurrentChannel = TargetChannel;

				if (NewExtChannelOffset != 0)
					pAd->StaCfg.TdlsCurrentChannelBW = NewExtChannelOffset;
				else
					pAd->StaCfg.TdlsCurrentChannelBW = EXTCHA_NONE;

				pTDLS->ChSwitchTime = PeerChSwitchTime;
				pAd->StaCfg.TdlsGlobalSwitchTime = PeerChSwitchTime;
				pTDLS->ChSwitchTimeout = PeerChSwitchTimeOut;
				pAd->StaCfg.TdlsGlobalSwitchTimeOut = PeerChSwitchTimeOut;

				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
				//Cancel the timer since the received packet to me.
#ifdef TDLS_HWTIMER_SUPPORT
				TDLS_SetChannelSwitchTimer(pAd,  (PeerChSwitchTimeOut / 1000));
#else
				RTMPCancelTimer(&pTDLS->ChannelSwitchTimeoutTimer, &TimerCancelled);
				NdisGetSystemUpTime(&pTDLS->ChannelSwitchTimerStartTime);
				RTMPSetTimer(&pTDLS->ChannelSwitchTimeoutTimer, (PeerChSwitchTimeOut / 1000));
#endif // TDLS_HWTIMER_SUPPORT //
				RTMPCancelTimer(&pAd->StaCfg.TdlsDisableChannelSwitchTimer, &TimerCancelled);
				pAd->StaCfg.bTdlsCurrentDoingChannelSwitchWaitSuccess = TRUE;
				pAd->StaCfg.bDoingPeriodChannelSwitch = TRUE;

				if (RTDebugLevel < RT_DEBUG_ERROR)
					RTMPusecDelay(300);
				else
					DBGPRINT(RT_DEBUG_ERROR, ("1041. %ld !!!\n", (jiffies * 1000) / OS_HZ));
				TDLS_InitChannelRelatedValue(pAd, pAd->StaCfg.TdlsCurrentChannel, pAd->StaCfg.TdlsCurrentChannelBW);
			}
			else
			{
				pTDLS->bDoingPeriodChannelSwitch = FALSE;
				pAd->StaCfg.bDoingPeriodChannelSwitch = FALSE;
				pAd->StaCfg.TdlsForcePowerSaveWithAP = FALSE;
				pAd->StaCfg.bTdlsNoticeAPPowerSave = FALSE;

				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
				RTMPusecDelay(300);

				if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
					TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
				else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
					TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_BELOW);
				else
					TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_NONE);
				TDLS_EnablePktChannel(pAd, TDLS_FIFO_ALL);
				RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);

				RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, FALSE);
			}
		}
	}

	DBGPRINT(RT_DEBUG_WARN,("TDLS <=== TDLS_PeerChannelSwitchReqAction() \n"));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerChannelSwitchRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS		pTDLS = NULL;
	INT					LinkId = 0xff;
	UCHAR				PeerAddr[MAC_ADDR_LEN];
	//BOOLEAN				IsInitator;
	BOOLEAN				TimerCancelled;
	//UCHAR				RegulatoryClass;
	//UCHAR				NewExtChannelOffset = 0xff;
	UCHAR				LinkIdentLen;
	USHORT				PeerChSwitchTime;
	USHORT				PeerChSwitchTimeOut;
	TDLS_LINK_IDENT_ELEMENT	LinkIdent;
	//NDIS_STATUS			NStatus = NDIS_STATUS_SUCCESS;
	USHORT				StatusCode = MLME_SUCCESS;

	DBGPRINT(RT_DEBUG_WARN,("TDLS ===> TDLS_PeerChannelSwitchRspAction() \n"));

	// Not TDLS Capable, ignore it
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	if (!INFRA_ON(pAd))
		return;

	hex_dump("TDLS peer channel switch response receive pack", Elem->Msg, Elem->MsgLen);

	if (!PeerTdlsChannelSwitchRspSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								PeerAddr,
								&StatusCode,
								&PeerChSwitchTime,
								&PeerChSwitchTimeOut,
								&LinkIdentLen,
								&LinkIdent))
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s(%d):  from %02x:%02x:%02x:%02x:%02x:%02x Sanity Check Fail !!!\n",
									__FUNCTION__,__LINE__, PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));
		return;
	}

	// Drop not within my TDLS Table that created before !
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s(%d):  can not find from %02x:%02x:%02x:%02x:%02x:%02x on TDLS entry !!!\n",
									__FUNCTION__,__LINE__, PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));
		return;
	}

	// Point to the current Link ID
	pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	if ((pTDLS->ChannelSwitchCurrentState == TDLS_CHANNEL_SWITCH_NONE) &&
		(StatusCode == MLME_REQUEST_DECLINED))
	{
		DBGPRINT(RT_DEBUG_OFF,("%s(%d): received a failed StatusCode = %d on Unsolicited response !!!\n",
								__FUNCTION__, __LINE__, StatusCode));
		return;
	}

	if (StatusCode == MLME_REQUEST_DECLINED)
	{
		if ((pAd->StaCfg.TdlsChannelSwitchRetryCount > 0) &&
			(pTDLS->bDoingPeriodChannelSwitch) &&
			(pAd->StaCfg.bDoingPeriodChannelSwitch))
		{
			pAd->StaCfg.TdlsChannelSwitchRetryCount--;

			DBGPRINT(RT_DEBUG_OFF,("%s(%d): received a failed StatusCode = %d  re-try again !!!\n",
									__FUNCTION__, __LINE__, StatusCode));
		}
		else
		{
			pTDLS->bDoingPeriodChannelSwitch = FALSE;
			pAd->StaCfg.bDoingPeriodChannelSwitch = FALSE;
			pAd->StaCfg.bTdlsNoticeAPPowerSave = FALSE;
			pAd->StaCfg.TdlsForcePowerSaveWithAP = FALSE;
			RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, FALSE);
		}

		DBGPRINT(RT_DEBUG_OFF,("TDLS - TDLS_PeerChannelSwitchRspAction() received a failed StatusCode = %d !!!\n", StatusCode ));
		return;
	}

	DBGPRINT(RT_DEBUG_WARN,("%s(%d):  from %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
								__FUNCTION__,__LINE__, PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));



	if (StatusCode == MLME_SUCCESS)
	{
		if (pTDLS->ChannelSwitchCurrentState == TDLS_CHANNEL_SWITCH_NONE)
		{
			if (pAd->StaCfg.bChannelSwitchInitiator == FALSE)
			{
				RTMPCancelTimer(&pAd->StaCfg.TdlsResponderGoBackBaseChTimer, &TimerCancelled);
				DBGPRINT(RT_DEBUG_WARN,("%s(%d): i am responder!!!\n",  __FUNCTION__,__LINE__));
			}
			else
			{
				RTMPCancelTimer(&pAd->StaCfg.TdlsPeriodGoBackBaseChTimer, &TimerCancelled);
				DBGPRINT(RT_DEBUG_WARN,("%s(%d): i am Initiator !!!\n", __FUNCTION__,__LINE__));
			}

			if (pAd->StaCfg.TdlsCurrentOperateChannel != pAd->CommonCfg.Channel)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("106. %ld !!!\n", (jiffies * 1000) / OS_HZ));

				RTMPusecDelay(300);
				NdisGetSystemUpTime(&pAd->StaCfg.TdlsGoBackStartTime);

				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
				if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
					TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
				else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
					TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_BELOW);
				else
					TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_NONE);
				TDLS_EnablePktChannel(pAd, TDLS_FIFO_ALL);
				RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
			}
		}
		else
		{
			if (pAd->StaCfg.TdlsCurrentChannel != pAd->CommonCfg.Channel)
			{
				if (pAd->StaCfg.bChannelSwitchInitiator)
				{
					UINT16 SwitchTime = pAd->StaCfg.TdlsInfo.TdlsSwitchTime; //micro seconds
					UINT16 SwitchTimeout = pAd->StaCfg.TdlsInfo.TdlsSwitchTimeout; // micro seconds

					pAd->StaCfg.TdlsChannelSwitchPairCount--;
					pAd->StaCfg.TdlsChannelSwitchRetryCount = 10;
					//pAd->StaCfg.bDoingPeriodChannelSwitch = TRUE;

					if (SwitchTime >= PeerChSwitchTime)
						PeerChSwitchTime = SwitchTime;

					if (SwitchTimeout >= PeerChSwitchTimeOut)
						PeerChSwitchTimeOut = SwitchTimeout;

					pTDLS->ChSwitchTime = PeerChSwitchTime;
					pAd->StaCfg.TdlsGlobalSwitchTime = PeerChSwitchTime;
					pTDLS->ChSwitchTimeout = PeerChSwitchTimeOut;
					pAd->StaCfg.TdlsGlobalSwitchTimeOut = PeerChSwitchTimeOut;
					pTDLS->ChannelSwitchCurrentState = TDLS_CHANNEL_SWITCH_NONE;

					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
					//Cancel the timer since the received packet to me.
#ifdef TDLS_HWTIMER_SUPPORT
					TDLS_SetChannelSwitchTimer(pAd,  ((PeerChSwitchTime + pAd->StaCfg.TdlsOffChannelDelay) / 1000));
#else
					RTMPCancelTimer(&pTDLS->ChannelSwitchTimer, &TimerCancelled);
					pTDLS->bEnableChSwitchTime = TRUE;
					NdisGetSystemUpTime(&pTDLS->ChannelSwitchTimerStartTime);
					RTMPSetTimer(&pTDLS->ChannelSwitchTimer, ((PeerChSwitchTime + pAd->StaCfg.TdlsOffChannelDelay) / 1000));
#endif // TDLS_HWTIMER_SUPPORT //

					if (RTDebugLevel < RT_DEBUG_ERROR)
						RTMPusecDelay(300);
					else
						DBGPRINT(RT_DEBUG_ERROR, ("104. %ld !!!\n", (jiffies * 1000) / OS_HZ));
					TDLS_InitChannelRelatedValue(pAd, pAd->StaCfg.TdlsCurrentChannel, pAd->StaCfg.TdlsCurrentChannelBW);
				}
			}
			else
			{
				pTDLS->bDoingPeriodChannelSwitch = FALSE;
				pAd->StaCfg.bDoingPeriodChannelSwitch = FALSE;
				pAd->StaCfg.TdlsForcePowerSaveWithAP = FALSE;
				pAd->StaCfg.bTdlsNoticeAPPowerSave = FALSE;

				if (pAd->StaCfg.bChannelSwitchInitiator == FALSE)
				{
					DBGPRINT(RT_DEBUG_OFF,("%s(%d): i am channel switch responder!!!\n",  __FUNCTION__,__LINE__));
				}
				else
				{
					RTMPCancelTimer(&pAd->StaCfg.TdlsDisableChannelSwitchTimer, &TimerCancelled);
					pAd->StaCfg.bChannelSwitchInitiator = FALSE;
					DBGPRINT(RT_DEBUG_OFF,("%s(%d): i am channel switch Initiator !!!\n", __FUNCTION__,__LINE__));
				}

				if (pAd->StaCfg.TdlsCurrentOperateChannel != pAd->CommonCfg.Channel)
				{
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
					if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
						TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
					else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
						TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_BELOW);
					else
						TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_NONE);
					TDLS_EnablePktChannel(pAd, TDLS_FIFO_ALL);
					RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
				}

				RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, FALSE);
			}
		}
	}




	DBGPRINT(RT_DEBUG_WARN,("TDLS <=== TDLS_PeerChannelSwitchRspAction() \n"));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_ChannelSwitchTimeAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd;
	PRT_802_11_TDLS	pTDLS = (PRT_802_11_TDLS)FunctionContext;
	

	DBGPRINT(RT_DEBUG_WARN, ("TDLS_ChannelSwitchTimeAction - channel switch procedure for (%02x:%02x:%02x:%02x:%02x:%02x)\n",
				pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2],
				pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5]));

	pAd = pTDLS->pAd;

	{
		UINT32 macCfg, TxCount;
		UINT32 MTxCycle;
		UINT16 MaxWaitingTime;

		DBGPRINT(RT_DEBUG_ERROR, ("105. %ld !!!\n", (jiffies * 1000) / OS_HZ));

		{
			ULONG Now, temp1;

			NdisGetSystemUpTime(&Now);
			temp1 = (((Now - pTDLS->ChannelSwitchTimerStartTime) * 1000) / OS_HZ);

			if (temp1 < (pTDLS->ChSwitchTime / 1000))
			{
				DBGPRINT(RT_DEBUG_OFF, ("Timer  = %ld < 11 !!!\n", temp1));
			}
		}
		RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);
		TDLS_SendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, RTMP_TDLS_SPECIFIC_NULL_FRAME);
		TDLS_EnablePktChannel(pAd, TDLS_FIFO_HCCA);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);

		MaxWaitingTime = ((pTDLS->ChSwitchTimeout - pTDLS->ChSwitchTime) / 1000);
		for (MTxCycle = 0; MTxCycle < ((MaxWaitingTime + 1) * 20); MTxCycle++)
		{
			RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);
			TxCount = macCfg & 0x0000ffff;
			if (TxCount > 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MTxCycle = %d, %ld !!!\n", MTxCycle, (jiffies * 1000) / OS_HZ));
				break;
			}
			else
				RTMPusecDelay(50);
		}

		if (MTxCycle == ((MaxWaitingTime + 1) * 20))
		{
			DBGPRINT(RT_DEBUG_OFF, ("24. %ld @@@!!!\n", (jiffies * 1000) / OS_HZ));

			RTMPusecDelay(300);
			NdisGetSystemUpTime(&pAd->StaCfg.TdlsGoBackStartTime);

			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
			if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
			else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_BELOW);
			else
				TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_NONE);
			TDLS_EnablePktChannel(pAd, TDLS_FIFO_ALL);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);

			return;
		}
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_ChannelSwitchTimeOutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRT_802_11_TDLS	pTDLS = (PRT_802_11_TDLS)FunctionContext;
	PRTMP_ADAPTER	pAd = pTDLS->pAd;
	BOOLEAN	TimerCancelled;

	DBGPRINT(RT_DEBUG_WARN, ("TDLS - Failed to wait for channel switch, terminate the channel switch procedure (%02x:%02x:%02x:%02x:%02x:%02x)\n",
							pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2],
							pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5]));

	{
		ULONG Now, temp1;

		NdisGetSystemUpTime(&Now);
		temp1 = (((Now - pTDLS->ChannelSwitchTimerStartTime) * 1000) / OS_HZ);

		if (temp1 < (pTDLS->ChSwitchTimeout / 1000))
		{
			RTMPSetTimer(&pTDLS->ChannelSwitchTimeoutTimer, ((pTDLS->ChSwitchTimeout / 1000) - temp1));
			return;
		}

		if (temp1 < (pTDLS->ChSwitchTimeout / 1000))
		{
			DBGPRINT(RT_DEBUG_OFF, ("Timer  = %ld < 11 !!!\n", temp1));
		}
	}
	
	RTMPCancelTimer(&pAd->StaCfg.TdlsResponderGoBackBaseChTimer, &TimerCancelled);
	pAd->StaCfg.bTdlsCurrentDoingChannelSwitchWaitSuccess = FALSE;
	pAd->StaCfg.bDoingPeriodChannelSwitch = FALSE;

	RTMPusecDelay(300);
	NdisGetSystemUpTime(&pAd->StaCfg.TdlsGoBackStartTime);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
	if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
		TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
	else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
		TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_BELOW);
	else
		TDLS_InitChannelRelatedValue(pAd, pAd->CommonCfg.Channel, EXTCHA_NONE);
	TDLS_EnablePktChannel(pAd, TDLS_FIFO_ALL);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_TDLS_DOING_CHANNEL_SWITCH);
}
#endif /* DOT11Z_TDLS_SUPPORT */

