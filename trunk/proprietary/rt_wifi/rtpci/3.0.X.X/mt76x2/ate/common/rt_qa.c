/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 
 	Module Name:
	rt_qa.c

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/
#include "rt_config.h"
 
#ifdef RALINK_QA
NDIS_STATUS TXSTOP(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0, atemode=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PTXD_STRUC		pTxD = NULL;
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
#endif /* RTMP_MAC_PCI */
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));
	
	atemode = pATEInfo->Mode;
	pATEInfo->Mode &= ATE_TXSTOP;
	pATEInfo->bQATxStart = FALSE;

	if (atemode == ATE_TXCARR)
	{
		if (pATEInfo->TxMethod == TX_METHOD_0)
		{
#ifdef RTMP_BBP
			if (pAd->chipCap.hif_type == HIF_RTMP)
			{
			/* No Carrier Test set BBP R22 bit7=0, bit6=0, bit[5~0]=0x0 */
			ATE_BBP_RESET_TX_MODE(pAd, BBP_R22, &BbpData);
		}
#endif /* RTMP_BBP */
		}
	}
	else if (atemode == ATE_TXCARRSUPP)
	{
		if (pATEInfo->TxMethod == TX_METHOD_0)
		{
#ifdef RTMP_BBP
			/* No Cont. TX set BBP R22 bit7=0 */
			/* QA will do this in new TXCARRSUPP proposal */
			ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);

			/* No Carrier Suppression set BBP R24 bit0=0 */
			ATE_BBP_CTS_TX_SIN_WAVE_DISABLE(pAd, BBP_R24, &BbpData);
#endif /* RTMP_BBP */
		}
	}		

	/*
		We should free some resource which was allocated
		when ATE_TXFRAME, ATE_STOP, and ATE_TXCONT.
	*/
	else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
	{
		if (atemode == ATE_TXCONT)
		{
			if (pATEInfo->TxMethod == TX_METHOD_0)
			{
#ifdef RTMP_BBP
				/* No Cont. TX set BBP R22 bit7=0 */
				/* QA will do this in new TXCONT proposal */
				ATE_BBP_STOP_CTS_TX_MODE(pAd, BBP_R22, &BbpData);
#endif /* RTMP_BBP */
			}
		}

#ifdef RTMP_MAC_PCI
		/* Abort Tx, Rx DMA. */
		RtmpDmaEnable(pAd, 0);

		for (ring_index=0; ring_index<TX_RING_SIZE; ring_index++)
		{
			PNDIS_PACKET  pPacket;

#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[ring_index].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[ring_index].AllocVa;
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* !RT_BIG_ENDIAN */
			pTxD->DMADONE = 0;
			pPacket = pTxRing->Cell[ring_index].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[ring_index].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNextNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
		/* Enable Tx, Rx DMA */
		RtmpDmaEnable(pAd, 1);
#endif /* RTMP_MAC_PCI */

	}

	/* task Tx status : 0 --> task is idle, 1 --> task is running */
	pATEInfo->TxStatus = 0;

	if (pATEInfo->TxMethod == TX_METHOD_0)
	{
		BbpSoftReset(pAd);/* Soft reset BBP. */
	}

	/* Disable Tx */
	ATE_MAC_TX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


NDIS_STATUS RXSTOP(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	/* Disable Rx */
	ATE_MAC_RX_DISABLE(pAd, MAC_SYS_CTRL, &MacData);

	pATEInfo->Mode &= ATE_RXSTOP;
	pATEInfo->bQARxStart = FALSE;


	if ((!IS_RT3883(pAd)) && (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)))
		BbpSoftReset(pAd);/* Soft reset BBP. */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static VOID memcpy_exl(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	UINT32 i, Value = 0;
	UCHAR *pDst = NULL, *pSrc = NULL;
	
	for (i = 0 ; i < (len >> 2); i++)
	{
		pDst = (dst + i*4);
		pSrc = (src + i*4);
		/* For alignment issue, we need a variable "Value". */
		memmove(&Value, pSrc, 4);
		Value = OS_HTONL(Value); 
		memmove(pDst, &Value, 4);		
		pDst += 4;
		pSrc += 4;
	}

	if ((len % 4) != 0)
	{
		/* wish that it will never reach here */
		memmove(&Value, pSrc, (len % 4));
		Value = OS_HTONL(Value); 
		memmove(pDst, &Value, (len % 4));
	}
}


static VOID memcpy_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i;
	{
		USHORT *pDst, *pSrc;
		
		pDst = (USHORT *) dst;
		pSrc = (USHORT *) src;

		for (i =0; i < (len >> 1); i++)
		{
			*pDst = OS_NTOHS(*pSrc);
			pDst++;
			pSrc++;
		}
		
		if ((len % 2) != 0)
		{
			memcpy(pDst, pSrc, (len % 2));
			*pDst = OS_NTOHS(*pDst);
		}
	}
	return;
}


static VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *dst, UINT32 offset, UINT32 len)
{
	UINT32 index, Value = 0;
	UCHAR *pDst;

	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	for (index = 0 ; index < (len >> 2); index++)
	{
		pDst = (dst + (index << 2));
		RTMP_IO_READ32(pAd, offset, &Value);
		DBGPRINT(RT_DEBUG_WARN,("mac r 0x%04X=0x%08X\n", offset, Value));
		
		Value = OS_HTONL(Value);
		memmove(pDst, &Value, 4);
		offset += 4;
	}
	DBGPRINT(RT_DEBUG_WARN,("\n\n"));

	return;
}


VOID BubbleSort(INT32 size, INT32 array[])
{ 
	INT32 outer, inner, temp;

	for (outer = size-1;  outer>0;  outer--)
	{
		for (inner = 0; inner<outer; inner++)
		{
			if (array[inner] > array[inner+1])
			{
				temp = array[inner]; 
				array[inner]=array[inner+1]; 
				array[inner+1]=temp;
			}
		}
	}
	return;
} 

#ifdef RTMP_BBP
VOID CalNoiseLevel(PRTMP_ADAPTER pAd, UCHAR channel, INT32 RSSI[3][10])
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT32		RSSI0, RSSI1, RSSI2;
 	CHAR		Rssi0Offset, Rssi1Offset, Rssi2Offset;
	UCHAR		BbpR50Rssi0 = 0, BbpR51Rssi1 = 0, BbpR52Rssi2 = 0;
	UCHAR		Org_BBP66value = 0, Org_BBP69value = 0, Org_BBP70value = 0, data = 0;
	USHORT		LNA_Gain = 0;
	INT32		column = 0;
	UCHAR		Org_Channel = pATEInfo->Channel;
	USHORT	    GainValue = 0, OffsetValue = 0;

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &Org_BBP66value);
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R69, &Org_BBP69value);	
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R70, &Org_BBP70value);

	/************************************************************************/
	/* Read the value of LNA gain and RSSI offset */
	/************************************************************************/
	RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, GainValue);

	/* for Noise Level */
	if (channel <= 14)
	{
		LNA_Gain = GainValue & 0x00FF;		 

		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, OffsetValue);
		Rssi0Offset = OffsetValue & 0x00FF;
		Rssi1Offset = (OffsetValue & 0xFF00) >> 8;

		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_BG_OFFSET + 2)/* 0x48 */, OffsetValue);
		Rssi2Offset = OffsetValue & 0x00FF;
	}
	else
	{
		LNA_Gain = (GainValue & 0xFF00) >> 8;

		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, OffsetValue);
		Rssi0Offset = OffsetValue & 0x00FF;
		Rssi1Offset = (OffsetValue & 0xFF00) >> 8;

		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET + 2)/* 0x4C */, OffsetValue);
		Rssi2Offset = OffsetValue & 0x00FF;
	}
	/***********************************************************************/	
	{
		pATEInfo->Channel = channel;
		ATEAsicSwitchChannel(pAd);
		RtmpOsMsDelay(5);

		data = 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);	
		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, data);
		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, data);
		RtmpOsMsDelay(5);

		/* start Rx */
		pATEInfo->bQARxStart = TRUE;
		Set_ATE_Proc(pAd, "RXFRAME");

		RtmpOsMsDelay(5);

		for (column = 0; column < 10; column++)
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R50, &BbpR50Rssi0);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R51, &BbpR51Rssi1);	
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R52, &BbpR52Rssi2);

			RtmpOsMsDelay(10);

			/* calculate RSSI 0 */
			if (BbpR50Rssi0 == 0)
			{
				RSSI0 = -100;
			}
			else
			{
				RSSI0 = (INT32)(-12 - BbpR50Rssi0 - LNA_Gain - Rssi0Offset);
			}
			RSSI[0][column] = RSSI0;

			if ( pAd->Antenna.field.RxPath >= 2 ) /* 2R */
			{
				/* calculate RSSI 1 */
				if (BbpR51Rssi1 == 0)
				{
					RSSI1 = -100;
				}
				else
				{
					RSSI1 = (INT32)(-12 - BbpR51Rssi1 - LNA_Gain - Rssi1Offset);
				}
				RSSI[1][column] = RSSI1;
			}

			if ( pAd->Antenna.field.RxPath >= 3 ) /* 3R */
			{
				/* calculate RSSI 2 */
				if (BbpR52Rssi2 == 0)
					RSSI2 = -100;
				else
					RSSI2 = (INT32)(-12 - BbpR52Rssi2 - LNA_Gain - Rssi2Offset);

				RSSI[2][column] = RSSI2;
			}
		}

		/* stop Rx */
		Set_ATE_Proc(pAd, "RXSTOP");

		RtmpOsMsDelay(5);

		BubbleSort(10, RSSI[0]); /* 1R */		

		if ( pAd->Antenna.field.RxPath >= 2 ) /* 2R */
		{
			BubbleSort(10, RSSI[1]);
		}

		if ( pAd->Antenna.field.RxPath >= 3 ) /* 3R */
		{
			BubbleSort(10, RSSI[2]);
		}	
	}

	pATEInfo->Channel = Org_Channel;
	ATEAsicSwitchChannel(pAd);

	/* restore original value */	

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, Org_BBP69value);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, Org_BBP70value);

	return;
}
#endif /* RTMP_BBP */


static VOID ATE_BBPRead(
		IN	PRTMP_ADAPTER	pAd,
		IN	UCHAR			offset,
		IN  UCHAR			*pValue)
{
#ifdef RTMP_BBP
	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, offset, pValue);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, offset, pValue);
	}
#endif /* RTMP_BBP */
}


static VOID ATE_BBPWrite(
		IN	PRTMP_ADAPTER	pAd,
		IN	UCHAR			offset,
		IN  UCHAR			value)
{
#ifdef RTMP_BBP
	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, offset, value);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, offset, value);
	}
#endif /* RTMP_BBP */
}


