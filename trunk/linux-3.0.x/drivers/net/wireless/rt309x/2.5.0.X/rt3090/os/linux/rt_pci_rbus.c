/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rt_pci_rbus.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include <rt_config.h>
#include <linux/pci.h>


IRQ_HANDLE_TYPE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
rt2860_interrupt(int irq, void *dev_instance);
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#endif

#ifdef WORKQUEUE_BH
static void rx_done_workq(struct work_struct *work);
static void mgmt_dma_done_workq(struct work_struct *work);
static void ac0_dma_done_workq(struct work_struct *work);
static void ac1_dma_done_workq(struct work_struct *work);
static void ac2_dma_done_workq(struct work_struct *work);
static void ac3_dma_done_workq(struct work_struct *work);
static void hcca_dma_done_workq(struct work_struct *work);
static void fifo_statistic_full_workq(struct work_struct *work);
#else
static void rx_done_tasklet(unsigned long data);
static void mgmt_dma_done_tasklet(unsigned long data);
static void ac0_dma_done_tasklet(unsigned long data);
#ifdef RALINK_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data);
#endif // RALINK_ATE //
static void ac1_dma_done_tasklet(unsigned long data);
static void ac2_dma_done_tasklet(unsigned long data);
static void ac3_dma_done_tasklet(unsigned long data);
static void hcca_dma_done_tasklet(unsigned long data);
static void fifo_statistic_full_tasklet(unsigned long data);
#endif // WORKQUEUE_BH //

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
#ifdef WORKQUEUE_BH
static void uapsd_eosp_sent_workq(struct work_struct *work);
#else
static void uapsd_eosp_sent_tasklet(unsigned long data);
#endif // WORKQUEUE_BH //
#endif // UAPSD_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/
#define RT2860_INT_RX_DLY				(1<<0)		// bit 0	
#define RT2860_INT_TX_DLY				(1<<1)		// bit 1
#define RT2860_INT_RX_DONE				(1<<2)		// bit 2
#define RT2860_INT_AC0_DMA_DONE			(1<<3)		// bit 3
#define RT2860_INT_AC1_DMA_DONE			(1<<4)		// bit 4
#define RT2860_INT_AC2_DMA_DONE			(1<<5)		// bit 5
#define RT2860_INT_AC3_DMA_DONE			(1<<6)		// bit 6
#define RT2860_INT_HCCA_DMA_DONE		(1<<7)		// bit 7
#define RT2860_INT_MGMT_DONE			(1<<8)		// bit 8
#if defined(TONE_RADAR_DETECT_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
#define RT2860_INT_TONE_RADAR			(1<<20)		// bit 20
#endif // defined(TONE_RADAR_DETECT_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //

#define INT_RX			RT2860_INT_RX_DONE

#define INT_AC0_DLY		(RT2860_INT_AC0_DMA_DONE) //| RT2860_INT_TX_DLY)
#define INT_AC1_DLY		(RT2860_INT_AC1_DMA_DONE) //| RT2860_INT_TX_DLY)
#define INT_AC2_DLY		(RT2860_INT_AC2_DMA_DONE) //| RT2860_INT_TX_DLY)
#define INT_AC3_DLY		(RT2860_INT_AC3_DMA_DONE) //| RT2860_INT_TX_DLY)
#define INT_HCCA_DLY 	(RT2860_INT_HCCA_DMA_DONE) //| RT2860_INT_TX_DLY)
#define INT_MGMT_DLY	RT2860_INT_MGMT_DONE
#if defined(TONE_RADAR_DETECT_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
#define INT_TONE_RADAR	(RT2860_INT_TONE_RADAR)
#endif // defined(TONE_RADAR_DETECT_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //


/***************************************************************************
  *
  *	Interface-depended memory allocation/Free related procedures.
  *		Mainly for Hardware TxDesc/RxDesc/MgmtDesc, DMA Memory for TxData/RxData, etc.,
  *
  **************************************************************************/



