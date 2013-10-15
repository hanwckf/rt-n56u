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


#ifdef RESOURCE_PRE_ALLOC
VOID RTMPResetTxRxRingMemory(
	IN RTMP_ADAPTER * pAd)
{
	int index, j;
	PRTMP_TX_RING pTxRing;
	PTXD_STRUC	  pTxD;
#ifdef RT_BIG_ENDIAN
	PTXD_STRUC      pDestTxD;
	TXD_STRUC       TxD;
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET  pPacket;
	unsigned int  IrqFlags;


	/* Free TxSwQueue Packet*/
	for (index=0; index <NUM_OF_TX_RING; index++)
	{
		PQUEUE_ENTRY pEntry;
		PNDIS_PACKET pPacket;
		PQUEUE_HEADER   pQueue;

		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		pQueue = &pAd->TxSwQueue[index];
		while (pQueue->Head)
		{
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	}


	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->TxRing[index];
		
		for (j=0; j< TX_RING_SIZE; j++)
		{	
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (PTXD_STRUC) (pTxRing->Cell[j].AllocVa);
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (PTXD_STRUC) (pTxRing->Cell[j].AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = pTxRing->Cell[j].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}		
			/*Always assign pNdisPacket as NULL after clear*/
			pTxRing->Cell[j].pNdisPacket = NULL;
					
			pPacket = pTxRing->Cell[j].pNextNdisPacket;
			
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

		}
	}	

	
	for (index = RX_RING_SIZE - 1 ; index >= 0; index--)
	{
		if ((pAd->RxRing.Cell[index].DmaBuf.AllocVa) && (pAd->RxRing.Cell[index].pNdisPacket))
		{
			PCI_UNMAP_SINGLE(pAd, pAd->RxRing.Cell[index].DmaBuf.AllocPa, pAd->RxRing.Cell[index].DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
			RELEASE_NDIS_PACKET(pAd, pAd->RxRing.Cell[index].pNdisPacket, NDIS_STATUS_SUCCESS);
		}
	}
	NdisZeroMemory(pAd->RxRing.Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));

	if (pAd->FragFrame.pFragPacket)
	{
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		pAd->FragFrame.pFragPacket = NULL;
	}

	NdisFreeSpinLock(&pAd->CmdQLock);
}


VOID RTMPFreeTxRxRingMemory(
    IN  PRTMP_ADAPTER   pAd)
{
	int num;

	
	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));


	/* Free RxDesc buffer*/
	if (pAd->RxDescRing.AllocVa)
	{
#ifdef DESC_32B_SUPPORT
		kfree(pAd->RxDescRing.AllocVa);
#else
		RTMP_FreeDescMemory(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, 
								pAd->RxDescRing.AllocSize, 
								pAd->RxDescRing.AllocVa, 
								pAd->RxDescRing.AllocPa);
#endif /* DESC_32B_SUPPORT */
	}
	NdisZeroMemory(&pAd->RxDescRing, sizeof(RTMP_DMABUF));

	/* Free MgmtDesc buffer*/
	if (pAd->MgmtDescRing.AllocVa)
	{
		RTMP_FreeDescMemory(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, 
								pAd->MgmtDescRing.AllocSize, 
								pAd->MgmtDescRing.AllocVa, 
								pAd->MgmtDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->MgmtDescRing, sizeof(RTMP_DMABUF));

	/* Free 1st TxBufSpace and TxDesc buffer*/
	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
		if (pAd->TxBufSpace[num].AllocVa)
		{
			RTMP_FreeFirstTxBuffer(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, 
									pAd->TxBufSpace[num].AllocSize, 
									FALSE, pAd->TxBufSpace[num].AllocVa, 
									pAd->TxBufSpace[num].AllocPa);
		}
		NdisZeroMemory(&pAd->TxBufSpace[num], sizeof(RTMP_DMABUF));

		if (pAd->TxDescRing[num].AllocVa)
		{
			RTMP_FreeDescMemory(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, 
									pAd->TxDescRing[num].AllocSize, 
									pAd->TxDescRing[num].AllocVa, 
									pAd->TxDescRing[num].AllocPa);
		}
		NdisZeroMemory(&pAd->TxDescRing[num], sizeof(RTMP_DMABUF));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}


NDIS_STATUS RTMPInitTxRxRingMemory
	(IN RTMP_ADAPTER *pAd)
{
	INT				num, index;
	ULONG			RingBasePaHigh;
	ULONG			RingBasePaLow;
	PVOID			RingBaseVa;
	PRTMP_TX_RING	pTxRing;
	PRTMP_DMABUF	pDmaBuf;
	PNDIS_PACKET	pPacket;
	PTXD_STRUC		pTxD;
	PRXD_STRUC		pRxD;
	ULONG			ErrorValue = 0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;


	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->CmdQLock);	
	NdisAcquireSpinLock(&pAd->CmdQLock);
	RTInitializeCmdQ(&pAd->CmdQ);
	NdisReleaseSpinLock(&pAd->CmdQLock);


	
	/* Initialize All Tx Ring Descriptors and associated buffer memory*/
	/* (5 TX rings = 4 ACs + 1 HCCA)*/
	for (num=0; num<NUM_OF_TX_RING; num++)
	{
		ULONG  BufBasePaHigh;
		ULONG  BufBasePaLow;
		PVOID  BufBaseVa;
		
		/* memory zero the  Tx ring descriptor's memory */
		NdisZeroMemory(pAd->TxDescRing[num].AllocVa, pAd->TxDescRing[num].AllocSize);
		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxDescRing[num].AllocPa);
		RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->TxDescRing[num].AllocPa);
		RingBaseVa     = pAd->TxDescRing[num].AllocVa;

		/* Zero init all 1st TXBuf's memory for this TxRing*/
		NdisZeroMemory(pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocSize);
		/* Save PA & VA for further operation*/
		BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBufSpace[num].AllocPa);
		BufBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->TxBufSpace[num].AllocPa);
		BufBaseVa     = pAd->TxBufSpace[num].AllocVa;

		/* linking Tx Ring Descriptor and associated buffer memory*/
		pTxRing = &pAd->TxRing[num];
		for (index = 0; index < TX_RING_SIZE; index++)
		{
			pTxRing->Cell[index].pNdisPacket = NULL;
			pTxRing->Cell[index].pNextNdisPacket = NULL;
			/* Init Tx Ring Size, Va, Pa variables*/
			pTxRing->Cell[index].AllocSize = TXD_SIZE;
			pTxRing->Cell[index].AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(pTxRing->Cell[index].AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (pTxRing->Cell[index].AllocPa, RingBasePaLow);

			/* Setup Tx Buffer size & address. only 802.11 header will store in this space*/
			pDmaBuf = &pTxRing->Cell[index].DmaBuf;
			pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
			pDmaBuf->AllocVa = BufBaseVa;
			RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
			RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (PTXD_STRUC) pTxRing->Cell[index].AllocVa;
			pTxD->SDPtr0 = BufBasePaLow;
			/* advance to next ring descriptor address*/
			pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, RXD_SIZE);

			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* advance to next TxBuf address*/
			BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
			BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("TxRing[%d]: total %d entry initialized\n", num, index));
	}

	/* Initialize MGMT Ring and associated buffer memory*/
	RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->MgmtDescRing.AllocPa);
	RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->MgmtDescRing.AllocPa);
	RingBaseVa     = pAd->MgmtDescRing.AllocVa;
	NdisZeroMemory(pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocSize);
	for (index = 0; index < MGMT_RING_SIZE; index++)
	{
		pAd->MgmtRing.Cell[index].pNdisPacket = NULL;
		pAd->MgmtRing.Cell[index].pNextNdisPacket = NULL;
		/* Init MGMT Ring Size, Va, Pa variables*/
		pAd->MgmtRing.Cell[index].AllocSize = TXD_SIZE;
		pAd->MgmtRing.Cell[index].AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(pAd->MgmtRing.Cell[index].AllocPa, RingBasePaHigh);
		RTMP_SetPhysicalAddressLow (pAd->MgmtRing.Cell[index].AllocPa, RingBasePaLow);

		/* Offset to next ring descriptor address*/
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

		/* link the pre-allocated TxBuf to TXD*/
		pTxD = (PTXD_STRUC) pAd->MgmtRing.Cell[index].AllocVa;
		pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxD, RXD_SIZE);

		/* no pre-allocated buffer required in MgmtRing for scatter-gather case*/
	}

	/* Initialize Rx Ring and associated buffer memory*/
	NdisZeroMemory(pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocSize);
	DBGPRINT(RT_DEBUG_OFF,  ("RX DESC %p  size = %ld\n", 
					pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocSize));

	/* Save PA & VA for further operation*/
	RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->RxDescRing.AllocPa);
	RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->RxDescRing.AllocPa);
	RingBaseVa     = pAd->RxDescRing.AllocVa;

	/* Linking Rx Ring and associated buffer memory*/
	for (index = 0; index < RX_RING_SIZE; index++)
	{
		/* Init RX Ring Size, Va, Pa variables*/
		pAd->RxRing.Cell[index].AllocSize = RXD_SIZE;
		pAd->RxRing.Cell[index].AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(pAd->RxRing.Cell[index].AllocPa, RingBasePaHigh);
		RTMP_SetPhysicalAddressLow (pAd->RxRing.Cell[index].AllocPa, RingBasePaLow);

		/* Offset to next ring descriptor address*/
		RingBasePaLow += RXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + RXD_SIZE;

		/* Setup Rx associated Buffer size & allocate share memory*/
		pDmaBuf = &pAd->RxRing.Cell[index].DmaBuf;
		pDmaBuf->AllocSize = RX_BUFFER_AGGRESIZE;
		pPacket = RTMP_AllocateRxPacketBuffer(
			pAd,
			((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pDmaBuf->AllocSize,
			FALSE,
			&pDmaBuf->AllocVa,
			&pDmaBuf->AllocPa);
		
		/* keep allocated rx packet */
		pAd->RxRing.Cell[index].pNdisPacket = pPacket;

		/* Error handling*/
		if (pDmaBuf->AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate RxRing's 1st buffer\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		/* Zero init this memory block*/
		NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

		/* Write RxD buffer address & allocated buffer length*/
		pRxD = (PRXD_STRUC) pAd->RxRing.Cell[index].AllocVa;
		pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
		pRxD->DDONE = 0;

#ifdef RX_DMA_SCATTER
		pRxD->SDL0 = RX_BUFFER_AGGRESIZE;
#endif /* RX_DMA_SCATTER */

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif

#ifdef DESC_32B_SUPPORT
		dma_cache_sync(NULL, pRxD, RXD_SIZE, DMA_TO_DEVICE);
#endif /* DESC_32B_SUPPORT */

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxD, RXD_SIZE);
	}


	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket =  RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

	if (pAd->FragFrame.pFragPacket == NULL)
	{
		Status = NDIS_STATUS_RESOURCES;
	}


	/* Following code segment get from original func:NICInitTxRxRingAndBacklogQueue(), now should integrate it to here.*/
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitTxRxRingAndBacklogQueue\n"));


		/* Initialize all transmit related software queues*/
		for(index = 0; index < NUM_OF_TX_RING; index++)
		{
			InitializeQueueHeader(&pAd->TxSwQueue[index]);
			/* Init TX rings index pointer*/
			pAd->TxRing[index].TxSwFreeIdx = 0;
			pAd->TxRing[index].TxCpuIdx = 0;
			/*RTMP_IO_WRITE32(pAd, (TX_CTX_IDX0 + i * 0x10) ,  pAd->TxRing[i].TX_CTX_IDX);*/
		}
	
		/* Init RX Ring index pointer*/
		pAd->RxRing.RxSwReadIdx = 0;
		pAd->RxRing.RxCpuIdx = RX_RING_SIZE - 1;
		/*RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RX_CRX_IDX0);*/

		
		/* init MGMT ring index pointer*/
		pAd->MgmtRing.TxSwFreeIdx = 0;
		pAd->MgmtRing.TxCpuIdx = 0;

		pAd->PrivateInfo.TxRingFullCnt = 0;
		
		DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitTxRxRingAndBacklogQueue\n"));
	}
	
	return Status;
	
}


