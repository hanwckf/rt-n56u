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
void VideoTurbineUpdate(
	IN PRTMP_ADAPTER pAd)
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

VOID VideoTurbineDynamicTune(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->VideoTurbine.Enable == TRUE) 
	{
		if (pAd->VideoTurbine.TxLifeTimeMode == 0) 
		{
			UINT32 MacReg = 0;

			/* Tx retry limit = 2f,1f */
			RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
			MacReg &= 0xFFFFFF00;
			MacReg |= GetAsicVideoRetry(pAd);
			//RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);
		}

		pAd->VideoTurbine.TxBASize = GetAsicVideoTxBA(pAd);
	}
	else 
	{
		if (pAd->VideoTurbine.TxLifeTimeMode == 0) 
		{
			UINT32 MacReg = 0;

			/* Tx retry limit = 2f,1f */
			RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
			MacReg &= 0xFFFFFF00;
			MacReg |= GetAsicDefaultRetry(pAd);
			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);
		}

		pAd->VideoTurbine.TxBASize = GetAsicDefaultTxBA(pAd);
	}
}

UINT32 GetAsicDefaultRetry(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 RetryLimit;

	/*
	if (pAd->MACVersion == )
	 */
		RetryLimit = 0x1f0f;

	return RetryLimit;
}

UINT32 GetAsicDefaultTxBA(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 BASize;

        BASize = pAd->CommonCfg.TxBASize;
	if (pAd->MACVersion == 0x28720200) {
		if (BASize > 13)
			BASize = 13;
	}
	else {
		if (BASize > 7)
			BASize = 7;
	}

	return BASize;
}

UINT32 GetAsicVideoRetry(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 RetryLimit;

	/*
	if (pAd->MACVersion == )
	 */
		RetryLimit = 0x2f1f;

	return RetryLimit;
}

UINT32 GetAsicVideoTxBA(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 BASize;

        BASize = pAd->CommonCfg.TxBASize;
	if (pAd->MACVersion == 0x28720200) {
		if (BASize > 13)
			BASize = 13;
	}
	else {
		if (BASize > 7)
			BASize = 7;
	}

	return BASize;
}

VOID VideoConfigInit(
	IN PRTMP_ADAPTER pAd)
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

#endif // VIDEO_TURBINE_SUPPORT //


