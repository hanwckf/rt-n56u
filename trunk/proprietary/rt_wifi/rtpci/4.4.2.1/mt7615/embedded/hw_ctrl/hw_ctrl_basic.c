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
	hw_ctrl_basic.c
*/
#include "rt_config.h"
#include "hw_ctrl_basic.h"

extern HW_CMD_TABLE_T *HwCmdTable[];
extern HW_FLAG_TABLE_T HwFlagTable[];

/*==========================================================/
 //	Basic Command API implement															/
/==========================================================*/
static inline HwCmdHdlr HwCtrlValidCmd(HwCmdQElmt *CmdQelmt)
{
	UINT CmdType =  CmdQelmt->type;
	UINT CmdIndex = CmdQelmt->command;
	SHORT CurIndex = 0;
	HwCmdHdlr Handler = NULL;
	HW_CMD_TABLE_T  *pHwTargetTable = NULL;

	if (CmdType >= HWCMD_TYPE_END) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("CMD TPYE(%u) OOB error! HWCMD_TYPE_END %u\n",
			  CmdType, HWCMD_TYPE_END));
		return NULL;
	}

	if (CmdIndex >= HWCMD_ID_END) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("CMD ID(%u) OOB error! HWCMD_ID_END %u\n",
			  CmdIndex, HWCMD_ID_END));
		return NULL;
	}

	pHwTargetTable = HwCmdTable[CmdType];
	if (!pHwTargetTable) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("No HwCmdTable entry for this CMD %u Type %u\n",
			  CmdIndex, CmdType));
		return NULL;
	}

	CurIndex= 0;
	do{
		if (pHwTargetTable[CurIndex].CmdID == CmdIndex)
		{
			Handler = pHwTargetTable[CurIndex].CmdHdlr;
			pHwTargetTable[CurIndex].RfCnt++;
			break;
		}
		CurIndex++;
	}while(pHwTargetTable[CurIndex].CmdHdlr !=NULL);

	if (Handler == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("No corresponding CMDHdlr for this CMD %u Type %u\n",
			  CmdIndex, CmdType));
	}
	return Handler;
}


static inline HwFlagHdlr HwCtrlValidFlag(PHwFlagCtrl pHwCtrlFlag)
{
	SHORT CurIndex = 0;
	HwFlagHdlr Handler = NULL;

	if(pHwCtrlFlag->FlagId > (1 << HWFLAG_ID_END))
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("FLAG ID(%x) is out of boundary\n", pHwCtrlFlag->FlagId));
		pHwCtrlFlag->FlagId = 0;
		return NULL;
	}

	CurIndex= 0;
	do{
		if (HwFlagTable[CurIndex].FlagId & pHwCtrlFlag->FlagId)
		{
			Handler = HwFlagTable[CurIndex].FlagHdlr;
			/*Unmask flag*/
			pHwCtrlFlag->FlagId&=~(HwFlagTable[CurIndex].FlagId);
			HwFlagTable[CurIndex].RfCnt++;
			break;
		}
		CurIndex++;
	}while(HwFlagTable[CurIndex].FlagHdlr!=NULL);

	if(Handler == NULL)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("No corresponding FlagHdlr for this FlagID(%x)\n",  pHwCtrlFlag->FlagId));
		pHwCtrlFlag->FlagId = 0;
	}
	return Handler;
}


static VOID HwCtrlFreeCmd(RTMP_ADAPTER *pAd,HwCmdQElmt *pCmdQlmt)
{
	if(pCmdQlmt->NeedWait)
	{
		RTMP_OS_EXIT_COMPLETION(&pCmdQlmt->ack_done);
	}

	if ((pCmdQlmt->buffer != NULL) && (pCmdQlmt->bufferlength != 0))
	{
		os_free_mem(pCmdQlmt->buffer);
	}
	os_free_mem(pCmdQlmt);
}


static VOID HwCtrlDequeueCmd(HwCmdQ *cmdq,HwCmdQElmt **pcmdqelmt)
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


