/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mlme.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang	2004-08-25		Modify from RT2500 code base
	John Chang	2004-09-06		modified for RT2600
*/

#include "rt_config.h"
#include <stdarg.h>


UCHAR	CISCO_OUI[] = {0x00, 0x40, 0x96};

UCHAR   RALINK_OUI[]  = {0x00, 0x0c, 0x43};
UCHAR	WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
UCHAR	RSN_OUI[] = {0x00, 0x0f, 0xac};
UCHAR	WAPI_OUI[] = {0x00, 0x14, 0x72};
UCHAR   WME_INFO_ELEM[]  = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR   WME_PARM_ELEM[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR   BROADCOM_OUI[]  = {0x00, 0x90, 0x4c};
UCHAR   WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};
#ifdef CONFIG_STA_SUPPORT
#ifdef IWSC_SUPPORT
UCHAR   IWSC_OUI[] = {0x00, 0x50, 0xf2, 0x10};
UCHAR   IWSC_ACTION_OUI[] = {0x50, 0x6F, 0x9A, 0x10};
#endif /* IWSC_SUPPORT */
#ifdef DOT11_N_SUPPORT
UCHAR	PRE_N_HT_OUI[]	= {0x00, 0x90, 0x4c};
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
UINT32 CW_MAX_IN_BITS;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */




extern UCHAR	 OfdmRateToRxwiMCS[];
/* since RT61 has better RX sensibility, we have to limit TX ACK rate not to exceed our normal data TX rate.*/
/* otherwise the WLAN peer may not be able to receive the ACK thus downgrade its data TX rate*/
ULONG BasicRateMask[12]				= {0xfffff001 /* 1-Mbps */, 0xfffff003 /* 2 Mbps */, 0xfffff007 /* 5.5 */, 0xfffff00f /* 11 */,
									  0xfffff01f /* 6 */	 , 0xfffff03f /* 9 */	  , 0xfffff07f /* 12 */ , 0xfffff0ff /* 18 */,
									  0xfffff1ff /* 24 */	 , 0xfffff3ff /* 36 */	  , 0xfffff7ff /* 48 */ , 0xffffffff /* 54 */};

UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* e.g. RssiSafeLevelForTxRate[RATE_36]" means if the current RSSI is greater than*/
/*		this value, then it's quaranteed capable of operating in 36 mbps TX rate in*/
/*		clean environment.*/
/*								  TxRate: 1   2   5.5	11	 6	  9    12	18	 24   36   48	54	 72  100*/
CHAR RssiSafeLevelForTxRate[] ={  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

UCHAR  RateIdToMbps[]	 = { 1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54, 72, 100};
USHORT RateIdTo500Kbps[] = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 144, 200};

UCHAR  SsidIe	 = IE_SSID;
UCHAR  SupRateIe = IE_SUPP_RATES;
UCHAR  ExtRateIe = IE_EXT_SUPP_RATES;
#ifdef DOT11_N_SUPPORT
UCHAR  HtCapIe = IE_HT_CAP;
UCHAR  AddHtInfoIe = IE_ADD_HT;
UCHAR  NewExtChanIe = IE_SECONDARY_CH_OFFSET;
UCHAR  BssCoexistIe = IE_2040_BSS_COEXIST;
UCHAR  ExtHtCapIe = IE_EXT_CAPABILITY;
#endif /* DOT11_N_SUPPORT */
UCHAR  ExtCapIe = IE_EXT_CAPABILITY;
UCHAR  ErpIe	 = IE_ERP;
UCHAR  DsIe 	 = IE_DS_PARM;
UCHAR  TimIe	 = IE_TIM;
UCHAR  WpaIe	 = IE_WPA;
UCHAR  Wpa2Ie	 = IE_WPA2;
UCHAR  IbssIe	 = IE_IBSS_PARM;
UCHAR  WapiIe	 = IE_WAPI;

extern UCHAR	WPA_OUI[];

UCHAR	SES_OUI[] = {0x00, 0x90, 0x4c};

UCHAR	ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


#ifdef INF_AMAZON_SE	
UINT16 MaxBulkOutsSizeLimit[5][4] =
{
	/* Priority high -> low*/
	{ 24576, 2048, 2048, 2048 },	/* 0 AC	*/
	{ 24576, 2048, 2048, 2048 },	/* 1 AC	 */
	{ 24576, 2048, 2048, 2048 }, 	/* 2 ACs*/
	{ 24576, 6144, 2048, 2048 }, 	/* 3 ACs*/
	{ 24576, 6144, 4096, 2048 }		/* 4 ACs*/
};

VOID SoftwareFlowControl(
	IN PRTMP_ADAPTER pAd) 
{

		BOOLEAN ResetBulkOutSize=FALSE;
		UCHAR i=0,RunningQueueNo=0,QueIdx=0,HighWorkingAcCount=0;
		UINT PacketsInQueueSize=0;
		UCHAR Priority[]={1,0,2,3};
		
		for (i=0;i<NUM_OF_TX_RING;i++)
		{

			if (pAd->TxContext[i].CurWritePosition>=pAd->TxContext[i].NextBulkOutPosition)
			{
				PacketsInQueueSize=pAd->TxContext[i].CurWritePosition-pAd->TxContext[i].NextBulkOutPosition;
			}
			else 
			{
				PacketsInQueueSize=MAX_TXBULK_SIZE-pAd->TxContext[i].NextBulkOutPosition+pAd->TxContext[i].CurWritePosition;
			}		

			if (pAd->BulkOutDataSizeCount[i]>20480 || PacketsInQueueSize>6144)
			{
				RunningQueueNo++;
				pAd->BulkOutDataFlag[i]=TRUE;
			}
			else
				pAd->BulkOutDataFlag[i]=FALSE;

			pAd->BulkOutDataSizeCount[i]=0;
		}

		if (RunningQueueNo>pAd->LastRunningQueueNo)
		{
			DBGPRINT(RT_DEBUG_INFO,("SoftwareFlowControl  reset %d > %d \n",RunningQueueNo,pAd->LastRunningQueueNo));
			
ResetBulkOutSize=TRUE;
			 pAd->RunningQueueNoCount=0;
			 pAd->LastRunningQueueNo=RunningQueueNo;
		}
		else if (RunningQueueNo==pAd->LastRunningQueueNo)
		{
pAd->RunningQueueNoCount=0;
		}
		else if (RunningQueueNo<pAd->LastRunningQueueNo)
		{
			DBGPRINT(RT_DEBUG_INFO,("SoftwareFlowControl  reset %d < %d \n",RunningQueueNo,pAd->LastRunningQueueNo));
			pAd->RunningQueueNoCount++;
			if (pAd->RunningQueueNoCount>=6)
			{
				ResetBulkOutSize=TRUE;
				pAd->RunningQueueNoCount=0;
				pAd->LastRunningQueueNo=RunningQueueNo;
			}
		}

		if (ResetBulkOutSize==TRUE)
		{
			for (QueIdx=0;QueIdx<NUM_OF_TX_RING;QueIdx++)
			{
				HighWorkingAcCount=0;
				for (i=0;i<NUM_OF_TX_RING;i++)
				{
					if (QueIdx==i)
						continue;

					if (pAd->BulkOutDataFlag[i]==TRUE && Priority[i]>Priority[QueIdx])
							HighWorkingAcCount++;
					
				}
				pAd->BulkOutDataSizeLimit[QueIdx]=MaxBulkOutsSizeLimit[RunningQueueNo][HighWorkingAcCount];
			}

				DBGPRINT(RT_DEBUG_TRACE, ("Reset bulkout size AC0(BE):%7d AC1(BK):%7d AC2(VI):%7d AC3(VO):%7d %d\n",pAd->BulkOutDataSizeLimit[0]
				,pAd->BulkOutDataSizeLimit[1]
				,pAd->BulkOutDataSizeLimit[2]
				,pAd->BulkOutDataSizeLimit[3]
				,RunningQueueNo));			
		}

}
#endif /* INF_AMAZON_SE */

/*
	==========================================================================
	Description:
		initialize the MLME task and its data structure (queue, spinlock, 
		timer, state machines).

	IRQL = PASSIVE_LEVEL

	Return:
		always return NDIS_STATUS_SUCCESS

	==========================================================================
*/
NDIS_STATUS MlmeInit(
	IN PRTMP_ADAPTER pAd) 
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("--> MLME Initialize\n"));

	do 
	{
		Status = MlmeQueueInit(pAd, &pAd->Mlme.Queue);
		if(Status != NDIS_STATUS_SUCCESS) 
			break;

		pAd->Mlme.bRunning = FALSE;
		NdisAllocateSpinLock(pAd, &pAd->Mlme.TaskLock);

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			BssTableInit(&pAd->ScanTab);

			/* init STA state machines*/
			AssocStateMachineInit(pAd, &pAd->Mlme.AssocMachine, pAd->Mlme.AssocFunc);
			AuthStateMachineInit(pAd, &pAd->Mlme.AuthMachine, pAd->Mlme.AuthFunc);
			AuthRspStateMachineInit(pAd, &pAd->Mlme.AuthRspMachine, pAd->Mlme.AuthRspFunc);
			SyncStateMachineInit(pAd, &pAd->Mlme.SyncMachine, pAd->Mlme.SyncFunc);

#ifdef QOS_DLS_SUPPORT
			DlsStateMachineInit(pAd, &pAd->Mlme.DlsMachine, pAd->Mlme.DlsFunc);
#endif /* QOS_DLS_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
			TDLS_StateMachineInit(pAd, &pAd->Mlme.TdlsMachine, pAd->Mlme.TdlsFunc);
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef WSC_STA_SUPPORT
#ifdef IWSC_SUPPORT
			IWSC_StateMachineInit(pAd, &pAd->Mlme.IWscMachine, pAd->Mlme.IWscFunc);
#endif /* IWSC_SUPPORT */
#endif /* WSC_STA_SUPPORT */



			/* Since we are using switch/case to implement it, the init is different from the above */
			/* state machine init*/
			MlmeCntlInit(pAd, &pAd->Mlme.CntlMachine, NULL);

#ifdef PCIE_PS_SUPPORT
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
			{
			    /* only PCIe cards need these two timers*/
				RTMPInitTimer(pAd, &pAd->Mlme.PsPollTimer, GET_TIMER_FUNCTION(PsPollWakeExec), pAd, FALSE);
				RTMPInitTimer(pAd, &pAd->Mlme.RadioOnOffTimer, GET_TIMER_FUNCTION(RadioOnExec), pAd, FALSE);
			}
#endif /* PCIE_PS_SUPPORT */

			RTMPInitTimer(pAd, &pAd->Mlme.LinkDownTimer, GET_TIMER_FUNCTION(LinkDownExec), pAd, FALSE);
			RTMPInitTimer(pAd, &pAd->StaCfg.StaQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(StaQuickResponeForRateUpExec), pAd, FALSE);
			pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = FALSE;
			RTMPInitTimer(pAd, &pAd->StaCfg.WpaDisassocAndBlockAssocTimer, GET_TIMER_FUNCTION(WpaDisassocApAndBlockAssoc), pAd, FALSE);



		}
#endif /* CONFIG_STA_SUPPORT */
		
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* init AP state machines*/
			APAssocStateMachineInit(pAd, &pAd->Mlme.ApAssocMachine, pAd->Mlme.ApAssocFunc);
			APAuthStateMachineInit(pAd, &pAd->Mlme.ApAuthMachine, pAd->Mlme.ApAuthFunc);
			APSyncStateMachineInit(pAd, &pAd->Mlme.ApSyncMachine, pAd->Mlme.ApSyncFunc);

#ifdef APCLI_SUPPORT
			/* init apcli state machines*/
			ASSERT(APCLI_AUTH_FUNC_SIZE == APCLI_MAX_AUTH_MSG * APCLI_MAX_AUTH_STATE);
			ApCliAuthStateMachineInit(pAd, &pAd->Mlme.ApCliAuthMachine, pAd->Mlme.ApCliAuthFunc);

			ASSERT(APCLI_ASSOC_FUNC_SIZE == APCLI_MAX_ASSOC_MSG * APCLI_MAX_ASSOC_STATE);
			ApCliAssocStateMachineInit(pAd, &pAd->Mlme.ApCliAssocMachine, pAd->Mlme.ApCliAssocFunc);

			ASSERT(APCLI_SYNC_FUNC_SIZE == APCLI_MAX_SYNC_MSG * APCLI_MAX_SYNC_STATE);
			ApCliSyncStateMachineInit(pAd, &pAd->Mlme.ApCliSyncMachine, pAd->Mlme.ApCliSyncFunc);

			ASSERT(APCLI_CTRL_FUNC_SIZE == APCLI_MAX_CTRL_MSG * APCLI_MAX_CTRL_STATE);
			ApCliCtrlStateMachineInit(pAd, &pAd->Mlme.ApCliCtrlMachine, pAd->Mlme.ApCliCtrlFunc);

#endif /* APCLI_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */

#ifdef WSC_INCLUDED
		/* Init Wsc state machine */
		ASSERT(WSC_FUNC_SIZE == MAX_WSC_MSG * MAX_WSC_STATE);
		WscStateMachineInit(pAd, &pAd->Mlme.WscMachine, pAd->Mlme.WscFunc);
#endif /* WSC_INCLUDED */

		WpaStateMachineInit(pAd, &pAd->Mlme.WpaMachine, pAd->Mlme.WpaFunc);


		ActionStateMachineInit(pAd, &pAd->Mlme.ActMachine, pAd->Mlme.ActFunc);

		/* Init mlme periodic timer*/
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExec), pAd, TRUE);

		/* Set mlme periodic timer*/
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

		/* software-based RX Antenna diversity*/
		RTMPInitTimer(pAd, &pAd->Mlme.RxAntEvalTimer, GET_TIMER_FUNCTION(AsicRxAntEvalTimeout), pAd, FALSE);

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* Init APSD periodic timer*/
			RTMPInitTimer(pAd, &pAd->Mlme.APSDPeriodicTimer, GET_TIMER_FUNCTION(APSDPeriodicExec), pAd, TRUE);
			RTMPSetTimer(&pAd->Mlme.APSDPeriodicTimer, 50);

			/* Init APQuickResponseForRateUp timer.*/
			RTMPInitTimer(pAd, &pAd->ApCfg.ApQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(APQuickResponeForRateUpExec), pAd, FALSE);
			pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef P2P_SUPPORT
			/* P2P Ctrl State Machine */
			ASSERT(P2P_CTRL_FUNC_SIZE == P2P_CTRL_MAX_EVENTS * P2P_CTRL_MAX_STATES);
			P2PCtrlStateMachineInit(pAd, &pAd->P2pCfg.P2PCtrlMachine, pAd->P2pCfg.P2PCtrlFunc);
	
			/* P2P Discovery State Machine */
			ASSERT(P2P_DISC_FUNC_SIZE == P2P_DISC_MAX_EVENTS * P2P_DISC_MAX_STATES);
			P2PDiscoveryStateMachineInit(pAd, &pAd->P2pCfg.P2PDiscMachine, pAd->P2pCfg.P2PDiscFunc);
	
			/* P2P Group Formation State Machine */
			ASSERT(P2P_GO_FORM_FUNC_SIZE == P2P_GO_NEGO_MAX_EVENTS * P2P_GO_FORM_MAX_STATES);
			P2PGoFormationStateMachineInit(pAd, &pAd->P2pCfg.P2PGoFormMachine, pAd->P2pCfg.P2PGoFormFunc);
	
			/* P2P Action Frame State Machine */
			ASSERT(P2P_ACTION_FUNC_SIZE == MAX_P2P_MSG * MAX_P2P_STATE);
			P2PStateMachineInit(pAd, &pAd->P2pCfg.P2PActionMachine, pAd->P2pCfg.P2PActionFunc);
	
			/* P2P CTWindows timer */
			RTMPInitTimer(pAd, &pAd->P2pCfg.P2pCTWindowTimer, GET_TIMER_FUNCTION(P2PCTWindowTimer), pAd, FALSE);
			/* P2P SwNOA timer */
			RTMPInitTimer(pAd, &pAd->P2pCfg.P2pSwNoATimer, GET_TIMER_FUNCTION(P2pSwNoATimeOut), pAd, FALSE);
			/* P2P Presence Absent timer */
			RTMPInitTimer(pAd, &pAd->P2pCfg.P2pPreAbsenTimer, GET_TIMER_FUNCTION(P2pPreAbsenTimeOut), pAd, FALSE);
			/* P2P WSC Timer */
			RTMPInitTimer(pAd, &pAd->P2pCfg.P2pWscTimer, GET_TIMER_FUNCTION(P2pWscTimeOut), pAd, FALSE);
			/* P2P Re-Transmit Action Frame Timer */
			RTMPInitTimer(pAd, &pAd->P2pCfg.P2pReSendTimer, GET_TIMER_FUNCTION(P2pReSendTimeOut), pAd, FALSE);
			/* P2P CLIENT Re-Connect Timer */
			RTMPInitTimer(pAd, &pAd->P2pCfg.P2pCliReConnectTimer, GET_TIMER_FUNCTION(P2pCliReConnectTimeOut), pAd, FALSE);

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				/* init AP state machines */
				APAssocStateMachineInit(pAd, &pAd->Mlme.ApAssocMachine, pAd->Mlme.ApAssocFunc);
				APAuthStateMachineInit(pAd, &pAd->Mlme.ApAuthMachine, pAd->Mlme.ApAuthFunc);
				APSyncStateMachineInit(pAd, &pAd->Mlme.ApSyncMachine, pAd->Mlme.ApSyncFunc);
#ifdef APCLI_SUPPORT
				/* init apcli state machines */
				ASSERT(APCLI_AUTH_FUNC_SIZE == APCLI_MAX_AUTH_MSG * APCLI_MAX_AUTH_STATE);
				ApCliAuthStateMachineInit(pAd, &pAd->Mlme.ApCliAuthMachine, pAd->Mlme.ApCliAuthFunc);
	
				ASSERT(APCLI_ASSOC_FUNC_SIZE == APCLI_MAX_ASSOC_MSG * APCLI_MAX_ASSOC_STATE);
				ApCliAssocStateMachineInit(pAd, &pAd->Mlme.ApCliAssocMachine, pAd->Mlme.ApCliAssocFunc);
	
				ASSERT(APCLI_SYNC_FUNC_SIZE == APCLI_MAX_SYNC_MSG * APCLI_MAX_SYNC_STATE);
				ApCliSyncStateMachineInit(pAd, &pAd->Mlme.ApCliSyncMachine, pAd->Mlme.ApCliSyncFunc);
	
				ASSERT(APCLI_CTRL_FUNC_SIZE == APCLI_MAX_CTRL_MSG * APCLI_MAX_CTRL_STATE);
				ApCliCtrlStateMachineInit(pAd, &pAd->Mlme.ApCliCtrlMachine, pAd->Mlme.ApCliCtrlFunc);
	
#endif /* APCLI_SUPPORT */
	
				/* Init APSD periodic timer */
				RTMPInitTimer(pAd, &pAd->Mlme.APSDPeriodicTimer, GET_TIMER_FUNCTION(APSDPeriodicExec), pAd, TRUE);
				RTMPSetTimer(&pAd->Mlme.APSDPeriodicTimer, 50);
			}
#endif /* CONFIG_STA_SUPPORT */
#endif /* P2P_SUPPORT */

	} while (FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- MLME Initialize\n"));

	return Status;
}


/*
	==========================================================================
	Description:
		main loop of the MLME
	Pre:
		Mlme has to be initialized, and there are something inside the queue
	Note:
		This function is invoked from MPSetInformation and MPReceive;
		This task guarantee only one MlmeHandler will run. 

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID MlmeHandler(
	IN PRTMP_ADAPTER pAd) 
{
	MLME_QUEUE_ELEM 	   *Elem = NULL;
#ifdef APCLI_SUPPORT
	SHORT apcliIfIndex;
#endif /* APCLI_SUPPORT */

	/* Only accept MLME and Frame from peer side, no other (control/data) frame should*/
	/* get into this state machine*/

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning) 
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	} 
	else 
	{
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) 
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Device Halted or Removed or MlmeRest, exit MlmeHandler! (queue num = %ld)\n", pAd->Mlme.Queue.Num));
			break;
		}
		
#ifdef RALINK_ATE			
		if(ATE_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("The driver is in ATE mode now in MlmeHandler\n"));
			break;
		}	
#endif /* RALINK_ATE */

		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) 
		{

			/* if dequeue success*/
			switch (Elem->Machine) 
			{
				/* STA state machines*/
#ifdef CONFIG_STA_SUPPORT
				case ASSOC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.AssocMachine,
										Elem, pAd->Mlme.AssocMachine.CurrState);
					break;

				case AUTH_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.AuthMachine,
										Elem, pAd->Mlme.AuthMachine.CurrState);
					break;

				case AUTH_RSP_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.AuthRspMachine,
										Elem, pAd->Mlme.AuthRspMachine.CurrState);
					break;

				case SYNC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.SyncMachine,
										Elem, pAd->Mlme.SyncMachine.CurrState);
					break;

				case MLME_CNTL_STATE_MACHINE:
					MlmeCntlMachinePerformAction(pAd, &pAd->Mlme.CntlMachine, Elem);
					break;

				case WPA_PSK_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.WpaPskMachine,
										Elem, pAd->Mlme.WpaPskMachine.CurrState);
					break;	

#ifdef QOS_DLS_SUPPORT
				case DLS_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.DlsMachine,
										Elem, pAd->Mlme.DlsMachine.CurrState);
					break;
#endif /* QOS_DLS_SUPPORT */


#ifdef DOT11Z_TDLS_SUPPORT
				case TDLS_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.TdlsMachine,
										Elem, pAd->Mlme.TdlsMachine.CurrState);
					break;
#endif /* DOT11Z_TDLS_SUPPORT */


#endif /* CONFIG_STA_SUPPORT */						

				case ACTION_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ActMachine,
										Elem, pAd->Mlme.ActMachine.CurrState);
					break;	

#ifdef CONFIG_AP_SUPPORT
				/* AP state amchines*/

				case AP_ASSOC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApAssocMachine,
									Elem, pAd->Mlme.ApAssocMachine.CurrState);
					break;

				case AP_AUTH_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApAuthMachine, 
									Elem, pAd->Mlme.ApAuthMachine.CurrState);
					break;

				case AP_SYNC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ApSyncMachine,
									Elem, pAd->Mlme.ApSyncMachine.CurrState);
					break;

#ifdef APCLI_SUPPORT
				case APCLI_AUTH_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= 64)
						apcliIfIndex = ((apcliIfIndex - 64) / 16);		
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))
					{
						ULONG AuthCurrState;
#ifdef MAC_REPEATER_SUPPORT
						UCHAR CliIdx = 0;

						apcliIfIndex = Elem->Priv;

						if (apcliIfIndex >= 64)
						{
							CliIdx = ((apcliIfIndex - 64) % 16);
							apcliIfIndex = ((apcliIfIndex - 64) / 16);

							AuthCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].RepeaterCli[CliIdx].AuthCurrState;
						}
						else
#endif /* MAC_REPEATER_SUPPORT */
							AuthCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AuthCurrState;

						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAuthMachine,
								Elem, AuthCurrState);
					}
					break;

				case APCLI_ASSOC_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= 64)
						apcliIfIndex = ((apcliIfIndex - 64) / 16);		
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))		
					{
						ULONG AssocCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].AssocCurrState;
#ifdef MAC_REPEATER_SUPPORT
						UCHAR CliIdx = 0;

						apcliIfIndex = Elem->Priv;

						if (apcliIfIndex >= 64)
						{
							CliIdx = ((apcliIfIndex - 64) % 16);
							apcliIfIndex = ((apcliIfIndex - 64) / 16);
							AssocCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].RepeaterCli[CliIdx].AssocCurrState;
						}
#endif /* MAC_REPEATER_SUPPORT */
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliAssocMachine,
								Elem, AssocCurrState);
					}
					break;

				case APCLI_SYNC_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
					if(isValidApCliIf(apcliIfIndex))
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliSyncMachine, Elem,
							(pAd->ApCfg.ApCliTab[apcliIfIndex].SyncCurrState));
					break;

				case APCLI_CTRL_STATE_MACHINE:
					apcliIfIndex = Elem->Priv;
#ifdef MAC_REPEATER_SUPPORT
					if (apcliIfIndex >= 64)
						apcliIfIndex = ((apcliIfIndex - 64) / 16);		
#endif /* MAC_REPEATER_SUPPORT */

					if(isValidApCliIf(apcliIfIndex))
					{
						ULONG CtrlCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].CtrlCurrState;
#ifdef MAC_REPEATER_SUPPORT
						UCHAR CliIdx = 0;

						apcliIfIndex = Elem->Priv;

						if (apcliIfIndex >= 64)
						{
							CliIdx = ((apcliIfIndex - 64) % 16);
							apcliIfIndex = ((apcliIfIndex - 64) / 16);
							CtrlCurrState = pAd->ApCfg.ApCliTab[apcliIfIndex].RepeaterCli[CliIdx].CtrlCurrState;
						}
#endif /* MAC_REPEATER_SUPPORT */
						StateMachinePerformAction(pAd, &pAd->Mlme.ApCliCtrlMachine, Elem, CtrlCurrState);
					}
					break;
#endif /* APCLI_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */
				case WPA_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.WpaMachine, Elem, pAd->Mlme.WpaMachine.CurrState);
					break;
#ifdef WSC_INCLUDED
                case WSC_STATE_MACHINE:
					if (pAd->pWscElme)
					{
						RTMP_SEM_LOCK(&pAd->WscElmeLock);
						NdisMoveMemory(pAd->pWscElme, Elem, sizeof(MLME_QUEUE_ELEM));
						RTMP_SEM_UNLOCK(&pAd->WscElmeLock);
/*#ifdef KTHREAD_SUPPORT*/
/*						WAKE_UP(&(pAd->wscTask));*/
/*#else*/
/*						RTMP_SEM_EVENT_UP(&(pAd->wscTask.taskSema));*/
/*#endif*/
						RtmpOsTaskWakeUp(&(pAd->wscTask));
					}
                    break;
#ifdef IWSC_SUPPORT
				case IWSC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.IWscMachine, Elem, pAd->Mlme.IWscMachine.CurrState);
                    break;
#endif /* IWSC_SUPPORT */
#endif /* WSC_INCLUDED */


#ifdef P2P_SUPPORT
				case P2P_CTRL_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PCtrlMachine, Elem,
							pAd->P2pCfg.CtrlCurrentState);
					break;
				case P2P_DISC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PDiscMachine, Elem,
							pAd->P2pCfg.DiscCurrentState);
					break;
				case P2P_GO_FORM_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PGoFormMachine, Elem,
							pAd->P2pCfg.GoFormCurrentState);
					break;
				case P2P_ACTION_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->P2pCfg.P2PActionMachine, Elem,
							pAd->P2pCfg.ActionState);
					break;
