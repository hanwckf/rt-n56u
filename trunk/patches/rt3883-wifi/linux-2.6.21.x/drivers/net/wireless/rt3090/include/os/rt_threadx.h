/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_threadx.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RT_THREADX_H__
#define __RT_THREADX_H__

#include <string.h>                // memcpy(), strcpy()
#include "tx_api.h"                // The ThreadX API.

#ifdef SIGMATEL_SDK
#include "dc_type.h"
#include "dc_return_codes.h"	// SDK should include these two files.  They contain the basic definitions and types.
#include "dc_log_api.h"
#include "dc_rammgr_api.h"
#include "dc_net_drv_api.h"
#include "dc_time_api.h"
#include "errno.h"
#include "dc_am_malloc_api.h"		// for dc_am_global_setup()
#include "dc_serial_flash_api.h"   	// Serial Flash Driver
#include "dc_flash_api.h"       	// Flash API
#include "nwk_init/net_sy.h"    	// Sigmatel wifi system manager API
#include "dc_nvram_api.h"       	// NVRAM API
//#include "dc_int_api.h"


#endif // SIGMATEL_SDK //

#ifdef EBS_RTIP
#include "rtip.h"
#include "rtipapi.h"
#endif // EBS_RTIP //


#ifdef KODAK_DEMO
#include "dc_radef_common.h"
#include "radef.h"
#include "ra_ac.h"
#endif // KODAK_DEMO //


/***********************************************************************************
 *	Profile related sections
 ***********************************************************************************/
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
#define AP_PROFILE_PATH			"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define AP_RTMP_FIRMWARE_FILE_NAME "/etc/Wireless/RT2860AP/RT2860AP.bin"
#define AP_NIC_DEVICE_NAME			"RT2860AP"
#define AP_DRIVER_VERSION			"2.3.0.0"
#ifdef MULTIPLE_CARD_SUPPORT
#define CARD_INFO_PATH			"/etc/Wireless/RT2860AP/RT2860APCard.dat"
#endif // MULTIPLE_CARD_SUPPORT //
#endif // RTMP_MAC_PCI //

#endif // CONFIG_AP_SUPPORT //




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
 typedef unsigned int		dma_addr_t;

typedef struct _iface RTP_FAR *PNET_DEV;
typedef void 				* PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET	* PPNDIS_PACKET;
typedef unsigned long		NDIS_PHYSICAL_ADDRESS;
typedef	unsigned long		* PNDIS_PHYSICAL_ADDRESS;
typedef void				* NDIS_HANDLE;
typedef char 				* PNDIS_BUFFER;

typedef struct TX_THREAD_STRUCT 	*RTMP_OS_PID;
typedef struct TX_SEMAPHORE_STRUCT	RTMP_OS_SEM;

#define RT_MOD_DEC_USE_COUNT()	do{}while(0)
#define RT_MOD_INC_USE_COUNT()	do{}while(0)

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

// Define Linux ioctl relative structure, keep only necessary things
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


// TODO: for ifreq structure, we need to check that if there are any difference between Linux and VxWorks


#ifdef EBS_RTIP
/*
	For threadX, we don't have a pointer in OS Net Device structure to hook our private device
	control data structure(i.e., the "RTMP_ADAPTER" data structure), so, we delcare it as a gobal
	paramters so that driver can easily access it.
*/
extern UCHAR *RT_WIFI_DevCtrl;
#endif // EBS_RTIP //


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


/********************** MACROS FOR HANDLING ENDIANESS *************************/
#ifdef RT_BIG_ENDIAN
/*******************************************************************************
 * MACRO NAME    : OS_UINT32_CPU_TO_LE
 * DESCRIPTION   : Macro to convert an 32 bit integer from CPU format to the
 *                 little endian format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT32_CPU_TO_LE(DATA)                                       \
        (((DATA & 0xFF) << 24) |                                        \
                (((DATA >> 8) & 0xFF) << 16) |                          \
                (((DATA >> 16) & 0xFF) << 8) |                          \
                ((DATA >> 24) & 0xFF))

/*******************************************************************************
 * MACRO NAME    : OS_UINT32_LE_TO_CPU
 * DESCRIPTION   : Macro to convert an 32 bit integer from little endian format
 *                 to the CPU format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT32_LE_TO_CPU(DATA)                                       \
        (((DATA & 0xFF) << 24) |                                        \
                (((DATA >> 8) & 0xFF) << 16) |                          \
                (((DATA >> 16) & 0xFF) << 8) |                          \
                ((DATA >> 24) & 0xFF))

/*******************************************************************************
 * MACRO NAME    : OS_UINT16_LE_TO_CPU
 * DESCRIPTION   : Macro to convert an 16 bit integer from little endian format
 *                 to the CPU format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT16_LE_TO_CPU(DATA)                                       \
        ((((DATA >> 8) & 0xFF) | ((DATA & 0xFF) << 8)))

/*******************************************************************************
 * MACRO NAME    : OS_UINT16_CPU_TO_LE
 * DESCRIPTION   : Macro to convert an 16 bit integer from CPU format to the
 *                 little endian format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT16_CPU_TO_LE(DATA)                                       \
        ((((DATA >> 8) & 0xFF) | ((DATA & 0xFF) << 8)))

#define OS_BUFFER_CPU_TO_LE(BUFFER, LENGTH)

#define OS_BUFFER_LE_TO_CPU(BUFFER, LENGTH)

#else

/*
 * Macros for handling the endianess on little endian
 * systems
 */

