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
#include "dot11z_tdls.h"

UCHAR	TDLS_LLC_SNAP_WITH_CATEGORY[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x89, 0x0d, PROTO_NAME_TDLS, CATEGORY_TDLS};
UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};


VOID
TDLS_Table_Init(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR idx;
	PRT_802_11_TDLS	pTDLS = NULL;

	// initialize TDLS allocate spin lock
	NdisAllocateSpinLock(&pAd->StaCfg.TDLSEntryLock);

	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = &pAd->StaCfg.TDLSEntry[idx];

		pTDLS->Valid = FALSE;
		pTDLS->Status = TDLS_MODE_NONE;
	}

	TDLS_SearchTabInit(pAd);
}

VOID
TDLS_SearchTabInit(
	IN PRTMP_ADAPTER pAd)
{
	INT idx;
	ULONG i;

	NdisAllocateSpinLock(&pAd->StaCfg.TdlsSearchTabLock);

	pAd->StaCfg.pTdlsSearchEntryPool = kmalloc(sizeof(TDLS_SEARCH_ENTRY) * TDLS_SEARCH_POOL_SIZE, GFP_ATOMIC);
	if (pAd->StaCfg.pTdlsSearchEntryPool)
	{
		NdisZeroMemory(pAd->StaCfg.pTdlsSearchEntryPool, sizeof(TDLS_SEARCH_ENTRY) * TDLS_SEARCH_POOL_SIZE);
		initList(&pAd->StaCfg.TdlsSearchEntryFreeList);
		for (i = 0; i < TDLS_SEARCH_POOL_SIZE; i++)
			insertTailList(&pAd->StaCfg.TdlsSearchEntryFreeList, (PLIST_ENTRY)(pAd->StaCfg.pTdlsSearchEntryPool + (ULONG)i));
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Fail to alloc memory for pAd->StaCfg.pTdlsSearchEntryPool", __FUNCTION__));
		return;
	}

	for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
		initList(&pAd->StaCfg.TdlsSearchTab[idx]);

	return;
}

VOID
TDLS_Table_Destory(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR idx;
	PRT_802_11_TDLS	pTDLS = NULL;

	NdisFreeSpinLock(&pAd->StaCfg.TDLSEntryLock);

	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = &pAd->StaCfg.TDLSEntry[idx];

		pTDLS->Valid = FALSE;
		pTDLS->Status = TDLS_MODE_NONE;
	}

	TDLS_SearchTabDestory(pAd);
}

VOID
TDLS_SearchTabDestory(
	IN PRTMP_ADAPTER pAd)
{
	INT idx;
	PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

	NdisFreeSpinLock(&pAd->StaCfg.TdlsSearchTabLock);

	for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
	{
		pTdlsSearchEntry =
			(PTDLS_SEARCH_ENTRY)pAd->StaCfg.TdlsSearchTab[idx].pHead;
		while(pTdlsSearchEntry)
		{
			PTDLS_SEARCH_ENTRY pTdlsSearchEntryNext = pTdlsSearchEntry->pNext;
			TDLS_SearchEntyFree(pAd, pTdlsSearchEntry);
			pTdlsSearchEntry = pTdlsSearchEntryNext;
		}
	}

	if (pAd->StaCfg.pTdlsSearchEntryPool)
		kfree(pAd->StaCfg.pTdlsSearchEntryPool);
	pAd->StaCfg.pTdlsSearchEntryPool = NULL;	

	return;
}

PTDLS_SEARCH_ENTRY
TDLS_SearchEntyAlloc(
	IN PRTMP_ADAPTER pAd)
{
	PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsSearchTabLock);

	pTdlsSearchEntry = (PTDLS_SEARCH_ENTRY)removeHeadList(&pAd->StaCfg.TdlsSearchEntryFreeList);

	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsSearchTabLock);

	return pTdlsSearchEntry;
}


VOID
TDLS_SearchEntyFree(
	IN PRTMP_ADAPTER pAd,
	IN PTDLS_SEARCH_ENTRY pTdlsSearchEntry)
{
	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsSearchTabLock);

	insertTailList(&pAd->StaCfg.TdlsSearchEntryFreeList, (PLIST_ENTRY)pTdlsSearchEntry);

	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsSearchTabLock);

	return;
}

BOOLEAN
TLDS_SearchEntryLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac)
{
	UINT8 HashId = (*(pMac + 5) & (TDLS_SEARCH_HASH_TAB_SIZE - 1));
	PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

	pTdlsSearchEntry =
		(PTDLS_SEARCH_ENTRY)pAd->StaCfg.TdlsSearchTab[HashId].pHead;
	while (pTdlsSearchEntry)
	{
		if (MAC_ADDR_EQUAL(pMac, pTdlsSearchEntry->Addr))
			return TRUE;

		pTdlsSearchEntry = pTdlsSearchEntry->pNext;
	}
	return FALSE;
}

BOOLEAN
TDLS_SearchEntryUpdate(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac)
{
	UINT8 HashId = (*(pMac + 5) & (TDLS_SEARCH_HASH_TAB_SIZE - 1));
	PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

	if (TLDS_SearchEntryLookup(pAd, pMac) == TRUE)
		return FALSE;

	pTdlsSearchEntry = TDLS_SearchEntyAlloc(pAd);
	if (pTdlsSearchEntry)
	{
		ULONG Now;
		NdisGetSystemUpTime(&Now);

		COPY_MAC_ADDR(&pTdlsSearchEntry->Addr, pMac);
		pTdlsSearchEntry->InitRefTime = Now;
		pTdlsSearchEntry->LastRefTime = Now;
		pTdlsSearchEntry->pNext = NULL;
		insertTailList(&pAd->StaCfg.TdlsSearchTab[HashId], (PLIST_ENTRY)pTdlsSearchEntry);

		return TRUE;
	}

	return FALSE;
}