#endif /* P2P_SUPPORT */
				default:
					DBGPRINT(RT_DEBUG_TRACE, ("ERROR: Illegal machine %ld in MlmeHandler()\n", Elem->Machine));
					break;
			} /* end of switch*/

			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			DBGPRINT_ERR(("MlmeHandler: MlmeQueue empty\n"));
		}
	}

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
}

/*
	==========================================================================
	Description:
		Destructor of MLME (Destroy queue, state machine, spin lock and timer)
	Parameters:
		Adapter - NIC Adapter pointer
	Post:
		The MLME task will no longer work properly

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
VOID MlmeHalt(
	IN PRTMP_ADAPTER pAd) 
{
	BOOLEAN 	  Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeHalt\n"));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		/* disable BEACON generation and other BEACON related hardware timers*/
		AsicDisableSync(pAd);
	}

	RTMPCancelTimer(&pAd->Mlme.PeriodicTimer,		&Cancelled);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef QOS_DLS_SUPPORT
		UCHAR		i;
#endif /* QOS_DLS_SUPPORT */
		/* Cancel pending timers*/
		RTMPCancelTimer(&pAd->MlmeAux.AssocTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer,	&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.AuthTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer,		&Cancelled);


#ifdef PCIE_PS_SUPPORT
	    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
			&&(pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
	    {
	   	    RTMPCancelTimer(&pAd->Mlme.PsPollTimer,		&Cancelled);
		    RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,		&Cancelled);
		}
#endif /* PCIE_PS_SUPPORT */

#ifdef QOS_DLS_SUPPORT
		for (i=0; i<MAX_NUM_OF_DLS_ENTRY; i++)
		{
			RTMPCancelTimer(&pAd->StaCfg.DLSEntry[i].Timer, &Cancelled);
		}
#endif /* QOS_DLS_SUPPORT */
		RTMPCancelTimer(&pAd->Mlme.LinkDownTimer,		&Cancelled);


#ifdef WSC_STA_SUPPORT
		if (pAd->StaCfg.WscControl.WscProfileRetryTimerRunning)
		{
			pAd->StaCfg.WscControl.WscProfileRetryTimerRunning = FALSE;
			RTMPCancelTimer(&pAd->StaCfg.WscControl.WscProfileRetryTimer, &Cancelled);
		}
#endif /* WSC_STA_SUPPORT */


		if (pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
		{
			RTMPCancelTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, &Cancelled);
			pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = FALSE;
		}
		RTMPCancelTimer(&pAd->StaCfg.WpaDisassocAndBlockAssocTimer, &Cancelled);
#ifdef IWSC_SUPPORT
		RTMPCancelTimer(&pAd->StaCfg.IWscInfo.IWscT1Timer, &Cancelled);
		RTMPCancelTimer(&pAd->StaCfg.IWscInfo.IWscT2Timer, &Cancelled);
#endif /* IWSC_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */

	RTMPCancelTimer(&pAd->Mlme.RxAntEvalTimer,		&Cancelled);


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
			UCHAR		i;
#endif /* APCLI_SUPPORT */

		RTMPCancelTimer(&pAd->Mlme.APSDPeriodicTimer,	&Cancelled);

		if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE)
			RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);

#ifdef APCLI_SUPPORT
		for (i = 0; i < MAX_APCLI_NUM; i++)
		{
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ProbeTimer, &Cancelled);
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ApCliAssocTimer, &Cancelled);
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ApCliAuthTimer, &Cancelled);
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.WpaDisassocAndBlockAssocTimer, &Cancelled);

#ifdef WSC_AP_SUPPORT
			if (pAd->ApCfg.ApCliTab[i].WscControl.WscProfileRetryTimerRunning)
			{
				pAd->ApCfg.ApCliTab[i].WscControl.WscProfileRetryTimerRunning = FALSE;
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].WscControl.WscProfileRetryTimer, &Cancelled);
			}
#endif /* WSC_AP_SUPPORT */
		}
#endif /* APCLI_SUPPORT */
		RTMPCancelTimer(&pAd->MlmeAux.APScanTimer, &Cancelled);
	}

#endif /* CONFIG_AP_SUPPORT */


#ifdef P2P_SUPPORT
		/* P2P CTWindows timer */
		RTMPCancelTimer(&pAd->P2pCfg.P2pCTWindowTimer, &Cancelled);
		/* P2P SwNOA timer */
		RTMPCancelTimer(&pAd->P2pCfg.P2pSwNoATimer, &Cancelled);
		/* P2P Presence Absent timer */
		RTMPCancelTimer(&pAd->P2pCfg.P2pPreAbsenTimer, &Cancelled);

		if (pAd->P2pCfg.bP2pCliReConnectTimerRunning)
		{
			pAd->P2pCfg.bP2pCliReConnectTimerRunning = FALSE;
			RTMPCancelTimer(&pAd->P2pCfg.P2pCliReConnectTimer, &Cancelled);
		}

		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			RTMPCancelTimer(&pAd->Mlme.APSDPeriodicTimer,	&Cancelled);
			UCHAR i;	
			for (i = 0; i < MAX_APCLI_NUM; i++)
			{
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ProbeTimer, &Cancelled);
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ApCliAssocTimer, &Cancelled);
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.ApCliAuthTimer, &Cancelled);
			}
	
#ifdef WSC_AP_SUPPORT
			if (pAd->ApCfg.ApCliTab[BSS0].WscControl.WscProfileRetryTimerRunning)
			{
				pAd->ApCfg.ApCliTab[BSS0].WscControl.WscProfileRetryTimerRunning = FALSE;
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[BSS0].WscControl.WscProfileRetryTimer, &Cancelled);
			}
#endif /* WSC_AP_SUPPORT */
			RTMPCancelTimer(&pAd->MlmeAux.APScanTimer,	&Cancelled);
		}
#endif /* P2P_SUPPORT */

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
		
#ifdef LED_CONTROL_SUPPORT		
		/* Set LED*/
		RTMPSetLED(pAd, LED_HALT);
		RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, firmware is not done it.*/
#endif /* LED_CONTROL_SUPPORT */

#if defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)
		if (pAd->WOW_Cfg.bEnable == FALSE)
#endif /* WOW_SUPPORT */
			if (pChipOps->AsicHaltAction)
				pChipOps->AsicHaltAction(pAd);
	}

	RTMPusecDelay(5000);    /*  5 msec to gurantee Ant Diversity timer canceled*/

	MlmeQueueDestroy(&pAd->Mlme.Queue);
	NdisFreeSpinLock(&pAd->Mlme.TaskLock);

	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeHalt\n"));
}

VOID MlmeResetRalinkCounters(
	IN  PRTMP_ADAPTER   pAd)
{
	pAd->RalinkCounters.LastOneSecRxOkDataCnt = pAd->RalinkCounters.OneSecRxOkDataCnt;

#ifdef RALINK_ATE
	if (!ATE_ON(pAd))
#endif /* RALINK_ATE */
	{
		/* for performace enchanement */
		NdisZeroMemory(&pAd->RalinkCounters,
						(UINT32)&pAd->RalinkCounters.OneSecEnd -
						(UINT32)&pAd->RalinkCounters.OneSecStart);
	}

	return;
}


/*
	==========================================================================
	Description:
		This routine is executed periodically to -
		1. Decide if it's a right time to turn on PwrMgmt bit of all 
		   outgoiing frames
		2. Calculate ChannelQuality based on statistics of the last
		   period, so that TX rate won't toggling very frequently between a 
		   successful TX and a failed TX.
		3. If the calculated ChannelQuality indicated current connection not 
		   healthy, then a ROAMing attempt is tried here.

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#define ADHOC_BEACON_LOST_TIME		(8*OS_HZ)  /* 8 sec*/
VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	ULONG			TxTotalCnt;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	/* No More 0x84 MCU CMD from v.30 FW*/


#ifdef MICROWAVE_OVEN_SUPPORT
	//printk("MO_Cfg.bEnable=%d \n", pAd->CommonCfg.MO_Cfg.bEnable);
	if (pAd->CommonCfg.MO_Cfg.bEnable)
	{
		UINT8 stage = pAd->CommonCfg.MO_Cfg.nPeriod_Cnt%10;

		if (stage == MO_MEAS_PERIOD)
		{
			//printk("Microwave measure... \n");
			AsicMeasureFalseCCA(pAd);
			pAd->CommonCfg.MO_Cfg.nPeriod_Cnt = 0;
		}
		else if (stage == MO_IDLE_PERIOD)
		{
			RX_STA_CNT1_STRUC   RxStaCnt1;

			RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);
			pAd->CommonCfg.MO_Cfg.nFalseCCACnt += RxStaCnt1.field.FalseCca;

			//printk("%s: fales cca1 %d\n", __FUNCTION__, pAd->CommonCfg.MO_Cfg.nFalseCCACnt);
			if (pAd->CommonCfg.MO_Cfg.nFalseCCACnt > pAd->CommonCfg.MO_Cfg.nFalseCCATh)
				AsicMitigateMicrowave(pAd);
		}
		pAd->CommonCfg.MO_Cfg.nPeriod_Cnt++;
	}
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef INF_AMAZON_SE
	SoftwareFlowControl(pAd);
#endif /* INF_AMAZON_SE */

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
	    /* If Hardware controlled Radio enabled, we have to check GPIO pin2 every 2 second.*/
		/* Move code to here, because following code will return when radio is off*/
		if ((pAd->Mlme.PeriodicRound % (MLME_TASK_EXEC_MULTIPLE * 2) == 0) &&
/*			(pAd->StaCfg.bHardwareRadio == TRUE) &&*/
			((IDLE_ON(pAd)) || (pAd->StaCfg.Psm == PWR_ACTIVE)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			((pAd->StaCfg.bHardwareRadio == TRUE))
/*			|| (pAd->StaCfg.WscControl.CheckHWPBCState == HWPBCState_GUI)))*/
			/*&&(pAd->bPCIclkOff == FALSE)*/)
		{
			UINT32				data = 0;


			/* Read GPIO pin2 as Hardware controlled radio state*/
/*#ifndef RT3090*/
			/*RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);*/
/*#endif  RT3090 */
/*KH(PCIE PS):Added based on Jane<--*/
#ifdef PCIE_PS_SUPPORT
			/* Read GPIO pin2 as Hardware controlled radio state*/
			/* We need to Read GPIO if HW said so no mater what advance power saving*/
			if ((pAd->OpMode == OPMODE_STA) && (IDLE_ON(pAd))
				&& (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
				&& (pAd->StaCfg.PSControl.field.EnablePSinIdle == TRUE))
			{
				/* Want to make sure device goes to L0 state before reading register.*/
				RTMPPCIeLinkCtrlValueRestore(pAd, 0);
				RTMP_IO_FORCE_READ32(pAd, GPIO_CTRL_CFG, &data);
				RTMPPCIeLinkCtrlSetting(pAd, 3);
			}
			else
				RTMP_IO_FORCE_READ32(pAd, GPIO_CTRL_CFG, &data);
#else
				RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
#endif /* defined(RT3090) || defined(RT3592) || defined(RT3390) */
/*KH(PCIE PS):Added based on Jane-->*/

			/* Update Radio state from GPIO*/
			if (pAd->StaCfg.bHardwareRadio == TRUE)
			{
				{
					if (data & 0x04)
						pAd->StaCfg.bHwRadio = TRUE;
					else
						pAd->StaCfg.bHwRadio = FALSE;
				}
#ifdef RT_CFG80211_SUPPORT
#ifdef RFKILL_HW_SUPPORT
				RT_CFG80211_RFKILL_STATUS_UPDATE(pAd, pAd->StaCfg.bHwRadio); 
#endif // RFKILL_HW_SUPPORT //
#endif /* RT_CFG80211_SUPPORT */
			}

			/* Always read HW radio configuration.*/
			if (pAd->StaCfg.bHardwareRadio == TRUE)
			{
				if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
				{
					pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
					if (pAd->StaCfg.bRadio == TRUE)
					{
						MlmeRadioOn(pAd);
						/* Update extra information*/
						pAd->ExtraInfo = EXTRA_INFO_CLEAR;
					}
					else
					{			    
						MlmeRadioOff(pAd);
						/* Update extra information*/
						pAd->ExtraInfo = HW_RADIO_OFF;
					}
				}
			}
		}
	}
#endif /* RTMP_MAC_PCI */

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		RTMP_MLME_PRE_SANITY_CHECK(pAd);
	}


#endif /* CONFIG_STA_SUPPORT */

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RADIO_MEASUREMENT |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
		return;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Do nothing if monitor mode is on*/
		if (MONITOR_ON(pAd))
			return;

		if (pAd->Mlme.PeriodicRound & 0x1)
		{
			/* This is the fix for wifi 11n extension channel overlapping test case.  for 2860D*/
			if (((pAd->MACVersion & 0xffff) == 0x0101) && 
				(STA_TGN_WIFI_ON(pAd)) &&
				(pAd->CommonCfg.IOTestParm.bToggle == FALSE))

				{
					RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, 0x24Bf);
					pAd->CommonCfg.IOTestParm.bToggle = TRUE;
				}
				else if ((STA_TGN_WIFI_ON(pAd)) &&
						((pAd->MACVersion & 0xffff) == 0x0101))
				{
					RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, 0x243f);
					pAd->CommonCfg.IOTestParm.bToggle = FALSE;
				}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	pAd->bUpdateBcnCntDone = FALSE;
	
/*	RECBATimerTimeout(SystemSpecific1,FunctionContext,SystemSpecific2,SystemSpecific3);*/
	pAd->Mlme.PeriodicRound ++;
	pAd->Mlme.GPIORound++;


	/* by default, execute every 500ms */
	if ((pAd->ra_interval) && 
		((pAd->Mlme.PeriodicRound % (pAd->ra_interval / 100)) == 0) && 
		RTMPAutoRateSwitchCheck(pAd)/*(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))*/
	)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			APMlmeDynamicTxRateSwitching(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		/* perform dynamic tx rate switching based on past TX history*/
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
#ifdef P2P_SUPPORT
					|| P2P_GO_ON(pAd) || P2P_CLI_ON(pAd)
#endif /* P2P_SUPPORT */
					)
				&& (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)))
				MlmeDynamicTxRateSwitching(pAd);
		}
#endif /* CONFIG_STA_SUPPORT */
	}

#ifdef DFS_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#endif /* DFS_SUPPORT */



	/* Normal 1 second Mlme PeriodicExec.*/
	if (pAd->Mlme.PeriodicRound %MLME_TASK_EXEC_MULTIPLE == 0)
	{
		pAd->Mlme.OneSecPeriodicRound ++;

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		if (pAd->bApCliCertTest == FALSE )
		{
#endif /* APCLI_CERT_SUPPORT */		
#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			dynamic_tune_be_tx_op(pAd, 50);	/* change form 100 to 50 for WMM WiFi test @20070504*/
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		}
#endif /* APCLI_CERT_SUPPORT */		
#endif /* APCLI_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */


		/*ORIBATimerTimeout(pAd);*/
		NdisGetSystemUpTime(&pAd->Mlme.Now32);

		/* add the most up-to-date h/w raw counters into software variable, so that*/
		/* the dynamic tuning mechanism below are based on most up-to-date information*/
		/* Hint: throughput impact is very serious in the function */
		NICUpdateRawCounters(pAd);																										

#ifdef DYNAMIC_VGA_SUPPORT
		if ((pAd->CommonCfg.MO_Cfg.bDyncVGAEnable) && (pAd->bCalibrationDone))
		{
			if (pAd->Mlme.PeriodicRound %MLME_TASK_EXEC_MULTIPLE == 0)
			{
				UCHAR BbpReg = 0;

				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &BbpReg);

				DBGPRINT(RT_DEBUG_TRACE,
					("one second False CCA=%d, fixed R66 at 0x%x\n", pAd->RalinkCounters.OneSecFalseCCACnt, BbpReg));

				if (pAd->RalinkCounters.OneSecFalseCCACnt > pAd->CommonCfg.MO_Cfg.nFalseCCATh)
				{
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xAD);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x28);

					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xB1);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x20);

					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x8D);
					if (pAd->CommonCfg.BBPCurrentBW == BW_20)
					{
#ifdef RT6352_EL_SUPPORT
						if (pAd->NicConfig2.field.ExternalLNAForG)
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x15);
						else
#endif /* RT6352_EL_SUPPORT */
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x1A);
					}
					else
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x10);
					}

					if (BbpReg < (pAd->CommonCfg.MO_Cfg.Stored_BBP_R66 + 0x10))
					{
						BbpReg += 4;
						AsicBBPWriteWithRxChain(pAd, BBP_R66, BbpReg, RX_CHAIN_ALL);
					}
					else if (BbpReg == (pAd->CommonCfg.MO_Cfg.Stored_BBP_R66 + 0x10))
					{
						if (pAd->RalinkCounters.OneSecFalseCCACnt > 1500)
						{
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9C);
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x3D);

							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9D);
							if (pAd->CommonCfg.BBPCurrentBW == BW_20)
								RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x40);
							else
								RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x2F);
						}
					}
				}
				else if (pAd->RalinkCounters.OneSecFalseCCACnt < pAd->CommonCfg.MO_Cfg.nLowFalseCCATh)
				{
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9C);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x27);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9D);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x27);

					if (BbpReg > pAd->CommonCfg.MO_Cfg.Stored_BBP_R66)
					{
						BbpReg -= 4;
						AsicBBPWriteWithRxChain(pAd, BBP_R66, BbpReg, RX_CHAIN_ALL);
					}
					else if (BbpReg == pAd->CommonCfg.MO_Cfg.Stored_BBP_R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xAD);
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x0D);
						
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xB1);
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x16);

						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x8D);
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x0C);
					}
				}
				else
				{
					if (BbpReg == pAd->CommonCfg.MO_Cfg.Stored_BBP_R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x8D);
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x0C);
					}
				}
			}
		}
#endif /* DYNAMIC_VGA_SUPPORT */



#ifdef DOT11_N_SUPPORT
   		/* Need statistics after read counter. So put after NICUpdateRawCounters*/
		ORIBATimerTimeout(pAd);
#endif /* DOT11_N_SUPPORT */

		/* if MGMT RING is full more than twice within 1 second, we consider there's*/
		/* a hardware problem stucking the TX path. In this case, try a hardware reset*/
		/* to recover the system*/
	/*	if (pAd->RalinkCounters.MgmtRingFullCount >= 2)*/
	/*		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HARDWARE_ERROR);*/
	/*	else*/
	/*		pAd->RalinkCounters.MgmtRingFullCount = 0;*/

		/* The time period for checking antenna is according to traffic*/
		{
			if (pAd->Mlme.bEnableAutoAntennaCheck)
			{
				TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
								 pAd->RalinkCounters.OneSecTxRetryOkCount + 
								 pAd->RalinkCounters.OneSecTxFailCount;
				
				/* dynamic adjust antenna evaluation period according to the traffic*/
				if (TxTotalCnt > 50)
				{
					if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
					{
						AsicEvaluateRxAnt(pAd);
					}
				}
				else
				{
					if (pAd->Mlme.OneSecPeriodicRound % 3 == 0)
					{
						AsicEvaluateRxAnt(pAd);
					}
				}
			}
		}

#ifdef VIDEO_TURBINE_SUPPORT
		/*VideoTurbineUpdate(pAd);*/
		/*VideoTurbineDynamicTune(pAd);*/
#endif /* VIDEO_TURBINE_SUPPORT */

#ifdef VCORECAL_SUPPORT
		{
			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 0)
				AsicVCORecalibration(pAd);
		}
#endif /* VCORECAL_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			APMlmePeriodicExec(pAd);


			if ((pAd->RalinkCounters.OneSecBeaconSentCnt == 0)
				&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
				&& (OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
				&& ((pAd->CommonCfg.bIEEE80211H != 1)
					|| (pAd->Dot11_H.RDMode != RD_SILENCE_MODE))				
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode != WDS_BRIDGE_MODE)		
#endif /* WDS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
				&& (isCarrierDetectExist(pAd) == FALSE)
#endif /* CARRIER_DETECTION_SUPPORT */
				)
				pAd->macwd ++;
			else
				pAd->macwd = 0;

			if ((pAd->macwd > 1) && (pAd->bEnableMacWD == TRUE))
			{
				int count = 0;
				BOOLEAN MAC_ready = FALSE;
				UINT32	MacCsr12 = 0;
				
				/* Disable MAC*/
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);
				
				/* polling MAC status*/
				while (count < 10)
				{
					RTMPusecDelay(1000); /* 1 ms*/
					RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacCsr12);

					/* if MAC is idle*/
					if ((MacCsr12 & 0x03) == 0)	
					{
						MAC_ready = TRUE;
						break;
					}				
					count ++;
				}
				
				if (MAC_ready)
				{
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
					RTMPusecDelay(1);
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("Warning, MAC isn't ready \n"));
				}

				{
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xC);
				}

				DBGPRINT(RT_DEBUG_WARN, ("MAC specific condition \n"));

#if 0
#ifdef AP_QLOAD_SUPPORT
				Show_QoSLoad_Proc(pAd, NULL);
#endif /* AP_QLOAD_SUPPORT */
#endif
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef DOT11Z_TDLS_SUPPORT
			RtmpPsActiveExtendCheck(pAd);
#ifdef TDLS_AUTOLINK_SUPPORT
   			 /* TDLS discovery link maintenance */
    			if (IS_TDLS_SUPPORT(pAd) && (pAd->StaCfg.TdlsInfo.TdlsAutoLink))
    			{
				TDLS_MaintainDiscoveryEntryList(pAd);
    			}
#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
			STAMlmePeriodicExec(pAd);
		}
#endif /* CONFIG_STA_SUPPORT */



		MlmeResetRalinkCounters(pAd);

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef RTMP_MAC_PCI		
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && (pAd->bPCIclkOff == FALSE))
#endif /* RTMP_MAC_PCI */
#ifdef RTMP_MAC_PCI
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && 
				!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
#endif /* RTMP_MAC_PCI */
			{


			UINT32	MacReg = 0;
			
			RTMP_IO_READ32(pAd, 0x10F4, &MacReg);
			if (((MacReg & 0x20000000) && (MacReg & 0x80)) || ((MacReg & 0x20000000) && (MacReg & 0x20)))
			{
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
				RTMPusecDelay(1);
				{
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xC);
				}

				DBGPRINT(RT_DEBUG_WARN,("Warning, MAC specific condition occurs \n"));
			}
		}
		}
#endif /* CONFIG_STA_SUPPORT */

		RTMP_MLME_HANDLER(pAd);
	}

#ifdef RT6352
	if (IS_RT6352(pAd) && (pAd->CommonCfg.bEnTemperatureTrack == TRUE))
	{
#ifdef RTMP_INTERNAL_TX_ALC
		if (pAd->TxPowerCtrl.bInternalTxALC)
		{

			if (RT635xCheckTssiCompensation(pAd))
			{
#ifdef RTMP_TEMPERATURE_CALIBRATION
				RT6352_TemperatureCalibration(pAd);
#endif /* RTMP_TEMPERATURE_CALIBRATION */
				DoDPDCalibration(pAd);
			}
		}
		else
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef RTMP_TEMPERATURE_COMPENSATION
		if (pAd->bAutoTxAgcG)
		{
			if (RT6352_TemperatureCompensation(pAd, FALSE) == TRUE)
			{
				INT32 TemperatureDiff = 0;

				pAd->CommonCfg.bEnTemperatureTrack = FALSE;
				
				TemperatureDiff = ((pAd->CurrTemperature - pAd->TemperatureRef25C) * 19) - pAd->DoCalibrationTemperature;
			}
		}
		else
#endif /* RTMP_TEMPERATURE_COMPENSATION */
		{
#ifdef RTMP_TEMPERATURE_CALIBRATION
			UCHAR bbpval = 0;
			CHAR BBPR49 = 0;
			INT32 TemperatureDiff = 0;

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
			if ((bbpval & 0x10) == 0)
			{
				bbpval &= 0xf8;
				bbpval |= 0x04;
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);

				/* save temperature */ 
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
				pAd->CurrTemperature = (INT32) BBPR49;

				DBGPRINT(RT_DEBUG_INFO, ("Current Temperature from BBP_R49=0x%x\n", pAd->CurrTemperature));
				RT6352_TemperatureCalibration(pAd);
				pAd->CommonCfg.bEnTemperatureTrack = FALSE;

				TemperatureDiff = ((pAd->CurrTemperature - pAd->TemperatureRef25C) * 19) - pAd->DoCalibrationTemperature;
			}
#endif /* RTMP_TEMPERATURE_CALIBRATION */
		}
	}
#endif /* RT6352 */

#ifdef WSC_INCLUDED
	WSC_HDR_BTN_MR_HANDLE(pAd);
#endif /* WSC_INCLUDED */


#ifdef P2P_SUPPORT
	if (P2P_INF_ON(pAd))
		P2pPeriodicExec(SystemSpecific1, FunctionContext, SystemSpecific2, SystemSpecific3);
#endif /* P2P_SUPPORT */

	pAd->bUpdateBcnCntDone = FALSE;
}


/*
	==========================================================================
	Validate SSID for connection try and rescan purpose
	Valid SSID will have visible chars only.
	The valid length is from 0 to 32.
	IRQL = DISPATCH_LEVEL
	==========================================================================
 */
BOOLEAN MlmeValidateSSID(
	IN PUCHAR	pSsid,
	IN UCHAR	SsidLen)
{
	int	index;

	if (SsidLen > MAX_LEN_OF_SSID)
		return (FALSE);

	/* Check each character value*/
	for (index = 0; index < SsidLen; index++)
	{
		if (pSsid[index] < 0x20)
			return (FALSE);
	}

	/* All checked*/
	return (TRUE);
}


