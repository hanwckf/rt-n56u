/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	cmm_cmd.c

	Abstract:
	All command related API.

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
	Paul Lin    06-25-2004  created
*/

#include "rt_config.h"




/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
VOID	RTInitializeCmdQ(
	IN	PCmdQ	cmdq)
{
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
VOID	RTThreadDequeueCmd(
	IN	PCmdQ		cmdq,
	OUT	PCmdQElmt	*pcmdqelmt)
{
	*pcmdqelmt = cmdq->head;

	if (*pcmdqelmt != NULL)
	{
		cmdq->head = cmdq->head->next;
		cmdq->size--;
		if (cmdq->size == 0)
			cmdq->tail = NULL;
	}
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
NDIS_STATUS RTEnqueueInternalCmd(
	IN PRTMP_ADAPTER	pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength)
{
	NDIS_STATUS	status;
	ULONG	flag = 0;
	PCmdQElmt	cmdqelmt = NULL;


	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->RTEnqueueInternalCmd - NIC is not exist!!\n"));
		return NDIS_STATUS_FAILURE;
	}

	status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt, sizeof(CmdQElmt));
	if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt == NULL))
		return (NDIS_STATUS_RESOURCES);
	NdisZeroMemory(cmdqelmt, sizeof(CmdQElmt));

	if(InformationBufferLength > 0)
	{
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, InformationBufferLength);
		if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt->buffer == NULL))
		{
			os_free_mem(cmdqelmt);
			return (NDIS_STATUS_RESOURCES);
		}
		else
		{
			NdisMoveMemory(cmdqelmt->buffer, pInformationBuffer, InformationBufferLength);
			cmdqelmt->bufferlength = InformationBufferLength;
		}
	}
	else
	{
		cmdqelmt->buffer = NULL;
		cmdqelmt->bufferlength = 0;
	}

	cmdqelmt->command = Oid;
	cmdqelmt->CmdFromNdis = FALSE;

	if (cmdqelmt != NULL)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&pAd->CmdQLock,&flag);
		if (pAd->CmdQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)
		{
			EnqueueCmd((&pAd->CmdQ), cmdqelmt);
			status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			status = NDIS_STATUS_FAILURE;
		}
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CmdQLock,&flag);

		if (status == NDIS_STATUS_FAILURE)
		{
			if (cmdqelmt->buffer)
				os_free_mem(cmdqelmt->buffer);
			os_free_mem(cmdqelmt);
		}
		else
			RTCMDUp(&pAd->cmdQTask);
	}
	return(NDIS_STATUS_SUCCESS);
}




/*Define common Cmd Thread*/







#ifdef CONFIG_AP_SUPPORT
static NTSTATUS _802_11_CounterMeasureHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MAC_TABLE_ENTRY *pEntry;

		pEntry = (MAC_TABLE_ENTRY *)CMDQelmt->buffer;
		HandleCounterMeasure(pAd, pEntry);
	}

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS ApSoftReStart(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR RfIC;
	NdisMoveMemory(&RfIC , CMDQelmt->buffer, sizeof(UCHAR));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("cmd> ApSoftReStart: RfIC = %d\n", RfIC));
	APStopByRf(pAd, RfIC);
	APStartUpByRf(pAd, RfIC);
	return NDIS_STATUS_SUCCESS;
}

#ifdef APCLI_SUPPORT
static NTSTATUS ApCliSetChannel(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PAPCLI_STRUCT pApCliTab = (PAPCLI_STRUCT)CMDQelmt->buffer;
	UCHAR channel = 0;

	channel = pApCliTab->MlmeAux.Channel;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cmd> channel=%d CMDTHREAD_APCLI_PBC_TIMEOUT! \n",channel));
#ifdef WH_EZ_SETUP
	if(IS_EZ_SETUP_ENABLED(&pApCliTab->wdev))
	{
#ifdef EZ_MOD_SUPPORT	
		pApCliTab->wdev.ez_driver_params.do_not_restart_interfaces = TRUE;
#else
		pApCliTab->wdev.ez_security.do_not_restart_interfaces = TRUE;
#endif
	}
#endif
	rtmp_set_channel(pAd, &pApCliTab->wdev, channel);
#ifdef WH_EZ_SETUP
	if(IS_EZ_SETUP_ENABLED(&pApCliTab->wdev))
	{
#ifdef EZ_MOD_SUPPORT	
		pApCliTab->wdev.ez_driver_params.do_not_restart_interfaces = FALSE;
#else
		pApCliTab->wdev.ez_security.do_not_restart_interfaces = FALSE;
#endif
	}
#endif

	return NDIS_STATUS_SUCCESS;	
}

