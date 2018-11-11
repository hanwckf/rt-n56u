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
	mlme.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include <stdarg.h>
#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
#include "wnm.h"
#endif /* DOT11V_WNM_SUPPORT */

UCHAR CISCO_OUI[] = {0x00, 0x40, 0x96};
UCHAR RALINK_OUI[]  = {0x00, 0x0c, 0x43};
#if (defined(WH_EZ_SETUP) || defined(MWDS))
UCHAR MTK_OUI[]  = {0x00, 0x0c, 0xe7};
#endif
UCHAR WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
UCHAR RSN_OUI[] = {0x00, 0x0f, 0xac};
UCHAR WAPI_OUI[] = {0x00, 0x14, 0x72};
UCHAR WME_INFO_ELEM[]  = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR WME_PARM_ELEM[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR BROADCOM_OUI[]  = {0x00, 0x90, 0x4c};
UCHAR WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};


UCHAR OfdmRateToRxwiMCS[12] = {
	0,  0,	0,  0,
	0,  1,	2,  3,	/* OFDM rate 6,9,12,18 = rxwi mcs 0,1,2,3 */
	4,  5,	6,  7,	/* OFDM rate 24,36,48,54 = rxwi mcs 4,5,6,7 */
};

UCHAR RxwiMCSToOfdmRate[12] = {
	RATE_6,  RATE_9,	RATE_12,  RATE_18,
	RATE_24,  RATE_36,	RATE_48,  RATE_54,	/* OFDM rate 6,9,12,18 = rxwi mcs 0,1,2,3 */
	4,  5,	6,  7,	/* OFDM rate 24,36,48,54 = rxwi mcs 4,5,6,7 */
};





/* since RT61 has better RX sensibility, we have to limit TX ACK rate not to exceed our normal data TX rate.*/
/* otherwise the WLAN peer may not be able to receive the ACK thus downgrade its data TX rate*/
ULONG BasicRateMask[12] = {0xfffff001 /* 1-Mbps */, 0xfffff003 /* 2 Mbps */, 0xfffff007 /* 5.5 */, 0xfffff00f /* 11 */,
							0xfffff01f /* 6 */	 , 0xfffff03f /* 9 */	  , 0xfffff07f /* 12 */ , 0xfffff0ff /* 18 */,
							0xfffff1ff /* 24 */	 , 0xfffff3ff /* 36 */	  , 0xfffff7ff /* 48 */ , 0xffffffff /* 54 */};

UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* e.g. RssiSafeLevelForTxRate[RATE_36]" means if the current RSSI is greater than*/
/*		this value, then it's quaranteed capable of operating in 36 mbps TX rate in*/
/*		clean environment.*/
/*								  TxRate: 1   2   5.5	11	 6	  9    12	18	 24   36   48	54	 72  100*/
CHAR RssiSafeLevelForTxRate[] ={  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

UCHAR  RateIdToMbps[] = { 1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54, 72, 100};
USHORT RateIdTo500Kbps[] = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 144, 200};

UCHAR SsidIe = IE_SSID;
UCHAR SupRateIe = IE_SUPP_RATES;
UCHAR ExtRateIe = IE_EXT_SUPP_RATES;
#ifdef DOT11_N_SUPPORT
UCHAR HtCapIe = IE_HT_CAP;
UCHAR AddHtInfoIe = IE_ADD_HT;
UCHAR NewExtChanIe = IE_SECONDARY_CH_OFFSET;
UCHAR BssCoexistIe = IE_2040_BSS_COEXIST;
UCHAR ExtHtCapIe = IE_EXT_CAPABILITY;
#endif /* DOT11_N_SUPPORT */
UCHAR ExtCapIe = IE_EXT_CAPABILITY;
UCHAR ErpIe = IE_ERP;
UCHAR DsIe = IE_DS_PARM;
UCHAR TimIe = IE_TIM;
UCHAR WpaIe = IE_WPA;
UCHAR Wpa2Ie = IE_WPA2;
UCHAR IbssIe = IE_IBSS_PARM;
UCHAR WapiIe = IE_WAPI;

extern UCHAR	WPA_OUI[];
extern UCHAR SES_OUI[];

UCHAR ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


#ifdef DYNAMIC_VGA_SUPPORT
void periodic_monitor_false_cca_adjust_vga(RTMP_ADAPTER *pAd)
{
	if ((pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable) && (pAd->chipCap.dynamic_vga_support) &&
		OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		UCHAR val1, val2;
		UINT32 bbp_val1, bbp_val2;

		RTMP_BBP_IO_READ32(pAd, AGC1_R8, &bbp_val1);
		val1 = ((bbp_val1 & (0x00007f00)) >> 8) & 0x7f;
		RTMP_BBP_IO_READ32(pAd, AGC1_R9, &bbp_val2);
		val2 = ((bbp_val2 & (0x00007f00)) >> 8) & 0x7f;

		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("vga_init_0 = %x, vga_init_1 = %x\n",  pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0, pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1));
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("one second False CCA=%d, fixed agc_vga_0:0%x, fixed agc_vga_1:0%x\n", pAd->RalinkCounters.OneSecFalseCCACnt, val1, val2));

		if (pAd->RalinkCounters.OneSecFalseCCACnt > pAd->CommonCfg.lna_vga_ctl.nFalseCCATh) {
			if (val1 > (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0 - 0x10)) {
				val1 -= 0x02;
				bbp_val1 = (bbp_val1 & 0xffff80ff) | (val1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val1);
#ifdef DFS_SUPPORT
       		pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif
			}

			if (pAd->Antenna.field.RxPath >= 2) {
				if (val2 > (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1 - 0x10)) {
					val2 -= 0x02;
					bbp_val2 = (bbp_val2 & 0xffff80ff) | (val2 << 8);
					RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val2);
				}
			}
		} else if (pAd->RalinkCounters.OneSecFalseCCACnt <
					pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh) {
			if (val1 < pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0) {
				val1 += 0x02;
				bbp_val1 = (bbp_val1 & 0xffff80ff) | (val1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val1);
#ifdef DFS_SUPPORT
				pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif
			}

			if (pAd->Antenna.field.RxPath >= 2) {
				if (val2 < pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1) {
					val2 += 0x02;
					bbp_val2 = (bbp_val2 & 0xffff80ff) | (val2 << 8);
					RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val2);
				}
			}
		}
	}
}

#endif


VOID set_default_ap_edca_param(EDCA_PARM *pEdca)
{
	pEdca->bValid = TRUE;
	pEdca->Aifsn[0] = 3;
	pEdca->Aifsn[1] = 7;
	pEdca->Aifsn[2] = 1;
	pEdca->Aifsn[3] = 1;

	pEdca->Cwmin[0] = 4;
	pEdca->Cwmin[1] = 4;
	pEdca->Cwmin[2] = 3;
	pEdca->Cwmin[3] = 2;

	pEdca->Cwmax[0] = 6;
	pEdca->Cwmax[1] = 10;
	pEdca->Cwmax[2] = 4;
	pEdca->Cwmax[3] = 3;

	pEdca->Txop[0]  = 0;
	pEdca->Txop[1]  = 0;
	pEdca->Txop[2]  = 94;
	pEdca->Txop[3]  = 47;
}


VOID set_default_sta_edca_param(EDCA_PARM *pEdca)
{
	pEdca->bValid= TRUE;
	pEdca->Aifsn[0] = 3;
	pEdca->Aifsn[1] = 7;
	pEdca->Aifsn[2] = 2;
	pEdca->Aifsn[3] = 2;

	pEdca->Cwmin[0] = 4;
	pEdca->Cwmin[1] = 4;
	pEdca->Cwmin[2] = 3;
	pEdca->Cwmin[3] = 2;

	pEdca->Cwmax[0] = 10;
	pEdca->Cwmax[1] = 10;
	pEdca->Cwmax[2] = 4;
	pEdca->Cwmax[3] = 3;

	pEdca->Txop[0]  = 0;
	pEdca->Txop[1]  = 0;
	pEdca->Txop[2]  = 94;	/*96; */
	pEdca->Txop[3]  = 47;	/*48; */
}


UCHAR dot11_max_sup_rate(INT SupRateLen, UCHAR *SupRate, INT ExtRateLen, UCHAR *ExtRate)
{
	INT idx;
	UCHAR MaxSupportedRateIn500Kbps = 0;

	/* supported rates array may not be sorted. sort it and find the maximum rate */
	for (idx = 0; idx < SupRateLen; idx++) {
		if (MaxSupportedRateIn500Kbps < (SupRate[idx] & 0x7f))
			MaxSupportedRateIn500Kbps = SupRate[idx] & 0x7f;
	}

	if (ExtRateLen > 0 && ExtRate != NULL)
	{
		for (idx = 0; idx < ExtRateLen; idx++) {
			if (MaxSupportedRateIn500Kbps < (ExtRate[idx] & 0x7f))
				MaxSupportedRateIn500Kbps = ExtRate[idx] & 0x7f;
		}
	}

	return MaxSupportedRateIn500Kbps;
}


UCHAR dot11_2_ra_rate(UCHAR MaxSupportedRateIn500Kbps)
{
	UCHAR MaxSupportedRate;

	switch (MaxSupportedRateIn500Kbps)
	{
		case 108: MaxSupportedRate = RATE_54;   break;
		case 96:  MaxSupportedRate = RATE_48;   break;
		case 72:  MaxSupportedRate = RATE_36;   break;
		case 48:  MaxSupportedRate = RATE_24;   break;
		case 36:  MaxSupportedRate = RATE_18;   break;
		case 24:  MaxSupportedRate = RATE_12;   break;
		case 18:  MaxSupportedRate = RATE_9;    break;
		case 12:  MaxSupportedRate = RATE_6;    break;
		case 22:  MaxSupportedRate = RATE_11;   break;
		case 11:  MaxSupportedRate = RATE_5_5;  break;
		case 4:   MaxSupportedRate = RATE_2;    break;
		case 2:   MaxSupportedRate = RATE_1;    break;
		default:  MaxSupportedRate = RATE_11;   break;
	}

	return MaxSupportedRate;
}


/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPSuspendMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("SCANNING, suspend MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier */
	/* no carrier detection when scanning */
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStop(pAd);
#endif
#endif /* CONFIG_AP_SUPPORT */

	/*
		Before BSS_SCAN_IN_PROGRESS, we need to keep Current R66 value and
		use Lowbound as R66 value on ScanNextChannel(...)
	*/
	bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);

	pAd->hw_cfg.bbp_bw = pAd->CommonCfg.BBPCurrentBW;

	RTMPSetAGCInitValue(pAd, BW_20);

	MSDU_FORBID_SET(wdev, MSDU_FORBID_CHANNEL_MISMATCH);

	RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPResumeMsduTransmission(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("SCAN done, resume MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
	/* no carrier detection when scanning*/
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
			CarrierDetectionStart(pAd);
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
		R66 should not be 0
	*/
	if (pAd->BbpTuning.R66CurrentValue == 0)
	{
		pAd->BbpTuning.R66CurrentValue = 0x38;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}
	bbp_set_agc(pAd, pAd->BbpTuning.R66CurrentValue, RX_CHAIN_ALL);

	pAd->CommonCfg.BBPCurrentBW = pAd->hw_cfg.bbp_bw;

	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CHANNEL_MISMATCH);

	RTMP_OS_NETDEV_WAKE_QUEUE(wdev->if_dev);

/* sample, for IRQ LOCK to SEM LOCK */
/*
	IrqState = pAd->irq_disabled;
	if (IrqState)
		RTMPDeQueuePacket(pAd, TRUE, WMM_NUM_OF_AC, MAX_TX_PROCESS);
	else
*/
	RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
}


/*
	==========================================================================
	Description:
		Send out a NULL frame to a specified STA at a higher TX rate. The
		purpose is to ensure the designated client is okay to received at this
		rate.
	==========================================================================
 */
VOID RtmpEnqueueNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP)
{
	NDIS_STATUS NState;
	HEADER_802_11 *pNullFr;
	UCHAR *pFrame;
	UINT frm_len;
	MAC_TABLE_ENTRY *pEntry;
#ifdef CONFIG_AP_SUPPORT
	pEntry = MacTableLookup(pAd, pAddr);
#endif

	NState = MlmeAllocateMemory(pAd, (UCHAR **)&pFrame);
	pNullFr = (PHEADER_802_11) pFrame;

	if (NState == NDIS_STATUS_SUCCESS)
	{
		frm_len = sizeof(HEADER_802_11);

#ifdef CONFIG_AP_SUPPORT
//		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		if (pEntry && (pEntry->wdev->wdev_type == WDEV_TYPE_AP
						|| pEntry->wdev->wdev_type == WDEV_TYPE_GO))
		{
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr,
							pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.ToDs = 0;
			pNullFr->FC.FrDs = 1;
			goto body;
		}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
body:
#endif /* CONFIG_AP_SUPPORT */


		pNullFr->FC.Type = FC_TYPE_DATA;

		if (bQosNull)
		{
			UCHAR *qos_p = ((UCHAR *)pNullFr) + frm_len;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			frm_len += 2;
		} else
			pNullFr->FC.SubType = SUBTYPE_DATA_NULL;

		/* since TxRate may change, we have to change Duration each time */
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, frm_len);

		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("send NULL Frame @%d Mbps to AID#%d...\n", RateIdToMbps[TxRate], AID & 0x3f));
		MiniportMMRequest(pAd, WMM_UP2AC_MAP[7], (PUCHAR)pNullFr, frm_len);

		MlmeFreeMemory( pFrame);
	}
}


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
VOID ApCliRTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN 		bQosNull,
	IN 	PMAC_TABLE_ENTRY pMacEntry,
	IN	USHORT			PwrMgmt)
{
	UCHAR NullFrame[48];
	ULONG Length;
	HEADER_802_11 *wifi_hdr;
	STA_TR_ENTRY *tr_entry;
	PAPCLI_STRUCT pApCliEntry = NULL;
	struct wifi_dev *wdev;

	if (!pMacEntry)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
	wdev = &pApCliEntry->wdev;

    /* WPA 802.1x secured port control */
	// TODO: shiang-usw, check [wdev/tr_entry]->PortSecured!
	if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED) ||
	    (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		return;

	NdisZeroMemory(NullFrame, 48);
	Length = sizeof(HEADER_802_11);

	wifi_hdr = (HEADER_802_11 *)NullFrame;

	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = SUBTYPE_DATA_NULL;
	wifi_hdr->FC.ToDs = 1;

	COPY_MAC_ADDR(wifi_hdr->Addr1, pMacEntry->Addr);
#ifdef MAC_REPEATER_SUPPORT
	if (pMacEntry && (pMacEntry->bReptCli == TRUE))
		COPY_MAC_ADDR(wifi_hdr->Addr2, pMacEntry->ReptCliAddr);
	else
#endif /* MAC_REPEATER_SUPPORT */
		COPY_MAC_ADDR(wifi_hdr->Addr2, pApCliEntry->wdev.if_addr);
	COPY_MAC_ADDR(wifi_hdr->Addr3, pMacEntry->Addr);

	if (pAd->CommonCfg.bAPSDForcePowerSave)
		wifi_hdr->FC.PwrMgmt = PWR_SAVE;
	else
		wifi_hdr->FC.PwrMgmt = PwrMgmt;
	wifi_hdr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);

	/* sequence is increased in MlmeHardTx */
	wifi_hdr->Sequence = pAd->Sequence;
	pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ;	/* next sequence  */

	/* Prepare QosNull function frame */
	if (bQosNull) {
		wifi_hdr->FC.SubType = SUBTYPE_QOS_NULL;

		/* copy QOS control bytes */
		NullFrame[Length] = 0;
		NullFrame[Length + 1] = 0;
		Length += 2;	/* if pad with 2 bytes for alignment, APSD will fail */
	}

	HAL_KickOutNullFrameTx(pAd, 0, NullFrame, Length);
}
#endif/*APCLI_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */




/*
	==========================================================================
	Description:
		main loop of the MLME
	Pre:
		Mlme has to be initialized, and there are something inside the queue
	Note:
		This function is invoked from MPSetInformation and MPReceive;
		This task guarantee only one FSM will run.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#ifdef WH_EZ_SETUP 
#ifdef DUAL_CHIP
extern NDIS_SPIN_LOCK ez_mlme_sync_lock;
#endif
#endif
VOID MlmeHandler(RTMP_ADAPTER *pAd)
{
	MLME_QUEUE_ELEM *Elem = NULL;
#ifdef APCLI_SUPPORT
	SHORT apcliIfIndex;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UCHAR CliIdx = 0xff;
	RTMP_CHIP_CAP *cap = &pAd->chipCap;
#endif /*MAC_REPEATER_SUPPORT*/
#endif /* APCLI_SUPPORT */

	/* Only accept MLME and Frame from peer side, no other (control/data) frame should*/
	/* get into this state machine*/
#ifdef WH_EZ_SETUP 	
#ifdef DUAL_CHIP
#ifndef EZ_MOD_SUPPORT
		PRTMP_ADAPTER OtherBandAd = NULL;
#endif
	if (IS_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[0].wdev) )
	{
		if (IS_DUAL_CHIP_DBDC(pAd) && !ez_is_triband())
		{
#ifndef EZ_MOD_SUPPORT
			OtherBandAd = pAd->pAdOthBand;
			NdisAcquireSpinLock(&ez_mlme_sync_lock);
#else
			ez_acquire_lock(pAd, NULL, MLME_SYNC_LOCK);
#endif
		}
	}
#endif
#endif
	
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning)
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#ifdef WH_EZ_SETUP 		
#ifdef DUAL_CHIP
				if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[0].wdev))
				{
					if (IS_DUAL_CHIP_DBDC(pAd) && !ez_is_triband())
					{
#ifdef EZ_MOD_SUPPORT					
						ez_release_lock(pAd, NULL, MLME_SYNC_LOCK);
#else
						NdisReleaseSpinLock(&ez_mlme_sync_lock);
#endif
					}
				}
#endif		
#endif
		return;
	}
	else
	{
#ifdef WH_EZ_SETUP 	
#ifdef DUAL_CHIP
	if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[0].wdev))
	{
		if (IS_DUAL_CHIP_DBDC(pAd) && !ez_is_triband())
		{
#ifdef EZ_MOD_SUPPORT		
			if(ez_is_other_band_mlme_running(&pAd->ApCfg.MBSSID[0].wdev))
			{
				NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
				ez_release_lock(pAd, NULL, MLME_SYNC_LOCK);
				//NdisReleaseSpinLock(&ez_mlme_sync_lock);
				return;
			}
#else	
			if(OtherBandAd->Mlme.bRunning)
			{
				NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
				NdisReleaseSpinLock(&ez_mlme_sync_lock);
				return;
			}
#endif
		}
	}
#endif
#endif
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#ifdef WH_EZ_SETUP 		
#ifdef DUAL_CHIP
	if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[0].wdev))
	{
		if (IS_DUAL_CHIP_DBDC(pAd) && !ez_is_triband())
		{
#ifdef EZ_MOD_SUPPORT
			ez_release_lock(pAd, NULL, MLME_SYNC_LOCK);
#else					
			NdisReleaseSpinLock(&ez_mlme_sync_lock);
#endif
		}
	}
#endif		
#endif
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue))
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND) ||
			!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		{
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("System halted, removed or MlmeRest, exit MlmeTask!(QNum = %ld)\n",
						pAd->Mlme.Queue.Num));
			break;
		}

#ifdef CONFIG_ATE
		if(ATE_ON(pAd))
		{
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Driver is in ATE mode\n", __FUNCTION__));
			break;
		}
#endif /* CONFIG_ATE */

		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem))
		{


			/* if dequeue success*/
			switch (Elem->Machine)
			{
				/* STA state machines*/

				case ACTION_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ActMachine,
										Elem, pAd->Mlme.ActMachine.CurrState);
					break;

#ifdef CONFIG_AP_SUPPORT
				/* AP state amchines*/

				case AP_ASSOC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApAssocMachine,
									Elem, pAd->Mlme.ApAssocMachine.CurrState);
					break;

				case AP_AUTH_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApAuthMachine,
									Elem, pAd->Mlme.ApAuthMachine.CurrState);
					break;

				case AP_SYNC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApSyncMachine,
									Elem, pAd->Mlme.ApSyncMachine.CurrState);
					break;
