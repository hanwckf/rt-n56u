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


/*
========================================================================
Routine Description:
    Remove ApCli-BSS network interface.

Arguments:
    ad_p            points to our adapter

Return Value:
    None

Note:
========================================================================
*/
VOID RT28xx_ApCli_Remove(
	IN PRTMP_ADAPTER ad_p)
{
	UINT index;


	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (ad_p->ApCfg.ApCliTab[index].dev)
		{
			RtmpOSNetDevDetach(ad_p->ApCfg.ApCliTab[index].dev);

			RtmpOSNetDevFree(ad_p->ApCfg.ApCliTab[index].dev);

			// Clear it as NULL to prevent latter access error.
			ad_p->flg_apcli_init = FALSE;
			ad_p->ApCfg.ApCliTab[index].dev = NULL;
		}
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
	IN		PRTMP_ADAPTER 		pAd,
	IN		USHORT 				IfIndex,
	IN OUT	HT_CAPABILITY_IE 	*pHtCapability,
	IN OUT	ADD_HT_INFO_IE 		*pAddHtInfo)
{
	PAPCLI_STRUCT pApCliEntry = NULL;
	
	if (IfIndex >= MAX_APCLI_NUM)
		return FALSE;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];

	// If use AMSDU, set flag.
	if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
		CLIENT_STATUS_SET_FLAG(pApCliEntry, fCLIENT_STATUS_AMSDU_INUSED);
	// Save Peer Capability
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

	if ((pAd->OpMode == OPMODE_STA))
	{
	}
	pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
	pAd->MlmeAux.HtCapability.MCSSet[4] = 0x1;
	//2008/12/17:KH modified to fix the low throughput of AP-Client on Big-Endian Platform<--
	 switch (pAd->CommonCfg.RxStream)
	{
		case 1:			
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
		case 2:
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
		case 3:				
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
	}

	/* Record the RxMcs of AP */
	NdisMoveMemory(pApCliEntry->RxMcsSet, pHtCapability->MCSSet, 16);


	/*
	if (pAd->Antenna.field.TxPath == 2)	// 2: 2Tx   1: 1Tx
	{
		pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
	}
	else
	{
		pAd->MlmeAux.HtCapability.MCSSet[1] = 0x00;
	}
	*/
	//2008/12/17:KH modified to fix the low throughput of AP-Client on Big-Endian Platform-->

	// choose smaller setting
	pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pAd->CommonCfg.DesiredHtPhy.ChannelWidth;
	pAd->MlmeAux.HtCapability.HtCapInfo.GF =  pHtCapability->HtCapInfo.GF &pAd->CommonCfg.DesiredHtPhy.GF;

	// Send Assoc Req with my HT capability.
	pAd->MlmeAux.HtCapability.HtCapInfo.AMsduSize =  pAd->CommonCfg.DesiredHtPhy.AmsduSize;
	pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs =  pAd->CommonCfg.DesiredHtPhy.MimoPs;
	pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20 =  (pAd->CommonCfg.DesiredHtPhy.ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 =  (pAd->CommonCfg.DesiredHtPhy.ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC =  (pAd->CommonCfg.DesiredHtPhy.TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC =  (pAd->CommonCfg.DesiredHtPhy.RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);
	pAd->MlmeAux.HtCapability.HtCapParm.MaxRAmpduFactor = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor;
	pAd->MlmeAux.HtCapability.HtCapParm.MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	pAd->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	if (pAd->CommonCfg.bRdg)
	{
		pAd->MlmeAux.HtCapability.ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
	}
	
	//COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCapability);
	return TRUE;
}
#endif // DOT11_N_SUPPORT //

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


	do
	{
		if (ifIndex < MAX_APCLI_NUM)
		{
			printk("!!! APCLI LINK UP - IF(%s%d) AuthMode(%d)=%s, WepStatus(%d)=%s !!!\n", INF_APCLI_DEV_NAME, ifIndex,
					pAd->ApCfg.ApCliTab[ifIndex].AuthMode, GetAuthMode(pAd->ApCfg.ApCliTab[ifIndex].AuthMode),
					pAd->ApCfg.ApCliTab[ifIndex].WepStatus, GetEncryptType(pAd->ApCfg.ApCliTab[ifIndex].WepStatus));
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : APCLI LINK UP - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
			result = FALSE;
			break;
		}

		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

		// Sanity check: This link had existed. 
		if (pApCliEntry->Valid)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : This link had existed - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
			result = FALSE;
			break;
		}
	
		// Insert the Remote AP to our MacTable.
		//pMacEntry = MacTableInsertApCliEntry(pAd, (PUCHAR)(pAd->MlmeAux.Bssid));
		pMacEntry = MacTableInsertEntry(pAd, (PUCHAR)(pAd->MlmeAux.Bssid), (ifIndex + MIN_NET_DEVICE_FOR_APCLI), TRUE);
		if (pMacEntry)
		{
			UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES];
			PUCHAR pRates = Rates;
			UCHAR RatesLen;
			UCHAR MaxSupportedRate = 0;

			pMacEntry->Sst = SST_ASSOC;
			
			pApCliEntry->Valid = TRUE;
			pApCliEntry->MacTabWCID = pMacEntry->Aid;

			COPY_MAC_ADDR(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID), pAd->MlmeAux.Bssid);
			pApCliEntry->SsidLen = pAd->MlmeAux.SsidLen;
			NdisMoveMemory(pApCliEntry->Ssid, pAd->MlmeAux.Ssid, pApCliEntry->SsidLen);

			if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				pMacEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
				pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;

			NdisGetSystemUpTime(&pApCliEntry->ApCliLinkUpTime);

			// Store appropriate RSN_IE for WPA SM negotiation later 
			// If WPAPSK/WPA2SPK mix mode, driver just stores either WPAPSK or WPA2PSK
			// RSNIE. It depends on the AP-Client's authentication mode to store the corresponding RSNIE.   
			if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA) && (pAd->MlmeAux.VarIELen != 0))
			{
				PUCHAR              pVIE;
				UCHAR               len;
				PEID_STRUCT         pEid;

				pVIE = pAd->MlmeAux.VarIEs;
				len	 = pAd->MlmeAux.VarIELen;

				while (len > 0)
				{
					pEid = (PEID_STRUCT) pVIE;	
					// For WPA/WPAPSK
					if ((pEid->Eid == IE_WPA) && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4)) 
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA || pMacEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
					{
						NdisMoveMemory(pMacEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pMacEntry->RSNIE_Len = (pEid->Len + 2);							
						DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp: Store RSN_IE for WPA SM negotiation \n"));
					}
					// For WPA2/WPA2PSK
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

			SupportRate(pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRate,
				pAd->MlmeAux.ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);

			pMacEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxTxRate, MaxSupportedRate);
			pMacEntry->RateLen = RatesLen;
			if (pMacEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
			{
				pMacEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
				pMacEntry->MaxHTPhyMode.field.MCS = pMacEntry->MaxSupportedRate;
				pMacEntry->MinHTPhyMode.field.MODE = MODE_CCK;
				pMacEntry->MinHTPhyMode.field.MCS = pMacEntry->MaxSupportedRate;
				pMacEntry->HTPhyMode.field.MODE = MODE_CCK;
				pMacEntry->HTPhyMode.field.MCS = pMacEntry->MaxSupportedRate;
			}
			else
			{
				pMacEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
				pMacEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pMacEntry->MaxSupportedRate];
				pMacEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
				pMacEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pMacEntry->MaxSupportedRate];
				pMacEntry->HTPhyMode.field.MODE = MODE_OFDM;
				pMacEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pMacEntry->MaxSupportedRate];
			}
			pMacEntry->CapabilityInfo = pAd->MlmeAux.CapabilityInfo;

			// If WEP is enabled, add paiewise and shared key
			if (pApCliEntry->WepStatus == Ndis802_11WEPEnabled)
			{			
				PCIPHER_KEY pKey; 			
				INT 		idx, BssIdx;

				BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
			
				for (idx=0; idx < SHARE_KEY_NUM; idx++)
				{
					pKey = &pApCliEntry->SharedKey[idx];
										
					if (pKey->KeyLen > 0)
					{
						// Set key material and cipherAlg to Asic
						RTMP_ASIC_SHARED_KEY_TABLE(pAd, 
	    									  		BssIdx, 
	    									  		idx, 
		    										pKey);	

						if (idx == pApCliEntry->DefaultKeyId)
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
			// If this Entry supports 802.11n, upgrade to HT rate. 
			if (pAd->MlmeAux.HtCapabilityLen != 0)
			{
				UCHAR	j, bitmask; //k,bitmask;
				CHAR    i;
				PHT_CAPABILITY_IE pHtCapability = (PHT_CAPABILITY_IE)&pAd->MlmeAux.HtCapability;

				if ((pAd->MlmeAux.HtCapability.HtCapInfo.GF) && (pAd->CommonCfg.DesiredHtPhy.GF))
				{
					pMacEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
				}
				else
				{	
					pMacEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
					pAd->MacTab.fAnyStationNonGF = TRUE;
					pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
				}

				if ((pHtCapability->HtCapInfo.ChannelWidth) && (pAd->CommonCfg.DesiredHtPhy.ChannelWidth))
				{
					pMacEntry->MaxHTPhyMode.field.BW= BW_40;
					pMacEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor40)&(pHtCapability->HtCapInfo.ShortGIfor40));
				}
				else
				{	
					pMacEntry->MaxHTPhyMode.field.BW = BW_20;
					pMacEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor20)&(pHtCapability->HtCapInfo.ShortGIfor20));
					pAd->MacTab.fAnyStation20Only = TRUE;
				}

				// find max fixed rate
				//2008/12/17:KH modified to fix the low throughput of AP-Client on Big-Endian Platform<--

				//for (i=15; i>=0; i--)
				for (i=23; i>=0; i--)	//3*3
				//2008/12/17:KH modified to fix the low throughput of AP-Client on Big-Endian Platform-->
				{	
					j = i/8;	
					bitmask = (1<<(i-(j*8)));
					if ((pAd->ApCfg.ApCliTab[ifIndex].DesiredHtPhyInfo.MCSSet[j] & bitmask) && (pHtCapability->MCSSet[j] & bitmask))
					{
						pMacEntry->MaxHTPhyMode.field.MCS = i;
						break;
					}
					if (i==0)
						break;
				}

				 
				if (pAd->ApCfg.ApCliTab[ifIndex].DesiredTransmitSetting.field.MCS != MCS_AUTO)
				{
					printk("IF-%s%d : Desired MCS = %d\n", INF_APCLI_DEV_NAME, ifIndex,
						pAd->ApCfg.ApCliTab[ifIndex].DesiredTransmitSetting.field.MCS);
					
					if (pAd->ApCfg.ApCliTab[ifIndex].DesiredTransmitSetting.field.MCS == 32)
					{
						// Fix MCS as HT Duplicated Mode
						pMacEntry->MaxHTPhyMode.field.BW = 1;
						pMacEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
						pMacEntry->MaxHTPhyMode.field.STBC = 0;
						pMacEntry->MaxHTPhyMode.field.ShortGI = 0;
						pMacEntry->MaxHTPhyMode.field.MCS = 32;
					}
					else if (pMacEntry->MaxHTPhyMode.field.MCS > pAd->ApCfg.ApCliTab[ifIndex].HTPhyMode.field.MCS)
					{
						// STA supports fixed MCS 
						pMacEntry->MaxHTPhyMode.field.MCS = pAd->ApCfg.ApCliTab[ifIndex].HTPhyMode.field.MCS;
					}
				}

				pMacEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
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
				NdisMoveMemory(&pMacEntry->HTCapability, &pAd->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
				NdisMoveMemory(pMacEntry->HTCapability.MCSSet, pApCliEntry->RxMcsSet, 16);
			}
			else
			{
				pAd->MacTab.fAnyStationIsLegacy = TRUE;
				printk("ApCliLinkUp - MaxSupRate=%d Mbps\n", RateIdToMbps[pMacEntry->MaxSupportedRate]);
			}

