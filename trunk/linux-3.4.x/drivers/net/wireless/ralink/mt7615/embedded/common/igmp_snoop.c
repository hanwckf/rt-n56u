#ifdef IGMP_SNOOP_SUPPORT

#include "rt_config.h"
#include "ipv6.h"
#include "igmp_snoop.h"


UINT16 IPv6MulticastFilterExclued[] =
{
	IPV6_NEXT_HEADER_ICMPV6,	/* ICMPv6. */
	IPV6_NEXT_HEADER_PIM,		/* PIM. */
};
#define IPV6_MULTICAST_FILTER_EXCLUED_SIZE  \
	(sizeof(IPv6MulticastFilterExclued) / sizeof(UINT16))

static inline void initFreeEntryList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList)
{
	int i;

	for (i = 0; i < FREE_MEMBER_POOL_SIZE; i++)
		insertTailList(pList, (RT_LIST_ENTRY *)&(pMulticastFilterTable->freeMemberPool[i]));

	return;
}

static inline PMEMBER_ENTRY AllocaGrpMemberEntry(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable)
{
	PMEMBER_ENTRY pMemberEntry;

	RTMP_SEM_LOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);

	pMemberEntry = (PMEMBER_ENTRY)removeHeadList(&pMulticastFilterTable->freeEntryList);

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);

	return (PMEMBER_ENTRY)pMemberEntry;
}

static inline VOID FreeGrpMemberEntry(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PMEMBER_ENTRY pEntry)
{
	RTMP_SEM_LOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);

	insertTailList(&pMulticastFilterTable->freeEntryList, (RT_LIST_ENTRY *)pEntry);

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
}

static VOID IGMPTableDisplay(
	IN PRTMP_ADAPTER pAd);

static BOOLEAN isIgmpMacAddr(
	IN PUCHAR pMacAddr);

static VOID InsertIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr);

static VOID DeleteIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr);

static VOID DeleteIgmpMemberList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList);

/*
    ==========================================================================
    Description:
        This routine init the entire IGMP table.
    ==========================================================================
 */
VOID MulticastFilterTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE *ppMulticastFilterTable)
{

	if ((pAd->chipCap.asic_caps & fASIC_CAP_IGMP_SNOOP_OFFLOAD) == fASIC_CAP_IGMP_SNOOP_OFFLOAD)
	{
		return;
	}

	/* Initialize MAC table and allocate spin lock */
	os_alloc_mem(NULL, (UCHAR **)ppMulticastFilterTable, sizeof(MULTICAST_FILTER_TABLE));
	if (*ppMulticastFilterTable == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s unable to alloc memory for Multicase filter table, size=%lu\n",
			__FUNCTION__, (ULONG)sizeof(MULTICAST_FILTER_TABLE)));
		return;
	}

	NdisZeroMemory(*ppMulticastFilterTable, sizeof(MULTICAST_FILTER_TABLE));
	NdisAllocateSpinLock(pAd, &((*ppMulticastFilterTable)->MulticastFilterTabLock));

	NdisAllocateSpinLock(pAd, &((*ppMulticastFilterTable)->FreeMemberPoolTabLock));
	initList(&((*ppMulticastFilterTable)->freeEntryList));
	initFreeEntryList(*ppMulticastFilterTable, &((*ppMulticastFilterTable)->freeEntryList));
	return;
}

/*
    ==========================================================================
    Description:
        This routine reset the entire IGMP table.
    ==========================================================================
 */
VOID MultiCastFilterTableReset(RTMP_ADAPTER *pAd,
	IN PMULTICAST_FILTER_TABLE *ppMulticastFilterTable)
{
	if ((pAd->chipCap.asic_caps & fASIC_CAP_IGMP_SNOOP_OFFLOAD) == fASIC_CAP_IGMP_SNOOP_OFFLOAD)
	{
		return;
	}

	if(*ppMulticastFilterTable == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __FUNCTION__));
		return;
	}

	NdisFreeSpinLock(&((*ppMulticastFilterTable)->FreeMemberPoolTabLock));
	NdisFreeSpinLock(&((*ppMulticastFilterTable)->MulticastFilterTabLock));
	os_free_mem(*ppMulticastFilterTable);
	*ppMulticastFilterTable = NULL;
}

/*
    ==========================================================================
    Description:
        Display all entrys in IGMP table
    ==========================================================================
 */
static VOID IGMPTableDisplay(
	IN PRTMP_ADAPTER pAd)
{
	int i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if (pMulticastFilterTable == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("%s Multicase filter table is not ready.\n", __FUNCTION__));
		return;
	}

	/* if FULL, return */
	if (pMulticastFilterTable->Size == 0)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("Table empty.\n"));
		return;
	}

	/* allocate one MAC entry */
	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	for (i = 0; i< MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++)
	{
		/* pick up the first available vacancy */
		if (pMulticastFilterTable->Content[i].Valid == TRUE)
		{
			PMEMBER_ENTRY pMemberEntry = NULL;
			pEntry = &pMulticastFilterTable->Content[i];

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("IF(%s) entry #%d, type=%s, GrpId=(%02x:%02x:%02x:%02x:%02x:%02x) memberCnt=%d\n",
				RTMP_OS_NETDEV_GET_DEVNAME(pEntry->net_dev), i, (pEntry->type==0 ? "static":"dynamic"),
				PRINT_MAC(pEntry->Addr), IgmpMemberCnt(&pEntry->MemberList)));

			pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;
			while (pMemberEntry)
			{
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("member mac=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
										PRINT_MAC(pMemberEntry->Addr)));

				pMemberEntry = pMemberEntry->pNext;
			}
		}
	}

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	return;
}

/*
    ==========================================================================
    Description:
        Add and new entry into MAC table
    ==========================================================================
 */
