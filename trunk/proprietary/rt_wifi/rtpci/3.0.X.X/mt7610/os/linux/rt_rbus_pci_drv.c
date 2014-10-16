/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/



#ifdef RTMP_MAC_PCI
#include	"rt_config.h"


static void rx_done_tasklet(unsigned long data);
#ifdef RLT_MAC
static void rx1_done_tasklet(unsigned long data);
#endif /* RLT_MAC */
static void mgmt_dma_done_tasklet(unsigned long data);
static void ac0_dma_done_tasklet(unsigned long data);
#ifdef RALINK_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data);
#endif /* RALINK_ATE */
static void ac1_dma_done_tasklet(unsigned long data);
static void ac2_dma_done_tasklet(unsigned long data);
static void ac3_dma_done_tasklet(unsigned long data);
static void hcca_dma_done_tasklet(unsigned long data);
static void fifo_statistic_full_tasklet(unsigned long data);

#ifdef UAPSD_SUPPORT
static void uapsd_eosp_sent_tasklet(unsigned long data);
#endif /* UAPSD_SUPPORT */


/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/
NDIS_STATUS RtmpNetTaskInit(IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	

	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx_done_task, rx_done_tasklet, (unsigned long)pAd);
#ifdef RLT_MAC
	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx1_done_task, rx1_done_tasklet, (unsigned long)pAd);
#endif /* RLT_MAC */
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mgmt_dma_done_task, mgmt_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac0_dma_done_task, ac0_dma_done_tasklet, (unsigned long)pAd);
#ifdef RALINK_ATE
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ate_ac0_dma_done_task, ate_ac0_dma_done_tasklet, (unsigned long)pAd);
#endif /* RALINK_ATE */
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac1_dma_done_task, ac1_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac2_dma_done_task, ac2_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac3_dma_done_task, ac3_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->hcca_dma_done_task, hcca_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->tbtt_task, tbtt_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->fifo_statistic_full_task, fifo_statistic_full_tasklet, (unsigned long)pAd);
#ifdef UAPSD_SUPPORT	
		RTMP_OS_TASKLET_INIT(pAd, &pObj->uapsd_eosp_sent_task, uapsd_eosp_sent_tasklet, (unsigned long)pAd);
#endif /* UAPSD_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		RTMP_OS_TASKLET_INIT(pAd, &pObj->dfs_task, dfs_tasklet, (unsigned long)pAd);
#endif /* DFS_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


void RtmpNetTaskExit(IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_OS_TASKLET_KILL(&pObj->rx_done_task);
#ifdef RLT_MAC
	RTMP_OS_TASKLET_KILL(&pObj->rx1_done_task);
#endif /* RLT_MAC */
	RTMP_OS_TASKLET_KILL(&pObj->mgmt_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac0_dma_done_task);
#ifdef RALINK_ATE
	RTMP_OS_TASKLET_KILL(&pObj->ate_ac0_dma_done_task);
#endif /* RALINK_ATE */
	RTMP_OS_TASKLET_KILL(&pObj->ac1_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac2_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac3_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->hcca_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->tbtt_task);
	RTMP_OS_TASKLET_KILL(&pObj->fifo_statistic_full_task);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef UAPSD_SUPPORT
		RTMP_OS_TASKLET_KILL(&pObj->uapsd_eosp_sent_task);
#endif /* UAPSD_SUPPORT */

#ifdef DFS_SUPPORT
		RTMP_OS_TASKLET_KILL(&pObj->dfs_task);
#endif /* DFS_SUPPORT */

	}
#endif /* CONFIG_AP_SUPPORT */
}


NDIS_STATUS RtmpMgmtTaskInit(IN RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;


	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpCmdQTask", pAd);
	status = RtmpOSTaskAttach(pTask, RTPCICmdThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE) 
	{
		printk ("Unable to start RTPCICmdThread!\n");
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
	INT ret;


	/* Terminate cmdQ thread */
	RTMP_OS_TASK_LEGALITY(&pAd->cmdQTask)
	{
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);
		
		/*RTUSBCMDUp(&pAd->cmdQTask); */
		ret = RtmpOSTaskKill(&pAd->cmdQTask);
		if (ret == NDIS_STATUS_FAILURE)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Kill command task fail!\n"));
		}
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
	}

