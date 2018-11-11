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
    ap_sec.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/
#include "rt_config.h"
BUILD_TIMER_FUNCTION(GroupRekeyExec);

#ifdef DOT11W_PMF_SUPPORT
VOID APPMFInit (
    IN RTMP_ADAPTER *pAd, 
    IN struct wifi_dev *wdev)
{
    struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

    /*
       IEEE 802.11W/P.10 -
       A STA that has associated with Management Frame Protection enabled
       shall not use pairwise cipher suite selectors WEP-40, WEP-104,
       TKIP, or "Use Group cipher suite".

       IEEE 802.11W/P.3 -
       IEEE Std 802.11 provides one security protocol, CCMP, for protection
       of unicast Robust Management frames.
     */
    pSecConfig->PmfCfg.MFPC = FALSE;
    pSecConfig->PmfCfg.MFPR = FALSE;
    pSecConfig->PmfCfg.PMFSHA256 = FALSE;
    if ((IS_AKM_WPA2(pSecConfig->AKMMap) || IS_AKM_WPA2PSK(pSecConfig->AKMMap))
        && IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
        && IS_CIPHER_CCMP128(pSecConfig->GroupCipher)
        && (pSecConfig->PmfCfg.Desired_MFPC))
    {
        pSecConfig->PmfCfg.MFPC = TRUE;
        pSecConfig->PmfCfg.MFPR = pSecConfig->PmfCfg.Desired_MFPR;

        if ((pSecConfig->PmfCfg.Desired_PMFSHA256) || (pSecConfig->PmfCfg.MFPR))
            pSecConfig->PmfCfg.PMFSHA256 = TRUE;
    }
    else if (pSecConfig->PmfCfg.Desired_MFPC)
    {
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __FUNCTION__));
    }

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: apidx=%d, MFPC=%d, MFPR=%d, SHA256=%d\n",
        __FUNCTION__, wdev->func_idx, pSecConfig->PmfCfg.MFPC,
        pSecConfig->PmfCfg.MFPR, pSecConfig->PmfCfg.PMFSHA256));
}
#endif /* DOT11W_PMF_SUPPORT */


INT APSecInit (
    IN RTMP_ADAPTER *pAd, 
    IN struct wifi_dev *wdev)
{
    struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

    if (pSecConfig->AKMMap == 0x0)
        SET_AKM_OPEN(pSecConfig->AKMMap);

    if (pSecConfig->PairwiseCipher == 0x0)
        SET_CIPHER_NONE(pSecConfig->PairwiseCipher);

    /* Decide Group cipher */
    if ((IS_AKM_OPEN(pSecConfig->AKMMap) || IS_AKM_SHARED(pSecConfig->AKMMap)) 
        && (IS_CIPHER_WEP(pSecConfig->PairwiseCipher)))
    {     /* WEP */
        pSecConfig->GroupCipher = pSecConfig->PairwiseCipher;
        pSecConfig->GroupKeyId = pSecConfig->PairwiseKeyId;
    } 
    else if (IS_AKM_WPA_CAPABILITY(pSecConfig->AKMMap) 
                && IS_CIPHER_TKIP(pSecConfig->PairwiseCipher))
    {     /* Mix mode */
        SET_CIPHER_TKIP(pSecConfig->GroupCipher);
    } 
    else
    {
        pSecConfig->GroupCipher = pSecConfig->PairwiseCipher;
    }

    /* Default key index is always 2 in WPA mode */
    if (IS_AKM_WPA_CAPABILITY(pSecConfig->AKMMap))
        pSecConfig->GroupKeyId = 1;

#ifdef DOT11R_FT_SUPPORT
	if (wdev->FtCfg.FtCapFlag.Dot11rFtEnable) {
		if (IS_AKM_WPA2(pSecConfig->AKMMap))
			SET_AKM_FT_WPA2(pSecConfig->AKMMap);
		if (IS_AKM_WPA2PSK(pSecConfig->AKMMap))
			SET_AKM_FT_WPA2PSK(pSecConfig->AKMMap);
	}
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
    APPMFInit(pAd, wdev);
#endif /* DOT11W_PMF_SUPPORT */

    /* Generate the corresponding RSNIE */
#ifdef HOSTAPD_SUPPORT
    if(pAd->ApCfg.MBSSID[wdev->func_id].Hostapd)
    {
        return TRUE;
    } else
#endif /* HOSTAPD_SUPPORT */
        WPAMakeRSNIE(wdev->wdev_type, &wdev->SecConfig);

    return TRUE;
}