#ifdef CONFIG_STA_SUPPORT
VOID STAMlmePeriodicExec(
	PRTMP_ADAPTER pAd)
{
	ULONG			    TxTotalCnt;
	int 	i;
	BOOLEAN bCheckBeaconLost = TRUE;
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND	
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	RTMP_CHIP_HIGH_POWER_TUNING(pAd, &pAd->StaCfg.RssiSample);

#ifdef WPA_SUPPLICANT_SUPPORT
    if (pAd->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)    
#endif /* WPA_SUPPLICANT_SUPPORT */        
    {
    	/* WPA MIC error should block association attempt for 60 seconds*/
		if (pAd->StaCfg.bBlockAssoc && 
			RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastMicErrorTime + (60*OS_HZ)))
    		pAd->StaCfg.bBlockAssoc = FALSE;
    }



	

	if (ADHOC_ON(pAd))
	{
	}
	else
	{
		AsicStaBbpTuning(pAd);
	}
	
	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
					 pAd->RalinkCounters.OneSecTxRetryOkCount + 
					 pAd->RalinkCounters.OneSecTxFailCount;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && 
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
	{
		/* update channel quality for Roaming/Fast-Roaming and UI LinkQuality display*/
		/* bImprovedScan True means scan is not completed */
		if (pAd->StaCfg.bImprovedScan)
			bCheckBeaconLost = FALSE;
		
#ifdef P2P_SUPPORT
		if (NdisEqualMemory(ZERO_MAC_ADDR, pAd->P2pCfg.ConnectingMAC, MAC_ADDR_LEN) == FALSE)
		{
			UCHAR p2pindex;
			p2pindex = P2pGroupTabSearch(pAd, pAd->P2pCfg.ConnectingMAC);
			if (p2pindex != P2P_NOT_FOUND)
			{
				if (pAd->P2pTable.Client[p2pindex].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
					bCheckBeaconLost = FALSE;
			}			
		}
#endif /* P2P_SUPPORT */

		if (bCheckBeaconLost)
		{
			/* The NIC may lost beacons during scaning operation.*/
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[BSSID_WCID];
			MlmeCalculateChannelQuality(pAd, pEntry, pAd->Mlme.Now32);
		}
	}


	/* must be AFTER MlmeDynamicTxRateSwitching() because it needs to know if*/
	/* Radio is currently in noisy environment*/
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
#ifdef RT6352
		if (IS_RT6352(pAd))
			RT6352_AsicAdjustTxPower(pAd);
		else
#endif /* RT6352 */
		AsicAdjustTxPower(pAd);
	}

	/*
		Driver needs to up date value of LastOneSecTotalTxCount here;
		otherwise UI couldn't do scanning sometimes when STA doesn't connect to AP or peer Ad-Hoc.
	*/
	pAd->RalinkCounters.LastOneSecTotalTxCount = TxTotalCnt;
	

#ifdef P2P_SUPPORT
    /* MAC table maintenance */
	if ((pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0) && P2P_GO_ON(pAd))
	{
		/* one second timer */
	    P2PMacTableMaintenance(pAd);

#ifdef DOT11_N_SUPPORT
		if (pAd->CommonCfg.bHTProtect)
		{
			/*APUpdateCapabilityAndErpIe(pAd); */
			APUpdateOperationMode(pAd);
			if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
			{
				AsicUpdateProtect(pAd, (USHORT)pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, pAd->MacTab.fAnyStationNonGF);
			}
		}
#endif /* DOT11_N_SUPPORT */
	}
#endif /* P2P_SUPPORT */

		/* resume Improved Scanning*/
		if ((pAd->StaCfg.bImprovedScan) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
			(pAd->Mlme.SyncMachine.CurrState == SCAN_PENDING))
		{
			MLME_SCAN_REQ_STRUCT       ScanReq;

			pAd->StaCfg.LastScanTime = pAd->Mlme.Now32;
			
			ScanParmFill(pAd, &ScanReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
			MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
			DBGPRINT(RT_DEBUG_WARN, ("bImprovedScan ............. Resume for bImprovedScan, SCAN_PENDING .............. \n"));
		}

	if (INFRA_ON(pAd))
	{		
#ifdef QOS_DLS_SUPPORT
		/* Check DLS time out, then tear down those session*/
		RTMPCheckDLSTimeOut(pAd);
#endif /* QOS_DLS_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
    	/* TDLS link maintenance*/
    	if (IS_TDLS_SUPPORT(pAd))
    	{
			if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0)
			{
				TDLS_LinkMaintenance(pAd);
			}
    	}
#endif /* DOT11Z_TDLS_SUPPORT */

		/* Is PSM bit consistent with user power management policy?*/
		/* This is the only place that will set PSM bit ON.*/
		if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		MlmeCheckPsmChange(pAd, pAd->Mlme.Now32);

		/*
			When we are connected and do the scan progress, it's very possible we cannot receive
			the beacon of the AP. So, here we simulate that we received the beacon.
		*/
		if ((bCheckBeaconLost == FALSE) && 
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) && 
			(RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastBeaconRxTime + (1*OS_HZ))))
		{
			ULONG BPtoJiffies;
			LONG timeDiff;

			BPtoJiffies = (((pAd->CommonCfg.BeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);
			timeDiff = (pAd->Mlme.Now32 - pAd->StaCfg.LastBeaconRxTime) / BPtoJiffies;
			if (timeDiff > 0) 
				pAd->StaCfg.LastBeaconRxTime += (timeDiff * BPtoJiffies);

			if (RTMP_TIME_AFTER(pAd->StaCfg.LastBeaconRxTime, pAd->Mlme.Now32))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - BeaconRxTime adjust wrong(BeaconRx=0x%lx, Now=0x%lx)\n", 
								pAd->StaCfg.LastBeaconRxTime, pAd->Mlme.Now32));
			}
		}
		
		if ((RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastBeaconRxTime + (1*OS_HZ))) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
			(pAd->StaCfg.bImprovedScan == FALSE) &&
			(((TxTotalCnt + pAd->RalinkCounters.OneSecRxOkCnt) < 600)))
		{
			RTMPSetAGCInitValue(pAd, BW_20);
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - No BEACON. restore R66 to the low bound(%d) \n", (0x2E + GET_LNA_GAIN(pAd))));
		}


				

        /*if ((pAd->RalinkCounters.OneSecTxNoRetryOkCount == 0) &&*/
        /*    (pAd->RalinkCounters.OneSecTxRetryOkCount == 0))*/
       if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
        {
    		if (pAd->StaCfg.UapsdInfo.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable)
    		{
    		    /* When APSD is enabled, the period changes as 20 sec*/
    			if ((pAd->Mlme.OneSecPeriodicRound % 20) == 8)
    			{
    				RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
    			}
    		}
    		else
    		{
    		    /* Send out a NULL frame every 10 sec to inform AP that STA is still alive (Avoid being age out)*/
    			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
			{
				RTMPSendNullFrame(pAd, 
								  pAd->CommonCfg.TxRate, 
								  (pAd->CommonCfg.bWmmCapable & pAd->CommonCfg.APEdcaParm.bValid),
								  pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
			}
    		}
        }

		if (CQI_IS_DEAD(pAd->Mlme.ChannelQuality))
			{
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - No BEACON. Dead CQI. Auto Recovery attempt #%ld\n", pAd->RalinkCounters.BadCQIAutoRecoveryCount));

			if (pAd->StaCfg.bAutoConnectByBssid)
				pAd->StaCfg.bAutoConnectByBssid = FALSE;
			
#ifdef WPA_SUPPLICANT_SUPPORT
			if ((pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
				(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2))
				pAd->StaCfg.bLostAp = TRUE;
#endif /* WPA_SUPPLICANT_SUPPORT */

			pAd->MlmeAux.CurrReqIsFromNdis = FALSE;
			/* Lost AP, send disconnect & link down event*/
			LinkDown(pAd, FALSE);
			
/* should mark this two function, because link down alse will call this function */
			/* RTMPPatchMacBbpBug(pAd);*/
			MlmeAutoReconnectLastSSID(pAd);
		}
		else if (CQI_IS_BAD(pAd->Mlme.ChannelQuality))
		{
			pAd->RalinkCounters.BadCQIAutoRecoveryCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Bad CQI. Auto Recovery attempt #%ld\n", pAd->RalinkCounters.BadCQIAutoRecoveryCount));
			MlmeAutoReconnectLastSSID(pAd);
		}
		
		if (pAd->StaCfg.bAutoRoaming)
		{
			BOOLEAN	rv = FALSE;
			CHAR	dBmToRoam = pAd->StaCfg.dBmToRoam;
			CHAR 	MaxRssi = RTMPMaxRssi(pAd, 
										  pAd->StaCfg.RssiSample.LastRssi0, 
										  pAd->StaCfg.RssiSample.LastRssi1, 
										  pAd->StaCfg.RssiSample.LastRssi2);			
			
			if (pAd->StaCfg.bAutoConnectByBssid)
				pAd->StaCfg.bAutoConnectByBssid = FALSE;
			
			/* Scanning, ignore Roaming*/
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
				(pAd->Mlme.SyncMachine.CurrState == SYNC_IDLE) &&
				(MaxRssi <= dBmToRoam))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Rssi=%d, dBmToRoam=%d\n", MaxRssi, (CHAR)dBmToRoam));


				/* Add auto seamless roaming*/
				if (rv == FALSE)
					rv = MlmeCheckForFastRoaming(pAd);
				
				if (rv == FALSE)
				{
					if ((pAd->StaCfg.LastScanTime + 10 * OS_HZ) < pAd->Mlme.Now32)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming, No eligable entry, try new scan!\n"));
						pAd->StaCfg.LastScanTime = pAd->Mlme.Now32;
						MlmeAutoScan(pAd);
					}
				}
			}
		}
	}
	else if (ADHOC_ON(pAd))
	{

		/* If all peers leave, and this STA becomes the last one in this IBSS, then change MediaState*/
		/* to DISCONNECTED. But still holding this IBSS (i.e. sending BEACON) so that other STAs can*/
		/* join later.*/
		if (/*(RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastBeaconRxTime + ADHOC_BEACON_LOST_TIME)
			|| (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
			&& */OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		{

			for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) 
			{
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

				if (!IS_ENTRY_CLIENT(pEntry))
					continue;

				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pEntry->LastBeaconRxTime + ADHOC_BEACON_LOST_TIME)
#ifdef IWSC_SUPPORT
					/*
						2011/09/05:
						Broadcom test bed doesn't broadcast beacon when Broadcom is Enrollee.
					*/
					&& (pAd->StaCfg.WscControl.bWscTrigger == FALSE)
#endif /* IWSC_SUPPORT */
					)
                    MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
			}

            if (pAd->MacTab.Size == 0)
            {			                
    			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
    			RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
            }            
		}
			
	}
	else /* no INFRA nor ADHOC connection*/
	{

#ifdef P2P_SUPPORT
		if (P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
			goto SKIP_AUTO_SCAN_CONN;
#endif /* P2P_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT
		if (pAd->StaCfg.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
			goto SKIP_AUTO_SCAN_CONN;
#endif /* WPA_SUPPLICANT_SUPPORT */

		if (pAd->StaCfg.bSkipAutoScanConn &&
			RTMP_TIME_BEFORE(pAd->Mlme.Now32, pAd->StaCfg.LastScanTime + (30 * OS_HZ)))
			goto SKIP_AUTO_SCAN_CONN;
		else
			pAd->StaCfg.bSkipAutoScanConn = FALSE;
        
		if ((pAd->StaCfg.bAutoReconnect == TRUE)
			&& RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
			&& (MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == TRUE))
		{
			if ((pAd->ScanTab.BssNr==0) && (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
				)
			{
				MLME_SCAN_REQ_STRUCT	   ScanReq;

				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastScanTime + (10 * OS_HZ))
					||(pAd->StaCfg.bNotFirstScan == FALSE))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("STAMlmePeriodicExec():CNTL - ScanTab.BssNr==0, start a new ACTIVE scan SSID[%s]\n", pAd->MlmeAux.AutoReconnectSsid));
					if (pAd->StaCfg.BssType == BSS_ADHOC)	
						pAd->StaCfg.bNotFirstScan = TRUE;
					ScanParmFill(pAd, &ScanReq, (PSTRING) pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen, BSS_ANY, SCAN_ACTIVE);
					MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
					pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
					/* Reset Missed scan number*/
					pAd->StaCfg.LastScanTime = pAd->Mlme.Now32;
				}
				else
					MlmeAutoReconnectLastSSID(pAd);
			}
			else if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
			{
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier*/
				if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
				{
					if ((pAd->Mlme.OneSecPeriodicRound % 5) == 1)
						MlmeAutoReconnectLastSSID(pAd);
				}
				else
#endif /* CARRIER_DETECTION_SUPPORT */ 
				{
#ifdef WPA_SUPPLICANT_SUPPORT
					if(pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_ENABLE)
#endif // WPA_SUPPLICANT_SUPPORT //
					MlmeAutoReconnectLastSSID(pAd);
				}
			}
		}
	}

SKIP_AUTO_SCAN_CONN:

#ifdef DOT11_N_SUPPORT
    if ((pAd->MacTab.Content[BSSID_WCID].TXBAbitmap !=0) && (pAd->MacTab.fAnyBASession == FALSE)
		&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		pAd->MacTab.fAnyBASession = TRUE;
		AsicUpdateProtect(pAd, HT_FORCERTSCTS,  ALLN_SETPROTECT, FALSE, FALSE);
	}
	else if ((pAd->MacTab.Content[BSSID_WCID].TXBAbitmap ==0) && (pAd->MacTab.fAnyBASession == TRUE)
		&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		pAd->MacTab.fAnyBASession = FALSE;
		AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, FALSE);
	}
#endif /* DOT11_N_SUPPORT */


#ifdef P2P_SUPPORT
	if (P2P_CLI_ON(pAd))
	{
		if (pAd->Mlme.OneSecPeriodicRound % 2 == 0)
			ApCliIfMonitor(pAd);

		if (pAd->Mlme.OneSecPeriodicRound % 2 == 1)
			ApCliIfUp(pAd);

		{
			INT loop;
			ULONG Now32;
			NdisGetSystemUpTime(&Now32);
			for (loop = 0; loop < MAX_APCLI_NUM; loop++)
			{
				PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[loop];
				if ((pApCliEntry->Valid == TRUE)
					&& (pApCliEntry->MacTabWCID < MAX_LEN_OF_MAC_TABLE))
				{
					/*
						When we are connected and do the scan progress, it's very possible we cannot receive
						the beacon of the AP. So, here we simulate that we received the beacon.
					*/
					if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
						(RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApCliRcvBeaconTime + (1*OS_HZ))))
					{
						ULONG BPtoJiffies;
						LONG timeDiff;

						BPtoJiffies = (((pApCliEntry->ApCliBeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);
						timeDiff = (pAd->Mlme.Now32 - pApCliEntry->ApCliRcvBeaconTime) / BPtoJiffies;
						if (timeDiff > 0) 
							pApCliEntry->ApCliRcvBeaconTime += (timeDiff * BPtoJiffies);

						if (RTMP_TIME_AFTER(pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32))
						{
							DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - APCli BeaconRxTime adjust wrong(BeaconRx=0x%lx, Now=0x%lx)\n", 
														pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32));
						}
					}

					/* update channel quality for Roaming and UI LinkQuality display */
					MlmeCalculateChannelQuality(pAd,
						&pAd->MacTab.Content[pApCliEntry->MacTabWCID], Now32);
				}
			}
		}
	}
#endif /* P2P_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	/* Perform 20/40 BSS COEX scan every Dot11BssWidthTriggerScanInt	*/
	if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) && 
		(pAd->CommonCfg.Dot11BssWidthTriggerScanInt != 0) && 
		((pAd->Mlme.OneSecPeriodicRound % pAd->CommonCfg.Dot11BssWidthTriggerScanInt) == (pAd->CommonCfg.Dot11BssWidthTriggerScanInt-1)))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - LastOneSecTotalTxCount/LastOneSecRxOkDataCnt  = %d/%d \n", 
								pAd->RalinkCounters.LastOneSecTotalTxCount,
								pAd->RalinkCounters.LastOneSecRxOkDataCnt));
		
		/* Check last scan time at least 30 seconds from now. 		*/
		/* Check traffic is less than about 1.5~2Mbps.*/
		/* it might cause data lost if we enqueue scanning.*/
		/* This criteria needs to be considered*/
		if ((pAd->RalinkCounters.LastOneSecTotalTxCount < 70) && (pAd->RalinkCounters.LastOneSecRxOkDataCnt < 70)
			/*&& ((pAd->StaCfg.LastScanTime + 10 * OS_HZ) < pAd->Mlme.Now32) */)		
		{
			MLME_SCAN_REQ_STRUCT            ScanReq;
			/* Fill out stuff for scan request and kick to scan*/
			ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
			MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
			/* Set InfoReq = 1, So after scan , alwats sebd 20/40 Coexistence frame to AP*/
			pAd->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
			RTMP_MLME_HANDLER(pAd);
		}

		DBGPRINT(RT_DEBUG_TRACE, (" LastOneSecTotalTxCount/LastOneSecRxOkDataCnt  = %d/%d \n", 
							pAd->RalinkCounters.LastOneSecTotalTxCount, 
							pAd->RalinkCounters.LastOneSecRxOkDataCnt));	
	}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	return;
}

/* Link down report*/
VOID LinkDownExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	if (pAd != NULL)
	{
		MLME_DISASSOC_REQ_STRUCT   DisassocReq;
		
		if ((pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED) &&
			(INFRA_ON(pAd)))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("LinkDownExec(): disassociate with current AP...\n"));
			DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
						sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, 0);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;

			RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
		    pAd->ExtraInfo = GENERAL_LINK_DOWN;
		}	
	}
}

/* IRQL = DISPATCH_LEVEL*/
VOID MlmeAutoScan(
	IN PRTMP_ADAPTER pAd)
{
	/* check CntlMachine.CurrState to avoid collision with NDIS SetOID request*/
	if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Driver auto scan\n"));
		MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE, 
					OID_802_11_BSSID_LIST_SCAN, 
					pAd->MlmeAux.AutoReconnectSsidLen, 
					pAd->MlmeAux.AutoReconnectSsid, 0);
		RTMP_MLME_HANDLER(pAd);
	}
}
	
/* IRQL = DISPATCH_LEVEL*/
VOID MlmeAutoReconnectLastSSID(
	IN PRTMP_ADAPTER pAd)
{
#ifdef WSC_STA_SUPPORT
	PWSC_CTRL           pWscControl = &pAd->StaCfg.WscControl;

	if ((pWscControl->WscConfMode != WSC_DISABLE) && 
		(pWscControl->bWscTrigger) &&
		(pWscControl->WscMode == WSC_PBC_MODE) &&
		(pWscControl->WscPBCBssCount != 1))
		return;

	if ((pWscControl->WscConfMode != WSC_DISABLE) &&
		(pWscControl->WscState >= WSC_STATE_START))
	{		
		ULONG ApIdx = 0;

		ApIdx = WscSearchWpsApBySSID(pAd,
									 pWscControl->WscSsid.Ssid, 
									 pWscControl->WscSsid.SsidLength,
									 pWscControl->WscMode);

		if ((ApIdx != BSS_NOT_FOUND) &&
			(pAd->StaCfg.BssType == BSS_INFRA))
		{
			NdisMoveMemory(pWscControl->WscBssid, pAd->ScanTab.BssEntry[ApIdx].Bssid, MAC_ADDR_LEN);
			pAd->MlmeAux.Channel = pAd->ScanTab.BssEntry[ApIdx].Channel;
		}

		CntlWscIterate(pAd);
	}
	else
#endif /* WSC_STA_SUPPORT */
	if (pAd->StaCfg.bAutoConnectByBssid)
	{	
		DBGPRINT(RT_DEBUG_TRACE, ("Driver auto reconnect to last OID_802_11_BSSID setting - %02X:%02X:%02X:%02X:%02X:%02X\n",
									pAd->MlmeAux.Bssid[0],
									pAd->MlmeAux.Bssid[1],
									pAd->MlmeAux.Bssid[2],
									pAd->MlmeAux.Bssid[3],
									pAd->MlmeAux.Bssid[4],
									pAd->MlmeAux.Bssid[5]));

		pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
		MlmeEnqueue(pAd,
			 MLME_CNTL_STATE_MACHINE,
			 OID_802_11_BSSID,
			 MAC_ADDR_LEN,
			 pAd->MlmeAux.Bssid, 0);

		pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

		RTMP_MLME_HANDLER(pAd);
	}
	/* check CntlMachine.CurrState to avoid collision with NDIS SetOID request*/
	else if ((pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE) && 
		(MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == TRUE))
	{
		NDIS_802_11_SSID OidSsid;
		OidSsid.SsidLength = pAd->MlmeAux.AutoReconnectSsidLen;
		NdisMoveMemory(OidSsid.Ssid, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

		DBGPRINT(RT_DEBUG_TRACE, ("Driver auto reconnect to last OID_802_11_SSID setting - %s, len - %d\n", pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen));
		MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE, 
					OID_802_11_SSID, 
					sizeof(NDIS_802_11_SSID), 
					&OidSsid, 0);
		RTMP_MLME_HANDLER(pAd);
	}
}


/*
	==========================================================================
	Description:
		This routine checks if there're other APs out there capable for
		roaming. Caller should call this routine only when Link up in INFRA mode
		and channel quality is below CQI_GOOD_THRESHOLD.

	IRQL = DISPATCH_LEVEL

	Output:
	==========================================================================
 */
VOID MlmeCheckForRoaming(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	Now32)
{
	USHORT	   i;
	BSS_TABLE  *pRoamTab = &pAd->MlmeAux.RoamTab;
	BSS_ENTRY  *pBss;

	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeCheckForRoaming\n"));
	/* put all roaming candidates into RoamTab, and sort in RSSI order*/
	BssTableInit(pRoamTab);
	for (i = 0; i < pAd->ScanTab.BssNr; i++)
	{
		pBss = &pAd->ScanTab.BssEntry[i];

		if (RTMP_TIME_AFTER(Now32, pBss->LastBeaconRxTime + pAd->StaCfg.BeaconLostTime))
			continue;	 /* AP disappear*/
		if (pBss->Rssi <= RSSI_THRESHOLD_FOR_ROAMING)
			continue;	 /* RSSI too weak. forget it.*/
		if (MAC_ADDR_EQUAL(pBss->Bssid, pAd->CommonCfg.Bssid))
			continue;	 /* skip current AP*/
		if (pBss->Rssi < (pAd->StaCfg.RssiSample.LastRssi0 + RSSI_DELTA))
			continue;	 /* only AP with stronger RSSI is eligible for roaming*/

		/* AP passing all above rules is put into roaming candidate table		 */
		NdisMoveMemory(&pRoamTab->BssEntry[pRoamTab->BssNr], pBss, sizeof(BSS_ENTRY));
		pRoamTab->BssNr += 1;
	}

	if (pRoamTab->BssNr > 0)
	{
		/* check CntlMachine.CurrState to avoid collision with NDIS SetOID request*/
		if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
		{
			pAd->RalinkCounters.PoorCQIRoamingCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming attempt #%ld\n", pAd->RalinkCounters.PoorCQIRoamingCount));
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_MLME_ROAMING_REQ, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForRoaming(# of candidate= %d)\n",pRoamTab->BssNr));   
}

/*
	==========================================================================
	Description:
		This routine checks if there're other APs out there capable for
		roaming. Caller should call this routine only when link up in INFRA mode
		and channel quality is below CQI_GOOD_THRESHOLD.

	IRQL = DISPATCH_LEVEL

	Output:
	==========================================================================
 */
BOOLEAN MlmeCheckForFastRoaming(
	IN	PRTMP_ADAPTER	pAd)
{
	USHORT		i;
	BSS_TABLE	*pRoamTab = &pAd->MlmeAux.RoamTab;
	BSS_ENTRY	*pBss;

	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeCheckForFastRoaming\n"));
	/* put all roaming candidates into RoamTab, and sort in RSSI order*/
	BssTableInit(pRoamTab);
	for (i = 0; i < pAd->ScanTab.BssNr; i++)
	{
		pBss = &pAd->ScanTab.BssEntry[i];

        if ((pBss->Rssi <= -50) && (pBss->Channel == pAd->CommonCfg.Channel))
			continue;	 /* RSSI too weak. forget it.*/
		if (MAC_ADDR_EQUAL(pBss->Bssid, pAd->CommonCfg.Bssid))
			continue;	 /* skip current AP*/
		if (!SSID_EQUAL(pBss->Ssid, pBss->SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen))
			continue;	 /* skip different SSID*/
        if (pBss->Rssi < (RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2) + RSSI_DELTA)) 
			continue;	 /* skip AP without better RSSI*/
		
        DBGPRINT(RT_DEBUG_TRACE, ("LastRssi0 = %d, pBss->Rssi = %d\n", RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2), pBss->Rssi));
		/* AP passing all above rules is put into roaming candidate table		 */
		NdisMoveMemory(&pRoamTab->BssEntry[pRoamTab->BssNr], pBss, sizeof(BSS_ENTRY));
		pRoamTab->BssNr += 1;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForFastRoaming (BssNr=%d)\n", pRoamTab->BssNr));
	if (pRoamTab->BssNr > 0)
	{
		/* check CntlMachine.CurrState to avoid collision with NDIS SetOID request*/
		if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
		{
			pAd->RalinkCounters.PoorCQIRoamingCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming attempt #%ld\n", pAd->RalinkCounters.PoorCQIRoamingCount));
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_MLME_ROAMING_REQ, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
			return TRUE;
		}
	}

	return FALSE;
}


/*
	==========================================================================
	Description:
		This routine is executed periodically inside MlmePeriodicExec() after 
		association with an AP.
		It checks if StaCfg.Psm is consistent with user policy (recorded in
		StaCfg.WindowsPowerMode). If not, enforce user policy. However, 
		there're some conditions to consider:
		1. we don't support power-saving in ADHOC mode, so Psm=PWR_ACTIVE all
		   the time when Mibss==TRUE
		2. When link up in INFRA mode, Psm should not be switch to PWR_SAVE
		   if outgoing traffic available in TxRing or MgmtRing.
	Output:
		1. change pAd->StaCfg.Psm to PWR_SAVE or leave it untouched

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeCheckPsmChange(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	Now32)
{
	ULONG	PowerMode;

	/*
		condition -
		1. Psm maybe ON only happen in INFRASTRUCTURE mode
		2. user wants either MAX_PSP or FAST_PSP
		3. but current psm is not in PWR_SAVE
		4. CNTL state machine is not doing SCANning
		5. no TX SUCCESS event for the past 1-sec period
	*/
	PowerMode = pAd->StaCfg.WindowsPowerMode;

	if (INFRA_ON(pAd) &&
		(PowerMode != Ndis802_11PowerModeCAM) &&
		(pAd->StaCfg.Psm == PWR_ACTIVE) &&
/*		(! RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))*/
		(pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
#ifdef PCIE_PS_SUPPORT
		&& RTMP_TEST_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP)
#endif /* PCIE_PS_SUPPORT */
		 /*&&
		(pAd->RalinkCounters.OneSecTxNoRetryOkCount == 0) &&
		(pAd->RalinkCounters.OneSecTxRetryOkCount == 0)*/)
	{
		NdisGetSystemUpTime(&pAd->Mlme.LastSendNULLpsmTime);
		pAd->RalinkCounters.RxCountSinceLastNULL = 0;
		RTMP_SET_PSM_BIT(pAd, PWR_SAVE);

		if (!(pAd->StaCfg.UapsdInfo.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable))
		{
			RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE), pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
		}
		else
		{
			RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE, pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
		}

	}
}

