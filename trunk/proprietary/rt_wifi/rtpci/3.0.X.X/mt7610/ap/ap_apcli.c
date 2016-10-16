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
       APHandleRxDoneInterrupt() by using FromWhichBSSID = pEntry->apidx;
       Or FromWhichBSSID = BSS0;

    4. ApCli index (0) of different tx packet is assigned in
       MBSS_VirtualIF_PacketSend() by using RTMP_SET_PACKET_NET_DEVICE_MBSSID()
    5. ApCli index (0) of different interface is got in APHardTransmit() by using
       RTMP_GET_PACKET_IF()

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



/* --------------------------------- Public -------------------------------- */
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
VOID RT28xx_ApCli_Close(
	IN PRTMP_ADAPTER ad_p)
{
	UINT index;


	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (ad_p->ApCfg.ApCliTab[index].dev)
			RtmpOSNetDevClose(ad_p->ApCfg.ApCliTab[index].dev);
	}

}




/* --------------------------------- Private -------------------------------- */
INT ApCliIfLookUp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	SHORT i;
	SHORT ifIndex = -1;

	do
	{
		for(i = 0; i < MAX_APCLI_NUM; i++)
		{
			if(	MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].CurrentAddress, pAddr))
			{
				ifIndex = i;
				DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCliIfIndex = %d\n", __FUNCTION__, ifIndex));
				break;
			}
		}
	} while (FALSE);

	return ifIndex;
}

BOOLEAN isValidApCliIf(
	SHORT ifIndex)
{
	if((ifIndex >= 0) && (ifIndex < MAX_APCLI_NUM))
		return TRUE;
	else
		return FALSE;
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
    IN	PRTMP_ADAPTER	pAd, 
    IN OUT PHEADER_802_11 pHdr80211, 
    IN UCHAR SubType, 
    IN UCHAR ToDs, 
    IN PUCHAR pDA, 
    IN PUCHAR pBssid,
    IN USHORT ifIndex)
{
    NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
    pHdr80211->FC.Type = BTYPE_MGMT;
    pHdr80211->FC.SubType = SubType;
    pHdr80211->FC.ToDs = ToDs;
    COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
    COPY_MAC_ADDR(pHdr80211->Addr2, pAd->ApCfg.ApCliTab[ifIndex].CurrentAddress);
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
	PAPCLI_STRUCT pApCliEntry = NULL;
	HT_CAPABILITY_IE *aux_ht_cap;
	RT_HT_CAPABILITY *rt_ht_cap = &pAd->CommonCfg.DesiredHtPhy;

	if (IfIndex >= MAX_APCLI_NUM)
		return FALSE;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];

	/* If use AMSDU, set flag. */
	if (rt_ht_cap->AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
		CLIENT_STATUS_SET_FLAG(pApCliEntry, fCLIENT_STATUS_AMSDU_INUSED);
	/* Save Peer Capability */
	if (pHtCapability->HtCapInfo.ShortGIfor20)
		CLIENT_STATUS_SET_FLAG(pApCliEntry, fCLIENT_STATUS_SGI20_CAPABLE);
	if (pHtCapability->HtCapInfo.ShortGIfor40)
		CLIENT_STATUS_SET_FLAG(pApCliEntry, fCLIENT_STATUS_SGI40_CAPABLE);
	if (pHtCapability->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(pApCliEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
	if (pHtCapability->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(pApCliEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
	if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)
		CLIENT_STATUS_SET_FLAG(pApCliEntry, fCLIENT_STATUS_RDG_CAPABLE);
	pApCliEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;


	aux_ht_cap = &pApCliEntry->ApCliMlmeAux.HtCapability;
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
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & rt_ht_cap->ChannelWidth;
	aux_ht_cap->HtCapInfo.GF =  pHtCapability->HtCapInfo.GF & rt_ht_cap->GF;

	/* Send Assoc Req with my HT capability. */
	aux_ht_cap->HtCapInfo.AMsduSize =  rt_ht_cap->AmsduSize;
	aux_ht_cap->HtCapInfo.MimoPs = pHtCapability->HtCapInfo.MimoPs;
	aux_ht_cap->HtCapInfo.ShortGIfor20 =  (rt_ht_cap->ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	aux_ht_cap->HtCapInfo.ShortGIfor40 =  (rt_ht_cap->ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	aux_ht_cap->HtCapInfo.TxSTBC =  (rt_ht_cap->TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	aux_ht_cap->HtCapInfo.RxSTBC =  (rt_ht_cap->RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);
	aux_ht_cap->HtCapParm.MaxRAmpduFactor = rt_ht_cap->MaxRAmpduFactor;
	aux_ht_cap->HtCapParm.MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	aux_ht_cap->ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	if (pAd->CommonCfg.bRdg)
	{
		aux_ht_cap->ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
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
BOOLEAN ApCliLinkUp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex)
{
	BOOLEAN result = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	APCLI_STRUCT *wdev = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

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
				struct net_device *pNetDev;
				struct net *net= &init_net;

				for_each_netdev(net, pNetDev)
				{
					if (pNetDev->priv_flags == IFF_EBRIDGE)
					{
							COPY_MAC_ADDR(pAd->ApCfg.BridgeAddress, pNetDev->dev_addr);
					}
						RTMPRepeaterInsertInvaildMacEntry(pAd, pNetDev->dev_addr);
				}
			}

			if (ifIndex >= 64)
			{
				CliIdx = ((ifIndex - 64) % 16);
				ifIndex = ((ifIndex - 64) / 16);

					pMacEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].OriginalAddress);
				if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
					pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli = FALSE;
				else
					pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli = TRUE;

				pMacEntry = NULL;
			}
			}
#endif /* MAC_REPEATER_SUPPORT */

				wdev = &pAd->ApCfg.ApCliTab[ifIndex];

				printk("!!! APCLI LINK UP - IF(%s%d) AuthMode(%d)=%s, WepStatus(%d)=%s !!!\n", 
											INF_APCLI_DEV_NAME, ifIndex, 
											wdev->AuthMode, GetAuthMode(wdev->AuthMode),
											wdev->WepStatus, GetEncryptType(wdev->WepStatus));
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : APCLI LINK UP - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
				result = FALSE;
				break;
			}

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			DBGPRINT(RT_DEBUG_ERROR, ("(%s) ifIndex = %d, CliIdx = %d !!!\n", __FUNCTION__, ifIndex, CliIdx));
#endif /* MAC_REPEATER_SUPPORT */

		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		wdev = &pAd->ApCfg.ApCliTab[ifIndex];

#ifndef APCLI_CONNECTION_TRIAL
		/* Sanity check: This link had existed. */
		if ((pApCliEntry->Valid)
#ifdef MAC_REPEATER_SUPPORT
			 && (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : This link had existed - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
			result = FALSE;
			break;
		}
#endif /* APCLI_CONNECTION_TRIAL */	
	
		/* Insert the Remote AP to our MacTable. */
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
			pMacEntry = RTMPInsertRepeaterMacEntry(pAd, (PUCHAR)(pApCliEntry->ApCliMlmeAux.Bssid), (ifIndex + MIN_NET_DEVICE_FOR_APCLI), CliIdx, TRUE);
		else
#endif /* MAC_REPEATER_SUPPORT */
		pMacEntry = MacTableInsertEntry(pAd, (PUCHAR)(pApCliEntry->ApCliMlmeAux.Bssid), (ifIndex + MIN_NET_DEVICE_FOR_APCLI), OPMODE_AP, TRUE);

		if (pMacEntry)
		{
			UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES];
			PUCHAR pRates = Rates;
			UCHAR RatesLen;
			UCHAR MaxSupportedRate = 0;

			pMacEntry->Sst = SST_ASSOC;
			
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID = pMacEntry->Aid;
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = TRUE;
				pMacEntry->bReptCli = TRUE;
				pMacEntry->MatchReptCliIdx = CliIdx;
				pMacEntry->ReptCliIdleCount = 0;
				COPY_MAC_ADDR(pMacEntry->ReptCliAddr, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli == TRUE)
					pMacEntry->bReptEthCli = TRUE;
				else
					pMacEntry->bReptEthCli = FALSE;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pApCliEntry->Valid = TRUE;
				pApCliEntry->MacTabWCID = pMacEntry->Aid;
#ifdef MAC_REPEATER_SUPPORT
				pMacEntry->bReptCli = FALSE;
#endif /* MAC_REPEATER_SUPPORT */

				COPY_MAC_ADDR(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID), pApCliEntry->ApCliMlmeAux.Bssid);
				pApCliEntry->SsidLen = pApCliEntry->ApCliMlmeAux.SsidLen;
				NdisMoveMemory(pApCliEntry->Ssid, pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->SsidLen);
			}

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
			/*
				If ApCli connects to different AP, ApCli couldn't send EAPOL_Start for WpaSupplicant.
			*/
			if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) &&
				(NdisEqualMemory(pAd->MlmeAux.Bssid, pApCliEntry->LastBssid, MAC_ADDR_LEN) == FALSE) &&
				(pApCliEntry->bLostAp == TRUE))
			{
				pApCliEntry->bLostAp = FALSE;
			}

			COPY_MAC_ADDR(pApCliEntry->LastBssid, pAd->MlmeAux.Bssid);
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */


			if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				pMacEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
			{

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
				if (pApCliEntry->WpaSupplicantUP &&
					(pMacEntry->WepStatus == Ndis802_11WEPEnabled) &&
					(pApCliEntry->IEEE8021X == TRUE))
						pMacEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
				else
#endif /*APCLI_WPA_SUPPLICANT_SUPPORT*/

				pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef MAC_REPEATER_SUPPORT
				if (CliIdx != 0xFF)
					pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliConnectState = 2;
#endif /* MAC_REPEATER_SUPPORT */
			}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE) &&