INT APKeyTableInit (
    IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
    BSS_STRUCT *pMbss = NULL;
    struct _SECURITY_CONFIG *pSecConfig = NULL;
    ASIC_SEC_INFO Info = {0};
    USHORT Wcid = 0;
    STA_TR_ENTRY *tr_entry = NULL;

    if (wdev == NULL)
        return 0;

    /*
        Initialize security variable per entry,
        1. pairwise key table, re-set all WCID entry as NO-security mode.
        2. access control port status
    */
    /* Init Security variables */
    pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
    pSecConfig = &wdev->SecConfig;

    RTMPInitTimer(pAd, &pSecConfig->GroupRekeyTimer, GET_TIMER_FUNCTION(GroupRekeyExec), pAd,  TRUE);
    if (IS_AKM_WPA_CAPABILITY(pSecConfig->AKMMap))
        pSecConfig->GroupKeyId = 1;

    /* Get a specific WCID to record this MBSS key attribute */
    GET_GroupKey_WCID(wdev, Wcid);

    tr_entry = &pAd->MacTab.tr_entry[Wcid];
    wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
    tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

    /* Set key material to Asic */
    os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
    Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
    Info.Direction = SEC_ASIC_KEY_TX;
    Info.Wcid = Wcid;
    Info.BssIndex = wdev->func_idx;
    Info.Cipher = pSecConfig->GroupCipher;
    Info.KeyIdx = pSecConfig->GroupKeyId;
    os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);

    /* When WEP, TKIP or AES is enabled, set group key info to Asic */
    if (IS_CIPHER_WEP(pSecConfig->GroupCipher))
    {
        INT i;
        /* Generate 3-bytes IV randomly for software encryption using */
        for(i = 0; i < LEN_WEP_TSC; i++)
            pSecConfig->WepKey[Info.KeyIdx].TxTsc[i] = RandomByte(pAd);

        os_move_mem(&Info.Key,&pSecConfig->WepKey[Info.KeyIdx],sizeof(SEC_KEY_INFO));

        HW_ADDREMOVE_KEYTABLE(pAd, &Info);

        /* Update WCID attribute table and IVEIV table */
        RTMPSetWcidSecurityInfo(pAd,
                                                            wdev->func_idx,
                                                            Info.KeyIdx,
                                                            pSecConfig->GroupCipher,
                                                            Wcid,
                                                            SHAREDKEYTABLE);
    }
    else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher)
                   || IS_CIPHER_CCMP128(pSecConfig->GroupCipher)
                   || IS_CIPHER_CCMP256(pSecConfig->GroupCipher)
                   || IS_CIPHER_GCMP128(pSecConfig->GroupCipher)
                   || IS_CIPHER_GCMP256(pSecConfig->GroupCipher)) 
    {
        struct _SEC_KEY_INFO *pGroupKey = &Info.Key;

        /* Calculate PMK */
        SetWPAPSKKey(pAd, pSecConfig->PSK, strlen(pSecConfig->PSK), (PUCHAR) pMbss->Ssid, pMbss->SsidLen, pSecConfig->PMK);

        /* Generate GMK and GNonce randomly per MBSS */
        GenRandom(pAd, wdev->bssid, pSecConfig->GMK);
        GenRandom(pAd, wdev->bssid, pSecConfig->Handshake.GNonce);

        /* Derive GTK per BSSID */
        WpaDeriveGTK(pSecConfig->GMK,
                                        (UCHAR  *) pSecConfig->Handshake.GNonce,
                                        wdev->bssid,
                                        (UCHAR *) pSecConfig->GTK,
                                        LEN_MAX_GTK);

    /* Install Shared key */
    os_move_mem(pGroupKey->Key,pSecConfig->GTK,LEN_MAX_GTK);

#ifdef DOT11W_PMF_SUPPORT
        if (pSecConfig->PmfCfg.MFPC == TRUE)
        {
            /* IGTK default key index as 4 */
            pSecConfig->PmfCfg.IGTK_KeyIdx = 4;
            /* Clear IPN */
            os_move_mem(Info.IGTK, &pSecConfig->PmfCfg.IPN[0][0], LEN_WPA_TSC);
            /* Derive IGTK */
            PMF_DeriveIGTK(pAd, &pSecConfig->PmfCfg.IGTK[0][0]);
            os_move_mem(Info.IGTK, &pSecConfig->PmfCfg.IGTK[0][0], LEN_TK);
            Info.IGTKKeyLen = LEN_TK;
        }
#endif /* DOT11W_PMF_SUPPORT */

        WPAInstallKey(pAd, &Info, TRUE);
        pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
    }
