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
RTMP_STRING *wdev_type2str(int type);


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

	iPin = macAddr[0] * 256 * 256 + macAddr[4] * 256 + macAddr[5];

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
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS) && (*pCountryRegion & EEPROM_IS_PROGRAMMED))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CfgSetCountryRegion():CountryRegion in eeprom was programmed\n"));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CfgSetCountryRegion():region(%ld) out of range!\n", region));
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
#ifdef DOT11_VHT_AC
	PHY_11VHT_N_ABG_MIXED, (WMODE_B | WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC), /* 12 => B/G/GN/A/AN/AC mixed*/
	PHY_11VHT_N_AG_MIXED, (WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC), /* 13 => G/GN/A/AN/AC mixed , no B mode */
	PHY_11VHT_N_A_MIXED, (WMODE_A | WMODE_AN | WMODE_AC), /* 14 => A/AC/AN mixed */
	PHY_11VHT_N_MIXED, (WMODE_AN | WMODE_AC), /* 15 => AC/AN mixed, but no A mode */
#endif /* DOT11_VHT_AC */
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
	INT i, mode_cnt = sizeof(CFG_WMODE_MAP) / (sizeof(UCHAR) * 2);

	for (i = 0; i < mode_cnt; i++)
	{
		if (CFG_WMODE_MAP[i*2+1] == wmode)
			return CFG_WMODE_MAP[i*2];
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot get cfgmode by wmode(%x)\n",
				__FUNCTION__, wmode));

	return 0;
}


UCHAR cfgmode_2_wmode(UCHAR cfg_mode)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cfg_mode=%d\n", cfg_mode));
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

	if (str1)
		os_free_mem( str1);
	if (str2)
		os_free_mem(str2);

	return eq;
}

UCHAR wmode_2_rfic(UCHAR PhyMode)
{
	if(WMODE_CAP_2G(PhyMode) && WMODE_CAP_5G(PhyMode))
	{
		return RFIC_DUAL_BAND;
	}else
	if(WMODE_CAP_2G(PhyMode))
	{
		return RFIC_24GHZ;
	}else
	if(WMODE_CAP_5G(PhyMode))
	{
		return RFIC_5GHZ;
	}

	return RFIC_24GHZ;
}

/*N9 CMD BW value*/
void bw_2_str(UCHAR bw,CHAR* bw_str)
{
	switch(bw){
	case BW_20:
		sprintf(bw_str,"20");
	break;
	case BW_40:
		sprintf(bw_str,"20/40");
	break;
	case BW_80:
		sprintf(bw_str,"20/40/80");
	break;
	case BW_8080:
		sprintf(bw_str,"20/40/80/160NC");
	break;
	case BW_160:
		sprintf(bw_str,"20/40/80/160C");
	break;
	case BW_10:
		sprintf(bw_str,"10");
	break;
	case BW_5:
		sprintf(bw_str,"5");
	break;
	default:
		sprintf(bw_str,"Invaild");
	break;
	}
}


void extcha_2_str(UCHAR extcha,CHAR *ec_str)
{
	switch(extcha){
	case EXTCHA_NONE:
		sprintf(ec_str,"NONE");
	break;
	case EXTCHA_ABOVE:
		sprintf(ec_str,"ABOVE");
	break;
	case EXTCHA_BELOW:
		sprintf(ec_str,"BELOW");
	break;
	case EXTCHA_NOASSIGN:
		sprintf(ec_str,"Not assignment");
	break;
	default:
		sprintf(ec_str,"Invaild");
	break;
	}
}

/*
    ==========================================================================
    Description:
        Set Wireless Mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/

INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg,struct wifi_dev *wdev)
{
	LONG cfg_mode;
	UCHAR wmode, *mode_str;
#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */

	cfg_mode = simple_strtol(arg, 0, 10);

	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): Invalid wireless mode(%ld, wmode=0x%x), ChipCap(%s)\n",
				__FUNCTION__, cfg_mode, wmode,
				BAND_STR[pAd->chipCap.phy_caps & 0x3]));
		return FALSE;
	}

#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */

	if (wmode_band_equal(wdev->PhyMode, wmode) == TRUE)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Equal!\n"));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Not Equal!\n"));

	wdev->PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	mode_str = wmode_2_str(wmode);
	if (mode_str)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set WMODE=%s(0x%x)\n",
				__FUNCTION__, mode_str, wmode));
		os_free_mem(mode_str);
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
#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */

	cfg_mode = simple_strtol(arg, 0, 10);

	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
				__FUNCTION__, cfg_mode, wmode,
				BAND_STR[pAd->chipCap.phy_caps & 0x3]));
		return FALSE;
	}

	if (WMODE_CAP_5G(wmode) && WMODE_CAP_2G(wmode))
	{
		if (pAd->CommonCfg.dbdc_mode == 1) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP cannot support 2.4G/5G band mxied mode!\n"));
			return FALSE;
		}
	}


#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */

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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx, keyString));
			return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
#ifdef MT_MAC
	if (KeyLen == 32)
		pSharedKey->CipherAlg = CIPHER_WEP152;
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey:(KeyIdx=%d,type=%s, Alg=%s)\n",
						keyIdx, (bKeyIsHex == FALSE ? "Ascii" : "Hex"), CipherName[pSharedKey->CipherAlg]));

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
			default:
				fix_tx_mode = FIXED_TXMODE_HT;
		}
	}

	return fix_tx_mode;

}


INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR idx)
{
	INT	i, mac_len;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(arg);
	if(mac_len != 17)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid length (%d)\n", __FUNCTION__, mac_len));
		return FALSE;
	}

	if(strcmp(arg, "00:00:00:00:00:00") == 0)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid mac setting \n", __FUNCTION__));
		return FALSE;
	}

    if (idx == 0) {
	for (i = 0; i < MAC_ADDR_LEN; i++)
	{
		AtoH(arg, &pAd->CurrentAddress[i], 1);
		arg = arg + 3;
	}
	pAd->bLocalAdminMAC = TRUE;
    }
#if defined (MT_MAC) && defined (MBSS_SUPPORT)
    else {
        if (IS_MT7615(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
        {
            for (i = 0; i < MAC_ADDR_LEN; i++)
            {
                AtoH(arg, &pAd->ExtendMBssAddr[idx-1][i], 1);
                arg = arg + 3;
            }
			//TODO: Carter, is the below code still has its meaning?
            pAd->bLocalAdminExtendMBssMAC = TRUE;
        }
    }
#endif

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
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetAutoFallBack::(AutoFallBack=%d)\n", AutoFallBack));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RT_CfgSetWscPinCode(): invalid Wsc PinCode (%d)\n", pinCode));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWscPinCode():Wsc PinCode=%d\n", pinCode));

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


VOID rtmp_chip_prepare(RTMP_ADAPTER *pAd)
{
}

#ifdef FWDL_IN_PROBE
VOID rtmp_fwdl_prepare(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rtmp_fwdl_prepare(): FW DL in probe stage\n"));

	/* reset Adapter flags */
	RTMP_CLEAR_FLAGS(pAd);

	RtmpOSIRQRequest(pAd->net_dev);
	RTMP_DRIVER_IRQ_INIT(pAd);

	AsicGetMacInfo(pAd,&pAd->ChipID,&pAd->HWVersion,&pAd->FWVersion);

	if((ret = WfHifInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err0;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Hif Init Done!\n"));

	if((ret = WfMcuInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err1;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCU Init Done!\n"));

	/* hook e2p operation: the .dat is not available yet, need to force FLASH or EFUSE mode (BIN is forbidden). */
	RtmpChipOpsEepromHook(pAd, pAd->infType, E2P_FLASH_MODE);
	/* init EEPROM buffer (EFUSE mode can bypass this step) */
	pAd->chipOps.eeinit(pAd);
	
	/* clear the calibration sync CR PSE_SPARE_DUMMY_CR7 0x8206c06c bit[0] */
	RTMP_IO_WRITE32(pAd, 0xe000 + 0x06c , 0);
	/* do calibration, force FLASH or EFUSE mode. */
	MtCmdEfusBufferModeSetNoRsp(pAd, EEPROM_FLASH);

	/* free EEPROM buffer */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	if(pAd->CalDCOCImage != NULL)
		os_free_mem(pAd->CalDCOCImage);
	if(pAd->CalDPDAPart1Image != NULL)
		os_free_mem(pAd->CalDPDAPart1Image);
	if(pAd->CalDPDAPart2GImage != NULL)
		os_free_mem(pAd->CalDPDAPart2GImage);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

	/* driver IRQ is deinit here, while MCU and Hif are not.
		Because their init process will be bypassed during first interface up. */
	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	RTMP_DRIVER_IRQ_RELEASE(pAd);

	return;
	
err1:
	WfHifSysExit(pAd);
err0:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): initial faild!! ret=%d\n",__FUNCTION__, ret));
	return;
}

VOID rtmp_fwdl_exit(RTMP_ADAPTER *pAd)
{
	if (pAd->MCUCtrl.fwdl_in_probe == TRUE) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rtmp_fwdl_exit(): clear FW DL in probe stage\n"));
	
		/* enable interrupt to receive event */
        RtmpOSIRQRequest(pAd->net_dev);
	    RTMP_DRIVER_IRQ_INIT(pAd);
		RTMP_ASIC_INTERRUPT_ENABLE(pAd);

		RTMP_DRIVER_FWDL_IN_PROBE_CAL_WAIT_AND_CLEAR(pAd);
		
		/* reset FW to ROM code stage */
		NICRestartFirmware(pAd);

		/* free FW buffer */
		NICEraseFirmware(pAd);

		RTMP_DRIVER_IRQ_RELEASE(pAd);
		/* RTMP_ASIC_INTERRUPT_DISABLE will be done in WfMcuSysExit */
		WfSysPosExit(pAd);
	}
}
#endif /* FWDL_IN_PROBE */

static VOID rtmp_netdev_set(RTMP_ADAPTER *pAd, PNET_DEV net_dev)
{
	struct wifi_dev *wdev = NULL;
    INT32 ret = 0;
	/* set main net_dev */
	pAd->net_dev = net_dev;

#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		BSS_STRUCT *pMbss;
		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		ASSERT(pMbss);

        wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		RTMP_OS_NETDEV_SET_WDEV(net_dev, wdev);
        ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, net_dev, MAIN_MBSSID, (VOID *)pMbss, (VOID *)pAd);
	}
#endif /* CONFIG_AP_SUPPORT */

    if (!ret)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
                    RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
        RtmpOSNetDevFree(pAd->net_dev);
    }
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
	struct wifi_dev *wdev = NULL;


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
				rtmp_netdev_set(pAd,pData);
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


		case CMD_RTPRIV_IOCTL_SANITY_CHECK:
		/* sanity check before IOCTL */
			if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
#ifdef IFUP_IN_PROBE
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

#ifdef WDS_SUPPORT
            if (pObj->ioctl_if_type == INT_WDS)
                wdev = get_wdev_by_idx(pAd,(INT)Data+MIN_NET_DEVICE_FOR_WDS);
            else
#endif
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI)
				wdev = get_wdev_by_idx(pAd,(INT)Data+MIN_NET_DEVICE_FOR_APCLI);
			else
#endif
            wdev = get_wdev_by_idx(pAd,(INT)Data);

            if (wdev)
    			*(ULONG *)pData = wdev->channel;

            break;




		case CMD_RTPRIV_IOCTL_BEACON_UPDATE:
		/* update all beacon contents */
#ifdef CONFIG_AP_SUPPORT
            //TODO: Carter, the oid seems been obsoleted.
			UpdateBeaconHandler(
                    pAd,
                    NULL,
                    AP_RENEW);
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
			break;

		case CMD_RTPRIV_IOCTL_PCI_RESUME:
			break;

		case CMD_RTPRIV_IOCTL_PCI_CSR_SET:
			pAd->PciHif.CSRBaseAddress = (PUCHAR)Data;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->PciHif.CSRBaseAddress =0x%lx, csr_addr=0x%lx!\n", (ULONG)pAd->PciHif.CSRBaseAddress, (ULONG)Data));
			break;

		case CMD_RTPRIV_IOCTL_PCIE_INIT:
			RTMPInitPCIeDevice(pData, pAd);
			break;
#endif /* RTMP_PCI_SUPPORT */

		case CMD_RTPRIV_IOCTL_CHIP_PREPARE:
		{
			rtmp_chip_prepare(pAd);
		}
		break;
#ifdef FWDL_IN_PROBE
		case CMD_RTPRIV_IOCTL_FWDL_IN_PROBE_PREPARE:
		{
			rtmp_fwdl_prepare(pAd);
		}
		break;		
		case CMD_RTPRIV_IOCTL_FWDL_IN_PROBE_EXIT:
		{
			rtmp_fwdl_exit(pAd);
		}
		break;	
		case CMD_RTPRIV_IOCTL_FWDL_IN_PROBE_SET_FLAG:
		{
			pAd->MCUCtrl.fwdl_in_probe = TRUE;
		}
		break;		
		case CMD_RTPRIV_IOCTL_FWDL_IN_PROBE_CLEAR_FLAG:
		{
			pAd->MCUCtrl.fwdl_in_probe = FALSE;
		}
		break;
		case CMD_RTPRIV_IOCTL_FWDL_IN_PROBE_CAL_WAIT_AND_CLEAR:
		{
			/* calibration cmd is issued to FW, driver and FW sync status through a CR to see if the cmd is done. */
			UINT32 Value = 0;
			UINT32 counter = 0;
			
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	        ("%s: Wait FW complete the calibration cmd \n", __FUNCTION__));

			while(!(Value & BIT0)) { 
				RTMP_IO_READ32(pAd, 0xe000 + 0x06c , &Value);
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Polling Calibration Sync CR = %x\n", Value));
				counter++;
				os_msec_delay(50);
				if(counter > 300){
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Error: Polling Calibration CR fail. \n"));
					break;
				}
			}

			/* clear the calibration sync CR PSE_SPARE_DUMMY_CR7 0x8206c06c bit[0] */
			RTMP_IO_WRITE32(pAd, 0xe000 + 0x06c , 0);
		}
		break;		
#endif
#ifdef RT_CFG80211_SUPPORT
		case CMD_RTPRIV_IOCTL_CFG80211_CFG_START:
			RT_CFG80211_REINIT(pAd, wdev);
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
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unregister PPA::status=%d, if_id=%d\n", status, pAd->g_if_id));
			}
			os_free_mem(pAd->pDirectpathCb);
			break;
