#ifdef MTK_LICENSE
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
	rt_sdio.c
*/
#endif /* MTK_LICENSE */
#include "rt_config.h"

VOID
RtmpInitSDIODevice(
        IN VOID *prAdSrc
)
{
    PRTMP_ADAPTER prAd = (PRTMP_ADAPTER) prAdSrc;
    UINT32 u4value = 0;

    ASSERT(prAd);

    prAd->infType = RTMP_DEV_INF_SDIO;

    prAd->BlockSize = 512;   //shoud read block size from sdio cccr info

    RTMP_SDIO_READ32(prAd, WCIR, &u4value);
    MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("%s(): WCIR1:%x\n",__FUNCTION__,u4value));

    prAd->ChipID = GET_CHIP_ID(u4value);

    if(IS_MT76x6(prAd) || IS_MT7637(prAd)){
	MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("%s():chip is MT7636\n",__FUNCTION__));
	//return FALSE;
	}else{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("%s():chip is not MT7636\n",__FUNCTION__));
	}

    RtmpRaDevCtrlInit(prAd, prAd->infType);
   
   return;
}

NDIS_STATUS
RtmpMgmtTaskInit(
        IN PRTMP_ADAPTER prAd
)
{
	RTMP_OS_TASK *prTask = NULL;
	NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;

	/*
		Creat TimerQ Thread, We need init timerQ related structure before create the timer thread.
	*/
	RtmpTimerQInit(prAd);

	prTask = &prAd->timerTask;
	RTMP_OS_TASK_INIT(prTask, "RtmpTimerTask", prAd);
	rStatus = RtmpOSTaskAttach(prTask, RtmpTimerQThread, (ULONG) prTask);

	if (rStatus == NDIS_STATUS_FAILURE) 
	{
		printk (KERN_WARNING "%s: unable to start RtmpTimerQThread\n", RTMP_OS_NETDEV_GET_DEVNAME(prAd->net_dev));
		return NDIS_STATUS_FAILURE;
}

	/* Creat Command Thread */
	prTask = &prAd->cmdQTask;
	//dc_log_printf("CmdTask=0x%x", (ULONG)prTask, &prAd->RscTaskMemList);
	RTMP_OS_TASK_INIT(prTask, "RtCmdQTask", prAd);

	rStatus = RtmpOSTaskAttach(prTask, MTSDIOCmdThread, (ULONG)prTask);
	if (rStatus == NDIS_STATUS_FAILURE) 
{
		MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("%s: unable to start MTSDIOCmdThread\n", RTMP_OS_NETDEV_GET_DEVNAME(prAd->net_dev)));
		return NDIS_STATUS_FAILURE;
	}

#ifdef WSC_INCLUDED
		/* start the crediential write task first. */
		WscThreadInit(prAd);
#endif /* WSC_INCLUDED */

	return NDIS_STATUS_SUCCESS;
}


VOID
RtmpMgmtTaskExit(
        IN PRTMP_ADAPTER prAd
)
	{
    INT 		u4ret = 0;
    RTMP_OS_TASK	*prTask;
    /* Sleep 50 milliseconds so pending io might finish normally */
    RtmpusecDelay(50000);
    /* We want to wait until all pending receives and sends to the */
    /* device object. We cancel any */
    /* irps. Wait until sends and receives have stopped. */

    /* We need clear timerQ related structure before exits of the timer thread. */
    RtmpTimerQExit(prAd);

    /* Terminate cmdQ thread */
    prTask = &prAd->cmdQTask;
    RTMP_OS_TASK_LEGALITY(prTask)
    {
        NdisAcquireSpinLock(&prAd->CmdQLock);
        prAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
        NdisReleaseSpinLock(&prAd->CmdQLock);
			
        u4ret = RtmpOSTaskKill(prTask);
        if (u4ret == NDIS_STATUS_FAILURE)
        {
            /*			MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("%s: kill task(%s) failed!\n", */
            /*					RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev), pTask->taskName)); */
            MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("kill command task failed!\n"));
	}
        prAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
}

    /* Terminate timer thread */
    prTask = &prAd->timerTask;
    u4ret = RtmpOSTaskKill(prTask);
    if (u4ret == NDIS_STATUS_FAILURE)
{
        /*		MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("%s: kill task(%s) failed!\n", */
        /*					RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev), pTask->taskName)); */
        MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_ERROR, ("kill timer task failed!\n"));
    }

#ifdef WSC_INCLUDED
    WscThreadExit(prAd);
#endif /* WSC_INCLUDED */
}


NDIS_STATUS
RtmpNetTaskInit(
        IN PRTMP_ADAPTER prAd
)
	{
    RTMP_OS_TASK *prTask = NULL;
#if CFG_SDIO_RX_THREAD
    RTMP_OS_TASK *prTaskRx = NULL;
#endif
    NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;

    ASSERT(prAd);
	
    prTask = &prAd->rSDIOTask;
	
    prAd->u4TaskEvent = 0;
    RTMP_OS_TASK_INIT(prTask, "MTSDIOThread", prAd);

    rStatus = RtmpOSTaskAttach(prTask, MTSDIOWorkerThread,(ULONG) prTask);
	
    if (rStatus == NDIS_STATUS_FAILURE) {
        MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_OFF, ("%s: create mtk sdio thread failed\n", __FUNCTION__));
        rStatus = NDIS_STATUS_FAILURE;
        goto err;
    }
#if CFG_SDIO_RX_THREAD
	prTaskRx = &prAd->rSDIORxTask;
	
	prAd->u4TaskRxEvent = 0;
	RTMP_OS_TASK_INIT(prTaskRx, "MTSDIORxThread", prAd);
	
	rStatus = RtmpOSTaskAttach(prTaskRx, MTSDIORxThread,(ULONG) prTaskRx);
	
    if (rStatus == NDIS_STATUS_FAILURE) {
        MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_OFF, ("%s: create mtk sdio rx thread failed\n", __FUNCTION__));
        rStatus = NDIS_STATUS_FAILURE;
        goto err;
    }
	/*initial lock for Rx thread*/
	NdisAllocateSpinLock(prAd, &prAd->RxThreadLock);

#endif
    return rStatus;
err:
    return rStatus;
}


VOID RtmpNetTaskExit(
        IN PRTMP_ADAPTER prAd
)
{
    RTMP_OS_TASK *prTask = NULL;
	
    ASSERT(prAd);

    prTask = &prAd->rSDIOTask;
    RtmpOSTaskKill(prTask);
#if CFG_SDIO_RX_THREAD
    prTask = &prAd->rSDIORxTask;
    RtmpOSTaskKill(prTask);
    NdisFreeSpinLock(&prAd->RxThreadLock);
#endif

   
}


VOID rt_sdio_interrupt(struct sdio_func *func)
{
	VOID *prAd = NULL;
	struct net_device *net_dev = sdio_get_drvdata(func);
   
	GET_PAD_FROM_NET_DEV(prAd, net_dev);
	MTWF_LOG(DBG_CAT_HIF, CATHIF_SDIO, DBG_LVL_TRACE, ("%s()!!!!\n", __FUNCTION__));
	
	MTSDIODataIsr((PRTMP_ADAPTER) prAd);
}

struct device* rtmp_get_dev(void *ad)
{
	struct device *dev;	
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER*)ad;	
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	struct sdio_func *inf_dev = obj->sdio_dev;
	dev = (struct device *)(&(((struct sdio_func *)(inf_dev))->dev));
	return dev;
}