BOOLEAN
TDLS_SearchEntryDelete(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac)
{
	UINT8 HashId = (*(pMac + 5) & (TDLS_SEARCH_HASH_TAB_SIZE - 1));
	PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

	pTdlsSearchEntry =
		(PTDLS_SEARCH_ENTRY)pAd->StaCfg.TdlsSearchTab[HashId].pHead;

	while (pTdlsSearchEntry)
	{
		PTDLS_SEARCH_ENTRY pTdlsSearchEntryNext = pTdlsSearchEntry->pNext;

		if (MAC_ADDR_EQUAL(pMac, pTdlsSearchEntry->Addr))
		{
			delEntryList(&pAd->StaCfg.TdlsSearchTab[HashId], (PLIST_ENTRY)pTdlsSearchEntry);
			TDLS_SearchEntyFree(pAd, pTdlsSearchEntry);

			return TRUE;
		}

		pTdlsSearchEntry = pTdlsSearchEntryNext;
	}

	return FALSE;
}

VOID
TDLS_SearchTabReset(
	IN PRTMP_ADAPTER pAd)
{
	INT idx;
	PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

	for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
	{
		pTdlsSearchEntry =
			(PTDLS_SEARCH_ENTRY)pAd->StaCfg.TdlsSearchTab[idx].pHead;
		while(pTdlsSearchEntry)
		{
			PTDLS_SEARCH_ENTRY pTdlsSearchEntryNext = pTdlsSearchEntry->pNext;
			delEntryList(&pAd->StaCfg.TdlsSearchTab[idx], (PLIST_ENTRY)pTdlsSearchEntry);
			TDLS_SearchEntyFree(pAd, pTdlsSearchEntry);
			pTdlsSearchEntry = pTdlsSearchEntryNext;
		}
	}

	return;
}


VOID
TDLS_SearchTabMaintain(
	IN PRTMP_ADAPTER pAd)
{
	ULONG idx;
	PTDLS_SEARCH_ENTRY pTdlsSearchEntry;
	ULONG Now;

	NdisGetSystemUpTime(&Now);
	for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
	{
		pTdlsSearchEntry = (PTDLS_SEARCH_ENTRY)(pAd->StaCfg.TdlsSearchTab[idx].pHead);

		while (pTdlsSearchEntry)
		{
			PTDLS_SEARCH_ENTRY pTdlsSearchEntryNext = pTdlsSearchEntry->pNext;

			if (RTMP_TIME_AFTER(Now, pTdlsSearchEntry->LastRefTime + (TDLS_SEARCH_ENTRY_AGEOUT * OS_HZ / 1000)))
			{
				pTdlsSearchEntry->LastRefTime = Now;
					
				if (pTdlsSearchEntry->RetryCount < 7)
				{
					RT_802_11_TDLS	Tdls;

					DBGPRINT(RT_DEBUG_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x from after %d-sec silence retry %d times\n",
												pTdlsSearchEntry->Addr[0],pTdlsSearchEntry->Addr[1],pTdlsSearchEntry->Addr[2],
												pTdlsSearchEntry->Addr[3],pTdlsSearchEntry->Addr[4],pTdlsSearchEntry->Addr[5], 
												TDLS_SEARCH_ENTRY_AGEOUT, pTdlsSearchEntry->RetryCount));

					NdisZeroMemory(&Tdls, sizeof(RT_802_11_TDLS));
					Tdls.TimeOut = 0;
					COPY_MAC_ADDR(Tdls.MacAddr, pTdlsSearchEntry->Addr);
					Tdls.Valid = 1;

					MlmeEnqueue(pAd, 
								MLME_CNTL_STATE_MACHINE, 
								RT_OID_802_11_SET_TDLS_PARAM, 
								sizeof(RT_802_11_TDLS), 
								&Tdls,
								0);
					pTdlsSearchEntry->RetryCount++;
				}
				else
				{
					if (RTMP_TIME_AFTER(Now, pTdlsSearchEntry->InitRefTime + (TDLS_SEARCH_ENTRY_AGEOUT_LIMIT * OS_HZ / 1000)))
					{
						delEntryList(&pAd->StaCfg.TdlsSearchTab[idx], (PLIST_ENTRY)pTdlsSearchEntry);
						TDLS_SearchEntyFree(pAd, pTdlsSearchEntry);
					}
				}

			}

			pTdlsSearchEntry = pTdlsSearchEntryNext;
		}
	}
	return;
}


