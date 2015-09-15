/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2009, Ralink Technology, Inc.
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

	All related CRDA (Central Regulatory Domain Agent) function body.

	History:
		1. 2009/09/17	Sample Lin
			(1) Init version.
		2. 2009/10/27	Sample Lin
			(1) Do not use ieee80211_register_hw() to create virtual interface.
				Use wiphy_register() to register nl80211 command handlers.
			(2) Support iw utility.
		3. 2009/11/03	Sample Lin
			(1) Change name MAC80211 to CFG80211.
			(2) Modify CFG80211_OpsSetChannel().
			(3) Move CFG80211_Register()/CFG80211_UnRegister() to open/close.
		4. 2009/12/16	Sample Lin
			(1) Patch for Linux 2.6.32.
			(2) Add more supported functions in CFG80211_Ops.

	Note:
		The feature is supported only in "LINUX" 2.6.28 ~ 2.6.32.

***************************************************************************/

#include "rt_config.h"


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
#ifdef RT_CFG80211_SUPPORT

#define RT_CFG80211_DEBUG /* debug use */
#define CFG80211CB			((CFG80211_CB *)(pAd->pCfg80211_CB))


#ifdef RT_CFG80211_DEBUG
#define CFG80211DBG(__Flg, __pMsg)		DBGPRINT(__Flg, __pMsg)
#else
#define CFG80211DBG(__Flg, __pMsg)
#endif // RT_CFG80211_DEBUG //

/* 1 ~ 14 */
#define CFG80211_NUM_OF_CHAN_2GHZ			14

/* 36 ~ 64, 100 ~ 136, 140 ~ 161 */
#define CFG80211_NUM_OF_CHAN_5GHZ			\
							(sizeof(Cfg80211_Chan)-CFG80211_NUM_OF_CHAN_2GHZ)

/*
	Array of bitrates the hardware can operate with
	in this band. Must be sorted to give a valid "supported
	rates" IE, i.e. CCK rates first, then OFDM.

	For HT, assign MCS in another structure, ieee80211_sta_ht_cap.
*/
const struct ieee80211_rate Cfg80211_SupRate[12] = {
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 10,
		.hw_value = 0,
		.hw_value_short = 0,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 20,
		.hw_value = 1,
		.hw_value_short = 1,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 55,
		.hw_value = 2,
		.hw_value_short = 2,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 110,
		.hw_value = 3,
		.hw_value_short = 3,
	},
	{
		.flags = 0,
		.bitrate = 60,
		.hw_value = 4,
		.hw_value_short = 4,
	},
	{
		.flags = 0,
		.bitrate = 90,
		.hw_value = 5,
		.hw_value_short = 5,
	},
	{
		.flags = 0,
		.bitrate = 120,
		.hw_value = 6,
		.hw_value_short = 6,
	},
	{
		.flags = 0,
		.bitrate = 180,
		.hw_value = 7,
		.hw_value_short = 7,
	},
	{
		.flags = 0,
		.bitrate = 240,
		.hw_value = 8,
		.hw_value_short = 8,
	},
	{
		.flags = 0,
		.bitrate = 360,
		.hw_value = 9,
		.hw_value_short = 9,
	},
	{
		.flags = 0,
		.bitrate = 480,
		.hw_value = 10,
		.hw_value_short = 10,
	},
	{
		.flags = 0,
		.bitrate = 540,
		.hw_value = 11,
		.hw_value_short = 11,
	},
};

/* all available channels */
static const UCHAR Cfg80211_Chan[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,

	/* 802.11 UNI / HyperLan 2 */
	36, 38, 40, 44, 46, 48, 52, 54, 56, 60, 62, 64,

	/* 802.11 HyperLan 2 */
	100, 104, 108, 112, 116, 118, 120, 124, 126, 128, 132, 134, 136,

	/* 802.11 UNII */
	140, 149, 151, 153, 157, 159, 161, 165, 167, 169, 171, 173,

	/* Japan */
	184, 188, 192, 196, 208, 212, 216,
};


static const UINT32 CipherSuites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
};


typedef struct __CFG80211_CB
{
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
} CFG80211_CB;

/*
	The driver's regulatory notification callback.
*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static INT32 CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN struct regulatory_request	*pRequest);
#else
static INT32 CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN enum reg_set_by				Request);
#endif // LINUX_VERSION_CODE //

/*
	Initialize wireless channel in 2.4GHZ and 5GHZ.
*/
static BOOLEAN CFG80211_SupBandInit(
	IN PRTMP_ADAPTER 				pAd,
	IN struct wiphy					*pWiphy,
	IN struct ieee80211_channel		*pChannels,
	IN struct ieee80211_rate		*pRates);




/* =========================== Private Function ============================== */

/* get RALINK pAd control block in 80211 Ops */
#define MAC80211_PAD_GET(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (PRTMP_ADAPTER)(*__pPriv);							\
		if (__pAd == NULL)											\
		{															\
			DBGPRINT(RT_DEBUG_ERROR,								\
					("80211> %s but pAd = NULL!", __FUNCTION__));	\
			return -EINVAL;											\
		}															\
	}

/*
========================================================================
Routine Description:
	Set channel.

Arguments:
	pWiphy			- Wireless hardware description
	pChan			- Channel information
	ChannelType		- Channel type

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: set channel, set freq

	enum nl80211_channel_type {
		NL80211_CHAN_NO_HT,
		NL80211_CHAN_HT20,
		NL80211_CHAN_HT40MINUS,
		NL80211_CHAN_HT40PLUS
	};
========================================================================
*/
static int CFG80211_OpsSetChannel(
	IN struct wiphy					*pWiphy,
	IN struct ieee80211_channel		*pChan,
	IN enum nl80211_channel_type	ChannelType)
{
	PRTMP_ADAPTER pAd;
	UINT32 ChanId;
	STRING ChStr[5] = { 0 };
#ifdef DOT11_N_SUPPORT
	UCHAR BW_Old;
	BOOLEAN FlgIsChanged;
#endif // DOT11_N_SUPPORT //


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_OpsSetChannel ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(RT_DEBUG_ERROR, ("80211> Channel = %d\n", ChanId));
	CFG80211DBG(RT_DEBUG_ERROR, ("80211> ChannelType = %d\n", ChannelType));

#ifdef DOT11_N_SUPPORT
	if (CFG80211CB->pCfg80211_Wdev->iftype != NL80211_IFTYPE_MONITOR)
	{
		/* get channel BW */
		FlgIsChanged = FALSE;
		BW_Old = pAd->CommonCfg.RegTransmitSetting.field.BW;
	
		/* set to new channel BW */
		if (ChannelType == NL80211_CHAN_HT20)
		{
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;
			FlgIsChanged = TRUE;
		}
		else if ((ChannelType == NL80211_CHAN_HT40MINUS) ||
				(ChannelType == NL80211_CHAN_HT40PLUS))
		{
			/* not support NL80211_CHAN_HT40MINUS or NL80211_CHAN_HT40PLUS */
			/* i.e. primary channel = 36, secondary channel must be 40 */
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
			FlgIsChanged = TRUE;
		} /* End of if */
	
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> New BW = %d\n",
					pAd->CommonCfg.RegTransmitSetting.field.BW));
	
		/* change HT/non-HT mode (do NOT change wireless mode here) */
		if (((ChannelType == NL80211_CHAN_NO_HT) &&
			(pAd->CommonCfg.HT_Disable == 0)) ||
			((ChannelType != NL80211_CHAN_NO_HT) &&
			(pAd->CommonCfg.HT_Disable == 1)))
		{
			if (ChannelType == NL80211_CHAN_NO_HT)
				pAd->CommonCfg.HT_Disable = 1;
			else
				pAd->CommonCfg.HT_Disable = 0;
			/* End of if */
	
			FlgIsChanged = TRUE;
			CFG80211DBG(RT_DEBUG_ERROR, ("80211> HT Disable = %d\n",
						pAd->CommonCfg.HT_Disable));
		} /* End of if */
	}
	else
	{
		/* for monitor mode */
		FlgIsChanged = TRUE;
		pAd->CommonCfg.HT_Disable = 0;
		pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
	} /* End of if */

	if (FlgIsChanged == TRUE)
		SetCommonHT(pAd);
	/* End of if */
