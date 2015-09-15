#include "rt_config.h"

ULONG	RTDebugLevel = RT_DEBUG_ERROR;
ULONG	RTDebugFunc = 0;

#ifdef RTMP_RBUS_SUPPORT
#if defined(CONFIG_RA_HW_NAT) || defined(CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../../../net/nat/hw_nat/frame_engine.h"
#endif
#endif // RTMP_RBUS_SUPPORT //

#ifdef VENDOR_FEATURE4_SUPPORT
ULONG	OS_NumOfMemAlloc = 0, OS_NumOfMemFree = 0;
#endif // VENDOR_FEATURE4_SUPPORT //


#ifdef SYSTEM_LOG_SUPPORT
// for wireless system event message
char const *pWirelessSysEventText[IW_SYS_EVENT_TYPE_NUM] = {    
	// system status event     
    "had associated successfully",							/* IW_ASSOC_EVENT_FLAG */
    "had disassociated",									/* IW_DISASSOC_EVENT_FLAG */
    "had deauthenticated",									/* IW_DEAUTH_EVENT_FLAG */
    "had been aged-out and disassociated",					/* IW_AGEOUT_EVENT_FLAG */
    "occurred CounterMeasures attack",						/* IW_COUNTER_MEASURES_EVENT_FLAG */	
    "occurred replay counter different in Key Handshaking",	/* IW_REPLAY_COUNTER_DIFF_EVENT_FLAG */
    "occurred RSNIE different in Key Handshaking",			/* IW_RSNIE_DIFF_EVENT_FLAG */
    "occurred MIC different in Key Handshaking",			/* IW_MIC_DIFF_EVENT_FLAG */
    "occurred ICV error in RX",								/* IW_ICV_ERROR_EVENT_FLAG */
    "occurred MIC error in RX",								/* IW_MIC_ERROR_EVENT_FLAG */
	"Group Key Handshaking timeout",						/* IW_GROUP_HS_TIMEOUT_EVENT_FLAG */ 
	"Pairwise Key Handshaking timeout",						/* IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG */ 
	"RSN IE sanity check failure",							/* IW_RSNIE_SANITY_FAIL_EVENT_FLAG */ 
	"set key done in WPA/WPAPSK",							/* IW_SET_KEY_DONE_WPA1_EVENT_FLAG */ 
	"set key done in WPA2/WPA2PSK",                         /* IW_SET_KEY_DONE_WPA2_EVENT_FLAG */ 
	"connects with our wireless client",                    /* IW_STA_LINKUP_EVENT_FLAG */ 
	"disconnects with our wireless client",                 /* IW_STA_LINKDOWN_EVENT_FLAG */
	"scan completed",										/* IW_SCAN_COMPLETED_EVENT_FLAG */
	"scan terminate!! Busy!! Enqueue fail!!",				/* IW_SCAN_ENQUEUE_FAIL_EVENT_FLAG */
	"channel switch to ",									/* IW_CHANNEL_CHANGE_EVENT_FLAG */
	"wireless mode is not support",							/* IW_STA_MODE_EVENT_FLAG */
	"blacklisted in MAC filter list",						/* IW_MAC_FILTER_LIST_EVENT_FLAG */
	"Authentication rejected because of challenge failure",	/* IW_AUTH_REJECT_CHALLENGE_FAILURE */
	"Scanning",												/* IW_SCANNING_EVENT_FLAG */
	"Start a new IBSS",										/* IW_START_IBSS_FLAG */
	"Join the IBSS",										/* IW_JOIN_IBSS_FLAG */
	};

#ifdef IDS_SUPPORT
// for wireless IDS_spoof_attack event message
char const *pWirelessSpoofEventText[IW_SPOOF_EVENT_TYPE_NUM] = {   	
    "detected conflict SSID",								/* IW_CONFLICT_SSID_EVENT_FLAG */
    "detected spoofed association response",				/* IW_SPOOF_ASSOC_RESP_EVENT_FLAG */
    "detected spoofed reassociation responses",				/* IW_SPOOF_REASSOC_RESP_EVENT_FLAG */
    "detected spoofed probe response",						/* IW_SPOOF_PROBE_RESP_EVENT_FLAG */
    "detected spoofed beacon",								/* IW_SPOOF_BEACON_EVENT_FLAG */
    "detected spoofed disassociation",						/* IW_SPOOF_DISASSOC_EVENT_FLAG */
    "detected spoofed authentication",						/* IW_SPOOF_AUTH_EVENT_FLAG */
    "detected spoofed deauthentication",					/* IW_SPOOF_DEAUTH_EVENT_FLAG */
    "detected spoofed unknown management frame",			/* IW_SPOOF_UNKNOWN_MGMT_EVENT_FLAG */
	"detected replay attack"								/* IW_REPLAY_ATTACK_EVENT_FLAG */	
	};

// for wireless IDS_flooding_attack event message
char const *pWirelessFloodEventText[IW_FLOOD_EVENT_TYPE_NUM] = {   	
	"detected authentication flooding",						/* IW_FLOOD_AUTH_EVENT_FLAG */
    "detected association request flooding",				/* IW_FLOOD_ASSOC_REQ_EVENT_FLAG */
    "detected reassociation request flooding",				/* IW_FLOOD_REASSOC_REQ_EVENT_FLAG */
    "detected probe request flooding",						/* IW_FLOOD_PROBE_REQ_EVENT_FLAG */
    "detected disassociation flooding",						/* IW_FLOOD_DISASSOC_EVENT_FLAG */
    "detected deauthentication flooding",					/* IW_FLOOD_DEAUTH_EVENT_FLAG */
    "detected 802.1x eap-request flooding"					/* IW_FLOOD_EAP_REQ_EVENT_FLAG */	
	};
#endif // IDS_SUPPORT //

#ifdef WSC_INCLUDED
// for WSC wireless event message
char const *pWirelessWscEventText[IW_WSC_EVENT_TYPE_NUM] = {   	
	"PBC Session Overlap",									/* IW_WSC_PBC_SESSION_OVERLAP */
	"This WPS Registrar supports PBC",						/* IW_WSC_REGISTRAR_SUPPORT_PBC */
	"This WPS Registrar supports PIN",						/* IW_WSC_REGISTRAR_SUPPORT_PIN */
	"WPS status success",									/* IW_WSC_STATUS_SUCCESS */
	"WPS status fail",										/* IW_WSC_STATUS_FAIL */
	"WPS 2 mins time out!",									/* IW_WSC_2MINS_TIMEOUT */
	"WPS Send EAPOL_Start!",								/* IW_WSC_SEND_EAPOL_START */
	"WPS Send WscStart!",									/* IW_WSC_SEND_WSC_START */
	"WPS Send M1!",											/* IW_WSC_SEND_M1 */
	"WPS Send M2!",											/* IW_WSC_SEND_M2 */
	"WPS Send M3!",											/* IW_WSC_SEND_M3 */
	"WPS Send M4!",											/* IW_WSC_SEND_M4 */
	"WPS Send M5!",											/* IW_WSC_SEND_M5 */
	"WPS Send M6!",											/* IW_WSC_SEND_M6 */
	"WPS Send M7!",											/* IW_WSC_SEND_M7 */
	"WPS Send M8!",											/* IW_WSC_SEND_M8 */
	"WPS Send WscDone!",									/* IW_WSC_SEND_DONE */
	"WPS Send WscAck!",										/* IW_WSC_SEND_ACK */
	"WPS Send WscNack!",									/* IW_WSC_SEND_NACK */
	"WPS Receive WscStart!",								/* IW_WSC_RECEIVE_WSC_START */
	"WPS Receive M1!",										/* IW_WSC_RECEIVE_M1 */
	"WPS Receive M2!",										/* IW_WSC_RECEIVE_M2 */
	"WPS Receive M3!",										/* IW_WSC_RECEIVE_M3 */
	"WPS Receive M4!",										/* IW_WSC_RECEIVE_M4 */
	"WPS Receive M5!",										/* IW_WSC_RECEIVE_M5 */
	"WPS Receive M6!",										/* IW_WSC_RECEIVE_M6 */
	"WPS Receive M7!",										/* IW_WSC_RECEIVE_M7 */
	"WPS Receive M8!",										/* IW_WSC_RECEIVE_M8 */
	"WPS Receive WscDone!",									/* IW_WSC_RECEIVE_DONE */
	"WPS Receive WscAck!",									/* IW_WSC_RECEIVE_ACK */
	"WPS Receive WscNack!",									/* IW_WSC_RECEIVE_NACK */
	"Not only one candidate found"							/* IW_WSC_MANY_CANDIDATE */
	};
#endif // WSC_INCLUDED //
#endif // SYSTEM_LOG_SUPPORT //

