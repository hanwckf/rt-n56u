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

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "mt_phy.tmh"
#endif
#else
#include "rt_config.h"
#endif

/*define local variable*/
static struct phy_ops MtPhyOp;

static INT32 MTBbpInit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Init BBP Registers\n", __FUNCTION__));

	if (pAd->chipOps.AsicBbpInit != NULL)
		pAd->chipOps.AsicBbpInit(pAd);

	return NDIS_STATUS_SUCCESS;
}


INT32 MTShowPartialBBP(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	UINT32 Offset, Value;

	for (Offset = Start; Offset <= End; Offset += 4)
	{
		PHY_IO_READ32(pAd, Offset, &Value);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():0x%04x 0x%08x\n", __FUNCTION__, Offset, Value));
	}

	return TRUE;
}


INT32 MTShowAllBBP(RTMP_ADAPTER *pAd)
{
	UINT32 Offset, Value;

	for (Offset = 0x10000; Offset <= 0x20000; Offset += 4)
	{
		PHY_IO_READ32(pAd, Offset, &Value);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():0x%04x 0x%08x\n", __FUNCTION__, Offset, Value));
	}

	return TRUE;
}


static INT mt_bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw)
{
	INT ret = FALSE;

#ifdef COMPOS_WIN
#else
	pAd->CommonCfg.BBPCurrentBW = bw;
#endif

	if ((bw == BW_20) || (bw == BW_40) || (bw == BW_80) || (bw == BW_160) || (bw == BW_8080))
	{
#ifdef COMPOS_WIN
#else
		pAd->CommonCfg.BBPCurrentBW = bw;
#endif
		ret = TRUE;
	}
	else
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("unknow bw setting = %d\n", bw));
		ret = FALSE;
	}

	return ret;
}

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT MTSmartCarrierSense(RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.SmartCarrierSense!= NULL)
		pAd->chipOps.SmartCarrierSense(pAd);

	return NDIS_STATUS_SUCCESS;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

static VOID mt_phy_ops(VOID)
{
	os_zero_mem(&MtPhyOp,sizeof(struct phy_ops));
	MtPhyOp.bbp_init = MTBbpInit;
	MtPhyOp.bbp_set_bw = mt_bbp_set_bw;
	MtPhyOp.ShowPartialBBP = MTShowPartialBBP;
	MtPhyOp.ShowAllBBP = MTShowAllBBP;
	MtPhyOp.ShowPartialRF = MTShowPartialRF;
	MtPhyOp.ShowAllRF = MTShowAllRF;
#ifdef CONFIG_AP_SUPPORT    
       MtPhyOp.AutoCh = MTAPAutoSelectChannel;
#endif/*CONFIG_AP_SUPPORT*/ 
#ifdef SMART_CARRIER_SENSE_SUPPORT
	MtPhyOp.Smart_Carrier_Sense = MTSmartCarrierSense;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
}

INT mt_phy_probe(RTMP_ADAPTER *pAd)
{
	mt_phy_ops();
	pAd->phy_op = &MtPhyOp;

	return TRUE;
}
