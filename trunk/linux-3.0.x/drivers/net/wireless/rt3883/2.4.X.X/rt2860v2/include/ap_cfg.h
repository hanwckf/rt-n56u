#ifndef __AP_CFG_H__
#define __AP_CFG_H__


#include "rt_config.h"

INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER *pAd, 
	IN struct iwreq *pIoctlCmdStr);

INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd, 
	IN struct iwreq *pIoctlCmdStr);

#ifdef INF_AR9
#ifdef AR9_MAPI_SUPPORT
INT RTMPAPPrivIoctlAR9Show(
	IN RTMP_ADAPTER *pAd, 
	IN struct iwreq *pIoctlCmdStr);

VOID RTMPAR9IoctlGetMacTable(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq);

VOID RTMPIoctlGetSTAT2(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq);

VOID RTMPIoctlGetRadioDynInfo(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq);
#endif //AR9_MAPI_SUPPORT//
#endif//INF_AR9//

INT RTMPAPSetInformation(
	IN	PRTMP_ADAPTER	pAd,
	IN	OUT	struct iwreq	*rq,
	IN	INT				cmd);

INT RTMPAPQueryInformation(
	IN	PRTMP_ADAPTER       pAd,
	IN	OUT	struct iwreq    *rq,
	IN	INT                 cmd);
	
VOID RTMPIoctlStatistics(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq);

#ifdef RTMP_RBUS_SUPPORT
/* +++ added by Red@Ralink, 2009/09/30 */
VOID RTMPIoctlGetMacTableStaInfo(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq);
/* ++ end of addition */
#endif // RTMP_RBUS_SUPPORT //

VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq);


VOID RTMPAPIoctlE2PROM(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  struct iwreq    *wrq);

#ifdef DBG
VOID RTMPAPIoctlBBP(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  struct iwreq    *wrq);

VOID RTMPAPIoctlMAC(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  struct iwreq    *wrq);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPAPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq);
#endif // RTMP_RF_RW_SUPPORT //

#endif // DBG //

VOID RT28XX_IOCTL_MaxRateGet(
	IN	RTMP_ADAPTER			*pAd,
	IN	PHTTRANSMIT_SETTING	pHtPhyMode,
	OUT	UINT32					*pRate);

#ifdef WSC_AP_SUPPORT
VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);
//add by woody
#ifdef INF_AR9
#ifdef AR9_MAPI_SUPPORT
VOID RTMPAR9IoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlWscPINCode(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);


VOID RTMPIoctlWscStatus(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlGetWscDynInfo(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlGetWscRegsDynInfo(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);
#endif//AR9_MAPI_SUPPORT//
#endif//INF_AR9//
#endif // WSC_AP_SUPPORT //

#ifdef DOT11_N_SUPPORT
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq);
#endif // DOT11_N_SUPPORT //

#ifdef DOT1X_SUPPORT
VOID RTMPIoctlStaticWepCopy(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq);

VOID RTMPIoctlRadiusData(
	IN PRTMP_ADAPTER	pAd, 
	IN struct iwreq		*wrq);

VOID RTMPIoctlAddWPAKey(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq);

VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq);

VOID RTMPIoctlSetIdleTimeout(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq);
#endif // DOT1X_SUPPORT //

INT	ApCfg_Set_AuthMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	INT				apidx,
	IN	PSTRING			arg);

INT	ApCfg_Set_MaxStaNum_Proc(
	IN PRTMP_ADAPTER 	pAd,
	IN INT				apidx,
	IN PSTRING 			arg);

INT	ApCfg_Set_IdleTimeout_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#endif // __AP_CFG_H__ //