VOID Invalid_Remaining_Packet(
	IN	PRTMP_ADAPTER pAd,
	IN	 ULONG VirtualAddress)
{
	NDIS_PHYSICAL_ADDRESS PhysicalAddress;

	PhysicalAddress = PCI_MAP_SINGLE(pAd, (void *)(VirtualAddress+1600), RX_BUFFER_NORMSIZE-1600, -1, PCI_DMA_FROMDEVICE);
}


NDIS_STATUS RtmpNetTaskInit(IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
#ifdef WORKQUEUE_BH
	INIT_WORK(&pObj->rx_done_work, rx_done_workq);
	INIT_WORK(&pObj->mgmt_dma_done_work, mgmt_dma_done_workq);
	INIT_WORK(&pObj->ac0_dma_done_work, ac0_dma_done_workq);
	INIT_WORK(&pObj->ac1_dma_done_work, ac1_dma_done_workq);
	INIT_WORK(&pObj->ac2_dma_done_work, ac2_dma_done_workq);
	INIT_WORK(&pObj->ac3_dma_done_work, ac3_dma_done_workq);
	INIT_WORK(&pObj->hcca_dma_done_work, hcca_dma_done_workq);
	INIT_WORK(&pObj->tbtt_work, tbtt_workq);
	INIT_WORK(&pObj->fifo_statistic_full_work, fifo_statistic_full_workq);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef UAPSD_AP_SUPPORT	
		INIT_WORK(&pObj->uapsd_eosp_sent_work, uapsd_eosp_sent_workq);
#endif // UAPSD_AP_SUPPORT //

#ifdef DFS_SUPPORT
		INIT_WORK(&pObj->pulse_radar_detect_work, pulse_radar_detect_workq);
		INIT_WORK(&pObj->width_radar_detect_work, width_radar_detect_workq);
#endif // DFS_SUPPORT //
#ifdef CARRIER_DETECTION_SUPPORT
		INIT_WORK(&pObj->carrier_sense_work, carrier_sense_workq);
#endif // CARRIER_DETECTION_SUPPORT //

#ifdef DFS_HARDWARE_SUPPORT
		INIT_WORK(&pObj->dfs_work, dfs_workq);
#endif // DFS_HARDWARE_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //
#else	
	tasklet_init(&pObj->rx_done_task, rx_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->mgmt_dma_done_task, mgmt_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->ac0_dma_done_task, ac0_dma_done_tasklet, (unsigned long)pAd);
#ifdef RALINK_ATE
	tasklet_init(&pObj->ate_ac0_dma_done_task, ate_ac0_dma_done_tasklet, (unsigned long)pAd);
#endif // RALINK_ATE //
	tasklet_init(&pObj->ac1_dma_done_task, ac1_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->ac2_dma_done_task, ac2_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->ac3_dma_done_task, ac3_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->hcca_dma_done_task, hcca_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->tbtt_task, tbtt_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->fifo_statistic_full_task, fifo_statistic_full_tasklet, (unsigned long)pAd);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef UAPSD_AP_SUPPORT	
		tasklet_init(&pObj->uapsd_eosp_sent_task, uapsd_eosp_sent_tasklet, (unsigned long)pAd);
#endif // UAPSD_AP_SUPPORT //

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
		tasklet_init(&pObj->pulse_radar_detect_task, pulse_radar_detect_tasklet, (unsigned long)pAd);
		tasklet_init(&pObj->width_radar_detect_task, width_radar_detect_tasklet, (unsigned long)pAd);
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //
#ifdef CARRIER_DETECTION_SUPPORT
		tasklet_init(&pObj->carrier_sense_task, carrier_sense_tasklet, (unsigned long)pAd);
#endif // CARRIER_DETECTION_SUPPORT //

#ifdef DFS_HARDWARE_SUPPORT
		tasklet_init(&pObj->dfs_task, dfs_tasklet, (unsigned long)pAd);
#endif // DFS_HARDWARE_SUPPORT //
	}
#endif // CONFIG_AP_SUPPORT //
#endif // WORKQUEUE_BH //

	return NDIS_STATUS_SUCCESS;
}


