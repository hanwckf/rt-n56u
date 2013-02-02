#include "rt_config.h"
#ifdef LINUX
#include "linux/time.h"
#endif // LINUX //


#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
#define ATE_BBP_REG_NUM	168
UCHAR restore_BBP[ATE_BBP_REG_NUM]={0};
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //

// 802.11 MAC Header, Type:Data, Length:24bytes
UCHAR TemplateFrame[24] = {0x08,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x00,0xAA,0xBB,0x12,0x34,0x56,0x00,0x11,0x22,0xAA,0xBB,0xCC,0x00,0x00};	
 
extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

extern FREQUENCY_ITEM FreqItems3020[];
extern UCHAR NUM_OF_3020_CHNL;




static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1}; /* CCK Mode. */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1}; /* OFDM Mode. */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1}; /* HT Mix Mode. */

extern TX_POWER_TUNING_ENTRY_STRUCT TxPowerTuningTable[];

#define MDSM_NORMAL_TX_POWER							0x00
#define MDSM_DROP_TX_POWER_BY_6dBm					0x01
#define MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define MDSM_ADD_TX_POWER_BY_6dBm						0x03
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK	0x03

static INT TxDmaBusy(
	IN PRTMP_ADAPTER pAd);

static INT RxDmaBusy(
	IN PRTMP_ADAPTER pAd);

static VOID RtmpDmaEnable(
	IN PRTMP_ADAPTER pAd,
	IN INT Enable);

static VOID BbpSoftReset(
	IN PRTMP_ADAPTER pAd);

static VOID RtmpRfIoWrite(
	IN PRTMP_ADAPTER pAd);

static INT ATESetUpFrame(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx);

static INT ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);

static NDIS_STATUS ATESTART(
	IN PRTMP_ADAPTER pAd);

static NDIS_STATUS ATESTOP(
	IN PRTMP_ADAPTER pAd);

static NDIS_STATUS TXCARR(
	IN PRTMP_ADAPTER pAd);

static NDIS_STATUS TXCONT(
	IN PRTMP_ADAPTER pAd);

static NDIS_STATUS TXFRAME(
	IN PRTMP_ADAPTER pAd);

static NDIS_STATUS RXFRAME(
	IN PRTMP_ADAPTER pAd);

#ifdef RALINK_QA
static NDIS_STATUS TXSTOP(
	IN PRTMP_ADAPTER pAd);

static NDIS_STATUS RXSTOP(
	IN PRTMP_ADAPTER pAd);
#endif // RALINK_QA //

static NDIS_STATUS ATECmdHandler(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

static int CheckMCSValid(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR Mode,
	IN UCHAR Mcs);

#ifdef RTMP_MAC_PCI
static VOID ATEWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC 	pOutTxWI,	
	IN	BOOLEAN			FRAG,	
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN 		AMPDU,
	IN	BOOLEAN 		Ack,
	IN	BOOLEAN 		NSeq,		// HW new a sequence.
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR 			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,	
	IN	BOOLEAN			CfAck,	
	IN	HTTRANSMIT_SETTING	*pTransmit);
#endif // RTMP_MAC_PCI //


static VOID SetJapanFilter(
	IN	PRTMP_ADAPTER	pAd);


#define	LEN_OF_ARG 16


#ifdef RALINK_QA

static VOID memcpy_exl(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len);
static VOID memcpy_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len);
static VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, UINT32 len);

static INT ResponseToGUI(
	IN  struct ate_racfghdr *pRaCfg,
	IN	struct iwreq	*pwrq,
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
		DBGPRINT(RT_DEBUG_ERROR, ("copy_to_user() fail in %s\n", __FUNCTION__));
		return (-EFAULT);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s is done !\n", __FUNCTION__));
	}

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START\n"));
	
	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
	
#ifdef	CONFIG_RT2880_ATE_CMD_NEW
	Set_ATE_Proc(pAdapter, "ATESTART");
#else
	Set_ATE_Proc(pAdapter, "APSTOP");
#endif // CONFIG_RT2880_ATE_CMD_NEW //

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	INT32 ret;

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_STOP\n"));

	/*
		Distinguish this command came from QA(via ate agent)
		or ate agent according to the existence of pid in payload.

		No need to prepare feedback if this cmd came directly from ate agent,
		not from QA.
	*/
	pRaCfg->length = OS_NTOHS(pRaCfg->length);

	if (pRaCfg->length == sizeof(pAdapter->ate.AtePid))
	{
		/*
			This command came from QA.
			Get the pid of ATE agent.
		*/
		memcpy((UCHAR *)&pAdapter->ate.AtePid,
						(&pRaCfg->data[0]) - 2/* == sizeof(pRaCfg->status) */,
						sizeof(pAdapter->ate.AtePid));					

		/* Prepare feedback as soon as we can to avoid QA timeout. */
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

		/*
			Kill ATE agent when leaving ATE mode.

			We must kill ATE agent first before setting ATESTOP,
			or Microsoft will report sth. wrong. 
		*/
		ret = KILL_THREAD_PID(pAdapter->ate.AtePid, SIGTERM, 1);

		if (ret)
			DBGPRINT(RT_DEBUG_ERROR, ("%s: unable to kill ate thread\n", 
				RTMP_OS_NETDEV_GET_DEVNAME(pAdapter->net_dev)));
	}
		

	/* AP/STA might have in ATE_STOP mode due to cmd from QA. */
	if (ATE_ON(pAdapter))
	{
		/* Someone has killed ate agent while QA GUI is still open. */

#ifdef	CONFIG_RT2880_ATE_CMD_NEW
		Set_ATE_Proc(pAdapter, "ATESTOP");
#else
		Set_ATE_Proc(pAdapter, "APSTART");
#endif
		DBGPRINT(RT_DEBUG_TRACE, ("RACFG_CMD_AP_START is done !\n"));
	}
	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_RF_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 R1, R2, R3, R4;
	USHORT channel;
	
	memcpy(&R1, pRaCfg->data-2, 4);
	memcpy(&R2, pRaCfg->data+2, 4);
	memcpy(&R3, pRaCfg->data+6, 4);
	memcpy(&R4, pRaCfg->data+10, 4);
	memcpy(&channel, pRaCfg->data+14, 2);		
	
	pAdapter->LatchRfRegs.R1 = OS_NTOHL(R1);
	pAdapter->LatchRfRegs.R2 = OS_NTOHL(R2);
	pAdapter->LatchRfRegs.R3 = OS_NTOHL(R3);
	pAdapter->LatchRfRegs.R4 = OS_NTOHL(R4);
	pAdapter->LatchRfRegs.Channel = OS_NTOHS(channel);

	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R3);
	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return  NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_READ16(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT16	offset=0, value=0;
	USHORT  tmp=0;				

	offset = OS_NTOHS(pRaCfg->status);

	/* "tmp" is especially for some compilers... */
	RT28xx_EEPROM_READ16(pAdapter, offset, tmp);
	value = tmp;
	value = OS_HTONS(value);
	
	DBGPRINT(RT_DEBUG_TRACE,("EEPROM Read offset = 0x%04x, value = 0x%04x\n", offset, value));
	memcpy(pRaCfg->data, &value, 2);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+2, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_WRITE16(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset, value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 2);
	value = OS_NTOHS(value);
	RT28xx_EEPROM_WRITE16(pAdapter, offset, value);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_READ_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE/2];

	rt_ee_read_all(pAdapter,(USHORT *)buffer);
	memcpy_exs(pAdapter, pRaCfg->data, (UCHAR *)buffer, EEPROM_SIZE);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+EEPROM_SIZE, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_E2PROM_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE/2];

	NdisZeroMemory((UCHAR *)buffer, EEPROM_SIZE);
	memcpy_exs(pAdapter, (UCHAR *)buffer, (UCHAR *)&pRaCfg->status, EEPROM_SIZE);
	rt_ee_write_all(pAdapter,(USHORT *)buffer);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_READ(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	UINT32	value;
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);

	/*
		We do not need the base address.
		So just extract the offset out.
	*/
	{
		offset &= 0x0000FFFF;
		RTMP_IO_READ32(pAdapter, offset, &value);
	}
	value = OS_HTONL(value);
	memcpy(pRaCfg->data, &value, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_WRITE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset, value;
					
	memcpy(&offset, pRaCfg->data-2, 4);
	memcpy(&value, pRaCfg->data+2, 4);

	offset = OS_NTOHL(offset);

	/*
		We do not need the base address.
		So just extract the offset out.
	*/
	offset &= 0x0000FFFF;
	value = OS_NTOHL(value);
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_IO_WRITE: offset = %x, value = %x\n", offset, value));
	RTMP_IO_WRITE32(pAdapter, offset, value);
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_IO_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	USHORT	len;
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);

	/*
		We do not need the base address.
		So just extract the offset out.
	*/
	offset &= 0x0000FFFF;
	memcpy(&len, pRaCfg->data+2, 2);
	len = OS_NTOHS(len);

	if (len > 371)
	{
		DBGPRINT(RT_DEBUG_TRACE,("length requested is too large, make it smaller\n"));
		pRaCfg->length = OS_HTONS(2);
		pRaCfg->status = OS_HTONS(1);

		return -EFAULT;
	}

	RTMP_IO_READ_BULK(pAdapter, pRaCfg->data, (UCHAR *)offset, len*4);// unit in four bytes

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(len*4), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_READ8(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;
	
	value = 0;
	offset = OS_NTOHS(pRaCfg->status);

	if (ATE_ON(pAdapter))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, offset, &value);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, offset, &value);	
	}

	pRaCfg->data[0] = value;
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+1, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_WRITE8(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 1);

	if (ATE_ON(pAdapter))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAdapter, offset, value);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, offset, value);
	}

	if ((offset == BBP_R1) || (offset == BBP_R3))
	{
		SyncTxRxConfig(pAdapter, offset, value);
	}
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_BBP_READ_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT bbp_reg_index;
	
	for (bbp_reg_index = 0; bbp_reg_index < MAX_BBP_ID+1; bbp_reg_index++)
	{
		pRaCfg->data[bbp_reg_index] = 0;
		
		if (ATE_ON(pAdapter))
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, bbp_reg_index, &pRaCfg->data[bbp_reg_index]);
		}
		else
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, bbp_reg_index, &pRaCfg->data[bbp_reg_index]);
		}
	}
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+MAX_BBP_ID+1, NDIS_STATUS_SUCCESS);
	
	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_GET_NOISE_LEVEL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UCHAR	channel;
	INT32   buffer[3][10];/* 3 : RxPath ; 10 : no. of per rssi samples */

	channel = (OS_NTOHS(pRaCfg->status) & 0x00FF);
	CalNoiseLevel(pAdapter, channel, buffer);
	memcpy_exl(pAdapter, (UCHAR *)pRaCfg->data, (UCHAR *)&(buffer[0][0]), (sizeof(INT32)*3*10));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(sizeof(INT32)*3*10), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}



static  INT DO_RACFG_CMD_GET_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	memcpy_exl(pAdapter, &pRaCfg->data[0], (UCHAR *)&pAdapter->ate.U2M, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[4], (UCHAR *)&pAdapter->ate.OtherData, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[8], (UCHAR *)&pAdapter->ate.Beacon, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[12], (UCHAR *)&pAdapter->ate.OtherCount, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[16], (UCHAR *)&pAdapter->ate.TxAc0, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[20], (UCHAR *)&pAdapter->ate.TxAc1, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[24], (UCHAR *)&pAdapter->ate.TxAc2, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[28], (UCHAR *)&pAdapter->ate.TxAc3, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[32], (UCHAR *)&pAdapter->ate.TxHCCA, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[36], (UCHAR *)&pAdapter->ate.TxMgmt, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[40], (UCHAR *)&pAdapter->ate.RSSI0, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[44], (UCHAR *)&pAdapter->ate.RSSI1, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[48], (UCHAR *)&pAdapter->ate.RSSI2, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[52], (UCHAR *)&pAdapter->ate.SNR0, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[56], (UCHAR *)&pAdapter->ate.SNR1, 4);
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+60, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_CLEAR_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	pAdapter->ate.U2M = 0;
	pAdapter->ate.OtherData = 0;
	pAdapter->ate.Beacon = 0;
	pAdapter->ate.OtherCount = 0;
	pAdapter->ate.TxAc0 = 0;
	pAdapter->ate.TxAc1 = 0;
	pAdapter->ate.TxAc2 = 0;
	pAdapter->ate.TxAc3 = 0;
	pAdapter->ate.TxHCCA = 0;
	pAdapter->ate.TxMgmt = 0;
	pAdapter->ate.TxDoneCount = 0;
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_TX_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT *p;
	USHORT	err = 1;
	UCHAR   Bbp22Value = 0, Bbp24Value = 0;

	if ((pAdapter->ate.TxStatus != 0) && (pAdapter->ate.Mode & ATE_TXFRAME))
	{
		DBGPRINT(RT_DEBUG_TRACE,("Ate Tx is already running, to run next Tx, you must stop it first\n"));
		err = 2;
		goto tx_start_error;
	}
	else if ((pAdapter->ate.TxStatus != 0) && !(pAdapter->ate.Mode & ATE_TXFRAME))
	{
		int i = 0;

		while ((i++ < 10) && (pAdapter->ate.TxStatus != 0))
		{
			RTMPusecDelay(5000);
		}

		/* force it to stop */
		pAdapter->ate.TxStatus = 0;
		pAdapter->ate.TxDoneCount = 0;
		pAdapter->ate.bQATxStart = FALSE;
	}

#ifdef NEW_TXCONT
	/* Reset ATE mode and set Tx/Rx Idle */
	/* New proposed TXCONT solution. */
	if (pAdapter->ate.Mode |= ATE_TXFRAME)
	{
		TXSTOP(pAdapter);
	}
#endif // NEW_TXCONT //

	/*
		If pRaCfg->length == 0, this "RACFG_CMD_TX_START"
		is for Carrier test or Carrier Suppression.
	*/
	if (OS_NTOHS(pRaCfg->length) != 0)
	{
		/* get frame info */

		NdisMoveMemory(&pAdapter->ate.TxWI, pRaCfg->data + 2, 16);						
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange((PUCHAR)&pAdapter->ate.TxWI, TYPE_TXWI);
#endif // RT_BIG_ENDIAN //

		NdisMoveMemory(&pAdapter->ate.TxCount, pRaCfg->data + 18, 4);
		pAdapter->ate.TxCount = OS_NTOHL(pAdapter->ate.TxCount);

		p = (USHORT *)(&pRaCfg->data[22]);

		/* always use QID_AC_BE */
		pAdapter->ate.QID = 0;

		p = (USHORT *)(&pRaCfg->data[24]);
		pAdapter->ate.HLen = OS_NTOHS(*p);

		if (pAdapter->ate.HLen > 32)
		{
			DBGPRINT(RT_DEBUG_ERROR,("pAdapter->ate.HLen > 32\n"));
			err = 3;
			goto tx_start_error;
		}

		NdisMoveMemory(&pAdapter->ate.Header, pRaCfg->data + 26, pAdapter->ate.HLen);

		pAdapter->ate.PLen = OS_NTOHS(pRaCfg->length) - (pAdapter->ate.HLen + 28);

		if (pAdapter->ate.PLen > 32)
		{
			DBGPRINT(RT_DEBUG_ERROR,("pAdapter->ate.PLen > 32\n"));
			err = 4;
			goto tx_start_error;
		}

		NdisMoveMemory(&pAdapter->ate.Pattern, pRaCfg->data + 26 + pAdapter->ate.HLen, pAdapter->ate.PLen);
		pAdapter->ate.DLen = pAdapter->ate.TxWI.MPDUtotalByteCount - pAdapter->ate.HLen;


	}

	ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R22, &Bbp22Value);

	switch (Bbp22Value)
	{
		case BBP22_TXFRAME:
			{
				if (pAdapter->ate.TxCount == 0)
				{
#ifdef RTMP_MAC_PCI
					pAdapter->ate.TxCount = 0xFFFFFFFF;
#endif // RTMP_MAC_PCI //
				}
				DBGPRINT(RT_DEBUG_TRACE,("START TXFRAME\n"));
				pAdapter->ate.bQATxStart = TRUE;
				Set_ATE_Proc(pAdapter, "TXFRAME");
			}
			break;

		case BBP22_TXCONT_OR_CARRSUPP:
			{
				DBGPRINT(RT_DEBUG_TRACE,("BBP22_TXCONT_OR_CARRSUPP\n"));
				ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R24, &Bbp24Value);

				switch (Bbp24Value)
				{
					case BBP24_TXCONT:
						{
							DBGPRINT(RT_DEBUG_TRACE,("START TXCONT\n"));
							pAdapter->ate.bQATxStart = TRUE;
#ifdef NEW_TXCONT
							/* New proposed solution. */
							/* Let QA handle all hardware manipulation. */ 
							/* Driver only needs to send a few training frames. */
							DBGPRINT(RT_DEBUG_OFF,("ATE ERROR! It should not reach here!\n"));
							Set_ATE_Proc(pAdapter, "TXFRAME");
#else
							Set_ATE_Proc(pAdapter, "TXCONT");
#endif // NEW_TXCONT //
						}
						break;

					case BBP24_CARRSUPP:
						{
							DBGPRINT(RT_DEBUG_TRACE,("START TXCARRSUPP\n"));
							pAdapter->ate.bQATxStart = TRUE;
							pAdapter->ate.Mode |= ATE_TXCARRSUPP;
						}
						break;

					default:
						{
							DBGPRINT(RT_DEBUG_ERROR,("Unknown TX subtype !"));
						}
						break;
				}
			}
			break;	

		case BBP22_TXCARR:
			{
				DBGPRINT(RT_DEBUG_TRACE,("START TXCARR\n"));
				pAdapter->ate.bQATxStart = TRUE;
				Set_ATE_Proc(pAdapter, "TXCARR");
			}
			break;							

		default:
			{
				DBGPRINT(RT_DEBUG_ERROR,("Unknown Start TX subtype !"));
			}
			break;
	}

	if (pAdapter->ate.bQATxStart == TRUE)
	{
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
		return NDIS_STATUS_SUCCESS;
	}

tx_start_error:
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), err);

	return err;
}


static  INT DO_RACFG_CMD_GET_TX_STATUS(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 count=0;
	
	count = OS_HTONL(pAdapter->ate.TxDoneCount);
	NdisMoveMemory(pRaCfg->data, &count, 4);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_TX_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_TX_STOP\n"));

	Set_ATE_Proc(pAdapter, "TXSTOP");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_RX_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	pAdapter->ate.bQARxStart = TRUE;
	Set_ATE_Proc(pAdapter, "RXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	


static  INT DO_RACFG_CMD_RX_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_STOP\n"));

	Set_ATE_Proc(pAdapter, "RXSTOP");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_CARRIER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CARRIER\n"));

	Set_ATE_Proc(pAdapter, "TXCARR");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_CONT(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CONT\n"));

	Set_ATE_Proc(pAdapter, "TXCONT");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_TX_FRAME(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_FRAME\n"));

	Set_ATE_Proc(pAdapter, "TXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}	


static  INT DO_RACFG_CMD_ATE_SET_BW(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_BW\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);

	Set_ATE_TX_BW_Proc(pAdapter, str);
	
	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_POWER0(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER0\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_POWER0_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_POWER1(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER1\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_POWER1_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}




static  INT DO_RACFG_CMD_ATE_SET_FREQ_OFFSET(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_FREQ_OFFSET\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_FREQOFFSET_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_GET_STATISTICS(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_GET_STATISTICS\n"));

	memcpy_exl(pAdapter, &pRaCfg->data[0], (UCHAR *)&pAdapter->ate.TxDoneCount, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[4], (UCHAR *)&pAdapter->WlanCounters.RetryCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[8], (UCHAR *)&pAdapter->WlanCounters.FailedCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[12], (UCHAR *)&pAdapter->WlanCounters.RTSSuccessCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[16], (UCHAR *)&pAdapter->WlanCounters.RTSFailureCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[20], (UCHAR *)&pAdapter->WlanCounters.ReceivedFragmentCount.QuadPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[24], (UCHAR *)&pAdapter->WlanCounters.FCSErrorCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[28], (UCHAR *)&pAdapter->Counters8023.RxNoBuffer, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[32], (UCHAR *)&pAdapter->WlanCounters.FrameDuplicateCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[36], (UCHAR *)&pAdapter->RalinkCounters.OneSecFalseCCACnt, 4);
	
	if (pAdapter->ate.RxAntennaSel == 0)
	{
		INT32 RSSI0 = 0;
		INT32 RSSI1 = 0;
		INT32 RSSI2 = 0;

		RSSI0 = (INT32)(pAdapter->ate.LastRssi0 - pAdapter->BbpRssiToDbmDelta);
		RSSI1 = (INT32)(pAdapter->ate.LastRssi1 - pAdapter->BbpRssiToDbmDelta);
		RSSI2 = (INT32)(pAdapter->ate.LastRssi2 - pAdapter->BbpRssiToDbmDelta);
		memcpy_exl(pAdapter, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		memcpy_exl(pAdapter, &pRaCfg->data[44], (UCHAR *)&RSSI1, 4);
		memcpy_exl(pAdapter, &pRaCfg->data[48], (UCHAR *)&RSSI2, 4);
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+52, NDIS_STATUS_SUCCESS);
	}
	else
	{
		INT32 RSSI0 = 0;
	
		RSSI0 = (INT32)(pAdapter->ate.LastRssi0 - pAdapter->BbpRssiToDbmDelta);
		memcpy_exl(pAdapter, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+44, NDIS_STATUS_SUCCESS);
	}

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_RESET_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 1;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_RESET_COUNTER\n"));				

	sprintf((char *)str, "%d", value);
	Set_ResetStatCounter_Proc(pAdapter, str);

	pAdapter->ate.TxDoneCount = 0;

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SEL_TX_ANTENNA(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)	
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_TX_ANTENNA\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_Antenna_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SEL_RX_ANTENNA(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_RX_ANTENNA\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_RX_Antenna_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_PREAMBLE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_PREAMBLE\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_MODE_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_CHANNEL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_CHANNEL\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_CHANNEL_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR1(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR1\n"));

	/*
		Addr is an array of UCHAR,
		so no need to perform endian swap.
	*/
	memcpy(pAdapter->ate.Addr1, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR2(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR2\n"));

	/*
		Addr is an array of UCHAR,
		so no need to perform endian swap.
	*/
	memcpy(pAdapter->ate.Addr2, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_ADDR3(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR3\n"));

	/*
		Addr is an array of UCHAR,
		so no need to perform endian swap.
	*/
	memcpy(pAdapter->ate.Addr3, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_RATE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_RATE\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_MCS_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);
	
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_FRAME_LEN\n"));				

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = OS_NTOHS(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_LENGTH_Proc(pAdapter, str);

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
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
		pAdapter->ate.TxCount = 0xFFFFFFFF;
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pAdapter->ate.TxCount));
		DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_COUNT_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

	}
	else
#endif // RTMP_MAC_PCI //
	{
		sprintf((char *)str, "%d", value);
		Set_ATE_TX_COUNT_Proc(pAdapter, str);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_START_RX_FRAME(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	DBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	Set_ATE_Proc(pAdapter, "RXFRAME");

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_E2PROM_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE/2];
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
	
	rt_ee_read_all(pAdapter, (USHORT *)buffer);

	if (offset + len <= EEPROM_SIZE)
		memcpy_exs(pAdapter, pRaCfg->data, (UCHAR *)buffer+offset, len);
	else
		DBGPRINT(RT_DEBUG_ERROR, ("exceed EEPROM size\n"));

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE/2];
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	memcpy_exs(pAdapter, (UCHAR *)buffer + offset, (UCHAR *)pRaCfg->data + 2, len);
	
	if ((offset + len) <= EEPROM_SIZE)
	{
		rt_ee_write_bulk(pAdapter,(USHORT *)(((UCHAR *)buffer) + offset), offset, len);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("exceed EEPROM size(%d)\n", EEPROM_SIZE));
		DBGPRINT(RT_DEBUG_ERROR, ("offset=%d\n", offset));
		DBGPRINT(RT_DEBUG_ERROR, ("length=%d\n", len));
		DBGPRINT(RT_DEBUG_ERROR, ("offset+length=%d\n", (offset+len)));
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_IO_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 offset, i, value;
	USHORT len;
	
	memcpy(&offset, &pRaCfg->status, 4);
	offset = OS_NTOHL(offset);
	memcpy(&len, pRaCfg->data+2, 2);
	len = OS_NTOHS(len);
	
	for (i = 0; i < len; i += 4)
	{
		memcpy_exl(pAdapter, (UCHAR *)&value, pRaCfg->data+4+i, 4);
		DBGPRINT(RT_DEBUG_TRACE,("Write %x %x\n", offset + i, value));
		RTMP_IO_WRITE32(pAdapter, ((offset+i) & (0xffff)), value);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_BBP_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
	
	for (j = offset; j < (offset+len); j++)
	{
		pRaCfg->data[j - offset] = 0;
		
		if (pAdapter->ate.Mode == ATE_STOP)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, j, &pRaCfg->data[j - offset]);
		}
		else
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, j, &pRaCfg->data[j - offset]);
		}
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_BBP_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	UCHAR *value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);
					
	for (j = offset; j < (offset+len); j++)
	{
		value = pRaCfg->data + 2 + (j - offset);
		if (pAdapter->ate.Mode == ATE_STOP)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, j,  *value);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAdapter, j,  *value);
		}
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}




#ifdef RTMP_RF_RW_SUPPORT
static  INT DO_RACFG_CMD_ATE_RF_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	for (j = offset; j < (offset+len); j++)
	{
		pRaCfg->data[j - offset] = 0;
		ATE_RF_IO_READ8_BY_REG_ID(pAdapter, j,  &pRaCfg->data[j - offset]);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static  INT DO_RACFG_CMD_ATE_RF_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	UCHAR *value;
	
	offset = OS_NTOHS(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = OS_NTOHS(len);

	for (j = offset; j < (offset+len); j++)
	{
		value = pRaCfg->data + 2 + (j - offset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAdapter, j,  *value);
	}

	ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}
#endif // RTMP_RF_RW_SUPPORT //

typedef INT (*RACFG_CMD_HANDLER)(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg);


RACFG_CMD_HANDLER RACFG_CMD_SET1[] =
{
	/* cmd id start from 0x0 */
	DO_RACFG_CMD_RF_WRITE_ALL,	
	DO_RACFG_CMD_E2PROM_READ16,
	DO_RACFG_CMD_E2PROM_WRITE16,
	DO_RACFG_CMD_E2PROM_READ_ALL,
	DO_RACFG_CMD_E2PROM_WRITE_ALL,
	DO_RACFG_CMD_IO_READ,
	DO_RACFG_CMD_IO_WRITE,
	DO_RACFG_CMD_IO_READ_BULK,
	DO_RACFG_CMD_BBP_READ8,
	DO_RACFG_CMD_BBP_WRITE8,
	DO_RACFG_CMD_BBP_READ_ALL,
	DO_RACFG_CMD_GET_COUNTER,
	DO_RACFG_CMD_CLEAR_COUNTER,
	NULL /* RACFG_CMD_RSV1 */,
	NULL /* RACFG_CMD_RSV2 */,
	NULL /* RACFG_CMD_RSV3 */,
	DO_RACFG_CMD_TX_START,
	DO_RACFG_CMD_GET_TX_STATUS,
	DO_RACFG_CMD_TX_STOP,
	DO_RACFG_CMD_RX_START,
	DO_RACFG_CMD_RX_STOP,
	DO_RACFG_CMD_GET_NOISE_LEVEL,
	NULL

	/* cmd id end with 0x1e */
};


RACFG_CMD_HANDLER RACFG_CMD_SET2[] =
{
	/* cmd id start from 0x80 */
	DO_RACFG_CMD_ATE_START,
	DO_RACFG_CMD_ATE_STOP
	/* cmd id end with 0x81 */
};


RACFG_CMD_HANDLER RACFG_CMD_SET3[] =
{
	/* cmd id start from 0x100 */
	DO_RACFG_CMD_ATE_START_TX_CARRIER,
	DO_RACFG_CMD_ATE_START_TX_CONT,
	DO_RACFG_CMD_ATE_START_TX_FRAME,
	DO_RACFG_CMD_ATE_SET_BW,
	DO_RACFG_CMD_ATE_SET_TX_POWER0,
	DO_RACFG_CMD_ATE_SET_TX_POWER1,
	DO_RACFG_CMD_ATE_SET_FREQ_OFFSET,
	DO_RACFG_CMD_ATE_GET_STATISTICS,
	DO_RACFG_CMD_ATE_RESET_COUNTER,
	DO_RACFG_CMD_ATE_SEL_TX_ANTENNA,
	DO_RACFG_CMD_ATE_SEL_RX_ANTENNA,
	DO_RACFG_CMD_ATE_SET_PREAMBLE,
	DO_RACFG_CMD_ATE_SET_CHANNEL,
	DO_RACFG_CMD_ATE_SET_ADDR1,
	DO_RACFG_CMD_ATE_SET_ADDR2,
	DO_RACFG_CMD_ATE_SET_ADDR3,
	DO_RACFG_CMD_ATE_SET_RATE,
	DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN,
	DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT,
	DO_RACFG_CMD_ATE_START_RX_FRAME,
	DO_RACFG_CMD_ATE_E2PROM_READ_BULK,
	DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK,
	DO_RACFG_CMD_ATE_IO_WRITE_BULK,
	DO_RACFG_CMD_ATE_BBP_READ_BULK,
	DO_RACFG_CMD_ATE_BBP_WRITE_BULK,
#ifdef RTMP_RF_RW_SUPPORT
	DO_RACFG_CMD_ATE_RF_READ_BULK,
	DO_RACFG_CMD_ATE_RF_WRITE_BULK,
#else
	NULL,
	NULL,
#endif // RTMP_RF_RW_SUPPORT //
	NULL
	/* cmd id end with 0x11b */
};


RACFG_CMD_HANDLER RACFG_CMD_SET4[] =
{
	/* cmd id start from 0x200 */
	/* cmd id end with 0x200 */
};


/*
#define MAX_RACFG_TABLE_NUMBER 4
UINT16 RACFG_CMD_INDEX_RANGE_LIMIT[] = { 0x0000, 0x0080, 0x0100, 0x0200, 0xffff };
RACFG_CMD_HANDLER *RACFG_CMD_TABLE[] = { RACFG_CMD_TABLE1, RACFG_CMD_TABLE2, RACFG_CMD_TABLE3, RACFG_CMD_TABLE4 };
*/


typedef struct _RACFG_CMD_TABLE_{
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
};
#endif // RALINK_QA //


#ifdef RTMP_MAC_PCI
static INT TxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	// disable DMA
	if (GloCfg.field.TxDMABusy)
		result = 1;
	else
		result = 0;

	return result;
}


static INT RxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	// disable DMA
	if (GloCfg.field.RxDMABusy)
		result = 1;
	else
		result = 0;

	return result;
}


static VOID RtmpDmaEnable(
	IN PRTMP_ADAPTER pAd,
	IN INT Enable)
{
	BOOLEAN value;
	ULONG WaitCnt;
	WPDMA_GLO_CFG_STRUC GloCfg;
	
	value = Enable > 0 ? 1 : 0;

	// check if DMA is in busy mode or not.
	WaitCnt = 0;

	while (TxDmaBusy(pAd) || RxDmaBusy(pAd))
	{
		RTMPusecDelay(10);
		if (WaitCnt++ > 100)
			break;
	}
	
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	// disable DMA
	GloCfg.field.EnableTxDMA = value;
	GloCfg.field.EnableRxDMA = value;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);	// abort all TX rings
	RTMPusecDelay(5000);

	return;
}
#endif // RTMP_MAC_PCI //




static VOID BbpSoftReset(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BbpData = 0;

	// Soft reset, set BBP R21 bit0=1->0
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpData);
	BbpData |= 0x00000001; //set bit0=1
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpData);
	BbpData &= ~(0x00000001); //set bit0=0
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);

	return;
}

VOID rt_ee_write_bulk(PRTMP_ADAPTER pAd, 
	IN USHORT *Data, 
	IN USHORT offset, 
	IN USHORT length)
{
	USHORT pos;
	USHORT value;
	USHORT len = length;

	for (pos = 0; pos < (len >> 1);)
	{
		/* "value" is especially for some compilers... */
		value = Data[pos];
		RT28xx_EEPROM_WRITE16(pAd, offset+(pos*2), value);
		pos++;
	}
}

static VOID RtmpRfIoWrite(
	IN PRTMP_ADAPTER pAd)
{
	// Set RF value 1's set R3[bit2] = [0]
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RTMPusecDelay(200);

	// Set RF value 2's set R3[bit2] = [1]
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 | 0x04));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RTMPusecDelay(200);

	// Set RF value 3's set R3[bit2] = [0]
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	return;
}


