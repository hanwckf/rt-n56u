#include "rt_config.h"
#include "hw_ctrl.h"

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
/*---------------------------------------------------------------------*/
/* WIFI and MD Coexistence Realize                                     */
/*---------------------------------------------------------------------*/

/* realize the wifi md coex_tx event func*/
int wifi_md_coex_tx_event(struct notifier_block *nb, unsigned long event, void *msg)
{
	struct _COEX_APCCCI2FW_CMD *p_coex_apccci2fw_cmd;
	struct _RTMP_ADAPTER *pAd;

	switch (event) {
	case APCCCI_DRIVER_FW:
	{
		if (msg == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:received NULL data in notifier chain!!", __func__));
			return NOTIFY_DONE;
		}
		/* get the structure address*/
		p_coex_apccci2fw_cmd = CONTAINER_OF(nb, struct _COEX_APCCCI2FW_CMD, coex_apccci2fw_notifier);
		pAd = p_coex_apccci2fw_cmd->priv;
		/* send cmd to FW*/
		HW_WIFI_COEX_APCCCI2FW(pAd, msg);
	}
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

/* register */
int register_wifi_md_coex(struct _RTMP_ADAPTER *pAd)
{
	pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier.notifier_call = wifi_md_coex_tx_event;
	pAd->coex_apccci2fw_cmd.priv = pAd;
	return register_wifi_md_coex_notifier(&pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier);
}

/* unregister */
int unregister_wifi_md_coex(struct _RTMP_ADAPTER *pAd)
{
	int err = 0;

	err = unregister_wifi_md_coex_notifier(&pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier);
	pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier.notifier_call = NULL;
	pAd->coex_apccci2fw_cmd.priv = NULL;
	return err;
}
#endif /* WIFI_MD_COEX_SUPPORT */