#endif /* INF_PPA_SUPPORT*/

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP:
        /* interface up */
        {
#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
            INT apidx = 0;
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */
            RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;
            if (VIRTUAL_IF_NUM(pAd) == 0)
            {
                VIRTUAL_IF_INC(pAd);
                if (pInfConf->mt_wifi_open(pAd->net_dev) != 0)
                {
                    VIRTUAL_IF_DEC(pAd);
                    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                            ("mt_wifi_open return fail!\n"));
                    return NDIS_STATUS_FAILURE;
                }
#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT

                               for(apidx = 0; apidx < MAX_MBSSID_NUM(pAd); apidx++)
                               {
                                        if (pAd->ApCfg.MBSSID[apidx].wdev.if_dev == pAd->net_dev)
                                                break;
                               }                                               
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				         ("Correct apidx from %d to %d for WscUUIDInit\n",pObj->ioctl_if,apidx));				
				WscUUIDInit(pAd, apidx, FALSE);
#else
				WscUUIDInit(pAd, pObj->ioctl_if, FALSE);
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */
            }
            else
            {
                VIRTUAL_IF_INC(pAd);
            }
            pInfConf->virtual_if_up_handler(pInfConf->operation_dev_p);
        }
			
#ifdef LINUX_NET_TXQ_SUPPORT
			/* reconfigure Linux net txq length for main BSS */
			if (pAd->tx_net_queue_len != 0)
				pAd->net_dev->tx_queue_len = pAd->tx_net_queue_len;
#endif /* LINUX_NET_TXQ_SUPPORT */
			
			break;

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN:
		/* interface down */
		{
			RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

			VIRTUAL_IF_DEC(pAd);

                        pInfConf->virtual_if_down_handler(pInfConf->operation_dev_p);
                        
			if (VIRTUAL_IF_NUM(pAd) == 0)
				pInfConf->mt_wifi_close(pAd->net_dev);
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
				RT_CMD_STATS *pStats = (RT_CMD_STATS *)pData;
				pStats->pStats = pAd->stats;
				if(pAd->OpMode == OPMODE_STA)
				{
					pStats->rx_packets = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
					pStats->tx_packets = pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart;
					pStats->rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
					pStats->tx_bytes = pAd->RalinkCounters.TransmittedByteCount;
					pStats->rx_errors = pAd->Counters8023.RxErrors;
					pStats->tx_errors = pAd->Counters8023.TxErrors;
					pStats->multicast = pAd->WlanCounters[0].MulticastReceivedFrameCount.QuadPart;   /* multicast packets received*/
					pStats->collisions = 0;  /* Collision packets*/
					pStats->rx_over_errors = pAd->Counters8023.RxNoBuffer;                   /* receiver ring buff overflow*/
					pStats->rx_crc_errors = 0;/*pAd->WlanCounters[0].FCSErrorCount;      recved pkt with crc error*/
					pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
					pStats->rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   /* recv'r fifo overrun*/
				}
#ifdef CONFIG_AP_SUPPORT
				else if(pAd->OpMode == OPMODE_AP)
				{
					INT index;
					BOOLEAN found_it = FALSE;
					INT stat_db_source;
					for(index = 0; index < MAX_MBSSID_NUM(pAd); index++)
					{
						if (pAd->ApCfg.MBSSID[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev))
						{
							found_it = TRUE;
							stat_db_source = 0;
							break;
						}
					}
#ifdef APCLI_SUPPORT
					if (found_it == FALSE) {
						for(index = 0; index < pAd->ApCfg.ApCliNum; index++)
						{
							if (pAd->ApCfg.ApCliTab[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev))
							{
								found_it = TRUE;
								stat_db_source = 1;
								break;
							}
						}
					}
#endif
					if (found_it == FALSE)
					{
						//reset counters
						pStats->rx_packets = 0;
						pStats->tx_packets = 0;
						pStats->rx_bytes = 0;
						pStats->tx_bytes = 0;
						pStats->rx_errors = 0;
						pStats->tx_errors = 0;
						pStats->multicast = 0;   /* multicast packets received*/
						pStats->collisions = 0;  /* Collision packets*/
						pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
						pStats->rx_crc_errors = 0; /* recved pkt with crc error*/
						pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
						pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/

						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMD_RTPRIV_IOCTL_INF_STATS_GET: can not find mbss I/F\n"));
						return NDIS_STATUS_FAILURE;
					}
					if (stat_db_source == 0) {
						pStats->rx_packets = pAd->ApCfg.MBSSID[index].RxCount;
						pStats->tx_packets = pAd->ApCfg.MBSSID[index].TxCount;
						pStats->rx_bytes = pAd->ApCfg.MBSSID[index].ReceivedByteCount;
						pStats->tx_bytes = pAd->ApCfg.MBSSID[index].TransmittedByteCount;
						pStats->rx_errors = pAd->ApCfg.MBSSID[index].RxErrorCount;
						pStats->tx_errors = pAd->ApCfg.MBSSID[index].TxErrorCount;
						pStats->multicast = pAd->ApCfg.MBSSID[index].mcPktsRx; /* multicast packets received */
						pStats->collisions = 0;  /* Collision packets*/
						pStats->rx_over_errors = 0;                   /* receiver ring buff overflow*/
						pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
						pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
						pStats->rx_fifo_errors = 0;                   /* recv'r fifo overrun*/
					} 
#ifdef APCLI_SUPPORT
					else if (stat_db_source == 1){
						pStats->rx_packets = pAd->ApCfg.ApCliTab[index].RxCount;
						pStats->tx_packets = pAd->ApCfg.ApCliTab[index].TxCount;
						pStats->rx_bytes = pAd->ApCfg.ApCliTab[index].ReceivedByteCount;
						pStats->tx_bytes = pAd->ApCfg.ApCliTab[index].TransmittedByteCount;
						pStats->rx_errors = pAd->ApCfg.ApCliTab[index].RxErrorCount;
						pStats->tx_errors = pAd->ApCfg.ApCliTab[index].TxErrorCount;
						pStats->multicast = pAd->ApCfg.ApCliTab[index].mcPktsRx; /* multicast packets received */
						pStats->collisions = 0;  /* Collision packets*/
						pStats->rx_over_errors = 0;                   /* receiver ring buff overflow*/
						pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
						pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
						pStats->rx_fifo_errors = 0;                   /* recv'r fifo overrun*/
					}
#endif
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


			/*check if the interface is down*/
			if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
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
			
		case CMD_RTPRIV_IOCTL_WDS_OPEN:
			WDS_Open(pAd,pData);
			break;
		case CMD_RTPRIV_IOCTL_WDS_CLOSE:
			WDS_Close(pAd,pData);
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

#ifdef CONFIG_ATE
#ifdef CONFIG_QA
		case (CMD_RTPRIV_IOCTL_COMMON)CMD_RTPRIV_IOCTL_ATE:
			{
				HQA_CMD_FRAME *HqaCmdFrame;

				os_alloc_mem_suspend(pAd, (UCHAR **)&HqaCmdFrame, sizeof(*HqaCmdFrame));
				if (!HqaCmdFrame) {
					Status = -ENOMEM;
					break;
				}
				NdisZeroMemory(HqaCmdFrame, sizeof(*HqaCmdFrame));
				Status = copy_from_user((PUCHAR)HqaCmdFrame, wrq->u.data.pointer, wrq->u.data.length);
				if (Status)	{
					Status = -EFAULT;
					goto IOCTL_ATE_ERROR;
				}
				Status = HQA_CMDHandler(pAd, wrq, HqaCmdFrame);
				//TODO: Check sanity
				IOCTL_ATE_ERROR:
					os_free_mem( HqaCmdFrame);
			}
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
		case (CMD_RTPRIV_IOCTL_COMMON)CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ:
		/* handle for SIOCGIWRATEQ */
		{
			RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
			HTTRANSMIT_SETTING HtPhyMode;
			UINT8					BW;
			UINT8					Antenna = 0;
			USHORT					MCS;
#ifdef APCLI_SUPPORT
			MAC_TABLE_ENTRY 		*pEntry = NULL;
#endif
			struct wifi_dev 		*wdev = NULL;

#ifdef APCLI_SUPPORT
			if (pRate->priv_flags == INT_APCLI) {
				pEntry = MacTableLookup2(pAd, pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.bssid, &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev);
				if (!pEntry) { // show maximum capability
					HtPhyMode = pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.HTPhyMode;
				} else {
					HtPhyMode = pEntry->HTPhyMode;
					if (HtPhyMode.field.MODE == MODE_VHT)
						Antenna = (HtPhyMode.field.MCS>>4) + 1;
				}
				wdev = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev;
			} else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
			if (pRate->priv_flags == INT_WDS) {
				HtPhyMode = pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev.HTPhyMode;
				wdev = &pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev;
			} else
#endif /* WDS_SUPPORT */
			{
				HtPhyMode = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.HTPhyMode;
				wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
			}
			
			MCS = HtPhyMode.field.MCS;
#ifdef DOT11_N_SUPPORT	
			if ((HtPhyMode.field.MODE == MODE_HTMIX) 
				|| (HtPhyMode.field.MODE == MODE_HTGREENFIELD)) {
				Antenna = (HtPhyMode.field.MCS>>3) +1;
				MCS = MCS & 0xffff;
			}
#endif
#ifdef DOT11_VHT_AC
			if (HtPhyMode.field.MODE == MODE_VHT) {
				MCS = MCS & 0xf;
			}
#endif

			if ((HtPhyMode.field.MODE >= MODE_VHT) && (wdev != NULL)) {
				BW = wlan_operate_get_vht_bw(wdev);
				if (BW == 0) // HT20/40
					BW = 1;
				else if(BW == 1) // VHT80
					BW = 2; 
				else if(BW >= 2) // VHT80-80,VHT160
					BW = 3; 
			} else if ((HtPhyMode.field.MODE >= MODE_HTMIX) && (wdev != NULL))
				BW = wlan_operate_get_ht_bw(wdev);
			else
				BW = HtPhyMode.field.BW;

            if (Antenna == 0)
                Antenna = wlan_config_get_tx_stream(wdev);

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("HtPhyMode.field.MODE=%d\n\r",HtPhyMode.field.MODE));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("HtPhyMode.field.ShortGI=%d\n\r",HtPhyMode.field.ShortGI));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("HtPhyMode.field.BW=%d\n\r",HtPhyMode.field.BW));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("HtPhyMode.field.MCS=%d\n\r",MCS));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("HT_TxStream=%d\n\r",wlan_config_get_tx_stream(wdev)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BW=%d\n\r",BW));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Antenna=%d\n\r",Antenna));
			RtmpDrvMaxRateGet(pAd, HtPhyMode.field.MODE, HtPhyMode.field.ShortGI,
							  BW, MCS,
							  Antenna,
							  (UINT32 *)&pRate->BitRate);
		}
			break;
#endif /* CONFIG_AP_SUPPORT */

		case CMD_RTPRIV_IOCTL_SIOCGIWNAME:
			RtmpIoctl_rt_ioctl_giwname(pAd, pData, 0);
			break;

#ifdef PROFILE_PATH_DYNAMIC
		case CMD_RTPRIV_IOCTL_PROFILEPATH_SET:
			pAd->profilePath = (CHAR *)Data;
		break;
#endif /* PROFILE_PATH_DYNAMIC */

	}

#ifdef RT_CFG80211_SUPPORT
	if ((CMD_RTPRIV_IOCTL_80211_START <= cmd) &&
		(cmd <= CMD_RTPRIV_IOCTL_80211_END))
	{
		Status = CFG80211DRV_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);
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
	POS_COOKIE pObj;
#ifdef CONFIG_AP_SUPPORT
	UCHAR ifIndex;
	struct wifi_dev *wdev = NULL;
#endif
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	//check if the interface is down
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

#ifdef CONFIG_AP_SUPPORT
	ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
		wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
	else 
#endif
	if (pObj->ioctl_if_type == INT_MBSSID)
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;
#endif


    NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID))
		{
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);
		}
		else
		{
			Ssid.SsidLength = 0;
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_PASSIVE, FALSE, wdev);
		}


		return TRUE;
	}
#endif /* AP_SCAN_SUPPORT */
#endif // CONFIG_AP_SUPPORT //


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_SiteSurvey_Proc\n"));

    return TRUE;
}

#ifdef WH_EZ_SETUP
INT Set_PartialScan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    POS_COOKIE  pObj= (POS_COOKIE) pAd->OS_Cookie;
        UCHAR           ifIndex;
    UINT8       bPartialScanning;

        if ((pObj->ioctl_if_type != INT_APCLI) &&
        (pObj->ioctl_if_type != INT_MAIN) &&
        (pObj->ioctl_if_type != INT_MBSSID))
                return FALSE;
    ifIndex = pObj->ioctl_if;
    bPartialScanning = simple_strtol(arg, 0, 10);
    //if(bPartialScanning > 0)
    {
        if(((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) &&
           (ifIndex < HW_BEACON_MAX_NUM))
		{
			if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[ifIndex].wdev))
           	{
		   		pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				pAd->ScanCtrl.PartialScan.bPartialScanAllowed = bPartialScanning ? TRUE:FALSE;
				pAd->ScanCtrl.PartialScan.LastScanChannel = 0;
				if (IS_SINGLE_CHIP_DBDC(pAd))
				{
					if (pAd->ApCfg.BssidNum == 2 && pAd->CommonCfg.dbdc_mode == 1)
					{
						pAd->ApCfg.MBSSID[ifIndex].wdev.ez_driver_params.bPartialScanRunning = FALSE;
						pAd->ApCfg.MBSSID[ifIndex ^ 1].wdev.ez_driver_params.bPartialScanRunning = FALSE;
						pAd->ApCfg.ApCliTab[ifIndex].wdev.ez_driver_params.bPartialScanRunning = FALSE;
						pAd->ApCfg.ApCliTab[ifIndex ^ 1].wdev.ez_driver_params.bPartialScanRunning = FALSE;
					}
				}

           	}
		   	else 
			{
				if(bPartialScanning > 0)
					pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		   	}
		}
#ifdef APCLI_SUPPORT
        else if((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
		{
			if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[ifIndex].wdev))
       		{
				pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
				pAd->ScanCtrl.PartialScan.bPartialScanAllowed = bPartialScanning ? TRUE:FALSE;
				pAd->ScanCtrl.PartialScan.LastScanChannel = 0;
				if (IS_SINGLE_CHIP_DBDC(pAd))
				{
					if (pAd->ApCfg.BssidNum == 2 && pAd->CommonCfg.dbdc_mode == 1)
					{
						pAd->ApCfg.MBSSID[ifIndex].wdev.ez_driver_params.bPartialScanRunning = FALSE;
						pAd->ApCfg.MBSSID[ifIndex ^ 1].wdev.ez_driver_params.bPartialScanRunning = FALSE;
						pAd->ApCfg.ApCliTab[ifIndex].wdev.ez_driver_params.bPartialScanRunning = FALSE;
						pAd->ApCfg.ApCliTab[ifIndex ^ 1].wdev.ez_driver_params.bPartialScanRunning = FALSE;
					}
				}

			}
			else
			{
				if(bPartialScanning > 0)
					pAd->ScanCtrl.PartialScan.pwdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
			}
		}
