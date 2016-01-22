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
	mt_phy.c
*/


#include "rt_config.h"
static INT32 MTBbpInit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Init BBP Registers\n", __FUNCTION__));

	if (pAd->chipOps.AsicBbpInit != NULL)
		pAd->chipOps.AsicBbpInit(pAd);

	return NDIS_STATUS_SUCCESS;
}


INT32 MTShowPartialBBP(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	UINT32 Offset, Value;

	for (Offset = Start; Offset <= End; Offset += 4)
	{
		RTMP_IO_READ32(pAd, Offset, &Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():0x%04x 0x%08x\n", __FUNCTION__, Offset, Value));
	}

	return TRUE;
}


INT32 MTShowAllBBP(RTMP_ADAPTER *pAd)
{
	UINT32 Offset, Value;

	for (Offset = 0x10000; Offset <= 0x20000; Offset += 4)
	{
		RTMP_IO_READ32(pAd, Offset, &Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():0x%04x 0x%08x\n", __FUNCTION__, Offset, Value));
	}

	return TRUE;
}


static INT mt_bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw)
{
	INT ret = FALSE;

	pAd->CommonCfg.BBPCurrentBW = bw;

	if ((bw == BW_20) || (bw == BW_40))
	{
		pAd->CommonCfg.BBPCurrentBW = bw;


		ret = TRUE;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("unknow bw setting = %d\n", bw));
		ret = FALSE;
	}

	return ret;
}


static struct phy_ops mt_phy_ops = {
	.bbp_init = MTBbpInit,
	.bbp_set_bw = mt_bbp_set_bw,
	.ShowPartialBBP = MTShowPartialBBP,
	.ShowAllBBP = MTShowAllBBP,
	.ShowPartialRF = MTShowPartialRF,
	.ShowAllRF = MTShowAllRF,
};


INT mt_phy_probe(RTMP_ADAPTER *pAd)
{
	pAd->phy_op = &mt_phy_ops;

	return TRUE;
}
