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
	cmm_usb_io.c
*/
#include	"rt_config.h"

NTSTATUS RTUSBReadEEPROM(RTMP_ADAPTER *pAd, USHORT adr, UCHAR *buf, USHORT len)
{
	return RTUSB_VendorRequest(pAd, (USBD_TRANSFER_DIRECTION_IN |
									USBD_SHORT_TRANSFER_OK),
								DEVICE_VENDOR_REQUEST_IN,
								0x9, 0, adr, buf, len);
}


NTSTATUS RTUSBWriteEEPROM(RTMP_ADAPTER *pAd, USHORT adr, UCHAR *buf, USHORT len)
{
	return RTUSB_VendorRequest(pAd, USBD_TRANSFER_DIRECTION_OUT,
								DEVICE_VENDOR_REQUEST_OUT,
								0x8, 0, adr, buf, len);
}


BOOLEAN RTUSBReadEEPROM16(RTMP_ADAPTER *pAd, USHORT offset, USHORT *pData)
{
	NTSTATUS status;
	UINT16  localData;
	BOOLEAN IsEmpty = 0;

	status = RTUSBReadEEPROM(pAd, offset, (PUCHAR)(&localData), 2);
	if (status == STATUS_SUCCESS)
		*pData = le2cpu16(localData);

	if ((*pData == 0xffff) || (*pData == 0x0000))
		IsEmpty = 1;

	return IsEmpty;
}


NTSTATUS RTUSBWriteEEPROM16(RTMP_ADAPTER *pAd, USHORT offset, USHORT value)
{
	USHORT tmpVal;

	tmpVal = cpu2le16(value);
	return RTUSBWriteEEPROM(pAd, offset, (PUCHAR)&(tmpVal), 2);
}

#define MAX_VENDOR_REQ_RETRY_COUNT  10