#ifdef WSC_INCLUDED
	WscThreadExit(pAd);
#endif /* WSC_INCLUDED */

	return;
}


static inline void rt2860_int_enable(PRTMP_ADAPTER pAd, unsigned int mode)
{
	u32 regValue;

	pAd->int_disable_mask &= ~(mode);
	regValue = pAd->int_enable_reg & ~(pAd->int_disable_mask);
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);

	if (regValue != 0)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);

#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
}


static inline void rt2860_int_disable(PRTMP_ADAPTER pAd, unsigned int mode)
{
	u32 regValue;

	pAd->int_disable_mask |= mode;
	regValue = pAd->int_enable_reg & ~(pAd->int_disable_mask);
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);
	
	if (regValue == 0)
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);		
	}

#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */

}


/***************************************************************************
  *
  *	tasklet related procedures.
  *
  **************************************************************************/
static void mgmt_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mgmt_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	INT_SOURCE_CSR_STRUC IntSource;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~INT_MGMT_DLY;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;

/*	printk("mgmt_dma_done_process\n"); */
	IntSource.word = 0;
#ifdef RT65xx
	if (IS_RT65XX(pAd) && (!IS_RT8592(pAd)))
		IntSource.field.TxDone9 = 1;
	else
#endif /* RT65xx */
	IntSource.field.MgmtDmaDone = 1;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_MGMT_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	
	RTMPHandleMgmtRingDmaDoneInterrupt(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	if (pAd->int_pending & INT_MGMT_DLY) 
	{	/* double check to avoid lose of interrupts */
		RTMP_OS_TASKLET_SCHE(&pObj->mgmt_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_MGMT_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
}


static void rx_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN	bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, rx_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
#ifdef RLT_MAC
		pAd->int_disable_mask &= ~(INT_R0_DONE); 
#else /* RLT_MAC */
		pAd->int_disable_mask &= ~(INT_RX); 
#endif /* !RLT_MAC */
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
#ifdef UAPSD_SUPPORT
	UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_TASKLET);
#endif /* UAPSD_SUPPORT */

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
#ifdef RLT_MAC
		pAd->int_pending &= ~(INT_R0_DONE); 
#else /* RLT_MAC */
		pAd->int_pending &= ~(INT_RX); 
#endif /* !RLT_MAC */
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#ifdef CONFIG_AP_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		bReschedule = APRxDoneInterruptHandle(pAd);
#endif /* CONFIG_AP_SUPPORT */	


#ifdef UAPSD_SUPPORT
	UAPSD_TIMING_RECORD_STOP();
#endif /* UAPSD_SUPPORT */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid rotting packet  */
#ifdef RLT_MAC
	if (pAd->int_pending & INT_R0_DONE || bReschedule) 
#else /* RLT_MAC */
	if (pAd->int_pending & INT_RX || bReschedule) 
#endif /* !RLT_MAC */
	{
		RTMP_OS_TASKLET_SCHE(&pObj->rx_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable RxINT again */
#ifdef RLT_MAC
	rt2860_int_enable(pAd, INT_R0_DONE);
#else /* RLT_MAC */
	rt2860_int_enable(pAd, INT_RX);
#endif /* !RLT_MAC */
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

}

#ifdef RLT_MAC
static void rx1_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN	bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, rx1_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_R1_DONE); 
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~(INT_R1_DONE);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	
	bReschedule = RxRing1DoneInterruptHandle(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid rotting packet  */
	if ((pAd->int_pending & INT_R1_DONE) || bReschedule) 
	{
		RTMP_OS_TASKLET_SCHE(&pObj->rx1_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable Rx1INT again */
	rt2860_int_enable(pAd, INT_R1_DONE);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

}
#endif /* RLT_MAC */

void fifo_statistic_full_tasklet(unsigned long data)
{
	unsigned long flags;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, fifo_statistic_full_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	 {
		  RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		  pAd->int_disable_mask &= ~(FifoStaFullInt); 
		  RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
 	  }
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~(FifoStaFullInt); 
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	
	NICUpdateFifoStaCounters(pAd);
	
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);  
	/*
	 * double check to avoid rotting packet 
	 */
	if (pAd->int_pending & FifoStaFullInt) 
	{
		RTMP_OS_TASKLET_SCHE(&pObj->fifo_statistic_full_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable RxINT again */

	rt2860_int_enable(pAd, FifoStaFullInt);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

}


static void hcca_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, hcca_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	INT_SOURCE_CSR_STRUC IntSource;
	BOOLEAN bReschedule = 0;
	
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~INT_HCCA_DLY; 
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;


	IntSource.word = 0;
	IntSource.field.HccaDmaDone = 1;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_HCCA_DLY;

	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#ifdef RLT_MAC
	RTMPHandleTxRing8DmaDoneInterrupt(pAd);
#else /* RLT_MAC */
	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, (UINT32)IntSource.word);
#endif /* !RLT_MAC */ 

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_HCCA_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->hcca_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_HCCA_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
}


static void ac3_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac3_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	INT_SOURCE_CSR_STRUC IntSource;
	BOOLEAN bReschedule = 0;


	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC3_DLY); 
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

/*	printk("ac0_dma_done_process\n"); */
	IntSource.word = 0;
	IntSource.field.Ac3DmaDone = 1;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC3_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, (UINT32)IntSource.word);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC3_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac3_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC3_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
}


