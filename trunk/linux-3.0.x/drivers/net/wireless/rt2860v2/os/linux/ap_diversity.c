/***************************************************************************
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
	ap_diversity.c

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
	Ying Yuan Huang	06-10-2008		Init for RT3052.
*/



#include "rt_config.h"

#include <asm/rt2880/rt_mmap.h>

/* Ralink GPIO definations */
#define outw(address, value)    *((volatile uint32_t *)(address)) = cpu_to_le32(value)
#define inw(address)            le32_to_cpu(*(volatile u32 *)(address))
#define RALINK_PRGIO_ADDR       RALINK_PIO_BASE			// Programmable I/O
#define RALINK_GPIO(x)          (1 << x)
#define RALINK_SYSCTL_ADDR      RALINK_SYSCTL_BASE  // system control
#define RALINK_REG_GPIOMODE     (RALINK_SYSCTL_ADDR + 0x60)
#define RALINK_REG_PIODATA      (RALINK_PRGIO_ADDR + 0x20)
#define RALINK_REG_PIODIR       (RALINK_PRGIO_ADDR + 0x24)

/* Antenna Diversity export varaibles */
atomic_t	AD_RUN			=  ATOMIC_INIT(0);	// 0: disable, 1:enable
IMPLEMENT_PROC_ENTRY(AD_RUN, 0, 1)

atomic_t	AD_ACTIVE_INTERVAL	=  ATOMIC_INIT(2000);	// msecs
IMPLEMENT_PROC_ENTRY(AD_ACTIVE_INTERVAL, 1024, 10240)

atomic_t	AD_CHOSEN_ANTENNA	=  ATOMIC_INIT(0);	// Ant0 or Ant2
IMPLEMENT_PROC_ENTRY(AD_CHOSEN_ANTENNA, 0 ,2)

atomic_t	AD_EV_RSSI_DIFF		=  ATOMIC_INIT(5);	/* db */
IMPLEMENT_PROC_ENTRY(AD_EV_RSSI_DIFF, 0, 256)

atomic_t	AD_SW_RSSI_DIFF	=  ATOMIC_INIT(3);	/* db */
IMPLEMENT_PROC_ENTRY(AD_SW_RSSI_DIFF, 0, 256)

atomic_t	AD_FORCE_RESCAN_ROUND	=  ATOMIC_INIT(5);	/* round */
IMPLEMENT_PROC_ENTRY(AD_FORCE_RESCAN_ROUND, 0, 256)

atomic_t	AD_FORCE_ANTENNA	=  ATOMIC_INIT(-1);	/* -1, 0, 1, 2, write only*/
IMPLEMENT_PROC_ENTRY(AD_FORCE_ANTENNA, -1, 2)

atomic_t	AD_ALGORITHM		=  ATOMIC_INIT(1);	/* algorithm 1 or 2 */
IMPLEMENT_PROC_ENTRY(AD_ALGORITHM, 1, 2)

atomic_t	BBP_RSSI_RANGE_MAX	=  ATOMIC_INIT(-15);	/* db */
IMPLEMENT_PROC_ENTRY(BBP_RSSI_RANGE_MAX, -255, 0)

atomic_t	BBP_RSSI_RANGE_MIN	=  ATOMIC_INIT(-86);	/* db */
IMPLEMENT_PROC_ENTRY(BBP_RSSI_RANGE_MIN, -255, 0)

atomic_t	BBP_RSSI_SAMPLE_COUNT	=  ATOMIC_INIT(20);	/* times */
IMPLEMENT_PROC_ENTRY(BBP_RSSI_SAMPLE_COUNT, 1, 256)

atomic_t	BBP_RSSI_SAMPLE_INTERVAL=  ATOMIC_INIT(10);	/* ms */
IMPLEMENT_PROC_ENTRY(BBP_RSSI_SAMPLE_INTERVAL, 10, 1024)