#endif // DOT11_N_SUPPORT //

			pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
			pMacEntry->CurrTxRate = pMacEntry->MaxSupportedRate;
			
			if (pAd->ApCfg.ApCliTab[ifIndex].bAutoTxRateSwitch == FALSE)
			{
				pMacEntry->bAutoTxRateSwitch = FALSE;
				// If the legacy mode is set, overwrite the transmit setting of this entry.  			
				RTMPUpdateLegacyTxSetting((UCHAR)pAd->ApCfg.ApCliTab[ifIndex].DesiredTransmitSetting.field.FixedTxMode, pMacEntry);	
			}
			else
			{
				PUCHAR pTable;
				UCHAR TableSize = 0;

				pMacEntry->bAutoTxRateSwitch = TRUE;
				APMlmeSelectTxRateTable(pAd, pMacEntry, &pTable, &TableSize, &pMacEntry->CurrTxRateIndex);
			}
			
			// It had been set in APStartUp. Don't set again.
			//AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
			
			// set this entry WMM capable or not
			if ((pAd->MlmeAux.APEdcaParm.bValid)
#ifdef DOT11_N_SUPPORT
				|| IS_HT_STA(pMacEntry)
#endif // DOT11_N_SUPPORT //
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
				if ((pAd->CommonCfg.bPiggyBackCapable) && (pAd->MlmeAux.APRalinkIe & 0x00000003) == 3)
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_PIGGYBACK_INUSED);
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
                	RTMPSetPiggyBack(pAd, TRUE);
					DBGPRINT(RT_DEBUG_TRACE, ("Turn on Piggy-Back\n"));
				}
				else if (pAd->MlmeAux.APRalinkIe & 0x00000001)
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
					DBGPRINT(RT_DEBUG_TRACE, ("Ralink Aggregation\n"));
				}
			}

			if (pAd->MlmeAux.APRalinkIe != 0x0)
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);
			else
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);

			NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);

			// set the apcli interface be valid.
			pApCliEntry->Valid = TRUE;
			result = TRUE;

			pAd->ApCfg.ApCliInfRunned++;
			break;
		}
		result = FALSE;

	} while(FALSE);

