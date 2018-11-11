/** $Id: $
*/

/*! \file   "ra_wrapper_embedded.c"
    \brief
*/

/*******************************************************************************
* Copyright (c) 2014 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/

/*
** $Log: ra_wrapper_embedded.c $
**
** 08 19 2016 by.huang
** [WCNCR00128952] There are some limitation by current SKU algorithm in MT7615 power
** 	
** 	1) Purpose:
** 	
** 	1. revise SKU mechanism for compatibility with Nss combine gain with spatial extension
** 	2. add Spatial extension control(on/off) by profile
** 	
** 	2) Changed function name:
** 	
** 	1. SKUTxPwrOffsetGet
** 	2. EventTxPowerShowInfo
** 	3. EventTxPowerCompTable
** 	4. MtSingleSkuLoadParam
** 	
** 	3) Code change description brief:
** 	
** 	1. revise SKU compensation look table mechanism for Nss spatial extension combine gain backoff
** 	2. add compensation info command
** 	
** 	4) Unit Test Result:
** 	
** 	1. build pass 
** 	2. function pass (use IQxel check Power upper bound for all phymode and phy rate and al Tx stream in QA mode and Normal mode)
**
** 06 16 2016 chunting.wu
** [WCNCR00121389] [JEDI][64-bit porting]
** 	
** 	1) Purpose:
** 	Fix 4-byte alignment.
** 	2) Changed function name:
** 	RA_PHY_CFG_T, CMD_STAREC_AUTO_RATE_T, 
** 	CMD_STAREC_AUTO_RATE_CFG_T
** 	3) Code change description brief:
** 	Fix 4-byte alignment.
** 	4) Unit Test Result:
** 	UT pass.
**
** 05 26 2016 chunting.wu
** [WCNCR00121272] [MT7615] dynamic adjust max phy rate for mt7621 platform
** 	
** 	1) Purpose:
** 	MT7621 can TX 4SS MCS8,9 when initial.
** 	2) Changed function name:
** 	MtCmdSetMaxPhyRate()
** 	MacTableMaintenance()
** 	3) Code change description brief:
** 	send MCU command to limit max phy rate when TP > 50mbps.	
** 	4) Unit Test Result:
** 	RDUT pass.
**
** 05 04 2016 chunting.wu
** [WCNCR00120115] [MT7615] change text format from dos to Unix avoid compile error
** 	
** 	1) Purpose:
** 	Change text format from dos to unix.
** 	2) Changed function name:
** 	
** 	3) Code change description brief:
** 	
** 	4) Unit Test Result:
** 	Build pass.
**
** 03 11 2016 chunting.wu
** [WCNCR00036330] [MT7615] Auto rate control
** 	
** 	1) Purpose:
** 	Profile support G band 256QAM enable/disable.
** 	2) Changed function name:
** 	ExtEventGBand256QamProbeResule()
** 	raWrapperEntrySet()
** 	3) Code change description brief:
** 	driver notify FW RA enable/disable G band 256QAM probing.
** 	4) Unit Test Result:
** 	RD UT pass
** 	
** 	Review: http://mtksap20:8080/go?page=NewReview&reviewid=242440
**
**
**
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "rt_config.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#ifdef MT_MAC
/*----------------------------------------------------------------------------*/
/*!
* \brief     Set RaEntry by pEntry
*
* \param[in] pAd
* \param[in] pEntry
* \param[out] pRaEntry
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
raWrapperEntrySet(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	OUT P_RA_ENTRY_INFO_T pRaEntry
	)
{
    pRaEntry->ucWcid = pEntry->wcid;
    pRaEntry->fgAutoTxRateSwitch = pEntry->bAutoTxRateSwitch;

    pRaEntry->ucPhyMode = pEntry->wdev->PhyMode;
    pRaEntry->ucChannel= pEntry->wdev->channel;
	//use the maximum bw capability
	pRaEntry->ucBBPCurrentBW = HcGetBw(pAd, pEntry->wdev);

    {
        pRaEntry->fgDisableCCK = FALSE;
    }

    pRaEntry->fgHtCapMcs32 = (pEntry->HTCapability.MCSSet[4] & 0x1)? TRUE:FALSE;
    pRaEntry->fgHtCapInfoGF = pEntry->HTCapability.HtCapInfo.GF;
    pRaEntry->aucHtCapMCSSet[0] = pEntry->HTCapability.MCSSet[0];
    pRaEntry->aucHtCapMCSSet[1] = pEntry->HTCapability.MCSSet[1];
    pRaEntry->aucHtCapMCSSet[2] = pEntry->HTCapability.MCSSet[2];
    pRaEntry->aucHtCapMCSSet[3] = pEntry->HTCapability.MCSSet[3];
    pRaEntry->ucMmpsMode = pEntry->MmpsMode;

    if (pEntry->fgGband256QAMSupport == TRUE)
    {
        pRaEntry->ucGband256QAMSupport = RA_G_BAND_256QAM_ENABLE;
    }
    else if (pAd->CommonCfg.g_band_256_qam == TRUE)
    {
        pRaEntry->ucGband256QAMSupport = RA_G_BAND_256QAM_PROBING;
    }
    else
    {
        pRaEntry->ucGband256QAMSupport = RA_G_BAND_256QAM_DISABLE;
    }

    pRaEntry->ucMaxAmpduFactor = pEntry->MaxRAmpduFactor;
    pRaEntry->RateLen = pEntry->RateLen;
    pRaEntry->ucSupportRateMode = pEntry->SupportRateMode;
    pRaEntry->ucSupportCCKMCS = pEntry->SupportCCKMCS;
    pRaEntry->ucSupportOFDMMCS = pEntry->SupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
    pRaEntry->u4SupportHTMCS = pEntry->SupportHTMCS;
#ifdef DOT11_VHT_AC
    pRaEntry->u2SupportVHTMCS1SS = pEntry->SupportVHTMCS1SS;
    pRaEntry->u2SupportVHTMCS2SS = pEntry->SupportVHTMCS2SS;
    if (pEntry->MaxHTPhyMode.field.BW < BW_160)
    {
        pRaEntry->u2SupportVHTMCS3SS = pEntry->SupportVHTMCS3SS;
        pRaEntry->u2SupportVHTMCS4SS = pEntry->SupportVHTMCS4SS;
    }
    pRaEntry->force_op_mode = pEntry->force_op_mode;
    pRaEntry->vhtOpModeChWidth = pEntry->operating_mode.ch_width;
    pRaEntry->vhtOpModeRxNss = pEntry->operating_mode.rx_nss;
    pRaEntry->vhtOpModeRxNssType = pEntry->operating_mode.rx_nss_type;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

    pRaEntry->AvgRssiSample[0] = pEntry->RssiSample.AvgRssi[0];
    pRaEntry->AvgRssiSample[1] = pEntry->RssiSample.AvgRssi[1];
    pRaEntry->AvgRssiSample[2] = pEntry->RssiSample.AvgRssi[2];

#ifdef WAPI_SUPPORT
    if (IS_AKM_WAICERT(pEntry->SecConfig.AKMMap) || IS_AKM_WPIPSK(pEntry->SecConfig.AKMMap))
    {
        pRaEntry->fgAuthWapiMode = TRUE;
    }
    else
#endif /* WAPI_SUPPORT */
    {
        pRaEntry->fgAuthWapiMode = FALSE;
    }

    pRaEntry->ClientStatusFlags = pEntry->ClientStatusFlags;

    pRaEntry->MaxPhyCfg.MODE = pEntry->MaxHTPhyMode.field.MODE;
    pRaEntry->MaxPhyCfg.STBC = pEntry->MaxHTPhyMode.field.STBC;
    pRaEntry->MaxPhyCfg.ShortGI = pEntry->MaxHTPhyMode.field.ShortGI;
    pRaEntry->MaxPhyCfg.BW = pEntry->MaxHTPhyMode.field.BW;
    pRaEntry->MaxPhyCfg.ldpc = pEntry->MaxHTPhyMode.field.ldpc;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC
    if (pRaEntry->MaxPhyCfg.MODE == MODE_VHT)
    {
        pRaEntry->MaxPhyCfg.MCS = pEntry->MaxHTPhyMode.field.MCS & 0xf;
        pRaEntry->MaxPhyCfg.VhtNss = ((pEntry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
    }
    else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
    {
        pRaEntry->MaxPhyCfg.MCS = pEntry->MaxHTPhyMode.field.MCS;
        pRaEntry->MaxPhyCfg.VhtNss = 0;
    }

    pRaEntry->TxPhyCfg.MODE = pEntry->HTPhyMode.field.MODE;
    pRaEntry->TxPhyCfg.STBC = pEntry->HTPhyMode.field.STBC;
    pRaEntry->TxPhyCfg.ShortGI = pEntry->HTPhyMode.field.ShortGI;
    pRaEntry->TxPhyCfg.BW = pEntry->HTPhyMode.field.BW;

    pRaEntry->TxPhyCfg.ldpc = pEntry->HTPhyMode.field.ldpc;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC
    if (pRaEntry->TxPhyCfg.MODE == MODE_VHT)
    {
        pRaEntry->TxPhyCfg.MCS = pEntry->HTPhyMode.field.MCS & 0xf;
        pRaEntry->TxPhyCfg.VhtNss = ((pEntry->HTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
    }
    else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
    {
        pRaEntry->TxPhyCfg.MCS = pEntry->HTPhyMode.field.MCS;
        pRaEntry->TxPhyCfg.VhtNss = 0;
    }

}


/*----------------------------------------------------------------------------*/
/*!
* \brief     Restore RaEntry to pEntry
*
* \param[in] pAd
* \param[in] pEntry
* \param[in] pRaEntry
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
raWrapperEntryRestore(
    IN PRTMP_ADAPTER pAd,
    IN PMAC_TABLE_ENTRY	pEntry,
    IN P_RA_ENTRY_INFO_T pRaEntry
    )
{
    pEntry->MaxHTPhyMode.field.MODE = pRaEntry->MaxPhyCfg.MODE;
    pEntry->MaxHTPhyMode.field.STBC = pRaEntry->MaxPhyCfg.STBC;
    pEntry->MaxHTPhyMode.field.ShortGI = pRaEntry->MaxPhyCfg.ShortGI ? 1:0;
    pEntry->MaxHTPhyMode.field.BW = pRaEntry->MaxPhyCfg.BW;
    pEntry->MaxHTPhyMode.field.ldpc = pRaEntry->MaxPhyCfg.ldpc ? 1:0;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC
    if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
    {
        pEntry->MaxHTPhyMode.field.MCS = (((pRaEntry->MaxPhyCfg.VhtNss - 1) & 0x3) << 4) + pRaEntry->MaxPhyCfg.MCS;
    }
    else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
    {
        pEntry->MaxHTPhyMode.field.MCS = pRaEntry->MaxPhyCfg.MCS;
    }

    pEntry->HTPhyMode.field.MODE = pRaEntry->TxPhyCfg.MODE;
    pEntry->HTPhyMode.field.STBC = pRaEntry->TxPhyCfg.STBC;
    pEntry->HTPhyMode.field.ShortGI = pRaEntry->TxPhyCfg.ShortGI ? 1:0;
    pEntry->HTPhyMode.field.BW = pRaEntry->TxPhyCfg.BW;
    pEntry->HTPhyMode.field.ldpc = pRaEntry->TxPhyCfg.ldpc ? 1:0;
    pEntry->HTPhyMode.field.MCS = pRaEntry->TxPhyCfg.MCS;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC
    if (pRaEntry->TxPhyCfg.MODE == MODE_VHT)
    {
        pEntry->HTPhyMode.field.MCS = (((pRaEntry->TxPhyCfg.VhtNss - 1) & 0x3) << 4) + pRaEntry->TxPhyCfg.MCS;
    }
    else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
    {
        pEntry->HTPhyMode.field.MCS = pRaEntry->TxPhyCfg.MCS;
    }

    pEntry->LastTxRate = pEntry->HTPhyMode.word;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     Set RaCfg according pAd and pAd->CommonCfg.
*
* \param[in] pAd
* \param[out] pRaCfg
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
raWrapperConfigSet(
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev,
    OUT P_RA_COMMON_INFO_T pRaCfg)
{
    pRaCfg->OpMode = pAd->OpMode;
    pRaCfg->fgAdHocOn = ADHOC_ON(pAd);
    pRaCfg->fgShortPreamble = OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED)?TRUE:FALSE;

    //pRaCfg->TxStream = pAd->CommonCfg.TxStream;
    //pRaCfg->RxStream = pAd->CommonCfg.RxStream;
    pRaCfg->TxStream = wlan_config_get_tx_stream(wdev);
    pRaCfg->RxStream = wlan_config_get_rx_stream(wdev);

    pRaCfg->ucRateAlg = pAd->rateAlg;

    pRaCfg->TestbedForceShortGI = pAd->WIFItestbed.bShortGI;
    pRaCfg->TestbedForceGreenField = pAd->WIFItestbed.bGreenField;

#ifdef DOT11_N_SUPPORT
    pRaCfg->HtMode = pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
    pRaCfg->fAnyStation20Only = pAd->MacTab.fAnyStation20Only;
    pRaCfg->bRcvBSSWidthTriggerEvents = pAd->CommonCfg.bRcvBSSWidthTriggerEvents;
#ifdef DOT11_VHT_AC
    pRaCfg->vht_nss_cap = pAd->CommonCfg.vht_nss_cap;
#ifdef WFA_VHT_PF
    pRaCfg->vht_bw_signal = pAd->CommonCfg.vht_bw_signal;
    pRaCfg->vht_force_sgi = pAd->vht_force_sgi;
#endif /* WFA_VHT_PF */
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
    pRaCfg->fgSeOff = pAd->CommonCfg.bSeOff;
    if (wlan_config_get_tx_stream(wdev) == 4)
    {
        pRaCfg->ucAntennaIndex = pAd->CommonCfg.ucAntennaIndex;
    }
#ifdef THERMAL_PROTECT_SUPPORT
    pRaCfg->fgThermalProtectToggle = pAd->fgThermalProtectToggle;
    pRaCfg->force_one_tx_stream = pAd->force_one_tx_stream;
#else
    pRaCfg->fgThermalProtectToggle = FALSE;
    pRaCfg->force_one_tx_stream = FALSE;
#endif /* THERMAL_PROTECT_SUPPORT */

    pRaCfg->TrainUpRule = pAd->CommonCfg.TrainUpRule;
    pRaCfg->TrainUpHighThrd = pAd->CommonCfg.TrainUpHighThrd;
    pRaCfg->TrainUpRuleRSSI = pAd->CommonCfg.TrainUpRuleRSSI;
    pRaCfg->lowTrafficThrd = pAd->CommonCfg.lowTrafficThrd;

#if defined(MT7615) || defined(MT7622)
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
    if (pAd->fgRaLimitPhyRate == TRUE)
    {
        pRaCfg->u2MaxPhyRate = RACTRL_LIMIT_MAX_PHY_RATE;
    }
    else
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
    {
        pRaCfg->u2MaxPhyRate = 0;
    }
#endif /* defined(MT7615) || defined(MT7622) */

    pRaCfg->PhyCaps = pAd->chipCap.phy_caps;

    pRaCfg->u4RaInterval = pAd->ra_interval;
    pRaCfg->u4RaFastInterval = pAd->ra_fast_interval;

#ifdef DBG_CTRL_SUPPORT
    pRaCfg->DebugFlags = pAd->CommonCfg.DebugFlags;
#endif /* DBG_CTRL_SUPPORT */
}


#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
/*----------------------------------------------------------------------------*/
/*!
* \brief     The wrapper function of QuickResponeForRateAdaptMTCore()
*
* \param[in] pAd
* \param[in] idx
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
QuickResponeForRateAdaptMT(/* actually for both up and down */
    IN PRTMP_ADAPTER pAd,
    IN UINT_8 idx)
{
    P_RA_ENTRY_INFO_T pRaEntry;
    P_RA_INTERNAL_INFO_T pRaInternal;
    RA_COMMON_INFO_T RaCfg;
    MAC_TABLE_ENTRY *pEntry;
    UCHAR TableSize = 0;
    UCHAR InitTxRateIdx;

    pEntry = &pAd->MacTab.Content[idx]; /* point to information of the individual station */

    pRaEntry = &pEntry->RaEntry;
    pRaInternal = &pEntry->RaInternal;

    if (pRaInternal->ucLastSecTxRateChangeAction == RATE_NO_CHANGE)
    {
        return;
    }

    //os_zero_mem(pRaEntry, sizeof(RA_ENTRY_INFO_T));
    os_zero_mem(&RaCfg, sizeof(RaCfg));

    raWrapperEntrySet(pAd, pEntry, pRaEntry);
    raWrapperConfigSet(pAd, pEntry->wdev, &RaCfg);

    raSelectTxRateTable(pRaEntry, &RaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &InitTxRateIdx);

#ifdef NEW_RATE_ADAPT_SUPPORT
    if (RaCfg.ucRateAlg == RATE_ALG_GRP) 
    {
        QuickResponeForRateAdaptMTCore(pAd, pRaEntry, &RaCfg, pRaInternal);
    }
#endif /* NEW_RATE_ADAPT_SUPPORT */

#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))
    if (RaCfg.ucRateAlg == RATE_ALG_AGBS) 
    {
        QuickResponeForRateAdaptAGBSMTCore(pAd, pRaEntry, &RaCfg, pRaInternal);
    }
#endif /* RATE_ADAPT_AGBS_SUPPORT */

    raWrapperEntryRestore(pAd, pEntry, pRaEntry);
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     The wrapper function of DynamicTxRateSwitchingAdaptMtCore()
*
* \param[in] pAd
* \param[in] idx
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
DynamicTxRateSwitchingAdaptMT(
    RTMP_ADAPTER *pAd, 
    UINT_8 idx
    )
{
    P_RA_ENTRY_INFO_T pRaEntry;
    P_RA_INTERNAL_INFO_T pRaInternal;
    RA_COMMON_INFO_T RaCfg;
    MAC_TABLE_ENTRY *pEntry;
    UCHAR TableSize = 0;
    UCHAR InitTxRateIdx;

    pEntry = &pAd->MacTab.Content[idx]; /* point to information of the individual station */

    pRaEntry = &pEntry->RaEntry;
    pRaInternal = &pEntry->RaInternal;

    //os_zero_mem(pRaEntry, sizeof(RA_ENTRY_INFO_T));
    os_zero_mem(&RaCfg, sizeof(RaCfg));

    raWrapperEntrySet(pAd, pEntry, pRaEntry);
    raWrapperConfigSet(pAd, pEntry->wdev, &RaCfg);

    raSelectTxRateTable(pRaEntry, &RaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &InitTxRateIdx);

#ifdef NEW_RATE_ADAPT_SUPPORT
    if (RaCfg.ucRateAlg == RATE_ALG_GRP) 
    {
        DynamicTxRateSwitchingAdaptMtCore(pAd, pRaEntry, &RaCfg, pRaInternal);
    }
#endif /* NEW_RATE_ADAPT_SUPPORT */

#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))
    if (RaCfg.ucRateAlg == RATE_ALG_AGBS) 
    {
        DynamicTxRateSwitchingAGBSMtCore(pAd, pRaEntry, &RaCfg, pRaInternal);
    }
#endif /* RATE_ADAPT_AGBS_SUPPORT */

    raWrapperEntryRestore(pAd, pEntry, pRaEntry);    
}
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */


#ifdef CONFIG_AP_SUPPORT
/*----------------------------------------------------------------------------*/
/*!
* \brief     This routine walks through the MAC table, see if TX rate change is
*            required for each associated client.
*
* \param[in] pAd
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID 
APMlmeDynamicTxRateSwitching(
    RTMP_ADAPTER *pAd
    )
{
    UINT i;
#if defined(RTMP_MAC) || defined(RLT_MAC)
    PUCHAR pTable;
    UCHAR TableSize = 0, InitTxRateIdx;
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
    MAC_TABLE_ENTRY *pEntry;
    UINT32 ret;

#ifdef CONFIG_ATE
    if (ATE_ON(pAd))
    {
        return;
    }
#endif /* CONFIG_ATE */

    RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);

    /* walk through MAC table, see if need to change AP's TX rate toward each entry */
    for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
    {
        /* point to information of the individual station */
        pEntry = &pAd->MacTab.Content[i];

        if (IS_ENTRY_NONE(pEntry))
        {
        	continue;
        }

        if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
        {
        	continue;
        }

#ifdef APCLI_SUPPORT
        if (IS_ENTRY_APCLI(pEntry) && (pEntry->Sst != SST_ASSOC))
        {      
            continue;
        }
#ifdef MAC_REPEATER_SUPPORT
        if (IS_ENTRY_REPEATER(pEntry) && (pEntry->Sst != SST_ASSOC))
        {
            continue;
        }
#endif
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
        if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx))
        {      
            continue;
        }
