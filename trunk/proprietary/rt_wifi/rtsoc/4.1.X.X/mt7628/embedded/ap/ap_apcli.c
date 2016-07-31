/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_apcli.c

    Abstract:
    Support AP-Client function.

    Note:
    1. Call RT28xx_ApCli_Init() in init function and
       call RT28xx_ApCli_Remove() in close function

    2. MAC of ApCli-interface is initialized in RT28xx_ApCli_Init()

    3. ApCli index (0) of different rx packet is got in

    4. ApCli index (0) of different tx packet is assigned in

    5. ApCli index (0) of different interface is got in APHardTransmit() by using

    6. ApCli index (0) of IOCTL command is put in pAd->OS_Cookie->ioctl_if

    8. The number of ApCli only can be 1

	9. apcli convert engine subroutines, we should just take care data packet.
    Revision History:
    Who             When            What
    --------------  ----------      ----------------------------------------------
    Shiang, Fonchi  02-13-2007      created
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

BOOLEAN ApCliWaitProbRsp(PRTMP_ADAPTER pAd, USHORT ifIndex)
{
        if (ifIndex >= MAX_APCLI_NUM)
                return FALSE;

        return (pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState == APCLI_JOIN_WAIT_PROBE_RSP) ?
                TRUE : FALSE;
}

VOID ApCliSimulateRecvBeacon(RTMP_ADAPTER *pAd)
{
	INT loop;
        ULONG Now32, BPtoJiffies;
        PAPCLI_STRUCT pApCliEntry = NULL;
	LONG timeDiff;
	
	NdisGetSystemUpTime(&Now32);
        for (loop = 0; loop < MAX_APCLI_NUM; loop++)
        {
        	pApCliEntry = &pAd->ApCfg.ApCliTab[loop];
                if ((pApCliEntry->Valid == TRUE) && (pApCliEntry->MacTabWCID < MAX_LEN_OF_MAC_TABLE))
                {
                	/*
                          When we are connected and do the scan progress, it's very possible we cannot receive
                          the beacon of the AP. So, here we simulate that we received the beacon.
                         */
                        if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
                            (RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApCliRcvBeaconTime + (1 * OS_HZ))))
                        {
                        	BPtoJiffies = (((pApCliEntry->ApCliBeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);
                                timeDiff = (pAd->Mlme.Now32 - pApCliEntry->ApCliRcvBeaconTime) / BPtoJiffies;
                                if (timeDiff > 0)
                                	pApCliEntry->ApCliRcvBeaconTime += (timeDiff * BPtoJiffies);

                                if (RTMP_TIME_AFTER(pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32))
                                {
                                	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MMCHK - APCli BeaconRxTime adjust wrong(BeaconRx=0x%lx, Now=0x%lx)\n",
                                                                   pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32));
                                }
                        }

                        /* update channel quality for Roaming and UI LinkQuality display */
                        MlmeCalculateChannelQuality(pAd, &pAd->MacTab.Content[pApCliEntry->MacTabWCID], Now32);
		}
	}
}

BOOLEAN ApcliCompareAuthEncryp(
	IN PAPCLI_STRUCT pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthModeAux,
	IN NDIS_802_11_WEP_STATUS			WEPstatus,
	IN CIPHER_SUITE WPA);


/*
========================================================================
Routine Description:
    Close ApCli network interface.

Arguments:
    ad_p            points to our adapter

Return Value:
    None

Note:
========================================================================
*/
VOID RT28xx_ApCli_Close(RTMP_ADAPTER *ad_p)
{
	UINT index;


	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (ad_p->ApCfg.ApCliTab[index].wdev.if_dev)
			RtmpOSNetDevClose(ad_p->ApCfg.ApCliTab[index].wdev.if_dev);
	}

}


/* --------------------------------- Private -------------------------------- */
INT ApCliIfLookUp(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	SHORT if_idx;

	for(if_idx = 0; if_idx < MAX_APCLI_NUM; if_idx++)
	{
		if(MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[if_idx].wdev.if_addr, pAddr))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ApCliIfIndex=%d\n",
						__FUNCTION__, if_idx));
			return if_idx;
		}
	}

	return -1;
}


BOOLEAN isValidApCliIf(SHORT if_idx)
{
	return (((if_idx >= 0) && (if_idx < MAX_APCLI_NUM)) ? TRUE : FALSE);
}


/*! \brief init the management mac frame header
 *  \param p_hdr mac header
 *  \param subtype subtype of the frame
 *  \param p_ds destination address, don't care if it is a broadcast address
 *  \return none
 *  \pre the station has the following information in the pAd->UserCfg
 *   - bssid
 *   - station address
 *  \post
 *  \note this function initializes the following field
 */
VOID ApCliMgtMacHeaderInit(
    IN RTMP_ADAPTER *pAd,
    INOUT HEADER_802_11 *pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN UCHAR *pDA,
    IN UCHAR *pBssid,
    IN USHORT ifIndex)
{
    NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
    pHdr80211->FC.Type = FC_TYPE_MGMT;
    pHdr80211->FC.SubType = SubType;
    pHdr80211->FC.ToDs = ToDs;
    COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
    COPY_MAC_ADDR(pHdr80211->Addr2, pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
    COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}


#ifdef DOT11_N_SUPPORT
/*
	========================================================================

	Routine Description:
		Verify the support rate for HT phy type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		FALSE if pAd->CommonCfg.SupportedHtPhy doesn't accept the pHtCapability.  (AP Mode)

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
BOOLEAN ApCliCheckHt(
	IN RTMP_ADAPTER *pAd,
	IN USHORT IfIndex,
	INOUT HT_CAPABILITY_IE *pHtCapability,
	INOUT ADD_HT_INFO_IE *pAddHtInfo)
{
	APCLI_STRUCT *pApCliEntry = NULL;
	HT_CAPABILITY_IE *aux_ht_cap;
	RT_HT_CAPABILITY *rt_ht_cap = &pAd->CommonCfg.DesiredHtPhy;


	if (IfIndex >= MAX_APCLI_NUM)
		return FALSE;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];

	aux_ht_cap = &pApCliEntry->MlmeAux.HtCapability;
	aux_ht_cap->MCSSet[0] = 0xff;
	aux_ht_cap->MCSSet[4] = 0x1;
	 switch (pAd->CommonCfg.RxStream)
	{
		case 1:
			aux_ht_cap->MCSSet[0] = 0xff;
			aux_ht_cap->MCSSet[1] = 0x00;
			aux_ht_cap->MCSSet[2] = 0x00;
			aux_ht_cap->MCSSet[3] = 0x00;
			break;
		case 2:
			aux_ht_cap->MCSSet[0] = 0xff;
			aux_ht_cap->MCSSet[1] = 0xff;
            aux_ht_cap->MCSSet[2] = 0x00;
            aux_ht_cap->MCSSet[3] = 0x00;
			break;
		case 3:
			aux_ht_cap->MCSSet[0] = 0xff;
			aux_ht_cap->MCSSet[1] = 0xff;
            aux_ht_cap->MCSSet[2] = 0xff;
            aux_ht_cap->MCSSet[3] = 0x00;
			break;
	}

	/* Record the RxMcs of AP */
	NdisMoveMemory(pApCliEntry->RxMcsSet, pHtCapability->MCSSet, 16);

	/* choose smaller setting */
#ifdef CONFIG_MULTI_CHANNEL
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth;
#else /* CONFIG_MULTI_CHANNEL */
#ifdef APCLI_AUTO_BW_SUPPORT
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pApCliEntry->wdev.bw;
#else
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & rt_ht_cap->ChannelWidth;
#endif /* APCLI_AUTO_BW_SUPPORT */
#endif /* !CONFIG_MULTI_CHANNEL */

	aux_ht_cap->HtCapInfo.GF =  pHtCapability->HtCapInfo.GF & rt_ht_cap->GF;

#if	defined(CONFIG_MULTI_CHANNEL) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
	if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
	{
		pApCliEntry->wdev.bw = aux_ht_cap->HtCapInfo.ChannelWidth;
		if (pApCliEntry->wdev.bw == HT_BW_20)
		{
			pApCliEntry->wdev.channel = pAddHtInfo->ControlChan;
			pApCliEntry->wdev.CentralChannel = pApCliEntry->wdev.channel;
			pApCliEntry->wdev.extcha = EXTCHA_NONE;
		}
		else if (pApCliEntry->wdev.bw == HT_BW_40)
		{
			pApCliEntry->wdev.channel = pAddHtInfo->ControlChan;
		
			if (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE )
			{
				pApCliEntry->wdev.extcha = EXTCHA_ABOVE;
				pApCliEntry->wdev.CentralChannel = pApCliEntry->wdev.channel + 2;
			}
			else if (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_BELOW)
			{
				pApCliEntry->wdev.extcha = EXTCHA_BELOW;
				pApCliEntry->wdev.CentralChannel = pApCliEntry->wdev.channel - 2;
			}
			else /* EXTCHA_NONE , should not be here!*/
			{
				pApCliEntry->wdev.extcha = EXTCHA_NONE;
				pApCliEntry->wdev.CentralChannel = pApCliEntry->wdev.channel; 
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliCheckHt :  channel=%u,  CentralChannel=%u, bw=%u\n", pApCliEntry->wdev.channel, pApCliEntry->wdev.CentralChannel, pApCliEntry->wdev.bw));
		
	}				
#endif /* CONFIG_MULTI_CHANNEL */


	/* Send Assoc Req with my HT capability. */
	aux_ht_cap->HtCapInfo.AMsduSize =  rt_ht_cap->AmsduSize;
	aux_ht_cap->HtCapInfo.MimoPs = pHtCapability->HtCapInfo.MimoPs;
	aux_ht_cap->HtCapInfo.ShortGIfor20 =  (rt_ht_cap->ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	aux_ht_cap->HtCapInfo.ShortGIfor40 =  (rt_ht_cap->ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	aux_ht_cap->HtCapInfo.TxSTBC =  (rt_ht_cap->TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	aux_ht_cap->HtCapInfo.RxSTBC =  (rt_ht_cap->RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);

	/* Fix throughput issue for some vendor AP with AES mode */
	if (pAddHtInfo->AddHtInfo.RecomWidth & rt_ht_cap->ChannelWidth)
		aux_ht_cap->HtCapInfo.CCKmodein40 = pHtCapability->HtCapInfo.CCKmodein40;
	else 
		aux_ht_cap->HtCapInfo.CCKmodein40 = 0;
	
	aux_ht_cap->HtCapParm.MaxRAmpduFactor = rt_ht_cap->MaxRAmpduFactor;
	aux_ht_cap->HtCapParm.MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	aux_ht_cap->ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	if (pAd->CommonCfg.bRdg)
	{
		aux_ht_cap->ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
	}

	if (pAd->CommonCfg.Channel < 14) {
		pApCliEntry->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth = pAddHtInfo->AddHtInfo.RecomWidth;
		pApCliEntry->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset = pAddHtInfo->AddHtInfo.ExtChanOffset;
		pApCliEntry->MlmeAux.AddHtInfo.ControlChan = pAddHtInfo->ControlChan;
	}

	/*COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCapability); */
	return TRUE;
}
#endif /* DOT11_N_SUPPORT */


/*
    ==========================================================================

	Routine	Description:
		Connected to the BSSID

	Arguments:
		pAd				- Pointer to our adapter
		ApCliIdx		- Which ApCli interface
	Return Value:
		FALSE: fail to alloc Mac entry.

	Note:

	==========================================================================
*/
BOOLEAN ApCliLinkUp(RTMP_ADAPTER *pAd, UCHAR ifIndex)
{
	BOOLEAN result = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
#if defined(MAC_REPEATER_SUPPORT) || defined(MT_MAC)
	UCHAR CliIdx = 0xFF;
#ifdef MAC_REPEATER_SUPPORT
	INVAILD_TRIGGER_MAC_ENTRY *pSkipEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* defined(MAC_REPEATER_SUPPORT) || defined(MT_MAC) */

	do
	{
		if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			|| (ifIndex >= 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn)
			{
				if (ifIndex < MAX_APCLI_NUM)
				{
#ifdef LINUX
					struct net_device *pNetDev;
					struct net *net= &init_net;

					for_each_netdev(net, pNetDev)
					{
						if (pNetDev->priv_flags == IFF_EBRIDGE)
						{
							COPY_MAC_ADDR(pAd->ApCfg.BridgeAddress, pNetDev->dev_addr);
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Bridge Addr = %02X:%02X:%02X:%02X:%02X:%02X. !!!\n",
										PRINT_MAC(pAd->ApCfg.BridgeAddress)));

						}
						pSkipEntry = RepeaterInvaildMacLookup(pAd, pNetDev->dev_addr);

						if (pSkipEntry == NULL)
						RTMPRepeaterInsertInvaildMacEntry(pAd, pNetDev->dev_addr);
					}

					if (!MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, ZERO_MAC_ADDR))
					{
						pSkipEntry = RepeaterInvaildMacLookup(pAd, pAd->ApCfg.BridgeAddress);

						if (pSkipEntry)
						{
							UCHAR MacAddr[MAC_ADDR_LEN];
							UCHAR entry_idx;

							COPY_MAC_ADDR(MacAddr, pSkipEntry->MacAddr);
							entry_idx = pSkipEntry->entry_idx;
							RTMPRepeaterRemoveInvaildMacEntry(pAd, entry_idx, MacAddr);
						}
					}					
#endif
				}

				if (ifIndex >= 64)
				{
					CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
					ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);

					pMacEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].OriginalAddress);
					if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
						pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli = FALSE;
					else
						pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli = TRUE;

					pMacEntry = NULL;
				}
			}
#endif /* MAC_REPEATER_SUPPORT */

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! APCLI LINK UP - IF(apcli%d) AuthMode(%d)=%s, WepStatus(%d)=%s !!!\n",
										ifIndex,
										pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode, GetAuthMode(pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode),
										pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus, GetEncryptType(pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus)));
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!! ERROR : APCLI LINK UP - IF(apcli%d)!!!\n", ifIndex));
			result = FALSE;
			break;
		}

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) ifIndex = %d, CliIdx = %d !!!\n", __FUNCTION__, ifIndex, CliIdx));
#endif /* MAC_REPEATER_SUPPORT */

		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifndef APCLI_CONNECTION_TRIAL
		if ((pApCliEntry->Valid)
#ifdef MAC_REPEATER_SUPPORT
			 && (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!! ERROR : This link had existed - IF(apcli%d)!!!\n", ifIndex));
			result = FALSE;
			break;
		}
#endif /* APCLI_CONNECTION_TRIAL */

		wdev = &pApCliEntry->wdev;
		printk("!!! APCLI LINK UP - IF(apcli%d) AuthMode(%d)=%s, WepStatus(%d)=%s!\n",
					ifIndex,
					wdev->AuthMode, GetAuthMode(wdev->AuthMode),
					wdev->WepStatus, GetEncryptType(wdev->WepStatus));

#if defined (CONFIG_WIFI_PKT_FWD)
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		{
			if (wf_fwd_get_rep_hook)
				wf_fwd_get_rep_hook(pAd->CommonCfg.EtherTrafficBand);

			if (wf_fwd_entry_insert_hook)
				wf_fwd_entry_insert_hook (wdev->if_dev, pAd->net_dev, pAd);

			if (wf_fwd_insert_repeater_mapping_hook)
				wf_fwd_insert_repeater_mapping_hook (pAd, &pAd->ApCfg.ReptCliEntryLock, &pAd->ApCfg.ReptCliHash[0], &pAd->ApCfg.ReptMapHash[0], &pAd->ApCfg.ApCliTab[0].wdev.if_addr);
		}

#endif /* CONFIG_WIFI_PKT_FWD */

		/* Insert the Remote AP to our MacTable. */
		/*pMacEntry = MacTableInsertApCliEntry(pAd, (PUCHAR)(pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid)); */
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn && (pAd->chipCap.hif_type == HIF_RLT || pAd->chipCap.hif_type == HIF_RTMP))
			pMacEntry = RTMPInsertRepeaterMacEntry(
							pAd,
							(PUCHAR)(pApCliEntry->MlmeAux.Bssid),
							wdev,
							(ifIndex + MIN_NET_DEVICE_FOR_APCLI),
							CliIdx,
							TRUE);
		else