/*
    ========================================================================
 	Routine Description:
		RTUSB_VendorRequest - Builds a ralink specific request, sends it off to USB endpoint zero and waits for completion

	Arguments:
		@pAd:
	  	@TransferFlags:
	  	@RequestType: USB message request type value
	  	@Request: USB message request value
	  	@Value: USB message value
	  	@Index: USB message index value
	  	@TransferBuffer: USB data to be sent
	  	@TransferBufferLength: Lengths in bytes of the data to be sent

	Context: ! in atomic context

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	Note:
		This function sends a simple control message to endpoint zero
		and waits for the message to complete, or CONTROL_TIMEOUT_JIFFIES timeout.
		Because it is synchronous transfer, so don't use this function within an atomic context,
		otherwise system will hang, do be careful.

		TransferBuffer may located in stack region which may not in DMA'able region in some embedded platforms,
		so need to copy TransferBuffer to UsbVendorReqBuf allocated by kmalloc to do DMA transfer.
		Use UsbVendorReq_semaphore to protect this region which may be accessed by multi task.
		Normally, coherent issue is resloved by low-level HC driver, so do not flush this zone by RTUSB_VendorRequest.

	========================================================================
*/
NTSTATUS RTUSB_VendorRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			TransferFlags,
	IN	UCHAR			RequestType,
	IN	UCHAR			Request,
	IN	USHORT			Value,
	IN	USHORT			Index,
	IN	PVOID			TransferBuffer,
	IN	UINT32			TransferBufferLength)
{
	int				RET = 0;
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;

	if(in_interrupt())
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BUG: RTUSB_VendorRequest is called from invalid context\n"));
		return NDIS_STATUS_FAILURE;
	}

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND))
	{
		return NDIS_STATUS_FAILURE;
	}
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WIFI device has been disconnected\n"));*/
		return NDIS_STATUS_FAILURE;
	}
	else if (RTMP_TEST_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MCU has entered sleep mode\n"));
		return NDIS_STATUS_FAILURE;
	}
	else
	{
		int RetryCount = 0; /* RTUSB_CONTROL_MSG retry counts*/
		ASSERT(TransferBufferLength <MAX_PARAM_BUFFER_SIZE);

		RTMP_SEM_EVENT_WAIT(&(pAd->UsbVendorReq_semaphore), RET);
		if (RET != 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("UsbVendorReq_semaphore get failed! (%d)\n",RET));
			return NDIS_STATUS_FAILURE;
		}

		if ((TransferBufferLength > 0) && ((RequestType == DEVICE_VENDOR_REQUEST_OUT) || (RequestType == DEVICE_CLASS_REQUEST_OUT)))
			NdisMoveMemory(pAd->UsbVendorReqBuf, TransferBuffer, TransferBufferLength);

		do {
			RTUSB_CONTROL_MSG(pObj->pUsb_Dev, 0, Request, RequestType, Value,
								Index, pAd->UsbVendorReqBuf, TransferBufferLength,
								CONTROL_TIMEOUT_JIFFIES, RET);

			if (RET < 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("#\n"));
				if (RET == RTMP_USB_CONTROL_MSG_ENODEV)
				{
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
					break;
				}
				RetryCount++;
				RtmpusecDelay(5000); /* wait for 5ms*/
			}
		} while((RET < 0) && (RetryCount < MAX_VENDOR_REQ_RETRY_COUNT));

	  	if ( (!(RET < 0)) && (TransferBufferLength > 0) && (RequestType == DEVICE_VENDOR_REQUEST_IN))
			NdisMoveMemory(TransferBuffer, pAd->UsbVendorReqBuf, TransferBufferLength);

	  	RTMP_SEM_EVENT_UP(&(pAd->UsbVendorReq_semaphore));

        	if (RET < 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTUSB_VendorRequest failed(%d),TxFlags=0x%x, ReqType=%s, Req=0x%x, Idx=0x%x,pAd->Flags=0x%lx\n",
						RET, TransferFlags, (RequestType == DEVICE_VENDOR_REQUEST_OUT ? "OUT" : "IN"), Request, Index, pAd->Flags));
			if (Request == 0x2)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tRequest Value=0x%04x!\n", Value));

			if ((!TransferBuffer) && (TransferBufferLength > 0))
				hex_dump("Failed TransferBuffer value", TransferBuffer, TransferBufferLength);

			if (RET == RTMP_USB_CONTROL_MSG_ENODEV)
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);

		}

	}

	if (RET < 0)
		return NDIS_STATUS_FAILURE;
	else
		return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS	RTUSBEnqueueCmdFromNdis(
	IN	PRTMP_ADAPTER	pAd,
	IN	NDIS_OID		Oid,
	IN	BOOLEAN			SetInformation,
	IN	PVOID			pInformationBuffer,
	IN	UINT32			InformationBufferLength)
{
	NDIS_STATUS	status;
	PCmdQElmt	cmdqelmt = NULL;
	RTMP_OS_TASK	*pTask = &pAd->cmdQTask;


	RTMP_OS_TASK_LEGALITY(pTask)
		;
	else
		return (NDIS_STATUS_RESOURCES);

	status = os_alloc_mem(pAd, (PUCHAR *)(&cmdqelmt), sizeof(CmdQElmt));
	if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt == NULL))
		return (NDIS_STATUS_RESOURCES);

		cmdqelmt->buffer = NULL;
		if (pInformationBuffer != NULL)
		{
			status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, InformationBufferLength);
			if ((status != NDIS_STATUS_SUCCESS) || (cmdqelmt->buffer == NULL))
			{
				os_free_mem(NULL, cmdqelmt);
				return (NDIS_STATUS_RESOURCES);
			}
			else
			{
				NdisMoveMemory(cmdqelmt->buffer, pInformationBuffer, InformationBufferLength);
				cmdqelmt->bufferlength = InformationBufferLength;
			}
		}
		else
			cmdqelmt->bufferlength = 0;

	cmdqelmt->command = Oid;
	cmdqelmt->CmdFromNdis = TRUE;
	if (SetInformation == TRUE)
		cmdqelmt->SetOperation = TRUE;
	else
		cmdqelmt->SetOperation = FALSE;

	NdisAcquireSpinLock(&pAd->CmdQLock);
	if (pAd->CmdQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)
	{
		EnqueueCmd((&pAd->CmdQ), cmdqelmt);
		status = NDIS_STATUS_SUCCESS;
	}
	else
	{
		status = NDIS_STATUS_FAILURE;
	}
	NdisReleaseSpinLock(&pAd->CmdQLock);

	if (status == NDIS_STATUS_FAILURE)
	{
		if (cmdqelmt->buffer)
			os_free_mem(pAd, cmdqelmt->buffer);
		os_free_mem(pAd, cmdqelmt);
	}
	else
	RTCMDUp(&pAd->cmdQTask);


    return(NDIS_STATUS_SUCCESS);
}

NTSTATUS CheckGPIOHdlr(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt)
{
#ifdef CONFIG_ATE
		if (ATE_ON(pAd))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The driver is in ATE mode now\n"));
			return NDIS_STATUS_SUCCESS;
		}
