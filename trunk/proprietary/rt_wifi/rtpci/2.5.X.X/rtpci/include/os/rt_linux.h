/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_linux.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RT_LINUX_H__
#define __RT_LINUX_H__

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/wireless.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/if_arp.h>
#include <linux/ctype.h>
#include <linux/vmalloc.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>

#ifdef INF_PPA_SUPPORT
#include <net/ifx_ppa_api.h>
#include <net/ifx_ppa_hook.h>
#endif // INF_PPA_SUPPORT //

// load firmware
#define __KERNEL_SYSCALLS__
#include <linux/unistd.h>
#include <asm/uaccess.h>
#include <asm/types.h>
#include <asm/unaligned.h>	// for get_unaligned()

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#define KTHREAD_SUPPORT 1
#endif

#ifdef KTHREAD_SUPPORT
#include <linux/err.h>
#include <linux/kthread.h>
#endif // KTHREAD_SUPPORT //

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/pid.h>
#endif

#ifdef RT_CFG80211_SUPPORT
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
#include <net/mac80211.h>
#ifndef EXT_BUILD_CHANNEL_LIST
#define EXT_BUILD_CHANNEL_LIST	/* must define with CRDA */
#endif
#else // LINUX_VERSION_CODE //
#undef RT_CFG80211_SUPPORT
#endif // LINUX_VERSION_CODE //
#endif // RT_CFG80211_SUPPORT //

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif // WSC_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#if defined(WSC_AP_SUPPORT) || defined(WSC_STA_SUPPORT)
#define WSC_INCLUDED
#endif

#ifdef KTHREAD_SUPPORT
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,4)
#error "This kerne version doesn't support kthread!!"
#endif
#endif // KTHREAD_SUPPORT //


/***********************************************************************************
 *	Profile related sections
 ***********************************************************************************/
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
#if defined (RT_IFNAME_1ST)
 #define AP_RTMP_FIRMWARE_FILE_NAME 			"/etc_ro/Wireless/RT2860AP.bin"
 #define AP_PROFILE_PATH				"/etc/Wireless/RT2860/RT2860AP.dat"
 #define CARD_INFO_PATH					"/etc/Wireless/RT2860/RT2860APCard.dat"
#else
 #define AP_RTMP_FIRMWARE_FILE_NAME 			"/etc_ro/Wireless/iNIC_ap.bin"
 #define AP_PROFILE_PATH				"/etc/Wireless/iNIC/iNIC_ap.dat"
 #define CARD_INFO_PATH					"/etc/Wireless/iNIC/RT2860APCard.dat"
#endif
#define AP_NIC_DEVICE_NAME				"RT309xAP"
#define AP_DRIVER_VERSION				"2.5.0.11"
#endif // RTMP_MAC_PCI //
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_APSTA_MIXED_SUPPORT
extern	const struct iw_handler_def rt28xx_ap_iw_handler_def;
#endif // CONFIG_APSTA_MIXED_SUPPORT //

/***********************************************************************************
 *	Compiler related definitions
 ***********************************************************************************/
#undef __inline
#define __inline		static inline
#define IN
#define OUT
#define INOUT
#define NDIS_STATUS		INT


/***********************************************************************************
 *	OS Specific definitions and data structures
 ***********************************************************************************/
typedef struct pci_dev 		* PPCI_DEV;
typedef struct net_device	* PNET_DEV;
typedef void				* PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET		* PPNDIS_PACKET;
typedef	dma_addr_t			NDIS_PHYSICAL_ADDRESS;
typedef	dma_addr_t			* PNDIS_PHYSICAL_ADDRESS;
typedef void				* NDIS_HANDLE;
typedef char 				* PNDIS_BUFFER;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
typedef	struct pid *	RTMP_OS_PID;
#else
typedef pid_t 				RTMP_OS_PID;
#endif

typedef struct semaphore	RTMP_OS_SEM;

typedef int (*HARD_START_XMIT_FUNC)(struct sk_buff *skb, struct net_device *net_dev);

#ifdef RTMP_MAC_PCI
#ifndef PCI_DEVICE
#define PCI_DEVICE(vend,dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif // PCI_DEVICE //
#endif // RTMP_MAC_PCI //

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define RT_MOD_INC_USE_COUNT() \
	if (!try_module_get(THIS_MODULE)) \
	{ \
		DBGPRINT(RT_DEBUG_ERROR, ("%s: cannot reserve module\n", __FUNCTION__)); \
		return -1; \
	}

#define RT_MOD_DEC_USE_COUNT() module_put(THIS_MODULE);
#else
#define RT_MOD_INC_USE_COUNT()	MOD_INC_USE_COUNT;
#define RT_MOD_DEC_USE_COUNT() MOD_DEC_USE_COUNT;
#endif

#define RTMP_INC_REF(_A)		0
#define RTMP_DEC_REF(_A)		0
#define RTMP_GET_REF(_A)		0


#if WIRELESS_EXT >= 12
// This function will be called when query /proc
struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev);
#endif


/***********************************************************************************
 *	Network related constant definitions
 ***********************************************************************************/
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define ETH_LENGTH_OF_ADDRESS	6

#define NDIS_STATUS_SUCCESS                     0x00
#define NDIS_STATUS_FAILURE                     0x01
#define NDIS_STATUS_INVALID_DATA				0x02
#define NDIS_STATUS_RESOURCES                   0x03

#define NDIS_SET_PACKET_STATUS(_p, _status)			do{} while(0)
#define NdisWriteErrorLogEntry(_a, _b, _c, _d)		do{} while(0)

/* statistics counter */
#define STATS_INC_RX_PACKETS(_pAd, _dev)
#define STATS_INC_TX_PACKETS(_pAd, _dev)

#define STATS_INC_RX_BYTESS(_pAd, _dev, len)
#define STATS_INC_TX_BYTESS(_pAd, _dev, len)

#define STATS_INC_RX_ERRORS(_pAd, _dev)
#define STATS_INC_TX_ERRORS(_pAd, _dev)

#define STATS_INC_RX_DROPPED(_pAd, _dev)
#define STATS_INC_TX_DROPPED(_pAd, _dev)


