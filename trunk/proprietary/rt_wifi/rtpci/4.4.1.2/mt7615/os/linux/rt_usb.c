#ifdef MTK_LICENSE
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
	rtusb_bulk.c

	Abstract:

	Revision History:
	Who			When		What
	--------	----------	----------------------------------------------
	Name		Date		Modification logs

*/
#endif /* MTK_LICENSE */
#include "rt_config.h"

#define MCAST_WCID_TO_REMOVE 0 //Pat: TODO

/*
========================================================================
Routine Description:
    Create kernel threads & tasklets.

Arguments:
    *net_dev			Pointer to wireless net device interface

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE

Note:
========================================================================
*/
NDIS_STATUS	 RtmpMgmtTaskInit(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;

	/*
		Creat TimerQ Thread, We need init timerQ related structure before create the timer thread.
	*/
	RtmpTimerQInit(pAd);

	pTask = &pAd->timerTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpTimerTask", pAd);
	status = RtmpOSTaskAttach(pTask, RtmpTimerQThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE)
	{
		printk (KERN_WARNING "%s: unable to start RtmpTimerQThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

	/* WCNCR00034259: init CmdQ resources before run thread */
	RtmpCmdQInit(pAd);

	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpCmdQTask", pAd);
	status = RtmpOSTaskAttach(pTask, RTUSBCmdThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE)
	{
		printk (KERN_WARNING "%s: unable to start RTUSBCmdThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

#ifdef WSC_INCLUDED
	/* start the crediential write task first. */
	WscThreadInit(pAd);
#endif /* WSC_INCLUDED */

	return NDIS_STATUS_SUCCESS;
}



/*
========================================================================
Routine Description:
    Close kernel threads.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
    NONE

Note:
========================================================================
*/
VOID RtmpMgmtTaskExit(
	IN RTMP_ADAPTER *pAd)
{
	INT			ret;
	RTMP_OS_TASK	*pTask;

	/* Sleep 50 milliseconds so pending io might finish normally */
	RtmpusecDelay(50000);

	/* We want to wait until all pending receives and sends to the */
	/* device object. We cancel any */
	/* irps. Wait until sends and receives have stopped. */
	RTUSBCancelPendingIRPs(pAd);

	RtmpCmdQExit(pAd); /* WCNCR00034259: unify CmdQ init and exit */

	/* Terminate cmdQ thread */
	pTask = &pAd->cmdQTask;
	RTMP_OS_TASK_LEGALITY(pTask)
	{
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);

		ret = RtmpOSTaskKill(pTask);
		if (ret == NDIS_STATUS_FAILURE)
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("kill command task failed!\n"));
		}
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
	}

	/* We need clear timerQ related structure before exits of the timer thread. */
	RtmpTimerQExit(pAd);

	/* Terminate timer thread */
	pTask = &pAd->timerTask;
	ret = RtmpOSTaskKill(pTask);
	if (ret == NDIS_STATUS_FAILURE)
	{
/*		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("%s: kill task(%s) failed!\n", */
/*					RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev), pTask->taskName)); */
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("kill timer task failed!\n"));
	}

#ifdef WSC_INCLUDED
	WscThreadExit(pAd);
#endif /* WSC_INCLUDED */

}


