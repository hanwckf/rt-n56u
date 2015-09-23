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
	cmm_cfg.c

    Abstract:
    Ralink WiFi Driver configuration related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/



#include "rt_config.h"



INT ComputeChecksum(
	IN UINT PIN)
{
	INT digit_s;
    UINT accum = 0;

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 
	accum += 1 * ((PIN / 1000000) % 10); 
	accum += 3 * ((PIN / 100000) % 10); 
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10); 

	digit_s = (accum % 10);
	return ((10 - digit_s) % 10);
} // ComputeChecksum

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
    IN  BOOLEAN         bFromApcli,	
	IN	UCHAR			apidx)
{
	UCHAR	macAddr[MAC_ADDR_LEN];
	UINT 	iPin;
	UINT	checksum;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
	    if (bFromApcli)
	        NdisMoveMemory(&macAddr[0], pAd->ApCfg.ApCliTab[apidx].CurrentAddress, MAC_ADDR_LEN);
	    else
#endif // APCLI_SUPPORT //
		NdisMoveMemory(&macAddr[0], pAd->ApCfg.MBSSID[apidx].Bssid, MAC_ADDR_LEN);
	}
#endif // CONFIG_AP_SUPPORT //

	iPin = macAddr[3] * 256 * 256 + macAddr[4] * 256 + macAddr[5];

	iPin = iPin % 10000000;
	checksum = ComputeChecksum( iPin );
	iPin = iPin*10 + checksum;

	return iPin;
}

char* GetPhyMode(
	int Mode)
{
	switch(Mode)
	{
		case MODE_CCK:
			return "CCK";

		case MODE_OFDM:
			return "OFDM";
#ifdef DOT11_N_SUPPORT
		case MODE_HTMIX:
			return "HTMIX";

		case MODE_HTGREENFIELD:
			return "GREEN";
#endif // DOT11_N_SUPPORT //
		default:
			return "N/A";
	}
}


char* GetBW(
	int BW)
{
	switch(BW)
	{
		case BW_10:
			return "10M";

		case BW_20:
			return "20M";
#ifdef DOT11_N_SUPPORT
		case BW_40:
			return "40M";
#endif // DOT11_N_SUPPORT //
		default:
			return "N/A";
	}
}


/* 
    ==========================================================================
    Description:
        Set Country Region to pAd->CommonCfg.CountryRegion.
        This command will not work, if the field of CountryRegion in eeprom is programmed.
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetCountryRegion(
	IN PRTMP_ADAPTER	pAd, 
	IN PSTRING			arg,
	IN INT				band)
{
	LONG region;
	UCHAR *pCountryRegion;
	
	region = simple_strtol(arg, 0, 10);

	if (band == BAND_24G)
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
	else
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;
	
	// TODO: Is it neccesay for following check???
	// Country can be set only when EEPROM not programmed
	if (*pCountryRegion & 0x80)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("CfgSetCountryRegion():CountryRegion in eeprom was programmed\n"));
		return FALSE;
	}

	if((region >= 0) && 
	   (((band == BAND_24G) && ((region <= REGION_MAXIMUM_BG_BAND) || (region == REGION_31_BG_BAND))) || 
	    ((band == BAND_5G) && (region <= REGION_MAXIMUM_A_BAND) ))
	  )
	{
		*pCountryRegion= (UCHAR) region;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("CfgSetCountryRegion():region(%ld) out of range!\n", region));
		return FALSE;
	}

	return TRUE;
	
}


/* 
    ==========================================================================
    Description:
        Set Wireless Mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetWirelessMode(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT		MaxPhyMode = PHY_11G;
	LONG	WirelessMode;
	
#ifdef DOT11_N_SUPPORT
	if (!RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
		MaxPhyMode = PHY_11N_5G;
#endif // DOT11_N_SUPPORT //

	WirelessMode = simple_strtol(arg, 0, 10);

	/* check if chip support 5G band when WirelessMode is 5G band */
	if (PHY_MODE_IS_5G_BAND(WirelessMode))
	{
		if (!RFIC_IS_5G_BAND(pAd))
		{
			DBGPRINT(RT_DEBUG_ERROR,
					("phy mode> Error! The chip does not support 5G band %d!\n",
					pAd->RfIcType));
			return FALSE;
		}
	}

	if (WirelessMode <= MaxPhyMode)
	{
		pAd->CommonCfg.PhyMode = WirelessMode;
		pAd->CommonCfg.DesiredPhyMode = WirelessMode;
		return TRUE;
	}
	
	return FALSE;
	
}


