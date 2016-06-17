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

static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd);


INT ComputeChecksum(UINT PIN)
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
} /* ComputeChecksum*/

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
    IN  BOOLEAN         bFromApcli,
	IN	UCHAR			apidx)
{
	UCHAR	macAddr[MAC_ADDR_LEN];
	UINT 	iPin;
	UINT	checksum;

	NdisZeroMemory(macAddr, MAC_ADDR_LEN);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
	    if (bFromApcli)
	        NdisMoveMemory(&macAddr[0], pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, MAC_ADDR_LEN);
	    else
#endif /* APCLI_SUPPORT */
		NdisMoveMemory(&macAddr[0], pAd->ApCfg.MBSSID[apidx].wdev.if_addr, MAC_ADDR_LEN);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(&macAddr[0], pAd->CurrentAddress, MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */

	iPin = macAddr[3] * 256 * 256 + macAddr[4] * 256 + macAddr[5];

	iPin = iPin % 10000000;


	checksum = ComputeChecksum( iPin );
	iPin = iPin*10 + checksum;

	return iPin;
}


static char *phy_mode_str[]={"CCK", "OFDM", "HTMIX", "GF", "VHT"};
char* get_phymode_str(int Mode)
{
	if (Mode >= MODE_CCK && Mode <= MODE_VHT)
		return phy_mode_str[Mode];
	else
		return "N/A";
}


static UCHAR *phy_bw_str[] = {"20M", "40M", "80M", "10M"};
char* get_bw_str(int bandwidth)
{
	if (bandwidth >= BW_20 && bandwidth <= BW_10)
		return phy_bw_str[bandwidth];
	else
		return "N/A";
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
INT RT_CfgSetCountryRegion(RTMP_ADAPTER *pAd, RTMP_STRING *arg, INT band)
{
	LONG region;
	UCHAR *pCountryRegion;

	region = simple_strtol(arg, 0, 10);

	if (band == BAND_24G)
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
	else
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;

    /*
               1. If this value is set before interface up, do not reject this value.
               2. Country can be set only when EEPROM not programmed
    */
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE) && (*pCountryRegion & EEPROM_IS_PROGRAMMED))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CfgSetCountryRegion():CountryRegion in eeprom was programmed\n"));
		return FALSE;
	}

	if((region >= 0) &&
	   (((band == BAND_24G) &&((region <= REGION_MAXIMUM_BG_BAND) ||
	   (region == REGION_31_BG_BAND) || (region == REGION_32_BG_BAND) || (region == REGION_33_BG_BAND) )) ||
	    ((band == BAND_5G) && (region <= REGION_MAXIMUM_A_BAND) ))
	  )
	{
		*pCountryRegion= (UCHAR) region;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CfgSetCountryRegion():region(%ld) out of range!\n", region));
		return FALSE;
	}

	return TRUE;

}


static UCHAR CFG_WMODE_MAP[]={
	PHY_11BG_MIXED, (WMODE_B | WMODE_G), /* 0 => B/G mixed */
	PHY_11B, (WMODE_B), /* 1 => B only */
	PHY_11A, (WMODE_A), /* 2 => A only */
	PHY_11ABG_MIXED, (WMODE_A | WMODE_B | WMODE_G), /* 3 => A/B/G mixed */
	PHY_11G, WMODE_G, /* 4 => G only */
	PHY_11ABGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 5 => A/B/G/GN/AN mixed */
	PHY_11N_2_4G, (WMODE_GN), /* 6 => N in 2.4G band only */
	PHY_11GN_MIXED, (WMODE_G | WMODE_GN), /* 7 => G/GN, i.e., no CCK mode */
	PHY_11AN_MIXED, (WMODE_A | WMODE_AN), /* 8 => A/N in 5 band */
	PHY_11BGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN), /* 9 => B/G/GN mode*/
	PHY_11AGN_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 10 => A/AN/G/GN mode, not support B mode */
	PHY_11N_5G, (WMODE_AN), /* 11 => only N in 5G band */
	PHY_MODE_MAX, WMODE_INVALID /* default phy mode if not match */
};


static RTMP_STRING *BAND_STR[] = {"Invalid", "2.4G", "5G", "2.4G/5G"};
static RTMP_STRING *WMODE_STR[]= {"", "A", "B", "G", "gN", "aN", "AC"};

UCHAR *wmode_2_str(UCHAR wmode)
{
	UCHAR *str;
	INT idx, pos, max_len;

	max_len = WMODE_COMP * 3;
	if (os_alloc_mem(NULL, &str, max_len) == NDIS_STATUS_SUCCESS)
	{
		NdisZeroMemory(str, max_len);
		pos = 0;
		for (idx = 0; idx < WMODE_COMP; idx++)
		{
			if (wmode & (1 << idx)) {
				if ((strlen(str) +  strlen(WMODE_STR[idx + 1])) >= (max_len - 1))
					break;
				if (strlen(str)) {
					NdisMoveMemory(&str[pos], "/", 1);
					pos++;
				}
				NdisMoveMemory(&str[pos], WMODE_STR[idx + 1], strlen(WMODE_STR[idx + 1]));
				pos += strlen(WMODE_STR[idx + 1]);
			}
			if (strlen(str) >= max_len)
				break;
		}

		return str;
	}
	else
		return NULL;
}


RT_802_11_PHY_MODE wmode_2_cfgmode(UCHAR wmode)
{
	INT i, mode_cnt = sizeof(CFG_WMODE_MAP) / sizeof(UCHAR);

	for (i = 1; i < mode_cnt; i+=2)
	{
		if (CFG_WMODE_MAP[i] == wmode)
			return CFG_WMODE_MAP[i - 1];
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot get cfgmode by wmode(%x)\n",
				__FUNCTION__, wmode));

	return 0;
}


UCHAR cfgmode_2_wmode(UCHAR cfg_mode)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cfg_mode=%d\n", cfg_mode));
	if (cfg_mode >= PHY_MODE_MAX)
		cfg_mode =  PHY_MODE_MAX;

	return CFG_WMODE_MAP[cfg_mode * 2 + 1];
}

BOOLEAN wmode_valid_and_correct(RTMP_ADAPTER *pAd, UCHAR* wmode)
{
	BOOLEAN ret = TRUE;

	if (*wmode == WMODE_INVALID)
		*wmode = (WMODE_B | WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC);

	while(1)
	{
		if (WMODE_CAP_5G(*wmode) && (!PHY_CAP_5G(pAd->chipCap.phy_caps)))
		{
			*wmode = *wmode & ~(WMODE_A | WMODE_AN | WMODE_AC);
		}
		else if (WMODE_CAP_2G(*wmode) && (!PHY_CAP_2G(pAd->chipCap.phy_caps)))
		{
			*wmode = *wmode & ~(WMODE_B | WMODE_G | WMODE_GN);
		}
		else if (WMODE_CAP_N(*wmode) && ((!PHY_CAP_N(pAd->chipCap.phy_caps)) || RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N)))
		{
			*wmode = *wmode & ~(WMODE_GN | WMODE_AN);
		}
		else if (WMODE_CAP_AC(*wmode) && (!PHY_CAP_AC(pAd->chipCap.phy_caps)))
		{
			*wmode = *wmode & ~(WMODE_AC);
		}

		if ( *wmode == 0 )
		{
			*wmode = (WMODE_B | WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC);
			break;
		}
		else
			break;
	}

	return ret;
}


BOOLEAN wmode_band_equal(UCHAR smode, UCHAR tmode)
{
	BOOLEAN eq = FALSE;
	UCHAR *str1, *str2;

	if ((WMODE_CAP_5G(smode) == WMODE_CAP_5G(tmode)) &&
		(WMODE_CAP_2G(smode) == WMODE_CAP_2G(tmode)))
		eq = TRUE;

	str1 = wmode_2_str(smode);
	str2 = wmode_2_str(tmode);
	if (str1 && str2)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("Old WirelessMode:%s(0x%x), "
			 "New WirelessMode:%s(0x%x)!\n",
			str1, smode, str2, tmode));
	}
	if (str1)
		os_free_mem(NULL, str1);
	if (str2)
		os_free_mem(NULL, str2);

	return eq;
}


/*
    ==========================================================================
    Description:
        Set Wireless Mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG cfg_mode;
	UCHAR wmode, *mode_str;

	cfg_mode = simple_strtol(arg, 0, 10);

	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): Invalid wireless mode(%ld, wmode=0x%x), ChipCap(%s)\n",
				__FUNCTION__, cfg_mode, wmode,
				BAND_STR[pAd->chipCap.phy_caps & 0x3]));
		return FALSE;
	}


	if (wmode_band_equal(pAd->CommonCfg.PhyMode, wmode) == TRUE)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Equal!\n"));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Not Equal!\n"));

	pAd->CommonCfg.PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;

	mode_str = wmode_2_str(wmode);
	if (mode_str)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set WMODE=%s(0x%x)\n",
				__FUNCTION__, mode_str, wmode));
		os_free_mem(NULL, mode_str);
	}

	return TRUE;
}


/* maybe can be moved to GPL code, ap_mbss.c, but the code will be open */
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT

static BOOLEAN wmode_valid(RTMP_ADAPTER *pAd, enum WIFI_MODE wmode)
{
	if ((WMODE_CAP_5G(wmode) && (!PHY_CAP_5G(pAd->chipCap.phy_caps))) ||
		(WMODE_CAP_2G(wmode) && (!PHY_CAP_2G(pAd->chipCap.phy_caps))) ||
		(WMODE_CAP_N(wmode) && RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
	)
		return FALSE;
	else
		return TRUE;
}

static UCHAR RT_CfgMbssWirelessModeMaxGet(RTMP_ADAPTER *pAd)
{
	UCHAR wmode = 0, *mode_str;
	INT idx;
	struct wifi_dev *wdev;

	for(idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		mode_str = wmode_2_str(wdev->PhyMode);
		if (mode_str)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(BSS%d): wmode=%s(0x%x)\n",
					__FUNCTION__, idx, mode_str, wdev->PhyMode));
			os_free_mem(pAd, mode_str);
		}
		wmode |= wdev->PhyMode;
	}

	mode_str = wmode_2_str(wmode);
	if (mode_str)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Combined WirelessMode = %s(0x%x)\n",
					__FUNCTION__, mode_str, wmode));
		os_free_mem(pAd, mode_str);
	}
	return wmode;
}


