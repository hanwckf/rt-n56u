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
	fwdl.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "fwdl.tmh"
#endif
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#ifdef MT7615
#if defined(RTMP_PCI_SUPPORT)
VOID NICResetFirmware(RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.loadFirmware(pAd)) 
	{
	    pAd->chipOps.resetFirmware(pAd);
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
 		    ("%s::Reset fw %s\n", __FUNCTION__, "SUCCESS"));
	}
}
#endif
#endif
VOID NICEraseRomPatch(RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.erase_rom_patch)
	{
		pAd->chipOps.erase_rom_patch(pAd);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                                ("%s\n", __FUNCTION__));
	}
}

INT NICLoadRomPatch(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	if (ad->chipOps.load_rom_patch) 
    {
        ret = ad->chipOps.load_rom_patch(ad);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s::load Rom Patch %s\n", __FUNCTION__, 
            (ret == NDIS_STATUS_SUCCESS)?"SUCCESS":"FAIL"));

	}

	return ret;
}


INT NICLoadFirmware(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;

	if (ad->chipOps.loadFirmware) 
	{
	    ret = ad->chipOps.loadFirmware(ad);
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
 		("%s::load fw %s\n", __FUNCTION__, (ret == NDIS_STATUS_SUCCESS)?"SUCCESS":"FAIL"));
	}

	return ret;
}


VOID NICEraseFirmware(RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.eraseFirmware)
	{
		pAd->chipOps.eraseFirmware(pAd);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                                    ("%s\n", __FUNCTION__));
	}
}

VOID NICRestartFirmware(RTMP_ADAPTER *pAd)
{
	if(pAd->chipOps.restartFirmware)
	{
		pAd->chipOps.restartFirmware(pAd);
	}
}


INT FwdlHookInit(RTMP_ADAPTER *pAd)
{
	int ret = NDIS_STATUS_SUCCESS;
#ifdef MT_MAC
	if(pAd->chipCap.hif_type == HIF_MT)
	{
		AndesMtFwdlHookInit(pAd);
	}
#endif

	return ret;
}