atomic_t	RXWI_RSSI_RANGE_MAX	=  ATOMIC_INIT(-14);	/* db */
IMPLEMENT_PROC_ENTRY(RXWI_RSSI_RANGE_MAX, -255, 0)

atomic_t	RXWI_RSSI_RANGE_MIN	=  ATOMIC_INIT(-80);	/* db */
IMPLEMENT_PROC_ENTRY(RXWI_RSSI_RANGE_MIN, -255, 0)

atomic_t	DEBUG_VERBOSE_MODE	=  ATOMIC_INIT(0);	/* boolean */
IMPLEMENT_PROC_ENTRY(DEBUG_VERBOSE_MODE, 0 ,1)

//static PMAC_TABLE_ENTRY		 pAckedEntry	= NULL;

static struct proc_dir_entry	*pProcDir	= NULL;

/*
 * Read RSSI from BBP R50/R51/R52
 */
INT AntDiversity_BBPRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	INT		Ant)
{
	UCHAR	BBPReg;
	CHAR	rssi_once = 0;
	INT32	RSSI;
	INT	rssi_total = 0, rssi_avg = 0, rssi_count = 0, i;

	switch(Ant){
		case 0:
			BBPReg = 50;
			break;
		case 1:
			BBPReg = 51;
			break;
		case 2:
			BBPReg = 52;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR,("Don't support more than 3 antennas yet"));
			BUG();
			return 0;
	}

	for(i=0; i< atomic_read(&BBP_RSSI_SAMPLE_COUNT); i++){
		RTMP_BBP_IO_READ8_BY_REG_ID_SHORT_DELAY(pAd, BBPReg, &rssi_once);
		RSSI = ConvertToRssi(pAd, rssi_once, Ant);

		// Max-Min filter
		if(RSSI < atomic_read(&BBP_RSSI_RANGE_MAX) && RSSI > atomic_read(&BBP_RSSI_RANGE_MIN)){
			rssi_count ++;
			rssi_total += RSSI;
			ADDBGPRINT("%02d  ", RSSI);
		}else
			ADDBGPRINT("%02d* ", RSSI);

		if(atomic_read(&BBP_RSSI_SAMPLE_INTERVAL)){
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(msecs_to_jiffies(atomic_read(&BBP_RSSI_SAMPLE_INTERVAL)));	// sleep
		}
	}

	rssi_avg = rssi_count ? rssi_total / rssi_count : 0;

	ADDBGPRINT("Ant%d==>   %02d\n", Ant, rssi_avg);
	return rssi_avg;
}

/*
 * for debug.
 */
VOID AntDiversity_BBPReadAll(
	IN	PRTMP_ADAPTER	pAd)
{
	printk("=====================\n");
	AntDiversity_BBPRead(pAd, 0);
	AntDiversity_BBPRead(pAd, 1);
	printk("=====================\n");
}


/*
 *  Until now we disable the Algorithm 2(RXWI) implementation.
 */
#ifdef ALGORITHM_2

/*
 *  Update_Rssi_Sample() in ap/ap_data.c.
 */