#endif /* WDS_SUPPORT */


        /* check if this entry need to switch rate automatically */
        if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
        {      
            continue;
        }

#ifdef MT_MAC
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
        if (pAd->chipCap.hif_type == HIF_MT) 
        {
            DynamicTxRateSwitchingAdaptMT(pAd, (UINT_8)i);

#ifdef NEW_RATE_ADAPT_SUPPORT
            if (pAd->rateAlg == RATE_ALG_GRP) 
            {
		    UCHAR pkt_num = wlan_operate_get_rts_pkt_thld(pEntry->wdev);
		    UINT32 length = wlan_oeprate_get_rts_len_thld(pEntry->wdev);
                if ( pAd->MacTab.Size == 1 )
                {
                    if ( ((pEntry->RaInternal.pucTable == RateSwitchTableAdapt11N2S) && pEntry->HTPhyMode.field.MCS >= 14 ) ||
                            ((pEntry->RaInternal.pucTable == RateSwitchTableAdapt11N1S) && pEntry->HTPhyMode.field.MCS >= 6 ) )
                    {
                        if (pAd->bDisableRtsProtect != TRUE)
                        {
			    pkt_num = MAX_RTS_PKT_THRESHOLD;
			    length = MAX_RTS_THRESHOLD;
                            pAd->bDisableRtsProtect = TRUE;
                        }
                    }
                    else
                    {
                        if (pAd->bDisableRtsProtect != FALSE)
                        {
                            pAd->bDisableRtsProtect = FALSE;
                        }
                    }
                }
                else
                {
                    if (pAd->bDisableRtsProtect != FALSE)
                    {
			    pAd->bDisableRtsProtect = FALSE;
                    }
                }
		HW_SET_RTS_THLD(pAd, pEntry->wdev, pkt_num, length);
            }
#endif /* NEW_RATE_ADAPT_SUPPORT */

            continue;
        }
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)

        MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
        pEntry->pTable = pTable;