/* maybe can be moved to GPL code, ap_mbss.c, but the code will be open */
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
BOOLEAN RT_CfgMbssWirelessModeSameBand(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			WirelessModeNew)
{
	BOOLEAN FlgIsOldMode24G = TRUE;
	BOOLEAN FlgIsNewMode24G = TRUE;


	if ((pAd->CommonCfg.PhyMode == PHY_11A) ||
		(pAd->CommonCfg.PhyMode == PHY_11AN_MIXED) ||
		(pAd->CommonCfg.PhyMode == PHY_11N_5G))
	{
		FlgIsOldMode24G = FALSE;
	}

	if ((WirelessModeNew == PHY_11A) ||
		(WirelessModeNew == PHY_11AN_MIXED) ||
		(WirelessModeNew == PHY_11N_5G))
	{
		FlgIsNewMode24G = FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE,
			("mbss> Old phy mode %d, New phy mode %d!\n",
			pAd->CommonCfg.PhyMode, WirelessModeNew));

	if (FlgIsOldMode24G != FlgIsNewMode24G)
		return FALSE; /* different phy band */

	return TRUE; /* same phy band */
}


UCHAR RT_CfgMbssWirelessModeMaxGet(
	IN	PRTMP_ADAPTER	pAd)
{
	MULTISSID_STRUCT *pMbss;
	UCHAR MaxPhyMode = PHY_11G, WirelessMode;
	UINT32 IdBss;
	BOOLEAN IsAnyB = FALSE; /* any b mode exist */
	BOOLEAN IsAnyG = FALSE; /* any g mode exist */
	BOOLEAN IsAnyA = FALSE; /* any a mode exist */
	BOOLEAN IsAny24N = FALSE; /* any n mode in 2.4G band exist */
	BOOLEAN IsAny5N = FALSE; /* any n mode in 5G band exist */
	BOOLEAN IsAny5 = FALSE; /* any 5G mode */


#ifdef DOT11_N_SUPPORT
	if (!RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
		MaxPhyMode = PHY_11N_5G;
#endif // DOT11_N_SUPPORT //

	for(IdBss=0; IdBss<pAd->ApCfg.BssidNum; IdBss++)
	{
		pMbss = &pAd->ApCfg.MBSSID[IdBss];

		/* check if the phy mode is out of range */
		if (pMbss->PhyMode > MaxPhyMode)
			pMbss->PhyMode = PHY_11BG_MIXED; /* default */

		/* check if the phy mode is legal */
		if (pMbss->PhyMode == PHY_11ABG_MIXED)
			pMbss->PhyMode = PHY_11BG_MIXED;

		if (pMbss->PhyMode == PHY_11ABGN_MIXED)
			pMbss->PhyMode = PHY_11BGN_MIXED;

		if (pMbss->PhyMode == PHY_11AGN_MIXED)
			pMbss->PhyMode = PHY_11GN_MIXED;

		/* record the legacy phy mode */
		/*
			Not use array to avoid the value of PHY_11B is changed in the future
			If any code size problem, we can use array to replace if check.
		*/
		if (pMbss->PhyMode == PHY_11B)
			IsAnyB = TRUE;

		if (pMbss->PhyMode == PHY_11G)
			IsAnyG = TRUE;

		if (pMbss->PhyMode == PHY_11BG_MIXED)
		{
			IsAnyB = TRUE;
			IsAnyG = TRUE;
		}

		if (pMbss->PhyMode == PHY_11A)
		{
			IsAnyA = TRUE;
			IsAny5 = TRUE;
		}

#ifdef DOT11_N_SUPPORT
		/* record the N phy mode */
		if (pMbss->PhyMode == PHY_11N_5G)
		{
			IsAny5N = TRUE;
			IsAny5 = TRUE;
		}

		if (pMbss->PhyMode == PHY_11AN_MIXED)
		{
			IsAnyA = TRUE;
			IsAny5N = TRUE;
			IsAny5 = TRUE;
		}

		if (pMbss->PhyMode == PHY_11N_2_4G)
			IsAny24N = TRUE;

		if (pMbss->PhyMode == PHY_11BGN_MIXED)
		{
			IsAnyB = TRUE;
			IsAnyG = TRUE;
			IsAny24N = TRUE;
		}

		if (pMbss->PhyMode == PHY_11GN_MIXED)
		{
			IsAnyG = TRUE;
			IsAny24N = TRUE;
		}
#endif // DOT11_N_SUPPORT //
	}

	DBGPRINT(RT_DEBUG_TRACE,
			("mbss> b g a 2.4n 5n %d %d %d %d %d\n",
			IsAnyB, IsAnyG, IsAnyA, IsAny24N, IsAny5N));

	if (IsAny5 == 0)
	{
		if (IsAny24N == 0)
		{
			/* no N phy exists */
			if ((IsAnyB == 1) && (IsAnyG == 1))
				WirelessMode = PHY_11BG_MIXED; /* B & G phy exists */
			else if (IsAnyG == 1)
				WirelessMode = PHY_11G; /* no B phy exists */
			else
				WirelessMode = PHY_11B; /* no G phy exists */
		}
#ifdef DOT11_N_SUPPORT
		else
		{
			/* N phy exists */
			if ((IsAnyB == 1) && (IsAnyG == 1))
				WirelessMode = PHY_11BGN_MIXED; /* B & G phy exists */
			else if (IsAnyG == 1)
				WirelessMode = PHY_11GN_MIXED; /* no B phy exists */
			else
				WirelessMode = PHY_11N_2_4G; /* no G phy exists */
		}
#endif // DOT11_N_SUPPORT //
	}
	else
	{
		if (IsAny5N == 0)
		{
			/* no N phy exists */
			WirelessMode = PHY_11A; /* A phy exists */
		}
#ifdef DOT11_N_SUPPORT
		else
		{
			/* N phy exists */
			if (IsAnyA == 1)
				WirelessMode = PHY_11AN_MIXED; /* A phy exists */
			else
				WirelessMode = PHY_11N_5G;
		}
#endif // DOT11_N_SUPPORT //
	} /* End of if */

	DBGPRINT(RT_DEBUG_TRACE, ("mbss> Get WirelessMode = %d\n", WirelessMode));
	return WirelessMode;
}


/* 
    ==========================================================================
    Description:
        Set Wireless Mode for MBSS
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetMbssWirelessMode(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32	MaxPhyMode = PHY_11G;
	UINT32	WirelessMode;
	
#ifdef DOT11_N_SUPPORT
	if (!RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
		MaxPhyMode = PHY_11N_5G;
#endif // DOT11_N_SUPPORT //

	WirelessMode = simple_strtol(arg, 0, 10);

	if ((WirelessMode == PHY_11ABG_MIXED) ||
		(WirelessMode == PHY_11ABGN_MIXED) ||
		(WirelessMode == PHY_11AGN_MIXED))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("mbss> Wrong phy mode for AP!\n"));
		return FALSE;
	}

	/* check if chip support 5G band when WirelessMode is 5G band */
	if (PHY_MODE_IS_5G_BAND(WirelessMode))
	{
		if (!RFIC_IS_5G_BAND(pAd))
		{
			DBGPRINT(RT_DEBUG_ERROR,
					("phy mode> Error! The chip does not support 5G band!\n"));
			return FALSE;
		}
	}

	if (WirelessMode <= MaxPhyMode)
	{
		if (pAd->ApCfg.BssidNum > 1)
		{
			/* pAd->CommonCfg.PhyMode = maximum capability of all MBSS */
			if (RT_CfgMbssWirelessModeSameBand(pAd, WirelessMode) == TRUE)
			{
				WirelessMode = RT_CfgMbssWirelessModeMaxGet(pAd);

				DBGPRINT(RT_DEBUG_TRACE,
						("mbss> Maximum phy mode = %d!\n", WirelessMode));
			}
			else
			{
				UINT32 IdBss;

				/* replace all phy mode with the one with different band */
				DBGPRINT(RT_DEBUG_TRACE,
						("mbss> Different band with the current one!\n"));
				DBGPRINT(RT_DEBUG_TRACE,
						("mbss> Reset band of all BSS to the new one!\n"));

				for(IdBss=0; IdBss<pAd->ApCfg.BssidNum; IdBss++)
					pAd->ApCfg.MBSSID[IdBss].PhyMode = WirelessMode;
			}
		}

		pAd->CommonCfg.PhyMode = WirelessMode;
		pAd->CommonCfg.DesiredPhyMode = WirelessMode;
		return TRUE;
	}
	
	return FALSE;
}
#endif // MBSS_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