#ifdef WAPI_SUPPORT
    else if (IS_CIPHER_WPI_SMS4(pSecConfig->GroupCipher))
    {
        INT cnt;

         /* Calculate PMK */
        SetWPAPSKKey(pAd, pSecConfig->PSK, strlen(pSecConfig->PSK), (PUCHAR) pMbss->Ssid, pMbss->SsidLen, pSecConfig->PMK);

        /* Initial the related variables */
        pSecConfig->GroupKeyId = 0;
        NdisMoveMemory(pSecConfig->key_announce_flag, AE_BCAST_PN, LEN_WAPI_TSC);
        if (IS_HW_WAPI_SUPPORT(pAd))
            pSecConfig->sw_wpi_encrypt = FALSE;
        else
            pSecConfig->sw_wpi_encrypt = TRUE;

        /* Generate NMK randomly */
        for (cnt = 0; cnt < LEN_WAPI_NMK; cnt++)
            pSecConfig->NMK[cnt] = RandomByte(pAd);

        /* Count GTK for this BSSID */
        RTMPDeriveWapiGTK(pSecConfig->NMK, pSecConfig->GTK);

        /* Install Shared key */
        WAPIInstallSharedKey(pAd, pSecConfig->GroupCipher, wdev->func_idx, pSecConfig->GroupKeyId, Wcid, pSecConfig->GTK);
    }
#endif /* WAPI_SUPPORT */

    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("### BSS(%d) AKM=0x%x, PairwiseCipher=0x%x, GroupCipher=0x%x \n",
            wdev->func_idx, pSecConfig->AKMMap, pSecConfig->PairwiseCipher, pSecConfig->GroupCipher));

    return TRUE;
}


