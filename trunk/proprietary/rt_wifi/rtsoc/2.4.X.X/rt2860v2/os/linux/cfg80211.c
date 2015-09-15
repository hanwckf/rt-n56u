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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
static int CFG80211_OpsSetChannel(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pDev,
	IN struct ieee80211_channel		*pChan,
	IN enum nl80211_channel_type	ChannelType)

#else
static int CFG80211_OpsSetChannel(
	IN struct wiphy					*pWiphy,
	IN struct ieee80211_channel		*pChan,
	IN enum nl80211_channel_type	ChannelType)
#endif /* LINUX_VERSION_CODE */
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

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT
	if ((CFG80211CB->pCfg80211_Wdev->iftype == NL80211_IFTYPE_STATION) &&
		(FlgIsChanged == TRUE))
	{
		/*
			1. Station mode;
			2. New BW settings is 20MHz but current BW is not 20MHz;
			3. New BW settings is 40MHz but current BW is 20MHz;

			Re-connect to the AP due to BW 20/40 or HT/non-HT change.
		*/
		Set_SSID_Proc(pAd, (PSTRING)pAd->CommonCfg.Ssid);
	} /* End of if */
#endif // DOT11_N_SUPPORT //

	if (CFG80211CB->pCfg80211_Wdev->iftype == NL80211_IFTYPE_ADHOC)
	{
		/* update IBSS beacon */
		MlmeUpdateTxRates(pAd, FALSE, 0);
		MakeIbssBeacon(pAd);
		AsicEnableIbssSync(pAd);

		Set_SSID_Proc(pAd, (PSTRING)pAd->CommonCfg.Ssid);
	} /* End of if */

	if (CFG80211CB->pCfg80211_Wdev->iftype == NL80211_IFTYPE_MONITOR)
	{
		/* reset monitor mode in the new channel */
		Set_NetworkType_Proc(pAd, "Monitor");
		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, CFG80211CB->MonFilterFlag);
	} /* End of if */
#endif // CONFIG_STA_SUPPORT //

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
#ifdef CONFIG_STA_SUPPORT
	if ((Type != NL80211_IFTYPE_ADHOC) &&
		(Type != NL80211_IFTYPE_STATION) &&
		(Type != NL80211_IFTYPE_MONITOR))
#endif // CONFIG_STA_SUPPORT //
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
#ifdef CONFIG_STA_SUPPORT
	if (Type == NL80211_IFTYPE_ADHOC)
		Set_NetworkType_Proc(pAd, "Adhoc");
	else if (Type == NL80211_IFTYPE_STATION)
		Set_NetworkType_Proc(pAd, "Infra");
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,20))
	else if (Type == NL80211_IFTYPE_MONITOR)
	{
		/* set packet filter */
		Set_NetworkType_Proc(pAd, "Monitor");

		if (pFlags != NULL)
		{
			UINT32 Filter;


			RTMP_IO_READ32(pAd, RX_FILTR_CFG, &Filter);

			if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_FCSFAIL)
				Filter = Filter & (~0x01);
			else
				Filter = Filter | 0x01;
			/* End of if */
	
			if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_PLCPFAIL)
				Filter = Filter & (~0x02);
			else
				Filter = Filter | 0x02;
			/* End of if */
	
			if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_CONTROL)
				Filter = Filter & (~0xFF00);
			else
				Filter = Filter | 0xFF00;
			/* End of if */
	
			if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_OTHER_BSS)
				Filter = Filter & (~0x08);
			else
				Filter = Filter | 0x08;
			/* End of if */

			RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, Filter);
			CFG80211CB->MonFilterFlag = Filter;
		} /* End of if */

		return 0; /* not need to set SSID */
	} /* End of if */
#endif // LINUX_VERSION_CODE //

	pAd->StaCfg.bAutoReconnect = TRUE;

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> SSID = %s\n", pAd->CommonCfg.Ssid));
	Set_SSID_Proc(pAd, (PSTRING)pAd->CommonCfg.Ssid);
