/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wapi.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Albert		2008-4-3      	Supoort WAPI protocol
*/
/*#include <linux/stdio.h> */
/*#include <linux/stdlib.h> */
/*#include <linux/string.h> */
/*#include <linux/time.h> */

#ifdef WAPI_SUPPORT

#include "rt_config.h"

/* WAPI AKM OUI */
UCHAR   OUI_WAI_CERT_AKM[4]   	= {0x00, 0x14, 0x72, 0x01};
UCHAR   OUI_WAI_PSK_AKM[4]   	= {0x00, 0x14, 0x72, 0x02};

/* WAPI CIPHER OUI */
UCHAR	OUI_WPI_CIPHER_SMS4[4] = {0x00, 0x14, 0x72, 0x01};

UCHAR	WAPI_TYPE[] = {0x88, 0xb4};

/* IV default value */
UCHAR 	AE_BCAST_PN[LEN_WAPI_TSC] = {0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
									 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c};
UCHAR 	ASUE_UCAST_PN[LEN_WAPI_TSC] = {0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
									   0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c};
UCHAR 	AE_UCAST_PN[LEN_WAPI_TSC] = {0x37, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c,
									 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c, 0x36, 0x5c};

BUILD_TIMER_FUNCTION(RTMPWapiUskRekeyPeriodicExec);
BUILD_TIMER_FUNCTION(RTMPWapiMskRekeyPeriodicExec);

static void kd_hmac_sha256(	
    unsigned char 	*key, 
    unsigned int 	key_len,
    unsigned char 	*text, 
	unsigned int 	text_len,
    unsigned char 	*output, 
    unsigned int 	output_len)
{
	int i;

	for (i = 0; output_len/SHA256_DIGEST_SIZE; i++, output_len -= SHA256_DIGEST_SIZE)
	{
		RT_HMAC_SHA256(key, key_len, text, text_len, &output[i*SHA256_DIGEST_SIZE], SHA256_DIGEST_SIZE);
		text = &output[i*SHA256_DIGEST_SIZE];
		text_len = SHA256_DIGEST_SIZE;
	}

	if (output_len > 0)
		RT_HMAC_SHA256(key, key_len, text, text_len, &output[i*SHA256_DIGEST_SIZE], output_len);

}


/*
	========================================================================
	
	Routine Description:
		Build WAPI IE in RSN-IE. 
		It only shall be called by RTMPMakeRSNIE. 

	Arguments:		
		pAd			-	pointer to our pAdapter context	
  		AuthMode	-	indicate the authentication mode 
    	WepStatus	-	indicate the encryption type
		
	Return Value:
		
	Note:
		
	========================================================================
*/
VOID RTMPInsertWapiIe(	
	IN	UINT			AuthMode,
	IN	UINT			WepStatus,
	OUT	PUCHAR			pWIe,
	OUT	UCHAR			*w_len)
{			
	WAPIIE	*pWapiHdr = (WAPIIE*)pWIe;
	WAPIIE_UCAST *pWIE_ucast;
	WAPIIE_MCAST *pWIE_mcast;

	*w_len = 0;

	/* Assign the verson as 1 */
	pWapiHdr->version = 1;

	/* Set the AKM count and suite */
	pWapiHdr->acount = 1;
	switch (AuthMode)
	{
		case Ndis802_11AuthModeWAICERT:
			NdisMoveMemory(pWapiHdr->auth[0].oui, OUI_WAI_CERT_AKM, 4);
			break;

		case Ndis802_11AuthModeWAIPSK:
			NdisMoveMemory(pWapiHdr->auth[0].oui, OUI_WAI_PSK_AKM, 4);
			break;
	}

	/* swap for big-endian platform */
	pWapiHdr->version = cpu2le16(pWapiHdr->version);
	pWapiHdr->acount = cpu2le16(pWapiHdr->acount);
	
	/* update current length */
	(*w_len) += sizeof(WAPIIE);	

	/* Set the unicast cipher and count */
	pWIE_ucast = (WAPIIE_UCAST*)(pWIe + (*w_len));
	pWIE_ucast->ucount = 1;
	NdisMoveMemory(pWIE_ucast->ucast[0].oui, OUI_WPI_CIPHER_SMS4, 4);

	/* swap for big-endian platform */
	pWIE_ucast->ucount = cpu2le16(pWIE_ucast->ucount);

	/* update current length */
	(*w_len) += sizeof(WAPIIE_UCAST);

	/* Set the multicast cipher and capability */
	pWIE_mcast = (WAPIIE_MCAST*)(pWIe + (*w_len));
	NdisMoveMemory(pWIE_mcast->mcast, OUI_WPI_CIPHER_SMS4, 4);
	pWIE_mcast->capability = 0;	/* Todo AlbertY - support pre-authentication */

	/* update current length */
	(*w_len) += sizeof(WAPIIE_MCAST);

}

