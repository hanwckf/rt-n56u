/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
 
	Abstract:

	All related CFG80211 Scan function body.

	History:

***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"



/* Refine on 2013/04/30 for two functin into one */
INT CFG80211DRV_OpsScanGetNextChannel(
	VOID						*pAdOrg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	
	if (cfg80211_ctrl->pCfg80211ChanList != NULL)
	{
		if (cfg80211_ctrl->Cfg80211CurChanIndex < cfg80211_ctrl->Cfg80211ChanListLen)
		{
			return cfg80211_ctrl->pCfg80211ChanList[cfg80211_ctrl->Cfg80211CurChanIndex++];
		}
		else
		{
            os_free_mem(NULL, cfg80211_ctrl->pCfg80211ChanList);
            cfg80211_ctrl->pCfg80211ChanList = NULL;
            cfg80211_ctrl->Cfg80211ChanListLen = 0;
			cfg80211_ctrl->Cfg80211CurChanIndex = 0;
			
			return 0;
		}
	}

	return 0;
}

BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT8						 dataLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	UINT32 *pChanList = (UINT32 *) pData;

	if (pChanList != NULL) 
	{
		if (cfg80211_ctrl->pCfg80211ChanList != NULL)
			os_free_mem(NULL, cfg80211_ctrl->pCfg80211ChanList);

		os_alloc_mem(NULL, (UCHAR **)&cfg80211_ctrl->pCfg80211ChanList, sizeof(UINT32 *) * dataLen);
		if (cfg80211_ctrl->pCfg80211ChanList != NULL)
		{
			NdisCopyMemory(cfg80211_ctrl->pCfg80211ChanList, pChanList, sizeof(UINT32 *) * dataLen);
			cfg80211_ctrl->Cfg80211ChanListLen = dataLen;
			cfg80211_ctrl->Cfg80211CurChanIndex = 0 ; /* Start from index 0 */
			return NDIS_STATUS_SUCCESS;
		}
		else
		{
			return NDIS_STATUS_FAILURE;
		}
	}
	
	return NDIS_STATUS_FAILURE;
}

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	VOID						*pAdOrg,
	UINT8						 IfType)
{
	return TRUE;
}

BOOLEAN CFG80211DRV_OpsScanExtraIesSet(
	VOID						*pAdOrg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CFG80211_CB *pCfg80211_CB = pAd->pCfg80211_CB;
	UINT ie_len = 0;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_CB->pCfg80211_ScanReq)
		ie_len = pCfg80211_CB->pCfg80211_ScanReq->ie_len;

    CFG80211DBG(RT_DEBUG_INFO, ("80211> CFG80211DRV_OpsExtraIesSet ==> %d\n", ie_len)); 
	if (ie_len == 0)
		return FALSE;

	/* Reset the ExtraIe and Len */
	if (cfg80211_ctrl->pExtraIe)
	{	
		os_free_mem(NULL, cfg80211_ctrl->pExtraIe);
		cfg80211_ctrl->pExtraIe = NULL;
	}
	cfg80211_ctrl->ExtraIeLen = 0;
	
	os_alloc_mem(pAd, (UCHAR **)&(cfg80211_ctrl->pExtraIe), ie_len);
	if (cfg80211_ctrl->pExtraIe)
	{
		NdisCopyMemory(cfg80211_ctrl->pExtraIe, pCfg80211_CB->pCfg80211_ScanReq->ie, ie_len);
		cfg80211_ctrl->ExtraIeLen = ie_len;
		hex_dump("CFG8021_SCAN_EXTRAIE", cfg80211_ctrl->pExtraIe, cfg80211_ctrl->ExtraIeLen);
	}
	else
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211DRV_OpsExtraIesSet ==> allocate fail. \n")); 
		return FALSE;
	}
	
	return TRUE;
}

#ifdef CFG80211_SCAN_SIGNAL_AVG
static void CFG80211_CalBssAvgRssi(
	IN      BSS_ENTRY 				*pBssEntry)
{
        BOOLEAN bInitial = FALSE;

        if (!(pBssEntry->AvgRssiX8 | pBssEntry->AvgRssi))
        {
                bInitial = TRUE;
        }

        if (bInitial)
        {
                pBssEntry->AvgRssiX8 = pBssEntry->Rssi << 3;
                pBssEntry->AvgRssi  = pBssEntry->Rssi;
        }
        else
        {
        		/* For smooth purpose, oldRssi for 7/8, newRssi for 1/8 */
                pBssEntry->AvgRssiX8 = 
					(pBssEntry->AvgRssiX8 - pBssEntry->AvgRssi) + pBssEntry->Rssi;
        }

        pBssEntry->AvgRssi = pBssEntry->AvgRssiX8 >> 3;

}

static void CFG80211_UpdateBssTableRssi(
	IN VOID							*pAdCB)
{

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct ieee80211_channel *chan;
	struct cfg80211_bss *bss;
	BSS_ENTRY *pBssEntry;
	UINT index;
	UINT32 CenFreq;
	
	for (index = 0; index < pAd->ScanTab.BssNr; index++) 
	{
		pBssEntry = &pAd->ScanTab.BssEntry[index];
			
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)) 
		if (pAd->ScanTab.BssEntry[index].Channel > 14) 
			CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel , IEEE80211_BAND_5GHZ);
		else 
			CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel , IEEE80211_BAND_2GHZ);
#else
		CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel);
#endif

		chan = ieee80211_get_channel(pWiphy, CenFreq);			
		bss = cfg80211_get_bss(pWiphy, chan, pBssEntry->Bssid, pBssEntry->Ssid, pBssEntry->SsidLen, 
						WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
		if (bss == NULL)
		{
			/* ScanTable Entry not exist in kernel buffer */
		}
		else
		{
			/* HIT */
			CFG80211_CalBssAvgRssi(pBssEntry);
			bss->signal = pBssEntry->AvgRssi * 100; //UNIT: MdBm
			cfg80211_put_bss(bss);
		}
	}	
}
#endif /* CFG80211_SCAN_SIGNAL_AVG */

/*
========================================================================
Routine Description:
	Inform us that a scan is got.

Arguments:
	pAdCB				- WLAN control block pointer

Return Value:
	NONE

Note:
	Call RT_CFG80211_SCANNING_INFORM, not CFG80211_Scaning
========================================================================
*/
VOID CFG80211_Scaning(
	IN VOID							*pAdCB,
	IN UINT32						BssIdx,
	IN UINT32						ChanId,
	IN UCHAR						*pFrame,
	IN UINT32						FrameLen,
	IN INT32						RSSI)
{
}


/*
========================================================================
Routine Description:
	Inform us that scan ends.

Arguments:
	pAdCB			- WLAN control block pointer
	FlgIsAborted	- 1: scan is aborted

Return Value:
	NONE

Note:
========================================================================
*/
VOID CFG80211_ScanEnd(
	IN VOID						*pAdCB,
	IN BOOLEAN					FlgIsAborted)
{
} 

VOID CFG80211_ScanStatusLockInit(
	IN VOID						*pAdCB,
	IN UINT                      init)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
		
	if (init)
	{
		NdisAllocateSpinLock(pAd, &pCfg80211_CB->scan_notify_lock);
	}
	else
	{
		NdisFreeSpinLock(&pCfg80211_CB->scan_notify_lock);
	}
}

#endif /* RT_CFG80211_SUPPORT */