/*
	========================================================================
	
	Routine Description:
		Allocate DMA memory blocks for send, receive

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
		NDIS_STATUS_RESOURCES

	IRQL = PASSIVE_LEVEL

	Note:
	
	========================================================================
*/
NDIS_STATUS	RTMPAllocTxRxRingMemory(
	IN	PRTMP_ADAPTER	pAd)
{
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	INT				num;
	ULONG			ErrorValue = 0;
/*	PRTMP_REORDERBUF	pReorderBuf;*/

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocTxRxRingMemory\n"));
	do
	{
		/*
			Allocate all ring descriptors, include TxD, RxD, MgmtD.
			Although each size is different, to prevent cacheline and alignment
			issue, I intentional set them all to 64 bytes.
		*/
		for (num=0; num<NUM_OF_TX_RING; num++)
		{
			/* Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)*/
			
			pAd->TxDescRing[num].AllocSize = TX_RING_SIZE * TXD_SIZE;
			RTMP_AllocateTxDescMemory(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				/* pAd,*/
				num,
				pAd->TxDescRing[num].AllocSize,
				FALSE,
				&pAd->TxDescRing[num].AllocVa,
				&pAd->TxDescRing[num].AllocPa);

			if (pAd->TxDescRing[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* 
				Allocate all 1st TXBuf's memory for this TxRing
			*/
			pAd->TxBufSpace[num].AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				num,
				pAd->TxBufSpace[num].AllocSize,
				FALSE,
				&pAd->TxBufSpace[num].AllocVa,
				&pAd->TxBufSpace[num].AllocPa);

			if (pAd->TxBufSpace[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("TxRing[%d]: total %d bytes allocated\n", num, (INT)pAd->TxDescRing[num].AllocSize));
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

		
		/*
			Allocate MGMT ring descriptor's memory except Tx ring which allocated eariler
		*/
		pAd->MgmtDescRing.AllocSize = MGMT_RING_SIZE * TXD_SIZE;
		RTMP_AllocateMgmtDescMemory(
			((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pAd->MgmtDescRing.AllocSize,
			FALSE,
			&pAd->MgmtDescRing.AllocVa,
			&pAd->MgmtDescRing.AllocPa);

		if (pAd->MgmtDescRing.AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MGMT Ring: total %d bytes allocated\n", (INT)pAd->MgmtDescRing.AllocSize));


		
		/*
			Allocate RX ring descriptor's memory except Tx ring which allocated eariler
		*/
		pAd->RxDescRing.AllocSize = RX_RING_SIZE * RXD_SIZE;
		RTMP_AllocateRxDescMemory(
			((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pAd->RxDescRing.AllocSize,
			FALSE,
			&pAd->RxDescRing.AllocVa,
			&pAd->RxDescRing.AllocPa);

		if (pAd->RxDescRing.AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("Rx Ring: total %d bytes allocated\n", (INT)pAd->RxDescRing.AllocSize));

	}	while (FALSE);


	if (Status != NDIS_STATUS_SUCCESS)
	{
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	DBGPRINT_S(Status, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}

#else
/*
	========================================================================
	
	Routine Description:
		Allocate DMA memory blocks for send, receive

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
		NDIS_STATUS_RESOURCES

	IRQL = PASSIVE_LEVEL

	Note:
	
	========================================================================
*/
NDIS_STATUS	RTMPAllocTxRxRingMemory(
	IN	PRTMP_ADAPTER	pAd)
{
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	ULONG			RingBasePaHigh;
	ULONG			RingBasePaLow;
	PVOID			RingBaseVa;
	INT				index, num;
	PTXD_STRUC		pTxD;
	PRXD_STRUC		pRxD;
	ULONG			ErrorValue = 0;
	PRTMP_TX_RING	pTxRing;
	PRTMP_DMABUF	pDmaBuf;
	PNDIS_PACKET	pPacket;
/*	PRTMP_REORDERBUF	pReorderBuf;*/

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocTxRxRingMemory\n"));
	do
	{
		/* Init the CmdQ and CmdQLock*/
		NdisAllocateSpinLock(pAd, &pAd->CmdQLock);	
		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTInitializeCmdQ(&pAd->CmdQ);
		NdisReleaseSpinLock(&pAd->CmdQLock);

		/* 
			Allocate all ring descriptors, include TxD, RxD, MgmtD.
			Although each size is different, to prevent cacheline and alignment
			issue, I intentional set them all to 64 bytes
		*/
		for (num=0; num<NUM_OF_TX_RING; num++)
		{
			ULONG  BufBasePaHigh;
			ULONG  BufBasePaLow;
			PVOID  BufBaseVa;
			
			/* 
				Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)
			*/
			pAd->TxDescRing[num].AllocSize = TX_RING_SIZE * TXD_SIZE;
			RTMP_AllocateTxDescMemory(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				/* pAd,*/
				num,
				pAd->TxDescRing[num].AllocSize,
				FALSE,
				&pAd->TxDescRing[num].AllocVa,
				&pAd->TxDescRing[num].AllocPa);

			if (pAd->TxDescRing[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(pAd->TxDescRing[num].AllocVa, pAd->TxDescRing[num].AllocSize);

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxDescRing[num].AllocPa);
			RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->TxDescRing[num].AllocPa);
			RingBaseVa     = pAd->TxDescRing[num].AllocVa;

			/*
				Allocate all 1st TXBuf's memory for this TxRing
			*/
			pAd->TxBufSpace[num].AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				num,
				pAd->TxBufSpace[num].AllocSize,
				FALSE,
				&pAd->TxBufSpace[num].AllocVa,
				&pAd->TxBufSpace[num].AllocPa);

			if (pAd->TxBufSpace[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocSize);

			/* Save PA & VA for further operation*/
			BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBufSpace[num].AllocPa);
			BufBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->TxBufSpace[num].AllocPa);
			BufBaseVa     = pAd->TxBufSpace[num].AllocVa;

			/*
				Initialize Tx Ring Descriptor and associated buffer memory
			*/
			pTxRing = &pAd->TxRing[num];
			for (index = 0; index < TX_RING_SIZE; index++)
			{
				pTxRing->Cell[index].pNdisPacket = NULL;
				pTxRing->Cell[index].pNextNdisPacket = NULL;
				/* Init Tx Ring Size, Va, Pa variables*/
				pTxRing->Cell[index].AllocSize = TXD_SIZE;
				pTxRing->Cell[index].AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(pTxRing->Cell[index].AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow (pTxRing->Cell[index].AllocPa, RingBasePaLow);

				/* Setup Tx Buffer size & address. only 802.11 header will store in this space*/
				pDmaBuf = &pTxRing->Cell[index].DmaBuf;
				pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
				pDmaBuf->AllocVa = BufBaseVa;
				RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
				RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

				/* link the pre-allocated TxBuf to TXD*/
				pTxD = (PTXD_STRUC) pTxRing->Cell[index].AllocVa;
				pTxD->SDPtr0 = BufBasePaLow;
				/* advance to next ring descriptor address*/
				pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, RXD_SIZE);

				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				/* advance to next TxBuf address*/
				BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
				BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("TxRing[%d]: total %d entry allocated\n", num, index));
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

		/*
			Allocate MGMT ring descriptor's memory except Tx ring which allocated eariler
		*/
		pAd->MgmtDescRing.AllocSize = MGMT_RING_SIZE * TXD_SIZE;
		RTMP_AllocateMgmtDescMemory(
			((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pAd->MgmtDescRing.AllocSize,
			FALSE,
			&pAd->MgmtDescRing.AllocVa,
			&pAd->MgmtDescRing.AllocPa);

		if (pAd->MgmtDescRing.AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		/* Zero init this memory block*/
		NdisZeroMemory(pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocSize);

		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->MgmtDescRing.AllocPa);
		RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->MgmtDescRing.AllocPa);
		RingBaseVa     = pAd->MgmtDescRing.AllocVa;

		/*
			Initialize MGMT Ring and associated buffer memory
		*/
		for (index = 0; index < MGMT_RING_SIZE; index++)
		{
			pAd->MgmtRing.Cell[index].pNdisPacket = NULL;
			pAd->MgmtRing.Cell[index].pNextNdisPacket = NULL;
			/* Init MGMT Ring Size, Va, Pa variables*/
			pAd->MgmtRing.Cell[index].AllocSize = TXD_SIZE;
			pAd->MgmtRing.Cell[index].AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(pAd->MgmtRing.Cell[index].AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (pAd->MgmtRing.Cell[index].AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address*/
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (PTXD_STRUC) pAd->MgmtRing.Cell[index].AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, RXD_SIZE);

			/* no pre-allocated buffer required in MgmtRing for scatter-gather case*/
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MGMT Ring: total %d entry allocated\n", index));

		/*
			Allocate RX ring descriptor's memory except Tx ring which allocated eariler
		*/
		pAd->RxDescRing.AllocSize = RX_RING_SIZE * RXD_SIZE;
		RTMP_AllocateRxDescMemory(
			((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pAd->RxDescRing.AllocSize,
			FALSE,
			&pAd->RxDescRing.AllocVa,
			&pAd->RxDescRing.AllocPa);

		if (pAd->RxDescRing.AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		/* Zero init this memory block*/
		NdisZeroMemory(pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocSize);


		DBGPRINT(RT_DEBUG_OFF, 
					("RX DESC %p  size = %ld\n", pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocSize));

		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->RxDescRing.AllocPa);
		RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->RxDescRing.AllocPa);
		RingBaseVa     = pAd->RxDescRing.AllocVa;

		
		/* Initialize Rx Ring and associated buffer memory*/
		
		for (index = 0; index < RX_RING_SIZE; index++)
		{
			/* Init RX Ring Size, Va, Pa variables*/
			pAd->RxRing.Cell[index].AllocSize = RXD_SIZE;
			pAd->RxRing.Cell[index].AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(pAd->RxRing.Cell[index].AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (pAd->RxRing.Cell[index].AllocPa, RingBasePaLow);

			/*NdisZeroMemory(RingBaseVa, RXD_SIZE);*/

			/* Offset to next ring descriptor address*/
			RingBasePaLow += RXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + RXD_SIZE;

			/* Setup Rx associated Buffer size & allocate share memory*/
			pDmaBuf = &pAd->RxRing.Cell[index].DmaBuf;
			pDmaBuf->AllocSize = RX_BUFFER_AGGRESIZE;
			pPacket = RTMP_AllocateRxPacketBuffer(
				pAd,
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				pDmaBuf->AllocSize,
				FALSE,
				&pDmaBuf->AllocVa,
				&pDmaBuf->AllocPa);
			
			/* keep allocated rx packet */
			pAd->RxRing.Cell[index].pNdisPacket = pPacket;

			/* Error handling*/
			if (pDmaBuf->AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate RxRing's 1st buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

			/* Write RxD buffer address & allocated buffer length*/
			pRxD = (PRXD_STRUC) pAd->RxRing.Cell[index].AllocVa;
			pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			pRxD->DDONE = 0;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif

#ifdef DESC_32B_SUPPORT
			dma_cache_sync(NULL, pRxD, RXD_SIZE, DMA_TO_DEVICE);
#endif /* DESC_32B_SUPPORT */

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxD, RXD_SIZE);
		}

		DBGPRINT(RT_DEBUG_TRACE, ("Rx Ring: total %d entry allocated\n", index));

	}	while (FALSE);


	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket =  RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

	if (pAd->FragFrame.pFragPacket == NULL)
	{
		Status = NDIS_STATUS_RESOURCES;
	}

	if (Status != NDIS_STATUS_SUCCESS)
	{
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	/* Following code segment get from original func:NICInitTxRxRingAndBacklogQueue(), now should integrate it to here.*/
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitTxRxRingAndBacklogQueue\n"));


		/* Initialize all transmit related software queues*/
		for(index = 0; index < NUM_OF_TX_RING; index++)
		{
			InitializeQueueHeader(&pAd->TxSwQueue[index]);
			/* Init TX rings index pointer*/
			pAd->TxRing[index].TxSwFreeIdx = 0;
			pAd->TxRing[index].TxCpuIdx = 0;
			/*RTMP_IO_WRITE32(pAd, (TX_CTX_IDX0 + i * 0x10) ,  pAd->TxRing[i].TX_CTX_IDX);*/
		}
	
		/* Init RX Ring index pointer*/
		pAd->RxRing.RxSwReadIdx = 0;
		pAd->RxRing.RxCpuIdx = RX_RING_SIZE - 1;
		/*RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RX_CRX_IDX0);*/

		
		/* init MGMT ring index pointer*/
		pAd->MgmtRing.TxSwFreeIdx = 0;
		pAd->MgmtRing.TxCpuIdx = 0;

		pAd->PrivateInfo.TxRingFullCnt = 0;
		
		DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitTxRxRingAndBacklogQueue\n"));
	}

	DBGPRINT_S(Status, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}




VOID RTMPFreeTxRxRingMemory(
    IN  PRTMP_ADAPTER   pAd)
{
	int index, num , j;
	PRTMP_TX_RING pTxRing;
	PTXD_STRUC	  pTxD;
#ifdef RT_BIG_ENDIAN
	PTXD_STRUC      pDestTxD;
	TXD_STRUC       TxD;
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET  pPacket;
	unsigned int  IrqFlags;

	/*POS_COOKIE pObj =(POS_COOKIE) pAd->OS_Cookie;*/

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));

	/* Free TxSwQueue Packet*/
	for (index=0; index <NUM_OF_TX_RING; index++)
	{
		PQUEUE_ENTRY pEntry;
		PNDIS_PACKET pPacket;
		PQUEUE_HEADER   pQueue;

		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		pQueue = &pAd->TxSwQueue[index];
		while (pQueue->Head)
		{
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	}

	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->TxRing[index];
		
		for (j=0; j< TX_RING_SIZE; j++)
		{	
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (PTXD_STRUC) (pTxRing->Cell[j].AllocVa);
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (PTXD_STRUC) (pTxRing->Cell[j].AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = pTxRing->Cell[j].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}		
			/*Always assign pNdisPacket as NULL after clear*/
			pTxRing->Cell[j].pNdisPacket = NULL;
					
			pPacket = pTxRing->Cell[j].pNextNdisPacket;
			
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
	}	

	{
	PRTMP_MGMT_RING pMgmtRing = &pAd->MgmtRing;
	NdisAcquireSpinLock(&pAd->MgmtRingLock);

	RTMP_IO_READ32(pAd, TX_MGMTDTX_IDX, &pMgmtRing->TxDmaIdx);
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
#ifdef RT_BIG_ENDIAN
        pDestTxD = (PTXD_STRUC) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
        TxD = *pDestTxD;
        pTxD = &TxD;
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (PTXD_STRUC) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
#endif
		pTxD->DMADONE = 0;
		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket = NULL;

		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket;
		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket = NULL;
		INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}
	NdisReleaseSpinLock(&pAd->MgmtRingLock);
	}

	for (index = RX_RING_SIZE - 1 ; index >= 0; index--)
	{
		if ((pAd->RxRing.Cell[index].DmaBuf.AllocVa) && (pAd->RxRing.Cell[index].pNdisPacket))
		{
			PCI_UNMAP_SINGLE(pAd, pAd->RxRing.Cell[index].DmaBuf.AllocPa, pAd->RxRing.Cell[index].DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
			RELEASE_NDIS_PACKET(pAd, pAd->RxRing.Cell[index].pNdisPacket, NDIS_STATUS_SUCCESS);
		}
	}
	NdisZeroMemory(pAd->RxRing.Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));
	
	if (pAd->RxDescRing.AllocVa)
    {
#ifdef DESC_32B_SUPPORT
		kfree(pAd->RxDescRing.AllocVa);
#else
		RTMP_FreeDescMemory(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->RxDescRing.AllocSize, pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocPa);
#endif /* DESC_32B_SUPPORT */
    }
    NdisZeroMemory(&pAd->RxDescRing, sizeof(RTMP_DMABUF));

	if (pAd->MgmtDescRing.AllocVa)
	{
		RTMP_FreeDescMemory(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->MgmtDescRing.AllocSize, pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->MgmtDescRing, sizeof(RTMP_DMABUF));

	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
    	if (pAd->TxBufSpace[num].AllocVa)
		{
			RTMP_FreeFirstTxBuffer(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxBufSpace[num].AllocSize, FALSE, pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBufSpace[num], sizeof(RTMP_DMABUF));
	    
    	if (pAd->TxDescRing[num].AllocVa)
		{
			RTMP_FreeDescMemory(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxDescRing[num].AllocSize, pAd->TxDescRing[num].AllocVa, pAd->TxDescRing[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxDescRing[num], sizeof(RTMP_DMABUF));
	}

	if (pAd->FragFrame.pFragPacket)
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);

	NdisFreeSpinLock(&pAd->CmdQLock);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}

#endif /* RESOURCE_PRE_ALLOC */


/*
	========================================================================
	
	Routine Description:
		Reset NIC Asics. Call after rest DMA. So reset TX_CTX_IDX to zero.

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		Reset NIC to initial state AS IS system boot up time.
		
	========================================================================
*/
VOID	RTMPRingCleanUp(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RingType)
{
	PTXD_STRUC		pTxD;
	PRXD_STRUC		pRxD;
#ifdef RT_BIG_ENDIAN
	PTXD_STRUC      pDestTxD;
	TXD_STRUC       TxD;
	PRXD_STRUC		pDestRxD;
	RXD_STRUC		RxD;
#endif /* RT_BIG_ENDIAN */
	PQUEUE_ENTRY	pEntry;
	PNDIS_PACKET	pPacket;
	int				i;
	PRTMP_TX_RING	pTxRing;
	unsigned long	IrqFlags;
	/*UINT32			RxSwReadIdx;*/


	DBGPRINT(RT_DEBUG_TRACE,("RTMPRingCleanUp(RingIdx=%d, Pending-NDIS=%ld)\n", RingType, pAd->RalinkCounters.PendingNdisPacketCount));
	switch (RingType)
	{
		case QID_AC_BK:
		case QID_AC_BE:
		case QID_AC_VI:
		case QID_AC_VO:
		case QID_HCCA:
			
			pTxRing = &pAd->TxRing[RingType];
			
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			/* We have to clean all descriptors in case some error happened with reset*/
			for (i=0; i<TX_RING_SIZE; i++) /* We have to scan all TX ring*/
			{
				pTxD  = (PTXD_STRUC) pTxRing->Cell[i].AllocVa;

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNdisPacket = NULL;
				}

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNextNdisPacket = NULL;
				}
			}

			RTMP_IO_READ32(pAd, TX_DTX_IDX0 + RingType * 0x10, &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + RingType * 0x10, pTxRing->TxCpuIdx);
			
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->TxSwQueue[RingType].Head != NULL)
			{
				pEntry = RemoveHeadQueue(&pAd->TxSwQueue[RingType]);
				pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				DBGPRINT(RT_DEBUG_TRACE,("Release 1 NDIS packet from s/w backlog queue\n"));
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			break;

		case QID_MGMT:
			/* We have to clean all descriptors in case some error happened with reset*/
			NdisAcquireSpinLock(&pAd->MgmtRingLock);
			
			for (i=0; i<MGMT_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (PTXD_STRUC) pAd->MgmtRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (PTXD_STRUC) pAd->MgmtRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNdisPacket;
				/* rlease scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->MgmtRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);			
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
				pAd->MgmtRing.Cell[i].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

			RTMP_IO_READ32(pAd, TX_MGMTDTX_IDX, &pAd->MgmtRing.TxDmaIdx);
			pAd->MgmtRing.TxSwFreeIdx = pAd->MgmtRing.TxDmaIdx;
			pAd->MgmtRing.TxCpuIdx = pAd->MgmtRing.TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_MGMTCTX_IDX, pAd->MgmtRing.TxCpuIdx);
			
			NdisReleaseSpinLock(&pAd->MgmtRingLock);
			pAd->RalinkCounters.MgmtRingFullCount = 0;
			break;
			
		case QID_RX:
			/* We have to clean all descriptors in case some error happened with reset*/
			NdisAcquireSpinLock(&pAd->RxRingLock);

			for (i=0; i<RX_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestRxD = (PRXD_STRUC) pAd->RxRing.Cell[i].AllocVa;
				RxD = *pDestRxD;
				pRxD = &RxD;
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
				/* Point to Rx indexed rx ring descriptor*/
				pRxD  = (PRXD_STRUC) pAd->RxRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

                pRxD->DDONE = 0 ;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
				WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif /* RT_BIG_ENDIAN */
			}
			 
			RTMP_IO_READ32(pAd, RX_DRX_IDX, &pAd->RxRing.RxDmaIdx);
			pAd->RxRing.RxSwReadIdx = pAd->RxRing.RxDmaIdx;
			pAd->RxRing.RxCpuIdx = ((pAd->RxRing.RxDmaIdx == 0) ? (RX_RING_SIZE-1) : (pAd->RxRing.RxDmaIdx-1));
			RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);
			 
			NdisReleaseSpinLock(&pAd->RxRingLock);
			break;
			
		default:
			break;
	}
}


/***************************************************************************
  *
  *	register related procedures.
  *
  **************************************************************************/
/*
========================================================================
Routine Description:
    Disable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMADisable(
	IN RTMP_ADAPTER 		*pAd)
{
	WPDMA_GLO_CFG_STRUC     GloCfg;


	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
}


/*
========================================================================
Routine Description:
    Enable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMAEnable(
	IN RTMP_ADAPTER 		*pAd)
{
	WPDMA_GLO_CFG_STRUC	GloCfg;
	int i = 0;
	
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x4);
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
			break;
		
		DBGPRINT(RT_DEBUG_TRACE, ("==>  DMABusy\n"));
		RTMPusecDelay(1000);
		i++;
	}while ( i <200);

	RTMPusecDelay(50);

	GloCfg.field.EnTXWriteBackDDONE = 1;
	GloCfg.field.WPDMABurstSIZE = pAd->chipCap.WPDMABurstSIZE;
	GloCfg.field.EnableRxDMA = 1;
	GloCfg.field.EnableTxDMA = 1;
	
	DBGPRINT(RT_DEBUG_TRACE, ("<== WRITE DMA offset 0x208 = 0x%x\n", GloCfg.word));	
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

}


BOOLEAN AsicCheckCommanOk(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command)
{
	UINT32	CmdStatus = 0, CID = 0, i;
	UINT32	ThisCIDMask = 0;
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	ULONG	IrqFlags = 0;

	RTMP_MAC_SHR_MSEL_PROTECT_LOCK(pAd, IrqFlags);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, H2M_MAILBOX_CID, &CID);
		/* Find where the command is. Because this is randomly specified by firmware.*/
		if ((CID & CID0MASK) == Command)
		{
			ThisCIDMask = CID0MASK;
			break;
		}
		else if ((((CID & CID1MASK)>>8) & 0xff) == Command)
		{
			ThisCIDMask = CID1MASK;
			break;
		}
		else if ((((CID & CID2MASK)>>16) & 0xff) == Command)
		{
			ThisCIDMask = CID2MASK;
			break;
		}
		else if ((((CID & CID3MASK)>>24) & 0xff) == Command)
		{
			ThisCIDMask = CID3MASK;
			break;
		}

		RTMPusecDelay(100);
		i++;
	}while (i < 200);

	/* Get CommandStatus Value*/
	RTMP_IO_READ32(pAd, H2M_MAILBOX_STATUS, &CmdStatus);
	
	/* This command's status is at the same position as command. So AND command position's bitmask to read status.	*/
	if (i < 200)
	{
		/* If Status is 1, the comamnd is success.*/
		if (((CmdStatus & ThisCIDMask) == 0x1) || ((CmdStatus & ThisCIDMask) == 0x100) 
			|| ((CmdStatus & ThisCIDMask) == 0x10000) || ((CmdStatus & ThisCIDMask) == 0x1000000))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanOk CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
#ifdef SPECIFIC_BCN_BUF_SUPPORT
			RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(pAd, IrqFlags);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
			return TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail1 CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail2 Timeout Command = %d, CmdStatus= 0x%x \n", Command, CmdStatus));
	}
	/* Clear Command and Status.*/
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
#ifdef SPECIFIC_BCN_BUF_SUPPORT
   	RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(pAd, IrqFlags);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
	return FALSE;
}


/*
========================================================================
Routine Description:
    Write Beacon buffer to Asic.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER		*pAd,
	IN INT				apidx,
	IN ULONG			FrameLen, 
	IN ULONG			UpdatePos)
{
	ULONG				CapInfoPos = 0;
	UCHAR  			*ptr, *ptr_update, *ptr_capinfo;
	UINT  			i;
	BOOLEAN			bBcnReq = FALSE;
	UCHAR			bcn_idx = 0;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


#ifdef CONFIG_AP_SUPPORT
	if (apidx < pAd->ApCfg.BssidNum &&
		apidx < HW_BEACON_MAX_NUM &&
#ifdef P2P_SUPPORT
		P2P_GO_ON(pAd)
#else
	       (pAd->OpMode == OPMODE_AP)
#endif /* P2P_SUPPORT */
		)
	{
		MULTISSID_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[apidx];
		bcn_idx = pMbss->BcnBufIdx;
		CapInfoPos = pMbss->CapabilityInfoLocationInBeacon;
		bBcnReq = BeaconTransmitRequired(pAd, apidx, pMbss);

		ptr_capinfo = (PUCHAR)&pMbss->BeaconBuf[CapInfoPos];
		ptr_update  = (PUCHAR)&pMbss->BeaconBuf[UpdatePos];
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() : No valid Interface be found.\n", __FUNCTION__));
		return;
	}

	if (bBcnReq == FALSE)
	{
		/* when the ra interface is down, do not send its beacon frame */
		/* clear all zero */
		for(i=0; i < TXWISize; i+=4)
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + i, 0x00, 4);
	}
	else
	{
		ptr = (PUCHAR)&pAd->BeaconTxWI;
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif
		for (i=0; i < TXWISize; i+=4)  /* 16-byte TXWI field*/
		{
			UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + i, longptr, 4);
			ptr += 4;
		}

		/* Update CapabilityInfo in Beacon*/
		for (i = CapInfoPos; i < (CapInfoPos+2); i++)
		{
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, *ptr_capinfo, 1);
			ptr_capinfo ++;
		}

		if (FrameLen > UpdatePos)
		{
			for (i= UpdatePos; i< (FrameLen); i++)
			{
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, *ptr_update, 1);
				ptr_update ++;
			}
		}
	}
}


#ifdef CONFIG_STA_SUPPORT
VOID RT28xxPciStaAsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       bFromTx)
{
    AUTO_WAKEUP_STRUC	AutoWakeupCfg;

    if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        return;

    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WAKEUP_NOW))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("waking up now!\n"));
        return;
    }

    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);

   /* RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_GO_TO_SLEEP_NOW);*/