#ifdef MAC_REPEATER_SUPPORT
				(CliIdx == 0xFF) &&
#endif /* MAC_REPEATER_SUPPORT */
				(pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("ApCli auto connected: ApCliLinkUp()\n"));
				pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
			}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			NdisGetSystemUpTime(&pApCliEntry->ApCliLinkUpTime);
			/* Store appropriate RSN_IE for WPA SM negotiation later */
			/* If WPAPSK/WPA2SPK mix mode, driver just stores either WPAPSK or WPA2PSK */
			/* RSNIE. It depends on the AP-Client's authentication mode to store the corresponding RSNIE. */
			if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA) && (pApCliEntry->ApCliMlmeAux.VarIELen != 0))
			{
				PUCHAR              pVIE;
				UCHAR               len;
				PEID_STRUCT         pEid;

				pVIE = pApCliEntry->ApCliMlmeAux.VarIEs;
				len	 = pApCliEntry->ApCliMlmeAux.VarIELen;

				while (len > 0)
				{
					pEid = (PEID_STRUCT) pVIE;	
					/* For WPA/WPAPSK */
					if ((pEid->Eid == IE_WPA) && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4)) 
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA || pMacEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
					{
						NdisMoveMemory(pMacEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pMacEntry->RSNIE_Len = (pEid->Len + 2);							
						DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp: Store RSN_IE for WPA SM negotiation \n"));
					}
					/* For WPA2/WPA2PSK */
					else if ((pEid->Eid == IE_RSN) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA2 || pMacEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
					{
						NdisMoveMemory(pMacEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pMacEntry->RSNIE_Len = (pEid->Len + 2);	
						DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp: Store RSN_IE for WPA2 SM negotiation \n"));
					}

					pVIE += (pEid->Len + 2);
					len  -= (pEid->Len + 2);
				}							
			}

			if (pMacEntry->RSNIE_Len == 0)
			{			
				DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp: root-AP has no RSN_IE \n"));			
			}
			else
			{
				hex_dump("The RSN_IE of root-AP", pMacEntry->RSN_IE, pMacEntry->RSNIE_Len);
			}		

			SupportRate(pApCliEntry->ApCliMlmeAux.SupRate, pApCliEntry->ApCliMlmeAux.SupRateLen, pApCliEntry->ApCliMlmeAux.ExtRate,
				pApCliEntry->ApCliMlmeAux.ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);

			pMacEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxTxRate, MaxSupportedRate);
			pMacEntry->RateLen = RatesLen;
			set_entry_phy_cfg(pAd, pMacEntry);

			pMacEntry->CapabilityInfo = pApCliEntry->ApCliMlmeAux.CapabilityInfo;

			if ((wdev->WepStatus == Ndis802_11WEPEnabled)
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
				&& (pApCliEntry->WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */
			)
			{			
				PCIPHER_KEY pKey; 			
				INT 		idx, BssIdx;

#ifdef RLT_MAC
				BssIdx = APCLI_BSSID_IDX + ifIndex;
#else
				BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
#endif
			
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
												pMacEntry->Aid, 
												SHAREDKEYTABLE);
						}
					}	
				}    		   		  		   					
			}

#ifdef DOT11_N_SUPPORT
			/* If this Entry supports 802.11n, upgrade to HT rate. */
			if (pApCliEntry->ApCliMlmeAux.HtCapabilityLen != 0)
			{
				PHT_CAPABILITY_IE pHtCapability = (PHT_CAPABILITY_IE)&pApCliEntry->ApCliMlmeAux.HtCapability;

				ht_mode_adjust(pAd, pMacEntry, pHtCapability, &pAd->CommonCfg.DesiredHtPhy);

				/* find max fixed rate */
				pMacEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0],
																	&pHtCapability->MCSSet[0]);
				 
				if (pAd->ApCfg.ApCliTab[ifIndex].DesiredTransmitSetting.field.MCS != MCS_AUTO)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("IF-%s%d : Desired MCS = %d\n", INF_APCLI_DEV_NAME, ifIndex,
						wdev->DesiredTransmitSetting.field.MCS));

					set_ht_fixed_mcs(pAd, pMacEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pMacEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
				if (pHtCapability->HtCapParm.MpduDensity < 5)
					pMacEntry->MpduDensity = 5;
				else
					pMacEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
				pMacEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
				pMacEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
				pMacEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;				
				pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
				if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);
				if (pHtCapability->HtCapInfo.ShortGIfor20)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI20_CAPABLE);
				if (pHtCapability->HtCapInfo.ShortGIfor40)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI40_CAPABLE);
				if (pHtCapability->HtCapInfo.TxSTBC)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
				if (pHtCapability->HtCapInfo.RxSTBC)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
				if (pHtCapability->ExtHtCapInfo.PlusHTC)				
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HTC_CAPABLE);
				if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)				
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE);	
				if (pHtCapability->ExtHtCapInfo.MCSFeedback == 0x03)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);		
				NdisMoveMemory(&pMacEntry->HTCapability, &pApCliEntry->ApCliMlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
				NdisMoveMemory(pMacEntry->HTCapability.MCSSet, pApCliEntry->RxMcsSet, 16);
			}
			else
			{
				pAd->MacTab.fAnyStationIsLegacy = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp - MaxSupRate=%d Mbps\n",
								  RateIdToMbps[pMacEntry->MaxSupportedRate]));
			}				

#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && pApCliEntry->ApCliMlmeAux.vht_cap_len &&  pApCliEntry->ApCliMlmeAux.vht_op_len)
                         vht_mode_adjust(pAd, pMacEntry, &(pApCliEntry->ApCliMlmeAux.vht_cap), &(pApCliEntry->ApCliMlmeAux.vht_op));