#endif // DOT11_N_SUPPORT //

	/* switch to the channel */
	sprintf(ChStr, "%d", ChanId);
	if (Set_Channel_Proc(pAd, ChStr) == FALSE)
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> Change channel fail!\n"));
	} /* End of if */


	return 0;
} /* End of CFG80211_OpsSetChannel */


/*
========================================================================
Routine Description:
	Change type/configuration of virtual interface.

Arguments:
	pWiphy			- Wireless hardware description
	IfIndex			- Interface index
	Type			- Interface type, managed/adhoc/ap/station, etc.
	pFlags			- Monitor flags
	pParams			- Mesh parameters

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: set type, set monitor
========================================================================
*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
static int CFG80211_OpsChgVirtualInf(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNetDevIn,
	IN enum nl80211_iftype			Type,
	IN u32							*pFlags,
	struct vif_params				*pParams)
#else
static int CFG80211_OpsChgVirtualInf(
	IN struct wiphy					*pWiphy,
	IN int							IfIndex,
	IN enum nl80211_iftype			Type,
	IN u32							*pFlags,
	struct vif_params				*pParams)
#endif // LINUX_VERSION_CODE //
{
	PRTMP_ADAPTER pAd;
	struct net_device *pNetDev;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_OpsChgVirtualInf ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> Type = %d\n", Type));

	/* sanity check */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Wrong interface type %d!\n", Type));
		return -EINVAL;
	} /* End of if */

	/* update interface type */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
	pNetDev = pNetDevIn;
#else
	pNetDev = __dev_get_by_index(&init_net, IfIndex);
#endif // LINUX_VERSION_CODE //

	if (pNetDev == NULL)
		return -ENODEV;
	/* End of if */

	pNetDev->ieee80211_ptr->iftype = Type;

	/* change type */

	return 0;
} /* End of CFG80211_OpsChgVirtualInf */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#endif // LINUX_VERSION_CODE //


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
#endif // LINUX_VERSION_CODE //


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
/*
========================================================================
Routine Description:
	Set the transmit power according to the parameters.

Arguments:
	pWiphy			- Wireless hardware description
	Type			- 
	dBm				- dBm

Return Value:
	0				- success
	-x				- fail

Note:
	Type -
	TX_POWER_AUTOMATIC: the dbm parameter is ignored
	TX_POWER_LIMITED: limit TX power by the dbm parameter
	TX_POWER_FIXED: fix TX power to the dbm parameter
========================================================================
*/
static int CFG80211_TxPwrSet(
	IN struct wiphy						*pWiphy,
	IN enum tx_power_setting			Type,
	IN int								dBm)
{
	return -EOPNOTSUPP;
} /* End of CFG80211_TxPwrSet */


/*
========================================================================
Routine Description:
	Store the current TX power into the dbm variable.

Arguments:
	pWiphy			- Wireless hardware description
	pdBm			- dBm

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_TxPwrGet(
	IN struct wiphy						*pWiphy,
	IN int								*pdBm)
{
	return -EOPNOTSUPP;
} /* End of CFG80211_TxPwrGet */


/*
========================================================================
Routine Description:
	Power management.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- 
	FlgIsEnabled	-
	Timeout			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_PwrMgmt(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN bool								FlgIsEnabled,
	IN int								Timeout)
{
	return -EOPNOTSUPP;
} /* End of CFG80211_PwrMgmt */


/*
========================================================================
Routine Description:
	Get information for a specific station.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	pMac			- STA MAC
	pSinfo			- STA INFO

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_StaGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							*pMac,
	IN struct station_info				*pSinfo)
{
	PRTMP_ADAPTER pAd;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_StaGet ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	memset(pSinfo, 0, sizeof(*pSinfo));

#ifdef CONFIG_AP_SUPPORT
{
	MAC_TABLE_ENTRY *pEntry;
	ULONG DataRate = 0;
	UINT32 RSSI;


	pEntry = MacTableLookup(pAd, pMac);
	if (pEntry == NULL)
		return -ENOENT;
	/* End of if */

	/* fill tx rate */
	getRate(pEntry->HTPhyMode, &DataRate);

	if ((pEntry->HTPhyMode.field.MODE == MODE_HTMIX) ||
		(pEntry->HTPhyMode.field.MODE == MODE_HTGREENFIELD))
	{
		pSinfo->txrate.flags = RATE_INFO_FLAGS_MCS;
		if (pEntry->HTPhyMode.field.BW)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
		/* End of if */
		if (pEntry->HTPhyMode.field.ShortGI)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
		/* End of if */

		pSinfo->txrate.mcs = pEntry->HTPhyMode.field.MCS;
	}
	else
	{
		pSinfo->txrate.legacy = DataRate*1000; /* unit: 100kbps */
	} /* End of if */

	pSinfo->filled |= STATION_INFO_TX_BITRATE;

	/* fill signal */
	RSSI = (pEntry->RssiSample.AvgRssi0 +
			pEntry->RssiSample.AvgRssi1 +
			pEntry->RssiSample.AvgRssi2) / 3;
	pSinfo->signal = le32_to_cpu(RSSI);
	pSinfo->filled |= STATION_INFO_SIGNAL;

	/* fill tx count */
	pSinfo->tx_packets = pEntry->OneSecTxNoRetryOkCount + 
						pEntry->OneSecTxRetryOkCount + 
						pEntry->OneSecTxFailCount;
	pSinfo->filled |= STATION_INFO_TX_PACKETS;

	/* fill inactive time */
	pSinfo->inactive_time = pEntry->NoDataIdleCount * 1000; /* unit: ms */
	pSinfo->inactive_time *= MLME_TASK_EXEC_MULTIPLE;
	pSinfo->inactive_time /= 20;
	pSinfo->filled |= STATION_INFO_INACTIVE_TIME;
}
#endif // CONFIG_AP_SUPPORT //


	return 0;
} /* End of CFG80211_StaGet */


