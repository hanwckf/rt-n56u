/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ap_rls.c
*/

#ifdef RADIO_LINK_SELECTION
#include "rt_config.h"

extern RLS_OPS D_RlsOps;

BOOLEAN DaemonEnabled = FALSE;;	
UINT32	DaemonPid = 0xffffffff;;

/* static function */
static INT Check_All_Inf_State (PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	int i;
	struct wifi_dev *pMbss_wifi_dev = NULL;

	if (!wdev)
		return -1;

	for (i=0; i<pAd->ApCfg.BssidNum; i++)
	{
		pMbss_wifi_dev = &pAd->ApCfg.MBSSID[i].wdev;

		if (pMbss_wifi_dev &&
			pMbss_wifi_dev != wdev &&
			pMbss_wifi_dev->channel == wdev->channel &&
			pMbss_wifi_dev->bAllowBeaconing == TRUE)
				return i;
	}
	
	return -1;

}

static INT Check_All_Cli_State (PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	int ifIndex;
	APCLI_STRUCT *apcli_entry;

	if (!wdev)
		return -1;

	for (ifIndex=0; ifIndex<MAX_APCLI_NUM; ifIndex++)
	{
		apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];

		if (apcli_entry &&
			apcli_entry->wdev.channel == wdev->channel &&
			wdev != &apcli_entry->wdev &&
			wlan_operate_get_state(&apcli_entry->wdev) != WLAN_OPER_STATE_INVALID)
			return ifIndex;

	}
	return -1;
}

static void SetInfraInfDown(PRTMP_ADAPTER pAd, UCHAR Channel)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, /* TRACE */
			("%s(): Channel = %d.\n",__FUNCTION__, Channel));

	table->bTiggerByUser = TRUE;

	if (Channel > 14)
	{
		APStopByRf(pAd, RFIC_5GHZ);	
	} 
	else
		APStopByRf(pAd, RFIC_24GHZ);	
	
	table->bTiggerByUser = FALSE;

}

static void SetInfraInfUp(PRTMP_ADAPTER pAd, UCHAR Channel)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, /* TRACE */
			("%s(): Channel = %d.\n",__FUNCTION__, Channel));

	table->bTiggerByUser = TRUE;
	

	if (Channel > 14)
	{
		APStartUpByRf(pAd, RFIC_5GHZ);
	}
	else
		APStartUpByRf(pAd, RFIC_24GHZ);

	table->bTiggerByUser = FALSE;
	
}

static INT D_RlsSendMsg(
	PRTMP_ADAPTER pAd,
	UCHAR *msg,
	UINT32 size)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	if (table->bInitialized == FALSE)
		return RLS_NOT_INITIALIZED;

	if (DaemonEnabled)
		return	RtmpOSWrielessEventSend(
					pAd->net_dev,
					RT_WLAN_EVENT_CUSTOM,
					OID_RLS_MSG,
					NULL,
					(UCHAR *) msg,
					size);

	return RLS_SUCCESS;
}

static INT Rls_DaemonEnable(PRTMP_ADAPTER pAd, PRLS_CLI_TABLE table, BOOLEAN enable, UCHAR state)
{
	UCHAR ifIndex;

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, /* TRACE */
			("%s(): enable = %d. state(%d)\n",__FUNCTION__, enable, state));	

	if (table == NULL)
		return RLS_FAILURE;

	if (!(DaemonEnabled ^ enable))
	{
		/* Already enabled/disabled */
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, /* TRACE */
				("%s(): Radio Link Selection is already %s.\n",
				__FUNCTION__, (enable ? "enabled" : "disabled")));
		return RLS_SUCCESS;
	}

	if (enable)
	{
		DaemonEnabled = TRUE;
	}
	else
	{
#ifdef APCLI_AUTO_CONNECT_SUPPORT	
		if (state == RLS_SCAN_STATE)
		{
			for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
				pAd->ApCfg.ApCliAutoConnectType[ifIndex] = TRIGGER_SCAN_BY_USER;
		}
#endif
		APStopByRf(pAd, RFIC_5GHZ);
		APStartUpByRf(pAd, RFIC_5GHZ);

		DaemonEnabled = FALSE;
	}

	return RLS_SUCCESS;
}