/*
    ==========================================================================
    Description:
        Set Wireless Mode for MBSS
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT cfg_mode;
	UCHAR wmode;
	cfg_mode = simple_strtol(arg, 0, 10);

	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
				__FUNCTION__, cfg_mode, wmode,
				BAND_STR[pAd->chipCap.phy_caps & 0x3]));
		return FALSE;
	}

	if (WMODE_CAP_5G(wmode) && WMODE_CAP_2G(wmode))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP cannot support 2.4G/5G band mxied mode!\n"));
		return FALSE;
	}



	if (pAd->ApCfg.BssidNum > 1)
	{
		/* pAd->CommonCfg.PhyMode = maximum capability of all MBSS */
		if (wmode_band_equal(pAd->CommonCfg.PhyMode, wmode) == TRUE)
		{
			wmode = RT_CfgMbssWirelessModeMaxGet(pAd);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("mbss> Maximum phy mode = %d!\n", wmode));
		}
		else
		{
			UINT32 IdBss;

			/* replace all phy mode with the one with different band */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("mbss> Different band with the current one!\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("mbss> Reset band of all BSS to the new one!\n"));

			for(IdBss=0; IdBss<pAd->ApCfg.BssidNum; IdBss++)
				pAd->ApCfg.MBSSID[IdBss].wdev.PhyMode = wmode;
		}
	}

	pAd->CommonCfg.PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	return TRUE;
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd)
{
		BOOLEAN TestFlag;
		TestFlag =	!strcmp(SetCmd, "Debug") ||
#ifdef CONFIG_APSTA_MIXED_SUPPORT
					!strcmp(SetCmd, "OpMode") ||
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST
					!strcmp(SetCmd, "CountryCode") ||
					!strcmp(SetCmd, "DfsType") ||
					!strcmp(SetCmd, "ChannelListAdd") ||
					!strcmp(SetCmd, "ChannelListShow") ||
					!strcmp(SetCmd, "ChannelListDel") ||
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef SINGLE_SKU
					!strcmp(SetCmd, "ModuleTxpower") ||
#endif /* SINGLE_SKU */
					FALSE; /* default */
       return TestFlag;
}


INT RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG ShortSlot;

	ShortSlot = simple_strtol(arg, 0, 10);

	if (ShortSlot == 1)
		pAd->CommonCfg.bUseShortSlotTime = TRUE;
	else if (ShortSlot == 0)
		pAd->CommonCfg.bUseShortSlotTime = FALSE;
	else
		return FALSE;  /*Invalid argument */

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
	IN	RTMP_STRING *keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx)
{
	INT				KeyLen;
	INT				i;
	/*UCHAR			CipherAlg = CIPHER_NONE;*/
	BOOLEAN			bKeyIsHex = FALSE;

	/* TODO: Shall we do memset for the original key info??*/
	memset(pSharedKey, 0, sizeof(CIPHER_KEY));
	KeyLen = strlen(keyString);
	switch (KeyLen)
	{
		case 5: /*wep 40 Ascii type*/
		case 13: /*wep 104 Ascii type*/
#ifdef MT_MAC
		case 16: /*wep 128 Ascii type*/
#endif
			bKeyIsHex = FALSE;
			pSharedKey->KeyLen = KeyLen;
			NdisMoveMemory(pSharedKey->Key, keyString, KeyLen);
			break;

		case 10: /*wep 40 Hex type*/
		case 26: /*wep 104 Hex type*/
#ifdef MT_MAC
		case 32: /*wep 128 Hex type*/
#endif
			for(i=0; i < KeyLen; i++)
			{
				if( !isxdigit(*(keyString+i)) )
					return FALSE;  /*Not Hex value;*/
			}
			bKeyIsHex = TRUE;
			pSharedKey->KeyLen = KeyLen/2 ;
			AtoH(keyString, pSharedKey->Key, pSharedKey->KeyLen);
			break;

		default: /*Invalid argument */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx, keyString));
			return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
#ifdef MT_MAC
	if (KeyLen == 32)
		pSharedKey->CipherAlg = CIPHER_WEP152;
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey:(KeyIdx=%d,type=%s, Alg=%s)\n",
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
	IN RTMP_STRING *keyString,
	IN INT			keyStringLen,
	IN UCHAR		*pHashStr,
	IN INT			hashStrLen,
	OUT PUCHAR		pPMKBuf)
{
	UCHAR keyMaterial[40];

	if ((keyStringLen < 8) || (keyStringLen > 64))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPAPSK Key length(%d) error, required 8 ~ 64 characters!(keyStr=%s)\n",
									keyStringLen, keyString));
		return FALSE;
	}

	NdisZeroMemory(pPMKBuf, 32);
	if (keyStringLen == 64)
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


INT	RT_CfgSetFixedTxPhyMode(RTMP_STRING *arg)
{
	INT fix_tx_mode = FIXED_TXMODE_HT;
	ULONG value;


	if (rtstrcasecmp(arg, "OFDM") == TRUE)
		fix_tx_mode = FIXED_TXMODE_OFDM;
	else if (rtstrcasecmp(arg, "CCK") == TRUE)
	    fix_tx_mode = FIXED_TXMODE_CCK;
	else if (rtstrcasecmp(arg, "HT") == TRUE)
	    fix_tx_mode = FIXED_TXMODE_HT;
	else if (rtstrcasecmp(arg, "VHT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_VHT;
	else
	{
		value = simple_strtol(arg, 0, 10);
		switch (value)
		{
			case FIXED_TXMODE_CCK:
			case FIXED_TXMODE_OFDM:
			case FIXED_TXMODE_HT:
			case FIXED_TXMODE_VHT:
				fix_tx_mode = value;
				break;
			default:
				fix_tx_mode = FIXED_TXMODE_HT;
		}
	}

	return fix_tx_mode;

}


INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i, mac_len;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(arg);
	if(mac_len != 17)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid length (%d)\n", __FUNCTION__, mac_len));
		return FALSE;
	}

	if(strcmp(arg, "00:00:00:00:00:00") == 0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid mac setting \n", __FUNCTION__));
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


INT	RT_CfgSetTxMCSProc(RTMP_STRING *arg, BOOLEAN *pAutoRate)
{
	INT	Value = simple_strtol(arg, 0, 10);
	INT	TxMcs;

	if ((Value >= 0 && Value <= 23) || (Value == 32)) /* 3*3*/
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


INT	RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR AutoFallBack = (UCHAR)simple_strtol(arg, 0, 10);

	if (AutoFallBack)
		AutoFallBack = TRUE;
	else
		AutoFallBack = FALSE;

	AsicSetAutoFallBack(pAd, (AutoFallBack) ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetAutoFallBack::(AutoFallBack=%d)\n", AutoFallBack));
	return TRUE;
}


#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pPinCodeStr,
	OUT PWSC_CTRL   pWscControl)
{
	UINT pinCode;

	pinCode = (UINT) simple_strtol(pPinCodeStr, 0, 10); /* When PinCode is 03571361, return value is 3571361.*/
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
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RT_CfgSetWscPinCode(): invalid Wsc PinCode (%d)\n", pinCode));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWscPinCode():Wsc PinCode=%d\n", pinCode));

	return TRUE;

}
#endif /* WSC_INCLUDED */

/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWNAME.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_giwname(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	UCHAR CurOpMode = OPMODE_AP;

	if (CurOpMode == OPMODE_AP)
	{
		strcpy(pData, "RTWIFI SoftAP");
	}

	return NDIS_STATUS_SUCCESS;
}


INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS, i;


	pObj = pObj; /* avoid compile warning */

	switch(cmd)
	{
		case CMD_RTPRIV_IOCTL_NETDEV_GET:
		/* get main net_dev */
		{
			VOID **ppNetDev = (VOID **)pData;
			*ppNetDev = (VOID *)(pAd->net_dev);
		}
			break;

		case CMD_RTPRIV_IOCTL_NETDEV_SET:
			{
				struct wifi_dev *wdev = NULL;
				/* set main net_dev */
				pAd->net_dev = pData;

#ifdef CONFIG_AP_SUPPORT
				if (pAd->OpMode == OPMODE_AP) {
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_dev = (void *)pData;
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.func_dev = (void *)&pAd->ApCfg.MBSSID[MAIN_MBSSID];
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.func_idx = MAIN_MBSSID;
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.sys_handle = (void *)pAd;
					RTMP_OS_NETDEV_SET_WDEV(pData, &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
					wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
				}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
				if (pAd->OpMode == OPMODE_STA) {
					pAd->StaCfg.wdev.if_dev = pData;
					pAd->StaCfg.wdev.func_dev = (void *)&pAd->StaCfg;
					pAd->StaCfg.wdev.func_idx = 0;
					pAd->StaCfg.wdev.sys_handle = (void *)pAd;
					RTMP_OS_NETDEV_SET_WDEV(pData, &pAd->StaCfg.wdev);
					wdev = &pAd->StaCfg.wdev;
				}
#endif /* CONFIG_STA_SUPPORT */
				if (wdev) {
					if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
								RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
						RtmpOSNetDevFree(pAd->net_dev);
					}
				}
				break;
			}
		case CMD_RTPRIV_IOCTL_OPMODE_GET:
		/* get Operation Mode */
			*(ULONG *)pData = pAd->OpMode;
			break;


		case CMD_RTPRIV_IOCTL_TASK_LIST_GET:
		/* get all Tasks */
		{
			RT_CMD_WAIT_QUEUE_LIST *pList = (RT_CMD_WAIT_QUEUE_LIST *)pData;

			pList->pMlmeTask = &pAd->mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
			pList->pTimerTask = &pAd->timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */
			pList->pCmdQTask = &pAd->cmdQTask;
#ifdef WSC_INCLUDED
			pList->pWscTask = &pAd->wscTask;
#endif /* WSC_INCLUDED */
		}
			break;

#ifdef RTMP_MAC_PCI
		case CMD_RTPRIV_IOCTL_IRQ_INIT:
			/* init IRQ */
			rtmp_irq_init(pAd);
			break;
#endif /* RTMP_MAC_PCI */

		case CMD_RTPRIV_IOCTL_IRQ_RELEASE:
			/* release IRQ */
			RTMP_OS_IRQ_RELEASE(pAd, pAd->net_dev);
			break;

#ifdef RTMP_MAC_PCI
		case CMD_RTPRIV_IOCTL_MSI_ENABLE:
			/* enable MSI */
			RTMP_MSI_ENABLE(pAd);
			*(ULONG **)pData = (ULONG *)(pObj->pci_dev);
			break;
#endif /* RTMP_MAC_PCI */

		case CMD_RTPRIV_IOCTL_NIC_NOT_EXIST:
			/* set driver state to fRTMP_ADAPTER_NIC_NOT_EXIST */
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			break;

		case CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR:
			RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
			break;

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
                case CMD_RTPRIV_IOCTL_USB_DEV_GET:
                /* get USB DEV */
                {
                        VOID **ppUsb_Dev = (VOID **)pData;
                        *ppUsb_Dev = (VOID *)(pObj->pUsb_Dev);
                }
                        break;

                case CMD_RTPRIV_IOCTL_USB_INTF_GET:
                /* get USB INTF */
                {
                        VOID **ppINTF = (VOID **)pData;
                        *ppINTF = (VOID *)(pObj->intf);
                }
                        break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_SET_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_CLEAR_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
			RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SEND_DISSASSOCIATE:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
			if (INFRA_ON(pAd) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
			{
				MLME_DISASSOC_REQ_STRUCT	DisReq;
				MLME_QUEUE_ELEM *MsgElem;
				os_alloc_mem(NULL, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));
				if (MsgElem)
				{
					COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
					DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;
					MsgElem->Machine = ASSOC_STATE_MACHINE;
					MsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
					MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
					NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));
					/* Prevent to connect AP again in STAMlmePeriodicExec*/
					pAd->MlmeAux.AutoReconnectSsidLen= 32;
					NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
					pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
					MlmeDisassocReqAction(pAd, MsgElem);
					os_free_mem(NULL, MsgElem);
				}
				/*				RtmpusecDelay(1000);*/
				RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1, NULL, NULL, 0);
			}
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_TEST:
		/* test driver state to fRTMP_ADAPTER_SUSPEND */
			*(UCHAR *)pData = RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_IDLE_RADIO_OFF_TEST:
		/* test driver state to fRTMP_ADAPTER_IDLE_RADIO_OFF */
			*(UCHAR *)pData = RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_IDLE_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
			ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
			ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
			break;

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_STATUS:
			*(UCHAR *)pData = (UCHAR)pAd->WOW_Cfg.bEnable;
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_ENABLE:
			ASIC_WOW_ENABLE(pAd);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_DISABLE:
			ASIC_WOW_DISABLE(pAd);
			break;
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */
#endif /* CONFIG_PM */

		case CMD_RTPRIV_IOCTL_AP_BSSID_GET:
			if (pAd->StaCfg.wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)
				NdisCopyMemory(pData, pAd->MlmeAux.Bssid, 6);
			else
				return NDIS_STATUS_FAILURE;
			break;

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_SET_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_CLEAR_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		/* RT28xxUsbAsicRadioOff */
			//RT28xxUsbAsicRadioOff(pAd);
			ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		/* RT28xxUsbAsicRadioOn */
			//RT28xxUsbAsicRadioOn(pAd);
			ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
			break;
