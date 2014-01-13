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
	cmm_wpa.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
    Eddy Tsai   09-10-13        Rewrite all functions
*/
#include "rt_config.h"

/* Local Function Declaration */
static VOID WpaEAPOLKeySend(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile);

static VOID WpaPeerPairMsg1Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator);

static VOID WpaPeerPairMsg2Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant);

static VOID WpaPeerPairMsg3Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator);

static VOID WpaPeerPairMsg4Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant);

static VOID WpaPeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant,
    IN MLME_QUEUE_ELEM  *Elem);

static VOID WpaPeerPairMsg2Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator,
    IN MLME_QUEUE_ELEM  *Elem);

static VOID WpaPeerPairMsg3Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant,    
    IN MLME_QUEUE_ELEM  *Elem) ;

static VOID WpaPeerPairMsg4Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator,    
    IN MLME_QUEUE_ELEM  *Elem);

static VOID Wpa4WayComplete(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry);


VOID WpaProfileInit(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry)
{
	DBGPRINT(RT_DEBUG_TRACE, ("===> WpaProfileInit \n"));

    switch (pEntry->WpaRole)
    {
		case WPA_Authenticator:
            if (pEntry->pWPA_Authenticator == NULL) {
                os_alloc_mem(pAd, (PUCHAR *) &pEntry->pWPA_Authenticator, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
                if (pEntry->pWPA_Authenticator == NULL)
                    goto MEMORY_FAILURE;
            } /* End of if */
            NdisZeroMemory(pEntry->pWPA_Authenticator, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
            pEntry->pWPA_Authenticator->Role = WPA_Authenticator;
   		    RTMPInitTimer(pAd, &pEntry->pWPA_Authenticator->MsgRetryTimer, GET_TIMER_FUNCTION(Adhoc_WpaRetryExec), pEntry, FALSE);
            WpaProfileReset(pAd, pEntry, pEntry->pWPA_Authenticator);
			break;    
		case WPA_Supplicant:
            if (pEntry->pWPA_Supplicant == NULL) {
                os_alloc_mem(pAd, (PUCHAR *) &pEntry->pWPA_Supplicant, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
                if (pEntry->pWPA_Supplicant == NULL)
                    goto MEMORY_FAILURE; 
            } /* End of if */
            NdisZeroMemory(pEntry->pWPA_Supplicant, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
            pEntry->pWPA_Supplicant->Role = WPA_Supplicant;
            WpaProfileReset(pAd, pEntry, pEntry->pWPA_Supplicant);
			break;
		case WPA_BOTH:
            // Authenticator
            if (pEntry->pWPA_Authenticator == NULL) {
                os_alloc_mem(pAd, (PUCHAR *) &pEntry->pWPA_Authenticator, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
                if (pEntry->pWPA_Authenticator == NULL)
                    goto MEMORY_FAILURE; 
            } /* End of if */
            NdisZeroMemory(pEntry->pWPA_Authenticator, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
            pEntry->pWPA_Authenticator->Role = WPA_Authenticator;
   		    RTMPInitTimer(pAd, &pEntry->pWPA_Authenticator->MsgRetryTimer, GET_TIMER_FUNCTION(Adhoc_WpaRetryExec), pEntry, FALSE);
            WpaProfileReset(pAd, pEntry, pEntry->pWPA_Authenticator);            
            // Supplicant
            if (pEntry->pWPA_Supplicant == NULL) {
                os_alloc_mem(pAd, (PUCHAR *) &pEntry->pWPA_Supplicant, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
                if (pEntry->pWPA_Supplicant == NULL)
                    goto MEMORY_FAILURE; 
            } /* End of if */
            NdisZeroMemory(pEntry->pWPA_Supplicant, sizeof(FOUR_WAY_HANDSHAKE_PROFILE));
            pEntry->pWPA_Supplicant->Role = WPA_Supplicant;
            WpaProfileReset(pAd, pEntry, pEntry->pWPA_Supplicant);
			break;    
    }  /* End of switch */

	DBGPRINT(RT_DEBUG_TRACE, ("<=== WpaProfileInit \n"));    
    return;

MEMORY_FAILURE:
    DBGPRINT(RT_DEBUG_ERROR, ("WpaProfileInit: No memory for 4-way Handshake Profile !!!\n")); 
    return;
} /* End of WpaProfileInit */


VOID WpaProfileRelease(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry)
{
	DBGPRINT(RT_DEBUG_TRACE, ("===> WpaProfileRelease \n"));

    switch (pEntry->WpaRole)
    {
        BOOLEAN         Cancelled;         
        PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile;
		case WPA_Authenticator:
            if (pEntry->pWPA_Authenticator) {
                p4WayProfile = pEntry->pWPA_Authenticator;
                if (p4WayProfile->MsgRetryTimer.Valid)
                    RTMPCancelTimer(&p4WayProfile->MsgRetryTimer, &Cancelled);
                os_free_mem(pAd, (PUCHAR) p4WayProfile);
            } /* End of if */
			break;
		case WPA_Supplicant:
            if (pEntry->pWPA_Supplicant) {
                p4WayProfile = pEntry->pWPA_Supplicant;
                if (p4WayProfile->MsgRetryTimer.Valid)
                    RTMPCancelTimer(&p4WayProfile->MsgRetryTimer, &Cancelled);
                os_free_mem(pAd, (PUCHAR) p4WayProfile);
            } /* End of if */
			break;
		case WPA_BOTH:
            // Authenticator
            if (pEntry->pWPA_Authenticator) {
                p4WayProfile = pEntry->pWPA_Authenticator;
                if (p4WayProfile->MsgRetryTimer.Valid)
                    RTMPCancelTimer(&p4WayProfile->MsgRetryTimer, &Cancelled);
                os_free_mem(pAd, (PUCHAR) p4WayProfile);
            } /* End of if */
            // Supplicant
            if (pEntry->pWPA_Supplicant) {
                p4WayProfile = pEntry->pWPA_Supplicant;
                if (p4WayProfile->MsgRetryTimer.Valid)
                    RTMPCancelTimer(&p4WayProfile->MsgRetryTimer, &Cancelled);
                os_free_mem(pAd, (PUCHAR) p4WayProfile);
            } /* End of if */
			break;    
    }  /* End of switch */
    pEntry->pWPA_Authenticator = NULL;
    pEntry->pWPA_Supplicant = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("<=== WpaProfileRelease \n"));    
    return;
} /* End of WpaProfileRelease */


VOID WpaProfileReset(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile)
{
	BOOLEAN     Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("===> WpaProfileReset \n"));
    
    if (p4WayProfile == NULL)
        return;
    
    if (p4WayProfile->MsgRetryTimer.Valid)
        RTMPCancelTimer(&p4WayProfile->MsgRetryTimer, &Cancelled);

    p4WayProfile->WpaState = AS_INITPSK;
    p4WayProfile->MsgRetryCounter = 4;
    if (ADHOC_ON(pAd))
        p4WayProfile->MsgRetryCounter = 10;   
    NdisZeroMemory(p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY);        
} /* End of WpaProfileInit */


VOID WpaProfileDataHook(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile)
{
	DBGPRINT(RT_DEBUG_TRACE, ("===> WpaProfileDataHook \n"));

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
    	switch(p4WayProfile->RxMsgType)
    	{
    		case EAPOL_PAIR_MSG_1:
    		case EAPOL_PAIR_MSG_3:
                p4WayProfile->AA = pEntry->Addr;
                p4WayProfile->SPA = pAd->CurrentAddress;
    			break;
    		case EAPOL_PAIR_MSG_2:
                p4WayProfile->pGTK = pAd->StaCfg.GTK;
                p4WayProfile->DefaultKeyId = pAd->StaCfg.DefaultKeyId;
                NdisZeroMemory(p4WayProfile->TxTsc, 6); //Eddy??
    		case EAPOL_MSG_INVALID:
                p4WayProfile->MsgRetryTimeInterval = PEER_MSG1_RETRY_EXEC_INTV;
    		case EAPOL_PAIR_MSG_4:
                p4WayProfile->AA = pAd->CurrentAddress;
                p4WayProfile->SPA = pEntry->Addr;
    			break;
        } /* End of switch */
   		p4WayProfile->pBssid = pAd->CommonCfg.Bssid;
   		p4WayProfile->GroupCipher = pAd->StaCfg.GroupCipher;
        p4WayProfile->pPMK = pAd->StaCfg.PMK;
        p4WayProfile->pRSN_IE = pAd->StaCfg.RSN_IE;
        p4WayProfile->RSNIELen = pAd->StaCfg.RSNIE_Len;
	}
#endif // CONFIG_STA_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("<=== WpaProfileDataHook \n"));
} /* End of WpaProfileHook */


/*
    ==========================================================================
    Description:
       Start 4-way HS when rcv EAPOL_START which may create by our driver in assoc.c
    Return:
    ==========================================================================
*/
VOID Adhoc_WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem) 
{   
    MAC_TABLE_ENTRY     *pEntry;
    PHEADER_802_11      pHeader;

    DBGPRINT(RT_DEBUG_TRACE, ("WpaEAPOLStartAction ===> \n"));
   
    pHeader = (PHEADER_802_11)Elem->Msg;
    
    //For normaol PSK, we enqueue an EAPOL-Start command to trigger the process.
    if (Elem->MsgLen == 6)
        pEntry = MacTableLookup(pAd, Elem->Msg);
    else
    {
        pEntry = MacTableLookup(pAd, pHeader->Addr2);
    } /* End of if */
    
    if ((pEntry) 
        && ((pEntry->WpaRole == WPA_Authenticator) || (pEntry->WpaRole == WPA_BOTH)))
    {    
        PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator;

        if (pEntry->pWPA_Authenticator == NULL)
            WpaProfileInit(pAd, pEntry);

        pAuthenticator = pEntry->pWPA_Authenticator;
        
		DBGPRINT(RT_DEBUG_TRACE, (" PortSecured(%d), WpaState(%d), AuthMode(%d), PMKID_CacheIdx(%d) \n", pEntry->PortSecured, pAuthenticator->WpaState, pEntry->AuthMode, pEntry->PMKID_CacheIdx));

        if ((pEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (pAuthenticator->WpaState < AS_PTKSTART)
            && ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) 
                || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK) 
                || ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->PMKID_CacheIdx != ENTRY_NOT_FOUND))))
        {
            pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
            pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

            WpaProfileReset(pAd, pEntry, pAuthenticator);
            pAuthenticator->RxMsgType = EAPOL_MSG_INVALID;
            pAuthenticator->TxMsgType = EAPOL_PAIR_MSG_1;
            pAuthenticator->WpaState = AS_PTKSTART;
            NdisZeroMemory(pAuthenticator->ReplayCounter, LEN_KEY_DESC_REPLAY);
            WpaProfileDataHook(pAd, pEntry, pAuthenticator);
            WpaEAPOLKeySend(pAd, pEntry, pAuthenticator);
        } /* End of if */
    } /* End of if */

	DBGPRINT(RT_DEBUG_TRACE, ("<=== WpaEAPOLStartAction \n"));    
} /* End of WpaEAPOLStartAction */