VOID GroupRekeyExec (
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    UINT i, apidx;
    ULONG temp_counter = 0;
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
    PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;
    struct wifi_dev *wdev = NULL;
    struct _SECURITY_CONFIG *pSecConfig = NULL;

    for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
    {
        if (&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.GroupRekeyTimer == pTimer)
            break;
    }

    if (apidx == pAd->ApCfg.BssidNum)
        return;
	
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    pSecConfig = &wdev->SecConfig;

    if (pSecConfig->GroupReKeyInterval == 0) 
        return;

    if (pSecConfig->Handshake.GTKState == REKEY_NEGOTIATING)
    {
        pSecConfig->GroupReKeyInstallCountDown--;
        if (pSecConfig->GroupReKeyInstallCountDown == 0)
            goto INSTALL_KEY;
    }

    if (pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_TIME)
        temp_counter = (++pSecConfig->GroupPacketCounter);
    else if (pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_PACKET)
        temp_counter = pSecConfig->GroupPacketCounter/1000;  /* Packet-based: kilo-packets */
    else
        return;
	
    if (temp_counter > pSecConfig->GroupReKeyInterval)
    {
        UINT entry_count = 0;

        pSecConfig->GroupPacketCounter = 0;
        pSecConfig->Handshake.GTKState = REKEY_NEGOTIATING;
	
        /* change key index */
        pSecConfig->GroupKeyId = (pSecConfig->GroupKeyId == 1) ? 2 : 1;

        /* Generate GNonce randomly per MBSS */
        GenRandom(pAd, wdev->bssid, pSecConfig->Handshake.GNonce);
		
        /* Derive GTK per BSSID */
        WpaDeriveGTK(pSecConfig->GMK,
                                (UCHAR	*) pSecConfig->Handshake.GNonce,
                                wdev->bssid,
                                (UCHAR *) pSecConfig->GTK,
                                LEN_MAX_GTK);

        /* Process 2-way handshaking */
        for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
        {
            MAC_TABLE_ENTRY  *pEntry = &pAd->MacTab.Content[i];

            if (IS_ENTRY_CLIENT(pEntry) 
                        && (pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) 
                        && (pEntry->func_tb_idx == apidx))
            {
#ifdef MWDS
				if(IS_MWDS_OPMODE_AP(pEntry))
					continue;
#endif /* MWDS */
                entry_count++;
                RTMPSetTimer(&pEntry->SecConfig.StartFor2WayTimer, ENQUEUE_EAPOL_2WAY_START_TIMER);
                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rekey interval excess, Update Group Key for  %02X:%02X:%02X:%02X:%02X:%02X , DefaultKeyId= %x \n",\
                            PRINT_MAC(pEntry->Addr), pSecConfig->GroupKeyId));
            }
        }

        if (entry_count == 0)
            goto INSTALL_KEY;
        else
            pSecConfig->GroupReKeyInstallCountDown = 1; /* 1 seconds */
    }

    return;

INSTALL_KEY:	
    /* If no sta connect, directly install group rekey, else install key after 2 way completed or 1 seconds */
    {
        ASIC_SEC_INFO Info = {0};
        USHORT Wcid;

        /* Get a specific WCID to record this MBSS key attribute */
        GET_GroupKey_WCID(wdev, Wcid);

        /* Set key material to Asic */
        os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
        Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
        Info.Direction = SEC_ASIC_KEY_TX;
        Info.Wcid = Wcid;
        Info.BssIndex = apidx;
        Info.Cipher = pSecConfig->GroupCipher;
        Info.KeyIdx = pSecConfig->GroupKeyId;
        os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);

        /* Install Shared key */
        os_move_mem(Info.Key.Key,pSecConfig->GTK,LEN_MAX_GTK);
        WPAInstallKey(pAd, &Info, TRUE);
        pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
    }
}		

VOID WPAGroupRekeyByWdev (
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev)

{
    struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
    if (IS_CIPHER_TKIP(pSecConfig->GroupCipher)
        || IS_CIPHER_CCMP128(pSecConfig->GroupCipher)
        || IS_CIPHER_CCMP256(pSecConfig->GroupCipher)
        || IS_CIPHER_GCMP128(pSecConfig->GroupCipher)
        || IS_CIPHER_GCMP256(pSecConfig->GroupCipher))
    {
            /* Group rekey related */
            if ((pSecConfig->GroupReKeyInterval != 0) 
                && ((pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_TIME) 
                    || (pSecConfig->GroupReKeyMethod == SEC_GROUP_REKEY_PACKET))) 
            {
                pSecConfig->GroupPacketCounter = 0;
                //Ellis note:avoid AP crash due to set timer twice when ra1 up before ra0 up   
                RTMPModTimer(&pSecConfig->GroupRekeyTimer, GROUP_KEY_UPDATE_EXEC_INTV);
                MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s : Group rekey method= %d , interval = 0x%lx\n",
                    __FUNCTION__, pSecConfig->GroupReKeyMethod, pSecConfig->GroupReKeyInterval));
            }
    }
}

/*
	Set group re-key timer if necessary.
	It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
*/
VOID APStartRekeyTimer (
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev)
{
	if(HcIsRadioAcq(wdev))  
	{
#ifdef WAPI_SUPPORT
		RTMPStartWapiRekeyTimerByWdev(pAd,wdev);
#endif /* WAPI_SUPPORT */

		WPAGroupRekeyByWdev(pAd,wdev);
	}
}