#endif /* CONFIG_STA_SUPPORT */

		case CMD_RTPRIV_IOCTL_SANITY_CHECK:
		/* sanity check before IOCTL */
			if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
#ifdef IFUP_IN_PROBE
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif /* IFUP_IN_PROBE */
			)
			{
				if(pData == NULL ||	RT_isLegalCmdBeforeInfUp((RTMP_STRING *) pData) == FALSE)
				return NDIS_STATUS_FAILURE;
			}
			break;

		case CMD_RTPRIV_IOCTL_SIOCGIWFREQ:
		/* get channel number */
			*(ULONG *)pData = pAd->CommonCfg.Channel;
			break;
#ifdef CONFIG_SNIFFER_SUPPORT
		case CMD_RTPRIV_IOCTL_SNIFF_INIT:
			Monitor_Init(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_SNIFF_OPEN:
			if (Monitor_Open(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_SNIFF_CLOSE:
			if (Monitor_Close(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_SNIFF_REMOVE:
			Monitor_Remove(pAd);
			break;
#endif /*CONFIG_SNIFFER_SUPPORT*/



		case CMD_RTPRIV_IOCTL_BEACON_UPDATE:
		/* update all beacon contents */
#ifdef CONFIG_AP_SUPPORT
			APMakeAllBssBeacon(pAd);
			APUpdateAllBeaconFrame(pAd);
#endif /* CONFIG_AP_SUPPORT */
			break;

		case CMD_RTPRIV_IOCTL_RXPATH_GET:
		/* get the number of rx path */
			*(ULONG *)pData = pAd->Antenna.field.RxPath;
			break;

		case CMD_RTPRIV_IOCTL_CHAN_LIST_NUM_GET:
			*(ULONG *)pData = pAd->ChannelListNum;
			break;

		case CMD_RTPRIV_IOCTL_CHAN_LIST_GET:
		{
			UINT32 i;
			UCHAR *pChannel = (UCHAR *)pData;

			for (i = 1; i <= pAd->ChannelListNum; i++)
			{
				*pChannel = pAd->ChannelList[i-1].Channel;
				pChannel ++;
			}
		}
			break;

		case CMD_RTPRIV_IOCTL_FREQ_LIST_GET:
		{
			UINT32 i;
			UINT32 *pFreq = (UINT32 *)pData;
			UINT32 m;

			for (i = 1; i <= pAd->ChannelListNum; i++)
			{
				m = 2412000;
				MAP_CHANNEL_ID_TO_KHZ(pAd->ChannelList[i-1].Channel, m);
				(*pFreq) = m;
				pFreq ++;
			}
		}
			break;

#ifdef EXT_BUILD_CHANNEL_LIST
		case CMD_RTPRIV_SET_PRECONFIG_VALUE:
			/* Set some preconfigured value before interface up*/
			pAd->CommonCfg.DfsType = MAX_RD_REGION;
			break;
#endif /* EXT_BUILD_CHANNEL_LIST */



#ifdef RTMP_PCI_SUPPORT
		case CMD_RTPRIV_IOCTL_PCI_SUSPEND:
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_PCI_RESUME:
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_PCI_CSR_SET:
			pAd->CSRBaseAddress = (PUCHAR)Data;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->CSRBaseAddress =0x%lx, csr_addr=0x%lx!\n", (ULONG)pAd->CSRBaseAddress, (ULONG)Data));
			break;

		case CMD_RTPRIV_IOCTL_PCIE_INIT:
			RTMPInitPCIeDevice(pData, pAd);
			break;
#endif /* RTMP_PCI_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		case CMD_RTPRIV_IOCTL_CFG80211_CFG_START:
			RT_CFG80211_REINIT(pAd);
			RT_CFG80211_CRDA_REG_RULE_APPLY(pAd);
			break;
#endif /* RT_CFG80211_SUPPORT */

#ifdef INF_PPA_SUPPORT
		case CMD_RTPRIV_IOCTL_INF_PPA_INIT:
			os_alloc_mem(NULL, (UCHAR **)&(pAd->pDirectpathCb), sizeof(PPA_DIRECTPATH_CB));
			break;

		case CMD_RTPRIV_IOCTL_INF_PPA_EXIT:
			if (ppa_hook_directpath_register_dev_fn && (pAd->PPAEnable == TRUE))
			{
				UINT status;
				status = ppa_hook_directpath_register_dev_fn(&pAd->g_if_id, pAd->net_dev, NULL, 0);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unregister PPA::status=%d, if_id=%d\n", status, pAd->g_if_id));
			}
			os_free_mem(NULL, pAd->pDirectpathCb);
			break;
#endif /* INF_PPA_SUPPORT*/

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP:
		/* interface up */
		{
			RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;
			// TODO: Shiang-usw, this function looks have some problem, need to revise!
			if (VIRTUAL_IF_NUM(pAd) == 0)
			{
				ULONG start, end, diff_ms;
				/* Get the current time for calculating startup time */
				NdisGetSystemUpTime(&start);
				
				VIRTUAL_IF_INC(pAd);
				if (pInfConf->rt28xx_open(pAd->net_dev) != 0)
				{
					VIRTUAL_IF_DEC(pAd);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("rt28xx_open return fail!\n"));
					return NDIS_STATUS_FAILURE;
				}
				
				/* Get the current time for calculating startup time */
				NdisGetSystemUpTime(&end); diff_ms = (end-start)*1000/OS_HZ;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WiFi Startup Cost (%s): %lu.%03lus\n",
						RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev),diff_ms/1000,diff_ms%1000));
			}
			else
			{
				VIRTUAL_IF_INC(pAd);
#ifdef CONFIG_AP_SUPPORT
				{
					extern VOID APMakeAllBssBeacon(IN PRTMP_ADAPTER pAd);
					extern VOID  APUpdateAllBeaconFrame(IN PRTMP_ADAPTER pAd);
					APMakeAllBssBeacon(pAd);
					APUpdateAllBeaconFrame(pAd);
				}
#endif /* CONFIG_AP_SUPPORT */
			}
		}
			break;

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN:
		/* interface down */
		{
			RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

			VIRTUAL_IF_DEC(pAd);
			if (VIRTUAL_IF_NUM(pAd) == 0)
				pInfConf->rt28xx_close(pAd->net_dev);
		}
			break;

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET:
		/* get virtual interface number */
			*(ULONG *)pData = VIRTUAL_IF_NUM(pAd);
			break;

		case CMD_RTPRIV_IOCTL_INF_TYPE_GET:
		/* get current interface type */
			*(ULONG *)pData = pAd->infType;
			break;

		case CMD_RTPRIV_IOCTL_INF_STATS_GET:
			/* get statistics */
			{
				RT_CMD_STATS64 *pStats = (RT_CMD_STATS64 *)pData;
#ifdef CONFIG_AP_SUPPORT
				if(pAd->OpMode == OPMODE_AP)
				{
					INT index;
					BSS_STRUCT *pMBSSID;
					
					for(index = 0; index < MAX_MBSSID_NUM(pAd); index++)
					{
						if (pAd->ApCfg.MBSSID[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev))
						{
							break;
						}
					}

					if(index >= MAX_MBSSID_NUM(pAd))
					{
						//reset counters
						NdisZeroMemory(pStats, sizeof(RT_CMD_STATS64));

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMD_RTPRIV_IOCTL_INF_STATS_GET: can not find mbss I/F\n"));
						return NDIS_STATUS_FAILURE;
					}

					pMBSSID = &pAd->ApCfg.MBSSID[index];
					
					pStats->rx_bytes = pMBSSID->ReceivedByteCount.QuadPart;
					pStats->tx_bytes = pMBSSID->TransmittedByteCount.QuadPart;
					pStats->rx_packets = pMBSSID->RxCount;
					pStats->tx_packets = pMBSSID->TxCount;
					pStats->rx_errors = pMBSSID->RxErrorCount;
					pStats->tx_errors = 0;
					pStats->multicast = pMBSSID->mcPktsRx; /* multicast packets received */
					pStats->collisions = 0;
					pStats->rx_over_errors = 0;
					pStats->rx_crc_errors = 0;
					pStats->rx_frame_errors = 0;
					pStats->rx_fifo_errors = 0;
				}
#endif
#ifdef CONFIG_STA_SUPPORT
				if(pAd->OpMode == OPMODE_STA)
				{
					pStats->rx_packets = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
					pStats->tx_packets = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;
					pStats->rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
					pStats->tx_bytes = pAd->RalinkCounters.TransmittedByteCount;
					pStats->rx_errors = pAd->Counters8023.RxErrors;
					pStats->tx_errors = pAd->Counters8023.TxErrors;
					pStats->multicast = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;   /* multicast packets received*/
					pStats->collisions = 0;  /* Collision packets*/
					pStats->rx_over_errors = pAd->Counters8023.RxNoBuffer;                   /* receiver ring buff overflow*/
					pStats->rx_crc_errors = 0;/*pAd->WlanCounters.FCSErrorCount;      recved pkt with crc error*/
					pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
					pStats->rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   /* recv'r fifo overrun*/
				}
#endif
			}
			break;

		case CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET:
		/* get wireless statistics */
		{
			UCHAR CurOpMode = OPMODE_AP;
#ifdef CONFIG_AP_SUPPORT
			PMAC_TABLE_ENTRY pMacEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
			RT_CMD_IW_STATS *pStats = (RT_CMD_IW_STATS *)pData;

			pStats->qual = 0;
			pStats->level = 0;
			pStats->noise = 0;
			pStats->pStats = pAd->iw_stats;

#ifdef CONFIG_STA_SUPPORT
			if (pAd->OpMode == OPMODE_STA)
			{
				CurOpMode = OPMODE_STA;
			}
#endif /* CONFIG_STA_SUPPORT */

			/*check if the interface is down*/
			if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
				return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == OPMODE_AP)
			{
#ifdef APCLI_SUPPORT
				if ((pStats->priv_flags == INT_APCLI)
					)
				{
					INT ApCliIdx = ApCliIfLookUp(pAd, (PUCHAR)pStats->dev_addr);
					if ((ApCliIdx >= 0) && VALID_WCID(pAd->ApCfg.ApCliTab[ApCliIdx].MacTabWCID))
						pMacEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[ApCliIdx].MacTabWCID];
				}
				else
#endif /* APCLI_SUPPORT */
				{
					/*
						only AP client support wireless stats function.
						return NULL pointer for all other cases.
					*/
					pMacEntry = NULL;
				}
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
			if (CurOpMode == OPMODE_STA)
				pStats->qual = ((pAd->Mlme.ChannelQuality * 12)/10 + 10);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == OPMODE_AP)
			{
				if (pMacEntry != NULL)
					pStats->qual = ((pMacEntry->ChannelQuality * 12)/10 + 10);
				else
					pStats->qual = ((pAd->Mlme.ChannelQuality * 12)/10 + 10);
			}
