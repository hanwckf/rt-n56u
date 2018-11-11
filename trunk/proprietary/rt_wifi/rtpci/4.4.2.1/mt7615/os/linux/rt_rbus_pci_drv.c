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

static void tr_done_tasklet(unsigned long data);

#ifdef BB_SOC
__IMEM void rx_done_tasklet(unsigned long data);
#else
static void rx_done_tasklet(unsigned long data);
#endif

#ifdef CONFIG_ANDES_SUPPORT
static void rx1_done_tasklet(unsigned long data);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
static void bcn_dma_done_tasklet(unsigned long data);
static void bmc_dma_done_tasklet(unsigned long data);
#ifdef ERR_RECOVERY
static void mt_mac_recovery_tasklet(unsigned long data);
#endif /* ERR_RECOVERY */
#ifdef CONFIG_FWOWN_SUPPORT
static void mt_mac_fw_own_tasklet(unsigned long data);
#endif /* CONFIG_FWOWN_SUPPORT */

#endif /* MT_MAC */

static void mgmt_dma_done_tasklet(unsigned long data);
static void ac0_dma_done_tasklet(unsigned long data);
#ifdef CONFIG_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data);
#endif /* CONFIG_ATE */
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
NDIS_STATUS RtmpNetTaskInit(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;


	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx_done_task, rx_done_tasklet, (unsigned long)pAd);

#ifdef CONFIG_ANDES_SUPPORT
	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx1_done_task, rx1_done_tasklet, (unsigned long)pAd);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	RTMP_OS_TASKLET_INIT(pAd, &pObj->bcn_dma_done_task, bcn_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_0_task, mt_mac_int_0_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_1_task, mt_mac_int_1_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_2_task, mt_mac_int_2_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_3_task, mt_mac_int_3_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_4_task, mt_mac_int_4_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->bmc_dma_done_task, bmc_dma_done_tasklet, (unsigned long)pAd);
#ifdef ERR_RECOVERY
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mac_error_recovey_task, mt_mac_recovery_tasklet, (unsigned long)pAd);
#endif /* ERR_RECOVERY */
#ifdef CONFIG_FWOWN_SUPPORT
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_fw_own_task, mt_mac_fw_own_tasklet, (unsigned long)pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
#endif /* MT_MAC */

	RTMP_OS_TASKLET_INIT(pAd, &pObj->tr_done_task, tr_done_tasklet, (unsigned long)pAd);

	RTMP_OS_TASKLET_INIT(pAd, &pObj->mgmt_dma_done_task, mgmt_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac0_dma_done_task, ac0_dma_done_tasklet, (unsigned long)pAd);

#ifdef CONFIG_ATE
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ate_ac0_dma_done_task, ate_ac0_dma_done_tasklet, (unsigned long)pAd);
#endif /* CONFIG_ATE */

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


void RtmpNetTaskExit(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_OS_TASKLET_KILL(&pObj->rx_done_task);
#ifdef CONFIG_ANDES_SUPPORT
	RTMP_OS_TASKLET_KILL(&pObj->rx1_done_task);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
#ifdef ERR_RECOVERY
	RTMP_OS_TASKLET_KILL(&pObj->mac_error_recovey_task);
#endif /* ERR_RECOVERY */
#ifdef CONFIG_FWOWN_SUPPORT
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_fw_own_task);
#endif /* CONFIG_FWOWN_SUPPORT */
	RTMP_OS_TASKLET_KILL(&pObj->bcn_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_0_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_1_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_2_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_3_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_4_task);
	RTMP_OS_TASKLET_KILL(&pObj->bmc_dma_done_task);
#endif /* MT_MAC */

	RTMP_OS_TASKLET_KILL(&pObj->tr_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->mgmt_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac0_dma_done_task);
#ifdef CONFIG_ATE
	RTMP_OS_TASKLET_KILL(&pObj->ate_ac0_dma_done_task);
#endif /* CONFIG_ATE */
	RTMP_OS_TASKLET_KILL(&pObj->ac1_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac2_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac3_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->hcca_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->tbtt_task);
	RTMP_OS_TASKLET_KILL(&pObj->fifo_statistic_full_task);
#ifdef UAPSD_SUPPORT
	RTMP_OS_TASKLET_KILL(&pObj->uapsd_eosp_sent_task);
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		RTMP_OS_TASKLET_KILL(&pObj->dfs_task);
#endif /* DFS_SUPPORT */

	}
#endif /* CONFIG_AP_SUPPORT */
}


NDIS_STATUS RtmpMgmtTaskInit(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;

	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;    

	NdisAllocateSpinLock(pAd, &pAd->CmdQLock); //wilsonl, to move to other place

	NdisAcquireSpinLock(&pAd->CmdQLock);
	RTInitializeCmdQ(&pAd->CmdQ);
	NdisReleaseSpinLock(&pAd->CmdQLock);
    
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
VOID RtmpMgmtTaskExit(RTMP_ADAPTER *pAd)
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
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Kill command task fail!\n"));
		}
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
	}

#ifdef WSC_INCLUDED
	WscThreadExit(pAd);
#endif /* WSC_INCLUDED */

	return;
}