/*******************************************************************************
 * MACRO NAME    : OS_BUFFER_LE_TO_CPU
 * DESCRIPTION   : Macro to convert a buffer from little endian format to the
 *                 CPU format.
 * PARAMETERS    : BUFFER   IN  Pointer to the buffer location.
 *                 LENGTH   IN  Number of bytes to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_BUFFER_LE_TO_CPU(BUFFER, LENGTH)

/*******************************************************************************
 * MACRO NAME    : OS_BUFFER_CPU_TO_LE
 * DESCRIPTION   : Macro to convert a buffer from CPU format to the little
 *                 endian format .
 * PARAMETERS    : BUFFER   IN  Pointer to the buffer location.
 *                 LENGTH   IN  Number of bytes to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_BUFFER_CPU_TO_LE(BUFFER, LENGTH)

/*******************************************************************************
 * MACRO NAME    : OS_UINT16_LE_TO_CPU
 * DESCRIPTION   : Macro to convert an 16 bit integer from little endian format
 *                 to the CPU format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT16_LE_TO_CPU(DATA)                                       \
        DATA

/*******************************************************************************
 * MACRO NAME    : OS_UINT16_CPU_TO_LE
 * DESCRIPTION   : Macro to convert an 16 bit integer from CPU format to the
 *                 little endian format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT16_CPU_TO_LE(DATA)                                       \
        DATA

/*******************************************************************************
 * MACRO NAME    : OS_UINT32_LE_TO_CPU
 * DESCRIPTION   : Macro to convert an 32 bit integer from little endian format
 *                 to the CPU format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT32_LE_TO_CPU(DATA)                                       \
        DATA

/*******************************************************************************
 * MACRO NAME    : OS_UINT32_CPU_TO_LE
 * DESCRIPTION   : Macro to convert an 32 bit integer from CPU format to the
 *                 little endian format.
 * PARAMETERS    : DATA IN  Data to be converted.
 * RETURN TYPE   : None
 ******************************************************************************/
#define OS_UINT32_CPU_TO_LE(DATA)                                       \
        DATA

#endif /* RT_BIG_ENDIAN */

/***********************************************************************************
 *	Network related constant definitions
 ***********************************************************************************/
#ifndef IFNAMSIZ
/*
	For the definition of IFNAMESIZ, should refer to the device name exactlly the Network subsytem 
	of Target OS declared.
	For EBS_RTIP, the max length of device name is 22, which defined as "MAX_DEVICE_NAME_LEN" 
	and in "PDEVTABLE->device_name[MAX_DEVICE_NAME_LEN]"
*/
#define IFNAMSIZ 16
#endif

#define ETH_LENGTH_OF_ADDRESS	6

#define ETH_P_IPV6	0x86DD
#define ETH_P_IP	0x0800

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
#define NET_DEVICE_REAL_IDX_MASK		0x0f		// for each operation mode, maximum support 15 entities.




/***********************************************************************************
 *	OS signaling related constant definitions
 ***********************************************************************************/
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
typedef struct _RT_SIGNAL_STRUC {
	unsigned short					Sequence;
	unsigned char					MacAddr[ETH_LENGTH_OF_ADDRESS];
	unsigned char					CurrAPAddr[ETH_LENGTH_OF_ADDRESS];
	unsigned char					Sig;
} RT_SIGNAL_STRUC, *PRT_SIGNAL_STRUC;

// definition of signal
#define	SIG_NONE					0
#define SIG_ASSOCIATION				1
#define SIG_REASSOCIATION			2
#endif // IAPP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


/***********************************************************************************
 *	OS file operation related data structure definitions
 ***********************************************************************************/
 /*
	NOTE: For ThreadX, currently don't support file system. so the following definitions is useless.
 */
typedef int RTMP_OS_FD;

typedef struct _RTMP_OS_FS_INFO_
{
	int	fsinfo; /* Dummy structure, for cross-platform compatibility */
}RTMP_OS_FS_INFO;

#define IS_FILE_OPEN_ERR(_fd) 	((_fd) < 0)

#define O_RDONLY 0
#define O_WRONLY 0
#define O_CREAT	0
#define O_TRUNC 0

/***********************************************************************************
 *	OS semaphore related data structure and definitions
 ***********************************************************************************/
// TODO: We need to find a wrapper function for in_interrupt()
#define in_interrupt() (0)
#define in_atomic()	(0)

struct os_lock  {
	TX_MUTEX	lock;
	UINT		mutex_status;
};

typedef struct os_lock		NDIS_SPIN_LOCK;

/*
	For ThreadX, it use TX_MUTEX for inter-thread mutual exclusion for resource protection.
	
	There is a priority_inherit parameter used for tx_mutex_create to indicate if the mutex 
	support priority inherit or not. For RT_WIFI, all threads are created with the same thread 
	priority, so the priority_inherit is not necessary and disable for all mutex created by 
	RT_WIFI driver.
*/
#define NdisAllocateSpinLock(__lock)      \
	do{                                       \
		 (__lock)->mutex_status = tx_mutex_create(&((__lock)->lock), "", TX_NO_INHERIT);\
		if ((__lock)->mutex_status != TX_SUCCESS)	\
			dc_log_printf("%s(%d):Create mutexlock failed!\n", __FUNCTION__, (__lock)->mutex_status);\
	}while(0)


/*
	In ThreadX, after the mutex is deleted by following function, all threads suspended waiting 
	for the mutex are resumed and given  TX_DLELETD return status.
*/
#define NdisFreeSpinLock(__lock)          \
	do{\
		(__lock)->mutex_status = tx_mutex_delete(&((__lock)->lock));\
		if ((__lock)->mutex_status != TX_SUCCESS)\
			dc_log_printf("%s(%d): delMutex failed!\n", __FUNCTION__, (__lock)->mutex_status);\
	}while(0)