#ifdef WH_EZ_SETUP	
#ifdef EZ_MOD_SUPPORT
				case EZ_STATE_MACHINE:
//! Levarage from MP1.0 CL#170063
					StateMachinePerformAction(pAd, &pAd->Mlme.EzMachine,
									Elem, pAd->Mlme.EzMachine.CurrState);
					break;					

#else
				case EZ_ROAM_STATE_MACHINE:
//! Levarage from MP1.0 CL#170063
					StateMachinePerformAction(pAd, &pAd->Mlme.EzRoamMachine,
									Elem, pAd->Mlme.EzRoamMachine.CurrState);
					break;					
				case AP_TRIBAND_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApTriBandMachine,
									Elem, pAd->Mlme.ApTriBandMachine.CurrState);
					break;

#endif
#endif

#ifdef APCLI_SUPPORT
				case APCLI_AUTH_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;			
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= REPT_MLME_START_IDX) {
						CliIdx = (apcliIfIndex - REPT_MLME_START_IDX);
						ASSERT(CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap));

						pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

						if(pReptEntry->CliEnable == FALSE)
						{
							MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR
							, ("%s(): mlme sm %ld trying to access a non-inserted reptEntry , please check.\n" 
							,__FUNCTION__,Elem->Machine));
							break;
						}
						apcliIfIndex = pReptEntry->wdev->func_idx;
						
					}
                    else
                    {                    	
                        CliIdx = 0xff;
                    }
#endif /* MAC_REPEATER_SUPPORT */


					if(isValidApCliIf(apcliIfIndex))
					{
						ULONG AuthCurrState;
#ifdef MAC_REPEATER_SUPPORT
						if (CliIdx != 0xff)
						{
							AuthCurrState = pReptEntry->AuthCurrState;
						}
						else
#endif /* MAC_REPEATER_SUPPORT */
							AuthCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AuthCurrState;

						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAuthMachine,
								Elem, AuthCurrState);
					}
					break;

				case APCLI_ASSOC_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= REPT_MLME_START_IDX) {
						CliIdx = (apcliIfIndex - REPT_MLME_START_IDX);
						ASSERT(CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap));

						pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
						if(pReptEntry->CliEnable == FALSE)
						{
							MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR
							, ("%s(): mlme sm %ld trying to access a non-inserted reptEntry , please check.\n" 
							,__FUNCTION__,Elem->Machine));
							break;
						}
						apcliIfIndex = pReptEntry->wdev->func_idx;
					}
                    else
                    {
                        CliIdx = 0xff;
                    }
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))
					{
						ULONG AssocCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AssocCurrState;
#ifdef MAC_REPEATER_SUPPORT
						if (CliIdx != 0xff)
						{
							AssocCurrState = pReptEntry->AssocCurrState;
						}
#endif /* MAC_REPEATER_SUPPORT */
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAssocMachine,
								Elem, AssocCurrState);
					}
					break;

				case APCLI_SYNC_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
					if(isValidApCliIf(apcliIfIndex))
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliSyncMachine, Elem,
							(pAd->ApCfg.ApCliTab[apcliIfIndex].SyncCurrState));
					break;

				case APCLI_CTRL_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= REPT_MLME_START_IDX) {
						CliIdx = (apcliIfIndex - REPT_MLME_START_IDX);
						ASSERT(CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap));

						pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
						if(pReptEntry->CliEnable == FALSE)
						{
							MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR
							, ("%s(): mlme sm %ld trying to access a non-inserted reptEntry , please check.\n" 
							,__FUNCTION__,Elem->Machine));
							break;
						}
						apcliIfIndex = pReptEntry->wdev->func_idx;
					}
                    else
                    {
                        CliIdx = 0xff;
                    }
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))
					{
						ULONG CtrlCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].CtrlCurrState;
#ifdef MAC_REPEATER_SUPPORT
						if (CliIdx != 0xff)
						{
							CtrlCurrState = pReptEntry->CtrlCurrState;
						}
#endif /* MAC_REPEATER_SUPPORT */
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliCtrlMachine, Elem, CtrlCurrState);
					}
					break;
#endif /* APCLI_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */
				case WPA_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.WpaMachine, Elem, pAd->Mlme.WpaMachine.CurrState);
					break;
#ifdef WSC_INCLUDED
                case WSC_STATE_MACHINE:
					if (pAd->pWscElme)
					{
						RTMP_SEM_LOCK(&pAd->WscElmeLock);
						NdisMoveMemory(pAd->pWscElme, Elem, sizeof(MLME_QUEUE_ELEM));
						RTMP_SEM_UNLOCK(&pAd->WscElmeLock);
						RtmpOsTaskWakeUp(&(pAd->wscTask));
					}
                    break;
#endif /* WSC_INCLUDED */



#ifdef CONFIG_HOTSPOT
				case HSCTRL_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.HSCtrlMachine, Elem,
									HSCtrlCurrentState(pAd, Elem));
					break;
#endif

#ifdef CONFIG_DOT11U_INTERWORKING
				case GAS_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.GASMachine, Elem,
										GASPeerCurrentState(pAd, Elem));
					break;
#endif

#ifdef CONFIG_DOT11V_WNM
				case BTM_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.BTMMachine, Elem,
										BTMPeerCurrentState(pAd, Elem));
					break;
#ifdef CONFIG_HOTSPOT_R2
				case WNM_NOTIFY_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.WNMNotifyMachine, Elem,
										WNMNotifyPeerCurrentState(pAd, Elem));
					break;
#endif
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
				case BGND_SCAN_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->BgndScanCtrl.BgndScanStatMachine, Elem,
										pAd->BgndScanCtrl.BgndScanStatMachine.CurrState);
					break;
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef MT_DFS_SUPPORT
				case DFS_STATE_MACHINE: //Jelly20150402
					StateMachinePerformAction(pAd, &pAd->DfsParameter.DfsStatMachine, Elem,
										pAd->DfsParameter.DfsStatMachine.CurrState);
					break;
#endif /* MT_DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
				case AUTO_CH_SEL_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->AutoChSelCtrl.AutoChScanStatMachine, Elem,
									pAd->AutoChSelCtrl.AutoChScanStatMachine.CurrState);
                    break;
#endif /* CONFIG_AP_SUPPORT */
				default:
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Illegal SM %ld\n",
								__FUNCTION__, Elem->Machine));
					break;
			} /* end of switch*/

			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): MlmeQ empty\n", __FUNCTION__));
		}
	}

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
}


/*
========================================================================
Routine Description:
    MLME kernel thread.

Arguments:
	Context			the pAd, driver control block pointer

Return Value:
    0					close the thread

Note:
========================================================================
*/
static INT MlmeThread(ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	if (pAd == NULL)
		goto LabelExit;

	RtmpOSTaskCustomize(pTask);

	while (!RTMP_OS_TASK_IS_KILLED(pTask))
	{
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		/* lock the device pointers , need to check if required*/
		/*down(&(pAd->usbdev_semaphore)); */

		if (!pAd->PM_FlgSuspend)
			MlmeHandler(pAd);
	}

	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
LabelExit:
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,( "<---%s\n",__FUNCTION__));
	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}



#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
static VOID ApCliMlmeInit(RTMP_ADAPTER *pAd)
{
	/* init apcli state machines*/
        ASSERT(APCLI_AUTH_FUNC_SIZE == APCLI_MAX_AUTH_MSG * APCLI_MAX_AUTH_STATE);
        ApCliAuthStateMachineInit(pAd, &pAd->Mlme.ApCliAuthMachine, pAd->Mlme.ApCliAuthFunc);

        ASSERT(APCLI_ASSOC_FUNC_SIZE == APCLI_MAX_ASSOC_MSG * APCLI_MAX_ASSOC_STATE);
        ApCliAssocStateMachineInit(pAd, &pAd->Mlme.ApCliAssocMachine, pAd->Mlme.ApCliAssocFunc);

        ASSERT(APCLI_SYNC_FUNC_SIZE == APCLI_MAX_SYNC_MSG * APCLI_MAX_SYNC_STATE);
        ApCliSyncStateMachineInit(pAd, &pAd->Mlme.ApCliSyncMachine, pAd->Mlme.ApCliSyncFunc);

        ASSERT(APCLI_CTRL_FUNC_SIZE == APCLI_MAX_CTRL_MSG * APCLI_MAX_CTRL_STATE);
        ApCliCtrlStateMachineInit(pAd, &pAd->Mlme.ApCliCtrlMachine, pAd->Mlme.ApCliCtrlFunc);
}
#endif /* APCLI_SUPPORT */

static VOID ApMlmeInit(RTMP_ADAPTER *pAd)
{
	/* init AP state machines*/
        APAssocStateMachineInit(pAd, &pAd->Mlme.ApAssocMachine, pAd->Mlme.ApAssocFunc);
        APAuthStateMachineInit(pAd, &pAd->Mlme.ApAuthMachine, pAd->Mlme.ApAuthFunc);
        APSyncStateMachineInit(pAd, &pAd->Mlme.ApSyncMachine, pAd->Mlme.ApSyncFunc);
#ifdef WH_EZ_SETUP
#ifdef EZ_MOD_SUPPORT
		EzStateMachineInit(pAd, &pAd->Mlme.EzMachine, pAd->Mlme.EzFunc);
#else
//! Levarage from MP1.0 CL#170063
		EzRoamStateMachineInit(pAd, &pAd->Mlme.EzRoamMachine, pAd->Mlme.EzRoamFunc);
		APTriBandStateMachineInit(pAd, &pAd->Mlme.ApTriBandMachine, pAd->Mlme.ApTriBandFunc);
#endif
#endif		
}
#endif /* CONFIG_AP_SUPPORT */

/*
	==========================================================================
	Description:
		initialize the MLME task and its data structure (queue, spinlock,
		timer, state machines).

	IRQL = PASSIVE_LEVEL

	Return:
		always return NDIS_STATUS_SUCCESS

	==========================================================================
*/
NDIS_STATUS MlmeInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> MLME Initialize\n"));

	do
	{
		Status = MlmeQueueInit(pAd, &pAd->Mlme.Queue);
		if(Status != NDIS_STATUS_SUCCESS)
			break;

		pAd->Mlme.bRunning = FALSE;
		NdisAllocateSpinLock(pAd, &pAd->Mlme.TaskLock);


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			ApMlmeInit(pAd);
#ifdef APCLI_SUPPORT
			ApCliMlmeInit(pAd);
#endif /* APCLI_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */

#ifdef WSC_INCLUDED
		/* Init Wsc state machine */
		ASSERT(WSC_FUNC_SIZE == MAX_WSC_MSG * MAX_WSC_STATE);
		WscStateMachineInit(pAd, &pAd->Mlme.WscMachine, pAd->Mlme.WscFunc);
#endif /* WSC_INCLUDED */

		WpaStateMachineInit(pAd, &pAd->Mlme.WpaMachine, pAd->Mlme.WpaFunc);

#ifdef CONFIG_HOTSPOT
		HSCtrlStateMachineInit(pAd, &pAd->Mlme.HSCtrlMachine, pAd->Mlme.HSCtrlFunc);
		GASStateMachineInit(pAd, &pAd->Mlme.GASMachine, pAd->Mlme.GASFunc);
#endif

#ifdef DSCP_QOS_MAP_SUPPORT
		DscpQosMapInit(pAd);	
#endif

#ifdef CONFIG_DOT11V_WNM
		WNMCtrlInit(pAd);
		BTMStateMachineInit(pAd, &pAd->Mlme.BTMMachine, pAd->Mlme.BTMFunc);
#ifdef CONFIG_HOTSPOT_R2
		WNMNotifyStateMachineInit(pAd, &pAd->Mlme.WNMNotifyMachine, pAd->Mlme.WNMNotifyFunc);
#endif
#endif


		ActionStateMachineInit(pAd, &pAd->Mlme.ActMachine, pAd->Mlme.ActFunc);

		/* Init mlme periodic timer*/
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExecTimer), pAd, TRUE);

		/* Set mlme periodic timer*/
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

		/* software-based RX Antenna diversity*/
		RTMPInitTimer(pAd, &pAd->Mlme.RxAntEvalTimer, GET_TIMER_FUNCTION(AsicRxAntEvalTimeout), pAd, FALSE);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* Init APSD periodic timer*/
			RTMPInitTimer(pAd, &pAd->Mlme.APSDPeriodicTimer, GET_TIMER_FUNCTION(APSDPeriodicExec), pAd, TRUE);
			RTMPSetTimer(&pAd->Mlme.APSDPeriodicTimer, 50);

			/* Init APQuickResponseForRateUp timer.*/
			RTMPInitTimer(pAd, &pAd->ApCfg.ApQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(APQuickResponeForRateUpExec), pAd, FALSE);
			pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
		}
#endif /* CONFIG_AP_SUPPORT */


#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	/*CFG_TODO*/
	ApMlmeInit(pAd);

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
	ApCliMlmeInit(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#endif /* RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */

	} while (FALSE);

	{
		RTMP_OS_TASK *pTask;

		/* Creat MLME Thread */
		pTask = &pAd->mlmeTask;
		RTMP_OS_TASK_INIT(pTask, "RtmpMlmeTask", pAd);
		Status = RtmpOSTaskAttach(pTask, MlmeThread, (ULONG)pTask);
		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unable to start MlmeThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
		}
	}
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<-- MLME Initialize\n"));
	RTMP_SET_FLAG(pAd,fRTMP_ADAPTER_SYSEM_READY);

	return Status;
}


/*
	==========================================================================
	Description:
		Destructor of MLME (Destroy queue, state machine, spin lock and timer)
	Parameters:
		Adapter - NIC Adapter pointer
	Post:
		The MLME task will no longer work properly

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID MlmeHalt(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancelled;
	RTMP_OS_TASK *pTask;


	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> MlmeHalt\n"));
	RTMP_CLEAR_FLAG(pAd,fRTMP_ADAPTER_SYSEM_READY);
	/* Terminate Mlme Thread */
	pTask = &pAd->mlmeTask;
	if (RtmpOSTaskKill(pTask) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("kill mlme task failed!\n"));
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		/* disable BEACON generation and other BEACON related hardware timers*/
		AsicDisableSync(pAd, HW_BSSID_0);
	}

	RTMPReleaseTimer(&pAd->Mlme.PeriodicTimer, &Cancelled);



	RTMPReleaseTimer(&pAd->Mlme.RxAntEvalTimer, &Cancelled);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		UCHAR idx;
		idx = 0;
		RTMPReleaseTimer(&pAd->Mlme.APSDPeriodicTimer, &Cancelled);
		RTMPReleaseTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);

#ifdef APCLI_SUPPORT
		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
		{
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
			RTMPReleaseTimer(&pApCliEntry->MlmeAux.ProbeTimer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->MlmeAux.ApCliAssocTimer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->MlmeAux.ApCliAuthTimer, &Cancelled);
#ifdef APCLI_CERT_SUPPORT
			RTMPReleaseTimer(&pApCliEntry->MlmeAux.WpaDisassocAndBlockAssocTimer, &Cancelled);
#endif /* APCLI_CERT_SUPPORT */

#ifdef APCLI_CONNECTION_TRIAL
			RTMPReleaseTimer(&pApCliEntry->TrialConnectTimer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->TrialConnectPhase2Timer, &Cancelled);
			RTMPReleaseTimer(&pApCliEntry->TrialConnectRetryTimer, &Cancelled);
#endif /* APCLI_CONNECTION_TRIAL */

#ifdef WSC_AP_SUPPORT
			if (pApCliEntry->WscControl.WscProfileRetryTimerRunning)
		{
				pApCliEntry->WscControl.WscProfileRetryTimerRunning = FALSE;
				RTMPReleaseTimer(&pApCliEntry->WscControl.WscProfileRetryTimer, &Cancelled);
		}
#endif /* WSC_AP_SUPPORT */
		}
#endif /* APCLI_SUPPORT */
		RTMPReleaseTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);
	}

#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_HOTSPOT
	HSCtrlHalt(pAd);
	HSCtrlExit(pAd);
#endif

#ifdef CONFIG_DOT11U_INTERWORKING
	GASCtrlExit(pAd);
#endif

#ifdef CONFIG_DOT11V_WNM
	WNMCtrlExit(pAd);
#endif


	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

#ifdef LED_CONTROL_SUPPORT
		/* Set LED*/
		RTMPSetLED(pAd, LED_HALT);
		RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, firmware is not done it.*/
#endif /* LED_CONTROL_SUPPORT */

		if ((pChipOps->AsicHaltAction))
			pChipOps->AsicHaltAction(pAd);
	}

	RtmpusecDelay(5000);    /*  5 msec to gurantee Ant Diversity timer canceled*/

	MlmeQueueDestroy(&pAd->Mlme.Queue);
	NdisFreeSpinLock(&pAd->Mlme.TaskLock);

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== MlmeHalt\n"));
}


VOID MlmeResetRalinkCounters(RTMP_ADAPTER *pAd)
{
	pAd->RalinkCounters.LastOneSecRxOkDataCnt = pAd->RalinkCounters.OneSecRxOkDataCnt;

#ifdef CONFIG_ATE
	if (!ATE_ON(pAd))
#endif /* CONFIG_ATE */
		/* for performace enchanement */
		NdisZeroMemory(&pAd->RalinkCounters,
						(LONG)&pAd->RalinkCounters.OneSecEnd -
						(LONG)&pAd->RalinkCounters.OneSecStart);

	return;
}


/*
	==========================================================================
	Description:
		This routine is executed periodically to -
		1. Decide if it's a right time to turn on PwrMgmt bit of all
		   outgoiing frames
		2. Calculate ChannelQuality based on statistics of the last
		   period, so that TX rate won't toggling very frequently between a
		   successful TX and a failed TX.
		3. If the calculated ChannelQuality indicated current connection not
		   healthy, then a ROAMing attempt is tried here.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#define ADHOC_BEACON_LOST_TIME		(8*OS_HZ)  /* 8 sec*/
#ifdef MLME_BY_CMDTHREAD
#else
VOID MlmePeriodicExecTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MlmePeriodicExec(SystemSpecific1, FunctionContext, SystemSpecific2, SystemSpecific3);
}
#endif /*MLME_BY_CMDTHREAD */