static int CheckMCSValid(
	IN PRTMP_ADAPTER	pAd, 
	IN UCHAR Mode,
	IN UCHAR Mcs)
{
	int i;
	PCHAR pRateTab = NULL;

	switch (Mode)
	{
		case MODE_CCK:
			pRateTab = CCKRateTable;
			break;
		case MODE_OFDM:
			pRateTab = OFDMRateTable;
			break;
			
		case 2: /*MODE_HTMIX*/
		case 3: /*MODE_HTGREENFIELD*/
				pRateTab = HTMIXRateTable;
			break;
			
		default: 
			DBGPRINT(RT_DEBUG_ERROR, ("unrecognizable Tx Mode %d\n", Mode));
			return -1;
			break;
	}

	i = 0;
	while (pRateTab[i] != -1)
	{
		if (pRateTab[i] == Mcs)
			return 0;
		i++;
	}

	return -1;
}


static INT ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	ULONG R;
	CHAR TxPower = 0;
	UCHAR Bbp94 = 0;
	BOOLEAN bPowerReduce = FALSE;
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR RFValue = 0;
	UCHAR		BbpR1 = 0;
#endif // RTMP_RF_RW_SUPPORT //
#ifdef RT33xx
	CHAR		TotalDeltaPower = 0; // (non-positive number) including the transmit power controlled by the MAC and the BBP R1
	/*CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};*/
	INT			i,j;
	CHAR		Value;
	ULONG		TxPwr[5];
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
#endif // RT33xx //

#ifdef RALINK_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		/* 
			When QA is used for Tx, pAd->ate.TxPower0/1 and real tx power
			are not synchronized.
		*/
		return 0;
	}
	else
#endif // RALINK_QA //

	if (index == 0)
	{
		TxPower = pAd->ate.TxPower0;
	}
	else if (index == 1)
	{
		TxPower = pAd->ate.TxPower1;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Only TxPower0 and TxPower1 are adjustable !\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("TxPower%d is out of range !\n", index));
	}

#ifdef RTMP_RF_RW_SUPPORT
		if (IS_RT30xx(pAd) || IS_RT3390(pAd))
		{
			if (IS_RT3390(pAd))
			{
				pAd->TxPowerCtrl.idxTxPowerTable = TxPower;

				//
				// Valide pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 45
				//

				pTxPowerTuningEntry = &TxPowerTuningTable[pAd->TxPowerCtrl.idxTxPowerTable + TX_POWER_TUNING_ENTRY_OFFSET]; // zero-based array
				pAd->TxPowerCtrl.RF_TX0_ALC = pTxPowerTuningEntry->RF_TX0_ALC;
				pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

				//
				// Tx power adjustment over RF
				//
				RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)(&RFValue));
				RFValue = ((RFValue & 0xE0) | pAd->TxPowerCtrl.RF_TX0_ALC);
				RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)(RFValue));

				//
				// Tx power adjustment over MAC
				//
				TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;
				
				//AsicGetTxPowerOffset(pAd, (PULONG) &CfgOfTxPwrCtrlOverMAC);
				
				if (pAd->ate.TxWI.BW == BW_40)
				{		
					TxPwr[0] = pAd->Tx40MPwrCfgGBand[0];
					TxPwr[1] = pAd->Tx40MPwrCfgGBand[1];
					TxPwr[2] = pAd->Tx40MPwrCfgGBand[2];
					TxPwr[3] = pAd->Tx40MPwrCfgGBand[3];
					TxPwr[4] = pAd->Tx40MPwrCfgGBand[4];
				}
				else
				{		
					TxPwr[0] = pAd->Tx20MPwrCfgGBand[0];
					TxPwr[1] = pAd->Tx20MPwrCfgGBand[1];
					TxPwr[2] = pAd->Tx20MPwrCfgGBand[2];
					TxPwr[3] = pAd->Tx20MPwrCfgGBand[3];
					TxPwr[4] = pAd->Tx20MPwrCfgGBand[4];		
				}
				
				// The BBP R1 controls the transmit power for all rates
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
				BbpR1 &= ~MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;

				if (TotalDeltaPower <= -12)
				{
					TotalDeltaPower += 12;
					BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;

					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

					DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: Drop the transmit power by 12 dBm (BBP R1)\n", __FUNCTION__));
				}
				else if ((TotalDeltaPower <= -6) && (TotalDeltaPower > -12))
				{
					TotalDeltaPower += 6;
					BbpR1 |= MDSM_DROP_TX_POWER_BY_6dBm;		

					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

					DBGPRINT(RT_DEBUG_INFO, ("TPC: %s: Drop the transmit power by 6 dBm (BBP R1)\n", __FUNCTION__));
				}
				else
				{
					// Control the the transmit power by using the MAC only
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);
				}
				
				for (i=0; i<5; i++)
				{
					if (TxPwr[i] != 0xffffffff)
					{
						for (j=0; j<8; j++)
						{
							Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F);							

							// The upper bounds of the MAC 0x1314~0x1324 are variable when the STA uses the internal Tx ALC.
							switch (TX_PWR_CFG_0 + (i * 4))
							{
								case TX_PWR_CFG_0: 
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xE)
									{
										Value = 0xE;
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
								break;

								case TX_PWR_CFG_1: 
								{
									if ((j >= 0) && (j <= 3))
									{
										if ((Value + TotalDeltaPower) < 0)
										{
											Value = 0;
										}
										else if ((Value + TotalDeltaPower) > 0xC)
										{
											Value = 0xC;
										}
										else
										{
											Value += TotalDeltaPower;
										}
									}
									else
									{
										if ((Value + TotalDeltaPower) < 0)
										{
											Value = 0;
										}
										else if ((Value + TotalDeltaPower) > 0xE)
										{
											Value = 0xE;
										}
										else
										{
											Value += TotalDeltaPower;
										}
									}
								}
								break;

								case TX_PWR_CFG_2: 
								{
									if ((j == 0) || (j == 2) || (j == 3))
									{
										if ((Value + TotalDeltaPower) < 0)
										{
											Value = 0;
										}
										else if ((Value + TotalDeltaPower) > 0xC)
										{
											Value = 0xC;
										}
										else
										{
											Value += TotalDeltaPower;
										}
									}
									else
									{
										if ((Value + TotalDeltaPower) < 0)
										{
											Value = 0;
										}
										else if ((Value + TotalDeltaPower) > 0xE)
										{
											Value = 0xE;
										}
										else
										{
											Value += TotalDeltaPower;
										}
									}
								}
								break;

								case TX_PWR_CFG_3: 
								{
									if ((j == 0) || (j == 2) || (j == 3) || 
									    ((j >= 4) && (j <= 7)))
									{
										if ((Value + TotalDeltaPower) < 0)
										{
											Value = 0;
										}
										else if ((Value + TotalDeltaPower) > 0xC)
										{
											Value = 0xC;
										}
										else
										{
											Value += TotalDeltaPower;
										}
									}
									else
									{
										if ((Value + TotalDeltaPower) < 0)
										{
											Value = 0;
										}
										else if ((Value + TotalDeltaPower) > 0xE)
										{
											Value = 0xE;
										}
										else
										{
											Value += TotalDeltaPower;
										}
									}
								}
								break;

								case TX_PWR_CFG_4: 
								{
									if ((Value + TotalDeltaPower) < 0)
									{
										Value = 0;
									}
									else if ((Value + TotalDeltaPower) > 0xC)
									{
										Value = 0xC;
									}
									else
									{
										Value += TotalDeltaPower;
									}
								}
								break;

								default: 
								{							
									// do nothing
									DBGPRINT(RT_DEBUG_ERROR, ("%s: unknown register = 0x%X\n", 
										__FUNCTION__, 
										(TX_PWR_CFG_0 + (i * 4))));
								}
								break;
							}
							TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);							
						}
					}
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, TxPwr[i]);
				}
			}
			else //RT30xx
			{				
				// Set Tx Power
				UCHAR ANT_POWER_INDEX=RF_R12+index;
				ATE_RF_IO_READ8_BY_REG_ID(pAd, ANT_POWER_INDEX, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | (TxPower & 0x1F);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, ANT_POWER_INDEX, (UCHAR)RFValue);
				DBGPRINT(RT_DEBUG_TRACE, ("3070 or 2070:%s (TxPower[%d]=%d, RFValue=%x)\n", __FUNCTION__, index,TxPower, RFValue));
			}
		}
		else
#ifdef RT53xx
		if (IS_RT5390(pAd))
		{
				UCHAR ANT_POWER_INDEX=RF_R49+index;
				ATE_RF_IO_READ8_BY_REG_ID(pAd, ANT_POWER_INDEX, (PUCHAR)&RFValue);
				RFValue = ((RFValue & ~0x3F) | (TxPower & 0x3F)); // tx0_alc
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, ANT_POWER_INDEX, (UCHAR)RFValue);
				if (!IS_RT5392(pAd))
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, 0x04);
		}
		else
#endif // RT53xx //
#endif // RTMP_RF_RW_SUPPORT //
	{
		if (pAd->ate.Channel <= 14)
		{
			if (TxPower > 31)
			{
				// R3, R4 can't large than 31 (0x24), 31 ~ 36 used by BBP 94
				R = 31;
				if (TxPower <= 36)
					Bbp94 = BBPR94_DEFAULT + (UCHAR)(TxPower - 31);		
			}
			else if (TxPower < 0)
			{
				// R3, R4 can't less than 0, -1 ~ -6 used by BBP 94
				R = 0;
				if (TxPower >= -6)
					Bbp94 = BBPR94_DEFAULT + TxPower;
			}
			else
			{  
				// 0 ~ 31
				R = (ULONG) TxPower;
				Bbp94 = BBPR94_DEFAULT;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%ld, BBP_R94=%d)\n", __FUNCTION__, TxPower, R, Bbp94));
		}
		else /* 5.5 GHz */
		{
			if (TxPower > 15)
			{
				// R3, R4 can't large than 15 (0x0F)
				R = 15;
			}
			else if (TxPower < 0)
			{
				// R3, R4 can't less than 0
				// -1 ~ -7
				ASSERT((TxPower >= -7));
				R = (ULONG)(TxPower + 7);
				bPowerReduce = TRUE;
			}
			else
			{  
				// 0 ~ 15
				R = (ULONG) TxPower;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%lu)\n", __FUNCTION__, TxPower, R));
		}

		if (pAd->ate.Channel <= 14)
		{
			if (index == 0)
			{
				// shift TX power control to correct RF(R3) register bit position
				R = R << 9;		
				R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
				pAd->LatchRfRegs.R3 = R;
			}
			else
			{
				// shift TX power control to correct RF(R4) register bit position
				R = R << 6;		
				R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
				pAd->LatchRfRegs.R4 = R;
			}
		}
		else /* 5.5GHz */
		{
			if (bPowerReduce == FALSE)
			{
				if (index == 0)
				{
					// shift TX power control to correct RF(R3) register bit position
					R = (R << 10) | (1 << 9);		
					R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
					pAd->LatchRfRegs.R3 = R;
				}
				else
				{
					// shift TX power control to correct RF(R4) register bit position
					R = (R << 7) | (1 << 6);		
					R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
					pAd->LatchRfRegs.R4 = R;
				}
			}
			else
			{
				if (index == 0)
				{
					// shift TX power control to correct RF(R3) register bit position
					R = (R << 10);		
					R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);

					/* Clear bit 9 of R3 to reduce 7dB. */ 
					pAd->LatchRfRegs.R3 = (R & (~(1 << 9)));
				}
				else
				{
					// shift TX power control to correct RF(R4) register bit position
					R = (R << 7);		
					R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);

					/* Clear bit 6 of R4 to reduce 7dB. */ 
					pAd->LatchRfRegs.R4 = (R & (~(1 << 6)));
				}
			}
		}
		RtmpRfIoWrite(pAd);
	}

	return 0;
}

#ifdef NEW_TXCONT
static UINT32 Default_TX_PIN_CFG;
#define RA_TX_PIN_CFG 0x1328
#define TxCont_TX_PIN_CFG 0x000C00F0
#endif // NEW_TXCONT //

static NDIS_STATUS ATESTART(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0, atemode=0, temp=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PTXD_STRUC		pTxD = NULL;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
    TXD_STRUC       TxD;
#endif // RT_BIG_ENDIAN //
#endif // RTMP_MAC_PCI //
#if defined(RT30xx) || defined(RT305x) || defined(RT3883) || defined(RT2883)
	UINT32			bbp_index=0;
	UCHAR			RestoreRfICType=pAd->RfIcType;
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3883) || defined(RT2883) //
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

#ifdef RTMP_MAC_PCI
	/* check if we have removed the firmware */
	if (!(ATE_ON(pAd)))
	{
		NICEraseFirmware(pAd);
	}
#endif // RTMP_MAC_PCI //

	atemode = pAd->ate.Mode;
	pAd->ate.Mode = ATE_START;
/*Set the channel parameters because of the station will calling swtiching channels during scanning.
   The ATESTART will block it and cause the RF could not receive any packets and the rx_done_complete
   will not be called anymore.*/
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	/* empty function */
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);
/*
	If there is an peding rx urb, the ATE will be block until there are not any pending rx urb.
*/
	/* Disable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);
	
	/* Disable auto responder */
	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &temp);
	temp = temp & 0xFFFFFFFE;
	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, temp);

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= 0xFFFFFFEF; 
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	if (atemode == ATE_TXCARR)
	{
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
		/* RT35xx ATE will reuse this code segment. */
		/* Hardware Reset BBP */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &temp);
		temp = temp | 0x00000002;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, temp);
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &temp);
		temp = temp & ~(0x00000002);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, temp);

		/* Restore All BBP Value */
		for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

		/*
			The RfIcType will be reset to zero after the hardware reset bbp command.
			Therefore, we must restore the original RfIcType.
		*/
		pAd->RfIcType=RestoreRfICType;
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //

		/* No Carrier Test set BBP R22 bit7=0, bit6=0, bit[5~0]=0x0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
		BbpData &= 0xFFFFFF00; // clear bit7, bit6, bit[5~0]	
	    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
	}
	else if (atemode == ATE_TXCARRSUPP)
	{
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
		/* RT35xx ATE will reuse this code segment. */
		/* Hardware Reset BBP */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &temp);
		temp = temp | 0x00000002;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, temp);
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &temp);
		temp = temp & ~(0x00000002);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, temp);

		/* Restore All BBP Value */
		for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);


		/*
			The RfIcType will be reset to zero after the hardware reset bbp command.
			Therefore, we must restore the original RfIcType.
		*/
		pAd->RfIcType=RestoreRfICType;			
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //

		/* No Cont. TX set BBP R22 bit7=0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
		BbpData &= ~(1 << 7); // set bit7=0
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

		/* No Carrier Suppression set BBP R24 bit0=0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R24, &BbpData);
		BbpData &= 0xFFFFFFFE; // clear bit0	
	    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, BbpData);
	}		

	/*
		We should free some resource which was allocated
		when ATE_TXFRAME , ATE_STOP, and ATE_TXCONT.
	*/
	else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
	{
#ifdef RTMP_MAC_PCI
		PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];
#endif // RTMP_MAC_PCI //
		if ((atemode == ATE_TXCONT))
		{
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
			/* RT35xx ATE will reuse this code segment. */
			/* Hardware Reset BBP */
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &temp);
			temp = temp | 0x00000002;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, temp);
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &temp);
			temp = temp & ~(0x00000002);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, temp);

			/* Restore All BBP Value */
			for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);


		pAd->RfIcType=RestoreRfICType;			
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //

			/* Not Cont. TX anymore, so set BBP R22 bit7=0 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= ~(1 << 7); // set bit7=0
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
#ifdef NEW_TXCONT
			BbpSoftReset(pAd);
			RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, Default_TX_PIN_CFG);
#endif // NEW_TXCONT //
		}

		/* Abort Tx, Rx DMA. */
		RtmpDmaEnable(pAd, 0);
#ifdef RTMP_MAC_PCI
		for (ring_index=0; ring_index<TX_RING_SIZE; ring_index++)
		{
			PNDIS_PACKET  pPacket;

#ifndef RT_BIG_ENDIAN
		    pTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[ring_index].AllocVa;
#else
    		pDestTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[ring_index].AllocVa;
    		TxD = *pDestTxD;
    		pTxD = &TxD;
    		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif // !RT_BIG_ENDIAN //
			pTxD->DMADONE = 0;
			pPacket = pTxRing->Cell[ring_index].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[ring_index].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNextNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif // RT_BIG_ENDIAN //
		}
#endif // RTMP_MAC_PCI //

		/* Start Tx, RX DMA */
		RtmpDmaEnable(pAd, 1);
	}


	/* reset Rx statistics. */
	pAd->ate.LastSNR0 = 0;
	pAd->ate.LastSNR1 = 0;
	pAd->ate.LastRssi0 = 0;
	pAd->ate.LastRssi1 = 0;
	pAd->ate.LastRssi2 = 0;
	pAd->ate.AvgRssi0 = 0;
	pAd->ate.AvgRssi1 = 0;
	pAd->ate.AvgRssi2 = 0;
	pAd->ate.AvgRssi0X8 = 0;
	pAd->ate.AvgRssi1X8 = 0;
	pAd->ate.AvgRssi2X8 = 0;
	pAd->ate.NumOfAvgRssiSample = 0;

#ifdef RALINK_QA
	/* Tx frame */
	pAd->ate.bQATxStart = FALSE;
	pAd->ate.bQARxStart = FALSE;
	pAd->ate.seq = 0; 

	/* counters */
	pAd->ate.U2M = 0;
	pAd->ate.OtherData = 0;
	pAd->ate.Beacon = 0;
	pAd->ate.OtherCount = 0;
	pAd->ate.TxAc0 = 0;
	pAd->ate.TxAc1 = 0;
	pAd->ate.TxAc2 = 0;
	pAd->ate.TxAc3 = 0;
	pAd->ate.TxHCCA = 0;
	pAd->ate.TxMgmt = 0;
	pAd->ate.RSSI0 = 0;
	pAd->ate.RSSI1 = 0;
	pAd->ate.RSSI2 = 0;
	pAd->ate.SNR0 = 0;
	pAd->ate.SNR1 = 0;

	/* control */
	pAd->ate.TxDoneCount = 0;
	// TxStatus : 0 --> task is idle, 1 --> task is running
	pAd->ate.TxStatus = 0;
#endif // RALINK_QA //

	// Soft reset BBP.
	BbpSoftReset(pAd);

#ifdef CONFIG_AP_SUPPORT 
#ifdef RTMP_MAC_PCI
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#endif // RTMP_MAC_PCI //

   	/* pAd->ate.Mode must HAVE been ATE_START prior to call ATEAPStop(pAd) */
	/* We must disable DFS and Radar Detection, or 8051 will modify BBP registers. */
	ATEAPStop(pAd);
#endif // CONFIG_AP_SUPPORT //


#ifdef RTMP_MAC_PCI
	/* Disable Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Disable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

#ifdef RT3390
#ifdef RTMP_RF_RW_SUPPORT
	if (IS_RT3390(pAd))
	{
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R10,0x71);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd,RF_R19,0x84);
	}
#endif // RTMP_RF_RW_SUPPORT //
#endif // RT3390 //
#endif // RTMP_MAC_PCI //


#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPEnable == TRUE)
	{
		DisableAPMIMOPS(pAd);
		pAd->ApCfg.GreenAPLevel=GREENAP_WITHOUT_ANY_STAS_CONNECT;
	}
#endif // GREENAP_SUPPORT //

#ifdef IQ_CAL_SUPPORT
	pAd->ate.bIQCompEnable = FALSE;
#endif /* IQ_CAL_SUPPORT */

#ifdef RT5390
	if (IS_RT5390(pAd))
	{
		/* Extra set MAC registers to compensate Tx power if any */
		RT539x_ATEAsicExtraPowerOverMAC(pAd);	
	}
#endif /* RT5390 */

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS ATESTOP(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0, ring_index=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	PRXD_STRUC		pRxD = NULL;
#endif // RTMP_MAC_PCI //
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
	UINT32			bbp_index=0;
	UCHAR			RestoreRfICType=pAd->RfIcType;
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) // 
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
	/* RT35xx ATE will reuse this code segment. */
	/* hardware reset BBP */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData = MacData | 0x00000002;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	RTMPusecDelay(10000);

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData = MacData & ~(0x00000002);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Supposed that we have had a record in restore_BBP[] */
	/* restore all BBP value */
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd,bbp_index,restore_BBP[bbp_index]);

	/*
		The RfIcType will be reset to zero after the hardware reset bbp command.
		Therefore, we must restore the original RfIcType.
	*/
	ASSERT(RestoreRfICType != 0);
	pAd->RfIcType=RestoreRfICType;
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) // 

	/* Default value in BBP R22 is 0x0. */
	BbpData = 0;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

	/* Clear bit4 to stop continuous Tx production test. */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= 0xFFFFFFEF;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData); 
	
	/* Disable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);
	
	/* Abort Tx, RX DMA */
	RtmpDmaEnable(pAd, 0);

	/* Disable Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

#ifdef RTMP_MAC_PCI
	pAd->ate.bFWLoading = TRUE;

	Status = NICLoadFirmware(pAd);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		return Status;
	}
	pAd->ate.Mode = ATE_STOP;

	/*
		Even the firmware has been loaded, 
		we still could use ATE_BBP_IO_READ8_BY_REG_ID(). 
		But this is not suggested.
	*/
	BbpSoftReset(pAd);

	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	
	NICInitializeAdapter(pAd, TRUE);
	
	/*
		Reinitialize Rx Ring before Rx DMA is enabled.
		>>>RxCoherent<<< was gone !
	*/
	for (ring_index = 0; ring_index < RX_RING_SIZE; ring_index++)
	{
		pRxD = (PRXD_STRUC) pAd->RxRing.Cell[ring_index].AllocVa;
		pRxD->DDONE = 0;
	}

	/* We should read EEPROM for all cases. */  
	NICReadEEPROMParameters(pAd, NULL);
	NICInitAsicFromEEPROM(pAd); 

	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);

	/* empty function */
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);		

	/* clear garbage interrupts */
	RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, 0xffffffff);
	/* Enable Interrupt */
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
#endif // RTMP_MAC_PCI //


	/* restore RX_FILTR_CFG */