static inline void rt2860_int_enable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;

	// TODO: shiang-7603

	pAd->PciHif.intDisableMask &= ~(mode);
	regValue = pAd->PciHif.IntEnableReg & ~(pAd->PciHif.intDisableMask);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
	{
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		HIF_IO_WRITE32(pAd, MT_INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
#if (CFG_CPU_LOADING_DISABLE_IOREAD == 0)
		HIF_IO_READ32(pAd, MT_INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif //(CFG_CPU_LOADING_DISABLE_IOREAD == 0)
#endif /* RTMP_MAC_PCI */
	}
#endif /* MT_MAC */

	if (regValue != 0) {
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
	}
}


static inline void rt2860_int_disable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;
	// TODO: shiang-7603



	pAd->PciHif.intDisableMask |= mode;
	regValue = pAd->PciHif.IntEnableReg & ~(pAd->PciHif.intDisableMask);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
	{
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		HIF_IO_WRITE32(pAd, MT_INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
		HIF_IO_READ32(pAd, MT_INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
	}
#endif /* MT_MAC */

	if (regValue == 0) {
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
	}
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
	UINT32 INT_MGMT_DLY = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_MGMT_DLY = RLT_INT_MGMT_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_MGMT_DLY = RTMP_INT_MGMT_DLY;
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_MGMT_DLY = MT_INT_MGMT_DLY;
#endif /* MT_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~INT_MGMT_DLY;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
    if (IsStopingPdma(&pAd->ErrRecoveryCtl))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->PciHif.intDisableMask &= ~INT_MGMT_DLY;
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }
#endif /* ERR_RECOVERY */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~INT_MGMT_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	RTMPHandleMgmtRingDmaDoneInterrupt(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	if (pAd->PciHif.IntPending & INT_MGMT_DLY)
	{
		/* double check to avoid lose of interrupts */
		pObj = (POS_COOKIE) pAd->OS_Cookie;
		RTMP_OS_TASKLET_SCHE(&pObj->mgmt_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_MGMT_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

static VOID tr_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;
	unsigned long flags;
	PCI_HIF_T *pci_hif = &pAd->PciHif;
	UINT32 int_pending;
	BOOLEAN	reschedule_rx0 = pAd->reschedule_rx0;
	BOOLEAN reschedule_rx1 = pAd->reschedule_rx1;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		pci_hif->intDisableMask &= ~(MT_INT_RX_CMD | MT_INT_RX_DATA | MT_INT_RX_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	int_pending = pci_hif->IntPending;
	pci_hif->IntPending &= ~(MT_INT_RX_CMD | MT_INT_RX_DATA | MT_INT_RX_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	if (pAd->BATable.ba_timeout_check)
		ba_timeout_flush(pAd);

	if ((int_pending & MT_INT_RX_DATA) || reschedule_rx0) {
		reschedule_rx0 = rtmp_rx_done_handle(pAd);
		reschedule_rx1 = RxRing1DoneInterruptHandle(pAd);
	} else if ((int_pending & MT_INT_RX_CMD) || reschedule_rx1) {
		reschedule_rx1 = RxRing1DoneInterruptHandle(pAd);
		reschedule_rx0 = rtmp_rx_done_handle(pAd);
	}

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	if (reschedule_rx0 || reschedule_rx1) {
		pAd->reschedule_rx0 = reschedule_rx0;
		pAd->reschedule_rx1 = reschedule_rx1;
		RTMP_OS_TASKLET_SCHE(&pObj->tr_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, MT_INT_RX_CMD | MT_INT_RX_DATA | MT_INT_RX_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

#ifdef BB_SOC
__IMEM void rx_done_tasklet(unsigned long data)
#else
static void rx_done_tasklet(unsigned long data)
#endif
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
	UINT32 INT_RX = 0;

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_RX_DONE_TASKLET(__FUNCTION__);
#endif

	MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s():\n", __FUNCTION__));

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_RX = RLT_INT_RX_DATA;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_RX = RTMP_INT_RX;
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_RX = MT_INT_RX_DATA;
#endif /* MT_MAC */

#ifdef CONFIG_BA_REORDER_MONITOR
	if (pAd->BATable.ba_timeout_check) {
		ba_timeout_flush(pAd);	

		if (!((pAd->PciHif.IntPending & INT_RX) || 
				(pAd->PciHif.intDisableMask & INT_RX)))
			return;
	}
#endif

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	if (RTMP_TEST_FLAG(pAd,  fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(INT_RX);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
#ifdef UAPSD_SUPPORT
	UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_TASKLET);
#endif /* UAPSD_SUPPORT */

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~(INT_RX);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = rtmp_rx_done_handle(pAd);

#ifdef UAPSD_SUPPORT
	UAPSD_TIMING_RECORD_STOP();
#endif /* UAPSD_SUPPORT */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid rotting packet  */
	if (pAd->PciHif.IntPending & INT_RX || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->rx_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_RX);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

}


#ifdef CONFIG_ANDES_SUPPORT
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
	UINT32 int_mask = 0;

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_RX1_DONE_TASKLET(__FUNCTION__);
#endif

#ifdef MT_MAC
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT)
		int_mask = MT_INT_RX_CMD;
#endif /* MT_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		int_mask = RLT_INT_RX_CMD;
#endif /* RLT_MAC */


#if defined(RLT_MAC) || defined(MT_MAC)
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	//if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(int_mask);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~(int_mask);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RxRing1DoneInterruptHandle(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid rotting packet  */
	if ((pAd->PciHif.IntPending & int_mask) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->rx1_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	/* enable Rx1INT again */
	rt2860_int_enable(pAd, int_mask);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
#endif /* defined(RLT_MAC) || defined(MT_MAC) */

}
#endif /* CONFIG_ANDES_SUPPORT */


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
	UINT32 FifoStaFullInt = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		FifoStaFullInt = RLT_FifoStaFullInt;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		FifoStaFullInt = RTMP_FifoStaFullInt;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	 {
		  RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		  pAd->PciHif.intDisableMask &= ~(FifoStaFullInt);
		  RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
 	  }

    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~(FifoStaFullInt);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	NICUpdateFifoStaCounters(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/*
	 * double check to avoid rotting packet
	 */
	if (pAd->PciHif.IntPending & FifoStaFullInt)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->fifo_statistic_full_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, FifoStaFullInt);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

}


#ifdef MT_MAC
static void bcn_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, bcn_dma_done_tasklet);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	BOOLEAN bReschedule = 0;
	UINT32 INT_BCN_DLY = MT_INT_BCN_DLY;



	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(INT_BCN_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~INT_BCN_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	RTMPHandleBcnDmaDoneInterrupt(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & INT_BCN_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->bcn_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_BCN_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}
#endif /* MT_MAC */


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
	BOOLEAN bReschedule = 0;
	UINT32 INT_HCCA_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_HCCA_DLY = MT_INT_CMD;
#endif

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_HCCA_DLY = RLT_INT_HCCA_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_HCCA_DLY = RTMP_INT_HCCA_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~INT_HCCA_DLY;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
    if (IsStopingPdma(&pAd->ErrRecoveryCtl))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->PciHif.intDisableMask &= ~INT_HCCA_DLY;
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }
#endif /* ERR_RECOVERY */


	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~INT_HCCA_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#if defined(RLT_MAC) || defined(MT_MAC)
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_RLT || pAd->chipCap.hif_type == HIF_MT)
		RTMPHandleTxRing8DmaDoneInterrupt(pAd);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_HCCA_DONE);
#endif /* RTMP_MAC */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & INT_HCCA_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->hcca_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

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
	BOOLEAN bReschedule = 0;
	UINT32 INT_AC3_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC3_DLY = MT_INT_AC3_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC3_DLY = RLT_INT_AC3_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC3_DLY = RTMP_INT_AC3_DLY;
#endif /* RTMP_MAC */


	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(INT_AC3_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
    if (IsStopingPdma(&pAd->ErrRecoveryCtl))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->PciHif.intDisableMask &= ~(INT_AC3_DLY);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }
#endif /* ERR_RECOVERY */


	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~INT_AC3_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#if defined(MT7615) || defined(MT7622)
    bReschedule = RTMPHandleFwDwloCmdRingDmaDoneInterrupt(pAd);
#else
	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC3_DONE);
#endif /* defined(MT7615) || defined(MT7622) */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & INT_AC3_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac3_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

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
	BOOLEAN bReschedule = 0;
	UINT32 INT_AC2_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC2_DLY = MT_INT_AC2_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC2_DLY = RLT_INT_AC2_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC2_DLY = RTMP_INT_AC2_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(INT_AC2_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
    if (IsStopingPdma(&pAd->ErrRecoveryCtl))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->PciHif.intDisableMask &= ~(INT_AC2_DLY);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }
#endif /* ERR_RECOVERY */


	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~INT_AC2_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#if defined(MT7615) || defined(MT7622)
	RTMPHandleTxRing8DmaDoneInterrupt(pAd);
#else
	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC2_DONE);
#endif

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & INT_AC2_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac2_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_AC2_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


static void ac1_dma_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac1_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	UINT32 INT_AC1_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC1_DLY = MT_INT_AC1_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC1_DLY = RLT_INT_AC1_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC1_DLY = RTMP_INT_AC1_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(INT_AC1_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
    if (IsStopingPdma(&pAd->ErrRecoveryCtl))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->PciHif.intDisableMask &= ~(INT_AC1_DLY);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }
#endif /* ERR_RECOVERY */

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~INT_AC1_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC1_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & INT_AC1_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac1_dma_done_task);

		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_AC1_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