BOOLEAN SyncTxRxConfig(PRTMP_ADAPTER pAd, USHORT offset, UCHAR value)
{ 
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR tmp = 0, bbp_data = 0;

	ATE_BBPRead(pAd, offset, &bbp_data);

	/* confirm again */
	ASSERT(bbp_data == value);

	switch (offset)
	{
		case BBP_R1:
			/* Need to synchronize tx configuration with legacy ATE. */
			tmp = (bbp_data & ((1 << 4) | (1 << 3))/* 0x18 */) >> 3;
		    switch (tmp)
		    {
				/* The BBP R1 bit[4:3] = 2 :: Both DACs will be used by QA. */
		        case 2:
					/* All */
					pATEInfo->TxAntennaSel = 0;
		            break;
				/* The BBP R1 bit[4:3] = 0 :: DAC 0 will be used by QA. */
		        case 0:
					/* Antenna one */
					pATEInfo->TxAntennaSel = 1;
		            break;
				/* The BBP R1 bit[4:3] = 1 :: DAC 1 will be used by QA. */
		        case 1:
					/* Antenna two */
					pATEInfo->TxAntennaSel = 2;
		            break;
		        default:
		            DBGPRINT_ERR(("%s -- Sth. wrong!  : return FALSE; \n", __FUNCTION__));    
		            return FALSE;
		    }
			break;/* case BBP_R1 */

		case BBP_R3:
			/* Need to synchronize rx configuration with legacy ATE. */
			tmp = (bbp_data & ((1 << 1) | (1 << 0))/* 0x03 */);
		    switch(tmp)
		    {
				/* The BBP R3 bit[1:0] = 3 :: All ADCs will be used by QA. */
		        case 3:
					/* All */
					pATEInfo->RxAntennaSel = 0;
		            break;
				/*
					The BBP R3 bit[1:0] = 0 :: ADC 0 will be used by QA,
					unless the BBP R3 bit[4:3] = 2
				*/
		        case 0:
					/* Antenna one */
					pATEInfo->RxAntennaSel = 1;
					tmp = ((bbp_data & ((1 << 4) | (1 << 3))/* 0x03 */) >> 3);
					if (tmp == 2) /* 3R */
					{
						/* Default : All ADCs will be used by QA */
						pATEInfo->RxAntennaSel = 0;
					}
		            break;
				/* The BBP R3 bit[1:0] = 1 :: ADC 1 will be used by QA. */
		        case 1:
					/* Antenna two */
					pATEInfo->RxAntennaSel = 2;
		            break;
				/* The BBP R3 bit[1:0] = 2 :: ADC 2 will be used by QA. */
		        case 2:
					/* Antenna three */
					pATEInfo->RxAntennaSel = 3;
		            break;
		        default:
		            DBGPRINT_ERR(("%s -- Impossible!  : return FALSE; \n", __FUNCTION__));    
		            return FALSE;
		    }
			break;/* case BBP_R3 */

        default:
            DBGPRINT_ERR(("%s -- Sth. wrong!  : return FALSE; \n", __FUNCTION__));    
            return FALSE;
		
	}
	return TRUE;
}


static INT ResponseToGUI(
	IN  struct ate_racfghdr *pRaCfg,
	IN	RTMP_IOCTL_INPUT_STRUCT	*pwrq,
	IN  INT Length,
	IN  INT Status)
{
	(pRaCfg)->length = OS_HTONS((Length));
	(pRaCfg)->status = OS_HTONS((Status));
	(pwrq)->u.data.length = sizeof((pRaCfg)->magic_no) + sizeof((pRaCfg)->command_type)
							+ sizeof((pRaCfg)->command_id) + sizeof((pRaCfg)->length)
							+ sizeof((pRaCfg)->sequence) + OS_NTOHS((pRaCfg)->length);
	DBGPRINT(RT_DEBUG_TRACE, ("wrq->u.data.length = %d\n", (pwrq)->u.data.length));

	if (copy_to_user((pwrq)->u.data.pointer, (UCHAR *)(pRaCfg), (pwrq)->u.data.length))
	{
		
		DBGPRINT_ERR(("copy_to_user() fail in %s\n", __FUNCTION__));
		return (-EFAULT);
	}
	else
	{
	}

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START\n"));

	pATEInfo->bQAEnabled = TRUE;
	DBGPRINT(RT_DEBUG_TRACE,("pATEInfo->bQAEnabled = %s\n", (pATEInfo->bQAEnabled)? "TRUE":"FALSE"));
	
	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
	
#ifdef	CONFIG_RT2880_ATE_CMD_NEW
	Set_ATE_Proc(pAd, "ATESTART");
#else
	Set_ATE_Proc(pAd, "APSTOP");
#endif /* CONFIG_RT2880_ATE_CMD_NEW */

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_STOP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT32 ret;

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_STOP\n"));

	pATEInfo->bQAEnabled = FALSE;
	DBGPRINT(RT_DEBUG_TRACE,("pATEInfo->bQAEnabled = %s\n", (pATEInfo->bQAEnabled)? "TRUE":"FALSE"));

	/*
		Distinguish this command came from QA(via ate agent)
		or ate agent according to the existence of pid in payload.

		No need to prepare feedback if this cmd came directly from ate agent,
		not from QA.
	*/
	pRaCfg->length = OS_NTOHS(pRaCfg->length);

	if (pRaCfg->length == sizeof(pATEInfo->AtePid))
	{
		/*
			This command came from QA.
			Get the pid of ATE agent.
		*/
		memcpy((UCHAR *)&pATEInfo->AtePid,
						(&pRaCfg->data[0]) - 2/* == sizeof(pRaCfg->status) */,
						sizeof(pATEInfo->AtePid));					

		/* Prepare feedback as soon as we can to avoid QA timeout. */
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
		/* Kill ATE agent when leaving ATE mode. */
#ifdef LINUX
		ret = RTMP_THREAD_PID_KILL(pATEInfo->AtePid);
		if (ret)
			DBGPRINT_ERR(("%s : unable to kill ate thread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
#endif /* LINUX */
	}
		

	/* AP/STA may be already in ATE_STOP mode due to cmd from QA. */
	if (ATE_ON(pAd))
	{
		/* Someone has killed ate agent while QA GUI is still open. */

#ifdef	CONFIG_RT2880_ATE_CMD_NEW
		Set_ATE_Proc(pAd, "ATESTOP");
#else
		Set_ATE_Proc(pAd, "APSTART");
#endif
		DBGPRINT(RT_DEBUG_TRACE, ("RACFG_CMD_AP_START is done !\n"));
	}
	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_RF_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{

#ifdef RT_RF
	UINT32 R1, R2, R3, R4;
	USHORT channel;
	
	memcpy(&R1, pRaCfg->data-2, 4);
	memcpy(&R2, pRaCfg->data+2, 4);
	memcpy(&R3, pRaCfg->data+6, 4);
	memcpy(&R4, pRaCfg->data+10, 4);
	memcpy(&channel, pRaCfg->data+14, 2);		
	
	pAd->LatchRfRegs.R1 = OS_NTOHL(R1);
	pAd->LatchRfRegs.R2 = OS_NTOHL(R2);
	pAd->LatchRfRegs.R3 = OS_NTOHL(R3);
	pAd->LatchRfRegs.R4 = OS_NTOHL(R4);
	pAd->LatchRfRegs.Channel = OS_NTOHS(channel);

	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R3);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);
#endif /* RT_RF */

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return  NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_READ16(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT16	offset=0, value=0;
	USHORT  tmp=0;				

	offset = OS_NTOHS(pRaCfg->status);
	RT28xx_EEPROM_READ16(pAd, offset, tmp);
	value = tmp;
	DBGPRINT(RT_DEBUG_WARN,("e2p r %03Xh=0x%02X\n"
		, offset, (value&0x00FF)));
	DBGPRINT(RT_DEBUG_WARN,("e2p r %03Xh=0x%02X\n"
		, offset+1, (value&0xFF00)>>8));
	value = OS_HTONS(value);
	
	memcpy(pRaCfg->data, &value, 2);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+2, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_WRITE16(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset, value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 2);
	value = OS_NTOHS(value);
	RT28xx_EEPROM_WRITE16(pAd, offset, value);
	DBGPRINT(RT_DEBUG_WARN,("e2p w 0x%04X=0x%04X\n", offset, value));
	DBGPRINT(RT_DEBUG_WARN,("e2p w %02Xh=0x%02X\n"
		, (offset&0x00FF), (value&0x00FF)));
	DBGPRINT(RT_DEBUG_WARN,("e2p w %02Xh=0x%02X\n"
		, (offset&0x00FF)+1, (value&0xFF00)>>8));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_READ_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE >> 1];

	rt_ee_read_all(pAd,(USHORT *)buffer);
	memcpy_exs(pAd, pRaCfg->data, (UCHAR *)buffer, EEPROM_SIZE);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+EEPROM_SIZE, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE >> 1];

	NdisZeroMemory((UCHAR *)buffer, EEPROM_SIZE);
	memcpy_exs(pAd, (UCHAR *)buffer, (UCHAR *)&pRaCfg->status, EEPROM_SIZE);
	rt_ee_write_all(pAd,(USHORT *)buffer);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_READ(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	UINT32	value;
#ifdef RLT_RF
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
	UCHAR bank, regID, rfValue;
#endif /* RLT_RF */
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);

	{
		/*
			We do not need the base address.
			So just extract the offset out.
		*/
		offset &= 0x0000FFFF;
		RTMP_IO_READ32(pAd, offset, &value);

#ifdef RT8592
		if (IS_RT8592(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE,("mac r 0x%04X=0x%08X\n", offset, value));
		}
		else
#endif /* RT8592 */
		{
			DBGPRINT(RT_DEBUG_WARN,("mac r 0x%04X=0x%08X\n", offset, value));
		}
	}
	value = OS_HTONL(value);
	memcpy(pRaCfg->data, &value, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_WRITE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset, value;
	RF_CSR_CFG_STRUC rfcsr = { { 0 } };
#ifdef RLT_RF
	UCHAR bank, regID, rfValue;
#endif /* RLT_RF */
					
	memcpy(&offset, pRaCfg->data-2, 4);
	memcpy(&value, pRaCfg->data+2, 4);

	offset = OS_NTOHL(offset);

	/*
		We do not need the base address.
		So just extract the offset out.
	*/
	offset &= 0x0000FFFF;
	value = OS_NTOHL(value);

	RTMP_IO_WRITE32(pAd, offset, value);
	
#ifdef RT8592
	if (IS_RT8592(pAd))
	{
		DBGPRINT(RT_DEBUG_WARN,("mac w 0x%04X=0x%08X\n", offset, value));
	}
	else
#endif /* RT8592 */
	{
		DBGPRINT(RT_DEBUG_WARN,("mac w 0x%04X=0x%08X\n", offset, value));
	}
			
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	USHORT	len;

	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);

	offset &= 0x0000FFFF;
	memcpy(&len, pRaCfg->data+2, 2);
	len = OS_NTOHS(len);

	if (len > 371)
	{
		DBGPRINT_ERR(("%s : length requested is too large, make it smaller\n", __FUNCTION__));
		pRaCfg->length = OS_HTONS(2);
		pRaCfg->status = OS_HTONS(1);

		return -EFAULT;
	}

	RTMP_IO_READ_BULK(pAd, pRaCfg->data, offset, (len << 2));/* unit in four bytes*/

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(len << 2), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_READ8(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;
	
	value = 0;
	offset = OS_NTOHS(pRaCfg->status);

	ATE_BBPRead(pAd, offset, &value);
	DBGPRINT(RT_DEBUG_WARN,("bbp r R%u=0x%02X\n", offset, value));
	pRaCfg->data[0] = value;
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+1, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_WRITE8(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 1);
	ATE_BBPWrite(pAd, offset, value);
	DBGPRINT(RT_DEBUG_WARN,("bbp w R%u=0x%02X\n", offset, value));

	if ((offset == BBP_R1) || (offset == BBP_R3))
	{
		SyncTxRxConfig(pAd, offset, value);
	}
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_READ_ALL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT bbp_reg_index;
	
	for (bbp_reg_index = 0; bbp_reg_index < pAd->chipCap.MaxNumOfBbpId+1; bbp_reg_index++)
	{
		pRaCfg->data[bbp_reg_index] = 0;
		ATE_BBPRead(pAd, bbp_reg_index, &pRaCfg->data[bbp_reg_index]);
	}
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+ pAd->chipCap.MaxNumOfBbpId+1, NDIS_STATUS_SUCCESS);
	
	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_GET_NOISE_LEVEL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UCHAR	channel;
	INT32   buffer[3][10];/* 3 : RxPath ; 10 : no. of per rssi samples */

	channel = (OS_NTOHS(pRaCfg->status) & 0x00FF);
#ifdef RTMP_BBP
	if (pAd->chipCap.bbp_type == BBP_RTMP) {
		CalNoiseLevel(pAd, channel, buffer);
	}
#endif /* RTMP_BBP */

#ifdef RLT_BBP
	if (pAd->chipCap.bbp_type == BBP_RLT) {
		/* TBD */
	}
#endif /* RLT_BBP */

	memcpy_exl(pAd, (UCHAR *)pRaCfg->data, (UCHAR *)&(buffer[0][0]), (sizeof(INT32)*3*10));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(sizeof(INT32)*3*10), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}