#endif /* APCLI_SUPPORT */
    }
    pAd->ScanCtrl.PartialScan.bScanning = bPartialScanning ? TRUE:FALSE;
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                                ("%s(): bScanning = %u, bPartialScanAllowed %u\n", __FUNCTION__, pAd->ScanCtrl.PartialScan.bScanning, pAd->ScanCtrl.PartialScan.bPartialScanAllowed));
        return TRUE;
}
#endif




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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (ON)\n"));
	}
	else
	{
		MacReg &= (~0x00040000);
		pAd->bUseHwTxLURate = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (OFF)\n"));
	}
	RTMP_IO_WRITE32(pAd, TX_FBK_LIMIT, MacReg);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseHwTxLURate = %d \n", pAd->bUseHwTxLURate));

	return TRUE;
}
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable = simple_strtol(arg, 0, 10);

	pAd->bUseMultiMacAddrExt = (Enable ? TRUE : FALSE);
	AsicSetReptFuncEnable(pAd, pAd->bUseMultiMacAddrExt);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseMultiMacAddrExt = %d, UseMultiMacAddrExt(%s)\n",
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

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn on TSSI mechanism\n"));
	} else if (tssi_enable == 0) {
		pAd->chipCap.tssi_enable = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn off TSS mechanism\n"));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal param(%u)\n", tssi_enable));
		return FALSE;
    }
	return TRUE;
}

#ifdef RLT_MAC
#ifdef CONFIG_FW_DEBUG
INT set_fw_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 fw_debug_param;

	fw_debug_param = simple_strtol(arg, 0, 10);

#ifdef RLT_MAC
	AndesRltFunSet(pAd, LOG_FW_DEBUG_MSG, fw_debug_param);
#endif /* RLT_MAC */

	return TRUE;
}
#endif /* CONFIG_FW_DEBUG */
#endif

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR radio;
    struct wifi_dev *wdev;
    POS_COOKIE pObj;
	UCHAR RfIC = 0;

    pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifdef CONFIG_AP_SUPPORT
    wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	RfIC = wmode_2_rfic(wdev->PhyMode);
	radio = simple_strtol(arg, 0, 10);

	if (!wdev->if_up_down_state)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==>Set_RadioOn_Proc (%s) but IF is done, ignore!!! (wdev_idx %d)\n", 
			radio? "ON":"OFF", wdev->wdev_idx));
		return TRUE;
	}
	
	if (radio == !IsHcRadioCurStatOffByChannel(pAd, wdev->channel))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==>Set_RadioOn_Proc (%s) equal to current state, ignore!!! (wdev_idx %d)\n", 
			radio? "ON":"OFF", wdev->wdev_idx));
		return TRUE;
	}

	if (radio)
	{
		MlmeRadioOn(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (ON)\n"));

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
			APStartUpByRf(pAd, RfIC);
		}
#endif
	}
	else
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
			APStopByRf(pAd, RfIC);
		}
#endif
		MlmeRadioOff(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (OFF)\n"));
	}

	return TRUE;
}

#ifdef NEW_SET_RX_STREAM
INT	Set_RxStream_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT RxStream;

	RxStream = simple_strtol(arg, 0, 10);

    AsicSetRxStream(pAd, RxStream, 0);

	return TRUE;
}
#endif

INT	Set_Lp_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR lp_enable;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UCHAR RfIC = 0;
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		RfIC = wmode_2_rfic(wdev->PhyMode);
	}
#endif /* CONFIG_AP_SUPPORT */
	lp_enable = simple_strtol(arg, 0, 10);

	if (lp_enable)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
			APStopByRf(pAd, RfIC);
		}
#endif
		MlmeLpEnter(pAd);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_Lp_Proc (Enetr)\n"));
	}
	else
	{
		MlmeLpExit(pAd);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
			APStartUpByRf(pAd, RfIC);
		}
#endif
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_Lp_Proc (Exit)\n"));
	}

	return TRUE;
}

#ifdef MT_MAC
INT setTmrVerProc(
    IN  PRTMP_ADAPTER   pAd,
    IN  RTMP_STRING *arg)
{
    UCHAR ver;
    RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

    ver = (UCHAR)simple_strtol(arg, 0, 10);

    if (ver > TMR_VER_2_0)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s: wrong setting %d, remain default %d!!\n",
                    __FUNCTION__, ver, pChipCap->TmrHwVer));
        return FALSE;
    }

#ifdef MT7615
    if ((MTK_REV_GTE(pAd, MT7615, MT7615E3)) && (ver <= TMR_VER_1_5))
    {
        pChipCap->TmrHwVer = ver;
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s: set to TMR_VER:%d!!\n",
                    __FUNCTION__, ver));
    }
    else
#endif
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s: only support TMR_VER 1.0!!\n",
                    __FUNCTION__));
    }
    return TRUE;
}

INT setTmrEnableProc(
    IN  PRTMP_ADAPTER   pAd,
    IN  RTMP_STRING *arg)
{
    LONG enable;

    enable = simple_strtol(arg, 0, 10);

    if ((enable < TMR_DISABLE) || (enable > TMR_RESPONDER)) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: enable is incorrect!!\n", __func__));
        return FALSE;
    }

    if (pAd->chipCap.hif_type != HIF_MT) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: chipCap is not HIF_MT\n", __func__));
        return FALSE;
    }

    TmrCtrl(pAd, (UCHAR)enable, pAd->chipCap.TmrHwVer);

    return TRUE;
}

#ifndef COMPOS_TESTMODE_WIN
INT SetTmrCalProc(
    IN  PRTMP_ADAPTER   pAd,
    IN  RTMP_STRING *arg)
{
	UCHAR TmrType = simple_strtol(arg, 0, 10);
	UCHAR Channel = HcGetRadioChannel(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("%s(): TMR Calibration, TmrType: %d\n", __FUNCTION__, TmrType));

	if (pAd->chipCap.hif_type == HIF_MT)
	    MtSetTmrCal(pAd, TmrType,Channel,pAd->CommonCfg.BBPCurrentBW);

	return TRUE;
}
#endif

#ifdef TMR_VERIFY
INT Set_SendTmrAction_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  RTMP_STRING *arg)
{
    UCHAR                   macAddr[MAC_ADDR_LEN] = {0,0,0,0,0,0};
    RTMP_STRING *value;
    INT                     i;
    PUCHAR pOutBuffer = NULL;

    NDIS_STATUS NStatus;
    ULONG FrameLen;
    FRAME_FTM_ACTION Frame;

    NdisZeroMemory(&Frame, sizeof(FRAME_FTM_ACTION));

    if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
        return FALSE;

    for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
    {
        if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
            return FALSE;  /*Invalid */

        AtoH(value, (UCHAR *)&macAddr[i++], 1);
    }

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory*/
    if(NStatus != NDIS_STATUS_SUCCESS)
    {
        return FALSE;
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("send Tmr action %02x:%02x:%02x:%02x:%02x:%02x\n",
                    PRINT_MAC(macAddr)));

    ActHeaderInit(pAd, &Frame.Hdr, macAddr, pAd->CurrentAddress, macAddr);
    Frame.Category = CATEGORY_PUBLIC;
    Frame.Action = ACTION_FTM;
    /*Other field remains to zero now.*/

    MakeOutgoingFrame(pOutBuffer, &FrameLen,
                      sizeof(FRAME_FTM_ACTION), &Frame,
                      END_OF_ARGS);

    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
    MlmeFreeMemory(pOutBuffer);

    return TRUE;
}

INT Set_TmrRate_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  RTMP_STRING *arg)
{
    UCHAR rate;

    rate = simple_strtol(arg, 0, 10);
    pAd->tmr_rate = rate;

    return TRUE;
}

INT Set_TmrSpeEn_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  RTMP_STRING *arg)
{
    UCHAR spe_en;

    spe_en = simple_strtol(arg, 0, 10);
    pAd->spe_en = spe_en;

    return TRUE;
}
#endif /* TMR_VERIFY */
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

	return TRUE;
}
#endif

INT set_cr4_query(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 option = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: arg = %s\n", __FUNCTION__, arg));

	if (arg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Invalid parameters\n", __FUNCTION__));
		return FALSE;
	}

	option = simple_strtoul(arg, 0, 16);

	MtCmdCr4Query(pAd, option, 0, 0);

	return TRUE;
}


INT set_cr4_set(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 arg0 = 0;
	UINT32 arg1 = 0;
    UINT32 arg2 = 0;

    RTMP_STRING *arg0_ptr = NULL;
    RTMP_STRING *arg1_ptr = NULL;


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: arg = %s\n", __FUNCTION__, arg));

	arg0_ptr = strsep(&arg, ":");
    arg1_ptr = strsep(&arg, ":");


	if (arg0_ptr == NULL || arg1_ptr == NULL || arg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Invalid parameters\n", __FUNCTION__));
		return FALSE;
	}

	arg0 = simple_strtoul(arg0_ptr, 0, 16);
    arg1 = simple_strtoul(arg1_ptr, 0, 16);
    arg2 = simple_strtoul(arg, 0, 16);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("%s: arg0 = 0x%x, arg1 = 0x%x, arg2 = 0x%x\n",
            __FUNCTION__, arg0, arg1, arg2));

	MtCmdCr4Set(pAd, arg0, arg1, arg2);

	return TRUE;
}


INT set_cr4_capability(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 option = 0;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            (":%s: arg = %s\n", __FUNCTION__, arg));

    if (arg == NULL)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            (":%s: Invalid parameters\n", __FUNCTION__));
        return FALSE;
    }

    option = simple_strtoul(arg, 0, 16);


    MtCmdCr4Capability(pAd, option);

    return TRUE;
}


INT set_cr4_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 option = 0;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            (":%s: arg = %s\n", __FUNCTION__, arg));

    if (arg == NULL)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            (":%s: Invalid parameters\n", __FUNCTION__));
        return FALSE;
    }

    option = simple_strtoul(arg, 0, 16);


    MtCmdCr4Debug(pAd, option);

    return TRUE;
}

INT dump_cr4_pdma_debug_probe(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 i = 0;
	UINT32 u4Low = 0, u4High = 0;
    UINT32 origonal_cr1_value = 0;

	if (IS_MT7615(pAd))
	{
        /* keep the origonal remap cr1 value for restore */
        HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &origonal_cr1_value);
        /* do PCI-E remap for CR4 PDMA physical base address to 0x40000 */
        HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, 0x82000000);
	}
    else
    {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s:Non 7615 don't support this cmd !\n", __FUNCTION__));
		return FALSE;
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("%s: ======= CR4 PDMA1 Debug Probe ========\n", __FUNCTION__));

	for (i = 0; i <= 0x10; i++)
	{
        RTMP_IO_WRITE32(pAd, 0x40408, (i | (i << 8)));
        RTMP_IO_WRITE32(pAd, 0x40400, 0x0201);
        RTMP_IO_READ32(pAd, 0x40404, &u4Low);

        RTMP_IO_WRITE32(pAd, 0x40400, 0x0403);
        RTMP_IO_READ32(pAd, 0x40404, &u4High);

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("cr4_dbg_pdma1_%u: 0x%x\n", i, ((u4High & 0xFFFF) << 16) | (u4Low & 0xFFFF)));
	}

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("%s: ======== CR4 PDMA2 Debug Probe ========\n", __FUNCTION__));

    for (i = 0; i <= 0x10; i++)
	{
        RTMP_IO_WRITE32(pAd, 0x40408, (i | (i << 8)));
        RTMP_IO_WRITE32(pAd, 0x40400, 0x0605);
        RTMP_IO_READ32(pAd, 0x40404, &u4Low);

        RTMP_IO_WRITE32(pAd, 0x40400, 0x0807);
        RTMP_IO_READ32(pAd, 0x40404, &u4High);

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("cr4_dbg_pdma2_%u: 0x%x\n", i, ((u4High & 0xFFFF) << 16) | (u4Low & 0xFFFF)));
	}

    if (IS_MT7615(pAd))
    {
        /* restore the origonal remap cr1 value */
        HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, origonal_cr1_value);
    }
    return TRUE;
}

INT dump_remap_cr_content(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 origonal_remap_cr_value = 0;
    UINT32 remapped_register_value = 0;

    UINT32 cr_selected_to_remap = 0;
    UINT32 address_you_want_to_remap = 0;
    UINT32 remap_cr_record_base_address = 0;
    UINT32 offset_between_target_and_remap_cr_base = 0;

    RTMP_STRING *cr_selected_to_remap_ptr = NULL;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s: arg = %s\n", __FUNCTION__, arg));

    if (arg == NULL)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s: Invalid parameters!\n", __FUNCTION__));
        return FALSE;
    }

	cr_selected_to_remap_ptr = strsep(&arg, ":");

    if (!cr_selected_to_remap_ptr || !arg)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s:miss input parameters!\n", __FUNCTION__));
        return FALSE;
    }

    cr_selected_to_remap = simple_strtoul(cr_selected_to_remap_ptr, 0, 16);
    address_you_want_to_remap = simple_strtoul(arg, 0, 16);

	if (IS_MT7615(pAd) || IS_MT7622(pAd))
	{
	    if (cr_selected_to_remap == 1)
        {
            /* keep the origonal remap cr1 value for restore */
            HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &origonal_remap_cr_value);
            /* do PCI-E remap for CR4 PDMA physical base address to 0x40000 */
            HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, address_you_want_to_remap);

            HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &remap_cr_record_base_address);

            if ((address_you_want_to_remap - remap_cr_record_base_address) > REMAP_1_OFFSET_MASK)
            {
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    ("%s:Exceed CR1 remap range(offset: 0x%x > mask: 0x%x)!!\n", __FUNCTION__,
                    (address_you_want_to_remap - remap_cr_record_base_address), REMAP_1_OFFSET_MASK));

                /* restore the origonal remap cr1 value */
                HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, origonal_remap_cr_value);
                return FALSE;
            }
            offset_between_target_and_remap_cr_base =
                ((address_you_want_to_remap - remap_cr_record_base_address)
                                                    & REMAP_1_OFFSET_MASK);
        }
        else if (cr_selected_to_remap == 2)
        {
            /* keep the origonal remap cr2 value for restore */
            HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &origonal_remap_cr_value);
            /* do PCI-E remap the address you want to 0x80000 */
            HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, address_you_want_to_remap);

            HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &remap_cr_record_base_address);

            if ((address_you_want_to_remap - remap_cr_record_base_address) > REMAP_2_OFFSET_MASK)
            {
                 MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                     ("%s:Exceed CR2 remap range(offset: 0x%x > mask: 0x%x)!!\n", __FUNCTION__,
                     (address_you_want_to_remap - remap_cr_record_base_address), REMAP_2_OFFSET_MASK));

                 /* restore the origonal remap cr2 value */
                 HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, origonal_remap_cr_value);
                 return FALSE;
            }

            offset_between_target_and_remap_cr_base =
                ((address_you_want_to_remap - remap_cr_record_base_address)
                                                    & REMAP_2_OFFSET_MASK);
        }
        else
        {
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("%s: Error! Unknow remap CR selected\n", __FUNCTION__));

            /* restore the origonal remap cr2 value */
            HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, origonal_remap_cr_value);
            return FALSE;
        }
    }
    else
    {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s:Non 7615 don't support this cmd !\n", __FUNCTION__));
		return FALSE;
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("%s:%s origonal value = 0x%x, remap address = 0x%x,\n"
        "new remap cr value = 0x%x, offset = 0x%x\n", __FUNCTION__,
            ((cr_selected_to_remap == 1) ? "CR1" : "CR2"),
            origonal_remap_cr_value, address_you_want_to_remap,
            remap_cr_record_base_address, offset_between_target_and_remap_cr_base));



    if (IS_MT7615(pAd) || IS_MT7622(pAd))
    {
        if (cr_selected_to_remap == 1)
        {
            RTMP_IO_READ32(pAd, 0x40000 + offset_between_target_and_remap_cr_base,
                                                        &remapped_register_value);
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("%s:0x%x = 0x%x\n", __FUNCTION__,
                        (0x40000 + offset_between_target_and_remap_cr_base),
                        remapped_register_value));
            /* restore the origonal remap cr1 value */
            HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, origonal_remap_cr_value);
        }
        else if (cr_selected_to_remap == 2)
        {
            RTMP_IO_READ32(pAd, 0x80000 + offset_between_target_and_remap_cr_base,
                                        &remapped_register_value);
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                            ("%s:0x%x = 0x%x\n", __FUNCTION__,
                (0x80000 + offset_between_target_and_remap_cr_base),
                                        remapped_register_value));
            /* restore the origonal remap cr2 value */
            HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, origonal_remap_cr_value);
        }
    }
    return TRUE;
}