static VOID Rls_CliScanState(PRTMP_ADAPTER pAd, union rls_msg *msg)
{

#ifdef APCLI_SUPPORT
	UCHAR ifIndex;
	APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev = NULL;
	NDIS_802_11_SSID Ssid;
	PRLS_CLI_TABLE table = P_RLS_TABLE;
	
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): \n", __FUNCTION__));

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];

		if (apcli_entry->ApCliInit != TRUE)
		   continue;

		wdev = &apcli_entry->wdev;

		if (wdev && !strcmp(msg->rls_inf_msg.ucIfName, wdev->if_dev->name))
		{
			if (APCLI_IF_UP_CHECK(pAd, ifIndex) &&
				pAd->ScanCtrl.PartialScan.bScanning == FALSE)
			{
				Set_ApCli_Enable_Proc(pAd, "0");
				SetInfraInfDown(pAd, wdev->channel);
				NdisCopyMemory(Ssid.Ssid, &apcli_entry->CfgSsid, apcli_entry->CfgSsidLen);
				Ssid.SsidLength = apcli_entry->CfgSsidLen ;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
				apcli_entry->AutoConnectFlag = TRUE;
				pAd->ApCfg.ApCliAutoConnectRunning[ifIndex] = TRUE;
				pAd->ApCfg.ApCliAutoBWAdjustCnt[ifIndex] = 0;
				pAd->ApCfg.ApCliAutoConnectType[ifIndex] = TRIGGER_SCAN_BY_DRIVER;	
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

				table->status = RLS_SCAN_STATE;
			}
		}
	}
#endif /* APCLI_SUPPORT */
}

static VOID Rls_CliDisable(PRTMP_ADAPTER pAd, union rls_msg *msg)
{
	int ifIndex;
	APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s():\n", __FUNCTION__));	

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];

		if (apcli_entry->ApCliInit != TRUE)
		   continue;

		wdev = &apcli_entry->wdev;

		if (wdev && (msg->rls_inf_msg.Channel == wdev->channel))
		{
			pAd->ApCfg.ApCliTab[ifIndex].Enable = FALSE;

#ifdef APCLI_AUTO_CONNECT_SUPPORT
			pAd->ApCfg.ApCliAutoConnectType[ifIndex] = TRIGGER_SCAN_BY_USER;
#endif	
					
		}
	}
	ApCliIfDown(pAd);
	
}

/* ioctl */
INT Show_Rls_Info(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	if (table->Ops)
		table->Ops->ShowTableInfo(pAd, table);

	return TRUE;	
}

INT Set_Rls_Enable(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;
	BOOLEAN enable = (BOOLEAN) simple_strtol(arg, 0, 10);
	INT ret_val = RLS_SUCCESS;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): enable = %u\n", __FUNCTION__,  enable));

	pAd->ApCfg.RadioLinkSelection = enable;

	if (!(table->bInitialized ^ enable))
	{
		/* Already enabled/disabled */
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, /* TRACE */
				("%s(): Radio Link Selection is already %s.\n",
				__FUNCTION__, (enable ? "enabled" : "disabled")));
		return TRUE;
	}

	if (enable)
		ret_val = Rls_Init(pAd);
	else
	{
		ret_val = Rls_Release(pAd);
		APStopByRf(pAd, RFIC_5GHZ);
		APStartUpByRf(pAd, RFIC_5GHZ);
	}

	if (ret_val != RLS_SUCCESS)
	{
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return TRUE;

}

INT Set_Rls_Period(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	union rls_msg msg;
	PRLS_CLI_TABLE table = P_RLS_TABLE;
	UCHAR time = simple_strtol(arg, 0, 10);


	if (table == NULL)
		return RLS_FAILURE;

	if (table->bInitialized == FALSE)
		return RLS_NOT_INITIALIZED;

	msg.rls_period_msg.Action = CLI_SCAN_PERIOD_TIME;
	msg.rls_period_msg.Time = time;

	D_RlsSendMsg(pAd, (UCHAR*)&msg, sizeof(union rls_msg));

	return TRUE;
}