/*
    ==========================================================================
    Description:
        This is state machine function. 
        When receiving EAPOL packets which is for 802.1x key management. 
        Use both in WPA, and WPAPSK case. 
        In this function, further dispatch to different functions according to the received packet. 3 categories are:
          1.  normal 4-way pairwisekey and 2-way groupkey handshake
          2.  MIC error (Countermeasures attack)  report packet from STA.
          3.  Request for pairwise/group key update from STA
    Return:
    ==========================================================================
*/
VOID Adhoc_WpaEAPOLKeyAction(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem) 
{	
    MAC_TABLE_ENTRY     *pEntry;    
    PHEADER_802_11      pHeader;
    PEAPOL_PACKET       pEapolPacket;	
	KEY_INFO			peerKeyInfo;
	UINT				eapol_len;
    PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile;
    
    DBGPRINT(RT_DEBUG_TRACE, ("WpaEAPOLKeyAction ===>\n"));

    // Check the packet length
    if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("The length of eapol-key packet is invalid.\n"));
        goto EXIT;
    } /* End of if */

    // Parsing the packet
    pHeader = (PHEADER_802_11) Elem->Msg;
    pEapolPacket = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	eapol_len = CONV_ARRARY_TO_UINT16(pEapolPacket->Body_Len) + LENGTH_EAPOL_H;
	NdisMoveMemory((PUCHAR)&peerKeyInfo, (PUCHAR)&pEapolPacket->KeyDesc.KeyInfo, sizeof(KEY_INFO));
	*((USHORT *)&peerKeyInfo) = cpu2le16(*((USHORT *)&peerKeyInfo));

    // Check eapol-key length and field
	if (eapol_len > Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H)
	{
        DBGPRINT(RT_DEBUG_ERROR, ("The length of EAPoL packet is invalid.\n"));
        goto EXIT;
    } /* End of if */
    if (((pEapolPacket->ProVer != EAPOL_VER) && (pEapolPacket->ProVer != EAPOL_VER2)) || 
        ((pEapolPacket->KeyDesc.Type != WPA1_KEY_DESC) && (pEapolPacket->KeyDesc.Type != WPA2_KEY_DESC)))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Key descripter does not match with WPA rule\n"));
        goto EXIT;
    } /* End of if */

    // Search the entry from mac table
    pEntry = MacTableLookup(pAd, pHeader->Addr2);
    if (!pEntry || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))		
        goto EXIT;
	DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPoL-Key frame from STA %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pEntry->Addr)));

    // Compare the entry and packet cipher
    if (pEntry->Sst != SST_ASSOC)
        goto EXIT;
	if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
        goto EXIT;
	if ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) && (peerKeyInfo.KeyDescVer != KEY_DESC_TKIP))
    {
	    /* The value 1 shall be used for all EAPOL-Key frames to and from a STA when 
	       neither the group nor pairwise ciphers are CCMP for Key Descriptor 1. */
        DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(TKIP) \n"));
        goto EXIT;
    }	
    else if ((pEntry->WepStatus == Ndis802_11Encryption3Enabled) && (peerKeyInfo.KeyDescVer != KEY_DESC_AES))
    {
        /* The value 2 shall be used for all EAPOL-Key frames to and from a STA when 
           either the pairwise or the group cipher is AES-CCMP for Key Descriptor 2. */
        DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(AES) \n"));
        goto EXIT;
    } /* End of if */

    if (peerKeyInfo.Request == 1)
    {
        if (peerKeyInfo.Error == 1)
        {
            /*
                The Supplicant uses a single Michael MIC Failure Report frame 
                to report a MIC failure event to the Authenticator. 
                A Michael MIC Failure Report is an EAPOL-Key frame with 
                the following Key Information field bits set to 1: 
                MIC bit, Error bit, Request bit, Secure bit.
            */
            DBGPRINT(RT_DEBUG_ERROR, ("Received an Michael MIC Failure Report, active countermeasure \n"));
            //RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
	goto EXIT;
        } else {
            if (peerKeyInfo.KeyType == GROUPKEY)
            {
            } else if (peerKeyInfo.KeyType == PAIRWISEKEY) {
            } /* End of if */
        } /* End of if */
        goto EXIT;
    } /* End of if */


    /* 
        Determine the Authenticator or Supplicant by Key Ack (bit 7) of the Key Information
        4-Way Handshake 
            - Authenticator  == Message1(KeyAck=1) ==> Supplicant (Triggered by Adhoc_WpaStart4WayHS)
            - Authenticator <== Message2(KeyAck=0) ==  Supplicant
            - Authenticator  == Message3(KeyAck=1) ==> Supplicant
            - Authenticator <== Message4(KeyAck=0) ==  Supplicant

        Group Key Handshake
            - Authenticator  == Message1(KeyAck=1) ==> Supplicant (Triggered by WpaStart2WayGroupHS)
            - Authenticator <== Message2(KeyAck=0) ==  Supplicant
    */
    if (peerKeyInfo.KeyAck == 1) {
        if (pEntry->pWPA_Supplicant == NULL) {
            WpaProfileInit(pAd, pEntry);
            if (ADHOC_ON(pAd)) {
                pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;
                RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER);
            } /* End of if */
        } /* End of if */
        p4WayProfile = pEntry->pWPA_Supplicant;
        if ((!p4WayProfile) || p4WayProfile->WpaState < AS_INITPSK)
            goto EXIT;

    } else {
        if (pEntry->pWPA_Authenticator == NULL)
            WpaProfileInit(pAd, pEntry);
        p4WayProfile = pEntry->pWPA_Authenticator;
        if ((!p4WayProfile) || (p4WayProfile->WpaState < AS_INITPSK) || (p4WayProfile->WpaState >= AS_PTKINITDONE))
            goto EXIT;
    } /* End of if */

        
    /* 
        Determine the message type
        Message format:
            EAPOL-Key(S,M,A,I,K,KeyRSC,ANonce/SNonce,MIC,RSNIE,GTK[N])
        4-Way Handshake
            - Message1: EAPOL-Key(0,0,1,0,P,0,ANonce,0,0,0)
            - Message2: EAPOL-Key(0,1,0,0,P,0,SNonce,MIC,RSNIE,0)
            - Message3: EAPOL-Key(1,1,1,1,P,KeyRSC,ANonce,MIC,RSNIE,GTK[N])
            - Message4: EAPOL-Key(1,1,0,0,P,0,0,MIC,0,0)

        Group Key Handshake
            - Message1: EAPOL-Key(1,1,1,0,G,KeyRSC,0,MIC,0,GTK[N])
            - Message2: EAPOL-Key(1,1,0,0,G,0,0,MIC,0,0)

        Notes: Different between WPA and WPA2
            - Message2 of WPA : Secure = 0, KeyDataLen != 0 (RSNIE)
            - Message4 of WPA : Secure = 0, KeyDataLen  = 0
            - Message2 of WPA2: Secure = 0, KeyDataLen != 0 (RSNIE)
            - Message4 of WPA2: Secure = 1, KeyDataLen  = 0
    */
    p4WayProfile->RxMsgType = EAPOL_MSG_INVALID;
    if (peerKeyInfo.KeyType == PAIRWISEKEY)
    {
        if (peerKeyInfo.KeyAck == 1)
        {
            if (peerKeyInfo.KeyMic == 0)
                p4WayProfile->RxMsgType = EAPOL_PAIR_MSG_1; //Message1: KeyAck=1, KeyMic = 0
            else
                p4WayProfile->RxMsgType = EAPOL_PAIR_MSG_3; //Message3: KeyAck=1, KeyMic = 1        
        } else {
            p4WayProfile->RxMsgType = EAPOL_PAIR_MSG_4;
            
            if ((peerKeyInfo.Secure == 0) 
               && (CONV_ARRARY_TO_UINT16(pEapolPacket->KeyDesc.KeyDataLen) != 0))
                p4WayProfile->RxMsgType = EAPOL_PAIR_MSG_2;
        } /* End of if */
    } else if ((peerKeyInfo.KeyType == GROUPKEY) && (peerKeyInfo.Secure == 1)) {
        // GROUPKEY Message1: KeyAck=1, Message2: KeyAck=0
        p4WayProfile->RxMsgType = EAPOL_GROUP_MSG_2 - peerKeyInfo.KeyAck;
    } /* End of if */
    if ((p4WayProfile->RxMsgType != EAPOL_PAIR_MSG_4) && (p4WayProfile->RxMsgType != EAPOL_GROUP_MSG_2))
        p4WayProfile->TxMsgType = p4WayProfile->RxMsgType + 1;
    
    WpaProfileDataHook(pAd, pEntry, p4WayProfile);

    // Message handle
	switch(p4WayProfile->RxMsgType)
	{
		case EAPOL_PAIR_MSG_1:            
            WpaPeerPairMsg1Action(pAd, pEntry, p4WayProfile, Elem);
			break;
		case EAPOL_PAIR_MSG_2:
            WpaPeerPairMsg2Action(pAd, pEntry, p4WayProfile, Elem);
			break;
		case EAPOL_PAIR_MSG_3:
            WpaPeerPairMsg3Action(pAd, pEntry, p4WayProfile, Elem);
			break;
		case EAPOL_PAIR_MSG_4:
            if (p4WayProfile->WpaState < AS_PTKINIT_NEGOTIATING)
                goto EXIT;
            WpaPeerPairMsg4Action(pAd, pEntry, p4WayProfile, Elem);
			break;
		case EAPOL_GROUP_MSG_1:
//            PeerGroupMsg1Action(pAd, pEntry, Elem);
			break;
		case EAPOL_GROUP_MSG_2:
//            PeerGroupMsg2Action(pAd, pEntry, &Elem->Msg[LENGTH_802_11], (Elem->MsgLen - LENGTH_802_11));
			break;
    } /* End of switch */