/***********************************************************************************
 *	Ralink Specific network related constant definitions
 ***********************************************************************************/
#define MIN_NET_DEVICE_FOR_AID			0x00		//0x00~0x3f
#define MIN_NET_DEVICE_FOR_MBSSID		0x00		//0x00,0x10,0x20,0x30
#define MIN_NET_DEVICE_FOR_WDS			0x10		//0x40,0x50,0x60,0x70
#define MIN_NET_DEVICE_FOR_APCLI		0x20
#define MIN_NET_DEVICE_FOR_MESH			0x30
#define NET_DEVICE_REAL_IDX_MASK		0x0f		// for each operation mode, we maximum support 15 entities.




/***********************************************************************************
 *	OS signaling related constant definitions
 ***********************************************************************************/


/***********************************************************************************
 *	OS file operation related data structure definitions
 ***********************************************************************************/
typedef struct file* RTMP_OS_FD;

typedef struct _RTMP_OS_FS_INFO_
{
	uid_t		fsuid;
	gid_t		fsgid;
	mm_segment_t	fs;
}RTMP_OS_FS_INFO;

#define IS_FILE_OPEN_ERR(_fd) 	((_fd == NULL) || IS_ERR((_fd)))


/***********************************************************************************
 *	OS semaphore related data structure and definitions
 ***********************************************************************************/
struct os_lock  {
	spinlock_t		lock;
	unsigned long  	flags;
};

typedef spinlock_t			NDIS_SPIN_LOCK;

//
//  spin_lock enhanced for Nested spin lock
//
#define NdisAllocateSpinLock(__lock)      \
{                                       \
    spin_lock_init((spinlock_t *)(__lock));               \
}

#define NdisFreeSpinLock(lock)          \
	do{}while(0)


#define RTMP_SEM_LOCK(__lock)					\
{												\
	spin_lock_bh((spinlock_t *)(__lock));		\
}

#define RTMP_SEM_UNLOCK(__lock)					\
{												\
	spin_unlock_bh((spinlock_t *)(__lock));		\
}


// sample, use semaphore lock to replace IRQ lock, 2007/11/15
#ifdef MULTI_CORE_SUPPORT

#define RTMP_IRQ_LOCK(__lock, __irqflags)			\
{													\
	__irqflags = 0;									\
	spin_lock_irqsave((spinlock_t *)(__lock), __irqflags);			\
	pAd->irq_disabled |= 1; \
}

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)			\
{													\
	pAd->irq_disabled &= 0; 						\
	spin_unlock_irqrestore((spinlock_t *)(__lock), __irqflag);			\
}
#else
#define RTMP_IRQ_LOCK(__lock, __irqflags)			\
{													\
	__irqflags = 0;									\
	spin_lock_bh((spinlock_t *)(__lock));			\
	pAd->irq_disabled |= 1; \
}

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)			\
{													\
	pAd->irq_disabled &= 0; 						\
	spin_unlock_bh((spinlock_t *)(__lock));			\
}
#endif // MULTI_CORE_SUPPORT //

#define RTMP_INT_LOCK(__lock, __irqflags)			\
{													\
	spin_lock_irqsave((spinlock_t *)__lock, __irqflags);	\
}

#define RTMP_INT_UNLOCK(__lock, __irqflag)			\
{													\
	spin_unlock_irqrestore((spinlock_t *)(__lock), ((unsigned long)__irqflag));	\
}

#define NdisAcquireSpinLock		RTMP_SEM_LOCK
#define NdisReleaseSpinLock		RTMP_SEM_UNLOCK

/*
	Following lock/unlock definition used for BBP/RF register read/write.
	Currently we don't use it to protect MAC register access.

	For USB: 
			we use binary semaphore to do the protection because all register 
			access done in kernel thread and should allow task go sleep when
			in protected status.

	For PCI/PCI-E/RBUS:
			We use interrupt to do the protection because the register may accessed
			in thread/tasklet/timer/inteerupt, so we use interrupt_disable to protect
			the access.
*/	
#define RTMP_MCU_RW_LOCK(_pAd, _irqflags)	\
	do{								\
		if (_pAd->infType == RTMP_DEV_INF_USB)	\
		{\
			RTMP_SEM_EVENT_WAIT(&_pAd->McuCmdSem, _irqflags);\
		}\
		else\
		{\
			RTMP_SEM_LOCK(&_pAd->McuCmdLock, _irqflags);\
		}\
	}while(0)

#define RTMP_MCU_RW_UNLOCK(_pAd, _irqflags)	\
	do{				\
		if(_pAd->infType == RTMP_DEV_INF_USB)\
		{	\
			RTMP_SEM_EVENT_UP(&_pAd->McuCmdSem);\
		}		\
		else\
		{\
			RTMP_SEM_UNLOCK(&_pAd->McuCmdLock, _irqflags);\
		}\
	}while(0)

	
#ifndef wait_event_interruptible_timeout
#define __wait_event_interruptible_timeout(wq, condition, ret) \
do { \
        wait_queue_t __wait; \
        init_waitqueue_entry(&__wait, current); \
        add_wait_queue(&wq, &__wait); \
        for (;;) { \
                set_current_state(TASK_INTERRUPTIBLE); \
                if (condition) \
                        break; \
                if (!signal_pending(current)) { \
                        ret = schedule_timeout(ret); \
                        if (!ret) \
                                break; \
                        continue; \
                } \
                ret = -ERESTARTSYS; \
                break; \
        } \
        current->state = TASK_RUNNING; \
        remove_wait_queue(&wq, &__wait); \
} while (0)

#define wait_event_interruptible_timeout(wq, condition, timeout) \
({ \
        long __ret = timeout; \
        if (!(condition)) \
                __wait_event_interruptible_timeout(wq, condition, __ret); \
        __ret; \
})
#endif

#define RTMP_SEM_EVENT_INIT_LOCKED(_pSema) 	sema_init((_pSema), 0)
#define RTMP_SEM_EVENT_INIT(_pSema)			sema_init((_pSema), 1)
#define RTMP_SEM_EVENT_DESTORY(_pSema)		do{}while(0)
#define RTMP_SEM_EVENT_WAIT(_pSema, _status)	((_status) = down_interruptible((_pSema)))
#define RTMP_SEM_EVENT_UP(_pSema)			up(_pSema)