/*
========================================================================
Routine Description:
	List all stations known, e.g. the AP on managed interfaces.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	Idx				- 
	pMac			-
	pSinfo			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_StaDump(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN int								Idx,
	IN UINT8							*pMac,
	IN struct station_info				*pSinfo)
{
	PRTMP_ADAPTER pAd;


	if (Idx != 0)
		return -ENOENT;
	/* End of if */

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_StaDump ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);


	return -EOPNOTSUPP;
} /* End of CFG80211_StaDump */


/*
========================================================================
Routine Description:
	Notify that wiphy parameters have changed.

Arguments:
	pWiphy			- Wireless hardware description
	Changed			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_WiphyParamsSet(
	IN struct wiphy						*pWiphy,
	IN UINT32							Changed)
{
	return -EOPNOTSUPP;
} /* End of CFG80211_WiphyParamsSet */


/*
========================================================================
Routine Description:
	Add a key with the given parameters.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-
	pMacAddr		-
	pParams			-

Return Value:
	0				- success
	-x				- fail

Note:
	pMacAddr will be NULL when adding a group key.
========================================================================
*/
static int CFG80211_KeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr,
	IN struct key_params				*pParams)
{
	PRTMP_ADAPTER pAd;
	UCHAR KeyBuf[50];


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_KeyAdd ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

#ifdef RT_CFG80211_DEBUG
	hex_dump("KeyBuf=", (UINT8 *)pParams->key, pParams->key_len);
#endif // RT_CFG80211_DEBUG //

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> KeyIdx = %d\n", KeyIdx));

	if (pParams->key_len >= sizeof(KeyBuf))
		return -EINVAL;
	/* End of if */

	memcpy(KeyBuf, pParams->key, pParams->key_len);
	KeyBuf[pParams->key_len] = 0x00;


#ifdef CONFIG_AP_SUPPORT
	return -ENOTSUPP;
#endif // CONFIG_AP_SUPPORT //
} /* End of CFG80211_KeyAdd */


/*
========================================================================
Routine Description:
	Get information about the key with the given parameters.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-
	pMacAddr		-
	pCookie			-
	pCallback		-

Return Value:
	0				- success
	-x				- fail

Note:
	pMacAddr will be NULL when requesting information for a group key.

	All pointers given to the pCallback function need not be valid after
	it returns.

	This function should return an error if it is not possible to
	retrieve the key, -ENOENT if it doesn't exist.
========================================================================
*/
static int CFG80211_KeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *cookie,
												 struct key_params *))
{

	return -ENOTSUPP;
} /* End of CFG80211_KeyGet */


/*
========================================================================
Routine Description:
	Remove a key given the pMacAddr (NULL for a group key) and KeyIdx.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-
	pMacAddr		-

Return Value:
	0				- success
	-x				- fail

Note:
	return -ENOENT if the key doesn't exist.
========================================================================
*/
static int CFG80211_KeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr)
{
	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_KeyDel ==>\n"));
	return -ENOTSUPP;
} /* End of CFG80211_KeyDel */


/*
========================================================================
Routine Description:
	Set the default key on an interface.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			-
	KeyIdx			-

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_KeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx)
{
	PRTMP_ADAPTER pAd;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_KeyDefaultSet ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> KeyIdx = %d\n", KeyIdx));

	return 0;
} /* End of CFG80211_KeyDefaultSet */


#endif // LINUX_VERSION_CODE //

#ifdef RFKILL_HW_SUPPORT
static int CFG80211_RFKill(
	IN struct wiphy						*pWiphy)
{
	PRTMP_ADAPTER	pAd;
	UINT32			data = 0;
	BOOLEAN			active;

	MAC80211_PAD_GET(pAd, pWiphy);

	// Read GPIO pin2 as Hardware controlled radio state
	RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
	active = !!(data & 0x04);
	if (!active)
  		 RTMPSetLED(pAd, LED_RADIO_OFF);


	wiphy_rfkill_set_hw_state(pWiphy, !active);
	
	return active;
}
#endif // RFKILL_HW_SUPPORT //

static struct cfg80211_ops CFG80211_Ops = {
	.set_channel			= CFG80211_OpsSetChannel,
	.change_virtual_intf	= CFG80211_OpsChgVirtualInf,

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#endif // LINUX_VERSION_CODE //

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
#endif // LINUX_VERSION_CODE //

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
	.set_tx_power			= CFG80211_TxPwrSet,
	.get_tx_power			= CFG80211_TxPwrGet,
	.set_power_mgmt			= CFG80211_PwrMgmt,
	.get_station			= CFG80211_StaGet,
	.dump_station			= CFG80211_StaDump,
	.set_wiphy_params		= CFG80211_WiphyParamsSet,
	.add_key				= CFG80211_KeyAdd,
	.get_key				= CFG80211_KeyGet,
	.del_key				= CFG80211_KeyDel,
	.set_default_key		= CFG80211_KeyDefaultSet,
#endif // LINUX_VERSION_CODE //
#ifdef RFKILL_HW_SUPPORT
	.rfkill_poll			= CFG80211_RFKill,
#endif // RFKILL_HW_SUPPORT //
};


/*
========================================================================
Routine Description:
	Allocate a wireless device.

Arguments:
	pAd				- WLAN control block pointer
	pDev			- Generic device interface

Return Value:
	wireless device

Note:
========================================================================
*/
static struct wireless_dev *CFG80211_WdevAlloc(
	IN PRTMP_ADAPTER 				pAd,
	struct device					*pDev)
{
	struct wireless_dev *pWdev;
	ULONG *pPriv;


	/*
	 * We're trying to have the following memory layout:
	 *
	 * +------------------------+
	 * | struct wiphy			|
	 * +------------------------+
	 * | pAd pointer			|
	 * +------------------------+
	 */

	pWdev = kzalloc(sizeof(struct wireless_dev), GFP_KERNEL);
	if (pWdev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Wireless device allocation fail!\n"));
		return NULL;
	} /* End of if */

	pWdev->wiphy = wiphy_new(&CFG80211_Ops, sizeof(ULONG *));
	if (pWdev->wiphy == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Wiphy device allocation fail!\n"));
		goto LabelErrWiphyNew;
	} /* End of if */

	/* keep pAd pointer */
	pPriv = (ULONG *)(wiphy_priv(pWdev->wiphy));
	*pPriv = (ULONG)pAd;

	set_wiphy_dev(pWdev->wiphy, pDev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	pWdev->wiphy->max_scan_ssids = MAX_LEN_OF_BSS_TABLE;
#endif // KERNEL_VERSION //

#ifdef CONFIG_AP_SUPPORT
	pWdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_AP);
#ifdef WDS_SUPPORT
	pWdev->wiphy->interface_modes | = BIT(NL80211_IFTYPE_WDS);
#endif // WDS_SUPPORT //
#endif // CONFIG_STA_SUPPORT //