/* IRQL = PASSIVE_LEVEL*/
/* IRQL = DISPATCH_LEVEL*/
VOID MlmeSetPsmBit(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT psm)
{
#ifdef DOT11Z_TDLS_SUPPORT
	USHORT PsmOld = pAd->StaCfg.Psm;
#endif /* DOT11Z_TDLS_SUPPORT */

	pAd->StaCfg.Psm = psm;	  

#ifdef DOT11Z_TDLS_SUPPORT
	/* pAd->StaCfg.Psm must be updated before calling the function */
	TDLS_UAPSDP_PsmModeChange(pAd, PsmOld, psm);
#endif /* DOT11Z_TDLS_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsmBit = %d\n", psm));
}
#endif /* CONFIG_STA_SUPPORT */

/*
	==========================================================================
	Description:
		This routine calculates TxPER, RxPER of the past N-sec period. And 
		according to the calculation result, ChannelQuality is calculated here 
		to decide if current AP is still doing the job. 

		If ChannelQuality is not good, a ROAMing attempt may be tried later.
	Output:
		StaCfg.ChannelQuality - 0..100

	IRQL = DISPATCH_LEVEL

	NOTE: This routine decide channle quality based on RX CRC error ratio.
		Caller should make sure a function call to NICUpdateRawCounters(pAd)
		is performed right before this routine, so that this routine can decide
		channel quality based on the most up-to-date information
	==========================================================================
 */
VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now32)
{
	ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
	ULONG RxCnt, RxPER;
	UCHAR NorRssi;
	CHAR  MaxRssi;
	RSSI_SAMPLE *pRssiSample = NULL;
	UINT32 OneSecTxNoRetryOkCount = 0;
	UINT32 OneSecTxRetryOkCount = 0;
	UINT32 OneSecTxFailCount = 0;
	UINT32 OneSecRxOkCnt = 0;
	UINT32 OneSecRxFcsErrCnt = 0;
	ULONG ChannelQuality = 0;  /* 0..100, Channel Quality Indication for Roaming*/
#ifdef CONFIG_STA_SUPPORT
	ULONG	LastBeaconRxTime = 0;
	ULONG BeaconLostTime = pAd->StaCfg.BeaconLostTime;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier*/
	/* longer beacon lost time when carrier detection enabled*/
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
	{
		BeaconLostTime = pAd->StaCfg.BeaconLostTime + (pAd->StaCfg.BeaconLostTime/2);
	}
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef APCLI_SUPPORT
		if (pMacEntry && IS_ENTRY_APCLI(pMacEntry) && (pMacEntry->MatchAPCLITabIdx < MAX_APCLI_NUM))
			LastBeaconRxTime = pAd->ApCfg.ApCliTab[pMacEntry->MatchAPCLITabIdx].ApCliRcvBeaconTime; 
		else
#endif /*APCLI_SUPPORT*/
			LastBeaconRxTime = pAd->StaCfg.LastBeaconRxTime;
#endif /* CONFIG_STA_SUPPORT */


		if (pMacEntry != NULL)
		{
			pRssiSample = &pMacEntry->RssiSample;
			OneSecTxNoRetryOkCount = pMacEntry->OneSecTxNoRetryOkCount;
			OneSecTxRetryOkCount = pMacEntry->OneSecTxRetryOkCount;
			OneSecTxFailCount = pMacEntry->OneSecTxFailCount;
			OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
			OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
		}
		else
		{
			pRssiSample = &pAd->MacTab.Content[0].RssiSample;
			OneSecTxNoRetryOkCount = pAd->RalinkCounters.OneSecTxNoRetryOkCount;
			OneSecTxRetryOkCount = pAd->RalinkCounters.OneSecTxRetryOkCount;
			OneSecTxFailCount = pAd->RalinkCounters.OneSecTxFailCount;
			OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
			OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
		}

	if (pRssiSample == NULL)
		return;
	MaxRssi = RTMPMaxRssi(pAd, pRssiSample->LastRssi0,
								pRssiSample->LastRssi1,
								pRssiSample->LastRssi2);

	
	/*
		calculate TX packet error ratio and TX retry ratio - if too few TX samples, 
		skip TX related statistics
	*/	
	TxOkCnt = OneSecTxNoRetryOkCount + OneSecTxRetryOkCount;
	TxCnt = TxOkCnt + OneSecTxFailCount;
	if (TxCnt < 5) 
	{
		TxPER = 0;
		TxPRR = 0;
	}
	else 
	{
		TxPER = (OneSecTxFailCount * 100) / TxCnt; 
		TxPRR = ((TxCnt - OneSecTxNoRetryOkCount) * 100) / TxCnt;
	}

	
	/* calculate RX PER - don't take RxPER into consideration if too few sample*/
	RxCnt = OneSecRxOkCnt + OneSecRxFcsErrCnt;
	if (RxCnt < 5)
		RxPER = 0;	
	else
		RxPER = (OneSecRxFcsErrCnt * 100) / RxCnt;

	
	/* decide ChannelQuality based on: 1)last BEACON received time, 2)last RSSI, 3)TxPER, and 4)RxPER*/
#ifdef CONFIG_STA_SUPPORT
	if ((pAd->OpMode == OPMODE_STA) &&
		INFRA_ON(pAd) && 
		(OneSecTxNoRetryOkCount < 2) && /* no heavy traffic*/
		RTMP_TIME_AFTER(Now32, LastBeaconRxTime + BeaconLostTime))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("BEACON lost > %ld msec with TxOkCnt=%ld -> CQI=0\n", BeaconLostTime * (1000 / OS_HZ) , TxOkCnt)); 
		ChannelQuality = 0;
	}
	else
#endif /* CONFIG_STA_SUPPORT */
	{
		/* Normalize Rssi*/
		if (MaxRssi > -40)
			NorRssi = 100;
		else if (MaxRssi < -90)
			NorRssi = 0;
		else
			NorRssi = (MaxRssi + 90) * 2;
		
		/* ChannelQuality = W1*RSSI + W2*TxPRR + W3*RxPER	 (RSSI 0..100), (TxPER 100..0), (RxPER 100..0)*/
		ChannelQuality = (RSSI_WEIGHTING * NorRssi + 
								   TX_WEIGHTING * (100 - TxPRR) + 
								   RX_WEIGHTING* (100 - RxPER)) / 100;
	}


#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
		pAd->Mlme.ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		if (pMacEntry != NULL)
			pMacEntry->ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
	}
#endif /* CONFIG_AP_SUPPORT */

	
}


/* IRQL = DISPATCH_LEVEL*/
VOID MlmeSetTxPreamble(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TxPreamble)
{
	AUTO_RSP_CFG_STRUC csr4;

	
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/
	
	/*TxPreamble = Rt802_11PreambleLong;*/
	
	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &csr4.word);
	if (TxPreamble == Rt802_11PreambleLong)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetTxPreamble (= LONG PREAMBLE)\n"));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED); 
		csr4.field.AutoResponderPreamble = 0;
	}
	else
	{
		/* NOTE: 1Mbps should always use long preamble*/
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetTxPreamble (= SHORT PREAMBLE)\n"));
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
		csr4.field.AutoResponderPreamble = 1;
	}

	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, csr4.word);
}

/*
    ==========================================================================
    Description:
        Update basic rate bitmap
    ==========================================================================
 */
 
VOID UpdateBasicRateBitmap(
    IN  PRTMP_ADAPTER   pAdapter)
{
    INT  i, j;
                  /* 1  2  5.5, 11,  6,  9, 12, 18, 24, 36, 48,  54 */
    UCHAR rate[] = { 2, 4,  11, 22, 12, 18, 24, 36, 48, 72, 96, 108 };
    UCHAR *sup_p = pAdapter->CommonCfg.SupRate;
    UCHAR *ext_p = pAdapter->CommonCfg.ExtRate;
    ULONG bitmap = pAdapter->CommonCfg.BasicRateBitmap;

    /* if A mode, always use fix BasicRateBitMap */
    /*if (pAdapter->CommonCfg.Channel == PHY_11A)*/
	if (pAdapter->CommonCfg.Channel > 14)
	{
		if (pAdapter->CommonCfg.BasicRateBitmap & 0xF)
		{
			/* no 11b rate in 5G band */
			pAdapter->CommonCfg.BasicRateBitmapOld = \
										pAdapter->CommonCfg.BasicRateBitmap;
			pAdapter->CommonCfg.BasicRateBitmap &= (~0xF); /* no 11b */
		}

		/* force to 6,12,24M in a-band */
		pAdapter->CommonCfg.BasicRateBitmap |= 0x150; /* 6, 12, 24M */
    }
	else
	{
		/* no need to modify in 2.4G (bg mixed) */
		pAdapter->CommonCfg.BasicRateBitmap = \
										pAdapter->CommonCfg.BasicRateBitmapOld;
	} /* End of if */

    if (pAdapter->CommonCfg.BasicRateBitmap > 4095)
    {
        /* (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1 */
        return;
    } /* End of if */

	bitmap = pAdapter->CommonCfg.BasicRateBitmap;  /* renew bitmap value */

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        sup_p[i] &= 0x7f;
        ext_p[i] &= 0x7f;
    } /* End of for */

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        if (bitmap & (1 << i))
        {
            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (sup_p[j] == rate[i])
                    sup_p[j] |= 0x80;
                /* End of if */
            } /* End of for */

            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (ext_p[j] == rate[i])
                    ext_p[j] |= 0x80;
                /* End of if */
            } /* End of for */
        } /* End of if */
    } /* End of for */
} /* End of UpdateBasicRateBitmap */

/*
	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	bLinkUp is to identify the inital link speed.
	TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
*/
VOID MlmeUpdateTxRates(
	IN PRTMP_ADAPTER 		pAd,
	IN 	BOOLEAN		 		bLinkUp,
	IN	UCHAR				apidx)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	PHTTRANSMIT_SETTING		pHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMinHtPhy = NULL;	
	BOOLEAN 				*auto_rate_cur_p;
	UCHAR					HtMcs = MCS_AUTO;

	/* find max desired rate*/
	UpdateBasicRateBitmap(pAd);
	
	num = 0;
	auto_rate_cur_p = NULL;
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		switch (pAd->CommonCfg.DesireRate[i] & 0x7f)
		{
			case 2:  Rate = RATE_1;   num++;   break;
			case 4:  Rate = RATE_2;   num++;   break;
			case 11: Rate = RATE_5_5; num++;   break;
			case 22: Rate = RATE_11;  num++;   break;
			case 12: Rate = RATE_6;   num++;   break;
			case 18: Rate = RATE_9;   num++;   break;
			case 24: Rate = RATE_12;  num++;   break;
			case 36: Rate = RATE_18;  num++;   break;
			case 48: Rate = RATE_24;  num++;   break;
			case 72: Rate = RATE_36;  num++;   break;
			case 96: Rate = RATE_48;  num++;   break;
			case 108: Rate = RATE_54; num++;   break;
			/*default: Rate = RATE_1;   break;*/
		}
		if (MaxDesire < Rate)  MaxDesire = Rate;
	}

/*===========================================================================*/
/*===========================================================================*/
	do
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_P2P_GO)
		{	
			UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_P2P_GO;
							
			pHtPhy 		= &pAd->ApCfg.MBSSID[idx].HTPhyMode;	
			pMaxHtPhy	= &pAd->ApCfg.MBSSID[idx].MaxHTPhyMode;
			pMinHtPhy	= &pAd->ApCfg.MBSSID[idx].MinHTPhyMode;

			auto_rate_cur_p = &pAd->ApCfg.MBSSID[idx].bAutoTxRateSwitch;	
			HtMcs 		= pAd->ApCfg.MBSSID[idx].DesiredTransmitSetting.field.MCS;
			break;
		}
#endif /* P2P_SUPPORT */
#ifdef APCLI_SUPPORT	
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
		{			
			UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;
			
			if (idx < MAX_APCLI_NUM)
			{
			pHtPhy 		= &pAd->ApCfg.ApCliTab[idx].HTPhyMode;	
			pMaxHtPhy	= &pAd->ApCfg.ApCliTab[idx].MaxHTPhyMode;
			pMinHtPhy	= &pAd->ApCfg.ApCliTab[idx].MinHTPhyMode;

			auto_rate_cur_p = &pAd->ApCfg.ApCliTab[idx].bAutoTxRateSwitch;	
			HtMcs 		= pAd->ApCfg.ApCliTab[idx].DesiredTransmitSetting.field.MCS;
			break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MlmeUpdateTxRates: invalid idx(%d)\n", idx));
				return;
			}
		}
#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{			
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;

			if (idx < MAX_WDS_ENTRY)
			{
				pHtPhy 		= &pAd->WdsTab.WdsEntry[idx].HTPhyMode;	
				pMaxHtPhy	= &pAd->WdsTab.WdsEntry[idx].MaxHTPhyMode;
				pMinHtPhy	= &pAd->WdsTab.WdsEntry[idx].MinHTPhyMode;

				auto_rate_cur_p = &pAd->WdsTab.WdsEntry[idx].bAutoTxRateSwitch;	
				HtMcs 		= pAd->WdsTab.WdsEntry[idx].DesiredTransmitSetting.field.MCS;
				break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MlmeUpdateTxRates: invalid apidx(%d)\n", apidx));
				return;
			}
			}
#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) && 
				(apidx < MAX_MBSSID_NUM(pAd)) &&
				(apidx < HW_BEACON_MAX_NUM))
			{								
				pHtPhy 		= &pAd->ApCfg.MBSSID[apidx].HTPhyMode;	
				pMaxHtPhy	= &pAd->ApCfg.MBSSID[apidx].MaxHTPhyMode;
				pMinHtPhy	= &pAd->ApCfg.MBSSID[apidx].MinHTPhyMode;

				auto_rate_cur_p = &pAd->ApCfg.MBSSID[apidx].bAutoTxRateSwitch;	
				HtMcs 		= pAd->ApCfg.MBSSID[apidx].DesiredTransmitSetting.field.MCS;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MlmeUpdateTxRates: invalid apidx(%d)\n", apidx));
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pHtPhy 		= &pAd->StaCfg.HTPhyMode;
			pMaxHtPhy	= &pAd->StaCfg.MaxHTPhyMode;
			pMinHtPhy	= &pAd->StaCfg.MinHTPhyMode;		

			auto_rate_cur_p = &pAd->StaCfg.bAutoTxRateSwitch;
			HtMcs 		= pAd->StaCfg.DesiredTransmitSetting.field.MCS;

			if ((pAd->StaCfg.BssType == BSS_ADHOC) &&
				(pAd->CommonCfg.PhyMode == PHY_11B) && 
				(MaxDesire > RATE_11))
			{
				MaxDesire = RATE_11;
			}
			break;
		}
#endif /* CONFIG_STA_SUPPORT */
	} while(FALSE);


	pAd->CommonCfg.MaxDesiredRate = MaxDesire;

	if (pMinHtPhy == NULL)
		return;
	pMinHtPhy->word = 0;
	pMaxHtPhy->word = 0;
	pHtPhy->word = 0;

	/*
		Auto rate switching is enabled only if more than one DESIRED RATES are
		specified; otherwise disabled
	*/
	if (num <= 1)
	{
		/*OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED);*/
		/*pAd->CommonCfg.bAutoTxRateSwitch	= FALSE;*/
		*auto_rate_cur_p = FALSE;
	}
	else
	{
		/*OPSTATUS_SET_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED); */
		/*pAd->CommonCfg.bAutoTxRateSwitch	= TRUE;*/
		*auto_rate_cur_p = TRUE;
	}

	if (HtMcs != MCS_AUTO)
	{
		/*OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED);*/
		/*pAd->CommonCfg.bAutoTxRateSwitch	= FALSE;*/
		*auto_rate_cur_p = FALSE;
	}
	else
	{
		/*OPSTATUS_SET_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED); */
		/*pAd->CommonCfg.bAutoTxRateSwitch	= TRUE;*/
		*auto_rate_cur_p = TRUE;
	}

#ifdef CONFIG_STA_SUPPORT
	if ((ADHOC_ON(pAd) || INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA)
#ifdef P2P_SUPPORT
		&& (apidx == MIN_NET_DEVICE_FOR_MBSSID)
#endif /* P2P_SUPPORT */
		)
	{
		pSupRate = &pAd->StaActive.SupRate[0];
		pExtRate = &pAd->StaActive.ExtRate[0];
		SupRateLen = pAd->StaActive.SupRateLen;
		ExtRateLen = pAd->StaActive.ExtRateLen;
	}
	else
#endif /* CONFIG_STA_SUPPORT */	
	{
		pSupRate = &pAd->CommonCfg.SupRate[0];
		pExtRate = &pAd->CommonCfg.ExtRate[0];
		SupRateLen = pAd->CommonCfg.SupRateLen;
		ExtRateLen = pAd->CommonCfg.ExtRateLen;
	}

	/* find max supported rate*/
	for (i=0; i<SupRateLen; i++)
	{
		switch (pSupRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0001;	 break;
			case 4:   Rate = RATE_2;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0002;	 break;
			case 11:  Rate = RATE_5_5;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0004;	 break;
			case 22:  Rate = RATE_11;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0008;	 break;
			case 12:  Rate = RATE_6;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 0x0010;  break;
			case 18:  Rate = RATE_9;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0020;	 break;
			case 24:  Rate = RATE_12;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 0x0040;  break;
			case 36:  Rate = RATE_18;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0080;	 break;
			case 48:  Rate = RATE_24;	/*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 0x0100;  break;
			case 72:  Rate = RATE_36;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0200;	 break;
			case 96:  Rate = RATE_48;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0400;	 break;
			case 108: Rate = RATE_54;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0800;	 break;
			default:  Rate = RATE_1;	break;
		}
		if (MaxSupport < Rate)	MaxSupport = Rate;

		if (MinSupport > Rate) MinSupport = Rate;		
	}
	
	for (i=0; i<ExtRateLen; i++)
	{
		switch (pExtRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0001;	 break;
			case 4:   Rate = RATE_2;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0002;	 break;
			case 11:  Rate = RATE_5_5;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0004;	 break;
			case 22:  Rate = RATE_11;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0008;	 break;
			case 12:  Rate = RATE_6;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 0x0010;  break;
			case 18:  Rate = RATE_9;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0020;	 break;
			case 24:  Rate = RATE_12;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 0x0040;  break;
			case 36:  Rate = RATE_18;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0080;	 break;
			case 48:  Rate = RATE_24;	/*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 0x0100;  break;
			case 72:  Rate = RATE_36;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0200;	 break;
			case 96:  Rate = RATE_48;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0400;	 break;
			case 108: Rate = RATE_54;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0800;	 break;
			default:  Rate = RATE_1;	break;
		}
		if (MaxSupport < Rate)	MaxSupport = Rate;

		if (MinSupport > Rate) MinSupport = Rate;		
	}

	RTMP_IO_WRITE32(pAd, LEGACY_BASIC_RATE, BasicRateBitmap);
	
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;
		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateTxRates[MaxSupport = %d] = MaxDesire %d Mbps\n", RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));
	/* max tx rate = min {max desire rate, max supported rate}*/
	if (MaxSupport < MaxDesire)
		pAd->CommonCfg.MaxTxRate = MaxSupport;
	else
		pAd->CommonCfg.MaxTxRate = MaxDesire;

	pAd->CommonCfg.MinTxRate = MinSupport;
	/*
		2003-07-31 john - 2500 doesn't have good sensitivity at high OFDM rates. to increase the success
		ratio of initial DHCP packet exchange, TX rate starts from a lower rate depending
		on average RSSI
			1. RSSI >= -70db, start at 54 Mbps (short distance)
			2. -70 > RSSI >= -75, start at 24 Mbps (mid distance)
			3. -75 > RSSI, start at 11 Mbps (long distance)
	*/
	if (*auto_rate_cur_p)
	{
		short dbm = 0;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			dbm = pAd->StaCfg.RssiSample.AvgRssi0 - pAd->BbpRssiToDbmDelta;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			dbm =0;
#endif /* CONFIG_AP_SUPPORT */
		if (bLinkUp == TRUE)
			pAd->CommonCfg.TxRate = RATE_24;
		else
			pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate; 

		if (dbm < -75)
			pAd->CommonCfg.TxRate = RATE_11;
		else if (dbm < -70)
			pAd->CommonCfg.TxRate = RATE_24;

		/* should never exceed MaxTxRate (consider 11B-only mode)*/
		if (pAd->CommonCfg.TxRate > pAd->CommonCfg.MaxTxRate)
			pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate; 

		pAd->CommonCfg.TxRateIndex = 0;

	}
	else
	{
		pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate;
		/*pHtPhy->field.MCS	= (pAd->CommonCfg.MaxTxRate > 3) ? (pAd->CommonCfg.MaxTxRate - 4) : pAd->CommonCfg.MaxTxRate;*/
		/*pHtPhy->field.MODE	= (pAd->CommonCfg.MaxTxRate > 3) ? MODE_OFDM : MODE_CCK;*/

		/* Choose the Desire Tx MCS in CCK/OFDM mode */
		if (num > RATE_6)
		{
			if (HtMcs <= MCS_7)		
				MaxDesire = RxwiMCSToOfdmRate[HtMcs];
			else
				MaxDesire = MinSupport;
		}
		else
		{
			if (HtMcs <= MCS_3)		
				MaxDesire = HtMcs;
			else
				MaxDesire = MinSupport;
		}
		
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.STBC	= pHtPhy->field.STBC;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.ShortGI	= pHtPhy->field.ShortGI;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MCS		= pHtPhy->field.MCS;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE	= pHtPhy->field.MODE;

	}

	if (pAd->CommonCfg.TxRate <= RATE_11)
	{
		pMaxHtPhy->field.MODE = MODE_CCK;

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
		pMaxHtPhy->field.MCS = pAd->CommonCfg.TxRate;
		pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = MaxDesire;
		}
#endif /* CONFIG_AP_SUPPORT */	
#ifdef P2P_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			pMaxHtPhy->field.MCS = MaxDesire;
#endif /* P2P_SUPPORT */	

	}
	else
	{
		pMaxHtPhy->field.MODE = MODE_OFDM;

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
		pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.TxRate];
		if (pAd->CommonCfg.MinTxRate >= RATE_6 && (pAd->CommonCfg.MinTxRate <= RATE_54))
			{pMinHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MinTxRate];}
		else
			{pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
		}
#endif /* CONFIG_AP_SUPPORT */				

#ifdef P2P_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[MaxDesire];
#endif /* P2P_SUPPORT */
	}

	pHtPhy->word = (pMaxHtPhy->word);
	if (bLinkUp && (pAd->OpMode == OPMODE_STA))
	{
			pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word = pHtPhy->word;
			pAd->MacTab.Content[BSSID_WCID].MaxHTPhyMode.word = pMaxHtPhy->word;
			pAd->MacTab.Content[BSSID_WCID].MinHTPhyMode.word = pMinHtPhy->word;
	}
	else
	{
		switch (pAd->CommonCfg.PhyMode) 
		{
			case PHY_11BG_MIXED:
			case PHY_11B:
#ifdef DOT11_N_SUPPORT
			case PHY_11BGN_MIXED:
#endif /* DOT11_N_SUPPORT */
				pAd->CommonCfg.MlmeRate = RATE_1;
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
				pAd->CommonCfg.MlmeTransmit.field.MCS = RATE_1;
				
/*#ifdef	WIFI_TEST			*/
				pAd->CommonCfg.RtsRate = RATE_11;
/*#else*/
/*				pAd->CommonCfg.RtsRate = RATE_1;*/
/*#endif*/
				break;
			case PHY_11G:
			case PHY_11A:
#ifdef DOT11_N_SUPPORT
			case PHY_11AGN_MIXED:
			case PHY_11GN_MIXED:
			case PHY_11N_2_4G:
			case PHY_11AN_MIXED:
			case PHY_11N_5G:	
#endif /* DOT11_N_SUPPORT */
				pAd->CommonCfg.MlmeRate = RATE_6;
				pAd->CommonCfg.RtsRate = RATE_6;
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
				pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
				break;
			case PHY_11ABG_MIXED:
#ifdef DOT11_N_SUPPORT
			case PHY_11ABGN_MIXED:
#endif /* DOT11_N_SUPPORT */
				if (pAd->CommonCfg.Channel <= 14)
				{
					pAd->CommonCfg.MlmeRate = RATE_1;
					pAd->CommonCfg.RtsRate = RATE_1;
					pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
					pAd->CommonCfg.MlmeTransmit.field.MCS = RATE_1;
				}
				else
				{
					pAd->CommonCfg.MlmeRate = RATE_6;
					pAd->CommonCfg.RtsRate = RATE_6;
					pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
					pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
				}
				break;
			default: /* error*/
				pAd->CommonCfg.MlmeRate = RATE_6;
                        	pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
				pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
				pAd->CommonCfg.RtsRate = RATE_1;
				break;
		}
		
		/* Keep Basic Mlme Rate.*/
		pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word = pAd->CommonCfg.MlmeTransmit.word;
		if (pAd->CommonCfg.MlmeTransmit.field.MODE == MODE_OFDM)
			/* MTK patch fix dhcp issue on new Apple and others buggy clients (use RATE_6 instead of 24) */
			pAd->MacTab.Content[MCAST_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_6];
		else
			pAd->MacTab.Content[MCAST_WCID].HTPhyMode.field.MCS = RATE_1;
		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;

#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
		{
			/* set default value if MCastPhyMode is not initialized */	
			HTTRANSMIT_SETTING tPhyMode;

			memset(&tPhyMode, 0, sizeof(HTTRANSMIT_SETTING));
			if (memcmp(&pAd->CommonCfg.MCastPhyMode, &tPhyMode, sizeof(HTTRANSMIT_SETTING)) == 0)
			{
				memmove(&pAd->CommonCfg.MCastPhyMode, &pAd->MacTab.Content[MCAST_WCID].HTPhyMode,
							sizeof(HTTRANSMIT_SETTING));
			}
		}
#endif /* MCAST_RATE_SPECIFIC */
#endif /* CONFIG_AP_SUPPORT */
	}

	DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n", 
			 RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport], RateIdToMbps[pAd->CommonCfg.MaxTxRate], RateIdToMbps[pAd->CommonCfg.MinTxRate], 
			 /*OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)*/*auto_rate_cur_p));
	DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n", 
			 RateIdToMbps[pAd->CommonCfg.TxRate], RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));
	DBGPRINT(RT_DEBUG_TRACE, ("MlmeUpdateTxRates (MlmeTransmit=0x%x, MinHTPhyMode=%x, MaxHTPhyMode=0x%x, HTPhyMode=0x%x)\n", 
			 pAd->CommonCfg.MlmeTransmit.word, pAd->MacTab.Content[BSSID_WCID].MinHTPhyMode.word ,pAd->MacTab.Content[BSSID_WCID].MaxHTPhyMode.word ,pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word ));
}