/*
    ==========================================================================
    Description:
		Check whether the received frame is WAPI frame.

	Arguments:
		pAd				-	pointer to our pAdapter context			
		pData			-	the received frame
		DataByteCount 	-	the received frame's length				
       
    Return:
         TRUE 			-	This frame is WAPI frame
         FALSE 			-	otherwise
    ==========================================================================
*/
BOOLEAN RTMPCheckWAIframe(
    IN PUCHAR           pData,
    IN ULONG            DataByteCount)
{
    if(DataByteCount < (LENGTH_802_1_H + LENGTH_WAI_H))
        return FALSE;


	/* Skip LLC header */
    if (NdisEqualMemory(SNAP_802_1H, pData, 6)) 
    {
        pData += 6;
    }
	/* Skip 2-bytes EAPoL type */
    if (NdisEqualMemory(WAPI_TYPE, pData, 2)) 
    {
    	MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_TRACE, ("--> Receive a WAI frame \n"));
        pData += 2;         
    }
    else    
        return FALSE;
	
    return TRUE;
}


/*
    ==========================================================================
    Description:
		Check whether the cipher is SMS4.

	Arguments:
		pAd				-	pointer to our pAdapter context			
		apidx			-	interface index				
       
    Return:
         TRUE 			-	The cipher is SMS4
         FALSE 			-	otherwise
    ==========================================================================
*/
BOOLEAN RTMPIsWapiCipher(
    IN PRTMP_ADAPTER    pAd,
    IN UCHAR           	apidx)
{
	UINT32 cipher_mode=0;
	
	/* Currently, WAPI only support MBSS */
	if (apidx >= MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (apidx < MAX_MBSSID_NUM(pAd))
			cipher_mode = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher;
	}
#endif /* CONFIG_AP_SUPPORT */

	if (IS_CIPHER_WPI_SMS4(cipher_mode))
		return TRUE;
	
    return FALSE;
}

/*
    ==========================================================================
    Description:
		Insert the WPI-SMS4 IV header

		+-------+------+-------------+
		| KeyId | resv | sequence PN |
		+-------+------+-------------+

	Arguments:
		
    Return:
         
    ==========================================================================
*/
VOID RTMPConstructWPIIVHdr(
	IN	UCHAR			key_id,
	IN	UCHAR			*tx_iv,
	OUT UCHAR 			*iv_hdr)
{
	iv_hdr[0] = key_id;
	iv_hdr[1] = 0x00;

	NdisMoveMemory(&iv_hdr[2], tx_iv, LEN_WAPI_TSC);
}

VOID RTMPDeriveWapiGTK(
	IN	PUCHAR			nmk,
	OUT	PUCHAR			gtk_ptr)
{
	const char group_context[100] = "multicast or station key expansion for station unicast and multicast and broadcast";		

	NdisZeroMemory(gtk_ptr, 32);
	kd_hmac_sha256(nmk, 
				   16, 
				   (UCHAR *)group_context, 
				   strlen(group_context), 
				   gtk_ptr,
				   32);	
}

VOID RT_SMS4_TEST(
	IN UINT8			test)
{
	CIPHER_KEY		CipherKey;
	UINT16			data_len;
	UINT8			rcvd_data[50];
	UINT8 mac_hdr_qos[] = {0x88, 0x42, 0x00, 0x00, 0x08, 0xda, 0x75, 0x84, 
						0xd0, 0xcc, 0x27, 0xe8, 0x72, 0xaa, 0x2c, 0xb9, 
						0x6b, 0xbb, 0xea, 0x35, 0xa4, 0x20, 0x1e, 0xd2, 
						0xcf, 0x14};
	
	UINT8 payload_qos[] = {0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					   0x00};
	UINT8 pn[] = 	 {0x98, 0xba, 0xdc, 0xfe, 0x10, 0x32, 0x54, 0x76, 
					  0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89};
	UINT8 key[] = {0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
				   0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
				   0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01,
				   0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe};
		
	RTMPSoftEncryptSMS4(mac_hdr_qos, 
						payload_qos, 
						1, 
						1, 
						key, 
						pn);

	hex_dump("encrypted payload", payload_qos, 17);

	NdisZeroMemory(&CipherKey, sizeof(CIPHER_KEY));
	NdisMoveMemory(CipherKey.Key, key, 16);
	NdisMoveMemory(CipherKey.TxMic, &key[16], 8);
	NdisMoveMemory(CipherKey.RxMic, &key[24], 8);	
	CipherKey.KeyLen = 16;


	NdisZeroMemory(rcvd_data, 50);
	rcvd_data[0] = 1;
	data_len = 2;
	NdisMoveMemory(&rcvd_data[data_len], pn, 16);
	data_len += 16;
	NdisMoveMemory(&rcvd_data[data_len], payload_qos, 17);
	data_len += 17;


	if (RTMPSoftDecryptSMS4(mac_hdr_qos, 
							FALSE, 
							&CipherKey, 
							rcvd_data, 
							&data_len) == 0)
		hex_dump("decrypted payload", rcvd_data, data_len);
	else
		printk("decrypted fail\n");
}