#endif /* CONFIG_ATE */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			UINT32 data;
			
			/* Read GPIO pin2 as Hardware controlled radio state*/
			RTMP_IO_READ32( pAd, GPIO_CTRL_CFG, &data);
			pAd->StaCfg.bHwRadio = (data & 0x04) ? TRUE : FALSE;

			if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
			{
				pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!! Radio %s !!!\n",
								(pAd->StaCfg.bRadio == TRUE ? "On" : "Off")));
				if (pAd->StaCfg.bRadio == TRUE)
				{
					MlmeRadioOn(pAd);
					pAd->ExtraInfo = EXTRA_INFO_CLEAR;
				}
				else
				{
					MlmeRadioOff(pAd);
					pAd->ExtraInfo = HW_RADIO_OFF;
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS ResetBulkOutHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
#ifndef MT_MAC
	INT32 MACValue = 0;
	UCHAR Index = 0;
	int ret=0;
	PHT_TX_CONTEXT	pHTTXContext;
	ULONG IrqFlags = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CMDTHREAD_RESET_BULK_OUT(ResetPipeid=0x%0x)===>\n", pAd->bulkResetPipeid));

	/* All transfers must be aborted or cancelled before attempting to reset the pipe. */
	/*RTUSBCancelPendingBulkOutIRP(pAd);*/
	/* Wait 10ms to let previous packet that are already in HW FIFO to clear. by MAXLEE 12-25-2007*/
	do
	{
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			break;

		RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
		if ((MACValue & 0xf00000/*0x800000*/) == 0)
			break;

		Index++;
		RtmpusecDelay(10000);
	}while(Index < 100);

	USB_CFG_READ(pAd, &MACValue);

	/* 2nd, to prevent Read Register error, we check the validity.*/
	if ((MACValue & 0xc00000) == 0)
		USB_CFG_READ(pAd, &MACValue);

	/* 3rd, to prevent Read Register error, we check the validity.*/
	if ((MACValue & 0xc00000) == 0)
		USB_CFG_READ(pAd, &MACValue);

	MACValue |= 0x80000;
	USB_CFG_WRITE(pAd, MACValue);

	/* Wait 1ms to prevent next URB to bulkout before HW reset. by MAXLEE 12-25-2007*/
	RtmpusecDelay(1000);

	MACValue &= (~0x80000);
	USB_CFG_WRITE(pAd, MACValue);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tSet 0x2a0 bit19. Clear USB DMA TX path\n"));

	/* Wait 5ms to prevent next URB to bulkout before HW reset. by MAXLEE 12-25-2007*/
	/*RtmpusecDelay(5000);*/

	if ((pAd->bulkResetPipeid & BULKOUT_MGMT_RESET_FLAG) == BULKOUT_MGMT_RESET_FLAG)
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

		if (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE /* pMLMEContext->bWaitingBulkOut == TRUE */)
			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);

		RTUSBKickBulkOut(pAd);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tTX MGMT RECOVER Done!\n"));
	}
	else
	{
		pHTTXContext = &(pAd->TxContext[pAd->bulkResetPipeid]);

		/*NdisAcquireSpinLock(&pAd->BulkOutLock[pAd->bulkResetPipeid]);*/
		RTMP_INT_LOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
		if ( pAd->BulkOutPending[pAd->bulkResetPipeid] == FALSE)
		{
			pAd->BulkOutPending[pAd->bulkResetPipeid] = TRUE;
			pHTTXContext->IRPPending = TRUE;
			pAd->watchDogTxPendingCnt[pAd->bulkResetPipeid] = 1;

			/* no matter what, clean the flag*/
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

			/*NdisReleaseSpinLock(&pAd->BulkOutLock[pAd->bulkResetPipeid]);*/
			RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

#ifdef CONFIG_ATE
			if (ATE_ON(pAd))
				ret = ATEResetBulkOut(pAd);
			else
#endif /* CONFIG_ATE */
			{
				RTUSBInitHTTxDesc(pAd, pHTTXContext, pAd->bulkResetPipeid,
													pHTTXContext->BulkOutSize,
													RtmpUsbBulkOutDataPacketComplete);

				if ((ret = RTUSB_SUBMIT_URB(pHTTXContext->pUrb))!=0)
				{
						RTMP_INT_LOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);
						pAd->BulkOutPending[pAd->bulkResetPipeid] = FALSE;
						pHTTXContext->IRPPending = FALSE;
						pAd->watchDogTxPendingCnt[pAd->bulkResetPipeid] = 0;
						RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMDTHREAD_RESET_BULK_OUT:Submit Tx URB failed %d\n", ret));
				}
				else
				{
						RTMP_INT_LOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\tCMDTHREAD_RESET_BULK_OUT: TxContext[%d]:CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d, pending=%d!\n",
											pAd->bulkResetPipeid, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition,
											pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad,
											pAd->BulkOutPending[pAd->bulkResetPipeid]));
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\t\tBulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n",
											pAd->BulkOutReq, pAd->BulkOutComplete, pAd->BulkOutCompleteOther));

						RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tCMDTHREAD_RESET_BULK_OUT: Submit Tx DATA URB for failed BulkReq(0x%lx) Done, status=%d!\n",
											pAd->bulkResetReq[pAd->bulkResetPipeid],
											RTMP_USB_URB_STATUS_GET(pHTTXContext->pUrb)));
				}
			}
		}
		else
		{
			/*NdisReleaseSpinLock(&pAd->BulkOutLock[pAd->bulkResetPipeid]);*/
			/*RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);*/

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CmdThread : TX DATA RECOVER FAIL for BulkReq(0x%lx) because BulkOutPending[%d] is TRUE!\n",
								pAd->bulkResetReq[pAd->bulkResetPipeid], pAd->bulkResetPipeid));

			if (pAd->bulkResetPipeid == 0)
			{
				UCHAR	pendingContext = 0;
				PHT_TX_CONTEXT pHTTXContext = (PHT_TX_CONTEXT)(&pAd->TxContext[pAd->bulkResetPipeid ]);
				PTX_CONTEXT pMLMEContext = (PTX_CONTEXT)(pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa);
				PTX_CONTEXT pNULLContext = (PTX_CONTEXT)(&pAd->PsPollContext);
				PTX_CONTEXT pPsPollContext = (PTX_CONTEXT)(&pAd->NullContext);

				if (pHTTXContext->IRPPending)
					pendingContext |= 1;
				else if (pMLMEContext->IRPPending)
					pendingContext |= 2;
				else if (pNULLContext->IRPPending)
					pendingContext |= 4;
				else if (pPsPollContext->IRPPending)
					pendingContext |= 8;
				else
					pendingContext = 0;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tTX Occupied by %d!\n", pendingContext));
			}

			/* no matter what, clean the flag*/
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);

			RTMP_INT_UNLOCK(&pAd->BulkOutLock[pAd->bulkResetPipeid], IrqFlags);

			RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << pAd->bulkResetPipeid));
		}

		if (!(RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF |
				fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))))
			RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, MAX_TX_PROCESS);
		/*RTUSBKickBulkOut(pAd);*/
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_OUT<===\n"));
#endif

	return NDIS_STATUS_SUCCESS;
}