static void ac2_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac2_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	INT_SOURCE_CSR_STRUC IntSource;
	BOOLEAN bReschedule = 0;
	
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC2_DLY); 
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	IntSource.word = 0;
	IntSource.field.Ac2DmaDone = 1;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC2_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, (UINT32)IntSource.word);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);

	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC2_DLY) || bReschedule) 
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac2_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC2_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
}


static void ac1_dma_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
	INT_SOURCE_CSR_STRUC IntSource;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac1_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC1_DLY); 
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

/*	printk("ac0_dma_done_process\n"); */
	IntSource.word = 0;
	IntSource.field.Ac1DmaDone = 1;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC1_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, (UINT32)IntSource.word);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC1_DLY) || bReschedule) 
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac1_dma_done_task);

		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC1_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
}


static void ac0_dma_done_tasklet(unsigned long data)
{
	unsigned long flags;
	INT_SOURCE_CSR_STRUC	IntSource;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac0_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC0_DLY); 
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;

/*	printk("ac0_dma_done_process\n"); */
	IntSource.word = 0;
	IntSource.field.Ac0DmaDone = 1;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC0_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

/*	RTMPHandleMgmtRingDmaDoneInterrupt(pAd); */
	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, (UINT32)IntSource.word);
	
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC0_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac0_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC0_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);    
}


#ifdef RALINK_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data)
{
	return;
}
#endif /* RALINK_ATE */


#ifdef UAPSD_SUPPORT
/*
========================================================================
Routine Description:
    Used to send the EOSP frame.

Arguments:
    data			Pointer to our adapter

Return Value:
    None

Note:
========================================================================
*/
static void uapsd_eosp_sent_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, uapsd_eosp_sent_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif /* WORKQUEUE_BH */

	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
void schedule_dfs_task(PRTMP_ADAPTER pAd)
{
	POS_COOKIE pObj;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_OS_TASKLET_SCHE(&pObj->dfs_task);
}


