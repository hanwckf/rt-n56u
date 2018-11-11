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
 ***************************************************************************

    Module Name:
	rt_ecos.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RT_ECOS_H__
#define __RT_ECOS_H__

#define LLTD_SUPPORT 1

#include <stdio.h>
#include <stdlib.h>

#include <cyg/infra/diag.h>     /* For dial_printf */
#include <cyg/hal/hal_if.h>     /* For CYGACC_CALL_IF_DELAY_US */
#include <cyg/hal/hal_cache.h>  /* For HAL_DCACHE_INVALIDATE */
#include <cyg/kernel/kapi.h>
#include <cyg/io/eth/netdev.h>  /* For struct eth_drv_sc */
#include <cyg/io/eth/eth_drv.h> /* For eth_drv_netdev */
#include <cyg/io/flash.h>       /* For SPI Flash */
#include <sys/mbuf.h>           /* Memory Pool */
#include <sys/param.h>          /* For tsleep */
#include <stdarg.h>

#include <pkgconf/system.h>
#ifdef BRANCH_ADV
#undef CONFIG_BRIDGE
#include <autoconf.h>
#endif

#include "rtmp_type.h"
/*#include "rtmp_def.h" */

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define RTMP_TIMER_TASK_SUPPORT

/***********************************************************************************
 *	Profile related sections
 ***********************************************************************************/
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
#define AP_PROFILE_PATH			"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define AP_RTMP_FIRMWARE_FILE_NAME "/etc/Wireless/RT2860AP/RT2860AP.bin"
#define AP_DRIVER_VERSION			"2.6.0.0"
#ifdef MULTIPLE_CARD_SUPPORT
#define CARD_INFO_PATH			"/etc/Wireless/RT2860AP/RT2860APCard.dat"
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RTMP_MAC_PCI */


#endif /* CONFIG_AP_SUPPORT */



#ifdef CONFIG_APSTA_MIXED_SUPPORT
extern	const struct iw_handler_def rt28xx_ap_iw_handler_def;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

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
#define RTMP_INTERRUPT_INIC  CYGNUM_HAL_INTERRUPT_IP5
typedef unsigned int		ra_dma_addr_t;
typedef struct eth_drv_sc	*PNET_DEV;
typedef void				*PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET		*PPNDIS_PACKET;
typedef	ra_dma_addr_t			NDIS_PHYSICAL_ADDRESS;
typedef	ra_dma_addr_t			*PNDIS_PHYSICAL_ADDRESS;
typedef void				*NDIS_HANDLE;
typedef char 				*PNDIS_BUFFER;

typedef int 				RTMP_OS_PID;
typedef cyg_sem_t	        RTMP_OS_SEM;

#ifdef RTMP_MAC_PCI
#ifndef PCI_DEVICE
#define PCI_DEVICE(vend,dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif /* PCI_DEVICE */
#endif /* RTMP_MAC_PCI */

#define RT_MOD_DEC_USE_COUNT()	do{}while(0)
#define RT_MOD_INC_USE_COUNT()	do{}while(0)

#define RT_MOD_HNAT_DEREG(_net_dev) do{}while(0)
#define RT_MOD_HNAT_REG(_net_dev) do{}while(0)

#define RTMP_INC_REF(_A)		0
#define RTMP_DEC_REF(_A)		0
#define RTMP_GET_REF(_A)		0

#define isxdigit(_char)  \
	(('0' <= (_char) && (_char) <= '9') ||\
	 ('a' <= (_char) && (_char) <= 'f') || \
	 ('A' <= (_char) && (_char) <= 'F') \
	)

#define isdigit(ch) \
	((ch >= '0') && (ch <= '9'))


#define tolower(ch) ((ch >= 'A' && (ch) <= 'Z') ? (ch) - ('A' - 'a') : (ch))

#define toupper(ch) ((ch >= 'a' && (ch) <= 'z') ? (ch) + ('A' - 'a') : (ch))

#define IWEVCUSTOM	0x8C02		/* Driver specific ascii string */
#define IW_PRIV_SIZE_MASK       0x07FF  /* Max number of those args */

/* Define Linux ioctl relative structure, keep only necessary things */
struct iw_point
{
	PVOID		pointer;
	USHORT		length;
	USHORT		flags;
};
	
union iwreq_data
{
	struct iw_point data;
};

struct iwreq 
{
	union
	{
		char    ifrn_name[IFNAMSIZ];    /* if name, e.g. "eth0" */
	} ifr_ifrn;
	
	union   iwreq_data      u;
};

/* Maximum size of returned data, sync with wireless.h in Linux */
#define IW_SCAN_MAX_DATA        4096    /* In bytes */

/*
 *	Data unalignment handling related definitions and data structure
 */