#ifdef MLME_BY_CMDTHREAD
NTSTATUS  MlmePeriodicExec(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
#else
VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
#endif /*MLME_BY_CMDTHREAD */
{
	ULONG TxTotalCnt;
#ifdef MLME_BY_CMDTHREAD
#else
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
#endif /*MLME_BY_CMDTHREAD */
#ifdef ANT_DIVERSITY_SUPPORT
	SHORT realavgrssi;
#endif /* ANT_DIVERSITY_SUPPORT */

	/* No More 0x84 MCU CMD from v.30 FW*/

//CFG MCC

#ifdef MICROWAVE_OVEN_SUPPORT
	/* update False CCA count to an array */
	NICUpdateRxStatusCnt1(pAd, pAd->Mlme.PeriodicRound%10);

	if (pAd->CommonCfg.MO_Cfg.bEnable)
	{
		UINT8 stage = pAd->Mlme.PeriodicRound%10;

		if (stage == MO_MEAS_PERIOD)
		{
			ASIC_MEASURE_FALSE_CCA(pAd);
			pAd->CommonCfg.MO_Cfg.nPeriod_Cnt = 0;
		}
		else if (stage == MO_IDLE_PERIOD)
		{
			UINT16 Idx;

			for (Idx = MO_MEAS_PERIOD + 1; Idx < MO_IDLE_PERIOD + 1; Idx++)
				pAd->CommonCfg.MO_Cfg.nFalseCCACnt += pAd->RalinkCounters.FalseCCACnt_100MS[Idx];

			//printk("%s: fales cca1 %d\n", __FUNCTION__, pAd->CommonCfg.MO_Cfg.nFalseCCACnt);
			if (pAd->CommonCfg.MO_Cfg.nFalseCCACnt > pAd->CommonCfg.MO_Cfg.nFalseCCATh)
				ASIC_MITIGATE_MICROWAVE(pAd);

		}
	}
#endif /* MICROWAVE_OVEN_SUPPORT */



	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_RADIO_MEASUREMENT |
								fRTMP_ADAPTER_NIC_NOT_EXIST)))
                                || IsHcAllSupportedBandsRadioOff(pAd))
	{
#ifdef MLME_BY_CMDTHREAD
		return STATUS_SUCCESS;
#else
		return;
#endif
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
#ifdef MLME_BY_CMDTHREAD
		return STATUS_SUCCESS;
#else
		return;
#endif

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_status_chk_and_get(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */



	pAd->bUpdateBcnCntDone = FALSE;

/*	RECBATimerTimeout(SystemSpecific1,FunctionContext,SystemSpecific2,SystemSpecific3);*/
	pAd->Mlme.PeriodicRound ++;
	pAd->Mlme.GPIORound++;

#ifdef MT7615
#ifdef CONFIG_AP_SUPPORT
	BcnCheck(pAd, DBDC_BAND0);
	if (pAd->CommonCfg.dbdc_mode)
		BcnCheck(pAd, DBDC_BAND1);
#endif /* CONFIG_AP_SUPPORT */
 	if ((pAd->Mlme.PeriodicRound % 5) == 0) //500ms update
    	{
		Update_Mib_Bucket_500Ms(pAd);
#ifdef SMART_CARRIER_SENSE_SUPPORT

		Smart_Carrier_Sense(pAd);

#endif /* SMART_CARRIER_SENSE_SUPPORT */	
 	}	
#endif /* MT7615 */

	if (pAd->chipOps.heart_beat_check)
		pAd->chipOps.heart_beat_check(pAd);

#ifdef CONFIG_BA_REORDER_MONITOR
	ba_timeout_monitor(pAd);
#endif

#ifndef WFA_VHT_PF
#endif /* WFA_VHT_PF */

#ifdef DYNAMIC_VGA_SUPPORT
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef MT_MAC
    /* Following is the TxOP scenario, monitor traffic in every minutes */
#ifdef RTMP_MAC_PCI
// TODO: Hugo, will refine it.
/*
    if ((pAd->Mlme.PeriodicRound % 10) == 0)
    {
	    TxOPUpdatingAlgo(pAd);
    }
 */
#endif /* RTMP_MAC_PCI */

	if ((pAd->Mlme.PeriodicRound % 1) == 0)
	{
#ifdef RTMP_MAC_PCI
		if (pAd->PDMAWatchDogEn)
		{
			PDMAWatchDog(pAd);
		}
#endif

		if (pAd->PSEWatchDogEn)
		{
			PSEWatchDog(pAd);
		}
	}
#endif /* MT_MAC */


	/* by default, execute every 500ms */
	if ((pAd->ra_interval) &&
		((pAd->Mlme.PeriodicRound % (pAd->ra_interval / 100)) == 0))
	{

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
            if (pAd->chipCap.fgRateAdaptFWOffload == TRUE )
            {
            }
            else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
            {
                //AsicRssiUpdate(pAd);
                //AsicTxCntUpdate(pAd, 0);
            }
		}
#endif /* MT_MAC */

		if (RTMPAutoRateSwitchCheck(pAd) == TRUE )
		{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
            if (pAd->chipCap.fgRateAdaptFWOffload == TRUE )
            {
            }
            else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
            {
#ifdef CONFIG_AP_SUPPORT
    			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    				APMlmeDynamicTxRateSwitching(pAd);
#endif /* CONFIG_AP_SUPPORT */
            }
		}else {
#ifdef MT_MAC
			if (pAd->chipCap.hif_type == HIF_MT) {
				MAC_TABLE_ENTRY *pEntry;
				MT_TX_COUNTER TxInfo;
				UINT16 i;
                //TODO:Carter, check why start from 1
				for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
				{
                    if (i >= MAX_LEN_OF_MAC_TABLE)
                        break;
                
					/* point to information of the individual station */
					pEntry = &pAd->MacTab.Content[i];

					if (IS_ENTRY_NONE(pEntry))
						continue;

					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;

#ifdef APCLI_SUPPORT
					if (IS_ENTRY_APCLI(pEntry) && (pEntry->Sst != SST_ASSOC))
						continue;
#ifdef MAC_REPEATER_SUPPORT
                    if (IS_ENTRY_REPEATER(pEntry) && (pEntry->Sst != SST_ASSOC))
                        continue;
#endif
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
					if (IS_ENTRY_WDS(pEntry) && !WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx))
						continue;
#endif /* WDS_SUPPORT */


					if (IS_VALID_ENTRY(pEntry))
						AsicTxCntUpdate(pAd, pEntry->wcid, &TxInfo);
				}
			}
#endif /* MT_MAC */
		}
	}

#ifdef DFS_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#endif /* DFS_SUPPORT */



	/* Normal 1 second Mlme PeriodicExec.*/
	if (pAd->Mlme.PeriodicRound %MLME_TASK_EXEC_MULTIPLE == 0)
	{
		pAd->Mlme.OneSecPeriodicRound ++;

#ifdef CONFIG_AP_SUPPORT
#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
		{
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				dynamic_tune_be_tx_op(pAd, 50);	/* change form 100 to 50 for WMM WiFi test @20070504*/
		}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
			if (IS_ASIC_CAP(pAd, fASIC_CAP_WMM_PKTDETECT_OFFLOAD))
			{
				MtCmdCr4QueryBssAcQPktNum(pAd,
				  CR4_GET_BSS_ACQ_PKT_NUM_CMD_DEFAULT);
			} else {
				mt_dynamic_wmm_be_tx_op(pAd,
				 ONE_SECOND_NON_BE_PACKETS_THRESHOLD);
			}
		}
#endif /* MT_MAC */

		NdisGetSystemUpTime(&pAd->Mlme.Now32);

		/* add the most up-to-date h/w raw counters into software variable, so that*/
		/* the dynamic tuning mechanism below are based on most up-to-date information*/
		/* Hint: throughput impact is very serious in the function */
		//NICUpdateRawCountersNew(pAd);
		RTMP_UPDATE_RAW_COUNTER(pAd);
		RTMP_SECOND_CCA_DETECTION(pAd);

#ifdef DYNAMIC_VGA_SUPPORT
		dynamic_vga_adjust(pAd);
#endif /* DYNAMIC_VGA_SUPPORT */


#ifdef DOT11_N_SUPPORT
   		/* Need statistics after read counter. So put after NICUpdateRawCountersNew*/
		ORIBATimerTimeout(pAd);
#endif /* DOT11_N_SUPPORT */


#ifdef DYNAMIC_VGA_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#endif /* DYNAMIC_VGA_SUPPORT */

	/*
		if (pAd->RalinkCounters.MgmtRingFullCount >= 2)
			RTMP_SET_FLAG(pAd, fRTMP_HW_ERR);
		else
			pAd->RalinkCounters.MgmtRingFullCount = 0;
	*/

		/* The time period for checking antenna is according to traffic*/
		{
			if (pAd->Mlme.bEnableAutoAntennaCheck)
			{
				TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
								 pAd->RalinkCounters.OneSecTxRetryOkCount +
								 pAd->RalinkCounters.OneSecTxFailCount;

				/* dynamic adjust antenna evaluation period according to the traffic*/
				if (TxTotalCnt > 50)
				{
					if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
						AsicEvaluateRxAnt(pAd);
				}
				else
				{
					if (pAd->Mlme.OneSecPeriodicRound % 3 == 0)
						AsicEvaluateRxAnt(pAd);
				}
			}
		}

#ifdef VIDEO_TURBINE_SUPPORT
	/*
		VideoTurbineUpdate(pAd);
		VideoTurbineDynamicTune(pAd);
	*/
#endif /* VIDEO_TURBINE_SUPPORT */

#ifdef MT76x0_TSSI_CAL_COMPENSATION
		if (IS_MT76x0(pAd) &&
			(pAd->chipCap.bInternalTxALC) &&
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET) == FALSE))
		{
			if ((pAd->Mlme.OneSecPeriodicRound % 1) == 0)
			{
				/* TSSI compensation */
				MT76x0_IntTxAlcProcess(pAd);
			}
		}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */


		if (RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET) == FALSE)
		{
			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 0)
			{
				{
				}
			}
		}

#ifdef DOT11_N_SUPPORT
#ifdef MT_MAC

        if (pAd->chipCap.hif_type == HIF_MT)
	{
		/* Not RDG, update the TxOP else keep the default RDG's TxOP */
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE) == FALSE)
		{
		}
	}


#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */

	/* update RSSI each 1 second*/
	RTMP_SET_UPDATE_RSSI(pAd);

#ifdef LINK_TEST_SUPPORT
	if (pAd->CommonCfg.LinkTestSupport)
	{
		/* Configure RCPI Calculation Method to refer to Response Frame and Data Frame */
		MtCmdLinkTestRcpiCtrl(pAd, TRUE);

		/*-------------------------------------------------------------------------------------------------------------------------*/ 
		/* State Transition:  Link up algorithm */
		/*-------------------------------------------------------------------------------------------------------------------------*/  

		if (!pAd->fgCmwLinkDone)
		{
			/* CSD config for state transition */
			LinkTestTxCsdCtrl(pAd, FALSE);
			
			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("(Link Test) NOT Link up!!! \n"));
		}
		else
		{
			/* Auto Link Test Control Handler executes with period 100 ms */
			RTMP_AUTO_LINK_TEST(pAd);

			MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("(Link Test) Link up!!! \n"));
		}
	}
#endif /* LINK_TEST_SUPPORT */

	/*Update some MIB counter per second */
	Update_Mib_Bucket_One_Sec(pAd);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef CONFIG_HOTSPOT_R2
#ifdef AP_QLOAD_SUPPORT	
			/* QBSS_LoadUpdate from Mib_Bucket */
			hotspot_update_ap_qload_to_bcn(pAd);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_HOTSPOT_R2 */

			APMlmePeriodicExec(pAd);
#ifdef BACKGROUND_SCAN_SUPPORT
			if (pAd->BgndScanCtrl.BgndScanSupport)
				ChannelQualityDetection(pAd);
#endif /* BACKGROUND_SCAN_SUPPORT */			

#ifdef SMART_CARRIER_SENSE_SUPPORT

				//Smart_Carrier_Sense(pAd);

#endif /* SMART_CARRIER_SENSE_SUPPORT */		            

            if (pAd->chipCap.hif_type == HIF_MT) {
                CCI_ACI_scenario_maintain(pAd);    
#if defined(MT_MAC) && defined(VHT_TXBF_SUPPORT)                
                Mumimo_scenario_maintain(pAd);
#endif /* MT_MAC && VHT_TXBF_SUPPORT */
            }

			if ((pAd->RalinkCounters.OneSecBeaconSentCnt == 0)
				&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				&& ((pAd->CommonCfg.bIEEE80211H != 1)
					|| (pAd->Dot11_H.RDMode != RD_SILENCE_MODE))
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode != WDS_BRIDGE_MODE)
#endif /* WDS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
				&& (isCarrierDetectExist(pAd) == FALSE)
#endif /* CARRIER_DETECTION_SUPPORT */
				)
				pAd->macwd ++;
			else
				pAd->macwd = 0;

			if (pAd->macwd > 1)
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("MAC specific condition \n"));

				AsicSetMacWD(pAd);

#ifdef AP_QLOAD_SUPPORT
				Show_QoSLoad_Proc(pAd, NULL);
#endif /* AP_QLOAD_SUPPORT */
			}
		}
#endif /* CONFIG_AP_SUPPORT */



		RTMP_SECOND_CCA_DETECTION(pAd);

		MlmeResetRalinkCounters(pAd);

#if defined(RTMP_MAC) || defined(RLT_MAC)
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

		RTMP_MLME_HANDLER(pAd);
	}


#ifdef CONFIG_AP_SUPPORT
	/* (pAd->ScanCtrl.PartialScan.NumOfChannels == DEFLAUT_PARTIAL_SCAN_CH_NUM) means that one partial scan is finished */
	if (pAd->ScanCtrl.PartialScan.bScanning == TRUE  &&
		pAd->ScanCtrl.PartialScan.NumOfChannels == DEFLAUT_PARTIAL_SCAN_CH_NUM)
	{
	    UCHAR ScanType = SCAN_ACTIVE;
		if ((pAd->ScanCtrl.PartialScan.BreakTime++ % DEFLAUT_PARTIAL_SCAN_BREAK_TIME) == 0) {
			if (!ApScanRunning(pAd)) {
                struct wifi_dev *pwdev = pAd->ScanCtrl.PartialScan.pwdev;
                if(pwdev)
                {
#ifdef WSC_AP_SUPPORT
#ifdef APCLI_SUPPORT
                    if((pwdev->wdev_type == WDEV_TYPE_APCLI) && 
                       (pwdev->func_idx < MAX_APCLI_NUM))
                    {
                        WSC_CTRL *pWpsCtrl = &pAd->ApCfg.ApCliTab[pwdev->func_idx].WscControl;
                        if ((pWpsCtrl->WscConfMode != WSC_DISABLE) && 
                            (pWpsCtrl->bWscTrigger == TRUE))
                            ScanType = SCAN_WSC_ACTIVE;
                    }
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */
                    ApSiteSurvey_by_wdev(pAd, NULL, ScanType, FALSE, pwdev);
                 }
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef WSC_INCLUDED
	WSC_HDR_BTN_MR_HANDLE(pAd);
#endif /* WSC_INCLUDED */



	pAd->bUpdateBcnCntDone = FALSE;

#ifdef MLME_BY_CMDTHREAD
	return STATUS_SUCCESS;
#endif
}


/*
	==========================================================================
	Validate SSID for connection try and rescan purpose
	Valid SSID will have visible chars only.
	The valid length is from 0 to 32.
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
BOOLEAN MlmeValidateSSID(UCHAR *pSsid, UCHAR SsidLen)
{
	int index;

	if (SsidLen > MAX_LEN_OF_SSID)
		return (FALSE);

	/* Check each character value*/
	for (index = 0; index < SsidLen; index++)
	{
		if (pSsid[index] < 0x20)
			return (FALSE);
	}

	/* All checked*/
	return (TRUE);
}



VOID RTMPSetEnterPsmNullBit(PPWR_MGMT_STRUCT pPwrMgmt)
{
	//pAd->StaCfg[0].PwrMgmt.bEnterPsmNull = TRUE;
	pPwrMgmt->bEnterPsmNull = TRUE;
}

VOID RTMPClearEnterPsmNullBit(PPWR_MGMT_STRUCT pPwrMgmt)
{
	//pAd->StaCfg[0].PwrMgmt.bEnterPsmNull = FALSE;
	pPwrMgmt->bEnterPsmNull = FALSE;
}

BOOLEAN RTMPEnterPsmNullBitStatus(PPWR_MGMT_STRUCT pPwrMgmt)
{
	//return (pAd->StaCfg[0].PwrMgmt.bEnterPsmNull);
	return (pPwrMgmt->bEnterPsmNull);
}

/*
	==========================================================================
	Description:
		This routine calculates TxPER, RxPER of the past N-sec period. And
		according to the calculation result, ChannelQuality is calculated here
		to decide if current AP is still doing the job.

		If ChannelQuality is not good, a ROAMing attempt may be tried later.
	Output:
		StaCfg[0].ChannelQuality - 0..100

	IRQL = DISPATCH_LEVEL

	NOTE: This routine decide channle quality based on RX CRC error ratio.
		Caller should make sure a function call to NICUpdateRawCountersNew(pAd)
		is performed right before this routine, so that this routine can decide
		channel quality based on the most up-to-date information
	==========================================================================
 */
VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now32)
{
	ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
	ULONG RxCnt, RxPER;
	UCHAR NorRssi;
	CHAR  MaxRssi;
	RSSI_SAMPLE *pRssiSample = NULL;
	UINT32 OneSecTxNoRetryOkCount = 0;
	UINT32 OneSecTxRetryOkCount = 0;
	UINT32 OneSecTxFailCount = 0;
	UINT32 OneSecRxOkCnt = 0;
	UINT32 OneSecRxFcsErrCnt = 0;
	ULONG ChannelQuality = 0;  /* 0..100, Channel Quality Indication for Roaming*/

#ifdef CONFIG_MULTI_CHANNEL
	if (pAd->Mlme.bStartMcc)
		BeaconLostTime += (8 * OS_HZ); // increase 8 seconds
	else
		BeaconLostTime = pStaCfg->BeaconLostTime;
#endif /* CONFIG_MULTI_CHANNEL */




		if (pMacEntry != NULL)
		{
			pRssiSample = &pMacEntry->RssiSample;
			OneSecTxNoRetryOkCount = pMacEntry->OneSecTxNoRetryOkCount;
			OneSecTxRetryOkCount = pMacEntry->OneSecTxRetryOkCount;
			OneSecTxFailCount = pMacEntry->OneSecTxFailCount;
			OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
			OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
		}
		else
		{
			pRssiSample = &pAd->MacTab.Content[0].RssiSample;
			OneSecTxNoRetryOkCount = pAd->RalinkCounters.OneSecTxNoRetryOkCount;
			OneSecTxRetryOkCount = pAd->RalinkCounters.OneSecTxRetryOkCount;
			OneSecTxFailCount = pAd->RalinkCounters.OneSecTxFailCount;
			OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
			OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
		}

	if (pRssiSample == NULL)
		return;
	MaxRssi = RTMPMaxRssi(pAd, pRssiSample->LastRssi[0],
								pRssiSample->LastRssi[1],
								pRssiSample->LastRssi[2]);


	/*
		calculate TX packet error ratio and TX retry ratio - if too few TX samples,
		skip TX related statistics
	*/
	TxOkCnt = OneSecTxNoRetryOkCount + OneSecTxRetryOkCount;
	TxCnt = TxOkCnt + OneSecTxFailCount;
	if (TxCnt < 5)
	{
		TxPER = 0;
		TxPRR = 0;
	}
	else
	{
		TxPER = (OneSecTxFailCount * 100) / TxCnt;
		TxPRR = ((TxCnt - OneSecTxNoRetryOkCount) * 100) / TxCnt;
	}


	/* calculate RX PER - don't take RxPER into consideration if too few sample*/
	RxCnt = OneSecRxOkCnt + OneSecRxFcsErrCnt;
	if (RxCnt < 5)
		RxPER = 0;
	else
		RxPER = (OneSecRxFcsErrCnt * 100) / RxCnt;

#if defined(CONFIG_STA_SUPPORT) && (defined(STA_LP_PHASE_1_SUPPORT) || defined(STA_LP_PHASE_2_SUPPORT))
	if (INFRA_ON(pStaCfg) && pMacEntry &&
		(pMacEntry->wcid == pMacEntry->wcid) &&
		(pStaCfg->PwrMgmt.bBeaconLost) &&
		pStaCfg->PwrMgmt.bDoze)
	{
		pStaCfg->ChannelQuality = 0;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s::MT7636 BEACON lost meet\n", __FUNCTION__));
	}
	else
#endif /* CONFIG_STA_SUPPORT && (STA_LP_PHASE_1_SUPPORT || STA_LP_PHASE_2_SUPPORT) */
	{
	/* decide ChannelQuality based on: 1)last BEACON received time, 2)last RSSI, 3)TxPER, and 4)RxPER*/
		{
			/* Normalize Rssi*/
			if (MaxRssi > -40)
				NorRssi = 100;
			else if (MaxRssi < -90)
				NorRssi = 0;
			else
				NorRssi = (MaxRssi + 90) * 2;

			/* ChannelQuality = W1*RSSI + W2*TxPRR + W3*RxPER	 (RSSI 0..100), (TxPER 100..0), (RxPER 100..0)*/
			ChannelQuality = (RSSI_WEIGHTING * NorRssi +
									   TX_WEIGHTING * (100 - TxPRR) +
									   RX_WEIGHTING* (100 - RxPER)) / 100;
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(line:%d), ChannelQuality(%lu)\n", __FUNCTION__, __LINE__, ChannelQuality));
		}
	}


#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		if (pMacEntry != NULL)
			pMacEntry->ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
	}
#endif /* CONFIG_AP_SUPPORT */


}

VOID MlmeSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/

	if (TxPreamble == Rt802_11PreambleLong)
	{
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	}
	else
	{
		/* NOTE: 1Mbps should always use long preamble*/
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	}
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeSetTxPreamble = %s PREAMBLE\n",
				((TxPreamble == Rt802_11PreambleLong) ? "LONG" : "SHORT")));

	AsicSetTxPreamble(pAd, TxPreamble);
}