#endif /* CONFIG_AP_SUPPORT */

			if (pStats->qual > 100)
				pStats->qual = 100;

#ifdef CONFIG_STA_SUPPORT
			if (CurOpMode == OPMODE_STA)
			{
				pStats->level =
					RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.AvgRssi[0],
									pAd->StaCfg.RssiSample.AvgRssi[1],
									pAd->StaCfg.RssiSample.AvgRssi[2]);
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == OPMODE_AP)
			{
				if (pMacEntry != NULL)
					pStats->level =
						RTMPMaxRssi(pAd, pMacEntry->RssiSample.AvgRssi[0],
										pMacEntry->RssiSample.AvgRssi[1],
										pMacEntry->RssiSample.AvgRssi[2]);
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			pStats->noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
										pAd->ApCfg.RssiSample.AvgRssi[1],
										pAd->ApCfg.RssiSample.AvgRssi[2]) -
										RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr[0],
										pAd->ApCfg.RssiSample.AvgSnr[1]);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			pStats->noise = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.AvgRssi[0],
										pAd->StaCfg.RssiSample.AvgRssi[1],
										pAd->StaCfg.RssiSample.AvgRssi[2]) -
										RTMPMinSnr(pAd, pAd->StaCfg.RssiSample.AvgSnr[0],
										pAd->StaCfg.RssiSample.AvgSnr[1]);
#endif /* CONFIG_STA_SUPPORT */
		}
			break;

		case CMD_RTPRIV_IOCTL_INF_MAIN_CREATE:
			*(VOID **)pData = RtmpPhyNetDevMainCreate(pAd);
			break;

		case CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET:
			*(ULONG *)pData = INT_MAIN;
			break;

		case CMD_RTPRIV_IOCTL_INF_MAIN_CHECK:
			if (Data != INT_MAIN)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_INF_P2P_CHECK:
			if (Data != INT_P2P)
				return NDIS_STATUS_FAILURE;
			break;

#ifdef WDS_SUPPORT
		case CMD_RTPRIV_IOCTL_WDS_INIT:
			WDS_Init(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_WDS_REMOVE:
			WDS_Remove(pAd);
			break;

		case CMD_RTPRIV_IOCTL_WDS_STATS_GET:
			if (Data == INT_WDS)
			{
				if (WDS_StatsGet(pAd, pData) != TRUE)
					return NDIS_STATUS_FAILURE;
			}
			else
				return NDIS_STATUS_FAILURE;
			break;
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
		case CMD_RTPRIV_IOCTL_APCLI_STATS_GET:
			if (Data == INT_APCLI)
			{
				if (ApCli_StatsGet(pAd, pData) != TRUE)
					return NDIS_STATUS_FAILURE;
			}
			else
				return NDIS_STATUS_FAILURE;
			break;
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_ATE
#ifdef CONFIG_QA
		case CMD_RTPRIV_IOCTL_ATE:
			RtmpDoAte(pAd, wrq, pData);
			break;
#endif /* CONFIG_QA */
#endif /* CONFIG_ATE */

		case CMD_RTPRIV_IOCTL_MAC_ADDR_GET:
			{
				UCHAR mac_addr[MAC_ADDR_LEN];
				USHORT Addr01, Addr23, Addr45;

				RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
				RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
				RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);

				mac_addr[0] = (UCHAR)(Addr01 & 0xff);
				mac_addr[1] = (UCHAR)(Addr01 >> 8);
				mac_addr[2] = (UCHAR)(Addr23 & 0xff);
				mac_addr[3] = (UCHAR)(Addr23 >> 8);
				mac_addr[4] = (UCHAR)(Addr45 & 0xff);
				mac_addr[5] = (UCHAR)(Addr45 >> 8);

				for(i=0; i<6; i++)
					*(UCHAR *)(pData+i) = mac_addr[i];
				break;
			}
#ifdef CONFIG_AP_SUPPORT
		case CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ:
		/* handle for SIOCGIWRATEQ */
		{
			RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
			HTTRANSMIT_SETTING HtPhyMode;

#ifdef APCLI_SUPPORT
			if (pRate->priv_flags == INT_APCLI)
				HtPhyMode = pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.HTPhyMode;
			else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
			if (pRate->priv_flags == INT_WDS)
				HtPhyMode = pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev.HTPhyMode;
			else
#endif /* WDS_SUPPORT */
			{
				HtPhyMode = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.HTPhyMode;
			}
			RtmpDrvMaxRateGet(pAd, HtPhyMode.field.MODE, HtPhyMode.field.ShortGI,
							HtPhyMode.field.BW, HtPhyMode.field.MCS,
							(UINT32 *)&pRate->BitRate);
		}
			break;
#endif /* CONFIG_AP_SUPPORT */

		case CMD_RTPRIV_IOCTL_SIOCGIWNAME:
			RtmpIoctl_rt_ioctl_giwname(pAd, pData, 0);
			break;

	}

#ifdef RT_CFG80211_SUPPORT
	if ((CMD_RTPRIV_IOCTL_80211_START <= cmd) &&
		(cmd <= CMD_RTPRIV_IOCTL_80211_END))
	{
		CFG80211DRV_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);
	}
#endif /* RT_CFG80211_SUPPORT */

	if (cmd >= CMD_RTPRIV_IOCTL_80211_COM_LATEST_ONE)
		return NDIS_STATUS_FAILURE;

	return Status;
}

/*
    ==========================================================================
    Description:
        Issue a site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage:
               1.) iwpriv ra0 set site_survey
    ==========================================================================
*/
INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid;

	//check if the interface is down
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (MONITOR_ON(pAd))
    	{
        	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
        	return -EINVAL;
    	}
	}
#endif // CONFIG_STA_SUPPORT //

    NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((strlen(arg) != 0) && (strlen(arg) <= MAX_LEN_OF_SSID))
    	{
        	NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
        	Ssid.SsidLength = strlen(arg);
		}

#ifdef AIRPLAY_SUPPORT 
		if(arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X'))
		{
				int ii,jj;
				CHAR temp[MAX_LEN_OF_SSID*2+1];

				NdisZeroMemory(Ssid.Ssid, MAX_LEN_OF_SSID);
				for(ii=2; ii<strlen(arg); ii++)
				{
						if(arg[ii] >= '0' && arg[ii] <= '9')
								temp[ii-2] = arg[ii] - '0';
						else if(arg[ii] >= 'A' && arg[ii] <= 'F')
								temp[ii-2] = arg[ii] - 'A' + 10;
						else if(arg[ii] >= 'a' && arg[ii] <= 'f')
								temp[ii-2] = arg[ii] - 'a' + 10;
				}

				temp[strlen(arg)-2]= '\0';
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s=>arg:",__FUNCTION__));
				for(ii=0; ii<strlen(arg)-2; ii++)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%x",temp[ii]));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

				jj=0;
				for(ii=0; ii<strlen(arg)-2; ii+=2)
				{
						if (jj > MAX_LEN_OF_SSID)
						{
								MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s=> unicode SSID len error.",__FUNCTION__));
								NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
								goto ret;
						}

						Ssid.Ssid[jj++] = (UCHAR)(temp[ii]*16+temp[ii+1]);
				}
				Ssid.Ssid[jj] = '\0';
				Ssid.SsidLength = jj;

ret:

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s=>SSID:",__FUNCTION__));
				for(ii=0; ii<jj; ii++)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%x",(UCHAR)Ssid.Ssid[ii]));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
		}
#endif /* AIRPLAY_SUPPORT */
		
#ifndef APCLI_CONNECTION_TRIAL
		if (Ssid.SsidLength == 0)
			ApSiteSurvey(pAd, &Ssid, SCAN_PASSIVE, FALSE);
		else
			ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE);
#else
		/*for shorter scan time. use active scan and send probe req.*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! Fast Scan for connection trial !!!\n"));
		ApSiteSurvey(pAd, &Ssid, FAST_SCAN_ACTIVE, FALSE);
#endif /* APCLI_CONNECTION_TRIAL */

		return TRUE;
	}
#endif /* AP_SCAN_SUPPORT */
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		Ssid.SsidLength = 0;
		if ((arg != NULL) &&
			(strlen(arg) <= MAX_LEN_OF_SSID))
		{
			RTMPMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
		}

		pAd->StaCfg.bSkipAutoScanConn = TRUE;
		StaSiteSurvey(pAd, &Ssid, SCAN_ACTIVE);
	}
#endif // CONFIG_STA_SUPPORT //

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_SiteSurvey_Proc\n"));

    return TRUE;
}

INT	Set_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ANT_DIVERSITY_TYPE UsedAnt;
	int i;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==> Set_Antenna_Proc *******************\n"));

	for (i = 0; i < strlen(arg); i++)
		if (!isdigit(arg[i]))
			return -EINVAL;

	UsedAnt = simple_strtol(arg, 0, 10);

	switch (UsedAnt)
	{
		/* 2: Fix in the PHY Antenna CON1*/
		case ANT_FIX_ANT0:
			AsicSetRxAnt(pAd, 0);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<== Set_Antenna_Proc(Fix in Ant CON1), (%d,%d)\n",
					pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
			break;
    	/* 3: Fix in the PHY Antenna CON2*/
		case ANT_FIX_ANT1:
			AsicSetRxAnt(pAd, 1);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<== %s(Fix in Ant CON2), (%d,%d)\n",
							__FUNCTION__, pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("<== %s(N/A cmd: %d), (%d,%d)\n", __FUNCTION__, UsedAnt,
					pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
			break;
	}

	return TRUE;
}





