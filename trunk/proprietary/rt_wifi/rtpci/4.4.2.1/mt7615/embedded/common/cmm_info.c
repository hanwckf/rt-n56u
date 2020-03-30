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
	cmm_info.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include	"rt_config.h"

#define MCAST_WCID_TO_REMOVE 0 //Pat: TODO

INT MCSMappingRateTable[] =
	{    2,  4, 11, 22, 12,  18,  24,  36, 48,  72,  96, 108, 109, 110, 111, 112,/* CCK and OFDM */
		13, 26, 39, 52, 78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,
		39, 78,117,156,234, 312, 351, 390, /* BW 20, 800ns GI, MCS 0~23 */
		27, 54, 81,108,162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
		81,162,243,324,486, 648, 729, 810, /* BW 40, 800ns GI, MCS 0~23 */
		14, 29, 43, 57, 87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,
		43, 87,130,173,260, 317, 390, 433, /* BW 20, 400ns GI, MCS 0~23 */
		30, 60, 90,120,180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
		90,180,270,360,540, 720, 810, 900, /* BW 40, 400ns GI, MCS 0~23 */

		/*for 11ac:20 Mhz 800ns GI*/
		6,  13, 19, 26,  39,  52,  58,  65,  78,  0,     /*1ss mcs 0~8*/
		13, 26, 39, 52,  78,  104, 117, 130, 156, 0,     /*2ss mcs 0~8*/
		19, 39, 58, 78,  117, 156, 175, 195, 234, 260,   /*3ss mcs 0~9*/
		26, 52, 78, 104, 156, 208, 234, 260, 312, 0,     /*4ss mcs 0~8*/	

		/*for 11ac:40 Mhz 800ns GI*/
		13,	27,	40,	54,	 81,  108, 121, 135, 162, 180,   /*1ss mcs 0~9*/
		27,	54,	81,	108, 162, 216, 243, 270, 324, 360,   /*2ss mcs 0~9*/
		40,	81,	121,162, 243, 324, 364, 405, 486, 540,   /*3ss mcs 0~9*/
		54,	108,162,216, 324, 432, 486, 540, 648, 720,   /*4ss mcs 0~9*/

		/*for 11ac:80 Mhz 800ns GI*/
		29,	58,	87,	117, 175, 234, 263, 292, 351, 390,   /*1ss mcs 0~9*/
		58,	117,175,243, 351, 468, 526, 585, 702, 780,   /*2ss mcs 0~9*/
		87,	175,263,351, 526, 702, 0,	877, 1053,1170,  /*3ss mcs 0~9*/
		117,234,351,468, 702, 936, 1053,1170,1404,1560,  /*4ss mcs 0~9*/

		/*for 11ac:160 Mhz 800ns GI*/
		58,	117,175,234, 351, 468, 526, 585, 702, 780,   /*1ss mcs 0~9*/
		117,234,351,468, 702, 936, 1053,1170,1404,1560,  /*2ss mcs 0~9*/
		175,351,526,702, 1053,1404,1579,1755,2160,0,     /*3ss mcs 0~8*/
		234,468,702,936, 1404,1872,2106,2340,2808,3120,  /*4ss mcs 0~9*/

		/*for 11ac:20 Mhz 400ns GI*/
		7,	14,	21,	28,  43,  57,   65,	 72,  86,  0,    /*1ss mcs 0~8*/
		14,	28,	43,	57,	 86,  115,  130, 144, 173, 0,    /*2ss mcs 0~8*/
		21,	43,	65,	86,	 130, 173,  195, 216, 260, 288,  /*3ss mcs 0~9*/
		28,	57,	86,	115, 173, 231,  260, 288, 346, 0,    /*4ss mcs 0~8*/

		/*for 11ac:40 Mhz 400ns GI*/	
		15,	30,	45,	60,	 90,  120,  135, 150, 180, 200,  /*1ss mcs 0~9*/
		30,	60,	90,	120, 180, 240,  270, 300, 360, 400,  /*2ss mcs 0~9*/
		45,	90,	135,180, 270, 360,  405, 450, 540, 600,  /*3ss mcs 0~9*/
		60,	120,180,240, 360, 480,  540, 600, 720, 800,  /*4ss mcs 0~9*/

		/*for 11ac:80 Mhz 400ns GI*/		
		32,	65,	97,	130, 195, 260,  292, 325, 390, 433,  /*1ss mcs 0~9*/
		65,	130,195,260, 390, 520,  585, 650, 780, 866,  /*2ss mcs 0~9*/
		97,	195,292,390, 585, 780,  0,	 975, 1170,1300, /*3ss mcs 0~9*/
		130,260,390,520, 780, 1040,	1170,1300,1560,1733, /*4ss mcs 0~9*/

		/*for 11ac:160 Mhz 400ns GI*/	
		65,	130,195,260, 390, 520,  585, 650, 780, 866,  /*1ss mcs 0~9*/
		130,260,390,520, 780, 1040,	1170,1300,1560,1733, /*2ss mcs 0~9*/
		195,390,585,780, 1170,1560,	1755,1950,2340,0,    /*3ss mcs 0~8*/
		260,520,780,1040,1560,2080,	2340,2600,3120,3466, /*4ss mcs 0~9*/
	
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
		20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37}; /* 3*3 */

/*
    ==========================================================================
    Description:
        Get Driver version.

    Return:
    ==========================================================================
*/
INT Set_DriverVersion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {	
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("Driver version-%s %s %s\n", AP_DRIVER_VERSION, __DATE__, __TIME__));
    }    
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_ANDES_SUPPORT
	if ((pAd->chipCap.MCUType & ANDES) == ANDES) 
    {
		UINT32 loop = 0;
		RTMP_CHIP_CAP *cap = &pAd->chipCap;

		if (pAd->chipCap.need_load_fw) 
        {
			USHORT fw_ver, build_ver;
			fw_ver = (*(cap->FwImgBuf + 11) << 8) | (*(cap->FwImgBuf + 10));
			build_ver = (*(cap->FwImgBuf + 9) << 8) | (*(cap->FwImgBuf + 8));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                ("fw version:%d.%d.%02d ", (fw_ver & 0xf000) >> 8,
							(fw_ver & 0x0f00) >> 8, fw_ver & 0x00ff));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                                    ("build:%x\n", build_ver));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                                            ("build time:"));

			for (loop = 0; loop < 16; loop++)
            {	
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                        ("%c", *(cap->FwImgBuf + 16 + loop)));
            }
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}

		if (pAd->chipCap.need_load_rom_patch) 
        {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                                ("rom patch version = \n"));

			for (loop = 0; loop < 4; loop++)
            {	
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                            ("%c", *(cap->rom_patch + 24 + loop)));
            }
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("build time = \n"));

			for (loop = 0; loop < 16; loop++)
            {	
                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                            ("%c", *(cap->rom_patch + loop)));
            }
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}
	}
#endif

    return TRUE;
}

/*
    ==========================================================================
    Description:
        Set Country Region.
        This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;

#ifdef RF_LOCKDOWN
    /* Check RF lock Status */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config CountryRegion status!! \n", __FUNCTION__));
        return TRUE;
    }
#endif /* RF_LOCKDOWN */

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */

	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_24G);
	if (retval == FALSE)
	{	
	    return FALSE;
    }
	/* if set country region, driver needs to be reset*/
	BuildChannelList(pAd);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
        ("Set_CountryRegion_Proc::(CountryRegion=%d)\n", 
                        pAd->CommonCfg.CountryRegion));

	return TRUE;
}


/*
    ==========================================================================
    Description:
        Set Country Region for A band.
        This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryRegionABand_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;

#ifdef RF_LOCKDOWN
    /* Check RF lock Status */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! Cannot config CountryRegion status!! \n", __FUNCTION__));
        return TRUE;
    }
#endif /* RF_LOCKDOWN */

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */

	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_5G);
	if (retval == FALSE)
		return FALSE;

	/* if set country region, driver needs to be reset*/
	BuildChannelList(pAd);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_CountryRegionABand_Proc::(CountryRegion=%d)\n", pAd->CommonCfg.CountryRegionForABand));

	return TRUE;
}


INT	Set_Cmm_WirelessMode_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	INT	success = TRUE;
	LONG cfg_mode = simple_strtol(arg, 0, 10);
	UCHAR wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef MBSS_SUPPORT
    UCHAR i;
    struct wifi_dev *TmpWdev = NULL;
#endif

	if(!wmode_valid_and_correct(pAd,&wmode))
	{
		success = FALSE;
		goto error;
	}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
			wdev->PhyMode = wmode;
#ifdef MBSS_SUPPORT
			if(!(success = RT_CfgSetMbssWirelessMode(pAd, arg)))
			{
				goto error;
			}

			for(i=0; i<pAd->ApCfg.BssidNum; i++)
			{
				TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;
				/*update WmmCapable*/
				if(!wmode_band_equal(TmpWdev->PhyMode,wmode))
				{
					continue;
				}
				TmpWdev->bWmmCapable = pAd->ApCfg.MBSSID[i].bWmmCapableOrg;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_Cmm_WirelessMode_Proc::(BSS%d=%d)\n",pObj->ioctl_if,wdev->PhyMode));
#else
			if(!(success = RT_CfgSetWirelessMode(pAd, arg,wdev)))
			{
				goto error;
			}
#endif /*MBSS_SUPPORT*/
			HcAcquireRadioForWdev(pAd,wdev);
			RTMPSetPhyMode(pAd,wdev, wmode);
			RTMPUpdateRateInfo(wmode, &wdev->rate);
			UpdateBeaconHandler(pAd,wdev,IE_CHANGE);

		}
#endif /* CONFIG_AP_SUPPORT */


	return success;
error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_WirelessMode_Proc::parameters out of range\n"));
	return success;
}


#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
/*
    ==========================================================================
    Description:
        Set Wireless Mode for MBSS
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_MBSS_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return Set_Cmm_WirelessMode_Proc(pAd, arg);
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

/*
    ==========================================================================
    Description:
        Set Wireless Mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_WirelessMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return Set_Cmm_WirelessMode_Proc(pAd, arg);
}

#ifdef RT_CFG80211_SUPPORT
INT Set_DisableCfg2040Scan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan = (UCHAR) simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan  %d \n",pAd->cfg80211_ctrl.FlgCfg8021Disable2040Scan ));
	return TRUE;
}
#endif



/*
    ==========================================================================
    Description:
        Set Channel
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	UCHAR Channel = (UCHAR) simple_strtol(arg, 0, 10);

    if (wdev == NULL)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev == NULL! if_type %d, if_idx = %d\n", __FUNCTION__, 
			pObj->ioctl_if_type, 
			if_idx));
        return FALSE;
    }

	return rtmp_set_channel(pAd, wdev, Channel);
}
#ifdef WH_EZ_SETUP
INT ap_phy_rrm_init_byRf(RTMP_ADAPTER *pAd, UCHAR RfIC);
#endif
INT	rtmp_set_channel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
 	INT32 Success = TRUE;
	UCHAR OriChannel;
	UCHAR RFChannel;
	UCHAR PhyMode;
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	PNET_DEV ndev_ap_if = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.if_dev; 
#endif
	UCHAR RfIC = 0;
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
    BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
    MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg = &(BgndScanCtrl->CurrentSwChCfg[0]);
    INT32 ret = 0;
#endif /* BACKGROUND_SCAN_SUPPORT */
	UCHAR op_ht_bw;

    if (wdev == NULL)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev == NULL!\n", __FUNCTION__));
        return FALSE;
    }

	if (IsHcRadioCurStatOffByWdev(wdev))
	    return Success;
	op_ht_bw = wlan_operate_get_ht_bw(wdev);

	if(Channel > 14) {
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_5GHZ);
        OriChannel = HcGetChannelByRf(pAd, RFIC_5GHZ);
		RfIC = RFIC_5GHZ;
    }
	else {
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_24GHZ);
        OriChannel = HcGetChannelByRf(pAd, RFIC_24GHZ);
		RfIC = RFIC_24GHZ;
    }

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if (pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
				pAd->ApCfg.ApCliAutoConnectChannelSwitching = TRUE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

	/* check if this channel is valid*/
	if (ChannelSanity(pAd, Channel) == TRUE)
	{

		Success = TRUE;
	}
	else
	{

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			Channel = FirstChannel(pAd);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,("This channel is out of channel list, set as the first channel(%d) \n ", Channel));
		}
#endif /* CONFIG_AP_SUPPORT */

	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((WMODE_CAP_5G(PhyMode))
			&& (pAd->CommonCfg.bIEEE80211H == TRUE))
		{
			if (CheckNonOccupancyChannel(pAd, Channel) == FALSE)
				Success = FALSE;
		}

		if (Success == TRUE)
		{
			if ((Channel > 14 )
				&& (pAd->CommonCfg.bIEEE80211H == TRUE))
			{
				pAd->Dot11_H.org_ch = OriChannel;
			}

#ifdef DOT11_N_SUPPORT
			N_ChannelCheck(pAd,PhyMode,Channel);

			if (WMODE_CAP_N(PhyMode) &&
					op_ht_bw == BW_40)
			{
				RFChannel = N_SetCenCh(pAd, Channel,op_ht_bw);
			}
			else
#endif /* DOT11_N_SUPPORT */
				RFChannel = Channel;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): CtrlChannel(%d), CentralChannel(%d) RDMode %d \n",
							__FUNCTION__, Channel,
							pAd->CommonCfg.CentralChannel,
							pAd->Dot11_H.RDMode));

			if ((Channel > 14 )
				&& (pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef WH_EZ_SETUP
				&& (!(IS_EZ_SETUP_ENABLED(wdev) && (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)))
#endif
				)
			{
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)  
#ifdef DFS_DBG_LOG_3
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Current Channel is %d; DfsZeroWaitStat/Support=%d/%d\n", 
                                                                   Channel, 
                                                                   GET_MT_ZEROWAIT_DFS_STATE(pAd),
                                                                   BgndScanCtrl->DfsZeroWaitSupport));
#endif
                if (Channel !=0 && 
                    BgndScanCtrl->DfsZeroWaitSupport && 
                    BgndScanCtrl->BgndScanSupport && 
                   (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT) == FALSE)) 
                {   
                    if (RadarChannelCheck(pAd, Channel) 
#ifdef MAC_REPEATER_SUPPORT
					    && (pAd->ApCfg.bMACRepeaterEn == FALSE)
#endif
					    )//Logic refine for repeater mode 
                    {  
                        //For CAC & set Dfs Ch case: Ch assign move into ZeroWaitStop
                        if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) == TRUE)
                        {
                            BgndScanCtrl->DfsZeroWaitChannel = Channel; /* Assign this Ch into zero wait band1 Ch */
                        }
                        else
                        {      
                            /* Previous state is Idle/InservMonitor and setting Ch is DFS Ch  */
#ifdef DFS_DBG_LOG_3			    
                            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Channel %d is a DFS channel. Trigger Auto Channel selection!\n",
                                                                             __FUNCTION__,
                                                                             Channel));
#endif
                            BgndScanCtrl->DfsZeroWaitChannel = Channel; /* Set DFS zero wait band1 Ch */

                            /* Re-select a non-DFS channel. */
                            Channel = WrapDfsRandomSelectChannel(pAd, TRUE, 0); /* Skip DFS CH */
#ifdef DFS_DBG_LOG_3			    
                            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Get a new Channel %d for DFS zero wait (temporary) using!\n", Channel));	       
#endif
			}
                    }
                    else
                    {
                        /* Set Zero wait Ch into 0 to distinguish from ORI_CAC_SetNonDfsCh & ORI_CAC_SetDfsCh */
                        BgndScanCtrl->DfsZeroWaitChannel = 0;
                    }
                    
                    if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC))/* Ori state is CAC & setting Ch is DFS/NonDFS Ch */
                    {
                        if (BgndScanCtrl->DfsZeroWaitChannel == 0)
                        {
                            BgndScanCtrl->DfsZeroWaitChannel = Channel; //Save non-Dfs target Channel in DfsZeroWaitChannel temporary
                        }

                        pAd->Dot11_H.RDMode = RD_SWITCHING_MODE; //

                        /* Do ZeroWaitStop */
                        MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_RDD_TIMEOUT, 0, NULL, 0);
                        RTMP_MLME_HANDLER(pAd);
                        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:Enqueue ZeroWaitStop Cmd\n", __FUNCTION__));
                        return TRUE;
                    }
                    else /* Ori state is Idle or InServMonitor */
                    {    
                        if (BgndScanCtrl->DfsZeroWaitChannel == 0)
                        {
                            /* 1.Previous state is Idle/InSrv state and next Ch is NonDFS Ch 
                            /  2.Repeater mode dynamic enable
                            */
                            UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
                            pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
#ifdef MAC_REPEATER_SUPPORT
                            //Disable DFS zero wait support  for repeater mode enable
                            if (pAd->ApCfg.bMACRepeaterEn)
                            {
                                BgndScanCtrl->DfsZeroWaitSupport = FALSE;
                                UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
                            }
#endif /* MAC_REPEATER_SUPPORT */                            
                        }
                        else
                        {
                            /* 1. Previous is Idle/InSrv state and next Ch is DFS Ch */
                            pAd->Dot11_H.org_ch = HcGetChannelByRf(pAd, RFIC_5GHZ); 
                            ret = HcUpdateChannel(pAd,Channel);
                            if(ret < 0 )
                            {
                                MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                                   ("%s(): Update Channel %d faild, not support this RF\n", __FUNCTION__, Channel));                 
                            }
                            UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC);
                            pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
                            pAd->Dot11_H.CSCount = 0;
                           

                            /* Update Control/Central Channel into BgndScanCtrl.CurrentSwChCfg
                               In here, Channel is zero wait band0 channel */
                            CurrentSwChCfg->ControlChannel = Channel;
                            CurrentSwChCfg->CentralChannel = DfsGetCentCh(pAd, Channel, CurrentSwChCfg->Bw);
                            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Trigger DFS Zero wait procedure Support=%d, DfsZeroWaitChannel=%d", 
                                                                        __FUNCTION__,
                                                                        BgndScanCtrl->DfsZeroWaitSupport, 
                                                                        BgndScanCtrl->DfsZeroWaitChannel));
                                
                            /* Update Beacon channel for ZeroWait working state */
                            if(HcUpdateCsaCntByChannel(pAd, Channel) != 0)
                            {
                                return Success;
                            }                            
                            return TRUE;
                        }
                    }
                }
#endif /* BACKGROUND_SCAN_SUPPORT && MT_DFS_SUPPORT */
    
				if ((pAd->Dot11_H.RDMode == RD_SILENCE_MODE) 
					|| ((ndev_ap_if!=NULL) && (!RTMP_OS_NETDEV_STATE_RUNNING(ndev_ap_if)))) 
				{
				    pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
					AsicSwitchChannel(pAd, RFChannel, FALSE);
					/*update channel for all of wdev belong this band*/
					if(HcUpdateChannel(pAd,Channel) !=0)
					{
						return Success;
					}
#ifdef WH_EZ_SETUP					
#ifdef EZ_NETWORK_MERGE_SUPPORT
					//! base channel is different from target channel, do restart AP 					
					if (!IS_EZ_SETUP_ENABLED(wdev) || 
#ifdef EZ_MOD_SUPPORT
						wdev->ez_driver_params.do_not_restart_interfaces != 1
#else
						wdev->ez_security.do_not_restart_interfaces != 1
#endif
					)
					{
#endif					
#endif
						APStopByRf(pAd, RfIC);
						APStartUpByRf(pAd, RfIC);
#ifdef WH_EZ_SETUP							
#ifdef EZ_NETWORK_MERGE_SUPPORT
					} else {
							
						if(Channel > 14)
							ap_phy_rrm_init_byRf(pAd,RFIC_5GHZ);
						else
							ap_phy_rrm_init_byRf(pAd,RFIC_24GHZ);
					}
#endif
#endif					
				}
				else
				{
					NotifyChSwAnnToPeerAPs(pAd, ZERO_MAC_ADDR, pAd->CurrentAddress, 1, Channel);
					pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
					pAd->Dot11_H.CSCount = 0;
					pAd->Dot11_H.new_channel = Channel;

					if(HcUpdateChannel(pAd,Channel) !=0)
					{
						return Success;
					}

	                if(HcUpdateCsaCntByChannel(pAd, Channel) != 0)
	                {
	                    return Success;
					}
				}
			}
			else
			{
				/*update channel for all of wdev belong this band*/
				if(HcUpdateChannel(pAd,Channel) !=0)
				{
					return Success;
				}

#ifdef WH_EZ_SETUP					
#ifdef EZ_NETWORK_MERGE_SUPPORT				
				//! base channel is different from target channel, do not restart AP
				if (!IS_EZ_SETUP_ENABLED(wdev) || 
#ifdef EZ_MOD_SUPPORT
					wdev->ez_driver_params.do_not_restart_interfaces != 1
#else
					wdev->ez_security.do_not_restart_interfaces != 1
#endif
				)
				{
#endif				
#endif
					AsicSwitchChannel(pAd, RFChannel, FALSE);

					APStopByRf(pAd, RfIC);
					APStartUpByRf(pAd, RfIC);
#ifdef WH_EZ_SETUP						
#ifdef EZ_NETWORK_MERGE_SUPPORT
				} else {
					if(Channel > 14)
						ap_phy_rrm_init_byRf(pAd,RFIC_5GHZ);
					else
						ap_phy_rrm_init_byRf(pAd,RFIC_24GHZ);
				}

#endif
#endif				
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (Success == TRUE)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_Channel_Proc_by_Wdev::(Channel=%d)\n", Channel));
#ifdef RADIO_LINK_SELECTION
		if (pAd->ApCfg.RadioLinkSelection)
			Rls_UpdateTableChannel(pAd, OriChannel, Channel);
#endif /* RADIO_LINK_SELECTION */
	}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

	return Success;
}


/*
    ==========================================================================
    Description:
        Set Short Slot Time Enable or Disable
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ShortSlot_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int retval;

	retval = RT_CfgSetShortSlot(pAd, arg);
	if (retval == TRUE)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ShortSlot_Proc::(ShortSlot=%d)\n", pAd->CommonCfg.bUseShortSlotTime));

	return retval;
}


/*
    ==========================================================================
    Description:
        Set Tx power
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxPower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    LONG    TxPower;
    INT     status = FALSE;
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

    TxPower = simple_strtol(arg, 0, 10);
    
    if (TxPower <= 100)
    {
#ifdef CONFIG_AP_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
        {
            pAd->CommonCfg.TxPowerPercentage[BandIdx] = TxPower;
        }
#endif /* CONFIG_AP_SUPPORT */

        status = TRUE;
    }
    else
    {   
        status = FALSE;
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_TxPower_Proc: BandIdx: %d, (TxPowerPercentage=%ld)\n", BandIdx, pAd->CommonCfg.TxPowerPercentage[BandIdx]));

    return status;
}


/*
    ==========================================================================
    Description:
        Set 11B/11G Protection
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_BGProtection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	switch (simple_strtol(arg, 0, 10))
	{
		case 0: /*AUTO*/
			pAd->CommonCfg.UseBGProtection = 0;
			break;
		case 1: /*Always On*/
			pAd->CommonCfg.UseBGProtection = 1;
			break;
		case 2: /*Always OFF*/
			pAd->CommonCfg.UseBGProtection = 2;
			break;
		default:  /*Invalid argument */
			return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		ApUpdateCapabilityAndErpIe(pAd,&pAd->ApCfg.MBSSID[pObj->ioctl_if]);
	}
#endif /* CONFIG_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BGProtection_Proc::(BGProtection=%ld)\n", pAd->CommonCfg.UseBGProtection));

	return TRUE;
}

/*
    ==========================================================================
    Description:
        Set TxPreamble
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxPreamble_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RT_802_11_PREAMBLE	Preamble;

	Preamble = (RT_802_11_PREAMBLE)simple_strtol(arg, 0, 10);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	if (Preamble == Rt802_11PreambleAuto)
		return FALSE;
#endif /* CONFIG_AP_SUPPORT */

	switch (Preamble)
	{
		case Rt802_11PreambleShort:
			pAd->CommonCfg.TxPreamble = Preamble;
			break;
		case Rt802_11PreambleLong:
			pAd->CommonCfg.TxPreamble = Preamble;
			break;
		default: /*Invalid argument */
			return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_TxPreamble_Proc::(TxPreamble=%ld)\n", pAd->CommonCfg.TxPreamble));

	return TRUE;
}

/*
    ==========================================================================
    Description:
        Set RTS Threshold
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
static VOID set_rts_len_thld(struct wifi_dev *wdev, UINT32 length)
{
	wlan_operate_set_rts_len_thld(wdev, length);
}

INT Set_RTSThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 length = MAX_RTS_THRESHOLD;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Usage:\niwpriv raN set RTSThreshold=[length]\n"));
		return FALSE;
	}
	if (!wdev)
		return FALSE;

	length = simple_strtol(arg, 0, 10);
	set_rts_len_thld(wdev, length);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: set wdev%d rts length threshold=%d(0x%x)\n", __func__, wdev->wdev_idx, length, length));

	return TRUE;
}

/*
    ==========================================================================
    Description:
        Set Fragment Threshold
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_FragThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 frag_thld;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);

	if (!arg)
		return FALSE;
	if (!wdev)
		return FALSE;

	frag_thld = simple_strtol(arg, 0, 10);
	if (frag_thld > MAX_FRAG_THRESHOLD || frag_thld < MIN_FRAG_THRESHOLD)
		frag_thld = MAX_FRAG_THRESHOLD;
	else if ((frag_thld % 2) == 1)
		frag_thld -= 1;
	wlan_operate_set_frag_thld(wdev, frag_thld);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s::set wdev%d FragThreshold=%d)\n", __func__, wdev->wdev_idx, frag_thld));

	return TRUE;
}

/*
    ==========================================================================
    Description:
        Set TxBurst
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_TxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG TxBurst;

	TxBurst = simple_strtol(arg, 0, 10);
	if (TxBurst == 1)
		pAd->CommonCfg.bEnableTxBurst = TRUE;
	else if (TxBurst == 0)
		pAd->CommonCfg.bEnableTxBurst = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_TxBurst_Proc::(TxBurst=%d)\n", pAd->CommonCfg.bEnableTxBurst));

	return TRUE;
}


#ifdef RTMP_MAC_PCI
INT Set_ShowRF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int ShowRF = simple_strtol(arg, 0, 10);

	if (ShowRF == 1)
		pAd->ShowRf = TRUE;
	else
		pAd->ShowRf = FALSE;

	return TRUE;
}
#endif /* RTMP_MAC_PCI */


#ifdef AGGREGATION_SUPPORT
/*
    ==========================================================================
    Description:
        Set TxBurst
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_PktAggregate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG aggre;

	aggre = simple_strtol(arg, 0, 10);

	if (aggre == 1)
		pAd->CommonCfg.bAggregationCapable = TRUE;
	else if (aggre == 0)
		pAd->CommonCfg.bAggregationCapable = FALSE;
	else
		return FALSE;  /*Invalid argument */

#ifdef CONFIG_AP_SUPPORT
#ifdef PIGGYBACK_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
		AsicSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
	}
#endif /* PIGGYBACK_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_PktAggregate_Proc::(AGGRE=%d)\n", pAd->CommonCfg.bAggregationCapable));

	return TRUE;
}
#endif


/*
    ==========================================================================
    Description:
        Set IEEE80211H.
        This parameter is 1 when needs radar detection, otherwise 0
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_IEEE80211H_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    LONG ieee80211h;

	ieee80211h = simple_strtol(arg, 0, 10);

	if (ieee80211h == 1)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else if (ieee80211h == 0)
		pAd->CommonCfg.bIEEE80211H = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_IEEE80211H_Proc::(IEEE80211H=%d)\n", pAd->CommonCfg.bIEEE80211H));

	return TRUE;
}

#ifdef EXT_BUILD_CHANNEL_LIST
/*
    ==========================================================================
    Description:
        Set Country Code.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ExtCountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __FUNCTION__));
		return TRUE;
	}

	if(strlen(arg) == 2)
	{
		NdisMoveMemory(pAd->CommonCfg.CountryCode, arg, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
	}
	else
	{
		NdisZeroMemory(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	{
		UCHAR CountryCode[3] = {0};
		NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_CountryCode_Proc::(bCountryFlag=%d, CountryCode=%s)\n",
							pAd->CommonCfg.bCountryFlag,
							CountryCode));
	}
	return TRUE;
}
/*
    ==========================================================================
    Description:
        Set Ext DFS Type
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ExtDfsType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR	*pDfsType = &pAd->CommonCfg.DfsType;
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __FUNCTION__));
		return TRUE;
	}

	if (!strcmp(arg, "CE"))
		*pDfsType = CE;
	else if (!strcmp(arg, "FCC"))
		*pDfsType = FCC;
	else if (!strcmp(arg, "JAP"))
		*pDfsType = JAP;
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unsupported DFS type:%s (Legal types are: CE, FCC, JAP)\n", arg));

	return TRUE;
}

/*
    ==========================================================================
    Description:
        Add new channel list
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_ChannelListAdd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CH_DESP		inChDesp;
	PCH_REGION pChRegion = NULL;

	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __FUNCTION__));
		return TRUE;
	}

	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0)
		{
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0)
			{
				pChRegion = &ChRegion[loop];
				break;
			}
			loop++;
		}
		if (pChRegion == NULL)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode is not configured or not valid\n"));
			return TRUE;
		}
	}

	/* Parsing the arg, IN:arg; OUT:inChRegion */
	{
		UCHAR strBuff[64], count = 0;
		PUCHAR	pStart, pEnd, tempIdx, tempBuff[5];

		if (strlen(arg) <64)
			NdisCopyMemory(strBuff, arg, strlen(arg));

		if ((pStart = rtstrchr(strBuff, '[')) != NULL)
		{
			if ((pEnd = rtstrchr(pStart++, ']')) != NULL)
			{
				tempBuff[count++] = pStart;
				for(tempIdx = pStart ;tempIdx != pEnd; tempIdx++)
				{
					if(*tempIdx == ',')
					{
						*tempIdx = '\0';
						tempBuff[count++] = ++tempIdx;
					}
				}
				*(pEnd) = '\0';

				if (count != 5)
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Input Error. Too more or too less parameters.\n"));
					return TRUE;
				}
				else
				{
					inChDesp.FirstChannel = (UCHAR) simple_strtol(tempBuff[0], 0, 10);
					inChDesp.NumOfCh = (UCHAR) simple_strtol(tempBuff[1], 0, 10);
					inChDesp.MaxTxPwr = (UCHAR) simple_strtol(tempBuff[2], 0, 10);
					inChDesp.Geography = (!strcmp(tempBuff[3], "BOTH") ? BOTH: (!strcmp(tempBuff[3], "IDOR") ? IDOR : ODOR));
					inChDesp.DfsReq= (!strcmp(tempBuff[4], "TRUE") ? TRUE : FALSE);
				}
			}
			else
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Missing End \"]\"\n"));
				return TRUE;
			}
		}
		else
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Invalid input format.\n", __FUNCTION__));
			return TRUE;
		}
	}

	/* Add entry to Channel List*/
	{
		UCHAR EntryIdx;
		PCH_DESP pChDesp = NULL;
		UCHAR CountryCode[3] = {0};
		if (pAd->CommonCfg.pChDesp == NULL)
		{
			os_alloc_mem(pAd,  &pAd->CommonCfg.pChDesp, MAX_PRECONFIG_DESP_ENTRY_SIZE*sizeof(CH_DESP));
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
			if (pChDesp)
			{
				for (EntryIdx= 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++)
				{
					if (EntryIdx == (MAX_PRECONFIG_DESP_ENTRY_SIZE-2)) /* Keep an NULL entry in the end of table*/
					{
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Table is full.\n"));
						return TRUE;
					}
					NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
				}
				/* Copy the NULL entry*/
				NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
			}
			else
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("os_alloc_mem failded.\n"));
				return FALSE;
			}
		}
		else
		{
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
			for (EntryIdx= 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++)
			{
				if(EntryIdx ==  (MAX_PRECONFIG_DESP_ENTRY_SIZE-2)) /* Keep an NULL entry in the end of table*/
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Table is full.\n"));
					return TRUE;
				}
			}
		}
		NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Add channel lists {%u, %u, %u, %s, %s} to %s.\n",
							inChDesp.FirstChannel,
							inChDesp.NumOfCh,
							inChDesp.MaxTxPwr,
							(inChDesp.Geography == BOTH) ? "BOTH" : (inChDesp.Geography == IDOR) ?  "IDOR" : "ODOR",
							(inChDesp.DfsReq == TRUE) ? "TRUE" : "FALSE",
							CountryCode));
		NdisCopyMemory(&pChDesp[EntryIdx], &inChDesp, sizeof(CH_DESP));
		pChDesp[++EntryIdx].FirstChannel = 0;
	}
	return TRUE;
}

INT Set_ChannelListShow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCH_REGION	pChRegion = NULL;
	UCHAR		EntryIdx, CountryCode[3]={0};

	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;

		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0)
		{
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0)
			{
				pChRegion = &ChRegion[loop];
				break;
			}
			loop++;
		}
		if (pChRegion == NULL)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode is not configured or not valid\n"));
			return TRUE;
		}
	}

	NdisMoveMemory(CountryCode, pAd->CommonCfg.CountryCode, 2);
	if (pAd->CommonCfg.DfsType == MAX_RD_REGION)
		pAd->CommonCfg.DfsType = pChRegion->op_class_region;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("=========================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CountryCode:%s\n", CountryCode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("DfsType:%s\n",
					(pAd->CommonCfg.DfsType == JAP) ? "JAP" :
					((pAd->CommonCfg.DfsType == FCC) ? "FCC" : "CE" )));

	if (pAd->CommonCfg.pChDesp != NULL)
	{
		PCH_DESP pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
		for (EntryIdx = 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%u. {%3u, %2u, %2u, %s, %5s}.\n",
						EntryIdx,
						pChDesp[EntryIdx].FirstChannel,
						pChDesp[EntryIdx].NumOfCh,
						pChDesp[EntryIdx].MaxTxPwr,
						(pChDesp[EntryIdx].Geography == BOTH) ? "BOTH" : (pChDesp[EntryIdx].Geography == IDOR) ?  "IDOR" : "ODOR",
						(pChDesp[EntryIdx].DfsReq == TRUE) ? "TRUE" : "FALSE"));
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Default channel list table:\n"));
		for (EntryIdx = 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%u. {%3u, %2u, %2u, %s, %5s}.\n",
						EntryIdx,
						pChRegion->pChDesp[EntryIdx].FirstChannel,
						pChRegion->pChDesp[EntryIdx].NumOfCh,
						pChRegion->pChDesp[EntryIdx].MaxTxPwr,
						(pChRegion->pChDesp[EntryIdx].Geography == BOTH) ? "BOTH" : (pChRegion->pChDesp[EntryIdx].Geography == IDOR) ?  "IDOR" : "ODOR",
						(pChRegion->pChDesp[EntryIdx].DfsReq == TRUE) ? "TRUE" : "FALSE"));
		}
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("=========================================\n"));
	return TRUE;
}

INT Set_ChannelListDel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR EntryIdx, TargetIdx, NumOfEntry;
	PCH_REGION	pChRegion = NULL;
	PCH_DESP pChDesp = NULL;
	TargetIdx = simple_strtol(arg, 0, 10);
	if (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) == NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s can only be used when interface is down.\n", __FUNCTION__));
		return TRUE;
	}

	/* Get Channel Region (CountryCode)*/
	{
		INT loop = 0;
		while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0)
		{
			if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, pAd->CommonCfg.CountryCode, 2) == 0)
			{
				pChRegion = &ChRegion[loop];
				break;
			}
			loop++;
		}
		if (pChRegion == NULL)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CountryCode is not configured or not valid\n"));
			return TRUE;
		}
	}

	if (pAd->CommonCfg.pChDesp == NULL)
	{
		os_alloc_mem(pAd,  &pAd->CommonCfg.pChDesp, MAX_PRECONFIG_DESP_ENTRY_SIZE*sizeof(CH_DESP));
		if (pAd->CommonCfg.pChDesp)
		{
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
			for (EntryIdx= 0; pChRegion->pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++)
			{
				if (EntryIdx == (MAX_PRECONFIG_DESP_ENTRY_SIZE-2)) /* Keep an NULL entry in the end of table*/
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Table is full.\n"));
					return TRUE;
				}
				NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
			}
			/* Copy the NULL entry*/
			NdisCopyMemory(&pChDesp[EntryIdx], &pChRegion->pChDesp[EntryIdx], sizeof(CH_DESP));
		}
		else
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("os_alloc_mem failded.\n"));
			return FALSE;
		}
	}
	else
		pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;

	if (!strcmp(arg, "default"))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Default table used.\n" ));
		if (pAd->CommonCfg.pChDesp != NULL)
			os_free_mem(pAd->CommonCfg.pChDesp);
		pAd->CommonCfg.pChDesp = NULL;
		pAd->CommonCfg.DfsType = MAX_RD_REGION;
	}
	else if (!strcmp(arg, "all"))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Remove all entries.\n" ));
		for (EntryIdx = 0; EntryIdx < MAX_PRECONFIG_DESP_ENTRY_SIZE; EntryIdx++)
			NdisZeroMemory(&pChDesp[EntryIdx], sizeof(CH_DESP));
	}
	else if (TargetIdx < (MAX_PRECONFIG_DESP_ENTRY_SIZE-1))
	{
		for (EntryIdx= 0; pChDesp[EntryIdx].FirstChannel != 0; EntryIdx++)
		{
			if(EntryIdx ==  (MAX_PRECONFIG_DESP_ENTRY_SIZE-2)) /* Keep an NULL entry in the end of table */
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Last entry should be NULL.\n"));
				pChDesp[EntryIdx].FirstChannel = 0;
				return TRUE;
			}
		}
		NumOfEntry = EntryIdx;
		if (TargetIdx >= NumOfEntry)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Out of table range.\n"));
			return TRUE;
		}
		for (EntryIdx = TargetIdx; EntryIdx < NumOfEntry; EntryIdx++)
			NdisCopyMemory(&pChDesp[EntryIdx], &pChDesp[EntryIdx+1], sizeof(CH_DESP));
		NdisZeroMemory(&pChDesp[EntryIdx], sizeof(CH_DESP)); /*NULL entry*/
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Entry %u deleted.\n", TargetIdx));
	}
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Entry not found.\n"));

	return TRUE;
}
#endif /* EXT_BUILD_CHANNEL_LIST  */

#ifdef WSC_INCLUDED
INT	Set_WscGenPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    PWSC_CTRL   pWscControl = NULL;
    POS_COOKIE  pObj;
    UCHAR       apidx;

    if (pAd == NULL)
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pAd == NULL!\n", __FUNCTION__));
        return TRUE;
    }

    pObj = (POS_COOKIE) pAd->OS_Cookie;
    apidx = pObj->ioctl_if;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
	    if (pObj->ioctl_if_type == INT_APCLI)
	    {
	        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
	        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(apcli%d) Set_WscGenPinCode_Proc:: This command is from apcli interface now.\n", apidx));
	    }
	    else
#endif /* APCLI_SUPPORT */
	    {
			pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
	        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WscGenPinCode_Proc:: This command is from ra interface now.\n", apidx));
	    }
	}
#endif /* CONFIG_AP_SUPPORT */



	if (pWscControl == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pWscControl == NULL!\n", __FUNCTION__));
		return TRUE;
	}

	if (pWscControl->WscEnrollee4digitPinCode)
	{
		pWscControl->WscEnrolleePinCodeLen = 4;
		pWscControl->WscEnrolleePinCode = WscRandomGen4digitPinCode(pAd);
	}
	else
	{
		pWscControl->WscEnrolleePinCodeLen = 8;
#ifdef P2P_SSUPPORT
#endif /* P2P_SUPPORT */
		pWscControl->WscEnrolleePinCode = WscRandomGeneratePinCode(pAd, apidx);
	}


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscGenPinCode_Proc:: Enrollee PinCode\t\t%08u\n", pWscControl->WscEnrolleePinCode));

	return TRUE;
}

#ifdef BB_SOC
INT	Set_WscResetPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL   pWscControl = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		{
			pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(ra%d) Set_WscResetPinCode_Proc:: This command is from ra interface now.\n", apidx));
		}

		pWscControl->WscEnrolleePinCode = GenerateWpsPinCode(pAd, 0, apidx);
	}
#endif // CONFIG_AP_SUPPORT //

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscResetPinCode_Proc:: Enrollee PinCode\t\t%08u\n", pWscControl->WscEnrolleePinCode));

		return TRUE;
}
#endif

INT Set_WscVendorPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL   pWscControl = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT    
	UCHAR       apidx = pObj->ioctl_if;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		if (pObj->ioctl_if_type == INT_APCLI)
		{
			pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscVendorPinCode_Proc() for apcli(%d)\n", apidx));
		}
		else
#endif /* APCLI_SUPPORT */
		{
			pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_WscVendorPinCode_Proc() for ra%d!\n", apidx));
		}
	}
#endif /* CONFIG_AP_SUPPORT */



	if (!pWscControl)
		return FALSE;
	else
	return RT_CfgSetWscPinCode(pAd, arg, pWscControl);
}
#endif /* WSC_INCLUDED */


#ifdef DBG
INT rx_temp_dbg = 0;

/*
    ==========================================================================
    Description:
        For Debug information
        Change DebugLevel
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_Debug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG dbg;

	dbg = simple_strtol(arg, 0, 10);
	if( dbg <= DBG_LVL_MAX)
		DebugLevel = dbg;

	MTWF_PRINT("%s(): (DebugLevel = %d)\n", __FUNCTION__, DebugLevel);

	return TRUE;
}


/*
    ==========================================================================
    Description:
        Change DebugCategory
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_DebugCategory_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG category = (ULONG)simple_strtol(arg, 0, 16);
	DebugCategory = category;
	MTWF_PRINT("%s(): Set DebugCategory = 0x%lx\n", __FUNCTION__, DebugCategory);
	return TRUE;
}

INT Set_Debug_MonitorAddr(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UCHAR					MonitorAddr[MAC_ADDR_LEN];
	RTMP_STRING				*value;
	INT						i;
	

	if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;
	
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid */
	
		AtoH(value, (UCHAR *)&MonitorAddr[i++], 1);
	}
	
	memcpy(pAd->MonitorAddr,MonitorAddr,MAC_ADDR_LEN);
	
	return TRUE;
}

static BOOLEAN ascii2hex(RTMP_STRING *in, UINT32 *out)
{
	UINT32 hex_val, val;
	CHAR *p, asc_val;

	hex_val = 0;
	p = (char *)in;
	while((*p) != 0)
	{
		val = 0;
		asc_val = *p;
		if ((asc_val >= 'a') && (asc_val <= 'f'))
			val = asc_val - 87;
		else if ((*p >= 'A') && (asc_val <= 'F'))
			val = asc_val - 55;
		else if ((asc_val >= '0') && (asc_val <= '9'))
			val = asc_val - 48;
		else
			return FALSE;

		hex_val = (hex_val << 4) + val;
		p++;
	}
	*out = hex_val;

	return TRUE;
}

#if defined(CONFIG_ARCH_MT7623) || defined(CONFIG_ARCH_MT8590) || defined(CONFIG_ARCH_MT7622)
BOOLEAN mt_mac_cr_range_mapping(RTMP_ADAPTER *pAd, UINT32 *mac_addr);
#endif

/*
    ==========================================================================
    Description:
        Read / Write MAC
    Arguments:
        pAd                    Pointer to our adapter
        wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage:
               1.) iwpriv ra0 mac 0        ==> read MAC where Addr=0x0
               2.) iwpriv ra0 mac 0=12     ==> write MAC where Addr=0x0, value=12
    ==========================================================================
*/
VOID RTMPIoctlMAC(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_STRING *seg_str, *addr_str, *val_str, *range_str;
	RTMP_STRING *mpool, *msg;
	RTMP_STRING *arg, *ptr;
	UINT32 macVal = 0;
	UINT32 macValue;
	BOOLEAN bIsPrintAllMAC = FALSE, bFromUI, is_write, is_range;
	UINT32 IdMac, mac_s = 0x1000, mac_e = 0x1700, mac_range = 0xffff;
	BOOLEAN Ret;
#ifdef RTMP_PCI_SUPPORT
	BOOLEAN IsMapAddrNeed=FALSE;
#endif

	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR)*(4096+256+12));
	if (!mpool)
		return;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		mac_range = 0xcffff;
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		mac_range = 0xfffff;
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* MT_MAC */

	bFromUI = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_UI) == RTPRIV_IOCTL_FLAG_UI) ? TRUE : FALSE;

	msg = (RTMP_STRING *)((ULONG)(mpool+3) & (ULONG)~0x03);
	arg = (RTMP_STRING *)((ULONG)(msg+4096+3) & (ULONG)~0x03);

	memset(msg, 0x00, 4096);
	memset(arg, 0x00, 256);

	if (wrq->u.data.length > 1) {
#ifdef LINUX
		INT Status = NDIS_STATUS_SUCCESS;
		Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#else
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#endif /* LINUX */
		arg[255] = 0x00;
	}

	ptr = arg;
	if ((ptr!= NULL) && (strlen(ptr) > 0)) {
		while ((*ptr != 0) && (*ptr == 0x20)) // remove space
			ptr++;

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():wrq->u.data.length=%d, pointer(%p)=%s!\n",
                       __FUNCTION__, wrq->u.data.length, arg, arg));
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():after trim space, ptr len=%zu, pointer(%p)=%s!\n",
                    __FUNCTION__, strlen(ptr), ptr, ptr));
    }

	if ((ptr == NULL) || strlen(ptr) == 0) {
		bIsPrintAllMAC = TRUE;
		goto print_all;
	}

	{
		while ((seg_str = strsep((char **)&ptr, ",")) != NULL)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("seg_str[%zu]=%s\n", strlen(seg_str), seg_str));
			is_write = FALSE;
			addr_str = seg_str;
			val_str = NULL;
			if ((val_str = strchr(seg_str, '=')) != NULL) {
				*val_str++ = 0;
				is_write = 1;
			} else {
				is_write = 0;
			}

			if (addr_str) {
				if ((range_str = strchr(addr_str, '-')) != NULL) {
					*range_str++ = 0;
					is_range = 1;
				} else {
					is_range = 0;
				}

				if ((ascii2hex(addr_str, &mac_s) == FALSE)) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid MAC CR Addr, str=%s\n", addr_str));
					break;
				}

#ifdef MT_MAC
				if (pAd->chipCap.hif_type == HIF_MT){
					Ret=mt_mac_cr_range_mapping(pAd, &mac_s);
				#ifdef RTMP_PCI_SUPPORT
					IsMapAddrNeed = (Ret)?FALSE:TRUE;
				#endif
				}
#endif /* MT_MAC */

#if defined(MT7636) || defined(MT7637) || defined(MT7615)
#else
				if (mac_s >= mac_range) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MAC CR Addr[0x%x] out of range[0x%x], str=%s\n",
								mac_s, mac_range, addr_str));
					break;
				}
#endif

				if (is_range) {
					if (ascii2hex(range_str, &mac_e) == FALSE) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid Range End MAC CR Addr[0x%x], str=%s\n",
									mac_e, range_str));
						break;
					}

#ifdef MT_MAC
					if (pAd->chipCap.hif_type == HIF_MT){
						Ret=mt_mac_cr_range_mapping(pAd, &mac_e);
					#ifdef RTMP_PCI_SUPPORT
						IsMapAddrNeed = (Ret)?FALSE:TRUE;
					#endif
					}
#endif /* MT_MAC */

#if defined(MT7636) || defined(MT7637) || defined(MT7615)
#else
					if (mac_e >= mac_range) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MAC CR Addr[0x%x] out of range[0x%x], str=%s\n",
									mac_e, mac_range, range_str));
						break;
					}
#endif

					if (mac_e < mac_s) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid Range MAC Addr[%s - %s] => [0x%x - 0x%x]\n",
									addr_str, range_str, mac_s, mac_e));
						break;
					}
				} else {
					mac_e = mac_s;
				}
			}

			if (val_str) {
				if ((strlen(val_str) == 0) || ascii2hex(val_str, &macVal) == FALSE) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid MAC value[0x%s]\n", val_str));
					break;
				}
			}

			if (is_write) {
#ifdef RTMP_PCI_SUPPORT
					if(IsMapAddrNeed&&((mac_s<MT_PCI_REMAP_ADDR_1)||(mac_s >MT_PCI_REMAP_ADDR_2+REMAP_2_OFFSET_MASK))){
						UINT32 RemapBase, RemapOffset;
						UINT32 RestoreValue;
						RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
						RemapBase = GET_REMAP_2_BASE(mac_s) << 19;
						RemapOffset = GET_REMAP_2_OFFSET(mac_s);
						RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
						RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, macVal);
						RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
					}else{
						RTMP_IO_WRITE32(pAd, mac_s, macVal);
					}
#else
				RTMP_IO_WRITE32(pAd, mac_s, macVal);
#endif


				sprintf(msg+strlen(msg), "[0x%04x]:%08x  ", mac_s, macVal);
				if (!bFromUI)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("MacAddr=0x%x, MacValue=0x%x\n", mac_s, macVal));
			} else {
				for(IdMac = mac_s; IdMac <= mac_e; IdMac += 4)
				{
#ifdef RTMP_PCI_SUPPORT
					if(IsMapAddrNeed&&((IdMac<MT_PCI_REMAP_ADDR_1)||(IdMac >MT_PCI_REMAP_ADDR_2+REMAP_2_OFFSET_MASK))){
						UINT32 RemapBase, RemapOffset;
						UINT32 RestoreValue;
						RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
						RemapBase = GET_REMAP_2_BASE(IdMac) << 19;
						RemapOffset = GET_REMAP_2_OFFSET(IdMac);
						RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
						RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &macVal);
						RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
					}else{
						RTMP_IO_READ32(pAd, IdMac, &macVal);
					}
#else
					RTMP_IO_READ32(pAd, IdMac, &macVal);
#endif
					sprintf(msg+strlen(msg), "[0x%04x]:%08x  ", IdMac , macVal);
					if (!bFromUI)
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacAddr=0x%x, MacValue=0x%x\n", IdMac, macVal));
				}
			}


			if (ptr)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("NextRound: ptr[%zu]=%s\n", strlen(ptr), ptr));
		}

	}

print_all:
	if (bIsPrintAllMAC)
	{
		mac_s = 0x1000;
		mac_e = 0x1700;

		{
			for(IdMac = mac_s; IdMac < mac_e; IdMac += 4)
			{
				RTMP_IO_READ32(pAd, IdMac, &macValue);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%08x = %08x\n", IdMac, macValue));
			}
		}
	}


	if(strlen(msg) == 1)
		sprintf(msg+strlen(msg), "===>Error command format!");

#ifdef LINUX
	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}
#endif /* LINUX */



	os_free_mem(mpool);
	if (!bFromUI)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<==%s()\n", __FUNCTION__));
}

#endif /* DBG */

INT	Show_WifiSysInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	WifiSysInfoDump(pAd);
	return TRUE;
}



INT	Show_DescInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef RTMP_MAC_PCI
	INT32 i, QueIdx;
	RXD_STRUC *pRxD;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD, RxD;
	TXD_STRUC *pDestTxD, TxD;
#endif /* RT_BIG_ENDIAN */
	RTMP_TX_RING *pTxRing;
	RTMP_MGMT_RING *pMgmtRing = &pAd->MgmtRing;
	RTMP_RX_RING *pRxRing;
#ifdef MT_MAC
	RTMP_BCN_RING *pBcnRing = &pAd->BcnRing;
	RTMP_TX_RING *pTxBmcRing = &pAd->TxBmcRing;
#endif /* MT_MAC */
	RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;
	PUCHAR pDMAHeaderBufVA;

    if (arg != NULL && strlen(arg))
	QueIdx = simple_strtol(arg, 0, 10);
    else
        QueIdx = 0xff;

	switch (QueIdx)
	{
		case 0:
		case 1:
			pTxRing = &pAd->PciHif.TxRing[QueIdx];

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Tx Ring %d ---------------------------------\n", QueIdx));
			for(i = 0; i < TX_RING_SIZE; i++)
			{
				pDMAHeaderBufVA = (UCHAR *)pTxRing->Cell[i].DmaBuf.AllocVa;
#ifdef RT_BIG_ENDIAN
				pDestTxD = (TXD_STRUC *)pTxRing->Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

				if (pDMAHeaderBufVA)
				{
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}
#else
	    		pTxD = (TXD_STRUC *)pTxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
	    		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n",i));
				if (pTxD)
				{
					dump_txd(pAd, pTxD);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXD null!!\n"));
				}

				if (pDMAHeaderBufVA)
				{
					dump_tmac_info(pAd, pDMAHeaderBufVA);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt physical address = %x\n",
								(UINT32)pTxRing->Cell[i].PacketPa));
#ifdef RT_BIG_ENDIAN
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
#endif
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}
			}
			break;

		case 4:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Mgmt Ring ---------------------------------\n"));
			for(i = 0; i < MGMT_RING_SIZE; i++)
			{
				pDMAHeaderBufVA = (UCHAR *)pMgmtRing->Cell[i].DmaBuf.AllocVa;
#ifdef RT_BIG_ENDIAN
				pDestTxD = (TXD_STRUC *)pMgmtRing->Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

				if (pDMAHeaderBufVA)
				{
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}

#else
	    		pTxD = (TXD_STRUC *)pMgmtRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
	    		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n",i));

				if (pTxD)
				{
					dump_txd(pAd, pTxD);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXD null!!!\n"));
				}

				if (pDMAHeaderBufVA)
				{
					dump_tmac_info(pAd, pDMAHeaderBufVA);


					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt physical address = %x\n",
									(UINT32)pMgmtRing->Cell[i].PacketPa));
#ifdef RT_BIG_ENDIAN
				MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
#endif
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}
			}
			break;

#ifdef MT_MAC
		case 8:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BCN ring---------------------------------------------\n"));
			for (i = 0; i < BCN_RING_SIZE; i++)
			{
				pDMAHeaderBufVA = (UCHAR *)pBcnRing->Cell[i].DmaBuf.AllocVa;
#ifdef RT_BIG_ENDIAN
				pDestTxD = (TXD_STRUC *)pBcnRing->Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

				MTMacInfoEndianChange(pAd, (PUCHAR)(pTxD) + pAd->chipCap.tx_hw_hdr_len, TYPE_TMACINFO, 32);

#else
	    		pTxD = (TXD_STRUC *)pBcnRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
	    		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n",i));
				if (pTxD)
				{
					dump_txd(pAd, pTxD);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXD null!!!\n"));
				}

				if (pTxD)
				{
					dump_tmac_info(pAd, (PUCHAR)(pTxD) + pAd->chipCap.tx_hw_hdr_len);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt physical address = %x\n",
									(UINT32)pBcnRing->Cell[i].PacketPa));
#ifdef RT_BIG_ENDIAN
					MTMacInfoEndianChange(pAd, (PUCHAR)(pTxD) + pAd->chipCap.tx_hw_hdr_len, TYPE_TMACINFO, 32);
#endif
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}
			}
			break;
		case 7:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BMC ring---------------------------------------------\n"));
			for (i = 0; i < TX_RING_SIZE; i++)
			{
				pDMAHeaderBufVA = (UCHAR *)pTxBmcRing->Cell[i].DmaBuf.AllocVa;
#ifdef RT_BIG_ENDIAN
				pDestTxD = (TXD_STRUC *)pTxBmcRing->Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

				if (pDMAHeaderBufVA)
				{
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}

#else
	    		pTxD = (TXD_STRUC *)pTxBmcRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
	    		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n",i));
				if (pTxD)
				{
					dump_txd(pAd, pTxD);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXD null!!\n"));
				}

				if (pDMAHeaderBufVA)
				{
					dump_tmac_info(pAd, pDMAHeaderBufVA);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt physical address = %x\n",
									(UINT32)pTxBmcRing->Cell[i].PacketPa));
#ifdef RT_BIG_ENDIAN
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
#endif
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}
			}
			break;
#endif /* MT_MAC */
		case 5:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Control ring---------------------------------------------\n"));
			for(i = 0; i < CTL_RING_SIZE; i++)
			{
				pDMAHeaderBufVA = (UCHAR *)pCtrlRing->Cell[i].DmaBuf.AllocVa;
#ifdef RT_BIG_ENDIAN
				pDestTxD = (TXD_STRUC *)pCtrlRing->Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);

				if (pDMAHeaderBufVA)
				{
					MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}
#else
	    		pTxD = (TXD_STRUC *)pCtrlRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
	    		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n",i));

				if (pTxD)
				{
					dump_txd(pAd, pTxD);
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXD null!!\n"));
				}

				if (pDMAHeaderBufVA)
				{
					dump_tmac_info(pAd, pDMAHeaderBufVA);
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt physical address = %x\n",
									(UINT32)pCtrlRing->Cell[i].PacketPa));
#ifdef RT_BIG_ENDIAN
				MTMacInfoEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, 32);
#endif
				}
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pkt is null\n"));
				}
			}

			break;

		case 12:
			for (QueIdx = 0; QueIdx < NUM_OF_RX_RING; QueIdx++)
			{
				UINT16 RxRingSize = (QueIdx == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
				pRxRing = &pAd->PciHif.RxRing[QueIdx];

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx Ring %d ---------------------------------\n", QueIdx));
				for(i = 0;i < RxRingSize; i++)
				{
#ifdef RT_BIG_ENDIAN
					pDestRxD = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
					RxD = *pDestRxD;
					pRxD = &RxD;
					RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	    			pRxD = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Desc #%d\n",i));

					if (pRxD)
					{
						dump_rxd(pAd, pRxD);
	    				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pRxD->DDONE = %x\n", pRxD->DDONE));
					}
					else
					{
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RXD null!!\n"));
					}
				}
			}
			break;
		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown QueIdx(%d)\n", QueIdx));
			break;
	}
#endif /* RTMP_MAC_PCI */
	return TRUE;
}

/*
    ==========================================================================
    Description:
        Reset statistics counter

    Arguments:
        pAd            Pointer to our adapter
        arg

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ResetStatCounter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
    UINT8 ucBand = BAND0;

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);

    if (ucBand > DBDC_BAND_NUM)
        ucBand = BAND0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_ResetStatCounter_Proc\n"));

	/* add the most up-to-date h/w raw counters into software counters*/
	NICUpdateRawCountersNew(pAd);

	NdisZeroMemory(&pAd->WlanCounters[ucBand], sizeof(COUNTER_802_11));
	NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
	NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));

#ifdef INT_STATISTIC

	pAd->INTCNT = 0;
#ifdef MT_MAC
	pAd->INTWFMACINT0CNT = 0;
	pAd->INTWFMACINT1CNT = 0;
	pAd->INTWFMACINT2CNT = 0;
	pAd->INTWFMACINT3CNT = 0;
	pAd->INTWFMACINT4CNT = 0;
	pAd->INTBCNDLY = 0;
	pAd->INTBMCDLY = 0;
#endif
	pAd->INTTxCoherentCNT = 0;
	pAd->INTRxCoherentCNT = 0;
	pAd->INTFifoStaFullIntCNT = 0;
	pAd->INTMGMTDLYCNT =0;
	pAd->INTRXDATACNT =0;
	pAd->INTRXCMDCNT =0;
	pAd->INTHCCACNT =0;
	pAd->INTAC3CNT =0;
	pAd->INTAC2CNT =0;
	pAd->INTAC1CNT =0;
	pAd->INTAC0CNT =0;

	pAd->INTPreTBTTCNT =0;
	pAd->INTTBTTIntCNT =0;
	pAd->INTGPTimeOutCNT =0;
	pAd->INTAutoWakeupIntCNT =0;
#endif

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	{
	    /* clear TX success/fail count in MCU */
        EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
        MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, ucBand, 0, &rTxStatResult);
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#ifdef CONFIG_ATE
#ifdef CONFIG_QA
	MT_ATEUpdateRxStatistic(pAd, 2, NULL);

#endif /* CONFIG_QA */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgHwTxBfCap)
	{
		int i;
		for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
			NdisZeroMemory(&pAd->MacTab.Content[i].TxBFCounters, sizeof(pAd->MacTab.Content[i].TxBFCounters));
	}
#endif /* TXBF_SUPPORT */

	return TRUE;
}


BOOLEAN RTMPCheckStrPrintAble(
    IN  CHAR *pInPutStr,
    IN  UCHAR strLen)
{
    UCHAR i=0;

    for (i=0; i<strLen; i++)
    {
        if ((pInPutStr[i] < 0x20) || (pInPutStr[i] > 0x7E))
            return FALSE;
    }

    return TRUE;
}


/*
	========================================================================

	Routine Description:
		Remove WPA Key process

	Arguments:
		pAd 					Pointer to our adapter
		pBuf							Pointer to the where the key stored

	Return Value:
		NDIS_SUCCESS					Add key successfully

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/





/*
	========================================================================
	Routine Description:
		Change NIC PHY mode. Re-association may be necessary

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID RTMPSetPhyMode(RTMP_ADAPTER *pAd,struct wifi_dev *wdev,UCHAR phymode)
{
	INT i;
	UCHAR RfIC = wmode_2_rfic(phymode);
	UCHAR Channel = (wdev->channel) ? wdev->channel : HcGetChannelByRf(pAd,RfIC);

	if(HcUpdatePhyMode(pAd,phymode)!=0)
	{

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s(): Set PhyMode=%d failed, Chip not support\n",
				__FUNCTION__, phymode));
		return ;
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd);
#else
	BuildChannelList(pAd);
#endif /* EXT_BUILD_CHANNEL_LIST */

	/* sanity check user setting*/
	for (i = 0; (i < pAd->ChannelListNum) && (i < MAX_NUM_OF_CHANNELS); i++)
	{
		if (Channel== pAd->ChannelList[i].Channel)
			break;
	}

	if (i == pAd->ChannelListNum)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		if (Channel != 0)
		    Channel= GetFirstChByPhyMode(pAd, phymode);
#endif /* CONFIG_AP_SUPPORT */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): channel out of range, use first ch=%d\n",
					__FUNCTION__, Channel));

		HcUpdateChannel(pAd,Channel);

	}

	MlmeUpdateTxRatesWdev(pAd,FALSE,wdev);

//CFG_TODO
#ifdef RT_CFG80211_P2P_SUPPORT
	NdisZeroMemory(pAd->cfg80211_ctrl.P2pSupRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(pAd->cfg80211_ctrl.P2pExtRate, MAX_LEN_OF_SUPPORTED_RATES);

	pAd->cfg80211_ctrl.P2pSupRate[0]  = 0x8C;        /* 6 mbps, in units of 0.5 Mbps, basic rate*/
	pAd->cfg80211_ctrl.P2pSupRate[1]  = 0x12;        /* 9 mbps, in units of 0.5 Mbps*/
	pAd->cfg80211_ctrl.P2pSupRate[2]  = 0x98;        /* 12 mbps, in units of 0.5 Mbps, basic rate*/
	pAd->cfg80211_ctrl.P2pSupRate[3]  = 0x24;        /* 18 mbps, in units of 0.5 Mbps*/
	pAd->cfg80211_ctrl.P2pSupRate[4]  = 0xb0;        /* 24 mbps, in units of 0.5 Mbps, basic rate*/
	pAd->cfg80211_ctrl.P2pSupRate[5]  = 0x48;        /* 36 mbps, in units of 0.5 Mbps*/
	pAd->cfg80211_ctrl.P2pSupRate[6]  = 0x60;        /* 48 mbps, in units of 0.5 Mbps*/
	pAd->cfg80211_ctrl.P2pSupRate[7]  = 0x6c;        /* 54 mbps, in units of 0.5 Mbps*/
	pAd->cfg80211_ctrl.P2pSupRateLen  = 8;

	pAd->cfg80211_ctrl.P2pExtRateLen  = 0;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
    printk("%s: Update for AP\n", __FUNCTION__);
    MlmeUpdateTxRates(pAd, FALSE, MAIN_MBSSID + MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO);

    printk("%s: Update for APCLI\n", __FUNCTION__);
    MlmeUpdateTxRates(pAd, FALSE, MAIN_MBSSID + MIN_NET_DEVICE_FOR_APCLI);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd,wdev);
#endif /* DOT11_N_SUPPORT */

}

VOID RTMPUpdateRateInfo(
	UCHAR phymode,
	struct dev_rate_info *rate
	)
{

	NdisZeroMemory(rate->SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(rate->DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
	switch (phymode) {
		case (WMODE_B):
			rate->SupRate[0]  = 0x82;	  /* 1 mbps, in units of 0.5 Mbps, basic rate */
			rate->SupRate[1]  = 0x84;	  /* 2 mbps, in units of 0.5 Mbps, basic rate */
			rate->SupRate[2]  = 0x8B;	  /* 5.5 mbps, in units of 0.5 Mbps, basic rate */
			rate->SupRate[3]  = 0x96;	  /* 11 mbps, in units of 0.5 Mbps, basic rate */
			rate->SupRateLen  = 4;
			rate->ExtRateLen  = 0;

			rate->DesireRate[0]  = 2;	   /* 1 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[1]  = 4;	   /* 2 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[2]  = 11;    /* 5.5 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[3]  = 22;    /* 11 mbps, in units of 0.5 Mbps*/
			/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_CCK;  This MODE is only FYI. not use*/
			/*update MlmeTransmit rate*/
			rate->MlmeTransmit.field.MCS = MCS_0;
			rate->MlmeTransmit.field.BW = BW_20;
			rate->MlmeTransmit.field.MODE = MODE_CCK;
			break;

		/*
			In current design, we will put supported/extended rate element in
			beacon even we are 11n-only mode.
			Or some 11n stations will not connect to us if we do not put
			supported/extended rate element in beacon.
		*/
		case (WMODE_G):
		case (WMODE_B | WMODE_G):
		case (WMODE_A | WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
		case (WMODE_GN):
		case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
		case (WMODE_B | WMODE_G | WMODE_GN):
		case (WMODE_G | WMODE_GN):
		case (WMODE_B | WMODE_GN):
#endif /* DOT11_N_SUPPORT */
			rate->SupRate[0]  = 0x82;	  /* 1 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[1]  = 0x84;	  /* 2 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[2]  = 0x8B;	  /* 5.5 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[3]  = 0x96;	  /* 11 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[4]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
			rate->SupRate[5]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
			rate->SupRate[6]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
			rate->SupRate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
			rate->SupRateLen  = 8;
			rate->ExtRate[0]  = 0x0C;	  /* 6 mbps, in units of 0.5 Mbps*/
			rate->ExtRate[1]  = 0x18;	  /* 12 mbps, in units of 0.5 Mbps*/
			rate->ExtRate[2]  = 0x30;	  /* 24 mbps, in units of 0.5 Mbps*/
			rate->ExtRate[3]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
			rate->ExtRateLen  = 4;

			rate->DesireRate[0]  = 2;	   /* 1 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[1]  = 4;	   /* 2 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[2]  = 11;    /* 5.5 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[3]  = 22;    /* 11 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[4]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[5]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[6]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[7]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[8]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[9]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[10] = 96;    /* 48 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[11] = 108;   /* 54 mbps, in units of 0.5 Mbps*/

			/*update MlmeTransmit rate*/
			rate->MlmeTransmit.field.MCS = MCS_0;
			rate->MlmeTransmit.field.BW = BW_20;
			rate->MlmeTransmit.field.MODE = MODE_CCK;
			break;

		case (WMODE_A):
#ifdef DOT11_N_SUPPORT
		case (WMODE_A | WMODE_AN):
		case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
		case (WMODE_AN):
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		case (WMODE_A | WMODE_AN | WMODE_AC):
		case (WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
			rate->SupRate[0]  = 0x8C;	  /* 6 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[1]  = 0x12;	  /* 9 mbps, in units of 0.5 Mbps*/
			rate->SupRate[2]  = 0x98;	  /* 12 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[3]  = 0x24;	  /* 18 mbps, in units of 0.5 Mbps*/
			rate->SupRate[4]  = 0xb0;	  /* 24 mbps, in units of 0.5 Mbps, basic rate*/
			rate->SupRate[5]  = 0x48;	  /* 36 mbps, in units of 0.5 Mbps*/
			rate->SupRate[6]  = 0x60;	  /* 48 mbps, in units of 0.5 Mbps*/
			rate->SupRate[7]  = 0x6c;	  /* 54 mbps, in units of 0.5 Mbps*/
			rate->SupRateLen  = 8;
			rate->ExtRateLen  = 0;

			rate->DesireRate[0]  = 12;    /* 6 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[1]  = 18;    /* 9 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[2]  = 24;    /* 12 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[3]  = 36;    /* 18 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[4]  = 48;    /* 24 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[5]  = 72;    /* 36 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[6]  = 96;    /* 48 mbps, in units of 0.5 Mbps*/
			rate->DesireRate[7]  = 108;   /* 54 mbps, in units of 0.5 Mbps*/
			/*pAd->CommonCfg.HTPhyMode.field.MODE = MODE_OFDM;  This MODE is only FYI. not use*/
			/*update MlmeTransmit rate*/
			rate->MlmeTransmit.field.MCS = MCS_RATE_6;
			rate->MlmeTransmit.field.BW = BW_20;
			rate->MlmeTransmit.field.MODE = MODE_OFDM;
			break;

		default:
			break;
	}
}

/*
	========================================================================
	Description:
		Add Client security information into ASIC WCID table and IVEIV table.
    Return:
	========================================================================
*/
VOID RTMPAddWcidAttributeEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR BssIdx,
	IN UCHAR KeyIdx,
	IN UCHAR CipherAlg,
	IN MAC_TABLE_ENTRY *pEntry)
{
	UINT32		WCIDAttri = 0;
	USHORT		offset;
	UCHAR		IVEIV = 0;
	USHORT		Wcid = 0;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN		IEEE8021X = FALSE;
    struct wifi_dev *wdev = NULL;
#endif /* CONFIG_AP_SUPPORT */


	// TODO: shiang-7603!! fix me
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MT7603 Not support yet!\n", __FUNCTION__));
		return;
	}

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_SUPPORT
			if (BssIdx >= MIN_NET_DEVICE_FOR_APCLI)
			{
				if (pEntry)
					BssIdx -= MIN_NET_DEVICE_FOR_APCLI;
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("RTMPAddWcidAttributeEntry: AP-Client link doesn't need to set Group WCID Attribute. \n"));
					return;
				}
			}
			else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
			if (BssIdx >= MIN_NET_DEVICE_FOR_WDS)
			{
				if (pEntry)
					BssIdx = BSS0;
				else
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("RTMPAddWcidAttributeEntry: WDS link doesn't need to set Group WCID Attribute. \n"));
					return;
				}
			}
			else
#endif /* WDS_SUPPORT */
			{
				if (BssIdx >= pAd->ApCfg.BssidNum)
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPAddWcidAttributeEntry: The BSS-index(%d) is out of range for MBSSID link. \n", BssIdx));
					return;
				}
			}

			/* choose wcid number*/
			if (pEntry)
				Wcid = pEntry->wcid;
			else {
				if (BssIdx >= HW_BEACON_MAX_NUM) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPAddWcidAttributeEntry: The BSS-index(%d) is out of range for MBSSID link. \n", BssIdx));
					return;
				}
                wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
                GET_GroupKey_WCID(wdev, Wcid);
            }

#ifdef DOT1X_SUPPORT
			if ((BssIdx < pAd->ApCfg.BssidNum) && (BssIdx < MAX_MBSSID_NUM(pAd)) && (BssIdx < HW_BEACON_MAX_NUM))
			{
				if (!pEntry) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPAddWcidAttributeEntry: pEntry is Null\n"));
					return;
				}
				IEEE8021X = pEntry->SecConfig.IEEE8021X;
			}
#endif /* DOT1X_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* Update WCID attribute table*/
	{
		UINT32 wcid_attr_base = 0, wcid_attr_size = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			wcid_attr_base = MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = HW_WCID_ATTRI_SIZE;
		}
#endif /* RTMP_MAC */

		offset = wcid_attr_base + (Wcid * wcid_attr_size);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/*
				1.	Wds-links and Mesh-links always use Pair-wise key table.
				2. 	When the CipherAlg is TKIP, AES or the dynamic WEP is enabled,
					it needs to set key into Pair-wise Key Table.
				3.	The pair-wise key security mode is set NONE, it means as no security.
			*/
			if (pEntry && (IS_ENTRY_WDS(pEntry) || IS_ENTRY_MESH(pEntry)))
				WCIDAttri = (BssIdx<<4) | (CipherAlg<<1) | PAIRWISEKEYTABLE;
			else if ((pEntry) &&
					((CipherAlg == CIPHER_TKIP) ||
					 (CipherAlg == CIPHER_AES) ||
					 (CipherAlg == CIPHER_NONE) ||
					 (IEEE8021X == TRUE)))
				WCIDAttri = (BssIdx<<4) | (CipherAlg<<1) | PAIRWISEKEYTABLE;
			else
				WCIDAttri = (BssIdx<<4) | (CipherAlg<<1) | SHAREDKEYTABLE;
		}
#endif /* CONFIG_AP_SUPPORT */


		RTMP_IO_WRITE32(pAd, offset, WCIDAttri);
	}

	/* Update IV/EIV table*/
	{
		UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

		offset = iveiv_tb_base + (Wcid * iveiv_tb_size);

		/* WPA mode*/
		if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_AES))
		{
			/* Eiv bit on. keyid always is 0 for pairwise key */
			IVEIV = (KeyIdx <<6) | 0x20;
		}
		else
		{
			/* WEP KeyIdx is default tx key. */
			IVEIV = (KeyIdx << 6);
		}

		/* For key index and ext IV bit, so only need to update the position(offset+3).*/
#ifdef RTMP_MAC_PCI
		RTMP_IO_WRITE8(pAd, offset+3, IVEIV);
#endif /* RTMP_MAC_PCI */
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTMPAddWcidAttributeEntry: WCID #%d, KeyIndex #%d, Alg=%s\n",Wcid, KeyIdx, CipherName[CipherAlg]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("	WCIDAttri = 0x%x \n",  WCIDAttri));

}

/*
    ==========================================================================
    Description:
        Parse encryption type
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
    ==========================================================================
*/
RTMP_STRING *GetEncryptType(CHAR enc)
{
    if(enc == Ndis802_11WEPDisabled)
        return "NONE";
    if(enc == Ndis802_11WEPEnabled)
    	return "WEP";
    if(enc == Ndis802_11TKIPEnable)
    	return "TKIP";
    if(enc == Ndis802_11AESEnable)
    	return "AES";
	if(enc == Ndis802_11TKIPAESMix)
    	return "TKIPAES";
#ifdef WAPI_SUPPORT
	if(enc == Ndis802_11EncryptionSMS4Enabled)
    	return "SMS4";
#endif /* WAPI_SUPPORT */
    else
    	return "UNKNOW";
}

RTMP_STRING *GetAuthMode(CHAR auth)
{
    if(auth == Ndis802_11AuthModeOpen)
    	return "OPEN";
    if(auth == Ndis802_11AuthModeShared)
    	return "SHARED";
	if(auth == Ndis802_11AuthModeAutoSwitch)
    	return "AUTOWEP";
    if(auth == Ndis802_11AuthModeWPA)
    	return "WPA";
    if(auth == Ndis802_11AuthModeWPAPSK)
    	return "WPAPSK";
    if(auth == Ndis802_11AuthModeWPANone)
    	return "WPANONE";
    if(auth == Ndis802_11AuthModeWPA2)
    	return "WPA2";
    if(auth == Ndis802_11AuthModeWPA2PSK)
    	return "WPA2PSK";
	if(auth == Ndis802_11AuthModeWPA1WPA2)
    	return "WPA1WPA2";
	if(auth == Ndis802_11AuthModeWPA1PSKWPA2PSK)
    	return "WPA1PSKWPA2PSK";
#ifdef WAPI_SUPPORT
	if(auth == Ndis802_11AuthModeWAICERT)
    	return "WAI-CERT";
	if(auth == Ndis802_11AuthModeWAIPSK)
    	return "WAI-PSK";
#endif /* WAPI_SUPPORT */

    	return "UNKNOW";
}


/*
    ==========================================================================
    Description:
        Get site survey results
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage:
        		1.) UI needs to wait 4 seconds after issue a site survey command
        		2.) iwpriv ra0 get_site_survey
        		3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
#ifndef WH_EZ_SETUP
#define	LINE_LEN	(4+33+20+23+9+9+7+3)	/* Channel+SSID+Bssid+Security+Signal+WiressMode+ExtCh+NetworkType*/
#endif

VOID RTMPCommSiteSurveyData(
	IN  RTMP_STRING *msg,
	IN  BSS_ENTRY *pBss,
	IN  UINT32 MsgLen)
{
	INT         Rssi = 0;
	UINT        Rssi_Quality = 0;
	NDIS_802_11_NETWORK_TYPE    wireless_mode;
	CHAR		Ssid[MAX_LEN_OF_SSID +1];
	RTMP_STRING SecurityStr[32] = {0};

		/*Channel*/
		sprintf(msg+strlen(msg),"%-4d", pBss->Channel);


		/*SSID*/
	NdisZeroMemory(Ssid, (MAX_LEN_OF_SSID +1));
	if (RTMPCheckStrPrintAble((PCHAR)pBss->Ssid, pBss->SsidLen))
		NdisMoveMemory(Ssid, pBss->Ssid, pBss->SsidLen);
	else
	{
		INT idx = 0;
		sprintf(Ssid, "0x");
		for (idx = 0; (idx < 15) && (idx < pBss->SsidLen); idx++)
			sprintf(Ssid + 2 + (idx*2), "%02X", (UCHAR)pBss->Ssid[idx]);
	}
		sprintf(msg+strlen(msg),"%-33s", Ssid);

		/*BSSID*/
		sprintf(msg+strlen(msg),"%02x:%02x:%02x:%02x:%02x:%02x   ",
			pBss->Bssid[0],
			pBss->Bssid[1],
			pBss->Bssid[2],
			pBss->Bssid[3],
			pBss->Bssid[4],
			pBss->Bssid[5]);

	/*Security*/
	RTMPZeroMemory(SecurityStr, 32);
	sprintf(SecurityStr, "%s/%s", GetAuthModeStr(pBss->AKMMap), GetEncryModeStr(pBss->PairwiseCipher));

	sprintf(msg+strlen(msg), "%-23s", SecurityStr);

		/* Rssi*/
		Rssi = (INT)pBss->Rssi;
		if (Rssi >= -50)
			Rssi_Quality = 100;
		else if (Rssi >= -80)    /* between -50 ~ -80dbm*/
			Rssi_Quality = (UINT)(24 + ((Rssi + 80) * 26)/10);
		else if (Rssi >= -90)   /* between -80 ~ -90dbm*/
			Rssi_Quality = (UINT)(((Rssi + 90) * 26)/10);
		else    /* < -84 dbm*/
			Rssi_Quality = 0;
		sprintf(msg+strlen(msg),"%-9d", Rssi_Quality);

		/* Wireless Mode*/
		wireless_mode = NetworkTypeInUseSanity(pBss);
		if (wireless_mode == Ndis802_11FH ||
			wireless_mode == Ndis802_11DS)
			sprintf(msg+strlen(msg),"%-9s", "11b");
		else if (wireless_mode == Ndis802_11OFDM5)
			sprintf(msg+strlen(msg),"%-9s", "11a");
		else if (wireless_mode == Ndis802_11OFDM5_N)
			sprintf(msg+strlen(msg),"%-9s", "11a/n");
		else if (wireless_mode == Ndis802_11OFDM5_AC)
			sprintf(msg+strlen(msg),"%-9s", "11a/n/ac");
		else if (wireless_mode == Ndis802_11OFDM24)
			sprintf(msg+strlen(msg),"%-9s", "11b/g");
		else if (wireless_mode == Ndis802_11OFDM24_N)
			sprintf(msg+strlen(msg),"%-9s", "11b/g/n");
		else
			sprintf(msg+strlen(msg),"%-9s", "unknow");

		/* Ext Channel*/
		if (pBss->AddHtInfoLen > 0)
		{
			if (pBss->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)
				sprintf(msg+strlen(msg),"%-7s", " ABOVE");
			else if (pBss->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW)
				sprintf(msg+strlen(msg),"%-7s", " BELOW");
			else
				sprintf(msg+strlen(msg),"%-7s", " NONE");
		}
		else
		{
			sprintf(msg+strlen(msg),"%-7s", " NONE");
		}

		/*Network Type		*/
		if (pBss->BssType == BSS_ADHOC)
			sprintf(msg+strlen(msg),"%-3s", " Ad");
		else
			sprintf(msg+strlen(msg),"%-3s", " In");

//        sprintf(msg+strlen(msg),"\n");

	return;
}

#ifndef WH_EZ_SETUP
static
#endif
BOOLEAN ascii2int(RTMP_STRING *in, UINT32 *out)
{
	UINT32 decimal_val, val;
	CHAR *p, asc_val;

	decimal_val = 0;
	p = (char *)in;
	while((*p) != 0)
	{
		val = 0;
		asc_val = *p;

		if ((asc_val >= '0') && (asc_val <= '9'))
			val = asc_val - 48;
		else
			return FALSE;

		decimal_val = (decimal_val * 10) + val;
		p++;
	}
	*out = decimal_val;

	return TRUE;
}

#if defined (AP_SCAN_SUPPORT) || defined (CONFIG_STA_SUPPORT)
VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	RTMP_STRING *msg;
	INT 		i=0;
	INT			WaitCnt;
	INT 		Status=0;
    INT         max_len = LINE_LEN;
	RTMP_STRING *this_char;
	UINT32		bss_start_idx;
	BSS_ENTRY *pBss;
	UINT32 TotalLen, BufLen = IW_SCAN_MAX_DATA;
	BSS_TABLE *pScanTab;
#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT) 
    POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	INT allow_2G_entry = 0;
	INT allow_5G_entry = 0;
	INT bss_2G_cnt = 0;
	INT bss_5G_cnt = 0;
	INT output_no = 0;
	INT total_bss_entry_cnt = 0;
#endif


	if (pAdapter->CommonCfg.dbdc_mode == 1)
	{
#ifdef CONFIG_AP_SUPPORT
		if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		{
			if (WMODE_CAP_2G(pAdapter->ApCfg.MBSSID[pObj->ioctl_if].wdev.PhyMode)) 
				allow_2G_entry = 1;
			if (WMODE_CAP_5G(pAdapter->ApCfg.MBSSID[pObj->ioctl_if].wdev.PhyMode)) 
				allow_5G_entry = 1;
		}
#ifdef APCLI_SUPPORT
		else if (pObj->ioctl_if_type == INT_APCLI)
		{
			if (WMODE_CAP_2G(pAdapter->ApCfg.ApCliTab[pObj->ioctl_if].wdev.PhyMode)) 
				allow_2G_entry = 1;
			if (WMODE_CAP_5G(pAdapter->ApCfg.ApCliTab[pObj->ioctl_if].wdev.PhyMode)) 
				allow_5G_entry = 1;
		} 
#endif /*APCLI_SUPPORT*/
		else
#endif /*CONFIG_AP_SUPPORT*/
		{
			allow_2G_entry = 1;
			allow_5G_entry = 1;
		}
	} 
	else 
	{
		allow_2G_entry = 1;
		allow_5G_entry = 1;
	}
        if(wrq->u.data.length==0)
         wrq->u.data.length+=1;
       os_alloc_mem(NULL, (PUCHAR *)&this_char, wrq->u.data.length);
    if(copy_from_user(this_char,wrq->u.data.pointer,wrq->u.data.length))
       {
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): copy from user failed \n" 
		                                     , __FUNCTION__));
       }
	  MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Before check, this_char = %s\n" 
			 , __FUNCTION__, this_char));
	if (ascii2int(this_char, &bss_start_idx) == FALSE)
		bss_start_idx = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): After check, this_char = %s, out = %d\n" 
			 , __FUNCTION__, this_char, bss_start_idx));

	TotalLen = sizeof(CHAR)*((MAX_LEN_OF_BSS_TABLE)*max_len) + 100;

		BufLen = IW_SCAN_MAX_DATA;

	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n"));
		return;
	}

	memset(msg, 0 , TotalLen);
	for(i=0; i<pAdapter->ScanTab.BssNr ;i++)
	{
		pBss = &pAdapter->ScanTab.BssEntry[i];
		
		if( pBss->Channel > 14)
			bss_5G_cnt++;
		else
			bss_2G_cnt++;
	}
	//sanity check for bss_start_idx
	if ((allow_5G_entry == 1) && (allow_2G_entry == 1))
		total_bss_entry_cnt = pAdapter->ScanTab.BssNr;
	else if ((allow_5G_entry == 0) && (allow_2G_entry == 1))
		total_bss_entry_cnt = bss_2G_cnt;
	else if ((allow_5G_entry == 1) && (allow_2G_entry == 0))
		total_bss_entry_cnt = bss_5G_cnt;

	if (total_bss_entry_cnt ==0) {
		sprintf(msg,"No BssInfo\n");
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n", wrq->u.data.length));
		os_free_mem((PUCHAR)msg);
		return;
	}
	if (bss_start_idx > (total_bss_entry_cnt-1)) {
		sprintf(msg,"BssInfo Idx(%d) is out of range(0~%d)\n", 
				bss_start_idx, (total_bss_entry_cnt-1));
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n", wrq->u.data.length));
		os_free_mem((PUCHAR)msg);
		return;
	}

	sprintf(msg,"%s","\n");
//	sprintf(msg+strlen(msg),"Total=%-4d, 2G=%d, 5G=%d",pAdapter->ScanTab.BssNr,bss_2G_cnt,bss_5G_cnt);
//	sprintf(msg+strlen(msg),"%s","\n");
	sprintf(msg+strlen(msg),"%-4s%-33s%-20s%-23s%-9s%-9s%-7s%-3s\n",
	    "Ch", "SSID", "BSSID", "Security", "Signal(%)", "W-Mode", " ExtCH"," NT");

#ifdef WSC_INCLUDED
	sprintf(msg+strlen(msg)-1,"%-4s%-5s\n", " WPS", " DPID");
#endif /* WSC_INCLUDED */

//	sprintf(msg+strlen(msg)-1,"%-8s\n", " BcnRept");

#ifdef MWDS
	sprintf(msg+strlen(msg)-1,"%-8s\n", " MWDSCap");
#endif /* MWDS */


	WaitCnt = 0;

	while ((ScanRunning(pAdapter) == TRUE) && (WaitCnt++ < 200))
		OS_WAIT(500);

	pScanTab = &pAdapter->ScanTab;
	BssTableSortByRssi(pScanTab,FALSE);

	for(i=0; i < pAdapter->ScanTab.BssNr ;i++)
	{
		pBss = &pAdapter->ScanTab.BssEntry[i];

		if( pBss->Channel==0)
			break;

		if((strlen(msg)+100 ) >= BufLen)
			break;

		if (pBss->Channel > 14)
		{
			if (allow_5G_entry == 0) 
				continue;
		} 
		else
		{
			if (allow_2G_entry == 0) 
				continue;
		}
		if (output_no < bss_start_idx) 
		{
			output_no++;
			continue;
		}
		/*No*/
		//sprintf(msg+strlen(msg),"%-4d", output_no);
		output_no++;

		RTMPCommSiteSurveyData(msg, pBss, TotalLen);

#ifdef WSC_INCLUDED
        /*WPS*/
        if (pBss->WpsAP & 0x01)
			sprintf(msg+strlen(msg)-1,"%-4s", " YES");
		else
			sprintf(msg+strlen(msg)-1,"%-4s", "  NO");

		if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PIN)
			sprintf(msg+strlen(msg),"%-5s", " PIN");
		else if (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PBC)
			sprintf(msg+strlen(msg),"%-5s", " PBC");
		else
			sprintf(msg+strlen(msg),"%-5s", " ");
#endif /* WSC_INCLUDED */

#ifndef MWDS
	//	sprintf(msg+strlen(msg),"%-8s\n", pBss->FromBcnReport ? " YES" : " NO");
		sprintf(msg+strlen(msg),"\n");
#else
		sprintf(msg+strlen(msg),"%-7s", pBss->FromBcnReport ? " YES" : " NO");

        if (pBss->bSupportMWDS)
            sprintf(msg+strlen(msg),"%-4s\n", " YES");
        else
            sprintf(msg+strlen(msg),"%-4s\n", " NO");

#endif /* MWDS */

	}

	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n", wrq->u.data.length));
	os_free_mem((PUCHAR)msg);
}
#endif

USHORT RTMPGetLastTxRate(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
        EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
        HTTRANSMIT_SETTING lastTxRate;
        MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
        lastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
        lastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
        lastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1:0;
        lastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1:0;
        lastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;
        if (lastTxRate.field.MODE == MODE_VHT)
        {
                lastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
        }
        else if (lastTxRate.field.MODE == MODE_OFDM)
        {
                lastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
        }
        else
        {
                lastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;
        }
        return lastTxRate.word;
}

static VOID
copy_mac_table_entry(RT_802_11_MAC_ENTRY *pDst, MAC_TABLE_ENTRY *pEntry)
{
	pDst->ApIdx = (UCHAR)pEntry->func_tb_idx;
	COPY_MAC_ADDR(pDst->Addr, &pEntry->Addr);
	pDst->Aid = (UCHAR)pEntry->Aid;
	pDst->Psm = pEntry->PsMode;

#ifdef DOT11_N_SUPPORT
	pDst->MimoPs = pEntry->MmpsMode;
#endif /* DOT11_N_SUPPORT */

	/* Fill in RSSI per entry*/
	pDst->AvgRssi0 = pEntry->RssiSample.AvgRssi[0];
	pDst->AvgRssi1 = pEntry->RssiSample.AvgRssi[1];
	pDst->AvgRssi2 = pEntry->RssiSample.AvgRssi[2];

	/* the connected time per entry*/
	pDst->ConnectedTime = pEntry->StaConnectTime;

	pDst->TxRate.word = pEntry->HTPhyMode.word;
	pDst->LastRxRate = pEntry->LastRxRate;
}

VOID RTMPIoctlGetMacTableStaInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT i, MacTabWCID;
	RT_802_11_MAC_TABLE *pMacTab = NULL;
	PRT_802_11_MAC_ENTRY pDst;
	MAC_TABLE_ENTRY *pEntry;
	UINT16 wrq_len = wrq->u.data.length;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	wrq->u.data.length = 0;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
	{
		STA_TR_ENTRY *tr_entry;
		
		if (wrq_len < sizeof(RT_802_11_MAC_ENTRY))
			return;
		if (pObj->ioctl_if >= MAX_APCLI_NUM)
			return;
		if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].CtrlCurrState != APCLI_CTRL_CONNECTED)
			return;
		MacTabWCID = pAd->ApCfg.ApCliTab[pObj->ioctl_if].MacTabWCID;
		if (!VALID_WCID(MacTabWCID))
			return;
		if (!VALID_TR_WCID(MacTabWCID))
			return;
		pEntry = &pAd->MacTab.Content[MacTabWCID];
		tr_entry = &pAd->MacTab.tr_entry[MacTabWCID];
		if (IS_ENTRY_APCLI(pEntry) && (pEntry->Sst == SST_ASSOC) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
		{
			RT_802_11_MAC_ENTRY MacEntry;
			
			pDst = &MacEntry;
			copy_mac_table_entry(pDst, pEntry);
			
			wrq->u.data.length = sizeof(RT_802_11_MAC_ENTRY);
			copy_to_user(wrq->u.data.pointer, pDst, wrq->u.data.length);
		}
		
		return;
	}
#endif

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pMacTab, sizeof(RT_802_11_MAC_TABLE));
	if (pMacTab == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(pMacTab, sizeof(RT_802_11_MAC_TABLE));
	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		pEntry = &(pAd->MacTab.Content[i]);
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
			pDst = &pMacTab->Entry[pMacTab->Num];

			pDst->ApIdx = pEntry->func_tb_idx;
			COPY_MAC_ADDR(pDst->Addr, &pEntry->Addr);
			pDst->Aid = (UCHAR)pEntry->Aid;
			pDst->Psm = pEntry->PsMode;

#ifdef DOT11_N_SUPPORT
			pDst->MimoPs = pEntry->MmpsMode;
#endif /* DOT11_N_SUPPORT */

			/* Fill in RSSI per entry*/
			pDst->AvgRssi0 = pEntry->RssiSample.AvgRssi[0];
			pDst->AvgRssi1 = pEntry->RssiSample.AvgRssi[1];
			pDst->AvgRssi2 = pEntry->RssiSample.AvgRssi[2];

			/* the connected time per entry*/
			pDst->ConnectedTime = pEntry->StaConnectTime;
                        pDst->TxRate.word = RTMPGetLastTxRate(pAd,pEntry);

			pDst->LastRxRate = pEntry->LastRxRate;

			pMacTab->Num += 1;
		}
	}

	wrq->u.data.length = sizeof(RT_802_11_MAC_TABLE);
	if (copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	if (pMacTab != NULL)
		os_free_mem(pMacTab);
}

#define	MAC_LINE_LEN	(1+14+4+4+4+4+10+10+10+6+6)	/* "\n"+Addr+aid+psm+datatime+rxbyte+txbyte+current tx rate+last tx rate+"\n" */
VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT i;
/*	RT_802_11_MAC_TABLE MacTab;*/
	RT_802_11_MAC_TABLE *pMacTab = NULL;
	RT_802_11_MAC_ENTRY *pDst;
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	char *msg;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pMacTab, sizeof(RT_802_11_MAC_TABLE));
	if (pMacTab == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory(pMacTab, sizeof(RT_802_11_MAC_TABLE));
	pMacTab->Num = 0;
	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		pEntry = &(pAd->MacTab.Content[i]);
		tr_entry = &(pAd->MacTab.tr_entry[i]);
                if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
			if(pMacTab->Num >= MAX_NUMBER_OF_MAC)
			{
				break;
			}
			pDst = &pMacTab->Entry[pMacTab->Num];


			pDst->ApIdx = (UCHAR)pEntry->func_tb_idx;
			COPY_MAC_ADDR(pDst->Addr, &pEntry->Addr);
			pDst->Aid = (UCHAR)pEntry->Aid;
			pDst->Psm = pEntry->PsMode;
#ifdef DOT11_N_SUPPORT
			pDst->MimoPs = pEntry->MmpsMode;
#endif /* DOT11_N_SUPPORT */

			/* Fill in RSSI per entry*/
			pDst->AvgRssi0 = pEntry->RssiSample.AvgRssi[0];
			pDst->AvgRssi1 = pEntry->RssiSample.AvgRssi[1];
			pDst->AvgRssi2 = pEntry->RssiSample.AvgRssi[2];

			/* the connected time per entry*/
			pDst->ConnectedTime = pEntry->StaConnectTime;
                        pDst->TxRate.word = RTMPGetLastTxRate(pAd,pEntry);

			pMacTab->Num += 1;
		}
	}

	wrq->u.data.length = sizeof(RT_802_11_MAC_TABLE);
	if (copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR)*(GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN));
	if (msg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		goto LabelOK;
	}
	memset(msg, 0 , GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN );
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"%-14s%-4s%-4s%-4s%-4s%-6s%-6s%-10s%-10s%-10s\n",
		"MAC", "AP",  "AID", "PSM", "AUTH", "CTxR", "LTxR","LDT", "RxB", "TxB");

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
			if((strlen(msg)+MAC_LINE_LEN ) >= (GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN) )
				break;
			sprintf(msg+strlen(msg),"%02x%02x%02x%02x%02x%02x  ", PRINT_MAC(pEntry->Addr));
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->func_tb_idx);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->Aid);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->PsMode);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->AuthState);
			sprintf(msg+strlen(msg),"%-6d",RateIdToMbps[pAd->MacTab.Content[i].CurrTxRate]);
			sprintf(msg+strlen(msg),"%-6d",0/*RateIdToMbps[pAd->MacTab.Content[i].HTPhyMode.word]*/); /* ToDo*/
			sprintf(msg+strlen(msg),"%-10d",0/*pAd->MacTab.Content[i].HSCounter.LastDataPacketTime*/); /* ToDo*/
			sprintf(msg+strlen(msg),"%-10d",0/*pAd->MacTab.Content[i].HSCounter.TotalRxByteCount*/); /* ToDo*/
			sprintf(msg+strlen(msg),"%-10d\n",0/*pAd->MacTab.Content[i].HSCounter.TotalTxByteCount*/); /* ToDo*/

		}
	}
	/* for compatible with old API just do the printk to console*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s", msg));
	os_free_mem( msg);

LabelOK:
	if (pMacTab != NULL)
		os_free_mem( pMacTab);
}

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
#ifdef CONFIG_AP_SUPPORT
VOID RTMPAR9IoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT i;
	char *msg;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR)*(GET_MAX_UCAST_NUM(pAd)*MAC_LINE_LEN));
	if (msg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return;
	}
	memset(msg, 0, GET_MAX_UCAST_NUM(pAd) * MAC_LINE_LEN);
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"%-14s%-4s%-4s%-4s%-4s%-6s%-6s%-10s%-10s%-10s\n",
		"MAC", "AP",  "AID", "PSM", "AUTH", "CTxR", "LTxR","LDT", "RxB", "TxB");

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
			if((strlen(msg)+MAC_LINE_LEN ) >= (GET_MAX_UCAST_NUM(pAd) * MAC_LINE_LEN) )
				break;
			sprintf(msg+strlen(msg),"%02x%02x%02x%02x%02x%02x  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->func_tb_idx);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->Aid);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->PsMode);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->AuthState);
			sprintf(msg+strlen(msg),"%-6d",RateIdToMbps[pAd->MacTab.Content[i].CurrTxRate]);
			sprintf(msg+strlen(msg),"%-6d",0/*RateIdToMbps[pAd->MacTab.Content[i].HTPhyMode.word]*/); /* ToDo*/
			sprintf(msg+strlen(msg),"%-10d",0/*pAd->MacTab.Content[i].HSCounter.LastDataPacketTime*/); /* ToDo*/
			sprintf(msg+strlen(msg),"%-10d",0/*pAd->MacTab.Content[i].HSCounter.TotalRxByteCount*/); /* ToDo*/
			sprintf(msg+strlen(msg),"%-10d\n",0/*pAd->MacTab.Content[i].HSCounter.TotalTxByteCount*/); /* ToDo*/

		}
	}
	/* for compatible with old API just do the printk to console*/
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));
	}

	os_free_mem(msg);
}

VOID RTMPIoctlGetSTAT2(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT apidx;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR)*(pAd->ApCfg.BssidNum*(14*128)));
	if (msg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return;
	}
	memset(msg, 0 ,pAd->ApCfg.BssidNum*(14*128));
	sprintf(msg,"%s","\n");

	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		pMbss=&pAd->ApCfg.MBSSID[apidx];

		sprintf(msg+strlen(msg),"ra%d\n",apidx);
		sprintf(msg+strlen(msg),"bytesTx = %ld\n",(pMbss->TransmittedByteCount));
		sprintf(msg+strlen(msg),"bytesRx = %ld\n",(pMbss->ReceivedByteCount));
		sprintf(msg+strlen(msg),"pktsTx = %ld\n",pMbss->TxCount);
		sprintf(msg+strlen(msg),"pktsRx = %ld\n",pMbss->RxCount);
		sprintf(msg+strlen(msg),"errorsTx = %ld\n",pMbss->TxErrorCount);
		sprintf(msg+strlen(msg),"errorsRx = %ld\n",pMbss->RxErrorCount);
		sprintf(msg+strlen(msg),"discardPktsTx = %ld\n",pMbss->TxDropCount);
		sprintf(msg+strlen(msg),"discardPktsRx = %ld\n",pMbss->RxDropCount);
		sprintf(msg+strlen(msg),"ucPktsTx = %ld\n",pMbss->ucPktsTx);
		sprintf(msg+strlen(msg),"ucPktsRx = %ld\n",pMbss->ucPktsRx);
		sprintf(msg+strlen(msg),"mcPktsTx = %ld\n",pMbss->mcPktsTx);
		sprintf(msg+strlen(msg),"mcPktsRx = %ld\n",pMbss->mcPktsRx);
		sprintf(msg+strlen(msg),"bcPktsTx = %ld\n",pMbss->bcPktsTx);
		sprintf(msg+strlen(msg),"bcPktsRx = %ld\n",pMbss->bcPktsRx);

	}

	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));
	}

	os_free_mem(msg);
}


VOID RTMPIoctlGetRadioDynInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT status,bandwidth,ShortGI;
	struct wifi_dev *wdev;
	UCHAR op_ht_bw;

	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(CHAR)*(4096));
	if (msg == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return;
	}
	memset(msg, 0 ,4096);
	sprintf(msg,"%s","\n");


		pMbss=&pAd->ApCfg.MBSSID[0];
		wdev = &pMbss->wdev;
		op_ht_bw = wlan_operate_get_ht_bw(wdev);

		if (IsHcRadioCurStatOffByWdev(wdev))
			status = 0;
		else
			status = 1;

		if(op_ht_bw  == BW_40)
			bandwidth = 1;
		else
			bandwidth = 0;

		if(pAd->CommonCfg.RegTransmitSetting.field.ShortGI == GI_800)
			ShortGI = 1;
		else
			ShortGI = 0;


		sprintf(msg+strlen(msg),"status = %d\n",status);
		sprintf(msg+strlen(msg),"channelsInUse = %d\n",pAd->ChannelListNum);
		sprintf(msg+strlen(msg),"channel = %d\n",wdev->channel);
		sprintf(msg+strlen(msg),"chanWidth = %d\n",bandwidth);
		sprintf(msg+strlen(msg),"guardIntvl = %d\n",ShortGI);
		sprintf(msg+strlen(msg),"MCS = %d\n",wdev->DesiredTransmitSetting.field.MCS);

	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", msg));
	}

	os_free_mem( msg);
}
#endif/*CONFIG_AP_SUPPORT*/
#endif/*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */

#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UCHAR mac[6], tid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

/*
	The BASetup inupt string format should be xx:xx:xx:xx:xx:xx-d,
		=>The six 2 digit hex-decimal number previous are the Mac address,
		=>The seventh decimal number is the tid value.
*/
	/*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/

	if(strlen(arg) < 19)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = (UCHAR) simple_strtol((token+1), 0, 10);
		/* tid is 0 ~ 7; Or kernel will crash in BAOriSessionSetUp() */
		if (tid > (NUM_OF_TID-1))
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], tid));

#ifdef CONFIG_AP_SUPPORT
	    pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif

    	if (pEntry) {
        	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nSetup BA Session: Tid = %d\n", tid));
	        BAOriSessionSetUp(pAd, pEntry, tid, 0, 100, TRUE);
    	}

		return TRUE;
	}

	return FALSE;

}

INT	Set_BADecline_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG bBADecline;

	bBADecline = simple_strtol(arg, 0, 10);

	if (bBADecline == 0)
	{
		pAd->CommonCfg.bBADecline = FALSE;
	}
	else if (bBADecline == 1)
	{
		pAd->CommonCfg.bBADecline = TRUE;
	}
	else
	{
		return FALSE; /*Invalid argument*/
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BADecline_Proc::(BADecline=%d)\n", pAd->CommonCfg.bBADecline));

	return TRUE;
}

INT	Set_BAOriTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UCHAR mac[6], tid;
	UCHAR wcid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    /*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/
/*
	The BAOriTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
		=>The six 2 digit hex-decimal number previous are the Mac address,
		=>The seventh decimal number is the tid value.
*/
    if(strlen(arg) < 19)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
	{
		//another acceptable format wcid-tid
		token = strchr(arg, DASH);
		if ((token != NULL) && (strlen(token)>1))
		{
			tid = simple_strtol((token+1), 0, 10);
			/* tid will be 0 ~ 7; Or kernel will crash in BARecSessionTearDown() */
			if (tid > (NUM_OF_TID-1)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("tid=%d is wrong\n\r",tid));
				return FALSE;
			}
			*token = '\0';
			wcid = simple_strtol(arg, 0, 10);
			if (wcid >= 128) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("wcid=%d is wrong\n\r",wcid));
				return FALSE;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("tear down ori ba,wcid=%d,tid=%d\n\r",wcid,tid));
			BAOriSessionTearDown(pAd, wcid, tid, FALSE, TRUE);
			return TRUE;
		}
		return FALSE;
	}

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = simple_strtol((token+1), 0, 10);
		/* tid will be 0 ~ 7; Or kernel will crash in BAOriSessionTearDown() */
		if (tid > (NUM_OF_TID-1))
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

	    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
								PRINT_MAC(mac), tid));

#ifdef CONFIG_AP_SUPPORT
	    pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif

	    if (pEntry) {
	        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nTear down Ori BA Session: Tid = %d\n", tid));
	        BAOriSessionTearDown(pAd, pEntry->wcid, tid, FALSE, TRUE);
	    }

		return TRUE;
	}

	return FALSE;

}

INT	Set_BARecTearDown_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UCHAR mac[6], tid;
	UCHAR wcid;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    /*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/
/*
	The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
		=>The six 2 digit hex-decimal number previous are the Mac address,
		=>The seventh decimal number is the tid value.
*/
    if(strlen(arg) < 19)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
	{
		//another acceptable format wcid-tid
		token = strchr(arg, DASH);
		if ((token != NULL) && (strlen(token)>1))
		{
			tid = simple_strtol((token+1), 0, 10);
			/* tid will be 0 ~ 7; Or kernel will crash in BARecSessionTearDown() */
			if (tid > (NUM_OF_TID-1)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("tid=%d is wrong\n\r",tid));
				return FALSE;
			}
			*token = '\0';
			wcid = simple_strtol(arg, 0, 10);
			if (wcid >= 128) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("wcid=%d is wrong\n\r",wcid));
				return FALSE;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("tear down rec ba,wcid=%d,tid=%d\n\r",wcid,tid));
			BARecSessionTearDown(pAd, wcid, tid, FALSE);
			return TRUE;
		}
		return FALSE;
	}
	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = simple_strtol((token+1), 0, 10);
		/* tid will be 0 ~ 7; Or kernel will crash in BARecSessionTearDown() */
		if (tid > (NUM_OF_TID-1))
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
								PRINT_MAC(mac), tid));

#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
#endif

		if (pEntry) {
		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nTear down Rec BA Session: Tid = %d\n", tid));
		    BARecSessionTearDown(pAd, pEntry->wcid, tid, FALSE);
		}

		return TRUE;
	}

	return FALSE;

}

INT	Set_HtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG HtBw;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	HtBw = simple_strtol(arg, 0, 10);

	if (HtBw == BW_40)
		wlan_config_set_ht_bw(wdev,BW_40);
	else if (HtBw == BW_20)
		wlan_config_set_ht_bw(wdev,BW_20);
	else
		return FALSE;  /*Invalid argument */

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtBw_Proc::(HtBw=%d)\n", wlan_config_get_ht_bw(wdev)));

	return TRUE;
}


INT	Set_HtMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR HtMcs = MCS_AUTO, Mcs_tmp, ValidMcs = 15;
#ifdef DOT11_VHT_AC
	RTMP_STRING *mcs_str, *ss_str;
	UCHAR ss = 0, mcs = 0;
#endif /* DOT11_VHT_AC */
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

#ifdef DOT11_VHT_AC
	ss_str = arg;
	if ((mcs_str = rtstrchr(arg, ':'))!= NULL)
	{
		*mcs_str = 0;
		mcs_str++;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ss_str=%s, mcs_str=%s\n",
						__FUNCTION__, ss_str, mcs_str));

		if (strlen(ss_str) && strlen(mcs_str)) {
			mcs = simple_strtol(mcs_str, 0, 10);
			ss = simple_strtol(ss_str, 0, 10);

			if ((ss <= wlan_config_get_tx_stream(wdev)) && (mcs <= 7))
				HtMcs = ((ss - 1) <<4) | mcs;
			else {
				HtMcs = MCS_AUTO;
				ss = 0;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): %dSS-MCS%d, Auto=%s\n",
						__FUNCTION__, ss, mcs,
						(HtMcs == MCS_AUTO && ss == 0) ? "TRUE" : "FALSE"));
			Set_FixedTxMode_Proc(pAd, "VHT");
		}
	}
	else
#endif /* DOT11_VHT_AC */
	{
		Mcs_tmp = simple_strtol(arg, 0, 10);
		if (Mcs_tmp <= ValidMcs || Mcs_tmp == 32)
			HtMcs = Mcs_tmp;
		else
			HtMcs = MCS_AUTO;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

		wdev->DesiredTransmitSetting.field.MCS = HtMcs;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMcs_Proc::(HtMcs=%d) for ra%d\n",
				wdev->DesiredTransmitSetting.field.MCS, pObj->ioctl_if));
	}
#endif /* CONFIG_AP_SUPPORT */

	SetCommonHtVht(pAd,wdev);

#ifdef WFA_VHT_PF
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		INT idx;

		NdisAcquireSpinLock(&pAd->MacTabLock);
        //TODO:Carter, check why start from 1
		for (idx = 1; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++)
		{
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[idx];

			if (IS_ENTRY_CLIENT(pEntry) && (pEntry->func_tb_idx == pObj->ioctl_if)) {
				if ((HtMcs == MCS_AUTO) &&  ss == 0) {
					UCHAR TableSize = 0;

					pEntry->bAutoTxRateSwitch = TRUE;

					MlmeSelectTxRateTable(pAd, pEntry, &pEntry->pTable, &TableSize, &pEntry->CurrTxRateIndex);
					MlmeNewTxRate(pAd, pEntry);

#ifdef NEW_RATE_ADAPT_SUPPORT
					if (! ADAPT_RATE_TABLE(pEntry->pTable))
#endif /* NEW_RATE_ADAPT_SUPPORT */
						pEntry->HTPhyMode.field.ShortGI = GI_800;
				}
				else
				{
					pEntry->HTPhyMode.field.MCS = pMbss->HTPhyMode.field.MCS;
					pEntry->bAutoTxRateSwitch = FALSE;

					/* If the legacy mode is set, overwrite the transmit setting of this entry. */
					RTMPUpdateLegacyTxSetting((UCHAR)pMbss->DesiredTransmitSetting.field.FixedTxMode, pEntry);
				}
			}
		}
		NdisReleaseSpinLock(&pAd->MacTabLock);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* WFA_VHT_PF */

	return TRUE;
}

INT	Set_HtGi_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG HtGi;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	HtGi = simple_strtol(arg, 0, 10);

	if ( HtGi == GI_400)
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_400;
	else if ( HtGi == GI_800 )
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
	else
		return FALSE; /* Invalid argument */

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtGi_Proc::(ShortGI=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.ShortGI));

	return TRUE;
}


INT	Set_HtTxBASize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Size;

	Size = simple_strtol(arg, 0, 10);

	if (Size <=0 || Size >=64)
	{
		Size = 8;
	}
	pAd->CommonCfg.TxBASize = Size-1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_HtTxBASize ::(TxBASize= %d)\n", Size));

	return TRUE;
}

INT	Set_HtDisallowTKIP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value == 1)
	{
		pAd->CommonCfg.HT_DisallowTKIP = TRUE;
	}
	else
	{
		pAd->CommonCfg.HT_DisallowTKIP = FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtDisallowTKIP_Proc ::%s\n",
				(pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "enabled" : "disabled"));

	return TRUE;
}

INT	Set_HtOpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);

	if (Value == HTMODE_GF)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
	else if ( Value == HTMODE_MM )
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtOpMode_Proc::(HtOpMode=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.HTMODE));

	return TRUE;

}

INT	Set_HtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);

	if (Value == STBC_USE)
		wlan_config_set_ht_stbc(wdev, STBC_USE);
	else if ( Value == STBC_NONE )
		wlan_config_set_ht_stbc(wdev, STBC_NONE);
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_Stbc_Proc::(HtStbc=%d)\n",wlan_config_get_ht_stbc(wdev)));

	return TRUE;
}


/*configure useage*/
INT	set_extcha_for_wdev(RTMP_ADAPTER *pAd,struct wifi_dev *wdev,UCHAR value)
{
	value = value ? EXTCHA_ABOVE : EXTCHA_BELOW;
	wlan_config_set_ext_cha(wdev,value);
	SetCommonHtVht(pAd,wdev);
	return TRUE;
}


INT	Set_HtExtcha_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	ULONG Value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	UCHAR ext_cha;

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);

	if(Value !=0 && Value !=1)
		return FALSE;

	set_extcha_for_wdev(pAd,wdev,Value);
	ext_cha = wlan_config_get_ext_cha(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtExtcha_Proc::(HtExtcha=%d)\n",ext_cha));

	return TRUE;
}

INT	Set_HtMpduDensity_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value <=7)
		pAd->CommonCfg.BACapability.field.MpduDensity = Value;
	else
		pAd->CommonCfg.BACapability.field.MpduDensity = 4;

	SetCommonHtVht(pAd,NULL);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMpduDensity_Proc::(HtMpduDensity=%d)\n",pAd->CommonCfg.BACapability.field.MpduDensity));

	return TRUE;
}

INT	Set_HtBaWinSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);

#ifdef CONFIG_AP_SUPPORT
	/* Intel IOT*/
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		Value = 64;
#endif /* CONFIG_AP_SUPPORT */

	if (Value >=1 && Value <= 64)
	{
		pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = Value;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = Value;
#ifdef MT_MAC_BTCOEX
		pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = Value;
		pAd->CommonCfg.BACapability.field.TxBAWinLimit = Value;
#endif /*MT_MAC_BTCOEX*/
	}
	else
	{
        pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = 64;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64;
#ifdef MT_MAC_BTCOEX
		pAd->CommonCfg.REGBACapability.field.TxBAWinLimit = 64;
		pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64;
#endif /*MT_MAC_BTCOEX*/

	}

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtBaWinSize_Proc::(HtBaWinSize=%d)\n",pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	return TRUE;
}


INT	Set_HtRdg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.bRdg = FALSE;
	else if (Value == 1)
        pAd->CommonCfg.bRdg = TRUE;
	else
		return FALSE; /*Invalid argument*/

#ifdef MT_MAC
    if (pAd->chipCap.hif_type == HIF_MT) {
        if (IS_MT7637(pAd)) {
            /* workable for RDG */
        }
        else {
            pAd->CommonCfg.bRdg = FALSE;
        }
    }
#endif

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("Set_HtRdg_Proc::(HtRdg=%d)\n",pAd->CommonCfg.bRdg));

	return TRUE;
}

INT	Set_HtLinkAdapt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->bLinkAdapt = FALSE;
	else if ( Value ==1 )
		pAd->bLinkAdapt = TRUE;
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtLinkAdapt_Proc::(HtLinkAdapt=%d)\n",pAd->bLinkAdapt));

	return TRUE;
}


INT	Set_HtAmsdu_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);
	pAd->CommonCfg.BACapability.field.AmsduEnable = (Value == 0) ? FALSE : TRUE;
	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtAmsdu_Proc::(HtAmsdu=%d)\n",pAd->CommonCfg.BACapability.field.AmsduEnable));

	return TRUE;
}


INT	Set_HtAutoBa_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
	{
		pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
		pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
	}
    else if (Value == 1)
    {
		pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
		pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
    }
	else
		return FALSE; /*Invalid argument*/

    pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
	pAd->CommonCfg.REGBACapability.field.Policy = pAd->CommonCfg.BACapability.field.Policy;
	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtAutoBa_Proc::(HtAutoBa=%d)\n",pAd->CommonCfg.BACapability.field.AutoBA));

	return TRUE;

}


INT	Set_HtProtect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.bHTProtect = FALSE;
    else if (Value == 1)
		pAd->CommonCfg.bHTProtect = TRUE;
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtProtect_Proc::(HtProtect=%d)\n",pAd->CommonCfg.bHTProtect));

	return TRUE;
}

INT	Set_SendSMPSAction_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UCHAR mac[6], mode;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    /*MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));*/
/*
	The BARecTearDown inupt string format should be xx:xx:xx:xx:xx:xx-d,
		=>The six 2 digit hex-decimal number previous are the Mac address,
		=>The seventh decimal number is the mode value.
*/
    if(strlen(arg) < 19)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and mode value in decimal format.*/
		return FALSE;

   	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		mode = simple_strtol((token+1), 0, 10);
		if (mode > MMPS_DISABLE)
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
					PRINT_MAC(mac), mode));
#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, mac);
#endif

		if (pEntry) {
		    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nSendSMPSAction SMPS mode = %d\n", mode));
		    SendSMPSAction(pAd, pEntry->wcid, mode);
		}

		return TRUE;
	}

	return FALSE;


}

INT	Set_HtMIMOPSmode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);

	if (Value <=3)
		pAd->CommonCfg.BACapability.field.MMPSmode = Value;
	else
		pAd->CommonCfg.BACapability.field.MMPSmode = 3;

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMIMOPSmode_Proc::(MIMOPS mode=%d)\n",pAd->CommonCfg.BACapability.field.MMPSmode));

	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
/*
    ==========================================================================
    Description:
        Set Tx Stream number
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtTxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG 	Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR RfIC = 0;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	UINT8 max_tx_path;
	UCHAR band_idx;

	if(!wdev)
		return FALSE;

	if (pAd->CommonCfg.dbdc_mode)
	{
		band_idx = HcGetBandByWdev(wdev);
		if (band_idx == DBDC_BAND0)
			max_tx_path = pAd->dbdc_2G_tx_stream;
		else
			max_tx_path = pAd->dbdc_5G_tx_stream;
	} else {
		max_tx_path = pAd->Antenna.field.TxPath;
	}

	Value = simple_strtol(arg, 0, 10);

	if ((Value <= 4) && (Value >= 1) && (Value <= max_tx_path))
        wlan_config_set_tx_stream(wdev, Value);
	else
        wlan_config_set_tx_stream(wdev, max_tx_path);

	SetCommonHtVht(pAd,wdev);

	RfIC = wmode_2_rfic(wdev->PhyMode);
	APStopByRf(pAd, RfIC);
	APStartUpByRf(pAd, RfIC);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtTxStream_Proc::(Tx Stream=%d)\n",wlan_config_get_tx_stream(wdev)));

	return TRUE;
}

/*
    ==========================================================================
    Description:
        Set Rx Stream number
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HtRxStream_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG 	Value;
	UCHAR RfIC = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	UINT8 max_rx_path;
	UCHAR band_idx;

	if(!wdev)
		return FALSE;

	if (pAd->CommonCfg.dbdc_mode)
	{
		band_idx = HcGetBandByWdev(wdev);
		if (band_idx == DBDC_BAND0)
			max_rx_path = pAd->dbdc_2G_rx_stream;
		else
			max_rx_path = pAd->dbdc_5G_rx_stream;
	} else {
		max_rx_path = pAd->Antenna.field.RxPath;
	}

	Value = simple_strtol(arg, 0, 10);

	if ((Value <= 4) && (Value >= 1) && (Value <= max_rx_path))
		wlan_config_set_rx_stream(wdev, Value);
	else
		wlan_config_set_rx_stream(wdev, max_rx_path);

	SetCommonHtVht(pAd,wdev);

	RfIC = wmode_2_rfic(wdev->PhyMode);
	APStopByRf(pAd, RfIC);
	APStartUpByRf(pAd, RfIC);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtRxStream_Proc::(Rx Stream=%d)\n", wlan_config_get_rx_stream(wdev)));

	return TRUE;
}

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
INT	Set_GreenAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
    struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0) {
	    greenap_proc(pAd, greenap, FALSE);
	}
	else if (Value == 1) {
	    greenap_proc(pAd, greenap, TRUE);
	}
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_GreenAP_Proc::(greenap_cap=%d)\n", greenap_get_capability(greenap)));

	return TRUE;
}
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

INT	Set_ForceShortGI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->WIFItestbed.bShortGI = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bShortGI = TRUE;
	else
		return FALSE; /*Invalid argument*/

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ForceShortGI_Proc::(ForceShortGI=%d)\n", pAd->WIFItestbed.bShortGI));

	return TRUE;
}



INT	Set_ForceGF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->WIFItestbed.bGreenField = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bGreenField = TRUE;
	else
		return FALSE; /*Invalid argument*/

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ForceGF_Proc::(ForceGF=%d)\n", pAd->WIFItestbed.bGreenField));

	return TRUE;
}

INT	Set_HtMimoPs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.bMIMOPSEnable = FALSE;
	else if (Value == 1)
		pAd->CommonCfg.bMIMOPSEnable = TRUE;
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_HtMimoPs_Proc::(HtMimoPs=%d)\n",pAd->CommonCfg.bMIMOPSEnable));

	return TRUE;
}


#ifdef DOT11N_DRAFT3
INT Set_HT_BssCoex_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *pParam)
{
	UCHAR bBssCoexEnable = simple_strtol(pParam, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	
	if(!wdev)
		return FALSE;

	pAd->CommonCfg.bBssCoexEnable = ((bBssCoexEnable == 1) ? TRUE: FALSE);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set bBssCoexEnable=%d!\n", pAd->CommonCfg.bBssCoexEnable));

	if ((pAd->CommonCfg.bBssCoexEnable == FALSE)
		&& pAd->CommonCfg.bRcvBSSWidthTriggerEvents)
	{
		/* switch back 20/40 */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set bBssCoexEnable:  Switch back 20/40. \n"));
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;
		if ((HcIsRfSupport(pAd,RFIC_24GHZ)) && (wlan_config_get_ht_bw(wdev) == BW_40))
		{
			wlan_operate_set_ht_bw(wdev,HT_BW_40);
		}

#if (defined(WH_EZ_SETUP) && defined(EZ_NETWORK_MERGE_SUPPORT))
		if (IS_EZ_SETUP_ENABLED(wdev)){
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("\nSet_HT_BssCoex_Proc: Coex support disabled ****\n"));
			ez_set_ap_fallback_context(wdev,FALSE,0);
		}
#endif /* WH_EZ_SETUP */

	}

	return TRUE;
}


INT Set_HT_BssCoexApCntThr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *pParam)
{
	pAd->CommonCfg.BssCoexApCntThr = simple_strtol(pParam, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set BssCoexApCntThr=%d!\n", pAd->CommonCfg.BssCoexApCntThr));

	return TRUE;
}
#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
INT	Set_VhtBw_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG vht_cw;
	UCHAR cent_ch;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	vht_cw = simple_strtol(arg, 0, 10);

	if(wdev->channel <=14)
		goto direct_done;

	if (vht_cw <= VHT_BW_8080){
		pAd->CommonCfg.vht_bw = vht_cw;
	}
	else{
		pAd->CommonCfg.vht_bw = VHT_BW_2040;
	}
	pAd->CommonCfg.cfg_vht_bw = pAd->CommonCfg.vht_bw;
	wlan_config_set_vht_bw(wdev,pAd->CommonCfg.vht_bw);

	if ( !WMODE_CAP_AC(wdev->PhyMode))
		goto direct_done;

	SetCommonHtVht(pAd,wdev);
	if(pAd->CommonCfg.BBPCurrentBW == BW_80)
		cent_ch = pAd->CommonCfg.vht_cent_ch;
	else
		cent_ch = pAd->CommonCfg.CentralChannel;

	AsicSwitchChannel(pAd, cent_ch, FALSE);
	AsicLockChannel(pAd, cent_ch);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BW_%s, PrimaryChannel(%d), %s CentralChannel = %d, apply it immediately\n",
						(pAd->CommonCfg.BBPCurrentBW == BW_80 ? "80":
							(pAd->CommonCfg.BBPCurrentBW == BW_40 ? "40" :
							"20")),
						wdev->channel,
						(pAd->CommonCfg.BBPCurrentBW == BW_80 ? "VHT" : "HT"), cent_ch));

direct_done:

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_VhtBw_Proc::(VHT_BW=%d)\n", pAd->CommonCfg.vht_bw));

	return TRUE;
}


INT set_VhtBwSignal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG bw_signal = simple_strtol(arg, 0, 10);

	if (bw_signal <= 2)
		pAd->CommonCfg.vht_bw_signal = bw_signal;
	else
		pAd->CommonCfg.vht_bw_signal = BW_SIGNALING_DISABLE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): vht_bw_signal=%d(%s)\n",
				__FUNCTION__, pAd->CommonCfg.vht_bw_signal,
				(pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_DYNAMIC ? "Dynamic" :
				(pAd->CommonCfg.vht_bw_signal == BW_SIGNALING_STATIC ? "Static" : "Disable"))));

	return TRUE;
}


INT	Set_VhtStbc_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if(!wdev)
		return FALSE;

	Value = simple_strtol(arg, 0, 10);

	if (Value == STBC_USE)
		wlan_config_set_vht_stbc(wdev, STBC_USE);
	else if ( Value == STBC_NONE )
		wlan_config_set_vht_stbc(wdev, STBC_NONE);
	else
		return FALSE; /*Invalid argument */

	SetCommonHtVht(pAd,wdev);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_VhtStbc_Proc::(VhtStbc=%d)\n", wlan_config_get_vht_stbc(wdev)));

	return TRUE;
}

INT	Set_VhtDisallowNonVHT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.bNonVhtDisallow = FALSE;
	else
		pAd->CommonCfg.bNonVhtDisallow = TRUE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_VhtDisallowNonVHT_Proc::(bNonVhtDisallow=%d)\n", pAd->CommonCfg.bNonVhtDisallow));

	return TRUE;
}
#endif /* DOT11_VHT_AC */



INT	Set_FixedTxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT	fix_tx_mode = RT_CfgSetFixedTxPhyMode(arg);


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */

	if (wdev)
		wdev->DesiredTransmitSetting.field.FixedTxMode = fix_tx_mode;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():(FixedTxMode=%d)\n",
								__FUNCTION__, fix_tx_mode));

	return TRUE;
}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

#ifdef RTMP_MAC_PCI
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
#endif /* RTMP_MAC_PCI */
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can not switch operate mode on interface up !! \n"));
		return FALSE;
	}

	if (Value == 0)
		pAd->OpMode = OPMODE_STA;
	else if (Value == 1)
		pAd->OpMode = OPMODE_AP;
	else
		return FALSE; /*Invalid argument*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_OpMode_Proc::(OpMode=%s)\n", pAd->OpMode == 1 ? "AP Mode" : "STA Mode"));

	return TRUE;
}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */


#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
/* ---------------------- DEBUG QUEUE ------------------------*/

#define DBQ_LENGTH	512
#define DBQ_DATA_LENGTH	8

/* Define to include TX and RX HT Control field in log */
/* #define DBQ_INCLUDE_HTC */

typedef
struct {
	UCHAR type;					/* type of data*/
	ULONG timestamp;			/* sec/usec timestamp from gettimeofday*/
	UCHAR data[DBQ_DATA_LENGTH];	/* data*/
} DBQUEUE_ENTRY;

/* Type field definitions */
#define DBQ_TYPE_EMPTY	0
#define DBQ_TYPE_TXWI	0x70		/* TXWI*/
#define DBQ_TYPE_TXHDR	0x72		/* TX Header*/
#define DBQ_TYPE_TXFIFO	0x73		/* TX Stat FIFO*/
#define DBQ_TYPE_RXWI	0x78		/* RXWI uses 0x78 to 0x7A for 5 longs*/
#define DBQ_TYPE_RXHDR	0x7B		/* RX Header*/
#define DBQ_TYPE_TXQHTC	0x7c		/* RX Qos+HT Control field*/
#define DBQ_TYPE_RXQHTC	0x7d		/* RX Qos+HT Control field */
#define DBQ_TYPE_RALOG	0x7e		/* RA Log */

#define DBQ_INIT_SIG	0x4442484E	/* 'DBIN' - dbqInit initialized flag*/
#define DBQ_ENA_SIG		0x4442454E	/* 'DBEN' - dbqEnable enabled flag*/

static DBQUEUE_ENTRY dbQueue[DBQ_LENGTH];
static ULONG dbqTail=0;
static ULONG dbqEnable=0;
static ULONG dbqInit=0;

/* dbQueueInit - initialize Debug Queue variables and clear the queue*/
void dbQueueInit(void)
{
	int i;

	for (i=0; i<DBQ_LENGTH; i++)
		dbQueue[i].type = DBQ_TYPE_EMPTY;
	dbqTail = 0;
	dbqInit = DBQ_INIT_SIG;
}

/* dbQueueEnqueue - enqueue data*/
void dbQueueEnqueue(UCHAR type, UCHAR *data)
{
	DBQUEUE_ENTRY *oldTail;
	struct timeval tval;

	if (dbqEnable!=DBQ_ENA_SIG || data==NULL)
		return;

	if (dbqInit!=DBQ_INIT_SIG || dbqTail>=DBQ_LENGTH)
		dbQueueInit();

	oldTail = &dbQueue[dbqTail];

	/* Advance tail and mark as empty*/
	if (dbqTail >= DBQ_LENGTH-1)
		dbqTail = 0;
	else
		dbqTail++;
	dbQueue[dbqTail].type = DBQ_TYPE_EMPTY;

	/* Enqueue data*/
	oldTail->type = type;
	do_gettimeofday(&tval);
	oldTail->timestamp = tval.tv_sec*1000000L + tval.tv_usec;
	memcpy(oldTail->data, data, DBQ_DATA_LENGTH);
}

void dbQueueEnqueueTxFrame(UCHAR *pTxWI, UCHAR *pHeader_802_11)
{
	dbQueueEnqueue(DBQ_TYPE_TXWI, pTxWI);

	/* 802.11 Header */
	if (pHeader_802_11 != NULL) {
		dbQueueEnqueue(DBQ_TYPE_TXHDR, pHeader_802_11);
#ifdef DBQ_INCLUDE_HTC
		/* Qos+HT Control field */
		if ((pHeader_802_11[0] & 0x08) && (pHeader_802_11[1] & 0x80))
			dbQueueEnqueue(DBQ_TYPE_TXQHTC, pHeader_802_11+24);
#endif /* DBQ_INCLUDE_HTC */
	}
}

void dbQueueEnqueueRxFrame(UCHAR *pRxWI, UCHAR *pHeader_802_11, ULONG flags)
{
	/* Ignore Beacons if disabled */
	if ((flags & DBF_DBQ_NO_BCN) && (pHeader_802_11[0] & 0xfc)==0x80)
		return;

	/* RXWI */
	dbQueueEnqueue(DBQ_TYPE_RXWI, pRxWI);
	if (flags & DBF_DBQ_RXWI_FULL) {
		dbQueueEnqueue(DBQ_TYPE_RXWI+1, pRxWI+8);
		dbQueueEnqueue(DBQ_TYPE_RXWI+2, pRxWI+16);
	}

	/* 802.11 Header */
	dbQueueEnqueue(DBQ_TYPE_RXHDR, (UCHAR *)pHeader_802_11);

#ifdef DBQ_INCLUDE_HTC
	/* Qos+HT Control field */
	if ((pHeader_802_11[0] & 0x08) &&
		(pHeader_802_11[1] & 0x80))
		dbQueueEnqueue(DBQ_TYPE_RXQHTC, pHeader_802_11+24);
#endif /* DBQ_INCLUDE_HTC */
}


/* dbQueueDisplayPhy - Display PHY rate */
static void dbQueueDisplayPHY(USHORT phyRate)
{
	static CHAR *mode[4] = {" C", "oM","mM", "gM"};

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%2s%02d %c%c%c%c",
		//(phyRate>>8) & 0xFF, phyRate & 0xFF,
		mode[(phyRate>>14) & 0x3],							// Mode: c, o, m, g
		phyRate & 0x7F,										// MCS
		(phyRate & 0x0100)? 'S': 'L',						// Guard Int: S or L
		(phyRate & 0x0080)? '4': '2',						// BW: 4 or 2
		(phyRate & 0x0200)? 'S': 's',						// STBC:  S or s
		(phyRate & 0x2000)? 'I': ((phyRate & 0x800)? 'E': '_')	// Beamforming:  E or I or _
		) );
}

/* dbQueueDump - dump contents of debug queue*/
static void dbQueueDump(
	IN  PRTMP_ADAPTER   pAd,
	BOOLEAN decode)
{
	DBQUEUE_ENTRY *oldTail;
	int i, origMCS, succMCS;
	ULONG lastTimestamp=0;
	BOOLEAN showTimestamp;
	USHORT phyRate;

	if (dbqInit!=DBQ_INIT_SIG || dbqTail>=DBQ_LENGTH)
		return;

	oldTail = &dbQueue[dbqTail];

	for (i=0; i<DBQ_LENGTH; i++) {
		if (++oldTail >= &dbQueue[DBQ_LENGTH])
			oldTail = dbQueue;

		/* Skip empty entries*/
		if (oldTail->type == DBQ_TYPE_EMPTY)
			continue;

		showTimestamp = FALSE;

		switch (oldTail->type) {
		case 0x70:	/* TXWI - 2 longs, MSB to LSB */
		case 0x78:	/* RXWI - 2 longs, MSB to LSB */
			showTimestamp = TRUE;

			if (decode && oldTail->type==0x70) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nTxWI ") );
				dbQueueDisplayPHY(oldTail->data[3]*256 + oldTail->data[2]);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c s=%03X %02X %s-",
						(oldTail->data[0] & 0x10)? 'A': '_',				// AMPDU
						(oldTail->data[7]*256 + oldTail->data[6]) & 0xFFF,	// Size
						oldTail->data[5],									// WCID
						(oldTail->data[4] & 0x01)? "AK": "NA" ));			// ACK/NACK
			}
			else if (decode && oldTail->type==0x78) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRxWI ") );
				dbQueueDisplayPHY(oldTail->data[7]*256 + oldTail->data[6]);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" s=%03X %02X %02X%01X-",
						(oldTail->data[3]*256 + oldTail->data[2]) & 0xFFF,	// Size
						oldTail->data[0],									// WCID
						oldTail->data[5], oldTail->data[4]>>4 ));			// Seq Number
			}
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%cxWI %02X%02X %02X%02X-%02X%02X %02X%02X----",
					oldTail->type==0x70? 'T': 'R',
					oldTail->data[3], oldTail->data[2], oldTail->data[1], oldTail->data[0],
					oldTail->data[7], oldTail->data[6], oldTail->data[5], oldTail->data[4]) );
			break;

		case 0x79:	/* RXWI - next 2 longs, MSB to LSB */
			if (decode) {
				struct raw_rssi_info rssi_info;

				rssi_info.raw_rssi[0] = (CHAR)oldTail->data[0];
				rssi_info.raw_rssi[1] = (CHAR)oldTail->data[1];
				rssi_info.raw_rssi[2] = (CHAR)oldTail->data[2];
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx2  %2d %2d %2d S:%d %d %d ",
						ConvertToRssi(pAd, &rssi_info, RSSI_IDX_0),
						ConvertToRssi(pAd, &rssi_info, RSSI_IDX_1),
						ConvertToRssi(pAd, &rssi_info, RSSI_IDX_2),
						(oldTail->data[4]*3 + 8)/16,
						(oldTail->data[5]*3 + 8)/16,
						(oldTail->data[6]*3 + 8)/16) );
			}
			else
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx2  %02X%02X %02X%02X-%02X%02X %02X%02X    ",
						oldTail->data[3], oldTail->data[2], oldTail->data[1], oldTail->data[0],
						oldTail->data[7], oldTail->data[6], oldTail->data[5], oldTail->data[4]) );
			break;


		case 0x7c:	/* TX HTC+QoS, 6 bytes, MSB to LSB */
		case 0x7d:	/* RX HTC+QoS, 6 bytes, MSB to LSB */
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%cxHTC  H:%02X%02X%02X%02X Q:%02X%02X   ",
					oldTail->type==0x7c? 'T': 'R',
					oldTail->data[5], oldTail->data[4], oldTail->data[3], oldTail->data[2],
					oldTail->data[1], oldTail->data[0]) );
			break;

		case 0x72:	/* Tx 802.11 header, MSB to LSB, translate type/subtype*/
		case 0x7b:	/* Rx*/
			{
			UCHAR tCode;
			struct _typeTableEntry {
				UCHAR code;	/* Type/subtype*/
				CHAR  str[4];
			} *pTab;
			static struct _typeTableEntry typeTable[] = {
				{0x00, "mARq"}, {0x01, "mARp"}, {0x02, "mRRq"}, {0x03, "mRRp"},
				{0x04, "mPRq"}, {0x05, "mPRp"}, {0x08, "mBcn"}, {0x09, "mATI"},
				{0x0a, "mDis"}, {0x0b, "mAut"}, {0x0c, "mDAu"}, {0x0d, "mAct"},
				{0x0e, "mANA"},
				{0x17, "cCWr"}, {0x18, "cBAR"}, {0x19, "cBAc"}, {0x1a, "cPSP"},
				{0x1b, "cRTS"}, {0x1c, "cCTS"}, {0x1d, "cACK"}, {0x1e, "cCFE"},
				{0x1f, "cCEA"},
				{0x20, "dDat"}, {0x21, "dDCA"}, {0x22, "dDCP"}, {0x23, "dDAP"},
				{0x24, "dNul"}, {0x25, "dCFA"}, {0x26, "dCFP"}, {0x27, "dCAP"},
				{0x28, "dQDa"}, {0x29, "dQCA"}, {0x2a, "dQCP"}, {0x2b, "dQAP"},
				{0x2c, "dQNu"}, {0x2e, "dQNP"}, {0x2f, "dQNA"},
				{0xFF, "RESV"}};

			tCode = ((oldTail->data[0]<<2) & 0x30) | ((oldTail->data[0]>>4) & 0xF);
			for (pTab=typeTable; pTab->code!=0xFF; pTab++) {
				if (pTab->code == tCode)
					break;
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%cxH  %c%c%c%c [%02X%02X %02X%02X]       \n",
					oldTail->type==0x72? 'T': 'R',
					pTab->str[0], pTab->str[1], pTab->str[2], pTab->str[3],
					oldTail->data[3], oldTail->data[2], oldTail->data[1], oldTail->data[0]) );
			}
			break;

		case 0x73:	/* TX STAT FIFO*/
			showTimestamp = TRUE;

			/* origMCS is limited to 4 bits. Check for case of MCS16 to 23*/
			origMCS = (oldTail->data[0]>>1) & 0xF;
			succMCS = (oldTail->data[2] & 0x7F);
			if (succMCS>origMCS && origMCS<8)
				origMCS += 16;
			phyRate = (oldTail->data[3]<<8) + oldTail->data[2];

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxFI %02X%02X%02X%02X=%c%c%c%c%c M%02d/%02d%c%c",
					oldTail->data[3], oldTail->data[2],
					oldTail->data[1], oldTail->data[0],
					(phyRate & 0x0100)? 'S': 'L',				/* Guard Int:    S or L */
					(phyRate & 0x0080)? '4': '2',				/* BW: 			 4 or 2 */
					(phyRate & 0x0200)? 'S': 's',				/* STBC:         S or s */
					(phyRate & 0x2000)? 'I': ((phyRate & 0x0800)? 'E': '_'), /* Beamforming:  E or I or _ */
					(oldTail->data[0] & 0x40)? 'A': '_',		/* Aggregated:   A or _ */
					succMCS, origMCS,							/* MCS:          <Final>/<orig> */
					succMCS==origMCS? ' ': '*',					/* Retry:        '*' if MCS doesn't match */
					(oldTail->data[0] & 0x20)? ' ': 'F') );		/* Success/Fail  _ or F */
			break;
		case 0x7E:	/* RA Log info */
			{
				struct {USHORT phy; USHORT per; USHORT tp; USHORT bfPer;} *p = (void*)(oldTail->data);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RALog %02X%02X %d %d %d    ",
											(p->phy>>8) & 0xFF, p->phy & 0xFF, p->per, p->tp, p->bfPer) );
			}
			break;

		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X   %02X%02X %02X%02X %02X%02X %02X%02X   ", oldTail->type,
					oldTail->data[0], oldTail->data[1], oldTail->data[2], oldTail->data[3],
					oldTail->data[4], oldTail->data[5], oldTail->data[6], oldTail->data[7]) );
			break;
		}

		if (showTimestamp)
		{
			ULONG t = oldTail->timestamp;
			ULONG dt = oldTail->timestamp-lastTimestamp;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%lu.%06lu ", t/1000000L, t % 1000000L) );

			if (dt>999999L)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("+%lu.%06lu s\n", dt/1000000L, dt % 1000000L) );
			else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("+%lu us\n", dt) );
			lastTimestamp = oldTail->timestamp;
		}
	}
}

/*
	Set_DebugQueue_Proc - Control DBQueue
		iwpriv ra0 set DBQueue=dd.
			dd: 0=>disable, 1=>enable, 2=>dump, 3=>clear, 4=>dump & decode
*/
INT Set_DebugQueue_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    ULONG argValue = simple_strtol(arg, 0, 10);

	switch (argValue) {
	case 0:
		dbqEnable = 0;
		break;
	case 1:
		dbqEnable = DBQ_ENA_SIG;
		break;
	case 2:
		dbQueueDump(pAd, FALSE);
		break;
	case 3:
		dbQueueInit();
		break;
	case 4:
		dbQueueDump(pAd, TRUE);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
/*
	========================================================================
	Routine Description:
		Set the enable/disable the stream mode

	Arguments:
		1:	enable for 1SS
		2:	enable for 2SS
		3:	enable for 1SS and 2SS
		0:	disable

	Notes:
		Currently only support 1SS
	========================================================================
*/
INT Set_StreamMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 streamWord, reg, regAddr;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	if (pAd->chipCap.FlgHwStreamMode == FALSE)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("chip not supported feature\n"));
		return FALSE;
	}

	pAd->CommonCfg.StreamMode = (simple_strtol(arg, 0, 10) & 0x3);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():StreamMode=%d\n", __FUNCTION__, pAd->CommonCfg.StreamMode));

	streamWord = StreamModeRegVal(pAd);
	for (regAddr = TX_CHAIN_ADDR0_H; regAddr <= TX_CHAIN_ADDR3_H; regAddr += 8)
	{
		RTMP_IO_READ32(pAd, regAddr, &reg);
		reg &= (~0x000F0000);
		RTMP_IO_WRITE32(pAd, regAddr, streamWord | reg);
	}

	return TRUE;
}


INT Set_StreamModeMac_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return FALSE;
}


INT Set_StreamModeMCS_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->CommonCfg.StreamModeMCS = simple_strtol(arg, 0, 16);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():StreamModeMCS=%02X\n",
				__FUNCTION__, pAd->CommonCfg.StreamModeMCS));

	return TRUE;
}
#endif /* STREAM_MODE_SUPPORT */


#ifdef PRE_ANT_SWITCH
/*
	Set_PreAntSwitch_Proc - enable/disable Preamble Antenna Switch
		usage: iwpriv ra0 set PreAntSwitch=[0 | 1]
*/
INT Set_PreAntSwitch_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    pAd->CommonCfg.PreAntSwitch = simple_strtol(arg, 0, 10)!=0;
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():(PreAntSwitch=%d)\n",
				__FUNCTION__, pAd->CommonCfg.PreAntSwitch));
	return TRUE;
}


/*
	Set_PreAntSwitchRSSI_Proc - set Preamble Antenna Switch RSSI threshold
		usage: iwpriv ra0 set PreAntSwitchRSSI=<RSSI threshold in dBm>
*/
INT Set_PreAntSwitchRSSI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    pAd->CommonCfg.PreAntSwitchRSSI = simple_strtol(arg, 0, 10);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():(PreAntSwitchRSSI=%d)\n",
				__FUNCTION__, pAd->CommonCfg.PreAntSwitchRSSI));
	return TRUE;
}

/*
	Set_PreAntSwitchTimeout_Proc - set Preamble Antenna Switch Timeout threshold
		usage: iwpriv ra0 set PreAntSwitchTimeout=<timeout in seconds, 0=>disabled>
*/
INT Set_PreAntSwitchTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    pAd->CommonCfg.PreAntSwitchTimeout = simple_strtol(arg, 0, 10);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():(PreAntSwitchTimeout=%d)\n",
				__FUNCTION__, pAd->CommonCfg.PreAntSwitchTimeout));
	return TRUE;
}
#endif /* PRE_ANT_SWITCH */




#ifdef CFO_TRACK
/*
	Set_CFOTrack_Proc - enable/disable CFOTrack
		usage: iwpriv ra0 set CFOTrack=[0..8]
*/
INT Set_CFOTrack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    pAd->CommonCfg.CFOTrack = simple_strtol(arg, 0, 10);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():(CFOTrack=%d)\n",
				__FUNCTION__, pAd->CommonCfg.CFOTrack));
	return TRUE;
}
#endif /* CFO_TRACK */


#ifdef DBG_CTRL_SUPPORT
INT Set_DebugFlags_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    pAd->CommonCfg.DebugFlags = simple_strtol(arg, 0, 16);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_DebugFlags_Proc::(DebugFlags=%02lX)\n", pAd->CommonCfg.DebugFlags));
	return TRUE;
}
#endif /* DBG_CTRL_SUPPORT */






INT Set_LongRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR LongRetryLimit = (UCHAR)simple_strtol(arg, 0, 10);

	AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG, LongRetryLimit);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF Set_LongRetryLimit_Proc::(LongRetryLimit=0x%x)\n", LongRetryLimit));
	return TRUE;
}


INT Set_ShortRetryLimit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR ShortRetryLimit = (UCHAR)simple_strtol(arg, 0, 10);

	AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT, ShortRetryLimit);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF Set_ShortRetryLimit_Proc::(ShortRetryLimit=0x%x)\n", ShortRetryLimit));
	return TRUE;
}


INT Set_AutoFallBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return RT_CfgSetAutoFallBack(pAd, arg);
}



#ifdef MEM_ALLOC_INFO_SUPPORT
INT Show_MemInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ShowMemAllocInfo();
	return TRUE;
}

INT Show_PktInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ShowPktAllocInfo();
	return TRUE;
}
#endif /* MEM_ALLOC_INFO_SUPPORT */


INT	Show_SSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	UCHAR	ssid_str[33];
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;


	NdisZeroMemory(&ssid_str[0], 33);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		NdisMoveMemory(&ssid_str[0],
						pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid,
						pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen);
	}
#endif /* CONFIG_AP_SUPPORT */


	snprintf(pBuf, BufLen, "\t%s", ssid_str);
	return 0;
}

static VOID GetWirelessMode(UCHAR PhyMode,UCHAR *pBuf,UCHAR BufLen)
{
	switch(PhyMode)
	{
		case (UCHAR)(WMODE_B | WMODE_G):
			snprintf(pBuf, BufLen, "\t11B/G");
			break;
		case (UCHAR)(WMODE_B):
			snprintf(pBuf, BufLen, "\t11B");
			break;
		case (UCHAR)(WMODE_A):
			snprintf(pBuf, BufLen, "\t11A");
			break;
		case (UCHAR)(WMODE_A | WMODE_B | WMODE_G):
			snprintf(pBuf, BufLen, "\t11A/B/G");
			break;
		case (UCHAR)(WMODE_G):
			snprintf(pBuf, BufLen, "\t11G");
			break;
#ifdef DOT11_N_SUPPORT
		case (UCHAR)(WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
			snprintf(pBuf, BufLen, "\t11A/B/G/N");
			break;
		case (UCHAR)(WMODE_GN):
			snprintf(pBuf, BufLen, "\t11N only with 2.4G");
			break;
		case (UCHAR)(WMODE_G | WMODE_GN):
			snprintf(pBuf, BufLen, "\t11G/N");
			break;
		case (UCHAR)(WMODE_A | WMODE_AN):
			snprintf(pBuf, BufLen, "\t11A/N");
			break;
		case (UCHAR)(WMODE_B | WMODE_G | WMODE_GN):
			snprintf(pBuf, BufLen, "\t11B/G/N");
			break;
		case (UCHAR)(WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
			snprintf(pBuf, BufLen, "\t11A/G/N");
			break;
		case (UCHAR)(WMODE_AN):
			snprintf(pBuf, BufLen, "\t11N only with 5G");
			break;
#endif /* DOT11_N_SUPPORT */
		default:
			snprintf(pBuf, BufLen, "\tUnknow Value(%d)", PhyMode);
			break;
	}
}

INT	Show_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{

	UCHAR PhyMode;
	BOOLEAN IsRun5G = HcIsRfRun(pAd,RFIC_5GHZ);
	BOOLEAN IsRun2G = HcIsRfRun(pAd,RFIC_24GHZ);

	if(IsRun5G)
	{
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_5GHZ);
		snprintf(pBuf, BufLen,"5G Band: ");
		GetWirelessMode(PhyMode,pBuf,BufLen);
	}

	if(IsRun2G)
	{
		PhyMode = HcGetPhyModeByRf(pAd,RFIC_24GHZ);
		snprintf(pBuf, BufLen,"2.4G Band: ");
		GetWirelessMode(PhyMode,pBuf,BufLen);
	}

	return 0;
}

#ifdef RTMP_UDMA_SUPPORT
INT	Show_UdmaMode_Proc(
	IN      PRTMP_ADAPTER	pAd,
	OUT     RTMP_STRING		*pBuf,
	IN      ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bUdmaFlag ? "TRUE":"FALSE");
	return 0;
}

INT Show_UdmaPortNum_Proc(
	IN      PRTMP_ADAPTER	pAd,
	OUT     RTMP_STRING 	*pBuf,
	IN      ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\tUdmaportNum(%d)", pAd->CommonCfg.UdmaPortNum );
	return 0;
}
#endif	/* RTMP_UDMA_SUPPORT */

INT	Show_TxBurst_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bEnableTxBurst ? "TRUE":"FALSE");
	return 0;
}

INT	Show_TxPreamble_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	switch(pAd->CommonCfg.TxPreamble)
	{
		case Rt802_11PreambleShort:
			snprintf(pBuf, BufLen, "\tShort");
			break;
		case Rt802_11PreambleLong:
			snprintf(pBuf, BufLen, "\tLong");
			break;
		case Rt802_11PreambleAuto:
			snprintf(pBuf, BufLen, "\tAuto");
			break;
		default:
			snprintf(pBuf, BufLen, "\tUnknown Value(%lu)", pAd->CommonCfg.TxPreamble);
			break;
	}

	return 0;
}

INT	Show_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
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
        return 1;

    snprintf(pBuf, BufLen, "\t%lu", pAd->CommonCfg.TxPowerPercentage[BandIdx]);

    return 0;
}


INT	Show_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{

	UCHAR Channel;
	BOOLEAN IsRun5G = HcIsRfRun(pAd,RFIC_5GHZ);
	BOOLEAN IsRun2G = HcIsRfRun(pAd,RFIC_24GHZ);

	if(IsRun5G)
	{
		Channel = HcGetChannelByRf(pAd,RFIC_5GHZ);
		snprintf(pBuf, BufLen, "\t5G Band: %d\n",Channel);
	}

	if(IsRun2G)
	{
		Channel = HcGetChannelByRf(pAd,RFIC_24GHZ);
		snprintf(pBuf, BufLen, "\t2.4G Band: %d\n",Channel);
	}
	return 0;
}

INT	Show_BGProtection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	switch(pAd->CommonCfg.UseBGProtection)
	{
		case 1: /*Always On*/
			snprintf(pBuf, BufLen, "\tON");
			break;
		case 2: /*Always OFF*/
			snprintf(pBuf, BufLen, "\tOFF");
			break;
		case 0: /*AUTO*/
			snprintf(pBuf, BufLen, "\tAuto");
			break;
		default:
			snprintf(pBuf, BufLen, "\tUnknow Value(%lu)", pAd->CommonCfg.UseBGProtection);
			break;
	}
	return 0;
}

INT	Show_RTSThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 oper_len_thld;
	UINT32 conf_len_thld;

	if (!wdev)
		return 0;
	conf_len_thld = wlan_config_get_rts_len_thld(wdev);
	oper_len_thld = wlan_operate_get_rts_len_thld(wdev);
	snprintf(pBuf, BufLen, "\tRTSThreshold:: conf=%d, oper=%d", conf_len_thld, oper_len_thld);

	return 0;
}

INT	Show_FragThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	UINT32 conf_frag_thld;
	UINT32 oper_frag_thld;

	POS_COOKIE pobj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pobj->ioctl_if, pobj->ioctl_if_type);

	if (!wdev)
		return 0;
	conf_frag_thld = wlan_config_get_frag_thld(wdev);
	oper_frag_thld = wlan_operate_get_frag_thld(wdev);
	snprintf(pBuf, BufLen, "\tFrag thld:: conf=%u, oper=%u", conf_frag_thld, oper_frag_thld);
	return 0;
}

#ifdef DOT11_N_SUPPORT
INT	Show_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);

	if (wlan_config_get_ht_bw(wdev) == BW_40)
	{
		snprintf(pBuf, BufLen, "\t40 MHz");
	}
	else
	{
        snprintf(pBuf, BufLen, "\t20 MHz");
	}
	return 0;
}

INT	Show_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */

	if (wdev)
		snprintf(pBuf, BufLen, "\t%u", wdev->DesiredTransmitSetting.field.MCS);
	return 0;
}

INT	Show_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	switch(pAd->CommonCfg.RegTransmitSetting.field.ShortGI)
	{
		case GI_400:
			snprintf(pBuf, BufLen, "\tGI_400");
			break;
		case GI_800:
			snprintf(pBuf, BufLen, "\tGI_800");
			break;
		default:
			snprintf(pBuf, BufLen, "\tUnknow Value(%u)", pAd->CommonCfg.RegTransmitSetting.field.ShortGI);
			break;
	}
	return 0;
}

INT	Show_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	switch(pAd->CommonCfg.RegTransmitSetting.field.HTMODE)
	{
		case HTMODE_GF:
			snprintf(pBuf, BufLen, "\tGF");
			break;
		case HTMODE_MM:
			snprintf(pBuf, BufLen, "\tMM");
			break;
		default:
			snprintf(pBuf, BufLen, "\tUnknow Value(%u)", pAd->CommonCfg.RegTransmitSetting.field.HTMODE);
			break;
	}
	return 0;
}

INT	Show_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UCHAR ext_cha;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		}
#endif /* CONFIG_AP_SUPPORT */

	ext_cha = wlan_config_get_ext_cha(wdev);
	switch(ext_cha)
	{
		case EXTCHA_BELOW:
			snprintf(pBuf, BufLen, "\tBelow");
			break;
		case EXTCHA_ABOVE:
			snprintf(pBuf, BufLen, "\tAbove");
			break;
		default:
			snprintf(pBuf, BufLen, "\tUnknow Value(%u)", ext_cha);
			break;
	}
	return 0;
}


INT	Show_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%u", pAd->CommonCfg.BACapability.field.MpduDensity);
	return 0;
}

INT	Show_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%u", pAd->CommonCfg.BACapability.field.RxBAWinLimit);
	return 0;
}

INT	Show_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bRdg ? "TRUE":"FALSE");
	return 0;
}

INT	Show_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.BACapability.field.AmsduEnable ? "TRUE":"FALSE");
	return 0;
}

INT	Show_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.BACapability.field.AutoBA ? "TRUE":"FALSE");
	return 0;
}
#endif /* DOT11_N_SUPPORT */

INT	Show_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%d", pAd->CommonCfg.CountryRegion);
	return 0;
}

INT	Show_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%d", pAd->CommonCfg.CountryRegionForABand);
	return 0;
}

INT	Show_CountryCode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.CountryCode);
	return 0;
}

#ifdef AGGREGATION_SUPPORT
INT	Show_PktAggregate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bAggregationCapable ? "TRUE":"FALSE");
	return 0;
}
#endif /* AGGREGATION_SUPPORT */

INT	Show_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		snprintf(pBuf, BufLen, "\t%s", pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.bWmmCapable ? "TRUE":"FALSE");
#endif /* CONFIG_AP_SUPPORT */


	return 0;
}


INT	Show_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\t%s", pAd->CommonCfg.bIEEE80211H ? "TRUE":"FALSE");
	return 0;
}


INT	Show_STA_RAInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	sprintf(pBuf, "\n");
#ifdef PRE_ANT_SWITCH
	sprintf(pBuf+strlen(pBuf), "PreAntSwitch: %d\n", pAd->CommonCfg.PreAntSwitch);
	sprintf(pBuf+strlen(pBuf), "PreAntSwitchRSSI: %d\n", pAd->CommonCfg.PreAntSwitchRSSI);
#endif /* PRE_ANT_SWITCH */


#ifdef NEW_RATE_ADAPT_SUPPORT
	sprintf(pBuf+strlen(pBuf), "LowTrafficThrd: %d\n", pAd->CommonCfg.lowTrafficThrd);
	sprintf(pBuf+strlen(pBuf), "TrainUpRule: %d\n", pAd->CommonCfg.TrainUpRule);
	sprintf(pBuf+strlen(pBuf), "TrainUpRuleRSSI: %d\n", pAd->CommonCfg.TrainUpRuleRSSI);
	sprintf(pBuf+strlen(pBuf), "TrainUpLowThrd: %d\n", pAd->CommonCfg.TrainUpLowThrd);
	sprintf(pBuf+strlen(pBuf), "TrainUpHighThrd: %d\n", pAd->CommonCfg.TrainUpHighThrd);
#endif // NEW_RATE_ADAPT_SUPPORT //

#ifdef STREAM_MODE_SUPPORT
	sprintf(pBuf+strlen(pBuf), "StreamMode: %d\n", pAd->CommonCfg.StreamMode);
	sprintf(pBuf+strlen(pBuf), "StreamModeMCS: 0x%04x\n", pAd->CommonCfg.StreamModeMCS);
#endif // STREAM_MODE_SUPPORT //
#ifdef TXBF_SUPPORT
	sprintf(pBuf+strlen(pBuf), "ITxBfEn: %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
	sprintf(pBuf+strlen(pBuf), "ITxBfTimeout: %ld\n", pAd->CommonCfg.ITxBfTimeout);
	sprintf(pBuf+strlen(pBuf), "ETxBfTimeout: %ld\n", pAd->CommonCfg.ETxBfTimeout);
	sprintf(pBuf+strlen(pBuf), "ETxBfEnCond: %ld\n", pAd->CommonCfg.ETxBfEnCond);
	sprintf(pBuf+strlen(pBuf), "ETxBfNoncompress: %d\n", pAd->CommonCfg.ETxBfNoncompress);
	sprintf(pBuf+strlen(pBuf), "ETxBfIncapable: %d\n", pAd->CommonCfg.ETxBfIncapable);
#endif // TXBF_SUPPORT //

#ifdef DBG_CTRL_SUPPORT
	sprintf(pBuf+strlen(pBuf), "DebugFlags: 0x%lx\n", pAd->CommonCfg.DebugFlags);
#endif /* DBG_CTRL_SUPPORT */
	return 0;
}

#ifdef MBSS_SUPPORT
static INT total_connected_sta[HW_BEACON_MAX_NUM][((MAX_LEN_OF_MAC_TABLE/32)+1)];
#endif
#ifdef MAC_REPEATER_SUPPORT
static INT total_connected_repeater[MAX_APCLI_NUM][((MAX_LEN_OF_MAC_TABLE/32)+1)];
#endif
static INT dump_mac_table(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	INT i;
	ULONG DataRate=0;
	ULONG DataRate_r=0;
	ULONG max_DataRate=0;
	INT sta_cnt=0;
	INT apcli_cnt=0;
	INT rept_cnt=0;
	UCHAR	tmp_str[30];
	INT		temp_str_len = sizeof(tmp_str);
	ADD_HT_INFO_IE *addht;
#if defined(MT7615)
	UINT32 ps_stat[4] = {0};
#endif

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);

	printk("\n");

#if defined(MT7615)
	HW_IO_READ32(pAd, PLE_STATION_PAUSE0, &ps_stat[0]);
	HW_IO_READ32(pAd, PLE_STATION_PAUSE1, &ps_stat[1]);
	HW_IO_READ32(pAd, PLE_STATION_PAUSE2, &ps_stat[2]);
	HW_IO_READ32(pAd, PLE_STATION_PAUSE3, &ps_stat[3]);
#endif

#ifdef MBSS_SUPPORT
	memset(&total_connected_sta[0][0],0x00,sizeof(total_connected_sta));
#endif
#ifdef MAC_REPEATER_SUPPORT
	memset(&total_connected_repeater[0][0],0x00,sizeof(total_connected_repeater));
#endif

#ifdef CONFIG_HOTSPOT_R2
	printk("\n%-19s%-6s%-5s%-4s%-4s%-4s%-7s%-20s%-12s%-9s%-12s%-9s%-10s%-7s%-10s%-7s\n",
		         "MAC", "MODE", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0/1/2/3", "PhMd",      "BW",      "MCS",      "SGI",      "STBC",      "Idle", "Rate",     "QosMap");
#else
	printk("\n%-19s%-6s%-5s%-4s%-4s%-4s%-7s%-20s%-12s%-9s%-12s%-9s%-10s%-7s%-10s\n",
		         "MAC", "MODE", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0/1/2/3", "PhMd(T/R)", "BW(T/R)", "MCS(T/R)", "SGI(T/R)", "STBC(T/R)", "Idle", "Rate(T/R)");
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef MWDS
	printk("%-8s","MWDSCap");
#endif /* MWDS */

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if ((ent_type == ENTRY_NONE))
		{
			/* dump all MacTable entries */
			if (pEntry->EntryType == ENTRY_NONE)
				continue;
		} else {
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
				&& (pEntry->Sst != SST_ASSOC))
				continue;

#ifdef MAC_REPEATER_SUPPORT
			if (bReptCli == FALSE)
			{
				/* only dump the apcli entry which not a RepeaterCli */
				if (IS_ENTRY_REPEATER(pEntry) && (pEntry->bReptCli == TRUE))
					continue;
			}
#endif /* MAC_REPEATER_SUPPORT */
		}

		if ((pEntry->wdev == NULL) || HcGetBandByWdev(pEntry->wdev) != ucBand)
			continue;

		if (IS_ENTRY_CLIENT(pEntry))
			sta_cnt++;
		if (IS_ENTRY_APCLI(pEntry))
			apcli_cnt++;
		if (IS_ENTRY_REPEATER(pEntry))
			rept_cnt++;

		addht = wlan_operate_get_addht(pEntry->wdev);
#ifdef DOT11_N_SUPPORT
		printk("HT Operating Mode : %d\n",addht->AddHtInfo2.OperaionMode);
		printk("\n");
#endif /* DOT11_N_SUPPORT */
		
		DataRate=0;
		getRate(pEntry->HTPhyMode, &DataRate);
		if (memcmp(pEntry->Addr,pAd->MonitorAddr,MAC_ADDR_LEN) == 0) {
#define RED(_text)  "\033[1;31m"_text"\033[0m"
			printk(RED("%02X:%02X:%02X:%02X:%02X:%02X  "), PRINT_MAC(pEntry->Addr));
		} else {
		printk("%02X:%02X:%02X:%02X:%02X:%02X  ", PRINT_MAC(pEntry->Addr));
		}
		printk("%-6x", pEntry->EntryType);
		printk("%-5d", (int)pEntry->Aid);
		printk("%-4d", (int)pEntry->func_tb_idx);
#if defined(MT7615)
		if (((1 << (pEntry->wcid%32)) & ps_stat[pEntry->wcid/32]) != 0)
			printk("%-4d", (int)PWR_SAVE);
		else
			printk("%-4d", (int)PWR_ACTIVE);
#else
		printk("%-4d", (int)pEntry->PsMode);
#endif
		printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
		printk("%-7d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
		snprintf(tmp_str,temp_str_len,"%d/%d/%d/%d",pEntry->RssiSample.AvgRssi[0],
				pEntry->RssiSample.AvgRssi[1],
				pEntry->RssiSample.AvgRssi[2],
				pEntry->RssiSample.AvgRssi[3]);
		printk("%-20s", tmp_str);
		
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (pAd->chipCap.fgRateAdaptFWOffload == TRUE && (pEntry->bAutoTxRateSwitch == TRUE))
		{
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
			UCHAR vht_nss;
			UCHAR vht_nss_r;
#endif
			UINT32 RawData;
			UINT32 RawData_r;
			UINT32 lastTxRate = pEntry->LastTxRate;
			UINT32 lastRxRate = pEntry->LastRxRate;
			if (pEntry->bAutoTxRateSwitch == TRUE)
			{
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				HTTRANSMIT_SETTING LastTxRate;
				HTTRANSMIT_SETTING LastRxRate;
				MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);

				LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1:0;
				LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1:0;
				LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;
				if (LastTxRate.field.MODE == MODE_VHT)
				{
					LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
				}
                                else if (LastTxRate.field.MODE == MODE_OFDM)
                                {
                                        LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
                                }
				else
				{
					LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;
				}
				lastTxRate = (UINT32)(LastTxRate.word);
				LastRxRate.word = (USHORT)lastRxRate;
				RawData = lastTxRate;
				phy_mode = (RawData>>13) & 0x7;
				rate = RawData & 0x3F;
				bw = (RawData>>7) & 0x3;
				sgi = (RawData>>9) & 0x1;
				stbc = ((RawData>>10) & 0x1);
//----
				RawData_r = lastRxRate;
				phy_mode_r = (RawData_r>>13) & 0x7;
				rate_r = RawData_r & 0x3F;
				bw_r = (RawData_r>>7) & 0x3;
				sgi_r = (RawData_r>>9) & 0x1;
				stbc_r = ((RawData_r>>10) & 0x1);
				snprintf(tmp_str,temp_str_len,"%s/%s",get_phymode_str(phy_mode),get_phymode_str(phy_mode_r));
				printk("%-12s", tmp_str);
				snprintf(tmp_str,temp_str_len,"%s/%s",get_bw_str(bw),get_bw_str(bw_r));
				printk("%-9s", tmp_str);
#ifdef DOT11_VHT_AC
				if ( phy_mode == MODE_VHT ) {
					vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
					rate = rate & 0xF;
					snprintf(tmp_str,temp_str_len,"%dS-M%d/",vht_nss, rate);
				} else
#endif /* DOT11_VHT_AC */
					snprintf(tmp_str,temp_str_len,"%d/",rate);
				
#ifdef DOT11_VHT_AC
				if ( phy_mode_r == MODE_VHT ) {
					vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
					rate_r = rate_r & 0xF;
					snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%dS-M%d",vht_nss_r, rate_r);
				} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
				if ( phy_mode_r >= MODE_HTMIX ){
						snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",rate_r);
				} else
#endif
				if ( phy_mode_r == MODE_OFDM ) {
					if ( rate_r == TMI_TX_RATE_OFDM_6M )
						LastRxRate.field.MCS = 0;
					else if ( rate_r == TMI_TX_RATE_OFDM_9M )
						LastRxRate.field.MCS = 1;
					else if ( rate_r == TMI_TX_RATE_OFDM_12M )
						LastRxRate.field.MCS = 2;
					else if ( rate_r == TMI_TX_RATE_OFDM_18M )
						LastRxRate.field.MCS = 3;
					else if ( rate_r == TMI_TX_RATE_OFDM_24M )
						LastRxRate.field.MCS = 4;
					else if ( rate_r == TMI_TX_RATE_OFDM_36M )
						LastRxRate.field.MCS = 5;
					else if ( rate_r == TMI_TX_RATE_OFDM_48M )
						LastRxRate.field.MCS = 6;
					else if ( rate_r == TMI_TX_RATE_OFDM_54M )
						LastRxRate.field.MCS = 7;
					else
						LastRxRate.field.MCS = 0;
					snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",LastRxRate.field.MCS);
				} else if ( phy_mode_r == MODE_CCK ) {	
					if ( rate_r == TMI_TX_RATE_CCK_1M_LP )
						LastRxRate.field.MCS = 0;
					else if ( rate_r == TMI_TX_RATE_CCK_2M_LP )
						LastRxRate.field.MCS = 1;
					else if ( rate_r == TMI_TX_RATE_CCK_5M_LP )
						LastRxRate.field.MCS = 2;
					else if ( rate_r == TMI_TX_RATE_CCK_11M_LP )
						LastRxRate.field.MCS = 3;
					else if ( rate_r == TMI_TX_RATE_CCK_2M_SP )
						LastRxRate.field.MCS = 1;
					else if ( rate_r == TMI_TX_RATE_CCK_5M_SP )
						LastRxRate.field.MCS = 2;
					else if ( rate_r == TMI_TX_RATE_CCK_11M_SP )
						LastRxRate.field.MCS = 3;
					else
						LastRxRate.field.MCS = 0;
					snprintf(tmp_str+strlen(tmp_str),temp_str_len-strlen(tmp_str),"%d",LastRxRate.field.MCS);
				}
				printk("%-12s", tmp_str);

				snprintf(tmp_str,temp_str_len,"%d/%d", sgi, sgi_r);
				printk("%-9s", tmp_str);
				snprintf(tmp_str,temp_str_len,"%d/%d",  stbc, stbc_r);
				printk("%-10s", tmp_str);
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
			}
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */		
		{
			printk("%-12s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			printk("%-9s", get_bw_str(pEntry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC
			if (pEntry->HTPhyMode.field.MODE == MODE_VHT)
				snprintf(tmp_str,temp_str_len,"%dS-M%d", ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
			else
#endif /* DOT11_VHT_AC */
			snprintf(tmp_str,temp_str_len,"%d", pEntry->HTPhyMode.field.MCS);
			printk("%-12s", tmp_str);
			printk("%-9d", pEntry->HTPhyMode.field.ShortGI);
			printk("%-10d", pEntry->HTPhyMode.field.STBC);
		}
		
		printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
		snprintf(tmp_str,temp_str_len,"%d/%d",(int)DataRate,(int)DataRate_r);
		printk("%-10s", tmp_str);

#ifdef CONFIG_HOTSPOT_R2
		printk("%-7d", (int)pEntry->QosMapSupport);
#endif
		printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
					(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);
#ifdef CONFIG_HOTSPOT_R2
		if (pEntry->QosMapSupport)
		{
			int k =0;
			printk("DSCP Exception:\n");
			for(k=0;k<pEntry->DscpExceptionCount/2;k++)
			{
				printk("[Value: %4d] [UP: %4d]\n", pEntry->DscpException[k] & 0xff, (pEntry->DscpException[k] >> 8) & 0xff);
			}
			printk("DSCP Range:\n");
			for(k=0;k<8;k++)
			{
				printk("[UP :%3d][Low Value: %4d] [High Value: %4d]\n", k, pEntry->DscpRange[k] & 0xff, (pEntry->DscpRange[k] >> 8) & 0xff);
			}
		}
#endif

#ifdef MWDS
		if(IS_ENTRY_APCLI(pEntry))
		{
			if(pEntry->func_tb_idx < MAX_APCLI_NUM)
			{
				if (pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].MlmeAux.bSupportMWDS)
					printk("%-8s", "YES");
				else
					printk("%-8s", "NO");
			}
		}
		else
		{
			if (pEntry->bSupportMWDS)
				printk("%-8s", "YES");
			else
				printk("%-8s", "NO");
		}
#endif /* MWDS */

//+++Add by shiang for debug
		printk("%69s%-12s","MaxCap:",get_phymode_str(pEntry->MaxHTPhyMode.field.MODE));
		printk("%-9s", get_bw_str(pEntry->MaxHTPhyMode.field.BW));
#ifdef DOT11_VHT_AC
		if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
			snprintf(tmp_str,temp_str_len,"%dS-M%d",((pEntry->MaxHTPhyMode.field.MCS>>4) + 1), (pEntry->MaxHTPhyMode.field.MCS & 0xf));
		else
#endif /* DOT11_VHT_AC */
		snprintf(tmp_str,temp_str_len,"%d",pEntry->MaxHTPhyMode.field.MCS);
		printk("%-12s", tmp_str);
		printk("%-9d", pEntry->MaxHTPhyMode.field.ShortGI);
		printk("%-10d", pEntry->MaxHTPhyMode.field.STBC);
		getRate(pEntry->MaxHTPhyMode, &max_DataRate);
		printk("%-7s","-");
		printk("%-10d", (int)max_DataRate);

#ifdef HTC_DECRYPT_IOT
		printk("%20s%-10d", "HTC_ICVErr:", pEntry->HTC_ICVErrCnt);
		printk("%20s%-10s", "HTC_AAD_OM_Force:", pEntry->HTC_AAD_OM_Force ?"YES":"NO");
#endif /* HTC_DECRYPT_IOT */
		printk("  wdev%d\n", (int)pEntry->wdev->wdev_idx);
//---Add by shiang for debug
		printk("\n");
#ifdef MBSS_SUPPORT
		if (IS_ENTRY_CLIENT(pEntry)) {
			total_connected_sta[pEntry->wdev->wdev_idx][pEntry->wcid/32] = 
				total_connected_sta[pEntry->wdev->wdev_idx][pEntry->wcid/32] | 
				1 << (pEntry->wcid%32);
		}
#endif
#ifdef MAC_REPEATER_SUPPORT
		if (IS_ENTRY_REPEATER(pEntry)) {
			total_connected_repeater[pEntry->wdev->func_idx][pEntry->wcid/32] = 
				total_connected_repeater[pEntry->wdev->func_idx][pEntry->wcid/32] | 
				1 << (pEntry->wcid%32);
		}
#endif
	}
	printk("sta_cnt=%d\n\r", sta_cnt);
	printk("apcli_cnt=%d\n\r", apcli_cnt);
	printk("rept_cnt=%d\n\r", rept_cnt);
#ifdef OUI_CHECK_SUPPORT
	printk("oui_mgroup=%d\n\r",pAd->MacTab.oui_mgroup_cnt);
	printk("repeater_wcid_error_cnt=%d\n\r",pAd->MacTab.repeater_wcid_error_cnt);
	printk("repeater_bm_wcid_error_cnt=%d\n\r",pAd->MacTab.repeater_bm_wcid_error_cnt);
#endif /*OUI_CHECK_SUPPORT*/

#ifdef HTC_DECRYPT_IOT
		printk("HTC_ICV_Err_TH=%d\n\r", pAd->HTC_ICV_Err_TH);
#endif /* HTC_DECRYPT_IOT */

#ifdef MBSS_SUPPORT
	{
		// dump sta belong to which ra
		INT mbss_idx;
		INT sta_idx;
		for(mbss_idx = 0;mbss_idx<pAd->ApCfg.BssidNum;mbss_idx++) {
			printk("\n\rra[%d]", mbss_idx);
			for(sta_idx = 0;sta_idx < MAX_LEN_OF_MAC_TABLE; sta_idx++) {
				if((total_connected_sta[mbss_idx][sta_idx/32] & (1<<(sta_idx%32))) != 0) {
					printk("%d ", sta_idx);
				}
			}
		}
		printk("\n");
	}
#endif
#ifdef MAC_REPEATER_SUPPORT
	{
		// dump repeater belong to which apcli
		INT apcli_idx;
		INT repeater_idx;
		for(apcli_idx = 0;apcli_idx<MAX_APCLI_NUM;apcli_idx++) {
			printk("\n\rapcli[%d]", apcli_idx);
			for(repeater_idx = 0;repeater_idx < MAX_LEN_OF_MAC_TABLE; repeater_idx++) {
				if((total_connected_repeater[apcli_idx][repeater_idx/32] & (1<<(repeater_idx%32))) != 0) {
					INT32 CliIdx;
					RTMP_CHIP_CAP *cap = &pAd->chipCap;
					PREPEATER_CLIENT_ENTRY 		pReptCliEntry_tmp;
					for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
						pReptCliEntry_tmp = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
						if (pReptCliEntry_tmp->MacTabWCID == repeater_idx) {
							MAC_TABLE_ENTRY *entry_tmp = NULL;
							entry_tmp = MacTableLookup(pAd, pReptCliEntry_tmp->OriginalAddress);
							if (entry_tmp != NULL)
								printk("(%d<-%d) ", repeater_idx,entry_tmp->wcid);
							else
								printk("(%d<-Eth) ", repeater_idx);
						}
					}
				}
			}
		}
		printk("\n");
	}
#endif
	return TRUE;
}


INT Show_MacTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ent_type = ENTRY_CLIENT;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): arg=%s\n", __FUNCTION__, (arg == NULL ? "" : arg)));
	if (arg && strlen(arg)) {
		if (rtstrcasecmp(arg, "sta") == TRUE)
			ent_type = ENTRY_CLIENT;
		else if (rtstrcasecmp(arg, "ap") == TRUE)
			ent_type = ENTRY_AP;
		else
			ent_type = ENTRY_NONE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump MacTable entries info, EntType=0x%x\n", ent_type));

	return dump_mac_table(pAd, ent_type, FALSE);
}

#ifdef MT_MAC
#if !defined(MT7615) && !defined(MT7622)
// TODO: shiang-MT7615, fix me!
static INT dump_ps_table(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli)
{
   INT i,j;
   struct wtbl_entry tb_entry;
   union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
   UINT32  rPseRdTabAccessReg;
   BOOLEAN pfgForce;
   UCHAR pucPort, pucQueue;
   INT Total_Packet_Number = 0 ;
   ADD_HT_INFO_IE *addht;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%-19s\t%-10s\t%-5s\t%-5s\t%-5s\t%-6s\t%-5s\t%-5s\t%-5s\t%-5s\t%-5s\t%-5s\t%-6s\t%-6s", "MAC", "EntryType", "AID", "BSS", "PSM", "status", "ipsm", "iips", "sktx", "redt", "port", "queu", "pktnum","psnum"));

#ifdef UAPSD_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-7s", "APSD"));
#endif /* UAPSD_SUPPORT */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
           PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
           STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
           Total_Packet_Number = 0 ;

		if ((ent_type == ENTRY_NONE))
		{
			/* dump all MacTable entries */
			if (pEntry->EntryType == ENTRY_NONE)
				continue;
		}
		else
		{
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
				&& (pEntry->Sst != SST_ASSOC))
				continue;

#ifdef MAC_REPEATER_SUPPORT
			if (bReptCli == FALSE)
			{
				/* only dump the apcli entry which not a RepeaterCli */
				if (IS_ENTRY_REPEATER(pEntry) && (pEntry->bReptCli == TRUE))
					continue;
			}
#endif /* MAC_REPEATER_SUPPORT */
		}

		addht = wlan_operate_get_addht(pEntry->wdev);
#ifdef DOT11_N_SUPPORT
		printk("HT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
#endif /* DOT11_N_SUPPORT */

		NdisZeroMemory(&tb_entry, sizeof(tb_entry));
		if (mt_wtbl_get_entry234(pAd, pEntry->wcid, &tb_entry) == FALSE)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Cannot found WTBL2/3/4\n",__FUNCTION__));
			 return FALSE;
		}
		RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0]+12, &dw3->word);

		//get PSE register

		//      rPseRdTabAccessReg.field.rd_kick_busy=1;
		//      rPseRdTabAccessReg.field.rd_tag=pEntry->wcid;
		rPseRdTabAccessReg = PSE_RTA_RD_KICK_BUSY |PSE_RTA_TAG(pEntry->wcid);
		RTMP_IO_WRITE32(pAd, PSE_RTA,rPseRdTabAccessReg);

		do
		{
			RTMP_IO_READ32(pAd,PSE_RTA,&rPseRdTabAccessReg);

			pfgForce = ( BOOLEAN ) GET_PSE_RTA_RD_RULE_F(rPseRdTabAccessReg);
			pucPort  = ( UCHAR )  GET_PSE_RTA_RD_RULE_PID(rPseRdTabAccessReg);
			pucQueue = ( UCHAR )  GET_PSE_RTA_RD_RULE_QID(rPseRdTabAccessReg);
		}
		while ( GET_PSE_RTA_RD_KICK_BUSY(rPseRdTabAccessReg) == 1 );

		Total_Packet_Number = Total_Packet_Number + tr_entry->ps_queue.Number;
		for (j = 0; j < WMM_QUE_NUM; j++)
			Total_Packet_Number = Total_Packet_Number + tr_entry->tx_queue[j].Number;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X", PRINT_MAC(pEntry->Addr)));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-10x", pEntry->EntryType));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)pEntry->Aid));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)pEntry->func_tb_idx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)pEntry->PsMode));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-6d", (int)tr_entry->ps_state));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)dw3->field.i_psm));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)dw3->field.du_i_psm));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)dw3->field.skip_tx));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)pfgForce));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)pucPort));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-5d", (int)pucQueue));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-6d", (int)Total_Packet_Number));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%-6d", (int)tr_entry->ps_queue.Number));
#ifdef UAPSD_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d,%d,%d,%d",(int)pEntry->bAPSDCapablePerAC[QID_AC_BE], pEntry->bAPSDCapablePerAC[QID_AC_BK], pEntry->bAPSDCapablePerAC[QID_AC_VI], pEntry->bAPSDCapablePerAC[QID_AC_VO]));
#endif /* UAPSD_SUPPORT */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	return TRUE;
}
#endif /* !defined(MT7615) && !defined(MT7622) */

#if defined(MT7615) || defined(MT7622)
static INT dump_ps_table_MT7615(RTMP_ADAPTER *pAd, UINT32 ent_type, BOOLEAN bReptCli)
{
    int i;

	UINT32 ps_stat[4] = {0};
    UINT32 regValue = 0;
	UINT8 bmc_cnt[4] = {0};
	UINT8 ext_bmc_cnt[15] = {0};

	HW_IO_READ32(pAd, PLE_STATION_PAUSE0, &ps_stat[0]);
	HW_IO_READ32(pAd, PLE_STATION_PAUSE1, &ps_stat[1]);
	HW_IO_READ32(pAd, PLE_STATION_PAUSE2, &ps_stat[2]);
	HW_IO_READ32(pAd, PLE_STATION_PAUSE3, &ps_stat[3]);

	HW_IO_READ32(pAd, ARB_BMCCR0, &regValue);
    bmc_cnt[0] = regValue & 0xff;
    bmc_cnt[1] = (regValue & 0xff00) >> 8;
    bmc_cnt[2] = (regValue & 0xff0000) >> 16;
    bmc_cnt[3] = (regValue & 0xff000000) >> 24;

	HW_IO_READ32(pAd, ARB_BMCCR1, &regValue);
    ext_bmc_cnt[0] = regValue & 0xff;
    ext_bmc_cnt[1] = (regValue & 0xff00) >> 8;
    ext_bmc_cnt[2] = (regValue & 0xff0000) >> 16;
    ext_bmc_cnt[3] = (regValue & 0xff000000) >> 24;

	HW_IO_READ32(pAd, ARB_BMCCR2, &regValue);
    ext_bmc_cnt[4] = regValue & 0xff;
    ext_bmc_cnt[5] = (regValue & 0xff00) >> 8;
    ext_bmc_cnt[6] = (regValue & 0xff0000) >> 16;
    ext_bmc_cnt[7] = (regValue & 0xff000000) >> 24;

	HW_IO_READ32(pAd, ARB_BMCCR3, &regValue);
    ext_bmc_cnt[8] = regValue & 0xff;
    ext_bmc_cnt[9] = (regValue & 0xff00) >> 8;
    ext_bmc_cnt[10] = (regValue & 0xff0000) >> 16;
    ext_bmc_cnt[11] = (regValue & 0xff000000) >> 24;

	HW_IO_READ32(pAd, ARB_BMCCR4, &regValue);
    ext_bmc_cnt[12] = regValue & 0xff;
    ext_bmc_cnt[13] = (regValue & 0xff00) >> 8;
    ext_bmc_cnt[14] = (regValue & 0xff0000) >> 16;


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PS_info:\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%08x %08x %08x %08x\n", ps_stat[0], ps_stat[1], ps_stat[2], ps_stat[3]));

    for (i = 0; i < 4; i++){
        if (bmc_cnt[i])
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BMC(%x)%d ", i, bmc_cnt[i]));
    }

    for (i = 0; i < 15; i++){
        if (ext_bmc_cnt[i])
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BMC(%x)%d ", i+0x11, ext_bmc_cnt[i]));
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n"));

    //Dump PS info from FW
    CmdExtGeneralTestAPPWS(pAd, APPWS_ACTION_DUMP_INFO);


	return TRUE;
}
#endif /* defined(MT7615) || defined(MT7622) */

INT Show_PSTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ent_type = ENTRY_CLIENT;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): arg=%s\n", __FUNCTION__, (arg == NULL ? "" : arg)));
	if (arg && strlen(arg)) {
		if (rtstrcasecmp(arg, "sta") == TRUE)
			ent_type = ENTRY_CLIENT;
		else if (rtstrcasecmp(arg, "ap") == TRUE)
			ent_type = ENTRY_AP;
		else
			ent_type = ENTRY_NONE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump MacTable entries info, EntType=0x%x\n", ent_type));

#if defined(MT7615) || defined(MT7622)
	return dump_ps_table_MT7615(pAd, ent_type, FALSE);;
#else
	return dump_ps_table(pAd, ent_type, FALSE);
#endif /* defined(MT7615) || defined(MT7622) */
}
#endif /* MT_MAC */


#ifdef DOT11_N_SUPPORT
INT Show_BaTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i, j;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;
	RTMP_STRING tmpBuf[10];

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;

		if (IS_ENTRY_APCLI(pEntry))
			strcpy(tmpBuf, "ApCli");
        if (IS_ENTRY_REPEATER(pEntry))
            strcpy(tmpBuf, "Repeater");
		else if (IS_ENTRY_WDS(pEntry))
			strcpy(tmpBuf, "WDS");
		else if (IS_ENTRY_MESH(pEntry))
			strcpy(tmpBuf, "Mesh");
		else if (IS_ENTRY_AP(pEntry))
			strcpy(tmpBuf, "AP");
		else
			strcpy(tmpBuf, "STA");

		printk("%02X:%02X:%02X:%02X:%02X:%02X (Aid = %d) (%s) -\n",
			PRINT_MAC(pEntry->Addr), pEntry->Aid, tmpBuf);

		printk("[Recipient]\n");
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Total Recipient Nums = %ld\n", pAd->BATable.numAsRecipient));

		for (j=0; j < NUM_OF_TID; j++)
		for (j=0; j < NUM_OF_TID; j++)
		{
			if (pEntry->BARecWcidArray[j] != 0)
			{
				pRecBAEntry =&pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]];
				printk("TID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d, FreeMpduBls=%d\n", j, pRecBAEntry->BAWinSize, pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen, pAd->mpdu_blk_pool.freelist.qlen);
			}
		}
		printk("\n");

		printk("[Originator]\n");
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Total Originator Nums = %ld\n", pAd->BATable.numAsOriginator));

		for (j=0; j < NUM_OF_TID; j++)
		{
			if (pEntry->BAOriWcidArray[j] != 0)
			{
				pOriBAEntry =&pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]];
				printk("TID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n",
						j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence,
						pAd->MacTab.tr_entry[pEntry->wcid].TxSeq[j]);
			}
		}
		printk("\n\n");
	}

	return TRUE;
}
#endif /* DOT11_N_SUPPORT */


#ifdef MT_MAC
INT show_wtbl_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i, start, end, idx = -1;
	//WTBL_ENTRY wtbl_entry;

	if (arg == NULL)
	{
		return TRUE;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): arg=%s\n", __FUNCTION__, (arg == NULL ? "" : arg)));
	if (strlen(arg)) {
		idx = simple_strtoul(arg, NULL, 10);
		start = end = idx;
	} else {
		start = 0;
		end = pAd->mac_ctrl.wtbl_entry_cnt[0] - 1;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump WTBL entries info, start=%d, end=%d, idx=%d\n",
				start, end, idx));

	dump_wtbl_base_info(pAd);
	for (i = start; i <= end; i++)
	{
		dump_wtbl_info(pAd, i);
	}
	return TRUE;
}

INT show_wtbltlv_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *Param;
	UCHAR ucWcid = 0;
	UCHAR ucCmdId = 0;
	UCHAR ucAction = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s::param=%s\n", __FUNCTION__, arg));

	if (arg == NULL)
	{
		goto error;
	}

	Param = rstrtok(arg, ":");
	if (Param != NULL)
		ucWcid = simple_strtol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ":");
	if (Param != NULL)
		ucCmdId = simple_strtol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ":");
	if (Param != NULL)
		ucAction = simple_strtol(Param, 0, 10);
	else
		goto error;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s():ucWcid(%d), CmdId(%d), Action(%d)\n",  __FUNCTION__, ucWcid, ucCmdId, ucAction));

	mt_wtbltlv_debug(pAd, ucWcid, ucCmdId, ucAction);

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: param = %s not correct\n", __FUNCTION__, arg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: iwpriv ra0 show wtbltlv=Wcid,CmdId,Action\n", __FUNCTION__));
	return 0;
}


INT show_mib_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
	{

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		dump_dmac_mib_info(pAd, arg);
#endif /* defined(MT7615) || defined(MT7622) */

	return TRUE;
}

#ifdef DBDC_MODE
INT32 ShowDbdcProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcShowBandInfo(pAd);
	return TRUE;
}
#endif


static UINT16 txop_to_ms(UINT16 *txop_level)
{
    UINT16 ms = (*txop_level) >> 5;

    ms += ((*txop_level) & (1<<4))?1:0;

    return ms;
}

static void dump_txop_level(UINT16 *txop_level, UINT32 len)
{
    UINT32 prio;

    for (prio=0; prio<len; prio++) {
        UINT16 ms = txop_to_ms(txop_level+prio);
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                (" {%x:0x%x(%ums)} ", prio, *(txop_level+prio), ms));
    }
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
}

static void dump_tx_burst_info(struct _RTMP_ADAPTER *pAd)
{
    struct wifi_dev **wdev = pAd->wdev_list;
    EDCA_PARM *edca_param = NULL;
    UINT32 idx = 0;
    UCHAR wmm_idx = 0;
    UCHAR bss_idx = 0xff;

    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("[%s]\n", __FUNCTION__));
    do {
        if (wdev[idx] == NULL)
            break;

        if (bss_idx != wdev[idx]->bss_info_argument.ucBssIndex) {
            edca_param = HcGetEdca(pAd, wdev[idx]);
            if (edca_param == NULL)
                break;
            wmm_idx = edca_param->WmmSet;

            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    ("<bss_%x>\n", wdev[idx]->bss_info_argument.ucBssIndex));
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    (" |-[wmm_idx]: %x\n", wmm_idx));
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    (" |-[bitmap]: %08x\n", wdev[idx]->bss_info_argument.prio_bitmap));
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    (" |-[prio:level]:"));
            dump_txop_level(wdev[idx]->bss_info_argument.txop_level, MAX_PRIO_NUM);
            bss_idx = wdev[idx]->bss_info_argument.ucBssIndex;
        }
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" |---<wdev_%x>\n", idx));
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("      |-[bitmap]: %08x\n", wdev[idx]->prio_bitmap));
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("      |-[prio:level]:"));
        dump_txop_level(wdev[idx]->txop_level, MAX_PRIO_NUM);

        idx++;
    } while (idx < WDEV_NUM_MAX);
}


INT32 show_tx_burst_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    dump_tx_burst_info(pAd);
	return TRUE;
}


INT32 show_wifi_sys(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	WifiSysInfoDump(pAd);
	return TRUE;
}

INT32 ShowTmacInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Value;

	RTMP_IO_READ32(pAd, TMAC_TCR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TX Stream = %d\n", GET_TMAC_TCR_TX_STREAM_NUM(Value) + 1));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TX RIFS Enable = %d\n", GET_TX_RIFS_EN(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RX RIFS Mode = %d\n", GET_RX_RIFS_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXOP TBTT Control = %d\n", GET_TXOP_TBTT_CONTROL(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXOP TBTT Stop Control = %d\n", GET_TBTT_TX_STOP_CONTROL(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXOP Burst Stop = %d\n", GET_TXOP_BURST_STOP(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDG Mode = %d\n", GET_RDG_RA_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDG Responser Enable = %d\n", GET_RDG_RESP_EN(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Smoothing = %d\n", GET_SMOOTHING(Value)));

	RTMP_IO_READ32(pAd, TMAC_PSCR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Save RXPE Off Time(unit 2us) = %d\n", GET_APS_OFF_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Save RXPE On Time(unit 2us) = %d\n", APS_ON_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Save Halt Time (unit 32us) = %d\n", GET_APS_HALT_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP Power Enable = %d\n", GET_APS_EN(Value)));

	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC0 TXOP = 0x%x (unit: 32us)\n", GET_AC0LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC1 TXOP = 0x%x (unit: 32us)\n", GET_AC1LIMIT(Value)));

	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC2 TXOP = 0x%x (unit: 32us)\n", GET_AC2LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC3 TXOP = 0x%x (unit: 32us)\n", GET_AC3LIMIT(Value)));

	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR3, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC10 TXOP = 0x%x (unit: 32us)\n", GET_AC10LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC11 TXOP = 0x%x (unit: 32us)\n", GET_AC11LIMIT(Value)));

	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC12 TXOP = 0x%x (unit: 32us)\n", GET_AC12LIMIT(Value)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC13 TXOP = 0x%x (unit: 32us)\n", GET_AC13LIMIT(Value)));
#if defined(MT7615) || defined(MT7622)
	RTMP_IO_READ32(pAd, TMAC_ICR_BAND_0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SIFS Time, Band0 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SLOT Time, Band0 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value)));
	RTMP_IO_READ32(pAd, TMAC_ICR_BAND_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SIFS Time, Band1 (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SLOT Time, Band1 (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value)));

#else
	RTMP_IO_READ32(pAd, TMAC_ICR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EIFS Time (unit: 1us) = %d\n", GET_ICR_EIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RIFS Time (unit: 1us) = %d\n", GET_ICR_RIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SIFS Time (unit: 1us) = %d\n", GET_ICR_SIFS_TIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SLOT Time (unit: 1us) = %d\n", GET_ICR_SLOT_TIME(Value)));

#endif /* defined(MT7615) || defined(MT7622) */

#if defined(MT7615) || defined(MT7622)
	RTMP_IO_READ32(pAd, TMAC_ATCR, &Value);
#else
	RTMP_IO_READ32(pAd, ATCR, &Value);
#endif /* defined(MT7615) || defined(MT7622) */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Aggregation Timeout (unit: 50ns) = 0x%x\n", GET_AGG_TOUT(Value)));

	return 0;
}


INT32 ShowAggInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Value;

	RTMP_IO_READ32(pAd, AGG_PCR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MM Protection = %d\n", GET_MM_PROTECTION(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("GF Protection = %d\n", GET_GF_PROTECTION(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Protection Mode = %d\n", GET_PROTECTION_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BW40 Protection = %d\n", GET_BW40_PROTECTION(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RIFS Protection = %d\n", GET_RIFS_PROTECTION(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BW80 Protection = %d\n", GET_BW80_PROTECTION(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BW160 Protection = %d\n", GET_BW160_PROTECTION(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERP Protection = 0x%x\n", GET_ERP_PROTECTION(Value)));

	RTMP_IO_READ32(pAd, AGG_PCR1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RTS Threshold(packet length) = 0x%x\n", GET_RTS_THRESHOLD(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RTS PKT Nums Threshold = %d\n", GET_RTS_PKT_NUM_THRESHOLD(Value)));
	RTMP_IO_READ32(pAd, AGG_MRCR, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RTS Retry Count Limit = %d\n", GET_RTS_RTY_CNT_LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BAR Frame Tx Count Limit = %d\n", GET_BAR_TX_CNT_LIMIT(Value)));
#if defined(MT7615) || defined(MT7622)
	RTMP_IO_READ32(pAd, AGG_ACR0, &Value);
#else
	RTMP_IO_READ32(pAd, AGG_ACR, &Value);
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AMPDU No BA Rule = %d\n", GET_AMPDU_NO_BA_RULE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AMPDU No BA AR Rule = %d\n", GET_AGG_ACR_AMPDU_NO_BA_AR_RULE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BAR Tx Rate = 0x%x\n", GET_BAR_RATE_TX_RATE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BAR Tx Mode = 0x%x\n", GET_BAR_RATE_TX_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BAR Nsts = %d\n", GET_BAR_RATE_NSTS(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BAR STBC = %d\n", GET_BAR_RATE_STBC(Value)));

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		RTMP_IO_READ32(pAd, AGG_AALCR0, &Value);
#else
	RTMP_IO_READ32(pAd, AGG_AALCR, &Value);
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC0 Agg limit = %d\n", GET_AC0_AGG_LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC1 Agg limit = %d\n", GET_AC1_AGG_LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC2 Agg limit = %d\n", GET_AC2_AGG_LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC3 Agg limit = %d\n", GET_AC3_AGG_LIMIT(Value)));

	RTMP_IO_READ32(pAd, AGG_AALCR1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC10 Agg limit = %d\n", GET_AC10_AGG_LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC11 Agg limit = %d\n", GET_AC11_AGG_LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC12 Agg limit = %d\n", GET_AC12_AGG_LIMIT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AC13 Agg limit = %d\n", GET_AC13_AGG_LIMIT(Value)));

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		RTMP_IO_READ32(pAd, AGG_AWSCR0, &Value);
#else
	RTMP_IO_READ32(pAd, AGG_AWSCR, &Value);
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize0 limit = %d\n", GET_WINSIZE0(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize1 limit = %d\n", GET_WINSIZE1(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize2 limit = %d\n", GET_WINSIZE2(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize3 limit = %d\n", GET_WINSIZE3(Value)));

	RTMP_IO_READ32(pAd, AGG_AWSCR1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize4 limit = %d\n", GET_WINSIZE4(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize5 limit = %d\n", GET_WINSIZE5(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize6 limit = %d\n", GET_WINSIZE6(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Winsize7 limit = %d\n", GET_WINSIZE7(Value)));

	return 0;
}


INT ShowManualTxOP(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 txop;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CURRENT: ManualTxOP = %d\n", pAd->CommonCfg.ManualTxop));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : bEnableTxBurst = %d\n", pAd->CommonCfg.bEnableTxBurst));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : MacTab.Size = %d\n", pAd->MacTab.Size));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : RDG_ACTIVE = %d\n", RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE)));
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &txop);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : AC0 TxOP = 0x%x\n", GET_AC0LIMIT(txop)));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       : AC1 TxOP = 0x%x\n", GET_AC1LIMIT(txop)));

    return TRUE;
}


INT32 ShowPseInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		dump_dmac_pse_info(pAd);
#endif


	return TRUE;
}


INT32 ShowPseData(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *Param;
	UINT8 StartFID, FrameNums;

	Param = rstrtok(arg, ",");
	if (Param != NULL)
		StartFID = simple_strtol(Param, 0, 10);
	else
		goto error;

	Param = rstrtok(NULL, ",");
	if (Param != NULL)
		FrameNums = simple_strtol(Param, 0, 10);
	else
		goto error;

#if defined(MT7615) || defined(MT7622)
	if (IS_MT7615(pAd) || IS_MT7622(pAd))
		dump_dmac_pse_data(pAd, StartFID, FrameNums);
#endif /* defined(MT7615) || defined(MT7622) */


	return 0;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: param = %s not correct\n", __FUNCTION__, arg));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: iwpriv ra0 show psedata=startfid,framenums\n", __FUNCTION__));
	return 0;
	}


#if defined(MT7615) || defined(MT7622)
static INT ple_pg_cnt[]={512,1024,1024,2048, 1536, 3072, 2048, 4095, 0};
static PCHAR sta_ctrl_reg[]={"ENABLE", "DISABLE", "PAUSE"};
#if defined(MT7615)
static PCHAR Queue_Empty_name[] = 
	{"CPU Q0", "CPU Q1","CPU Q2", "CPU Q3",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //4~15 not defined
	"ALTX Q	0", "BMC Q0", "BCN Q0", "PSMP Q0", "ALTX Q1", "BMC Q1", "BCN Q1", "PSMP Q1", //Q16~Q23
	NULL, NULL, NULL, NULL, NULL, NULL, "RLS Q", "RLS2 Q"}; // 24~29 not defined
#elif defined(MT7622)
static PCHAR Queue_Empty_name[] = 
	{"CPU Q0", "CPU Q1","CPU Q2", "CPU Q3", 
	NULL, NULL, NULL, NULL, //4~7 not defined
	"ALTX Q	0", "BMC Q0", "BCN Q0", "PSMP Q0", "ALTX Q1", "BMC Q1", "BCN Q1", "PSMP Q1", //Q8~Q15
	"NAF Q", "NBCN Q", NULL, NULL, NULL, NULL, NULL, NULL, // 18~23 not defined
	NULL, NULL, NULL, NULL, NULL, NULL, "RLS Q", "RLS2 Q"}; // 24~29 not defined
#endif
INT ShowPLEInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ple_buf_ctrl[3] = {0}, pg_sz, pg_num, bit_field_1, bit_field_2;
	UINT32 ple_stat[17] = {0}, pg_flow_ctrl[6] = {0};
	UINT32 sta_pause[4] = {0}, dis_sta_map[4] = {0};
	UINT32 fpg_cnt, ffa_cnt, fpg_head, fpg_tail, hif_max_q, hif_min_q;
	UINT32 rpg_hif, upg_hif, cpu_max_q, cpu_min_q, rpg_cpu, upg_cpu;
	INT32 i, j;
	UINT32 dumptxd = 0;

	if(arg != NULL)
		dumptxd = simple_strtoul(arg, 0, 16);


	HW_IO_READ32(pAd, PLE_PBUF_CTRL, &ple_buf_ctrl[0]);
	HW_IO_READ32(pAd, PLE_RELEASE_CTRL, &ple_buf_ctrl[1]);
	HW_IO_READ32(pAd, PLE_HIF_REPORT, &ple_buf_ctrl[2]);

	HW_IO_READ32(pAd, PLE_QUEUE_EMPTY, &ple_stat[0]);
	HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_0, &ple_stat[1]);
	HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_1, &ple_stat[2]);
	HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_2, &ple_stat[3]);
	HW_IO_READ32(pAd, PLE_AC0_QUEUE_EMPTY_3, &ple_stat[4]);
	HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_0, &ple_stat[5]);
	HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_1, &ple_stat[6]);
	HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_2, &ple_stat[7]);
	HW_IO_READ32(pAd, PLE_AC1_QUEUE_EMPTY_3, &ple_stat[8]);
	HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_0, &ple_stat[9]);
	HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_1, &ple_stat[10]);
	HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_2, &ple_stat[11]);
	HW_IO_READ32(pAd, PLE_AC2_QUEUE_EMPTY_3, &ple_stat[12]);
	HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_0, &ple_stat[13]);
	HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_1, &ple_stat[14]);
	HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_2, &ple_stat[15]);
	HW_IO_READ32(pAd, PLE_AC3_QUEUE_EMPTY_3, &ple_stat[16]);

	HW_IO_READ32(pAd, PLE_FREEPG_CNT, &pg_flow_ctrl[0]);
	HW_IO_READ32(pAd, PLE_FREEPG_HEAD_TAIL, &pg_flow_ctrl[1]);
	HW_IO_READ32(pAd, PLE_PG_HIF_GROUP, &pg_flow_ctrl[2]);
	HW_IO_READ32(pAd, PLE_HIF_PG_INFO, &pg_flow_ctrl[3]);
	HW_IO_READ32(pAd, PLE_PG_CPU_GROUP, &pg_flow_ctrl[4]);
	HW_IO_READ32(pAd, PLE_CPU_PG_INFO, &pg_flow_ctrl[5]);

	HW_IO_READ32(pAd, DIS_STA_MAP0, &dis_sta_map[0]);
	HW_IO_READ32(pAd, DIS_STA_MAP1, &dis_sta_map[1]);
	HW_IO_READ32(pAd, DIS_STA_MAP2, &dis_sta_map[2]);
	HW_IO_READ32(pAd, DIS_STA_MAP3, &dis_sta_map[3]);
	HW_IO_READ32(pAd, STATION_PAUSE0, &sta_pause[0]);
	HW_IO_READ32(pAd, STATION_PAUSE1, &sta_pause[1]);
	HW_IO_READ32(pAd, STATION_PAUSE2, &sta_pause[2]);
	HW_IO_READ32(pAd, STATION_PAUSE3, &sta_pause[3]);

	//Configuration Info
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("PLE Configuration Info:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tPacket Buffer Control(0x82060014): 0x%08x\n", ple_buf_ctrl[0]));
	pg_sz = (ple_buf_ctrl[0] & (0x1<<31))>>31;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 128 : 64)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tPage Offset=%d(in unit of 64KB)\n", (ple_buf_ctrl[0] & (0xf<<20))>>20));
	pg_num = (ple_buf_ctrl[0] & (0xf<<16))>>16;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tConfigured Total Page=%d(%d pages)\n", pg_num, (pg_num < 8 ? ple_pg_cnt[pg_num] : 0)));
	pg_num = (ple_buf_ctrl[0] & 0xfff);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tAvailable Total Page=%d pages\n", (ple_buf_ctrl[0] & 0xfff)));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tRelease Control(0x82060030): 0x%08x\n", ple_buf_ctrl[1]));
	bit_field_1 = (ple_buf_ctrl[1] & 0x1f);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 6)) >> 6);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tNormalTx Release Pid/Qid=%d/%d\n", bit_field_2, bit_field_1));
	bit_field_1 = ((ple_buf_ctrl[1] & (0x1f << 8)) >> 8);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 14)) >> 14);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tDropTx Release Pid/Qid=%d/%d\n", bit_field_2, bit_field_1));
	bit_field_1 = ((ple_buf_ctrl[1] & (0x1f << 16)) >> 16);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 22)) >> 22);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tBCN0 Release Pid/Qid=%d/%d\n", bit_field_2, bit_field_1));
	bit_field_1 = ((ple_buf_ctrl[1] & (0x1f << 24)) >> 24);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 30)) >> 30);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tBCN1 Release Pid/Qid=%d/%d\n", bit_field_2, bit_field_1));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tHIF Report Control(0x82060034): 0x%08x\n", ple_buf_ctrl[2]));
	bit_field_1 = ((ple_buf_ctrl[2] & (0x1<<1)) >> 1);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tHostReportQSel/HostReportDisable=%d/%d\n",
				(ple_buf_ctrl[2] & 0x1), bit_field_1));
	//Page Flow Control
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("PLE Page Flow Control:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tFree page counter(0x82060100): 0x%08x\n", pg_flow_ctrl[0]));
	fpg_cnt = pg_flow_ctrl[0] & 0xfff;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe toal page number of free=0x%03x\n",fpg_cnt));
	ffa_cnt = (pg_flow_ctrl[0] & (0xfff << 16)) >> 16;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe free page numbers of free for all=0x%03x\n",ffa_cnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tFree page head and tail(0x82060104): 0x%08x\n", pg_flow_ctrl[1]));
	fpg_head = pg_flow_ctrl[1] & 0xfff;
	fpg_tail = (pg_flow_ctrl[1] & (0xfff << 16)) >> 16;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe tail/head page of free page list=0x%03x/0x%03x\n",fpg_tail ,fpg_head));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tReserved page counter of HIF group(0x82060110): 0x%08x\n", pg_flow_ctrl[2]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tHIF group page status(0x82060114): 0x%08x\n", pg_flow_ctrl[3]));
	hif_min_q = pg_flow_ctrl[2] & 0xfff;
	hif_max_q = (pg_flow_ctrl[2] & (0xfff << 16)) >> 16;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n",hif_max_q ,hif_min_q));
	rpg_hif = pg_flow_ctrl[3] & 0xfff;
	upg_hif = (pg_flow_ctrl[3] & (0xfff << 16)) >> 16;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n",upg_hif ,rpg_hif));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tReserved page counter of CPU group(0x82060150): 0x%08x\n", pg_flow_ctrl[4]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tCPU group page status(0x82060154): 0x%08x\n", pg_flow_ctrl[5]));
	cpu_min_q = pg_flow_ctrl[4] & 0xfff;
	cpu_max_q = (pg_flow_ctrl[4] & (0xfff << 16)) >> 16;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n",cpu_max_q ,cpu_min_q));
	rpg_cpu = pg_flow_ctrl[5] & 0xfff;
	upg_cpu = (pg_flow_ctrl[5] & (0xfff << 16)) >> 16;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n",upg_cpu ,rpg_cpu));

	if(((ple_stat[0] & (0x1 << 24)) >> 24) == 0)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC0_QUEUE_EMPTY0(0x82060300): 0x%08x\n", ple_stat[1]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC0_QUEUE_EMPTY1(0x82060304): 0x%08x\n", ple_stat[2]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC0_QUEUE_EMPTY2(0x82060308): 0x%08x\n", ple_stat[3]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC0_QUEUE_EMPTY3(0x8206030c): 0x%08x\n", ple_stat[4]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC1_QUEUE_EMPTY0(0x82060310): 0x%08x\n", ple_stat[5]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC1_QUEUE_EMPTY1(0x82060314): 0x%08x\n", ple_stat[6]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC1_QUEUE_EMPTY2(0x82060318): 0x%08x\n", ple_stat[7]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC1_QUEUE_EMPTY3(0x8206031c): 0x%08x\n", ple_stat[8]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC2_QUEUE_EMPTY0(0x82060320): 0x%08x\n", ple_stat[9]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC2_QUEUE_EMPTY1(0x82060324): 0x%08x\n", ple_stat[10]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC2_QUEUE_EMPTY2(0x82060328): 0x%08x\n", ple_stat[11]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC2_QUEUE_EMPTY3(0x8206032c): 0x%08x\n", ple_stat[12]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC3_QUEUE_EMPTY0(0x82060330): 0x%08x\n", ple_stat[13]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC3_QUEUE_EMPTY1(0x82060334): 0x%08x\n", ple_stat[14]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC3_QUEUE_EMPTY2(0x82060338): 0x%08x\n", ple_stat[15]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tAC3_QUEUE_EMPTY3(0x8206033c): 0x%08x\n", ple_stat[16]));
		for(j = 0;j < 16;j++)
		{
			if(j % 4 == 0)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\n\tNonempty AC%d Q of STA#: ", j/4));
			}
			for(i = 0;i < 32;i++)
			{
				if(((ple_stat[j+1] & (0x1 << i)) >> i) == 0)
				{
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%d ",i + (j % 4) * 32));
				}
			}

		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Nonempty Q info:\n"));
	for(i = 0;i < 31;i++)
	{
		if(((ple_stat[0] & (0x1 << i)) >> i) == 0)
		{
			UINT32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};
			if (Queue_Empty_name[i] != NULL)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s: ", Queue_Empty_name[i]));
			else
				continue;
			if(i < 4)
				fl_que_ctrl[0] |= (ENUM_UMAC_CPU_PORT_1 << 14);
			else if (i > 29)
				fl_que_ctrl[0] |= (ENUM_PLE_CTRL_PSE_PORT_3 << 14);
			else
				fl_que_ctrl[0] |= (ENUM_UMAC_LMAC_PORT_2 << 14);
	
			fl_que_ctrl[0] |= (0x1 << 31);
			fl_que_ctrl[0] |= (i << 8);
			HW_IO_WRITE32(pAd, PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
			HW_IO_READ32(pAd, PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
			HW_IO_READ32(pAd, PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);

			hfid = fl_que_ctrl[1] & 0xfff;
			tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
			pktcnt = fl_que_ctrl[2] & 0xfff;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
								("tail/head fid = 0x%03x/0x%03x, pkt cnt = %x\n",
									tfid, hfid, pktcnt));
			if(pktcnt > 0 && dumptxd > 0) {
				ShowTXDInfo(pAd, hfid);
			}


		}
	}
	for(j = 0;j < 16;j++) //show AC Q info
		{
			for(i = 0;i < 32;i++)
			{
				if(((ple_stat[j+1] & (0x1 << i)) >> i) == 0)
				{
					UINT32 hfid, tfid, pktcnt, ac_num = j/4, ctrl = 0;
					UINT32 sta_num = i + (j % 4) * 32, fl_que_ctrl[3] = {0};
					struct wifi_dev *wdev = WdevSearchByWcid(pAd, sta_num);
					UINT32 wmmidx = 0;

					if (wdev)
					    wmmidx = HcGetWmmIdx(pAd, wdev);

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\tSTA%d AC%d: ",sta_num, ac_num));

					fl_que_ctrl[0] |= (0x1 << 31);
					fl_que_ctrl[0] |= (0x2 << 14);
					fl_que_ctrl[0] |= (ac_num << 8);
					fl_que_ctrl[0] |= sta_num;
					HW_IO_WRITE32(pAd, PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
					HW_IO_READ32(pAd, PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
					HW_IO_READ32(pAd, PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);

					hfid = fl_que_ctrl[1] & 0xfff;
					tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
					pktcnt = fl_que_ctrl[2] & 0xfff;

					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
								("tail/head fid = 0x%03x/0x%03x, pkt cnt = %x",
									tfid, hfid, pktcnt));
					if(((sta_pause[j % 4] & 0x1 << i) >> i) == 1)
						ctrl = 2;
					if(((dis_sta_map[j % 4] & 0x1 << i) >> i) == 1)
						ctrl = 1;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
								(" ctrl = %s", sta_ctrl_reg[ctrl]));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				                (" (wmmidx=%d)\n", wmmidx));
				if(pktcnt > 0 && dumptxd > 0)
						ShowTXDInfo(pAd, hfid);
					}

			}
		}
	return TRUE;
}
INT show_TXD_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT fid;
	if (arg == NULL)
		return FALSE;

	if(strlen(arg) == 0)
		return FALSE;

	fid = simple_strtol(arg, 0, 16);
	return ShowTXDInfo(pAd, fid);
}
#define UMAC_FID_FAULT	0xFFF
#define DUMP_MEM_SIZE 64
INT ShowTXDInfo(RTMP_ADAPTER *pAd, UINT fid)
{
	INT i = 0;
	UINT8 data[DUMP_MEM_SIZE];
	UINT32 Addr=0;

	if (fid >= UMAC_FID_FAULT) 
		return FALSE;
	os_zero_mem(data, DUMP_MEM_SIZE);
	Addr = 0xa << 28 | fid << 16; /* TXD addr: 0x{a}{fid}{0000}*/
	MtCmdMemDump(pAd, Addr, &data[0]);

	for (i = 0;i < DUMP_MEM_SIZE;i = i + 4) 
	   	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DW%02d: 0x%02x%02x%02x%02x\n",i/4,data[i+3],data[i+2],data[i+1],data[i]));

	dump_tmac_info(pAd,&data[0]);
    	return TRUE;
}
INT show_mem_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT addr = simple_strtol(arg, 0, 16);
	UINT8 data[DUMP_MEM_SIZE];
	INT i = 0;

	os_zero_mem(data, DUMP_MEM_SIZE);
	MtCmdMemDump(pAd, addr, &data[0]);
	for (i = 0;i < DUMP_MEM_SIZE;i = i + 4) 
	   	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("addr 0x%08x: 0x%02x%02x%02x%02x\n",addr+i,data[i+3],data[i+2],data[i+1],data[i]));

	return TRUE;
}


INT show_protect_info (RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev **wdev = pAd->wdev_list;
	UINT32 idx = 0;
	UINT32 val;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("[%s]\n", __FUNCTION__));
	do {
		if (wdev[idx] == NULL)
			break;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("wdev[%x]: %u NON ERP STA(s) joined, Use_Protection = %x",
				 idx, (UINT32) wdev[idx]->conn_sta.nonerp_sta_cnt,
				 wdev[idx]->prot_cfg.erp.use_protection));

		idx++;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,("\n"));
	} while (idx < WDEV_NUM_MAX);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(" -Proetction\n"));
	RTMP_IO_READ32(pAd, AGG_PCR, &val);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("  > AGG_PCR 0x%08x\n", val));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(" -Proetction ERP PPDU over BW20\n"));
	RTMP_IO_READ32(pAd, AGG_SCR, &val);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("  > AGG_SCR 0x%08x\n", val));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(" -RTS Threshold\n"));
	RTMP_IO_READ32(pAd, AGG_PCR1, &val);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("  > AGG_PCR1 0x%08x\n", val));

    return TRUE;
}


INT show_cca_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 val;

    MAC_IO_READ32(pAd, RMAC_DEBUG_CR, &val);
    val |= (1<<31); // For Band0
    MAC_IO_WRITE32(pAd, RMAC_DEBUG_CR, val);
    // Debug CR
    MAC_IO_WRITE32(pAd, (WF_CFG_OFF_BASE + 0x2c), 0xf);
    MAC_IO_WRITE32(pAd, (WF_CFG_BASE + 0x14), 0x1f);
    MAC_IO_WRITE32(pAd, (WF_CFG_BASE + 0x18), 0x06060606);
    MAC_IO_WRITE32(pAd, (WF_CFG_BASE + 0x4c), 0x1c1c1d1d);
    MAC_IO_READ32(pAd, (WF_CFG_BASE + 0x24), &val);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("CCA for BAND0 info:\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
             ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
             ((val & (1 << 5)) >> 5)));

    MAC_IO_READ32(pAd, RMAC_DEBUG_CR, &val);
    val &= ~(1<<31); // For Band1
    MAC_IO_WRITE32(pAd, RMAC_DEBUG_CR, val);
    MAC_IO_READ32(pAd, (WF_CFG_BASE + 0x24), &val);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("CCA for BAND1 info:\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("-- CCA Prim: %d, SE20: %d, SEC40: %d\n",
             ((val & (1 << 14)) >> 14), ((val & (1 << 6)) >> 6),
             ((val & (1 << 5)) >> 5)));
	return 0;
}
#endif /* defined(MT7615) || defined(MT7622) */


#ifdef CUT_THROUGH
INT ShowCutThroughInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    dump_ct_token_list(pAd->PktTokenCb, CUT_THROUGH_TYPE_TX);

    return TRUE;
}
#endif /* CUT_THROUGH */


#ifdef DMA_SCH_SUPPORT
INT32 ShowDMASchInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    UINT32 Value;
    RTMP_IO_READ32(pAd, MT_PAGE_CNT_0, &Value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue0 reservation page thd = 0x%x\n", GET_PAGE_CNT_0(Value)));
    RTMP_IO_READ32(pAd, MT_PAGE_CNT_1, &Value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue1 reservation page thd = 0x%x\n", GET_PAGE_CNT_1(Value)));
    RTMP_IO_READ32(pAd, MT_PAGE_CNT_2, &Value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue2 reservation page thd = 0x%x\n", GET_PAGE_CNT_2(Value)));
    RTMP_IO_READ32(pAd, MT_PAGE_CNT_3, &Value);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue3 reservation page thd = 0x%x\n", GET_PAGE_CNT_3(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_4, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue4 reservation page thd = 0x%x\n", GET_PAGE_CNT_4(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_5, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue5 reservation page thd = 0x%x\n", GET_PAGE_CNT_5(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_6, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue6 reservation page thd = 0x%x\n", GET_PAGE_CNT_6(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_7, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue7 reservation page thd = 0x%x\n", GET_PAGE_CNT_7(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_8, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue8 reservation page thd = 0x%x\n", GET_PAGE_CNT_8(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_9, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue9 reservation page thd = 0x%x\n", GET_PAGE_CNT_9(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_10, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue10 reservation page thd = 0x%x\n", GET_PAGE_CNT_10(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_11, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue11 reservation page thd = 0x%x\n", GET_PAGE_CNT_11(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_12, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue12 reservation page thd = 0x%x\n", GET_PAGE_CNT_12(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_13, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue13 reservation page thd = 0x%x\n", GET_PAGE_CNT_13(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_14, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue14 reservation page thd = 0x%x\n", GET_PAGE_CNT_14(Value)));
	RTMP_IO_READ32(pAd, MT_PAGE_CNT_15, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue15 reservation page thd = 0x%x\n", GET_PAGE_CNT_15(Value)));

	RTMP_IO_READ32(pAd, MT_QUEUE_PRIORITY_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue0 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_0(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue1 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_1(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue2 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_2(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue3 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_3(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue4 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_4(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue5 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_5(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue6 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_6(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue7 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_7(Value)));

	RTMP_IO_READ32(pAd, MT_QUEUE_PRIORITY_2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue8 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_8(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue9 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_9(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue10 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_10(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue11 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_11(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue12 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_12(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue13 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_13(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue14 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_14(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue15 priority = 0x%x\n", GET_RG_QUEUE_PRIORITY_15(Value)));

	RTMP_IO_READ32(pAd, MT_SCH_REG_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Free for all buffer thd = 0x%x\n", GET_RG_FFA_THD(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Packet bytes per page = %d\n", GET_RG_PAGE_SIZE(Value)));

	RTMP_IO_READ32(pAd, MT_SCH_REG_2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Max packet size in one packet = 0x%x\n", GET_RG_MAX_PKT_SIZE(Value)));

	RTMP_IO_READ32(pAd, MT_SCH_REG_4, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Force qid = %d\n", GET_FORCE_QID(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Force mode = %d\n", GET_FORCE_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bypass mode = %d\n", GET_BYPASS_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Hybird mode = %d\n", GET_HYBIRD_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Force in predict mode = %d\n", GET_RG_PREDICT_NO_MASK(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SW mode = %d\n", GET_SW_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rate map(0x%x) when sw mode = 1\n", GET_RG_RATE_MAP(Value)));

	RTMP_IO_READ32(pAd, MT_GROUP_THD_0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 0 page thd = 0x%x\n", GET_GROUP_THD_0(Value)));
	RTMP_IO_READ32(pAd, MT_GROUP_THD_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 1 page thd = 0x%x\n", GET_GROUP_THD_1(Value)));
	RTMP_IO_READ32(pAd, MT_GROUP_THD_2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 2 page thd = 0x%x\n", GET_GROUP_THD_2(Value)));
	RTMP_IO_READ32(pAd, MT_GROUP_THD_3, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 3 page thd = 0x%x\n", GET_GROUP_THD_3(Value)));
	RTMP_IO_READ32(pAd, MT_GROUP_THD_4, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 4 page thd = 0x%x\n", GET_GROUP_THD_4(Value)));
	RTMP_IO_READ32(pAd, MT_GROUP_THD_5, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 5 page thd = 0x%x\n", GET_GROUP_THD_5(Value)));

	RTMP_IO_READ32(pAd, MT_BMAP_0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("One queue on bit mapping(0x%x) for group 0\n", GET_RG_BMAP_0(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("One queue on bit mapping(0x%x) for group 1\n", GET_RG_BMAP_1(Value)));

	RTMP_IO_READ32(pAd, MT_BMAP_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("One queue on bit mapping(0x%x) for group 2\n", GET_RG_BMAP_2(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("One queue on bit mapping(0x%x) for group 3\n", GET_RG_BMAP_3(Value)));

	RTMP_IO_READ32(pAd, MT_BMAP_2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("One queue on bit mapping(0x%x) for group 4\n", GET_RG_BMAP_4(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("One queue on bit mapping(0x%x) for group 5\n", GET_RG_BMAP_5(Value)));

	RTMP_IO_READ32(pAd, MT_HIGH_PRIORITY_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue0 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_0(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue1 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_1(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue2 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_2(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue3 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_3(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue4 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_4(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue5 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_5(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue6 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_6(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue7 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_7(Value)));

	RTMP_IO_READ32(pAd, MT_HIGH_PRIORITY_2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue8 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_8(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue9 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_9(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue10 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_10(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue11 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_11(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue12 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_12(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue13 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_13(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue14 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_14(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue15 high priority = 0x%x\n", GET_RG_HIGH_PRIORITY_15(Value)));

	RTMP_IO_READ32(pAd, MT_PRIORITY_MASK, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Mask for queue priority = 0x%x\n", GET_RG_QUEUE_PRIORITY(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Mask for high priority = 0x%x\n", GET_RG_HIGH_PRIORITY(Value)));

	RTMP_IO_READ32(pAd, MT_RSV_MAX_THD, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FFA1 max thd = 0x%x\n", GET_RG_RSV_MAX_THD(Value)));

	RTMP_IO_READ32(pAd, RSV_AC_CNT_0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 0 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_0(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 1 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_1(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 2 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_2(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_3, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 3 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_3(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_4, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 4 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_4(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_5, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 5 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_5(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_6, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 6 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_6(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_7, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 7 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_7(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_8, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 8 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_8(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_9, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 9 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_9(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_10, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 10 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_10(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_11, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 11 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_11(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_12, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 12 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_12(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_13, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 13 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_13(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_14, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 14 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_14(Value)));
	RTMP_IO_READ32(pAd, RSV_AC_CNT_15, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue 15 rsv paged used count = 0x%x\n", GET_RSV_AC_CNT_15(Value)));

	RTMP_IO_READ32(pAd, SCH_DBG0_0, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Each queue rsv space if enough(0x%x)\n", GET_RSV_ENOUGH(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group remain page great than max pkt page = %d\n", GET_GROUP_ENOUGH(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Queue remain page great than max pkt pagew = %d\n", GET_QUEUE_ENOUGH(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Pedict mode = %d\n", GET_PREDICT_MODE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use queue reservation = %d\n", GET_REST_QUEUE_EN(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Current queue tx op time if enough = %d\n", GET_ENOUGH_TXTIME(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Current queue pse page space if enough = %d\n", GET_ENOUGH_BUF(Value)));

	RTMP_IO_READ32(pAd, SCH_DBG_1, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Free for all buffer used counter = 0x%x\n", GET_FFA_PAGE_CNT(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_2, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Current packet tx op time = 0x%x\n", GET_PKT_TX_TIME(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_3, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx op time period per byte = 0x%x\n", GET_TX_TIME_PER_BYTE(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_4, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PSE poer resrvation latched by dma scheduler = 0x%x\n", GET_PSE_RSV_SPACE(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PSE port reservation space = 0x%x\n", GET_HIF_RSV_PCNT(Value)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PSE poer resrvation space update = %d\n", GET_HIF_RSV_PCNT_UPDT(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_5, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Per-Queue group is enough page space for en-queue = 0x%x\n", GET_GROUP_EN(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_6, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 0 used page count = 0x%x\n", GET_USED_GROUP_0(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_7, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 1 used page count = 0x%x\n", GET_USED_GROUP_1(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_8, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 2 used page count = 0x%x\n", GET_USED_GROUP_2(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_9, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 3 used page count = 0x%x\n", GET_USED_GROUP_3(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_10, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 4 used page count = 0x%x\n", GET_USED_GROUP_4(Value)));
	RTMP_IO_READ32(pAd, SCH_DBG_11, &Value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group 5 used page count = 0x%x\n", GET_USED_GROUP_5(Value)));

	return TRUE;
}
#endif
#endif /*MT_MAC*/


INT Show_sta_tr_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT idx;
	STA_TR_ENTRY *tr_entry;

	for (idx = 0; idx < MAX_LEN_OF_TR_TABLE; idx++)
	{
		tr_entry = &pAd->MacTab.tr_entry[idx];
		if (IS_VALID_ENTRY(tr_entry))
			TRTableEntryDump(pAd, idx, __FUNCTION__, __LINE__);
	}

	return TRUE;
}


INT show_stainfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	ULONG DataRate=0, irqflags;
	UCHAR mac_addr[MAC_ADDR_LEN];
	RTMP_STRING *token;
	CHAR sep[1] = {':'};
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Input string=%s\n",
				__FUNCTION__, arg));
	for (i = 0, token = rstrtok(arg, &sep[0]); token; token = rstrtok(NULL, &sep[0]), i++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): token(len=%zu) =%s\n",
					__FUNCTION__, strlen(token), token));
		if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
			return FALSE;
		AtoH(token, (&mac_addr[i]), 1);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): i= %d\n", __FUNCTION__, i));
	if(i != 6)
		return FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nAddr %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(mac_addr)));

#ifdef CONFIG_AP_SUPPORT
	pEntry = MacTableLookup(pAd, (UCHAR *)mac_addr);
#endif

    	if (!pEntry)
		return FALSE;

	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Invalid MAC address!\n"));
		return FALSE;
	}

	printk("\n");

	printk("EntryType : %d\n", pEntry->EntryType);
	printk("Entry Capability:\n");
	printk("\tPhyMode:%-10s\n", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE));
	printk("\tBW:%-6s\n", get_bw_str(pEntry->MaxHTPhyMode.field.BW));
	printk("\tDataRate: \n");
#ifdef DOT11_VHT_AC
	if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT)
		printk("%dS-M%d", ((pEntry->MaxHTPhyMode.field.MCS>>4) + 1), (pEntry->MaxHTPhyMode.field.MCS & 0xf));
	else
#endif /* DOT11_VHT_AC */
	printk(" %-6d", pEntry->MaxHTPhyMode.field.MCS);
	printk(" %-6d", pEntry->MaxHTPhyMode.field.ShortGI);
	printk(" %-6d\n", pEntry->MaxHTPhyMode.field.STBC);

	printk("Entry Operation Features\n");
	printk("\t%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1",
		   "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");

	DataRate=0;
	getRate(pEntry->HTPhyMode, &DataRate);
	printk("\t%-4d", (int)pEntry->Aid);
	printk("%-4d", (int)pEntry->func_tb_idx);
	printk("%-4d", (int)pEntry->PsMode);
	printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
	printk("%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
	printk("%-7d", pEntry->RssiSample.AvgRssi[0]);
	printk("%-7d", pEntry->RssiSample.AvgRssi[1]);
	printk("%-7d", pEntry->RssiSample.AvgRssi[2]);
	printk("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
	printk("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC
	if (pEntry->HTPhyMode.field.MODE == MODE_VHT)
		printk("%dS-M%d", ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
	else
#endif /* DOT11_VHT_AC */
		printk("%-6d", pEntry->HTPhyMode.field.MCS);
	printk("%-6d", pEntry->HTPhyMode.field.ShortGI);
	printk("%-6d", pEntry->HTPhyMode.field.STBC);
	printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
	printk("%-7d", (int)DataRate);
	printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
				(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);

	printk("\n");

	ASSERT(pEntry->wcid <= GET_MAX_UCAST_NUM(pAd));
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	printk("Entry TxRx Info\n");
	printk("\tEntryType : %d\n", tr_entry->EntryType);
	printk("\tHookingWdev : %p\n", tr_entry->wdev);
	printk("\tIndexing : FuncTd=%d, WCID=%d\n", tr_entry->func_tb_idx, tr_entry->wcid);
	printk("Entry TxRx Features\n");
	printk("\tIsCached, PortSecured, PsMode, LockTx, VndAth\n");
	printk("\t%d\t%d\t%d\t%d\t%d\n", tr_entry->isCached, tr_entry->PortSecured,
				tr_entry->PsMode, tr_entry->LockEntryTx,
				tr_entry->bIAmBadAtheros);

	printk("\t%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n", "TxQId", "PktNum", "QHead", "QTail", "EnQCap", "DeQCap", "PktSeq");
	for (i = 0; i < WMM_QUE_NUM;  i++){
		RTMP_IRQ_LOCK(&pAd->irq_lock /* &tr_entry->txq_lock[i] */, irqflags);
		printk("\t%d %6d  %p  %6p %d %d %d\n",
				i,
				tr_entry->tx_queue[i].Number,
				tr_entry->tx_queue[i].Head,
				tr_entry->tx_queue[i].Tail,
				tr_entry->enq_cap, tr_entry->deq_cap,
				tr_entry->TxSeq[i]);
		RTMP_IRQ_UNLOCK(&pAd->irq_lock /* &tr_entry->txq_lock[i] */, irqflags);
	}
	RTMP_IRQ_LOCK(&pAd->irq_lock /* &tr_entry->ps_queue_lock */, irqflags);
	printk("\tpsQ %6d  %p  %p %d %d  NoQ:%d\n",
				tr_entry->ps_queue.Number,
				tr_entry->ps_queue.Head,
				tr_entry->ps_queue.Tail,
				tr_entry->enq_cap, tr_entry->deq_cap,
				tr_entry->NonQosDataSeq);
	RTMP_IRQ_UNLOCK(&pAd->irq_lock /* &tr_entry->ps_queue_lock */, irqflags);

	printk("\n");
	return TRUE;
}


static VOID show_devinfo_byRf(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	UCHAR PhyMode,Channel;
	BOOLEAN IsRfRun = HcIsRfRun(pAd,RfIC);
	UCHAR *pstr;
#ifdef DOT11_VHT_AC
	UCHAR *vht_bw[] = {"20/40", "80", "160", "80+80"};
#endif /* DOT11_VHT_AC */
	UCHAR bw;

	if(IsRfRun)
	{
		PhyMode = HcGetPhyModeByRf(pAd,RfIC);
		Channel = HcGetChannelByRf(pAd,RfIC);
		bw= HcGetBwByRf(pAd,RfIC);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band: %d\n",RfIC));
		pstr = wmode_2_str(PhyMode);
		if(pstr) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWirelessMode: %s(%d)\n", pstr, PhyMode));
			os_free_mem(pstr);
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannel: %d\n", Channel));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tCentralChannel: %d\n", HcGetCentralChByRf(pAd,RfIC)));

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tVHT CentralChannel1: %d\n", pAd->CommonCfg.vht_cent_ch));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tVHT CentralChannel2: %d\n", pAd->CommonCfg.vht_cent_ch2));
		}
#endif /* DOT11_VHT_AC */
		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRF Channel: %d\n",  HcGetChannelByRf(pAd,RfIC)));
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRF Channel: %d\n", pAd->LatchRfRegs.Channel));
		}
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBandwidth\n"));
		pstr = (bw) ? "20/40" : "20";
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tHT-BW: %s\n", pstr));
#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode))
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tVHT-BW: %s\n", vht_bw[pAd->CommonCfg.vht_bw]));
		}
#endif /* DOT11_VHT_AC */
	}
}

INT show_devinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR *pstr;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Device MAC\n"));
	if (pAd->OpMode == OPMODE_AP)
		pstr = "AP";
	else if (pAd->OpMode == OPMODE_STA)
		pstr = "STA";
	else
		pstr = "Unknown";

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Operation Mode: %s\n", pstr));

	show_devinfo_byRf(pAd,RFIC_24GHZ);
	show_devinfo_byRf(pAd,RFIC_5GHZ);
#if defined(RTMP_MAC) || defined(RLT_MAC)
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type != HIF_MT) {
		UINT32 mac_val;

		RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &mac_val);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BackOff Slot      : %s slot time, BKOFF_SLOT_CFG(0x1104) = 0x%08x\n",
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED) ? "short" : "long",
 					mac_val));
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Security\n"));

	return TRUE;
}


INT show_wdev_info(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;

	for (idx = 0 ; idx < WDEV_NUM_MAX; idx++)
	{
		if (pAd->wdev_list[idx] == wdev) {
			break;
		}
	}

	if (idx >= WDEV_NUM_MAX)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERR! Cannot found required wdev(%p)!\n", wdev));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WDEV Instance(%d) Info:\n", idx));

	return TRUE;
}

CHAR *wdev_type_str[]={"AP", "STA", "ADHOC", "WDS", "MESH", "GO", "GC", "APCLI", "REPEATER", "P2P_DEVICE", "Unknown"};

RTMP_STRING *wdev_type2str(int type)
{
	switch (type)
	{
		case WDEV_TYPE_AP:
			return wdev_type_str[0];
		case WDEV_TYPE_STA:
			return wdev_type_str[1];
		case WDEV_TYPE_ADHOC:
			return wdev_type_str[2];
		case WDEV_TYPE_WDS:
			return wdev_type_str[3];
		case WDEV_TYPE_MESH:
			return wdev_type_str[4];
		case WDEV_TYPE_GO:
			return wdev_type_str[5];
		case WDEV_TYPE_GC:
			return wdev_type_str[6];
		case WDEV_TYPE_APCLI:
			return wdev_type_str[7];
		case WDEV_TYPE_REPEATER:
			return wdev_type_str[8];
		case WDEV_TYPE_P2P_DEVICE:
			return wdev_type_str[9];
		default:
			return wdev_type_str[10];
	}
}


INT show_sysinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT idx;
	UINT32 total_size = 0, cntr_size;
	struct wifi_dev *wdev;
	UCHAR ext_cha;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Device Instance\n"));
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWDEV %02d:", idx));
		if (pAd->wdev_list[idx])
		{
			UCHAR *str = NULL;

			wdev = pAd->wdev_list[idx];
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\t\tName/Type:%s/%s\n",
						RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev),
						wdev_type2str(wdev->wdev_type)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tWdev(list) Idx:%d\n", wdev->wdev_idx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMacAddr:%02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(wdev->if_addr)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBSSID:%02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(wdev->bssid)));
			str = wmode_2_str(wdev->PhyMode);
			if (str) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPhyMode:%s\n", str));
				os_free_mem(str);
			}
			ext_cha = wlan_config_get_ext_cha(wdev);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tChannel:%d,ExtCha:%d\n", wdev->channel,ext_cha));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPortSecured/ForbidTx: %d(%sSecured)/%lx\n",
						wdev->PortSecured,
						(wdev->PortSecured == WPA_802_1X_PORT_SECURED ? "" : "Not"),
						wdev->forbid_data_tx));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tEdcaIdx:%d\n", wdev->EdcaIdx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_pkt_checker:%p\n", wdev->tx_pkt_allowed));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_pkt_handlerer:%p\n", wdev->tx_pkt_handle));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\ttx_pkt_hardTransmit:%p\n", wdev->wdev_hard_tx));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tif_dev:0x%p\tfunc_dev:[%d]0x%p\tsys_handle:0x%p\n",
						wdev->if_dev, wdev->func_idx, wdev->func_dev, wdev->sys_handle));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tIgmpSnoopEnable:%d\n", wdev->IgmpSnoopEnable));
#ifdef LINUX
			if (wdev->if_dev)
			{
				UINT idx, q_num;
				UCHAR *mac_str = RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
						("\t\tOS NetDev status(%s[%d]-%02x:%02x:%02x:%02x:%02x:%02x):\n", 
						RtmpOsGetNetDevName(wdev->if_dev),
						RtmpOsGetNetIfIndex(wdev->if_dev),
						PRINT_MAC(mac_str)));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tdev->state: 0x%lx\n", RtmpOSGetNetDevState(wdev->if_dev)));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tdev->flag: 0x%x\n", RtmpOSGetNetDevFlag(wdev->if_dev)));
				q_num = RtmpOSGetNetDevQNum(wdev->if_dev);
				for (idx = 0; idx < q_num; idx++) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
							("\t\t\tdev->queue[%d].state: 0x%lx\n",idx,
							RtmpOSGetNetDevQState(wdev->if_dev, idx)));
				}
				
			}
#endif /* LINUX */
		}
		else
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}

	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Memory Statistics:\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsize>\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tpAd = \t\t%zu bytes\n\n", sizeof(*pAd)));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tCommonCfg = \t%zu bytes\n", sizeof(pAd->CommonCfg)));
	total_size += sizeof(pAd->CommonCfg);
#ifdef CONFIG_AP_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tApCfg = \t%zu bytes\n", sizeof(pAd->ApCfg)));
	total_size += sizeof(pAd->ApCfg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\t\tMBSSID = \t%zu B (PerMBSS =%zu B, Total MBSS Num= %d)\n",
				sizeof(pAd->ApCfg.MBSSID), sizeof(struct _BSS_STRUCT), HW_BEACON_MAX_NUM));
#ifdef APCLI_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\t\t\tAPCLI = \t%zu bytes (PerAPCLI =%zu bytes, Total APCLI Num= %d)\n",
				sizeof(pAd->ApCfg.ApCliTab), sizeof(struct _APCLI_STRUCT), MAX_APCLI_NUM));
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef RTMP_MAC_PCI
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tTxRing = \t%zu bytes\n", sizeof(pAd->PciHif.TxRing)));
	total_size += sizeof(pAd->PciHif.TxRing);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tRxRing = \t%zu bytes\n", sizeof(pAd->PciHif.RxRing)));
	total_size += sizeof(pAd->PciHif.RxRing);
#ifdef CONFIG_ANDES_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tCtrlRing = \t%zu bytes\n", sizeof(pAd->CtrlRing)));
	total_size += sizeof(pAd->CtrlRing);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tBcnRing = \t%zu bytes\n", sizeof(pAd->BcnRing)));
	total_size += sizeof(pAd->BcnRing);
#endif /* MT_MAC */
#endif /* RTMP_MAC_PCI */

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMgmtRing = \t%zu bytes\n", sizeof(pAd->MgmtRing)));
	total_size += sizeof(pAd->MgmtRing);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMlme = \t%zu bytes\n", sizeof(pAd->Mlme)));
	total_size += sizeof(pAd->Mlme);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tMacTab = \t%zu bytes\n", sizeof(pAd->MacTab)));
	total_size += sizeof(pAd->MacTab);

#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tBATable = \t%zu bytes\n", sizeof(pAd->BATable)));
	total_size += sizeof(pAd->BATable);
#endif /* DOT11_N_SUPPORT */

	cntr_size = sizeof(pAd->Counters8023) + sizeof(pAd->WlanCounters) +
			sizeof(pAd->RalinkCounters) + /* sizeof(pAd->DrsCounters) */+
			sizeof(pAd->PrivateInfo);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tCounter** = \t%d bytes\n", cntr_size));
	total_size += cntr_size;

#if defined (AP_SCAN_SUPPORT) || defined (CONFIG_STA_SUPPORT)
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t\tScanTab = \t%zu bytes\n", sizeof(pAd->ScanTab)));
	total_size += sizeof(pAd->ScanTab);
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tsize> Total = \t\t%d bytes, Others = %zu bytes\n\n",
			total_size, sizeof(*pAd)-total_size));

	return TRUE;
}

void wifi_dump_info(void)
{
	RTMP_ADAPTER *pAd = NULL;
	struct net_device *ndev = NULL;
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s--------------------\n", __FUNCTION__));

	ndev = dev_get_by_name(&init_net, "ra0");
	
	if (ndev) { 
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------RA0--------\n"));
	
		pAd = ((struct mt_dev_priv *)netdev_priv(ndev))->sys_handle;
		show_tpinfo_proc(pAd, "");
		show_trinfo_proc(pAd, "");
		ShowPLEInfo(pAd, "");
		Show_PSTable_Proc(pAd, "");
	}

	ndev = dev_get_by_name(&init_net, "rai0");
	
	if (ndev) { 
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------RAI0--------\n"));

		pAd = ((struct mt_dev_priv *)netdev_priv(ndev))->sys_handle;
		show_tpinfo_proc(pAd, "");
		show_trinfo_proc(pAd, "");
		ShowPLEInfo(pAd, "");
		Show_PSTable_Proc(pAd, "");
	}

	ndev = dev_get_by_name(&init_net, "rae0");
	
	if (ndev) { 
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------RAE0--------\n"));

		pAd = ((struct mt_dev_priv *)netdev_priv(ndev))->sys_handle;
		show_tpinfo_proc(pAd, "");
		show_trinfo_proc(pAd, "");
		ShowPLEInfo(pAd, "");
		Show_PSTable_Proc(pAd, "");
	}
}
EXPORT_SYMBOL(wifi_dump_info);

INT show_tpinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CUT_THROUGH
    PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)(pAd->PktTokenCb);
#ifdef CUT_THROUGH_DBG
	UINT8 SlotIndex;
#endif
#endif
	struct rx_delay_control *rx_delay_ctl = &pAd->tr_ctl.rx_delay_ctl;

#ifdef CUT_THROUGH
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxFreeToken Configuration\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxFreeToken Number = %d\n", pktTokenCb->tx_id_list.list->FreeTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxFreeToken LowMark = %d\n", pktTokenCb->TxTokenLowWaterMark));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxFreeToken HighMark = %d\n", pktTokenCb->TxTokenHighWaterMark));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTotalTxUsedToken Number = %d\n", pktTokenCb->tx_id_list.list->TotalTxUsedTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTotalTxBackToken Number = %d\n", pktTokenCb->tx_id_list.list->TotalTxBackTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTotalTxTokenEvent Number = %d\n", pktTokenCb->tx_id_list.list->TotalTxTokenEventCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTotalTxToken(From CR4) Number = %d\n", pktTokenCb->tx_id_list.list->TotalTxTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFPTxElementFullNum = %d\n", pAd->FPTxElementFullNum));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFPTxElementFreeNum = %d\n", pAd->FPTxElementFreeNum));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMinFPTxElementFreeNum = %d\n", pAd->MinFPTxElementFreeNum));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWrong Wlan Index Num = %d\n", pAd->wrong_wlan_idx_num));
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxFreeToken Usage\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTimeSlot \tUsedTokenCnt \tBackTokenCnt \tAgg0_31 \tAgg32_63 \tAgg64_95 \tAgg96_127\n"));

	for (SlotIndex = 0; SlotIndex < TIME_SLOT_NUMS; SlotIndex++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d\n", SlotIndex,
				pktTokenCb->tx_id_list.list->UsedTokenCntRec[SlotIndex],
				pktTokenCb->tx_id_list.list->BackTokenCntRec[SlotIndex],
				pktTokenCb->tx_id_list.list->FreeAgg0_31Rec[SlotIndex],
				pktTokenCb->tx_id_list.list->FreeAgg32_63Rec[SlotIndex],
				pktTokenCb->tx_id_list.list->FreeAgg64_95Rec[SlotIndex],
				pktTokenCb->tx_id_list.list->FreeAgg96_127Rec[SlotIndex]));
	}

#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxFreeToken Configuration\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxFreeToken Number = %d\n", pktTokenCb->rx_id_list.list->FreeTokenCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxFreeToken LowMark = %d\n", pktTokenCb->RxTokenLowWaterMark));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxFreeToken HighMark = %d\n", pktTokenCb->RxTokenHighWaterMark));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxRingFull Count = %d\n", pktTokenCb->TxRingFullCnt));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxTokenFull Count = %d\n", pktTokenCb->TxTokenFullCnt));

#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxFreeToken Usage\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTimeSlot \tUsedTokenCnt \tBackTokenCnt \tAgg0_31 \tAgg32_63 \tAgg64_95 \tAgg96_127 \tDropCnt\n"));

	for (SlotIndex = 0; SlotIndex < TIME_SLOT_NUMS; SlotIndex++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d\n", SlotIndex,
				pktTokenCb->rx_id_list.list->UsedTokenCntRec[SlotIndex],
				pktTokenCb->rx_id_list.list->BackTokenCntRec[SlotIndex],
				pktTokenCb->rx_id_list.list->FreeAgg0_31Rec[SlotIndex],
				pktTokenCb->rx_id_list.list->FreeAgg32_63Rec[SlotIndex],
				pktTokenCb->rx_id_list.list->FreeAgg64_95Rec[SlotIndex],
				pktTokenCb->rx_id_list.list->FreeAgg96_127Rec[SlotIndex],
				pktTokenCb->rx_id_list.list->DropPktCntRec[SlotIndex]));
	}


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRxDropPacket Count = %d\n", pAd->RxDropPacket));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\tTimeSlot \tTxIsr \t\tRxIsr \t\tRx1Isr \t\tTxIoRead \tTxIoWrite \tRxIoRead \tRxIoWrite \tRx1IoRead \tRx1IoWrite\n"));

	for (SlotIndex = 0; SlotIndex < TIME_SLOT_NUMS; SlotIndex++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d\n", SlotIndex, pAd->IsrTxCntRec[SlotIndex], pAd->IsrRxCntRec[SlotIndex], pAd->IsrRx1CntRec[SlotIndex],
				pAd->IoReadTxRec[SlotIndex],
				pAd->IoWriteTxRec[SlotIndex],
				pAd->IoReadRxRec[SlotIndex],
				pAd->IoWriteRxRec[SlotIndex],
				pAd->IoReadRx1Rec[SlotIndex],
				pAd->IoWriteRx1Rec[SlotIndex]));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\tTimeSlot \tRx0Cnt1_64 \tRx0Cnt65_128 \tRx0Cnt129_192 \tRx0Cnt193_256 \tRx1Cnt1_64 \tRx1Cnt65_128 \tRx1Cnt129_192 \tRx1Cnt193_256\n"));

	for (SlotIndex = 0; SlotIndex < TIME_SLOT_NUMS; SlotIndex++)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d \t\t%d\n", SlotIndex, pAd->MaxProcessCntRxRecA[SlotIndex],
		pAd->MaxProcessCntRxRecB[SlotIndex],
		pAd->MaxProcessCntRxRecC[SlotIndex],
		pAd->MaxProcessCntRxRecD[SlotIndex],
		pAd->MaxProcessCntRx1RecA[SlotIndex],
		pAd->MaxProcessCntRx1RecB[SlotIndex],
		pAd->MaxProcessCntRx1RecC[SlotIndex],
		pAd->MaxProcessCntRx1RecD[SlotIndex]));
	}

#endif
#endif

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rx_delay_en = %d\n", rx_delay_ctl->en));

	return TRUE;
}

INT show_trinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	if (IS_RBUS_INF(pAd) || IS_PCI_INF(pAd))
	{
		ULONG irq_flags;
		UINT32 tbase[NUM_OF_TX_RING], tcnt[NUM_OF_TX_RING];
		UINT32 tcidx[NUM_OF_TX_RING], tdidx[NUM_OF_TX_RING];
		UINT32 rbase[NUM_OF_RX_RING], rcnt[NUM_OF_RX_RING];
		UINT32 rcidx[NUM_OF_RX_RING], rdidx[NUM_OF_RX_RING];
		UINT32 mbase[4] = {0}, mcnt[4] = {0}, mcidx[4] = {0}, mdidx[4] = {0};
		UINT32 sys_ctrl[4];
		UINT32 cr_int_src, cr_int_mask, cr_delay_int, cr_wpdma_glo_cfg;
#ifdef RLT_MAC
		UINT32 pbf_val, pcnt[5];
#endif /* RLT_MAC */
		INT idx;
		INT TxHwRingNum = NUM_OF_TX_RING;
		INT RxHwRingNum = NUM_OF_RX_RING;

#ifdef ERR_RECOVERY
    if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
        return TRUE;
    }
#endif /* ERR_RECOVERY */

#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
			TxHwRingNum = 2;
#endif /* defined(MT7615) || defined(MT7622) */

		RTMP_IRQ_LOCK(&pAd->irq_lock, irq_flags);
		for (idx = 0; idx < TxHwRingNum; idx++)
		{
			HIF_IO_READ32(pAd,pAd->PciHif.TxRing[idx].hw_desc_base,&tbase[idx]);
			HIF_IO_READ32(pAd,pAd->PciHif.TxRing[idx].hw_cnt_addr,&tcnt[idx]);
			HIF_IO_READ32(pAd,pAd->PciHif.TxRing[idx].hw_cidx_addr,&tcidx[idx]);
			HIF_IO_READ32(pAd,pAd->PciHif.TxRing[idx].hw_didx_addr,&tdidx[idx]);
		}

#ifdef CONFIG_ANDES_SUPPORT
			HIF_IO_READ32(pAd, pAd->CtrlRing.hw_desc_base, &mbase[1]);
			HIF_IO_READ32(pAd, pAd->CtrlRing.hw_cnt_addr, &mcnt[1]);
			HIF_IO_READ32(pAd, pAd->CtrlRing.hw_cidx_addr, &mcidx[1]);
			HIF_IO_READ32(pAd, pAd->CtrlRing.hw_didx_addr, &mdidx[1]);
#endif /* CONFIG_ANDES_SUPPORT */

#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			HIF_IO_READ32(pAd, pAd->FwDwloRing.hw_desc_base, &mbase[2]);
			HIF_IO_READ32(pAd, pAd->FwDwloRing.hw_cnt_addr, &mcnt[2]);
			HIF_IO_READ32(pAd, pAd->FwDwloRing.hw_cidx_addr, &mcidx[2]);
			HIF_IO_READ32(pAd, pAd->FwDwloRing.hw_didx_addr, &mdidx[2]);
		}
		else
#endif /* defined(MT7615) || defined(MT7622) */
		{
			HIF_IO_READ32(pAd, pAd->MgmtRing.hw_desc_base, &mbase[0]);
			HIF_IO_READ32(pAd, pAd->MgmtRing.hw_cnt_addr, &mcnt[0]);
			HIF_IO_READ32(pAd, pAd->MgmtRing.hw_cidx_addr, &mcidx[0]);
			HIF_IO_READ32(pAd, pAd->MgmtRing.hw_didx_addr, &mdidx[0]);

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
			HIF_IO_READ32(pAd, pAd->BcnRing.hw_desc_base, &mbase[2]);
			HIF_IO_READ32(pAd, pAd->BcnRing.hw_cnt_addr, &mcnt[2]);
			HIF_IO_READ32(pAd, pAd->BcnRing.hw_cidx_addr, &mcidx[2]);
			HIF_IO_READ32(pAd, pAd->BcnRing.hw_didx_addr, &mdidx[2]);

			HIF_IO_READ32(pAd, pAd->TxBmcRing.hw_desc_base, &mbase[3]);
			HIF_IO_READ32(pAd, pAd->TxBmcRing.hw_cnt_addr, &mcnt[3]);
			HIF_IO_READ32(pAd, pAd->TxBmcRing.hw_cidx_addr, &mcidx[3]);
			HIF_IO_READ32(pAd, pAd->TxBmcRing.hw_didx_addr, &mdidx[3]);
		}
#endif /* MT_MAC */
		}

		for (idx = 0; idx < RxHwRingNum; idx++)
		{
			HIF_IO_READ32(pAd, pAd->PciHif.RxRing[idx].hw_desc_base, &rbase[idx]);
			HIF_IO_READ32(pAd, pAd->PciHif.RxRing[idx].hw_cnt_addr, &rcnt[idx]);
			HIF_IO_READ32(pAd, pAd->PciHif.RxRing[idx].hw_cidx_addr, &rcidx[idx]);
			HIF_IO_READ32(pAd, pAd->PciHif.RxRing[idx].hw_didx_addr, &rdidx[idx]);
		}

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			RTMP_IO_READ32(pAd, RLT_PBF_CFG, &pbf_val);
			RTMP_IO_READ32(pAd, TX_MAX_PCNT, &pcnt[0]);
			RTMP_IO_READ32(pAd, RX_MAX_PCNT, &pcnt[1]);
			RTMP_IO_READ32(pAd, TXQ_STA, &pcnt[2]);
			RTMP_IO_READ32(pAd, RXQ_STA, &pcnt[3]);
			RTMP_IO_READ32(pAd, TXRXQ_PCNT, &pcnt[4]);
			RTMP_IO_READ32(pAd, PBF_DBG, &pcnt[5]);
		}
#endif /* RLT_MAC */

		cr_int_src = cr_int_mask = cr_wpdma_glo_cfg = cr_delay_int = 0;
#ifdef MT_MAC
	// TODO: shiang-7603
		if (pAd->chipCap.hif_type == HIF_MT) {
			cr_int_src = MT_INT_SOURCE_CSR;
			cr_int_mask = MT_INT_MASK_CSR;
			cr_delay_int = MT_DELAY_INT_CFG;
			cr_wpdma_glo_cfg = MT_WPDMA_GLO_CFG;
		}
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RLT || pAd->chipCap.hif_type == HIF_RTMP) {
			cr_int_src = INT_SOURCE_CSR;
			cr_int_mask = INT_MASK_CSR;
			cr_delay_int = DELAY_INT_CFG;
			cr_wpdma_glo_cfg = WPDMA_GLO_CFG;
		}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
		HIF_IO_READ32(pAd,cr_int_src,&sys_ctrl[0]);
		HIF_IO_READ32(pAd,cr_int_mask,&sys_ctrl[1]);

		HIF_IO_READ32(pAd, cr_delay_int, &sys_ctrl[2]);
		HIF_IO_READ32(pAd, cr_wpdma_glo_cfg, &sys_ctrl[3]);

		RTMP_IRQ_UNLOCK(&pAd->irq_lock, irq_flags);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxRing Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
		for (idx = 0; idx < TxHwRingNum; idx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
						idx, pAd->PciHif.TxRing[idx].hw_desc_base, tbase[idx], tcnt[idx], tcidx[idx], tdidx[idx]));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx \tTx Free TxD resource\n"));
		for (idx = 0; idx < TxHwRingNum; idx++)
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t%d\n",
				idx, GET_TXRING_FREENO(pAd, idx)));

		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRxRing Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
		for (idx = 0; idx < RxHwRingNum; idx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
						idx, pAd->PciHif.RxRing[idx].hw_desc_base, rbase[idx], rcnt[idx], rcidx[idx], rdidx[idx]));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nMgmtRing Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
									0, pAd->MgmtRing.hw_desc_base, mbase[0], mcnt[0], mcidx[0], mdidx[0]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx \tRx Pending RX Packet\n"));
		for (idx = 0; idx < RxHwRingNum; idx++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t\t%d\n",
				idx, GET_RXRING_PENDINGNO(pAd, idx)));
		}
#ifdef CONFIG_ANDES_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nCtrlRing Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
									0, pAd->CtrlRing.hw_desc_base, mbase[1], mcnt[1], mcidx[1], mdidx[1]));
#endif /* CONFIG_ANDES_SUPPORT */

#if defined(MT7615) || defined(MT7622)
		if (IS_MT7615(pAd) || IS_MT7622(pAd))
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nFwDwloadRing Configuration\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
						0, pAd->FwDwloRing.hw_desc_base, mbase[2], mcnt[2], mcidx[2], mdidx[2]));
		}
		else
#endif /* defined(MT7615) || defined(MT7622) */
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\nMgmtRing Configuration\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
						0, pAd->MgmtRing.hw_desc_base, mbase[0], mcnt[0], mcidx[0], mdidx[0]));

#ifdef MT_MAC
			if (pAd->chipCap.hif_type == HIF_MT)
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\nBcnRing Configuration\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
									0, pAd->BcnRing.hw_desc_base, mbase[2], mcnt[2], mcidx[2], mdidx[2]));

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\nBMCRing Configuration\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\tRingIdx Reg \tBase \t\tCnt \tCIDX \tDIDX\n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\t%d \t0x%04x \t0x%08x \t0x%x \t0x%x \t0x%x\n",
								0, pAd->TxBmcRing.hw_desc_base, mbase[3], mcnt[3], mcidx[3], mdidx[3]));
		}
#endif /* MT_MAC */
		}

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nPBF Configuration\n"
									"\tRLT_PBF_CFG: 0x%08x\n", pbf_val));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tPCNT_CFG:\n"
									"\t\tTxMax[0x%04x] -0x%08x\n", TX_MAX_PCNT, pcnt[0]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxStat[0x%04x] -0x%08x\n", RX_MAX_PCNT, pcnt[2]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxUsed[0x%04x] -0x%08x\n", TXRXQ_PCNT, pcnt[4]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxMax[0x%04x] -0x%08x\n", RX_MAX_PCNT, pcnt[1]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tRxStat[0x%04x] -0x%08x\n", RXQ_STA, pcnt[3]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tFreCNT[0x%04x] -0x%08x\n", PBF_DBG, pcnt[5]));
		}
#endif /* RLT_MAC */

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Interrupt Configuration\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tIntCSR \tIntMask \tDelayINT\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t0x%x \t0x%x \t0x%x\n", sys_ctrl[0], sys_ctrl[1], sys_ctrl[2]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tIntPending \tintDisableMask\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t0x%x \t0x%x\n", pAd->PciHif.IntPending, 
									pAd->PciHif.intDisableMask));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DMA Configuration(0x%x)\n", sys_ctrl[3]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTx/RxDMAEn=%d/%d, \tTx/RxDMABusy=%d/%d\n",
									sys_ctrl[3] & 0x1, sys_ctrl[3] & 0x4,
									sys_ctrl[3] & 0x2, sys_ctrl[3] & 0x8));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */


#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
#ifdef RTMP_PCI_SUPPORT
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PDMA Info\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\tPDMAMonitorEn=%d, TxRCntr = %ld, TxDMACheckTimes = %d,  RxRCounter = %ld, RxDMACheckTimes = %d, PDMARFailCount = %ld\n",
						pAd->PDMAWatchDogEn, pAd->TxDMAResetCount,
						pAd->TxDMACheckTimes, pAd->RxDMAResetCount,
						pAd->RxDMACheckTimes, pAd->PDMAResetFailCount));
#endif
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PSE Info\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("\tPSEMonitorEn=%d, RCounter = %lu, RxPseCheckTimes = %d, PSETriggerType1Count = %lu, PSETriggerType2Count = %lu, PSERFailCount = %lu\n",
						pAd->PSEWatchDogEn, pAd->PSEResetCount,
						pAd->RxPseCheckTimes, pAd->PSETriggerType1Count,
						pAd->PSETriggerType2Count, pAd->PSEResetFailCount));
		}
#endif

#ifdef INT_STATISTIC

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("INT_CNT=%lu INT_TxCoherent_CNT=%lu INT_RxCoherent_CNT=%lu INT_FifoStaFullInt_CNT=%lu INT_MGMTDLY_CNT=%lu INT_RXDATA_CNT=%lu pAd->INT_RXCMD_CNT=%lu INT_HCCA_CNT=%lu INT_AC3_CNT=%lu INT_AC2_CNT=%lu INT_AC1_CNT=%lu INT_AC0_CNT=%lu INT_PreTBTT_CNT=%lu INT_TBTTInt_CNT=%lu INT_GPTimeOut_CNT=%lu INT_Radar_CNT=%lu \n",
	pAd->INTCNT,
	pAd->INTTxCoherentCNT,
	pAd->INTRxCoherentCNT,
	pAd->INTFifoStaFullIntCNT,
	pAd->INTMGMTDLYCNT,
	pAd->INTRXDATACNT,
	pAd->INTRXCMDCNT,
	pAd->INTHCCACNT,
	pAd->INTAC3CNT,
	pAd->INTAC2CNT,
	pAd->INTAC1CNT,
	pAd->INTAC0CNT,
	pAd->INTPreTBTTCNT,
	pAd->INTTBTTIntCNT,
	pAd->INTGPTimeOutCNT,
	pAd->INTRadarCNT));

#ifdef MT_MAC
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("INTWFMACINT0CNT=%lu INTWFMACINT1CNT=%lu INTWFMACINT2CNT=%lu INTWFMACINT3CNT=%lu INTWFMACINT4CNT=%lu INTBCNDLY=%lu INTBMCDLY=%lu\n",
	pAd->INTWFMACINT0CNT,
	pAd->INTWFMACINT1CNT,
	pAd->INTWFMACINT2CNT,
	pAd->INTWFMACINT3CNT,
	pAd->INTWFMACINT4CNT,
	pAd->INTBCNDLY,
	pAd->INTBMCDLY));
#endif

#endif

	return TRUE;
}


INT show_txqinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT qidx;
	unsigned long irqflag;
	UCHAR *TxSwQ = NULL;
	struct tx_swq_fifo *fifo_swq;

	os_alloc_mem(NULL, &TxSwQ, 512 * 4);
	if (TxSwQ == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():mem alloc failed!\n", __FUNCTION__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Dump TxQ Info\n", __FUNCTION__));
	for (qidx = 0; qidx < 4; qidx++)
	{
		fifo_swq = &pAd->tx_swq[qidx];

		RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
		NdisMoveMemory(&TxSwQ[qidx * 512], fifo_swq->swq, 512);
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);
	}

	os_free_mem(TxSwQ);

	rtmp_tx_swq_dump(pAd, 0);
	rtmp_sta_txq_dump(pAd, &pAd->MacTab.tr_entry[1], 0);

	return TRUE;
}


#ifdef WSC_STA_SUPPORT
INT	Show_WpsManufacturer_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
IN ULONG			BufLen)
{
    POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
    PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	sprintf(pBuf, "\tManufacturer = %s", pStaCfg->WscControl.RegData.SelfInfo.Manufacturer);
	return 0;
}

INT	Show_WpsModelName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
    POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
    PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
    
	sprintf(pBuf, "\tModelName = %s", pStaCfg->WscControl.RegData.SelfInfo.ModelName);
	return 0;
}

INT	Show_WpsDeviceName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
    POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
    PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
    
	sprintf(pBuf, "\tDeviceName = %s", pStaCfg->WscControl.RegData.SelfInfo.DeviceName);
	return 0;
}

INT	Show_WpsModelNumber_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
    POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
    PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
    
	sprintf(pBuf, "\tModelNumber = %s", pStaCfg->WscControl.RegData.SelfInfo.ModelNumber);
	return 0;
}

INT	Show_WpsSerialNumber_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN ULONG			BufLen)
{
    POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
    PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
    
	sprintf(pBuf, "\tSerialNumber = %s", pStaCfg->WscControl.RegData.SelfInfo.SerialNumber);
	return 0;
}
#endif /* WSC_STA_SUPPORT */


#ifdef SINGLE_SKU
INT	Show_ModuleTxpower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	RTMP_STRING *pBuf,
	IN	ULONG			BufLen)
{
	snprintf(pBuf, BufLen, "\tModuleTxpower = %d", pAd->CommonCfg.ModuleTxpower);
	return 0;
}
#endif /* SINGLE_SKU */

#ifdef APCLI_SUPPORT
 INT RTMPIoctlConnStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

 	INT i=0;
 	POS_COOKIE pObj;
 	UCHAR ifIndex;
	BOOLEAN bConnect=FALSE;
#ifdef MAC_REPEATER_SUPPORT
	MBSS_TO_CLI_LINK_MAP_T  *pMbssToCliLinkMap = NULL;
	INT	MbssIdx;
#endif
 	pObj = (POS_COOKIE) pAd->OS_Cookie;

 	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>RTMPIoctlConnStatus\n"));

 	if (pObj->ioctl_if_type != INT_APCLI)
 		return FALSE;

 	ifIndex = pObj->ioctl_if;

 	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================================\n"));
 	if((pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState == APCLI_CTRL_CONNECTED)
 		&& (pAd->ApCfg.ApCliTab[ifIndex].SsidLen != 0))
 	{
 		for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
 		{
 			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
 			STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[i];

 			if ( IS_ENTRY_APCLI(pEntry)
				&& (pEntry->Sst == SST_ASSOC)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
 			{
				if (pEntry->wdev == &pAd->ApCfg.ApCliTab[ifIndex].wdev) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCli%d         Connected AP : %02X:%02X:%02X:%02X:%02X:%02X   SSID:%s\n",
							ifIndex, PRINT_MAC(pEntry->Addr), pAd->ApCfg.ApCliTab[ifIndex].Ssid));
					bConnect=TRUE;

#ifdef MWDS
					if(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.bSupportMWDS)
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MWDSCap : YES\n"));
					else
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MWDSCap : NO\n"));
#endif /* MWDS */
				}
			}
#ifdef MAC_REPEATER_SUPPORT
			else if ( IS_ENTRY_REPEATER(pEntry)
						&& (pEntry->Sst == SST_ASSOC)
						&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)){
				if (pEntry->wdev == &pAd->ApCfg.ApCliTab[ifIndex].wdev) {
					INT CliIdx;
					BOOLEAN print_red = FALSE;
					RTMP_CHIP_CAP *cap = &pAd->chipCap;
					for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
						PREPEATER_CLIENT_ENTRY 		pReptCliEntry;
						pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
						if (pReptCliEntry->MacTabWCID == pEntry->wcid) {
							if (memcmp(pAd->MonitorAddr,pReptCliEntry->OriginalAddress,MAC_ADDR_LEN) == 0) {
								print_red = TRUE;
								break;
							}
						}
					}
#define RED(_text)  "\033[1;31m"_text"\033[0m"
					if (print_red == TRUE) { 
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (RED("Rept[wcid=%-3d] Connected AP : %02X:%02X:%02X:%02X:%02X:%02X   SSID:%s\n"),
								 i, PRINT_MAC(pEntry->Addr), pAd->ApCfg.ApCliTab[ifIndex].Ssid));
					} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rept[wcid=%-3d] Connected AP : %02X:%02X:%02X:%02X:%02X:%02X   SSID:%s\n",
							 i, PRINT_MAC(pEntry->Addr), pAd->ApCfg.ApCliTab[ifIndex].Ssid));
					}
					bConnect=TRUE;
				}
			}
#endif
 		}

		if (!bConnect)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCli%d Connected AP : Disconnect\n",ifIndex));
 	}
 	else
 	{
 		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCli%d Connected AP : Disconnect\n",ifIndex));
 	}
#ifdef MAC_REPEATER_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCli%d CliLinkMap ra:",ifIndex));
	for (MbssIdx=0; MbssIdx<pAd->ApCfg.BssidNum; MbssIdx++) {
		pMbssToCliLinkMap = &pAd->ApCfg.MbssToCliLinkMap[MbssIdx];
		if (pMbssToCliLinkMap->cli_link_wdev == &pAd->ApCfg.ApCliTab[ifIndex].wdev) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d ",MbssIdx));
		}
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ignore repeater MAC address\n\r"));
	for (i = 0; i< MAX_IGNORE_AS_REPEATER_ENTRY_NUM; i++)
	{		
		INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[i];
		if (pEntry->bInsert) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%d]%02X:%02X:%02X:%02X:%02X:%02X\n\r",i,PRINT_MAC(pEntry->MacAddr)));
		}
	}
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
 	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================================\n"));
     	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==RTMPIoctlConnStatus\n"));
 	return TRUE;
}
#endif/*APCLI_SUPPORT*/

INT32 getLegacyOFDMMCSIndex(UINT8 MCS)
{
    INT32 mcs_index = MCS;
#ifdef MT7615
        if(MCS == 0xb)
            mcs_index = 0;
        else if(MCS == 0xf)
            mcs_index = 1;
        else if(MCS == 0xa)
            mcs_index = 2;
        else if(MCS == 0xe)
            mcs_index = 3;
        else if(MCS == 0x9)
            mcs_index = 4;
        else if(MCS == 0xd)
            mcs_index = 5;
        else if(MCS == 0x8)
            mcs_index = 6;
        else if(MCS == 0xc)
            mcs_index = 7;
#endif /* MT7615 */

    return mcs_index;
}

void  getRate(HTTRANSMIT_SETTING HTSetting, ULONG* fLastTxRxRate)

{
	UINT8					Antenna = 0;
	UINT8					MCS = HTSetting.field.MCS;
	
	int rate_count = sizeof(MCSMappingRateTable)/sizeof(int);
	int rate_index = 0;
	int value = 0;

#ifdef DOT11_VHT_AC
    if (HTSetting.field.MODE >= MODE_VHT)
    {
		MCS = HTSetting.field.MCS & 0xf;
		Antenna = (HTSetting.field.MCS>>4) + 1;
       if (HTSetting.field.BW == BW_20) {
               rate_index = 112 + ((Antenna - 1) * 10) +
                       ((UCHAR)HTSetting.field.ShortGI * 160) +
                       ((UCHAR)MCS);
       }
       else if (HTSetting.field.BW == BW_40) {
               rate_index = 152 + ((Antenna - 1) * 10) +
                       ((UCHAR)HTSetting.field.ShortGI * 160) +
                       ((UCHAR)MCS);
       }
       else if (HTSetting.field.BW == BW_80) {
               rate_index = 192 + ((Antenna - 1) * 10) +
                       ((UCHAR)HTSetting.field.ShortGI * 160) +
                       ((UCHAR)MCS);
       }
	   else if (HTSetting.field.BW == BW_160) {
				rate_index = 232 + ((Antenna - 1) * 10) +
						((UCHAR)HTSetting.field.ShortGI * 160) +
						((UCHAR)MCS);
       }
    }
    else
#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT
    if (HTSetting.field.MODE >= MODE_HTMIX)
    {
		MCS = HTSetting.field.MCS;
		if ((HTSetting.field.MODE == MODE_HTMIX) 
		|| (HTSetting.field.MODE == MODE_HTGREENFIELD)) {
			Antenna = (MCS >> 3)+1;
		}
		/* map back to 1SS MCS , multiply by antenna numbers later */
		if(MCS > 7)
			MCS %= 8;
    	rate_index = 16 + ((UCHAR)HTSetting.field.BW *24) + ((UCHAR)HTSetting.field.ShortGI *48) + ((UCHAR)MCS);
    }
    else
#endif /* DOT11_N_SUPPORT */
    if (HTSetting.field.MODE == MODE_OFDM)
    	rate_index = getLegacyOFDMMCSIndex(HTSetting.field.MCS) + 4;
    else if (HTSetting.field.MODE == MODE_CCK)
    	rate_index = (UCHAR)(HTSetting.field.MCS);

    if (rate_index < 0)
        rate_index = 0;

    if (rate_index >= rate_count)
        rate_index = rate_count-1;
	if(HTSetting.field.MODE != MODE_VHT)
    	value = (MCSMappingRateTable[rate_index] * 5)/10;
	else
		value =  MCSMappingRateTable[rate_index];		
	
#if defined(DOT11_VHT_AC) || defined(DOT11_N_SUPPORT)
    if (HTSetting.field.MODE >= MODE_HTMIX && HTSetting.field.MODE < MODE_VHT)
		value *= Antenna;
#endif /* DOT11_VHT_AC */

	*fLastTxRxRate=(ULONG)value;
	return;
}


#ifdef MT_MAC
INT Show_TxVinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR VectorIdx;
#if !defined(MT7615) && !defined(MT7622)
	UINT32 value32;
	TXV1_MAC2PHY txv1;
	TXV2_MAC2PHY txv2;
	TXV3_MAC2PHY txv3;
#endif /* !defined(MT7615) && !defined(MT7622) */

	VectorIdx = simple_strtol(arg, 0, 10);

#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, not finish yet
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotFinishYet!\n", __FUNCTION__));
	return FALSE;
#else
	switch (VectorIdx)
	{
		case 1:
			RTMP_IO_READ32(pAd, CR_PHYMUX_12, &value32);
			txv1.txrate = value32 & 0x7F;
			txv1.STBC  = (value32 >> 7) & 0x3;
			txv1.Ldpc_Bcc = (value32 >> 9) & 0x1;
			txv1.Ness   = (value32 >> 10) & 0x3;
			txv1.txmode   = (value32 >> 12) & 0x7;
			txv1.bw      = (value32 >> 15) & 0x3;
			txv1.txpower   = (value32 >> 22) & 0x7F;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
					"================== Tx vector 1 ================ \n"
					" Tx mode = %d \n"
					" 0 (CCK), 1 (OFDM), 2 (11n Mixed mode), 3 (11n Green mode), 4 (11ac) \n"
					" Tx rate   = %x : \n"
					" CCK long preamble  : 0 (1M), 1 (2M), 2 (5.5M), 3(11M) \n"
					" CCK short preamble : 5 (2M), 6 (2M), 7 (11M) \n"
					" OFDM                    : B (6M), F (9M), A (12M), E (18M), 9 (24M) \n"
					"                               D (36M), 8 (48M), C (54M) \n"
					" 11n                       : 0 ~ 15 (MCS0 ~ MCS15) \n"
					" STBC = %d \n"
					" Ness = %d \n"
					" FEC   = %s \n"
					" BW   = %d \n"
					" 0 (20M), 1 (40M), 2 (80M), 4 (160M or 80 + 80M\n"
					" Tx power = %d \n"
					" S(5.1) dBm \n"
					"========================================== \n",
					txv1.txmode, txv1.txrate, txv1.STBC, txv1.Ness,
					(txv1.Ldpc_Bcc == 1) ? "LDPC" : "BCC",
					txv1.bw, txv1.txpower));
			break;
		case 2:
			RTMP_IO_READ32(pAd, CR_PHYMUX_13, &value32);
			txv2.agg = value32 & 0x1;
			txv2.gi    = (value32 >> 1) & 0x1;
			txv2.smooth   = (value32 >> 2) & 0x1;
			txv2.sounding = (value32 >> 3) & 0x1;
			txv2.txbf = (value32 >> 4) & 0x7F;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
					"================== Tx vector 2 ================ \n"
					" Agg        = %s \n"
					" GI          = %s \n"
					" Smooth   = %s \n"
					" Sounding = 0x%x \n"
					" TxBf        = %d \n"
					" 1 (ITxBf enable), 2 (ETxBf enable), 4 (Spatial ext) \n"
					" Antenna priority = %d \n"
					" 0 (Tx0 > Tx1 > Tx2), 1 (Tx0 > Tx2 > Tx1), 2 (Tx1 > Tx0 > Tx2), \n"
					" 3 (Tx1 > Tx2 > Tx0), 4 (Tx2 > Tx0 > Tx1), 5 (Tx2 > Tx1 > Tx0), \n"
					" 6 (Tx0 > Tx1, Tx2=off), 7 (Tx0 > Tx2, Tx1=off) \n"
					"========================================== \n",
					(txv2.agg == 1) ? "Enable" : "Disable",
					(txv2.gi == 1) ? "Short GI" : "Long GI",
					(txv2.smooth == 1) ? "Enable" : "Disable",
					txv2.sounding,
					(INT)(txv2.txbf & 0x7),
					(txv2.txbf >> 3) & 0x7));
			break;

		case 3:
			RTMP_IO_READ32(pAd, CR_PHYMUX_14, &value32);
			txv3.bw_status = (value32 >> 10) & 0x1;
			txv3.mac[5] = (value32 >> 24) & 0xFF;
			txv3.mac[4] = (value32 >> 16) & 0xFF;

			RTMP_IO_READ32(pAd, CR_PHYMUX_15, &value32);
			txv3.mac[3] = (value32 >> 24) & 0xFF;
			txv3.mac[2] = (value32 >> 16) & 0xFF;
			txv3.mac[1] = (value32 >> 8) & 0xFF;
			txv3.mac[0] = (value32 >> 0) & 0xFF;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
					"================== Tx vector 3 & 4 ================ \n"
					" BW status = %s \n"
					" MAC address = %x : %x : %x : %x :%x : %x \n"
					" ============================================ \n",
					(txv3.bw_status == 1) ? "Dynamic" : "Static",
					txv3.mac[5], txv3.mac[4], txv3.mac[3], txv3.mac[2], txv3.mac[1], txv3.mac[0]));
			break;
		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" Please select value between 1 ~ 3 \n"));
			break;
	}
#endif /* defined(MT7615) || defined(MT7622) */

	return TRUE;

}
#endif


#ifdef TXBF_SUPPORT


/*
	Set_InvTxBfTag_Proc - Invalidate BF Profile Tags
		usage: "iwpriv ra0 set InvTxBfTag=n"
		Reset Valid bit and zero out MAC address of each profile. The next profile will be stored in profile 0
*/
INT	Set_InvTxBfTag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/*int profileNum;
	UCHAR row[EXP_MAX_BYTES];
	UCHAR r163Value = 0;
	UINT  rValue;*/

	/* Disable Profile Updates during access */
#ifndef MT_MAC
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R163, &r163Value);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, r163Value & ~0x88);

	/* Invalidate Implicit tags */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, 0);
	for (profileNum=0; profileNum<4; profileNum++) {
		Read_TagField(pAd, row, profileNum);
		row[0] &= 0x7F;
		row[1] = row[2] = row[3] = row[4] = row[5] = row[6] = 0xAA;
		Write_TagField(pAd, row, profileNum);
	}

	/* Invalidate Explicit tags */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, 4);
	for (profileNum=0; profileNum<4; profileNum++) {
		Read_TagField(pAd, row, profileNum);
		row[0] &= 0x7F;
		row[1] = row[2] = row[3] = row[4] = row[5] = row[6] = 0x55;
		Write_TagField(pAd, row, profileNum);
	}

	/* Restore Profile Updates */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R163, r163Value);
#endif /* MT_MAC */

	return TRUE;
}



#if defined(MT76x2) || defined(MT_MAC)
/*
	Set_ETxBfCodebook_Proc - Set ETxBf Codebook
	usage: iwpriv ra0 set ETxBfCodebook=0 to 3
*/
INT Set_ETxBfCodebook_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = simple_strtol(arg, 0, 10);

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	if (t > 3) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfCodebook_Proc: value > 3!\n"));
		return FALSE;
	}

	return TRUE;
}


/*
	Set_ETxBfCoefficient_Proc - Set ETxBf Coefficient
		usage: iwpriv ra0 set ETxBfCoefficient=0 to 3
*/
INT Set_ETxBfCoefficient_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = simple_strtol(arg, 0, 10);

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	if (t > 3) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfCoefficient_Proc: value > 3!\n"));
		return FALSE;
	}

	return TRUE;
}


/*
	Set_ETxBfGrouping_Proc - Set ETxBf Grouping
		usage: iwpriv ra0 set ETxBfGrouping=0 to 2
*/
INT Set_ETxBfGrouping_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = simple_strtol(arg, 0, 10);

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

	if (t > 2) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfGrouping_Proc: value > 2!\n"));
		return FALSE;
	}

	return TRUE;
}
#endif /* MT76x2 || MT_MAC */

/*
	Set_ETxBfNoncompress_Proc - Set ETxBf Noncompress option
		usage: iwpriv ra0 set ETxBfNoncompress=0 or 1
*/
INT Set_ETxBfNoncompress_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG t = simple_strtol(arg, 0, 10);

	if (t > 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ETxBfNoncompress_Proc: value > 1!\n"));
		return FALSE;
	}

	pAd->CommonCfg.ETxBfNoncompress = t;
	return TRUE;
}


/*
	Set_ETxBfIncapable_Proc - Set ETxBf Incapable option
		usage: iwpriv ra0 set ETxBfIncapable=0 or 1
*/
INT Set_ETxBfIncapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	ULONG t = simple_strtol(arg, 0, 10);

	if (t > 1)
		return FALSE;

	pAd->CommonCfg.ETxBfIncapable = t;
#ifdef MT_MAC
        mt_WrapSetETxBFCap(pAd, wdev, &pAd->CommonCfg.HtCapability.TxBFCap);
#else
	setETxBFCap(pAd, &pAd->CommonCfg.HtCapability.TxBFCap);
#endif /* MT_MAC */
	return TRUE;
}


/*
	Set_ITxBfDivCal_Proc - Calculate ITxBf Divider Calibration parameters
	usage: iwpriv ra0 set ITxBfDivCal=dd
			0=>display calibration parameters
			1=>update EEPROM values
			2=>update BBP R176
			10=>display calibration parameters and dump capture data
			11=>Skip divider calibration, just capture and dump capture data
*/
INT	Set_ITxBfDivCal_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int calFunction;

	UINT32 value, value1, restore_value, loop = 0;

	/* backup mac 1004 value */
	RTMP_IO_READ32(pAd, 0x1004, &restore_value);

	/* Backup the original RTS retry count and then set to 0 */
	//RTMP_IO_READ32(pAd, 0x1344, &pAd->rts_tx_retry_num);

	/* disable mac tx/rx */
	value = restore_value;
	value &= ~0xC;
	RTMP_IO_WRITE32(pAd, 0x1004, value);

	/* set RTS retry count = 0 */
	RTMP_IO_WRITE32(pAd, 0x1344, 0x00092B00);

	/* wait mac 0x1200, bbp 0x2130 idle */
	do {
		RTMP_IO_READ32(pAd, 0x1200, &value);
		value &= 0x1;
		RTMP_IO_READ32(pAd, 0x2130, &value1);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0\n", __FUNCTION__));
		RtmpusecDelay(1);
		loop++;
	} while (((value != 0) || (value1 != 0)) && (loop < 300));

	if (loop >= 300) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0 > 300 times\n", __FUNCTION__));
		return FALSE;
	}

	calFunction = simple_strtol(arg, 0, 10);
	pAd->chipOps.fITxBfDividerCalibration(pAd, calFunction, 0, NULL);

	/* enable TX/RX */
	RTMP_IO_WRITE32(pAd, 0x1004, restore_value);

	/* Restore RTS retry count */
	//RTMP_IO_WRITE32(pAd, 0x1344, pAd->rts_tx_retry_num);

	return TRUE;
}





#ifdef MT_MAC
/*
	Set_ETxBfEnCond_Proc - enable/disable ETxBF
	usage: iwpriv ra0 set ETxBfEnCond=dd
		0=>disable, 1=>enable
	Note: After use this command, need to re-run apStartup()/LinkUp() operations to sync all status.
		  If ETxBfIncapable!=0 then we don't need to reassociate.
*/
INT	Set_ETxBfEnCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i, ucETxBfEn;
	UCHAR ucStatus = FALSE;
	MAC_TABLE_ENTRY		*pEntry;
	TXBF_STATUS_INFO    TxBfInfo;
	struct wifi_dev *wdev = NULL;
	ucETxBfEn = simple_strtol(arg, 0, 10);

	if (ucETxBfEn > 1)
		return FALSE;

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_NONE(pEntry))
		{
			wdev = pEntry->wdev;
		     TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
             TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;
		     TxBfInfo.ucPhyMode   = wdev->PhyMode;
		     TxBfInfo.u2Channel   = wdev->channel;
		     TxBfInfo.pHtTxBFCap  = &pAd->CommonCfg.HtCapability.TxBFCap;
             TxBfInfo.cmmCfgETxBfIncapable= pAd->CommonCfg.ETxBfIncapable;
             TxBfInfo.cmmCfgETxBfNoncompress= pAd->CommonCfg.ETxBfNoncompress;
#ifdef VHT_TXBF_SUPPORT
		     TxBfInfo.pVhtTxBFCap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
#endif
		     TxBfInfo.u4WTBL1 = pAd->mac_ctrl.wtbl_base_addr[0] +  i * pAd->mac_ctrl.wtbl_entry_size[0];
             TxBfInfo.u4WTBL2 = pAd->mac_ctrl.wtbl_base_addr[1] +  i * pAd->mac_ctrl.wtbl_entry_size[1];
             TxBfInfo.ucETxBfTxEn = ucETxBfEn;
             TxBfInfo.ucITxBfTxEn  = FALSE;
             TxBfInfo.ucWcid = i;
             TxBfInfo.ucBW = pEntry->HTPhyMode.field.BW;
	         TxBfInfo.ucNDPARate = 2; // MCS2

	         ucStatus = AsicTxBfEnCondProc(pAd, &TxBfInfo);
		}
	}

	return ucStatus;
}
#endif /* MT_MAC */



INT	Set_NoSndgCntThrd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++){
		pAd->MacTab.Content[i].noSndgCntThrd = simple_strtol(arg, 0, 10);
	}
	return TRUE;
}

INT	Set_NdpSndgStreams_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++){
		pAd->MacTab.Content[i].ndpSndgStreams = simple_strtol(arg, 0, 10);
	}
	return TRUE;
}



#if defined(MT_MAC) && (!defined(MT7636))
INT Set_TxBfTxApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR	*value;
    UCHAR	Input[5];
    INT		i;
    CHAR    ucWlanIdx;
    BOOLEAN fgETxBf, fgITxBf, fgMuTxBf, fgPhaseCali;
    BOOLEAN fgStatus = TRUE;

	os_zero_mem(Input, sizeof(Input));

    if(strlen(arg) != 14)
        return FALSE;

    for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
    {
        if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
        return FALSE;  /*Invalid*/

        AtoH(value, &Input[i++], 1);
    }

    ucWlanIdx   = Input[0];
    fgETxBf     = Input[1];
    fgITxBf     = Input[2];
    fgMuTxBf    = Input[3];
    fgPhaseCali = Input[4];

    CmdTxBfTxApplyCtrl(pAd,
	                   ucWlanIdx,
	                   fgETxBf,
	                   fgITxBf,
	                   fgMuTxBf,
	                   fgPhaseCali);

    return fgStatus;
}

#ifdef TXBF_DYNAMIC_DISABLE
INT Set_TxBfDynamicDisable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
   BOOLEAN fgStatus = TRUE;
   BOOLEAN fgDisable;
   
   fgDisable = simple_strtol(arg, 0, 10);

   DynamicTxBfDisable(pAd, fgDisable);

    return fgStatus;
}
#endif /* TXBF_DYNAMIC_DISABLE */

INT	Set_Trigger_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR			Input[7];
	CHAR			*value;
	INT				i;
	BOOLEAN         fgStatus = FALSE;
	UCHAR           u4SndInterval, ucSu_Mu, ucMuNum, ucWlanId[4];

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 20)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSu_Mu       = Input[0];
	ucMuNum       = Input[1];
	u4SndInterval = Input[2] << 2;
	ucWlanId[0]   = Input[3];
	ucWlanId[1]   = Input[4];
	ucWlanId[2]   = Input[5];
	ucWlanId[3]   = Input[6];

    if (pAd->Antenna.field.TxPath <= 1)
        return FALSE;

    if(mt_Trigger_Sounding_Packet(pAd,
                                  TRUE,
	                              u4SndInterval,
	                              ucSu_Mu,
	                              ucMuNum,
	                              ucWlanId) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

	return fgStatus;
}

INT	Set_Stop_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;

    if(mt_Trigger_Sounding_Packet(pAd,
                                  FALSE,
	                              0,
	                              0,
	                              0,
	                              NULL) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

	return fgStatus;
}


INT	Set_TxBfPfmuMemAlloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR			Input[2];
	CHAR			*value;
	INT				i;
	BOOLEAN         fgStatus = FALSE;
	UCHAR           ucSu_Mu, ucWlanIdx;

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 5)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSu_Mu   = Input[0];
	ucWlanIdx = Input[1];

    if(CmdPfmuMemAlloc(pAd,
                       ucSu_Mu,
                       ucWlanIdx) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

	return fgStatus;
}


INT	Set_TxBfPfmuMemRelease(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;
	UCHAR           ucWlanIdx;

	ucWlanIdx  = simple_strtol(arg, 0, 10);

    if(CmdPfmuMemRelease(pAd, ucWlanIdx) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

	return fgStatus;
}


INT	Set_TxBfPfmuMemAllocMapRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN         fgStatus = FALSE;


    if(CmdPfmuMemAllocMapRead(pAd) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

	return fgStatus;
}


INT Set_StaRecBfUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR			 *value;
    UCHAR			 Input[23];
	INT				 i;
	CHAR             BssIdx, WlanIdx;
	PMAC_TABLE_ENTRY pEntry;
	BOOLEAN          fgStatus = FALSE;

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 68)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

    WlanIdx                          = Input[0];
    BssIdx                           = Input[1];

	pEntry                           = &pAd->MacTab.Content[WlanIdx];
	if (pEntry == NULL)
	{
	    return FALSE;
	}
	pEntry->rStaRecBf.u2PfmuId       = Input[2];
	pEntry->rStaRecBf.fgSU_MU        = Input[3];
	pEntry->rStaRecBf.fgETxBfCap     = Input[4];
	pEntry->rStaRecBf.ucNdpaRate     = Input[5];
	pEntry->rStaRecBf.ucNdpRate      = Input[6];
	pEntry->rStaRecBf.ucReptPollRate = Input[7];
	pEntry->rStaRecBf.ucTxMode       = Input[8];
	pEntry->rStaRecBf.ucNc           = Input[9];
	pEntry->rStaRecBf.ucNr           = Input[10];
	pEntry->rStaRecBf.ucCBW          = Input[11];
	pEntry->rStaRecBf.ucSEIdx        = Input[12];
	pEntry->rStaRecBf.ucTotMemRequire= Input[13];
	pEntry->rStaRecBf.ucMemRequire20M= Input[14];
	pEntry->rStaRecBf.ucMemRow0      = Input[15];
	pEntry->rStaRecBf.ucMemCol0      = Input[16];
    pEntry->rStaRecBf.ucMemRow1      = Input[17];
	pEntry->rStaRecBf.ucMemCol1      = Input[18];
	pEntry->rStaRecBf.ucMemRow2      = Input[19];
	pEntry->rStaRecBf.ucMemCol2      = Input[20];
	pEntry->rStaRecBf.ucMemRow3      = Input[21];
	pEntry->rStaRecBf.ucMemCol3      = Input[22];

    // Default setting
	pEntry->rStaRecBf.u2SmartAnt     = 0;
    pEntry->rStaRecBf.ucSoundingPhy  = 1;


    pEntry->rStaRecBf.uciBfTimeOut   = 0xFF;
    pEntry->rStaRecBf.uciBfDBW       = 0;
    pEntry->rStaRecBf.uciBfNcol      = 0;
    pEntry->rStaRecBf.uciBfNrow      = 0;


	{
		STA_REC_CFG_T StaCfg;
		os_zero_mem(&StaCfg,sizeof(STA_REC_CFG_T));
        StaCfg.MuarIdx = 0;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = 0;
		StaCfg.u4EnableFeature = (1 << STA_REC_BF);
		StaCfg.ucBssIndex = BssIdx;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
	    if (CmdExtStaRecUpdate(pAd,StaCfg) == STATUS_TRUE)
		{
		    fgStatus = TRUE;
		}
	}
	return fgStatus;
}


INT Set_StaRecBfRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR             WlanIdx;
	BOOLEAN          fgStatus = FALSE;

    WlanIdx = simple_strtol(arg, 0, 10);

    if (CmdETxBfStaRecRead(pAd,
	                       WlanIdx) == STATUS_TRUE)
	{
	    fgStatus = TRUE;
	}

	return fgStatus;
}


INT Set_TxBfTxPwrBackOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR			 *value;
    UCHAR			 Input[21];
	INT				 i;
	UCHAR            ucBand;
	CHAR             acTxPwrFccBfOnCase[10];
	CHAR             acTxPwrFccBfOffCase[10];
	BOOLEAN          fgStatus = FALSE;
	
	os_zero_mem(Input, sizeof(Input));

    if(strlen(arg) != 62)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

    ucBand  = Input[0];
    os_move_mem(acTxPwrFccBfOnCase, &Input[1], 10);
    os_move_mem(acTxPwrFccBfOffCase, &Input[11], 10);

    if (CmdTxBfTxPwrBackOff(pAd,
                            ucBand,
                            acTxPwrFccBfOnCase,
                            acTxPwrFccBfOffCase) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
    
}


INT Set_TxBfAwareCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    BOOLEAN fgBfAware, fgStatus = FALSE;

    fgBfAware = simple_strtol(arg, 0, 10);

    if (CmdTxBfAwareCtrl(pAd, fgBfAware) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}


INT Set_StaRecCmmUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    PMAC_TABLE_ENTRY pEntry;
    CHAR			 *value;
    UCHAR			 Input[9];
	INT				 i;
	CHAR             BssIdx, WlanIdx;
	BOOLEAN          fgStatus = FALSE;

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 26)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

    WlanIdx       = Input[0];
    BssIdx        = Input[1];

    pEntry = &pAd->MacTab.Content[WlanIdx];
	if (pEntry == NULL)
	{
	    return FALSE;
	}

	pEntry->Aid     = Input[2];
	pEntry->Addr[0] = Input[3];
	pEntry->Addr[1] = Input[4];
	pEntry->Addr[2] = Input[5];
	pEntry->Addr[3] = Input[6];
	pEntry->Addr[4] = Input[7];
	pEntry->Addr[5] = Input[8];

	{
		STA_REC_CFG_T StaCfg;

		os_zero_mem(&StaCfg,sizeof(STA_REC_CFG_T));

        StaCfg.MuarIdx = 0;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = CONNECTION_INFRA_AP;
		StaCfg.u4EnableFeature = (1 << STA_REC_BASIC_STA_RECORD);
		StaCfg.ucBssIndex = BssIdx;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.IsNewSTARec = TRUE;

	    if (CmdExtStaRecUpdate(pAd,StaCfg) == STATUS_TRUE)
		{
		    fgStatus = TRUE;
		}
	}
	return fgStatus;
}


INT Set_BssInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR			 *value;
    UCHAR			 Input[8];
    UCHAR            Bssid[MAC_ADDR_LEN];
	INT				 i;
	CHAR             OwnMacIdx, BssIdx;
	BOOLEAN          fgStatus = FALSE;
	BSS_INFO_ARGUMENT_T bss_info_argument;

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 23)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

    OwnMacIdx = Input[0];
	BssIdx    = Input[1];
	Bssid[0]  = Input[2];
	Bssid[1]  = Input[3];
	Bssid[2]  = Input[4];
	Bssid[3]  = Input[5];
	Bssid[4]  = Input[6];
	Bssid[5]  = Input[7];

    NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
    bss_info_argument.OwnMacIdx = OwnMacIdx;
    bss_info_argument.ucBssIndex = BssIdx;
	os_move_mem(bss_info_argument.Bssid,Bssid,MAC_ADDR_LEN);
    bss_info_argument.ucBcMcWlanIdx = 0; //MCAST_WCID which needs to be modified by Patrick;
    bss_info_argument.NetworkType = NETWORK_INFRA;
    bss_info_argument.u4ConnectionType = CONNECTION_INFRA_AP;
    bss_info_argument.CipherSuit = CIPHER_SUIT_NONE;
    bss_info_argument.bss_state = BSS_ACTIVE;
    bss_info_argument.u4BssInfoFeature = BSS_INFO_BASIC_FEATURE;

	if(AsicBssInfoUpdate(pAd, bss_info_argument) == STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}


INT Set_DevInfoUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR			 *value;
    UCHAR			 Input[8];
    UCHAR            OwnMacAddr[MAC_ADDR_LEN];
	INT				 i;
	CHAR             OwnMacIdx;
	BOOLEAN          fgStatus = FALSE;
	UINT8		     BandIdx = 0;

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 23)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	OwnMacIdx     = Input[0];
	OwnMacAddr[0] = Input[1];
	OwnMacAddr[1] = Input[2];
	OwnMacAddr[2] = Input[3];
	OwnMacAddr[3] = Input[4];
	OwnMacAddr[4] = Input[5];
	OwnMacAddr[5] = Input[6];
	BandIdx = Input[7];

	if (AsicDevInfoUpdate(
	    pAd,
	    OwnMacIdx,
	    OwnMacAddr,
	    BandIdx,
	    TRUE,
	    DEVINFO_ACTIVE_FEATURE) == STATUS_TRUE)
	{
		fgStatus = TRUE;
	}

    return fgStatus;
}
#endif /* MT_MAC */

/*
	Set_ITxBfEn_Proc - enable/disable ITxBF
	usage: iwpriv ra0 set ITxBfEn=dd
	0=>disable, 1=>enable
*/
#if (!defined(MT_MAC)) && (!defined(MT76x2))
INT	Set_ITxBfEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	UCHAR enableITxBF;
	BOOLEAN bCalibrated;
	UINT8 byteValue;

	enableITxBF = simple_strtol(arg, 0, 10);

	if (enableITxBF > 1)
		return FALSE;
#ifdef MT_MAC
    //bCalibrated = mt_Wrap_chk_itxbf_calibration(pAd);
    bCalibrated = 0;
#else
	bCalibrated = rtmp_chk_itxbf_calibration(pAd);
#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set ITxBfEn=%d, calibration of ITxBF=%d, so enableITxBF=%d!\n",
					enableITxBF , bCalibrated, (enableITxBF & bCalibrated)));

	enableITxBF &= bCalibrated;

	pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn = enableITxBF && (pAd->Antenna.field.TxPath > 1);

	rtmp_asic_set_bf(pAd);

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[i];
		if ((!IS_ENTRY_NONE(pMacEntry)) && (pAd->Antenna.field.TxPath> 1))
			pMacEntry->iTxBfEn = enableITxBF;
	}

	if (enableITxBF || pAd->CommonCfg.ETxBfEnCond)
	{
		RT30xxReadRFRegister(pAd, RF_R39, (PUCHAR)&byteValue);
		byteValue |= 0x40;
		RT30xxWriteRFRegister(pAd, RF_R39, (UCHAR)byteValue);

		RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)&byteValue);
		byteValue |= 0x20;
		RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)byteValue);
	}

	/* If enabling ITxBF then set LNA compensation, do a Divider Calibration and update BBP registers */
	if (enableITxBF) {
		pAd->chipOps.fITxBfLNAPhaseCompensate(pAd);
		pAd->chipOps.fITxBfDividerCalibration(pAd, 2, 0, NULL);
	}
	else
	{
		/* depends on Gary Tsao's comments. */
		if (pAd->CommonCfg.ETxBfEnCond == 0)
		{
		RT30xxReadRFRegister(pAd, RF_R39, (PUCHAR)&byteValue);
			byteValue &= (~0x40);
		RT30xxWriteRFRegister(pAd, RF_R39, (UCHAR)byteValue);

		RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)&byteValue);
			byteValue &= (~0x20);
		RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)byteValue);
	}

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R173, 0);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &byteValue);
		byteValue &= ~0x60;
		for ( i = 0; i < 3; i++)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, (byteValue & (i << 5)));
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R174, 0);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R176, 0);
		}
	}
	return TRUE;
}

#endif /* !(MT76x2 || MT_MAC) */


#if defined(MT_MAC)
INT	Set_ITxBfEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i, ucITxBfEn;
	INT   u4Status = FALSE;
	MAC_TABLE_ENTRY		*pEntry;
	TXBF_STATUS_INFO    TxBfInfo;
	struct wifi_dev *wdev = NULL;

	ucITxBfEn = simple_strtol(arg, 0, 10);

	if (ucITxBfEn > 1)
		return FALSE;

	for (i=0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_NONE(pEntry))
		{
		    wdev = pEntry->wdev;
		    TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
            TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;
		    TxBfInfo.ucPhyMode   = wdev->PhyMode;
		    TxBfInfo.u2Channel   = wdev->channel;
		    TxBfInfo.pHtTxBFCap  = &pAd->CommonCfg.HtCapability.TxBFCap;
            TxBfInfo.cmmCfgETxBfIncapable  = pAd->CommonCfg.ETxBfIncapable;
            TxBfInfo.cmmCfgETxBfNoncompress= pAd->CommonCfg.ETxBfNoncompress;
#ifdef VHT_TXBF_SUPPORT
		    TxBfInfo.pVhtTxBFCap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
#endif
		    TxBfInfo.u4WTBL1 = pAd->mac_ctrl.wtbl_base_addr[0] + i * pAd->mac_ctrl.wtbl_entry_size[0];
            TxBfInfo.u4WTBL2 = pAd->mac_ctrl.wtbl_base_addr[1] + i * pAd->mac_ctrl.wtbl_entry_size[1];
            TxBfInfo.ucETxBfTxEn = FALSE;
            TxBfInfo.ucITxBfTxEn = ucITxBfEn;
            TxBfInfo.ucWcid      = i;
            TxBfInfo.ucBW        = pEntry->HTPhyMode.field.BW;
            TxBfInfo.ucNDPARate  = 2; // MCS2
	        u4Status = AsicTxBfEnCondProc(pAd, &TxBfInfo);
		}
	}

	return u4Status;
}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */


#ifdef VHT_TXBF_SUPPORT
/*
	The VhtNDPA sounding inupt string format should be xx:xx:xx:xx:xx:xx-d,
		=>The six 2 digit hex-decimal number previous are the Mac address,
		=>The seventh decimal number is the MCS value.
*/
INT Set_VhtNDPA_Sounding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR mac[6];
	UINT mcs;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n%s\n", arg));

	/*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and MCS value in decimal format.*/
	if(strlen(arg) < 19)
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		mcs = (UINT)simple_strtol((token+1), 0, 10);

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
					PRINT_MAC(mac), mcs));

		pEntry = MacTableLookup(pAd, (PUCHAR) mac);
	    	if (pEntry) {
#ifdef SOFT_SOUNDING
			pEntry->snd_rate.field.MODE = MODE_VHT;
			pEntry->snd_rate.field.BW = (mcs / 100) > BW_80 ? BW_80 : (mcs / 100);
			mcs %= 100;
			pEntry->snd_rate.field.MCS = ((mcs / 10) << 4 | (mcs % 10));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Trigger VHT NDPA Sounding=%02x:%02x:%02x:%02x:%02x:%02x, snding rate=VHT-%sHz, %dSS-MCS%d\n",
					__FUNCTION__, PRINT_MAC(mac),
					get_bw_str(pEntry->snd_rate.field.BW),
					(pEntry->snd_rate.field.MCS >> 4) + 1,
					pEntry->snd_rate.field.MCS & 0xf));
#endif
			trigger_vht_ndpa(pAd, pEntry);
	    	}

		return TRUE;
	}

	return FALSE;
}
#endif /* VHT_TXBF_SUPPORT */


#if defined(MT_MAC) && (!defined(MT7636))
#ifdef TXBF_SUPPORT

INT Set_TxBfProfileTag_Help(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
		"========================================================================================================================\n"
		"TxBfProfile Tag1 setting example :\n"
		"iwpriv ra0 set TxBfProfileTagPfmuIdx  =xx \n"
		"iwpriv ra0 set TxBfProfileTagBfType   =xx (0: iBF; 1: eBF) \n"
		"iwpriv ra0 set TxBfProfileTagBw       =xx (0/1/2/3 : BW20/40/80/160NC) \n"
		"iwpriv ra0 set TxBfProfileTagSuMu     =xx (0:SU, 1:MU) \n"
		"iwpriv ra0 set TxBfProfileTagInvalid  =xx (0: valid, 1: invalid) \n"
		"iwpriv ra0 set TxBfProfileTagMemAlloc =xx:xx:xx:xx:xx:xx:xx:xx (mem_row, mem_col), .. \n"
	    "iwpriv ra0 set TxBfProfileTagMatrix   =nrow:nol:ng:LM\n"
	    "iwpriv ra0 set TxBfProfileTagSnr      =SNR_STS0:SNR_STS1:SNR_STS2:SNR_STS3\n"
	    "\n\n"
	    "TxBfProfile Tag2 setting example :\n"
	    "iwpriv ra0 set TxBfProfileTagSmtAnt   =xx (11:0) \n"
	    "iwpriv ra0 set TxBfProfileTagSeIdx    =xx \n"
	    "iwpriv ra0 set TxBfProfileTagRmsdThrd =xx \n"
	    "iwpriv ra0 set TxBfProfileTagMcsThrd  =xx:xx:xx:xx:xx:xx (MCS TH L1SS:S1SS:L2SS:....)\n"
	    "iwpriv ra0 set TxBfProfileTagTimeOut  =xx \n"
	    "iwpriv ra0 set TxBfProfileTagDesiredBw=xx (0/1/2/3 : BW20/40/80/160NC) \n"
	    "iwpriv ra0 set TxBfProfileTagDesiredNc=xx \n"
	    "iwpriv ra0 set TxBfProfileTagDesiredNr=xx \n"
		"\n\n"
	    "Read TxBf profile Tag :\n"
	    "iwpriv ra0 set TxBfProfileTagRead     =xx (PFMU ID)\n"
	    "\n"
	    "Write TxBf profile Tag :\n"
	    "iwpriv ra0 set TxBfProfileTagWrite    =xx (PFMU ID)\n"
	    "When you use one of relative CMD to update one of tag parameters, you should call TxBfProfileTagWrite to update Tag\n"
		"\n\n"
		"Read TxBf profile Data	:\n"
	    "iwpriv ra0 set TxBfProfileDataRead    =xx (PFMU ID)\n"
	    "\n"
	    "Write TxBf profile Data :\n"
	    "iwpriv ra0 set TxBfProfileDataWrite   =BW :subcarrier:phi11:psi2l:Phi21:Psi31:Phi31:Psi41:Phi22:Psi32:Phi32:Psi42:Phi33:Psi43\n"
	    "iwpriv ra0 set TxBfProfileDataWriteAll=Profile ID : BW (BW       : 0x00 (20M) , 0x01 (40M), 0x02 (80M), 0x3 (160M)\n"
	    "When you use CMD TxBfProfileDataWrite to update profile data per subcarrier, you should call TxBfProfileDataWriteAll to update all of \n"
	    "subcarrier's profile data.\n\n"
	    "Read TxBf profile PN	:\n"
	    "iwpriv ra0 set TxBfProfilePnRead      =xx (PFMU ID)\n"
	    "\n"
	    "Write TxBf profile PN :\n"
	    "iwpriv ra0 set TxBfProfilePnWrite     =Profile ID:BW:1STS_Tx0:1STS_Tx1:1STS_Tx2:1STS_Tx3:2STS_Tx0:2STS_Tx1:2STS_Tx2:2STS_Tx3:3STS_Tx1:3STS_Tx2:3STS_Tx3\n"
		"========================================================================================================================\n"));

	return TRUE;
}


INT Set_TxBfProfileTag_PfmuIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;

	profileIdx    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_PfmuIdx(&pAd->rPfmuTag1, profileIdx);

	return TRUE;
}


INT Set_TxBfProfileTag_BfType(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   ucBfType;

	ucBfType    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_TxBfType(&pAd->rPfmuTag1, ucBfType);

	return TRUE;
}


INT Set_TxBfProfileTag_DBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   ucBw;

	ucBw    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_DBW(&pAd->rPfmuTag1, ucBw);

	return TRUE;
}


INT Set_TxBfProfileTag_SuMu(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   ucSuMu;

	ucSuMu    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_SuMu(&pAd->rPfmuTag1, ucSuMu);

	return TRUE;
}


INT Set_TxBfProfileTag_InValid(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   InValid;

	InValid    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_InValid(&pAd->rPfmuTag1, InValid);

	return TRUE;
}


INT Set_TxBfProfileTag_Mem(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[8];
	CHAR	*value;
	INT 	i;
	UCHAR   aMemAddrColIdx[4], aMemAddrRowIdx[4];

	os_zero_mem(Input, sizeof(Input));

	/* mem col0:row0:col1:row1:col2:row2:col3:row3 */
	if(strlen(arg) != 23)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	aMemAddrColIdx[0] = Input[0];
	aMemAddrRowIdx[0] = Input[1];
	aMemAddrColIdx[1] = Input[2];
	aMemAddrRowIdx[1] = Input[3];
	aMemAddrColIdx[2] = Input[4];
	aMemAddrRowIdx[2] = Input[5];
	aMemAddrColIdx[3] = Input[6];
	aMemAddrRowIdx[3] = Input[7];

    TxBfProfileTag_Mem(&pAd->rPfmuTag1, aMemAddrColIdx, aMemAddrRowIdx);

	return TRUE;
}


INT Set_TxBfProfileTag_Matrix(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[8];
	CHAR	*value;
	INT 	i;
	UCHAR   ucNrow,ucNcol,ucNgroup,ucLM,ucCodeBook,ucHtcExist;

	os_zero_mem(Input, sizeof(Input));

	/* nrow:nol:ng:LM:CodeBook:HtcExist */
	if(strlen(arg) != 17)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucNrow     = Input[0];
	ucNcol     = Input[1];
	ucNgroup   = Input[2];
	ucLM       = Input[3];
	ucCodeBook = Input[4];
	ucHtcExist = Input[5];

    TxBfProfileTag_Matrix(&pAd->rPfmuTag1,
                          ucNrow,
                          ucNcol,
                          ucNgroup,
                          ucLM,
                          ucCodeBook,
                          ucHtcExist);

	return TRUE;
}


INT Set_TxBfProfileTag_SNR(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[4];
	CHAR	*value;
	INT 	i;
	UCHAR   ucSNR_STS0, ucSNR_STS1, ucSNR_STS2, ucSNR_STS3;

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 11)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucSNR_STS0 = Input[0];
	ucSNR_STS1 = Input[1];
	ucSNR_STS2 = Input[2];
	ucSNR_STS3 = Input[3];

    TxBfProfileTag_SNR(&pAd->rPfmuTag1,
                        ucSNR_STS0,
                        ucSNR_STS1,
                        ucSNR_STS2,
                        ucSNR_STS3);

	return TRUE;
}


INT Set_TxBfProfileTag_SmartAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   u2SmartAnt;

	u2SmartAnt    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_SmtAnt(&pAd->rPfmuTag2, u2SmartAnt);

	return TRUE;
}


INT Set_TxBfProfileTag_SeIdx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucSeIdx;

	ucSeIdx    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_SeIdx(&pAd->rPfmuTag2, ucSeIdx);

	return TRUE;
}


INT Set_TxBfProfileTag_RmsdThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucRmsdThrd;

	ucRmsdThrd    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_RmsdThd(&pAd->rPfmuTag2, ucRmsdThrd);

	return TRUE;
}


INT Set_TxBfProfileTag_McsThrd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   Input[6];
	CHAR	*value;
	INT 	i;
	UCHAR   ucMcsLss[3], ucMcsSss[3];

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 17)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	ucMcsLss[0] = Input[0];
	ucMcsSss[0] = Input[1];
	ucMcsLss[1] = Input[2];
	ucMcsSss[1] = Input[3];
	ucMcsLss[2] = Input[4];
	ucMcsSss[2] = Input[5];

    TxBfProfileTag_McsThd(&pAd->rPfmuTag2,
                           ucMcsLss,
                           ucMcsSss);

	return TRUE;
}


INT Set_TxBfProfileTag_TimeOut(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucTimeOut;

	ucTimeOut    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_TimeOut(&pAd->rPfmuTag2, ucTimeOut);

	return TRUE;
}


INT Set_TxBfProfileTag_DesiredBW(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucDesiredBW;

	ucDesiredBW    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_DesiredBW(&pAd->rPfmuTag2, ucDesiredBW);

	return TRUE;
}


INT Set_TxBfProfileTag_DesiredNc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucDesiredNc;

	ucDesiredNc    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_DesiredNc(&pAd->rPfmuTag2, ucDesiredNc);

	return TRUE;
}


INT Set_TxBfProfileTag_DesiredNr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT   ucDesiredNr;

	ucDesiredNr    = simple_strtol(arg, 0, 10);
	TxBfProfileTag_DesiredNr(&pAd->rPfmuTag2, ucDesiredNr);

	return TRUE;
}


INT Set_TxBfProfileTagRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	BOOLEAN fgBFer;
	UCHAR   Input[2];
	CHAR	*value;
	INT 	i;

	os_zero_mem(Input, sizeof(Input));

	if(strlen(arg) != 5)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	profileIdx = Input[0];
	fgBFer     = Input[1];

	return TxBfProfileTagRead(pAd, profileIdx, fgBFer);
}


INT Set_TxBfProfileTagWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;

	profileIdx = simple_strtol(arg, 0, 10);

	return TxBfProfileTagWrite(pAd,
	                           &pAd->rPfmuTag1,
	                           &pAd->rPfmuTag2,
	                           profileIdx);
}


INT Set_TxBfProfileDataRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR	*value;
    UCHAR   Input[4];
    INT 	i;
    UCHAR   profileIdx, subcarrIdx_H, subcarrIdx_L;
    BOOLEAN fgBFer;
    USHORT  subcarrIdx;

	os_zero_mem(Input, sizeof(Input));

	/* Profile Select : Subcarrier Select */
	if(strlen(arg) != 11)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		AtoH(value, &Input[i++], 1);
	}

	profileIdx   = Input[0];
	fgBFer       = Input[1];
	subcarrIdx_H = Input[2];
	subcarrIdx_L = Input[3];

	subcarrIdx = ((USHORT)(subcarrIdx_H << 8) | (USHORT)subcarrIdx_L);

	return TxBfProfileDataRead(pAd, profileIdx, fgBFer, subcarrIdx);
}


INT Set_TxBfProfileDataWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	USHORT  subcarrierIdx;
	USHORT  Input[18];
	CHAR	*value, value_T[12], onebyte;
	UCHAR   strLen;
	INT 	i;
	PFMU_DATA rPfmuProfileData;
	PUCHAR  pProfile;

	os_zero_mem(Input, 36);

	/* Profile Select : Subcarrier Select */
	if(strlen(arg) != 60)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++) {
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

		strLen=strlen(value);
		if (strLen & 1) {
		    strcpy(value_T, "0");
		    strcat(value_T, value);
		    AtoH(value_T, (PCHAR)(&Input[i]), 2);
		    Input[i] = be2cpu16(Input[i]);
		} else if (strLen == 2) {
		    AtoH(value, (PCHAR)(&onebyte), 1);
		    Input[i] = ((USHORT)onebyte) & ((USHORT)0x00FF);
		} else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:Error: Un-expected string len!!!!!\n", __FUNCTION__));
	}

    profileIdx    = Input[0];
    subcarrierIdx = Input[1];
	rPfmuProfileData.rField.u2Phi11  = Input[2];
	rPfmuProfileData.rField.ucPsi21  = Input[3];
	rPfmuProfileData.rField.u2Phi21  = Input[4];
	rPfmuProfileData.rField.ucPsi31  = Input[5];
	rPfmuProfileData.rField.u2Phi31  = Input[6];
	rPfmuProfileData.rField.ucPsi41  = Input[7];
	rPfmuProfileData.rField.u2Phi22  = Input[8];
	rPfmuProfileData.rField.ucPsi32  = Input[9];
	rPfmuProfileData.rField.u2Phi32  = Input[10];
	rPfmuProfileData.rField.ucPsi42  = Input[11];
	rPfmuProfileData.rField.u2Phi33  = Input[12];
	rPfmuProfileData.rField.ucPsi43  = Input[13];
	rPfmuProfileData.rField.u2dSNR00 = Input[14];
	rPfmuProfileData.rField.u2dSNR01 = Input[15];
	rPfmuProfileData.rField.u2dSNR02 = Input[16];
	rPfmuProfileData.rField.u2dSNR03 = Input[17];
    pProfile = (PUCHAR)&rPfmuProfileData;
#ifdef RT_BIG_ENDIAN
    RTMPEndianChange(pProfile,sizeof(PFMU_DATA));
#endif
    return TxBfProfileDataWrite(pAd, profileIdx, subcarrierIdx, pProfile);
}


INT Set_TxBfProfilePnRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;

	profileIdx = simple_strtol(arg, 0, 10);

	return TxBfProfilePnRead(pAd, profileIdx);
}


INT Set_TxBfProfilePnWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR   profileIdx;
	UCHAR   ucBw;
    CHAR    *value, value_T[12], onebyte;
    UCHAR   strLen;
    SHORT   Input[14];
	INT 	status, i;
    PFMU_PN rPfmuPn;
    PFMU_PN_DBW80_80M rPfmuPn160M;
    PUCHAR  pPfmuPn;

	os_zero_mem(Input, sizeof(Input));

	/* Profile Select : Subcarrier Select */
    if(strlen(arg) != 55)
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++) {
		if((!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid*/

	  strLen=strlen(value);
	  if (strLen & 1) {
	      strcpy(value_T, "0");
	      strcat(value_T, value);
	      AtoH(value_T, (PCHAR)(&Input[i]), 2);
	      Input[i] = be2cpu16(Input[i]);
	  } else if (strLen == 2) {
	      AtoH(value, (PCHAR)(&onebyte), 1);
	      Input[i] = ((USHORT)onebyte) & ((USHORT)0x00FF);
	  } else
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s:Error: Un-expected string len!!!!!\n", __FUNCTION__));
	}

	profileIdx    = Input[0];
	ucBw          = Input[1];

	if (ucBw != P_DBW160M)
	{
	    rPfmuPn.rField.u2CMM_1STS_Tx0    = Input[2];
        rPfmuPn.rField.u2CMM_1STS_Tx1    = Input[3];
        rPfmuPn.rField.u2CMM_1STS_Tx2    = Input[4] & 0x3FF;
        rPfmuPn.rField.u2CMM_1STS_Tx2Msb = Input[4] >> 11;
        rPfmuPn.rField.u2CMM_1STS_Tx3    = Input[5];
        rPfmuPn.rField.u2CMM_2STS_Tx0    = Input[6];
        rPfmuPn.rField.u2CMM_2STS_Tx1    = Input[7] & 0x1FF;
        rPfmuPn.rField.u2CMM_2STS_Tx1Msb = Input[7] >> 10;
        rPfmuPn.rField.u2CMM_2STS_Tx2    = Input[8];
        rPfmuPn.rField.u2CMM_2STS_Tx3    = Input[9];
        rPfmuPn.rField.u2CMM_3STS_Tx0    = Input[10] & 0x0FF;
        rPfmuPn.rField.u2CMM_3STS_Tx0Msb = Input[10] >> 9;
        rPfmuPn.rField.u2CMM_3STS_Tx1    = Input[11];
        rPfmuPn.rField.u2CMM_3STS_Tx2    = Input[12];
        rPfmuPn.rField.u2CMM_3STS_Tx3    = Input[13] & 0x07F;
        rPfmuPn.rField.u2CMM_3STS_Tx3Msb = Input[13] >> 8;

        pPfmuPn = (PUCHAR) (&rPfmuPn);
        status = TxBfProfilePnWrite(pAd, profileIdx, ucBw, pPfmuPn);
	}
	else
	{
	    rPfmuPn160M.rField.u2DBW160_1STS_Tx0    = Input[2];
        rPfmuPn160M.rField.u2DBW160_1STS_Tx1    = Input[3];
        rPfmuPn160M.rField.u2DBW160_2STS_Tx0    = Input[4] & 0x3FF;
        rPfmuPn160M.rField.u2DBW160_2STS_Tx0Msb = Input[4] >> 11;
        rPfmuPn160M.rField.u2DBW160_2STS_Tx1    = Input[5];

        pPfmuPn = (PUCHAR) (&rPfmuPn160M);
        status = TxBfProfilePnWrite(pAd, profileIdx, ucBw, pPfmuPn);
	}

    return status;
}
#endif  //TXBF_SUPPORT
#endif  //MT_MAC

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
INT Set_WifiFwd_Proc(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __FUNCTION__, active));

	if (active == 0) {
		if (wf_fwd_pro_halt_hook)
			wf_fwd_pro_halt_hook();
	} else  {
		if (wf_fwd_pro_active_hook)
			wf_fwd_pro_active_hook();
	}

	return TRUE;
}

INT WifiFwdSet(
	IN int disabled)
{
	if (disabled != 0) {
		if (wf_fwd_pro_disabled_hook)
			wf_fwd_pro_disabled_hook();
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::disabled=%d\n", __FUNCTION__, disabled));
	
	return TRUE;
}

INT Set_WifiFwd_Down(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int disable = simple_strtol(arg, 0, 10);

	WifiFwdSet(disable);
	return TRUE;
}

INT Set_WifiFwdAccessSchedule_Proc(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = simple_strtol(arg, 0, 10);
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __FUNCTION__, active));

	if (active == 0) {
		if (wf_fwd_access_schedule_halt_hook)
			wf_fwd_access_schedule_halt_hook();
	} else  {
		if (wf_fwd_access_schedule_active_hook)
			wf_fwd_access_schedule_active_hook();
	}
	
	return TRUE;
}

INT Set_WifiFwdHijack_Proc(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = simple_strtol(arg, 0, 10);
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __FUNCTION__, active));

	if (active == 0) {
		if (wf_fwd_hijack_halt_hook)
			wf_fwd_hijack_halt_hook();
	} else  {
		if (wf_fwd_hijack_active_hook)
			wf_fwd_hijack_active_hook();
	}
	
	return TRUE;
}

INT Set_WifiFwdBpdu_Proc(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int active = simple_strtol(arg, 0, 10);
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::active=%d\n", __FUNCTION__, active));

	if (active == 0) {
		if (wf_fwd_bpdu_halt_hook)
			wf_fwd_bpdu_halt_hook();
	} else  {
		if (wf_fwd_bpdu_active_hook)
			wf_fwd_bpdu_active_hook();
	}
	
	return TRUE;
}

INT Set_WifiFwdRepDevice(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int rep = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::rep=%d\n", __FUNCTION__, rep));

	if (wf_fwd_get_rep_hook)
		wf_fwd_get_rep_hook(rep);

	return TRUE;
}

INT Set_WifiFwdShowEntry(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (wf_fwd_show_entry_hook)
		wf_fwd_show_entry_hook();

	return TRUE;
}

INT Set_WifiFwdDeleteEntry(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int idx = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::idx=%d\n", __FUNCTION__, idx));
	
	if (wf_fwd_delete_entry_hook)
		wf_fwd_delete_entry_hook(idx);

	return TRUE;
}

INT Set_PacketSourceShowEntry(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (packet_source_show_entry_hook)
		packet_source_show_entry_hook();

	return TRUE;
}

INT Set_PacketSourceDeleteEntry(
    IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	int idx = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::idx=%d\n", __FUNCTION__, idx));

	if (packet_source_delete_entry_hook)
		packet_source_delete_entry_hook(idx);

	return TRUE;
}
#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef DOT11_N_SUPPORT
void assoc_ht_info_debugshow(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR ht_cap_len,
	IN HT_CAPABILITY_IE *pHTCapability)
{
	HT_CAP_INFO			*pHTCap;
	HT_CAP_PARM		*pHTCapParm;
	EXT_HT_CAP_INFO		*pExtHT;
#ifdef TXBF_SUPPORT
	HT_BF_CAP			*pBFCap;
#endif /* TXBF_SUPPORT */


	if (pHTCapability && (ht_cap_len > 0))
	{
		pHTCap = &pHTCapability->HtCapInfo;
		pHTCapParm = &pHTCapability->HtCapParm;
		pExtHT = &pHTCapability->ExtHtCapInfo;
#ifdef TXBF_SUPPORT
		pBFCap = &pHTCapability->TxBFCap;
#endif /* TXBF_SUPPORT */

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Peer - 11n HT Info\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tHT Cap Info: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t HT_RX_LDPC(%d), BW(%d), MIMOPS(%d), GF(%d), ShortGI_20(%d), ShortGI_40(%d)\n",
			pHTCap->ht_rx_ldpc, pHTCap->ChannelWidth, pHTCap->MimoPs, pHTCap->GF,
			pHTCap->ShortGIfor20, pHTCap->ShortGIfor40));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t TxSTBC(%d), RxSTBC(%d), DelayedBA(%d), A-MSDU(%d), CCK_40(%d)\n",
			pHTCap->TxSTBC, pHTCap->RxSTBC, pHTCap->DelayedBA, pHTCap->AMsduSize, pHTCap->CCKmodein40));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t PSMP(%d), Forty_Mhz_Intolerant(%d), L-SIG(%d)\n",
			pHTCap->PSMP, pHTCap->Forty_Mhz_Intolerant, pHTCap->LSIGTxopProSup));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tHT Parm Info: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t MaxRx A-MPDU Factor(%d), MPDU Density(%d)\n",
			pHTCapParm->MaxRAmpduFactor, pHTCapParm->MpduDensity));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tHT MCS set: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t RxMCS(%02x %02x %02x %02x %02x) MaxRxMbps(%d) TxMCSSetDef(%02x)\n",
			pHTCapability->MCSSet[0], pHTCapability->MCSSet[1], pHTCapability->MCSSet[2],
			pHTCapability->MCSSet[3], pHTCapability->MCSSet[4],
			(pHTCapability->MCSSet[11]<<8) + pHTCapability->MCSSet[10],
			pHTCapability->MCSSet[12]));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tExt HT Cap Info: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t PCO(%d), TransTime(%d), MCSFeedback(%d), +HTC(%d), RDG(%d)\n",
			pExtHT->Pco, pExtHT->TranTime, pExtHT->MCSFeedback, pExtHT->PlusHTC, pExtHT->RDGSupport));

#ifdef TXBF_SUPPORT
        	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tTX BF Cap: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t ImpRxCap(%d), RXStagSnd(%d), TXStagSnd(%d), RxNDP(%d), TxNDP(%d) ImpTxCap(%d)\n",
			pBFCap->TxBFRecCapable, pBFCap->RxSoundCapable, pBFCap->TxSoundCapable,
			pBFCap->RxNDPCapable, pBFCap->TxNDPCapable, pBFCap->ImpTxBFCapable));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t Calibration(%d), ExpCSICapable(%d), ExpComSteerCapable(%d), ExpCSIFbk(%d), ExpNoComBF(%d) ExpComBF(%d)\n",
			pBFCap->Calibration, pBFCap->ExpCSICapable, pBFCap->ExpComSteerCapable,
			pBFCap->ExpCSIFbk, pBFCap->ExpNoComBF, pBFCap->ExpComBF));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\t MinGrouping(%d), CSIBFAntSup(%d), NoComSteerBFAntSup(%d), ComSteerBFAntSup(%d), CSIRowBFSup(%d) ChanEstimation(%d)\n",
			pBFCap->MinGrouping, pBFCap->CSIBFAntSup, pBFCap->NoComSteerBFAntSup,
			pBFCap->ComSteerBFAntSup, pBFCap->CSIRowBFSup, pBFCap->ChanEstimation));
#endif /* TXBF_SUPPORT */

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nPeer - MODE=%d, BW=%d, MCS=%d, ShortGI=%d, MaxRxFactor=%d, MpduDensity=%d, MIMOPS=%d, AMSDU=%d\n",
			pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.BW,
			pEntry->HTPhyMode.field.MCS, pEntry->HTPhyMode.field.ShortGI,
			pEntry->MaxRAmpduFactor, pEntry->MpduDensity,
			pEntry->MmpsMode, pEntry->AMsduSize));

#ifdef DOT11N_DRAFT3
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tExt Cap Info: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tBss2040CoexistMgmt=%d\n", pEntry->BSS2040CoexistenceMgmtSupport));
#endif /* DOT11N_DRAFT3 */
	}
}


INT	Set_BurstMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	pAd->CommonCfg.bRalinkBurstMode = ((Value == 1) ? TRUE : FALSE);

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_BurstMode_Proc ::%s\n",
				(pAd->CommonCfg.bRalinkBurstMode == TRUE) ? "enabled" : "disabled"));

	return TRUE;
}
#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
VOID assoc_vht_info_debugshow(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE *vht_op)
{
	VHT_CAP_INFO *cap_info;
	VHT_MCS_SET *mcs_set;
	VHT_OP_INFO *op_info;
	VHT_MCS_MAP *mcs_map;
	UCHAR PhyMode = HcGetPhyModeByRf(pAd,RFIC_5GHZ);

	if ( !WMODE_CAP_AC(PhyMode))
		return;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Peer - 11AC VHT Info\n"));
	if (vht_cap)
	{
		cap_info = &vht_cap->vht_cap;
		mcs_set = &vht_cap->mcs_set;

		hex_dump("peer vht_cap raw data", (UCHAR *)cap_info, sizeof(VHT_CAP_INFO));
		hex_dump("peer vht_mcs raw data", (UCHAR *)mcs_set, sizeof(VHT_MCS_SET));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tVHT Cap Info: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tMaxMpduLen(%d), BW(%d), SGI_80M(%d), RxLDPC(%d), TxSTBC(%d), RxSTBC(%d), +HTC-VHT(%d)\n",
				cap_info->max_mpdu_len, cap_info->ch_width, cap_info->sgi_80M, cap_info->rx_ldpc, cap_info->tx_stbc,
				cap_info->rx_stbc, cap_info->htc_vht_cap));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tMaxAmpduExp(%d), VhtLinkAdapt(%d), RxAntConsist(%d), TxAntConsist(%d)\n",
				cap_info->max_ampdu_exp, cap_info->vht_link_adapt, cap_info->rx_ant_consistency, cap_info->tx_ant_consistency));
		mcs_map = &mcs_set->rx_mcs_map;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tRxMcsSet: HighRate(%d), RxMCSMap(%d,%d,%d,%d,%d,%d,%d)\n",
			mcs_set->rx_high_rate, mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
			mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6, mcs_map->mcs_ss7));
		mcs_map = &mcs_set->tx_mcs_map;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tTxMcsSet: HighRate(%d), TxMcsMap(%d,%d,%d,%d,%d,%d,%d)\n",
			mcs_set->tx_high_rate, mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
			mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6, mcs_map->mcs_ss7));
#ifdef VHT_TXBF_SUPPORT
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tETxBfCap: Bfer(%d), Bfee(%d), SndDim(%d)\n",
			cap_info->bfer_cap_su, cap_info->bfee_cap_su, cap_info->num_snd_dimension));
#endif
	}

	if (vht_op)
	{
		op_info = &vht_op->vht_op_info;
		mcs_map = &vht_op->basic_mcs_set;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tVHT OP Info: \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tChannel Width(%d), CenteralFreq1(%d), CenteralFreq2(%d)\n",
			op_info->ch_width, op_info->center_freq_1, op_info->center_freq_2));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\tBasicMCSSet(SS1:%d, SS2:%d, SS3:%d, SS4:%d, SS5:%d, SS6:%d, SS7:%d)\n",
			mcs_map->mcs_ss1, mcs_map->mcs_ss2, mcs_map->mcs_ss3,
			mcs_map->mcs_ss4, mcs_map->mcs_ss5, mcs_map->mcs_ss6,
			mcs_map->mcs_ss7));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));

}
#endif /* DOT11_VHT_AC */


INT Set_RateAdaptInterval(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ra_time, ra_qtime;
	RTMP_STRING *token;
	char sep = ':';
	ULONG irqFlags;

/*
	The ra_interval inupt string format should be d:d, in units of ms.
		=>The first decimal number indicates the rate adaptation checking period,
		=>The second decimal number indicates the rate adaptation quick response checking period.
*/
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s():%s\n", __FUNCTION__, arg));

	token = strchr(arg, sep);
	if (token != NULL)
	{
		*token = '\0';

		if (strlen(arg) && strlen(token+1))
		{
			ra_time = simple_strtol(arg, 0, 10);
			ra_qtime = simple_strtol(token+1, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Set RateAdaptation TimeInterval as(%d:%d) ms\n",
						__FUNCTION__, ra_time, ra_qtime));

			RTMP_IRQ_LOCK(&pAd->irq_lock, irqFlags);
			pAd->ra_interval = ra_time;
			pAd->ra_fast_interval = ra_qtime;
#ifdef CONFIG_AP_SUPPORT
			if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE)
			{
				BOOLEAN Cancelled;

				RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);
				pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
			}
#endif /* CONFIG_AP_SUPPORT  */
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqFlags);
			return TRUE;
		}
	}

	return FALSE;

}


INT Set_VcoPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->chipCap.VcoPeriod = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("VCO Period = %d seconds\n", pAd->chipCap.VcoPeriod));
	return TRUE;
}


#ifdef SINGLE_SKU
INT Set_ModuleTxpower_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 Value;

	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Do NOT accept this command after interface is up.\n"));
		return FALSE;
	}

	Value = (UINT16)simple_strtol(arg, 0, 10);
	pAd->CommonCfg.ModuleTxpower = Value;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF Set_ModuleTxpower_Proc::(ModuleTxpower=%d)\n", pAd->CommonCfg.ModuleTxpower));
	return TRUE;
}
#endif /* SINGLE_SKU */


#ifdef CONFIG_FPGA_MODE

#ifdef CAPTURE_MODE
INT set_cap_dump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG seg = simple_strtol(arg, 0, 10);;
	CHAR *buf1, *buf2;
	UINT32 offset = 0;

	seg = ((seg > 0 && seg <= 4)  ? seg : 1);
	if (pAd->fpga_ctl.cap_done == TRUE && (pAd->fpga_ctl.cap_buf != NULL)) {
		switch (seg)
		{
			case 1:
				offset = 0;
				break;
			case 2:
				offset = 0x2000;
				break;
			case 3:
				offset = 0x4000;
				break;
			case 4:
				offset = 0x6000;
				break;
		}
		cap_dump(pAd,
					(pAd->fpga_ctl.cap_buf + offset),
					(pAd->fpga_ctl.cap_buf + 0x8000 + offset),
					0x2000);
	}

	return TRUE;
}


INT set_cap_start(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG cap_start;
	BOOLEAN do_cap;
	BOOLEAN cap_done;	/* 1: capture done, 0: capture not finish yet */


	cap_start = simple_strtol(arg, 0, 10);
	do_cap = cap_start == 1 ? TRUE : FALSE;

	if (!pAd->fpga_ctl.cap_support) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): cap mode is not support yet!\n", __FUNCTION__));
		return FALSE;
	}

	if (do_cap)
	{
		/*
			start to do cap,
			if auto 	=>will triggered depends on trigger condition,
			if manual =>start immediately
		*/
		if (pAd->fpga_ctl.do_cap == FALSE)
			asic_cap_start(pAd);
		else
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): alreay in captureing\n", __FUNCTION__));
		}
	}
	else
	{

		if (pAd->fpga_ctl.do_cap == TRUE) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): force stop captureing\n", __FUNCTION__));
			// TODO: force stop capture!
			asic_cap_stop(pAd);
		}
		else
		{

		}
		pAd->fpga_ctl.do_cap = FALSE;
	}
	return TRUE;
}


INT set_cap_trigger_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG trigger_offset;	/* in unit of us */

	trigger_offset = simple_strtol(arg, 0, 10);

	pAd->fpga_ctl.trigger_offset = (UINT32)trigger_offset;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():set trigger_offset=%d\n",
				__FUNCTION__, pAd->fpga_ctl.trigger_offset));

	return TRUE;
}



INT set_cap_trigger(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG trigger;	/* 1: manual trigger, 2: auto trigger */


	trigger = simple_strtol(arg, 0, 10);
	pAd->fpga_ctl.cap_trigger = trigger <= 2 ? trigger : 0;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():set cap_trigger=%s trigger\n", __FUNCTION__,
						(pAd->fpga_ctl.cap_trigger == 0 ? "Invalid" :
						(pAd->fpga_ctl.cap_trigger == 1 ? "Manual" : "Auto"))));

	return TRUE;
}


INT set_cap_type(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG cap_type;			/* 1: ADC6, 2: ADC8, 3: FEQ */

	cap_type = simple_strtol(arg, 0, 10);

	pAd->fpga_ctl.cap_type = cap_type <= 3 ? cap_type : 0;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():set cap_type=%s\n",
				__FUNCTION__,
				pAd->fpga_ctl.cap_type == 1 ? "ADC6" :\
				(pAd->fpga_ctl.cap_type == 2  ? "ADC8" : "FEQ")));

	return TRUE;
}


INT set_cap_support(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR cap_support;	/* 0: no cap mode; 1: cap mode enable */

	cap_support = simple_strtol(arg, 0, 10);
	pAd->fpga_ctl.cap_support = (cap_support == 1 ?  TRUE : FALSE);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():set cap_support=%s\n",
				__FUNCTION__,
				(pAd->fpga_ctl.cap_support == TRUE ? "TRUE" : "FALSE")));

	return TRUE;
}
#endif /* CAPTURE_MODE */


INT set_vco_cal(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR vco_cal;

	vco_cal = simple_strtol(arg, 0, 10);
	pAd->fpga_ctl.vco_cal = vco_cal ? TRUE : FALSE;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): vco_cal=%s\n",
				__FUNCTION__, (pAd->fpga_ctl.vco_cal ? "Enabled" : "Stopped")));

	return TRUE;

}

INT set_tr_stop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR stop;

	stop = simple_strtol(arg, 0, 10);
	pAd->fpga_ctl.fpga_tr_stop = (stop <= 4 ? stop : 0);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): fpga_tr_stop=0x%x\n", __FUNCTION__, pAd->fpga_ctl.fpga_tr_stop));

	return TRUE;
}


INT set_tx_kickcnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fpga_ctl.tx_kick_cnt = (INT)simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():tx_kick_cnt=%d\n", __FUNCTION__, pAd->fpga_ctl.tx_kick_cnt));

	return TRUE;
}


INT set_data_phy_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fpga_ctl.tx_data_phy = (INT)simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): tx_data_phy=%d\n", __FUNCTION__, pAd->fpga_ctl.tx_data_phy));

	return TRUE;
}


INT set_data_bw(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fpga_ctl.tx_data_bw = (UCHAR)simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): tx_data_bw=%d\n", __FUNCTION__, pAd->fpga_ctl.tx_data_bw));

	return TRUE;
}


INT set_data_mcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR DASH = '-';
	RTMP_STRING *mcs_str, *nss_str;
	UINT8 nss, mcs;

	mcs_str = strchr(arg, DASH);
	nss_str = NULL;
	if ((mcs_str != NULL) && (strlen(mcs_str) > 1))
	{
		*mcs_str = '\0';
		mcs_str++;
		if (strlen(arg)> 1)
			nss_str = arg;
	} else {
		mcs_str = arg;
	}

printk("mcs_str=%s, nss_str=%s\n", mcs_str, nss_str);
	if (mcs_str) {
		mcs = (UINT8)simple_strtol(mcs_str, 0, 10);
	pAd->fpga_ctl.tx_data_mcs = mcs;
	}
	if (nss_str) {
		nss = (UINT8)simple_strtol(nss_str, 0, 10);
		pAd->fpga_ctl.tx_data_nss = nss;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): tx_data_mcs=%d, tx_data_nss=%d\n",
				__FUNCTION__, pAd->fpga_ctl.tx_data_mcs, pAd->fpga_ctl.tx_data_nss));

	return TRUE;
}


INT set_data_ldpc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fpga_ctl.tx_data_ldpc = (UCHAR)simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): tx_data_ldpc=%d\n", __FUNCTION__, pAd->fpga_ctl.tx_data_ldpc));

	return TRUE;
}

INT set_data_gi(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fpga_ctl.tx_data_gi = (UCHAR)simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): tx_data_gi=%d\n", __FUNCTION__, pAd->fpga_ctl.tx_data_gi));

	return TRUE;
}


INT set_data_basize(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fpga_ctl.data_basize = (UCHAR)simple_strtol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): data_basize=%d\n", __FUNCTION__, pAd->fpga_ctl.data_basize));

	return TRUE;
}


#ifdef MT_MAC
INT set_txs_report_type(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MT_MAC_TXS_TYPE txs_mode = TXS_NONE;

	if (arg && strlen(arg)) {
		if (strstr(arg, "data") != NULL)
			txs_mode |= TXS_DATA;

		if (strstr(arg, "qdata") != NULL)
			txs_mode |= TXS_QDATA;

		if (strstr(arg, "noqdata") != NULL)
			txs_mode |= TXS_NON_QDATA;

		if (strstr(arg, "mgmt") != NULL)
			txs_mode |= TXS_MGMT;
		if (strstr(arg, "bcn") != NULL)
			txs_mode |= TXS_BCN;

		if (strstr(arg, "mgmt_other") != NULL)
			txs_mode |= TXS_MGMT_OTHER;

		if (strstr(arg, "ctrl") != NULL)
			txs_mode |= TXS_CTRL;

		if (strstr(arg, "all") != NULL)
			txs_mode |= TXS_ALL;
	}

	pAd->fpga_ctl.txs_type = txs_mode;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Set TXS Report type as: 0x%x\n",
				__FUNCTION__, pAd->fpga_ctl.txs_type));

	return TRUE;
}


INT set_no_bcn(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG no_bcn;

	no_bcn = simple_strtol(arg, 0, 10);
	pAd->fpga_ctl.no_bcn = (no_bcn ? 1 : 0);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Set no beacon as:%d\n",
							__FUNCTION__, pAd->fpga_ctl.no_bcn));

	return TRUE;
}
#endif /* MT_MAC */


INT set_fpga_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG fpga_on;
#ifdef CONFIG_AP_SUPPORT
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[0].wdev;
#endif /* CONFIG_AP_SUPPORT */

	fpga_on = simple_strtol(arg, 0, 10);

	if (fpga_on & 2)
		pAd->fpga_ctl.tx_data_mcs = 7;

	if (fpga_on & 4)
		pAd->fpga_ctl.tx_data_mcs = (1 << 4) | 7;

#ifdef CONFIG_AP_SUPPORT
	if (fpga_on & 0x6) {
		pAd->fpga_ctl.tx_data_phy = MODE_HTMIX;
		pAd->fpga_ctl.tx_data_ldpc = 0;
		pAd->fpga_ctl.tx_data_bw = BW_80;
		pAd->fpga_ctl.tx_data_gi = 1;
		pAd->fpga_ctl.data_basize = 31;
		wdev->bAutoTxRateSwitch = FALSE;
	}
	else
	{
		wdev->bAutoTxRateSwitch = TRUE;
	}
#endif /* CONFIG_AP_SUPPORT */

	pAd->fpga_ctl.fpga_on = fpga_on;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): fpga_on=%d\n", __FUNCTION__, pAd->fpga_ctl.fpga_on));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tdata_phy=%d\n", pAd->fpga_ctl.tx_data_phy));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tdata_bw=%d\n", pAd->fpga_ctl.tx_data_bw));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tdata_mcs=%d\n", pAd->fpga_ctl.tx_data_mcs));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tdata_gi=%d\n", pAd->fpga_ctl.tx_data_gi));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tdata_basize=%d\n", pAd->fpga_ctl.data_basize));

#ifdef CONFIG_AP_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tbAutoTxRateSwitch=%d\n",
				wdev->bAutoTxRateSwitch));
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
#endif /* CONFIG_FPGA_MODE */


#ifdef MANUAL_CONNECT
INT set_dev_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	return TRUE;
}


INT set_disassoc_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	return TRUE;
}


INT assoc_proc_type_handle(RTMP_ADAPTER *pAd, RTMP_STRING *type, RTMP_STRING *val)
{

	return TRUE;
}


#define ASSOC_CMD_MAC 1
#define ASSOC_CMD_TYPE 2
#define ASSOC_CMD_MODE 3
#define ASSOC_CMD_BW 4
#define ASSOC_CMD_RATE 5
INT manual_parsing_param(RTMP_ADAPTER *pAd, RTMP_STRING *type, RTMP_STRING *val)
{
	UINT8 mac[MAC_ADDR_LEN] = {0};
	INT op_type = 0;
	INT wtbl_idx = 1;
	INT own_mac_idx = 0;
	INT phy_mode = 0;
	INT bw = BW_20;
	INT nss = 1;
	INT maxrate_mode = MODE_CCK;
	INT maxrate_mcs = 0;
	INT pfmuId = 0;
	INT aid = 0;
	UINT8 fgIsSuBFee = 0;
	UINT8 fgIsMuBFee = 0;
	UINT8 fgIsSGIFor20 = 0;
	UINT8 fgIsSGIFor40 = 0;
	UINT8 fgIsSGIFor80 = 0;
	UINT8 fgIsSGIFor160 = 0;
	UINT8 bFeeNsts = 0;
	UINT8 mcsSupport = 0;

	if ((!type) || (!val))
		return FALSE;

	/* mac:xx:xx:xx:xx:xx:xx */
	if (strcmp("mac", type) == 0)
	{
		if (mac_str2hex(val, &mac[0]) == FALSE) {
			NdisZeroMemory(&mac[0], MAC_ADDR_LEN);
			printk("\t%s(): Invalid MAC address(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		} else {
			printk("\t%s(): Invalid MAC address(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		NdisMoveMemory(&pAd->fpga_ctl.manual_conn_info.peer_mac[0], mac, MAC_ADDR_LEN);
		printk("%s(): MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	/* type:ap/sta */
	if (strcmp("type", type) == 0)
	{
		if (strcmp(val, "ap") == 0) {
			op_type = OPMODE_AP;
		}
		else if (strcmp(val, "sta") == 0)
		{
			op_type = OPMODE_STA;
		}
		else
		{
			printk("\t%s(): Invalid type(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		pAd->fpga_ctl.manual_conn_info.peer_op_type = op_type;
		printk("%s(): TYPE=%d\n", __FUNCTION__, op_type);
	}

	/* wtbl:1~127 */
	if (strcmp("wtbl", type) == 0) {
		if (strlen(val)) {
			wtbl_idx = simple_strtol(val, 0, 10);
			if (wtbl_idx <= 0 || wtbl_idx > 127) {
				printk("\t%s(): Invalid wtbl idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				wtbl_idx = 1;
			}
		} else {
			printk("\t%s(): Invalid wtbl idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}
		pAd->fpga_ctl.manual_conn_info.wtbl_idx = wtbl_idx;
		printk("%s(): WTBL_IDX=%d\n", __FUNCTION__, wtbl_idx);
	}

	/* ownmac:0~4, 0x10~0x1f */
	if (strcmp("ownmac", type)  == 0) {
		if (strlen(val)) {
			own_mac_idx = simple_strtol(val, 0, 10);
			if (!((own_mac_idx >= 0 || own_mac_idx <= 4) || (own_mac_idx >= 0x10 || own_mac_idx <= 0x1f)))
			{
				printk("\t%s(): Invalid OwnMac idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				own_mac_idx = 1;
			}
		} else {
			printk("\t%s(): Invalid wtbl idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}
		pAd->fpga_ctl.manual_conn_info.ownmac_idx = own_mac_idx;
		printk("%s(): OWN_MAC_IDX=%d\n", __FUNCTION__, own_mac_idx);
	}

	/* pfmuId: */
	if (strcmp("pfmuId", type)  == 0) {
		if (strlen(val)) {
			pfmuId = simple_strtol(val, 0, 10);
			if (!(pfmuId >= 0x00 || pfmuId <= 0x3f))
			{
				printk("\t%s(): Invalid PFMU idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				pfmuId = 0;
			}
		} else {
			printk("\t%s(): Invalid PFMU idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}
		pAd->fpga_ctl.manual_conn_info.pfmuId = pfmuId;
		printk("%s(): PFMU_IDX=%d\n", __FUNCTION__, pfmuId);
	}

	/* aid: */
	if (strcmp("aid", type) == 0) {
		if (strlen(val)) {
			aid = simple_strtol(val, 0, 10);
			if (!(aid >= 0x00 || aid <= 2007))
			{
				printk("\t%s(): Invalid aid(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				aid = 0;
			}
		} else {
			printk("\t%s(): Invalid aid(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}
		pAd->fpga_ctl.manual_conn_info.aid = aid;
		printk("%s(): AID =%d\n", __FUNCTION__, aid);
	}

	if (strcmp("mubfee", type) == 0) {
		if (strlen(val)) {
			fgIsMuBFee = simple_strtol(val, 0, 10);
			if (!(fgIsMuBFee == 0 || fgIsMuBFee == 1))
			{
				printk("\t%s(): Invalid mubfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				fgIsMuBFee = 0;
			}
		} else {
			printk("\t%s(): Invalid mubfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		if (fgIsMuBFee) {
			pAd->fpga_ctl.manual_conn_info.vht_cap_info.bfee_cap_mu = fgIsMuBFee;
		}

		printk("%s(): mubfee =%d\n", __FUNCTION__, fgIsMuBFee);
	}

	if (strcmp("sgi160", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor160 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor160 == 0 || fgIsSGIFor160 == 1))
			{
				printk("\t%s(): Invalid sgi160(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				fgIsSGIFor160 = 0;
			}
		} else {
			printk("\t%s(): Invalid sgi160(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		if (fgIsSGIFor160) {
			pAd->fpga_ctl.manual_conn_info.vht_cap_info.sgi_160M = fgIsSGIFor160;
		}

		printk("%s(): sgi160 =%d\n", __FUNCTION__, fgIsSGIFor160);
	}

	if (strcmp("sgi80", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor80 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor80 == 0 || fgIsSGIFor80 == 1))
			{
				printk("\t%s(): Invalid sgi80(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				fgIsSGIFor80 = 0;
			}
		} else {
			printk("\t%s(): Invalid sgi80(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		if (fgIsSGIFor80) {
			pAd->fpga_ctl.manual_conn_info.vht_cap_info.sgi_80M = fgIsSGIFor80;
		}

		printk("%s(): sgi80 =%d\n", __FUNCTION__, fgIsSGIFor80);
	}

	if (strcmp("sgi40", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor40 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor40 == 0 || fgIsSGIFor40 == 1))
			{
				printk("\t%s(): Invalid sgi40(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				fgIsSGIFor40 = 0;
			}
		} else {
			printk("\t%s(): Invalid sgi40(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		if (fgIsSGIFor40) {
			pAd->fpga_ctl.manual_conn_info.ht_cap_info.ShortGIfor40 = fgIsSGIFor40;
		}

		printk("%s(): sgi40 =%d\n", __FUNCTION__, fgIsSGIFor40);
	}

	if (strcmp("sgi20", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor20 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor20 == 0 || fgIsSGIFor20 == 1))
			{
				printk("\t%s(): Invalid sgi20(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				fgIsSGIFor20 = 0;
			}
		} else {
			printk("\t%s(): Invalid sgi20(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		if (fgIsSGIFor20) {
			pAd->fpga_ctl.manual_conn_info.ht_cap_info.ShortGIfor20 = fgIsSGIFor20;
		}

		printk("%s(): sgi20 =%d\n", __FUNCTION__, fgIsSGIFor20);
	}

	if (strcmp("rxmcsnss1", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				printk("\t%s(): Invalid rxmcsnss1(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			printk("\t%s(): Invalid rxmcsnss1(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		pAd->fpga_ctl.manual_conn_info.vht_mcs_set.rx_mcs_map.mcs_ss1 = mcsSupport;

		printk("%s(): rxmcsnss1 =%d\n", __FUNCTION__, mcsSupport);
	}

	if (strcmp("rxmcsnss2", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				printk("\t%s(): Invalid rxmcsnss2(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			printk("\t%s(): Invalid rxmcsnss2(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		pAd->fpga_ctl.manual_conn_info.vht_mcs_set.rx_mcs_map.mcs_ss2 = mcsSupport;

		printk("%s(): rxmcsnss2 =%d\n", __FUNCTION__, mcsSupport);
	}

	if (strcmp("rxmcsnss3", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				printk("\t%s(): Invalid rxmcsnss3(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			printk("\t%s(): Invalid rxmcsnss3(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		pAd->fpga_ctl.manual_conn_info.vht_mcs_set.rx_mcs_map.mcs_ss3 = mcsSupport;

		printk("%s(): rxMcsNSS3 =%d\n", __FUNCTION__, mcsSupport);
	}

	if (strcmp("rxmcsnss4", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				printk("\t%s(): Invalid rxmcsnss4(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			printk("\t%s(): Invalid rxmcsnss4(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		pAd->fpga_ctl.manual_conn_info.vht_mcs_set.rx_mcs_map.mcs_ss4 = mcsSupport;

		printk("%s(): rxmcsnss4 =%d\n", __FUNCTION__, mcsSupport);
	}

	if (strcmp("subfee", type) == 0) {
		if (strlen(val)) {
			fgIsSuBFee = simple_strtol(val, 0, 10);
			if (!(fgIsSuBFee == 0 || fgIsSuBFee == 1))
			{
				printk("\t%s(): Invalid subfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				fgIsSuBFee = 0;
			}
		} else {
			printk("\t%s(): Invalid subfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		if (fgIsSuBFee) {
			pAd->fpga_ctl.manual_conn_info.vht_cap_info.bfee_cap_su = fgIsSuBFee;
		}

		printk("%s(): subfee =%d\n", __FUNCTION__, fgIsSuBFee);
	}

	if (strcmp("bfeensts", type) == 0) {
		if (strlen(val)) {
			bFeeNsts = simple_strtol(val, 0, 10);
			if (!(bFeeNsts >= 0 || bFeeNsts < 4))
			{
				printk("\t%s(): Invalid bfeensts(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
				bFeeNsts = 4;
			}
		} else {
			printk("\t%s(): Invalid bfeensts(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val));
		}

		pAd->fpga_ctl.manual_conn_info.vht_cap_info.bfee_sts_cap = bFeeNsts;

		printk("%s(): bfeensts =%d\n", __FUNCTION__, bFeeNsts);
	}

	/* mode:a/bg/n/ac */
	if (strcmp("mode", type) == 0) {
		RTMP_STRING *tok;

		tok = val;
		while(strlen(tok)) {
			if (*tok == 'b')
			{
				phy_mode |= WMODE_B;
				tok++;
			}
			else if (*tok == 'g')
			{
				if ((*(tok+1) == 'n') && (strlen(tok) >=2)) {
					phy_mode |= WMODE_GN;
					tok += 2;
				} else {
					phy_mode |= WMODE_G;
					tok += 1;
				}
			}
			else if (*tok == 'a')
			{
				if ((*(tok+1) == 'n') && (strlen(tok) >=2)) {
					phy_mode |= WMODE_AN;
					tok += 2;
				} else if ((*(tok+1) == 'c') && (strlen(tok) >=2)) {
					phy_mode |= WMODE_AC;
					tok += 2;
				} else {
					phy_mode |= WMODE_A;
					tok += 1;
				}
			} else {
				printk("\t%s(): Invalid phy_mode %c\n", __FUNCTION__, *tok);
				tok++;
			}
		}

		pAd->fpga_ctl.manual_conn_info.peer_phy_mode = phy_mode;
		printk("%s(): phy_mode=%s, convert to PhyMode= 0x%x\n",
					__FUNCTION__, (val == NULL ? "" : val), phy_mode);
	}

	/* bw:20/40/80/160 */
	if (strcmp("bw", type) == 0) {
		if (strlen(val)) {
			bw = simple_strtol(val, 0, 10);
			switch (bw) {
				case 20:
					bw = BW_20;
					break;
				case 40:
					bw = BW_40;
					break;
				case 80:
					bw = BW_80;
					break;
				case 160:
					bw = BW_160;
					break;
				default:
					bw = BW_20;
					break;
			}
		}
		else {
			printk("\t%s(): Invalid BW string(%s), use default!\n",
					__FUNCTION__, (val == NULL ? "" : val));
		}

		pAd->fpga_ctl.manual_conn_info.peer_bw = bw;
		printk("%s(): BW=%d\n", __FUNCTION__, bw);
	}

	if (strcmp("nss", type) == 0) {
		if (strlen(val)) {
			UINT8 max_tx_path;

			if(pAd->CommonCfg.dbdc_mode) {
				if (WMODE_CAP_2G(phy_mode)) {
					max_tx_path = pAd->dbdc_2G_tx_stream;
				} else {
					max_tx_path = pAd->dbdc_5G_tx_stream;
				}
			} else {
				max_tx_path = pAd->Antenna.field.TxPath;
			}

			nss = simple_strtol(val, 0, 10);
			if (nss > max_tx_path) {
				printk("\t%s(): Invalid NSS string(%s), use default!\n",
						__FUNCTION__, (val == NULL ? "" : val));
				nss = 1;
			}

		}
		else {
			printk("\t%s(): Invalid NSS setting, use default!\n", __FUNCTION__);
		}

		pAd->fpga_ctl.manual_conn_info.peer_nss = nss;
		printk("%s(): NSS=%d\n", __FUNCTION__, nss);
	}

	/* maxrate:cck/ofdm/htmix/htgf/vht/_0~32 */
	if (strcmp("maxrate", type) == 0) {
		RTMP_STRING *tok;

		if (strlen(val)) {
			tok = rtstrchr(val, '_');
			if (tok && strlen(tok) > 1) {
				*tok = 0;
				tok++;
			} else {
				printk("\t%s(): Invalid maxmcs setting(%s), use default!\n",
						__FUNCTION__, (val == NULL ? "" : val));
				goto maxrate_final;
			}
		} else {
				printk("\t%s(): Invalid maxrate setting(%s), use default!\n",
						__FUNCTION__, (val == NULL ? "" : val));
				goto maxrate_final;
		}

		if (strlen(tok)) {
			maxrate_mcs = simple_strtol(tok, 0, 10);
			printk("\t%s(): input MCS string(%s) =%d\n",
					__FUNCTION__, tok, maxrate_mcs);
		}

		if (strcmp(val, "cck") == 0)
		{
			maxrate_mode = MODE_CCK;
			if (maxrate_mcs > 4)
				maxrate_mcs = 3;
		}
		else if (strcmp(val, "ofdm") == 0)
		{
			maxrate_mode = MODE_OFDM;
			if (maxrate_mcs > 7)
				maxrate_mcs = 7;
		}
		else if (strcmp(val, "htmix") == 0)
		{
			maxrate_mode = MODE_HTMIX;
			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		}
		else if (strcmp(val, "htgf") == 0)
		{
			maxrate_mode = MODE_HTGREENFIELD;
			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		}
		else if (strcmp(val, "vht") == 0)
		{
			maxrate_mode = MODE_VHT;
			if (maxrate_mcs > 9)
				maxrate_mcs = 9;
		}
		else
		{
			printk("%s(): Invalid RateMode string(%s), use default!\n",
						__FUNCTION__, val);
			maxrate_mode = MODE_CCK;
			maxrate_mcs = 0;
		}

maxrate_final:
		pAd->fpga_ctl.manual_conn_info.peer_maxrate_mode = maxrate_mode;
		pAd->fpga_ctl.manual_conn_info.peer_maxrate_mcs = maxrate_mcs;

		printk("%s(): MAXRATE=>MODE=%d,MCS=%d\n",
				__FUNCTION__, maxrate_mode, maxrate_mcs);
	}

	return TRUE;
}

INT apply_sta_to_mac_tbl_entry(RTMP_ADAPTER *pAd)
{
	struct manual_conn *pManual_cfg = &pAd->fpga_ctl.manual_conn_info;
	UCHAR WCID = pManual_cfg->wtbl_idx;
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[WCID];

	/* Currently, for MU-MIMO, we only care the VHT/HT Cap Info and VHT MCS set */

	os_move_mem(&pEntry->vht_cap_ie.vht_cap, &pManual_cfg->vht_cap_info, sizeof(pEntry->vht_cap_ie.vht_cap));
	os_move_mem(&pEntry->HTCapability.HtCapInfo, &pManual_cfg->ht_cap_info, sizeof(pEntry->HTCapability.HtCapInfo));
	os_move_mem(&pEntry->vht_cap_ie.mcs_set, &pManual_cfg->vht_mcs_set, sizeof(pEntry->vht_cap_ie.mcs_set));

	return TRUE;
}

INT apply_sta_to_asic(RTMP_ADAPTER *pAd)
{
	struct manual_conn *manual_cfg = &pAd->fpga_ctl.manual_conn_info;
	UCHAR WCID = manual_cfg->wtbl_idx;
	UCHAR *pAddr = &manual_cfg->peer_mac[0];
	MT_WCID_TABLE_INFO_T WtblInfo;
	//MAC_TABLE_ENTRY *mac_entry = NULL;
#ifdef CONFIG_WTBL_TLV_MODE
#else
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
#endif /*CONFIG_WTBL_TLV_MODE */

#ifdef CONFIG_WTBL_TLV_MODE
#else
	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
	{
		WCID = (WCID < wtbl_ctrl->wtbl_entry_cnt[0] ? WCID : MCAST_WCID_TO_REMOVE);
	}
	else
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():PSE not init yet!\n", __FUNCTION__));
		return FALSE;
	}
#endif /* CONFIG_WTBL_TLV_MODE */

	os_zero_mem(&WtblInfo,sizeof(MT_WCID_TABLE_INFO_T));
	WtblInfo.Wcid = WCID;
	os_move_mem(&WtblInfo.Addr[0],&pAddr[0],6);
	// TODO: shiang-MT7615, risk here!!!
	//if (WCID < MAX_LEN_OF_MAC_TABLE)
	//	mac_entry = &pAd->MacTab.Content[WCID];

	if (WCID == MCAST_WCID_TO_REMOVE || WCID == MAX_LEN_OF_MAC_TABLE)
	{
		WtblInfo.MacAddrIdx = 0xe;
		WtblInfo.WcidType = MT_WCID_TYPE_BMCAST;
		 WtblInfo.CipherSuit = WTBL_CIPHER_NONE;
	}
	else
	{
		//if (!mac_entry) {
		//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		//				("%s(): mac_entry is NULL!\n", __FUNCTION__));
		//	return;
		//}

		if (pAd->fpga_ctl.manual_conn_info.peer_op_type == OPMODE_AP)
			WtblInfo.WcidType = MT_WCID_TYPE_AP;
		else
		WtblInfo.WcidType = MT_WCID_TYPE_CLI;
		WtblInfo.MacAddrIdx = manual_cfg->ownmac_idx; //mac_entry->wdev->OmacIdx;
		//WtblInfo.Aid = manual_cfg->wtbl_idx; //mac_entry->Aid;
		WtblInfo.CipherSuit = WTBL_CIPHER_NONE;

		//if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_WMM_CAPABLE))
		WtblInfo.SupportQoS = TRUE;

		if(WMODE_CAP_N(manual_cfg->peer_phy_mode))
		{
			WtblInfo.SupportHT = TRUE;
			//if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE))
			{
				WtblInfo.SupportRDG= TRUE;
			}
			WtblInfo.SmpsMode = 0; //mac_entry->MmpsMode ;
			WtblInfo.MpduDensity = 0; //mac_entry->MpduDensity;
			WtblInfo.MaxRAmpduFactor = 3; //mac_entry->MaxRAmpduFactor;

#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(manual_cfg->peer_phy_mode))
			{
				WtblInfo.SupportVHT = TRUE;
			}
#endif /* DOT11_VHT_AC */
		}
	}

	WtblInfo.PfmuId = manual_cfg->pfmuId;

MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s():Update WTBL table, WCID=%d, Addr=%02x:%02x:%02x:%02x:%02x:%02x, WtblInfo.MacAddrIdx=%d\n",
		__FUNCTION__, WCID, PRINT_MAC(pAddr), WtblInfo.MacAddrIdx));


	MtAsicUpdateRxWCIDTable(pAd, WtblInfo);

#ifdef MANUAL_MU
        if (WMODE_CAP_N(manual_cfg->peer_phy_mode)) {
		MT_BA_CTRL_T BaCtrl;
              INT tid;

		os_zero_mem(&BaCtrl,sizeof(MT_BA_CTRL_T));
		BaCtrl.BaSessionType = BA_SESSION_ORI;
		BaCtrl.BaWinSize = 64;
		BaCtrl.isAdd = TRUE;
		BaCtrl.Sn = 0;
		BaCtrl.Wcid = WtblInfo.Wcid;
		BaCtrl.band_idx = 0;
		os_move_mem(&BaCtrl.PeerAddr[0],&WtblInfo.Addr[0],MAC_ADDR_LEN);
              for (tid = 0; tid < 4; tid++) {
		    BaCtrl.Tid = 0;
        		MtAsicUpdateBASession(pAd, BaCtrl);
              }
        }

        dump_wtbl_info(pAd, WtblInfo.Wcid);
#endif /* MANUAL_MU */

	return TRUE;
}


/*
	Assoc Parameters:
		mac:xx:xx:xx:xx:xx:xx-type:ap/sta-mode:a/b/g/gn/an/ac-bw:20/40/80/160-nss:1/2/3/4-pfmuId:xx-aid:xx-maxrate:

	@jeffrey: For MU-MIMO, we need to configure the HT/VHP cap info to emulate different STAs (# of STA >= 2)  which
		  supports different Tx and Rx dimension for early algorithm verification
*/
INT set_assoc_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char sep_type = '-', sep_val = ':';
	RTMP_STRING *tok, *param_str, *param_type, *param_val;
	INT stat;
	RTMP_STRING rate_str[64];

	NdisZeroMemory(&pAd->fpga_ctl.manual_conn_info, sizeof(struct manual_conn));

	tok = arg;
	while (tok)
	{
		if (strlen(tok)) {
			param_str = tok;
			tok = rtstrchr(tok, sep_type);
			if (tok) {
				*tok = 0;
				tok++;
			}
			printk("%s(): param_str=%s\n", __FUNCTION__, param_str);
			if (strlen(param_str)) {
				param_type = param_str;
				param_val = rtstrchr(param_str, sep_val);
				if (param_val)
				{
					*param_val = 0;
					param_val++;
				}

				if (strlen(param_type) && param_val && strlen(param_val)) {
					stat = manual_parsing_param(pAd, param_type, param_val);
					if (stat == FALSE)
						goto err_dump_usage;
				}
			}
		} else {
			break;
		}
	}

	printk("%s():User manual configured peer STA info:\n", __FUNCTION__);
	printk("\tMAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->fpga_ctl.manual_conn_info.peer_mac));
	printk("\tBAND=>%d\n", pAd->fpga_ctl.manual_conn_info.peer_band);
	printk("\tOwnMacIdx=>%d\n", pAd->fpga_ctl.manual_conn_info.ownmac_idx);
	printk("\tWTBL_Idx=>%d\n", pAd->fpga_ctl.manual_conn_info.wtbl_idx);
	printk("\tOperationType=>%d\n", pAd->fpga_ctl.manual_conn_info.peer_op_type);
	printk("\tPhyMode=>%d\n", pAd->fpga_ctl.manual_conn_info.peer_phy_mode);
	printk("\tBandWidth=>%d\n", pAd->fpga_ctl.manual_conn_info.peer_bw);
	printk("\tNSS=>%d\n", pAd->fpga_ctl.manual_conn_info.peer_nss);
	printk("\tPfmuId=>%d\n", pAd->fpga_ctl.manual_conn_info.pfmuId);
	printk("\tAid=>%d\n", pAd->fpga_ctl.manual_conn_info.aid);
	printk("\tMaxRate_Mode=>%d\n", pAd->fpga_ctl.manual_conn_info.peer_maxrate_mode);
	printk("\tMaxRate_MCS=>%d\n", pAd->fpga_ctl.manual_conn_info.peer_maxrate_mcs);
	printk("Now apply it to hardware!\n");

	/* This applied the manual config info into the mac table entry, including the HT/VHT cap, VHT MCS set */
	apply_sta_to_mac_tbl_entry(pAd);

	apply_sta_to_asic(pAd);


#if defined(MT7615) || defined(MT7622)
	// sync the STA record to firmware.
	{
		STA_REC_CFG_T StaCfg;
		MAC_TABLE_ENTRY *pEntry;

		os_zero_mem(&StaCfg,sizeof(STA_REC_CFG_T));

		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (pAd->fpga_ctl.manual_conn_info.wtbl_idx >= GET_MAX_UCAST_NUM(pAd))
		{
			pEntry = NULL;
		}
		else
		{
			pEntry	= &pAd->MacTab.Content[pAd->fpga_ctl.manual_conn_info.wtbl_idx];
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s::Wcid(%d), u4EnableFeature(%d)\n",
		__FUNCTION__, WlanIdx, EnableFeature));


		if (!pEntry->wdev)
		{
			ASSERT(pEntry->wdev);
			return -1;
		}

		StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = CONNECTION_INFRA_STA;
		StaCfg.u4EnableFeature =  (1 << STA_REC_BASIC_STA_RECORD)
#ifdef DOT11_N_SUPPORT
			      				 | (1 << STA_REC_BASIC_HT_INFO)
#ifdef DOT11_VHT_AC
			       				| (1 << STA_REC_BASIC_VHT_INFO)
#endif
#endif
		StaCfg.ucBssIndex = pAd->fpga_ctl.manual_conn_info.ownmac_idx;
		StaCfg.ucWlanIdx = pAd->fpga_ctl.manual_conn_info.wtbl_idx;
		StaCfg.pEntry = pEntry;
		StaCfg.IsNewSTARec = TRUE;
		if (CmdExtStaRecUpdate(pAd,StaCfg)) == NDIS_STATUS_SUCCESS)
		{
		}
	}
#endif /* defined(MT7615) || defined(MT7622) */

	NdisZeroMemory(&rate_str[0], sizeof(rate_str));
	sprintf(rate_str, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
			pAd->fpga_ctl.manual_conn_info.wtbl_idx,
			pAd->fpga_ctl.manual_conn_info.peer_maxrate_mode,
			pAd->fpga_ctl.manual_conn_info.peer_bw,
			pAd->fpga_ctl.manual_conn_info.peer_maxrate_mcs,
			pAd->fpga_ctl.manual_conn_info.peer_nss,
			0, 0, 0, 0, 0);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tSet fixed RateInfo string as %s\n", rate_str));
	Set_Fixed_Rate_Proc(pAd, rate_str);

	return TRUE;

err_dump_usage:
	printk("Parameter Usage:\n");
	printk("\tiwpriv ra0 set assoc=[mac:hh:hh:hh:hh:hh:hh]-[wtbl:dd]-[ownmac:dd]-[type:xx]-[mode:mmm]-[bw:dd]-[nss:ss]-[maxrate:kkk_dd]\n");
	printk("\t\tmac: peer's mac address in hex format\n");
	printk("\t\t\tExample=> mac:00:0c:43:12:34:56\n");
	printk("\t\twtbl: the WTBL entry index peer will occupied, in range 1~127\n");
	printk("\t\t\tExample=> wtbl:1\n");
	printk("\t\townmac: the OwnMAC index we'll used to send frame to this peer, in range 0~4 or 16~31\n");
	printk("\t\t\tExample=> ownmac:0\n");
	printk("\t\ttype: peer's operation type, is a ap or sta, allow input: \"ap\" or \"sta\"\n");
	printk("\t\t\tExample=> type:ap\n");
	printk("\t\tmode: peer's phy operation mode, allow input: a/b/g/gn/an/ac \n");
	printk("\t\t\tExample=> mode:aanac	to indicate peer can support A/AN/AC mode\n");
	printk("\t\tbw: Peer's bandwidth capability, in range to 20/40/80/160\n");
	printk("\t\t\tExample=> bw:40	indicate peer can support BW_40\n");
	printk("\t\tnss: Peer's capability for Spatial stream which can tx/rx, in range of 1~4 with restriction of Software/Hardware cap.\n");
	printk("\t\t\tExample=> nss:2	indicate peer can support 2ss for both tx/rx\n");
	printk("\t\tmaxrate: Peer's data rate capability for tx/rx, separate as two parts and separate by '_' character\n");
	printk("\t\t\t\t kkk: phy modulation mode, allow input:'cck', 'ofdm', 'htmix', 'htgf', 'vht'\n");
	printk("\t\t\t\t dd:phy mcs rate, for CCK:0~3, OFDM:0~7, HT:0~32, VHT:0~9\n");
	printk("\t\t\tExample=> maxrate:cck_1	indicate we only can transmit CCK and MCS 1(2Mbps) or lower MCS to peer\n");
	printk("\t\t\tExample=> maxrate:ofdm_3	indicate we only can transmit OFDM and MCS 3(24Mbps) to peer\n");
	printk("\t\t\tExample=> maxrate:htmix_3	indicate we only can transmit OFDM and MCS 3(24Mbps) to peer\n");

	return FALSE;
}


INT set_del_ba_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	return TRUE;
}


INT set_add_ba_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	return TRUE;
}
#endif /* MANUAL_CONNECT */


#if defined(WFA_VHT_PF) || defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) ||  defined(MT7637_FPGA)
INT set_force_amsdu(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->force_amsdu = (simple_strtol(arg, 0, 10) > 0 ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): force_amsdu=%d\n",
				__FUNCTION__, pAd->force_amsdu));
	return TRUE;
}
#endif /* defined(WFA_VHT_PF) || defined(MT7603_FPGA) */


#ifdef WFA_VHT_PF
INT set_vht_nss_mcs_cap(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *token, sep[2] = {':', '-'};
	UCHAR val[3] = {0}, ss, mcs_l, mcs_h, mcs_cap, status = FALSE;
	INT idx = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string=%s\n", __FUNCTION__, arg));
	ss = mcs_l = mcs_h = 0;

	while (arg)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string[len=%d]=%s\n", __FUNCTION__, strlen(arg), arg));
		if (idx < 2) {
			token = rtstrchr(arg, sep[idx]);
			if (!token) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cannot found token '%c' in string \"%s\"!\n", sep[idx], arg));
				return FALSE;
			}
			*token++ = 0;
		} else
			token = NULL;

		if (strlen(arg)) {
			val[idx] = (UCHAR)simple_strtoul(arg, NULL, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():token string[len=%d]=%s, val[%d]=%d\n",
						__FUNCTION__, strlen(arg), arg, idx, val[idx]));
			idx++;
		}
		arg = token;
		if (idx == 3)
			break;
	}

	if (idx <3)
		return FALSE;

	ss = val[0];
	mcs_l = val[1];
	mcs_h = val[2];
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ss=%d, mcs_l=%d, mcs_h=%d\n", ss, mcs_l, mcs_h));
	if (ss && mcs_h)
	{
		if (ss <= pAd->chipCap.max_nss)
			pAd->CommonCfg.vht_nss_cap = ss;
		else
			pAd->CommonCfg.vht_nss_cap = pAd->chipCap.max_nss;

		switch (mcs_h)
		{
			case 7:
				mcs_cap = VHT_MCS_CAP_7;
				break;
			case 8:
				mcs_cap = VHT_MCS_CAP_8;
				break;
			case 9:
				mcs_cap = VHT_MCS_CAP_9;
				break;
			default:
				mcs_cap = VHT_MCS_CAP_9;
				break;
		}

		if (mcs_h <= pAd->chipCap.max_vht_mcs)
			pAd->CommonCfg.vht_mcs_cap = mcs_cap;
		else
			pAd->CommonCfg.vht_mcs_cap = pAd->chipCap.max_vht_mcs;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ss=%d, mcs_cap=%d, vht_nss_cap=%d, vht_mcs_cap=%d\n",
					__FUNCTION__, ss, mcs_cap,
					pAd->CommonCfg.vht_nss_cap,
					pAd->CommonCfg.vht_mcs_cap));
		status = TRUE;
	}

	return status;
}


INT set_vht_nss_mcs_opt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string=%s\n", __FUNCTION__, arg));

	return Set_HtMcs_Proc(pAd, arg);
}


INT set_vht_opmode_notify_ie(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *token;
	UINT ss, bw;
	BOOLEAN status = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():intput string=%s\n", __FUNCTION__, arg));
	token = rtstrchr(arg, ':');
	if (!token)
		return FALSE;

	*token = 0;
	token++;
	if (strlen(arg) && strlen(token))
	{
		ss = simple_strtoul(arg, NULL, 10);
		bw = simple_strtoul(token, NULL, 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():ss=%d, bw=%d\n", __FUNCTION__, ss, bw));
		if (ss > 0 && ss <= 2)
			pAd->vht_pf_op_ss = ss;
		else
			pAd->vht_pf_op_ss = pAd->Antenna.field.RxPath;

		switch (bw) {
			case 20:
				pAd->vht_pf_op_bw = BAND_WIDTH_20;
				break;
			case 40:
				pAd->vht_pf_op_bw = BAND_WIDTH_40;
				break;
			case 80:
			default:
				pAd->vht_pf_op_bw = BAND_WIDTH_80;
				break;
		}
		status = TRUE;
	}

	pAd->force_vht_op_mode = status;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():force_vht_op_mode=%d, vht_pf_op_ss=%d, vht_pf_op_bw=%d\n",
				__FUNCTION__, pAd->force_vht_op_mode, pAd->vht_pf_op_ss, pAd->vht_pf_op_bw));

	return status;
}


INT set_force_operating_mode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->force_vht_op_mode = (simple_strtol(arg, 0, 10) > 0 ? TRUE : FALSE);

	if (pAd->force_vht_op_mode == TRUE) {
		pAd->vht_pf_op_ss = 1; // 1SS
		pAd->vht_pf_op_bw = BAND_WIDTH_20; // 20M
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): force_operating_mode=%d\n",
				__FUNCTION__, pAd->force_vht_op_mode));
	if (pAd->force_vht_op_mode == TRUE)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tforce_operating_mode as %dSS in 20MHz BW\n",
					pAd->vht_pf_op_ss));
	}

	return TRUE;
}


INT set_force_noack(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->force_noack = (simple_strtol(arg, 0, 10) > 0 ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): force_noack=%d\n",
				__FUNCTION__, pAd->force_noack));

	return TRUE;
}


INT set_force_vht_sgi(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->vht_force_sgi = (simple_strtol(arg, 0, 10) > 0 ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): vht_force_sgi=%d\n",
				__FUNCTION__, pAd->vht_force_sgi));

	return TRUE;
}


INT set_force_vht_tx_stbc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	UCHAR TxStream;

	if(!wdev)
		return FALSE;

	if (pAd->CommonCfg.dbdc_mode)
	{
		UCHAR band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			TxStream = pAd->dbdc_2G_tx_stream;
		else
			TxStream = pAd->dbdc_5G_tx_stream;
	} else {
		TxStream = pAd->Antenna.field.TxPath;
	}

	pAd->vht_force_tx_stbc = (simple_strtol(arg, 0, 10) > 0 ? TRUE : FALSE);

	if (TxStream < 2)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): TxStream=%d is not enough for TxSTBC!\n",
				__FUNCTION__, TxStream));
		pAd->vht_force_tx_stbc = 0;
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): vht_force_tx_stbc=%d\n",
				__FUNCTION__, pAd->vht_force_tx_stbc));

	return TRUE;
}


INT set_force_ext_cca(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG cca_cfg;
	UINT32 mac_val;

	cca_cfg = (simple_strtol(arg, 0, 10) > 0 ? TRUE : FALSE);
	if (cca_cfg)
		mac_val = 0x04101b3f;
	else
		mac_val = 0x583f;
	RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);

	return TRUE;
}
#endif /* WFA_VHT_PF */


#ifdef DOT11_N_SUPPORT

#define MAX_AGG_CNT	8

/* DisplayTxAgg - display Aggregation statistics from MAC */
void DisplayTxAgg (RTMP_ADAPTER *pAd)
{
	ULONG totalCount;
	ULONG aggCnt[MAX_AGG_CNT + 2];
	int i;

	AsicReadAggCnt(pAd, aggCnt, sizeof(aggCnt) / sizeof(ULONG));
	totalCount = aggCnt[0] + aggCnt[1];
	if (totalCount > 0)
		for (i=0; i<MAX_AGG_CNT; i++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d MPDU=%ld (%ld%%)\n", i+1, aggCnt[i+2], aggCnt[i+2]*100/totalCount));
		}
	printk("====================\n");

}
#endif /* DOT11_N_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
INT Set_ReduceAckEnable_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  RTMP_STRING     *pParam)
{
    if (pParam == NULL)
        return FALSE;
    ReduceAckSetEnable(pAdapter, simple_strtol(pParam, 0, 10));
    return TRUE;
}


INT Show_ReduceAckInfo_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  RTMP_STRING     *pParam)
{
    ReduceAckShow(pAdapter);
    return TRUE;
}


INT Set_ReduceAckProb_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  RTMP_STRING     *pParam)
{
    if (pParam == NULL)
        return FALSE;

    ReduceAckSetProbability(pAdapter,simple_strtol(pParam, 0, 10));
    return TRUE;
}
#endif

#ifdef RLT_RF
static INT32 SetRltRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT bank_id = 0, rf_id = 0, rv = 0, rf_v;
	UCHAR rf_val = 0;

	if (Arg)
	{
		rv = sscanf(Arg, "%d-%d-%x", &(bank_id), &(rf_id), &(rf_v));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():rv = %d, bank_id = %d, rf_id = %d, rf_val = 0x%02x\n", __FUNCTION__, rv, bank_id, rf_id, rf_v));
		rf_val = (UCHAR )rf_v;
		if (rv == 3)
		{
			rlt_rf_write(pAd, (UCHAR)bank_id, (UCHAR)rf_id, (UCHAR)rf_val);

			rlt_rf_read(pAd, bank_id, rf_id, &rf_val);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%d %03d 0x%02X\n", __FUNCTION__, bank_id, rf_id, rf_val));
		}
		else if (rv == 2)
		{
			rlt_rf_read(pAd, bank_id, rf_id, &rf_val);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%d %03d 0x%02X\n", __FUNCTION__, bank_id, rf_id, rf_val));
		}
	}

	return TRUE;
}
#endif




#ifdef MT_MAC
static INT32 SetMTRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT RFIdx, Offset, Value, Rv;

	if (Arg)
	{
		Rv = sscanf(Arg, "%d-%x-%x", (int *)&RFIdx, (int *)&Offset, (int *)&Value);
		//MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RfIdx = %d, Offset = 0x%04x, Value = 0x%08x\n", RFIdx, Offset, Value));

		if (Rv == 2)
		{
			Value = 0;
			MtCmdRFRegAccessRead(pAd, (UINT32)RFIdx, (UINT32)Offset, (UINT32 *)&Value);

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():%d 0x%04x 0x%08x\n", __FUNCTION__, RFIdx, Offset, Value));

		}
		if (Rv == 3)
		{
			MtCmdRFRegAccessWrite(pAd, (UINT32)RFIdx, (UINT32)Offset, (UINT32)Value);
			Value = 0;
			MtCmdRFRegAccessRead(pAd, (UINT32)RFIdx, (UINT32)Offset, (UINT32 *)&Value);

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():%d 0x%04x 0x%08x\n", __FUNCTION__, RFIdx, Offset, Value));

		}
	}

	return TRUE;
}
#endif


INT32 SetRF(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

#ifdef RLT_RF
	if (pAd->chipCap.rf_type == RF_RLT)
		Ret = SetRltRF(pAd, Arg);
#endif


#ifdef MT_MAC
	if (pAd->chipCap.rf_type == RF_MT)
		Ret = SetMTRF(pAd, Arg);
#endif

	return Ret;
}


static struct {
	RTMP_STRING *name;
	INT (*show_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg, ULONG BufLen);
} *PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC, RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC[] = {
#ifdef DBG
	{"SSID",					Show_SSID_Proc},
	{"WirelessMode",			Show_WirelessMode_Proc},
	{"TxBurst",					Show_TxBurst_Proc},
	{"TxPreamble",				Show_TxPreamble_Proc},
	{"TxPower",					Show_TxPower_Proc},
	{"Channel",					Show_Channel_Proc},
	{"BGProtection",			Show_BGProtection_Proc},
	{"RTSThreshold",			Show_RTSThreshold_Proc},
	{"FragThreshold",			Show_FragThreshold_Proc},
#ifdef DOT11_N_SUPPORT
	{"HtBw",					Show_HtBw_Proc},
	{"HtMcs",					Show_HtMcs_Proc},
	{"HtGi",					Show_HtGi_Proc},
	{"HtOpMode",				Show_HtOpMode_Proc},
	{"HtExtcha",				Show_HtExtcha_Proc},
	{"HtMpduDensity",			Show_HtMpduDensity_Proc},
	{"HtBaWinSize",		        Show_HtBaWinSize_Proc},
	{"HtRdg",		        	Show_HtRdg_Proc},
	{"HtAmsdu",		        	Show_HtAmsdu_Proc},
	{"HtAutoBa",		        Show_HtAutoBa_Proc},
#endif /* DOT11_N_SUPPORT */
	{"CountryRegion",			Show_CountryRegion_Proc},
	{"CountryRegionABand",		Show_CountryRegionABand_Proc},
	{"CountryCode",				Show_CountryCode_Proc},
#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",			Show_PktAggregate_Proc},
#endif
#ifdef RTMP_UDMA_SUPPORT
	{"UdmaEnable",		        Show_UdmaMode_Proc},
	{"UdmaPortNum", 			Show_UdmaPortNum_Proc},
#endif	/* RTMP_UDMA_SUPPORT */
	{"WmmCapable",				Show_WmmCapable_Proc},

	{"IEEE80211H",				Show_IEEE80211H_Proc},
#ifdef SINGLE_SKU
	{"ModuleTxpower",			Show_ModuleTxpower_Proc},
#endif /* SINGLE_SKU */
#endif /* DBG */
	{"rainfo",					Show_STA_RAInfo_Proc},
	{NULL, NULL}
};


INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *pName,
	IN	RTMP_STRING *pBuf,
	IN	UINT32			MaxLen)
{
	INT	Status = 0;

	for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++)
	{
		if (!strcmp(pName, PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name))
		{
			if(PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->show_proc(pAd, pBuf, MaxLen))
				Status = -EINVAL;
			break;  /*Exit for loop.*/
		}
	}

	if(PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name == NULL)
	{
		snprintf(pBuf, MaxLen, "\n");
		for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++)
		{
			if ((strlen(pBuf) + strlen(PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name)) >= MaxLen)
				break;
			sprintf(pBuf, "%s%s\n", pBuf, PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name);
		}
	}

	return Status;
}


INT show_pwr_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (pAd->chipOps.show_pwr_info) {
		pAd->chipOps.show_pwr_info(pAd);
	}

	return 0;
}




INT32 ShowBBPInfo(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ShowAllBBP(pAd);

	return TRUE;
}


INT32 ShowRFInfo(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{

	ShowAllRF(pAd);

	return 0;
}

#define WIFI_INTERRUPT_NUM_MAX  1

INT32 ShowWifiInterruptCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    const UCHAR WifiIntMaxNum = WIFI_INTERRUPT_NUM_MAX;
    const CHAR WifiIntDesc[WIFI_INTERRUPT_NUM_MAX][32] = {"Wifi Abnormal counter"};
    UINT32 WifiIntCnt[WIFI_INTERRUPT_NUM_MAX];
    UINT32 WifiIntMask = 0xF;
    UCHAR BandIdx;
    UINT32 WifiIntIdx;

    os_zero_mem(WifiIntCnt,sizeof(WifiIntCnt));

    for(BandIdx = 0; BandIdx < DBDC_BAND_NUM ; BandIdx++)
    {
        MtCmdGetWifiInterruptCnt(pAd, BandIdx, WifiIntMaxNum, WifiIntMask, WifiIntCnt);
        for (WifiIntIdx = 0; WifiIntIdx < WifiIntMaxNum; WifiIntIdx++)
        {
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band %u:%s = %u\n", BandIdx, WifiIntDesc[WifiIntIdx], WifiIntCnt[WifiIntIdx]));
        }
    }

    return TRUE;
}

#ifdef BACKGROUND_SCAN_SUPPORT
INT set_background_scan(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BgndscanType= simple_strtol(arg, 0, 10);
	BackgroundScanStart(pAd, BgndscanType);
	return TRUE;
}

INT set_background_scan_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32	bgndscanduration = 0; /* ms */
	UINT32	bgndscaninterval = 0; /* second */
	UINT32	bgndscannoisyth = 0; 
	UINT32	bgndscanchbusyth = 0;
	UINT32	DriverTrigger =0;
	UINT32	bgndscansupport = 0;
	UINT32	ipith = 0;
	INT32	Recv=0;

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d", &(bgndscanduration), &(bgndscaninterval), &(bgndscannoisyth), &(bgndscanchbusyth), &(ipith), &(DriverTrigger), &(bgndscansupport));
	if (Recv != 7){
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set bgndscancfg=[Scan_Duration]-[Partial_Scan_Interval]-[Noisy_TH]-[BusyTime_TH]-[IPI_TH]-[Driver_trigger_Support]-[BGND_Support] \n"));

	}
	else {
		pAd->BgndScanCtrl.ScanDuration = bgndscanduration;
		pAd->BgndScanCtrl.PartialScanInterval = bgndscaninterval;
		pAd->BgndScanCtrl.NoisyTH= bgndscannoisyth;
		pAd->BgndScanCtrl.ChBusyTimeTH = bgndscanchbusyth;
		pAd->BgndScanCtrl.DriverTrigger= (BOOL)DriverTrigger;
		pAd->BgndScanCtrl.BgndScanSupport= (BOOL)bgndscansupport;
		pAd->BgndScanCtrl.IPIIdleTimeTH= (BOOL)ipith;
	}
		

	return TRUE;
}


INT set_background_scan_test(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	MT_BGND_SCAN_CFG BgndScanCfg;

	os_zero_mem(&BgndScanCfg,sizeof(MT_BGND_SCAN_CFG));
	
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0: /* ControlChannel */
				BgndScanCfg.ControlChannel = simple_strtol(value, 0, 10);
				break;
			case 1: /*  CentralChannel */
				BgndScanCfg.CentralChannel = simple_strtol(value, 0, 10);
				break;
			case 2: /* BW */
				BgndScanCfg.Bw = simple_strtol(value, 0, 10);
				break;
			case 3: /* TxStream */
				BgndScanCfg.TxStream = simple_strtol(value, 0, 10);
				break;
			case 4: /* RxPath */
				BgndScanCfg.RxPath = simple_strtol(value, 0, 16);
				break;
			case 5: /* Reason */
				BgndScanCfg.Reason = simple_strtol(value, 0, 10);
				break;
			case 6: /* BandIdx */
				BgndScanCfg.BandIdx= simple_strtol(value, 0, 10);
				break;
			default:
				break;
		}
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s  Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n", 
			__FUNCTION__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
	BackgroundScanTest(pAd, BgndScanCfg);
	return TRUE;
}
INT set_background_scan_notify(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *value = 0;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	int i;

	os_zero_mem(&BgScNotify,sizeof(MT_BGND_SCAN_NOTIFY));
	
	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0: /* Notify function */
				BgScNotify.NotifyFunc = simple_strtol(value, 0, 10);
				break;
			case 1: /*  Status */
				BgScNotify.BgndScanStatus = simple_strtol(value, 0, 10);
				break;
			default:
				break;	
		}
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s  NotifyFunc=%d, BgndScanStatus=%d\n", 
			__FUNCTION__, BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
	MtCmdBgndScanNotify(pAd, BgScNotify);
	return TRUE;
}

INT show_background_scan_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Background scan support = %d\n", pAd->BgndScanCtrl.BgndScanSupport));
	 if (pAd->BgndScanCtrl.BgndScanSupport == TRUE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    ("===== Configuration ===== \n"));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" Channel busy time Threshold = %d\n", pAd->BgndScanCtrl.ChBusyTimeTH));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" Noisy Threshold = %d  \n", pAd->BgndScanCtrl.NoisyTH));		
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" IPI Idle Threshold (*8us) = %d  \n", pAd->BgndScanCtrl.IPIIdleTimeTH));	
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" Scan Duration = %d ms\n", pAd->BgndScanCtrl.ScanDuration));	 
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" Partial Scan Interval = %d second\n", pAd->BgndScanCtrl.PartialScanInterval));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" DriverTrigger support= %d \n", pAd->BgndScanCtrl.DriverTrigger));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    ("===== Status / Statistic ===== \n"));
	                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    		    (" One sec channel busy time = %d\n", pAd->OneSecMibBucket.ChannelBusyTime[0]));
	                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" One sec My Tx Airtime = %d\n", pAd->OneSecMibBucket.MyTxAirtime[0]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" One sec My Rx Airtime = %d\n", pAd->OneSecMibBucket.MyRxAirtime[0]));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" IPI Idle time = %d\n", pAd->BgndScanCtrl.IPIIdleTime));
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    		    (" Noisy = %d\n", pAd->BgndScanCtrl.Noisy));
		 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    		    (" Current state = %ld\n", pAd->BgndScanCtrl.BgndScanStatMachine.CurrState));	
		 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			    (" Scan type = %d\n", pAd->BgndScanCtrl.ScanType));
		 //MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    	//	    (" Interval count = %d\n", pAd->BgndScanCtrl.BgndScanIntervalCount));
		 //MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    	//	    (" Interval = %d\n", pAd->BgndScanCtrl.BgndScanInterval));
			 
	 }
	 return TRUE;
}
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
/*
    ==========================================================================
    Description:
        RF test switch mode.

    Return:
    ==========================================================================
*/

INT Set_Icap_WifiSpec_Switch(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg)
{
        UCHAR ModeEnable = 0;

        ModeEnable = simple_strtol(arg, 0, 10);
        if (ModeEnable == 0)
		MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0,
								RF_TEST_DEFAULT_RESP_LEN);
	else if (ModeEnable == 1)
		MtCmdRfTestSwitchMode(pAd, OPERATION_RFTEST_MODE, 0,
								RF_TEST_DEFAULT_RESP_LEN);
	else if (ModeEnable == 2)
		MtCmdRfTestSwitchMode(pAd, OPERATION_ICAP_MODE, 0,
								RF_TEST_DEFAULT_RESP_LEN);
	else if (ModeEnable == 4)
		MtCmdRfTestSwitchMode(pAd, OPERATION_WIFI_SPECTRUM, 0,
								RF_TEST_DEFAULT_RESP_LEN);
	else
		MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0,
								RF_TEST_DEFAULT_RESP_LEN);
	return TRUE;
}

/*
    ==========================================================================
    Description:
        Set Icap/Wifi-spectrum start/stop capture parameters.

    Return:
    ==========================================================================
*/
INT Set_Icap_WifiSpec_Param(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg)
{
    INT i, j;
    UINT32 ret = 0;        
    CHAR *value = 0;
    RTMP_STRING Temp1[2] = {0};
    CHAR *pTemp1 = Temp1;
    UINT32 Temp2[6] = {0};
    UINT32 Trigger = 0;
    UINT32 RingCapEn = 0;
    UINT32 TriggerEvent = 0;
    UINT32 CaptureNode = 0;
    UINT32 CaptureLen = 0;
    UINT32 CapStopCycle = 0;
    UINT32 MACTriggerEvent = 0;
    UINT32 SourceAddrLSB = 0;
    UINT32 SourceAddrMSB = 0;
    UINT32 Band = 0;
    UINT8 BW = 0;

    for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
    {
        
        switch (i)
        {
			case 0:
				Trigger = simple_strtol(value, 0, 16);
				break;
			case 1:
				RingCapEn = simple_strtol(value, 0, 16);
				break;
			case 2:
				TriggerEvent = simple_strtol(value, 0, 16);
				break;
			case 3:
				CaptureNode = simple_strtol(value, 0, 16);
				break;
			case 4:
				CaptureLen = simple_strtol(value, 0, 16);
				break;
			case 5:
				CapStopCycle = simple_strtol(value, 0, 16);
				break;
			case 6:
				BW = simple_strtol(value, 0, 16);
				break;
			case 7:
				MACTriggerEvent = simple_strtol(value, 0, 16);
				break;
			case 8:
				for (j=0;j <6;j++)
				{
					RTMPMoveMemory(pTemp1, value, 2);
					Temp2[j] = simple_strtol(pTemp1, 0, 16);
					value += 2;
				}
				SourceAddrLSB = ((Temp2[0])  | (Temp2[1] << 8) |
                  					     (Temp2[2]) << 16 | (Temp2[3]) << 24);
				SourceAddrMSB = ((Temp2[4]) | (Temp2[5]<<8)|(0x1 <<16));
				break;
			case 9:
				Band = simple_strtol(value, 0, 16);
				break;
			default:
				break;
        }
    }
    
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : \n Trigger = 0x%08x\n"
    " RingCapEn  = 0x%08x\n TriggerEvent  = 0x%08x\n CaptureNode = 0x%08x\n CaptureLen = 0x%08x\n"
    " CapStopCycle = 0x%08x\n BW = 0x%08x\n MACTriggerEvent = 0x%08x\n SourceAddrLSB = 0x%08x\n"
    " SourceAddrMSB = 0x%08x\n Band = 0x%08x\n", __FUNCTION__, Trigger, RingCapEn, TriggerEvent, CaptureNode,
    CaptureLen, CapStopCycle, BW, MACTriggerEvent, SourceAddrLSB, SourceAddrMSB, Band));

    if ((CaptureNode == WF0_ADC) ||(CaptureNode == WF1_ADC) 
         || (CaptureNode == WF2_ADC) || (CaptureNode == WF3_ADC)
         || (CaptureNode == WF0_FIIQ) || (CaptureNode == WF1_FIIQ) 
         || (CaptureNode == WF2_FIIQ) || (CaptureNode == WF3_FIIQ)
         || (CaptureNode == WF0_FDIQ) || (CaptureNode == WF1_FDIQ)
         || (CaptureNode == WF2_FDIQ) || (CaptureNode == WF3_FDIQ))
    {
        ret = MtCmdWifiSpectrumParamSet(pAd, Trigger, RingCapEn, TriggerEvent, CaptureNode, CaptureLen,
              CapStopCycle, BW, MACTriggerEvent, SourceAddrLSB, SourceAddrMSB, Band);
    }
    else
    {
        ret = MtCmdRfTestIcapParamSet(pAd, Trigger, RingCapEn, TriggerEvent, CaptureNode, CaptureLen,
              CapStopCycle, BW, MACTriggerEvent, SourceAddrLSB, SourceAddrMSB, Band);
    }
     
    if(!ret)
    {
        ret = TRUE;            
    }
    
    return ret;
}

/*
    ==========================================================================
    Description:
         Get Icap/Wifi-spectrum capture node information.

    Return:
    ==========================================================================
*/
UINT32 Get_Icap_WifiSpec_Capture_Node_Info(
    IN RTMP_ADAPTER *pAd)
{
    UINT32 CaptureNode = 0, Value = 0;
        
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n",__FUNCTION__));

#if defined(MT7615)    
    PHY_IO_READ32(pAd, CR_DBGSGD_MODE, &Value);
#endif/*defined (MT7615)*/

    CaptureNode = Value & BITS(CR_SGD_MODE1, CR_SGD_DBG_SEL);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : CaptureNode = 0x%08x\n", __FUNCTION__, CaptureNode));

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n",__FUNCTION__));

    return CaptureNode;           
}

/*
    ==========================================================================
    Description:
         Get Icap/Wifi-spectrum stop capture information.

    Return:
    ==========================================================================
*/
INT Get_Icap_WifiSpec_Result(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg)
{
    UINT32 ret;
    UINT32 CaptureNode = 0;

    CaptureNode = Get_Icap_WifiSpec_Capture_Node_Info(pAd);

    if ((CaptureNode == WF0_ADC) ||(CaptureNode == WF1_ADC) 
         || (CaptureNode == WF2_ADC) || (CaptureNode == WF3_ADC)
         || (CaptureNode == WF0_FIIQ) || (CaptureNode == WF1_FIIQ)
         || (CaptureNode == WF2_FIIQ) || (CaptureNode == WF3_FIIQ)
         || (CaptureNode == WF0_FDIQ) || (CaptureNode == WF1_FDIQ)
         || (CaptureNode == WF2_FDIQ) || (CaptureNode == WF3_FDIQ))
    {
        ret = MtCmdWifiSpectrumResultGet(pAd, NULL);    
    }
    else
    {
        ret = MtCmdRfTestIcapResultGet(pAd, NULL);
    }
    
    if(!ret)
    {
        ret = TRUE;            
    }
        
    return ret;
}

/*
    ==========================================================================
    Description:
         Get current band central frequency information.

    Return:
    ==========================================================================
*/
UINT32 Get_Icap_WifiSpec_CentralFreq_Info(
    IN RTMP_ADAPTER *pAd, 
    IN UINT32 CaptureNode)
{
    UINT32 ChIdx;
    UCHAR CentralCh = 0;
    USHORT CentralFreq = 0;

    if (pAd->CommonCfg.dbdc_mode)//Dual Band
    {
        if ((CaptureNode == WF0_ADC) || (CaptureNode == WF1_ADC)
            || (CaptureNode == WF0_FIIQ) || (CaptureNode == WF1_FIIQ)
            || (CaptureNode == WF0_FDIQ) || (CaptureNode == WF1_FDIQ))
        {
            CentralCh = HcGetCentralChByRf(pAd, RFIC_24GHZ);
        }
        else if ((CaptureNode == WF2_ADC)|| (CaptureNode == WF3_ADC)
                 || (CaptureNode == WF2_FIIQ) || (CaptureNode == WF3_FIIQ)
                 || (CaptureNode == WF2_FDIQ) || (CaptureNode == WF3_FDIQ))
        {
            CentralCh = HcGetCentralChByRf(pAd, RFIC_5GHZ);
        }
    }
    else//Single Band 
    {         
        if (HcGetRadioChannel(pAd) <= 14)
        {
            CentralCh = HcGetCentralChByRf(pAd, RFIC_24GHZ);
        } 
        else
        {
            CentralCh = HcGetCentralChByRf(pAd, RFIC_5GHZ);
        } 
    }
    
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s : CentralCh = %d\n",__FUNCTION__, CentralCh));
    
	for (ChIdx = 0; ChIdx < CH_HZ_ID_MAP_NUM; ChIdx++)
	{
		if (CentralCh == CH_HZ_ID_MAP[ChIdx].channel)
		{
			CentralFreq = CH_HZ_ID_MAP[ChIdx].freqKHz;
			break;
		}
	}
    
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s : CentralFreq = %d\n",__FUNCTION__, CentralFreq)); 

    return CentralFreq;

}

/*
    ==========================================================================
    Description:
         Get current band bandwidth information.

    Return:
    ==========================================================================
*/
UCHAR Get_Icap_WifiSpec_Bw_Info(
    IN RTMP_ADAPTER *pAd, 
    IN UINT32 CaptureNode)
{
    CHAR Bw = 0, CaptureBw = 0;    

    if (pAd->CommonCfg.dbdc_mode)//Dual Band
    {
        if ((CaptureNode == WF0_ADC) || (CaptureNode == WF1_ADC)
            || (CaptureNode == WF0_FIIQ) || (CaptureNode == WF1_FIIQ)
            || (CaptureNode == WF0_FDIQ) || (CaptureNode == WF1_FDIQ))
        {
            Bw = HcGetBwByRf(pAd, RFIC_24GHZ);
        }
        else if ((CaptureNode == WF2_ADC)|| (CaptureNode == WF3_ADC)
                 || (CaptureNode == WF2_FIIQ) || (CaptureNode == WF3_FIIQ)
                 || (CaptureNode == WF2_FDIQ) || (CaptureNode == WF3_FDIQ))            
        {
            Bw = HcGetBwByRf(pAd, RFIC_5GHZ);
        }
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
            ("%s : Bw = %d\n",__FUNCTION__, Bw));
    }
    else//Single Band 
    {         
        if (HcGetRadioChannel(pAd) <= 14)
        {
            Bw = HcGetBwByRf(pAd, RFIC_24GHZ);
        } 
        else
        {
            Bw = HcGetBwByRf(pAd, RFIC_5GHZ);
        } 
    }
    
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s : Bw = %d\n",__FUNCTION__, Bw));

    switch(Bw)
    {
        case CMD_BW_20:
            CaptureBw = CAP_BW_20;
            break;
        case CMD_BW_40:
            CaptureBw = CAP_BW_40;
            break;
        case CMD_BW_80:
            CaptureBw = CAP_BW_80;
            break;
        case CMD_BW_160:
            CaptureBw = CAP_BW_80;
            break; 
        case CMD_BW_8080:
            CaptureBw = CAP_BW_80;
            break;
        default:
            CaptureBw = CAP_BW_20;
            break;
    }

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
        ("%s : CaptureBw = %d\n",__FUNCTION__, CaptureBw));
    
    return CaptureBw;

}

/*
    ==========================================================================
    Description:
         Used for getting current band wireless information.

    Return:
    ==========================================================================
*/

INT Get_Icap_WifiSpec_WirelessInfo(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg)
{
    USHORT CentralFreq = 0;
    UINT32 CaptureNode = 0;
    UCHAR Bw = 0;
    INT Idx;
    
    Idx = simple_strtol(arg, 0, 10);

    CaptureNode = Get_Icap_WifiSpec_Capture_Node_Info(pAd);

    switch(Idx)
    {
        case 0:
           CentralFreq = Get_Icap_WifiSpec_CentralFreq_Info(pAd, CaptureNode);
           break;
        case 1:
           Bw = Get_Icap_WifiSpec_Bw_Info(pAd, CaptureNode);
           break;
        default:
           break;
    }    
     
    return TRUE;
}

/*
    ==========================================================================
    Description:
         Get Icap RBIST sysram raw data.

    Return:
    ==========================================================================
*/
VOID Get_Icap_WifiSpec_Rbist_Data(
    IN RTMP_ADAPTER *pAd,
    IN PUINT32 pBank0_3Value,
    IN PUINT32 pBank4_7Value,
    IN PUINT32 pBank8_11Value,
    IN SHORT * pI_0,
    IN SHORT * pQ_0,
    IN SHORT * pI_1,
    IN SHORT * pQ_1,
    IN SHORT * pI_2,
    IN SHORT * pQ_2,
    IN SHORT * pI_3,
    IN SHORT * pQ_3)
{
    PRTMP_REG_PAIR pReg = NULL, pRegStopAddr = NULL, pRegWrap = NULL;
    PUINT32 pTemp = NULL;
    UINT32 StartAdd , offset, Bank, StopPoint, CaptureNode = 0;
    BOOLEAN Wrap;
    INT i, j;

        
    os_alloc_mem(pAd, (UCHAR **)&pReg, MULTIPLE_ACCESS_READ_SAMPLE*sizeof(RTMP_REG_PAIR));       
    os_alloc_mem(pAd, (UCHAR **)&pTemp, ICAP_WIFISPEC_BANK_SAMPLE_CNT * sizeof(UINT32)); 
    os_alloc_mem(pAd, (UCHAR **)&pRegStopAddr, sizeof(RTMP_REG_PAIR)); 
    os_alloc_mem(pAd, (UCHAR **)&pRegWrap, sizeof(RTMP_REG_PAIR));

    NdisZeroMemory(pReg, MULTIPLE_ACCESS_READ_SAMPLE*sizeof(RTMP_REG_PAIR));
    NdisZeroMemory(pTemp, ICAP_WIFISPEC_BANK_SAMPLE_CNT * sizeof(UINT32));
    NdisZeroMemory(pRegStopAddr, sizeof(RTMP_REG_PAIR));
    NdisZeroMemory(pRegWrap, sizeof(RTMP_REG_PAIR));
        
    offset = 0x20;
    for (Bank = 0 ; Bank < ICAP_WIFISPEC_TOTAL_BANK_CNT; Bank++)
	{
		if (Bank < 8)
		{
			StartAdd = 0x100000 + Bank*4;// Bolck 1~2
		}
		else
		{
			offset = 0x10;
	        StartAdd = 0x140000 + (Bank - 8)*4;//Block 3
		}
		for(i = 0; i < (ICAP_WIFISPEC_BANK_SAMPLE_CNT/MULTIPLE_ACCESS_READ_SAMPLE); i++ )
		{
			for (j = 0; j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
			{
				pReg[j].Register = StartAdd + (offset*(i*MULTIPLE_ACCESS_READ_SAMPLE + j));
			}
			MtCmdMultipleMacRegAccessRead(pAd, pReg, MULTIPLE_ACCESS_READ_SAMPLE);
			for (j = 0; j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
		    {
		        //Store raw data to
		        //Bank0_3Value => Block1
		        //Bank4_7Value => Block2
		        //Bank8_11Value => Block3
		        if (Bank < 4)
		        {
					*(pBank0_3Value + (ICAP_WIFISPEC_BANK_SAMPLE_CNT * Bank + i * MULTIPLE_ACCESS_READ_SAMPLE + j)) = pReg[j].Value;
				}
				else if (Bank > 3 && Bank < 8)
				{
				       *(pBank4_7Value + (ICAP_WIFISPEC_BANK_SAMPLE_CNT * (Bank - 4) + i * MULTIPLE_ACCESS_READ_SAMPLE + j)) = pReg[j].Value;
				}
				else
				{
					*(pBank8_11Value + (ICAP_WIFISPEC_BANK_SAMPLE_CNT * (Bank-8) + i * MULTIPLE_ACCESS_READ_SAMPLE+j)) = pReg[j].Value;
				}
			}
		}
    }

    //Re-arrange each buffer for stop address and wrap             
    pRegStopAddr->Register = RBISTCR9;
    MtCmdMultipleMacRegAccessRead(pAd, pRegStopAddr, 1);
    pRegWrap->Register = RBISTCR0;
    MtCmdMultipleMacRegAccessRead(pAd, pRegWrap, 1);
    Wrap = pRegWrap->Value & BIT(ICAP_WRAP);
    printk("RBISTCR9:0x%08x, Wrap: %d\n", pRegStopAddr->Value, Wrap);

    StartAdd = 0x100000;          
    StopPoint = (pRegStopAddr->Value - StartAdd)/4 ;
    if(!Wrap)
    {
        NdisZeroMemory((pBank0_3Value + StopPoint + 1), (ICAP_WIFISPEC_BLOCK_SAMPLE_CNT - StopPoint -1)*4);
        NdisZeroMemory((pBank4_7Value + StopPoint + 1), (ICAP_WIFISPEC_BLOCK_SAMPLE_CNT - StopPoint -1)*4);
        NdisZeroMemory((pBank8_11Value + StopPoint + 1), (ICAP_WIFISPEC_BLOCK_SAMPLE_CNT - StopPoint -1)*4);
    }
    else
    {
        memcpy(pTemp, pBank0_3Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT*4);
        for (i = 0; i < ICAP_WIFISPEC_BLOCK_SAMPLE_CNT; i++)
        {
			*(pBank0_3Value + i) = *(pTemp+ ((StopPoint + 1 + i) % ICAP_WIFISPEC_BLOCK_SAMPLE_CNT));
        }
        memcpy(pTemp, pBank4_7Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT*4);
        for (i = 0;i < ICAP_WIFISPEC_BLOCK_SAMPLE_CNT; i++)
        {
			*(pBank4_7Value + i) = *(pTemp+ ((StopPoint + 1 + i) % ICAP_WIFISPEC_BLOCK_SAMPLE_CNT));
        }
		memcpy(pTemp, pBank8_11Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT*4);
        for (i = 0;i < ICAP_WIFISPEC_BLOCK_SAMPLE_CNT; i++)
        {
			*(pBank8_11Value + i) = *(pTemp+ ((StopPoint + 1 + i) % ICAP_WIFISPEC_BLOCK_SAMPLE_CNT));
        }
    } 

    //Parsing 4-way I/Q
    CaptureNode = Get_Icap_WifiSpec_Capture_Node_Info(pAd);

    if (CaptureNode == FOUR_WAY_ADC) //4//4-way 10-bit RXADC
    {
        printk("Dump 4-Way RXADC ---->\n");
        for (i=0; i < ICAP_WIFISPEC_BLOCK_SAMPLE_CNT; i++)
        {
            *(pQ_0 + i) = (*(pBank0_3Value + i) & 0x3FF);                                         // Parsing Q0
            *(pI_0 + i)= ((*(pBank0_3Value + i) & (0x3FF<<10))>>10);     // Parsing I0
            *(pQ_1+ i) = ((*(pBank0_3Value + i) & (0x3FF<<20))>>20);     // Parsing Q1
            *(pI_1 + i) = ((*(pBank0_3Value + i) & (0x3<<30))>>30);      // Parsing I1
            *(pI_1 + i) |= ((*(pBank4_7Value + i) & (0xFF))<<2);         // Parsing I1
            *(pQ_2+ i) = ((*(pBank4_7Value + i) & (0x3FF<<8))>>8);       // Parsing Q2
            *(pI_2 + i) = ((*(pBank4_7Value + i) & (0x3FF<<18))>>18);    // Parsing I2
            *(pQ_3+ i) = ((*(pBank4_7Value + i) & (0xF<<28))>>28);       // Parsing Q3
            *(pQ_3+ i) |= ((*(pBank8_11Value + i) & (0x3F))<<4);         // Parsing Q3
            *(pI_3 + i) = ((*(pBank8_11Value + i) & (0x3FF<<6))>>6) ;    // Parsing I3
            if(*(pQ_0 + i) >= 512) // Calculation of two complement
            {
                *(pQ_0 + i) -=1024;
            }        
            if(*(pI_0 + i) >= 512)
            {
                *(pI_0 + i) -=1024;
            }    
            if(*(pQ_1 + i) >= 512)
            {
                *(pQ_1 + i) -=1024;
            }    
            if(*(pI_1 + i) >= 512)
            {
                *(pI_1 + i) -=1024;
            }    
            if(*(pQ_2+ i) >= 512)
            {
                *(pQ_2 + i) -=1024;
            }    
            if(*(pI_2 + i) >= 512)
            {
                *(pI_2 + i) -=1024;
            }    
            if(*(pQ_3 + i) >= 512)
            {
                *(pQ_3 + i) -=1024;
            }    
            if(*(pI_3 + i) >= 512)
            {
                *(pI_3 + i) -=1024;
            }    
        }
    }
    else if ((CaptureNode == FOUR_WAY_FIIQ) || (CaptureNode == FOUR_WAY_FDIQ))//4-way 12-bit IQC
    {
        printk("Dump 4-Way IQC ---->\n");
        for (i=0; i< ICAP_WIFISPEC_BLOCK_SAMPLE_CNT;i++)
        {
            *(pQ_0 + i) = (*(pBank0_3Value + i) & 0xFFF);             // Parsing Q0
            *(pI_0 + i) = ((*(pBank0_3Value + i) & (0xFFF<<12))>>12); // Parsing I0
            *(pQ_1+ i) = ((*(pBank0_3Value + i) & (0xFF<<24))>>24);   // Parsing Q1
            *(pQ_1+ i) |= ((*(pBank4_7Value + i) & 0xF)<<8);		  // Parsing Q1
            *(pI_1+ i) = ((*(pBank4_7Value + i) & (0xFFF<<4))>>4);    // Parsing I1
            *(pQ_2+ i) = ((*(pBank4_7Value + i) & (0xFFF<<16))>>16);  // Parsing Q2
            *(pI_2+ i) = ((*(pBank4_7Value + i) & (0xF<<28))>>28);    // Parsing I2
            *(pI_2+ i) |= ((*(pBank8_11Value + i) & 0xFF)<<4);        // Parsing I2
            *(pQ_3+ i) = (*(pBank8_11Value + i) & (0xFFF<<8))>>8;     // Parsing Q3
            *(pI_3+ i) = (*(pBank8_11Value + i) & (0xFFF<<20))>>20;   // Parsing I3
            if(*(pQ_0 + i) >= 2048) // Calculation of two complement
            {
                *(pQ_0 + i) -=4096;
            }    
            if(*(pI_0 + i) >= 2048)
            {
                *(pI_0 + i) -=4096;
            }
            if(*(pQ_1 + i) >= 2048)
            {
                *(pQ_1 + i) -=4096;
            }
            if(*(pI_1 + i) >= 2048)
            {
                *(pI_1 + i) -=4096;
            }
            if(*(pQ_2+ i) >= 2048)
            {
                *(pQ_2 + i) -=4096;
            }    
            if(*(pI_2 + i) >= 2048)
            {
                *(pI_2 + i) -=4096;
            }    
            if(*(pQ_3 + i) >= 2048)
            {
                *(pQ_3 + i) -=4096;
            }   
            if(*(pI_3 + i) >= 2048)
            {
                *(pI_3 + i) -=4096;
            }
        }
    }

    os_free_mem(pReg);
    os_free_mem(pTemp);
    os_free_mem(pRegStopAddr);
    os_free_mem(pRegWrap);
}

/*
    ==========================================================================
    Description:
         Get Icap/Wifi-spectrum RBIST sysram raw data.

    Return:
    ==========================================================================
*/
INT Get_Icap_WifiSpec_Sysram(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg)
{
    INT	i, j;	
	PRTMP_REG_PAIR pReg = NULL;
	UINT32 Choice, StartAdd , offset, Bank, CaptureNode;
	RTMP_OS_FD Srcf_RawData, Srcf_Blk1, Srcf_Blk2, Srcf_Blk3, Srcf_I0Q0, Srcf_I1Q1;
    RTMP_OS_FD Srcf_I2Q2, Srcf_I3Q3, Srcf_SpectrumData, Srcf_IQ, Srcf_Gain;
	RTMP_STRING *Src_RawData = "/tmp/RBISTRawDataDump.txt";
	RTMP_STRING *Src_Blk1= "/tmp/IcapRearrangeForStopAddr_Bank0_3Data.txt";
	RTMP_STRING *Src_Blk2= "/tmp/IcapRearrangeForStopAddr_Bank4_7Data.txt";
	RTMP_STRING *Src_Blk3 = "/tmp/IcapRearrangeForStopAddr_Bank8_11Data.txt";
	RTMP_STRING *Src_I0Q0 = "/tmp/IcapI_0Q_0Dump.txt";
	RTMP_STRING *Src_I1Q1 = "/tmp/IcapI_1Q_1Dump.txt";
	RTMP_STRING *Src_I2Q2 = "/tmp/IcapI_2Q_2Dump.txt";
	RTMP_STRING *Src_I3Q3 = "/tmp/IcapI_3Q_3Dump.txt";
 	RTMP_STRING *Src_SpectrumData = "/tmp/WifiSpectrumData.txt";   
	RTMP_STRING *Src_IQ = "/tmp/WifiSpectrumIQDump.txt";
	RTMP_STRING *Src_Gain = "/tmp/WifiSpectrumLNA_LPF_IndexDump.txt";   
	RTMP_OS_FS_INFO osFSInfo;
	UCHAR msg[64] , retval;
	PUINT32 pBank0_3Value = NULL;
	PUINT32 pBank4_7Value = NULL;
	PUINT32 pBank8_11Value = NULL;
    PUINT32 pWifiSpectrumValue = NULL;
	SHORT *pI_0 = NULL;
	SHORT *pQ_0 = NULL;
	SHORT *pI_1 = NULL;
	SHORT *pQ_1 = NULL;
	SHORT *pI_2 = NULL;
	SHORT *pQ_2 = NULL;
	SHORT *pI_3 = NULL;
	SHORT *pQ_3 = NULL;
	SHORT *pLNA = NULL;
	SHORT *pLPF = NULL;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n",__FUNCTION__));

    Choice = simple_strtol(arg, 0, 10);

	RtmpOSFSInfoChange(&osFSInfo, TRUE);// Change limits of authority in order to read/write file
	
	memset(msg, 0x00, 64);       
		
	if (Choice == NormalDump)//Dump whole raw data of RBIST sysram wo re-arrangement
	{
        os_alloc_mem(pAd, (UCHAR **)&pReg, MULTIPLE_ACCESS_READ_SAMPLE * sizeof(RTMP_REG_PAIR));
        NdisZeroMemory(pReg, MULTIPLE_ACCESS_READ_SAMPLE * sizeof(RTMP_REG_PAIR));
            
        Srcf_RawData = RtmpOSFileOpen(Src_RawData, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_RawData))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_RawData));
			goto error;
	  	}
        
		offset = 0x4;
		StartAdd = 0x100000;
		for(i = 0; i < ((ICAP_WIFISPEC_BLOCK1_SAMPLE_CNT + ICAP_WIFISPEC_BLOCK2_SAMPLE_CNT)/MULTIPLE_ACCESS_READ_SAMPLE); i++)
		{
			for (j = 0;j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
			{
				pReg[j].Register = StartAdd + (offset*(i * MULTIPLE_ACCESS_READ_SAMPLE + j));
			}
			MtCmdMultipleMacRegAccessRead(pAd, pReg, MULTIPLE_ACCESS_READ_SAMPLE);
			for (j = 0; j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
			{
				sprintf(msg, "0x%08x = 0x%08x\n", pReg[j].Register, pReg[j].Value);
				retval = RtmpOSFileWrite(Srcf_RawData, (RTMP_STRING *)msg, strlen(msg));
			}
		}
		StartAdd = 0x140000;
		for(i = 0; i< (ICAP_WIFISPEC_BLOCK3_SAMPLE_CNT/MULTIPLE_ACCESS_READ_SAMPLE); i++ )
		{
			for (j = 0; j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
			{
				pReg[j].Register = StartAdd + (offset * (i * MULTIPLE_ACCESS_READ_SAMPLE + j));
			}
			MtCmdMultipleMacRegAccessRead(pAd, pReg, MULTIPLE_ACCESS_READ_SAMPLE);
			for (j = 0; j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
			{
				sprintf(msg, "0x%08x = 0x%08x\n", pReg[j].Register, pReg[j].Value);
				retval = RtmpOSFileWrite(Srcf_RawData, (RTMP_STRING *)msg, strlen(msg));
			}
		}
		retval=RtmpOSFileClose(Srcf_RawData);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_RawData));
            goto error;
		}

        	os_free_mem(pReg);
	}
	else if (Choice == IcapDump)
	{
		os_alloc_mem(pAd, (UCHAR **)&pBank0_3Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(UINT32));// Dynamic allocate memory
		os_alloc_mem(pAd, (UCHAR **)&pBank4_7Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(UINT32));
		os_alloc_mem(pAd, (UCHAR **)&pBank8_11Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(UINT32));     
		os_alloc_mem(pAd, (UCHAR **)&pI_0, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pQ_0, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pI_1, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pQ_1, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pI_2, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pQ_2, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pI_3, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pQ_3, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));

		NdisZeroMemory(pBank0_3Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(UINT32));
		NdisZeroMemory(pBank4_7Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(UINT32));
		NdisZeroMemory(pBank8_11Value, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(UINT32));       
		NdisZeroMemory(pI_0, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pQ_0, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pI_1, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pQ_1, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pI_2, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pQ_2, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pI_3, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pQ_3, ICAP_WIFISPEC_BLOCK_SAMPLE_CNT * sizeof(SHORT));

		Get_Icap_WifiSpec_Rbist_Data(pAd, pBank0_3Value, pBank4_7Value, 
		         pBank8_11Value, pI_0, pQ_0, pI_1, pQ_1, pI_2, pQ_2, pI_3, pQ_3);
               
		//Dump raw data to file
		Srcf_Blk1 = RtmpOSFileOpen(Src_Blk1, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_Blk1))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_Blk1));
			goto error;
	  	}
		Srcf_Blk2 = RtmpOSFileOpen(Src_Blk2, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_Blk2))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_Blk2));
			goto error;
  		}
		Srcf_Blk3 = RtmpOSFileOpen(Src_Blk3, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_Blk3))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_Blk3));
			goto error;
  		}
        
		for (i=0;i < ICAP_WIFISPEC_BLOCK_SAMPLE_CNT; i++)
		{
			sprintf(msg, "0x%08x\n", *(pBank0_3Value + i));
		    retval = RtmpOSFileWrite(Srcf_Blk1, (RTMP_STRING *)msg, strlen(msg));
			sprintf(msg, "0x%08x\n", *(pBank4_7Value + i));
			retval = RtmpOSFileWrite(Srcf_Blk2, (RTMP_STRING *)msg, strlen(msg));
			sprintf(msg, "0x%08x\n", *(pBank8_11Value + i));
			retval = RtmpOSFileWrite(Srcf_Blk3, (RTMP_STRING *)msg, strlen(msg));
		}
        
		retval=RtmpOSFileClose(Srcf_Blk1);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_Blk1));
            goto error;
		}
		retval=RtmpOSFileClose(Srcf_Blk2);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_Blk2));
            goto error;
		}
		retval=RtmpOSFileClose(Srcf_Blk3);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_Blk3));
            goto error;
		}
        
		//Dump 4-way IQ to file
		Srcf_I0Q0 = RtmpOSFileOpen(Src_I0Q0, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_I0Q0))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_I0Q0));
			goto error;
	  	}
		Srcf_I1Q1 = RtmpOSFileOpen(Src_I1Q1, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_I1Q1))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_I1Q1));
			goto error;
  		}
		Srcf_I2Q2 = RtmpOSFileOpen(Src_I2Q2, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_I2Q2))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_I2Q2));
			goto error;
  		}
		Srcf_I3Q3 = RtmpOSFileOpen(Src_I3Q3, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_I3Q3))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_I3Q3));
			goto error;
  		}
        
		for (i = 0; i < ICAP_WIFISPEC_BLOCK_SAMPLE_CNT; i++)
		{
            sprintf(msg, "%+04d\t%+04d\n", *(pI_0 + i), *(pQ_0 + i));
            retval = RtmpOSFileWrite(Srcf_I0Q0, (RTMP_STRING *)msg, strlen(msg));
        	sprintf(msg, "%+04d\t%+04d\n", *(pI_1 + i), *(pQ_1 + i));
			retval = RtmpOSFileWrite(Srcf_I1Q1, (RTMP_STRING *)msg, strlen(msg));
			sprintf(msg, "%+04d\t%+04d\n", *(pI_2 + i), *(pQ_2 + i));
			retval = RtmpOSFileWrite(Srcf_I2Q2, (RTMP_STRING *)msg, strlen(msg));
			sprintf(msg, "%+04d\t%+04d\n", *(pI_3 + i), *(pQ_3 + i));
			retval = RtmpOSFileWrite(Srcf_I3Q3, (RTMP_STRING *)msg, strlen(msg));
		}
               
		retval=RtmpOSFileClose(Srcf_I0Q0);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_I0Q0));
            goto error;
		}
		retval=RtmpOSFileClose(Srcf_I1Q1);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_I1Q1));
            goto error;
		}
		retval=RtmpOSFileClose(Srcf_I2Q2);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_I2Q2));
            goto error;
		}
		retval=RtmpOSFileClose(Srcf_I3Q3);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_I3Q3));
            goto error;
		}

		os_free_mem(pBank0_3Value);
		os_free_mem(pBank4_7Value);
		os_free_mem(pBank8_11Value);
		os_free_mem(pI_0);
		os_free_mem(pI_1);
		os_free_mem(pI_2);
		os_free_mem(pI_3);
		os_free_mem(pQ_0);
		os_free_mem(pQ_1);
		os_free_mem(pQ_2);
		os_free_mem(pQ_3);
    }
	else if (Choice == WifiSpectrumDump)             
	{	
		os_alloc_mem(pAd, (UCHAR **)&pWifiSpectrumValue, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(UINT32));// Dynamic allocate memory
		os_alloc_mem(pAd, (UCHAR **)&pI_0, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pQ_0, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pLNA, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
		os_alloc_mem(pAd, (UCHAR **)&pLPF, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
	    os_alloc_mem(pAd, (UCHAR **)&pReg, MULTIPLE_ACCESS_READ_SAMPLE * sizeof(RTMP_REG_PAIR));
          	            
		NdisZeroMemory(pWifiSpectrumValue, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(UINT32));
		NdisZeroMemory(pI_0, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pQ_0, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pLNA, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
		NdisZeroMemory(pLPF, WIFISPECTRUM_SYSRAM_SAMPLE_CNT * sizeof(SHORT));
        NdisZeroMemory(pReg, MULTIPLE_ACCESS_READ_SAMPLE * sizeof(RTMP_REG_PAIR));
        
		//Dump raw data of WifiSpectrumValue
		Srcf_SpectrumData = RtmpOSFileOpen(Src_SpectrumData, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_SpectrumData))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_SpectrumData));
			goto error;
	  	}
#ifdef MT7622
        Bank = WIFISPECTRUM_SYSRAM_FIRST_BANK;
#else
		for (Bank = WIFISPECTRUM_SYSRAM_FIRST_BANK ; Bank < ICAP_WIFISPEC_TOTAL_BANK_CNT; Bank++)
#endif            
		{
			offset = WIFISPECTRUM_SYSRAM_ADDR_OFFSET;
		    StartAdd = WIFISPECTRUM_SYSRAM_START_ADDR + (Bank - WIFISPECTRUM_SYSRAM_FIRST_BANK) * 4;
			for(i = 0; i < (ICAP_WIFISPEC_BANK_SAMPLE_CNT/MULTIPLE_ACCESS_READ_SAMPLE); i++)
			{
				for (j = 0; j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
				{
					pReg[j].Register = StartAdd + (offset * (i * MULTIPLE_ACCESS_READ_SAMPLE + j));
				}
				MtCmdMultipleMacRegAccessRead(pAd, pReg, MULTIPLE_ACCESS_READ_SAMPLE);
				for (j = 0; j < MULTIPLE_ACCESS_READ_SAMPLE; j++)
			    {
                    // Store raw data to WifiSpectrumValue
					*(pWifiSpectrumValue + (ICAP_WIFISPEC_BANK_SAMPLE_CNT * (Bank - WIFISPECTRUM_SYSRAM_FIRST_BANK) + i * MULTIPLE_ACCESS_READ_SAMPLE + j)) = pReg[j].Value;
					sprintf(msg, "0x%08x\n", *(pWifiSpectrumValue + (ICAP_WIFISPEC_BANK_SAMPLE_CNT * (Bank - WIFISPECTRUM_SYSRAM_FIRST_BANK) + i * MULTIPLE_ACCESS_READ_SAMPLE + j)));                    
				    retval = RtmpOSFileWrite(Srcf_SpectrumData, (RTMP_STRING *)msg, strlen(msg));
     	        }
			}
		}
		retval = RtmpOSFileClose(Srcf_SpectrumData);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_SpectrumData));
            goto error;
		}
        
		//Dump IQ/LNA/LPF
		Srcf_IQ = RtmpOSFileOpen(Src_IQ, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(Srcf_IQ))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("--> Error opening %s\n", Src_IQ));
			goto error;
	  	}
		Srcf_Gain= RtmpOSFileOpen(Src_Gain, O_WRONLY|O_CREAT, 0);	
		if (IS_FILE_OPEN_ERR(Srcf_Gain)) 
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("--> Error opening %s\n", Src_Gain));
			goto error;
	  	}

        CaptureNode = Get_Icap_WifiSpec_Capture_Node_Info(pAd);
        
		for (i = 0; i < WIFISPECTRUM_SYSRAM_SAMPLE_CNT;i++)
		{
            if ((CaptureNode == WF0_ADC) || (CaptureNode == WF1_ADC) 
                || (CaptureNode == WF2_ADC) || (CaptureNode == WF3_ADC))
            {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                    ("Dump 1-Way RXADC/LNA/LPF ---->\n"));       

                *(pQ_0 + i) = (*(pWifiSpectrumValue + i) & 0x3FF);
			    *(pI_0 + i)= ((*(pWifiSpectrumValue + i) & (0x3FF<<10))>>10);
			    if(*(pQ_0 + i) >= 512)
                {         
				    *(pQ_0 + i) -=1024;
                }    
			    if(*(pI_0 + i) >= 512)
                {         
				    *(pI_0 + i) -=1024;
                }    
			    sprintf(msg, "%+04d\t%+04d\n", *(pI_0 + i), *(pQ_0 + i));
			    retval = RtmpOSFileWrite(Srcf_IQ, (RTMP_STRING *)msg, strlen(msg));
            
			    *(pLNA + i) = ((*(pWifiSpectrumValue + i) & (0x3<<28))>>28);
			    *(pLPF + i)= ((*(pWifiSpectrumValue + i) & (0xF<<24))>>24);
			    sprintf(msg, "%+04d\t%+04d\n", *(pLNA + i), *(pLPF + i));
			    retval = RtmpOSFileWrite(Srcf_Gain, (RTMP_STRING *)msg, strlen(msg));
            } 
            else if ((CaptureNode == WF0_FIIQ) || (CaptureNode == WF1_FIIQ) 
                     || (CaptureNode == WF2_FIIQ) || (CaptureNode == WF3_FIIQ)
                     || (CaptureNode == WF0_FDIQ) || (CaptureNode == WF1_FDIQ)
                     || (CaptureNode == WF2_FDIQ) || (CaptureNode == WF3_FDIQ))
            {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                    ("Dump 1-Way RXIQC/LNA/LPF ---->\n"));       

                *(pQ_0 + i) = (*(pWifiSpectrumValue + i) & 0xFFF);
			    *(pI_0 + i)= ((*(pWifiSpectrumValue + i) & (0xFFF<<12))>>12);
			    if(*(pQ_0 + i) >= 2048)
                {         
				    *(pQ_0 + i) -=4096;
                }    
			    if(*(pI_0 + i) >= 2048)
                {         
				    *(pI_0 + i) -=4096;
                }    
			    sprintf(msg, "%+04d\t%+04d\n", *(pI_0 + i), *(pQ_0 + i));
			    retval = RtmpOSFileWrite(Srcf_IQ, (RTMP_STRING *)msg, strlen(msg));
            
			    *(pLNA + i) = ((*(pWifiSpectrumValue + i) & (0x3<<28))>>28);
			    *(pLPF + i)= ((*(pWifiSpectrumValue + i) & (0xF<<24))>>24);
			    sprintf(msg, "%+04d\t%+04d\n", *(pLNA + i), *(pLPF + i));
			    retval = RtmpOSFileWrite(Srcf_Gain, (RTMP_STRING *)msg, strlen(msg));
            }
		}
        
		retval = RtmpOSFileClose(Srcf_IQ);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("--> Error %d closing %s\n", -retval, Src_IQ));
            goto error;
		}
		retval = RtmpOSFileClose(Srcf_Gain);
		if (retval)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("--> Error %d closing %s\n", -retval, Src_Gain));
            goto error;
		}
        
		os_free_mem(pWifiSpectrumValue);
		os_free_mem(pI_0);
		os_free_mem(pQ_0);
		os_free_mem(pLNA);
		os_free_mem(pLPF);
      	os_free_mem(pReg);
    }
    
	RtmpOSFSInfoChange(&osFSInfo, FALSE);// Change limits of authority in order to read/write file

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n",__FUNCTION__));

    return TRUE;

error:                  
    return FALSE;    
}

/*
    ==========================================================================
    Description:
         Get Icap/Wifi-spectrum RBIST sysram raw data offload to FW.

    Return:
    ==========================================================================
*/
INT Get_Icap_WifiSpec_RawData_Proc(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg)
{
    UINT32 ret;
      
    ret = MtCmdWifiSpectrumRawDataProc(pAd);

    if(!ret)
    {
        ret = TRUE;            
    }
        
    return ret;   
}
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef DSCP_QOS_MAP_SUPPORT
/*    ==========================================================================    
	Description:         
		Add qos pool to CR4    

	Return:    
	==========================================================================*/
VOID dscp_add_qos_map_pool_to_cr4(RTMP_ADAPTER *pAd,UINT8 PoolID)
{	
	EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;
	P_DSCP_QOS_MAP_TABLE_T pQosMapPool = NULL;	
	if(PoolID > 1)	
	{		
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s  PoolID %d not supported!!!!!!!!!!!!!!!!!!!!!\n",__FUNCTION__,PoolID));
		return;	
	}		
	NdisZeroMemory(&HotspotInfoUpdateT,sizeof(HotspotInfoUpdateT));	
	HotspotInfoUpdateT.ucUpdateType |= fgUpdateDSCPPoolMap;	
	pQosMapPool = &pAd->ApCfg.DscpQosMapTable[PoolID];	
	HotspotInfoUpdateT.ucPoolID = PoolID;	
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s  ===> Update Pool %d \n",__FUNCTION__,PoolID));	
	NdisCopyMemory(&HotspotInfoUpdateT.ucTableValid,pQosMapPool,sizeof(DSCP_QOS_MAP_TABLE_T));	 	
	MtCmdHotspotInfoUpdate(pAd,HotspotInfoUpdateT);	
	RtmpusecDelay(100);
}

/*    ==========================================================================    
	Description:         
		Update Sta qos mapping for dscp in CR4    

	Return:
	==========================================================================*/
VOID dscp_qosmap_update_sta_mapping_to_cr4(RTMP_ADAPTER *pAd,MAC_TABLE_ENTRY *pEntry,UINT8 PoolID)
{	
	EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;
	if(PoolID > 1)	
	{	
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s  PoolID %d not supported\n",	__FUNCTION__,PoolID));	
		return;	
	}		
	NdisZeroMemory(&HotspotInfoUpdateT,sizeof(HotspotInfoUpdateT));
	HotspotInfoUpdateT.ucUpdateType |= fgUpdateStaDSCPMap;	
	HotspotInfoUpdateT.ucStaWcid = pEntry->wcid;	
	HotspotInfoUpdateT.ucStaQosMapFlagAndIdx = PoolID;	
	HotspotInfoUpdateT.ucStaQosMapFlagAndIdx |= 0x80;	
	MtCmdHotspotInfoUpdate(pAd,HotspotInfoUpdateT);	
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s  wcid %d qosmap sta mapping flag 0x%x\n",__FUNCTION__,
													HotspotInfoUpdateT.ucStaWcid,HotspotInfoUpdateT.ucStaQosMapFlagAndIdx));
}

/*    ==========================================================================    
Description:         
	Initialize Dscp Qos mapping.    

	Return:    
==========================================================================*/
VOID DscpQosMapInit(RTMP_ADAPTER *pAd)
{	
	int i;		
	EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;		
	NdisZeroMemory(&HotspotInfoUpdateT,sizeof(HotspotInfoUpdateT));		
	HotspotInfoUpdateT.ucUpdateType |= fgDscpUpdateBssCapability;		
	HotspotInfoUpdateT.ucHotspotBssFlags = fgDscpQosMapEnable;		
	for(i=0; i<pAd->ApCfg.BssidNum; i++)		
	{		
		if(pAd->ApCfg.MBSSID[i].DscpQosMapEnable)
		{
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,("DscpQosMapInit Bss %d Pool Id %d \n",i,pAd->ApCfg.MBSSID[i].DscpQosPoolId));
			HotspotInfoUpdateT.ucHotspotBssId =  i;			
			MtCmdHotspotInfoUpdate(pAd,HotspotInfoUpdateT);			
		}
	}		
	for(i=0; i<2; i++)
	{
		if(pAd->ApCfg.DscpQosMapSupport[i])
			dscp_add_qos_map_pool_to_cr4(pAd, i);
	}
}
#endif	/*DSCP_QOS_MAP_SUPPORT*/


/*
    ==========================================================================
    Description:
         Set IRR ADC parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_ADC(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{       
        INT i;
	CHAR    *value = 0;
	UINT32  ChannelFreq = 0;
	UINT8   AntIndex = 0;
	UINT8   BW = 0;
	UINT8   SX = 0;
	UINT8   DbdcIdx = 0;
	UINT8   RunType = 0;
	UINT8   FType = 0;

	for (i = 0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0:
				AntIndex = simple_strtol(value, 0, 10);
                              switch (AntIndex)
                              {
                                    case QA_IRR_WF0:
                                            AntIndex = WF0;
                                            break;
                                    case QA_IRR_WF1:
                                            AntIndex = WF1;
                                            break;
                                    case QA_IRR_WF2:
                                            AntIndex = WF2;
                                             break;
                                    case QA_IRR_WF3:
                                             AntIndex = WF3;
                                             break;
                              }
				break;
			case 1:
				ChannelFreq = simple_strtol(value, 0, 10);
				break;
			case 2:
				BW = simple_strtol(value, 0, 10);
				break;
			case 3:
				SX = simple_strtol(value, 0, 10);
				break;
			case 4:
				DbdcIdx = simple_strtol(value, 0, 10);
				break;
			case 5:
				RunType = simple_strtol(value, 0, 10);
				break;
			case 6:
				FType = simple_strtol(value, 0, 10);
				break;
			default:
				break;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetADC> Input Checking Log \n\
					--------------------------------------------------------------\n\
					ChannelFreq = %d \n\
					AntIndex = %d \n\
					BW = %d \n\
					SX= %d \n\
					DbdcIdx = %d \n\
					RunType = %d \n\
					FType = %d \n\n",__FUNCTION__,\
					ChannelFreq,\
					AntIndex,\
					BW,\
					SX,\
					DbdcIdx,\
					RunType,\
					FType));

	MtCmdRfTestSetADC(pAd, ChannelFreq, AntIndex, BW, SX, DbdcIdx, RunType, FType);
	return TRUE;
}

/*
    ==========================================================================
    Description:
         Set IRR Rx Gain parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_RxGain(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

        INT i;
	CHAR    *value = 0;
	UINT8   LPFG = 0;
	UINT8   LNA = 0;
	UINT8   DbdcIdx = 0;
	UINT8   AntIndex = 0;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0:
				LPFG = simple_strtol(value, 0, 10);
				break;
			case 1:
				LNA = simple_strtol(value, 0, 10);
				break;
			case 2:
				DbdcIdx = simple_strtol(value, 0, 10);
				break;
			case 3:
				AntIndex = simple_strtol(value, 0, 10);
                              switch (AntIndex)
                              {
                                    case QA_IRR_WF0:
                                            AntIndex = WF0;
                                            break;
                                    case QA_IRR_WF1:
                                            AntIndex = WF1;
                                            break;
                                    case QA_IRR_WF2:
                                            AntIndex = WF2;
                                             break;
                                    case QA_IRR_WF3:
                                             AntIndex = WF3;
                                             break;
                              }                                        
				break;
			default:
				break;
			}
		}
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetRxGain> Input Checking Log \n\
					--------------------------------------------------------------\n\
					LPFG = %d \n\
					LNA = %d \n\
					DbdcIdx = %d \n\
					AntIndex= %d \n\n",__FUNCTION__,\
					LPFG,\
					LNA,\
					DbdcIdx,\
					AntIndex));

	MtCmdRfTestSetRxGain(pAd, LPFG, LNA, DbdcIdx, AntIndex);
	return TRUE;
}

/*
    ==========================================================================
    Description:
         Set IRR TTG parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_TTG(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

        INT i;
	CHAR    *value = 0;
	UINT32  ChannelFreq = 0;
	UINT32  ToneFreq = 0;
	UINT8   TTGPwrIdx = 0;
	UINT8   DbdcIdx = 0;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0:
				TTGPwrIdx = simple_strtol(value, 0, 10);
				break;
			case 1:
				ToneFreq = simple_strtol(value, 0, 10);
				break;
			case 2:
				ChannelFreq = simple_strtol(value, 0, 10);
				break;
			case 3:
				DbdcIdx = simple_strtol(value, 0, 10);
				break;
			default:
				break;
		}
	}
	
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetTTG> Input Checking Log \n\
					--------------------------------------------------------------\n\
					ChannelFreq = %d \n\
					ToneFreq = %d \n\
					TTGPwrIdx = %d \n\
					DbdcIdx= %d \n\n",__FUNCTION__,\
					ChannelFreq,\
					ToneFreq,\
					TTGPwrIdx,\
					DbdcIdx));

	MtCmdRfTestSetTTG(pAd, ChannelFreq, ToneFreq, TTGPwrIdx, DbdcIdx);
	return TRUE;
}

/*
    ==========================================================================
    Description:
         Set IRR TTGOnOff parameters.

    Return:
    ==========================================================================
*/
INT Set_IRR_TTGOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

        INT i;
	CHAR    *value = 0;
	UINT8   TTGEnable = 0;
	UINT8   DbdcIdx = 0;
	UINT8   AntIndex = 0;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++)
	{
		switch (i)
		{
			case 0:
				TTGEnable = simple_strtol(value, 0, 10);
				break;
			case 1:
				DbdcIdx = simple_strtol(value, 0, 10);
				break;
			case 2:
				AntIndex = simple_strtol(value, 0, 10);
                              switch (AntIndex)
                              {
                                    case QA_IRR_WF0:
                                            AntIndex = WF0;
                                            break;
                                    case QA_IRR_WF1:
                                            AntIndex = WF1;
                                            break;
                                    case QA_IRR_WF2:
                                            AntIndex = WF2;
                                             break;
                                    case QA_IRR_WF3:
                                             AntIndex = WF3;
                                             break;
                              }                
				break;
			default:
				break;
		}
	}
		
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: <SetTTGOnOff> Input Checking Log \n\
					--------------------------------------------------------------\n\
					TTGEnable = %d \n\
					DbdcIdx = %d \n\
					AntIndex = %d \n\n",__FUNCTION__,\
					TTGEnable,\
					DbdcIdx,\
					AntIndex));

	MtCmdRfTestSetTTGOnOff(pAd, TTGEnable, DbdcIdx, AntIndex);
	return TRUE;
}


INT set_manual_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    CHAR *token;
    UINT32 wdev_idx = 0, mode = 0;

    if (arg == NULL) {
        goto err1;
    }

    if (arg != NULL) {
        token = strsep (&arg, "-");
        wdev_idx = simple_strtol(token, 0, 10);
        if (pAd->wdev_list[wdev_idx] == NULL) {
            goto err2;
        }
    }

    while (arg != NULL) {
        token = strsep (&arg, "+");
        if (!strcmp (token, "erp")) {
            mode |= SET_PROTECT(ERP);
        }
        else if (!strcmp (token, "no")) {
            mode |= SET_PROTECT(NO_PROTECTION);
        }
        else if (!strcmp (token, "non_member")) {
            mode |= SET_PROTECT(NON_MEMBER_PROTECT);
        }
        else if (!strcmp (token, "ht20")) {
            mode |= SET_PROTECT(HT20_PROTECT);
        }
        else if (!strcmp (token, "non_ht_mixmode")) {
            mode |= SET_PROTECT(NON_HT_MIXMODE_PROTECT);
        }
        else if (!strcmp (token, "longnav")) {
            mode |= SET_PROTECT(LONG_NAV_PROTECT);
        }
        else if (!strcmp (token, "gf")) {
            mode |= SET_PROTECT(GREEN_FIELD_PROTECT);
        }
        else if (!strcmp (token, "rifs")) {
            mode |= SET_PROTECT(RIFS_PROTECT);
        }
        else if (!strcmp (token, "rdg")) {
            mode |= SET_PROTECT(RDG_PROTECT);
        }
        else if (!strcmp (token, "force_rts")) {
            mode |= SET_PROTECT(FORCE_RTS_PROTECT);
        }
        else {
            goto err3;
        }
    }

    pAd->wdev_list[wdev_idx]->protection = mode;

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            (" <<< manual trigger >>>\n HWFLAG_ID_UPDATE_PROTECT\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("   -- wdev_%d->protection: 0x%08x\n",
             wdev_idx, pAd->wdev_list[wdev_idx]->protection));
    HwCtrlSetFlag(pAd, HWFLAG_ID_UPDATE_PROTECT);

    goto end;

err3:
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            (" -no mode [ERROR 3]\n"));
    goto err1;

err2:
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            (" -no wdev_idx: 0x%x [ERROR 2]\n", wdev_idx));

err1:
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("Usage: \niwpriv ra0 set protect=[wdev_idx]-[mode]+...\n"));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
            ("       mode: [erp|no|non_member|ht20|non_ht_mixmode|longnav|gf|rifs|rdg|force_rts]\n"));

end:
    return TRUE;
}


#if defined(MT7615) || defined(MT7622)
INT set_cca_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    BOOLEAN enable;
    UINT32 val;

    enable = simple_strtol(arg, 0, 10);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("Enable CCA on Band0 SEC40: %s\n", (enable) ? "ON" : "OFF"));
    // RF CR for BAND0 CCA
    PHY_IO_READ32(pAd, PHY_BAND0_PHY_CCA, &val);
    val |= ((1<<18)|(1<<2));
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("-- Force Mode: %d, Force CCA SEC40: %d [0x%08x]\n",
             ((val&(1<<18))>>18), ((val&(1<<2))>>2), val));
    PHY_IO_WRITE32(pAd, PHY_BAND0_PHY_CCA, val);

    // TMAC_TCR for the normal Tx BW
    MAC_IO_READ32(pAd, TMAC_TCR, &val);
    val &= ~(PRE_RTS_IDLE_DET_DIS);
    val |= DCH_DET_DIS;
    MAC_IO_WRITE32(pAd, TMAC_TCR, val);

    return TRUE;
}
#endif /* defined(MT7615) || defined(MT7622) */


INT show_timer_list(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMPShowTimerList(pAd);
	return TRUE;
}


INT show_wtbl_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HcWtblRecDump(pAd);
	return TRUE;
}


/*
*
*/
UINT VIRTUAL_IF_INC(RTMP_ADAPTER *pAd)
{
	UINT cnt;
	ULONG flags=0;
	OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock,&flags);
	cnt= pAd->VirtualIfCnt++;
	OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock,&flags);
	return cnt;
}


/*
*
*/
UINT VIRTUAL_IF_DEC(RTMP_ADAPTER *pAd)
{
	UINT cnt;
	ULONG flags=0;
	OS_SPIN_LOCK_IRQSAVE(&pAd->VirtualIfLock,&flags);
	cnt = pAd->VirtualIfCnt--;
	OS_SPIN_UNLOCK_IRQRESTORE(&pAd->VirtualIfLock,&flags);

	return cnt;
}


/*
*
*/
UINT VIRTUAL_IF_NUM(RTMP_ADAPTER *pAd)
{
	UINT cnt;

	cnt = pAd->VirtualIfCnt;

	return cnt;
}

#if defined(MT7615) || defined(MT7622)
INT Set_Rx_Vector_Control(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg, 
    IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
        BOOLEAN Enable = 1;
        
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __FUNCTION__));
        
        if(arg)
        {
                Enable = simple_strtol(arg, 0, 10);
        }
        Enable = (Enable == 0 ? 0 : 1) ;

        MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, Enable, DBDC_BAND0);
#ifdef DBDC_MODE
        if(pAd->CommonCfg.dbdc_mode)
        {
                MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, Enable, DBDC_BAND1);
        }
#endif /*DBDC_MODE*/

        if (Enable)
        {    
                pAd->parse_rxv_stat_enable = 1;
        }
        else
        {
                pAd->parse_rxv_stat_enable = 0;
        }
        

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __FUNCTION__));

        return TRUE;        
}

INT Show_Rx_Statistic(
    IN RTMP_ADAPTER *pAd, 
    IN RTMP_STRING *arg, 
    IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
        #define MSG_LEN 1024
        #define ENABLE 1
        #define DISABLE 0

        RX_STATISTIC_RXV *rx_stat_rxv = &pAd->rx_stat_rxv;
        RX_STATISTIC_CR rx_stat_cr;
        UINT32 value = 0, i = 0, set = 1;
        UINT32 IBRssi0, IBRssi1, WBRssi0, WBRssi1, Status;
        UINT32 CurrBand0FCSErr, CurrBand0MDRDY;
        static UINT32 PreBand0FCSErr = 0, PreBand0MDRDY = 0;
#ifdef DBDC_MODE       
        UINT32 CurrBand1FCSErr, CurrBand1MDRDY;
        static UINT32 PreBand1FCSErr = 0, PreBand1MDRDY = 0;
#endif/*DBDC_MODE*/        
        RTMP_STRING *msg;

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __FUNCTION__));
        
        if(arg)
        {
                set = simple_strtol(arg, 0, 10);
        }
        set = (set == 0 ? 0 : 1) ;

        os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);
        memset(msg, 0x00, MSG_LEN);
        sprintf(msg, "\n");
                
        switch (set)
        {
                case RESET_COUNTER:
                        sprintf(msg+strlen(msg),"Reset counter !!\n");
 
#ifdef CONFIG_HW_HAL_OFFLOAD
                        /*Disable PHY Counter*/
	                MtCmdSetPhyCounter(pAd, DISABLE, DBDC_BAND0);
#ifdef DBDC_MODE
                       if (pAd->CommonCfg.dbdc_mode == TRUE) 
                       {
                                MtCmdSetPhyCounter(pAd, DISABLE, DBDC_BAND1);
                        }
#endif /*DBDC_MODE*/
#endif/*CONFIG_HW_HAL_OFFLOAD*/
                            
                        PreBand0FCSErr = 0;
                        PreBand0MDRDY = 0;
                        pAd->AccuOneSecRxBand0FcsErrCnt = 0; 
                        pAd->AccuOneSecRxBand0MdrdyCnt = 0;
                        pAd->AccuOneSecRxBand1FcsErrCnt = 0;
                        pAd->AccuOneSecRxBand1MdrdyCnt = 0;
                    break;
                case SHOW_RX_STATISTIC:                    
#ifdef CONFIG_HW_HAL_OFFLOAD
                        /*Enable PHY Counter*/
                        MtCmdSetPhyCounter(pAd, ENABLE, DBDC_BAND0);
#ifdef DBDC_MODE
                        if (pAd->CommonCfg.dbdc_mode == TRUE) 
                        {
                                MtCmdSetPhyCounter(pAd, ENABLE, DBDC_BAND1);
                        }
#endif /*DBDC_MODE*/
#endif/*CONFIG_HW_HAL_OFFLOAD*/

                        value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI);
                        IBRssi0 = (value&0xFF000000) >> 24;
                        if (IBRssi0 >= 128)
                                IBRssi0 -=256;
                        WBRssi0 = (value&0x00FF0000) >> 16;
                        if (WBRssi0 >= 128)
                                WBRssi0 -=256;
                        IBRssi1 = (value&0x0000FF00) >> 8;
                        if (IBRssi1 >= 128)
                                IBRssi1 -=256;
                        WBRssi1 = (value&0x000000FF);
                        if (WBRssi1 >= 128)
                                WBRssi1 -=256;
                        rx_stat_cr.Inst_IB_RSSSI[0] = IBRssi0;
                        rx_stat_cr.Inst_WB_RSSSI[0] = WBRssi0;
                        rx_stat_cr.Inst_IB_RSSSI[1] = IBRssi1;
                        rx_stat_cr.Inst_WB_RSSSI[1] = WBRssi1;

                        value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI_RX23);
                        IBRssi0 = (value&0xFF000000) >> 24;
                        if (IBRssi0 >= 128)
                                IBRssi0 -=256;
                        WBRssi0 = (value&0x00FF0000) >> 16;
                        if (WBRssi0 >= 128)
                                WBRssi0 -=256;
                        IBRssi1 = (value&0x0000FF00) >> 8;
                        if (IBRssi1 >= 128)
                                IBRssi1 -=256;
                        WBRssi1 = (value&0x000000FF);
                        if (WBRssi1 >= 128)
                                WBRssi1 -=256;
                        rx_stat_cr.Inst_IB_RSSSI[2] = IBRssi0;
                        rx_stat_cr.Inst_WB_RSSSI[2] = WBRssi0;
                        rx_stat_cr.Inst_IB_RSSSI[3] = IBRssi1;
                        rx_stat_cr.Inst_WB_RSSSI[3] = WBRssi1;
                    
                         /*Band0 MAC Counter*/ 
                         CurrBand0FCSErr = pAd->AccuOneSecRxBand0FcsErrCnt;  
                         rx_stat_cr.RxMacFCSErrCount = CurrBand0FCSErr - PreBand0FCSErr;
                         PreBand0FCSErr = CurrBand0FCSErr;
                      
                         CurrBand0MDRDY = pAd->AccuOneSecRxBand0MdrdyCnt;  
                         rx_stat_cr.RxMacMdrdyCount = CurrBand0MDRDY - PreBand0MDRDY;
                         PreBand0MDRDY = CurrBand0MDRDY;

                         rx_stat_cr.RxMacFCSOKCount = rx_stat_cr.RxMacMdrdyCount - rx_stat_cr.RxMacFCSErrCount;

                         /*Band0 PHY Counter */                      
                         value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_FCSERRCNT);
                         rx_stat_cr.FCSErr_OFDM = (value >> 16);
                         rx_stat_cr.FCSErr_CCK = (value & 0xFFFF);
                    
                         value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PD);
                         rx_stat_cr.OFDM_PD = (value >> 16);
                         rx_stat_cr.CCK_PD = (value & 0xFFFF);

                         value = MtAsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD);
                         rx_stat_cr.CCK_SIG_Err = (value >> 16);
                         rx_stat_cr.CCK_SFD_Err = (value & 0xFFFF);

                         value = MtAsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG);
                         rx_stat_cr.OFDM_SIG_Err = (value >> 16);
                         rx_stat_cr.OFDM_TAG_Err = (value & 0xFFFF);

                         value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITL);
                         rx_stat_cr.ACIHitLow = ((value >> 18) & 0x1);

                         value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITH);
                         rx_stat_cr.ACIHitHigh = ((value >> 18) & 0x1);

                         value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT);
                         rx_stat_cr.PhyMdrdyOFDM = (value >>16);
                         rx_stat_cr.PhyMdrdyCCK = (value & 0xFFFF);
                    
                         sprintf(msg+strlen(msg),"\x1b[41m%s : \x1b[m\n", __FUNCTION__);  
                         sprintf(msg+strlen(msg),"FreqOffsetFromRx   = %d\n", rx_stat_rxv->FreqOffsetFromRx);                      
                         for(i = 0; i < 4; i++) 
                         {
                                sprintf(msg+strlen(msg),"RCPI_%d             = %d\n", i, rx_stat_rxv->RCPI[i]);
                         }
                         for(i = 0; i < 4; i++) 
                         {
                                sprintf(msg+strlen(msg),"FAGC_RSSI_IB_%d     = %d\n",  i, rx_stat_rxv->FAGC_RSSI_IB[i]);                         
                         }
                         for(i = 0; i < 4; i++) 
                         {
                                sprintf(msg+strlen(msg),"FAGC_RSSI_WB_%d     = %d\n",  i, rx_stat_rxv->FAGC_RSSI_WB[i]);                          
                         }
                         for(i = 0; i < 4; i++) 
                         {
                                sprintf(msg+strlen(msg),"Inst_IB_RSSI_%d     = %d\n",  i, rx_stat_cr.Inst_IB_RSSSI[i]);                         
                         }
                         for(i = 0; i < 4; i++)
                         {
                                sprintf(msg+strlen(msg),"Inst_WB_RSSI_%d     = %d\n",  i, rx_stat_cr.Inst_WB_RSSSI[i]);                         ;
                         }
                         sprintf(msg+strlen(msg),"SNR                = %d\n",  rx_stat_rxv->SNR[0]);
                         sprintf(msg+strlen(msg),"ACIHitHigh         = %u\n",  rx_stat_cr.ACIHitHigh);
                         sprintf(msg+strlen(msg),"ACIHitLow          = %u\n",  rx_stat_cr.ACIHitLow);
                    
                          sprintf(msg+strlen(msg),"\x1b[41mFor Band0Index : \x1b[m \n");
                          sprintf(msg+strlen(msg),"MacMdrdyCount      = %u\n",  rx_stat_cr.RxMacMdrdyCount);
                          sprintf(msg+strlen(msg),"MacFCSErrCount     = %u\n",  rx_stat_cr.RxMacFCSErrCount);      
                          sprintf(msg+strlen(msg),"MacFCSOKCount      = %u\n",  rx_stat_cr.RxMacFCSOKCount);
                          sprintf(msg+strlen(msg),"CCK_PD             = %u\n",  rx_stat_cr.CCK_PD);
                          sprintf(msg+strlen(msg),"CCK_SFD_Err        = %u\n",  rx_stat_cr.CCK_SFD_Err);
                          sprintf(msg+strlen(msg),"CCK_SIG_Err        = %u\n",  rx_stat_cr.CCK_SIG_Err);
                          sprintf(msg+strlen(msg),"CCK_FCS_Err        = %u\n",  rx_stat_cr.FCSErr_CCK);
                          sprintf(msg+strlen(msg),"OFDM_PD            = %u\n",  rx_stat_cr.OFDM_PD);
                          sprintf(msg+strlen(msg),"OFDM_SIG_Err       = %u\n",  rx_stat_cr.OFDM_SIG_Err);
                          sprintf(msg+strlen(msg),"OFDM_FCS_Err       = %u\n",  rx_stat_cr.FCSErr_OFDM);
                  
#ifdef DBDC_MODE
                          if (pAd->CommonCfg.dbdc_mode == TRUE) 
                          {
                                /*Band1 MAC Counter*/ 
                                CurrBand1FCSErr = pAd->AccuOneSecRxBand1FcsErrCnt;  
                                rx_stat_cr.RxMacFCSErrCount_band1 = CurrBand1FCSErr - PreBand1FCSErr;
                                PreBand1FCSErr = CurrBand1FCSErr;
                      
                                CurrBand1MDRDY = pAd->AccuOneSecRxBand1MdrdyCnt;  
                                rx_stat_cr.RxMacMdrdyCount_band1 = CurrBand1MDRDY - PreBand1MDRDY;
                                PreBand1MDRDY = CurrBand1MDRDY;                                                            
                              
                                rx_stat_cr.RxMacFCSOKCount_band1 = rx_stat_cr.RxMacMdrdyCount_band1 - rx_stat_cr.RxMacFCSErrCount_band1;

                                /*Band1 PHY Counter*/
                                value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT_BAND1);
                                rx_stat_cr.PhyMdrdyOFDM_band1 = (value >>16);
                                rx_stat_cr.PhyMdrdyCCK_band1 = (value & 0xFFFF);

                                value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PD_BAND1);
                                rx_stat_cr.OFDM_PD_band1 = (value >> 16);
                                rx_stat_cr.CCK_PD_band1 = (value & 0xFFFF);

                                value = MtAsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD_BAND1);
                                rx_stat_cr.CCK_SIG_Err_band1 = (value >> 16);
                                rx_stat_cr.CCK_SFD_Err_band1 = (value & 0xFFFF);

                                value = MtAsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG_BAND1);
                                rx_stat_cr.OFDM_SIG_Err_band1 = (value >> 16);
                                rx_stat_cr.OFDM_TAG_Err_band1 = (value & 0xFFFF);
                            
                                sprintf(msg+strlen(msg),"\x1b[41mFor Band1Index : \x1b[m\n");
                                sprintf(msg+strlen(msg),"MacMdrdyCount      = %u\n",  rx_stat_cr.RxMacMdrdyCount_band1);
                                sprintf(msg+strlen(msg),"MacFCSErrCount     = %u\n",  rx_stat_cr.RxMacFCSErrCount_band1);
                                sprintf(msg+strlen(msg),"MacFCSOKCount      = %u\n",  rx_stat_cr.RxMacFCSOKCount_band1);
                                sprintf(msg+strlen(msg),"CCK_PD             = %u\n",  rx_stat_cr.CCK_PD_band1);
                                sprintf(msg+strlen(msg),"CCK_SFD_Err        = %u\n",  rx_stat_cr.CCK_SFD_Err_band1);
                                sprintf(msg+strlen(msg),"CCK_SIG_Err        = %u\n",  rx_stat_cr.CCK_SIG_Err_band1);
                                sprintf(msg+strlen(msg),"OFDM_PD            = %u\n",  rx_stat_cr.OFDM_PD_band1);
                                sprintf(msg+strlen(msg),"OFDM_SIG_Err       = %u\n",  rx_stat_cr.OFDM_SIG_Err_band1);  
                          }
#endif/*DBDC_MODE*/
                   break; 
        }

        wrq->u.data.length = strlen(msg);        
        Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
        os_free_mem( msg);

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __FUNCTION__));
        return TRUE;
}
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Show_SCSinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MT7615
        INT i=0;
	UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);
	
	if (IS_MT7615(pAd))
        {
		for(i=0;i<concurrent_bands;i++)
            {
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    ("************** Bnad%d  Information************* \n", i));
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d SCSEnable = %d\n", i, pAd->SCSCtrl.SCSEnable[i]));
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d SCSStatus = %d\n", i, pAd->SCSCtrl.SCSStatus[i]));
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d SCSMinRssi = %d\n", i, pAd->SCSCtrl.SCSMinRssi[i]));
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(" Bnad%d CckPdBlkTh = %d (%ddBm)\n", i, pAd->SCSCtrl.CckPdBlkTh[i], (pAd->SCSCtrl.CckPdBlkTh[i] - 256)));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(" Bnad%d OfdmPdBlkTh = %d(%ddBm)\n", i, pAd->SCSCtrl.OfdmPdBlkTh[i], (pAd->SCSCtrl.OfdmPdBlkTh[i]-512)/2));
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d Traffic TH = %d\n", i, pAd->SCSCtrl.SCSTrafficThreshold[i]));
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d MinRssiTolerance = %d\n", i, pAd->SCSCtrl.SCSMinRssiTolerance[i]));
                MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d SCSThTolerance = %d\n", i, pAd->SCSCtrl.SCSThTolerance[i]));
		                //MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    	//	    (" Bnad%d OFDM Support = %d\n", i, pAd->SCSCtrl.OfdmPdSupport[i]));
	 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d One sec TxByte = %d\n", i, pAd->SCSCtrl.OneSecTxByteCount[i]));
	 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d One sec RxByte = %d\n", i, pAd->SCSCtrl.OneSecRxByteCount[i]));
	 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d RTS count = %d\n", i, pAd->SCSCtrl.RtsCount[i]));
	 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
    		    (" Bnad%d RTS retry count = %d\n", i, pAd->SCSCtrl.RtsRtyCount[i]));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    ("=========CCK============= \n"));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d CCK false-CCA= %d\n", i, pAd->SCSCtrl.CckFalseCcaCount[i]));
			  MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d CCK false-CCA up bond= %d\n", i, pAd->SCSCtrl.CckFalseCcaUpBond[i]));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d CCK false-CCA low bond= %d\n", i, pAd->SCSCtrl.CckFalseCcaLowBond[i]));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d CCK fixed RSSI boundary= %d (%ddBm)\n", i, pAd->SCSCtrl.CckFixedRssiBond[i], (pAd->SCSCtrl.CckFixedRssiBond[i]-256)));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    ("=========OFDM============= \n"));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d OFDM false-CCA= %d\n", i, pAd->SCSCtrl.OfdmFalseCcaCount[i]));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d OFDM false-CCA up bond= %d\n", i, pAd->SCSCtrl.OfdmFalseCcaUpBond[i]));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d OFDM false-CCA low bond= %d\n", i, pAd->SCSCtrl.OfdmFalseCcaLowBond[i]));
			 MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		    		    (" Bnad%d OFDM fixed RSSI boundary= %d(%ddBm)\n", i, pAd->SCSCtrl.OfdmFixedRssiBond[i], (pAd->SCSCtrl.OfdmFixedRssiBond[i]-512) /2));
			 
             }   
            
        }
		
#endif /* MT7615 */

	return TRUE;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
INT	Set_Led_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR thisChar;
	long led_param[8];
	INT i=0, j=0;

	printk ("\n %s ==> arg = %s\n", __FUNCTION__, arg);
        memset(led_param, 0, sizeof(long)*8);


	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		led_param[i] = simple_strtol(thisChar, 0, 10);
		i++;
		if (i>=8)
			break;

	}
	printk("\n%s\n", __FUNCTION__);
	for (j=0; j<i; j++)
		printk("%02x\n", (UINT)led_param[j]);

#ifdef MT7615
	AndesLedEnhanceOP(pAd, led_param[0], led_param[1], led_param[2], led_param[3], led_param[4], led_param[5], led_param[6], led_param[7]);
#endif	
	
	return TRUE;
}
#endif


INT TxPowerSKUCtrl(
    IN PRTMP_ADAPTER     	pAd,
	IN UCHAR             	TxPowerSKUEn,
	IN UCHAR                BandIdx
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTxPowerSKUCtrl(pAd, TxPowerSKUEn, BandIdx) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT TxPowerPercentCtrl(
    IN PRTMP_ADAPTER     	pAd,
	IN UCHAR             	TxPowerPercentEn,
	IN UCHAR                BandIdx
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTxPowerPercentCtrl(pAd, TxPowerPercentEn, BandIdx) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT TxPowerDropCtrl(
    IN PRTMP_ADAPTER     	pAd,
	IN UINT8             	ucPowerDrop,
	IN UCHAR                BandIdx
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTxPowerDropCtrl(pAd, ucPowerDrop, BandIdx) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT TxPowerBfBackoffCtrl(
    IN PRTMP_ADAPTER        pAd,
    IN UCHAR                TxBFBackoffEn,
    IN UCHAR                BandIdx
    )
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTxBfBackoffCtrl(pAd, TxBFBackoffEn, BandIdx) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT ThermoCompCtrl(
    IN PRTMP_ADAPTER        pAd,
    IN BOOLEAN              fgThermoCompEn,
    IN UCHAR                BandIdx
    )
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdThermoCompCtrl(pAd, fgThermoCompEn, BandIdx) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT TxPowerRfTxAnt(
    IN PRTMP_ADAPTER     	pAd,
    IN UINT8                ucTxAntIdx
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTxPwrRfTxAntCtrl(pAd, ucTxAntIdx) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT TxPowerShowInfo(
    IN PRTMP_ADAPTER     	pAd,
    IN UCHAR                TxPowerInfoEn,
    IN UINT8                BandIdx
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (TxPowerInfoEn)
    {
        if (MtCmdTxPwrShowInfo(pAd, TxPowerInfoEn, BandIdx) == RETURN_STATUS_TRUE)
        {
            fgStatus = TRUE;
        }
    }
    else
    {
        fgStatus = TRUE;
    }
    
    return fgStatus;
}

INT TOAECtrlCmd(
    IN PRTMP_ADAPTER     	pAd,
    IN UCHAR                TOAECtrl
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTOAECalCtrl(pAd, TOAECtrl) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT EDCCACtrlCmd(
    IN PRTMP_ADAPTER     	pAd,
    IN UCHAR                BandIdx,
    IN UCHAR                EDCCACtrl
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdEDCCACtrl(pAd, BandIdx, EDCCACtrl) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT MUPowerCtrlCmd(
    IN PRTMP_ADAPTER     	pAd,
    IN BOOLEAN              MUPowerForce,
    IN UCHAR                MUPowerCtrl,
    IN UCHAR                BandIdx
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdMUPowerCtrl(pAd, MUPowerForce, MUPowerCtrl, BandIdx) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT BFNDPATxDCtrlCmd(
    IN PRTMP_ADAPTER        pAd,
    IN BOOLEAN              fgNDPA_ManualMode,
    IN UINT8                ucNDPA_TxMode,
    IN UINT8                ucNDPA_Rate,
    IN UINT8                ucNDPA_BW,
    IN UINT8                ucNDPA_PowerOffset
    )
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdBFNDPATxDCtrl(pAd, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

INT TemperatureCtrl(
    IN PRTMP_ADAPTER     	pAd,
    IN BOOLEAN              fgManualMode,
    IN CHAR             	cTemperature
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTemperatureCtrl(pAd, fgManualMode, cTemperature) == RETURN_STATUS_TRUE)
    {
        fgStatus = TRUE;
    }

    return fgStatus;
}

#ifdef TX_POWER_CONTROL_SUPPORT
INT TxPwrUpCtrl(
    IN PRTMP_ADAPTER    pAd,
    IN UINT8            ucBandIdx,
    IN CHAR            	cPwrUpCat,
    IN CHAR             cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM]
	)
{
    BOOLEAN  fgStatus = FALSE;

    if (MtCmdTxPwrUpCtrl(pAd, ucBandIdx, cPwrUpCat, cPwrUpValue) == RETURN_STATUS_TRUE)
        fgStatus = TRUE;

    return fgStatus;
}
#endif /* TX_POWER_CONTROL_SUPPORT */

/* [channel_band] 0: 2.4G, 1: 5G*/
UINT8 TxPowerGetChBand(
    IN UINT8				BandIdx,
    IN UINT8				CentralCh
	)
{
	UINT8	ChannelBand = 0;

	if (CentralCh >= 14)
	{
		ChannelBand = 1;
	}
	else
	{
		if (BandIdx == 0)
			ChannelBand = 0;
		else
			ChannelBand = 1;
	}

	return ChannelBand;
}

#ifdef TPC_SUPPORT
INT TxPowerTpcFeatureCtrl(
    IN PRTMP_ADAPTER     	pAd,
    IN struct wifi_dev		*wdev,
    IN INT8					TpcPowerValue
	)
{
    BOOLEAN  fgStatus = FALSE;
#ifdef MT7615
    UINT8 BandIdx;
    UINT8 RfIC;
    UINT8 CentralCh;

	if (!wdev)
		return fgStatus;

	if (!pAd->CommonCfg.b80211TPC)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): (X) b80211TPC=%d\n",
				__FUNCTION__, pAd->CommonCfg.b80211TPC));

		return fgStatus;
	}

	BandIdx = HcGetBandByWdev(wdev);

	switch (BandIdx)
	{
	case 0:
		RfIC = RFIC_24GHZ;
		break;
	case 1:
		RfIC = RFIC_5GHZ;
		break;
	default:
		RfIC = RFIC_24GHZ;
		break;
	}

	CentralCh = HcGetCentralChByRf(pAd, RfIC);

	if (MtCmdTpcFeatureCtrl(pAd, TpcPowerValue, BandIdx, CentralCh) == RETURN_STATUS_TRUE)
		fgStatus = TRUE;
#endif

    return fgStatus;
}

INT TxPowerTpcFeatureForceCtrl(
    IN PRTMP_ADAPTER     	pAd,
    IN INT8					TpcPowerValue,
    IN UINT8				BandIdx,
    IN UINT8				CentralChannel
	)
{
    BOOLEAN  fgStatus = FALSE;
#ifdef MT7615
    if (MtCmdTpcFeatureCtrl(pAd, TpcPowerValue, BandIdx, CentralChannel) == RETURN_STATUS_TRUE)
        fgStatus = TRUE;
#endif

    return fgStatus;
}
#endif /* TPC_SUPPORT */

INT Show_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{	
#ifdef MT7615
	INT i=0;
	UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);
	
	if (IS_MT7615(pAd)) {
		for(i=0;i<concurrent_bands;i++) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("====Bnad%d Enable = %d====\n", i, pAd->OneSecMibBucket.Enabled[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("Channel Busy Time = %d\n", pAd->OneSecMibBucket.ChannelBusyTime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("OBSS Air Time = %d\n", pAd->OneSecMibBucket.OBSSAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("My Tx Air Time = %d\n", pAd->OneSecMibBucket.MyTxAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("My Rx Air Time = %d\n", pAd->OneSecMibBucket.MyRxAirtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("EDCCA Time = %d\n", pAd->OneSecMibBucket.EDCCAtime[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("PD count = %x\n", pAd->OneSecMibBucket.PdCount[i]));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			           	 ("MDRDY Count = %x\n", pAd->OneSecMibBucket.MdrdyCount[i]));

		}
	}
#endif	
	return TRUE;
}

#ifdef RED_SUPPORT

VOID red_is_enabled(PRTMP_ADAPTER pAd)
{
	if(pAd->red_en)
	{
		MtCmdSetRedEnable(pAd, HOST2CR4, pAd->red_en);
        MtCmdSetRedEnable(pAd, HOST2N9, pAd->red_en);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: set CR4/N9 RED Enable to %d.\n", __FUNCTION__, pAd->red_en));
	}
}

INT set_vow_red_target_delay(
    IN  PRTMP_ADAPTER pAd,
    IN  RTMP_STRING *arg)
{
    UINT32 rv, tarDelay;

    if (arg)
    {
        rv = sscanf(arg, "%d", &tarDelay);
        if ((rv > 0) && (tarDelay >= 1) && (tarDelay <= 32767))
        {

            MtCmdSetRedTargetDelay(pAd, HOST2CR4, tarDelay);
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: set CR4 RED TARGET_DELAY to %d.\n", __FUNCTION__, tarDelay));
        }
        else
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}


INT set_vow_red_en(
    IN  PRTMP_ADAPTER pAd,
    IN  RTMP_STRING *arg)
{
    UINT32 en, rv;

    if (arg)
    {
        rv = sscanf(arg, "%d", &en);
        if ((rv > 0) && (en <= 1))
        {

            /* to CR4 & N9 */
            MtCmdSetRedEnable(pAd, HOST2CR4, en);
            MtCmdSetRedEnable(pAd, HOST2N9, en);
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: set CR4/N9 RED Enable to %d.\n", __FUNCTION__, en));
        }
        else
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}


INT set_vow_red_show_sta(
		IN	PRTMP_ADAPTER pAd,
		IN	RTMP_STRING *arg)
{
	UINT32 sta, rv;
	
	if (arg)
	{
		rv = sscanf(arg, "%d", &sta);
		if ((rv > 0) && (sta <= 127))
		{
	
			MtCmdSetRedShowSta(pAd, HOST2CR4, sta);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s: set CR4 RED show sta to %d.\n", __FUNCTION__, sta));
		}
		else
			return FALSE;
	}
	else
		return FALSE;
	
	return TRUE;
}

INT show_vow_red_info(
    IN  PRTMP_ADAPTER pAd,
    IN  RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("======== RED(per-STA Tail Drop) Information ========\n"));	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("RED Enbale: %d\n",pAd->red_en));



    return TRUE;
}
#endif /* RED_SUPPORT */

VOID cp_support_is_enabled(PRTMP_ADAPTER pAd)
{
	if((pAd->cp_support >= 1) && (pAd->cp_support <= 3))
	{
		MtCmdSetCPSEnable(pAd, HOST2CR4, pAd->cp_support);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: set CR4 CP_SUPPORT to Mode %d.\n", __FUNCTION__, pAd->cp_support));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("The Mode is invaild. Mode should be 1~3.\n"));
	}
		
}

INT set_cp_support_en(
    IN  PRTMP_ADAPTER pAd,
    IN  RTMP_STRING *arg)
{
    UINT32 rv, Mode;

    if (arg)
    {
        rv = sscanf(arg, "%d", &Mode);
        if ((rv > 0) && (Mode >= 1) && (Mode <= 3))
        {
            MtCmdSetCPSEnable(pAd, HOST2CR4, Mode);
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: set CR4 CP_SUPPORT to Mode %d.\n", __FUNCTION__, Mode));
        } else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
	                ("The Mode is invaild. Mode should be 1~3.\n"));
            return FALSE;
		}
    } else
        return FALSE;

    return TRUE;
}



#ifdef DHCP_UC_SUPPORT
static UINT32  checksum(PUCHAR buf, INT32  nbytes, UINT32 sum) 
{ 
	uint i; 
	/* Checksum all the pairs of bytes first... */ 
	for (i = 0; i < (nbytes & ~1U); i += 2) { 
		sum += (UINT16)ntohs(*((u_int16_t *)(buf + i))); 
		if (sum > 0xFFFF) 
			sum -= 0xFFFF; 
	} 

	/*
	 * If there's a single byte left over, checksum it, too. 
	 * Network byte order is big-endian, so the remaining byte is 
	 * the high byte. 
	 */ 
	if (i < nbytes) { 
		sum += buf[i] << 8; 
		if (sum > 0xFFFF) 
			sum -= 0xFFFF; 
	} 
	return (sum); 
} 

static UINT32 wrapsum(UINT32 sum) 
{ 
	sum = ~sum & 0xFFFF; 
	return (htons(sum)); 
} 
UINT16 RTMP_UDP_Checksum(IN PNDIS_PACKET pSkb) 
{
	PUCHAR pPktHdr, pLayerHdr;
	PUCHAR pPseudo_Hdr;
	PUCHAR pPayload_Hdr;
	PUCHAR pUdpHdr;
	UINT16 udp_chksum;
	UINT16 udp_len;
	UINT16 payload_len;
	
	pPktHdr = GET_OS_PKT_DATAPTR(pSkb);
	
	if (IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(pSkb))) {
		pLayerHdr = (pPktHdr + MAT_VLAN_ETH_HDR_LEN);
	} else {
		pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
	}
	pUdpHdr = pLayerHdr + 20;
	pPseudo_Hdr = pUdpHdr-8;
	pPayload_Hdr = pUdpHdr+8;
	udp_chksum = (*((UINT16*) (pUdpHdr+6)));
	udp_len = ntohs(*((UINT16*) (pUdpHdr+4)));
	payload_len = udp_len-8;
	udp_chksum = wrapsum(
					 checksum(
						 pUdpHdr,
						 8, 
						 checksum(
							 pPayload_Hdr, 
							 payload_len, 
							 checksum(
								 (unsigned char *)pPseudo_Hdr, 
								 2 * 4, 
								 17 + udp_len)))
				 );
	return udp_chksum;
}
#endif /* DHCP_UC_SUPPORT */

#ifdef ERR_RECOVERY
INT32 ShowSerProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E R , stat=0x%08X\n", 
		__FUNCTION__, ErrRecoveryCurStat(&pAd->ErrRecoveryCtl)));

	/* Dump SER related CRs */
	RTMPHandleInterruptSerDump(pAd);

	/* print out ser log timing */
	SerTimeLogDump(pAd);

	return TRUE;
}
#endif

INT32 ShowBcnProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	UINT32 band_idx = 0;

	for (band_idx = 0; band_idx < 2; band_idx++)
	{
	    if(arg != NULL && band_idx != simple_strtoul(arg, 0, 10))
	    {
	        continue;
	    }
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, Band %d \n", __FUNCTION__, band_idx));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================\n"));
		dump_bcn_debug_info(pAd, band_idx);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===============================\n"));
#endif
	return TRUE;
}


INT32 ShowCnInfoProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MT7615
    UINT32 value;
    UINT8 idx;

    if (pAd->cn_cnt != 10)
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

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump condition number:\n"));

    for (idx = 0; idx < 10; idx++)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CN = %d\n", pAd->rxv2_cyc3[idx]));
    }

#endif

	return TRUE;
}

#ifdef TX_POWER_CONTROL_SUPPORT
INT32 ShowTxPowerBoostInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8 ucRateIdx;
	UINT_8 ucBandIdx;
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
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid Band Index!!!\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==============================================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Power Up Table (Band%d)\n", ucBandIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==============================================================\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CCK\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0M1)-(M2M3)\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_CCK_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("OFDM\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0M1)-(M2M3)-(M4M5)-(M6  )-(M7  )\n"));
	for (ucRateIdx = RATE_POWER_CCK_NUM; ucRateIdx < (RATE_POWER_CCK_NUM + RATE_POWER_OFDM_NUM); ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT20\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0  )-(M32 )-(M1M2)-(M3M4)-(M5  )-(M6  )-(M7  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_HT20_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpHt20[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT40\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0  )-(M32)- (M1M2)-(M3M4)-(M5  )-(M6  )-(M7  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_HT40_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpHt40[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT20\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT20_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpVht20[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT40\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT40_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpVht40[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT80\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT80_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpVht80[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT160\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(M0  )-(M1M2)-(M3M4)-(M5M6)-(M7  )-(M8  )-(M9  )\n"));
	for (ucRateIdx = 0; ucRateIdx < RATE_POWER_VHT160_NUM; ucRateIdx++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%4d) ", pAd->CommonCfg.cPowerUpVht160[ucBandIdx][ucRateIdx]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	return TRUE;
}
#endif /* TX_POWER_CONTROL_SUPPORT */