#endif /* DOT11_VHT_AC */

			pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
			pMacEntry->CurrTxRate = pMacEntry->MaxSupportedRate;
			
			RTMPSetSupportMCS(pAd,
							OPMODE_AP,
							pMacEntry,
							pApCliEntry->ApCliMlmeAux.SupRate,
							pApCliEntry->ApCliMlmeAux.SupRateLen,
							pApCliEntry->ApCliMlmeAux.ExtRate,
							pApCliEntry->ApCliMlmeAux.ExtRateLen,
#ifdef DOT11_VHT_AC
							pApCliEntry->ApCliMlmeAux.vht_cap_len,
							&pApCliEntry->ApCliMlmeAux.vht_cap,
#endif /* DOT11_VHT_AC */
							&pApCliEntry->ApCliMlmeAux.HtCapability,
							pApCliEntry->ApCliMlmeAux.HtCapabilityLen);

#ifdef DISANLE_VHT80_256_QAM
			/*
				To check SupportVHTMCS for APCLI again.
			*/
			if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && pApCliEntry->ApCliMlmeAux.vht_cap_len &&  pApCliEntry->ApCliMlmeAux.vht_op_len)
			{
				if (pApCliEntry->ApCliMlmeAux.vht_op.vht_op_info.ch_width != VHT_BW_80)
				{
					pMacEntry->SupportVHTMCS[8] = TRUE;
					if (pMacEntry->MaxHTPhyMode.field.BW == BW_40)
					{
						pMacEntry->SupportVHTMCS[9] = TRUE;
					}
				}
			}
#endif /* DISANLE_VHT80_256_QAM */
			
			if (pAd->ApCfg.ApCliTab[ifIndex].bAutoTxRateSwitch == FALSE)
			{
				pMacEntry->bAutoTxRateSwitch = FALSE;
				/* If the legacy mode is set, overwrite the transmit setting of this entry. */
				RTMPUpdateLegacyTxSetting((UCHAR)pAd->ApCfg.ApCliTab[ifIndex].DesiredTransmitSetting.field.FixedTxMode, pMacEntry);	
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
			
			/* It had been set in APStartUp. Don't set again. */
			/*AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm); */
			
			/* set this entry WMM capable or not */
			if ((pApCliEntry->ApCliMlmeAux.APEdcaParm.bValid)
#ifdef DOT11_N_SUPPORT
				|| IS_HT_STA(pMacEntry)
#endif /* DOT11_N_SUPPORT */
				)
			{
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}

			if (pAd->CommonCfg.bAggregationCapable)
			{
				if ((pAd->CommonCfg.bPiggyBackCapable) && (pApCliEntry->ApCliMlmeAux.APRalinkIe & 0x00000003) == 3)
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_PIGGYBACK_INUSED);
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
					RTMPSetPiggyBack(pAd, TRUE);
					DBGPRINT(RT_DEBUG_TRACE, ("Turn on Piggy-Back\n"));
				}
				else if (pApCliEntry->ApCliMlmeAux.APRalinkIe & 0x00000001)
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
					DBGPRINT(RT_DEBUG_TRACE, ("Ralink Aggregation\n"));
				}
			}

			/* Set falg to identify if peer AP is Ralink chipset */
			if (pApCliEntry->ApCliMlmeAux.APRalinkIe != 0x0)
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);
			else
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);

			NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);
			/* set the apcli interface be valid. */
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->Valid = TRUE;
			result = TRUE;

#ifdef RLT_MAC
#ifdef MAC_REPEATER_SUPPORT
			if (
				(CliIdx == 0xFF)
#ifdef APCLI_CONNECTION_TRIAL
				&& (pApCliEntry->TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
			)
#endif /* MAC_REPEATER_SUPPORT */
			AsicSetApCliBssid(pAd, pApCliEntry->ApCliMlmeAux.Bssid, ifIndex);
#endif /* RLT_MAC */
			pAd->ApCfg.ApCliInfRunned++;

			break;
		}
		result = FALSE;

	} while(FALSE);

	if (result == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" (%s) alloc mac entry fail!!!\n", __FUNCTION__));
		return result;
	}

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
	/*
		When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP, 
		WpaSupplicant would not send EapolStart to AP after STA re-connect to AP again.
		In this case, driver would send EapolStart to AP.
	*/
				if ((pMacEntry->AuthMode == Ndis802_11AuthModeWPA2) &&
					(NdisEqualMemory(pAd->MlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].LastBssid, MAC_ADDR_LEN)) &&
					(pAd->ApCfg.ApCliTab[ifIndex].bLostAp == TRUE))
				{
					ApcliWpaSendEapolStart(pAd, pApCliEntry->ApCliMlmeAux.Bssid,pMacEntry, pApCliEntry);
					pApCliEntry->bLostAp = FALSE;
				}
#endif /*APCLI_WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_AP_SUPPORT
    /* WSC initial connect to AP, jump to Wsc start action and set the correct parameters */
	if ((result == TRUE) && 
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode == WSC_ENROLLEE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
	{
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscState = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscStatus = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].WscControl.EntryAddr, MAC_ADDR_LEN);        
		NdisMoveMemory(pAd->ApCfg.ApCliTab[ifIndex].WscControl.EntryAddr, pApCliEntry->ApCliMlmeAux.Bssid, MAC_ADDR_LEN);
		WscSendEapolStart(pAd, pMacEntry->Addr, AP_MODE);
	}
    else
    {
        WscStop(pAd, TRUE, &pApCliEntry->WscControl);
    }
#endif /* WSC_AP_SUPPORT */

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
VOID ApCliLinkDown(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex)
{
	PAPCLI_STRUCT pApCliEntry = NULL;
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
			CliIdx = ((ifIndex - 64) % 16);
			ifIndex = ((ifIndex - 64) / 16);
			printk("!!! REPEATER CLI LINK DOWN - IF(%s%d) Cli %d !!!\n", INF_APCLI_DEV_NAME, ifIndex, CliIdx);
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
			printk("!!! APCLI LINK DOWN - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("!!! ERROR : APCLI LINK DOWN - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
		return;
	}
    	
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if ((pApCliEntry->Valid == FALSE)
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

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

	MacTableDeleteEntry(pAd, MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID));

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pApCliEntry->Valid = FALSE;	/* This link doesn't associated with any remote-AP */
	pAd->ApCfg.ApCliTab[ifIndex].bPeerExist = FALSE;

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
	if (pApCliEntry->WpaSupplicantUP) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] Send RT_DISASSOC_EVENT_FLAG.\n", __FUNCTION__, ifIndex));
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
	}   
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */  
}

/* 
    ==========================================================================
    Description:
        APCLI Interface Up.
    ==========================================================================
 */