/* All transfers must be aborted or cancelled before attempting to reset the pipe.*/
static NTSTATUS ResetBulkInHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
#ifndef MT_MAC
	UINT32 MACValue;
	NTSTATUS ntStatus;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN === >\n"));

#ifdef CONFIG_STA_SUPPORT
	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
		return NDIS_STATUS_SUCCESS;
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		ATEResetBulkIn(pAd);
	else
#endif /* CONFIG_ATE */
	{
		/*while ((atomic_read(&pAd->PendingRx) > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) */
		if((pAd->PendingRx > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BulkIn IRP Pending!!!\n"));
			RTUSBCancelPendingBulkInIRP(pAd);
			RtmpusecDelay(100000);
			pAd->PendingRx = 0;
		}
	}

	/* Wait 10ms before reading register.*/
	RtmpusecDelay(10000);
	ntStatus = RTMP_IO_READ32(pAd, MAC_CSR0, &MACValue);

	/* It must be removed. Or ATE will have no RX success. */ 
	if ((NT_SUCCESS(ntStatus) == TRUE) &&
				(!(RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF |
												fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))))
	{
		UCHAR	i;

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_RADIO_OFF |
									fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)))
			return NDIS_STATUS_SUCCESS;

		pAd->NextRxBulkInPosition = pAd->RxContext[pAd->NextRxBulkInIndex].BulkInOffset;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BULK_IN_RESET: NBIIdx=0x%x,NBIRIdx=0x%x, BIRPos=0x%lx. BIReq=x%lx, BIComplete=0x%lx, BICFail0x%lx\n",
					pAd->NextRxBulkInIndex,  pAd->NextRxBulkInReadIndex, pAd->NextRxBulkInPosition, pAd->BulkInReq, pAd->BulkInComplete, pAd->BulkInCompleteFail));

		for (i = 0; i < RX_RING_SIZE; i++)
		{
 			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\tRxContext[%d]: IRPPending=%d, InUse=%d, Readable=%d!\n"
							, i, pAd->RxContext[i].IRPPending, pAd->RxContext[i].InUse, pAd->RxContext[i].Readable));
		}

		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);

		for (i = 0; i < pAd->CommonCfg.NumOfBulkInIRP; i++)
		{
			/*RTUSBBulkReceive(pAd);*/
			PRX_CONTEXT		pRxContext;
			PURB			pUrb;
			int				ret = 0;
			ULONG	IrqFlags = 0;

			RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
			pRxContext = &(pAd->RxContext[pAd->NextRxBulkInIndex]);

			if ((pAd->PendingRx > 0) || (pRxContext->Readable == TRUE) || (pRxContext->InUse == TRUE))
			{
				RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
				return NDIS_STATUS_SUCCESS;
			}

			pRxContext->InUse = TRUE;
			pRxContext->IRPPending = TRUE;
			pAd->PendingRx++;
			pAd->BulkInReq++;
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

			/* Init Rx context descriptor*/
			RTUSBInitRxDesc(pAd, pRxContext);
			pUrb = pRxContext->pUrb;
			if ((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
			{	/* fail*/
				RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
				pRxContext->InUse = FALSE;
				pRxContext->IRPPending = FALSE;
				pAd->PendingRx--;
				pAd->BulkInReq--;
				RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMDTHREAD_RESET_BULK_IN: Submit Rx URB failed(%d), status=%d\n", ret, RTMP_USB_URB_STATUS_GET(pUrb)));
			}
			else
			{	/* success*/
				/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BIDone, Pend=%d,BIIdx=%d,BIRIdx=%d!\n", */
				/*							pAd->PendingRx, pAd->NextRxBulkInIndex, pAd->NextRxBulkInReadIndex));*/
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CMDTHREAD_RESET_BULK_IN: Submit Rx URB Done, status=%d!\n", RTMP_USB_URB_STATUS_GET(pUrb)));
				ASSERT((pRxContext->InUse == pRxContext->IRPPending));
			}
		}

	}
	else
	{
		/* Card must be removed*/
		if (NT_SUCCESS(ntStatus) != TRUE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMDTHREAD_RESET_BULK_IN: Read Register Failed!Card must be removed!!\n\n"));
		}
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMDTHREAD_RESET_BULK_IN: Cannot do bulk in because flags(0x%lx) on !\n", pAd->Flags));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread : CMDTHREAD_RESET_BULK_IN <===\n"));
#endif

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetAsicWcidHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	RT_SET_ASIC_WCID	SetAsicWcid;
	
	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));

	if (SetAsicWcid.WCID >= MAX_LEN_OF_MAC_TABLE)
		return NDIS_STATUS_FAILURE;