/*Global Function*/
void Rls_UpdateTableChannel(PRTMP_ADAPTER pAd, INT old_channel, INT new_channel)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;
	union rls_msg msg;

	if (!table)
		return;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("%s():old_channel = (%d) new_channel = (%d)\n", __FUNCTION__, old_channel, new_channel));	

	if (old_channel != new_channel)
	{
		if (old_channel > 14)
		{
			if (table->Inf_5G.Channel == old_channel)
				table->Inf_5G.Channel = new_channel;

			if (table->Apcli_5G.Channel == old_channel)
				table->Apcli_5G.Channel = new_channel;
		}
		else
		{
			if (table->Inf_2G.Channel == old_channel)
				table->Inf_2G.Channel = new_channel;

			if (table->Apcli_2G.Channel == old_channel)
				table->Apcli_2G.Channel = new_channel;

		}

		msg.rls_channel_update_msg.Action = INF_CHANNEL_UPDATE;
		msg.rls_channel_update_msg.Old_Channel = old_channel;	
		msg.rls_channel_update_msg.New_Channel = new_channel;
		D_RlsSendMsg(pAd, (UCHAR*)&msg, sizeof(union rls_msg));
	}
}

void Rls_UpdateDevOpMode(PRTMP_ADAPTER pAd, BOOLEAN enable, struct wifi_dev *wdev)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;
	union rls_msg msg;
	bool bSetting = FALSE;

	if (!wdev || !table)
		return ;

	if (!(table->bInitialized))
	{
		/* Already enabled/disabled */
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, /* TRACE */
				("%s(): Radio Link Selection is %s.\n",
				__FUNCTION__, (table->bInitialized ? "Initialized" : "UNInitialized")));
		return ;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s(): enable (%d)\n", __FUNCTION__, enable));

	if (wdev->channel > 14)
	{
		/* update operation mode  */
		if(table->Inf_5G.bInfOp ^ enable && 
			table->Inf_5G.bInfReady == TRUE &&
			wdev->wdev_type == WDEV_TYPE_AP)
		{
			table->Inf_5G.bInfOp = enable;
			bSetting = TRUE;
		}
	}
	else
	{
		/* update operation mode  */
		if(table->Inf_2G.bInfOp ^ enable && 
			table->Inf_2G.bInfReady == TRUE &&
			wdev->wdev_type == WDEV_TYPE_AP)
		{
			table->Inf_2G.bInfOp = enable;
			bSetting = TRUE;
		}
	}

	if (bSetting)
	{
		msg.rls_opmode_update_msg.Action = INF_OPMODE_UPDATE;
		msg.rls_opmode_update_msg.Channel = wdev->channel;
		msg.rls_opmode_update_msg.OpMode = enable;
		D_RlsSendMsg(pAd, (UCHAR*)&msg, sizeof(union rls_msg));
	}

}