/* timeout -- ms */
VOID RTMP_SetPeriodicTimer(
	IN	NDIS_MINIPORT_TIMER *pTimer, 
	IN	unsigned long timeout)
{
	timeout = ((timeout*OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
VOID RTMP_OS_Init_Timer(
	IN	PRTMP_ADAPTER pAd,
	IN	NDIS_MINIPORT_TIMER *pTimer, 
	IN	TIMER_FUNCTION function,
	IN	PVOID data)
{
	if (!timer_pending(pTimer))
	{
		init_timer(pTimer);
		pTimer->data = (unsigned long)data;
		pTimer->function = function;		
	}
}


VOID RTMP_OS_Add_Timer(
	IN	NDIS_MINIPORT_TIMER		*pTimer,
	IN	unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout*OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

VOID RTMP_OS_Mod_Timer(
	IN	NDIS_MINIPORT_TIMER		*pTimer,
	IN	unsigned long timeout)
{
	timeout = ((timeout*OS_HZ) / 1000);
	mod_timer(pTimer, jiffies + timeout);
}

VOID RTMP_OS_Del_Timer(
	IN	NDIS_MINIPORT_TIMER		*pTimer,
	OUT	BOOLEAN					*pCancelled)
{
	if (timer_pending(pTimer))
	{	
		*pCancelled = del_timer_sync(pTimer);	
	}
	else
	{
		*pCancelled = TRUE;
	}
	
}

	
// Unify all delay routine by using udelay
VOID RTMPusecDelay(
	IN	ULONG	usec)
{
	ULONG	i;

	for (i = 0; i < (usec / 50); i++)
		udelay(50);

	if (usec % 50)
		udelay(usec % 50);
}

VOID RtmpOsMsDelay(
	IN ULONG msec)
{
	mdelay(msec);
}

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time)
{
	time->u.LowPart = jiffies;
}

// pAd MUST allow to be NULL
NDIS_STATUS os_alloc_mem(
	IN	RTMP_ADAPTER *pAd,
	OUT	UCHAR **mem,
	IN	ULONG  size)
{	
	*mem = (PUCHAR) kmalloc(size, GFP_ATOMIC);
	if (*mem)
	{
#ifdef VENDOR_FEATURE4_SUPPORT
		OS_NumOfMemAlloc ++;
#endif // VENDOR_FEATURE4_SUPPORT //

		return (NDIS_STATUS_SUCCESS);
	}
	else
		return (NDIS_STATUS_FAILURE);
}

// pAd MUST allow to be NULL
NDIS_STATUS os_free_mem(
	IN	PRTMP_ADAPTER pAd,
	IN	PVOID mem)
{
	ASSERT(mem);
	kfree(mem);

#ifdef VENDOR_FEATURE4_SUPPORT
	OS_NumOfMemFree ++;
#endif // VENDOR_FEATURE4_SUPPORT //

	return (NDIS_STATUS_SUCCESS);
}


#if defined(RTMP_RBUS_SUPPORT) || defined (RTMP_FLASH_SUPPORT)

#ifdef RA_MTD_RW_BY_NUM
#if defined (CONFIG_RT2880_FLASH_32M)
#define MTD_NUM_FACTORY 5
#else
#define MTD_NUM_FACTORY 2
#endif
extern int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);
#else
extern int ra_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);
#endif

void RtmpFlashRead(UCHAR * p, ULONG a, ULONG b)
{
#ifdef RA_MTD_RW_BY_NUM
	ra_mtd_read(MTD_NUM_FACTORY, 0, (size_t)b, p);
#else
	ra_mtd_read_nm("Factory", 0, (size_t)b, p);
#endif
}

void RtmpFlashWrite(UCHAR * p, ULONG a, ULONG b)
{
#ifdef RA_MTD_RW_BY_NUM
	ra_mtd_write(MTD_NUM_FACTORY, 0, (size_t)b, p);
#else
	ra_mtd_write_nm("Factory", 0, (size_t)b, p);
#endif
}
#endif // defined(RTMP_RBUS_SUPPORT) || defined (RTMP_FLASH_SUPPORT) //


PNDIS_PACKET RtmpOSNetPktAlloc(
	IN RTMP_ADAPTER *pAd, 
	IN int size)
{
	struct sk_buff *skb;
	/* Add 2 more bytes for ip header alignment*/
	skb = dev_alloc_skb(size+2);
	if (skb != NULL)
	{
		MEM_DBG_PKT_ALLOC_INC(pAd);
	}
	
	return ((PNDIS_PACKET)skb);
}


PNDIS_PACKET RTMP_AllocateFragPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length)
{
	struct sk_buff *pkt;

	pkt = dev_alloc_skb(Length);

	if (pkt == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("can't allocate frag rx %ld size packet\n",Length));
	}

	if (pkt)
	{
		MEM_DBG_PKT_ALLOC_INC(pAd);
	}

	return (PNDIS_PACKET) pkt;
}

VOID	RTMPFreeAdapter(
	IN	PRTMP_ADAPTER	pAd)
{
	POS_COOKIE os_cookie;
	int index;	

	os_cookie=(POS_COOKIE)pAd->OS_Cookie;

	if (pAd->BeaconBuf)
		kfree(pAd->BeaconBuf);


	NdisFreeSpinLock(&pAd->MgmtRingLock);
	
#ifdef RTMP_MAC_PCI 
	NdisFreeSpinLock(&pAd->RxRingLock);
#endif // RTMP_MAC_PCI //

	for (index =0 ; index < NUM_OF_TX_RING; index++)
	{
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);
		pAd->DeQueueRunning[index] = FALSE;
	}
	
	NdisFreeSpinLock(&pAd->irq_lock);

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	NdisFreeSpinLock(&pAd->ShrMemLock);
#endif // SPECIFIC_BCN_BUF_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_AP_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock); // OS_ABL_SUPPORT
#endif // UAPSD_AP_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

	vfree(pAd); // pci_free_consistent(os_cookie->pci_dev,sizeof(RTMP_ADAPTER),pAd,os_cookie->pAd_pa);
	if (os_cookie)
		kfree(os_cookie);
}


// the allocated NDIS PACKET must be freed via RTMPFreeNdisPacket()
NDIS_STATUS RTMPAllocateNdisPacket(
	IN	PRTMP_ADAPTER	pAd,
	OUT PNDIS_PACKET   *ppPacket,
	IN	PUCHAR			pHeader,
	IN	UINT			HeaderLen,
	IN	PUCHAR			pData,
	IN	UINT			DataLen)
{
	PNDIS_PACKET	pPacket;
	ASSERT(pData);
	ASSERT(DataLen);

	// 1. Allocate a packet 
	pPacket = (PNDIS_PACKET *) dev_alloc_skb(HeaderLen + DataLen + RTMP_PKT_TAIL_PADDING);

	if (pPacket == NULL)
 	{
		*ppPacket = NULL;
#ifdef DEBUG
		printk("RTMPAllocateNdisPacket Fail\n\n");
#endif
		return NDIS_STATUS_FAILURE;
	}
	MEM_DBG_PKT_ALLOC_INC(pAd);

	// 2. clone the frame content
	if (HeaderLen > 0)
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket), pHeader, HeaderLen);
	if (DataLen > 0)
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket) + HeaderLen, pData, DataLen);

	// 3. update length of packet
 	skb_put(GET_OS_PKT_TYPE(pPacket), HeaderLen+DataLen);

//	printk("%s : pPacket = %p, len = %d\n", __FUNCTION__, pPacket, GET_OS_PKT_LEN(pPacket));
	*ppPacket = pPacket;
	return NDIS_STATUS_SUCCESS;
}

/*
  ========================================================================
  Description:
	This routine frees a miniport internally allocated NDIS_PACKET and its
	corresponding NDIS_BUFFER and allocated memory.
  ========================================================================
*/
VOID RTMPFreeNdisPacket(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET  pPacket)
{
	dev_kfree_skb_any(RTPKT_TO_OSPKT(pPacket));
	MEM_DBG_PKT_FREE_INC(pAd);
}


// IRQL = DISPATCH_LEVEL
// NOTE: we do have an assumption here, that Byte0 and Byte1 always reasid at the same 
//			 scatter gather buffer
NDIS_STATUS Sniff2BytesFromNdisBuffer(
	IN	PNDIS_BUFFER	pFirstBuffer,
	IN	UCHAR			DesiredOffset,
	OUT PUCHAR			pByte0,
	OUT PUCHAR			pByte1)
{
    *pByte0 = *(PUCHAR)(pFirstBuffer + DesiredOffset);
    *pByte1 = *(PUCHAR)(pFirstBuffer + DesiredOffset + 1);

	return NDIS_STATUS_SUCCESS;
}


void RTMP_QueryPacketInfo(
	IN  PNDIS_PACKET pPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen)
{
	pPacketInfo->BufferCount = 1;
	pPacketInfo->pFirstBuffer = (PNDIS_BUFFER)GET_OS_PKT_DATAPTR(pPacket);
	pPacketInfo->PhysicalBufferCount = 1;
	pPacketInfo->TotalPacketLength = GET_OS_PKT_LEN(pPacket);

	*pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
	*pSrcBufLen = GET_OS_PKT_LEN(pPacket); 	
}

	
PNDIS_PACKET DuplicatePacket(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID)
{
	struct sk_buff	*skb;
	PNDIS_PACKET	pRetPacket = NULL;
	USHORT			DataSize;
	UCHAR			*pData;

	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);
	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);	


	skb = skb_clone(RTPKT_TO_OSPKT(pPacket), MEM_ALLOC_FLAG);
	if (skb)
	{
		MEM_DBG_PKT_ALLOC_INC(pAd);
		skb->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
		pRetPacket = OSPKT_TO_RTPKT(skb);
	}


	return pRetPacket;

}

PNDIS_PACKET duplicate_pkt(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID)
{
	struct sk_buff	*skb;
	PNDIS_PACKET	pPacket = NULL;

	if ((skb = __dev_alloc_skb(HdrLen + DataSize + 2, MEM_ALLOC_FLAG)) != NULL)
	{
		MEM_DBG_PKT_ALLOC_INC(pAd);

		skb_reserve(skb, 2);				
		NdisMoveMemory(skb->tail, pHeader802_3, HdrLen);
		skb_put(skb, HdrLen);
		NdisMoveMemory(skb->tail, pData, DataSize);
		skb_put(skb, DataSize);
		skb->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
		pPacket = OSPKT_TO_RTPKT(skb);
	}

	return pPacket;
}


#define TKIP_TX_MIC_SIZE		8
PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{
	struct sk_buff	*skb, *newskb;
	

	skb = RTPKT_TO_OSPKT(pPacket);
	if (skb_tailroom(skb) < TKIP_TX_MIC_SIZE)
	{
		// alloc a new skb and copy the packet
		newskb = skb_copy_expand(skb, skb_headroom(skb), TKIP_TX_MIC_SIZE, GFP_ATOMIC);
		MEM_DBG_PKT_ALLOC_INC(pAd);

		dev_kfree_skb_any(skb);
		MEM_DBG_PKT_FREE_INC(pAd);

		if (newskb == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Extend Tx.MIC for packet failed!, dropping packet!\n"));
			return NULL;
		}
		skb = newskb;
	}

	return OSPKT_TO_RTPKT(skb);


}