BOOLEAN MulticastFilterTableInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN UINT8 type,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT8 WlanIndex)
{
	UCHAR HashIdx;
	int i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL, *pCurrEntry, *pPrevEntry;
	PMEMBER_ENTRY pMemberEntry;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if (pMulticastFilterTable == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __FUNCTION__));
		return FALSE;
	}

	/* if FULL, return */
	if (pMulticastFilterTable->Size >= MAX_LEN_OF_MULTICAST_FILTER_TABLE)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table full. max-entries = %d\n",
			__FUNCTION__, MAX_LEN_OF_MULTICAST_FILTER_TABLE));
		return FALSE;
	}

	/* check the rule is in table already or not. */
	if ((pEntry = MulticastFilterTableLookup(pMulticastFilterTable, pGrpId, dev)))
	{
		/* doesn't indicate member mac address. */
		if(pMemberAddr == NULL)
		{
			return FALSE;
		}

		pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

		while (pMemberEntry)
		{
			if (MAC_ADDR_EQUAL(pMemberAddr, pMemberEntry->Addr))
			{
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: already in Members list.\n", __FUNCTION__));
				return FALSE;
			}

			pMemberEntry = pMemberEntry->pNext;
		}
	}

	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	do
	{
		ULONG Now;
		/* the multicast entry already exist but doesn't include the member yet. */
		if (pEntry != NULL && pMemberAddr != NULL)
		{
			InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);
			break;
		}

		/* allocate one MAC entry */
		for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++)
		{
			/* pick up the first available vacancy */
			pEntry = &pMulticastFilterTable->Content[i];
			NdisGetSystemUpTime(&Now);
			if ((pEntry->Valid == TRUE) && (pEntry->type == MCAT_FILTER_DYNAMIC)
				&& ((Now - pEntry->lastTime) > IGMPMAC_TB_ENTRY_AGEOUT_TIME))
			{
				PMULTICAST_FILTER_TABLE_ENTRY pHashEntry;

				HashIdx = MULTICAST_ADDR_HASH_INDEX(pEntry->Addr);
				pHashEntry = pMulticastFilterTable->Hash[HashIdx];

				if ((pEntry->net_dev == pHashEntry->net_dev)
					&& MAC_ADDR_EQUAL(pEntry->Addr, pHashEntry->Addr))
				{
					pMulticastFilterTable->Hash[HashIdx] = pHashEntry->pNext;
					pMulticastFilterTable->Size --;
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 1 - Total= %d\n", pMulticastFilterTable->Size));
				} else
				{
					while (pHashEntry->pNext)
					{
						pPrevEntry = pHashEntry;
						pHashEntry = pHashEntry->pNext;
						if ((pEntry->net_dev == pHashEntry->net_dev)
							&& MAC_ADDR_EQUAL(pEntry->Addr, pHashEntry->Addr))
						{
							pPrevEntry->pNext = pHashEntry->pNext;
							pMulticastFilterTable->Size --;
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
							break;
						}
					}
				}
				pEntry->Valid = FALSE;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
			}

			if (pEntry->Valid == FALSE)
			{
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pEntry->Valid = TRUE;

				COPY_MAC_ADDR(pEntry->Addr, pGrpId);
				pEntry->net_dev = dev;
				NdisGetSystemUpTime(&Now);
				pEntry->lastTime = Now;
				pEntry->type = type;
				initList(&pEntry->MemberList);
				if (pMemberAddr != NULL)
					InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);

				pMulticastFilterTable->Size ++;

				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MulticastFilterTableInsertEntry -IF(%s) allocate entry #%d, Total= %d\n", RTMP_OS_NETDEV_GET_DEVNAME(dev), i, pMulticastFilterTable->Size));
				break;
			}
		}

		/* add this MAC entry into HASH table */
		if (pEntry)
		{
			HashIdx = MULTICAST_ADDR_HASH_INDEX(pGrpId);
			if (pMulticastFilterTable->Hash[HashIdx] == NULL)
			{
				pMulticastFilterTable->Hash[HashIdx] = pEntry;
			} else
			{
				pCurrEntry = pMulticastFilterTable->Hash[HashIdx];
				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;
				pCurrEntry->pNext = pEntry;
			}
		}
	}while(FALSE);

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	return TRUE;
}


/*
    ==========================================================================
    Description:
        Delete a specified client from MAC table
    ==========================================================================
 */
BOOLEAN MulticastFilterTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN UINT8 type,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT8 WlanIndex)
{
	USHORT HashIdx;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry, *pPrevEntry;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if (pMulticastFilterTable == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __FUNCTION__));
		return FALSE;
	}

	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	do
	{
		HashIdx = MULTICAST_ADDR_HASH_INDEX(pGrpId);
		pPrevEntry = pEntry = pMulticastFilterTable->Hash[HashIdx];

		while (pEntry && pEntry->Valid)
		{
			if ((pEntry->net_dev ==  dev)
				&& MAC_ADDR_EQUAL(pEntry->Addr, pGrpId))
				break;
			else
			{
				pPrevEntry = pEntry;
				pEntry = pEntry->pNext;
			}
		}

		/* check the rule is in table already or not. */
		if (pEntry && (pMemberAddr != NULL))
		{
			DeleteIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);
			if (IgmpMemberCnt(&pEntry->MemberList) > 0)
				break;
		}

		if (pEntry)
		{
			if (pEntry == pMulticastFilterTable->Hash[HashIdx])
			{
				pMulticastFilterTable->Hash[HashIdx] = pEntry->pNext;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pMulticastFilterTable->Size --;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 1 - Total= %d\n", pMulticastFilterTable->Size));
			}
			else
			{
				pPrevEntry->pNext = pEntry->pNext;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pMulticastFilterTable->Size --;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
			}
		}
		else
		{
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: the Group doesn't exist.\n", __FUNCTION__));
		}
	} while(FALSE);

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	return TRUE;
}

/*
    ==========================================================================
    Description:
        Look up the MAC address in the IGMP table. Return NULL if not found.
    Return:
        pEntry - pointer to the MAC entry; NULL is not found
    ==========================================================================
*/
PMULTICAST_FILTER_TABLE_ENTRY MulticastFilterTableLookup(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PUCHAR pAddr,
	IN PNET_DEV dev)
{
	ULONG HashIdx, Now;
	PMULTICAST_FILTER_TABLE_ENTRY pEntry = NULL, pPrev = NULL;

	if (pMulticastFilterTable == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __FUNCTION__));
		return NULL;
	}

	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	HashIdx = MULTICAST_ADDR_HASH_INDEX(pAddr);
	pEntry = pPrev = pMulticastFilterTable->Hash[HashIdx];

	while (pEntry && pEntry->Valid)
	{
		if ((pEntry->net_dev ==  dev)
			&& MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			break;
		}
		else
		{
			NdisGetSystemUpTime(&Now);
			if ((pEntry->Valid == TRUE) && (pEntry->type == MCAT_FILTER_DYNAMIC)
				&& RTMP_TIME_AFTER(Now, pEntry->lastTime+IGMPMAC_TB_ENTRY_AGEOUT_TIME))
			{
				/* Remove the aged entry */
				if (pEntry == pMulticastFilterTable->Hash[HashIdx])
				{
					pMulticastFilterTable->Hash[HashIdx] = pEntry->pNext;
					pPrev = pMulticastFilterTable->Hash[HashIdx];
					DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
					NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
					pMulticastFilterTable->Size --;
					pEntry = pPrev;
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
				}
				else
				{
					pPrev->pNext = pEntry->pNext;
					DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
					NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
					pMulticastFilterTable->Size --;
					pEntry = pPrev->pNext;
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
				}
			}
			else
			{
				pPrev = pEntry;
				pEntry = pEntry->pNext;
			}
		}
	}

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	return pEntry;
}