#ifdef CONFIG_AP_SUPPORT 
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, APNORMAL);
#endif // CONFIG_AP_SUPPORT //


	/* Enable Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData |= (1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Enable Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);

	/* Enable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData |= (1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

#ifdef RTMP_MAC_PCI
#ifdef CONFIG_AP_SUPPORT 
	APStartUp(pAd);
#endif // CONFIG_AP_SUPPORT // 

#endif // RTMP_MAC_PCI

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXCARR(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
	UINT32			bbp_index=0;	 
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	pAd->ate.Mode = ATE_TXCARR;

#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
	/* RT35xx ATE will reuse this code segment. */
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		restore_BBP[bbp_index]=0;

	/* Record All BBP Value */
	for (bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //


	/* QA has done the following steps if it is used. */
	if (pAd->ate.bQATxStart == FALSE) 
	{
		/* RT3883 does not need BbpSoftReset() */
		/* Soft reset BBP. */
		BbpSoftReset(pAd);

		/* Carrier Test set BBP R22 bit7=1, bit6=1, bit[5~0]=0x01 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
		BbpData &= 0xFFFFFF00; // clear bit7, bit6, bit[5~0]
		BbpData |= 0x000000C1; // set bit7=1, bit6=1, bit[5~0]=0x01
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

		/* set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1 */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
		MacData = MacData | 0x00000010;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXCONT(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PTXD_STRUC		pTxD = NULL;
	PRTMP_TX_RING 	pTxRing = &pAd->TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
    TXD_STRUC       TxD;
#endif // RT_BIG_ENDIAN //
#endif // RTMP_MAC_PCI //
#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
	UINT32			bbp_index=0;
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	if (pAd->ate.bQATxStart == TRUE)
	{
		/*
			set MAC_SYS_CTRL(0x1004) bit4(Continuous Tx Production Test)
			and bit2(MAC TX enable) back to zero.
		*/ 
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
		MacData &= 0xFFFFFFEB;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

		/* set BBP R22 bit7=0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
		BbpData &= 0xFFFFFF7F; // set bit7=0
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
	}
#ifdef NEW_TXCONT 
	else
	{
		/* store the original value of RA_TX_PIN_CFG */
		RTMP_IO_READ32(pAd, RA_TX_PIN_CFG, &Default_TX_PIN_CFG);
	}
#endif // NEW_TXCONT //

#if defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883)
	/* RT35xx ATE will reuse this code segment. */
	for(bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		restore_BBP[bbp_index]=0;

	/* Record All BBP Value */
	for(bbp_index=0;bbp_index<ATE_BBP_REG_NUM;bbp_index++)
		ATE_BBP_IO_READ8_BY_REG_ID(pAd,bbp_index,&restore_BBP[bbp_index]);
#endif // defined(RT30xx) || defined(RT305x) || defined(RT3350) || defined(RT3352) || defined(RT3883) || defined(RT2883) //


	/* Step 1: send 50 packets first. */
	pAd->ate.Mode = ATE_TXCONT;
	pAd->ate.TxCount = 50;

	/* RT3883 does not need BbpSoftReset() */
	/* Soft reset BBP. */
	BbpSoftReset(pAd);

	/* Abort Tx, RX DMA. */
	RtmpDmaEnable(pAd, 0);

#ifdef RTMP_MAC_PCI
	{
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
		pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
		pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);
	}
#endif // RTMP_MAC_PCI //

	/* Do it after Tx/Rx DMA is aborted. */
	pAd->ate.TxDoneCount = 0;
	
	/* Only needed if we have to send some normal frames. */
	SetJapanFilter(pAd);

#ifdef RTMP_MAC_PCI
	for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pAd->ate.TxCount); ring_index++)
	{
		PNDIS_PACKET pPacket;
		UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
		pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
		pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
		TxD = *pDestTxD;
		pTxD = &TxD;
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif // !RT_BIG_ENDIAN //

		/* Clear current cell. */
		pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNdisPacket = NULL;

		pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNextNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif // RT_BIG_ENDIAN //

		if (ATESetUpFrame(pAd, TxIdx) != 0)
			return NDIS_STATUS_FAILURE;

		INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
	}

	ATESetUpFrame(pAd, pTxRing->TxCpuIdx);
#endif // RTMP_MAC_PCI //


	/* Enable Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData |= (1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Disable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Start Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);


#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		pAd->ate.TxStatus = 1;
	}
#endif // RALINK_QA //

#ifdef RTMP_MAC_PCI
	/* kick Tx Ring */
	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

	RTMPusecDelay(5000);
#endif // RTMP_MAC_PCI //


#ifdef NEW_TXCONT
	/* give RA_TX_PIN_CFG(0x1328) a proper value. */
	MacData = TxCont_TX_PIN_CFG;
	RTMP_IO_WRITE32(pAd, RA_TX_PIN_CFG, MacData);

	/* Cont. TX set BBP R22 bit7=1 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
	BbpData |= 0x00000080; // set bit7=1
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
#else
	/* Step 2: send more 50 packets then start continue mode. */
	/* Abort Tx, RX DMA. */
	RtmpDmaEnable(pAd, 0);

	/* Cont. TX set BBP R22 bit7=1 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
	BbpData |= 0x00000080; // set bit7=1
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

	pAd->ate.TxCount = 50;
#ifdef RTMP_MAC_PCI
	{
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
		pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
		pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);					
	}
#endif // RTMP_MAC_PCI //

	pAd->ate.TxDoneCount = 0;
	SetJapanFilter(pAd);

#ifdef RTMP_MAC_PCI
	for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pAd->ate.TxCount); ring_index++)
	{
		PNDIS_PACKET pPacket;
		UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
		pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
		pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
		TxD = *pDestTxD;
		pTxD = &TxD;
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif // !RT_BIG_ENDIAN //
		/* clear current cell */
		pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNdisPacket as NULL after clear. */
		pTxRing->Cell[TxIdx].pNdisPacket = NULL;

		pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNextNdisPacket as NULL after clear. */
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif // RT_BIG_ENDIAN //

		if (ATESetUpFrame(pAd, TxIdx) != 0)
			return NDIS_STATUS_FAILURE;

		INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
	}

	ATESetUpFrame(pAd, pTxRing->TxCpuIdx);
#endif // RTMP_MAC_PCI //


	/* Enable Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData |= (1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Disable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Start Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);

#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		pAd->ate.TxStatus = 1;
	}
#endif // RALINK_QA //

#ifdef RTMP_MAC_PCI
	/* kick Tx Ring */
	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);
#endif // RTMP_MAC_PCI //

	RTMPusecDelay(500);

	/* enable continuous tx production test */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData |= 0x00000010;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);							
#endif // NEW_TXCONT //

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS TXFRAME(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PRTMP_TX_RING 	pTxRing = &pAd->TxRing[QID_AC_BE];
	PTXD_STRUC		pTxD = NULL;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
    TXD_STRUC       TxD;
#endif // RT_BIG_ENDIAN //
#endif // RTMP_MAC_PCI //
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s(Count=%d)\n", __FUNCTION__, pAd->ate.TxCount));
	pAd->ate.Mode |= ATE_TXFRAME;

#ifdef RTMP_MAC_PCI
	/* Default value in BBP R22 is 0x0. */
	BbpData = 0;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

	/* RT3883 does not need BbpSoftReset() */
	/* Soft reset BBP. */
	BbpSoftReset(pAd);

	/* clear bit4 to stop continuous Tx production test */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= 0xFFFFFFEF; 
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Abort Tx, RX DMA. */
	RtmpDmaEnable(pAd, 0);

	{
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * RINGREG_DIFF,  &pTxRing->TxDmaIdx);
		pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
		pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pTxRing->TxCpuIdx);					
	}

	pAd->ate.TxDoneCount = 0;

	SetJapanFilter(pAd);
	
	for (ring_index = 0; (ring_index < TX_RING_SIZE-1) && (ring_index < pAd->ate.TxCount); ring_index++)
	{
		PNDIS_PACKET pPacket;
		UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
		pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
		pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
		TxD = *pDestTxD;
		pTxD = &TxD;
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif // !RT_BIG_ENDIAN //
		/* Clear current cell. */
		pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNdisPacket = NULL;

		pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* Always assign pNextNdisPacket as NULL after clear */
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif // RT_BIG_ENDIAN //

		if (ATESetUpFrame(pAd, TxIdx) != 0)
			return NDIS_STATUS_FAILURE;

		INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);

	}

	ATESetUpFrame(pAd, pTxRing->TxCpuIdx);

	/* Start Tx, Rx DMA. */
	RtmpDmaEnable(pAd, 1);

	/* Enable Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData |= (1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);
#endif // RTMP_MAC_PCI //


#ifdef RALINK_QA
	/* add this for LoopBack mode */
	if (pAd->ate.bQARxStart == FALSE)  
	{
		/* Disable Rx */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
		MacData &= ~(1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);
	}

	if (pAd->ate.bQATxStart == TRUE)  
	{
		pAd->ate.TxStatus = 1;
	}
#else
	/* Disable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);
#endif // RALINK_QA //

#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * RINGREG_DIFF, &pAd->TxRing[QID_AC_BE].TxDmaIdx);
	/* kick Tx Ring */
	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

	pAd->RalinkCounters.KickTxCount++;
#endif // RTMP_MAC_PCI //


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS RXFRAME(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	/* Disable Rx of MAC block */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	/* Default value in BBP R22 is 0x0. */
	BbpData = 0;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);


	/* Clear bit4 to stop continuous Tx production test. */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= 0xFFFFFFEF;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	pAd->ate.Mode |= ATE_RXFRAME;


	/* Disable Tx of MAC block. */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);


	/* Enable Rx of MAC block. */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData |= (1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


#ifdef RALINK_QA
static NDIS_STATUS TXSTOP(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0, atemode=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	UINT32			ring_index=0;
	PTXD_STRUC		pTxD = NULL;
	PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD = NULL;
    TXD_STRUC       TxD;
#endif // RT_BIG_ENDIAN //
#endif // RTMP_MAC_PCI //
	UCHAR			BbpData = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));
	
	atemode = pAd->ate.Mode;
	pAd->ate.Mode &= ATE_TXSTOP;
	pAd->ate.bQATxStart = FALSE;

	if (atemode == ATE_TXCARR)
	{
		/* No Carrier Test set BBP R22 bit7=0, bit6=0, bit[5~0]=0x0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
		BbpData &= 0xFFFFFF00; // clear bit7, bit6, bit[5~0]	
	    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
	}
	else if (atemode == ATE_TXCARRSUPP)
	{
		/* No Cont. TX set BBP R22 bit7=0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
		BbpData &= ~(1 << 7); // set bit7=0
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

		/* No Carrier Suppression set BBP R24 bit0=0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R24, &BbpData);
		BbpData &= 0xFFFFFFFE; // clear bit0	
	    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, BbpData);
	}		

	/*
		We should free some resource which was allocated
		when ATE_TXFRAME, ATE_STOP, and ATE_TXCONT.
	*/
	else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
	{
		if (atemode == ATE_TXCONT)
		{
#ifndef NEW_TXCONT
			// No Cont. TX set BBP R22 bit7=0
			/* QA will do this in new TXCONT proposal */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= ~(1 << 7); //set bit7=0
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
#endif // !NEW_TXCONT //
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
    		TxD = *pDestTxD;
    		pTxD = &TxD;
    		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif // !RT_BIG_ENDIAN //
			pTxD->DMADONE = 0;
			pPacket = pTxRing->Cell[ring_index].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[ring_index].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNextNdisPacket as NULL after clear */
			pTxRing->Cell[ring_index].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif // RT_BIG_ENDIAN //
		}
		/* Enable Tx, Rx DMA */
		RtmpDmaEnable(pAd, 1);
#endif // RTMP_MAC_PCI //

	}

	/* task Tx status : 0 --> task is idle, 1 --> task is running */
	pAd->ate.TxStatus = 0;

#ifndef NEW_TXCONT
	/* Soft reset BBP. */
	BbpSoftReset(pAd);
#endif // !NEW_TXCONT //

	/* Disable Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);


	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}


static NDIS_STATUS RXSTOP(
	IN PRTMP_ADAPTER pAd)
{
	UINT32			MacData=0;
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : ===> %s\n", __FUNCTION__));

	/* Disable Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
	MacData &= ~(1 << 3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

	pAd->ate.Mode &= ATE_RXSTOP;
	pAd->ate.bQARxStart = FALSE;
//	pAd->ate.TxDoneCount = pAd->ate.TxCount;


	/* RT3883 does not need BbpSoftReset() */
	/* Soft reset BBP. */
	BbpSoftReset(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("ATE : <=== %s\n", __FUNCTION__));
	return Status;
}
#endif // RALINK_QA //


/*
==========================================================================
    Description:
        Set ATE operation mode to
        0. ATESTART  = Start ATE Mode
        1. ATESTOP   = Stop ATE Mode
        2. TXCARR    = Transmit Carrier
        3. TXCONT    = Continuous Transmit
        4. TXFRAME   = Transmit Frames
        5. RXFRAME   = Receive Frames
#ifdef RALINK_QA
        6. TXSTOP    = Stop Any Type of Transmition
        7. RXSTOP    = Stop Receiving Frames        
#endif // RALINK_QA //

    Return:
        NDIS_STATUS_SUCCESS if all parameters are OK.
==========================================================================
*/
static NDIS_STATUS	ATECmdHandler(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s\n", __FUNCTION__));

#ifdef CONFIG_RT2880_ATE_CMD_NEW
	if (!strcmp(arg, "ATESTART")) 		
	{
            	/* Enter/Reset ATE mode and set Tx/Rx Idle */
		Status = ATESTART(pAd);
	}
	else if (!strcmp(arg, "ATESTOP")) 
	{
            	/* Leave ATE mode */
		Status = ATESTOP(pAd);
	}
#else
	if (!strcmp(arg, "APSTOP")) 		
	{
		Status = ATESTART(pAd);
	}
	else if (!strcmp(arg, "APSTART")) 
	{
		Status = ATESTOP(pAd);
	}
#endif /* CONFIG_RT2880_ATE_CMD_NEW */
	else if (!strcmp(arg, "TXCARR"))	
	{
		ATEAsicSwitchChannel(pAd);
		RTMPusecDelay(5000);
		Status = TXCARR(pAd);
	}
	else if (!strcmp(arg, "TXCONT"))	
	{
		ATEAsicSwitchChannel(pAd);
		RTMPusecDelay(5000);
		Status = TXCONT(pAd);
	}
	else if (!strcmp(arg, "TXFRAME")) 
	{
		ATEAsicSwitchChannel(pAd);
		RTMPusecDelay(5000);
		Status = TXFRAME(pAd);
	}
	else if (!strcmp(arg, "RXFRAME")) 
	{
		ATEAsicSwitchChannel(pAd);
		RTMPusecDelay(5000);
		Status = RXFRAME(pAd);
	}
#ifdef RALINK_QA
	/* Enter ATE mode and set Tx/Rx Idle */
	else if (!strcmp(arg, "TXSTOP"))
	{
		Status = TXSTOP(pAd);
	}
	else if (!strcmp(arg, "RXSTOP"))
	{
		Status = RXSTOP(pAd);
	}
#endif /* RALINK_QA */
	else
	{	
		DBGPRINT(RT_DEBUG_TRACE, ("ATE : Invalid arg !\n"));
		Status = NDIS_STATUS_INVALID_DATA;
	}
	
	RTMPusecDelay(5000);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s\n", __FUNCTION__));
	return Status;
}


INT	Set_ATE_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	// Handle ATEACTIVE and ATEPASSIVE commands as a special case
	if (!strcmp(arg, "ATEACTIVE"))
	{
		pAd->ate.PassiveMode = FALSE;
		return TRUE;
	}

	if (!strcmp(arg, "ATEPASSIVE"))
	{
		pAd->ate.PassiveMode = TRUE;
		return TRUE;
	}

	// Disallow all other ATE commands in passive mode
	if (pAd->ate.PassiveMode)
		return TRUE;

	if (ATECmdHandler(pAd, arg) == NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Proc Failed\n"));
		return FALSE;
	}
}


/* 
==========================================================================
    Description:
        Set ATE ADDR1=DA for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR3=DA for TxFrame(STA : To DS = 1 ; From DS = 0)        
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_DA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PSTRING				value;
	INT					i;

	// Mac address acceptable format 01:02:03:04:05:06 length 17	
	if (strlen(arg) != 17)  
		return FALSE;

    for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pAd->ate.Addr1[i++], 1);
#endif // CONFIG_AP_SUPPORT //

	}

	/* sanity check */
	if (i != 6)
	{
		return FALSE;  
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %2X:%2X:%2X:%2X:%2X:%2X)\n", pAd->ate.Addr1[0],
		pAd->ate.Addr1[1], pAd->ate.Addr1[2], pAd->ate.Addr1[3], pAd->ate.Addr1[4], pAd->ate.Addr1[5]));

#endif // CONFIG_AP_SUPPORT //


	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_DA_Proc Success\n"));
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE ADDR3=SA for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR2=SA for TxFrame(STA : To DS = 1 ; From DS = 0)
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_SA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PSTRING				value;
	INT					i;

	// Mac address acceptable format 01:02:03:04:05:06 length 17	
	if (strlen(arg) != 17)  
		return FALSE;

    for (i=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pAd->ate.Addr3[i++], 1);
#endif // CONFIG_AP_SUPPORT //

	}

	/* sanity check */
	if (i != 6)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %2X:%2X:%2X:%2X:%2X:%2X)\n", pAd->ate.Addr3[0],
		pAd->ate.Addr3[1], pAd->ate.Addr3[2], pAd->ate.Addr3[3], pAd->ate.Addr3[4], pAd->ate.Addr3[5]));
#endif // CONFIG_AP_SUPPORT //


	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_SA_Proc Success\n"));

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE ADDR2=BSSID for TxFrame(AP  : To DS = 0 ; From DS = 1)
        or
        Set ATE ADDR1=BSSID for TxFrame(STA : To DS = 1 ; From DS = 0)

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_BSSID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PSTRING				value;
	INT					i;

	// Mac address acceptable format 01:02:03:04:05:06 length 17	
	if (strlen(arg) != 17)  
		return FALSE;

    for (i=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) 
	{
		/* sanity check */
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;  
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(value, &pAd->ate.Addr2[i++], 1);
#endif // CONFIG_AP_SUPPORT //

	}

	/* sanity check */
	if(i != 6)
	{
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %2X:%2X:%2X:%2X:%2X:%2X)\n",	pAd->ate.Addr2[0],
		pAd->ate.Addr2[1], pAd->ate.Addr2[2], pAd->ate.Addr2[3], pAd->ate.Addr2[4], pAd->ate.Addr2[5]));
#endif // CONFIG_AP_SUPPORT //


	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_BSSID_Proc Success\n"));

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx Channel

    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_CHANNEL_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR channel;

	channel = simple_strtol(arg, 0, 10);

	// to allow A band channel : ((channel < 1) || (channel > 14))
	if ((channel < 1) || (channel > 216))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_CHANNEL_Proc::Out of range, it should be in range of 1~14.\n"));
		return FALSE;
	}

	pAd->ate.Channel = channel;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_CHANNEL_Proc (ATE Channel = %d)\n", pAd->ate.Channel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_CHANNEL_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx Power0
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_POWER0_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR TxPower;
	
	TxPower = simple_strtol(arg, 0, 10);

	if (pAd->ate.Channel <= 14)
	{
		if (!IS_RT3390(pAd))
		{
			if ((TxPower > 31) || (TxPower < 0))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER0_Proc::Out of range (Value=%d)\n", TxPower));
				return FALSE;
			}
		}	
	}
#ifdef A_BAND_SUPPORT
	else/* 5.5 GHz */
	{
		if ((TxPower > 15) || (TxPower < -7))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER0_Proc::Out of range (Value=%d)\n", TxPower));
			return FALSE;
		}
	}
#endif /* A_BAND_SUPPORT */

	pAd->ate.TxPower0 = TxPower;
	ATETxPwrHandler(pAd, 0);


	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_POWER0_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx Power1
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_POWER1_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR TxPower;
	
	TxPower = simple_strtol(arg, 0, 10);

	if (pAd->ate.Channel <= 14)
	{
		if (!IS_RT3390(pAd))
		{
			if ((TxPower > 31) || (TxPower < 0))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER1_Proc::Out of range (Value=%d)\n", TxPower));
				return FALSE;
			}
		}
	}
#ifdef A_BAND_SUPPORT
	else /* 5.5 GHz */
	{
		if ((TxPower > 15) || (TxPower < -7))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER1_Proc::Out of range (Value=%d)\n", TxPower));
			return FALSE;
		}
	}
#endif /* A_BAND_SUPPORT */

	pAd->ate.TxPower1 = TxPower;
	ATETxPwrHandler(pAd, 1);


	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_POWER1_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}




/* 
==========================================================================
    Description:
        Set ATE Tx Antenna
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR value;
	
	value = simple_strtol(arg, 0, 10);

	if ((value > 2) || (value < 0))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_Antenna_Proc::Out of range (Value=%d)\n", value));
		return FALSE;
	}

	pAd->ate.TxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_Antenna_Proc (Antenna = %d)\n", pAd->ate.TxAntennaSel));
	DBGPRINT(RT_DEBUG_TRACE,("Ralink: Set_ATE_TX_Antenna_Proc Success\n"));

	/* calibration power unbalance issues */
	ATEAsicSwitchChannel(pAd);

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Rx Antenna
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_RX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR value;
	
	value = simple_strtol(arg, 0, 10);

	if ((value > 3) || (value < 0))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_RX_Antenna_Proc::Out of range (Value=%d)\n", value));
		return FALSE;
	}

	pAd->ate.RxAntennaSel = value;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_Antenna_Proc (Antenna = %d)\n", pAd->ate.RxAntennaSel));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_Antenna_Proc Success\n"));

	/* calibration power unbalance issues */
	ATEAsicSwitchChannel(pAd);

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}




/* 
==========================================================================
    Description:
        Set ATE RF frequence offset
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_FREQOFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR RFFreqOffset = 0;
	ULONG R4 = 0;
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR RFValue = 0;
#endif // RTMP_RF_RW_SUPPORT //
	
	RFFreqOffset = simple_strtol(arg, 0, 10);

#ifdef RTMP_RF_RW_SUPPORT
	/* RT35xx ATE will reuse this code segment */
	if (RFFreqOffset >= 96)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_FREQOFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}
#else
	if (RFFreqOffset >= 64)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_FREQOFFSET_Proc::Out of range(0 ~ 63).\n"));
		return FALSE;
	}
#endif // RTMP_RF_RW_SUPPORT //

	pAd->ate.RFFreqOffset = RFFreqOffset;

#ifdef RTMP_RF_RW_SUPPORT
	if (IS_RT30xx(pAd) || IS_RT3390(pAd) || IS_RT3572(pAd))
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
		RFValue = ((RFValue & 0x80) | pAd->ate.RFFreqOffset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);
	}
	else
#ifdef RT53xx
	if (IS_RT5390(pAd))
	{
		UCHAR HighCurrentBit = 0;
		RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RFValue);
		pAd->ate.PreRFXCodeValue = (UCHAR)RFValue;
		HighCurrentBit = pAd->ate.PreRFXCodeValue & 0x80;
		RFValue = min((pAd->ate.RFFreqOffset & 0x7F), 0x5F);
		RFValue = HighCurrentBit | (RFValue & 0x7F);
		RFMultiStepXoCode(pAd, RF_R17, (UCHAR)RFValue, pAd->ate.PreRFXCodeValue);
	}
	else
#endif // RT53xx //
#endif // RTMP_RF_RW_SUPPORT //
	{
		// RT28xx
		// shift TX power control to correct RF register bit position
		R4 = pAd->ate.RFFreqOffset << 15;		
		R4 |= (pAd->LatchRfRegs.R4 & ((~0x001f8000)));
		pAd->LatchRfRegs.R4 = R4;
		
		RtmpRfIoWrite(pAd);
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_FREQOFFSET_Proc (RFFreqOffset = %d)\n", pAd->ate.RFFreqOffset));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_FREQOFFSET_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if ((BBPCurrentBW == 0)
#ifdef RT30xx
		|| IS_RT2070(pAd)
#endif // RT30xx //
		)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
 	}

	/* RT35xx ATE will reuse this code segment. */
	// Fix the error spectrum of CCK-40MHZ.
	// Turn on BBP 20MHz mode by request here.
	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\nBandwidth switch to 20\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgGBand[powerIndex]);	
				}
				RTMPusecDelay(5000);				
			}
		}
#ifdef A_BAND_SUPPORT
		else
		{
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

 				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
 				}
 				RTMPusecDelay(5000);				
 			}
		}
#endif /* A_BAND_SUPPORT */

		// Set BBP R4 bit[4:3]=0:0
 		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
 		value &= (~0x18);
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);


		// Set BBP R66=0x3C
		value = 0x3C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);

		// Set BBP R68=0x0B
		// to improve Rx sensitivity.
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		// Set BBP R69=0x16
		if ( IS_RT5390(pAd))
			value = 0x12;
		else
		value = 0x16;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		// Set BBP R70=0x08
		if ( IS_RT5390(pAd))
			value = 0x0A;
		else
		value = 0x08;
		
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		// Set BBP R73=0x11
		if ( IS_RT5390(pAd))
			value = 0x13;
		else
			value = 0x11;

 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

			if (TxMode == MODE_CCK)
			{
				// when Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1
 				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
				value |= 0x20; //set bit5=1
 				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);				
			}
		}

#ifdef RT30xx
		// set BW = 20 MHz
		if (IS_RT30xx(pAd) || IS_RT3390(pAd))
                {
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR) pAd->Mlme.CaliBW20RfR24);
#ifdef RT3390
			if (IS_RT3390(pAd))
			{
				UCHAR    RFValue;
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R24, (PUCHAR)(&RFValue));			
				if(pAd->ate.TxWI.PHYMODE == MODE_CCK)				
					RFValue |= 0x0F;			
				else				
					RFValue &= 0xF8;	
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, RFValue);
			}
#endif // RT3390 //			
			
                        ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R31, &value);
			value &= (~0x20);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, value);
                }
		else
#endif // RT30xx //
#ifdef RT53xx
		if (IS_RT5390(pAd))
		{
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, 0x10); /* 20111208 Update */

			if (IS_RT5392(pAd))
			{
				/* Fix the throughput drop issue while STA 20MBW connection with AP 20/40M BW */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x3C);	
			}
		}	
#endif /* RT53xx */
		// set BW = 20 MHz
		{
			pAd->LatchRfRegs.R4 &= ~0x00200000;
			RtmpRfIoWrite(pAd);
		}
	}
	// If bandwidth = 40M, set RF Reg4 bit 21 = 0.
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgGBand[powerIndex]);	
				}
					RTMPusecDelay(5000);				
			}
		}
#ifdef A_BAND_SUPPORT
		else
		{
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				}
					RTMPusecDelay(5000);				
			}		