/*
    ==========================================================================
    Description:
        Update basic rate bitmap
    ==========================================================================
*/
VOID UpdateBasicRateBitmap(RTMP_ADAPTER *pAdapter, struct wifi_dev *wdev)
{
    INT  i, j;
                  /* 1  2  5.5, 11,  6,  9, 12, 18, 24, 36, 48,  54 */
    UCHAR rate[] = { 2, 4,  11, 22, 12, 18, 24, 36, 48, 72, 96, 108 };
    UCHAR *sup_p = wdev->rate.SupRate;
    UCHAR *ext_p = wdev->rate.ExtRate;
    ULONG bitmap = pAdapter->CommonCfg.BasicRateBitmap;

    /* if A mode, always use fix BasicRateBitMap */
    /*if (wdev->channel)*/
	if (wdev->channel > 14)
	{
		if (pAdapter->CommonCfg.BasicRateBitmap & 0xF)
		{
			/* no 11b rate in 5G band */
			pAdapter->CommonCfg.BasicRateBitmapOld = \
										pAdapter->CommonCfg.BasicRateBitmap;
			pAdapter->CommonCfg.BasicRateBitmap &= (~0xF); /* no 11b */
		}

		/* force to 6,12,24M in a-band */
		pAdapter->CommonCfg.BasicRateBitmap |= 0x150; /* 6, 12, 24M */
    }
	else
	{
		/* no need to modify in 2.4G (bg mixed) */
		pAdapter->CommonCfg.BasicRateBitmap = \
										pAdapter->CommonCfg.BasicRateBitmapOld;
	}

    if (pAdapter->CommonCfg.BasicRateBitmap > 4095)
    {
        /* (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1 */
        return;
    }

	bitmap = pAdapter->CommonCfg.BasicRateBitmap;  /* renew bitmap value */

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        sup_p[i] &= 0x7f;
        ext_p[i] &= 0x7f;
    }

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        if (bitmap & (1 << i))
        {
            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (sup_p[j] == rate[i])
                    sup_p[j] |= 0x80;
            }

            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (ext_p[j] == rate[i])
                    ext_p[j] |= 0x80;
            }
        }
    }
}

#define MCAST_WCID_TO_REMOVE 0 //Pat: TODO

/*
	bLinkUp is to identify the inital link speed.
	TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
*/
VOID MlmeUpdateTxRates(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp, UCHAR apidx)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	HTTRANSMIT_SETTING *pHtPhy = NULL, *pMaxHtPhy = NULL, *pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	UCHAR HtMcs = MCS_AUTO;
	struct wifi_dev *wdev = NULL;

	wdev = get_wdev_by_idx(pAd,apidx);

	if(!wdev)
	{
		return;
	}


	/* find max desired rate*/
	UpdateBasicRateBitmap(pAd,wdev);


	num = 0;
	auto_rate_cur_p = NULL;
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		switch (wdev->rate.DesireRate[i] & 0x7f)
		{
			case 2:  Rate = RATE_1;   num++;   break;
			case 4:  Rate = RATE_2;   num++;   break;
			case 11: Rate = RATE_5_5; num++;   break;
			case 22: Rate = RATE_11;  num++;   break;
			case 12: Rate = RATE_6;   num++;   break;
			case 18: Rate = RATE_9;   num++;   break;
			case 24: Rate = RATE_12;  num++;   break;
			case 36: Rate = RATE_18;  num++;   break;
			case 48: Rate = RATE_24;  num++;   break;
			case 72: Rate = RATE_36;  num++;   break;
			case 96: Rate = RATE_48;  num++;   break;
			case 108: Rate = RATE_54; num++;   break;
			/*default: Rate = RATE_1;   break;*/
		}
		if (MaxDesire < Rate)  MaxDesire = Rate;
	}

/*===========================================================================*/
	do
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
		//CFG TODO
		if (apidx >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO)
		{
			printk("%s: Update for GO\n", __FUNCTION__);
			wdev = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX].wdev;
			break;
		}
#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef APCLI_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
		{
			UCHAR idx = apidx - MIN_NET_DEVICE_FOR_APCLI;

			if (idx < MAX_APCLI_NUM)
			{
				wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
				break;
			}
			else
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid idx(%d)\n", __FUNCTION__, idx));
				return;
			}
		}
#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{
				UCHAR idx = apidx - MIN_NET_DEVICE_FOR_WDS;

				if (idx < MAX_WDS_ENTRY)
				{
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					break;
				}
				else
				{
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __FUNCTION__, apidx));
					return;
				}
			}
#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) &&
				(apidx < MAX_MBSSID_NUM(pAd)) &&
				(apidx < HW_BEACON_MAX_NUM))
			{
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			}
			else
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __FUNCTION__, apidx));
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */

	} while(FALSE);

	if (wdev)
	{
		pHtPhy = &wdev->HTPhyMode;
		pMaxHtPhy = &wdev->MaxHTPhyMode;
		pMinHtPhy = &wdev->MinHTPhyMode;

		auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
		HtMcs = wdev->DesiredTransmitSetting.field.MCS;
	}

	wdev->rate.MaxDesiredRate = MaxDesire;

	if (pMinHtPhy == NULL)
		return;
	pMinHtPhy->word = 0;
	pMaxHtPhy->word = 0;
	pHtPhy->word = 0;

	/*
		Auto rate switching is enabled only if more than one DESIRED RATES are
		specified; otherwise disabled
	*/
	if (num <= 1)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	if (HtMcs != MCS_AUTO)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	{
		pSupRate = &wdev->rate.SupRate[0];
		pExtRate = &wdev->rate.ExtRate[0];
		SupRateLen = wdev->rate.SupRateLen;
		ExtRateLen = wdev->rate.ExtRateLen;
	}

	/* find max supported rate */
	for (i=0; i<SupRateLen; i++)
	{
		switch (pSupRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 0;	 break;
			case 4:   Rate = RATE_2;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 1;	 break;
			case 11:  Rate = RATE_5_5;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 2;	 break;
			case 22:  Rate = RATE_11;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 3;	 break;
			case 12:  Rate = RATE_6;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 4;  break;
			case 18:  Rate = RATE_9;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 5;	 break;
			case 24:  Rate = RATE_12;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 6;  break;
			case 36:  Rate = RATE_18;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 7;	 break;
			case 48:  Rate = RATE_24;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 8;  break;
			case 72:  Rate = RATE_36;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 9;	 break;
			case 96:  Rate = RATE_48;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 10;	 break;
			case 108: Rate = RATE_54;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 11;	 break;
			default:  Rate = RATE_1;	break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	for (i=0; i<ExtRateLen; i++)
	{
		switch (pExtRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 0;	 break;
			case 4:   Rate = RATE_2;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 1;	 break;
			case 11:  Rate = RATE_5_5;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 2;	 break;
			case 22:  Rate = RATE_11;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 3;	 break;
			case 12:  Rate = RATE_6;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 4;  break;
			case 18:  Rate = RATE_9;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 5;	 break;
			case 24:  Rate = RATE_12;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 6;  break;
			case 36:  Rate = RATE_18;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 7;	 break;
			case 48:  Rate = RATE_24;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 8;  break;
			case 72:  Rate = RATE_36;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 9;	 break;
			case 96:  Rate = RATE_48;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 10;	 break;
			case 108: Rate = RATE_54;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 11;	 break;
			default:  Rate = RATE_1;
				break;
		}
		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	// TODO: shiang-7603, need to revise for MT7603!!
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		RTMP_IO_WRITE32(pAd, LEGACY_BASIC_RATE, BasicRateBitmap);
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;
		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s():[MaxSupport = %d] = MaxDesire %d Mbps\n",
				__FUNCTION__, RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));
	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		wdev->rate.MaxTxRate = MaxSupport;
	else
		wdev->rate.MaxTxRate = MaxDesire;

	wdev->rate.MinTxRate = MinSupport;
	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p)
	{
		short dbm = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			dbm =0;
#endif /* CONFIG_AP_SUPPORT */
		if (bLinkUp == TRUE)
			wdev->rate.TxRate = RATE_24;
		else
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		if (dbm < -75)
			wdev->rate.TxRate = RATE_11;
		else if (dbm < -70)
			wdev->rate.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (wdev->rate.TxRate > wdev->rate.MaxTxRate)
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		wdev->rate.TxRateIndex = 0;

	}
	else
	{
		wdev->rate.TxRate =wdev->rate.MaxTxRate;

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6)
		{
			if (HtMcs <= MCS_7)
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		}
		else
		{
			if (HtMcs <= MCS_3)
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}


	}

	if (wdev->rate.TxRate <= RATE_11)
	{
		pMaxHtPhy->field.MODE = MODE_CCK;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */

	}
	else
	{
		pMaxHtPhy->field.MODE = MODE_OFDM;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */

	}

	pHtPhy->word = (pMaxHtPhy->word);
	if (bLinkUp && (pAd->OpMode == OPMODE_STA))
	{
	}
	else
	{
		if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
			wdev->channel <= 14)
		{
			pAd->CommonCfg.MlmeRate = RATE_1;
			wdev->rate.MlmeTransmit.field.MODE = MODE_CCK;
			wdev->rate.MlmeTransmit.field.MCS = RATE_1;
			pAd->CommonCfg.RtsRate = RATE_11;
		}
		else
		{
			pAd->CommonCfg.MlmeRate = RATE_6;
			pAd->CommonCfg.RtsRate = RATE_6;
			wdev->rate.MlmeTransmit.field.MODE = MODE_OFDM;
			wdev->rate.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}

		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.word = wdev->rate.MlmeTransmit.word;
		if (wdev->rate.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_24];
		else
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = RATE_1;
		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;

#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */
			HTTRANSMIT_SETTING tPhyMode, *pTransmit;
			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
			pTransmit  = (wdev->channel > 14) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);
			if (memcmp(pTransmit, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0)
			{
				memmove(pTransmit, &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode,
							sizeof(HTTRANSMIT_SETTING));
			}
			/*
			printk("%s: %s, McastPhyMode.MODE=%d, MCS=%d, BW=%d\n", __FUNCTION__, 
				(wdev->channel > 14) ? "5G": "2.4G", pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
				pAd->CommonCfg.MCastPhyMode_5G.field.MCS, pAd->CommonCfg.MCastPhyMode_5G.field.BW);
			*/
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n",
				__FUNCTION__, RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport],
				RateIdToMbps[wdev->rate.MaxTxRate],
				RateIdToMbps[wdev->rate.MinTxRate],
			 /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n",
				__FUNCTION__, RateIdToMbps[wdev->rate.TxRate],
				RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));

}


/*
	bLinkUp is to identify the inital link speed.
	TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
*/
VOID MlmeUpdateTxRatesWdev(RTMP_ADAPTER *pAd, BOOLEAN bLinkUp,struct wifi_dev *wdev)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	HTTRANSMIT_SETTING *pHtPhy = NULL, *pMaxHtPhy = NULL, *pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	UCHAR HtMcs = MCS_AUTO;

	if(!wdev)
		return ;

	/* find max desired rate*/
	UpdateBasicRateBitmap(pAd,wdev);


	num = 0;
	auto_rate_cur_p = NULL;
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		switch (wdev->rate.DesireRate[i] & 0x7f)
		{
			case 2:  Rate = RATE_1;   num++;   break;
			case 4:  Rate = RATE_2;   num++;   break;
			case 11: Rate = RATE_5_5; num++;   break;
			case 22: Rate = RATE_11;  num++;   break;
			case 12: Rate = RATE_6;   num++;   break;
			case 18: Rate = RATE_9;   num++;   break;
			case 24: Rate = RATE_12;  num++;   break;
			case 36: Rate = RATE_18;  num++;   break;
			case 48: Rate = RATE_24;  num++;   break;
			case 72: Rate = RATE_36;  num++;   break;
			case 96: Rate = RATE_48;  num++;   break;
			case 108: Rate = RATE_54; num++;   break;
			/*default: Rate = RATE_1;   break;*/
		}
		if (MaxDesire < Rate)  MaxDesire = Rate;
	}
	

		pHtPhy = &wdev->HTPhyMode;
		pMaxHtPhy = &wdev->MaxHTPhyMode;
		pMinHtPhy = &wdev->MinHTPhyMode;

		auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
		HtMcs = wdev->DesiredTransmitSetting.field.MCS;

	wdev->rate.MaxDesiredRate = MaxDesire;

	if (pMinHtPhy == NULL)
		return;
	
		pMinHtPhy->word = 0;
		pMaxHtPhy->word = 0;
		pHtPhy->word = 0;

	/*
		Auto rate switching is enabled only if more than one DESIRED RATES are
		specified; otherwise disabled
	*/
	if (num <= 1)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	if (HtMcs != MCS_AUTO)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	{
		pSupRate = &wdev->rate.SupRate[0];
		pExtRate = &wdev->rate.ExtRate[0];
		SupRateLen = wdev->rate.SupRateLen;
		ExtRateLen = wdev->rate.ExtRateLen;
	}

	/* find max supported rate */
	for (i=0; i<SupRateLen; i++)
	{
		switch (pSupRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 0;	 break;
			case 4:   Rate = RATE_2;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 1;	 break;
			case 11:  Rate = RATE_5_5;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 2;	 break;
			case 22:  Rate = RATE_11;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 3;	 break;
			case 12:  Rate = RATE_6;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 4;  break;
			case 18:  Rate = RATE_9;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 5;	 break;
			case 24:  Rate = RATE_12;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 6;  break;
			case 36:  Rate = RATE_18;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 7;	 break;
			case 48:  Rate = RATE_24;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 8;  break;
			case 72:  Rate = RATE_36;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 9;	 break;
			case 96:  Rate = RATE_48;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 10;	 break;
			case 108: Rate = RATE_54;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 1 << 11;	 break;
			default:  Rate = RATE_1;	break;
		}

		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}

	for (i=0; i<ExtRateLen; i++)
	{
		switch (pExtRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 0;	 break;
			case 4:   Rate = RATE_2;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 1;	 break;
			case 11:  Rate = RATE_5_5;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 2;	 break;
			case 22:  Rate = RATE_11;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 3;	 break;
			case 12:  Rate = RATE_6;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 4;  break;
			case 18:  Rate = RATE_9;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 5;	 break;
			case 24:  Rate = RATE_12;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 6;  break;
			case 36:  Rate = RATE_18;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 7;	 break;
			case 48:  Rate = RATE_24;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 1 << 8;  break;
			case 72:  Rate = RATE_36;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 9;	 break;
			case 96:  Rate = RATE_48;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 10;	 break;
			case 108: Rate = RATE_54;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 1 << 11;	 break;
			default:  Rate = RATE_1;
				break;
		}
		if (MaxSupport < Rate)
			MaxSupport = Rate;

		if (MinSupport > Rate)
			MinSupport = Rate;
	}
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;
		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s():[MaxSupport = %d] = MaxDesire %d Mbps\n",
				__FUNCTION__, RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));
	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		wdev->rate.MaxTxRate = MaxSupport;
	else
		wdev->rate.MaxTxRate = MaxDesire;

	wdev->rate.MinTxRate = MinSupport;
	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p)
	{
		short dbm = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			dbm =0;
#endif /* CONFIG_AP_SUPPORT */
		if (bLinkUp == TRUE)
			wdev->rate.TxRate = RATE_24;
		else
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		if (dbm < -75)
			wdev->rate.TxRate = RATE_11;
		else if (dbm < -70)
			wdev->rate.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (wdev->rate.TxRate > wdev->rate.MaxTxRate)
			wdev->rate.TxRate = wdev->rate.MaxTxRate;

		wdev->rate.TxRateIndex = 0;

	}
	else
	{
		wdev->rate.TxRate =wdev->rate.MaxTxRate;

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6)
		{
			if (HtMcs <= MCS_7)
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		}
		else
		{
			if (HtMcs <= MCS_3)
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}


	}

	if (wdev->rate.TxRate <= RATE_11)
	{
		pMaxHtPhy->field.MODE = MODE_CCK;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */

	}
	else
	{
		pMaxHtPhy->field.MODE = MODE_OFDM;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */

	}

	pHtPhy->word = (pMaxHtPhy->word);
	if (bLinkUp && (pAd->OpMode == OPMODE_STA))
	{
	}
	else
	{
		if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
			wdev->channel <= 14)
		{
			pAd->CommonCfg.MlmeRate = RATE_1;
			wdev->rate.MlmeTransmit.field.MODE = MODE_CCK;
			wdev->rate.MlmeTransmit.field.MCS = RATE_1;
			pAd->CommonCfg.RtsRate = RATE_11;
		}
		else
		{
			pAd->CommonCfg.MlmeRate = RATE_6;
			pAd->CommonCfg.RtsRate = RATE_6;
			wdev->rate.MlmeTransmit.field.MODE = MODE_OFDM;
			wdev->rate.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}

		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.word = wdev->rate.MlmeTransmit.word;
		if (wdev->rate.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_24];
		else
			pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode.field.MCS = RATE_1;
		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;

#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */
				HTTRANSMIT_SETTING tPhyMode, *pTransmit;
			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
				pTransmit  = (wdev->channel > 14) ? (&pAd->CommonCfg.MCastPhyMode_5G) : (&pAd->CommonCfg.MCastPhyMode);
				if (memcmp(pTransmit, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0)
			{
					memmove(pTransmit, &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode,
							sizeof(HTTRANSMIT_SETTING));
			}
				/*
				printk("%s: %s, McastPhyMode.MODE=%d, MCS=%d, BW=%d\n", __FUNCTION__, 
						(wdev->channel > 14) ? "5G": "2.4G", pAd->CommonCfg.MCastPhyMode_5G.field.MODE,
						pAd->CommonCfg.MCastPhyMode_5G.field.MCS, pAd->CommonCfg.MCastPhyMode_5G.field.BW);
				*/
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n",
				__FUNCTION__, RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport],
				RateIdToMbps[wdev->rate.MaxTxRate],
				RateIdToMbps[wdev->rate.MinTxRate],
			 /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s(): (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n",
				__FUNCTION__, RateIdToMbps[wdev->rate.TxRate],
				RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));

}


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		This function update HT Rate setting.
		Input Wcid value is valid for 2 case :
		1. it's used for Station in infra mode that copy AP rate to Mactable.
		2. OR Station 	in adhoc mode to copy peer's HT rate to Mactable.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeUpdateHtTxRates(RTMP_ADAPTER *pAd, UCHAR apidx)
{
	UCHAR StbcMcs;
	RT_HT_CAPABILITY *pRtHtCap = NULL;
	RT_PHY_INFO *pActiveHtPhy = NULL;
	ULONG BasicMCS;
	RT_PHY_INFO *pDesireHtPhy = NULL;
	PHTTRANSMIT_SETTING pHtPhy = NULL;
	PHTTRANSMIT_SETTING pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING pMinHtPhy = NULL;
	BOOLEAN *auto_rate_cur_p;
	struct wifi_dev *wdev = NULL;
	ADD_HT_INFO_IE *addht;
	UCHAR cfg_ht_bw;


	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s()===> \n", __FUNCTION__));

	auto_rate_cur_p = NULL;

	wdev = get_wdev_by_idx(pAd,apidx);

	if (!wdev)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n", __FUNCTION__, apidx));
		return;
	}


	pDesireHtPhy = &wdev->DesiredHtPhyInfo;
	pActiveHtPhy = &wdev->DesiredHtPhyInfo;
	pHtPhy = &wdev->HTPhyMode;
	pMaxHtPhy = &wdev->MaxHTPhyMode;
	pMinHtPhy = &wdev->MinHTPhyMode;
	auto_rate_cur_p = &wdev->bAutoTxRateSwitch;
	addht = wlan_operate_get_addht(wdev);
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	{
		if ((!pDesireHtPhy) || pDesireHtPhy->bHtEnable == FALSE)
			return;

		pRtHtCap = &pAd->CommonCfg.DesiredHtPhy;
		StbcMcs = (UCHAR) addht->AddHtInfo3.StbcMcs;
		BasicMCS = addht->MCSSet[0]+(addht->MCSSet[1]<<8)+(StbcMcs<<16);
		if ((pAd->CommonCfg.DesiredHtPhy.TxSTBC) && (pAd->Antenna.field.TxPath >= 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}

	/* Decide MAX ht rate.*/
	if ((pRtHtCap->GF) && (pAd->CommonCfg.DesiredHtPhy.GF))
		pMaxHtPhy->field.MODE = MODE_HTGREENFIELD;
	else
		pMaxHtPhy->field.MODE = MODE_HTMIX;

    if (cfg_ht_bw)
		pMaxHtPhy->field.BW = BW_40;
	else
		pMaxHtPhy->field.BW = BW_20;

    if (pMaxHtPhy->field.BW == BW_20)
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 & pRtHtCap->ShortGIfor20);
	else
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 & pRtHtCap->ShortGIfor40);

	if (pDesireHtPhy->MCSSet[4] != 0)
	{
		pMaxHtPhy->field.MCS = 32;
	}

	pMaxHtPhy->field.MCS = get_ht_max_mcs(pAd, &pDesireHtPhy->MCSSet[0],
											&pActiveHtPhy->MCSSet[0]);

	/* Copy MIN ht rate.  rt2860???*/
	pMinHtPhy->field.BW = BW_20;
	pMinHtPhy->field.MCS = 0;
	pMinHtPhy->field.STBC = 0;
	pMinHtPhy->field.ShortGI = 0;
	/*If STA assigns fixed rate. update to fixed here.*/


	/* Decide ht rate*/
	pHtPhy->field.STBC = pMaxHtPhy->field.STBC;
	pHtPhy->field.BW = pMaxHtPhy->field.BW;
	pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
	pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
	pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;

	/* use default now. rt2860*/
	if (pDesireHtPhy->MCSSet[0] != 0xff)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(wdev->PhyMode)) {
		pDesireHtPhy->bVhtEnable = TRUE;
		rtmp_set_vht(pAd, pDesireHtPhy);
		if (pDesireHtPhy->bVhtEnable == TRUE) {
			PHTTRANSMIT_SETTING pHtPhy = NULL;
			PHTTRANSMIT_SETTING pMaxHtPhy = NULL;
			PHTTRANSMIT_SETTING pMinHtPhy = NULL;
			pHtPhy = &wdev->HTPhyMode;
			pMaxHtPhy = &wdev->MaxHTPhyMode;
			pMinHtPhy = &wdev->MinHTPhyMode;

			pMaxHtPhy->field.MODE = MODE_VHT;
			if (pDesireHtPhy->vht_bw == VHT_BW_2040) {//use HT BW setting
				if (pHtPhy->field.BW == BW_20)
					pMaxHtPhy->field.MCS = 8;
				else
					pMaxHtPhy->field.MCS = 9;
			} else if(pDesireHtPhy->vht_bw == VHT_BW_80) {
				pMaxHtPhy->field.BW = BW_80;
				pMaxHtPhy->field.MCS = 9;
			} else if(pDesireHtPhy->vht_bw == VHT_BW_160) {
				pMaxHtPhy->field.BW = BW_160;
				pMaxHtPhy->field.MCS = 9;
			} else if(pDesireHtPhy->vht_bw == VHT_BW_8080) {
				pMaxHtPhy->field.BW = BW_160;
				pMaxHtPhy->field.MCS = 9;
			}
			pMaxHtPhy->field.ShortGI = pAd->CommonCfg.vht_sgi;
			/* Decide ht rate*/
			pHtPhy->field.BW = pMaxHtPhy->field.BW;
			pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
			pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
			pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;
		}
	}