VOID APStopRekeyTimer (
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev)
{
	BOOLEAN Cancelled;

#ifdef WAPI_SUPPORT
	RTMPCancelWapiRekeyTimerByWdev(pAd,wdev);
#endif /* WAPI_SUPPORT */

	RTMPCancelTimer(&wdev->SecConfig.GroupRekeyTimer, &Cancelled);
}

VOID APReleaseRekeyTimer (
    IN PRTMP_ADAPTER pAd,
    IN struct wifi_dev *wdev)
{
	BOOLEAN Cancelled;

#ifdef WAPI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		RTMPCancelTimer(&pSecConfig->WapiMskRekeyTimer, &Cancelled);
	}
#endif /* WAPI_SUPPORT */

	RTMPReleaseTimer(&wdev->SecConfig.GroupRekeyTimer, &Cancelled);
}


static PCHAR portsecured[]={"NONE", "PORT_SECURED", "NOT_SECURED"};
INT Show_APSecurityInfo_Proc (
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg)
{
	UCHAR idx;
	USHORT Wcid;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
    struct wifi_dev *wdev = NULL;
    	STA_TR_ENTRY *tr_entry = NULL;

	if (!pAd)
		return FALSE; 

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security Infomation: AP\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS\tWCID\tAuthMode\tPairwiseCipher\tGroupCipher\tGroupKeyId\tPortSecured\n"));

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		pSecConfig = &pAd->ApCfg.MBSSID[idx].wdev.SecConfig;
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		GET_GroupKey_WCID(wdev, Wcid);
		tr_entry = &pAd->MacTab.tr_entry[Wcid];
		
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d\t%d\t%s\t\t%s\t\t%s\t\t%d\t\t%s\n",
							idx,
							Wcid,
							GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
							GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
							GetEncryModeStr(GET_GROUP_CIPHER(pSecConfig)),
							pSecConfig->GroupKeyId,
							portsecured[tr_entry->PortSecured]));

	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

#ifdef APCLI_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security Infomation: AP Client\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS\tWCID\tAuthMode\tPairwiseCipher\tPortSecured\n"));

	for (idx = 0; idx < MAX_APCLI_NUM; idx++)
	{
		PAPCLI_STRUCT  pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
		pSecConfig = &pApCliEntry->wdev.SecConfig;
		wdev = &pApCliEntry->wdev;
		tr_entry = &pAd->MacTab.tr_entry[wdev->bss_info_argument.ucBcMcWlanIdx];
		if (pApCliEntry->Enable == TRUE)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d\t%d\t%s\t\t%s\t\t%s\n",
						idx,
						wdev->bss_info_argument.ucBcMcWlanIdx,
						GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
						GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
						portsecured[tr_entry->PortSecured]));
		}
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
#endif

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security Infomation: STA\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS\t\t\tAID\tWCID\tAuthMode\tPairwiseCipher\tPortSecured\n"));

	for (idx=0; idx<MAX_LEN_OF_MAC_TABLE; idx++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[idx];
		pSecConfig = &pEntry->SecConfig;
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
		{
			tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X\t%d\t%d\t%s\t\t%s\t\t%s\n",
						PRINT_MAC(pEntry->Addr),
						pEntry->Aid,
						pEntry->wcid,
						GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
						GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
						portsecured[tr_entry->PortSecured]));
			if ((!IS_AKM_OPEN(GET_SEC_AKM(pSecConfig))) && tr_entry->PortSecured == 1)
			{
				INT32 i,key_len=32;
				if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_TKIP_TK;
				}
				else if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_CCMP128_TK;
				}
				else if (IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_CCMP256_TK;
				}
				else if (IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_GCMP128_TK;
				}
				else if (IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_GCMP256_TK;
				}
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  PTK:"));
#define RED(_text)  "\033[1;31m"_text"\033[0m"
				for (i=0;i<64;i++) {
					if ((i>=32) && (i<(key_len+32))) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (RED("%02x"),pSecConfig->PTK[i]));
					} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x",pSecConfig->PTK[i]));
					}
				}
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			}
		}
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

