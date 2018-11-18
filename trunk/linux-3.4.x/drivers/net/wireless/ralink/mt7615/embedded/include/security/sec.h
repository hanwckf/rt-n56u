/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sec.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__SEC_H__
#define	__SEC_H__

#include "security/sec_cmm.h"
#include "security/crypt_md5.h"
#include "security/crypt_sha2.h"
#include "security/crypt_hmac.h"
#include "security/crypt_aes.h"
#include "security/crypt_arc4.h"
#include "security/wpa.h"

#ifdef DOT11W_PMF_SUPPORT
#include "security/pmf.h"
#endif /* WAPI_SUPPORT */

#ifdef WAPI_SUPPORT
#include "security/wapi.h"
#endif /* WAPI_SUPPORT */

/*========================================
	The prototype is defined in cmm_sec.c
  ========================================*/
VOID SetWdevAuthMode (
    IN struct _SECURITY_CONFIG *pSecConfig, 
    IN	RTMP_STRING *arg);

VOID SetWdevEncrypMode (
    IN struct _SECURITY_CONFIG *pSecConfig, 
    IN RTMP_STRING *arg);

#ifdef WH_EZ_SETUP
#ifndef EZ_MOD_SUPPORT
VOID ez_setWdevAuthMode (
    IN struct __ez_triband_sec_config *pSecConfig, 
    IN	RTMP_STRING *arg);

VOID ez_setWdevEncrypMode (
    IN struct __ez_triband_sec_config *pSecConfig, 
    IN RTMP_STRING *arg);
#endif
#endif

INT Set_SecAuthMode_Proc (
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg);

INT Set_SecEncrypType_Proc (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *arg);

INT Set_SecDefaultKeyID_Proc (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *arg);

INT Set_SecWPAPSK_Proc (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *arg);

INT Set_SecWEPKey_Proc (
    IN PRTMP_ADAPTER pAd,
    IN CHAR KeyId,
    IN RTMP_STRING *arg);

INT Set_SecKey1_Proc (
    IN PRTMP_ADAPTER pAd,
    IN	RTMP_STRING *arg);

INT Set_SecKey2_Proc (
    IN PRTMP_ADAPTER pAd,
    IN	RTMP_STRING *arg);

INT Set_SecKey3_Proc (
    IN PRTMP_ADAPTER pAd,
    IN	RTMP_STRING *arg);

INT Set_SecKey4_Proc (
    IN PRTMP_ADAPTER pAd,
    IN	RTMP_STRING *arg);

UINT8 SecHWCipherSuitMapping (
    IN UINT32 encryMode);

INT ParseWebKey (
    IN  struct _SECURITY_CONFIG *pSecConfig,
    IN  RTMP_STRING *buffer,
    IN  INT KeyIdx,
    IN  INT Keylength);

#ifdef DOT1X_SUPPORT
 INT SetWdevOwnIPAddr (
    IN struct _SECURITY_CONFIG *pSecConfig, 
    IN RTMP_STRING *arg);

 VOID ReadRadiusParameterFromFile (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *tmpbuf, 
    IN RTMP_STRING *pBuffer);

#ifdef CONFIG_AP_SUPPORT
 VOID Dot1xIoctlQueryRadiusConf (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_IOCTL_INPUT_STRUCT *wrq);

VOID Dot1xIoctlRadiusData (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_IOCTL_INPUT_STRUCT *wrq);

VOID Dot1xIoctlAddWPAKey (
    IN PRTMP_ADAPTER	pAd,
    IN RTMP_IOCTL_INPUT_STRUCT *wrq);

VOID Dot1xIoctlStaticWepCopy (
    IN PRTMP_ADAPTER	pAd,
    IN RTMP_IOCTL_INPUT_STRUCT *wrq);
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT1X_SUPPORT */

#ifdef APCLI_SUPPORT
VOID ReadApcliSecParameterFromFile (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *tmpbuf, 
    IN RTMP_STRING *pBuffer);
#endif /* APCLI_SUPPORT */

 VOID ReadSecurityParameterFromFile (
    IN PRTMP_ADAPTER pAd,
    IN RTMP_STRING *tmpbuf, 
    IN RTMP_STRING *pBuffer);

#ifdef CONFIG_AP_SUPPORT
/* ========================================
    The prototype is defined in ap/ap_sec.c
  ========================================*/
INT APSecInit (
    IN RTMP_ADAPTER *pAd, 
    IN struct wifi_dev *wdev);

INT APKeyTableInit (
    IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev);

DECLARE_TIMER_FUNCTION(GroupRekeyExec);
VOID GroupRekeyExec (
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) ;

VOID WPAGroupRekeyByWdev (
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev);


VOID APStartRekeyTimer(
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev);


VOID APStopRekeyTimer(
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev);


VOID APReleaseRekeyTimer(
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev);


INT Show_APSecurityInfo_Proc (
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg);

VOID CheckBMCPortSecured(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN BOOLEAN isConnect);

#endif /* CONFIG_AP_SUPPORT */



#endif