#endif /* DOT11_VHT_AC */
	
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s():<---.AMsduSize = %d  \n", __FUNCTION__, pAd->CommonCfg.DesiredHtPhy.AmsduSize ));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TX: MCS[0] = %x (choose %d), BW = %d, ShortGI = %d, MODE = %d,  \n", pActiveHtPhy->MCSSet[0],pHtPhy->field.MCS,
		pHtPhy->field.BW, pHtPhy->field.ShortGI, pHtPhy->field.MODE));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s():<=== \n", __FUNCTION__));
}




VOID BATableInit(RTMP_ADAPTER *pAd, BA_TABLE *Tab)
{
	int i;

	Tab->numAsOriginator = 0;
	Tab->numAsRecipient = 0;
	Tab->numDoneOriginator = 0;
	NdisAllocateSpinLock(pAd, &pAd->BATabLock);
#ifdef CONFIG_BA_REORDER_MONITOR
	pAd->BATable.ba_timeout_check = FALSE;
	os_zero_mem((UCHAR *)&pAd->BATable.ba_timeout_bitmap[0], sizeof(UINT32) * 16);
#endif

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++)
	{
		Tab->BARecEntry[i].REC_BA_Status = Recipient_NONE;
		NdisAllocateSpinLock(pAd, &(Tab->BARecEntry[i].RxReRingLock));
	}
	for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++)
	{
		Tab->BAOriEntry[i].ORI_BA_Status = Originator_NONE;
	}
}


VOID BATableExit(RTMP_ADAPTER *pAd)
{
	int i;

	for(i=0; i<MAX_LEN_OF_BA_REC_TABLE; i++)
	{
		NdisFreeSpinLock(&pAd->BATable.BARecEntry[i].RxReRingLock);
	}
	NdisFreeSpinLock(&pAd->BATabLock);
}
#endif /* DOT11_N_SUPPORT */

VOID MlmeRadioOff(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
    RTMP_OS_NETDEV_STOP_QUEUE(wdev->if_dev);

    wdev->fgRadioOnRequest = FALSE;    
    
    MTRadioOff(pAd, wdev);
}

VOID MlmeRadioOn(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
    wdev->fgRadioOnRequest = TRUE;
    
    MTRadioOn(pAd, wdev);
    
    RTMP_OS_NETDEV_START_QUEUE(wdev->if_dev);
}

VOID MlmeLpEnter(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
		MTMlmeLpEnter(pAd, wdev);
#endif /* RTMP_MAC_PCI */
}


VOID MlmeLpExit(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
		MTMlmeLpExit(pAd, wdev);
#endif /* RTMP_MAC_PCI */
}

/*! \brief initialize BSS table
 *	\param p_tab pointer to the table
 *	\return none
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 */
VOID BssTableInit(BSS_TABLE *Tab)
{
	int i;

	Tab->BssNr = 0;
	Tab->BssOverlapNr = 0;

	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
	{
		UCHAR *pOldAddr = Tab->BssEntry[i].pVarIeFromProbRsp;

		NdisZeroMemory(&Tab->BssEntry[i], sizeof(BSS_ENTRY));

		Tab->BssEntry[i].Rssi = -127;	/* initial the rssi as a minimum value */
		if (pOldAddr)
		{
			RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
			Tab->BssEntry[i].pVarIeFromProbRsp = pOldAddr;
		}
	}
}


#if defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT)
/*
	backup the other band's scan result, and init the Band from input.

	Band 0 : init 2.4G
	Band 1 : init 5G
*/
VOID BssTableInitByBand(BSS_TABLE *Tab, UCHAR Band)
{
	int i;
	INT bss_2G_cnt = 0;
	INT bss_5G_cnt = 0;
	INT bss_backup_cnt = 0;
	UCHAR *pOldAddr = NULL;
	BSS_TABLE *tab_buf = NULL;

	// check the total 2.4G/5G BSS
	if (Tab->BssNr >0)
	{
		for (i = 0; i < Tab->BssNr; i++)
		{
			if (Tab->BssEntry[i].Channel > 14)
				bss_5G_cnt++;
			else
				bss_2G_cnt++;
		}
	}


	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ==> Band=%u, bss_2G_cnt=%d, bss_5G_cnt=%d\n",__FUNCTION__,Band, bss_2G_cnt, bss_5G_cnt));

	if (((Band==0) && bss_5G_cnt) || //init 2.4G, backup 5G
		((Band==1) && bss_2G_cnt)) //init 5G, backup 2.4G
	{
		os_alloc_mem(NULL, (UCHAR **)&tab_buf, sizeof(BSS_TABLE));

		if (tab_buf == NULL)
		{
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): alloc tab_buf fail!!\n",__FUNCTION__));
			return;
		}


		//backup the dedicated band
		for (i = 0; i < Tab->BssNr; i++)
		{
			if (((Tab->BssEntry[i].Channel > 14) && (Band==0))  //init 2.4G, backup 5G
				|| ((Tab->BssEntry[i].Channel > 0) && (Tab->BssEntry[i].Channel <=14) && (Band==1))) //init 5G, backup 2.4G
			{
				pOldAddr = Tab->BssEntry[i].pVarIeFromProbRsp;
				os_move_mem(&tab_buf->BssEntry[bss_backup_cnt],&Tab->BssEntry[i],sizeof(BSS_ENTRY));
				
				if (Tab->BssEntry[i].VarIeFromProbeRspLen && pOldAddr)
					os_move_mem(tab_buf->BssEntry[bss_backup_cnt].pVarIeFromProbRsp,pOldAddr,Tab->BssEntry[i].VarIeFromProbeRspLen);

				bss_backup_cnt++;
			}
		}

		tab_buf->BssNr = bss_backup_cnt;
		tab_buf->BssOverlapNr = 0;


		//MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ==> Band=%u, bss_2G_cnt=%d, bss_5G_cnt=%d, bss_backup_cnt=%d\n",__FUNCTION__,Band, bss_2G_cnt, bss_5G_cnt, bss_backup_cnt));


		//reset the global table
		BssTableInit(Tab);

		//restore the backup band
		for (i = 0; i < bss_backup_cnt; i++)
		{
			pOldAddr = tab_buf->BssEntry[i].pVarIeFromProbRsp;		
			os_move_mem(&Tab->BssEntry[i], &tab_buf->BssEntry[i], sizeof(BSS_ENTRY));

			if (tab_buf->BssEntry[i].VarIeFromProbeRspLen && pOldAddr)
				os_move_mem(Tab->BssEntry[i].pVarIeFromProbRsp,pOldAddr,Tab->BssEntry[i].VarIeFromProbeRspLen);
			
		}

		Tab->BssNr = tab_buf->BssNr;
		Tab->BssOverlapNr = tab_buf->BssOverlapNr;

		//MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): <== Band=%u,  Tab->BssNr=%d\n",__FUNCTION__,Band, Tab->BssNr));
		
		if (tab_buf != NULL)
			os_free_mem(tab_buf);			
	}
	else
	{	/*
			no need to backup, reset the global table
		*/
		BssTableInit(Tab);
	}
	
}
#endif /* defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT) */


/*! \brief search the BSS table by SSID
 *	\param p_tab pointer to the bss table
 *	\param ssid SSID string
 *	\return index of the table, BSS_NOT_FOUND if not in the table
 *	\pre
 *	\post
 *	\note search by sequential search

 IRQL = DISPATCH_LEVEL

 */
ULONG BssTableSearch(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++)
	{

		/*
			Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
			We should distinguish this case.
		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid))
		{
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pBssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++)
	{

		/* Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.*/
		/* We should distinguish this case.*/
		/*		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid) &&
			SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
		{
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 Bssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++)
	{
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen) ||
			(NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) ||
			(NdisEqualMemory(Tab->BssEntry[i].Ssid, ZeroSsid, Tab->BssEntry[i].SsidLen))))
		{
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearchBySSID(BSS_TABLE *Tab, UCHAR *pSsid, UCHAR SsidLen)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++)
	{
		if (SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
		{
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


VOID BssTableDeleteEntry(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UCHAR i, j;

#ifdef WH_EZ_SETUP
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BssTableDeleteEntry called for : %02x-%02x-%02x-%02x-%02x-%02x, Channel=%d\n",
		pBssid[0],pBssid[1],pBssid[2],pBssid[3],pBssid[4],pBssid[5],Channel));
#endif

	for (i = 0; i < Tab->BssNr && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++)
	{
		if ((Tab->BssEntry[i].Channel == Channel) &&
			(MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)))
		{
			UCHAR *pOldAddr = NULL;

			for (j = i; j < Tab->BssNr - 1; j++)
			{
				pOldAddr = Tab->BssEntry[j].pVarIeFromProbRsp;
				NdisMoveMemory(&(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]), sizeof(BSS_ENTRY));
				if (pOldAddr)
				{
					RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
					NdisMoveMemory(pOldAddr,
								   Tab->BssEntry[j + 1].pVarIeFromProbRsp,
								   Tab->BssEntry[j + 1].VarIeFromProbeRspLen);
					Tab->BssEntry[j].pVarIeFromProbRsp = pOldAddr;
				}
			}

			pOldAddr = Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp;
			NdisZeroMemory(&(Tab->BssEntry[Tab->BssNr - 1]), sizeof(BSS_ENTRY));
			if (pOldAddr)
			{
				RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
				Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp = pOldAddr;
			}

			Tab->BssNr -= 1;
			return;
		}
	}
}


/*! \brief
 *	\param
 *	\return
 *	\pre
 *	\post
 */
VOID BssEntrySet(
	IN RTMP_ADAPTER *pAd,
	OUT BSS_ENTRY *pBss,
	IN BCN_IE_LIST *ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE)
{
	COPY_MAC_ADDR(pBss->Bssid, ie_list->Bssid);
	/* Default Hidden SSID to be TRUE, it will be turned to FALSE after coping SSID*/
	pBss->Hidden = 1;
	pBss->FromBcnReport = ie_list->FromBcnReport;	
	if (ie_list->SsidLen > 0)
	{
		/* For hidden SSID AP, it might send beacon with SSID len equal to 0*/
		/* Or send beacon /probe response with SSID len matching real SSID length,*/
		/* but SSID is all zero. such as "00-00-00-00" with length 4.*/
		/* We have to prevent this case overwrite correct table*/
		if (NdisEqualMemory(ie_list->Ssid, ZeroSsid, ie_list->SsidLen) == 0)
		{
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pBss->Ssid, ie_list->Ssid, ie_list->SsidLen);
			pBss->SsidLen = ie_list->SsidLen;
			pBss->Hidden = 0;
		}
	}
	else
	{
		/* avoid  Hidden SSID form beacon to overwirite correct SSID from probe response */
		if (NdisEqualMemory(pBss->Ssid, ZeroSsid, pBss->SsidLen))
		{
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			pBss->SsidLen = 0;
		}
	}

	pBss->BssType = ie_list->BssType;
	pBss->BeaconPeriod = ie_list->BeaconPeriod;
	if (ie_list->BssType == BSS_INFRA)
	{
		if (ie_list->CfParm.bValid)
		{
			pBss->CfpCount = ie_list->CfParm.CfpCount;
			pBss->CfpPeriod = ie_list->CfParm.CfpPeriod;
			pBss->CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			pBss->CfpDurRemaining = ie_list->CfParm.CfpDurRemaining;
		}
	}
	else
	{
		pBss->AtimWin = ie_list->AtimWin;
	}

	NdisGetSystemUpTime(&pBss->LastBeaconRxTime);
	pBss->CapabilityInfo = ie_list->CapabilityInfo;
	/* The privacy bit indicate security is ON, it maight be WEP, TKIP or AES*/
	/* Combine with AuthMode, they will decide the connection methods.*/
	pBss->Privacy = CAP_IS_PRIVACY_ON(pBss->CapabilityInfo);
	ASSERT(ie_list->SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	if (ie_list->SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)
		NdisMoveMemory(pBss->SupRate, ie_list->SupRate, ie_list->SupRateLen);
	else
		NdisMoveMemory(pBss->SupRate, ie_list->SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	pBss->SupRateLen = ie_list->SupRateLen;
	ASSERT(ie_list->ExtRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	if (ie_list->ExtRateLen > MAX_LEN_OF_SUPPORTED_RATES)
		ie_list->ExtRateLen = MAX_LEN_OF_SUPPORTED_RATES;
	NdisMoveMemory(pBss->ExtRate, ie_list->ExtRate, ie_list->ExtRateLen);
	pBss->NewExtChanOffset = ie_list->NewExtChannelOffset;
	pBss->ExtRateLen = ie_list->ExtRateLen;
	pBss->Erp = ie_list-> Erp;
	pBss->Channel = ie_list->Channel;
	pBss->CentralChannel = ie_list->Channel;
	pBss->Rssi = Rssi;
	/* Update CkipFlag. if not exists, the value is 0x0*/
	pBss->CkipFlag = ie_list->CkipFlag;

	/* New for microsoft Fixed IEs*/
	NdisMoveMemory(pBss->FixIEs.Timestamp, &ie_list->TimeStamp, 8);
	pBss->FixIEs.BeaconInterval = ie_list->BeaconPeriod;
	pBss->FixIEs.Capabilities = ie_list->CapabilityInfo;

	/* New for microsoft Variable IEs*/
	if (LengthVIE != 0)
	{
		pBss->VarIELen = LengthVIE;
		NdisMoveMemory(pBss->VarIEs, pVIE, pBss->VarIELen);
	}
	else
	{
		pBss->VarIELen = 0;
	}

	pBss->AddHtInfoLen = 0;
	pBss->HtCapabilityLen = 0;
#ifdef DOT11_N_SUPPORT
	if (ie_list->HtCapabilityLen> 0)
	{
		pBss->HtCapabilityLen = ie_list->HtCapabilityLen;
		NdisMoveMemory(&pBss->HtCapability, &ie_list->HtCapability, ie_list->HtCapabilityLen);
		if (ie_list->AddHtInfoLen > 0)
		{
			pBss->AddHtInfoLen = ie_list->AddHtInfoLen;
			NdisMoveMemory(&pBss->AddHtInfo, &ie_list->AddHtInfo, ie_list->AddHtInfoLen);

			pBss->CentralChannel = get_cent_ch_by_htinfo(pAd, &ie_list->AddHtInfo,
											&ie_list->HtCapability);
		}

#ifdef DOT11_VHT_AC
		if (ie_list->vht_cap_len) {
			NdisMoveMemory(&pBss->vht_cap_ie, &ie_list->vht_cap_ie, ie_list->vht_cap_len);
			pBss->vht_cap_len = ie_list->vht_cap_len;
		}

		if (ie_list->vht_op_len) {
			VHT_OP_IE *vht_op;

			NdisMoveMemory(&pBss->vht_op_ie, &ie_list->vht_op_ie, ie_list->vht_op_len);
			pBss->vht_op_len = ie_list->vht_op_len;
			vht_op = &ie_list->vht_op_ie;
			if ((vht_op->vht_op_info.ch_width > 0) &&
				(ie_list->AddHtInfo.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
				(ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40) &&
				(pBss->CentralChannel != ie_list->AddHtInfo.ControlChan))
			{
				UCHAR cent_ch;

				cent_ch = vht_cent_ch_freq(ie_list->AddHtInfo.ControlChan, pAd->CommonCfg.vht_bw);
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():VHT cent_ch=%d, vht_op_info->center_freq_1=%d, Bss->CentCh=%d, change from CentralChannel to cent_ch!\n",
											__FUNCTION__, cent_ch, vht_op->vht_op_info.center_freq_1, pBss->CentralChannel));
				pBss->CentralChannel = vht_op->vht_op_info.center_freq_1;
			}
		}
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */

	BssCipherParse(pBss);

	/* new for QOS*/
	if (ie_list->EdcaParm.bValid)
		NdisMoveMemory(&pBss->EdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
	else
		pBss->EdcaParm.bValid = FALSE;
	if (ie_list->QosCapability.bValid)
		NdisMoveMemory(&pBss->QosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
	else
		pBss->QosCapability.bValid = FALSE;
	if (ie_list->QbssLoad.bValid)
		NdisMoveMemory(&pBss->QbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
	else
		pBss->QbssLoad.bValid = FALSE;

#ifdef WH_EZ_SETUP // Rakesh: recognize an easy enabled network only if enabled on own device too, acceptable??
	//! differentiate between EZ and NON Ez beacons from other triband repeaters
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd)){
		if (ie_list->vendor_ie.support_easy_setup && !(ie_list->vendor_ie.non_ez_beacon))
		{
			pBss->support_easy_setup = 1;
			pBss->easy_setup_capability = ie_list->vendor_ie.ez_capability;
		}		
		else 
		{
			//! if it is a non ez beacon, disable EZ setup from that BSS
			pBss->non_ez_beacon = ie_list->vendor_ie.non_ez_beacon;
			pBss->support_easy_setup = 0;
			pBss->easy_setup_capability = 0;
		}
#ifdef NEW_CONNECTION_ALGO
		ez_update_bss_entry(pBss, ie_list);
#endif
	}
#endif /* WH_EZ_SETUP */

	{
		PEID_STRUCT pEid;
		USHORT Length = 0;

#ifdef WSC_INCLUDED
		pBss->WpsAP = 0x00;
		pBss->WscDPIDFromWpsAP = 0xFFFF;
#endif /* WSC_INCLUDED */

		pEid = (PEID_STRUCT) pVIE;
		while ((Length + 2 + (USHORT)pEid->Len) <= LengthVIE)
		{
#define WPS_AP		0x01
			switch(pEid->Eid)
			{
				case IE_WPA:
					if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
						)
					{
#ifdef WSC_INCLUDED
						pBss->WpsAP |= WPS_AP;
						WscCheckWpsIeFromWpsAP(pAd,
													pEid,
													&pBss->WscDPIDFromWpsAP);
#endif /* WSC_INCLUDED */
						break;
					}
#ifdef MWDS
                    check_vendor_ie(pAd, (UCHAR *)pEid, &(ie_list->vendor_ie));
                    if(ie_list->vendor_ie.mtk_cap_found)
                    {
                        if(ie_list->vendor_ie.support_mwds)
                            pBss->bSupportMWDS = TRUE;
                        else
                            pBss->bSupportMWDS = FALSE;
                    }
#endif /* MWDS */
					break;

#if defined(DOT11R_FT_SUPPORT) || defined(DOT11K_RRM_SUPPORT)
				case IE_FT_MDIE:
					if (pEid->Len == sizeof(FT_MDIE))
					{
						pBss->bHasMDIE = TRUE;
						NdisMoveMemory(&pBss->FT_MDIE, pEid->Octet, pEid->Len);
					}
					break;
#endif /* DOT11R_FT_SUPPORT || DOT11K_RRM_SUPPORT */
			}
			Length = Length + 2 + (USHORT)pEid->Len;  /* Eid[1] + Len[1]+ content[Len]*/
			pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
		}
	}
}



/*!
 *	\brief insert an entry into the bss table
 *	\param p_tab The BSS table
 *	\param Bssid BSSID
 *	\param ssid SSID
 *	\param ssid_len Length of SSID
 *	\param bss_type
 *	\param beacon_period
 *	\param timestamp
 *	\param p_cf
 *	\param atim_win
 *	\param cap
 *	\param rates
 *	\param rates_len
 *	\param channel_idx
 *	\return none
 *	\pre
 *	\post
 *	\note If SSID is identical, the old entry will be replaced by the new one

 IRQL = DISPATCH_LEVEL

 */
ULONG BssTableSetEntry(
	IN PRTMP_ADAPTER pAd,
	OUT BSS_TABLE *Tab,
	IN BCN_IE_LIST *ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE)
{
	ULONG	Idx;
#ifdef APCLI_SUPPORT
	BOOLEAN bInsert = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	UCHAR i;
#endif /* APCLI_SUPPORT */


	Idx = BssTableSearch(Tab, ie_list->Bssid, ie_list->Channel);
	if (Idx == BSS_NOT_FOUND)
	{
		if (Tab->BssNr >= MAX_LEN_OF_BSS_TABLE)
	    {
			/*
				It may happen when BSS Table was full.
				The desired AP will not be added into BSS Table
				In this case, if we found the desired AP then overwrite BSS Table.
			*/
#ifdef APCLI_SUPPORT
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++)
			{
				pApCliEntry = &pAd->ApCfg.ApCliTab[i];
				if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ie_list->Bssid)
					|| SSID_EQUAL(pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen, ie_list->Ssid, ie_list->SsidLen))
				{
					bInsert = TRUE;
					break;
				}
			}
#endif /* APCLI_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
            if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) ||
#endif                
				!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
			{
				if (MAC_ADDR_EQUAL(pAd->ScanCtrl.Bssid, ie_list->Bssid) ||
					SSID_EQUAL(pAd->ScanCtrl.Ssid, pAd->ScanCtrl.SsidLen, ie_list->Ssid, ie_list->SsidLen)
#ifdef APCLI_SUPPORT
					|| bInsert
#endif /* APCLI_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
					/* YF: Driver ScanTable full but supplicant the SSID exist on supplicant */
					|| SSID_EQUAL(pAd->cfg80211_ctrl.Cfg_pending_Ssid, pAd->cfg80211_ctrl.Cfg_pending_SsidLen, ie_list->Ssid, ie_list->SsidLen)
#endif /* RT_CFG80211_SUPPORT */
					)
				{
					Idx = Tab->BssOverlapNr;
					NdisZeroMemory(&(Tab->BssEntry[Idx]), sizeof(BSS_ENTRY));
					BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE);
					Tab->BssOverlapNr += 1;
					Tab->BssOverlapNr = Tab->BssOverlapNr % MAX_LEN_OF_BSS_TABLE;
#ifdef RT_CFG80211_SUPPORT
					pAd->cfg80211_ctrl.Cfg_pending_SsidLen = 0;
					NdisZeroMemory(pAd->cfg80211_ctrl.Cfg_pending_Ssid, MAX_LEN_OF_SSID+1);
#endif /* RT_CFG80211_SUPPORT */
				}
				return Idx;
			}
			else
			{
				return BSS_NOT_FOUND;
			}
		}
		Idx = Tab->BssNr;
		BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE);
		Tab->BssNr++;
	}
	else if (Idx < MAX_LEN_OF_BSS_TABLE)
	{
		BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE);
	}
	else
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s(error):Idx is larger than MAX_LEN_OF_BSS_TABLE", __FUNCTION__));
	}

	return Idx;
}

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID  TriEventInit(RTMP_ADAPTER *pAd)
{
	UCHAR i;

	for (i = 0;i < MAX_TRIGGER_EVENT;i++)
		pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;

	pAd->CommonCfg.TriggerEventTab.EventANo = 0;
	pAd->CommonCfg.TriggerEventTab.EventBCountDown = 0;
}