/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsCapableProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN	bTdlsCapable;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	bTdlsCapable = simple_strtol(arg, 0, 10);

	if (!bTdlsCapable && pAd->StaCfg.bTDLSCapable)
	{
		// tear	down local dls table entry
		TDLS_LinkTearDown(pAd);
		TDLS_SearchTabReset(pAd);
	}

	pAd->StaCfg.bTDLSCapable = bTdlsCapable;
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_TdlsCapableProc::(bTdlsCapable=%d)\n", 
		pObj->ioctl_if, pAd->StaCfg.bTDLSCapable));

	return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsSearchResetProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN	bTdlsRest;

	//POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	bTdlsRest = simple_strtol(arg, 0, 10);

	TDLS_SearchTabReset(pAd);

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsSetupProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR			macAddr[MAC_ADDR_LEN];
	PSTRING			value;
	INT				value_offset;
	RT_802_11_TDLS	Tdls;

	if(strlen(arg) != 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
		return FALSE;

	for (value_offset=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  //Invalid

		AtoH(value, &macAddr[value_offset++], 1);
	}

	// TDLS will not be supported when Adhoc mode
	if (INFRA_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_TRACE,("\n%02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2],
									macAddr[3], macAddr[4], macAddr[5]));

		NdisZeroMemory(&Tdls, sizeof(RT_802_11_TDLS));
		Tdls.TimeOut = 0;
		COPY_MAC_ADDR(Tdls.MacAddr, macAddr);
		Tdls.Valid = 1;

		MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE, 
					RT_OID_802_11_SET_TDLS_PARAM, 
					sizeof(RT_802_11_TDLS), 
					&Tdls,
					0);

		return TRUE;
	}

	return FALSE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsTearDownProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR			macAddr[MAC_ADDR_LEN];
	PSTRING			value;
	INT				value_offset;
	CHAR			idx;

	if(strlen(arg) != 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
		return FALSE;

	for (value_offset=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  //Invalid

		AtoH(value, &macAddr[value_offset++], 1);
	}

	// TDLS will not be supported when Adhoc mode
	if (INFRA_ON(pAd))
	{
		MLME_TDLS_REQ_STRUCT	MlmeTdlsReq;
		USHORT		Reason = REASON_UNSPECIFY;

		DBGPRINT(RT_DEBUG_TRACE,("\n%02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2],
									macAddr[3], macAddr[4], macAddr[5]));

		idx = TDLS_SearchLinkId(pAd, macAddr);

		if (idx >= 0)
		{
			Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
			pAd->StaCfg.TDLSEntry[idx].Valid	= FALSE;
			pAd->StaCfg.TDLSEntry[idx].Status	= TDLS_MODE_NONE;
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_TEAR_DOWN,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);

			return TRUE;
		}
	}

	return FALSE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_SetupRequestAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_SetupRequestAction\n"));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildSetupRequest(pAd, pOutBuffer, &FrameLen, pTDLS);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS setup request send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_SetupRequestAction\n"));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_SetupResponseAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_SetupResponseAction\n"));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildSetupResponse(pAd, pOutBuffer, &FrameLen, pTDLS, RsnLen, pRsnIe, FTLen, pFTIe, TILen, pTIIe, StatusCode);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS send setup response pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_SetupResponseAction\n"));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_SetupConfirmAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_SetupConfirmAction\n"));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildSetupConfirm(pAd, pOutBuffer, &FrameLen, pTDLS, RsnLen, pRsnIe, FTLen, pFTIe, TILen, pTIIe, StatusCode);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS setup confirm pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_SetupConfirmAction\n"));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_LinkTearDown(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR		idx;
	BOOLEAN		TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_LinkTearDown\n"));

	// tear down tdls table entry
	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = &pAd->StaCfg.TDLSEntry[idx];
		if (pTDLS->Valid && (pTDLS->Status >= TDLS_MODE_CONNECTED))
		{
			pTDLS->Status	= TDLS_MODE_NONE;
			pTDLS->Valid	= FALSE;
			RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);

			if (!VALID_WCID(pTDLS->MacTabMatchWCID))
			return;

			TDLS_TearDownAction(pAd, pTDLS, TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON, TRUE);

			MacTableDeleteEntry(pAd,pTDLS->MacTabMatchWCID, pTDLS->MacAddr);
		}
		else if (pTDLS->Valid)
		{
			pTDLS->Status	= TDLS_MODE_NONE;
			pTDLS->Valid	= FALSE;
			RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
		}	
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_LinkTearDown\n"));
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_TearDownAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UINT16	ReasonCode,
	IN BOOLEAN	bDirect)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	idx = 1;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_TearDownAction\n"));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildTeardown(pAd, pOutBuffer, &FrameLen, pTDLS, ReasonCode);

	if (bDirect == TRUE)
		idx = pTDLS->MacTabMatchWCID;
	else
		idx = BSSID_WCID;

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[idx], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS teardown send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_TearDownAction\n"));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InitPeerEntryRateCapability(
	IN	PRTMP_ADAPTER pAd,
	IN	MAC_TABLE_ENTRY *pEntry,
	IN USHORT *pCapabilityInfo,
	IN UCHAR SupportRateLens,
	IN UCHAR *pSupportRates,
	IN UCHAR HtCapabilityLen,
	IN HT_CAPABILITY_IE *pHtCapability)
{
	UCHAR MaxSupportedRate = RATE_11;
	UCHAR MaxSupportedRateIn500Kbps = 0;
	UCHAR idx;

    for (idx = 0; idx < SupportRateLens; idx++)
    {
        if (MaxSupportedRateIn500Kbps < (pSupportRates[idx] & 0x7f)) 
            MaxSupportedRateIn500Kbps = pSupportRates[idx] & 0x7f;
    }

	switch (MaxSupportedRateIn500Kbps)
	{
		case 108: MaxSupportedRate = RATE_54;   break;
		case 96:  MaxSupportedRate = RATE_48;   break;
		case 72:  MaxSupportedRate = RATE_36;   break;
		case 48:  MaxSupportedRate = RATE_24;   break;
		case 36:  MaxSupportedRate = RATE_18;   break;
		case 24:  MaxSupportedRate = RATE_12;   break;
		case 18:  MaxSupportedRate = RATE_9;    break;
		case 12:  MaxSupportedRate = RATE_6;    break;
		case 22:  MaxSupportedRate = RATE_11;   break;
		case 11:  MaxSupportedRate = RATE_5_5;  break;
		case 4:   MaxSupportedRate = RATE_2;    break;
		case 2:   MaxSupportedRate = RATE_1;    break;
		default:  MaxSupportedRate = RATE_11;   break;
	}

	pEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxDesiredRate, MaxSupportedRate);

	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	}
	else
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}

	pEntry->MaxHTPhyMode.field.BW = BW_20;
	pEntry->MinHTPhyMode.field.BW = BW_20;
				