#ifdef APCLI_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security Infomation: ApCli/Repeater\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSS\t\t\tAID\tWCID\tAuthMode\tPairwiseCipher\tPortSecured\n"));
	
	for (idx=0; idx<MAX_LEN_OF_MAC_TABLE; idx++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[idx];
		pSecConfig = &pEntry->SecConfig;
		if (pEntry && (IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_APCLI(pEntry)) && pEntry->Sst == SST_ASSOC)
		{
			tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X\t%d\t%d\t%s\t\t%s\t\t%s\n",
					 PRINT_MAC(pEntry->Addr),
					 pEntry->Aid,
					 pEntry->wcid,
					 GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
					 GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
					 portsecured[tr_entry->PortSecured]));
			if ((!IS_AKM_OPEN(GET_SEC_AKM(pSecConfig))) && tr_entry->PortSecured == 1)
			{
				INT32 i,key_len=32;
				if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_TKIP_TK;
				}
				else if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_CCMP128_TK;
				}
				else if (IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_CCMP256_TK;
				}
				else if (IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_GCMP128_TK;
				}
				else if (IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher))
				{
					key_len = LEN_GCMP256_TK;
				}
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  PTK:"));
#define RED(_text)  "\033[1;31m"_text"\033[0m"
				for (i=0;i<64;i++) {
					if ((i>=32) && (i<(key_len+32))) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (RED("%02x"),pSecConfig->PTK[i]));
					} else {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x",pSecConfig->PTK[i]));
					}
				}
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			}
		}
	}
#endif

	return TRUE;
}

VOID CheckBMCPortSecured(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN BOOLEAN isConnect)
{
	UINT32 bss_index = pEntry->func_tb_idx;
	UINT32 wcid;
	UCHAR PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[bss_index].wdev;
	
	if (wdev->tr_tb_idx == 0xff)
		return;//skip uninit tr_tb_idx.
	if (isConnect) {
		PortSecured = WPA_802_1X_PORT_SECURED;
	}
	else {
		RETURN_IF_PAD_NULL(pAd);
		for (wcid = 1; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
			pMacEntry = &pAd->MacTab.Content[wcid];
			tr_entry = &pAd->MacTab.tr_entry[wcid];

			/*if (wcid == pEntry->wcid)
				continue;*/

			if (((pMacEntry)
				&& (IS_ENTRY_CLIENT(pMacEntry))
				&& (pMacEntry->Sst == SST_ASSOC)
				&& (pMacEntry->func_tb_idx == bss_index)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
#ifdef CONFIG_FPGA_MODE
					|| (pAd->fpga_ctl.fpga_on & 0x1)
#endif /* CONFIG_FPGA_MODE */
				) {
				PortSecured = WPA_802_1X_PORT_SECURED;
				break;
			}
		}
	}
	if (wdev->PortSecured != PortSecured) {
		tr_entry = &pAd->MacTab.tr_entry[wdev->bss_info_argument.ucBcMcWlanIdx];
		tr_entry->PortSecured = PortSecured;
		wdev->PortSecured = PortSecured;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("%s: bss_index = %d, wcid = %d, PortSecured = %d\n",
					__FUNCTION__,
					bss_index,
					wdev->bss_info_argument.ucBcMcWlanIdx,
					PortSecured)); 
		
	}
}