extern void bad_unaligned_access_length(void) __attribute__((noreturn));

struct UINT64 { UINT64 x __attribute__((packed)); };
struct UINT32 { UINT32 x __attribute__((packed)); };
struct UINT16 { UINT16 x __attribute__((packed)); };

/*
 * Elemental unaligned loads 
 */
static inline UINT64 __uldq(const UINT64 *addr)
{
	const struct UINT64 *ptr = (const struct UINT64 *) addr;
	return ptr->x;
}

static inline UINT32 __uldl(const UINT32 *addr)
{
	const struct UINT32 *ptr = (const struct UINT32 *) addr;
	return ptr->x;
}



/*
 * Elemental unaligned stores 
 */
static inline void __ustq(UINT64 val, UINT64 *addr)
{
	struct UINT64 *ptr = (struct UINT64 *) addr;
	ptr->x = val;
}

static inline void __ustl(UINT32 val, UINT32 *addr)
{
	struct UINT32 *ptr = (struct UINT32 *) addr;
	ptr->x = val;
}

static inline void __ustw(UINT16 val, UINT16 *addr)
{
	struct UINT16 *ptr = (struct UINT16 *) addr;
	ptr->x = val;
}

static inline USHORT __uldw(const UINT16 *addr)
{
	const struct UINT16 *ptr = (const struct UINT16 *) addr;
	return ptr->x;
}

#define __get_unaligned(ptr, size) ({		\
	const void *__gu_p = ptr;		\
	__typeof__(*(ptr)) val;			\
	switch (size) {				\
	case 1:					\
		val = *(const UINT8 *)__gu_p;	\
		break;				\
	case 2:					\
		val = __uldw(__gu_p);		\
		break;				\
	case 4:					\
		val = __uldl(__gu_p);		\
		break;				\
	case 8:					\
		val = __uldq(__gu_p);		\
		break;				\
	default:				\
		bad_unaligned_access_length();	\
	};					\
	val;					\
})

#define __put_unaligned(val, ptr, size)		\
do {						\
	void *__gu_p = ptr;			\
	switch (size) {				\
	case 1:					\
		*(UINT8 *)__gu_p = val;		\
	        break;				\
	case 2:					\
		__ustw(val, __gu_p);		\
		break;				\
	case 4:					\
		__ustl(val, __gu_p);		\
		break;				\
	case 8:					\
		__ustq(val, __gu_p);		\
		break;				\
	default:				\
	    	bad_unaligned_access_length();	\
	};					\
} while(0)

#define get_unaligned(ptr) \
	__get_unaligned((ptr), sizeof(*(ptr)))
#define put_unaligned(x,ptr) \
	__put_unaligned((UINT64)(x), (ptr), sizeof(*(ptr)))


/***********************************************************************************
 *	Network related constant definitions
 ***********************************************************************************/
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define ETH_P_IPV6	0x86DD
#define ETH_P_IP		0x0800

#define NDIS_STATUS_SUCCESS			0x00
#define NDIS_STATUS_FAILURE			0x01
#define NDIS_STATUS_INVALID_DATA		0x02
#define NDIS_STATUS_RESOURCES			0x03

#define NDIS_SET_PACKET_STATUS(_p, _status)		do{} while(0)
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
#define RT_DEV_PRIV_FLAGS_GET(__pDev)									\
	((PNET_DEV)pDev)->priv_flags

#define RT_DEV_PRIV_FLAGS_SET(__pDev, __PrivFlags)						\
	((PNET_DEV)pDev)->priv_flags = __PrivFlags



/***********************************************************************************
 *	OS signaling related constant definitions
 ***********************************************************************************/
/*EddyTODO */
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
typedef struct _RT_SIGNAL_STRUC {
	unsigned short Sequence;
	unsigned char MacAddr[MAC_ADDR_LEN];
	unsigned char CurrAPAddr[MAC_ADDR_LEN];
	unsigned char Sig;
} RT_SIGNAL_STRUC, *PRT_SIGNAL_STRUC;

/* definition of signal */
#define	SIG_NONE					0
#define SIG_ASSOCIATION				1
#define SIG_REASSOCIATION			2
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

/***********************************************************************************
 *	OS file operation related data structure definitions
 ***********************************************************************************/
/*EddyTODO */
typedef int RTMP_OS_FD;

typedef struct _RTMP_OS_FS_INFO_
{
	int	fsinfo; /* Dummy structure, for cross-platform compatibility */
}RTMP_OS_FS_INFO;

#define IS_FILE_OPEN_ERR(_fd) 	((_fd) < 0)

/***********************************************************************************
 *	OS semaphore related data structure and definitions
 ***********************************************************************************/
typedef cyg_sem_t           NDIS_SPIN_LOCK;