	pWdev->wiphy->reg_notifier = CFG80211_RegNotifier;

	/* init channel information */
	CFG80211_SupBandInit(pAd, pWdev->wiphy, NULL, NULL);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	/* CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm) */
	pWdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
#endif // KERNEL_VERSION //

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
	pWdev->wiphy->cipher_suites = CipherSuites;
	pWdev->wiphy->n_cipher_suites = ARRAY_SIZE(CipherSuites);
#endif // LINUX_VERSION_CODE //

	if (wiphy_register(pWdev->wiphy) < 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Register wiphy device fail!\n"));
		goto LabelErrReg;
	} /* End of if */

	return pWdev;

 LabelErrReg:
	wiphy_free(pWdev->wiphy);

 LabelErrWiphyNew:
	os_free_mem(NULL, pWdev);

	return NULL;
} /* End of CFG80211_WdevAlloc */




/* =========================== Global Function ============================== */

/*
========================================================================
Routine Description:
	Register MAC80211 Module.

Arguments:
	pAdCB			- WLAN control block pointer
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
VOID CFG80211_Register(
	IN VOID 				*pAdCB,
	IN struct device		*pDev,
	IN struct net_device	*pNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	/* allocate MAC80211 structure */
	if (pAd->pCfg80211_CB != NULL)
		return;
	/* End of if */

	os_alloc_mem(pAd, (UCHAR **)&pAd->pCfg80211_CB, sizeof(CFG80211_CB));
	if (pAd->pCfg80211_CB == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("80211> Allocate MAC80211 CB fail!\n"));
		return;
	} /* End of if */

	/* allocate wireless device */
	CFG80211CB->pCfg80211_Wdev = CFG80211_WdevAlloc(pAd, pDev);
	if (CFG80211CB->pCfg80211_Wdev == NULL)
	{
		os_free_mem(NULL, pAd->pCfg80211_CB);
		pAd->pCfg80211_CB = NULL;
		return;
	} /* End of if */

	/* bind wireless device with net device */
#ifdef CONFIG_AP_SUPPORT
	/* default we are AP mode */
	CFG80211CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_AP;
#endif // CONFIG_AP_SUPPORT //


	pNetDev->ieee80211_ptr = CFG80211CB->pCfg80211_Wdev;
	SET_NETDEV_DEV(pNetDev, wiphy_dev(CFG80211CB->pCfg80211_Wdev->wiphy));
	CFG80211CB->pCfg80211_Wdev->netdev = pNetDev;

	/* init API function pointers */
	RT_CFG80211_API_INIT(pAd);

#ifdef RFKILL_HW_SUPPORT
	wiphy_rfkill_start_polling(CFG80211CB->pCfg80211_Wdev->wiphy);
#endif // RFKILL_HW_SUPPORT //

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_Register\n"));
} /* End of CFG80211_Register */


/*
========================================================================
Routine Description:
	UnRegister MAC80211 Module.

Arguments:
	pAd				- WLAN control block pointer
	pNetDev			- Network device

Return Value:
	NONE

Note:
========================================================================
*/
VOID CFG80211_UnRegister(
	IN PRTMP_ADAPTER 		pAd,
	IN struct net_device	*pNetDev)
{

	/* sanity check */
	if (pAd->pCfg80211_CB == NULL)
		return;
	/* End of if */

	/* unregister */
	if (CFG80211CB->pCfg80211_Wdev != NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("80211> unregister/free wireless device\n"));

		/*
			Must unregister, or you will suffer problem when you change
			regulatory domain by using iw.
		*/
		
#ifdef RFKILL_HW_SUPPORT
		wiphy_rfkill_stop_polling(CFG80211CB->pCfg80211_Wdev->wiphy);
#endif // RFKILL_HW_SUPPORT //
		wiphy_unregister(CFG80211CB->pCfg80211_Wdev->wiphy);
		wiphy_free(CFG80211CB->pCfg80211_Wdev->wiphy);
		os_free_mem(NULL, CFG80211CB->pCfg80211_Wdev);

		if (CFG80211CB->pCfg80211_Channels != NULL)
			os_free_mem(NULL, CFG80211CB->pCfg80211_Channels);
		/* End of if */

		if (CFG80211CB->pCfg80211_Rates != NULL)
			os_free_mem(NULL, CFG80211CB->pCfg80211_Rates);
		/* End of if */

		CFG80211CB->pCfg80211_Wdev = NULL;
		CFG80211CB->pCfg80211_Channels = NULL;
		CFG80211CB->pCfg80211_Rates = NULL;

		/* must reset to NULL; or kernel will panic in unregister_netdev */
		pNetDev->ieee80211_ptr = NULL;
		SET_NETDEV_DEV(pNetDev, NULL);
	} /* End of if */

	os_free_mem(NULL, pAd->pCfg80211_CB);
	pAd->pCfg80211_CB = NULL;
	pAd->CommonCfg.HT_Disable = 0;
} /* End of CFG80211_UnRegister */


/*
========================================================================
Routine Description:
	Parse and handle country region in beacon from associated AP.

Arguments:
	pAdCB			- WLAN control block pointer
	pVIE			- Beacon elements
	LenVIE			- Total length of Beacon elements

Return Value:
	NONE

Note:
========================================================================
*/
VOID CFG80211_BeaconCountryRegionParse(
	IN VOID						*pAdCB,
	IN NDIS_802_11_VARIABLE_IEs	*pVIE,
	IN UINT16					LenVIE)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	UCHAR *pElement = (UCHAR *)pVIE;
	UINT32 LenEmt;


	while(LenVIE > 0)
	{
		pVIE = (NDIS_802_11_VARIABLE_IEs *)pElement;

		if (pVIE->ElementID == IE_COUNTRY)
		{
			/* send command to do regulation hint only when associated */
			RTEnqueueInternalCmd(pAd, CMDTHREAD_REG_HINT_11D,
								pVIE->data, pVIE->Length);
			break;
		} /* End of if */

		LenEmt = pVIE->Length + 2;

		if (LenVIE <= LenEmt)
			break; /* length is not enough */
		/* End of if */

		pElement += LenEmt;
		LenVIE -= LenEmt;
	} /* End of while */
} /* End of CFG80211_BeaconCountryRegionParse */


/*
========================================================================
Routine Description:
	Hint to the wireless core a regulatory domain from driver.

Arguments:
	pAd				- WLAN control block pointer
	pCountryIe		- pointer to the country IE
	CountryIeLen	- length of the country IE

Return Value:
	NONE

Note:
	Must call the function in kernel thread.
========================================================================
*/
VOID CFG80211_RegHint(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	CFG80211DBG(RT_DEBUG_ERROR,
			("crda> regulatory domain hint: %c%c\n",
			pCountryIe[0], pCountryIe[1]));

	if ((CFG80211CB->pCfg80211_Wdev == NULL) || (pCountryIe == NULL))
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("crda> regulatory domain hint not support!\n"));
		return;
	} /* End of if */

	/* hints a country IE as a regulatory domain "without" channel/power info. */