#define MAX_POLLING_RESET_COUNT 5
INT set_recover_lmac(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 restore_arb_tqsw0_cr_value = 0;
    UINT32 restore_arb_tqsw1_cr_value = 0;
    UINT32 restore_arb_tqsw2_cr_value = 0;
    UINT32 restore_arb_tqsw3_cr_value = 0;
    UINT32 restore_arb_tqsm0_cr_value = 0;
    UINT32 restore_arb_tqsm1_cr_value = 0;
    UINT32 restore_arb_tqse0_cr_value = 0;
    UINT32 restore_arb_tqse1_cr_value = 0;
    UINT32 restore_arb_tqsn_cr_value = 0;
    UINT32 restore_arb_rqcr_cr_value = 0;
    UINT32 restore_arb_scr_cr_value = 0;
    UINT32 restore_cfg_ccr_cr_value = 0;
    UINT32 loop_counter = 0;
    UINT32 temp_cr_value = 0;
    
    /* 1. Store status */
    /* 1.1 Queue enable, Read 0x820f3100 ~ 0x820f311C, 820f3000 (tx_que_en0 ~ tx_que_en8) */
   	MAC_IO_READ32(pAd, ARB_TQSW0, &restore_arb_tqsw0_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSW1, &restore_arb_tqsw1_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSW2, &restore_arb_tqsw2_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSW3, &restore_arb_tqsw3_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSM0, &restore_arb_tqsm0_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSM1, &restore_arb_tqsm1_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSE0, &restore_arb_tqse0_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSE1, &restore_arb_tqse1_cr_value);
    MAC_IO_READ32(pAd, ARB_TQSN, &restore_arb_tqsn_cr_value);

    /* Read 0x820f3070, store bit [24,23,20,16,8,7,4,0] as rx_que_en */
    MAC_IO_READ32(pAd, ARB_RQCR, &restore_arb_rqcr_cr_value);
    restore_arb_rqcr_cr_value &= ARB_RQCR_ALL_RXQ_EN_MASK;

    /* 1.2 PHY disable, Read 0x820f3080[11:8], store value as phy_dis */
    MAC_IO_READ32(pAd, ARB_SCR, &restore_arb_scr_cr_value);
    restore_arb_scr_cr_value &= MT_ARB_SCR_DISABLE_PHY_MASK;

    /* 1.3 LMAC clock control, Read 0x820F0000, store value as clk_en */
    MAC_IO_READ32(pAd, CFG_CCR, &restore_cfg_ccr_cr_value);


    /* 2. SW reset LMAC */ 
    /* 2.1 Enable LMAC clk for recovery Write 0x820F0000 = 0xC300_0000 */
    MAC_IO_READ32(pAd, CFG_CCR, &temp_cr_value);
    temp_cr_value |= CGG_CCR_GC_ALL_CK_FREE_RUN_MASK;
    MAC_IO_WRITE32(pAd, CFG_CCR, temp_cr_value);

    
    /* 2.2  Flush Tx queue 0x820f3120 ~ 0x820f313C, 820f3004(tx_que_en0 ~ tx_que_en8) */
    MAC_IO_WRITE32(pAd, ARB_TQFW0, restore_arb_tqsw0_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFW1, restore_arb_tqsw1_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFW2, restore_arb_tqsw2_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFW3, restore_arb_tqsw3_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFM0, restore_arb_tqsm0_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFM1, restore_arb_tqsm1_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFE0, restore_arb_tqse0_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFE1, restore_arb_tqse1_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQFN, restore_arb_tqsn_cr_value);

    /* 2.3 Reset Rx queue, Write 820f3070[2] = 1 */
    MAC_IO_READ32(pAd, ARB_RQCR, &temp_cr_value);
    temp_cr_value |= ARB_RQCR_RX_RESET;
    MAC_IO_WRITE32(pAd, ARB_RQCR, temp_cr_value);

    /* 2.4 Wait Tx queue flush done and Rx reset queue done  */    
    do {
        UINT32 tqfw0_cr_value = 0;
        UINT32 tqfw1_cr_value = 0;
        UINT32 tqfw2_cr_value = 0;
        UINT32 tqfw3_cr_value = 0;
        UINT32 tqfm0_cr_value = 0;
        UINT32 tqfm1_cr_value = 0;
        UINT32 tqfe0_cr_value = 0;
        UINT32 tqfe1_cr_value = 0;
        UINT32 tqfn_cr_value = 0;
        UINT32 rx_reset_result = 0;        
        UINT32 tx_flush_result = 0;
        UINT32 finish_result = 0;
        
        /* 2.4.1 Wait Tx flush done: wait 0x820f3120~13c, 820f3004 = 0 */
        MAC_IO_READ32(pAd, ARB_TQFW0, &tqfw0_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFW1, &tqfw1_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFW2, &tqfw2_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFW3, &tqfw3_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFM0, &tqfm0_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFM1, &tqfm1_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFE0, &tqfe0_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFE1, &tqfe1_cr_value);
        MAC_IO_READ32(pAd, ARB_TQFN, &tqfn_cr_value);

        tx_flush_result = (tqfw0_cr_value   | 
                           tqfw1_cr_value   | 
                           tqfw2_cr_value   | 
                           tqfw3_cr_value   | 
                           tqfm0_cr_value   | 
                           tqfm1_cr_value   | 
                           tqfe0_cr_value   | 
                           tqfe1_cr_value   |
                           tqfn_cr_value);

        if (tx_flush_result != NDIS_STATUS_SUCCESS)
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                                    ("tx_flush not done!\n"));
        
        /*2.4.2 Wait Rx reset done: wait 820f3070[2] = 0 */
        MAC_IO_READ32(pAd, ARB_RQCR, &rx_reset_result);
        rx_reset_result =  ((rx_reset_result & ARB_RQCR_RX_RESET) >> 2);

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                                       ("rx_reset_result = 0x%x\n", rx_reset_result));

        if (rx_reset_result != NDIS_STATUS_SUCCESS)
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                                    ("rx_reset not done!\n"));
        
        finish_result = (tx_flush_result | rx_reset_result);
        if (finish_result == NDIS_STATUS_SUCCESS)
            break;

        /* Total Max Delay 1ms */
        RtmpusecDelay(200);
        //RtmpOsMsDelay(1);
        loop_counter++; 

    } while (loop_counter < MAX_POLLING_RESET_COUNT);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                               ("loop_counter = 0x%x\n", loop_counter));

    if (loop_counter == MAX_POLLING_RESET_COUNT)
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                            ("exit loop cause counter!\n"));
    else
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                    ("exit loop cause reset done!\n"));

    /* 2.5 Mask UMAC <=> LMAC interface, Write 82060118[9:8] = 2'h3 */
    MAC_IO_READ32(pAd, PLE_FUNC_CTRL_0, &temp_cr_value);
    temp_cr_value |= WF_PLE_UMAC_LMAC_MASK;
    MAC_IO_WRITE32(pAd, PLE_FUNC_CTRL_0, temp_cr_value);

    RtmpOsMsDelay(1);

    /* 2.6 Disable PHY, Write 820f3080[11:8] = 4'hF */
    MAC_IO_READ32(pAd, ARB_SCR, &temp_cr_value);
    temp_cr_value |= MT_ARB_SCR_DISABLE_PHY_MASK;
    MAC_IO_WRITE32(pAd, ARB_SCR, temp_cr_value);

    /* 2.7 SW reset LMAC by WF_CFG_TOP and WF_OFF_CFG_TOP */
    /* 2.7.1 Write 820f0800[2] = 1 */
    MAC_IO_READ32(pAd, CFG_OFF_WOSWRST_EN, &temp_cr_value);
    temp_cr_value |= MACOFF_LOGRST_EN;
    MAC_IO_WRITE32(pAd, CFG_OFF_WOSWRST_EN, temp_cr_value);

    /* 2.7.2 Write 820f0004[2] = 1 */
    MAC_IO_READ32(pAd, CFG_SWRST_EN, &temp_cr_value);
    temp_cr_value |= MACON_LOGRST_EN;
    MAC_IO_WRITE32(pAd, CFG_SWRST_EN, temp_cr_value);

    /* 2.7.3 Delay 10us */
    RtmpusecDelay(10);

    /* 3. Recovery */ 
    /* 3.1 Unmask UMAC <=> LMAC interface, Write 82060118[9:8] = 2'h0 */
    MAC_IO_READ32(pAd, PLE_FUNC_CTRL_0, &temp_cr_value);
    temp_cr_value &= ~WF_PLE_UMAC_LMAC_MASK;
    MAC_IO_WRITE32(pAd, PLE_FUNC_CTRL_0, temp_cr_value);

    /* 3.2 enable PHY, Write 820f3080[11:8] = phy_dis */
    MAC_IO_READ32(pAd, ARB_SCR, &temp_cr_value);
    temp_cr_value &= ~MT_ARB_SCR_DISABLE_PHY_MASK;
    temp_cr_value |= restore_arb_scr_cr_value;
    MAC_IO_WRITE32(pAd, ARB_SCR, temp_cr_value);

    /* 3.3 Start queue, */
    /* 3.3.1 Write 0x820f3100~1C, 820f3000(tx_que_en 0 ~ tx_que_en 8) */
    MAC_IO_WRITE32(pAd, ARB_TQSW0, restore_arb_tqsw0_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSW1, restore_arb_tqsw1_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSW2, restore_arb_tqsw2_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSW3, restore_arb_tqsw3_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSM0, restore_arb_tqsm0_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSM1, restore_arb_tqsm1_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSE0, restore_arb_tqse0_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSE1, restore_arb_tqse1_cr_value);
    MAC_IO_WRITE32(pAd, ARB_TQSN, restore_arb_tqsn_cr_value);

    /* 3.3.2 Write 0x820f3070[24,23,20,16,8,7,4,0] = rx_que_en */
    MAC_IO_READ32(pAd, ARB_RQCR, &temp_cr_value);
    temp_cr_value &= ~ARB_RQCR_ALL_RXQ_EN_MASK;
    temp_cr_value |= restore_arb_rqcr_cr_value;
    MAC_IO_WRITE32(pAd, ARB_RQCR, temp_cr_value);

    /* 3.4 LMAC clock control, Write 0x820F0000 = clk_en */
    MAC_IO_WRITE32(pAd, CFG_CCR, restore_cfg_ccr_cr_value);

    /*  	LP Re-calculate TBTT */
    MAC_IO_READ32(pAd, LPON_T0TPCR, &temp_cr_value);
    MAC_IO_WRITE32(pAd, LPON_T0TPCR, temp_cr_value);
    MAC_IO_READ32(pAd, LPON_T1TPCR, &temp_cr_value);
    MAC_IO_WRITE32(pAd, LPON_T1TPCR, temp_cr_value);
    MAC_IO_READ32(pAd, LPON_T2TPCR, &temp_cr_value);
    MAC_IO_WRITE32(pAd, LPON_T2TPCR, temp_cr_value);
    MAC_IO_READ32(pAd, LPON_T3TPCR, &temp_cr_value);
    MAC_IO_WRITE32(pAd, LPON_T3TPCR, temp_cr_value);

    return TRUE;
}

INT set_re_calibration(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 BandIdx = 0;
	UINT32 CalItem = 0;
    UINT32 CalItemIdx = 0;
	RTMP_STRING *pBandIdx  = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: arg = %s\n", __FUNCTION__, arg));

	pBandIdx = strsep(&arg, ":");


	if (pBandIdx == NULL || arg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Invalid parameters\n", __FUNCTION__));
		return FALSE;
	}

	BandIdx = simple_strtoul(pBandIdx, 0, 10);
	CalItem = simple_strtoul(arg, 0, 10);


	if (BandIdx > 1)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Unknown BandIdx = %d\n", __FUNCTION__, BandIdx));
		return FALSE;
	}

    if((CalItem > 12) || (CalItem == 3) || (CalItem == 4))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Unknown CalItem = %d\n", __FUNCTION__, CalItem));
		return FALSE;
	}        

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: BandIdx: %d, CalItem: %d\n", __FUNCTION__, BandIdx, CalItem));

    switch(CalItem){

    case 0:        
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: RC_CAL\n", __FUNCTION__));
        CalItemIdx = RC_CAL;
        break;

    case 1:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: RX_RSSI_DCOC_CAL\n", __FUNCTION__));
        CalItemIdx = RX_RSSI_DCOC_CAL;
        break;        

    case 2:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: RX_DCOC_CAL\n", __FUNCTION__));
        CalItemIdx = RX_DCOC_CAL;
        break;             

    case 5:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: RX_FIIQ_CAL\n", __FUNCTION__));
        CalItemIdx = RX_FIIQ_CAL;
        break;       

    case 6:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: RX_FDIQ_CAL\n", __FUNCTION__));
        CalItemIdx = RX_FDIQ_CAL;
        break;               

    case 7:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: TX_DPD_LINK\n", __FUNCTION__));
        CalItemIdx = TX_DPD_LINK;
        break;   

    case 8:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: TX_LPFG\n", __FUNCTION__));
        CalItemIdx = TX_LPFG;
        break;   

    case 9:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: TX_DCIQC\n", __FUNCTION__));
        CalItemIdx = TX_DCIQC;
        break;   

    case 10:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: TX_IQM\n", __FUNCTION__));
        CalItemIdx = TX_IQM;
        break;           

    case 11:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: TX_PGA\n", __FUNCTION__));
        CalItemIdx = TX_PGA;
        break;         

    case 12:
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: CAL_ALL\n", __FUNCTION__));
        CalItemIdx = CAL_ALL;
        break;
        
    default:  
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(":%s: Unknown CalItem = %d\n", __FUNCTION__, CalItem));
        break;
    }
    
	MtCmdDoCalibration(pAd, RE_CALIBRATION, CalItemIdx, BandIdx);

	return TRUE;
}