static VOID HwCtrlCmdHandler(RTMP_ADAPTER *pAd)
{
	PHwCmdQElmt	cmdqelmt;
	NDIS_STATUS	NdisStatus = NDIS_STATUS_SUCCESS;
	NTSTATUS		ntStatus;
	HwCmdHdlr 		Handler = NULL;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;

	while (pAd && pHwCtrl->HwCtrlQ.size > 0)
	{
		NdisStatus = NDIS_STATUS_SUCCESS;

		NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &cmdqelmt);
		NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

		if (cmdqelmt == NULL)
			break;


		if(!RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_NIC_NOT_EXIST) && RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_START_UP))
		{
			Handler = HwCtrlValidCmd(cmdqelmt);
			if(Handler)
			{
				ntStatus = Handler(pAd,cmdqelmt);
				if(cmdqelmt->CallbackFun)
				{
					cmdqelmt->CallbackFun(pAd,cmdqelmt->CallbackArgs);
				}
			}
		}
		if(cmdqelmt->NeedWait)
		{
			RTMP_SEM_LOCK(&cmdqelmt->lock);
			
			if (cmdqelmt->status == HWCTRL_STATUS_TIMEOUT)
			{	
				RTMP_SEM_UNLOCK(&cmdqelmt->lock);
				HwCtrlFreeCmd(pAd,cmdqelmt);
			}
			else
			{
				RTMP_SEM_UNLOCK(&cmdqelmt->lock);
				RTMP_OS_COMPLETE(&cmdqelmt->ack_done);
			}
		}
		else
		{
			HwCtrlFreeCmd(pAd,cmdqelmt);
		}
	}	/* end of while */
}


static VOID HwCtrlFlagHandler(RTMP_ADAPTER *pAd)
{
	HW_CTRL_T 		*pHwCtrl = &pAd->HwCtrl;
	PHwFlagCtrl		pHwCtrlFlag = &pHwCtrl->HwCtrlFlag;
	NTSTATUS		ntStatus;
	HwFlagHdlr 		Handler = NULL;
    HwFlagCtrl      HwCtrlFlag;

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
    HwCtrlFlag.FlagId = pHwCtrlFlag->FlagId;
    HwCtrlFlag.IsFlagSet = pHwCtrlFlag->IsFlagSet;
    pHwCtrlFlag->FlagId = 0;
    pHwCtrlFlag->IsFlagSet = 0;
    NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) )
	{
		while(HwCtrlFlag.FlagId)
		{
			Handler = HwCtrlValidFlag(&HwCtrlFlag);
			if(Handler)
			{
				ntStatus = Handler(pAd);
			}
		}
	}
}


static INT HwCtrlThread(ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	HwCmdQElmt	*pCmdQElmt = NULL;
	HW_CTRL_T *pHwCtrl;
	int status;
	status = 0;


	pTask = (RTMP_OS_TASK *)Context;

	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		return 0;

	pHwCtrl = &pAd->HwCtrl;

	RtmpOSTaskCustomize(pTask);

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	while (pHwCtrl->HwCtrlQ.CmdQState == RTMP_TASK_STAT_RUNNING)
	{
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pHwCtrl->HwCtrlQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

		/*support flag type*/
		if(pHwCtrl->HwCtrlFlag.IsFlagSet)
		{
			HwCtrlFlagHandler(pAd);
		}
		/*every time check command formate event*/
		HwCtrlCmdHandler(pAd);
		pHwCtrl->TotalCnt++;
	}


	/* Clear the CmdQElements. */
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_STOPED;
	while(pHwCtrl->HwCtrlQ.size)
	{
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &pCmdQElmt);
		if (pCmdQElmt)
		{
			if(pCmdQElmt->NeedWait)
				RTMP_OS_COMPLETE(&pCmdQElmt->ack_done);
			else
				HwCtrlFreeCmd(pAd,pCmdQElmt);
		}
	}
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,( "<---%s\n",__FUNCTION__));

	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}