#ifdef DOT11_N_SUPPORT
	pEntry->HTCapability.MCSSet[0] = 0;
	pEntry->HTCapability.MCSSet[1] = 0;

	// If this Entry supports 802.11n, upgrade to HT rate. 
	if ((HtCapabilityLen != 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
	{
		UCHAR	j, bitmask; //k,bitmask;
		CHAR    ii;

		DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Receive Peer HT Capable STA from %02x:%02x:%02x:%02x:%02x:%02x\n",
								pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
								pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));

		if ((pHtCapability->HtCapInfo.GF) &&
			(pAd->CommonCfg.DesiredHtPhy.GF) &&
			(pAd->StaActive.SupportedHtPhy.GF))
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
		}
		else
		{	
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
			pAd->MacTab.fAnyStationNonGF = TRUE;
			pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
		}

		if ((pHtCapability->HtCapInfo.ChannelWidth) &&
			(pAd->CommonCfg.DesiredHtPhy.ChannelWidth) &&
			(pAd->StaActive.SupportedHtPhy.ChannelWidth))
		{
			pEntry->MaxHTPhyMode.field.BW= BW_40;
			pEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor40)&(pHtCapability->HtCapInfo.ShortGIfor40));
		}
		else
		{	
			pEntry->MaxHTPhyMode.field.BW = BW_20;
			pEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor20)&(pHtCapability->HtCapInfo.ShortGIfor20));
			pAd->MacTab.fAnyStation20Only = TRUE;
		}

		// find max fixed rate
		for (ii = 23; ii >= 0; ii--) // 3*3
		{
			j = ii/8;
			bitmask = (1<<(ii-(j*8)));
			if ( (pAd->StaCfg.DesiredHtPhyInfo.MCSSet[j]&bitmask) && (pHtCapability->MCSSet[j]&bitmask))
			{
				pEntry->MaxHTPhyMode.field.MCS = ii;
				break;
			}

			if (ii==0)
				break;
		}

		if (pAd->StaCfg.DesiredTransmitSetting.field.MCS != MCS_AUTO)
		{
			if (pAd->StaCfg.DesiredTransmitSetting.field.MCS == 32)
			{
				// Fix MCS as HT Duplicated Mode
				pEntry->MaxHTPhyMode.field.BW = 1;
				pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
				pEntry->MaxHTPhyMode.field.STBC = 0;
				pEntry->MaxHTPhyMode.field.ShortGI = 0;
				pEntry->MaxHTPhyMode.field.MCS = 32;
			}
			else if (pEntry->MaxHTPhyMode.field.MCS > pAd->StaCfg.HTPhyMode.field.MCS)
			{
				// STA supports fixed MCS 
				pEntry->MaxHTPhyMode.field.MCS = pAd->StaCfg.HTPhyMode.field.MCS;
			}
		}

		pEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
		pEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
		pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
		pEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
		pEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;				
		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

		if (pHtCapability->HtCapInfo.ShortGIfor20)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);
		if (pHtCapability->HtCapInfo.ShortGIfor40)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);
		if (pHtCapability->HtCapInfo.TxSTBC)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
		if (pHtCapability->HtCapInfo.RxSTBC)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
		if (pHtCapability->ExtHtCapInfo.PlusHTC)				
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HTC_CAPABLE);
		if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)				
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);	
		if (pHtCapability->ExtHtCapInfo.MCSFeedback == 0x03)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);		

		NdisMoveMemory(&pEntry->HTCapability, pHtCapability, sizeof(HT_CAPABILITY_IE));
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
	}
#endif // DOT11_N_SUPPORT //

	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
	pEntry->CurrTxRate = pEntry->MaxSupportedRate;

	if (pAd->StaCfg.bAutoTxRateSwitch == TRUE)
	{
		UCHAR TableSize = 0;

		MlmeSelectTxRateTable(pAd, pEntry, &pEntry->pTable, &TableSize, &pEntry->CurrTxRateIndex);
		pEntry->bAutoTxRateSwitch = TRUE;
	}
	else
	{
		pEntry->HTPhyMode.field.MODE	= pAd->StaCfg.HTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.MCS	= pAd->StaCfg.HTPhyMode.field.MCS;
		pEntry->bAutoTxRateSwitch = FALSE;

		RTMPUpdateLegacyTxSetting((UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}

	pEntry->RateLen = SupportRateLens;
}

/*
	========================================================================
	
	Routine Description:
		Classify TDLS message type

	Arguments:
		TDLSActionType		Value of TDLS message type
		MsgType				Internal Message definition for MLME state machine
		
	Return Value:
		TRUE		Found appropriate message type
		FALSE		No appropriate message type
			
	========================================================================
*/
BOOLEAN
TDLS_MsgTypeSubst(
	IN	UCHAR	TDLSActionType,
	OUT	INT		*MsgType)
{
	switch (TDLSActionType)
	{
		case TDLS_ACTION_CODE_SETUP_REQUEST:
			*MsgType = MT2_PEER_TDLS_SETUP_REQ;
			break;
        case TDLS_ACTION_CODE_SETUP_RESPONSE:
            *MsgType = MT2_PEER_TDLS_SETUP_RSP;
			break;
        case TDLS_ACTION_CODE_SETUP_CONFIRM:
            *MsgType = MT2_PEER_TDLS_SETUP_CONF;
			break;
        case TDLS_ACTION_CODE_TEARDOWN:
            *MsgType = MT2_PEER_TDLS_TEAR_DOWN;
			break;
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("TDLS_MsgTypeSubst : unsupported TDLS Action Type(%d); \n", TDLSActionType));
			return FALSE;		
	}	

	return TRUE;
}