VOID IGMPSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT8 Wcid
	)
{
	INT i;
	INT IpHeaderLen;
	UCHAR GroupType;
	UINT16 numOfGroup;
	UCHAR IgmpVerType;
	PUCHAR pIgmpHeader;
	PUCHAR pGroup;
	UCHAR AuxDataLen;
	UINT16 numOfSources;
	PUCHAR pGroupIpAddr;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;
	UINT8 Type = MCAT_FILTER_DYNAMIC;
	struct wifi_dev *wdev = pEntry->wdev;

	if(isIgmpPkt(pDstMacAddr, pIpHeader))
	{
		IpHeaderLen = (*(pIpHeader + 2) & 0x0f) * 4;
		pIgmpHeader = pIpHeader + 2 + IpHeaderLen;
		IgmpVerType = (UCHAR)(*(pIgmpHeader));
#ifdef MWDS
		if(pEntry && IS_MWDS_OPMODE_AP(pEntry))
			Type |= MCAT_FILTER_MWDS_CLI;	// set info about message on MWDS link
#endif
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMP type=%0x\n", IgmpVerType));

		switch(IgmpVerType)
		{
		case IGMP_V1_MEMBERSHIP_REPORT: /* IGMP version 1 membership report. */
		case IGMP_V2_MEMBERSHIP_REPORT: /* IGMP version 2 membership report. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
				ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryInsert IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
				GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
			
			AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, Type, pSrcMacAddr, wdev->if_dev, Wcid);
			
			break;

		case IGMP_LEAVE_GROUP: /* IGMP version 1 and version 2 leave group. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
				ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryDelete IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
				GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
#ifdef MWDS
			if(pEntry && !IS_MWDS_OPMODE_AP(pEntry)) // skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion
#endif
				AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case IGMP_V3_MEMBERSHIP_REPORT: /* IGMP version 3 membership report. */
			numOfGroup = ntohs(*((UINT16 *)(pIgmpHeader + 6)));
			pGroup = (PUCHAR)(pIgmpHeader + 8);
			for (i=0; i < numOfGroup; i++)
			{
				GroupType = (UCHAR)(*pGroup);
				AuxDataLen = (UCHAR)(*(pGroup + 1));
				numOfSources = ntohs(*((UINT16 *)(pGroup + 2)));
				pGroupIpAddr = (PUCHAR)(pGroup + 4);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMPv3 Type=%d, ADL=%d, numOfSource=%d\n",
								GroupType, AuxDataLen, numOfSources));
				ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
					GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2],
					GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));

				do
				{
					if ((GroupType == MODE_IS_EXCLUDE)
						|| (GroupType == CHANGE_TO_EXCLUDE_MODE)
						|| (GroupType == ALLOW_NEW_SOURCES))
					{
						AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, Type, pSrcMacAddr, wdev->if_dev, Wcid);	
						break;
					}

					if ((GroupType == CHANGE_TO_INCLUDE_MODE)
						|| (GroupType == MODE_IS_INCLUDE)
						|| (GroupType == BLOCK_OLD_SOURCES))
					{
						if(numOfSources == 0){
#ifdef MWDS
								if(pEntry && !IS_MWDS_OPMODE_AP(pEntry)) // skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion
#endif
									AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
						}
						else
							AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, Type, pSrcMacAddr, wdev->if_dev, Wcid);
						break;	
					}
				} while(FALSE);
				pGroup += (8 + (numOfSources * 4) + AuxDataLen);
			}
			break;

		default:
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("unknow IGMP Type=%d\n", IgmpVerType));
			break;
		}
	}

	return;
}

#ifdef MWDS
/* Indicate if Specific Pkt is an IGMP query message*/
BOOLEAN isIGMPquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	INT IpHeaderLen;
	UCHAR IgmpVerType;
	PUCHAR pIgmpHeader;
	BOOLEAN isIGMPquery = FALSE;

	if(isIgmpPkt(pDstMacAddr, pIpHeader))
	{
		IpHeaderLen = (*(pIpHeader + 2) & 0x0f) * 4;
		pIgmpHeader = pIpHeader + 2 + IpHeaderLen;
		IgmpVerType = (UCHAR)(*(pIgmpHeader));

		switch(IgmpVerType)
		{
			case IGMP_MEMBERSHIP_QUERY:
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, ("isIGMPquery-> IGMP Type=0x%x IGMP_MEMBERSHIP_QUERY\n", IgmpVerType));
				isIGMPquery = TRUE;
				break;
		}
	}

	return isIGMPquery;
}
#endif

static BOOLEAN isIgmpMacAddr(
	IN PUCHAR pMacAddr)
{
	if((pMacAddr[0] == 0x01)
		&& (pMacAddr[1] == 0x00)
		&& (pMacAddr[2] == 0x5e))
		return TRUE;
	return FALSE;
}

BOOLEAN isIgmpPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));
	UCHAR IgmpProtocol;

	if(!isIgmpMacAddr(pDstMacAddr))
		return FALSE;

	if(IpProtocol == ETH_P_IP)
	{
		IgmpProtocol = (UCHAR)*(pIpHeader + 11);
		if(IgmpProtocol == IGMP_PROTOCOL_DESCRIPTOR)
				return TRUE;
	}

	return FALSE;
}

static VOID InsertIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr)
{
	PMEMBER_ENTRY pMemberEntry;

	if(pList == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __FUNCTION__));
		return;
	}

	if (pMemberAddr == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: invalid member.\n", __FUNCTION__));
		return;
	}

	if((pMemberEntry = (PMEMBER_ENTRY)AllocaGrpMemberEntry(pMulticastFilterTable)) != NULL)
	{
		NdisZeroMemory(pMemberEntry, sizeof(MEMBER_ENTRY));
		COPY_MAC_ADDR(pMemberEntry->Addr, pMemberAddr);
		insertTailList(pList, (RT_LIST_ENTRY *)pMemberEntry);

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s Member Mac=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
			pMemberEntry->Addr[0], pMemberEntry->Addr[1], pMemberEntry->Addr[2],
			pMemberEntry->Addr[3], pMemberEntry->Addr[4], pMemberEntry->Addr[5]));
	}
	return;
}

static VOID DeleteIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr)
{
	PMEMBER_ENTRY pCurEntry;

	if (pList == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __FUNCTION__));
		return;
	}

	if (pList->pHead == NULL)
	{
		return;
	}

	if (pMemberAddr == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: invalid member.\n", __FUNCTION__));
		return;
	}

	pCurEntry = (PMEMBER_ENTRY)pList->pHead;
	while (pCurEntry)
	{
		PMEMBER_ENTRY pCurEntryNext = pCurEntry->pNext;
		if(MAC_ADDR_EQUAL(pMemberAddr, pCurEntry->Addr))
		{
			delEntryList(pList, (RT_LIST_ENTRY *)pCurEntry);
			FreeGrpMemberEntry(pMulticastFilterTable, pCurEntry);
			break;
		}
		pCurEntry = pCurEntryNext;
	}

	return;
}

static VOID DeleteIgmpMemberList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList)
{
	PMEMBER_ENTRY pCurEntry, pPrvEntry;

	if (pList == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __FUNCTION__));
		return;
	}

	if (pList->pHead == NULL)
	{
		return;
	}

	pPrvEntry = pCurEntry = (PMEMBER_ENTRY)pList->pHead;
	while (pCurEntry)
	{
		delEntryList(pList, (RT_LIST_ENTRY *)pCurEntry);
		pPrvEntry = pCurEntry;
		pCurEntry = pCurEntry->pNext;
		FreeGrpMemberEntry(pMulticastFilterTable, pPrvEntry);
	}

	initList(pList);
	return;
}


UCHAR IgmpMemberCnt(
	IN PLIST_HEADER pList)
{
	if(pList == NULL)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __FUNCTION__));
		return 0;
	}

	return getListSize(pList);
}

VOID IgmpGroupDelMembers(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMemberAddr,
	IN struct wifi_dev *wdev,
	UINT8 Wcid)
{
	INT i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if ((pAd->chipCap.asic_caps & fASIC_CAP_IGMP_SNOOP_OFFLOAD) == fASIC_CAP_IGMP_SNOOP_OFFLOAD)
	{
		return;	
	}

	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++)
	{
		/* pick up the first available vacancy */
		pEntry = &pMulticastFilterTable->Content[i];
		if (pEntry->Valid == TRUE)
		{
			if(pMemberAddr != NULL)
			{
				RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
				DeleteIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);
				RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
			}

			if((pEntry->type == MCAT_FILTER_DYNAMIC)
				&& (IgmpMemberCnt(&pEntry->MemberList) == 0))
				AsicMcastEntryDelete(pAd, pEntry->Addr, wdev->bss_info_argument.ucBssIndex, pMemberAddr, wdev->if_dev, Wcid);
		}
	}
}

INT Set_IgmpSn_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	POS_COOKIE pObj;
	UINT Enable;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