/*
	Some characteristics in ThreadX
		(1). A mutex can multiple get by the same thread if this thread alreay owns this mutex
		
		(2). the wait_option parameter in tx_mutex_get defines how the service behaves if the 
			mutex is already own by another thread.
				TX_NO_WAIT 				->immediate return, only valid option in initialization
				TX_WAIT_FOREVER		->calling thread suspend indefinitely until the mutex is available
				timeout_value(1~0xfffffffe)	->timer-ticks	to stay suspended while waiting the mutex.
		
		(3). The mutex is made available if the ownership count is zero(via function "tx_mutex_put()")
		
		(4). The MUTEX CANNOT be used in ISR!!!

		(5). tx_mutex_get() is preempt-able and can be called in:
						initialization, threads, and timers
			tx_mutex_put() is preempt-able and can be called in:
						initialization and threads.
			
*/
#define RTMP_SEM_LOCK(__lock)					\
do{												\
	(__lock)->mutex_status = tx_mutex_get(&((__lock)->lock), TX_WAIT_FOREVER);		\
	if ((__lock)->mutex_status != TX_SUCCESS)\
		dc_log_printf("%s(%d): semLock failed!\n", __FUNCTION__, (__lock)->mutex_status);\
}while(0)


#define RTMP_SEM_UNLOCK(__lock)					\
do{												\
	(__lock)->mutex_status = tx_mutex_put(&((__lock)->lock));	\
	if ((__lock)->mutex_status != TX_SUCCESS)\
		dc_log_printf("%s(%d): semUnLock failed!\n", __FUNCTION__, (__lock)->mutex_status);\
}while(0)


// sample, use semaphore lock to replace IRQ lock, 2007/11/15
#define RTMP_IRQ_LOCK(__lock, __irqflags)\
do{					\
	__irqflags = 0;			\
	(__lock)->mutex_status = tx_mutex_get(&((__lock)->lock), TX_WAIT_FOREVER);		\
	if ((__lock)->mutex_status != TX_SUCCESS)\
		dc_log_printf("%s(%d): IRQLock failed!\n", __FUNCTION__, (__lock)->mutex_status);\
	else\
		pAd->irq_disabled |= 1; \
}while(0)

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)			\
do{	\
	pAd->irq_disabled &= 0; \
	(__lock)->mutex_status = tx_mutex_put(&((__lock)->lock));\
	if ((__lock)->mutex_status != TX_SUCCESS)\
		dc_log_printf("%s(%d): IRQUnLock failed!\n", __FUNCTION__, (__lock)->mutex_status);\
}while(0)


/*
	For ThreadX, the hardware interrupt enable/disable are controled by the following procedures.
	NOTE: 
		(1). Even one thread disable the hardware interrupt, the thread may suspend too. 
		(2). If the thread in suspend stage, the interrupt will enable again. Until the thread resume 
			again, the interrupt will go back to diable mode.
*/
#define RTMP_INT_LOCK(__lock, __irqflags)			\
do{													\
	__irqflags = tx_interrupt_control(TX_INT_DISABLE);	\
}while(0)


#define RTMP_INT_UNLOCK(__lock, __irqflag)			\
do{													\
	tx_interrupt_control((UINT)__irqflag);	\
}while(0)


#define NdisAcquireSpinLock		RTMP_SEM_LOCK
#define NdisReleaseSpinLock		RTMP_SEM_UNLOCK

typedef int atomic_t;

atomic_t atomic_read(atomic_t *v);
void atomic_inc(atomic_t *v);
void atomic_dec(atomic_t *v);
void atomic_set(atomic_t *v, int i);

#define InterlockedIncrement 	 	atomic_inc
#define NdisInterlockedIncrement 	atomic_inc
#define InterlockedDecrement		atomic_dec
#define NdisInterlockedDecrement 	atomic_dec
#define InterlockedExchange			atomic_set

typedef int	wait_queue_head_t;

#define init_waitqueue_head(_wait)		do{}while(0)
#define wait_event_interruptible_timeout(_wq, _condition, _timeout)		\
		RtmpMSleep((_timeout))


// TODO: Shiang, maybe we can implement this data structure with EVENT notification mechanism of TheadX
struct completion
{
	unsigned int	done;
	TX_MUTEX	wait;
};


/*
	In ThreadX, we use the TX_SEMAPHORE(counting semaphore) to implement this semaphore/event 
	trigger/wakeup mechanism
	Creation of semaphore:
		tx_semaphore_create(TX_SEMAPHORE *pSema, char *name_ptr, ULONG initial_count)
			=>pSema    : Pointer to a semaphore contorl block
			     name_ptr: Pointer to the name of the semaphore
			     init_cnt   : Specifies the initial count for this semaphore. Range from 0x0~0xffffffff
			Only available call this function in Initialization and Threads
			
		tx_semaphore_delete(TX_SEMAPHORE *pSema)
			=>pSema    : Pointer to a previously created semahore
			Only available call this function in Threads

		tx_semaphore_get(TX_SEMAPHORE *pSema, ULONG wait_opt)
			=>pSema    : Pointer to previously created counting semaphore
			     wait_opt  :Define how the service behaves if there are no instances of the semaphore 
			     			available. As a result, the specified semaphore's count is decreased by one.
			     			TX_NO_WAIT			-> only valid option for 	non-thread services.
			     			TX_WAIT_FOREVER
			     			timeout_value	(0x1~0xfffffffe ticks)
		
*/
#define RTMP_SEM_EVENT_INIT_LOCKED(_pSema)								\
	do{																\
		UINT __sem_status;													\
		__sem_status = tx_semaphore_create(_pSema, "", 0);	\
		if (__sem_status != TX_SUCCESS)										\
			dc_log_printf("%s(%d):semaCreateLocked failed!\n", __FUNCTION__, __sem_status);\
	}while(0)
 	