#ifdef CONFIG_AP_SUPPORT
PNDIS_PACKET duplicate_pkt_with_VLAN(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID,
	IN	UCHAR			*TPID)
{
	struct sk_buff	*skb;
	PNDIS_PACKET	pPacket = NULL;
	UINT16			VLAN_Size;


	if ((skb = __dev_alloc_skb(HdrLen + DataSize + LENGTH_802_1Q + 2, \
								MEM_ALLOC_FLAG)) != NULL)
	{
		MEM_DBG_PKT_ALLOC_INC(pAd);

		skb_reserve(skb, 2);

		/* copy header (maybe +VLAN tag) */
		VLAN_Size = VLAN_8023_Header_Copy(pAd, pHeader802_3, HdrLen,
											skb->tail, FromWhichBSSID, TPID);
		skb_put(skb, HdrLen + VLAN_Size);

		/* copy data body */
		NdisMoveMemory(skb->tail, pData, DataSize);
		skb_put(skb, DataSize);
		skb->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
		pPacket = OSPKT_TO_RTPKT(skb);
	} /* End of if */

	return pPacket;
} /* End of duplicate_pkt_with_VLAN */
#endif // CONFIG_AP_SUPPORT //

/*
	========================================================================
	
	Routine Description:
		Send a L2 frame to upper daemon to trigger state machine

	Arguments:		
		pAd			-	pointer to our pAdapter context	
  				
	Return Value:
		
	Note:
		
	========================================================================
*/
BOOLEAN RTMPL2FrameTxAction(
		IN  PRTMP_ADAPTER		pAd,
		IN	UCHAR				apidx,
		IN	PUCHAR				pData,
		IN	UINT32				data_len)
{												   
	struct sk_buff *skb = dev_alloc_skb(data_len+2);

	if (!skb)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Error! Can't allocate a skb.\n", __FUNCTION__));
		return FALSE;
	}

	MEM_DBG_PKT_ALLOC_INC(pAd);
	SET_OS_PKT_NETDEV(skb, get_netdev_from_bssid(pAd, apidx));

	/* 16 byte align the IP header */
	skb_reserve(skb, 2);

	/* Insert the frame content */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), pData, data_len);

	/* End this frame */
	skb_put(GET_OS_PKT_TYPE(skb), data_len);

	DBGPRINT(RT_DEBUG_TRACE, ("%s doen\n", __FUNCTION__));

	announce_802_3_packet(pAd, skb);

	return TRUE;
	
}	

PNDIS_PACKET ExpandPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UINT32			ext_head_len,
	IN	UINT32			ext_tail_len)
{
	struct sk_buff	*skb, *newskb;
	

	skb = RTPKT_TO_OSPKT(pPacket);
	//printk("original skb_headroom(%d)/skb_tailroom(%d)\n", skb_headroom(skb), skb_tailroom(skb));
	if (skb_cloned(skb) || (skb_headroom(skb) < ext_head_len) || (skb_tailroom(skb) < ext_tail_len))
	{
		UINT32 head_len = (skb_headroom(skb) < ext_head_len) ? ext_head_len : skb_headroom(skb);
		UINT32 tail_len = (skb_tailroom(skb) < ext_tail_len) ? ext_tail_len : skb_tailroom(skb);
	
		// alloc a new skb and copy the packet
		newskb = skb_copy_expand(skb, head_len, tail_len, GFP_ATOMIC);
		MEM_DBG_PKT_ALLOC_INC(pAd);

		dev_kfree_skb_any(skb);
		MEM_DBG_PKT_FREE_INC(pAd);

		if (newskb == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Extend Tx buffer for WPI failed!, dropping packet!\n"));
			return NULL;
		}
		skb = newskb;
		//printk("new skb_headroom(%d)/skb_tailroom(%d)\n", skb_headroom(skb), skb_tailroom(skb));
	}

	return OSPKT_TO_RTPKT(skb);
	
}

PNDIS_PACKET ClonePacket(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket,	
	IN	PUCHAR			pData,
	IN	ULONG			DataSize)
{
	struct sk_buff	*pRxPkt;
	struct sk_buff	*pClonedPkt;

	ASSERT(pPacket);
	pRxPkt = RTPKT_TO_OSPKT(pPacket);

	// clone the packet 
	pClonedPkt = skb_clone(pRxPkt, MEM_ALLOC_FLAG);

	if (pClonedPkt)
	{
    	// set the correct dataptr and data len
		MEM_DBG_PKT_ALLOC_INC(pAd);
    	pClonedPkt->dev = pRxPkt->dev;
    	pClonedPkt->data = pData;
    	pClonedPkt->len = DataSize;
    	pClonedPkt->tail = pClonedPkt->data + pClonedPkt->len;
		ASSERT(DataSize < 1530);
	}
	return pClonedPkt;
}

// 
// change OS packet DataPtr and DataLen
// 
void  update_os_packet_info(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk,
	IN  UCHAR			FromWhichBSSID)
{
	struct sk_buff	*pOSPkt;

	ASSERT(pRxBlk->pRxPacket);
	pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

	pOSPkt->dev = get_netdev_from_bssid(pAd, FromWhichBSSID); 
	pOSPkt->data = pRxBlk->pData;
	pOSPkt->len = pRxBlk->DataSize;
	pOSPkt->tail = pOSPkt->data + pOSPkt->len;
}


void wlan_802_11_to_802_3_packet(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk,
	IN	PUCHAR			pHeader802_3,
	IN  UCHAR			FromWhichBSSID,
	IN	UCHAR			*TPID)
{
	struct sk_buff	*pOSPkt;

	ASSERT(pRxBlk->pRxPacket);
	ASSERT(pHeader802_3);

	pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

	pOSPkt->dev = get_netdev_from_bssid(pAd, FromWhichBSSID); 
	pOSPkt->data = pRxBlk->pData;
	pOSPkt->len = pRxBlk->DataSize;
	pOSPkt->tail = pOSPkt->data + pOSPkt->len;

	//
	// copy 802.3 header
	// 
	// 
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* maybe insert VLAN tag to the received packet */
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;

		/* VLAN related */
		if ((FromWhichBSSID < pAd->ApCfg.BssidNum) &&
			(pAd->ApCfg.MBSSID[FromWhichBSSID].VLAN_VID != 0))
		{
			VLAN_Size = LENGTH_802_1Q;
		} /* End of if */

		data_p = skb_push(pOSPkt, LENGTH_802_3+VLAN_Size);

		VLAN_8023_Header_Copy(pAd, pHeader802_3, LENGTH_802_3,
								data_p, FromWhichBSSID, TPID);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(skb_push(pOSPkt, LENGTH_802_3), pHeader802_3, LENGTH_802_3);
#endif // CONFIG_STA_SUPPORT //
	}



void announce_802_3_packet(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PNDIS_PACKET	pPacket)
{
	struct sk_buff *pRxPkt;

	ASSERT(pPacket);
	MEM_DBG_PKT_FREE_INC(pAd);

	pRxPkt = RTPKT_TO_OSPKT(pPacket);

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (RTMP_MATPktRxNeedConvert(pAd, pRxPkt->dev))
			RTMP_MATEngineRxHandle(pAd, pPacket, 0);
	}
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
#endif // CONFIG_STA_SUPPORT //

    /* Push up the protocol stack */
#ifdef IKANOS_VX_1X0
	IKANOS_DataFrameRx(pAd, pRxPkt->dev, pRxPkt, pRxPkt->len);
	return;
#endif

#ifdef INF_PPA_SUPPORT
	if (ppa_hook_directpath_send_fn && pAd->PPAEnable==TRUE ) 
	{
		int ret = 0;
		unsigned int ppa_flags = 0; /* reserved for now */

		pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
		memset(pRxPkt->head,0,pRxPkt->data-pRxPkt->head-14);
		DBGPRINT(RT_DEBUG_TRACE, ("ppa_hook_directpath_send_fn rx :ret:%d headroom:%d dev:%s pktlen:%d<===\n",ret,skb_headroom(pRxPkt)
			,pRxPkt->dev->name,pRxPkt->len));
		hex_dump("rx packet", pRxPkt->data, 32);
		ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pRxPkt, pRxPkt->len, ppa_flags);
		return;

	}
#endif // INF_PPA_SUPPORT //

//#ifdef CONFIG_5VT_ENHANCE
//	*(int*)(pRxPkt->cb) = BRIDGE_TAG; 
//#endif

#ifdef CONFIG_RT2880_BRIDGING_ONLY
	pRxPkt->cb[22]=0xa8;
#endif // CONFIG_RT2880_BRIDGING_ONLY //

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#if !defined (CONFIG_RA_NAT_NONE)
	/*
	 * ra_sw_nat_hook_rx return 1 --> continue
	 * ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
	*/
	if (ra_sw_nat_hook_rx != NULL)
	{
		pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
		FOE_MAGIC_TAG(pRxPkt) = FOE_MAGIC_EXTIF;
		if (ra_sw_nat_hook_rx(pRxPkt))
		{
			FOE_MAGIC_TAG(pRxPkt) = 0;
			netif_rx(pRxPkt);
		}
		
		return;
	}
#endif
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	if (BG_FTPH_PacketFromApHandle(pRxPkt) == 0)
		return;
#endif // BG_FT_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
	netif_rx(pRxPkt);
}


PRTMP_SCATTER_GATHER_LIST
rt_get_sg_list_from_packet(PNDIS_PACKET pPacket, RTMP_SCATTER_GATHER_LIST *sg)
{
	sg->NumberOfElements = 1;
	sg->Elements[0].Address =  GET_OS_PKT_DATAPTR(pPacket);	
	sg->Elements[0].Length = GET_OS_PKT_LEN(pPacket);	
	return (sg);
}

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
#ifdef DBG
	unsigned char *pt;
	int x;

	if (RTDebugLevel < RT_DEBUG_TRACE)
		return;
	
	pt = pSrcBufVA;
	printk("%s: %p, len = %d\n",str,  pSrcBufVA, SrcBufLen);
	for (x=0; x<SrcBufLen; x++)
	{
		if (x % 16 == 0) 
			printk("0x%04x : ", x);
		printk("%02x ", ((unsigned char)pt[x]));
		if (x%16 == 15) printk("\n");
	}
	printk("\n");
#endif // DBG //
}

