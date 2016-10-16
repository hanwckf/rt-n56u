
#ifdef APCLI_AUTO_BW_SUPPORT
#ifdef BW_TRIGGER_BY_ROOT_SUPPORT
#error "BW_TRIGGER_BY_ROOT_SUPPORT can't enable with APCLI_AUTO_BW_SUPPORT at the same time !"
#endif /* BW_TRIGGER_BY_ROOT_SUPPORT */

#include "rt_config.h"

BOOLEAN ApCliAutoBwAction(PRTMP_ADAPTER pAd, USHORT ifIndex)
{
	BOOLEAN needChange = FALSE;
        UINT8 rf_bw = BW_20, ext_ch = EXTCHA_NONE;
	APCLI_STRUCT *pApCliEntry;
        struct wifi_dev *wdev;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
        wdev = &pApCliEntry->wdev;

	if (!pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AutoBW(%s): apCient%d\n not using HT Mode", 
			__FUNCTION__, ifIndex));
		return needChange;
	}


	if (!WMODE_CAP_N(pApCliEntry->wdev.PhyMode))
	{
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AutoBW(%s): apCient%d\n not in N mode",
                        __FUNCTION__, ifIndex));
                return needChange;
        }

	/* For Dissallow */
        if (pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(wdev->WepStatus))
	{
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AutoBW(%s): apCient%d\n can't in N mode due invalid sec",
                        __FUNCTION__, ifIndex));
                return needChange;
        }

	
        /* our AP is 20M: bw extend from 20 to 40 */
        if ((wdev->bw == BW_40) &&
/*            (ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40) && */
            (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_20))
        {
        	needChange = TRUE;
                rf_bw = BW_40;
                pAd->CommonCfg.CentralChannel = pApCliEntry->MlmeAux.CentralChannel;
                pAd->CommonCfg.Channel = pApCliEntry->MlmeAux.Channel;

                if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
                {
                	ext_ch = EXTCHA_ABOVE;
                }
                else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
                {
                	ext_ch = EXTCHA_BELOW;
                }

                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AutoBW(%s): change the BW to 40\n", __FUNCTION__));
         }

         if (needChange)
         	AsicSetChannel(pAd, pAd->CommonCfg.CentralChannel, rf_bw, ext_ch, FALSE);

	 return needChange;
}