INT set_thermal_recal_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 Mode;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,(":%s: arg = %s\n", __FUNCTION__, arg));
	Mode = simple_strtol(arg, 0, 10);
	
	if (Mode > 2)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Unknown Mode = %d (0: thermal recal OFF; 1: thermal recal ON; 2: trigger thermal recal)\n", 
			__FUNCTION__, Mode));
		
		return FALSE;
	}

	if((Mode == 2) && (pAd->CommonCfg.ThermalRecalMode == 0))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Can't trigger recal in Thermal recal off mode\n", __FUNCTION__));
		
		return FALSE;
	}
	
	pAd->CommonCfg.ThermalRecalMode = Mode;

	MtCmdThermalReCalMode(pAd, Mode);
	
	return TRUE;
}

INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 McuDest = 0;
	UINT32 LogType = 0;
	RTMP_STRING *pMcuDest  = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: arg = %s\n", __FUNCTION__, arg));

	pMcuDest = strsep(&arg, ":");


	if (pMcuDest == NULL || arg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Invalid parameters\n", __FUNCTION__));
		return FALSE;
	}

	McuDest = simple_strtoul(pMcuDest, 0, 10);
	LogType = simple_strtoul(arg, 0, 10);


	if (McuDest > 2)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Unknown Mcu Dest = %d, Log Type = %d\n",
				__FUNCTION__, McuDest, LogType));
		return FALSE;
	}


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		(":%s: Mcu Dest = %s, Log Type = %s\n",
		__FUNCTION__, (McuDest == 0 ? "HOST2N9" : "HOST2CR4"),
			(LogType == 0 ? "Disable MCU Log Message" :
			(LogType == 1 ? "Print MCU Log to UART":"Send MCU log by Event"))));

	MtCmdFwLog2Host(pAd, McuDest, LogType);

	return TRUE;
}


INT32 set_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    CHAR *value;
    UCHAR *Input;
    UCHAR ExtendID;
    INT i,len;
    BOOLEAN fgStatus = FALSE;

    /* get ExtID */
    value = rstrtok(Arg,":");
	if (value == NULL)
		return fgStatus;
    AtoH(value, &ExtendID, 1);

    /* get cmd raw data */
    value += 3;
    len = strlen(value)>>1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=======Set_FwCmd==========\n"));


    os_alloc_mem(pAd, (UCHAR **)&Input, len);
    for (i=0; i < len; i++)
    {
        AtoH(value+i*2, &Input[i], 1);
    }

    /* print cmd raw data */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EID= 0x%x, CMD[%d] = ", ExtendID, len));
    for (i=0; i < len; i++)
    {
    	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%x ", Input[i]));
    }
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

    /* send cmd to fw */
    MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_SET);

    os_free_mem((PVOID)Input);
    return fgStatus;
}


INT32 get_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    CHAR *value;
    UCHAR *Input;
    UCHAR ExtendID;
    INT i,len;
    BOOLEAN fgStatus = FALSE;

    /* get ExtID */
    value = rstrtok(Arg,":");
	if (value == NULL)
		return fgStatus;
    AtoH(value, &ExtendID, 1);

    /* get cmd raw data */
    value += 3;
    len = strlen(value)>>1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=======Get_FwCmd==========\n"));

    os_alloc_mem(pAd, (UCHAR **)&Input, len);
    for (i=0; i < len; i++)
    {
        AtoH(value+i*2, &Input[i], 1);
    }

    /* print cmd raw data */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EID= 0x%x, CMD[%d] = ", ExtendID, len));
    for (i=0; i < len; i++)
    {
    	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%x ", Input[i]));
    }
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

    /* send cmd to fw */
    MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_QUERY);

    os_free_mem((PVOID)Input);
    return fgStatus;
}

#ifdef FW_DUMP_SUPPORT
INT set_fwdump_max_size(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    pAd->fw_dump_max_size = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT set_fwdump_path(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    RTMP_OS_FWDUMP_SETPATH(pAd, arg);
	return TRUE;
}

INT fwdump_print(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 x;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: len = %d\n", __func__, pAd->fw_dump_size));
	for (x=0; x<pAd->fw_dump_size; x++)
	{
		if (x % 16 == 0)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%04x : ", x));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", ((unsigned char)pAd->fw_dump_buffer[x])));
		if (x%16 == 15)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	return TRUE;
}
#endif

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
	UINT32 RechkTimer;
    UINT8 RFOffEn;
    CHAR RFOffTh;
	UINT8 ucType;
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
    
	Pos = rtstrstr(Pos, "REC_T-");
	if (Pos != NULL) {
		Pos = Pos + 6;
		RechkTimer = simple_strtol(Pos, 0, 10);
	}
	else
		goto error;

    Pos = rtstrstr(Pos, "RFOFF_en-");
    if (Pos != NULL) {
        Pos = Pos + 9;
        RFOffEn = simple_strtol(Pos, 0, 10);
    }
    else
        goto error;

    Pos = rtstrstr(Pos, "RFOFF_th-");
    if (Pos != NULL) {
        Pos = Pos + 9;
        RFOffTh = simple_strtol(Pos, 0, 10);
    }
    else
        goto error;

	Pos = rtstrstr(Pos, "Type-");
	if (Pos != NULL) {
		Pos = Pos + 5;
		ucType = simple_strtol(Pos, 0, 10);
	}
	else
		goto error;

#ifdef MT_MAC
	AsicThermalProtect(pAd, HighEn, HighTempTh, LowEn, LowTempTh, RechkTimer, RFOffEn, RFOffTh, ucType);
#endif /* MT_MAC */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: high_en=%d, high_thd = %d, low_en = %d, low_thd = %d, rec_timer = %d, RFOFF_en = %d, RFOFF_th = %d, Type = %s\n",
            __FUNCTION__, HighEn, HighTempTh, LowEn, LowTempTh, RechkTimer, RFOffEn, RFOffTh, ucType?"Duty Cycle":"TxStream"));

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("iwpriv ra0 set tpc=h_en-\"value\"_h_th-\"value\"_l_en-\"value\"_l_th-\"value\"_REC_T-\"value\"_RFOFF_en-\"value\"_RFOFF_th-\"value\"\n"));
	return TRUE;
}

INT set_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	UINT32 u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty;
	INT32 i4Ret = 0;

	i4Ret = sscanf (arg, "%d-%d-%d-%d", &u4Lv0Duty, &u4Lv1Duty, &u4Lv2Duty, &u4Lv3Duty);

	if (i4Ret != 4)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("iwpriv ra0 set tpc_duty=Lv0Duty-Lv1Duty-Lv2Duty-Lv3Duty\n"));
		return FALSE;
	}
	else
	{
		if (u4Lv0Duty > 100)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Parameters error! Lv0Duty > 100\n"));
			return FALSE;
		}
		if (u4Lv1Duty > u4Lv0Duty)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Parameters error! Lv1Duty > Lv0Duty"));
			return FALSE;
		}
		if (u4Lv2Duty > u4Lv1Duty)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Parameters error! Lv2Duty > Lv1Duty"));
			return FALSE;
		}
		if (u4Lv3Duty > u4Lv2Duty)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Parameters error! Lv3Duty > Lv2Duty"));
			return FALSE;
		}

		AsicThermalProtectAdmitDuty(pAd, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);
	}

	return TRUE;
}
#endif /* THERMAL_PROTECT_SUPPORT */

#ifdef MT_MAC
VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData)
{
	extern UCHAR tmi_rate_map_ofdm[];
	extern UCHAR tmi_rate_map_cck_lp[];
	extern UCHAR tmi_rate_map_cck_sp[];
	UCHAR phy_mode, rate, bw, preamble, sgi, vht_nss;
	UCHAR phy_idx, bw_idx;
	CHAR *phyMode[6] = {"CCK", "OFDM", "MM", "GF", "VHT", "unknow"};
	CHAR *FecCoding[2] = {"BCC", "LDPC"};
	CHAR *bwMode[4] = {"BW20", "BW40", "BW80", "BW160/8080"};

	phy_mode = (RawData>>13) & 0x7;
	rate = RawData & 0x3F;
	bw = (RawData>>7) & 0x3;
	sgi = (RawData>>9) & 0x1;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	if (phy_mode <= MODE_VHT)
		phy_idx = phy_mode;
	else
		phy_idx = 5;

	if (bw == BW_20)
		bw_idx = 0;
	else if (bw == BW_40)
		bw_idx = 1;
	else if (bw == BW_80)
		bw_idx = 2;
	else if (bw == BW_160)
		bw_idx = 3;

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
			sprintf(Output+strlen(Output), "1M LP, ");
		else if ( rate == TMI_TX_RATE_CCK_2M_LP )
			sprintf(Output+strlen(Output), "2M LP, ");
		else if ( rate == TMI_TX_RATE_CCK_5M_LP )
			sprintf(Output+strlen(Output), "5M LP, ");
		else if ( rate == TMI_TX_RATE_CCK_11M_LP )
			sprintf(Output+strlen(Output), "11M LP, ");
		else if ( rate == TMI_TX_RATE_CCK_2M_SP )
			sprintf(Output+strlen(Output), "2M SP, ");
		else if ( rate == TMI_TX_RATE_CCK_5M_SP )
			sprintf(Output+strlen(Output), "5M SP, ");
		else if ( rate == TMI_TX_RATE_CCK_11M_SP )
			sprintf(Output+strlen(Output), "11M SP, ");
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
	} else if ( phy_mode == MODE_VHT ) {
		vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
		rate = rate & 0xF;
		sprintf(Output+strlen(Output), "NSS%d_MCS%d, ", vht_nss, rate);
	} else {
		sprintf(Output+strlen(Output), "MCS%d, ", rate);
	}

	sprintf(Output+strlen(Output), "%s, ", bwMode[bw_idx]);
	sprintf(Output+strlen(Output), "%cGI, ", sgi ? 'S': 'L');
	sprintf(Output+strlen(Output), "%s%s %s\n",
			phyMode[phy_idx],
			((RawData>>10) & 0x1)? ", STBC": " ",
			FecCoding[((RawData >> 6) & 0x1)]);

}

INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* 0: get temperature; 1: get adc */
	UINT32 value;
	UINT32 Sensor = 0;
	value = simple_strtol(arg, 0, 10);

	if ((value == 0) || (value == 1)) {
		MtCmdGetThermalSensorResult(pAd, value, &Sensor);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: 0: get temperature; 1: get adc\n", __FUNCTION__));

	return TRUE;
}



INT set_manual_rdg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT32   ret = 0;
    UINT32   init, resp, txop, wcid, band;
    MT_RDG_CTRL_T   rdg;
    RTMP_ARCH_OP *arch_op = &pAd->archOps;

    ret = sscanf (arg, "%u-%u-%u-%u-%u",
            &(init), &(resp), &(txop), &(wcid), &(band));
    if (ret != 5)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("Format Error!! should be: iwpriv ra0 set manual_rdg=[init]-[resp]-[txop]-[wcid]-[band]\n"));
    }
    else
    {
        rdg.Txop = txop;
        rdg.Init = init;
        rdg.Resp = resp;
        rdg.WlanIdx = wcid;
        rdg.BandIdx = band;
        rdg.LongNav = (init && resp)?(1):(0);

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("\n>> Initiator=%x, Responder=%x, Txop=0x%x, LongNav=%x, Wcid=%u, BandIdx=%x\n",
                 rdg.Init, rdg.Resp, rdg.Txop, rdg.LongNav, rdg.WlanIdx, rdg.BandIdx));

        if (arch_op->archSetRDG)
        {
            arch_op->archSetRDG(pAd, &rdg);
        }
        else
        {
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    ("[Error], not support arch_op->archSetRdg\n"));
        }
    }

    return TRUE;
}
#endif /* MT_MAC */

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Set_SCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32  SCSEnable;
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR apidx = pObj->ioctl_if;
    UCHAR BandIdx=0;
    struct wifi_dev *wdev;


    if (apidx >= pAd->ApCfg.BssidNum)
    	return FALSE;

    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

    BandIdx = HcGetBandByWdev(wdev);
    SCSEnable = simple_strtol(arg, 0, 10);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, SCSEnable=%d \n", __FUNCTION__, BandIdx, SCSEnable));

    RTMP_CHIP_ASIC_SET_SCS(pAd, BandIdx, SCSEnable);

    return TRUE;
}

INT Set_SCSCfg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR apidx = pObj->ioctl_if;
    UCHAR BandIdx = 0;
    INT32 SCSMinRssiTolerance = 0;
    INT32 SCSThTolerance = 0;
    UINT32 SCSTrafficThreshold = 0;
    UINT32 OfdmPdSupport = 0;
    INT32   Recv=0;
    UINT32 CckUpBond = 0, CckLowBond = 0, OfdmUpBond = 0, OfdmLowBond = 0, CckFixedBond = 0, OfdmFixedBond = 0;
    struct wifi_dev *wdev;


    if (apidx >= pAd->ApCfg.BssidNum)
    	return FALSE;

    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

    BandIdx = HcGetBandByWdev(wdev);



    Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(SCSMinRssiTolerance), &(SCSThTolerance), &(SCSTrafficThreshold), &(OfdmPdSupport), &(CckUpBond), &(CckLowBond), &(OfdmUpBond), &(OfdmLowBond), &(CckFixedBond), &(OfdmFixedBond));
    if (Recv != 10){
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set SCSCfg=[MinRssiTolerance]-[ThTolerance]-[TrafficThreshold]-[OfdmSupport]-[CckUpBoundary]-[CckLowBoundary]-[OfdmUpBoundary]-[OfdmLowBoundary]-[FixedCckBoundary]-[FixedOfdmBoundary]"));
    } else {
        pAd->SCSCtrl.SCSMinRssiTolerance[BandIdx] = SCSMinRssiTolerance;
        pAd->SCSCtrl.SCSThTolerance[BandIdx] = SCSThTolerance;
        pAd->SCSCtrl.SCSTrafficThreshold[BandIdx] = SCSTrafficThreshold;
        pAd->SCSCtrl.OfdmPdSupport[BandIdx] = (UCHAR) OfdmPdSupport;
        pAd->SCSCtrl.CckFalseCcaUpBond[BandIdx] = (UINT16)CckUpBond;
        pAd->SCSCtrl.CckFalseCcaLowBond[BandIdx] = (UINT16)CckLowBond;
        pAd->SCSCtrl.OfdmFalseCcaUpBond[BandIdx] = (UINT16)OfdmUpBond;
        pAd->SCSCtrl.OfdmFalseCcaLowBond[BandIdx] = (UINT16)OfdmLowBond;	
        pAd->SCSCtrl.CckFixedRssiBond[BandIdx] = (INT32)CckFixedBond;	
        pAd->SCSCtrl.OfdmFixedRssiBond[BandIdx] = (INT32)OfdmFixedBond;
    }
    //MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, SCSMinRssiTolerance=%d
    //    , SCSThTolerance=%d, SCSTrafficThreshold=%d\n", __FUNCTION__,
    //    BandIdx, SCSMinRssiTolerance, SCSThTolerance, SCSTrafficThreshold));

    return TRUE;
}

