/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sta.c

	Abstract:
	initialization for STA module

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


VOID STARxErrorHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{

}


INT StaAllowToSendPacket(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket,
	UCHAR *pWcid)
{
	BOOLEAN allowToSend = FALSE;
	MAC_TABLE_ENTRY *pEntry;
	PUCHAR pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);

	// TODO: shiang-usw, fix me about this!!
	if (!(pAd->StaCfg.BssType == BSS_ADHOC || pAd->StaCfg.BssType == BSS_INFRA))
		return FALSE;

	/* Drop send request since we are in monitor mode */
	// TODO: shiang-usw, integrate this check to wdev->allow_data_tx = FALSE!
	if (MONITOR_ON(pAd))
		return FALSE;

	if ((pAd->StaCfg.BssType == BSS_ADHOC) && ADHOC_ON(pAd))
	{	
		if (MAC_ADDR_IS_GROUP(pSrcBufVA)) {
			RTMP_SET_PACKET_WCID(pPacket, wdev->tr_tb_idx);
			*pWcid = wdev->tr_tb_idx;
			allowToSend = TRUE;
		}
		else {
			pEntry = MacTableLookup(pAd, pSrcBufVA);
			if (pEntry) {
				allowToSend = TRUE;
				*pWcid = pEntry->wcid;
			}
		}
		return allowToSend;
	}

	if ((pAd->StaCfg.BssType == BSS_INFRA) && INFRA_ON(pAd))
	{
		*pWcid = BSSID_WCID;
		
		if (0
#ifdef QOS_DLS_SUPPORT
		    || (pAd->CommonCfg.bDLSCapable)
#endif /* QOS_DLS_SUPPORT */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) 
		    || IS_TDLS_SUPPORT(pAd)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		    )
		{
			pEntry = MacTableLookup(pAd, pSrcBufVA);
			
			if (pEntry && (IS_ENTRY_DLS(pEntry)
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				|| IS_ENTRY_TDLS(pEntry)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
				))
				*pWcid = pEntry->wcid;
		}

		return TRUE;
	}

#ifdef CONFIG_FPGA_MODE
	// TODO: shiang-MT7603 work around!
	if ((IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd)) && (pAd->fpga_ctl.fpga_on & 0x1))
	{
		*pWcid = MCAST_WCID;
		return TRUE;
	}
#endif /* CONFIG_FPGA_MODE */

	return FALSE;
}


INT StaAllowToSendPacket_new(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pWcid)
{
	MAC_TABLE_ENTRY *pEntry;
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;

	/* Drop send request since we are in monitor mode */
	// TODO: shiang-usw, integrate this check to wdev->allow_data_tx = FALSE!
	if (MONITOR_ON(pAd))
		return FALSE;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (MAC_ADDR_IS_GROUP(pSrcBufVA))
	{
		*pWcid = wdev->tr_tb_idx;
#ifdef CONFIG_FPGA_MODE
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Group Packet, wdev->tr_tb_idx=%d\n", __FUNCTION__, wdev->tr_tb_idx));
		// TODO: shiang-MT7603 work around!
		if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd))
			*pWcid = MCAST_WCID;
#endif /* CONFIG_FPGA_MODE */

		return TRUE;
	}

	pEntry = MacTableLookup(pAd, pSrcBufVA);
	if (pEntry) {
		ASSERT((pEntry->wdev == wdev));
	}

	if (pEntry && (pEntry->Sst == SST_ASSOC))
	{
		*pWcid = (UCHAR)pEntry->wcid;
		return TRUE;
	}

	return FALSE;
}


INT sta_func_init(RTMP_ADAPTER *pAd)
{

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
	/* send wireless event to wpa_supplicant for infroming interface up.*/
	RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_UP, NULL, NULL, 0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */


	return TRUE;
}


INT STAInitialize(RTMP_ADAPTER *pAd)
{

	wdev_init(pAd, &pAd->StaCfg.wdev, WDEV_TYPE_STA);

	return 0;
}