#ifdef DOT11_N_SUPPORT
			if ((pAd->ate.TxWI.PHYMODE >= MODE_HTMIX) && (pAd->ate.TxWI.MCS == 7))
			{
    				value = 0x28;
    				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, value);
			}
#endif /* DOT11_N_SUPPORT */
		}
#endif /* A_BAND_SUPPORT */

		// Set BBP R4 bit[4:3]=1:0
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		value |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);


		// Set BBP R66=0x3C
		value = 0x3C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);

		// Set BBP R68=0x0C
		// to improve Rx sensitivity
		value = 0x0C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		// Set BBP R69=0x1A
		value = 0x1A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		// Set BBP R70=0x0A
		value = 0x0A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		// Set BBP R73=0x16
		if ( IS_RT5390(pAd))
			value = 0x13;
		else
		        value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		// If bandwidth = 40M, set RF Reg4 bit 21 = 1.
#ifdef RT30xx
		// set BW = 40 MHz
		if(IS_RT30xx(pAd) || IS_RT3390(pAd))
                 {
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR) pAd->Mlme.CaliBW40RfR24);
                        ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R31, &value);
			value |= 0x20;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, value);
                  }
		else
#endif // RT30xx //
#ifdef RT53xx
		if (IS_RT5390(pAd))
		{
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, 0x16); /* 20111208 Update */

			if (IS_RT5392(pAd))
			{
				/* Fix the throughput drop issue while STA 20MBW connection with AP 20/40M BW */
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x1C);	
			}
		}		
#endif /* RT53xx */
		// set BW = 40 MHz
		{
		pAd->LatchRfRegs.R4 |= 0x00200000;
		RtmpRfIoWrite(pAd);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame length
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_LENGTH_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->ate.TxLength = simple_strtol(arg, 0, 10);

	if ((pAd->ate.TxLength < 24) || (pAd->ate.TxLength > (MAX_FRAME_SIZE - 34/* == 2312 */)))
	{
		pAd->ate.TxLength = (MAX_FRAME_SIZE - 34/* == 2312 */);
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_LENGTH_Proc::Out of range, it should be in range of 24~%d.\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_LENGTH_Proc (TxLength = %d)\n", pAd->ate.TxLength));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_LENGTH_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame count
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_COUNT_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->ate.TxCount = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pAd->ate.TxCount));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_COUNT_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame MCS
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_MCS_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR MCS;
	INT result;

	MCS = simple_strtol(arg, 0, 10);
	result = CheckMCSValid(pAd, pAd->ate.TxWI.PHYMODE, MCS);

	if (result != -1)
	{
		pAd->ate.TxWI.MCS = (UCHAR)MCS;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MCS_Proc::Out of range, refer to rate table.\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MCS_Proc (MCS = %d)\n", pAd->ate.TxWI.MCS));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MCS_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame Mode
        0: MODE_CCK
        1: MODE_OFDM
        2: MODE_HTMIX
        3: MODE_HTGREENFIELD
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_MODE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR BbpData = 0;
	UCHAR calRFValue;
	UCHAR RFValue;
	
	pAd->ate.TxWI.PHYMODE = simple_strtol(arg, 0, 10);

	if (pAd->ate.TxWI.PHYMODE > 3)
	{
		pAd->ate.TxWI.PHYMODE = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MODE_Proc::Out of range.\nIt should be in range of 0~3\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("0: CCK, 1: OFDM, 2: HT_MIX, 3: HT_GREEN_FIELD.\n"));
		return FALSE;
	}

	// Turn on BBP 20MHz mode by request here.
	if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);
		BbpData &= (~0x18);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpData);
		pAd->ate.TxWI.BW = BW_20;
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MODE_Proc::CCK Only support 20MHZ. Switch to 20MHZ.\n"));
	}
	
#ifdef RT30xx	
	if (IS_RT30xx(pAd))
	{
	// Set BW
	if (pAd->ate.TxWI.BW == BW_40)
	{
		calRFValue = pAd->Mlme.CaliBW40RfR24;
	}
	else
	{
		calRFValue = pAd->Mlme.CaliBW20RfR24;
	}
				
	RT30xxReadRFRegister(pAd, RF_R24, (PUCHAR)(&RFValue));
	calRFValue = (RFValue & 0xC0) | (calRFValue & ~0xC0); // <bit 5>:tx_h20M<bit 5> and <bit 4:0>:tx_agc_fc<bit 4:0>		
#ifdef RT3390
	if (IS_RT3390(pAd))
	{
		/*R24, BW=20M*/
		if(pAd->ate.TxWI.PHYMODE == MODE_CCK && pAd->ate.TxWI.BW == BW_20)
			calRFValue |= 0x0F;
		else
			calRFValue |= 0x08;
	}
#endif // RT3390 //
	RT30xxWriteRFRegister(pAd, RF_R24, calRFValue);
	}
#endif // RT30xx //

#ifdef RT53xx
	if (IS_RT5390F(pAd) || IS_RT5390H(pAd))
	{
		if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x46);
		else			
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x43);
	}

	if (IS_RT5392(pAd))
	{
		if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
		{
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0xF8);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, 0xC0);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x47);
		}
		else
		{
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, 0x20);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x43);	
		}
	}
#endif // RT53xx //

#ifdef RT33xx
	if (IS_RT3390(pAd))
		RTMP_TxEvmCalibration(pAd);
#endif // RT33XX //

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MODE_Proc (TxMode = %d)\n", pAd->ate.TxWI.PHYMODE));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MODE_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame GI
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_TX_GI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->ate.TxWI.ShortGI = simple_strtol(arg, 0, 10);

	if (pAd->ate.TxWI.ShortGI > 1)
	{
		pAd->ate.TxWI.ShortGI = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_GI_Proc::Out of range\n"));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_GI_Proc (GI = %d)\n", pAd->ate.TxWI.ShortGI));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_GI_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


INT	Set_ATE_RX_FER_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->ate.bRxFER = simple_strtol(arg, 0, 10);

	if (pAd->ate.bRxFER == 1)
	{
		pAd->ate.RxCntPerSec = 0;
		pAd->ate.RxTotalCnt = 0;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_FER_Proc (bRxFER = %d)\n", pAd->ate.bRxFER));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_FER_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	
	return TRUE;
}


INT Set_ATE_Read_RF_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef RTMP_RF_RW_SUPPORT
	/* modify by WY for Read RF Reg. error */
	UCHAR RFValue;
	INT index=0;

//2008/07/10:KH add to support RT30xx ATE<--
	if (IS_RT30xx(pAd) || IS_RT3390(pAd) || IS_RT3572(pAd))
	{
		for (index = 0; index < 32; index++)
		{
			ATE_RF_IO_READ8_BY_REG_ID(pAd, index, (PUCHAR)&RFValue);
			DBGPRINT(RT_DEBUG_OFF, ("R%d=%d\n",index,RFValue));
		}		
	}
	else
//2008/07/10:KH add to support RT30xx ATE-->
#endif // RTMP_RF_RW_SUPPORT //
	{
		DBGPRINT(RT_DEBUG_OFF, ("R1 = %x\n", pAd->LatchRfRegs.R1));
		DBGPRINT(RT_DEBUG_OFF, ("R2 = %x\n", pAd->LatchRfRegs.R2));
		DBGPRINT(RT_DEBUG_OFF, ("R3 = %x\n", pAd->LatchRfRegs.R3));
		DBGPRINT(RT_DEBUG_OFF, ("R4 = %x\n", pAd->LatchRfRegs.R4));
	}
	return TRUE;
}


#ifndef RTMP_RF_RW_SUPPORT
INT Set_ATE_Write_RF1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);	

	if (IS_RT30xx(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd))
	{
		DBGPRINT(RT_DEBUG_OFF, ("Warning!! RT3xxx Don't Support !\n"));
		return FALSE;
	}
	else
	{
		pAd->LatchRfRegs.R1 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}


INT Set_ATE_Write_RF2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

	if (IS_RT30xx(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd))
	{
		DBGPRINT(RT_DEBUG_OFF, ("Warning!! RT3xxx Don't Support !\n"));
		return FALSE;
	}
	else
	{
		pAd->LatchRfRegs.R2 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}


INT Set_ATE_Write_RF3_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

	if (IS_RT30xx(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd))
	{
		DBGPRINT(RT_DEBUG_OFF, ("Warning!! RT3xxx Don't Support !\n"));
		return FALSE;
	}
	else
	{
		pAd->LatchRfRegs.R3 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}


INT Set_ATE_Write_RF4_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

	if (IS_RT30xx(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd))
	{
		ate_print("Warning!! RT3xxx Don't Support !\n");
		return FALSE;
	}
	else
	{
		pAd->LatchRfRegs.R4 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}
#endif // RTMP_RF_RW_SUPPORT //


/* 
==========================================================================
    Description:
        Load and Write EEPROM from a binary file prepared in advance.
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT Set_ATE_Load_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN		    	ret = FALSE;
	PSTRING			src = EEPROM_BIN_FILE_NAME;
	RTMP_OS_FD		srcf;
	INT32 			retval;
	USHORT 			WriteEEPROM[(EEPROM_SIZE/2)];
	INT				FileLength = 0;
	UINT32 			value = (UINT32) simple_strtol(arg, 0, 10);
	RTMP_OS_FS_INFO	osFSInfo;

	DBGPRINT(RT_DEBUG_ERROR, ("===> %s (value=%d)\n\n", __FUNCTION__, value));

	if (value > 0)
	{
		/* zero the e2p buffer */
		NdisZeroMemory((PUCHAR)WriteEEPROM, EEPROM_SIZE);

		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		do
		{
			/* open the bin file */
			srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(srcf)) 
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s - Error opening file %s\n", __FUNCTION__, src));
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(srcf, (PSTRING)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: error file length (=%d) in e2p.bin\n",
					   __FUNCTION__, FileLength));
				break;
			}
			else
			{
				/* write the content of .bin file to EEPROM */
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)
                {
                    USHORT index=0;
                    USHORT value=0;

                    INT32 e2p_size=512;/* == 0x200 for PCI interface */
                    USHORT tempData=0;
                    for (index = 0 ; index < (e2p_size / 2); )
                    {
                        /* "value" is especially for some compilers... */
                        tempData = le2cpu16(WriteEEPROM[index]);
                        value = tempData;
                        RT28xx_EEPROM_WRITE16(pAd, index*2, value);
                        index ++;
                    }
                }
#else
				rt_ee_write_all(pAd, WriteEEPROM);
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
				ret = TRUE;
			}
			break;
		} while(TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(srcf))
		{
				;
		}
		else
		{
			retval = RtmpOSFileClose(srcf);			

			if (retval)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("--> Error %d closing %s\n", -retval, src));
				
			} 
		}

		/* restore */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);		
	}

    DBGPRINT(RT_DEBUG_ERROR, ("<=== %s (ret=%d)\n", __FUNCTION__, ret));

    return ret;
	
}




INT Set_ATE_Read_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT buffer[EEPROM_SIZE/2];
	USHORT *p;
	int i;
	
	rt_ee_read_all(pAd, (USHORT *)buffer);
	p = buffer;
	for (i = 0; i < (EEPROM_SIZE/2); i++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", *p));
		if (((i+1) % 16) == 0)
			DBGPRINT(RT_DEBUG_OFF, ("\n"));
		p++;
	}
	return TRUE;
}




/* 
==========================================================================
    Description:
        Enable ATE auto Tx alc (Tx auto level control).
        According to the chip temperature, auto adjust the transmit power.  
        
        0: disable
        1: enable
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_AUTO_ALC_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32 value = simple_strtol(arg, 0, 10);

	if (value > 0)
	{
		pAd->ate.bAutoTxAlc = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = TRUE , auto alc enabled!\n"));
	}
	else
	{
		pAd->ate.bAutoTxAlc = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("ATEAUTOALC = FALSE , auto alc disabled!\n"));
	}	

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE Tx frame IPG
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_IPG_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
    UINT32           data, value;

	pAd->ate.IPG = simple_strtol(arg, 0, 10);
	value = pAd->ate.IPG;

	RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &data);

    if (value <= 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_IPG_Proc::IPG is disabled(IPG == 0).\n"));
		return TRUE;
	}

	ASSERT(value > 0);

    if ((value > 0) && (value < 256))
    {               
        RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &data);
        data &= 0x0;
        RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, data);

        RTMP_IO_READ32(pAd, EDCA_AC1_CFG, &data);
        data &= 0x0;
        RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, data);

        RTMP_IO_READ32(pAd, EDCA_AC2_CFG, &data);
        data &= 0x0;
        RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, data);

        RTMP_IO_READ32(pAd, EDCA_AC3_CFG, &data);
        data &= 0x0;
        RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, data);
    }
    else
    {
        UINT32 aifsn, slottime;

        RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &slottime);
        slottime &= 0x000000FF;

        aifsn = value / slottime;                  
        value = value % slottime;

        RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &data);
        data &= 0x0;
        data |= (aifsn << 8);
        RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, data);

        RTMP_IO_READ32(pAd, EDCA_AC1_CFG, &data);
        data &= 0x0;
        data |= (aifsn << 8);
        RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, data);

        RTMP_IO_READ32(pAd, EDCA_AC2_CFG, &data);
        data &= 0x0;
        data |= (aifsn << 8);
        RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, data);

        RTMP_IO_READ32(pAd, EDCA_AC3_CFG, &data);
        data &= 0x0;
        data |= (aifsn << 8);
        RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, data);
    }

    data = (value & 0xFFFF0000) | value | (value << 8);
	RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, data);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_IPG_Proc (IPG = %u)\n", pAd->ate.IPG));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_IPG_Proc Success\n"));
	
	return TRUE;
}


/* 
==========================================================================
    Description:
        Set ATE payload pattern for TxFrame
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_Payload_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PSTRING				value;

	value = arg;

	/* only one octet acceptable */	
	if (strlen(value) != 2)  
		return FALSE;

	AtoH(value, &(pAd->ate.Payload), 1);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_Payload_Proc (repeated pattern = 0x%2x)\n", pAd->ate.Payload));

#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Payload_Proc Success\n"));
	
	return TRUE;
}


INT	Set_ATE_Show_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	DBGPRINT(RT_DEBUG_OFF, ("Mode=%u\n", pAd->ate.Mode));
	DBGPRINT(RT_DEBUG_OFF, ("TxPower0=%d\n", pAd->ate.TxPower0));
	DBGPRINT(RT_DEBUG_OFF, ("TxPower1=%d\n", pAd->ate.TxPower1));
	DBGPRINT(RT_DEBUG_OFF, ("TxAntennaSel=%d\n", pAd->ate.TxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("RxAntennaSel=%d\n", pAd->ate.RxAntennaSel));
	DBGPRINT(RT_DEBUG_OFF, ("BBPCurrentBW=%u\n", pAd->ate.TxWI.BW));
	DBGPRINT(RT_DEBUG_OFF, ("GI=%u\n", pAd->ate.TxWI.ShortGI));
	DBGPRINT(RT_DEBUG_OFF, ("MCS=%u\n", pAd->ate.TxWI.MCS));
	DBGPRINT(RT_DEBUG_OFF, ("TxMode=%u\n", pAd->ate.TxWI.PHYMODE));
	DBGPRINT(RT_DEBUG_OFF, ("Addr1=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pAd->ate.Addr1[0], pAd->ate.Addr1[1], pAd->ate.Addr1[2], pAd->ate.Addr1[3], pAd->ate.Addr1[4], pAd->ate.Addr1[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr2=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pAd->ate.Addr2[0], pAd->ate.Addr2[1], pAd->ate.Addr2[2], pAd->ate.Addr2[3], pAd->ate.Addr2[4], pAd->ate.Addr2[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Addr3=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pAd->ate.Addr3[0], pAd->ate.Addr3[1], pAd->ate.Addr3[2], pAd->ate.Addr3[3], pAd->ate.Addr3[4], pAd->ate.Addr3[5]));
	DBGPRINT(RT_DEBUG_OFF, ("Channel=%u\n", pAd->ate.Channel));
	DBGPRINT(RT_DEBUG_OFF, ("TxLength=%u\n", pAd->ate.TxLength));
	DBGPRINT(RT_DEBUG_OFF, ("TxCount=%u\n", pAd->ate.TxCount));
	DBGPRINT(RT_DEBUG_OFF, ("RFFreqOffset=%u\n", pAd->ate.RFFreqOffset));
	DBGPRINT(RT_DEBUG_OFF, ("IPG=%u\n", pAd->ate.IPG));
	DBGPRINT(RT_DEBUG_OFF, ("Payload=0x%02x\n", pAd->ate.Payload));

	DBGPRINT(RT_DEBUG_OFF, ("Set_ATE_Show_Proc Success\n"));
#ifdef CONFIG_AP_SUPPORT
#endif // CONFIG_AP_SUPPORT //
	return TRUE;
}


INT	Set_ATE_Help_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	DBGPRINT(RT_DEBUG_OFF, ("ATE=ATESTART, ATESTOP, TXCONT, TXCARR, TXFRAME, RXFRAME\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEDA\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATESA\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEBSSID\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATECHANNEL, range:0~14(unless A band !)\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXPOW0, set power level of antenna 1.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXPOW1, set power level of antenna 2.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXANT, set TX antenna. 0:all, 1:antenna one, 2:antenna two.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERXANT, set RX antenna.0:all, 1:antenna one, 2:antenna two, 3:antenna three.\n"));
#ifdef RTMP_RF_RW_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATETXFREQOFFSET, set frequency offset, range 0~95\n"));
#else
	DBGPRINT(RT_DEBUG_OFF, ("ATETXFREQOFFSET, set frequency offset, range 0~63\n"));
#endif // RTMP_RF_RW_SUPPORT //
	DBGPRINT(RT_DEBUG_OFF, ("ATETXBW, set BandWidth, 0:20MHz, 1:40MHz.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXLEN, set Frame length, range 24~%d\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXCNT, set how many frame going to transmit.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXMCS, set MCS, reference to rate table.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXMODE, set Mode 0:CCK, 1:OFDM, 2:HT-Mix, 3:GreenField, reference to rate table.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATETXGI, set GI interval, 0:Long, 1:Short\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERXFER, 0:disable Rx Frame error rate. 1:enable Rx Frame error rate.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERRF, show all RF registers.\n"));
#ifndef RTMP_RF_RW_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF1, set RF1 register.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF2, set RF2 register.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF3, set RF3 register.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEWRF4, set RF4 register.\n"));
#endif // !RTMP_RF_RW_SUPPORT //
	DBGPRINT(RT_DEBUG_OFF, ("ATELDE2P, load EEPROM from .bin file.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATERE2P, display all EEPROM content.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEIPG, set ATE Tx frame IPG.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEPAYLOAD, set ATE payload pattern for TxFrame.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATESHOW, display all parameters of ATE.\n"));
	DBGPRINT(RT_DEBUG_OFF, ("ATEHELP, online help.\n"));

	return TRUE;
}


#if defined( RT35xx)  || defined(RT53xx)

#ifdef RT53xx
#define	TXPowerEnMask		0xA8;
#define	RXPowerEnMask		0x54;
#endif // RT53xx //
 
VOID ATEAsicSetTxRxPath(
    IN PRTMP_ADAPTER pAd)
{	
	UCHAR BbpValue = 0, RFValue = 0;

#ifdef RT53xx
	if (IS_RT5392(pAd))
	{
		RFValue = 0x03;

		//Set TX path, pAd->TxAntennaSel : 0 -> All, 1 -> TX0, 2 -> TX1
		switch(pAd->Antenna.field.TxPath)
		{
			case 2:
				switch (pAd->ate.TxAntennaSel)
				{
					case 1:
						//set BBP R1, bit 4:3 = 00
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;		//11100111B
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						//set RF R1, bit 7:5:3 = 001					
						RFValue &=  ~TXPowerEnMask;
						RFValue = RFValue | 0x08;
						break;
					case 2:	
						//set BBP R1, bit 4:3 = 01
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;	
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						//set RF R1, bit 7:5:3 = 010					
						RFValue &=  ~TXPowerEnMask;
						RFValue = RFValue | 0x20;
						break;
					default:		
						//set BBP R1, bit 4:3 = 10
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						//set RF R1, bit 7:5:3 = 011
						RFValue &=  ~TXPowerEnMask;
						RFValue = RFValue | 0x28;
						break;
				}
				break;

			default:
				//set RF R1, bit 7:5:3 = 011
				RFValue &=  ~TXPowerEnMask;
				RFValue = RFValue | 0x28;
				break;
		}


		//Set RX path, pAd->RxAntennaSel : 0 -> All, 1 -> RX0, 2 -> RX1, 3 -> RX2
		switch (pAd->Antenna.field.RxPath)
		{
			case 3:
				switch (pAd->ate.RxAntennaSel)
				{
					case 1:
						//set BBP R3, bit 4:3:1:0 = 0000							
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x00;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

						//set RF R1, bit 6:4:2 = 110
						ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
						RFValue = RFValue & 0xAB;
						RFValue = RFValue | 0x50;
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
						break;
					case 2:
						//set BBP R3, bit 4:3:1:0 = 0001								
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x01;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									

						//set RF R1, bit 6:4:2 = 101
						ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
						RFValue = RFValue & 0xAB;
						RFValue = RFValue | 0x44;
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
						break;
					case 3:	
						//set BBP R3, bit 4:3:1:0 = 0002								
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x02;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

						//set RF R1, bit 6:4:2 = 011
						ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
						RFValue = RFValue & 0xAB;
						RFValue = RFValue | 0x14;
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
						break;								
					default:	
						//set BBP R3, bit 4:3:1:0 = 1000
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

						//set RF R1, bit 6:4:2 = 000
						//RT30xxReadRFRegister(pAd, RF_R01, (PUCHAR)&Value);
						//Value = Value & 0xAB;
						//RT30xxWriteRFRegister(pAd, RF_R01, (UCHAR)Value);
						break;
				}
				break;

			case 2:						
				switch (pAd->ate.RxAntennaSel)
				{
					case 1:	
						//set BBP R3, bit 4:3:1:0 = 0000		
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x00;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

						//set RF R1, bit 6:4:2 = 001				
						RFValue &=  ~RXPowerEnMask;
						RFValue |=  0x04;
						break;
					case 2:								
						//set BBP R3, bit 4:3:1:0 = 0001
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x01;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

						//set RF R1, bit 6:4:2 = 010
						RFValue &=  ~RXPowerEnMask;
						RFValue |= 0x10;
						break;							
					default:	
						//set BBP R3, bit 4:3:1:0 = 0100		
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

						//set RF R1, bit 6:4:2 = 011
						RFValue &=  ~RXPowerEnMask;
						RFValue |= 0x14;
						break;						
				}
				break;

			default:
				//set RF R1, bit 6:4:2 = 011
				RFValue &=  ~RXPowerEnMask;
				RFValue |= 0x14;			
				break;		
		}

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);
	}
#endif // RT53xx //	
}
#endif // defined( RT35xx)  || defined(RT53xx) //


/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for ATE.
    
==========================================================================
*/

VOID ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd) 
{
	UINT32 /*R2 = 0, R3 = DEFAULT_RF_TX_POWER, R4 = 0,*/ Value = 0;
	/*RTMP_RF_REGS *RFRegTable = NULL;*/
	UCHAR index = 0, BbpValue = 0, R66 = 0x30, Channel = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
#ifdef RTMP_RF_RW_SUPPORT
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0/*, RFValue2 = 0*/;
#endif // RTMP_RF_RW_SUPPORT //
#ifdef RT53xx
	UCHAR TxRxh20M=0;
#endif // RT53xx //

#ifdef RALINK_QA
	// for QA mode, TX power values are passed from UI
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)			
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif // RALINK_QA //
	Channel = pAd->ate.Channel;

#ifdef ANT_DIVERSITY_SUPPORT
	// select antenna for RT3090
	AsicAntennaSelect(pAd, Channel);
#endif // ANT_DIVERSITY_SUPPORT //

	// fill Tx power value
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;

#ifdef RT30xx
//2008/07/10:KH add to support 3070 ATE<--

	/*
		The RF programming sequence is difference between 3xxx and 2xxx.
		The 3070 is 1T1R. Therefore, we don't need to set the number of Tx/Rx path
		and the only job is to set the parameters of channels.
	*/
	if ((IS_RT30xx(pAd) || IS_RT3390(pAd)) && 
		((pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_2020) ||
		(pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022) || (pAd->RfIcType == RFIC_3320)))
	{
		/* modify by WY for Read RF Reg. error */

		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
				// Programming channel parameters.
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, FreqItems3020[index].N);

				//ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, FreqItems3020[index].K);
	                                ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
        	                        RFValue = (RFValue & 0xF0) | (FreqItems3020[index].K&~0xF0);
                	                ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);
                               
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xFC) | FreqItems3020[index].R;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);

				if (IS_RT3390(pAd))
				{
					// Set Tx Power.
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
					RFValue = ((RFValue & 0xE0) | pAd->TxPowerCtrl.RF_TX0_ALC);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);
				}
				else
				{
					/* Set Tx0 Power */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
					RFValue = ((RFValue & 0xE0) | TxPwer);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

					/* Set Tx1 Power */
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R13, (PUCHAR)&RFValue);
					RFValue = ((RFValue & 0xE0) | TxPwer2);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);
				}

				// Set RF offset.
#if defined(RT5390) || defined(RT5370)
				if (IS_RT5390(pAd))
				{
					RT30xxReadRFRegister(pAd, RF_R23, (PUCHAR)&RFValue);
					pAd->PreRFXCodeValue = (UCHAR)RFValue;
					RFValue = ((RFValue & ~0x7F) | (pAd->RfFreqOffset& 0x7F)); // xo_code (C1 value control) - Crystal calibration
					RFValue = min(RFValue, 0x5F);
#ifdef RTMP_MAC_PCI
					RFMultiStepXoCode(pAd, RF_R23, (UCHAR)RFValue,pAd->PreRFXCodeValue);	
#endif /* RTMP_MAC_PCI */
				}
				else
#endif /* defined(RT5390) || defined(RT5370) */
				{
					RT30xxReadRFRegister(pAd, RF_R23, &RFValue);
					RFValue = (RFValue & 0x80) | pAd->RfFreqOffset;
					RT30xxWriteRFRegister(pAd, RF_R23, RFValue);
				}

				// Set BW.
				if (pAd->ate.TxWI.BW == BW_40)
				{
					RFValue = pAd->Mlme.CaliBW40RfR24;
//					DISABLE_11N_CHECK(pAd);
				}
				else
				{
					RFValue = pAd->Mlme.CaliBW20RfR24;
#ifdef RT3390
					if (IS_RT3390(pAd))
					{
						if(pAd->ate.TxWI.PHYMODE == MODE_CCK)
							RFValue |= 0x0F;
						else
							RFValue &= 0xF8;
					}
#endif // RT3390 //					
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);

				// Enable RF tuning
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R07, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x1;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R07, (UCHAR)RFValue);

				// latch channel for future usage
				pAd->LatchRfRegs.Channel = Channel;
 				if (pAd->Antenna.field.RxPath >= 2)
	                     {

					  // antenna selection
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
	   
					RFValue = (RFValue & ~(0x17)) | 0xC1;
					BbpValue &= 0xE4;
					if (pAd->ate.RxAntennaSel == 1)
					{
						RFValue = RFValue | 0x10;
						BbpValue |= 0x00;

					}
					else if (pAd->ate.RxAntennaSel == 2)
					{
						RFValue = RFValue | 0x04;
						BbpValue |= 0x01;
					}
					else
					{
						 /* Only enable two Antenna to receive. */
	                                     BbpValue |= 0x0B;
					}
					
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);  
	                     }
				if (pAd->Antenna.field.TxPath >= 2)
				{

					  // antenna selection
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
				       ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
	   
					RFValue = (RFValue & ~(0x2B)) | 0xC1;
					BbpValue &= 0xE7;
					if (pAd->ate.TxAntennaSel == 1)
					{
						RFValue = RFValue | 0x20;
					}
					else if (pAd->ate.TxAntennaSel == 2)
					{
						RFValue = RFValue | 0x08;
						BbpValue |= 0x08;
					}
					else
					{
						BbpValue |= 0x10;
					}
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
	                     }
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);
				RFValue |= 0x80;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);
				RTMPusecDelay(1000);
				RFValue &= 0x7F;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

				break;				
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
			Channel, 
			pAd->RfIcType, 
			TxPwer,
			TxPwer2,
			pAd->Antenna.field.TxPath,
			FreqItems3020[index].N, 
			FreqItems3020[index].K, 
			FreqItems3020[index].R));
	}
	else