#endif // CONFIG_STA_SUPPORT //

	return 0;
} /* End of CFG80211_OpsChgVirtualInf */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
	Request to do a scan. If returning zero, the scan request is given
	the driver, and will be valid until passed to cfg80211_scan_done().
	For scan results, call cfg80211_inform_bss(); you can call this outside
	the scan/scan_done bracket too.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pRequest		- Scan request

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: scan

	struct cfg80211_scan_request {
		struct cfg80211_ssid *ssids;
		int n_ssids;
		struct ieee80211_channel **channels;
		u32 n_channels;
		const u8 *ie;
		size_t ie_len;

	 * @ssids: SSIDs to scan for (active scan only)
	 * @n_ssids: number of SSIDs
	 * @channels: channels to scan on.
	 * @n_channels: number of channels for each band
	 * @ie: optional information element(s) to add into Probe Request or %NULL
	 * @ie_len: length of ie in octets
========================================================================
*/
static int CFG80211_OpsScan(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNdev,
	IN struct cfg80211_scan_request *pRequest)
{
	PRTMP_ADAPTER pAd;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_OpsScan ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	/* sanity check */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("80211> Network is down!\n"));
		return -ENETDOWN;
	} /* End of if */

	if ((pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_STATION) &&
		(pNdev->ieee80211_ptr->iftype != NL80211_IFTYPE_ADHOC))
	{
		return -EOPNOTSUPP;
	} /* End of if */

	if (pAd->FlgCfg80211Scanning == TRUE)
		return -EBUSY; /* scanning */
	/* End of if */

	/* do scan */
	pAd->FlgCfg80211Scanning = TRUE;
	CFG80211CB->pCfg80211_ScanReq = pRequest; /* used in scan end */

	rt_ioctl_siwscan(pNdev, NULL, NULL, NULL);
	return 0;
} /* End of CFG80211_OpsScan */
#endif // CONFIG_STA_SUPPORT //
#endif // LINUX_VERSION_CODE //


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
	Join the specified IBSS (or create if necessary). Once done, call
	cfg80211_ibss_joined(), also call that function when changing BSSID due
	to a merge.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pParams			- IBSS parameters

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: ibss join

	No fixed-freq and fixed-bssid support.
========================================================================
*/
static int CFG80211_OpsJoinIbss(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNdev,
	IN struct cfg80211_ibss_params	*pParams)
{
	PRTMP_ADAPTER pAd;

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_OpsJoinIbss ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> SSID = %s\n",
				pParams->ssid));
	CFG80211DBG(RT_DEBUG_ERROR, ("80211> Beacon Interval = %d\n",
				pParams->beacon_interval));

	pAd->StaCfg.bAutoReconnect = TRUE;

	pAd->CommonCfg.BeaconPeriod = pParams->beacon_interval;
	Set_SSID_Proc(pAd, (PSTRING)pParams->ssid);

	return 0;
} /* End of CFG80211_OpsJoinIbss */