INT Set_SCSPd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR apidx = pObj->ioctl_if;
    UCHAR BandIdx = 0;
	INT32	CckPdBlkTh = 0;
	INT32	OfdmPdBlkTh = 0;
    INT32   Recv=0;
	UINT32 CrValue;
    struct wifi_dev *wdev;
    if (apidx >= pAd->ApCfg.BssidNum)
    	return FALSE;
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    BandIdx = HcGetBandByWdev(wdev);


    Recv = sscanf(arg, "%d-%d", &(CckPdBlkTh), &(OfdmPdBlkTh));
    if (Recv != 2 || ( CckPdBlkTh < 30 || CckPdBlkTh > 110) || ( OfdmPdBlkTh < 30 || OfdmPdBlkTh > 98)  ){
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error or Out of range \n"));
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set SCSCfg=[CckPdBlkTh]-[OfdmPdBlkTh]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CckPdBlkTh  Range: 30~110 dBm (Represents a negative number) \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("OfdmPdBlkTh Range: 30~98  dBm (Represents a negative number) \n"));
    } else {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_MANUAL;
		pAd->SCSCtrl.CckPdBlkTh[BandIdx] = ((CckPdBlkTh * (-1)) + 256);
		pAd->SCSCtrl.OfdmPdBlkTh[BandIdx] = ((OfdmPdBlkTh * (-2)) + 512);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Band%d CckPdBlkTh  = -%ddBm (%d)\n",
			BandIdx, CckPdBlkTh, pAd->SCSCtrl.CckPdBlkTh[BandIdx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Band%d OfdmPdBlkTh = -%ddBm (%d)\n",
			BandIdx, OfdmPdBlkTh, pAd->SCSCtrl.OfdmPdBlkTh[BandIdx]));


	    HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
        CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
        CrValue |= (pAd->SCSCtrl.OfdmPdBlkTh[BandIdx] <<PdBlkOfmdThOffset);
        HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);

        HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
        CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
        CrValue |= (pAd->SCSCtrl.CckPdBlkTh[BandIdx] <<PdBlkCckThOffset);
        HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);

        HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
        CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
        CrValue |= (pAd->SCSCtrl.CckPdBlkTh[BandIdx] << PdBlkCck1RThOffset);
        HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);

    }

    return TRUE;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

INT SetSKUCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8	i;
	CHAR	*value = 0;
	UCHAR 	TxPowerSKUEn = 0;
	INT     status = TRUE;
    UINT8   BandIdx = 0;
    struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
    /* obtain Band index */
    if (apidx >= pAd->ApCfg.BssidNum)
       return FALSE;
   
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
        

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx)); 
        
    /* sanity check for Band index */
    if (BandIdx >= DBDC_BAND_NUM)
        return FALSE;

#ifdef RF_LOCKDOWN
    /* Check RF lock Status */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config SKU status!! \n", __FUNCTION__));
        return TRUE;
    }
#endif /* RF_LOCKDOWN */

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */    
	for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0:
				TxPowerSKUEn = simple_strtol(value, 0, 10);
				break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

    /* sanity check for input parameter */
    if((TxPowerSKUEn != FALSE) && (TxPowerSKUEn != TRUE))
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!! \n", __FUNCTION__));
        return FALSE;
    }

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerSKUEn = %d \n", __FUNCTION__, TxPowerSKUEn));

    /* Update Profile Info for SKU */
#ifdef SINGLE_SKU_V2
	pAd->CommonCfg.SKUenable[BandIdx] = TxPowerSKUEn;
#endif /* SINGLE_SKU_V2 */
	
    return TxPowerSKUCtrl(pAd, TxPowerSKUEn, BandIdx);
}

INT SetPercentageCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8	i;
	CHAR	*value = 0;
	UCHAR 	TxPowerPercentEn = 0;
	INT     status = TRUE;
    UINT8   BandIdx = 0;
    struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
    /* obtain Band index */
    if (apidx >= pAd->ApCfg.BssidNum)
       return FALSE;
   
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
        

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx)); 
        
    /* sanity check for Band index */
    if (BandIdx >= DBDC_BAND_NUM)
        return FALSE;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */        
	for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0:
				TxPowerPercentEn = simple_strtol(value, 0, 10);
				break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

    /* sanity check for input parameter */
    if((TxPowerPercentEn != FALSE) && (TxPowerPercentEn != TRUE))
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!! \n", __FUNCTION__));
        return FALSE;
    }

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerPercentEn = %d \n", __FUNCTION__, TxPowerPercentEn));

    /* Update Profile Info for Power Percentage */
    pAd->CommonCfg.PERCENTAGEenable[BandIdx] = TxPowerPercentEn;

    return TxPowerPercentCtrl(pAd, TxPowerPercentEn, BandIdx);
}

INT SetPowerDropCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR	*value = 0;
	UINT8 	ucPowerDrop = 0;
    UINT8   BandIdx = 0;
    struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
    /* obtain Band index */
    if (apidx >= pAd->ApCfg.BssidNum)
       return FALSE;
   
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
        

    if (BandIdx > DBDC_BAND_NUM)
        return FALSE;

    /* sanity check for input parameter format */
    if (!arg)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */ 
    ucPowerDrop = simple_strtol(value, 0, 10);
    
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucPowerDrop = %d \n", __FUNCTION__, ucPowerDrop));

    /* Update Profile Info for Power Percentage */
    pAd->CommonCfg.TxPowerPercentage[BandIdx] = (ULONG)ucPowerDrop;

    return TxPowerDropCtrl(pAd, ucPowerDrop, BandIdx);
}

INT SetBfBackoffCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8   i;
    CHAR    *value = 0;
    UCHAR   TxBFBackoffEn = 0;
    INT     status = TRUE;
    UINT8   BandIdx = 0;
    struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
    /* obtain Band index */
    if (apidx >= pAd->ApCfg.BssidNum)
       return FALSE;
   
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
        

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx)); 
        
    /* sanity check for Band index */
    if (BandIdx >= DBDC_BAND_NUM)
        return FALSE;

#ifdef RF_LOCKDOWN
    /* Check RF lock Status */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config BF Backoff status!! \n", __FUNCTION__));
        return TRUE;
    }
#endif /* RF_LOCKDOWN */

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */    
    for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
    {
        switch (i)
        {
            case 0:
                TxBFBackoffEn = simple_strtol(value, 0, 10);
                break;
            default:
            {
                status = FALSE;
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;
            }   
        }
    }

    /* sanity check for input parameter */
    if((TxBFBackoffEn != FALSE) && (TxBFBackoffEn != TRUE))
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!! \n", __FUNCTION__));
        return FALSE;
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxBFBackoffEn = %d \n", __FUNCTION__, TxBFBackoffEn));

    /* Update Profile Info for Power Percentage */
    pAd->CommonCfg.BFBACKOFFenable[BandIdx] = TxBFBackoffEn;
    
    return TxPowerBfBackoffCtrl(pAd, TxBFBackoffEn, BandIdx);
}

INT SetThermoCompCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR    *value = 0;
    BOOLEAN fgThermoCompEn = 0;
    UINT8   BandIdx = 0;

    BandIdx = MTGetBandIdxByIf(pAd);

    if (BandIdx > BAND1)
        return FALSE;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */    
    fgThermoCompEn = simple_strtol(value, 0, 10);
    
    /* sanity check for input parameter */
    if((fgThermoCompEn != FALSE) && (fgThermoCompEn != TRUE))
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!! \n", __FUNCTION__));
        return FALSE;
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgThermoCompEn = %d \n", __FUNCTION__, fgThermoCompEn));
    
    return ThermoCompCtrl(pAd, fgThermoCompEn, BandIdx);
}

INT SetRfTxAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR	*value = 0;
    UINT8   ucTxAntIdx;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 2)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format with 2-digit. \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
    ucTxAntIdx = simple_strtol(value, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxAntIdx: 0x%x \n", __FUNCTION__, ucTxAntIdx));
	
    return TxPowerRfTxAnt(pAd, ucTxAntIdx);
}

INT SetTxPowerInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8	i;
	CHAR	*value = 0;
    UCHAR   TxPowerInfoEn = 0;
	INT     status = TRUE;
    UINT8   BandIdx = 0;
    
    BandIdx = MTGetBandIdxByIf(pAd);

    if (BandIdx > BAND1)
        return FALSE;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
            case 0:
                TxPowerInfoEn = simple_strtol(value, 0, 10);
                break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

    /* sanity check for input parameter */
    if((TxPowerInfoEn != FALSE) && (TxPowerInfoEn != TRUE))
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!! \n", __FUNCTION__));
        return FALSE;
    }

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerInfoEn = %d \n", __FUNCTION__, TxPowerInfoEn));
	
    return TxPowerShowInfo(pAd, TxPowerInfoEn, BandIdx);
}

INT SetTOAECtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8	i;
	CHAR	*value = 0;
    UCHAR   TOAECtrl = 0;
	INT     status = TRUE;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
            case 0:
                TOAECtrl = simple_strtol(value, 0, 10);
                break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

    /* sanity check for input parameter */
    if((TOAECtrl != FALSE) && (TOAECtrl != TRUE))
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!! \n", __FUNCTION__));
        return FALSE;
    }

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TOAECtrl = %d \n", __FUNCTION__, TOAECtrl));
	
    return TOAECtrlCmd(pAd, TOAECtrl);
}

INT SetEDCCACtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8	    i;
	CHAR	    *value = 0;
    UCHAR       BandIdx = 0;
    UCHAR       EDCCACtrl = 0;
	INT         status = TRUE; 

#ifdef CONFIG_AP_SUPPORT
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
    struct wifi_dev *wdev;
    
    /* obtain Band index */
    if (apidx >= pAd->ApCfg.BssidNum)
        return FALSE;
    
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    BandIdx = HcGetBandByWdev(wdev);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx)); 
#endif /* CONFIG_AP_SUPPORT */
	
	/* sanity check for Band index */
    if (BandIdx >= DBDC_BAND_NUM)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: BandIdx = %d. Improper Band Index. \n", __FUNCTION__, BandIdx));
        return FALSE;
    }
	
    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
            case 0:
                EDCCACtrl = simple_strtol(value, 0, 10);
                break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

    /* sanity check for input parameter */
    if((EDCCACtrl != FALSE) && (EDCCACtrl != TRUE))
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!! \n", __FUNCTION__));
        return FALSE;
    }

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: EDCCACtrl = %d \n", __FUNCTION__, EDCCACtrl));

    pAd->CommonCfg.ucEDCCACtrl[BandIdx] = EDCCACtrl;
    status = EDCCACtrlCmd(pAd, BandIdx, pAd->CommonCfg.ucEDCCACtrl[BandIdx]);

    return status;
}

INT ShowEDCCAStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: EDCCACtrl[Band0] = %d \n", __FUNCTION__, pAd->CommonCfg.ucEDCCACtrl[DBDC_BAND0]));
#ifdef DBDC_MODE
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: EDCCACtrl[Band1] = %d \n", __FUNCTION__, pAd->CommonCfg.ucEDCCACtrl[DBDC_BAND1]));
#endif /*DBDC_MODE*/
    return status;
}

INT SetSKUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT       status = TRUE;
    
#ifdef SINGLE_SKU_V2	
	UINT8     i;
    CH_POWER  *ch, *ch_temp;


    MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("SKU table index: %d \n", pAd->CommonCfg.SKUTableIdx));

	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("start ch = %d, ch->num = %d\n", ch->StartChannel, ch->num));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band: %d \n", ch->band));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel: "));

        for ( i = 0 ; i < ch->num ; i++ )
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->Channel[i]));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CCK: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_CCK_LENGTH ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrCCK[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("OFDM: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_OFDM_LENGTH ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrOFDM[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT20: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d ", ch->PwrVHT20[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT40: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrVHT40[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT80: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrVHT80[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT160: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_VHT_LENGTH ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrVHT160[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PwrTxStreamDelta: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrTxStreamDelta[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PwrTxNSSDelta: "));

        for ( i= 0 ; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM ; i++ )
		{
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrTxNSSDelta[i]));
		}

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
	}
#endif /* SINGLE_SKU_V2 */

    return status;
}

INT SetBFBackoffInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT           status = TRUE;
    
#ifdef SINGLE_SKU_V2	
	UINT8         i;
    BFback_POWER  *ch, *ch_temp;

    MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("BF Backof table index: %d \n", pAd->CommonCfg.SKUTableIdx));

    DlListForEachSafe(ch, ch_temp, &pAd->BFBackoffList, BFback_POWER, List)
    {
        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("start ch = %d, ch->num = %d\n", ch->StartChannel, ch->num));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band: %d \n", ch->band));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel: "));

        for ( i = 0 ; i < ch->num ; i++ )
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->Channel[i]));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Max Power: "));

        for ( i= 0 ; i < 3 ; i++ )
        {
            MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%d ", ch->PwrMax[i]));
        }

        MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
    }
#endif /* SINGLE_SKU_V2 */
	
    return status;
}


INT SetCNUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG enable;

	enable = simple_strtol(arg, 0, 10);

#ifdef MT7615
    if(IS_MT7615(pAd))
    {
        UINT8 idx;
        UINT32 value;

        if (enable)
        {
            pAd->cn_cnt = 0;
            for (idx=0; idx < 10; idx++)
            {
                pAd->rxv2_cyc3[idx] = 0xFFFFFFFF;
            }
            RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x66c, &value);
            value |= 0xD << 4;
            RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x66c, value);

            RTMP_IO_READ32(pAd, ARB_RQCR, &value);
            value |= ARB_RQCR_RXV_R_EN;
            if (pAd->CommonCfg.dbdc_mode)
            {
                value |= ARB_RQCR_RXV1_R_EN;
            }
            RTMP_IO_WRITE32(pAd, ARB_RQCR, value);
        }
        else
        {
            RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x66c, &value);
            value &= ~0xF0;
            RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x66c, value);

            RTMP_IO_READ32(pAd, ARB_RQCR, &value);
            value &= ~ARB_RQCR_RXV_R_EN;
            if (pAd->CommonCfg.dbdc_mode)
            {
                value &= ~ARB_RQCR_RXV1_R_EN;
            }
            RTMP_IO_WRITE32(pAd, ARB_RQCR, value);
        }
    }