//2008/07/10:KH add to support 3070 ATE-->
#endif // RT30xx //
#ifdef RT53xx
	if (IS_RT5390(pAd))
	{	
		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{

			if (Channel == FreqItems3020[index].Channel)
			{
				//
				// Set the BBP Tx fine power control in 0.1dB step
				//
				// Programming channel parameters
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, FreqItems3020[index].N); // N
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, (FreqItems3020[index].K & 0x0F)); // K, N<11:8> is set to zero

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R11, (PUCHAR)&RFValue);
				RFValue = ((RFValue & ~0x03) | (FreqItems3020[index].R & 0x03)); // R
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)&RFValue);
				RFValue = ((RFValue & ~0x3F) | (TxPwer & 0x3F)); // tx0_alc
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue);

				if (IS_RT5392(pAd))
				{
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R50, &RFValue);
					RFValue = ((RFValue & ~0x3F) | (TxPwer2 & 0x3F)); // tx0_alc
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R50, RFValue);
				}

				/* zero patch based on windows driver */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, &RFValue);
				if (IS_RT5392(pAd))
				{
					RFValue = ((RFValue & ~0x3F) | 0x3F);
				}
				else
				{
					RFValue = ((RFValue & ~0x0F) | 0x0F); // Enable rf_block_en, pll_en, rx0_en and tx0_en
				}	
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, RFValue);

				if (!IS_RT5392(pAd))
				{
					ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R02,(PUCHAR)&RFValue);
					RFValue |= 0x80;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, (UCHAR)RFValue);
					
					RTMPusecDelay(1000);
					
					RFValue &= 0x7F;
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, (UCHAR)RFValue);
				}
				
				if (IS_RT5392(pAd))
				{
					ATEAsicSetTxRxPath(pAd);
				}

				UCHAR HighCurrentBit = 0;
				RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RFValue);
				pAd->ate.PreRFXCodeValue = (UCHAR)RFValue;
				HighCurrentBit = pAd->ate.PreRFXCodeValue & 0x80;
				RFValue = min((pAd->ate.RFFreqOffset & 0x7F), 0x5F);
				RFValue = HighCurrentBit | (RFValue & 0x7F);
				RFMultiStepXoCode(pAd, RF_R17, (UCHAR)RFValue, pAd->ate.PreRFXCodeValue);
					
				if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, 0xC0);

					if (IS_RT5390F(pAd) || IS_RT5390H(pAd))
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x46);
					}
					else if (IS_RT5392(pAd))
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0xF8);
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x47);
					}						
				}
				else
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, 0x80);
					
					if (IS_RT5390F(pAd) || IS_RT5390H(pAd))
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x43);
					}
					else if (IS_RT5392(pAd))
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, 0x20);
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, 0x43);
					}
				}

				if (pAd->ate.TxWI.BW == BW_20) // BW 20
				{
					//write BBP R4 value 0x40
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, 0x40);

					if (IS_RT5392(pAd))
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, 0x10);
				}
				else
				{
					//write BBP R4 value 0x50
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, 0x50);

					if (IS_RT5392(pAd))
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, 0x16);
				}

				if (  (pAd->ate.TxWI.BW == BW_40)) // BW 40
				{
					TxRxh20M = ((pAd->Mlme.CaliBW40RfR24 & 0x20) >> 5); // Tx/Rx h20M
				}
				else // BW 20
				{
					TxRxh20M = ((pAd->Mlme.CaliBW20RfR24 & 0x20) >> 5); // Tx/Rx h20M
				}

				/* Fix the throughput drop issue while STA 20MBW connection with AP 20/40M BW */
				if (IS_RT5392(pAd))
				{
					if (pAd->ate.TxWI.BW == BW_40)
					{
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x1C);
					}
					else
					{
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, 0x3C);
					}
				}
				
#ifdef RT5390
				if (IS_RT5392(pAd)) /* RT5392 */
				{
				}
				else if (IS_RT5390H(pAd)) /* RT5390H */
				{
					if ((Channel >= 1) && (Channel <= 3))
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x0F);
					}
					else if ((Channel >= 4) && (Channel <= 5))
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x0E);
					}
					else if (Channel == 6)
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x0C);
					}
					else if (Channel == 7)
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x0B);
					}
					else if ((Channel >= 8) && (Channel <= 11))
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x09);
					}
					else if ((Channel >= 12) && (Channel <= 13))
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x08);
					}
					else if (Channel == 14)
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x06);
					}
				}
				else if (IS_RT5390F(pAd)) // >= RT5390F
				{
					if ((Channel >= 1) && (Channel <= 10))
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x07); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if (Channel == 11)
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x06); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if (Channel == 12)
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x05); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if ((Channel >= 13) && (Channel <=14))
					{							
						RT30xxWriteRFRegister(pAd, RF_R59, 0x04); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
				}
				else if (IS_RT5390(pAd))
				{
					if ((Channel >= 1) && (Channel <= 7))
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x8F); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if (Channel == 8)
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x8D); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if (Channel == 9)
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x8A); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if ((Channel >= 10) && (Channel <= 11))
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x88); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if ((Channel >= 12) && (Channel <= 13))
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x87); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
					else if (Channel == 14)
					{
						ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, 0x86); // pa2_cc_ofdm<3:0> (PA2 Cascode Bias OFDM mode)
					}
				}
#endif // RT5390 //
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);
				RFValue = ((RFValue & ~0x06) | (TxRxh20M << 1) | (TxRxh20M << 2)); // tx_h20M and rx_h20M
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);
				RFValue = ((RFValue & ~0x18) | 0x10); // rxvcm (Rx BB filter VCM)
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
				RFValue = ((RFValue & ~0x80) | 0x80); // vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration.
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);					

				DBGPRINT(RT_DEBUG_TRACE, ("%s: 5390: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
					__FUNCTION__, 
					Channel, 
					pAd->RfIcType, 
					TxPwer, 
					TxPwer2, 
					pAd->ate.TxAntennaSel, 
					FreqItems3020[index].N, 
					FreqItems3020[index].K, 
					FreqItems3020[index].R));

				break;
			}
		}
	}
	else
#endif // RT53xx //

	{
#if defined(RT28xx) || defined(RT2880) || defined(RT2883)
		/* RT28xx */
		RFRegTable = RF2850RegTable;

		switch (pAd->RfIcType)
		{
#if defined(RT28xx) || defined(RT2880)
			/* But only 2850 and 2750 support 5.5GHz band... */
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:
#endif // defined(RT28xx) || defined(RT2880) //
				for (index = 0; index < NUM_OF_2850_CHNL; index++)
				{
					if (Channel == RFRegTable[index].Channel)
					{
						R2 = RFRegTable[index].R2;

						// If TX path is 1, bit 14 = 1;
						if (pAd->Antenna.field.TxPath == 1)
						{
							R2 |= 0x4000;	
						}
						if (pAd->Antenna.field.TxPath == 2)
						{
							if (pAd->ate.TxAntennaSel == 1)
							{
								// If TX Antenna select is 1 , bit 14 = 1; Disable Ant 2
								R2 |= 0x4000;	
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
								BbpValue &= 0xE7;		// 11100111B
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
							}
							else if (pAd->ate.TxAntennaSel == 2)
							{
								// If TX Antenna select is 2 , bit 15 = 1; Disable Ant 1
								R2 |= 0x8000;	
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
								BbpValue &= 0xE7;	
								BbpValue |= 0x08;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
							}
							else
							{
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
								BbpValue &= 0xE7;
								BbpValue |= 0x10;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
							}
						}

						if (pAd->Antenna.field.RxPath >= 2)	// ASUS EXT
						{
							switch (pAd->ate.RxAntennaSel)
							{
								case 1:
									R2 |= 0x20040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x00;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
									break;
								case 2:
									R2 |= 0x10040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x01;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									
									break;
								default:	
									R2 |= 0x40;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									/* Only enable two Antenna to receive. */
									BbpValue |= 0x08;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
									break;
							}
						}
						else if (pAd->Antenna.field.RxPath == 1)
						{
							// write 1 to off RxPath
							R2 |= 0x20040;	
						}

						if (pAd->Antenna.field.RxPath == 3)
						{
							switch (pAd->ate.RxAntennaSel)
							{
								case 1:
									R2 |= 0x20040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x00;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
									break;
								case 2:
									R2 |= 0x10040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x01;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);									
									break;
								case 3:	
									R2 |= 0x30000;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x02;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;								
								default:	
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x10;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								
									break;
							}
						}
						
						if (Channel > 14)
						{
							// initialize R3, R4
							R3 = (RFRegTable[index].R3 & 0xffffc1ff);
							R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15);

		                    /*
			                    According the Rory's suggestion to solve the middle range issue.

								5.5G band power range : 0xF9~0X0F, TX0 Reg3 bit9/TX1 Reg4 bit6="0"
														means the TX power reduce 7dB.
							*/
							// R3
							if ((TxPwer >= -7) && (TxPwer < 0))
							{
								TxPwer = (7+TxPwer);
								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
								R3 |= (TxPwer << 10);
								DBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer=%d \n", TxPwer));
							}
							else
							{
								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
								R3 |= (TxPwer << 10) | (1 << 9);
							}

							// R4
							if ((TxPwer2 >= -7) && (TxPwer2 < 0))
							{
								TxPwer2 = (7+TxPwer2);
								TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
								R4 |= (TxPwer2 << 7);
								DBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer2=%d \n", TxPwer2));
							}
							else
							{
								TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
								R4 |= (TxPwer2 << 7) | (1 << 6);
							}
						}
						else
						{
							// Set TX power0.
							R3 = (RFRegTable[index].R3 & 0xffffc1ff) | (TxPwer << 9);
							// Set frequency offset and TX power1.
							R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15) | (TxPwer2 <<6);

						}

						// based on BBP current mode before changing RF channel
						if (pAd->ate.TxWI.BW == BW_40)
						{
							R4 |=0x200000;
						}
						
						// Update variables.
						pAd->LatchRfRegs.Channel = Channel;
						pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
						pAd->LatchRfRegs.R2 = R2;
						pAd->LatchRfRegs.R3 = R3;
						pAd->LatchRfRegs.R4 = R4;

						RtmpRfIoWrite(pAd);
						
						break;
					}
				}
				break;

			default:
				break;
		}
#endif // defined(RT28xx) || defined(RT2880) || defined(RT2883) //
	}

	// Change BBP setting during switch from a->g, g->a
	if (Channel <= 14)
	{
	    UINT32 TxPinCfg = 0x00050F0A;// 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);//(0x44 - GET_LNA_GAIN(pAd)));	// According the Rory's suggestion to solve the middle range issue.
		//ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);

		/* For 1T/2R chip only... */
	    if (pAd->NicConfig2.field.ExternalLNAForG)
	    {
	        ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
	    }
	    else
	    {
	        ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x84);
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		// 5G band selection PIN, bit1 and bit2 are complement
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

        // Turn off unused PA or LNA when only 1T or 1R.
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		/* calibration power unbalance issues */
		if (pAd->Antenna.field.TxPath == 2)
		{
			if (pAd->ate.TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pAd->ate.TxAntennaSel == 2)
			{
				TxPinCfg &= 0xFFFFFFFD;
			}
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
	/* channel > 14 */
	else
	{
	    UINT32	TxPinCfg = 0x00050F05;// 2007.10.09 by Brian : 0x00050505 ==> 0x00050F05
		
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		{
		// According the Rory's suggestion to solve the middle range issue.
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);//(0x44 - GET_LNA_GAIN(pAd)));        
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0xF2);
		}

		// Rx High power VGA offset for LNA select
		if (pAd->NicConfig2.field.ExternalLNAForA)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R91, &BbpValue);
		ASSERT((BbpValue == 0x04));

		// 5 G band selection PIN, bit1 and bit2 are complement
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		// Turn off unused PA or LNA when only 1T or 1R.
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
#ifdef RT53xx
	if (IS_RT5390(pAd))
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38);
#endif /* RT53xx */

    // R66 should be set according to Channel and use 20MHz when scanning
	if (Channel <= 14)
	{	
		// BG band
		R66 = 0x2E + GET_LNA_GAIN(pAd);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
	}
	else
	{	
		// 5.5 GHz band
		if (pAd->ate.TxWI.BW == BW_20)
		{
			R66 = (UCHAR)(0x32 + (GET_LNA_GAIN(pAd)*5)/3);

	    		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
		else
		{
			R66 = (UCHAR)(0x3A + (GET_LNA_GAIN(pAd)*5)/3);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
	}

#if defined(RT5390)  || defined(RT5370)
#ifdef IQ_CAL_SUPPORT
	if (IS_RT5392(pAd))
		RT5392ATEIQCompensation(pAd, pAd->ate.Channel);	
	else if (IS_RT5390(pAd))
		RT5390ATEIQCompensation(pAd, pAd->ate.Channel);	
#endif /* IQ_CAL_SUPPORT */
#endif /* defined(RT5390)  || defined(RT5370) */

	/*
		On 11A, We should delay and wait RF/BBP to be stable
		and the appropriate time should be 1000 micro seconds. 

		2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.
	*/
	RTMPusecDelay(1000);  

#ifndef RTMP_RF_RW_SUPPORT
	if (Channel > 14)
	{
		// When 5.5GHz band the LSB of TxPwr will be used to reduced 7dB or not.
		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%u, Pwr1=%u, %dT) to , R1=0x%08x, R2=0x%08x, R3=0x%08x, R4=0x%08x\n",
								  Channel, 
								  pAd->RfIcType, 
								  (R3 & 0x00003e00) >> 9,
								  (R4 & 0x000007c0) >> 6,
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1, 
								  pAd->LatchRfRegs.R2, 
								  pAd->LatchRfRegs.R3, 
								  pAd->LatchRfRegs.R4));
    }
#endif // RTMP_RF_RW_SUPPORT //
}


/*
==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in ATE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. if current radio is a noisy environment (pAd->DrsCounters.fNoisyEnvironment)
		1. TxPowerPercentage
		2. auto calibration based on TSSI feedback
		3. extra 2 db for CCK
		4. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

	NOTE: Since this routine requires the value of (pAd->DrsCounters.fNoisyEnvironment),
		it should be called AFTER MlmeDynamicTxRateSwitching()
==========================================================================
*/
#if defined(RT5370) || defined(RT5390)
VOID RT539x_ATEAsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg8 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	if (IS_RT5390(pAd))
	{
		/* For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 */
		RTMP_IO_READ32(pAd, 0x1318, &ExtraPwrOverMAC);  
		ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for OFDM 54 */
		RTMP_IO_READ32(pAd, 0x131C, &ExtraPwrOverMAC);  
		ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) << 8; /* Get Tx power for HT MCS 7 */			
		RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

		/* For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC */
		RTMP_IO_READ32(pAd, 0x1324, &ExtraPwrOverMAC);  
		ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for STBC MCS 7 */
		RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);

		if (IS_RT5392(pAd))
		{	
			/*  For HT_MCS_15, extra fill the corresponding register value into MAC 0x13DC */
			RTMP_IO_READ32(pAd, 0x1320, &ExtraPwrOverMAC);  
			ExtraPwrOverTxPwrCfg8 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for HT MCS 15 */
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, ExtraPwrOverTxPwrCfg8);
			
			DBGPRINT(RT_DEBUG_TRACE, ("Offset =0x13D8, TxPwr = 0x%08X, ", (UINT)ExtraPwrOverTxPwrCfg8));
		}
		
		DBGPRINT(RT_DEBUG_TRACE, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
			(UINT)ExtraPwrOverTxPwrCfg7, 
			(UINT)ExtraPwrOverTxPwrCfg9));
	}
}
#endif /* defined(RT5370) || defined(RT5390) */

VOID ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	INT			i, j, maxTxPwrCnt;
	CHAR		DeltaPwr = 0, Value;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep, idx;
	PCHAR		pTxAgcCompensate;
	ULONG		TxPwr[7]; /* NOTE: the TxPwr array size should be the maxima value of all supported chipset!!!! */
	BOOLEAN		bAutoTxAgc = FALSE;
	BBP_R49_STRUC BbpR49;
#ifdef RTMP_INTERNAL_TX_ALC
	/* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */  
	extern TX_POWER_TUNING_ENTRY_STRUCT TxPowerTuningTable[];
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	UCHAR 		RFValue = 0;   
	CHAR 		desiredTSSI = 0, currentTSSI = 0, TotalDeltaPower = 0;
	CHAR 		TuningTableIndex0 = 0;
#endif /* RTMP_INTERNAL_TX_ALC */
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392)
	extern TX_POWER_TUNING_ENTRY_STRUCT TxPowerTuningTableOver5390[];
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry2 = NULL;
	CHAR 		TuningTableIndex2 = 0;
	UCHAR 		BbpValue = 0;
	INT 			CurrentTemp = 0;
	INT 			LookupTableIndex = pAd->TxPowerCtrl.LookupTableIndex + TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET;
	BOOLEAN 	bTempSuccess = FALSE;	
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */

	BbpR49.byte = 0;

		maxTxPwrCnt = 5;

	if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel > 14)
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx40MPwrCfgABand[i];	
			}
		}
		else
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx40MPwrCfgGBand[i];	
			}
		}
	}
	else
	{
		if (pAd->ate.Channel > 14)
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx20MPwrCfgABand[i];	
			}
		}
		else
		{
			for (i =0 ; i < maxTxPwrCnt; i ++)
			{
				TxPwr[i] = pAd->Tx20MPwrCfgGBand[i];	
			}
		}
	}

#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT5390
	/* Locate the internal Tx ALC tuning entry */
	if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE) && (IS_RT5390(pAd)))
	{
		if (ATEGetDesiredTssiAndCurrentTssi(pAd, &desiredTSSI, &currentTSSI) == FALSE)
		{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect desired TSSI or current TSSI\n", __FUNCTION__));
				
				/* Tx power adjustment over RF */
				RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)(&RFValue));
				RFValue = ((RFValue & ~0x3F) | pAd->TxPowerCtrl.RF_TX0_ALC);
				if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
				{
					RFValue = ((RFValue & ~0x3F) | 0x27);
				}
				RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)(RFValue));

				/* Tx power adjustment over MAC */
				TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;
		}
		else
		{
			if (desiredTSSI > currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable++;
			}

			if (desiredTSSI < currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable--;
			}

			TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPower[pAd->ate.Channel-1].Power;

			TuningTableIndex0 = (TuningTableIndex0 < LOWERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390) ? 
								LOWERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390 : TuningTableIndex0;
			TuningTableIndex0 = (TuningTableIndex0 > UPPERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390) ? 
								UPPERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390 : TuningTableIndex0;

			/* Valide pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 69 */
			pTxPowerTuningEntry = &TxPowerTuningTableOver5390[TuningTableIndex0 + TX_POWER_TUNING_ENTRY_OFFSET_OVER_5390]; 

			pAd->TxPowerCtrl.RF_TX0_ALC = pTxPowerTuningEntry->RF_TX0_ALC;
			pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

			/* Tx power adjustment over RF */
			RT30xxReadRFRegister(pAd, RF_R49, (PUCHAR)(&RFValue));
			RFValue = ((RFValue & ~0x3F) | pAd->TxPowerCtrl.RF_TX0_ALC);
			if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x1F */
			{
				RFValue = ((RFValue & ~0x3F) | 0x27);
			}
			RT30xxWriteRFRegister(pAd, RF_R49, (UCHAR)(RFValue));

			/* Tx power adjustment over MAC */
			TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, currentTSSI = %d, idxTxPowerTable = %d, TuningTableIndex = %d, {RF_TX0_ALC = 0x%x, MAC_PowerDelta = %d}\n", 
				__FUNCTION__, 
				desiredTSSI, 
				currentTSSI, 
				pAd->TxPowerCtrl.idxTxPowerTable, 
				TuningTableIndex0,
				pTxPowerTuningEntry->RF_TX0_ALC, 
				pTxPowerTuningEntry->MAC_PowerDelta));
		}
	}
#endif /* RT5390 */
#ifdef RT3390
	if (pAd->TxPowerCtrl.bInternalTxALC == TRUE && (IS_RT3390(pAd)))
	{
		desiredTSSI = ATEGetDesiredTSSI(pAd);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
		
		currentTSSI = BbpR49.field.TSSI;

		if (desiredTSSI > currentTSSI)
		{
			pAd->TxPowerCtrl.idxTxPowerTable++;
		}

		if (desiredTSSI < currentTSSI)
		{
			pAd->TxPowerCtrl.idxTxPowerTable--;
		}

		if (pAd->TxPowerCtrl.idxTxPowerTable < LOWERBOUND_TX_POWER_TUNING_ENTRY)
		{
			pAd->TxPowerCtrl.idxTxPowerTable = LOWERBOUND_TX_POWER_TUNING_ENTRY;
		}

		if (pAd->TxPowerCtrl.idxTxPowerTable > UPPERBOUND_TX_POWER_TUNING_ENTRY)
		{
			pAd->TxPowerCtrl.idxTxPowerTable = UPPERBOUND_TX_POWER_TUNING_ENTRY;
		}

		/* Valide pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 45 */
		pTxPowerTuningEntry = &TxPowerTuningTable[pAd->TxPowerCtrl.idxTxPowerTable + TX_POWER_TUNING_ENTRY_OFFSET];
		
		pAd->TxPowerCtrl.RF_TX0_ALC = pTxPowerTuningEntry->RF_TX0_ALC;
		pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

		/* Tx power adjustment over RF */
		RT30xxReadRFRegister(pAd, RF_R12, (PUCHAR)(&RFValue));
		RFValue = ((RFValue & 0xE0) | pAd->TxPowerCtrl.RF_TX0_ALC);
		RT30xxWriteRFRegister(pAd, RF_R12, (UCHAR)(RFValue));

		/* Tx power adjustment over MAC */
		TotalDeltaPower += pAd->TxPowerCtrl.MAC_PowerDelta;

		DBGPRINT(RT_DEBUG_TRACE, ("%s: desiredTSSI = %d, currentTSSI = %d, idxTxPowerTable = %d, {RF_TX0_ALC = 0x%x, MAC_PowerDelta = %d}\n", 
			__FUNCTION__, 
			desiredTSSI, 
			currentTSSI, 
			pAd->TxPowerCtrl.idxTxPowerTable, 
			pTxPowerTuningEntry->RF_TX0_ALC, 
			pTxPowerTuningEntry->MAC_PowerDelta));
	}
