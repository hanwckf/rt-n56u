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
    cmm_radar.c

    Abstract:
    CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)

#include "rt_config.h"

INT	Set_RadarShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef DFS_SUPPORT
	int i;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;

	if (pRadarDetect->dfs_func >= HARDWARE_DFS_V1)
	{
		UINT8 idx;
		printk("DFSUseTasklet = %d\n", pRadarDetect->use_tasklet);
		printk("McuRadarDebug = %x\n", (unsigned int)pRadarDetect->McuRadarDebug);
		printk("PollTime = %d\n", pRadarDetect->PollTime);
		printk("ChEnable = %d (0x%x)\n", pDfsProgramParam->ChEnable, pDfsProgramParam->ChEnable);
		printk("DeltaDelay = %d\n", pDfsProgramParam->DeltaDelay);
		printk("Fcc5Thrd = %d\n", pDfsSwParam->fcc_5_threshold);
		printk("PeriodErr = %d\n", pDfsSwParam->dfs_period_err);
		printk("MaxPeriod = %d\n", (unsigned int)pDfsSwParam->dfs_max_period);
		printk("Ch0LErr = %d\n", pDfsSwParam->dfs_width_ch0_err_L);
		printk("Ch0HErr = %d\n", pDfsSwParam->dfs_width_ch0_err_H);
		printk("Ch1Shift = %d\n", pDfsSwParam->dfs_width_diff_ch1_Shift);
		printk("Ch2Shift = %d\n", pDfsSwParam->dfs_width_diff_ch2_Shift);
		/*printk("CeSwCheck = %d\n", pAd->CommonCfg.ce_sw_check);*/
		/*printk("CEStagCheck = %d\n", pAd->CommonCfg.ce_staggered_check);*/
		/*printk("HWDFSDisabled = %d\n", pAd->CommonCfg.hw_dfs_disabled);*/
		/*printk("WidthDiffShift = %d\n", pDfsSwParam->dfs_width_diff_Shift);*/
		printk("DfsRssiHigh = %d\n", pRadarDetect->DfsRssiHigh);
		printk("DfsRssiLow = %d\n", pRadarDetect->DfsRssiLow);

		printk("CheckLoop = %d\n", pDfsSwParam->dfs_check_loop);
		printk("DeclareThres = %d\n", pDfsSwParam->dfs_declare_thres);
		for (i =0; i < pRadarDetect->fdf_num; i++)
		{
			printk("ChBusyThrd[%d] = %d\n", i, pRadarDetect->ch_busy_threshold[i]);
			printk("RssiThrd[%d] = %d\n", i, pRadarDetect->rssi_threshold[i]);
		}
		for (idx=0; idx < pAd->chipCap.DfsEngineNum; idx++)
			printk("sw_idx[%u] = %u\n", idx, pDfsSwParam->sw_idx[idx]);
		for (idx=0; idx < pAd->chipCap.DfsEngineNum; idx++)
			printk("hw_idx[%u] = %u\n", idx, pDfsSwParam->hw_idx[idx]);
#ifdef DFS_DEBUG
		printk("Total[0] = %lu\n", pDfsSwParam->TotalEntries[0]);
		printk("Total[1] = %lu\n", pDfsSwParam->TotalEntries[1]);
		printk("Total[2] = %lu\n", pDfsSwParam->TotalEntries[2]);
		printk("Total[3] = %lu\n", pDfsSwParam->TotalEntries[3]);

		pDfsSwParam->TotalEntries[0] = pDfsSwParam->TotalEntries[1] = pDfsSwParam->TotalEntries[2] = pDfsSwParam->TotalEntries[3] = 0;

		printk("T_Matched_2 = %lu\n", pDfsSwParam->T_Matched_2);
		printk("T_Matched_3 = %lu\n", pDfsSwParam->T_Matched_3);
		printk("T_Matched_4 = %lu\n", pDfsSwParam->T_Matched_4);
		printk("T_Matched_5 = %lu\n", pDfsSwParam->T_Matched_5);		
#endif /* DFS_DEBUG */
	}

	printk("pAd->CommonCfg.ChMovingTime = %d\n", pAd->CommonCfg.ChMovingTime);
	printk("pAd->CommonCfg.RDMode = %d\n", pAd->CommonCfg.RDMode);
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	printk("pAd->CommonCfg.CarrierDetect.CD_State = %d\n", pAd->CommonCfg.CarrierDetect.CD_State);
	printk("pAd->CommonCfg.CarrierDetect.criteria = %d\n", pAd->CommonCfg.CarrierDetect.criteria);
	printk("pAd->CommonCfg.CarrierDetect.Delta = %d\n", pAd->CommonCfg.CarrierDetect.delta);
	printk("pAd->CommonCfg.CarrierDetect.DivFlag = %d\n", pAd->CommonCfg.CarrierDetect.div_flag);
	printk("pAd->CommonCfg.CarrierDetect.Threshold = %d(0x%x)\n", pAd->CommonCfg.CarrierDetect.threshold, pAd->CommonCfg.CarrierDetect.threshold);
#endif /* CARRIER_DETECTION_SUPPORT */

	return TRUE;
}


/*
       ========================================================================
       Routine Description:
               Control CCK_MRC Status
       Arguments:
               pAd     Pointer to our adapter
       Return Value:

       ========================================================================
*/
VOID CckMrcStatusCtrl(IN PRTMP_ADAPTER pAd)
{
}


/*
       ========================================================================
       Routine Description:
               Enhance DFS/CS when using GLRT.
       Arguments:
               pAd     Pointer to our adapter
       Return Value:

       ========================================================================
*/
VOID RadarGLRTCompensate(IN PRTMP_ADAPTER pAd)
{
}
#endif /*defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) */

