/*

*/

#include "rt_config.h"


#ifdef DOT11_VHT_AC

VOID rtmp_set_vht(RTMP_ADAPTER *pAd, RT_PHY_INFO *phy_info)
{
	if (!phy_info)
		return;

	if (phy_info->bVhtEnable && 
		(pAd->CommonCfg.vht_bw == VHT_BW_80))
			phy_info->vht_bw = VHT_BW_80;
	else
		phy_info->vht_bw = VHT_BW_2040;

}


INT SetCommonVHT(RTMP_ADAPTER *pAd)
{
	if (!WMODE_CAP_AC(pAd->CommonCfg.PhyMode))
	{
		/* Clear previous VHT information */
		return FALSE;
	}

	
	pAd->CommonCfg.vht_cent_ch = vht_cent_ch_freq(pAd, pAd->CommonCfg.Channel);
		
	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Config VHT parameters!vht_cent_ch = %d, vht_cent_ch2 = %d\n",
				__FUNCTION__, pAd->CommonCfg.vht_cent_ch, pAd->CommonCfg.vht_cent_ch2));
	return TRUE;
}

#endif /* DOT11_VHT_AC */
