/***********************************************************************/
/*                                                                     */
/*   Program:    rt_linux.c                                            */
/*   Created:    4/21/2006 1:17:38 PM                                  */
/*   Author:     Wu Xi-Kun                                             */
/*   Comments:   `description`                                         */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/* History:                                                            */
/*    Revision 1.1 4/21/2006 1:17:38 PM  xsikun                        */
/*    Initial revision                                                 */
/*                                                                     */
/***********************************************************************/
#ifndef __RT_UCOS_H__
#define __RT_UCOS_H__

#include <stdio.h>
#include "includes.h"
#include <mips/config.h>
#include <mips/mips16.h>
#include <mips/types.h>
#include <mips/io.h>
#include <init.h>
#include <timer.h>
#include <port/spinlock.h>
#include <delay.h>
#include <cache.h>
#include <malloc.h>


#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/pbuf.h>
#include <lwip/mem.h>
#include <lwip/ip.h>
#include <lwip/sys.h>
#include <arch/cc.h>

#include <netif/etharp.h>

#include <rt2882.h>
#include <rt_pkt.h>


/*#define RX_SCATTERED */
#define RX_SCATTER_SIZE		(1024*2)
/*#define CONFIG_CKIP_SUPPORT */

#undef __inline
#define __inline	   static inline 

#define dev_addr	hwaddr
#define OS_HZ			HZ

/* Define Linux ioctl relative structure, keep only necessary things */
struct  iw_point
{
	PVOID          pointer;
	USHORT         length;
	USHORT         flags;
};
	
union   iwreq_data
{
	struct iw_point data;
};

struct iwreq {
	union   iwreq_data      u;
};

struct ifreq {
	union   iwreq_data      u;
};

#define MAC_ADDR_LEN	6

#define IN
#define OUT

#define NDIS_STATUS                             INT
#define NDIS_STATUS_SUCCESS                     0x00
#define NDIS_STATUS_FAILURE                     0x01
#define NDIS_STATUS_INVALID_DATA				0x02
#define NDIS_STATUS_RESOURCES                   0x03


#ifdef CONFIG_AP_SUPPORT
#define PROFILE_PATH			"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define NIC_DEVICE_NAME			"RT2860AP"
#define DRIVER_VERSION                  "1.4.0.0"
#endif /* CONFIG_AP_SUPPORT */


struct os_lock  {
	spinlock_t		lock;
	unsigned long  	flags;
};

#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
typedef struct _RT_SIGNAL_STRUC {
	USHORT					Sequence;
    UCHAR					MacAddr[MAC_ADDR_LEN];
    UCHAR					CurrAPAddr[MAC_ADDR_LEN];
    UCHAR					Sig;
} RT_SIGNAL_STRUC, *PRT_SIGNAL_STRUC;

/* definition of signal */
#define	SIG_NONE					0
#define SIG_ASSOCIATION				1
#define SIG_REASSOCIATION			2
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

struct os_cookie {
	struct netif			*pci_dev;
	unsigned long			apd_pid; /*802.1x daemon pid */
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
	RT_SIGNAL_STRUC			RTSignal;
	unsigned long			IappPid; /*IAPP daemon pid */
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	INT						ioctl_if_type;
	INT 					ioctl_if;
};	

/*+++shall be removed after verification */
/*---shall be removed after verification */

typedef struct os_cookie	* POS_COOKIE;
typedef struct netif		* PNET_DEV;
typedef void				* PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET		* PPNDIS_PACKET;
typedef	ra_dma_addr_t			NDIS_PHYSICAL_ADDRESS;
typedef	ra_dma_addr_t			* PNDIS_PHYSICAL_ADDRESS;
typedef struct os_lock		NDIS_SPIN_LOCK;
typedef struct timer_list	NDIS_MINIPORT_TIMER;
typedef void				* NDIS_HANDLE;
typedef char 				* PNDIS_BUFFER;


void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

ra_dma_addr_t rt_pci_map_single(void *handle, void *ptr, size_t size, int direction);
void rt_pci_unmap_single(void *handle, ra_dma_addr_t dma_addr, size_t size, int direction);


/*////////////////////////////////////*/
/* MOVE TO rtmp.h ? */
/*/////////////////////////////////////*/
#define PKTSRC_NDIS             0x7f
#define PKTSRC_DRIVER           0x0f
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]