static void rtusb_dataout_complete(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	purbb_t				pUrb;
	POS_COOKIE			pObj;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId;
	NTSTATUS			Status;
	unsigned long		IrqFlags;


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status			= RTMP_USB_URB_STATUS_GET(pUrb);
	pAd				= pHTTXContext->pAd;
	pObj 			= (POS_COOKIE) pAd->OS_Cookie;
/*	Status			= pUrb->status; */

	/* Store BulkOut PipeId */
	BulkOutPipeId = pHTTXContext->BulkOutPipeId;
	pAd->BulkOutDataOneSecCount++;

	/*MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_LOUD, ("Done-B(%d):I=0x%lx, CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n", BulkOutPipeId, in_interrupt(), pHTTXContext->CurWritePosition, */
	/*		pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad)); */

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutPending[BulkOutPipeId] = FALSE;
	pHTTXContext->IRPPending = FALSE;
	pAd->watchDogTxPendingCnt[BulkOutPipeId] = 0;

	if (Status == USB_ST_NOERROR)
	{
		pAd->BulkOutComplete++;

		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		pAd->Counters8023.GoodTransmits++;
		/*RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */
		FREE_HTTX_RING(pAd, BulkOutPipeId, pHTTXContext);
		/*RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */

#ifdef UAPSD_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		UAPSD_UnTagFrame(pAd, BulkOutPipeId, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition);
#else
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_UnTagFrame(pAd, BulkOutPipeId, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition);
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */

	}
	else	/* STATUS_OTHER */
	{
		PUCHAR	pBuf;

		pAd->BulkOutCompleteOther++;

		pBuf = &pHTTXContext->TransferBuffer->field.WirelessPacket[pHTTXContext->NextBulkOutPosition];

		if (!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_NIC_NOT_EXIST |
									fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = BulkOutPipeId;
			pAd->bulkResetReq[BulkOutPipeId] = pAd->BulkOutReq;
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("BulkOutDataPacket failed: ReasonCode=%d!\n", Status));
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("\t>>BulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n", pAd->BulkOutReq, pAd->BulkOutComplete, pAd->BulkOutCompleteOther));
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("\t>>BulkOut Header:%x %x %x %x %x %x %x %x\n", pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5], pBuf[6], pBuf[7]));
		/*MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, (">>BulkOutCompleteCancel=0x%x, BulkOutCompleteOther=0x%x\n", pAd->BulkOutCompleteCancel, pAd->BulkOutCompleteOther)); */

	}

	/* */
	/* bInUse = TRUE, means some process are filling TX data, after that must turn on bWaitingBulkOut */
	/* bWaitingBulkOut = TRUE, means the TX data are waiting for bulk out. */
	/* */
	/*RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */
	if (((pHTTXContext->ENextBulkOutPosition != pHTTXContext->CurWritePosition) &&
		(pHTTXContext->ENextBulkOutPosition != (pHTTXContext->CurWritePosition+8)) &&
		!RTUSB_TEST_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId)))
#ifdef USB_BULK_BUF_ALIGMENT
				||	(pHTTXContext->NextBulkIdx != pHTTXContext->CurWriteIdx)
#endif /* USB_BULK_BUF_ALIGMENT */
	)
	{
		/* Indicate There is data avaliable */
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
	}
	/*RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */

	/* Always call Bulk routine, even reset bulk. */
	/* The protection of rest bulk should be in BulkOut routine */
	RTUSBKickBulkOut(pAd);
}