#ifdef CONFIG_AP_SUPPORT
	    wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	Enable = (UINT) simple_strtol(arg, 0, 10);

	wdev->IgmpSnoopEnable = (BOOLEAN)(Enable == 0 ? FALSE : TRUE);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
		("%s:: wdev[%d],OMAC[%d]-%s\n", __FUNCTION__, wdev->wdev_idx,
		wdev->DevInfo.OwnMacIdx,
		Enable == TRUE ? "Enable IGMP Snooping":"Disable IGMP Snooping"));

	if ((pAd->chipCap.asic_caps & fASIC_CAP_IGMP_SNOOP_OFFLOAD) == fASIC_CAP_IGMP_SNOOP_OFFLOAD)
		CmdMcastCloneEnable(pAd, Enable, wdev->DevInfo.OwnMacIdx);

	return TRUE;
}

INT Set_IgmpSn_AddEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[4];
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR GroupId[MAC_ADDR_LEN];
	PUCHAR *pAddr = (PUCHAR *)&Addr;
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	MAC_TABLE_ENTRY *pEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		/* refuse the Member if it's not a MAC address. */
		if((bGroupId == 0) && (strlen(thisChar) != 17))
			continue;

		if(strlen(thisChar) == 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		{
			for (i=0, value = rstrtok(thisChar,":"); value; value = rstrtok(NULL,":"))
			{
				if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
					return FALSE;  /*Invalid */

				AtoH(value, &Addr[i++], 1);
			}

			if(i != 6)
				return FALSE;  /*Invalid */
		}
		else
		{
			for (i=0, value = rstrtok(thisChar,"."); value; value = rstrtok(NULL,"."))
			{
				if((strlen(value) > 0) && (strlen(value) <= 3))
				{
					int ii;
					for(ii=0; ii<strlen(value); ii++)
						if (!isxdigit(*(value + ii)))
							return FALSE;
				}
				else
					return FALSE;  /*Invalid */

				IpAddr[i] = (UCHAR)simple_strtol(value, NULL, 10);
				i++;
			}

			if(i != 4)
				return FALSE;  /*Invalid */

			ConvertMulticastIP2MAC(IpAddr, (PUCHAR *)&pAddr, ETH_P_IP);
		}

		if(bGroupId == 1)
			COPY_MAC_ADDR(GroupId, Addr);

		pEntry = MacTableLookup(pAd, Addr);
		
		/* Group-Id must be a MCAST address. */
		if((bGroupId == 1) && IS_MULTICAST_MAC_ADDR(Addr))
		{
			if (pEntry)
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, NULL, pDev, pEntry->wcid);
			else	
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, NULL, pDev, 0);
		
		}
		/* Group-Member must be a UCAST address. */
		else if ((bGroupId == 0) && !IS_MULTICAST_MAC_ADDR(Addr))
		{

			if (pEntry)
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, Addr, pDev, pEntry->wcid);
			else
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, Addr, pDev, 0);
		}
		else
		{
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s (%2X:%2X:%2X:%2X:%2X:%2X) is not a acceptable address.\n",
				__FUNCTION__, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));
			return FALSE;
		}
		
		bGroupId = 0;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s (%2X:%2X:%2X:%2X:%2X:%2X)\n",
			__FUNCTION__, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));



	}

	return TRUE;
}

INT Set_IgmpSn_DelEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i, memberCnt = 0;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[4];
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR GroupId[MAC_ADDR_LEN];
	PUCHAR *pAddr = (PUCHAR *)&Addr;
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	MAC_TABLE_ENTRY *pEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		/* refuse the Member if it's not a MAC address. */
		if((bGroupId == 0) && (strlen(thisChar) != 17))
			continue;

		if(strlen(thisChar) == 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		{
			for (i=0, value = rstrtok(thisChar,":"); value; value = rstrtok(NULL,":"))
			{
				if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
					return FALSE;  /*Invalid */

				AtoH(value, &Addr[i++], 1);
			}

			if(i != 6)
				return FALSE;  /*Invalid */
		}
		else
		{
			for (i=0, value = rstrtok(thisChar,"."); value; value = rstrtok(NULL,"."))
			{
				if((strlen(value) > 0) && (strlen(value) <= 3))
				{
					int ii;
					for(ii=0; ii<strlen(value); ii++)
						if (!isxdigit(*(value + ii)))
							return FALSE;
				}
				else
					return FALSE;  /*Invalid */

				IpAddr[i] = (UCHAR)simple_strtol(value, NULL, 10);
				i++;
			}

			if(i != 4)
				return FALSE;  /*Invalid */

			ConvertMulticastIP2MAC(IpAddr, (PUCHAR *)&pAddr, ETH_P_IP);
		}

		pEntry = MacTableLookup(pAd, Addr);
		
		if(bGroupId == 1)
			COPY_MAC_ADDR(GroupId, Addr);
		else
			memberCnt++;

		if (memberCnt > 0) {
			if (pEntry)
				AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, 
							Addr, pDev, pEntry->wcid);
			else
				AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, 
							Addr, pDev, 0);
		}

		bGroupId = 0;
	}

	if(memberCnt == 0)
		AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, NULL, pDev, 0);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s (%2X:%2X:%2X:%2X:%2X:%2X)\n",
		__FUNCTION__, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));

	return TRUE;
}

INT Set_IgmpSn_TabDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	IGMPTableDisplay(pAd);
	return TRUE;
}

void rtmp_read_igmp_snoop_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT		i;
	INT		band_idx;
	RTMP_STRING 	*macptr = NULL;

	/*IgmpSnEnable */
	if (RTMPGetKeyParameter("IgmpSnEnable", tmpbuf, 128, buffer, FALSE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr ; macptr = rstrtok(NULL,";"), i++)
		{
			if (i < DBDC_BAND_NUM)
			{
				band_idx = rtmp_band_index_get_by_order(pAd, i);
				pAd->ApCfg.IgmpSnoopEnable[band_idx] = simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, (" Band[%d]-IGMP Snooping Enable=%d\n", band_idx, pAd->ApCfg.IgmpSnoopEnable[band_idx]));
			}
			else
			{
				break;
			}
		}
	}
}