/*
    ========================================================================

    Routine Description:
        In kernel mode read parameters from file

    Arguments:
        src                     the location of the file.
        dest                        put the parameters to the destination.
        Length                  size to read.

    Return Value:
        None

    Note:

    ========================================================================
*/
void rtmp_read_wapi_parms_from_file(
    IN PRTMP_ADAPTER pAd, 
    IN RTMP_STRING *tmpbuf, 
    IN RTMP_STRING *buffer)
{	
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
        struct _SECURITY_CONFIG *pSecConfig = NULL;
        INT apidx;
        RTMP_STRING *macptr = NULL;
        RTMP_STRING tok_str[32];

        /* wapi interface name */
        if (RTMPGetKeyParameter("Wapiifname", tmpbuf, 32, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                NdisMoveMemory(pSecConfig->comm_wapi_info.wapi_ifname, (CHAR *) macptr, strlen(macptr));
                pSecConfig->comm_wapi_info.wapi_ifname_len = strlen(macptr); 

                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> Wapiifname=%s, len=%d",
                                INF_MBSSID_DEV_NAME, 
                                apidx, pSecConfig->comm_wapi_info.wapi_ifname, 
                                pSecConfig->comm_wapi_info.wapi_ifname_len));
            }
        }

        /* WapiAsCertPath */
        if (RTMPGetKeyParameter("WapiAsCertPath", tmpbuf, 128, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                NdisMoveMemory(pSecConfig->comm_wapi_info.as_cert_path[0], (CHAR *) macptr, strlen(macptr));
                pSecConfig->comm_wapi_info.as_cert_path_len[0] = strlen(macptr);
                pSecConfig->comm_wapi_info.as_cert_no = 1;

                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiAsCertPath=%s, len=%d",
                                INF_MBSSID_DEV_NAME, 
                                apidx, pSecConfig->comm_wapi_info.as_cert_path[0], 
                                pSecConfig->comm_wapi_info.as_cert_path_len[0]));
		}
        }

        /* WapiAsCertPath2 ~ WapiAsCertPath10 */
        for (apidx = 1; apidx < MAX_ID_NO; apidx++)
        {
            sprintf(tok_str, "WapiAsCertPath%d", apidx + 1);

            if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE))
            {
                if (strlen(tmpbuf) > 0)
                {
                    pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                    NdisMoveMemory(pSecConfig->comm_wapi_info.as_cert_path[0], macptr, strlen(tmpbuf));
                    pSecConfig->comm_wapi_info.as_cert_path_len[0] = strlen(tmpbuf);
                    pSecConfig->comm_wapi_info.as_cert_no = 1;

                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiAsCertPath=%s, len=%d",
                                INF_MBSSID_DEV_NAME, 
                                apidx, pSecConfig->comm_wapi_info.as_cert_path[0], 
                                pSecConfig->comm_wapi_info.as_cert_path_len[0]));
                }
            }
        }

        /* WapiCaCertPath */
        if (RTMPGetKeyParameter("WapiCaCertPath", tmpbuf, 128, buffer, TRUE))
        {
            if (strlen(tmpbuf) > 0)
            {
                for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
                {
                    pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                    NdisMoveMemory(pSecConfig->comm_wapi_info.ca_cert_path, macptr, strlen(macptr));
                    pSecConfig->comm_wapi_info.ca_cert_path_len = strlen(macptr);
			
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiCaCertPath=%s, len=%d",
                                        INF_MBSSID_DEV_NAME, 
                                        apidx, pSecConfig->comm_wapi_info.ca_cert_path, 
                                        pSecConfig->comm_wapi_info.ca_cert_path_len));
                }
            }
        }

        /* WapiUserCertPath */
        if (RTMPGetKeyParameter("WapiUserCertPath", tmpbuf, 128, buffer, TRUE))
        {
            if (strlen(tmpbuf) > 0)
            {
                for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
                {
                    pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                    NdisMoveMemory(pSecConfig->comm_wapi_info.user_cert_path, macptr, strlen(macptr));
                    pSecConfig->comm_wapi_info.user_cert_path_len = strlen(macptr);
			
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiUserCertPath=%s, len=%d",
                                        INF_MBSSID_DEV_NAME, 
                                        apidx, pSecConfig->comm_wapi_info.user_cert_path, 
                                        pSecConfig->comm_wapi_info.user_cert_path_len));
                }
            }
        }

        /* WapiAsIpAddr */
        if (RTMPGetKeyParameter("WapiAsIpAddr", tmpbuf, 32, buffer, TRUE))
        {        
            UINT32 ip_addr;

            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                if (rtinet_aton(macptr, &ip_addr))
                {
                    pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                    pSecConfig->comm_wapi_info.wapi_as_ip = ip_addr;  
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiAsIpAddr=%s(%x)",
                                        INF_MBSSID_DEV_NAME, apidx,
                                        macptr,  pSecConfig->comm_wapi_info.wapi_as_ip));
                }
            }
        }

        /* WapiAsPort */
        if (RTMPGetKeyParameter("WapiAsPort", tmpbuf, 32, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                pSecConfig->comm_wapi_info.wapi_as_port = simple_strtol(macptr, 0, 10);
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiAsPort=%d",
                                INF_MBSSID_DEV_NAME, apidx, pSecConfig->comm_wapi_info.wapi_as_port));
            }
        }

        /* WapiUskRekeyMethod */
        if (RTMPGetKeyParameter("WapiUskRekeyMethod", tmpbuf, 32, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                if ((strcmp(macptr, "TIME") == 0) || (strcmp(macptr, "time") == 0))
                    pSecConfig->wapi_usk_rekey_method = REKEY_METHOD_TIME;
                else if ((strcmp(macptr, "PKT") == 0) || (strcmp(macptr, "pkt") == 0))
                    pSecConfig->wapi_usk_rekey_method = REKEY_METHOD_PKT;
                else
                    pSecConfig->wapi_usk_rekey_method = REKEY_METHOD_DISABLE;

                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiUskRekeyMethod=%d",
                                INF_MBSSID_DEV_NAME, apidx, pSecConfig->wapi_usk_rekey_method));
            }
        }

        /* WapiUskRekeyThreshold */
        if (RTMPGetKeyParameter("WapiUskRekeyThreshold", tmpbuf, 32, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                if (simple_strtol(macptr, 0, 10) == 0)
                {
                    pSecConfig->wapi_usk_rekey_method = REKEY_METHOD_DISABLE;
                    pSecConfig->wapi_usk_rekey_threshold = 0;
                }
                else
                    pSecConfig->wapi_usk_rekey_threshold = simple_strtol(macptr, 0, 10);

                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiUskRekeyThreshold=%d",
                                INF_MBSSID_DEV_NAME, apidx, pSecConfig->wapi_usk_rekey_threshold));
            }
        }

        /* WapiMskRekeyMethod */
        if (RTMPGetKeyParameter("WapiMskRekeyMethod", tmpbuf, 32, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
			pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
			if ((strcmp(macptr, "TIME") == 0) || (strcmp(macptr, "time") == 0))
				pSecConfig->wapi_msk_rekey_method = REKEY_METHOD_TIME;
			else if ((strcmp(macptr, "PKT") == 0) || (strcmp(macptr, "pkt") == 0))
				pSecConfig->wapi_msk_rekey_method = REKEY_METHOD_PKT;
			else
				pSecConfig->wapi_msk_rekey_method = REKEY_METHOD_DISABLE;
	
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiMskRekeyMethod=%d",
							INF_MBSSID_DEV_NAME, apidx, pSecConfig->wapi_msk_rekey_method));
            }
        }
	
        /* WapiMskRekeyThreshold */
        if (RTMPGetKeyParameter("WapiMskRekeyThreshold", tmpbuf, 32, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                if (simple_strtol(macptr, 0, 10) == 0)
                {
                    pSecConfig->wapi_msk_rekey_method = REKEY_METHOD_DISABLE;
                    pSecConfig->wapi_msk_rekey_threshold = 0;
                }
                else
                    pSecConfig->wapi_msk_rekey_threshold = simple_strtol(macptr, 0, 10);

                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiMskRekeyThreshold=%d",
					INF_MBSSID_DEV_NAME, apidx, pSecConfig->wapi_msk_rekey_threshold));
            }
        }

        /* WapiPskX */
        for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
        {
            snprintf(tok_str, sizeof(tok_str), "WapiPsk%d", apidx + 1);

            pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;

            os_zero_mem(pSecConfig->WAPIPassPhrase, 64);
            pSecConfig->WAPIPassPhraseLen = 0;
            if(RTMPGetKeyParameter(tok_str, tmpbuf, 65, buffer, FALSE))
            {
                if (strlen(tmpbuf) >= 8 && strlen(tmpbuf) <= 64)
                {
                    os_move_mem(pSecConfig->WAPIPassPhrase, tmpbuf, strlen(tmpbuf));
                    pSecConfig->WAPIPassPhraseLen = strlen(tmpbuf);
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> WapiPsk=(%s), len=%d",
                                        INF_MBSSID_DEV_NAME, apidx, tmpbuf, strlen(tmpbuf)));
                }
                else
                {
                    if (IS_AKM_WPIPSK(pSecConfig->AKMMap))
                    {
                        CLEAR_SEC_AKM(pSecConfig->AKMMap);
                        CLEAR_PAIRWISE_CIPHER(pSecConfig);
                        CLEAR_GROUP_CIPHER(pSecConfig);

			SET_AKM_OPEN(pSecConfig->AKMMap);
			SET_CIPHER_NONE(pSecConfig->PairwiseCipher);
			SET_CIPHER_NONE(pSecConfig->GroupCipher);
                    }

                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> The length of WAPI PSKPassPhrase is invalid(len=%d).",
                                        INF_MBSSID_DEV_NAME, apidx, strlen(tmpbuf)));
                }																			
            }
        }					


        /* WapiPskType */
        if (RTMPGetKeyParameter("WapiPskType", tmpbuf, 32, buffer, TRUE))
        {
            for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
            {
                pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
                /* HEX */
                if(simple_strtol(macptr, 0, 10) == 0)
                {
                    pSecConfig->WapiPskType = HEX_MODE;
                    if (pSecConfig->WAPIPassPhraseLen % 2 != 0)
                    {
                        CLEAR_SEC_AKM(pSecConfig->AKMMap);
                        CLEAR_PAIRWISE_CIPHER(pSecConfig);
                        CLEAR_GROUP_CIPHER(pSecConfig);
                        SET_AKM_OPEN(pSecConfig->AKMMap);
                        SET_CIPHER_NONE(pSecConfig->PairwiseCipher);
                        SET_CIPHER_NONE(pSecConfig->GroupCipher);

                         MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) The WAPI-PSK key length MUST be even in Hex mode\n",
                                        INF_MBSSID_DEV_NAME, apidx));
                    }
                }
                else /* ASCII */	
                {
                    pSecConfig->WapiPskType = ASCII_MODE;
                }
            }
        }
    }