#ifdef HW_TX_RATE_LOOKUP_SUPPORT
INT Set_HwTxRateLookUp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable;
	UINT32 MacReg;

	Enable = simple_strtol(arg, 0, 10);

	RTMP_IO_READ32(pAd, TX_FBK_LIMIT, &MacReg);
	if (Enable)
	{
		MacReg |= 0x00040000;
		pAd->bUseHwTxLURate = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (ON)\n"));
	}
	else
	{
		MacReg &= (~0x00040000);
		pAd->bUseHwTxLURate = FALSE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (OFF)\n"));
	}
	RTMP_IO_WRITE32(pAd, TX_FBK_LIMIT, MacReg);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseHwTxLURate = %d \n", pAd->bUseHwTxLURate));

	return TRUE;
}
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable = simple_strtol(arg, 0, 10);

	pAd->bUseMultiMacAddrExt = (Enable ? TRUE : FALSE);
	AsicSetMacAddrExt(pAd, pAd->bUseMultiMacAddrExt);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseMultiMacAddrExt = %d, UseMultiMacAddrExt(%s)\n",
				pAd->bUseMultiMacAddrExt, (Enable ? "ON" : "OFF")));

	return TRUE;
}

INT	Set_MultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR tempMAC[6], idx;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	ULONG offset, Addr;
	INT i;

	if(strlen(arg) < 19)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		idx = (UCHAR) simple_strtol((token+1), 0, 10);

		if (idx > 15)
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&tempMAC[i]), 1);
		}

		if(i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
								tempMAC[0], tempMAC[1], tempMAC[2], tempMAC[3], tempMAC[4], tempMAC[5], idx));

		offset = 0x1480 + (HW_WCID_ENTRY_SIZE * idx);
		Addr = tempMAC[0] + (tempMAC[1] << 8) +(tempMAC[2] << 16) +(tempMAC[3] << 24);
		RTMP_IO_WRITE32(pAd, offset, Addr);
		Addr = tempMAC[4] + (tempMAC[5] << 8);
		RTMP_IO_WRITE32(pAd, offset + 4, Addr);

		return TRUE;
	}

	return FALSE;
}
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */
#endif /* MAC_REPEATER_SUPPORT */

INT set_tssi_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 tssi_enable = 0;

	tssi_enable = simple_strtol(arg, 0, 10);

	if (tssi_enable == 1) {
		pAd->chipCap.tssi_enable = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn on TSSI mechanism\n"));
	} else if (tssi_enable == 0) {
		pAd->chipCap.tssi_enable = FALSE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn off TSS mechanism\n"));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal param(%u)\n", tssi_enable));
		return FALSE;
    }
	return TRUE;
}

#ifdef RLT_MAC
#ifdef CONFIG_WIFI_TEST
INT set_pbf_loopback(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 enable = 0;
	UINT32 value;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	enable = simple_strtol(arg, 0, 10);
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &value);

	if (enable == 1) {
		pAd->chipCap.pbf_loopback = TRUE;
		value |= PBF_LOOP_EN;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn on pbf loopback\n"));
	} else if(enable == 0) {
		pAd->chipCap.pbf_loopback = FALSE;
		value &= ~PBF_LOOP_EN;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn off pbf loopback\n"));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal param(%d)\n"));
		return FALSE;
	}

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, value);

	return TRUE;
}

INT set_pbf_rx_drop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 enable = 0;
	UINT32 value;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	enable = simple_strtol(arg, 0, 10);

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		RTMP_IO_READ32(pAd, RLT_PBF_CFG, &value);
#endif

	if (enable == 1) {
		pAd->chipCap.pbf_rx_drop = TRUE;
		value |= RX_DROP_MODE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn on pbf loopback\n"));
	} else if (enable == 0) {
		pAd->chipCap.pbf_rx_drop = FALSE;
		value &= ~RX_DROP_MODE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn off pbf loopback\n"));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal param(%d)\n"));
		return FALSE;
	}

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		RTMP_IO_WRITE32(pAd, RLT_PBF_CFG, value);
#endif

	return TRUE;
}
#endif

INT set_fw_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 fw_debug_param;

	fw_debug_param = simple_strtol(arg, 0, 10);

#ifdef RLT_MAC
	AndesRltFunSet(pAd, LOG_FW_DEBUG_MSG, fw_debug_param);
#endif /* RLT_MAC */

	return TRUE;
}
#endif

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR radio;

	radio = simple_strtol(arg, 0, 10);

#ifdef MT_MAC
	pAd->iwpriv_command = TRUE;
#endif /* MT_MAC */

	if (radio)
	{
		MlmeRadioOn(pAd);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (ON)\n"));
	}
	else
	{
		MlmeRadioOff(pAd);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (OFF)\n"));
	}

#ifdef MT_MAC
	pAd->iwpriv_command = FALSE;
#endif /* MT_MAC */

	return TRUE;
}


#ifdef MT_MAC
INT set_get_fid(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    //TODO: Carter, at present, only can read pkt in Port2(LMAC port)
    volatile UCHAR   q_idx = 0, loop = 0, dw_idx = 0;
    volatile UINT32  head_fid_addr = 0, next_fid_addr = 0, value = 0x00000000L, dw_content;
    q_idx = simple_strtol(arg, 0, 10);

    value = 0x00400000 | (q_idx << 16);//port2. queue by input value.
    RTMP_IO_WRITE32(pAd, 0x8024, value);
    RTMP_IO_READ32(pAd, 0x8024, (UINT32 *)&head_fid_addr);//get head FID.
    head_fid_addr = head_fid_addr & 0xfff;

    if (head_fid_addr == 0xfff) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, q_idx:%d empty!!\n", __func__, q_idx));
        return TRUE;
    }

    value = (0 | (head_fid_addr << 16));
    while (1) {
        for (dw_idx = 0; dw_idx < 8; dw_idx++) {
            RTMP_IO_READ32(pAd, ((MT_PCI_REMAP_ADDR_1 + (((value & 0x0fff0000) >> 16) * 128)) + (dw_idx * 4)), (UINT32 *)&dw_content);//get head FID.
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pkt:%d, fid:%x, dw_idx = %d, dw_content = 0x%x\n", loop, ((value & 0x0fff0000) >> 16), dw_idx, dw_content));
        }
        RTMP_IO_WRITE32(pAd, 0x8028, value);
        RTMP_IO_READ32(pAd, 0x8028, (UINT32 *)&next_fid_addr);//get next FID.
        if ((next_fid_addr & 0xfff) == 0xfff) {
            return TRUE;
        }

        value = (0 | ((next_fid_addr & 0xffff) << 16));
        loop++;
        if (loop > 5) {
            return TRUE;
        }
    }
    return TRUE;
}

#ifdef RTMP_MAC_PCI
INT Set_PDMAWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;

    Dbg = simple_strtol(arg, 0, 10);

	if (Dbg == 1)
	{
		pAd->PDMAWatchDogEn = 1;
	}
	else if (Dbg == 0)
	{
		pAd->PDMAWatchDogEn = 0;
	}
	else if (Dbg == 2)
	{
		PDMAResetAndRecovery(pAd);
	}
	else if (Dbg == 3)
	{
		pAd->PDMAWatchDogDbg = 0;
	}
	else if (Dbg == 4)
	{
		pAd->PDMAWatchDogDbg = 1;
	}

	return TRUE;
}


INT SetPSERegister_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 idx;
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;
	
    	idx = simple_strtol(arg, 0, 10);

	if (idx == 1)
	{

			MAC_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
			RemapBase = GET_REMAP_2_BASE(0x800c006c) << 19;
			RemapOffset = GET_REMAP_2_OFFSET(0x800c006c);
			MAC_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

			MAC_IO_WRITE32(pAd, 0x80000 + RemapOffset, 3);

			MAC_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x800c006c [3]=0x%x\n",Value));
			
	}
			

	return TRUE;
}

INT SetPDMARegister_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 idx;
	UINT32 Value;
    	idx = simple_strtol(arg, 0, 10);

			switch (idx)
			{
				case 0:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x08000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x08000000]=0x%x\n",Value));
					break;
				case 1:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x18000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x18000000]=0x%x\n",Value));
					break;
				case 2:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x28000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x28000000]=0x%x\n",Value));
					break;
				case 3:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x38000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x38000000]=0x%x\n",Value));
					break;
				case 4:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x48000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x48000000]=0x%x\n",Value));
					break;
				case 5:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x58000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x58000000]=0x%x\n",Value));
					break;
				case 6:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x68000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x68000000]=0x%x\n",Value));					
					break;
				case 7:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x78000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x78000000]=0x%x\n",Value));										
					break;
				case 8:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x88000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x88000000]=0x%x\n",Value));
					break;
				case 9:/* improvement: use round robin mcs when group == 0 */
					MAC_IO_WRITE32(pAd, 0x4244, 0x98000000);
					MAC_IO_READ32(pAd, 0x4244, &Value);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mac 4244[0x98000000]=0x%x\n",Value));
					break;				
				default:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wrong idx\n"));
					break;
			}
			
	return TRUE;
}

INT SetPSEWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;

    Dbg = simple_strtol(arg, 0, 10);

	if (Dbg == 1)
	{
		pAd->PSEWatchDogEn = 1;
	}
	else if (Dbg == 0)
	{
		pAd->PSEWatchDogEn = 0;
	}
	else if (Dbg == 2)
	{
		PSEResetAndRecovery(pAd);
	}
	else if (Dbg == 3)
	{
		DumpPseInfo(pAd);
	}
#ifdef DMA_RESET_SUPPORT	
	else if (Dbg == 4)
	{
		pAd->bcn_reset_en=TRUE;
	}
	else if (Dbg == 5)
	{
		pAd->bcn_reset_en=FALSE;
	}	
#endif /* DMA_RESET_SUPPORT */
	return TRUE;
}
#endif
INT SetPsmWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 val = 0;
	val = simple_strtol(arg, 0, 10);

	pAd->PsmWatchDogDisabled = val;

	return TRUE;
}

INT SetPsmDbg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
        UINT32 wcid = 0, Value = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
        STA_TR_ENTRY *tr_entry = NULL;
	struct wtbl_entry tb_entry;
        union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;

    	wcid = simple_strtol(arg, 0, 10);
	if (wcid < MAX_LEN_OF_MAC_TABLE)
	{ 
		pEntry = &pAd->MacTab.Content[wcid];
        	tr_entry = &pAd->MacTab.tr_entry[wcid];
	
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
                {
			 NdisZeroMemory(&tb_entry, sizeof(tb_entry));
                         if (mt_wtbl_get_entry234(pAd, wcid, &tb_entry) == FALSE)
                         {
                         	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4\n",__FUNCTION__));
                         	goto err;
                         }
                         RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0]+12, &dw3->word);

                         MAC_IO_READ32(pAd, WTBL1OR, &Value);
                         Value |= PSM_W_FLAG;
                         MAC_IO_WRITE32(pAd, WTBL1OR, Value);

                         dw3->field.psm = 0;
                         HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 12, dw3->word);

                         pEntry->PsMode = 0;
                         tr_entry->PsMode = 0;

                         MAC_IO_READ32(pAd, WTBL1OR, &Value);
                         Value &= ~PSM_W_FLAG;
                         MAC_IO_WRITE32(pAd, WTBL1OR, Value);
                         pEntry->PsmResetCount++;

			 NdisZeroMemory(&tb_entry, sizeof(tb_entry));
                         if (mt_wtbl_get_entry234(pAd, wcid, &tb_entry) == FALSE)
                         {
                         	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4\n",__FUNCTION__));
                                goto err;
                         }

                         RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0]+12, &dw3->word);
                         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wcid %d on PSM RES ==> %d\n", wcid, dw3->field.psm));
		}
		else
			goto err;	

	}
	else
		goto err;

        return TRUE;
err:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: can't set the psm dbg\n", __FUNCTION__));

	return TRUE;
}

#ifdef TXRXCR_DEBUG_SUPPORT
INT SetTxRxCr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;
	UINT32 mac_val;
	UINT32 idx;
    	Dbg = simple_strtol(arg, 0, 10);

	if (Dbg == 0)
	{
		RTMP_IO_WRITE32(pAd, 0x14108, 0x80);
		RTMP_IO_WRITE32(pAd, 0x14108, 0xc0);
		RTMP_IO_WRITE32(pAd, 0x14108, 0x80);

		RTMP_IO_READ32(pAd, 0x2c030, &mac_val);
		printk("0x60330030=0x%x\n",mac_val);
		RTMP_IO_READ32(pAd, 0x2c01c, &mac_val);
		printk("0x6033001C=0x%x\n",mac_val);
	}
	else if (Dbg == 1)
	{
		printk("=============================================\n");
		printk("RX Status Counter\n");	
		RTMP_IO_READ32(pAd, 0x1410c, &mac_val);
		printk("0x6020410C=0x%x\n",mac_val);	

		RTMP_IO_READ32(pAd, 0x14110, &mac_val);
		printk("0x60204110=0x%x\n",mac_val);

		RTMP_IO_READ32(pAd, 0x14114, &mac_val);
		printk("0x60204114=0x%x\n",mac_val);

		RTMP_IO_READ32(pAd, 0x1411c, &mac_val);
		printk("0x6020411C=0x%x\n",mac_val);

		RTMP_IO_READ32(pAd, 0x14120, &mac_val);
		printk("0x60204120=0x%x\n",mac_val);	


		RTMP_IO_READ32(pAd, 0x2c030, &mac_val);
		printk("0x60330030=0x%x\n",mac_val);
		RTMP_IO_READ32(pAd, 0x2c01c, &mac_val);
		printk("0x6033001C=0x%x\n",mac_val);

		
		printk("=============================================\n");
		printk("CCA Status\n");	
		RTMP_IO_WRITE32(pAd, 0x20014, 0x1F);
		RTMP_IO_WRITE32(pAd, 0x20018, 0x07070707);
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x02021919);

		RTMP_IO_READ32(pAd, 0x20024, &mac_val);
		printk("0x60000024=0x%x\n",mac_val);	
		printk("=============================================\n");


		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x21518, &mac_val);
			printk("0x60120118[%d]=0x%08x    ",idx, mac_val);	
			if (idx==4)
				printk("\n");	
		}
		printk("\n");	
		
		RTMP_IO_WRITE32(pAd, 0x20014, 0x1f);
		RTMP_IO_WRITE32(pAd, 0x20014, 0x07070707);

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x00000101);
		printk("write 0x60000024=0x00000101\n");	
		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			printk("0x60000024[%d]=0x%08x    ",idx, mac_val);	
			if (idx==4)
				printk("\n");	
		}
		printk("\n");	
		
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x02021919);
		printk("write 0x60000024=0x02021919\n");	
		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			printk("0x60000024[%d]=0x%08x    ",idx, mac_val);	
			if (idx==4)
				printk("\n");	
		}	
		printk("\n");	
		
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x10101212);
		printk("write 0x60000024=0x10101212\n");	
		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			printk("0x60000024[%d]=0x%08x    ",idx, mac_val);	
			if (idx==4)
				printk("\n");
		}
		printk("\n");	
		
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x00000B0B);
		printk("write 0x60000024=0x00000B0B\n");	
		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			printk("0x60000024[%d]=0x%08x    ",idx, mac_val);	
			if (idx==4)
				printk("\n");	
		}	
		printk("\n");	
		printk("=============================================\n");

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x13131414);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x13131414]=0x%08x    ", mac_val);	
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x1A1A1B1B);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x1A1A1B1B]=0x%08x    ", mac_val);		
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x16161D1D);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x16161D1D]=0x%08x    ", mac_val);	
		printk("\n");	

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x26262727);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x26262727]=0x%08x    ", mac_val);			
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x28282929);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x28282929]=0x%08x    ", mac_val);	
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x2A2A2B2B);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x2A2A2B2B]=0x%08x    ", mac_val);	
		printk("\n");	
		printk("=============================================\n");

		RTMP_IO_READ32(pAd, 0x21710, &mac_val);	
		printk("0x60130110=0x%08x    ", mac_val);	

		RTMP_IO_READ32(pAd, 0x21714, &mac_val);	
		printk("0x60130114=0x%08x    ", mac_val);		

		RTMP_IO_READ32(pAd, 0x21718, &mac_val);	
		printk("0x60130118=0x%08x    ", mac_val);
		printk("\n");

		RTMP_IO_READ32(pAd, 0x2171c, &mac_val);	
		printk("0x60130118=0x%08x    ", mac_val);

		RTMP_IO_READ32(pAd, 0x21720, &mac_val);	
		printk("0x60130120=0x%08x    ", mac_val);	

		RTMP_IO_READ32(pAd, 0x21724, &mac_val);	
		printk("0x60130124=0x%08x    ", mac_val);
		printk("\n");
		RTMP_IO_READ32(pAd, 0x21728, &mac_val);	
		printk("0x60130128=0x%08x    ", mac_val);
		printk("\n");	
		printk("=============================================\n");

		RTMP_IO_WRITE32(pAd, 0x20014, 0x1f);
		RTMP_IO_WRITE32(pAd, 0x20018, 0x05050505);

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x85858686);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x85858686]=0x%08x\n", mac_val);	

		RTMP_IO_WRITE32(pAd, 0x20018, 0x05050404);
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x81812A2A);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	
		printk("0x60000024[0x81812A2A]=0x%08x\n", mac_val);
		printk("\n\n\n\n\n\n\n\n\n\n");	
		printk("=============================================\n");				
	}
	
	return TRUE;
}
#endif /* TXRXCR_DEBUG_SUPPORT */

INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 LogType;
    LogType = simple_strtol(arg, 0, 10);

	if (LogType < 3)
		CmdFwLog2Host(pAd, LogType);
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: Unknown Log Type = %d\n", __FUNCTION__, LogType));

	return TRUE;
}

#endif

#ifdef THERMAL_PROTECT_SUPPORT
INT set_thermal_protection_criteria_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	UINT8 HighEn;
	CHAR HighTempTh;
	UINT8 LowEn;
	CHAR LowTempTh;
	CHAR *Pos = NULL;

	Pos = arg;

        Pos = rtstrstr(Pos, "h_en-");
	if (Pos != NULL) {
		Pos = Pos + 5;
		HighEn = simple_strtol(Pos, 0, 10);
	}
	else
		goto error;

	Pos = rtstrstr(Pos, "h_th-");
	if (Pos != NULL) {
		Pos = Pos + 5;
		HighTempTh = simple_strtol(Pos, 0, 10);
	}
	else
		goto error;

	Pos = rtstrstr(Pos, "l_en-");
	if (Pos != NULL) {
		Pos = Pos + 5;
		LowEn = simple_strtol(Pos, 0, 10);
	}
	else
		goto error;

	Pos = rtstrstr(Pos, "l_th-");
	if (Pos != NULL) {
		Pos = Pos + 5;
		LowTempTh = simple_strtol(Pos, 0, 10);
	}
	else
		goto error;

#ifdef MT_MAC
	CmdThermalProtect(pAd, HighEn, HighTempTh, LowEn, LowTempTh);
#endif /* MT_MAC */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: high_en=%d, high_thd = %d, low_en = %d, low_thd = %d\n", __FUNCTION__, HighEn, HighTempTh, LowEn, LowTempTh));

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("iwpriv ra0 set tpc=h_en-\"value\"_h_th-\"value\"_l_en-\"value\"_l_th-\"value\"\n"));
	return TRUE;
}
#endif /* THERMAL_PROTECT_SUPPORT */

#define DELTA_ELEMENT   21
char *obtw_delta_str[]=
    {
        "cck1m_", "cck5m_",
        "ofdm6m_", "ofdm12m_", "ofdm24m_", "ofdm48m_", "ofdm54m_",
        "ht20mcs0_", "ht20mcs32_", "ht20mcs1_", "ht20mcs3_",
        "ht20mcs5_", "ht20mcs6_", "ht20mcs7_",
        "ht40mcs0_", "ht40mcs32_", "ht40mcs1_", "ht40mcs3_",
        "ht40mcs5_", "ht40mcs6_", "ht40mcs7_",
    };

INT set_obtw_delta_proc(
    IN PRTMP_ADAPTER    pAd,
    IN RTMP_STRING      *arg)
{
    UINT8 otbw_delta_array[DELTA_ELEMENT] = {0};
    CHAR *Pos = NULL;
    UINT8 DeltaVal = 0;
    int i;
    BOOLEAN anyEnable = FALSE;

    if (arg && strlen(arg)) {
        Pos = strstr(arg, "disable");
        if (Pos != NULL) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: disable\n", __FUNCTION__));
            goto sendCmd;
        }
    }

    for (i = 0; i < DELTA_ELEMENT; i++)
    {
        if (arg && strlen(arg))
            Pos = strstr(arg, obtw_delta_str[i]);
        else
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: input arguments error\n", __FUNCTION__));

        if (Pos != NULL) {
            Pos = Pos + strlen(obtw_delta_str[i]);
            DeltaVal = simple_strtol(Pos, 0, 10);
            otbw_delta_array[i] = DeltaVal;
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("%s: found %s, DeltaVal = %d\n", __FUNCTION__, obtw_delta_str[i], DeltaVal));
            if (DeltaVal > 0)
                anyEnable = TRUE;
        }
    }

sendCmd:
#ifdef MT_MAC
    CmdObtwDelta(pAd, anyEnable, otbw_delta_array);
#endif /* MT_MAC */

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: anyEnable=%d\n", __FUNCTION__, anyEnable));

    return TRUE;
}