NDIS_STATUS IgmpPktInfoQuery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSrcBufVA,
	IN PNDIS_PACKET pPacket,
	IN struct wifi_dev *wdev,
	OUT INT *pInIgmpGroup,
	OUT PMULTICAST_FILTER_TABLE_ENTRY *ppGroupEntry)
{
	if(IS_MULTICAST_MAC_ADDR(pSrcBufVA))
	{
		BOOLEAN IgmpMldPkt = FALSE;
		PUCHAR pIpHeader = pSrcBufVA + 12;

		if(ntohs(*((UINT16 *)(pIpHeader))) == ETH_P_IPV6)
			IgmpMldPkt = IPv6MulticastFilterExcluded(pSrcBufVA, pIpHeader);
		else
			IgmpMldPkt = isIgmpPkt(pSrcBufVA, pIpHeader);

		if (IgmpMldPkt)
		{
			*ppGroupEntry = NULL;
			*pInIgmpGroup = IGMP_PKT;
		}
		else if ((*ppGroupEntry = MulticastFilterTableLookup(pAd->pMulticastFilterTable, pSrcBufVA,
									wdev->if_dev)) == NULL)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
		else
			*pInIgmpGroup = IGMP_IN_GROUP;
	}
	else if (IS_BROADCAST_MAC_ADDR(pSrcBufVA))
	{
		PUCHAR pDstIpAddr = pSrcBufVA + 30; /* point to Destination of Ip address of IP header. */
		UCHAR GroupMacAddr[6];
		PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;

		ConvertMulticastIP2MAC(pDstIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
		if ((*ppGroupEntry = MulticastFilterTableLookup(pAd->pMulticastFilterTable, pGroupMacAddr,
								wdev->if_dev)) != NULL)
		{
			*pInIgmpGroup = IGMP_IN_GROUP;
		}
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS IgmpPktClone(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN INT IgmpPktInGroup,
	IN PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry,
	IN UCHAR QueIdx,
	IN UINT8 UserPriority,
	IN PNET_DEV pNetDev)
{
	PNDIS_PACKET pSkbClone = NULL;
	PMEMBER_ENTRY pMemberEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	USHORT Aid;
	SST	Sst = SST_ASSOC;
	UCHAR PsMode = PWR_ACTIVE;
	UCHAR Rate;
	//unsigned long IrqFlags;
	INT MacEntryIdx;
	BOOLEAN bContinue;
	PUCHAR pMemberAddr = NULL;

	bContinue = FALSE;

	if ((IgmpPktInGroup == IGMP_IN_GROUP) && (pGroupEntry == NULL))
		return NDIS_STATUS_FAILURE;

	if (IgmpPktInGroup == IGMP_IN_GROUP)
	{
		pMemberEntry = (PMEMBER_ENTRY)pGroupEntry->MemberList.pHead;
		if (pMemberEntry)
		{
			pMemberAddr = pMemberEntry->Addr;
			pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
			bContinue = TRUE;
		}
	}
	else if (IgmpPktInGroup == IGMP_PKT)
	{
		   PUCHAR src_addr = GET_OS_PKT_DATAPTR(pPacket);
                src_addr += 6;

		for(MacEntryIdx=1; MacEntryIdx<MAX_NUMBER_OF_MAC; MacEntryIdx++)
		{
			pMemberAddr = pAd->MacTab.Content[MacEntryIdx].Addr;
			pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
			// TODO: shiang-usw, check get_netdev_from_bssid() here!!
			if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)
				&& get_netdev_from_bssid(pAd, pMacEntry->wdev->wdev_idx) == pNetDev
				&& (!NdisEqualMemory(src_addr, pMacEntry->Addr, MAC_ADDR_LEN)))
			{
				pMemberAddr = pMacEntry->Addr;
				bContinue = TRUE;
				break;
			}
		}
	}
	else
		return NDIS_STATUS_FAILURE;

	/* check all members of the IGMP group. */
	while(bContinue == TRUE)
	{
		if (pMacEntry && (Sst == SST_ASSOC) &&
			(pAd->MacTab.tr_entry[pMacEntry->wcid].PortSecured == WPA_802_1X_PORT_SECURED))
		{
			tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

			OS_PKT_CLONE(pAd, pPacket, pSkbClone, MEM_ALLOC_FLAG);
			if ((pSkbClone)
#ifdef DOT11V_WNM_SUPPORT
				&& (pMacEntry->Beclone == FALSE)
#endif /* DOT11V_WNM_SUPPORT */
			)
			{
				RTMP_SET_PACKET_WCID(pSkbClone, (UCHAR)pMacEntry->wcid);

//copy APSendPacket() unicast check portion.
#ifdef MT_MAC
				if (pAd->chipCap.hif_type == HIF_MT)
				{
						if ((tr_entry->EntryType != ENTRY_CAT_MCAST) && (tr_entry->PsMode == PWR_SAVE))
						{
							if (tr_entry->tx_queue[QID_AC_BE].Number+tr_entry->tx_queue[QID_AC_BK].Number+tr_entry->tx_queue[QID_AC_VI].Number+tr_entry->tx_queue[QID_AC_VO].Number > MAX_PACKETS_IN_PS_QUEUE)
							{
								MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s(%d): (wcid=%u)STA tx_queue full\n", __FUNCTION__, __LINE__,pMacEntry->wcid));
								RELEASE_NDIS_PACKET(pAd, pSkbClone, NDIS_STATUS_FAILURE);
								return NDIS_STATUS_FAILURE;
							}
						}

#if defined(RTMP_MAC) || defined(RLT_MAC)
						/* detect AC Category of tx packets to tune AC0(BE) TX_OP (MAC reg 0x1300) */
						// TODO: shiang-usw, check this for REG access, it should not be here!
						if  ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
							detect_wmm_traffic(pAd, UserPriority, 1);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

						RTMP_SET_PACKET_UP(pSkbClone, UserPriority);


						if (rtmp_enq_req(pAd, pSkbClone, QueIdx, tr_entry, FALSE, NULL) == FALSE)
						{
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s(%d): (wcid=%u)STA rtmp_enq_req() fail!\n", __FUNCTION__, __LINE__,pMacEntry->wcid));
							RELEASE_NDIS_PACKET(pAd, pSkbClone, NDIS_STATUS_FAILURE);
							return NDIS_STATUS_FAILURE;
						}

						if (tr_entry->EntryType == ENTRY_CAT_MCAST) //should not be here!!
						{
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s(%d): (wcid=%u) ENTRY_CAT_MCAST !! ERROR check should not be here!\n", __FUNCTION__, __LINE__,pMacEntry->wcid));

							if (pAd->MacTab.fAnyStationInPsm == 1)
								WLAN_MR_TIM_BCMC_SET(tr_entry->func_tb_idx); /* mark MCAST/BCAST TIM bit */
						}
						else
						{
							if (IS_ENTRY_CLIENT(tr_entry) && (tr_entry->PsMode == PWR_SAVE))
							{
								/* mark corresponding TIM bit in outgoing BEACON frame */
#ifdef UAPSD_SUPPORT
								if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(&pAd->MacTab.Content[tr_entry->wcid], QueIdx))
								{
									/*
										1. the station is UAPSD station;
										2. one of AC is non-UAPSD (legacy) AC;
										3. the destinated AC of the packet is UAPSD AC.
									*/
									/* So we can not set TIM bit due to one of AC is legacy AC */
								}
								else
#endif /* UAPSD_SUPPORT */
								{
									WLAN_MR_TIM_BIT_SET(pAd, tr_entry->func_tb_idx, tr_entry->wcid);
								}
							}
						}

				}
#endif /* MT_MAC */

			}
			else
			{
				if (IgmpPktInGroup == IGMP_IN_GROUP)
				{
					pMemberEntry = pMemberEntry->pNext;
					if (pMemberEntry != NULL)
					{
						pMemberAddr = pMemberEntry->Addr;
						pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
						bContinue = TRUE;
					}
					else
						bContinue = FALSE;
				}
				else if (IgmpPktInGroup == IGMP_PKT)
				{
		   			PUCHAR src_addr = GET_OS_PKT_DATAPTR(pPacket);
                			src_addr += 6;
					for(MacEntryIdx=pMacEntry->wcid + 1; MacEntryIdx<MAX_NUMBER_OF_MAC; MacEntryIdx++)
					{
						pMemberAddr = pAd->MacTab.Content[MacEntryIdx].Addr;
						pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
						if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)
							&& get_netdev_from_bssid(pAd, pMacEntry->wdev->wdev_idx) == pNetDev
							&& (!NdisEqualMemory(src_addr, pMacEntry->Addr, MAC_ADDR_LEN)))
						{
							pMemberAddr = pMacEntry->Addr;
							bContinue = TRUE;
							break;
						}
					}
					if (MacEntryIdx == MAX_NUMBER_OF_MAC)
						bContinue = FALSE;
				}
				else
					bContinue = FALSE;

#ifdef DOT11V_WNM_SUPPORT
				pMacEntry->Beclone = FALSE;
#endif /* DOT11V_WNM_SUPPORT */
				continue;
			}

#ifndef MT_MAC
/*did't queue to AC queue for MT_MAC */
			if (PsMode == PWR_SAVE)
			{
				APInsertPsQueue(pAd, pSkbClone, pMacEntry, QueIdx);
			}
			else
			{
				if (pAd->TxSwQueue[QueIdx].Number >= pAd->TxSwQMaxLen)
				{
#ifdef BLOCK_NET_IF
					StopNetIfQueue(pAd, QueIdx, pSkbClone);
#endif /* BLOCK_NET_IF */
					RELEASE_NDIS_PACKET(pAd, pSkbClone, NDIS_STATUS_FAILURE);
					return NDIS_STATUS_FAILURE;
				}
				else
				{
					RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
					InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QueIdx], PACKET_TO_QUEUE_ENTRY(pSkbClone));
					RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
				}
			}
