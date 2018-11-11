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
	rt_vxworks.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RT_VXWORKS_H__
#define __RT_VXWORKS_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>	/* for isxdigit() */
#include <sysLib.h>
#include <intLib.h>
#include <iv.h>
#include <ioLib.h>
#include <tickLib.h>
#include <logLib.h>	/* for logMsg() */
#include <lstLib.h>
#include <end.h>
#include <endLib.h>
#include <muxLib.h>
#include <usb2/usbHst.h>
#include <usb2/usbd.h>
#include <usb/usb.h>
#include <usb/ossLib.h>
#ifdef VXWORKS_6X
#include <semLibCommon.h>
#include <taskLibCommon.h>
#endif /* VXWORKS_6X */
#ifdef VXWORKS_5X
#include <semLib.h>
#include <taskLib.h>
#endif /* VXWORKS_5X */
#include <wdLib.h>		/* for wdStart(), need to revise that */
#include <netLib.h>
#include <net/if.h>
#ifdef VXWORKS_6X
#include <net/if_ll.h>
#endif /* VXWORKS_6X */
#include <netinet/in.h>

#ifdef VXWORKS_PCD10
#include <netinet/if_ether.h>	/* For vxWorks 5.5 with PCD V1.0 */
#include <private/stdioP.h>
#else
#include <net/ethernet.h>		/* For vxWorks 5.5.1 with PCD V2.0 and higher */
#endif /* VXWORKS_PCD10 */

#include <m2Lib.h>
#include <signal.h>
#ifdef DBG
#include <logLib.h>
#include <fioLib.h>
#endif
#include <time.h>
#include <cacheLib.h>

#include <ralink.h>


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


/***********************************************************************************
 *	Profile related sections
 ***********************************************************************************/
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
#define AP_PROFILE_PATH			"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define AP_RTMP_FIRMWARE_FILE_NAME "/etc/Wireless/RT2860AP/RT2860AP.bin"
#define AP_DRIVER_VERSION			"1.1.0.0"
#ifdef MULTIPLE_CARD_SUPPORT
#define CARD_INFO_PATH			"/etc/Wireless/RT2860AP/RT2860APCard.dat"
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RTMP_MAC_PCI */


#endif /* CONFIG_AP_SUPPORT */




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
typedef unsigned int		ra_dma_addr_t;
typedef struct end_object	* PNET_DEV;
typedef void				* PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET		* PPNDIS_PACKET;
typedef	ra_dma_addr_t			NDIS_PHYSICAL_ADDRESS;
typedef	ra_dma_addr_t			* PNDIS_PHYSICAL_ADDRESS;
typedef void				* NDIS_HANDLE;
typedef char 				* PNDIS_BUFFER;

typedef struct ifreq		NET_IOCTL;
typedef struct ifreq		* PNET_IOCTL;

typedef int 				RTMP_OS_PID;
typedef SEM_ID				RTMP_OS_SEM;

#define in_interrupt() intContext()
#define in_atomic()	intContext()

#define RT_MOD_DEC_USE_COUNT()	do{}while(0)
#define RT_MOD_INC_USE_COUNT()	do{}while(0)

#define RTMP_INC_REF(_A)		0
#define RTMP_DEC_REF(_A)		0
#define RTMP_GET_REF(_A)		0

#define RT_MOD_HNAT_DEREG(_net_dev) do{}while(0)
#define RT_MOD_HNAT_REG(_net_dev) do{}while(0)

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

struct iwreq {
	union   iwreq_data      u;
};

/* Maximum size of returned data, sync with wireless.h in Linux */
#define IW_SCAN_MAX_DATA        4096    /* In bytes */

#define RTMP_OS_MAX_SCAN_DATA_GET()		IW_SCAN_MAX_DATA


/* TODO: for ifreq structure, we need to check that if there are any difference between Linux and VxWorks */


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

#define MAC_ADDR_LEN	6
#ifndef END_SPEED
#define END_SPEED       10000000         /* 10Mbs */
#endif

#ifdef VXWORKS_6X
#define ETH_P_IPV6	NET_ETH_P_IPV6
#define ETH_P_IP		NET_ETH_P_IP
#endif /* VXWORKS_6X */

#ifdef VXWORKS_5X
#define ETH_P_IPV6	0x86DD
#define ETH_P_IP		0x0800
#endif /* VXWORKS_5X */

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


/* TODO: For VxWorks the size is 256. Shall we cahnge the value as 256 for all OS????? */
#define MAX_PACKETS_IN_QUEUE				(256) /*(512)    // to pass WMM A5-WPAPSK */


/***********************************************************************************
 *	Ralink Specific network related constant definitions
 ***********************************************************************************/