#ifdef CONFIG_DVT_MODE
#define STOP_DP_OUT	0x50029080
INT16 i2SetDvt(RTMP_ADAPTER *pAd, RTMP_STRING *pArg)
{
	/*
	test item=0: normal mode
	test item=1: test tx endpint, param1=
	*/
	INT16	i2ParameterNumber = 0;
	UCHAR	ucTestItem = 0;
	UCHAR	ucTestParam1 = 0;
	UCHAR	ucTestParam2 = 0;
	UINT32	u4Value;

	if (pArg)
	{
		i2ParameterNumber = sscanf(pArg, "%d,%d,%d", &(ucTestItem), &(ucTestParam1), &(ucTestParam2));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: i2ParameterNumber(%d), ucTestItem(%d), ucTestParam1(%d), ucTestParam2(%d)\n", __FUNCTION__, i2ParameterNumber, ucTestItem, ucTestParam1, ucTestParam2));
		pAd->rDvtCtrl.ucTestItem = ucTestItem;
		pAd->rDvtCtrl.ucTestParam1 = ucTestParam1;
		pAd->rDvtCtrl.ucTestParam2 = ucTestParam2;

		/* Tx Queue Mode*/
		if (pAd->rDvtCtrl.ucTestItem == 1 )
		{
			pAd->rDvtCtrl.ucTxQMode = pAd->rDvtCtrl.ucTestItem;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxQMode(%d)\n", __FUNCTION__, pAd->rDvtCtrl.ucTxQMode));
		}
		else if (pAd->rDvtCtrl.ucTestItem == 2)
		{
			pAd->rDvtCtrl.ucTxQMode = pAd->rDvtCtrl.ucTestItem;
			pAd->rDvtCtrl.ucQueIdx = pAd->rDvtCtrl.ucTestParam1;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxQMode(%d), ucQueIdx(%d)\n", __FUNCTION__, pAd->rDvtCtrl.ucTxQMode, pAd->rDvtCtrl.ucQueIdx));
		}
		/* UDMA Drop CR Access */
		else if (pAd->rDvtCtrl.ucTestItem == 3 && pAd->rDvtCtrl.ucTestParam1 == 0)
		{
			RTMP_IO_READ32(pAd, STOP_DP_OUT, &u4Value);
			u4Value &= ~(BIT25 | BIT24 | BIT23 | BIT22 | BIT21 | BIT20);
			RTMP_IO_WRITE32(pAd, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Drop=0\n", __FUNCTION__));
		}
		else if (pAd->rDvtCtrl.ucTestItem == 3 && pAd->rDvtCtrl.ucTestParam1 == 1)
		{

			RTMP_IO_READ32(pAd, STOP_DP_OUT, &u4Value);
			u4Value |= (BIT25 | BIT24 | BIT23 | BIT22 | BIT21 | BIT20);
			RTMP_IO_WRITE32(pAd, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Drop=1\n", __FUNCTION__));
		}
		else if (pAd->rDvtCtrl.ucTestItem == 4 && pAd->rDvtCtrl.ucTestParam1 == 0)
		{
			RTMP_IO_READ32(pAd, STOP_DP_OUT, &u4Value);
			u4Value &= ~(BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4);
			RTMP_IO_WRITE32(pAd, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Stop=0\n", __FUNCTION__));
		}
		else if (pAd->rDvtCtrl.ucTestItem == 4 && pAd->rDvtCtrl.ucTestParam1 == 1)
		{

			RTMP_IO_READ32(pAd, STOP_DP_OUT, &u4Value);
			u4Value |= (BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4);
			RTMP_IO_WRITE32(pAd, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Stop=1\n", __FUNCTION__));
		}
		/* ACQTxNumber */
		else if ((pAd->rDvtCtrl.ucTestItem == 5) && (pAd->rDvtCtrl.ucTestParam1 == 0))
		{
			UCHAR ucIdx = 0;
			for (ucIdx = 0; ucIdx < 5; ucIdx++)
			{
				pAd->rDvtCtrl.au4ACQTxNum[ucIdx] = 0;
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Reset au4ACQTxNum EP4_AC0(%d), EP5_AC1(%d), EP6_AC2(%d), EP7_AC3(%d), EP9_AC0(%d)\n",
									__FUNCTION__,
									pAd->rDvtCtrl.au4ACQTxNum[0],
									pAd->rDvtCtrl.au4ACQTxNum[1],
									pAd->rDvtCtrl.au4ACQTxNum[2],
									pAd->rDvtCtrl.au4ACQTxNum[3],
									pAd->rDvtCtrl.au4ACQTxNum[4]));
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: i2ParameterNumber(%d), ucTestItem(%d), parameters error\n", __FUNCTION__, i2ParameterNumber, ucTestItem));
		}
	}

	return TRUE;
}
#endif /* CONFIG_DVT_MODE */

#ifdef MT_MAC
VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData)
{
	extern UCHAR tmi_rate_map_ofdm[];
	extern UCHAR tmi_rate_map_cck_lp[];
	extern UCHAR tmi_rate_map_cck_sp[];
	UCHAR phy_mode, rate, preamble;
	CHAR *phyMode[5] = {"CCK", "OFDM", "MM", "GF", "VHT"};

	phy_mode = RawData>>13;
	rate = RawData & 0x3F;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	if ( TxRx == 0 )
		sprintf(Output+strlen(Output), "Last TX Rate                    = ");
	else
		sprintf(Output+strlen(Output), "Last RX Rate                    = ");

	if ( phy_mode == MODE_CCK ) {

		if ( TxRx == 0 )
		{
			if (preamble)
				rate = tmi_rate_map_cck_lp[rate];
			else
				rate = tmi_rate_map_cck_sp[rate];
		}

		if ( rate == TMI_TX_RATE_CCK_1M_LP )
			sprintf(Output+strlen(Output), "1M, ");
		else if ( rate == TMI_TX_RATE_CCK_2M_LP )
			sprintf(Output+strlen(Output), "2M, ");
		else if ( rate == TMI_TX_RATE_CCK_5M_LP )
			sprintf(Output+strlen(Output), "5M, ");
		else if ( rate == TMI_TX_RATE_CCK_11M_LP )
			sprintf(Output+strlen(Output), "11M, ");
		else if ( rate == TMI_TX_RATE_CCK_2M_SP )
			sprintf(Output+strlen(Output), "2M, ");
		else if ( rate == TMI_TX_RATE_CCK_5M_SP )
			sprintf(Output+strlen(Output), "5M, ");
		else if ( rate == TMI_TX_RATE_CCK_11M_SP )
			sprintf(Output+strlen(Output), "11M, ");
		else
			sprintf(Output+strlen(Output), "unkonw, ");

	} else if ( phy_mode == MODE_OFDM ) {

		if ( TxRx == 0 )
		{
			rate = tmi_rate_map_ofdm[rate];
		}

		if ( rate == TMI_TX_RATE_OFDM_6M )
			sprintf(Output+strlen(Output), "6M, ");
		else if ( rate == TMI_TX_RATE_OFDM_9M )
			sprintf(Output+strlen(Output), "9M, ");
		else if ( rate == TMI_TX_RATE_OFDM_12M )
			sprintf(Output+strlen(Output), "12M, ");
		else if ( rate == TMI_TX_RATE_OFDM_18M )
			sprintf(Output+strlen(Output), "18M, ");
		else if ( rate == TMI_TX_RATE_OFDM_24M )
			sprintf(Output+strlen(Output), "24M, ");
		else if ( rate == TMI_TX_RATE_OFDM_36M )
			sprintf(Output+strlen(Output), "36M, ");
		else if ( rate == TMI_TX_RATE_OFDM_48M )
			sprintf(Output+strlen(Output), "48M, ");
		else if ( rate == TMI_TX_RATE_OFDM_54M )
			sprintf(Output+strlen(Output), "54M, ");
		else
			sprintf(Output+strlen(Output), "unkonw, ");
	} else {
			sprintf(Output+strlen(Output), "MCS%d, ", rate);
	}
	sprintf(Output+strlen(Output), "%2dM, ", ((RawData>>7) & 0x1)? 40: 20);
	sprintf(Output+strlen(Output), "%cGI, ", ((RawData>>9) & 0x1)? 'S': 'L');
	sprintf(Output+strlen(Output), "%s%s\n", phyMode[(phy_mode) & 0x3], ((RawData>>10) & 0x3)? ", STBC": " ");

}

INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* 0: get temperature; 1: get adc */
	UINT32 value;	
	UINT32 temperature=0; 
	value = simple_strtol(arg, 0, 10);

	if ((value == 0) || (value == 1)) {
#if defined(MT7603) || defined(MT7628)
		temperature = MtAsicGetThemalSensor(pAd, value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ThemalSensor = 0x%x\n", __FUNCTION__, temperature));
#else
		CmdGetThermalSensorResult(pAd, value,&temperature);
#endif /* MT7603 ||MT7628  */
	} else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: 0: get temperature; 1: get adc\n", __FUNCTION__));

	return TRUE;
}


INT Set_rx_pspoll_filter_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;	
	value = simple_strtol(arg, 0, 10);

	pAd->rx_pspoll_filter = value;
	MtAsicSetRxPspollFilter(pAd, pAd->rx_pspoll_filter);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: rx_pspoll_filter=%d\n", __FUNCTION__, pAd->rx_pspoll_filter));

	return TRUE;
}

#ifdef SINGLE_SKU_V2
INT SetSKUEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR value;
	
	value = simple_strtol(arg, 0, 10);
	
	if (value)
	{
		pAd->SKUEn = 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s (ON)\n", __FUNCTION__));
	}
	else
	{
		pAd->SKUEn = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s (OFF)\n", __FUNCTION__));
	}

	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	
	return TRUE;
}
#endif /* SINGLE_SKU_V2 */

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT SetSCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;
	
	value = simple_strtol(arg, 0, 10);
	
	if (value > 500)
	{
		pAd->SCSCtrl.SCSThreshold= value;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s (Traffice Threshold=%d)\n", __FUNCTION__, value));
	}
	else
	{
		if (value == 1)
		{
			pAd->SCSCtrl.SCSEnable= SCS_ENABLE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s (ON)\n", __FUNCTION__));
		}
		else if (value == 0)
		{
			pAd->SCSCtrl.SCSEnable= SCS_DISABLE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s (OFF)\n", __FUNCTION__));
			/* Restore to default */
			RTMP_IO_WRITE32(pAd, CR_AGC_0, pAd->SCSCtrl.CR_AGC_0_default);
			RTMP_IO_WRITE32(pAd, CR_AGC_0_RX1, pAd->SCSCtrl.CR_AGC_0_default);
			RTMP_IO_WRITE32(pAd, CR_AGC_3, pAd->SCSCtrl.CR_AGC_3_default);
			RTMP_IO_WRITE32(pAd, CR_AGC_3_RX1, pAd->SCSCtrl.CR_AGC_3_default);
			pAd->SCSCtrl.SCSStatus = SCS_STATUS_DEFAULT;
		}
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==>%s (Unknow value = %d)\n", __FUNCTION__, value));
	}
	
	return TRUE;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#endif /* MT_MAC */

#ifdef ED_MONITOR
/* run-time turn EDCCA on/off */
INT Set_ed_chk_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT ed_chk = simple_strtol(arg, 0, 10);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()::ed_chk=%d\n", 
		__FUNCTION__, ed_chk));

	pAd->ed_chk = ed_chk;
	if (ed_chk != 0)
		RTMP_CHIP_ASIC_SET_EDCCA(pAd, TRUE);
	else
		RTMP_CHIP_ASIC_SET_EDCCA(pAd, FALSE);
	
	return TRUE;
}