#ifdef PCIE_PS_SUPPORT
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
    {
        /* Support PCIe Advance Power Save*/
    	if (bFromTx == TRUE
			&&(pAd->Mlme.bPsPollTimerRunning == TRUE))
    	{
            pAd->Mlme.bPsPollTimerRunning = FALSE;
    		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
    		RTMPusecDelay(3000);
            DBGPRINT(RT_DEBUG_TRACE, ("=======AsicForceWakeup===bFromTx\n"));
    	}

		AutoWakeupCfg.word = 0;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
	
        if (RT28xxPciAsicRadioOn(pAd, DOT11POWERSAVE))
        {
			RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
				
			if (pChipOps->AsicReverseRfFromSleepMode)
				pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);
			else
			{
			/* end johnli*/
				/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
				if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
					&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
				{	
					/* Must using 40MHz.*/
					AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
				}
				else
				{	
					/* Must using 20MHz.*/
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
				}
			} 
        }
    }
    else
#endif /* PCIE_PS_SUPPORT */
    {
        /* PCI, 2860-PCIe*/
         DBGPRINT(RT_DEBUG_TRACE, ("<==RT28xxPciStaAsicForceWakeup::Original PCI Power Saving\n"));
        AsicSendCommandToMcu(pAd, 0x31, 0xff, 0x00, 0x02, FALSE);
        AutoWakeupCfg.word = 0;
	    RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
    }
    
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);
    DBGPRINT(RT_DEBUG_TRACE, ("<=======RT28xxPciStaAsicForceWakeup\n"));
}