void RtmpNetTaskExit(IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifndef WORKQUEUE_BH
	tasklet_kill(&pObj->rx_done_task);
	tasklet_kill(&pObj->mgmt_dma_done_task);
	tasklet_kill(&pObj->ac0_dma_done_task);
#ifdef RALINK_ATE
	tasklet_kill(&pObj->ate_ac0_dma_done_task);
#endif // RALINK_ATE //
	tasklet_kill(&pObj->ac1_dma_done_task);
	tasklet_kill(&pObj->ac2_dma_done_task);
	tasklet_kill(&pObj->ac3_dma_done_task);
	tasklet_kill(&pObj->hcca_dma_done_task);
	tasklet_kill(&pObj->tbtt_task);
	tasklet_kill(&pObj->fifo_statistic_full_task);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef UAPSD_AP_SUPPORT
		tasklet_kill(&pObj->uapsd_eosp_sent_task);
#endif // UAPSD_AP_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
		tasklet_kill(&pObj->carrier_sense_task);
#endif // CARRIER_DETECTION_SUPPORT //
#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
		tasklet_kill(&pObj->width_radar_detect_task);
		tasklet_kill(&pObj->pulse_radar_detect_task);
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //

#ifdef DFS_HARDWARE_SUPPORT
		tasklet_kill(&pObj->dfs_task);
#endif // DFS_HARDWARE_SUPPORT //

	}
#endif // CONFIG_AP_SUPPORT //
#endif // WORKQUEUE_BH //
}


NDIS_STATUS RtmpMgmtTaskInit(IN RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;


	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;
	RtmpOSTaskInit(pTask, "RtmpCmdQTask", pAd);
	status = RtmpOSTaskAttach(pTask, RTPCICmdThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE) 
	{
		printk (KERN_WARNING "%s: unable to start RTPCICmdThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

#ifdef WSC_INCLUDED
	// start the crediential write task first.
	WscThreadInit(pAd);
#endif // WSC_INCLUDED //

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

	/* Terminate cmdQ thread */
	pTask = &pAd->cmdQTask;
#ifdef KTHREAD_SUPPORT
	if (pTask->kthread_task)
#else
	CHECK_PID_LEGALITY(pTask->taskPID)
#endif
	{
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);
		
		//RTUSBCMDUp(pAd);
		ret = RtmpOSTaskKill(pTask);
		if (ret == NDIS_STATUS_FAILURE)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: kill task(%s) failed!\n", 
					RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev), pTask->taskName));
		}
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
	}

#ifdef WSC_INCLUDED
	WscThreadExit(pAd);
#endif // WSC_INCLUDED //

	return;
}


static inline void rt2860_int_enable(PRTMP_ADAPTER pAd, unsigned int mode)
{
	u32 regValue;

	pAd->int_disable_mask &= ~(mode);
	regValue = pAd->int_enable_reg & ~(pAd->int_disable_mask);		
	//if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);     // 1:enable
	}
	//else
	//	DBGPRINT(RT_DEBUG_TRACE, ("fOP_STATUS_DOZE !\n"));

	if (regValue != 0)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}


static inline void rt2860_int_disable(PRTMP_ADAPTER pAd, unsigned int mode)
{
	u32 regValue;

	pAd->int_disable_mask |= mode;
	regValue = 	pAd->int_enable_reg & ~(pAd->int_disable_mask);
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);     // 0: disable 
	
	if (regValue == 0)
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);		
	}
}


/***************************************************************************
  *
  *	tasklet related procedures.
  *
  **************************************************************************/
#ifdef WORKQUEUE_BH
static void mgmt_dma_done_workq(struct work_struct *work)
#else
static void mgmt_dma_done_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
    INT_SOURCE_CSR_STRUC	IntSource;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, mgmt_dma_done_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //
	
	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
		pAd->int_disable_mask &= ~INT_MGMT_DLY;
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