VOID AntDiversity_Update_Rssi_Sample(
	IN PRTMP_ADAPTER	pAd,
	IN RSSI_SAMPLE		*pRssi,
	IN PRXWI_STRUC		pRxWI)
{
	CHAR	rssi0 = pRxWI->RSSI0;
	CHAR	rssi1 = pRxWI->RSSI1;
	CHAR	rssi2 = pRxWI->RSSI2;

	if (rssi0 != 0)	{
		pRssi->LastRssi0	= ConvertToRssi(pAd, (CHAR)rssi0, RSSI_0);
		pRssi->LastRssi0Jiffy	= jiffies;
		pRssi->AvgRssi0X8	= (pRssi->AvgRssi0X8 - pRssi->AvgRssi0) + pRssi->LastRssi0;
		pRssi->AvgRssi0		= pRssi->AvgRssi0X8 >> 3;
	}

	if (rssi1 != 0){   
		pRssi->LastRssi1	= ConvertToRssi(pAd, (CHAR)rssi1, RSSI_1);
		pRssi->LastRssi1Jiffy	= jiffies;
		pRssi->AvgRssi1X8	= (pRssi->AvgRssi1X8 - pRssi->AvgRssi1) + pRssi->LastRssi1;
		pRssi->AvgRssi1		= pRssi->AvgRssi1X8 >> 3;
	}

	if (rssi2 != 0){
		pRssi->LastRssi2	= ConvertToRssi(pAd, (CHAR)rssi2, RSSI_2);
		pRssi->LastRssi2Jiffy	= jiffies;
		pRssi->AvgRssi2X8	= (pRssi->AvgRssi2X8 - pRssi->AvgRssi2) + pRssi->LastRssi2;
		pRssi->AvgRssi2		= pRssi->AvgRssi2X8 >> 3;
	}
}


#define PRE_WINDOW  (HZ * 1)

static VOID EnableACKFrame(IN PRTMP_ADAPTER pAd, int enable)
{
	UINT32 rx_filter_flag = APNORMAL;
	if(enable){
		rx_filter_flag &= (~0x4400);
	}else{
		rx_filter_flag |= 0x4400;
	}
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, rx_filter_flag);
}

VOID AntDivHandleACK(
	IN PRTMP_ADAPTER	pAd,
	PRXWI_STRUC		pRxWI,
	PHEADER_802_11		pHeader)
{
	PMAC_TABLE_ENTRY	pEntry;

	pEntry = MacTableLookup(pAd, pHeader->Addr1);

	if(!pEntry){
		ADDBGPRINT("Can't find sta: - %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pHeader->Addr1));
		return;
	}
	ADDBGPRINT("Update RSSI via ACK frame\n");
	AntDiversity_Update_Rssi_Sample(pAd, &pEntry->RssiSample, pRxWI);
}


INT AntDiversity_RXWIRead(
	IN PRTMP_ADAPTER	pAd,
	IN INT			Ant,
	IN INT			pre_delay)
{
	INT		i, sta_count = 0, total_rssi = 0;
	ULONG	now;
	ULONG	LastRSSIJiffy;
	ULONG	LastRSSI;

	if(pre_delay){
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(PRE_WINDOW/2);	// sleep
	}

	now = jiffies;
	for (i=0; i< MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry)) && (pEntry->Sst == SST_ASSOC))
		{

			switch(Ant){
				case 0:
					LastRSSIJiffy = pEntry->RssiSample.LastRssi0Jiffy;
					LastRSSI	  = pEntry->RssiSample.LastRssi0;
					break;
				case 1:
					LastRSSIJiffy = pEntry->RssiSample.LastRssi1Jiffy;
					LastRSSI	  = pEntry->RssiSample.LastRssi1;
					break;
				case 2:
					LastRSSIJiffy = pEntry->RssiSample.LastRssi2Jiffy;
					LastRSSI	  = pEntry->RssiSample.LastRssi2;
					break;
				default:
					return 0;
			}

			ADDBGPRINT("%02X:%02X:%02X:%02X:%02X:%02X = ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);

			/* 
			 *
			 * 1) LastRSSIJiffy >= now
			 *     The STA last-rssi field is updated after we got the current time.
			 *	   Recognize it always.
			 *
			 * 2) LastRSSIJiffy < now
			 *     The STA last-rssi field is updated before we got the current time.
			 *	   Check the time gap between LastRSSIJiffy and now.
			 */
			if( (LastRSSIJiffy >= now) ||
				((now - LastRSSIJiffy) < PRE_WINDOW) )

				if(  LastRSSI > atomic_read(&RXWI_RSSI_RANGE_MAX)  ||
				     LastRSSI < atomic_read(&RXWI_RSSI_RANGE_MIN)  ){
					ADDBGPRINT("%02d(f) ", (INT)LastRSSI);
				}else{
				ADDBGPRINT("%02d(   %d,%d) ", (INT)LastRSSI, now, LastRSSIJiffy);
					/* the packet we are interesting in. */
					total_rssi += LastRSSI;
					sta_count++;
				}
			else{
				ADDBGPRINT("%02d(e, %d,%d) ", (INT)LastRSSI, now, LastRSSIJiffy);

			}
		} /* if */
	}/* for each STA */

	return sta_count ? (total_rssi / sta_count) : 0;
}
#endif /* ALGORITHM_2 */