#define RTMP_SEM_EVENT_INIT(_pSema)									\
	do{																\
		UINT __sem_status;												\
		__sem_status = tx_semaphore_create((_pSema), "", 1);					\
		if (__sem_status != TX_SUCCESS)										\
			dc_log_printf("%s(%d):semaCreate failed!\n", __FUNCTION__, __sem_status);\
	}while(0)

#define RTMP_SEM_EVENT_DESTORY(_pSema)	\
	do{																\
		UINT __sem_status;												\
		__sem_status = tx_semaphore_delete((_pSema));					\
		if (__sem_status != TX_SUCCESS)										\
			dc_log_printf("%s(%d):semaDel failed!\n", __FUNCTION__, __sem_status);\
	}while(0)

#define RTMP_SEM_EVENT_WAIT(_pSema, _status)	\
	do{\
		((_status)  = tx_semaphore_get(_pSema, TX_WAIT_FOREVER));\
		if ((_status) != TX_SUCCESS)\
			dc_log_printf("%s(%d):semGetFailed!\n", __FUNCTION__, (_status));\
	}while(0)

#define RTMP_SEM_EVENT_UP(_pSema)				\
	do{\
		UINT _status;\
		ULONG current_value = 0, suspend_cnt;\
		char *name;\
		TX_THREAD *first_suspended;\
		TX_SEMAPHORE *next_sema;\
		_status = tx_semaphore_info_get((_pSema), &name, &current_value, &first_suspended, &suspend_cnt, &next_sema);\
		if ((_status == TX_SUCCESS) && (current_value < 0xffffffff)){\
			_status = tx_semaphore_put((_pSema));\
			if ((_status) != TX_SUCCESS)\
				dc_log_printf("%s(%d):semaPut failed!\n", __FUNCTION__, (_status));\
		}else{\
			dc_log_printf("%s(%d):semaGetInfo failed(CurVal=%ld)!\n", __FUNCTION__, (_status), current_value);\
		}\
	}while(0)


/***********************************************************************************
 *	OS Memory Access related data structure and definitions
 ***********************************************************************************/
#ifdef SIGMATEL_SDK
/*
	Memory allocation related functions. 

	Because SigmaTel DC RAMMGR only allow memory allocation when system 
	do initialization. So, we need a alternately way to handle runtime dynamic 
	memory allocation requirments.
*/
typedef enum _RTMP_DC_RAMMGR_BUF_ID_{
	RTMP_DC_RAM_ID_DEVCTRL = 0,  /* pAd*/
	RTMP_DC_RAM_ID_STACK,
	RTMP_DC_RAM_ID_HTTX,
	RTMP_DC_RAM_ID_HTTX_ALIGN,	/* For alternate 512 bytes aligned HTTX buffer */
	RTMP_DC_RAM_ID_TX,
	RTMP_DC_RAM_ID_RX,
	RTMP_DC_RAM_ID_CTRL_PIPE,
	RTMP_DC_RAM_ID_URB,
#ifdef EBS_RTIP
	RTMP_DC_RAM_ID_NETBUF,
	RTMP_DC_RAM_ID_PKT,
#endif // EBS_RTIP //
#ifdef KODAK_DEMO
	RTMP_DC_RAM_ID_SHELLCMDBUF,
#endif // KODAK_DEMO
	RTMP_DC_RAM_ID_MAX,
}RTMP_DC_RAMMGER_BUF_ID;


typedef struct _RTMP_DC_RAM_LIST_{
	struct _RTMP_DC_RAM_LIST_ *next;
}RTMP_DC_RAM_LIST;

typedef struct _RTMP_DC_RAM_BLOCK_ELEMENT_
{
	RTMP_DC_RAMMGER_BUF_ID Id;
	char	*name;
	UINT32	size;
	UINT32	baseAddr;
	UINT32	endAddr;
	UINT32	alignment;
	UINT32	blkSize;
	UINT32	blkNum;
	RTMP_DC_RAM_LIST *pFreeList;
	UINT32	freeCnt;
	DC_RAMMGR_BUFFER_ENTRY dcMemBufEntry;
	NDIS_SPIN_LOCK DcRamBlkLock;
	BOOLEAN		bValid;
}RTMP_DC_RAM_BLOCK_ELEMENT;

extern RTMP_DC_RAM_BLOCK_ELEMENT RtmpDCRamMgrPSBBuf[];

#endif // SIGMATEL_SDK //

#define MEM_ALLOC_FLAG      (0)
#define GFP_ATOMIC          (0)
#define GFP_DMA             (0)
#define GFP_KERNEL          (0)

#define kmalloc(_size, _flags)	dc_malloc((_size))
#define kfree(_ptr)			dc_free((_ptr))

#define vmalloc(_size)		malloc((_size))
#define vfree(_ptr)			free((_ptr))


#define NdisMoveMemory(Destination, Source, Length) memmove(Destination, Source, Length)
#define NdisCopyMemory(Destination, Source, Length) memcpy(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)         memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)   memset(Destination, Fill, Length)
#define NdisCmpMemory(Destination, Source, Length)  memcmp(Destination, Source, Length)
#define NdisEqualMemory(Source1, Source2, Length)   (!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	(!memcmp(Source1, Source2, Length))