//	regulatory_hint(CFG80211CB->pMac80211_Hw->wiphy, pCountryIe);
	regulatory_hint(CFG80211CB->pCfg80211_Wdev->wiphy, (const char *)pCountryIe);
} /* End of CFG80211_RegHint */


/*
========================================================================
Routine Description:
	Hint to the wireless core a regulatory domain from country element.

Arguments:
	pAdCB			- WLAN control block pointer
	pCountryIe		- pointer to the country IE
	CountryIeLen	- length of the country IE

Return Value:
	NONE

Note:
	Must call the function in kernel thread.
========================================================================
*/
VOID CFG80211_RegHint11D(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	/* no regulatory_hint_11d() in 2.6.32 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	if ((CFG80211CB->pCfg80211_Wdev == NULL) || (pCountryIe == NULL))
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("crda> regulatory domain hint not support!\n"));
		return;
	} /* End of if */

	CFG80211DBG(RT_DEBUG_ERROR,
				("crda> regulatory domain hint: %c%c\n",
				pCountryIe[0], pCountryIe[1]));

	/*
		hints a country IE as a regulatory domain "with" channel/power info.
		but if you use regulatory_hint(), it only hint "regulatory domain".
	*/
//	regulatory_hint_11d(CFG80211CB->pMac80211_Hw->wiphy, pCountryIe, CountryIeLen);
	regulatory_hint_11d(CFG80211CB->pCfg80211_Wdev->wiphy, pCountryIe, CountryIeLen);
#endif // LINUX_VERSION_CODE //
} /* End of CFG80211_RegHint11D */


/*
========================================================================
Routine Description:
	Apply new regulatory rule.

Arguments:
	pAdCB			- WLAN control block pointer
	pWiphy			- Wireless hardware description
	pAlpha2			- Regulation domain (2B)

Return Value:
	NONE

Note:
	Can only be called when interface is up.

	For general mac80211 device, it will be set to new power by Ops->config()
	In rt2x00/, the settings is done in rt2x00lib_config().
========================================================================
*/
VOID CFG80211_RegRuleApply(
	IN VOID							*pAdCB,
	IN struct wiphy					*pWiphy,
	IN UCHAR						*pAlpha2)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	enum ieee80211_band IdBand;
	struct ieee80211_supported_band *pSband;
	struct ieee80211_channel *pChan;
	RADAR_DETECT_STRUCT	*pRadarDetect;
	UINT32 IdChan, IdPwr;
	UINT32 ChanId, RecId, DfsType;
	ULONG IrqFlags;


	CFG80211DBG(RT_DEBUG_ERROR, ("crda> CFG80211_RegRuleApply ==>\n"));

	/* sanity check */
	if (pWiphy == NULL)
	{
		if ((CFG80211CB != NULL) && (CFG80211CB->pCfg80211_Wdev != NULL))
			pWiphy = CFG80211CB->pCfg80211_Wdev->wiphy;
		/* End of if */
	} /* End of if */

	if ((pWiphy == NULL) || (pAlpha2 == NULL))
		return;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

	/* zero first */
	NdisZeroMemory(pAd->ChannelList,
					MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

	/* 2.4GHZ & 5GHz */
	RecId = 0;
	pRadarDetect = &pAd->CommonCfg.RadarDetect;

	/* find the DfsType */
	DfsType = CE;

#ifdef EXT_BUILD_CHANNEL_LIST
	if ((pAlpha2[0] != '0') && (pAlpha2[1] != '0'))
	{
		UINT32 IdReg;

		if (pWiphy->bands[IEEE80211_BAND_5GHZ] != NULL)
		{
			for(IdReg=0; ; IdReg++)
			{
				if (ChRegion[IdReg].CountReg[0] == 0x00)
					break;
				/* End of if */
	
				if ((pAlpha2[0] == ChRegion[IdReg].CountReg[0]) &&
					(pAlpha2[1] == ChRegion[IdReg].CountReg[1]))
				{
					DfsType = ChRegion[IdReg].DfsType;
	
					CFG80211DBG(RT_DEBUG_ERROR,
								("crda> find region %c%c, DFS Type %d\n",
								pAlpha2[0], pAlpha2[1], DfsType));
					break;
				} /* End of if */
			} /* End of for */
		} /* End of if */
	} /* End of if */
#endif // EXT_BUILD_CHANNEL_LIST //

	for(IdBand=0; IdBand<IEEE80211_NUM_BANDS; IdBand++)
	{
		if (!pWiphy->bands[IdBand])
			continue;
		/* End of if */

		if (IdBand == IEEE80211_BAND_2GHZ)
		{
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> reset chan/power for 2.4GHz\n"));
		}
		else
		{
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> reset chan/power for 5GHz\n"));
		} /* End of if */

		pSband = pWiphy->bands[IdBand];

		for(IdChan=0; IdChan<pSband->n_channels; IdChan++)
		{
			pChan = &pSband->channels[IdChan];
			ChanId = ieee80211_frequency_to_channel(pChan->center_freq);

			if ((pAd->CommonCfg.PhyMode == PHY_11A) ||
				(pAd->CommonCfg.PhyMode == PHY_11AN_MIXED))
			{
				/* 5G-only mode */
				if (ChanId <= CFG80211_NUM_OF_CHAN_2GHZ)
					continue; /* check next */
				/* End of if */
			} /* End of if */

			if ((pAd->CommonCfg.PhyMode != PHY_11A) &&
				(pAd->CommonCfg.PhyMode != PHY_11ABG_MIXED) &&
				(pAd->CommonCfg.PhyMode != PHY_11AN_MIXED) &&
				(pAd->CommonCfg.PhyMode != PHY_11ABGN_MIXED) &&
				(pAd->CommonCfg.PhyMode != PHY_11AGN_MIXED))
			{
				/* 2.5G-only mode */
				if (ChanId > CFG80211_NUM_OF_CHAN_2GHZ)
					continue; /* check next */
				/* End of if */
			} /* End of if */

			if (pChan->flags & IEEE80211_CHAN_DISABLED)
			{
				/* the channel is not allowed in the regulatory domain */
				CFG80211DBG(RT_DEBUG_ERROR,
							("Chan %03d (frq %d):\tnot allowed!\n",
							ChanId, pChan->center_freq));

				/* get next channel information */
				continue;
			} /* End of if */

			for(IdPwr=0; IdPwr<MAX_NUM_OF_CHANNELS; IdPwr++)
			{
				if (ChanId == pAd->TxPower[IdPwr].Channel)
				{
					/* init the channel info. */
					NdisMoveMemory(&pAd->ChannelList[RecId],
									&pAd->TxPower[IdPwr],
									sizeof(CHANNEL_TX_POWER));

					/* keep channel number */
					pAd->ChannelList[RecId].Channel = ChanId;

					/* keep maximum tranmission power */
					pAd->ChannelList[RecId].MaxTxPwr = pChan->max_power;

					/* keep DFS flag */
					if (pChan->flags & IEEE80211_CHAN_RADAR)
						pAd->ChannelList[RecId].DfsReq = TRUE;
					else
						pAd->ChannelList[RecId].DfsReq = FALSE;
					/* End of if */

					/* keep DFS type */
					pAd->ChannelList[RecId].RegulatoryDomain = DfsType;

					/* re-set DFS info. */
					pRadarDetect->RDDurRegion = DfsType;

					if (DfsType == JAP_W53)
						pRadarDetect->DfsSessionTime = 15;
					else if (DfsType == JAP_W56)
						pRadarDetect->DfsSessionTime = 13;
					else if (DfsType == JAP)
						pRadarDetect->DfsSessionTime = 5;
					else if (DfsType == FCC)
					{
						pRadarDetect->DfsSessionTime = 5;
					}
					else if (DfsType == CE)
						pRadarDetect->DfsSessionTime = 13;
					else
						pRadarDetect->DfsSessionTime = 13;
					/* End of if */

					CFG80211DBG(RT_DEBUG_ERROR,
								("Chan %03d (frq %d):\tpower %d dBm, "
								"DFS %d, DFS Type %d\n",
								ChanId, pChan->center_freq, pChan->max_power,
								((pChan->flags & IEEE80211_CHAN_RADAR)?1:0),
								DfsType));

					/* change to record next channel info. */
					RecId ++;
					break;
				} /* End of if */
			} /* End of for */
		} /* End of for */
	} /* End of for */

	pAd->ChannelListNum = RecId;
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	CFG80211DBG(RT_DEBUG_ERROR, ("crda> Number of channels = %d\n", RecId));
} /* End of CFG80211_RegRuleApply */


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
	IN INT32						RSSI,
	IN INT32						MemFlag)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#endif // LINUX_VERSION_CODE //
} /* End of CFG80211_Scaning */


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
	IN VOID							*pAdCB,
	IN BOOLEAN						FlgIsAborted)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#endif // LINUX_VERSION_CODE //
} /* End of CFG80211_ScanEnd */


