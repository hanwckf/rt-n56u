/** $Id: $
*/

/*! \file   "txbf_wrapper_embedded.c"
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
** $Log: txbf_wrapper_embedded.c $
**
** 06 29 2017 wish.chen
** [WCNCR00155771] [MT7615][BF] Beamform by channel
** 1) Purpose:
** Add new feature: beamform by channel
** 
** 2) Changed function name:
** AsicSwitchChannel
** SwitchBfByChannel
** Show_RAInfo_Proc
** RTMPSetProfileParameters
** RTMPSetProfileParameters
** mt_WrapSetETxBFCapByChannel
** mt_WrapSetVHTETxBFCapByChannel
** multi_profile_merge_bandnobf
** multi_profile_merge
** 
** 3) Code change description brief:
** Add new profile BandNoBf to indicate which 5G bands do not enable beamform
** 	
** 4) Unit Test Result:
** Test pass: profile test. Function correctness. DFS. Auto-channel.
**
** 11 13 2015 by.huang
** [WCNCR00053227] [MT7621+MT7615+MT7615][Noise][E3] Client WNDA3100v3 have many P1
** 	
** 	1) Purpose:
** 	When eBF enable parameter isn't enabled at profile.dat, eBF capability should not be claimed in IE
** 	2) Changed function name:
** 	mt_WrapSetETxBFCap(), mt_WrapSetVHTETxBFCap(), mt7615_setETxBFCap(), mt7615_setVHTETxBFCap()
** 	3) Code change description brief:
** 	When eBF enable parameter isn't enabled at profile.dat, eBF capability should not be claimed in IE
** 	4) Unit Test Result:
** 	Check the eBF capability in beacon and probe response
**
** 08 19 2014 kuani.lee
** [WCNCR00012358] [JEDI] TXBF Unify SW Integration
** .TXBF Unify SW Integration
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
#ifdef TXBF_SUPPORT
#ifdef MT_MAC
/*----------------------------------------------------------------------------*/

/*
	Wrap function for TxBFInit 
*/
VOID mt_WrapTxBFInit(
	IN PRTMP_ADAPTER 	pAd,
	IN MAC_TABLE_ENTRY	*pEntry,
	IN IE_LISTS         *ie_list,
	IN BOOLEAN			supportsETxBF)
{

    TXBF_MAC_TABLE_ENTRY TxBfMacEntry; 
    TXBF_STATUS_INFO  TxBfInfo;
    HT_BF_CAP *pTxBFCap = &ie_list->HTCapability.TxBFCap;
    UCHAR TxStream;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByWdev(pEntry->wdev);

		if (band_idx == DBDC_BAND0)
			TxStream = pAd->dbdc_2G_tx_stream;
		else
			TxStream = pAd->dbdc_5G_tx_stream;
	} else {
		TxStream = pAd->Antenna.field.TxPath;
	}

    TxBfInfo.ucTxPathNum = TxStream;
    TxBfInfo.ucETxBfTxEn = (UCHAR) pAd->CommonCfg.ETxBfEnCond;
    TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
    TxBfInfo.ucITxBfTxEn = pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;

    mt_TxBFInit(pAd, &TxBfInfo, &TxBfMacEntry, pTxBFCap, supportsETxBF);

    pEntry->bfState        = TxBfMacEntry.bfState;
	pEntry->sndgMcs        = TxBfMacEntry.sndgMcs;
	pEntry->sndg0Snr0      = TxBfMacEntry.sndg0Snr0;
	pEntry->sndg0Snr1      = TxBfMacEntry.sndg0Snr1;
	pEntry->sndg0Snr2      = TxBfMacEntry.sndg0Snr2;
	pEntry->sndg0Mcs       = TxBfMacEntry.sndg0Mcs;
    pEntry->noSndgCnt      = TxBfMacEntry.noSndgCnt;
    pEntry->eTxBfEnCond    = TxBfMacEntry.eTxBfEnCond;
    pEntry->noSndgCntThrd  = TxBfMacEntry.noSndgCntThrd;
    pEntry->ndpSndgStreams = TxBfMacEntry.ndpSndgStreams;
    pEntry->iTxBfEn        = TxBfMacEntry.iTxBfEn;
}

/* 
	Wrap function for clientSupportsETxBF
*/
BOOLEAN mt_WrapClientSupportsETxBF(
        IN  PRTMP_ADAPTER    pAd,
        IN  HT_BF_CAP       *pTxBFCap)
{    
    TXBF_STATUS_INFO  TxBfInfo;
    
    TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
    
    return mt_clientSupportsETxBF(pAd, pTxBFCap, TxBfInfo.cmmCfgETxBfNoncompress);
}


#ifdef VHT_TXBF_SUPPORT
/* 
	Wrap function for clientSupportsVHTETxBF
*/
BOOLEAN mt_WrapClientSupportsVhtETxBF(
        IN  PRTMP_ADAPTER    pAd,
        IN  VHT_CAP_INFO     *pTxBFCap)
{        
    return mt_clientSupportsVhtETxBF(pAd, pTxBFCap);
}
#endif /* VHT_TXBF_SUPPORT */