void dfs_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, dfs_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif /* WORKQUEUE_BH */
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;
	
	if (pRadarDetect->DFSAPRestart == 1)
	{
		int i, j;

		pDfsSwParam->dfs_w_counter += 10;
		/* reset period table */
		for (i = 0; i < pAd->chipCap.DfsEngineNum; i++)
		{
			for (j = 0; j < NEW_DFS_MPERIOD_ENT_NUM; j++)
			{
				pDfsSwParam->DFS_T[i][j].period = 0;
				pDfsSwParam->DFS_T[i][j].idx = 0;
				pDfsSwParam->DFS_T[i][j].idx2 = 0;
			}
		}

		APStop(pAd);
		APStartUp(pAd);
		pRadarDetect->DFSAPRestart = 0;
	}
	else
	/* check radar here */
	{
		int idx;
		if (pRadarDetect->radarDeclared == 0)
		{
			for (idx = 0; idx < 3; idx++)
			{
				if (SWRadarCheck(pAd, idx) == 1)
				{
					/*find the radar signals */
					pRadarDetect->radarDeclared = 1;
					break;
				}
			}
		}
	}
}
#endif /* DFS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


int print_int_count;
VOID RTMPHandleInterrupt(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	INT_SOURCE_CSR_STRUC IntSource;
	POS_COOKIE pObj;
	unsigned long flags=0;


	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/*
		Note 03312008: we can not return here before

		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource.word);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word);
		Or kernel will panic after ifconfig ra0 down sometimes
	*/


	/* Inital the Interrupt source. */
	IntSource.word = 0x00000000L;
	/* McuIntSource.word = 0x00000000L; */

	/* Get the interrupt sources & saved to local variable */
	/*RTMP_IO_READ32(pAd, where, &McuIntSource.word); */
	/*RTMP_IO_WRITE32(pAd, , McuIntSource.word); */

	/*
	Flag fOP_STATUS_DOZE On, means ASIC put to sleep, elase means ASICK WakeUp
	And at the same time, clock maybe turned off that say there is no DMA service.
	when ASIC get to sleep.
	To prevent system hang on power saving.
	We need to check it before handle the INT_SOURCE_CSR, ASIC must be wake up.
	*/
	/* RT2661 => when ASIC is sleeping, MAC register cannot be read and written. */
	/* RT2860 => when ASIC is sleeping, MAC register can be read and written. */
/*	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)) */
	{
		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource.word);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word); /* write 1 to clear */
	}
/*	else
		DBGPRINT(RT_DEBUG_TRACE, (">>>fOP_STATUS_DOZE<<<\n")); */

/*	RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IsrAfterClear); */
/*	RTMP_IO_READ32(pAd, MCU_INT_SOURCE_CSR, &McuIsrAfterClear); */
/*	DBGPRINT(RT_DEBUG_INFO, ("==>RTMPHandleInterrupt(ISR=%08x,Mcu ISR=%08x, After clear ISR=%08x, MCU ISR=%08x)\n", */
/*			IntSource.word, McuIntSource.word, IsrAfterClear, McuIsrAfterClear)); */

//DBGPRINT(RT_DEBUG_OFF, ("%s():IntSource=0x%x\n", __FUNCTION__, IntSource.word));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		return;

	/*
		Handle interrupt, walk through all bits
		Should start from highest priority interrupt
		The priority can be adjust by altering processing if statement
	*/

#ifdef DBG

#endif
		
#ifdef  INF_VR9_HW_INT_WORKAROUND	
redo: 
#endif 

	pAd->bPCIclkOff = FALSE;

	/*
		If required spinlock, each interrupt service routine has to
		acquire and release itself.
	*/

	/* Do nothing if NIC doesn't exist */
	if (IntSource.word == 0xffffffff)
	{
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
		return;
	}

	if (IntSource.word & TxCoherent)
	{
		/*
			When the interrupt occurs, it means we kick a register to send
			a packet, such as TX_MGMTCTX_IDX, but MAC finds some fields in
			the transmit buffer descriptor is not correct, ex: all zeros.
		*/
		DBGPRINT(RT_DEBUG_ERROR, (">>>TxCoherent<<<\n"));
	}

	if (IntSource.word & RxCoherent)
	{
		DBGPRINT(RT_DEBUG_ERROR, (">>>RxCoherent<<<\n"));
	}

	if (IntSource.word & FifoStaFullInt) 
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & FifoStaFullInt) == 0) 
		{
			/* mask FifoStaFullInt */
			rt2860_int_disable(pAd, FifoStaFullInt);
			RTMP_OS_TASKLET_SCHE(&pObj->fifo_statistic_full_task);
		}
		pAd->int_pending |= FifoStaFullInt; 
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if (IntSource.word & INT_MGMT_DLY) 
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & INT_MGMT_DLY) ==0 )
		{
			rt2860_int_disable(pAd, INT_MGMT_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->mgmt_dma_done_task);			
		}
		pAd->int_pending |= INT_MGMT_DLY ;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