#ifdef NEW_RATE_ADAPT_SUPPORT
        if (ADAPT_RATE_TABLE(pTable))
        {
            if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
            {
                APMlmeDynamicTxRateSwitchingAdapt(pAd, i);
            }

        }
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd) && AGS_IS_USING(pAd, pTable))
        {
            ApMlmeDynamicTxRateSwitchingAGS(pAd, i);
            continue;
        }
#endif /* AGS_SUPPORT */

        APMlmeDynamicTxRateSwitchingLegacy(pAd, i);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

    }
#ifdef THERMAL_PROTECT_SUPPORT
    pAd->fgThermalProtectToggle = FALSE;
#endif /* THERMAL_PROTECT_SUPPORT */

    RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     AP side, Auto TxRate faster train up timer call back function.
*
* \param[in] SystemSpecific1
* \param[in] FunctionContext    Pointer to our Adapter context.
* \param[in] SystemSpecific2
* \param[in] SystemSpecific3
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3
    )
{
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
    UINT i;
    MAC_TABLE_ENTRY *pEntry;
#if defined(RTMP_MAC) || defined(RLT_MAC)
    PUCHAR pTable;
    UCHAR TableSize = 0, InitTxRateIdx;
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

    pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;

    /* walk through MAC table, see if need to change AP's TX rate toward each entry */
    for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
    {
        pEntry = &pAd->MacTab.Content[i];

        if (IS_ENTRY_NONE(pEntry))
        {      
            continue;
        }

        if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
        {      
            continue;
        }

#ifdef APCLI_SUPPORT
        if (IS_ENTRY_APCLI(pEntry) && (pEntry->Sst != SST_ASSOC))
        {      
            continue;
        }
#ifdef MAC_REPEATER_SUPPORT
        if (IS_ENTRY_REPEATER(pEntry) && (pEntry->Sst != SST_ASSOC))
        {
            continue;
        }
#endif
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
        if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx))
        {      
            continue;
        }