/* This defines the direction arg to the DMA mapping routines. */
#define PCI_DMA_BIDIRECTIONAL	0
#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2
#define PCI_DMA_NONE		3

#define RTMP_PCI_DMA_TODEVICE		PCI_DMA_TODEVICE
#define RTMP_PCI_DMA_FROMDEVICE		PCI_DMA_FROMDEVICE


#define PCI_MAP_SINGLE(_handle, _ptr, _size, _dir) \
	rt_pci_map_single(_handle, _ptr, _size, _dir)

#define PCI_UNMAP_SINGLE(_handle, _ptr, _size, _dir) \
	rt_pci_unmap_single(_handle, _ptr, _size, _dir)


/*//////////////////////////////////////*/
/* */
/*//////////////////////////////////////*/


#define NdisMIndicateStatus(_w, _x, _y, _z)

typedef struct timer_list	RTMP_OS_TIMER;

typedef struct  _RALINK_TIMER_STRUCT    {
    RTMP_OS_TIMER		TimerObj;       /* Ndis Timer object */
	BOOLEAN				Valid;			/* Set to True when call RTMPInitTimer */
    BOOLEAN             State;          /* True if timer cancelled */
    BOOLEAN             Repeat;         /* True if periodic timer */
    ULONG               TimerValue;     /* Timer value in milliseconds */
	ULONG				cookie;			/* os specific object */
}   RALINK_TIMER_STRUCT, *PRALINK_TIMER_STRUCT;




#define RTMP_TIME_AFTER(a,b)		\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(b) - (long)(a) < 0))
#define RTMP_TIME_BEFORE(a,b)	RTMP_TIME_AFTER_EQ(b,a)

#define RTMP_TIME_AFTER_EQ(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(a) - (long)(b) >= 0))
#define RTMP_TIME_BEFORE(a,b)	RTMP_TIME_AFTER_EQ(b,a)

#define DBG	1

/* */
/*  MACRO for debugging information */
/* */
#define MTWF_PRINT	printk


/* */
/*  spin_lock enhanced for Nested spin lock */
/* */
#define NdisAllocateSpinLock(__pReserve, __lock)      \
{                                       \
    spin_lock_init(&(__lock)->lock);               \
}

#define NdisFreeSpinLock(lock)          \
{                                       \
}


#define RTMP_SEM_LOCK(__lock)					\
{												\
	spin_lock_bh(&(__lock)->lock); 				\
}

#define RTMP_SEM_UNLOCK(__lock)					\
{												\
	spin_unlock_bh(&(__lock)->lock); 				\
}

#define RTMP_IRQ_LOCK(__lock, __irqflag)					\
{													\
	local_irq_save(__irqflag);	\
}

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)						\
{														\
	local_irq_restore(__irqflag);	\
}

#define RTMP_IO_READ32(_A, _R, _pV)								\
{																\
	*_pV = RT2882_REG(((_A)->PciHif.CSRBaseAddress + (_R)));		\
}
#define RTMP_IO_WRITE32(_A, _R, _V)												\
{																\
	RT2882_REG((_A)->PciHif.CSRBaseAddress + (_R)) = _V;							\
}


#define RTMP_IO_READ8(_A, _R, _pV)								\
{																\
	(*_pV = readb((void *)((_A)->PciHif.CSRBaseAddress + (_R))));				\
}

#define RTMP_IO_WRITE8(_A, _R, _V)												\
do{																				\
	UINT	Val;																\
	Val = readl(((_A)->PciHif.CSRBaseAddress + ((_R)/4)*4));	\
	Val = (Val & ~(0xFF<<(((_R)%4)*8))) |  ((_V) <<(((_R)%4)*8));	\
	writel(Val, (void *)((_A)->PciHif.CSRBaseAddress + ((_R)/4)*4));								\
	/*writeb((_V), (PUCHAR)((_A)->PciHif.CSRBaseAddress + (_R)));*/		\
}while(0)


#define RTMP_IO_WRITE16(_A, _R, _V)												\
do{																				\
	UINT	Val;																\
	writew((_V), (PUSHORT)((_A)->PciHif.CSRBaseAddress + (_R)));	\
}while(0)


