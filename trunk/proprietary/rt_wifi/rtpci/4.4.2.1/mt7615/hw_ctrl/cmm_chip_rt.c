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
	cmm_chip_rt.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"




INT rt_hif_sys_init(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_MAC_PCI
{
	/* pbf hardware reset, asic simulation sequence put this ahead before loading firmware */
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
			RTMP_IO_WRITE32(pAd, WPDMA_RST_IDX, 0xffffffff /*0x1003f*/);

		{
			RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe1f);
			RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe00);
		}
	}
	AsicInitTxRxRing(pAd);
}
#endif /* RTMP_MAC_PCI */
	return TRUE;
}