VOID RT28xxPciStaAsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TbttNumToNextWakeUp) 
{
#ifdef PCIE_PS_SUPPORT
	BOOLEAN brc;
#endif /* PCIE_PS_SUPPORT */
	
	if (pAd->StaCfg.bRadio == FALSE)
	{
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
		return;
	}
#ifdef PCIE_PS_SUPPORT
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
		ULONG	Now = 0;
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WAKEUP_NOW))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("waking up now!\n"));
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
			return;
		}

		NdisGetSystemUpTime(&Now);
		/* If last send NULL fram time is too close to this receiving beacon (within 8ms), don't go to sleep for this DTM.*/
		/* Because Some AP can't queuing outgoing frames immediately.*/
		if (((pAd->Mlme.LastSendNULLpsmTime + 8) >= Now) && (pAd->Mlme.LastSendNULLpsmTime <= Now))		
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Now = %lu, LastSendNULLpsmTime=%lu :  RxCountSinceLastNULL = %lu. \n", Now, pAd->Mlme.LastSendNULLpsmTime, pAd->RalinkCounters.RxCountSinceLastNULL));
			return;
		}
		else if ((pAd->RalinkCounters.RxCountSinceLastNULL > 0) && ((pAd->Mlme.LastSendNULLpsmTime + pAd->CommonCfg.BeaconPeriod) >= Now))		
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Now = %lu, LastSendNULLpsmTime=%lu: RxCountSinceLastNULL = %lu > 0 \n", Now, pAd->Mlme.LastSendNULLpsmTime,  pAd->RalinkCounters.RxCountSinceLastNULL));			
			return;
		}
		
		brc = RT28xxPciAsicRadioOff(pAd, DOT11POWERSAVE, TbttNumToNextWakeUp);
		if (brc==TRUE)
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
	}
	else
