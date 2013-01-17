/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#include "rt_config.h"

UCHAR	CipherSuiteTDLSWpa2PskTkip[] = {
		0x30,					// RSN IE
		0x14,					// Length	
		0x01, 0x00,				// Version
		0x00, 0x00, 0x00, 0x00,	// no group cipher
		0x01, 0x00,				// number of pairwise
		0x00, 0x0f, 0xac, 0x02,	// unicast, TKIP
		0x01, 0x00,				// number of authentication method
		0x00, 0x0f, 0xac, TDLS_AKM_SUITE_PSK,	// TDLS authentication
		0x02, 0x00,				// RSN capability, peer key enabled
		};

UCHAR	CipherSuiteTDLSWpa2PskAes[] = {
		0x30,					// RSN IE
		0x14,					// Length	
		0x01, 0x00,				// Version
		0x00, 0x00, 0x00, 0x00,	// no group cipher
		0x01, 0x00,				// number of pairwise
		0x00, 0x0f, 0xac, 0x04,	// unicast, AES
		0x01, 0x00,				// number of authentication method
		0x00, 0x0f, 0xac, TDLS_AKM_SUITE_PSK,	// TDLS authentication
		0x02, 0x00,				// RSN capability, peer key enabled
		};
UCHAR	CipherSuiteTDLSLen = sizeof(CipherSuiteTDLSWpa2PskAes)/ sizeof(UCHAR);

UCHAR	TdlsZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildSetupRequest(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS)
{
	ULONG			Timeout = TDLS_TIMEOUT;
	BOOLEAN			TimerCancelled;
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen, CATEGORY_TDLS, TDLS_ACTION_CODE_SETUP_REQUEST);

	/* fill Dialog Token */
	pAd->StaCfg.TdlsDialogToken++;
	if (pAd->StaCfg.TdlsDialogToken == 0)
		pAd->StaCfg.TdlsDialogToken++;

	pTDLS->Token = pAd->StaCfg.TdlsDialogToken;
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->Token);

	/* fill link identifier */
	TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	// fill capability
	TDLS_InsertCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill ssid
	TDLS_InsertSSIDIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill support rate
	TDLS_InsertSupportRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill ext rate
	TDLS_InsertExtRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill Qos Capability
	TDLS_InsertQosCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

#ifdef DOT11_N_SUPPORT
	// fill HT Capability
	TDLS_InsertHtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// fill 20/40 BSS Coexistence (7.3.2.61)
#ifdef DOT11N_DRAFT3
	TDLS_InsertBSSCoexistenceIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

	// fill  Extended Capabilities (7.3.2.27)
	TDLS_InsertExtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

	// TPK Handshake if RSNA Enabled
	// Pack TPK Message 1 here! 
	if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
		((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
	{		
		UCHAR			CipherTmp[64] = {0};
		UCHAR			CipherTmpLen = 0;
		FT_FTIE			FtIe;
		ULONG			KeyLifetime = TDLS_KEY_TIMEOUT;	// sec
		ULONG			tmp;
		UCHAR			Length;
		
		// RSNIE (7.3.2.25)
		CipherTmpLen = CipherSuiteTDLSLen;	
		if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
			NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
		else
			NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);
		
		// update AKM
		if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
			CipherTmp[19] = TDLS_AKM_SUITE_1X;
		
		// Insert RSN_IE to outgoing frame
		MakeOutgoingFrame((pFrameBuf + *pFrameLen),	&tmp,
				CipherTmpLen,						&CipherTmp,
				END_OF_ARGS);

		*pFrameLen = *pFrameLen + tmp;
	
			
		// FTIE (7.3.2.48)
		NdisZeroMemory(&FtIe, sizeof(FtIe));
		Length =  sizeof(FtIe);

		// generate SNonce
		GenRandom(pAd, pAd->CurrentAddress, FtIe.SNonce);
		hex_dump("TDLS - Generate SNonce ", FtIe.SNonce, 32);
		NdisMoveMemory(pTDLS->SNonce, FtIe.SNonce, 32);
		
		TDLS_InsertFTIE(
				pAd, 
				(pFrameBuf + *pFrameLen), 
				pFrameLen, 
				Length, 
				FtIe.MICCtr,
				FtIe.MIC, 
				FtIe.ANonce, 
				FtIe.SNonce);

		
		// Timeout Interval (7.3.2.49)
		TDLS_InsertTimeoutIntervalIE(
				pAd, 
				(pFrameBuf + *pFrameLen), 
				pFrameLen, 
				2, /* key lifetime interval */
				KeyLifetime);

		pTDLS->KeyLifetime = KeyLifetime;

	}

	// ==>> Set sendout timer
	RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
	RTMPSetTimer(&pTDLS->Timer, Timeout);
	// ==>> State Change

	pTDLS->Status = TDLS_MODE_WAIT_RESPONSE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildSetupResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen, CATEGORY_TDLS, TDLS_ACTION_CODE_SETUP_RESPONSE);

	/* fill status code */
	TDLS_InsertStatusCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, StatusCode);

	/* fill Dialog Token */
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->Token);

	if (StatusCode == MLME_SUCCESS)
	{
		/* fill link identifier */
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->MacAddr, pAd->CurrentAddress);

		// fill capability
		TDLS_InsertCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill ssid
		TDLS_InsertSSIDIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill support rate
		TDLS_InsertSupportRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill ext rate
		TDLS_InsertExtRateIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

		// fill Qos Capability
		TDLS_InsertQosCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);