#ifdef RLT_MAC	
	if (IntSource.word & INT_R0_DONE)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & INT_R0_DONE) == 0)
		{
#ifdef UAPSD_SUPPORT
			UAPSD_TIMING_RECORD_START();
			UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_ISR);
#endif /* UAPSD_SUPPORT */

			/* mask INT_R0 */
			rt2860_int_disable(pAd, INT_R0_DONE);
			RTMP_OS_TASKLET_SCHE(&pObj->rx_done_task);
		}
		pAd->int_pending |= INT_R0_DONE;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if (IntSource.word & INT_R1_DONE)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & INT_R1_DONE) == 0)
		{
			/* mask INT_R1 */
			rt2860_int_disable(pAd, INT_R1_DONE);
			RTMP_OS_TASKLET_SCHE(&pObj->rx1_done_task);
		}
		pAd->int_pending |= INT_R0_DONE;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}
#else /* RLT_MAC */
	if (IntSource.word & INT_RX)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & INT_RX) == 0) 
		{
#ifdef UAPSD_SUPPORT
			UAPSD_TIMING_RECORD_START();
			UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_ISR);
#endif /* UAPSD_SUPPORT */

			/* mask RxINT */
			rt2860_int_disable(pAd, INT_RX);
			RTMP_OS_TASKLET_SCHE(&pObj->rx_done_task);
		}
		pAd->int_pending |= INT_RX;
       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}
#endif /* !RLT_MAC */

	if (IntSource.word & INT_HCCA_DLY)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & INT_HCCA_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_HCCA_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->hcca_dma_done_task);
		}
		pAd->int_pending |= INT_HCCA_DLY;						
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if (IntSource.word & INT_AC3_DLY)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & INT_AC3_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC3_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac3_dma_done_task);
		}
		pAd->int_pending |= INT_AC3_DLY;						
       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if (IntSource.word & INT_AC2_DLY)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		if ((pAd->int_disable_mask & INT_AC2_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC2_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac2_dma_done_task);
		}
		pAd->int_pending |= INT_AC2_DLY;
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if (IntSource.word & INT_AC1_DLY)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_pending |= INT_AC1_DLY;

		if ((pAd->int_disable_mask & INT_AC1_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC1_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac1_dma_done_task);
		}
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if (IntSource.word & INT_AC0_DLY)
	{

/*
		if (IntSource.word & 0x2) {
			u32 reg;
			RTMP_IO_READ32(pAd, DELAY_INT_CFG, &reg);
			printk("IntSource = %08x, DELAY_REG = %08x\n", IntSource.word, reg);
		}
*/
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_pending |= INT_AC0_DLY;

		if ((pAd->int_disable_mask & INT_AC0_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC0_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac0_dma_done_task);
		}
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}

	if (IntSource.word & PreTBTTInt)
	{
		RTMPHandlePreTBTTInterrupt(pAd);
	}

	if (IntSource.word & TBTTInt)
	{
		RTMPHandleTBTTInterrupt(pAd);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		if (IntSource.word & GPTimeOutInt)
		{
		      NewTimerCB_Radar(pAd);
		}
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
		if ((IntSource.word & INT_TONE_RADAR))
		{
			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
				RTMPHandleRadarInterrupt(pAd);
		}
#endif /* CARRIER_DETECTION_SUPPORT*/

		if (IntSource.word & McuCommand)
		{
			/*RTMPHandleMcuInterrupt(pAd);*/
		}
	}