#ifdef KTHREAD_SUPPORT
#define RTMP_WAIT_EVENT_INTERRUPTIBLE(_pAd, _pTask) \
{ \
		wait_event_interruptible(_pTask->kthread_q, \
								 _pTask->kthread_running || kthread_should_stop()); \
		_pTask->kthread_running = FALSE; \
		if (kthread_should_stop()) \
		{ \
			RTMP_SET_FLAG(_pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); \
			break; \
		} \
}
#endif

#ifdef KTHREAD_SUPPORT
#define WAKE_UP(_pTask) \
	do{ \
		if ((_pTask)->kthread_task) \
        { \
			(_pTask)->kthread_running = TRUE; \
	        wake_up(&(_pTask)->kthread_q); \
		} \
	}while(0)
#endif

/***********************************************************************************
 *	OS Memory Access related data structure and definitions
 ***********************************************************************************/
#define MEM_ALLOC_FLAG      (GFP_ATOMIC) //(GFP_DMA | GFP_ATOMIC)

#define NdisMoveMemory(Destination, Source, Length) memmove(Destination, Source, Length)
#define NdisCopyMemory(Destination, Source, Length) memcpy(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)         memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)   memset(Destination, Fill, Length)
#define NdisCmpMemory(Destination, Source, Length)  memcmp(Destination, Source, Length)
#define NdisEqualMemory(Source1, Source2, Length)   (!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	(!memcmp(Source1, Source2, Length))

#define MlmeAllocateMemory(_pAd, _ppVA)		os_alloc_mem(_pAd, _ppVA, MGMT_DMA_BUFFER_SIZE)
#define MlmeFreeMemory(_pAd, _pVA)			os_free_mem(_pAd, _pVA)

#define COPY_MAC_ADDR(Addr1, Addr2)             memcpy((Addr1), (Addr2), MAC_ADDR_LEN)


/***********************************************************************************
 *	OS task related data structure and definitions
 ***********************************************************************************/
#define RTMP_OS_MGMT_TASK_FLAGS	CLONE_VM

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
typedef	struct pid *	THREAD_PID;
#define	THREAD_PID_INIT_VALUE	NULL
// TODO: Use this IOCTL carefully when linux kernel version larger than 2.6.27, because the PID only correct when the user space task do this ioctl itself.
//#define RTMP_GET_OS_PID(_x, _y)    _x = get_task_pid(current, PIDTYPE_PID);
#define RTMP_GET_OS_PID(_x, _y)		do{rcu_read_lock(); _x=current->pids[PIDTYPE_PID].pid; rcu_read_unlock();}while(0)
#define	GET_PID_NUMBER(_v)	pid_nr((_v))
#define CHECK_PID_LEGALITY(_pid)	if (pid_nr((_pid)) > 0)
#define KILL_THREAD_PID(_A, _B, _C)	kill_pid((_A), (_B), (_C))
#else
typedef	pid_t	THREAD_PID;
#define	THREAD_PID_INIT_VALUE	-1
#define RTMP_GET_OS_PID(_x, _pid)		_x = _pid
#define	GET_PID_NUMBER(_v)	(_v)
#define CHECK_PID_LEGALITY(_pid)	if ((_pid) >= 0)
#define KILL_THREAD_PID(_A, _B, _C)	kill_proc((_A), (_B), (_C))
#endif

typedef INT (*RTMP_OS_TASK_CALLBACK)(VOID*);
typedef struct tasklet_struct  RTMP_NET_TASK_STRUCT;
typedef struct tasklet_struct  *PRTMP_NET_TASK_STRUCT;


/***********************************************************************************
 * Timer related definitions and data structures.
 **********************************************************************************/
#define OS_HZ			HZ

typedef struct timer_list	NDIS_MINIPORT_TIMER;
typedef struct timer_list	RTMP_OS_TIMER;
typedef void (*TIMER_FUNCTION)(unsigned long);