/*
========================================================================
Routine Description:
	Leave the IBSS.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: ibss leave
========================================================================
*/
static int CFG80211_OpsLeaveIbss(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNdev)
{
	PRTMP_ADAPTER pAd;

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_OpsLeaveIbss ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	pAd->StaCfg.bAutoReconnect = FALSE;
	LinkDown(pAd, FALSE);

	return 0;
} /* End of CFG80211_OpsLeaveIbss */
#endif // CONFIG_STA_SUPPORT //
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
static int CFG80211_OpsTxPwrSet(
	IN struct wiphy						*pWiphy,
	IN enum nl80211_tx_power_setting	Type,
	IN int								dBm)
{
	CFG80211DBG(RT_DEBUG_ERROR, ("80211> %s ==>\n", __FUNCTION__));
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsTxPwrSet */

#else
static int CFG80211_OpsTxPwrSet(
	IN struct wiphy						*pWiphy,
	IN enum tx_power_setting			Type,
	IN int								dBm)
{
	CFG80211DBG(RT_DEBUG_ERROR, ("80211> %s ==>\n", __FUNCTION__));
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsTxPwrSet */
#endif /* LINUX_VERSION_CODE */


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
static int CFG80211_OpsTxPwrGet(
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
static int CFG80211_OpsPwrMgmt(
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
static int CFG80211_OpsStaGet(
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

#ifdef CONFIG_STA_SUPPORT
{
	HTTRANSMIT_SETTING PhyInfo;
	ULONG DataRate = 0;
	UINT32 RSSI;


	/* fill tx rate */
    if ((pAd->CommonCfg.PhyMode <= PHY_11G) ||
		(pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
	{
		PhyInfo.word = pAd->StaCfg.HTPhyMode.word;
	}
    else
		PhyInfo.word = pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word;
	/* End of if */

	getRate(PhyInfo, &DataRate);

	if ((PhyInfo.field.MODE == MODE_HTMIX) ||
		(PhyInfo.field.MODE == MODE_HTGREENFIELD))
	{
		pSinfo->txrate.flags = RATE_INFO_FLAGS_MCS;
		if (PhyInfo.field.BW)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
		/* End of if */
		if (PhyInfo.field.ShortGI)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
		/* End of if */

		pSinfo->txrate.mcs = PhyInfo.field.MCS;
	}
	else
	{
		pSinfo->txrate.legacy = DataRate*10; /* unit: 100kbps */
	} /* End of if */

	pSinfo->filled |= STATION_INFO_TX_BITRATE;

	/* fill signal */
	RSSI = (pAd->StaCfg.RssiSample.AvgRssi0 +
			pAd->StaCfg.RssiSample.AvgRssi1 +
			pAd->StaCfg.RssiSample.AvgRssi2) / 3;
	pSinfo->signal = le32_to_cpu(RSSI);
	pSinfo->filled |= STATION_INFO_SIGNAL;
}
#endif // CONFIG_STA_SUPPORT //

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
static int CFG80211_OpsStaDump(
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

#ifdef CONFIG_STA_SUPPORT
	if (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED)
	{
		memcpy(pMac, pAd->MlmeAux.Bssid, 6);
		return CFG80211_StaGet(pWiphy, pNdev, pMac, pSinfo);
	}
	else
		return -EBUSY;
	/* End of if */
#endif // CONFIG_STA_SUPPORT //

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
static int CFG80211_OpsWiphyParamsSet(
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
static int CFG80211_OpsKeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr,
	IN struct key_params				*pParams)
#else
static int CFG80211_OpsKeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr,
	IN struct key_params				*pParams)
#endif /* LINUX_VERSION_CODE */
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

#ifdef CONFIG_STA_SUPPORT
	switch(pParams->cipher)
	{
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
			switch(KeyIdx+1)
			{
				case 1:
				default:
					Set_Key1_Proc(pAd, (PSTRING)KeyBuf);
					break;
	
				case 2:
					Set_Key2_Proc(pAd, (PSTRING)KeyBuf);
					break;
	
				case 3:
					Set_Key3_Proc(pAd, (PSTRING)KeyBuf);
					break;
	
				case 4:
					Set_Key4_Proc(pAd, (PSTRING)KeyBuf);
					break;
			} /* End of switch */
			break;

		case WLAN_CIPHER_SUITE_TKIP:
		case WLAN_CIPHER_SUITE_CCMP:
			Set_WPAPSK_Proc(pAd, (PSTRING)KeyBuf);
			break;

		default:
			return -ENOTSUPP;
	} /* End of switch */

	return 0;
#endif // CONFIG_STA_SUPPORT //

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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
static int CFG80211_OpsKeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *cookie,
												 struct key_params *))
#else
static int CFG80211_OpsKeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *cookie,
												 struct key_params *))
#endif /* LINUX_VERSION_CODE */
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
static int CFG80211_OpsKeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr)
#else
static int CFG80211_OpsKeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr)
#endif /* LINUX_VERSION_CODE */
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int CFG80211_OpsKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Unicast,
	IN bool								Multicast)