#define RT_DEV_PRIV_FLAGS_GET(__pDev)									\
	((PNET_DEV)pDev)->priv_flags

#define RT_DEV_PRIV_FLAGS_SET(__pDev, __PrivFlags)						\
	((PNET_DEV)pDev)->priv_flags = __PrivFlags


typedef enum{
	RTMP_NETPOOL_TX = 0,
	RTMP_NETPOOL_RX = 1,
	RTMP_NETPOOL_CNT = 2,
}RTMP_VX_NETPOOL_TYPE;


/***********************************************************************************
 *	OS signaling related constant definitions
 ***********************************************************************************/
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
typedef struct _RT_SIGNAL_STRUC {
	unsigned short					Sequence;
	unsigned char					MacAddr[MAC_ADDR_LEN];
	unsigned char					CurrAPAddr[MAC_ADDR_LEN];
	unsigned char					Sig;
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
typedef int RTMP_OS_FD;

typedef struct _RTMP_OS_FS_INFO_
{
	int	fsinfo; /* Dummy structure, for cross-platform compatibility */
}RTMP_OS_FS_INFO;

#define IS_FILE_OPEN_ERR(_fd) 	((_fd) < 0)


/***********************************************************************************
 *	OS semaphore related data structure and definitions
 ***********************************************************************************/
struct os_lock  {
	SEM_ID		lock;
	int			flags;
};

typedef struct os_lock		NDIS_SPIN_LOCK;

/* */
/*  spin_lock enhanced for Nested spin lock */
/* */
#define NdisAllocateSpinLock(__pReserve, __lock)      \
	do{                                       \
		(__lock)->lock = semMCreate(SEM_Q_PRIORITY |SEM_DELETE_SAFE |SEM_INVERSION_SAFE);	\
		/*(__lock)->lock = semBCreate(SEM_Q_FIFO, SEM_FULL);*/	\
		if ((__lock)->lock == NULL)	\
			logMsg("%s():Create mutexlock failed!\n", __FUNCTION__, 0,0,0,0,0);	\
	}while(0)

#define NdisFreeSpinLock(__lock)          \
	do{\
		if ((__lock)->lock)	\
		{					\
			semDelete((__lock)->lock);\
			(__lock)->lock = NULL;\
		}	\
	}while(0)

#define RTMP_SEM_LOCK(__lock)					\
{												\
	if ((__lock)->lock)								\
		semTake((__lock)->lock, WAIT_FOREVER);		\
}

#define RTMP_SEM_UNLOCK(__lock)					\
{												\
	if ((__lock)->lock)						\
		semGive((__lock)->lock);					\
}


/* sample, use semaphore lock to replace IRQ lock, 2007/11/15 */
#define RTMP_IRQ_LOCK(__lock, __irqflags)			\
{													\
	__irqflags = 0;									\
	if ((__lock)->lock)								\
		semTake((__lock)->lock, WAIT_FOREVER);		\
}

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)			\
{													\
	if ((__lock)->lock)							\
		semGive((__lock)->lock);						\
}

#define RTMP_INT_LOCK(__lock, __irqflags)			\
{													\
	__irqflags = intLock();							\
}

#define RTMP_INT_UNLOCK(__lock, __irqflag)			\
{													\
	intUnlock((__irqflag));							\
}

#define NdisAcquireSpinLock		RTMP_SEM_LOCK
#define NdisReleaseSpinLock		RTMP_SEM_UNLOCK
#define RTMP_SPIN_LOCK			RTMP_SEM_LOCK
#define RTMP_SPIN_UNLOCK		RTMP_SEM_UNLOCK

typedef int		wait_queue_head_t;

struct completion
{
	unsigned int done;
	SEM_ID 	    wait;
};

#ifdef VXWORKS_5X
#define SEM_INTERRUPTIBLE 0
#endif /* VXWORKS_5X */

#define RTMP_SEM_EVENT_INIT_LOCKED(_pSema, __pSemaList)	\
	do{																\
		SEM_ID	__pSemaID;											\
		__pSemaID = semCCreate((SEM_Q_FIFO | SEM_INTERRUPTIBLE), 0);	\
		if ((__pSemaID) == NULL)										\
			printf("RTMP_SEM_EVENT_INIT_LOCKED() failed!\n");			\
		else															\
			*(_pSema) = __pSemaID;										\
	}while(0)
 	
#define RTMP_SEM_EVENT_INIT(_pSema)									\
	do{																\
		(*(_pSema)) = semCCreate((SEM_Q_FIFO | SEM_INTERRUPTIBLE), 1);	\
		if ((*(_pSema)) == NULL)										\
			printf("RTMP_SEM_EVENT_INIT() failed!\n");				\
	}while(0)