EXIT:
	DBGPRINT(RT_DEBUG_TRACE, ("<=== WpaEAPOLKeyAction \n"));    
    return;
} /* End of Adhoc_WpaEAPOLKeyAction */

static VOID WpaEAPOLKeySend(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile) 
{
    BOOLEAN         Cancelled;    

    DBGPRINT(RT_DEBUG_TRACE, ("WpaEAPOLKeySend ===>\n"));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		DBGPRINT(RT_DEBUG_ERROR, ("WpaEAPOLKeySend: The interface is closed.\n"));
        goto EXIT;
	} /* End of if */

    if (!pEntry) {
        DBGPRINT(RT_DEBUG_ERROR, ("WpaEAPOLKeySend: The entry doesn't exist.\n"));		
        goto EXIT;
    } /* End of if */

	if (!p4WayProfile) {
		DBGPRINT(RT_DEBUG_ERROR, ("WpaEAPOLKeySend: The 4-way profile doesn't initialize.\n"));		
        goto EXIT;
    } /* End of if */

	if (p4WayProfile->pBssid == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("WpaEAPOLKeySend: No corresponding Authenticator.\n"));		
        goto EXIT;
    } /* End of if */

    if ((p4WayProfile->WpaState < AS_INITPMK) || (p4WayProfile->WpaState > AS_PTKINITDONE)) {
        DBGPRINT(RT_DEBUG_ERROR, ("WpaEAPOLKeySend: Not expect calling=%d\n", p4WayProfile->WpaState));
        goto EXIT;
    } /* End of if */

    if (p4WayProfile->MsgRetryTimer.Valid)
        RTMPCancelTimer(&p4WayProfile->MsgRetryTimer, &Cancelled);

	switch(p4WayProfile->TxMsgType)
	{
		case EAPOL_PAIR_MSG_1:
            WpaPeerPairMsg1Send(pAd, pEntry, p4WayProfile);
			break;
		case EAPOL_PAIR_MSG_2:
            WpaPeerPairMsg2Send(pAd, pEntry, p4WayProfile);
			break;
		case EAPOL_PAIR_MSG_3:
            WpaPeerPairMsg3Send(pAd, pEntry, p4WayProfile);
			break;
		case EAPOL_PAIR_MSG_4:
            WpaPeerPairMsg4Send(pAd, pEntry, p4WayProfile);
			break;
		case EAPOL_GROUP_MSG_1:
			break;
		case EAPOL_GROUP_MSG_2:
			break;
    } /* End of switch */