#ifdef MULTI_WMM_SUPPORT
static void rtusb_wmm1_dataout_complete(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	purbb_t				pUrb;
	POS_COOKIE			pObj;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId;
	NTSTATUS			Status;
	unsigned long		IrqFlags;


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status			= RTMP_USB_URB_STATUS_GET(pUrb);
	pAd				= pHTTXContext->pAd;
	pObj 			= (POS_COOKIE) pAd->OS_Cookie;
/*	Status			= pUrb->status; */

	/* Store BulkOut PipeId */
	BulkOutPipeId = pHTTXContext->BulkOutPipeId - EDCA_WMM1_AC0_PIPE;
	pAd->BulkOutDataOneSecCount++;

	/*MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_LOUD, ("Done-B(%d):I=0x%lx, CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n", BulkOutPipeId, in_interrupt(), pHTTXContext->CurWritePosition, */
	/*		pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad)); */

	RTMP_IRQ_LOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutWmm1Pending[BulkOutPipeId] = FALSE;
	pHTTXContext->IRPPending = FALSE;
	pAd->watchDogWmm1TxPendingCnt[BulkOutPipeId] = 0;

	if (Status == USB_ST_NOERROR)
	{
		pAd->BulkOutComplete++;

		RTMP_IRQ_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);

		pAd->Counters8023.GoodTransmits++;
		/*RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */
		FREE_HTTX_RING(pAd, BulkOutPipeId, pHTTXContext);
		/*RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */
	}
	else	/* STATUS_OTHER */
	{
		PUCHAR	pBuf;

		pAd->BulkOutCompleteOther++;

		pBuf = &pHTTXContext->TransferBuffer->field.WirelessPacket[pHTTXContext->NextBulkOutPosition];

		if (!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_NIC_NOT_EXIST |
									fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = BulkOutPipeId;
			pAd->bulkResetReq[BulkOutPipeId] = pAd->BulkOutReq;
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);

		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("BulkOutDataPacket failed: ReasonCode=%d!\n", Status));
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("\t>>BulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n", pAd->BulkOutReq, pAd->BulkOutComplete, pAd->BulkOutCompleteOther));
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("\t>>BulkOut Header:%x %x %x %x %x %x %x %x\n", pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5], pBuf[6], pBuf[7]));
		/*MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, (">>BulkOutCompleteCancel=0x%x, BulkOutCompleteOther=0x%x\n", pAd->BulkOutCompleteCancel, pAd->BulkOutCompleteOther)); */

	}

	/* */
	/* bInUse = TRUE, means some process are filling TX data, after that must turn on bWaitingBulkOut */
	/* bWaitingBulkOut = TRUE, means the TX data are waiting for bulk out. */
	/* */
	/*RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */
	if (((pHTTXContext->ENextBulkOutPosition != pHTTXContext->CurWritePosition) &&
		(pHTTXContext->ENextBulkOutPosition != (pHTTXContext->CurWritePosition+8)) &&
		!RTUSB_TEST_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_WMM1_FRAG << BulkOutPipeId)))
	)
	{
		/* Indicate There is data avaliable */
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_WMM1_NORMAL << BulkOutPipeId));
	}
	/*RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags); */

	/* Always call Bulk routine, even reset bulk. */
	/* The protection of rest bulk should be in BulkOut routine */
	RTUSBKickBulkOut(pAd);
}
#endif /* MULTI_WMM_SUPPORT */

static void rtusb_null_frame_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER	pAd;
	PTX_CONTEXT		pNullContext;
	purbb_t			pUrb;
	NTSTATUS		Status;
	unsigned long	irqFlag;


	pUrb			= (purbb_t)data;
/*	pNullContext	= (PTX_CONTEXT)pUrb->context; */
	pNullContext	= (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status			= RTMP_USB_URB_STATUS_GET(pUrb);
	pAd 			= pNullContext->pAd;
/*	Status 			= pUrb->status; */

	/* Reset Null frame context flags */
	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], irqFlag);
	pNullContext->IRPPending 	= FALSE;
	pNullContext->InUse 		= FALSE;
	pAd->BulkOutPending[0] = FALSE;
	pAd->watchDogTxPendingCnt[0] = 0;

	if (Status == USB_ST_NOERROR)
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);

		RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
	}
	else	/* STATUS_OTHER */
	{
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("Bulk Out Null Frame Failed, ReasonCode=%d!\n", Status));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
		}
	}

	/* Always call Bulk routine, even reset bulk. */
	/* The protectioon of rest bulk should be in BulkOut routine */
	RTUSBKickBulkOut(pAd);
}


static void rtusb_pspoll_frame_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER	pAd;
	PTX_CONTEXT		pPsPollContext;
	purbb_t			pUrb;
	NTSTATUS		Status;



	pUrb			= (purbb_t)data;
/*	pPsPollContext	= (PTX_CONTEXT)pUrb->context; */
	pPsPollContext	= (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status			= RTMP_USB_URB_STATUS_GET(pUrb);
	pAd				= pPsPollContext->pAd;
/*	Status			= pUrb->status; */

	/* Reset PsPoll context flags */
	pPsPollContext->IRPPending	= FALSE;
	pPsPollContext->InUse		= FALSE;
	pAd->watchDogTxPendingCnt[0] = 0;

	if (Status == USB_ST_NOERROR)
	{
		RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
	}
	else /* STATUS_OTHER */
	{
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("Bulk Out PSPoll Failed\n"));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
			RTMP_RESET_BULK_OUT(pAd);
		}
	}

	RTMP_SEM_LOCK(&pAd->BulkOutLock[0]);
	pAd->BulkOutPending[0] = FALSE;
	RTMP_SEM_UNLOCK(&pAd->BulkOutLock[0]);

	/* Always call Bulk routine, even reset bulk. */
	/* The protectioon of rest bulk should be in BulkOut routine */
	RTUSBKickBulkOut(pAd);

}