/* */
/*  spin_lock enhanced for Nested spin lock */
/* */
#define NdisAllocateSpinLock(__pReserve, __lock)	\
        cyg_semaphore_init(__lock, 1);

#define NdisFreeSpinLock(__lock)		\
		cyg_semaphore_destroy(__lock)

#define RTMP_Spinlock_LOCK(__lock)	\
		cyg_semaphore_wait(__lock)
		
#define RTMP_Spinlock_UNLOCK(__lock)	\
		cyg_semaphore_post(__lock)


/* sample, use semaphore lock to replace IRQ lock, 2007/11/15 */
#define RTMP_IRQ_LOCK(__lock, __irqflags)	\
{											\
	__irqflags = 0;							\
	cyg_scheduler_lock();				\
}

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)	\
{											\
	cyg_scheduler_unlock();				\
}

#define RTMP_INT_LOCK(__lock, __irqflags)	\
{											\
    HAL_DISABLE_INTERRUPTS(__irqflags);     \
}

#define RTMP_INT_UNLOCK(__lock, __irqflag)	\
{											\
    HAL_RESTORE_INTERRUPTS(__irqflag);      \
}

#define RTMP_SEM_LOCK 			RTMP_Spinlock_LOCK
#define RTMP_SEM_UNLOCK 		RTMP_Spinlock_UNLOCK
#define NdisAcquireSpinLock		RTMP_Spinlock_LOCK
#define NdisReleaseSpinLock		RTMP_Spinlock_UNLOCK
#define RTMP_SPIN_LOCK			RTMP_Spinlock_LOCK
#define RTMP_SPIN_UNLOCK		RTMP_Spinlock_UNLOCK

typedef int		wait_queue_head_t;

struct completion
{
	unsigned int done;
	cyg_sem_t    wait;
};

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

#define RTMP_SEM_EVENT_INIT_LOCKED(_pSema, __pSemaList)	\
	cyg_semaphore_init(_pSema, 1);      \
    cyg_semaphore_wait(_pSema);
	 	
#define RTMP_SEM_EVENT_INIT(_pSema)			\
	cyg_semaphore_init(_pSema, 1);
	
#define RTMP_SEM_EVENT_WAIT(_pSema, _status)	\
	(_status) = ((~cyg_semaphore_wait(_pSema)) & 0x01);

#define RTMP_SEM_EVENT_UP(_pSema)			\
	cyg_semaphore_post(_pSema)

#define RTMP_OS_TASK_WAKE_UP(__pTask)						\
	RTMP_SEM_EVENT_UP(&(pTask)->taskSema);

#define RTMP_OS_TASK_GET(__pTask)							\
	(__pTask)

#define RTCMDUp					OS_RTCMDUp

/***********************************************************************************
 *	OS Memory Access related data structure and definitions
 ***********************************************************************************/
#define GFP_ATOMIC          (0)
#define GFP_DMA             (0)
#define GFP_KERNEL          (0)

#define kmalloc(_size, _flags)	malloc((_size), M_TEMP, M_NOWAIT)
#define kfree(_ptr)             free((_ptr), M_TEMP)

#define vmalloc(_size)          malloc((_size), M_TEMP, M_NOWAIT)
#define vfree(_ptr)             free((_ptr), M_TEMP)

#define MEM_ALLOC_FLAG          (GFP_ATOMIC) /*(GFP_DMA | GFP_ATOMIC) */
#define copy_to_user(x,y,z)     (memcpy(x,y,z) == NULL)
#define copy_from_user(x,y,z)   (memcpy(x,y,z) == NULL)

#define NdisMoveMemory(Destination, Source, Length) memmove(Destination, Source, Length)
#define NdisCopyMemory(Destination, Source, Length) memcpy(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)         memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)   memset(Destination, Fill, Length)
#define NdisCmpMemory(Destination, Source, Length)  memcmp(Destination, Source, Length)
#define NdisEqualMemory(Source1, Source2, Length)   (!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	(!memcmp(Source1, Source2, Length))
#define COPY_MAC_ADDR(Addr1, Addr2)                 memcpy((Addr1), (Addr2), MAC_ADDR_LEN)

#define MlmeAllocateMemory(_pAd, _ppVA)		os_alloc_mem(_pAd, _ppVA, MGMT_DMA_BUFFER_SIZE)
#define MlmeFreeMemory( _pVA)			os_free_mem( _pVA)

/***********************************************************************************
 *	OS task related data structure and definitions
 ***********************************************************************************/
/*EddyTODO */
typedef	int	THREAD_PID;
#define RTMP_OS_MGMT_TASK_FLAGS	    0
#define	THREAD_PID_INIT_VALUE	    -1
#define RTMP_GET_OS_PID(_x, _pid)   _x = _pid
#define	GET_PID_NUMBER(_v)	        (_v)
#define CHECK_PID_LEGALITY(_pid)	if ((_pid) >= 0)
#define KILL_THREAD_PID(_A, _B, _C)	kill_proc((_A), (_B), (_C))