#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		This function update HT Rate setting.
		Input Wcid value is valid for 2 case :
		1. it's used for Station in infra mode that copy AP rate to Mactable.
		2. OR Station 	in adhoc mode to copy peer's HT rate to Mactable. 

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MlmeUpdateHtTxRates(
	IN PRTMP_ADAPTER 		pAd,
	IN	UCHAR				apidx)
{
	UCHAR	StbcMcs; /*j, StbcMcs, bitmask;*/
	CHAR 	i; /* 3*3*/
	RT_HT_CAPABILITY 	*pRtHtCap = NULL;
	RT_HT_PHY_INFO		*pActiveHtPhy = NULL;	
	ULONG		BasicMCS;
	UCHAR j, bitmask;
	PRT_HT_PHY_INFO			pDesireHtPhy = NULL;
	PHTTRANSMIT_SETTING		pHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMinHtPhy = NULL;	
	BOOLEAN 				*auto_rate_cur_p;
	
	DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateHtTxRates===> \n"));

	auto_rate_cur_p = NULL;

	do
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_P2P_GO)
		{		
			UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_P2P_GO;

			pDesireHtPhy	= &pAd->ApCfg.MBSSID[idx].DesiredHtPhyInfo;
			pActiveHtPhy	= &pAd->ApCfg.MBSSID[idx].DesiredHtPhyInfo;
			pHtPhy 			= &pAd->ApCfg.MBSSID[idx].HTPhyMode;	
			pMaxHtPhy		= &pAd->ApCfg.MBSSID[idx].MaxHTPhyMode;
			pMinHtPhy		= &pAd->ApCfg.MBSSID[idx].MinHTPhyMode;

			auto_rate_cur_p = &pAd->ApCfg.MBSSID[idx].bAutoTxRateSwitch;									
			break;
		}
#endif /* P2P_SUPPORT */

#ifdef APCLI_SUPPORT	
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
		{
			UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;
		
			if (idx < MAX_APCLI_NUM)
			{
			pDesireHtPhy	= &pAd->ApCfg.ApCliTab[idx].DesiredHtPhyInfo;
			pActiveHtPhy	= &pAd->ApCfg.ApCliTab[idx].DesiredHtPhyInfo;
			pHtPhy 			= &pAd->ApCfg.ApCliTab[idx].HTPhyMode;	
			pMaxHtPhy		= &pAd->ApCfg.ApCliTab[idx].MaxHTPhyMode;
			pMinHtPhy		= &pAd->ApCfg.ApCliTab[idx].MinHTPhyMode;

			auto_rate_cur_p = &pAd->ApCfg.ApCliTab[idx].bAutoTxRateSwitch;
			break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MlmeUpdateHtTxRates: invalid idx(%d)\n", idx));			
				return;
			}
		}
#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;

			if (idx < MAX_WDS_ENTRY)
			{
				pDesireHtPhy	= &pAd->WdsTab.WdsEntry[idx].DesiredHtPhyInfo;
				pActiveHtPhy	= &pAd->WdsTab.WdsEntry[idx].DesiredHtPhyInfo;
				pHtPhy 			= &pAd->WdsTab.WdsEntry[idx].HTPhyMode;	
				pMaxHtPhy		= &pAd->WdsTab.WdsEntry[idx].MaxHTPhyMode;
				pMinHtPhy		= &pAd->WdsTab.WdsEntry[idx].MinHTPhyMode;

				auto_rate_cur_p = &pAd->WdsTab.WdsEntry[idx].bAutoTxRateSwitch;
				break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MlmeUpdateHtTxRates: invalid apidx(%d)\n", apidx));			
				return;
			}
			}
#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) && (apidx < HW_BEACON_MAX_NUM))
			{		
				pDesireHtPhy	= &pAd->ApCfg.MBSSID[apidx].DesiredHtPhyInfo;
				pActiveHtPhy	= &pAd->ApCfg.MBSSID[apidx].DesiredHtPhyInfo;
				pHtPhy 			= &pAd->ApCfg.MBSSID[apidx].HTPhyMode;	
				pMaxHtPhy		= &pAd->ApCfg.MBSSID[apidx].MaxHTPhyMode;
				pMinHtPhy		= &pAd->ApCfg.MBSSID[apidx].MinHTPhyMode;

				auto_rate_cur_p = &pAd->ApCfg.MBSSID[apidx].bAutoTxRateSwitch;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MlmeUpdateHtTxRates: invalid apidx(%d)\n", apidx));			
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{		
			pDesireHtPhy	= &pAd->StaCfg.DesiredHtPhyInfo;
			pActiveHtPhy	= &pAd->StaCfg.DesiredHtPhyInfo;
			pHtPhy 		= &pAd->StaCfg.HTPhyMode;
			pMaxHtPhy	= &pAd->StaCfg.MaxHTPhyMode;
			pMinHtPhy	= &pAd->StaCfg.MinHTPhyMode;		

			auto_rate_cur_p = &pAd->StaCfg.bAutoTxRateSwitch;
			break;
		}		
#endif /* CONFIG_STA_SUPPORT */
	} while (FALSE);


#ifdef CONFIG_STA_SUPPORT	
	if ((ADHOC_ON(pAd) || INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA)
#ifdef P2P_SUPPORT
		&& (apidx == BSS0)
#endif /* P2P_SUPPORT */
		)
	{
		if (pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE)
			return;

		pRtHtCap = &pAd->StaActive.SupportedHtPhy;
		pActiveHtPhy = &pAd->StaActive.SupportedPhyInfo;
		StbcMcs = (UCHAR)pAd->MlmeAux.AddHtInfo.AddHtInfo3.StbcMcs;
		BasicMCS =pAd->MlmeAux.AddHtInfo.MCSSet[0]+(pAd->MlmeAux.AddHtInfo.MCSSet[1]<<8)+(StbcMcs<<16);
		if ((pAd->CommonCfg.DesiredHtPhy.TxSTBC) && (pRtHtCap->RxSTBC) && (pAd->Antenna.field.TxPath == 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}
	else
#endif /* CONFIG_STA_SUPPORT */
	{
		if ((!pDesireHtPhy) || pDesireHtPhy->bHtEnable == FALSE)
			return;

		pRtHtCap = &pAd->CommonCfg.DesiredHtPhy;
		StbcMcs = (UCHAR)pAd->CommonCfg.AddHTInfo.AddHtInfo3.StbcMcs;
		BasicMCS = pAd->CommonCfg.AddHTInfo.MCSSet[0]+(pAd->CommonCfg.AddHTInfo.MCSSet[1]<<8)+(StbcMcs<<16);
		if ((pAd->CommonCfg.DesiredHtPhy.TxSTBC) && (pAd->Antenna.field.TxPath >= 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}

	/* Decide MAX ht rate.*/
	if ((pRtHtCap->GF) && (pAd->CommonCfg.DesiredHtPhy.GF))
		pMaxHtPhy->field.MODE = MODE_HTGREENFIELD;
	else
		pMaxHtPhy->field.MODE = MODE_HTMIX;

    if ((pAd->CommonCfg.DesiredHtPhy.ChannelWidth) && (pRtHtCap->ChannelWidth))
		pMaxHtPhy->field.BW = BW_40;
	else
		pMaxHtPhy->field.BW = BW_20;

    if (pMaxHtPhy->field.BW == BW_20)
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 & pRtHtCap->ShortGIfor20);
	else
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 & pRtHtCap->ShortGIfor40);

	if (pDesireHtPhy->MCSSet[4] != 0)
	{
		pMaxHtPhy->field.MCS = 32;	
	}

	for (i=23; i>=0; i--) /* 3*3*/
	{ 
		j = i/8; 
		bitmask = (1<<(i-(j*8)));

		if ((pActiveHtPhy->MCSSet[j] & bitmask) && (pDesireHtPhy->MCSSet[j] & bitmask))
		{
			pMaxHtPhy->field.MCS = i;
			break;
		}

		if (i==0)
			break;
	}

	/* Copy MIN ht rate.  rt2860???*/
	pMinHtPhy->field.BW = BW_20;
	pMinHtPhy->field.MCS = 0;
	pMinHtPhy->field.STBC = 0;
	pMinHtPhy->field.ShortGI = 0;
	/*If STA assigns fixed rate. update to fixed here.*/
#ifdef CONFIG_STA_SUPPORT
	if ( (pAd->OpMode == OPMODE_STA) && (pDesireHtPhy->MCSSet[0] != 0xff)
#ifdef P2P_SUPPORT
		&& (apidx == BSS0)
#endif /* P2P_SUPPORT */
		)
	{
		if (pDesireHtPhy->MCSSet[4] != 0)
		{
			pMaxHtPhy->field.MCS = 32;
			pMinHtPhy->field.MCS = 32;
			DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateHtTxRates<=== Use Fixed MCS = %d\n",pMinHtPhy->field.MCS));
		}
		
		for (i=23; (CHAR)i >= 0; i--) /* 3*3*/
		{	
			j = i/8;	
			bitmask = (1<<(i-(j*8)));
			if ( (pDesireHtPhy->MCSSet[j] & bitmask) && (pActiveHtPhy->MCSSet[j] & bitmask))
			{
				pMaxHtPhy->field.MCS = i;
				pMinHtPhy->field.MCS = i;
				break;
			}
			if (i==0)
				break;
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	
	
	/* Decide ht rate*/
	pHtPhy->field.STBC = pMaxHtPhy->field.STBC;
	pHtPhy->field.BW = pMaxHtPhy->field.BW;
	pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
	pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
	pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;

	/* use default now. rt2860*/
	if (pDesireHtPhy->MCSSet[0] != 0xff)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;
	
	DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateHtTxRates<---.AMsduSize = %d  \n", pAd->CommonCfg.DesiredHtPhy.AmsduSize ));
	DBGPRINT(RT_DEBUG_TRACE,("TX: MCS[0] = %x (choose %d), BW = %d, ShortGI = %d, MODE = %d,  \n", pActiveHtPhy->MCSSet[0],pHtPhy->field.MCS,
		pHtPhy->field.BW, pHtPhy->field.ShortGI, pHtPhy->field.MODE));
	DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateHtTxRates<=== \n"));
}


VOID BATableInit(
	IN PRTMP_ADAPTER pAd, 
    IN BA_TABLE *Tab) 
{
	int i;

	Tab->numAsOriginator = 0;
	Tab->numAsRecipient = 0;
	Tab->numDoneOriginator = 0;
	NdisAllocateSpinLock(pAd, &pAd->BATabLock);
	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) 
	{
		Tab->BARecEntry[i].REC_BA_Status = Recipient_NONE;
		NdisAllocateSpinLock(pAd, &(Tab->BARecEntry[i].RxReRingLock));
	}
	for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++) 
	{
		Tab->BAOriEntry[i].ORI_BA_Status = Originator_NONE;
	}
}

VOID BATableExit(
	IN RTMP_ADAPTER *pAd)
{
	int i;
	
	for(i=0; i<MAX_LEN_OF_BA_REC_TABLE; i++)
	{
		NdisFreeSpinLock(&pAd->BATable.BARecEntry[i].RxReRingLock);
	}
	NdisFreeSpinLock(&pAd->BATabLock);
}
#endif /* DOT11_N_SUPPORT */

/* IRQL = DISPATCH_LEVEL*/
VOID MlmeRadioOff(
	IN PRTMP_ADAPTER pAd)
{
	RTMP_MLME_RADIO_OFF(pAd);
}

/* IRQL = DISPATCH_LEVEL*/
VOID MlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{	
	RTMP_MLME_RADIO_ON(pAd);
}

/*
===========================================================================================
	bss_table.c
===========================================================================================
*/


/*! \brief initialize BSS table
 *	\param p_tab pointer to the table
 *	\return none
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
VOID BssTableInit(
	IN BSS_TABLE *Tab) 
{
	int i;

	Tab->BssNr = 0;
    Tab->BssOverlapNr = 0;
	
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
	{
		UCHAR *pOldAddr = Tab->BssEntry[i].pVarIeFromProbRsp;

		NdisZeroMemory(&Tab->BssEntry[i], sizeof(BSS_ENTRY));

		Tab->BssEntry[i].Rssi = -127;	/* initial the rssi as a minimum value */
		if (pOldAddr)
		{
			RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
			Tab->BssEntry[i].pVarIeFromProbRsp = pOldAddr;
		}
	}
}


/*! \brief search the BSS table by SSID
 *	\param p_tab pointer to the bss table
 *	\param ssid SSID string 
 *	\return index of the table, BSS_NOT_FOUND if not in the table
 *	\pre
 *	\post
 *	\note search by sequential search

 IRQL = DISPATCH_LEVEL

 */
ULONG BssTableSearch(
	IN BSS_TABLE *Tab, 
	IN PUCHAR	 pBssid,
	IN UCHAR	 Channel) 
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		
		/*
			Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
			We should distinguish this case.
		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)) 
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}

ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab, 
	IN PUCHAR	 pBssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel) 
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		
		/* Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.*/
		/* We should distinguish this case.*/
		/*		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid) &&
			SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen)) 
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab, 
	IN PUCHAR	 Bssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen) ||
			(NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) || 
			(NdisEqualMemory(Tab->BssEntry[i].Ssid, ZeroSsid, Tab->BssEntry[i].SsidLen))))
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearchBySSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		if (SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen)) 
		{ 
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


/* IRQL = DISPATCH_LEVEL*/
VOID BssTableDeleteEntry(
	IN OUT	BSS_TABLE *Tab, 
	IN		PUCHAR	  pBssid,
	IN		UCHAR	  Channel)
{
	UCHAR i, j;

	for (i = 0; i < Tab->BssNr; i++) 
	{
		if ((Tab->BssEntry[i].Channel == Channel) && 
			(MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)))
		{
			UCHAR *pOldAddr = NULL;
			
			for (j = i; j < Tab->BssNr - 1; j++)
			{
				pOldAddr = Tab->BssEntry[j].pVarIeFromProbRsp;
				NdisMoveMemory(&(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]), sizeof(BSS_ENTRY));
				if (pOldAddr)
				{
					RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
					NdisMoveMemory(pOldAddr, 
								   Tab->BssEntry[j + 1].pVarIeFromProbRsp, 
								   Tab->BssEntry[j + 1].VarIeFromProbeRspLen);
					Tab->BssEntry[j].pVarIeFromProbRsp = pOldAddr;
				}
			}

			pOldAddr = Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp;
			NdisZeroMemory(&(Tab->BssEntry[Tab->BssNr - 1]), sizeof(BSS_ENTRY));
			if (pOldAddr)
			{
				RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
				Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp = pOldAddr;
			}
			
			Tab->BssNr -= 1;
			return;
		}
	}
}


/*! \brief
 *	\param 
 *	\return
 *	\pre
 *	\post
	 
 IRQL = DISPATCH_LEVEL
 
 */
VOID BssEntrySet(
	IN PRTMP_ADAPTER	pAd, 
	OUT BSS_ENTRY *pBss, 
	IN PUCHAR pBssid, 
	IN CHAR Ssid[], 
	IN UCHAR SsidLen, 
	IN UCHAR BssType, 
	IN USHORT BeaconPeriod, 
	IN PCF_PARM pCfParm, 
	IN USHORT AtimWin, 
	IN USHORT CapabilityInfo, 
	IN UCHAR SupRate[], 
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[], 
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR Channel,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,	
	IN PNDIS_802_11_VARIABLE_IEs pVIE) 
{
	COPY_MAC_ADDR(pBss->Bssid, pBssid);
	/* Default Hidden SSID to be TRUE, it will be turned to FALSE after coping SSID*/
	pBss->Hidden = 1;	
	if (SsidLen > 0)
	{
		/* For hidden SSID AP, it might send beacon with SSID len equal to 0*/
		/* Or send beacon /probe response with SSID len matching real SSID length,*/
		/* but SSID is all zero. such as "00-00-00-00" with length 4.*/
		/* We have to prevent this case overwrite correct table*/
		if (NdisEqualMemory(Ssid, ZeroSsid, SsidLen) == 0)
		{
		    NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pBss->Ssid, Ssid, SsidLen);
			pBss->SsidLen = SsidLen;
			pBss->Hidden = 0;
		}
	}
	else
	{
		/* avoid  Hidden SSID form beacon to overwirite correct SSID from probe response */
		if (NdisEqualMemory(pBss->Ssid, ZeroSsid, pBss->SsidLen))
		{
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			pBss->SsidLen = 0;
		}
	}
	
	pBss->BssType = BssType;
	pBss->BeaconPeriod = BeaconPeriod;
	if (BssType == BSS_INFRA) 
	{
		if (pCfParm->bValid) 
		{
			pBss->CfpCount = pCfParm->CfpCount;
			pBss->CfpPeriod = pCfParm->CfpPeriod;
			pBss->CfpMaxDuration = pCfParm->CfpMaxDuration;
			pBss->CfpDurRemaining = pCfParm->CfpDurRemaining;
		}
	} 
	else 
	{
		pBss->AtimWin = AtimWin;
	}

	NdisGetSystemUpTime(&pBss->LastBeaconRxTime);
	pBss->CapabilityInfo = CapabilityInfo;
	/* The privacy bit indicate security is ON, it maight be WEP, TKIP or AES*/
	/* Combine with AuthMode, they will decide the connection methods.*/
	pBss->Privacy = CAP_IS_PRIVACY_ON(pBss->CapabilityInfo);
	ASSERT(SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	if (SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)		
		NdisMoveMemory(pBss->SupRate, SupRate, SupRateLen);
	else		
		NdisMoveMemory(pBss->SupRate, SupRate, MAX_LEN_OF_SUPPORTED_RATES);	
	pBss->SupRateLen = SupRateLen;
	ASSERT(ExtRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	if (ExtRateLen > MAX_LEN_OF_SUPPORTED_RATES)
		ExtRateLen = MAX_LEN_OF_SUPPORTED_RATES;
	NdisMoveMemory(pBss->ExtRate, ExtRate, ExtRateLen);
	pBss->NewExtChanOffset = NewExtChanOffset;
	pBss->ExtRateLen = ExtRateLen;
	pBss->Channel = Channel;
	pBss->CentralChannel = Channel;
	pBss->Rssi = Rssi;
	/* Update CkipFlag. if not exists, the value is 0x0*/
	pBss->CkipFlag = CkipFlag;

	/* New for microsoft Fixed IEs*/
	NdisMoveMemory(pBss->FixIEs.Timestamp, &TimeStamp, 8);
	pBss->FixIEs.BeaconInterval = BeaconPeriod;
	pBss->FixIEs.Capabilities = CapabilityInfo;

	/* New for microsoft Variable IEs*/
	if (LengthVIE != 0)
	{
		pBss->VarIELen = LengthVIE;
		NdisMoveMemory(pBss->VarIEs, pVIE, pBss->VarIELen);
	}
	else
	{
		pBss->VarIELen = 0;
	}

	pBss->AddHtInfoLen = 0;
	pBss->HtCapabilityLen = 0;
#ifdef DOT11_N_SUPPORT
	if (HtCapabilityLen> 0)
	{
		pBss->HtCapabilityLen = HtCapabilityLen;
		NdisMoveMemory(&pBss->HtCapability, pHtCapability, HtCapabilityLen);
		if (AddHtInfoLen > 0)
		{
			pBss->AddHtInfoLen = AddHtInfoLen;
			NdisMoveMemory(&pBss->AddHtInfo, pAddHtInfo, AddHtInfoLen);
			
	 			if ((pAddHtInfo->ControlChan > 2)&& (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
	 			{
	 				pBss->CentralChannel = pAddHtInfo->ControlChan - 2;
	 			}
	 			else if ((pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
				{
		 				pBss->CentralChannel = pAddHtInfo->ControlChan + 2;
				}
		}
	}
#endif /* DOT11_N_SUPPORT */
	
	BssCipherParse(pBss);

	/* new for QOS*/
	if (pEdcaParm)
		NdisMoveMemory(&pBss->EdcaParm, pEdcaParm, sizeof(EDCA_PARM));
	else
		pBss->EdcaParm.bValid = FALSE;
	if (pQosCapability)
		NdisMoveMemory(&pBss->QosCapability, pQosCapability, sizeof(QOS_CAPABILITY_PARM));
	else
		pBss->QosCapability.bValid = FALSE;
	if (pQbssLoad)
		NdisMoveMemory(&pBss->QbssLoad, pQbssLoad, sizeof(QBSS_LOAD_PARM));
	else
		pBss->QbssLoad.bValid = FALSE;

	{
		PEID_STRUCT     pEid;
		USHORT          Length = 0;

#ifdef WSC_INCLUDED
		pBss->WpsAP = 0x00;
		pBss->WscDPIDFromWpsAP = 0xFFFF;
#endif /* WSC_INCLUDED */

#ifdef CONFIG_STA_SUPPORT
		NdisZeroMemory(&pBss->WpaIE.IE[0], MAX_CUSTOM_LEN);
		NdisZeroMemory(&pBss->RsnIE.IE[0], MAX_CUSTOM_LEN);
		NdisZeroMemory(&pBss->WpsIE.IE[0], MAX_CUSTOM_LEN);
		pBss->WpaIE.IELen = 0;
		pBss->RsnIE.IELen = 0;
		pBss->WpsIE.IELen = 0;
#ifdef WAPI_SUPPORT
		NdisZeroMemory(&pBss->WapiIE.IE[0], MAX_CUSTOM_LEN);
		pBss->WapiIE.IELen = 0;
#endif /* WAPI_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST
		NdisZeroMemory(&pBss->CountryString[0], 3);
		pBss->bHasCountryIE = FALSE;
#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* CONFIG_STA_SUPPORT */
		pEid = (PEID_STRUCT) pVIE;
		while ((Length + 2 + (USHORT)pEid->Len) <= LengthVIE)    
		{
#define WPS_AP		0x01
			switch(pEid->Eid)
			{
				case IE_WPA:
					if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
#ifdef IWSC_SUPPORT
						|| NdisEqualMemory(pEid->Octet, IWSC_OUI, 4)
#endif /* IWSC_SUPPORT */
						)
					{
#ifdef WSC_INCLUDED
						pBss->WpsAP |= WPS_AP;
						WscCheckWpsIeFromWpsAP(pAd, 
													pEid, 
													&pBss->WscDPIDFromWpsAP);
#endif /* WSC_INCLUDED */
#ifdef CONFIG_STA_SUPPORT
						if ((pEid->Len + 2) > MAX_CUSTOM_LEN)
						{
							pBss->WpsIE.IELen = 0;
							break;
						}
						pBss->WpsIE.IELen = pEid->Len + 2;
						NdisMoveMemory(pBss->WpsIE.IE, pEid, pBss->WpsIE.IELen);
#endif /* CONFIG_STA_SUPPORT */
						break;
					}
#ifdef CONFIG_STA_SUPPORT
					if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					{
						if ((pEid->Len + 2) > MAX_CUSTOM_LEN)
						{
							pBss->WpaIE.IELen = 0;
							break;
						}
						pBss->WpaIE.IELen = pEid->Len + 2;
						NdisMoveMemory(pBss->WpaIE.IE, pEid, pBss->WpaIE.IELen);
					}
#endif /* CONFIG_STA_SUPPORT */
					break;

#ifdef CONFIG_STA_SUPPORT
				case IE_RSN:
					if (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
					{
						if ((pEid->Len + 2) > MAX_CUSTOM_LEN)
						{
							pBss->RsnIE.IELen = 0;
							break;
						}
						pBss->RsnIE.IELen = pEid->Len + 2;
						NdisMoveMemory(pBss->RsnIE.IE, pEid, pBss->RsnIE.IELen);
					}
					break;
#ifdef WAPI_SUPPORT
				case IE_WAPI:
					if (NdisEqualMemory(pEid->Octet + 4, WAPI_OUI, 3))
					{
						UCHAR           idx;
						snprintf((PSTRING) pBss->WapiIE.IE, sizeof(pBss->WapiIE.IE), "wapi_ie=%02x%02x", pEid->Eid, pEid->Len);
						for (idx = 0; idx < pEid->Len; idx++)
						{
							snprintf((PSTRING) pBss->WapiIE.IE, sizeof(pBss->WapiIE.IE), "%s%02x", pBss->WapiIE.IE, (unsigned char)pEid->Octet[idx]);
						}
						pBss->WapiIE.IELen = (pEid->Len*2) + 8; /* 2: ID(1 byte), LEN (1 byte), 8: len of "wapi_ie="*/
					}
					break;
#endif /* WAPI_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST					
				case IE_COUNTRY:					
					NdisMoveMemory(&pBss->CountryString[0], pEid->Octet, 3);
					pBss->bHasCountryIE = TRUE;
					break;
#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* CONFIG_STA_SUPPORT */
			}
			Length = Length + 2 + (USHORT)pEid->Len;  /* Eid[1] + Len[1]+ content[Len]*/
			pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
		}
	}
}

/*! 
 *	\brief insert an entry into the bss table
 *	\param p_tab The BSS table
 *	\param Bssid BSSID
 *	\param ssid SSID
 *	\param ssid_len Length of SSID
 *	\param bss_type
 *	\param beacon_period
 *	\param timestamp
 *	\param p_cf
 *	\param atim_win
 *	\param cap
 *	\param rates
 *	\param rates_len
 *	\param channel_idx
 *	\return none
 *	\pre
 *	\post
 *	\note If SSID is identical, the old entry will be replaced by the new one
	 
 IRQL = DISPATCH_LEVEL
 
 */
ULONG BssTableSetEntry(
	IN	PRTMP_ADAPTER	pAd, 
	OUT BSS_TABLE *Tab, 
	IN PUCHAR pBssid, 
	IN CHAR Ssid[], 
	IN UCHAR SsidLen, 
	IN UCHAR BssType, 
	IN USHORT BeaconPeriod, 
	IN CF_PARM *CfParm, 
	IN USHORT AtimWin, 
	IN USHORT CapabilityInfo, 
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR ChannelNo,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,	
	IN PNDIS_802_11_VARIABLE_IEs pVIE)
{
	ULONG	Idx;
#ifdef APCLI_SUPPORT
	BOOLEAN bInsert = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	UCHAR i;
#endif /* APCLI_SUPPORT */

	/*Idx = BssTableSearchWithSSID(Tab, pBssid,  (UCHAR *)Ssid, SsidLen, ChannelNo);*/
	Idx = BssTableSearch(Tab, pBssid, ChannelNo);
	if (Idx == BSS_NOT_FOUND) 
	{
		if (Tab->BssNr >= MAX_LEN_OF_BSS_TABLE)
	    {
			
			/* It may happen when BSS Table was full.*/
			/* The desired AP will not be added into BSS Table*/
			/* In this case, if we found the desired AP then overwrite BSS Table.*/
#ifdef APCLI_SUPPORT
			for (i = 0; i < MAX_APCLI_NUM; i++)
			{
				pApCliEntry = &pAd->ApCfg.ApCliTab[i];
				if (MAC_ADDR_EQUAL(pApCliEntry->ApCliMlmeAux.Bssid, pBssid)
					|| SSID_EQUAL(pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->ApCliMlmeAux.SsidLen, Ssid, SsidLen))
				{
					bInsert = TRUE;
					break;
				}
			}
#endif /* APCLI_SUPPORT */



			if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) ||
				!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
			{
				if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, pBssid) ||
					SSID_EQUAL(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Ssid, SsidLen)
#ifdef APCLI_SUPPORT
					|| bInsert
#endif /* APCLI_SUPPORT */
					)
				{
					Idx = Tab->BssOverlapNr;
					NdisZeroMemory(&(Tab->BssEntry[Idx]), sizeof(BSS_ENTRY));
					BssEntrySet(pAd, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod, CfParm, AtimWin, 
						CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pAddHtInfo,HtCapabilityLen, AddHtInfoLen,
						NewExtChanOffset, ChannelNo, Rssi, TimeStamp, CkipFlag, pEdcaParm, pQosCapability, pQbssLoad, LengthVIE, pVIE);
					Tab->BssOverlapNr = Tab->BssOverlapNr + 1;
                    Tab->BssOverlapNr = Tab->BssOverlapNr % MAX_LEN_OF_BSS_TABLE;
				}
				return Idx;
			}
			else
			{
			return BSS_NOT_FOUND;
			}
		}
		Idx = Tab->BssNr;
		BssEntrySet(pAd, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod, CfParm, AtimWin, 
					CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pAddHtInfo,HtCapabilityLen, AddHtInfoLen,
					NewExtChanOffset, ChannelNo, Rssi, TimeStamp, CkipFlag, pEdcaParm, pQosCapability, pQbssLoad, LengthVIE, pVIE);
		Tab->BssNr++;
	} 
	else
	{
		BssEntrySet(pAd, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod,CfParm, AtimWin, 
					CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pAddHtInfo,HtCapabilityLen, AddHtInfoLen,
					NewExtChanOffset, ChannelNo, Rssi, TimeStamp, CkipFlag, pEdcaParm, pQosCapability, pQbssLoad, LengthVIE, pVIE);
	}

	return Idx;
}

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID  TriEventInit(
	IN	PRTMP_ADAPTER	pAd) 
{
	UCHAR		i;

	for (i = 0;i < MAX_TRIGGER_EVENT;i++)
		pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;
	
	pAd->CommonCfg.TriggerEventTab.EventANo = 0;
	pAd->CommonCfg.TriggerEventTab.EventBCountDown = 0;
}

INT TriEventTableSetEntry(
	IN	PRTMP_ADAPTER	pAd, 
	OUT TRIGGER_EVENT_TAB *Tab, 
	IN PUCHAR pBssid, 
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			RegClass,
	IN UCHAR ChannelNo)
{
	/* Event A, legacy AP exist.*/
	if (HtCapabilityLen == 0)
	{
		UCHAR index;
		
		/*
			Check if we already set this entry in the Event Table.
		*/
		for (index = 0; index<MAX_TRIGGER_EVENT; index++)
		{
			if ((Tab->EventA[index].bValid == TRUE) && 
				(Tab->EventA[index].Channel == ChannelNo) && 
				(Tab->EventA[index].RegClass == RegClass)
			)
			{
				return 0;
			}
		}
		
		/*
			If not set, add it to the Event table
		*/
		if (Tab->EventANo < MAX_TRIGGER_EVENT)
		{
			RTMPMoveMemory(Tab->EventA[Tab->EventANo].BSSID, pBssid, 6);
			Tab->EventA[Tab->EventANo].bValid = TRUE;
			Tab->EventA[Tab->EventANo].Channel = ChannelNo;
			if (RegClass != 0)
			{
				/* Beacon has Regulatory class IE. So use beacon's*/
				Tab->EventA[Tab->EventANo].RegClass = RegClass;
			}
			else
			{
				/* Use Station's Regulatory class instead.*/
				/* If no Reg Class in Beacon, set to "unknown"*/
				/* TODO:  Need to check if this's valid*/
				Tab->EventA[Tab->EventANo].RegClass = 0; /* ????????????????? need to check*/
			}
			Tab->EventANo ++;
		}
	}
	else if (pHtCapability->HtCapInfo.Forty_Mhz_Intolerant)
	{
		/* Event B.   My BSS beacon has Intolerant40 bit set*/
		Tab->EventBCountDown = pAd->CommonCfg.Dot11BssWidthChanTranDelay;
	}

	return 0;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */

#ifdef CONFIG_STA_SUPPORT
/* IRQL = DISPATCH_LEVEL*/
VOID BssTableSsidSort(
	IN	PRTMP_ADAPTER	pAd, 
	OUT BSS_TABLE *OutTab, 
	IN	CHAR Ssid[], 
	IN	UCHAR SsidLen) 
{
	INT i;
#ifdef WSC_STA_SUPPORT
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
#endif /* WSC_STA_SUPPORT */
	BssTableInit(OutTab);

	if ((SsidLen == 0) && 
		(pAd->StaCfg.bAutoConnectIfNoSSID == FALSE))
		return;

	for (i = 0; i < pAd->ScanTab.BssNr; i++) 
	{
		BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];
		BOOLEAN	bIsHiddenApIncluded = FALSE;
		
		if ( ((pAd->CommonCfg.bIEEE80211H == 1) && 
				(pAd->MlmeAux.Channel > 14) && 
				 RadarChannelCheck(pAd, pInBss->Channel))
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier             */
             || (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
            )
		{
			if (pInBss->Hidden)
				bIsHiddenApIncluded = TRUE;
		}            


		if ((pInBss->BssType == pAd->StaCfg.BssType) && 
			(SSID_EQUAL(Ssid, SsidLen, pInBss->Ssid, pInBss->SsidLen) || bIsHiddenApIncluded))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

#ifdef WPA_SUPPLICANT_SUPPORT
			if (pAd->StaCfg.WpaSupplicantUP & 0x80)
			{
				/* copy matching BSS from InTab to OutTab*/
				NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
				OutTab->BssNr++;
				continue;
			}
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_STA_SUPPORT
			if ((pWpsCtrl->WscConfMode != WSC_DISABLE) && pWpsCtrl->bWscTrigger)
			{
				/* copy matching BSS from InTab to OutTab*/
				NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
				OutTab->BssNr++;
				continue;
			}
#endif /* WSC_STA_SUPPORT */

#ifdef EXT_BUILD_CHANNEL_LIST
			/* If no Country IE exists no Connection will be established when IEEE80211dClientMode is strict.*/
			if ((pAd->StaCfg.IEEE80211dClientMode == Rt802_11_D_Strict) &&
				(pInBss->bHasCountryIE == FALSE))
			{
				DBGPRINT(RT_DEBUG_TRACE,("StaCfg.IEEE80211dClientMode == Rt802_11_D_Strict, but this AP doesn't have country IE.\n"));
				continue;
			}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DOT11_N_SUPPORT
			/* 2.4G/5G N only mode*/
			if ((pInBss->HtCapabilityLen == 0) &&
				((pAd->CommonCfg.PhyMode == PHY_11N_2_4G) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			if ((pAd->CommonCfg.PhyMode == PHY_11GN_MIXED) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in GN-only Mode, this AP is in B mode.\n"));
				continue;
			}
#endif /* DOT11_N_SUPPORT */



			/* New for WPA2*/
			/* Check the Authmode first*/
			if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
			{
				/* Check AuthMode and AuthModeAux for matching, in case AP support dual-mode*/
				if ((pAd->StaCfg.AuthMode != pInBss->AuthMode) && (pAd->StaCfg.AuthMode != pInBss->AuthModeAux))
					/* None matched*/
					continue;
				
				/* Check cipher suite, AP must have more secured cipher than station setting*/
				if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA.GroupCipher)
							continue;
						
					/* check group cipher*/
					if ((pAd->StaCfg.WepStatus < pInBss->WPA.GroupCipher) &&
						(pInBss->WPA.GroupCipher != Ndis802_11GroupWEP40Enabled) && 
						(pInBss->WPA.GroupCipher != Ndis802_11GroupWEP104Enabled))
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipher) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipherAux))
						continue;						
				}
				else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA2.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA2.GroupCipher)
							continue;
						
					/* check group cipher*/
					if ((pAd->StaCfg.WepStatus < pInBss->WPA.GroupCipher) &&
						(pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP40Enabled) && 
						(pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP104Enabled))
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipher) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipherAux))
						continue;						
				}