INT Rls_SetInfInfo(PRTMP_ADAPTER pAd, BOOLEAN bInfReady, struct wifi_dev *wdev)
{
	INT ret_val = RLS_SUCCESS;
	INT ret, ret_cli;
	enum ACT_CODE action_code; 
	RLS_INF_INFO *inf_info = NULL;
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	if (!wdev || !table)
		return RLS_FAILURE;

	if (wdev->channel > 14)
	{
		action_code = INF_STATUS_5G_RSP;

		/* Announce 5G interface status  */
		if(table->Inf_5G.bInfReady ^ bInfReady && 
			wdev->wdev_type == WDEV_TYPE_AP)
		{
			inf_info = &table->Inf_5G;
			
			if (bInfReady)
			{
				table->Band |= BAND_5G;
				inf_info->bInfReady = bInfReady;
				inf_info->bInfOp = bInfReady;
			}
			else
			{
				ret = Check_All_Inf_State(pAd, wdev);

				/* If have no other ap inface at same channel , bInfReady is FALSE */
				if (ret<0)
				{
					inf_info->bInfReady = bInfReady;
					table->Band &= ~BAND_5G;
				}
				else
					return RLS_SUCCESS;
			}
		}
		/* Announce 5G apcli interface status  */
		else if(table->Apcli_5G.bInfReady ^ bInfReady && 
			wdev->wdev_type == WDEV_TYPE_APCLI)
		{
			inf_info = &table->Apcli_5G;

			if (bInfReady)
			{
				table->Band |= BAND_5G;
				inf_info->bInfReady = bInfReady;
			}
			else
			{
				/* If have no other apcli inface at same channel , bInfReady is FALSE */
				ret_cli = Check_All_Cli_State(pAd, wdev);

				if (ret_cli<0)
				{
					inf_info->bInfReady = bInfReady;
					table->Band &= ~BAND_5G;
				}
				else
					return RLS_SUCCESS;
			}
		}
	}
	else 
	{
		action_code = INF_STATUS_2G_RSP;

		/* Announce 2G interface status  */
		if ((table->Inf_2G.bInfReady ^ bInfReady) && 
			wdev->wdev_type == WDEV_TYPE_AP)
		{
			inf_info = &table->Inf_2G;

			if (bInfReady)
			{
				table->Band |= BAND_24G;
				inf_info->bInfReady = bInfReady;
				inf_info->bInfOp = bInfReady;
			}
			else
			{
				ret = Check_All_Inf_State(pAd, wdev);

				/* If have no other ap inface at same channel , bInfReady is FALSE */
				if (ret<0)
				{
					inf_info->bInfReady = bInfReady;
					table->Band &= ~BAND_24G;
				}
				else
					return RLS_SUCCESS;
			}	
		}
		/* Announce 2G apcli interface status  */
		else if(table->Apcli_2G.bInfReady ^ bInfReady && 
			wdev->wdev_type == WDEV_TYPE_APCLI)
		{
			inf_info = &table->Apcli_2G;

			if (bInfReady)
			{
				table->Band |= BAND_5G;
				inf_info->bInfReady = bInfReady;
			}
			else
			{
				ret_cli = Check_All_Cli_State(pAd, wdev);

				/* If have no other apcli inface at same channel , bInfReady is FALSE */
				if (ret_cli<0)
				{
					inf_info->bInfReady = bInfReady;
					table->Band &= ~BAND_24G;
				}
				else
					return RLS_SUCCESS;
			}
		}
	}

	if (inf_info)
	{
		union rls_msg msg;
	
		inf_info->Channel = wdev->channel;
	
		if (wdev->if_dev)
			strcpy(inf_info->ucIfName, wdev->if_dev->name);
			
		msg.rls_inf_msg.Action = action_code;
		msg.rls_inf_msg.ucType	  = wdev->wdev_type;
		msg.rls_inf_msg.Channel   = inf_info->Channel;	
		msg.rls_inf_msg.bInfReady = inf_info->bInfReady;
		msg.rls_inf_msg.bInfOp = inf_info->bInfOp;
		msg.rls_inf_msg.isUP = bInfReady;
		strcpy(msg.rls_inf_msg.ucIfName, inf_info->ucIfName);

		if (bInfReady)
			inf_info->wdev = wdev;
		else
			inf_info->wdev = NULL;

		D_RlsSendMsg(pAd, (UCHAR*)&msg, sizeof(union rls_msg));
	}

	return ret_val;
}

/*
	When Apcli link to Root-AP
	Notice to Daemon
*/
INT Rls_InfCliLinkUp(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	if (table && table->Ops)
		table->Ops->CliLinkRsp(pAd,
							wdev,
							TRUE);

	return TRUE;
}

/*
	When Apcli link down from Root-AP
	Notice to Daemon
*/
INT Rls_InfCliLinkDown(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	if (table && table->Ops)
		table->Ops->CliLinkRsp(pAd,
							wdev,
							FALSE);
	return TRUE;
}

INT Rls_TableInit(PRTMP_ADAPTER pAd, PRLS_CLI_TABLE table)
{

	struct wifi_dev *wdev = NULL;
	INT apidx;
#ifdef APCLI_SUPPORT
	UCHAR ifIndex;
	APCLI_STRUCT *pApCliEntry;
#endif /* APCLI_SUPPORT */

	ASSERT(pAd != NULL);

	if (table->bInitialized == TRUE)
		return RLS_SUCCESS;

	NdisZeroMemory(table, sizeof(RLS_CLI_TABLE));
	table->bInitialized = TRUE;

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) 
	{
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		Rls_SetInfInfo(pAd, TRUE, wdev);
	}

#ifdef APCLI_SUPPORT
	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
        /* sanity check whether the interface is initialized. */
	    if (pApCliEntry->ApCliInit != TRUE)
		    continue;

		if(!HcIsRadioAcq(&pApCliEntry->wdev))
			continue;
		
		if (APCLI_IF_UP_CHECK(pAd, ifIndex))
		{
			Rls_SetInfInfo(pAd, TRUE, &pApCliEntry->wdev);

			if (pApCliEntry->Valid == TRUE)
				Rls_InfCliLinkUp(pAd, &pApCliEntry->wdev);
		}
	}