//	printk("mgmt_dma_done_process\n");
	IntSource.word = 0;
	IntSource.field.MgmtDmaDone = 1;
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~INT_MGMT_DLY;
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
	
	RTMPHandleMgmtRingDmaDoneInterrupt(pAd);

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if (pAd->int_pending & INT_MGMT_DLY) 
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->mgmt_dma_done_work);
#else
		tasklet_hi_schedule(&pObj->mgmt_dma_done_task);
#endif // WORKQUEUE_BH //
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_MGMT_DLY);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
}


#ifdef WORKQUEUE_BH
static void rx_done_workq(struct work_struct *work)
#else
static void rx_done_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
	BOOLEAN	bReschedule = 0;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, rx_done_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //
	
	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
		pAd->int_disable_mask &= ~(INT_RX); 
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
	}
#ifdef UAPSD_AP_SUPPORT
	UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_TASKLET);
#endif // UAPSD_AP_SUPPORT //

    pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~(INT_RX);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
	
#ifdef CONFIG_AP_SUPPORT	
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		bReschedule = APRxDoneInterruptHandle(pAd);
#endif // CONFIG_AP_SUPPORT //	

#ifdef UAPSD_AP_SUPPORT
	UAPSD_TIMING_RECORD_STOP();
#endif // UAPSD_AP_SUPPORT //

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	/*
	 * double check to avoid rotting packet 
	 */
	if (pAd->int_pending & INT_RX || bReschedule) 
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->rx_done_work);
#else
		tasklet_hi_schedule(&pObj->rx_done_task);
#endif // WORKQUEUE_BH //
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable RxINT again */
	rt2860_int_enable(pAd, INT_RX);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

}

#ifdef WORKQUEUE_BH
void fifo_statistic_full_workq(struct work_struct *work)
#else
void fifo_statistic_full_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, fifo_statistic_full_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //
	
	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	 {
		  RTMP_INT_LOCK(&pAd->irq_lock, flags);
		  pAd->int_disable_mask &= ~(FifoStaFullInt); 
		  RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
 	  }
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~(FifoStaFullInt); 
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
	
	NICUpdateFifoStaCounters(pAd);
	
	RTMP_INT_LOCK(&pAd->irq_lock, flags);  
	/*
	 * double check to avoid rotting packet 
	 */
	if (pAd->int_pending & FifoStaFullInt) 
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->fifo_statistic_full_work);
#else
		tasklet_hi_schedule(&pObj->fifo_statistic_full_task);
#endif // WORKQUEUE_BH //
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable RxINT again */

	rt2860_int_enable(pAd, FifoStaFullInt);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

}

#ifdef WORKQUEUE_BH
static void hcca_dma_done_workq(struct work_struct *work)
#else
static void hcca_dma_done_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
    INT_SOURCE_CSR_STRUC	IntSource;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, hcca_dma_done_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //
	
	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
		pAd->int_disable_mask &= ~INT_HCCA_DLY; 
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;


	IntSource.word = 0;
	IntSource.field.HccaDmaDone = 1;
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~INT_HCCA_DLY;

	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
	
	RTMPHandleTxRingDmaDoneInterrupt(pAd, IntSource);

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if (pAd->int_pending & INT_HCCA_DLY)
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->hcca_dma_done_work);
#else
		tasklet_hi_schedule(&pObj->hcca_dma_done_task);
#endif // WORKQUEUE_BH //
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_HCCA_DLY);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
}

#ifdef WORKQUEUE_BH
static void ac3_dma_done_workq(struct work_struct *work)
#else
static void ac3_dma_done_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
    INT_SOURCE_CSR_STRUC	IntSource;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac3_dma_done_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //

	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
		pAd->int_disable_mask &= ~(INT_AC3_DLY); 
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

//	printk("ac0_dma_done_process\n");
	IntSource.word = 0;
	IntSource.field.Ac3DmaDone = 1;
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~INT_AC3_DLY;
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, IntSource);

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC3_DLY) || bReschedule)
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->ac3_dma_done_work);
#else
		tasklet_hi_schedule(&pObj->ac3_dma_done_task);