#define MlmeAllocateMemory(_pAd, _ppVA)		os_alloc_mem(_pAd, _ppVA, MGMT_DMA_BUFFER_SIZE)
#define MlmeFreeMemory(_pAd, _pVA)			os_free_mem(_pAd, _pVA)

#define COPY_MAC_ADDR(Addr1, Addr2)         memcpy((Addr1), (Addr2), MAC_ADDR_LEN)

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

#define wmb()				do{}while(0)

#ifdef SIGMATEL_SDK

#define RTMP_DC_RAM_CTRL_PIPE_NUM	10
#define RTMP_DC_RAM_CTRL_PIPE_SIZE	256

#endif // SIGMATEL_SDK //

/***********************************************************************************
 *	OS task related data structure and definitions
 ***********************************************************************************/
#define RTMP_TX_TASK_NUM 5		/* 4 for mgmtTask, 1 for NetTask, each with stack size 8092bytes */
#define RTMP_TX_TASK_STACK_SIZE 0x8000
#define RTMP_TX_TASK_PRIORITY 16

typedef void (*RTMP_OS_TASK_CALLBACK)(ULONG);

#define RTMP_OS_MGMT_TASK_FLAGS	0

// TODO: These definitions are for RT2870, not verify yet.
typedef	int	THREAD_PID;
#define	THREAD_PID_INIT_VALUE	(0)
#define	GET_PID(_v)	do{}while(0)
#define	GET_PID_NUMBER(_v)	do{}while(0)
#define CHECK_PID_LEGALITY(_pid) if(_pid)
#define KILL_THREAD_PID(_A, _B, _C) (0)


typedef void (*PNETFUNC)(ULONG arg1);

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
	PNETFUNC				funcPtr;
	ULONG					data;
	RTMP_NET_TASK_STATUS	taskStatus;
	char					taskName[8];
}RTMP_NET_TASK_STRUCT, *PRTMP_NET_TASK_STRUCT;

#define RTMP_NET_TASK_INIT(_pNetTask, _pFuncPtr, _pData)	\
	do{														\
		(_pNetTask)->funcPtr = (_pFuncPtr);					\
		(_pNetTask)->data = (unsigned int)(_pData);			\
		(_pNetTask)->taskStatus = RTMP_NET_TASK_INITED;		\
	}while(0)


#define tasklet_hi_schedule(_pTask)														\
	do{																					\
		DC_RETURN_CODE _retVal;															\
		_retVal = netJobAdd(_pTask, (_pTask)->data);							\
		if (_retVal == DC_SUCCESS)														\
			(_pTask)->taskStatus = RTMP_NET_TASK_PENDING;								\
		else																			\
			dc_log_printf("NetTask(%s) add failed!\n", (INT32)((_pTask)->taskName));	\
	}while(0)


/***********************************************************************************
 * Timer related definitions and data structures.
 **********************************************************************************/
// TODO: Need to check about this in ThreadX
#ifndef HZ
#define HZ 100
#endif

#ifdef SIGMATEL_SDK
/*
	For SigmaTel SDK, the tick initialized by dc_time_init(UINT32 sys_period_usec, UINT32 main_clock_hz)
	There are two way to get the OS_HZ
		1. by sys_period_usec (Which defined in main.c and now SYS_TIMESLICE_USEC = 1000)
			=> OS_HZ = 1 second / sys_period_usec = (1 * 1000 * 1000) / sys_period_usec = x ticks
		2. by dc_timer_ticks_from_usec(UINT32 *system_ticks, UINT32 usec) with usec = 1000000
		3. by dc_utime_get_from_tick(UINT32 *per_tick_usecs) to retrieves the system tick period in microseconds.
			=> OS_HZ = 1sec/per_tick_usecs = (1* 1000 * 1000) / per_tick_usecs = x ticks
	Here we use the method 1 to convert to OS_HZ.
*/
#define SYS_TIMESLICE_USEC 1000   /* NOTE: for this constant definition, need to sync with main.c */
#define OS_HZ	((1 * 1000 * 1000) / SYS_TIMESLICE_USEC)  /*HZ */
#endif // SIGMATEL_SDK //

typedef struct TX_TIMER_STRUCT RTMP_OS_TIMER;
typedef struct TX_TIMER_STRUCT NDIS_MINIPORT_TIMER;


typedef void (*TIMER_FUNCTION)(unsigned long);


/*
	The OS_WAIT ONLY can be called from a application thread!
*/
#define OS_WAIT(_time) 			\
	do{\
		unsigned long _ticks = (_time * OS_HZ) /1000; \
		if (_ticks == 0)\
			_ticks++;\
		tx_thread_sleep(_ticks);\
	}while(0)

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
#define mdelay(_us)		RTMPusecDelay((_us))

static inline void NdisGetSystemUpTime(ULONG *sys_time)
{
	*sys_time = tx_time_get();
}


/***********************************************************************************
 *	OS specific cookie data structure binding to RTMP_ADAPTER
 ***********************************************************************************/

struct os_cookie {

#ifdef SIGMATEL_SDK
	
#endif // SIGMATEL_SDK //

	/*
		For ThreadX, all task declaration should defined between "_taskStartAddr" and "_taskEndAddr", 
		because we use "(_taskEndAddr - _taskStartAddr)/ sizeof(RTMP_NET_TASK_STRUCT)" to calculate
		the total number to tasklet will used in ThreadX, and depends on this number to allocate the stack
		memory segments.
	*/
	ULONG _taskStartAddr;
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
#ifdef RTMP_MAC_PCI
	RTMP_NET_TASK_STRUCT fifo_statistic_full_task;
#endif // RTMP_MAC_PCI //

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
	RTMP_NET_TASK_STRUCT uapsd_eosp_sent_task;
#endif // UAPSD_AP_SUPPORT //

#ifdef DFS_SUPPORT
	RTMP_NET_TASK_STRUCT pulse_radar_detect_task;
	RTMP_NET_TASK_STRUCT width_radar_detect_task;
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
	RTMP_NET_TASK_STRUCT carrier_sense_task;
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