static void ac0_dma_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac0_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	UINT32 INT_AC0_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC0_DLY = MT_INT_AC0_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC0_DLY = RLT_INT_AC0_DLY;
#endif /* RLT_MAC*/
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC0_DLY = RTMP_INT_AC0_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(INT_AC0_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
    if (IsStopingPdma(&pAd->ErrRecoveryCtl))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->PciHif.intDisableMask &= ~(INT_AC0_DLY);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }
#endif /* ERR_RECOVERY */

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~INT_AC0_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & INT_AC0_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac0_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->PciHif.IntPending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, INT_AC0_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


#ifdef MT_MAC
void mt_mac_int_0_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_0_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(WF_MAC_INT_0);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~WF_MAC_INT_0;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & WF_MAC_INT_0) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_0_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->PciHif.IntPending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_0);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_1_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_1_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(WF_MAC_INT_1);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~WF_MAC_INT_1;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & WF_MAC_INT_1) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_1_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->PciHif.IntPending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_1);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_2_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_2_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(WF_MAC_INT_2);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~WF_MAC_INT_2;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & WF_MAC_INT_2) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_2_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->PciHif.IntPending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_2);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_3_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_3_task);
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
		pAd->PciHif.intDisableMask &= ~(WF_MAC_INT_3);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():HALT in progress(Flags=%lu)!\n", __FUNCTION__, pAd->Flags));
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~WF_MAC_INT_3;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	RTMPHandlePreTBTTInterrupt(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & WF_MAC_INT_3) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_3_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

//	rt2860_int_enable(pAd, WF_MAC_INT_3);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_4_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_4_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(WF_MAC_INT_4);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->PciHif.IntPending &= ~WF_MAC_INT_4;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->PciHif.IntPending & WF_MAC_INT_4) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_4_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->PciHif.IntPending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_4);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

static void bmc_dma_done_tasklet(unsigned long data)
{
    unsigned long flags;
    BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
    struct work_struct *work = (struct work_struct *)data;
    POS_COOKIE pObj = container_of(work, struct os_cookie, bmc_dma_done_task);
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
    POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
    UINT32 INT_BMC_DLY = MT_INT_BMC_DLY;

	//printk("==>bmc tasklet\n");
    /* Do nothing if the driver is starting halt state. */
    /* This might happen when timer already been fired before cancel timer with mlmehalt */
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->PciHif.intDisableMask &= ~(INT_BMC_DLY);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->PciHif.intDisableMask &= ~(INT_BMC_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
#endif /* ERR_RECOVERY */

	pObj = (POS_COOKIE) pAd->OS_Cookie;

    RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
    pAd->PciHif.IntPending &= ~INT_BMC_DLY;
    RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

    bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_BMC_DONE);

    RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
    /* double check to avoid lose of interrupts */
    if ((pAd->PciHif.IntPending & INT_BMC_DLY) || bReschedule)
    {
        RTMP_OS_TASKLET_SCHE(&pObj->bmc_dma_done_task);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
            __FUNCTION__, pAd->PciHif.IntPending, bReschedule));
        return;
    }

    rt2860_int_enable(pAd, INT_BMC_DLY);
    RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

#ifdef ERR_RECOVERY
static void mt_mac_recovery_tasklet(unsigned long data)
{
    unsigned long Flags;
    UINT32 status;

#ifdef WORKQUEUE_BH
    struct work_struct *work = (struct work_struct *)data;
    POS_COOKIE pObj = container_of(work, struct os_cookie, bmc_dma_done_task);
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif /* WORKQUEUE_BH */
    UINT32 INT_MCU_CMD = MT_McuCommand;

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->DriverOwnLock, &Flags);
    status = pAd->ErrRecoveryCtl.status;
    pAd->ErrRecoveryCtl.status = 0;
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->DriverOwnLock, &Flags);

    RTMP_MAC_RECOVERY(pAd, status);
    rt2860_int_enable(pAd, INT_MCU_CMD);
}
#endif /* ERR_RECOVERY */