#ifdef DOT1X_SUPPORT
/*
    ========================================================================

    Routine Description:
        Sending EAP Req. frame to station in authenticating state.
        These frames come from Authenticator deamon.

    Arguments:
        pAdapter        Pointer to our adapter
        pPacket     Pointer to outgoing EAP frame body + 8023 Header
        Len             length of pPacket
        
    Return Value:
        None
    ========================================================================
*/
VOID WpaSend(RTMP_ADAPTER *pAdapter, UCHAR *pPacket, ULONG Len)
{
	PEAP_HDR pEapHdr;
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR Header802_3[LENGTH_802_3];
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	PUCHAR pData;
        
    NdisMoveMemory(Addr, pPacket, 6);
	NdisMoveMemory(Header802_3, pPacket, LENGTH_802_3);
    pEapHdr = (EAP_HDR*)(pPacket + LENGTH_802_3);
	pData = (pPacket + LENGTH_802_3);
	
    if ((pEntry = MacTableLookup(pAdapter, Addr)) == NULL)
    {	
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WpaSend - No such MAC - %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(Addr)));
		return;
    }

	tr_entry = &pAdapter->MacTab.tr_entry[pEntry->wcid];
	/* Send EAP frame to STA */
    if ((IS_AKM_WPA_CAPABILITY_Entry(pEntry) && (pEapHdr->ProType != EAPOLKey)) ||
        (IS_IEEE8021X(&pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig)))
		RTMPToWirelessSta(pAdapter, 
						  pEntry, 
						  Header802_3, 
						  LENGTH_802_3, 
						  pData, 
						  Len - LENGTH_802_3, 
						  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);		
	
    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d), pEapHdr->code=%d, pEntry->SecConfig.Handshake.WpaState=%d\n", __FUNCTION__, __LINE__, pEapHdr->code, pEntry->SecConfig.Handshake.WpaState));
	
    if (RTMPEqualMemory((pPacket+12), EAPOL, 2))
    {
        switch (pEapHdr->code)
        {
		case EAP_CODE_REQUEST:
			if ((pEntry->SecConfig.Handshake.WpaState >= AS_PTKINITDONE) && (pEapHdr->ProType == EAPPacket))
			{
				pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Start to re-authentication by 802.1x daemon\n"));					
			}
			break;

			/* After receiving EAP_SUCCESS, trigger state machine */	
            case EAP_CODE_SUCCESS:
                if (IS_AKM_WPA_CAPABILITY_Entry(pEntry) && (pEapHdr->ProType != EAPOLKey))
                {
                    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Send EAP_CODE_SUCCESS\n\n"));
                    if (pEntry->Sst == SST_ASSOC)
                    {
			pEntry->SecConfig.Handshake.WpaState = AS_INITPMK;
			pEntry->SecConfig.Handshake.MsgRetryCounter = 0;
			os_move_mem(&pEntry->SecConfig.Handshake.AAddr, pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid,MAC_ADDR_LEN);
			os_move_mem(&pEntry->SecConfig.Handshake.SAddr, pEntry->Addr,MAC_ADDR_LEN);
			os_move_mem(&pEntry->SecConfig.PMK, &pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PMK,LEN_PMK);
			WPABuildPairMsg1(pAdapter, &pEntry->SecConfig, pEntry);
                    }
                }
                else
                {
                    pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
                    pEntry->SecConfig.Handshake.WpaState = AS_PTKINITDONE;
                    pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.PortSecured = WPA_802_1X_PORT_SECURED;
                    tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef WSC_AP_SUPPORT
                    if (pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.WscConfMode != WSC_DISABLE)
                        WscInformFromWPA(pEntry);
#endif /* WSC_AP_SUPPORT */
                    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IEEE8021X-WEP : Send EAP_CODE_SUCCESS\n\n"));
                }
                break;

            case EAP_CODE_FAILURE:
                break;

            default:
                break;    
        }
    }
    else     
    {
        MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Send Deauth, Reason : REASON_NO_LONGER_VALID\n"));
        MlmeDeAuthAction(pAdapter, pEntry, REASON_NO_LONGER_VALID, FALSE);
    }
}    