#endif /* PCIE_PS_SUPPORT */
	{
		AUTO_WAKEUP_STRUC	AutoWakeupCfg;	
		/* we have decided to SLEEP, so at least do it for a BEACON period.	*/
		if (TbttNumToNextWakeUp == 0)
			TbttNumToNextWakeUp = 1; 

		/*RTMP_IO_WRITE32(pAd, INT_MASK_CSR, AutoWakeupInt);*/

		AutoWakeupCfg.word = 0; 
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);  
		AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;    
		AutoWakeupCfg.field.EnableAutoWakeup = 1;   
		AutoWakeupCfg.field.AutoLeadTime = 5;   
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);  
		AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x00, FALSE);   /* send POWER-SAVE command to MCU. Timeout 40us.*/
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
		DBGPRINT(RT_DEBUG_TRACE, ("<-- %s, TbttNumToNextWakeUp=%d \n", __FUNCTION__, TbttNumToNextWakeUp));
	}

}

#ifdef PCIE_PS_SUPPORT
VOID PsPollWakeExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	unsigned long flags;

    DBGPRINT(RT_DEBUG_TRACE,("-->PsPollWakeExec \n"));

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
    if (pAd->Mlme.bPsPollTimerRunning)
    {
	    RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
    }
    pAd->Mlme.bPsPollTimerRunning = FALSE;
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

}