INT ed_status_read(RTMP_ADAPTER *pAd)
{
	UINT32 period_us = pAd->ed_chk_period * 1000;
	ULONG irqflag=0;
	INT percent;

	UINT32 ch_idle_stat=0, ch_busy_stat=0, ed_stat=0, rssi_stat=0;
	INT32 WBRssi0, WBRssi1, RefWBRssi;
	
	RTMP_IO_READ32(pAd, 0x2c050, &ed_stat);

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	
	pAd->ch_idle_stat[pAd->ed_stat_lidx] = ch_idle_stat;
	pAd->ch_busy_stat[pAd->ed_stat_lidx] = ch_busy_stat;	
	pAd->ed_stat[pAd->ed_stat_lidx] = ed_stat;
	NdisGetSystemUpTime(&pAd->chk_time[pAd->ed_stat_lidx]);

	RTMP_IO_READ32(pAd,RO_AGC_DEBUG_2,&rssi_stat);
	
	//IBRssi0 = (rssi_stat&0xFF000000)>>24;
	WBRssi0 = (rssi_stat&0x00FF0000)>>16;
	//IBRssi1 = (rssi_stat&0x0000FF00)>>8;
	WBRssi1 = (rssi_stat&0x000000FF);

	if(WBRssi0>=128)
		WBRssi0-=256;	

	if(WBRssi1>=128)
		WBRssi1-=256;

	RefWBRssi = max(WBRssi0,WBRssi1);
	pAd->rssi_stat[pAd->ed_stat_lidx] = RefWBRssi;
	if(RefWBRssi >= -40 && pAd->ed_big_rssi_count < pAd->ed_block_tx_threshold)
		pAd->ed_big_rssi_count++;
	else if(pAd->ed_big_rssi_count > 0)
		pAd->ed_big_rssi_count--;
	
	pAd->ed_big_rssi_stat[pAd->ed_stat_lidx] = pAd->ed_big_rssi_count * 100 / pAd->ed_block_tx_threshold;
	
	if ((pAd->ed_threshold > 0) && (period_us > 0) && (pAd->ed_block_tx_threshold > 0)) {
		percent = (pAd->ed_stat[pAd->ed_stat_lidx] * 100 ) / period_us;
		if (percent > 100)
			percent = 100;

		if (percent > pAd->ed_threshold) 
		{
			pAd->ed_trigger_cnt++;
			pAd->ed_silent_cnt = 0;
		}
		else		
		{
			pAd->ed_trigger_cnt = 0;
			pAd->ed_silent_cnt++;
		}
				
		if(pAd->ed_debug)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("@@@ %s: pAd->ed_trigger_cnt=%d,	pAd->ed_silent_cnt=%u, percent=%d !!\n", 
				__FUNCTION__, pAd->ed_trigger_cnt, pAd->ed_silent_cnt
			,  percent));
		}
	}
	pAd->ed_trigger_stat[pAd->ed_stat_lidx] = pAd->ed_trigger_cnt;
	pAd->ed_silent_stat[pAd->ed_stat_lidx] = pAd->ed_silent_cnt;

	
	INC_RING_INDEX(pAd->ed_stat_lidx, ED_STAT_CNT);
	//pAd->false_cca_stat[pAd->ed_stat_lidx] = 0;
	if (pAd->ed_stat_sidx == pAd->ed_stat_lidx) {
		INC_RING_INDEX(pAd->ed_stat_sidx, ED_STAT_CNT);
	}	
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

	if(pAd->ed_chk != FALSE)
	{
		UINT32 macVal = 0, macVal2 = 0;

		RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0634, &macVal2);
		if (pAd->ed_trigger_cnt > pAd->ed_block_tx_threshold || pAd->ed_big_rssi_stat[pAd->ed_stat_lidx] < 50) {
			//EDCCA CERT
			if(pAd->ed_threshold_strict != TRUE)
			{
				pAd->ed_threshold_strict = TRUE;
				macVal = 0xD7C87D10;  		
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

				macVal2 &= 0xFFFFFFFE;
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\n====\n@@@ %s: EDCCA TH - H\npAd->ed_trigger_cnt : %d > %d ||  pAd->ed_big_rssi_stat : %d < 50\n====\n"
				,__FUNCTION__,pAd->ed_trigger_cnt,pAd->ed_block_tx_threshold,pAd->ed_big_rssi_stat[pAd->ed_stat_lidx]));
			}
		}

		if (pAd->ed_silent_cnt > pAd->ed_block_tx_threshold && pAd->ed_big_rssi_stat[pAd->ed_stat_lidx] >= 50) {
			//EDCCA USER
			if(pAd->ed_threshold_strict != FALSE)
			{
				pAd->ed_threshold_strict = FALSE;
				macVal = 0xD7C87D0F;  
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

				macVal2 |= 0x1;
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\n====\n@@@ %s: EDCCA TH - L\npAd->ed_silent_cnt : %d > %d && pAd->ed_big_rssi_stat : %d >= 50\n====\n"
				,__FUNCTION__,pAd->ed_silent_cnt,pAd->ed_block_tx_threshold,pAd->ed_big_rssi_stat[pAd->ed_stat_lidx]));
			}
		}
	}


	return TRUE;
}

//this function will be called in multi entry
INT ed_monitor_exit(RTMP_ADAPTER *pAd)
{
	ULONG irqflag=0;

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("@@@ %s : ===>\n", __FUNCTION__));
	
#ifdef ED_SMART
	RTMPCancelTimer(&pAd->ed_timer, &cancelled);
#endif //ED_SMART

	NdisZeroMemory(&pAd->ed_stat[0], sizeof(pAd->ed_stat));
	NdisZeroMemory(&pAd->ch_idle_stat[0], sizeof(pAd->ch_idle_stat));
	NdisZeroMemory(&pAd->ch_busy_stat[0], sizeof(pAd->ch_busy_stat));
	NdisZeroMemory(&pAd->chk_time[0], sizeof(pAd->chk_time));
	NdisZeroMemory(&pAd->ed_trigger_stat[0], sizeof(pAd->ed_trigger_stat));
	NdisZeroMemory(&pAd->ed_silent_stat[0], sizeof(pAd->ed_silent_stat));
	NdisZeroMemory(&pAd->rssi_stat[0], sizeof(pAd->rssi_stat));
	
	pAd->ed_stat_lidx = pAd->ed_stat_sidx = 0;
	pAd->ed_trigger_cnt = 0;
	pAd->ed_silent_cnt = 0;
	//ignore fisrt time's incorrect false cca 
	pAd->ed_false_cca_cnt = 0;

	pAd->ed_tx_stoped = FALSE;
	pAd->ed_threshold_strict = FALSE;
	//also clear top level flags
	pAd->ed_chk = FALSE;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("@@@ %s : <===\n", __FUNCTION__));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

	RTMP_CHIP_ASIC_SET_EDCCA(pAd,FALSE);
	
	return TRUE;
}


// open & muanl cmd will call
INT ed_monitor_init(RTMP_ADAPTER *pAd)
{
	ULONG irqflag=0;

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("@@@ %s : ===>\n", __FUNCTION__));
	NdisZeroMemory(&pAd->ed_stat[0], sizeof(pAd->ed_stat));
	NdisZeroMemory(&pAd->ch_idle_stat[0], sizeof(pAd->ch_idle_stat));
	NdisZeroMemory(&pAd->ch_busy_stat[0], sizeof(pAd->ch_busy_stat));
	NdisZeroMemory(&pAd->chk_time[0], sizeof(pAd->chk_time));
	NdisZeroMemory(&pAd->ed_trigger_stat[0], sizeof(pAd->ed_trigger_stat));
	NdisZeroMemory(&pAd->ed_silent_stat[0], sizeof(pAd->ed_silent_stat));
	NdisZeroMemory(&pAd->rssi_stat[0], sizeof(pAd->rssi_stat));
	
	pAd->ed_stat_lidx = pAd->ed_stat_sidx = 0;
	pAd->ed_trigger_cnt = 0;
	pAd->ed_silent_cnt = 0;

	// ignore fisrt time's incorrect false cca
	pAd->ed_false_cca_cnt = 0;
	
	pAd->ed_tx_stoped = FALSE;
	pAd->ed_threshold_strict = FALSE;
	
	//also set  top level flags
	if(pAd->ed_chk == 0)	//turn on if disabled , otherwise decide by profile
		pAd->ed_chk = 1; 

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("@@@ %s : <===\n", __FUNCTION__));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

	RTMP_CHIP_ASIC_SET_EDCCA(pAd,TRUE);
	return TRUE;
}

#ifdef DBG
INT show_ed_stat_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	unsigned long irqflags=0;
	UINT32 ed_stat[ED_STAT_CNT], ed_big_rssi_stat[ED_STAT_CNT], rssi_stat[ED_STAT_CNT];
	UINT32 silent_stat[ED_STAT_CNT], trigger_stat[ED_STAT_CNT]; 
	UINT32 busy_stat[ED_STAT_CNT], idle_stat[ED_STAT_CNT];
	ULONG chk_time[ED_STAT_CNT];
	INT period_us;
	UCHAR start, end, idx;
		
	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflags);
	start = pAd->ed_stat_sidx;
	end = pAd->ed_stat_lidx;
	NdisMoveMemory(&ed_stat[0], &pAd->ed_stat[0], sizeof(ed_stat));
	NdisMoveMemory(&ed_big_rssi_stat[0], &pAd->ed_big_rssi_stat[0], sizeof(ed_big_rssi_stat));
	NdisMoveMemory(&busy_stat[0], &pAd->ch_busy_stat[0], sizeof(busy_stat));
	NdisMoveMemory(&idle_stat[0], &pAd->ch_idle_stat[0], sizeof(idle_stat));
	NdisMoveMemory(&chk_time[0], &pAd->chk_time[0], sizeof(chk_time));
	NdisMoveMemory(&trigger_stat[0], &pAd->ed_trigger_stat[0], sizeof(
trigger_stat));
	NdisMoveMemory(&silent_stat[0], &pAd->ed_silent_stat[0], sizeof(silent_stat));
	NdisMoveMemory(&rssi_stat[0], &pAd->rssi_stat[0], sizeof(
rssi_stat));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflags);

#ifdef CONFIG_AP_SUPPORT
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump ChannelBusy Counts, ed_chk=%u, false_cca_threshold=%u, ChkPeriod=%dms, ED_Threshold=%d%%, HitCntForBlockTx=%d\n", 
	pAd->ed_chk, pAd->ed_false_cca_threshold, pAd->ed_chk_period, pAd->ed_threshold, pAd->ed_block_tx_threshold));
#endif

	period_us = pAd->ed_chk_period * 1000;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TimeSlot:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%ld  ", chk_time[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump ED_STAT\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RawCnt:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d  ", ed_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Percent:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", (ed_stat[idx] * 100) / period_us));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RSSI:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", rssi_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TriggerCnt:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", trigger_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SilentCnt:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", silent_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n==========================\n"));


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BIG WBRSSI %% - Over -40dBm \n"));
	/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RawCnt:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d  ", ed_2nd_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Percent:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", ed_big_rssi_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n==========================\n"));


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump CH_IDLE_STAT\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RawCnt:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d  ", idle_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Percent:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", (idle_stat[idx] *100)/ period_us));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n==========================\n"));	

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump CH_BUSY_STAT\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RawCnt:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d  ", busy_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Percent:"));
	idx = start;
	do {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", (busy_stat[idx] *100 )/ period_us));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n==========================\n"));

	return TRUE;
}
#endif /* DBG */
#endif /* ED_MONITOR */