/*
 *  Antenna Switch GPIO mapping.
 *
 *	GPIO3    GPIO5
 *        1        0   -> ANT2
 *        0        1   -> ANT0
 * 
 */
#define GPIO_REVERSE			1
#define ANT0_GPIO			3
#define ANT2_GPIO			5
#define GPIO_REG_BIT		1
static VOID AntSwitch(INT ant)
{
	UINT32 data;

	/* Set SPI to GPIO mode */
	data = inw(RALINK_REG_GPIOMODE);
	data |= RALINK_GPIO(GPIO_REG_BIT);
	outw(RALINK_REG_GPIOMODE, data);

	/* set direction*/
	data = inw(RALINK_REG_PIODIR);
	data |= RALINK_GPIO(ANT0_GPIO);
	data |= RALINK_GPIO(ANT2_GPIO);
	outw(RALINK_REG_PIODIR, data);

	/* write Data*/
	data = inw(RALINK_REG_PIODATA);
#ifdef GPIO_REVERSE
	data |= (UINT32)1 << ANT0_GPIO;
	data |=	(UINT32)1 << ANT2_GPIO;
#else
	data &= ~((UINT32)1 << ANT0_GPIO);
	data &=	~((UINT32)1 << ANT2_GPIO);
#endif

	if(ant == 0){
#ifdef GPIO_REVERSE
		data &= ~((UINT32)1 << ANT0_GPIO);
#else
		data |= ((UINT32)1 << ANT0_GPIO);
#endif
	}else if(ant == 2){
#ifdef GPIO_REVERSE
		data &= ~((UINT32)1 << ANT2_GPIO);
#else
		data |= ((UINT32)1 << ANT2_GPIO);
#endif
	}else{
		printk(KERN_ERR "not support Ant%d\n.", ant);
		BUG();
	}
	outw(RALINK_REG_PIODATA, data);

	ADDBGPRINT("Ant switch to %d\n", ant);
	return;
}

/*
 *	Main kernel thread.
 */