#ifndef MT_MAC
	if (pAd->chipCap.hif_type != HIF_MT) 
	{
		USHORT		offset;
		UINT32		MACValue, MACRValue = 0;
		
		offset = MAC_WCID_BASE + ((UCHAR)SetAsicWcid.WCID)*HW_WCID_ENTRY_SIZE;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread : CMDTHREAD_SET_ASIC_WCID : WCID = %ld, SetTid  = %lx, DeleteTid = %lx.\n",
							SetAsicWcid.WCID, SetAsicWcid.SetTid, SetAsicWcid.DeleteTid));

		MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[3]<<24)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[2]<<16)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[1]<<8)+(pAd->MacTab.Content[SetAsicWcid.WCID].Addr[0]);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("1-MACValue= %x,\n", MACValue));
		RTMP_IO_WRITE32(pAd, offset, MACValue);
		/* Read bitmask*/
		RTMP_IO_READ32(pAd, offset+4, &MACRValue);
		if ( SetAsicWcid.DeleteTid != 0xffffffff)
			MACRValue &= (~SetAsicWcid.DeleteTid);
		if (SetAsicWcid.SetTid != 0xffffffff)
			MACRValue |= (SetAsicWcid.SetTid);

		MACRValue &= 0xffff0000;
		MACValue = (pAd->MacTab.Content[SetAsicWcid.WCID].Addr[5]<<8)+pAd->MacTab.Content[SetAsicWcid.WCID].Addr[4];
		MACValue |= MACRValue;
		RTMP_IO_WRITE32(pAd, offset+4, MACValue);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("2-MACValue= %x,\n", MACValue));
	} 
	else
#endif /* MT_MAC */		
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread : CMDTHREAD_SET_ASIC_WCID : WCID = %ld, Tid  = %x, SN = %d, BaSize = %d, IsAdd = %d, Ses_type = %d.\n",
						SetAsicWcid.WCID, SetAsicWcid.Tid, SetAsicWcid.SN, SetAsicWcid.Basize, SetAsicWcid.IsAdd, SetAsicWcid.Ses_type));
						
		AsicUpdateBASession(pAd, SetAsicWcid.WCID, SetAsicWcid.Tid, SetAsicWcid.SN, SetAsicWcid.Basize, SetAsicWcid.IsAdd, SetAsicWcid.Ses_type);
	}

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS DelAsicWcidHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	RT_SET_ASIC_WCID SetAsicWcid;
	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));
        
	if (SetAsicWcid.WCID >= MAX_LEN_OF_MAC_TABLE)
		return NDIS_STATUS_FAILURE;
        
        AsicDelWcidTab(pAd, (UCHAR)SetAsicWcid.WCID);

        return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SetWcidSecInfoHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_SEC_INFO pInfo;

	pInfo = (PRT_ASIC_WCID_SEC_INFO)CMDQelmt->buffer;
	RTMPSetWcidSecurityInfo(pAd,
							 pInfo->BssIdx,
							 pInfo->KeyIdx,
							 pInfo->CipherAlg,
							 pInfo->Wcid,
							 pInfo->KeyTabFlag);

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetAsicWcidIVEIVHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_IVEIV_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_IVEIV_ENTRY)CMDQelmt->buffer;
	AsicUpdateWCIDIVEIV(pAd,
						  pInfo->Wcid,
						  pInfo->Iv,
						  pInfo->Eiv);

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetAsicWcidAttrHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_WCID_ATTR_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_ATTR_ENTRY)CMDQelmt->buffer;
	AsicUpdateWcidAttributeEntry(pAd,
								  pInfo->BssIdx,
								  pInfo->KeyIdx,
								  pInfo->CipherAlg,
								  pInfo->Wcid,
								  pInfo->KeyTabFlag);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SETAsicSharedKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_SHARED_KEY pInfo;

	pInfo = (PRT_ASIC_SHARED_KEY)CMDQelmt->buffer;
	AsicAddSharedKeyEntry(pAd,
						       pInfo->BssIndex,
							pInfo->KeyIdx,
							&pInfo->CipherKey);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SetAsicPairwiseKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_PAIRWISE_KEY pInfo;

	pInfo = (PRT_ASIC_PAIRWISE_KEY)CMDQelmt->buffer;
	AsicAddPairwiseKeyEntry(pAd,
							 pInfo->WCID,
							 &pInfo->CipherKey);

	return NDIS_STATUS_SUCCESS;
}