#endif // WORKQUEUE_BH //
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC3_DLY);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
}

#ifdef WORKQUEUE_BH
static void ac2_dma_done_workq(struct work_struct *work)
#else
static void ac2_dma_done_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
    INT_SOURCE_CSR_STRUC	IntSource;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac2_dma_done_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //
	
	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
		pAd->int_disable_mask &= ~(INT_AC2_DLY); 
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	IntSource.word = 0;
	IntSource.field.Ac2DmaDone = 1;
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~INT_AC2_DLY;
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, IntSource);

	RTMP_INT_LOCK(&pAd->irq_lock, flags);

	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC2_DLY) || bReschedule) 
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->ac2_dma_done_work);
#else
		tasklet_hi_schedule(&pObj->ac2_dma_done_task);
#endif // WORKQUEUE_BH //
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC2_DLY);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
}

#ifdef WORKQUEUE_BH
static void ac1_dma_done_workq(struct work_struct *work)
#else
static void ac1_dma_done_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
    INT_SOURCE_CSR_STRUC	IntSource;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac1_dma_done_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //

	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
		pAd->int_disable_mask &= ~(INT_AC1_DLY); 
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
	}
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

//	printk("ac0_dma_done_process\n");
	IntSource.word = 0;
	IntSource.field.Ac1DmaDone = 1;
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~INT_AC1_DLY;
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, IntSource);

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC1_DLY) || bReschedule) 
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->ac1_dma_done_work);
#else
		tasklet_hi_schedule(&pObj->ac1_dma_done_task);
#endif // WORKQUEUE_BH //

		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC1_DLY);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
}

#ifdef WORKQUEUE_BH
static void ac0_dma_done_workq(struct work_struct *work)
#else
static void ac0_dma_done_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
	unsigned long flags;
	INT_SOURCE_CSR_STRUC	IntSource;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac0_dma_done_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif // WORKQUEUE_BH //

	// Do nothing if the driver is starting halt state.
	// This might happen when timer already been fired before cancel timer with mlmehalt
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
		pAd->int_disable_mask &= ~(INT_AC0_DLY); 
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
		return;
	}
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;

//	printk("ac0_dma_done_process\n");
	IntSource.word = 0;
	IntSource.field.Ac0DmaDone = 1;
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	pAd->int_pending &= ~INT_AC0_DLY;
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

//	RTMPHandleMgmtRingDmaDoneInterrupt(pAd);
	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, IntSource);
	
	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	/*
	 * double check to avoid lose of interrupts
	 */
	if ((pAd->int_pending & INT_AC0_DLY) || bReschedule)
	{
#ifdef WORKQUEUE_BH
		schedule_work(&pObj->ac0_dma_done_work);
#else
		tasklet_hi_schedule(&pObj->ac0_dma_done_task);
#endif // WORKQUEUE_BH //
		RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
		return;
	}

	/* enable TxDataInt again */
	rt2860_int_enable(pAd, INT_AC0_DLY);
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);    
}


#ifdef RALINK_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data)
{
	return;
}
#endif // RALINK_ATE //


#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
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
#ifdef WORKQUEUE_BH
static void uapsd_eosp_sent_workq(struct work_struct *work)
{
	POS_COOKIE pObj = container_of(work, struct os_cookie, uapsd_eosp_sent_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
	
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}
#else
static void uapsd_eosp_sent_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;

	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}
#endif // WORKQUEUE_BH //
#endif // UAPSD_AP_SUPPORT //


#ifdef DFS_HARDWARE_SUPPORT
void schedule_dfs_task(PRTMP_ADAPTER pAd)
{
	POS_COOKIE pObj;
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef WORKQUEUE_BH
	schedule_work(&pObj->dfs_work);
#else
	tasklet_hi_schedule(&pObj->dfs_task);
#endif // WORKQUEUE_BH //
}

