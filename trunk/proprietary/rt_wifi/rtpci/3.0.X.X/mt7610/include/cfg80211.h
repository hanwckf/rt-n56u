/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
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

	All MAC80211/CFG80211 Related Structure & Definition.

***************************************************************************/

#ifdef RT_CFG80211_SUPPORT

#include <linux/ieee80211.h>


/* get RALINK pAd control block in 80211 Ops */
#define MAC80211_PAD_GET(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL)											\
		{															\
			CFG80211DBG(RT_DEBUG_ERROR,								\
					("80211> %s but pAd = NULL!", __FUNCTION__));	\
			return -EINVAL;											\
		}															\
	}

#define MAC80211_PAD_GET_NO_RV(__pAd, __pWiphy)						\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL)											\
		{															\
			CFG80211DBG(RT_DEBUG_ERROR,								\
					("80211> %s but pAd = NULL!", __FUNCTION__));	\
			return;											\
		}															\
	}

#define MAC80211_PAD_GET_RETURN_NULL(__pAd, __pWiphy)				\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL)											\
		{															\
			CFG80211DBG(RT_DEBUG_ERROR,								\
					("80211> %s but pAd = NULL!", __FUNCTION__));	\
			return NULL;											\
		}															\
	}


typedef struct __CFG80211_CB {

	/* we can change channel/rate information on the fly so we backup them */
	struct ieee80211_supported_band Cfg80211_bands[IEEE80211_NUM_BANDS];
	struct ieee80211_channel *pCfg80211_Channels;
	struct ieee80211_rate *pCfg80211_Rates;

	/* used in wiphy_unregister */
	struct wireless_dev *pCfg80211_Wdev;

	/* used in scan end */
	struct cfg80211_scan_request *pCfg80211_ScanReq;

	/* monitor filter */
	UINT32 MonFilterFlag;

	/* channel information */
	struct ieee80211_channel ChanInfo[MAX_NUM_OF_CHANNELS];

	/* to protect scan status */
	spinlock_t scan_notify_lock;
} CFG80211_CB;




/*
========================================================================
Routine Description:
	Register MAC80211 Module.

Arguments:
	pAd				- WLAN control block pointer
	pDev			- Generic device interface
	pNetDev			- Network device

Return Value:
	NONE

Note:
	pDev != pNetDev
	#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))

	Can not use pNetDev to replace pDev; Or kernel panic.
========================================================================
*/
BOOLEAN CFG80211_Register(
	VOID						*pAd,
	struct device				*pDev,
	struct net_device			*pNetDev);


BOOLEAN CFG80211DRV_OpsBeaconSet(
    VOID                                            *pAdOrg,
    VOID                                            *pData,
	BOOLEAN                                          isAdd);

VOID CFG80211_UpdateBeacon(
	VOID                                            *pAdOrg,
	UCHAR 										    *beacon_head_buf,
	UINT32											beacon_head_len,
	UCHAR 										    *beacon_tail_buf,
	UINT32											beacon_tail_len,
	BOOLEAN											isAllUpdate);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0))
INT CFG80211_OpsSetBeacon(
        struct wiphy *pWiphy,
        struct net_device *netdev,
        struct beacon_parameters *info);

INT CFG80211_OpsAddBeacon(
        struct wiphy *pWiphy,
        struct net_device *netdev,
        struct beacon_parameters *info);

INT CFG80211_OpsDelBeacon(
        struct wiphy *pWiphy,
        struct net_device *netdev);
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0) */
INT CFG80211_OpsStartAp(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_ap_settings *settings);

INT CFG80211_OpsChangeBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_beacon_data *info);

INT CFG80211_OpsStopAp(
	struct wiphy *pWiphy,
	struct net_device *netdev);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0) */
#endif /* RT_CFG80211_SUPPORT */

/* End of cfg80211.h */

