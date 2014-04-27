/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_mbss.h

    Abstract:
    Support multi-BSS function.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Sample Lin  01-02-2007      created
*/

#ifndef MODULE_MBSS

#define MBSS_EXTERN    extern

#else

#define MBSS_EXTERN

#endif /* MODULE_MBSS */


/*
	For MBSS, the phy mode is different;
	So MBSS_PHY_MODE_RESET() can help us to adjust the correct mode &
	maximum MCS for the BSS.
*/
#define MBSS_PHY_MODE_RESET(__BssId, __HtPhyMode)				\
	{															\
		UCHAR __PhyMode = pAd->ApCfg.MBSSID[__BssId].PhyMode;	\
		if ((__PhyMode == WMODE_B) &&							\
			(__HtPhyMode.field.MODE != MODE_CCK))				\
		{														\
			__HtPhyMode.field.MODE = MODE_CCK;					\
			__HtPhyMode.field.MCS = 3;							\
		}														\
		else if ((!WMODE_CAP_N(__PhyMode)) &&						\
				(__PhyMode != WMODE_B) &&						\
				(__HtPhyMode.field.MODE != MODE_OFDM))			\
		{														\
			__HtPhyMode.field.MODE = MODE_OFDM;					\
			__HtPhyMode.field.MCS = 7;							\
		}														\
		else if ((__PhyMode != WMODE_B) &&						\
				(__PhyMode != WMODE_G) &&						\
				(__PhyMode != WMODE_A) &&						\
				(!WMODE_CAP_AC(__PhyMode)) &&	\
				(__HtPhyMode.field.MODE != MODE_HTMIX))			\
		{														\
			__HtPhyMode.field.MODE = MODE_HTMIX;					\
			__HtPhyMode.field.MCS = 7;							\
		}	\
		else	\
		{	\
			__HtPhyMode.field.MODE = MODE_VHT;					\
			__HtPhyMode.field.MCS = 9;							\
			__HtPhyMode.field.BW = BW_80;	\
		}	\
	}


/* Public function list */
INT	Show_MbssInfo_Display_Proc(
	IN	PRTMP_ADAPTER				pAd,
	IN	PSTRING						arg);

VOID MBSS_Init(
	IN PRTMP_ADAPTER				pAd,
	IN RTMP_OS_NETDEV_OP_HOOK		*pNetDevOps);

VOID MBSS_Remove(
	IN PRTMP_ADAPTER				pAd);

INT MBSS_Open(
	IN	PNET_DEV					pDev);

INT MBSS_Close(
	IN	PNET_DEV					pDev);

INT32 RT28xx_MBSS_IdxGet(
	IN PRTMP_ADAPTER	pAd,
	IN PNET_DEV			pDev);