#ifdef SYSTEM_LOG_SUPPORT
/*
	========================================================================
	
	Routine Description:
		Send log message through wireless event

		Support standard iw_event with IWEVCUSTOM. It is used below.

		iwreq_data.data.flags is used to store event_flag that is defined by user. 
		iwreq_data.data.length is the length of the event log.

		The format of the event log is composed of the entry's MAC address and
		the desired log message (refer to pWirelessEventText).

			ex: 11:22:33:44:55:66 has associated successfully

		p.s. The requirement of Wireless Extension is v15 or newer. 

	========================================================================
*/
VOID RTMPSendWirelessEvent(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Event_flag,
	IN	PUCHAR 			pAddr,
	IN	UCHAR			BssIdx,
	IN	CHAR			Rssi)
{
	PSTRING	pBuf = NULL, pBufPtr = NULL;
	USHORT	event, type, BufLen;	
	UCHAR	event_table_len = 0;

	if (pAd->CommonCfg.bWirelessEvent == FALSE)
		return;

#if WIRELESS_EXT >= 15

	type = Event_flag & 0xFF00;	
	event = Event_flag & 0x00FF;

	switch (type)
	{
		case IW_SYS_EVENT_FLAG_START:
			event_table_len = IW_SYS_EVENT_TYPE_NUM;
			break;
#ifdef IDS_SUPPORT
		case IW_SPOOF_EVENT_FLAG_START:
			event_table_len = IW_SPOOF_EVENT_TYPE_NUM;
			break;

		case IW_FLOOD_EVENT_FLAG_START:
			event_table_len = IW_FLOOD_EVENT_TYPE_NUM;
			break;
#endif // IDS_SUPPORT // 			
#ifdef WSC_INCLUDED
		case IW_WSC_EVENT_FLAG_START:
			event_table_len = IW_WSC_EVENT_TYPE_NUM;
			break;
#endif // WSC_INCLUDED //
	}
	
	if (event_table_len == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The type(%0x02x) is not valid.\n", __FUNCTION__, type));			       		       		
		return;
	}
	
	if (event >= event_table_len)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The event(%0x02x) is not valid.\n", __FUNCTION__, event));			       		       		
		return;
	}	
 
	//Allocate memory and copy the msg.
	if((pBuf = kmalloc(IW_CUSTOM_MAX_LEN, GFP_ATOMIC)) != NULL)
	{
		//Prepare the payload 
		memset(pBuf, 0, IW_CUSTOM_MAX_LEN);

		pBufPtr = pBuf;		

		if (pAddr)
			pBufPtr += sprintf(pBufPtr, "(RT2860) STA(%02x:%02x:%02x:%02x:%02x:%02x) ", PRINT_MAC(pAddr));				
		else if (BssIdx < MAX_MBSSID_NUM)
			pBufPtr += sprintf(pBufPtr, "(RT2860) BSS(ra%d) ", BssIdx);
		else
			pBufPtr += sprintf(pBufPtr, "(RT2860) ");

		if (type == IW_SYS_EVENT_FLAG_START)
        {
			pBufPtr += sprintf(pBufPtr, "%s", pWirelessSysEventText[event]);
		    
            if (Event_flag == IW_CHANNEL_CHANGE_EVENT_FLAG)
		  	{
			 	pBufPtr += sprintf(pBufPtr, "%3d", Rssi);
			}			
		}
#ifdef IDS_SUPPORT		
		else if (type == IW_SPOOF_EVENT_FLAG_START)
			pBufPtr += sprintf(pBufPtr, "%s (RSSI=%d)", pWirelessSpoofEventText[event], Rssi);
		else if (type == IW_FLOOD_EVENT_FLAG_START)
			pBufPtr += sprintf(pBufPtr, "%s", pWirelessFloodEventText[event]);
#endif // IDS_SUPPORT //		
#ifdef WSC_INCLUDED
		else if (type == IW_WSC_EVENT_FLAG_START)
			pBufPtr += sprintf(pBufPtr, "%s", pWirelessWscEventText[event]);
#endif // WSC_INCLUDED //
		else
			pBufPtr += sprintf(pBufPtr, "%s", "unknown event");
		
		pBufPtr[pBufPtr - pBuf] = '\0';
		BufLen = pBufPtr - pBuf;
		
		RtmpOSWrielessEventSend(pAd, IWEVCUSTOM, Event_flag, NULL, (PUCHAR)pBuf, BufLen);
		//DBGPRINT(RT_DEBUG_TRACE, ("%s : %s\n", __FUNCTION__, pBuf));	
	
		kfree(pBuf);
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Can't allocate memory for wireless event.\n", __FUNCTION__));			       		       				
#else
	DBGPRINT(RT_DEBUG_ERROR, ("%s : The Wireless Extension MUST be v15 or newer.\n", __FUNCTION__));	
#endif  /* WIRELESS_EXT >= 15 */  
}
#endif // SYSTEM_LOG_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
VOID SendSignalToDaemon(
					   IN INT              sig,
		RTMP_OS_PID			pid,
		unsigned long		pid_no)
{
}
#endif // CONFIG_AP_SUPPORT //


#ifdef CONFIG_STA_SUPPORT
__s32 ralinkrate[] =
	{2,  4,   11,  22, // CCK
	12, 18,   24,  36, 48, 72, 96, 108, // OFDM
	13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260, // 20MHz, 800ns GI, MCS: 0 ~ 15
	39, 78,  117, 156, 234, 312, 351, 390,										  // 20MHz, 800ns GI, MCS: 16 ~ 23
	27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540, // 40MHz, 800ns GI, MCS: 0 ~ 15
	81, 162, 243, 324, 486, 648, 729, 810,										  // 40MHz, 800ns GI, MCS: 16 ~ 23
	14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288, // 20MHz, 400ns GI, MCS: 0 ~ 15
	43, 87,  130, 173, 260, 317, 390, 433,										  // 20MHz, 400ns GI, MCS: 16 ~ 23
	30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600, // 40MHz, 400ns GI, MCS: 0 ~ 15
	90, 180, 270, 360, 540, 720, 810, 900};										  // 40MHz, 400ns GI, MCS: 16 ~ 23

UINT32 RT_RateSize = sizeof(ralinkrate);