#ifdef WORKQUEUE_BH
void dfs_workq(struct work_struct *work)
#else
void dfs_tasklet(unsigned long data)
#endif // WORKQUEUE_BH //
{
#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, dfs_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif // WORKQUEUE_BH //
	
	if (pAd->CommonCfg.DFSAPRestart == 1)
	{
		int i, j;

		pAd->CommonCfg.dfs_w_counter += 10;
		// reset period table
		for (i = 0; i < NEW_DFS_MAX_CHANNEL; i++)
		{
			for (j = 0; j < NEW_DFS_MPERIOD_ENT_NUM; j++)
			{
				pAd->CommonCfg.DFS_T[i][j].period = 0;
				pAd->CommonCfg.DFS_T[i][j].idx = 0;
				pAd->CommonCfg.DFS_T[i][j].idx2 = 0;
			}
		}

		APStop(pAd);
		APStartUp(pAd);
		pAd->CommonCfg.DFSAPRestart = 0;
	}
	else
	// check radar here
	{
		
#ifdef DFS_HWTIMER_SUPPORT
		int idx;
		if (pAd->CommonCfg.radarDeclared == 0)
		{
			for (idx = 0; idx < 3; idx++)
			{
				if (SWRadarCheck(pAd, idx) == 1)
				{
					//find the radar signals
					pAd->CommonCfg.radarDeclared = 1;
					break;
				}
			}
		}
#endif // DFS_HWTIMER_SUPPORT //
	}
	
	
}
#endif // DFS_HARDWARE_SUPPORT //

#endif // CONFIG_AP_SUPPORT //


/***************************************************************************
  *
  *	interrupt handler related procedures.
  *
  **************************************************************************/
int print_int_count;

IRQ_HANDLE_TYPE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
rt2860_interrupt(int irq, void *dev_instance)
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct net_device *net_dev = (struct net_device *) dev_instance;
	PRTMP_ADAPTER pAd = NULL;
	INT_SOURCE_CSR_STRUC	IntSource;
	POS_COOKIE pObj;
#ifdef MULTI_CORE_SUPPORT
	unsigned long flags=0;
#endif // MULTI_CORE_SUPPORT //	
	
	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	pObj = (POS_COOKIE) pAd->OS_Cookie;


	/* Note 03312008: we can not return here before
		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource.word);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word);
		Or kernel will panic after ifconfig ra0 down sometimes */


	//
	// Inital the Interrupt source.
	//
	IntSource.word = 0x00000000L;
//	McuIntSource.word = 0x00000000L;

	//
	// Get the interrupt sources & saved to local variable
	//
	//RTMP_IO_READ32(pAd, where, &McuIntSource.word);
	//RTMP_IO_WRITE32(pAd, , McuIntSource.word);

	//
	// Flag fOP_STATUS_DOZE On, means ASIC put to sleep, elase means ASICK WakeUp
	// And at the same time, clock maybe turned off that say there is no DMA service.
	// when ASIC get to sleep. 
	// To prevent system hang on power saving.
	// We need to check it before handle the INT_SOURCE_CSR, ASIC must be wake up.
	//
	// RT2661 => when ASIC is sleeping, MAC register cannot be read and written.
	// RT2860 => when ASIC is sleeping, MAC register can be read and written.
//	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource.word);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word); // write 1 to clear
	}
//	else
//		DBGPRINT(RT_DEBUG_TRACE, (">>>fOP_STATUS_DOZE<<<\n"));

//	RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IsrAfterClear);
//	RTMP_IO_READ32(pAd, MCU_INT_SOURCE_CSR, &McuIsrAfterClear);
//	DBGPRINT(RT_DEBUG_INFO, ("====> RTMPHandleInterrupt(ISR=%08x,Mcu ISR=%08x, After clear ISR=%08x, MCU ISR=%08x)\n",
//			IntSource.word, McuIntSource.word, IsrAfterClear, McuIsrAfterClear));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
        return  IRQ_HANDLED;
#else
        return;
#endif
	}

	// Do nothing if Reset in progress
	if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |fRTMP_ADAPTER_HALT_IN_PROGRESS)))
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
        return  IRQ_HANDLED;