EXIT:
	DBGPRINT(RT_DEBUG_TRACE, ("<=== WpaEAPOLKeySend \n"));    
    return;
} /* End of WpaEAPOLKeySend */

/*
    ==========================================================================
    Description:
        Check the validity of the received EAPoL frame
    Return:
        TRUE if all parameters are OK, 
        FALSE otherwise
    ==========================================================================
 */
BOOLEAN Adhoc_PeerWpaMessageSanity (
    IN 	PRTMP_ADAPTER 		pAd, 
    IN 	PEAPOL_PACKET 		pMsg, 
    IN 	ULONG 				MsgLen, 
    IN 	UCHAR				MsgType,
	IN  PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile,    
    IN 	MAC_TABLE_ENTRY  	*pEntry)
{
	UCHAR			mic[LEN_KEY_DESC_MIC], digest[80], KEYDATA[MAX_LEN_OF_RSNIE];
	BOOLEAN			bReplayDiff = FALSE;
	BOOLEAN			bWPA2 = FALSE;
	KEY_INFO		EapolKeyInfo;	
	UCHAR			GroupKeyIndex = 0;
	
	
	NdisZeroMemory(mic, sizeof(mic));
	NdisZeroMemory(digest, sizeof(digest));
	NdisZeroMemory(KEYDATA, sizeof(KEYDATA));
	NdisZeroMemory((PUCHAR)&EapolKeyInfo, sizeof(EapolKeyInfo));
	
	NdisMoveMemory((PUCHAR)&EapolKeyInfo, (PUCHAR)&pMsg->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	*((USHORT *)&EapolKeyInfo) = cpu2le16(*((USHORT *)&EapolKeyInfo));

	// Choose WPA2 or not
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;

	// 0. Check MsgType
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("The message type is invalid(%d)! \n", MsgType));
		return FALSE;
	}
				
	// 1. Replay counter check	
 	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1)	// For supplicant
    {
    	// First validate replay counter, only accept message with larger replay counter.
		// Let equal pass, some AP start with all zero replay counter
		UCHAR	ZeroReplay[LEN_KEY_DESC_REPLAY];
		
        NdisZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);
		if ((RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY) != 1) &&
			(RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
    	{
			bReplayDiff = TRUE;
    	}						
 	}
	else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2)	// For authenticator
	{
		// check Replay Counter coresponds to MSG from authenticator, otherwise discard
    	if (!NdisEqualMemory(pMsg->KeyDesc.ReplayCounter, p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY))
    	{	
			bReplayDiff = TRUE;	        
    	}
	}

	// Replay Counter different condition
	if (bReplayDiff)
	{
	
#ifdef SYSTEM_LOG_SUPPORT
		// send wireless event - for replay counter different
		if (pAd->CommonCfg.bWirelessEvent)
			RTMPSendWirelessEvent(pAd, IW_REPLAY_COUNTER_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 
#endif

		if (MsgType < EAPOL_GROUP_MSG_1)
		{
           	DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in pairwise msg %d of 4-way handshake!\n", MsgType));
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));
		}
		
		hex_dump("Receive replay counter ", pMsg->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
		hex_dump("Current replay counter ", p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY);	
        return FALSE;
	}

	// 2. Verify MIC except Pairwise Msg1
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		UCHAR			rcvd_mic[LEN_KEY_DESC_MIC];
		UINT			eapol_len = CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4;

		// Record the received MIC for check later
		NdisMoveMemory(rcvd_mic, pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
		NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
							
        if (EapolKeyInfo.KeyDescVer == KEY_DESC_TKIP)	// TKIP
        {	
            RT_HMAC_MD5(p4WayProfile->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, mic, MD5_DIGEST_SIZE);
        }
        else if (EapolKeyInfo.KeyDescVer == KEY_DESC_AES)	// AES        
        {                        
            RT_HMAC_SHA1(p4WayProfile->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, digest, SHA1_DIGEST_SIZE);
            NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
        }
	
        if (!NdisEqualMemory(rcvd_mic, mic, LEN_KEY_DESC_MIC))
        {

#ifdef SYSTEM_LOG_SUPPORT
			// send wireless event - for MIC different
			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_MIC_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 
#endif

			if (MsgType < EAPOL_GROUP_MSG_1)
			{
            	DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in pairwise msg %d of 4-way handshake!\n", MsgType));
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));
			}
	
			hex_dump("Received MIC", rcvd_mic, LEN_KEY_DESC_MIC);
			hex_dump("Desired  MIC", mic, LEN_KEY_DESC_MIC);

			return FALSE;
        }        
	}

	// 1. Decrypt the Key Data field if GTK is included.
	// 2. Extract the context of the Key Data field if it exist.	 
	// The field in pairwise_msg_2_WPA1(WPA2) & pairwise_msg_3_WPA1 is clear.
	// The field in group_msg_1_WPA1(WPA2) & pairwise_msg_3_WPA2 is encrypted.
	if (CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen) > 0)
	{		
		// Decrypt this field		
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
		{					
			if((EapolKeyInfo.KeyDescVer == KEY_DESC_AES))
			{
				UINT aes_unwrap_len = 0;

   				AES_Key_Unwrap(pMsg->KeyDesc.KeyData, 
                               CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
  							   &p4WayProfile->PTK[LEN_PTK_KCK], LEN_PTK_KEK, 
   							   KEYDATA, &aes_unwrap_len);
    			SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, aes_unwrap_len);
			} 
			else	  
			{
				TKIP_GTK_KEY_UNWRAP(&pEntry->PTK[LEN_PTK_KCK], 
									pMsg->KeyDesc.KeyIv,									
									pMsg->KeyDesc.KeyData, 
									CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
									KEYDATA);
			}	

			if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
				GroupKeyIndex = EapolKeyInfo.KeyIndex;
			
		}
		else if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3 && !bWPA2))
		{					
			NdisMoveMemory(KEYDATA, pMsg->KeyDesc.KeyData, CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen));			     
		}
		else
		{
			
			return TRUE;
		}

		// Parse Key Data field to 
		// 1. verify RSN IE for pairwise_msg_2_WPA1(WPA2) ,pairwise_msg_3_WPA1(WPA2)
		// 2. verify KDE format for pairwise_msg_3_WPA2, group_msg_1_WPA2
		// 3. update shared key for pairwise_msg_3_WPA2, group_msg_1_WPA1(WPA2)
		if (!RTMPParseEapolKeyData(pAd, KEYDATA, 
								  CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen), 
								  GroupKeyIndex, MsgType, bWPA2, pEntry))
		{
			return FALSE;
		}
	}

	return TRUE;
	
} /* End of Adhoc_PeerWpaMessageSanity */