BOOLEAN ApCliSetPhyMode(PRTMP_ADAPTER pAd, USHORT ifIndex, UINT wmode_ori)
{
        APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev;
	UCHAR wmode;

        wmode = cfgmode_2_wmode((UCHAR)wmode_ori);
        if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                                ("%s(): Invalid wireless mode(%d, wmode=0x%x)\n",
                                __FUNCTION__, wmode_ori, wmode));
                return FALSE;
        } 

        apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &apcli_entry->wdev;

        wdev->PhyMode = wmode;

	NdisZeroMemory(wdev->SupRate, MAX_LEN_OF_SUPPORTED_RATES);
        NdisZeroMemory(wdev->ExtRate, MAX_LEN_OF_SUPPORTED_RATES);

       	switch (wdev->PhyMode) {
            case (WMODE_B):
                wdev->SupRate[0]  = 0x82;        /* 1 mbps, in units of 0.5 Mbps, basic rate*/
                wdev->SupRate[1]  = 0x84;        /* 2 mbps, in units of 0.5 Mbps, basic rate*/
                wdev->SupRate[2]  = 0x8B;        /* 5.5 mbps, in units of 0.5 Mbps, basic rate*/
                wdev->SupRate[3]  = 0x96;        /* 11 mbps, in units of 0.5 Mbps, basic rate*/
                wdev->SupRateLen  = 4;
                break;

            case (WMODE_B | WMODE_G):
            case (WMODE_A | WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
            case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
            case (WMODE_B | WMODE_G | WMODE_GN):
#endif /* DOT11_N_SUPPORT */
                 wdev->SupRate[0]  = 0x82;        /* 1 mbps, in units of 0.5 Mbps, basic rate*/
                 wdev->SupRate[1]  = 0x84;        /* 2 mbps, in units of 0.5 Mbps, basic rate*/
                 wdev->SupRate[2]  = 0x8B;        /* 5.5 mbps, in units of 0.5 Mbps, basic rate*/
                 wdev->SupRate[3]  = 0x96;        /* 11 mbps, in units of 0.5 Mbps, basic rate*/
                 wdev->SupRate[4]  = 0x12;        /* 9 mbps, in units of 0.5 Mbps*/
                 wdev->SupRate[5]  = 0x24;        /* 18 mbps, in units of 0.5 Mbps*/
                 wdev->SupRate[6]  = 0x48;        /* 36 mbps, in units of 0.5 Mbps*/
                 wdev->SupRate[7]  = 0x6c;        /* 54 mbps, in units of 0.5 Mbps*/
                 wdev->SupRateLen  = 8;

                 wdev->ExtRate[0]  = 0x0C;        /* 6 mbps, in units of 0.5 Mbps*/
                 wdev->ExtRate[1]  = 0x18;        /* 12 mbps, in units of 0.5 Mbps*/
                 wdev->ExtRate[2]  = 0x30;        /* 24 mbps, in units of 0.5 Mbps*/
                 wdev->ExtRate[3]  = 0x60;        /* 48 mbps, in units of 0.5 Mbps*/
                 wdev->ExtRateLen  = 4;
                 break;

            case (WMODE_A):
            case (WMODE_G):
#ifdef DOT11_N_SUPPORT
            case (WMODE_A | WMODE_AN):
            case (WMODE_A | WMODE_G | WMODE_GN | WMODE_AN):
            case (WMODE_G | WMODE_GN):
            case (WMODE_GN):
            case (WMODE_AN):
#endif /* DOT11_N_SUPPORT */
                  wdev->SupRate[0]  = 0x8C;        /* 6 mbps, in units of 0.5 Mbps, basic rate*/
                  wdev->SupRate[1]  = 0x12;        /* 9 mbps, in units of 0.5 Mbps*/
                  wdev->SupRate[2]  = 0x98;        /* 12 mbps, in units of 0.5 Mbps, basic rate*/
                  wdev->SupRate[3]  = 0x24;        /* 18 mbps, in units of 0.5 Mbps*/
                  wdev->SupRate[4]  = 0xb0;        /* 24 mbps, in units of 0.5 Mbps, basic rate*/
                  wdev->SupRate[5]  = 0x48;        /* 36 mbps, in units of 0.5 Mbps*/
		  wdev->SupRate[6]  = 0x60;        /* 48 mbps, in units of 0.5 Mbps*/
                  wdev->SupRate[7]  = 0x6c;        /* 54 mbps, in units of 0.5 Mbps*/
                  wdev->SupRateLen  = 8;
                  wdev->ExtRateLen  = 0;
                  break;

             default:
                  break;
        }

	MlmeUpdateTxRates(pAd, FALSE, ifIndex + MIN_NET_DEVICE_FOR_APCLI);

        if (WMODE_CAP_N(wdev->PhyMode))
        {
		RTMPSetIndividualHT(pAd, ifIndex + MIN_NET_DEVICE_FOR_APCLI);
        }

        return TRUE;
}

INT Set_ApCli_Bw_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
        UINT bw = 0;
        POS_COOKIE pObj;
        UCHAR ifIndex = 0;

        APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev;

        pObj = (POS_COOKIE) pAd->OS_Cookie;
        if (pObj->ioctl_if_type != INT_APCLI)
                return FALSE;

        ifIndex = pObj->ioctl_if;

        bw = simple_strtol(arg, 0, 10);

       	apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &apcli_entry->wdev;

       	wdev->bw = bw;
       	return TRUE;
}

INT Set_ApCli_PhyMode_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
       	UINT wmode = 0;
        POS_COOKIE pObj;
        UCHAR ifIndex = 0;

        pObj = (POS_COOKIE) pAd->OS_Cookie;
        if (pObj->ioctl_if_type != INT_APCLI)
                return FALSE;

       	ifIndex = pObj->ioctl_if;

        wmode = simple_strtol(arg, 0, 10);

       	if (!ApCliSetPhyMode(pAd, ifIndex, wmode))
               return FALSE;

       return TRUE;
}



#endif /* APCLI_AUTO_BW_SUPPORT */