#define OS_WAIT(_time) \
{	\
	if (in_interrupt()) \
	{\
		RTMPusecDelay(_time * 1000);\
	}else	\
	{\
		int _i; \
		long _loop = ((_time)/(1000/OS_HZ)) > 0 ? ((_time)/(1000/OS_HZ)) : 1;\
		wait_queue_head_t _wait; \
		init_waitqueue_head(&_wait); \
		for (_i=0; _i<(_loop); _i++) \
			wait_event_interruptible_timeout(_wait, 0, ONE_TICK); \
	}\
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define RTMP_TIME_AFTER(a,b)		\
	(typecheck(unsigned long, (unsigned long)a) && \
	 typecheck(unsigned long, (unsigned long)b) && \
	 ((long)(b) - (long)(a) < 0))

#define RTMP_TIME_AFTER_EQ(a,b)	\
	(typecheck(unsigned long, (unsigned long)a) && \
	 typecheck(unsigned long, (unsigned long)b) && \
	 ((long)(a) - (long)(b) >= 0))
#define RTMP_TIME_BEFORE(a,b)	RTMP_TIME_AFTER_EQ(b,a)
#else
#define typecheck(type,x) \
({      type __dummy; \
        typeof(x) __dummy2; \
        (void)(&__dummy == &__dummy2); \
        1; \
})
#define RTMP_TIME_AFTER_EQ(a,b)	\
	(typecheck(unsigned long, (unsigned long)a) && \
	 typecheck(unsigned long, (unsigned long)b) && \
	 ((long)(a) - (long)(b) >= 0))
#define RTMP_TIME_BEFORE(a,b)	RTMP_TIME_AFTER_EQ(b,a)
#define RTMP_TIME_AFTER(a,b) time_after(a, b)
#endif

#define ONE_TICK 1

static inline void NdisGetSystemUpTime(ULONG *time)
{
	*time = jiffies;
}


/***********************************************************************************
 *	OS specific cookie data structure binding to RTMP_ADAPTER
 ***********************************************************************************/

struct os_cookie {
#ifdef RTMP_MAC_PCI
	struct pci_dev 			*pci_dev;
	struct pci_dev 			*parent_pci_dev;
	USHORT                  DeviceID;
	dma_addr_t		  		pAd_pa;
#endif // RTMP_MAC_PCI //


#ifdef WORKQUEUE_BH
	UINT32		     pAd_va;

	struct work_struct   rx_done_work;
	struct work_struct   mgmt_dma_done_work;
	struct work_struct   ac0_dma_done_work;
	struct work_struct   ac1_dma_done_work;
	struct work_struct   ac2_dma_done_work;
	struct work_struct   ac3_dma_done_work;
	struct work_struct   hcca_dma_done_work;
	
	struct work_struct   tbtt_work;

#else
	RTMP_NET_TASK_STRUCT rx_done_task;
	RTMP_NET_TASK_STRUCT mgmt_dma_done_task;
	RTMP_NET_TASK_STRUCT ac0_dma_done_task;
#ifdef RALINK_ATE
	RTMP_NET_TASK_STRUCT ate_ac0_dma_done_task;
#endif // RALINK_ATE //
	RTMP_NET_TASK_STRUCT ac1_dma_done_task;
	RTMP_NET_TASK_STRUCT ac2_dma_done_task;
	RTMP_NET_TASK_STRUCT ac3_dma_done_task;
	RTMP_NET_TASK_STRUCT hcca_dma_done_task;
	RTMP_NET_TASK_STRUCT tbtt_task;
#endif // WORKQUEUE_BH //

#ifdef RTMP_MAC_PCI
#ifdef WORKQUEUE_BH
	struct work_struct   fifo_statistic_full_work;
#else
	RTMP_NET_TASK_STRUCT fifo_statistic_full_task;
#endif // WORKQUEUE_BH //
#endif // RTMP_MAC_PCI //

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct   uapsd_eosp_sent_work;
#else
	RTMP_NET_TASK_STRUCT uapsd_eosp_sent_task;
#endif // WORKQUEUE_BH //
#endif // UAPSD_AP_SUPPORT //

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct   pulse_radar_detect_work;
	struct work_struct   width_radar_detect_work;
#else
	RTMP_NET_TASK_STRUCT pulse_radar_detect_task;
	RTMP_NET_TASK_STRUCT width_radar_detect_task;
#endif // WORKQUEUE_BH //
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct   carrier_sense_work;
#else
	RTMP_NET_TASK_STRUCT carrier_sense_task;
#endif // WORKQUEUE_BH //
#endif // CARRIER_DETECTION_SUPPORT //

#ifdef DFS_HARDWARE_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct  	dfs_work;
#else
	struct tasklet_struct	dfs_task;
#endif // WORKQUEUE_BH //
#endif // DFS_HARDWARE_SUPPORT //

#endif // CONFIG_AP_SUPPORT //


	RTMP_OS_PID			apd_pid; //802.1x daemon pid
	unsigned long			apd_pid_nr;
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
//	RT_SIGNAL_STRUC			RTSignal;
	RTMP_OS_PID			IappPid; //IAPP daemon pid
	unsigned long			IappPid_nr;
#endif // IAPP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //
#ifdef WAPI_SUPPORT
	RTMP_OS_PID			wapi_pid; //wapi daemon pid
	unsigned long			wapi_pid_nr;
#endif // WAPI_SUPPORT //
	INT						ioctl_if_type;
	INT 					ioctl_if;
};

typedef struct os_cookie	* POS_COOKIE;



/***********************************************************************************
 *	OS debugging and printing related definitions and data structure
 ***********************************************************************************/
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#ifdef DBG
extern ULONG		RTDebugLevel;

#define DBGPRINT_RAW(Level, Fmt)    \
do{                                   \
    if (Level <= RTDebugLevel)      \
    {                               \
        printk Fmt;               \
    }                               \
}while(0)

#define DBGPRINT(Level, Fmt)    DBGPRINT_RAW(Level, Fmt)


#define DBGPRINT_ERR(Fmt)           \
{                                   \
    printk("ERROR!!! ");          \
    printk Fmt;                  \
}

#define DBGPRINT_S(Status, Fmt)		\
{									\
	printk Fmt;					\
}
#else
#define DBGPRINT(Level, Fmt)		do{}while(0)
#define DBGPRINT_RAW(Level, Fmt)	do{}while(0)
#define DBGPRINT_S(Status, Fmt)		do{}while(0)
#define DBGPRINT_ERR(Fmt)		do{}while(0)
#endif

#undef  ASSERT
#ifdef DBG
#define ASSERT(x)                                                               \
{                                                                               \
    if (!(x))                                                                   \
    {                                                                           \
        printk(KERN_WARNING __FILE__ ":%d assert " #x "failed\n", __LINE__);    \
    }                                                                           \
}
#else
#define ASSERT(x)
#endif // DBG //

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);


/*********************************************************************************************************
	The following code are not revised, temporary put it here.
  *********************************************************************************************************/


/***********************************************************************************
 * Device DMA Access related definitions and data structures.
 **********************************************************************************/
#ifdef RTMP_MAC_PCI
dma_addr_t linux_pci_map_single(void *handle, void *ptr, size_t size, int sd_idx, int direction);
void linux_pci_unmap_single(void *handle, dma_addr_t dma_addr, size_t size, int direction);

#define PCI_MAP_SINGLE(_handle, _ptr, _size, _sd_idx, _dir) \
	linux_pci_map_single(_handle, _ptr, _size, _sd_idx, _dir)
	
#define PCI_UNMAP_SINGLE(_handle, _ptr, _size, _dir) \
	linux_pci_unmap_single(_handle, _ptr, _size, _dir)

#define PCI_ALLOC_CONSISTENT(_pci_dev, _size, _ptr) \
	pci_alloc_consistent(_pci_dev, _size, _ptr)

#define PCI_FREE_CONSISTENT(_pci_dev, _size, _virtual_addr, _physical_addr) \
	pci_free_consistent(_pci_dev, _size, _virtual_addr, _physical_addr)
#endif // RTMP_MAC_PCI //

#ifdef VENDOR_FEATURE2_SUPPORT
#define DEV_ALLOC_SKB(_pAd, _Pkt, _length)	\
	_Pkt = dev_alloc_skb(_length);			\
	if (_Pkt != NULL) _pAd->NumOfPktAlloc ++;
#else

#define DEV_ALLOC_SKB(_pAd, _Pkt, _length)	\
	_Pkt = dev_alloc_skb(_length);
#endif // VENDOR_FEATURE2_SUPPORT //



/*
 * ULONG
 * RTMP_GetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressLow(PhysicalAddress)		(PhysicalAddress)

/*
 * ULONG
 * RTMP_GetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressHigh(PhysicalAddress)		(0)

/*
 * VOID
 * RTMP_SetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressLow(PhysicalAddress, Value)	\
			PhysicalAddress = Value;

/*
 * VOID
 * RTMP_SetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressHigh(PhysicalAddress, Value)

#define NdisMIndicateStatus(_w, _x, _y, _z)



/***********************************************************************************
 * Device Register I/O Access related definitions and data structures.
 **********************************************************************************/
#ifdef RTMP_MAC_PCI
#if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(INF_AR9) || defined(IKANOS_VX_1X0)
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)									\
{																	\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
	(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
}

#define RTMP_IO_READ32(_A, _R, _pV)									\
{																	\
    if ((_A)->bPCIclkOff == FALSE)                                      \
    {                                                                   \
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
	(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
    }                                                                   \
}

#define RTMP_IO_READ8(_A, _R, _pV)									\
{																	\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readb((void *)((_A)->CSRBaseAddress + (_R))));			\
}

#define _RTMP_IO_WRITE32(_A, _R, _V)									\
{																	\
    if ((_A)->bPCIclkOff == FALSE)                                      \
    {                                                                   \
	UINT32	_Val;													\
	_Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));		\
	_Val = SWAP32(_V);												\
	writel(_Val, (void *)((_A)->CSRBaseAddress + (_R)));			\
    }                                                                   \
}