#endif /* CONFIG_AP_SUPPORT */

}

/* 
    ==========================================================================
    Description:
        It only shall be queried by wapi daemon for querying the related 
        configuration. This routine process the WAPI configuration for per BSS.
        
	==========================================================================
*/
VOID RTMPQueryWapiConfPerBss(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PWAPI_CONF		wapi_conf_ptr,
	IN	UCHAR			apidx)
{
	PMBSS_WAPI_INFO pConf = &wapi_conf_ptr->mbss_wapi_info[apidx];		
 
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	
		if (pMbss->wdev.if_dev != NULL)
		{
			PNET_DEV dev = pMbss->wdev.if_dev;
			
			NdisMoveMemory(pConf->ifname, RtmpOsGetNetDevName(dev), strlen(RtmpOsGetNetDevName(dev)));
			pConf->ifname_len = strlen(RtmpOsGetNetDevName(dev));			
		}
		else
		{
			RTMP_STRING slot_name[IFNAMSIZ];

			snprintf(slot_name, sizeof(slot_name), "ra%d", apidx);
			NdisMoveMemory(pConf->ifname, slot_name, strlen(slot_name));
			pConf->ifname_len = strlen(slot_name);					
		}

		/* Decide the authentication mode */
		if (IS_AKM_WAICERT(pMbss->wdev.SecConfig.AKMMap))
			pConf->auth_mode = WAPI_AUTH_CERT;
		else if (IS_AKM_WPIPSK(pMbss->wdev.SecConfig.AKMMap))
			pConf->auth_mode = WAPI_AUTH_PSK;
		else
			pConf->auth_mode = WAPI_AUTH_DISABLE;

		/* Fill in WAI pre-shared key */
		if (pMbss->WAPIPassPhraseLen > 0)
		{
			if (pMbss->WapiPskType == HEX_MODE)
			{
				pConf->psk_len = pMbss->WAPIPassPhraseLen / 2;
				AtoH((RTMP_STRING *) pMbss->WAPIPassPhrase, (PUCHAR) pConf->psk, pConf->psk_len);
			}
			else
			{
				pConf->psk_len = pMbss->WAPIPassPhraseLen; 
				NdisMoveMemory(pConf->psk, pMbss->WAPIPassPhrase, pConf->psk_len);
			}
		}

		/* Fill in WIE */
		if (pMbss->RSNIE_Len[0] > 0)
		{
			pConf->wie_len = pMbss->RSNIE_Len[0] + 2; 

			pConf->wie[0] = IE_WAPI;
			pConf->wie[1] = pMbss->RSNIE_Len[0];
			NdisMoveMemory(&pConf->wie[2], pMbss->RSN_IE[0], pMbss->RSNIE_Len[0]);
		}
	}