#endif /* WDS_SUPPORT */


#if defined(RTMP_MAC) || defined(RLT_MAC)
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
        if (pAd->chipCap.hif_type == HIF_MT)
        {
            QuickResponeForRateAdaptMT(pAd, (UINT_8)i);
            continue;
        }
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */
#endif /* MT_MAC */

        /* Do nothing if this entry didn't change */
        if (pEntry->LastSecTxRateChangeAction == RATE_NO_CHANGE
#ifdef DBG_CTRL_SUPPORT
                && (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
                )
        {
            continue;
        }

#if defined(RTMP_MAC) || defined(RLT_MAC)
        MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
        pEntry->pTable = pTable;

#ifdef NEW_RATE_ADAPT_SUPPORT
        if (ADAPT_RATE_TABLE(pTable))
        {
            if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
            {
                APQuickResponeForRateUpExecAdapt(pAd, i);
            }

            continue;
		}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd) && AGS_IS_USING(pAd, pTable))
        {
            ApMlmeDynamicTxRateSwitchingAGS(pAd, i);
            continue;
        }
#endif /* AGS_SUPPORT */

        APQuickResponeForRateAdaptLegacy(pAd, i);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

    }
}
#endif /* CONFIG_AP_SUPPORT */




/*----------------------------------------------------------------------------*/
/*!
* \brief     This routine parse rate IEs and ouput the supported MCS table.
*
* \param[in] pAd
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID RTMPSetSupportMCS(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR OpMode,
    IN PMAC_TABLE_ENTRY	pEntry,
    IN UCHAR SupRate[],
    IN UCHAR SupRateLen,
    IN UCHAR ExtRate[],
    IN UCHAR ExtRateLen,
#ifdef DOT11_VHT_AC
    IN UCHAR vht_cap_len,
    IN VHT_CAP_IE *vht_cap,
#endif /* DOT11_VHT_AC */
    IN HT_CAPABILITY_IE *pHtCapability,
    IN UCHAR HtCapabilityLen
    )
{
    UCHAR idx, SupportedRatesLen = 0;
    UCHAR SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];

    if (SupRateLen > 0)
    {
        if (SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)
        {
            os_move_mem(SupportedRates, SupRate, SupRateLen);
            SupportedRatesLen = SupRateLen;
        }
        else
        {
            UCHAR RateDefault[8] = {0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c};

            os_move_mem(SupportedRates, RateDefault, 8);
            SupportedRatesLen = 8;

            MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s():wrong SUPP RATES., Len=%d\n",
                    __FUNCTION__, SupRateLen));
        }
    }

    if (ExtRateLen > 0)
    {
        if ((SupRateLen + ExtRateLen) <= MAX_LEN_OF_SUPPORTED_RATES)
        {
            os_move_mem(&SupportedRates[SupRateLen], ExtRate, ExtRateLen);
            SupportedRatesLen += ExtRateLen;
        }
        else
        {
            os_move_mem(&SupportedRates[SupRateLen], ExtRate, MAX_LEN_OF_SUPPORTED_RATES - ExtRateLen);
            SupportedRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
        }
    }

    /* Clear Supported MCS Table */
    pEntry->SupportCCKMCS = 0;
    pEntry->SupportOFDMMCS = 0;
    pEntry->SupportHTMCS = 0;
