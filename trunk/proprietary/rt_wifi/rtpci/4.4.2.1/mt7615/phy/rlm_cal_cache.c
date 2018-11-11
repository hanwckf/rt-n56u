/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rlm_cal_cache.c
*/

//#include "rtmp_type.h"
//#include "os/rt_linux.h"
//#include "common/mt_os_util.h"

#include "rt_config.h"
#include "phy/rlm_cal_cache.h"

INT rlmCalCacheDone(VOID *rlmCalCache)
{
	return RLM_CAL_CACHE_IS_DONE(rlmCalCache);
}

INT rlmCalCacheInit(VOID* pAd, VOID **ppRlmCalCache)
{
	if (os_alloc_mem(pAd, (UCHAR **)ppRlmCalCache, sizeof(RLM_CAL_CACHE)) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("fail to alloca rlmCalCache size=%zu\n", sizeof(RLM_CAL_CACHE)));
		return FALSE;
	}
	os_zero_mem(*ppRlmCalCache, sizeof(RLM_CAL_CACHE));

	return TRUE;
}

INT rlmCalCacheDeinit(VOID **ppRlmCalCache)
{
	if (*ppRlmCalCache == NULL)
		return FALSE;

	os_free_mem(*ppRlmCalCache);
	*ppRlmCalCache = NULL;
	return TRUE;
}

INT RlmCalCacheTxLpfInfo(VOID *rlmCalCtrl, UINT8* Data, UINT32 Length)
{
	P_TXLPF_CAL_INFO_T ptxLpfCalInfo;

	if (Length != sizeof(TXLPF_CAL_INFO_T))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("P_TXLPF_CAL_INFO_T: length mismatch=%d\n", Length));

		return FALSE;
	}
	/*hex_dump("P_TXLPF_CAL_INFO", Data, Length);*/

	if (rlmCalCtrl == NULL)
		return FALSE;
	ptxLpfCalInfo = (P_TXLPF_CAL_INFO_T)Data;
#ifdef RT_BIG_ENDIAN
	ptxLpfCalInfo->u2BitMap = le2cpu16(ptxLpfCalInfo->u2BitMap);
	RTMPEndianChange((UCHAR *)ptxLpfCalInfo->au4Data, sizeof(ptxLpfCalInfo->au4Data));
#endif
	os_move_mem((PVOID)&RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCtrl), Data, Length);
	RLM_CAL_CACHE_TXLPF_CAL_DONE(rlmCalCtrl);

	return TRUE;
}

INT RlmCalCacheTxIqInfo(VOID *rlmCalCtrl, UINT8* Data, UINT32 Length)
{
	P_TXIQ_CAL_INFO_T ptxIqCalInfo;

	if (Length != sizeof(TXIQ_CAL_INFO_T))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("P_TXIQ_CAL_INFO_T: length mismatch=%d\n", Length));

		return FALSE;
	}
	/*hex_dump("P_TXIQ_CAL_INFO", Data, Length);*/

	if (rlmCalCtrl == NULL)
		return FALSE;
	ptxIqCalInfo = (P_TXIQ_CAL_INFO_T)Data;
#ifdef RT_BIG_ENDIAN
	ptxIqCalInfo->u2BitMap = le2cpu16(ptxIqCalInfo->u2BitMap);
	RTMPEndianChange((UCHAR *)ptxIqCalInfo->au4Data, sizeof(ptxIqCalInfo->au4Data));
#endif
	os_move_mem((PVOID)&RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCtrl), Data, Length);
	RLM_CAL_CACHE_TXIQ_CAL_DONE(rlmCalCtrl);

	return TRUE;
}

INT RlmCalCacheTxDcInfo(VOID *rlmCalCtrl, UINT8* Data, UINT32 Length)
{
	P_TXDC_CAL_INFO_T ptxDcCalInfo;

	if (Length != sizeof(TXDC_CAL_INFO_T))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("P_TXDC_CAL_INFO_T: length mismatch=%d\n", Length));

		return FALSE;
	}
	/*hex_dump("P_TXDC_CAL_INFO", Data, Length);*/

	if (rlmCalCtrl == NULL)
		return TRUE;
	ptxDcCalInfo = (P_TXDC_CAL_INFO_T)Data;
#ifdef RT_BIG_ENDIAN
	ptxDcCalInfo->u2BitMap = le2cpu16(ptxDcCalInfo->u2BitMap);
	RTMPEndianChange((UCHAR *)ptxDcCalInfo->au4Data, sizeof(ptxDcCalInfo->au4Data));
#endif
	os_move_mem((PVOID)&RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCtrl), Data, Length);
	RLM_CAL_CACHE_TXDC_CAL_DONE(rlmCalCtrl);

	return TRUE;
}