#else
        return;
#endif
	}

	//
	// Handle interrupt, walk through all bits
	// Should start from highest priority interrupt
	// The priority can be adjust by altering processing if statement
	//

#ifdef DBG

#endif
		
#ifdef  INF_VR9_HW_INT_WORKAROUND	
redo: 
#endif 

	pAd->bPCIclkOff = FALSE;

	// If required spinlock, each interrupt service routine has to acquire
	// and release itself.
	//
	
	// Do nothing if NIC doesn't exist
	if (IntSource.word == 0xffffffff)
	{
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
        return  IRQ_HANDLED;
#else
        return;
#endif
	}
	
	if (IntSource.word & TxCoherent)
	{
		DBGPRINT(RT_DEBUG_ERROR, (">>>TxCoherent<<<\n"));
		RTMPHandleRxCoherentInterrupt(pAd);
	}

	if (IntSource.word & RxCoherent)
	{
		DBGPRINT(RT_DEBUG_ERROR, (">>>RxCoherent<<<\n"));
		RTMPHandleRxCoherentInterrupt(pAd);
	}

	if (IntSource.word & FifoStaFullInt) 
	{
#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		if ((pAd->int_disable_mask & FifoStaFullInt) == 0) 
		{
			/* mask FifoStaFullInt */
			rt2860_int_disable(pAd, FifoStaFullInt);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->fifo_statistic_full_work);
#else
			tasklet_hi_schedule(&pObj->fifo_statistic_full_task);
#endif // WORKQUEUE_BH //
		}
		pAd->int_pending |= FifoStaFullInt; 
#ifdef MULTI_CORE_SUPPORT
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
	}

	if (IntSource.word & INT_MGMT_DLY) 
	{
#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		if ((pAd->int_disable_mask & INT_MGMT_DLY) ==0 )
		{
			rt2860_int_disable(pAd, INT_MGMT_DLY);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->mgmt_dma_done_work);
#else
			tasklet_hi_schedule(&pObj->mgmt_dma_done_task);			
#endif // WORKQUEUE_BH //
		}
		pAd->int_pending |= INT_MGMT_DLY ;
#ifdef MULTI_CORE_SUPPORT
	        RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
	}

	if (IntSource.word & INT_RX)
	{
#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		if ((pAd->int_disable_mask & INT_RX) == 0) 
		{
#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
			UAPSD_TIMING_RECORD_START();
			UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_ISR);
#endif // UAPSD_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

			/* mask RxINT */
			rt2860_int_disable(pAd, INT_RX);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->rx_done_work);
#else
			tasklet_hi_schedule(&pObj->rx_done_task);
#endif // WORKQUEUE_BH //
		}
		pAd->int_pending |= INT_RX; 		
#ifdef MULTI_CORE_SUPPORT
         	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
	}

	if (IntSource.word & INT_HCCA_DLY)
	{

#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		if ((pAd->int_disable_mask & INT_HCCA_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_HCCA_DLY);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->hcca_dma_done_work);
#else
			tasklet_hi_schedule(&pObj->hcca_dma_done_task);
#endif // WORKQUEUE_BH //
		}
		pAd->int_pending |= INT_HCCA_DLY;						
#ifdef MULTI_CORE_SUPPORT
	        RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
	}

	if (IntSource.word & INT_AC3_DLY)
	{

#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		if ((pAd->int_disable_mask & INT_AC3_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC3_DLY);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->ac3_dma_done_work);
#else
			tasklet_hi_schedule(&pObj->ac3_dma_done_task);
#endif // WORKQUEUE_BH //
		}
		pAd->int_pending |= INT_AC3_DLY;						
#ifdef MULTI_CORE_SUPPORT
         	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
	}

	if (IntSource.word & INT_AC2_DLY)
	{

#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		if ((pAd->int_disable_mask & INT_AC2_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC2_DLY);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->ac2_dma_done_work);
#else
			tasklet_hi_schedule(&pObj->ac2_dma_done_task);
#endif // WORKQUEUE_BH //
		}
		pAd->int_pending |= INT_AC2_DLY;						