VOID ApCliIfUp(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR ifIndex;
	PAPCLI_STRUCT pApCliEntry;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCurrState = NULL;
#endif /* APCLI_CONNECTION_TRIAL */

	/* Reset is in progress, stop immediately */
	if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
		 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
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
		if (
			APCLI_IF_UP_CHECK(pAd, ifIndex) 
			&& (pApCliEntry->Enable == TRUE)
			&& (pApCliEntry->Valid == FALSE)
#ifdef	APCLI_CONNECTION_TRIAL
			&& (ifIndex == 0)
#endif /* APCLI_CONNECTION_TRIAL */
		)
		{
			if (IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE))
			{	
				if (pApCliEntry->bPeerExist == TRUE)
				{
					/* Got peer's beacon; change to normal mode */
					pAd->Dot11_H.RDCount = pAd->Dot11_H.ChMovingTime;
					DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfUp - PeerExist\n"));
				}
				else
					DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfUp - Stop probing while Radar state is silent\n"));
				
				continue;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startup.\n", __FUNCTION__, ifIndex));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex);
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->bPeerExist = FALSE;
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
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) Enqueue APCLI_CTRL_TRIAL_CONNECT\n", __func__));
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
VOID ApCliIfDown(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR ifIndex;
	PAPCLI_STRUCT pApCliEntry;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx, idx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
		{
		for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
		{
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable)
			{
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL,
									(64 + MAX_EXT_MAC_ADDR_SIZE*ifIndex + CliIdx));
					RTMP_MLME_HANDLER(pAd);
					RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
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
VOID ApCliIfMonitor(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR index;
	PAPCLI_STRUCT pApCliEntry;
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

				if ((pReptCliEntry->CliEnable) && (pReptCliEntry->CliValid))
				{
					Wcid = pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].MacTabWCID;
					if (!VALID_WCID(Wcid))
						continue;
					pMacEntry = &pAd->MacTab.Content[Wcid];

					if ((pMacEntry->PortSecured != WPA_802_1X_PORT_SECURED) &&
						RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pReptCliEntry->CliTriggerTime + (5 * OS_HZ))))
					{
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*index + CliIdx));
						RTMP_MLME_HANDLER(pAd);
						RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */

		if (pApCliEntry->Valid == TRUE)
		{
			Wcid = pAd->ApCfg.ApCliTab[index].MacTabWCID;
			if (!VALID_WCID(Wcid))
				continue;
			pMacEntry = &pAd->MacTab.Content[Wcid];
 
			if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				&& (pMacEntry->PortSecured != WPA_802_1X_PORT_SECURED)
				&& (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pApCliEntry->ApCliLinkUpTime + (30 * OS_HZ)))))
				bForceBrocken = TRUE;
 
			if (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pApCliEntry->ApCliRcvBeaconTime + (12 * OS_HZ))))
			{
				printk("ApCliIfMonitor: IF(%s%d) - no Beacon is received from Root-AP.\n", INF_APCLI_DEV_NAME, index);
				bForceBrocken = TRUE;
			}

		}
		else
			continue;
 
		if (bForceBrocken == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfMonitor: Reconnect the Root-Ap again.\n"));

#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn)
			{
			for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
					if (pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].CliEnable)
				{
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*index + CliIdx));
					RTMP_MLME_HANDLER(pAd);
						RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, index);
			RTMP_MLME_HANDLER(pAd);
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
	if (pFrame->Hdr.FC.Type == BTYPE_DATA)
	{		
#ifdef WSC_AP_SUPPORT    
        /*WSC EAPOL PACKET */
        pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
        if (pEntry && IS_ENTRY_APCLI(pEntry) && pAd->ApCfg.ApCliTab[pEntry->apidx].WscControl.WscConfMode == WSC_ENROLLEE)
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
	else if (pFrame->Hdr.FC.Type == BTYPE_MGMT) 		
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
				{
					*MsgType = MT2_ACT_INVALID;
				}
				else
				{
					*MsgType = (pFrame->Octet[0]&0x7F);
				}
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
	if (pFrame->Hdr.FC.Type == BTYPE_MGMT) 		
	{
		switch (pFrame->Hdr.FC.SubType) 
		{
			/* Beacon must be processed be AP Sync state machine. */
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
	CHAR          IeType, *Ptr;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PEID_STRUCT   pEid;
	ULONG         Length = 0;
    
	*pNewExtChannelOffset = 0xff;
	*pHtCapabilityLen = 0;
	*pAddHtInfoLen = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	Ptr = (CHAR *) pFrame->Octet;
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
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): fail - wrong SupportedRates IE\n", __FUNCTION__));
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
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_HT_CAP\n", __FUNCTION__));
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
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_ADD_HT\n", __FUNCTION__));
				}
				break;
			case IE_SECONDARY_CH_OFFSET:
				if (pEid->Len == 1)
				{
					*pNewExtChannelOffset = pEid->Octet[0];
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_SECONDARY_CH_OFFSET\n", __FUNCTION__));
				}
				break;
#ifdef DOT11_VHT_AC

			case IE_VHT_CAP:
				if (pEid->Len == sizeof(VHT_CAP_IE)) {
					NdisMoveMemory(&ie_list->vht_cap, pEid->Octet, sizeof(VHT_CAP_IE));
					ie_list->vht_cap_len = sizeof(VHT_CAP_IE);
				} else {
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_VHT_CAP\n", __FUNCTION__));
				}
				break;

			case IE_VHT_OP:
				if (pEid->Len == sizeof(VHT_OP_IE)) {
					NdisMoveMemory(&ie_list->vht_op, pEid->Octet, sizeof(VHT_OP_IE));
					ie_list->vht_op_len = sizeof(VHT_OP_IE);
				}else {
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_VHT_OP\n", __FUNCTION__));
				}
				break;
			
#endif /* DOT11_VHT_AC */					
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
					DBGPRINT(RT_DEBUG_TRACE, ("%s():ignore unrecognized EID = %d\n", __FUNCTION__, pEid->Eid));
					break;
		}

		Length = Length + 2 + pEid->Len; 
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	}

	return TRUE;
}


MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid,
	IN PUCHAR pAddrs)
{
	ULONG ApCliIndex;
	PMAC_TABLE_ENTRY pCurEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;

	if (wcid <=0 || wcid >= MAX_LEN_OF_MAC_TABLE )
		return NULL;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	do
	{
		pCurEntry = &pAd->MacTab.Content[wcid];

		ApCliIndex = 0xff;
		if ((pCurEntry) && IS_ENTRY_APCLI(pCurEntry))
		{
			ApCliIndex = pCurEntry->MatchAPCLITabIdx;
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
static inline BOOLEAN ValidApCliEntry(
	IN PRTMP_ADAPTER pAd,
	IN INT apCliIdx)
{
	BOOLEAN result;
	PMAC_TABLE_ENTRY pMacEntry;
	APCLI_STRUCT *pApCliEntry;
	UINT32 MaxWcidNum = MAX_LEN_OF_MAC_TABLE;

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

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
			MaxWcidNum = MAX_MAC_TABLE_SIZE_WITH_REPEATER;
#endif /* MAC_REPEATER_SUPPORT */

		if ((pApCliEntry->MacTabWCID <= 0) 
			|| (pApCliEntry->MacTabWCID >= MaxWcidNum))
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

BOOLEAN ApCliAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR		*pWcid)
{
	UCHAR apCliIdx;
	BOOLEAN	allowed;

	/*DBGPRINT(RT_DEBUG_TRACE, ("ApCliAllowToSendPacket():Packet to ApCli interface!\n")); */
	apCliIdx = RTMP_GET_PACKET_NET_DEVICE(pPacket) - MIN_NET_DEVICE_FOR_APCLI;
	if (ValidApCliEntry(pAd, apCliIdx))
	{
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn == TRUE)
		{
			PUCHAR pSrcBufVA = NULL;
			PACKET_INFO PacketInfo;
			UINT SrcBufLen;
			PMAC_TABLE_ENTRY pEntry = NULL;
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

			RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, (pSrcBufVA + ETH_LENGTH_OF_ADDRESS));

			if (pReptEntry)
			{
				*pWcid = pReptEntry->MacTabWCID;
				return TRUE;
			}
			else
			{
				if (RTMPRepeaterVaildMacEntry(pAd, pSrcBufVA + MAC_ADDR_LEN))
				{
					pEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[apCliIdx].MacTabWCID];

					if ((pEntry) && (pEntry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
						RTMPInsertRepeaterEntry(pAd, apCliIdx, (pSrcBufVA + ETH_LENGTH_OF_ADDRESS));
						DBGPRINT(RT_DEBUG_ERROR, (" Receive trigger packet !!!\n"));
				return FALSE;
			}
			}
			}

				*pWcid = pAd->ApCfg.ApCliTab[apCliIdx].MacTabWCID;
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
		*pWcid = pAd->ApCfg.ApCliTab[apCliIdx].MacTabWCID;
		
		allowed = TRUE;
	}
	else
	{
		allowed = FALSE;
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
BOOLEAN 	ApCliValidateRSNIE(
	IN		PRTMP_ADAPTER	pAd, 
	IN 		PEID_STRUCT    	pEid_ptr,
	IN		USHORT			eid_len,
	IN		USHORT			idx)
{
	PUCHAR              pVIE;
	PUCHAR				pTmp;
	UCHAR         		len;
	PEID_STRUCT         pEid;			
	CIPHER_SUITE		WPA;			/* AP announced WPA cipher suite */
	CIPHER_SUITE		WPA2;			/* AP announced WPA2 cipher suite */
	USHORT				Count;
	UCHAR               Sanity;	 
	PAPCLI_STRUCT   	pApCliEntry = NULL;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	NDIS_802_11_AUTHENTICATION_MODE TmpAuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA_AuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA_AuthModeAux;
	NDIS_802_11_AUTHENTICATION_MODE WPA2_AuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA2_AuthModeAux;

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
						WPA.GroupCipher = Ndis802_11Encryption1Enabled;
						break;
					case 2:
						WPA.GroupCipher = Ndis802_11Encryption2Enabled;
						break;
					case 4:
						WPA.GroupCipher = Ndis802_11Encryption3Enabled;
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
							TmpCipher = Ndis802_11Encryption1Enabled;
							break;
						case 2:
							TmpCipher = Ndis802_11Encryption2Enabled;							
							break;
						case 4:
							TmpCipher = Ndis802_11Encryption3Enabled;						
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

				DBGPRINT(RT_DEBUG_TRACE, ("ApCliValidateRSNIE - RSN-WPA1 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
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
					DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - RSN Version isn't 1(%d) \n", pRsnHeader->Version));
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
						WPA2.GroupCipher = Ndis802_11Encryption1Enabled;
						break;
					case 2:
						WPA2.GroupCipher = Ndis802_11Encryption2Enabled;
						break;
					case 4:
						WPA2.GroupCipher = Ndis802_11Encryption3Enabled;
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
							TmpCipher = Ndis802_11Encryption1Enabled;
							break;
						case 2:
							TmpCipher = Ndis802_11Encryption2Enabled;							
							break;
						case 4:
							TmpCipher = Ndis802_11Encryption3Enabled;							
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

				DBGPRINT(RT_DEBUG_TRACE, ("ApCliValidateRSNIE - RSN-WPA2 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
									(WPA2.bMixMode ? "Mix" : GetEncryptType(WPA2.PairCipher)), GetEncryptType(WPA2.GroupCipher),
									GetAuthMode(WPA2_AuthMode)));

				Sanity |= 0x2;
				break; /* End of case IE_RSN */
			default:
					DBGPRINT(RT_DEBUG_WARN, ("ApCliValidateRSNIE - Unknown pEid->Eid(%d) \n", pEid->Eid));
				break;
		}

		/* skip this Eid */
		pVIE += (pEid->Len + 2);
		len  -= (pEid->Len + 2);
	
	}

	/* 2. Validate this RSNIE with mine */
	pApCliEntry = &pAd->ApCfg.ApCliTab[idx];

	/* Peer AP doesn't include WPA/WPA2 capable */
	if (Sanity == 0) 
	{
		/* Check the authenticaton mode */		
		if (pApCliEntry->AuthMode >= Ndis802_11AuthModeWPA)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s - The authentication mode doesn't match \n", __FUNCTION__));
			return FALSE;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s - The pre-RSNA authentication mode is used. \n", __FUNCTION__));
			return TRUE;
		}
	}
	else if (pApCliEntry->AuthMode < Ndis802_11AuthModeWPA)
	{
		/* Peer AP has RSN capability, but our AP-Client is pre-RSNA. Discard this */
		DBGPRINT(RT_DEBUG_ERROR, ("%s - The authentication mode doesn't match. AP is WPA security but APCLI is not. \n", __FUNCTION__));
		return FALSE;
	}


	/* Recovery user-defined cipher suite */
	pApCliEntry->PairCipher  = pApCliEntry->WepStatus;
	pApCliEntry->GroupCipher = pApCliEntry->WepStatus;
	pApCliEntry->bMixCipher  = FALSE;

	Sanity = 0;	
	
	/* Check AuthMode and WPA_AuthModeAux for matching, in case AP support dual-AuthMode */
	/* WPAPSK */
	if ((WPA_AuthMode == pApCliEntry->AuthMode) || 
		((WPA_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA_AuthModeAux == pApCliEntry->AuthMode)))
	{
		/* Check cipher suite, AP must have more secured cipher than station setting */
		if (WPA.bMixMode == FALSE)
		{
			if (pApCliEntry->WepStatus != WPA.GroupCipher)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate cipher suite error \n"));
				return FALSE;
			}
		}

		/* check group cipher */
		if (pApCliEntry->WepStatus < WPA.GroupCipher)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate group cipher error \n"));
			return FALSE;
		}

		/*
			check pairwise cipher, skip if none matched
				If profile set to AES, let it pass without question.
				If profile set to TKIP, we must find one mateched
		*/
		if ((pApCliEntry->WepStatus == Ndis802_11Encryption2Enabled) && 
			(pApCliEntry->WepStatus != WPA.PairCipher) && 
			(pApCliEntry->WepStatus != WPA.PairCipherAux))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate pairwise cipher error \n"));
			return FALSE;
		}	

		Sanity |= 0x1;
	}
	/* WPA2PSK */
	else if ((WPA2_AuthMode == pApCliEntry->AuthMode) || 
			 ((WPA2_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA2_AuthModeAux == pApCliEntry->AuthMode)))
	{
		/* Check cipher suite, AP must have more secured cipher than station setting */
		if (WPA2.bMixMode == FALSE)
		{
			if (pApCliEntry->WepStatus != WPA2.GroupCipher)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate cipher suite error \n"));
				return FALSE;
			}
		}

		/* check group cipher */
		if (pApCliEntry->WepStatus < WPA2.GroupCipher)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate group cipher error \n"));
			return FALSE;
		}

		/*
			check pairwise cipher, skip if none matched
				If profile set to AES, let it pass without question.
				If profile set to TKIP, we must find one mateched
		*/
		if ((pApCliEntry->WepStatus == Ndis802_11Encryption2Enabled) && 
			(pApCliEntry->WepStatus != WPA2.PairCipher) && 
			(pApCliEntry->WepStatus != WPA2.PairCipherAux))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate pairwise cipher error \n"));
			return FALSE;
		}

		Sanity |= 0x2;
	}

	if (Sanity == 0) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - Validate RSIE Failure \n"));
		return FALSE;
	}

	/*Re-assign pairwise-cipher and group-cipher. Re-build RSNIE. */
	if ((pApCliEntry->AuthMode == Ndis802_11AuthModeWPA) || (pApCliEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
	{
		pApCliEntry->GroupCipher = WPA.GroupCipher;
			
		if (pApCliEntry->WepStatus == WPA.PairCipher)
			pApCliEntry->PairCipher = WPA.PairCipher;
		else if (WPA.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA.PairCipherAux;
		else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
			pApCliEntry->PairCipher = Ndis802_11Encryption2Enabled;			
	}
	else if ((pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		pApCliEntry->GroupCipher = WPA2.GroupCipher;
			
		if (pApCliEntry->WepStatus == WPA2.PairCipher)
			pApCliEntry->PairCipher = WPA2.PairCipher;
		else if (WPA2.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA2.PairCipherAux;
		else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
			pApCliEntry->PairCipher = Ndis802_11Encryption2Enabled;					
	}

	/* Set Mix cipher flag */
	if (pApCliEntry->PairCipher != pApCliEntry->GroupCipher)
	{
		pApCliEntry->bMixCipher = TRUE;	

		/* re-build RSNIE */
		/*RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI)); */
	}
	
	/* re-build RSNIE */
	RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI));
	
	return TRUE;	
}

/* 
	===================================================
	
	Description:	
		Check the validation of Authentication mode and Security type between 
		entries in the Apcli table and the inputs. 

	Arguments:
		pApcliTabEntry:
		WPA_AuthMode
		WPA_AuthModeAux
		WPA: WPA cipher suite

	Return Value:
		TRUE:	valid
		FALSE:	invalid

	Note:
	===================================================
*/
BOOLEAN ApCliValidateAuthEncryp(
	IN PAPCLI_STRUCT pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE WPA_AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE WPA_AuthModeAux,
	IN CIPHER_SUITE WPA)
{
	CIPHER_SUITE tempWPA;
	tempWPA.PairCipher = pApCliEntry->PairCipher;
	tempWPA.GroupCipher = pApCliEntry->GroupCipher;
	tempWPA.bMixMode = pApCliEntry->bMixCipher;
		
	// Recovery user-defined cipher suite
	pApCliEntry->PairCipher  = pApCliEntry->WepStatus;
	pApCliEntry->GroupCipher = pApCliEntry->WepStatus;
	pApCliEntry->bMixCipher  = FALSE;
	
	// Check AuthMode and WPA_AuthModeAux for matching, in case AP support dual-AuthMode
	// WPAPSK
	if (WPA_AuthMode == pApCliEntry->AuthMode || WPA_AuthModeAux == pApCliEntry->AuthMode)
	{
		// Check cipher suite, AP must have more secured cipher than station setting
		if (WPA.bMixMode == FALSE)
		{
			if (pApCliEntry->WepStatus != WPA.GroupCipher)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate cipher suite error \n"));
				return FALSE;
			}
		}

		// check group cipher
		if (pApCliEntry->WepStatus < WPA.GroupCipher)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate group cipher error \n"));
			return FALSE;
		}

		// check pairwise cipher, skip if none matched
		// If profile set to AES, let it pass without question.
		// If profile set to TKIP, we must find one mateched
		if ((pApCliEntry->WepStatus == Ndis802_11Encryption2Enabled) && 
			(pApCliEntry->WepStatus != WPA.PairCipher) && 
			(pApCliEntry->WepStatus != WPA.PairCipherAux))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate pairwise cipher error \n"));
			return FALSE;
		}	
	}

	/*
		restore previous cipher suite
	*/
	pApCliEntry->PairCipher = tempWPA.PairCipher;
	pApCliEntry->GroupCipher = tempWPA.GroupCipher;
	pApCliEntry->bMixCipher = tempWPA.bMixMode;
	return TRUE; 
}

BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN	RX_BLK			*pRxBlk,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			FromWhichBSSID)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
#ifndef MAC_APCLI_SUPPORT
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
#endif
	APCLI_STRUCT *pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	/* 
		It is possible to receive the multicast packet when in AP Client mode
		ex: broadcast from remote AP to AP-client, 
				addr1=ffffff, addr2=remote AP's bssid, addr3=sta4_mac_addr
	*/
																																								
	pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->MatchAPCLITabIdx];																											
					
	/* Filter out Bcast frame which AP relayed for us */
	/* Multicast packet send from AP1 , received by AP2 and send back to AP1, drop this frame */
	if (MAC_ADDR_EQUAL(pHeader->Addr3, pApCliEntry->CurrentAddress))
		return FALSE;	/* give up this frame */

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn)
	{
		pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pHeader->Addr3);
		if (pReptEntry)
			return FALSE;	/* give up this frame */
	}
#endif /* MAC_REPEATER_SUPPORT */

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;	/* give up this frame */
					
							
	/* skip the 802.11 header */
	pRxBlk->pData += LENGTH_802_11;
	pRxBlk->DataSize -= LENGTH_802_11;

#ifndef MAC_APCLI_SUPPORT
	/* Use software to decrypt the encrypted frame. */
	/* Because this received frame isn't my BSS frame, Asic passed to driver without decrypting it. */
	/* If receiving an "encrypted" unicast packet(its WEP bit as 1) and doesn't match my BSSID, it */
	/* pass to driver with "Decrypted" marked as 0 in RxD. */
	if ((pRxInfo->MyBss == 0) && (pRxInfo->Decrypted == 0) && (pHeader->FC.Wep == 1)) 
	{											
		if (RTMPSoftDecryptionAction(pAd, 
									 (PUCHAR)pHeader, 0, 
									 &pApCliEntry->SharedKey[pRxWI->RxWIKeyIndex], 
									 pRxBlk->pData, 
									 &(pRxBlk->DataSize)) == NDIS_STATUS_FAILURE)			
		{						
			return FALSE;  /* give up this frame */
		}
	}
#endif /* MAC_APCLI_SUPPORT */
	pRxInfo->MyBss = 1;				

#ifdef HDR_TRANS_SUPPORT
	if (pRxBlk->bHdrRxTrans)
		Indicate_Legacy_Packet_Hdr_Trns(pAd, pRxBlk, FromWhichBSSID);
	else
#endif /* HDR_TRANS_SUPPORT */
		Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);

	return TRUE;
}


VOID APCliInstallPairwiseKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	UCHAR	IfIdx;
	UINT8	BssIdx;

	IfIdx = pEntry->MatchAPCLITabIdx;

#ifdef RLT_MAC
	BssIdx = APCLI_BSSID_IDX + IfIdx;
#else /* RLT_MAC */
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + IfIdx;
#endif /* !RLT_MAC */

	WPAInstallPairwiseKey(pAd, 
						  BssIdx, 
						  pEntry, 
						  FALSE);	
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
#ifdef RLT_MAC
	UINT8	BssIdx;
#endif /* RLT_MAC */

	if (!pEntry || !IS_ENTRY_APCLI(pEntry))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : This Entry doesn't exist!!! \n", __FUNCTION__));		
		return FALSE;
	}

	IfIdx = pEntry->MatchAPCLITabIdx;

	if (pAd->ApCfg.ApCliTab[IfIdx].GroupCipher == Ndis802_11Encryption2Enabled && KeyLen >= LEN_TKIP_GTK)
	{
		GTK_len = LEN_TKIP_GTK;
	}
	else if (pAd->ApCfg.ApCliTab[IfIdx].GroupCipher == Ndis802_11Encryption3Enabled && 
			 KeyLen >= LEN_AES_GTK)
	{
		GTK_len = LEN_AES_GTK;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : GTK is invalid (GroupCipher=%d, DataLen=%d) !!! \n", 
								__FUNCTION__, pAd->ApCfg.ApCliTab[IfIdx].GroupCipher, KeyLen));		
		return FALSE;
	}
	
	/* Update GTK */
	/* set key material, TxMic and RxMic for WPAPSK */
	NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].GTK, pKey, GTK_len);
	pAd->ApCfg.ApCliTab[IfIdx].DefaultKeyId = DefaultKeyIdx;

	/* Update shared key table */
	NdisZeroMemory(&pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx], sizeof(CIPHER_KEY));  
	pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].KeyLen = GTK_len;
	NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].Key, pKey, LEN_TK);
	if (GTK_len == LEN_TKIP_GTK)
	{
		NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].RxMic, pKey + 16, LEN_TKIP_MIC);
		NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].TxMic, pKey + 24, LEN_TKIP_MIC);
	}

	/* Update Shared Key CipherAlg */
	pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_NONE;
	if (pAd->ApCfg.ApCliTab[IfIdx].GroupCipher == Ndis802_11Encryption2Enabled)
		pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_TKIP;
	else if (pAd->ApCfg.ApCliTab[IfIdx].GroupCipher == Ndis802_11Encryption3Enabled)
		pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_AES;