#ifdef WAPI_SUPPORT
				else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWAICERT) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWAIPSK))
				{					
					/* check cipher algorithm*/
					if ((pAd->StaCfg.WepStatus != pInBss->WAPI.GroupCipher) || 
						(pAd->StaCfg.WepStatus != pInBss->WAPI.PairCipher))
						continue;											
				}
#endif /* WAPI_SUPPORT */
			}			
			/* Bss Type matched, SSID matched. */
			/* We will check wepstatus for qualification Bss*/
			else if (pAd->StaCfg.WepStatus != pInBss->WepStatus)
			{
				DBGPRINT(RT_DEBUG_TRACE,("StaCfg.WepStatus=%d, while pInBss->WepStatus=%d\n", pAd->StaCfg.WepStatus, pInBss->WepStatus));
				
				/* For the SESv2 case, we will not qualify WepStatus.*/
				
				if (!pInBss->bSES)
					continue;
			}

			/* Since the AP is using hidden SSID, and we are trying to connect to ANY*/
			/* It definitely will fail. So, skip it.*/
			/* CCX also require not even try to connect it!!*/
			if (SsidLen == 0)
				continue;
			
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			OutTab->BssNr++;
		}
		else if ((pInBss->BssType == pAd->StaCfg.BssType) && (SsidLen == 0))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

#ifdef WSC_STA_SUPPORT
			if ((pWpsCtrl->WscConfMode != WSC_DISABLE) && pWpsCtrl->bWscTrigger)
			{
				/* copy matching BSS from InTab to OutTab*/
				NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));
				OutTab->BssNr++;
				continue;
			}
#endif /* WSC_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
			/* 2.4G/5G N only mode*/
			if ((pInBss->HtCapabilityLen == 0) &&
				((pAd->CommonCfg.PhyMode == PHY_11N_2_4G) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			if ((pAd->CommonCfg.PhyMode == PHY_11GN_MIXED) &&
				((pInBss->SupRateLen + pInBss->ExtRateLen) < 12))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in GN-only Mode, this AP is in B mode.\n"));
				continue;
			}
#endif /* DOT11_N_SUPPORT */

			/* New for WPA2*/
			/* Check the Authmode first*/
			if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
			{
				/* Check AuthMode and AuthModeAux for matching, in case AP support dual-mode*/
				if ((pAd->StaCfg.AuthMode != pInBss->AuthMode) && (pAd->StaCfg.AuthMode != pInBss->AuthModeAux))
					/* None matched*/
					continue;
				
				/* Check cipher suite, AP must have more secured cipher than station setting*/
				if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA.GroupCipher)
							continue;
						
					/* check group cipher*/
					if (pAd->StaCfg.WepStatus < pInBss->WPA.GroupCipher)
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipher) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipherAux))
						continue;						
				}
				else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
				{
					/* If it's not mixed mode, we should only let BSS pass with the same encryption*/
					if (pInBss->WPA2.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA2.GroupCipher)
							continue;
						
					/* check group cipher*/
					if (pAd->StaCfg.WepStatus < pInBss->WPA2.GroupCipher)
						continue;

					/* check pairwise cipher, skip if none matched*/
					/* If profile set to AES, let it pass without question.*/
					/* If profile set to TKIP, we must find one mateched*/
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipher) && 
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipherAux))
						continue;						
				}
#ifdef WAPI_SUPPORT
				else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWAICERT) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWAIPSK))
				{					
					/* check cipher algorithm*/
					if ((pAd->StaCfg.WepStatus != pInBss->WAPI.GroupCipher) || 
						(pAd->StaCfg.WepStatus != pInBss->WAPI.PairCipher))
						continue;											
				}
#endif /* WAPI_SUPPORT */				
			}
			/* Bss Type matched, SSID matched. */
			/* We will check wepstatus for qualification Bss*/
			else if (pAd->StaCfg.WepStatus != pInBss->WepStatus)
					continue;
			
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			OutTab->BssNr++;
		}
#ifdef WSC_STA_SUPPORT		
		else if ((pWpsCtrl->WscConfMode != WSC_DISABLE) && 
				 (pWpsCtrl->bWscTrigger) &&
				 MAC_ADDR_EQUAL(pWpsCtrl->WscBssid, pInBss->Bssid))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];
			
			/* copy matching BSS from InTab to OutTab*/
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			/*
				Linksys WRT610N WPS AP will change the SSID from linksys to linksys_WPS_<four random characters> 
				when the Linksys WRT610N is in the state 'WPS Unconfigured' after set to factory default.
			*/
			NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pAd->MlmeAux.Ssid, pInBss->Ssid, pInBss->SsidLen);
			pAd->MlmeAux.SsidLen = pInBss->SsidLen;

			
			/* Update Reconnect Ssid, that user desired to connect.*/
			
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
			pAd->MlmeAux.AutoReconnectSsidLen = pAd->MlmeAux.SsidLen;

			OutTab->BssNr++;
			continue;
		}
#endif /* WSC_STA_SUPPORT */

		if (OutTab->BssNr >= MAX_LEN_OF_BSS_TABLE)
			break;
	}

	BssTableSortByRssi(OutTab, FALSE);
}
#endif /* CONFIG_STA_SUPPORT */


/* IRQL = DISPATCH_LEVEL*/
VOID BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder)
{
	INT 	  i, j;
/*	BSS_ENTRY TmpBss;*/
	BSS_ENTRY *pTmpBss = NULL;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pTmpBss, sizeof(BSS_ENTRY));
	if (pTmpBss == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	for (i = 0; i < OutTab->BssNr - 1; i++) 
	{
		for (j = i+1; j < OutTab->BssNr; j++) 
		{
			if (OutTab->BssEntry[j].Rssi > OutTab->BssEntry[i].Rssi ?
				!isInverseOrder : isInverseOrder)
			{
				if (OutTab->BssEntry[j].Rssi != OutTab->BssEntry[i].Rssi )
				{
					NdisMoveMemory(pTmpBss, &OutTab->BssEntry[j], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[j], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[i], pTmpBss, sizeof(BSS_ENTRY));
				}
			}
		}
	}

	if (pTmpBss != NULL)
		os_free_mem(NULL, pTmpBss);
}


VOID BssCipherParse(
	IN OUT	PBSS_ENTRY	pBss)
{
	PEID_STRUCT 		 pEid;
	PUCHAR				pTmp;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	PCIPHER_SUITE_STRUCT			pCipher;
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	INT								Length;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;

	
	/* WepStatus will be reset later, if AP announce TKIP or AES on the beacon frame.*/
	
	if (pBss->Privacy)
	{
		pBss->WepStatus 	= Ndis802_11WEPEnabled;
	}
	else
	{
		pBss->WepStatus 	= Ndis802_11WEPDisabled;
	}
	/* Set default to disable & open authentication before parsing variable IE*/
	pBss->AuthMode		= Ndis802_11AuthModeOpen;
	pBss->AuthModeAux	= Ndis802_11AuthModeOpen;

	/* Init WPA setting*/
	pBss->WPA.PairCipher	= Ndis802_11WEPDisabled;
	pBss->WPA.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WPA.GroupCipher	= Ndis802_11WEPDisabled;
	pBss->WPA.RsnCapability = 0;
	pBss->WPA.bMixMode		= FALSE;

	/* Init WPA2 setting*/
	pBss->WPA2.PairCipher	 = Ndis802_11WEPDisabled;
	pBss->WPA2.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WPA2.GroupCipher	 = Ndis802_11WEPDisabled;
	pBss->WPA2.RsnCapability = 0;
	pBss->WPA2.bMixMode 	 = FALSE;

#ifdef WAPI_SUPPORT
	/* Init WAPI setting*/
	pBss->WAPI.PairCipher	 = Ndis802_11WEPDisabled;
	pBss->WAPI.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WAPI.GroupCipher	 = Ndis802_11WEPDisabled;
	pBss->WAPI.RsnCapability = 0;
	pBss->WAPI.bMixMode 	 = FALSE;
#endif /* WAPI_SUPPORT */
	
	Length = (INT) pBss->VarIELen;

	while (Length > 0)
	{
		/* Parse cipher suite base on WPA1 & WPA2, they should be parsed differently*/
		pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - Length;
		pEid = (PEID_STRUCT) pTmp;
		switch (pEid->Eid)
		{
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7))
				{
					pBss->bSES = TRUE;
					break;
				}				
				else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
				{
					/* if unsupported vendor specific IE*/
					break;
				}				
				/*
					Skip OUI, version, and multicast suite
					This part should be improved in the future when AP supported multiple cipher suite.
					For now, it's OK since almost all APs have fixed cipher suite supported.
				*/
				/* pTmp = (PUCHAR) pEid->Octet;*/
				pTmp   += 11;

				/* 
					Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
					Value	   Meaning
					0			None
					1			WEP-40
					2			Tkip
					3			WRAP
					4			AES
					5			WEP-104
				*/
				/* Parse group cipher*/
				switch (*pTmp)
				{
					case 1:
						pBss->WPA.GroupCipher = Ndis802_11GroupWEP40Enabled;
						break;
					case 5:
						pBss->WPA.GroupCipher = Ndis802_11GroupWEP104Enabled;
						break;
					case 2:
						pBss->WPA.GroupCipher = Ndis802_11Encryption2Enabled;
						break;
					case 4:
						pBss->WPA.GroupCipher = Ndis802_11Encryption3Enabled;
						break;
					default:
						break;
				}
				/* number of unicast suite*/
				pTmp   += 1;

				/* skip all unicast cipher suites*/
				/*Count = *(PUSHORT) pTmp;				*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
							TmpCipher = Ndis802_11Encryption1Enabled;
							break;
						case 2:
							TmpCipher = Ndis802_11Encryption2Enabled;
							break;
						case 4:
							TmpCipher = Ndis802_11Encryption3Enabled;
							break;
						default:
							break;
					}
					if (TmpCipher > pBss->WPA.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						pBss->WPA.PairCipherAux = pBss->WPA.PairCipher;
						pBss->WPA.PairCipher	= TmpCipher;
					}
					else
					{
						pBss->WPA.PairCipherAux = TmpCipher;
					}
					pTmp++;
					Count--;
				}
				
				/* 4. get AKM suite counts*/
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				pTmp   += 3;
				
				switch (*pTmp)
				{
					case 1:
						/* Set AP support WPA-enterprise mode*/
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPA;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPA;
						break;
					case 2:
						/* Set AP support WPA-PSK mode*/
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPAPSK;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPAPSK;
						break;
					default:
						break;
				}
				pTmp   += 1;

				/* Fixed for WPA-None*/
				if (pBss->BssType == BSS_ADHOC)
				{
					pBss->AuthMode	  = Ndis802_11AuthModeWPANone;
					pBss->AuthModeAux = Ndis802_11AuthModeWPANone;
					pBss->WepStatus   = pBss->WPA.GroupCipher;
					/* Patched bugs for old driver*/
					if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
						pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
				}
				else
					pBss->WepStatus   = pBss->WPA.PairCipher;					
				
				/* Check the Pair & Group, if different, turn on mixed mode flag*/
				if (pBss->WPA.GroupCipher != pBss->WPA.PairCipher)
					pBss->WPA.bMixMode = TRUE;
				
				break;

			case IE_RSN:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
				
				/* 0. Version must be 1*/
				if (le2cpu16(pRsnHeader->Version) != 1)
					break;
				pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Check group cipher*/
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
					break;

				/* Parse group cipher*/
				switch (pCipher->Type)
				{
					case 1:
						pBss->WPA2.GroupCipher = Ndis802_11GroupWEP40Enabled;
						break;
					case 5:
						pBss->WPA2.GroupCipher = Ndis802_11GroupWEP104Enabled;
						break;
					case 2:
						pBss->WPA2.GroupCipher = Ndis802_11Encryption2Enabled;
						break;
					case 4:
						pBss->WPA2.GroupCipher = Ndis802_11Encryption3Enabled;
						break;
					default:
						break;
				}
				/* set to correct offset for next parsing*/
				pTmp   += sizeof(CIPHER_SUITE_STRUCT);

				/* 2. Get pairwise cipher counts*/
				/*Count = *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);			

				/* 3. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (pCipher->Type)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
							TmpCipher = Ndis802_11Encryption1Enabled;
							break;
						case 2:
							TmpCipher = Ndis802_11Encryption2Enabled;
							break;
						case 4:
							TmpCipher = Ndis802_11Encryption3Enabled;
							break;
						default:
							break;
					}
					if (TmpCipher > pBss->WPA2.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						pBss->WPA2.PairCipherAux = pBss->WPA2.PairCipher;
						pBss->WPA2.PairCipher	 = TmpCipher;
					}
					else
					{
						pBss->WPA2.PairCipherAux = TmpCipher;
					}
					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}
				
				/* 4. get AKM suite counts*/
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* 5. Get AKM ciphers*/
				/* Parsing all AKM ciphers*/
				while (Count > 0)
				{					
					pAKM = (PAKM_SUITE_STRUCT) pTmp;
					if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
						break;

					switch (pAKM->Type)
					{
						case 0:
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeWPANone;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeWPANone;
							break;                                                        
						case 1:
							/* Set AP support WPA-enterprise mode*/
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeWPA2;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeWPA2;
							break;
						case 2:
							/* Set AP support WPA-PSK mode*/
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeWPA2PSK;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeWPA2PSK;


							break;
						default:
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeMax;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeMax;
							break;
					}
					pTmp   += sizeof(AKM_SUITE_STRUCT);
					Count--;
				}

				/* Fixed for WPA-None*/
				if (pBss->BssType == BSS_ADHOC)
				{
					pBss->WPA.PairCipherAux = pBss->WPA2.PairCipherAux;
					pBss->WPA.GroupCipher	= pBss->WPA2.GroupCipher;
					pBss->WepStatus 		= pBss->WPA.GroupCipher;
					/* Patched bugs for old driver*/
					if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
						pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
				}
				pBss->WepStatus   = pBss->WPA2.PairCipher;					
				
				/* 6. Get RSN capability*/
				/*pBss->WPA2.RsnCapability = *(PUSHORT) pTmp;*/
				pBss->WPA2.RsnCapability = (pTmp[1]<<8) + pTmp[0];
				pTmp += sizeof(USHORT);
				
				/* Check the Pair & Group, if different, turn on mixed mode flag*/
				if (pBss->WPA2.GroupCipher != pBss->WPA2.PairCipher)
					pBss->WPA2.bMixMode = TRUE;
				
				break;
#ifdef WAPI_SUPPORT
			case IE_WAPI:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

				/* 0. The version number must be 1*/
				if (le2cpu16(pRsnHeader->Version) != 1)
					break;
				pTmp += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Get AKM suite counts*/
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));	
    			Count = cpu2le16(Count);				
				pTmp += sizeof(USHORT);

				/* 2. Get AKM ciphers*/
				pAKM = (PAKM_SUITE_STRUCT) pTmp;
				if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
					break;

				switch (pAKM->Type)
				{					
					case 1:
						/* Support WAI certificate authentication*/
						pBss->AuthMode = Ndis802_11AuthModeWAICERT;						
						break;
					case 2:
						/* Support WAI PSK*/
						pBss->AuthMode = Ndis802_11AuthModeWAIPSK;						
						break;
					default:
						break;
				}
				pTmp += (Count * sizeof(AKM_SUITE_STRUCT));

				/* 3. Get pairwise cipher counts*/
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));	
    			Count = cpu2le16(Count);	
				pTmp += sizeof(USHORT);			

				/* 4. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0)
				{
					if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
						break;
				
					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;					
					TmpCipher = Ndis802_11WEPDisabled;
					switch (pCipher->Type)
					{
						case 1:						
							TmpCipher = Ndis802_11EncryptionSMS4Enabled;
							break;
						default:
							break;
					}
					
					if (TmpCipher > pBss->WAPI.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux*/
						pBss->WAPI.PairCipherAux = pBss->WAPI.PairCipher;
						pBss->WAPI.PairCipher	 = TmpCipher;
					}
					else
					{
						pBss->WAPI.PairCipherAux = TmpCipher;
					}
					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}
				
				/* 5. Check group cipher*/
				if (!RTMPEqualMemory(pTmp, WAPI_OUI, 3))
					break;
				
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;				
				/* Parse group cipher*/
				switch (pCipher->Type)
				{
					case 1:
						pBss->WAPI.GroupCipher = Ndis802_11EncryptionSMS4Enabled;
						break;
					default:
						break;
				}
				/* set to correct offset for next parsing*/
				pTmp += sizeof(CIPHER_SUITE_STRUCT);

				/* update the encryption type*/
				pBss->WepStatus = pBss->WAPI.PairCipher;

				/* update the WAPI capability*/
				pBss->WAPI.RsnCapability = (pTmp[1]<<8) + pTmp[0];
				pTmp += sizeof(USHORT);

				break;
#endif /* WAPI_SUPPORT */				
			default:
				break;
		}
		Length -= (pEid->Len + 2);
	}
}

/* ===========================================================================================*/
/* mac_table.c*/
/* ===========================================================================================*/

/*! \brief generates a random mac address value for IBSS BSSID
 *	\param Addr the bssid location
 *	\return none
 *	\pre
 *	\post
 */
VOID MacAddrRandomBssid(
	IN PRTMP_ADAPTER pAd, 
	OUT PUCHAR pAddr) 
{
	INT i;

	for (i = 0; i < MAC_ADDR_LEN; i++) 
	{
		pAddr[i] = RandomByte(pAd);
	}

	pAddr[0] = (pAddr[0] & 0xfe) | 0x02;  /* the first 2 bits must be 01xxxxxxxx*/
}

/*! \brief init the management mac frame header
 *	\param p_hdr mac header
 *	\param subtype subtype of the frame
 *	\param p_ds destination address, don't care if it is a broadcast address
 *	\return none
 *	\pre the station has the following information in the pAd->StaCfg
 *	 - bssid
 *	 - station address
 *	\post
 *	\note this function initializes the following field

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
VOID MgtMacHeaderInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN OUT PHEADER_802_11 pHdr80211, 
	IN UCHAR SubType, 
	IN UCHAR ToDs, 
	IN PUCHAR pDA, 
#ifdef P2P_SUPPORT
	IN PUCHAR pSA,
#endif /* P2P_SUPPORT */
	IN PUCHAR pBssid) 
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	
	pHdr80211->FC.Type = BTYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