#endif /* MAC_REPEATER_SUPPORT */
			pMacEntry = MacTableInsertEntry(pAd, (PUCHAR)(pApCliEntry->MlmeAux.Bssid),
										wdev, ENTRY_APCLI,
										OPMODE_AP, TRUE);

#ifdef MT_MAC
		if (CliIdx == 0xff && pAd->chipCap.hif_type == HIF_MT)
		{
#ifdef MULTI_APCLI_SUPPORT
			AsicUpdateRxWCIDTable(pAd, APCLI_MCAST_WCID(ifIndex), (PUCHAR)(pApCliEntry->MlmeAux.Bssid));
#else /* MULTI_APCLI_SUPPORT */
			AsicUpdateRxWCIDTable(pAd, APCLI_MCAST_WCID, (PUCHAR)(pApCliEntry->MlmeAux.Bssid));
#endif /*! MULTI_APCLI_SUPPORT */
		}
#endif /* MT_MAC */

		if (pMacEntry)
		{
			UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES];
			PUCHAR pRates = Rates;
			UCHAR RatesLen;
			UCHAR MaxSupportedRate = 0;

			tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
			pMacEntry->Sst = SST_ASSOC;
			pMacEntry->wdev = &pApCliEntry->wdev;
#ifdef MULTI_APCLI_SUPPORT
			pMacEntry->wdev ->tr_tb_idx = APCLI_MCAST_WCID(ifIndex);
			tr_tb_set_mcast_entry(pAd, APCLI_MCAST_WCID(ifIndex), pMacEntry->wdev);
#endif /* MULTI_APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID = pMacEntry->Aid;
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = TRUE;
				pMacEntry->bReptCli = TRUE;
				pMacEntry->MatchReptCliIdx = CliIdx;
				pMacEntry->ReptCliIdleCount = 0;
				COPY_MAC_ADDR(pMacEntry->ReptCliAddr, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
				pMacEntry->bReptEthBridgeCli = FALSE;
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli == TRUE) {
					pMacEntry->bReptEthCli = TRUE;
					if (MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].OriginalAddress)) {
						pMacEntry->bReptEthBridgeCli = TRUE;
					}
				}
				else
					pMacEntry->bReptEthCli = FALSE;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pApCliEntry->Valid = TRUE;
				pApCliEntry->MacTabWCID = pMacEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT
				pMacEntry->bReptCli = FALSE;
#endif /* MAC_REPEATER_SUPPORT */
				COPY_MAC_ADDR(&wdev->bssid[0], &pApCliEntry->MlmeAux.Bssid[0]);
				COPY_MAC_ADDR(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID), pApCliEntry->MlmeAux.Bssid);
				pApCliEntry->SsidLen = pApCliEntry->MlmeAux.SsidLen;
				NdisMoveMemory(pApCliEntry->Ssid, pApCliEntry->MlmeAux.Ssid, pApCliEntry->SsidLen);
			}

#ifdef WPA_SUPPLICANT_SUPPORT
		/*
			If ApCli connects to different AP, ApCli couldn't send EAPOL_Start for WpaSupplicant.
		*/
		if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) &&
			(NdisEqualMemory(pAd->MlmeAux.Bssid, pApCliEntry->LastBssid, MAC_ADDR_LEN) == FALSE) &&
			(pApCliEntry->wpa_supplicant_info.bLostAp == TRUE))
		{
			pApCliEntry->wpa_supplicant_info.bLostAp = FALSE;
		}

		COPY_MAC_ADDR(pApCliEntry->LastBssid, pAd->MlmeAux.Bssid);
#endif /* WPA_SUPPLICANT_SUPPORT */

			if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
			{

#ifdef WPA_SUPPLICANT_SUPPORT
				if (pApCliEntry->wpa_supplicant_info.WpaSupplicantUP &&
					(pMacEntry->WepStatus == Ndis802_11WEPEnabled) &&
					(wdev->IEEE8021X == TRUE))
					tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
				else
#endif /*WPA_SUPPLICANT_SUPPORT*/
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef MAC_REPEATER_SUPPORT
				if (CliIdx != 0xFF)
					pApCliEntry->RepeaterCli[CliIdx].CliConnectState = 2;
#endif /* MAC_REPEATER_SUPPORT */
			}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE) &&
#ifdef MAC_REPEATER_SUPPORT
				(CliIdx == 0xFF) &&
#endif /* MAC_REPEATER_SUPPORT */
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli auto connected: ApCliLinkUp()\n"));
				pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
			}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				NdisGetSystemUpTime(&pApCliEntry->ApCliLinkUpTime);

			/*
				Store appropriate RSN_IE for WPA SM negotiation later
				If WPAPSK/WPA2SPK mix mode, driver just stores either WPAPSK or
				WPA2PSK RSNIE. It depends on the AP-Client's authentication mode
				to store the corresponding RSNIE.
			*/
			if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA) && (pApCliEntry->MlmeAux.VarIELen != 0))
			{
				PUCHAR pVIE;
				UCHAR len;
				PEID_STRUCT pEid;

				pVIE = pApCliEntry->MlmeAux.VarIEs;
				len	 = pApCliEntry->MlmeAux.VarIELen;

				while (len > 0)
				{
					pEid = (PEID_STRUCT) pVIE;
					/* For WPA/WPAPSK */
					if ((pEid->Eid == IE_WPA) && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA || pMacEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
					{
						NdisMoveMemory(pMacEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pMacEntry->RSNIE_Len = (pEid->Len + 2);
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkUp: Store RSN_IE for WPA SM negotiation \n"));
					}
					/* For WPA2/WPA2PSK */
					else if ((pEid->Eid == IE_RSN) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA2 || pMacEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
					{
						NdisMoveMemory(pMacEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pMacEntry->RSNIE_Len = (pEid->Len + 2);
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkUp: Store RSN_IE for WPA2 SM negotiation \n"));
					}

					pVIE += (pEid->Len + 2);
					len  -= (pEid->Len + 2);
				}
			}

			if (pMacEntry->RSNIE_Len == 0)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkUp: root-AP has no RSN_IE \n"));
			}
			else
			{
				hex_dump("The RSN_IE of root-AP", pMacEntry->RSN_IE, pMacEntry->RSNIE_Len);
			}

			SupportRate(pApCliEntry->MlmeAux.SupRate, pApCliEntry->MlmeAux.SupRateLen, pApCliEntry->MlmeAux.ExtRate,
				pApCliEntry->MlmeAux.ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);

			pMacEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxTxRate, MaxSupportedRate);
			pMacEntry->RateLen = RatesLen;
			set_entry_phy_cfg(pAd, pMacEntry);

			pMacEntry->CapabilityInfo = pApCliEntry->MlmeAux.CapabilityInfo;

			pApCliEntry->ApCliBeaconPeriod = pApCliEntry->MlmeAux.BeaconPeriod;
			
			if ((wdev->WepStatus == Ndis802_11WEPEnabled)
#ifdef WPA_SUPPLICANT_SUPPORT
				&& (pApCliEntry->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* WPA_SUPPLICANT_SUPPORT */
			)
			{
				CIPHER_KEY *pKey;
				INT idx, BssIdx;

				BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
#ifdef MAC_APCLI_SUPPORT
				BssIdx = APCLI_BSS_BASE + ifIndex;
#endif /* MAC_APCLI_SUPPORT */
				for (idx=0; idx < SHARE_KEY_NUM; idx++)
				{
					pKey = &pApCliEntry->SharedKey[idx];
					if (pKey->KeyLen > 0)
					{
						/* Set key material and cipherAlg to Asic */
#ifdef MAC_REPEATER_SUPPORT
						if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
						{
							RTMP_ASIC_SHARED_KEY_TABLE(pAd,
	    									  		BssIdx,
	    									  		idx,
		    										pKey);
						}

						if (idx == wdev->DefaultKeyId)
						{
							INT	cnt;

							/* Generate 3-bytes IV randomly for software encryption using */
							for(cnt = 0; cnt < LEN_WEP_TSC; cnt++)
								pKey->TxTsc[cnt] = RandomByte(pAd);

							RTMP_SET_WCID_SEC_INFO(pAd,
												BssIdx,
												idx,
												pKey->CipherAlg,
												pMacEntry->wcid,
												SHAREDKEYTABLE);

#ifdef MT_MAC
							if (pAd->chipCap.hif_type == HIF_MT)
							{
								CmdProcAddRemoveKey(pAd, 0, pMacEntry->func_tb_idx, idx, pMacEntry->wcid, PAIRWISEKEYTABLE, pKey, pMacEntry->Addr);
#ifdef MULTI_APCLI_SUPPORT
								CmdProcAddRemoveKey(pAd, 0, pMacEntry->func_tb_idx, idx, APCLI_MCAST_WCID(ifIndex), SHAREDKEYTABLE, pKey, BROADCAST_ADDR);
#else /* MULTI_APCLI_SUPPORT */
								CmdProcAddRemoveKey(pAd, 0, pMacEntry->func_tb_idx, idx, APCLI_MCAST_WCID, SHAREDKEYTABLE, pKey, BROADCAST_ADDR);
#endif /* !MULTI_APCLI_SUPPORT */
							}
#endif /* MT_MAC */

						}
					}
				}
			}

#ifdef DOT11_N_SUPPORT
			/* If this Entry supports 802.11n, upgrade to HT rate. */
			if (pApCliEntry->MlmeAux.HtCapabilityLen != 0)
			{
				PHT_CAPABILITY_IE pHtCapability = (PHT_CAPABILITY_IE)&pApCliEntry->MlmeAux.HtCapability;

				ht_mode_adjust(pAd, pMacEntry, pHtCapability, &pAd->CommonCfg.DesiredHtPhy);

				/* find max fixed rate */
				pMacEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0],
																	&pHtCapability->MCSSet[0]);

				if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF-apcli%d : Desired MCS = %d\n",
								ifIndex, wdev->DesiredTransmitSetting.field.MCS));

					set_ht_fixed_mcs(pAd, pMacEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pMacEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
				pMacEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
				pMacEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
				pMacEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
				pMacEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;
				pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
				if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);

				set_sta_ht_cap(pAd, pMacEntry, pHtCapability);

				NdisMoveMemory(&pMacEntry->HTCapability, &pApCliEntry->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
				NdisMoveMemory(pMacEntry->HTCapability.MCSSet, pApCliEntry->RxMcsSet, 16);

#ifdef MT_MAC
                if (pAd->chipCap.hif_type == HIF_MT)
                    AsicUpdateRxWCIDTable(pAd, pMacEntry->wcid, (PUCHAR)(pApCliEntry->MlmeAux.Bssid));
#endif
			}
			else
			{
				pAd->MacTab.fAnyStationIsLegacy = TRUE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkUp - MaxSupRate=%d Mbps\n",
								  RateIdToMbps[pMacEntry->MaxSupportedRate]));
			}