INT RT_CfgSetShortSlot(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	LONG ShortSlot;

	ShortSlot = simple_strtol(arg, 0, 10);

	if (ShortSlot == 1)
		pAd->CommonCfg.bUseShortSlotTime = TRUE;
	else if (ShortSlot == 0)
		pAd->CommonCfg.bUseShortSlotTime = FALSE;
	else
		return FALSE;  //Invalid argument 

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY base on KeyIdx
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	RT_CfgSetWepKey(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx)
{
	INT				KeyLen;
	INT				i;
	//UCHAR			CipherAlg = CIPHER_NONE;
	BOOLEAN			bKeyIsHex = FALSE;

	// TODO: Shall we do memset for the original key info??
	memset(pSharedKey, 0, sizeof(CIPHER_KEY));
	KeyLen = strlen(keyString);
	switch (KeyLen)
	{
		case 5: //wep 40 Ascii type
		case 13: //wep 104 Ascii type
			bKeyIsHex = FALSE;
			pSharedKey->KeyLen = KeyLen;
			NdisMoveMemory(pSharedKey->Key, keyString, KeyLen);
			break;
			
		case 10: //wep 40 Hex type
		case 26: //wep 104 Hex type
			for(i=0; i < KeyLen; i++)
			{
				if( !isxdigit(*(keyString+i)) )
					return FALSE;  //Not Hex value;
			}
			bKeyIsHex = TRUE;
			pSharedKey->KeyLen = KeyLen/2 ;
			AtoH(keyString, pSharedKey->Key, pSharedKey->KeyLen);
			break;
			
		default: //Invalid argument 
			DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx, keyString));
			return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
	DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWepKey:(KeyIdx=%d,type=%s, Alg=%s)\n", 
						keyIdx, (bKeyIsHex == FALSE ? "Ascii" : "Hex"), CipherName[pSharedKey->CipherAlg]));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set WPA PSK key

    Arguments:
        pAdapter	Pointer to our adapter
        keyString	WPA pre-shared key string
        pHashStr	String used for password hash function
        hashStrLen	Lenght of the hash string
        pPMKBuf		Output buffer of WPAPSK key

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetWPAPSKKey(
	IN RTMP_ADAPTER	*pAd, 
	IN PSTRING		keyString,
	IN UCHAR		*pHashStr,
	IN INT			hashStrLen,
	OUT PUCHAR		pPMKBuf)
{
	int keyLen;
	UCHAR keyMaterial[40];

	keyLen = strlen(keyString);
	if ((keyLen < 8) || (keyLen > 64))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPAPSK Key length(%d) error, required 8 ~ 64 characters!(keyStr=%s)\n", 
									keyLen, keyString));
		return FALSE;
	}

	memset(pPMKBuf, 0, 32);
	if (keyLen == 64)
	{
	    AtoH(keyString, pPMKBuf, 32);
	}
	else
	{
	    RtmpPasswordHash(keyString, pHashStr, hashStrLen, keyMaterial);
	    NdisMoveMemory(pPMKBuf, keyMaterial, 32);		
	}

	return TRUE;
}