#ifdef INF_VR9
#define RTMP_IO_WRITE8(_A, _R, _V)            \
{                    \
        ULONG Val;                \
        UCHAR _i;                \
	UINT32 _Val;		\
        _i = ((_R) & 0x3);             \
        Val = readl((void *)((_A)->CSRBaseAddress + ((_R) - _i)));   \
	Val = SWAP32(Val);				\
        Val = Val & (~(0x000000ff << ((_i)*8)));         \
        Val = Val | ((ULONG)(_V) << ((_i)*8));         \
	Val = SWAP32(Val);				\
        writel((Val), (void *)((_A)->CSRBaseAddress + ((_R) - _i)));    \
}
#else
#define RTMP_IO_WRITE8(_A, _R, _V)									\
{																	\
	UINT	Val;													\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));		\
	writeb((_V), (PUCHAR)((_A)->CSRBaseAddress + (_R)));			\
}
#endif

#define RTMP_IO_WRITE16(_A, _R, _V)									\
{																	\
	UINT	Val;													\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));		\
	writew(SWAP16((_V)), (PUSHORT)((_A)->CSRBaseAddress + (_R)));	\
}
#else
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)								\
{																\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
}

#define RTMP_IO_READ32(_A, _R, _pV)								\
{																\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
		(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
		(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
    }                                                               \
    else															\
		*_pV = 0;													\
}

#define RTMP_IO_FORCE_READ32(_A, _R, _pV)							\
{																	\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
}

#define RTMP_IO_READ8(_A, _R, _pV)								\
{																\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));			\
	(*_pV = readb((void *)((_A)->CSRBaseAddress + (_R))));				\
}

#define _RTMP_IO_WRITE32(_A, _R, _V)												\
{																				\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
	UINT	Val;																\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));			\
	writel((_V), (void *)((_A)->CSRBaseAddress + (_R)));								\
    }                                                               \
}

#define RTMP_IO_FORCE_WRITE32(_A, _R, _V)												\
{																				\
	UINT	Val;																\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));			\
	writel(_V, (void *)((_A)->CSRBaseAddress + (_R)));								\
}



#if defined(BRCM_6358) || defined(RALINK_2880) || defined(RALINK_3052) || defined(RALINK_2883) || defined(RTMP_RBUS_SUPPORT) || defined(BB_PCIE_ADDR_SWAP)
#define RTMP_IO_WRITE8(_A, _R, _V)            \
{                    \
	ULONG Val;                \
	UCHAR _i;                \
	_i = ((_R) & 0x3);             \
	Val = readl((void *)((_A)->CSRBaseAddress + ((_R) - _i)));   \
	Val = Val & (~(0x000000ff << ((_i)*8)));         \
	Val = Val | ((ULONG)(_V) << ((_i)*8));         \
	writel((Val), (void *)((_A)->CSRBaseAddress + ((_R) - _i)));    \
}
#else
#define RTMP_IO_WRITE8(_A, _R, _V)							\
{															\
	UINT	Val;												\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));	\
	writeb((_V), (PUCHAR)((_A)->CSRBaseAddress + (_R)));		\
}
#endif /* defined(BRCM_6358) || defined(RALINK_2880) || defined(RALINK_3052) || defined(RALINK_2883) || defined(RTMP_RBUS_SUPPORT) || defined(BB_PCIE_ADDR_SWAP) */

#define RTMP_IO_WRITE16(_A, _R, _V)							\
{															\
	UINT	Val;											\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));	\
	writew((_V), (PUSHORT)((_A)->CSRBaseAddress + (_R)));	\
}
#endif // #if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(IKANOS_VX_1X0) //


#ifndef VENDOR_FEATURE3_SUPPORT
#define RTMP_IO_WRITE32		_RTMP_IO_WRITE32
#endif // VENDOR_FEATURE3_SUPPORT //

#endif // RTMP_MAC_PCI //



/***********************************************************************************
 *	Network Related data structure and marco definitions
 ***********************************************************************************/