#endif /* !MT_MAC */

			if (!pAd->chipCap.BATriggerOffload)
			{
				RTMP_BASetup(pAd, tr_entry, UserPriority);
			}
		}

		if (IgmpPktInGroup == IGMP_IN_GROUP)
		{
			pMemberEntry = pMemberEntry->pNext;
			if (pMemberEntry != NULL)
			{
				pMemberAddr = pMemberEntry->Addr;
				pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
				bContinue = TRUE;
			}
			else
				bContinue = FALSE;
		}
		else if (IgmpPktInGroup == IGMP_PKT)
		{
			for(MacEntryIdx=pMacEntry->wcid + 1; MacEntryIdx<MAX_NUMBER_OF_MAC; MacEntryIdx++)
			{
				pMemberAddr = pAd->MacTab.Content[MacEntryIdx].Addr;
				pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
				// TODO: shiang-usw, check for pMacEntry->wdev->wdev_idx here!
				if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)
					&& get_netdev_from_bssid(pAd, pMacEntry->wdev->wdev_idx) == pNetDev)
				{
					pMemberAddr = pMacEntry->Addr;
					bContinue = TRUE;
					break;
				}
			}
			if (MacEntryIdx == MAX_NUMBER_OF_MAC)
				bContinue = FALSE;
		}
		else
			bContinue = FALSE;
	}

	return NDIS_STATUS_SUCCESS;
}

static inline BOOLEAN isMldMacAddr(
	IN PUCHAR pMacAddr)
{
	return ((pMacAddr[0] == 0x33) && (pMacAddr[1] == 0x33)) ? TRUE : FALSE;
}

static inline BOOLEAN IsSupportedMldMsg(
	IN UINT8 MsgType)
{
	BOOLEAN result = FALSE;
	switch(MsgType)
	{
		case MLD_V1_LISTENER_REPORT:
		case MLD_V1_LISTENER_DONE:
		case MLD_V2_LISTERNER_REPORT:
		case MLD_LISTENER_QUERY:
			result = TRUE;
			break;
		default:
			result = FALSE;
			break;
	}

	return result;
}

BOOLEAN isMldPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader,
	OUT UINT8 *pProtoType,
	OUT PUCHAR *pMldHeader)
{
	BOOLEAN result = FALSE;
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));

	if(!isMldMacAddr(pDstMacAddr))
		return FALSE;

	if(IpProtocol != ETH_P_IPV6)
		return FALSE;

	/* skip protocol (2 Bytes). */
	pIpHeader += 2;
	do
	{
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(pIpHeader);
		UINT8 nextProtocol = pIpv6Hdr->nextHdr;
		UINT32 offset = IPV6_HDR_LEN;

		while(nextProtocol != IPV6_NEXT_HEADER_ICMPV6)
		{
			if(IPv6ExtHdrHandle((RT_IPV6_EXT_HDR *)(pIpHeader + offset), &nextProtocol, &offset) == FALSE)
				break;
		}

		if(nextProtocol == IPV6_NEXT_HEADER_ICMPV6)
		{
			PRT_ICMPV6_HDR pICMPv6Hdr = (PRT_ICMPV6_HDR)(pIpHeader + offset);
			if (IsSupportedMldMsg(pICMPv6Hdr->type) == TRUE)
			{
				if (pProtoType != NULL)
					*pProtoType = pICMPv6Hdr->type;
				if (pMldHeader != NULL)
					*pMldHeader = (PUCHAR)pICMPv6Hdr;
				result = TRUE;
			}
		}
	}while(FALSE);

	return result;
}