#ifdef CONFIG_STA_SUPPORT
static NTSTATUS SetPortSecuredHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	STA_PORT_SECURED(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT */


static NTSTATUS RemovePairwiseKeyHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR Wcid = *((PUCHAR)(CMDQelmt->buffer));

	AsicRemovePairwiseKeyEntry(pAd, Wcid);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS SetClientMACEntryHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_SET_ASIC_WCID pInfo;

	pInfo = (PRT_SET_ASIC_WCID)CMDQelmt->buffer;
	AsicUpdateRxWCIDTable(pAd, pInfo->WCID, pInfo->Addr);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS UpdateProtectHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_ASIC_PROTECT_INFO pAsicProtectInfo;

	pAsicProtectInfo = (PRT_ASIC_PROTECT_INFO)CMDQelmt->buffer;
	AsicUpdateProtect(pAd, pAsicProtectInfo->OperationMode, pAsicProtectInfo->SetMask,
							pAsicProtectInfo->bDisableBGProtect, pAsicProtectInfo->bNonGFExist);
	
	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS APUpdateCapabilityAndErpieHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	APUpdateCapabilityAndErpIe(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS _802_11_CounterMeasureHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MAC_TABLE_ENTRY *pEntry;

		pEntry = (MAC_TABLE_ENTRY *)CMDQelmt->buffer;
		HandleCounterMeasure(pAd, pEntry);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
static NTSTATUS SetPSMBitHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		USHORT *pPsm = (USHORT *)CMDQelmt->buffer;
		MlmeSetPsmBit(pAd, *pPsm);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS ForceWakeUpHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		AsicForceWakeup(pAd, TRUE);

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS ForceSleepAutoWakeupHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	USHORT  TbttNumToNextWakeUp;
	USHORT  NextDtim = pAd->StaCfg.DtimPeriod;
	ULONG   Now;

	NdisGetSystemUpTime(&Now);
	NextDtim -= (USHORT)(Now - pAd->StaCfg.LastBeaconRxTime)/pAd->CommonCfg.BeaconPeriod;

	TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
		TbttNumToNextWakeUp = NextDtim;

	RTMP_SET_PSM_BIT(pAd, PWR_SAVE);

	/* if WMM-APSD is failed, try to disable following line*/
	AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);

	return NDIS_STATUS_SUCCESS;
}


NTSTATUS QkeriodicExecutHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	StaQuickResponeForRateUpExec(NULL, pAd, NULL, NULL);
	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_STA_SUPPORT*/


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS APEnableTXBurstHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_ENABLE_TX_BURST  \n"));

		AsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x20);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APDisableTXBurstHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_DISABLE_TX_BURST  \n"));

		AsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x00);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APAdjustEXPAckTimeHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME  \n"));
		RTMP_IO_WRITE32(pAd, EXP_ACK_TIME, 0x005400ca);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS APRecoverEXPAckTimeHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME  \n"));
		RTMP_IO_WRITE32(pAd, EXP_ACK_TIME, 0x002400ca);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef LED_CONTROL_SUPPORT
static NTSTATUS SetLEDStatusHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UCHAR LEDStatus = *((PUCHAR)(CMDQelmt->buffer));

	RTMPSetLEDStatus(pAd, LEDStatus);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: CMDTHREAD_SET_LED_STATUS (LEDStatus = %d)\n",
								__FUNCTION__, LEDStatus));

	return NDIS_STATUS_SUCCESS;
}
#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/*WPS LED MODE 10*/
static NTSTATUS LEDWPSMode10Hdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UINT WPSLedMode10 = *((PUINT)(CMDQelmt->buffer));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10::ON or Flash or OFF : %x\n", WPSLedMode10));

	switch(WPSLedMode10)
	{
		case LINK_STATUS_WPS_MODE10_TURN_ON:
			RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_ON);
			break;
		case LINK_STATUS_WPS_MODE10_FLASH:
			RTMPSetLEDStatus(pAd,LED_WPS_MODE10_FLASH);
			break;
		case LINK_STATUS_WPS_MODE10_TURN_OFF:
			RTMPSetLEDStatus(pAd, LED_WPS_MODE10_TURN_OFF);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10:: No this status %d!!!\n", WPSLedMode10));
			break;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS ChannelRescanHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cmd> Re-scan channel! \n"));

	pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, TRUE);