#endif /* DOT11_N_SUPPORT */



			pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
			pMacEntry->CurrTxRate = pMacEntry->MaxSupportedRate;

			RTMPSetSupportMCS(pAd,
							OPMODE_AP,
							pMacEntry,
							pApCliEntry->MlmeAux.SupRate,
							pApCliEntry->MlmeAux.SupRateLen,
							pApCliEntry->MlmeAux.ExtRate,
							pApCliEntry->MlmeAux.ExtRateLen,
							&pApCliEntry->MlmeAux.HtCapability,
							pApCliEntry->MlmeAux.HtCapabilityLen);

			if (wdev->bAutoTxRateSwitch == FALSE)
			{
				pMacEntry->bAutoTxRateSwitch = FALSE;
				/* If the legacy mode is set, overwrite the transmit setting of this entry. */
				RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pMacEntry);
			}
			else
			{
				UCHAR TableSize = 0;

				pMacEntry->bAutoTxRateSwitch = TRUE;

				MlmeSelectTxRateTable(pAd, pMacEntry, &pMacEntry->pTable, &TableSize, &pMacEntry->CurrTxRateIndex);
				MlmeNewTxRate(pAd, pMacEntry);
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (! ADAPT_RATE_TABLE(pMacEntry->pTable))
#endif /* NEW_RATE_ADAPT_SUPPORT */
					pMacEntry->HTPhyMode.field.ShortGI = GI_800;
			}

			/* set this entry WMM capable or not */
			if ((pApCliEntry->MlmeAux.APEdcaParm.bValid)
#ifdef DOT11_N_SUPPORT
				|| IS_HT_STA(pMacEntry)
#endif /* DOT11_N_SUPPORT */
			)
			{
#ifdef APCLI_CERT_SUPPORT
				if (pAd->bApCliCertTest == TRUE)
				{
					AsicSetEdcaParm(pAd, &pApCliEntry->MlmeAux.APEdcaParm);
					pAd->ApCfg.BssEdcaParm.EdcaUpdateCount++;
				}	
#endif /* APCLI_CERT_SUPPORT */				
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}

			set_sta_ra_cap(pAd, pMacEntry, pApCliEntry->MlmeAux.APRalinkIe);

#ifdef PIGGYBACK_SUPPORT
			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE))
			{
				AsicSetPiggyBack(pAd, TRUE);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Turn on Piggy-Back\n"));
			}
#endif /* PIGGYBACK_SUPPORT */

			NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);
			/* set the apcli interface be valid. */
#ifdef MAC_APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			if ((CliIdx == 0xFF)
#ifdef APCLI_CONNECTION_TRIAL
				&& (pApCliEntry->TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
			   )
#endif /* MAC_REPEATER_SUPPORT */
#endif /* MAC_APCLI_SUPPORT */
			{
				pApCliEntry->Valid = TRUE;
				pApCliEntry->wdev.allow_data_tx = TRUE;
				pApCliEntry->wdev.PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef MAC_APCLI_SUPPORT
				AsicSetApCliBssid(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex);
#endif /* MAC_APCLI_SUPPORT */
#ifdef MT_MAC
				if (pAd->chipCap.hif_type == HIF_MT)
				{
#ifdef MULTI_APCLI_SUPPORT
					AsicSetBssid(pAd, pApCliEntry->MlmeAux.Bssid, (0x1 + ifIndex));					
#else /* MULTI_APCLI_SUPPORT */
					AsicSetBssid(pAd, pApCliEntry->MlmeAux.Bssid, 0x1);
#endif /* !MULTI_APCLI_SUPPORT */
				}
#endif
			}

#ifdef MT_MAC
			if (CliIdx != 0xff && pAd->chipCap.hif_type == HIF_MT) {
#ifdef MAC_REPEATER_SUPPORT
				insert_repeater_root_entry(pAd, pMacEntry, (PUCHAR)(pApCliEntry->MlmeAux.Bssid), CliIdx);
#endif /* MAC_REPEATER_SUPPORT */
			}
#endif /* MT_MAC */

			result = TRUE;
			pAd->ApCfg.ApCliInfRunned++;

			break;
		}
		result = FALSE;
	} while(FALSE);


	if (result == FALSE)
	{
	 	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" (%s) alloc mac entry fail!!!\n", __FUNCTION__));
		return result;
	}

#ifdef WPA_SUPPLICANT_SUPPORT
	/*
		When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP,
		WpaSupplicant would not send EapolStart to AP after STA re-connect to AP again.
		In this case, driver would send EapolStart to AP.
	*/
				if ((pMacEntry->AuthMode == Ndis802_11AuthModeWPA2) &&
					(NdisEqualMemory(pAd->MlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].LastBssid, MAC_ADDR_LEN)) &&
					(pAd->ApCfg.ApCliTab[ifIndex].wpa_supplicant_info.bLostAp == TRUE))
				{
					ApcliWpaSendEapolStart(pAd, pAd->MlmeAux.Bssid,pMacEntry,&pAd->ApCfg.ApCliTab[ifIndex]);
					pAd->ApCfg.ApCliTab[ifIndex].wpa_supplicant_info.bLostAp = FALSE;
				}
#endif /*WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_AP_SUPPORT
    /* WSC initial connect to AP, jump to Wsc start action and set the correct parameters */
	if ((result == TRUE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode == WSC_ENROLLEE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
	{
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscState = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscStatus = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
		NdisZeroMemory(pApCliEntry->WscControl.EntryAddr, MAC_ADDR_LEN);
		NdisMoveMemory(pApCliEntry->WscControl.EntryAddr, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
		WscSendEapolStart(pAd, pMacEntry->Addr, AP_MODE);
	}
    else
    {
        WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[ifIndex].WscControl);
    }
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#ifdef APCLI_CERT_SUPPORT
	if (pAd->bApCliCertTest == TRUE)
	{
		if ((pAd->CommonCfg.bBssCoexEnable == TRUE)
		    && (pAd->CommonCfg.Channel <= 14)
		    && (pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable == TRUE)
		    && (pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport == 1)) {
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SCAN_2040);
			BuildEffectedChannelList(pAd);
			/*pAd->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt = 150; */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("LinkUP AP supports 20/40 BSS COEX !!! Dot11BssWidthTriggerScanInt[%d]\n",
				  pAd->CommonCfg.Dot11BssWidthTriggerScanInt));
		} 
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("not supports 20/40 BSS COEX !!! \n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("pAd->CommonCfg.bBssCoexEnable %d !!! \n",
				  pAd->CommonCfg.bBssCoexEnable));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("pAd->CommonCfg.Channel %d !!! \n",
				  pAd->CommonCfg.Channel));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable %d !!! \n",
				  pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("pAd->ApCliMlmeAux.ExtCapInfo.BssCoexstSup %d !!! \n",
				  pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("pAd->CommonCfg.CentralChannel %d !!! \n",
				  pAd->CommonCfg.CentralChannel));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("pAd->CommonCfg.PhyMode %d !!! \n",
				  pAd->CommonCfg.PhyMode));
		}
	}
#endif /* APCLI_CERT_SUPPORT */	
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	/* When root AP is Open WEP, it will cause a fake connection state if user keys in wrong password. */
	if((result == TRUE) &&
	   (wdev->AuthMode == Ndis802_11AuthModeOpen) &&
	   (wdev->WepStatus == Ndis802_11WEPEnabled))
		pApCliEntry->OpenWEPErrPktChk = TRUE;
	else
		pApCliEntry->OpenWEPErrPktChk = FALSE;
	pApCliEntry->OpenWEPErrPktCnt = 0;
	pApCliEntry->OpenWEPErrMCPktCnt = 0;

	return result;
}


/*
    ==========================================================================

	Routine	Description:
		Disconnect current BSSID

	Arguments:
		pAd				- Pointer to our adapter
		ApCliIdx		- Which ApCli interface
	Return Value:
		None

	Note:

	==========================================================================
*/
VOID ApCliLinkDown(RTMP_ADAPTER *pAd, UCHAR ifIndex)
{
	APCLI_STRUCT *pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	UCHAR MacTabWCID = 0;

	if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		|| (ifIndex >= 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
	{
#ifdef MAC_REPEATER_SUPPORT
		if (ifIndex >= 64)
		{
			CliIdx = ((ifIndex - 64) % MAX_EXT_MAC_ADDR_SIZE);
			ifIndex = ((ifIndex - 64) / MAX_EXT_MAC_ADDR_SIZE);
			printk("!!! REPEATER CLI LINK DOWN - IF(apcli%d) Cli %d !!!\n", ifIndex, CliIdx);
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
		printk("!!! APCLI LINK DOWN - IF(apcli%d)!!!\n", ifIndex);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! ERROR : APCLI LINK DOWN - IF(apcli%d)!!!\n", ifIndex));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if ((pApCliEntry->Valid == FALSE)
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#if defined (CONFIG_WIFI_PKT_FWD)
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		if (wf_fwd_entry_delete_hook)
			wf_fwd_entry_delete_hook (pApCliEntry->wdev.if_dev, pAd->net_dev, 1);
	}
#ifdef MAC_REPEATER_SUPPORT
	else
	{
		if (packet_source_delete_entry_hook)
			packet_source_delete_entry_hook(100);
	}
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	pAd->ApCfg.ApCliInfRunned--;

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		MacTabWCID = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID;
	else
#endif /* MAC_REPEATER_SUPPORT */
	MacTabWCID = pApCliEntry->MacTabWCID;

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkDown CliIdx != 0xFF(1) MacTabWCID=%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",MacTabWCID,PRINT_MAC(pAd->MacTab.Content[MacTabWCID].Addr)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkDown CliIdx != 0xFF(2) pApCliEntry->MacTabWCID =%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",pApCliEntry->MacTabWCID,PRINT_MAC(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID))));
	
	} else 
#endif	/* MAC_REPEATER_SUPPORT */
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkDown CliIdx == 0xFF(1) MacTabWCID=%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",MacTabWCID,PRINT_MAC(pAd->MacTab.Content[MacTabWCID].Addr)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliLinkDown CliIdx == 0xFF(2) pApCliEntry->MacTabWCID =%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",pApCliEntry->MacTabWCID,PRINT_MAC(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID))));
	
	}
	
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
    		MacTableDeleteEntry(pAd, MacTabWCID, pAd->MacTab.Content[MacTabWCID].Addr);
	}
  	else		
#endif	/* MAC_REPEATER_SUPPORT */	
  	{
	MacTableDeleteEntry(pAd, MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID));
  	}
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif
#ifdef MT_MAC
#ifdef MULTI_APCLI_SUPPORT
		RTMP_STA_ENTRY_MAC_RESET(pAd,APCLI_MCAST_WCID(ifIndex));
#else

		RTMP_STA_ENTRY_MAC_RESET(pAd, APCLI_MCAST_WCID);
#endif
#endif

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;	/* This link doesn't associated with any remote-AP */
		pApCliEntry->wdev.allow_data_tx = FALSE;
		pApCliEntry->wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	}
	pApCliEntry->OpenWEPErrPktChk = FALSE;
	pApCliEntry->OpenWEPErrPktCnt = 0;
	pApCliEntry->OpenWEPErrMCPktCnt = 0;

#ifdef WPA_SUPPLICANT_SUPPORT
	if (pApCliEntry->wpa_supplicant_info.WpaSupplicantUP)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) ApCli interface[%d] Send RT_DISASSOC_EVENT_FLAG.\n", __FUNCTION__, ifIndex));
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
	}
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef APCLI_CERT_SUPPORT
	if (pAd->bApCliCertTest == TRUE)
	{
		AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm); /* Restore AP's EDCA parameters. */
		pAd->ApCfg.BssEdcaParm.EdcaUpdateCount++;
	}	