void send_monitor_packets(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RX_BLK			*pRxBlk,
	IN  CHAR			(*RTMPMaxRssi)(
			IN PRTMP_ADAPTER	pAd,
			IN CHAR				Rssi0,
			IN CHAR				Rssi1,
			IN CHAR				Rssi2),
	IN  CHAR			(*ConvertToRssi)(
			IN PRTMP_ADAPTER	pAd,
			IN CHAR				Rssi,
			IN UCHAR			RssiNumber))
{   	
    struct sk_buff	*pOSPkt;
    wlan_ng_prism2_header *ph;
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	ETHEREAL_RADIO h, *ph_11n33; // for new 11n sniffer format
#endif // MONITOR_FLAG_11N_SNIFFER_SUPPORT //
    int rate_index = 0;
    USHORT header_len = 0;
    UCHAR temp_header[40] = {0};

    
	MEM_DBG_PKT_FREE_INC(pAd);

    ASSERT(pRxBlk->pRxPacket);
    if (pRxBlk->DataSize < 10)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too small! (%d)\n", __FUNCTION__, pRxBlk->DataSize));
		goto err_free_sk_buff;
    }

    if (pRxBlk->DataSize + sizeof(wlan_ng_prism2_header) > RX_BUFFER_AGGRESIZE)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(wlan_ng_prism2_header)));
		goto err_free_sk_buff;
    }
        
    pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
	pOSPkt->dev = get_netdev_from_bssid(pAd, BSS0); 
    if (pRxBlk->pHeader->FC.Type == BTYPE_DATA)
    {
        pRxBlk->DataSize -= LENGTH_802_11;
        if ((pRxBlk->pHeader->FC.ToDs == 1) &&
            (pRxBlk->pHeader->FC.FrDs == 1))
            header_len = LENGTH_802_11_WITH_ADDR4;
        else
            header_len = LENGTH_802_11;

        // QOS
    	if (pRxBlk->pHeader->FC.SubType & 0x08)
    	{
    	    header_len += 2;
    		// Data skip QOS contorl field
    		pRxBlk->DataSize -=2;
    	}

    	// Order bit: A-Ralink or HTC+
    	if (pRxBlk->pHeader->FC.Order)
    	{
    	    header_len += 4;
			// Data skip HTC contorl field
			pRxBlk->DataSize -= 4;
    	}

        // Copy Header
        if (header_len <= 40)
            NdisMoveMemory(temp_header, pRxBlk->pData, header_len);

        // skip HW padding 
    	if (pRxBlk->RxD.L2PAD)
    	    pRxBlk->pData += (header_len + 2);
        else
            pRxBlk->pData += header_len;
    } //end if

                
	if (pRxBlk->DataSize < pOSPkt->len) {
        skb_trim(pOSPkt,pRxBlk->DataSize);
    } else {
        skb_put(pOSPkt,(pRxBlk->DataSize - pOSPkt->len));
    } //end if

    if ((pRxBlk->pData - pOSPkt->data) > 0) {
	    skb_put(pOSPkt,(pRxBlk->pData - pOSPkt->data));
	    skb_pull(pOSPkt,(pRxBlk->pData - pOSPkt->data));
    } //end if

    if (skb_headroom(pOSPkt) < (sizeof(wlan_ng_prism2_header)+ header_len)) {
        if (pskb_expand_head(pOSPkt, (sizeof(wlan_ng_prism2_header) + header_len), 0, GFP_ATOMIC)) {
	        DBGPRINT(RT_DEBUG_ERROR, ("%s : Reallocate header size of sk_buff fail!\n", __FUNCTION__));
			goto err_free_sk_buff;
	    } //end if
    } //end if

    if (header_len > 0)
        NdisMoveMemory(skb_push(pOSPkt, header_len), temp_header, header_len);

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	if ((pAd->StaCfg.BssMonitorFlag & MONITOR_FLAG_11N_SNIFFER) == 0)
#endif // MONITOR_FLAG_11N_SNIFFER_SUPPORT //
	{
	    ph = (wlan_ng_prism2_header *) skb_push(pOSPkt, sizeof(wlan_ng_prism2_header));
		NdisZeroMemory(ph, sizeof(wlan_ng_prism2_header));
	    
	    ph->msgcode		    = DIDmsg_lnxind_wlansniffrm;
		ph->msglen		    = sizeof(wlan_ng_prism2_header);
		strcpy((PSTRING) ph->devname, (PSTRING) pAd->net_dev->name);

	    ph->hosttime.did = DIDmsg_lnxind_wlansniffrm_hosttime;
		ph->hosttime.status = 0;
		ph->hosttime.len = 4;
		ph->hosttime.data = jiffies;

		ph->mactime.did = DIDmsg_lnxind_wlansniffrm_mactime;
		ph->mactime.status = 0;
		ph->mactime.len = 0;
		ph->mactime.data = 0;

	    ph->istx.did = DIDmsg_lnxind_wlansniffrm_istx;
		ph->istx.status = 0;
		ph->istx.len = 0;
		ph->istx.data = 0;
	 
	    ph->channel.did = DIDmsg_lnxind_wlansniffrm_channel;
		ph->channel.status = 0;
		ph->channel.len = 4;

	    ph->channel.data = (u_int32_t)pAd->CommonCfg.Channel;

	    ph->rssi.did = DIDmsg_lnxind_wlansniffrm_rssi;
		ph->rssi.status = 0;
		ph->rssi.len = 4;
	    ph->rssi.data = (u_int32_t)RTMPMaxRssi(pAd, ConvertToRssi(pAd, pRxBlk->pRxWI->RSSI0, RSSI_0), ConvertToRssi(pAd, pRxBlk->pRxWI->RSSI1, RSSI_1), ConvertToRssi(pAd, pRxBlk->pRxWI->RSSI2, RSSI_2));;
	            
		ph->signal.did = DIDmsg_lnxind_wlansniffrm_signal;
		ph->signal.status = 0;
		ph->signal.len = 4;
		ph->signal.data = 0; //rssi + noise;
	            
		ph->noise.did = DIDmsg_lnxind_wlansniffrm_noise;
		ph->noise.status = 0;
		ph->noise.len = 4;
		ph->noise.data = 0;

#ifdef DOT11_N_SUPPORT
	    if (pRxBlk->pRxWI->PHYMODE >= MODE_HTMIX)
	    {
	    	rate_index = 12 + ((UCHAR)pRxBlk->pRxWI->BW *24) + ((UCHAR)pRxBlk->pRxWI->ShortGI *48) + ((UCHAR)pRxBlk->pRxWI->MCS);
	    }
	    else
#endif // DOT11_N_SUPPORT //
		if (pRxBlk->pRxWI->PHYMODE == MODE_OFDM)
	    	rate_index = (UCHAR)(pRxBlk->pRxWI->MCS) + 4;
	    else 
	    	rate_index = (UCHAR)(pRxBlk->pRxWI->MCS);

	    if (rate_index < 0)
	        rate_index = 0;
	    if (rate_index > 255)
	        rate_index = 255;
	    
		ph->rate.did = DIDmsg_lnxind_wlansniffrm_rate;
		ph->rate.status = 0;
		ph->rate.len = 4;
	    ph->rate.data = ralinkrate[rate_index]; /* real rate = ralinkrate[rate_index] / 2 */

		ph->frmlen.did = DIDmsg_lnxind_wlansniffrm_frmlen;
	    ph->frmlen.status = 0;
		ph->frmlen.len = 4;
		ph->frmlen.data	= (u_int32_t)pRxBlk->DataSize;
	}
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	else
	{
		ph_11n33 = &h;
		NdisZeroMemory((unsigned char *)ph_11n33, sizeof(ETHEREAL_RADIO));

		//802.11n fields
		if (pRxBlk->pRxWI->MCS > 15)
	    	ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_3x3;    
	    	                         
	    if (pRxBlk->pRxWI->PHYMODE == MODE_HTGREENFIELD )
	    	ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_GF;

	    
	    if (pRxBlk->pRxWI->BW == 1){                                               
	    	ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW40;                 
	    }
	    else if (pAd->CommonCfg.Channel < pAd->CommonCfg.CentralChannel){
	        ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW20U;
	    }    
	    else if (pAd->CommonCfg.Channel > pAd->CommonCfg.CentralChannel){
	        ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_BW20D;
	    }    
	   	else{
	        ph_11n33->Flag_80211n |= (WIRESHARK_11N_FLAG_BW20U | WIRESHARK_11N_FLAG_BW20D);
	    }    

	    if (pRxBlk->pRxWI->ShortGI == 1)
	      	ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_SGI;

	  	if (pRxBlk->RxD.AMPDU)   // RXD_STRUC   PRT28XX_RXD_STRUC				pRxD = &(pRxBlk->RxD);
	      	ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_AMPDU;

	    if (pRxBlk->pRxWI->STBC)
	       	ph_11n33->Flag_80211n |= WIRESHARK_11N_FLAG_STBC;    

		ph_11n33->signal_level = (UCHAR)pRxBlk->pRxWI->RSSI1;

		/* data_rate is the rate index in the wireshark rate table */
	  	if (pRxBlk->pRxWI->PHYMODE >= MODE_HTMIX)
	  	{
	     	if (pRxBlk->pRxWI->MCS > 15)
	     		ph_11n33->data_rate = (16*4 + ((UCHAR)pRxBlk->pRxWI->BW *16) + ((UCHAR)pRxBlk->pRxWI->ShortGI *32) + ((UCHAR)pRxBlk->pRxWI->MCS));                          
	     	else
	     		ph_11n33->data_rate = 16 + ((UCHAR)pRxBlk->pRxWI->BW *16) + ((UCHAR)pRxBlk->pRxWI->ShortGI *32) + ((UCHAR)pRxBlk->pRxWI->MCS);
	  	}
	   	else if (pRxBlk->pRxWI->PHYMODE == MODE_OFDM)
	     	ph_11n33->data_rate = (UCHAR)(pRxBlk->pRxWI->MCS) + 4;
	   	else
	      	ph_11n33->data_rate = (UCHAR)(pRxBlk->pRxWI->MCS);
	    
	    //channel field
	    ph_11n33->channel = (UCHAR)pAd->CommonCfg.Channel;   

		NdisMoveMemory(skb_put(pOSPkt, sizeof(ETHEREAL_RADIO)), (UCHAR *)ph_11n33, sizeof(ETHEREAL_RADIO));
	}
#endif // MONITOR_FLAG_11N_SNIFFER_SUPPORT //

    pOSPkt->pkt_type = PACKET_OTHERHOST;
    pOSPkt->protocol = eth_type_trans(pOSPkt, pOSPkt->dev);
    pOSPkt->ip_summed = CHECKSUM_NONE;
    netif_rx(pOSPkt);
    
    return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);	
	return;
	
}
#endif // CONFIG_STA_SUPPORT //




/*******************************************************************************

	File open/close related functions.
	
 *******************************************************************************/
RTMP_OS_FD RtmpOSFileOpen(char *pPath,  int flag, int mode)
{
	struct file	*filePtr;
		
	filePtr = filp_open(pPath, flag, 0);
	if (IS_ERR(filePtr))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Error %ld opening %s\n", __FUNCTION__, -PTR_ERR(filePtr), pPath));
	}

	return (RTMP_OS_FD)filePtr;
}

int RtmpOSFileClose(RTMP_OS_FD osfd)
{
	filp_close(osfd, NULL);
	return 0;
}


void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset)
{
	osfd->f_pos = offset;
}


int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen)
{
	// The object must have a read method
	if (osfd->f_op && osfd->f_op->read)
	{
		return osfd->f_op->read(osfd,  pDataPtr, readLen, &osfd->f_pos);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("no file read method\n"));
		return -1;
	}
}


int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen)
{
	return osfd->f_op->write(osfd, pDataPtr, (size_t)writeLen, &osfd->f_pos);
}


void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfo, BOOLEAN bSet)
{
	if (bSet)
	{
		// Save uid and gid used for filesystem access.
		// Set user and group to 0 (root)	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		pOSFSInfo->fsuid= current->fsuid;
		pOSFSInfo->fsgid = current->fsgid;
		current->fsuid = current->fsgid = 0;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
		pOSFSInfo->fsuid = __kuid_val(current_fsuid());
		pOSFSInfo->fsgid = __kgid_val(current_fsgid());
#else
		pOSFSInfo->fsuid = current_fsuid();
		pOSFSInfo->fsgid = current_fsgid();
#endif
		pOSFSInfo->fs = get_fs();
		set_fs(KERNEL_DS);
	}
	else
	{
		set_fs(pOSFSInfo->fs);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		current->fsuid = pOSFSInfo->fsuid;
		current->fsgid = pOSFSInfo->fsgid;
#endif
	}
}



/*******************************************************************************

	Task create/management/kill related functions.
	
 *******************************************************************************/
NDIS_STATUS RtmpOSTaskKill(
	IN RTMP_OS_TASK *pTask)
{
	RTMP_ADAPTER *pAd;
	int ret = NDIS_STATUS_FAILURE;

	pAd = (RTMP_ADAPTER *)pTask->priv;

#ifdef KTHREAD_SUPPORT
	if (pTask->kthread_task)
	{
		kthread_stop(pTask->kthread_task);
		ret = NDIS_STATUS_SUCCESS;
	}
#else
	CHECK_PID_LEGALITY(pTask->taskPID)
	{
		DBGPRINT(RT_DEBUG_TRACE,
				("Terminate the task(%s) with pid(%d)!\n", pTask->taskName, GET_PID_NUMBER(pTask->taskPID)));
		mb();
		pTask->task_killed = 1;
		mb();
		ret = KILL_THREAD_PID(pTask->taskPID, SIGTERM, 1);
		if (ret)
		{
			printk(KERN_WARNING "kill task(%s) with pid(%d) failed(retVal=%d)!\n", 
				pTask->taskName, GET_PID_NUMBER(pTask->taskPID), ret);
		}
		else 
		{
			wait_for_completion(&pTask->taskComplete);
			pTask->taskPID = THREAD_PID_INIT_VALUE;
			pTask->task_killed = 0;
			RTMP_SEM_EVENT_DESTORY(&pTask->taskSema);
			ret = NDIS_STATUS_SUCCESS;
		}
	}
#endif

	return ret;
	
}


INT RtmpOSTaskNotifyToExit(
	IN RTMP_OS_TASK *pTask)
{

#ifndef KTHREAD_SUPPORT
	complete_and_exit(&pTask->taskComplete, 0);
#endif
	
	return 0;
}