#endif /* APCLI_SUPPORT */

	table->Ops = &D_RlsOps;
	table->priv = (VOID *) pAd;

	return RLS_SUCCESS;
}

INT Rls_Init(PRTMP_ADAPTER pAd)
{
	INT ret_val = RLS_SUCCESS;
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()\n", __FUNCTION__));

	ret_val = Rls_TableInit(pAd, table);

	if (ret_val != RLS_SUCCESS)
	{
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}


INT Rls_Release(PRTMP_ADAPTER pAd)
{
	INT ret_val = RLS_SUCCESS;
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()\n", __FUNCTION__));

	ret_val = Rls_TableRelease(pAd, table);

	if (ret_val != RLS_SUCCESS)
	{
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}


INT Rls_TableRelease(PRTMP_ADAPTER pAd, PRLS_CLI_TABLE table)
{
	INT ret_val = RLS_SUCCESS;

	if (!table)
		return RLS_FAILURE;

	if (table->bInitialized == FALSE)
		return RLS_NOT_INITIALIZED;

	if (table->Inf_2G.bInfReady)
		Rls_SetInfInfo(pAd, FALSE, table->Inf_2G.wdev);

	if (table->Inf_5G.bInfReady)
		Rls_SetInfInfo(pAd, FALSE, table->Inf_5G.wdev);

	if (table->Apcli_2G.bInfReady)
		Rls_SetInfInfo(pAd, FALSE, table->Apcli_2G.wdev);	

	if (table->Apcli_5G.bInfReady)
		Rls_SetInfInfo(pAd, FALSE, table->Apcli_5G.wdev);

	NdisZeroMemory(table, sizeof(RLS_CLI_TABLE));
	table->bInitialized = FALSE;

	if (ret_val != RLS_SUCCESS)
	{
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}



static VOID D_ShowTableInfo(PRTMP_ADAPTER pAd, PRLS_CLI_TABLE table)
{
	union rls_msg msg;
	
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("-------(%s)-------\n", __FUNCTION__));

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bInitialized = %s\n", table->bInitialized? "TRUE":"FALSE"));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bEnabled = %s\n", DaemonEnabled? "TRUE":"FALSE"));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band = %d\n", table->Band));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DaemonPid = %d\n", DaemonPid));

//--------------------------------------------------------------------------------------------------------------------------//
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("uc2GIfName = %s\n", table->Inf_2G.ucIfName));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel_2G = %d\n", table->Inf_2G.Channel));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("b2GInfReady = %s\n", table->Inf_2G.bInfReady? "TRUE":"FALSE"));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("b2GInfOp = %s\n\n\n", table->Inf_2G.bInfOp? "TRUE":"FALSE"));


	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("uc5GIfName = %s\n", table->Inf_5G.ucIfName));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel_5G = %d\n", table->Inf_5G.Channel));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("b5GInfReady = %s\n", table->Inf_5G.bInfReady? "TRUE":"FALSE"));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("b5GInfOp = %s\n\n\n", table->Inf_5G.bInfOp? "TRUE":"FALSE"));

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucApcli_2GIfName = %s\n", table->Apcli_2G.ucIfName));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel_Apcli_2G = %d\n", table->Apcli_2G.Channel));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bApcli_2GInfReady = %s\n", table->Apcli_2G.bInfReady? "TRUE":"FALSE"));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bApcli_2GInfOp = %s\n\n\n", table->Apcli_2G.bInfOp? "TRUE":"FALSE"));

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucApcli_5GIfName = %s\n", table->Apcli_5G.ucIfName));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel_Apcli_5G = %d\n", table->Apcli_5G.Channel));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bApcli_5GInfReady = %s\n", table->Apcli_5G.bInfReady? "TRUE":"FALSE"));
	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bApcli_5GInfOp = %s\n\n\n", table->Apcli_5G.bInfOp? "TRUE":"FALSE"));


	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("------------------\n"));

	msg.rls_action_msg.Action = DEBUG_INFORMATION;

	D_RlsSendMsg(pAd, (UCHAR*)&msg, sizeof(union rls_msg));
	

}

/* Daemon Relation */