#endif /* MT7615 */

    return TRUE;
}


#ifdef LINK_TEST_SUPPORT
INT SetLinkTestRxParamCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8  i;
	CHAR   *value;

    /* sanity check for input parameter format */
    if (!arg)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 23)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++)
	{
        switch (i)
		{
            case 0:
                pAd->cLargePowerTh = simple_strtol(value, 0, 10);      // 2 symbol representation
                break;

            case 1:
                pAd->ucHighPowerRssiTh = simple_strtol(value, 0, 10);  // 2 symbol representation
                break;

            case 2:
                pAd->ucLowPowerRssiTh = simple_strtol(value, 0, 10);   // 2 symbol representation
                break;    

            case 3:
                pAd->ucRxCountTh = simple_strtol(value, 0, 10);        // 2 symbol representation
                break;

            case 4:
                pAd->ucTimeOutTh = simple_strtol(value, 0, 10);        // 3 symbol representation
                break;

            case 5:
                pAd->cNrRssiTh = simple_strtol(value, 0, 10);          // 3 symbol representation
                break;

            case 6:
                pAd->cChgTestPathTh = simple_strtol(value, 0, 10);     // 3 symbol representation
                break;
                
			default:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __FUNCTION__));
				return FALSE;
			}	
		}
	}

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: cLargePowerTh: %d, ucHighPowerRssiTh: %d, ucLowPowerRssiTh: %d\n", __FUNCTION__,
                                                                pAd->cLargePowerTh, pAd->ucHighPowerRssiTh, pAd->ucLowPowerRssiTh));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucRxCountTh: %d, ucTimeOutTh: %d\n", __FUNCTION__,
                                                                pAd->ucRxCountTh, pAd->ucTimeOutTh));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: cNrRssiTh: %d, cChgTestPathTh: %d\n", __FUNCTION__,
                                                                pAd->cNrRssiTh, pAd->cChgTestPathTh));
    
    return TRUE;
}

INT SetLinkTestModeCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8  i;
    CHAR   *value;

    /* sanity check for input parameter format */
    if (!arg)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
    for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++)
    {
        switch (i)
        {
            case 0:
                pAd->fgTxSpurEn = simple_strtol(value, 0, 10);
                break;
            case 1:
                pAd->fgNrFloatingEn = simple_strtol(value, 0, 10);
                break;    
            case 2:
                pAd->fgACREn = simple_strtol(value, 0, 10);
                break;    
            case 3:
                pAd->cMaxInRssiTh = simple_strtol(value, 0, 10);
                break;
            case 4:
                pAd->ucACRConfidenceCntTh = simple_strtol(value, 0, 10);
                break;
            case 5:
                pAd->ucMaxInConfidenceCntTh = simple_strtol(value, 0, 10);
                break;
                
            default:
            {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __FUNCTION__));
                return FALSE;
            }   
        }
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgTxSpurEn: %d, fgNrFloatingEn: %d, fgACREn: %d\n", __FUNCTION__,
                                                            pAd->fgTxSpurEn, pAd->fgNrFloatingEn, pAd->fgACREn));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: cMaxInRssiTh: %d, ucACRConfidenceCntTh: %d, ucMaxInConfidenceCntTh: %d \n", __FUNCTION__,
                                                            pAd->cMaxInRssiTh, pAd->ucACRConfidenceCntTh, pAd->ucMaxInConfidenceCntTh));
    
    return TRUE;
}

INT SetLinkTestPowerUpTblCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;
	UINT8  ucTxPwrUpCat = 0;
	UINT8  ucTxPwrUpRate = 0;
	UINT8  ucTxPwrUpValue = 0;

	/* sanity check for input parameter format */
	if (!arg)
	{	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++)
	{
		switch (i)
		{
			case 0:
				ucTxPwrUpCat = simple_strtol(value, 0, 10);
				break;
			case 1:
				ucTxPwrUpRate = simple_strtol(value, 0, 10);
				break;	  
			case 2:
				ucTxPwrUpValue = simple_strtol(value, 0, 10);
				break;	  
			default:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __FUNCTION__));
				return FALSE;
			}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxPwrUpCat: %d, ucTxPwrUpRate: %d, ucTxPwrUpValue: %d\n", __FUNCTION__,
															ucTxPwrUpCat, ucTxPwrUpRate, ucTxPwrUpValue));

	/* update Tx Power Up Table */
	pAd->ucTxPwrUpTbl[ucTxPwrUpCat][ucTxPwrUpRate] = ucTxPwrUpValue;

	/* sync Tx Power up table to Firmware */
	MtCmdLinkTestTxPwrUpTblCtrl(pAd, ucTxPwrUpCat, pAd->ucTxPwrUpTbl[ucTxPwrUpCat]);
	
	return TRUE;
}

INT SetLinkTestPowerUpTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  ucRateIdx, ucCatIdx;
	CHAR   cRateInfo[CMW_POWER_UP_RATE_NUM][12] = {"CCK_1M2M", "CCK_5M11M", "OFDM_6M9M", "OFDM_12M18M", "OFDM_24M36M", "OFDM_48M", "OFDM_54M",
							                       "HT20_MCS0", "HT20_MCS12", "HT20_MCS34", "HT20_MCS5", "HT20_MCS6", "HT20_MCS7"};

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                          Link Test Power Up Table                              \n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("              2G(S)     5G(S)     2G(D)     5G(D)\n"));

	for (ucRateIdx = 0; ucRateIdx < CMW_POWER_UP_RATE_NUM; ucRateIdx++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:  ", cRateInfo[ucRateIdx]));
		for (ucCatIdx = 0; ucCatIdx < CMW_POWER_UP_CATEGORY_NUM; ucCatIdx++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   %d", pAd->ucTxPwrUpTbl[ucCatIdx][ucRateIdx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

    return TRUE;
}

INT SetLinkTestInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 u4value;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                                 Link Status                                    \n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n\n"));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Link Up Done: %d \n\n", pAd->fgCmwLinkDone));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ChannelBand: %dG(Band0) %dG(Band1)\n\n", pAd->ucCmwChannelBand[BAND0]? 2 : 5, pAd->ucCmwChannelBand[BAND1]? 2 : 5));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CSD State: %d \n\n", pAd->ucTxCsdState));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Power Up State: %d \n\n", pAd->ucTxPwrBoostState));
    
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxStream Mode: %d \n\n", pAd->ucRxStreamState));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx Filter State: %d \n\n", pAd->ucRxFilterstate));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ACR Confidence count and Threshoild: (%d-%d) \n\n", pAd->ucRxFilterConfidenceCnt, pAd->ucACRConfidenceCntTh));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxIn Confidence count and Threshoild: (%d-%d) \n\n", pAd->ucRxFilterConfidenceCnt, pAd->ucMaxInConfidenceCntTh));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                               Tx Antenna Status                                \n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n\n"));

    /* Read RF CR */
    MtCmdRFRegAccessRead(pAd, (UINT32)WF0, (UINT32)0x48, (UINT32 *)&u4value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n\n", WF0, 0x48, u4value));

    MtCmdRFRegAccessRead(pAd, (UINT32)WF1, (UINT32)0x48, (UINT32 *)&u4value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n\n", WF1, 0x48, u4value));

    MtCmdRFRegAccessRead(pAd, (UINT32)WF2, (UINT32)0x48, (UINT32 *)&u4value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n\n", WF2, 0x48, u4value));

    MtCmdRFRegAccessRead(pAd, (UINT32)WF3, (UINT32)0x48, (UINT32 *)&u4value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n\n", WF3, 0x48, u4value));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));

    return TRUE;
}
#endif /* LINK_TEST_SUPPORT */

INT SetMUTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT      status = TRUE;
    UINT8    i;
	CHAR     *value = 0;
    BOOLEAN  MUPowerForce = FALSE;
    UCHAR    MUPower = 0;
	UINT8    BandIdx = 0;
    struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
    /* obtain Band index */
    if (apidx >= pAd->ApCfg.BssidNum)
       return FALSE;
   
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
        

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx)); 
        
    /* sanity check for Band index */
    if (BandIdx >= DBDC_BAND_NUM)
        return FALSE;
    
    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
    for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
    {
        switch (i)
        {
            case 0:
                MUPowerForce = simple_strtol(value, 0, 10);
                break;
            case 1:
                MUPower = simple_strtol(value, 0, 10);
                break;    
            default:
            {
                status = FALSE;
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;
            }   
        }
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Force: %d, MUPower: %d \n", __FUNCTION__, MUPowerForce, MUPower));

    return MUPowerCtrlCmd(pAd, MUPowerForce, MUPower, BandIdx);
}

INT SetBFNDPATxDCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT      status = TRUE;
    UINT8    i;
	CHAR     *value = 0;
    BOOLEAN  fgNDPA_ManualMode = FALSE;
    UINT8    ucNDPA_TxMode = 0;
    UINT8    ucNDPA_Rate = 0;
    UINT8    ucNDPA_BW = 0;
    UINT8    ucNDPA_PowerOffset = 0;
    
    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
    for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
    {
        switch (i)
        {
            case 0:
                fgNDPA_ManualMode = simple_strtol(value, 0, 10);
                break;
            case 1:
                ucNDPA_TxMode = simple_strtol(value, 0, 10);
                break;
            case 2:
                ucNDPA_Rate = simple_strtol(value, 0, 10);
                break;
            case 3:
                ucNDPA_BW = simple_strtol(value, 0, 10);
                break;
            case 4:
                ucNDPA_PowerOffset = simple_strtol(value, 0, 10); // negative value need to use 2's complement
                break;
            default:
            {
                status = FALSE;
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;
            }   
        }
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgNDPA_ManualMode = %d, ucNDPA_TxMode = %d, ucNDPA_Rate = %d, ucNDPA_BW = %d, ucNDPA_PowerOffset = %d \n", __FUNCTION__, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset));

    return BFNDPATxDCtrlCmd(pAd, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset);
}

INT SetTxPowerCompInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT  status = TRUE;
        
#ifdef SINGLE_SKU_V2	
    UINT8  ucPowerTableIdx;
    CHAR   STRING[SKU_TABLE_SIZE][15] =
           {"CCK_1M2M   ",
            "CCK5M11M   ",
            "OFDM6M9M   ",
            "OFDM12M18M ",
            "OFDM24M36M ",
            "OFDM48M    ",
            "OFDM54M    ",
            "HT20M0     ",
            "HT20M32    ",
            "HT20M1M2   ",
            "HT20M3M4   ",
            "HT20M5     ",
            "HT20M6     ",
            "HT20M7     ",
            "HT40M0     ",
            "HT40M32    ",
            "HT40M1M2   ",
            "HT40M3M4   ",
            "HT40M5     ",
            "HT40M6     ",
            "HT40M7     ",
            "VHT20M0    ",
            "VHT20M1M2  ",
            "VHT20M1M2  ",
            "VHT20M5M6  ",
            "VHT20M7    ",
            "VHT20M8    ",
            "VHT20M9    ",
            "VHT40M0    ",
            "VHT40M1M2  ",
            "VHT40M3M4  ",
            "VHT40M5M6  ",
            "VHT40M7    ",
            "VHT40M8    ",
            "VHT40M9    ",
            "VHT80M0    ",
            "VHT80M1M2  ",
            "VHT80M3M4  ",
            "VHT80M5M6  ",
            "VHT80M7    ",
            "VHT80M8    ",
            "VHT80M9    ",
            "VHT160M0   ",
            "VHT160M1M2 ",
            "VHT160M3M4 ",
            "VHT160M5M6 ",
            "VHT160M7   ",
            "VHT160M8   ",
            "VHT160M9   "};

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("============================================================================= \n"));
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("                       Tx Power Compenstation Info                            \n"));
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("============================================================================= \n"));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,(KGRN "         Band0        (1SS,2SS,3SS,4SS)                                  \n" KNRM));

    for (ucPowerTableIdx = 0; ucPowerTableIdx < SKU_TABLE_SIZE; ucPowerTableIdx++)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("         %s : %3d, %3d, %3d, %3d                                          \n",
            &STRING[ucPowerTableIdx][0],
            pAd->CommonCfg.cTxPowerCompBackup[BAND0][ucPowerTableIdx][ATE_1_TX_STREAM], 
            pAd->CommonCfg.cTxPowerCompBackup[BAND0][ucPowerTableIdx][ATE_2_TX_STREAM], 
            pAd->CommonCfg.cTxPowerCompBackup[BAND0][ucPowerTableIdx][ATE_3_TX_STREAM], 
            pAd->CommonCfg.cTxPowerCompBackup[BAND0][ucPowerTableIdx][ATE_4_TX_STREAM]));
    }

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("----------------------------------------------------------------------------- \n"));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,(KGRN "         Band1         (1SS,2SS,3SS,4SS)                                 \n" KNRM));

    for (ucPowerTableIdx = 0; ucPowerTableIdx < SKU_TABLE_SIZE; ucPowerTableIdx++)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("         %s : %3d, %3d, %3d, %3d                                          \n", 
            &STRING[ucPowerTableIdx][0],
            pAd->CommonCfg.cTxPowerCompBackup[BAND1][ucPowerTableIdx][ATE_1_TX_STREAM],
            pAd->CommonCfg.cTxPowerCompBackup[BAND1][ucPowerTableIdx][ATE_2_TX_STREAM],
            pAd->CommonCfg.cTxPowerCompBackup[BAND1][ucPowerTableIdx][ATE_3_TX_STREAM],
            pAd->CommonCfg.cTxPowerCompBackup[BAND1][ucPowerTableIdx][ATE_4_TX_STREAM]));
    }

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("----------------------------------------------------------------------------- \n"));

#endif /* SINGLE_SKU_V2 */
        
    return status;
}

INT SetThermalManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT8	i;
	CHAR	*value = 0;
    BOOLEAN fgManualMode = FALSE;
	CHAR 	cTemperature = 0;
	INT     status = TRUE;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */	
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
            case 0:
                fgManualMode = simple_strtol(value, 0, 10);
                break;
            case 1:
				cTemperature = simple_strtol(value, 0, 10);
				break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgManualMode: %d, cTemperature: %d \n", __FUNCTION__, fgManualMode, cTemperature));
	
    return TemperatureCtrl(pAd, fgManualMode, cTemperature);
}