void RtmpOSTaskCustomize(
	IN RTMP_OS_TASK *pTask)
{

#ifndef KTHREAD_SUPPORT

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	daemonize((PSTRING)&pTask->taskName[0]/*"%s",pAd->net_dev->name*/);

	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	current->flags |= PF_NOFREEZE;
#else
	unsigned long flags;

	daemonize();
	reparent_to_init();
	strcpy(current->comm, &pTask->taskName[0]);

	siginitsetinv(&current->blocked, sigmask(SIGTERM) | sigmask(SIGKILL));	
	
	/* Allow interception of SIGKILL only
	 * Don't allow other signals to interrupt the transmission */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
	spin_lock_irqsave(&current->sigmask_lock, flags);
	flush_signals(current);
	recalc_sigpending(current);
	spin_unlock_irqrestore(&current->sigmask_lock, flags);
#endif
#endif
	
	RTMP_GET_OS_PID(pTask->taskPID, current->pid);

    /* signal that we've started the thread */
	complete(&pTask->taskComplete);

#endif
}


NDIS_STATUS RtmpOSTaskAttach(
	IN RTMP_OS_TASK *pTask,
	IN RTMP_OS_TASK_CALLBACK fn, 
	IN ULONG arg)
{	
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
#ifndef KTHREAD_SUPPORT
	pid_t pid_number = -1;
#endif // KTHREAD_SUPPORT //
	
#ifdef KTHREAD_SUPPORT
	pTask->task_killed = 0;
	pTask->kthread_task = NULL;
	pTask->kthread_task = kthread_run(fn, (void *)arg, pTask->taskName);
	if (IS_ERR(pTask->kthread_task))
		status = NDIS_STATUS_FAILURE;
#else
	pid_number = kernel_thread(fn, (void *)arg, RTMP_OS_MGMT_TASK_FLAGS);
	if (pid_number < 0) 
	{
		DBGPRINT (RT_DEBUG_ERROR, ("Attach task(%s) failed!\n", pTask->taskName));
		status = NDIS_STATUS_FAILURE;
	}
	else
	{
		// Wait for the thread to start
		wait_for_completion(&pTask->taskComplete);
		status = NDIS_STATUS_SUCCESS;
	}
#endif
	return status;
}


NDIS_STATUS RtmpOSTaskInit(
	IN RTMP_OS_TASK *pTask,
	IN PSTRING		pTaskName,
	IN VOID			*pPriv)
{
	int len;

	ASSERT(pTask);

#ifndef KTHREAD_SUPPORT
	NdisZeroMemory((PUCHAR)(pTask), sizeof(RTMP_OS_TASK));
#endif

	len = strlen(pTaskName);
	len = len > (RTMP_OS_TASK_NAME_LEN -1) ? (RTMP_OS_TASK_NAME_LEN-1) : len;
	NdisMoveMemory(&pTask->taskName[0], pTaskName, len);
	pTask->priv = pPriv;

#ifndef KTHREAD_SUPPORT
	RTMP_SEM_EVENT_INIT_LOCKED(&(pTask->taskSema));
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	
	init_completion (&pTask->taskComplete);
#endif

#ifdef KTHREAD_SUPPORT	
	init_waitqueue_head(&(pTask->kthread_q));
#endif /* KTHREAD_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


void RTMP_IndicateMediaState(
	IN	PRTMP_ADAPTER	pAd)
{
#ifdef SYSTEM_LOG_SUPPORT
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
		else
		{							
			RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0); 		
		}
#endif // SYSTEM_LOG_SUPPORT //
}


#if LINUX_VERSION_CODE <= 0x20402	// Red Hat 7.1
//static struct net_device *alloc_netdev(int sizeof_priv, const char *mask, void (*setup)(struct net_device *)) //sample
struct net_device *alloc_netdev(
	int sizeof_priv,
	const char *mask,
	void (*setup)(struct net_device *))
{
    struct net_device	*dev;
    INT					alloc_size;


    /* ensure 32-byte alignment of the private area */
    alloc_size = sizeof (*dev) + sizeof_priv + 31;

    dev = (struct net_device *) kmalloc(alloc_size, GFP_KERNEL);
    if (dev == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR,
				("alloc_netdev: Unable to allocate device memory.\n"));
        return NULL;
    }

    memset(dev, 0, alloc_size);

    if (sizeof_priv)
        dev->priv = (void *) (((long)(dev + 1) + 31) & ~31);

    setup(dev);
    strcpy(dev->name, mask);

    return dev;
}
#endif // LINUX_VERSION_CODE //


int RtmpOSWrielessEventSend(
	IN RTMP_ADAPTER *pAd,
	IN UINT32		eventType,
	IN INT			flags,
	IN PUCHAR		pSrcMac,
	IN PUCHAR		pData,
	IN UINT32		dataLen)
{
	union iwreq_data    wrqu;
	
       memset(&wrqu, 0, sizeof(wrqu));
	   
	if (flags > -1)
	       wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);
	
	if ((pData!= NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;
	
       wireless_send_event(pAd->net_dev, eventType, &wrqu, (char *)pData);
	return 0;
}


int RtmpOSNetDevAddrSet(
	IN PNET_DEV pNetDev,
	IN PUCHAR	pMacAddr)
{
	struct net_device *net_dev;
	RTMP_ADAPTER *pAd;
		
	net_dev = pNetDev;
	GET_PAD_FROM_NET_DEV(pAd, net_dev);	
	
#ifdef CONFIG_STA_SUPPORT
	// work-around for the SuSE due to it has it's own interface name management system.
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		NdisZeroMemory(pAd->StaCfg.dev_name, 16);
		NdisMoveMemory(pAd->StaCfg.dev_name, net_dev->name, strlen(net_dev->name));
	}
#endif // CONFIG_STA_SUPPORT //

	NdisMoveMemory(net_dev->dev_addr, pMacAddr, 6);

	return 0;
}



/*
  *	Assign the network dev name for created Ralink WiFi interface.
  */