VOID  RadioOnExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	WPDMA_GLO_CFG_STRUC	DmaCfg;
	BOOLEAN				Cancelled;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("-->RadioOnExec() return on fOP_STATUS_DOZE == TRUE; \n"));
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
			&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
		return;
	}
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_TRACE,("-->RadioOnExec() return on SCAN_IN_PROGRESS; \n"));


		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
				&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
		return;
	}
	
if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
	&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
	pAd->Mlme.bPsPollTimerRunning = FALSE;
	RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
	}


	if (pAd->StaCfg.bRadio == TRUE)
	{
		pAd->bPCIclkOff = FALSE;
		RTMPRingCleanUp(pAd, QID_AC_BK);
		RTMPRingCleanUp(pAd, QID_AC_BE);
		RTMPRingCleanUp(pAd, QID_AC_VI);
		RTMPRingCleanUp(pAd, QID_AC_VO);
		RTMPRingCleanUp(pAd, QID_HCCA);
		RTMPRingCleanUp(pAd, QID_MGMT);
		RTMPRingCleanUp(pAd, QID_RX);

		/* 2. Send wake up command.*/
		AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);   
		/* 2-1. wait command ok.*/
		AsicCheckCommanOk(pAd, PowerWakeCID);	
        
		/* When PCI clock is off, don't want to service interrupt. So when back to clock on, enable interrupt.*/
		/*RTMP_IO_WRITE32(pAd, INT_MASK_CSR, (DELAYINTMASK|RxINT));*/
		RTMP_ASIC_INTERRUPT_ENABLE(pAd);
		
		/* 3. Enable Tx DMA.*/
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &DmaCfg.word);
		DmaCfg.field.EnableTxDMA = 1;
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, DmaCfg.word);
		
		/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
		if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
			&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
		{	
			/* Must using 40MHz.*/
			AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
		}
		else
		{	
			/* Must using 20MHz.*/
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.Channel);
		}
				
		if (pChipOps->AsicReverseRfFromSleepMode)
			pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);

		/* Clear Radio off flag*/
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

#ifdef LED_CONTROL_SUPPORT
		/* Set LED*/
		RTMPSetLED(pAd, LED_RADIO_ON);
#endif /* LED_CONTROL_SUPPORT */

		if (RtmpPktPmBitCheck(pAd) == FALSE)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
		}
	}
	else
	{
		RT28xxPciAsicRadioOff(pAd, GUIRADIO_OFF, 0);
	}
}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to wake up mode from power save mode.
		Both RadioOn and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF : call this function is from Radio Off to Radio On.  Need to restore PCI host value.
		Level = other value : normal wake up function.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOn(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR     Level)
{
    /*WPDMA_GLO_CFG_STRUC	DmaCfg;*/
#ifdef CONFIG_STA_SUPPORT    
#ifdef PCIE_PS_SUPPORT
	BOOLEAN				Cancelled;   
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
    /*UINT32			    MACValue;*/

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
	{
		if (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		{
	    	pAd->Mlme.bPsPollTimerRunning = FALSE;
			RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
		}
		if ((pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)&&
			(((Level == GUIRADIO_OFF) || (Level == GUI_IDLE_POWER_SAVE))
			||(RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND))))
		{
			/* Some chips don't need to delay 6ms, so copy RTMPPCIePowerLinkCtrlRestore*/
			/* return condition here.*/
			/*
			if (((pAd->MACVersion&0xffff0000) != 0x28600000)
				&& ((pAd->DeviceID == NIC2860_PCIe_DEVICE_ID)
				||(pAd->DeviceID == NIC2790_PCIe_DEVICE_ID)))
			*/
			{
				DBGPRINT(RT_DEBUG_TRACE, ("RT28xxPciAsicRadioOn ()\n"));
				/* 1. Set PCI Link Control in Configuration Space.*/
				RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
				RTMPusecDelay(6000);
			}
		}
	}
    
	{	
    		pAd->bPCIclkOff = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("PSM :309xbPCIclkOff == %d\n", pAd->bPCIclkOff));
	}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	/* 2. Send wake up command.*/
	AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);
    pAd->bPCIclkOff = FALSE;
	/* 2-1. wait command ok.*/
	AsicCheckCommanOk(pAd, PowerWakeCID);	
    	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
        
    	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
    	if (Level == GUI_IDLE_POWER_SAVE)
    	{
 /*2009/06/09: AP and stations need call the following function*/
 #ifndef RTMP_RBUS_SUPPORT
			/* add by johnli, RF power sequence setup, load RF normal operation-mode setup*/

				RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
				
				if (pChipOps->AsicReverseRfFromSleepMode)
				{
					pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
				/* 3090 MCU Wakeup command needs more time to be stable.*/
				/* Before stable, don't issue other MCU command to prevent from firmware error.	*/
			
                                if ( pAd->chipCap.HW_PCIE_PS_L3_ENABLE==TRUE)
				{
					RTMP_SEM_LOCK(&pAd->McuCmdLock);
					pAd->brt30xxBanMcuCmd = FALSE;
					RTMP_SEM_UNLOCK(&pAd->McuCmdLock);
				}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

			}
			else
			/* end johnli*/
#endif /* RTMP_RBUS_SUPPORT */
			{
	    		/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
	    			if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
	    				&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
	    			{	
	    				/* Must using 40MHz.*/
	    				AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
	    				AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
	    			}
	    			else
	    			{	
	    				/* Must using 20MHz.*/
	    				AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	    				AsicLockChannel(pAd, pAd->CommonCfg.Channel);
	    			}
				}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
				}
