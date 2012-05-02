
#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY *pNext;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _LIST_HEADR
{
	PLIST_ENTRY pHead;
	PLIST_ENTRY pTail;
	UCHAR size;
} LIST_HEADER, *PLIST_HEADER;

static inline VOID initList(
	IN PLIST_HEADER pList)
{
	pList->pHead = pList->pTail = NULL;
	pList->size = 0;
	return;
}

static inline VOID insertTailList(
	IN PLIST_HEADER pList,
	IN PLIST_ENTRY pEntry)
{
	pEntry->pNext = NULL;
	if (pList->pTail)
		pList->pTail->pNext = pEntry;
	else
		pList->pHead = pEntry;
	pList->pTail = pEntry;
	pList->size++;

	return;
}

static inline PLIST_ENTRY removeHeadList(
	IN PLIST_HEADER pList)
{
	PLIST_ENTRY pNext;
	PLIST_ENTRY pEntry;

	pEntry = pList->pHead;
	if (pList->pHead != NULL)
	{
		pNext = pList->pHead->pNext;
		pList->pHead = pNext;
		if (pNext == NULL)
			pList->pTail = NULL;
		pList->size--;
	}
	return pEntry;
}

static inline int getListSize(
	IN PLIST_HEADER pList)
{
	return pList->size;
}

static inline PLIST_ENTRY delEntryList(
	IN PLIST_HEADER pList,
	IN PLIST_ENTRY pEntry)
{
	PLIST_ENTRY pCurEntry;
	PLIST_ENTRY pPrvEntry;

	if(pList->pHead == NULL)
		return NULL;

	if(pEntry == pList->pHead)
	{
		pCurEntry = pList->pHead;
		pList->pHead = pCurEntry->pNext;

		if(pList->pHead == NULL)
			pList->pTail = NULL;

		pList->size--;
		return pCurEntry;
	}

	pPrvEntry = pList->pHead;
	pCurEntry = pPrvEntry->pNext;
	while(pCurEntry != NULL)
	{
		if (pEntry == pCurEntry)
		{
			pPrvEntry->pNext = pCurEntry->pNext;

			if(pEntry == pList->pTail)
				pList->pTail = pPrvEntry;

			pList->size--;
			break;
		}
		pPrvEntry = pCurEntry;
		pCurEntry = pPrvEntry->pNext;
	}

	return pCurEntry;
}

#endif // ___LINK_LIST_H__ //