/*	if (SubType == SUBTYPE_ACK)	 sample, no use, it will conflict with ACTION frame sub type*/
/*		pHdr80211->FC.Type = BTYPE_CNTL;*/
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
#ifdef P2P_SUPPORT
		COPY_MAC_ADDR(pHdr80211->Addr2, pSA);
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		COPY_MAC_ADDR(pHdr80211->Addr2, pBssid);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		COPY_MAC_ADDR(pHdr80211->Addr2, pAd->CurrentAddress);
#endif /* CONFIG_STA_SUPPORT */
#endif /* P2P_SUPPORT */
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}

VOID MgtMacHeaderInitExt(
    IN  PRTMP_ADAPTER   pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN PUCHAR pAddr1,
    IN PUCHAR pAddr2,
    IN PUCHAR pAddr3)
{
    NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));

    pHdr80211->FC.Type = BTYPE_MGMT;
    pHdr80211->FC.SubType = SubType;
    pHdr80211->FC.ToDs = ToDs;
    COPY_MAC_ADDR(pHdr80211->Addr1, pAddr1);
    COPY_MAC_ADDR(pHdr80211->Addr2, pAddr2);
    COPY_MAC_ADDR(pHdr80211->Addr3, pAddr3);
}


/* ===========================================================================================*/
/* mem_mgmt.c*/
/* ===========================================================================================*/

/*!***************************************************************************
 * This routine build an outgoing frame, and fill all information specified 
 * in argument list to the frame body. The actual frame size is the summation 
 * of all arguments.
 * input params:
 *		Buffer - pointer to a pre-allocated memory segment
 *		args - a list of <int arg_size, arg> pairs.
 *		NOTE NOTE NOTE!!!! the last argument must be NULL, otherwise this
 *						   function will FAIL!!!
 * return:
 *		Size of the buffer
 * usage:  
 *		MakeOutgoingFrame(Buffer, output_length, 2, &fc, 2, &dur, 6, p_addr1, 6,p_addr2, END_OF_ARGS);

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 ****************************************************************************/
ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer, 
	OUT ULONG *FrameLen, ...) 
{
	UCHAR   *p;
	int 	leng;
	ULONG	TotLeng;
	va_list Args;

	/* calculates the total length*/
	TotLeng = 0;
	va_start(Args, FrameLen);
	do 
	{
		leng = va_arg(Args, int);
		if (leng == END_OF_ARGS) 
		{
			break;
		}
		p = va_arg(Args, PVOID);
		NdisMoveMemory(&Buffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while(TRUE);

	va_end(Args); /* clean up */
	*FrameLen = TotLeng;
	return TotLeng;
}

/* ===========================================================================================*/
/* mlme_queue.c*/
/* ===========================================================================================*/

/*! \brief	Initialize The MLME Queue, used by MLME Functions
 *	\param	*Queue	   The MLME Queue
 *	\return Always	   Return NDIS_STATE_SUCCESS in this implementation
 *	\pre
 *	\post
 *	\note	Because this is done only once (at the init stage), no need to be locked

 IRQL = PASSIVE_LEVEL
 
 */
NDIS_STATUS MlmeQueueInit(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE *Queue) 
{
	INT i;

	NdisAllocateSpinLock(pAd, &Queue->Lock);

	Queue->Num	= 0;
	Queue->Head = 0;
	Queue->Tail = 0;

	for (i = 0; i < MAX_LEN_OF_MLME_QUEUE; i++) 
	{
		Queue->Entry[i].Occupied = FALSE;
		Queue->Entry[i].MsgLen = 0;
		NdisZeroMemory(Queue->Entry[i].Msg, MGMT_DMA_BUFFER_SIZE);
	}

	return NDIS_STATUS_SUCCESS;
}

/*! \brief	 Enqueue a message for other threads, if they want to send messages to MLME thread
 *	\param	*Queue	  The MLME Queue
 *	\param	 Machine  The State Machine Id
 *	\param	 MsgType  The Message Type
 *	\param	 MsgLen   The Message length
 *	\param	*Msg	  The message pointer
 *	\return  TRUE if enqueue is successful, FALSE if the queue is full
 *	\pre
 *	\post
 *	\note	 The message has to be initialized

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
BOOLEAN MlmeEnqueue(
	IN	PRTMP_ADAPTER	pAd,
	IN ULONG Machine, 
	IN ULONG MsgType, 
	IN ULONG MsgLen, 
	IN VOID *Msg,
	IN ULONG Priv) 
{
	INT Tail;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		DBGPRINT_ERR(("MlmeEnqueue: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}
	
	if (MlmeQueueFull(Queue, 1)) 
	{
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	/* Double check for safety multi-thread system. */
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE) 
	{
		Queue->Tail = 0;
	}
	
	Queue->Entry[Tail].Wcid = RESERVED_WCID;
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;	
	Queue->Entry[Tail].Priv = Priv;
	
	if (Msg != NULL)
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}
		
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}

/*! \brief	 This function is used when Recv gets a MLME message
 *	\param	*Queue			 The MLME Queue
 *	\param	 TimeStampHigh	 The upper 32 bit of timestamp
 *	\param	 TimeStampLow	 The lower 32 bit of timestamp
 *	\param	 Rssi			 The receiving RSSI strength
 *	\param	 MsgLen 		 The length of the message
 *	\param	*Msg			 The message pointer
 *	\return  TRUE if everything ok, FALSE otherwise (like Queue Full)
 *	\pre
 *	\post
 
 IRQL = DISPATCH_LEVEL
 
 */
BOOLEAN MlmeEnqueueForRecv(
	IN	PRTMP_ADAPTER	pAd, 
	IN ULONG Wcid, 
	IN ULONG TimeStampHigh, 
	IN ULONG TimeStampLow,
	IN UCHAR Rssi0, 
	IN UCHAR Rssi1, 
	IN UCHAR Rssi2, 
	IN ULONG MsgLen, 
	IN VOID *Msg,
	IN UCHAR Signal,
	IN UCHAR OpMode)
{
	INT 		 Tail, Machine = 0xff;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
	INT		 MsgType = 0x0;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
#ifdef APCLI_SUPPORT
	UCHAR ApCliIdx = 0;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef RALINK_ATE			
	/* Nothing to do in ATE mode */
	if(ATE_ON(pAd))
		return FALSE;
#endif /* RALINK_ATE */

	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		DBGPRINT_ERR(("MlmeEnqueueForRecv: fRTMP_ADAPTER_HALT_IN_PROGRESS\n"));
		return FALSE;
	}

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		DBGPRINT_ERR(("MlmeEnqueueForRecv: frame too large, size = %ld \n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue, 0)) 
	{
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef P2P_SUPPORT
	if (OpMode == OPMODE_AP)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
	{
#ifdef P2P_SUPPORT
		if (P2P_CLI_ON(pAd))
		{
			if (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_REQ)
				return FALSE;
		}
#endif /* P2P_SUPPORT */
#ifdef APCLI_SUPPORT
		/* Beacon must be handled by ap-sync state machine.*/
		/* Probe-rsp must be handled by apcli-sync state machine.*/
		/* Those packets don't need to check its MAC address.*/
		do
		{
			BOOLEAN bToApCli = FALSE;
			UCHAR i;

			/* 
			   1. When P2P GO On and receive Probe Response, preCheckMsgTypeSubset function will 
			      enquene Probe response to APCli sync state machine
			      Solution: when GO On skip preCheckMsgTypeSubset redirect to APMsgTypeSubst
			   2. When P2P Cli On and receive Probe Response, preCheckMsgTypeSubset function will
			      enquene Probe response to APCli sync state machine
			      Solution: handle MsgType == APCLI_MT2_PEER_PROBE_RSP on ApCli Sync state machine
			                when ApCli on idle state.
			*/

			for (i = 0; i < MAX_APCLI_NUM; i++)
			{
				if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[i].ApCliMlmeAux.Bssid, pFrame->Hdr.Addr2))
				{
					bToApCli = TRUE;
					ApCliIdx = i;
					break;
				}
			}

			if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr1, pAd->CurrentAddress) &&
#ifdef P2P_SUPPORT
				!P2P_GO_ON(pAd) &&
#endif /* P2P_SUPPORT */
				preCheckMsgTypeSubset(pAd, pFrame, &Machine, &MsgType))
				break;

			if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr1, pAd->CurrentAddress) && bToApCli)
			{
				if (ApCliMsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
					break;
			}
			else
			{
				if (APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
					break;
			}

			DBGPRINT_ERR(("MlmeEnqueueForRecv: un-recongnized mgmt->subtype=%d, STA-%02x:%02x:%02x:%02x:%02x:%02x\n", 
						pFrame->Hdr.FC.SubType, pFrame->Hdr.Addr2[0], pFrame->Hdr.Addr2[1], pFrame->Hdr.Addr2[2],
						pFrame->Hdr.Addr2[3], pFrame->Hdr.Addr2[4], pFrame->Hdr.Addr2[5]));
			return FALSE;

		} while (FALSE);
#else
		if (!APMsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) 
		{
			DBGPRINT_ERR(("MlmeEnqueueForRecv: un-recongnized mgmt->subtype=%d\n",pFrame->Hdr.FC.SubType));
			return FALSE;
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */	
#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
	if (OpMode == OPMODE_STA)
#else
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT */
	{
		if (!MsgTypeSubst(pAd, pFrame, &Machine, &MsgType)) 
		{
			DBGPRINT_ERR(("MlmeEnqueueForRecv: un-recongnized mgmt->subtype=%d\n",pFrame->Hdr.FC.SubType));
			return FALSE;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	/* OK, we got all the informations, it is time to put things into queue*/

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	/* Double check for safety multi-thread system. */
	if (Queue->Entry[Tail].Occupied)
	{
		NdisReleaseSpinLock(&(Queue->Lock));
		return FALSE;
	}
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE) 
	{
		Queue->Tail = 0;
	}
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = TimeStampLow;
	Queue->Entry[Tail].TimeStamp.u.HighPart = TimeStampHigh;
	Queue->Entry[Tail].Rssi0 = Rssi0;
	Queue->Entry[Tail].Rssi1 = Rssi1;
	Queue->Entry[Tail].Rssi2 = Rssi2;
	Queue->Entry[Tail].Signal = Signal;
	Queue->Entry[Tail].Wcid = (UCHAR)Wcid;
	Queue->Entry[Tail].OpMode = (ULONG)OpMode;
	Queue->Entry[Tail].Priv = 0;
#ifdef APCLI_SUPPORT
	Queue->Entry[Tail].Priv = ApCliIdx;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn)
	{
		for (CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
		{
			if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ApCliIdx].RepeaterCli[CliIdx].CurrentAddress, pFrame->Hdr.Addr1))
			{
				Queue->Entry[Tail].Priv = (64 + (MAX_EXT_MAC_ADDR_SIZE * ApCliIdx) + CliIdx);
				break;
			}
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

	Queue->Entry[Tail].Channel = pAd->LatchRfRegs.Channel;
	
	if (Msg != NULL)
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}

	NdisReleaseSpinLock(&(Queue->Lock));	
	RTMP_MLME_HANDLER(pAd);

	return TRUE;
}

#ifdef WSC_INCLUDED
/*! \brief   Enqueue a message for other threads, if they want to send messages to MLME thread
 *  \param  *Queue    The MLME Queue
 *  \param   TimeStampLow    The lower 32 bit of timestamp, here we used for eventID.
 *  \param   Machine  The State Machine Id
 *  \param   MsgType  The Message Type
 *  \param   MsgLen   The Message length
 *  \param  *Msg      The message pointer
 *  \return  TRUE if enqueue is successful, FALSE if the queue is full
 *  \pre
 *  \post
 *  \note    The message has to be initialized
 */
BOOLEAN MlmeEnqueueForWsc(
	IN	PRTMP_ADAPTER	pAd,
	IN ULONG eventID,
	IN LONG senderID,
	IN ULONG Machine, 
	IN ULONG MsgType, 
	IN ULONG MsgLen, 
	IN VOID *Msg) 
{
    INT Tail;
    /*ULONG			IrqFlags;*/
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	DBGPRINT(RT_DEBUG_TRACE, ("-----> MlmeEnqueueForWsc\n"));
    /* Do nothing if the driver is starting halt state.*/
    /* This might happen when timer already been fired before cancel timer with mlmehalt*/
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
        DBGPRINT_ERR(("MlmeEnqueueForWsc: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}
	
    if (MlmeQueueFull(Queue, 1)) 
    {
        
        return FALSE;
    }

    /* OK, we got all the informations, it is time to put things into queue*/
	NdisAcquireSpinLock(&(Queue->Lock));
    Tail = Queue->Tail;
    /* Double check for safety multi-thread system. */
    if (Queue->Entry[Tail].Occupied)
    {
        NdisReleaseSpinLock(&(Queue->Lock));
        return FALSE;
    }
    Queue->Tail++;
    Queue->Num++;
    if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE) 
    {
        Queue->Tail = 0;
    }
    
    Queue->Entry[Tail].Occupied = TRUE;
    Queue->Entry[Tail].Machine = Machine;
    Queue->Entry[Tail].MsgType = MsgType;
    Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = eventID;
	Queue->Entry[Tail].TimeStamp.u.HighPart = senderID;
    if (Msg != NULL)
        NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

    NdisReleaseSpinLock(&(Queue->Lock));

	DBGPRINT(RT_DEBUG_TRACE, ("<----- MlmeEnqueueForWsc\n"));
	
    return TRUE;
}
#endif /* WSC_INCLUDED */

/*! \brief	 Dequeue a message from the MLME Queue
 *	\param	*Queue	  The MLME Queue
 *	\param	*Elem	  The message dequeued from MLME Queue
 *	\return  TRUE if the Elem contains something, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue, 
	OUT MLME_QUEUE_ELEM **Elem) 
{
	NdisAcquireSpinLock(&(Queue->Lock));
	*Elem = &(Queue->Entry[Queue->Head]);    
	Queue->Num--;
	Queue->Head++;
	if (Queue->Head == MAX_LEN_OF_MLME_QUEUE) 
	{
		Queue->Head = 0;
	}
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}

/* IRQL = DISPATCH_LEVEL*/
VOID	MlmeRestartStateMachine(
	IN	PRTMP_ADAPTER	pAd)
{
#ifdef RTMP_MAC_PCI
	MLME_QUEUE_ELEM		*Elem = NULL;
#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN				Cancelled;
#ifdef P2P_SUPPORT
	PAPCLI_STRUCT	pApCliEntry = NULL;
#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	
	DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine \n"));

#ifdef RTMP_MAC_PCI
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning) 
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	} 
	else 
	{
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	/* Remove all Mlme queues elements*/
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) 
	{
		/*From message type, determine which state machine I should drive*/
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) 
		{
			/* free MLME element*/
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			DBGPRINT_ERR(("MlmeRestartStateMachine: MlmeQueue empty\n"));
		}
	}
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef QOS_DLS_SUPPORT
		UCHAR i;
#endif /* QOS_DLS_SUPPORT */
		/* Cancel all timer events*/
		/* Be careful to cancel new added timer*/
		RTMPCancelTimer(&pAd->MlmeAux.AssocTimer,	  &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer,   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer,  &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.AuthTimer,	   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer,	   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer,	   &Cancelled);

#ifdef QOS_DLS_SUPPORT
		for (i=0; i<MAX_NUM_OF_DLS_ENTRY; i++)
		{
			RTMPCancelTimer(&pAd->StaCfg.DLSEntry[i].Timer, &Cancelled);
		}
#endif /* QOS_DLS_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Change back to original channel in case of doing scan*/
#ifdef P2P_SUPPORT
	pApCliEntry = &pAd->ApCfg.ApCliTab[BSS0];
	
	if (!P2P_GO_ON(pAd) && (pApCliEntry->Valid == FALSE))
#endif /* P2P_SUPPORT */
	{
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
	}

	/* Resume MSDU which is turned off durning scan*/
	RTMPResumeMsduTransmission(pAd);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Set all state machines back IDLE*/
		pAd->Mlme.CntlMachine.CurrState    = CNTL_IDLE;
		pAd->Mlme.AssocMachine.CurrState   = ASSOC_IDLE;
		pAd->Mlme.AuthMachine.CurrState    = AUTH_REQ_IDLE;
		pAd->Mlme.AuthRspMachine.CurrState = AUTH_RSP_IDLE;
		pAd->Mlme.SyncMachine.CurrState    = SYNC_IDLE;
		pAd->Mlme.ActMachine.CurrState    = ACT_IDLE;
#ifdef QOS_DLS_SUPPORT
		pAd->Mlme.DlsMachine.CurrState    = DLS_IDLE;
#endif /* QOS_DLS_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
		pAd->Mlme.TdlsMachine.CurrState    = TDLS_IDLE;
#endif /* DOT11Z_TDLS_SUPPORT */

	}
#endif /* CONFIG_STA_SUPPORT */
	
#ifdef RTMP_MAC_PCI	
	/* Remove running state*/
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#endif /* RTMP_MAC_PCI */
}

/*! \brief	test if the MLME Queue is empty
 *	\param	*Queue	  The MLME Queue
 *	\return TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post
 
 IRQL = DISPATCH_LEVEL
 
 */
BOOLEAN MlmeQueueEmpty(
	IN MLME_QUEUE *Queue) 
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == 0);
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}

/*! \brief	 test if the MLME Queue is full
 *	\param	 *Queue 	 The MLME Queue
 *	\return  TRUE if the Queue is empty, FALSE otherwise
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeQueueFull(
	IN MLME_QUEUE *Queue,
	IN UCHAR SendId) 
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	if (SendId == 0)
		Ans = ((Queue->Num >= (MAX_LEN_OF_MLME_QUEUE / 2)) || Queue->Entry[Queue->Tail].Occupied);
	else
		Ans = (Queue->Num == MAX_LEN_OF_MLME_QUEUE)  || Queue->Entry[Queue->Tail].Occupied;
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}

/*! \brief	 The destructor of MLME Queue
 *	\param 
 *	\return
 *	\pre
 *	\post
 *	\note	Clear Mlme Queue, Set Queue->Num to Zero.

 IRQL = PASSIVE_LEVEL
 
 */
VOID MlmeQueueDestroy(
	IN MLME_QUEUE *pQueue) 
{
	NdisAcquireSpinLock(&(pQueue->Lock));
	pQueue->Num  = 0;
	pQueue->Head = 0;
	pQueue->Tail = 0;
	NdisReleaseSpinLock(&(pQueue->Lock));
	NdisFreeSpinLock(&(pQueue->Lock));
}


/*! \brief	 To substitute the message type if the message is coming from external
 *	\param	pFrame		   The frame received
 *	\param	*Machine	   The state machine
 *	\param	*MsgType	   the message type for the state machine
 *	\return TRUE if the substitution is successful, FALSE otherwise
 *	\pre
 *	\post

 IRQL = DISPATCH_LEVEL

 */
#ifdef CONFIG_STA_SUPPORT
BOOLEAN MsgTypeSubst(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame, 
	OUT INT *Machine, 
	OUT INT *MsgType) 
{
	USHORT	Seq, Alg;
	UCHAR	EAPType;
	PUCHAR	pData;
	BOOLEAN bRV = FALSE;
#ifdef WSC_STA_SUPPORT
	UCHAR EAPCode;
#endif /* WSC_STA_SUPPORT */    

	/* Pointer to start of data frames including SNAP header*/
	pData = (PUCHAR) pFrame + LENGTH_802_11;

	/* The only data type will pass to this function is EAPOL frame*/
	if (pFrame->Hdr.FC.Type == BTYPE_DATA) 
	{
#ifdef DOT11Z_TDLS_SUPPORT
		if (NdisEqualMemory(TDLS_LLC_SNAP_WITH_CATEGORY, pData, LENGTH_802_1_H + 2))
		{
			UCHAR	TDLSType;

			/* ieee802.11z TDLS SNAP header*/
			*Machine = TDLS_STATE_MACHINE;
			TDLSType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 2);
			return (TDLS_MsgTypeSubst(TDLSType, (INT *)MsgType));
		}
		else
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef WSC_STA_SUPPORT
        /* check for WSC state machine first*/
		if (pAd->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP)
        {
            *Machine = WSC_STATE_MACHINE;
            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
            EAPCode = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
            
            bRV = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
 			if (bRV)
				return bRV;
        }
#endif /* WSC_STA_SUPPORT */
		if (bRV == FALSE)
		{
	        *Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
	        return (WpaMsgTypeSubst(EAPType, (INT *) MsgType));		
		}
	}

	switch (pFrame->Hdr.FC.SubType) 
	{
		case SUBTYPE_ASSOC_REQ:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_ASSOC_REQ;
			break;
		case SUBTYPE_ASSOC_RSP:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_ASSOC_RSP;
			break;
		case SUBTYPE_REASSOC_REQ:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_REASSOC_REQ;
			break;
		case SUBTYPE_REASSOC_RSP:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_REASSOC_RSP;
			break;
		case SUBTYPE_PROBE_REQ:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_PROBE_REQ;
			break;
		case SUBTYPE_PROBE_RSP:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_PROBE_RSP;
			break;
		case SUBTYPE_BEACON:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_BEACON;
			break;
		case SUBTYPE_ATIM:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_ATIM;
			break;
		case SUBTYPE_DISASSOC:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_DISASSOC_REQ;
			break;
		case SUBTYPE_AUTH:
			/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm*/
			NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
			NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
			if (Seq == 1 || Seq == 3) 
			{
				*Machine = AUTH_RSP_STATE_MACHINE;
				*MsgType = MT2_PEER_AUTH_ODD;
			} 
			else if (Seq == 2 || Seq == 4) 
			{
				if (Alg == AUTH_MODE_OPEN || Alg == AUTH_MODE_KEY)
				{
					*Machine = AUTH_STATE_MACHINE;
					*MsgType = MT2_PEER_AUTH_EVEN;
				} 
			} 
			else 
			{
				return FALSE;
			}
			break;
		case SUBTYPE_DEAUTH:
			*Machine = AUTH_RSP_STATE_MACHINE;
			*MsgType = MT2_PEER_DEAUTH;
			break;
		case SUBTYPE_ACTION:
		case SUBTYPE_ACTION_NO_ACK:
			*Machine = ACTION_STATE_MACHINE;
			/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support*/
			if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG) 
			{
				*MsgType = MT2_ACT_INVALID;
			} 
			else
			{
				*MsgType = (pFrame->Octet[0]&0x7F);
			} 
			break;
		default:
			return FALSE;
			break;
	}

	return TRUE;
}
#endif /* CONFIG_STA_SUPPORT */

/* ===========================================================================================*/
/* state_machine.c*/
/* ===========================================================================================*/

/*! \brief Initialize the state machine.
 *	\param *S			pointer to the state machine 
 *	\param	Trans		State machine transition function
 *	\param	StNr		number of states 
 *	\param	MsgNr		number of messages 
 *	\param	DefFunc 	default function, when there is invalid state/message combination 
 *	\param	InitState	initial state of the state machine 
 *	\param	Base		StateMachine base, internal use only
 *	\pre p_sm should be a legal pointer
 *	\post

 IRQL = PASSIVE_LEVEL
 
 */
VOID StateMachineInit(
	IN STATE_MACHINE *S, 
	IN STATE_MACHINE_FUNC Trans[], 
	IN ULONG StNr, 
	IN ULONG MsgNr, 
	IN STATE_MACHINE_FUNC DefFunc, 
	IN ULONG InitState, 
	IN ULONG Base) 
{
	ULONG i, j;

	/* set number of states and messages*/
	S->NrState = StNr;
	S->NrMsg   = MsgNr;
	S->Base    = Base;

	S->TransFunc  = Trans;

	/* init all state transition to default function*/
	for (i = 0; i < StNr; i++) 
	{
		for (j = 0; j < MsgNr; j++) 
		{
			S->TransFunc[i * MsgNr + j] = DefFunc;
		}
	}

	/* set the starting state*/
	S->CurrState = InitState;
}

/*! \brief This function fills in the function pointer into the cell in the state machine 
 *	\param *S	pointer to the state machine
 *	\param St	state
 *	\param Msg	incoming message
 *	\param f	the function to be executed when (state, message) combination occurs at the state machine
 *	\pre *S should be a legal pointer to the state machine, st, msg, should be all within the range, Base should be set in the initial state
 *	\post

 IRQL = PASSIVE_LEVEL
 
 */
VOID StateMachineSetAction(
	IN STATE_MACHINE *S, 
	IN ULONG St, 
	IN ULONG Msg, 
	IN STATE_MACHINE_FUNC Func) 
{
	ULONG MsgIdx;

	MsgIdx = Msg - S->Base;

	if (St < S->NrState && MsgIdx < S->NrMsg) 
	{
		/* boundary checking before setting the action*/
		S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
	} 
}

/*! \brief	 This function does the state transition
 *	\param	 *Adapter the NIC adapter pointer
 *	\param	 *S 	  the state machine
 *	\param	 *Elem	  the message to be executed
 *	\return   None
 
 IRQL = DISPATCH_LEVEL
 
 */
VOID StateMachinePerformAction(
	IN	PRTMP_ADAPTER	pAd, 
	IN STATE_MACHINE *S, 
	IN MLME_QUEUE_ELEM *Elem,
	IN ULONG CurrState)
{

	if (S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base])
		(*(S->TransFunc[(CurrState) * S->NrMsg + Elem->MsgType - S->Base]))(pAd, Elem);
}

/*
	==========================================================================
	Description:
		The drop function, when machine executes this, the message is simply 
		ignored. This function does nothing, the message is freed in 
		StateMachinePerformAction()
	==========================================================================
 */
VOID Drop(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
UCHAR RandomByte(
	IN PRTMP_ADAPTER pAd) 
{
	ULONG i;
	UCHAR R, Result;

	R = 0;

	if (pAd->Mlme.ShiftReg == 0)
	NdisGetSystemUpTime((ULONG *)&pAd->Mlme.ShiftReg);

	for (i = 0; i < 8; i++) 
	{
		if (pAd->Mlme.ShiftReg & 0x00000001) 
		{
			pAd->Mlme.ShiftReg = ((pAd->Mlme.ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		} 
		else 
		{
			pAd->Mlme.ShiftReg = pAd->Mlme.ShiftReg >> 1;
			Result = 0;
		}
		R = (R << 1) | Result;
	}

	return R;
}


UCHAR RandomByte2(
        IN PRTMP_ADAPTER pAd)
{
	UINT32 a,b;
	UCHAR value, value1 = 0, value2 = 0, value3 = 0, value4 = 0, value5 = 0;

	/*MAC statistic related*/
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &a);
	a &= 0x0000ffff;
	RTMP_IO_READ32(pAd, RX_STA_CNT0, &b); 
	b &= 0x0000ffff;
	value = (a<<16)|b;

	/*R50~R54: RSSI or SNR related*/
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R50, &value1);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R51, &value2);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R52, &value3);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R53, &value4);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R54, &value5);

	return value^value1^value2^value3^value4^value5^RandomByte(pAd);
}