BOOLEAN IPv6MulticastFilterExcluded(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	BOOLEAN result = FALSE;
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));
	INT idx;
	UINT8 nextProtocol;

	if(!IS_IPV6_MULTICAST_MAC_ADDR(pDstMacAddr))
		return FALSE;

	if(IpProtocol != ETH_P_IPV6)
		return FALSE;

	/* skip protocol (2 Bytes). */
	pIpHeader += 2;
	do
	{
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(pIpHeader);
		UINT32 offset = IPV6_HDR_LEN;

		nextProtocol = pIpv6Hdr->nextHdr;
		while(nextProtocol == IPV6_NEXT_HEADER_HOP_BY_HOP)
		{
			if(IPv6ExtHdrHandle((RT_IPV6_EXT_HDR *)(pIpHeader + offset), &nextProtocol, &offset) == FALSE)
				break;
		}
	} while(FALSE);

	for (idx = 0; idx < IPV6_MULTICAST_FILTER_EXCLUED_SIZE; idx++)
	{
		if (nextProtocol == IPv6MulticastFilterExclued[idx])
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

/*  MLD v1 messages have the following format:
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Type      |     Code      |          Checksum             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Maximum Response Delay    |          Reserved             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	+                                                               +
	|                                                               |
	+                       Multicast Address                       +
	|                                                               |
	+                                                               +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/*	Version 3 Membership Report Message
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|  Type = 143   |    Reserved   |           Checksum            |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Reserved            |  Number of Group Records (M)  |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [1]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [2]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                               .                               |
	.                               .                               .
	|                               .                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [M]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


	where each Group Record has the following internal format:
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|  Record Type  |  Aux Data Len |     Number of Sources (N)     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Multicast Address                       *
    |                                                               |
    *                                                               *
    |                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [1]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-                                                             -+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [2]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-                                                             -+
    .                               .                               .
    .                               .                               .
    .                               .                               .
    +-                                                             -+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [N]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    .                                                               .
    .                         Auxiliary Data                        .
    .                                                               .
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

VOID MLDSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT8 Wcid)
{
	INT i;
	UCHAR GroupType;
	UINT16 numOfGroup;
	PUCHAR pGroup;
	UCHAR AuxDataLen;
	UINT16 numOfSources;
	PUCHAR pGroupIpAddr;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;

	UINT8 MldType;
	PUCHAR pMldHeader;
	UINT8 Type = MCAT_FILTER_DYNAMIC;
	struct wifi_dev *wdev = pEntry->wdev;

	if(isMldPkt(pDstMacAddr, pIpHeader, &MldType, &pMldHeader) == TRUE)
	{
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MLD type=%0x\n", MldType));
#ifdef MWDS
		if(pEntry && IS_MWDS_OPMODE_AP(pEntry))
			Type |= MCAT_FILTER_MWDS_CLI;	// set info about message on MWDS link
#endif
		switch(MldType)
		{
			case MLD_V1_LISTENER_REPORT:
				/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
				pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
				ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryInsert Group Id=%02x:%02x:%02x:%02x:%02x:%02x\n",
						GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
				AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, Type, pSrcMacAddr, wdev->if_dev, Wcid);
				break;

			case MLD_V1_LISTENER_DONE:
				/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
				pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
				ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryDelete Group Id=%02x:%02x:%02x:%02x:%02x:%02x\n",
						GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
#ifdef MWDS
				if(pEntry && !IS_MWDS_OPMODE_AP(pEntry)) // skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion
#endif
					AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
			
				break;

			case MLD_V2_LISTERNER_REPORT: /* IGMP version 3 membership report. */
				numOfGroup = ntohs(*((UINT16 *)(pMldHeader + 6)));
				pGroup = (PUCHAR)(pMldHeader + 8);
				for (i=0; i < numOfGroup; i++)
				{
					GroupType = (UCHAR)(*pGroup);
					AuxDataLen = (UCHAR)(*(pGroup + 1));
					numOfSources = ntohs(*((UINT16 *)(pGroup + 2)));
					pGroupIpAddr = (PUCHAR)(pGroup + 4);
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MLDv2 Type=%d, ADL=%d, numOfSource=%d\n",
									GroupType, AuxDataLen, numOfSources));
					ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MLD Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
							GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2],
							GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));

					do
					{
						if ((GroupType == MODE_IS_EXCLUDE)
							|| (GroupType == CHANGE_TO_EXCLUDE_MODE)
							|| (GroupType == ALLOW_NEW_SOURCES))
						{
							AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, Type, pSrcMacAddr, wdev->if_dev, Wcid);
							break;	
						}


						if ((GroupType == CHANGE_TO_INCLUDE_MODE)
							|| (GroupType == MODE_IS_INCLUDE)
							|| (GroupType == BLOCK_OLD_SOURCES))
						{
							if(numOfSources == 0){
#ifdef MWDS
								if(pEntry && !IS_MWDS_OPMODE_AP(pEntry)) // skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion
#endif
									AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
							}
							else
									AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, Type, pSrcMacAddr, wdev->if_dev, Wcid);
							break;
						}
					} while(FALSE);
					/* skip 4 Bytes (Record Type, Aux Data Len, Number of Sources) + a IPv6 address. */
					pGroup += (4 + IPV6_ADDR_LEN + (numOfSources * 16) + AuxDataLen);
				}
				break;

			default:
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("unknow MLD Type=%d\n", MldType));
				break;
		}
	}

	return;
}
#ifdef MWDS
/* Indicate if Specific Pkt is an MLD query message*/
BOOLEAN isMLDquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	UINT8 MldType = 0;
	PUCHAR pMldHeader;
	BOOLEAN isMLDquery = FALSE;

	if(isMldPkt(pDstMacAddr, pIpHeader, &MldType, &pMldHeader) == TRUE)
	{
		switch(MldType)
		{
			case MLD_LISTENER_QUERY:
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, ("isMLDquery-> MLD type=0x%x MLD_LISTENER_QUERY\n", MldType));
				isMLDquery = TRUE;
				break;

		}
	}

	return isMLDquery;
}

UCHAR	TYPEIPV4[] = {0x08, 0x00};

/* Send an IGMP query message on particular AP interface*/
void send_igmpv3_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry)
{
	UCHAR ALL_HOST_ADDR[MAC_ADDR_LEN] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01}; // as per IGMP spec
	UCHAR Header802_3[14]={0};
	UCHAR               CustomPayload[36];

	UCHAR			IpHdr[24]={0x46,0x00, // version(4bit), hdr lenght(4bit), tos
							   0x00,0x24, // total ip datagram length
							   0x00,0x01, // identification (random)
							   0x00,0x00, // flag & fragmentation
							   0x01,0x02, // TTL, Protocol type (as per igmp spec)
							   0x44,0xD2, // hdr checksum (considered 0 for calculation and computed manually for this msg)
							   0x00,0x00, // Source IP (0.0.0.0)
							   0x00,0x00, // Source IP
							   0xE0,0x00, // Dest IP (224.0.0.1 - All Host addr as per IGMP spec)
							   0x00,0x01, // Dest IP
							   0x94,0x04, // Router Alert (as per IPv4 Router alert spec & IGMP spec)
							   0x00,0x00}; // Router Alert

	UCHAR               IgmpGenQuery[12]={0x11,0x0A, // type(Mmbrship Query), Max Rsp Code (10 i.e 1 sec)
		                                  0xEE,0xF5, // chksum (considered 0 for calculation and computed manually for this msg)
		                                  0x00,0x00, // Grp addrss (general query)
		                                  0x00,0x00, // Grp addr
										  0x00,0x00, // rsv, S, QRC	QQIC (other routers to use defaults)
										  0x00,0x00}; // No of Sources	(general query)

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("send_igmpv3_gen_query_pkt---->\n"));

	NdisZeroMemory(Header802_3,sizeof(UCHAR)*14);

	MAKE_802_3_HEADER(Header802_3, ALL_HOST_ADDR, &pMacEntry->wdev->if_addr[0], TYPEIPV4);

	// Using fixed payload due to checksum calculation required using one's complement
	NdisCopyMemory(&CustomPayload[0],IpHdr,24);
	NdisCopyMemory(&CustomPayload[24],IgmpGenQuery,12);

	// Copy frame to Tx ring

	// Pkt 1
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("send_igmpv3_gen_query_pkt: wdev_idx:0x%x => Send IGMPv3 Gen Query Pkt 1\n",pMacEntry->wdev->wdev_idx));
	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 36, FALSE);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("<----- send_igmpv3_gen_query_pkt\n"));
}

UCHAR	TYPEIPV6[] = {0x86, 0xDD};

/* Send a MLD query message on particular AP interface*/
void send_mldv2_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry)
{
	UCHAR ALL_HOST_ADDR[MAC_ADDR_LEN] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01}; // as per MLD spec
	UCHAR Header802_3[14]={0};
	UCHAR               CustomPayload[76];
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pMacEntry->wdev->func_idx];

	UCHAR               Ipv6Hdr[40]={0x60,0x00, // version(4bit), Tclass(8bit), Flow label (4bit+)
		                           	0x00,0x00, // Flow Label (16 bit)
		                           	0x00,0x24, // Payload length	(Ipv6 Router alert + MLD Gen Query)
		                           	0x00,0x01, // Next hdr (Hop y Hop Options for Router Alert), Hop Limit
									0x00,0x00, // ipv6 src addr	(Ipv6 link-local addr)		<-------------- to update from preformed address
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0xFF,0x02, // ipv6 dst addr	(All node Ipv6 Multcast addr, as per MLD spec)
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
								   	0x00,0x01};

	UCHAR			Ipv6RouterAlert[8]={0x3A,0x00, // NxtHdr (ICMPv6-MLD), Hdr Ext len
									   0x05,0x02, // Option Type - Router Alert, Length
									   0x00,0x00, // Value - MLD
									   0x01,0x00}; // Padding - Pad2

	UCHAR               MldGenQuery[28]={0x82,0x00, // type(MLD Mmbrship Query), Code
		                                 0x00,0x00, // chksum						<------- to update from precomputed checksum for each mbss
		                                 0x03,0xE8, // max rsp code (1000 ms i.e. 1 second)
		                                 0x00,0x00, // rsvd
										 0x00,0x00, // ipv6 grp addr (general query)
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00, // rsv, S, QRC	QQIC	(other routers to use defaults)
										 0x00,0x00}; // No of Sources	(general query)

	// Get the Link Local Src Addr for this interface
	NdisCopyMemory(&Ipv6Hdr[8],&pMbss->ipv6LinkLocalSrcAddr[0],16);

	// Get Checksum
	MldGenQuery[2] = (pMbss->MldQryChkSum >> 8);
	MldGenQuery[3] = (pMbss->MldQryChkSum & 0xff);

	// Form the pkt to be sent
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("send_mldv2_gen_query_pkt---->\n"));

	NdisZeroMemory(Header802_3,sizeof(UCHAR)*14);

	MAKE_802_3_HEADER(Header802_3, ALL_HOST_ADDR, &pMacEntry->wdev->if_addr[0], TYPEIPV6);

	// Using fixed payload due to checksum calculation required using one's complement
	NdisZeroMemory(CustomPayload,76);
	NdisCopyMemory(&CustomPayload[0],Ipv6Hdr,40);
	NdisCopyMemory(&CustomPayload[40],Ipv6RouterAlert,8);
	NdisCopyMemory(&CustomPayload[48],MldGenQuery,28);

	// Copy frame to Tx ring

	// Pkt 1
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("send_igmpv3_gen_query_pkt: wdev_idx:0x%x => Send IGMPv3 Gen Query Pkt 1\n",pMacEntry->wdev->wdev_idx));
	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 76, FALSE);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("<----- send_mldv2_gen_query_pkt\n"));
}