/*
    ==========================================================================
    Description:
		Check whether the received frame is TDLS frame.

	Arguments:
		pAd				-	pointer to our pAdapter context			
		pData			-	the received frame
		DataByteCount 	-	the received frame's length				
       
    Return:
         TRUE 			-	This frame is TDLS frame
         FALSE 			-	otherwise
    ==========================================================================
*/
BOOLEAN TDLS_CheckTDLSframe(
    IN PRTMP_ADAPTER    pAd,
    IN PUCHAR           pData,
    IN ULONG            DataByteCount)
{
    if(DataByteCount < (LENGTH_802_1_H + LENGTH_TDLS_H))
        return FALSE;


	/* LLC Header(6) + TDLS Ethernet Type(2) + Protocol(1) + Category(1) */
	if (!NdisEqualMemory(TDLS_LLC_SNAP_WITH_CATEGORY, pData, LENGTH_802_1_H + 2))
		return FALSE;

	
    return TRUE;
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID TDLS_CntlOidTDLSRequestProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS			pTDLS = (PRT_802_11_TDLS)Elem->Msg;
	MLME_TDLS_REQ_STRUCT	MlmeTdlsReq;
	USHORT		Reason = REASON_UNSPECIFY;
	BOOLEAN		TimerCancelled;
	INT			Idx, i;
	
	DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS_CntlOidTDLSRequestProc set %02x:%02x:%02x:%02x:%02x:%02x with Valid=%d, Status=%d\n",
		pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2], pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5],
		pTDLS->Valid, pTDLS->Status));

	if (!pAd->StaCfg.bTDLSCapable)
		return;

	if (!INFRA_ON(pAd))
		return;

	Idx = TDLS_SearchLinkId(pAd, pTDLS->MacAddr);
	
	if (Idx == -1) // not found and the entry is not full
	{
		if (pTDLS->Valid) 
		{
			// 1. Enable case, start TDLS setup procedure
			for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
			{
				if (!pAd->StaCfg.TDLSEntry[i].Valid)
				{
					NdisMoveMemory(&pAd->StaCfg.TDLSEntry[i], pTDLS, sizeof(RT_802_11_TDLS_UI));
					TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[i], Reason, FALSE);
					MlmeEnqueue(pAd,
								TDLS_STATE_MACHINE,
								MT2_MLME_TDLS_SETUP_REQ,
								sizeof(MLME_TDLS_REQ_STRUCT),
								&MlmeTdlsReq,
								0);
					DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS setup case\n"));
					break;
				}
			}	
		}
		else
			DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in Idx = -1 (Valid=%d)\n", pTDLS->Valid));

	}
	else  if (Idx == MAX_NUM_OF_TDLS_ENTRY)		// not found and full
	{
		if (pTDLS->Valid) 
		{
			// 2. table full, cancel the non-finished entry and restart a new one
			for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
			{
				if ((pAd->StaCfg.TDLSEntry[i].Valid) &&(pAd->StaCfg.TDLSEntry[i].Status < TDLS_MODE_CONNECTED))
				{
					// update mac case
					NdisMoveMemory(&pAd->StaCfg.TDLSEntry[i], pTDLS, sizeof(RT_802_11_TDLS));
					TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[i], Reason, FALSE);
					MlmeEnqueue(pAd,
								TDLS_STATE_MACHINE,
								MT2_MLME_TDLS_SETUP_REQ,
								sizeof(MLME_TDLS_REQ_STRUCT),
								&MlmeTdlsReq,
								0);
					DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS restart case\n"));
					break;
				}
			}	
		}
		else
			DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in Idx = MAX_NUM_OF_TDLS_ENTRY (Valid=%d)\n", pTDLS->Valid));
	}
	else	// found one in entry
	{
		if ((!pTDLS->Valid) && (pAd->StaCfg.TDLSEntry[Idx].Status >= TDLS_MODE_CONNECTED))
		{
			// 3. Disable TDLS link case, just tear down TDLS link
			Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
			pAd->StaCfg.TDLSEntry[Idx].Valid	= FALSE;
			pAd->StaCfg.TDLSEntry[Idx].Status	= TDLS_MODE_NONE;
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[Idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_TEAR_DOWN,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);
			DBGPRINT(RT_DEBUG_TRACE,("CNTL - start tear down procedure\n"));
		}
		else if ((pTDLS->Valid) && (pAd->StaCfg.TDLSEntry[Idx].Status >= TDLS_MODE_CONNECTED)) 
		{
			// 4. re-setup case, tear down old link and re-start TDLS setup procedure 
			Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
			pAd->StaCfg.TDLSEntry[Idx].Valid	= FALSE;
			pAd->StaCfg.TDLSEntry[Idx].Status	= TDLS_MODE_NONE;
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[Idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_TEAR_DOWN,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);
		
			RTMPCancelTimer(&pAd->StaCfg.TDLSEntry[Idx].Timer, &TimerCancelled);
			NdisMoveMemory(&pAd->StaCfg.TDLSEntry[Idx], pTDLS, sizeof(RT_802_11_TDLS_UI));
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[Idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_SETUP_REQ,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);
			DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS retry setup procedure\n"));
		}
		else
		{
			DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in entry - %d - Valid=%d, Status=%d\n",
				Idx, pAd->StaCfg.TDLSEntry[Idx].Valid, pAd->StaCfg.TDLSEntry[Idx].Status));
		}
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/

/* Not found and full : return MAX_NUM_OF_TDLS_ENTRY
 *  not found and the entry is not full : return -1
 */