#define RTMP_SEM_EVENT_DESTORY(_pSema)	\
	do{\
		semDelete(*(_pSema));\
	}while(0)

#define RTMP_SEM_EVENT_WAIT(_pSema, _status)	\
	((_status)  = semTake((*(_pSema)), WAIT_FOREVER))
#define RTMP_SEM_EVENT_UP(_pSema)				\
	semGive((*(_pSema)))

#define RTMP_OS_TASK_WAKE_UP(__pTask)						\
	RTMP_SEM_EVENT_UP(&(pTask)->taskSema);

#define RTMP_OS_TASK_GET(__pTask)							\
	(__pTask)

#define RTCMDUp					OS_RTCMDUp

#define init_waitqueue_head(_wait)		do{}while(0)
#define wait_event_interruptible_timeout(_wq, _condition, _timeout)		\
		RtmpMSleep((_timeout))


/***********************************************************************************
 *	OS Memory Access related data structure and definitions
 ***********************************************************************************/
#define MEM_ALLOC_FLAG      (0)
#define GFP_ATOMIC          (0)
#define GFP_DMA             (0)
#define GFP_KERNEL          (0)

#define kmalloc(_size, _flags)	malloc((_size))
#define kfree(_ptr)			free((_ptr))

#define vmalloc(_size)		malloc((_size))
#define vfree(_ptr)			free((_ptr))

/* in VxWorks 5.5 PCD 2.0, bcopy performance is better than memcpy */
#define NdisMoveMemory(Destination, Source, Length) bcopy(Source, Destination, Length)
#define NdisCopyMemory(Destination, Source, Length) bcopy(Source, Destination, Length)
#define NdisZeroMemory(Destination, Length)         memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)   memset(Destination, Fill, Length)
#define NdisCmpMemory(Destination, Source, Length)  memcmp(Destination, Source, Length)
#define NdisEqualMemory(Source1, Source2, Length)   (!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	(!memcmp(Source1, Source2, Length))	

#define MlmeAllocateMemory(_pAd, _ppVA)		os_alloc_mem(_pAd, _ppVA, MGMT_DMA_BUFFER_SIZE)
#define MlmeFreeMemory( _pVA)			os_free_mem( _pVA)

#define COPY_MAC_ADDR(Addr1, Addr2)         bcopy((Addr2), (Addr1), MAC_ADDR_LEN)

static inline int copy_to_user(void *pDst, void *pSrc, int len)
{
	memcpy(pDst,pSrc,len);
	return 0;
}

static inline int copy_from_user(void *pDst, void *pSrc, int len)
{
	memcpy(pDst, pSrc, len);
	return 0;
}

#define wmb()				write_mem_barrier()


/***********************************************************************************
 *	OS task related data structure and definitions
 ***********************************************************************************/
 #define RTMP_OS_TASK_CALLBACK 	(FUNCPTR)

#define RTMP_OS_MGMT_TASK_FLAGS	0

/* TODO: These definitions are for RT2870, not verify yet. */
typedef	int	THREAD_PID;
#define	THREAD_PID_INIT_VALUE	(-1)
#define	GET_PID(_v)	do{}while(0)
#define	GET_PID_NUMBER(_v)	do{}while(0)
#define CHECK_PID_LEGALITY(_pid)
#define KILL_THREAD_PID(_A, _B, _C)	kill(_A, _B)

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
	void 					*funcPtr;
	unsigned long				data;
	RTMP_NET_TASK_STATUS	taskStatus;
	char						taskName[8];
}RTMP_NET_TASK_STRUCT, *PRTMP_NET_TASK_STRUCT;

#define RTMP_NET_TASK_INIT(_pNetTask, _pFuncPtr, _pData)	\
	do{														\
		(_pNetTask)->funcPtr = (_pFuncPtr);					\
		(_pNetTask)->data = (unsigned int)(_pData);				\
		(_pNetTask)->taskStatus = RTMP_NET_TASK_INITED;		\
	}while(0)


#define tasklet_hi_schedule(_pTask)		\
	do{									\
		STATUS _retVal;					\
		_retVal = netJobAdd((FUNCPTR)((_pTask)->funcPtr), (int)((_pTask)->data), 0, 0, 0, 0);	\
		if (_retVal == OK)														\
			(_pTask)->taskStatus = RTMP_NET_TASK_PENDING;						\
		else																	\
			logMsg("NetTask(%s) add failed!\n", (int)((_pTask)->taskName),0,0,0,0,0);	\
	}while(0)

#define RTMP_OS_TASKLET_SCHE(__pTasklet)							\
		tasklet_hi_schedule(__pTasklet)