/*
========================================================================
Routine Description:
	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.

Arguments:
	pAdCB			- WLAN control block pointer

Return Value:
	TRUE			- re-init successfully
	FALSE			- re-init fail

Note:
	CFG80211_SupBandInit() is called in xx_probe().
	But we do not have complete chip information in xx_probe() so we
	need to re-init bands in xx_open().
========================================================================
*/
BOOLEAN CFG80211_SupBandReInit(
	IN VOID							*pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	struct wiphy *pWiphy;


	/* sanity check */
	if (CFG80211CB->pCfg80211_Wdev == NULL)
		return FALSE;
	/* End of if */

	pWiphy = CFG80211CB->pCfg80211_Wdev->wiphy;

	if (pWiphy != NULL)
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> re-init bands...\n"));

		/* re-init bands */
		CFG80211_SupBandInit(pAd, pWiphy,
							CFG80211CB->pCfg80211_Channels,
							CFG80211CB->pCfg80211_Rates);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
		{
			UINT32 Value = 0;

			/* re-init PHY */
			pWiphy->rts_threshold = pAd->CommonCfg.RtsThreshold;
			pWiphy->frag_threshold = pAd->CommonCfg.FragmentThreshold;
	
			RTMP_IO_READ32(pAd, TX_RTY_CFG, &Value);
			pWiphy->retry_short = Value & 0xff;
			pWiphy->retry_long = (Value & 0xff00)>>8;
		}
#endif // LINUX_VERSION_CODE //

		return TRUE;
	} /* End of if */

	return FALSE;
} /* End of CFG80211_SupBandReInit */


/*
========================================================================
Routine Description:
	Inform CFG80211 about association status.

Arguments:
	pAdCB			- WLAN control block pointer
	pBSSID			- the BSSID of the AP
	pReqIe			- the element list in the association request frame
	ReqIeLen		- the request element length
	pRspIe			- the element list in the association response frame
	RspIeLen		- the response element length
	FlgIsSuccess	- 1: success; otherwise: fail

Return Value:
	None

Note:
========================================================================
*/
void CFG80211_ConnectResultInform(
	IN VOID							*pAdCB,
	IN UCHAR						*pBSSID,
	IN UCHAR						*pReqIe,
	IN UINT32						ReqIeLen,
	IN UCHAR						*pRspIe,
	IN UINT32						RspIeLen,
	IN UCHAR						FlgIsSuccess)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_ConnectResultInform ==>\n"));

	if ((CFG80211CB->pCfg80211_Wdev->netdev == NULL) || (pBSSID == NULL))
		return;
	/* End of if */

	if (FlgIsSuccess)
	{
		cfg80211_connect_result(CFG80211CB->pCfg80211_Wdev->netdev,
								pBSSID,
								pReqIe,
								ReqIeLen,
								pRspIe,
								RspIeLen,
								WLAN_STATUS_SUCCESS,
								GFP_KERNEL);
	}
	else
	{
		cfg80211_connect_result(CFG80211CB->pCfg80211_Wdev->netdev,
								pBSSID,
								NULL, 0, NULL, 0,
								WLAN_STATUS_UNSPECIFIED_FAILURE,
								GFP_KERNEL);
	} /* End of if */

	pAd->FlgCfg80211Connecting = FALSE;
#endif // LINUX_VERSION_CODE //
} /* End of CFG80211_ConnectResultInform */




/* =========================== Local Function =============================== */