INT TDLS_SearchLinkId(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pAddr)
{
	INT		i = 0;
	UCHAR	empty = 0;
	PRT_802_11_TDLS	pTDLS = NULL;
	
	for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
	{
		pTDLS = &pAd->StaCfg.TDLSEntry[i];
		if (!pTDLS->Valid)
			empty |= 1;
		
		if (pTDLS->Valid && MAC_ADDR_EQUAL(pAddr, pTDLS->MacAddr))
		{
			DBGPRINT(RT_DEBUG_INFO, ("TDLS_SearchLinkId - Find Link ID with Peer Address %02x:%02x:%02x:%02x:%02x:%02x(%d) \n", 
            	pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5], i));
			break;
		}
	}

	if (i == MAX_NUM_OF_TDLS_ENTRY)
	{
		if (empty == 0)
		{
			DBGPRINT(RT_DEBUG_INFO, ("TDLS_SearchLinkId -  not found and full\n"));
			return MAX_NUM_OF_TDLS_ENTRY;
		}
		else
		{
			DBGPRINT(RT_DEBUG_INFO, ("TDLS_SearchLinkId -  not found\n"));
			return -1;
		}	
	}

	return i;
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID TDLS_MlmeParmFill(
	IN PRTMP_ADAPTER pAd, 
	IN OUT MLME_TDLS_REQ_STRUCT *pTdlsReq,
	IN PRT_802_11_TDLS pTdls,
	IN USHORT Reason,
	IN BOOLEAN IsViaAP) 
{
	pTdlsReq->pTDLS = pTdls;
	pTdlsReq->Reason = Reason;
	pTdlsReq->IsViaAP = IsViaAP;
}