#ifdef MULTI_CORE_SUPPORT
    	        RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
	}

	if (IntSource.word & INT_AC1_DLY)
	{

#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		pAd->int_pending |= INT_AC1_DLY;						

		if ((pAd->int_disable_mask & INT_AC1_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC1_DLY);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->ac1_dma_done_work);		
#else
			tasklet_hi_schedule(&pObj->ac1_dma_done_task);
#endif // WORKQUEUE_BH //
		}
#ifdef MULTI_CORE_SUPPORT
	        RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		
	}

	if (IntSource.word & INT_AC0_DLY)
	{

/*
		if (IntSource.word & 0x2) {
			u32 reg;
			RTMP_IO_READ32(pAd, DELAY_INT_CFG, &reg);
			printk("IntSource.word = %08x, DELAY_REG = %08x\n", IntSource.word, reg);
		}
*/
#ifdef MULTI_CORE_SUPPORT
		RTMP_INT_LOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
		pAd->int_pending |= INT_AC0_DLY;

		if ((pAd->int_disable_mask & INT_AC0_DLY) == 0) 
		{
			/* mask TxDataInt */
			rt2860_int_disable(pAd, INT_AC0_DLY);
#ifdef WORKQUEUE_BH
			schedule_work(&pObj->ac0_dma_done_work);
#else
			tasklet_hi_schedule(&pObj->ac0_dma_done_task);
#endif // WORKQUEUE_BH //
		}
#ifdef MULTI_CORE_SUPPORT
	        RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#endif // MULTI_CORE_SUPPORT //
	}

#ifdef CONFIG_AP_SUPPORT
#if defined(CARRIER_DETECTION_SUPPORT)  || defined(DFS_HARDWARE_SUPPORT) 
#if defined(TONE_RADAR_DETECT_SUPPORT)  || defined (RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
	if (IntSource.word & INT_TONE_RADAR)
	{
		RTMPHandleRadarInterrupt(pAd);
	}
#endif // define(CARRIER_DETECTION_SUPPORT)  || define(DFS_HARDWARE_SUPPORT)  //
#ifdef DFS_HWTIMER_SUPPORT
if (IntSource.word & GPTimeOutInt)
	{
#ifdef DFS_1_SUPPORT
	TimerCB(pAd);
#endif // DFS_1_SUPPORT //
#ifdef DFS_2_SUPPORT
	NewTimerCB_Radar(pAd);
#endif // DFS_1_SUPPORT //
	}
#endif // DFS_HWTIMER_SUPPORT //
#endif // define( TONE_RADAR_DETECT_SUPPORT) || defined (RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //
#endif // CONFIG_AP_SUPPORT //

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
		if (IntSource.word & McuCommand)
		{
			RTMPHandleMcuInterrupt(pAd);
		}
	}

#endif // CONFIG_AP_SUPPORT //



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
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word); // write 1 to clear
	}	
	if (IntSource.word != 0) 
	{		
		goto redo;
	}	
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	return  IRQ_HANDLED;
#endif

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
	IN VOID *Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)pTask->priv;
	
	RtmpOSTaskCustomize(pTask);

	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (pAd && pAd->CmdQ.CmdQState == RTMP_TASK_STAT_RUNNING)
	{
#ifdef KTHREAD_SUPPORT
		RTMP_WAIT_EVENT_INTERRUPTIBLE(pAd, pTask);
#else
		/* lock the device pointers */
		RTMP_SEM_EVENT_WAIT(&(pTask->taskSema), status);

		if (status != 0)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
#endif

		if (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

		if (!pAd->PM_FlgSuspend)
			CMDHandler(pAd);
	}

	if (pAd && !pAd->PM_FlgSuspend)
	{	// Clear the CmdQElements.
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

#ifndef KTHREAD_SUPPORT
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	complete_and_exit (&pTask->taskComplete, 0);
#endif
	return 0;

}