#ifdef TX_POWER_CONTROL_SUPPORT
INT SetTxPowerBoostCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	INT 	status = TRUE;
	CHAR	cPwrUpCat = 0;
	CHAR	cPwrUpValue[7] = {0, 0, 0, 0, 0, 0, 0};

	UINT8	ucBandIdx = 0;
	struct	wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
	   return FALSE;
   
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
		
		
	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg)
	{	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0:
				cPwrUpCat = simple_strtol(value, 0, 10);
				break;

			case 1:
				cPwrUpValue[0] = simple_strtol(value, 0, 10);
				break;

			case 2:
				cPwrUpValue[1] = simple_strtol(value, 0, 10);
				break;

			case 3:
				cPwrUpValue[2] = simple_strtol(value, 0, 10);
				break;

			case 4:
				cPwrUpValue[3] = simple_strtol(value, 0, 10);
				break;

			case 5:
				cPwrUpValue[4] = simple_strtol(value, 0, 10);
				break;

			case 6:
				cPwrUpValue[5] = simple_strtol(value, 0, 10);
				break;

			case 7:
				cPwrUpValue[6] = simple_strtol(value, 0, 10);
				break;

			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx: %d, cPwrUpCat: %d\n", __FUNCTION__, ucBandIdx, cPwrUpCat));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: cPwrUpValue: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n", __FUNCTION__, cPwrUpValue[0], cPwrUpValue[1], cPwrUpValue[2], cPwrUpValue[3], cPwrUpValue[4], cPwrUpValue[5], cPwrUpValue[6]));

	/* update power up table structure */
	switch (cPwrUpCat)
	{
		case POWER_UP_CATE_CCK_OFDM:
			os_move_mem(pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx], cPwrUpValue, sizeof(CHAR)* POWER_UP_CATEGORY_RATE_NUM);
			break;
			
		case POWER_UP_CATE_HT20:
			os_move_mem(pAd->CommonCfg.cPowerUpHt20[ucBandIdx], cPwrUpValue, sizeof(CHAR)* POWER_UP_CATEGORY_RATE_NUM);
			break;

		case POWER_UP_CATE_HT40:
			os_move_mem(pAd->CommonCfg.cPowerUpHt40[ucBandIdx], cPwrUpValue, sizeof(CHAR)* POWER_UP_CATEGORY_RATE_NUM);
			break;

		case POWER_UP_CATE_VHT20:
			os_move_mem(pAd->CommonCfg.cPowerUpVht20[ucBandIdx], cPwrUpValue, sizeof(CHAR)* POWER_UP_CATEGORY_RATE_NUM);
			break;

		case POWER_UP_CATE_VHT40:
			os_move_mem(pAd->CommonCfg.cPowerUpVht40[ucBandIdx], cPwrUpValue, sizeof(CHAR)* POWER_UP_CATEGORY_RATE_NUM);
			break;

		case POWER_UP_CATE_VHT80:
			os_move_mem(pAd->CommonCfg.cPowerUpVht80[ucBandIdx], cPwrUpValue, sizeof(CHAR)* POWER_UP_CATEGORY_RATE_NUM);
			break;

		case POWER_UP_CATE_VHT160:
			os_move_mem(pAd->CommonCfg.cPowerUpVht160[ucBandIdx], cPwrUpValue, sizeof(CHAR)* POWER_UP_CATEGORY_RATE_NUM);
			break;

		default:
			break;
	}
	
	return TxPwrUpCtrl(pAd, ucBandIdx, cPwrUpCat, cPwrUpValue);
}
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef RF_LOCKDOWN
INT SetCalFreeApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT     status = TRUE;
    UINT8   i;
    CHAR    *value = 0;
    UCHAR   CalFreeApply = 0;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
	for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
            case 0:
                CalFreeApply = simple_strtol(value, 0, 10);
                break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: CalFreeApply = %d \n", __FUNCTION__, CalFreeApply));

    /* Configure to Global pAd structure */
    pAd->fgCalFreeApply = CalFreeApply;
    
    return status;
}

INT SetWriteEffuseRFpara(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT status = TRUE;

    UCHAR   block[EFUSE_BLOCK_SIZE]="";
    USHORT  length = pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE;
    UCHAR   *ptr = pAd->EEPROMImage;
    UCHAR   index, i;
    USHORT  offset = 0;
    UINT    isVaild = 0;
    BOOL    NeedWrite;
    BOOL    WriteStatus;
    
    /* Only Write to Effuse when RF is not lock down */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == FALSE)) 
    {
        /* Write to Effuse block by block */
        for (offset = 0; offset < length; offset += EFUSE_BLOCK_SIZE)
        {
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("offset 0x%04x: \n", offset));
            NeedWrite = FALSE;
            MtCmdEfuseAccessRead(pAd,offset,&block[0],&isVaild);

            /* Check the Needed contents are different and update the buffer content for write back to Effuse */
            for (index = 0; index < EFUSE_BLOCK_SIZE; index++)
            {
                /* Obtain the status of this effuse column need to write or not */
                WriteStatus = pAd->chipOps.write_RF_lock_parameter(pAd, offset + index);
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Effuse[0x%04x]: Write(%d) \n", offset + index, WriteStatus));

                if ((block[index] != ptr[index]) && (WriteStatus == TRUE))
                    NeedWrite = TRUE;
                else
                    continue;
                
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("index 0x%04x: ", offset + index));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("orignal block value=0x%04x, write value=0x%04x\n", block[index], ptr[index]));

                if (WriteStatus == TRUE)
                    block[index] = ptr[index];
            }

            /* RF Lock Protection */
            if (offset == RF_LOCKDOWN_EEPROME_BLOCK_OFFSET)
            {
                block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] |= RF_LOCKDOWN_EEPROME_BIT;
                NeedWrite = TRUE;
            }

            /* Only write to Effuse when Needed contents are different in Effuse and Flash */
            if (NeedWrite == TRUE)
            {
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("write block content: "));
                for (i=0; i < EFUSE_BLOCK_SIZE; i++)
                    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%x ", (UINT)block[i]));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
                MtCmdEfuseAccessWrite(pAd,offset,&block[0]);
            }

            ptr += EFUSE_BLOCK_SIZE;
        }
    }
    else
    {
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RF is lock now. Cannot write back to Effuse!! \n"));
    }
    
    return status;
}

INT SetRFBackup(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT     status = TRUE;
    CHAR    *value = 0;
    UCHAR   Param  = 0;
    UCHAR   block[EFUSE_BLOCK_SIZE]="";
    UCHAR   i;
    USHORT  offset = RF_LOCKDOWN_EEPROME_BLOCK_OFFSET;
    UINT    isVaild = 0;

    /* sanity check for input parameter format */
    if (arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    /* parameter parsing */
    for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
    {
        switch (i)
        {
            case 0:
                Param = simple_strtol(value, 0, 10);
                break;
            default:
            {
                status = FALSE;
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;
            }   
        }
    }

    if (Param == RF_VALIDATION_NUMBER)
    {
        /* Check RF lock down status */
        if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
        {
            /* Read Effuse Contents of Block Address 0x120 */
            MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);

            /* Configue Block 0x12C Content (Unlock RF lock) */
            block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] &= (~(RF_LOCKDOWN_EEPROME_BIT));

            /* Write to Effuse */
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("write block content: "));
            for (i=0; i < EFUSE_BLOCK_SIZE; i++)
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%x ", (UINT)block[i]));
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
            MtCmdEfuseAccessWrite(pAd, offset, &block[0]);
        }
        else
        {
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("No need to unlock!! \n"));
        }
    }

    return status;
}
#endif /* RF_LOCKDOWN */

INT set_hnat_register(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 reg_en;
    INT idx;
    struct wifi_dev *wdev;

    reg_en = simple_strtol(arg, 0, 10);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Device Instance\n"));
    for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWDEV %02d:", idx));
        if (pAd->wdev_list[idx])
        {
            wdev = pAd->wdev_list[idx];
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t\tName:%s\n",
                        RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev)));
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWdev(list) Idx:%d\n", wdev->wdev_idx));
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t Idx:%d\n", RtmpOsGetNetIfIndex(wdev->if_dev)));
#if !defined(CONFIG_RA_NAT_NONE)
#if defined (CONFIG_RA_HW_NAT_WIFI_NEW_ARCH)
        	if (ppe_dev_unregister_hook != NULL && 
                ppe_dev_register_hook != NULL)
	        {
	            if (reg_en) {
                    ppe_dev_register_hook(wdev->if_dev);
                } 
                else {
                    ppe_dev_unregister_hook(wdev->if_dev);
                }
	        }
#endif
#endif
        }
        else
        {
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
        }

    }
    return TRUE;
}

INT Set_MibBucket_Proc (RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UCHAR  MibBucketEnable;
    UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);
    UCHAR i=0;

	

	
    MibBucketEnable = simple_strtol(arg, 0, 10);

    //MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, MibBucket Enable=%d \n", __FUNCTION__, BandIdx, MibBucketEnable));
	for(i=0;i<concurrent_bands;i++) 
    		pAd->OneSecMibBucket.Enabled[i] = MibBucketEnable;

	pAd->MsMibBucket.Enabled = MibBucketEnable;
    return TRUE;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
INT Set_PBC_Proc (RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set PBC Up bound:\n"));
	
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		pAd->pbc_bound[i] = simple_strtol(value, 0, 10);
		
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d: %d\n",i,pAd->pbc_bound[i]));
	}

    return TRUE;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

/* hwnat optimize */
#ifdef CONFIG_WLAN_LAN_BY_PASS_HWNAT
INT Set_LanNatSpeedUpEn_Proc (RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  LanNatSpeedUpEn;

	LanNatSpeedUpEn = simple_strtol(arg, 0, 10);

	if( LanNatSpeedUpEn )
		pAd->LanNatSpeedUpEn = 1;
	else
		pAd->LanNatSpeedUpEn = 0;

	pAd->isInitBrLan = 0;
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set LanNatSpeedUpEn = %d\n",pAd->LanNatSpeedUpEn ));
	

    return TRUE;
}
#endif

/*dump radio information*/
INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	UCHAR ch;
	UCHAR c1;
#ifdef DOT11_VHT_AC
	UCHAR c2;
#endif /*DOT11_VHT_AC*/
	UCHAR bw;
	UCHAR pm;
	UCHAR ech;
	CHAR str[32]="";
#ifdef DOT11_N_SUPPORT
	CHAR str2[32]="";
#endif /*DOT11_N_SUPPORT*/
	CHAR *pstr=NULL;
	UCHAR i;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========BBP radio information==========\n"));
#ifdef DBDC_MODE
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DBDCEn\t: %s\n",(pAd->CommonCfg.dbdc_mode) ? "Enable" : "Disable"));
#endif /*DBDC_MODE*/
	/*for 2.4G check*/
	pm = HcGetPhyModeByRf(pAd,RFIC_24GHZ);
	if(WMODE_CAP_2G(pm)){
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========2.4G band==========\n"));
		pstr = wmode_2_str(pm);
		if(pstr!=NULL){
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode\t: %s\n",pstr));
			os_free_mem(pstr);
		}
		ch = HcGetChannelByRf(pAd,RFIC_24GHZ);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ch\t: %d\n",ch));
#ifdef DOT11_N_SUPPORT
		if(WMODE_CAP_N(pm)){
			bw = HcGetBwByRf(pAd,RFIC_24GHZ);
			bw_2_str(bw,str);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bw\t: %s\n",str));
			ech = HcGetExtCha(pAd,ch);
			extcha_2_str(ech,str);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("extcha\t: %s\n",str));
			c1 = HcGetCentralChByRf(pAd,RFIC_24GHZ);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch\t: %d\n",c1));
		}
#endif /*DOT11_N_SUPPORT*/
	}

	/*for 5G check*/
	pm = HcGetPhyModeByRf(pAd,RFIC_5GHZ);
	if(WMODE_CAP_5G(pm)){
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========5G band==========\n"));
		pstr = wmode_2_str(pm);
		if(pstr!=NULL){
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode\t: %s\n",pstr));
			os_free_mem(pstr);
		}
		ch = HcGetChannelByRf(pAd,RFIC_5GHZ);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ch\t: %d\n",ch));
#ifdef DOT11_N_SUPPORT
		if(WMODE_CAP_N(pm)){
			bw = HcGetBwByRf(pAd,RFIC_5GHZ);
			bw_2_str(bw,str);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bw\t: %s\n",str));
			ech = HcGetExtCha(pAd,ch);
			extcha_2_str(ech,str);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("extcha\t: %s\n",str));
			c1 = HcGetCentralChByRf(pAd,RFIC_5GHZ);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch1\t: %d\n",c1));
		}
#ifdef DOT11_VHT_AC
		if(WMODE_CAP_AC(pm)){
			c1 = pAd->CommonCfg.vht_cent_ch;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch1\t: %d\n",c1));
			c2 = pAd->CommonCfg.vht_cent_ch2;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen ch2\t: %d\n",c2));
		}
#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("##########WDEV radio information##########\n"));
	for(i=0;i<WDEV_NUM_MAX;i++){
		wdev = pAd->wdev_list[i];
		if(wdev){
			UCHAR cfg_ext_cha = wlan_config_get_ext_cha(wdev);
			UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);
			pstr = wdev_type2str(wdev->wdev_type);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========wdev(%d)==========\n",i));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("type\t: %s\n",pstr));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fun_idx\t: %d\n",wdev->func_idx));
			pstr = wmode_2_str(wdev->PhyMode);
			if(pstr!=NULL){
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode\t: %s\n",pstr));
				os_free_mem(pstr);
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel\t: %d\n",wdev->channel));
#ifdef DOT11_N_SUPPORT
			if(WMODE_CAP_N(wdev->PhyMode)){
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch1\t: %d\n",wdev->CentralChannel));
				bw_2_str(wlan_config_get_ht_bw(wdev),str);
				bw_2_str(wlan_operate_get_ht_bw(wdev),str2);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ht_bw\t: (%s,%s)\n",str,str2));
				extcha_2_str(cfg_ext_cha,str);
				extcha_2_str(op_ext_cha,str2);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ext_ch\t: (%s,%s)\n",str,str2));
			}
#ifdef DOT11_VHT_AC
			if(WMODE_CAP_AC(wdev->PhyMode)){
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch2\t: %d\n",pAd->CommonCfg.vht_cent_ch2));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("vht_bw\t: (%d,%d)\n",
					wlan_config_get_vht_bw(wdev),wlan_operate_get_vht_bw(wdev)));
			}
#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
			bw_2_str(wlan_operate_get_bw(wdev),str);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bw\t: %s\n",str));
		}
	}
    return TRUE;
}

UINT8 MTGetBandIdxByIf(RTMP_ADAPTER *pAd)
{
	UINT8 band_idx = 0;
	struct wifi_dev *wdev = NULL;
	PNET_DEV if_dev = NULL;
	wdev = MTGetWDev(pAd);

	if (!wdev)
		goto err0;
	if_dev = wdev->if_dev;
	if (!if_dev)
		goto err0;


	band_idx = HcGetBandByChannel(pAd, wdev->channel);
	return band_idx;
	err0:
	return -1;
}

struct wifi_dev* MTGetWDev(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;

	switch (pObj->ioctl_if_type) {
	case INT_APCLI:
#ifdef APCLI_SUPPORT
		wdev = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev;
#endif
		break;
	case INT_MAIN:
	case INT_MBSSID:
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
		break;
	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s, interface type %x is not supported in ATE\n"
			,__FUNCTION__, pObj->ioctl_if_type));
		break;
	}
	return wdev;
}