#ifdef ERR_RECOVERY
static INT ser_ctrl_task(ULONG context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *task;
	HW_CTRL_T *hw_ctrl;
	int status = 0;


	task = (RTMP_OS_TASK *)context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(task);
	if (pAd == NULL)
		return 0;

	hw_ctrl = &pAd->HwCtrl;

	RtmpOSTaskCustomize(task);

	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);

	while (task && !RTMP_OS_TASK_IS_KILLED(task))
	{
		if (RtmpOSTaskWait(pAd, task, &status) == FALSE)
			break;

		HwRecoveryFromError(pAd);
	}

	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_UNKNOWN;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);

	status = RtmpOSTaskNotifyToExit(task);

	return status;
}


INT ser_init(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	RTMP_OS_TASK *task = &hw_ctrl->ser_task;
	

	NdisAllocateSpinLock(pAd, &hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_INITED;
	RTMP_OS_TASK_INIT(task, "ser_task", pAd);
	Status = RtmpOSTaskAttach(task, ser_ctrl_task, (ULONG)task);
	if (Status == NDIS_STATUS_FAILURE)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s: unable to start %s\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev),__FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	return TRUE;

}


INT ser_exit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	
	/*kill task*/
	ret = RtmpOSTaskKill(&hw_ctrl->ser_task);	
	NdisFreeSpinLock(&hw_ctrl->ser_lock);

	return ret;
}
#endif /* ERR_RECOVERY */


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
UINT32 HwCtrlInit(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HwCmdQ *cmdq = &pHwCtrl->HwCtrlQ;
	RTMP_OS_TASK *pTask = &pHwCtrl->HwCtrlTask;

	NdisAllocateSpinLock(pAd, &pHwCtrl->HwCtrlQLock);

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	pHwCtrl->TotalCnt = 0;
	pTask = &pHwCtrl->HwCtrlTask;
	RTMP_OS_TASK_INIT(pTask, "HwCtrlTask", pAd);
	Status = RtmpOSTaskAttach(pTask, HwCtrlThread, (ULONG)pTask);
	if (Status == NDIS_STATUS_FAILURE)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unable to start %s\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev),__FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

#ifdef ERR_RECOVERY
	Status = ser_init(pAd);
#endif /* ERR_RECOVERY */

	return NDIS_STATUS_SUCCESS;
}


VOID HwCtrlExit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	/*flush all queued command*/
	HwCtrlCmdHandler(pAd);
	/*kill task*/
	ret = RtmpOSTaskKill(&pHwCtrl->HwCtrlTask);
	NdisFreeSpinLock(&pHwCtrl->HwCtrlQLock);

#ifdef ERR_RECOVERY
	ret = ser_exit(pAd);
#endif /* ERR_RECOVERY */
}