INT TriEventTableSetEntry(
	IN RTMP_ADAPTER *pAd,
	OUT TRIGGER_EVENT_TAB *Tab,
	IN UCHAR *pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN UCHAR RegClass,
	IN UCHAR ChannelNo)
{
	/* Event A, legacy AP exist.*/
	if (HtCapabilityLen == 0)
	{
		UCHAR index;

		/*
			Check if we already set this entry in the Event Table.
		*/
		for (index = 0; index<MAX_TRIGGER_EVENT; index++)
		{
			if ((Tab->EventA[index].bValid == TRUE) &&
				(Tab->EventA[index].Channel == ChannelNo) &&
				(Tab->EventA[index].RegClass == RegClass)
			)
			{
				return 0;
			}
		}

		/*
			If not set, add it to the Event table
		*/
		if (Tab->EventANo < MAX_TRIGGER_EVENT)
		{
			RTMPMoveMemory(Tab->EventA[Tab->EventANo].BSSID, pBssid, 6);
			Tab->EventA[Tab->EventANo].bValid = TRUE;
			Tab->EventA[Tab->EventANo].Channel = ChannelNo;
			if (RegClass != 0)
			{
				/* Beacon has Regulatory class IE. So use beacon's*/
				Tab->EventA[Tab->EventANo].RegClass = RegClass;
			}
			else
			{
				/* Use Station's Regulatory class instead.*/
				/* If no Reg Class in Beacon, set to "unknown"*/
				/* TODO:  Need to check if this's valid*/
				Tab->EventA[Tab->EventANo].RegClass = 0; /* ????????????????? need to check*/
			}
			Tab->EventANo ++;
		}
	}
#ifdef DOT11V_WNM_SUPPORT
	/* Not complete yet. Ignore for compliing successfully.*/
#else
	else if (pHtCapability->HtCapInfo.Forty_Mhz_Intolerant)
	{
		/* Event B.   My BSS beacon has Intolerant40 bit set*/
		Tab->EventBCountDown = pAd->CommonCfg.Dot11BssWidthChanTranDelay;
	}
#endif /* DOT11V_WNM_SUPPORT */

	return 0;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */

#if (defined(CONFIG_STA_SUPPORT) || defined(WH_EZ_SETUP))
VOID BssTableSsidSort(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT BSS_TABLE *OutTab,
	IN CHAR Ssid[],
	IN UCHAR SsidLen)
{
	INT i;
#if defined(DOT11W_PMF_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	struct _SECURITY_CONFIG *pSecConfig  = &wdev->SecConfig;
#endif /* defined(DOT11W_PMF_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
#ifdef WH_EZ_SETUP
	UCHAR RfIC;
	UCHAR BssType;
#endif

	// Rakesh: no need for easy enabled checks below

	BssTableInit(OutTab);

#ifdef WH_EZ_SETUP
	BssType = BSS_INFRA;
#endif


#ifdef WH_EZ_SETUP
	RfIC = wmode_2_rfic(wdev->PhyMode);
#endif
	for (i = 0; i < pAd->ScanTab.BssNr && pAd->ScanTab.BssNr < MAX_LEN_OF_BSS_TABLE; i++)
	{
		BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];
		BOOLEAN	bIsHiddenApIncluded = FALSE;
#ifdef WH_EZ_SETUP
#ifdef NEW_CONNECTION_ALGO
		if(IS_EZ_SETUP_ENABLED(wdev))
		{
#ifdef EZ_MOD_SUPPORT
			if (ez_is_weight_same(wdev,pInBss->beacon_info.network_weight))
#else
			if (ez_is_weight_same(ez_adapter.device_info.network_weight,
				pInBss->beacon_info.network_weight))
#endif
			{
				continue;
			}
		}
#endif
#endif
		if ( ((pAd->CommonCfg.bIEEE80211H == 1) &&
#ifdef WH_EZ_SETUP
				(RfIC & RFIC_5GHZ) &&
#else
				(pStaCfg->MlmeAux.Channel > 14) &&
#endif
				 RadarChannelCheck(pAd, pInBss->Channel))
#ifdef WIFI_REGION32_HIDDEN_SSID_SUPPORT
			 ||((pInBss->Channel == 12) || (pInBss->Channel == 13))
#endif /* WIFI_REGION32_HIDDEN_SSID_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier             */
             || (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
            )
		{
			if (pInBss->Hidden)
				bIsHiddenApIncluded = TRUE;
		}


#ifdef DOT11W_PMF_SUPPORT
		if ((IS_AKM_WPA2(pSecConfig->AKMMap)|| IS_AKM_WPA2PSK(pSecConfig->AKMMap))
			&& (pInBss))
		{
			RSN_CAPABILITIES RsnCap;

			NdisMoveMemory(&RsnCap, &pInBss->RsnCapability, sizeof(RSN_CAPABILITIES));
			RsnCap.word = cpu2le16(RsnCap.word);

			if ((pSecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
				continue;

			if (((pSecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPC == FALSE))
				|| (pSecConfig->PmfCfg.MFPC && RsnCap.field.MFPC && (pSecConfig->PmfCfg.MFPR == FALSE) && (RsnCap.field.MFPR == FALSE)))
			{
				if ((pSecConfig->PmfCfg.PMFSHA256) && (pInBss->IsSupportSHA256KeyDerivation == FALSE))
						continue;
			}
		}
#endif /* DOT11W_PMF_SUPPORT */

#if(defined(DBDC_MODE) && defined(WH_EZ_SETUP))
		if((pAd->CommonCfg.dbdc_mode) && IS_EZ_SETUP_ENABLED(wdev)){
			if( ( WMODE_2G_ONLY(wdev->PhyMode) && (pInBss->Channel <= 14) ) ||
			    ( WMODE_5G_ONLY(wdev->PhyMode) && (pInBss->Channel > 14) ) )
			{
				// Own band network
			}
			else{
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Ignore Other Band network.\n"));
				continue;
			}
		}
#endif

		if (
#ifdef WH_EZ_SETUP
			(pInBss->BssType == BssType) &&
#else
			(pInBss->BssType == pStaCfg->BssType) &&
#endif
			(((pInBss->SsidLen <= MAX_LEN_OF_SSID) && SSID_EQUAL(Ssid, SsidLen, pInBss->Ssid, pInBss->SsidLen)) || bIsHiddenApIncluded)
				 && (OutTab->BssNr < MAX_LEN_OF_BSS_TABLE))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];


#ifdef DOT11_N_SUPPORT
			/* 2.4G/5G N only mode*/
			if ((pInBss->HtCapabilityLen == 0) &&
				(WMODE_HT_ONLY(wdev->PhyMode)))
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			if ((wdev->PhyMode == (WMODE_G | WMODE_GN)) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12))
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("STA is in GN-only Mode, this AP is in B mode.\n"));
				continue;
			}
#endif /* DOT11_N_SUPPORT */

			/* Since the AP is using hidden SSID, and we are trying to connect to ANY*/
			/* It definitely will fail. So, skip it.*/
			/* CCX also require not even try to connect it!!*/
			if (SsidLen == 0)
				continue;

			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("-------->%s(%d), AKMMap=0x%x, PairwiseCipher=0x%x, GroupCipher=0x%x, CapabilityInfo=0x%x\n", 
				__FUNCTION__, __LINE__, pInBss->AKMMap, pInBss->PairwiseCipher, pInBss->GroupCipher, pInBss->CapabilityInfo));

			OutTab->BssNr++;
		}
		else if (
#ifdef WH_EZ_SETUP
				(pInBss->BssType == BssType) && 
#else
				(pInBss->BssType == pStaCfg->BssType) &&
#endif
				(SsidLen == 0) && OutTab->BssNr < MAX_LEN_OF_BSS_TABLE)
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];


#ifdef DOT11_N_SUPPORT
			/* 2.4G/5G N only mode*/
			if ((pInBss->HtCapabilityLen == 0) &&
				WMODE_HT_ONLY(wdev->PhyMode))
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			if ((wdev->PhyMode == (WMODE_G | WMODE_GN)) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12))
			{
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("STA is in GN-only Mode, this AP is in B mode.\n"));
				continue;
			}
#endif /* DOT11_N_SUPPORT */

			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("-------->%s(%d), AKMMap=0x%x, PairwiseCipher=0x%x\n", 
				__FUNCTION__, __LINE__, pInBss->AKMMap, pInBss->PairwiseCipher));

			OutTab->BssNr++;
		}
#if defined(CONFIG_STA_SUPPORT) && defined(WSC_STA_SUPPORT)
		else if ((wdev->wdev_type == WDEV_TYPE_STA) &&
				 (pWpsCtrl->WscConfMode != WSC_DISABLE) &&
				 (pWpsCtrl->bWscTrigger) &&
				 MAC_ADDR_EQUAL(pWpsCtrl->WscBssid, pInBss->Bssid) &&
					 OutTab->BssNr < MAX_LEN_OF_BSS_TABLE)
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			/*
				Linksys WRT610N WPS AP will change the SSID from linksys to linksys_WPS_<four random characters>
				when the Linksys WRT610N is in the state 'WPS Unconfigured' after set to factory default.
			*/
			NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pStaCfg->MlmeAux.Ssid, pInBss->Ssid, pInBss->SsidLen);
			pStaCfg->MlmeAux.SsidLen = pInBss->SsidLen;


			/* Update Reconnect Ssid, that user desired to connect.*/

			NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.Ssid, pStaCfg->MlmeAux.SsidLen);
			pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->MlmeAux.SsidLen;

			OutTab->BssNr++;
			continue;
		}
#endif /* defined(CONFIG_STA_SUPPORT) && defined(WSC_STA_SUPPORT) */

		if (OutTab->BssNr >= MAX_LEN_OF_BSS_TABLE)
			break;
	}

	BssTableSortByRssi(OutTab, FALSE);
}
#endif

VOID BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder)
{
	INT i, j;
	BSS_ENTRY *pTmpBss = NULL;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pTmpBss, sizeof(BSS_ENTRY));
	if (pTmpBss == NULL)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	for (i = 0; i < OutTab->BssNr - 1; i++)
	{
		for (j = i+1; j < OutTab->BssNr; j++)
		{
			if (OutTab->BssEntry[j].Rssi > OutTab->BssEntry[i].Rssi ?
				!isInverseOrder : isInverseOrder)
			{
				if (OutTab->BssEntry[j].Rssi != OutTab->BssEntry[i].Rssi )
				{
					NdisMoveMemory(pTmpBss, &OutTab->BssEntry[j], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[j], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[i], pTmpBss, sizeof(BSS_ENTRY));
				}
			}
		}
	}

	if (pTmpBss != NULL)
		os_free_mem(pTmpBss);
}