#define RTMP_OS_NETDEV_STATE_RUNNING(_pNetDev)	((_pNetDev)->flags & IFF_UP)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#define _RTMP_OS_NETDEV_GET_PRIV(_pNetDev)		((_pNetDev)->ml_priv)
#define _RTMP_OS_NETDEV_SET_PRIV(_pNetDev, _pPriv)	((_pNetDev)->ml_priv = (_pPriv))
#else
#define _RTMP_OS_NETDEV_GET_PRIV(_pNetDev)		((_pNetDev)->priv)
#define _RTMP_OS_NETDEV_SET_PRIV(_pNetDev, _pPriv)	((_pNetDev)->priv = (_pPriv))
#endif
#define RTMP_OS_NETDEV_GET_DEVNAME(_pNetDev)	((_pNetDev)->name)
#define RTMP_OS_NETDEV_GET_PHYADDR(_pNetDev)	((_pNetDev)->dev_addr)

/* Get & Set NETDEV interface hardware type */
#define RTMP_OS_NETDEV_GET_TYPE(_pNetDev)			((_pNetDev)->type)
#define RTMP_OS_NETDEV_SET_TYPE(_pNetDev, _type)	((_pNetDev)->type = (_type))

#define RTMP_OS_NETDEV_START_QUEUE(_pNetDev)	netif_start_queue((_pNetDev))
#define RTMP_OS_NETDEV_STOP_QUEUE(_pNetDev)	netif_stop_queue((_pNetDev))
#define RTMP_OS_NETDEV_WAKE_QUEUE(_pNetDev)	netif_wake_queue((_pNetDev))
#define RTMP_OS_NETDEV_CARRIER_OFF(_pNetDev)	netif_carrier_off((_pNetDev))

#define QUEUE_ENTRY_TO_PACKET(pEntry) \
	(PNDIS_PACKET)(pEntry)

#define PACKET_TO_QUEUE_ENTRY(pPacket) \
	(PQUEUE_ENTRY)(pPacket)

#ifdef CONFIG_5VT_ENHANCE
#define BRIDGE_TAG 0x35564252    // depends on 5VT define in br_input.c
#endif

#define GET_SG_LIST_FROM_PACKET(_p, _sc)	\
    rt_get_sg_list_from_packet(_p, _sc)

#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)                    \
{                                                                       \
        RTMPFreeNdisPacket(_pAd, _pPacket);                             \
}

/*
 * packet helper
 * 	- convert internal rt packet to os packet or
 *             os packet to rt packet
 */
#define RTPKT_TO_OSPKT(_p)		((struct sk_buff *)(_p))
#define OSPKT_TO_RTPKT(_p)		((PNDIS_PACKET)(_p))

#define GET_OS_PKT_DATAPTR(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->data)
#define SET_OS_PKT_DATAPTR(_pkt, _dataPtr)	\
		(RTPKT_TO_OSPKT(_pkt)->data) = (_dataPtr)

#define GET_OS_PKT_LEN(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->len)
#define SET_OS_PKT_LEN(_pkt, _len)	\
		(RTPKT_TO_OSPKT(_pkt)->len) = (_len)
		
#define GET_OS_PKT_DATATAIL(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->tail)
#define SET_OS_PKT_DATATAIL(_pkt, _start, _len)	\
		((RTPKT_TO_OSPKT(_pkt))->tail) = (PUCHAR)((_start) + (_len))
		
#define GET_OS_PKT_HEAD(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->head)

#define GET_OS_PKT_END(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->end)

#define GET_OS_PKT_NETDEV(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->dev)
#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
		(RTPKT_TO_OSPKT(_pkt)->dev) = (_pNetDev)
		
#define GET_OS_PKT_TYPE(_pkt) \
		(RTPKT_TO_OSPKT(_pkt))

#define GET_OS_PKT_NEXT(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->next)


#define OS_PKT_CLONED(_pkt)		skb_cloned(RTPKT_TO_OSPKT(_pkt))

#ifdef VENDOR_FEATURE2_SUPPORT
#define OS_PKT_CLONE(_pAd, _pkt, _src, _flag)		\
	_src = skb_clone(RTPKT_TO_OSPKT(_pkt), _flag);	\
	if (_src != NULL) _pAd->NumOfPktAlloc ++;
#else

#define OS_PKT_CLONE(_pAd, _pkt, _src, _flag)		\
	_src = skb_clone(RTPKT_TO_OSPKT(_pkt), _flag);
#endif // VENDOR_FEATURE2_SUPPORT //

#define OS_NTOHS(_Val) \
		(ntohs(_Val))
#define OS_HTONS(_Val) \
		(htons(_Val))
#define OS_NTOHL(_Val) \
		(ntohl(_Val))
#define OS_HTONL(_Val) \
		(htonl(_Val))

#define CB_OFF  10

// User Priority
#define RTMP_SET_PACKET_UP(_p, _prio)			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+0] = _prio)
#define RTMP_GET_PACKET_UP(_p)					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+0])

// Fragment #
#define RTMP_SET_PACKET_FRAGMENTS(_p, _num)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+1] = _num)
#define RTMP_GET_PACKET_FRAGMENTS(_p)			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+1])

// 0x0 ~0x7f: TX to AP's own BSS which has the specified AID. if AID>127, set bit 7 in RTMP_SET_PACKET_EMACTAB too.
//(this value also as MAC(on-chip WCID) table index)
// 0x80~0xff: TX to a WDS link. b0~6: WDS index
#define RTMP_SET_PACKET_WCID(_p, _wdsidx)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+2] = _wdsidx)
#define RTMP_GET_PACKET_WCID(_p)          		((UCHAR)(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+2]))

// RTS/CTS-to-self protection method
#define RTMP_SET_PACKET_RTS(_p, _num)      		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+4] = _num)
#define RTMP_GET_PACKET_RTS(_p)          		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+4])
// see RTMP_S(G)ET_PACKET_EMACTAB

// TX rate index
#define RTMP_SET_PACKET_TXRATE(_p, _rate)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+5] = _rate)
#define RTMP_GET_PACKET_TXRATE(_p)		  		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+5])