NDIS_STATUS HwCtrlEnqueueCmd(
	RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PHwCmdQElmt	cmdqelmt = NULL;
	PHwCmdQ	cmdq = NULL;
	ULONG  flag;
	UINT32 wait_time = 0;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s - NIC is not exist!!\n",__FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt, sizeof(HwCmdQElmt));

	if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt == NULL))
	{
		return (NDIS_STATUS_RESOURCES);
	}

	NdisZeroMemory(cmdqelmt, sizeof(HwCmdQElmt));

	/*initial lock*/
	NdisAllocateSpinLock(NULL,&cmdqelmt->lock);
	cmdqelmt->status = HWCTRL_STATUS_OK;
	/*creat wait */
	cmdqelmt->NeedWait = HwCtrlTxd.NeedWait;
	if(HwCtrlTxd.NeedWait)
	{
		RTMP_OS_INIT_COMPLETION(&cmdqelmt->ack_done);
	}

	if(HwCtrlTxd.InformationBufferLength > 0)
	{
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, HwCtrlTxd.InformationBufferLength);
		if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt->buffer == NULL))
		{
			os_free_mem(cmdqelmt);
			return (NDIS_STATUS_RESOURCES);
		}
		else
		{
			os_move_mem(cmdqelmt->buffer, HwCtrlTxd.pInformationBuffer, HwCtrlTxd.InformationBufferLength);
			cmdqelmt->bufferlength = HwCtrlTxd.InformationBufferLength;
		}
	}
	else
	{
		cmdqelmt->buffer = NULL;
		cmdqelmt->bufferlength = 0;
	}

	cmdqelmt->command = HwCtrlTxd.CmdId;
	cmdqelmt->type = HwCtrlTxd.CmdType;
	cmdqelmt->RspBuffer = HwCtrlTxd.pRespBuffer;
	cmdqelmt->RspBufferLen = HwCtrlTxd.RespBufferLength;
	cmdqelmt->CallbackFun = HwCtrlTxd.CallbackFun;
	cmdqelmt->CallbackArgs = HwCtrlTxd.CallbackArgs;

	if (cmdqelmt != NULL)
	{
		RTMP_SPIN_LOCK_IRQSAVE(&pHwCtrl->HwCtrlQLock,&flag);
		if (pHwCtrl->HwCtrlQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)
		{
			cmdq = &pHwCtrl->HwCtrlQ;
			if (cmdq->size == 0)
			{
				cmdq->head = cmdqelmt;
			}else
			{
				cmdq->tail->next = cmdqelmt;
			}
			cmdq->tail = cmdqelmt;
			cmdqelmt->next = NULL;
			cmdq->size++;
			status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			status = NDIS_STATUS_FAILURE;
		}
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pHwCtrl->HwCtrlQLock,&flag);

		if (status == NDIS_STATUS_FAILURE)
		{
			if (cmdqelmt->buffer)
			{
				os_free_mem(cmdqelmt->buffer);
			}
			os_free_mem(cmdqelmt);
		}
		else
		{
			/*Send command*/
			RTCMDUp(&pHwCtrl->HwCtrlTask);
			if(HwCtrlTxd.NeedWait)
			{
				wait_time = HwCtrlTxd.wait_time ? HwCtrlTxd.wait_time : HWCTRL_CMD_TIMEOUT;
				if(!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&cmdqelmt->ack_done, RTMPMsecsToJiffies(wait_time)))
				{
					status = NDIS_STATUS_FAILURE;
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): HwCtrl CmdTimeout, TYPE:%d,ID:%d!!\n",__FUNCTION__,
					cmdqelmt->type,cmdqelmt->command));
					RTMP_SEM_LOCK(&cmdqelmt->lock);
					cmdqelmt->status = HWCTRL_STATUS_TIMEOUT;
					RTMP_SEM_UNLOCK(&cmdqelmt->lock);					
				}else
				{
					/*if need wait, free cmd msg here*/
					HwCtrlFreeCmd(pAd,cmdqelmt);
				}

			}
		}
	}
	return(status);
}


NDIS_STATUS HwCtrlSetFlag(
	RTMP_ADAPTER	*pAd,
	INT32			FlagId)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s - NIC is not exist!!\n",__FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlFlag.IsFlagSet = TRUE;
	pHwCtrl->HwCtrlFlag.FlagId |= FlagId;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	/*Send command*/
	RTCMDUp(&pHwCtrl->HwCtrlTask);

	return(status);
}



/*
*
*/
INT Show_HwCtrlStatistic_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HW_CMD_TABLE_T *pHwCmdTable = NULL;
	UCHAR i=0,j=0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHwCtrlTask Totaol Ref. Cnt: %d\n",pHwCtrl->TotalCnt));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHwCtrlTask CMD Statistic:\n"));

	pHwCmdTable = HwCmdTable[i];
	while(pHwCmdTable!=NULL)
	{
		j=0;
		while(pHwCmdTable[j].CmdID!=HWCMD_ID_END)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCMDID: %d, Handler: %p, RfCnt: %d\n",
				pHwCmdTable[j].CmdID,pHwCmdTable[j].CmdHdlr,pHwCmdTable[j].RfCnt));
			j++;
		}
		pHwCmdTable = HwCmdTable[++i];
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHwCtrlTask Flag Statistic:\n"));

	i=0;
	while(HwFlagTable[i].FlagId!=HWFLAG_ID_END)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFLAGID: %d, Handler: %p, RfCnt: %d\n",
			HwFlagTable[i].FlagId,HwFlagTable[i].FlagHdlr,HwFlagTable[i].RfCnt));
		i++;
	}

	return TRUE;
}