VOID BssCipherParse(BSS_ENTRY *pBss)
{
	PEID_STRUCT 		 pEid;
	PUCHAR				pTmp;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	PCIPHER_SUITE_STRUCT			pCipher;
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	SHORT								Length;

	/* WepStatus will be reset later, if AP announce TKIP or AES on the beacon frame.*/
	CLEAR_SEC_AKM(pBss->AKMMap);
	CLEAR_CIPHER(pBss->PairwiseCipher);
	CLEAR_CIPHER(pBss->GroupCipher);

	Length = (SHORT) pBss->VarIELen;

	while (Length > 0)
	{
		/* Parse cipher suite base on WPA1 & WPA2, they should be parsed differently*/
		pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - ((USHORT)Length);
		pEid = (PEID_STRUCT) pTmp;
		switch (pEid->Eid)
		{
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7))
				{
					pBss->bSES = TRUE;
					break;
				}
				else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
				{
					/* if unsupported vendor specific IE*/
					break;
				}
				/*
					Skip OUI, version, and multicast suite
					This part should be improved in the future when AP supported multiple cipher suite.
					For now, it's OK since almost all APs have fixed cipher suite supported.
				*/
				/* pTmp = (PUCHAR) pEid->Octet;*/
				pTmp   += 11;

				/*
					Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
					Value	   Meaning
					0			None
					1			WEP-40
					2			Tkip
					3			WRAP
					4			AES
					5			WEP-104
				*/
				/* Parse group cipher*/
				switch (*pTmp)
				{
					case 1:
						SET_CIPHER_WEP40(pBss->GroupCipher);
						break;
					case 5:
						SET_CIPHER_WEP104(pBss->GroupCipher);
						break;
					case 2:
						SET_CIPHER_TKIP(pBss->GroupCipher);
						break;
					case 4:
						SET_CIPHER_CCMP128(pBss->GroupCipher);
						break;
					default:
						break;
				}
				/* number of unicast suite*/
				pTmp   += 1;

				/* skip all unicast cipher suites*/
				/*Count = *(PUSHORT) pTmp;				*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pTmp += 3;
					switch (*pTmp)
					{
						case 1:
							SET_CIPHER_WEP40(pBss->PairwiseCipher);
							break;
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
							SET_CIPHER_WEP104(pBss->PairwiseCipher);
							break;
						case 2:
							SET_CIPHER_TKIP(pBss->PairwiseCipher);
							break;
						case 4:
							SET_CIPHER_CCMP128(pBss->PairwiseCipher);
							break;
						default:
							break;
					}
					pTmp++;
					Count--;
				}

				/* 4. get AKM suite counts*/
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				pTmp   += 3;

				switch (*pTmp)
				{
					case 1:
						/* Set AP support WPA-enterprise mode*/
						SET_AKM_WPA1(pBss->AKMMap);
						break;
					case 2:
						/* Set AP support WPA-PSK mode*/
						SET_AKM_WPA1PSK(pBss->AKMMap);
						break;
					default:
						break;
				}
				pTmp   += 1;

				/* Fixed for WPA-None*/
				if (pBss->BssType == BSS_ADHOC)
				{
					SET_AKM_WPANONE(pBss->AKMMap);
				}
				break;

			case IE_RSN:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

				/* 0. Version must be 1*/
				if (le2cpu16(pRsnHeader->Version) != 1)
					break;
				pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Check group cipher*/
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

				if (!RTMPEqualMemory(&pCipher->Oui, RSN_OUI, 3))
					break;

				/* Parse group cipher*/
				switch (pCipher->Type)
				{
					case 1:
						SET_CIPHER_WEP40(pBss->GroupCipher);
						break;
					case 2:
						SET_CIPHER_TKIP(pBss->GroupCipher);
						break;
					case 4:
						SET_CIPHER_CCMP128(pBss->GroupCipher);
						break;
					case 5:
						SET_CIPHER_WEP104(pBss->GroupCipher);
						break;
					case 8:
						SET_CIPHER_GCMP128(pBss->GroupCipher);
						break;
					case 9:
						SET_CIPHER_GCMP256(pBss->GroupCipher);
						break;
					case 10:
						SET_CIPHER_CCMP256(pBss->GroupCipher);
						break;
					default:
						break;
				}
				/* set to correct offset for next parsing*/
				pTmp   += sizeof(CIPHER_SUITE_STRUCT);

				/* 2. Get pairwise cipher counts*/
				/*Count = *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* 3. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
					switch (pCipher->Type)
					{
						case 1:
							SET_CIPHER_WEP40(pBss->PairwiseCipher);
							break;
						case 2:
							SET_CIPHER_TKIP(pBss->PairwiseCipher);
							break;
						case 4:
							SET_CIPHER_CCMP128(pBss->PairwiseCipher);
							break;
						case 5:
							SET_CIPHER_WEP104(pBss->PairwiseCipher);
							break;
						case 8:
							SET_CIPHER_GCMP128(pBss->PairwiseCipher);
							break;
						case 9:
							SET_CIPHER_GCMP256(pBss->PairwiseCipher);
							break;
						case 10:
							SET_CIPHER_CCMP256(pBss->PairwiseCipher);
							break;
						default:
							break;
					}
					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}

				/* 4. get AKM suite counts*/
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* 5. Get AKM ciphers*/
				/* Parsing all AKM ciphers*/
				while (Count > 0)
				{
					pAKM = (PAKM_SUITE_STRUCT) pTmp;
					if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
						break;

					switch (pAKM->Type)
					{
						case 0:
							SET_AKM_WPANONE(pBss->AKMMap);
							break;
						case 1:
							SET_AKM_WPA2(pBss->AKMMap);
							break;
						case 2:
							SET_AKM_WPA2PSK(pBss->AKMMap);
							break;
						case 3:
							SET_AKM_FT_WPA2(pBss->AKMMap);
							break;
						case 4:
							SET_AKM_FT_WPA2PSK(pBss->AKMMap);
							break;
#ifdef DOT11W_PMF_SUPPORT
						case 5:
//							SET_AKM_WPA2_SHA256(pBss->AKMMap);
							SET_AKM_WPA2(pBss->AKMMap);
							pBss->IsSupportSHA256KeyDerivation = TRUE;
							break;
						case 6:
//							SET_AKM_WPA2PSK_SHA256(pBss->AKMMap);
							SET_AKM_WPA2PSK(pBss->AKMMap);
							pBss->IsSupportSHA256KeyDerivation = TRUE;
							break;
#endif /* DOT11W_PMF_SUPPORT */
						case 7:
							SET_AKM_TDLS(pBss->AKMMap);
							break;
						case 8:
							SET_AKM_SAE_SHA256(pBss->AKMMap);
							break;
						case 9:
							SET_AKM_FT_SAE_SHA256(pBss->AKMMap);
							break;
						case 11:
							SET_AKM_SUITEB_SHA256(pBss->AKMMap);
							break;
						case 12:
							SET_AKM_SUITEB_SHA384(pBss->AKMMap);
							break;
						case 13:
							SET_AKM_FT_WPA2_SHA384(pBss->AKMMap);
							break;
						default:
							break;
					}
					pTmp   += sizeof(AKM_SUITE_STRUCT);
					Count--;
				}

				/* Fixed for WPA-None*/
				if (pBss->BssType == BSS_ADHOC)
				{
					SET_AKM_WPANONE(pBss->AKMMap);
				}

				/* 6. Get RSN capability*/
				/*pBss->WPA2.RsnCapability = *(PUSHORT) pTmp;*/
				pBss->RsnCapability = (pTmp[1]<<8) + pTmp[0];
				pTmp += sizeof(USHORT);

				break;
#ifdef WAPI_SUPPORT
			case IE_WAPI:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

				/* 0. The version number must be 1*/
				if (le2cpu16(pRsnHeader->Version) != 1)
					break;
				pTmp += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Get AKM suite counts*/
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
	    			Count = cpu2le16(Count);
				pTmp += sizeof(USHORT);

				/* 2. Get AKM ciphers*/
				pAKM = (PAKM_SUITE_STRUCT) pTmp;
				if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
					break;

				switch (pAKM->Type)
				{
					case 1:
						SET_AKM_WAICERT(pBss->AKMMap);
						break;
					case 2:
						SET_AKM_WPIPSK(pBss->AKMMap);
						break;
					default:
						break;
				}
				pTmp += (Count * sizeof(AKM_SUITE_STRUCT));

				/* 3. Get pairwise cipher counts*/
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
    				Count = cpu2le16(Count);
				pTmp += sizeof(USHORT);

				/* 4. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
						break;

					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
					switch (pCipher->Type)
					{
						case 1:
							SET_CIPHER_WPI_SMS4(pBss->PairwiseCipher);
							break;
						default:
							break;
					}

					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}

				/* 5. Check group cipher*/
				if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
					break;

				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
				/* Parse group cipher*/
				switch (pCipher->Type)
				{
					case 1:
						SET_CIPHER_WPI_SMS4(pBss->GroupCipher);
						break;
					default:
						break;
				}
				/* set to correct offset for next parsing*/
				pTmp += sizeof(CIPHER_SUITE_STRUCT);

				/* update the WAPI capability*/
				pBss->RsnCapability = (pTmp[1]<<8) + pTmp[0];
				pTmp += sizeof(USHORT);

				break;
#endif /* WAPI_SUPPORT */
			default:
				break;
		}
		Length -= (pEid->Len + 2);
	}

	if (pBss->AKMMap == 0x0)
	{
		SET_AKM_OPEN(pBss->AKMMap);
		if (pBss->Privacy)
		{
			SET_AKM_SHARED(pBss->AKMMap);
			SET_CIPHER_WEP(pBss->PairwiseCipher);
			SET_CIPHER_WEP(pBss->GroupCipher);
		} else {
			SET_CIPHER_NONE(pBss->PairwiseCipher);
			SET_CIPHER_NONE(pBss->GroupCipher);
		}
	}

}


/*! \brief generates a random mac address value for IBSS BSSID
 *	\param Addr the bssid location
 *	\return none
 *	\pre
 *	\post
 */
VOID MacAddrRandomBssid(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i;

	for (i = 0; i < MAC_ADDR_LEN; i++)
	{
		pAddr[i] = RandomByte(pAd);
	}

	pAddr[0] = (pAddr[0] & 0xfe) | 0x02;  /* the first 2 bits must be 01xxxxxxxx*/
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID AssocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
	IN PUCHAR                     pAddr,
	IN USHORT                     CapabilityInfo,
	IN ULONG                      Timeout,
	IN USHORT                     ListenIntv)
{
	COPY_MAC_ADDR(AssocReq->Addr, pAddr);
	/* Add mask to support 802.11b mode only */
	AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; /* not cf-pollable, not cf-poll-request*/
	AssocReq->Timeout = Timeout;
	AssocReq->ListenIntv = ListenIntv;
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL

	==========================================================================
*/
VOID DisassocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	IN PUCHAR pAddr,
	IN USHORT Reason)
{
	COPY_MAC_ADDR(DisassocReq->Addr, pAddr);
	DisassocReq->Reason = Reason;
}


/*! \brief init the management mac frame header
 *	\param p_hdr mac header
 *	\param subtype subtype of the frame
 *	\param p_ds destination address, don't care if it is a broadcast address
 *	\return none
 *	\pre the station has the following information in the pAd->StaCfg[0]
 *	 - bssid
 *	 - station address
 *	\post
 *	\note this function initializes the following field
 */
VOID MgtMacHeaderInit(
	IN RTMP_ADAPTER *pAd,
	INOUT HEADER_802_11 *pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN UCHAR *pDA,
	IN UCHAR *pSA,
	IN UCHAR *pBssid)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));

	pHdr80211->FC.Type = FC_TYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
		COPY_MAC_ADDR(pHdr80211->Addr2, pSA);
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		COPY_MAC_ADDR(pHdr80211->Addr2, pBssid);
#endif /* CONFIG_AP_SUPPORT */
#endif /* P2P_SUPPORT */
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}


VOID MgtMacHeaderInitExt(
    IN RTMP_ADAPTER *pAd,
    IN OUT HEADER_802_11 *pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN UCHAR *pAddr1,
    IN UCHAR *pAddr2,
    IN UCHAR *pAddr3)
{
    NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));

    pHdr80211->FC.Type = FC_TYPE_MGMT;
    pHdr80211->FC.SubType = SubType;
    pHdr80211->FC.ToDs = ToDs;
    COPY_MAC_ADDR(pHdr80211->Addr1, pAddr1);
    COPY_MAC_ADDR(pHdr80211->Addr2, pAddr2);
    COPY_MAC_ADDR(pHdr80211->Addr3, pAddr3);
}


/*!***************************************************************************
 * This routine build an outgoing frame, and fill all information specified
 * in argument list to the frame body. The actual frame size is the summation
 * of all arguments.
 * input params:
 *		Buffer - pointer to a pre-allocated memory segment
 *		args - a list of <int arg_size, arg> pairs.
 *		NOTE NOTE NOTE!!!! the last argument must be NULL, otherwise this
 *						   function will FAIL!!!
 * return:
 *		Size of the buffer
 * usage:
 *		MakeOutgoingFrame(Buffer, output_length, 2, &fc, 2, &dur, 6, p_addr1, 6,p_addr2, END_OF_ARGS);

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 ****************************************************************************/
ULONG MakeOutgoingFrame(UCHAR *Buffer, ULONG *FrameLen, ...)
{
	UCHAR   *p;
	int 	leng;
	ULONG	TotLeng;
	va_list Args;

	/* calculates the total length*/
	TotLeng = 0;
	va_start(Args, FrameLen);
	do
	{
		leng = va_arg(Args, int);
		if (leng == END_OF_ARGS)
		{
			break;
		}
		p = va_arg(Args, PVOID);
		NdisMoveMemory(&Buffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while(TRUE);

	va_end(Args); /* clean up */
	*FrameLen = TotLeng;
	return TotLeng;
}


/*! \brief	Initialize The MLME Queue, used by MLME Functions
 *	\param	*Queue	   The MLME Queue
 *	\return Always	   Return NDIS_STATE_SUCCESS in this implementation
 *	\pre
 *	\post
 *	\note	Because this is done only once (at the init stage), no need to be locked

 IRQL = PASSIVE_LEVEL

 */
NDIS_STATUS MlmeQueueInit(RTMP_ADAPTER *pAd, MLME_QUEUE *Queue)
{
	INT i;

	NdisAllocateSpinLock(pAd, &Queue->Lock);

	Queue->Num	= 0;
	Queue->Head = 0;
	Queue->Tail = 0;

	for (i = 0; i < MAX_LEN_OF_MLME_QUEUE; i++)
	{
		Queue->Entry[i].Occupied = FALSE;
		Queue->Entry[i].MsgLen = 0;
		NdisZeroMemory(Queue->Entry[i].Msg, MGMT_DMA_BUFFER_SIZE);
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_AP_SUPPORT

/*! \brief	 Enqueue a message for other threads, if they want to send messages to MLME thread
 *	\param	*Queue	  The MLME Queue
 *	\param	 Machine  The State Machine Id
 *	\param	 MsgType  The Message Type
 *	\param	 MsgLen   The Message length
 *	\param	*Msg	  The message pointer
 *	\return  TRUE if enqueue is successful, FALSE if the queue is full
 *	\pre
 *	\post
 *	\note	 The message has to be initialized
 */
BOOLEAN MlmeEnqueue(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN ULONG Priv)
{
	INT Tail;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueue: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 1))
	{
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	Queue->Entry[Tail].Wcid = RESERVED_WCID;
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;
	Queue->Entry[Tail].Priv = Priv;

	if (Msg != NULL)
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}

	NdisReleaseSpinLock(&(Queue->Lock));

	return TRUE;
}

#endif



/*! \brief	 This function is used when Recv gets a MLME message
 *	\param	*Queue			 The MLME Queue
 *	\param	 TimeStampHigh	 The upper 32 bit of timestamp
 *	\param	 TimeStampLow	 The lower 32 bit of timestamp
 *	\param	 Rssi			 The receiving RSSI strength
 *	\param	 MsgLen 		 The length of the message
 *	\param	*Msg			 The message pointer
 *	\return  TRUE if everything ok, FALSE otherwise (like Queue Full)
 *	\pre
 *	\post
 */
BOOLEAN MlmeEnqueueForRecv(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Wcid,
	IN struct raw_rssi_info *rssi_info,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN UCHAR OpMode,
	IN struct wifi_dev *wdev,
	IN UCHAR RxPhyMode
	)
{
	INT Tail, Machine = 0xff;
	UINT32 TimeStampHigh, TimeStampLow;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
	INT MsgType = 0x0;
	MLME_QUEUE *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
    BOOLEAN bToApCli = FALSE;
	UCHAR ApCliIdx = 0;
	BOOLEAN ApCliIdx_find = FALSE;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0;
    REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
    RTMP_CHIP_CAP *cap = &pAd->chipCap;
#endif /* MAC_REPEATER_SUPPORT */
#endif

#if defined(CONFIG_AP_SUPPORT) && defined(MBSS_SUPPORT)
#ifdef APCLI_SUPPORT
	UCHAR MBSSIdx = 0;
	BOOLEAN ApBssIdx_find = FALSE;
#endif /* APCLI_SUPPORT */
#endif

#ifdef CONFIG_ATE
	/* Nothing to do in ATE mode */
	if(ATE_ON(pAd))
		return FALSE;
#endif /* CONFIG_ATE */

	/*
		Do nothing if the driver is starting halt state.
		This might happen when timer already been fired before cancel timer with mlmehalt
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __FUNCTION__));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): frame too large, size = %ld \n", __FUNCTION__, MsgLen));
		return FALSE;
	}

	if (Msg == NULL)
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): frame is Null \n", __FUNCTION__));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 0))
	{
		RTMP_MLME_HANDLER(pAd);

		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	if (OpMode == OPMODE_AP)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
	{
		if(MAC_ADDR_EQUAL(ZERO_MAC_ADDR,pFrame->Hdr.Addr1) && pFrame->Hdr.FC.SubType == SUBTYPE_DEAUTH)
		{	
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR
				, ("%s(): receiving DEAUTH with (DA/BSSID) all zero mac addr, skip \n", __FUNCTION__));
			return FALSE;
		}

#ifdef APCLI_SUPPORT
		/*
			Beacon must be handled by ap-sync state machine.
			Probe-rsp must be handled by apcli-sync state machine.
			Those packets don't need to check its MAC address
		*/
		do
		{
			UCHAR i;
			/*
			   1. When P2P GO On and receive Probe Response, preCheckMsgTypeSubset function will
			      enquene Probe response to APCli sync state machine
			      Solution: when GO On skip preCheckMsgTypeSubset redirect to APMsgTypeSubst
			   2. When P2P Cli On and receive Probe Response, preCheckMsgTypeSubset function will
			      enquene Probe response to APCli sync state machine
			      Solution: handle MsgType == APCLI_MT2_PEER_PROBE_RSP on ApCli Sync state machine
			                when ApCli on idle state.
			*/
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++)
			{
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].MlmeAux.Bssid, pFrame->Hdr.Addr2))
				{
					ApCliIdx = i;
					bToApCli = TRUE;
					break;
				}
			}
			/* check if da is to apcli */
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++)
			{
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].wdev.if_addr, pFrame->Hdr.Addr1))
				{
					ApCliIdx = i;
					bToApCli = TRUE;
					ApCliIdx_find = TRUE;
					break;
				}
			}

			/* check if da is to ap */
			for (i = 0; i < pAd->ApCfg.BssidNum; i++)
			{
				if (MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[i].wdev.if_addr, pFrame->Hdr.Addr1))
				{
#if defined(CONFIG_AP_SUPPORT) && defined(MBSS_SUPPORT)
					MBSSIdx = i;
					ApBssIdx_find = TRUE;
#endif
					break;
				}
			}

			if ((ApBssIdx_find == FALSE) && 
				preCheckMsgTypeSubset(pAd, pFrame, &Machine, &MsgType)) {
				/* ap case and apcli case*/
				if (bToApCli == TRUE) {
					if ((Machine == AP_SYNC_STATE_MACHINE) &&
							(MsgType == APMT2_PEER_BEACON)) {
						ULONG Now32;
						NdisGetSystemUpTime(&Now32);
						pAd->ApCfg.ApCliTab[ApCliIdx].ApCliRcvBeaconTime_MlmeEnqueueForRecv = Now32;
						pAd->ApCfg.ApCliTab[ApCliIdx].ApCliRcvBeaconTime_MlmeEnqueueForRecv_2 = pAd->Mlme.Now32;
					}
				}
				break;
			}
			
			if ((ApBssIdx_find == FALSE) && 
				bToApCli) {
				if (ApCliMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
					/* apcli and repeater case */
					break;
				}
			}
			if (APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) {
				/* ap case */
				break;
			}
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): un-recongnized mgmt->subtype=%d, STA-%02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__, pFrame->Hdr.FC.SubType, PRINT_MAC(pFrame->Hdr.Addr2)));
			return FALSE;

		} while (FALSE);
#else
		if (!APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
		{
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): un-recongnized mgmt->subtype=%d\n",
							__FUNCTION__, pFrame->Hdr.FC.SubType));
			return FALSE;
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	TimeStampHigh = TimeStampLow = 0;
#ifdef RTMP_MAC_PCI
	if ((!IS_USB_INF(pAd)) && (pAd->chipCap.hif_type != HIF_MT))
		AsicGetTsfTime(pAd, &TimeStampHigh, &TimeStampLow, HW_BSSID_0);
#endif /* RTMP_MAC_PCI */

	/* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	/*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
		Queue->Tail = 0;

	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = TimeStampLow;
	Queue->Entry[Tail].TimeStamp.u.HighPart = TimeStampHigh;
	NdisMoveMemory(&Queue->Entry[Tail].rssi_info, rssi_info, sizeof(struct raw_rssi_info));
	Queue->Entry[Tail].Signal = rssi_info->raw_snr;
	Queue->Entry[Tail].Wcid = (UCHAR)Wcid;
	Queue->Entry[Tail].OpMode = (ULONG)OpMode;
#ifdef MT7615
	Queue->Entry[Tail].Channel = (rssi_info->Channel == 0) ? pAd->LatchRfRegs.Channel:rssi_info->Channel;
#else
		Queue->Entry[Tail].Channel = pAd->LatchRfRegs.Channel;
#endif

	Queue->Entry[Tail].Priv = 0;
	Queue->Entry[Tail].RxPhyMode = RxPhyMode;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	Queue->Entry[Tail].Priv = ApCliIdx;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	if ((pAd->ApCfg.bMACRepeaterEn) && (bToApCli == TRUE))
	{
        NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++)
		{
            pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
                        
			if (MAC_ADDR_EQUAL(pReptEntry->CurrentAddress, pFrame->Hdr.Addr1))
			{
				Queue->Entry[Tail].Priv = (REPT_MLME_START_IDX + CliIdx);
				
				break;
			}
		}
        NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	}
#endif /* MAC_REPEATER_SUPPORT */
#endif /*CONFIG_AP_SUPPORT*/


	if (Msg != NULL)
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}

	NdisReleaseSpinLock(&(Queue->Lock));
	RTMP_MLME_HANDLER(pAd);

	return TRUE;
}


#ifdef WSC_INCLUDED
/*! \brief   Enqueue a message for other threads, if they want to send messages to MLME thread
 *  \param  *Queue    The MLME Queue
 *  \param   TimeStampLow    The lower 32 bit of timestamp, here we used for eventID.
 *  \param   Machine  The State Machine Id
 *  \param   MsgType  The Message Type
 *  \param   MsgLen   The Message length
 *  \param  *Msg      The message pointer
 *  \return  TRUE if enqueue is successful, FALSE if the queue is full
 *  \pre
 *  \post
 *  \note    The message has to be initialized
 */
BOOLEAN MlmeEnqueueForWsc(
	IN RTMP_ADAPTER *pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg)
{
	INT Tail;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----> MlmeEnqueueForWsc\n"));
    /* Do nothing if the driver is starting halt state.*/
    /* This might happen when timer already been fired before cancel timer with mlmehalt*/
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
        MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeEnqueueForWsc: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}

    if (MlmeQueueFull(Queue, 1))
    {

        return FALSE;
    }

    /* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
    Tail = Queue->Tail;
    /*
		Double check for safety in multi-thread system.
	*/
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
    Queue->Tail++;
    Queue->Num++;
    if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
    {
        Queue->Tail = 0;
    }

    Queue->Entry[Tail].Occupied = TRUE;
    Queue->Entry[Tail].Machine = Machine;
    Queue->Entry[Tail].MsgType = MsgType;
    Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = eventID;
	Queue->Entry[Tail].TimeStamp.u.HighPart = senderID;
    if (Msg != NULL)
        NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

    NdisReleaseSpinLock(&(Queue->Lock));

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<----- MlmeEnqueueForWsc\n"));

    return TRUE;
}
#endif /* WSC_INCLUDED */