/*
	========================================================================
	
	Routine Description:
		It is used to derive the TDLS Peer Key and its identifier TPK-Name.
		(IEEE 802.11z/D4.0, 8.5.9.1)
		
	Arguments:

	Return Value:

	Note:
		TPK = KDF-256(0, "TDLS PMK", min(MAC_I, MAC_R) || max(MAC_I, MAC_R) || min(SNonce, ANonce) || max(SNonce, ANonce) || BSSID || N_KEY)
		TPK-Name = Truncate-128(SHA-256(min(MAC_I, MAC_R) || max(MAC_I, MAC_R) || min(SNonce, ANonce) || max(SNonce, ANonce) || BSSID || 256))
	========================================================================
*/
VOID TDLS_FTDeriveTPK(
	IN	PUCHAR 	mac_i,
	IN	PUCHAR 	mac_r,
	IN	PUCHAR 	a_nonce,
	IN	PUCHAR 	s_nonce,
	IN	PUCHAR 	bssid,
	IN	UINT	key_len,
 	OUT	PUCHAR	tpk,
	OUT	PUCHAR	tpk_name)
{	
	UCHAR	temp_result[64];
	UCHAR   context[128];
	UINT    c_len=0; 
	UCHAR	temp_var[32];
	//UINT	key_len = LEN_PMK;
	USHORT	len_in_bits = (key_len << 3);
	UCHAR	key_material[LEN_PMK];

	// ===============================
	// 		TPK derivation	
	// ===============================

	// construct the concatenated context for TPK
	// min(MAC_I, MAC_R)	(6 bytes)
	// max(MAC_I, MAC_R)	(6 bytes)
	// min(SNonce, ANonce)	(32 bytes)
	// max(SNonce, ANonce)	(32 bytes)
	// BSSID				(6 bytes)
	// Number of Key in bits(2 bytes)

	// Initial the related parameter
	NdisZeroMemory(temp_result, 64);
	NdisZeroMemory(context, 128);
	NdisZeroMemory(temp_var, 32);
	c_len = 0;

	// concatenate min(MAC_I, MAC_R) with 6-octets
	if (RTMPCompareMemory(mac_i, mac_r, 6) == 1)
		NdisMoveMemory(temp_var, mac_r, 6);
	else
		NdisMoveMemory(temp_var, mac_i, 6);
    NdisMoveMemory(&context[c_len], temp_var, 6);
	c_len += 6;

	// concatenate max(MAC_I, MAC_R) with 6-octets
	if (RTMPCompareMemory(mac_i, mac_r, 6) == 1)
		NdisMoveMemory(temp_var, mac_i, 6);
	else
		NdisMoveMemory(temp_var, mac_r, 6);
    NdisMoveMemory(&context[c_len], temp_var, 6);
	c_len += 6;

	// concatenate min(SNonce, ANonce) with 32-octets 
	if (RTMPCompareMemory(s_nonce, a_nonce, 32) == 1)
		NdisMoveMemory(temp_var, a_nonce, 32);
	else
		NdisMoveMemory(temp_var, s_nonce, 32);
    NdisMoveMemory(&context[c_len], temp_var, 32);
	c_len += 32;
	
	// concatenate max(SNonce, ANonce) with 32-octets
	if (RTMPCompareMemory(s_nonce, a_nonce, 32) == 1)
		NdisMoveMemory(temp_var, s_nonce, 32);
	else
		NdisMoveMemory(temp_var, a_nonce, 32);
    NdisMoveMemory(&context[c_len], temp_var, 32);
	c_len += 32;

	// concatenate the BSSID with 6-octets 
	NdisMoveMemory(&context[c_len], bssid, MAC_ADDR_LEN);
	c_len += MAC_ADDR_LEN;

	// concatenate the N_KEY with 2-octets
	NdisMoveMemory(&context[c_len], &len_in_bits, 2);
	c_len += 2;
	
	//hex_dump("TDLS_FTDeriveTPK", context, 128);

	// Zero key material
	NdisZeroMemory(key_material, LEN_PMK);
	
	// Calculate a key material through FT-KDF
	KDF(key_material, 
			LEN_PMK, 
			(PUCHAR)"TDLS PMK", 
			8, 
			context, 
			c_len, 
			temp_result, 
			key_len);
	NdisMoveMemory(tpk, temp_result, key_len);

	hex_dump("TPK ", tpk , key_len);


	// ===============================
	// 		TPK-Name derivation	
	// ===============================

	// construct the concatenated context for TPK-Name
	// min(MAC_I, MAC_R)	(6 bytes)
	// max(MAC_I, MAC_R)	(6 bytes)
	// min(SNonce, ANonce)	(32 bytes)
	// max(SNonce, ANonce)	(32 bytes)
	// BSSID				(6 bytes)
	// Number of Key in bits(2 bytes)

	// The context is the same as the contxex of TPK.
	
	// Initial the related parameter
	NdisZeroMemory(temp_result, 64);

	// derive TPK-Name
	RT_SHA256(context, c_len, temp_result);
	NdisMoveMemory(tpk_name, temp_result, LEN_PMK_NAME);

	hex_dump("TPK-Name ", tpk_name, LEN_PMK_NAME);

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN MlmeTdlsReqSanity(
	IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PRT_802_11_TDLS *pTDLS,
    OUT PUINT16 pReason,
    OUT BOOLEAN *pIsViaAP)
{
	MLME_TDLS_REQ_STRUCT *pInfo;

    pInfo = (MLME_TDLS_REQ_STRUCT *)Msg;
    
	*pTDLS = pInfo->pTDLS;
	*pReason = pInfo->Reason;
	*pIsViaAP = pInfo->IsViaAP;// default = FALSE, not pass through AP

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_SendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN 		bQosNull)
{
	UCHAR	NullFrame[48];
	ULONG	Length;
	PHEADER_802_11	pHeader_802_11;
	UCHAR	idx;
	PRT_802_11_TDLS	pTDLS = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_SendNullFrame\n"));

    // WPA 802.1x secured port control
    if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || 
         (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
         (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || 
         (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
			  || (pAd->StaCfg.IEEE8021X == TRUE)		
#endif 		
#ifdef WAPI_SUPPORT
		  || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWAICERT)
		  || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWAIPSK)
#endif // WAPI_SUPPORT //
        ) &&
       (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED)) 
	{
		return;
	}

	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[idx];

		if ((pTDLS->Valid) && (pTDLS->Status == TDLS_MODE_CONNECTED))
		{
			NdisZeroMemory(NullFrame, 48);
			Length = sizeof(HEADER_802_11);

			pHeader_802_11 = (PHEADER_802_11) NullFrame;

			pHeader_802_11->FC.Type = BTYPE_DATA;
			pHeader_802_11->FC.SubType = SUBTYPE_NULL_FUNC;
			pHeader_802_11->FC.ToDs = 0;
			pHeader_802_11->FC.FrDs = 0;
			COPY_MAC_ADDR(pHeader_802_11->Addr1, pTDLS->MacAddr);
			COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pHeader_802_11->Addr3, pAd->CommonCfg.Bssid);

			if (pAd->CommonCfg.bAPSDForcePowerSave)
			{
				pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
			}
			else
			{
				pHeader_802_11->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE) ? 1: 0;
			}

			pHeader_802_11->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);

			pAd->Sequence++;
			pHeader_802_11->Sequence = pAd->Sequence;

			// Prepare QosNull function frame
			if (bQosNull)
			{
				pHeader_802_11->FC.SubType = SUBTYPE_QOS_NULL;

				// copy QOS control bytes
				NullFrame[Length]	=  0;
				NullFrame[Length+1] =  0;
				Length += 2;// if pad with 2 bytes for alignment, APSD will fail
			}

			HAL_KickOutNullFrameTx(pAd, 0, NullFrame, Length);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_SendNullFrame\n"));
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_LinkMaintenance(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR idx;
	PRT_802_11_TDLS	pTDLS = NULL;

	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[idx];

		if ((pTDLS->Valid == TRUE) && (pTDLS->Status == TDLS_MODE_CONNECTED))
		{
			UCHAR wcid = pTDLS->MacTabMatchWCID;
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[wcid];
		
			if(!IS_ENTRY_TDLS(pEntry))
				continue;

			NdisAcquireSpinLock(&pAd->MacTabLock);
			pEntry->NoDataIdleCount++;
			NdisReleaseSpinLock(&pAd->MacTabLock);

			// delete those TDLS entry that has been idle for a long time
			if (pEntry->NoDataIdleCount >= TDLS_ENTRY_AGEOUT_TIME)
			{
				DBGPRINT(RT_DEBUG_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x from TDLS #%d after %d-sec silence\n",
											pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
											pEntry->Addr[4],pEntry->Addr[5], idx, TDLS_ENTRY_AGEOUT_TIME));

				NdisAcquireSpinLock(&pAd->StaCfg.TDLSEntryLock);
				pTDLS->Valid = FALSE;
				pTDLS->Status = TDLS_MODE_NONE;
				NdisReleaseSpinLock(&pAd->StaCfg.TDLSEntryLock);

				TDLS_TearDownAction(pAd, pTDLS, TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON, FALSE);

				MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, pTDLS->MacAddr);
			}
		}
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT Set_TdlsEntryInfo_Display_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR arg)
{
	INT i;

	DBGPRINT(RT_DEBUG_OFF, ("\n%-19s\n", "MAC\n"));
	for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
	{
		if ((pAd->StaCfg.TDLSEntry[i].Valid) && (pAd->StaCfg.TDLSEntry[i].Status == TDLS_MODE_CONNECTED))
		{
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pAd->StaCfg.TDLSEntry[i].MacTabMatchWCID];

			DBGPRINT(RT_DEBUG_OFF, ("%02x:%02x:%02x:%02x:%02x:%02x  \n",
				pAd->StaCfg.TDLSEntry[i].MacAddr[0], pAd->StaCfg.TDLSEntry[i].MacAddr[1], pAd->StaCfg.TDLSEntry[i].MacAddr[2],
				pAd->StaCfg.TDLSEntry[i].MacAddr[3], pAd->StaCfg.TDLSEntry[i].MacAddr[4], pAd->StaCfg.TDLSEntry[i].MacAddr[5]));
			//DBGPRINT(RT_DEBUG_OFF, ("%-8d\n", pAd->StaCfg.DLSEntry[i].TimeOut));

			DBGPRINT(RT_DEBUG_OFF, ("\n"));
			DBGPRINT(RT_DEBUG_OFF, ("\n%-19s%-4s%-4s%-4s%-4s%-7s%-7s%-7s","MAC", "AID", "BSS", "PSM", "WMM", "RSSI0", "RSSI1", "RSSI2"));
#ifdef DOT11_N_SUPPORT			
			DBGPRINT(RT_DEBUG_OFF, ("%-8s%-10s%-6s%-6s%-6s%-6s", "MIMOPS", "PhMd", "BW", "MCS", "SGI", "STBC"));
#endif // DOT11_N_SUPPORT //
			DBGPRINT(RT_DEBUG_OFF, ("\n%02X:%02X:%02X:%02X:%02X:%02X  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->Aid));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->apidx));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->PsMode));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi0));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi1));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi2));
#ifdef DOT11_N_SUPPORT
			DBGPRINT(RT_DEBUG_OFF, ("%-8d", (int)pEntry->MmpsMode));
			DBGPRINT(RT_DEBUG_OFF, ("%-10s", GetPhyMode(pEntry->HTPhyMode.field.MODE)));
			DBGPRINT(RT_DEBUG_OFF, ("%-6s", GetBW(pEntry->HTPhyMode.field.BW)));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.MCS));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.ShortGI));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.STBC));