static VOID WpaPeerPairMsg1Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator)
{
    UCHAR           Header802_3[14];
	UCHAR   		*mpool;
    PEAPOL_PACKET	pEapolFrame;

	// Increment replay counter by 1
	ADD_ONE_To_64BIT_VAR(pAuthenticator->ReplayCounter);
	
	// Randomly generate ANonce		
	GenRandom(pAd, (UCHAR *)pAuthenticator->pBssid, pAuthenticator->ANonce);	

	// Allocate memory for output
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	
	// Construct EAPoL message - Pairwise Msg 1
	// EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)		
	Adhoc_ConstructEapolMsg(pEntry,
					  pAuthenticator->GroupCipher,
					  pAuthenticator->TxMsgType,
					  0,					// Default key index
					  pAuthenticator->ANonce,
					  NULL,					// TxRSC
					  NULL,					// GTK
					  NULL,					// RSNIE
					  0,					// RSNIE length	
					  pAuthenticator,
					  pEapolFrame);

    if (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
    {
        UCHAR	digest[80], PMK_key[20];
        PKEY_DESCRIPTER  pKeyDesc = &pEapolFrame->KeyDesc;

        pKeyDesc->KeyData[0] = 0xDD;
        pKeyDesc->KeyData[2] = 0x00;
        pKeyDesc->KeyData[3] = 0x0F;
        pKeyDesc->KeyData[4] = 0xAC;
        pKeyDesc->KeyData[5] = 0x04;

        NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
        NdisMoveMemory(&PMK_key[8], pAuthenticator->AA, MAC_ADDR_LEN);
        NdisMoveMemory(&PMK_key[14], pAuthenticator->SPA, MAC_ADDR_LEN);
        RT_HMAC_SHA1(pAd->StaCfg.PMK, PMK_LEN, PMK_key, 20, digest, LEN_PMKID);

        NdisMoveMemory(&pKeyDesc->KeyData[6], digest, LEN_PMKID);
        pKeyDesc->KeyData[1] = 0x14;// 4+LEN_PMKID
        INC_UINT16_TO_ARRARY(pKeyDesc->KeyDataLen, 6 + LEN_PMKID);    			
        INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
    } /* End of if */
        
	// Make outgoing frame
    MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pAuthenticator->AA, EAPOL);            
    RTMPToWirelessSta(pAd, pEntry, Header802_3, 
					  LENGTH_802_3, (PUCHAR)pEapolFrame, 
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, 
					  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	// Trigger Retry Timer
    RTMPSetTimer(&pAuthenticator->MsgRetryTimer, pAuthenticator->MsgRetryTimeInterval);
    
	// Update State
    pAuthenticator->WpaState = AS_PTKSTART;

	os_free_mem(NULL, mpool);

	DBGPRINT(RT_DEBUG_ERROR, ("WpaPeerPairMsg1Send: send Msg1 of 4-way \n"));
} /* End of WpaPeerPairMsg1Send */ 


static VOID WpaPeerPairMsg2Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant)
{
	UCHAR               Header802_3[14];
	UCHAR   			*mpool;
    PEAPOL_PACKET		pEapolFrame;

	// Generate random SNonce
	GenRandom(pAd, (UCHAR *)pSupplicant->SPA, pSupplicant->SNonce);

    // Calculate PTK(ANonce, SNonce)
    WpaDerivePTK(pAd,
                pSupplicant->pPMK,
		     	pSupplicant->ANonce,
			 	pSupplicant->AA,
			 	pSupplicant->SNonce,
			 	pSupplicant->SPA,
			 	pSupplicant->PTK,
			    LEN_PTK);
		
	// Update WpaState
	pSupplicant->WpaState = AS_PTKINIT_NEGOTIATING;

	// Allocate memory for output
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	// Construct EAPoL message - Pairwise Msg 2
	//  EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,DataKD_M2)
	Adhoc_ConstructEapolMsg(pEntry,
	                  pSupplicant->GroupCipher,
					  pSupplicant->TxMsgType,  
					  0,				// DefaultKeyIdx
					  pSupplicant->SNonce,
					  NULL,				// TxRsc
					  NULL,				// GTK
	                  pSupplicant->pRSN_IE,
	                  pSupplicant->RSNIELen,
					  pSupplicant,
					  pEapolFrame);

	// Make outgoing frame
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pSupplicant->SPA, EAPOL);	
	
	RTMPToWirelessSta(pAd, pEntry, 
					  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame, 
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, TRUE);

	os_free_mem(NULL, mpool);
    
	DBGPRINT(RT_DEBUG_ERROR, ("WpaPeerPairMsg2Send: send Msg2 of 4-way \n"));    
} /* End of WpaPeerPairMsg2Send */

static VOID WpaPeerPairMsg3Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator)
{
	UCHAR   			*mpool;
	PEAPOL_PACKET		pEapolFrame;
    UCHAR               Header802_3[LENGTH_802_3];

	// Allocate memory for input
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	    
	// Increment replay counter by 1
	ADD_ONE_To_64BIT_VAR(pAuthenticator->ReplayCounter);

	// Construct EAPoL message - Pairwise Msg 3
	Adhoc_ConstructEapolMsg(pEntry,
    					  pAuthenticator->GroupCipher,
    					  pAuthenticator->TxMsgType,
						  pAuthenticator->DefaultKeyId,
						  pAuthenticator->ANonce,
						  pAuthenticator->TxTsc,
						  pAuthenticator->pGTK,	
						  pAuthenticator->pRSN_IE,
						  pAuthenticator->RSNIELen,
						  pAuthenticator,
						  pEapolFrame);

    // Make outgoing frame
    MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pAuthenticator->AA, EAPOL);            
    RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3, 
					  (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, 
					  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

    RTMPSetTimer(&pAuthenticator->MsgRetryTimer, pAuthenticator->MsgRetryTimeInterval);

    // Update State
    pAuthenticator->WpaState = AS_PTKINIT_NEGOTIATING;

    os_free_mem(NULL, mpool);

	DBGPRINT(RT_DEBUG_ERROR, ("WpaPeerPairMsg3Send: send Msg3 of 4-way \n"));    
} /* End of WpaPeerPairMsg3Send */


static VOID WpaPeerPairMsg4Send (
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant)
{
	UCHAR               Header802_3[14];
	UCHAR				*mpool;
    PEAPOL_PACKET		pEapolFrame;
    
	// Allocate memory for output
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	// Construct EAPoL message - Pairwise Msg 4
	Adhoc_ConstructEapolMsg(pEntry,
					  pSupplicant->GroupCipher,
					  pSupplicant->TxMsgType,  
					  0,					// group key index not used in message 4
					  NULL,					// Nonce not used in message 4
					  NULL,					// TxRSC not used in message 4
					  NULL,					// GTK not used in message 4
					  NULL,					// RSN IE not used in message 4
					  0,
					  pSupplicant,
					  pEapolFrame);

	// Update WpaState
	pSupplicant->WpaState = AS_PTKINITDONE;

	// open 802.1x port control and privacy filter
	if (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK || 
		pEntry->AuthMode == Ndis802_11AuthModeWPA2)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("PeerPairMsg3Action: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(pSupplicant->GroupCipher)));
	}

	// Init 802.3 header and send out
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pSupplicant->SPA, EAPOL);	
	RTMPToWirelessSta(pAd, pEntry, 
					  Header802_3, sizeof(Header802_3), 
					  (PUCHAR)pEapolFrame, 
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, TRUE);

	os_free_mem(NULL, mpool);

	DBGPRINT(RT_DEBUG_ERROR, ("WpaPeerPairMsg4Send: send Msg4 of 4-way \n"));

    Wpa4WayComplete(pAd, pEntry);
} /* End of WpaPeerPairMsg4Send */