#endif /* APCLI_CERT_SUPPORT */

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
	RT_CFG80211_LOST_GO_INFORM(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

}


/*
    ==========================================================================
    Description:
        APCLI Interface Up.
    ==========================================================================
 */
VOID ApCliIfUp(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
	APCLI_STRUCT *pApCliEntry;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCurrState = NULL;
#endif /* APCLI_CONNECTION_TRIAL */

	/* Reset is in progress, stop immediately */
	if ( RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)))
		return;

	/* sanity check whether the interface is initialized. */
	if (pAd->flg_apcli_init != TRUE)
		return;

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef APCLI_CONNECTION_TRIAL				
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#endif /* APCLI_CONNECTION_TRIAL */

		if (APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (pApCliEntry->Enable == TRUE)
			&& (pApCliEntry->Valid == FALSE)
#ifdef APCLI_CONNECTION_TRIAL
			&& (ifIndex == 0)
#endif /* APCLI_CONNECTION_TRIAL */
			)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) ApCli interface[%d] startup.\n", __FUNCTION__, ifIndex));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex);
		}
#ifdef APCLI_CONNECTION_TRIAL
		else if (
			APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (*pCurrState == APCLI_CTRL_DISCONNECTED)//Apcli1 is not connected state.
			&& (pApCliEntry->TrialCh != 0)
			//&& NdisCmpMemory(pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->CfgSsid, pApCliEntry->SsidLen) != 0
			&& (pApCliEntry->CfgSsidLen != 0)
			&& (pApCliEntry->Enable != 0)
			//new ap ssid shall different from the origin one.
		)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Enqueue APCLI_CTRL_TRIAL_CONNECT\n", __func__));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_CONNECT, 0, NULL, ifIndex);
		}
#endif /* APCLI_CONNECTION_TRIAL */
	}

	return;
}


/*
    ==========================================================================
    Description:
        APCLI Interface Down.
    ==========================================================================
 */
VOID ApCliIfDown(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
#if defined (CONFIG_WIFI_PKT_FWD)
	PAPCLI_STRUCT pApCliEntry;
#endif /* CONFIG_WIFI_PKT_FWD */
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx,idx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
#if defined (CONFIG_WIFI_PKT_FWD)
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];


		if (wf_fwd_entry_delete_hook)
            wf_fwd_entry_delete_hook (pApCliEntry->wdev.if_dev, pAd->net_dev, 1);
#endif /* CONFIG_WIFI_PKT_FWD */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ApCli interface[%d] start down.\n", __FUNCTION__, ifIndex));

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
		{
			for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable)
				{
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*ifIndex + CliIdx));
					RTMP_MLME_HANDLER(pAd);
					//RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
	}

#ifdef MAC_REPEATER_SUPPORT
	for (idx = 0; idx< 32; idx++)
	{
		pEntry = &pAd->ApCfg.ReptControl.RepeaterInvaildEntry[idx];

		RTMPRepeaterRemoveInvaildMacEntry(pAd, idx, pEntry->MacAddr);
	}
#endif /* MAC_REPEATER_SUPPORT */

	return;
}


/*
    ==========================================================================
    Description:
        APCLI Interface Monitor.
    ==========================================================================
 */
VOID ApCliIfMonitor(RTMP_ADAPTER *pAd)
{
	UCHAR index;
	APCLI_STRUCT *pApCliEntry;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx;
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	/* Reset is in progress, stop immediately */
	if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
		 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	/* sanity check whether the interface is initialized. */
	if (pAd->flg_apcli_init != TRUE)
		return;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		UCHAR Wcid;
		PMAC_TABLE_ENTRY pMacEntry;
		STA_TR_ENTRY *tr_entry;
		BOOLEAN bForceBrocken = FALSE;

#ifdef APCLI_CONNECTION_TRIAL
		if (index == 1)
			continue;//skip apcli1 monitor. FIXME:Carter shall find a better way.
#endif /* APCLI_CONNECTION_TRIAL */

		pApCliEntry = &pAd->ApCfg.ApCliTab[index];
#ifdef MAC_REPEATER_SUPPORT
		if ((pAd->ApCfg.bMACRepeaterEn) && (pApCliEntry->Enable))
		{
			for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
				pReptCliEntry = &pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx];

				if (pReptCliEntry->CliEnable)
				{
					Wcid = pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].MacTabWCID;

					if (!VALID_WCID(Wcid))
						continue;

					pMacEntry = &pAd->MacTab.Content[Wcid];
					tr_entry = &pAd->MacTab.tr_entry[Wcid];
					if ((tr_entry->PortSecured != WPA_802_1X_PORT_SECURED) &&
						RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pReptCliEntry->CliTriggerTime + (5 * OS_HZ))))
					{
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*index + CliIdx));
						RTMP_MLME_HANDLER(pAd);
						//RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */

		if (pApCliEntry->Valid == TRUE)
		{
			BOOLEAN ApclibQosNull = FALSE;

			Wcid = pAd->ApCfg.ApCliTab[index].MacTabWCID;
			if (!VALID_WCID(Wcid))
				continue;
			pMacEntry = &pAd->MacTab.Content[Wcid];
 			tr_entry = &pAd->MacTab.tr_entry[Wcid];

			if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				&& (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
				&& (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pApCliEntry->ApCliLinkUpTime + (30 * OS_HZ)))))
				bForceBrocken = TRUE;

 			if (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pApCliEntry->ApCliRcvBeaconTime + (12 * OS_HZ)))) {
				bForceBrocken = TRUE;
				printk("ApCliIfMonitor: IF(%s%d) - no Beacon is received from Root-AP.\n", INF_APCLI_DEV_NAME, index);
			}

			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
				ApclibQosNull = TRUE;

			if ((bForceBrocken == FALSE)
#ifdef CONFIG_MULTI_CHANNEL
			&& (pAd->Mlme.bStartMcc == FALSE)	
#endif /* CONFIG_MULTI_CHANNEL */
			)
				ApCliRTMPSendNullFrame(pAd, pMacEntry->CurrTxRate, ApclibQosNull, pMacEntry, PWR_ACTIVE);
		}
		else
			continue;

		if (bForceBrocken == TRUE)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliIfMonitor: Reconnect the Root-Ap again.\n"));

#ifdef CONFIG_MULTI_CHANNEL
			if(pAd->Mlme.bStartMcc == TRUE)
				return;
#endif /* CONFIG_MULTI_CHANNEL */

#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn)
			{
				for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
				{
					if (pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].CliEnable)
					{
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*index + CliIdx));
						RTMP_MLME_HANDLER(pAd);
						//RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
#endif /* MAC_REPEATER_SUPPORT */
			//MCC TODO: WCID Not Correct when MCC on
#ifdef MCC_TEST
#else
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, index);
			RTMP_MLME_HANDLER(pAd);
#endif /* MCC_TEST */		
		}
	}

	return;
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  pFrame         The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN ApCliMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	USHORT Seq;
	UCHAR EAPType;
	BOOLEAN Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
    PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */


	/* only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, */
	/* ignore this frame */

	/* WPA EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA)
	{
#ifdef WSC_AP_SUPPORT
	        /*WSC EAPOL PACKET */
	        pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
	        if (pEntry && IS_ENTRY_APCLI(pEntry) && pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].WscControl.WscConfMode == WSC_ENROLLEE)
	        {
	            *Machine = WSC_STATE_MACHINE;
	            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
	            EAPCode = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
	            Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
	        }
	        if (!Return)
#endif /* WSC_AP_SUPPORT */
	        {
	    		*Machine = WPA_STATE_MACHINE;
	    		EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
	    		Return = WpaMsgTypeSubst(EAPType, MsgType);
	        }
		return Return;
	}
	else if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT)
	{
		switch (pFrame->Hdr.FC.SubType)
		{
			case SUBTYPE_ASSOC_RSP:
				*Machine = APCLI_ASSOC_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_ASSOC_RSP;
				break;

			case SUBTYPE_DISASSOC:
				*Machine = APCLI_ASSOC_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_DISASSOC_REQ;
				break;

			case SUBTYPE_DEAUTH:
				*Machine = APCLI_AUTH_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_DEAUTH;
				break;

			case SUBTYPE_AUTH:
				/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
				NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
				if (Seq == 2 || Seq == 4)
				{
					*Machine = APCLI_AUTH_STATE_MACHINE;
					*MsgType = APCLI_MT2_PEER_AUTH_EVEN;
				}
				else
				{
					return FALSE;
				}
				break;

			case SUBTYPE_ACTION:
				*Machine = ACTION_STATE_MACHINE;
				/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
				if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG)
					*MsgType = MT2_ACT_INVALID;
				else
					*MsgType = (pFrame->Octet[0]&0x7F);
				break;

			default:
				return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}


BOOLEAN preCheckMsgTypeSubset(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT)
	{
		switch (pFrame->Hdr.FC.SubType)
		{
			/* Beacon must be processed by AP Sync state machine. */
			case SUBTYPE_BEACON:
				*Machine = AP_SYNC_STATE_MACHINE;
				*MsgType = APMT2_PEER_BEACON;
				break;

			/* Only Sta have chance to receive Probe-Rsp. */
			case SUBTYPE_PROBE_RSP:
				*Machine = APCLI_SYNC_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_PROBE_RSP;
				break;

			default:
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


/*
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN ApCliPeerAssocRspSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pStatus,
    OUT USHORT *pAid,
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
    OUT UCHAR *pHtCapabilityLen,
    OUT UCHAR *pAddHtInfoLen,
    OUT UCHAR *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag,
    OUT IE_LISTS *ie_list)
{
	CHAR          IeType/*, *Ptr*/;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PEID_STRUCT   pEid;
	ULONG         Length = 0;

	*pNewExtChannelOffset = 0xff;
	*pHtCapabilityLen = 0;
	*pAddHtInfoLen = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	//Ptr = (CHAR *) pFrame->Octet;
	Length += LENGTH_802_11;

	NdisMoveMemory(pCapabilityInfo, &pFrame->Octet[0], 2);
	Length += 2;
	NdisMoveMemory(pStatus,         &pFrame->Octet[2], 2);
	Length += 2;
	*pCkipFlag = 0;
	*pExtRateLen = 0;
	pEdcaParm->bValid = FALSE;

	if (*pStatus != MLME_SUCCESS)
		return TRUE;

	NdisMoveMemory(pAid, &pFrame->Octet[4], 2);
	Length += 2;

	/* Aid already swaped byte order in RTMPFrameEndianChange() for big endian platform */
	*pAid = (*pAid) & 0x3fff; /* AID is low 14-bit */

	/* -- get supported rates from payload and advance the pointer */
	IeType = pFrame->Octet[6];
	*pSupRateLen = pFrame->Octet[7];
	if ((IeType != IE_SUPP_RATES) || (*pSupRateLen > MAX_LEN_OF_SUPPORTED_RATES))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): fail - wrong SupportedRates IE\n", __FUNCTION__));
		return FALSE;
	}
	else
		NdisMoveMemory(SupRate, &pFrame->Octet[8], *pSupRateLen);

	Length = Length + 2 + *pSupRateLen;

	/* many AP implement proprietary IEs in non-standard order, we'd better */
	/* tolerate mis-ordered IEs to get best compatibility */
	pEid = (PEID_STRUCT) &pFrame->Octet[8 + (*pSupRateLen)];

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen)
	{
		switch (pEid->Eid)
		{
			case IE_EXT_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
					*pExtRateLen = pEid->Len;
				}
				break;
#ifdef DOT11_N_SUPPORT
			case IE_HT_CAP:
			case IE_HT_CAP2:
				if (pEid->Len >= SIZE_HT_CAP_IE)  /*Note: allow extension.!! */
				{
					NdisMoveMemory(pHtCapability, pEid->Octet, SIZE_HT_CAP_IE);
					*(USHORT *) (&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
					*(USHORT *) (&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
					*pHtCapabilityLen = SIZE_HT_CAP_IE;
				}
				else
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s():wrong IE_HT_CAP\n", __FUNCTION__));
				}

				break;
			case IE_ADD_HT:
			case IE_ADD_HT2:
				if (pEid->Len >= sizeof(ADD_HT_INFO_IE))
				{
					/* This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only */
					/* copy first sizeof(ADD_HT_INFO_IE) */
					NdisMoveMemory(pAddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
					*pAddHtInfoLen = SIZE_ADD_HT_INFO_IE;
				}
				else
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s():wrong IE_ADD_HT\n", __FUNCTION__));
				}
				break;
			case IE_SECONDARY_CH_OFFSET:
				if (pEid->Len == 1)
				{
					*pNewExtChannelOffset = pEid->Octet[0];
				}
				else
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s():wrong IE_SECONDARY_CH_OFFSET\n", __FUNCTION__));
				}
				break;
#endif /* DOT11_N_SUPPORT */
			/* CCX2, WMM use the same IE value */
			/* case IE_CCX_V2: */
			case IE_VENDOR_SPECIFIC:
				/* handle WME PARAMTER ELEMENT */
				if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
				{
					PUCHAR ptr;
					int i;

					/* parsing EDCA parameters */
					pEdcaParm->bValid          = TRUE;
					pEdcaParm->bQAck           = FALSE; /* pEid->Octet[0] & 0x10; */
					pEdcaParm->bQueueRequest   = FALSE; /* pEid->Octet[0] & 0x20; */
					pEdcaParm->bTxopRequest    = FALSE; /* pEid->Octet[0] & 0x40; */
					/*pEdcaParm->bMoreDataAck    = FALSE; // pEid->Octet[0] & 0x80; */
					pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
					pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
					ptr = (PUCHAR) &pEid->Octet[8];
					for (i=0; i<4; i++)
					{
						UCHAR aci = (*ptr & 0x60) >> 5; /* b5~6 is AC INDEX */
						pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   /* b5 is ACM */
						pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               /* b0~3 is AIFSN */
						pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             /* b0~4 is Cwmin */
						pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               /* b5~8 is Cwmax */
						pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); /* in unit of 32-us */
						ptr += 4; /* point to next AC */
					}
				}
				break;
				default:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ignore unrecognized EID = %d\n", __FUNCTION__, pEid->Eid));
					break;
		}

		Length = Length + 2 + pEid->Len;
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
	}

	return TRUE;
}


MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR *pAddrs)
{
	ULONG ApCliIndex;
	PMAC_TABLE_ENTRY pCurEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;

	if (!VALID_WCID(wcid))
		return NULL;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	do
	{
		pCurEntry = &pAd->MacTab.Content[wcid];

		ApCliIndex = 0xff;
		if ((pCurEntry) && IS_ENTRY_APCLI(pCurEntry))
		{
			ApCliIndex = pCurEntry->func_tb_idx;
		}

		if ((ApCliIndex == 0xff) || (ApCliIndex >= MAX_APCLI_NUM))
			break;

		if (pAd->ApCfg.ApCliTab[ApCliIndex].Valid != TRUE)
			break;

		if (MAC_ADDR_EQUAL(pCurEntry->Addr, pAddrs))
		{
			pEntry = pCurEntry;
			break;
		}
	} while(FALSE);

	NdisReleaseSpinLock(&pAd->MacTabLock);

	return pEntry;
}


/*
	==========================================================================
	Description:
		Check the Apcli Entry is valid or not.
	==========================================================================
 */
static inline BOOLEAN ValidApCliEntry(RTMP_ADAPTER *pAd, INT apCliIdx)
{
	BOOLEAN result;
	PMAC_TABLE_ENTRY pMacEntry;
	APCLI_STRUCT *pApCliEntry;

	do
	{
		if ((apCliIdx < 0) || (apCliIdx >= MAX_APCLI_NUM))
		{
			result = FALSE;
			break;
		}

		pApCliEntry = (APCLI_STRUCT *)&pAd->ApCfg.ApCliTab[apCliIdx];
		if (pApCliEntry->Valid != TRUE)
		{
			result = FALSE;
			break;
		}

		if ((pApCliEntry->MacTabWCID <= 0)
			|| (pApCliEntry->MacTabWCID >= MAX_LEN_OF_MAC_TABLE))
		{
			result = FALSE;
			break;
		}

		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		if (!IS_ENTRY_APCLI(pMacEntry))
		{
			result = FALSE;
			break;
		}

		result = TRUE;
	} while(FALSE);

	return result;
}


INT ApCliAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR *pWcid)
{
	UCHAR idx;
	BOOLEAN	allowed = FALSE;
	APCLI_STRUCT *apcli_entry;


	for(idx = 0; idx < MAX_APCLI_NUM; idx++)
	{
		apcli_entry = &pAd->ApCfg.ApCliTab[idx];
		if (apcli_entry && (&apcli_entry->wdev == wdev))
		{
			if (ValidApCliEntry(pAd, idx) == FALSE)
				break;

#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn == TRUE)
			{
				PUCHAR pSrcBufVA = NULL;
				PACKET_INFO PacketInfo;
				UINT SrcBufLen;
				MAC_TABLE_ENTRY *pMacEntry;
				STA_TR_ENTRY *tr_entry;
				REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
				UCHAR isLinkValid;

				RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

				pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, (pSrcBufVA + MAC_ADDR_LEN), TRUE, &isLinkValid);
				if (pReptEntry && pReptEntry->CliValid)
				{
					*pWcid = pReptEntry->MacTabWCID;
					return TRUE;
				}
				else
				{
					if (!isLinkValid)
						return FALSE;
					if (RTMPRepeaterVaildMacEntry(pAd, pSrcBufVA + MAC_ADDR_LEN))
					{
						pMacEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[idx].MacTabWCID];
						tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
						if ((tr_entry) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
						{
							RTMPInsertRepeaterEntry(pAd, idx, (pSrcBufVA + MAC_ADDR_LEN));
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" Receive trigger packet !!!\n"));
							return FALSE;
						}

					}
				}

				*pWcid = apcli_entry->MacTabWCID;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pAd->RalinkCounters.PendingNdisPacketCount ++;
				RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);
				*pWcid = apcli_entry->MacTabWCID;
			}
			allowed = TRUE;
			break;
		}
	}

	return allowed;
}


/*
	========================================================================

	Routine Description:
		Validate the security configuration against the RSN information
		element

	Arguments:
		pAdapter	Pointer	to our adapter
		eid_ptr 	Pointer to VIE

	Return Value:
		TRUE 	for configuration match
		FALSE	for otherwise

	Note:

	========================================================================
*/
BOOLEAN ApCliValidateRSNIE(
	IN RTMP_ADAPTER *pAd,
	IN PEID_STRUCT pEid_ptr,
	IN USHORT eid_len,
	IN USHORT idx)
{
	PUCHAR pVIE, pTmp;
	UCHAR len;
	PEID_STRUCT         pEid;
	CIPHER_SUITE		WPA;			/* AP announced WPA cipher suite */
	CIPHER_SUITE		WPA2;			/* AP announced WPA2 cipher suite */
	USHORT Count;
	UCHAR Sanity;
	PAPCLI_STRUCT pApCliEntry = NULL;
	PRSN_IE_HEADER_STRUCT pRsnHeader;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	NDIS_802_11_AUTHENTICATION_MODE TmpAuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA_AuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA_AuthModeAux;
	NDIS_802_11_AUTHENTICATION_MODE WPA2_AuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA2_AuthModeAux;
	struct wifi_dev *wdev;

	pVIE = (PUCHAR) pEid_ptr;
	len	 = eid_len;

	/*if (len >= MAX_LEN_OF_RSNIE || len <= MIN_LEN_OF_RSNIE) */
	/*	return FALSE; */

	/* Init WPA setting */
	WPA.PairCipher    	= Ndis802_11WEPDisabled;
	WPA.PairCipherAux 	= Ndis802_11WEPDisabled;
	WPA.GroupCipher   	= Ndis802_11WEPDisabled;
	WPA.RsnCapability 	= 0;
	WPA.bMixMode      	= FALSE;
	WPA_AuthMode	  	= Ndis802_11AuthModeOpen;
	WPA_AuthModeAux		= Ndis802_11AuthModeOpen;

	/* Init WPA2 setting */
	WPA2.PairCipher    	= Ndis802_11WEPDisabled;
	WPA2.PairCipherAux 	= Ndis802_11WEPDisabled;
	WPA2.GroupCipher   	= Ndis802_11WEPDisabled;
	WPA2.RsnCapability 	= 0;
	WPA2.bMixMode      	= FALSE;
	WPA2_AuthMode	  	= Ndis802_11AuthModeOpen;
	WPA2_AuthModeAux	= Ndis802_11AuthModeOpen;

	Sanity = 0;

	/* 1. Parse Cipher this received RSNIE */
	while (len > 0)
	{
		pTmp = pVIE;
		pEid = (PEID_STRUCT) pTmp;

		switch(pEid->Eid)
		{
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
				{
					/* if unsupported vendor specific IE */
					break;
				}
				/* Skip OUI ,version and multicast suite OUI */
				pTmp += 11;

				/*
					Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
						Value      Meaning
						0           None
						1           WEP-40
						2           Tkip
						3           WRAP
						4           AES
						5           WEP-104
				*/
				/* Parse group cipher */
				switch (*pTmp)
				{
					case 1:
					case 5:	/* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
						WPA.GroupCipher = Ndis802_11WEPEnabled;
						break;
					case 2:
						WPA.GroupCipher = Ndis802_11TKIPEnable;
						break;
					case 4:
						WPA.GroupCipher = Ndis802_11AESEnable;
						break;
					default:
						break;
				}

				/* number of unicast suite */
				pTmp += 1;

				/* Store unicast cipher count */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);

				/* pointer to unicast cipher */
			    pTmp += sizeof(USHORT);

				/* Parsing all unicast cipher suite */
				while (Count > 0)
				{
					/* Skip cipher suite OUI */
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;
							break;
						default:
							break;
					}
					if (TmpCipher > WPA.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux */
						WPA.PairCipherAux = WPA.PairCipher;
						WPA.PairCipher    = TmpCipher;
					}
					else
					{
						WPA.PairCipherAux = TmpCipher;
					}
					pTmp++;
					Count--;
				}

				/* Get AKM suite counts */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);

				pTmp   += sizeof(USHORT);

				/* Parse AKM ciphers */
				/* Parsing all AKM cipher suite */
				while (Count > 0)
				{
			    	/* Skip cipher suite OUI */
					pTmp   += 3;
					TmpAuthMode = Ndis802_11AuthModeOpen;
					switch (*pTmp)
					{
						case 1:
							/* WPA-enterprise */
							TmpAuthMode = Ndis802_11AuthModeWPA;
							break;
						case 2:
							/* WPA-personal */
							TmpAuthMode = Ndis802_11AuthModeWPAPSK;
							break;
						default:
							break;
					}
					if (TmpAuthMode > WPA_AuthMode)
					{
						/* Move the lower AKM suite to WPA_AuthModeAux */
						WPA_AuthModeAux = WPA_AuthMode;
						WPA_AuthMode    = TmpAuthMode;
					}
					else
					{
						WPA_AuthModeAux = TmpAuthMode;
					}
				    pTmp++;
					Count--;
				}

				/* ToDo - Support WPA-None ? */

				/* Check the Pair & Group, if different, turn on mixed mode flag */
				if (WPA.GroupCipher != WPA.PairCipher)
					WPA.bMixMode = TRUE;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliValidateRSNIE - RSN-WPA1 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
											((WPA.bMixMode) ? "Mix" : GetEncryptType(WPA.PairCipher)),
											GetEncryptType(WPA.GroupCipher),
											GetAuthMode(WPA_AuthMode)));

				Sanity |= 0x1;
				break; /* End of case IE_WPA */
			case IE_RSN:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

				/* 0. Version must be 1 */
				/*  The pRsnHeader->Version exists in native little-endian order, so we may need swap it for RT_BIG_ENDIAN systems. */
				if (le2cpu16(pRsnHeader->Version) != 1)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - RSN Version isn't 1(%d) \n", pRsnHeader->Version));
					break;
				}

				pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Check cipher OUI */
				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
				{
					/* if unsupported vendor specific IE */
					break;
				}

				/* Skip cipher suite OUI */
				pTmp += 3;

				/* Parse group cipher */
				switch (*pTmp)
				{
					case 1:
					case 5:	/* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
						WPA2.GroupCipher = Ndis802_11WEPEnabled;
						break;
					case 2:
						WPA2.GroupCipher = Ndis802_11TKIPEnable;
						break;
					case 4:
						WPA2.GroupCipher = Ndis802_11AESEnable;
						break;
					default:
						break;
				}

				/* number of unicast suite */
				pTmp += 1;

				/* Get pairwise cipher counts */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);

				pTmp   += sizeof(USHORT);

				/* 3. Get pairwise cipher */
				/* Parsing all unicast cipher suite */
				while (Count > 0)
				{
					/* Skip OUI */
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;
							break;
						default:
							break;
					}
					if (TmpCipher > WPA2.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux */
						WPA2.PairCipherAux = WPA2.PairCipher;
						WPA2.PairCipher    = TmpCipher;
					}
					else
					{
						WPA2.PairCipherAux = TmpCipher;
					}
					pTmp ++;
					Count--;
				}

				/* Get AKM suite counts */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);

				pTmp   += sizeof(USHORT);

				/* Parse AKM ciphers */
				/* Parsing all AKM cipher suite */
				while (Count > 0)
				{
			    	/* Skip cipher suite OUI */
					pTmp   += 3;
					TmpAuthMode = Ndis802_11AuthModeOpen;
					switch (*pTmp)
					{
						case 1:
							/* WPA2-enterprise */
							TmpAuthMode = Ndis802_11AuthModeWPA2;
							break;
						case 2:
							/* WPA2-personal */
							TmpAuthMode = Ndis802_11AuthModeWPA2PSK;
							break;
						default:
							break;
					}
					if (TmpAuthMode > WPA2_AuthMode)
					{
						/* Move the lower AKM suite to WPA2_AuthModeAux */
						WPA2_AuthModeAux = WPA2_AuthMode;
						WPA2_AuthMode    = TmpAuthMode;
					}
					else
					{
						WPA2_AuthModeAux = TmpAuthMode;
					}
				    pTmp++;
					Count--;
				}

				/* Check the Pair & Group, if different, turn on mixed mode flag */
				if (WPA2.GroupCipher != WPA2.PairCipher)
					WPA2.bMixMode = TRUE;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliValidateRSNIE - RSN-WPA2 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
									(WPA2.bMixMode ? "Mix" : GetEncryptType(WPA2.PairCipher)), GetEncryptType(WPA2.GroupCipher),
									GetAuthMode(WPA2_AuthMode)));

				Sanity |= 0x2;
				break; /* End of case IE_RSN */
			default:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("ApCliValidateRSNIE - Unknown pEid->Eid(%d) \n", pEid->Eid));
				break;
		}

		/* skip this Eid */
		pVIE += (pEid->Len + 2);
		len  -= (pEid->Len + 2);

	}

	/* 2. Validate this RSNIE with mine */
	pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
	wdev = &pApCliEntry->wdev;

	/* Peer AP doesn't include WPA/WPA2 capable */
	if (Sanity == 0)
	{
		/* Check the authenticaton mode */
		if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - The authentication mode doesn't match \n", __FUNCTION__));
			return FALSE;
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - The pre-RSNA authentication mode is used. \n", __FUNCTION__));
			return TRUE;
		}
	}
	else if (wdev->AuthMode < Ndis802_11AuthModeWPA)
	{
		/* Peer AP has RSN capability, but our AP-Client is pre-RSNA. Discard this */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - The authentication mode doesn't match. AP is WPA security but APCLI is not. \n", __FUNCTION__));
		return FALSE;
	}


	/* if mixed mode is set on APCLI then change the APCLI mode to more secure mode available on AP */
	if (pApCliEntry->bMixAuthMode == TRUE)
	{
		wdev->WepStatus = Ndis802_11AESEnable;
		if (WPA2_AuthMode == Ndis802_11AuthModeWPA2PSK)
		{
			wdev->AuthMode = Ndis802_11AuthModeWPA2PSK;
			if(WPA2.bMixMode == FALSE)
                wdev->WepStatus = WPA2.PairCipher;
		}
		else if(WPA_AuthMode == Ndis802_11AuthModeWPAPSK)
		{
			wdev->AuthMode = Ndis802_11AuthModeWPAPSK;
			if (WPA.bMixMode == FALSE)
                    wdev->WepStatus = WPA.PairCipher;
		}
	}
	/* Recovery user-defined cipher suite */
	pApCliEntry->PairCipher  = wdev->WepStatus;
	pApCliEntry->GroupCipher = wdev->WepStatus;
	pApCliEntry->bMixCipher  = FALSE;

	Sanity = 0;

	/* Check AuthMode and WPA_AuthModeAux for matching, in case AP support dual-AuthMode */
	/* WPAPSK */
	if ((WPA_AuthMode == wdev->AuthMode) ||
		((WPA_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA_AuthModeAux == wdev->AuthMode)))
	{
		/* Check cipher suite, AP must have more secured cipher than station setting */
		if (WPA.bMixMode == FALSE)
		{
			if (wdev->WepStatus != WPA.GroupCipher)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - WPA validate cipher suite error \n"));
				return FALSE;
			}
		}

		/* check group cipher */
		if (wdev->WepStatus < WPA.GroupCipher)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - WPA validate group cipher error \n"));
			return FALSE;
		}

		/*
			check pairwise cipher, skip if none matched
				If profile set to AES, let it pass without question.
				If profile set to TKIP, we must find one mateched
		*/
		if ((wdev->WepStatus == Ndis802_11TKIPEnable) &&
			(wdev->WepStatus != WPA.PairCipher) &&
			(wdev->WepStatus != WPA.PairCipherAux))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - WPA validate pairwise cipher error \n"));
			return FALSE;
		}

		Sanity |= 0x1;
	}
	/* WPA2PSK */
	else if ((WPA2_AuthMode == wdev->AuthMode) ||
			 ((WPA2_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA2_AuthModeAux == wdev->AuthMode)))
	{
		/* Check cipher suite, AP must have more secured cipher than station setting */
		if (WPA2.bMixMode == FALSE)
		{
			if (wdev->WepStatus != WPA2.GroupCipher)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - WPA2 validate cipher suite error \n"));
				return FALSE;
			}
		}

		/* check group cipher */
		if (wdev->WepStatus < WPA2.GroupCipher)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - WPA2 validate group cipher error \n"));
			return FALSE;
		}

		/*
			check pairwise cipher, skip if none matched
				If profile set to AES, let it pass without question.
				If profile set to TKIP, we must find one mateched
		*/
		if ((wdev->WepStatus == Ndis802_11TKIPEnable) &&
			(wdev->WepStatus != WPA2.PairCipher) &&
			(wdev->WepStatus != WPA2.PairCipherAux))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - WPA2 validate pairwise cipher error \n"));
			return FALSE;
		}

		Sanity |= 0x2;
	}

	if (Sanity == 0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCliValidateRSNIE - Validate RSIE Failure \n"));
		return FALSE;
	}

	/*Re-assign pairwise-cipher and group-cipher. Re-build RSNIE. */
	if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPAPSK))
	{
		pApCliEntry->GroupCipher = WPA.GroupCipher;

		if (wdev->WepStatus == WPA.PairCipher)
			pApCliEntry->PairCipher = WPA.PairCipher;
		else if (WPA.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA.PairCipherAux;
		else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
			pApCliEntry->PairCipher = Ndis802_11TKIPEnable;
	}
	else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) || (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		pApCliEntry->GroupCipher = WPA2.GroupCipher;

		if (wdev->WepStatus == WPA2.PairCipher)
			pApCliEntry->PairCipher = WPA2.PairCipher;
		else if (WPA2.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA2.PairCipherAux;
		else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
			pApCliEntry->PairCipher = Ndis802_11TKIPEnable;
	}

	/* Set Mix cipher flag */
	if (pApCliEntry->PairCipher != pApCliEntry->GroupCipher)
	{
		pApCliEntry->bMixCipher = TRUE;

		/* re-build RSNIE */
		/*RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI)); */
	}

	/* re-build RSNIE */
	RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI));

	return TRUE;
}


BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR wdev_idx)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
	APCLI_STRUCT *pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	/*
		It is possible to receive the multicast packet when in AP Client mode
		ex: broadcast from remote AP to AP-client,
				addr1=ffffff, addr2=remote AP's bssid, addr3=sta4_mac_addr
	*/
	pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];

	/* Filter out Bcast frame which AP relayed for us */
	/* Multicast packet send from AP1 , received by AP2 and send back to AP1, drop this frame */
	if (MAC_ADDR_EQUAL(pHeader->Addr3, pApCliEntry->wdev.if_addr))
		return FALSE;

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn)
	{
		UCHAR isLinkValid;
		
		pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pHeader->Addr3, TRUE, &isLinkValid);
		if (pReptEntry)
			return FALSE;	/* give up this frame */
	}
#endif /* MAC_REPEATER_SUPPORT */



	/* skip the 802.11 header */
	pRxBlk->pData += LENGTH_802_11;
	pRxBlk->DataSize -= LENGTH_802_11;

	/* Use software to decrypt the encrypted frame. */
	/* Because this received frame isn't my BSS frame, Asic passed to driver without decrypting it. */
	/* If receiving an "encrypted" unicast packet(its WEP bit as 1) and doesn't match my BSSID, it */
	/* pass to driver with "Decrypted" marked as 0 in RxD. */
	if ((pRxInfo->MyBss == 0) && (pRxInfo->Decrypted == 0) && (pHeader->FC.Wep == 1))
	{
		if (RTMPSoftDecryptionAction(pAd,
									 (PUCHAR)pHeader, 0,
									 &pApCliEntry->SharedKey[pRxBlk->key_idx],
									 pRxBlk->pData,
									 &(pRxBlk->DataSize)) == NDIS_STATUS_FAILURE)
		{
			return FALSE;  /* give up this frame */
		}
	}
	pRxInfo->MyBss = 1;

	Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);

	return TRUE;
}


VOID APCliInstallPairwiseKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	UCHAR	IfIdx;
	UINT8	BssIdx;

	IfIdx = pEntry->func_tb_idx;
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + IfIdx;
#ifdef MAC_APCLI_SUPPORT
	BssIdx = APCLI_BSSID_IDX + IfIdx;
#endif /* MAC_APCLI_SUPPORT */
#ifdef APCLI_CERT_SUPPORT
	NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].PTK, pEntry->PTK, LEN_PTK);
#endif /* APCLI_CERT_SUPPORT */

	WPAInstallPairwiseKey(pAd, BssIdx, pEntry, FALSE);
}


BOOLEAN APCliInstallSharedKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKey,
	IN  UCHAR           KeyLen,
	IN	UCHAR			DefaultKeyIdx,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	UCHAR	IfIdx;
	UCHAR	GTK_len = 0;
	APCLI_STRUCT *apcli_entry;


	if (!pEntry || !IS_ENTRY_APCLI(pEntry))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : This Entry doesn't exist!!! \n", __FUNCTION__));
		return FALSE;
	}

	IfIdx = pEntry->func_tb_idx;
	ASSERT((IfIdx < MAX_APCLI_NUM));

	apcli_entry = &pAd->ApCfg.ApCliTab[IfIdx];
	if (apcli_entry->GroupCipher == Ndis802_11TKIPEnable && KeyLen >= LEN_TKIP_GTK)
		GTK_len = LEN_TKIP_GTK;
	else if (apcli_entry->GroupCipher == Ndis802_11AESEnable && KeyLen >= LEN_AES_GTK)
		GTK_len = LEN_AES_GTK;
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : GTK is invalid (GroupCipher=%d, DataLen=%d) !!! \n",
								__FUNCTION__, apcli_entry->GroupCipher, KeyLen));
		return FALSE;
	}

	/* Update GTK */
	/* set key material, TxMic and RxMic for WPAPSK */
	NdisMoveMemory(apcli_entry->GTK, pKey, GTK_len);
	apcli_entry->wdev.DefaultKeyId = DefaultKeyIdx;

	/* Update shared key table */
	NdisZeroMemory(&apcli_entry->SharedKey[DefaultKeyIdx], sizeof(CIPHER_KEY));
	apcli_entry->SharedKey[DefaultKeyIdx].KeyLen = GTK_len;
	NdisMoveMemory(apcli_entry->SharedKey[DefaultKeyIdx].Key, pKey, LEN_TK);
	if (GTK_len == LEN_TKIP_GTK)
	{
		NdisMoveMemory(apcli_entry->SharedKey[DefaultKeyIdx].RxMic, pKey + 16, LEN_TKIP_MIC);
		NdisMoveMemory(apcli_entry->SharedKey[DefaultKeyIdx].TxMic, pKey + 24, LEN_TKIP_MIC);
	}

	/* Update Shared Key CipherAlg */
	apcli_entry->SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_NONE;
	if (apcli_entry->GroupCipher == Ndis802_11TKIPEnable)
		apcli_entry->SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_TKIP;
	else if (apcli_entry->GroupCipher == Ndis802_11AESEnable)
		apcli_entry->SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_AES;

#ifdef MAC_APCLI_SUPPORT
	RTMP_ASIC_SHARED_KEY_TABLE(pAd,
								APCLI_BSS_BASE + IfIdx,
								DefaultKeyIdx,
								&apcli_entry->SharedKey[DefaultKeyIdx]);