#endif // DOT11_N_SUPPORT //
			DBGPRINT(RT_DEBUG_OFF, ("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount, 
						(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0));
			DBGPRINT(RT_DEBUG_OFF, ("\n"));

		}
	}

	return TRUE;
}

VOID
TDLS_DiscoveryPacketLearnAndCheck(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR			pAddr)
{
	BOOLEAN				bValid = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("====> TDLS_DiscoveryPacketLearnAndCheck\n"));

	bValid = TDLS_SearchEntryUpdate(pAd, pAddr);

	if (bValid)
	{
		RT_802_11_TDLS	Tdls;

		NdisZeroMemory(&Tdls, sizeof(RT_802_11_TDLS));
		Tdls.TimeOut = 0;
		COPY_MAC_ADDR(Tdls.MacAddr, pAddr);
		Tdls.Valid = 1;

		MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE, 
					RT_OID_802_11_SET_TDLS_PARAM, 
					sizeof(RT_802_11_TDLS), 
					&Tdls,
					0);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<==== TDLS_DiscoveryPacketLearnAndCheck\n"));

	return;
}

VOID TDLS_InitChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	IN HT_CAPABILITY_IE *pHtCapability)
{
	UCHAR	Value = 0;
	UINT32	Data = 0;

#ifdef RTMP_MAC_PCI
	// In power save , We will force use 1R.
	// So after link up, check Rx antenna # again.
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
	if(pAd->Antenna.field.RxPath == 3)
	{
		Value |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		Value |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		Value |= (0x0);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
	pAd->StaCfg.BBPR3 = Value;
#endif // RTMP_MAC_PCI //

	//pAd->CommonCfg.CentralChannel = pAd->MlmeAux.CentralChannel;
	//pAd->CommonCfg.Channel = pAd->MlmeAux.Channel;

#ifdef DOT11_N_SUPPORT
	// Change to AP channel
    if ((pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel) &&
		(pHtCapability->HtCapInfo.ChannelWidth == BW_40))
	{	
		// Must using 40MHz.
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
			
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		Value |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);
		
		//  RX : control channel at lower 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
		Value &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
#ifdef RTMP_MAC_PCI
        pAd->StaCfg.BBPR3 = Value;
#endif // RTMP_MAC_PCI //

		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data &= 0xfffffffe;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);
		
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
            DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}	

		DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower !!! Control Channel at Below. Central = %d \n", pAd->CommonCfg.CentralChannel ));
	}
	else if ((pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel) &&
			(pHtCapability->HtCapInfo.ChannelWidth == BW_40))
    {	
	    // Must using 40MHz.
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
	    AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		Value |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);
		
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data |= 0x1;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
	    Value |= (0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
#ifdef RTMP_MAC_PCI
        pAd->StaCfg.BBPR3 = Value;
#endif // RTMP_MAC_PCI //
	
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
			    DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}

	    DBGPRINT(RT_DEBUG_TRACE, ("!!! 40MHz Upper !!! Control Channel at UpperCentral = %d \n", pAd->CommonCfg.CentralChannel ));
    }
    else
#endif // DOT11_N_SUPPORT //
    {
	    pAd->CommonCfg.BBPCurrentBW = BW_20;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);
		
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data &= 0xfffffffe;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
		Value &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
#ifdef RTMP_MAC_PCI
        pAd->StaCfg.BBPR3 = Value;
#endif // RTMP_MAC_PCI //
		
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);
			DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}
		
	    DBGPRINT(RT_DEBUG_TRACE, ("!!! 20MHz !!! \n" ));
	}

	RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);
}
#endif // DOT11Z_TDLS_SUPPORT //
 