/***********************************************************************************
 * Timer related definitions and data structures.
 **********************************************************************************/
/* TODO: Need to check about this in VxWorks */
#ifndef HZ
#define HZ 100
#endif
#define OS_HZ			sysClkRateGet() /*HZ */

struct RtmpVxTimer
{
	WDOG_ID	id;
	FUNCPTR		func;
	PVOID		data;
};
typedef struct RtmpVxTimer	NDIS_MINIPORT_TIMER;
typedef struct RtmpVxTimer	RTMP_OS_TIMER;
typedef void (*TIMER_FUNCTION)(unsigned long);


#define OS_WAIT(_time) 			\
	do{\
		unsigned long _ticks = (_time * OS_HZ) /1000; \
		if (_ticks == 0)\
			_ticks++;\
		taskDelay(_ticks);\
	}while(0)

/* Roger, below is from web, is it correct?? */
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
#define mdelay(_us)		RtmpusecDelay((_us))

static inline void NdisGetSystemUpTime(ULONG *sys_time)
{
	*sys_time = tickGet();
}


/***********************************************************************************
 *	OS specific cookie data structure binding to RTMP_ADAPTER
 ***********************************************************************************/

struct os_cookie {
	int				unitNum;
	NET_POOL_ID		pNetPool[RTMP_NETPOOL_CNT];			/* netPool memory pointers */
	unsigned char		*pClMemPtr[RTMP_NETPOOL_CNT];		/* cluster memory pointer */
	unsigned char		*pMclMemPtr[RTMP_NETPOOL_CNT];		/* mBlk memory pointer */
	
	RTMP_NET_TASK_STRUCT rx_done_task;
	RTMP_NET_TASK_STRUCT mgmt_dma_done_task;
	RTMP_NET_TASK_STRUCT ac0_dma_done_task;
#ifdef CONFIG_ATE
	RTMP_NET_TASK_STRUCT ate_ac0_dma_done_task;
#endif /* CONFIG_ATE */
	RTMP_NET_TASK_STRUCT ac1_dma_done_task;
	RTMP_NET_TASK_STRUCT ac2_dma_done_task;
	RTMP_NET_TASK_STRUCT ac3_dma_done_task;
	RTMP_NET_TASK_STRUCT hcca_dma_done_task;
	RTMP_NET_TASK_STRUCT tbtt_task;
#ifdef RTMP_MAC_PCI
	RTMP_NET_TASK_STRUCT fifo_statistic_full_task;
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
	RTMP_NET_TASK_STRUCT uapsd_eosp_sent_task;
#endif /* UAPSD_SUPPORT */

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
	RTMP_NET_TASK_STRUCT pulse_radar_detect_task;
	RTMP_NET_TASK_STRUCT width_radar_detect_task;
#endif /* DFS_SOFTWARE_SUPPORT */
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	RTMP_NET_TASK_STRUCT carrier_sense_task;
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	unsigned long			apd_pid; /*802.1x daemon pid */
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
	RT_SIGNAL_STRUC			RTSignal;
	unsigned long			IappPid; /*IAPP daemon pid */
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef WAPI_SUPPORT
	unsigned long			wapi_pid; /*wapi daemon pid */
#endif /* WAPI_SUPPORT */
	INT						ioctl_if_type;
	INT 					ioctl_if;
#ifdef WSC_AP_SUPPORT
	int					(*WscMsgCallBack)();
#endif /* WSC_AP_SUPPORT */
};

typedef struct os_cookie	* POS_COOKIE;


/***********************************************************************************
 *	OS debugging and printing related definitions and data structure
 ***********************************************************************************/
#define printk		printf
#define MTWF_PRINT	logMsg