INT RlmCalCacheRxFiInfo(VOID *rlmCalCtrl, UINT8* Data, UINT32 Length)
{
	P_RXFI_CAL_INFO_T rxfi_event;

	if (Data == NULL)
		return FALSE;

	if (Length != sizeof(RXFI_CAL_INFO_T))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("P_RXFI_CAL_INFO_T: length mismatch=%d\n", Length));

		return FALSE;
	}

	rxfi_event = (P_RXFI_CAL_INFO_T)Data;

	/*hex_dump("P_RXFI_CAL_INFO", Data, Length);*/

	if (rlmCalCtrl == NULL)
		return FALSE;
	/*os_move_mem((PVOID)RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCtrl).au4Data,
	rxfi_event->au4Data, sizeof(rxfi_event->au4Data));*/
#ifdef RT_BIG_ENDIAN
	rxfi_event->u2BitMap = le2cpu16(rxfi_event->u2BitMap);
	RTMPEndianChange((UCHAR *)rxfi_event->au4Data, sizeof(rxfi_event->au4Data));
#endif
	os_move_mem((PVOID)&RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCtrl),
	Data, Length);    
	RLM_CAL_CACHE_RXFI_CAL_DONE(rlmCalCtrl);

	return TRUE;

}

INT RlmCalCacheRxFdInfo(VOID *rlmCalCtrl, UINT8* Data, UINT32 Length)
{
	P_RXFD_CAL_INFO_T rxfd_event;
	UINT32 ch_group_id;

	if (Data == NULL)
		return FALSE;

	if (Length != sizeof(RXFD_CAL_INFO_T))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("P_RXFD_CAL_INFO_T: length mismatch=%d\n", Length));

		return FALSE;
	}

	rxfd_event = (P_RXFD_CAL_INFO_T)Data;
#ifdef RT_BIG_ENDIAN
	rxfd_event->u4ChGroupId = le2cpu32(rxfd_event->u4ChGroupId);
	rxfd_event->u2BitMap = le2cpu16(rxfd_event->u2BitMap);
	RTMPEndianChange((UCHAR *)rxfd_event->au4Data, sizeof(rxfd_event->au4Data));
#endif
	ch_group_id = rxfd_event->u4ChGroupId;

	/*hex_dump("P_RXFD_CAL_INFO", Data, Length);*/

	if (rlmCalCtrl == NULL)
		return FALSE;

	if (rxfd_event->u4ChGroupId >= CHANNEL_GROUP_NUM)
	{
		return FALSE;
	}
	/*os_move_mem((PVOID)(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCtrl, rxfd_event->u4ChGroupId).au4Data),
	rxfd_event->au4Data, sizeof(rxfd_event->au4Data));*/
	os_move_mem((PVOID)(&RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCtrl, rxfd_event->u4ChGroupId)),
	Data, Length);

	RLM_CAL_CACHE_RXFD_CAL_DONE(rlmCalCtrl, rxfd_event->u4ChGroupId);

	return TRUE;

}

VOID rlmCalCacheStatus(VOID *rlmCalCache)
{
	UINT idx;
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("TxLPF Cal Done::(%c)\n",
	RLM_CAL_CACHE_IS_TXLPF_CAL_DONE(rlmCalCache) ? 'T':'F'));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("TxIQ Cal Done::(%c)\n",
	RLM_CAL_CACHE_IS_TXIQ_CAL_DONE(rlmCalCache) ? 'T':'F'));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("TxDC Cal Done::(%c)\n",
	RLM_CAL_CACHE_IS_TXDC_CAL_DONE(rlmCalCache) ? 'T':'F'));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("RxFI Cal Done::(%c)\n",
	RLM_CAL_CACHE_IS_RXFI_CAL_DONE(rlmCalCache) ? 'T':'F'));

	for (idx=0; idx<9; idx++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("RxFD(%d) Cal Done::(%c)\n", idx,
		RLM_CAL_CACHE_IS_RXFD_CAL_DONE(rlmCalCache, idx) ? 'T':'F'));
	}

	return;
}

VOID rlmCalCacheDump(VOID *rlmCalCache)
{
	UINT idx;
	
	if (RLM_CAL_CACHE_IS_TXLPF_CAL_DONE(rlmCalCache))
		hex_dump("TxLpf", (UCHAR *)(RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data),
		sizeof(RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data));

	if (RLM_CAL_CACHE_IS_TXIQ_CAL_DONE(rlmCalCache))
		hex_dump("TxIq", (UCHAR *)(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data),
		sizeof(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data));

	if (RLM_CAL_CACHE_IS_TXDC_CAL_DONE(rlmCalCache))
		hex_dump("TxDc", (UCHAR *)(RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data),
		sizeof(RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data));

	if (RLM_CAL_CACHE_IS_RXFI_CAL_DONE(rlmCalCache))
		hex_dump("RxFi", (UCHAR *)(RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data),
		sizeof(RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data));

	for (idx=0; idx<9; idx++)
	{
		if (RLM_CAL_CACHE_IS_RXFD_CAL_DONE(rlmCalCache, idx))
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("RxFD(%d)\n", idx));
			hex_dump("RxFd", (UCHAR *)(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, idx).au4Data),
			sizeof(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, idx).au4Data));
		}
	}

	return;
}