static  INT DO_RACFG_CMD_GET_COUNTER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	memcpy_exl(pAd, &pRaCfg->data[0], (UCHAR *)&pATEInfo->U2M, 4);
	memcpy_exl(pAd, &pRaCfg->data[4], (UCHAR *)&pATEInfo->OtherData, 4);
	memcpy_exl(pAd, &pRaCfg->data[8], (UCHAR *)&pATEInfo->Beacon, 4);
	memcpy_exl(pAd, &pRaCfg->data[12], (UCHAR *)&pATEInfo->OtherCount, 4);
	memcpy_exl(pAd, &pRaCfg->data[16], (UCHAR *)&pATEInfo->TxAc0, 4);
	memcpy_exl(pAd, &pRaCfg->data[20], (UCHAR *)&pATEInfo->TxAc1, 4);
	memcpy_exl(pAd, &pRaCfg->data[24], (UCHAR *)&pATEInfo->TxAc2, 4);
	memcpy_exl(pAd, &pRaCfg->data[28], (UCHAR *)&pATEInfo->TxAc3, 4);
	memcpy_exl(pAd, &pRaCfg->data[32], (UCHAR *)&pATEInfo->TxHCCA, 4);
	memcpy_exl(pAd, &pRaCfg->data[36], (UCHAR *)&pATEInfo->TxMgmt, 4);
	memcpy_exl(pAd, &pRaCfg->data[40], (UCHAR *)&pATEInfo->RSSI0, 4);
	memcpy_exl(pAd, &pRaCfg->data[44], (UCHAR *)&pATEInfo->RSSI1, 4);
	memcpy_exl(pAd, &pRaCfg->data[48], (UCHAR *)&pATEInfo->RSSI2, 4);
	memcpy_exl(pAd, &pRaCfg->data[52], (UCHAR *)&pATEInfo->SNR0, 4);
	memcpy_exl(pAd, &pRaCfg->data[56], (UCHAR *)&pATEInfo->SNR1, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+60, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}

#ifdef MT76x2
INT DO_RACFG_CMD_ATE_MT76x2_Calibration(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	USHORT length;
	UINT32 size, cal_id, param0;

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_MT76x2_Calibration\n"));				

	length = OS_NTOHS(pRaCfg->length);
	memcpy(&size, &(pRaCfg->status), 4);
	size = OS_NTOHL(size);

	if ( length >= 8 )
	{
		memcpy(&cal_id, pRaCfg->data + 2, 4);
		cal_id = OS_NTOHL(cal_id);
		DBGPRINT(RT_DEBUG_TRACE,("cal_id = %x\n", cal_id));
	}

	DBGPRINT(RT_DEBUG_TRACE,("Calibration ID = %d\n", cal_id));				

	if ( length >= 12 )
	{
		memcpy(&param0, pRaCfg->data + 6, 4);
		param0 = OS_NTOHL(param0);
		DBGPRINT(RT_DEBUG_TRACE,("param0 = %x\n", param0));
	}

	mt76x2_ate_do_calibration(pAd, cal_id, param0);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* MT76x2 */

static  INT DO_RACFG_CMD_CLEAR_COUNTER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	pATEInfo->U2M = 0;
	pATEInfo->OtherData = 0;
	pATEInfo->Beacon = 0;
	pATEInfo->OtherCount = 0;
	pATEInfo->TxAc0 = 0;
	pATEInfo->TxAc1 = 0;
	pATEInfo->TxAc2 = 0;
	pATEInfo->TxAc3 = 0;
	pATEInfo->TxHCCA = 0;
	pATEInfo->TxMgmt = 0;
	pATEInfo->TxDoneCount = 0;
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_TX_START(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	USHORT *p;
	USHORT	err = 1;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 TxInfoSize = 4;
	USHORT data_len = 0;

	if ((pATEInfo->TxStatus != 0) && (pATEInfo->Mode & ATE_TXFRAME))
	{
		DBGPRINT_ERR(("%s : Ate Tx is already running,"
			"to run next Tx, you must stop it first\n", __FUNCTION__));
		err = 2;
		goto tx_start_error;
	}
	else if ((pATEInfo->TxStatus != 0) && !(pATEInfo->Mode & ATE_TXFRAME))
	{
		int slot = 0;

		while ((slot++ < 10) && (pATEInfo->TxStatus != 0))
		{
			RtmpOsMsDelay(5);
		}

		/* force it to stop */
		pATEInfo->TxStatus = 0;
		pATEInfo->TxDoneCount = 0;
		pATEInfo->bQATxStart = FALSE;
	}

	/*
		Reset ATE mode and set Tx/Rx idle
		for new proposed TXCONT/TXCARR/TXCARRSUPP solution.
	*/
	if ((pATEInfo->Mode & ATE_TXFRAME) && (pATEInfo->TxMethod == TX_METHOD_1))
	{
		TXSTOP(pAd);
	}

	/*
		If pRaCfg->length == 0, this "RACFG_CMD_TX_START"
		is for Carrier test or Carrier Suppression test.
	*/
	if (OS_NTOHS(pRaCfg->length) != 0)
	{
		/* get frame info */

		NdisMoveMemory(&pATEInfo->TxWI, pRaCfg->data + 2, TXWISize);						
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, (PUCHAR)&pATEInfo->TxWI, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */

		NdisMoveMemory(&pATEInfo->TxCount, pRaCfg->data + TXWISize + 2, 4);
		pATEInfo->TxCount = OS_NTOHL(pATEInfo->TxCount);

/*		p = (USHORT *)(&pRaCfg->data[TXWISize + TxInfoSize + 2]); */

		/* always use QID_AC_BE */
		pATEInfo->QID = 0;

		p = (USHORT *)(&pRaCfg->data[TXWISize + TxInfoSize + 2*2]);
		pATEInfo->HLen = OS_NTOHS(*p);

		if (pATEInfo->HLen > 32)
		{
			DBGPRINT_ERR(("%s : pATEInfo->HLen > 32\n", __FUNCTION__));
			DBGPRINT_ERR(("pATEInfo->HLen = %d\n", pATEInfo->HLen));
			err = 3;
			goto tx_start_error;
		}

		NdisMoveMemory(&pATEInfo->Header, pRaCfg->data + (TXWISize + TxInfoSize + 2*3), pATEInfo->HLen);

		pATEInfo->PLen = OS_NTOHS(pRaCfg->length) - (pATEInfo->HLen + (TXWISize + TxInfoSize + 2*4));

		if (pATEInfo->PLen > 32)
		{
			DBGPRINT_ERR(("%s : pATEInfo->PLen > 32\n", __FUNCTION__));
			err = 4;
			goto tx_start_error;
		}

		NdisMoveMemory(&pATEInfo->Pattern, pRaCfg->data + (TXWISize + TxInfoSize + 2*3) + pATEInfo->HLen, pATEInfo->PLen);

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
			data_len = pATEInfo->TxWI.TXWI_N.MPDUtotalByteCnt;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
		{
			data_len = pATEInfo->TxWI.TXWI_O.MPDUtotalByteCnt;
		}
#endif /* RTMP_MAC */
		pATEInfo->DLen = data_len - pATEInfo->HLen;


	}

	ReadQATxTypeFromBBP(pAd);

	if (pATEInfo->bQATxStart == TRUE)
	{
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
		return NDIS_STATUS_SUCCESS;
	}

tx_start_error:
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), err);

	return err;
}