#ifdef WSC_AP_SUPPORT
    // WSC initial connect to AP, jump to Wsc start action and set the correct parameters    
	if ((result == TRUE) && 
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode == WSC_ENROLLEE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
	{
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscState = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscStatus = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].WscControl.EntryAddr, MAC_ADDR_LEN);        
		NdisMoveMemory(pAd->ApCfg.ApCliTab[ifIndex].WscControl.EntryAddr, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
		WscSendEapolStart(pAd, pMacEntry->Addr);
	}
    else
    {
        WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[ifIndex].WscControl);
    }
#endif // WSC_AP_SUPPORT //

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

	if (ifIndex < MAX_APCLI_NUM)
	{
		printk("!!! APCLI LINK DOWN - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("!!! ERROR : APCLI LINK DOWN - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
		return;
	}
    	
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if (pApCliEntry->Valid == FALSE)	
		return;

	pAd->ApCfg.ApCliInfRunned--;
	MacTableDeleteEntry(pAd, pApCliEntry->MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID));

	pApCliEntry->Valid = FALSE;	// This link doesn't associated with any remote-AP 
	
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

	// Reset is in progress, stop immediately
	if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
		 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	// sanity check whether the interface is initialized.
	if (pAd->flg_apcli_init != TRUE)
		return;

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		if (APCLI_IF_UP_CHECK(pAd, ifIndex) 
			&& (pApCliEntry->Enable == TRUE)
			&& (pApCliEntry->Valid == FALSE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startup.\n", __FUNCTION__, ifIndex));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex);
		}
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

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
	}

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

	// Reset is in progress, stop immediately
	if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
		 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	// sanity check whether the interface is initialized.
	if (pAd->flg_apcli_init != TRUE)
		return;
	
	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		UCHAR Wcid;
		PMAC_TABLE_ENTRY pMacEntry;
		BOOLEAN bForceBrocken = FALSE;

		pApCliEntry = &pAd->ApCfg.ApCliTab[index];
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
#endif // WSC_AP_SUPPORT //


	// only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, 
	// ignore this frame

	// WPA EAPOL PACKET
	if (pFrame->Hdr.FC.Type == BTYPE_DATA)
	{		
#ifdef WSC_AP_SUPPORT    
        //WSC EAPOL PACKET        
        pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
        if (pEntry && IS_ENTRY_APCLI(pEntry) && pAd->ApCfg.ApCliTab[pEntry->apidx].WscControl.WscConfMode == WSC_ENROLLEE)
        {
            *Machine = WSC_STATE_MACHINE;
            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
            EAPCode = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
            Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
        }
        if (!Return)
#endif // WSC_AP_SUPPORT //
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
				// get the sequence number from payload 24 Mac Header + 2 bytes algorithm
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
				//  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support
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
			// Beacon must be processed be AP Sync state machine.
        	case SUBTYPE_BEACON:
				*Machine = AP_SYNC_STATE_MACHINE;
				*MsgType = APMT2_PEER_BEACON;
            	break;

			// Only Sta have chance to receive Probe-Rsp.
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
    OUT ADD_HT_INFO_IE *pAddHtInfo,	// AP might use this additional ht info IE 
    OUT UCHAR *pHtCapabilityLen,
    OUT UCHAR *pAddHtInfoLen,
    OUT UCHAR *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag) 
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

	// Aid already swaped byte order in RTMPFrameEndianChange() for big endian platform
	*pAid = (*pAid) & 0x3fff; // AID is low 14-bit
        
	// -- get supported rates from payload and advance the pointer
	IeType = pFrame->Octet[6];
	*pSupRateLen = pFrame->Octet[7];
	if ((IeType != IE_SUPP_RATES) || (*pSupRateLen > MAX_LEN_OF_SUPPORTED_RATES))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspSanity fail - wrong SupportedRates IE\n"));
		return FALSE;
	}
	else 
		NdisMoveMemory(SupRate, &pFrame->Octet[8], *pSupRateLen);

	Length = Length + 2 + *pSupRateLen;

	// many AP implement proprietary IEs in non-standard order, we'd better
	// tolerate mis-ordered IEs to get best compatibility
	pEid = (PEID_STRUCT) &pFrame->Octet[8 + (*pSupRateLen)];
            
	// get variable fields from payload and advance the pointer
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
				if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
				{
					NdisMoveMemory(pHtCapability, pEid->Octet, SIZE_HT_CAP_IE);
					*pHtCapabilityLen = SIZE_HT_CAP_IE;
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_HT_CAP. \n"));
				}
				
				break;
			case IE_ADD_HT:
			case IE_ADD_HT2:
				if (pEid->Len >= sizeof(ADD_HT_INFO_IE))				
				{
					// This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only
					// copy first sizeof(ADD_HT_INFO_IE)
					NdisMoveMemory(pAddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
					*pAddHtInfoLen = SIZE_ADD_HT_INFO_IE;
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_ADD_HT. \n"));
				}
				break;
			case IE_SECONDARY_CH_OFFSET:
				if (pEid->Len == 1)
				{
					*pNewExtChannelOffset = pEid->Octet[0];
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_SECONDARY_CH_OFFSET. \n"));
				}
				break;
#endif // DOT11_N_SUPPORT //
			// CCX2, WMM use the same IE value
			// case IE_CCX_V2:
			case IE_VENDOR_SPECIFIC:
				// handle WME PARAMTER ELEMENT
				if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
				{
					PUCHAR ptr;
					int i;
        
					// parsing EDCA parameters
					pEdcaParm->bValid          = TRUE;
					pEdcaParm->bQAck           = FALSE; // pEid->Octet[0] & 0x10;
					pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
					pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
					//pEdcaParm->bMoreDataAck    = FALSE; // pEid->Octet[0] & 0x80;
					pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
					pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
					ptr = (PUCHAR) &pEid->Octet[8];
					for (i=0; i<4; i++)
					{
						UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
						pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   // b5 is ACM
						pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               // b0~3 is AIFSN
						pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             // b0~4 is Cwmin
						pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               // b5~8 is Cwmax
						pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
						ptr += 4; // point to next AC
					}
				}
				break;
				default:
					DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspSanity - ignore unrecognized EID = %d\n", pEid->Eid));
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
	//USHORT HashIdx;
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
		Check the WDS Entry is valid or not.
	==========================================================================
 */
static inline BOOLEAN ValidApCliEntry(
	IN PRTMP_ADAPTER pAd,
	IN INT apCliIdx)
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


BOOLEAN ApCliAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR		*pWcid)
{
	UCHAR apCliIdx;
	BOOLEAN	allowed;
		
	//DBGPRINT(RT_DEBUG_TRACE, ("ApCliAllowToSendPacket():Packet to ApCli interface!\n"));
	apCliIdx = RTMP_GET_PACKET_NET_DEVICE(pPacket) - MIN_NET_DEVICE_FOR_APCLI;
	if (ValidApCliEntry(pAd, apCliIdx))
	{
		//DBGPRINT(RT_DEBUG_TRACE, ("ApCliAllowToSendPacket(): Set the WCID as %d!\n", pAd->ApCfg.ApCliTab[apCliIdx].MacTabWCID));
		
		*pWcid = pAd->ApCfg.ApCliTab[apCliIdx].MacTabWCID;
		//RTMP_SET_PACKET_WCID(pPacket, pAd->ApCfg.ApCliTab[apCliIdx].MacTabWCID); // to ApClient links.
		
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
	CIPHER_SUITE		WPA;			// AP announced WPA cipher suite
	CIPHER_SUITE		WPA2;			// AP announced WPA2 cipher suite
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

	//if (len >= MAX_LEN_OF_RSNIE || len <= MIN_LEN_OF_RSNIE)
	//	return FALSE;

	// Init WPA setting
	WPA.PairCipher    	= Ndis802_11WEPDisabled;
	WPA.PairCipherAux 	= Ndis802_11WEPDisabled;
	WPA.GroupCipher   	= Ndis802_11WEPDisabled;
	WPA.RsnCapability 	= 0;
	WPA.bMixMode      	= FALSE;
	WPA_AuthMode	  	= Ndis802_11AuthModeOpen;
	WPA_AuthModeAux		= Ndis802_11AuthModeOpen;	

	// Init WPA2 setting
	WPA2.PairCipher    	= Ndis802_11WEPDisabled;
	WPA2.PairCipherAux 	= Ndis802_11WEPDisabled;
	WPA2.GroupCipher   	= Ndis802_11WEPDisabled;
	WPA2.RsnCapability 	= 0;
	WPA2.bMixMode      	= FALSE;
	WPA2_AuthMode	  	= Ndis802_11AuthModeOpen;
	WPA2_AuthModeAux	= Ndis802_11AuthModeOpen;

	Sanity = 0;

	// 1. Parse Cipher this received RSNIE
	while (len > 0)
	{		
		pTmp = pVIE;
		pEid = (PEID_STRUCT) pTmp;	

		switch(pEid->Eid)
		{
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
				{
					// if unsupported vendor specific IE
					break;
				}	
				// Skip OUI ,version and multicast suite OUI
				pTmp += 11;

				// Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
	            //  Value      Meaning
	            //  0           None 
	            //  1           WEP-40
	            //  2           Tkip
	            //  3           WRAP
	            //  4           AES
	            //  5           WEP-104
				// Parse group cipher
				switch (*pTmp)
				{
					case 1:
					case 5:	// Although WEP is not allowed in WPA related auth mode, we parse it anyway
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

				// number of unicast suite
				pTmp += 1;

				// Store unicast cipher count
			    NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
    			Count = cpu2le16(Count);		

				// pointer to unicast cipher
			    pTmp += sizeof(USHORT);	

				// Parsing all unicast cipher suite				
				while (Count > 0)
				{
					// Skip cipher suite OUI
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: // Although WEP is not allowed in WPA related auth mode, we parse it anyway
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
						// Move the lower cipher suite to PairCipherAux
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
			
				// Get AKM suite counts
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);		

				pTmp   += sizeof(USHORT);

				// Parse AKM ciphers
				// Parsing all AKM cipher suite				
				while (Count > 0)
				{
			    	// Skip cipher suite OUI
					pTmp   += 3;
					TmpAuthMode = Ndis802_11AuthModeOpen;
					switch (*pTmp)
					{	
						case 1:
							// WPA-enterprise
							TmpAuthMode = Ndis802_11AuthModeWPA;							
							break;
						case 2:
							// WPA-personal
							TmpAuthMode = Ndis802_11AuthModeWPAPSK;									    	
							break;
						default:
							break;
					}
					if (TmpAuthMode > WPA_AuthMode)
					{
						// Move the lower AKM suite to WPA_AuthModeAux
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

				// ToDo - Support WPA-None ?

				// Check the Pair & Group, if different, turn on mixed mode flag
				if (WPA.GroupCipher != WPA.PairCipher)
					WPA.bMixMode = TRUE;

				DBGPRINT(RT_DEBUG_TRACE, ("ApCliValidateRSNIE - RSN-WPA1 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
											((WPA.bMixMode) ? "Mix" : GetEncryptType(WPA.PairCipher)), 
											GetEncryptType(WPA.GroupCipher),
											GetAuthMode(WPA_AuthMode)));

				Sanity |= 0x1;
				break; // End of case IE_WPA //
			case IE_RSN:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
				
				// 0. Version must be 1
				//  The pRsnHeader->Version exists in native little-endian order, so we may need swap it for RT_BIG_ENDIAN systems.
				if (le2cpu16(pRsnHeader->Version) != 1)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - RSN Version isn't 1(%d) \n", pRsnHeader->Version));
					break;
				}	

				pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

				// 1. Check cipher OUI				
				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
				{
					// if unsupported vendor specific IE
					break;
				}

				// Skip cipher suite OUI
				pTmp += 3;

				// Parse group cipher
				switch (*pTmp)
				{
					case 1:
					case 5:	// Although WEP is not allowed in WPA related auth mode, we parse it anyway
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

				// number of unicast suite
				pTmp += 1;

				// Get pairwise cipher counts				
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);
				
				pTmp   += sizeof(USHORT);

				// 3. Get pairwise cipher
				// Parsing all unicast cipher suite
				while (Count > 0)
				{
					// Skip OUI
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: // Although WEP is not allowed in WPA related auth mode, we parse it anyway
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
						// Move the lower cipher suite to PairCipherAux
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

				// Get AKM suite counts				
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);		

				pTmp   += sizeof(USHORT);

				// Parse AKM ciphers
				// Parsing all AKM cipher suite				
				while (Count > 0)
				{
			    	// Skip cipher suite OUI
					pTmp   += 3;
					TmpAuthMode = Ndis802_11AuthModeOpen;
					switch (*pTmp)
					{	
						case 1:
							// WPA2-enterprise
							TmpAuthMode = Ndis802_11AuthModeWPA2;							
							break;
						case 2:
							// WPA2-personal
							TmpAuthMode = Ndis802_11AuthModeWPA2PSK;									    	
							break;
						default:
							break;
					}
					if (TmpAuthMode > WPA2_AuthMode)
					{
						// Move the lower AKM suite to WPA2_AuthModeAux
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

				// Check the Pair & Group, if different, turn on mixed mode flag
				if (WPA2.GroupCipher != WPA2.PairCipher)
					WPA2.bMixMode = TRUE;

				DBGPRINT(RT_DEBUG_TRACE, ("ApCliValidateRSNIE - RSN-WPA2 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
									(WPA2.bMixMode ? "Mix" : GetEncryptType(WPA2.PairCipher)), GetEncryptType(WPA2.GroupCipher),
									GetAuthMode(WPA2_AuthMode)));

				Sanity |= 0x2;
				break; // End of case IE_RSN //
			default:
					DBGPRINT(RT_DEBUG_WARN, ("ApCliValidateRSNIE - Unknown pEid->Eid(%d) \n", pEid->Eid));
				break;
		}

		// skip this Eid
		pVIE += (pEid->Len + 2);
		len  -= (pEid->Len + 2);
	
	}

	// 2. Validate this RSNIE with mine
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

	// Recovery user-defined cipher suite
	pApCliEntry->PairCipher  = pApCliEntry->WepStatus;
	pApCliEntry->GroupCipher = pApCliEntry->WepStatus;
	pApCliEntry->bMixCipher  = FALSE;

	Sanity = 0;	
	
	// Check AuthMode and WPA_AuthModeAux for matching, in case AP support dual-AuthMode
	// WPAPSK
	if ((WPA_AuthMode == pApCliEntry->AuthMode) || 
		((WPA_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA_AuthModeAux == pApCliEntry->AuthMode)))
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

		Sanity |= 0x1;
	}
	// WPA2PSK
	else if ((WPA2_AuthMode == pApCliEntry->AuthMode) || 
			 ((WPA2_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA2_AuthModeAux == pApCliEntry->AuthMode)))
	{
		// Check cipher suite, AP must have more secured cipher than station setting
		if (WPA2.bMixMode == FALSE)
		{
			if (pApCliEntry->WepStatus != WPA2.GroupCipher)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate cipher suite error \n"));
				return FALSE;
			}
		}

		// check group cipher
		if (pApCliEntry->WepStatus < WPA2.GroupCipher)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate group cipher error \n"));
			return FALSE;
		}

		// check pairwise cipher, skip if none matched
		// If profile set to AES, let it pass without question.
		// If profile set to TKIP, we must find one mateched
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

	//Re-assign pairwise-cipher and group-cipher. Re-build RSNIE. 
	if ((pApCliEntry->AuthMode == Ndis802_11AuthModeWPA) || (pApCliEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
	{
		pApCliEntry->GroupCipher = WPA.GroupCipher;
			
		if (pApCliEntry->WepStatus == WPA.PairCipher)
			pApCliEntry->PairCipher = WPA.PairCipher;
		else if (WPA.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA.PairCipherAux;
		else	// There is no PairCipher Aux, downgrade our capability to TKIP
			pApCliEntry->PairCipher = Ndis802_11Encryption2Enabled;			
	}
	else if ((pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pApCliEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		pApCliEntry->GroupCipher = WPA2.GroupCipher;
			
		if (pApCliEntry->WepStatus == WPA2.PairCipher)
			pApCliEntry->PairCipher = WPA2.PairCipher;
		else if (WPA2.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA2.PairCipherAux;
		else	// There is no PairCipher Aux, downgrade our capability to TKIP
			pApCliEntry->PairCipher = Ndis802_11Encryption2Enabled;					
	}

	// Set Mix cipher flag
	if (pApCliEntry->PairCipher != pApCliEntry->GroupCipher)
	{
		pApCliEntry->bMixCipher = TRUE;	

		// re-build RSNIE
		//RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI));
	}
	
	// re-build RSNIE
	RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI));
	
	return TRUE;	
}

BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN	RX_BLK			*pRxBlk,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			FromWhichBSSID)
{
	PRT28XX_RXD_STRUC	pRxD = &(pRxBlk->RxD);
	PHEADER_802_11		pHeader = pRxBlk->pHeader;
	PRXWI_STRUC			pRxWI = pRxBlk->pRxWI;		
	PAPCLI_STRUCT   	pApCliEntry = NULL;
	
	// It is possible to receive the multicast packet when in AP Client mode
	// Such as a broadcast from remote AP to AP-client, address1 is ffffff, address2 is remote AP's bssid, addr3 is sta4 mac address
																																								
	pApCliEntry	= &pAd->ApCfg.ApCliTab[pEntry->MatchAPCLITabIdx];																											
					
	// Filter out Bcast frame which AP relayed for us
	// Multicast packet send from AP1 , received by AP2 and send back to AP1, drop this frame   					
	if (MAC_ADDR_EQUAL(pHeader->Addr3, pApCliEntry->CurrentAddress))
		return FALSE;	// give up this frame

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;	// give up this frame
					
							
	/* skip the 802.11 header */
	pRxBlk->pData += LENGTH_802_11;
	pRxBlk->DataSize -= LENGTH_802_11;

	// Use software to decrypt the encrypted frame.
	// Because this received frame isn't my BSS frame, Asic passed to driver without decrypting it.
	// If receiving an "encrypted" unicast packet(its WEP bit as 1) and doesn't match my BSSID, it 
	// pass to driver with "Decrypted" marked as 0 in RxD.
	if ((pRxD->MyBss == 0) && (pRxD->Decrypted == 0) && (pHeader->FC.Wep == 1)) 
	{											
		if (RTMPSoftDecryptionAction(pAd, 
									 (PUCHAR)pHeader, 0, 
									 &pApCliEntry->SharedKey[pRxWI->KeyIndex], 
									 pRxBlk->pData, 
									 &(pRxBlk->DataSize)) == NDIS_STATUS_FAILURE)			
		{						
			return FALSE;  // give up this frame
		}
	}
	pRxD->MyBss = 1;				
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
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + IfIdx;

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

	// Update GTK
	// set key material, TxMic and RxMic for WPAPSK	
	NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].GTK, pKey, GTK_len);
	pAd->ApCfg.ApCliTab[IfIdx].DefaultKeyId = DefaultKeyIdx;

	// Update shared key table
	NdisZeroMemory(&pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx], sizeof(CIPHER_KEY));  
	pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].KeyLen = GTK_len;
	NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].Key, pKey, LEN_TK);
	if (GTK_len == LEN_TKIP_GTK)
	{
		NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].RxMic, pKey + 16, LEN_TKIP_MIC);
		NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].TxMic, pKey + 24, LEN_TKIP_MIC);
	}

	// Update Shared Key CipherAlg
	pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_NONE;
	if (pAd->ApCfg.ApCliTab[IfIdx].GroupCipher == Ndis802_11Encryption2Enabled)
		pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_TKIP;
	else if (pAd->ApCfg.ApCliTab[IfIdx].GroupCipher == Ndis802_11Encryption3Enabled)
		pAd->ApCfg.ApCliTab[IfIdx].SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_AES;

	return TRUE;
}

#endif // APCLI_SUPPORT //