/*
	wrapper for mt_chk_itxbf_calibration
*/
BOOLEAN mt_Wrap_chk_itxbf_calibration(
	IN RTMP_ADAPTER *pAd)
{

    TXBF_STATUS_INFO TxBfInfo;
    TxBfInfo.u2Channel = pAd->CommonCfg.Channel;

    //return mt_chk_itxbf_calibration(pAd,&TxBfInfo);
    return TRUE;
}

/*
	wrapper for setETxBFCap 
*/
void mt_WrapSetETxBFCap(
    IN  RTMP_ADAPTER      *pAd,
    IN  struct wifi_dev   *wdev,
    IN  HT_BF_CAP         *pTxBFCap)
{
    TXBF_STATUS_INFO   TxBfInfo;
    UCHAR RxStream;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			RxStream = pAd->dbdc_2G_rx_stream;
		else
			RxStream = pAd->dbdc_5G_rx_stream;
	} else {
		RxStream = pAd->Antenna.field.RxPath;
	}

    TxBfInfo.pHtTxBFCap = pTxBFCap;
    TxBfInfo.cmmCfgETxBfEnCond= pAd->CommonCfg.ETxBfEnCond;
    TxBfInfo.cmmCfgETxBfNoncompress= pAd->CommonCfg.ETxBfNoncompress;
    TxBfInfo.ucRxPathNum = RxStream;

    pAd->chipOps.setETxBFCap(pAd, &TxBfInfo);

}    


#ifdef TXBF_BY_CHANNEL
/*
	does the same as mt_WrapSetETxBFCap but take Channel as parameter
*/
void mt_WrapSetETxBFCapByChannel(
    IN  RTMP_ADAPTER *pAd,
    IN  HT_BF_CAP *pTxBFCap,
    IN  UCHAR Channel)
{
    TXBF_STATUS_INFO   TxBfInfo;
    UCHAR RxStream;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByChannel(pAd, Channel);

		if (band_idx == DBDC_BAND0)
			RxStream = pAd->dbdc_2G_rx_stream;
		else
			RxStream = pAd->dbdc_5G_rx_stream;
	} else {
		RxStream = pAd->Antenna.field.RxPath;
	}

    TxBfInfo.pHtTxBFCap = pTxBFCap;
    TxBfInfo.cmmCfgETxBfEnCond= pAd->CommonCfg.ETxBfEnCond;
    TxBfInfo.cmmCfgETxBfNoncompress= pAd->CommonCfg.ETxBfNoncompress;
    TxBfInfo.ucRxPathNum = RxStream;

    pAd->chipOps.setETxBFCap(pAd, &TxBfInfo);
}    
#endif /* TXBF_BY_CHANNEL */


#ifdef VHT_TXBF_SUPPORT
/*
	Wrapper for mt_setVHTETxBFCap 
*/
void mt_WrapSetVHTETxBFCap(
    IN  RTMP_ADAPTER *pAd,
    IN  struct wifi_dev *wdev,
    IN  VHT_CAP_INFO *pTxBFCap)
{
    TXBF_STATUS_INFO   TxBfInfo;
    UCHAR TxStream;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			TxStream = pAd->dbdc_2G_tx_stream;
		else
			TxStream = pAd->dbdc_5G_tx_stream;
	} else {
		TxStream = pAd->Antenna.field.TxPath;
	}

    TxBfInfo.pVhtTxBFCap = pTxBFCap;
    TxBfInfo.ucTxPathNum = TxStream;
    TxBfInfo.cmmCfgETxBfEnCond= pAd->CommonCfg.ETxBfEnCond;    
    
    pAd->chipOps.setVHTETxBFCap(pAd,&TxBfInfo);
}


#ifdef TXBF_BY_CHANNEL
/*
 	does the same as mt_WrapSetVHTETxBFCap but take Channel as parameter
*/
void mt_WrapSetVHTETxBFCapByChannel(
    IN  RTMP_ADAPTER *pAd,
    IN  VHT_CAP_INFO *pTxBFCap,
    IN  UCHAR Channel)
{
    TXBF_STATUS_INFO   TxBfInfo;
    UCHAR TxStream;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByChannel(pAd, Channel);

		if (band_idx == DBDC_BAND0)
			TxStream = pAd->dbdc_2G_tx_stream;
		else
			TxStream = pAd->dbdc_5G_tx_stream;
	} else {
		TxStream = pAd->Antenna.field.TxPath;
	}

    TxBfInfo.pVhtTxBFCap = pTxBFCap;
    TxBfInfo.ucTxPathNum = TxStream;
    TxBfInfo.cmmCfgETxBfEnCond= pAd->CommonCfg.ETxBfEnCond;    
    
    pAd->chipOps.setVHTETxBFCap(pAd,&TxBfInfo);
}
#endif /* TXBF_BY_CHANNEL */

#endif /* VHT_TXBF_SUPPORT */

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