static int RtmpOSNetDevRequestName(
	IN RTMP_ADAPTER *pAd, 
	IN PNET_DEV dev, 
	IN PSTRING pPrefixStr, 
	IN INT	devIdx)
{
	PNET_DEV		existNetDev;
	STRING		suffixName[IFNAMSIZ];
	STRING		desiredName[IFNAMSIZ];
	int	ifNameIdx, prefixLen, slotNameLen;
	int Status;
	

	prefixLen = strlen(pPrefixStr);
	ASSERT((prefixLen < IFNAMSIZ));
	
	for (ifNameIdx = devIdx; ifNameIdx < 32; ifNameIdx++)
	{
		memset(suffixName, 0, IFNAMSIZ);
		memset(desiredName, 0, IFNAMSIZ);
		strncpy(&desiredName[0], pPrefixStr, prefixLen);
		
#ifdef MULTIPLE_CARD_SUPPORT
		if (pAd->MC_RowID >= 0)
			sprintf(suffixName, "%02d_%d", pAd->MC_RowID, ifNameIdx);
		else
#endif // MULTIPLE_CARD_SUPPORT //
		sprintf(suffixName, "%d", ifNameIdx);

		slotNameLen = strlen(suffixName);
		ASSERT(((slotNameLen + prefixLen) < IFNAMSIZ));
		strcat(desiredName, suffixName);
		
		existNetDev = RtmpOSNetDevGetByName(dev, &desiredName[0]);
		if (existNetDev == NULL) 
			break;
		else
			RtmpOSNetDeviceRefPut(existNetDev);
	}
	
	if(ifNameIdx < 32)
	{
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF=ifNameIdx;
#endif//HOSTAPD_SUPPORT//
		strcpy(&dev->name[0], &desiredName[0]);
		Status = NDIS_STATUS_SUCCESS;
	} 
	else 
	{
		DBGPRINT(RT_DEBUG_ERROR, 
					("Cannot request DevName with preifx(%s) and in range(0~32) as suffix from OS!\n", pPrefixStr));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


void RtmpOSNetDevClose(
	IN PNET_DEV pNetDev)
{
	dev_close(pNetDev);
}


void RtmpOSNetDevFree(PNET_DEV pNetDev)
{
	DEV_PRIV_INFO *pDevInfo = NULL;
	ASSERT(pNetDev);
	
	/* free assocaited private information */
	pDevInfo = _RTMP_OS_NETDEV_GET_PRIV(pNetDev);
	if (pDevInfo != NULL)
		os_free_mem(NULL, pDevInfo);
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	free_netdev(pNetDev);
#else
	kfree(pNetDev);
#endif
}


INT RtmpOSNetDevAlloc(
	IN PNET_DEV *new_dev_p,
	IN UINT32	privDataSize)
{
	// assign it as null first.
	*new_dev_p = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("Allocate a net device with private data size=%d!\n", privDataSize));
#if LINUX_VERSION_CODE <= 0x20402 // Red Hat 7.1
	*new_dev_p = alloc_netdev(privDataSize, "eth%d", ether_setup);
#else
	*new_dev_p = alloc_etherdev(privDataSize);
#endif // LINUX_VERSION_CODE //

	if (*new_dev_p)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
INT RtmpOSNetDevOpsAlloc(
	IN PVOID *pNetDevOps)
{
	*pNetDevOps = (PVOID)vmalloc(sizeof(struct net_device_ops));
	if (*pNetDevOps)
	{
		NdisZeroMemory(*pNetDevOps,sizeof(struct net_device_ops)); 
		return NDIS_STATUS_SUCCESS;
	}
	else
	{
		return NDIS_STATUS_FAILURE;
	}
}
#endif
PNET_DEV RtmpOSNetDevGetByName(PNET_DEV pNetDev, PSTRING pDevName)
{
	PNET_DEV	pTargetNetDev = NULL;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	pTargetNetDev = dev_get_by_name(dev_net(pNetDev), pDevName);
#else
	ASSERT(pNetDev);
	pTargetNetDev = dev_get_by_name(pNetDev->nd_net, pDevName);
#endif
#else
	pTargetNetDev = dev_get_by_name(pDevName);
#endif // KERNEL_VERSION(2,6,24) //

#else
	int	devNameLen;

	devNameLen = strlen(pDevName);
	ASSERT((devNameLen <= IFNAMSIZ));
	
#ifndef for_each_netdev
	for(pTargetNetDev=dev_base; pTargetNetDev!=NULL; pTargetNetDev=pTargetNetDev->next)
#else
        for_each_netdev(pTargetNetDev) {
#endif
	{
		if (strncmp(pTargetNetDev->name, pDevName, devNameLen) == 0)
			break;
	}
#endif // KERNEL_VERSION(2,5,0) //

	return pTargetNetDev;
}


void RtmpOSNetDeviceRefPut(PNET_DEV pNetDev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	/* 
		every time dev_get_by_name is called, and it has returned a valid struct 
		net_device*, dev_put should be called afterwards, because otherwise the 
		machine hangs when the device is unregistered (since dev->refcnt > 1).
	*/
	if(pNetDev)
		dev_put(pNetDev);
#endif // LINUX_VERSION_CODE //
}


INT RtmpOSNetDevDestory(
	IN RTMP_ADAPTER *pAd,
	IN PNET_DEV		pNetDev)
{

	// TODO: Need to fix this
	printk("WARNING: This function(%s) not implement yet!!!\n", __FUNCTION__);
	return 0;
}


void RtmpOSNetDevDetach(PNET_DEV pNetDev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;
#endif

	unregister_netdev(pNetDev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	vfree(pNetDevOps);
#endif
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static void RALINK_ET_DrvInfoGet(
	IN struct net_device *pDev,
	IN struct ethtool_drvinfo *pInfo)
{
	strcpy(pInfo->driver, "RALINK WLAN");

#ifdef CONFIG_AP_SUPPORT
	strcpy(pInfo->version, AP_DRIVER_VERSION);
#endif // CONFIG_AP_SUPPORT //
#ifdef CONFIG_STA_SUPPORT
	strcpy(pInfo->version, STA_DRIVER_VERSION);
#endif // CONFIG_STA_SUPPORT //

	sprintf(pInfo->bus_info, "CSR 0x%lx", pDev->base_addr);
}

static struct ethtool_ops RALINK_Ethtool_Ops = {
	.get_drvinfo		= RALINK_ET_DrvInfoGet,
};
#endif

int RtmpOSNetDevAttach(
	IN PNET_DEV pNetDev, 
	IN RTMP_OS_NETDEV_OP_HOOK *pDevOpHook)
{	
	int ret, rtnl_locked = FALSE;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("RtmpOSNetDevAttach()--->\n"));

	// If we need hook some callback function to the net device structrue, now do it.
	if (pDevOpHook)
	{
		PRTMP_ADAPTER pAd = NULL;
	
		GET_PAD_FROM_NET_DEV(pAd, pNetDev);	
		
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
		pNetDevOps->ndo_open			= pDevOpHook->open;
		pNetDevOps->ndo_stop			= pDevOpHook->stop;
		pNetDevOps->ndo_start_xmit	= (HARD_START_XMIT_FUNC)(pDevOpHook->xmit);
		pNetDevOps->ndo_do_ioctl		= pDevOpHook->ioctl;
#else
		pNetDev->open			= pDevOpHook->open;
		pNetDev->stop			= pDevOpHook->stop;
		pNetDev->hard_start_xmit	= (HARD_START_XMIT_FUNC)(pDevOpHook->xmit);
		pNetDev->do_ioctl		= pDevOpHook->ioctl;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
		pNetDev->ethtool_ops = &RALINK_Ethtool_Ops;
#endif
		
		/* if you don't implement get_stats, just leave the callback function as NULL, a dummy 
		     function will make kernel panic.
		*/
		if (pDevOpHook->get_stats)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
			pNetDevOps->ndo_get_stats = pDevOpHook->get_stats;
#else
			pNetDev->get_stats = pDevOpHook->get_stats;
#endif

		/* OS specific flags, here we used to indicate if we are virtual interface */
		RT_DEV_PRIV_FLAGS_SET(pNetDev, pDevOpHook->priv_flags);

#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
//		pNetDev->get_wireless_stats = rt28xx_get_wireless_stats;
		pNetDev->get_wireless_stats = pDevOpHook->get_wstats;
#endif

#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12
		if (pAd->OpMode == OPMODE_STA)
		{
//			pNetDev->wireless_handlers = &rt28xx_iw_handler_def;
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;
		}
#endif //WIRELESS_EXT >= 12
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12
		if (pAd->OpMode == OPMODE_AP)
		{
//			pNetDev->wireless_handlers = &rt28xx_ap_iw_handler_def;
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;
		}
#endif //WIRELESS_EXT >= 12
#endif // CONFIG_APSTA_MIXED_SUPPORT //

		// copy the net device mac address to the net_device structure.
		NdisMoveMemory(pNetDev->dev_addr, &pDevOpHook->devAddr[0], MAC_ADDR_LEN);

		rtnl_locked = pDevOpHook->needProtcted;

#ifdef RT_CFG80211_SUPPORT
		/*
			In 2.6.32, cfg80211 register must be before register_netdevice();
			We can not put the register in rt28xx_open();
			Or you will suffer NULL pointer in list_add of
			cfg80211_netdev_notifier_call().
		*/
		CFG80211_Register(pAd, pAd->pCfgDev, pNetDev);
#endif // RT_CFG80211_SUPPORT //
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	pNetDevOps->ndo_validate_addr = NULL;
	//pNetDev->netdev_ops = ops;
#else
	pNetDev->validate_addr = NULL;
#endif
#endif

	if (rtnl_locked)
		ret = register_netdevice(pNetDev);
	else
		ret = register_netdev(pNetDev);

	netif_stop_queue(pNetDev);

	DBGPRINT(RT_DEBUG_TRACE, ("<---RtmpOSNetDevAttach(), ret=%d\n", ret));
	if (ret == 0)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


PNET_DEV RtmpOSNetDevCreate(
	IN RTMP_ADAPTER *pAd,
	IN INT 			devType, 
	IN INT			devNum,
	IN INT			privMemSize,
	IN PSTRING		pNamePrefix)
{
	struct net_device *pNetDev = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	struct net_device_ops *pNetDevOps = NULL;
#endif
	int status;


	/* allocate a new network device */
	status = RtmpOSNetDevAlloc(&pNetDev, 0 /*privMemSize*/);
	if (status != NDIS_STATUS_SUCCESS)
	{
		/* allocation fail, exit */
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (%s)...\n", pNamePrefix));
		return NULL;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
	status = RtmpOSNetDevOpsAlloc((PVOID)&pNetDevOps);
	if (status != NDIS_STATUS_SUCCESS)
	{
		/* error! no any available ra name can be used! */
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate net device ops fail!\n"));
		RtmpOSNetDevFree(pNetDev);
		
		return NULL;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate net device ops success!\n"));
		pNetDev->netdev_ops = pNetDevOps;
	}
#endif
	/* find a available interface name, max 32 interfaces */
	status = RtmpOSNetDevRequestName(pAd, pNetDev, pNamePrefix, devNum);
	if (status != NDIS_STATUS_SUCCESS)
	{
		/* error! no any available ra name can be used! */
		DBGPRINT(RT_DEBUG_ERROR, ("Assign interface name (%s with suffix 0~32) failed...\n", pNamePrefix));
		RtmpOSNetDevFree(pNetDev);
		
		return NULL;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("The name of the new %s interface is %s...\n", pNamePrefix, pNetDev->name));
	}

	return pNetDev;
}


// OS_ABL_SUPPORT
// not yet support MBSS
PNET_DEV get_netdev_from_bssid(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			FromWhichBSSID)
{
	PNET_DEV dev_p = NULL;

	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		UCHAR infRealIdx;

		infRealIdx = FromWhichBSSID & (NET_DEVICE_REAL_IDX_MASK);
#ifdef APCLI_SUPPORT
		if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_APCLI)
		{
			dev_p = (infRealIdx > MAX_APCLI_NUM ? NULL : pAd->ApCfg.ApCliTab[infRealIdx].dev);
		} 
		else
#endif // APCLI_SUPPORT //
#ifdef WDS_SUPPORT
		if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS)
		{
			dev_p = ((infRealIdx > MAX_WDS_ENTRY) ? NULL : pAd->WdsTab.WdsEntry[infRealIdx].dev);
		}
		else
#endif // WDS_SUPPORT //
		{
			if (FromWhichBSSID >= pAd->ApCfg.BssidNum)
	    		{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: fatal error ssid > ssid num!\n", __FUNCTION__));
				dev_p = pAd->net_dev;
	    		}

	    		if (FromWhichBSSID == BSS0)
				dev_p = pAd->net_dev;
	    		else
	    		{
	    	    		dev_p = pAd->ApCfg.MBSSID[FromWhichBSSID].MSSIDDev;
	    		}
		}
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		dev_p = pAd->net_dev;
	}
#endif // CONFIG_STA_SUPPORT //

	ASSERT(dev_p);
	return dev_p; /* return one of MBSS */
}

#ifdef CONFIG_AP_SUPPORT
UCHAR VLAN_8023_Header_Copy(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PUCHAR			pHeader802_3,
	IN	UINT            HdrLen,
	OUT PUCHAR			pData,
	IN	UCHAR			FromWhichBSSID,
	IN	UCHAR			*TPID)
{
	UINT16 TCI;
	UCHAR VLAN_Size = 0;


	if ((FromWhichBSSID < pAd->ApCfg.BssidNum) && (pAd->ApCfg.MBSSID[FromWhichBSSID].VLAN_VID != 0))
	{
		MULTISSID_STRUCT *mbss_p = &pAd->ApCfg.MBSSID[FromWhichBSSID];

		/* need to insert VLAN tag */
		VLAN_Size = LENGTH_802_1Q;

		/* make up TCI field */
		TCI = (mbss_p->VLAN_VID & 0x0fff) | ((mbss_p->VLAN_Priority & 0x7)<<13);

#ifndef RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif // RT_BIG_ENDIAN //

		/* copy dst + src MAC (12B) */
		memcpy(pData, pHeader802_3, LENGTH_802_3_NO_TYPE);

		/* copy VLAN tag (4B) */
		/* do NOT use memcpy to speed up */
		*(UINT16 *)(pData+LENGTH_802_3_NO_TYPE) = *(UINT16 *)TPID;
		*(UINT16 *)(pData+LENGTH_802_3_NO_TYPE+2) = TCI;

		/* copy type/len (2B) */
		*(UINT16 *)(pData+LENGTH_802_3_NO_TYPE+LENGTH_802_1Q) = \
				*(UINT16 *)&pHeader802_3[LENGTH_802_3-LENGTH_802_3_TYPE];

		/* copy tail if exist */
		if (HdrLen > LENGTH_802_3)
		{
			memcpy(pData+LENGTH_802_3+LENGTH_802_1Q,
					pHeader802_3+LENGTH_802_3,
					HdrLen - LENGTH_802_3);
		} /* End of if */
	}
	else
	{
		/* no VLAN tag is needed to insert */
		memcpy(pData, pHeader802_3, HdrLen);
	} /* End of if */

	return VLAN_Size;
} /* End of VLAN_Tag_Insert */
#endif // CONFIG_AP_SUPPORT //


/*
========================================================================
Routine Description:
    Allocate memory for adapter control block.

Arguments:
    pAd					Pointer to our adapter

Return Value:
	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE
	NDIS_STATUS_RESOURCES

Note:
========================================================================
*/
NDIS_STATUS AdapterBlockAllocateMemory(
	IN PVOID	handle,
	OUT	PVOID	*ppAd)
{
#ifdef WORKQUEUE_BH
	POS_COOKIE cookie;
#endif // WORKQUEUE_BH //


	*ppAd = (PVOID)vmalloc(sizeof(RTMP_ADAPTER)); //pci_alloc_consistent(pci_dev, sizeof(RTMP_ADAPTER), phy_addr);
    
	if (*ppAd) 
	{
		NdisZeroMemory(*ppAd, sizeof(RTMP_ADAPTER));
		((PRTMP_ADAPTER)*ppAd)->OS_Cookie = handle;
#ifdef WORKQUEUE_BH
		cookie = (POS_COOKIE)(((PRTMP_ADAPTER)*ppAd)->OS_Cookie);
		cookie->pAd_va = *ppAd;
#endif // WORKQUEUE_BH //

		return (NDIS_STATUS_SUCCESS);
	} else {
		return (NDIS_STATUS_FAILURE);
	}
}

/*
========================================================================
Routine Description:
	Assign private data pointer to the network interface.

Arguments:
	pDev			- the device
	pPriv			- the pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsSetNetDevPriv(IN VOID *pDev,
			 IN VOID *pPriv) {

	DEV_PRIV_INFO *pDevInfo = NULL;


	pDevInfo = (DEV_PRIV_INFO *) _RTMP_OS_NETDEV_GET_PRIV((PNET_DEV) pDev);
	if (pDevInfo == NULL)
	{
		os_alloc_mem(NULL, (UCHAR **)&pDevInfo, sizeof(DEV_PRIV_INFO));
		if (pDevInfo == NULL)
			return;
	}

	pDevInfo->pPriv = (VOID *)pPriv;
	pDevInfo->priv_flags = 0;

	_RTMP_OS_NETDEV_SET_PRIV((PNET_DEV) pDev, pDevInfo);
}


/*
========================================================================
Routine Description:
	Get private data pointer from the network interface.

Arguments:
	pDev			- the device
	pPriv			- the pointer

Return Value:
	None

Note:
========================================================================
*/
VOID *RtmpOsGetNetDevPriv(IN VOID *pDev) {

	DEV_PRIV_INFO *pDevInfo = NULL;


	pDevInfo = (DEV_PRIV_INFO *) _RTMP_OS_NETDEV_GET_PRIV((PNET_DEV) pDev);
	if (pDevInfo != NULL)
		return pDevInfo->pPriv;
	return NULL;
}


/*
========================================================================
Routine Description:
	Get private flags from the network interface.

Arguments:
	pDev			- the device

Return Value:
	pPriv			- the pointer

Note:
========================================================================
*/
USHORT RtmpDevPrivFlagsGet(IN VOID *pDev) {

	DEV_PRIV_INFO *pDevInfo = NULL;


	pDevInfo = (DEV_PRIV_INFO *) _RTMP_OS_NETDEV_GET_PRIV((PNET_DEV) pDev);
	if (pDevInfo != NULL)
		return pDevInfo->priv_flags;
	return 0;
}


/*
========================================================================
Routine Description:
	Get private flags from the network interface.

Arguments:
	pDev			- the device

Return Value:
	pPriv			- the pointer

Note:
========================================================================
*/
VOID RtmpDevPrivFlagsSet(IN VOID *pDev, IN USHORT PrivFlags) {

	DEV_PRIV_INFO *pDevInfo = NULL;


	pDevInfo = (DEV_PRIV_INFO *) _RTMP_OS_NETDEV_GET_PRIV((PNET_DEV) pDev);
	if (pDevInfo != NULL)
		pDevInfo->priv_flags = PrivFlags;
}
/* export utility function symbol list */
#ifdef OS_ABL_SUPPORT

EXPORT_SYMBOL(RTDebugLevel);

EXPORT_SYMBOL(RtmpOSTaskNotifyToExit);

EXPORT_SYMBOL(RTMPFreeNdisPacket);
EXPORT_SYMBOL(AdapterBlockAllocateMemory);
EXPORT_SYMBOL(RTMP_IndicateMediaState);

EXPORT_SYMBOL(RTMP_SetPeriodicTimer);
EXPORT_SYMBOL(RTMP_OS_Add_Timer);
EXPORT_SYMBOL(RTMP_OS_Mod_Timer);
EXPORT_SYMBOL(RTMP_OS_Del_Timer);
EXPORT_SYMBOL(RTMP_OS_Init_Timer);

EXPORT_SYMBOL(os_alloc_mem);
EXPORT_SYMBOL(os_free_mem);

EXPORT_SYMBOL(RTMPL2FrameTxAction);
EXPORT_SYMBOL(ExpandPacket);
EXPORT_SYMBOL(ClonePacket);
EXPORT_SYMBOL(RTMP_AllocateFragPacketBuffer);
EXPORT_SYMBOL(announce_802_3_packet);
EXPORT_SYMBOL(Sniff2BytesFromNdisBuffer);
EXPORT_SYMBOL(RtmpOSNetPktAlloc);
EXPORT_SYMBOL(duplicate_pkt);
EXPORT_SYMBOL(duplicate_pkt_with_TKIP_MIC);
EXPORT_SYMBOL(DuplicatePacket);
EXPORT_SYMBOL(wlan_802_11_to_802_3_packet);
EXPORT_SYMBOL(RTMPAllocateNdisPacket);
EXPORT_SYMBOL(update_os_packet_info);
EXPORT_SYMBOL(RTMP_QueryPacketInfo);

EXPORT_SYMBOL(RtmpOSNetDevCreate);
EXPORT_SYMBOL(RtmpOSNetDevAddrSet);
EXPORT_SYMBOL(RtmpOSNetDevClose);
EXPORT_SYMBOL(RtmpOSNetDevAttach);
EXPORT_SYMBOL(RtmpOSNetDevDetach);
EXPORT_SYMBOL(RtmpOSNetDevFree);

EXPORT_SYMBOL(RtmpOSFileOpen);
EXPORT_SYMBOL(RtmpOSFSInfoChange);
EXPORT_SYMBOL(RtmpOSFileWrite);
EXPORT_SYMBOL(RtmpOSFileRead);
EXPORT_SYMBOL(RtmpOSFileClose);
EXPORT_SYMBOL(RtmpOSFileSeek);

EXPORT_SYMBOL(RtmpOSTaskInit);
EXPORT_SYMBOL(RtmpOSTaskAttach);
EXPORT_SYMBOL(RtmpOSTaskCustomize);
EXPORT_SYMBOL(RtmpOSTaskKill);

EXPORT_SYMBOL(get_netdev_from_bssid);
EXPORT_SYMBOL(hex_dump);
EXPORT_SYMBOL(RTMPFreeAdapter);
EXPORT_SYMBOL(RTMP_GetCurrentSystemTime);
#ifdef SYSTEM_LOG_SUPPORT
EXPORT_SYMBOL(RTMPSendWirelessEvent);
#endif // SYSTEM_LOG_SUPPORT //
EXPORT_SYMBOL(RTMPusecDelay);
EXPORT_SYMBOL(RtmpOSWrielessEventSend);

#ifdef CONFIG_AP_SUPPORT
EXPORT_SYMBOL(duplicate_pkt_with_VLAN);
EXPORT_SYMBOL(VLAN_8023_Header_Copy);
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
EXPORT_SYMBOL(ralinkrate);
EXPORT_SYMBOL(RT_RateSize);
EXPORT_SYMBOL(send_monitor_packets);
#endif // CONFIG_STA_SUPPORT //

#ifdef RTMP_MAC_PCI
EXPORT_SYMBOL(RTMP_AllocateMgmtDescMemory);
EXPORT_SYMBOL(RTMP_AllocateRxPacketBuffer);
EXPORT_SYMBOL(RTMP_FreeDescMemory);
EXPORT_SYMBOL(RTMP_AllocateTxDescMemory);
EXPORT_SYMBOL(linux_pci_unmap_single);
EXPORT_SYMBOL(RTMP_AllocateFirstTxBuffer);
EXPORT_SYMBOL(linux_pci_map_single);
EXPORT_SYMBOL(RTMP_AllocateRxDescMemory);
EXPORT_SYMBOL(RTMP_FreeFirstTxBuffer);
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
EXPORT_SYMBOL(RTMPPCIeLinkCtrlValueRestore);
EXPORT_SYMBOL(RTMPPCIeLinkCtrlSetting);
#endif // PCIE_PS_SUPPORT //
#endif // CONFIG_STA_SUPPORT //
#endif // RTMP_MAC_PCI //


#ifdef RTMP_RBUS_SUPPORT
EXPORT_SYMBOL(RtmpFlashRead);
EXPORT_SYMBOL(RtmpFlashWrite);
#endif // RTMP_RBUS_SUPPORT //

//#ifdef CONFIG_AP_SUPPORT
//#ifdef NEW_DFS
//EXPORT_SYMBOL(schedule_dfs_task);
//#endif // NEW_DFS //
//#endif // CONFIG_AP_SUPPORT //

EXPORT_SYMBOL(RtmpOsSetNetDevPriv);
EXPORT_SYMBOL(RtmpOsGetNetDevPriv);
EXPORT_SYMBOL(RtmpDevPrivFlagsGet);
#endif // OS_ABL_SUPPORT //