/*! \brief	 Dequeue a message from the MLME Queue
 *	\param	*Queue	  The MLME Queue
 *	\param	*Elem	  The message dequeued from MLME Queue
 *	\return  TRUE if the Elem contains something, FALSE otherwise
 *	\pre
 *	\post
 */
BOOLEAN MlmeDequeue(MLME_QUEUE *Queue, MLME_QUEUE_ELEM **Elem)
{
	NdisAcquireSpinLock(&(Queue->Lock));
	*Elem = &(Queue->Entry[Queue->Head]);
	Queue->Num--;
	Queue->Head++;
	if (Queue->Head == MAX_LEN_OF_MLME_QUEUE)
	{
		Queue->Head = 0;
	}
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


static VOID MlmeSwitchChannelByRf(RTMP_ADAPTER *pAd,UCHAR RfIC)
{
	BOOLEAN IsSupportRf = HcIsRfSupport(pAd,RfIC);
	UCHAR Channel = HcGetChannelByRf(pAd,RfIC);

	if(IsSupportRf)
	{
		AsicSwitchChannel(pAd, Channel, FALSE);
		AsicLockChannel(pAd, Channel);
	}
}


VOID MlmeRestartStateMachine(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef RTMP_MAC_PCI
	MLME_QUEUE_ELEM *Elem = NULL;
#endif /* RTMP_MAC_PCI */


	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeRestartStateMachine \n"));

#ifdef RTMP_MAC_PCI
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning)
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	}
	else
	{
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
	/* Remove all Mlme queues elements*/
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue))
	{
		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem))
		{
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeRestartStateMachine: MlmeQueue empty\n"));
		}
	}
#endif /* RTMP_MAC_PCI */


	/* Change back to original channel in case of doing scan*/
	{
		MlmeSwitchChannelByRf(pAd,RFIC_24GHZ);
		MlmeSwitchChannelByRf(pAd,RFIC_5GHZ);
	}


#if defined(RTMP_MAC_PCI) && !defined(CONFIG_STA_SUPPORT)
	/* Remove running state*/
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#endif /* RTMP_MAC_PCI */

//CFG_TODO for SCAN
#ifdef RT_CFG80211_SUPPORT
	RTEnqueueInternalCmd(pAd, CMDTHREAD_SCAN_END, NULL, 0);
#endif /* RT_CFG80211_SUPPORT */
}


/*! \brief	test if the MLME Queue is empty
 *	\param	*Queue	  The MLME Queue
 *	\return TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeQueueEmpty(MLME_QUEUE *Queue)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == 0);
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}

/*! \brief	 test if the MLME Queue is full
 *	\param	 *Queue 	 The MLME Queue
 *	\return  TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeQueueFull(MLME_QUEUE *Queue, UCHAR SendId)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	if (SendId == 0)
		Ans = ((Queue->Num >= (MAX_LEN_OF_MLME_QUEUE / 2)) || Queue->Entry[Queue->Tail].Occupied);
	else
		Ans = ((Queue->Num == MAX_LEN_OF_MLME_QUEUE) || Queue->Entry[Queue->Tail].Occupied);
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}


/*! \brief	 The destructor of MLME Queue
 *	\param
 *	\return
 *	\pre
 *	\post
 *	\note	Clear Mlme Queue, Set Queue->Num to Zero.

 IRQL = PASSIVE_LEVEL

 */
VOID MlmeQueueDestroy(MLME_QUEUE *pQueue)
{
	NdisAcquireSpinLock(&(pQueue->Lock));
	pQueue->Num  = 0;
	pQueue->Head = 0;
	pQueue->Tail = 0;
	NdisReleaseSpinLock(&(pQueue->Lock));
	NdisFreeSpinLock(&(pQueue->Lock));
}


/*! \brief	 To substitute the message type if the message is coming from external
 *	\param	pFrame		   The frame received
 *	\param	*Machine	   The state machine
 *	\param	*MsgType	   the message type for the state machine
 *	\return TRUE if the substitution is successful, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */


/*! \brief Initialize the state machine.
 *	\param *S			pointer to the state machine
 *	\param	Trans		State machine transition function
 *	\param	StNr		number of states
 *	\param	MsgNr		number of messages
 *	\param	DefFunc 	default function, when there is invalid state/message combination
 *	\param	InitState	initial state of the state machine
 *	\param	Base		StateMachine base, internal use only
 *	\pre p_sm should be a legal pointer
 *	\post

 IRQL = PASSIVE_LEVEL

 */
VOID StateMachineInit(
	IN STATE_MACHINE *S,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base)
{
	ULONG i, j;

	/* set number of states and messages*/
	S->NrState = StNr;
	S->NrMsg   = MsgNr;
	S->Base    = Base;

	S->TransFunc  = Trans;

	/* init all state transition to default function*/
	for (i = 0; i < StNr; i++)
	{
		for (j = 0; j < MsgNr; j++)
		{
			S->TransFunc[i * MsgNr + j] = DefFunc;
		}
	}

	/* set the starting state*/
	S->CurrState = InitState;
}


/*! \brief This function fills in the function pointer into the cell in the state machine
 *	\param *S	pointer to the state machine
 *	\param St	state
 *	\param Msg	incoming message
 *	\param f	the function to be executed when (state, message) combination occurs at the state machine
 *	\pre *S should be a legal pointer to the state machine, st, msg, should be all within the range, Base should be set in the initial state
 *	\post

 IRQL = PASSIVE_LEVEL

 */
VOID StateMachineSetAction(
	IN STATE_MACHINE *S,
	IN ULONG St,
	IN ULONG Msg,
	IN STATE_MACHINE_FUNC Func)
{
	ULONG MsgIdx;

	MsgIdx = Msg - S->Base;

	if (St < S->NrState && MsgIdx < S->NrMsg)
	{
		/* boundary checking before setting the action*/
		S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
	}
}


/*! \brief	 This function does the state transition
 *	\param	 *Adapter the NIC adapter pointer
 *	\param	 *S 	  the state machine
 *	\param	 *Elem	  the message to be executed
 *	\return   None
 */
VOID StateMachinePerformAction(
	IN	PRTMP_ADAPTER	pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem,
	IN ULONG CurrState)
{
	if (S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base])
		(*(S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base]))(pAd, Elem);
}


/*
	==========================================================================
	Description:
		The drop function, when machine executes this, the message is simply
		ignored. This function does nothing, the message is freed in
		StateMachinePerformAction()
	==========================================================================
 */
VOID Drop(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
}


/*
	==========================================================================
	Description:
	==========================================================================
 */
UCHAR RandomByte(RTMP_ADAPTER *pAd)
{
	ULONG i;
	UCHAR R, Result;

	R = 0;

	if (pAd->Mlme.ShiftReg == 0)
	NdisGetSystemUpTime((ULONG *)&pAd->Mlme.ShiftReg);

	for (i = 0; i < 8; i++)
	{
		if (pAd->Mlme.ShiftReg & 0x00000001)
		{
			pAd->Mlme.ShiftReg = ((pAd->Mlme.ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		}
		else
		{
			pAd->Mlme.ShiftReg = pAd->Mlme.ShiftReg >> 1;
			Result = 0;
		}
		R = (R << 1) | Result;
	}

	return R;
}


UCHAR RandomByte2(RTMP_ADAPTER *pAd)
{
	UINT32 a,b;
	UCHAR value, seed = 0;

	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
					__FUNCTION__, __LINE__));
		return 0;
	}

	/*MAC statistic related*/
	a = AsicGetCCACnt(pAd);
	a &= 0x0000ffff;
	b = AsicGetCrcErrCnt(pAd);
	b &= 0x0000ffff;
	value = (a<<16)|b;

	/*get seed by RSSI or SNR related info */
	seed = get_random_seed_by_phy(pAd);

	return value ^ seed ^ RandomByte(pAd);
}


/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID RTMPCheckRates(RTMP_ADAPTER *pAd, UCHAR SupRate[], UCHAR *SupRateLen, UCHAR PhyMode)
{
	UCHAR	RateIdx, i, j;
	UCHAR	NewRate[12], NewRateLen;

	NdisZeroMemory(NewRate, sizeof(NewRate));
	NewRateLen = 0;

	if (WMODE_EQUAL(PhyMode, WMODE_B))
		RateIdx = 4;
	else
		RateIdx = 12;

	/* Check for support rates exclude basic rate bit */
	for (i = 0; i < *SupRateLen; i++)
		for (j = 0; j < RateIdx; j++)
			if ((SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
				NewRate[NewRateLen++] = SupRate[i];

	*SupRateLen = NewRateLen;
	NdisMoveMemory(SupRate, NewRate, NewRateLen);
}



CHAR RTMPAvgRssi(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RSSI_SAMPLE *pRssi)
{
	CHAR Rssi;
	INT Rssi_temp;
	UINT32	rx_stream;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			rx_stream = pAd->dbdc_2G_rx_stream;
		else
			rx_stream = pAd->dbdc_5G_rx_stream;
	} else {
		rx_stream = pAd->Antenna.field.RxPath;
	}

	if (rx_stream == 4)
	{
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1] + pRssi->AvgRssi[2] + pRssi->AvgRssi[3]) >> 2;
	}
	else if (rx_stream == 3)
	{
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1] + pRssi->AvgRssi[2])/3;
	}
	else if (rx_stream == 2)
	{
		Rssi_temp = (pRssi->AvgRssi[0] + pRssi->AvgRssi[1])>>1;
	}
	else
	{
		Rssi_temp = pRssi->AvgRssi[0];
	}
	Rssi = (CHAR)Rssi_temp;
	return Rssi;
}


CHAR RTMPMaxRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2)
{
	CHAR	larger = -127;

	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 <= 0))
	{
		larger = Rssi0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 <= 0))
	{
		larger = max(Rssi0, Rssi1);
	}

	if ((pAd->Antenna.field.RxPath >= 3) && (Rssi2 <= 0))
	{
		larger = max(larger, Rssi2);
	}

	if (larger == -127)
		larger = 0;

	return larger;
}

CHAR RTMPMinRssi(RTMP_ADAPTER *pAd, CHAR Rssi0, CHAR Rssi1, CHAR Rssi2, CHAR Rssi3)
{
	CHAR	smaller = -127;

	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 != 0))
	{
		smaller = Rssi0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 != 0))
	{
		smaller = min(Rssi0, Rssi1);
	}
	
	if ((pAd->Antenna.field.RxPath >= 3) && (Rssi2 != 0))
	{
		smaller = min(smaller, Rssi2);
	}

	if ((pAd->Antenna.field.RxPath == 4) && (Rssi3 != 0))
	{
		smaller = min(smaller, Rssi3);
	}
        
	if (smaller == -127)
		smaller = 0;

	return smaller;
}

CHAR RTMPMinSnr(RTMP_ADAPTER *pAd, CHAR Snr0, CHAR Snr1)
{
	CHAR	smaller = Snr0;

	if (pAd->Antenna.field.RxPath == 1)
	{
		smaller = Snr0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Snr1 != 0))
	{
		smaller = min(Snr0, Snr1);
	}

	return smaller;
}


/*
    ========================================================================
    Routine Description:
        Periodic evaluate antenna link status

    Arguments:
        pAd         - Adapter pointer

    Return Value:
        None

    ========================================================================
*/
VOID AsicEvaluateRxAnt(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */


	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS |
							fRTMP_ADAPTER_NIC_NOT_EXIST |
							fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)
		|| IsHcAllSupportedBandsRadioOff(pAd)
#ifdef ANT_DIVERSITY_SUPPORT
		|| (pAd->EepromAccess)
#endif /* ANT_DIVERSITY_SUPPORT */
	)
		return;


#ifdef MT_MAC
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT)
		return;
#endif /* MT_MAC */

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
			/* for SmartBit 64-byte stream test */
			if (pAd->MacTab.Size > 0)
				APAsicEvaluateRxAnt(pAd);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}


/*
    ========================================================================
    Routine Description:
        After evaluation, check antenna link status

    Arguments:
        pAd         - Adapter pointer

    Return Value:
        None

    ========================================================================
*/
VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)FunctionContext;


#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS |
							fRTMP_ADAPTER_NIC_NOT_EXIST)
		|| IsHcAllSupportedBandsRadioOff(pAd)
#ifdef ANT_DIVERSITY_SUPPORT
		|| (pAd->EepromAccess)
#endif /* ANT_DIVERSITY_SUPPORT */
	)
		return;

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
				APAsicRxAntEvalTimeout(pAd);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}


VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
}


/*
    ========================================================================
    Routine Description:
        check if this entry need to switch rate automatically

    Arguments:
        pAd
        pEntry

    Return Value:
        TURE
        FALSE

    ========================================================================
*/
BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry)
{
	BOOLEAN result = TRUE;

	if ((!pEntry) || (!(pEntry->wdev))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): entry(%p) or wdev(%p) is NULL!\n",
					__FUNCTION__, pEntry,
					 pEntry ? pEntry->wdev : NULL));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		result = pEntry->wdev->bAutoTxRateSwitch;
	}
#endif /* CONFIG_AP_SUPPORT */




#if defined(TXBF_SUPPORT) && (!defined(MT_MAC))
		if (result == FALSE)
		{
			//Force MCS will be fixed
			if (pAd->chipCap.FlgHwTxBfCap)
			{
				eTxBFProbing(pAd, pEntry);
			}
		}
#endif /* TXBF_SUPPORT */


	return result;
}


BOOLEAN RTMPAutoRateSwitchCheck(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		INT	apidx = 0;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
			if (pAd->ApCfg.MBSSID[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}
#ifdef WDS_SUPPORT
		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++)
		{
			if (pAd->WdsTab.WdsEntry[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++)
		{
			if (pAd->ApCfg.ApCliTab[apidx].wdev.bAutoTxRateSwitch)
				return TRUE;
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	return FALSE;
}


/*
    ========================================================================
    Routine Description:
        check if this entry need to fix tx legacy rate

    Arguments:
        pAd
        pEntry

    Return Value:
        TURE
        FALSE

    ========================================================================
*/
UCHAR RTMPStaFixedTxMode(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UCHAR tx_mode = FIXED_TXMODE_HT;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	if (pEntry)
	{
		if (IS_ENTRY_CLIENT(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;
#ifdef WDS_SUPPORT
		else if (IS_ENTRY_WDS(pEntry))
			tx_mode = (UCHAR)pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		else if (IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.DesiredTransmitSetting.field.FixedTxMode;
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


	return tx_mode;
}


/*
    ========================================================================
    Routine Description:
        Overwrite HT Tx Mode by Fixed Legency Tx Mode, if specified.

    Arguments:
        pAd
        pEntry

    Return Value:
        TURE
        FALSE

    ========================================================================
*/
VOID RTMPUpdateLegacyTxSetting(UCHAR fixed_tx_mode, MAC_TABLE_ENTRY *pEntry)
{
	HTTRANSMIT_SETTING TransmitSetting;

	if ((fixed_tx_mode != FIXED_TXMODE_CCK) &&
		(fixed_tx_mode != FIXED_TXMODE_OFDM)
#ifdef DOT11_VHT_AC
		&& (fixed_tx_mode != FIXED_TXMODE_VHT)
#endif /* DOT11_VHT_AC */
	)
		return;

	TransmitSetting.word = 0;

	TransmitSetting.field.MODE = pEntry->HTPhyMode.field.MODE;
	TransmitSetting.field.MCS = pEntry->HTPhyMode.field.MCS;

#ifdef DOT11_VHT_AC
	if (fixed_tx_mode == FIXED_TXMODE_VHT)
	{
		UCHAR nss, mcs;
		TransmitSetting.field.MODE = MODE_VHT;
		TransmitSetting.field.BW = pEntry->MaxHTPhyMode.field.BW;
		nss = (TransmitSetting.field.MCS >> 4) & 0x3;
		mcs = (TransmitSetting.field.MCS & 0xf);
		if (nss > 3)
			nss = 3;

		if ((TransmitSetting.field.BW == BW_20) && (mcs > MCS_8))
			mcs = MCS_8;

		if ((TransmitSetting.field.BW == BW_40) && (mcs > MCS_9))
			mcs = MCS_9;

		if (TransmitSetting.field.BW == BW_80)
		{
			if (mcs > MCS_9 )
				mcs = MCS_9;
			if ((nss == 2) && (mcs == MCS_6))
				mcs = MCS_5;
		}

		TransmitSetting.field.MCS = ((nss << 4) + mcs);
	}
	else
#endif /* DOT11_VHT_AC */
	if (fixed_tx_mode == FIXED_TXMODE_CCK)
	{
		TransmitSetting.field.MODE = MODE_CCK;
		/* CCK mode allow MCS 0~3*/
		if (TransmitSetting.field.MCS > MCS_3)
			TransmitSetting.field.MCS = MCS_3;
	}
	else
	{
		TransmitSetting.field.MODE = MODE_OFDM;
		/* OFDM mode allow MCS 0~7*/
		if (TransmitSetting.field.MCS > MCS_7)
			TransmitSetting.field.MCS = MCS_7;
	}

	if (pEntry->HTPhyMode.field.MODE >= TransmitSetting.field.MODE)
	{
		pEntry->HTPhyMode.word = TransmitSetting.word;
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPUpdateLegacyTxSetting : wcid-%d, MODE=%s, MCS=%d \n",
				pEntry->wcid, get_phymode_str(pEntry->HTPhyMode.field.MODE), pEntry->HTPhyMode.field.MCS));
	}
	else
	{
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : the fixed TxMode is invalid \n", __FUNCTION__));
	}
}



VOID RTMPSetAGCInitValue(RTMP_ADAPTER *pAd, UCHAR BandWidth)
{
	if (pAd->chipOps.ChipAGCInit != NULL)
		pAd->chipOps.ChipAGCInit(pAd, BandWidth);
}


/*
========================================================================
Routine Description:
	Check if the channel has the property.

Arguments:
	pAd				- WLAN control block pointer
	ChanNum			- channel number
	Property		- channel property, CHANNEL_PASSIVE_SCAN, etc.

Return Value:
	TRUE			- YES
	FALSE			- NO

Note:
========================================================================
*/
BOOLEAN CHAN_PropertyCheck(RTMP_ADAPTER *pAd, UINT32 ChanNum, UCHAR Property)
{
	UINT32 IdChan;

	/* look for all registered channels */
	for(IdChan=0; IdChan<pAd->ChannelListNum; IdChan++)
	{
		if (pAd->ChannelList[IdChan].Channel == ChanNum)
		{
			if ((pAd->ChannelList[IdChan].Flags & Property) == Property)
				return TRUE;

			break;
		}
	}

	return FALSE;
}

NDIS_STATUS RtmpEnqueueTokenFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN UCHAR OldUP)
{
	NDIS_STATUS NState = NDIS_STATUS_FAILURE;
	HEADER_802_11 *pNullFr;
	UCHAR *pFrame;
	UINT frm_len;

	NState = MlmeAllocateMemory(pAd, (UCHAR **)&pFrame);
	if (NState == NDIS_STATUS_SUCCESS)
	{
                UCHAR *qos_p;
                pNullFr = (PHEADER_802_11) pFrame;
		frm_len = sizeof(HEADER_802_11);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr,
							pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.ToDs = 0;
			pNullFr->FC.FrDs = 1;
			pNullFr->Frag = 0x0f;
		}
#endif /* CONFIG_AP_SUPPORT */

		pNullFr->FC.Type = FC_TYPE_DATA;
                qos_p = ((UCHAR *)pNullFr) + frm_len;
			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = OldUP;
		   qos_p[1] = PS_RETRIEVE_TOKEN;
			frm_len += 2;

		/* since TxRate may change, we have to change Duration each time */
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, frm_len);

		NState = MiniportMMRequest(pAd, OldUP | MGMT_USE_QUEUE_FLAG, (PUCHAR)pNullFr, frm_len);
		MlmeFreeMemory( pFrame);
	}

   return NState;
}