/*
========================================================================
Routine Description:
    Handle received packets.

Arguments:
	data				- URB information pointer

Return Value:
    None

Note:
========================================================================
*/
static void rx_done_tasklet(unsigned long data)
{
	purbb_t pURB;
	PRX_CONTEXT pRxContext;
	PRTMP_ADAPTER pAd;
	NTSTATUS Status;

	pURB = (purbb_t)data;
	Status = RTMP_USB_URB_STATUS_GET(pURB);
	pRxContext = (PRX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	pAd = pRxContext->pAd;



	if (Status != USB_ST_NOERROR)
	{
	        // TODO: move radio off from pAd to wdev
		/* Parsing all packets. because after reset, the index will reset to all zero. */
		if ((!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_BULKIN_RESET |
									fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_RADIO_OFF |
									fRTMP_ADAPTER_NIC_NOT_EXIST))))
		{

			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
			RTMP_RESET_BULK_IN(pAd);
		}
	}

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
	{
		/* If the driver is in ATE mode and Rx frame is set into here. */
		if (pAd->ContinBulkIn == TRUE)
		{
			RTUSBBulkReceive(pAd);
		}
	}
	else
#endif /* CONFIG_ATE */

	RTUSBBulkReceive(pAd);


	return;
}




static void rtusb_mgmt_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER 	pAd;
	PTX_CONTEXT		pMLMEContext;
	int				index;
	PNDIS_PACKET	pPacket;
	purbb_t			pUrb;
	NTSTATUS		Status;
	unsigned long	IrqFlags;


	pUrb			= (purbb_t)data;
/*	pMLMEContext	= (PTX_CONTEXT)pUrb->context; */
	pMLMEContext	= (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status			= RTMP_USB_URB_STATUS_GET(pUrb);
	pAd 			= pMLMEContext->pAd;
/*	Status			= pUrb->status; */
	index 			= pMLMEContext->SelfIdx;

	ASSERT((pAd->MgmtRing.TxDmaIdx == index));

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);


#ifdef UAPSD_SUPPORT
	/* Qos Null frame with EOSP shall have valid Wcid value. reference RtmpUSBMgmtKickOut() API. */
	/* otherwise will be value of MCAST_WCID. */
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	if ((pMLMEContext->Wcid != MCAST_WCID) && (VALID_UCAST_ENTRY_WCID(pAd, pMLMEContext->Wcid)))
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pMLMEContext->Wcid];

		UAPSD_SP_Close(pAd, pEntry);
		pMLMEContext->Wcid = MCAST_WCID;
	}
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* Qos Null frame with EOSP shall have valid Wcid value. reference RtmpUSBMgmtKickOut() API. */
		/* otherwise will be value of MCAST_WCID. */
		if ((pMLMEContext->Wcid != MCAST_WCID_TO_REMOVE) && (VALID_UCAST_ENTRY_WCID(pAd, pMLMEContext->Wcid)))
		{
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pMLMEContext->Wcid];

			UAPSD_SP_Close(pAd, pEntry);
			pMLMEContext->Wcid = MCAST_WCID_TO_REMOVE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */


	if (Status != USB_ST_NOERROR)
	{
		/*Bulk-Out fail status handle */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("Bulk Out MLME Failed, Status=%d!\n", Status));
			/* TODO: How to handle about the MLMEBulkOut failed issue. Need to resend the mgmt pkt? */
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
		}
	}

	pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	RTMP_IRQ_LOCK(&pAd->MLMEBulkOutLock, IrqFlags);
	/* Reset MLME context flags */
	pMLMEContext->IRPPending = FALSE;
	pMLMEContext->InUse = FALSE;
	pMLMEContext->bWaitingBulkOut = FALSE;
	pMLMEContext->BulkOutSize = 0;

	pPacket = pAd->MgmtRing.Cell[index].pNdisPacket;
	pAd->MgmtRing.Cell[index].pNdisPacket = NULL;

	/* Increase MgmtRing Index */
	INC_RING_INDEX(pAd->MgmtRing.TxDmaIdx, MGMT_RING_SIZE);
	pAd->MgmtRing.TxSwFreeIdx++;
	RTMP_IRQ_UNLOCK(&pAd->MLMEBulkOutLock, IrqFlags);