/*
	========================================================================
	
	Routine Description:
		Process Pairwise key Msg-1 of 4-way handshaking and send Msg-2 

	Arguments:
		pAd			Pointer	to our adapter
		Elem		Message body
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
static VOID WpaPeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant,
    IN MLME_QUEUE_ELEM  *Elem)
{
	PEAPOL_PACKET		pMsg1;
	UINT            	MsgLen;	
   	   
	DBGPRINT(RT_DEBUG_ERROR, ("===> Adhoc_PeerPairMsg1Action: receive Msg1 of 4-way \n"));

	// Store the received frame
	pMsg1 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;
	
	// Sanity Check peer Pairwise message 1 - Replay Counter
	if (Adhoc_PeerWpaMessageSanity(pAd, pMsg1, MsgLen, EAPOL_PAIR_MSG_1, pSupplicant, pEntry) == FALSE)
		return;
    
	// Store Replay counter, it will use to verify message 3 and construct message 2
	NdisMoveMemory(pSupplicant->ReplayCounter, pMsg1->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);		
        
	// Store ANonce
	NdisMoveMemory(pSupplicant->ANonce, pMsg1->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);		

    WpaEAPOLKeySend(pAd, pEntry, pSupplicant);
	DBGPRINT(RT_DEBUG_ERROR, ("<=== PeerPairMsg1Action \n"));
} /* End of WpaPeerPairMsg1Action */


/*
    ==========================================================================
    Description:
        When receiving the second packet of 4-way pairwisekey handshake.
    Return:
    ==========================================================================
*/
static VOID WpaPeerPairMsg2Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator,
    IN MLME_QUEUE_ELEM  *Elem) 
{   
    PHEADER_802_11      pHeader;
	PEAPOL_PACKET       pMsg2;
	UINT            	MsgLen;
    BOOLEAN             Cancelled;
    
    DBGPRINT(RT_DEBUG_ERROR, ("===> Adhoc_PeerPairMsg2Action: receive Msg2 of 4-way \n"));

    // pointer to 802.11 header
	pHeader = (PHEADER_802_11)Elem->Msg;

	// skip 802.11_header(24-byte) and LLC_header(8) 
	pMsg2 = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];       
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;
   
	// Store SNonce
	NdisMoveMemory(pAuthenticator->SNonce, pMsg2->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

    // Calculate PTK(ANonce, SNonce)
    WpaDerivePTK(pAd,
                pAuthenticator->pPMK,
		     	pAuthenticator->ANonce,
			 	pAuthenticator->AA,
			 	pAuthenticator->SNonce,
			 	pAuthenticator->SPA,
			 	pAuthenticator->PTK,
			    LEN_PTK);

	// Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE
	if (Adhoc_PeerWpaMessageSanity(pAd, pMsg2, MsgLen, EAPOL_PAIR_MSG_2, pAuthenticator, pEntry) == FALSE)
		return;

    RTMPCancelTimer(&pAuthenticator->MsgRetryTimer, &Cancelled);
    WpaEAPOLKeySend(pAd, pEntry, pAuthenticator);
    
	DBGPRINT(RT_DEBUG_ERROR, ("<=== WpaPeerPairMsg2Action \n"));
} /* End of WpaPeerPairMsg2Action */


/*
	========================================================================
	
	Routine Description:
		Process Pairwise key Msg 3 of 4-way handshaking and send Msg 4 

	Arguments:
		pAd	Pointer	to our adapter
		Elem		Message body
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
static VOID WpaPeerPairMsg3Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant,    
    IN MLME_QUEUE_ELEM  *Elem) 
{
	PHEADER_802_11		pHeader;
	PEAPOL_PACKET		pMsg3;
	UINT            	MsgLen;				
    
	DBGPRINT(RT_DEBUG_ERROR, ("===> Adhoc_PeerPairMsg3Action: receive Msg3 of 4-way \n"));
			
	// Record 802.11 header & the received EAPOL packet Msg3
	pHeader	= (PHEADER_802_11) Elem->Msg;
	pMsg3 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	// Sanity Check peer Pairwise message 3 - Replay Counter, MIC, RSNIE
	if (Adhoc_PeerWpaMessageSanity(pAd, pMsg3, MsgLen, EAPOL_PAIR_MSG_3, pSupplicant, pEntry) == FALSE)
		return;
	
	// Save Replay counter, it will use construct message 4
	NdisMoveMemory(pSupplicant->ReplayCounter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	// Double check ANonce
	if (!NdisEqualMemory(pSupplicant->ANonce, pMsg3->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
		return;

    WpaEAPOLKeySend(pAd, pEntry, pSupplicant);
	DBGPRINT(RT_DEBUG_ERROR, ("<=== Adhoc_PeerPairMsg3Action\n"));
} /* End of WpaPeerPairMsg3Action */


/*
    ==========================================================================
    Description:
        When receiving the last packet of 4-way pairwisekey handshake.
        Initilize 2-way groupkey handshake following.
    Return:
    ==========================================================================
*/
static VOID WpaPeerPairMsg4Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator,    
    IN MLME_QUEUE_ELEM  *Elem) 
{    
	PEAPOL_PACKET   	pMsg4;    
    PHEADER_802_11      pHeader;
    UINT            	MsgLen;
    BOOLEAN             Cancelled;
    
    DBGPRINT(RT_DEBUG_ERROR, ("===> Adhoc_PeerPairMsg4Action: receive Msg4 of 4-way\n"));

    // pointer to 802.11 header
    pHeader = (PHEADER_802_11)Elem->Msg;

    // skip 802.11_header(24-byte) and LLC_header(8) 
    pMsg4 = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H]; 
    MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

    // Sanity Check peer Pairwise message 4 - Replay Counter, MIC
    if (Adhoc_PeerWpaMessageSanity(pAd, pMsg4, MsgLen, EAPOL_PAIR_MSG_4, pAuthenticator, pEntry) == FALSE)
        return;

    pAuthenticator->WpaState = AS_PTKINITDONE;
    RTMPCancelTimer(&pAuthenticator->MsgRetryTimer, &Cancelled);
        
    if (pEntry->AuthMode == Ndis802_11AuthModeWPA2 || 
        pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
    {
        pEntry->GTKState = REKEY_ESTABLISHED;

#ifdef SYSTEM_LOG_SUPPORT
        // send wireless event - for set key done WPA2
        if (pAd->CommonCfg.bWirelessEvent)
            RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 	 
#endif
		
    } /* End of if */
    
    DBGPRINT(RT_DEBUG_ERROR, ("<=== Adhoc_PeerPairMsg4Action\n"));

    Wpa4WayComplete(pAd, pEntry);    
} /* End of WpaPeerPairMsg4Action */


static VOID Wpa4WayComplete(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry)
{
    PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = NULL; 
    PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant = NULL; 

    DBGPRINT(RT_DEBUG_ERROR, ("===> Adhoc_Wpa4WayComplete\n"));
    
    pAuthenticator = pEntry->pWPA_Authenticator;
    pSupplicant = pEntry->pWPA_Supplicant;

    if (((pEntry->WpaRole == WPA_BOTH) && (pAuthenticator->WpaState == AS_PTKINITDONE) && (pSupplicant->WpaState == AS_PTKINITDONE))
        || ((pEntry->WpaRole == WPA_Authenticator) && (pAuthenticator->WpaState == AS_PTKINITDONE))
        || ((pEntry->WpaRole == WPA_Supplicant) && (pSupplicant->WpaState == AS_PTKINITDONE)))
    {
        BOOLEAN Cancelled;

        RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);

#ifdef CONFIG_STA_SUPPORT
    	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
    	{			    
        	if (ADHOC_ON(pAd)) {
        		BOOLEAN compare_address;

        		compare_address = MAC_ADDR_EQUAL(pAd->CurrentAddress, pEntry->Addr)? TRUE : FALSE;
                if (compare_address) {
            		NdisMoveMemory(pEntry->PTK, pAuthenticator->PTK, 64);
                } else {
            		NdisMoveMemory(pEntry->PTK, pSupplicant->PTK, 64);
                }
            }
        }
#endif // CONFIG_STA_SUPPORT //

		WPAInstallPairwiseKey(pAd, 
							  BSS0, 
							  pEntry, 
							  FALSE);

        /* Upgrade state */
        pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
        pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
                        
		STA_PORT_SECURED(pAd);
	    // Indicate Connected for GUI
	    pAd->IndicateMediaState = NdisMediaStateConnected;

        WpaProfileRelease(pAd, pEntry);

        DBGPRINT(RT_DEBUG_OFF, ("Wpa4WayComplete - AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n", 
								pEntry->AuthMode, GetAuthMode(pEntry->AuthMode), 
								pEntry->WepStatus, GetEncryptType(pEntry->WepStatus), 
								pAd->StaCfg.GroupCipher, 
								GetEncryptType(pAd->StaCfg.GroupCipher)));
    } /* End of if */
} /* End of Wpa4WayComplete */