#endif /* RT3390 */
#endif /* RTMP_INTERNAL_TX_ALC */

	{
		if (pAd->ate.Channel <= 14)
		{
			/* bg channel */
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			TssiRef            = pAd->TssiRefG;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryG[0];
			TxAgcStep          = pAd->TxAgcStepG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			/* a channel */
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			TssiRef            = pAd->TssiRefA;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryA[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryA[0];
			TxAgcStep          = pAd->TxAgcStepA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
		{
#ifdef RT5390
			if (IS_RT5392(pAd))
			{
				BbpR49.byte = 0;
				
				/* If temperature compensation is enabled */
				if (pAd->CommonCfg.TempComp != 0)
				{
					/* For method 1, the value is set before and /after reading temperature */
					if (pAd->CommonCfg.TempComp == 1)
					{
						/* Set [7:6] = 1 */
						RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)(&RFValue));
						RFValue = (RFValue & 0x7f);
						RFValue = (RFValue | 0x40);
						RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

						/* Set BBP_R47[2] = 1 */
						RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpValue);
						BbpValue = (BbpValue | 0x04);
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpValue);

						RTMPusecDelay(1000);

						BbpValue = 0;
						RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpValue);

						/* If R47[2] == 0, it means reading temperature succeeds */
						if ((BbpValue & 0x04) == 0)
						{
							bTempSuccess = TRUE;

							/* Current temperature */
							RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
							CurrentTemp = (CHAR)BbpR49.byte;
							DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] BBP_R49 = %02x, current temp = %d\n", 
								BbpR49.byte, CurrentTemp));
						}
						else
						{
							bTempSuccess = FALSE;

							/* Set R47[2] = 0 */
							BbpValue = BbpValue & 0xfb;
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpValue);
						}

						/* Set [7:6] = 0 */
						RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)(&RFValue));
						RFValue = (RFValue & 0x3f);
						RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
					}
					else if (pAd->CommonCfg.TempComp == 2)
					{
						bTempSuccess = TRUE;

						/* Current temperature */
						RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);
						CurrentTemp = (CHAR)BbpR49.byte;
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] BBP_R49 = %02x, current temp = %d\n", 
							BbpR49.byte, CurrentTemp));
					}

					if (!bTempSuccess)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Fail to read temperature.\n"));
						return;
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] RefTempG = %d\n", pAd->TxPowerCtrl.RefTempG));
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] index = %d\n", pAd->TxPowerCtrl.LookupTableIndex));
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex - 1, 
							pAd->TxPowerCtrl.LookupTable[LookupTableIndex - 1]));
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex, 
							pAd->TxPowerCtrl.LookupTable[LookupTableIndex]));
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex + 1, 
							pAd->TxPowerCtrl.LookupTable[LookupTableIndex + 1]));
						if (CurrentTemp > pAd->TxPowerCtrl.RefTempG + pAd->TxPowerCtrl.LookupTable[LookupTableIndex + 1] + 
							((pAd->TxPowerCtrl.LookupTable[LookupTableIndex + 1] - pAd->TxPowerCtrl.LookupTable[LookupTableIndex]) >> 2) &&
							LookupTableIndex < 32)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ++\n"));
							LookupTableIndex++;
							pAd->TxPowerCtrl.LookupTableIndex++;
						}
						else if (CurrentTemp < pAd->TxPowerCtrl.RefTempG + pAd->TxPowerCtrl.LookupTable[LookupTableIndex] - ((pAd->TxPowerCtrl.LookupTable[LookupTableIndex] - pAd->TxPowerCtrl.LookupTable[LookupTableIndex - 1]) >> 2) &&
							LookupTableIndex > 0)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] --\n"));
							LookupTableIndex--;
							pAd->TxPowerCtrl.LookupTableIndex--;
						}
						else
						{
							DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ==\n"));
						}

						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] idxTxPowerTable %d, idxTxPowerTable2 %d\n",
							pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPowerCtrl.LookupTableIndex,
							pAd->TxPowerCtrl.idxTxPowerTable2 + pAd->TxPowerCtrl.LookupTableIndex));

						TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPowerCtrl.LookupTableIndex
											+ pAd->TxPower[pAd->ate.Channel-1].Power + TX_POWER_TUNING_ENTRY_OFFSET_OVER_5390;
						TuningTableIndex0 = (TuningTableIndex0 > UPPERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390) ?
											UPPERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390 : TuningTableIndex0;
						TuningTableIndex0 = (TuningTableIndex0 < LOWERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390) ? 
											LOWERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390 : TuningTableIndex0;
						pTxPowerTuningEntry = &TxPowerTuningTableOver5390[TuningTableIndex0];
						
						TuningTableIndex2 = pAd->TxPowerCtrl.idxTxPowerTable2 + pAd->TxPowerCtrl.LookupTableIndex 
											+ pAd->TxPower[pAd->ate.Channel-1].Power2 + TX_POWER_TUNING_ENTRY_OFFSET_OVER_5390;	
						TuningTableIndex2 = (TuningTableIndex2 > UPPERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390) ?
											UPPERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390 : TuningTableIndex2;
						TuningTableIndex2 = (TuningTableIndex2 < LOWERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390) ? 
											LOWERBOUND_TX_POWER_TUNING_ENTRY_OVER_5390 : TuningTableIndex2;					
						pTxPowerTuningEntry2 = &TxPowerTuningTableOver5390[TuningTableIndex2];				
						
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx0)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex=%d\n",
							pTxPowerTuningEntry->RF_TX0_ALC, pTxPowerTuningEntry->MAC_PowerDelta, TuningTableIndex0));
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx1)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex=%d\n",
							pTxPowerTuningEntry2->RF_TX0_ALC, pTxPowerTuningEntry2->MAC_PowerDelta, TuningTableIndex2));

						/* Update RF_R49 [0:5] */
						RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
						RFValue = ((RFValue & ~0x3F) | pTxPowerTuningEntry->RF_TX0_ALC);
						if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
						{
							RFValue = ((RFValue & ~0x3F) | 0x27);
						}
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Update RF_R49[0:5] to 0x%x\n", pTxPowerTuningEntry->RF_TX0_ALC));
						RT30xxWriteRFRegister(pAd, RF_R49, RFValue);

						/* Update RF_R50 [0:5] */
						RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
						RFValue = ((RFValue & ~0x3F) | pTxPowerTuningEntry2->RF_TX0_ALC);
						if ((RFValue & 0x3F) > 0x27) /* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
						{
							RFValue = ((RFValue & ~0x3F) | 0x27);
						}
						DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Update RF_R50[0:5] to 0x%x\n", pTxPowerTuningEntry2->RF_TX0_ALC));
						RT30xxWriteRFRegister(pAd, RF_R50, RFValue);

						TotalDeltaPower += pTxPowerTuningEntry->MAC_PowerDelta;
					}
				}
			}
			else
#endif /* RT5390 */
			{
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49.byte);

				/* TSSI representation */
				if (IS_RT3071(pAd) || IS_RT3390(pAd) || IS_RT3090A(pAd) || IS_RT3572(pAd)) /* 5-bits */
				{
					BbpR49.byte = (BbpR49.byte & 0x1F);
				}
				
				/* (p) TssiPlusBoundaryG[0] = 0 = (m) TssiMinusBoundaryG[0] */
				/* compensate: +4     +3   +2   +1    0   -1   -2   -3   -4 * steps */
				/* step value is defined in pAd->TxAgcStepG for tx power value */

				/* [4]+1+[4]   p4     p3   p2   p1   o1   m1   m2   m3   m4 */
				/* ex:         0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
				   above value are examined in mass factory production */
				/*             [4]    [3]  [2]  [1]  [0]  [1]  [2]  [3]  [4] */

				/* plus (+) is 0x00 ~ 0x45, minus (-) is 0xa0 ~ 0xf0 */
				/* if value is between p1 ~ o1 or o1 ~ s1, no need to adjust tx power */
				/* if value is 0xa5, tx power will be -= TxAgcStep*(2-1) */

				if (BbpR49.byte > pTssiMinusBoundary[1])
				{
					/* Reading is larger than the reference value */
					/* Check for how large we need to decrease the Tx power */
					for (idx = 1; idx < 5; idx++)
					{
						if (BbpR49.byte <= pTssiMinusBoundary[idx]) /* Found the range */
							break;
					}
					/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */
					*pTxAgcCompensate = -(TxAgcStep * (idx-1));	
					DeltaPwr += (*pTxAgcCompensate);
					DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
						                BbpR49.byte, TssiRef, TxAgcStep, idx-1));                    
				}
				else if (BbpR49.byte < pTssiPlusBoundary[1])
				{
					/* Reading is smaller than the reference value */
					/* Check for how large we need to increase the Tx power */
					for (idx = 1; idx < 5; idx++)
					{
						if (BbpR49.byte >= pTssiPlusBoundary[idx]) /* Found the range */
							break;
					}
					/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
					*pTxAgcCompensate = TxAgcStep * (idx-1);
					DeltaPwr += (*pTxAgcCompensate);
					DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
						                BbpR49.byte, TssiRef, TxAgcStep, idx-1));
				}
				else
				{
					*pTxAgcCompensate = 0;
					DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
						                BbpR49.byte, TssiRef, TxAgcStep, 0));
				}
			}
		}
	}

	/* 
		Calculate delta power based on the percentage specified from UI.
		E2PROM setting is calibrated for maximum TX power (i.e. 100%).
		We lower TX power here according to the percentage specified from UI.
	*/

	if (pAd->CommonCfg.TxPowerPercentage >= 100) /* AUTO TX POWER control */
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 90) /* 91 ~ 100% & AUTO, treat as 100% in terms of mW */
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60) /* 61 ~ 90%, treat as 75% in terms of mW		 DeltaPwr -= 1; */
	{
		DeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30) /* 31 ~ 60%, treat as 50% in terms of mW		 DeltaPwr -= 3; */
	{
		DeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15) /* 16 ~ 30%, treat as 25% in terms of mW		 DeltaPwr -= 6; */
	{
		DeltaPwr -= 6;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9) /* 10 ~ 15%, treat as 12.5% in terms of mW		 DeltaPwr -= 9; */
	{
		DeltaPwr -= 9;
	}
	else /* 0 ~ 9 %, treat as MIN(~3%) in terms of mW		 DeltaPwr -= 12; */
	{
		DeltaPwr -= 12;
	}

	/* Set new Tx power for different Tx rates */
	for (i=0; i<maxTxPwrCnt; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F);

#ifdef RTMP_INTERNAL_TX_ALC
				/* The upper bounds of MAC 0x1314 ~ 0x1324 are variable */
				if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE)
#if defined(RT5390) || defined(RT5370)
					^(IS_RT5392(pAd) && (pAd->bAutoTxAgcG) && (pAd->CommonCfg.TempComp != 0))
#endif /* defined(RT5390) || defined(RT5370) */
				)
				{
					switch (TX_PWR_CFG_0 + (i * 4))
					{
						case TX_PWR_CFG_0: 
						{
							if ((Value + TotalDeltaPower) < 0)
							{
								Value = 0;
							}
							else if ((Value + TotalDeltaPower) > 0xE)
							{
								Value = 0xE;
							}
							else
							{
								Value += TotalDeltaPower;
							}
						}
						break;

						case TX_PWR_CFG_1: 
						{
							if ((j >= 0) && (j <= 3))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_2: 
						{
							if ((j == 0) || (j == 2) || (j == 3))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_3: 
						{
							if ((j == 0) || (j == 2) || (j == 3) || 
							((j >= 4) && (j <= 7)))
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xC)
								{
									Value = 0xC;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
							else
							{
								if ((Value + TotalDeltaPower) < 0)
								{
									Value = 0;
								}
								else if ((Value + TotalDeltaPower) > 0xE)
								{
									Value = 0xE;
								}
								else
								{
									Value += TotalDeltaPower;
								}
							}
						}
						break;

						case TX_PWR_CFG_4: 
						{
							if ((Value + TotalDeltaPower) < 0)
							{
								Value = 0;
							}
							else if ((Value + TotalDeltaPower) > 0xC)
							{
								Value = 0xC;
							}
							else
							{
								Value += TotalDeltaPower;
							}
						}
						break;

						default: 
						{                                                      
							/* do nothing */
							DBGPRINT(RT_DEBUG_ERROR, ("%s: unknown register = 0x%X\n", __FUNCTION__, (TX_PWR_CFG_0 + (i * 4))));
						}
						break;
					}
				}
				else
#endif /* RTMP_INTERNAL_TX_ALC */
				{
					if ((Value + DeltaPwr) < 0)
					{
						Value = 0; /* min */
					}
					else if ((Value + DeltaPwr) > 0xF)
					{
						Value = 0xF; /* max */
					}
					else
					{
						Value += DeltaPwr; /* temperature compensation */
					}
				}

				/* Fill new value into the corresponding MAC offset */
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);

			DBGPRINT(RT_DEBUG_TRACE, ("%s: After compensation, Offset = 0x%04X, RegisterValue = 0x%08lX\n",
				__FUNCTION__, TX_PWR_CFG_0 + (i << 2), TxPwr[i]));			
		}
	}

#ifdef RT5390
	if (IS_RT5390(pAd))
	{
		/* Extra set MAC registers to compensate Tx power if any */
		RT539x_ATEAsicExtraPowerOverMAC(pAd);	
	}
#endif /* RT5390 */
}


/*
========================================================================
	Routine Description:
		Write TxWI for ATE mode.
		
	Return Value:
		None
========================================================================
*/
#ifdef RTMP_MAC_PCI
static VOID ATEWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC 	pOutTxWI,	
	IN	BOOLEAN			FRAG,	
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN 		AMPDU,
	IN	BOOLEAN 		Ack,
	IN	BOOLEAN 		NSeq,		// HW new a sequence.
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR 			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,	
	IN	BOOLEAN			CfAck,	
	IN	HTTRANSMIT_SETTING	*pTransmit)
{
	TXWI_STRUC 		TxWI;
	PTXWI_STRUC 	pTxWI;

	//
	// Always use Long preamble before verifiation short preamble functionality works well.
	// Todo: remove the following line if short preamble functionality works
	//
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWI_SIZE);
	pTxWI = &TxWI;

	pTxWI->FRAG= FRAG;

	pTxWI->CFACK = CFACK;
	pTxWI->TS= InsTimestamp;
	pTxWI->AMPDU = AMPDU;
	pTxWI->ACK = Ack;
	pTxWI->txop= Txopmode;
	
	pTxWI->NSEQ = NSeq;

	// John tune the performace with Intel Client in 20 MHz performance
	if ( BASize >7 )
		BASize =7;
		
	pTxWI->BAWinSize = BASize;	
	pTxWI->WirelessCliID = WCID;
	pTxWI->MPDUtotalByteCount = Length; 
	pTxWI->PacketId = PID; 
	
	// If CCK or OFDM, BW must be 20
	pTxWI->BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	pTxWI->ShortGI = pTransmit->field.ShortGI;
	pTxWI->STBC = pTransmit->field.STBC;
	
	pTxWI->MCS = pTransmit->field.MCS;
	pTxWI->PHYMODE = pTransmit->field.MODE;
	pTxWI->CFACK = CfAck;
	pTxWI->MIMOps = 0;
	pTxWI->MpduDensity = 0;

	pTxWI->PacketId = pTxWI->MCS;
	NdisMoveMemory(pOutTxWI, &TxWI, sizeof(TXWI_STRUC));

    return;
}
#endif // RTMP_MAC_PCI //




/*
========================================================================

	Routine Description:
		Disable protection for ATE.
========================================================================
*/
VOID ATEDisableAsicProtect(
	IN		PRTMP_ADAPTER	pAd)
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6];
	USHORT			offset;
	UCHAR			i;
	UINT32 MacReg = 0;

	// Config ASIC RTS threshold register
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;
	MacReg |= (0xFFF << 8);
	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	// Initial common protection settings
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	// Handle legacy(B/G) protection
	ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;
	ProtCfg.field.ProtectCtrl = 0;
	Protect[0] = ProtCfg.word;
	Protect[1] = ProtCfg.word;
	/* CTS-self is not used */
	pAd->FlgCtsEnabled = 0; 

	// NO PROTECT 
	// 1.All STAs in the BSS are 20/40 MHz HT
	// 2. in ai 20/40MHz BSS
	// 3. all STAs are 20MHz in a 20MHz BSS
	// Pure HT. no protection.

	// MM20_PROT_CFG
	//	Reserved (31:27)
	// 	PROT_TXOP(25:20) -- 010111
	//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
	//  PROT_CTRL(17:16) -- 00 (None)
	// 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
	Protect[2] = 0x01744004;	

	// MM40_PROT_CFG
	//	Reserved (31:27)
	// 	PROT_TXOP(25:20) -- 111111
	//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
	//  PROT_CTRL(17:16) -- 00 (None) 
	// 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
	Protect[3] = 0x03f44084;

	// CF20_PROT_CFG
	//	Reserved (31:27)
	// 	PROT_TXOP(25:20) -- 010111
	//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
	//  PROT_CTRL(17:16) -- 00 (None)
	// 	PROT_RATE(15:0)  -- 0x4004 (OFDM 24M)
	Protect[4] = 0x01744004;

	// CF40_PROT_CFG
	//	Reserved (31:27)
	// 	PROT_TXOP(25:20) -- 111111
	//	PROT_NAV(19:18)  -- 01 (Short NAV protection)
	//  PROT_CTRL(17:16) -- 00 (None)
	// 	PROT_RATE(15:0)  -- 0x4084 (duplicate OFDM 24M)
	Protect[5] = 0x03f44084;

	pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
	
	offset = CCK_PROT_CFG;
	for (i = 0;i < 6;i++)
		RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);

}




CHAR ATEConvertToRssi(
	IN PRTMP_ADAPTER pAd,
	IN	CHAR	Rssi,
	IN  UCHAR   RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;

	// Rssi equals to zero should be an invalid value
	if (Rssi == 0)
		return -99;
	
	LNAGain = GET_LNA_GAIN(pAd);
	if (pAd->LatchRfRegs.Channel > 14)
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->ARssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->ARssiOffset1;
		else
			RssiOffset = pAd->ARssiOffset2;
	}
	else
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->BGRssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->BGRssiOffset1;
		else
			RssiOffset = pAd->BGRssiOffset2;
	}

	return (-12 - RssiOffset - LNAGain - Rssi);
}


/*
========================================================================

	Routine Description:
		Set Japan filter coefficients if needed.
	Note:
		This routine should only be called when
		entering TXFRAME mode or TXCONT mode.
				
========================================================================
*/
static VOID SetJapanFilter(
	IN		PRTMP_ADAPTER	pAd)
{
	UCHAR			BbpData = 0;
	
	//
	// If Channel=14 and Bandwidth=20M and Mode=CCK, set BBP R4 bit5=1
	// (Japan Tx filter coefficients)when (TXFRAME or TXCONT).
	//
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);

    if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.Channel == 14) && (pAd->ate.TxWI.BW == BW_20))
    {
        BbpData |= 0x20;    // turn on
        DBGPRINT(RT_DEBUG_TRACE, ("SetJapanFilter!!!\n"));
    }
    else
    {
		BbpData &= 0xdf;    // turn off
		DBGPRINT(RT_DEBUG_TRACE, ("ClearJapanFilter!!!\n"));
    }

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpData);
}


VOID ATESampleRssi(
	IN PRTMP_ADAPTER	pAd,
	IN PRXWI_STRUC		pRxWI)
{
	if (pRxWI->RSSI0 != 0)
	{
		pAd->ate.LastRssi0	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI0, RSSI_0);
		pAd->ate.AvgRssi0X8	= (pAd->ate.AvgRssi0X8 - pAd->ate.AvgRssi0) + pAd->ate.LastRssi0;
		pAd->ate.AvgRssi0  	= pAd->ate.AvgRssi0X8 >> 3;
	}
	if (pRxWI->RSSI1 != 0)
	{
		pAd->ate.LastRssi1	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI1, RSSI_1);
		pAd->ate.AvgRssi1X8	= (pAd->ate.AvgRssi1X8 - pAd->ate.AvgRssi1) + pAd->ate.LastRssi1;
		pAd->ate.AvgRssi1	= pAd->ate.AvgRssi1X8 >> 3;
	}
	if (pRxWI->RSSI2 != 0)
	{
		pAd->ate.LastRssi2	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI2, RSSI_2);
		pAd->ate.AvgRssi2X8	= (pAd->ate.AvgRssi2X8 - pAd->ate.AvgRssi2) + pAd->ate.LastRssi2;
		pAd->ate.AvgRssi2	= pAd->ate.AvgRssi2X8 >> 3;
	}

	pAd->ate.LastSNR0 = (CHAR)(pRxWI->SNR0);// CHAR ==> UCHAR ?
	pAd->ate.LastSNR1 = (CHAR)(pRxWI->SNR1);// CHAR ==> UCHAR ?

	pAd->ate.NumOfAvgRssiSample ++;
}




/* 
==========================================================================
	Description:
		Setup Frame format.
	NOTE:
		This routine should only be used in ATE mode.
==========================================================================
*/
#ifdef RTMP_MAC_PCI
static INT ATESetUpFrame(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx)
{
	UINT j;
	PTXD_STRUC pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	PNDIS_PACKET pPacket;
	PUCHAR pDest;
	PVOID AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	HTTRANSMIT_SETTING	TxHTPhyMode;

	PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];
	PTXWI_STRUC pTxWI = (PTXWI_STRUC) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	PUCHAR pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;

#ifdef RALINK_QA
	PHEADER_802_11	pHeader80211;
#endif // RALINK_QA //

	if (pAd->ate.bQATxStart == TRUE) 
	{
		// always use QID_AC_BE and FIFO_EDCA

		// fill TxWI
		TxHTPhyMode.field.BW = pAd->ate.TxWI.BW;
		TxHTPhyMode.field.ShortGI = pAd->ate.TxWI.ShortGI;
		TxHTPhyMode.field.STBC = pAd->ate.TxWI.STBC;
		TxHTPhyMode.field.MCS = pAd->ate.TxWI.MCS;
		TxHTPhyMode.field.MODE = pAd->ate.TxWI.PHYMODE;

		ATEWriteTxWI(pAd, pTxWI, pAd->ate.TxWI.FRAG, pAd->ate.TxWI.CFACK,
			pAd->ate.TxWI.TS,  pAd->ate.TxWI.AMPDU, pAd->ate.TxWI.ACK, pAd->ate.TxWI.NSEQ, 
			pAd->ate.TxWI.BAWinSize, 0, pAd->ate.TxWI.MPDUtotalByteCount, pAd->ate.TxWI.PacketId, 0, 0,
			pAd->ate.TxWI.txop/*IFS_HTTXOP*/, pAd->ate.TxWI.CFACK/*FALSE*/, &TxHTPhyMode);
	}
	else
	{
		TxHTPhyMode.field.BW = pAd->ate.TxWI.BW;
		TxHTPhyMode.field.ShortGI = pAd->ate.TxWI.ShortGI;
		TxHTPhyMode.field.STBC = pAd->ate.TxWI.STBC;
		TxHTPhyMode.field.MCS = pAd->ate.TxWI.MCS;
		TxHTPhyMode.field.MODE = pAd->ate.TxWI.PHYMODE;
		ATEWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE,  FALSE, FALSE, FALSE, 
			4, 0, pAd->ate.TxLength, 0, 0, 0, IFS_HTTXOP, FALSE, &TxHTPhyMode);
	}
	
	// fill 802.11 header
#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE) 
	{
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE, pAd->ate.Header, pAd->ate.HLen);
	}
	else
#endif // RALINK_QA //
	{
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE, TemplateFrame, LENGTH_802_11);
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE+4, pAd->ate.Addr1, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE+10, pAd->ate.Addr2, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE+16, pAd->ate.Addr3, ETH_LENGTH_OF_ADDRESS);
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA)+TXWI_SIZE), DIR_READ, FALSE);
#endif // RT_BIG_ENDIAN //

	/* alloc buffer for payload */
#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE) 
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, pAd->ate.DLen + 0x100, FALSE, &AllocVa, &AllocPa);
	}
	else
#endif // RALINK_QA //
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, pAd->ate.TxLength, FALSE, &AllocVa, &AllocPa);
	}

	if (pPacket == NULL)
	{
		pAd->ate.TxCount = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("%s fail to alloc packet space.\n", __FUNCTION__));
		return -1;
	}
	pTxRing->Cell[TxIdx].pNextNdisPacket = pPacket;

	pDest = (PUCHAR) AllocVa;

#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE) 
	{
		GET_OS_PKT_LEN(pPacket) = pAd->ate.DLen;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pAd->ate.DLen;
#endif // LIMUX //
	}
	else
#endif // RALINK_QA //
	{
		GET_OS_PKT_LEN(pPacket) = pAd->ate.TxLength - LENGTH_802_11;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pAd->ate.TxLength - LENGTH_802_11;
#endif // LINUX //
	}

	// prepare frame payload
#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE) 
	{
		// copy pattern
		if ((pAd->ate.PLen != 0))
		{
			int j;
			
			for (j = 0; j < pAd->ate.DLen; j+=pAd->ate.PLen)
			{
				memcpy(GET_OS_PKT_DATAPTR(pPacket) + j, pAd->ate.Pattern, pAd->ate.PLen);
			}
		}
	}
	else
#endif // RALINK_QA //
	{
		for(j = 0; j < GET_OS_PKT_LEN(pPacket); j++)
			/* default payload is 0xA5 */
			pDest[j] = pAd->ate.Payload;/*kurtis: 0xAA ATE test EVM will be positive*/
	}

	/* build Tx Descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#else
    pDestTxD  = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
    TxD = *pDestTxD;
    pTxD = &TxD;
#endif // !RT_BIG_ENDIAN //

#ifdef RALINK_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		// prepare TxD
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		// build TX DESC
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWI_SIZE + pAd->ate.HLen;
		pTxD->LastSec0 = 0;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = GET_OS_PKT_LEN(pPacket);
		pTxD->LastSec1 = 1;

		pDest = (PUCHAR)pTxWI;
		pDest += TXWI_SIZE;
		pHeader80211 = (PHEADER_802_11)pDest;
		
		// modify sequence number...
		if (pAd->ate.TxDoneCount == 0)
		{
			pAd->ate.seq = pHeader80211->Sequence;
		}
		else
			pHeader80211->Sequence = ++pAd->ate.seq;
	}
	else
#endif // RALINK_QA //
	{
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		// build TX DESC
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow (pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWI_SIZE + LENGTH_802_11;
		pTxD->LastSec0 = 0;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = GET_OS_PKT_LEN(pPacket);
		pTxD->LastSec1 = 1;
	}

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange((PUCHAR)pTxWI, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA)+TXWI_SIZE), DIR_WRITE, FALSE);
    RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
    WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif // RT_BIG_ENDIAN //

	return 0;
}

/*=======================End of RTMP_MAC_PCI =======================*/
#endif // RTMP_MAC_PCI //




VOID rt_ee_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT i;
	USHORT value;

#ifdef RTMP_FLASH_SUPPORT
	{
		rtmp_ee_flash_read_all(pAd, Data);
		return;
	}
#endif /* RTMP_FLASH_SUPPORT */

	for (i = 0 ; i < EEPROM_SIZE/2 ; )
	{
		/* "value" is especially for some compilers... */
		RT28xx_EEPROM_READ16(pAd, i*2, value);
		Data[i] = value;
		i++;
	}
}


VOID rt_ee_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT i;
	USHORT value;


#ifdef RTMP_FLASH_SUPPORT
	{
		rtmp_ee_flash_write_all(pAd, Data);
		return;
	}
#endif /* RTMP_FLASH_SUPPORT */


	for (i = 0 ; i < EEPROM_SIZE/2 ; )
	{
		/* "value" is especially for some compilers... */
		value = Data[i];
		RT28xx_EEPROM_WRITE16(pAd, i*2, value);
		i++;
	}
}


#ifdef RALINK_QA
VOID ATE_QA_Statistics(
	IN PRTMP_ADAPTER			pAd,
	IN PRXWI_STRUC				pRxWI,
	IN PRT28XX_RXD_STRUC		pRxD,
	IN PHEADER_802_11			pHeader)
{

	// update counter first
	if (pHeader != NULL)
	{
		if (pHeader->FC.Type == BTYPE_DATA)
		{
			if (pRxD->U2M)
			{
				pAd->ate.U2M++;
			}
			else
				pAd->ate.OtherData++;
		}
		else if (pHeader->FC.Type == BTYPE_MGMT)
		{
			if (pHeader->FC.SubType == SUBTYPE_BEACON)
				pAd->ate.Beacon++;
			else
				pAd->ate.OtherCount++;
		}
		else if (pHeader->FC.Type == BTYPE_CNTL)
		{
			pAd->ate.OtherCount++;
		}
	}
	pAd->ate.RSSI0 = pRxWI->RSSI0; 
	pAd->ate.RSSI1 = pRxWI->RSSI1; 
	pAd->ate.RSSI2 = pRxWI->RSSI2; 
	pAd->ate.SNR0 = pRxWI->SNR0;
	pAd->ate.SNR1 = pRxWI->SNR1;
}




INT RtmpDoAte(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	struct iwreq	*wrq)
{
	USHORT Command_Id;
	INT32 Status = NDIS_STATUS_SUCCESS;
	struct ate_racfghdr *pRaCfg;
	UINT32 RangeLimitIndex = 0;
	UINT32 TableIndex = 0;

	/* check if the ioctl from ated is for me */
	if (strcmp(wrq->ifr_name, pAdapter->net_dev->name) != 0)
	{
		/* not for me... */
		Status = -EINVAL;
		return Status;
	}
	
	pRaCfg = kmalloc(sizeof(struct ate_racfghdr), GFP_KERNEL);

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
    
	Command_Id = OS_NTOHS(pRaCfg->command_id);
	DBGPRINT(RT_DEBUG_TRACE,("\n%s: Command_Id = 0x%04x !\n", __FUNCTION__, Command_Id));
	
	while (TableIndex < (sizeof(RACFG_CMD_TABLES) / sizeof(RACFG_CMD_TABLE)))
	{
		int cmd_index = 0;
		cmd_index = Command_Id - RACFG_CMD_TABLES[TableIndex].cmdOffset;
		if ((cmd_index >=0) && (cmd_index < RACFG_CMD_TABLES[TableIndex].cmdSetSize))
		{
			RACFG_CMD_HANDLER *pCmdSet;

			pCmdSet = RACFG_CMD_TABLES[TableIndex].cmdSet;
			if (pCmdSet[cmd_index] != NULL)
				Status = (*pCmdSet[cmd_index])(pAdapter, wrq, pRaCfg);
			break;
		}
		RangeLimitIndex++;
		TableIndex++;
	}
	
	// In passive mode only allow commands that read registers
	if (pAdapter->ate.PassiveMode)
	{
		int i, allowCmd = FALSE;
		static int allowedCmds[] = {
				RACFG_CMD_E2PROM_READ16, RACFG_CMD_E2PROM_READ_ALL,
				RACFG_CMD_IO_READ, RACFG_CMD_IO_READ_BULK,
				RACFG_CMD_BBP_READ8, RACFG_CMD_BBP_READ_ALL,
				RACFG_CMD_ATE_E2PROM_READ_BULK,
				RACFG_CMD_ATE_BBP_READ_BULK,
				RACFG_CMD_ATE_RF_READ_BULK,
				};

		for (i=0; i<sizeof(allowedCmds)/sizeof(allowedCmds[0]); i++)
		{
			if (Command_Id==allowedCmds[i])
			{
				allowCmd = TRUE;
				break;
			}
		}

		// Also allow writes to BF profile registers
		if (Command_Id==RACFG_CMD_BBP_WRITE8)
		{
			int offset = ntohs(pRaCfg->status);
			if (offset==BBP_R27 || (offset>=BBP_R174 && offset<=BBP_R182))
				allowCmd = TRUE;
		}

		// If not allowed then ignore the command
		if (!allowCmd)
		{
			ResponseToGUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
			if (pRaCfg != NULL)
			{
				kfree(pRaCfg);
			}
			return NDIS_STATUS_FAILURE;
		}
	}
	
 ERROR1:
 	kfree(pRaCfg);
 ERROR0:
	return Status;
}