#ifdef RT_CFG80211_SUPPORT
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
	{
		HEADER_802_11  *pHeader;
		pHeader = (HEADER_802_11 *)(GET_OS_PKT_DATAPTR(pPacket)+ TXINFO_SIZE + pAd->chipCap.TXWISize);
		CFG80211_SendMgmtFrameDone(pAd, pHeader->Sequence, TRUE);
	}
#endif /* RT_CFG80211_SUPPORT */

	/* No-matter success or fail, we free the mgmt packet. */
	if (pPacket)
		RTMPFreeNdisPacket(pAd, pPacket);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET) &&
			((pAd->bulkResetPipeid & BULKOUT_MGMT_RESET_FLAG) == BULKOUT_MGMT_RESET_FLAG))
		{	/* For Mgmt Bulk-Out failed, ignore it now. */
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{

			/* Always call Bulk routine, even reset bulk. */
			/* The protectioon of rest bulk should be in BulkOut routine */
			if (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE /* pMLMEContext->bWaitingBulkOut == TRUE */)
			{
				RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);
			}
				RTUSBKickBulkOut(pAd);
			}
		}


}


static void rtusb_hcca_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 4;
	purbb_t				pUrb;


	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("--->hcca_dma_done_tasklet\n"));


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd				= pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{
			pHTTXContext = &pAd->TxContext[BulkOutPipeId];

			if (BulkOutPipeId >= NUM_OF_TX_RING)
			{
				MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_OFF, ("<--- %s:: ERROR!! (BulkOutPipeId = %d, NUM_OF_TX_RING = %d)\n\n",
					__FUNCTION__, BulkOutPipeId, NUM_OF_TX_RING));
				return;
			}

			/*if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&*/
			if ((pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, WCID_ALL, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL);
			RTUSBKickBulkOut(pAd);
		}
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("<---hcca_dma_done_tasklet\n"));

		return;
}


static void rtusb_ac3_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 3;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd				= pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			//if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				if ((pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<3);
			RTUSBKickBulkOut(pAd);
		}
	}


	return;
}


static void rtusb_ac2_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 2;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd				= pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			//if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				if ((pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<2);
			RTUSBKickBulkOut(pAd);
		}
	}


	return;
}


static void rtusb_ac1_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 1;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd				= pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			//if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				if ((pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<1);
			RTUSBKickBulkOut(pAd);
		}
	}

	return;

}

#ifdef MULTI_WMM_SUPPORT
static void rtusb_wmm1_ac0_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 0;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd				= pHTTXContext->pAd;

	rtusb_wmm1_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{
			UCHAR txidx = 0;
			pHTTXContext = &pAd->TxContextWmm1[BulkOutPipeId];

			for (txidx=0;txidx<=(NUM_OF_TX_RING-1);txidx++)
			{
				if ((pAd->DeQueueRunning[txidx] == FALSE) &&
					(pHTTXContext->bCurWriting == FALSE))
				{
					RTMPDeQueuePacket(pAd, FALSE, txidx, WCID_ALL, MAX_TX_PROCESS);
				}
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_WMM1_NORMAL);
			RTUSBKickBulkOut(pAd);
		}
	}


	return;

}
#endif /* MULTI_WMM_SUPPORT */

static void rtusb_ac0_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 0;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
/*	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context; */
	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	pAd				= pHTTXContext->pAd;

	rtusb_dataout_complete((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			//if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				if ((pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
							/*Re-decision Qidx, since BE will always have high priority, should fix it.*/
				RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
						}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL);
			RTUSBKickBulkOut(pAd);
		}
	}


	return;

}