#ifdef DOT11_VHT_AC
    pEntry->SupportVHTMCS1SS = 0;
    pEntry->SupportVHTMCS2SS = 0;
    pEntry->SupportVHTMCS3SS = 0;
    pEntry->SupportVHTMCS4SS = 0;
#endif /* DOT11_VHT_AC */

    pEntry->SupportRateMode = 0;

    for(idx = 0; idx < SupportedRatesLen; idx ++)
    {
        switch((SupportedRates[idx] & 0x7F)*5)
        {
            case 10:
                pEntry->SupportCCKMCS |= (1 << MCS_0);
                pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
                break;

            case 20:
                pEntry->SupportCCKMCS |= (1 << MCS_1);
                pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
                break;

            case 55:
                pEntry->SupportCCKMCS |= (1 << MCS_2);
                pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
                break;

            case 110:
                pEntry->SupportCCKMCS |= (1 << MCS_3);
                pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
                break;

            case 60:
                pEntry->SupportOFDMMCS |= (1 << MCS_0);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;

            case 90:
                pEntry->SupportOFDMMCS |= (1 << MCS_1);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;

            case 120:
                pEntry->SupportOFDMMCS |= (1 << MCS_2);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;

            case 180:
                pEntry->SupportOFDMMCS |= (1 << MCS_3);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;

            case 240:
                pEntry->SupportOFDMMCS |= (1 << MCS_4);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;

            case 360:
                pEntry->SupportOFDMMCS |= (1 << MCS_5);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;

            case 480:
                pEntry->SupportOFDMMCS |= (1 << MCS_6);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;

            case 540:
                pEntry->SupportOFDMMCS |= (1 << MCS_7);
                pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
                break;
        }	
    }

    if (HtCapabilityLen)
    {
        RT_PHY_INFO *pDesired_ht_phy = NULL;
        UCHAR j, bitmask;
        CHAR i;


#ifdef CONFIG_AP_SUPPORT
        if (OpMode == OPMODE_AP)
        {
#ifdef WDS_SUPPORT
            if (IS_ENTRY_WDS(pEntry))
            {
                pDesired_ht_phy = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
            }
            else
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
            if (IS_ENTRY_APCLI(pEntry)|| IS_ENTRY_REPEATER(pEntry))
            {
                pDesired_ht_phy = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
            }
            else
#endif /* APCLI_SUPPORT */
            {
                pDesired_ht_phy = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
            }
        }
#endif /* CONFIG_AP_SUPPORT */

        if (pDesired_ht_phy == NULL)
        {
            return;
        }

        for (i = 31; i >= 0; i--)
        {
            j = i / 8;
            bitmask = (1 << (i - (j * 8)));

            if ((pDesired_ht_phy->MCSSet[j] & bitmask)
                    && (pHtCapability->MCSSet[j] & bitmask))
            {
                pEntry->SupportHTMCS |= 1 << i;
                pEntry->SupportRateMode |= SUPPORT_HT_MODE;
            }
        }

#ifdef DOT11_VHT_AC
        if ((vht_cap_len > 0)&& (vht_cap != NULL) && pDesired_ht_phy->bVhtEnable)
        {
            /* Currently we only support for MCS0~MCS7, so don't check mcs_map */
            pEntry->SupportVHTMCS1SS = 0;
            pEntry->SupportVHTMCS2SS = 0;
            pEntry->SupportVHTMCS3SS = 0;
            pEntry->SupportVHTMCS4SS = 0;

            for (j = wlan_config_get_tx_stream(pEntry->wdev); j > 0; j--)
            {
                switch (j)
                {
                    case 1:
                        if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 < VHT_MCS_CAP_NA)
                        {
                            for (i = 0; i <= 7; i++)
                            {                  
                                pEntry->SupportVHTMCS1SS |= 1 << i;
                            }

                            if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_8) 
                            {
                                pEntry->SupportVHTMCS1SS |= 1 << 8;
                            }
                            else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss1 == VHT_MCS_CAP_9)
                            {
                                pEntry->SupportVHTMCS1SS |= 1 << 8;
                                pEntry->SupportVHTMCS1SS |= 1 << 9;
                            }

                            pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
                        }
                        break;
                    case 2:
                        if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 < VHT_MCS_CAP_NA)
                        {
                            for (i = 0; i <= 7; i++)
                            {                  
                                pEntry->SupportVHTMCS2SS |= 1 << i;
                            }

                            if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 == VHT_MCS_CAP_8)
                            {
                                pEntry->SupportVHTMCS2SS |= 1 << 8;
                            }
                            else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss2 == VHT_MCS_CAP_9)
                            {
                                pEntry->SupportVHTMCS2SS |= 1 << 8;
                                pEntry->SupportVHTMCS2SS |= 1 << 9;
                            }

                            pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
                        }
                        break;
                    case 3:
                        if (vht_cap->mcs_set.rx_mcs_map.mcs_ss3 < VHT_MCS_CAP_NA)
                        {
                            for (i = 0; i <= 7; i++)
                            {                  
                                pEntry->SupportVHTMCS3SS |= 1 << i;
                            }

                            if (vht_cap->mcs_set.rx_mcs_map.mcs_ss3 == VHT_MCS_CAP_8)
                            {
                                pEntry->SupportVHTMCS3SS |= 1 << 8;
                            }
                            else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss3 == VHT_MCS_CAP_9)
                            {
                                pEntry->SupportVHTMCS3SS |= 1 << 8;
                                pEntry->SupportVHTMCS3SS |= 1 << 9;
                            }

                            pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
                        }
                        break;
                    case 4:
                        if (vht_cap->mcs_set.rx_mcs_map.mcs_ss4 < VHT_MCS_CAP_NA)
                        {
                            for (i = 0; i <= 7; i++)
                            {                  
                                pEntry->SupportVHTMCS4SS |= 1 << i;
                            }

                            if (vht_cap->mcs_set.rx_mcs_map.mcs_ss4 == VHT_MCS_CAP_8)
                            {
                                pEntry->SupportVHTMCS4SS |= 1 << 8;
                            }
                            else if (vht_cap->mcs_set.rx_mcs_map.mcs_ss4 == VHT_MCS_CAP_9)
                            {
                                pEntry->SupportVHTMCS4SS |= 1 << 8;
                                pEntry->SupportVHTMCS4SS |= 1 << 9;
                            }

                            pEntry->SupportRateMode |= SUPPORT_VHT_MODE;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
#endif /* DOT11_VHT_AC */
    }
}

