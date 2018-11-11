/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include "rt_config.h"
#include "hdev/hdev.h"

/*Omac controller*/

static INT32 GetFirstAvailableOmacIdx(HD_CFG *pHdCfg, BOOLEAN NeedBss)
{
	UINT32 Index;
	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;

	if (NeedBss)
	{
		for (Index = 0; Index < pHdCfg->chipCap.BssNums; Index++)
		{
			if ((pHwResource->OmacBssCtl.OmacBitMap & (1 << Index)) == 0)
			{
				pHwResource->OmacBssCtl.OmacBitMap |= (1 << Index);
				return Index;
			}
		}
	}
	else
	{
		for (Index = pHdCfg->chipCap.BssNums; Index < pHdCfg->chipCap.OmacNums; Index++)
		{
			if ((pHwResource->OmacBssCtl.OmacBitMap & (1 << Index)) == 0)
			{
				pHwResource->OmacBssCtl.OmacBitMap |= (1 << Index);
				return Index;
			}
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: OmacIndex is not available\n",
					__FUNCTION__));

	return -1;
}

static INT32 GetFirstAvailableRepeaterOmacIdx(HD_CFG *pHdCfg)
{
 	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;
    UCHAR i;

    for (i = 0; i < pHdCfg->chipCap.MaxRepeaterNum; i++)
    {
        if ((pHwResource->OmacBssCtl.RepeaterBitMap & (1 << i)) == 0)
        {
            pHwResource->OmacBssCtl.RepeaterBitMap |= (1 << i);
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                    ("%s: found used OmacIndex:0x%x\n",
                    __FUNCTION__, pHdCfg->chipCap.RepeaterStartIdx + i));
            return pHdCfg->chipCap.RepeaterStartIdx + i;
        }
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: OmacIndex is not available\n",
                    __FUNCTION__));

    return -1;
}

VOID ReleaseOmacIdx(HD_CFG *pHdCfg, UINT32 OmacType, UINT32 Idx)
{
 	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;

    switch (OmacType)
    {
        case WDEV_TYPE_AP:
            if (Idx == 0)
            {
                pHwResource->OmacBssCtl.OmacBitMap &= ~(1 << 0);
                return;
            }
            else
            {
                pHwResource->OmacBssCtl.HwMbssBitMap &= ~(1 << (Idx));
                return;
            }
        break;
        case WDEV_TYPE_STA:
        case WDEV_TYPE_ADHOC:
        case WDEV_TYPE_GO:
        case WDEV_TYPE_GC:
        case WDEV_TYPE_APCLI:
        case WDEV_TYPE_P2P_DEVICE:
            pHwResource->OmacBssCtl.OmacBitMap &= ~(1 << Idx);
            return;
        break;

        /*
            Carter note,
            because of WDS0~WDS3 use the same mac address as RA0,
            so the OmacIdx should use ra0's.

            otherwise, if the pkt sa is ra0's but omacIdx use ra1's,
            it will cause the ack from peer could not be passed at Rmac.

            and WDS OmacIdx should not be released, unless Ra0 is released.
        */
        case WDEV_TYPE_WDS:
        break;
        case WDEV_TYPE_MESH:
            /* TODO */
        break;
        case WDEV_TYPE_REPEATER:
            pHwResource->OmacBssCtl.RepeaterBitMap &= ~(1 << (Idx - pHdCfg->chipCap.RepeaterStartIdx));
            return;
        break;
        default:
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: OmacType(%d)\n", __FUNCTION__, OmacType));
        break;
    }

    return;
}