#endif /* CONFIG_AP_SUPPORT */
			}
    	}
        return TRUE;

}


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to power save mode.
		Both RadioOff and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF  : GUI Radio Off mode
		Level = DOT11POWERSAVE  : 802.11 power save mode 
		Level = RTMP_HALT  : When Disable device. 
		
	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOff(
	IN PRTMP_ADAPTER    pAd,
	IN UCHAR            Level, 
	IN USHORT           TbttNumToNextWakeUp) 
{
#ifdef CONFIG_STA_SUPPORT	
	WPDMA_GLO_CFG_STRUC	DmaCfg;
	UCHAR		i, tempBBP_R3 = 0;
#ifdef PCIE_PS_SUPPORT	
    ULONG		BeaconPeriodTime;
	UINT32		PsPollTime = 0/*, MACValue*/;
	UINT32		TbTTTime = 0;
	BOOLEAN		Cancelled;
#endif /* PCIE_PS_SUPPORT */	
#endif /* CONFIG_STA_SUPPORT */
#if defined(CONFIG_STA_SUPPORT) || defined(RT2860)
	BOOLEAN		brc = FALSE;
#endif /* defined(CONFIG_STA_SUPPORT) || defined(RT2860) */


    UINT32		RxDmaIdx, RxCpuIdx;
	DBGPRINT(RT_DEBUG_TRACE, ("%s ===> Lv= %d, TxCpuIdx = %d, TxDmaIdx = %d. RxCpuIdx = %d, RxDmaIdx = %d.\n", 
								__FUNCTION__, Level,pAd->TxRing[0].TxCpuIdx, pAd->TxRing[0].TxDmaIdx, pAd->RxRing.RxCpuIdx, pAd->RxRing.RxDmaIdx));

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;

	if (Level == DOT11POWERSAVE)
	{
	    /* Check Rx DMA busy status, if more than half is occupied, give up this radio off.*/
		RTMP_IO_READ32(pAd, RX_DRX_IDX , &RxDmaIdx);
		RTMP_IO_READ32(pAd, RX_CRX_IDX , &RxCpuIdx);
		if ((RxDmaIdx > RxCpuIdx) && ((RxDmaIdx - RxCpuIdx) > RX_RING_SIZE/3))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff ===> return1. RxDmaIdx = %d ,  RxCpuIdx = %d. \n", RxDmaIdx, RxCpuIdx));
			return FALSE;
		}
		else if ((RxCpuIdx >= RxDmaIdx) && ((RxCpuIdx - RxDmaIdx) < RX_RING_SIZE/3))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff ===> return2.  RxCpuIdx = %d. RxDmaIdx = %d ,  \n", RxCpuIdx, RxDmaIdx));
			return FALSE;
		}
	}


    /* Once go into this function, disable tx because don't want too many packets in queue to prevent HW stops.*/
	/*pAd->bPCIclkOffDisableTx = TRUE;*/
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	RTMP_SET_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE) 
		&& pAd->OpMode == OPMODE_STA
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE
		)
	{

	    RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
	    RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);

	    if (Level == DOT11POWERSAVE)
		{
			RTMP_IO_READ32(pAd, TBTT_TIMER, &TbTTTime);
			TbTTTime &= 0x1ffff;
			/* 00. check if need to do sleep in this DTIM period.   If next beacon will arrive within 30ms , ...doesn't necessarily sleep.*/
			/* TbTTTime uint = 64us, LEAD_TIME unit = 1024us, PsPollTime unit = 1ms*/
	        if  (((64*TbTTTime) <((LEAD_TIME*1024) + 40000)) && (TbttNumToNextWakeUp == 0))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("TbTTTime = 0x%x , give up this sleep. \n", TbTTTime));
	            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
	            		/*pAd->bPCIclkOffDisableTx = FALSE;*/
	            		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
				return FALSE;
			}
			else
			{
				PsPollTime = (64*TbTTTime- LEAD_TIME*1024)/1000;
				PsPollTime -= 3;

	            BeaconPeriodTime = pAd->CommonCfg.BeaconPeriod*102/100;
				if (TbttNumToNextWakeUp > 0)
					PsPollTime += ((TbttNumToNextWakeUp -1) * BeaconPeriodTime);
	            
	            pAd->Mlme.bPsPollTimerRunning = TRUE;
				RTMPSetTimer(&pAd->Mlme.PsPollTimer, PsPollTime);
				}
			}
		}
    	else
    	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT28xxPciAsicRadioOff::Level!=DOT11POWERSAVE \n"));
    	}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	/*pAd->bPCIclkOffDisableTx = FALSE;*/
    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
    
#ifdef CONFIG_STA_SUPPORT
    /* Set to 1R.*/
	if (pAd->Antenna.field.RxPath > 1 && pAd->OpMode == OPMODE_STA)
	{
    	tempBBP_R3 = (pAd->StaCfg.BBPR3 & 0xE7);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, tempBBP_R3);
	}
#endif /* CONFIG_STA_SUPPORT */
    
	/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
	if ((INFRA_ON(pAd) || pAd->OpMode == OPMODE_AP) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
		&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
	{	
		/* Must using 40MHz.*/
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.CentralChannel);
	}
	else
	{	
		/* Must using 20MHz.*/
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.Channel);
	}

	if (Level != RTMP_HALT)
	{
		/* Change Interrupt bitmask.*/
    /* When PCI clock is off, don't want to service interrupt.*/
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, AutoWakeupInt);
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}

    
	RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);
	/*  2. Send Sleep command */
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);    
	/* send POWER-SAVE command to MCU. high-byte = 1 save power as much as possible. high byte = 0 save less power*/
	AsicSendCommandToMcu(pAd, SLEEP_MCU_CMD, PowerSafeCID, 0xff, 0x1, FALSE);   

#ifdef RT2860
	/* in rt3xxx, after issue SLEEP command, can't read/write register. So don't check Command ok.*/
	if ((pAd->DeviceID == NIC2860_PCIe_DEVICE_ID) 
		||(pAd->DeviceID == NIC2790_PCIe_DEVICE_ID))
	{
		/*  2-1. Wait command success*/
		/* Status = 1 : success, Status = 2, already sleep, Status = 3, Maybe MAC is busy so can't finish this task.*/
		brc = AsicCheckCommanOk(pAd, PowerSafeCID);	

		/*  3. After 0x30 command is ok, send radio off command. lowbyte = 0 for power safe.*/
		/* If 0x30 command is not ok this time, we can ignore 0x35 command. It will make sure not cause firmware'r problem.*/
		if ((Level == DOT11POWERSAVE) && (brc == TRUE))
		{
			AsicSendCommandToMcu(pAd, 0x35, PowerRadioOffCID, 0, 0x00, FALSE);	/* lowbyte = 0 means to do power safe, NOT turn off radio.*/
		 	/*  3-1. Wait command success*/
		 	AsicCheckCommanOk(pAd, PowerRadioOffCID);
		}
		else if (brc == TRUE)
		{
			AsicSendCommandToMcu(pAd, 0x35, PowerRadioOffCID, 1, 0x00, FALSE);	/* lowbyte = 0 means to do power safe, NOT turn off radio.*/
		 	/*  3-1. Wait command success*/
		 	AsicCheckCommanOk(pAd, PowerRadioOffCID);
		}
	}
#endif /* RT2860 */
    
#ifdef CONFIG_STA_SUPPORT
	/* 1. Wait DMA not busy*/
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &DmaCfg.word);
		if ((DmaCfg.field.RxDMABusy == 0) && (DmaCfg.field.TxDMABusy == 0))
			break;
		RTMPusecDelay(20);
		i++;
	}while(i < 50);

	/*
	if (i >= 50)
	{
		pAd->CheckDmaBusyCount++;
		DBGPRINT(RT_DEBUG_TRACE, ("DMA Rx keeps busy.  return on AsicRadioOff () CheckDmaBusyCount = %d \n", pAd->CheckDmaBusyCount));
	}	
	else
	{
		pAd->CheckDmaBusyCount = 0;
	}
	*/