/*
========================================================================
Routine Description:
	The driver's regulatory notification callback.

Arguments:
	pWiphy			- Wireless hardware description
	pRequest		- Regulatory request

Return Value:
	0

Note:
========================================================================
*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static INT32 CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN struct regulatory_request	*pRequest)
{
	PRTMP_ADAPTER pAd;
	ULONG *pPriv;


	/* sanity check */
	pPriv = (ULONG *)(wiphy_priv(pWiphy));
	pAd = (PRTMP_ADAPTER)(*pPriv);

	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("crda> reg notify but pAd = NULL!"));
		return 0;
	} /* End of if */

	/*
		Change the band settings (PASS scan, IBSS allow, or DFS) in mac80211
		based on EEPROM.

		IEEE80211_CHAN_DISABLED: This channel is disabled.
		IEEE80211_CHAN_PASSIVE_SCAN: Only passive scanning is permitted
					on this channel.
		IEEE80211_CHAN_NO_IBSS: IBSS is not allowed on this channel.
		IEEE80211_CHAN_RADAR: Radar detection is required on this channel.
		IEEE80211_CHAN_NO_FAT_ABOVE: extension channel above this channel
					is not permitted.
		IEEE80211_CHAN_NO_FAT_BELOW: extension channel below this channel
					is not permitted.
	*/

	/*
		Change regulatory rule here.

		struct ieee80211_channel {
			enum ieee80211_band band;
			u16 center_freq;
			u8 max_bandwidth;
			u16 hw_value;
			u32 flags;
			int max_antenna_gain;
			int max_power;
			bool beacon_found;
			u32 orig_flags;
			int orig_mag, orig_mpwr;
		};

		In mac80211 layer, it will change flags, max_antenna_gain,
		max_bandwidth, max_power.
	*/

	switch(pRequest->initiator)
	{
		case NL80211_REGDOM_SET_BY_CORE:
			/*
				Core queried CRDA for a dynamic world regulatory domain.
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by core: "));
			break;

		case NL80211_REGDOM_SET_BY_USER:
			/*
				User asked the wireless core to set the regulatory domain.
				(when iw, network manager, wpa supplicant, etc.)
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by user: "));
			break;

		case NL80211_REGDOM_SET_BY_DRIVER:
			/*
				A wireless drivers has hinted to the wireless core it thinks
				its knows the regulatory domain we should be in.
				(when driver initialization, calling regulatory_hint)
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by driver: "));
			break;

		case NL80211_REGDOM_SET_BY_COUNTRY_IE:
			/*
				The wireless core has received an 802.11 country information
				element with regulatory information it thinks we should consider.
				(when beacon receive, calling regulatory_hint_11d)
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by country IE: "));
			break;
	} /* End of switch */

	CFG80211DBG(RT_DEBUG_ERROR,
				("%c%c\n", pRequest->alpha2[0], pRequest->alpha2[1]));

	/* only follow rules from user */
	if (pRequest->initiator == NL80211_REGDOM_SET_BY_USER)
	{
		/* keep Alpha2 and we can re-call the function when interface is up */
		pAd->Cfg80211_Alpha2[0] = pRequest->alpha2[0];
		pAd->Cfg80211_Alpha2[1] = pRequest->alpha2[1];

		/* apply the new regulatory rule */
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		{
			/* interface is up */
			CFG80211_RegRuleApply(pAd, pWiphy, (UCHAR *)pRequest->alpha2);
		}
		else
		{
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> interface is down!\n"));
		} /* End of if */
	} /* End of if */

	return 0;
} /* End of CFG80211_RegNotifier */
#else

static INT32 CFG80211_RegNotifier(
	IN struct wiphy					*pWiphy,
	IN enum reg_set_by				Request)
{
	struct device *pDev = pWiphy->dev.parent;
	struct net_device *pNetDev = dev_get_drvdata(pDev);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)RTMP_OS_NETDEV_GET_PRIV(pNetDev);
	UINT32 ReqType = Request;


	/* sanity check */
	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("crda> reg notify but pAd = NULL!"));
		return 0;
	} /* End of if */

	/*
		Change the band settings (PASS scan, IBSS allow, or DFS) in mac80211
		based on EEPROM.

		IEEE80211_CHAN_DISABLED: This channel is disabled.
		IEEE80211_CHAN_PASSIVE_SCAN: Only passive scanning is permitted
					on this channel.
		IEEE80211_CHAN_NO_IBSS: IBSS is not allowed on this channel.
		IEEE80211_CHAN_RADAR: Radar detection is required on this channel.
		IEEE80211_CHAN_NO_FAT_ABOVE: extension channel above this channel
					is not permitted.
		IEEE80211_CHAN_NO_FAT_BELOW: extension channel below this channel
					is not permitted.
	*/

	/*
		Change regulatory rule here.

		struct ieee80211_channel {
			enum ieee80211_band band;
			u16 center_freq;
			u8 max_bandwidth;
			u16 hw_value;
			u32 flags;
			int max_antenna_gain;
			int max_power;
			bool beacon_found;
			u32 orig_flags;
			int orig_mag, orig_mpwr;
		};

		In mac80211 layer, it will change flags, max_antenna_gain,
		max_bandwidth, max_power.
	*/

	switch(ReqType)
	{
		case REGDOM_SET_BY_CORE:
			/*
				Core queried CRDA for a dynamic world regulatory domain.
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by core: "));
			break;

		case REGDOM_SET_BY_USER:
			/*
				User asked the wireless core to set the regulatory domain.
				(when iw, network manager, wpa supplicant, etc.)
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by user: "));
			break;

		case REGDOM_SET_BY_DRIVER:
			/*
				A wireless drivers has hinted to the wireless core it thinks
				its knows the regulatory domain we should be in.
				(when driver initialization, calling regulatory_hint)
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by driver: "));
			break;

		case REGDOM_SET_BY_COUNTRY_IE:
			/*
				The wireless core has received an 802.11 country information
				element with regulatory information it thinks we should consider.
				(when beacon receive, calling regulatory_hint_11d)
			*/
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> requlation requestion by country IE: "));
			break;
	} /* End of switch */

	DBGPRINT(RT_DEBUG_ERROR, ("00\n"));

	/* only follow rules from user */
	if (ReqType == REGDOM_SET_BY_USER)
	{
		/* keep Alpha2 and we can re-call the function when interface is up */
		pAd->Cfg80211_Alpha2[0] = '0';
		pAd->Cfg80211_Alpha2[1] = '0';

		/* apply the new regulatory rule */
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		{
			/* interface is up */
			CFG80211_RegRuleApply(pAd, pWiphy, pAd->Cfg80211_Alpha2);
		}
		else
		{
			CFG80211DBG(RT_DEBUG_ERROR, ("crda> interface is down!\n"));
		} /* End of if */
	} /* End of if */

	return 0;
} /* End of CFG80211_RegNotifier */
#endif // LINUX_VERSION_CODE //


/*
========================================================================
Routine Description:
	Initialize wireless channel in 2.4GHZ and 5GHZ.

Arguments:
	pAd				- WLAN control block pointer
	pWiphy			- WLAN PHY interface
	pChannels		- Current channel info
	pRates			- Current rate info

Return Value:
	TRUE			- init successfully
	FALSE			- init fail

Note:
	TX Power related:

	1. Suppose we can send power to 15dBm in the board.
	2. A value 0x0 ~ 0x1F for a channel. We will adjust it based on 15dBm/
		54Mbps. So if value == 0x07, the TX power of 54Mbps is 15dBm and
		the value is 0x07 in the EEPROM.
	3. Based on TX power value of 54Mbps/channel, adjust another value
		0x0 ~ 0xF for other data rate. (-6dBm ~ +6dBm)

	Other related factors:
	1. TX power percentage from UI/users;
	2. Maximum TX power limitation in the regulatory domain.
========================================================================
*/
static BOOLEAN CFG80211_SupBandInit(
	IN PRTMP_ADAPTER 				pAd,
	IN struct wiphy					*pWiphy,
	IN struct ieee80211_channel		*pChannels,
	IN struct ieee80211_rate		*pRates)
{
	struct ieee80211_supported_band *pBand;
	UINT32 NumOfChan, NumOfRate;
	UINT32 IdLoop;
	UINT32 CurTxPower;


	/* sanity check */
	if (pAd->RFICType == 0)
		pAd->RFICType = RFIC_24GHZ | RFIC_5GHZ;
	/* End of if */

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> RFICType = %d\n", pAd->RFICType));

	/* init */
	if (pAd->RFICType & RFIC_5GHZ)
		NumOfChan = CFG80211_NUM_OF_CHAN_2GHZ + CFG80211_NUM_OF_CHAN_5GHZ;
	else
		NumOfChan = CFG80211_NUM_OF_CHAN_2GHZ;
	/* End of if */

	if (pAd->CommonCfg.PhyMode == PHY_11B)
		NumOfRate = 4;
	else
		NumOfRate = 4 + 8;
	/* End of if */

	if (pChannels == NULL)
	{
		pChannels = kzalloc(sizeof(*pChannels) * NumOfChan, GFP_KERNEL);
		if (!pChannels)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("80211> ieee80211_channel allocation fail!\n"));
			return FALSE;
		} /* End of if */
	} /* End of if */

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> Number of channel = %d\n",
				CFG80211_NUM_OF_CHAN_5GHZ));

	if (pRates == NULL)
	{
		pRates = kzalloc(sizeof(*pRates) * NumOfRate, GFP_KERNEL);
		if (!pRates)
		{
			os_free_mem(NULL, pChannels);
			DBGPRINT(RT_DEBUG_ERROR, ("80211> ieee80211_rate allocation fail!\n"));
			return FALSE;
		} /* End of if */
	} /* End of if */

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> Number of rate = %d\n", NumOfRate));

	/* get TX power */