#ifdef MT_MAC
static void rtusb_bcn_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER 	pAd;
	PTX_CONTEXT		pBcnContext;
	int				index;
	PNDIS_PACKET	pPacket;
	purbb_t			pUrb;
	NTSTATUS		Status;
	unsigned long	IrqFlags;

    UCHAR           *ppkt_tail;
    UCHAR           padding_length;
    struct wifi_dev *wdev = NULL;
    UCHAR           wdev_idx = WDEV_NUM_MAX;
    BCN_BUF_STRUC *bcn_info = NULL;


	pUrb			= (purbb_t)data;
	pBcnContext	= (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pUrb);
	Status			= RTMP_USB_URB_STATUS_GET(pUrb);
	pAd 			= pBcnContext->pAd;
	index 			= pBcnContext->SelfIdx;

	ASSERT((pAd->BcnRing.TxDmaIdx == index));

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	if (Status != USB_ST_NOERROR)
	{
		/*Bulk-Out fail status handle */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("Bulk Out BCN Failed, Status=%d!\n", Status));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
		}
	}

	pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	RTMP_IRQ_LOCK(&pAd->MLMEBulkOutLock, IrqFlags);
	/* Reset MLME context flags */
	pBcnContext->IRPPending = FALSE;
	pBcnContext->InUse = FALSE;
	pBcnContext->bWaitingBulkOut = FALSE;
	pBcnContext->BulkOutSize = 0;

	pPacket = pAd->BcnRing.Cell[index].pNdisPacket;
    RTMP_SEM_LOCK(&pAd->BcnRingLock);

    ppkt_tail = GET_OS_PKT_DATATAIL(pPacket);
    padding_length = ppkt_tail - pBcnContext->ppkt_tail_before_padding;
    wdev_idx = pBcnContext->wdev_idx;
    ASSERT(wdev_idx <= WDEV_NUM_MAX);
    wdev = pAd->wdev_list[wdev_idx];
    bcn_info = &wdev->bcn_buf;

    /*
        because MT7603 doesn't free bcn_buf. if we don't push 4 byte after kickout done,
        next time tx will put extra 4byte.
        what if we keep put but without push back, it will cause overflow.
    */
    OS_PKT_TAIL_ADJUST(pPacket, padding_length);

	pAd->BcnRing.Cell[index].pNdisPacket = NULL;
    RTMP_SEM_UNLOCK(&pAd->BcnRingLock);

	/* Increase MgmtRing Index */
	INC_RING_INDEX(pAd->BcnRing.TxDmaIdx, BCN_RING_SIZE);
	pAd->BcnRing.TxSwFreeIdx++;
	RTMP_IRQ_UNLOCK(&pAd->MLMEBulkOutLock, IrqFlags);

	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		/* do nothing and return directly. */
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET) &&
			((pAd->bulkResetPipeid & BULKOUT_MGMT_RESET_FLAG) == BULKOUT_MGMT_RESET_FLAG))
		{	/* For Mgmt Bulk-Out failed, ignore it now. */
			RTMP_RESET_BULK_OUT(pAd);
		}
		else
		{

			/* Always call Bulk routine, even reset bulk. */
			/* The protectioon of rest bulk should be in BulkOut routine */
			if (pAd->BcnRing.TxSwFreeIdx < BCN_RING_SIZE)
			{
				RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_BCN);
			}
				RTUSBKickBulkOut(pAd);
		}
    }

}
#endif /* MT_MAC */