static NTSTATUS CmdApCliIfDown(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR apidx = 0;
	BOOLEAN apcliEn;
	
	NdisMoveMemory(&apidx , CMDQelmt->buffer, sizeof(UCHAR));
	
	apcliEn = pAd->ApCfg.ApCliTab[apidx].Enable;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cmd>  CMDTHREAD_APCLI_IF_DOWN! apidx=%u, apcliEn=%d\n",apidx,apcliEn));

	/* bring apcli interface down first */
	if(apcliEn == TRUE )
	{
		pAd->ApCfg.ApCliTab[apidx].Enable = FALSE;
		ApCliIfDown(pAd);
	}

	pAd->ApCfg.ApCliTab[apidx].Enable = apcliEn;
	
	return NDIS_STATUS_SUCCESS;	
}

#ifdef WSC_AP_SUPPORT
static NTSTATUS CmdWscApCliLinkDown(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR apidx = 0;
	NdisMoveMemory(&apidx , CMDQelmt->buffer, sizeof(UCHAR));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cmd>  CMDTHREAD_WSC_APCLI_LINK_DOWN! apidx=%u\n",apidx));
	WscApCliLinkDownById(pAd,apidx);
	return NDIS_STATUS_SUCCESS;	
}
#endif /* WSC_AP_SUPPORT */

#endif /* APCLI_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */




#ifdef CONFIG_AP_SUPPORT
static NTSTATUS ChannelRescanHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	/*SUPPORT RTMP_CHIP ONLY, Single Band*/
	UCHAR Channel = HcGetRadioChannel(pAd);
	UCHAR PhyMode = HcGetRadioPhyMode(pAd);
	UCHAR RfIC = wmode_2_rfic(PhyMode);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	Channel = AP_AUTO_CH_SEL(pAd, TRUE,pAutoChCtrl->pChannelInfo->IsABand);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cmd> Re-scan channel! \n"));
#ifdef DOT11_N_SUPPORT
	/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
	N_ChannelCheck(pAd,PhyMode,Channel);
#endif /* DOT11_N_SUPPORT */

	HcUpdateChannel(pAd,Channel);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cmd> Switch to %d! \n", Channel));
	APStopByRf(pAd, RfIC);
	APStartUpByRf(pAd, RfIC);

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadAlarmResume(pAd);
#endif /* AP_QLOAD_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT*/