#ifdef CONFIG_FWOWN_SUPPORT
static void mt_mac_fw_own_tasklet(unsigned long data)
{
	//unsigned long flags;
	//BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_fw_own_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif /* WORKQUEUE_BH */
	UINT32 INT_FW_CLEAR_OWN = MT_FW_CLEAR_OWN_BIT;

	/* Unmask interrupt */
	rt2860_int_enable(pAd, INT_FW_CLEAR_OWN);
}
#endif /* CONFIG_FWOWN_SUPPORT */

#endif /* MT_MAC */


#ifdef CONFIG_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data)
{
	return;
}
#endif /* CONFIG_ATE */


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

	RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
}
#endif /* UAPSD_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
void schedule_dfs_task(RTMP_ADAPTER *pAd)
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

		APStopByRf(pAd, RFIC_5GHZ);
		APStartUpByRf(pAd, RFIC_5GHZ);
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

#ifdef ERR_RECOVERY
void RTMPHandleInterruptSerDump(RTMP_ADAPTER *pAd)
{
	UINT32 reg_tmp_val;

	MAC_IO_READ32(pAd, MCU_COM_REG1, &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, MCU_COM_REG1=0x%08X\n", __FUNCTION__, reg_tmp_val));

	if (reg_tmp_val == MCU_COM_REG1_SER_PSE) {
		MAC_IO_READ32(pAd, PP_SPARE_DUMMY_CR5, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206c064=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, PP_SPARE_DUMMY_CR6, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206c068=0x%08X\n", __FUNCTION__, reg_tmp_val));	       
		MAC_IO_READ32(pAd, PP_SPARE_DUMMY_CR7, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206c06C=0x%08X\n", __FUNCTION__, reg_tmp_val));

		MAC_IO_READ32(pAd, 0x8244, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x82060244=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x8248, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x82060248=0x%08X\n", __FUNCTION__, reg_tmp_val));	       
		MAC_IO_READ32(pAd, 0x8258, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x82060258=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x825C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206025C=0x%08X\n", __FUNCTION__, reg_tmp_val));
	} else if (reg_tmp_val == MCU_COM_REG1_SER_LMAC_TX) { 
		MAC_IO_READ32(pAd, PP_SPARE_DUMMY_CR5, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206c064=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, PP_SPARE_DUMMY_CR6, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206c068=0x%08X\n", __FUNCTION__, reg_tmp_val));	       
		MAC_IO_READ32(pAd, PP_SPARE_DUMMY_CR7, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206c06C=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, PP_SPARE_DUMMY_CR8, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x8206c070=0x%08X\n", __FUNCTION__, reg_tmp_val));
	} else if (reg_tmp_val == MCU_COM_REG1_SER_SEC_RF_RX) {
		MAC_IO_READ32(pAd, 0x21620, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F6020=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x21624, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F6024=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x21628, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F6028=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x2162C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F602C=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x21630, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F6030=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x21634, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F6034=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x21638, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F6038=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x2163C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F603C=0x%08X\n", __FUNCTION__, reg_tmp_val));

		MAC_IO_READ32(pAd, 0x20824, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1024=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20924, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1124=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20a24, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1224=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20b24, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1324=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20820, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1020=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20920, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1120=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20a20, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1220=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20b20, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1320=0x%08X\n", __FUNCTION__, reg_tmp_val));
	
		MAC_IO_READ32(pAd, 0x20840, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1040=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20844, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1044=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20848, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1048=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x2084C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F104C=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20940, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1140=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20944, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1144=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x20948, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F1148=0x%08X\n", __FUNCTION__, reg_tmp_val));
		MAC_IO_READ32(pAd, 0x2094C, &reg_tmp_val);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820F114C=0x%08X\n", __FUNCTION__, reg_tmp_val));
	}   
	
	MAC_IO_READ32(pAd, PSE_SPARE_DUMMY_CR1, &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820681e4=0x%08X\n", __FUNCTION__, reg_tmp_val));
	
	MAC_IO_READ32(pAd, PSE_SPARE_DUMMY_CR2, &reg_tmp_val);	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820681e8=0x%08X\n", __FUNCTION__, reg_tmp_val));
	
	MAC_IO_READ32(pAd, PSE_SPARE_DUMMY_CR3, &reg_tmp_val);	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820682e8=0x%08X\n", __FUNCTION__, reg_tmp_val));
	
	MAC_IO_READ32(pAd, PSE_SPARE_DUMMY_CR4, &reg_tmp_val);	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820682ec=0x%08X\n", __FUNCTION__, reg_tmp_val));
	
	MAC_IO_READ32(pAd, (0x20afc), &reg_tmp_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R	, 0x820f20fc=0x%08X\n", __FUNCTION__, reg_tmp_val));

	/* Add the EDCCA Time for Debug */
	Show_MibBucket_Proc(pAd,"");
}
#endif // ERR_RECOVERY


int print_int_count = 0;
VOID RTMPHandleInterrupt(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT32 IntSource;
	UINT32 StatusRegister = 0, EnableRegister = 0, stat_reg = 0, en_reg = 0;
	POS_COOKIE pObj;
	unsigned long flags=0;
	UINT32 INT_RX_DATA = 0, INT_RX_CMD=0, TxCoherent = 0, RxCoherent = 0, FifoStaFullInt = 0;
	UINT32 INT_MGMT_DLY = 0, INT_HCCA_DLY = 0, INT_AC3_DLY = 0, INT_AC2_DLY = 0, INT_AC1_DLY = 0, INT_AC0_DLY = 0, INT_BMC_DLY = 0, INT_RX_DLY = 0;
	UINT32 RadarInt = 0;
#ifdef MT_MAC
	UINT32 McuCommand = 0;
	UINT32 FwClrOwnInt = 0;
#endif


#if (CFG_CPU_LOADING_DISABLE_TXDONE0 == 1)
    UINT32 IntSourceMajor = 0;
#endif

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/*
		Note 03312008: we can not return here before

		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource.word);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word);
		Or kernel will panic after ifconfig ra0 down sometimes
	*/

	/* Inital the Interrupt source. */
	IntSource = 0x00000000L;

	/*
		Flag fOP_STATUS_DOZE On, means ASIC put to sleep, elase means ASICK WakeUp
		And at the same time, clock maybe turned off that say there is no DMA service.
		when ASIC get to sleep.
		To prevent system hang on power saving.
		We need to check it before handle the INT_SOURCE_CSR, ASIC must be wake up.

		RT2661 => when ASIC is sleeping, MAC register cannot be read and written.
		RT2860 => when ASIC is sleeping, MAC register can be read and written.
	*/
	/* if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)) */
#ifdef MT_MAC

	HIF_IO_READ32(pAd,MT_INT_SOURCE_CSR,&IntSource);

	if (IntSource & WF_MAC_INT_3)
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        RTMP_IO_READ32(pAd, HWISR3, &stat_reg);
        RTMP_IO_READ32(pAd, HWIER3, &en_reg);
        /* disable the interrupt source */
		RTMP_IO_WRITE32(pAd, HWIER3, (~(stat_reg & en_reg)));
        /* write 1 to clear */
        RTMP_IO_WRITE32(pAd, HWISR3, stat_reg);
       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
	}
	HIF_IO_WRITE32(pAd,MT_INT_SOURCE_CSR,IntSource); /*write 1 to clear*/

#endif /* MT_MAC */

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
#ifdef MT_MAC

	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
		IntSource = IntSource & (MT_INT_CMD | MT_INT_RX | WF_MAC_INT_3);


		if (!IntSource)
		    return;
#endif /* MT_MAC */
    }

#ifdef MT_MAC
	{
		INT_RX_DATA = MT_INT_RX_DATA;
		INT_RX_CMD = MT_INT_RX_CMD;
		INT_RX_DLY = MT_INT_RX_DLY;
		INT_MGMT_DLY = MT_INT_MGMT_DLY;
		INT_HCCA_DLY = MT_INT_CMD;
		INT_AC3_DLY = MT_INT_AC3_DLY;
		INT_AC2_DLY = MT_INT_AC2_DLY;
		INT_AC1_DLY = MT_INT_AC1_DLY;
		INT_AC0_DLY = MT_INT_AC0_DLY;
		INT_BMC_DLY = MT_INT_BMC_DLY;
		TxCoherent = MT_TxCoherent;
		RxCoherent = MT_RxCoherent;

//		PreTBTTInt = MT_PreTBTTInt;
//		TBTTInt = MT_TBTTInt;
//		FifoStaFullInt = MT_FifoStaFullInt;
//		GPTimeOutInt = MT_GPTimeOutInt;
		RadarInt = 0;
//		AutoWakeupInt = MT_AutoWakeupInt;
//#ifdef ERR_RECOVERY
		McuCommand = MT_McuCommand;
//#endif /* ERR_RECOVERY */
		FwClrOwnInt = MT_FW_CLR_OWN_INT;
	}
#endif /* MT_MAC*/

	pAd->bPCIclkOff = FALSE;

	/* If required spinlock, each ISR has to acquire and release itself. */

	/* Do nothing if NIC doesn't exist */
	if (IntSource == 0xffffffff)
	{
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
		return;
	}

#ifdef INT_STATISTIC
	pAd->INTCNT++;
#ifdef MT_MAC

	if (IntSource & WF_MAC_INT_0)
	{
		pAd->INTWFMACINT0CNT++;
	}
	if (IntSource & WF_MAC_INT_1)
	{
		pAd->INTWFMACINT1CNT++;
	}
	if (IntSource & WF_MAC_INT_2)
	{
		pAd->INTWFMACINT2CNT++;
	}
	if (IntSource & WF_MAC_INT_3)
	{
		pAd->INTWFMACINT3CNT++;
	}
	if (IntSource & WF_MAC_INT_4)
	{
		pAd->INTWFMACINT4CNT++;
	}
	if (IntSource & MT_INT_BCN_DLY)
	{
		pAd->INTBCNDLY++;
	}
	if (IntSource & INT_BMC_DLY)
	{
		pAd->INTBMCDLY++;
	}

#endif
	if (IntSource & TxCoherent)
	{
		pAd->INTTxCoherentCNT++;
	}
	if (IntSource & RxCoherent)
	{
		pAd->INTRxCoherentCNT++;

	}
	if (IntSource & FifoStaFullInt)
	{
		pAd->INTFifoStaFullIntCNT++;
	}
	if (IntSource & INT_MGMT_DLY)
	{
		pAd->INTMGMTDLYCNT++;
	}
	if (IntSource & INT_RX_DATA)
	{
		pAd->INTRXDATACNT++;
	}
#ifdef CONFIG_ANDES_SUPPORT
	if (IntSource & INT_RX_CMD)
	{
		pAd->INTRXCMDCNT++;
	}
#endif
	if (IntSource & INT_HCCA_DLY)
	{
		pAd->INTHCCACNT++;
	}
	if (IntSource & INT_AC3_DLY)
	{
		pAd->INTAC3CNT++;
	}
	if (IntSource & INT_AC2_DLY)
	{
		pAd->INTAC2CNT++;
	}
	if (IntSource & INT_AC1_DLY)
	{
		pAd->INTAC1CNT++;
	}
	if (IntSource & INT_AC0_DLY)
	{
		pAd->INTAC0CNT++;

	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		if (IntSource & GPTimeOutInt){
			pAd->INTGPTimeOutCNT++;
		}
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
		if ((IntSource & RadarInt))
		{
			pAd->INTRadarCNT++;
		}
#endif /* CARRIER_DETECTION_SUPPORT*/
	}
#endif

#endif

//+++Add by Carter
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		if (IntSource & WF_MAC_INT_0)
		{	//Check HWISR0
			RTMP_IO_READ32(pAd, HWISR0, &StatusRegister);
			RTMP_IO_READ32(pAd, HWIER0, &EnableRegister);
			/* disable the interrupt source */
			RTMP_IO_WRITE32(pAd, HWIER0, (~StatusRegister & EnableRegister));

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): "
				"StatusRegister 0x2443c = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & WF_MAC_INT_1)
		{	//Check HWISR1
			RTMP_IO_READ32(pAd, HWISR1, &StatusRegister);
			RTMP_IO_READ32(pAd, HWIER1, &EnableRegister);
			/* disable the interrupt source */
			RTMP_IO_WRITE32(pAd, HWIER1, (~StatusRegister & EnableRegister));

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): "
				"StatusRegister 0x24444 = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & WF_MAC_INT_2)
		{	//Check HWISR2
			RTMP_IO_READ32(pAd, HWISR2, &StatusRegister);
			RTMP_IO_READ32(pAd, HWIER2, &EnableRegister);
			/* disable the interrupt source */
			RTMP_IO_WRITE32(pAd, HWIER2, (~StatusRegister & EnableRegister));

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): "
				"StatusRegister 0x2444c = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & WF_MAC_INT_3)
		{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
			if ((pAd->PciHif.intDisableMask & WF_MAC_INT_3) == 0)
			{
                rt2860_int_disable(pAd, WF_MAC_INT_3);
                RTMP_IO_WRITE32(pAd, HWIER3, en_reg);
                if (stat_reg & BIT31) {
                    RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_3_task);
                }
                rt2860_int_enable(pAd, WF_MAC_INT_3);
			}
			pAd->PciHif.IntPending |= WF_MAC_INT_3;
	       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		}

		if (IntSource & WF_MAC_INT_4)
		{	//Check HWISR4
			RTMP_IO_READ32(pAd, HWISR4, &StatusRegister);
			//RTMP_IO_WRITE32(pAd, 0x60310054, StatusRegister);
			/* write 1 to clear */
			RTMP_IO_READ32(pAd, HWIER4, &EnableRegister);
			/* disable the interrupt source */
			RTMP_IO_WRITE32(pAd, HWIER4, (~StatusRegister & EnableRegister));

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s(): "
				"StatusRegister 0x2445c = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & MT_INT_BCN_DLY)
		{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
			pAd->TxDMACheckTimes = 0;
			if ((pAd->PciHif.intDisableMask & MT_INT_BCN_DLY) == 0)
			{
				rt2860_int_disable(pAd, MT_INT_BCN_DLY);
				RTMP_OS_TASKLET_SCHE(&pObj->bcn_dma_done_task);
			}
			pAd->PciHif.IntPending |= MT_INT_BCN_DLY;
	       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		}

		if (IntSource & INT_BMC_DLY)
    	{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
			pAd->TxDMACheckTimes = 0;
            if ((pAd->PciHif.intDisableMask & INT_BMC_DLY) == 0)
            {
                rt2860_int_disable(pAd, INT_BMC_DLY);
                RTMP_OS_TASKLET_SCHE(&pObj->bmc_dma_done_task);
            }
            pAd->PciHif.IntPending |= INT_BMC_DLY;
            RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
    	}
	}
#endif /* MT_MAC */
//---Add by Carter

	if (IntSource & TxCoherent)
	{
		/*
			When the interrupt occurs, it means we kick a register to send
			a packet, such as TX_MGMTCTX_IDX, but MAC finds some fields in
			the transmit buffer descriptor is not correct, ex: all zeros.
		*/
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>TxCoherent<<<\n"));
		RTMPHandleRxCoherentInterrupt(pAd);
//+++Add by shiang for debug
		IntSourceMajor |= TxCoherent;

//---Add by shiang for debug
	}

	if (IntSource & RxCoherent)
	{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>RxCoherent<<<\n"));
		RTMPHandleRxCoherentInterrupt(pAd);
//+++Add by shiang for debug
		IntSourceMajor |= RxCoherent;
//---Add by shiang for debug
	}

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	if (IntSource & FifoStaFullInt)
	{
		if ((pAd->PciHif.intDisableMask & FifoStaFullInt) == 0)
		{
			IntSourceMajor |= FifoStaFullInt;
			RTMP_OS_TASKLET_SCHE(&pObj->fifo_statistic_full_task);
		}
		pAd->PciHif.IntPending |= FifoStaFullInt;
	}

	if (IntSource & INT_MGMT_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->PciHif.intDisableMask & INT_MGMT_DLY) ==0)
		{
			IntSourceMajor |= INT_MGMT_DLY;
			RTMP_OS_TASKLET_SCHE(&pObj->mgmt_dma_done_task);
		}
		pAd->PciHif.IntPending |= INT_MGMT_DLY ;
	}


	if (IntSource & INT_RX_DLY) {
		if ((!(IntSource & INT_RX_CMD)) && (!(IntSource & INT_RX_DATA)))
			IntSource &= ~INT_RX_DLY;
	}

	if (IntSource & INT_RX_DATA)
	{
		pAd->RxDMACheckTimes = 0;
		pAd->RxPseCheckTimes = 0;
#ifdef CUT_THROUGH_DBG
		pAd->IsrRxCnt++;
#endif
		IntSourceMajor |= INT_RX_DATA;

		if ((pAd->PciHif.intDisableMask & INT_RX_DATA) == 0)
		{
#ifdef UAPSD_SUPPORT
			UAPSD_TIMING_RECORD_START();
			UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_ISR);
#endif /* UAPSD_SUPPORT */
		}

		if (!(IntSource & INT_RX_DLY))
			IntSource |= INT_RX_DLY;

		pAd->PciHif.IntPending |= INT_RX_DATA;
	}

#ifdef CONFIG_ANDES_SUPPORT
	if (IntSource & INT_RX_CMD)
	{
		pAd->RxDMACheckTimes = 0;
		pAd->RxPseCheckTimes = 0;
#ifdef CUT_THROUGH_DBG
		pAd->IsrRx1Cnt++;
#endif

		IntSourceMajor |= INT_RX_CMD;

		if (!(IntSource & INT_RX_DLY))
			IntSource |= INT_RX_DLY;

		pAd->PciHif.IntPending |= INT_RX_CMD;
	}
#endif /* CONFIG_ANDES_SUPPORT */

	if (IntSource & INT_RX_DLY) {
		if ((pAd->PciHif.intDisableMask & INT_RX_DLY) == 0)
			RTMP_OS_TASKLET_SCHE(&pObj->tr_done_task);

		IntSourceMajor |= INT_RX_DLY;
		pAd->PciHif.IntPending |= MT_INT_RX_DLY;
	}

	if (IntSource & INT_HCCA_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->PciHif.intDisableMask & INT_HCCA_DLY) == 0)
		{
			IntSourceMajor |= INT_HCCA_DLY;
			RTMP_OS_TASKLET_SCHE(&pObj->hcca_dma_done_task);
		}
		pAd->PciHif.IntPending |= INT_HCCA_DLY;
	}

	if (IntSource & INT_AC3_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->PciHif.intDisableMask & INT_AC3_DLY) == 0)
		{
			IntSourceMajor |= INT_AC3_DLY;
			RTMP_OS_TASKLET_SCHE(&pObj->ac3_dma_done_task);
		}
		pAd->PciHif.IntPending |= INT_AC3_DLY;
	}

	if (IntSource & INT_AC2_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->PciHif.intDisableMask & INT_AC2_DLY) == 0)
		{
			IntSourceMajor |= INT_AC2_DLY;
			RTMP_OS_TASKLET_SCHE(&pObj->ac2_dma_done_task);
		}
		pAd->PciHif.IntPending |= INT_AC2_DLY;
	}

	if (IntSource & INT_AC1_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		pAd->PciHif.IntPending |= INT_AC1_DLY;

		if ((pAd->PciHif.intDisableMask & INT_AC1_DLY) == 0)
		{
			IntSourceMajor |= INT_AC1_DLY;
			RTMP_OS_TASKLET_SCHE(&pObj->ac1_dma_done_task);
		}
	}

	if (IntSource & INT_AC0_DLY)
	{
/*
		if (IntSource.word & 0x2) {
			UINT32 reg;
			RTMP_IO_READ32(pAd, DELAY_INT_CFG, &reg);
			printk("IntSource = %08x, DELAY_REG = %08x\n", IntSource.word, reg);
		}
*/
		pAd->TxDMACheckTimes = 0;
#ifdef CUT_THROUGH_DBG
		pAd->IsrTxCnt++;
#endif
		pAd->PciHif.IntPending |= INT_AC0_DLY;
		if ((pAd->PciHif.intDisableMask & INT_AC0_DLY) == 0)
		{
#if (CFG_CPU_LOADING_DISABLE_TXDONE0 == 1)
            if (IntSource & INT_AC0_DLY)
                IntSourceMajor |= INT_AC0_DLY;
#else
			rt2860_int_disable(pAd, INT_AC0_DLY);
#endif
			RTMP_OS_TASKLET_SCHE(&pObj->ac0_dma_done_task);
		}
	}

#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
    {
        if (IntSource & McuCommand)
        {
            UINT32 value;

            RTMP_IO_READ32(pAd, MT_MCU_CMD_CSR, &value);

#ifdef ERR_RECOVERY
            if (value & ERROR_DETECT_MASK)
            {
				IntSourceMajor |= McuCommand;
                /* updated ErrRecovery Status. */
                pAd->ErrRecoveryCtl.status = value;

                /* Trigger error recovery process with fw reload. */
                RTMP_OS_TASKLET_SCHE(&pObj->mac_error_recovey_task);
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s,::E  R  , status=0x%08X\n", __FUNCTION__, value));
		RTMPHandleInterruptSerDump(pAd);
            }
#endif /* ERR_RECOVERY */

#ifdef CONFIG_FWOWN_SUPPORT
            if ((value & MT_MCU_CMD_CLEAR_FW_OWN) == MT_MCU_CMD_CLEAR_FW_OWN)
            {
                IntSourceMajor |= McuCommand;

				/* Clear MCU CMD status*/
				RTMP_IO_WRITE32(pAd, MT_MCU_CMD_CSR, (value & ~MT_MCU_CMD_CLEAR_FW_OWN));

				/* Interrupt handler */
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s::DriverOwn = TRUE\n", __FUNCTION__));
				pAd->bDrvOwn = TRUE;
				RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_fw_own_task);	
            }
#endif /* CONFIG_FWOWN_SUPPORT */
            /*RTMPHandleMcuInterrupt(pAd);*/
        }

#ifdef CONFIG_FWOWN_SUPPORT
        if (IntSource & FwClrOwnInt) {
            IntSourceMajor |= FwClrOwnInt;
            /* Interrupt handler */
            MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s::DriverOwn = TRUE\n", __FUNCTION__));
            pAd->bDrvOwn = TRUE;
            RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_fw_own_task);	
        }