#ifdef DOT11_N_SUPPORT
		// fill HT Capability
		TDLS_InsertHtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
#endif // DOT11_N_SUPPORT //

		// TPK Handshake if RSNA Enabled
		// Pack TPK Message 2 here! 
		if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
			((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
		{		
			FT_FTIE	*ft;
			ULONG	tmp;
			UINT	key_len = LEN_PMK;

			// RSNIE (7.3.2.25)			
			// Insert RSN_IE of the Peer TDLS to outgoing frame
			MakeOutgoingFrame((pFrameBuf + *pFrameLen),	&tmp,
					RsnLen,								pRsnIe,
					END_OF_ARGS);
			*pFrameLen = *pFrameLen + tmp;

			// FTIE (7.3.2.48)
			// Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
			
			// point to the element of IE
			ft = (FT_FTIE *)(pFTIe + 2);	
			// generate ANonce
			GenRandom(pAd, pAd->CurrentAddress, ft->ANonce);
			hex_dump("TDLS - Generate ANonce ", ft->ANonce, 32);
			// set MIC field to zero before MIC calculation
			NdisZeroMemory(ft->MIC, 16);
			// copy SNonce from peer TDLS
			NdisMoveMemory(ft->SNonce, pTDLS->SNonce, 32);
			// copy ANonce to TDLS entry
			NdisMoveMemory(pTDLS->ANonce, ft->ANonce, 32);

			if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
				key_len = 48;
			// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// Derive TPK-KCK for MIC key, TPK-TK for direct link data
			TDLS_FTDeriveTPK(
					pTDLS->MacAddr,	/* MAC Address of Initiator */
					pAd->CurrentAddress, /* I am Responder */
					pTDLS->ANonce, 
					pTDLS->SNonce, 
					pAd->CommonCfg.Bssid,
					key_len,
					pTDLS->TPK,
					pTDLS->TPKName);

			// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

			////////////////////////////////////////////////////////////////////////
			// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
			// 1. MAC_I (6 bytes)
			// 2. MAC_R (6 bytes)
			// 3. Transaction Sequence = 2 (1 byte)
			// 4. Link Identifier (20 bytes)
			// 5. RSN IE without the IE header (20 bytes)
			// 6. Timeout Interval IE (7 bytes)
			// 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes)	
			{
				UCHAR	content[512];
				ULONG	c_len = 0;
				ULONG	tmp_len = 0;
				UCHAR	Seq = 2;
				UCHAR	mic[16];
				UCHAR	LinkIdentifier[20];
				UINT	tmp_aes_len = 0;

				NdisZeroMemory(LinkIdentifier, 20);
				LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
				LinkIdentifier[1] = 18;
				NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
				NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
				NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);

				NdisZeroMemory(mic, sizeof(mic));

				/* make a header frame for calculating MIC. */
				MakeOutgoingFrame(content,					&tmp_len,
			                      MAC_ADDR_LEN,				pTDLS->MacAddr,
			                      MAC_ADDR_LEN,				pAd->CurrentAddress,
			                      1,						&Seq,
			                      END_OF_ARGS);
				c_len += tmp_len;					

				/* concatenate Link Identifier */			
			    MakeOutgoingFrame(content + c_len,		&tmp_len,
			                      20,					LinkIdentifier,
			                      END_OF_ARGS);
				c_len += tmp_len;					

				/* concatenate RSNIE */
			    MakeOutgoingFrame(content + c_len,		&tmp_len,
			                      20,					(pRsnIe + 2),
			                      END_OF_ARGS);
				c_len += tmp_len;					

				/* concatenate Timeout Interval IE */
				MakeOutgoingFrame(content + c_len,     &tmp_len,
								  7,					pTIIe,
								  END_OF_ARGS);
				c_len += tmp_len;

				/* concatenate FTIE */			
			    MakeOutgoingFrame(content + c_len,		&tmp_len,
			                      sizeof(FT_FTIE),	(PUCHAR)ft,
			                      END_OF_ARGS);
				c_len += tmp_len;

				/* Calculate MIC */				
				//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

				/* Compute AES-128-CMAC over the concatenation */
				tmp_aes_len = AES_KEY128_LENGTH;
    			AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);

				// Fill Mic to ft struct
				NdisMoveMemory(ft->MIC, mic, 16);

			}
			////////////////////////////////////////////////////////////////////////
			
			// Insert FT_IE to outgoing frame
			TDLS_InsertFTIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					sizeof(FT_FTIE), 
					ft->MICCtr, 
					ft->MIC, 
					ft->ANonce, 
					ft->SNonce);

			// Timeout Interval (7.3.2.49)
			// Insert TI_IE to outgoing frame
			TDLS_InsertTimeoutIntervalIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					2, /* key lifetime interval */
					pTDLS->KeyLifetime);
		}	
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildSetupConfirm(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen, CATEGORY_TDLS, TDLS_ACTION_CODE_SETUP_CONFIRM);

	/* fill status code */
	TDLS_InsertStatusCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, StatusCode);

	/* fill Dialog Token */
	TDLS_InsertDialogToken(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->Token);

	/* fill link identifier */
	TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	// fill Qos Capability
	TDLS_InsertEDCAParameterSetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS);