	ULONG _taskEndAddr;

	/*
		signal related data elements
	*/
	unsigned long			apd_pid; //802.1x daemon pid
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
	RT_SIGNAL_STRUC			RTSignal;
	unsigned long			IappPid; //IAPP daemon pid
#endif // IAPP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //
#ifdef WAPI_SUPPORT
	unsigned long			wapi_pid; //wapi daemon pid
#endif // WAPI_SUPPORT //

	/* 
		IOCTL related data elements
	*/
	INT						ioctl_if_type;
	INT 					ioctl_if;

#ifdef WSC_AP_SUPPORT
	/*
		WSC related data elements
	*/
	int					(*WscMsgCallBack)();
#endif // WSC_AP_SUPPORT //
};

typedef struct os_cookie	* POS_COOKIE;


/***********************************************************************************
 *	OS debugging and printing related definitions and data structure
 ***********************************************************************************/
#define printk	dc_log_printf
#define PRINT_MAC(addr)	\
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]
	
#ifdef DBG
extern ULONG		RTDebugLevel;

#define DBGPRINT_RAW(Level, Fmt)    \
do{                                   \
    if (Level <= RTDebugLevel)      \
    {                               \
		dc_log_printf Fmt; 	\
    }                               \
}while(0)

#define DBGPRINT(Level, Fmt)    DBGPRINT_RAW(Level, Fmt)


#define DBGPRINT_ERR(Fmt)           \
do{                                   			\
	dc_log_printf("ERROR!!! ");      	\
    	dc_log_printf Fmt;                 	 	\
}while(0)

#define DBGPRINT_S(Status, Fmt)		\
do{									\
	dc_log_printf Fmt;					\
}while(0)
#else
#define DBGPRINT(Level, Fmt)		do{}while(0)
#define DBGPRINT_RAW(Level, Fmt)	do{}while(0)
#define DBGPRINT_S(Status, Fmt)		do{}while(0)
#define DBGPRINT_ERR(Fmt)			do{}while(0)
#define RTMP_ISR_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)	do{}while(0)
#endif

#undef  ASSERT
#define ASSERT(x)                                                               \
do{                                                                             \
    if (!(x))                                                                   \
    {                                                                           \
        dc_log_printf("%s:%d assert " #x "failed\n", __FILE__, __LINE__);    	\
    }                                                                           \
}while(0)

extern void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);


/***********************************************************************************
 * Device DMA Access related definitions and data structures.
 **********************************************************************************/
/* This defines the direction arg to the DMA mapping routines. */
#define PCI_DMA_BIDIRECTIONAL	0
#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2
#define PCI_DMA_NONE		3

#ifdef RTMP_MAC_PCI
/* signature for performance enhancement */
#ifdef CACHE_DMA_SUPPORT
extern UCHAR CACHE_DMA_SIGNAUTURE[];
#endif // CACHE_DMA_SUPPORT //

#endif // RTMP_MAC_PCI //



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

// in VxWorks, MMU is disabled so Physical address == virtual address
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
{																	\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
	(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
}

#define RTMP_IO_READ32(_A, _R, _pV)									\
do{																	\
    if ((_A)->bPCIclkOff == FALSE)                                      \
    {                                                                   \
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
	(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
    }                                                                   \
}while(0)

#define RTMP_IO_READ8(_A, _R, _pV)									\
do{																	\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readb((void *)((_A)->CSRBaseAddress + (_R))));			\
}while(0)

#define RTMP_IO_WRITE32(_A, _R, _V)									\
do{																	\
    if ((_A)->bPCIclkOff == FALSE)                                      \
    {                                                                   \
	UINT32	_Val;													\
	_Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));		\
	_Val = SWAP32(_V);												\
	writel(_Val, (void *)((_A)->CSRBaseAddress + (_R)));			\
    }                                                                   \
}while(0)

#define RTMP_IO_WRITE8(_A, _R, _V)									\
do{																	\
	UINT	Val;													\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));		\
	writeb((_V), (PUCHAR)((_A)->CSRBaseAddress + (_R)));			\
}while(0)

#define RTMP_IO_WRITE16(_A, _R, _V)									\
do{																	\
	UINT	Val;													\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));		\
	writew(SWAP16((_V)), (PUSHORT)((_A)->CSRBaseAddress + (_R)));	\
}while(0)
#else
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)								\
{																\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
}

#define RTMP_IO_READ32(_A, _R, _pV)								\
do{																\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
		(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));		\
		(*_pV = readl((void *)((_A)->CSRBaseAddress + (_R))));			\
    }                                                               \
    else															\
		*_pV = 0;													\
}while(0)

#define RTMP_IO_READ8(_A, _R, _pV)								\
do{																\
	(*_pV = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0)));			\
	(*_pV = readb((void *)((_A)->CSRBaseAddress + (_R))));				\
}while(0)

#define RTMP_IO_WRITE32(_A, _R, _V)												\
do{																				\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
	UINT	Val;																\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));			\
	writel((_V), (void *)((_A)->CSRBaseAddress + (_R)));								\
    }                                                               \
}while(0)