typedef void (*PNETFUNC)(ULONG arg1);
typedef INT (*RTMP_OS_TASK_CALLBACK)(IN ULONG Context);

typedef enum _RTMP_NET_TASK_STATUS_
{
	RTMP_NET_TASK_UNKNOWN = 0,
	RTMP_NET_TASK_INITED = 1,
	RTMP_NET_TASK_RUNNING = 2,
	RTMP_NET_TASK_PENDING = 4,
	RTMP_NET_TASK_STOP = 8,
}RTMP_NET_TASK_STATUS;

typedef struct _RTMP_NET_TASK_STRUCT_
{
	PNETFUNC 				funcPtr;
	unsigned long           data;
	RTMP_NET_TASK_STATUS	taskStatus;
	char                    taskName[8];
}RTMP_NET_TASK_STRUCT, *PRTMP_NET_TASK_STRUCT;

#define RTMP_NET_TASK_INIT(_pNetTask, _pFuncPtr, _pData)	\
    do{														\
		(_pNetTask)->funcPtr = (_pFuncPtr);					\
		(_pNetTask)->data = (unsigned int)(_pData);			\
		(_pNetTask)->taskStatus = RTMP_NET_TASK_INITED;		\
	}while(0)

#define tasklet_hi_schedule(_pTask)					                        						\
	do{																		                        \
		cyg_bool_t _retVal = 0;											                        \
		POS_COOKIE pOSCookie = (POS_COOKIE) ((PRTMP_ADAPTER) ((_pTask)->data))->OS_Cookie;             \
        if (pOSCookie->nettask_handle != 0)                                                      \
    		_retVal = cyg_mbox_tryput(pOSCookie->nettask_handle, (void*) (_pTask));                    \
		else 																                        \
        	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("NetTask(%s) nettask_handle is null!\n", (char *)((_pTask)->taskName)));	\
		if (_retVal == 1)												                        \
			(_pTask)->taskStatus = RTMP_NET_TASK_PENDING;					                        \
		else																                        \
        	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("NetTask(%s) add failed!\n", (char *)((_pTask)->taskName)));	\
	}while(0)

#define RTMP_OS_TASKLET_SCHE(__pTasklet)							\
		tasklet_hi_schedule(__pTasklet)


/***********************************************************************************
 * Timer related definitions and data structures.
 **********************************************************************************/
#define HZ				100
#define OS_HZ			HZ

typedef void (*FUNCPTR)(void *);

struct RtmpEcosTimer
{
	cyg_handle_t	sys_clk;
	cyg_handle_t 	counter_hdl;
	cyg_handle_t	alarm_hdl;
	cyg_alarm	alarm_obj;
	PVOID		data;
};

typedef struct RtmpEcosTimer	NDIS_MINIPORT_TIMER;
typedef struct RtmpEcosTimer	RTMP_OS_TIMER;
typedef void (*TIMER_FUNCTION) (cyg_handle_t alarm, cyg_addrword_t data);

/* The unit of _time is ticks instead of ms. */
#define OS_WAIT(_time) {int value; tsleep((caddr_t)&value, PWAIT, "os_wait", (_time*HZ/1000));}

#define typecheck(type,x) \
({      type __dummy; \
        typeof(x) __dummy2; \
        (void)(&__dummy == &__dummy2); \
        1; \
})
#define time_after(a,b)         \
        (typecheck(unsigned long, a) && \
         typecheck(unsigned long, b) && \
         ((long)(b) - (long)(a) < 0))
#define time_before(a,b)        time_after(b,a)

#define RTMP_TIME_BEFORE(a,b)	time_before(b,a)
#define RTMP_TIME_AFTER(a,b) 	time_after(a, b)
#define mdelay(_us)		        RtmpusecDelay((_us)*1000)

#define ONE_TICK 1

static inline void NdisGetSystemUpTime(ULONG *time)
{
	*time = cyg_current_time();
}

/***********************************************************************************
 * Memory pool & Network packet Structure
 **********************************************************************************/
typedef struct _ECOS_DESC_BUFFER
{
    struct mbuf             *pMBuf;
    PNDIS_PHYSICAL_ADDRESS  PhysicalAddress;
} ECOS_DESC_BUFFER, *PECOS_DESC_BUFFER;