#ifdef RLT_MAC

	BssIdx = APCLI_BSSID_IDX + IfIdx;

	/* Update group key table and group key mode */
    AsicAddSharedKeyEntry(
				pAd, 
				BssIdx, 
				DefaultKeyIdx, 
				&pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx]);
#endif /* RLT_MAC */

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
VOID ApCliUpdateMlmeRate(
	IN PRTMP_ADAPTER pAd,
	IN USHORT ifIndex)
{
	UCHAR	MinimumRate;
	UCHAR	ProperMlmeRate; /*= RATE_54; */
	UCHAR	i, j, RateIdx = 12; /* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
	BOOLEAN	bMatch = FALSE;

	PAPCLI_STRUCT pApCliEntry = NULL;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	
	switch (pAd->CommonCfg.PhyMode) 
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
#ifdef DOT11_VHT_AC
		case (WMODE_A |WMODE_B | WMODE_G | WMODE_GN | WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
			if ((pApCliEntry->ApCliMlmeAux.SupRateLen == 4) &&
				(pApCliEntry->ApCliMlmeAux.ExtRateLen == 0))
				/* B only AP */
				ProperMlmeRate = RATE_11;
			else
				ProperMlmeRate = RATE_24;
			
			if (pApCliEntry->ApCliMlmeAux.Channel <= 14)
				MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		case (WMODE_A):
#ifdef DOT11_N_SUPPORT
		case (WMODE_GN):	/* rt2860 need to check mlmerate for 802.11n */
		case (WMODE_G | WMODE_GN):
		case (WMODE_A | WMODE_G | WMODE_AN | WMODE_GN):
		case (WMODE_A | WMODE_AN):
		case (WMODE_AN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		case (WMODE_AC):
		case (WMODE_AN | WMODE_AC):
		case (WMODE_A | WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
			ProperMlmeRate = RATE_24;
			MinimumRate = RATE_6;
			break;
		case (WMODE_B | WMODE_A | WMODE_G):
			ProperMlmeRate = RATE_24;
			if (pApCliEntry->ApCliMlmeAux.Channel <= 14)
			   MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		default: /* error */
			ProperMlmeRate = RATE_1;
			MinimumRate = RATE_1;
			break;
	}

	for (i = 0; i < pApCliEntry->ApCliMlmeAux.SupRateLen; i++)
	{
		for (j = 0; j < RateIdx; j++)
		{
			if ((pApCliEntry->ApCliMlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
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
		for (i = 0; i < pApCliEntry->ApCliMlmeAux.ExtRateLen; i++)
		{
			for (j = 0; j < RateIdx; j++)
			{
				if ((pApCliEntry->ApCliMlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j])
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
	{
		ProperMlmeRate = MinimumRate;
	}

	if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
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

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPUpdateMlmeRate ==>   MlmeTransmit = 0x%x  \n" , pAd->CommonCfg.MlmeTransmit.word));
}


VOID ApCliCheckPeerExistence(RTMP_ADAPTER *pAd, CHAR *Ssid, UCHAR SsidLen, UCHAR Channel)
{
	UCHAR ifIndex;
	APCLI_STRUCT *pApCliEntry;
	
	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];


		if (pApCliEntry->bPeerExist == TRUE)
			continue;

		else if (Channel == pAd->CommonCfg.Channel &&
			(NdisEqualMemory(Ssid, pApCliEntry->CfgSsid, SsidLen) || SsidLen == 0 /* Hidden */))
		{
			pApCliEntry->bPeerExist = TRUE;
		}
		else
		{
			/* No Root AP match the SSID */
		}
	}
}


VOID APCli_Init(
	IN	PRTMP_ADAPTER				pAd,
	IN	RTMP_OS_NETDEV_OP_HOOK		*pNetDevOps)
{
	PNET_DEV	new_dev_p;
	INT apcli_index;
	APCLI_STRUCT	*pApCliEntry;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_apcli_init != FALSE)
		return;

	/* init */
	for(apcli_index = 0; apcli_index < MAX_APCLI_NUM; apcli_index++)
		pAd->ApCfg.ApCliTab[apcli_index].dev = NULL;

	/* create virtual network interface */
	for(apcli_index = 0; apcli_index < MAX_APCLI_NUM; apcli_index++)
	{
		UINT32 MC_RowID = 0, IoctlIF = 0;
#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_APCLI, apcli_index, sizeof(PRTMP_ADAPTER), INF_APCLI_DEV_NAME);
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);

		pApCliEntry = &pAd->ApCfg.ApCliTab[apcli_index];
		pApCliEntry->pAd = pAd;
#ifdef APCLI_CONNECTION_TRIAL
		pApCliEntry->ifIndex = apcli_index;
#endif /* APCLI_CONNECTION_TRIAL */

		NdisZeroMemory(&pApCliEntry->ApCliCounter, sizeof(APCLI_COUNTER));

		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(pApCliEntry->CurrentAddress, pAd->CurrentAddress);

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
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 16)
					MacMask = 0xF0;

				if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
				{
					/* 	
						Refer to HW definition - 
							Bit1 of MAC address Byte0 is local administration bit 
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
			 		*/ 
#ifdef ENHANCE_NEW_MBSSID_MODE
					pApCliEntry->CurrentAddress[0] &= (MacMask << 2);
#endif /* ENHANCE_NEW_MBSSID_MODE */
					pApCliEntry->CurrentAddress[0] |= 0x2;
					pApCliEntry->CurrentAddress[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
				}
#ifdef ENHANCE_NEW_MBSSID_MODE
				else
				{
					pApCliEntry->CurrentAddress[0] |= 0x2;
					pApCliEntry->CurrentAddress[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
					pApCliEntry->CurrentAddress[pAd->chipCap.MBSSIDMode - 1] += ((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
				}
#endif /* ENHANCE_NEW_MBSSID_MODE */
			}
		}
		else
		{
		pApCliEntry->CurrentAddress[ETH_LENGTH_OF_ADDRESS - 1] =
			(pApCliEntry->CurrentAddress[ETH_LENGTH_OF_ADDRESS - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;
		}

		
		pNetDevOps->priv_flags = INT_APCLI; /* we are virtual interface */
		pNetDevOps->needProtcted = TRUE;
		NdisMoveMemory(pNetDevOps->devAddr, &pApCliEntry->CurrentAddress[0], MAC_ADDR_LEN);

		/* backup our virtual network interface */
		pApCliEntry->dev = new_dev_p;

		/* register this device to OS */
		RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	} /* End of for */

	pAd->flg_apcli_init = TRUE;
	pAd->ApCfg.ApCliNum = MAX_APCLI_NUM;

}


VOID ApCli_Remove(
	IN PRTMP_ADAPTER 	pAd)
{
	UINT index;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (pAd->ApCfg.ApCliTab[index].dev)
		{
			RtmpOSNetDevDetach(pAd->ApCfg.ApCliTab[index].dev);

			RtmpOSNetDevFree(pAd->ApCfg.ApCliTab[index].dev);

			/* Clear it as NULL to prevent latter access error. */
			pAd->flg_apcli_init = FALSE;
			pAd->ApCfg.ApCliTab[index].dev = NULL;
		}
	}
}


BOOLEAN ApCli_Open(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNET_DEV			dev_p)
{
	UCHAR ifIndex;


	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		if (pAd->ApCfg.ApCliTab[ifIndex].dev == dev_p)
		{
			RTMP_OS_NETDEV_START_QUEUE(dev_p);
			ApCliIfUp(pAd);
#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_UP, NULL, NULL, 0);
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */
			return TRUE;
		}
	}

	return FALSE;
} /* End of ApCli_Open */


BOOLEAN ApCli_Close(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNET_DEV		dev_p)
{
	UCHAR ifIndex;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx;
#endif /* MAC_REPEATER_SUPPORT */

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		if (pAd->ApCfg.ApCliTab[ifIndex].dev == dev_p)
		{

#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);

		if (pAd->ApCfg.ApCliTab[ifIndex].pWpaAssocIe)
		{
			os_free_mem(NULL, pAd->ApCfg.ApCliTab[ifIndex].pWpaAssocIe);
			pAd->ApCfg.ApCliTab[ifIndex].pWpaAssocIe = NULL;
			pAd->ApCfg.ApCliTab[ifIndex].WpaAssocIeLen = 0;
		}
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */

			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

			/* send disconnect-req to sta State Machine. */
			if (pAd->ApCfg.ApCliTab[ifIndex].Enable)
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
							RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
						}
					}
				}
#endif /* MAC_REPEATER_SUPPORT */

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
				RTMP_MLME_HANDLER(pAd);
				DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));
			}
			return TRUE;
		}
	}

	return FALSE;
} /* End of ApCli_Close */