#endif /* CONFIG_STA_SUPPORT */
/* Disable for stability. If PCIE Link Control is modified for advance power save, re-covery this code segment.*/
/*RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x1280);*/
/*OPSTATUS_SET_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ);*/

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	if (Level == DOT11POWERSAVE)
	{
		AUTO_WAKEUP_STRUC	AutoWakeupCfg;
		/*RTMPSetTimer(&pAd->Mlme.PsPollTimer, 90);*/
			
		/* we have decided to SLEEP, so at least do it for a BEACON period.*/
		if (TbttNumToNextWakeUp == 0)
			TbttNumToNextWakeUp = 1;

		AutoWakeupCfg.word = 0;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

		/* 1. Set auto wake up timer.*/
		AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;
		AutoWakeupCfg.field.EnableAutoWakeup = 1;
		AutoWakeupCfg.field.AutoLeadTime = LEAD_TIME;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
	}
	
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT

	/*  4-1. If it's to disable our device. Need to restore PCI Configuration Space to its original value.*/
	if (Level == RTMP_HALT && pAd->OpMode == OPMODE_STA)
	{
		if ((brc == TRUE) && (i < 50))
			RTMPPCIeLinkCtrlSetting(pAd, 1);
	}
	/*  4. Set PCI configuration Space Link Comtrol fields.  Only Radio Off needs to call this function*/
	else if (pAd->OpMode == OPMODE_STA)
	{
		if ((brc == TRUE) && (i < 50))
			RTMPPCIeLinkCtrlSetting(pAd, 3);
	}
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

    	/*pAd->bPCIclkOffDisableTx = FALSE;*/
    	
	return TRUE;
}




VOID RT28xxPciMlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{    
    if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
   		return;
    
    DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));   
    
    if ((pAd->OpMode == OPMODE_AP) ||
        ((pAd->OpMode == OPMODE_STA) 
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
        && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
        ||pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
 #endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
        ))
    {
    	RTMPRingCleanUp(pAd, QID_AC_BK);
    	RTMPRingCleanUp(pAd, QID_AC_BE);
    	RTMPRingCleanUp(pAd, QID_AC_VI);
    	RTMPRingCleanUp(pAd, QID_AC_VO);
    	RTMPRingCleanUp(pAd, QID_HCCA);
    	RTMPRingCleanUp(pAd, QID_MGMT);
    	RTMPRingCleanUp(pAd, QID_RX);

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
		NICResetFromError(pAd);
#endif /* RTMP_RBUS_SUPPORT */
#ifdef RTMP_PCI_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
		RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
#endif /* RTMP_PCI_SUPPORT */

    	/* Enable Tx/Rx*/
    	RTMPEnableRxTx(pAd);
    	
    	/* Clear Radio off flag*/
    	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

#ifdef LED_CONTROL_SUPPORT
	    /* Set LED*/
	    RTMPSetLED(pAd, LED_RADIO_ON);
#ifdef CONFIG_AP_SUPPORT
		/* The LEN_RADIO_ON indicates "Radio on but link down", 
		   so AP shall set LED LINK_UP status */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	    	RTMPSetLED(pAd, LED_LINK_UP);
#endif /* CONFIG_AP_SUPPORT */
#endif /* LED_CONTROL_SUPPORT */
    }

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT

    if ((pAd->OpMode == OPMODE_STA) &&
        (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
        &&(pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
    {
        BOOLEAN		Cancelled;
    	
    	RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);

        pAd->Mlme.bPsPollTimerRunning = FALSE;
    	RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
    	RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
    	RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 40);
    }
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
}


VOID RT28xxPciMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd)
{
#ifdef RTMP_PCI_SUPPORT
	BOOLEAN brc=TRUE;
#endif /* RTMP_PCI_SUPPORT */

    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    	return;
    
    DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));

	/* Set Radio off flag*/
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
    {
    	BOOLEAN		Cancelled;
    	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
    	{
			RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
    	}

#ifdef PCIE_PS_SUPPORT
		/* If during power safe mode. */
		if ((pAd->StaCfg.bRadio == TRUE)
			&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
		{
			DBGPRINT(RT_DEBUG_TRACE,("-->MlmeRadioOff() return on bRadio == TRUE; \n"));
			return;
		}
#endif /* PCIE_PS_SUPPORT */

		/* Always radio on since the NIC needs to set the MCU command (LED_RADIO_OFF).*/
#ifdef RTMP_PCI_SUPPORT
		if ((pAd->infType == RTMP_DEV_INF_PCI)  || 
			(pAd->infType == RTMP_DEV_INF_PCIE))
		{
			if (IDLE_ON(pAd) &&
				(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
			{
				RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
			}
		}
#endif /* RTMP_PCI_SUPPORT */

#ifdef PCIE_PS_SUPPORT
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
        {
            BOOLEAN Cancelled;
            pAd->Mlme.bPsPollTimerRunning = FALSE;
            RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
	        RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
        }
#endif /* PCIE_PS_SUPPORT */
		

        /* Link down first if any association exists	*/
        if (INFRA_ON(pAd) || ADHOC_ON(pAd))		
            LinkDown(pAd, FALSE);   
        RTMPusecDelay(10000);   
        /*==========================================    */
        /* Clean up old bss table   */
        BssTableInit(&pAd->ScanTab);
		
        /*
        if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
        {
            RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
            return;
        }
        */
    }
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	{
		BOOLEAN		Cancelled;

		RTMPCancelTimer(&pAd->MlmeAux.APScanTimer, &Cancelled);
	}
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF);
#endif /* LED_CONTROL_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
/*Q:Does all PCIe devices need to use timer to execute radio off function? or only if the device is PCIe and EnableNewPS is true ?*/
/*A:It is right, because only when the PCIe and EnableNewPs is true, we need to delay the RadioOffTimer*/
/*to avoid the deadlock with PCIe Power saving function. */
if (pAd->OpMode == OPMODE_STA&&
	OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)&&
	pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10); 
	} 
else 
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef RTMP_PCI_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
{
		brc=RT28xxPciAsicRadioOff(pAd, GUIRADIO_OFF, 0);
	if (brc==FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__)); 
	}
}
#endif /* RTMP_PCI_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
		int	i;
		WPDMA_GLO_CFG_STRUC	GloCfg;
		UINT32	macStatus = 0, MTxCycle = 0;

		/* Disable Tx/Rx DMA*/
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	   /* disable DMA */
		GloCfg.field.EnableTxDMA = 0;
		GloCfg.field.EnableRxDMA = 0;
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);	   /* abort all TX rings*/

		// Wait up to 1000ms for DMA idle
		i = 0;
		do
		{
			RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
			if ((GloCfg.field.TxDMABusy == 0) && (GloCfg.field.RxDMABusy == 0))
				break;
			
			RTMPusecDelay(1000);
		}while (i++ < 1000);

		/* MAC_SYS_CTRL => value = 0x0 => 40mA*/
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0);

		// Check MAC Tx/Rx idle
		for (MTxCycle = 0; MTxCycle < 20000; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
			if (macStatus & 0x3)
				RTMPusecDelay(100);
			else
				break;
		}

		if (MTxCycle == 20000)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Polling Mac Status to MAX !!!\n"));
		}

		/* PWR_PIN_CFG => value = 0x0 => 40mA*/
		RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0);
		
		/* TX_PIN_CFG => value = 0x0 => 20mA*/
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x00010000);

		if (pAd->CommonCfg.BBPCurrentBW == BW_40)
		{	
			/* Must using 40MHz.*/
			AsicTurnOffRFClk(pAd, pAd->CommonCfg.CentralChannel);
		}
		else
		{	
			/* Must using 20MHz.*/
			AsicTurnOffRFClk(pAd, pAd->CommonCfg.Channel);
		}
}
#endif /* RTMP_RBUS_SUPPORT */
}


/*
========================================================================
Routine Description:
	Get a pci map buffer.

Arguments:
	pAd				- WLAN control block pointer
	*ptr			- Virtual address or TX control block
	size			- buffer size
	sd_idx			- 1: the ptr is TX control block
	direction		- RTMP_PCI_DMA_TODEVICE or RTMP_PCI_DMA_FROMDEVICE

Return Value:
	the PCI map buffer

Note:
========================================================================
*/
ra_dma_addr_t RtmpDrvPciMapSingle(
	IN	PRTMP_ADAPTER			pAd,
	IN	VOID					*ptr,
	IN	size_t					size,
	IN	int						sd_idx,
	IN	int						direction)
{
	if (sd_idx == 1)
	{
		PTX_BLK		pTxBlk;
		pTxBlk = (PTX_BLK)(ptr);
		return (pTxBlk->SrcBufLen)?linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					pTxBlk->pSrcBufData, pTxBlk->SrcBufLen, 0, direction):(ra_dma_addr_t)NULL;
	}
	else
		return linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					ptr, size, 0, direction);
}

#endif /* RTMP_MAC_PCI */