#endif /* CONFIG_AP_SUPPORT */


}


/* 
    ==========================================================================
    Description:
        It only shall be queried by wapi daemon for querying the related 
        configuration.        
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlQueryWapiConf(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{

	UCHAR		apidx;	
	UCHAR		*buf = NULL;
	PWAPI_CONF	pConf;
	
	MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_TRACE, ("RTMPIoctlQueryWapiConf==>\n"));

	/* Allocate memory for WAPI configuration */
	os_alloc_mem(NULL, (PUCHAR *)&buf, sizeof(WAPI_CONF));

	if (buf == NULL)
	{
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_ERROR, ("%s: allocate memory fail\n", __FUNCTION__));
		return;
	}

	pConf = (PWAPI_CONF)buf;
	
	NdisZeroMemory((PUCHAR)pConf, sizeof(WAPI_CONF));
	
	/* get MBSS number */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		pConf->mbss_num = pAd->ApCfg.BssidNum;		
	}
#endif /* CONFIG_AP_SUPPORT */
	
	/* Set common configuration */
	NdisMoveMemory(&pConf->comm_wapi_info, &pAd->ApCfg.MBSSID[0].wdev.SecConfig.comm_wapi_info, sizeof(COMMON_WAPI_INFO));

	for (apidx = 0; apidx < pConf->mbss_num; apidx++)
	{
		RTMPQueryWapiConfPerBss(pAd, pConf, apidx);				
	}
				
	wrq->u.data.length = sizeof(WAPI_CONF);
	if (copy_to_user(wrq->u.data.pointer, pConf, wrq->u.data.length))
	{
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	os_free_mem(buf);
	}