int APC_PacketSend(
	IN	PNDIS_PACKET				skb_p, 
	IN	PNET_DEV					dev_p,
	IN	RTMP_NET_PACKET_TRANSMIT	Func)
{
	RTMP_ADAPTER *pAd;
	PAPCLI_STRUCT pApCli;
	INT apcliIndex;



	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}
#endif /* RALINK_ATE */

	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))          ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
	{
		/* wlan is scanning/disabled/reset */
		RELEASE_NDIS_PACKET(pAd, skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}


	pApCli = (PAPCLI_STRUCT)&pAd->ApCfg.ApCliTab;

	for(apcliIndex = 0; apcliIndex < MAX_APCLI_NUM; apcliIndex++)
	{
		if (pApCli[apcliIndex].Valid != TRUE)
			continue;

		/* find the device in our ApCli list */
		if (pApCli[apcliIndex].dev == dev_p)
		{
			/* ya! find it */
			pAd->RalinkCounters.PendingNdisPacketCount ++;
			RTMP_SET_PACKET_MOREDATA(skb_p, FALSE);
			RTMP_SET_PACKET_NET_DEVICE_APCLI(skb_p, apcliIndex);
			SET_OS_PKT_NETDEV(skb_p, pAd->net_dev);

			/* transmit the packet */
			return Func(skb_p);
		}
    }

	RELEASE_NDIS_PACKET(pAd, skb_p, NDIS_STATUS_FAILURE);

	return 0;
}

BOOLEAN ApCli_StatsGet(
	IN	PRTMP_ADAPTER pAd,
	IN	RT_CMD_STATS64 *pStats)
{
	INT ifIndex = 0, index;
	APCLI_STRUCT *pApCliTab;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (pAd->ApCfg.ApCliTab[index].dev == (PNET_DEV)(pStats->pNetDev))
		{
			ifIndex = index;
			break;
		}
	}

	if(index >= MAX_APCLI_NUM)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_ioctl apcli_statsGet can not find apcli I/F\n"));
		return FALSE;
	}

	pApCliTab = &pAd->ApCfg.ApCliTab[ifIndex];

	pStats->rx_bytes = pApCliTab->ApCliCounter.ReceivedByteCount.QuadPart;
	pStats->tx_bytes = pApCliTab->ApCliCounter.TransmittedByteCount.QuadPart;

	pStats->rx_packets = pApCliTab->ApCliCounter.ReceivedFragmentCount;
	pStats->tx_packets = pApCliTab->ApCliCounter.TransmittedFragmentCount;

	pStats->rx_errors = pApCliTab->ApCliCounter.RxErrors;
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
	STRING			*pCfgSsid;
	BSS_TABLE		*pScanTab, *pSsidBssTab;

	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliAutoConnectExec()\n"));

	for(ifIdx=0; ifIdx<MAX_APCLI_NUM; ifIdx++)
	{
		if (pAd->ApCfg.ApCliTab[ifIdx].AutoConnectFlag == TRUE)
			break;
	}

	if(ifIdx >= MAX_APCLI_NUM)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Error  ifIdx=%d\n", ifIdx));
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
	pSsidBssTab = &pAd->MlmeAux.SsidBssTab;
	pSsidBssTab->BssNr = 0;
	
	/*
		Find out APs with the desired SSID.  
	*/
	for (entryIdx=0; entryIdx<pScanTab->BssNr;entryIdx++)
	{
		PBSS_ENTRY pBssEntry = &pScanTab->BssEntry[entryIdx];
		
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
				DBGPRINT(RT_DEBUG_TRACE, 
						("Found desired ssid in Entry %2d:\n", entryIdx));
				DBGPRINT(RT_DEBUG_TRACE,
						("I/F(%s%d) ApCliAutoConnectExec:(Len=%d,Ssid=%s, Channel=%d, Rssi=%d)\n", 
						INF_APCLI_DEV_NAME, ifIdx, pBssEntry->SsidLen, pBssEntry->Ssid,
						pBssEntry->Channel, pBssEntry->Rssi));
				DBGPRINT(RT_DEBUG_TRACE,
						("I/F(%s%d) ApCliAutoConnectExec::(AuthMode=%s, EncrypType=%s)\n", INF_APCLI_DEV_NAME, ifIdx,
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
		DBGPRINT(RT_DEBUG_TRACE, ("No match entry.\n"));
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
			DBGPRINT(RT_DEBUG_TRACE, ("Switch to channel :%s\n", tempBuf));
			Set_Channel_Proc(pAd, tempBuf);
		}
		
		ApCliSetBssid(pAd, ifIdx, pBssEntry->Bssid);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Error! Out of table range: (BssNr=%d).\n", pSsidBssTab->BssNr) );
		ApCliSetIfState(pAd, ifIdx, TRUE);
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("<--- ApCliAutoConnectExec()\n"));
		return FALSE;
	}

	ApCliSetIfState(pAd, ifIdx, TRUE);
	DBGPRINT(RT_DEBUG_TRACE, ("<--- ApCliAutoConnectExec()\n"));
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

	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliSwitchCandidateAP()\n"));

	pSsidBssTab = &pAd->MlmeAux.SsidBssTab;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

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
		UCHAR tempBuf[20];
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1];
		
		ApCliSetBssid(pAd, ifIndex, pBssEntry->Bssid);
		
		if (pAd->CommonCfg.Channel != pBssEntry->Channel)
		{
			ApCliSetIfState(pAd, ifIndex, FALSE);
			sprintf(tempBuf, "%d", pBssEntry->Channel);
			DBGPRINT(RT_DEBUG_TRACE, ("Switch to channel :%s\n", tempBuf));
			Set_Channel_Proc(pAd, tempBuf);
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("No candidate AP, the process is about to stop.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
	}

exit_and_enable:

	ApCliSetIfState(pAd, ifIndex, TRUE);
	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliSwitchCandidateAP()\n"));
}


BOOLEAN ApcliCompareAuthEncryp(
	IN PAPCLI_STRUCT pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthModeAux,
	IN NDIS_802_11_WEP_STATUS			WEPstatus,
	IN CIPHER_SUITE WPA)
{
	NDIS_802_11_AUTHENTICATION_MODE	tempAuthMode = pApCliEntry->AuthMode;
	NDIS_802_11_WEP_STATUS				tempWEPstatus = pApCliEntry->WepStatus;

	DBGPRINT(RT_DEBUG_TRACE, ("ApcliAuthMode=%s, AuthMode=%s, AuthModeAux=%s, ApcliWepStatus=%s,	WepStatus=%s, GroupCipher=%s, PairCipher=%s,  \n",
					GetAuthMode(pApCliEntry->AuthMode),
					GetAuthMode(AuthMode),
					GetAuthMode(AuthModeAux),
					GetEncryptType(pApCliEntry->WepStatus),
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
#endif /* APCLI_SUPPORT */