#endif /* CONFIG_FWOWN_SUPPORT */	
    }
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* MT_MAC */

#if (CFG_CPU_LOADING_DISABLE_TXDONE0 == 1)
		rt2860_int_disable(pAd, IntSourceMajor);
#endif
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		if (IntSource & GPTimeOutInt)
		      NewTimerCB_Radar(pAd);
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
		if ((IntSource & RadarInt))
		{
			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
				RTMPHandleRadarInterrupt(pAd);
		}
#endif /* CARRIER_DETECTION_SUPPORT*/

	}
#endif /* CONFIG_AP_SUPPORT */


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
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,( "<---RTPCICmdThread\n"));

	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}




#if defined(RTMP_MAC) || defined(RLT_MAC)
VOID rlt_pci_wifi_on(RTMP_ADAPTER *pAd, USHORT  device_id)
{

}

VOID rtmp_pci_chip_feature_cfg(RTMP_ADAPTER *pAd, RT_CMD_PCIE_INIT *pConfig, USHORT  id)
{
	if ((id == NIC2860_PCIe_DEVICE_ID) ||
		(id == NIC2790_PCIe_DEVICE_ID) ||
		(id == VEN_AWT_PCIe_DEVICE_ID) ||
		(id == NIC3090_PCIe_DEVICE_ID) ||
		(id == NIC3091_PCIe_DEVICE_ID) ||
		(id == NIC3092_PCIe_DEVICE_ID) ||
		(id == NIC3390_PCIe_DEVICE_ID) ||
		(id == NIC3593_PCI_OR_PCIe_DEVICE_ID)||
		(id == NIC3592_PCIe_DEVICE_ID) ||
		(id ==  NIC5390_PCIe_DEVICE_ID) ||
		(id ==  NIC539F_PCIe_DEVICE_ID) ||
		(id ==  NIC5392_PCIe_DEVICE_ID) ||
		(id ==  NIC5360_PCI_DEVICE_ID) ||
		(id ==  NIC5362_PCI_DEVICE_ID) ||
		(id ==  NIC5392_PCIe_DEVICE_ID) ||
		(id ==  NIC5362_PCI_DEVICE_ID) ||
		(id ==  NIC5592_PCIe_DEVICE_ID))
	{
		UINT32 MacCsr0 = 0;
		WaitForAsicReady(pAd);
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);


		/* Support advanced power save after 2892/2790. */
		/* MAC version at offset 0x1000 is 0x2872XXXX/0x2870XXXX(PCIe, USB, SDIO). */
		if ((MacCsr0 & 0xffff0000) != 0x28600000)
		{
#ifdef PCIE_PS_SUPPORT
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
#endif /* PCIE_PS_SUPPORT */

		}

		pAd->infType = RTMP_DEV_INF_PCIE;
	}
}