#endif /* CONFIG_AP_SUPPORT */



#ifdef  INF_VR9_HW_INT_WORKAROUND
	/*
		We found the VR9 Demo board provide from Lantiq at 2010.3.8 will miss interrup sometime caused of Rx-Ring Full
		and our driver no longer receive any packet after the interrupt missing.
		Below patch was recommand by Lantiq for temp workaround.
		And shall be remove in next VR9 platform.
	*/
	IntSource.word = 0x00000000L;
	{
		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource.word);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word); /* write 1 to clear */
	}	
	if (IntSource.word != 0) 
	{		
		goto redo;
	}	
#endif
	return;
}


/*
========================================================================
Routine Description:
    PCI command kernel thread.

Arguments:
	*Context			the pAd, driver control block pointer

Return Value:
    0					close the thread

Note:
========================================================================
*/
INT RTPCICmdThread(
	IN ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	
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
						os_free_mem(pAd, pCmdQElmt->buffer);
					os_free_mem(pAd, (PUCHAR)pCmdQElmt);
				}
				else
				{
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pAd, pCmdQElmt->buffer);
					os_free_mem(pAd, (PUCHAR)pCmdQElmt);
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
	DBGPRINT(RT_DEBUG_TRACE,( "<---RTPCICmdThread\n"));

	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}




/***************************************************************************
 *
 *	PCIe device initialization related procedures.
 *
 ***************************************************************************/
VOID RTMPInitPCIeDevice(
    IN	RT_CMD_PCIE_INIT	*pConfig,
    IN	VOID				*pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	VOID *pci_dev = pConfig->pPciDev;
	USHORT  device_id;
	POS_COOKIE pObj;
	BOOLEAN bPCIe = FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pci_read_config_word(pci_dev, pConfig->ConfigDeviceID, &device_id);
	pObj->DeviceID = device_id;
	DBGPRINT(RT_DEBUG_OFF, ("device_id =0x%x\n", device_id));

#ifdef RT3290
	if (device_id == NIC3290_PCIe_DEVICE_ID) {
		pAd->MACVersion = 0x3290ffff;
		RTMPEnableWlan(pAd, TRUE, TRUE);
		bPCIe = TRUE;
	}
#endif /* RT3290 */
#ifdef RT65xx
	if (device_id == NIC8592_PCIe_DEVICE_ID ||
		device_id == NIC8592_5592_PCIe_DEVICE_ID)
	{
		ral_wlan_chip_onoff(pAd, TRUE, TRUE);
		bPCIe = TRUE;
	}
	
	if (
#ifdef FPGA_MODE
		device_id == NIC6390_PCIe_DEVICE_ID ||
#endif /* FPGA_MODE */
		device_id == NIC7650_PCIe_DEVICE_ID ||
		device_id == NIC7630_PCIe_DEVICE_ID ||
		device_id == NIC7610_PCIe_DEVICE_ID
		) {
		MT76x0_WLAN_ChipOnOff(pAd, TRUE, TRUE);
		bPCIe = TRUE;
	}
#endif /* RT65xx */

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
	if (
	(RT3593_DEVICE_ID_CHECK(device_id))||
	(RT3592_DEVICE_ID_CHECK(device_id))||
		 0)
	{
		UINT32 MacCsr0 = 0;
		WaitForAsicReady(pAd);
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);


		/* Support advanced power save after 2892/2790. */
		/* MAC version at offset 0x1000 is 0x2872XXXX/0x2870XXXX(PCIe, USB, SDIO). */
		if ((MacCsr0&0xffff0000) != 0x28600000)
		{
#ifdef PCIE_PS_SUPPORT			
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
#endif /* PCIE_PS_SUPPORT */
			return;
		}
		bPCIe = TRUE;

	}

	if (bPCIe)
	{
		RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_PCIE);
	}
	else
		RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_PCI);

}