static INT work_handler(IN VOID *Context)
{
	PRTMP_ADAPTER	pAd;
    RTMP_OS_TASK	*pTask;
    //int				Status = 0;
	INT				AntArrayRSSI[]	= {0, 0, 0};		// 3 candidates
	const INT		AntFixedIdx		= 1;			// Ant1 is unchanged;
	INT				UpdateRSSI		= 0;
	BOOLEAN			EVChanged		= FALSE;
	INT				EVNotChangeCount= 0;

//	rtmp_os_thread_init("rtmpAntDiversity", (PVOID)&(pAd->ad_notify));

    pTask = (RTMP_OS_TASK *)Context;
    pAd = (PRTMP_ADAPTER)pTask->priv;

    RtmpOSTaskCustomize(pTask);

	while (!pTask->task_killed){
		INT ad_current;
		INT ad_force;
		INT ad_algorithm;

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(atomic_read(&AD_ACTIVE_INTERVAL)));	// sleep

#ifdef KTHREAD_SUPPORT
        if (pTask->task_killed == 1)
            break;
#endif

		/* Force switch antenna if user command */
		ad_force = atomic_read(&AD_FORCE_ANTENNA);
		if( ad_force != -1){
			atomic_set(&AD_FORCE_ANTENNA, (INT)-1);
			AntSwitch(ad_force);
			atomic_set(&AD_CHOSEN_ANTENNA, ad_force);
		}

		/* Check if running */
		if(!atomic_read(&AD_RUN))
			continue;

		ad_current = atomic_read(&AD_CHOSEN_ANTENNA);
		if(ad_current < 0 || ad_current >= sizeof(AntArrayRSSI)/sizeof(INT)){
			printk(KERN_ERR "Wrong AD_CHOSEN_ANTENNA: %d\n", ad_current);
			continue;
		}

		ad_algorithm = atomic_read(&AD_ALGORITHM);

		/* Force to rescan ? */
		if(atomic_read(&AD_FORCE_RESCAN_ROUND) && (EVNotChangeCount >= atomic_read(&AD_FORCE_RESCAN_ROUND)))
			EVChanged = TRUE;
		else
			EVNotChangeCount++;

		if(ad_algorithm == 1) 		/* Read the BBP RSSI registers. */
			UpdateRSSI = AntDiversity_BBPRead(pAd, 0);
#ifdef ALGORITHM_2
		else if(ad_algorithm == 2)	/* Read the RXWI RSSI field */
			UpdateRSSI = AntDiversity_RXWIRead(pAd, 0, 0);
#endif
		else
			UpdateRSSI = 0;

		ADDBGPRINT("AD_CHOSEN_ANTENNA:[%d,%d] <=> %d\n", ad_current, AntArrayRSSI[ad_current], UpdateRSSI);

		if(UpdateRSSI && (abs( UpdateRSSI - AntArrayRSSI[ ad_current ] ) > atomic_read(&AD_EV_RSSI_DIFF)) )
			EVChanged = TRUE;

		// TODO: use unlikely().
		if(EVChanged){
			INT i, lastSwitch = -1, theBest = -1, max = -4096;

			ADDBGPRINT("############################################\n");
			ADDBGPRINT("%s", ( atomic_read(&AD_FORCE_RESCAN_ROUND) && (EVNotChangeCount >= atomic_read(&AD_FORCE_RESCAN_ROUND)) )  ? "  Rescan happened\n" : "");

			EVChanged = FALSE;
			EVNotChangeCount = 0;
			AntArrayRSSI[ ad_current ] = UpdateRSSI;

			// Collect RSSI on each antenna.
			for(i=0; i<sizeof(AntArrayRSSI)/sizeof(INT); i++){
				if(i == ad_current || i == AntFixedIdx)
					continue;
				lastSwitch = i;
				AntSwitch(i);

				if(ad_algorithm == 1)           /* Read the BBP RSSI registers. */
					AntArrayRSSI[i] = AntDiversity_BBPRead(pAd, 0);
#ifdef ALGORITHM_2
				else if(ad_algorithm == 2){      /* Read the RXWI RSSI field */
					AntArrayRSSI[i] = AntDiversity_RXWIRead(pAd, 0, 1);
				}
#endif
			}

			if(lastSwitch == -1){
				printk("No candicate antenna is switched to.\n");
				BUG();
			}

			// Elect the best one.
			for(i=0; i<sizeof(AntArrayRSSI)/sizeof(INT); i++){
				if(i == AntFixedIdx)
					continue;
				if(AntArrayRSSI[i]){
					int rssi = AntArrayRSSI[i];
					if(i == lastSwitch)
						rssi = rssi + atomic_read(&AD_SW_RSSI_DIFF);	// level up the weight of the last switched Antenna.

					if(rssi > max){
						max = rssi;
						theBest = i;
					}
				}
			}
			ADDBGPRINT(" %d, %d, threshold %d, The Best is >>>  %d <<<\n", AntArrayRSSI[0], AntArrayRSSI[2], atomic_read(&AD_SW_RSSI_DIFF), theBest);
			ADDBGPRINT("####################################################\n ");

			// Champion
			if(theBest != -1){	//
				if(theBest != ad_current)
					atomic_set(&AD_CHOSEN_ANTENNA, (int) theBest);
				if(theBest != lastSwitch)
					AntSwitch(theBest);
			}
		}
	}

	RtmpOSTaskNotifyToExit(pTask);

	return 0;
}