#undef  ASSERT
#define ASSERT(x)                                                               \
{                                                                               \
    if (!(x))                                                                   \
    {                                                                           \
        printf(__FILE__ ":%d assert " #x "failed\n", __LINE__);    \
    }                                                                           \
}

void hex_dump_mBlk(PUCHAR callerStr, M_BLK_ID pMblk, BOOLEAN bDumpPkt);


/*********************************************************************************************************
	The following code are not revised, temporary put it here.
  *********************************************************************************************************/
/*
 * Default macro definitions for BSP interface.
 * These macros can be redefined in a wrapper file, to generate
 * a new module with an optimized interface.
 */
#ifdef VXWORKS_6X
#ifndef SYS_INT_CONNECT
#define SYS_INT_CONNECT(_pVector, _pRoutine, _pAd, _pResult) \
	do{                                                                   				\
		(void) intConnect ((_pVector), (_pRoutine), (int)(_pAd));	\
		(_pResult) = OK;										\
	}while(0)
#endif /* SYS_INT_CONNECT */

#ifndef SYS_INT_DISCONNECT
#define SYS_INT_DISCONNECT(_pVector, _pRoutine, _pAd, _pResult)  		\
	do{																\
		(_pResult) = intDisconnect((_pVector), (_pRoutine), (int)(_pAd));	\
	}while(0)
#endif /* SYS_INT_DISCONNECT */
#endif /* VXWORKS_6X */

#ifdef VXWORKS_5X
#ifndef SYS_INT_CONNECT
#define SYS_INT_CONNECT(_pVector, _pRoutine, _pAd, _pResult)	\
	do{                                                                   				\
		(void) intConnect ((_pVector), (_pRoutine), (int)(_pAd));	\
		(_pResult) = OK;										\
	}while(0)
#endif /* SYS_INT_CONNECT */

#ifndef SYS_INT_DISCONNECT
#define SYS_INT_DISCONNECT(_pVector, _pRoutine, _pAd, _pResult)  \
	do{															\
		(_pResult) = OK;											\
	}while(0)
#endif /* SYS_INT_DISCONNECT */

#endif /* VXWORKS_5X */


/***********************************************************************************
 * Device DMA Access related definitions and data structures.
 **********************************************************************************/
/* This defines the direction arg to the DMA mapping routines. */
#define PCI_DMA_BIDIRECTIONAL	0
#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2
#define PCI_DMA_NONE		3

#define RTMP_PCI_DMA_TODEVICE		PCI_DMA_TODEVICE
#define RTMP_PCI_DMA_FROMDEVICE		PCI_DMA_FROMDEVICE

#ifdef RTMP_MAC_PCI
/* signature for performance enhancement */
#ifdef CACHE_DMA_SUPPORT
extern UCHAR CACHE_DMA_SIGNAUTURE[];
#endif /* CACHE_DMA_SUPPORT */

ra_dma_addr_t vxworks_pci_map_single(void *handle, void *ptr, size_t size, int sd_idx, int direction);

#define PCI_MAP_SINGLE(_handle, _ptr, _size, _sd_idx, _dir) \
	vxworks_pci_map_single(_handle, _ptr, _size, _sd_idx, _dir)
	
#define PCI_UNMAP_SINGLE(_handle, _ptr, _size, _dir)	\
	do{}while(0)

#endif /* RTMP_MAC_PCI */



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

/* in VxWorks, MMU is disabled so Physical address == virtual address */
#define VIRT_TO_PHYS(address) 	(address)

#define NdisMIndicateStatus(_w, _x, _y, _z)



/***********************************************************************************
 * Device Register I/O Access related definitions and data structures.
 **********************************************************************************/
#define readl(_x) (*((volatile UINT32 *)(_x)))
#define readb(_x) (*((volatile UINT8 *)(_x)))
#define writel(_v,  _x)							\
	do{												\
		*(volatile UINT *)(_x) = (_v);					\
	}while(0)

#define writew(_v, _x)			\
	do{						\
		*(UINT16 *)(_x) = (_v); \
		/*volatile  sysOutWord((_v), (_x))	;*/		\
	}while(0)
#define writeb(_v, _x)			\
	do{						\
		*(UINT8 *)(_x) = (_v); 	\
		/*sysOutByte((_v), (_x));*/		\
	}while(0)

#ifdef RTMP_MAC_PCI
#if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(IKANOS_VX_1X0)
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)									\
do{																	\
	(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
	(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
}while(0)

#define RTMP_IO_READ32(_A, _R, _pV)									\
do{																	\
    if ((_A)->bPCIclkOff == FALSE)                                      \
    {                                                                   \
	(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
	(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
    }                                                                   \
}while(0)

#define RTMP_IO_READ8(_A, _R, _pV)									\
{																	\
	(*_pV = readb((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
}

#define RTMP_IO_WRITE32(_A, _R, _V)									\
do{																	\
    if ((_A)->bPCIclkOff == FALSE)                                      \
    {                                                                   \
	UINT32	_Val;													\
	_Val = SWAP32(_V);												\
	writel(_Val, (void *)((_A)->PciHif.CSRBaseAddress + (_R)));			\
    }                                                                   \
}while(0)

#define RTMP_IO_WRITE8(_A, _R, _V)									\
{																	\
	writeb((_V), (PUCHAR)((_A)->PciHif.CSRBaseAddress + (_R)));			\
}

#define RTMP_IO_WRITE16(_A, _R, _V)									\
{																	\
	writew(SWAP16((_V)), (PUSHORT)((_A)->PciHif.CSRBaseAddress + (_R)));	\
}
#else
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)								\
{																\
	(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
}

#define RTMP_IO_READ32(_A, _R, _pV)								\
do{																\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
		(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
    }                                                               \
    else															\
		*_pV = 0;													\
}while(0)

#define RTMP_IO_READ8(_A, _R, _pV)								\
{																\
	(*_pV = readb((void *)((_A)->PciHif.CSRBaseAddress + (_R))));				\
}

#define RTMP_IO_WRITE32(_A, _R, _V)												\
do{																				\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
	writel((_V), (void *)((_A)->PciHif.CSRBaseAddress + (_R)));								\
    }                                                               \
}while(0)



#if defined(BRCM_6358) || defined(RALINK_2880) || defined(RALINK_3052)
#define RTMP_IO_WRITE8(_A, _R, _V)            \
do{                    \
	ULONG Val;                \
	UCHAR _i;                \
	_i = ((_R) & 0x3);             \
	Val = readl((void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));   \
	Val = Val & (~(0x000000ff << ((_i)*8)));         \
	Val = Val | ((ULONG)(_V) << ((_i)*8));         \
	writel((Val), (void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));    \
}while(0)
#else
#define RTMP_IO_WRITE8(_A, _R, _V)							\
{															\
	writeb((_V), (PUCHAR)((_A)->PciHif.CSRBaseAddress + (_R)));		\
}
#endif /* #if defined(BRCM_6358) || defined(RALINK_2880) */

#define RTMP_IO_WRITE16(_A, _R, _V)							\
{															\
	writew((_V), (PUSHORT)((_A)->PciHif.CSRBaseAddress + (_R)));	\
}
#endif /* #if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(IKANOS_VX_1X0) */
#endif /* RTMP_MAC_PCI */



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

#ifdef VXWORKS_5X
/*
	For Vxworks 5.x, didn't have feature to reserve space in packet buffer head, 
	but we need this space to record the cb[] values, so we reserve it myself.
*/
#define NETBUF_LEADING_SPACE 64
#endif /* VXWORKS_5X */
#ifdef VXWORKS_6X
/* NETBUF_LEADING_CLSPACE_DRV is "Leading pad space for END pool clusters" define in */
/* Network components -> Network core components -> netBufLib */
#endif /* VXWORKS_6X */

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define skb_put(skb, n) m_data_put(skb, n)
#define skb_push(skb, n) m_data_push(skb, n)
#define skb_reserve(skb,n)	RTPKT_TO_OSPKT(skb)->m_data += n;

#define PKTSRC_NDIS             0x7f
#define PKTSRC_DRIVER           0x0f

#define RTMP_OS_NETDEV_STATE_RUNNING(_pNetDev)	((_pNetDev)->flags & IFF_UP)

#define RTMP_OS_NETDEV_GET_PRIV(_pNetDev)		((_pNetDev)->devObject.pDevice)
#define RTMP_OS_NETDEV_SET_PRIV(_pNetDev, _pPriv)	((_pNetDev)->devObject.pDevice = (void *)(_pPriv))
#define RTMP_OS_NETDEV_GET_DEVNAME(_pNetDev)	((_pNetDev)->devObject.name)
#define RTMP_OS_NETDEV_GET_PHYADDR(_pNetDev)	((_pNetDev)->mib2Tbl.ifPhysAddress.phyAddress)
#define RTMP_OS_NETDEV_PHYADDR_LEN(_pNetDev)	((_pNetDev)->mib2Tbl.ifPhysAddress.addrLength)

/* Get & Set NETDEV interface hardware type */
#define RTMP_OS_NETDEV_GET_TYPE(_pNetDev)
#define RTMP_OS_NETDEV_SET_TYPE(_pNetDev, _type)

#define RTMP_OS_NETDEV_START_QUEUE(_pNetDev)	do{}while(0)
#define RTMP_OS_NETDEV_STOP_QUEUE(_pNetDev)		do{}while(0)
#define RTMP_OS_NETDEV_WAKE_QUEUE(_pNetDev)		do{}while(0)
#define RTMP_OS_NETDEV_CARRIER_OFF(_pNetDev)		do{}while(0)

#define QUEUE_ENTRY_TO_PACKET(pEntry) \
	(PNDIS_PACKET)(pEntry)

#define PACKET_TO_QUEUE_ENTRY(pPacket) \
	(PQUEUE_ENTRY)(pPacket)

#ifdef CONFIG_5VT_ENHANCE
#define BRIDGE_TAG 0x35564252    /* depends on 5VT define in br_input.c */
#endif

#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)                    \
{                                                                       \
        RTMPFreeNdisPacket(_pAd, _pPacket);                             \
}


#define RELEASE_NDIS_PACKET_IRQ(_pAd, _pPacket, _Status)                \
{                                                                       \
        RTMPFreeNdisPacket(_pAd, _pPacket);                             \
}


/*
 * packet helper
 * 	- convert internal rt packet to os packet or
 *             os packet to rt packet
 */
#define RTPKT_TO_OSPKT(_p)		((struct mBlk *)(_p))
#define OSPKT_TO_RTPKT(_p)		((PNDIS_PACKET)(_p))

#define GET_OS_PKT_DATAPTR(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->m_data)
#define SET_OS_PKT_DATAPTR(_pkt, _dataPtr)	\
		(RTPKT_TO_OSPKT(_pkt)->m_data) = (_dataPtr)
		
#define GET_OS_PKT_LEN(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->m_len)
#define SET_OS_PKT_LEN(_pkt, _len)	\
		(RTPKT_TO_OSPKT(_pkt)->m_len) = (_len)
		
#define GET_OS_PKT_DATATAIL(_pkt) \
		((RTPKT_TO_OSPKT(_pkt)->m_data) + (RTPKT_TO_OSPKT(_pkt)->m_len))
#define SET_OS_PKT_DATATAIL(_pkt, _start, _len)	do {}while(0)

#define GET_OS_PKT_HEAD(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->m_extBuf)

#ifdef VXWORKS_5X
#define GET_OS_PKT_NETDEV(_pkt) \
		((END_OBJ *)(RTPKT_TO_OSPKT(_pkt)->m_pkthdr.rcvif))
#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
		(RTPKT_TO_OSPKT(_pkt)->m_pkthdr.rcvif) = (struct ifnet *)(_pNetDev)
#endif /* VXWORKS_5X */

#ifdef VXWORKS_6X
#define GET_OS_PKT_NETDEV(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->m_pkthdr.rcvif)
#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
		(RTPKT_TO_OSPKT(_pkt)->m_pkthdr.rcvif) = (_pNetDev)
#endif /* VXWORKS_6X */

#define GET_OS_PKT_END(_pkt) \
		((RTPKT_TO_OSPKT(_pkt)->m_extBuf) + (RTPKT_TO_OSPKT(_pkt)->m_extSize))
		
#define GET_OS_PKT_TYPE(_pkt) \
		(RTPKT_TO_OSPKT(_pkt))

#define GET_OS_PKT_NEXT(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->m_nextpkt)

#define OS_PKT_CLONED(_pkt)		(((RTPKT_TO_OSPKT(_pkt))->pClBlk->clRefCnt) > 1)
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

#define GET_OS_PKT_HEADROOM(_pkt)	\
		((RTPKT_TO_OSPKT(_pkt)->m_data) - (RTPKT_TO_OSPKT(_pkt)->m_extBuf))

#define GET_OS_PKT_TOTAL_LEN(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->mBlkPktHdr.len)

#define OS_NTOHS(_Val) \
		(ntohs(_Val))
#define OS_HTONS(_Val) \
		(htons(_Val))
#define OS_NTOHL(_Val) \
		(ntohl(_Val))
#define OS_HTONL(_Val) \
		(htonl(_Val))

/* For vxWorks, if the packet was come from the OS, the reserved header space size */
/*   is 32 bytes. so we need to set the CB_OFF to 4. */
#define cb 					pClBlk->clNode.pClBuf
#define CB_MAX_OFFSET		32
#define CB_OFF  			4
		
#define GET_OS_PKT_CB(_p)		(RTPKT_TO_OSPKT(_p)->cb)
#define PACKET_CB(_p, _offset)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF + (_offset)]))


/***********************************************************************************
 *	Other function prototypes definitions
 ***********************************************************************************/
extern STATUS sysFlashSet(UCHAR* pData, UINT32 len, UINT32 addr);
extern void sysUSecDelay(UINT32 uSecs);

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);
STATUS rt28xx_packet_xmit(M_BLK_ID pMblk);
void * m_data_put(struct mBlk *skb, int n);
void * m_data_push(struct mBlk *skb, int n);

long simple_strtol(const char *cp,char **endp,unsigned int base);
char *strsep(register char **stringp, register const char *delim);

PNDIS_PACKET skb_copy(PNDIS_PACKET pSrcMblk, INT flags);
void FlashWrite(UCHAR *p, ULONG a, ULONG b);
void FlashRead(UCHAR *p, ULONG a, ULONG b);

long RtmpMSleep(IN ULONG mSec);

int RtmpENDDevInitStrParse(char *initString, RTMP_VX_INIT_STRING *pInitToken);

extern NET_FUNCS RtmpMAINEndFuncTable;
extern NET_FUNCS RtmpApCliEndFuncTable;
extern NET_FUNCS RtmpMeshEndFuncTable;
extern NET_FUNCS RtmpMBSSEndFuncTable;
extern NET_FUNCS RtmpWDSEndFuncTable;

void RtmpVxIoctlConfig(
	IN END_OBJ *pEndObj);
	
INT rt28xx_ioctl(
	IN	PNET_DEV	endDev, 
	IN	int			cmd, 
	IN	caddr_t		data);

#ifdef CONFIG_AP_SUPPORT
INT rt28xx_ap_ioctl(
	IN	END_OBJ		*endDev, 
	IN	INT			cmd, 
	IN	PUCHAR		data);
#endif /* CONFIG_AP_SUPPORT */


#ifdef RTMP_MAC_PCI
void rt2860_interrupt(void *dev_instance);

END_OBJ* RtmpVxPciEndLoad(
	IN RTMP_VX_INIT_STRING *initToken);
#endif /* RTMP_MAC_PCI */




END_OBJ *sysRtmpEndLoad(char *initString, void *arg);
STATUS sysRtmpEndUnLoad(END_OBJ *pEndDev);

NDIS_STATUS RtmpVxNetPoolFree(struct os_cookie *pObj);
NDIS_STATUS RtmpVxNetPoolInit(struct os_cookie *pObj);


STATUS RtmpVxShellCmdArrayAdd(void);

M_BLK_ID RtmpVxNetTupleGet(
	NET_POOL_ID		pNetPool,	/* pointer to the net pool */
	int			bufSize,	/* size of the buffer to get */
	int			canWait,	/* wait or don't wait */
	UCHAR		type,		/* type of data */
	BOOL		fitOption/* TRUE/FALSE */
);

#define NdisFreeMemory(a, b, c) 	free((a))
#define NdisCopyMemory(Destination, Source, Length) memcpy(Destination, Source, Length)


#ifdef CONFIG_ATE
/******************************************************************************

  	ATE related definitions

******************************************************************************/
#define KERN_EMERG
#define ate_print printf
#ifdef RTMP_MAC_PCI
#ifdef CONFIG_AP_SUPPORT
#define EEPROM_BIN_FILE_NAME  "/tgtsvr/pci/e2p_ap.bin"
#endif /* CONFIG_AP_SUPPORT */
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_ATE */

#define RTMP_OS_PKT_INIT(__pRxPacket, __pNetDev, __pData, __DataSize)		\
{																			\
	PNDIS_PACKET __pRxPkt;													\
	__pRxPkt = RTPKT_TO_OSPKT(__pRxPacket);									\
	SET_OS_PKT_NETDEV(__pRxPkt, __pNetDev);									\
	SET_OS_PKT_DATAPTR(__pRxPkt, __pData);									\
	SET_OS_PKT_LEN(__pRxPkt, __DataSize);									\
	SET_OS_PKT_DATATAIL(__pRxPkt, __pData, __DataSize);						\
}

#define RTMP_THREAD_PID_KILL(__PID)		KILL_THREAD_PID(__PID, SIGTERM, 1)

#define RTMP_OS_ATOMIC							atomic_t
#define RTMP_OS_ATMOIC_INIT(__pAtomic, __pAtomicList)

#define RTMP_OS_TASK							OS_TASK

#define RTMP_OS_TASK_DATA_GET(__pTask)						\
	((__pTask)->priv)

#define RTMP_OS_TASK_IS_KILLED(__pTask)						\
	((__pTask)->task_killed)

#define get_unaligned32							get_unaligned
#define get_unalignedlong						get_unaligned

typedef VOID									* PPCI_DEV;
typedef struct net_device_stats				NET_DEV_STATS;

#ifdef RTMP_MAC_PCI
#ifdef MEMORY_OPTIMIZATION
#define TX_RING_SIZE            64
#define MGMT_RING_SIZE          32
#define RX_RING_SIZE            64
#else
#define TX_RING_SIZE            64 /*64 */
#define MGMT_RING_SIZE          128
#define RX_RING_SIZE            128 /*64 */
#endif
#define MAX_TX_PROCESS          TX_RING_SIZE /*8 */
#define MAX_DMA_DONE_PROCESS    TX_RING_SIZE
#define MAX_TX_DONE_PROCESS     TX_RING_SIZE /*8 */
#define LOCAL_TXBUF_SIZE        2
#endif /* RTMP_MAC_PCI */

#define APCLI_IF_UP_CHECK(pAd, ifidx) (RtmpOSNetDevIsUp((pAd)->ApCfg.ApCliTab[(ifidx)].dev) == TRUE)

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

#endif /* __RT_VXWORKS_H__ */