INT	RT_CfgSetFixedTxPhyMode(
	IN	PSTRING			arg)
{
	INT		fix_tx_mode = FIXED_TXMODE_HT;
	UINT32	value;

	if (strcmp(arg, "OFDM") == 0 || strcmp(arg, "ofdm") == 0)
	{
		fix_tx_mode = FIXED_TXMODE_OFDM;
	}	
	else if (strcmp(arg, "CCK") == 0 || strcmp(arg, "cck") == 0)
	{
	    fix_tx_mode = FIXED_TXMODE_CCK;
	}
	else if (strcmp(arg, "HT") == 0 || strcmp(arg, "ht") == 0)
	{
	    fix_tx_mode = FIXED_TXMODE_HT;
	}
	else
	{
		value = simple_strtol(arg, 0, 10);
		// 1 : CCK
		// 2 : OFDM
		// otherwise : HT
		if (value == FIXED_TXMODE_CCK || value == FIXED_TXMODE_OFDM)
			fix_tx_mode = value;	
		else
			fix_tx_mode = FIXED_TXMODE_HT;
	}

	return fix_tx_mode;
					
}	

INT	RT_CfgSetMacAddress(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PSTRING			arg)
{
	INT	i, mac_len;
	
	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(arg);
	if(mac_len != 17)  
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : invalid length (%d)\n", __FUNCTION__, mac_len));
		return FALSE;
	}

	if(strcmp(arg, "00:00:00:00:00:00") == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : invalid mac setting \n", __FUNCTION__));
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++)
	{
		AtoH(arg, &pAd->CurrentAddress[i], 1);
		arg = arg + 3;
	}	

	pAd->bLocalAdminMAC = TRUE;
	return TRUE;
}