typedef struct _ECOS_PKT_BUFFER
{
	struct _ECOS_PKT_BUFFER *Next;  /* Don't move, for InsertTailQueueAC use */

	/* Internal information */
	INT             MemPoolType;        /* Record from which mempool */
    struct mbuf     *pHeaderMBuf;       /* Pointer to header mbuf */
    struct mbuf     *pDataMBuf;         /* Pointer to data mbuf */

	/* packet information */
	PNET_DEV        net_dev;
	UCHAR           cb[48];

	PUCHAR          pDataPtr;
	LONG            pktLen;
	LONG            ref_cnt;
} ECOS_PKT_BUFFER, *PECOS_PKT_BUFFER;

/***********************************************************************************
 *	OS specific cookie data structure binding to RTMP_ADAPTER
 ***********************************************************************************/
typedef INT (*pWscMsgCallBack)(PUCHAR pData, UINT32	dataLen);

struct os_cookie {
#ifdef RTMP_MAC_PCI
	struct pci_dev          *pci_dev;
	struct pci_dev          *parent_pci_dev;
	USHORT                  DeviceID;
	ra_dma_addr_t              pAd_pa;
#endif /* RTMP_MAC_PCI */

	RTMP_NET_TASK_STRUCT    rx_done_task;
	RTMP_NET_TASK_STRUCT    mgmt_dma_done_task;
	RTMP_NET_TASK_STRUCT    ac0_dma_done_task;
	RTMP_NET_TASK_STRUCT    ac1_dma_done_task;
	RTMP_NET_TASK_STRUCT    ac2_dma_done_task;
	RTMP_NET_TASK_STRUCT    ac3_dma_done_task;
	RTMP_NET_TASK_STRUCT    hcca_dma_done_task;
	RTMP_NET_TASK_STRUCT    tbtt_task;

#ifdef MT_MAC
	RTMP_NET_TASK_STRUCT	mt_mac_int_0_tasklet;
	RTMP_NET_TASK_STRUCT	mt_mac_int_1_tasklet;
	RTMP_NET_TASK_STRUCT	mt_mac_int_2_tasklet;
	RTMP_NET_TASK_STRUCT	mt_mac_int_3_tasklet;
	RTMP_NET_TASK_STRUCT	mt_mac_int_4_tasklet;
#endif /*MT_MAC */

#ifdef RTMP_MAC_PCI
	RTMP_NET_TASK_STRUCT    fifo_statistic_full_task;
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
	RTMP_NET_TASK_STRUCT    uapsd_eosp_sent_task;
#endif /* UAPSD_SUPPORT */

#ifdef DFS_SUPPORT
	RTMP_NET_TASK_STRUCT    pulse_radar_detect_task;
	RTMP_NET_TASK_STRUCT    width_radar_detect_task;
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	RTMP_NET_TASK_STRUCT    carrier_sense_task;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef NEW_DFS
	RTMP_NET_TASK_STRUCT    dfs_task;
#endif /* NEW_DFS */
#endif /* CONFIG_AP_SUPPORT */

	RTMP_OS_PID             apd_pid; /*802.1x daemon pid */
	unsigned long			apd_pid_nr;
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
	RT_SIGNAL_STRUC			RTSignal;
	RTMP_OS_PID             IappPid; /*IAPP daemon pid */
	unsigned long			IappPid_nr;
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef WAPI_SUPPORT
	RTMP_OS_PID             wapi_pid; /*wapi daemon pid */
	unsigned long			wapi_pid_nr;
#endif /* WAPI_SUPPORT */
	INT						ioctl_if_type;
	INT 					ioctl_if;

    /* message queue handle & box */
    cyg_handle_t            nettask_handle;
    cyg_mbox                nettask_mbox;    

#ifdef WSC_AP_SUPPORT
#ifdef WSC_INCLUDED
	pWscMsgCallBack WscMsgCallBack;
#endif /* WSC_INCLUDED */	
#endif /* WSC_AP_SUPPORT */   
};

typedef struct os_cookie	* POS_COOKIE;



/***********************************************************************************
 *	OS debugging and printing related definitions and data structure
 ***********************************************************************************/
#define printk      diag_printf
#define printf      diag_printf
#define sprintf     diag_sprintf
#define snprintf    diag_snprintf
#define MTWF_PRINT	printk

#ifdef DBG
#ifdef BRANCH_ADV    
extern UCHAR    DbgBuffer[2048];
int printUI(const char *fmt, ...);
#define _dbg_print	printUI
#else /* !BRANCH_ADV */
#define _dbg_print	printk
#endif /* BRANCH_ADV */
#endif