#else
static int CFG80211_OpsKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx)
#endif /* LINUX_VERSION_CODE */
{
	PRTMP_ADAPTER pAd;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_KeyDefaultSet ==>\n"));
	MAC80211_PAD_GET(pAd, pWiphy);

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> KeyIdx = %d\n", KeyIdx));

#ifdef CONFIG_STA_SUPPORT
	pAd->StaCfg.DefaultKeyId = KeyIdx; /* base 0 */
#endif // CONFIG_STA_SUPPORT //
	return 0;
} /* End of CFG80211_KeyDefaultSet */


#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
	Connect to the ESS with the specified parameters. When connected,
	call cfg80211_connect_result() with status code %WLAN_STATUS_SUCCESS.
	If the connection fails for some reason, call cfg80211_connect_result()
	with the status from the AP.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pSme			- 

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: connect

	You must use "iw ra0 connect xxx", then "iw ra0 disconnect";
	You can not use "iw ra0 connect xxx" twice without disconnect;
	Or you will suffer "command failed: Operation already in progress (-114)".

	You must support add_key and set_default_key function;
	Or kernel will crash without any error message in linux 2.6.32.
========================================================================
*/
static int CFG80211_OpsConnect(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_connect_params	*pSme)
{
	PRTMP_ADAPTER pAd;
	struct ieee80211_channel *pChannel = pSme->channel;
	UCHAR SSID[NDIS_802_11_LENGTH_SSID];
	INT32 Pairwise = 0;
	INT32 Groupwise = 0;
	INT32 Keymgmt = 0;
	INT32 WpaVersion = NL80211_WPA_VERSION_2;
	INT32 Chan = -1, Idx, SSIDLen;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_Connect ==>\n"));

	/* init */
	MAC80211_PAD_GET(pAd, pWiphy);

	if (pChannel != NULL)
		Chan = ieee80211_frequency_to_channel(pChannel->center_freq);

	Groupwise = pSme->crypto.cipher_group;
	for(Idx=0; Idx<pSme->crypto.n_ciphers_pairwise; Idx++)
		Pairwise |= pSme->crypto.ciphers_pairwise[Idx];
	/* End of for */

	for(Idx=0; Idx<pSme->crypto.n_akm_suites; Idx++)
		Keymgmt |= pSme->crypto.akm_suites[Idx];
	/* End of for */

	WpaVersion = pSme->crypto.wpa_versions;

	/* change to infrastructure mode if we are in ADHOC mode */
	Set_NetworkType_Proc(pAd, "Infra");

	/* set authentication mode */
	if (WpaVersion & NL80211_WPA_VERSION_2)
	{
		if (Keymgmt & WLAN_AKM_SUITE_8021X)
			Set_AuthMode_Proc(pAd, "WPA2");
		else
			Set_AuthMode_Proc(pAd, "WPA2PSK");
		/* End of if */
	}
	else if (WpaVersion & NL80211_WPA_VERSION_1)
	{
		if (Keymgmt & WLAN_AKM_SUITE_8021X)
			Set_AuthMode_Proc(pAd, "WPA");
		else
			Set_AuthMode_Proc(pAd, "WPAPSK");
		/* End of if */
	}
	else if (pSme->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
		Set_AuthMode_Proc(pAd, "SHARED");
	else if (pSme->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
		Set_AuthMode_Proc(pAd, "OPEN");
	else
		Set_AuthMode_Proc(pAd, "OPEN");
	/* End of if */

	CFG80211DBG(RT_DEBUG_ERROR,
				("80211> SME %x, AuthMode = %d\n",
				pSme->auth_type, pAd->StaCfg.AuthMode));

	/* set encryption mode */
	if (Pairwise & WLAN_CIPHER_SUITE_CCMP)
		Set_EncrypType_Proc(pAd, "AES");
	else if (Pairwise & WLAN_CIPHER_SUITE_TKIP)
		Set_EncrypType_Proc(pAd, "TKIP");
	else if ((Pairwise & WLAN_CIPHER_SUITE_WEP40) ||
			(Pairwise & WLAN_CIPHER_SUITE_WEP104))
	{
		Set_EncrypType_Proc(pAd, "WEP");
	}
	else if (Groupwise & WLAN_CIPHER_SUITE_CCMP)
		Set_EncrypType_Proc(pAd, "AES");
	else if (Groupwise & WLAN_CIPHER_SUITE_TKIP)
		Set_EncrypType_Proc(pAd, "TKIP");
	else
		Set_EncrypType_Proc(pAd, "NONE");
	/* End of if */

	CFG80211DBG(RT_DEBUG_ERROR,
				("80211> EncrypType = %d\n", pAd->StaCfg.WepStatus));

	/* set channel: STATION will auto-scan */

	/* set WEP key */
	if (pSme->key && (((Groupwise | Pairwise) & WLAN_CIPHER_SUITE_WEP40) ||
		((Groupwise | Pairwise) & WLAN_CIPHER_SUITE_WEP104)))
	{
		UCHAR KeyBuf[50];

		/* reset AuthMode and EncrypType */
		Set_AuthMode_Proc(pAd, "SHARED");
		Set_EncrypType_Proc(pAd, "WEP");

		/* reset key */
#ifdef RT_CFG80211_DEBUG
		hex_dump("KeyBuf=", (UINT8 *)pSme->key, pSme->key_len);
#endif // RT_CFG80211_DEBUG //

		pAd->StaCfg.DefaultKeyId = pSme->key_idx; /* base 0 */
		if (pSme->key_len >= sizeof(KeyBuf))
			return -EINVAL;
		/* End of if */
		memcpy(KeyBuf, pSme->key, pSme->key_len);
		KeyBuf[pSme->key_len] = 0x00;

		CFG80211DBG(RT_DEBUG_ERROR,
					("80211> pAd->StaCfg.DefaultKeyId = %d\n",
					pAd->StaCfg.DefaultKeyId));

		switch(pSme->key_idx)
		{
			case 1:
			default:
				Set_Key1_Proc(pAd, (PSTRING)KeyBuf);
				break;

			case 2:
				Set_Key2_Proc(pAd, (PSTRING)KeyBuf);
				break;

			case 3:
				Set_Key3_Proc(pAd, (PSTRING)KeyBuf);
				break;

			case 4:
				Set_Key4_Proc(pAd, (PSTRING)KeyBuf);
				break;
		} /* End of switch */
	} /* End of if */

	/* TODO: We need to provide a command to set BSSID to associate a AP */

	/* re-set SSID */
	pAd->StaCfg.bAutoReconnect = TRUE;
	pAd->FlgCfg80211Connecting = TRUE;

	SSIDLen = pSme->ssid_len;
	if (SSIDLen > NDIS_802_11_LENGTH_SSID)
		SSIDLen = NDIS_802_11_LENGTH_SSID;
	/* End of if */

	memset(&SSID, 0, sizeof(SSID));
	memcpy(SSID, pSme->ssid, SSIDLen);
	Set_SSID_Proc(pAd, (PSTRING)SSID);

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> SSID = %s\n", SSID));
	return 0;
} /* End of CFG80211_Connect */


/*
========================================================================
Routine Description:
	Disconnect from the BSS/ESS.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	ReasonCode		- 

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: connect
========================================================================
*/
static int CFG80211_OpsDisconnect(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN u16								ReasonCode)
{
	PRTMP_ADAPTER pAd;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_Disconnect ==>\n"));
	CFG80211DBG(RT_DEBUG_ERROR, ("80211> ReasonCode = %d\n", ReasonCode));

	MAC80211_PAD_GET(pAd, pWiphy);

	pAd->StaCfg.bAutoReconnect = FALSE;
	pAd->FlgCfg80211Connecting = FALSE;
	LinkDown(pAd, FALSE);
	return 0;
} /* End of CFG80211_Disconnect */
#endif // CONFIG_STA_SUPPORT //
#endif // LINUX_VERSION_CODE //


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
/*
========================================================================
Routine Description:
	Get site survey information.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	Idx				-
	pSurvey			-

Return Value:
	0				- success
	-x				- fail

Note:
	For iw utility: survey dump
========================================================================
*/
static int CFG80211_OpsSurveyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN int								Idx,
	IN struct survey_info				*pSurvey)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_SURVEY SurveyInfo;


	if (Idx != 0)
		return -ENOENT;
	/* End of if */

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> %s ==>\n", __FUNCTION__));

	MAC80211_PAD_GET(pAd, pWiphy);

	/* get information from driver */
	RTMP_DRIVER_80211_SURVEY_GET(pAd, &SurveyInfo);

	/* return the information to upper layer */
	pSurvey->channel = ((CFG80211_CB *)(SurveyInfo.pCfg80211))->pCfg80211_Channels;
	pSurvey->filled = SURVEY_INFO_CHANNEL_TIME_BUSY |
						SURVEY_INFO_CHANNEL_TIME_EXT_BUSY;
	pSurvey->channel_time_busy = SurveyInfo.ChannelTimeBusy; /* unit: us */
	pSurvey->channel_time_ext_busy = SurveyInfo.ChannelTimeExtBusy;

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> busy time = %ld %ld\n",
				(ULONG)SurveyInfo.ChannelTimeBusy,
				(ULONG)SurveyInfo.ChannelTimeExtBusy));
	return 0;