#ifdef DOT11_N_SUPPORT
	/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
	N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("cmd> Switch to %d! \n", pAd->CommonCfg.Channel));
	APStop(pAd);
	APStartUp(pAd);

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadAlarmResume(pAd);
#endif /* AP_QLOAD_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT*/


#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
static NTSTATUS RegHintHdlr (RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RegHint11DHdlr(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CRDA_REG_HINT11D(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RT_Mac80211_ScanEnd(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_SCAN_END(pAd, FALSE);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS RT_Mac80211_ConnResultInfom(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	RT_CFG80211_CONN_RESULT_INFORM(pAd, pAd->MlmeAux.Bssid,
								pAd->StaCfg.ReqVarIEs, pAd->StaCfg.ReqVarIELen,
								CMDQelmt->buffer, CMDQelmt->bufferlength,
								TRUE);
	return NDIS_STATUS_SUCCESS;
}
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */



#ifdef STREAM_MODE_SUPPORT
static NTSTATUS UpdateTXChainAddress(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	AsicUpdateTxChainAddress(pAd, CMDQelmt->buffer);
	return NDIS_STATUS_SUCCESS;
}
#endif /* STREAM_MODE_SUPPORT */


#ifdef MT_MAC
static NTSTATUS AddRemoveKeyHdlr(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	PMT_ASIC_SEC_INFO pInfo;

	pInfo = (PMT_ASIC_SEC_INFO)CMDQelmt->buffer;
							
	CmdProcAddRemoveKey(pAd, pInfo->AddRemove, pInfo->BssIdx, pInfo->KeyIdx, pInfo->Wcid, pInfo->KeyTabFlag, &pInfo->CipherKey, pInfo->Addr);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS SetTREntryHdlr(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt)
{
	PRT_SET_TR_ENTRY pInfo;
	MAC_TABLE_ENTRY *pEntry;
	pInfo = (PRT_SET_TR_ENTRY)CMDQelmt->buffer;
	pEntry = (MAC_TABLE_ENTRY *)pInfo->pEntry;

	tr_tb_set_entry(pAd, pInfo->WCID, pEntry);

        return NDIS_STATUS_SUCCESS;
}
#endif /* MT_MAC */
#ifdef CFG_TDLS_SUPPORT

static NTSTATUS CFGTdlsSendCHSWSetupHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS CFGTdlsAutoTeardownHdlr(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)(CMDQelmt->buffer);
	cfg_tdls_auto_teardown(pAd, pEntry);
	return NDIS_STATUS_SUCCESS;
}

#endif /* CFG_TDLS_SUPPORT */

typedef NTSTATUS (*CMDHdlr)(RTMP_ADAPTER *pAd, IN PCmdQElmt CMDQelmt);

static CMDHdlr CMDHdlrTable[] = {
	ResetBulkOutHdlr,				/* CMDTHREAD_RESET_BULK_OUT*/
	ResetBulkInHdlr,					/* CMDTHREAD_RESET_BULK_IN*/
	CheckGPIOHdlr,					/* CMDTHREAD_CHECK_GPIO	*/
	SetAsicWcidHdlr,					/* CMDTHREAD_SET_ASIC_WCID*/
	DelAsicWcidHdlr,					/* CMDTHREAD_DEL_ASIC_WCID*/
	SetClientMACEntryHdlr,			/* CMDTHREAD_SET_CLIENT_MAC_ENTRY*/

#ifdef CONFIG_STA_SUPPORT
	SetPSMBitHdlr,					/* CMDTHREAD_SET_PSM_BIT*/
	ForceWakeUpHdlr,				/* CMDTHREAD_FORCE_WAKE_UP*/
	ForceSleepAutoWakeupHdlr,		/* CMDTHREAD_FORCE_SLEEP_AUTO_WAKEUP*/
	QkeriodicExecutHdlr,				/* CMDTHREAD_QKERIODIC_EXECUT*/
#else
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	APUpdateCapabilityAndErpieHdlr,	/* CMDTHREAD_AP_UPDATE_CAPABILITY_AND_ERPIE*/
	APEnableTXBurstHdlr,			/* CMDTHREAD_AP_ENABLE_TX_BURST*/
	APDisableTXBurstHdlr,			/* CMDTHREAD_AP_DISABLE_TX_BURST*/
	APAdjustEXPAckTimeHdlr,		/* CMDTHREAD_AP_ADJUST_EXP_ACK_TIME*/
	APRecoverEXPAckTimeHdlr,		/* CMDTHREAD_AP_RECOVER_EXP_ACK_TIME*/
	ChannelRescanHdlr,				/* CMDTHREAD_CHAN_RESCAN*/
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* CONFIG_AP_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	SetLEDStatusHdlr,			/* CMDTHREAD_SET_LED_STATUS*/
#else
    NULL,
#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	LEDWPSMode10Hdlr,				/* CMDTHREAD_LED_WPS_MODE10*/
#else
	NULL,
#endif /* WSC_LED_SUPPORT */

#else
	NULL,
#endif /* WSC_INCLUDED */

	/* Security related */
	SetWcidSecInfoHdlr,				/* CMDTHREAD_SET_WCID_SEC_INFO*/
	SetAsicWcidIVEIVHdlr,			/* CMDTHREAD_SET_ASIC_WCID_IVEIV*/
	SetAsicWcidAttrHdlr,				/* CMDTHREAD_SET_ASIC_WCID_ATTR*/
	SETAsicSharedKeyHdlr,			/* CMDTHREAD_SET_ASIC_SHARED_KEY*/
	SetAsicPairwiseKeyHdlr,			/* CMDTHREAD_SET_ASIC_PAIRWISE_KEY*/
	RemovePairwiseKeyHdlr,			/* CMDTHREAD_REMOVE_PAIRWISE_KEY*/

#ifdef CONFIG_STA_SUPPORT
	SetPortSecuredHdlr,				/* CMDTHREAD_SET_PORT_SECURED*/
#else
	NULL,
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	_802_11_CounterMeasureHdlr,	/* CMDTHREAD_802_11_COUNTER_MEASURE*/
#else
	NULL,
#endif /* CONFIG_AP_SUPPORT */

	UpdateProtectHdlr,				/* CMDTHREAD_UPDATE_PROTECT*/


#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RegHintHdlr,
	RegHint11DHdlr,
	RT_Mac80211_ScanEnd,
	RT_Mac80211_ConnResultInfom,
#else
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* RT_CFG80211_SUPPORT */

#else
	NULL,
	NULL,
	NULL,
	NULL,
#endif /* LINUX */

	NULL,

	NULL,

#ifdef STREAM_MODE_SUPPORT
	UpdateTXChainAddress, /* CMDTHREAD_UPDATE_TX_CHAIN_ADDRESS */
#else
	NULL,
#endif

	NULL,
#ifdef MT_MAC
	AddRemoveKeyHdlr, /* CMDTHREAD_ADDREMOVE_ASIC_KEY */
	NULL,   /* CMDTHREAD_PS_CLEAR */
	NULL,   /* CMDTHREAD_PS_RETRIEVE_START */
	SetTREntryHdlr, /*CMDTHREAD_SET_TR_ENTRY*/
#else
	NULL,
	NULL,   /* CMDTHREAD_PS_CLEAR */
	NULL,   /* CMDTHREAD_PS_RETRIEVE_START */
	NULL,
#endif /* MT_MAC */
#ifdef CFG_TDLS_SUPPORT
	CFGTdlsSendCHSWSetupHdlr, /* CMDTHREAD_TDLS_SEND_CH_SW_SETUP */
	CFGTdlsAutoTeardownHdlr,  /* CMDTHREAD_TDLS_AUTO_TEARDOWN */
#else
	NULL,
	NULL,
#endif /* CFG_TDLS_SUPPORT */
};


static inline BOOLEAN ValidCMD(IN PCmdQElmt CMDQelmt)
{
	SHORT CMDIndex = CMDQelmt->command - CMDTHREAD_FIRST_CMD_ID;
	USHORT CMDHdlrTableLength= sizeof(CMDHdlrTable) / sizeof(CMDHdlr);

	if ( (CMDIndex >= 0) && (CMDIndex < CMDHdlrTableLength))
	{
		if (CMDHdlrTable[CMDIndex] > 0)
			return TRUE;
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("No corresponding CMDHdlr for this CMD(%x)\n",  CMDQelmt->command));
			return FALSE;
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMD(%x) is out of boundary\n", CMDQelmt->command));
		return FALSE;
	}
}


VOID CMDHandler(RTMP_ADAPTER *pAd)
{
	PCmdQElmt		cmdqelmt;
	NDIS_STATUS		NdisStatus = NDIS_STATUS_SUCCESS;
	NTSTATUS		ntStatus;
/*	ULONG	IrqFlags = 0;*/

	while (pAd && pAd->CmdQ.size > 0)
	{
		NdisStatus = NDIS_STATUS_SUCCESS;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTThreadDequeueCmd(&pAd->CmdQ, &cmdqelmt);
		NdisReleaseSpinLock(&pAd->CmdQLock);

		if (cmdqelmt == NULL)
			break;


		if(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		{
			if(ValidCMD(cmdqelmt)) 
				ntStatus = (*CMDHdlrTable[cmdqelmt->command - CMDTHREAD_FIRST_CMD_ID])(pAd, cmdqelmt);
		}

		if (cmdqelmt->CmdFromNdis == TRUE)
		{
			if (cmdqelmt->buffer != NULL)
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
		else
		{
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
	}	/* end of while */
}