VOID BubbleSort(INT32 n, INT32 a[])
{ 
	INT32 k, j, temp;

	for (k = n-1;  k>0;  k--)
	{
		for (j = 0; j<k; j++)
		{
			if (a[j] > a[j+1])
			{
				temp = a[j]; 
				a[j]=a[j+1]; 
				a[j+1]=temp;
			}
		}
	}
} 


VOID CalNoiseLevel(PRTMP_ADAPTER pAd, UCHAR channel, INT32 RSSI[3][10])
{
	INT32		RSSI0, RSSI1, RSSI2;
 	CHAR		Rssi0Offset, Rssi1Offset, Rssi2Offset;
	UCHAR		BbpR50Rssi0 = 0, BbpR51Rssi1 = 0, BbpR52Rssi2 = 0;
	UCHAR		Org_BBP66value = 0, Org_BBP69value = 0, Org_BBP70value = 0, data = 0;
	USHORT		LNA_Gain = 0;
	INT32       j = 0;
	UCHAR		Org_Channel = pAd->ate.Channel;
	USHORT	    GainValue = 0, OffsetValue = 0;

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &Org_BBP66value);
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R69, &Org_BBP69value);	
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R70, &Org_BBP70value);

	//**********************************************************************
	// Read the value of LNA gain and Rssi offset 
	//**********************************************************************
	RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, GainValue);

	// for Noise Level
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
	//**********************************************************************	
	{
		pAd->ate.Channel = channel;
		ATEAsicSwitchChannel(pAd);
		mdelay(5);

		data = 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);	

		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, data);
		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, data);
		mdelay(5);

		// start Rx
		pAd->ate.bQARxStart = TRUE;
		Set_ATE_Proc(pAd, "RXFRAME");

		mdelay(5);

		for (j = 0; j < 10; j++)
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R50, &BbpR50Rssi0);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R51, &BbpR51Rssi1);	
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R52, &BbpR52Rssi2);

			mdelay(10);

			// calculate RSSI 0
			if (BbpR50Rssi0 == 0)
			{
				RSSI0 = -100;
			}
			else
			{
				RSSI0 = (INT32)(-12 - BbpR50Rssi0 - LNA_Gain - Rssi0Offset);
			}
			RSSI[0][j] = RSSI0;

			if ( pAd->Antenna.field.RxPath >= 2 ) // 2R
			{
				// calculate RSSI 1
				if (BbpR51Rssi1 == 0)
				{
					RSSI1 = -100;
				}
				else
				{
					RSSI1 = (INT32)(-12 - BbpR51Rssi1 - LNA_Gain - Rssi1Offset);
				}
				RSSI[1][j] = RSSI1;
			}

			if ( pAd->Antenna.field.RxPath >= 3 ) // 3R
			{
				// calculate RSSI 2
				if (BbpR52Rssi2 == 0)
					RSSI2 = -100;
				else
					RSSI2 = (INT32)(-12 - BbpR52Rssi2 - LNA_Gain - Rssi2Offset);

				RSSI[2][j] = RSSI2;
			}
		}

		// stop Rx
		Set_ATE_Proc(pAd, "RXSTOP");

		mdelay(5);

		BubbleSort(10, RSSI[0]);	// 1R		

		if ( pAd->Antenna.field.RxPath >= 2 ) // 2R
		{
			BubbleSort(10, RSSI[1]);
		}

		if ( pAd->Antenna.field.RxPath >= 3 ) // 3R
		{
			BubbleSort(10, RSSI[2]);
		}	
	}

	pAd->ate.Channel = Org_Channel;
	ATEAsicSwitchChannel(pAd);
	
	// restore original value
    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Org_BBP66value);
    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, Org_BBP69value);
    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, Org_BBP70value);

	return;
}


BOOLEAN SyncTxRxConfig(PRTMP_ADAPTER pAd, USHORT offset, UCHAR value)
{ 
	UCHAR tmp = 0, bbp_data = 0;

	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, offset, &bbp_data);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, offset, &bbp_data);
	}

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
					pAd->ate.TxAntennaSel = 0;
		            break;
				/* The BBP R1 bit[4:3] = 0 :: DAC 0 will be used by QA. */
		        case 0:
					/* Antenna one */
					pAd->ate.TxAntennaSel = 1;
		            break;
				/* The BBP R1 bit[4:3] = 1 :: DAC 1 will be used by QA. */
		        case 1:
					/* Antenna two */
					pAd->ate.TxAntennaSel = 2;
		            break;
		        default:
		            DBGPRINT(RT_DEBUG_TRACE, ("%s -- Sth. wrong!  : return FALSE; \n", __FUNCTION__));    
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
					pAd->ate.RxAntennaSel = 0;
		            break;
				/*
					The BBP R3 bit[1:0] = 0 :: ADC 0 will be used by QA,
					unless the BBP R3 bit[4:3] = 2
				*/
		        case 0:
					/* Antenna one */
					pAd->ate.RxAntennaSel = 1;
					tmp = ((bbp_data & ((1 << 4) | (1 << 3))/* 0x03 */) >> 3);
					if (tmp == 2)// 3R
					{
						/* Default : All ADCs will be used by QA */
						pAd->ate.RxAntennaSel = 0;
					}
		            break;
				/* The BBP R3 bit[1:0] = 1 :: ADC 1 will be used by QA. */
		        case 1:
					/* Antenna two */
					pAd->ate.RxAntennaSel = 2;
		            break;
				/* The BBP R3 bit[1:0] = 2 :: ADC 2 will be used by QA. */
		        case 2:
					/* Antenna three */
					pAd->ate.RxAntennaSel = 3;
		            break;
		        default:
		            DBGPRINT(RT_DEBUG_ERROR, ("%s -- Impossible!  : return FALSE; \n", __FUNCTION__));    
		            return FALSE;
		    }
			break;/* case BBP_R3 */

        default:
            DBGPRINT(RT_DEBUG_ERROR, ("%s -- Sth. wrong!  : return FALSE; \n", __FUNCTION__));    
            return FALSE;
		
	}
	return TRUE;
} 


static VOID memcpy_exl(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i, Value = 0;
	ULONG *pDst, *pSrc;
	UCHAR *p8;
	
	p8 = src;
	pDst = (ULONG *) dst;
	pSrc = (ULONG *) src;
	
	for (i = 0 ; i < (len/4); i++)
	{
		/* For alignment issue, we need a variable "Value". */
		memmove(&Value, pSrc, 4);
		Value = OS_HTONL(Value); 
		memmove(pDst, &Value, 4);		
		pDst++;
		pSrc++;
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

		for (i =0; i < (len/2); i++)
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

}


static VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, UINT32 len)
{
	UINT32 i, Value;
	UINT32 *pDst, *pSrc;
	
	pDst = (UINT32 *) dst;
	pSrc = (UINT32 *) src;

	for (i = 0 ; i < (len/4); i++)
	{
		RTMP_IO_READ32(pAd, (ULONG)pSrc, &Value);
		Value = OS_HTONL(Value);
		memmove(pDst, &Value, 4);
		pDst++;
		pSrc++;
	}
	return;	
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
	USHORT buffer[EEPROM_SIZE/2];
	USHORT *p;
	INT i;
	
	rt_ee_read_all(pAd, (USHORT *)buffer);
	p = buffer;

	for (i = 0; i < (EEPROM_SIZE/2); i++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%4.4x ", *p));
		if (((i+1) % 16) == 0)
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
		DBGPRINT(RT_DEBUG_ERROR, ("Offset can not exceed EEPROM_SIZE( == 0x%04x)\n", EEPROM_SIZE));	
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
			
	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, offset,  &value);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, offset,  &value);
	}

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

	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, offset,  value);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, offset,  value);
	}

	return TRUE;
}


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
#endif // DBG //
#endif // RALINK_QA //


#ifdef CONFIG_AP_SUPPORT 
/*
==========================================================================
	Description:
		Used only by ATE to disassociate all STAs and stop AP service.
	Note:
==========================================================================
*/
VOID ATEAPStop(
	IN PRTMP_ADAPTER pAd) 
{
	BOOLEAN     Cancelled;
	UINT32		Value = 0;
	INT         apidx = 0;
		
	DBGPRINT(RT_DEBUG_TRACE, ("!!! ATEAPStop !!!\n"));

	/* To prevent MCU to modify BBP registers w/o indications from driver. */
#ifdef DFS_SUPPORT
#ifdef DFS_HARDWARE_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	if (pAd->CommonCfg.dfs_func >= HARDWARE_DFS_V1)
		NewRadarDetectionStop(pAd)
#endif // CONFIG_AP_SUPPORT //
#endif // DFS_HARDWARE_SUPPORT //
#ifdef DFS_SOFTWARE_SUPPORT
	if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1)
	{
	RadarDetectionStop(pAd);
	BbpRadarDetectionStop(pAd);
	}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
		if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		{
			// make sure CarrierDetect wont send CTS
			CARRIER_DETECT_STOP(pAd);
		}
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif // WDS_SUPPORT //

#ifdef APCLI_SUPPORT
	ApCliIfDown(pAd);
#endif // APCLI_SUPPORT //

	MacTableReset(pAd);

	/* For ATE, this flag cannot be set. */
//	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	// disable pre-tbtt interrupt
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &=0xe;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	// disable piggyback
	RTMPSetPiggyBack(pAd, FALSE);

	ATEDisableAsicProtect(pAd);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
//		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
		AsicDisableSync(pAd);
#ifdef LED_CONTROL_SUPPORT
		// set LED
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif // LED_CONTROL_SUPPORT //
	}


	for (apidx = 0; apidx < MAX_MBSSID_NUM(pAd); apidx++)
	{
		if (pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning == TRUE)
		{
			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].REKEYTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].REKEYTimerRunning = FALSE;
		}
	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
	}
	
#ifdef WAPI_SUPPORT
	RTMPCancelWapiRekeyTimerAction(pAd, NULL);
#endif // WAPI_SUPPORT //
	
	/* Cancel the Timer, to make sure the timer was not queued. */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

//	RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
	if (pAd->ApCfg.ApQuickResponeForRateUpTimerRunning == TRUE)
		RTMPCancelTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, &Cancelled);

#ifdef IDS_SUPPORT
	// if necessary, cancel IDS timer
	RTMPIdsStop(pAd);
#endif // IDS_SUPPORT //


#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPEnable == TRUE)
	{
	DisableAPMIMOPS(pAd);
		pAd->ApCfg.GreenAPLevel=GREENAP_WITHOUT_ANY_STAS_CONNECT;
	}
#endif // GREENAP_SUPPORT //

}
#endif // CONFIG_AP_SUPPORT //

#ifdef RTMP_INTERNAL_TX_ALC
UCHAR ATEGetDesiredTSSI(
	IN PRTMP_ADAPTER 	pAd)
{
	extern CHAR desiredTSSIOverCCK[];
	extern CHAR desiredTSSIOverOFDM[];
	extern CHAR desiredTSSIOverHT[];
	extern CHAR desiredTSSIOverHTUsingSTBC[];

	UCHAR desiredTSSI = 0;
	UCHAR MCS = 0;

	MCS = (UCHAR)(pAd->ate.TxWI.MCS);
	
	if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
	{
		if (MCS > 3) // boundary verification
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));
			
			MCS = 0;
		}
	
		desiredTSSI = desiredTSSIOverCCK[MCS];
	}
	else if (pAd->ate.TxWI.PHYMODE == MODE_OFDM)
	{
		if (MCS > 7) // boundary verification
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));

			MCS = 0;
		}

		desiredTSSI = desiredTSSIOverOFDM[MCS];
	}
	else if ((pAd->ate.TxWI.PHYMODE == MODE_HTMIX) || (pAd->ate.TxWI.PHYMODE == MODE_HTGREENFIELD))
	{
		if (MCS > 7) // boundary verification
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: MCS = %d\n", 
				__FUNCTION__, 
				MCS));

			MCS = 0;
		}

		if (pAd->ate.TxWI.STBC == 0)
		{
			desiredTSSI = desiredTSSIOverHT[MCS];
		}
		else
		{
			desiredTSSI = desiredTSSIOverHTUsingSTBC[MCS];
		}

		//
		// For HT BW40 MCS 7 with/without STBC configuration, the desired TSSI value should subtract one from the formula.
		//
		if ((pAd->ate.TxWI.BW == BW_40) && (MCS == MCS_7))
		{
			desiredTSSI -= 1;
		}
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: desiredTSSI = %d, Latest Tx HT setting: MODE = %d, MCS = %d, STBC = %d\n", 
		__FUNCTION__, 
		desiredTSSI, 
		pAd->ate.TxWI.PHYMODE, 
		pAd->ate.TxWI.MCS, 
		pAd->ate.TxWI.STBC));

	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));

	return desiredTSSI;
}

INT Set_ATE_TSSI_CALIBRATION_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{    
	UINT i = 0;
	UCHAR inputDAC;
	
	inputDAC = simple_strtol(arg, 0, 10);

	if ((!IS_RT5390(pAd) && !IS_RT3390(pAd)) || !(pAd->TxPowerCtrl.bInternalTxALC))                  
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Not support TSSI calibration since not 5390/3390 chip or EEPROM not set!!!\n"));
		return FALSE;
	}
#ifdef RT53xx		
	else if (IS_RT5390(pAd))
	{                              
		UCHAR	BbpData = 0, RFValue, OrgBbp47Value/*, ChannelPower*/;
		USHORT	EEPData;
		UCHAR 	BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
		BBP_R47_STRUC BBPR47;

		//Set RF R27[3:0] TSSI gain			
		RT30xxReadRFRegister(pAd, RF_R27, (PUCHAR)(&RFValue));			
		RFValue = ((RFValue & 0xF0) | pAd->TssiGain); // [3:0] = (tssi_gain and tssi_atten)
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);	

		//Set RF R28 bit[7:6] = 00
		RT30xxReadRFRegister(pAd, RF_R28, &RFValue);
		//RF28Value = RFValue;
		RFValue &= (~0xC0); 
		RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

		//set BBP R47[7] = 1(ADC6 ON), R47[4:3] = 0x2(new average TSSI mode), R47[2] = 1(TSSI_UPDATE_REQ), R49[1:0] = 0(TSSI info 0 - TSSI)
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPR47.byte);
		OrgBbp47Value = BBPR47.byte;
		BBPR47.field.Adc6On = 1;
		BBPR47.field.TssiMode = 0x02;
		BBPR47.field.TssiUpdateReq = 1;
		BBPR47.field.TssiReportSel = 0;							
		DBGPRINT(RT_DEBUG_TRACE, ("Write BBP R47 = 0x%x\n", BBPR47.byte));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPR47.byte);		
		
		NdisZeroMemory(&pAd->ate, sizeof(struct _ATE_INFO));
		pAd->ate.TxCount = 100;
		pAd->ate.TxLength = 1024;
		pAd->ate.Channel = 1;
		//pAd->ate.TxWI.PHYMODE= 1; 	// MODE_OFDM
		//pAd->ate.TxWI.MCS = 7; 		// 54Mbps
		//pAd->ate.TxWI.BW = 0; 		// 20MHz
		COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
		COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);    

		Set_ATE_TX_MODE_Proc(pAd, "1");		// MODE_OFDM
		Set_ATE_TX_MCS_Proc(pAd, "7");		// 54Mbps
		Set_ATE_TX_BW_Proc(pAd, "0");		// 20MHz

		if (inputDAC > 1)
		{	
			/* Set power value calibrated DAC */
			pAd->ate.TxPower0 = inputDAC;
             		DBGPRINT(RT_DEBUG_TRACE, ("(Calibrated) Tx.Power0= 0x%x\n", pAd->ate.TxPower0));
		}
		else
		{
			/* Read calibrated channel power value from EEPROM */
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET, EEPData);
			pAd->ate.TxPower0 = (UCHAR) (EEPData & 0xff);
			DBGPRINT(RT_DEBUG_TRACE, ("(Calibrated) Tx.Power0= 0x%x\n", pAd->ate.TxPower0));
		}
		
		//read frequency offset from EEPROM                        
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
		pAd->ate.RFFreqOffset = (UCHAR) (EEPData & 0xff);
			
		Set_ATE_Proc(pAd, "TXFRAME"); 
		RTMPusecDelay(200000);

		while (i < 500)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpData);

			if ((BbpData & 0x04) == 0)
				break;

			RTMPusecDelay(2);
			i++;	
		}

		if (i >= 500)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("TSSI status not ready!!! (i=%d)\n", i));
			return FALSE;
		}	

		//read BBP R49[6:0] and write to EEPROM 0x6E
		DBGPRINT(RT_DEBUG_TRACE, ("Read  BBP_R49\n")); 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%x\n", BbpData)); 
		BbpData &= 0x7f;

		//the upper boundary of 0x6E (TSSI base) is 0x7C
		if (BbpData > 0x7C)
			BbpData = 0;

		RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
		EEPData &= 0xff00;
		EEPData |= BbpData;
		DBGPRINT(RT_DEBUG_TRACE, ("Write  E2P 0x6e: 0x%x\n", EEPData)); 		

#ifdef RTMP_EFUSE_SUPPORT
		if (pAd->bUseEfuse)
		{
			if (pAd->bFroceEEPROMBuffer)
				NdisMoveMemory(&(pAd->EEPROMImage[EEPROM_TSSI_OVER_OFDM_54]), (PUCHAR) (&EEPData) ,2);
			else
				eFuseWrite(pAd, EEPROM_TSSI_OVER_OFDM_54, (PUSHORT) (&EEPData), 2);
		}
		else
#endif // RTMP_EFUSE_SUPPORT //
		{
			RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
			RTMPusecDelay(10);
		}    

		//restore RF R27 and R28, BBP R47
		//RT30xxWriteRFRegister(pAd, RF_R27, RF27Value);				
		//RT30xxWriteRFRegister(pAd, RF_R28, RF28Value);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, OrgBbp47Value);

		Set_ATE_Proc(pAd, "ATESTART");
	}
#endif // RT53xx //
#ifdef RT33xx	
	else if (IS_RT3390(pAd))
	{                              
		UCHAR        	BbpData = 0, RFValue, RF27Value, RF28Value, BBP49Value;
		USHORT		EEPData;                              
		UCHAR         BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

		//set RF R27 bit[7][3] = 11
		RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
		RF27Value = RFValue;
		RFValue |= 0x88; 
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

		//set RF R28 bit[6][5] = 00
		RT30xxReadRFRegister(pAd, RF_R28, &RFValue);
		RF28Value = RFValue;
		RFValue &= 0x9f; 
		RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

		//set BBP R49[7] = 1
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);

		BBP49Value = BbpData;
		BbpData |= 0x80;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BbpData);

		//start TX at 54Mbps
		NdisZeroMemory(&pAd->ate, sizeof(struct _ATE_INFO));
		pAd->ate.TxCount = 10000000;
		pAd->ate.TxLength = 1024;
		pAd->ate.Channel = 1;
		//pAd->ate.TxWI.PHYMODE= 1; 	// MODE_OFDM
		//pAd->ate.TxWI.MCS = 7; 		// 54Mbps
		//pAd->ate.TxWI.BW = 0; 		// 20MHz
		COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
		COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);                     

		if (inputDAC > 1)
		{	
			/* Set power value calibrated DAC */
			pAd->ate.TxPower0 = inputDAC;
             		DBGPRINT(RT_DEBUG_TRACE, ("(Calibrated) Tx.Power0= 0x%x\n", pAd->ate.TxPower0));
		}
		else
		{
			/* Read calibrated channel power value from EEPROM */
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET, EEPData);
			pAd->ate.TxPower0 = (UCHAR) (EEPData & 0xff);
			DBGPRINT(RT_DEBUG_TRACE, ("(Calibrated) Tx.Power0= 0x%x\n", pAd->ate.TxPower0));
		}

		//read frequency offset from EEPROM                         
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
		pAd->ate.RFFreqOffset = (UCHAR) (EEPData & 0xff);

		Set_ATE_TX_MODE_Proc(pAd, "1");		// MODE_OFDM
		Set_ATE_TX_MCS_Proc(pAd, "7");		// 54Mbps
		Set_ATE_TX_BW_Proc(pAd, "0");		// 20MHz

		Set_ATE_Proc(pAd, "TXFRAME"); 
		RTMPusecDelay(500000);

		//read BBP R49[4:0] and write to EEPROM 0x6E
		DBGPRINT(RT_DEBUG_TRACE, ("Read  BBP_R49\n")); 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP R49 = 0x%x\n", BbpData)); 
		BbpData &= 0x1f;
		RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
		EEPData &= 0xff00;
		EEPData |= BbpData;
		DBGPRINT(RT_DEBUG_TRACE, ("Write  E2P 0x6e: 0x%x\n", EEPData)); 
#ifdef RTMP_EFUSE_SUPPORT			
		if (pAd->bUseEfuse)
		{
			if (pAd->bFroceEEPROMBuffer)
				NdisMoveMemory(&(pAd->EEPROMImage[EEPROM_TSSI_OVER_OFDM_54]), (PUCHAR) (&EEPData) ,2);
			else
				eFuseWrite(pAd, EEPROM_TSSI_OVER_OFDM_54, (PUSHORT) (&EEPData), 2);
		}
		else
#endif // RTMP_EFUSE_SUPPORT //
		{
			RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
			RTMPusecDelay(10);
		}                              

		//restore RF R27 and R28, BBP R49
		//RT30xxWriteRFRegister(pAd, RF_R27, RF27Value);                          
		//RT30xxWriteRFRegister(pAd, RF_R28, RF28Value);
		//RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, BBP49Value);     

		Set_ATE_Proc(pAd, "ATESTART");
	}
#endif // RT33xx //	

	return TRUE;	
}

#if defined(RT5390) || defined(RT5370)
BOOLEAN ATEGetDesiredTssiAndCurrentTssi(
	IN 		PRTMP_ADAPTER 		pAd, 
	IN OUT 	PCHAR 				pDesiredTssi, 
	IN OUT 	PCHAR 				pCurrentTssi)
{
	extern CHAR desiredTSSIOverCCKExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][4];
	extern CHAR desiredTSSIOverOFDMExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][8];
	extern CHAR desiredTSSIOverHTExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][8];
	extern CHAR desiredTSSIOverHTUsingSTBCExt[NUM_OF_CH_FOR_PER_CH_TX_PWR_OFFSET + 1][8];
	
	UCHAR BbpR47 = 0;
	UCHAR RateInfo = 0;
	CCK_TSSI_INFO cckTssiInfo = {{0}};
	OFDM_TSSI_INFO ofdmTssiInfo = {{0}};
	HT_TSSI_INFO htTssiInfo = {
			.PartA.value= 0,
			.PartB.value = 0,
		};

	UCHAR ch = 0;

	DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

	if (IS_RT5390(pAd))
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		if ((BbpR47 & 0x04) == 0x04) /* The TSSI INFO is not ready. */
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: BBP TSSI INFO is not ready. (BbpR47 = 0x%X)\n", __FUNCTION__, BbpR47));

			return FALSE;
		}
		
		/* Get TSSI */
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)(pCurrentTssi));

		if ((*pCurrentTssi < 0) || (*pCurrentTssi > 0x7C))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Out of range, *pCurrentTssi = %d\n", __FUNCTION__, *pCurrentTssi));
			
			*pCurrentTssi = 0;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("%s: *pCurrentTssi = %d\n", __FUNCTION__, *pCurrentTssi));
		
		/* Get packet information */
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		BbpR47 = ((BbpR47 & ~0x03) | 0x01); /* TSSI_REPORT_SEL (TSSI INFO 1 - Packet infomation) */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)(&RateInfo));

		if ((RateInfo & 0x03) == MODE_CCK) /* CCK */
		{
			cckTssiInfo.value = RateInfo;

			DBGPRINT(RT_DEBUG_TRACE, ("%s: CCK, cckTssiInfo.field.Rate = %d\n", 
				__FUNCTION__, 
				cckTssiInfo.field.Rate));

			DBGPRINT(RT_DEBUG_INFO, ("%s: RateInfo = 0x%X\n", __FUNCTION__, RateInfo));

			if (((cckTssiInfo.field.Rate >= 4) && (cckTssiInfo.field.Rate <= 7)) || 
			      (cckTssiInfo.field.Rate > 11)) /* boundary verification */
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: cckTssiInfo.field.Rate = %d\n", 
					__FUNCTION__, 
					cckTssiInfo.field.Rate));
				
				return FALSE;
			}

			/* Data rate mapping for short/long preamble over CCK */
			if (cckTssiInfo.field.Rate == 8)
			{
				cckTssiInfo.field.Rate = 0; /* Short preamble CCK 1Mbps => Long preamble CCK 1Mbps */
			}
			else if (cckTssiInfo.field.Rate == 9)
			{
				cckTssiInfo.field.Rate = 1; /* Short preamble CCK 2Mbps => Long preamble CCK 2Mbps */
			}
			else if (cckTssiInfo.field.Rate == 10)
			{
				cckTssiInfo.field.Rate = 2; /* Short preamble CCK 5.5Mbps => Long preamble CCK 5.5Mbps */
			}
			else if (cckTssiInfo.field.Rate == 11)
			{
				cckTssiInfo.field.Rate = 3; /* Short preamble CCK 11Mbps => Long preamble CCK 11Mbps */
			}

			if ((pAd->ate.Channel >= 1) && (pAd->ate.Channel <= 14))
			{
				ch = pAd->ate.Channel;
			}
			else
			{
				ch = 1;

				DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect channel #%d\n", __FUNCTION__, pAd->ate.Channel));
			}
		
			*pDesiredTssi = desiredTSSIOverCCKExt[ch][cckTssiInfo.field.Rate];

			DBGPRINT(RT_DEBUG_TRACE, ("%s: ch = %d, *pDesiredTssi = %d\n", __FUNCTION__, ch, *pDesiredTssi));

		}
		else if ((RateInfo & 0x03) == MODE_OFDM) /* OFDM */
		{
			ofdmTssiInfo.value = RateInfo;
			
			/* BBP OFDM rate format ==> MAC OFDM rate format */
			
			switch (ofdmTssiInfo.field.Rate)
			{
				case 0x0B: /* 6 Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_0;
				}
				break;

				case 0x0F: /* 9 Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_1;
				}
				break;

				case 0x0A: /* 12 Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_2;
				}
				break;

				case 0x0E: /* 18  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_3;
				}
				break;

				case 0x09: /* 24  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_4;
				}
				break;

				case 0x0D: /* 36  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_5;
				}
				break;

				case 0x08: /* 48  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_6;
				}
				break;

				case 0x0C: /* 54  Mbits/s */
				{
					ofdmTssiInfo.field.Rate = MCS_7;
				}
				break;

				default: 
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect OFDM rate = 0x%X\n", __FUNCTION__, ofdmTssiInfo.field.Rate));
					
					return FALSE;
				}
				break;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s: OFDM, ofdmTssiInfo.field.Rate = %d\n", 
				__FUNCTION__, 
				ofdmTssiInfo.field.Rate));

			if ((ofdmTssiInfo.field.Rate < 0) || (ofdmTssiInfo.field.Rate > 7)) /* boundary verification */
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: ofdmTssiInfo.field.Rate = %d\n", 
					__FUNCTION__, 
					ofdmTssiInfo.field.Rate));

				return FALSE;
			}

			if ((pAd->ate.Channel >= 1) && (pAd->ate.Channel <= 14))
			{
				ch = pAd->ate.Channel;
			}
			else
			{
				ch = 1;

				DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect channel #%d\n", __FUNCTION__, pAd->ate.Channel));
			}

			*pDesiredTssi = desiredTSSIOverOFDMExt[ch][ofdmTssiInfo.field.Rate];

			DBGPRINT(RT_DEBUG_TRACE, ("%s: ch = %d, *pDesiredTssi = %d\n", __FUNCTION__, ch, *pDesiredTssi));
			
			DBGPRINT(RT_DEBUG_INFO, ("%s: RateInfo = 0x%X\n", __FUNCTION__, RateInfo));
		}
		else /* Mixed mode or green-field mode */
		{
			htTssiInfo.PartA.value = RateInfo;
			DBGPRINT(RT_DEBUG_INFO, ("%s: RateInfo = 0x%X\n", __FUNCTION__, RateInfo));

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
			BbpR47 = ((BbpR47 & ~0x03) | 0x02); /* TSSI_REPORT_SEL (TSSI INFO 2 - Packet infomation) */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, 0x92);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)(&RateInfo));

			htTssiInfo.PartB.value = RateInfo;
			DBGPRINT(RT_DEBUG_INFO, ("%s: RateInfo = 0x%X\n", __FUNCTION__, RateInfo));

			DBGPRINT(RT_DEBUG_TRACE, ("%s: HT, htTssiInfo.PartA.field.STBC = %d, htTssiInfo.PartB.field.MCS = %d\n", 
				__FUNCTION__, 
				htTssiInfo.PartA.field.STBC, 
				htTssiInfo.PartB.field.MCS));

			if ((htTssiInfo.PartB.field.MCS < 0) || (htTssiInfo.PartB.field.MCS > 7)) /* boundary verification */
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: incorrect MCS: htTssiInfo.PartB.field.MCS = %d\n", 
					__FUNCTION__, 
					htTssiInfo.PartB.field.MCS));

				return FALSE;
			}

			if ((pAd->ate.Channel >= 1) && (pAd->ate.Channel <= 14))
			{
				ch = pAd->ate.Channel;
			}
			else
			{
				ch = 1;

				DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect channel #%d\n", __FUNCTION__, pAd->ate.Channel));
			}

			if (htTssiInfo.PartA.field.STBC == 0)
			{
				*pDesiredTssi = desiredTSSIOverHTExt[ch][htTssiInfo.PartB.field.MCS];
			}
			else
			{
				*pDesiredTssi = desiredTSSIOverHTUsingSTBCExt[ch][htTssiInfo.PartB.field.MCS];
			}

			DBGPRINT(RT_DEBUG_TRACE, ("%s: ch = %d, *pDesiredTssi = %d\n", __FUNCTION__, ch, *pDesiredTssi));			
		}	

		if (*pDesiredTssi < 0x00)
		{
			*pDesiredTssi = 0x00;
		}	
		else if (*pDesiredTssi > 0x7C)
		{
			*pDesiredTssi = 0x7C;
		}

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		BbpR47 = ((BbpR47 & ~0x07) | 0x04); /* TSSI_REPORT_SEL (TSSI INFO 0 - TSSI) and enable TSSI INFO udpate */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);
	}

	DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));

	return TRUE;
}