/*
	1.Enable from profile
	2.Enable from command
*/
static INT D_SetToEnable(
			PRLS_CLI_TABLE table,
			BOOLEAN enable)
{
	INT ret_val = RLS_SUCCESS;

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s(): enable = %u\n", __FUNCTION__,  enable));


	pAd->ApCfg.RadioLinkSelection = enable;

	if (!(table->bInitialized ^ enable))
	{
		/* Already enabled/disabled */
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, /* TRACE */
				("%s(): Radio Link Selection is already %s.\n",
				__FUNCTION__, (enable ? "enabled" : "disabled")));
		return RLS_SUCCESS;
	}

	if (enable)
		ret_val = Rls_Init(pAd);
	else
		ret_val = Rls_Release(pAd);

	if (ret_val != RLS_SUCCESS)
	{
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return TRUE;
}

/*
	When receive INF_STATUS_QUERY_INFO from Daemon, we notice our interface information
*/
static INT D_InfStatRsp(
			PRTMP_ADAPTER	pAd,
			PRLS_CLI_TABLE table)
{

	union rls_msg msg;
	
	if (!table)
		return RLS_FAILURE;

	if (!(table->bInitialized))
	{
		/* Already enabled/disabled */
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, /* TRACE */
				("%s(): Radio Link Selection is %s.\n",
				__FUNCTION__, (table->bInitialized ? "Initialized" : "UNInitialized")));
		return RLS_SUCCESS;
	}

	memset(&msg, 0x00, sizeof(union rls_msg));

	msg.rls_inf_msg.isUP = TRUE;

	if (table->Inf_2G.bInfReady == TRUE)
	{
		msg.rls_inf_msg.Action = INF_STATUS_2G_RSP;
		msg.rls_inf_msg.Channel =table->Inf_2G.Channel;
		msg.rls_inf_msg.ucType	= WDEV_TYPE_AP;
		msg.rls_inf_msg.bInfReady = table->Inf_2G.bInfReady;
		msg.rls_inf_msg.bInfOp = table->Inf_2G.bInfOp;
		
		strcpy(msg.rls_inf_msg.ucIfName, table->Inf_2G.ucIfName);
		D_RlsSendMsg(pAd, (UCHAR *)&msg, sizeof(union rls_msg));
	}

	if (table->Inf_5G.bInfReady == TRUE)
	{
		msg.rls_inf_msg.Action = INF_STATUS_5G_RSP;
		msg.rls_inf_msg.Channel = table->Inf_5G.Channel;
		msg.rls_inf_msg.ucType	= WDEV_TYPE_AP;
		msg.rls_inf_msg.bInfReady = table->Inf_5G.bInfReady;
		msg.rls_inf_msg.bInfOp = table->Inf_5G.bInfOp;
	
		strcpy(msg.rls_inf_msg.ucIfName, table->Inf_5G.ucIfName);
		D_RlsSendMsg(pAd, (UCHAR *)&msg, sizeof(union rls_msg));
	}

	if (table->Apcli_2G.bInfReady == TRUE)
	{
		msg.rls_inf_msg.Action = INF_STATUS_2G_RSP;
		msg.rls_inf_msg.Channel = table->Apcli_2G.Channel;
		msg.rls_inf_msg.ucType	= WDEV_TYPE_APCLI;
		msg.rls_inf_msg.bInfReady = table->Apcli_2G.bInfReady;
		msg.rls_inf_msg.bInfOp = table->Apcli_2G.bInfOp;
		
		strcpy(msg.rls_inf_msg.ucIfName, table->Apcli_2G.ucIfName);
		D_RlsSendMsg(pAd, (UCHAR *)&msg, sizeof(union rls_msg));
	}

	if (table->Apcli_5G.bInfReady == TRUE)
	{
		msg.rls_inf_msg.Action = INF_STATUS_5G_RSP;
		msg.rls_inf_msg.Channel = table->Apcli_5G.Channel;
		msg.rls_inf_msg.ucType	= WDEV_TYPE_APCLI;
		msg.rls_inf_msg.bInfReady = table->Apcli_5G.bInfReady;
		msg.rls_inf_msg.bInfOp = table->Apcli_5G.bInfOp;
		
		strcpy(msg.rls_inf_msg.ucIfName, table->Apcli_5G.ucIfName);
		D_RlsSendMsg(pAd, (UCHAR *)&msg, sizeof(union rls_msg));

	}	

	return TRUE;


}