VOID rtmp_rlt_pci_chip_cfg(RTMP_ADAPTER *pAd, RT_CMD_PCIE_INIT *pConfig, USHORT  id)
{
        rlt_pci_wifi_on(pAd, id);
        rtmp_pci_chip_feature_cfg(pAd, pConfig, id);
}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */


#ifdef MT_MAC
NDIS_STATUS mt_pci_chip_cfg(RTMP_ADAPTER *pAd, USHORT  id)
{
    NDIS_STATUS ret = NDIS_STATUS_SUCCESS;

    if (id == 0) 
	ret = NDIS_STATUS_FAILURE;

#ifdef CONFIG_FWOWN_SUPPORT
    if ((id == NIC7615_PCIe_DEVICE_ID) || (id == NIC7611_PCIe_DEVICE_ID) || (id == NIC7637_PCIe_DEVICE_ID))
    {
        DriverOwn(pAd);
    }
#endif

    if ((id == NIC7603_PCIe_DEVICE_ID)
        || (id == NIC7615_PCIe_DEVICE_ID)
        || (id == NIC7611_PCIe_DEVICE_ID)
        || (id == NIC7637_PCIe_DEVICE_ID)
    )
    {
        UINT32 Value;

        RTMP_IO_READ32(pAd, TOP_HVR, &Value);
        pAd->HWVersion = Value;
	if (Value == 0) 
	    ret = NDIS_STATUS_FAILURE;

        RTMP_IO_READ32(pAd, TOP_FVR, &Value);
        pAd->FWVersion = Value;
	if (Value == 0) 
	    ret = NDIS_STATUS_FAILURE;

        RTMP_IO_READ32(pAd, TOP_HCR, &Value);
        pAd->ChipID = Value;
	if (Value == 0) 
	    ret = NDIS_STATUS_FAILURE;

        if (IS_MT7603(pAd))
        {
            RTMP_IO_READ32(pAd, STRAP_STA, &Value);
            pAd->AntMode = (Value >> 24) & 0x1;
        }

        pAd->chipCap.hif_type = HIF_MT;
        pAd->infType = RTMP_DEV_INF_PCIE;

        MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, 
                        ("%s(): HWVer=0x%x, FWVer=0x%x, pAd->ChipID=0x%x\n",
                        __FUNCTION__, pAd->HWVersion, pAd->FWVersion, pAd->ChipID));

#ifdef MT7615
        if (IS_MT7615(pAd)) {
            RTMP_IO_READ32(pAd, HIF_SYS_REV, &Value);
	    if (Value == 0) 
		ret = NDIS_STATUS_FAILURE;
            MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, 
                        ("%s(): HIF_SYS_REV=0x%x\n", __FUNCTION__, Value));
        }