#if defined(BRCM_6358) || defined(RALINK_2880) || defined(RALINK_3052)
#define RTMP_IO_WRITE8(_A, _R, _V)            \
do{                    \
	ULONG Val;                \
	UCHAR _i;                \
	_i = ((_R) & 0x3);             \
	Val = readl((void *)((_A)->CSRBaseAddress + ((_R) - _i)));   \
	Val = Val & (~(0x000000ff << ((_i)*8)));         \
	Val = Val | ((ULONG)(_V) << ((_i)*8));         \
	writel((Val), (void *)((_A)->CSRBaseAddress + ((_R) - _i)));    \
}while(0)
#else
#define RTMP_IO_WRITE8(_A, _R, _V)							\
do{															\
	UINT	Val;												\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));	\
	writeb((_V), (PUCHAR)((_A)->CSRBaseAddress + (_R)));		\
}while(0)
#endif // #if defined(BRCM_6358) || defined(RALINK_2880) //

#define RTMP_IO_WRITE16(_A, _R, _V)							\
do{															\
	UINT	Val;											\
	Val = readl((void *)((_A)->CSRBaseAddress + MAC_CSR0));	\
	writew((_V), (PUSHORT)((_A)->CSRBaseAddress + (_R)));	\
}while(0)
#endif // #if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(IKANOS_VX_1X0) //
#endif // RTMP_MAC_PCI //



/***********************************************************************************
 *	Network Related data structure and marco definitions
 ***********************************************************************************/
#define PKTSRC_NDIS             0x7f
#define PKTSRC_DRIVER           0x0f

#ifdef EBS_RTIP

#define CB_OFF  	0
#define CB_MAX_OFFSET		32

#define RTMP_NETBUF_TX		0
#define RTMP_NETBUF_RX		1

typedef struct _RTMP_RTIP_NET_BUF_{
	void *next;
	UCHAR	cb[CB_MAX_OFFSET];
	UCHAR	*pFrameBuf; // Pointer to internal buffer for Rx.
	DCU		pDcuBuf;
	UCHAR	*pDataHead;
	UCHAR	*pDataPtr;
	int		pktLen;
	PIFACE	pi;
	BOOLEAN	 bDcuFlg;
}RTMP_RTIP_NET_BUF;

extern INT RtmpIfaceId;
extern PIFACE RtmpPI;

void * RtmpTxNetBuf_put(RTMP_RTIP_NET_BUF *pNetBuf, int n);
void *RtmpTxNetBuf_push(RTMP_RTIP_NET_BUF *pNetBuf, int n);

void hex_dump_DCU(PUCHAR callerStr, DCU pDcu, BOOLEAN bDumpPkt);
void hex_dump_PIFACE(PUCHAR callerStr, PIFACE pi);

#define skb_put(_pNetPkt, n)	RtmpTxNetBuf_put(_pNetPkt, n)
#define skb_push(_pNetPkt, n)		RtmpTxNetBuf_push(_pNetPkt, n)
#define skb_reserve(skb,n)

#define RTMP_OS_NETDEV_STATE_RUNNING(_pNetDev)		((_pNetDev)->open_count > 0)

#define RTMP_OS_NETDEV_GET_PRIV(_pNetDev)			(RT_WIFI_DevCtrl)
#define RTMP_OS_NETDEV_SET_PRIV(_pNetDev, _pPriv)		do{}while(0)
#define RTMP_OS_NETDEV_GET_DEVNAME(_pNetDev) 		((_pNetDev)->pdev->device_name)
#define RTMP_OS_NETDEV_GET_PHYADDR(_pNetDev)		((_pNetDev)->addr.my_hw_addr)
#define RTMP_OS_NETDEV_PHYADDR_LEN(_pNetDev)		(ETH_ALEN)

/* Get & Set NETDEV interface hardware type */
#define RTMP_OS_NETDEV_GET_TYPE(_pNetDev)
#define RTMP_OS_NETDEV_SET_TYPE(_pNetDev, _type)

#define QUEUE_ENTRY_TO_PACKET(pEntry) \
	(PNDIS_PACKET)(pEntry)

#define PACKET_TO_QUEUE_ENTRY(pPacket) \
	(PQUEUE_ENTRY)(pPacket)

#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)  \
		RTMPFreeNdisPacket(_pAd, _pPacket)

/*
 * packet helper
 * 	- convert internal rt packet to os packet or
 *             os packet to rt packet
 */
#define RTPKT_TO_OSPKT(_p)				((RTMP_RTIP_NET_BUF *)(_p))
#define OSPKT_TO_RTPKT(_p)				((PNDIS_PACKET)(_p))
#define GET_OS_PKT_DATAPTR(_pkt) 		((RTPKT_TO_OSPKT(_pkt)->pDataPtr))
#define SET_OS_PKT_DATAPTR(_pkt, _dataPtr) \
							((((RTMP_RTIP_NET_BUF *)(_pkt))->pDataPtr) = (_dataPtr))

#define GET_OS_PKT_LEN(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->pktLen)

#define SET_OS_PKT_LEN(_pkt, _len)	\
		(RTPKT_TO_OSPKT(_pkt)->pktLen) = (_len)


#define GET_OS_PKT_DATATAIL(_pkt) \
		((RTPKT_TO_OSPKT(_pkt)->pDcuBuf->data) + (RTPKT_TO_OSPKT(_pkt)->pDcuBuf->length))

#define SET_OS_PKT_DATATAIL(_pkt, _start, _len)	do {}while(0)

#define GET_OS_PKT_NETDEV(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->pi)

#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
		(RTPKT_TO_OSPKT(_pkt)->pi) = (_pNetDev)
		