#ifdef CONFIG_ATE
static void rtusb_ate_ac0_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd;
	PTX_CONTEXT pNullContext;
	UCHAR BulkOutPipeId;
	NTSTATUS Status;
	ULONG IrqFlags;
	ULONG OldValue;
	purbb_t pURB;

	pURB = (purbb_t)data;
	/*pNullContext = (PTX_CONTEXT)pURB->rtusb_urb_context; */
	pNullContext	= (PTX_CONTEXT)RTMP_USB_URB_DATA_GET(pURB);
	pAd = pNullContext->pAd;

	/* Reset Null frame context flags */
	pNullContext->IRPPending = FALSE;
	pNullContext->InUse = FALSE;
	Status = RTMP_USB_URB_STATUS_GET(pURB);/*pURB->rtusb_urb_status; */

	/* Store BulkOut PipeId. */
	BulkOutPipeId = pNullContext->BulkOutPipeId;
	pAd->BulkOutDataOneSecCount++;

	if (Status == USB_ST_NOERROR)
	{
		if ((ATE_ON(pAd)))
		{
			if (pAd->ATECtrl.QID == BulkOutPipeId)
			{
				RtmpusecDelay(10);
				pAd->ATECtrl.TxDoneCount++;
				pAd->RalinkCounters.KickTxCount++;
				ASSERT(pAd->ATECtrl.QID == 0);
			}
		}
		pAd->BulkOutComplete++;

		pAd->Counters8023.GoodTransmits++;

		RTMPDeQueuePacket(pAd, TRUE, BulkOutPipeId, WCID_ALL, MAX_TX_PROCESS);

	}
	else
	{
		pAd->BulkOutCompleteOther++;

		MTWF_LOG(DBG_CAT_TEST, CATHIF_USB, DBG_LVL_ERROR, ("BulkOutDataPacket Failed STATUS_OTHER = 0x%x . \n", Status));
		MTWF_LOG(DBG_CAT_TEST, CATHIF_USB, DBG_LVL_ERROR, (">>BulkOutReq=0x%lx, BulkOutComplete=0x%lx\n", pAd->BulkOutReq, pAd->BulkOutComplete));

		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

			/* In 28xx, RT_OID_USB_RESET_BULK_OUT ==> CMDTHREAD_RESET_BULK_OUT */
			RTMP_RESET_BULK_OUT(pAd);

			/* check */
			BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
			pAd->BulkOutPending[BulkOutPipeId] = FALSE;
			pAd->bulkResetPipeid = BulkOutPipeId;
			BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

			return;
		}
	}



	if (atomic_read(&pAd->BulkOutRemained) > 0)
	{
		atomic_dec(&pAd->BulkOutRemained);
	}

	/* 1st - Transmit Success */
	OldValue = pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart;
	pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart++;

	if (pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart < OldValue)
	{
		pAd->WlanCounters[0].TransmittedFragmentCount.u.HighPart++;
	}

	if (((pAd->ContinBulkOut == TRUE ) ||(atomic_read(&pAd->BulkOutRemained) > 0))
		&& (pAd->ATECtrl.Mode & ATE_TXFRAME))
	{
		RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE);
	}
	else
	{
		RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE);
#ifdef CONFIG_QA
		pAd->ATECtrl.TxStatus = 0;
#endif /* CONFIG_QA */
	}

	BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutPending[BulkOutPipeId] = FALSE;
	BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

	/* Always call Bulk routine, even reset bulk. */
	/* The protection of rest bulk should be in BulkOut routine. */
	RTUSBKickBulkOut(pAd);
}
#endif /* CONFIG_ATE */


NDIS_STATUS RtmpNetTaskInit(
	IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* Create receive tasklet */
	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx_done_task, rx_done_tasklet, (ULONG)pAd);
	//RTMP_OS_TASKLET_INIT(pAd, &pObj->cmd_rsp_event_task, cmd_rsp_event_tasklet, (ULONG)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mgmt_dma_done_task, rtusb_mgmt_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac0_dma_done_task, rtusb_ac0_dma_done_tasklet, (unsigned long)pAd);
#ifdef CONFIG_ATE
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ate_ac0_dma_done_task, rtusb_ate_ac0_dma_done_tasklet, (unsigned long)pAd);
#endif /* CONFIG_ATE */
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac1_dma_done_task, rtusb_ac1_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac2_dma_done_task, rtusb_ac2_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac3_dma_done_task, rtusb_ac3_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->hcca_dma_done_task, rtusb_hcca_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->tbtt_task, tbtt_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->null_frame_complete_task, rtusb_null_frame_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->pspoll_frame_complete_task, rtusb_pspoll_frame_done_tasklet, (unsigned long)pAd);
#ifdef MT_MAC
    RTMP_OS_TASKLET_INIT(pAd, &pObj->bcn_dma_done_task, rtusb_bcn_dma_done_tasklet, (unsigned long)pAd);
#endif
#ifdef MULTI_WMM_SUPPORT
	RTMP_OS_TASKLET_INIT(pAd, &pObj->wmm1_ac0_dma_done_task, rtusb_wmm1_ac0_dma_done_tasklet, (unsigned long)pAd);
#endif

	return NDIS_STATUS_SUCCESS;
}