#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
static NTSTATUS RegHintHdlr (RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RegHint11DHdlr(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT11D(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RT_Mac80211_ScanEnd(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_SCAN_END(pAd, FALSE);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RT_Mac80211_ConnResultInfom(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */



#ifdef STREAM_MODE_SUPPORT
static NTSTATUS UpdateTXChainAddress(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	AsicUpdateTxChainAddress(pAd, CMDQelmt->buffer);
	return NDIS_STATUS_SUCCESS;
}
#endif /* STREAM_MODE_SUPPORT */


#ifdef CFG_TDLS_SUPPORT

static NTSTATUS CFGTdlsSendCHSWSetupHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS CFGTdlsAutoTeardownHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)(CMDQelmt->buffer);
	cfg_tdls_auto_teardown(pAd, pEntry);
	return NDIS_STATUS_SUCCESS;
}

#endif /* CFG_TDLS_SUPPORT */


typedef NTSTATUS (*CMDHdlr)(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt);


typedef struct {
	UINT32 CmdID;
	CMDHdlr CmdHdlr;
}MT_CMD_TABL_T;

static MT_CMD_TABL_T CMDHdlrTable[] = {

	/*STA related*/
	/*AP related*/
#ifdef CONFIG_AP_SUPPORT
	{CMDTHREAD_CHAN_RESCAN,ChannelRescanHdlr},
	{CMDTHREAD_802_11_COUNTER_MEASURE,_802_11_CounterMeasureHdlr},
	{CMDTHREAD_AP_RESTART, ApSoftReStart},
#ifdef APCLI_SUPPORT
	{CMDTHREAD_APCLI_PBC_TIMEOUT, ApCliSetChannel},
	{CMDTHREAD_APCLI_IF_DOWN, CmdApCliIfDown},
#ifdef WSC_AP_SUPPORT
	{CMDTHREAD_WSC_APCLI_LINK_DOWN, CmdWscApCliLinkDown},
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif
	/*CFG 802.11*/
#if  defined(LINUX) && defined(RT_CFG80211_SUPPORT)
	{CMDTHREAD_REG_HINT, RegHintHdlr},
	{CMDTHREAD_REG_HINT_11D,RegHint11DHdlr},
	{CMDTHREAD_SCAN_END,RT_Mac80211_ScanEnd},
	{CMDTHREAD_CONNECT_RESULT_INFORM,RT_Mac80211_ConnResultInfom},
#endif
	/*P2P*/
	/*RT3593*/
#ifdef STREAM_MODE_SUPPORT
	{CMDTHREAD_UPDATE_TX_CHAIN_ADDRESS,UpdateTXChainAddress},
#endif
/*TDLS*/
#ifdef CFG_TDLS_SUPPORT
	{CMDTHREAD_TDLS_SEND_CH_SW_SETUP,CFGTdlsSendCHSWSetupHdlr},
	{CMDTHREAD_TDLS_AUTO_TEARDOWN,CFGTdlsAutoTeardownHdlr},
#endif
#ifdef INTERNAL_CAPTURE_SUPPORT
    {CMDTHRED_WIFISPECTRUM_RAWDATA_DUMP, WifiSpectrumRawDataDumpHandler},
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	{CMDTHRED_PRECAL_TXLPF, PreCalTxLPFStoreProcHandler},
	{CMDTHRED_PRECAL_TXIQ, PreCalTxIQStoreProcHandler},
	{CMDTHRED_PRECAL_TXDC, PreCalTxDCStoreProcHandler},
	{CMDTHRED_PRECAL_RXFI, PreCalRxFIStoreProcHandler},
	{CMDTHRED_PRECAL_RXFD, PreCalRxFDStoreProcHandler},
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */ 	    
#endif/*INTERNAL_CAPTURE_SUPPORT*/
#ifdef CONFIG_AP_SUPPORT
	{CMDTHRED_DOT11H_SWITCH_CHANNEL, Dot11HCntDownTimeoutAction},
#endif	
	{CMDTHREAD_END_CMD_ID,NULL}
};

static inline CMDHdlr ValidCMD(IN PCmdQElmt CMDQelmt)
{
	SHORT CMDIndex = CMDQelmt->command;
	SHORT CurIndex = 0;
	USHORT CMDHdlrTableLength= sizeof(CMDHdlrTable) / sizeof(MT_CMD_TABL_T);
	CMDHdlr Handler = NULL;

	if(CMDIndex > CMDTHREAD_END_CMD_ID)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMD(%x) is out of boundary\n", CMDQelmt->command));
		return NULL;
	}

	for(CurIndex=0; CurIndex < CMDHdlrTableLength; CurIndex++)
	{
		if (CMDHdlrTable[CurIndex].CmdID == CMDIndex)
		{
			Handler = CMDHdlrTable[CurIndex].CmdHdlr;
			break;
		}
	}

	if(Handler == NULL)
	{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("No corresponding CMDHdlr for this CMD(%x)\n",  CMDQelmt->command));
	}
	return Handler;
}


VOID CMDHandler(RTMP_ADAPTER *pAd)
{
	PCmdQElmt		cmdqelmt;
	NDIS_STATUS	NdisStatus = NDIS_STATUS_SUCCESS;
	NTSTATUS		ntStatus;
	CMDHdlr 		Handler = NULL;

	while (pAd && pAd->CmdQ.size > 0)
	{
		NdisStatus = NDIS_STATUS_SUCCESS;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTThreadDequeueCmd(&pAd->CmdQ, &cmdqelmt);
		NdisReleaseSpinLock(&pAd->CmdQLock);

		if (cmdqelmt == NULL)
			break;


		if(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		{
			Handler = ValidCMD(cmdqelmt);
			if(Handler)
			{
				ntStatus = Handler(pAd, cmdqelmt);
			}
		}

		if (cmdqelmt->CmdFromNdis == TRUE)
		{
			if (cmdqelmt->buffer != NULL)
			{
				os_free_mem(cmdqelmt->buffer);
			}
			os_free_mem(cmdqelmt);
		}
		else
		{
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
			{
				os_free_mem(cmdqelmt->buffer);
			}
			os_free_mem(cmdqelmt);
		}
	}	/* end of while */
}

void RtmpCmdQExit(RTMP_ADAPTER *pAd)
{
	/* WCNCR00034259: unify CmdQ init and exit. But cleanup is done by
	 * RTUSBCmdThread() before its exit.
	 */
	return;
}


void RtmpCmdQInit(RTMP_ADAPTER *pAd)
{
	/* WCNCR00034259: moved from RTMP{Init, Alloc}TxRxRingMemory() */
	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->CmdQLock);
	NdisAcquireSpinLock(&pAd->CmdQLock);
	RTInitializeCmdQ(&pAd->CmdQ);
	NdisReleaseSpinLock(&pAd->CmdQLock);
}