static VOID ProcFS_Init(VOID)
{
	pProcDir = proc_mkdir(PROC_DIR, NULL);

	CREATE_PROC_ENTRY(AD_ACTIVE_INTERVAL);
	CREATE_PROC_ENTRY(AD_RUN);
	CREATE_PROC_ENTRY(AD_EV_RSSI_DIFF);
	CREATE_PROC_ENTRY(AD_SW_RSSI_DIFF);
	CREATE_PROC_ENTRY(AD_CHOSEN_ANTENNA);
	CREATE_PROC_ENTRY(AD_FORCE_ANTENNA);
	CREATE_PROC_ENTRY(AD_FORCE_RESCAN_ROUND);
	CREATE_PROC_ENTRY(AD_ALGORITHM);
	CREATE_PROC_ENTRY(BBP_RSSI_RANGE_MAX);
	CREATE_PROC_ENTRY(BBP_RSSI_RANGE_MIN);
	CREATE_PROC_ENTRY(BBP_RSSI_SAMPLE_COUNT);
	CREATE_PROC_ENTRY(BBP_RSSI_SAMPLE_INTERVAL);
	CREATE_PROC_ENTRY(RXWI_RSSI_RANGE_MAX);
	CREATE_PROC_ENTRY(RXWI_RSSI_RANGE_MIN);
	CREATE_PROC_ENTRY(DEBUG_VERBOSE_MODE);
	return;
}

static VOID ProcFS_Fini(VOID)
{
	DESTORY_PROC_ENTRY(AD_ACTIVE_INTERVAL);
	DESTORY_PROC_ENTRY(AD_CHOSEN_ANTENNA);
	DESTORY_PROC_ENTRY(AD_RUN);
	DESTORY_PROC_ENTRY(AD_EV_RSSI_DIFF);
	DESTORY_PROC_ENTRY(AD_SW_RSSI_DIFF);
	DESTORY_PROC_ENTRY(AD_FORCE_ANTENNA);
	DESTORY_PROC_ENTRY(AD_FORCE_RESCAN_ROUND);
	DESTORY_PROC_ENTRY(AD_ALGORITHM);
	DESTORY_PROC_ENTRY(BBP_RSSI_RANGE_MAX);
	DESTORY_PROC_ENTRY(BBP_RSSI_RANGE_MIN);
	DESTORY_PROC_ENTRY(BBP_RSSI_SAMPLE_COUNT);
	DESTORY_PROC_ENTRY(BBP_RSSI_SAMPLE_INTERVAL);
	DESTORY_PROC_ENTRY(RXWI_RSSI_RANGE_MAX);
	DESTORY_PROC_ENTRY(RXWI_RSSI_RANGE_MIN);
	DESTORY_PROC_ENTRY(DEBUG_VERBOSE_MODE);
	if (pProcDir)
		remove_proc_entry(PROC_DIR, 0);

	return;
}

VOID RT3XXX_AntDiversity_Init(IN PRTMP_ADAPTER pAd)
{
	NDIS_STATUS status = TRUE;
	RTMP_OS_TASK *pTask = &pAd->ADTask;

	RtmpOSTaskInit(pTask, "RtmpADTask", pAd);
	status = RtmpOSTaskAttach(pTask, work_handler, &pAd->ADTask);
	if(status == NDIS_STATUS_SUCCESS)
		ProcFS_Init();
	else
		printk (KERN_ERR "%s() unable to init kernel thread\n", __FUNCTION__);
	return;
}

VOID RT3XXX_AntDiversity_Fini(IN PRTMP_ADAPTER pAd)
{
	ProcFS_Fini();
    RtmpOSTaskKill(&pAd->ADTask);
	return;
}