#undef  ASSERT
#ifdef VENDOR_FEATURE3_SUPPORT
#define ASSERT(x)
#else
#define ASSERT(x)                                   \
{                                                   \
    if (!(x))                                       \
    {                                               \
	printk("%s", __FILE__);   		                \
	printk(":%d assert " #x "failed\n", __LINE__);  \
    }                                               \
}
#endif /* VENDOR_FEATURE3_SUPPORT */


/***********************************************************************************
 * Device DMA Access related definitions and data structures.
 **********************************************************************************/
/* This defines the direction arg to the DMA mapping routines. */
#define PCI_DMA_BIDIRECTIONAL	0
#define PCI_DMA_TODEVICE	    1
#define PCI_DMA_FROMDEVICE	    2
#define PCI_DMA_NONE		    3

ra_dma_addr_t ecos_pci_map_single(void *handle, void *ptr, size_t size, int sd_idx, int direction);
#ifdef RTMP_MAC_PCI
#define PCI_MAP_SINGLE(_handle, _ptr, _size, _sd_idx, _dir) \
    ecos_pci_map_single(_handle, _ptr, _size, _sd_idx, _dir)

#define PCI_UNMAP_SINGLE(_handle, _ptr, _size, _dir) {}
#endif /* RTMP_MAC_PCI */

/*
 * Input: NDIS_PHYSICAL_ADDRESS  PhysicalAddress
 * Input: ULONG  Value
 */
#define RTMP_GetPhysicalAddressLow(PhysicalAddress)         (PhysicalAddress)
#define RTMP_GetPhysicalAddressHigh(PhysicalAddress)        (0)
#define RTMP_SetPhysicalAddressLow(PhysicalAddress, Value)  PhysicalAddress = Value;
#define RTMP_SetPhysicalAddressHigh(PhysicalAddress, Value)


/***********************************************************************************
 * Device Register I/O Access related definitions and data structures.
 **********************************************************************************/
#ifdef RTMP_MAC_PCI
#define RTMP_IO_READ32(_A, _R, _pV)								    \
do{																    \
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
		(*_pV = HAL_REG32(((_A)->PciHif.CSRBaseAddress + (_R))));			\
    }                                                               \
    else															\
		*_pV = 0;													\
}while(0)

#define RTMP_IO_FORCE_READ32(_A, _R, _pV)							\
{																	\
	(*_pV = HAL_REG32(((_A)->PciHif.CSRBaseAddress + (_R))));			    \
}

#define RTMP_IO_READ8(_A, _R, _pV)								    \
{																    \
	(*_pV = HAL_REG8(((_A)->PciHif.CSRBaseAddress + (_R))));				\
}

#ifndef VENDOR_FEATURE3_SUPPORT
#define RTMP_IO_WRITE32(_A, _R, _V)									\
do{																	\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
	HAL_REG32(((_A)->PciHif.CSRBaseAddress + (_R))) = (_V);		        \
    }                                                               \
}while(0)
#endif /* VENDOR_FEATURE3_SUPPORT */

#define RTMP_IO_FORCE_WRITE32(_A, _R, _V)							\
{																	\
	HAL_REG32(((_A)->PciHif.CSRBaseAddress + (_R))) = (_V);		        \
}