// From which Interface
#define RTMP_SET_PACKET_IF(_p, _ifdx)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+6] = _ifdx)
#define RTMP_GET_PACKET_IF(_p)		  		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+6])
#define RTMP_SET_PACKET_NET_DEVICE_MBSSID(_p, _bss)		RTMP_SET_PACKET_IF((_p), (_bss))
#define RTMP_SET_PACKET_NET_DEVICE_WDS(_p, _bss)		RTMP_SET_PACKET_IF((_p), ((_bss) + MIN_NET_DEVICE_FOR_WDS))
#define RTMP_SET_PACKET_NET_DEVICE_APCLI(_p, _idx)   	RTMP_SET_PACKET_IF((_p), ((_idx) + MIN_NET_DEVICE_FOR_APCLI))
#define RTMP_SET_PACKET_NET_DEVICE_MESH(_p, _idx)   	RTMP_SET_PACKET_IF((_p), ((_idx) + MIN_NET_DEVICE_FOR_MESH))
#define RTMP_GET_PACKET_NET_DEVICE_MBSSID(_p)			RTMP_GET_PACKET_IF((_p))
#define RTMP_GET_PACKET_NET_DEVICE(_p)					RTMP_GET_PACKET_IF((_p))

#define RTMP_SET_PACKET_MOREDATA(_p, _morebit)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+7] = _morebit)
#define RTMP_GET_PACKET_MOREDATA(_p)				(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+7])



#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
/* if we queue a U-APSD packet to any software queue, we will set the U-APSD
   flag and its physical queue ID for it */
#define RTMP_SET_PACKET_UAPSD(_p, _flg_uapsd, _que_id) \
                    (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+9] = ((_flg_uapsd<<7) | _que_id))

#define RTMP_SET_PACKET_QOS_NULL(_p)     (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+9] = 0xff)
#define RTMP_GET_PACKET_QOS_NULL(_p)	 (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+9])
#define RTMP_SET_PACKET_NON_QOS_NULL(_p) (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+9] = 0x00)
#define RTMP_GET_PACKET_UAPSD_Flag(_p)   (((RTPKT_TO_OSPKT(_p)->cb[CB_OFF+9]) & 0x80) >> 7)
#define RTMP_GET_PACKET_UAPSD_QUE_ID(_p) ((RTPKT_TO_OSPKT(_p)->cb[CB_OFF+9]) & 0x7f)

#define RTMP_SET_PACKET_EOSP(_p, _flg)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+10] = _flg)
#define RTMP_GET_PACKET_EOSP(_p)         (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+10])
#endif // UAPSD_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


//
//	Sepcific Pakcet Type definition
//
#define RTMP_PACKET_SPECIFIC_CB_OFFSET	11

#define RTMP_PACKET_SPECIFIC_DHCP		0x01
#define RTMP_PACKET_SPECIFIC_EAPOL		0x02
#define RTMP_PACKET_SPECIFIC_IPV4		0x04
#define RTMP_PACKET_SPECIFIC_WAI		0x08
#define RTMP_PACKET_SPECIFIC_VLAN		0x10
#define RTMP_PACKET_SPECIFIC_LLCSNAP	0x20

//Specific
#define RTMP_SET_PACKET_SPECIFIC(_p, _flg)	   	(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] = _flg)

//DHCP
#define RTMP_SET_PACKET_DHCP(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_DHCP);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_DHCP);	\
			}while(0)
#define RTMP_GET_PACKET_DHCP(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_DHCP)

//EAPOL
#define RTMP_SET_PACKET_EAPOL(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_EAPOL);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_EAPOL);	\
			}while(0)
#define RTMP_GET_PACKET_EAPOL(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_EAPOL)

//WAI
#define RTMP_SET_PACKET_WAI(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_WAI);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_WAI);	\
			}while(0)
#define RTMP_GET_PACKET_WAI(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_WAI)

#define RTMP_GET_PACKET_LOWRATE(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & (RTMP_PACKET_SPECIFIC_EAPOL | RTMP_PACKET_SPECIFIC_DHCP | RTMP_PACKET_SPECIFIC_WAI))

//VLAN
#define RTMP_SET_PACKET_VLAN(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_VLAN);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_VLAN);	\
			}while(0)
#define RTMP_GET_PACKET_VLAN(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_VLAN)

//LLC/SNAP
#define RTMP_SET_PACKET_LLCSNAP(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_LLCSNAP);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_LLCSNAP);		\
			}while(0)
			
#define RTMP_GET_PACKET_LLCSNAP(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_LLCSNAP)

// IP
#define RTMP_SET_PACKET_IPV4(_p, _flg)														\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_IPV4);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_IPV4);	\
			}while(0)
			
#define RTMP_GET_PACKET_IPV4(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_IPV4)


// If this flag is set, it indicates that this EAPoL frame MUST be clear.
#define RTMP_SET_PACKET_CLEAR_EAP_FRAME(_p, _flg)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+12] = _flg)
#define RTMP_GET_PACKET_CLEAR_EAP_FRAME(_p)         (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+12])


#define MAX_PACKETS_IN_QUEUE				(512)

#ifdef WMM_ACM_SUPPORT
/* total [CB_OFF+15] ~ [CB_OFF+19] 5B */

/* [CB_OFF+15], 1B (4bit vs 4bit, Packet Type vs UP), are reserved for ACM classification use */
/* reference to ACM_PACKET_INFO in acm_extr.h */
/* used in ACMP_DataPacketQueue() */
#define RTMP_GET_PACKET_ACM_PARAM_INFO(_p)		(&(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+15]))

/* [CB_OFF+16] (bit0~2, Phy Mode) ~ [CB_OFF+17] (bit0~7, MCS), 2B, are reserved for minimum tx rate use */
/* used in ACMP_PacketPhyModeMCSCheck() */
#define RTMP_SET_PACKET_ACM_MIN_PHY_MODE(_p, _mode)			\
	RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] = (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] & 0xF8) | (_mode & 0x07);
#define RTMP_GET_PACKET_ACM_MIN_PHY_MODE(_p)				\
	(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] & 0x07)

#define RTMP_SET_PACKET_ACM_MIN_PHY_MCS(_p, _mcs)	(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+17] = _mcs)
#define RTMP_GET_PACKET_ACM_MIN_PHY_MCS(_p)			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+17])

