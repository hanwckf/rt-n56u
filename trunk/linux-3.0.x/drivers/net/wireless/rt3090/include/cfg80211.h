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
VOID CFG80211_Register(
	IN VOID 				*pAdCB,
	IN struct device		*pDev,
	IN struct net_device	*pNetDev);

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
	IN struct net_device	*pNetDev);

/*
========================================================================
Routine Description:
	Parse and handle country region in beacon from associated AP.

Arguments:
	pAd				- WLAN control block pointer
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
	IN UINT16					LenVIE);

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
	IN ULONG					CountryIeLen);

/*
========================================================================
Routine Description:
	Hint to the wireless core a regulatory domain from country element.

Arguments:
	pAd				- WLAN control block pointer
	pCountryIe		- pointer to the country IE
	CountryIeLen	- length of the country IE

Return Value:
	NONE

Note:
========================================================================
*/
VOID CFG80211_RegHint11D(
	IN VOID							*pAdCB,
	IN UCHAR						*pCountryIe,
	IN ULONG						CountryIeLen);

/*
========================================================================
Routine Description:
	Apply new regulatory rule.

Arguments:
	pAd				- WLAN control block pointer
	pWiphy			- Wireless hardware description
	pAlpha2			- Regulation domain (2B)

Return Value:
	NONE

Note:
	Can only be called when interface is up.
========================================================================
*/
VOID CFG80211_RegRuleApply(
	IN VOID							*pAdCB,
	IN struct wiphy					*pWiphy,
	IN UCHAR						*pAlpha2);

/*
========================================================================
Routine Description:
	Inform us that a scan is got.

Arguments:
	pAd				- WLAN control block pointer

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
	IN INT32						MemFlag);

/*
========================================================================
Routine Description:
	Inform us that scan ends.

Arguments:
	pAd				- WLAN control block pointer
	FlgIsAborted	- 1: scan is aborted

Return Value:
	NONE

Note:
========================================================================
*/
VOID CFG80211_ScanEnd(
	IN VOID							*pAdCB,
	IN BOOLEAN						FlgIsAborted);

/*
========================================================================
Routine Description:
	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.

Arguments:
	pAd				- WLAN control block pointer

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
	IN VOID							*pAdCB);

/*
========================================================================
Routine Description:
	Inform CFG80211 about association status.

Arguments:
	pAd				- WLAN control block pointer
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
VOID CFG80211_ConnectResultInform(
	IN VOID							*pAdCB,
	IN UCHAR						*pBSSID,
	IN UCHAR						*pReqIe,
	IN UINT32						ReqIeLen,
	IN UCHAR						*pRspIe,
	IN UINT32						RspIeLen,
	IN UCHAR						FlgIsSuccess);

#endif // RT_CFG80211_SUPPORT //

/* End of cfg80211.h */