/* For specifed MBSS, compute & store IPv6 format checksum for MLD query message to be sent on that interface*/
void calc_mldv2_gen_query_chksum(
	IN	PRTMP_ADAPTER	pAd,
	IN  BSS_STRUCT *pMbss)
{
	UCHAR               CustomPayload[68];
	UINT32 sum = 0, operand = 0, exCarry = 0;
	UINT16 chksum = 0,dataLen = 0;
	UINT8 ctr = 0;

	UCHAR ipv6LinkLocalSrcAddr[16]={0xFE,0x80, // ipv6 src addr	(Ipv6 link-local addr)		<------------------to form
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0x00,
									0x00,0xFF,
									0xFE,0x00,
									0x00,0x00};

	UCHAR               Ipv6PsuedoHdr[40]={ 0x00,0x00, // ipv6 src addr (Ipv6 link-local addr)		<------------------to form
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0xFF,0x02, // ipv6 dst addr (All node Ipv6 Multcast addr as per spec)
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x00,
											0x00,0x01,
											0x00,0x00, // Next Hdr (MLD) Pkt Len
				                           	0x00,0x1C, // Next Hdr (MLD) Pkt Len
				                           	0x00,0x00, // Zero
				                           	0x00,0x3A}; // Zero, Next hdr (ICMPv6 - MLD)

	UCHAR               MldGenQuery[28]={0x82,0x00, // type(MLD Mmbrship Query), Code
		                                 0x00,0x00, // chksum		<--------- calculate
		                                 0x03,0xE8, // max rsp code (1000 ms i.e. 1 second)
		                                 0x00,0x00, // rsvd
										 0x00,0x00, // ipv6 grp addr (general query)
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00,
										 0x00,0x00, // rsv, S, QRC	QQIC	(other routers to use defaults)
										 0x00,0x00}; // No of Sources	(general query)

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("calc_mldv2_gen_query_chksum --> wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x IntfAddr[%02x-%02x-%02x-%02x-%02x-%02x] \n", 
	//	pMbss->wdev.wdev_idx,pMbss->wdev.wdev_type,pMbss->wdev.func_idx,
	//	pMbss->wdev.if_addr[0],pMbss->wdev.if_addr[1],pMbss->wdev.if_addr[2],
	//	pMbss->wdev.if_addr[3],pMbss->wdev.if_addr[4],pMbss->wdev.if_addr[5]));

	// Form the Link Local Src Addr for this interface in EUI-64 format, as per spec
	ipv6LinkLocalSrcAddr[8] = pMbss->wdev.if_addr[0];
	ipv6LinkLocalSrcAddr[9] = pMbss->wdev.if_addr[1];
	ipv6LinkLocalSrcAddr[10] = pMbss->wdev.if_addr[2];
	ipv6LinkLocalSrcAddr[13] = pMbss->wdev.if_addr[3];
	ipv6LinkLocalSrcAddr[14] = pMbss->wdev.if_addr[4];
	ipv6LinkLocalSrcAddr[15] = pMbss->wdev.if_addr[5];

	ipv6LinkLocalSrcAddr[8] = ipv6LinkLocalSrcAddr[8] ^ 0x02; // togle universal/local bit

	NdisCopyMemory(&pMbss->ipv6LinkLocalSrcAddr[0],&ipv6LinkLocalSrcAddr[0],16);
	NdisCopyMemory(&Ipv6PsuedoHdr[0],&ipv6LinkLocalSrcAddr[0],16);

	// Rakesh: A print is required here to avoid crash during reboot. Please don't remove !!!
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("calc_mldv2_gen_query_chksum -->"));

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("calc_mldv2_gen_query_chksum --> LinkLocal Ipv6Addr[%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x] \n", 
	//	pMbss->ipv6LinkLocalSrcAddr[0],pMbss->ipv6LinkLocalSrcAddr[1],pMbss->ipv6LinkLocalSrcAddr[2],
	//	pMbss->ipv6LinkLocalSrcAddr[3],pMbss->ipv6LinkLocalSrcAddr[4],pMbss->ipv6LinkLocalSrcAddr[5],
	//	pMbss->ipv6LinkLocalSrcAddr[6],pMbss->ipv6LinkLocalSrcAddr[7],pMbss->ipv6LinkLocalSrcAddr[8],
	//	pMbss->ipv6LinkLocalSrcAddr[9],pMbss->ipv6LinkLocalSrcAddr[10],pMbss->ipv6LinkLocalSrcAddr[11],
	//	pMbss->ipv6LinkLocalSrcAddr[12],pMbss->ipv6LinkLocalSrcAddr[13],pMbss->ipv6LinkLocalSrcAddr[14],
	//	pMbss->ipv6LinkLocalSrcAddr[15]));

	// Calculate Checksum
	NdisZeroMemory(CustomPayload,68);
	NdisCopyMemory(&CustomPayload[0],Ipv6PsuedoHdr,40);
	NdisCopyMemory(&CustomPayload[40],MldGenQuery,28);
	dataLen = 68; // total size of Pseudo Hdr & MLD

	// Note: current logic assumes even data len, as per IP checksum format
	sum = 0;
	operand = 0;
	for(ctr = 0;ctr < dataLen; ctr += 2)
	{
		operand = (UINT32)(CustomPayload[ctr] << 8);
		operand |= (UINT32)(CustomPayload[ctr+1]);

		sum += operand;
	}
	exCarry = sum >> 16;
	sum = sum & 0x0000ffff;

	while(exCarry != 0){
		sum += exCarry;
		exCarry = sum >> 16;
		sum = sum & 0x0000ffff;
	}
	chksum = (UINT16)sum;
	chksum = chksum ^ 0xFFFF;
	if(chksum == 0)
		chksum = 0xFFFF;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,("ChkSum Done: chksum: %04x \n", chksum));

	NdisCopyMemory(&pMbss->MldQryChkSum,&chksum,2);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("<----- calc_mldv2_gen_query_chksum\n"));
}
#endif

#endif /* IGMP_SNOOP_SUPPORT */


