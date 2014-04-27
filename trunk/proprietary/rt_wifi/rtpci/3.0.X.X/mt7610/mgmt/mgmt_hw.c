/*


*/


#include <rt_config.h>


INT dev_adjust_radio(RTMP_ADAPTER *pAd)
{
	struct hw_setting *hw_cfg = &pAd->hw_cfg, new_cfg;


	NdisZeroMemory(&new_cfg, sizeof(struct hw_setting));

	
	/* For all wdev, find the maximum inter-set */

	
	if (hw_cfg->bbp_bw != new_cfg.bbp_bw)
	{
		rtmp_bbp_set_bw(pAd, new_cfg.bbp_bw);
		hw_cfg->bbp_bw = new_cfg.bbp_bw;
	}

	if (hw_cfg->cent_ch != new_cfg.cent_ch)
	{
		UCHAR ext_ch = EXTCHA_NONE;
		
		rtmp_bbp_set_ctrlch(pAd, ext_ch);
		rtmp_mac_set_ctrlch(pAd, ext_ch);	
	}

	return TRUE;
}