#endif /* MAC_APCLI_SUPPORT */

	return TRUE;
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
// TODO: shiang-6590, modify this due to it's really a duplication of "RTMPUpdateMlmeRate()" in common/mlme.c
VOID ApCliUpdateMlmeRate(RTMP_ADAPTER *pAd, USHORT ifIndex)
{
	UCHAR	MinimumRate;
	UCHAR	ProperMlmeRate; /*= RATE_54; */
	UCHAR	i, j, RateIdx = 12; /* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
	BOOLEAN	bMatch = FALSE;
	UCHAR   PhyMode = pAd->CommonCfg.PhyMode;

	PAPCLI_STRUCT pApCliEntry = NULL;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef APCLI_AUTO_BW_SUPPORT
	PhyMode = pApCliEntry->wdev.PhyMode;	
#endif /* APCLI_AUTO_BW_SUPPORT */
	
	switch (PhyMode)
	{
		case (WMODE_B):
			ProperMlmeRate = RATE_11;
			MinimumRate = RATE_1;
			break;
		case (WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
		case (WMODE_A |WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
		case (WMODE_B | WMODE_G | WMODE_GN):
#endif /* DOT11_N_SUPPORT */
			if ((pApCliEntry->MlmeAux.SupRateLen == 4) &&
				(pApCliEntry->MlmeAux.ExtRateLen == 0))
				ProperMlmeRate = RATE_11; /* B only AP */
			else
				ProperMlmeRate = RATE_24;

			if (pApCliEntry->MlmeAux.Channel <= 14)
				MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		case (WMODE_A):
#ifdef DOT11_N_SUPPORT
		case (WMODE_GN):
		case (WMODE_G | WMODE_GN):
		case (WMODE_A | WMODE_G | WMODE_AN | WMODE_GN):
		case (WMODE_A | WMODE_AN):
		case (WMODE_AN):
#endif /* DOT11_N_SUPPORT */
			ProperMlmeRate = RATE_24;
			MinimumRate = RATE_6;
			break;
		case (WMODE_B | WMODE_A | WMODE_G):
			ProperMlmeRate = RATE_24;
			if (pApCliEntry->MlmeAux.Channel <= 14)
			   MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		default: /* error */
			ProperMlmeRate = RATE_1;
			MinimumRate = RATE_1;
			break;
	}

	for (i = 0; i < pApCliEntry->MlmeAux.SupRateLen; i++)
	{
		for (j = 0; j < RateIdx; j++)
		{
			if ((pApCliEntry->MlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
			{
				if (j == ProperMlmeRate)
				{
					bMatch = TRUE;
					break;
				}
			}
		}

		if (bMatch)
			break;
	}

	if (bMatch == FALSE)
	{
		for (i = 0; i < pApCliEntry->MlmeAux.ExtRateLen; i++)
		{
			for (j = 0; j < RateIdx; j++)
			{
				if ((pApCliEntry->MlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j])
				{
					if (j == ProperMlmeRate)
					{
						bMatch = TRUE;
						break;
					}
				}
			}

			if (bMatch)
				break;
		}
	}

	if (bMatch == FALSE)
		ProperMlmeRate = MinimumRate;

	if(!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
	{
		pAd->CommonCfg.MlmeRate = MinimumRate;
		pAd->CommonCfg.RtsRate = ProperMlmeRate;
		if (pAd->CommonCfg.MlmeRate >= RATE_6)
		{
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
			pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}
		else
		{
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
			pAd->CommonCfg.MlmeTransmit.field.MCS = pAd->CommonCfg.MlmeRate;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_CCK;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = pAd->CommonCfg.MlmeRate;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():=>MlmeTransmit=0x%x, MinimumRate=%d, ProperMlmeRate=%d\n",
				__FUNCTION__, pAd->CommonCfg.MlmeTransmit.word, MinimumRate, ProperMlmeRate));
}


extern INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
extern INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

VOID APCli_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	PNET_DEV new_dev_p;
	INT idx;
	APCLI_STRUCT *pApCliEntry;
	struct wifi_dev *wdev;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_apcli_init != FALSE)
		return;


	/* init */
	for(idx = 0; idx < MAX_APCLI_NUM; idx++)
		pAd->ApCfg.ApCliTab[idx].wdev.if_dev = NULL;

	/* create virtual network interface */
	for (idx = 0; idx < MAX_APCLI_NUM; idx++)
	{
		UINT32 MC_RowID = 0, IoctlIF = 0;
		char *dev_name;

#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		dev_name = get_dev_name_prefix(pAd, INT_APCLI);
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_APCLI, idx,
									sizeof(struct mt_dev_priv), dev_name);
		if (!new_dev_p) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Create net_device for %s(%d) fail!\n",
						__FUNCTION__, dev_name, idx));
			break;
		}
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
#if   defined( APCLI_CONNECTION_TRIAL) || defined(MULTI_APCLI_SUPPORT) 
		pApCliEntry->ifIndex = idx;
		pApCliEntry->pAd = pAd;
#endif /* APCLI_CONNECTION_TRIAL */
		pApCliEntry->OpenWEPErrPktChk = FALSE;
		pApCliEntry->OpenWEPErrPktCnt = 0;
		pApCliEntry->OpenWEPErrMCPktCnt = 0;
		wdev = &pApCliEntry->wdev;
		wdev->wdev_type = WDEV_TYPE_STA;
		wdev->func_dev = pApCliEntry;
		wdev->func_idx = idx;
		wdev->sys_handle = (void *)pAd;
		wdev->if_dev = new_dev_p;
		wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
		// TODO: shiang-usw, modify this to STASendPacket!
		wdev->tx_pkt_handle = APSendPacket;
		wdev->wdev_hard_tx = APHardTransmit;
		wdev->rx_pkt_allowed = sta_rx_pkt_allow;
		wdev->rx_pkt_foward = sta_rx_fwd_hnd;

		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
						RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		NdisZeroMemory(&pApCliEntry->ApCliCounter, sizeof(APCLI_COUNTER));

		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);

#ifdef MT_MAC
		if (pAd->chipCap.hif_type != HIF_MT)
		{
#endif /* MT_MAC */
			if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
			{
				if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0))
				{
					UCHAR MacMask = 0;

					if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
						MacMask = 0xFE;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
						MacMask = 0xFC;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
						MacMask = 0xF8;

					/*
						Refer to HW definition -
							Bit1 of MAC address Byte0 is local administration bit
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					*/
					if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
					{
						/*
							Refer to HW definition -
								Bit1 of MAC address Byte0 is local administration bit
								and should be set to 1 in extended multiple BSSIDs'
								Bit3~ of MAC address Byte0 is extended multiple BSSID index.
						*/
#ifdef ENHANCE_NEW_MBSSID_MODE
						wdev->if_addr[0] &= (MacMask << 2);
#endif /* ENHANCE_NEW_MBSSID_MODE */
						wdev->if_addr[0] |= 0x2;
						wdev->if_addr[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
					}
#ifdef ENHANCE_NEW_MBSSID_MODE
					else
					{
						wdev->if_addr[0] |= 0x2;
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += ((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
					}
#endif /* ENHANCE_NEW_MBSSID_MODE */
				}
			}
			else
			{
				wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;
			}
#ifdef MT_MAC
		}
		else {
			volatile UINT32 Value;
			UCHAR MacByte = 0;

			//TODO: shall we make choosing which byte to be selectable???
			Value = 0x00000000L;
			RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);//read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.
			Value = Value | (0x2 << 29);//Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address
			RTMP_IO_WRITE32(pAd, LPON_BTEIR, Value);
			MacByte = Value >> 29;


			//Carter, I make apcli interface use HWBSSID1 to go.
			//so fill own_mac and BSSID here.
			wdev->if_addr[0] |= 0x2;

			switch (MacByte) {
				case 0x1: /* choose bit[23:20]*/
					wdev->if_addr[2] = (wdev->if_addr[2] & 0x0f);
					break;
				case 0x2: /* choose bit[31:28]*/
					wdev->if_addr[3] = (wdev->if_addr[3] & 0x0f);
					break;
				case 0x3: /* choose bit[39:36]*/
					wdev->if_addr[4] = (wdev->if_addr[4] & 0x0f);
					break;
				case 0x4: /* choose bit [47:44]*/
					wdev->if_addr[5] = (wdev->if_addr[5] & 0x0f);
					break;
				default: /* choose bit[15:12]*/
					wdev->if_addr[1] = (wdev->if_addr[1] & 0x0f);
					break;
			}
#ifdef MULTI_APCLI_SUPPORT //tmp use byte5 to distinguish, may refine this by setting
						wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + idx);
#endif /* MULTI_APCLI_SUPPORT */

//			AsicSetDevMac(pAd, wdev->if_addr, 0x1);//set own_mac to HWBSSID1
			//AsicSetBssid(pAd, wdev->if_addr, 0x1);
		}
#endif /* MT_MAC */

		pNetDevOps->priv_flags = INT_APCLI; /* we are virtual interface */
		pNetDevOps->needProtcted = TRUE;
		pNetDevOps->wdev = wdev;
		NdisMoveMemory(pNetDevOps->devAddr, &wdev->if_addr[0], MAC_ADDR_LEN);

		/* register this device to OS */
		RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	}

	pAd->flg_apcli_init = TRUE;

}


VOID ApCli_Remove(RTMP_ADAPTER *pAd)
{
	UINT index;
	struct wifi_dev *wdev;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		wdev = &pAd->ApCfg.ApCliTab[index].wdev;
		if (wdev->if_dev)
		{
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);

			rtmp_wdev_idx_unreg(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);

			/* Clear it as NULL to prevent latter access error. */
			pAd->flg_apcli_init = FALSE;
			wdev->if_dev = NULL;
		}
	}
}


BOOLEAN ApCli_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	UCHAR ifIndex;
	APCLI_STRUCT *pApCliEntry;
	struct wifi_dev *wdev = NULL;
	
#if defined (CONFIG_WIFI_PKT_FWD)
	if (wf_fwd_probe_adapter)
		wf_fwd_probe_adapter(pAd);
#endif 

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		if (pAd->ApCfg.ApCliTab[ifIndex].wdev.if_dev == dev_p)
		{
			RTMP_OS_NETDEV_START_QUEUE(dev_p);

            pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
            wdev = &pApCliEntry->wdev;
#ifdef MULTI_APCLI_SUPPORT
			AsicSetDevMac(pAd, wdev->if_addr, (1+ifIndex));//Apcli OwnMac start from HWBSSID 1.	
#else /* MULTI_APCLI_SUPPORT */
            AsicSetDevMac(pAd, wdev->if_addr, 1);//Apcli OwnMac start from HWBSSID 1.
#endif

#ifdef MAC_REPEATER_SUPPORT
			//Fix AsicSetMacAddrExt() in init flow before load f/w, set it again after apclin up
			AsicSetMacAddrExt(pAd, pAd->ApCfg.bMACRepeaterEn);
#endif /* MAC_REPEATER_SUPPORT */
			
			ApCliIfUp(pAd);

#ifdef WPA_SUPPLICANT_SUPPORT
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_UP, NULL, NULL, 0);
#endif /* WPA_SUPPLICANT_SUPPORT */
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN ApCli_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	UCHAR ifIndex;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx;
#endif /* MAC_REPEATER_SUPPORT */
	struct wifi_dev *wdev;
	APCLI_STRUCT *apcli_entry;

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
		wdev = &apcli_entry->wdev;
		if (wdev->if_dev == dev_p)
		{
#ifdef WPA_SUPPLICANT_SUPPORT
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);

			if (apcli_entry->wpa_supplicant_info.pWpaAssocIe)
			{
				os_free_mem(NULL, apcli_entry->wpa_supplicant_info.pWpaAssocIe);
				apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
				apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
			}
#endif /* WPA_SUPPLICANT_SUPPORT */

			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

			/* send disconnect-req to sta State Machine. */
			if (apcli_entry->Enable)
			{
#ifdef MAC_REPEATER_SUPPORT
				if (pAd->ApCfg.bMACRepeaterEn)
				{
					for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
					{
						if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable)
						{
							MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*ifIndex + CliIdx));
							RTMP_MLME_HANDLER(pAd);
							//RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
						}
					}
				}
#endif /* MAC_REPEATER_SUPPORT */

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
				RTMP_MLME_HANDLER(pAd);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));
			}
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN ApCli_StatsGet(
	IN	PRTMP_ADAPTER pAd,
	IN	RT_CMD_STATS64 *pStats)
{
	INT ifIndex = 0, index;
	APCLI_STRUCT *pApCliTab;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (pAd->ApCfg.ApCliTab[index].wdev.if_dev == pStats->pNetDev)
		{
			ifIndex = index;
			break;
		}
	}

	if (index >= MAX_APCLI_NUM)
	{
		return FALSE;
	}

	pApCliTab = &pAd->ApCfg.ApCliTab[ifIndex];

	pStats->rx_bytes = pApCliTab->ApCliCounter.ReceivedByteCount.QuadPart;
	pStats->tx_bytes = pApCliTab->ApCliCounter.TransmittedByteCount.QuadPart;

	pStats->rx_packets = pApCliTab->ApCliCounter.ReceivedFragmentCount;
	pStats->tx_packets = pApCliTab->ApCliCounter.TransmittedFragmentCount;

	pStats->rx_errors = pApCliTab->ApCliCounter.RxErrorCount;
	pStats->multicast = pApCliTab->ApCliCounter.MulticastReceivedFrameCount;

	return TRUE;
}

#ifdef APCLI_AUTO_CONNECT_SUPPORT