void RtmpNetTaskExit(IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_OS_TASKLET_KILL(&pObj->rx_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->cmd_rsp_event_task);
	RTMP_OS_TASKLET_KILL(&pObj->mgmt_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac0_dma_done_task);
#ifdef CONFIG_ATE
	RTMP_OS_TASKLET_KILL(&pObj->ate_ac0_dma_done_task);
#endif
#ifdef MT_MAC
    RTMP_OS_TASKLET_KILL(&pObj->bcn_dma_done_task);
#endif /* MT_MAC */
	RTMP_OS_TASKLET_KILL(&pObj->ac1_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac2_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac3_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->hcca_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->tbtt_task);
	RTMP_OS_TASKLET_KILL(&pObj->null_frame_complete_task);
	RTMP_OS_TASKLET_KILL(&pObj->pspoll_frame_complete_task);
}


/*
========================================================================
Routine Description:
    USB command kernel thread.

Arguments:
	*Context			the pAd, driver control block pointer

Return Value:
    0					close the thread

Note:
========================================================================
*/
INT RTUSBCmdThread(
	IN ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		return 0;

	RtmpOSTaskCustomize(pTask);

	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_RUNNING)
	{
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

		if (!pAd->PM_FlgSuspend)
			CMDHandler(pAd);
	}

	if (!pAd->PM_FlgSuspend)
	{	/* Clear the CmdQElements. */
		CmdQElmt	*pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		while(pAd->CmdQ.size)
		{
			RTThreadDequeueCmd(&pAd->CmdQ, &pCmdQElmt);
			if (pCmdQElmt)
			{
				if (pCmdQElmt->CmdFromNdis == TRUE)
				{
					if (pCmdQElmt->buffer != NULL)
						os_free_mem(pCmdQElmt->buffer);
					os_free_mem((PUCHAR)pCmdQElmt);
				}
				else
				{
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pCmdQElmt->buffer);
					os_free_mem((PUCHAR)pCmdQElmt);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->CmdQLock);
	}
	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_TRACE,( "<---RTUSBCmdThread\n"));

	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}

void InitUSBDevice(RT_CMD_USB_INIT *config, VOID *ad_src)
{
	PRTMP_ADAPTER ad = (PRTMP_ADAPTER)ad_src;

	ad->infType = RTMP_DEV_INF_USB;

	RTMP_SEM_EVENT_INIT(&(ad->UsbVendorReq_semaphore), &ad->RscSemMemList);
#ifdef RLT_MAC
	RTMP_SEM_EVENT_INIT(&(ad->WlanEnLock), &ad->RscSemMemList);
#endif /* RLT_MAC */
	RTMP_SEM_EVENT_INIT(&(ad->reg_atomic), &ad->RscSemMemList);
	RTMP_SEM_EVENT_INIT(&(ad->hw_atomic), &ad->RscSemMemList);
	RTMP_SEM_EVENT_INIT(&(ad->cal_atomic), &ad->RscSemMemList);
	if (ad->UsbVendorReqBuf) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_WARN,
			 ("%s UsbVendorReqBuf not null 0x%p, skip alloc\n",
			 __func__, ad->UsbVendorReqBuf));
		goto skip_alloc_vrbuf;
	}
	os_alloc_mem(ad, (PUCHAR *)&ad->UsbVendorReqBuf, MAX_PARAM_BUFFER_SIZE - 1);

	if (ad->UsbVendorReqBuf == NULL)
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("Allocate vendor request temp buffer failed!\n"));
		return;
	}

skip_alloc_vrbuf:

#ifdef RLT_MAC
	if (config->driver_info == RLT_MAC_BASE) {
		UINT32 value;
		RTMP_IO_READ32(ad, 0x00, &value);
		ad->ChipID = value;
	}
#endif /* RLT_MAC */

#ifdef MT_MAC
    if (config->driver_info == MT_MAC_BASE) {
        UINT32 value;
        RTMP_IO_READ32(ad, TOP_HCR, &value);
        ad->ChipID = value;
    }
#endif /* MT_MAC */

	RtmpRaDevCtrlInit(ad, ad->infType);
}


struct device* rtmp_get_dev(void *ad)
{
	struct device *dev;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)ad;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	struct usb_device *inf_dev = obj->pUsb_Dev;
	dev = (struct device *)(&(((struct usb_device *)(inf_dev))->dev));

	return dev;
}

