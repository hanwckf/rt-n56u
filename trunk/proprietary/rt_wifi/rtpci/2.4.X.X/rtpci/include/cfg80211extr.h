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

	All MAC80211/CFG80211 Function Prototype.

***************************************************************************/

#ifdef RT_CFG80211_SUPPORT


#define RT_CFG80211_API_INIT(__pAd)												\
	__pAd->CFG80211_BeaconCountryRegionParse = CFG80211_BeaconCountryRegionParse;\
	__pAd->CFG80211_RegHint = CFG80211_RegHint;									\
	__pAd->CFG80211_RegHint11D = CFG80211_RegHint11D;							\
	__pAd->CFG80211_RegRuleApply = CFG80211_RegRuleApply;						\
	__pAd->CFG80211_Scaning = CFG80211_Scaning;									\
	__pAd->CFG80211_ScanEnd = CFG80211_ScanEnd;									\
	__pAd->CFG80211_SupBandReInit = CFG80211_SupBandReInit;						\
	__pAd->CFG80211_ConnectResultInform = CFG80211_ConnectResultInform

#define RT_CFG80211_REGISTER(__pAd, __pDev, __pNetDev)							\
do {																			\
	if (__pAd->CFG80211_Register != NULL)										\
		__pAd->CFG80211_Register((VOID *)__pAd, __pDev, __pNetDev);				\
} while(0);

#define RT_CFG80211_BEACON_CR_PARSE(__pAd, __pVIE, __LenVIE)					\
do {																			\
	if ((__pAd->pCfg80211_CB != NULL) &&										\
		(__pAd->CFG80211_BeaconCountryRegionParse != NULL))						\
		__pAd->CFG80211_BeaconCountryRegionParse((VOID *)__pAd, __pVIE, __LenVIE);\
} while(0);

#define RT_CFG80211_CRDA_REG_HINT(__pAd, __pCountryIe, __CountryIeLen)			\
do {																			\
	if ((__pAd->pCfg80211_CB != NULL) &&										\
		(__pAd->CFG80211_RegHint != NULL))										\
		__pAd->CFG80211_RegHint((VOID *)__pAd, __pCountryIe, __CountryIeLen);	\
} while(0);

#define RT_CFG80211_CRDA_REG_HINT11D(__pAd, __pCountryIe, __CountryIeLen)		\
do {																			\
	if ((__pAd->pCfg80211_CB != NULL) &&										\
		(__pAd->CFG80211_RegHint11D != NULL))									\
		__pAd->CFG80211_RegHint11D((VOID *)__pAd, __pCountryIe, __CountryIeLen);\
} while(0);

#define RT_CFG80211_CRDA_REG_RULE_APPLY(__pAd)									\
do {																			\
	if ((__pAd->pCfg80211_CB != NULL) &&										\
		(__pAd->CFG80211_RegHint != NULL))										\
		__pAd->CFG80211_RegRuleApply((VOID *)__pAd, NULL, __pAd->Cfg80211_Alpha2);\
} while(0);

#define RT_CFG80211_SCANNING_INFORM(__pAd, __BssIdx, __ChanId, __pFrame,		\
			__FrameLen, __RSSI, __MemFlag)										\
do {																			\
	if ((__pAd->pCfg80211_CB != NULL) &&										\
		(__pAd->CFG80211_Scaning != NULL))										\
		__pAd->CFG80211_Scaning((VOID *)__pAd, __BssIdx, __ChanId, __pFrame,	\
								__FrameLen, __RSSI, __MemFlag);					\
} while(0);

#define RT_CFG80211_SCAN_END(__pAd, __FlgIsAborted)								\
do {																			\
	if ((__pAd->pCfg80211_CB != NULL) &&										\
		(__pAd->CFG80211_ScanEnd != NULL))										\
		__pAd->CFG80211_ScanEnd((VOID *)__pAd, __FlgIsAborted);					\
} while(0);

#define RT_CFG80211_REINIT(__pAd)												\
do {																			\
		if ((__pAd->pCfg80211_CB != NULL) &&									\
			(__pAd->CFG80211_SupBandReInit != NULL))							\
			__pAd->CFG80211_SupBandReInit((VOID *)__pAd);						\
} while(0);

#define RT_CFG80211_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe, __ReqIeLen,	\
			__pRspIe, __RspIeLen, __FlgIsSuccess)								\
do {																			\
		if ((__pAd->pCfg80211_CB != NULL) &&									\
			(__pAd->CFG80211_ConnectResultInform != NULL))						\
			__pAd->CFG80211_ConnectResultInform((VOID *)__pAd, __pBSSID,		\
				__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);	\
} while(0);


#define CFG80211_FUNC_OPS								\
	VOID (*CFG80211_BeaconCountryRegionParse)(			\
		IN VOID							*pAd,			\
		IN NDIS_802_11_VARIABLE_IEs		*pVIE,			\
		IN UINT16						LenVIE);		\
	VOID (*CFG80211_RegHint)(							\
		IN VOID							*pAd,			\
		IN UCHAR						*pCountryIe,	\
		IN ULONG						CountryIeLen);	\
	VOID (*CFG80211_RegHint11D)(						\
		IN VOID							*pAd,			\
		IN UCHAR						*pCountryIe,	\
		IN ULONG						CountryIeLen);	\
	VOID (*CFG80211_RegRuleApply)(						\
		IN VOID							*pAd,			\
		IN struct wiphy					*pWiphy,		\
		IN UCHAR						*pAlpha2);		\
	VOID (*CFG80211_Scaning)(							\
		IN VOID							*pAd,			\
		IN UINT32						BssIdx,			\
		IN UINT32						ChanId,			\
		IN UCHAR						*pFrame,		\
		IN UINT32						FrameLen,		\
		IN INT32						RSSI,			\
		IN INT32						MemFlag);		\
	VOID (*CFG80211_ScanEnd)(							\
		IN VOID							*pAd,			\
		IN BOOLEAN						FlgIsAborted);	\
	BOOLEAN (*CFG80211_SupBandReInit)(					\
		IN VOID							*pAd);			\
	VOID (*CFG80211_ConnectResultInform)(				\
		IN VOID							*pAd,			\
		IN UCHAR						*pBSSID,		\
		IN UCHAR						*pReqIe,		\
		IN UINT32						ReqIeLen,		\
		IN UCHAR						*pRspIe,		\
		IN UINT32						RspIeLen,		\
		IN UCHAR						FlgIsSuccess)

#endif // RT_CFG80211_SUPPORT //

/* End of cfg80211extr.h */