#else

	return -ENOTSUPP;
#endif /* LINUX_VERSION_CODE */
} /* End of CFG80211_OpsSurveyGet */


/*
========================================================================
Routine Description:
	Cache a PMKID for a BSSID.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pPmksa			- PMKID information

Return Value:
	0				- success
	-x				- fail

Note:
	This is mostly useful for fullmac devices running firmwares capable of
	generating the (re) association RSN IE.
	It allows for faster roaming between WPA2 BSSIDs.
========================================================================
*/
static int CFG80211_OpsPmksaSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_pmksa			*pPmksa)
{
#ifdef CONFIG_STA_SUPPORT
	VOID *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> %s ==>\n", __FUNCTION__));
	MAC80211_PAD_GET(pAd, pWiphy);

	if ((pPmksa->bssid == NULL) || (pPmksa->pmkid == NULL))
		return -ENOENT;
	/* End of if */

	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_ADD;
	pIoctlPmaSa->pBssid = (UCHAR *)pPmksa->bssid;
	pIoctlPmaSa->pPmkid = pPmksa->pmkid;

	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */

	return 0;
} /* End of CFG80211_OpsPmksaSet */


/*
========================================================================
Routine Description:
	Delete a cached PMKID.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface
	pPmksa			- PMKID information

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsPmksaDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_pmksa			*pPmksa)
{
#ifdef CONFIG_STA_SUPPORT
	VOID *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> %s ==>\n", __FUNCTION__));
	MAC80211_PAD_GET(pAd, pWiphy);

	if ((pPmksa->bssid == NULL) || (pPmksa->pmkid == NULL))
		return -ENOENT;
	/* End of if */

	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_REMOVE;
	pIoctlPmaSa->pBssid = (UCHAR *)pPmksa->bssid;
	pIoctlPmaSa->pPmkid = pPmksa->pmkid;

	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */

	return 0;
} /* End of CFG80211_OpsPmksaDel */