/* Modified by Wu Xi-Kun 4/21/2006 */
typedef void (*TIMER_FUNCTION)(unsigned long);


#define RTMP_IndicateMediaState()
#define COPY_MAC_ADDR(Addr1, Addr2)             memcpy((Addr1), (Addr2), MAC_ADDR_LEN)

#define MlmeAllocateMemory(_pAd, _ppVA) os_alloc_mem(_pAd, _ppVA, MGMT_DMA_BUFFER_SIZE)
#define MlmeFreeMemory( _pVA)     os_free_mem( _pVA)


void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);


/*
 * packet helper 
 * 	- convert internal rt packet to os packet or 
 *             os packet to rt packet
 */      
#define RTPKT_TO_OSPKT(_p)		((struct net_pkt_blk *)(_p))
#define OSPKT_TO_RTPKT(_p)		((PNDIS_PACKET)(_p))

#define GET_OS_PKT_DATAPTR(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->data)

#define GET_OS_PKT_LEN(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->len)

#define GET_OS_PKT_TYPE(_pkt) \
		(RTPKT_TO_OSPKT(_pkt))

#define GET_OS_PKT_NEXT(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->next)


/* statistics counter */
#define STATS_INC_RX_PACKETS(_pAd, _dev)
#define STATS_INC_TX_PACKETS(_pAd, _dev)

#define STATS_INC_RX_BYTESS(_pAd, _dev, len)
#define STATS_INC_TX_BYTESS(_pAd, _dev, len)

#define STATS_INC_RX_ERRORS(_pAd, _dev)
#define STATS_INC_TX_ERRORS(_pAd, _dev)

#define STATS_INC_RX_DROPPED(_pAd, _dev)
#define STATS_INC_TX_DROPPED(_pAd, _dev)


#define CB_OFF  10
#define PACKET_CB(_p, _offset)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF + (_offset)]))

/*   check DDK NDIS_PACKET data structure and find out only MiniportReservedEx[0..7] can be used by our driver without */
/*   ambiguity. Fields after pPacket->MiniportReservedEx[8] may be used by other wrapper layer thus crashes the driver */
/* */
#define RTMP_GET_PACKET_MR(_p)			(RTPKT_TO_OSPKT(_p))


#define NDIS_SET_PACKET_STATUS(_p, _status)

#define GET_SG_LIST_FROM_PACKET(_p, _sc)	\
    rt_get_sg_list_from_packet(_p, _sc)


/* Dennis Lee */

#if defined(CONFIG_RALINK_RT2880_VER_A)
#define RTMP_MAC_CSR_ADDR 0xA0600000
#elif defined(CONFIG_RALINK_RT2880_VER_B)
#define RTMP_MAC_CSR_ADDR 0xA0480000
#else
#error Please Choice Chip Version (A/B)
#endif

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define	IFF_UP		0x1		/* interface is up		*/

#define MEM_ALLOC_FLAG	0
#define GFP_ATOMIC		0
#define GFP_KERNEL		0
#define GFP_DMA			0

#define kfree free
#define kmalloc(x, y) malloc(x)
#define copy_to_user(x,y,z) memcpy(x,y,z)

/* Dennis Lee - */
#define NdisMoveMemory(Destination, Source, Length) memmove(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)         memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)   memset(Destination, Fill, Length)
#define NdisEqualMemory(Source1, Source2, Length)   (!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	(!memcmp(Source1, Source2, Length))								   


#define RTMP_INC_REF(_A)		0
#define RTMP_DEC_REF(_A)		0
#define RTMP_GET_REF(_A)		0



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

#define QUEUE_ENTRY_TO_PACKET(pEntry) \
	(PNDIS_PACKET)(pEntry)

#define PACKET_TO_QUEUE_ENTRY(pPacket) \
	(PQUEUE_ENTRY)(pPacket)


#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)                    \
{                                                                       \
        RTMPFreeNdisPacket(_pAd, _pPacket);                             \
}


#define RELEASE_NDIS_PACKET_IRQ(_pAd, _pPacket, _Status)                \
{                                                                       \
        RTMPFreeNdisPacket(_pAd, _pPacket);                             \
}


