/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_video.c

    Abstract:
    Ralink WiFi Driver video mode related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

*/

#include "rt_config.h"


#ifdef VIDEO_TURBINE_SUPPORT

BOOLEAN UpdateFromGlobal = FALSE;

void VideoTurbineUpdate(
	IN PRTMP_ADAPTER pAd)
{
	if (UpdateFromGlobal == TRUE) 
	{
		pAd->VideoTurbine.Enable = GLOBAL_AP_VIDEO_CONFIG.Enable;
		pAd->VideoTurbine.ClassifierEnable = GLOBAL_AP_VIDEO_CONFIG.ClassifierEnable;
		pAd->VideoTurbine.HighTxMode = GLOBAL_AP_VIDEO_CONFIG.HighTxMode;
		pAd->VideoTurbine.TxPwr = GLOBAL_AP_VIDEO_CONFIG.TxPwr;
		pAd->VideoTurbine.VideoMCSEnable = GLOBAL_AP_VIDEO_CONFIG.VideoMCSEnable;
		pAd->VideoTurbine.VideoMCS = GLOBAL_AP_VIDEO_CONFIG.VideoMCS;
		pAd->VideoTurbine.TxBASize = GLOBAL_AP_VIDEO_CONFIG.TxBASize;
		pAd->VideoTurbine.TxLifeTimeMode = GLOBAL_AP_VIDEO_CONFIG.TxLifeTimeMode;
		pAd->VideoTurbine.TxLifeTime = GLOBAL_AP_VIDEO_CONFIG.TxLifeTime;
		pAd->VideoTurbine.TxRetryLimit = GLOBAL_AP_VIDEO_CONFIG.TxRetryLimit;
	}
}

VOID VideoTurbineDynamicTune(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->VideoTurbine.Enable == TRUE) 
	{
		UINT32 MacReg = 0;

#ifdef RT3883
		RTMP_IO_READ32(pAd, TX_AC_RTY_LIMIT, &MacReg);
		MacReg = 0x0f1f0f0f;
		RTMP_IO_WRITE32(pAd, TX_AC_RTY_LIMIT, MacReg);

		RTMP_IO_READ32(pAd, TX_AC_FBK_SPEED, &MacReg);
		MacReg = 0x06000003;
		RTMP_IO_WRITE32(pAd, TX_AC_FBK_SPEED, MacReg);
#else
		/* Tx retry limit = 2F,1F */
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &MacReg);
		MacReg &= 0xFFFF0000;
		MacReg |= GetAsicVideoRetry(pAd);
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, MacReg);
#endif // RT3883 //

		pAd->VideoTurbine.TxBASize = GetAsicVideoTxBA(pAd);
	}
	else 
	{
		UINT32 MacReg = 0;

#ifdef RT3883
		RTMP_IO_READ32(pAd, TX_AC_RTY_LIMIT, &MacReg);
		MacReg = 0x07070707;
		RTMP_IO_WRITE32(pAd, TX_AC_RTY_LIMIT, MacReg);
	
		RTMP_IO_READ32(pAd, TX_AC_FBK_SPEED, &MacReg);
		MacReg = 0x0;
		RTMP_IO_WRITE32(pAd, TX_AC_FBK_SPEED, MacReg);
#endif // RT3883 //

		/* Default Tx retry limit = 1F,0F */
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &MacReg);
		MacReg &= 0xFFFF0000;
		MacReg |= GetAsicDefaultRetry(pAd);
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, MacReg);

		pAd->VideoTurbine.TxBASize = GetAsicDefaultTxBA(pAd);
	}
}

UINT32 GetAsicDefaultRetry(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 RetryLimit;

	RetryLimit = 0x1F0F;

	return RetryLimit;
}

UCHAR GetAsicDefaultTxBA(
	IN PRTMP_ADAPTER pAd)
{
        return pAd->CommonCfg.TxBASize;
}

UINT32 GetAsicVideoRetry(
	IN PRTMP_ADAPTER pAd)
{
	return pAd->VideoTurbine.TxRetryLimit;
}

UCHAR GetAsicVideoTxBA(
	IN PRTMP_ADAPTER pAd)
{
	return pAd->VideoTurbine.TxBASize;
}

VOID VideoConfigInit(
	IN PRTMP_ADAPTER pAd)
{
	pAd->VideoTurbine.Enable = FALSE;
	pAd->VideoTurbine.TxRetryLimit = 0x2F1F;
	pAd->VideoTurbine.TxBASize = pAd->CommonCfg.TxBASize; 
}

#endif // VIDEO_TURBINE_SUPPORT //