#ifdef DOT11_N_SUPPORT
	// fill HT Capability
	TDLS_InsertHtCapIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen);
#endif // DOT11_N_SUPPORT //

	// TPK Handshake if RSNA Enabled
	// Pack TPK Message 3 here! 
	if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
		((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
	{		
		FT_FTIE	*ft;
		ULONG	tmp;
			
		// RSNIE (7.3.2.25)			
		// Insert RSN_IE of the Peer TDLS to outgoing frame
		MakeOutgoingFrame((pFrameBuf + *pFrameLen),	&tmp,
							RsnLen,					pRsnIe,
							END_OF_ARGS);
		*pFrameLen = *pFrameLen + tmp;
		
				
		// FTIE (7.3.2.48)
		// Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
	
		// point to the element of IE
		ft = (FT_FTIE *)(pFTIe + 2);	
		// set MIC field to zero before MIC calculation
		NdisZeroMemory(ft->MIC, 16);

		////////////////////////////////////////////////////////////////////////
		// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
		// 1. MAC_I (6 bytes)
		// 2. MAC_R (6 bytes)
		// 3. Transaction Sequence = 2 (1 byte)
		// 4. Link Identifier (20 bytes)
		// 5. RSN IE without the IE header (20 bytes)
		// 6. Timeout Interval IE (7 bytes)
		// 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes)	
		{
			UCHAR	content[512];
			ULONG	c_len = 0;
			ULONG	tmp_len = 0;
			UCHAR	Seq = 3;
			UCHAR	mic[16];
			UCHAR	LinkIdentifier[20];
			UINT	tmp_aes_len = 0;

			NdisZeroMemory(LinkIdentifier, 20);
			LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
			LinkIdentifier[1] = 18;
			NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
			NdisMoveMemory(&LinkIdentifier[8], pAd->CurrentAddress, 6);
			NdisMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);

			NdisZeroMemory(mic, sizeof(mic));

			/* make a header frame for calculating MIC. */
			MakeOutgoingFrame(content,					&tmp_len,
								MAC_ADDR_LEN,			pAd->CurrentAddress,
								MAC_ADDR_LEN,			pTDLS->MacAddr,
								1,						&Seq,
								END_OF_ARGS);
			c_len += tmp_len;					

			/* concatenate Link Identifier */			
			MakeOutgoingFrame(content + c_len,		&tmp_len,
								20,					LinkIdentifier,
								END_OF_ARGS);
			c_len += tmp_len;					

				
			/* concatenate RSNIE */
			MakeOutgoingFrame(content + c_len,		&tmp_len,
								20,					(pRsnIe + 2),
								END_OF_ARGS);
			c_len += tmp_len;					

			/* concatenate Timeout Interval IE */
			MakeOutgoingFrame(content + c_len,     &tmp_len,
								7,					pTIIe,
								END_OF_ARGS);
			c_len += tmp_len;

			/* concatenate FTIE */			
			MakeOutgoingFrame(content + c_len,		&tmp_len,
								sizeof(FT_FTIE),	(PUCHAR)ft,
								END_OF_ARGS);
			c_len += tmp_len;	

			/* Calculate MIC */				
			//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

			/* Compute AES-128-CMAC over the concatenation */
			tmp_aes_len = AES_KEY128_LENGTH;
    		AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);

			// Fill Mic to ft struct
			NdisMoveMemory(ft->MIC, mic, 16);
		}
		////////////////////////////////////////////////////////////////////////
	
		// Insert FT_IE to outgoing frame
		TDLS_InsertFTIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					sizeof(FT_FTIE), 
					ft->MICCtr, 
					ft->MIC, 
					ft->ANonce, 
					ft->SNonce);

		// Timeout Interval (7.3.2.49)
		// Insert TI_IE to outgoing frame
		TDLS_InsertTimeoutIntervalIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					2, /* key lifetime interval */
					pTDLS->KeyLifetime);

	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildTeardown(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS,
	IN	UINT16	ReasonCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen, CATEGORY_TDLS, TDLS_ACTION_CODE_TEARDOWN);

	/* fill reason code */
	TDLS_InsertReasonCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ReasonCode);

	/* fill link identifier */
	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->MacAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	// FTIE includes if RSNA Enabled
	if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
		((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) || (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)))
	{	
		UCHAR		FTIe[128];	
		FT_FTIE		*ft = NULL;
		UCHAR		content[256];
		ULONG		c_len = 0;
		ULONG		tmp_len = 0;
		UCHAR		seq = 4;
		UCHAR		mic[16];
		UCHAR		LinkIdentifier[20];
		UINT		tmp_aes_len = 0;

		NdisZeroMemory(LinkIdentifier, 20);
		LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
		LinkIdentifier[1] = 18;

		NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
		if (pTDLS->bInitiator)
		{
			NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
			NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);
		}
		else
		{
			NdisMoveMemory(&LinkIdentifier[8], pAd->CurrentAddress, 6);
			NdisMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);
		}


		//NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
		//NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);

		// FTIE (7.3.2.48)
		// The contents of FTIE in the TDLS Teardown frame shall be the same as that included
		// in the TPK Handshake Message3 with the exception of the MIC field.

		// Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
			
		// point to the element of IE
		NdisZeroMemory(FTIe, sizeof(FTIe));
		FTIe[0] = IE_FT_FTIE;
		FTIe[1] = sizeof(FT_FTIE);
		ft = (PFT_FTIE)&FTIe[2];	
		NdisMoveMemory(ft->ANonce, pTDLS->ANonce, 32);
		NdisMoveMemory(ft->SNonce, pTDLS->SNonce, 32);

		////////////////////////////////////////////////////////////////////////
		// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
		// 1. Link Identifier (20 bytes)
		// 2. Reason Code (2 bytes)
		// 3. Dialog token (1 byte)
		// 4. Transaction Sequence = 4 (1 byte)
		// 5. FTIE with the MIC field of FTIE set to zero (84 bytes)	
		
		/* concatenate Link Identifier, Reason Code, Dialog token, Transaction Sequence */
		MakeOutgoingFrame(content,            		&tmp_len,
						sizeof(LinkIdentifier),		LinkIdentifier,	
						2,							&ReasonCode,
						1,							&pTDLS->Token,
						1,							&seq,
						END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate FTIE */
		MakeOutgoingFrame(content + c_len,		&tmp_len,
							FTIe[1] + 2,		FTIe,  
						END_OF_ARGS);
		c_len += tmp_len;					
		
		/* Calculate MIC */
		NdisZeroMemory(mic, sizeof(mic));
		//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

		/* Compute AES-128-CMAC over the concatenation */
		tmp_aes_len = AES_KEY128_LENGTH;
    	AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);
		
		/* Fill Mic to ft struct */
		NdisMoveMemory(ft->MIC, mic, 16);
		////////////////////////////////////////////////////////////////////////

		// Insert FT_IE to outgoing frame
		TDLS_InsertFTIE(
					pAd, 
					(pFrameBuf + *pFrameLen), 
					pFrameLen, 
					FTIe[1], 
					ft->MICCtr, 
					ft->MIC, 
					ft->ANonce, 
					ft->SNonce);
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
USHORT	TDLS_TPKMsg1Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen)
{
	USHORT			StatusCode = MLME_SUCCESS;
	UCHAR			CipherTmp[64] = {0};
	UCHAR			CipherTmpLen = 0;
	FT_FTIE			*ft = NULL;

	// Validate RsnIE
	//
	if (RsnLen == 0) // RSN not exist
		return  MLME_INVALID_INFORMATION_ELEMENT;

	if (pRsnIe[2] < 1) // Smaller version
		return	MLME_NOT_SUPPORT_RSN_VERSION;

	CipherTmpLen = CipherSuiteTDLSLen;
	if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
	else
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

	if(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
		CipherTmp[19] = TDLS_AKM_SUITE_1X;

	if ( RTMPEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
		return MLME_INVALID_AKMP;

	if ( RTMPEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
		return MLME_INVALID_RSN_CAPABILITIES;

	if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
		return REASON_UCIPHER_NOT_VALID;

	// Validate FTIE
	//
	ft = (PFT_FTIE)(pFTIe + 2); // point to the element of IE
	if ((FTLen != (sizeof(FT_FTIE) + 2)) || RTMPEqualMemory(&ft->MICCtr, TdlsZeroSsid, 2) == 0 || 
		(RTMPEqualMemory(ft->MIC, TdlsZeroSsid, 16) == 0) || (RTMPEqualMemory(ft->ANonce, TdlsZeroSsid, 32) == 0))
		return REASON_FT_INVALID_FTIE;

	// Validate TIIE
	//
	if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < TDLS_KEY_TIMEOUT))
		return TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME;

	return StatusCode;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
USHORT	TDLS_TPKMsg2Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen,
	OUT	PUCHAR				pTPK,
	OUT PUCHAR				pTPKName)
{
	USHORT		StatusCode = MLME_SUCCESS;
	UCHAR		CipherTmp[64] = {0};
	UCHAR		CipherTmpLen = 0;
	FT_FTIE		*ft = NULL;			
	UCHAR		oldMic[16];
	UCHAR		LinkIdentifier[20];
	UINT		key_len = LEN_PMK;
	
	// Validate RsnIE
	//
	if (RsnLen == 0) // RSN not exist
		return  MLME_INVALID_INFORMATION_ELEMENT;

	if (pRsnIe[2] < 1) // Smaller version
		return	MLME_NOT_SUPPORT_RSN_VERSION;

	CipherTmpLen = CipherSuiteTDLSLen;
	if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
	{
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
		key_len = 48;
	}
	else
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

	if(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
		CipherTmp[19] = TDLS_AKM_SUITE_1X;

	if ( RTMPEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
		return MLME_INVALID_AKMP;

	if ( RTMPEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
		return MLME_INVALID_RSN_CAPABILITIES;

	if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
		return REASON_UCIPHER_NOT_VALID;

	// Validate FTIE
	//
	ft = (PFT_FTIE)(pFTIe + 2); // point to the element of IE
	if ((FTLen != (sizeof(FT_FTIE) + 2)) || RTMPEqualMemory(&ft->MICCtr, TdlsZeroSsid, 2) == 0 || 
		(RTMPEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0))
		return REASON_FT_INVALID_FTIE;

	// Validate TIIE
	//
	if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < TDLS_KEY_TIMEOUT))
		return TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME;


	// Validate the MIC field of FTIE
	//
	
	// point to the element of IE
	ft = (PFT_FTIE)(pFTIe + 2);	
	// backup MIC fromm the peer TDLS
	NdisMoveMemory(oldMic, ft->MIC, 16);

	
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Derive TPK-KCK for MIC key, TPK-TK for direct link data
	TDLS_FTDeriveTPK(
			pAd->CurrentAddress, /* I am Initiator */
			pTDLS->MacAddr,	/* MAC Address of Responder */
			ft->ANonce, 
			ft->SNonce, 
			pAd->CommonCfg.Bssid,
			key_len,
			pTPK,
			pTPKName);
	
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	
	// set MIC field to zero before MIC calculation
	NdisZeroMemory(ft->MIC, 16);

	// Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
	NdisZeroMemory(LinkIdentifier, 20);
	LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
	LinkIdentifier[1] = 18;
	NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
	NdisMoveMemory(&LinkIdentifier[8], pAd->CurrentAddress, 6);
	NdisMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);

	////////////////////////////////////////////////////////////////////////
	// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
	// 1. MAC_I (6 bytes)
	// 2. MAC_R (6 bytes)
	// 3. Transaction Sequence = 2 (1 byte)
	// 4. Link Identifier (20 bytes)
	// 5. RSN IE without the IE header (20 bytes)
	// 6. Timeout Interval IE (7 bytes)
	// 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes)	
	{
		UCHAR	content[512];
		ULONG	c_len = 0;
		ULONG	tmp_len = 0;
		UCHAR	Seq = 2;
		UCHAR	mic[16];
		UINT	tmp_aes_len = 0;
		
		NdisZeroMemory(mic, sizeof(mic));
			
		/* make a header frame for calculating MIC. */
		MakeOutgoingFrame(content,					&tmp_len,
	                      MAC_ADDR_LEN,				pAd->CurrentAddress,
	                      MAC_ADDR_LEN,				pTDLS->MacAddr,
	                      1,						&Seq,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Link Identifier */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      20,					LinkIdentifier,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		
		/* concatenate RSNIE */
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      20,					pRsnIe + 2,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Timeout Interval IE */
		MakeOutgoingFrame(content + c_len,     &tmp_len,
						  7,					pTIIe,
						  END_OF_ARGS);
		c_len += tmp_len;
		
		
		/* concatenate FTIE */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      sizeof(FT_FTIE),	(PUCHAR)ft,
	                      END_OF_ARGS);
		c_len += tmp_len;
		

		/* Calculate MIC */				
		//AES_128_CMAC(pTPK, content, c_len, mic);

		/* Compute AES-128-CMAC over the concatenation */
		tmp_aes_len = AES_KEY128_LENGTH;
    	AES_CMAC(content, c_len, pTPK, 16, mic, &tmp_aes_len);

		NdisMoveMemory(ft->MIC, mic, 16);

	}
	////////////////////////////////////////////////////////////////////////
	
	if (RTMPEqualMemory(oldMic, ft->MIC, 16) == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS_TPKMsg2Process() MIC Error!!! \n"));
		return MLME_REQUEST_DECLINED;
	}

	
	return StatusCode;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
USHORT	TDLS_TPKMsg3Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen)
{
	USHORT			StatusCode = MLME_SUCCESS;
	UCHAR			CipherTmp[64] = {0};
	UCHAR			CipherTmpLen = 0;
	FT_FTIE			*ft = NULL;			
	UCHAR			oldMic[16];
	UCHAR			LinkIdentifier[20];
	
	// Validate RsnIE
	//
	if (RsnLen == 0) // RSN not exist
		return  MLME_INVALID_INFORMATION_ELEMENT;

	if (pRsnIe[2] < 1) // Smaller version
		return	MLME_NOT_SUPPORT_RSN_VERSION;

	CipherTmpLen = CipherSuiteTDLSLen;
	if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
	else
		NdisMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

	if(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
		CipherTmp[19] = TDLS_AKM_SUITE_1X;

	if ( RTMPEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
		return MLME_INVALID_AKMP;

	if ( RTMPEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
		return MLME_INVALID_RSN_CAPABILITIES;

	if ((RsnLen != 22) || (RTMPEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
		return REASON_UCIPHER_NOT_VALID;

	// Validate FTIE
	//
	ft = (PFT_FTIE)(pFTIe + 2); // point to the element of IE
	if ((FTLen != (sizeof(FT_FTIE) + 2)) || RTMPEqualMemory(&ft->MICCtr, TdlsZeroSsid, 2) == 0 || 
		(RTMPEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0) || (RTMPEqualMemory(ft->ANonce, pTDLS->ANonce, 32) == 0))
		return REASON_FT_INVALID_FTIE;

	// Validate TIIE
	//
	if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < pTDLS->KeyLifetime))
		return TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME;


	// Validate the MIC field of FTIE
	//
	
	// point to the element of IE
	ft = (PFT_FTIE)(pFTIe + 2);	
	// backup MIC fromm the peer TDLS
	NdisMoveMemory(oldMic, ft->MIC, 16);

		
	// set MIC field to zero before MIC calculation
	NdisZeroMemory(ft->MIC, 16);

	// Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
	NdisZeroMemory(LinkIdentifier, 20);
	LinkIdentifier[0] = IE_TDLS_LINK_IDENTIFIER;
	LinkIdentifier[1] = 18;
	NdisMoveMemory(&LinkIdentifier[2], pAd->CommonCfg.Bssid, 6);
	NdisMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
	NdisMoveMemory(&LinkIdentifier[14], pAd->CurrentAddress, 6);

	////////////////////////////////////////////////////////////////////////
	// The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
	// 1. MAC_I (6 bytes)
	// 2. MAC_R (6 bytes)
	// 3. Transaction Sequence = 3 (1 byte)
	// 4. Link Identifier (20 bytes)
	// 5. RSN IE without the IE header (20 bytes)
	// 6. Timeout Interval IE (7 bytes)
	// 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes)
	{
		UCHAR	content[512];
		ULONG	c_len = 0;
		ULONG	tmp_len = 0;
		UCHAR	Seq = 3;
		UCHAR	mic[16];
		UINT	tmp_aes_len = 0;
		
		NdisZeroMemory(mic, sizeof(mic));
			
		/* make a header frame for calculating MIC. */
		MakeOutgoingFrame(content,					&tmp_len,
	                      MAC_ADDR_LEN,				pTDLS->MacAddr,
	                      MAC_ADDR_LEN,				pAd->CurrentAddress,
	                      1,						&Seq,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Link Identifier */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      20,					LinkIdentifier,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		
		/* concatenate RSNIE */
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      20,					pRsnIe + 2,
	                      END_OF_ARGS);
		c_len += tmp_len;					

		/* concatenate Timeout Interval IE */
		MakeOutgoingFrame(content + c_len,     &tmp_len,
						  7,					pTIIe,
						  END_OF_ARGS);
		c_len += tmp_len;
		
		
		/* concatenate FTIE */			
	    MakeOutgoingFrame(content + c_len,		&tmp_len,
	                      sizeof(FT_FTIE),	(PUCHAR)ft,
	                      END_OF_ARGS);
		c_len += tmp_len;
		

		/* Calculate MIC */				
		//AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

		/* Compute AES-128-CMAC over the concatenation */
		tmp_aes_len = AES_KEY128_LENGTH;
    	AES_CMAC(content, c_len, pTDLS->TPK, 16, mic, &tmp_aes_len);

		NdisMoveMemory(ft->MIC, mic, 16);

	}
	////////////////////////////////////////////////////////////////////////
	
	if (RTMPEqualMemory(oldMic, ft->MIC, 16) == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("TDLS_TPKMsg3Process() MIC Error!!! \n"));
		return MLME_REQUEST_DECLINED;
	}

	
	return StatusCode;
}
#endif // DOT11Z_TDLS_SUPPORT //