/* [CB_OFF+16] (bit4~7), are reserved for TSPEC TSID (0~15) use */
/* used in ACMP_MsduClassify() */
#define RTMP_SET_PACKET_STM_TSID(_p, _tsid)		\
	RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] = (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] & 0x0F) | ((_tsid & 0x0F) << 4);
#define RTMP_GET_PACKET_STM_TSID(_p)			\
	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] & 0xF0) >> 4)

/* [CB_OFF+18] ~ [CB_OFF+19], 2B, are reserved for tx time use */
/* used in ACMP_DataPacketQueue() */
#define RTMP_SET_PACKET_TX_TIME(_p, _txtime)	(*(UINT16 *)&RTPKT_TO_OSPKT(_p)->cb[CB_OFF+18] = _txtime)
#define RTMP_GET_PACKET_TX_TIME(_p)				(*(UINT16 *)&RTPKT_TO_OSPKT(_p)->cb[CB_OFF+18])
#endif // WMM_ACM_SUPPORT //

/* use bit3 of cb[CB_OFF+16] */
#ifdef CONFIG_AP_SUPPORT
#define RTMP_SET_PACKET_MGMT_PKT(_p, _flg)	        \
        RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] = (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] & 0xF7) | ((_flg & 0x01) << 3);
#define RTMP_GET_PACKET_MGMT_PKT(_p)				\
		((RTPKT_TO_OSPKT(_p)->cb[CB_OFF+16] & 0x08) >> 3)
#endif // CONFIG_AP_SUPPORT //

#define RTMP_SET_PACKET_5VT(_p, _flg)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+22] = _flg)
#define RTMP_GET_PACKET_5VT(_p)         (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+22])

#define RTMP_SET_PACKET_PROTOCOL(_p, _protocol) {\
	(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+23] = (UINT8)((_protocol) & 0x00ff)); \
	(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+24] = (UINT8)(((_protocol) & 0xff00) >> 8)); \
}

#define RTMP_GET_PACKET_PROTOCOL(_p) \
	((((UINT16)(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+24]) & 0x00ff) << 8) \
	| ((UINT16)(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+23]) & 0x00ff))

#ifdef INF_AMAZON_SE
/* [CB_OFF+28], 1B, Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
#define RTMP_SET_PACKET_NOBULKOUT(_p, _morebit)			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+28] = _morebit)
#define RTMP_GET_PACKET_NOBULKOUT(_p)					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+28])			
#endif // INF_AMAZON_SE //
/* Max skb->cb = 48B = [CB_OFF+38] */






/***********************************************************************************
 *	Other function prototypes definitions
 ***********************************************************************************/
void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);
int rt28xx_packet_xmit(struct sk_buff *skb);


#if LINUX_VERSION_CODE <= 0x20402	// Red Hat 7.1
struct net_device *alloc_netdev(int sizeof_priv, const char *mask, void (*setup)(struct net_device *));
#endif // LINUX_VERSION_CODE //


#ifdef RTMP_MAC_PCI
/* function declarations */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define IRQ_HANDLE_TYPE  irqreturn_t
#else
#define IRQ_HANDLE_TYPE  void
#endif

IRQ_HANDLE_TYPE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
rt2860_interrupt(int irq, void *dev_instance);
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#endif

#endif // RTMP_MAC_PCI //

INT rt28xx_ioctl(
	IN	PNET_DEV		net_dev,
	IN	OUT	struct ifreq	*rq, 
	IN	INT			cmd);

#ifdef CONFIG_AP_SUPPORT
INT rt28xx_ap_ioctl(
	IN	PNET_DEV		net_dev, 
	IN	OUT	struct ifreq	*rq, 
	IN	INT			cmd);
#endif // CONFIG_AP_SUPPORT //


extern int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#define _GET_PAD_FROM_NET_DEV(_pAd, _net_dev)	(_pAd) = (PRTMP_ADAPTER)(_net_dev)->ml_priv;
#else
#define _GET_PAD_FROM_NET_DEV(_pAd, _net_dev)	(_pAd) = (PRTMP_ADAPTER)(_net_dev)->priv;
#endif

#define GET_PAD_FROM_NET_DEV(_pAd, _net_dev)	\
	_pAd = RTMP_OS_NETDEV_GET_PRIV(_net_dev);

#ifdef RALINK_ATE
/******************************************************************************

  	ATE related definitions

******************************************************************************/
#define ate_print printk
#define ATEDBGPRINT DBGPRINT
#ifdef RTMP_MAC_PCI
#ifdef CONFIG_AP_SUPPORT
#if defined (RT_IFNAME_1ST)
 #define EEPROM_BIN_FILE_NAME		"/etc/Wireless/RT2860/e2p.bin"
#else
 #define EEPROM_BIN_FILE_NAME		"/etc/Wireless/iNIC/e2p.bin"
#endif
#endif // CONFIG_AP_SUPPORT //
#endif // RTMP_MAC_PCI //

#endif // RALINK_ATE //


#define RTMP_OS_NETDEV_SET_PRIV		RtmpOsSetNetDevPriv
#define RTMP_OS_NETDEV_GET_PRIV		RtmpOsGetNetDevPriv
#define RT_DEV_PRIV_FLAGS_GET		RtmpDevPrivFlagsGet
#define RT_DEV_PRIV_FLAGS_SET		RtmpDevPrivFlagsSet

VOID RtmpOsSetNetDevPriv(
	IN	VOID					*pDev,
	IN	VOID					*pPriv);

VOID *RtmpOsGetNetDevPriv(
	IN	VOID					*pDev);

USHORT RtmpDevPrivFlagsGet(
	IN	VOID					*pDev);

VOID RtmpDevPrivFlagsSet(
	IN	VOID					*pDev,
	IN	USHORT					PrivFlags);

/* associated with device interface */
typedef struct _DEV_PRIV_INFO {
	VOID			*pPriv; /* pAd */
	UINT32			priv_flags;
} DEV_PRIV_INFO;


#endif // __RT_LINUX_H__ //