#endif /* CONFIG_AP_SUPPORT */
}

/*
    ==========================================================================
    Description:
        Timer execution function for periodically updating WAPI key.
    Return:
    ==========================================================================
*/  
VOID RTMPWapiUskRekeyPeriodicExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
	UINT32          	tmp_cnt = 0;    
	PMAC_TABLE_ENTRY 	pEntry = (PMAC_TABLE_ENTRY)FunctionContext;
	PRTMP_ADAPTER 		pAd = (PRTMP_ADAPTER)pEntry->pAd;
	struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
	
	
    if (pSecConfig->wapi_usk_rekey_method == REKEY_METHOD_TIME)
    {
		tmp_cnt = (++pSecConfig->wapi_usk_rekey_cnt);
    }
	else if (pSecConfig->wapi_usk_rekey_method == REKEY_METHOD_PKT)
	{
		/* the unit is 1K packets */
		tmp_cnt = pSecConfig->wapi_usk_rekey_cnt/1000;
	}
	else
		return;

	/* Trigger rekey procedure */
	if (tmp_cnt > pSecConfig->wapi_usk_rekey_threshold)
	{		
		pSecConfig->wapi_usk_rekey_cnt = 0;
		WAPI_InternalCmdAction(pAd, 
			   				   pEntry->SecConfig.AKMMap,
			   				   pEntry->func_tb_idx,
			   				   pEntry->Addr,
			   				   WAI_MLME_UPDATE_USK);
	}
}


/*
    ==========================================================================
    Description:
        Timer execution function for periodically updating WAPI key.
    Return:
    ==========================================================================
*/  
VOID RTMPWapiMskRekeyPeriodicExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
#ifdef CONFIG_AP_SUPPORT
	UINT            apidx, i;
	UINT32          tmp_cnt = 0;    
	PRTMP_ADAPTER   pAd = (PRTMP_ADAPTER)FunctionContext;
	PRALINK_TIMER_STRUCT pTimer = (PRALINK_TIMER_STRUCT) SystemSpecific3;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;


	/* if no any WAPI STA associated, don't do anything. */
	if (pAd->MacTab.fAnyWapiStation == FALSE)
		return;

	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		if (&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.GroupRekeyTimer == pTimer)
			break;
	}
		
	if (apidx == pAd->ApCfg.BssidNum)
		return;
		
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	/* increase counter for TIME method */
	if (pSecConfig->wapi_msk_rekey_method == REKEY_METHOD_TIME)
	{
		tmp_cnt = (++pSecConfig->wapi_msk_rekey_cnt);
	}
	else if (pSecConfig->wapi_msk_rekey_method == REKEY_METHOD_PKT)
	{
		/* the unit is 1K packets */
		tmp_cnt = pSecConfig->wapi_msk_rekey_cnt/1000;
	}
	else
		return; 

	if (tmp_cnt > pSecConfig->wapi_msk_rekey_threshold)
	{
		INT cnt, m_wcid; 

		pSecConfig->wapi_msk_rekey_cnt = 0;

		pSecConfig->GroupKeyId = pSecConfig->GroupKeyId == 0 ? 1 : 0;
		inc_iv_byte(pSecConfig->key_announce_flag, LEN_WAPI_TSC, 1);

		/* Generate NMK randomly */
		for (cnt = 0; cnt < 16; cnt++)
			pSecConfig->NMK[cnt] = RandomByte(pAd);
			
		RTMPDeriveWapiGTK(pSecConfig->NMK, pSecConfig->GTK);				

		GET_GroupKey_WCID(wdev, m_wcid);
		/* Install Shared key */
		WAPIInstallSharedKey(pAd, 
						pSecConfig->GroupCipher, 
						apidx, 
						pSecConfig->GroupKeyId, 
						m_wcid,
						pSecConfig->GTK);
	}					

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		MAC_TABLE_ENTRY  *pEntry;
		STA_TR_ENTRY *tr_entry = NULL;

		pEntry = &pAd->MacTab.Content[i];
		tr_entry = &pAd->MacTab.tr_entry[i];
		if (IS_ENTRY_CLIENT(pEntry) && 
			(IS_CIPHER_WPI_SMS4(pEntry->SecConfig.GroupCipher)) &&
			(pEntry->func_tb_idx == apidx) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
		{
			WAPI_InternalCmdAction(pAd, 
								   pEntry->SecConfig.AKMMap,
								   pEntry->func_tb_idx,
								   pEntry->Addr,
								   WAI_MLME_UPDATE_MSK);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
}

VOID RTMPInitWapiRekeyTimerByWdev(
	IN PRTMP_ADAPTER 	pAd,
	IN struct wifi_dev	*wdev)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
	        struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
	        if (IS_CIPHER_WPI_SMS4(pSecConfig->GroupCipher))
	        {
	            RTMPInitTimer(pAd, &pSecConfig->WapiMskRekeyTimer, GET_TIMER_FUNCTION(RTMPWapiMskRekeyPeriodicExec), pAd, TRUE);
	            pSecConfig->WapiMskRekeyTimerRunning = FALSE;
	        }
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID RTMPInitWapiRekeyTimerByMacEntry(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry) 
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_TRACE, 
			(" %s : WAPI USK rekey timer (wcid-%d) \n",__FUNCTION__, pEntry->wcid));
		RTMPInitTimer(pAd, &pEntry->SecConfig.WapiUskRekeyTimer, GET_TIMER_FUNCTION(RTMPWapiUskRekeyPeriodicExec), pEntry, TRUE);
		pEntry->SecConfig.WapiUskRekeyTimerRunning = FALSE;
	}