#define SWITCH_PhyAB(_pAA, _pBB)    \
{                                                                           \
    ULONG	AABasePaHigh;                           \
    ULONG	AABasePaLow;                           \
    ULONG	BBBasePaHigh;                           \
    ULONG	BBBasePaLow;                           \
    BBBasePaHigh = RTMP_GetPhysicalAddressHigh(_pBB);                                                 \
    BBBasePaLow = RTMP_GetPhysicalAddressLow(_pBB);                                                 \
    AABasePaHigh = RTMP_GetPhysicalAddressHigh(_pAA);                                                 \
    AABasePaLow = RTMP_GetPhysicalAddressLow(_pAA);                                                 \
    RTMP_SetPhysicalAddressHigh(_pAA, BBBasePaHigh);                                                 \
    RTMP_SetPhysicalAddressLow(_pAA, BBBasePaLow);                                                 \
    RTMP_SetPhysicalAddressHigh(_pBB, AABasePaHigh);                                                 \
    RTMP_SetPhysicalAddressLow(_pBB, AABasePaLow);                                                 \
}


#define NdisWriteErrorLogEntry(_a, _b, _c, _d)


#define NdisAcquireSpinLock		RTMP_SEM_LOCK
#define NdisReleaseSpinLock		RTMP_SEM_UNLOCK
#define RTMP_SPIN_LOCK			RTMP_SEM_LOCK
#define RTMP_SPIN_UNLOCK		RTMP_SEM_UNLOCK

static inline void NdisGetSystemUpTime(ULONG *time)
{
	*time = jiffies;
}


extern struct _RTMP_ADAPTER	rtmp_Adapter;


#define RTMP_OS_PKT_INIT(__pPacket, __pNetDev, __pData, __DataSize)			\
{																			\
	PNDIS_PACKET __pRxPkt;													\
	__pRxPkt = RTPKT_TO_OSPKT(__pRxPacket);									\
	SET_OS_PKT_NETDEV(__pRxPkt, __pNetDev);									\
	SET_OS_PKT_DATAPTR(__pRxPkt, __pData);									\
	SET_OS_PKT_LEN(__pRxPkt, __DataSize);									\
	SET_OS_PKT_DATATAIL(__pRxPkt, __pData, __DataSize);						\
}


/* ATE */
/* RT2880_iNIC and RT288x router should define "RTMP_MAC_PCI" !!! */
/* iNIC has its own EEPROM_BIN_FILE_NAME */
#define fATE_LOAD_EEPROM						0x0C43
#ifdef CONFIG_PRINTK
extern INT ConsoleResponse(IN PUCHAR buff);
extern int (*remote_display)(char *);

/* specificly defined to redirect and show ate-related messages to host. */
/* Try to define ate_print as a macro. */
#define ate_print(fmt, args...)                 \
do{   int (*org_remote_display)(char *) = NULL;   \
	org_remote_display = remote_display;\
	/* Save original "remote_display" */\
	remote_display = (int (*)(char *))ConsoleResponse;           \
	printk(fmt, ## args);                       \
	/* Restore the remote_display function pointer */        \
	remote_display = org_remote_display; }while(0)

#define ATEDBGPRINT(Level, Fmt)    	\
{                                   \
    if ((Level) <= DebugLevel)      \
    {                               \
        ate_print Fmt;					\
    }                               \
}
#endif /* CONFIG_PRINTK */

INT IoctlResponse(PUCHAR payload, PUCHAR msg, INT len);

extern INT ConsoleResponse(IN PUCHAR buff);
extern int (*remote_display)(char *);

#undef	copy_to_user
#define copy_to_user(x,y,z) ate_copy_to_user((PUCHAR)x, (PUCHAR)y, z)

#define OS_PKT_HEAD_BUF_EXTEND(_pkt, _offset)								\
	net_pkt_push(RTPKT_TO_OSPKT(_pkt), _offset)

#include "os/rt_os.h"



#ifdef FW_DUMP_SUPPORT	
#define RTMP_OS_FWDUMP_PROCINIT(ad)
#define RTMP_OS_FWDUMP_PROCCREATE(ad)
#define RTMP_OS_FWDUMP_PROCREMOVE(ad)
#define RTMP_OS_FWDUMP_SETPATH(ad, ar)
#endif

#endif /* __RT_UCOS_H__ */