INT32 GetOmacIdx(HD_CFG *pHdCfg, UINT32 OmacType, INT8 Idx)
{
 	HD_RESOURCE_CFG *pHwResource = &pHdCfg->HwResourceCfg;

	switch (OmacType)
	{
		case WDEV_TYPE_AP:
			if (Idx == 0)
			{
				pHwResource->OmacBssCtl.OmacBitMap |= (1 << 0);
				return 0;
			}
			else
			{
				pHwResource->OmacBssCtl.HwMbssBitMap |= (1 << (Idx - 0));
				return pHdCfg->chipCap.ExtMbssOmacStartIdx + Idx;
			}
            break;
		case WDEV_TYPE_STA:
		case WDEV_TYPE_ADHOC:
		case WDEV_TYPE_GO:
		case WDEV_TYPE_GC:
		case WDEV_TYPE_APCLI:
			return GetFirstAvailableOmacIdx(pHdCfg, TRUE);
		break;

        /*
            Carter note,
            because of WDS0~WDS3 use the same mac address as RA0,
            so the OmacIdx should use ra0's.

            otherwise, if the pkt sa is ra0's but omacIdx use ra1's,
            it will cause the ack from peer could not be passed at Rmac.
        */
        case WDEV_TYPE_WDS:
                return 0;
		break;
		case WDEV_TYPE_MESH:
			/* TODO */
		break;
		case WDEV_TYPE_REPEATER:
			return GetFirstAvailableRepeaterOmacIdx(pHdCfg);
		break;
		case WDEV_TYPE_P2P_DEVICE:
			return GetFirstAvailableOmacIdx(pHdCfg, FALSE);
		break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: OmacType(%d)\n", __FUNCTION__, OmacType));
		break;
	}

	return -1;
}


/*
*
*/
INT32 OcAddRepeaterEntry(HD_DEV_OBJ *pObj,UCHAR ReptIdx)
{
	INT32 ret=0;
	HD_REPT_ENRTY *pReptEntry = NULL;
	HD_DEV *pHdev = pObj->pHdev;
	HD_CFG *pHdCfg = (HD_CFG*)pHdev->priv;

	ret = os_alloc_mem(NULL,(UCHAR **)&pReptEntry,sizeof(HD_REPT_ENRTY));

	if (ret != NDIS_STATUS_SUCCESS)
	{
	  MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			  (" Alloc memory for HD_REPT_ENRTY failed.\n"));
	  return ret;
	}

    OS_SPIN_LOCK(&pObj->RefCntLock);
    pObj->RefCnt++;
    OS_SPIN_UNLOCK(&pObj->RefCntLock);
	pReptEntry->CliIdx = ReptIdx;
	pReptEntry->ReptOmacIdx = GetOmacIdx(pHdCfg,WDEV_TYPE_REPEATER,ReptIdx);
	DlListAddTail(&pObj->RepeaterList,&pReptEntry->list);

	return NDIS_STATUS_SUCCESS;

}


/*
*
*/
VOID OcDelRepeaterEntry(HD_DEV_OBJ *pObj,UCHAR ReptIdx)
{
	HD_REPT_ENRTY *pReptEntry = NULL,*tmp=NULL;
	HD_DEV *pHdev = pObj->pHdev;
	HD_CFG *pHdCfg = (HD_CFG*)pHdev->priv;

	DlListForEachSafe(pReptEntry,tmp,&pObj->RepeaterList, struct _HD_REPT_ENRTY, list) {
		if(pReptEntry->CliIdx == ReptIdx)
		{
            OS_SPIN_LOCK(&pObj->RefCntLock);
            if (pObj->RefCnt > 0)
                pObj->RefCnt--;
            else
            {
                MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
                        ("%s, bug here? RefCnt zero already.\n", __FUNCTION__));
            }
            OS_SPIN_UNLOCK(&pObj->RefCntLock);
			ReleaseOmacIdx(pHdCfg,WDEV_TYPE_REPEATER,pReptEntry->ReptOmacIdx);
			DlListDel(&pReptEntry->list);
			os_free_mem(pReptEntry);
		}
	}
}


/*
*
*/
HD_REPT_ENRTY *OcGetRepeaterEntry(HD_DEV_OBJ *pObj,UCHAR ReptIdx)
{
	HD_REPT_ENRTY *pReptEntry = NULL;

	DlListForEach(pReptEntry, &pObj->RepeaterList, struct _HD_REPT_ENRTY, list) {
		if(pReptEntry->CliIdx == ReptIdx)
		{
			return pReptEntry;
		}
	}
	return NULL;
}