VOID RTMPInitPCIeLinkCtrlValue(
	IN	PRTMP_ADAPTER	pAd)
{
    INT     pos = 0;
    USHORT	reg16, Configuration;
    BOOLEAN	bFindIntel = FALSE;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;


    DBGPRINT(RT_DEBUG_TRACE, ("%s.===>\n", __FUNCTION__));

	/* Init EEPROM, and save settings */
#ifdef MT76x0
	if (IS_MT76x0(pAd)) {
		return;
	}
#endif /* MT76x0 */

#ifdef RT8592
	if (IS_RT8592(pAd)) {
		// TODO: shiang-6590, what shall we do for this??
	} else
#endif /* RT8592 */

    /* Find Ralink PCIe Device's Express Capability Offset */
	pos = pci_find_capability(pObj->pci_dev, PCI_CAP_ID_EXP);

    if (pos != 0)
    {
        /* Ralink PCIe Device's Link Control Register Offset */
        pAd->RLnkCtrlOffset = pos + PCI_EXP_LNKCTL;
    	pci_read_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, &reg16);
        Configuration = le2cpu16(reg16);
        DBGPRINT(RT_DEBUG_TRACE, ("Read (Ralink PCIe Link Control Register) offset 0x%x = 0x%x\n", 
                                    pAd->RLnkCtrlOffset, Configuration));
		pAd->RLnkCtrlConfiguration = (Configuration & 0x103);
		Configuration &= 0xfefc;
		Configuration |= (0x0);

		pObj->parent_pci_dev = RTMPFindHostPCIDev(pObj->pci_dev);
		if (pObj->parent_pci_dev)
		{
			USHORT  vendor_id;

			pci_read_config_word(pObj->parent_pci_dev, RTMP_OS_PCI_VENDOR_ID, &vendor_id);
			vendor_id = le2cpu16(vendor_id);
			if (vendor_id == PCIBUS_INTEL_VENDOR)
			{
				bFindIntel = TRUE;
				RTMP_SET_PSFLAG(pAd, fRTMP_PS_TOGGLE_L1);
			}
			/* Find PCI-to-PCI Bridge Express Capability Offset */
			pos = pci_find_capability(pObj->parent_pci_dev, PCI_CAP_ID_EXP);

			if (pos != 0)
			{
				/* PCI-to-PCI Bridge Link Control Register Offset */
				pAd->HostLnkCtrlOffset = pos + PCI_EXP_LNKCTL;
				pci_read_config_word(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, &reg16);    
				Configuration = le2cpu16(reg16);
				DBGPRINT(RT_DEBUG_TRACE, ("Read (Host PCI-to-PCI Bridge Link Control Register) offset 0x%x = 0x%x\n", 
				                            pAd->HostLnkCtrlOffset, Configuration));    
				pAd->HostLnkCtrlConfiguration = (Configuration & 0x103);
				Configuration &= 0xfefc;
				Configuration |= (0x0);


// TODO: check AP driver needs or not @20120919
			}
			else
			{
				pAd->HostLnkCtrlOffset = 0;
				DBGPRINT(RT_DEBUG_ERROR, ("%s: cannot find PCI-to-PCI Bridge PCI Express Capability!\n", __FUNCTION__));
			}
        }
    }
    else
    {
        pAd->RLnkCtrlOffset = 0;
        pAd->HostLnkCtrlOffset = 0;
        DBGPRINT(RT_DEBUG_ERROR, ("%s: cannot find Ralink PCIe Device's PCI Express Capability!\n", __FUNCTION__));
    }

    if (bFindIntel == FALSE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Doesn't find Intel PCI host controller. \n"));
		/* Doesn't switch L0, L1, So set PCIePowerSaveLevel to 0xff */
		pAd->PCIePowerSaveLevel = 0xff;
	}

}



#endif /* RTMP_MAC_PCI */