//Vx = V0 + t(V1 - V0) ? f(x), where t = (x-x0) / (x1 - x0)
CHAR RTATEInsertTssi(UCHAR InChannel, UCHAR Channel0, UCHAR Channel1,CHAR Tssi0, CHAR Tssi1)
{
	CHAR	InTssi, TssiDelta, ChannelDelta, InChannelDelta;
	
	ChannelDelta = Channel1 - Channel0;
	InChannelDelta = InChannel - Channel0;
	TssiDelta = Tssi1 - Tssi0;

	// channel delta should not be 0
	if (ChannelDelta == 0)
		InTssi = Tssi0;

	DBGPRINT(RT_DEBUG_WARN, ("--->RTATEInsertTssi\n")); 	
	
	if ((TssiDelta > 0) && (((InChannelDelta * TssiDelta * 10) / ChannelDelta) % 10 >= 5))
	{
		InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);
		InTssi += 1;
	}
	else	if ((TssiDelta < 0) && (((InChannelDelta * TssiDelta * 10) / ChannelDelta) % 10 <= -5))
	{
		InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);
		InTssi -= 1;
	}
	else
	{
		InTssi = Tssi0 + ((InChannelDelta * TssiDelta) / ChannelDelta);	
	}	

	DBGPRINT(RT_DEBUG_WARN, ("<---RTATEInsertTssi\n")); 		
	
	return InTssi;
}

UCHAR RTATEGetTssiByChannel(PRTMP_ADAPTER pAd, UCHAR Channel)
{
	UINT	i = 0;
	UCHAR	BbpData =0;
	UCHAR	ChannelPower;
	UCHAR 	BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	USHORT	EEPData;
	BBP_R47_STRUC BBPR47;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPR47.byte);
	BBPR47.field.Adc6On = 1;
	BBPR47.field.TssiMode = 0x02;
	BBPR47.field.TssiUpdateReq = 1;
	BBPR47.field.TssiReportSel = 0;							
	DBGPRINT(RT_DEBUG_WARN, ("Write BBP R47 = 0x%x\n", BBPR47.byte));
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPR47.byte);
		
	//start TX at 54Mbps	
	NdisZeroMemory(&pAd->ate, sizeof(struct _ATE_INFO));
	pAd->ate.TxCount = 100;
	pAd->ate.TxLength = 1024;
	pAd->ate.Channel = Channel;
	//pAd->ate.TxWI.PHYMODE= 1; 	// MODE_OFDM
	//pAd->ate.TxWI.MCS = 7; 		// 54Mbps
	//pAd->ate.TxWI.BW = 0; 		// 20MHz
	COPY_MAC_ADDR(pAd->ate.Addr1, BROADCAST_ADDR);
	COPY_MAC_ADDR(pAd->ate.Addr2, pAd->PermanentAddress);                                                     
	COPY_MAC_ADDR(pAd->ate.Addr3, BSSID_ADDR);    		

	Set_ATE_TX_MODE_Proc(pAd, "1");		// MODE_OFDM
	Set_ATE_TX_MCS_Proc(pAd, "7");		// 54Mbps
	Set_ATE_TX_BW_Proc(pAd, "0");		// 20MHz
		
	//read calibrated channel power value from EEPROM
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET+Channel-1, ChannelPower);
	pAd->ate.TxPower0  = ChannelPower;
	
	//read frequency offset from EEPROM                        
	RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
	pAd->ate.RFFreqOffset = (UCHAR) (EEPData & 0xff);
		
	Set_ATE_Proc(pAd, "TXFRAME"); 
	RTMPusecDelay(200000);

	while (i < 500)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpData);

		if ((BbpData & 0x04) == 0)
			break;

		RTMPusecDelay(2);
		i++;	
	}

	if (i >= 500)
	{
		DBGPRINT(RT_DEBUG_WARN, ("TSSI status not ready!!! (i=%d)\n", i));
		//return FALSE;
	}	

	//read BBP R49[6:0] and write to EEPROM 0x6E
	DBGPRINT(RT_DEBUG_WARN, ("Read  BBP_R49\n")); 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
	DBGPRINT(RT_DEBUG_WARN, ("BBP R49 = 0x%x\n", BbpData)); 
	BbpData &= 0x7f;

	//the upper boundary of 0x6E (TSSI base) is 0x7C
	if (BbpData > 0x7C)
		BbpData = 0;

	//back to ATE IDLE state
	Set_ATE_Proc(pAd, "ATESTART");

	return BbpData;	
}

//
// Get the power delta bound
//
#define GET_TSSI_RATE_TABLE_INDEX(x) (((x) > UPPER_POWER_DELTA_INDEX) ? (UPPER_POWER_DELTA_INDEX) : (((x) < LOWER_POWER_DELTA_INDEX) ? (LOWER_POWER_DELTA_INDEX) : ((x))))

//
CHAR GetPowerDeltaFromTssiRatio(CHAR TssiOfChannel, CHAR TssiBase)
{
	LONG	TssiRatio, TssiDelta, MinTssiDelta;
	CHAR	i, PowerDeltaStatIndex, PowerDeltaEndIndex, MinTssiDeltaIndex;	
	CHAR	PowerDelta;
	extern ULONG TssiRatioTable[][2];

	// TODO: If 0 is a valid value for TSSI base
	if (TssiBase == 0)
		return 0;
	
	TssiRatio = TssiOfChannel * TssiRatioTable[0][1] / TssiBase;

	DBGPRINT(RT_DEBUG_WARN, ("TssiOfChannel = %d, TssiBase = %d, TssiRatio = %d\n", TssiOfChannel,  TssiBase, TssiRatio));

	PowerDeltaStatIndex = 4;
	PowerDeltaEndIndex = 19;

	MinTssiDeltaIndex= PowerDeltaStatIndex;
	MinTssiDelta = TssiRatio - TssiRatioTable[MinTssiDeltaIndex][0];
	
	if (MinTssiDelta < 0)
		MinTssiDelta = -MinTssiDelta;

	for (i = PowerDeltaStatIndex+1; i <= PowerDeltaEndIndex; i++)
	{
		TssiDelta = TssiRatio -TssiRatioTable[i][0];
		
		if (TssiDelta < 0)
		{
			TssiDelta = -TssiDelta;
		}

		if (TssiDelta < MinTssiDelta)
		{
			MinTssiDelta = TssiDelta;
			MinTssiDeltaIndex = i;
		}
	}

	PowerDelta = MinTssiDeltaIndex - TSSI_RATIO_TABLE_OFFSET;

	DBGPRINT(RT_DEBUG_WARN, ("MinTssiDeltaIndex = %d, MinTssiDelta = %d, PowerDelta = %d\n", MinTssiDeltaIndex,  MinTssiDelta, PowerDelta));
	return (PowerDelta);

}

INT Set_ATE_TSSI_CALIBRATION_EX_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{  
	UCHAR inputData;
	
	inputData = simple_strtol(arg, 0, 10);
	
	if (!(IS_RT5390(pAd) && (pAd->TxPowerCtrl.bInternalTxALC) && (pAd->TxPowerCtrl.bExtendedTssiMode)))			
	{
			DBGPRINT(RT_DEBUG_WARN, ("Not support TSSI calibration since not 5390 chip or EEPROM not set!!!\n"));
			return FALSE;
	}			
	else
	{				
		UCHAR	RFValue;
		CHAR	TssiRefPerChannel[14+1], PowerDeltaPerChannel[14+1], TssiBase;
		USHORT	EEPData;
		UCHAR	CurrentChannel;

		// step 0: set init register values for TSSI calibration
		//Set RF R27[3:2] = 00, R27[1:0] = 11 
		RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
		//RF27Value = RFValue;
		//RFValue &= (~0x0F); 
		//RFValue |= 0x02; 
		RFValue = ((RFValue & 0xF0) | pAd->TssiGain); // [3:0] = (tssi_gain and tssi_atten)
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

		//Set RF R28 bit[7:6] = 00
		RT30xxReadRFRegister(pAd, RF_R28, &RFValue);
		//RF28Value = RFValue;
		RFValue &= (~0xC0); 
		RT30xxWriteRFRegister(pAd, RF_R28, RFValue);

		//step 1: get channel 7 TSSI as reference value
		CurrentChannel = 7;
		TssiRefPerChannel[CurrentChannel] = RTATEGetTssiByChannel(pAd, CurrentChannel);
		TssiBase = TssiRefPerChannel[CurrentChannel];
		PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase);

		//Save TSSI ref base to EEPROM 0x6E
		RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
		EEPData &= 0xff00;
		EEPData |= TssiBase;
		DBGPRINT(RT_DEBUG_WARN, ("Write  E2P 0x6E: 0x%x\n", EEPData)); 				
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_OVER_OFDM_54, EEPData);
		RTMPusecDelay(10); //delay for twp(MAX)=10ms
		
		//step 2: get channel 1 and 13 TSSI values
		//start TX at 54Mbps
		CurrentChannel = 1;
		TssiRefPerChannel[CurrentChannel] = RTATEGetTssiByChannel(pAd, CurrentChannel);
		PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase);

		//start TX at 54Mbps
		CurrentChannel = 13;
		TssiRefPerChannel[CurrentChannel] = RTATEGetTssiByChannel(pAd, CurrentChannel);
		PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase);

		//step 3: insert the power table
		//insert channel 2 to 6 TSSI values
		//for(CurrentChannel = 2; CurrentChannel <7; CurrentChannel++)
			//TssiRefPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 1, 7, TssiRefPerChannel[1], TssiRefPerChannel[7]);

		for (CurrentChannel = 2; CurrentChannel < 7; CurrentChannel++)
			PowerDeltaPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 1, 7, PowerDeltaPerChannel[1], PowerDeltaPerChannel[7]);

		//insert channel 8 to 12 TSSI values
		//for(CurrentChannel = 8; CurrentChannel < 13; CurrentChannel++)
			//TssiRefPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 7, 13, TssiRefPerChannel[7], TssiRefPerChannel[13]);
		for (CurrentChannel = 8; CurrentChannel < 13; CurrentChannel++)
			PowerDeltaPerChannel[CurrentChannel] = RTATEInsertTssi(CurrentChannel, 7, 13, PowerDeltaPerChannel[7], PowerDeltaPerChannel[13]);


		// channel 14 TSSI equals channel 13 TSSI
		//TssiRefPerChannel[14] = TssiRefPerChannel[13];
		PowerDeltaPerChannel[14] = PowerDeltaPerChannel[13];

		for (CurrentChannel = 1; CurrentChannel <= 14; CurrentChannel++)
		{
			DBGPRINT(RT_DEBUG_WARN, ("Channel %d, PowerDeltaPerChannel= 0x%x\n", CurrentChannel, PowerDeltaPerChannel[CurrentChannel]));
		
			//PowerDeltaPerChannel[CurrentChannel] = GetPowerDeltaFromTssiRatio(TssiRefPerChannel[CurrentChannel], TssiBase);

			// boundary check
			if (PowerDeltaPerChannel[CurrentChannel] > 7)
				PowerDeltaPerChannel[CurrentChannel] = 7;
			if (PowerDeltaPerChannel[CurrentChannel] < -8)
				PowerDeltaPerChannel[CurrentChannel] = -8;

			// eeprom only use 4 bit for TSSI delta
			PowerDeltaPerChannel[CurrentChannel] &= 0x0f;
			DBGPRINT(RT_DEBUG_WARN, ("Channel = %d, PowerDeltaPerChannel=0x%x\n", CurrentChannel, PowerDeltaPerChannel[CurrentChannel]));	
		}


		//step 4: store TSSI delta values to EEPROM 0x6f - 0x75
		RT28xx_EEPROM_READ16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);
		EEPData &= 0x00ff;
		EEPData |= (PowerDeltaPerChannel[1] << 8) | (PowerDeltaPerChannel[2] << 12);
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TX_POWER_OFFSET_OVER_CH_1-1, EEPData);
		
		for (CurrentChannel = 3; CurrentChannel <= 14; CurrentChannel += 4)
		{
			//EEPData = ( TssiDeltaPerChannel[CurrentChannel+2]  << 12) |(  TssiDeltaPerChannel[CurrentChannel+1]  << 8);
			//DBGPRINT(RT_DEBUG_TRACE, ("CurrentChannel=%d, TssiDeltaPerChannel[CurrentChannel+2] = 0x%x, EEPData=0x%x\n", CurrentChannel, TssiDeltaPerChannel[CurrentChannel+2], EEPData));
			EEPData = (PowerDeltaPerChannel[CurrentChannel + 3] << 12) | (PowerDeltaPerChannel[CurrentChannel + 2] << 8) | 
				(PowerDeltaPerChannel[CurrentChannel + 1] << 4) | PowerDeltaPerChannel[CurrentChannel];
			RT28xx_EEPROM_WRITE16(pAd, (EEPROM_TX_POWER_OFFSET_OVER_CH_3 + ((CurrentChannel - 3) / 2)), EEPData);
			//DBGPRINT(RT_DEBUG_TRACE, ("offset=0x%x, EEPData = 0x%x\n", (EEPROM_TSSI_DELTA_CH3_CH4 +((CurrentChannel-3)/2)),EEPData));	
		}
						
		// restore RF R27 and R28, BBP R47
		//RT30xxWriteRFRegister(pAd, RF_R27, RF27Value);				
		//RT30xxWriteRFRegister(pAd, RF_R28, RF28Value);

		Set_ATE_Proc(pAd, "ATESTART");
	}

	return TRUE;
}
#endif /* defined(RT5390) || defined(RT5370) */
#endif // RTMP_INTERNAL_TX_ALC //

#if defined(RT5390)  || defined(RT5370)
INT Set_ATE_READ_EXTERNAL_TSSI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR	inputData, RFValue, BbpData = 0;
	USHORT	EEPData/*, EEPTemp*/;

	inputData = simple_strtol(arg, 0, 10);
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, EEPData);

	if (!(IS_RT5392(pAd) && ((EEPData & 0x02) != 0)))			
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Not support TSSI calibration since not 5392 chip or EEPROM not set!!!\n"));
		return FALSE;
	}
	else
	{	
		//BBP R47[7]=1 : ADC 6 on
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpData);
		BbpData |= 0x80;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpData);

		//RF R27[7:6]=0x1 : Adc_insel 01:Temp	
		//Write RF R27[3:0]=EEPROM 0x76 bit[3:0]
		RT30xxReadRFRegister(pAd, RF_R27, &RFValue);	
		RFValue &= ~0xC0;
		RFValue |= 0x40;
		RFValue = ((RFValue & 0xF0) | pAd->TssiGain); // [3:0] = (tssi_gain and tssi_atten)
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);			

		//Wait 1ms.					
		RTMPusecDelay(1000);

		//Read BBP R49 reading as return value.
		DBGPRINT(RT_DEBUG_TRACE, ("Read  BBP_R49\n")); 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpData);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP R49=0x%x\n", BbpData));

		RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_STEP_OVER_2DOT4G, EEPData);
		//RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_STEP_OVER_2DOT4G, EEPTemp);
		//BbpData = (BbpData + (UCHAR) (EEPTemp & 0xff)) / 2;
		EEPData &= 0xff00;
		EEPData |= BbpData;
			
#ifdef RTMP_EFUSE_SUPPORT
		if (pAd->bUseEfuse)
		{
			if (pAd->bFroceEEPROMBuffer)
				NdisMoveMemory(&(pAd->EEPROMImage[EEPROM_TSSI_STEP_OVER_2DOT4G]), (PUCHAR) (&EEPData) ,2);
			else
			    	eFuseWrite(pAd, EEPROM_TSSI_STEP_OVER_2DOT4G, (PUSHORT) (&EEPData), 2);
		}
		else
#endif // RTMP_EFUSE_SUPPORT //
		{
			RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_STEP_OVER_2DOT4G, EEPData);
			RTMPusecDelay(10);
		}   

		//RF R27[7:6]=0x0
		RT30xxReadRFRegister(pAd, RF_R27, &RFValue);	
		RFValue &= ~0xC0;
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);		
	}

	return TRUE;
}

#ifdef IQ_CAL_SUPPORT
/* 
==========================================================================
    Description:
        Enable ATE I/Q gain/phase compensation.
        
        0: disable(default)
        1: enable
        
        Return:
        	TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	Set_ATE_IQ_Compensation_Proc(
	IN PRTMP_ADAPTER	pAd, 
	IN PSTRING			arg)
{
	UINT32 value = simple_strtol(arg, 0, 10);

	if (value > 0)
	{
		pAd->ate.bIQCompEnable = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("bIQCompEnable = TRUE , I/Q compensation enabled !\n"));
	}
	else
	{
		pAd->ate.bIQCompEnable = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("bIQCompEnable = FALSE , I/Q compensation disabled !\n"));
	}	

	return TRUE;
}

VOID RT5392ATEIQCompensation(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel)
{
	UCHAR 	BBPValue;
	UINT16 	E2PValue;	

	if (IS_RT5392(pAd) && (pAd->ate.bIQCompEnable == TRUE))
	{
		/* IQ Calibration */
		if (Channel <= 14)
		{
			/* TX0 IQ Gain */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2C);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX0, IQ_CAL_GAIN);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			
			/* TX0 IQ Phase */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2D);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX0, IQ_CAL_PHASE);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

			/* TX1 IQ Gain */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4A);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX1, IQ_CAL_GAIN);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			
			/* TX1 IQ Phase */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x4B);
			BBPValue = IQCal(IQ_CAL_2G, IQ_CAL_TX1, IQ_CAL_PHASE);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

			/* RF IQ Compensation Control */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x04);
			RT28xx_EEPROM_READ16(pAd, EEPROM_RF_IQ_COMPENSATION_CONTROL, E2PValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, E2PValue & 0x00FF);

			/* RF IQ Imbalance Compensation Control */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x03);
			RT28xx_EEPROM_READ16(pAd, EEPROM_RF_IQ_IMBALANCE_COMPENSATION_CONTROL, E2PValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, E2PValue & 0x00FF);
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("%s() : 5 GHz band not supported !\n", __FUNCTION__));
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("%s() : Not support IQ compensation since not RT5392 or not enabled !\n", __FUNCTION__));
}

VOID RT5390ATEIQCompensation(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			Channel)
{
	INT 		BBPIndex;
	UINT16 	E2PValue;

	if (IS_RT5390(pAd) && !IS_RT5392(pAd) && (pAd->ate.bIQCompEnable == TRUE))
	{
		/* IQ Calibration */
		if (Channel <= 14)
		{
			for (BBPIndex = 0; BBPIndex <= 7; BBPIndex++)
			{
				RT28xx_EEPROM_READ16(pAd, EEPROM_IQ_GLOBAL_BBP_ACCESS_BASE + (BBPIndex * 2), E2PValue);
				
				if (E2PValue == 0xFFFF)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s() : EEPROM 0x%X is not calibrated !\n", __FUNCTION__, 
						(EEPROM_IQ_GLOBAL_BBP_ACCESS_BASE + (BBPIndex * 2))));
					continue;
				}
				else
				{
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, (UCHAR)((E2PValue >> 8) & 0x00FF), (UCHAR)(E2PValue & 0x00FF));
				}
			}
		}
		else
			DBGPRINT(RT_DEBUG_TRACE, ("%s() : 5 GHz band not supported !\n", __FUNCTION__));
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("%s() : Not support IQ compensation since not RT5390 or not enabled !\n", __FUNCTION__));
}
#endif /* IQ_CAL_SUPPORT */
#endif /* defined(RT5390)  || defined(RT5370) */

#ifdef RT33xx
static VOID TX_EVM_CAL_GET(
	IN PRTMP_ADAPTER pAd,
	OUT PUINT8 pRfReg,
	OUT PUINT8 pBitMap,
	OUT PUINT8 pRange,
	OUT PUINT8 pShift)
{
	UINT16 Value;                              
 	UINT loop;
 
	do
 	{
		RT28xx_EEPROM_READ16(pAd, 0x012c,  Value);
		if (Value == 0xffff)
		{
		DBGPRINT(RT_DEBUG_OFF, ("Invalid value of TxEvmCal (%x)\n", Value));
		break;
		}
 
		*pRfReg = (UINT8) ((Value & 0xff00) >> 8);
		*pBitMap = (UINT8) (Value & 0x00ff);
 
		*pRange = *pBitMap;
		*pShift = 0;
		for (loop = 0; loop<8; loop++)
		{
   
		if ((*pRange & (0x01 << loop)) == 0)
			(*pShift)++;
		else
			break;
	}
	*pRange = (*pRange) >> (*pShift);
 
	} while (0);
}
 
INT Set_ATE_TX_EVM_CALIBRATION_Show_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING   arg)
{
	UINT8 RfReg;
	UINT8 BitMap;
	UINT8 Range;
	UINT8 Shift;
 
	TX_EVM_CAL_GET(pAd, &RfReg, &BitMap, &Range, &Shift);
	DBGPRINT(RT_DEBUG_OFF, ("RfReg=%d, Range=%d, BitMap=%x, Shift=%d\n", RfReg, Range, BitMap, Shift));
 
	return TRUE; 
}
 
INT Set_ATE_TX_EVM_CALIBRATION_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING   arg)
{
	UINT8 RfReg;
	UINT8 BitMap;
	UINT8 Range;
	UINT8 Shift;
	LONG Value;
	UCHAR RFValue;
	
 
	TX_EVM_CAL_GET(pAd, &RfReg, &BitMap, &Range, &Shift);
	Value = simple_strtol(arg, 0, 10);
 
	if (Value > Range)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Invalid Range value(%ld), Range=%d\n", Value, Range));
		return TRUE;
	}
	
 	// wite to coresponding Rf register.
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RfReg, &RFValue);
	RFValue &= ~(BitMap);
	RFValue |= (Value << Shift);
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RfReg, RFValue);
 
	return TRUE;
}
 
INT Set_ATE_TX_EVM_CALIBRATION_Fill_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING   arg)
{
	UINT8 RfReg;
	UINT8 BitMap;
	UINT8 Range;
	UINT8 Shift;
	LONG Value;
	UINT EPRomOffset;
	UINT16 TargetValue;
 
	TX_EVM_CAL_GET(pAd, &RfReg, &BitMap, &Range, &Shift);
 
	switch(RfReg)
	{
	case RF_R09:
		EPRomOffset = EEPROM_EVM_RF09;
		break;
	case RF_R19:
		EPRomOffset = EEPROM_EVM_RF19;
		break;
	case RF_R21:
		EPRomOffset = EEPROM_EVM_RF21;
		break;
	case RF_R29:
		EPRomOffset = EEPROM_EVM_RF29;
		break;
	default:
	DBGPRINT(RT_DEBUG_OFF, ("Invalid RfReg(%d)", RfReg));
	return TRUE;
	}
 
	TargetValue = simple_strtol(arg, 0, 10); 
	if (TargetValue > Range)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Invalid Range value(%d), Range=%d",TargetValue, Range));
		return TRUE;
	}
 
	RT28xx_EEPROM_READ16(pAd, EPRomOffset,  Value);
	Value &= ~(BitMap);
	Value |= (TargetValue << Shift);
	RT28xx_EEPROM_WRITE16(pAd, EPRomOffset, Value);
 
	return TRUE; 
}
#endif // RT33xx //
