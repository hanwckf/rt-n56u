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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"

/**
 * @param pAd
 * @param ioctl wifi device type
 * @param func_index function device index
 *
 * Get wifi_dev according to ioctl wifi device type
 *
 * @return wifi_dev
 */
struct wifi_dev *get_wdev_by_ioctl_idx_and_iftype(RTMP_ADAPTER *pAd, INT idx, INT iftype)
{
	INT net_device_offset = 0;

	switch (iftype) {
#ifdef P2P_SUPPORT

	case INT_P2P:
		if (P2P_CLI_ON(pAd))
			net_device_offset = MIN_NET_DEVICE_FOR_P2P_CLI;
		else
			net_device_offset = MIN_NET_DEVICE_FOR_P2P_GO;

		break;
#endif /* P2P_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	case INT_MAIN:
	case INT_MBSSID:
		net_device_offset = MIN_NET_DEVICE_FOR_MBSSID;
		break;
#ifdef APCLI_SUPPORT

	case INT_APCLI:
		net_device_offset = MIN_NET_DEVICE_FOR_APCLI;
		break;
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

	case INT_WDS:
		net_device_offset = MIN_NET_DEVICE_FOR_WDS;
		break;
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	case INT_MSTA:
		net_device_offset = MIN_NET_DEVICE_FOR_MBSSID;
		break;
#endif /* CONFIG_STA_SUPPORT */

	default:
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: can not find ioctl_if_type(%d), if_idx(%d)\n",
				 __func__, iftype, idx));
		break;
	}
	return get_wdev_by_idx(pAd, (idx + net_device_offset));
}

struct wifi_dev *get_wdev_by_idx(RTMP_ADAPTER *pAd, INT idx)
{
	struct wifi_dev *wdev = NULL;

	do {
#ifdef P2P_SUPPORT

		if (idx >= MIN_NET_DEVICE_FOR_P2P_GO) {
			wdev = &pAd->ApCfg.MBSSID[idx - MIN_NET_DEVICE_FOR_P2P_GO].wdev;
			break;
		}

#endif /* P2P_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

		if (idx >= MIN_NET_DEVICE_FOR_APCLI) {
			idx -= MIN_NET_DEVICE_FOR_APCLI;

			if (idx < MAX_APCLI_NUM) {
				wdev = &pAd->StaCfg[idx].wdev;
				break;
			}
		}

#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WDS_SUPPORT

			if (idx >= MIN_NET_DEVICE_FOR_WDS) {
				idx -= MIN_NET_DEVICE_FOR_WDS;

				if (idx < MAX_WDS_ENTRY) {
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					break;
				}
			}

#endif /* WDS_SUPPORT */
			if ((idx < pAd->ApCfg.BssidNum || idx == 0) && VALID_MBSS(pAd, idx))
				wdev = &pAd->ApCfg.MBSSID[idx].wdev;

			break;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (idx < MAX_MULTI_STA) {
				wdev = &pAd->StaCfg[idx].wdev;
				break;
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	} while (FALSE);

	if (wdev == NULL)
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("get_wdev_by_idx: invalid idx(%d)\n", idx));

	return wdev;
}