#ifdef SINGLE_SKU
	CurTxPower = pAd->CommonCfg.DefineMaxTxPwr; /* dBm */
#else
	CurTxPower = 0; /* unknown */
#endif // SINGLE_SKU //

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CurTxPower = %d dBm\n", CurTxPower));

	/* init channel */
	for(IdLoop=0; IdLoop<NumOfChan; IdLoop++)
	{
		pChannels[IdLoop].center_freq = \
					ieee80211_channel_to_frequency(Cfg80211_Chan[IdLoop]);
		pChannels[IdLoop].hw_value = IdLoop;

		if (IdLoop < CFG80211_NUM_OF_CHAN_2GHZ)
			pChannels[IdLoop].max_power = CurTxPower;
		else
			pChannels[IdLoop].max_power = CurTxPower;
		/* End of if */

		pChannels[IdLoop].max_antenna_gain = 0xff;
	} /* End of if */

	/* init rate */
	for(IdLoop=0; IdLoop<NumOfRate; IdLoop++)
		memcpy(&pRates[IdLoop], &Cfg80211_SupRate[IdLoop], sizeof(*pRates));
	/* End of for */

	pBand = &CFG80211CB->Cfg80211_bands[IEEE80211_BAND_2GHZ];
	if (pAd->RFICType & RFIC_24GHZ)
	{
		pBand->n_channels = CFG80211_NUM_OF_CHAN_2GHZ;
		pBand->n_bitrates = NumOfRate;
		pBand->channels = pChannels;
		pBand->bitrates = pRates;

#ifdef DOT11_N_SUPPORT
		/* for HT, assign pBand->ht_cap */
		pBand->ht_cap.ht_supported = true;
		pBand->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
					       IEEE80211_HT_CAP_SM_PS |
					       IEEE80211_HT_CAP_SGI_40 |
					       IEEE80211_HT_CAP_DSSSCCK40;
		pBand->ht_cap.ampdu_factor = 3; /* 2 ^ 16 */
		pBand->ht_cap.ampdu_density = \
						pAd->CommonCfg.BACapability.field.MpduDensity;

		memset(&pBand->ht_cap.mcs, 0, sizeof(pBand->ht_cap.mcs));
		CFG80211DBG(RT_DEBUG_ERROR,
					("80211> TxStream = %d\n", pAd->CommonCfg.TxStream));

		switch(pAd->CommonCfg.TxStream)
		{
			case 1:
			default:
				pBand->ht_cap.mcs.rx_mask[0] = 0xff;
				break;

			case 2:
				pBand->ht_cap.mcs.rx_mask[0] = 0xff;
				pBand->ht_cap.mcs.rx_mask[1] = 0xff;
				break;

			case 3:
				pBand->ht_cap.mcs.rx_mask[0] = 0xff;
				pBand->ht_cap.mcs.rx_mask[1] = 0xff;
				pBand->ht_cap.mcs.rx_mask[2] = 0xff;
				break;
		} /* End of switch */

		pBand->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
#endif // DOT11_N_SUPPORT //

		pWiphy->bands[IEEE80211_BAND_2GHZ] = pBand;
	}
	else
	{
		pWiphy->bands[IEEE80211_BAND_2GHZ] = NULL;
		pBand->channels = NULL;
		pBand->bitrates = NULL;
	} /* End of if */

	pBand = &CFG80211CB->Cfg80211_bands[IEEE80211_BAND_5GHZ];
	if (pAd->RFICType & RFIC_5GHZ)
	{
		pBand->n_channels = CFG80211_NUM_OF_CHAN_5GHZ;
		pBand->n_bitrates = NumOfRate - 4;
		pBand->channels = &pChannels[CFG80211_NUM_OF_CHAN_2GHZ];
		pBand->bitrates = &pRates[4];

		/* for HT, assign pBand->ht_cap */
#ifdef DOT11_N_SUPPORT
		/* for HT, assign pBand->ht_cap */
		pBand->ht_cap.ht_supported = true;
		pBand->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
					       IEEE80211_HT_CAP_SM_PS |
					       IEEE80211_HT_CAP_SGI_40 |
					       IEEE80211_HT_CAP_DSSSCCK40;
		pBand->ht_cap.ampdu_factor = 3; /* 2 ^ 16 */
		pBand->ht_cap.ampdu_density = \
						pAd->CommonCfg.BACapability.field.MpduDensity;

		memset(&pBand->ht_cap.mcs, 0, sizeof(pBand->ht_cap.mcs));
		switch(pAd->CommonCfg.RxStream)
		{
			case 1:
			default:
				pBand->ht_cap.mcs.rx_mask[0] = 0xff;
				break;

			case 2:
				pBand->ht_cap.mcs.rx_mask[0] = 0xff;
				pBand->ht_cap.mcs.rx_mask[1] = 0xff;
				break;

			case 3:
				pBand->ht_cap.mcs.rx_mask[0] = 0xff;
				pBand->ht_cap.mcs.rx_mask[1] = 0xff;
				pBand->ht_cap.mcs.rx_mask[2] = 0xff;
				break;
		} /* End of switch */

		pBand->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
#endif // DOT11_N_SUPPORT //

		pWiphy->bands[IEEE80211_BAND_5GHZ] = pBand;
	}
	else
	{
		pWiphy->bands[IEEE80211_BAND_5GHZ] = NULL;
		pBand->channels = NULL;
		pBand->bitrates = NULL;
	} /* End of if */

	CFG80211CB->pCfg80211_Channels = pChannels;
	CFG80211CB->pCfg80211_Rates = pRates;

	return TRUE;
} /* End of CFG80211_SupBandInit */


#endif // RT_CFG80211_SUPPORT //
#endif // LINUX_VERSION_CODE //

/* End of crda.c */