#define GET_OS_PKT_TYPE(_pkt) \
		(RTPKT_TO_OSPKT(_pkt))


#define GET_OS_PKT_HEADROOM(_pkt)	(0)
		
#define GET_OS_PKT_TOTAL_LEN(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->pktLen)
		

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

// 0xff: PKTSRC_NDIS, others: local TX buffer index. This value affects how to a packet
#define RTMP_SET_PACKET_SOURCE(_p, _pktsrc)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+3] = _pktsrc)
#define RTMP_GET_PACKET_SOURCE(_p)       		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+3])

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
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (!RTMP_PACKET_SPECIFIC_DHCP);	\
			}while(0)
#define RTMP_GET_PACKET_DHCP(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_DHCP)

//EAPOL
#define RTMP_SET_PACKET_EAPOL(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_EAPOL);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (!RTMP_PACKET_SPECIFIC_EAPOL);	\
			}while(0)
#define RTMP_GET_PACKET_EAPOL(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_EAPOL)

//WAI
#define RTMP_SET_PACKET_WAI(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_WAI);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (!RTMP_PACKET_SPECIFIC_WAI);	\
			}while(0)
#define RTMP_GET_PACKET_WAI(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_WAI)

#define RTMP_GET_PACKET_LOWRATE(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & (RTMP_PACKET_SPECIFIC_EAPOL | RTMP_PACKET_SPECIFIC_DHCP | RTMP_PACKET_SPECIFIC_WAI))

//VLAN
#define RTMP_SET_PACKET_VLAN(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_VLAN);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (!RTMP_PACKET_SPECIFIC_VLAN);	\
			}while(0)
#define RTMP_GET_PACKET_VLAN(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_VLAN)

//LLC/SNAP
#define RTMP_SET_PACKET_LLCSNAP(_p, _flg)   													\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_LLCSNAP);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (!RTMP_PACKET_SPECIFIC_LLCSNAP);		\
			}while(0)
			
#define RTMP_GET_PACKET_LLCSNAP(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_LLCSNAP)

// IP
#define RTMP_SET_PACKET_IPV4(_p, _flg)														\
			do{																				\
				if (_flg)																	\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_IPV4);		\
				else																		\
					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (!RTMP_PACKET_SPECIFIC_IPV4);	\
			}while(0)
			
#define RTMP_GET_PACKET_IPV4(_p)		(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_IPV4)


// If this flag is set, it indicates that this EAPoL frame MUST be clear.
#define RTMP_SET_PACKET_CLEAR_EAP_FRAME(_p, _flg)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+12] = _flg)
#define RTMP_GET_PACKET_CLEAR_EAP_FRAME(_p)         (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+12])


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
	(((UINT16)(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+23])) << 8) \
	| ((UINT16)(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+24]))

#ifdef INF_AMAZON_SE
/* [CB_OFF+28], 1B, Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
#define RTMP_SET_PACKET_NOBULKOUT(_p, _morebit)			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+28] = _morebit)
#define RTMP_GET_PACKET_NOBULKOUT(_p)					(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+28])			
#endif // INF_AMAZON_SE //


// TODO: Check if the ntohs is workable in ThreadX
#define OS_NTOHS(_Val) \
		(ntohs(_Val))
#define OS_HTONS(_Val) \
		(htons(_Val))
#define OS_NTOHL(_Val) \
		(ntohl(_Val))
#define OS_HTONL(_Val) \
		(htonl(_Val))


#define RTMP_OS_NETDEV_START_QUEUE(_pNetDev)	do{}while(0)
#define RTMP_OS_NETDEV_STOP_QUEUE(_pNetDev)		do{}while(0)
#endif // EBS_RTIP //



/******************************************************************************

  	net task related definitions

******************************************************************************/
DC_RETURN_CODE netJobAdd(PRTMP_NET_TASK_STRUCT net_task, ULONG  arg1);
DC_RETURN_CODE net_task_pre_init();


#ifdef RTMP_IOCTL_SUPPORT
/******************************************************************************

  	ThreadX SHELL CMD related definitions

******************************************************************************/
typedef struct _RTMP_IWPRIV_CMD_STRUCT_
{
	PSTRING	pDevName;
	PSTRING pCmd;
	PSTRING pParam;
}RTMP_IWPRIV_CMD_STRUCT;

#endif // RTMP_IOCTL_SUPPORT //

/*******************************************************************************
	ThreadX Error Code Mapping
 *******************************************************************************/
 #define EINVAL TX_THREAD_ERROR

#ifdef RALINK_ATE
/******************************************************************************

  	ATE related definitions

******************************************************************************/


#define KERN_EMERG
#define ate_print printf
#define ATEDBGPRINT DBGPRINT
#ifdef RTMP_MAC_PCI
#ifdef CONFIG_AP_SUPPORT
#define EEPROM_BIN_FILE_NAME  "/tgtsvr/pci/e2p_ap.bin"
#endif // CONFIG_AP_SUPPORT //
#endif // RTMP_MAC_PCI //

#endif // RALINK_ATE //


/***********************************************************************************
 *	Other function prototypes definitions
 ***********************************************************************************/
void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);

long simple_strtol(const char *cp,char **endp,unsigned int base);


RTP_BOOL MainVirtualIF_open(
	IN PIFACE pi);

void MainVirtualIF_close(
	IN PIFACE pi);

int rt28xx_send_packets(
	IN struct _iface RTP_FAR *pi,
	IN DCU pDcu);


#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif // WSC_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif

#define INT_MAX         ((int)(~0U>>1))

#define ENOMEM 0

#endif // __RT_THREADX_H__ //