#endif /* MT7615 */
     }

     return ret;
}
#endif /* MT_MAC */


/***************************************************************************
 *
 *	PCIe device initialization related procedures.
 *
 ***************************************************************************/
VOID RTMPInitPCIeDevice(RT_CMD_PCIE_INIT *pConfig, VOID *pAdSrc)
{
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
    VOID *pci_dev = pConfig->pPciDev;
    USHORT  device_id = 0;
    POS_COOKIE pObj;
    NDIS_STATUS ret;

    pObj = (POS_COOKIE) pAd->OS_Cookie;
    pci_read_config_word(pci_dev, pConfig->ConfigDeviceID, &device_id);
#ifndef RT_BIG_ENDIAN
    device_id = le2cpu16(device_id);
#endif /* RT_BIG_ENDIAN */
    pObj->DeviceID = device_id;

    MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s():device_id=0x%x\n", 
                        __FUNCTION__, device_id));

    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);

#if defined(RTMP_MAC) || defined(RLT_MAC)
    rtmp_rlt_pci_chip_cfg(pAd, pConfig, device_id);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
    ret = mt_pci_chip_cfg(pAd, device_id);
    /* check pci configuration CR can be read successfully */
    if (ret != NDIS_STATUS_SUCCESS) {
	pConfig->pci_init_succeed = FALSE;
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s():pci configuration space can't be read\n", 
                        __FUNCTION__));
	return;
    } else {
	pConfig->pci_init_succeed = TRUE;
    }
#endif /* */

    if (pAd->infType != RTMP_DEV_INF_UNKNOWN)
        RtmpRaDevCtrlInit(pAd, pAd->infType);
}



#endif /* RTMP_MAC_PCI */


struct device* rtmp_get_dev(void *ad)
{
	struct device *dev = NULL;

#ifdef RTMP_PCI_SUPPORT
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)ad;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	struct pci_dev *inf_dev = obj->pci_dev;
	dev = (struct device *)(&(((struct pci_dev *)(inf_dev))->dev));
#endif

	return dev;
}