#endif
}

VOID RTMPStartWapiRekeyTimerByWdev(
	IN PRTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		if (IS_CIPHER_WPI_SMS4(pSecConfig->GroupCipher))
		{
			/* Group rekey related */
			if ((pSecConfig->wapi_msk_rekey_method != REKEY_METHOD_DISABLE) 
				&& (pSecConfig->wapi_msk_rekey_threshold > 0) 
					&& (pSecConfig->WapiMskRekeyTimerRunning == FALSE)) 
			{
				RTMPModTimer(&pSecConfig->WapiMskRekeyTimer, WAPI_KEY_UPDATE_EXEC_INTV);
				
				pSecConfig->WapiMskRekeyTimerRunning = TRUE;
				pSecConfig->wapi_msk_rekey_cnt = 0;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_TRACE, (" %s : WAPI MSK rekey timer is started \n", __FUNCTION__));
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

}

VOID RTMPStartWapiRekeyTimerByMacEntry( 
	IN PRTMP_ADAPTER *pAd, 
	IN PMAC_TABLE_ENTRY pEntry)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((pEntry->SecConfig.wapi_usk_rekey_method != REKEY_METHOD_DISABLE) 
			&& (pEntry->SecConfig.wapi_usk_rekey_threshold > 0))
		{
		/* Regularly check the timer */
			if (pEntry->SecConfig.WapiUskRekeyTimerRunning == FALSE)
			{
				RTMPSetTimer(&pEntry->SecConfig.WapiUskRekeyTimer, WAPI_KEY_UPDATE_EXEC_INTV);

				pEntry->SecConfig.WapiUskRekeyTimerRunning = TRUE;
				pEntry->SecConfig.wapi_usk_rekey_cnt = 0;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_TRACE, 
					(" %s : WAPI USK rekey timer is started (%d) \n",__FUNCTION__ , pEntry->SecConfig.wapi_usk_rekey_threshold));
			}							
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

VOID  RTMPCancelWapiRekeyTimerByWdev(
	IN PRTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		if (pSecConfig->WapiMskRekeyTimerRunning == TRUE)
		{
			BOOLEAN Cancelled;

			RTMPCancelTimer(&pSecConfig->WapiMskRekeyTimer, &Cancelled);
			pSecConfig->wapi_msk_rekey_cnt = 0;
			pSecConfig->WapiMskRekeyTimerRunning = FALSE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

}

VOID RTMPCancelWapiRekeyTimerByMacEntry( 
	IN PRTMP_ADAPTER *pAd, 
	IN PMAC_TABLE_ENTRY pEntry)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pEntry->SecConfig.WapiUskRekeyTimerRunning == TRUE)
		{
			BOOLEAN	Cancelled;
			RTMPCancelTimer(&pEntry->SecConfig.WapiUskRekeyTimer, &Cancelled);
			pEntry->SecConfig.wapi_usk_rekey_cnt = 0;
			pEntry->SecConfig.WapiUskRekeyTimerRunning = FALSE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}


/*
	========================================================================
	
	Routine Description:
		Prepare a L2 frame to wapi daemon to trigger WAPI state machine

	Arguments:		
		pAd			-	pointer to our pAdapter context	
  				
	Return Value:
		
	Note:
		
	========================================================================
*/
BOOLEAN WAPI_InternalCmdAction(
		IN PRTMP_ADAPTER		pAd,
		IN UINT32				AKMMap,
		IN UCHAR				apidx,
		IN PUCHAR				pAddr,
		IN UCHAR				flag)
{
    if (IS_AKM_WAICERT(AKMMap) || 
		IS_AKM_WPIPSK(AKMMap))
	{				
		UCHAR			WAPI_IE[] = {0x88, 0xb4};
		UINT8			frame_len = LENGTH_802_3 + 12; /* 12 indicates the WAPI internal command length */
		UCHAR			FrameBuf[frame_len];
		UINT8			offset = 0;
		
		/* Init the frame buffer */
		NdisZeroMemory(FrameBuf, frame_len);
		
		/* Prepare the 802.3 header */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pAddr, WAPI_IE); 
		}
#endif /* CONFIG_AP_SUPPORT */
		offset += LENGTH_802_3;

		/* Prepare the specific WAPI header */
		NdisMoveMemory(&FrameBuf[offset], RALINK_OUI, 3);
		offset += 3;

		/* Set the state of this command */
		FrameBuf[offset] = flag;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_TRACE, ("Trigger WAPI for this sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pAddr)));

		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
			return FALSE;	

	}	

	return TRUE;
}	


VOID RTMPGetWapiTxTscFromAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN	UINT			Wcid,
	OUT	UCHAR			*tx_tsc)
{
	USHORT			offset;	
	int				i;

	if (IS_HW_WAPI_SUPPORT(pAd))
	{
		UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;
		UINT32 wapi_pn_base = 0, wapi_pn_size = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
			wapi_pn_base = RLT_WAPI_PN_TABLE_BASE;
			wapi_pn_size = RLT_WAPI_PN_ENTRY_SIZE;
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
			wapi_pn_base = WAPI_PN_TABLE_BASE;
			wapi_pn_size = WAPI_PN_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

		NdisZeroMemory(tx_tsc, LEN_WAPI_TSC);

		/* Read IVEIV from Asic */
		offset = iveiv_tb_base + (Wcid * iveiv_tb_size);				
		for (i=0 ; i < iveiv_tb_size; i++)
			RTMP_IO_READ8(pAd, offset+i, &tx_tsc[i]); 

		/* Read WAPI PM from Asic */
		offset = wapi_pn_base + (Wcid * wapi_pn_size);
		for (i=0 ; i < wapi_pn_size; i++)
			RTMP_IO_READ8(pAd, offset+i, &tx_tsc[iveiv_tb_size + i]); 

		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_TRACE, ("%s : WCID(%d) ", __FUNCTION__, Wcid));			
		hex_dump("TxTsc", tx_tsc, LEN_WAPI_TSC);
	}
	else
	{
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_WARN, ("%s : Not support HW_WAPI_PN_TABLE\n", 
									__FUNCTION__));
	}	
	
}


VOID WAPIInstallPairwiseKey(
	PRTMP_ADAPTER		pAd,
	PMAC_TABLE_ENTRY	pEntry,
	BOOLEAN				bAE)
{
	ASIC_SEC_INFO Info = {0};
	struct _SEC_KEY_INFO *pKey = &Info.Key;

	if (!IS_CIPHER_WPI_SMS4(pEntry->SecConfig.PairwiseCipher))
	{
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_ERROR, ("%s : fails (wcid-%d)\n", 
					__FUNCTION__, pEntry->wcid));	
		return;
	}	

	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = pEntry->func_tb_idx;
	Info.Cipher = pEntry->SecConfig.PairwiseCipher;
	Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);

	/* Prepare pair-wise key material */
	pKey->KeyLen = LEN_TK;
	NdisMoveMemory(pKey->Key, &pEntry->SecConfig.PTK[0], 16);
	NdisMoveMemory(pKey->TxMic, &pEntry->SecConfig.PTK[16], 8);
	NdisMoveMemory(pKey->RxMic, &pEntry->SecConfig.PTK[24], 8);			

	/* Initial TSC for unicast */
	if (bAE)
		NdisMoveMemory(pKey->TxTsc, AE_UCAST_PN, LEN_WAPI_TSC);
	else
		NdisMoveMemory(pKey->TxTsc, ASUE_UCAST_PN, LEN_WAPI_TSC);

	NdisZeroMemory(pKey->RxTsc, LEN_WAPI_TSC);

	HW_ADDREMOVE_KEYTABLE(pAd, &Info);	
}


VOID WAPIInstallSharedKey(
	PRTMP_ADAPTER		pAd,
	UINT32				GroupCipher,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				Wcid,
	PUINT8				pGtk)
{
	ASIC_SEC_INFO Info = {0};
	struct _SEC_KEY_INFO *pKey = &Info.Key;

	if (BssIdx >= MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM)
	{
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_ERROR, ("%s : The BSS-index(%d) is out of range for MBSSID link. \n", 
									__FUNCTION__, BssIdx));	
		return;
	}

	if (!IS_CIPHER_WPI_SMS4(GroupCipher))
	{
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WAPI, DBG_LVL_ERROR, ("%s : fails (IF/ra%d) \n", 
										__FUNCTION__, BssIdx));	
		return;
	}

	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = Wcid;
	Info.BssIndex = BssIdx;
	Info.Cipher = GroupCipher;
	Info.KeyIdx = KeyIdx;
	os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);

	/* Assign key material into SW key table */
	pKey->KeyLen = LEN_TK;
	NdisMoveMemory(pKey->Key, pGtk, LEN_TK);
	NdisMoveMemory(pKey->TxMic, pGtk + 16, LEN_TKIP_MIC);
	NdisMoveMemory(pKey->RxMic, pGtk + 24, LEN_TKIP_MIC);            

	/* Initial TSC for B/Mcast */
	NdisMoveMemory(pKey->TxTsc, AE_BCAST_PN, LEN_WAPI_TSC);
	NdisZeroMemory(pKey->RxTsc, LEN_WAPI_TSC);		

	HW_ADDREMOVE_KEYTABLE(pAd, &Info);	
}

#endif /* WAPI_SUPPORT */