VOID Adhoc_WpaRetryExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    MAC_TABLE_ENTRY     *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

    if ((pEntry) && IS_ENTRY_CLIENT(pEntry))
    {
        PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;
        PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = pEntry->pWPA_Authenticator;
       
        DBGPRINT(RT_DEBUG_TRACE, ("Adhoc_WPARetryExec---> ReTryCounter=%d, WpaState=%d \n", pAuthenticator->MsgRetryCounter, pAuthenticator->WpaState));

        switch (pEntry->AuthMode)
        {
			case Ndis802_11AuthModeWPA:
            case Ndis802_11AuthModeWPAPSK:
			case Ndis802_11AuthModeWPA2:
            case Ndis802_11AuthModeWPA2PSK:
                if (pAuthenticator->MsgRetryCounter == 0)
                {
#ifdef SYSTEM_LOG_SUPPORT
					// send wireless event - for pairwise key handshaking timeout
					if (pAd->CommonCfg.bWirelessEvent)
						RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);
#endif

//                        WpaProfileRelease(pAd, pEntry, pAuthenticator);
                    MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);
                    DBGPRINT(RT_DEBUG_ERROR, ("Adhoc_WPARetryExec::MSG1 timeout\n"));
                } 
                else if ((pAuthenticator->TxMsgType == EAPOL_PAIR_MSG_1) || (pAuthenticator->TxMsgType == EAPOL_PAIR_MSG_3))
                {
                    WpaEAPOLKeySend(pAd, pEntry, pAuthenticator);
                } /* End of if */
                break;
            default:
                break;
        } /* End of switch */
        pAuthenticator->MsgRetryCounter--;        
    }
} // End of Adhoc_WPARetryExec