INT	RT_CfgSetTxMCSProc(
	IN	PSTRING			arg,
	OUT	BOOLEAN			*pAutoRate)
{
	INT	Value = simple_strtol(arg, 0, 10);
	INT	TxMcs;
	
	if ((Value >= 0 && Value <= 23) || (Value == 32)) // 3*3
	{
		TxMcs = Value;
		*pAutoRate = FALSE;
	}
	else
	{		
		TxMcs = MCS_AUTO;
		*pAutoRate = TRUE;
	}

	return TxMcs;

}

INT	RT_CfgSetAutoFallBack(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PSTRING			arg)
{
	TX_RTY_CFG_STRUC	tx_rty_cfg;
	UCHAR				AutoFallBack = (UCHAR)simple_strtol(arg, 0, 10);

	RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
	tx_rty_cfg.field.TxautoFBEnable = (AutoFallBack) ? 1 : 0;
	RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);	
	DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetAutoFallBack::(tx_rty_cfg=0x%x)\n", tx_rty_cfg.word));
	return TRUE;
}

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING		pPinCodeStr,
	OUT PWSC_CTRL   pWscControl)
{
	UINT pinCode;

	pinCode = (UINT) simple_strtol(pPinCodeStr, 0, 10); // When PinCode is 03571361, return value is 3571361.
	if (strlen(pPinCodeStr) == 4)
	{
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 4;
	}
	else if ( ValidateChecksum(pinCode) )
	{
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 8;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RT_CfgSetWscPinCode(): invalid Wsc PinCode (%d)\n", pinCode));
		return FALSE;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWscPinCode():Wsc PinCode=%d\n", pinCode));
	
	return TRUE;
	
}
#endif // WSC_INCLUDED //