/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID	RTMPCheckRates(
	IN		PRTMP_ADAPTER	pAd,
	IN OUT	UCHAR			SupRate[],
	IN OUT	UCHAR			*SupRateLen)
{
	UCHAR	RateIdx, i, j;
	UCHAR	NewRate[12], NewRateLen;
	
	NewRateLen = 0;
	
	if (pAd->CommonCfg.PhyMode == PHY_11B)
		RateIdx = 4;
	else
		RateIdx = 12;

	/* Check for support rates exclude basic rate bit	*/
	for (i = 0; i < *SupRateLen; i++)
		for (j = 0; j < RateIdx; j++)
			if ((SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
				NewRate[NewRateLen++] = SupRate[i];
			
	*SupRateLen = NewRateLen;
	NdisMoveMemory(SupRate, NewRate, NewRateLen);
}

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT
BOOLEAN RTMPCheckChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		CentralChannel,
	IN UCHAR		Channel)
{
	UCHAR		k;
	UCHAR		UpperChannel = 0, LowerChannel = 0;
	UCHAR		NoEffectChannelinList = 0;
	
	/* Find upper and lower channel according to 40MHz current operation. */
	if (CentralChannel < Channel)
	{
		UpperChannel = Channel;
		if (CentralChannel > 2)
			LowerChannel = CentralChannel - 2;
		else
			return FALSE;
	}
	else if (CentralChannel > Channel)
	{
		UpperChannel = CentralChannel + 2;
		LowerChannel = Channel;
	}

	for (k = 0;k < pAd->ChannelListNum;k++)
	{
		if (pAd->ChannelList[k].Channel == UpperChannel)
		{
			NoEffectChannelinList ++;
		}
		if (pAd->ChannelList[k].Channel == LowerChannel)
		{
			NoEffectChannelinList ++;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE,("Total Channel in Channel List = [%d]\n", NoEffectChannelinList));
	if (NoEffectChannelinList == 2)
		return TRUE;
	else
		return FALSE;
}

/*
	========================================================================

	Routine Description:
		Verify the support rate for HT phy type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		FALSE if pAd->CommonCfg.SupportedHtPhy doesn't accept the pHtCapability.  (AP Mode)

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
BOOLEAN 	RTMPCheckHt(
	IN	PRTMP_ADAPTER			pAd,
	IN	UCHAR					Wcid,
	IN 	HT_CAPABILITY_IE		*pHtCapability,
	IN 	ADD_HT_INFO_IE			*pAddHtInfo)
{
	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	/* If use AMSDU, set flag.*/
	if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_AMSDU_INUSED);
	/* Save Peer Capability*/
	if (pHtCapability->HtCapInfo.ShortGIfor20)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_SGI20_CAPABLE);
	if (pHtCapability->HtCapInfo.ShortGIfor40)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_SGI40_CAPABLE);
	if (pHtCapability->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_TxSTBC_CAPABLE);
	if (pHtCapability->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_RxSTBC_CAPABLE);
	if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)
	{
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_RDG_CAPABLE);
	}
	
	if (Wcid < MAX_LEN_OF_MAC_TABLE)
	{
		pAd->MacTab.Content[Wcid].MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	}

	/* Will check ChannelWidth for MCSSet[4] below*/
	pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
	pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
	pAd->MlmeAux.HtCapability.MCSSet[4] = 0x1;
    switch (pAd->CommonCfg.RxStream)
	{
		case 1:			
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
		case 2:
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
		case 3:				
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
	}	

	pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pAd->CommonCfg.DesiredHtPhy.ChannelWidth;
		
	/*
		If both station and AP use 40MHz, still need to check if the 40MHZ band's legality in my country region
		If this 40MHz wideband is not allowed in my country list, use bandwidth 20MHZ instead,
	*/
	if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
	{
		if (RTMPCheckChannel(pAd, pAd->MlmeAux.CentralChannel, pAd->MlmeAux.Channel) == FALSE)
		{
			pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
		}
	}
		
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPCheckHt:: HtCapInfo.ChannelWidth=%d, RecomWidth=%d, DesiredHtPhy.ChannelWidth=%d, BW40MAvailForA/G=%d/%d, PhyMode=%d \n",
		pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth, pAddHtInfo->AddHtInfo.RecomWidth, pAd->CommonCfg.DesiredHtPhy.ChannelWidth,
		pAd->NicConfig2.field.BW40MAvailForA, pAd->NicConfig2.field.BW40MAvailForG, pAd->CommonCfg.PhyMode));
    
	pAd->MlmeAux.HtCapability.HtCapInfo.GF =  pHtCapability->HtCapInfo.GF &pAd->CommonCfg.DesiredHtPhy.GF;

	/* Send Assoc Req with my HT capability.*/
	pAd->MlmeAux.HtCapability.HtCapInfo.AMsduSize =  pAd->CommonCfg.DesiredHtPhy.AmsduSize;
	pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs =  pAd->CommonCfg.DesiredHtPhy.MimoPs;
	pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20 =  (pAd->CommonCfg.DesiredHtPhy.ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 =  (pAd->CommonCfg.DesiredHtPhy.ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC =  (pAd->CommonCfg.DesiredHtPhy.TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC =  (pAd->CommonCfg.DesiredHtPhy.RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);
	pAd->MlmeAux.HtCapability.HtCapParm.MaxRAmpduFactor = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor;
    pAd->MlmeAux.HtCapability.HtCapParm.MpduDensity = pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity;
	pAd->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	pAd->MacTab.Content[Wcid].HTCapability.ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	if (pAd->CommonCfg.bRdg)
	{
		pAd->MlmeAux.HtCapability.ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
        pAd->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = 1;
	}
	
    if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_20)
        pAd->MlmeAux.HtCapability.MCSSet[4] = 0x0;  /* BW20 can't transmit MCS32*/

#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgHwTxBfCap)
	    setETxBFCap(pAd, &pAd->MlmeAux.HtCapability.TxBFCap);
#endif /* TXBF_SUPPORT */

	COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCapability);
	return TRUE;
}
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
VOID RTMPUpdateMlmeRate(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR	MinimumRate;
	UCHAR	ProperMlmeRate; /*= RATE_54;*/
	UCHAR	i, j, RateIdx = 12; /*1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54*/
	BOOLEAN	bMatch = FALSE;

	switch (pAd->CommonCfg.PhyMode) 
	{
		case PHY_11B:
			ProperMlmeRate = RATE_11;
			MinimumRate = RATE_1;
			break;
		case PHY_11BG_MIXED:
#ifdef DOT11_N_SUPPORT
		case PHY_11ABGN_MIXED:
		case PHY_11BGN_MIXED:
#endif /* DOT11_N_SUPPORT */
			if ((pAd->MlmeAux.SupRateLen == 4) &&
				(pAd->MlmeAux.ExtRateLen == 0))
				/* B only AP*/
				ProperMlmeRate = RATE_11;
			else
				ProperMlmeRate = RATE_24;
			
			if (pAd->MlmeAux.Channel <= 14)
			MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		case PHY_11A:
#ifdef DOT11_N_SUPPORT
		case PHY_11N_2_4G:	/* rt2860 need to check mlmerate for 802.11n*/
		case PHY_11GN_MIXED:
		case PHY_11AGN_MIXED:
		case PHY_11AN_MIXED:
		case PHY_11N_5G:	
#endif /* DOT11_N_SUPPORT */
			ProperMlmeRate = RATE_24;
			MinimumRate = RATE_6;
			break;
		case PHY_11ABG_MIXED:
			ProperMlmeRate = RATE_24;
			if (pAd->MlmeAux.Channel <= 14)
			   MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		default: /* error*/
			ProperMlmeRate = RATE_1;
			MinimumRate = RATE_1;
			break;
	}

	for (i = 0; i < pAd->MlmeAux.SupRateLen; i++)
	{
		for (j = 0; j < RateIdx; j++)
		{
			if ((pAd->MlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
			{
				if (j == ProperMlmeRate)
				{
					bMatch = TRUE;
					break;
				}
			}			
		}

		if (bMatch)
			break;
	}

	if (bMatch == FALSE)
	{
	for (i = 0; i < pAd->MlmeAux.ExtRateLen; i++)
	{
		for (j = 0; j < RateIdx; j++)
		{
			if ((pAd->MlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j])
			{
					if (j == ProperMlmeRate)
					{
						bMatch = TRUE;
						break;
					}
			}				
		}
		
			if (bMatch)
			break;		
	}
	}

	if (bMatch == FALSE)
	{
		ProperMlmeRate = MinimumRate;
	}

	pAd->CommonCfg.MlmeRate = MinimumRate;
	pAd->CommonCfg.RtsRate = ProperMlmeRate;
	if (pAd->CommonCfg.MlmeRate >= RATE_6)
	{
		pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
		pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
	}
	else
	{
		pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
		pAd->CommonCfg.MlmeTransmit.field.MCS = pAd->CommonCfg.MlmeRate;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_CCK;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = pAd->CommonCfg.MlmeRate;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPUpdateMlmeRate ==>   MlmeTransmit = 0x%x  \n" , pAd->CommonCfg.MlmeTransmit.word));
}


CHAR RTMPAvgRssi(
	IN PRTMP_ADAPTER	pAd,
	IN RSSI_SAMPLE 		*pRssi)
{
	CHAR Rssi;

	if(pAd->Antenna.field.RxPath == 3)
	{
		Rssi = (pRssi->AvgRssi0 + pRssi->AvgRssi1 + pRssi->AvgRssi2)/3;
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		Rssi = (pRssi->AvgRssi0 + pRssi->AvgRssi1)>>1;
	}
	else
	{
		Rssi = pRssi->AvgRssi0;
	}

	return Rssi;
}


CHAR RTMPMaxRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2)
{
	CHAR	larger = -127;
	
	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 <= 0))
	{
		larger = Rssi0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 <= 0))
	{
		larger = max(Rssi0, Rssi1);
	}
	
	if ((pAd->Antenna.field.RxPath == 3) && (Rssi2 <= 0))
	{
		larger = max(larger, Rssi2);
	}

	if (larger == -127)
		larger = 0;

	return larger;
}


CHAR RTMPMinSnr(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Snr0,
	IN CHAR				Snr1)
{
	CHAR	smaller = Snr0;
	
	if (pAd->Antenna.field.RxPath == 1) 
	{
		smaller = Snr0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Snr1 != 0))
	{
		smaller = min(Snr0, Snr1);
	}
 
	return smaller;
}

/*
    ========================================================================
    Routine Description:
        Adjust frequency offset when do channel switching or frequency calabration.
        
    Arguments:
        pAd         		- Adapter pointer
        pRefFreqOffset	in: referenced Frequency offset   out: adjusted frequency offset
        
    Return Value:
        None
        
    ========================================================================
*/
BOOLEAN RTMPAdjustFrequencyOffset(
	IN PRTMP_ADAPTER    pAd,
	INOUT PUCHAR pRefFreqOffset)
{
	BOOLEAN RetVal = TRUE;
	UCHAR RFValue = 0; 
	UCHAR PreRFValue = 0; 
	UCHAR FreqOffset = 0;
	UCHAR HighCurrentBit = 0;
	
	RTMP_ReadRF(pAd, RF_R17, &FreqOffset, &HighCurrentBit, 0x7F);
	PreRFValue =  HighCurrentBit | FreqOffset;
	FreqOffset = min((*pRefFreqOffset & 0x7F), 0x5F);
	RFValue = HighCurrentBit | FreqOffset;
	if (PreRFValue != RFValue)
	{
		RetVal = (RT30xxWriteRFRegister(pAd, RF_R17, RFValue) == STATUS_SUCCESS ? TRUE:FALSE);
	}

	if (RetVal == FALSE)
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): Error in tuning frequency offset !!\n", __FUNCTION__));
	else
		*pRefFreqOffset = FreqOffset;

	return RetVal;

}

/*
    ========================================================================
    Routine Description:
        Periodic evaluate antenna link status
        
    Arguments:
        pAd         - Adapter pointer
        
    Return Value:
        None
        
    ========================================================================
*/
VOID AsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
#ifdef CONFIG_STA_SUPPORT
	UCHAR	BBPR3 = 0;
#endif /* CONFIG_STA_SUPPORT */

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */


	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS	|
							fRTMP_ADAPTER_HALT_IN_PROGRESS	|
							fRTMP_ADAPTER_RADIO_OFF			|
							fRTMP_ADAPTER_NIC_NOT_EXIST		|
							fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) ||
							OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) 
							)
		return;
	

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
			/* for SmartBit 64-byte stream test */
			if (pAd->MacTab.Size > 0)
				APAsicEvaluateRxAnt(pAd);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{

			if (pAd->StaCfg.Psm == PWR_SAVE)
				return;

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
			BBPR3 &= (~0x18);
			if(pAd->Antenna.field.RxPath == 3)
			{
				BBPR3 |= (0x10);
			}
			else if(pAd->Antenna.field.RxPath == 2)
			{
				BBPR3 |= (0x8);
			}
			else if(pAd->Antenna.field.RxPath == 1)
			{
				BBPR3 |= (0x0);
			}
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
#ifdef RTMP_MAC_PCI
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	    		pAd->StaCfg.BBPR3 = BBPR3;
#endif /* RTMP_MAC_PCI */
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
#ifdef P2P_SUPPORT
				 || P2P_GO_ON(pAd) || P2P_CLI_ON(pAd)
#endif /* P2P_SUPPORT */
			)
			{
				ULONG	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + 
									pAd->RalinkCounters.OneSecTxRetryOkCount + 
									pAd->RalinkCounters.OneSecTxFailCount;

				/* dynamic adjust antenna evaluation period according to the traffic*/
				if (TxTotalCnt > 50)
				{
					RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
					pAd->Mlme.bLowThroughput = FALSE;
				}
				else
				{
					RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
					pAd->Mlme.bLowThroughput = TRUE;
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}
}

/*
    ========================================================================
    Routine Description:
        After evaluation, check antenna link status
        
    Arguments:
        pAd         - Adapter pointer
        
    Return Value:
        None
        
    ========================================================================
*/
VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)FunctionContext;
#ifdef CONFIG_STA_SUPPORT
	UCHAR			BBPR3 = 0;
	CHAR			larger = -127, rssi0, rssi1, rssi2;
#endif /* CONFIG_STA_SUPPORT */



#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS	|
							fRTMP_ADAPTER_HALT_IN_PROGRESS	|
							fRTMP_ADAPTER_RADIO_OFF			|
							fRTMP_ADAPTER_NIC_NOT_EXIST) ||
							OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) 
							)
		return;

	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
/*			if (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_DISABLE)*/
				APAsicRxAntEvalTimeout(pAd);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (pAd->StaCfg.Psm == PWR_SAVE)
				return;


			/* if the traffic is low, use average rssi as the criteria*/
			if (pAd->Mlme.bLowThroughput == TRUE)
			{
				rssi0 = pAd->StaCfg.RssiSample.LastRssi0;
				rssi1 = pAd->StaCfg.RssiSample.LastRssi1;
				rssi2 = pAd->StaCfg.RssiSample.LastRssi2;
			}
			else
			{
				rssi0 = pAd->StaCfg.RssiSample.AvgRssi0;
				rssi1 = pAd->StaCfg.RssiSample.AvgRssi1;
				rssi2 = pAd->StaCfg.RssiSample.AvgRssi2;
			}

			if(pAd->Antenna.field.RxPath == 3)
			{
				larger = max(rssi0, rssi1);
#ifdef DOT11N_SS3_SUPPORT
				if (pAd->CommonCfg.TxStream >= 3)
				{
					pAd->Mlme.RealRxPath = 3;
				}
				else
#endif /* DOT11N_SS3_SUPPORT */
				if (larger > (rssi2 + 20))
					pAd->Mlme.RealRxPath = 2;
				else
					pAd->Mlme.RealRxPath = 3;
			}
			else if(pAd->Antenna.field.RxPath == 2)
			{
				if (rssi0 > (rssi1 + 20))
					pAd->Mlme.RealRxPath = 1;
				else
					pAd->Mlme.RealRxPath = 2;
			}

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
			BBPR3 &= (~0x18);
			if(pAd->Mlme.RealRxPath == 3)
			{
				BBPR3 |= (0x10);
			}
			else if(pAd->Mlme.RealRxPath == 2)
			{
				BBPR3 |= (0x8);
			}
			else if(pAd->Mlme.RealRxPath == 1)
			{
				BBPR3 |= (0x0);
			}
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
#ifdef RTMP_MAC_PCI    
			pAd->StaCfg.BBPR3 = BBPR3;
#endif /* RTMP_MAC_PCI */
		}
#endif /* CONFIG_STA_SUPPORT */
	}
}


VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) &&
		!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
		return;

	pAd->CommonCfg.TriggerTimerCount++;

/* Driver should not send trigger frame, it should be send by application layer*/
/*
	if (pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable
		&& (pAd->CommonCfg.bNeedSendTriggerFrame ||
		(((pAd->CommonCfg.TriggerTimerCount%20) == 19) && (!pAd->CommonCfg.bAPSDAC_BE || !pAd->CommonCfg.bAPSDAC_BK || !pAd->CommonCfg.bAPSDAC_VI || !pAd->CommonCfg.bAPSDAC_VO))))
	{
		DBGPRINT(RT_DEBUG_TRACE,("Sending trigger frame and enter service period when support APSD\n"));
		RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE);
		pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
		pAd->CommonCfg.TriggerTimerCount = 0;
		pAd->CommonCfg.bInServicePeriod = TRUE;
	}*/
}

/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter
        
    Arguments:
        pAd         - Adapter pointer
        bPiggyBack  - Enable / Disable Piggy-Back

    Return Value:
        None
        
    ========================================================================
*/
VOID RTMPSetPiggyBack(
    IN PRTMP_ADAPTER    pAd,
    IN BOOLEAN          bPiggyBack)
{
	TX_LINK_CFG_STRUC  TxLinkCfg;
    
	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);

	TxLinkCfg.field.TxCFAckEn = bPiggyBack;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
}

/*
    ========================================================================
    Routine Description:
        check if this entry need to switch rate automatically
        
    Arguments:
        pAd         
        pEntry 	 	

    Return Value:
        TURE
        FALSE
        
    ========================================================================
*/
BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry)	
{
	BOOLEAN		result = TRUE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	if (pEntry)
	{
		if (IS_ENTRY_CLIENT(pEntry))
			result = pAd->ApCfg.MBSSID[pEntry->apidx].bAutoTxRateSwitch;
#ifdef WDS_SUPPORT
		else if (IS_ENTRY_WDS(pEntry))
			result = pAd->WdsTab.WdsEntry[pEntry->MatchWDSTabIdx].bAutoTxRateSwitch;
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		else if (IS_ENTRY_APCLI(pEntry))
			result = pAd->ApCfg.ApCliTab[pEntry->MatchAPCLITabIdx].bAutoTxRateSwitch;
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* only associated STA counts*/
#ifdef P2P_SUPPORT
		if (pEntry && IS_P2P_GO_ENTRY(pEntry))
			result = pAd->ApCfg.MBSSID[pEntry->apidx].bAutoTxRateSwitch;
		else if (pEntry && IS_ENTRY_APCLI(pEntry))
			result = pAd->ApCfg.ApCliTab[pEntry->MatchAPCLITabIdx].bAutoTxRateSwitch;
		else
#endif /* P2P_SUPPORT */
		if ((pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
#ifdef QOS_DLS_SUPPORT
			|| (pEntry && IS_ENTRY_DLS(pEntry))
#endif /* QOS_DLS_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
			|| (pEntry && IS_ENTRY_TDLS(pEntry))
#endif /* DOT11Z_TDLS_SUPPORT */
			)
		{
			result = pAd->StaCfg.bAutoTxRateSwitch;
		}
		else
			result = FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */



	return result;
}


BOOLEAN RTMPAutoRateSwitchCheck(
	IN PRTMP_ADAPTER    pAd)	
{			
#ifdef CONFIG_AP_SUPPORT		
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		INT	apidx = 0;
	
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
			if (pAd->ApCfg.MBSSID[apidx].bAutoTxRateSwitch)
				return TRUE;
		}			
#ifdef WDS_SUPPORT
		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++)
		{
			if (pAd->WdsTab.WdsEntry[apidx].bAutoTxRateSwitch)
				return TRUE;
		}		
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++)
		{
			if (pAd->ApCfg.ApCliTab[apidx].bAutoTxRateSwitch)
				return TRUE;
		}		
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->StaCfg.bAutoTxRateSwitch)
			return TRUE;
	}
#endif /* CONFIG_STA_SUPPORT */
	return FALSE;
}


/*
    ========================================================================
    Routine Description:
        check if this entry need to fix tx legacy rate
        
    Arguments:
        pAd         
        pEntry 	 	

    Return Value:
        TURE
        FALSE
        
    ========================================================================
*/
UCHAR RTMPStaFixedTxMode(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry)	
{
	UCHAR	tx_mode = FIXED_TXMODE_HT;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	if (pEntry)
	{
		if (IS_ENTRY_CLIENT(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.MBSSID[pEntry->apidx].DesiredTransmitSetting.field.FixedTxMode;
#ifdef WDS_SUPPORT
		else if (IS_ENTRY_WDS(pEntry))
			tx_mode = (UCHAR)pAd->WdsTab.WdsEntry[pEntry->MatchWDSTabIdx].DesiredTransmitSetting.field.FixedTxMode;
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		else if (IS_ENTRY_APCLI(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.ApCliTab[pEntry->MatchAPCLITabIdx].DesiredTransmitSetting.field.FixedTxMode;
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef P2P_SUPPORT
		if (IS_P2P_GO_ENTRY(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.MBSSID[pEntry->apidx].DesiredTransmitSetting.field.FixedTxMode;
		else if (IS_ENTRY_APCLI(pEntry))
			tx_mode = (UCHAR)pAd->ApCfg.ApCliTab[pEntry->MatchAPCLITabIdx].DesiredTransmitSetting.field.FixedTxMode;
		else
#endif /* P2P_SUPPORT */
		tx_mode = (UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode;
	}
#endif /* CONFIG_STA_SUPPORT */

	return tx_mode;
}

/*
    ========================================================================
    Routine Description:
        Overwrite HT Tx Mode by Fixed Legency Tx Mode, if specified.
        
    Arguments:
        pAd         
        pEntry 	 	

    Return Value:
        TURE
        FALSE
        
    ========================================================================
*/
VOID RTMPUpdateLegacyTxSetting(
		UCHAR				fixed_tx_mode,
		PMAC_TABLE_ENTRY	pEntry)
{
	HTTRANSMIT_SETTING TransmitSetting;
	
	if (fixed_tx_mode == FIXED_TXMODE_HT)
		return;
							 				
	TransmitSetting.word = 0;

	TransmitSetting.field.MODE = pEntry->HTPhyMode.field.MODE;
	TransmitSetting.field.MCS = pEntry->HTPhyMode.field.MCS;
						
	if (fixed_tx_mode == FIXED_TXMODE_CCK)
	{
		TransmitSetting.field.MODE = MODE_CCK;
		/* CCK mode allow MCS 0~3*/
		if (TransmitSetting.field.MCS > MCS_3)
			TransmitSetting.field.MCS = MCS_3;
	}
	else 
	{
		TransmitSetting.field.MODE = MODE_OFDM;
		/* OFDM mode allow MCS 0~7*/
		if (TransmitSetting.field.MCS > MCS_7)
			TransmitSetting.field.MCS = MCS_7;
	}
	
	if (pEntry->HTPhyMode.field.MODE >= TransmitSetting.field.MODE)
	{
		pEntry->HTPhyMode.word = TransmitSetting.word;
		DBGPRINT(RT_DEBUG_TRACE, ("RTMPUpdateLegacyTxSetting : wcid-%d, MODE=%s, MCS=%d \n", 
				pEntry->Aid, GetPhyMode(pEntry->HTPhyMode.field.MODE), pEntry->HTPhyMode.field.MCS));		
	}													
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : the fixed TxMode is invalid \n", __FUNCTION__));	
	}
}

#ifdef CONFIG_STA_SUPPORT
/*
	==========================================================================
	Description:
		dynamic tune BBP R66 to find a balance between sensibility and 
		noise isolation

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID AsicStaBbpTuning(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR	OrigR66Value = 0, R66;/*, R66UpperBound = 0x30, R66LowerBound = 0x30;*/
	CHAR	Rssi;
#ifdef RT2883
	UCHAR	byteValue = 0;
#endif /* RT2883 */

	/* 2860C did not support Fase CCA, therefore can't tune*/
	if (pAd->MACVersion == 0x28600100)
		return;

	
	/* work as a STA*/
	if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)  /* no R66 tuning when SCANNING*/
		return;

	if ((pAd->OpMode == OPMODE_STA) 
		&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
			)
		&& !(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
#ifdef RTMP_MAC_PCI		
		&& (pAd->bPCIclkOff == FALSE)
#endif /* RTMP_MAC_PCI */
		)
	{
		AsicBBPReadWithRxChain(pAd, BBP_R66, &OrigR66Value, RX_CHAIN_0);
		R66 = OrigR66Value;
		
		if (pAd->Antenna.field.RxPath > 1)
			Rssi = (pAd->StaCfg.RssiSample.AvgRssi0 + pAd->StaCfg.RssiSample.AvgRssi1) >> 1;
		else
			Rssi = pAd->StaCfg.RssiSample.AvgRssi0;

		RTMP_CHIP_ASIC_AGC_ADJUST(pAd, Rssi, R66);

		// TODO: shiang,I didn't find  AsicAGCAdjust for RT30xx, so I move following code from upper #if case.


	}
}
#endif /* CONFIG_STA_SUPPORT */

VOID RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BandWidth)
{
	RTMP_CHIP_ASIC_AGC_INIT(pAd, BandWidth);

}


/*
========================================================================
Routine Description:
	Check if the channel has the property.

Arguments:
	pAd				- WLAN control block pointer
	ChanNum			- channel number
	Property		- channel property, CHANNEL_PASSIVE_SCAN, etc.

Return Value:
	TRUE			- YES
	FALSE			- NO

Note:
========================================================================
*/
BOOLEAN CHAN_PropertyCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UINT32			ChanNum,
	IN UCHAR			Property)
{
	UINT32 IdChan;


	/* look for all registered channels */
	for(IdChan=0; IdChan<pAd->ChannelListNum; IdChan++)
	{
		if (pAd->ChannelList[IdChan].Channel == ChanNum)
		{
			if ((pAd->ChannelList[IdChan].Flags & Property) == Property)
				return TRUE; /* same property */
			/* End of if */

			break;
		} /* End of if */
	} /* End of for */

	return FALSE;
}




/* End of mlme.c */