/*
	========================================================================
	
	Routine Description:
		Construct EAPoL message for WPA handshaking 
		Its format is below,
		
		+--------------------+
		| Protocol Version	 |  1 octet
		+--------------------+
		| Protocol Type		 |	1 octet	
		+--------------------+
		| Body Length		 |  2 octets
		+--------------------+
		| Descriptor Type	 |	1 octet
		+--------------------+
		| Key Information    |	2 octets
		+--------------------+
		| Key Length	     |  1 octet
		+--------------------+
		| Key Repaly Counter |	8 octets
		+--------------------+
		| Key Nonce		     |  32 octets
		+--------------------+
		| Key IV			 |  16 octets
		+--------------------+
		| Key RSC			 |  8 octets
		+--------------------+
		| Key ID or Reserved |	8 octets
		+--------------------+
		| Key MIC			 |	16 octets
		+--------------------+
		| Key Data Length	 |	2 octets
		+--------------------+
		| Key Data			 |	n octets
		+--------------------+
		

	Arguments:
		pAd			Pointer	to our adapter
				
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID    Adhoc_ConstructEapolMsg(
	IN 	PMAC_TABLE_ENTRY	pEntry,
    IN 	UCHAR				GroupKeyWepStatus,
    IN 	UCHAR				MsgType,  
    IN	UCHAR				DefaultKeyIdx,
	IN 	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
	IN  PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile,
    OUT PEAPOL_PACKET       pMsg)
{
	BOOLEAN	bWPA2 = FALSE;
	UCHAR	KeyDescVer;
    PKEY_DESCRIPTER pKeyDesc = &pMsg->KeyDesc;
	PKEY_INFO       pKeyInfo = &pMsg->KeyDesc.KeyInfo;

	// Choose WPA2 or not
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || 
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;
		
    // Init Packet and Fill header    
    pMsg->ProVer = EAPOL_VER;
    pMsg->ProType = EAPOLKey;

	// Default 95 bytes, the EAPoL-Key descriptor exclude Key-data field
	SET_UINT16_TO_ARRARY(pMsg->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG);

	// Fill in EAPoL descriptor
	if (bWPA2)
		pKeyDesc->Type = WPA2_KEY_DESC;
	else
		pKeyDesc->Type = WPA1_KEY_DESC;
			
	// Key Descriptor Version (bits 0-2) specifies the key descriptor version type
	// Fill in Key information, refer to IEEE Std 802.11i-2004 page 78 
	// When either the pairwise or the group cipher is AES, the KEY_DESC_AES shall be used.
	KeyDescVer = (((pEntry->WepStatus == Ndis802_11Encryption3Enabled) || 
	        		(GroupKeyWepStatus == Ndis802_11Encryption3Enabled)) ? (KEY_DESC_AES) : (KEY_DESC_TKIP));

	pKeyInfo->KeyDescVer = KeyDescVer;

	// Specify Key Type as Group(0) or Pairwise(1)
	if (MsgType >= EAPOL_GROUP_MSG_1)
		pKeyInfo->KeyType = GROUPKEY;
	else
		pKeyInfo->KeyType = PAIRWISEKEY;

	// Specify Key Index, only group_msg1_WPA1
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		pKeyInfo->KeyIndex = DefaultKeyIdx;
	
	if (MsgType == EAPOL_PAIR_MSG_3)
		pKeyInfo->Install = 1;
	
	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1))
		pKeyInfo->KeyAck = 1;

	if (MsgType != EAPOL_PAIR_MSG_1)	
		pKeyInfo->KeyMic = 1;
 
	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) || (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
       	pKeyInfo->Secure = 1;                   

	/* This subfield shall be set, and the Key Data field shall be encrypted, if
	   any key material (e.g., GTK or SMK) is included in the frame. */
	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1)))
        pKeyInfo->EKD_DL = 1;            

	// key Information element has done. 
	*(USHORT *)(pKeyInfo) = cpu2le16(*(USHORT *)(pKeyInfo));

	// Fill in Key Length
	if (bWPA2)
	{
		// In WPA2 mode, the field indicates the length of pairwise key cipher, 
		// so only pairwise_msg_1 and pairwise_msg_3 need to fill. 
		if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3))
			pKeyDesc->KeyLength[1] = ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_TK : LEN_AES_TK);
	}
	else
	{
		if (MsgType >= EAPOL_GROUP_MSG_1)
		{
			// the length of group key cipher
			pKeyDesc->KeyLength[1] = ((GroupKeyWepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_GTK : LEN_AES_GTK);
		}
		else
		{
			// the length of pairwise key cipher
			pKeyDesc->KeyLength[1] = ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_TK : LEN_AES_TK);			
		}				
	}			
	
 	// Fill in replay counter        		
    NdisMoveMemory(pKeyDesc->ReplayCounter, p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY);

	// Fill Key Nonce field		  
	// ANonce : pairwise_msg1 & pairwise_msg3
	// SNonce : pairwise_msg2
	// GNonce : group_msg1_wpa1	
	if ((MsgType <= EAPOL_PAIR_MSG_3) || ((!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))))
    	NdisMoveMemory(pKeyDesc->KeyNonce, KeyNonce, LEN_KEY_DESC_NONCE);

	// Fill key IV - WPA2 as 0, WPA1 as random
	if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
	{		
		// Suggest IV be random number plus some number,
		NdisMoveMemory(pKeyDesc->KeyIv, &KeyNonce[16], LEN_KEY_DESC_IV);		
        pKeyDesc->KeyIv[15] += 2;		
	}
	
    // Fill Key RSC field        
    // It contains the RSC for the GTK being installed.
	if ((TxRSC != NULL) && ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)))
	{		
        NdisMoveMemory(pKeyDesc->KeyRsc, TxRSC, 6);
	}

	// Clear Key MIC field for MIC calculation later   
    NdisZeroMemory(pKeyDesc->KeyMic, LEN_KEY_DESC_MIC);
	
	Adhoc_ConstructEapolKeyData(pEntry,
						  GroupKeyWepStatus, 
						  KeyDescVer,
						  MsgType, 
						  DefaultKeyIdx, 
						  GTK,
						  RSNIE,
						  RSNIE_Len,
						  p4WayProfile,
						  pMsg);
 
	// Calculate MIC and fill in KeyMic Field except Pairwise Msg 1.
	if (MsgType != EAPOL_PAIR_MSG_1)
        CalculateMIC(KeyDescVer, p4WayProfile->PTK, pMsg);

	DBGPRINT(RT_DEBUG_TRACE, ("===> ConstructEapolMsg for %s %s\n", ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
	DBGPRINT(RT_DEBUG_TRACE, ("	     Body length = %d \n", CONV_ARRARY_TO_UINT16(pMsg->Body_Len)));
	DBGPRINT(RT_DEBUG_TRACE, ("	     Key length  = %d \n", CONV_ARRARY_TO_UINT16(pKeyDesc->KeyLength)));
} //End of Adhoc_ConstructEapolMsg


/*
	========================================================================
	
	Routine Description:
		Construct the Key Data field of EAPoL message 

	Arguments:
		pAd			Pointer	to our adapter
		Elem		Message body
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID	Adhoc_ConstructEapolKeyData(
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN	UCHAR			GroupKeyWepStatus,
	IN	UCHAR			keyDescVer,
	IN 	UCHAR			MsgType,
	IN	UCHAR			DefaultKeyIdx,
	IN	UCHAR			*GTK,
	IN	UCHAR			*RSNIE,
	IN	UCHAR			RSNIE_LEN,
	IN  PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile,	
	OUT PEAPOL_PACKET   pMsg)
{
	UCHAR		*mpool, *Key_Data, *eGTK;  	  
	ULONG		data_offset;
	BOOLEAN		bWPA2Capable = FALSE;
	BOOLEAN		GTK_Included = FALSE;
    PKEY_DESCRIPTER pKeyDesc = &pMsg->KeyDesc;

	// Choose WPA2 or not
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || 
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2Capable = TRUE;

	if (MsgType == EAPOL_PAIR_MSG_1 || 
		MsgType == EAPOL_PAIR_MSG_4 || 
		MsgType == EAPOL_GROUP_MSG_2)
		return;
 
	// allocate memory pool
	os_alloc_mem(NULL, (PUCHAR *)&mpool, 1500);

    if (mpool == NULL)
		return;
        
	/* eGTK Len = 512 */
	eGTK = (UCHAR *) ROUND_UP(mpool, 4);
	/* Key_Data Len = 512 */
	Key_Data = (UCHAR *) ROUND_UP(eGTK + 512, 4);

	NdisZeroMemory(Key_Data, 512);
	SET_UINT16_TO_ARRARY(pKeyDesc->KeyDataLen, 0);
	data_offset = 0;
	
	// Encapsulate RSNIE in pairwise_msg2 & pairwise_msg3		
	if (RSNIE_LEN && ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)))
	{
		RTMPInsertRSNIE(&Key_Data[data_offset], 
						&data_offset,
						RSNIE, 
						RSNIE_LEN, 
						NULL, //PMK ID
						0); //PMK ID Length
	}

	// Encapsulate GTK 		
	// Only for pairwise_msg3_WPA2 and group_msg1
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2Capable) || (MsgType == EAPOL_GROUP_MSG_1))
	{
		UINT8	gtk_len;

		/* Decide the GTK length */ 
		if (GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
			gtk_len = LEN_AES_GTK;
		else
			gtk_len = LEN_TKIP_GTK;
		
		/* Insert GTK KDE format in WAP2 mode */
		if (bWPA2Capable)
		{
			/* Construct the common KDE format */
			WPA_ConstructKdeHdr(KDE_GTK, 2 + gtk_len, &Key_Data[data_offset]);
			data_offset += sizeof(KDE_HDR);

			// GTK KDE format - 802.11i-2004  Figure-43x
	        Key_Data[data_offset] = (DefaultKeyIdx & 0x03);
	        Key_Data[data_offset + 1] = 0x00;	// Reserved Byte
	        data_offset += 2;

		}

		/* Fill in GTK */
		NdisMoveMemory(&Key_Data[data_offset], GTK, gtk_len);
		data_offset += gtk_len;



		GTK_Included = TRUE;
	}

	/* If the Encrypted Key Data subfield (of the Key Information field) 
	   is set, the entire Key Data field shall be encrypted. */
	// This whole key-data field shall be encrypted if a GTK is included.
	// Encrypt the data material in key data field with KEK
	if (GTK_Included)
	{
		if ((keyDescVer == KEY_DESC_AES))
		{
			UCHAR 	remainder = 0;
			UCHAR	pad_len = 0;			
			UINT	wrap_len =0;

			// Key Descriptor Version 2 or 3: AES key wrap, defined in IETF RFC 3394, 
			// shall be used to encrypt the Key Data field using the KEK field from 
			// the derived PTK.

			// If the Key Data field uses the NIST AES key wrap, then the Key Data field 
			// shall be padded before encrypting if the key data length is less than 16 
			// octets or if it is not a multiple of 8. The padding consists of appending
			// a single octet 0xdd followed by zero or more 0x00 octets. 
			if ((remainder = data_offset & 0x07) != 0)
			{
				INT		i;
			
				pad_len = (8 - remainder);
				Key_Data[data_offset] = 0xDD;
				for (i = 1; i < pad_len; i++)
					Key_Data[data_offset + i] = 0;

				data_offset += pad_len;
			}
   			AES_Key_Wrap(Key_Data, (UINT) data_offset, 
   						 &p4WayProfile->PTK[LEN_PTK_KCK], LEN_PTK_KEK, 
   						 eGTK, &wrap_len);	
   			data_offset = wrap_len;
		}
		else
		{
   			TKIP_GTK_KEY_WRAP(&p4WayProfile->PTK[LEN_PTK_KCK], 
   								pMsg->KeyDesc.KeyIv,									
   								Key_Data, 
   								data_offset,
   								eGTK);
		}

		NdisMoveMemory(pKeyDesc->KeyData, eGTK, data_offset);
	}
	else
	{
		NdisMoveMemory(pKeyDesc->KeyData, Key_Data, data_offset);
	}

	// Update key data length field and total body length
	SET_UINT16_TO_ARRARY(pKeyDesc->KeyDataLen, data_offset);
	INC_UINT16_TO_ARRARY(pMsg->Body_Len, data_offset);

	os_free_mem(NULL, mpool);

} // End of Adhoc_ConstructEapolKeyData