BOOLEAN ApCliSetIfState(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN state)
{
	pAd->ApCfg.ApCliTab[ifIndex].Enable = state;

#ifdef APCLI_CONNECTION_TRIAL
	if (pAd->ApCfg.ApCliTab[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
	ApCliIfDown(pAd);

	return TRUE;
}


BOOLEAN ApCliSetBssid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN UCHAR *Bssid)
{
	BOOLEAN apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;

	/* bring apcli interface down first */
	if (apcliEn == TRUE)
	{
		pAd->ApCfg.ApCliTab[ifIndex].Enable = FALSE;
#ifdef APCLI_CONNECTION_TRIAL
		if (pAd->ApCfg.ApCliTab[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
		ApCliIfDown(pAd);
	}

	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, Bssid, MAC_ADDR_LEN);

	pAd->ApCfg.ApCliTab[ifIndex].Enable = apcliEn;

	return TRUE;
}


BOOLEAN ApCliAutoConnectStart(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex)
{
	AP_ADMIN_CONFIG *pApCfg = &pAd->ApCfg;
	NDIS_802_11_SSID Ssid;

#ifdef WSC_AP_SUPPORT
	if ((pApCfg->ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
	    (pApCfg->ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
		return FALSE;
#endif /* WSC_AP_SUPPORT */

	if (pApCfg->ApCliAutoConnectRunning == FALSE)
	{
		ApCliSetIfState(pAd, ifIndex, FALSE);
		pApCfg->ApCliAutoConnectRunning = TRUE;
	}
	else
	{
		return TRUE;
	}

	/*
		use site survey function to trigger auto connecting (when pAd->ApCfg.ApAutoConnectRunning == TRUE)
	*/
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE);

	return TRUE;
}

/*
	===================================================

	Description:
		Find the AP that is configured in the ApcliTab, and switch to
		the channel of that AP

	Arguments:
		pAd: pointer to our adapter

	Return Value:
		TRUE: no error occured
		FALSE: otherwise

	Note:
	===================================================
*/
BOOLEAN ApCliAutoConnectExec(
	IN  PRTMP_ADAPTER   pAd)
{
	UCHAR			ifIdx, CfgSsidLen, entryIdx;
	RTMP_STRING *pCfgSsid;
	BSS_TABLE		*pScanTab, *pSsidBssTab;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> ApCliAutoConnectExec()\n"));

	for(ifIdx=0; ifIdx<MAX_APCLI_NUM; ifIdx++)
	{
		if (pAd->ApCfg.ApCliTab[ifIdx].AutoConnectFlag== TRUE)
			break;
	}

	if(ifIdx >= MAX_APCLI_NUM)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error  ifIdx=%d \n", ifIdx));
		return FALSE;
	}

#ifdef WSC_AP_SUPPORT
	if ((pAd->ApCfg.ApCliTab[ifIdx].WscControl.WscConfMode != WSC_DISABLE) &&
	    (pAd->ApCfg.ApCliTab[ifIdx].WscControl.bWscTrigger == TRUE))
	{
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		return FALSE;
	}
#endif /* WSC_AP_SUPPORT */

	CfgSsidLen = pAd->ApCfg.ApCliTab[ifIdx].CfgSsidLen;
	pCfgSsid = pAd->ApCfg.ApCliTab[ifIdx].CfgSsid;
	pScanTab = &pAd->ScanTab;
	pSsidBssTab = &pAd->ApCfg.ApCliTab[ifIdx].MlmeAux.SsidBssTab;
	pSsidBssTab->BssNr = 0;

	/*
		Find out APs with the desired SSID.
	*/
	for (entryIdx=0; entryIdx<pScanTab->BssNr;entryIdx++)
	{
		BSS_ENTRY *pBssEntry = &pScanTab->BssEntry[entryIdx];

		if ( pBssEntry->Channel == 0)
			break;

		if (NdisEqualMemory(pCfgSsid, pBssEntry->Ssid, CfgSsidLen) &&
							(pBssEntry->SsidLen) &&
							(pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE))
		{
			if (ApcliCompareAuthEncryp(&pAd->ApCfg.ApCliTab[ifIdx],
										pBssEntry->AuthMode,
										pBssEntry->AuthModeAux,
										pBssEntry->WepStatus,
										pBssEntry->WPA) ||
				ApcliCompareAuthEncryp(&pAd->ApCfg.ApCliTab[ifIdx],
										pBssEntry->AuthMode,
										pBssEntry->AuthModeAux,
										pBssEntry->WepStatus,
										pBssEntry->WPA2))
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("Found desired ssid in Entry %2d:\n", entryIdx));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("I/F(apcli%d) ApCliAutoConnectExec:(Len=%d,Ssid=%s, Channel=%d, Rssi=%d)\n",
						ifIdx, pBssEntry->SsidLen, pBssEntry->Ssid,
						pBssEntry->Channel, pBssEntry->Rssi));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("I/F(apcli%d) ApCliAutoConnectExec::(AuthMode=%s, EncrypType=%s)\n", ifIdx,
						GetAuthMode(pBssEntry->AuthMode),
						GetEncryptType(pBssEntry->WepStatus)) );
				NdisMoveMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr++],
								pBssEntry, sizeof(BSS_ENTRY));
			}
		}
	}

	NdisZeroMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr], sizeof(BSS_ENTRY));

	/*
		Sort by Rssi in the increasing order, and connect to
		the last entry (strongest Rssi)
	*/
	BssTableSortByRssi(pSsidBssTab, TRUE);


	if ((pSsidBssTab->BssNr == 0))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("No match entry.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
	}
	else if (pSsidBssTab->BssNr > 0 &&
			pSsidBssTab->BssNr <=MAX_LEN_OF_BSS_TABLE)
	{
		/*
			Switch to the channel of the candidate AP
		*/
		UCHAR tempBuf[20];
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1];

		if (pAd->CommonCfg.Channel != pBssEntry->Channel)
		{
			ApCliSetIfState(pAd, ifIdx, FALSE);
			
			sprintf(tempBuf, "%d", pBssEntry->Channel);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Switch to channel :%s\n", tempBuf));
			Set_Channel_Proc(pAd, tempBuf);
		}
		
		ApCliSetBssid(pAd, ifIdx, pBssEntry->Bssid);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error! Out of table range: (BssNr=%d).\n", pSsidBssTab->BssNr) );
		ApCliSetIfState(pAd, ifIdx, TRUE);
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--- ApCliAutoConnectExec()\n"));
		return FALSE;
	}

	ApCliSetIfState(pAd, ifIdx, TRUE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--- ApCliAutoConnectExec()\n"));
	return TRUE;

}

/*
	===================================================

	Description:
		If the previous selected entry connected failed, this function will
		choose next entry to connect. The previous entry will be deleted.

	Arguments:
		pAd: pointer to our adapter

	Note:
		Note that the table is sorted by Rssi in the "increasing" order, thus
		the last entry in table has stringest Rssi.
	===================================================
*/

VOID ApCliSwitchCandidateAP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex)
{
	BSS_TABLE 		*pSsidBssTab;
	PAPCLI_STRUCT	pApCliEntry;

	if (pAd->ScanCtrl.PartialScan.bScanning == TRUE)
		return;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> ApCliSwitchCandidateAP()\n"));

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pSsidBssTab = &pApCliEntry->MlmeAux.SsidBssTab;

	if (pSsidBssTab->BssNr == 0)
	{
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		goto exit_and_enable;
	}

	/*
		delete (zero) the previous connected-failled entry and always
		connect to the last entry in talbe until the talbe is empty.
	*/
	NdisZeroMemory(&pSsidBssTab->BssEntry[--pSsidBssTab->BssNr], sizeof(BSS_ENTRY));

	if ((pSsidBssTab->BssNr > 0) && (pSsidBssTab->BssNr <= MAX_LEN_OF_BSS_TABLE))
	{
		UCHAR	tempBuf[20];
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];

		ApCliSetBssid(pAd, ifIndex, pBssEntry->Bssid);
		if (pAd->CommonCfg.Channel != pBssEntry->Channel)
		{
			ApCliSetIfState(pAd, ifIndex, FALSE);
			sprintf(tempBuf, "%d", pBssEntry->Channel);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Switch to channel :%s\n", tempBuf));
			Set_Channel_Proc(pAd, tempBuf);
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("No candidate AP, the process is about to stop.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
	}

exit_and_enable:

	ApCliSetIfState(pAd, ifIndex, TRUE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> ApCliSwitchCandidateAP()\n"));
}


BOOLEAN ApcliCompareAuthEncryp(
	IN PAPCLI_STRUCT pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthModeAux,
	IN NDIS_802_11_WEP_STATUS			WEPstatus,
	IN CIPHER_SUITE WPA)
{
	NDIS_802_11_AUTHENTICATION_MODE	tempAuthMode = pApCliEntry->wdev.AuthMode;
	NDIS_802_11_WEP_STATUS				tempWEPstatus = pApCliEntry->wdev.WepStatus;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApcliAuthMode=%s, AuthMode=%s, AuthModeAux=%s, ApcliWepStatus=%s,	WepStatus=%s, GroupCipher=%s, PairCipher=%s,  \n",
					GetAuthMode(pApCliEntry->wdev.AuthMode),
					GetAuthMode(AuthMode),
					GetAuthMode(AuthModeAux),
					GetEncryptType(pApCliEntry->wdev.WepStatus),
					GetEncryptType(WEPstatus),
					GetEncryptType(WPA.GroupCipher),
					GetEncryptType(WPA.PairCipher)));

	if (tempAuthMode <= Ndis802_11AuthModeAutoSwitch)
	{
		tempAuthMode = Ndis802_11AuthModeOpen;
		return ((tempAuthMode == AuthMode ||
				tempAuthMode == AuthModeAux) &&
				(tempWEPstatus == WEPstatus) );
	}
	else if (tempAuthMode <= Ndis802_11AuthModeWPA2PSK)
	{
		return ((tempAuthMode == AuthMode ||
			tempAuthMode == AuthModeAux) &&
			(tempWEPstatus == WPA.GroupCipher||
			tempWEPstatus == WPA.PairCipher) );
	}
	else
	{
		/* not supported cases */
		return FALSE;
	}

}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

/*
	===================================================

	Description:
		
	When root AP is Open WEP, it will cause a fake connection state if user keys in
	wrong password. So we need this to fix the issue.		

	Arguments:
		pAd: pointer to our adapter
		pRxBlk: carry necessary packet info 802.11 format
		bSuccessPkt: see if it is a successfully decrypted packet.
	Note:
	===================================================
*/
VOID ApCliRxOpenWEPCheck(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN BOOLEAN bSuccessPkt)
{
#ifndef MULTI_APCLI_SUPPORT
	APCLI_STRUCT *pApCliEntry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *pHeader = NULL;
	RXD_BASE_STRUCT *rxd_base = NULL;
	int FirstWcid=1, idx, wcid;
	BOOLEAN isMCPkt = FALSE;

	if(!pRxBlk)
		return;
		
	if(!bSuccessPkt)
	{
		rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;
		idx = rxd_base->rxd_2.wlan_idx;
	}
	else
		idx = pRxBlk->wcid;
	wcid = idx;

	if((wcid >= WCID_ALL) || (wcid == MCAST_WCID))
		return;

	if((pAd->chipCap.hif_type == HIF_MT)&&(wcid == APCLI_MCAST_WCID))
	{
		wcid = (idx - APCLI_MCAST_WCID)+FirstWcid;
		isMCPkt = TRUE;
	}

	if(!VALID_WCID(wcid))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("(%s:%d): Invalid wcid (%d) found! From MCPkt: %d\n",__func__,__LINE__,wcid,isMCPkt));
		return;	
	}

	pEntry = &pAd->MacTab.Content[wcid];
	if(!pEntry || !IS_ENTRY_APCLI(pEntry)) 
	   return;

	if(!VALID_TR_WCID(pEntry->wcid)) 
		return;

	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	if(!tr_entry) 
		return;

	if (pEntry->func_tb_idx >= MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("(%s:%d): func_tb_idx over MAX_APCLI_NUM!\n",__func__,__LINE__));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
	if(!pApCliEntry || 
	   !pApCliEntry->Valid ||
	   !pApCliEntry->OpenWEPErrPktChk) 
	   return;

	if(bSuccessPkt)
	{
		pHeader = pRxBlk->pHeader;
		if( pHeader &&
		   (pHeader->FC.Type == FC_TYPE_DATA) &&
		   (pHeader->FC.SubType != SUBTYPE_DATA_NULL) &&
		   (pHeader->FC.SubType != SUBTYPE_QOS_NULL) &&
		   (pRxBlk->DataSize > 0))
		{
			pApCliEntry->OpenWEPErrPktCnt = 0;
			pApCliEntry->OpenWEPErrMCPktCnt = 0;
			pApCliEntry->OpenWEPErrPktChk = FALSE;
		}
	}
	else
	{
		pApCliEntry->OpenWEPErrPktCnt++;
		if(pApCliEntry->OpenWEPErrPktCnt >= OPENWEP_ERRPKT_MAX_COUNT)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("(%s:%d): Stop Open WEP check!\n",__func__,__LINE__));
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pApCliEntry->OpenWEPErrPktChk = FALSE;
		}
		if(isMCPkt) pApCliEntry->OpenWEPErrMCPktCnt++;
	}
#endif//MULTI_APCLI_SUPPORT
}
#endif /* APCLI_SUPPORT */