static  INT DO_RACFG_CMD_GET_TX_STATUS(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 count=0;
	
	count = OS_HTONL(pATEInfo->TxDoneCount);
	NdisMoveMemory(pRaCfg->data, &count, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_TX_STOP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_TX_STOP\n"));

	if (IS_RT8592(pAd) || IS_MT7610(pAd))
	{
#ifdef CONFIG_RT2880_ATE_CMD_NEW
		Set_ATE_Proc(pAd, "ATESTART");
#else
		Set_ATE_Proc(pAd, "APSTOP");
#endif /* CONFIG_RT2880_ATE_CMD_NEW */
	}
	else
	{
		Set_ATE_Proc(pAd, "TXSTOP");
	}

	pATEInfo->bQATxStart = FALSE;

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_RX_START(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	pATEInfo->bQARxStart = TRUE;
	Set_ATE_Proc(pAd, "RXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	


static  INT DO_RACFG_CMD_RX_STOP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_STOP\n"));

	Set_ATE_Proc(pAd, "RXSTOP");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_CARRIER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CARRIER\n"));

	Set_ATE_Proc(pAd, "TXCARR");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_CONT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CONT\n"));

	Set_ATE_Proc(pAd, "TXCONT");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_FRAME(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_FRAME\n"));

	Set_ATE_Proc(pAd, "TXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	


static  INT DO_RACFG_CMD_ATE_START_TX_FRAME_V2(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 TxCount;
	UINT16 TxLength;
	UINT32 mac1004, MacReg = 0;
	UINT32 MTxCycle;

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_START_TX_FRAME_V2\n"));

	memcpy(&TxCount, &pRaCfg->status, 4);
	TxCount = OS_NTOHL(TxCount);

	memcpy(&TxLength, pRaCfg->data + 2, 2 /* sizeof(offset) */);
	TxLength = OS_NTOHS(TxLength);

	if ( TxCount == 0 )
		pATEInfo->TxCount = 0xFFFFFFFF;
	else
		pATEInfo->TxCount = TxCount;
	DBGPRINT(RT_DEBUG_TRACE, ("DO_RACFG_CMD_ATE_START_TX_FRAME_V2 (TxCount = %lu)\n", pATEInfo->TxCount));

	pATEInfo->TxLength = TxLength;

	if ((pATEInfo->TxLength < 24) || (pATEInfo->TxLength > (MAX_FRAME_SIZE - 34/* == 2312 */)))
	{
		pATEInfo->TxLength = (MAX_FRAME_SIZE - 34/* == 2312 */);
		DBGPRINT_ERR(("DO_RACFG_CMD_ATE_START_TX_FRAME_V2::Out of range, it should be in range of 24~%d.\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("DO_RACFG_CMD_ATE_START_TX_FRAME_V2 (TxLength = %d)\n", pATEInfo->TxLength));

	RTMP_IO_READ32(pAd, 0x1004, &mac1004);
	RTMP_IO_WRITE32(pAd, 0x1004, 4);
	/*
		Check page count in TxQ,
	*/
	for (MTxCycle = 0; MTxCycle < 1000; MTxCycle++)
	{
		BOOLEAN bFree = TRUE;
		RTMP_IO_READ32(pAd, 0x438, &MacReg);
		if (MacReg != 0)
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa30, &MacReg);
		if (MacReg & 0x000000FF)
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa34, &MacReg);
		if (MacReg & 0xFF00FF00)
			bFree = FALSE;
		if (bFree)
			break;
		RtmpOsMsDelay(1);
	}
	RTMP_IO_WRITE32(pAd, 0x1004, mac1004);

	//pATEInfo->bQATxStart = TRUE;

	Set_ATE_Proc(pAd, "TXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	

static	INT DO_RACFG_CMD_ATE_SET_IPG(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN	struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	STRING    str[LEN_OF_ARG];
	ULONG value = 0;
	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_SET_IPG\n"));

	memcpy(&value, &pRaCfg->status, 4);
	value = OS_NTOHL(value);
	pATEInfo->IPG = value;
	DBGPRINT(RT_DEBUG_TRACE,("set pATEInfo->IPG : (%d)\n",pATEInfo->IPG));
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_IPG_Proc(pAd, str);
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_BW(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_BW\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	Set_ATE_TX_BW_Proc(pAd, str);
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_POWER0(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER0\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_POWER0_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_POWER1(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER1\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_POWER1_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#ifdef DOT11N_SS3_SUPPORT
static  INT DO_RACFG_CMD_ATE_SET_TX_POWER2(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER2\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_POWER2_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* DOT11N_SS3_SUPPORT */


static  INT DO_RACFG_CMD_ATE_TX_POWER_EVAL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TX_POWER_EVAL\n"));
	
	Set_ATE_TX_POWER_EVALUATION_Proc(pAd, "1");
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_FREQ_OFFSET(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_FREQ_OFFSET\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_FREQ_OFFSET_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}

static  INT DO_RACFG_CMD_ATE_SET_TSSI_ON_OFF(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	SHORT    value = 0;
	UINT32 ret;

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_SET_TSSI_ON_OFF\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);

	if ( value > 0 )
	{
		pATEInfo->bAutoTxAlc = TRUE;
	}
	else
	{
		pATEInfo->bAutoTxAlc = FALSE;


#ifdef RTMP_PCI_SUPPORT
		if(IS_PCI_INF(pAd)) {
			NdisAcquireSpinLock(&pAd->tssi_lock);
		}
#endif

		RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
		value = value & (~TX_ALC_CFG_1_TX0_TEMP_COMP_MASK);
		RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, value);
		RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
		value = value & (~TX_ALC_CFG_2_TX1_TEMP_COMP_MASK);
		RTMP_IO_WRITE32(pAd, TX_ALC_CFG_2, value);

#ifdef RTMP_PCI_SUPPORT
		if (IS_PCI_INF(pAd)) {
			NdisReleaseSpinLock(&pAd->tssi_lock);
		}
#endif


	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}

static  INT DO_RACFG_CMD_ATE_GET_STATISTICS(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_GET_STATISTICS\n"));

	memcpy_exl(pAd, &pRaCfg->data[0], (UCHAR *)&pATEInfo->TxDoneCount, 4);
	memcpy_exl(pAd, &pRaCfg->data[4], (UCHAR *)&pAd->WlanCounters.RetryCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[8], (UCHAR *)&pAd->WlanCounters.FailedCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[12], (UCHAR *)&pAd->WlanCounters.RTSSuccessCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[16], (UCHAR *)&pAd->WlanCounters.RTSFailureCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[20], (UCHAR *)&pAd->WlanCounters.ReceivedFragmentCount.QuadPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[24], (UCHAR *)&pAd->WlanCounters.FCSErrorCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[28], (UCHAR *)&pAd->Counters8023.RxNoBuffer, 4);
	memcpy_exl(pAd, &pRaCfg->data[32], (UCHAR *)&pAd->WlanCounters.FrameDuplicateCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[36], (UCHAR *)&pAd->RalinkCounters.OneSecFalseCCACnt, 4);
	
	if (pATEInfo->RxAntennaSel == 0)
	{
		INT32 RSSI0 = 0;
		INT32 RSSI1 = 0;
		INT32 RSSI2 = 0;

		RSSI0 = (INT32)(pATEInfo->LastRssi0 - pAd->BbpRssiToDbmDelta);
		RSSI1 = (INT32)(pATEInfo->LastRssi1 - pAd->BbpRssiToDbmDelta);
		RSSI2 = (INT32)(pATEInfo->LastRssi2 - pAd->BbpRssiToDbmDelta);
		memcpy_exl(pAd, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		memcpy_exl(pAd, &pRaCfg->data[44], (UCHAR *)&RSSI1, 4);
		memcpy_exl(pAd, &pRaCfg->data[48], (UCHAR *)&RSSI2, 4);
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+52, NDIS_STATUS_SUCCESS);
	}
	else
	{
		INT32 RSSI0 = 0;
	
		RSSI0 = (INT32)(pATEInfo->LastRssi0 - pAd->BbpRssiToDbmDelta);
		memcpy_exl(pAd, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+44, NDIS_STATUS_SUCCESS);
	}

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_RESET_COUNTER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	SHORT    value = 1;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_RESET_COUNTER\n"));				

	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ResetStatCounter_Proc(pAd, str);

	pATEInfo->TxDoneCount = 0;

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SEL_TX_ANTENNA(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)	
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_TX_ANTENNA\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_Antenna_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SEL_RX_ANTENNA(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_RX_ANTENNA\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_RX_Antenna_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_PREAMBLE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_PREAMBLE\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_MODE_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_CHANNEL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_CHANNEL\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_CHANNEL_Proc(pAd, str);

	ATEAsicSwitchChannel(pAd);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR1(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR1\n"));
	memcpy(pATEInfo->Addr1, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR2(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR2\n"));
	memcpy(pATEInfo->Addr2, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR3(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR3\n"));
	memcpy(pATEInfo->Addr3, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_FIXED_PAYLOAD(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	PATE_INFO pATEInfo = &(pAd->ate);

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_SET_FIXED_PAYLOAD: %d\n", value));

	if ( value == 0 )
		Set_ATE_Fixed_Payload_Proc(pAd, "0");
	else
		Set_ATE_Fixed_Payload_Proc(pAd, "1");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_RATE(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_RATE\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);

#ifdef DOT11_VHT_AC
	if ( pAd->ate.vht_nss == 2 )
		value += 16;
#endif /* DOT11_VHT_AC */

	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_MCS_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#ifdef DOT11_VHT_AC
static  INT DO_RACFG_CMD_ATE_SET_VHT_NSS(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	PATE_INFO pATEInfo = &(pAd->ate);

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	value ++;
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_SET_VHT_NSS: %d\n", value));

	if ( value > pAd->chipCap.max_nss )
		value = pAd->chipCap.max_nss;

	pATEInfo->vht_nss = (UCHAR)value;

#ifdef MT76x2
	if ( (pATEInfo->vht_nss == 2) && (pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_VHT) ) {
		pATEInfo->TxWI.TXWI_N.MCS += 16;
	}
#endif /* MT76x2 */

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* VHT_SUPPORT */

static  INT DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_FRAME_LEN\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);
	Set_ATE_TX_LENGTH_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
#ifdef RTMP_MAC_PCI
	PATE_INFO pATEInfo = &(pAd->ate);
#endif /* RTMP_MAC_PCI */
	USHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_FRAME_COUNT\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);

#ifdef RTMP_MAC_PCI
	/* TX_FRAME_COUNT == 0 means tx infinitely */
	if (value == 0)
	{
		/* Use TxCount = 0xFFFFFFFF to approximate the infinity. */
		pATEInfo->TxCount = 0xFFFFFFFF;
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pATEInfo->TxCount));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	}
	else
#endif /* RTMP_MAC_PCI */
	{
		snprintf((char *)str, sizeof(str), "%d", value);
		Set_ATE_TX_COUNT_Proc(pAd, str);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_RX_FRAME(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	Set_ATE_Proc(pAd, "RXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_E2PROM_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE >> 1];
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
	
	rt_ee_read_all(pAd, (USHORT *)buffer);

	if (offset + len <= EEPROM_SIZE)
		memcpy_exs(pAd, pRaCfg->data, (UCHAR *)buffer+offset, len);
	else
		DBGPRINT_ERR(("%s : exceed EEPROM size\n", __FUNCTION__));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE >> 1];
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	memcpy_exs(pAd, (UCHAR *)buffer + offset, (UCHAR *)pRaCfg->data + 2, len);

	if ((offset + len) <= EEPROM_SIZE)
	{
#ifdef RTMP_FLASH_SUPPORT
		if ( pAd->E2pAccessMode == E2P_FLASH_MODE )
		{
			if (pAd->chipCap.ee_inited)
			{
				memcpy_exs(pAd, (UCHAR *)(pAd->eebuf + offset), (UCHAR *)pRaCfg->data + 2, len);

#ifdef MULTIPLE_CARD_SUPPORT
				DBGPRINT(RT_DEBUG_TRACE, ("rtmp_ee_flash_write:pAd->MC_RowID = %d\n", pAd->MC_RowID));
				DBGPRINT(RT_DEBUG_TRACE, ("E2P_OFFSET = 0x%08x\n", pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]));
				if ((pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x48000) || (pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID]==0x40000))
					RtmpFlashWrite(pAd->eebuf, pAd->E2P_OFFSET_IN_FLASH[pAd->MC_RowID], EEPROM_SIZE);
#else
				RtmpFlashWrite(pAd->eebuf, pAd->flash_offset, EEPROM_SIZE);
#endif /* MULTIPLE_CARD_SUPPORT */
			}
		}
		else
#endif /* RTMP_FLASH_SUPPORT */
		rt_ee_write_bulk(pAd,(USHORT *)(((UCHAR *)buffer) + offset), offset, len);
	}
	else
	{
		DBGPRINT_ERR(("%s : exceed EEPROM size(%d)\n", __FUNCTION__, EEPROM_SIZE));
		DBGPRINT(RT_DEBUG_ERROR,("offset=%u\n", offset));
		DBGPRINT(RT_DEBUG_ERROR,("length=%u\n", len));
		DBGPRINT(RT_DEBUG_ERROR,("offset+length=%u\n", (offset+len)));
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_IO_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 offset, pos, value;
	USHORT len;
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);
	memcpy(&len, pRaCfg->data+2, 2);
	len = OS_NTOHS(len);
	
//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));	
	for (pos = 0; pos < len; pos += 4)
	{
		memcpy_exl(pAd, (UCHAR *)&value, pRaCfg->data+4+pos, 4);
		RTMP_IO_WRITE32(pAd, ((offset+pos) & (0xffff)), value);
//		DBGPRINT(RT_DEBUG_WARN,("mac w 0x%04X=0x%08X\n", offset + pos, value));
		DBGPRINT(RT_DEBUG_WARN,("iwpriv %s0 mac %04X=%08X\n"
			, INF_MAIN_DEV_NAME, offset + pos, value));
	}
//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_BBP_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT pos;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
	
	DBGPRINT(RT_DEBUG_WARN,("\n\n"));	
	for (pos = offset; pos < (offset+len); pos++)
	{
		pRaCfg->data[pos - offset] = 0;
		
		ATE_BBPRead(pAd, pos, &pRaCfg->data[pos - offset]);
		DBGPRINT(RT_DEBUG_WARN,("bbp r R%u=0x%02X\n"
			, pos, pRaCfg->data[pos - offset]));
	}
	DBGPRINT(RT_DEBUG_WARN,("\n\n"));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_BBP_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT pos;
	UCHAR *value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
					
	DBGPRINT(RT_DEBUG_WARN,("\n\n"));					
	for (pos = offset; pos < (offset+len); pos++)
	{
		value = pRaCfg->data + 2 + (pos - offset);
		ATE_BBPWrite(pAd, pos,  *value);
		DBGPRINT(RT_DEBUG_WARN,("bbp w R%u=0x%02X\n", pos, *value));
	}
	DBGPRINT(RT_DEBUG_WARN,("\n\n"));					

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}










#ifdef RTMP_RF_RW_SUPPORT
#ifndef RLT_RF
static  INT DO_RACFG_CMD_ATE_RF_READ_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT pos;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	for (pos = offset; pos < (offset+len); pos++)
	{
		pRaCfg->data[pos - offset] = 0;
		ATE_RF_IO_READ8_BY_REG_ID(pAd, pos,  &pRaCfg->data[pos - offset]);
		DBGPRINT(RT_DEBUG_WARN,("rf r R%u=0x%02X\n"
			, pos, pRaCfg->data[pos - offset]));
	}
	DBGPRINT(RT_DEBUG_WARN,("\n\n"));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_RF_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT pos;
	UCHAR *value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	for (pos = offset; pos < (offset+len); pos++)
	{
		value = pRaCfg->data + 2 + (pos - offset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, pos, *value);
		DBGPRINT(RT_DEBUG_WARN,("rf w R%u=0x%02X\n", pos, *value));
	}
	DBGPRINT(RT_DEBUG_WARN,("\n\n"));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* !RLT_RF */


#ifdef RLT_RF
static  INT DO_RACFG_CMD_ATE_RF_READ_BULK_BANK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT bank;
	USHORT len;
	USHORT pos;

	bank = OS_NTOHS(pRaCfg->status);
	memcpy(&offset, pRaCfg->data, 2 /* sizeof(offset) */);
	offset = OS_NTOHS(offset);
	memcpy(&len, pRaCfg->data + 2 /* sizeof(offset) */, 2 /* sizeof(len) */);
	len = OS_NTOHS(len);

//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	for (pos = offset; pos < (offset+len); pos++)
	{
		pRaCfg->data[pos - offset] = 0;
		ATE_RF_IO_READ8_BY_REG_ID(pAd, bank, pos, &pRaCfg->data[pos - offset]);
//		DBGPRINT(RT_DEBUG_WARN,("rf bank%u r R%u=0x%02X\n"
//			, bank, pos, pRaCfg->data[pos - offset]));
		DBGPRINT(RT_DEBUG_TRACE,("rf bank%u r R%u=0x%02X\n"
			, bank, pos, pRaCfg->data[pos - offset]));
	}
//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	
	ResponseToGUI(pRaCfg, wrq, 2/* sizeof(pRaCfg->status) */+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_RF_WRITE_BULK_BANK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT bank;
	USHORT len;
	USHORT pos;
	UCHAR *value;
	
	bank = OS_NTOHS(pRaCfg->status);
	memcpy(&offset, pRaCfg->data, 2 /* sizeof(offset) */);
	offset = OS_NTOHS(offset);
	memcpy(&len, pRaCfg->data + 2 /* sizeof(offset) */, 2 /* sizeof(len) */);
	len = OS_NTOHS(len);

//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	for (pos = offset; pos < (offset+len); pos++)
	{
		value = pRaCfg->data + sizeof(offset) + sizeof(len) + (pos - offset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, bank, pos, *value);
//		DBGPRINT(RT_DEBUG_WARN,("rf bank%u w R%u=0x%02X\n", bank, pos, *value));
		DBGPRINT(RT_DEBUG_WARN,("iwpriv %s0 set rf=%u-%u-%02X\n"
			, INF_MAIN_DEV_NAME, bank, pos, *value));
	}
//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* RLT_RF */
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef MT_RF
static  INT DO_RACFG_CMD_ATE_MT_RF_READ_BULK_BANK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 offset;
	UINT32 bank;
	UINT32 len;
	USHORT pos;

	memcpy(&bank, &pRaCfg->status, 4);
	bank = OS_NTOHL(bank);

	memcpy(&offset, pRaCfg->data + 2, 4 /* sizeof(offset) */);
	offset = OS_NTOHL(offset);

	memcpy(&len, pRaCfg->data + 6 /* sizeof(offset) */, 4 /* sizeof(len) */);
	len = OS_NTOHL(len);

//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	for (pos = offset; pos < (offset+len); pos += 4)
	{
		*(UINT32 *)(&pRaCfg->data[pos - offset])= 0;
		mt_rf_read(pAd, bank, pos, &pRaCfg->data[pos - offset]);
//		DBGPRINT(RT_DEBUG_WARN,("rf bank%u r R%u=0x%02X\n"
//			, bank, pos, pRaCfg->data[pos - offset]));
		DBGPRINT(RT_DEBUG_TRACE,("rf bank%u r 0x%04X=0x%04X\n"
			, bank, pos, *(UINT32 *)&pRaCfg->data[pos - offset]));
	}
//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));


/*
	for (index = 0 ; index < (len >> 2); index++)
	{
		pDst = (dst + (index << 2));
		RTMP_IO_READ32(pAd, offset, &Value);
		DBGPRINT(RT_DEBUG_WARN,("mac r 0x%04X=0x%08X\n", offset, Value));
		
		Value = OS_HTONL(Value);
		memmove(pDst, &Value, 4);
		offset += 4;
	}
*/





	ResponseToGUI(pRaCfg, wrq, 2/* sizeof(pRaCfg->status) */+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_MT_RF_WRITE_BULK_BANK(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 offset;
	UINT32 bank;
	UINT32 len;
	USHORT pos;
	UINT32 *value;
	
	memcpy(&bank, &pRaCfg->status, 4);
	bank = OS_NTOHL(bank);

	memcpy(&offset, pRaCfg->data + 2, 4 /* sizeof(offset) */);
	offset = OS_NTOHL(offset);

	memcpy(&len, pRaCfg->data + 6 /* sizeof(offset) */, 4 /* sizeof(len) */);
	len = OS_NTOHL(len);

//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	for (pos = offset; pos < (offset+len); pos += 4)
	{
		value = pRaCfg->data + 2 + sizeof(offset) + sizeof(len) + (pos - offset);
		mt_rf_write(pAd, bank, pos, *value);
//		DBGPRINT(RT_DEBUG_WARN,("rf bank%u w R%u=0x%02X\n", bank, pos, *value));
		DBGPRINT(RT_DEBUG_WARN,("iwpriv %s0 set rf=%u-%u-%02X\n"
			, INF_MAIN_DEV_NAME, bank, pos, *value));
	}
//	DBGPRINT(RT_DEBUG_WARN,("\n\n"));
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* RLT_RF */


static  INT DO_RACFG_CMD_TX_START_V2(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	USHORT *p;
	USHORT	err = 1;
	USHORT TXWISize = 0;
	UINT32 TxWIMPDUByteCnt;

	if ((pATEInfo->TxStatus != 0) && (pATEInfo->Mode & ATE_TXFRAME))
	{
		DBGPRINT_ERR(("%s : Ate Tx is already running,"
			"to run next Tx, you must stop it first\n", __FUNCTION__));
		err = 2;
		goto tx_start_error;
	}
	else if ((pATEInfo->TxStatus != 0) && !(pATEInfo->Mode & ATE_TXFRAME))
	{
		int slot = 0;

		while ((slot++ < 10) && (pATEInfo->TxStatus != 0))
		{
			RtmpOsMsDelay(5);
		}

		/* force it to stop */
		pATEInfo->TxStatus = 0;
		pATEInfo->TxDoneCount = 0;
		pATEInfo->bQATxStart = FALSE;
	}

	/*
		Reset ATE mode and set Tx/Rx idle
		for new proposed TXCONT/TXCARR/TXCARRSUPP solution.
	*/
	if ((pATEInfo->Mode & ATE_TXFRAME) && (pATEInfo->TxMethod == TX_METHOD_1))
	{
		TXSTOP(pAd);
	}

	/*
		If pRaCfg->length == 0, this "RACFG_CMD_TX_START"
		is for Carrier test or Carrier Suppression test.
	*/
	if (OS_NTOHS(pRaCfg->length) != 0)
	{

		NdisMoveMemory(&pATEInfo->TxInfoLen, pRaCfg->data - 2, 2);
		pATEInfo->TxInfoLen = OS_NTOHS(pATEInfo->TxInfoLen);
		DBGPRINT(RT_DEBUG_TRACE,("TxInfoLen = %d\n", pATEInfo->TxInfoLen));

		NdisMoveMemory(&pATEInfo->TxInfo, pRaCfg->data, pATEInfo->TxInfoLen);
#ifdef RT_BIG_ENDIAN
		*((UINT32 *)(&pATEInfo->TxInfo)) = SWAP32(*((UINT32 *)(&pATEInfo->TxInfo)));
/*		RTMPDescriptorEndianChange((PUCHAR) &pATEInfo->TxInfo, TYPE_TXINFO); */
#endif /* RT_BIG_ENDIAN */

		NdisMoveMemory(&pATEInfo->TxWILen, pRaCfg->data+4, 2);
		pATEInfo->TxWILen = OS_NTOHS(pATEInfo->TxWILen);
		DBGPRINT(RT_DEBUG_TRACE,("TxWILen = %d\n", pATEInfo->TxWILen));
		TXWISize = pATEInfo->TxWILen;
		NdisMoveMemory(&pATEInfo->TxWI, pRaCfg->data+4+2, pATEInfo->TxWILen);						
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, (PUCHAR)&pATEInfo->TxWI, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */

		NdisMoveMemory(&pATEInfo->TxCount, pRaCfg->data+4+2+TXWISize, 4);
		pATEInfo->TxCount = OS_NTOHL(pATEInfo->TxCount);
		DBGPRINT(RT_DEBUG_TRACE,("TxCount = %u\n", pATEInfo->TxCount));

		/* always use QID_AC_BE */
		pATEInfo->QID = 0;
		p = (USHORT *)(&pRaCfg->data[4+2+TXWISize+4+2]);
		pATEInfo->HLen = OS_NTOHS(*p);
		DBGPRINT(RT_DEBUG_TRACE,("HLen = %d\n", pATEInfo->HLen));

		if (pATEInfo->HLen > 32)
		{
			DBGPRINT_ERR(("%s : pATEInfo->HLen > 32\n", __FUNCTION__));
			DBGPRINT_ERR(("pATEInfo->HLen = %d\n", pATEInfo->HLen));
			err = 3;

			goto tx_start_error;
		}

		NdisMoveMemory(&pATEInfo->Header, pRaCfg->data + (4+2+TXWISize+4+2+2), pATEInfo->HLen);
		pATEInfo->PLen = OS_NTOHS(pRaCfg->length) - (pATEInfo->HLen + (4+2+TXWISize+4+2+2+2));
		DBGPRINT(RT_DEBUG_TRACE,("PLen = %d\n", pATEInfo->PLen));

		if (pATEInfo->PLen > 32)
		{
			DBGPRINT_ERR(("%s : pATEInfo->PLen > 32\n", __FUNCTION__));
			err = 4;

			goto tx_start_error;
		}

		NdisMoveMemory(&pATEInfo->Pattern, pRaCfg->data + (4+2+TXWISize+4+2+2) + pATEInfo->HLen, pATEInfo->PLen);
		DBGPRINT(RT_DEBUG_TRACE,("pattern = 0x%02x\n", pATEInfo->Pattern[0]));

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
			TxWIMPDUByteCnt = pATEInfo->TxWI.TXWI_O.MPDUtotalByteCnt;
		else
#endif /* RTMP_MAC */
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
				TxWIMPDUByteCnt = pATEInfo->TxWI.TXWI_N.MPDUtotalByteCnt;
		}
		else
#endif /* RLT_MAC */
		{
			DBGPRINT(RT_DEBUG_OFF, ("Err!!Unknown HIF type(%d)!\n", pAd->chipCap.hif_type));
			TxWIMPDUByteCnt = pATEInfo->HLen;
		}
		pATEInfo->DLen = TxWIMPDUByteCnt - pATEInfo->HLen;
		DBGPRINT(RT_DEBUG_TRACE,("data length = %d\n", pATEInfo->DLen));
	}

#ifdef RTMP_MAC_PCI
	if (pATEInfo->TxCount == 0)
	{
		pATEInfo->TxCount = 0xFFFFFFFF;
	}
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("START TXFRAME V2\n"));
	pATEInfo->bQATxStart = TRUE;
	Set_ATE_Proc(pAd, "TXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;


tx_start_error:
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), err);

	return err;
}


#ifdef TXBF_SUPPORT
static  INT DO_RACFG_CMD_ATE_TXBF_DUT_INIT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_DUT_INIT\n"));

	Set_ATE_TXBF_INIT_Proc(pAd, "1");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_LNA_CAL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT band;
	CHAR bandStr[32] = {0};
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_LNA_CAL\n"));

	band = OS_NTOHS(pRaCfg->status);
	DBGPRINT(RT_DEBUG_TRACE, ("%s : band=0x%x(0x%x)\n", 
					__FUNCTION__, band, pRaCfg->status));
	snprintf(bandStr, sizeof(bandStr), "%d\n", band);
	Set_ATE_TXBF_LNACAL_Proc(pAd, &bandStr[0]);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_DIV_CAL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT band;
	CHAR bandStr[32] = {0};
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_DIV_CAL\n"));

	band = OS_NTOHS(pRaCfg->status);
	DBGPRINT(RT_DEBUG_TRACE, ("%s : band=0x%x(0x%x)\n", 
				__FUNCTION__, band, pRaCfg->status));
	snprintf(bandStr, sizeof(bandStr), "%d\n", band);
	Set_ATE_TXBF_DIVCAL_Proc(pAd, &bandStr[0]);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_PHASE_CAL(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];
	BOOLEAN	result = FALSE;

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_PHASE_CAL\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	result = (BOOLEAN)Set_ATE_TXBF_CAL_Proc(pAd, str);
	pRaCfg->data[0] = result;

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + 1, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_GOLDEN_INIT(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_GOLDEN_INIT\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	Set_ATE_TXBF_GOLDEN_Proc(pAd, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_VERIFY(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];
	BOOLEAN	result;

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_VERIFY\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	result = (BOOLEAN)Set_ATE_TXBF_VERIFY_Proc(pAd, str);

	pRaCfg->data[0] = result;
	pRaCfg->data[1] = pATEInfo->calParams[0];
	pRaCfg->data[2] = pATEInfo->calParams[1];
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + 3, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_TXBF_VERIFY_NOCOMP(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];
	BOOLEAN	result;

	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_TXBF_VERIFY_NOCOMP\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	snprintf((char *)str, sizeof(str), "%d", value);

	result = (BOOLEAN)Set_ATE_TXBF_VERIFY_NoComp_Proc(pAd, str);

	pRaCfg->data[0] = result;
	pRaCfg->data[1] = pATEInfo->calParams[0];
	pRaCfg->data[2] = pATEInfo->calParams[1];
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + 3, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif /* TXBF_SUPPORT */


static  INT DO_RACFG_CMD_ATE_GET_HW_COUNTER(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	
	DBGPRINT(RT_DEBUG_TRACE,("DO_RACFG_CMD_ATE_GET_HW_COUNTER\n"));

	memcpy_exl(pAd, &pRaCfg->data[0], (UCHAR *)&pAd->WlanCounters.FCSErrorCount.u.LowPart, 4);
	memcpy_exl(pAd, &pRaCfg->data[4], (UCHAR *)&pAd->Counters8023.RxNoBuffer, 4);
	memcpy_exl(pAd, &pRaCfg->data[8], (UCHAR *)&pAd->RalinkCounters.PhyErrCnt, 4);
	memcpy_exl(pAd, &pRaCfg->data[12], (UCHAR *)&pAd->RalinkCounters.FalseCCACnt, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+16, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static INT32 DO_RACFG_CMD_ATE_SHOW_PARAM(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT32 Status = NDIS_STATUS_SUCCESS;
	UINT32 Len;
	ATE_EX_PARAM ATEExParam;
	UCHAR phy_mode = 0, bw = 0, mcs = 0, sgi = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		phy_mode = pATEInfo->TxWI.TXWI_N.PHYMODE;
		bw = pATEInfo->TxWI.TXWI_N.BW;
		mcs = pATEInfo->TxWI.TXWI_N.MCS;
		sgi = pATEInfo->TxWI.TXWI_N.ShortGI;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		phy_mode = pATEInfo->TxWI.TXWI_O.PHYMODE;
		bw = pATEInfo->TxWI.TXWI_O.BW;
		mcs = pATEInfo->TxWI.TXWI_O.MCS;
		sgi = pATEInfo->TxWI.TXWI_O.ShortGI;
	}
#endif /* RTMP_MAC */

	ATEExParam.mode = pATEInfo->Mode;
	ATEExParam.TxPower0 = pATEInfo->TxPower0;
	ATEExParam.TxPower1 = pATEInfo->TxPower1;

#ifdef DOT11N_SS3_SUPPORT
	ATEExParam.TxPower2 = pATEInfo->TxPower2;
#endif /* DOT11N_SS3_SUPPORT */

	ATEExParam.TxAntennaSel = pATEInfo->TxAntennaSel;
	ATEExParam.RxAntennaSel = pATEInfo->RxAntennaSel;

#ifdef CONFIG_AP_SUPPORT
	NdisMoveMemory(ATEExParam.DA, pATEInfo->Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(ATEExParam.SA, pATEInfo->Addr3, MAC_ADDR_LEN);
	NdisMoveMemory(ATEExParam.BSSID, pATEInfo->Addr2, MAC_ADDR_LEN);
#endif /* CONFIG_AP_SUPPORT */


	ATEExParam.MCS = mcs;
	ATEExParam.PhyMode = phy_mode;
	ATEExParam.ShortGI = sgi;
	ATEExParam.BW = bw;
	ATEExParam.Channel = OS_HTONL(pATEInfo->Channel);
	ATEExParam.TxLength = OS_HTONL(pATEInfo->TxLength);
	ATEExParam.TxCount = OS_HTONL(pATEInfo->TxCount);
	ATEExParam.RFFreqOffset = OS_HTONL(pATEInfo->RFFreqOffset);
	ATEExParam.IPG = OS_HTONL(pATEInfo->IPG);
	ATEExParam.RxTotalCnt = OS_HTONL(pATEInfo->RxTotalCnt);
	ATEExParam.RxCntPerSec = OS_HTONL(pATEInfo->RxCntPerSec);
	ATEExParam.LastSNR0 = pATEInfo->LastSNR0;
	ATEExParam.LastSNR1 = pATEInfo->LastSNR1;
#ifdef DOT11N_SS3_SUPPORT
	ATEExParam.LastSNR2 = pATEInfo->LastSNR2;
#endif /* DOT11N_SS3_SUPPORT */
	ATEExParam.LastRssi0 = pATEInfo->LastRssi0;
	ATEExParam.LastRssi1 = pATEInfo->LastRssi1;
	ATEExParam.LastRssi2 = pATEInfo->LastRssi2;
	ATEExParam.AvgRssi0 = pATEInfo->AvgRssi0;
	ATEExParam.AvgRssi1 = pATEInfo->AvgRssi1;
	ATEExParam.AvgRssi2 = pATEInfo->AvgRssi2;
	ATEExParam.AvgRssi0X8 = OS_HTONS(pATEInfo->AvgRssi0X8);
	ATEExParam.AvgRssi1X8 = OS_HTONS(pATEInfo->AvgRssi1X8);
	ATEExParam.AvgRssi2X8 = OS_HTONS(pATEInfo->AvgRssi2X8);

	Len = sizeof(ATEExParam);
	NdisMoveMemory(pRaCfg->data, &ATEExParam, Len);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status) + Len, NDIS_STATUS_SUCCESS);
	
	return Status;
}


typedef INT (*RACFG_CMD_HANDLER)(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN  struct ate_racfghdr *pRaCfg);


static RACFG_CMD_HANDLER RACFG_CMD_SET1[] =
{
	/* cmd id start from 0x0 */
	DO_RACFG_CMD_RF_WRITE_ALL,/* 0x0000 */	
	DO_RACFG_CMD_E2PROM_READ16,/* 0x0001 */
	DO_RACFG_CMD_E2PROM_WRITE16,/* 0x0002 */
	DO_RACFG_CMD_E2PROM_READ_ALL,/* 0x0003 */
	DO_RACFG_CMD_E2PROM_WRITE_ALL,/* 0x0004 */
	DO_RACFG_CMD_IO_READ,/* 0x0005 */
	DO_RACFG_CMD_IO_WRITE,/* 0x0006 */
	DO_RACFG_CMD_IO_READ_BULK,/* 0x0007 */
	DO_RACFG_CMD_BBP_READ8,/* 0x0008 */
	DO_RACFG_CMD_BBP_WRITE8,/* 0x0009 */
	DO_RACFG_CMD_BBP_READ_ALL,/* 0x000a */
	DO_RACFG_CMD_GET_COUNTER,/* 0x000b */
	DO_RACFG_CMD_CLEAR_COUNTER,/* 0x000c */
	NULL /* RACFG_CMD_RSV1 */,/* 0x000d */
	NULL /* RACFG_CMD_RSV2 */,/* 0x000e */
	NULL /* RACFG_CMD_RSV3 */,/* 0x000f */
	DO_RACFG_CMD_TX_START,/* 0x0010 */
	DO_RACFG_CMD_GET_TX_STATUS,/* 0x0011 */
	DO_RACFG_CMD_TX_STOP,/* 0x0012 */
	DO_RACFG_CMD_RX_START,/* 0x0013 */
	DO_RACFG_CMD_RX_STOP,/* 0x0014 */
	DO_RACFG_CMD_GET_NOISE_LEVEL,/* 0x0015 */
	NULL
	/* cmd id end with 0x20 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET2[] =
{
	/* cmd id start from 0x80 */
	DO_RACFG_CMD_ATE_START,/* 0x0080 */
	DO_RACFG_CMD_ATE_STOP/* 0x0081 */
	/* cmd id end with 0x81 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET3[] =
{
	/* cmd id start from 0x100 */
	DO_RACFG_CMD_ATE_START_TX_CARRIER,/* 0x0100 */
	DO_RACFG_CMD_ATE_START_TX_CONT,/* 0x0101 */
	DO_RACFG_CMD_ATE_START_TX_FRAME,/* 0x0102 */
	DO_RACFG_CMD_ATE_SET_BW,/* 0x0103 */
	DO_RACFG_CMD_ATE_SET_TX_POWER0,/* 0x0104 */
	DO_RACFG_CMD_ATE_SET_TX_POWER1,/* 0x0105 */
	DO_RACFG_CMD_ATE_SET_FREQ_OFFSET,/* 0x0106 */
	DO_RACFG_CMD_ATE_GET_STATISTICS,/* 0x0107 */
	DO_RACFG_CMD_ATE_RESET_COUNTER,/* 0x0108 */
	DO_RACFG_CMD_ATE_SEL_TX_ANTENNA,/* 0x0109 */
	DO_RACFG_CMD_ATE_SEL_RX_ANTENNA,/* 0x010a */
	DO_RACFG_CMD_ATE_SET_PREAMBLE,/* 0x010b */
	DO_RACFG_CMD_ATE_SET_CHANNEL,/* 0x010c */
	DO_RACFG_CMD_ATE_SET_ADDR1,/* 0x010d */
	DO_RACFG_CMD_ATE_SET_ADDR2,/* 0x010e */
	DO_RACFG_CMD_ATE_SET_ADDR3,/* 0x010f */
	DO_RACFG_CMD_ATE_SET_RATE,/* 0x0110 */
	DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN,/* 0x0111 */
	DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT,/* 0x0112 */
	DO_RACFG_CMD_ATE_START_RX_FRAME,/* 0x0113 */
	DO_RACFG_CMD_ATE_E2PROM_READ_BULK,/* 0x0114 */
	DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK,/* 0x0115 */
	DO_RACFG_CMD_ATE_IO_WRITE_BULK,/* 0x0116 */
	DO_RACFG_CMD_ATE_BBP_READ_BULK,/* 0x0117 */
	DO_RACFG_CMD_ATE_BBP_WRITE_BULK,/* 0x0118 */
#ifdef RTMP_RF_RW_SUPPORT
#ifndef RLT_RF
	DO_RACFG_CMD_ATE_RF_READ_BULK,/* 0x0119 */
	DO_RACFG_CMD_ATE_RF_WRITE_BULK,/* 0x011a */
#else
	NULL,/* 0x0119 */
	NULL,/* 0x011a */
#endif /* !RLT_RF */
#else
	NULL,/* 0x0119 */
	NULL,/* 0x011a */
#endif /* RTMP_RF_RW_SUPPORT */
#ifdef DOT11N_SS3_SUPPORT
	DO_RACFG_CMD_ATE_SET_TX_POWER2,/* 0x011b */
#else
	NULL,/* 0x011b */
#endif /* DOT11N_SS3_SUPPORT */
#ifdef TXBF_SUPPORT
	DO_RACFG_CMD_ATE_TXBF_DUT_INIT,/* 0x011c */
	DO_RACFG_CMD_ATE_TXBF_LNA_CAL,/* 0x011d */
	DO_RACFG_CMD_ATE_TXBF_DIV_CAL,/* 0x011e */
	DO_RACFG_CMD_ATE_TXBF_PHASE_CAL,/* 0x011f */
	DO_RACFG_CMD_ATE_TXBF_GOLDEN_INIT,/* 0x0120 */
	DO_RACFG_CMD_ATE_TXBF_VERIFY,/* 0x0121 */
	DO_RACFG_CMD_ATE_TXBF_VERIFY_NOCOMP,/* 0x0122 */
#else
	NULL,/* 0x011c */
	NULL,/* 0x011d */
	NULL,/* 0x011e */
	NULL,/* 0x011f */
	NULL,/* 0x0120 */
	NULL,/* 0x0121 */
	NULL,/* 0x0122 */
#endif /* TXBF_SUPPORT */

#ifdef RTMP_RF_RW_SUPPORT
#ifdef RLT_RF
	DO_RACFG_CMD_ATE_RF_READ_BULK_BANK,/* 0x0123 */
	DO_RACFG_CMD_ATE_RF_WRITE_BULK_BANK,/* 0x0124 */
#else
	NULL,/* 0x0123 */
	NULL,/* 0x0124 */
#endif /* RLT_RF */
#else
	NULL,/* 0x0123 */
	NULL,/* 0x0124 */
#endif /* RTMP_RF_RW_SUPPORT */
	DO_RACFG_CMD_TX_START_V2,/* 0x0125 */
	/* cmd id end with 0x125 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET4[] =
{
	/* cmd id start from 0x200 */
	NULL,/* 0x0200 */
	NULL,/* 0x0201 */
	NULL,/* 0x0202 */
	NULL,/* 0x0203 */
	NULL,/* 0x0204 */
	NULL,/* 0x0205 */
	NULL,/* 0x0206 */

	/* cmd id end with 0x206 */
};


static RACFG_CMD_HANDLER RACFG_CMD_SET5[] =
{
	DO_RACFG_CMD_ATE_SHOW_PARAM
};

#ifdef MT76x2
static RACFG_CMD_HANDLER MT76x2_CMD_SET1[] =
{
	/* cmd id start from 0x1000 */
	DO_RACFG_CMD_ATE_START,	/* 0x1000 */
	DO_RACFG_CMD_ATE_STOP,	/* 0x1001 */
	DO_RACFG_CMD_ATE_START_TX_FRAME_V2,	/* 0x1002 */
	NULL,	/* 0x1003 */
	DO_RACFG_CMD_ATE_START_TX_CONT,	/* 0x1004 */
	DO_RACFG_CMD_ATE_START_TX_CARRIER,	/* 0x1005 */
	DO_RACFG_CMD_ATE_START_RX_FRAME,	/* 0x1006 */
	DO_RACFG_CMD_TX_STOP,	/* 0x1007 */
	DO_RACFG_CMD_TX_STOP,	/* 0x1008 */
	DO_RACFG_CMD_TX_STOP,	/* 0x1009 */
	DO_RACFG_CMD_RX_STOP,	/* 0x100A */
	DO_RACFG_CMD_ATE_SEL_TX_ANTENNA,	/* 0x100B */
	DO_RACFG_CMD_ATE_SEL_RX_ANTENNA,	/* 0x100C */
	DO_RACFG_CMD_ATE_SET_IPG,	/* 0x100D */
	DO_RACFG_CMD_ATE_SET_TX_POWER0,	/* 0x100E */
	DO_RACFG_CMD_ATE_SET_TX_POWER1,	/* 0x100F */
	DO_RACFG_CMD_ATE_TX_POWER_EVAL,	/* 0x1010 */

};

static RACFG_CMD_HANDLER MT76x2_CMD_SET2[] =
{
	/* cmd id start from 0x1100 */
	DO_RACFG_CMD_ATE_SET_CHANNEL,	/* 0x1100 */
	DO_RACFG_CMD_ATE_SET_PREAMBLE,	/* 0x1101 */
	DO_RACFG_CMD_ATE_SET_RATE,		/* 0x1102 */
#ifdef DOT11_VHT_AC
	DO_RACFG_CMD_ATE_SET_VHT_NSS,	/* 0x1103 */
#endif /* DOT11_VHT_AC */
	DO_RACFG_CMD_ATE_SET_BW,	/* 0x1104 */
	NULL,	/* 0x1105 */
	NULL,	/* 0x1106 */
	DO_RACFG_CMD_ATE_SET_FREQ_OFFSET,	/* 0x1107 */
	NULL,	/* 0x1108 */
	DO_RACFG_CMD_ATE_SET_TSSI_ON_OFF,	/* 0x1109 */
	NULL,	/* 0x110A */
	DO_RACFG_CMD_ATE_SET_FIXED_PAYLOAD, /* 0x110B */
};

static RACFG_CMD_HANDLER MT76x2_CMD_SET3[] =
{
	/* cmd id start from 0x1200 */
	DO_RACFG_CMD_ATE_RESET_COUNTER,	/* 0x1200 */
	DO_RACFG_CMD_GET_COUNTER,	/* 0x1201 */
	NULL,	/* 0x1202 */
	NULL,	/* 0x1203 */
	NULL,	/* 0x1204 */
	DO_RACFG_CMD_GET_TX_STATUS,	/* 0x1205 */
	DO_RACFG_CMD_ATE_GET_HW_COUNTER,	/* 0x1206 */
	DO_RACFG_CMD_ATE_MT76x2_Calibration,	/* 0x1207 */
};

static RACFG_CMD_HANDLER MT76x2_CMD_SET4[] =
{
	/* cmd id start from 0x1300 */
	DO_RACFG_CMD_IO_READ,	/* 0x1300 */
	DO_RACFG_CMD_IO_WRITE,	/* 0x1301 */
	DO_RACFG_CMD_IO_READ_BULK,	/* 0x1302 */
	DO_RACFG_CMD_ATE_MT_RF_READ_BULK_BANK,	/* 0x1303 */
	DO_RACFG_CMD_ATE_MT_RF_WRITE_BULK_BANK,	/* 0x1304 */
	DO_RACFG_CMD_E2PROM_READ16,	/* 0x1305 */
	DO_RACFG_CMD_E2PROM_WRITE16,	/* 0x1306 */
	DO_RACFG_CMD_ATE_E2PROM_READ_BULK,	/* 0x1307 */
	DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK,	/* 0x1308 */
};
#endif /* MT76x2 */

typedef struct RACFG_CMD_TABLE_{
	RACFG_CMD_HANDLER *cmdSet;
	int	cmdSetSize;
	int	cmdOffset;
}RACFG_CMD_TABLE;


RACFG_CMD_TABLE RACFG_CMD_TABLES[]={
	{
		RACFG_CMD_SET1,
		sizeof(RACFG_CMD_SET1) / sizeof(RACFG_CMD_HANDLER),
		0x0,
	},
	{
		RACFG_CMD_SET2,
		sizeof(RACFG_CMD_SET2) / sizeof(RACFG_CMD_HANDLER),
		0x80,
	},
	{
		RACFG_CMD_SET3,
		sizeof(RACFG_CMD_SET3) / sizeof(RACFG_CMD_HANDLER),
		0x100,
	},
	{
		RACFG_CMD_SET4,
		sizeof(RACFG_CMD_SET4) / sizeof(RACFG_CMD_HANDLER),
		0x200,
	},
	{
		RACFG_CMD_SET5,
		sizeof(RACFG_CMD_SET5) / sizeof(RACFG_CMD_HANDLER),
		0xff00,
	},
#ifdef MT76x2
	{
		MT76x2_CMD_SET1,
		sizeof(MT76x2_CMD_SET1) / sizeof(RACFG_CMD_HANDLER),
		0x1000,
	},
	{
		MT76x2_CMD_SET2,
		sizeof(MT76x2_CMD_SET2) / sizeof(RACFG_CMD_HANDLER),
		0x1100,
	},
	{
		MT76x2_CMD_SET3,
		sizeof(MT76x2_CMD_SET3) / sizeof(RACFG_CMD_HANDLER),
		0x1200,
	},
	{
		MT76x2_CMD_SET4,
		sizeof(MT76x2_CMD_SET4) / sizeof(RACFG_CMD_HANDLER),
		0x1300,
	},

#endif /* MT76x2 */
};


static INT32 RACfgCMDHandler(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN pRACFGHDR pRaCfg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT32 Status = NDIS_STATUS_SUCCESS;
	USHORT Command_Id;
	UINT32 TableIndex = 0;

	Command_Id = OS_NTOHS(pRaCfg->command_id);
	DBGPRINT(RT_DEBUG_TRACE,("\n%s: Command_Id = 0x%04x !\n", __FUNCTION__, Command_Id));
	
	while (TableIndex < (sizeof(RACFG_CMD_TABLES) / sizeof(RACFG_CMD_TABLE)))
	{
 		int cmd_index = 0;
 		cmd_index = Command_Id - RACFG_CMD_TABLES[TableIndex].cmdOffset;
 		if ((cmd_index >= 0) && (cmd_index < RACFG_CMD_TABLES[TableIndex].cmdSetSize))
 		{
			RACFG_CMD_HANDLER *pCmdSet;

			pCmdSet = RACFG_CMD_TABLES[TableIndex].cmdSet;
			
			if (pCmdSet[cmd_index] != NULL)
				Status = (*pCmdSet[cmd_index])(pAd, wrq, pRaCfg);
			break;
		}
		TableIndex++;
	}

	/* In passive mode, only commands that read registers are allowed. */ 
	if (pATEInfo->PassiveMode)
	{
		int entry, allowCmd = FALSE;
		static int allowedCmds[] = {
				RACFG_CMD_E2PROM_READ16, RACFG_CMD_E2PROM_READ_ALL,
				RACFG_CMD_IO_READ, RACFG_CMD_IO_READ_BULK,
				RACFG_CMD_BBP_READ8, RACFG_CMD_BBP_READ_ALL,
				RACFG_CMD_ATE_E2PROM_READ_BULK,
				RACFG_CMD_ATE_BBP_READ_BULK,
#ifdef RTMP_RF_RW_SUPPORT
				RACFG_CMD_ATE_RF_READ_BULK,
#ifdef RLT_RF
				RACFG_CMD_ATE_RF_READ_BULK_BANK,
#endif /* RLT_RF */
#endif /* RTMP_RF_RW_SUPPORT */
				};

		for (entry=0; entry<sizeof(allowedCmds)/sizeof(allowedCmds[0]); entry++)
		{
			if (Command_Id==allowedCmds[entry])
			{
				allowCmd = TRUE;
				break;
			}
		}

		/* Also allow writes to BF profile registers */
		if (Command_Id==RACFG_CMD_BBP_WRITE8)
		{
			int offset = OS_NTOHS(pRaCfg->status);
			if (offset==BBP_R27 || (offset>=BBP_R174 && offset<=BBP_R182))
				allowCmd = TRUE;
		}

		/* If not allowed, then ignore the command. */
		if (!allowCmd)
		{
			ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
			Status = NDIS_STATUS_FAILURE;
		}
	}

	return Status;
}


INT RtmpDoAte(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT		*wrq,
	IN	PSTRING			wrq_name)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	struct ate_racfghdr *pRaCfg;
	UINT32 ATEMagicNum;

	os_alloc_mem_suspend(pAd, (UCHAR **)&pRaCfg, sizeof(struct ate_racfghdr));

	if (!pRaCfg)
	{
		Status = -ENOMEM;
		goto ERROR0;
	}
				
	NdisZeroMemory(pRaCfg, sizeof(struct ate_racfghdr));
	Status = copy_from_user((PUCHAR)pRaCfg, wrq->u.data.pointer, wrq->u.data.length);
	
	if (Status)
	{
		Status = -EFAULT;
		goto ERROR1;
	}

	ATEMagicNum = OS_NTOHL(pRaCfg->magic_no);
	
	switch(ATEMagicNum)
	{
		case RACFG_MAGIC_NO:
			Status = RACfgCMDHandler(pAd, wrq, pRaCfg);
			break;

		default:
			Status = NDIS_STATUS_FAILURE;
			DBGPRINT_ERR(("Unknown magic number of RACFG command = %x\n", ATEMagicNum));
			break;
	}
	
 ERROR1:
	os_free_mem(NULL, pRaCfg);
 ERROR0:
	return Status;
}


VOID ATE_QA_Statistics(
	IN RTMP_ADAPTER *pAd,
	IN RXWI_STRUC *pRxWI,
	IN RXINFO_STRUC *pRxInfo,
	IN PHEADER_802_11 pHeader)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR rssi[3] = {0};
	UCHAR snr[3] = {0};

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		rssi[0] = pRxWI->RXWI_N.rssi[0];
		rssi[1] = pRxWI->RXWI_N.rssi[1];
		rssi[2] = pRxWI->RXWI_N.rssi[2];

		if ( IS_MT76x2(pAd) ) {
			snr[0] = pRxWI->RXWI_N.bbp_rxinfo[2];
			snr[1] = pRxWI->RXWI_N.bbp_rxinfo[3];
			snr[2] = pRxWI->RXWI_N.bbp_rxinfo[4];
		} else {
			snr[0] = pRxWI->RXWI_N.bbp_rxinfo[0];
			snr[1] = pRxWI->RXWI_N.bbp_rxinfo[1];
			snr[2] = pRxWI->RXWI_N.bbp_rxinfo[2];
		}
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		rssi[0] = pRxWI->RXWI_O.RSSI0;
		rssi[1] = pRxWI->RXWI_O.RSSI1;
		rssi[2] = pRxWI->RXWI_O.RSSI2;

		snr[0] = pRxWI->RXWI_O.SNR0;
		snr[1] = pRxWI->RXWI_O.SNR1;
		snr[2] = pRxWI->RXWI_O.SNR2;
	}
#endif /* RTMP_MAC */

	/* update counter first */
	if (pHeader != NULL)
	{
		if (pHeader->FC.Type == FC_TYPE_DATA)
		{
			if (pRxInfo->U2M)
			{
				pATEInfo->U2M++;
			}
			else
				pATEInfo->OtherData++;
		}
		else if (pHeader->FC.Type == FC_TYPE_MGMT)
		{
			if (pHeader->FC.SubType == SUBTYPE_BEACON)
				pATEInfo->Beacon++;
			else
				pATEInfo->OtherCount++;
		}
		else if (pHeader->FC.Type == FC_TYPE_CNTL)
		{
			pATEInfo->OtherCount++;
		}
	}
	pATEInfo->RSSI0 = rssi[0];
	pATEInfo->RSSI1 = rssi[1];
	pATEInfo->RSSI2 = rssi[2];
	pATEInfo->SNR0 = snr[0];
	pATEInfo->SNR1 = snr[1];


}


INT Set_TxStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	DBGPRINT(RT_DEBUG_TRACE,("Set_TxStop_Proc\n"));

	if (Set_ATE_Proc(pAd, "TXSTOP"))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


INT Set_RxStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	DBGPRINT(RT_DEBUG_TRACE,("Set_RxStop_Proc\n"));

	if (Set_ATE_Proc(pAd, "RXSTOP"))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


#ifdef DBG
INT Set_EERead_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT buffer[EEPROM_SIZE >> 1];
	USHORT *p;
	INT offset;
	
	rt_ee_read_all(pAd, (USHORT *)buffer);
	p = buffer;

	for (offset = 0; offset < (EEPROM_SIZE >> 1); offset++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", *p));
		if (((offset+1) % 16) == 0)
			DBGPRINT(RT_DEBUG_OFF, ("\n"));
		p++;
	}

	return TRUE;
}


INT Set_EEWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT offset = 0, value;
	PSTRING p2 = arg;
	
	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Hex(offset, arg);
		A2Hex(value, p2 + 1);
	}
	else
	{
		A2Hex(value, arg);
	}
	
	if (offset >= EEPROM_SIZE)
	{
		DBGPRINT_ERR(("Offset can not exceed EEPROM_SIZE( == 0x%04x)\n", EEPROM_SIZE));	
		return FALSE;
	}
	
	RT28xx_EEPROM_WRITE16(pAd, offset, value);

	return TRUE;
}


INT Set_BBPRead_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR value = 0, offset;

	A2Hex(offset, arg);	
			
	ATE_BBPRead(pAd, offset, &value);

	DBGPRINT(RT_DEBUG_OFF, ("%x\n", value));
		
	return TRUE;
}


INT Set_BBPWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT offset = 0;
	PSTRING p2 = arg;
	UCHAR value;
	
	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Hex(offset, arg);	
		A2Hex(value, p2 + 1);	
	}
	else
	{
		A2Hex(value, arg);	
	}

	ATE_BBPWrite(pAd, offset, value);

	return TRUE;
}

#ifdef RT_RF
INT Set_RFWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PSTRING p2, p3, p4;
	UINT32 R1, R2, R3, R4;
	
	p2 = arg;

	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 != ':')
		return FALSE;
	
	p3 = p2 + 1;

	while((*p3 != ':') && (*p3 != '\0'))
	{
		p3++;
	}

	if (*p3 != ':')
		return FALSE;
	
	p4 = p3 + 1;

	while ((*p4 != ':') && (*p4 != '\0'))
	{
		p4++;
	}

	if (*p4 != ':')
		return FALSE;

		
	A2Hex(R1, arg);	
	A2Hex(R2, p2 + 1);	
	A2Hex(R3, p3 + 1);	
	A2Hex(R4, p4 + 1);	
	
	RTMP_RF_IO_WRITE32(pAd, R1);
	RTMP_RF_IO_WRITE32(pAd, R2);
	RTMP_RF_IO_WRITE32(pAd, R3);
	RTMP_RF_IO_WRITE32(pAd, R4);
	
	return TRUE;
}
#endif /* RT_RF */
#endif /* DBG */
#endif /* RALINK_QA */