#if defined(RALINK_2880) || defined(RALINK_3052) || defined(RALINK_2883) || defined(RTMP_RBUS_SUPPORT)
#define RTMP_IO_WRITE8(_A, _R, _V)                                  \
do{                                                                   \
	ULONG Val;                                                      \
	UCHAR _i;                                                       \
	_i = ((_R) & 0x3);                                              \
	Val = HAL_REG32(((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));          \
	Val = Val & (~(0x000000ff << ((_i)*8)));                        \
	Val = Val | ((ULONG)(_V) << ((_i)*8));                          \
	HAL_REG32(((_A)->PciHif.CSRBaseAddress + ((_R) - _i))) = (Val);	    \
}while(0)
#else
#define RTMP_IO_WRITE8(_A, _R, _V)							        \
{															        \
	HAL_REG8(((_A)->PciHif.CSRBaseAddress + (_R))) = (_V);	                \
}
#endif /* defined(RT2880) || defined(RT3052) || defined(RT2883)  */

#define RTMP_IO_WRITE16(_A, _R, _V)							        \
{															        \
	HAL_REG16(((_A)->PciHif.CSRBaseAddress + (_R))) = (_V);                \
}
#endif /* RTMP_MAC_PCI */

#define RTMP_SYS_IO_READ32(_R, _pV)		\
{										\
	(*_pV = HAL_REG32(_R));				\
}

#define RTMP_SYS_IO_WRITE32(_R, _V)		\
{										\
	HAL_REG32((_R)) = (_V);			\
}

/*EddyTODO */
#define EFAULT  -1
#define SIOCIWFIRSTPRIV 0x00


/***********************************************************************************
 *	Network Related data structure and marco definitions
 ***********************************************************************************/
typedef struct _RTMP_VX_INIT_STRING_{
	int phyNum;
	int devNum;
	int infType;
	int irqNum;
	int devType;
	int privSize;
	unsigned int csrAddr;
	unsigned char devName[IFNAMSIZ];
}RTMP_VX_INIT_STRING;

#define PKTSRC_NDIS             0x7f
#define PKTSRC_DRIVER           0x0f

#define RTMP_OS_NETDEV_STATE_RUNNING(_pNetDev)	((_pNetDev)->sc_arpcom.ac_if.if_flags & IFF_UP)
#define RTMP_OS_NETDEV_GET_PRIV(_pNetDev)		((_pNetDev)->driver_private)
#define RTMP_OS_NETDEV_SET_PRIV(_pNetDev, _pPriv)	((_pNetDev)->driver_private = (_pPriv))
#define RTMP_OS_NETDEV_GET_DEVNAME(_pNetDev)	((_pNetDev)->dev_name)
#define RTMP_OS_NETDEV_GET_PHYADDR(_pNetDev)	((_pNetDev)->sc_arpcom.ac_enaddr)
#define RTMP_OS_NETDEV_PHYADDR_LEN(_pNetDev)	(MAC_ADDR_LEN)

#define RTMP_OS_NETDEV_START_QUEUE(_pNetDev)	do{}while(0)
#define RTMP_OS_NETDEV_STOP_QUEUE(_pNetDev)		do{}while(0)
#define RTMP_OS_NETDEV_WAKE_QUEUE(_pNetDev)		do{}while(0)
#define RTMP_OS_NETDEV_CARRIER_OFF(_pNetDev)	do{}while(0)


#define QUEUE_ENTRY_TO_PACKET(pEntry)   (PNDIS_PACKET)(pEntry)

#define PACKET_TO_QUEUE_ENTRY(pPacket)  (PQUEUE_ENTRY)(pPacket)


#define GET_SG_LIST_FROM_PACKET(_p, _sc)    rt_get_sg_list_from_packet(_p, _sc)

#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)    \
    RTMPFreeNdisPacket(_pAd, _pPacket);

#define RELEASE_NDIS_PACKET_IRQ(_pAd, _pPacket, _Status)    \
    RTMPFreeNdisPacket(_pAd, _pPacket);

/*
 * packet helper
 * 	- convert internal rt packet to os packet or
 *             os packet to rt packet
 */
#define RTPKT_TO_OSPKT(_p)		((PECOS_PKT_BUFFER)(_p))
#define OSPKT_TO_RTPKT(_p)		((PNDIS_PACKET)(_p))

#define GET_OS_PKT_DATAPTR(_pkt)    (RTPKT_TO_OSPKT(_pkt)->pDataPtr)
#define SET_OS_PKT_DATAPTR(_pkt, _dataPtr)	\
		(RTPKT_TO_OSPKT(_pkt)->pDataPtr) = (_dataPtr)
		
#define GET_OS_PKT_LEN(_pkt)        (RTPKT_TO_OSPKT(_pkt)->pktLen)
#define SET_OS_PKT_LEN(_pkt, _len)	\
		(RTPKT_TO_OSPKT(_pkt)->pktLen) = (_len)
		
#define GET_OS_PKT_DATATAIL(_pkt) \
		((RTPKT_TO_OSPKT(_pkt)->pDataPtr) + (RTPKT_TO_OSPKT(_pkt)->pktLen))
#define SET_OS_PKT_DATATAIL(_pkt, _start, _len)	do {}while(0)

#define GET_OS_PKT_NETDEV(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->net_dev)
#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
		(RTPKT_TO_OSPKT(_pkt)->net_dev) = (PNET_DEV)(_pNetDev)
		
#define GET_OS_PKT_TYPE(_pkt)   (RTPKT_TO_OSPKT(_pkt))

#define GET_OS_PKT_NEXT(_pkt)   do {}while(0)

#define OS_PKT_CLONE(_pAd, _pkt, _src, _flag)		\
	_src = skb_clone(RTPKT_TO_OSPKT(_pkt), _flag);

#define OS_PKT_CLONED(_pkt)		(((RTPKT_TO_OSPKT(_pkt))->ref_cnt) > 1)
#define OS_PKT_COPY(_pkt)		skb_copy(RTPKT_TO_OSPKT(_pkt), GFP_ATOMIC)

#define OS_PKT_TAIL_ADJUST(_pkt, _removedTagLen)								\
	SET_OS_PKT_DATATAIL(_pkt, GET_OS_PKT_DATATAIL(_pkt), (-_removedTagLen));	\
	GET_OS_PKT_LEN(_pkt) -= _removedTagLen;

#define OS_PKT_HEAD_BUF_EXTEND(_pkt, _offset)								\
	skb_push(RTPKT_TO_OSPKT(_pkt), _offset)

#define OS_PKT_TAIL_BUF_EXTEND(_pkt, _Len)									\
	skb_put(RTPKT_TO_OSPKT(_pkt), _Len)

#define OS_PKT_RESERVE(_pkt, _Len)											\
	skb_reserve(RTPKT_TO_OSPKT(_pkt), _Len)

#define GET_OS_PKT_TOTAL_LEN(_pkt)  (RTPKT_TO_OSPKT(_pkt)->pktLen)

#define OS_NTOHS(_Val)  (ntohs(_Val))
#define OS_HTONS(_Val)  (htons(_Val))
#define OS_NTOHL(_Val)  (ntohl(_Val))
#define OS_HTONL(_Val)  (htonl(_Val))

#define CB_OFF  10

#define GET_OS_PKT_CB(_p)		(RTPKT_TO_OSPKT(_p)->cb)
#define PACKET_CB(_p, _offset)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF + (_offset)]))

#define GET_PAD_FROM_NET_DEV(_pAd, _net_dev)	(_pAd) = (PRTMP_ADAPTER)(_net_dev)->driver_private;



/***********************************************************************************
 *	Other function prototypes definitions
 ***********************************************************************************/
/*void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time); */
int rt28xx_packet_xmit(void *pPacket);


#define RTMP_OS_PKT_INIT(__pRxPacket, __pNetDev, __pData, __DataSize)		\
{																			\
	PNDIS_PACKET __pRxPkt;													\
	__pRxPkt = RTPKT_TO_OSPKT(__pRxPacket);									\
	SET_OS_PKT_NETDEV(__pRxPkt, __pNetDev);									\
	SET_OS_PKT_DATAPTR(__pRxPkt, __pData);									\
	SET_OS_PKT_LEN(__pRxPkt, __DataSize);									\
	SET_OS_PKT_DATATAIL(__pRxPkt, __pData, __DataSize);						\
}

#define RTMP_OS_TASK							OS_TASK

#define RTMP_OS_TASK_DATA_GET(__pTask)						\
	((__pTask)->priv)

#define RTMP_OS_TASK_IS_KILLED(__pTask)						\
	((__pTask)->task_killed)

#define get_unaligned32							get_unaligned
#define get_unalignedlong						get_unaligned

typedef VOID									* PPCI_DEV;

#define KERN_EMERG
#define ate_print printf
#ifdef RTMP_MAC_PCI
#define EEPROM_SIZE								0x200
#ifdef CONFIG_AP_SUPPORT
#define EEPROM_BIN_FILE_NAME  "/tgtsvr/pci/e2p_ap.bin"
#endif /* CONFIG_AP_SUPPORT */
#endif /* RTMP_MAC_PCI */

#define MAX_PACKETS_IN_QUEUE				(128) /*(512)    // to pass WMM A5-WPAPSK */

INT rt28xx_ioctl(
	IN	PNET_DEV	endDev, 
	IN	caddr_t		data,
	IN	int			cmd);

long simple_strtol(const char *cp,char **endp,unsigned int base);
char *strsep(register char **stringp, register const char *delim);
VOID rt2860_interrupt(void *dev_instance);

#ifdef RTMP_MAC_PCI
#define TX_RING_SIZE            32 /*64 */
#define MGMT_RING_SIZE          128
#define RX_RING_SIZE            32 /*64 */
#define MAX_TX_PROCESS          TX_RING_SIZE /*8 */
#define MAX_DMA_DONE_PROCESS    TX_RING_SIZE
#define MAX_TX_DONE_PROCESS     TX_RING_SIZE /*8 */
#define LOCAL_TXBUF_SIZE        2
#endif /* RTMP_MAC_PCI */

typedef VOID					NET_DEV_STATS;

#define APCLI_IF_UP_CHECK(pAd, ifidx) ((pAd)->ApCfg.ApCliTab[(ifidx)].dev->state & ETH_DRV_STATE_ACTIVE)

#define RTMP_THREAD_PID_KILL(__PID)		KILL_THREAD_PID(__PID, SIGTERM, 1)

typedef int atomic_t;
#define RTMP_OS_ATOMIC							atomic_t
#define RTMP_OS_ATMOIC_INIT(__pAtomic, __pAtomicList)

#define OS_PKT_HEAD_BUF_EXTEND(_pkt, _offset)								\
	skb_push(RTPKT_TO_OSPKT(_pkt), _offset)

#include "os/rt_os.h"

#define TRACE_MCU_CMD_INFO


#ifdef FW_DUMP_SUPPORT	
#define RTMP_OS_FWDUMP_PROCINIT(ad)
#define RTMP_OS_FWDUMP_PROCCREATE(ad)
#define RTMP_OS_FWDUMP_PROCREMOVE(ad)
#define RTMP_OS_FWDUMP_SETPATH(ad, ar)
#endif

#endif /* __RT_ECOS_H__ */