INT TxLpfCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_TXLPF_CAL_INFO_T pTxLpfCalInfo;

	if (!RLM_CAL_CACHE_IS_TXLPF_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pTxLpfCalInfo, sizeof(TXLPF_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
	{
		return 0;
	}
	*pptr = pTxLpfCalInfo;
	os_zero_mem(pTxLpfCalInfo, sizeof(TXLPF_CAL_INFO_T));
	/*os_move_mem(pTxLpfCalInfo->au4Data, RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data,
	sizeof(RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pTxLpfCalInfo, &RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache),
	sizeof(TXLPF_CAL_INFO_T));

	return sizeof(TXLPF_CAL_INFO_T);
}

INT TxIqCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_TXIQ_CAL_INFO_T pTxIqCalInfo;

	if (!RLM_CAL_CACHE_IS_TXIQ_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pTxIqCalInfo, sizeof(TXIQ_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
	{
		return 0;
	}
	*pptr = pTxIqCalInfo;
	os_zero_mem(pTxIqCalInfo, sizeof(TXIQ_CAL_INFO_T));
	/*os_move_mem(pTxIqCalInfo->au4Data, &(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data),
	sizeof(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pTxIqCalInfo, &RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache),
	sizeof(TXIQ_CAL_INFO_T));

	return sizeof(TXIQ_CAL_INFO_T);
}

INT TxDcCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_TXDC_CAL_INFO_T pTxDcCalInfo;

	if (!RLM_CAL_CACHE_IS_TXDC_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pTxDcCalInfo, sizeof(TXDC_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
	{
		return 0;
	}
	*pptr = pTxDcCalInfo;
	os_zero_mem(pTxDcCalInfo, sizeof(TXDC_CAL_INFO_T));
	/*os_move_mem(pTxDcCalInfo->au4Data, RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data,
	sizeof(RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pTxDcCalInfo, &RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache),
	sizeof(TXDC_CAL_INFO_T));

	return sizeof(TXDC_CAL_INFO_T);
}

INT RxFiCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_RXFI_CAL_INFO_T pRxFiCalInfo = NULL;

	if (!RLM_CAL_CACHE_IS_RXFI_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pRxFiCalInfo, sizeof(RXFI_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
	{
		return 0;
	}

	*pptr = pRxFiCalInfo;
	os_zero_mem(pRxFiCalInfo, sizeof(RXFI_CAL_INFO_T));
	/*os_move_mem(pRxFiCalInfo->au4Data, RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data,
	sizeof(RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pRxFiCalInfo, &RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache),
	sizeof(RXFI_CAL_INFO_T));

	return sizeof(RXFI_CAL_INFO_T);
}

INT RxFdCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr, UINT32 chGroup)
{
	P_RXFD_CAL_INFO_T pRxFdCalInfo;

	if (!RLM_CAL_CACHE_IS_RXFD_CAL_DONE(rlmCalCache, chGroup))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pRxFdCalInfo, sizeof(RXFD_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
	{
        	return 0;
	}
	*pptr = pRxFdCalInfo;
	os_zero_mem(pRxFdCalInfo, sizeof(RXFD_CAL_INFO_T));
	pRxFdCalInfo->u4ChGroupId = chGroup;
	/*os_move_mem(pRxFdCalInfo->au4Data, RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, chGroup).au4Data,
	sizeof(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, chGroup).au4Data));*/
	os_move_mem(pRxFdCalInfo, &RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, chGroup),
	sizeof(RXFD_CAL_INFO_T));

	return sizeof(RXFD_CAL_INFO_T);
}

INT RlmPorCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_RLM_POR_CAL_INFO_T pRlmPorCalInfo = NULL;

	if (os_alloc_mem(pAd, (UCHAR **)&pRlmPorCalInfo, sizeof(RLM_POR_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
	{
		return 0;
	}

	*pptr = pRlmPorCalInfo;
	os_zero_mem(pRlmPorCalInfo, sizeof(RLM_POR_CAL_INFO_T));
	pRlmPorCalInfo->ucRlmPorCal = 1;

	return sizeof(RLM_POR_CAL_INFO_T);
}