INT RTMPAddPMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *PMKID,
	IN UCHAR *PMK)
{
	INT i, CacheIdx;

	/* Update PMKID status */
	CacheIdx = RTMPSearchPMKIDCache(pPMKIDCache, apidx, pAddr);
	if (CacheIdx != INVALID_PMKID_IDX)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): cache found and renew it(%d)\n",
			__FUNCTION__, CacheIdx));
	}
	else
	{
		ULONG ts = 0;
		INT old_entry = 0;
		/* Add a new PMKID */
		for (i = 0; i < MAX_PMKID_COUNT; i++)
		{
			if (pPMKIDCache->BSSIDInfo[i].Valid == FALSE)
			{
				CacheIdx = i;
				break;
			} else {
				if ((ts == 0) || (ts > pPMKIDCache->BSSIDInfo[i].RefreshTime))
				{
					ts = pPMKIDCache->BSSIDInfo[i].RefreshTime;
					old_entry = i;
				}
			}
		}
	 
		if (i == MAX_PMKID_COUNT)
		{
			CacheIdx = old_entry;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s():Cache full, replace oldest(%d)\n",
				__FUNCTION__, old_entry));
		}
	}

	pPMKIDCache->BSSIDInfo[CacheIdx].Valid = TRUE;
	pPMKIDCache->BSSIDInfo[CacheIdx].Mbssidx = apidx;
	NdisGetSystemUpTime(&(pPMKIDCache->BSSIDInfo[CacheIdx].RefreshTime));
	COPY_MAC_ADDR(&pPMKIDCache->BSSIDInfo[CacheIdx].MAC, pAddr);
	NdisMoveMemory(&pPMKIDCache->BSSIDInfo[CacheIdx].PMKID, PMKID, LEN_PMKID);
	NdisMoveMemory(&pPMKIDCache->BSSIDInfo[CacheIdx].PMK, PMK, PMK_LEN);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s(): add %02x:%02x:%02x:%02x:%02x:%02x cache(%d) for ra%d\n", 
		__FUNCTION__, PRINT_MAC(pAddr), CacheIdx, apidx));

	return CacheIdx;
}


INT RTMPSearchPMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr)
{
	INT	i = 0;
	
	for (i = 0; i < MAX_PMKID_COUNT; i++)
	{
		if ((pPMKIDCache->BSSIDInfo[i].Valid == TRUE)
			&& (pPMKIDCache->BSSIDInfo[i].Mbssidx == apidx)
			&& MAC_ADDR_EQUAL(&pPMKIDCache->BSSIDInfo[i].MAC, pAddr))
		{
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("%s():%02x:%02x:%02x:%02x:%02x:%02x cache(%d) from IF(ra%d)\n", 
            						__FUNCTION__, PRINT_MAC(pAddr), i, apidx));
			break;
		}
	}

	if (i >= MAX_PMKID_COUNT)
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s(): - IF(%d) not found\n", __FUNCTION__, apidx));
		return INVALID_PMKID_IDX;
	}

	return i;
}


INT RTMPValidatePMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN UCHAR *pAddr,
	IN UCHAR *pPMKID)
{
	INT CacheIdx;
	if ((CacheIdx = RTMPSearchPMKIDCache(pPMKIDCache, apidx, pAddr)) == INVALID_PMKID_IDX)
		return INVALID_PMKID_IDX;

	if (RTMPEqualMemory(pPMKID, &pPMKIDCache->BSSIDInfo[CacheIdx].PMKID, LEN_PMKID))
		return CacheIdx;
	else
		return INVALID_PMKID_IDX;
}


VOID RTMPDeletePMKIDCache(
	IN NDIS_AP_802_11_PMKID *pPMKIDCache,
	IN INT apidx,
	IN INT idx)
{
	PAP_BSSID_INFO pInfo = &pPMKIDCache->BSSIDInfo[idx];

	if (pInfo->Valid && (pInfo->Mbssidx == apidx))
	{
		pInfo->Valid = FALSE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s():(IF(%d), del PMKID CacheIdx=%d\n", __FUNCTION__, apidx, idx));
	}
}


VOID RTMPMaintainPMKIDCache(
	IN RTMP_ADAPTER *pAd)
{
	INT i;
	ULONG Now;
	
	for (i = 0; i < MAX_PMKID_COUNT; i++)
	{
		PAP_BSSID_INFO pBssInfo = &pAd->ApCfg.PMKIDCache.BSSIDInfo[i];

		NdisGetSystemUpTime(&Now);

		if ((pBssInfo->Valid)
			&& /*((Now - pBssInfo->RefreshTime) >= pMbss->PMKCachePeriod)*/
			(RTMP_TIME_AFTER(Now, (pBssInfo->RefreshTime + pAd->ApCfg.MBSSID[pBssInfo->Mbssidx].PMKCachePeriod))))
		{
			RTMPDeletePMKIDCache(&pAd->ApCfg.PMKIDCache, pBssInfo->Mbssidx, i);
		}
	}
}
#endif /* DOT1X_SUPPORT */