/*
	Notice to Daemon, when apcli linkup or linkdown
*/
static INT D_CliLinkRsp(
			PRTMP_ADAPTER	pAd,
			struct wifi_dev *wdev,
			BOOLEAN enable)
{

	PRLS_CLI_TABLE table = P_RLS_TABLE;
	union rls_msg msg;

	if (!wdev || !table)
		return RLS_FAILURE;

	if (!(table->bInitialized))
	{
		/* Already enabled/disabled */
		RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, /* TRACE */
				("%s(): Radio Link Selection is %s.\n",
				__FUNCTION__, (table->bInitialized ? "Initialized" : "UNInitialized")));
		return RLS_SUCCESS;
	}

	/* Set Apcli Linkup or LinkDown (bInfOp) */
	if (wdev->channel > 14)
	{
		if (table->Apcli_5G.bInfReady &&
			table->Apcli_5G.bInfOp ^ enable)
			table->Apcli_5G.bInfOp = enable;
		else
			return TRUE;
	}
	else 
	{
		if (table->Apcli_2G.bInfReady &&
			table->Apcli_2G.bInfOp ^ enable)
			table->Apcli_2G.bInfOp = enable;
		else
			return TRUE;
	}

	memset(&msg, 0x00, sizeof(union rls_msg));

	if (enable)
		msg.rls_cli_link_msg.Action = IF_CLI_LINKUP_RSP;
	else
		msg.rls_cli_link_msg.Action = IF_CLI_LINKDOWN_RSP;

	
	msg.rls_cli_link_msg.Channel = wdev->channel;
	msg.rls_cli_link_msg.ucType	= wdev->wdev_type;

	if (wdev->if_dev)
		strcpy(msg.rls_cli_link_msg.ucIfName, wdev->if_dev->name);
			
	D_RlsSendMsg(pAd, (UCHAR *)&msg, sizeof(union rls_msg));

	return TRUE;


}

/*
	Daemon msg handler
*/
static VOID D_MsgHandlePro(
			PRTMP_ADAPTER	pAd,
			//union rls_msg *msg)
			UCHAR *msg)
{
	PRLS_CLI_TABLE table = P_RLS_TABLE;

	RLS_MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: action code (%d)\n",__FUNCTION__, *msg));

	if (*msg == RLS_ONOFF)
	{
		Rls_DaemonEnable(pAd, table, ((union rls_msg *)msg)->rls_on_off_msg.bEnable, ((union rls_msg *)msg)->rls_on_off_msg.state);

		if (((union rls_msg *)msg)->rls_on_off_msg.bEnable)
			DaemonPid = current->pid;
		else 
			DaemonPid = 0xffffffff;
	}
	
	if (!table || !table->bInitialized)
		return;

	if ((DaemonPid!= 0xffffffff) && (DaemonPid != current->pid))
		return;
	
	switch (*msg)
	{
		case INF_STATUS_QUERY_INFO:
			D_InfStatRsp(pAd, table);
			break;

		case IF_CLI_SCAN_REQ:
			Rls_CliScanState(pAd, (union rls_msg *) msg);
			break;

		case IF_CLI_DISABLE_REQ:
			Rls_CliDisable(pAd, (union rls_msg *) msg);
			break;

		case INF_OFF:
			SetInfraInfDown(pAd, ((union rls_msg *)msg)->rls_inf_msg.Channel);
			break;	

		case INF_ON:
			SetInfraInfUp(pAd, ((union rls_msg *)msg)->rls_inf_msg.Channel);
			break;	

		case RLS_ONOFF:
			break;

		case INF_RESET:
			APStopByRf(pAd, RFIC_5GHZ);
			APStartUpByRf(pAd, RFIC_5GHZ);
			break;	

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknown action code. (%d)\n",__FUNCTION__, *msg));
			break;
	}
}

INT Rls_MsgHandlePro(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	union rls_msg msg;

	if (!wrq)
		return RLS_FAILURE;

	copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length);
	D_MsgHandlePro(pAd, (UCHAR *)&msg);

	return RLS_SUCCESS;
}


RLS_OPS D_RlsOps = {
	/* ioctl */
	.SetEnable = D_SetToEnable,
	.ShowTableInfo = D_ShowTableInfo,
	.CliLinkRsp = D_CliLinkRsp,
};
#endif /* RADIO_LINK_SELECTION */