/*
========================================================================
Routine Description:
	Flush a cached PMKID.

Arguments:
	pWiphy			- Wireless hardware description
	pNdev			- Network device interface

Return Value:
	0				- success
	-x				- fail

Note:
========================================================================
*/
static int CFG80211_OpsPmksaFlush(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev)
{
#ifdef CONFIG_STA_SUPPORT
	VOID *pAd;
	RT_CMD_STA_IOCTL_PMA_SA IoctlPmaSa, *pIoctlPmaSa = &IoctlPmaSa;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> %s ==>\n", __FUNCTION__));
	MAC80211_PAD_GET(pAd, pWiphy);

	pIoctlPmaSa->Cmd = RT_CMD_STA_IOCTL_PMA_SA_FLUSH;
	RTMP_DRIVER_80211_PMKID_CTRL(pAd, pIoctlPmaSa);
#endif /* CONFIG_STA_SUPPORT */

	return 0;
} /* End of CFG80211_OpsPmksaFlush */
#endif /* LINUX_VERSION_CODE */


static struct cfg80211_ops CFG80211_Ops = {
	.set_channel			= CFG80211_OpsSetChannel,
	.change_virtual_intf		= CFG80211_OpsChgVirtualInf,

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#ifdef CONFIG_STA_SUPPORT
	.scan				= CFG80211_OpsScan,
#endif // CONFIG_STA_SUPPORT //
#endif // LINUX_VERSION_CODE //

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
#ifdef CONFIG_STA_SUPPORT
	.join_ibss			= CFG80211_OpsJoinIbss,
	.leave_ibss			= CFG80211_OpsLeaveIbss,
#endif // CONFIG_STA_SUPPORT //
#endif // LINUX_VERSION_CODE //

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
	.set_tx_power			= CFG80211_OpsTxPwrSet,
	.get_tx_power			= CFG80211_OpsTxPwrGet,
	.set_power_mgmt			= CFG80211_OpsPwrMgmt,
	.get_station			= CFG80211_OpsStaGet,
	.dump_station			= CFG80211_OpsStaDump,
	.set_wiphy_params		= CFG80211_OpsWiphyParamsSet,
	.add_key			= CFG80211_OpsKeyAdd,
	.get_key			= CFG80211_OpsKeyGet,
	.del_key			= CFG80211_OpsKeyDel,
	.set_default_key		= CFG80211_OpsKeyDefaultSet,
#ifdef CONFIG_STA_SUPPORT
	.connect			= CFG80211_OpsConnect,
	.disconnect			= CFG80211_OpsDisconnect,
#endif // CONFIG_STA_SUPPORT //
#endif // LINUX_VERSION_CODE //

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
	/* get site survey information */
	.dump_survey			= CFG80211_OpsSurveyGet,
	/* cache a PMKID for a BSSID */
	.set_pmksa			= CFG80211_OpsPmksaSet,
	/* delete a cached PMKID */
	.del_pmksa			= CFG80211_OpsPmksaDel,
	/* flush all cached PMKIDs */
	.flush_pmksa			= CFG80211_OpsPmksaFlush,
#endif /* LINUX_VERSION_CODE */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34))
	/*
		Request the driver to remain awake on the specified
		channel for the specified duration to complete an off-channel
		operation (e.g., public action frame exchange).
	*/
	.remain_on_channel		= NULL,
	/* cancel an on-going remain-on-channel operation */
	.cancel_remain_on_channel	= NULL,
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,34))
	/* transmit an action frame */
	.action				= NULL,
#endif /* LINUX_VERSION_CODE */
#endif /* LINUX_VERSION_CODE */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	/* configure connection quality monitor RSSI threshold */
	.set_cqm_rssi_config		= NULL,
#endif /* LINUX_VERSION_CODE */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	/* notify driver that a management frame type was registered */
	.mgmt_frame_register		= NULL,
#endif /* LINUX_VERSION_CODE */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
	/* set antenna configuration (tx_ant, rx_ant) on the device */
	.set_antenna			= NULL,
	/* get current antenna configuration from device (tx_ant, rx_ant) */
	.get_antenna			= NULL,
#endif /* LINUX_VERSION_CODE */

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

#ifdef CONFIG_STA_SUPPORT
	pWdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
							       BIT(NL80211_IFTYPE_ADHOC) |
							       BIT(NL80211_IFTYPE_MONITOR);
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

#ifdef CONFIG_STA_SUPPORT
	/* default we are station mode */
	CFG80211CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_STATION;
#endif // CONFIG_STA_SUPPORT //

	pNetDev->ieee80211_ptr = CFG80211CB->pCfg80211_Wdev;
	SET_NETDEV_DEV(pNetDev, wiphy_dev(CFG80211CB->pCfg80211_Wdev->wiphy));
	CFG80211CB->pCfg80211_Wdev->netdev = pNetDev;

	/* init API function pointers */
	RT_CFG80211_API_INIT(pAd);

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
#ifdef CONFIG_STA_SUPPORT
	UINT32 BssId;

	/* free channel information for scan table */
	for(BssId=0; BssId<MAX_LEN_OF_BSS_TABLE; BssId++)
	{
		if (pAd->ScanTab.BssEntry[BssId].pCfg80211_Chan != NULL)
			os_free_mem(NULL, pAd->ScanTab.BssEntry[BssId].pCfg80211_Chan);
		/* End of if */

		pAd->ScanTab.BssEntry[BssId].pCfg80211_Chan = NULL;
	} /* End of for */
#endif // CONFIG_STA_SUPPORT //

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
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	struct ieee80211_channel *pChan;


	CFG80211DBG(RT_DEBUG_ERROR, ("80211> CFG80211_Scaning ==>\n"));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("80211> Network is down!\n"));
		return;
	} /* End of if */

	/*
		In connect function, we also need to report BSS information to cfg80211;
		Not only scan function.
	*/
	if ((pAd->FlgCfg80211Scanning == FALSE) &&
		(pAd->FlgCfg80211Connecting == FALSE))
	{
		return; /* no scan is running */
	} /* End of if */

	/* init */
	/* Note: Can not use local variable to do pChan */
	if (pAd->ScanTab.BssEntry[BssIdx].pCfg80211_Chan == NULL)
	{
		os_alloc_mem(pAd,
					(UCHAR **)&pAd->ScanTab.BssEntry[BssIdx].pCfg80211_Chan,
					sizeof(struct ieee80211_channel));
		if (pAd->ScanTab.BssEntry[BssIdx].pCfg80211_Chan == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("80211> Allocate chan fail!\n"));
			return;
		} /* End of if */
	} /* End of if */

	pChan = pAd->ScanTab.BssEntry[BssIdx].pCfg80211_Chan;
	memset(pChan, 0, sizeof(*pChan));

	if (ChanId > 14)
		pChan->band = IEEE80211_BAND_5GHZ;
	else
		pChan->band = IEEE80211_BAND_2GHZ;
	/* End of if */

	pChan->center_freq = ieee80211_channel_to_frequency(ChanId);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
	if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
	{
		if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_20)
			pChan->max_bandwidth = 20; /* 20MHz */
		else
			pChan->max_bandwidth = 40; /* 40MHz */
		/* End of if */
	}
	else
		pChan->max_bandwidth = 5; /* 5MHz for non-HT device */
	/* End of if */
#endif // LINUX_VERSION_CODE //

	/* no use currently in 2.6.30 */
//	if (ieee80211_is_beacon(((struct ieee80211_mgmt *)pFrame)->frame_control))
//		pChan->beacon_found = 1;
	/* End of if */

	/* inform 80211 a scan is got */
	/* we can use cfg80211_inform_bss in 2.6.31, it is easy more than the one */
	/* in cfg80211_inform_bss_frame(), it will memcpy pFrame but pChan */
	cfg80211_inform_bss_frame(CFG80211CB->pCfg80211_Wdev->wiphy,
								pAd->ScanTab.BssEntry[BssIdx].pCfg80211_Chan,
								(struct ieee80211_mgmt *)pFrame,
								FrameLen,
								RSSI,
								MemFlag);

	CFG80211DBG(RT_DEBUG_ERROR, ("80211> cfg80211_inform_bss_frame\n"));
#endif // CONFIG_STA_SUPPORT //
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
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("80211> Network is down!\n"));
		return;
	} /* End of if */

	if (pAd->FlgCfg80211Scanning == FALSE)
		return; /* no scan is running */
	/* End of if */

	if (FlgIsAborted == TRUE)
		FlgIsAborted = 1;
	else
		FlgIsAborted = 0;
	/* End of if */

	cfg80211_scan_done(CFG80211CB->pCfg80211_ScanReq, FlgIsAborted);

	pAd->FlgCfg80211Scanning = FALSE;
#endif // CONFIG_STA_SUPPORT //
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
